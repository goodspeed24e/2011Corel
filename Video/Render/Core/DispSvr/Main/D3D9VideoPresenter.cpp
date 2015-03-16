#include "Imports/ThirdParty/Microsoft/DXVAHD/d3d9.h"
#include "Imports/ThirdParty/Microsoft/DXVAHD/d3d9types.h"
#include "stdafx.h"
#include "DispSvr.h"
#include "ResourceManager.h"
#include "DynLibManager.h"
#include "D3D9VideoPresenter.h"
#include "Imports/LibGPU/GPUID.h"
#include "RegistryService.h"
#include "DriverExtensionHelper.h"

using namespace DispSvr;

extern HRESULT EnableScreenCaptureDefense(IDirect3DDevice9 *pDevice, BOOL bEnableProtection);

CD3D9VideoPresenter::CD3D9VideoPresenter()
{
	m_GUID = DISPSVR_RESOURCE_D3DVIDEOPRESENTER;
	m_bNeedEnableDwmQueuing = false;
	m_pDeviceEx = NULL;
	m_pSwapChain = NULL;
	m_bDwmEnableMMCSS = false;
	m_PresenterCaps.bCanVirtualizeFromOrigin = TRUE;
    m_pDriverExtAdapter = NULL;
	m_ePresentMode = D3D9_PRESENT_MODE_WINDOWED;
}

CD3D9VideoPresenter::~CD3D9VideoPresenter()
{
}

STDMETHODIMP CD3D9VideoPresenter::_SetDevice(IUnknown *pDevice)
{
	HRESULT hr = CD3D9VideoPresenterBase::_SetDevice(pDevice);
    if (SUCCEEDED(hr))
    {
        ASSERT(m_pDriverExtAdapter == 0);
        // just leave m_pDriverExtAdapter = 0 if failure.
        CDriverExtensionHelper::GetAdapter(m_pDevice, &m_pDriverExtAdapter);
    }
	
    BOOL bDWMEnable = FALSE;
	if(CDynLibManager::GetInstance()->pfnDwmIsCompositionEnabled)
	{
		CDynLibManager::GetInstance()->pfnDwmIsCompositionEnabled(&bDWMEnable);
		m_bNeedEnableDwmQueuing = bDWMEnable ? true : false;
	}

	if (SUCCEEDED(hr))
	{
		if (bDWMEnable && CDynLibManager::GetInstance()->pfnDwmEnableMMCSS)
		{
			hr = CDynLibManager::GetInstance()->pfnDwmEnableMMCSS(TRUE);
			if( hr == S_OK ) m_bDwmEnableMMCSS = true;
		}

		// should there be any device lost, the created additional swapchain is released.
		// and the user will be responsible to recreate an additonal swapchain if desired.
		hr = m_pDevice->GetSwapChain(0, &m_pSwapChain);

		m_ePresentMode = D3D9_PRESENT_MODE_WINDOWED;
		if (m_pSwapChain)
		{
			D3DPRESENT_PARAMETERS sPresentParam;
			if (SUCCEEDED(m_pSwapChain->GetPresentParameters(&sPresentParam)))
			{
				if (sPresentParam.Windowed == FALSE)
					m_ePresentMode = D3D9_PRESENT_MODE_FULLSCREEN;
				else if (sPresentParam.SwapEffect == D3DSWAPEFFECT_FLIPEX)
					m_ePresentMode = D3D9_PRESENT_MODE_FLIPEX;
			}
		}
	}

	return hr;
}

STDMETHODIMP CD3D9VideoPresenter::_ReleaseDevice()
{
	if (m_bDwmEnableMMCSS && CDynLibManager::GetInstance()->pfnDwmEnableMMCSS)
	{
		CDynLibManager::GetInstance()->pfnDwmEnableMMCSS(FALSE);
		m_bDwmEnableMMCSS = false;
	}

	{
		CAutoLock lock(&m_csLock);
		SAFE_RELEASE(m_pDeviceEx);
		SAFE_RELEASE(m_pSwapChain);
        SAFE_DELETE(m_pDriverExtAdapter);
	}
	return CD3D9VideoPresenterBase::_ReleaseDevice();
}

