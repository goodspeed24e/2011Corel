#include "stdafx.h"
#include "DispSvr.h"
#include "ResourceManager.h"
#include "RegistryService.h"
#include "AMDSORTPresenter.h"
#include "Imports/ThirdParty/AMD/SORT/ATISORTDef.h"

using namespace DispSvr;

#define SORT_QUEUE_SIZE	2

CAMDSORTVideoPresenter::CAMDSORTVideoPresenter()
{
	m_GUID = DISPSVR_RESOURCE_AMDSORTVIDEOPRESENTER;
	m_bOverlyHided = true;
	m_bPreFlip = false;
	m_dwCapsEAPI = 0;
	m_dwCapsSORT = 0;
	m_dwStatus = SORT_STATUS_OK;
	m_dwStatusEx = SORT_STATUSEX_UNDEFINED;
	m_dwNumOfQueuedUpPreflips = 0;
	ZeroMemory(&m_rcClip, sizeof(RECT));
	m_pEAPI = NULL;
	m_pSORT = NULL;
	m_pRenderTarget = NULL;
}

CAMDSORTVideoPresenter::~CAMDSORTVideoPresenter()
{
}

STDMETHODIMP CAMDSORTVideoPresenter::_SetDevice(IUnknown *pDevice)
{
	CHECK_POINTER(pDevice);

	HRESULT hr = E_FAIL;
	IDirect3DDevice9* pDevice9 = NULL;
	hr = pDevice->QueryInterface(__uuidof(IDirect3DDevice9), (void **)&pDevice9);
	if (FAILED(hr))
		return hr;

	if (pDevice9 != m_pDevice)
	{
		hr = CD3D9VideoPresenterBase::_SetDevice(pDevice);
		if (SUCCEEDED(hr))
		{
			CAutoLock lock(&m_csLock);
			hr = CreateEAPI();
			hr = CreateSORT();
		}
	}

	pDevice9->Release();
	return hr;
}

STDMETHODIMP CAMDSORTVideoPresenter::_ReleaseDevice()
{
	m_csLock.Lock();
	if (m_pDevice && m_pRenderTarget)
	{
		// Restore the original render target.
		HRESULT hr = m_pDevice->SetRenderTarget(0, m_pRenderTarget);
		ASSERT(SUCCEEDED(hr));
	}

	// Release all D3D device and D3D surface.
	SAFE_RELEASE(m_pSORT);
	SAFE_RELEASE(m_pEAPI);
	SAFE_RELEASE(m_pRenderTarget);
	m_csLock.Unlock();
	return CD3D9VideoPresenterBase::_ReleaseDevice();
}

