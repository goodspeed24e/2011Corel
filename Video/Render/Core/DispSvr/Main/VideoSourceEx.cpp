#include "stdafx.h"
#include "DispSvr.h"
#include "VideoSourceEx.h"
#include "ResourceManager.h"
#include "Exports/Inc/VideoMixer.h"
#include "Exports/Inc/VideoPresenter.h"
#include <dvdmedia.h>
// At 2X, 4X, we may receive samples at 120Hz, 240Hz
// It does not make sense to display them all and some of the presenters can't handle
// display frequency so high. It is easier to drop the exceeding samples by not presenting
// all of the samples.

// (Duration - Threshold) must be lower than 4ms due to the duration might be 4ms while playing 240hz
//Therefore set 13ms for 60fps, and 30ms for 30fps
#define INITIATIVE_RENDERING_SAMPLE_DURATION_THRESHOLD_60FPS (130000LL)
#define INITIATIVE_RENDERING_SAMPLE_DURATION_THRESHOLD_30FPS  (300000LL)

#define DISP_AUTO_PROFILE(id, subid) { if (m_bFirstTimeDisp) {AUTO_SCOPE_PROFILE(id, subid)}}
/////////////////////// Private class CCVideoSourceEx ///////////////////////////////////////

using namespace DispSvr;

CVideoSourceEx::CVideoSourceEx(IDisplayLock* pLock, IDisplayVideoSink* pVideoSink) :
CUnknown(NAME("CVideoSource"), NULL)
{
	m_dwNumBuf = 0L;
	m_dwNumBufActuallyAllocated = 0L;
	m_lImageWidth = 0L;
	m_lImageHeight = 0L;
	m_ppSurface = NULL;

	m_pLock = pLock;
	m_pVideoSink = pVideoSink;

	m_bValid = TRUE;
	m_bInitiativeDisplay = TRUE;
	m_bTextureSurface = false;
	m_bImageAvailable = false;
	m_hwndVideo	= 0;
	m_fNativeAspectRatio = 0.0;
	m_dwAspectRatioMode = VMR9ARMode_None;
	memset(&m_dstrect, 0, sizeof(m_dstrect));
	m_bStopState = TRUE;
	m_rtStart = m_rtEnd = 0;
    m_rtLaststart = 0;
    m_dwMaxDisplayFrameRate = 60;
    m_bFirstTimeDisp = true;
}

CVideoSourceEx::~CVideoSourceEx()
{
	Cleanup();
}

void CVideoSourceEx::Cleanup()
{
	m_pDefaultNotify.Release();
	m_pGraph.Release();
	DeleteSurfaces();
	IBaseFilter *pVMR = m_pVMR.Detach();
	if (pVMR)
	{
		int ref = pVMR->Release();
		if (ref > 0)
		{
			DbgMsg("CVideoSourceEx::Cleanup: m_pVMR ref count = %d, should be 0.\n", ref);
		}
	}

	m_lImageWidth = m_lImageHeight = 0L;
    m_rtLaststart = 0;
}

STDMETHODIMP CVideoSourceEx::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
	HRESULT hr;
	*ppv = NULL;

	if (riid == IID_IDisplayVideoSource)
	{
		hr = GetInterface((IDisplayVideoSource*) this, ppv);
	}
	else if (riid == IID_IVMRSurfaceAllocator9)
	{
		hr = GetInterface((IVMRSurfaceAllocator9*) this, ppv);
	}
	else if (riid == IID_IVMRImagePresenter9)
	{
		hr = GetInterface((IVMRImagePresenter9*) this, ppv);
	}
	else if (riid == __uuidof(IDisplayEventHandler))
	{
		hr = GetInterface((IDisplayEventHandler *) this, ppv);
	}
	else if (riid == IID_IVMRWindowlessControl9)
	{
		hr = GetInterface((IVMRWindowlessControl9*) this, ppv);
	}
	else if (riid == __uuidof(IDisplayVideoSourcePropertyExtension))
	{
		hr = GetInterface((IDisplayVideoSourcePropertyExtension *) this, ppv);
	}
	else
	{
		hr = CUnknown::NonDelegatingQueryInterface(riid, ppv);
	}
	return hr;
}

//////////////////////////////////////////////////////////////////////////
// IDisplayVideoSource

STDMETHODIMP CVideoSourceEx::GetGraph(IFilterGraph** ppGraph)
{
	if (!ppGraph)
	{
		DbgMsg(("CVideoSourceEx::GetGraph: ppGraph is NULL"));
		return E_POINTER;
	}

	if (m_pGraph)
	{
		m_pGraph.CopyTo(ppGraph);
		return S_OK;
	}

	DbgMsg(("CVideoSourceEx::GetGraph: FATAL: contains NULL IFilterGraph pointer"));
	return VFW_E_NOT_FOUND;
}

STDMETHODIMP CVideoSourceEx::GetTexture(IUnknown** ppTexture, NORMALIZEDRECT* lpNormRect)
{
	HRESULT hr = E_FAIL;
	BOOL bNotStopped = FALSE;
	//m_bImageAvailable becomes FALSE when pause/stop filter.
	//If filter become stopped state(m_bStopState = TRUE)
	//do not check filter state until receiving new image( m_bstopState = FALSE, see presentimage())
	//it is used to avoid getting paused state when filter changing state from stopped to running(stopped-paused-running)
	if (!m_bImageAvailable && !m_bStopState)
	{
		if (IsFilterStopped())
		{
			m_bStopState= TRUE;
			bNotStopped = FALSE;
		}
		else
			bNotStopped = TRUE;
	}
	// Running state: m_bImageAvailable = TRUE
	// Pause state : m_bImageAvailable = FALSE, bNotStopped = TRUE.
	// Stop state : m_bImageAvailable = FALSE, bNotStopped = FALSE.
	// block image on stop state and one kind paused state(see above)
	if (m_bImageAvailable || bNotStopped)
	{
		ASSERT(ppTexture);

		if (m_bTextureSurface)
			hr = m_ppSurface[0]->GetContainer(IID_IUnknown, (void **)ppTexture);
		else
			hr = m_ppSurface[0]->QueryInterface(IID_IUnknown, (void **)ppTexture);

		if (lpNormRect)
		{
			lpNormRect->right = lpNormRect->bottom = 1.0f;
			lpNormRect->left = lpNormRect->top = 0.0f;
		}
	}
	return hr;
}

