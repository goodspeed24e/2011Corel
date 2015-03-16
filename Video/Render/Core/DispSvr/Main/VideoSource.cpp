#include "stdafx.h"
#include "VideoSource.h"
#include "VideoSourceEvr.h"
#include <dvdmedia.h>

/////////////////////// Private class CCVideoSource ///////////////////////////////////////

CVideoSource::CVideoSource(IDisplayLock* pLock, IDisplayVideoSink* pVideoSink) :
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
	m_bTextureSurface = false;
	m_bDecimateBy2 = false;
	m_nrcTexture.left = m_nrcTexture.top = 0.0;
	m_nrcTexture.right = m_nrcTexture.bottom = 1.0;
	m_bInitiativeDisplay = FALSE;
}

CVideoSource::~CVideoSource()
{
	Cleanup();
}

void CVideoSource::Cleanup()
{
	m_pTexturePriv.Release();
	m_pTexturePrivSurf.Release();
	m_pDefaultNotify.Release();
	m_pGraph.Release();
	DeleteSurfaces();
	IBaseFilter *pVMR = m_pVMR.Detach();
	if (pVMR)
	{
		int ref = pVMR->Release();
		if (ref > 0)
		{
			DbgMsg("CVideoSource::Cleanup: m_pVMR ref count = %d, should be 0.\n", ref);
		}
	}

	m_lImageWidth = m_lImageHeight = 0L;
}

STDMETHODIMP CVideoSource::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
    HRESULT hr = E_NOINTERFACE;
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
    else if (riid == IID_IVMRImageCompositor9)
    {
        hr = GetInterface((IVMRImageCompositor9*) this, ppv);
    }
    else
    {
        hr = CUnknown::NonDelegatingQueryInterface(riid, ppv);
    }
    return hr;
}

//////////////////////////////////////////////////////////////////////////
// IDisplayVideoSource

STDMETHODIMP CVideoSource::GetGraph(IFilterGraph** ppGraph)
{
    if (!ppGraph)
    {
        DbgMsg("CVideoSource::GetGraph: ppGraph is NULL");
        return E_POINTER;
    }

    if (m_pGraph)
    {
        m_pGraph.CopyTo(ppGraph);
        return S_OK;
    }

    DbgMsg("CVideoSource::GetGraph: FATAL: contains NULL IFilterGraph pointer");
    return VFW_E_NOT_FOUND;
}

STDMETHODIMP CVideoSource::GetTexture(IUnknown** ppTexture, NORMALIZEDRECT* lpNormRect)
{
	ASSERT(ppTexture);

	if (m_bTextureSurface)
	{
		IDirect3DTexture9 *pTexture;
		if (SUCCEEDED(m_ppSurface[0]->GetContainer(IID_IDirect3DTexture9, (void **)&pTexture)))
			*ppTexture = (IUnknown*)pTexture;
		else
			ASSERT(0);
	}
	else
	{
		m_pTexturePriv.CopyTo((IDirect3DTexture9**)ppTexture);
	}

    if (lpNormRect)
		*lpNormRect = m_nrcTexture;

    return S_OK;
}

STDMETHODIMP CVideoSource::BeginDraw()
{
	return S_OK;
}

STDMETHODIMP CVideoSource::EndDraw()
{
	return S_OK;
}

STDMETHODIMP CVideoSource::IsValid()
{
	return m_bValid ? S_OK : E_FAIL;
}
STDMETHODIMP CVideoSource::ClearImage()
{
	return S_OK;
}

STDMETHODIMP CVideoSource::GetVideoSize(LONG* plWidth, LONG* plHeight, float *pfAspectRatio)
{
	ASSERT(plWidth && plHeight && pfAspectRatio);
    *plWidth = m_lImageWidth;
    *plHeight = m_lImageHeight;
	*pfAspectRatio = m_fApsectRatio;
    return S_OK;
}

HRESULT CVideoSource::BeginDeviceLoss()
{
    CAutoDisplayLock displayLock(m_pLock);

	m_bValid = FALSE;
    DeleteSurfaces();
    m_pTexturePriv.Release();
	m_pTexturePrivSurf.Release();
    m_pTextureComp.Release();
    return S_OK;
}

HRESULT CVideoSource::EndDeviceLoss(IUnknown* pDevice)
{
	// Do NOT hold display lock here because calling ChangeD3DDevice may introduce deadlock!
	// CAutoDisplayLock displayLock(m_pLock);

	HRESULT hr = S_OK;
	if (m_pDefaultNotify)
	{
		DbgMsg("CVideoSource::EndDeviceLoss: calling IVMRSurfaceAllocatorNotify9::ChangeD3DDevice");
		CComQIPtr<IDirect3DDevice9> pDevice9 = pDevice;
		CComPtr<IDirect3D9> pd3d9;
		HMONITOR hMon = NULL;
		if (pd3d9)
			hMon = pd3d9->GetAdapterMonitor(D3DADAPTER_DEFAULT);
		hr = m_pDefaultNotify->ChangeD3DDevice(pDevice9, hMon);
	}
	return hr;
}

