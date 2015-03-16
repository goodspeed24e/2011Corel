#include "stdafx.h"
#include "DXVA2VideoProcessor.h"
#include "DXVAHDVideoProcessor.h"
#include "D3D9VideoEffect3DManager.h"
#include "Imports/XVE/Inc/XVE.h"
#include "XVEManagerWrapper.h"
#include "DriverExtensionHelper.h"
#include "D3D9VideoPlane.h"
#include "Imports/XVE/Inc/XVEDef.h"
#include "Imports/XVE/Inc/XVEUtil.h"
#include "D3D9VideoMixerBase.h"

using namespace DispSvr;


ID3D9VBlt *DispSvr::CreateVBltFromVPStub(D3D9Plane *pPlane, VideoProcessorStub *pVPStub, CD3D9TexturePool *pPool)
{
	// some of video processor may not have factory a method.
	// plane.pVBlt should be set to null in those cases.
	if (!pVPStub->pfnVBltFactory)
	{
		if (pVPStub->pDelegateVPStub && pVPStub->pDelegateVPStub->pfnVBltFactory)
			pVPStub = pVPStub->pDelegateVPStub;
		else
			return NULL;
	}

	return pVPStub->pfnVBltFactory(pPlane, pVPStub, pPool);
}

//////////////////////////////////////////////////////////////////////////
// CD3D9VideoMixerBase
CD3D9VideoMixerBase::CD3D9VideoMixerBase()
{
	m_GUID = DISPSVR_RESOURCE_VIDEOMIXER;

	ZeroMemory(m_PlaneConfigs, sizeof(m_PlaneConfigs));
	ZeroMemory(&m_Property, sizeof(m_Property));
	ZeroMemory(&m_rcMixingDstClip, sizeof(m_rcMixingDstClip));
	ZeroMemory(&m_rcMixingDst, sizeof(m_rcMixingDst));
	ZeroMemory(&m_MixerCaps, sizeof(MixerCaps));
	m_MixerCaps.uMaxPlaneNumber = PLANE_MAX;
	// MIXER_CAP_CAN_CHANGE_DESTINATION does not make sense for video mixer because of security concern.
	m_MixerCaps.dwFlags = MIXER_CAP_3D_RENDERTARGET | MIXER_CAP_CAN_VIRTUALIZE_FROM_ORIGIN;

    m_pVideoEffect3DBlt = 0;
    m_pTexturePool = 0;
    m_pModel = 0;
    m_pDriverExtension = 0;

    IUnknown *pUnknown = 0;
	CResourceManager::GetInstance()->GetInterface(DISPSVR_RESOURCE_TEXTUREPOOL, __uuidof(IUnknown), (void **)&pUnknown);
    m_pTexturePool = (CD3D9TexturePool *)pUnknown;
	CResourceManager::GetInstance()->GetInterface(DISPSVR_RESOURCE_VIDEOMIXERMODEL, __uuidof(IUnknown), (void **)&pUnknown);
    m_pModel = (CD3D9VideoMixerModel *)pUnknown;
	CResourceManager::GetInstance()->GetInterface(DISPSVR_RESOURCE_VIDEOEFFECT3DMANAGER, __uuidof(ID3D9VideoEffect3DProcessor), (void **)&m_pVideoEffect3DBlt);
	CResourceManager::GetInstance()->GetInterface(DISPSVR_RESOURCE_DRIVEREXTENSION, __uuidof(IDispSvrDriverExtension), (void **)&m_pDriverExtension);

    m_uClearNonVideoAreaListCount = 0;
	m_fWindowAspectRatio = 1.0f;
	m_bRecalculateVideoPosition = true;
    m_bClearFullRenderTarget = false;

    m_pXVEManager = NULL;
	CheckXVEManager();
}

CD3D9VideoMixerBase::~CD3D9VideoMixerBase()
{
	_ReleaseDevice();
    SAFE_RELEASE(m_pTexturePool);
    SAFE_RELEASE(m_pModel);
    SAFE_RELEASE(m_pDriverExtension);
	SAFE_RELEASE(m_pVideoEffect3DBlt);

    if (m_pXVEManager)
    {
         // Unregister the video mixer from XVEMGR
        m_pXVEManager->UnAdviseEventNotify(this);
        m_pXVEManager->Release();
        m_pXVEManager = NULL;
    }
}

STDMETHODIMP CD3D9VideoMixerBase::QueryInterface(REFIID riid, void **ppv)
{
	if (riid == __uuidof(IDispSvrVideoMixer))
	{
		return GetInterface((IDispSvrVideoMixer *)this, ppv);
	}
	else if (riid == __uuidof(IDispSvrVideoProcessor))
	{
		return GetInterface((IDispSvrVideoProcessor *) this, ppv);
	}
	else if (riid == __uuidof(IDispSvrVideoEffect3DManager))
	{
		// delegate to a global effect manager.
		return CResourceManager::GetInstance()->GetInterface(DISPSVR_RESOURCE_VIDEOEFFECT3DMANAGER, riid, ppv);
	}
    else if (riid == __uuidof(IDispSvrVideoEffectManager))
    {
        return GetInterface((IDispSvrVideoEffectManager *) this, ppv);
    }
    else if (riid == __uuidof(IXVEManagerNotify))
    {
        return GetInterface((IXVEManagerNotify *) this, ppv);
    }
    else if (riid == __uuidof(IDispSvrVideoPlaneCallback))
    {
        return GetInterface((IDispSvrVideoPlaneCallback *) this, ppv);
    }
	return CD3D9PluginBase::QueryInterface(riid, ppv);
}

STDMETHODIMP CD3D9VideoMixerBase::ProcessMessage(RESOURCE_MESSAGE_TYPE eMessage, LPVOID ulParam)
{
	HRESULT hr = E_NOTIMPL;

	switch (eMessage)
	{
	case RESOURCE_MESSAGE_SETDEVICE:
	case RESOURCE_MESSAGE_RELEASEDEVICE:
	case RESOURCE_MESSAGE_SETWINDOWHANDLE:
	case RESOURCE_MESSAGE_MOVEWINDOW:
		{
			//CAutoLock selfLock(&m_csObj);
			OnVideoPositionChange();
		}
		hr = CD3D9PluginBase::ProcessMessage(eMessage, ulParam);
		break;

	case RESOURCE_MESSAGE_EVICTRESOURCES:
		{
			BOOL bEvictResource = *(reinterpret_cast<BOOL *> (ulParam));
			CAutoLock selfLock(&m_csObj);
			if (bEvictResource)
				hr = m_pTexturePool->EvictResources();
			else
				hr = m_pTexturePool->RestoreResources();
		}
		break;
	case RESOURCE_MESSAGE_RESETDEVICE:
		{
			hr = _ResetDevice();
		}
		break;
	default:
		hr = CD3D9PluginBase::ProcessMessage(eMessage, ulParam);
		break;
	}
	return hr;
}

