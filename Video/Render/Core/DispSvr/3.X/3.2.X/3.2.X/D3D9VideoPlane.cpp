#include "stdafx.h"
#include "D3D9VideoMixerModel.h"
#include "D3D9VideoMixerBase.h"
#include "D3D9VideoPlane.h"

using namespace DispSvr;
#define _BASE_VIEW_ID_ 0

// Recover colors by dividing with alpha.
class CSWInverseAlphaFilter : public ITextureFilter
{
public:
	CSWInverseAlphaFilter(D3D9Plane *pPlane) : m_pPlane(pPlane)
	{
		SetRectEmpty(&m_rcVisible);
	}

	virtual ~CSWInverseAlphaFilter() { }

	virtual HRESULT Process(const RECT &rcLocked, const LockedRect &lockedBuffer)
	{
		int Width = rcLocked.right - rcLocked.left;
		int Height = rcLocked.bottom - rcLocked.top;

		UCHAR *pLine = static_cast<UCHAR *> (lockedBuffer.pBuffer);
		UCHAR *pEnd = pLine + Height * lockedBuffer.uPitch;
		double r, g, b, a;
		bool bVisible = false;
		
		for ( ; pLine < pEnd; pLine += lockedBuffer.uPitch)
		{
			for (UCHAR *p = pLine; p < pLine + Width * 4; p += 4)
			{
				if (p[3])
				{
					a = 255.0 / p[3];
					b = p[0] * a;
					g = p[1] * a;
					r = p[2] * a;

					ASSERT(b <= 255.1 && g <= 255.1 && r <= 255.1);
					p[0] = UCHAR(b);
					p[1] = UCHAR(g);
					p[2] = UCHAR(r);
					bVisible = true;
				}
			}
		}

		if (!bVisible)
			SubtractRect(&m_rcVisible, &m_rcVisible, &rcLocked);
		else
			UnionRect(&m_rcVisible, &m_rcVisible, &rcLocked);

		// return other than S_OK to make the plane invalid (invisible)
		return IsRectEmpty(&m_rcVisible) ? E_FAIL : S_OK;
 	}

private:
	D3D9Plane *m_pPlane;
	RECT m_rcVisible;
};

HRESULT CD3D9VideoMixerPlane::Create(const PlaneInit &init, CD3D9VideoMixerPlane **ppPlane)
{
	HRESULT hr = E_FAIL;

	if (init.PlaneID < 0 || init.PlaneID >= PLANE_MAX)
		return E_INVALIDARG;

    *ppPlane = new CD3D9VideoMixerPlane(init);
    if (!*ppPlane)
    {
        return E_OUTOFMEMORY;
    }

    hr = (*ppPlane)->Init();
    if (FAILED(hr))
    {
        delete *ppPlane;
        *ppPlane = 0;
    }

    return hr;
}

CD3D9VideoMixerPlane::CD3D9VideoMixerPlane(const PlaneInit &InitOpt)
: m_pPlaneCB(0), m_pMixer(0), m_pModel(0), m_pTexturePool(0), m_cRef(0), m_InitOpt(InitOpt)
{
	m_bExternalSurface = (m_InitOpt.dwFlags & PLANE_INIT_EXTERNAL_SURFACE) != 0;
    IUnknown *pUnknown = 0;
	CResourceManager::GetInstance()->GetInterface(DISPSVR_RESOURCE_TEXTUREPOOL, __uuidof(IUnknown), (void **)&pUnknown);
    m_pTexturePool = (CD3D9TexturePool *)pUnknown;
	CResourceManager::GetInstance()->GetInterface(DISPSVR_RESOURCE_VIDEOMIXERMODEL, __uuidof(IUnknown), (void **)&pUnknown);
    m_pModel = (CD3D9VideoMixerModel *)pUnknown;
	CResourceManager::GetInstance()->GetInterface(DISPSVR_RESOURCE_VIDEOMIXER, __uuidof(IDispSvrVideoMixer), (void **)&m_pMixer);
	CResourceManager::GetInstance()->GetInterface(DISPSVR_RESOURCE_VIDEOMIXER, __uuidof(IDispSvrVideoPlaneCallback), (void **)&m_pPlaneCB);
}

