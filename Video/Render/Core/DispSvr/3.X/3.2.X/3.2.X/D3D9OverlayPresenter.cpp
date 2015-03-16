#include "stdafx.h"
#include "IntelCUISDKHelper.h"
#include "D3D9OverlayPresenter.h"


using namespace DispSvr;

CD3D9OverlayPresenter::CD3D9OverlayPresenter()
{
	m_GUID = DISPSVR_RESOURCE_D3DOVERALYPRESENTER;
	m_bHideOverlay = FALSE;
	m_bUpdateColorkey = FALSE;
	m_bUpdateColorkeyOnly = FALSE;
}

CD3D9OverlayPresenter::~CD3D9OverlayPresenter()
{
}

STDMETHODIMP CD3D9OverlayPresenter::ProcessMessage(RESOURCE_MESSAGE_TYPE msg, LPVOID ulParam)
{
	HRESULT hr = E_NOTIMPL;

	switch (msg)
	{
	case RESOURCE_MESSAGE_UPDATEWINDOW:
		m_bUpdateColorkey = TRUE;
		m_bHideOverlay = FALSE;
		hr = S_OK;
		break;
	case RESOURCE_MESSAGE_MOVEWINDOW:
		m_bUpdateColorkeyOnly = TRUE;
		m_bHideOverlay = FALSE;
		hr = S_OK;
		break;
	case RESOURCE_MESSAGE_HIDEWINDOW:
		m_bHideOverlay = *(reinterpret_cast<BOOL *> (ulParam));
		hr = S_OK;
		break;
	default:
		return CD3D9VideoPresenterBase::ProcessMessage(msg, ulParam);
	}
	return hr;
}

STDMETHODIMP CD3D9OverlayPresenter::_SetDevice(IUnknown *pDevice)
{
	HRESULT hr = CD3D9VideoPresenterBase::_SetDevice(pDevice);
	if (SUCCEEDED(hr))
	{
		// D3D9 overlay is used in IDirect3DDevice9Ex only.
		if (m_pDeviceEx == 0)
			return E_FAIL;

		// D3D9 overlay will update color by D3D9 runtime, and UI should not.
		m_PresenterCaps.bIsOverlay = FALSE;
        m_PresenterCaps.bSupportXvYCC = GetRegistry(REG_DISPLAY_XVYCC_MONITOR_TYPE, DISPLAY_XVYCC_MONITOR_NOT_SUPPORT) != DISPLAY_XVYCC_MONITOR_NOT_SUPPORT;
        m_PresenterCaps.dwPresenterInfo = PRESENTER_D3DOVERLAY;
        m_PresenterCaps.bCanVirtualizeFromOrigin = TRUE;
	}
	return hr;
}

STDMETHODIMP CD3D9OverlayPresenter::SetDisplayRect(const RECT *pDst, const RECT *pSrc)
{
	HRESULT hr = CD3D9VideoPresenterBase::SetDisplayRect(pDst, pSrc);
	if (SUCCEEDED(hr))
	{
		POINT pt = {0};
		::ClientToScreen(m_hwnd, &pt);
		::OffsetRect(&m_rcDst, GetRegistry(REG_DISPLAY_X, 0) - pt.x, GetRegistry(REG_DISPLAY_Y, 0) - pt.y);
	}

	return hr;
}