STDMETHODIMP CD3D9VideoMixerBase::_SetDevice(IUnknown *pDevice)
{
	HRESULT hr;

	CAutoLock selfLock(&m_csObj);
	hr = CD3D9PluginBase::_SetDevice(pDevice);
	if (FAILED(hr))
		return hr;

	ZeroMemory(m_PlaneConfigs, sizeof(m_PlaneConfigs));

	hr = m_pTexturePool->SetPoolUsage(TEXTURE_POOL_USAGE_D3D9);

	VideoProcessorStub vp = {0};
	vp.guidVP = DispSvr_VideoProcStretchRect;
	vp.RenderTargetFormat = D3DFMT_X8R8G8B8;
	vp.sCaps.eType = PROCESSOR_TYPE_HARDWARE;
	vp.pfnVBltFactory = CD3D9VBlt::Create;
	m_VideoProcessorList.push_back(vp);
	SelectVideoProcessor();

	SetRect(&m_rcMixingDstClip, 0, 0, GetRegistry(REG_BACKBUFFER_WIDTH, 0), GetRegistry(REG_BACKBUFFER_HEIGHT, 0));

	CheckXVEManager();	//Re-check video effect manager.
	return hr;
}

STDMETHODIMP CD3D9VideoMixerBase::_ResetDevice()
{
	SetRect(&m_rcMixingDstClip, 0, 0, GetRegistry(REG_BACKBUFFER_WIDTH, 0), GetRegistry(REG_BACKBUFFER_HEIGHT, 0));

	return S_OK;
}

STDMETHODIMP CD3D9VideoMixerBase::_ReleaseDevice()
{
	m_uClearNonVideoAreaListCount = 0;
	m_VideoProcessorList.clear();
	return CD3D9PluginBase::_ReleaseDevice();
}

STDMETHODIMP CD3D9VideoMixerBase::SetDestination(IUnknown *pDestSurface, const NORMALIZEDRECT *pDestRect)
{
	CAutoLock selfLock(m_pModel);
    const NORMALIZEDRECT &nrcCurrentDst = m_pModel->GetDestination();
	NORMALIZEDRECT nrcDst;
	if (pDestRect)
		nrcDst = *pDestRect;
	else
		LoadDefaultMixerCoordinate(nrcDst);

	if (nrcDst != nrcCurrentDst)
	{
        m_pModel->SetDestination(nrcDst);
		// some application my need to know the destination rectangle or surface change
		// in order to use the mixer correctly.
		NotifyListeners(EVENT_VIDEO_MIXING_CHANGE_DESTINATION, 0, reinterpret_cast<DWORD> (&nrcDst));
    }

    OnVideoPositionChange();
	return S_OK;
}

STDMETHODIMP CD3D9VideoMixerBase::Execute()
{
	HRESULT hr = S_OK;

	// wait until all surfaces are ready to be used (no one locks any mixer surfaces).
    m_pModel->WriterLock();

    {
        CAutoLock lockModel(m_pModel);
        const D3D9Plane &planeMainVideo = m_pModel->GetPlane(PLANE_MAINVIDEO);
        VideoTimeStamp TimeStamp = {0};
        if (planeMainVideo.IsValid())
        {
            TimeStamp.rtStart = planeMainVideo.VideoProperty.rtStart;
            TimeStamp.rtEnd = planeMainVideo.VideoProperty.rtEnd;
        }

	    NotifyListeners(EVENT_VIDEO_MIXING_BEGIN, 0, reinterpret_cast<DWORD> (&TimeStamp));
	    {
		    CAutoLock lock(&m_csObj);

		    if (m_bRecalculateVideoPosition)
			    hr = _OnMoveWindow();

		    if (SUCCEEDED(hr))
		    {
				if (MIXER_CAP_3D_RENDERTARGET & m_MixerCaps.dwFlags)
				{
					ClearNonVideoArea();
				}

				if (m_Property.dwFlags & MIXER_PROPERTY_SWAP_CHAIN)
				{
					CComPtr<IDirect3DSurface9> pBackBuffer;
					// Still pass down the current render target surface even if some mixer does not use it.
					hr = m_pDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer);
					hr = _Execute(pBackBuffer, m_rcMixingDst, m_rcMixingDstClip);			    
				}
				else //if (m_Property.dwFlags & MIXER_PROPERTY_RENDER_TARGET)
				{
					CComPtr<IDirect3DSurface9> pDestSurface;
					hr = m_pDevice->GetRenderTarget(0, &pDestSurface);
					hr = _Execute(pDestSurface, m_rcMixingDst, m_rcMixingDstClip);
				}
		    }
	    }
	    NotifyListeners(EVENT_VIDEO_MIXING_END, 0, 0);
    }

    m_pModel->WriterUnlock();

	return hr;
}

STDMETHODIMP CD3D9VideoMixerBase::QueryCaps(MixerCaps *pCaps)
{
	CHECK_POINTER(pCaps);
	memcpy(pCaps, &m_MixerCaps, sizeof(MixerCaps));
	return S_OK;
}

const D3D9PlaneConfig &CD3D9VideoMixerBase::GetPlaneConfig(PLANE_ID PlaneID)
{
    return m_PlaneConfigs[PlaneID];
}

STDMETHODIMP CD3D9VideoMixerBase::CreatePlane(PlaneInit *pInit, REFIID riid, void **ppPlane)
{
    CHECK_POINTER(pInit);
    CHECK_POINTER(ppPlane);

    CD3D9VideoMixerPlane *pPlane = NULL;
	HRESULT hr = CD3D9VideoMixerPlane::Create(*pInit, &pPlane);
	if (FAILED(hr))
		return hr;

	hr = pPlane->QueryInterface(riid, ppPlane);
	return hr;
}

STDMETHODIMP CD3D9VideoMixerBase::OnCreatePlane(const DispSvr::PlaneInit &pInit)
{
    return S_OK;
}

STDMETHODIMP CD3D9VideoMixerBase::SetClearRectangles(UINT uCount, ClearRect *pRects)
{
	CAutoLock selfLock(m_pModel);
    return m_pModel->SetClearRectangles(uCount, pRects);
}

STDMETHODIMP CD3D9VideoMixerBase::SetBackgroundColor(COLORREF Color)
{
	CAutoLock selfLock(m_pModel);
	return m_pModel->SetBackgroundColor(Color);
}

STDMETHODIMP CD3D9VideoMixerBase::GetBackgroundColor(COLORREF *pColor)
{
	CAutoLock selfLock(m_pModel);
    return m_pModel->GetBackgroundColor(pColor);
}