CD3D9VideoMixerPlane::~CD3D9VideoMixerPlane()
{
	CAutoLock selfLock(m_pModel);
	D3D9Plane &plane = m_pModel->GetPlane(m_InitOpt.PlaneID);
	HRESULT hr = E_FAIL;

	if (plane.hTexture)
	{
		hr = m_pTexturePool->ReleaseTexture(plane.hTexture);
		plane.hTexture = 0;
	}

    if (plane.hDeptViewTexture)
    {
        hr = m_pTexturePool->ReleaseTexture(plane.hDeptViewTexture);
        plane.hDeptViewTexture = 0;
    }

	while (!plane.BackwardSamples.empty())
	{
		hr = m_pTexturePool->ReleaseTexture(plane.BackwardSamples.back().hTexture);
		plane.BackwardSamples.pop_back();
	}

	SAFE_DELETE(plane.pPostTextureFilter);
	SAFE_DELETE(plane.pVBlt);
    SAFE_DELETE_ARRAY(plane.pStereoMetadata);
	memset(plane.Palette, 0, sizeof(plane.Palette));
    plane.uStereoMetadataMaxLength = 0;
    plane.uStereoMetadataLength = 0;
	plane.bPalettized = false;
	plane.bValid = false;
	plane.bCreated = false;

	m_pPlaneCB->OnDestroyPlane(m_InitOpt.PlaneID);
    SAFE_RELEASE(m_pPlaneCB);
	SAFE_RELEASE(m_pMixer);
    SAFE_RELEASE(m_pModel);
    SAFE_RELEASE(m_pTexturePool);
}

static inline void LoadDefaultD3D9Plane(D3D9Plane &plane)
{
	ZeroMemory(&plane, sizeof(D3D9Plane));
	plane.fAlpha = 1.0f;
	LoadDefaultMixerCoordinate(plane.nrcDst);
	LoadDefaultMixerCoordinate(plane.nrcCrop);
}

HRESULT CD3D9VideoMixerPlane::Init()
{
    CAutoLock selfLock(m_pModel);
	D3D9Plane &plane = m_pModel->GetPlane(m_InitOpt.PlaneID);
	const D3D9PlaneConfig &cfg = m_pPlaneCB->GetPlaneConfig(m_InitOpt.PlaneID);

	if (plane.bCreated)
		return E_FAIL;

	LoadDefaultD3D9Plane(plane);

	PlaneCaps caps;
	HRESULT hr = m_pMixer->QueryPlaneCaps(m_InitOpt.PlaneID, m_InitOpt.Format, &caps);
	if (FAILED(hr))
		return hr;

	// if size exceeds max cap, we treat it as an error.
	if (PLANE_CAP_MAX_SIZE & caps.dwFlags)
    {
		if (m_InitOpt.uWidth > caps.uMaxWidth || m_InitOpt.uHeight > caps.uMaxHeight)
			return E_INVALIDARG;
    }

	if (m_InitOpt.pListener)
	{
		// we don't add reference to listener, users should be responsible for the life time of the listener.
		if (SUCCEEDED(m_InitOpt.pListener->QueryInterface(__uuidof(IDispSvrVideoMixerEventListener), (void **)&plane.pListener)))
			plane.pListener->Release();
	}

	if (m_InitOpt.dwFlags & PLANE_INIT_EXTERNAL_SURFACE)
	{
		plane.bExternal = true;
		plane.rcSrc.right = m_InitOpt.uWidth;
		plane.rcSrc.bottom = m_InitOpt.uHeight;
		hr = S_OK;
	}
	else
	{
		if (PLANE_CAP_MIN_SIZE & caps.dwFlags)
		{
			if (m_InitOpt.uWidth < caps.uMinWidth)
				m_InitOpt.uWidth = caps.uMinWidth;
			if (m_InitOpt.uHeight < caps.uMinHeight)
				m_InitOpt.uHeight = caps.uMinHeight;
		}

		CreateTextureParam sParam = {0};

		sParam.uWidth = m_InitOpt.uWidth;
		sParam.uHeight = m_InitOpt.uHeight;
		sParam.Format = m_InitOpt.Format;
		memcpy(sParam.Palette, m_InitOpt.Palette, sizeof(sParam.Palette));
		// if we need to post processing on a lockable surface, we create a backing store on system memory.
        // the post filter of the plane has clean by LoadDefaultD3D9Plane()
		sParam.eUsage = plane.pPostTextureFilter ? TEXTURE_USAGE_LOCKABLE_BACKSTORE : cfg.eTextureUsage;
		hr = m_pTexturePool->CreateTexture(&sParam, &plane.hTexture);
		if (SUCCEEDED(hr))
		{
			LockedRect lockedRect;
			if (m_InitOpt.Format == PLANE_FORMAT_ARGB || m_InitOpt.Format == PLANE_FORMAT_P8)
  			{
 				// Clear texture while first initialized, because the later updates only updates the dirty region.
 				RECT rect = {0, 0, m_InitOpt.uWidth, m_InitOpt.uHeight};
 				hr = m_pTexturePool->LockBuffer(plane.hTexture, &lockedRect, &rect, 0);	ASSERT(SUCCEEDED(hr));
				memset(lockedRect.pBuffer, 0, lockedRect.uPitch*m_InitOpt.uHeight);
 			}
  			else 
 			{
 				hr = m_pTexturePool->LockBuffer(plane.hTexture, &lockedRect, NULL, PLANE_LOCK_READONLY);	ASSERT(SUCCEEDED(hr));
			}
			hr = m_pTexturePool->UnlockBuffer(plane.hTexture);	ASSERT(SUCCEEDED(hr));
			plane.uPitch = lockedRect.uPitch;
			plane.rcSrc.right = sParam.uWidth;
			plane.rcSrc.bottom = sParam.uHeight;
			plane.bHasBackingStore = (sParam.eUsage & TEXTURE_USAGE_BACKSTORE) != 0;
			plane.bHDVideo = IsHDVideo(sParam.uWidth, sParam.uHeight);
		}
	}

	if (SUCCEEDED(hr))
	{
		if (IsVideoPlane())
			m_pPlaneCB->OnVideoPositionChange();

		if (m_InitOpt.dwFlags & PLANE_INIT_PARTIAL_BLENDING)
		{
			plane.bPartialBlending = true;
#ifdef _EXPERIMENTING_
			if (m_InitOpt.Format == PLANE_FORMAT_ARGB && (caps.dwFlags & PLANE_CAP_HW_PARTIAL_BLENDING) == 0)
				plane.pPostTextureFilter = new CSWInverseAlphaFilter(&plane);
#endif
		}

		if (m_InitOpt.dwFlags & PLANE_INIT_FULLSCREEN_MIXING)
		{
			plane.bFullScreenMixing = true;
		}

		plane.bCreated = true;
		plane.uWidth = m_InitOpt.uWidth;
		plane.uHeight = m_InitOpt.uHeight;
		plane.Format = m_InitOpt.Format;
		plane.dwLastUpdateTS++;

		if (m_InitOpt.Format == PLANE_FORMAT_P8)
		{
			plane.bPalettized = true;
			memcpy(plane.Palette, m_InitOpt.Palette, sizeof(m_InitOpt.Palette));
		}

		// pVBlt is used as hardware deinterlacer or color space converter.
		if (cfg.pVideoProcessorStub)
		{
			ASSERT(plane.pVBlt == 0);
			plane.pVBlt = CreateVBltFromVPStub(&plane, cfg.pVideoProcessorStub, m_pTexturePool);
		}

        hr = m_pPlaneCB->OnCreatePlane(m_InitOpt);
	}
    return hr;
}