STDMETHODIMP CAMDSORTVideoPresenter::SetDisplayRect(const RECT *rcDst, const RECT *rcSrc)
{
	HRESULT hr = CD3D9VideoPresenterBase::SetDisplayRect(rcDst, rcSrc);
	if (FAILED(hr))
		return hr;

	CAutoLock lock(&m_csLock);
	DWORD dwCommand = SORT_CMD_NOP;
	bool bIsMinimized = false;

	SIZE szDisplay = {m_rcMonitor.right - m_rcMonitor.left, m_rcMonitor.bottom - m_rcMonitor.top};
	hr = CD3D9VideoPresenterBase::CaculateDstClipRect(&m_rcClip, &m_rcDst, &m_rcSrc, szDisplay);
	if (FAILED(hr) || CD3D9VideoPresenterBase::IsWindowMinimized())
	{
		dwCommand = SORT_CMD_HIDEOVERLAY;
		m_bOverlyHided = true;
		m_rcClip.left	= m_rcDst.left;
		m_rcClip.right	= m_rcDst.right;
		m_rcClip.top	= m_rcDst.top;
		m_rcClip.bottom = m_rcDst.bottom;
	}
	else
	{

#if 1
		//workaround for ATI driver to update the render target to fix the black screen after minimizing-maximizing the window
		if (m_bOverlyHided)
		{
			dwCommand = SORT_CMD_SHOWOVERLAY;
			hr = ExecSORT(dwCommand, 0);
			if (FAILED(hr))
				return hr;
		}
#endif
		m_bOverlyHided = false;

		// EAPI needs extra amd for fullscreen optimization.
		if (m_pEAPI && (m_dwCapsEAPI & EAPI_CAPS_FULLSCREENOPTIMIZATION))
		{
			bool bIsFullScreen = CD3D9VideoPresenterBase::IsWindowFullScreen();
			D3DLOCKED_RECT d3drect;
			hr = m_pEAPI->LockRect(&d3drect, 0, D3DLOCK_DISCARD); ASSERT(SUCCEEDED(hr));
			if (SUCCEEDED(hr))
			{
				PEAPI_FULLSCREEN_CMD_BUF pEAPIFullScreenCmd;
				pEAPIFullScreenCmd = (PEAPI_FULLSCREEN_CMD_BUF)d3drect.pBits;
				pEAPIFullScreenCmd->inHeader.dwCommandSize = sizeof(EAPI_FULLSCREEN_CMD_BUF);
				pEAPIFullScreenCmd->inHeader.dwCommand = EAPI_CMD_ENABLEFULLSCREN;
				pEAPIFullScreenCmd->dwValidFields = EAPI_CMD_FULLSCREEN_IN_FIELDS_FLG;
				pEAPIFullScreenCmd->dwFlags = (bIsFullScreen ? EAPI_FLAG_FULLSCREEN_ENABLE : 0);
				hr = m_pEAPI->UnlockRect();
			}
		}

		dwCommand = SORT_CMD_UPDATEOVERLAY;
	}

	// Update the render target.
	hr = ExecSORT(dwCommand, 0);
	return hr;
}

STDMETHODIMP CAMDSORTVideoPresenter::BeginRender()
{
	HRESULT hr = CD3D9VideoPresenterBase::BeginRender();
	if (SUCCEEDED(hr) && m_pSORT)
	{
		hr = m_pDevice->SetRenderTarget(0, m_pSORT);
	}
	return hr;
}

STDMETHODIMP CAMDSORTVideoPresenter::EndRender()
{
	HRESULT hr = CD3D9VideoPresenterBase::EndRender();
	if (SUCCEEDED(hr))
	{
		CAutoLock lock(&m_csLock);
		if (m_bPreFlip)
		{
			DWORD dwCommand = SORT_CMD_PREFLIPOVERLAY;
			while(1) 
			{
				hr = ExecSORT(dwCommand, 0);
				if (m_dwStatus == SORT_STATUS_QUEUE_IS_FULL)
				{
					hr = Flip();
					if (SUCCEEDED(hr))
						continue;
				}

				// Something wrong inside the SORT.
				ASSERT(!(m_dwStatus==SORT_STATUS_PREFLIP_NOT_READY && m_dwNumOfQueuedUpPreflips==SORT_QUEUE_SIZE));

				if (m_dwStatus != SORT_STATUS_OK)
					hr = E_FAIL;

				break;
			}
		}
	}

	return hr;
}

STDMETHODIMP CAMDSORTVideoPresenter::Present(const PresentHints *pHints)
{
	CAutoLock lock(&m_csLock);

	// Wait until the queue is full.
	if (m_dwNumOfQueuedUpPreflips < SORT_QUEUE_SIZE)
		return S_OK;

	HRESULT hr = Flip();
	return hr;
}

STDMETHODIMP CAMDSORTVideoPresenter::SetColorKey(const DWORD dwColorKey)
{
	HRESULT hr = CD3D9VideoPresenterBase::SetColorKey(dwColorKey);
	if (SUCCEEDED(hr))
	{
		CAutoLock lock(&m_csLock);
		DWORD dwCommand = SORT_CMD_UPDATEOVERLAY;
		hr = ExecSORT(dwCommand, 0);
	}
	return hr;
}

#define EAPI_SURFACE_WIDTH  10
#define EAPI_SURFACE_HEIGHT 10
#define EAPI_COLORFILL_ID   0x11235813