STDMETHODIMP CD3D9VideoMixerBase::SetLumaKey(const LumaKey *pLumaKey)
{
	CAutoLock selfLock(m_pModel);
	return m_pModel->SetLumaKey(pLumaKey);
}

STDMETHODIMP CD3D9VideoMixerBase::GetLumaKey(LumaKey *pLumaKey)
{
	CAutoLock selfLock(m_pModel);
    return m_pModel->GetLumaKey(pLumaKey);
}

STDMETHODIMP CD3D9VideoMixerBase::SetProperty(const MixerProperty *pProperty)
{
	CHECK_POINTER(pProperty);
	if ((MIXER_PROPERTY_VIRTUALIZE_FROM_ORIGIN & pProperty->dwFlags) != 0
		&& (MIXER_CAP_CAN_VIRTUALIZE_FROM_ORIGIN & m_MixerCaps.dwFlags) == 0)
		return E_INVALIDARG;
	memcpy(&m_Property, pProperty, sizeof(MixerProperty));
	return S_OK;
}

STDMETHODIMP CD3D9VideoMixerBase::GetProperty(MixerProperty *pProperty)
{
	CHECK_POINTER(pProperty);
	memcpy(pProperty, &m_Property, sizeof(MixerProperty));
	return S_OK;
}

STDMETHODIMP CD3D9VideoMixerBase::GetVideoEffectManager(IUnknown **ppManager)
{
    HRESULT hr = E_FAIL;
    CComPtr<IDispSvrXVEManagerWrapper> pXVEManagerWrapper;
    hr = CResourceManager::GetInstance()->GetInterface(DISPSVR_RESOURCE_XVIDEOEFFECTMANAGER,
        __uuidof(IDispSvrXVEManagerWrapper), (VOID **)&pXVEManagerWrapper);

    if (SUCCEEDED(hr) && pXVEManagerWrapper)
    {
        CComPtr<IXVideoEffectManager> pManager;
        hr = pXVEManagerWrapper->GetXVEManager(&pManager);
        if (SUCCEEDED(hr) && pManager)
        {
            hr = pManager->QueryInterface(IID_IUnknown, (VOID **)ppManager);
        }
    }
    return hr;
}

STDMETHODIMP CD3D9VideoMixerBase::SetVideoEffectManager(IUnknown *pManager)
{
	CHECK_POINTER(pManager);

	HRESULT hr = E_INVALIDARG;

	CComQIPtr<IXVideoEffectManager> pXVEManager = pManager;
	if (pXVEManager)
	{
		CComPtr<IDispSvrXVEManagerWrapper> pIXVEManagerWrapper;
		hr = CResourceManager::GetInstance()->GetInterface(DISPSVR_RESOURCE_XVIDEOEFFECTMANAGER,
			__uuidof(IDispSvrXVEManagerWrapper), (VOID **)&pIXVEManagerWrapper);

		if (SUCCEEDED(hr) && pIXVEManagerWrapper)
		{
			hr = pIXVEManagerWrapper->SetXVEManager(pXVEManager);
			if (SUCCEEDED(hr))
				hr = CheckXVEManager();
		}
    }

	return hr;
}

STDMETHODIMP CD3D9VideoMixerBase::QueryPlaneFormatCount(PLANE_ID PlaneID, UINT *pCount)
{
	CHECK_POINTER(pCount);
	if (m_pTexturePool)
		return m_pTexturePool->QuerySupportedFormatCount(pCount);
	return E_FAIL;
}

STDMETHODIMP CD3D9VideoMixerBase::QueryPlaneFormat(PLANE_ID PlaneID, PLANE_FORMAT *pFormats)
{
	CHECK_POINTER(pFormats);
	if (m_pTexturePool)
		return m_pTexturePool->QuerySupportedFormats(pFormats);
	return E_FAIL;
}

STDMETHODIMP CD3D9VideoMixerBase::QueryPlaneCaps(PLANE_ID PlaneID, PLANE_FORMAT Format, PlaneCaps *pCap)
{
	CHECK_POINTER(pCap);
	if (!m_pTexturePool)
		return E_FAIL;

	TextureCap cap;

	// First check if texturepool supports the format.
	HRESULT hr = m_pTexturePool->QueryFormatCaps(Format, &cap);
	if (FAILED(hr))
		return hr;

	ZeroMemory(pCap, sizeof(PlaneCaps));
	pCap->dwFlags = PLANE_CAP_MAX_SIZE;
	pCap->uMaxWidth = cap.uMaxWidth;
	pCap->uMaxHeight = cap.uMaxHeight;

	// Then check if mixer supports the format.
	return _QueryPlaneCaps(PlaneID, Format, pCap);
}

void CD3D9VideoMixerBase::OnDestroyPlane(PLANE_ID PlaneID)
{
	CAutoLock selfLock(&m_csObj);

    if (m_pDriverExtension)
        m_pDriverExtension->Clear();
}

STDMETHODIMP CD3D9VideoMixerBase::_QueryPlaneCaps(PLANE_ID PlaneID, PLANE_FORMAT Format, PlaneCaps *pCap)
{
	return S_OK;
}