STDMETHODIMP CD3D9VideoMixerPlane::QueryInterface(REFIID riid, void **ppv)
{
	HRESULT hr = E_NOINTERFACE;

	if (riid == __uuidof(IUnknown))
	{
        hr = GetInterface((IUnknown *)(IDispSvrVideoMixerVideoPlane *)this, ppv);
	}
	else if (riid == __uuidof(IDispSvrVideoMixerPlane))
	{
		hr = GetInterface((IDispSvrVideoMixerPlane *)this, ppv);
	}
	else if (riid == __uuidof(IDispSvrVideoMixerVideoPlane))
	{
		if (IsVideoPlane())
			hr = GetInterface((IDispSvrVideoMixerVideoPlane *)this, ppv);
	}
    else if (riid == __uuidof(IDispSvrVideoMixerDependentView))
    {
        hr = GetInterface((IDispSvrVideoMixerDependentView *)this, ppv);
    }
    else if (riid == __uuidof(IDispSvrVideoMixerPlaneStereoControl))
    {
        hr = GetInterface((IDispSvrVideoMixerPlaneStereoControl *)this, ppv);
    }
	return hr;
}

STDMETHODIMP_(ULONG) CD3D9VideoMixerPlane::AddRef()
{
	LONG lRef = InterlockedIncrement(&m_cRef);
	ASSERT(lRef > 0);
	return lRef;
}

STDMETHODIMP_(ULONG) CD3D9VideoMixerPlane::Release()
{
	LONG lRef = InterlockedDecrement(&m_cRef);
	ASSERT(lRef >= 0);

	if (lRef == 0)
		delete this;
	return lRef;
}