HRESULT CAMDSORTVideoPresenter::CreateEAPI()
{
	HRESULT hr = E_FAIL;

	// Get the video hwnd for EAPI
	D3DDEVICE_CREATION_PARAMETERS param;
	hr = m_pDevice->GetCreationParameters(&param);
	if (FAILED(hr))
		return hr;
	m_hwnd = param.hFocusWindow;

	hr = m_pDevice->CreateOffscreenPlainSurface(
		EAPI_SURFACE_WIDTH, 
		EAPI_SURFACE_HEIGHT, 
		(D3DFORMAT)MAKEFOURCC('E','A','P','I'), 
		D3DPOOL_DEFAULT, 
		&m_pEAPI, 
		NULL);
	if (FAILED(hr))
		return hr;

	{
		// Get EAPI Caps
		D3DLOCKED_RECT d3drect;
		hr = m_pEAPI->LockRect(&d3drect, 0, D3DLOCK_DISCARD); ASSERT(SUCCEEDED(hr));
		PEAPI_GETCAPS_IN_BUF pEAPICmd;
		pEAPICmd = (PEAPI_GETCAPS_IN_BUF)d3drect.pBits;
		pEAPICmd->inHeader.dwCommandSize = sizeof(EAPI_GETCAPS_IN_BUF);
		pEAPICmd->dwValidFields = EAPI_GETCAPS_OUT_FIELDS_REV | EAPI_GETCAPS_OUT_FIELDS_CAP;
		pEAPICmd->inHeader.dwCommand = EAPI_CMD_GETCAPS;
		hr = m_pEAPI->UnlockRect();
		m_dwCapsEAPI = ((EAPI_GETCAPS_OUT_BUF*)pEAPICmd)->dwCaps;
		if(!(m_dwCapsEAPI & EAPI_CAPS_TAGASPROTECTED))
		{
			SAFE_RELEASE(m_pEAPI);
			return E_FAIL;
		}

		// We might have problem in normal path.
		ASSERT(m_dwCapsEAPI & EAPI_CAPS_SORTVIRTUALIZATION);
		// Update PresenterCaps
		m_PresenterCaps.dwFPS = ((EAPI_GETCAPS_OUT_BUF*)pEAPICmd)->dwRecomFps;
		m_PresenterCaps.dwResPixels = ((EAPI_GETCAPS_OUT_BUF*)pEAPICmd)->dwRecomVideoPixelArea;
		m_PresenterCaps.bIsOverlay = TRUE;
		m_bPreFlip = true;
	}

	{
		// Register EAPI backdoor.
		D3DLOCKED_RECT d3drect;
		hr = m_pEAPI->LockRect(&d3drect, 0, D3DLOCK_DISCARD); ASSERT(SUCCEEDED(hr));
		PEAPI_COLFILTAGSURF_CMD_BUF pEAPICmd;
		pEAPICmd = (PEAPI_COLFILTAGSURF_CMD_BUF)d3drect.pBits;
		pEAPICmd->inHeader.dwCommandSize = sizeof(EAPI_COLFILTAGSURF_CMD_BUF);
		pEAPICmd->dwValidFields = EAPI_COLFILTAGSURF_IN_FIELDS_CFD | EAPI_COLFILTAGSURF_IN_FIELDS_FLG | EAPI_COLFILTAGSURF_IN_FIELDS_HWND;
		pEAPICmd->inHeader.dwCommand = EAPI_CMD_COLFILTAGSURF;
		pEAPICmd->dwColorFillID = EAPI_COLORFILL_ID;
		pEAPICmd->dwFlags = EAPI_FLAG_TAGASPROTECTED;
		pEAPICmd->hWindow = (unsigned __int64)m_hwnd;
		hr = m_pEAPI->UnlockRect();
	}

	if (m_dwCapsEAPI & EAPI_CAPS_SORTVIRTUALIZATION)
	{
		// Enable EAPI virtualization
		D3DLOCKED_RECT d3drect;
		hr = m_pEAPI->LockRect(&d3drect, 0, D3DLOCK_DISCARD); ASSERT(SUCCEEDED(hr));
		PEAPI_VIRTUALIZESORT_CMD_BUF pEAPIVirtualizeSortCmd;
		pEAPIVirtualizeSortCmd = (PEAPI_VIRTUALIZESORT_CMD_BUF)d3drect.pBits;
		pEAPIVirtualizeSortCmd->inHeader.dwCommandSize = sizeof(EAPI_VIRTUALIZESORT_CMD_BUF);
		pEAPIVirtualizeSortCmd->dwValidFields = EAPI_CMD_VIRTUALIZESORT_IN_FIELDS_FLG;
		pEAPIVirtualizeSortCmd->inHeader.dwCommand = EAPI_CMD_VIRTUALIZESORT;
		pEAPIVirtualizeSortCmd->dwFlags = EAPI_FLAG_VIRTUALIZESORT_ENABLE;
		hr = m_pEAPI->UnlockRect();
	}

	return hr;
}

