#include "stdafx.h"
#include "VideoSourceEx.h"
#include "VideoSourceEvrEx.h"
#include "VideoSourceEvr.h"
#include "VideoSourceMSEvr.h"
#include "VideoSourceDO.h"
#include "Imports/XVE/Inc/XVE.h"
#include "Imports/XVE/Inc/XVEDef.h"
#include "Imports/XVE/Inc/XVEUtil.h"

using namespace DispSvr;

static inline void ConvertMixerCoordToD3D(NORMALIZEDRECT &nrcD3D, const NORMALIZEDRECT &nrcMixer)
{
	nrcD3D.left = nrcMixer.left * 2 - 1.0f;
	nrcD3D.right = nrcMixer.right * 2 - 1.0f;
	nrcD3D.top = 1.0f - nrcMixer.top * 2;
	nrcD3D.bottom = 1.0f - nrcMixer.bottom * 2;
}


CVideoSourceDO::CVideoSourceDO(LPUNKNOWN pUnk, HRESULT *phr)
        : CUnknown(NAME("VideoSource Display Object"), pUnk)
{
	m_pVideoSource = 0;
	ZeroMemory(&m_ExternalSurfaceDesc, sizeof(m_ExternalSurfaceDesc));
	LoadDefaultMixerCoordinate(m_nrcOutput);
	m_nrcZoom = m_nrcCrop = m_nrcOutput;
	m_bLockRatio = true;
	m_bShow = true;
	m_fAspectRatio = 0.f;
	m_fAlpha = 1.f;
	m_bNeedUpdatePosition = true;
	m_bEnableRendering = true;
	ZeroMemory(&m_rcSrc, sizeof(m_rcSrc));
	m_bPullSurfaceFromVideoSource = true;	// default is pull-mode to ask video source for surface update when compositing is requested.
    m_eStereoDispMode = STEREO_DISPLAYPROP_NONE;
	m_uiVideoWidth = 0;
	m_uiVideoHeight = 0;
	m_dwFrameRate1000 = 0;
}

CVideoSourceDO::~CVideoSourceDO()
{
    Terminate();
}

STDMETHODIMP CVideoSourceDO::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
    HRESULT hr = E_NOINTERFACE;
    *ppv = NULL;

    if (riid == IID_IDisplayObject)
    {
        hr = GetInterface((IDisplayObject*) this, ppv);
    }
    else if (riid == IID_IDisplayVideoMixer)
    {
        hr = GetInterface((IDisplayVideoMixer*) this, ppv);
    }
    else if (riid == IID_IDisplayVideoSourceManager)
    {
        hr = GetInterface((IDisplayVideoSourceManager*) this, ppv);
    }
    else if (riid == IID_IDisplayVideoSink)
    {
        hr = GetInterface((IDisplayVideoSink*) this, ppv);
    }
    else if (riid == IID_IDisplayProperties)
    {
        hr = GetInterface((IDisplayProperties*) this, ppv);
    }
	else if (riid == __uuidof(IDispSvrVideoMixerEventListener))
	{
		hr = GetInterface(static_cast<IDispSvrVideoMixerEventListener *> (this), ppv);
	}
    else if (riid == __uuidof(IDisplayStereoControl))
    {
        hr = GetInterface(static_cast<IDisplayStereoControl *> (this), ppv);
    }
	else if (riid == __uuidof(IDispSvrVideoMixer))
	{
		hr = m_pOwner->QueryInterface(riid, ppv);
	}
    else
    {
        hr = CUnknown::NonDelegatingQueryInterface(riid, ppv);
    }
    return hr;
}

//////////////////////////////////////////////////////////////////////////
// IDisplayVideoMixer

STDMETHODIMP CVideoSourceDO::GetRenderEngineOwner(IDisplayRenderEngine** ppRenderEngine)
{
	CHECK_POINTER(ppRenderEngine);

    if (!m_pOwner)
    {
        return VFW_E_NOT_FOUND;
    }

    *ppRenderEngine = m_pOwner;
    (*ppRenderEngine)->AddRef();

    return S_OK;
}