STDMETHODIMP CD3D9VideoMixerPlane::GetPlaneDesc(DispSvr::PlaneDesc *pDesc)
{
	CHECK_POINTER(pDesc);
	CAutoLock selfLock(m_pModel);
	const D3D9Plane &plane = m_pModel->GetPlane(m_InitOpt.PlaneID);

	if (plane.bCreated)
	{
		pDesc->uWidth = plane.uWidth;
		pDesc->uHeight = plane.uHeight;
		pDesc->uPitch = plane.uPitch;
		pDesc->Format = plane.Format;
		return S_OK;
	}
	return E_INVALIDARG;
}

STDMETHODIMP CD3D9VideoMixerPlane::SetExternalSurface(const ExternalSurfaceDesc *pEx)
{
	if (m_bExternalSurface)
	{
		if (pEx && pEx->bQueryStatusOnly)
			return _QueryStatusOfExternalSurface(m_InitOpt.PlaneID, pEx);
		else
			return _SetExternalSurface(m_InitOpt.PlaneID, pEx);
	}
	else
		return E_INVALIDARG;
}

STDMETHODIMP CD3D9VideoMixerPlane::UpdateFromBuffer(UpdateBuffer *pBuffer)
{
	if (m_bExternalSurface)
		return E_INVALIDARG;
	else
		return E_NOTIMPL;;
}

STDMETHODIMP CD3D9VideoMixerPlane::LockBuffer(LockedRect *pOut, const RECT *rcSrc, DWORD dwFlags)
{
	if (m_bExternalSurface)
		return E_INVALIDARG;

    m_pModel->ReaderLock();

    CAutoLock selfLock(m_pModel);
	const D3D9Plane &plane = m_pModel->GetPlane(m_InitOpt.PlaneID);
    HRESULT hr = m_pTexturePool->LockBuffer(plane.hTexture, pOut, rcSrc, dwFlags);
	if (FAILED(hr))
	{
        m_pModel->ReaderUnlock();
	}
    return hr;
}

STDMETHODIMP CD3D9VideoMixerPlane::UnlockBuffer()
{
	if (m_bExternalSurface)
		return E_INVALIDARG;

    CAutoLock selfLock(m_pModel);
	D3D9Plane &plane = m_pModel->GetPlane(m_InitOpt.PlaneID);
	HRESULT hr = m_pTexturePool->UnlockBuffer(plane.hTexture, plane.pPostTextureFilter);
	if (SUCCEEDED(hr))
	{
	    if (S_OK == hr)
	    {
		    plane.bValid = true;
		    plane.dwLastUpdateTS++;
	    }
	    else
	    {
		    plane.bValid = false;
	    }

        m_pModel->ReaderUnlock();
	}
	return hr;
}

STDMETHODIMP CD3D9VideoMixerPlane::SetPlanarAlpha(float fAlpha)
{
	if (fAlpha < 0.0f || fAlpha > 1.0f)
		return E_INVALIDARG;

	CAutoLock selfLock(m_pModel);
	m_pModel->GetPlane(m_InitOpt.PlaneID).fAlpha = fAlpha;
	return S_OK;
}

STDMETHODIMP CD3D9VideoMixerPlane::GetPlanarAlpha(float *pfAlpha)
{
	CHECK_POINTER(pfAlpha);
	CAutoLock selfLock(m_pModel);
	*pfAlpha = m_pModel->GetPlane(m_InitOpt.PlaneID).fAlpha;
	return S_OK;
}

STDMETHODIMP CD3D9VideoMixerPlane::SetPosition(const NORMALIZEDRECT *lprcDst, const RECT *lprcSrc, const NORMALIZEDRECT *lprcCrop, float fAspectRatio)
{
	if (!lprcDst || !lprcSrc || !lprcCrop
		|| lprcCrop->right <= lprcCrop->left || lprcCrop->bottom <= lprcCrop->top
		|| lprcDst->right <= lprcDst->left || lprcDst->bottom <= lprcDst->top)
		return E_INVALIDARG;

	CAutoLock selfLock(m_pModel);
	D3D9Plane &plane = m_pModel->GetPlane(m_InitOpt.PlaneID);

	plane.nrcDst = *lprcDst;
	plane.nrcCrop = *lprcCrop;
	if (IsRectEmpty(lprcSrc))
		SetRect(&plane.rcSrc, 0, 0, plane.uWidth, plane.uHeight);
	else
		plane.rcSrc = *lprcSrc;
	plane.fAspectRatio = fAspectRatio;
	if (IsVideoPlane())
		m_pPlaneCB->OnVideoPositionChange();
	return S_OK;
}