HRESULT CAMDSORTVideoPresenter::CreateSORT()
{
	HRESULT hr = E_FAIL;

	hr = m_pDevice->GetRenderTarget(0, &m_pRenderTarget);
	if (FAILED(hr))
		return hr;

	D3DSURFACE_DESC desc;
	hr = m_pRenderTarget->GetDesc(&desc);
	if (FAILED(hr))
		return hr;

	DWORD dwBBWidth = m_rcMonitor.right - m_rcMonitor.left, dwBBHeight = m_rcMonitor.bottom - m_rcMonitor.top;

	CRegistryService::GetInstance()->Get(REG_BACKBUFFER_WIDTH, &dwBBWidth);
	CRegistryService::GetInstance()->Get(REG_BACKBUFFER_HEIGHT, &dwBBHeight);

	desc.Width = dwBBWidth;
	desc.Height = dwBBHeight;

CreateSurface:
	if (!m_pEAPI)	
		desc.Format = (D3DFORMAT) MAKEFOURCC('S','O','R','T');	// Create SORT render target.

	hr = m_pDevice->CreateRenderTarget(
		desc.Width, 
		desc.Height, 
		desc.Format, 
		desc.MultiSampleType, 
		desc.MultiSampleQuality, 
		TRUE, 
		&m_pSORT, 
		NULL);

	if (FAILED(hr))
		return hr;

	if (m_pEAPI)
	{
		// Register surface as SORT protected.
		hr = m_pDevice->ColorFill(m_pSORT, NULL, EAPI_COLORFILL_ID);
		if (FAILED(hr))
		{
			SAFE_RELEASE(m_pEAPI);
			goto CreateSurface;
		}

		// Try to lock the SORT surface.
		D3DLOCKED_RECT d3drect;
		hr = m_pSORT->LockRect(&d3drect, 0, D3DLOCK_DISCARD);
		if (FAILED(hr))
		{
			SAFE_RELEASE(m_pEAPI);
			goto CreateSurface;
		}
		hr = m_pSORT->UnlockRect();
	}
	else
	{
		// Using SORT if EAPI is not supported.
		D3DLOCKED_RECT d3drect;
		DWORD dwHeight, dwFPS;
		hr = m_pSORT->LockRect(&d3drect, 0, D3DLOCK_DISCARD); ASSERT(SUCCEEDED(hr));
		dwFPS = ((SORT_CMD_BUF*)d3drect.pBits)->sCaps.dwRecomFps;
		dwHeight = ((SORT_CMD_BUF*)d3drect.pBits)->sCaps.dwRecomHDHeight;
		m_bPreFlip = (((SORT_CMD_BUF*)d3drect.pBits)->sCaps.dwCaps & SORT_CAPS_RTFLIP) ? true : false;
		hr = m_pSORT->UnlockRect();

		if (dwHeight < desc.Height)
		{
			SAFE_RELEASE(m_pSORT);
			desc.Height = dwHeight;
			goto CreateSurface;
		}

		// Update PresenterCaps
		m_PresenterCaps.dwFPS = dwFPS;
		m_PresenterCaps.dwResPixels = desc.Width * desc.Height;
		m_PresenterCaps.bIsOverlay = TRUE;
	}

	hr = m_pDevice->SetRenderTarget(0, m_pSORT);

	return hr;
}