STDMETHODIMP CD3D9OverlayPresenter::Present(const PresentHints *pHints)
{
	HRESULT hr = E_FAIL;
	DWORD dwPresentFlags = D3DPRESENT_DONOTWAIT;
	RECT rcSrc, rcDst;

	CAutoLock lock(&m_csLock);
	const RECT *pSourceRect = m_rcSrc.left == m_rcSrc.right || m_rcSrc.top == m_rcSrc.bottom ? NULL : &m_rcSrc;
	const RECT *pDestRect = m_rcDst.left == m_rcDst.right || m_rcDst.top == m_rcDst.bottom ? NULL : &m_rcDst;
	ASSERT(m_pDeviceEx);

	// Most of D3D9 Overlay does not support scaling and it makes no sense to do so.
	// We should clip the source and destination rectangles before passing to PresentEx();
	if (m_bUpdateColorkey) // only do it once.
	{
		dwPresentFlags |= D3DPRESENT_UPDATECOLORKEY;
		m_bUpdateColorkey = FALSE;
	}

	if (m_bUpdateColorkeyOnly) // only do it once.
	{
		dwPresentFlags |= D3DPRESENT_UPDATEOVERLAYONLY;
		m_bUpdateColorkeyOnly = FALSE;
	}

	if (m_bHideOverlay)
		dwPresentFlags |= D3DPRESENT_HIDEOVERLAY;


	if (pSourceRect && pDestRect)
	{
		rcSrc = *pSourceRect;
		rcDst = *pDestRect;

		if (rcSrc.left < 0)
		{
			rcDst.left -= rcSrc.left;
			rcSrc.left = 0;
		}

		if (rcSrc.top < 0)
		{
			rcDst.top -= rcSrc.top;
			rcSrc.top = 0;
		}

		if (rcSrc.right > m_szSrc.cx)
		{
			rcDst.right -= rcSrc.right - m_szSrc.cx;
			rcSrc.right = m_szSrc.cx;
		}

		if (rcSrc.bottom > m_szSrc.cy)
		{
			rcDst.bottom -= rcSrc.bottom - m_szSrc.cy;
			rcSrc.bottom = m_szSrc.cy;
		}

		pSourceRect = &rcSrc;
		pDestRect = &rcDst;
	}
	else
	{
		dwPresentFlags |= D3DPRESENT_HIDEOVERLAY;
	}

    if (m_PresenterProperty.dwFlags & PRESENTER_PROPERTY_WAITUNTILPRESENTABLE)
        CD3D9VideoPresenterBase::WaitUntilPresentable();

	// Passing hDestWindowOverride as NULL because D3D9 Overlay only accepts NULL or the same with
	// hDeviceWindow used to create the device.
	//
	// D3DPRESENT_UPDATECOLORKEY is set when WM_PAINT is received.
	// D3DPRESENT_HIDEOVERLAY is set when window is minimized.
	// D3DPRESENT_UPDATEOVERLAYONLY is used when content is the same and window position is changed.
	while ((hr = m_pDeviceEx->PresentEx(pSourceRect, pDestRect, NULL, NULL, dwPresentFlags)) == D3DERR_WASSTILLDRAWING)
		Sleep(1);

	return hr;
}

STDMETHODIMP CD3D9OverlayPresenter::SetScreenCaptureDefense(BOOL bEnable)
{
    CAutoLock lock(&m_csLock);

	if (bEnable)
		return S_OK;
	return E_FAIL;	// can't turn off
}

STDMETHODIMP CD3D9OverlayPresenter::Clear()
{
	HRESULT hr = S_OK;
	ASSERT(m_pDeviceEx);
	for (int i = 0; i < _BACK_BUFFER_COUNT_ + _FRONT_BUFFER_COUNT_; i++)
	{
		CComPtr<IDirect3DSurface9> pBackBuffer;
		hr = m_pDeviceEx->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer);	ASSERT(SUCCEEDED(hr));
        // If monitor supports XvYCC, we fill black color in YUV format. Otherwise, fill black in RGB format
        if (m_PresenterCaps.bSupportXvYCC)
        {
            hr = m_pDeviceEx->ColorFill(pBackBuffer, NULL, D3DCOLOR_AYUV(0xff, 0x00, 0x80, 0x80));ASSERT(SUCCEEDED(hr));
        }
        else
        {
            hr = m_pDeviceEx->ColorFill(pBackBuffer, NULL, D3DCOLOR_ARGB(0xff, 0x00, 0x00, 0x00));ASSERT(SUCCEEDED(hr));
        }
		hr = m_pDeviceEx->SetRenderTarget(0, pBackBuffer);	ASSERT(SUCCEEDED(hr));
		PresentHints Hints = {0};
		hr = Present(&Hints);
	}
	return hr;
}

STDMETHODIMP CD3D9OverlayPresenter::SetGamutMetadata(const DWORD dwFormat, void *pGamutMetadata)
{
    if (!m_PresenterCaps.bSupportXvYCC)
    {
        return S_FALSE;
    }

    HRESULT hr = E_INVALIDARG;
    m_dwGamutFormat = dwFormat;

    // we should move this to DriverExtensionHelper
    if (PCI_VENDOR_ID_INTEL == GetRegistry(REG_VENDOR_ID, 0))
    {
        BOOL bUseExtendedGamut = FALSE;

        if (m_dwGamutFormat == GAMUT_METADATA_NONE)
        {
            memset(&m_GamutRange, 0, sizeof(m_GamutRange));
            memset(&m_GamutVertices, 0, sizeof(m_GamutVertices));
            hr = CIntelCUIHelper::GetHelper()->SetGamutMetadata(NULL, 0);
        }
        else if (m_dwGamutFormat == GAMUT_METADATA_RANGE)
        {
            memcpy(&m_GamutRange, pGamutMetadata, sizeof(m_GamutRange));
            hr = CIntelCUIHelper::GetHelper()->SetGamutMetadata(pGamutMetadata, sizeof(GamutMetadataRange));
            if (SUCCEEDED(hr))
                bUseExtendedGamut = TRUE;
        }
        SetRegistry(REG_DISPLAY_XVYCC_GAMUT, bUseExtendedGamut);
    }

    return hr;
}