STDMETHODIMP CVideoSourceEx::BeginDraw()
{
	return S_OK;
}

STDMETHODIMP CVideoSourceEx::EndDraw()
{
	return S_OK;
}

STDMETHODIMP CVideoSourceEx::IsValid()
{
	return m_bValid ? S_OK : E_FAIL;
}

STDMETHODIMP CVideoSourceEx::GetVideoSize(LONG* plWidth, LONG* plHeight, float *pfAspectRatio)
{
	if (m_dwAspectRatioMode == VMR9ARMode_LetterBox)
	{
		if (m_szNativeAspectRatio.cx != 0 && m_szNativeAspectRatio.cy != 0)
		{
			FLOAT fAspectRatio = (float)m_szNativeAspectRatio.cx / (float)m_szNativeAspectRatio.cy;
			
			m_fAspectRatio = m_fNativeAspectRatio = fAspectRatio;
		}
	}
	else if (m_dwAspectRatioMode == VMR9ARMode_None)
		m_fAspectRatio = 0.0;

	BOOL bNotStopped = FALSE;
	//If filter become stopped state once, blocking pause state until receiving new image
	//To avoid get paused state between changing state from stopped to running(stopped-paused-running)
	if (!m_bImageAvailable && !m_bStopState)
	{
		if (IsFilterStopped())
		{
			m_bStopState= TRUE;
			bNotStopped = FALSE;
		}
		else
			bNotStopped = TRUE;
	}

	if (m_bImageAvailable || bNotStopped)
	{
		ASSERT(plWidth && plHeight && pfAspectRatio);
		*plWidth = m_lImageWidth;
		*plHeight = m_lImageHeight;
		*pfAspectRatio = m_fAspectRatio;
		return S_OK;
	}
	return E_FAIL;
}

STDMETHODIMP CVideoSourceEx::ClearImage()
{
	//Set m_bStopState = True and m_bImageAvailable = false(see StopPresenting()),
	//then current image will become invalid until new image coming.(see presentimage())
	//On XP, GPI mode and DXVA1 on, the filter is always keeping running state even playback is in stop
	//use this way to ignore current image.
	//So when next time playback, VideoSourceEx will not show any frame until new frame received.
	//Fixed the bug#67441, "XP","GPI mode" and "DXVA1 enabled" situation
	m_bStopState = TRUE;
	return StopPresenting(GetID());
}

HRESULT CVideoSourceEx::BeginDeviceLoss()
{
	CAutoDisplayLock displayLock(m_pLock);
	m_bValid = FALSE;
	DbgMsg(("CVideoSourceEx::BeginDeviceLoss"));
	DeleteSurfaces();
    m_bFirstTimeDisp = true;
	return S_OK;
}

HRESULT CVideoSourceEx::EndDeviceLoss(IUnknown* pDevice)
{
	HRESULT hr = S_OK;
	if (m_pDefaultNotify)
	{
		DbgMsg(("CVideoSourceEx::EndDeviceLoss: calling IVMRSurfaceAllocatorNotify9::ChangeD3DDevice"));
		CComQIPtr<IDirect3DDevice9> pDevice9 = pDevice;
		CComPtr<IDirect3D9> pd3d9;
		if (pDevice9)
			hr = pDevice9->GetDirect3D(&pd3d9);

		HMONITOR hMon = NULL;
		if (pd3d9)
			hMon = pd3d9->GetAdapterMonitor(D3DADAPTER_DEFAULT);
		hr = m_pDefaultNotify->ChangeD3DDevice(pDevice9, hMon);
	}
	return hr;
}


HRESULT CVideoSourceEx::EnableInitiativeDisplay(BOOL bEnable)
{
	HRESULT hr = S_OK;
	m_bInitiativeDisplay = bEnable;
	return hr;
}

// Disconnects pins of VMR
HRESULT CVideoSourceEx::DisconnectPins()
{
	HRESULT hr = S_OK;
	if (!m_pVMR)
	{
		return E_POINTER;
	}

	try
	{
		CComPtr<IEnumPins> pEnum;
		hr = m_pVMR->EnumPins(&pEnum);
		CHECK_HR(hr, DbgMsg("CVideoSourceEx::DisconnectPins: failed to enumerate pins, hr = 0x%08x", hr));

		CComPtr<IPin> pPin;
		hr = pEnum->Next(1, &pPin, NULL);
		while (S_OK == hr && pPin)
		{
			hr = pPin->Disconnect();
			CHECK_HR(hr, DbgMsg("CVideoSourceEx::DisconnectPins: failed to disconnect pin, hr = 0x%08x", hr));

			pPin.Release();
			hr = pEnum->Next(1, &pPin, NULL);
		}
	}
	catch (HRESULT hrFailed)
	{
		hr = hrFailed;
	}

	return hr;
}

// deletes allocated surface buffers
void CVideoSourceEx::DeleteSurfaces()
{
	m_bImageAvailable = false;
	if (m_ppSurface)
	{
		for (DWORD dwS = 0; dwS < m_dwNumBuf; dwS++)
		{
			if (m_ppSurface[dwS])
			{
				int ref = (m_ppSurface[dwS])->Release();
				m_ppSurface[dwS] = NULL;
			}
		}
		delete[] m_ppSurface;
		m_ppSurface = NULL;
	}
	m_dwNumBuf = 0L;
	m_dwNumBufActuallyAllocated = 0L;
}

// allocates buffer of dwN surfaces
HRESULT CVideoSourceEx::AllocateSurfaceBuffer(DWORD dwN)
{
	ASSERT(dwN >= 0);
	DeleteSurfaces();
	m_dwNumBuf = dwN;
	m_ppSurface = new IDirect3DSurface9 * [m_dwNumBuf];
	if (!m_ppSurface)
	{
		m_dwNumBuf = 0L;
		return E_OUTOFMEMORY;
	}

	ZeroMemory(m_ppSurface, m_dwNumBuf * sizeof(IDirect3DSurface9*));
	return S_OK;
}

///////////////////////// IVMRSurfaceAllocator9 ///////////////////////////////