HRESULT CAMDSORTVideoPresenter::ExecSORT(DWORD dwCommand, DWORD dwFlags)
{
	if (!m_pSORT)
		return E_FAIL;

	HRESULT hr = E_FAIL;
	D3DLOCKED_RECT d3drect;

	hr = m_pSORT->LockRect(&d3drect, 0, D3DLOCK_DISCARD);
	if (SUCCEEDED(hr))
	{
		SORT_CMD_BUF *pCmd = (SORT_CMD_BUF*)d3drect.pBits;
		pCmd->dwSize = sizeof(SORT_CMD_BUF);
		pCmd->dwFlags = dwFlags;
		pCmd->dwCommand = dwCommand;
		if (dwCommand==SORT_CMD_UPDATEOVERLAY || dwCommand==SORT_CMD_SHOWOVERLAY)
		{
			pCmd->rcSrc = m_rcClip;
			pCmd->rcDst = m_rcDst;
			pCmd->dwDstColorKey = m_dwColorKey;
		}

		hr = m_pSORT->UnlockRect();
		m_dwStatus = pCmd->dwStatus;
//		m_dwStatusEx = pCmd->dwStatusEx;
		m_dwNumOfQueuedUpPreflips = pCmd->dwNumOfQueuedUpPreflips;
		if (m_dwStatus != SORT_STATUS_OK)
			hr = E_FAIL;
	}

	return hr;
}

HRESULT CAMDSORTVideoPresenter::Flip()
{
	HRESULT hr = S_OK;

	if (m_dwStatus == SORT_STATUS_QUEUE_IS_EMPTY)
	{
		ASSERT(0 && "m_dwStatus == SORT_STATUS_QUEUE_IS_EMPTY");
		return E_FAIL;
	}

#if 0
	if (m_dwRevision==SORT_VERSION && m_bEscCmd)
	{
		BYTE inBuf[sizeof(CWDDECMD)];
		CWDDECMD *pCmdBuf = (CWDDECMD *)&inBuf[0];
		HDC hDC = ::GetDC(NULL);
		if (hDC)
		{
			pCmdBuf->ulSize = sizeof(CWDDECMD);
			pCmdBuf->ulEscape32 = CWDDEVA_RTFLIP;
			pCmdBuf->ulIndex = 0;
			if(ExtEscape(hDC, Control_Config_CWDDE32, pCmdBuf->ulSize, (LPCSTR)&inBuf[0], 0, NULL) != CWDDEVA_OK)
			{
				DbgMsg("SORT: ExtEscape failed\n");
				hr = E_FAIL;
			}
		}
		else
		{
			DbgMsg("SORT: Get hDC failed\n");
			hr = E_FAIL;
		}
	}
	else
#endif
	{
		DWORD dwCommand = SORT_CMD_FLIPOVERLAY;
		DWORD dwFlags = (m_bPreFlip) ? SORT_FLAG_USERTFLIP|SORT_FLAG_CHECK_PREFLIP_STATUS : 0;

		int retries = (m_PresenterCaps.dwFPS == 60) ? 8 : 16;
		for (int i=0; i<retries; i++)
		{
			hr = ExecSORT(dwCommand, dwFlags);
			if (m_dwStatus != SORT_STATUS_PREFLIP_NOT_READY)
				break;
			else
				Sleep(2);
		}

#ifdef _DEBUG
		if (m_dwStatus != SORT_STATUS_OK)
		{
			DbgMsg("SORT: Flip status: %d. Number of retries: %d", m_dwStatus, retries);
		}
#endif
	}

	return hr;
}