STDMETHODIMP CD3D9VideoPresenter::_QueryCaps(PresenterCaps* pCaps)
{
    if (m_pDriverExtAdapter)
        return m_pDriverExtAdapter->QueryPresenterCaps(pCaps->VideoDecodeCaps, &m_PresenterCaps);

    return CD3D9VideoPresenterBase::_QueryCaps(pCaps);
}

STDMETHODIMP CD3D9VideoPresenter::SetDisplayRect(const RECT *pDst, const RECT *pSrc)
{
	HRESULT hr = CD3D9VideoPresenterBase::SetDisplayRect(pDst, pSrc);
	if (SUCCEEDED(hr))
	{
		POINT pt = {0};
		::ClientToScreen(m_hwnd, &pt);
		::OffsetRect(&m_rcDst, m_rcMonitor.left - pt.x, m_rcMonitor.top - pt.y);
	}

	return hr;
}

STDMETHODIMP CD3D9VideoPresenter::EndRender()
{
	HRESULT hr = CD3D9VideoPresenterBase::EndRender();

	// if using IDirect3DDevice9Ex, don't need to wait for GPU idle.
	if (m_pDeviceEx == 0)
		WaitUntilGpuIdle();

	return hr;
}

STDMETHODIMP CD3D9VideoPresenter::Clear()
{
	HRESULT hr = E_FAIL;
	DWORD dwVendorID = 0;
	
	hr = CRegistryService::GetInstance()->Get(REG_VENDOR_ID, &dwVendorID);
	// workaround the overlay surface is not cleared after stopping.
	if (m_pDevice && dwVendorID == PCI_VENDOR_ID_INTEL)
	{
		m_csLock.Lock();
		// Clear render target with black color.
		hr = m_pDevice->Clear(0L, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0L); ASSERT(SUCCEEDED(hr));
		hr = m_pDevice->BeginScene(); ASSERT(SUCCEEDED(hr));
		hr = m_pDevice->EndScene(); ASSERT(SUCCEEDED(hr));
		m_csLock.Unlock();
		// Flip until queue is filled with black content. note: we flip queue at least 3 times to make sure to update the black screen to the overlay successfully
		for (DWORD i = 0; i < 2; i++)
		{
			PresentHints Hints = {0};
			hr = Present(&Hints);
			ASSERT(SUCCEEDED(hr));
		}
	}
	return hr;
}

STDMETHODIMP CD3D9VideoPresenter::Present(const PresentHints *pHints)
{
	CAutoLock lock(&m_csLock);
	const RECT *pSrcRect = m_rcSrc.left == m_rcSrc.right || m_rcSrc.top == m_rcSrc.bottom ? NULL : &m_rcSrc;
	const RECT *pDstRect = m_rcDst.left == m_rcDst.right || m_rcDst.top == m_rcDst.bottom ? NULL : &m_rcDst;
	return D3D9Present(pSrcRect, pDstRect);
}

STDMETHODIMP CD3D9VideoPresenter::SetScreenCaptureDefense(BOOL bEnable)
{
	CAutoLock lock(&m_csLock);
	HRESULT hr = E_FAIL;
	DWORD dwVendorID = 0;
	DWORD dwOSVersion = 0;

	CRegistryService::GetInstance()->Get(REG_VENDOR_ID, &dwVendorID);
	CRegistryService::GetInstance()->Get(REG_OS_VERSION, &dwOSVersion);
	// screen capture defense is deprecated after windows 7.
	if (dwVendorID == PCI_VENDOR_ID_INTEL && dwOSVersion == OS_VISTA)
		hr = EnableScreenCaptureDefense(m_pDevice, bEnable);
	else
		hr = E_FAIL;

	return hr;
}

void CD3D9VideoPresenter::WaitUntilGpuIdle()
{
	// Create an event query from the current device
	CComPtr<IDirect3DQuery9> pEvent;
	if (SUCCEEDED(m_pDevice->CreateQuery(D3DQUERYTYPE_EVENT, &pEvent)))
	{
		// Add an end marker to the command buffer queue.
		pEvent->Issue(D3DISSUE_END);

		// Force the driver to execute the commands from the command buffer.
		// Empty the command buffer and wait until the GPU is idle.
		while (S_FALSE == pEvent->GetData( NULL, 0, D3DGETDATA_FLUSH))
			Sleep(1);
	}
}

