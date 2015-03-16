#include "stdafx.h"
#include "D3D9VideoPresenter.h"

using namespace DispSvr;

extern HRESULT EnableScreenCaptureDefense(IDirect3DDevice9 *pDevice, BOOL bEnableProtection);

CD3D9VideoPresenter::CD3D9VideoPresenter()
{
	m_GUID = DISPSVR_RESOURCE_D3DVIDEOPRESENTER;
	m_bNeedEnableDwmQueuing = false;
	m_pDeviceEx = NULL;
	m_pSwapChain = NULL;
	m_bDwmEnableMMCSS = false;
	m_ePresentMode = D3D9_PRESENT_MODE_WINDOWED;
    m_bDWMEnable = FALSE;
	m_bDWMQueueEnabled = FALSE;
    m_bFullScreen = false;
    m_dwOSVersion = GetRegistry(REG_OS_VERSION, 0);
}

CD3D9VideoPresenter::~CD3D9VideoPresenter()
{
}

STDMETHODIMP CD3D9VideoPresenter::_SetDevice(IUnknown *pDevice)
{
	HRESULT hr = CD3D9VideoPresenterBase::_SetDevice(pDevice);
	
	if(CDynLibManager::GetInstance()->pfnDwmIsCompositionEnabled)
	{
        CDynLibManager::GetInstance()->pfnDwmIsCompositionEnabled(&m_bDWMEnable);
        if (m_bDWMEnable)
        {
            m_bNeedEnableDwmQueuing = true;
            m_PresenterProperty.dwFlags |= PRESENTER_PROPERTY_DWMQUEUINGENABLED;
        }
        else
        {
            m_bNeedEnableDwmQueuing = false;
            m_PresenterProperty.dwFlags &= ~PRESENTER_PROPERTY_DWMQUEUINGENABLED;
        }
	}

	if (SUCCEEDED(hr))
	{
		if (m_bDWMEnable && CDynLibManager::GetInstance()->pfnDwmEnableMMCSS)
		{
			hr = CDynLibManager::GetInstance()->pfnDwmEnableMMCSS(TRUE);
			if( hr == S_OK ) m_bDwmEnableMMCSS = true;
		}

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

    m_PresenterCaps.dwPresenterInfo = PRESENTER_D3D;
    m_PresenterCaps.bCanVirtualizeFromOrigin = TRUE;

	return hr;
}

STDMETHODIMP CD3D9VideoPresenter::_ResetDevice()
{
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

	return S_OK;
}

STDMETHODIMP CD3D9VideoPresenter::_ReleaseDevice()
{
	if (m_bDWMQueueEnabled)	
	{
		m_PresenterProperty.dwFlags &= ~PRESENTER_PROPERTY_DWMQUEUINGENABLED;
		SetDwmQueuing(m_hwnd);
	}

	if (m_bDwmEnableMMCSS && CDynLibManager::GetInstance()->pfnDwmEnableMMCSS)
	{
		CDynLibManager::GetInstance()->pfnDwmEnableMMCSS(FALSE);
		m_bDwmEnableMMCSS = false;
	}

	{
		CAutoLock lock(&m_csLock);
		SAFE_RELEASE(m_pDeviceEx);
		SAFE_RELEASE(m_pSwapChain);
	}
	return CD3D9VideoPresenterBase::_ReleaseDevice();
}


STDMETHODIMP CD3D9VideoPresenter::ProcessMessage(RESOURCE_MESSAGE_TYPE eMessage, LPVOID ulParam)
{
	HRESULT hr = E_NOTIMPL;

	switch (eMessage)
	{
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

STDMETHODIMP CD3D9VideoPresenter::SetDisplayRect(const RECT *pDst, const RECT *pSrc)
{
	HRESULT hr = CD3D9VideoPresenterBase::SetDisplayRect(pDst, pSrc);
	if (SUCCEEDED(hr))
	{
		POINT pt = {0};
		::ClientToScreen(m_hwnd, &pt);
		::OffsetRect(&m_rcDst, GetRegistry(REG_DISPLAY_X, 0) - pt.x, GetRegistry(REG_DISPLAY_Y, 0) - pt.y);
	}

    m_bFullScreen = CD3D9VideoPresenterBase::IsWindowFullScreen();

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
	HRESULT hr = S_OK;
    DWORD dwVendorID = GetRegistry(REG_VENDOR_ID, 0);
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

STDMETHODIMP CD3D9VideoPresenter::SetProperty(const PresenterProperty *pProperty)
{
	CAutoLock lock(&m_csLock);
	HRESULT hr = CD3D9VideoPresenterBase::SetProperty(pProperty);
        if (SUCCEEDED(hr))
        {

            m_bNeedEnableDwmQueuing = m_PresenterProperty.dwFlags & PRESENTER_PROPERTY_DWMQUEUINGENABLED ? true : false;
	   if (!m_bNeedEnableDwmQueuing && m_bDWMQueueEnabled)	
	   {
		   SetDwmQueuing(m_hwnd);
	   }
	}
	return hr;
}

STDMETHODIMP CD3D9VideoPresenter::SetScreenCaptureDefense(BOOL bEnable)
{
	CAutoLock lock(&m_csLock);

    if (bEnable)
        return E_FAIL;
	else
        return E_NOTIMPL;
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

HRESULT CD3D9VideoPresenter::SetDwmQueuing(HWND hwnd)
{
	BOOL bEnabled = FALSE;
	HRESULT hr = E_FAIL;

	if (CDynLibManager::GetInstance()->pfnDwmIsCompositionEnabled)
		hr = CDynLibManager::GetInstance()->pfnDwmIsCompositionEnabled(&bEnabled);

	BOOL bQueue = (m_PresenterProperty.dwFlags & PRESENTER_PROPERTY_DWMQUEUINGENABLED) ? TRUE : FALSE;

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
        dwmpp.fQueue =  bQueue;
        dwmpp.cRefreshStart = dwmti.cRefresh + 1;
        dwmpp.cBuffer = 2;	// queue size
        dwmpp.fUseSourceRate = FALSE;
        dwmpp.cRefreshesPerFrame = 1;
        dwmpp.eSampling =  DWM_SOURCE_FRAME_SAMPLING_COVERAGE;

		hr = CDynLibManager::GetInstance()->pfnDwmSetPresentParameters(hwnd, &dwmpp);
		if (FAILED(hr))
			return hr;
    }

	m_bDWMQueueEnabled = (bEnabled && bQueue) ? TRUE : FALSE;

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
            // for bug104399, turn off DWM queue.
			/*DWORD dwVendorID = GetRegistry(REG_VENDOR_ID, 0);
			if (dwVendorID == PCI_VENDOR_ID_INTEL)
			{
				hr = SetDwmQueuing(m_hwnd);
				m_bNeedEnableDwmQueuing = SUCCEEDED(hr) ? false : true;
			}
			else*/
				m_bNeedEnableDwmQueuing = false;
		}

        if (m_PresenterProperty.dwFlags & PRESENTER_PROPERTY_WAITUNTILPRESENTABLE)
        {
            // Workaround for all vendor for Fullscreen mode and DWM OFF on D3DPRESENT_INTERVAL_ONE case.
            if (!(m_bFullScreen && !m_bDWMEnable && m_dwOSVersion >= OS_VISTA))
                CD3D9VideoPresenterBase::WaitUntilPresentable();
        }

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