/**
* AdviseNotify
*
* For usage, parameters and return codes
* see DirectX SDK documentation, IVMRSurfaceAllocator9 interface
*
* In custom implementation of this method, we do nothing. Why?
* When VMR is created, it creates its own (default) allocator-presenter
* and all together with IVMRSurfaceAllocatorNotify9::AdviseSurfaceAllocator,
* method IVMRSurfaceAllocator9::AdviseNotify() is used to make two objects
* talking to each other. We do not implement our custom IVMRSurfaceAllocatorNotify9
* in this sample, and we have complete control over our custom A/P (this class
* CVideoSourceEx), so we do not have to tell A/P about VMR's notifier.
*/
STDMETHODIMP CVideoSourceEx::AdviseNotify(IVMRSurfaceAllocatorNotify9* lpIVMRSurfAllocNotify)
{
	return S_OK;
}

/*
typedef enum VMR9SurfaceAllocationFlags{
VMR9AllocFlag_3DRenderTarget  = 0x0001,
VMR9AllocFlag_DXVATarget       = 0x0002,
VMR9AllocFlag_TextureSurface   = 0x0004,
VMR9AllocFlag_OffscreenSurface = 0x0008,
VMR9AllocFlag_RGBDynamicSwitch = 0x0010,
VMR9AllocFlag_UsageReserved    = 0x00E0,
VMR9AllocFlag_UsageMask        = 0x00FF
};
*/
STDMETHODIMP CVideoSourceEx::InitializeDevice(DWORD_PTR dwUserID, VMR9AllocationInfo* lpAllocInfo, DWORD* lpNumBuffers)
{
	HRESULT hr = S_OK;

	// check we are provided valid parameters
	if (!lpAllocInfo || !lpNumBuffers)
		return E_POINTER;

	if (*lpNumBuffers < 1)
		*lpNumBuffers = 1;

	// check we know about the default IVMRSurfaceAllocatorNotify9
	if (!m_pDefaultNotify)
	{
		DbgMsg(("CVideoSourceEx::InitializeDevice: FATAL: video source contains NULL pointer to IVMRSurfaceAllocatorNotify9"));
		return E_FAIL;
	}

	if (FAILED(VerifyID(dwUserID)))
		return VFW_E_NOT_FOUND;

	try
	{
		// we have to be thread safe only here when we actually mess up with shared data
		CAutoDisplayLock displayLock(m_pLock);

		DeleteSurfaces();

		// allocate surface buffer
		hr = AllocateSurfaceBuffer(*lpNumBuffers);
		CHECK_HR(hr, DbgMsg("CVideoSourceEx::InitializeDevice: failed to allocate surface buffer, hr = 0x%08x, dwBuffers = %ld", hr, *lpNumBuffers));

		// first try to allocate textureable surface
		if (lpAllocInfo->dwFlags & (VMR9AllocFlag_3DRenderTarget | VMR9AllocFlag_DXVATarget))
		{
			lpAllocInfo->dwFlags |= VMR9AllocFlag_TextureSurface;
			hr = m_pDefaultNotify->AllocateSurfaceHelper(lpAllocInfo, lpNumBuffers, m_ppSurface);
			if (SUCCEEDED(hr))
				m_bTextureSurface = true;
		}

		if (!m_bTextureSurface)
		{
			lpAllocInfo->dwFlags &= ~VMR9AllocFlag_TextureSurface;
			hr = m_pDefaultNotify->AllocateSurfaceHelper(lpAllocInfo, lpNumBuffers,	m_ppSurface);
			CHECK_HR(hr, DbgMsg("CVideoSourceEx::InitializeDevice: failed in IVMRSurfaceAllocatorNotify9::AllocateSurfaceHelper, hr = 0x%08x, dwBuffers = %ld", hr, *lpNumBuffers));
		}

		m_dwNumBufActuallyAllocated = *lpNumBuffers;
		m_lImageWidth = lpAllocInfo->dwWidth;
		m_lImageHeight = lpAllocInfo->dwHeight;

		if (lpAllocInfo->szAspectRatio.cy > 0)
			m_fAspectRatio = (float) lpAllocInfo->szAspectRatio.cx/lpAllocInfo->szAspectRatio.cy;
		else
			m_fAspectRatio = (float) lpAllocInfo->dwWidth / lpAllocInfo->dwHeight;

		// tell the mixer we are "on"
		hr = m_pVideoSink->OnVideoSourceAdded(this, m_fAspectRatio);
		CHECK_HR(hr, DbgMsg("CVideoSourceEx::InitializeDevice: failed in IDisplayVideoSink::AddVideoSource(), hr = 0x%08x", hr));

        CheckMaxDisplayFrameRate();

        m_rtLaststart = 0;

		m_bValid = TRUE;
	}
	catch (HRESULT hrFailed)
	{
		hr = hrFailed;
		DeleteSurfaces();
	}

	return hr;
}

/**
* TerminateDevice
*
* For usage, parameters and return codes
* see DirectX SDK documentation, IVMRSurfaceAllocator9 interface
*
* In the custom implementation of this method, we just release the surface buffer
* associated with correspondent video source
*
*/
STDMETHODIMP CVideoSourceEx::TerminateDevice(DWORD_PTR dwUserID)
{
	if (FAILED(VerifyID(dwUserID)))
		return VFW_E_NOT_FOUND;

	return m_pVideoSink->OnVideoSourceRemoved(this);
}

/**
* GetSurface
*
* For usage, parameters and return codes
* see DirectX SDK documentation, IVMRSurfaceAllocator9 interface
*
* In the custom implementation of this method, we check that dwUserID is valid,
* that Surface index does not exceed number of allocated buffers, and return 
* pointer to correspondent surface
*/
STDMETHODIMP CVideoSourceEx::GetSurface(DWORD_PTR dwUserID, DWORD SurfaceIndex,	DWORD SurfaceFlags, IDirect3DSurface9** lplpSurface)
{
	ASSERT(lplpSurface);
	ASSERT(SurfaceIndex < m_dwNumBufActuallyAllocated);

	if (FAILED(VerifyID(dwUserID)))
		return VFW_E_NOT_FOUND;

	CAutoDisplayLock displayLock(m_pLock);

	// check that requested surface is not null
	if (!m_ppSurface || !m_ppSurface[SurfaceIndex])
		return E_FAIL;

	// now we checked everything and can copy
	*lplpSurface = m_ppSurface[SurfaceIndex];
	(*lplpSurface)->AddRef();

	return S_OK;
}

///////////////////////// IVMRImagePresenter9 ///////////////////////////////