STDMETHODIMP CVideoSourceDO::Initialize(IDisplayRenderEngine* pRenderEngine)
{
	HRESULT hr = S_FALSE;

	CHECK_POINTER(pRenderEngine);

	if (m_pOwner)
	{
		ASSERT(m_pOwner == pRenderEngine);
		return hr;
	}

	hr = pRenderEngine->QueryInterface(__uuidof(IDispSvrVideoMixer), (void **) &m_pVideoMixer);
	if (SUCCEEDED(hr))
	{
		m_pOwner = pRenderEngine;
		ASSERT(m_pVideoMixer);
	}
    return hr;
}

// cleans data, releases interfaces
STDMETHODIMP CVideoSourceDO::Terminate()
{
	m_pVideoPlane.Release();
	m_pVSProperty.Release();
	HRESULT hr = RemoveVideoSource(m_pVideoSource);
	m_pVideoMixer.Release();

	// prevent owner be released while render thread want to present/render
	CAutoLock selfLock(&m_csObjForRender);
	m_pOwner.Release();
	return hr;
}

STDMETHODIMP CVideoSourceDO::BeginDeviceLoss(void)
{
	if (m_pVideoSource)
		m_pVideoSource->BeginDeviceLoss();

	//To solve issue: 
	//   Need to clear the video plane surface pointers before set external surface to NULL.
	//   Found one crash issue when apply multi-time device loss.
	//    -- another thread use unsafe external surface pointer to create texture.
	if (m_pVideoPlane)
		UnregisterSurface();
	return S_OK;
}

STDMETHODIMP CVideoSourceDO::EndDeviceLoss(IUnknown* pDevice)
{
	if (m_pVideoSource)
		m_pVideoSource->EndDeviceLoss(pDevice);
	return S_OK;
}

STDMETHODIMP CVideoSourceDO::ProcessMessage(HWND hWnd, UINT msg, UINT wParam, LONG lParam)
{
    HRESULT hr = E_INVALIDARG;

    switch(msg)
    {
    case VIDEOSOURCEDO_REQUEST_QUERY_SURFACE_STATUS:
        {
            IUnknown *pSurface = reinterpret_cast<IUnknown*>(lParam);
            ExternalSurfaceDesc desc = {0};
            desc.bQueryStatusOnly = TRUE;
            desc.pSurface = pSurface;
            hr = m_pVideoPlane->SetExternalSurface(&desc);
        }
        break;
    case VIDEOSOURCEDO_REQUEST_RELEASE_SURFACE:
        {
            UnregisterSurface();
            hr = S_OK;
        }
        break;

    case VIDEOSOURCEDO_REQUEST_UPDATE_SURFACE:
        {
            hr = UpdateSurfaceFromVideoSource();
        }
        break;

    default:
        hr = E_INVALIDARG;
    }

    return hr;
}

STDMETHODIMP CVideoSourceDO::Render(IUnknown *pDevice, const NORMALIZEDRECT* lpParentRect)
{
	CComQIPtr<IDirect3DDevice9> pDevice9 = pDevice;
	HRESULT hr = S_OK;

	if (!m_pOwner && !m_pVideoSource)
    {
        return VFW_E_WRONG_STATE;
    }

	hr = m_pVideoMixer->Execute();
    return hr;
}

STDMETHODIMP CVideoSourceDO::AddVideoSource(IBaseFilter* pVMR, IDisplayVideoSource** ppVidSrc)
{
	HRESULT hr;

	if (!m_pVideoMixer)
	{
		DbgMsg("CVideoSourceDO::AddVideoSource: no video mixer is found\n");
		return VFW_E_WRONG_STATE;
	}

	CHECK_POINTER(pVMR);
	ASSERT(m_pVideoSource == NULL);

	{
		CComQIPtr<IMFVideoRenderer> pEvr = pVMR;

		// we only enable push-mode in the MS presenter currently to avoid behavior change in other video source implementations.
		m_bPullSurfaceFromVideoSource = true;
		if (pEvr)
		{
			if (0)
			{
			m_pVideoSource = new CVideoSourceEvr(this, this);
			}
			else
			{
				m_pVideoSource = new CVideoSourceMSEvr(this, this); //new Custom Presenter from MS sample in Vista SDK.
				m_bPullSurfaceFromVideoSource = false;
			}
		}
		else
			m_pVideoSource = new CVideoSourceEx(this, this);
	}

	if (!m_pVideoSource)
		return E_OUTOFMEMORY;

	hr = m_pVideoSource->Attach(pVMR);
	if (SUCCEEDED(hr))
	{
		// we successfully attached subgraph, last thing left is to save
		// pVideoSource in the list
		m_pVideoSource->AddRef();
		if (ppVidSrc)
		{
			(*ppVidSrc = m_pVideoSource)->AddRef();
		}
	}
	else
	{
		int ref = m_pVideoSource->Release();
		ASSERT(ref == 0);
		m_pVideoSource = NULL;
	}

	NORMALIZEDRECT nrcOutput={0.0, 0.0, 1.0, 1.0};

	hr = m_pVideoMixer->SetDestination(NULL, &nrcOutput);

	return hr;
}