HRESULT CD3D9VideoPresenter::EnableDwmQueuing(HWND hwnd)
{
	BOOL bEnabled = FALSE;
	HRESULT hr = E_FAIL;

	if (CDynLibManager::GetInstance()->pfnDwmIsCompositionEnabled)
		hr = CDynLibManager::GetInstance()->pfnDwmIsCompositionEnabled(&bEnabled);

	if (bEnabled)
	{
		ASSERT(CDynLibManager::GetInstance()->pfnDwmGetCompositionTimingInfo
			&& CDynLibManager::GetInstance()->pfnDwmSetPresentParameters);
/*
		hr = CDynLibManager::GetInstance()->pfnDwmGetWindowAttribute(hwnd, DWMWA_NCRENDERING_ENABLED, &bEnabled, sizeof(BOOL));
		if (bEnabled)
		{
			DWMNCRENDERINGPOLICY ncrp = DWMNCRP_DISABLED;
			hr = CDynLibManager::GetInstance()->pfnDwmSetWindowAttribute(hwnd, DWMWA_NCRENDERING_POLICY, &ncrp, sizeof(ncrp));
		}
*/
		DWM_TIMING_INFO dwmti = {0};

		dwmti.cbSize = sizeof(dwmti);

		hr = CDynLibManager::GetInstance()->pfnDwmGetCompositionTimingInfo(hwnd, &dwmti);
		if (FAILED(hr))
			return hr;

		DWM_PRESENT_PARAMETERS dwmpp = {0};

		dwmpp.cbSize = sizeof(dwmpp);
		dwmpp.fQueue = TRUE;
		dwmpp.cRefreshStart = dwmti.cRefresh + 1;
		dwmpp.cBuffer = 2;	// queue size
		dwmpp.fUseSourceRate = FALSE;
		dwmpp.cRefreshesPerFrame = 1;
		dwmpp.eSampling =  DWM_SOURCE_FRAME_SAMPLING_COVERAGE;

		hr = CDynLibManager::GetInstance()->pfnDwmSetPresentParameters(hwnd, &dwmpp);
		if (FAILED(hr))
			return hr;
	}
	return hr;
}

HRESULT CD3D9VideoPresenter::D3D9Present(const RECT *pSourceRect, const RECT *pDestRect)
{
	HRESULT hr = E_FAIL;

	if (m_pDevice && m_pSwapChain)
	{
// disable queuing before resolving the issue with Vista SP1 that LOGO can't be drawn on top of video.

		// EnableDwmQueuing may fail with E_PENDING many times until succeeding.
		if (m_bNeedEnableDwmQueuing)
		{
			DWORD dwVendorID = 0;
			CRegistryService::GetInstance()->Get(REG_VENDOR_ID, &dwVendorID);
			if (dwVendorID == PCI_VENDOR_ID_INTEL)
			{
				hr = EnableDwmQueuing(m_hwnd);
				m_bNeedEnableDwmQueuing = (hr == E_PENDING);
			}
			else
				m_bNeedEnableDwmQueuing = false;
		}
		if (m_PresenterProperty.dwFlags & PRESENTER_PROPERTY_WAITUNTILPRESENTABLE)
			CD3D9VideoPresenterBase::WaitUntilPresentable();

		if (D3D9_PRESENT_MODE_FULLSCREEN == m_ePresentMode)
		{
			while ((hr = m_pSwapChain->Present(NULL, NULL, m_hwnd, NULL, D3DPRESENT_DONOTWAIT)) == D3DERR_WASSTILLDRAWING)
				Sleep(1);
		}
		else if (D3D9_PRESENT_MODE_FLIPEX == m_ePresentMode)
		{
			while ((hr = m_pSwapChain->Present(NULL, NULL, NULL, NULL, D3DPRESENT_DONOTWAIT)) == D3DERR_WASSTILLDRAWING)
				Sleep(1);
		}
		else
		{
			while ((hr = m_pSwapChain->Present(pSourceRect, pDestRect, m_hwnd, NULL, D3DPRESENT_DONOTWAIT)) == D3DERR_WASSTILLDRAWING)
				Sleep(1);
		}
	}
	return hr;
}