/**
* StartPresenting
*
* For usage, parameters and return codes
* see DirectX SDK documentation, IVMRImagePresenter9 interface
*
* In the custom implementation of this method, we check that dwUserID is valid
* and Direct3Ddevice from the video sink is ready to go
*/
STDMETHODIMP CVideoSourceEx::StartPresenting(DWORD_PTR dwUserID)
{
	if (FAILED(VerifyID(dwUserID)))
		return VFW_E_NOT_FOUND;

	CAutoDisplayLock displayLock(m_pLock);
	if (!m_pVideoSink)
	{
		DbgMsg(("CVideoSourceEx::StartPresenting: FATAL, video sink is NULL"));
		return E_UNEXPECTED;
	}

	CComPtr<IDirect3DDevice9> pDevice;
	HRESULT hr = m_pVideoSink->Get3DDevice((IUnknown**)&pDevice);
	if (FAILED(hr) || !pDevice)
	{
		DbgMsg("CVideoSourceEx::StartPresenting: FATAL, Direct3DDevice in the video sink is NULL, "\
			"hr = 0x%08x, pDirect3DDevice = 0x%08x", hr, pDevice);
		return (FAILED(hr) ? hr : E_UNEXPECTED);
	}

	return hr;
}

/**
* StopPresenting
*
* For usage, parameters and return codes
* see DirectX SDK documentation, IVMRImagePresenter9 interface
*
* In the custom implementation of this method, we do nothing. By default, method 
* does not actually perform anything special, but one can use it to reflect on 
* the status of the rendering (vs. status of the graph) etc. 
*
*/
STDMETHODIMP CVideoSourceEx::StopPresenting(DWORD_PTR dwUserID)
{
	m_bImageAvailable = false;
    m_rtLaststart = 0;
	return S_OK;
}

/*
typedef enum VMR9PresentationFlags{
VMR9Sample_SyncPoint  = 0x00000001,
VMR9Sample_Preroll  = 0x00000002,
VMR9Sample_Discontinuity  = 0x00000004,
VMR9Sample_TimeValid  = 0x00000008
0x00000010, src/dst rect is valid
};
*/
STDMETHODIMP CVideoSourceEx::PresentImage(DWORD_PTR dwUserID, VMR9PresentationInfo* lpPresInfo)
{
    DISP_AUTO_PROFILE(dog::CAT_VIDEO, dogsub::CAT_VIDEO_DISPSVR_DSHOW_1ST_PRESENTIMAGE_BEGIN);
	if (FAILED(VerifyID(dwUserID)))
		return VFW_E_NOT_FOUND;

	m_rtStart = lpPresInfo->rtStart;
	m_rtEnd = lpPresInfo->rtEnd;
	m_szNativeAspectRatio = lpPresInfo->szAspectRatio;
	m_bImageAvailable = true;
	m_bStopState = FALSE; // Set false to indicate that we receive a valid image
	if (m_bInitiativeDisplay)
	{
		// Force render new frame after PresentImage() is called to prevent drop frame.
		CComQIPtr<IDisplayObject> pObj = m_pVideoSink;
		if (pObj)
		{
            REFERENCE_TIME hnsDuration = m_rtEnd - m_rtStart;
            // Drop frame due to
            //1. present rate is higher than max frame rate, then drop 2nd frame.
            // Condition (duration < duration threshold) and (new present time - last present time) > duration threshold
            if (IsOverDisplayFrameRate(hnsDuration) && IsOverDisplayFrameRate(m_rtStart - m_rtLaststart))
            {
                //DbgMsg("VideoSourceEx - drop frame\n");
                return S_OK;
            }
            m_rtLaststart = m_rtStart;
            pObj->NotifyEvent(DISPLAY_EVENT_VideoSourceRender, 0, 0, this);
            pObj->NotifyEvent(DISPLAY_EVENT_VideoSourcePresent, 0, 0, this);
		}
	}
    DISP_AUTO_PROFILE(dog::CAT_VIDEO, dogsub::CAT_VIDEO_DISPSVR_DSHOW_1ST_PRESENTIMAGE_END);
    m_bFirstTimeDisp = false;
	return S_OK;
}

STDMETHODIMP CVideoSourceEx::NotifyEvent(DWORD event, DWORD param1, DWORD param2, LPVOID pInstance)
{
	if (event == DISPLAY_EVENT_EnableRendering)
		m_bImageAvailable = false;
	return S_OK;
}