STDMETHODIMP CVideoSourceDO::RemoveVideoSource(IDisplayVideoSource* pVidSrc)
{
	if (m_pVideoSource != pVidSrc)
        return E_INVALIDARG;

	HRESULT hr = S_FALSE;

    CAutoLock selfLock(&m_csObjForRender);

	if (m_pVideoSource)
	{
		hr = m_pVideoSource->Detach();
		m_pVideoSource->Release();
		m_pVideoSource = 0;
    }
	return hr;
}

STDMETHODIMP CVideoSourceDO::GetVideoSourceCount(LONG* plCount)
{
	*plCount = m_pVideoSource ? 1 : 0;
	return S_OK;
}

STDMETHODIMP CVideoSourceDO::GetVideoSourceByIndex(LONG lIndex, IDisplayVideoSource** ppVideoSource)
{
	if (lIndex == 1 && m_pVideoSource)
	{
		(*ppVideoSource = m_pVideoSource)->AddRef();
		return S_OK;
	}
	return E_INVALIDARG;
}

//////////////////////////////////////////////////////////////////////////
// IDisplayVideoSink

STDMETHODIMP CVideoSourceDO::OnVideoSourceAdded(IDisplayVideoSource* pVidSrc, FLOAT fAspectRatio)
{
	HRESULT hr;
	PlaneInit init = {0};

	init.PlaneID = PLANE_MAINVIDEO;
	init.uWidth = 0;
	init.uHeight = 0;
	init.dwFlags = PLANE_INIT_EXTERNAL_SURFACE;
	init.Format = PLANE_FORMAT_YUY2;	// YUY2 is supported by main video.
    init.pListener = this;  // will receive EVENT_VIDEO_MIXING_FRAMERATE_CONVERSION when such an effect is enabled.

	CAutoLock selfLock(&m_csObj);
	ASSERT(m_pVideoPlane == 0);
	hr = m_pVideoMixer->CreatePlane(&init, __uuidof(IDispSvrVideoMixerVideoPlane), (void **) &m_pVideoPlane);
	if (SUCCEEDED(hr))
	{
		pVidSrc->QueryInterface(__uuidof(m_pVSProperty), (void **) &m_pVSProperty);

        BOOL bEnableFRC = FALSE;
        if (CComQIPtr<IDispSvrVideoEffectManager> pXVEManagerWrapper = m_pVideoMixer)
        {
            CComPtr<IXVideoEffectManager> pXVEManager;
	        pXVEManagerWrapper->GetVideoEffectManager((IUnknown **)&pXVEManager);
            if (pXVEManager)
            {
                CComPtr<IXVEType> pType;

                pXVEManager->GetOutputCurrentType(init.PlaneID, &pType);
                if (pType)
                {
                    UINT32 uMultipleOutput = 0;
                    if (SUCCEEDED(XVEGetTypeMultipleOutput(pType, uMultipleOutput)))
                    {
                        if (uMultipleOutput & XVE::XVE_MULTIPLEOUTPUT_FRAMERATE_CONVERSION)
                            bEnableFRC = TRUE;
                    }
                }
            }
        }
        EnableFrameRateConversion(bEnableFRC);
    }

    return hr;
}

STDMETHODIMP CVideoSourceDO::OnVideoSourceRemoved(IDisplayVideoSource* pVidSrc)
{
	CAutoLock selfLock(&m_csObj);
	m_pVideoPlane.Release();
	m_pVSProperty.Release();
	ZeroMemory(&m_ExternalSurfaceDesc, sizeof(m_ExternalSurfaceDesc));
	ZeroMemory(&m_rcSrc, sizeof(m_rcSrc));
    return S_FALSE;
}