STDMETHODIMP CD3D9VideoMixerPlane::GetPosition(NORMALIZEDRECT *rcDst, RECT *rcSrc, NORMALIZEDRECT *rcCrop, float *pfAspectRatio)
{
	CAutoLock selfLock(m_pModel);
	D3D9Plane &plane = m_pModel->GetPlane(m_InitOpt.PlaneID);

	if (rcDst)
		*rcDst = plane.nrcDst;
	if (rcCrop)
		*rcCrop = plane.nrcCrop;
	if (rcSrc)
		*rcSrc = plane.rcSrc;
	if (pfAspectRatio)
		*pfAspectRatio = plane.fAspectRatio;
	return S_OK;
}

STDMETHODIMP CD3D9VideoMixerPlane::SetDirtyRect(const RECT *rcDirty)
{
	CHECK_POINTER(rcDirty);
	CAutoLock selfLock(m_pModel);
	D3D9Plane &plane = m_pModel->GetPlane(m_InitOpt.PlaneID);

	if (rcDirty)
		plane.rcDirty = *rcDirty;
	else
		ZeroMemory(&plane.nrcDst, sizeof(plane.nrcDst));
	return S_OK;
}

STDMETHODIMP CD3D9VideoMixerPlane::SetVideoProperty(const VideoProperty *pProperty)
{
	ASSERT(IsVideoPlane());

	CHECK_POINTER(pProperty);
	CAutoLock selfLock(m_pModel);
	D3D9Plane &plane = m_pModel->GetPlane(m_InitOpt.PlaneID);

	// check if samples are continuous; otherwise clear the reference samples.
	if (!plane.BackwardSamples.empty() && (pProperty->bStillMenuHint || pProperty->rtStart != plane.BackwardSamples.front().VideoProperty.rtEnd))
	{
		do
		{
			m_pTexturePool->ReleaseTexture(plane.BackwardSamples.back().hTexture);
			plane.BackwardSamples.pop_back();
		} while (!plane.BackwardSamples.empty());
	}

	if (memcmp(&plane.VideoProperty, pProperty, sizeof(VideoProperty)) != 0)
	{
		plane.dwLastUpdateTS++;
		memcpy(&plane.VideoProperty, pProperty, sizeof(VideoProperty));
	}
	return S_OK;
}

STDMETHODIMP CD3D9VideoMixerPlane::GetVideoProperty(VideoProperty *pProperty)
{
	ASSERT(IsVideoPlane());

	CHECK_POINTER(pProperty);
	CAutoLock selfLock(m_pModel);
	memcpy(pProperty, &m_pModel->GetPlane(m_InitOpt.PlaneID).VideoProperty, sizeof(VideoProperty));
    return S_OK;
}

STDMETHODIMP CD3D9VideoMixerPlane::SetColorControl(const ColorControl *pCC)
{
	return E_NOTIMPL;
}

STDMETHODIMP CD3D9VideoMixerPlane::GetColorControl(ColorControl *pCC)
{
	return E_NOTIMPL;
}

STDMETHODIMP CD3D9VideoMixerPlane::SetDeptViewExternalSurface(UINT uViewID, const DispSvr::ExternalSurfaceDesc *pEx)
{
    if (m_bExternalSurface)
    {
        if (pEx && pEx->bQueryStatusOnly)
            return _QueryStatusOfExternalSurface(m_InitOpt.PlaneID, pEx, uViewID+1);
        else
            return _SetExternalSurface(m_InitOpt.PlaneID, pEx, uViewID+1);
    }
    else
        return E_INVALIDARG;

}

STDMETHODIMP CD3D9VideoMixerPlane::UpdateDeptViewFromBuffer(UINT uViewID, DispSvr::UpdateBuffer *pBuffer)
{
    if (m_bExternalSurface)
        return E_INVALIDARG;
    else
        return E_NOTIMPL;
}

STDMETHODIMP CD3D9VideoMixerPlane::LockDeptViewBuffer(UINT uViewID, DispSvr::LockedRect *pOut, const RECT *rcSrc, DWORD dwFlags)
{
    if (m_bExternalSurface)
        return E_INVALIDARG;

    m_pModel->ReaderLock();

	CAutoLock selfLock(m_pModel);
	const D3D9Plane &plane = m_pModel->GetPlane(m_InitOpt.PlaneID);

    HRESULT hr = E_FAIL;

    if (!plane.hDeptViewTexture)
    {
        hr = PrepareDeptViewBuffer(m_InitOpt.PlaneID, uViewID + 1);
        if (FAILED(hr))
        {
            m_pModel->ReaderUnlock();
            return hr;
        }
    }

    hr = m_pTexturePool->LockBuffer(plane.hDeptViewTexture, pOut, rcSrc, dwFlags);
    if (FAILED(hr))
    {
        m_pModel->ReaderUnlock();
	}
    return hr;
}