STDMETHODIMP CD3D9VideoMixerBase::_OnMoveWindow()
{
	HRESULT hr;

	// clear and then calculate non video area list
	m_uClearNonVideoAreaListCount = 0;

	hr = GetClientRect(m_hwnd, &m_rcMixingDst);
	if (hr == 0)
		return E_FAIL;

    const NORMALIZEDRECT &nrcCurrentDst = m_pModel->GetDestination();
	// window aspect ratio
	m_fWindowAspectRatio = float(m_rcMixingDst.right) * (nrcCurrentDst.right - nrcCurrentDst.left) / (m_rcMixingDst.bottom * (nrcCurrentDst.bottom - nrcCurrentDst.top));

	// when using virtualization, render target is supposed to be the same as desktop resolution.
	if (m_Property.dwFlags & MIXER_PROPERTY_VIRTUALIZATION)
	{
		POINT pt = {0};
        LONG lMonitorLeft = GetRegistry(REG_DISPLAY_X, 0);
        LONG lMonitorTop = GetRegistry(REG_DISPLAY_Y, 0);

        hr = ClientToScreen(m_hwnd, &pt);	ASSERT(hr != 0);
		if (m_Property.dwFlags & MIXER_PROPERTY_VIRTUALIZE_FROM_ORIGIN)
		{
			if (pt.x > lMonitorLeft)
				pt.x = lMonitorLeft;
			if (pt.y > lMonitorTop)
				pt.y = lMonitorTop;
		}
		OffsetRect(&m_rcMixingDst, pt.x - lMonitorLeft, pt.y - lMonitorTop);
	}
	else
	{
		CopyRect(&m_rcMixingDst, &m_rcMixingDstClip);
	}

	// get window rectangle
	RECT rcWndRect = m_rcMixingDst;

	// now we have m_rcMixingDst as video mixer's mixing area.
	NRectToRect(m_rcMixingDst, nrcCurrentDst);

	// We are done calculation for mixing. (m_rcMixingDst and m_rcMixingDstClip are valid)
	m_bRecalculateVideoPosition = false;
    m_bClearFullRenderTarget = false;

	// check mixing area is valid
	if (IsRectEmpty(&m_rcMixingDst))
		return E_FAIL;

	if ((MIXER_PROPERTY_CLEARNONVIDEOAREA & m_Property.dwFlags) == 0
		|| (MIXER_CAP_3D_RENDERTARGET & m_MixerCaps.dwFlags) == 0)
		return S_OK;

	// we will try to calculate video area if relying on IDirect3DDevice9->Clear().
    const D3D9Plane &mv = m_pModel->GetPlane(PLANE_MAINVIDEO);
    const D3D9Plane &sv = m_pModel->GetPlane(PLANE_SUBVIDEO);
    PLANE_ID PlaneID;
    if (mv.IsValid())
    {
        // case1: clear non main-video area if main-video is visible. EX: Bug#80247: main-video area < destination area.
        PlaneID = PLANE_MAINVIDEO;
    }
    else if (sv.IsValid())
    {
        // case2: clear non sub-video area if sub-video is visible and main-video is not. EX: BD logo STD500N title#16: only sub-video case.
        PlaneID = PLANE_SUBVIDEO;
    }
    else
    {
        // case3: clear full size of RT if main-video and sub-video are not both visible.
        m_uClearNonVideoAreaListCount = 0;
        m_bClearFullRenderTarget = true;
        return S_OK;
    }

	const D3D9Plane &plane = m_pModel->GetPlane(PlaneID);
	RECT rcSrc = plane.rcSrc;
	RECT rcVideoDst = m_rcMixingDst;

	NRectToRect(rcVideoDst, plane.nrcDst);

	hr = PlaneToScreen(plane, rcSrc, rcVideoDst, m_rcMixingDstClip);
	if (SUCCEEDED(hr))
	{
		D3DRECT *pArea = m_ClearNonVideoAreaList;
		if (rcVideoDst.top > rcWndRect.top)
		{
			pArea->x1 = rcWndRect.left;
			pArea->y1 = rcWndRect.top;
			pArea->x2 = rcWndRect.right;
			pArea->y2 = rcVideoDst.top;
			pArea++;
		}

		if (rcVideoDst.bottom < rcWndRect.bottom)
		{
			pArea->x1 = rcWndRect.left;
			pArea->y1 = rcVideoDst.bottom;
			pArea->x2 = rcWndRect.right;
			pArea->y2 = rcWndRect.bottom;
			pArea++;
		}

		if (rcVideoDst.left > rcWndRect.left)
		{
			pArea->x1 = rcWndRect.left;
			pArea->y1 = rcWndRect.top;
			pArea->x2 = rcVideoDst.left;
			pArea->y2 = rcWndRect.bottom;
			pArea++;
		}

		if (rcVideoDst.right < rcWndRect.right)
		{
			pArea->x1 = (rcVideoDst.right-1);
			pArea->y1 = rcWndRect.top;
			pArea->x2 = rcWndRect.right;
			pArea->y2 = rcWndRect.bottom;
			pArea++;
		}
		ASSERT(pArea - m_ClearNonVideoAreaList <= 4 && pArea - m_ClearNonVideoAreaList >= 0);
		m_uClearNonVideoAreaListCount = pArea - m_ClearNonVideoAreaList;
	}
    else
    {
        m_uClearNonVideoAreaListCount = 0;
        m_bClearFullRenderTarget = true;
    }

	return S_OK;
}

HRESULT CD3D9VideoMixerBase::NotifyListeners(DispSvr::EVENT_VIDEO_MIXING event, DWORD dwParam1, DWORD dwParam2)
{
	HRESULT hr = S_OK;

	for (int i = 0; i < PLANE_MAX; i++)
	{
        D3D9Plane &plane = m_pModel->GetPlane(i);
		if (plane.bCreated && plane.pListener)
		{
			hr |= plane.pListener->Notify(event, dwParam1, dwParam2);
		}
	}
	return hr;
}

// check if main video exists and covers the whole background visible area.
bool CD3D9VideoMixerBase::IsBackgroundVisible() const
{
	const D3D9Plane &mv = m_pModel->GetPlane(PLANE_MAINVIDEO);
	const D3D9Plane &bk = m_pModel->GetPlane(PLANE_BACKGROUND);

	if (!bk.IsValid())
		return false;

	// test if main video covers whole background
	if (mv.IsValid())
	{
		return mv.nrcDst.left < bk.nrcDst.left
			|| mv.nrcDst.right > bk.nrcDst.right
			|| mv.nrcDst.top > bk.nrcDst.top
			|| mv.nrcDst.bottom < bk.nrcDst.bottom;
	}

	return true;
}

void CD3D9VideoMixerBase::ClearNonVideoArea()
{
    if (m_uClearNonVideoAreaListCount==0 && m_bClearFullRenderTarget==false)
        return;

    DWORD dwValue = 0;
    HRESULT hr = m_pDevice->GetRenderState(D3DRS_STENCILENABLE, &dwValue);
    dwValue = dwValue ? D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL : D3DCLEAR_TARGET;
	if (m_uClearNonVideoAreaListCount > 0)
	{
		hr = m_pDevice->Clear(m_uClearNonVideoAreaListCount,
				m_ClearNonVideoAreaList,
				dwValue,
				D3DCOLOR_XRGB(0, 0, 0),
				1.0f,
				0L);
	}
    else if (m_bClearFullRenderTarget)
    {
        hr = m_pDevice->Clear(  0L,	// no rects (clear all)
            NULL,					// clear entire view port
            dwValue,		        // clear render target
            D3DCOLOR_XRGB(0,0,0),
            1.0f,					// z buffer depth
            0L);					// no stencil
    }
}

/// Switch current video processor to the new one and update the plane's properties.
static inline void SetVideoProcessor(D3D9PlaneConfig &cfg, D3D9Plane &plane, VideoProcessorStub *pStub, CD3D9TexturePool *pPool)
{
	ASSERT(pStub);
	if (cfg.pVideoProcessorStub == pStub)
		return;

	cfg.pVideoProcessorStub = pStub;
	// if plane.pVBlt already exists and the plane is in use,
	// change the current video processor to the updated one when necessary.
	if (plane.bCreated)
	{
		SAFE_DELETE(plane.pVBlt);
		plane.pVBlt = CreateVBltFromVPStub(&plane, pStub, pPool);
	}
}