STDMETHODIMP CVideoSourceDO::GetOutputRect(IDisplayVideoSource* pVidSrc, NORMALIZEDRECT* lpNormRect)
{
    if (!m_pOwner)
    {
        return VFW_E_WRONG_STATE;
    }

	return GetOutputRect(lpNormRect);
}

STDMETHODIMP CVideoSourceDO::SetOutputRect(IDisplayVideoSource* pVidSrc, NORMALIZEDRECT* lpNormRect)
{
    if (!m_pOwner)
    {
        return VFW_E_WRONG_STATE;
    }

	return SetOutputRect(lpNormRect);
}

STDMETHODIMP CVideoSourceDO::GetIdealOutputRect(
    IDisplayVideoSource* pVidSrc,
    DWORD dwWidth,
    DWORD dwHeight,
    NORMALIZEDRECT* lpNormRect)
{
    if (!m_pOwner)
    {
        return VFW_E_WRONG_STATE;
    }

	CHECK_POINTER(lpNormRect);

    lpNormRect->left = -1;
    lpNormRect->right = 1;
    lpNormRect->top = 1;
    lpNormRect->bottom = -1;

    return S_OK;
}

STDMETHODIMP CVideoSourceDO::GetZOrder(IDisplayVideoSource* pVidSrc, DWORD *pdwZ)
{
    if (!m_pOwner)
    {
        return VFW_E_WRONG_STATE;
    }

	CHECK_POINTER(pdwZ);
    return S_OK;
}

STDMETHODIMP CVideoSourceDO::SetZOrder(IDisplayVideoSource* pVidSrc, DWORD dwZ)
{
    if (!m_pOwner)
    {
        return VFW_E_WRONG_STATE;
    }

    return S_OK;
}

STDMETHODIMP CVideoSourceDO::GetAlpha(IDisplayVideoSource* pVidSrc, float* pAlpha)
{
    if (!m_pOwner)
    {
        return VFW_E_WRONG_STATE;
    }

	CHECK_POINTER(pAlpha);
	CAutoLock selfLock(&m_csObj);
	*pAlpha = m_fAlpha;
    return S_OK;
}

STDMETHODIMP CVideoSourceDO::SetAlpha(IDisplayVideoSource* pVidSrc, float Alpha)
{
    if (!m_pOwner)
    {
        return VFW_E_WRONG_STATE;
    }

	if (Alpha < 0.0f)
		Alpha = 0.0f;
	if (Alpha > 1.0f)
		Alpha = 1.0f;

    CAutoLock selfLock(&m_csObj);
	m_fAlpha = Alpha;
	if (m_pVideoPlane)
		return m_pVideoPlane->SetPlanarAlpha(Alpha);
    return E_FAIL;
}

STDMETHODIMP CVideoSourceDO::GetCLSID(CLSID* pClsid)
{
	CHECK_POINTER(pClsid);

    *pClsid = CLSID_VideoSourceDisplayObject;
    return S_OK;
}

STDMETHODIMP CVideoSourceDO::NotifyEvent(DWORD event, DWORD param1, DWORD param2, LPVOID pInstance)
{
	// clean up the video frame when stopping rendering
	HRESULT hr = S_OK;

	CComQIPtr<IDisplayEventHandler> pEventHandler = m_pVideoSource;
	if (pEventHandler)
	{
		hr = pEventHandler->NotifyEvent(event, param1, param2, pInstance);
	}

	switch (event)
	{
	case DISPLAY_EVENT_EnableRendering:
		// We should ignore DISPLAY_EVENT_EnableRendering when using push mode, or we may
		// lose some frames right after device lost. VideoSource only pushes video once and
		// VideoSourceDO may not receive DISPLAY_EVENT_EnableRendering=TRUE again at that time.
		// Especially DVD still menu may show black video after device lost in this case.
		if (m_bPullSurfaceFromVideoSource)
		{
		if (param1 == FALSE)
			hr = ClearFrame();
			m_bEnableRendering = (TRUE == (BOOL)param1);
		}
		else
			m_bEnableRendering = true;
		break;
	case DISPLAY_EVENT_VideoSourceRender:        
        {
            CAutoLock selfLock(&m_csObjForRender);
            if(m_pOwner)
                hr = m_pOwner->NodeRequest(DISPLAY_REQUEST_Render, param1, param2, this);
            break;
        }
	case DISPLAY_EVENT_VideoSourcePresent:
        {
            CAutoLock selfLock(&m_csObjForRender);
            if(m_pOwner)
                hr = m_pOwner->NodeRequest(DISPLAY_REQUEST_Present, param1, param2, this);
            break;
        }
	}

	return hr;
}