HRESULT CVideoSource::EnableInitiativeDisplay(BOOL bEnable)
{
	HRESULT hr = S_OK;
	m_bInitiativeDisplay = bEnable;
	return hr;
}

// Disconnects pins of VMR
HRESULT CVideoSource::DisconnectPins()
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
        CHECK_HR(hr, DbgMsg("CVideoSource::DisconnectPins: failed to enumerate pins, hr = 0x%08x", hr));

        CComPtr<IPin> pPin;
        hr = pEnum->Next(1, &pPin, NULL);
        while (S_OK == hr && pPin)
        {
            hr = pPin->Disconnect();
            CHECK_HR(hr, DbgMsg("CVideoSource::DisconnectPins: failed to disconnect pin, hr = 0x%08x", hr));

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
void CVideoSource::DeleteSurfaces()
{
    if (m_ppSurface)
    {
        for (DWORD dwS = 0; dwS < m_dwNumBuf; dwS++)
        {
            if (m_ppSurface[dwS])
            {
                (m_ppSurface[dwS])->Release();
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
HRESULT CVideoSource::AllocateSurfaceBuffer(DWORD dwN)
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
  * CVideoSource), so we do not have to tell A/P about VMR's notifier.
  */
STDMETHODIMP CVideoSource::AdviseNotify(IVMRSurfaceAllocatorNotify9* lpIVMRSurfAllocNotify)
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
STDMETHODIMP CVideoSource::InitializeDevice(
    DWORD_PTR dwUserID,
    VMR9AllocationInfo* lpAllocInfo,
    DWORD* lpNumBuffers)
{
    HRESULT hr = S_OK;
    D3DFORMAT format;
    D3DSURFACE_DESC ddsd;
    D3DCAPS9 ddcaps;

    CComPtr<IUnknown> pDevice;

    // check we are provided valid parameters
    if (!lpAllocInfo || !lpNumBuffers)
        return E_POINTER;

    if (*lpNumBuffers < 1)
        *lpNumBuffers = 1;

    // check we know about the default IVMRSurfaceAllocatorNotify9
    if (!m_pDefaultNotify)
    {
        DbgMsg("CVideoSource::InitializeDevice: FATAL: video source contains NULL pointer to IVMRSurfaceAllocatorNotify9");
        return E_FAIL;
    }

    if (FAILED(VerifyID(dwUserID)))
    {
        DbgMsg("Failed to VerifyID()!");
        return VFW_E_NOT_FOUND;
    }

	try
	{
		// we have to be thread safe only here when we actually mess up with shared data
		CAutoDisplayLock displayLock(m_pLock);

		// obtain device from the video sink
		hr = m_pVideoSink->Get3DDevice(&pDevice);
		CComQIPtr<IDirect3DDevice9> pDevice9 = pDevice;
		CHECK_HR(hr, DbgMsg("CVideoSource::InitializeDevice: failed to get Direct3D device from the video sink, hr = 0x%08x, pDevice = 0x%08x", hr, pDevice));

		m_pTexturePriv.Release();
		m_pTexturePrivSurf.Release();
		DeleteSurfaces();

		// allocate surface buffer
		hr = AllocateSurfaceBuffer(*lpNumBuffers);
		CHECK_HR(hr, DbgMsg("CVideoSource::InitializeDevice: failed to allocate surface buffer, hr = 0x%08x, dwBuffers = %ld", hr, *lpNumBuffers));

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
			hr = m_pDefaultNotify->AllocateSurfaceHelper(
					 lpAllocInfo,
					 lpNumBuffers,
					 m_ppSurface);
			CHECK_HR(hr, DbgMsg("CVideoSource::InitializeDevice: failed in IVMRSurfaceAllocatorNotify9::AllocateSurfaceHelper, hr = 0x%08x, dwBuffers = %ld", hr, *lpNumBuffers));

			// here we are creating private texture to be used by the render engine
			if (lpAllocInfo->Format > '0000') // surface is YUV.
			{
				D3DDISPLAYMODE dm;
				pDevice9->GetDisplayMode(0, &dm);
				format = dm.Format;
			}
			else // RGB: use the same as original
			{
				format = lpAllocInfo->Format;
			}

			// first, check if we have to comply with pow2 requirement
			ZeroMemory(&ddcaps, sizeof(D3DCAPS9));
			hr = pDevice9->GetDeviceCaps(&ddcaps);
			CHECK_HR(hr, DbgMsg("CVideoSource::InitializeDevice: failed to get device caps"));

			UINT width = lpAllocInfo->dwWidth;
			UINT height = lpAllocInfo->dwHeight;

			CComQIPtr<IVMRMixerControl9> pMixer = m_pVMR;
			if (pMixer)
			{
				DWORD dwMixerPrefs = 0;
				hr = pMixer->GetMixingPrefs(&dwMixerPrefs);
				// MixerPref9_DynamicDecimateBy2 = 0x00200000
				if (SUCCEEDED(hr) && (dwMixerPrefs & 0x00200000))
				{
					m_bDecimateBy2 = true;
					width >>= 1;
					height >>= 1;
				}
			}

			if ((ddcaps.TextureCaps & D3DPTEXTURECAPS_POW2) != 0 &&
				(ddcaps.TextureCaps & D3DPTEXTURECAPS_NONPOW2CONDITIONAL) == 0)
			{
				unsigned int pow2;
				for (pow2 = 2; pow2 < width; pow2 <<= 1)
					;

				width = pow2;
				for (pow2 = 2; pow2 < height; pow2 <<= 1)
					;
				height = pow2;
			}

			hr = pDevice9->CreateTexture(width, height, 1, D3DUSAGE_RENDERTARGET, format, D3DPOOL_DEFAULT, &m_pTexturePriv, NULL);
			CHECK_HR(hr, DbgMsg("CVideoSource::InitializeDevice: failed to create private texture, hr = 0x%08x", hr));

			hr = m_pTexturePriv->GetSurfaceLevel(0, &m_pTexturePrivSurf);
			CHECK_HR(hr, DbgMsg("CVideoSource::InitializeDevice: failed to get 0-level surface of the private texture, hr = 0x%08x", hr));

			hr = pDevice9->ColorFill(m_pTexturePrivSurf, NULL, D3DCOLOR_XRGB(0x00, 0x00, 0x00));
			CHECK_HR(hr, DbgMsg("CVideoSource::InitializeDevice: failed to fill the surface of the private texture with solid color, hr = 0x%08x", hr));

			hr = m_pTexturePriv->GetLevelDesc(0, &ddsd);
			CHECK_HR(hr, DbgMsg("CVideoSource::InitializeDevice: failed to obtain surface description of the private texture, hr = 0x%08x", hr));
		}

		m_dwNumBufActuallyAllocated = *lpNumBuffers;

		m_lImageWidth = lpAllocInfo->dwWidth;
		m_lImageHeight = lpAllocInfo->dwHeight;

		if (lpAllocInfo->szAspectRatio.cy > 0)
			m_fApsectRatio = (float) lpAllocInfo->szAspectRatio.cx/lpAllocInfo->szAspectRatio.cy;
		else
			m_fApsectRatio = (float) lpAllocInfo->dwWidth / lpAllocInfo->dwHeight;

        // tell the mixer we are "on"
        hr = m_pVideoSink->OnVideoSourceAdded(
                 this,
				 m_fApsectRatio);
        CHECK_HR(hr, DbgMsg("CVideoSource::InitializeDevice: failed in IDisplayVideoSink::AddVideoSource(), hr = 0x%08x", hr));
    }
    catch (HRESULT hrFailed)
    {
        hr = hrFailed;
        m_pTexturePriv.Release();
		m_pTexturePrivSurf.Release();
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
STDMETHODIMP CVideoSource::TerminateDevice(DWORD_PTR dwUserID)
{
	if (FAILED(VerifyID(dwUserID)))
	{
		DbgMsg("Failed to VerifyID()!");
		return VFW_E_NOT_FOUND;
	}

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
STDMETHODIMP CVideoSource::GetSurface(
    DWORD_PTR dwUserID,
    DWORD SurfaceIndex,
    DWORD SurfaceFlags,
    IDirect3DSurface9** lplpSurface)
{
    // check for NULL pointers
    if (!lplpSurface)
    {
        DbgMsg("CVideoSource::GetSurface: fourth argument is NULL");
        return E_POINTER;
    }

    if (FAILED(VerifyID(dwUserID)))
    {
        DbgMsg("Failed to VerifyID()!");
        return VFW_E_NOT_FOUND;
    }

    CAutoDisplayLock displayLock(m_pLock);

    // check that requested index does not exceed number of actually allocated buffers
    if (SurfaceIndex >= m_dwNumBufActuallyAllocated)
    {
        DbgMsg("CVideoSource::GetSurface: requested surface index %ld falls out of "\
                 "valid range [0, %ld]", SurfaceIndex, m_dwNumBufActuallyAllocated);
        return E_INVALIDARG;
    }

    // check that requested surface is not null
    if (NULL == m_ppSurface[ SurfaceIndex ])
    {
        DbgMsg("CVideoSource::GetSurface: FATAL, requested surface of index %ld is NULL!",
                 SurfaceIndex);
        return E_UNEXPECTED;
    }

    // now we checked everything and can copy
    *lplpSurface = m_ppSurface[ SurfaceIndex ];
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
STDMETHODIMP CVideoSource::StartPresenting(DWORD_PTR dwUserID)
{
    CAutoDisplayLock displayLock(m_pLock);

    if (FAILED(VerifyID(dwUserID)))
    {
        DbgMsg("Failed to VerifyID()!");
        return VFW_E_NOT_FOUND;
    }

    if (!m_pVideoSink)
    {
        DbgMsg("CVideoSource::StartPresenting: FATAL, video sink is NULL");
        return E_UNEXPECTED;
    }

    CComPtr<IDirect3DDevice9> pDevice;
    HRESULT hr = m_pVideoSink->Get3DDevice((IUnknown**)&pDevice);
    if (FAILED(hr) || !pDevice)
    {
        DbgMsg("CVideoSource::StartPresenting: FATAL, Direct3DDevice in the video sink is NULL, "\
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
STDMETHODIMP CVideoSource::StopPresenting(DWORD_PTR dwUserID)
{
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
STDMETHODIMP CVideoSource::PresentImage(DWORD_PTR dwUserID, VMR9PresentationInfo* lpPresInfo)
{
    HRESULT hr = S_OK;
    CComPtr<IDirect3DDevice9> pSampleDevice;

    // first, check for NULLs
    if (!lpPresInfo || !(lpPresInfo->lpSurf))
    {
        DbgMsg("CVideoSource::PresentImage: received NULL pointer");
        return E_POINTER;
    }

    if (FAILED(VerifyID(dwUserID)))
    {
        DbgMsg("Failed to VerifyID()!");
        return VFW_E_NOT_FOUND;
    }

    try
    {
		// this is important to be thread safe here
		CAutoDisplayLock displayLock(m_pLock);

		if (lpPresInfo->szAspectRatio.cy)
			m_fApsectRatio = static_cast<float> (lpPresInfo->szAspectRatio.cx) / lpPresInfo->szAspectRatio.cy;

		if (!m_bTextureSurface)
		{
			if (!m_pTexturePrivSurf)
			{
				DbgMsg("CVideoSource::PresentImage: FATAL, private texture of the source %ld is NULL", dwUserID);
				return E_UNEXPECTED;
			}
			// now, get the device of the sample passed in (it is not necessarily the same
			// device we created in the render engine
			hr = lpPresInfo->lpSurf->GetDevice(&pSampleDevice);
			CHECK_HR(hr, DbgMsg("CVideoSource::PresentImage: failed to get the device of the surface passed in, hr = 0x%08x, pSampleDevice = 0x%08x", hr, pSampleDevice));

			if (lpPresInfo->dwFlags & 0x00000010)
			{
				hr = pSampleDevice->StretchRect(lpPresInfo->lpSurf, &lpPresInfo->rcSrc, m_pTexturePrivSurf, &lpPresInfo->rcDst, D3DTEXF_POINT);
			}
			else
			{
				hr = pSampleDevice->StretchRect(lpPresInfo->lpSurf, NULL, m_pTexturePrivSurf, NULL, D3DTEXF_POINT);
			}

			CHECK_HR(hr, DbgMsg("CVideoSource::PresentImage: StretchRect() to private texture failed, hr = 0x%08x", hr));
		}

		if (0)
		{
#ifndef _NO_USE_D3DXDLL
			if (lpPresInfo->lpSurf)
				hr = D3DXSaveSurfaceToFile(_T("C:\\reRT.bmp"), D3DXIFF_BMP, lpPresInfo->lpSurf, NULL, NULL);
			if (m_pTexturePrivSurf)
				hr = D3DXSaveSurfaceToFile(_T("C:\\reRT.bmp"), D3DXIFF_BMP, m_pTexturePrivSurf, NULL, NULL);
#endif
		}
    }
    catch (HRESULT hrFailed)
    {
        return hrFailed;
    }

	if (m_bInitiativeDisplay)
	{
		// Force render new frame after PresentImage() is called to prevent drop frame.
		CComQIPtr<IDisplayObject> pObj = m_pVideoSink;
		if (pObj)
		{
			CComPtr<IDisplayRenderEngine> pRenderEngine;
			if (SUCCEEDED(pObj->GetRenderEngineOwner(&pRenderEngine)) && pRenderEngine)
			{
#ifdef _DEBUG
				static DWORD dwLastTS = timeGetTime();
				DWORD dwCurrentTS = timeGetTime();
				DWORD dwDiff = dwCurrentTS - dwLastTS;
				dwLastTS = dwCurrentTS;
#endif
				hr = pRenderEngine->NodeRequest(DISPLAY_REQUEST_Render, 0, 0, pObj);
				if (SUCCEEDED(hr))
				{
					hr = pRenderEngine->NodeRequest(DISPLAY_REQUEST_Present, 0, 0, pObj);
				}
#ifdef _DEBUG
				DWORD dwDuration = timeGetTime() - dwCurrentTS;
				DbgMsg("CVideoSource::PresentImage: Interval: %ld, Duration: %ld", dwDiff, dwDuration);
#endif
			}
		}
	}

	return hr;
}

HRESULT CVideoSource::Attach(IBaseFilter* pVMR)
{
    HRESULT hr = S_OK;

    FILTER_INFO fiVMR;
    ZeroMemory(&fiVMR, sizeof(fiVMR));

    try
    {
        // check that provided VMR is part of the graph
		m_pVMR = pVMR;
        hr = m_pVMR->QueryFilterInfo(&fiVMR);
        hr = (NULL == fiVMR.pGraph) ? E_FAIL : S_OK;
        CHECK_HR(hr, DbgMsg("CVideoSource::Attach: provided VMR was not added to the graph"));

        // check that provided VMR is in renderless mode
        CComPtr<IVMRFilterConfig9> pFilterConfig;
        hr = m_pVMR->QueryInterface(IID_IVMRFilterConfig9, (void**) & pFilterConfig);
        hr = FAILED(hr) ? hr : (!pFilterConfig ? E_FAIL : S_OK);
        CHECK_HR(hr, DbgMsg("CVideoSource::Attach: failed to QI IVMRFilterConfig9, hr = 0x%08x", hr));

        DWORD dwVMRMode = 0L;
        hr = pFilterConfig->SetRenderingMode(VMRMode_Renderless);
        hr = pFilterConfig->GetRenderingMode(&dwVMRMode);
        CHECK_HR(hr, DbgMsg("CVideoSource::Attach: failed to get rendering mode, hr = 0x%08x", hr));

        hr = (VMRMode_Renderless != dwVMRMode) ? VFW_E_WRONG_STATE : S_OK;
        CHECK_HR(hr, DbgMsg("CVideoSource::Attach: provided VMR9 is not in renderless mode"));
        m_pGraph = fiVMR.pGraph;

        // check that provided pVMR exposes IVMRSurfaceAllocatorNotify9 interfaces
        hr = m_pVMR->QueryInterface(IID_IVMRSurfaceAllocatorNotify9,
                                    (void**) & m_pDefaultNotify);
        CHECK_HR(hr, DbgMsg("CVideoSource::Attach: cannot QI IVMRSurfaceAllocatorNotify9"));

        CComPtr<IMediaControl> pMediaControl;
        hr = m_pGraph->QueryInterface(IID_IMediaControl, (void**) & pMediaControl);
        CHECK_HR(hr, DbgMsg("CVideoSource::Attach: cannot QI IMediaControl"));

        OAFilterState state;
        hr = pMediaControl->GetState(100, &state);
        CHECK_HR(hr, DbgMsg("CVideoSource::Attach: failed to get state of IMediaControl, hr = 0x%08x", hr));

        hr = (state != State_Stopped) ? VFW_E_NOT_STOPPED : S_OK;
        CHECK_HR(hr, DbgMsg("CVideoSource::Attach: graph is not stopped, state = %ld", state));

        // we have to be thread safe only here when we actually mess up with shared data
        CAutoDisplayLock displayLock(m_pLock);

        // set device
        CComPtr<IDirect3DDevice9> pDevice;
        hr = m_pVideoSink->Get3DDevice((IUnknown**)&pDevice);
        CHECK_HR(hr, DbgMsg("CVideoSource::Attach: failed to obtain Direct3D device from the video sink, hr = 0x%08x", hr));

        CComPtr<IDirect3D9> pd3d9;
        hr = pDevice->GetDirect3D(&pd3d9);
        CHECK_HR(hr, DbgMsg("CVideoSource::Attach: failed to retrieve IDirect3D9"));

        HMONITOR hMonitor = pd3d9->GetAdapterMonitor(D3DADAPTER_DEFAULT);

        hr = m_pDefaultNotify->SetD3DDevice(pDevice, hMonitor);
        CHECK_HR(hr, DbgMsg("CVideoSource::Attach: failed in SetD3DDevice() of IVMRSurfaceAllocatorNotify, hr = 0x%08x", hr));

        // try to advise 'this' custom allocator-presenter to the VMR
        hr = m_pDefaultNotify->AdviseSurfaceAllocator(GetID(), (IVMRSurfaceAllocator9*) this);
        CHECK_HR(hr, DbgMsg("CVideoSource::Attach: failed to advise A/P, hr = 0x%08x", hr));

        hr = ConfigureVMR(m_pVMR);
        CHECK_HR(hr, DbgMsg("CVideoSource::Attach: failed to ConfigureVMR(), hr = 0x%08x", hr));

        hr = StartPresenting(GetID());
        CHECK_HR(hr, DbgMsg("CVideoSource::Attach: failed in StartPresenting(), hr = 0x%08x", hr));
    }
    catch (HRESULT hrFailed)
    {
        Detach();
        hr = hrFailed;
    }

    SAFE_RELEASE(fiVMR.pGraph);

    return hr;
}

HRESULT CVideoSource::ConfigureVMR(IBaseFilter* pVMR)
{
    HRESULT hr;

    try
    {
        CComPtr<IVMRFilterConfig9> pFilterConfig;
        hr = pVMR->QueryInterface(IID_IVMRFilterConfig9, (void**) & pFilterConfig);
        hr = FAILED(hr) ? hr : (!pFilterConfig ? E_FAIL : S_OK);
        CHECK_HR(hr, DbgMsg("CVideoSource::ConfigureVMR: failed to QI IVMRFilterConfig9, hr = 0x%08x", hr));
        
		// Force VMR into mixer mode
		DWORD cStreams = 0;
		hr = pFilterConfig->GetNumberOfStreams(&cStreams);
		if (cStreams < 1) 
		{
			hr = pFilterConfig->SetNumberOfStreams(1);
			CHECK_HR(hr, DbgMsg("CVideoSource::ConfigureVMR: failed in SetNumberOfStreams(), hr = 0x%08x", hr));
			hr = pFilterConfig->GetNumberOfStreams(&cStreams);
			CHECK_HR(hr, DbgMsg("CVideoSource::ConfigureVMR: failed in GetNumberOfStreams(), hr = 0x%08x", hr));
		}

		CComQIPtr<IVMRMixerControl9> pMixer = pVMR;

#define MixerPref9_NonSquareMixing      0x00000008
#define MixerPref9_RenderTargetYUV      0x00002000
#define MixerPref9_DynamicSwitchToBOB   0x00100000
#define MixerPref9_DynamicDecimateBy2   0x00200000

		// Use YUV-mixing mode and non-square mixing
		DWORD dwOrgMixerPrefs = 0;
		hr = pMixer->GetMixingPrefs(&dwOrgMixerPrefs);
        CHECK_HR(hr, DbgMsg("CVideoSource::ConfigureVMR: failed to GetMixingPrefs(), hr = 0x%08x", hr));

        DWORD dwNewMixerPrefs = dwOrgMixerPrefs;
		dwNewMixerPrefs &= ~MixerPref9_RenderTargetRGB;
		dwNewMixerPrefs |= (MixerPref9_RenderTargetYUV | MixerPref9_NonSquareMixing);
		hr = pMixer->SetMixingPrefs(dwNewMixerPrefs);
        if (FAILED(hr)) // it may return VFW_E_VMR_NO_DEINTERLACE_HW
        {
            hr = pMixer->SetMixingPrefs(dwOrgMixerPrefs);
            CHECK_HR(hr, DbgMsg("CVideoSource::ConfigureVMR: failed to SetMixingPrefs(), hr = 0x%08x", hr));
        }
    }
    catch (HRESULT hrFailed)
    {
        hr = hrFailed;
    }

    return hr;
}

HRESULT CVideoSource::Detach()
{
    HRESULT hr = S_OK;

    if (!m_pVideoSink)
    {
        DbgMsg("CVideoSource::Detach: FATAL IDisplayVideoSink pointer is NULL!");
        return E_UNEXPECTED;
    }

    if (!m_pGraph)
    {
        DbgMsg("CVideoSource::Detach: video source info does not contain pointer to IFilterGraph!");
        return VFW_E_NOT_FOUND;
    }

    try
    {
        CComPtr<IMediaControl> pMC;
        hr = m_pGraph->QueryInterface(
                 IID_IMediaControl, (void**) & pMC);
        CHECK_HR(hr, DbgMsg("CVideoSource::Detach: cannot QI IMediaControl of the graph, hr = 0x%08x", hr));

        OAFilterState state;
        hr = pMC->GetState(100, &state);
        CHECK_HR(hr, DbgMsg("CVideoSource::Detach: cannot obtain state from IMediaControl, hr = 0x%08x", hr));

        hr = (State_Stopped != state) ? VFW_E_NOT_STOPPED : S_OK;
        CHECK_HR(hr, DbgMsg("CVideoSource::Detach: correspondent graph was not stopped"));

        hr = !m_pDefaultNotify ? VFW_E_NOT_FOUND : S_OK;
        CHECK_HR(hr, DbgMsg("CVideoSource::Detach: video source info does not contain pointer to IVMRSurfaceAllocatorNotify9"));

        // we have to be thread safe only here when we actually mess up with shared data
        CAutoDisplayLock displayLock(m_pLock);

        hr = StopPresenting(GetID());
        CHECK_HR(hr, DbgMsg("CVideoSource::Detach: failed in StopPresenting(), hr = 0x%08x", hr));

        hr = DisconnectPins();
        CHECK_HR(hr, DbgMsg("CVideoSource::Detach: FATAL, failed to disconnect pins of VMR"));

        // advise NULL as A/P to VMR9 (this will return VMR9 to its default A/P)
        hr = m_pDefaultNotify->AdviseSurfaceAllocator(GetID(), NULL);
        CHECK_HR(hr, DbgMsg("CVideoSource::Detach: failed to unadvise surface allocator, hr = 0x%08x", hr));

		Cleanup();
    }
    catch (HRESULT hrFailed)
    {
        hr = hrFailed;
    }

    return hr;
}

//////////////////////////////////////////////////////////////////////////
// IVMRImageCompositor9

STDMETHODIMP CVideoSource::InitCompositionDevice(
    /* [in] */ IUnknown *pD3DDevice)
{
    return S_OK;
}

STDMETHODIMP CVideoSource::TermCompositionDevice(
    /* [in] */ IUnknown *pD3DDevice)
{
    CAutoDisplayLock displayLock(m_pLock);

    m_pTextureComp.Release();

    return S_OK;
}

STDMETHODIMP CVideoSource::SetStreamMediaType(
    /* [in] */ DWORD dwStrmID,
    /* [in] */ AM_MEDIA_TYPE *pmt,
    /* [in] */ BOOL fTexture)
{
    return S_OK;
}

// custom vertex
struct MixerVertex
{
    D3DVECTOR position;
    D3DCOLOR color;
    FLOAT tu;
    FLOAT tv;

    enum
    {
        FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1
    };
};

static HRESULT UpdateDestination(MixerVertex* v, VMR9VideoStreamInfo* si, float tw, float th)
{
    // coordinates in composition space [-1,1]x[1,-1]
    float cl; // left
    float ct; // top
    float cr; // right
    float cb; // bottom

    // update composition space coordinates
    // comp_space_x = 2. * norm_space_x - 1.;
    // comp_space_x = 1. - 2. * norm_space_y;
    cl = 2.f * si->rNormal.left - 1.f;
    cr = 2.f * si->rNormal.right - 1.f;
    ct = 1.f - 2.f * si->rNormal.top;
    cb = 1.f - 2.f * si->rNormal.bottom;

    BYTE alpha = (BYTE)(si->fAlpha * 255.0f);
    D3DCOLOR color = D3DCOLOR_RGBA(0xff, 0xff, 0xff, alpha);

    v[0].position.x = cl;
    v[0].position.y = ct;
    v[0].position.z = 0.5f;
    v[0].color = color;
    v[0].tu = 0.f;
    v[0].tv = 0.f;

    v[1].position.x = cr;
    v[1].position.y = ct;
    v[1].position.z = 0.5f;
    v[1].color = color;
    v[1].tu = tw;
    v[1].tv = 0.f;

    v[2].position.x = cl;
    v[2].position.y = cb;
    v[2].position.z = 0.5f;
    v[2].color = color;
    v[2].tu = 0.f;
    v[2].tv = th;

    v[3].position.x = cr;
    v[3].position.y = cb;
    v[3].position.z = 0.5f;
    v[3].color = color;
    v[3].tu = tw;
    v[3].tv = th;

    return S_OK;
}

HRESULT CVideoSource::PrepareCompositorTexture(IDirect3DDevice9* pDevice, DWORD width, DWORD height)
{
    try
    {
        HRESULT hr;

        bool bCreateNewTexture = true;
        if (m_pTextureComp)
        {
            CComPtr<IDirect3DSurface9> pSurf;
            hr = m_pTextureComp->GetSurfaceLevel(0, &pSurf);

            D3DSURFACE_DESC desc;
            hr = pSurf->GetDesc(&desc);

            if (desc.Width >= width && desc.Height >= height)
            {
                bCreateNewTexture = false;
            }
        }

        if (bCreateNewTexture)
        {
            m_pTextureComp.Release();

            D3DCAPS9 ddcaps;
            ZeroMemory(&ddcaps, sizeof(D3DCAPS9));
            hr = pDevice->GetDeviceCaps(&ddcaps);

            UINT w;
            UINT h;
            if ((ddcaps.TextureCaps & D3DPTEXTURECAPS_POW2) != 0 &&
                (ddcaps.TextureCaps & D3DPTEXTURECAPS_NONPOW2CONDITIONAL) == 0) // have to make it pow2 :-(
            {
                w = 2;
                while (w < width)
                    w = w << 1;

                h = 2;
                while (h < height)
                    h = h << 1;
            }
            else
            {
                w = width;
                h = height;
            }

            D3DDISPLAYMODE dm;
            hr = pDevice->GetDisplayMode(0, &dm);

            // Note that D3DUSAGE_RENDERTARGET and D3DPOOL_DEFAULT is necessary
            hr = pDevice->CreateTexture(
                      w, h,
                      1,                         // levels
                      D3DUSAGE_RENDERTARGET,     // usage
                      dm.Format,                 // format
                      D3DPOOL_DEFAULT,
                      &m_pTextureComp,
                      NULL);
        }
    }
    catch (HRESULT hrFailed)
    {
        DbgMsg("CVideoSource::PrepareCompositorTexture: failed, hr = 0x%08x", hrFailed);
        return hrFailed;
    }

    return m_pTextureComp ? S_OK : E_FAIL;
}

STDMETHODIMP CVideoSource::CompositeImage(
    /* [in] */ IUnknown *pD3DDevice,
    /* [in] */ IDirect3DSurface9 *pddsRenderTarget,
    /* [in] */ AM_MEDIA_TYPE *pmtRenderTarget,
    /* [in] */ REFERENCE_TIME rtStart,
    /* [in] */ REFERENCE_TIME rtEnd,
    /* [in] */ D3DCOLOR dwClrBkGnd,
    /* [in] */ VMR9VideoStreamInfo *pVideoStreamInfo,
    /* [in] */ UINT cStreams)
{
    try
    {
        // There are one CWizard::RenderThread and multiple VMR9::CVideoMixer::MixerThreadProc
        // sharing the same Direct3D device. We need to synchronize them to prevent race condition.
        CAutoDisplayLock displayLock(m_pLock);

        HRESULT hr;

        CComQIPtr<IDirect3DDevice9> pDevice = pD3DDevice;

        hr = pDevice->SetRenderTarget(0, pddsRenderTarget);

        hr = pDevice->SetFVF(MixerVertex::FVF);

        hr = pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        hr = pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        hr = pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        hr = pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
        hr = pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
        hr = pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

        for (UINT i = 0; i < cStreams; i++)
        {
            VMR9VideoStreamInfo* si = pVideoStreamInfo + i;

            D3DSURFACE_DESC descSrc;
            hr = si->pddsVideoSurface->GetDesc(&descSrc);

            hr = PrepareCompositorTexture(pDevice, descSrc.Width, descSrc.Height);

            hr = pDevice->SetTexture(0, m_pTextureComp);

            // Get texture level surface
            CComPtr<IDirect3DSurface9> pTextureSurf;
            hr = m_pTextureComp->GetSurfaceLevel(0, &pTextureSurf);
            D3DSURFACE_DESC descDest;
            hr = pTextureSurf->GetDesc(&descDest);

            // Copy the stream image to texture surface. Note that there is no stretching.
            RECT rect = { 0, 0, descSrc.Width, descSrc.Height };
            hr = pDevice->StretchRect(si->pddsVideoSurface, &rect, pTextureSurf, &rect, D3DTEXF_NONE);

            // Draw to the render target
            MixerVertex vertices[4];
            hr = UpdateDestination(vertices, si, (float)descSrc.Width / descDest.Width, (float)descSrc.Height / descDest.Height);
            hr = pDevice->DrawPrimitiveUP(
                      D3DPT_TRIANGLESTRIP,
                      2,
                      vertices,
                      sizeof(vertices[0]));
        }

        hr = pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

#ifdef _DEBUG

        CComPtr<IDirect3DSurface9> pRT;
        hr = pDevice->GetRenderTarget(0, &pRT);
        if (pRT != pddsRenderTarget)
        {
            D3DSURFACE_DESC descTarget;
            hr = pRT->GetDesc(&descTarget);

            DbgMsg("Unexpected render target: usage = 0x%08x, format = 0x%08x, pool = 0x%08x, width = 0x%08x, height = 0x%08x",
                     descTarget.Usage,
                     descTarget.Format,
                     descTarget.Pool,
                     descTarget.Width,
                     descTarget.Height);
        }
#endif

    }
    catch (HRESULT hrFailed)
    {
        DbgMsg("CVideoSource::CompositeImage: failed, hr = 0x%08x", hrFailed);
        return hrFailed;
    }

    return S_OK;
}
