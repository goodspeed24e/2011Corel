#include "stdafx.h"
#include "DispSvr.h"
#include "DynLibManager.h"
#include "ResourceManager.h"
#include "RegistryService.h"
#include "D3D9VideoPresenterBase.h"

using namespace DispSvr;

//////////////////////////////////////////////////////////////////////////
// CD3D9VideoPresenterBase
CD3D9VideoPresenterBase::CD3D9VideoPresenterBase()
{
	m_GUID = DISPSVR_RESOURCE_VIDEOPRESENTER;
	m_fScanLineInterval = 0.0f;
	m_dwSafePresentOffset = 0;
	m_dwColorKey = 0;
	ZeroMemory(&m_rcSrc, sizeof(RECT));
	ZeroMemory(&m_rcDst, sizeof(RECT));
	ZeroMemory(&m_szSrc, sizeof(SIZE));
	ZeroMemory(&m_PresenterProperty, sizeof(PresenterProperty));
	m_PresenterProperty.dwSize = sizeof(PresenterProperty);
	ZeroMemory(&m_PresenterCaps, sizeof(PresenterCaps));
	m_PresenterCaps.dwSize = sizeof(PresenterCaps);
	ZeroMemory(&m_GamutRange, sizeof(GamutMetadataRange));
	ZeroMemory(&m_GamutRange, sizeof(GamutMetadataRange));
	m_dwGamutFormat = GAMUT_METADATA_NONE;
	m_hPresent = CreateEvent(0, TRUE, FALSE, 0);
	m_lpDDraw = NULL;
}

CD3D9VideoPresenterBase::~CD3D9VideoPresenterBase()
{
	if (m_hPresent)
	{
		CloseHandle(m_hPresent);
		m_hPresent = 0;
	}
}

STDMETHODIMP CD3D9VideoPresenterBase::QueryInterface(REFIID riid, void **ppv)
{
	if (riid == __uuidof(IDispSvrVideoPresenter))
	{
		return GetInterface((IDispSvrVideoPresenter *)this, ppv);
	}
	return CD3D9PluginBase::QueryInterface(riid, ppv);
}

STDMETHODIMP CD3D9VideoPresenterBase::_SetDevice(IUnknown *pDevice)
{
	HRESULT hr;

	CAutoLock lock(&m_csLock);
	hr = CD3D9PluginBase::_SetDevice(pDevice);
	if (FAILED(hr))
		return hr;

	DWORD dwBBWidth = 0, dwBBHeight = 0;
	CRegistryService::GetInstance()->Get(REG_BACKBUFFER_WIDTH, &dwBBWidth);
	CRegistryService::GetInstance()->Get(REG_BACKBUFFER_HEIGHT, &dwBBHeight);

	// Initialize the source size and rectangles
	m_szSrc.cx = dwBBWidth;
	m_szSrc.cy = dwBBHeight;

	// Update the default presenter caps
	m_PresenterCaps.dwFPS = 30;
	m_PresenterCaps.dwBandWidth = 0;
	m_PresenterCaps.dwResPixels = (m_rcMonitor.right - m_rcMonitor.left) * (m_rcMonitor.bottom - m_rcMonitor.top);
	m_PresenterCaps.bIsOverlay = FALSE;
	m_PresenterCaps.bSupportXvYCC = FALSE;
	m_PresenterCaps.VideoDecodeCaps = 0;
	m_PresenterCaps.bHwDecode = TRUE;

	if (!m_pDeviceEx)
		hr = DirectDrawCreate(NULL, &m_lpDDraw, NULL);

	return hr;
}

STDMETHODIMP CD3D9VideoPresenterBase::_ReleaseDevice()
{
	CAutoLock lock(&m_csLock);
	SAFE_RELEASE(m_lpDDraw);
	return CD3D9PluginBase::_ReleaseDevice();
}

STDMETHODIMP CD3D9VideoPresenterBase::_QueryCaps(PresenterCaps* pCaps)
{
	if(pCaps->VideoDecodeCaps > 0) //In default, we think other vendors, like Intel, S3, SIS..., support 1HD only.
	{
		m_PresenterCaps.bHwDecode = (pCaps->VideoDecodeCaps & VIDEO_CAP_STREAM_SUB) ? FALSE : TRUE;
	}
	return S_OK;
}