STDMETHODIMP CVideoSourceDO::KeepAspectRatio(IDisplayVideoSource* pVidSrc, BOOL bLockRatio)
{
	m_bLockRatio = (bLockRatio == TRUE);
	if (m_pVideoSource && m_pVideoSource == pVidSrc)
		m_bNeedUpdatePosition = true;
	return S_OK;
}

STDMETHODIMP CVideoSourceDO::Lock()
{
    m_csObj.Lock();
    return S_OK;
}

STDMETHODIMP CVideoSourceDO::Unlock()
{
    m_csObj.Unlock();
    return S_OK;
}

STDMETHODIMP CVideoSourceDO::TryLock()
{
	return E_NOTIMPL;
}

STDMETHODIMP CVideoSourceDO::Notify(DWORD dwEvent, DWORD dwParam1, DWORD dwParam2)
{
	HRESULT hr = S_FALSE;

    switch (dwEvent)
    {
    case EVENT_VIDEO_MIXING_BEGIN:
    case EVENT_VIDEO_MIXING_CHANGE_DESTINATION:
        if (m_bPullSurfaceFromVideoSource)
            hr = UpdateSurfaceFromVideoSource();
        break;
    case EVENT_VIDEO_MIXING_FRAMERATE_CONVERSION:
        EnableFrameRateConversion(dwParam1 == TRUE);
        break;
    }
	return hr;
}

//////////////////////////////////////////////////////////////////////////
// IDisplayProperties

STDMETHODIMP CVideoSourceDO::GetOutputRect(NORMALIZEDRECT* lpNormRect)
{
	ASSERT(lpNormRect);
	ConvertMixerCoordToD3D(*lpNormRect, m_nrcOutput);
	return S_OK;
}

STDMETHODIMP CVideoSourceDO::SetOutputRect(NORMALIZEDRECT* lpNormRect)
{
	ASSERT(lpNormRect);
	NORMALIZEDRECT rect;

	ConvertD3DCoordToMixer(rect, *lpNormRect);
	if (rect != m_nrcOutput)
	{
		m_nrcOutput = rect;
		m_bNeedUpdatePosition = true;
	}
	return S_OK;
}

STDMETHODIMP CVideoSourceDO::GetCropRect(NORMALIZEDRECT* lpNormRect)
{
	CHECK_POINTER(lpNormRect);
	*lpNormRect = m_nrcCrop;
	return S_OK;
}

STDMETHODIMP CVideoSourceDO::SetCropRect(NORMALIZEDRECT* lpNormRect)
{
	CHECK_POINTER(lpNormRect);
	if (m_nrcCrop != *lpNormRect)
	{
		m_nrcCrop = *lpNormRect;
		m_bNeedUpdatePosition = true;
	}
    return S_OK;
}

STDMETHODIMP CVideoSourceDO::GetFrameColor(COLORREF* pColor)
{
    return E_NOTIMPL;
}

STDMETHODIMP CVideoSourceDO::SetFrameColor(COLORREF color)
{
	return E_NOTIMPL;
}

STDMETHODIMP CVideoSourceDO::CaptureFrame(DWORD dwFormat, BYTE **ppFrame, UINT  *pSize)
{
    return E_NOTIMPL;
}

STDMETHODIMP CVideoSourceDO::GetZoom(NORMALIZEDRECT *lpNormRect)
{
	ASSERT(lpNormRect);
	ConvertMixerCoordToD3D(*lpNormRect, m_nrcZoom);
	return S_OK;
}

STDMETHODIMP CVideoSourceDO::SetZoom(NORMALIZEDRECT *lpNormRect)
{
	ASSERT(lpNormRect);
	NORMALIZEDRECT rect;

	ConvertD3DCoordToMixer(rect, *lpNormRect);
	if (rect != m_nrcZoom)
	{
		m_nrcZoom = rect;
		m_bNeedUpdatePosition = true;
	}
	return S_OK;
}

STDMETHODIMP CVideoSourceDO::SetShow(BOOL b)
{
	bool bShow = b ? true : false;
	if (bShow != m_bShow)
	{
		m_bShow = bShow;
		if (m_pVideoPlane)
			m_pVideoPlane->SetPlanarAlpha(m_bShow ? m_fAlpha : 0.f);
	}
	return S_OK;
}