HRESULT CVideoSourceEx::Attach(IBaseFilter* pVMR)
{
	HRESULT hr;
	FILTER_INFO fiVMR;
	ZeroMemory(&fiVMR, sizeof(fiVMR));

	try
	{
		// check that provided VMR is part of the graph
		m_pVMR = pVMR;
		hr = m_pVMR->QueryFilterInfo(&fiVMR);
		hr = (NULL == fiVMR.pGraph) ? E_FAIL : S_OK;
		CHECK_HR(hr, DbgMsg("CVideoSourceEx::Attach: provided VMR was not added to the graph"));

		// check that provided VMR is in renderless mode
		CComPtr<IVMRFilterConfig9> pFilterConfig;
		hr = m_pVMR->QueryInterface(IID_IVMRFilterConfig9, (void**) & pFilterConfig);
		hr = FAILED(hr) ? hr : (!pFilterConfig ? E_FAIL : S_OK);
		CHECK_HR(hr, DbgMsg("CVideoSourceEx::Attach: failed to QI IVMRFilterConfig9, hr = 0x%08x", hr));

		DWORD dwVMRMode = 0L;
		hr = pFilterConfig->SetRenderingMode(VMRMode_Renderless);
		hr = pFilterConfig->GetRenderingMode(&dwVMRMode);
		CHECK_HR(hr, DbgMsg("CVideoSourceEx::Attach: failed to get rendering mode, hr = 0x%08x", hr));

		hr = (VMRMode_Renderless != dwVMRMode) ? VFW_E_WRONG_STATE : S_OK;
		CHECK_HR(hr, DbgMsg("CVideoSourceEx::Attach: provided VMR9 is not in renderless mode"));
		m_pGraph = fiVMR.pGraph;

		// check that provided pVMR exposes IVMRSurfaceAllocatorNotify9 interfaces
		hr = m_pVMR->QueryInterface(IID_IVMRSurfaceAllocatorNotify9,
			(void**) & m_pDefaultNotify);
		CHECK_HR(hr, DbgMsg("CVideoSourceEx::Attach: cannot QI IVMRSurfaceAllocatorNotify9"));

		CComPtr<IMediaControl> pMediaControl;
		hr = m_pGraph->QueryInterface(IID_IMediaControl, (void**) & pMediaControl);
		CHECK_HR(hr, DbgMsg("CVideoSourceEx::Attach: cannot QI IMediaControl"));

		OAFilterState state;
		hr = pMediaControl->GetState(100, &state);
		CHECK_HR(hr, DbgMsg("CVideoSourceEx::Attach: failed to get state of IMediaControl, hr = 0x%08x", hr));

		hr = (state != State_Stopped) ? VFW_E_NOT_STOPPED : S_OK;
		CHECK_HR(hr, DbgMsg("CVideoSourceEx::Attach: graph is not stopped, state = %ld", state));

		// we have to be thread safe only here when we actually mess up with shared data
		CAutoDisplayLock displayLock(m_pLock);

		// set device
		CComPtr<IDirect3DDevice9> pDevice;
		hr = m_pVideoSink->Get3DDevice((IUnknown**)&pDevice);
		CHECK_HR(hr, DbgMsg("CVideoSourceEx::Attach: failed to obtain Direct3D device from the video sink, hr = 0x%08x", hr));

		CComPtr<IDirect3D9> pd3d9;
		hr = pDevice->GetDirect3D(&pd3d9);
		CHECK_HR(hr, DbgMsg("CVideoSourceEx::Attach: failed to retrieve IDirect3D9"));

		HMONITOR hMonitor = pd3d9->GetAdapterMonitor(D3DADAPTER_DEFAULT);

		hr = m_pDefaultNotify->SetD3DDevice(pDevice, hMonitor);
		CHECK_HR(hr, DbgMsg("CVideoSourceEx::Attach: failed in SetD3DDevice() of IVMRSurfaceAllocatorNotify, hr = 0x%08x", hr));

		// try to advise 'this' custom allocator-presenter to the VMR
		hr = m_pDefaultNotify->AdviseSurfaceAllocator(GetID(), (IVMRSurfaceAllocator9*) this);
		CHECK_HR(hr, DbgMsg("CVideoSourceEx::Attach: failed to advise A/P, hr = 0x%08x", hr));

		hr = ConfigureVMR(m_pVMR);
		CHECK_HR(hr, DbgMsg("CVideoSourceEx::Attach: failed to ConfigureVMR(), hr = 0x%08x", hr));

		hr = StartPresenting(GetID());
		CHECK_HR(hr, DbgMsg("CVideoSourceEx::Attach: failed in StartPresenting(), hr = 0x%08x", hr));
	}
	catch (HRESULT hrFailed)
	{
		Detach();
		hr = hrFailed;
	}

	SAFE_RELEASE(fiVMR.pGraph);

	return hr;
}

HRESULT CVideoSourceEx::ConfigureVMR(IBaseFilter* pVMR)
{
	HRESULT hr;

	try
	{
		CComPtr<IVMRFilterConfig9> pFilterConfig;
		hr = pVMR->QueryInterface(IID_IVMRFilterConfig9, (void**) & pFilterConfig);
		hr = FAILED(hr) ? hr : (!pFilterConfig ? E_FAIL : S_OK);
		CHECK_HR(hr, DbgMsg("CVideoSourceEx::ConfigureVMR: failed to QI IVMRFilterConfig9, hr = 0x%08x", hr));

		// Force VMR into mixer mode
		DWORD cStreams = 0;
		hr = pFilterConfig->GetNumberOfStreams(&cStreams);
		if (cStreams < 1) 
		{
			hr = pFilterConfig->SetNumberOfStreams(1);
			CHECK_HR(hr, DbgMsg("CVideoSourceEx::ConfigureVMR: failed in SetNumberOfStreams(), hr = 0x%08x", hr));
			hr = pFilterConfig->GetNumberOfStreams(&cStreams);
			CHECK_HR(hr, DbgMsg("CVideoSourceEx::ConfigureVMR: failed in GetNumberOfStreams(), hr = 0x%08x", hr));
		}

		CComQIPtr<IVMRMixerControl9> pMixer = pVMR;

#define MixerPref9_NonSquareMixing      0x00000008
#define MixerPref9_RenderTargetYUV      0x00002000
#define MixerPref9_DynamicSwitchToBOB   0x00100000
#define MixerPref9_DynamicDecimateBy2   0x00200000

		// Use YUV-mixing mode and non-square mixing
		DWORD dwOrgMixerPrefs = 0;
		hr = pMixer->GetMixingPrefs(&dwOrgMixerPrefs);
		CHECK_HR(hr, DbgMsg("CVideoSourceEx::ConfigureVMR: failed to GetMixingPrefs(), hr = 0x%08x", hr));

		DWORD dwNewMixerPrefs = dwOrgMixerPrefs;
		dwNewMixerPrefs &= ~MixerPref9_RenderTargetRGB;
		dwNewMixerPrefs |= (MixerPref9_RenderTargetYUV | MixerPref9_NonSquareMixing);
		hr = pMixer->SetMixingPrefs(dwNewMixerPrefs);
		if (FAILED(hr)) // it may return VFW_E_VMR_NO_DEINTERLACE_HW
		{
			hr = pMixer->SetMixingPrefs(dwOrgMixerPrefs);
			CHECK_HR(hr, DbgMsg("CVideoSourceEx::ConfigureVMR: failed to SetMixingPrefs(), hr = 0x%08x", hr));
		}
	}
	catch (HRESULT hrFailed)
	{
		hr = hrFailed;
	}

	return hr;
}