STDMETHODIMP CD3D9VideoMixerPlane::UnlockDeptViewBuffer(UINT uViewID)
{
    if (m_bExternalSurface)
        return E_INVALIDARG;

	CAutoLock selfLock(m_pModel);
	D3D9Plane &plane = m_pModel->GetPlane(m_InitOpt.PlaneID);
	HRESULT hr = m_pTexturePool->UnlockBuffer(plane.hDeptViewTexture, plane.pPostTextureFilter);
	if (SUCCEEDED(hr))
	{
        m_pModel->ReaderUnlock();
	}
	return hr;
}

HRESULT CD3D9VideoMixerPlane::PrepareDeptViewBuffer(PLANE_ID PlaneID, UINT uViewID)
{
    HRESULT hr = E_FAIL;
    D3D9Plane &plane = m_pModel->GetPlane(PlaneID);

    if (!plane.bCreated)
        return E_FAIL;

    CreateTextureParam sParam = {0};

    sParam.uWidth = plane.uWidth;
    sParam.uHeight = plane.uHeight;
    sParam.Format = plane.Format;
    memcpy(sParam.Palette, plane.Palette, sizeof(sParam.Palette));
    // if we need to post processing on a lockable surface, we create a backing store on system memory.
    sParam.eUsage = plane.pPostTextureFilter ? TEXTURE_USAGE_LOCKABLE_BACKSTORE : TEXTURE_USAGE_LOCKABLE;
    HANDLE hTexture;
    hr = m_pTexturePool->CreateTexture(&sParam, &hTexture);
    if (SUCCEEDED(hr))
    {
        LockedRect lockedRect;
        if (plane.Format == PLANE_FORMAT_ARGB || plane.Format == PLANE_FORMAT_P8)
        {
            // Clear texture while first initialized, because the later updates only updates the dirty region.
            RECT rect = {0, 0, plane.uWidth, plane.uHeight};
            hr = m_pTexturePool->LockBuffer(hTexture, &lockedRect, &rect, 0);	ASSERT(SUCCEEDED(hr));
            memset(lockedRect.pBuffer, 0, lockedRect.uPitch*plane.uHeight);
        }
        else 
        {
            hr = m_pTexturePool->LockBuffer(hTexture, &lockedRect, NULL, PLANE_LOCK_READONLY);	ASSERT(SUCCEEDED(hr));
        }
        hr = m_pTexturePool->UnlockBuffer(hTexture);	ASSERT(SUCCEEDED(hr));
    }
//    plane.mapDeptViewTextureHandle.insert(std::pair<UINT, HANDLE>(uViewID, hTexture));
    plane.hDeptViewTexture = hTexture;
    return hr;
}

STDMETHODIMP CD3D9VideoMixerPlane::SetPlaneMetaData(const LPBYTE pMetaData, DWORD dwSize)
{
    if (dwSize > 0 && pMetaData == NULL)
        return E_FAIL;

	CAutoLock selfLock(m_pModel);
    D3D9Plane &plane = m_pModel->GetPlane(m_InitOpt.PlaneID);

    if (dwSize <= 0)
    {
        plane.uStereoMetadataLength = 0;
        return S_OK;
    }

    if (dwSize > plane.uStereoMetadataMaxLength)
    {
        SAFE_DELETE_ARRAY(plane.pStereoMetadata);
        plane.uStereoMetadataMaxLength = 0;
        plane.uStereoMetadataLength = 0;
        plane.pStereoMetadata = new INT8[dwSize];
        if (plane.pStereoMetadata == NULL)
        {
            return E_FAIL;
        }
        plane.uStereoMetadataMaxLength = dwSize;
        plane.uStereoMetadataLength = dwSize;
    }
    else
    {
        plane.uStereoMetadataLength = dwSize;
    }

    memcpy(plane.pStereoMetadata, pMetaData, sizeof(BYTE) * dwSize);

    return S_OK;
}

STDMETHODIMP CD3D9VideoMixerPlane::GetPlaneMetaData(LPBYTE *ppMetaData, LPDWORD pdwSize)
{
    return E_NOTIMPL;
}