// The coordinates of rcDst must be opposite to current monitor, not desktop.
// The coordinates of rcSrc must be opposite to backbuffer.
STDMETHODIMP CD3D9VideoPresenterBase::SetDisplayRect(const RECT *rcDst, const RECT *rcSrc)
{
	CHECK_POINTER(rcSrc);
	CHECK_POINTER(rcDst);

	// If there's 2 monitors in extend mode, rcDst should be opposite to 1st monitor.
	// Now we change the coordinates opposite to 2nd monitor.
	m_rcSrc = *rcSrc;
	m_rcDst = *rcDst;

	// Bug#81775
	// because of primary monitor change,
	// rcMonitor will change and need an update
	// when primary monitor change,
	// app should call SetDisplayRect() to update window info in Presenter
	HMONITOR hMonitor = MonitorFromWindow(m_hwnd, (m_hwnd) ? MONITOR_DEFAULTTONEAREST : MONITOR_DEFAULTTOPRIMARY);
	MONITORINFOEX MonInfo;
	ZeroMemory(&MonInfo, sizeof(MONITORINFOEX));
	MonInfo.cbSize = sizeof(MONITORINFOEX);
	if (GetMonitorInfo(hMonitor, &MonInfo) == TRUE)
	{
		m_rcMonitor = MonInfo.rcMonitor;
	}

	return S_OK;
}

STDMETHODIMP CD3D9VideoPresenterBase::BeginRender()
{
	CAutoLock lock(&m_csLock);
	HRESULT hr = E_FAIL;
	if (m_pDevice)
		hr = m_pDevice->BeginScene();
	return hr;
}

STDMETHODIMP CD3D9VideoPresenterBase::EndRender()
{
	CAutoLock lock(&m_csLock);
	HRESULT hr = E_FAIL;
	if (m_pDevice)
		hr = m_pDevice->EndScene();

	return hr;
}

STDMETHODIMP CD3D9VideoPresenterBase::Present(const PresentHints *pHints)
{
	return E_NOTIMPL;
}

STDMETHODIMP CD3D9VideoPresenterBase::Clear()
{
	return E_NOTIMPL;
}

STDMETHODIMP CD3D9VideoPresenterBase::SetProperty(const PresenterProperty *pProperty)
{
	CHECK_POINTER(pProperty);
	if (pProperty->dwSize != m_PresenterProperty.dwSize)
		return E_INVALIDARG;

	m_PresenterProperty.dwFlags = pProperty->dwFlags;
	return S_OK;
}

STDMETHODIMP CD3D9VideoPresenterBase::GetProperty(PresenterProperty *pProperty)
{
	CHECK_POINTER(pProperty);
	if (pProperty->dwSize != m_PresenterProperty.dwSize)
		return E_INVALIDARG;

	pProperty->dwFlags = m_PresenterProperty.dwFlags;
	return S_OK;
}

// Default D3D9 doesn't support color key.
// Color key is primarily for private overlay presenter.
STDMETHODIMP CD3D9VideoPresenterBase::SetColorKey(const DWORD dwColorKey)
{
	if (m_PresenterCaps.bIsOverlay == TRUE)
	{
		m_dwColorKey32 = dwColorKey;
		D3DDISPLAYMODE D3DDisplayMode;
		HRESULT hr = m_pDevice->GetDisplayMode(0, &D3DDisplayMode);
		if (SUCCEEDED(hr))
		{
			switch (D3DDisplayMode.Format)
			{
			case D3DFMT_R5G6B5:
				{
					DWORD dwColorKey16;
					unsigned char *p = (unsigned char *)&m_dwColorKey32;
					dwColorKey16 = p[0] >> 3 | p[1] >> 2 << 5 | p[2] >> 3 << 11;
					m_dwColorKey = dwColorKey16;
				}
				break;
			default:
				m_dwColorKey = m_dwColorKey32;
				break;
			}
		}
		
		return S_OK;
	}
	return E_NOTIMPL;
}

STDMETHODIMP CD3D9VideoPresenterBase::GetColorKey(DWORD* pdwColorKey)
{
	if (m_PresenterCaps.bIsOverlay == TRUE)
	{
		*pdwColorKey = m_dwColorKey32;
		return S_OK;
	}
	return E_NOTIMPL;
}

STDMETHODIMP CD3D9VideoPresenterBase::QueryCaps(PresenterCaps* pCaps)
{
	CHECK_POINTER(pCaps);
	if (pCaps->dwSize != sizeof(PresenterCaps))
		return E_INVALIDARG;

	_QueryCaps(pCaps);
	memcpy(pCaps, &m_PresenterCaps, sizeof(PresenterCaps));
	return S_OK;
}

STDMETHODIMP CD3D9VideoPresenterBase::SetScreenCaptureDefense(BOOL bEnable)
{
	if (bEnable == TRUE)
	{
		if (m_PresenterCaps.bIsOverlay)
			return S_OK;
		else
			return E_NOTIMPL;
	}
	else
	{
		if (m_PresenterCaps.bIsOverlay)
			return E_FAIL;
		else
			return S_OK;
	}
}

STDMETHODIMP CD3D9VideoPresenterBase::SetGamutMetadata(const DWORD dwFormat, void *pGamutMetadata)
{
	return E_NOTIMPL;
}