// Binary predicate for sorting, return true if a > b.
bool VidProcSelectingPredicate(const VideoProcessorStub &a, const VideoProcessorStub &b)
{
	if (a.sCaps.eType == b.sCaps.eType)
	{
		// make dedicated 3rd party mixer the preferred mixer.
		bool bIsAPreferred = a.guidVP == DispSvr_VideoProcDxvaHD || a.guidVP == DispSvr_VideoProcFastComp || a.guidVP == DispSvr_VideoProcPCOM;
		bool bIsBPreferred = b.guidVP == DispSvr_VideoProcDxvaHD || b.guidVP == DispSvr_VideoProcFastComp || b.guidVP == DispSvr_VideoProcPCOM;
 		if (bIsAPreferred ^ bIsBPreferred)
 			return bIsAPreferred;

		if (a.sCaps.uProcessorCaps == b.sCaps.uProcessorCaps)
		{
			// return false to keep original VP sequence if these two VP has same caps.
			if (a.sCaps.uFilterCaps == b.sCaps.uFilterCaps)
				return false;
			return a.sCaps.uFilterCaps > b.sCaps.uFilterCaps;
		}
		return a.sCaps.uProcessorCaps > b.sCaps.uProcessorCaps;
	}
	// PROCESSOR_TYPE_HARDWARE = 0, PROCESSOR_TYPE_SOFTWARE = 1
	return a.sCaps.eType > b.sCaps.eType;
}

void CD3D9VideoMixerBase::SelectVideoProcessor(VideoProcessorStub *pMainVP, VideoProcessorStub *pSubVP)
{
	if (m_VideoProcessorList.empty())
		return;

	// sort the list by VP capabilities, the best is at the beginning.
	m_VideoProcessorList.sort(VidProcSelectingPredicate);

	if (pMainVP == NULL)
		pMainVP = &m_VideoProcessorList.front();

	// Sub VP may not be used depending on mixer implementation.
	// Perhaps we should select simpler VP instead of the best available.
	if (pSubVP == NULL)
		pSubVP = &m_VideoProcessorList.front();

	SetVideoProcessor(m_PlaneConfigs[PLANE_MAINVIDEO], m_pModel->GetPlane(PLANE_MAINVIDEO), pMainVP, m_pTexturePool);
	SetVideoProcessor(m_PlaneConfigs[PLANE_SUBVIDEO], m_pModel->GetPlane(PLANE_SUBVIDEO), pSubVP, m_pTexturePool);
	SetVideoProcessor(m_PlaneConfigs[PLANE_GRAPHICS], m_pModel->GetPlane(PLANE_GRAPHICS), pMainVP, m_pTexturePool);
}

HRESULT CD3D9VideoMixerBase::GenerateDxva2VPList()
{
	UINT uCount = 0;
	DXVA2VP_Caps *pCaps = 0;
	HRESULT hr;

	hr = CDXVA2VideoProcessor::GetDXVA2VPCaps(m_pDevice, &uCount, &pCaps);
	if (SUCCEEDED(hr))
	{
		// DXVA2 VP must support at least progressive and bob devices.
		ASSERT(uCount >= 2 && pCaps);

		VideoProcessorStub vp;
		for (UINT i = 0; i < uCount; i++)
		{
			ZeroMemory(&vp, sizeof(vp));
			vp.guidVP = pCaps[i].guidVP;
			vp.RenderTargetFormat = pCaps[i].RenderTargetFormat;
			vp.sCaps = pCaps[i].sCaps;
			memcpy(vp.FilterRanges, pCaps[i].FilterRanges, sizeof(vp.FilterRanges));
			for (int j = 0; j < sizeof(vp.FilterRanges) / sizeof(vp.FilterRanges[0]); j++)
			{
				vp.fFilterValue[j] = vp.FilterRanges[j].fDefaultValue;
			}
			vp.pfnVBltFactory = CDXVA2VBlt::Create;
			m_VideoProcessorList.push_back(vp);
		}
		delete [] pCaps;	// caller should release pCaps returned by GetDXVA2VPCaps()
	}
	m_VideoProcessorList.sort(VidProcSelectingPredicate);
	return hr;
}

HRESULT CD3D9VideoMixerBase::PlaneToScreen(const D3D9Plane &plane, RECT &rcSrc, RECT &rcDst, const RECT &rcDstClip)
{
	NORMALIZEDRECT nrcOutput = plane.nrcDst;

    //Each plane should refer to the their own mixing area's(viewport) aspect ratio, not whole mixing area(Window)
    // mixing area's aspect ratio = (window(whole mixing area) width * viewport's normalized width) /
    //                                             (window(whole mixing area) height * viewport's normalized height)
    float fMixingAreaAspectRatio = m_fWindowAspectRatio * (nrcOutput.right - nrcOutput.left) / (nrcOutput.bottom - nrcOutput.top);
	CorrectAspectRatio(nrcOutput, plane.GetAspectRatio() / fMixingAreaAspectRatio);
	// cropping applies to both source and destination.
	CropRect(nrcOutput, plane.nrcCrop);
	CropRect(rcSrc, plane.nrcCrop);

	// map normailzed destination rectangle to actual rectangle in pixel.
	NRectToRect(rcDst, nrcOutput);

	// check and clip source/destination rectangles when rcDst is outside of rcDstClip.
	ClipRect(rcDst, rcSrc, rcDstClip);

	// fail if dst is not viewable.
	return IsRectEmpty(&rcDst) || IsRectEmpty(&rcSrc) ? E_FAIL : S_OK;
}

HRESULT CD3D9VideoMixerBase::StereoPlaneToScreen(UINT uViewID, const D3D9Plane &plane, RECT &rcSrc, RECT &rcDst, const RECT &rcDstClip)
{
	NORMALIZEDRECT nrcOutput = plane.nrcDst;

    //Each plane should refer to the their own mixing area's(viewport) aspect ratio, not whole mixing area(Window)
    // mixing area's aspect ratio = (window(whole mixing area) width * viewport's normalized width) /
    //                                             (window(whole mixing area) height * viewport's normalized height)
    float fMixingAreaAspectRatio = m_fWindowAspectRatio * (nrcOutput.right - nrcOutput.left) / (nrcOutput.bottom - nrcOutput.top);
	CorrectAspectRatio(nrcOutput, plane.GetAspectRatio() / fMixingAreaAspectRatio);
	// cropping applies to both source and destination.
	CropRect(nrcOutput, plane.nrcCrop);
	CropRect(rcSrc, plane.nrcCrop);

	INT iOffset = GetStereoOffset(plane);
	if (iOffset != 0 && plane.uWidth != 0)
	{
		float fNormalizedOffset = float(iOffset) / plane.uWidth;
		if (uViewID == 0)	// base view shifts right if offset is positive.
		{
			nrcOutput.left += fNormalizedOffset;
			nrcOutput.right += fNormalizedOffset;
		}
		else if (uViewID == 1)	// dep. view shifts left if offset is possitive.
		{
			nrcOutput.left -= fNormalizedOffset;
			nrcOutput.right -= fNormalizedOffset;
		}
	}

	// map normailzed destination rectangle to actual rectangle in pixel.
	NRectToRect(rcDst, nrcOutput);

	// check and clip source/destination rectangles when rcDst is outside of rcDstClip.
	ClipRect(rcDst, rcSrc, rcDstClip);

	// fail if dst is not viewable.
	return IsRectEmpty(&rcDst) || IsRectEmpty(&rcSrc) ? E_FAIL : S_OK;
}