STDMETHODIMP CD3D9VideoMixerPlane::SetStereoStreamMode(STEREO_STREAM_MODE eMode)
{
	CAutoLock selfLock(m_pModel);
    D3D9Plane &plane = m_pModel->GetPlane(m_InitOpt.PlaneID);

    plane.eStereoStreamMode = eMode;

    // force auto mode to side-by-side LR view until auto-detection has done.
    if (eMode == STEREO_STREAM_AUTO)
        plane.eStereoStreamMode = STEREO_STREAM_SIDEBYSIDE_LR;

    return S_OK;
}

STDMETHODIMP CD3D9VideoMixerPlane::GetStereoStreamMode(STEREO_STREAM_MODE *peMode)
{
	CAutoLock selfLock(m_pModel);
    D3D9Plane &plane = m_pModel->GetPlane(m_InitOpt.PlaneID);
    (*peMode) = plane.eStereoStreamMode;
    return S_OK;
}

STDMETHODIMP CD3D9VideoMixerPlane::SetOffsetProperty(const DispSvr::StereoOffsetProperty *pProperty)
{
	CAutoLock selfLock(m_pModel);
    D3D9Plane &plane = m_pModel->GetPlane(m_InitOpt.PlaneID);
    plane.eStereoOffsetMode = pProperty->eMode;
    plane.lStereoDisplayOffset = pProperty->nValue;
    return S_OK;
}

STDMETHODIMP CD3D9VideoMixerPlane::GetOffsetProperty(DispSvr::StereoOffsetProperty *pProperty)
{
	CAutoLock selfLock(m_pModel);
    D3D9Plane &plane = m_pModel->GetPlane(m_InitOpt.PlaneID);
    pProperty->eMode = plane.eStereoOffsetMode;
    pProperty->nValue = plane.lStereoDisplayOffset;
    return S_OK;
}

STDMETHODIMP CD3D9VideoMixerPlane::_QueryStatusOfExternalSurface(PLANE_ID PlaneID, const ExternalSurfaceDesc *pEx, UINT uDeptViewID)
{
	if (!pEx)
		return E_INVALIDARG;

	CAutoLock selfLock(m_pModel);
	D3D9Plane &plane = m_pModel->GetPlane(PlaneID);
	HRESULT hr = S_OK;
	CComPtr<IUnknown> pObj;

    hr = m_pTexturePool->GetRepresentation(plane.hTexture, __uuidof(IUnknown), (void **) &pObj);
	if (SUCCEEDED(hr) && pObj == pEx->pSurface)
		return DDERR_WASSTILLDRAWING;

    pObj.Release();
    hr = m_pTexturePool->GetRepresentation(plane.hDeptViewTexture, __uuidof(IUnknown), (void **) &pObj);
    if (SUCCEEDED(hr) && pObj == pEx->pSurface)
        return DDERR_WASSTILLDRAWING;

	for (VideoSampleList::const_iterator it = plane.BackwardSamples.begin();
		it != plane.BackwardSamples.end();
		++it)
	{
		pObj.Release();
		hr = m_pTexturePool->GetRepresentation(it->hTexture, __uuidof(IUnknown), (void **) &pObj);
		if (SUCCEEDED(hr) && pObj == pEx->pSurface)
			return DDERR_WASSTILLDRAWING;
	}
	return S_OK;
}