STDMETHODIMP CVideoSourceDO::GetShow(BOOL *pbShow)
{
	CHECK_POINTER(pbShow);
	*pbShow = m_bShow ? TRUE : FALSE;
	return S_OK;
}

STDMETHODIMP CVideoSourceDO::ClearFrame()
{
	HRESULT hr = S_OK;

	if (m_pVideoPlane)
		UnregisterSurface();
	return hr;
}

STDMETHODIMP CVideoSourceDO::SetDispFrameRate(DWORD dwFrameRate)
{
	HRESULT hr = S_OK;
	hr = m_pVSProperty->SetDispFrameRate(dwFrameRate);
	return hr;
}


//////////////////////////////////////////////////////////////////////////
// IDisplayStereoControl

STDMETHODIMP CVideoSourceDO::SetStereoDisplayMode(DWORD stereoDispProp)
{
    StereoDisplayProperties stereoDispMode = static_cast<StereoDisplayProperties>(stereoDispProp);
    if (m_eStereoDispMode != stereoDispMode)
    {
        m_eStereoDispMode = stereoDispMode;
    }
    
    return S_OK;
}

STDMETHODIMP CVideoSourceDO::GetStereoDisplayMode(DWORD *stereoDispProp)
{
    if (stereoDispProp)
        *stereoDispProp = static_cast<DWORD>(m_eStereoDispMode);
    return S_OK;
}

/////////////////////// Private routine ///////////////////////////////////////

STDMETHODIMP CVideoSourceDO::Get3DDevice(IUnknown** ppDevice)
{
    if (!m_pOwner)
    {
        return VFW_E_WRONG_STATE;
    }

    return m_pOwner->Get3DDevice(ppDevice);
}