HRESULT CVideoSourceEx::Detach()
{
	HRESULT hr;

	if (!m_pVideoSink)
	{
		DbgMsg("CVideoSourceEx::Detach: FATAL IDisplayVideoSink pointer is NULL!");
		return E_UNEXPECTED;
	}

	if (!m_pGraph)
	{
		DbgMsg("CVideoSourceEx::Detach: video source info does not contain pointer to IFilterGraph!");
		return VFW_E_NOT_FOUND;
	}

	try
	{
		CComPtr<IMediaControl> pMC;
		hr = m_pGraph->QueryInterface(IID_IMediaControl, (void**) & pMC);
		CHECK_HR(hr, DbgMsg("CVideoSourceEx::Detach: cannot QI IMediaControl of the graph, hr = 0x%08x", hr));

		OAFilterState state;
		hr = pMC->GetState(100, &state);
		CHECK_HR(hr, DbgMsg("CVideoSourceEx::Detach: cannot obtain state from IMediaControl, hr = 0x%08x", hr));

		hr = (State_Stopped != state) ? VFW_E_NOT_STOPPED : S_OK;
		CHECK_HR(hr, DbgMsg("CVideoSourceEx::Detach: correspondent graph was not stopped"));

		hr = !m_pDefaultNotify ? VFW_E_NOT_FOUND : S_OK;
		CHECK_HR(hr, DbgMsg("CVideoSourceEx::Detach: video source info does not contain pointer to IVMRSurfaceAllocatorNotify9"));

		// we have to be thread safe only here when we actually mess up with shared data
		CAutoDisplayLock displayLock(m_pLock);

		hr = StopPresenting(GetID());
		CHECK_HR(hr, DbgMsg("CVideoSourceEx::Detach: failed in StopPresenting(), hr = 0x%08x", hr));

		hr = DisconnectPins();
		CHECK_HR(hr, DbgMsg("CVideoSourceEx::Detach: FATAL, failed to disconnect pins of VMR"));

		// advise NULL as A/P to VMR9 (this will return VMR9 to its default A/P)
		hr = m_pDefaultNotify->AdviseSurfaceAllocator(GetID(), NULL);
		CHECK_HR(hr, DbgMsg("CVideoSourceEx::Detach: failed to unadvise surface allocator, hr = 0x%08x", hr));

		Cleanup();
	}
	catch (HRESULT hrFailed)
	{
		hr = hrFailed;
	}

	return hr;
}

//////////////////////////////////////////////////////////////////////////
// IVMRWindowlessControl9

STDMETHODIMP CVideoSourceEx::SetAspectRatioMode(DWORD dwAspectRatioMode)
{
	if (dwAspectRatioMode == VMR9ARMode_None || dwAspectRatioMode == VMR9ARMode_LetterBox)
		m_dwAspectRatioMode = dwAspectRatioMode;
	else
		return E_UNEXPECTED;

	return S_OK;
}

STDMETHODIMP CVideoSourceEx::GetAspectRatioMode(DWORD*  lpAspectRatioMode)
{
	ASSERT(lpAspectRatioMode);
	*lpAspectRatioMode = m_dwAspectRatioMode;
	return S_OK;
}

STDMETHODIMP CVideoSourceEx::GetMaxIdealVideoSize(LONG*  lpWidth, LONG*  lpHeight)
{
	CHECK_POINTER(lpWidth)
	CHECK_POINTER(lpHeight)

	*lpHeight = m_lImageHeight;

	if (m_szNativeAspectRatio.cx != 0 && m_szNativeAspectRatio.cy != 0)
	{
		*lpWidth = (LONG)ceil((float)m_lImageHeight * (float)m_szNativeAspectRatio.cx / (float)m_szNativeAspectRatio.cy);
	}
	else
		*lpWidth = m_lImageWidth;

	return S_OK;
}

STDMETHODIMP CVideoSourceEx::GetNativeVideoSize(LONG* lpWidth, LONG* lpHeight, LONG* lpARWidth, LONG* lpARHeight)
{
	if(lpWidth == NULL && lpHeight == NULL && lpARWidth == NULL && lpARHeight == NULL)
		return E_POINTER;

	if(lpWidth)
		*lpWidth = m_lImageWidth;
	if(lpHeight)
		*lpHeight = m_lImageHeight;
	if(lpARWidth)
	{
		if (m_fNativeAspectRatio)
			*lpARWidth = (INT)ceil((float)m_lImageHeight * m_fNativeAspectRatio);
		else
			*lpARWidth = m_lImageWidth;
	}
	if(lpARHeight)
		*lpARHeight = m_lImageHeight;
	return S_OK;
}

STDMETHODIMP CVideoSourceEx::GetVideoPosition(LPRECT lpSRCRect, LPRECT lpDSTRect)
{
	if (lpSRCRect)
	{
		lpSRCRect->left = 0;
		lpSRCRect->top = 0;
		lpSRCRect->right = m_lImageWidth;
		lpSRCRect->bottom = m_lImageHeight;
	}

	if (lpDSTRect)
	{
		RECT rcDst = {0};
		GetClientRect(m_hwndVideo, &rcDst);
		lpDSTRect->left = rcDst.left;
		lpDSTRect->top = rcDst.top;
		lpDSTRect->right = rcDst.right;
		lpDSTRect->bottom = rcDst.bottom;
	}

	return S_OK;
}

STDMETHODIMP CVideoSourceEx::SetVideoPosition(const LPRECT lpSRCRect, const LPRECT lpDSTRect)
{
	// One parameter can be NULL, but not both.
	if (lpSRCRect == NULL && lpDSTRect == NULL)
	{
		return E_POINTER;
	}

	HRESULT hr = S_OK;

	if(lpSRCRect)
	{
		// The source rectangle cannot be flipped.
		if ((lpSRCRect->left > lpSRCRect->right) ||
			(lpSRCRect->top > lpSRCRect->bottom))
		{
			return E_INVALIDARG;
		}

		// The source rectangle has range (0,0,m_lImageWidth,m_lImageHeight)
		if ((lpSRCRect->left < 0) || (lpSRCRect->right > m_lImageWidth) ||
			(lpSRCRect->top < 0) || (lpSRCRect->bottom > m_lImageHeight))
		{
			return E_INVALIDARG;
		}

		if (m_pVideoSink)
		{
			CComQIPtr<IDisplayProperties> pProp;
			if (SUCCEEDED(m_pVideoSink->QueryInterface(__uuidof(IDisplayProperties), (void **)&pProp)))
			{
				NORMALIZEDRECT nrcTexture, nrcZoom;
				nrcTexture.left   = (float) lpSRCRect->left/m_lImageWidth;
				nrcTexture.right  = (float) lpSRCRect->right/m_lImageWidth;
				nrcTexture.top    = (float) lpSRCRect->top/m_lImageHeight;
				nrcTexture.bottom = (float) lpSRCRect->bottom/m_lImageHeight;

				nrcZoom.left = nrcTexture.left * 2 - 1.0f;
				nrcZoom.right = nrcTexture.right * 2 - 1.0f;
				nrcZoom.top = 1.0f - nrcTexture.top * 2;
				nrcZoom.bottom = 1.0f - nrcTexture.bottom * 2;
				pProp->SetZoom(&nrcZoom);
			}
		}
	}

	if (lpDSTRect)
	{
		// The destination rectangle cannot be flipped.
		if ((lpDSTRect->left > lpDSTRect->right) ||
			(lpDSTRect->top > lpDSTRect->bottom))
		{
			return E_INVALIDARG;
		}

		// Update the destination rectangle.
		m_dstrect.left = lpDSTRect->left;
		m_dstrect.top = lpDSTRect->top;
		m_dstrect.right = lpDSTRect->right;
		m_dstrect.bottom = lpDSTRect->bottom;
	}

	return hr;
}