STDMETHODIMP CD3D9VideoMixerPlane::_SetExternalSurface(PLANE_ID PlaneID, const ExternalSurfaceDesc *pEx, UINT uDeptViewID)
{
	CAutoLock selfLock(m_pModel);
	D3D9Plane &plane = m_pModel->GetPlane(PlaneID);
	const D3D9PlaneConfig &cfg = m_pPlaneCB->GetPlaneConfig(PlaneID);
	UINT uNumBackwardSamples = cfg.pVideoProcessorStub ? cfg.pVideoProcessorStub->sCaps.uNumBackwardSamples : 0;
	HRESULT hr = S_OK;

	ASSERT(plane.bCreated);

	// workaround for bug#110575, m_pMixer instance is not updated after device reset.
	if (uNumBackwardSamples > 2)
		uNumBackwardSamples = 2;

    // if pEx == NULL, it is to unregister the external surface.
    if (uDeptViewID > _BASE_VIEW_ID_)
    {
        if (pEx == NULL) //release dependent view only.
        {
            m_pTexturePool->ReleaseTexture(plane.hDeptViewTexture);          
            plane.hDeptViewTexture = 0;
            return S_OK;
        }
        else
        {
            //the dependent view should be same size with base view
            ASSERT(plane.uWidth == pEx->uWidth && plane.uHeight == pEx->uHeight && plane.Format == pEx->Format);
            if (plane.hDeptViewTexture)
            {
                CComPtr<IUnknown> pObj;
                hr = m_pTexturePool->GetRepresentation(plane.hDeptViewTexture, __uuidof(IUnknown), (void **) &pObj);
                if (SUCCEEDED(hr) && pObj == pEx->pSurface)
                {
                    return hr;
                }
            }
             m_pTexturePool->ReleaseTexture(plane.hDeptViewTexture);
             plane.hDeptViewTexture = 0;
             hr = m_pTexturePool->CreateTexture(pEx->pSurface, &plane.hDeptViewTexture);
        }
        return hr;
    }

	if (!pEx)
	{
		if (plane.hTexture)
		{
			m_pTexturePool->ReleaseTexture(plane.hTexture);
			plane.hTexture = 0;
		}

        if (plane.hDeptViewTexture)
        {
            m_pTexturePool->ReleaseTexture(plane.hDeptViewTexture);
            plane.hDeptViewTexture = 0;
        }

		while (!plane.BackwardSamples.empty())
		{
			m_pTexturePool->ReleaseTexture(plane.BackwardSamples.back().hTexture);
			plane.BackwardSamples.pop_back();
		}
	}
	else
	{
		// check the previous registered surface.
		if (plane.hTexture)
		{
			if (plane.VideoProperty.Format == VIDEO_FORMAT_PROGRESSIVE)
				uNumBackwardSamples = 0;	// progressive samples do not require references.

			// if width/height/format changes, we should discard all previous samples.
			if (plane.uWidth != pEx->uWidth || plane.uHeight != pEx->uHeight || plane.Format != pEx->Format)
			{
				m_pTexturePool->ReleaseTexture(plane.hTexture);
				plane.hTexture = 0;

                if (plane.hDeptViewTexture) //release dept view texture if base view released
                {
                    m_pTexturePool->ReleaseTexture(plane.hDeptViewTexture);
                    plane.hDeptViewTexture = 0;
                }

				while (!plane.BackwardSamples.empty())
				{
					m_pTexturePool->ReleaseTexture(plane.BackwardSamples.back().hTexture);
					plane.BackwardSamples.pop_back();
				}
			}
			else
			{
				CComPtr<IUnknown> pObj;

				hr = m_pTexturePool->GetRepresentation(plane.hTexture, __uuidof(IUnknown), (void **) &pObj);
				// check if it is the same surface.
				if (SUCCEEDED(hr) && pObj == pEx->pSurface)
				{
					ASSERT(plane.uWidth == pEx->uWidth && plane.uHeight == pEx->uHeight && plane.Format == pEx->Format);
					return hr;
				}

				if (uNumBackwardSamples > 0)
				{
					VideoReferenceSample Sample;
					Sample.VideoProperty = plane.VideoProperty;
					Sample.hTexture = plane.hTexture;
					plane.BackwardSamples.push_front(Sample);
				}
				else
				{
					m_pTexturePool->ReleaseTexture(plane.hTexture);
				}
				plane.hTexture = 0;

				// discard excessive samples in queue.
				// note: plane.uNumBackwardSamples can change dynamically based on input video format/size.
				while (plane.BackwardSamples.size() > uNumBackwardSamples)
				{
					m_pTexturePool->ReleaseTexture(plane.BackwardSamples.back().hTexture);
					plane.BackwardSamples.pop_back();
				}
			}
		}

		ASSERT(plane.hTexture == 0 && plane.BackwardSamples.size() <= uNumBackwardSamples);
		// only reset rcSrc when input surface size is changed.
		if (plane.uWidth != pEx->uWidth || plane.uHeight != pEx->uHeight)
			SetRect(&plane.rcSrc, 0, 0, pEx->uWidth, pEx->uHeight);

		plane.uWidth = pEx->uWidth;
		plane.uHeight = pEx->uHeight;
		plane.Format = pEx->Format;
		plane.dwLastUpdateTS++;
		plane.bHDVideo = IsHDVideo(pEx->uWidth, pEx->uHeight);

		// register a surface with the texture pool.
		hr = m_pTexturePool->CreateTexture(pEx->pSurface, &plane.hTexture);
#ifdef DEBUG
		for (VideoSampleList::const_iterator it = plane.BackwardSamples.begin();
			it != plane.BackwardSamples.end(); ++it)
		{
			ASSERT(plane.hTexture != it->hTexture);
		}
#endif
	}
	plane.bValid = !!plane.hTexture;
	return hr;
}