STDMETHODIMP CD3D9VideoMixerBase::GetAvailableVideoProcessorModeCount(UINT *pCount)
{
	CHECK_POINTER(pCount);
	CAutoLock selfLock(&m_csObj);
	*pCount = m_VideoProcessorList.size();
	return S_OK;
}

STDMETHODIMP CD3D9VideoMixerBase::GetAvailableVideoProcessorModes(GUID *pGUID)
{
	CHECK_POINTER(pGUID);
	CAutoLock selfLock(&m_csObj);
	for (VideoProcessorList::const_iterator i = m_VideoProcessorList.begin();
		i != m_VideoProcessorList.end();
		++i)
	{
		*pGUID++ = i->guidVP;
	}
	return S_OK;
}

STDMETHODIMP CD3D9VideoMixerBase::GetVideoProcessorCaps(LPCGUID lpGUID, VideoProcessorCaps *pCaps)
{
	CHECK_POINTER(lpGUID);
	CAutoLock selfLock(&m_csObj);

	VideoProcessorList::const_iterator i = m_VideoProcessorList.begin();
	for (; i != m_VideoProcessorList.end(); ++i)
	{
		if (i->guidVP == *lpGUID)
			break;
	}
	if (i == m_VideoProcessorList.end())
		return E_FAIL;
	memcpy(pCaps, &i->sCaps, sizeof(VideoProcessorCaps));
	return S_OK;
}

STDMETHODIMP CD3D9VideoMixerBase::GetVideoProcessorMode(LPGUID lpGUID)
{
	CHECK_POINTER(lpGUID);
	CAutoLock selfLock(&m_csObj);
	const VideoProcessorStub *pVP = m_PlaneConfigs[PLANE_MAINVIDEO].pVideoProcessorStub;

	if (pVP)
	{
		*lpGUID = pVP->guidVP;
		return S_OK;
	}
	return E_FAIL;
}

STDMETHODIMP CD3D9VideoMixerBase::SetVideoProcessorMode(LPCGUID lpGUID)
{
	CHECK_POINTER(lpGUID);
	CAutoLock selfLock(&m_csObj);
	const VideoProcessorStub *pVP = m_PlaneConfigs[PLANE_MAINVIDEO].pVideoProcessorStub;

	if (pVP && IsEqualGUID(pVP->guidVP, *lpGUID))
		return S_FALSE;

	VideoProcessorList::iterator i = m_VideoProcessorList.begin();
	for (; i != m_VideoProcessorList.end(); ++i)
		if (i->guidVP == *lpGUID)
			break;

	if (i == m_VideoProcessorList.end())
		return E_INVALIDARG;

	SelectVideoProcessor(&*i);
	return S_OK;
}

STDMETHODIMP CD3D9VideoMixerBase::GetFilterValueRange(VIDEO_FILTER eFilter, ValueRange *pValueRange)
{
	CHECK_POINTER(pValueRange);
	CAutoLock selfLock(&m_csObj);
	const VideoProcessorStub *pVP = m_PlaneConfigs[PLANE_MAINVIDEO].pVideoProcessorStub;

	if (pVP)
	{
		if ((1 << eFilter) & pVP->sCaps.uFilterCaps)
		{
			memcpy(pValueRange, &pVP->FilterRanges[eFilter], sizeof(ValueRange));
			return S_OK;
		}
		return E_INVALIDARG;
	}
	return E_FAIL;
}

STDMETHODIMP CD3D9VideoMixerBase::SetFilterValue(VIDEO_FILTER eFilter, float fValue)
{
	CAutoLock selfLock(&m_csObj);
	const VideoProcessorStub *pVP = m_PlaneConfigs[PLANE_MAINVIDEO].pVideoProcessorStub;

	if (pVP)
	{
		if ((1 << eFilter) & pVP->sCaps.uFilterCaps)
		{
			if (pVP->FilterRanges[eFilter].fMinValue <= fValue
				&& pVP->FilterRanges[eFilter].fMaxValue >= fValue)
			{
				m_PlaneConfigs[PLANE_MAINVIDEO].pVideoProcessorStub->fFilterValue[eFilter] = fValue;
				return S_OK;
			}
		}
		return E_INVALIDARG;
	}
	return E_FAIL;
}

STDMETHODIMP CD3D9VideoMixerBase::GetFilterValue(VIDEO_FILTER eFilter, float *pfValue)
{
	CHECK_POINTER(pfValue);
	CAutoLock selfLock(&m_csObj);
	const VideoProcessorStub *pVP = m_PlaneConfigs[PLANE_MAINVIDEO].pVideoProcessorStub;

	if (pVP)
	{
		if ((1 << eFilter) & pVP->sCaps.uFilterCaps)
		{
			*pfValue = pVP->fFilterValue[eFilter];
			return S_OK;
		}
		return E_INVALIDARG;
	}
	return E_FAIL;
}