HRESULT CVideoSourceDO::UpdateSurfaceFromVideoSource()
{
	CAutoLock selfLock(&m_csObj);
	if (!m_pVideoSource || !m_pVideoMixer || !m_pVideoPlane || !m_bEnableRendering)
		return E_FAIL;

	HRESULT hr = E_FAIL;
	LONG wImage, hImage;
	float fAspectRatio;
	CComPtr<IUnknown> pObj;
	CComPtr<IDirect3DSurface9> pSurface;
	CSampleProperty prop;

	if (m_pVSProperty)
	{
		hr = m_pVSProperty->GetSampleProperty(&prop);
	}

	// if IDisplayVideoSourcePropertyExtension::GetSampleProperty succeeds
	if (SUCCEEDED(hr))
	{
		pObj = prop.pSurface;
		wImage = prop.uWidth;
		hImage = prop.uHeight;
		fAspectRatio = prop.fAspectRatio;
	}
	else
	{
		hr = m_pVideoSource->GetVideoSize(&wImage, &hImage, &fAspectRatio);
		if (SUCCEEDED(hr))
			hr = m_pVideoSource->GetTexture(&pObj, NULL);
	}

	if (FAILED(hr))
	{
		//last Image incorrect or not exist, we should clear external surface to avoid to draw old frame
		m_ExternalSurfaceDesc.pSurface = NULL; //clean pointer also, weak reference, no need release
		m_pVideoPlane->SetExternalSurface(NULL);
		return hr;
	}

	hr = pObj.QueryInterface(&pSurface);
	if (!pSurface)
	{
		CComQIPtr<IDirect3DTexture9> pTexture = pObj;
		if (pTexture)
		{
			pObj.Release();
			hr = pTexture->GetSurfaceLevel(0, &pSurface);
			pSurface.QueryInterface(&pObj);
		}
	}

	if (pSurface)
	{
		if (m_rcSrc.right != wImage
			|| m_rcSrc.bottom != hImage
			|| m_fAspectRatio != fAspectRatio)
		{
			SetRect(&m_rcSrc, 0, 0, wImage, hImage);
			m_fAspectRatio = fAspectRatio;
			m_bNeedUpdatePosition = true;
		}

		UpdatePosition();

		if (m_ExternalSurfaceDesc.uWidth != wImage
			|| m_ExternalSurfaceDesc.uHeight != hImage
			|| m_ExternalSurfaceDesc.pSurface != pObj)
		{
			D3DSURFACE_DESC desc;

			if (SUCCEEDED(hr))
				hr = pSurface->GetDesc(&desc);

			if (SUCCEEDED(hr))
			{
				m_ExternalSurfaceDesc.uWidth = wImage;
				m_ExternalSurfaceDesc.uHeight = hImage;
				if (desc.Format == D3DFMT_X8R8G8B8 || desc.Format == D3DFMT_A8R8G8B8 )
					m_ExternalSurfaceDesc.Format = PLANE_FORMAT_ARGB;
				else
					m_ExternalSurfaceDesc.Format = static_cast<PLANE_FORMAT> (desc.Format);
				m_ExternalSurfaceDesc.pSurface = pObj;
				hr = m_pVideoPlane->SetExternalSurface(&m_ExternalSurfaceDesc);
			}
		}

		if ( m_uiVideoWidth != wImage || m_uiVideoHeight != hImage || m_dwFrameRate1000 != prop.dwFrameRate1000)
		{
			DWORD dwParam[3];
			dwParam[0] = wImage;
			dwParam[1] = hImage;
			dwParam[2] = prop.dwFrameRate1000 / 1000;
			hr = m_pOwner->ProcessRequest(DISPLAY_REQUEST_VideoFormat, reinterpret_cast<DWORD>(dwParam), NULL);
			DbgMsg("CVideoSourceDO::UpdateSurfaceFromVideoSource(), VideoFormat, width= %d, height= %d, framerate= %d", dwParam[0], dwParam[1], dwParam[2]);
		}

		VideoProperty vp = {0};
		m_uiVideoWidth = vp.uWidth = wImage;
		m_uiVideoHeight = vp.uHeight = hImage;
		vp.rtStart = prop.rtStart;
		vp.rtEnd = prop.rtEnd;
		vp.dwFieldSelect = FIELD_SELECT_FIRST;
        vp.rtDisplayTarget = prop.rtDisplayTarget;
		m_dwFrameRate1000 = vp.dwFrameRate1000 = prop.dwFrameRate1000;

        if (prop.dwFlags & SAMPLE_FLAG_REPEAT_FRAME)
            vp.bRepeatFrame = TRUE;

		if (prop.dwFlags & SAMPLE_FLAG_INTERLACED)
		{
			if (prop.dwFlags & SAMPLE_FLAG_BOTTOMFIELDFIRST)
				vp.Format = VIDEO_FORMAT_FIELD_INTERLEAVED_ODD_FIRST;
			else
				vp.Format = VIDEO_FORMAT_FIELD_INTERLEAVED_EVEN_FIRST;

			if (prop.dwFlags & SAMPLE_FLAG_SELECTSECONDFIELD)
				vp.dwFieldSelect = FIELD_SELECT_SECOND;
		}
		else
		{
			vp.Format = VIDEO_FORMAT_PROGRESSIVE;
		}

		m_pVideoPlane->SetVideoProperty(&vp);
	}
	else
	{
		UnregisterSurface();
	}
	return hr;
}

void CVideoSourceDO::UpdatePosition()
{
	HRESULT hr = E_FAIL;

	if (m_pVideoPlane && m_bNeedUpdatePosition)
	{
		float fAspectRatio = m_bLockRatio ? m_fAspectRatio : 0;
		RECT rcSrc = m_rcSrc;

		CropRect(rcSrc, m_nrcZoom);
		m_bNeedUpdatePosition = FAILED(m_pVideoPlane->SetPosition(&m_nrcOutput, &rcSrc, &m_nrcCrop, fAspectRatio));
	}
}

void CVideoSourceDO::UnregisterSurface()
{
	CAutoLock selfLock(&m_csObj);

	ZeroMemory(&m_ExternalSurfaceDesc, sizeof(m_ExternalSurfaceDesc));
	m_pVideoPlane->SetExternalSurface(NULL);
	
	if(m_pVideoSource)
		m_pVideoSource->ClearImage();
}

void CVideoSourceDO::EnableFrameRateConversion(BOOL bEnable, DWORD dwNumerator, DWORD dwDenominator)
{
    if (CComQIPtr<IDisplayVideoSourceFRCControl> pFRCControl = m_pVideoSource)
    {
        if (bEnable)
        {
            FrameRateRatio ratio = { dwNumerator, dwDenominator };
            pFRCControl->EnableFRC(&ratio);
        }
        else
        {
            pFRCControl->EnableFRC(NULL);
        }
    }
}