STDMETHODIMP CVideoSourceEx::GetCurrentImage(BYTE**  lpDib)
{
	CHECK_POINTER(lpDib)

	HRESULT hr = E_FAIL;
	CComPtr<IDirect3DSurface9> pSurface9;
	if (m_bTextureSurface)
		hr = m_ppSurface[0]->GetContainer(IID_IUnknown, (void **)&pSurface9);
	else
		hr = m_ppSurface[0]->QueryInterface(IID_IUnknown, (void **)&pSurface9);

	if (FAILED(hr))
		return hr;

	if (!pSurface9)
		return E_FAIL;


	D3DSURFACE_DESC desc;
	pSurface9->GetDesc(&desc);

	CComPtr<IDirect3DDevice9> pDevice;
	hr = m_pVideoSink->Get3DDevice((IUnknown**)&pDevice);

	if (FAILED(hr) || !pDevice)
		return E_FAIL;

	CComPtr<IDirect3DTexture9> pARGBTexture9;
	CComPtr<IDirect3DSurface9> pARGBSurface9;

	CComPtr<IDirect3DTexture9> pDestTexture9;
	CComPtr<IDirect3DSurface9> pDestSurface9;

	RECT rectSrc = {0, 0, m_lImageWidth, m_lImageHeight},rectDest = {0};


	D3DTEXTUREFILTERTYPE TexFilterType = D3DTEXF_LINEAR;
	rectDest.bottom = m_lImageHeight;

	if (m_szNativeAspectRatio.cx != 0 && m_szNativeAspectRatio.cy != 0)
		rectDest.right = (LONG)ceil((float)m_lImageHeight * (float)m_szNativeAspectRatio.cx / (float)m_szNativeAspectRatio.cy);
	else
		rectDest.right = m_lImageWidth;

	UINT OutputWidth = rectDest.right, OutputHeight = rectDest.bottom;

	if (memcmp(&rectSrc,&rectDest, sizeof(RECT)) == 0)
		TexFilterType = D3DTEXF_POINT;

	unsigned int w, h;
	w = 2;
	while (w < OutputWidth)
		w <<= 1;

	h = 2;
	while (h < OutputHeight)
		h <<= 1;


	hr = pDevice->CreateTexture(w, h, 0, D3DUSAGE_RENDERTARGET, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &pARGBTexture9, NULL);
	hr = pDevice->CreateTexture(w, h, 0, D3DUSAGE_DYNAMIC, D3DFMT_X8R8G8B8, D3DPOOL_SYSTEMMEM, &pDestTexture9, NULL);
	if (!pARGBTexture9 && !pDestTexture9)
		return E_FAIL;
	hr = pARGBTexture9->GetSurfaceLevel( 0, &pARGBSurface9);
	hr = pDestTexture9->GetSurfaceLevel( 0, &pDestSurface9);

	if (!pARGBSurface9 && !pDestSurface9)
		return E_FAIL;

	hr = pDevice->StretchRect(pSurface9, &rectSrc, pARGBSurface9, &rectDest, TexFilterType);
	if (FAILED(hr))
		return hr;

	hr = pDevice->GetRenderTargetData(pARGBSurface9, pDestSurface9);
	if (FAILED(hr))
		return hr;

	//D3DXSaveSurfaceToFile(_T("c:\\texture.bmp"), D3DXIFF_BMP, pDestSurface9, NULL, &rectDest);

	UINT BmpheaderSize = sizeof(BITMAPINFOHEADER);
	UINT BufSize = BmpheaderSize + (OutputWidth * OutputHeight * 4);
	*lpDib = (BYTE *)CoTaskMemAlloc(BufSize);
	if ((*lpDib) == NULL)
		return E_UNEXPECTED;

	BITMAPINFOHEADER bmpheader;
	ZeroMemory(&bmpheader, BmpheaderSize);
	bmpheader.biSize = BmpheaderSize;
	bmpheader.biWidth = OutputWidth;
	bmpheader.biHeight = OutputHeight;
	bmpheader.biPlanes = 1;
	bmpheader.biBitCount = 32;
	bmpheader.biCompression = BI_RGB;
	bmpheader.biSizeImage = (OutputWidth * OutputHeight * 4);
	memcpy_s(*lpDib, BufSize, &bmpheader, BmpheaderSize);


	D3DLOCKED_RECT LockedTex;
	hr = pDestSurface9->LockRect(&LockedTex, NULL, D3DLOCK_READONLY);
	if (SUCCEEDED(hr))
	{
		BYTE* pDestBuf = (*lpDib) + BmpheaderSize;
		BYTE* pStartBuf = (BYTE *)LockedTex.pBits;
		BYTE* pSourceBuf = pStartBuf;

		for (UINT i = 1;i <= OutputHeight ; i++)
		{
			pSourceBuf = pStartBuf + (LockedTex.Pitch * (OutputHeight - i));
			memcpy(pDestBuf, pSourceBuf, OutputWidth * 4);
			pDestBuf += (OutputWidth * 4);
		}
		pDestSurface9->UnlockRect();
	}

	////Save to File
	//{
	//	HANDLE fh;
	//	DWORD nWritten;
	//	BITMAPFILEHEADER bmpfileheader;
	//	ZeroMemory( &bmpfileheader , sizeof(BITMAPFILEHEADER));
	//	{
	//		bmpfileheader.bfType = ('M' << 8) | 'B';
	//		bmpfileheader.bfSize = sizeof(BITMAPFILEHEADER) + BufSize;
	//		bmpfileheader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	//		fh = CreateFile("c:\\capture.bmp",
	//			GENERIC_WRITE, 0, NULL,
	//			CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	//		WriteFile(fh, &bmpfileheader, sizeof(bmpfileheader), &nWritten, NULL);
	//		WriteFile(fh, *lpDib, BufSize, &nWritten, NULL);
	//		CloseHandle(fh);
	//	}
	//}
	return hr;
}