STDMETHODIMP CD3D9VideoMixerBase::GetMappedFilterValue(VIDEO_FILTER eFilter, float *pfValue)
{
	CHECK_POINTER(pfValue);
	CAutoLock selfLock(&m_csObj);

	const VideoProcessorStub *pPlaneVP = m_PlaneConfigs[PLANE_MAINVIDEO].pVideoProcessorStub;
	VideoProcessorList::const_iterator itTextVP = m_VideoProcessorList.begin();
	if (pPlaneVP && m_VideoProcessorList.size() > 1)
	{   
		//Always reference the saturation range from "the 2nd item" of m_VideoProcessorList.
		itTextVP++;
		float fPlaneVal = pPlaneVP->fFilterValue[eFilter];
		float fPlaneMax = pPlaneVP->FilterRanges[eFilter].fMaxValue;
		float fPlaneMin = pPlaneVP->FilterRanges[eFilter].fMinValue; 

		float fTextMax = itTextVP->FilterRanges[eFilter].fMaxValue;
		float fTextMin = itTextVP->FilterRanges[eFilter].fMinValue;

        // For NIVIDA hue mapping workaround, its range is from 0 to 359 but default is 0
        if (eFilter == VIDEO_FILTER_HUE && pPlaneVP->FilterRanges[eFilter].fMaxValue == 359)
        {
            if (fPlaneVal <= (fPlaneMax+fPlaneMin)/2)
                *pfValue = (fTextMin+fTextMax)/2 + (float)((float)(fPlaneVal-fPlaneMin)/(float)(fPlaneMax-fPlaneMin) * (fTextMax-fTextMin));
            else
                *pfValue = (fTextMin+fTextMax)/2 + (float)((float)(fPlaneVal-fPlaneMax)/(float)(fPlaneMax-fPlaneMin) * (fTextMax-fTextMin));
        }
        else
            *pfValue = fTextMin + (float)((float)(fPlaneVal-fPlaneMin)/(float)(fPlaneMax-fPlaneMin) * (fTextMax-fTextMin));

		*pfValue = ((*pfValue==(float)1.00) ? (float)1.01 : *pfValue);  // for AMD's ARGB workaround. plus 0.01 to avoid 1.00 crash params.
		*pfValue = ((*pfValue>fTextMax) ? fTextMax : *pfValue);
		*pfValue = ((*pfValue<fTextMin) ? fTextMin : *pfValue);

		return S_OK;
	}
	else
		return E_INVALIDARG;
}

STDMETHODIMP CD3D9VideoMixerBase::OnNotify(DWORD dwStreamID, REFGUID rguidEffectID, XVE::XVE_EVENT_TYPE dwEvent, DWORD dwParams)
{
    HRESULT hr = S_FALSE;
    switch(dwEvent)
    {
    case XVE::XVE_EVENT_FRAMERATE_CONVERSION:
        if (dwStreamID == PLANE_MAINVIDEO)
        {
            BOOL bEnable = dwParams ? TRUE : FALSE;
    	    NotifyListeners(EVENT_VIDEO_MIXING_FRAMERATE_CONVERSION, bEnable, 0);
            break;
        }
    }
    return S_OK;
}

HRESULT CD3D9VideoMixerBase::CheckXVEManager()
{
	CComPtr<IDispSvrXVEManagerWrapper> pXVEManagerWrapper;
	HRESULT hr = CResourceManager::GetInstance()->GetInterface(DISPSVR_RESOURCE_XVIDEOEFFECTMANAGER, __uuidof(IDispSvrXVEManagerWrapper), (void **)&pXVEManagerWrapper);

    if (m_pXVEManager)
    {
         // Unregister the video mixer from XVEMGR
        m_pXVEManager->UnAdviseEventNotify(this);
        m_pXVEManager->Release();
        m_pXVEManager = NULL;
    }

	if (pXVEManagerWrapper)
	{
		hr = pXVEManagerWrapper->GetXVEManager(&m_pXVEManager);
        if (m_pXVEManager)
        {
             // Register the video mixer to receive events from XVEMGR.
            m_pXVEManager->AdviseEventNotify(this);
        }
	}
	return hr;
}