HRESULT CD3D9VideoPresenterBase::CaculateDstClipRect(RECT *rcClip, RECT *rcDst, const RECT *rcSrc, const SIZE szDisplay)
{
	if (rcDst->left <= 0 && rcDst->right <= 0 && rcDst->top <= 0 && rcDst->bottom <= 0)
		return E_FAIL;

	int SrcWidth = rcSrc->right - rcSrc->left;
	int SrcHeight = rcSrc->bottom - rcSrc->top;
	int DstWidth = rcDst->right - rcDst->left;
	int DstHeight = rcDst->bottom - rcDst->top;

	if (SrcWidth<=0 || SrcHeight<=0 || DstWidth<=0 || DstHeight<=0)
	{
		return E_FAIL;
	}

	*rcClip = *rcSrc;
	if (rcDst->left < 0)
	{
		rcClip->left += (LONG)((float)-rcDst->left/DstWidth * SrcWidth + 0.5);
		rcDst->left = 0;
	}

	if (rcDst->top < 0)
	{
		rcClip->top += (LONG)((float)-rcDst->top/DstHeight * SrcHeight + 0.5);
		rcDst->top = 0;
	}

	if (rcDst->right > szDisplay.cx)
	{
		rcClip->right -= (LONG)(((float)rcDst->right - szDisplay.cx)/DstWidth * SrcWidth + 0.5);
		rcDst->right = szDisplay.cx;
	}

	if (rcDst->bottom > szDisplay.cy)
	{
		rcClip->bottom -= (LONG)(((float)rcDst->bottom - szDisplay.cy)/DstHeight * SrcHeight + 0.5);
		rcDst->bottom = szDisplay.cy;
	}

	return S_OK;
}

#define MAX_PRESENT_INTERVAL 1 // The max time in ms that a present can draw onto screen.

void CD3D9VideoPresenterBase::UpdateScanLineProperty()
{
	LARGE_INTEGER liFrequency, liStart, liEnd;
	D3DRASTER_STATUS d3dRS1, d3dRS2;
	INT iFaultCount = 10;

	QueryPerformanceFrequency(&liFrequency);

	while(iFaultCount-- >0)
	{
		m_pDevice->GetRasterStatus(0, &d3dRS1);
		QueryPerformanceCounter(&liStart);

		Sleep(1);

		m_pDevice->GetRasterStatus(0, &d3dRS2);
		QueryPerformanceCounter(&liEnd);

		if (!d3dRS1.InVBlank && !d3dRS2.InVBlank && (d3dRS1.ScanLine < d3dRS2.ScanLine))
		{
			LONGLONG dwCost = liEnd.QuadPart - liStart.QuadPart;
			DWORD dwScanned = d3dRS2.ScanLine - d3dRS1.ScanLine;
			m_fScanLineInterval = ((float)dwCost / dwScanned) * ((float)1000/liFrequency.QuadPart);
			// m_fScanLineInterval for per scanline, 0.1 is a very unreasonable value
			// there are possibilities that Sleep(1) yields cpu for too long and (d3dRS1.ScanLine < d3dRS2.ScanLine)
			// still establishes in different video retraces.
			if (m_fScanLineInterval < 0.1)
				break;

			// reject the result and recalculate again.
			m_fScanLineInterval = 0.0;
		}
		Sleep(2);
	}

	if (m_fScanLineInterval == 0.0f)
	{
		// Can't get the time that actual scanlines cost.
		// Need to calculate by display property.
		// The real number of scanlines is a little more then screen height and is used for VBlank.
		D3DDISPLAYMODE D3DDisplayMode;
		HRESULT hr = m_pDevice->GetDisplayMode(0, &D3DDisplayMode); ASSERT(SUCCEEDED(hr));
		m_fScanLineInterval = (float)1000 / (D3DDisplayMode.RefreshRate * D3DDisplayMode.Height * 1.04f);
	}

	m_dwSafePresentOffset = (DWORD)(MAX_PRESENT_INTERVAL / m_fScanLineInterval);
	DbgMsg("Scanline interval: %f", m_fScanLineInterval);
}

bool CD3D9VideoPresenterBase::IsWindowFullScreen()
{
	bool bIsFullScreen = 
		m_rcDst.left == 0 && 
		m_rcDst.top == 0 &&
		(m_rcDst.right) == (m_rcMonitor.right-m_rcMonitor.left) &&
		(m_rcDst.bottom) == (m_rcMonitor.bottom-m_rcMonitor.top);

	return bIsFullScreen;
}

bool CD3D9VideoPresenterBase::IsWindowMinimized()
{
	POINT pt = {0};
	::ClientToScreen(m_hwnd, &pt);
	return (((pt.x & pt.y) & 0xffff0000) == 0xffff0000);
}

void CD3D9VideoPresenterBase::WaitUntilPresentable()
{
	if(m_pDeviceEx)
		m_pDeviceEx->WaitForVBlank(0);
 	else if (m_lpDDraw)
 	 	m_lpDDraw->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN , NULL);
}