STDMETHODIMP CVideoSourceEx::SetVideoClippingWindow(HWND  hwnd)
{
	 m_hwndVideo = hwnd; 
	 return S_OK;
}

BOOL CVideoSourceEx::IsFilterStopped()
{
	BOOL bStopped = FALSE;
	if (m_pVMR)
	{
		DWORD dwTimeOut = 0;
		FILTER_STATE State;
		m_pVMR->GetState( dwTimeOut, &State);
		if (State == State_Stopped)
			return TRUE;
		else 
			return FALSE;
	}
	return TRUE; // no filter exists,return stopped
}

HRESULT CVideoSourceEx::CheckMaxDisplayFrameRate()
{
    HRESULT hr = E_FAIL;
    CComPtr<IDispSvrVideoPresenter> pDispSvrVideoPresenter;

    hr = CResourceManager::GetInstance()->GetInterface(DISPSVR_RESOURCE_VIDEOPRESENTER, __uuidof(IDispSvrVideoPresenter), (void**)&pDispSvrVideoPresenter);
    if (SUCCEEDED(hr))
    {
        PresenterCaps Caps = {0};
        Caps.dwSize = sizeof(PresenterCaps);

        Caps.VideoDecodeCaps = VIDEO_CAP_CODEC_MPEG2; //default value
        if (m_lImageHeight > 720) // 1920x1080 and 1440x1080
        {
            Caps.VideoDecodeCaps |= VIDEO_CAP_FORMAT_1080;
        }
        else if (m_lImageHeight > 576) //1280x720
        {
            Caps.VideoDecodeCaps |= VIDEO_CAP_FORMAT_720;
        }
        else if (m_lImageHeight > 480)
        {
            Caps.VideoDecodeCaps |= VIDEO_CAP_FORMAT_576;
        }
        else
        {
            Caps.VideoDecodeCaps |= VIDEO_CAP_FORMAT_480;
        }

        hr = pDispSvrVideoPresenter->QueryCaps(&Caps);
        if (SUCCEEDED(hr))
        {
            if (Caps.bIsOverlay)
            {
                m_dwMaxDisplayFrameRate = Caps.dwFPS;
            }
            else 
            {
                CComPtr<IDispSvrPlugin> pPlugIn;
                CComPtr<IDispSvrVideoMixer> pDispSvrVideoMixer;
                BOOL bIsGeneralD3DMode = TRUE;
                hr = CResourceManager::GetInstance()->GetInterface(DISPSVR_RESOURCE_VIDEOMIXER, __uuidof(IDispSvrVideoMixer), (void**)&pDispSvrVideoMixer);
                if (SUCCEEDED(hr) && pDispSvrVideoMixer)
                {
                    pDispSvrVideoMixer->QueryInterface(__uuidof(IDispSvrPlugin), (VOID **)&pPlugIn);
                    if (pPlugIn)
                    {
                        GUID guidResID;
                        if (SUCCEEDED(pPlugIn->GetResourceId(&guidResID)))
                        {
                            if (IsEqualGUID(guidResID, DISPSVR_RESOURCE_INTELFASTCOMPVIDEOMIXER))
                            {
                                bIsGeneralD3DMode = FALSE;
                            }
                        }
                    }
                }

                if (bIsGeneralD3DMode) // D3D mode, always set 60 fps since we cannot query max flip rate.
                    m_dwMaxDisplayFrameRate = 60;
                else
                    m_dwMaxDisplayFrameRate = Caps.dwFPS;
            }
        }
    }
    return hr;
}

BOOL CVideoSourceEx::IsOverDisplayFrameRate(LONGLONG hnsDuration)
{
    if (hnsDuration < 0)
        return FALSE;

    LONGLONG llThrethold = 0;
    if (m_dwMaxDisplayFrameRate == 30)
    {
        llThrethold = INITIATIVE_RENDERING_SAMPLE_DURATION_THRESHOLD_30FPS;
    }
    else
    {
        llThrethold = INITIATIVE_RENDERING_SAMPLE_DURATION_THRESHOLD_60FPS;
    }

    if (hnsDuration < llThrethold)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

STDMETHODIMP CVideoSourceEx::GetSampleProperty(SampleProperty *pProp)
{
	if (!pProp)
		return E_POINTER;

	HRESULT hr = E_FAIL;
	BOOL bNotStopped = FALSE;
	//m_bImageAvailable becomes FALSE when pause/stop filter.
	//If filter become stopped state(m_bStopState = TRUE)
	//do not check filter state until receiving new image( m_bstopState = FALSE, see presentimage())
	//it is used to avoid getting paused state when filter changing state from stopped to running(stopped-paused-running)
	if (!m_bImageAvailable && !m_bStopState)
	{
		if (IsFilterStopped())
		{
			m_bStopState= TRUE;
			bNotStopped = FALSE;
		}
		else
			bNotStopped = TRUE;
	}
	// Running state: m_bImageAvailable = TRUE
	// Pause state : m_bImageAvailable = FALSE, bNotStopped = TRUE.
	// Stop state : m_bImageAvailable = FALSE, bNotStopped = FALSE.
	// block image on stop state and one kind paused state(see above)
	if (m_bImageAvailable || bNotStopped)
	{
		if (m_dwAspectRatioMode != VMR9ARMode_None && m_szNativeAspectRatio.cy != 0)
			m_fAspectRatio = m_fNativeAspectRatio = (float)m_szNativeAspectRatio.cx / (float)m_szNativeAspectRatio.cy;
		else
			m_fAspectRatio = 0.0;

		pProp->fAspectRatio = m_fAspectRatio;
		pProp->uWidth = m_lImageWidth;
		pProp->uHeight = m_lImageHeight;
		pProp->rtStart = m_rtStart;
		pProp->rtEnd = m_rtEnd;

		hr = m_ppSurface[0]->QueryInterface(IID_IUnknown, (void **)&pProp->pSurface);
	}
	return hr;
}