HRESULT CD3D9VideoMixerBase::ProcessVideoEffect(PLANE_ID PlaneID, bool *pbStereoDetected, RECT &rcSrc, const RECT &rcDst, PLANE_FORMAT IntermediateFormat, IUnknown **ppDestUnk, UINT uViewID)
{
    CHECK_POINTER(ppDestUnk);

    HRESULT hr = E_FAIL;

    // S_OK indicates video effect processing done.
    // S_FALSE indicates video effect is not enabled.
    if (!m_pXVEManager)
        return S_FALSE;

    BOOL bEnabled = FALSE;
    hr = m_pXVEManager->IsEnabled(PlaneID, &bEnabled);
    if (FAILED(hr) || !bEnabled)
        return S_FALSE;

    CComPtr<IDirect3DTexture9> pTexture;
    CComPtr<IDirect3DSurface9> pSurface;
    D3DSURFACE_DESC desc;
    D3D9Plane &plane = m_pModel->GetPlane(PlaneID);

    hr = m_pTexturePool->GetRepresentation(plane.GetViewTextureHandle(uViewID), IID_IDirect3DTexture9, (void **)&pTexture);
    hr = m_pTexturePool->GetRepresentation(plane.GetViewTextureHandle(uViewID), IID_IDirect3DSurface9, (void **)&pSurface);

    if (!pTexture && !pSurface)
        return E_FAIL;


    CComPtr<IXVESample> pInputSample;
    CComPtr<IXVESample> pOutputSample;
    CComPtr<IXVEType> pInputType;
    BOOL bRepeatFrame = plane.VideoProperty.bRepeatFrame;
    UINT uMultipleOutputType = 0;

    m_pXVEManager->CreateEmptyContainer(__uuidof(IXVESample), (VOID **)&pInputSample);
    m_pXVEManager->CreateEmptyContainer(__uuidof(IXVESample), (VOID **)&pOutputSample);

    if (pInputSample && pOutputSample)
    {
        hr = S_OK;
        UINT uIndex = 0;
        BOOL bSurface = FALSE;
        BOOL bTexture = FALSE;

        m_pXVEManager->LockStore();
        do 
        {
            pInputType = NULL;
            hr = m_pXVEManager->GetInputAvailableType(PlaneID, uIndex, &pInputType);
            if (SUCCEEDED(hr))
            {
                XVE::XVE_FORMAT_TYPE xveFormatType;
                if (FAILED(pInputType->GetFrameType(&xveFormatType)))
                    break;
                if (xveFormatType == XVE::XVE_FORMAT_UNKNOWN)
                {
                    bSurface = TRUE;
                    bTexture = TRUE;
                }
                else if (xveFormatType == XVE::XVE_FORMAT_TEXTURE)
                {
                    bTexture = TRUE;
                }
                else if (xveFormatType == XVE::XVE_FORMAT_SURFACE)
                {
                    bSurface = TRUE;
                }
            }
            uIndex++;

        } while (SUCCEEDED(hr));

        // Special case to report SxS effect detection status.
        if (pbStereoDetected)
        {
            *pbStereoDetected = false;
            CComPtr<IXVEType> pOutputCurrentType;
            hr = m_pXVEManager->GetOutputCurrentType(PlaneID, &pOutputCurrentType);
            if (SUCCEEDED(hr))
            {
                XVEGetTypeMultipleOutput(pOutputCurrentType, uMultipleOutputType);
            }
        }
        m_pXVEManager->UnLockStore();

        if (!bSurface && !bTexture) //video effect has to support at least one type.
            return E_FAIL;

        ZeroMemory(&desc, sizeof(D3DSURFACE_DESC));
        if (pSurface)
        {
            if (bSurface)
            {
                    if (plane.VideoProperty.Format != VIDEO_FORMAT_PROGRESSIVE) //change interlaced frame to progressive
                    {
                        CComPtr<IDirect3DSurface9> pProgressiveSurface;
                        hr = plane.pVBlt->IntermediateVBlt(pSurface, rcSrc, PLANE_FORMAT_YUY2/*IntermediateFormat*/, &pProgressiveSurface);
                        if (SUCCEEDED(hr) && pProgressiveSurface)
                        {
                            if (S_D3D9VBLT_RETURN_CACHE == hr)
                                bRepeatFrame = TRUE;
                            pSurface = pProgressiveSurface;
                            pProgressiveSurface = NULL;
                        }
                        else
                            return hr;
                    }

                    pSurface->GetDesc(&desc);
                    pInputSample->SetFrame(pSurface);
                    pInputSample->SetFrameType(XVE::XVE_FORMAT_SURFACE);
            }
            else // bTexture == TRUE
            {
                if ((plane.VideoProperty.Format != VIDEO_FORMAT_PROGRESSIVE) || !pTexture) // interlaced or texture not exist.
                {
					// Mapping filter values from DXVAHD to DXVA2
					float fFilterVal = (float)0.0;
					for (int i=0; i<DXVAHDVP_MAX_FILTER; i++)
					{	
						if (GetMappedFilterValue(static_cast<VIDEO_FILTER>(i), &fFilterVal) == S_OK)
							plane.pVBlt->SetFilterValue(static_cast<VIDEO_FILTER>(i), fFilterVal);
					}

                    CComPtr<IDirect3DTexture9> pProgressiveTexture;
                    hr = plane.pVBlt->IntermediateTextureVBlt(pSurface, rcSrc, &pProgressiveTexture);
                    if (S_D3D9VBLT_RETURN_CACHE == hr)
                        bRepeatFrame = TRUE;

                    pTexture = pProgressiveTexture;
                    pProgressiveTexture = NULL;
                    if (0)
                    {
                        hr = DXToFile(pProgressiveTexture);
                    }
                }
                pTexture->GetLevelDesc(0, &desc);
                pInputSample->SetFrame(pTexture);
                pInputSample->SetFrameType(XVE::XVE_FORMAT_TEXTURE);
            }
        }
        else /*pTexture*/
        {
            if (bTexture) //try same type first
            {
                if (plane.VideoProperty.Format != VIDEO_FORMAT_PROGRESSIVE) // interlaced
                {
                    pSurface = NULL;
                    pTexture->GetSurfaceLevel(0, &pSurface);

                    CComPtr<IDirect3DTexture9> pProgressiveTexture;
                    hr = plane.pVBlt->IntermediateTextureVBlt(pSurface, rcSrc, &pProgressiveTexture);
                    pTexture = pProgressiveTexture;
                    pProgressiveTexture = NULL;
                }
                pTexture->GetLevelDesc(0, &desc);
                pInputSample->SetFrame(pTexture);
                pInputSample->SetFrameType(XVE::XVE_FORMAT_TEXTURE);
            }
            else //bSurface == TRUE, need to convert texture to surface.
            {
                CComPtr<IDirect3DSurface9> pProgressiveSurface;

                pSurface = NULL;
                pTexture->GetSurfaceLevel(0, &pSurface);

                hr = plane.pVBlt->IntermediateVBlt(pSurface, rcSrc, PLANE_FORMAT_YUY2/*IntermediateFormat*/, &pProgressiveSurface);
                if (SUCCEEDED(hr) && pProgressiveSurface)
                {
                    pSurface = pProgressiveSurface;
                    pProgressiveSurface = NULL;
                }
                else
                {
                    return hr;
                }
                pSurface->GetDesc(&desc);
                pInputSample->SetFrame(pSurface);
                pInputSample->SetFrameType(XVE::XVE_FORMAT_SURFACE);
            }
        }

        XVESetSampleViewID(pInputSample, uViewID);
        XVESetSampleRepeatFrame(pInputSample, bRepeatFrame);
        XVESetSampleDisplayTimeStamp(pInputSample, plane.VideoProperty.rtDisplayTarget ? plane.VideoProperty.rtDisplayTarget : plane.VideoProperty.rtStart);
        XVESetSampleRestrictedContent(pInputSample, plane.VideoProperty.bRestrictedContent);

		pInputSample->SetFrameTimeStamp(plane.VideoProperty.rtStart);
		pInputSample->SetFrameDuration(plane.VideoProperty.rtEnd - plane.VideoProperty.rtStart);
		pInputSample->SetFieldSelectMode(plane.VideoProperty.dwFieldSelect == FIELD_SELECT_SECOND ? XVE::XVE_FIELDSELECT_SECOND : XVE::XVE_FIELDSELECT_FIRST);

        pInputSample->SetFrameFormat(desc.Format);
        pInputSample->SetDisplaySize((m_rcMixingDst.right - m_rcMixingDst.left), (m_rcMixingDst.bottom - m_rcMixingDst.top));
        pInputSample->SetFrameSize(rcSrc.right - rcSrc.left, rcSrc.bottom - rcSrc.top);
        hr = m_pXVEManager->ProcessEffect(PlaneID, pInputSample, pOutputSample);
        if (SUCCEEDED(hr))
        {
            UINT uWidth = 0, uHeight = 0;
            hr = pOutputSample->GetFrame(ppDestUnk);
            hr = pOutputSample->GetFrameSize(&uWidth, &uHeight);

            if (uMultipleOutputType & XVE::XVE_MULTIPLEOUTPUT_STEREO_CONVERSION)
            {
                *pbStereoDetected = ((rcSrc.right - rcSrc.left)/2 == uWidth);
            }
            rcSrc.left = 0;
            rcSrc.right = uWidth;
            rcSrc.top = 0;
            rcSrc.bottom = uHeight;
        }
    }
    return hr;
}

INT CD3D9VideoMixerBase::GetStereoOffset(const D3D9Plane &plane) const
{
    const D3D9Plane &VideoPlane = m_pModel->GetPlane(PLANE_MAINVIDEO);

    if (!plane.bCreated)
        return 0;

    if (plane.eStereoOffsetMode == OFFSET_MODE_FIXED_OFFSET)
    {
        return plane.lStereoDisplayOffset;
    }
    else if (plane.eStereoOffsetMode == OFFSET_MODE_SEQUENCE_ID)
    {
        if (!VideoPlane.bCreated || !VideoPlane.pStereoMetadata || (VideoPlane.uStereoMetadataLength <= (UINT)plane.lStereoDisplayOffset))
            return 0;

        return VideoPlane.pStereoMetadata[plane.lStereoDisplayOffset];
    }

    return 0;
}

void CD3D9VideoMixerBase::OnVideoPositionChange()
{
    m_bRecalculateVideoPosition = true;
}