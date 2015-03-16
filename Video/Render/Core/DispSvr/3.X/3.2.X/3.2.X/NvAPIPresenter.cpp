#include "stdafx.h"
#include <sstream>
#include "DirectDrawHelper.h"
#include "NvAPIPresenter.h"

#define POLARITY_MASK               (NV_PVFLAG_ODD | NV_PVFLAG_EVEN)
#define INTERLACE_DISPLAY_THRESHOLD 25
#define MIN_PRESENT_INTERVAL        8

static inline bool CanTelecine(DWORD r)
{
	return r == 0 || r == 24 || r == 25 || r == 30 || r == 60;
}

static inline void ConvertRecToNvSBox(NvSBox *Box, const RECT *Rect)
{
	Box->sX = Rect->left;
	Box->sY = Rect->top;
	Box->sWidth = Rect->right - Rect->left;
	Box->sHeight = Rect->bottom - Rect->top;
}

static inline void NormalizeNvSBox(NvSBox *Box, SIZE *szBoundary)
{
	if (Box->sX < 0)
	{
		Box->sWidth += Box->sX;
		Box->sX = 0;
	}

	if (Box->sY < 0)
	{
		Box->sHeight += Box->sY;
		Box->sY = 0;
	}

	if (Box->sX > szBoundary->cx)
	{
		Box->sX = szBoundary->cx;
		Box->sWidth = 0;
	}

	if (Box->sY > szBoundary->cy)
	{
		Box->sY = szBoundary->cy;
		Box->sHeight = 0;
	}

	if (Box->sX+Box->sWidth > szBoundary->cx)
	{
		Box->sWidth = szBoundary->cx - Box->sX;
	}

	if (Box->sY+Box->sHeight > szBoundary->cy)
	{
		Box->sHeight = szBoundary->cy - Box->sY;
	}
}

CNvAPIPresenter::CNvAPIPresenter()
{
	m_GUID = DISPSVR_RESOURCE_NVAPIVIDEOPRESENTER;
	m_bIsPrimaryDisplay = false;
	m_bIsInterlacedDisplay = false;
	m_bNvPresentBusy = false;
	m_bHalfResOnInterlaced = false;
	m_bGamutDataSent = false;
	m_bIsHDMIInfoSet = false;
	m_bIsHDMIColorSpaceChanged = false;
	m_dwFlipQueueHint = 0;
	m_dwMaxUseQueueSize = 0;
	m_dwQueueIdx = 0;
	m_dwPVFlags = 0;
	m_dwFlipTimes = 0;
	m_dwLastPolarity = 0;
	m_dwFrameProperty = 0;
	ZeroMemory(&m_rcClip, sizeof(RECT));
	ZeroMemory(&m_PresentHints, sizeof(PresentHints));
	m_PresentHints.dwFrameRate = 30;
	ZeroMemory(&m_NvSrc, sizeof(NvSBox));
	ZeroMemory(&m_NvClip, sizeof(NvSBox));
	ZeroMemory(&m_NvDst, sizeof(NvSBox));
	ZeroMemory(&m_HDMIInfo, sizeof(NV_HDMI_SUPPORT_INFO));
	ZeroMemory(&m_MetaData, sizeof(NV_GAMUT_METADATA));
	ZeroMemory(&m_hNvDisp, sizeof(NvDisplayHandle));

	m_n32NV_DX_PRESENT_VIDEO_PARAMS_VER = 0;
	ReleaseOverlay();
}

CNvAPIPresenter::~CNvAPIPresenter()
{
}

STDMETHODIMP CNvAPIPresenter::_SetDevice(IUnknown *pDevice)
{
	CHECK_POINTER(pDevice);

	HRESULT hr = E_FAIL;

	hr = CD3D9VideoPresenterBase::_SetDevice(pDevice);
	if (SUCCEEDED(hr))
	{
		CAutoLock lock(&m_csLock);
		hr = InitNvAPI();
		if (FAILED(hr))
        {
            DbgMsg("InitNvAPI fail");
			return hr;
        }

		hr = UpdateDisplayProperty();
		if (FAILED(hr))
			return hr;

		hr = CreateNvAPI();
		if (SUCCEEDED(hr))
		{
            DbgMsg("Creating NvAPI OK!");
			// Update PresenterCaps
			m_PresenterCaps.bIsOverlay = TRUE;
			m_PresenterCaps.dwPresenterInfo = PRESENTER_PROPRIETARYOVERLAY;
		}
		else
		{
            DbgMsg("CreateNvAPI fail");
			DestroyNvAPI();
		}
	}

	return hr;
}

STDMETHODIMP CNvAPIPresenter::_ReleaseDevice()
{
	{
		CAutoLock lock(&m_csLock);
		DestroyNvAPI();
	}
	return CD3D9VideoPresenterBase::_ReleaseDevice();
}

STDMETHODIMP CNvAPIPresenter::SetDisplayRect(const RECT *rcDst, const RECT *rcSrc)
{
	HRESULT hr = CD3D9VideoPresenterBase::SetDisplayRect(rcDst, rcSrc);
	if (SUCCEEDED(hr))
	{
		SIZE szDisplay = {GetRegistry(REG_DISPLAY_WIDTH, 0), GetRegistry(REG_DISPLAY_HEIGHT, 0)};
		hr = CD3D9VideoPresenterBase::CaculateDstClipRect(&m_rcClip, &m_rcDst, &m_rcSrc, szDisplay);
		if (SUCCEEDED(hr))
			m_dwPVFlags |= NV_PVFLAG_SHOW;
		else
			m_dwPVFlags &= ~NV_PVFLAG_SHOW;

		ConvertRecToNvSBox(&m_NvClip, &m_rcClip);
		NormalizeNvSBox(&m_NvClip, &m_szSrc);

		ConvertRecToNvSBox(&m_NvDst, &m_rcDst);
		NormalizeNvSBox(&m_NvDst, &szDisplay);

		// Need to update the NvAPI here?
	}
	return hr;
}

STDMETHODIMP CNvAPIPresenter::BeginRender()
{
	HRESULT hr = CD3D9VideoPresenterBase::BeginRender();
	if (SUCCEEDED(hr))
	{
		CAutoLock lock(&m_csLock);
		if(m_dwMaxUseQueueSize)
		{
			if(m_bNvPresentBusy)
			{
				m_bNvPresentBusy = false;
				return S_FALSE;
			}
			hr = m_pDevice->SetRenderTarget(0, m_pRT[m_dwQueueIdx]);
			m_ObjHandle = m_hObj[m_dwQueueIdx];
			m_dwQueueIdx = (++m_dwQueueIdx) % m_dwMaxUseQueueSize;
		}
	}
	return hr;
}

STDMETHODIMP CNvAPIPresenter::Present(const PresentHints *pHints)
{
	CAutoLock lock(&m_csLock);

	if ((m_dwPVFlags & NV_PVFLAG_SHOW) == 0)
		return S_OK;

	if (pHints)
		m_PresentHints = *pHints;

	if (m_PresenterProperty.dwFlags & PRESENTER_PROPERTY_WAITUNTILPRESENTABLE)
		CD3D9VideoPresenterBase::WaitUntilPresentable();

	// Transform RECT to NvSBox
	if (m_bIsInterlacedDisplay && m_bHalfResOnInterlaced)
	{
		if (!(m_bIsInterlacedDisplay && CanTelecine(m_PresentHints.dwFrameRate)))
		{
			m_PresentHints.dwVideoFlags = VIDEO_PROGRESSIVE;
		}

		return PresentVideo60i(&m_NvSrc, &m_NvClip, &m_NvDst);
	}
	else
		return PresentVideo(&m_NvSrc, &m_NvClip, &m_NvDst);
}

STDMETHODIMP CNvAPIPresenter::Clear()
{
	HRESULT hr = E_FAIL;
	// workaround the overlay surface is not cleared after stopping.
	if (m_pDevice && m_dwMaxUseQueueSize < NV_CV_MIN_OVERLAY_SURFACE_NUMBER)
	{
		m_csLock.Lock();
		// Clear render target with black color.
		hr = m_pDevice->Clear(0L, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0L); ASSERT(SUCCEEDED(hr));
		hr = m_pDevice->BeginScene(); ASSERT(SUCCEEDED(hr));
		hr = m_pDevice->EndScene(); ASSERT(SUCCEEDED(hr));
		m_csLock.Unlock();
		// Flip until queue is filled with black content. note: we flip queue at least 3 times to make sure to update the black screen to the overlay successfully
		for (DWORD i = 0; i < (m_dwFlipQueueHint + 1); i++)
		{
			PresentHints Hints = {0};
			Hints.dwFrameRate = m_PresentHints.dwFrameRate;
			hr = Present(&Hints);
			ASSERT(SUCCEEDED(hr));
		}
	}
	return hr;
}

STDMETHODIMP CNvAPIPresenter::SetColorKey(const DWORD dwColorKey)
{
	HRESULT hr = CD3D9VideoPresenterBase::SetColorKey(dwColorKey);
	if (SUCCEEDED(hr))
	{
		m_dwPVFlags |= NV_PVFLAG_DST_KEY;
	}
	return hr;
}

HRESULT CNvAPIPresenter::InitNvAPI()
{
	CAutoLock lock(&m_csLock);

	NV_DISPLAY_DRIVER_VERSION DriverVersion = {0};
	DriverVersion.version = NV_DISPLAY_DRIVER_VERSION_VER;
	if (NvAPI_GetDisplayDriverVersion(NVAPI_DEFAULT_HANDLE, &DriverVersion) == NVAPI_OK)
	{
		//Workarround : Detect Driver more than 100, if >= 100 set NV_DX_PRESENT_VIDEO_PARAMS_VER2 else set NV_DX_PRESENT_VIDEO_PARAMS_VER1
		if (DriverVersion.drvVersion/100 >= 100)
			m_n32NV_DX_PRESENT_VIDEO_PARAMS_VER = NV_DX_PRESENT_VIDEO_PARAMS_VER;
		else
			m_n32NV_DX_PRESENT_VIDEO_PARAMS_VER = MAKE_NVAPI_VERSION(NV_DX_PRESENT_VIDEO_PARAMS1, 1);
	}

	return S_OK;
}

HRESULT CNvAPIPresenter::CreateNvAPI()
{
	m_dwFlipQueueHint  = m_bIsInterlacedDisplay ? 4 : 2;

	NvAPI_Status nvret = NVAPI_OK;
	HRESULT hr = E_FAIL;
	CComPtr<IDirect3DSurface9> pRenderTarget;
	D3DSURFACE_DESC desc;
	DWORD dwBBWidth = GetRegistry(REG_BACKBUFFER_WIDTH, 0);
	DWORD dwBBHeight = GetRegistry(REG_BACKBUFFER_HEIGHT, 0);

	hr = m_pDevice->GetRenderTarget(0, &pRenderTarget); ASSERT(SUCCEEDED(hr));
	hr = pRenderTarget->GetDesc(&desc); ASSERT(SUCCEEDED(hr));

	desc.Width = dwBBWidth;
	desc.Height = dwBBHeight;

	NV_DX_CREATE_VIDEO_PARAMS CVParams;
	ZeroMemory(&CVParams, sizeof(NV_DX_CREATE_VIDEO_PARAMS));
	CVParams.version = NV_DX_CREATE_VIDEO_PARAMS_VER1;
	CVParams.maxSrcWidth = desc.Width;
	CVParams.maxSrcHeight = desc.Height;
	// m_NvSrc is for unclipped source rect.
	m_NvSrc.sWidth = desc.Width;
	m_NvSrc.sHeight = desc.Height;
	CVParams.cvFlags = NV_CVFLAG_OVERLAY;
	if(m_bIsPrimaryDisplay)
		CVParams.cvFlags &= ~NV_CVFLAG_SECONDARY_DISPLAY;
	else
		CVParams.cvFlags |= NV_CVFLAG_SECONDARY_DISPLAY;

	// Try to use optimized RGBoverlay
	nvret = NvAPI_D3D9_CreateVideoBegin(m_pDevice);
	if (NVAPI_OK == nvret)
	{
		DWORD dwTotal = 256;
        CDirectDrawHelper::QueryVideoMemorySize(&dwTotal, NULL);

		m_dwMaxUseQueueSize = (dwTotal <= 128) ? NV_CV_MIN_OVERLAY_SURFACE_NUMBER : MAX_QUEUE_SIZE;

		CVParams.version = NV_DX_CREATE_VIDEO_PARAMS_VER;
		CVParams.cvFlags |= NV_CVFLAG_EXTERNAL_OVERLAY;
		CVParams.dwNumOvlSurfs = m_dwMaxUseQueueSize;

		hr = CreateOverlay(&desc, &CVParams);
		if (SUCCEEDED(hr))
			nvret = NvAPI_D3D9_CreateVideo(m_pDevice, &CVParams);
		else
			nvret = NVAPI_ERROR;

		if(nvret == NVAPI_OK)
		{
			NvAPI_D3D9_CreateVideoEnd(m_pDevice);
			// When using pinned down surfaces, desktop virtualization must be used.
			m_PresenterCaps.bCanVirtualizeFromOrigin = FALSE;
		}
		else
		{
			// reset some parameters and fall back to original NvAPI_D3D9_CreateVideo
			ReleaseOverlay();
			m_dwMaxUseQueueSize = 0;
		}
	}

	// Fall back to original RGBoverlay if fail to use optimized RGBoveraly
	if (m_dwMaxUseQueueSize == 0)
	{
		m_PresenterCaps.bCanVirtualizeFromOrigin = TRUE;
		CVParams.version = NV_DX_CREATE_VIDEO_PARAMS_VER1;
		CVParams.flipQueueHint = m_dwFlipQueueHint;
		CVParams.dwNumOvlSurfs = 0;

		nvret = NvAPI_D3D9_CreateVideo(m_pDevice, &CVParams);
		// make NvAPI_D3D9_CreateVideoBegin/NvAPI_D3D9_CreateVideoEnd as pair, no matter CreateVideo succeeded or failed
		NvAPI_D3D9_CreateVideoEnd(m_pDevice);
		if (nvret != NVAPI_OK)
		{
			ASSERT(0);
			return (nvret == NVAPI_NO_IMPLEMENTATION) ? E_NOTIMPL : DDERR_NOOVERLAYHW;
		}

		m_dwMaxUseQueueSize = 1;
		hr = CreateOverlay(&desc, &CVParams);
	}

	DWORD dwDeviceID = GetRegistry(REG_DEVICE_ID, 0);

	//Detect the DeviceId is G7X series or not
	if (dwDeviceID >= PCI_DEVICE_ID_G72_BEGIN && dwDeviceID <= PCI_DEVICE_ID_G73_END)
	{
//			m_bNoSyncFlips = true;
		m_bHalfResOnInterlaced = true;
	}

	return S_OK;
}

HRESULT CNvAPIPresenter::DestroyNvAPI()
{
	NvAPI_Status nvret = NVAPI_OK;

	if (m_pDevice)
	{
		nvret = NvAPI_D3D9_FreeVideo(m_pDevice);
		ASSERT(nvret == NVAPI_OK);
	}

	HRESULT hr = ClearGamutMetadata();
	ReleaseOverlay();
	m_dwMaxUseQueueSize = 0;
	return hr;
}

HRESULT CNvAPIPresenter::PresentVideo(const NvSBox *pNvSrc, const NvSBox *pNvClip, const NvSBox *pNvDst)
{
	if (m_ObjHandle != NVDX_OBJECT_NONE)
	{
		NvAPI_Status nvret = NVAPI_OK;
		NV_DX_PRESENT_VIDEO_PARAMS PVParams = {0};
		int i = 0;

		PVParams.version = m_n32NV_DX_PRESENT_VIDEO_PARAMS_VER;
		PVParams.surfaceHandle = m_ObjHandle;
		PVParams.pvFlags = m_dwPVFlags | NV_PVFLAG_PROGRESSIVE;
		PVParams.colourKey = m_dwColorKey;
		PVParams.timeStampLow = 0;
		PVParams.timeStampHigh = 0;
		PVParams.flipRate = m_PresenterCaps.dwFPS;
		PVParams.srcUnclipped = *pNvSrc;
		PVParams.srcClipped = *pNvClip;
		PVParams.dst = *pNvDst;

		for (nvret = NVAPI_DEVICE_BUSY; nvret == NVAPI_DEVICE_BUSY && i < 8; i++)
		{
			nvret = NvAPI_D3D9_PresentVideo(m_pDevice, &PVParams);
			if(nvret == NVAPI_DEVICE_BUSY)
			{
				DbgMsg("NvAPI: Device busy, Sleep 2 msec.");
				Sleep(2);
			}
		}

		if (m_dwMaxUseQueueSize && nvret == NVAPI_DEVICE_BUSY)
			m_bNvPresentBusy = true;

#ifdef _DEBUG
		if (nvret == NVAPI_DEVICE_BUSY)
			DbgMsg("NvAPI: device busy for %d times, abort.\n", i);
#endif

		if (nvret != NVAPI_OK && nvret != NVAPI_DEVICE_BUSY)
		{
			return (nvret == NVAPI_NO_IMPLEMENTATION) ? E_NOTIMPL : E_FAIL;
		}
	}

	return S_OK;
}

HRESULT CNvAPIPresenter::PresentVideo60i(const NvSBox *pNvSrc, const NvSBox *pNvClip, const NvSBox *pNvDst)
{
	if (m_ObjHandle != NVDX_OBJECT_NONE)
	{
		NV_DX_PRESENT_VIDEO_PARAMS PVParams = {0};
		NvAPI_Status nvret = NVAPI_NO_IMPLEMENTATION;
		DWORD pPolarity[4] = {0, 0, 0, 0};
		DWORD dwPresentTimes = 2;
		DWORD dwProgressive = NV_PVFLAG_PROGRESSIVE;

		PVParams.version = m_n32NV_DX_PRESENT_VIDEO_PARAMS_VER;
		PVParams.surfaceHandle = m_ObjHandle;
		PVParams.colourKey = m_dwColorKey;
		PVParams.timeStampLow = 0;
		PVParams.timeStampHigh = 0;
		PVParams.flipRate = 60;	// fix fliprate to 60
		PVParams.srcUnclipped = *pNvSrc;
		PVParams.srcClipped = *pNvClip;
		PVParams.dst = *pNvDst;
/*
		// hardware can do deinterlace for us, but in most cases, we do not want it.
		if (m_bUseOverlayDeinterlace && (m_dwFrameProperty & OR_FRAME_PROGRESSIVE) == 0)
			dwProgressive = NV_PVFLAG_PROGRESSIVE;
*/
		switch (m_PresentHints.dwFrameRate) 
		{
		case 24:
			m_dwFlipTimes = (m_dwFlipTimes + 1) % 4;
			switch (m_dwFlipTimes) 
			{
			case 0:
				pPolarity[0] = NV_PVFLAG_ODD;
				pPolarity[1] = NV_PVFLAG_EVEN;
				break;
			case 1:
				pPolarity[0] = NV_PVFLAG_ODD;
				pPolarity[1] = NV_PVFLAG_EVEN;
				pPolarity[2] = NV_PVFLAG_ODD;
				dwPresentTimes = 3;
				break;
			case 2:
				pPolarity[0] = NV_PVFLAG_EVEN;
				pPolarity[1] = NV_PVFLAG_ODD;
				break;
			case 3:
				pPolarity[0] = NV_PVFLAG_EVEN;
				pPolarity[1] = NV_PVFLAG_ODD;
				pPolarity[2] = NV_PVFLAG_EVEN;
				dwPresentTimes = 3;
				break;
			default:
				ASSERT(0 && "24 fps m_dwFlipTimes is not inbetween 0 and 3\n");
			}
			break;
		case 25:
			{
				DWORD dwFirst, dwSecond;

				if (m_PresentHints.dwVideoFlags & VIDEO_EVEN_FIELD_FIRST)
				{
					dwFirst = NV_PVFLAG_EVEN;
					dwSecond = NV_PVFLAG_ODD;
				} else {	// progressive treated as top field first
					dwFirst = NV_PVFLAG_ODD;
					dwSecond = NV_PVFLAG_EVEN;
				}

				m_dwFlipTimes = (m_dwFlipTimes + 1) % 5;
				switch (m_dwFlipTimes) 
				{
				case 0:
				case 1:
					pPolarity[0] = dwFirst;
					pPolarity[1] = dwSecond;
					break;
				case 2:
					pPolarity[0] = dwFirst;
					pPolarity[1] = dwSecond;	// repeated
					pPolarity[2] = dwFirst;
					dwPresentTimes = 3;
					break;
				case 3:
					pPolarity[0] = dwSecond;
					pPolarity[1] = dwFirst;
					break;
				case 4:
					pPolarity[0] = dwSecond;
					pPolarity[1] = dwFirst;
					pPolarity[2] = dwSecond;	// repeated
					dwPresentTimes = 3;
					break;
				default:
					ASSERT(0 && "25 fps m_dwFlipTimes is not inbetween 0 and 4\n");
				}
			}
			break;
		case 30:
			if (m_PresentHints.dwVideoFlags & VIDEO_ODD_FIELD_FIRST)
			{
				pPolarity[0] = NV_PVFLAG_EVEN;
				pPolarity[1] = NV_PVFLAG_ODD;
			}
			else if ((m_PresentHints.dwVideoFlags & VIDEO_EVEN_FIELD_FIRST) || m_dwLastPolarity == 0)
			{
				pPolarity[0] = NV_PVFLAG_ODD;
				pPolarity[1] = NV_PVFLAG_EVEN;
			}
			else
			{		// progressive
				ASSERT(m_dwLastPolarity != 0);
				pPolarity[0] = (m_dwLastPolarity ^ POLARITY_MASK) & POLARITY_MASK;
				pPolarity[1] = m_dwLastPolarity;
			}

			if (m_PresentHints.bRepeatFirstField)
			{
				pPolarity[2] = pPolarity[0];
				dwPresentTimes = 3;
			}
			break;
		case 60:
			dwPresentTimes = 1;
			if (m_PresentHints.dwVideoFlags & VIDEO_PROGRESSIVE)
			{
				m_dwFlipTimes = (m_dwFlipTimes + 1) % 2;
				pPolarity[0] = m_dwFlipTimes ? NV_PVFLAG_EVEN : NV_PVFLAG_ODD;
			}
			else if (m_PresentHints.dwVideoFlags & VIDEO_ODD_FIELD_FIRST)
				pPolarity[0] = NV_PVFLAG_EVEN;
			else
				pPolarity[0] = NV_PVFLAG_ODD;
			break;
		case 0:	// still image/pause case
			PVParams.flipRate = 0;
			dwPresentTimes = 1;
			break;
		default:
			dwPresentTimes = 1;
		}

		ASSERT(dwPresentTimes <= 3);
		for (DWORD dwPresentCount = 0; dwPresentCount < dwPresentTimes; dwPresentCount++)
		{
			int i = 0;

			PVParams.pvFlags = m_dwPVFlags | pPolarity[dwPresentCount] | dwProgressive;
			for (nvret = NVAPI_DEVICE_BUSY; nvret == NVAPI_DEVICE_BUSY && i < 8; i++)
			{
				nvret = NvAPI_D3D9_PresentVideo(m_pDevice, &PVParams);
				if (nvret == NVAPI_DEVICE_BUSY)
					Sleep(PVParams.pvFlags & NV_PVFLAG_REPEAT ? MIN_PRESENT_INTERVAL/2 : MIN_PRESENT_INTERVAL);
			}


			if (m_dwMaxUseQueueSize && nvret == NVAPI_DEVICE_BUSY)
				m_bNvPresentBusy = true;

#ifdef _DEBUG
			if (nvret == NVAPI_DEVICE_BUSY)
				DbgMsg("NvAPI: device busy for %d times, abort.\n", i);
#endif

			if (nvret != NVAPI_OK && nvret != NVAPI_DEVICE_BUSY)
			{
				DbgMsg("NvAPI: Present video failed! error=%x\n", nvret);
				return (nvret == NVAPI_NO_IMPLEMENTATION) ? E_NOTIMPL : E_FAIL;
			}

			m_dwLastPolarity = pPolarity[dwPresentCount] & POLARITY_MASK;

			// it is for nvidia testing to hint driver to use previous frame
			dwProgressive |= NV_PVFLAG_REPEAT;
		}
	}
	return S_OK;
}

HRESULT CNvAPIPresenter::UpdateDisplayProperty()
{
	HMONITOR hMonitor = MonitorFromWindow(m_hwnd, (m_hwnd) ? MONITOR_DEFAULTTONEAREST : MONITOR_DEFAULTTOPRIMARY);
	MONITORINFOEX MonInfo;
	ZeroMemory(&MonInfo, sizeof(MONITORINFOEX));
	MonInfo.cbSize = sizeof(MONITORINFOEX);
	if (!GetMonitorInfo(hMonitor, &MonInfo))  // use your own hMonitor
	{
		ASSERT(0);
	}

	NvAPI_Status nvret = NVAPI_OK;
	NvDisplayHandle hNvDisp = NULL;
	USES_CONVERSION;
	nvret = NvAPI_GetAssociatedNvidiaDisplayHandle(T2A(MonInfo.szDevice), &hNvDisp);
	if (NVAPI_OK  == nvret)
	{
		if (m_PresenterCaps.bSupportXvYCC)
		{
			m_HDMIInfo.version = NV_HDMI_SUPPORT_INFO_VER;
			// outputId = "0", the default outputId from NvAPI_GetAssociatedDisplayOutputId() will be used.
			nvret = NvAPI_GetHDMISupportInfo(hNvDisp, 0, &m_HDMIInfo);
			if (nvret != NVAPI_OK || !(m_HDMIInfo.isMonxvYCC601Capable || m_HDMIInfo.isMonxvYCC709Capable))
				return E_FAIL;
		}

		m_hNvDisp = hNvDisp;
		NvU32 TargetCount = NVAPI_MAX_VIEW_TARGET;
		NV_TARGET_VIEW_MODE TargetViewMode = NV_VIEW_MODE_STANDARD;
		NV_VIEW_TARGET_INFO TargetInfo;
		memset(&TargetInfo, 0, sizeof(TargetInfo));
		TargetInfo.version = NV_VIEW_TARGET_INFO_VER;
		nvret = NvAPI_GetView(hNvDisp, &TargetInfo, &TargetCount,&TargetViewMode);
		if(NVAPI_OK  == nvret)
		{
			DWORD deviceMask;
			nvret = NvAPI_GetAssociatedDisplayOutputId(hNvDisp,&deviceMask);
			if(NVAPI_OK  == nvret)
			{
				DWORD dwTarget = 0;
				for (dwTarget=0; dwTarget<TargetCount; dwTarget++)
				{
					if (TargetInfo.target[dwTarget].deviceMask == deviceMask)
						break;
				}

				if (dwTarget < TargetCount)     // only valid if dwTarget is less than targetCount
				{
					m_bIsPrimaryDisplay = TargetInfo.target[dwTarget].bPrimary;
					m_bIsInterlacedDisplay = TargetInfo.target[dwTarget].bInterlaced;
				}
			}
		}
	}

	CD3D9VideoPresenterBase::UpdateScanLineProperty();
	if (nvret != NVAPI_OK)
	{
		m_bIsPrimaryDisplay = (MonInfo.szDevice[11] == '1') ? true : false;
		// we do not take vblank into consideration but the result may still reliable.
		m_bIsInterlacedDisplay = (INTERLACE_DISPLAY_THRESHOLD < m_fScanLineInterval * GetRegistry(REG_DISPLAY_HEIGHT, 0));
	}

	return S_OK;
}

STDMETHODIMP CNvAPIPresenter::SetGamutMetadata(const DWORD dwFormat, void *pGamutMetadata)
{
	NvAPI_Status nvret = NVAPI_OK;

	if (!m_PresenterCaps.bSupportXvYCC)
	{
		DbgMsg("NvAPI: No XVYCC supports.");
		return S_FALSE;
	}

	m_dwGamutFormat = dwFormat;
	if (m_dwGamutFormat == GAMUT_METADATA_NONE)
	{
		ZeroMemory(&m_GamutRange, sizeof(GamutMetadataRange));
		ZeroMemory(&m_GamutVertices, sizeof(GamutMetadataVertices));
        DbgMsg("NvAPI: XvYcc OFF. There is no gamut metadata.");
		return ClearGamutMetadata();
	}

	if(!m_bIsHDMIInfoSet)
	{
		NV_INFOFRAME NvInfoFrame;
		nvret = NvAPI_GetInfoFrame(m_hNvDisp, 0, NV_INFOFRAME_TYPE_AVI, &NvInfoFrame); // outputId = "0", the default outputId from NvAPI_GetAssociatedDisplayOutputId() will be used.
		if(nvret != NVAPI_OK)
        {
            DbgMsg("NvAPI: XvYcc OFF. Failed to get info frame.");
			return E_FAIL;
        }

		// if the HDMI output format is RGB, change it to YCbCr(4:4:4)
		if(NvInfoFrame.u.video.colorSpace == 0)
		{
			NvInfoFrame.u.video.colorSpace = 2;		// 0: RGB,  1: YCbCr(4:2:0),  2: YCbCr(4:4:4),  3: future (HDMI spec)
			nvret = NvAPI_SetInfoFrame(m_hNvDisp, 0, NV_INFOFRAME_TYPE_AVI, &NvInfoFrame);	// outputId = "0", the default outputId from NvAPI_GetAssociatedDisplayOutputId() will be used.
			if(nvret != NVAPI_OK)
			{
				DbgMsg("NvAPI: XvYcc OFF. Failed to change HDMI output format from RGB to YUV.");
				return E_FAIL;
			}
			m_bIsHDMIColorSpaceChanged = true;
		}
		m_bIsHDMIInfoSet = true;
	}

	if (m_bGamutDataSent)
	{
		// if the GBD packet is different from the previous one
		// set "m_bGamutDataSent = false" to resend the GBD packet
		GamutMetadataRange metaData;
		memcpy(&metaData, pGamutMetadata, sizeof(GamutMetadataRange));
		if( memcmp(&m_GamutRange, &metaData, sizeof(m_GamutRange)) )
			m_bGamutDataSent = false;
	}

	if (!m_bGamutDataSent && m_dwGamutFormat == GAMUT_METADATA_RANGE)
	{
		memcpy(&m_GamutRange, pGamutMetadata, sizeof(GamutMetadataRange));

		m_MetaData.data.rangeData.Format_Flag = m_GamutRange.Format_Flag;
		m_MetaData.data.rangeData.GBD_Color_Precision = m_GamutRange.GBD_Color_Precision;
		m_MetaData.data.rangeData.GBD_Color_Space = m_GamutRange.GBD_Color_Space;
		m_MetaData.data.rangeData.Rsvd = 0;

		m_MetaData.data.rangeData.Max_Red_Data = m_GamutRange.Max_Red_Data;
		m_MetaData.data.rangeData.Min_Red_Data = m_GamutRange.Min_Red_Data;
		m_MetaData.data.rangeData.Max_Green_Data = m_GamutRange.Max_Green_Data;
		m_MetaData.data.rangeData.Min_Green_Data = m_GamutRange.Min_Green_Data;
		m_MetaData.data.rangeData.Max_Blue_Data = m_GamutRange.Max_Blue_Data;
		m_MetaData.data.rangeData.Min_Blue_Data = m_GamutRange.Min_Blue_Data;

        NvU32 AssociatedDisplayOutputId = 0;
        nvret = NvAPI_GetAssociatedDisplayOutputId(m_hNvDisp, &AssociatedDisplayOutputId);
        if (nvret != NVAPI_OK)
        {
            DbgMsg("NvAPI: XvYcc OFF. Failed to get associated display output Id.");
            return E_FAIL;
        }

        nvret = NvAPI_D3D9_SetGamutData(m_pDevice, AssociatedDisplayOutputId, NV_GAMUT_FORMAT_RANGE, &m_MetaData);
		if(nvret != NVAPI_OK)
		{
			DbgMsg("NvAPI: XvYcc OFF. Failed to set gamut packet.");
			// According to NVidia, they may disable gamut packet transmission based on OEM's requirement.
			// So, we treat the driver as no XvYcc support if it failed while calling NvAPI_D3D9_SetGamutData().
			m_PresenterCaps.bSupportXvYCC = FALSE;
			ClearGamutMetadata();
			return E_FAIL;
		}
		
		DbgMsg("NvAPI: XvYcc ON.");
		m_bGamutDataSent = true;
        SetRegistry(REG_DISPLAY_XVYCC_GAMUT, TRUE);
	}

	return S_OK;
}

HRESULT CNvAPIPresenter::CreateOverlay(D3DSURFACE_DESC *desc, NV_DX_CREATE_VIDEO_PARAMS *CVParams)
{
	NvAPI_Status nvret = NVAPI_ERROR;
	HRESULT hr = S_OK;

	for (DWORD i = 0; i < m_dwMaxUseQueueSize; i++)
	{
		hr = m_pDevice->CreateRenderTarget(
			desc->Width,
			desc->Height,
			desc->Format,
			D3DMULTISAMPLE_NONE,
			0,
			FALSE,		// create lockable render target unless we want to force sync with G7x overlay by LockRect().
			&m_pRT[i],
			NULL);
		if (SUCCEEDED(hr))
		{
			// Get Surface Handle
			nvret = NvAPI_D3D9_GetSurfaceHandle(m_pRT[i], &CVParams->hOvlSurfs[i]);
			m_hObj[i] = CVParams->hOvlSurfs[i];
		}
	}

	if(nvret != NVAPI_OK)
		return E_FAIL;
	else
		return S_OK;
}

void CNvAPIPresenter::ReleaseOverlay()
{
	for (DWORD i = 0; i < m_dwMaxUseQueueSize; i++)
	{
		SAFE_RELEASE(m_pRT[i]);
		m_hObj[i] = NVDX_OBJECT_NONE;
	}

	m_ObjHandle = NVDX_OBJECT_NONE;
}

HRESULT CNvAPIPresenter::ClearGamutMetadata()
{
	NvAPI_Status nvret = NVAPI_OK;

	if (m_bGamutDataSent)
	{
        NvU32 AssociatedDisplayOutputId = 0;
        nvret = NvAPI_GetAssociatedDisplayOutputId(m_hNvDisp, &AssociatedDisplayOutputId);
        if (nvret != NVAPI_OK)
        {
            DbgMsg("NvAPI: XvYcc OFF. Failed to get associated display output Id.");
        }

		nvret = NvAPI_D3D9_SetGamutData(m_pDevice, AssociatedDisplayOutputId, NV_GAMUT_FORMAT_RANGE, NULL); 
		m_bGamutDataSent = false;
		if(nvret != NVAPI_OK)
		{
			DbgMsg("NvAPI: Clear Gamut Metadata Failed.");
		}
		else
		{
			DbgMsg("NvAPI: XvYcc OFF. Clear Gamut Metadata.");
		}
	}

	if(m_bIsHDMIInfoSet && m_bIsHDMIColorSpaceChanged )
	{
		// Cannot use the previous NV_INFOFRAME structure which is retrieved in the previous call,
		// Cal NvAPI_GetInfoFrame() to update the NV_INFOFRAME structure.
		NV_INFOFRAME NvInfoFrame;
		nvret = NvAPI_GetInfoFrame(m_hNvDisp, 0, NV_INFOFRAME_TYPE_AVI, &NvInfoFrame);

		// restore HDMI output format back to RGB
		NvInfoFrame.u.video.colorSpace = 0;
		nvret = NvAPI_SetInfoFrame(m_hNvDisp, 0, NV_INFOFRAME_TYPE_AVI, &NvInfoFrame); // outputId = "0", the default outputId from NvAPI_GetAssociatedDisplayOutputId() will be used.
		if(nvret != NVAPI_OK)
		{
			DbgMsg("NvAPI: Failed to restored HDMI Output format from YUV to RGB.");
		}
		else
		{
			DbgMsg("NvAPI: Restored HDMI Output format from YUV to RGB.");
		}
	}
	m_bIsHDMIInfoSet = false;
    SetRegistry(REG_DISPLAY_XVYCC_GAMUT, FALSE);
	return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
/// CNvAPIDeviceExtensionAdapter
CNvAPIDeviceExtensionAdapter::CNvAPIDeviceExtensionAdapter()
{
    m_pDevice9 = NULL;
    ZeroMemory(&m_NvMainVideo_Info, sizeof(NVAPI_VIDEO_SRC_INFO));
}

CNvAPIDeviceExtensionAdapter::CNvAPIDeviceExtensionAdapter(IDirect3DDevice9 *pDevice)
{
	m_pDevice9 = pDevice;
	m_pDevice9->AddRef();
	ZeroMemory(&m_NvMainVideo_Info, sizeof(NVAPI_VIDEO_SRC_INFO));
}

CNvAPIDeviceExtensionAdapter::~CNvAPIDeviceExtensionAdapter()
{
	if (m_pDevice9)
	{
		m_pDevice9->Release();
		m_pDevice9 = 0;
	}
}

HRESULT CNvAPIDeviceExtensionAdapter::SetDevice(IDirect3DDevice9 *pDevice9)
{
    if (m_pDevice9 != pDevice9)
    {
        SAFE_RELEASE(m_pDevice9);

        m_pDevice9 = pDevice9;

        if (m_pDevice9)
            m_pDevice9->AddRef();
    }
    return S_OK;
}

HRESULT CNvAPIDeviceExtensionAdapter::GetAdapter(IDriverExtensionAdapter **ppAdapter)
{
//	CHECK_POINTER(pDevice9);
	NvAPI_Status nvret = NvAPI_Initialize();

	*ppAdapter = 0;
	if (nvret != NVAPI_OK)
		return E_NOTIMPL;

	*ppAdapter = new CNvAPIDeviceExtensionAdapter();
	return S_OK;
}

HRESULT CNvAPIDeviceExtensionAdapter::QueryPresenterCaps(DWORD VideoDecodeCaps, PresenterCaps* pCaps)
{
	if (VideoDecodeCaps > 0)
	{
		NV_DX_VIDEO_CAPS caps = {0};
		NVAPI_VIDEO_CAPS_PACKET &packet = caps.videoCapsPacket;
		NvAPI_Status nvret = NVAPI_NO_IMPLEMENTATION;
		DWORD dwSrcWidth = 0, dwSrcHeight = 0;
		NV_CODEC NvCodcType = NV_CODEC_TYPE_NONE;
		BOOL bDualDxvaDriver = FALSE;
		UINT uDisplayWidth = GetRegistry(REG_DISPLAY_WIDTH, 0);
		UINT uDisplayHeight = GetRegistry(REG_DISPLAY_HEIGHT, 0);

		// The official WHQL driver supports DualDXVA is 174.53.
		NV_DISPLAY_DRIVER_VERSION DriverVersion = {0};
		DriverVersion.version = NV_DISPLAY_DRIVER_VERSION_VER;
		nvret = NvAPI_GetDisplayDriverVersion( NVAPI_DEFAULT_HANDLE, &DriverVersion );
		if (nvret == NVAPI_OK && DriverVersion.drvVersion >= 17453)
			bDualDxvaDriver = TRUE;

		caps.version = NV_DX_VIDEO_CAPS_VER;
		packet.packetVer = NVAPI_VIDEO_CAPS_PACKET_VER;
		packet.renderMode = 1 << RENDER_MODE_OVERLAY_BIT;
		packet.res[0].width = uDisplayWidth;
		packet.res[0].height = uDisplayHeight;
		packet.res[0].bitsPerPixel = 32;
		packet.res[0].refreshRate = GetRegistry(REG_DISPLAY_REFRESH_RATE, 0);

		if(VideoDecodeCaps & VIDEO_CAP_FORMAT_1080)
		{
			dwSrcHeight = 1088;
			dwSrcWidth = 1920;	//1440 or 1920
		}
		else if(VideoDecodeCaps & VIDEO_CAP_FORMAT_480)
		{
			dwSrcHeight = 480;
			dwSrcWidth = 720;
		}
		else if(VideoDecodeCaps & VIDEO_CAP_FORMAT_576)
		{
			dwSrcHeight = 576;
			dwSrcWidth = 720;
		}
		else if(VideoDecodeCaps & VIDEO_CAP_FORMAT_720)
		{
			dwSrcHeight = 720;
			dwSrcWidth = 1280;
		}

		ASSERT(dwSrcHeight != 0);

		if(VideoDecodeCaps & VIDEO_CAP_CODEC_MPEG2)
			NvCodcType = NV_CODEC_TYPE_MPEG2;
		else if(VideoDecodeCaps & VIDEO_CAP_CODEC_H264)
			NvCodcType = NV_CODEC_TYPE_H264;
		else if(VideoDecodeCaps & VIDEO_CAP_CODEC_VC1)
			NvCodcType = NV_CODEC_TYPE_VC1;

		ASSERT(NvCodcType != NV_CODEC_TYPE_NONE);

		//When Query MainStream, we need to clear all information of vidSrc besides MainStream.
		if(!(VideoDecodeCaps & VIDEO_CAP_STREAM_SUB))
		{
			m_NvMainVideo_Info.srcWidth = packet.vidSrcInfo[0].srcWidth = dwSrcWidth;
			m_NvMainVideo_Info.srcHeight = packet.vidSrcInfo[0].srcHeight = dwSrcHeight;
			m_NvMainVideo_Info.codecType = packet.vidSrcInfo[0].codecType = NvCodcType;
			packet.numVidStreams = 1;
		}
		else if (bDualDxvaDriver)
		{
			//Video_Info of Stream Main
			packet.vidSrcInfo[0].srcWidth = m_NvMainVideo_Info.srcWidth;
			packet.vidSrcInfo[0].srcHeight = m_NvMainVideo_Info.srcHeight;
			packet.vidSrcInfo[0].codecType = m_NvMainVideo_Info.codecType;
			//Video_Info of Stream Sub
			packet.vidSrcInfo[1].srcWidth = dwSrcWidth;
			packet.vidSrcInfo[1].srcHeight = dwSrcHeight;
			packet.vidSrcInfo[1].codecType = NvCodcType;
			packet.numVidStreams = 2;
		}

		nvret = NvAPI_D3D9_GetVideoCapabilities(m_pDevice9, &caps);
		if (nvret == NVAPI_OK)
		{
			pCaps->dwFPS = packet.videoCaps[0].maxFlipRate;
			//m_bNoSyncFlips = (packet.videoCaps[0].vidFeature & (1 << NV_VID_FEATURE_NO_SYNC_FLIPS_BIT)) != 0;
			// m_bHalfResOnInterlaced = (packet.videoCaps[0].vidFeature & (1 << NV_VID_FEATURE_HALF_RES_ON_INTERLACED_BIT)) != 0;
			pCaps->dwResPixels = (packet.videoCaps[0].maxResPixels) ? packet.videoCaps[0].maxResPixels : uDisplayWidth * uDisplayHeight;

			if (!(VideoDecodeCaps & VIDEO_CAP_STREAM_SUB))
			{
				// In 111.28 driver, packet.videoCaps[0].hwDecode is always NV_CODEC_TYPE_NONE.
				// Hence, we can't use hwDecode to determine driver can't support DXVA of main video.
				pCaps->bHwDecode = TRUE;
			}
			else
			{
				pCaps->bHwDecode = (packet.videoCaps[1].hwDecode != NV_CODEC_TYPE_NONE) ? TRUE : FALSE;
			}
		}
		else
		{
			pCaps->dwResPixels = uDisplayWidth * uDisplayHeight;
			pCaps->bHwDecode = (!(VideoDecodeCaps & VIDEO_CAP_STREAM_SUB)) ? TRUE : FALSE;
		}
	}

	return S_OK;
}

HRESULT CNvAPIDeviceExtensionAdapter::SetStereoInfo(IDirect3DSurface9 *pBaseView, IDirect3DSurface9 *pDependentView, INT iOffset, BOOL bStereoEnable, MIXER_STEREO_MODE stereoMixingMode)
{
	ASSERT(m_pDevice9);
	CHECK_POINTER(pBaseView);

	NV_DX_VIDEO_STEREO_INFO sInfo;
	NvAPI_Status nvret;
	HRESULT hr;

	ZeroMemory(&sInfo, sizeof(sInfo));
	sInfo.dwVersion = NV_DX_VIDEO_STEREO_INFO_VER;
	sInfo.eFormat = NV_STEREO_VIDEO_FORMAT_NOT_STEREO;
	sInfo.bStereoEnable = bStereoEnable;
    sInfo.sViewOffset = -iOffset; // for NV api, positive offset indicating left view is shifted left, it's inverse of 3DBD spec.

	hr = LockupSurfaceHandle(pBaseView, &sInfo.hSurface);
	if (FAILED(hr))
		return hr;

	if (sInfo.bStereoEnable)
	{
		if (pDependentView)
		{
			hr = LockupSurfaceHandle(pDependentView, &sInfo.hLinkedSurface);
			if (FAILED(hr))
				return hr;
			sInfo.eFormat = NV_STEREO_VIDEO_FORMAT_TWO_FRAMES_LR;
		}
		else
		{
			switch (stereoMixingMode)
			{
			case MIXER_STEREO_MODE_HALF_SIDEBYSIDE_LR:
				{
					sInfo.eFormat = NV_STEREO_VIDEO_FORMAT_SIDE_BY_SIDE_LR;
				}
				break;
			case MIXER_STEREO_MODE_HALF_SIDEBYSIDE_RL:
				{
					sInfo.eFormat = NV_STEREO_VIDEO_FORMAT_SIDE_BY_SIDE_RL;
				}
				break;
			case MIXER_STEREO_MODE_HALF_TOPBOTTOM_LR:
				{
					sInfo.eFormat = NV_STEREO_VIDEO_FORMAT_TOP_BOTTOM_LR;
				}
				break;
			case MIXER_STEREO_MODE_HALF_TOPBOTTOM_RL:
				{
					sInfo.eFormat = NV_STEREO_VIDEO_FORMAT_TOP_BOTTOM_LR;
				}
				break;
			default:
				{
			        sInfo.eFormat = NV_STEREO_VIDEO_FORMAT_MONO_PLUS_OFFSET;
		        }
				break;
			}
		}
	}

	nvret = NvAPI_D3D9_VideoSetStereoInfo(m_pDevice9, &sInfo);

	return (nvret == NVAPI_OK) ? S_OK : E_FAIL;
}

HRESULT CNvAPIDeviceExtensionAdapter::QueryHDMIStereoModeCaps(HWND hWnd, DriverExtHDMIStereoModeCap **ppCaps, UINT *puCount)
{
    CHECK_POINTER(ppCaps);
    CHECK_POINTER(puCount);

    HRESULT hr = E_FAIL;
    NvAPI_Status nvret = NVAPI_OK;

//    NV_HDMI_SUPPORT_INFO HDMIInfo;
//    NV_GAMUT_METADATA m_MetaData;
    NvDisplayHandle hNvDisp;
    NvPhysicalGpuHandle nvGPUHandle[NVAPI_MAX_PHYSICAL_GPUS] = {0};
    NvU32 GPUCount = 0;
    NvU32 outputId = 0;

    NV_HDMI_STEREO_MODES_LIST ModeList;
    ZeroMemory(&ModeList, sizeof(NV_HDMI_STEREO_MODES_LIST));
    NvU32 displayId = 0;

    HMONITOR hMonitor = MonitorFromWindow(hWnd, (hWnd) ? MONITOR_DEFAULTTONEAREST : MONITOR_DEFAULTTOPRIMARY);
    MONITORINFOEX MonInfo;
    ZeroMemory(&MonInfo, sizeof(MONITORINFOEX));
    MonInfo.cbSize = sizeof(MONITORINFOEX);
    if (!GetMonitorInfo(hMonitor, &MonInfo))  // use your own hMonitor
    {
        ASSERT(0);
    }

    USES_CONVERSION;
    //    nvret = NvAPI_GetAssociatedNvidiaDisplayHandle(T2A(MonInfo.szDevice), &hNvDisp);
    if (NvAPI_GetAssociatedNvidiaDisplayHandle(T2A(MonInfo.szDevice), &hNvDisp) != NVAPI_OK)
    {
        DbgMsg("NVAPI Failed to get Display Handle - NvAPI_GetAssociatedNvidiaDisplayHandle");
        return E_FAIL;
    }
    if (NvAPI_GetPhysicalGPUsFromDisplay(hNvDisp, nvGPUHandle, &GPUCount) != NVAPI_OK)
    {
        DbgMsg("NVAPI Failed to get Physical GPU - NvAPI_GetPhysicalGPUsFromDisplay");
        return E_FAIL;
    }
    if (NvAPI_GetAssociatedDisplayOutputId(hNvDisp, &outputId) != NVAPI_OK)
    {
        DbgMsg("NVAPI Failed to get Display Output ID - NvAPI_GetAssociatedDisplayOutputId");
        return E_FAIL;
    }
     if (NvAPI_SYS_GetDisplayIdFromGpuAndOutputId(nvGPUHandle[0], outputId, &displayId) != NVAPI_OK)
     {
         DbgMsg("NVAPI Failed to get Display ID - NvAPI_SYS_GetDisplayIdFromGpuAndOutputId");
         return E_FAIL;
     }


//    nvret = NvAPI_DISP_GetDisplayIdByDisplayName(T2A(MonInfo.szDevice), &displayId);
    if (nvret == NVAPI_OK)
    {
        ModeList.version = NV_HDMI_STEREO_MODES_LIST_VER;
        ModeList.displayId = displayId;
        ModeList.enumIndex = 0;
        ModeList.width = 0; // since bMatchDimension is false, this item can be ignored
        ModeList.height = 0; // since bMatchDimension is false, this item can be ignored
        ModeList.bMatchDimension = false;
        ModeList.refreshRate = 0; // since bMatchRR is false, this item can be ignored
        ModeList.bMatchRR = false;
        ModeList.count = NVAPI_HDMI_STEREO_MAX_MODES;
        nvret = NvAPI_DISP_EnumHDMIStereoModes(&ModeList);
        if (nvret == NVAPI_OK)
        {
            DWORD ModeMap[] = 
            {
                HDMI_STEREO_NONE,                        //    HDMI_STEREO_EXT_NONE}, // index 0, NV_HDMI_STEREO_3D_NONE
                HDMI_STEREO_NV_3D_VISION,         //    HDMI_STEREO_EXT_NONE}, // index 1, NV_STEREO_NVISION
                HDMI_STEREO_FRAME_PACKING,       //   HDMI_STEREO_EXT_NONE}, // index 2, NV_HDMI_STEREO_3D_FRAME_PACKING
                HDMI_STEREO_FRAME_PACKING_INT, //  HDMI_STEREO_EXT_NONE}, // index 3, NV_HDMI_STEREO_3D_FRAME_PACKING_INT
                HDMI_STEREO_LINE_ALTERNATIVE,     //   HDMI_STEREO_EXT_NONE}, // index 4, NV_HDMI_STEREO_3D_LINE_ALT
                HDMI_STEREO_SIDE_BY_SIDE_FULL,   //  HDMI_STEREO_EXT_NONE}, // index 5, NV_HDMI_STEREO_3D_SIDE_BY_SIDE_FULL
                HDMI_STEREO_SIDE_BY_SIDE_HALF|HDMI_STEREO_EXT_HORIZONTAIL_ODD_LEFT_ODD_RIGHT, // index 6, NV_HDMI_STEREO_3D_SIDE_BY_SIDE_HALF_HORIZONTAIL_ODD_LEFT_ODD_RIGHT
                HDMI_STEREO_SIDE_BY_SIDE_HALF|HDMI_STEREO_EXT_HORIZONTAIL_ODD_LEFT_EVEN_RIGHT, // index 7, NV_HDMI_STEREO_3D_SIDE_BY_SIDE_HALF_HORIZONTAIL_ODD_LEFT_EVEN_RIGHT
                HDMI_STEREO_SIDE_BY_SIDE_HALF|HDMI_STEREO_EXT_HORIZONTAIL_EVEN_LEFT_ODD_RIGHT, // index 8, NV_HDMI_STEREO_3D_SIDE_BY_SIDE_HALF_HORIZONTAIL_EVEN_LEFT_ODD_RIGHT
                HDMI_STEREO_SIDE_BY_SIDE_HALF|HDMI_STEREO_EXT_HORIZONTAIL_EVEN_LEFT_EVEN_RIGHT, // index 9, NV_HDMI_STEREO_3D_SIDE_BY_SIDE_HALF_HORIZONTAIL_EVEN_LEFT_EVEN_RIGHT
                HDMI_STEREO_SIDE_BY_SIDE_HALF|HDMI_STEREO_EXT_QUINCUNX_ODD_LEFT_ODD_RIGHT, // index 10, NV_HDMI_STEREO_3D_SIDE_BY_SIDE_HALF_QUINCUX_ODD_LEFT_ODD_RIGHT
                HDMI_STEREO_SIDE_BY_SIDE_HALF|HDMI_STEREO_EXT_QUINCUNX_ODD_LEFT_EVEN_RIGHT, // index 11, NV_HDMI_STEREO_3D_SIDE_BY_SIDE_HALF_QUINCUX_ODD_LEFT_EVEN_RIGHT
                HDMI_STEREO_SIDE_BY_SIDE_HALF|HDMI_STEREO_EXT_QUINCUNX_EVEN_LEFT_ODD_RIGHT, // index 12, NV_HDMI_STEREO_3D_SIDE_BY_SIDE_HALF_QUINCUX_EVEN_LEFT_ODD_RIGHT
                HDMI_STEREO_SIDE_BY_SIDE_HALF|HDMI_STEREO_EXT_QUINCUNX_EVEN_LEFT_EVEN_RIGHT, // index 13, NV_HDMI_STEREO_3D_SIDE_BY_SIDE_HALF_QUINCUX_EVEN_LEFT_EVEN_RIGHT
                HDMI_STEREO_FIELD_ALTERNATIVE, //     HDMI_STEREO_EXT_NONE}, // index 14, NV_HDMI_STEREO_3D_FIELD_ALT
                HDMI_STEREO_L_DEPTH,                  //     HDMI_STEREO_EXT_NONE}, // index 15, NV_HDMI_STEREO_3D_L_DEPTH
                HDMI_STEREO_L_DEPTH_GFX,          //     HDMI_STEREO_EXT_NONE}, // index 16, NV_HDMI_STEREO_3D_L_DEPTH_GFX
            };
            DbgMsg(_T("HDMI Stereo Mode Total modes=%d"), ModeList.numberOfModes);

            (*puCount) = ModeList.numberOfModes;
            (*ppCaps) = new DriverExtHDMIStereoModeCap[ModeList.numberOfModes];

            for(UINT i = 0; i < ModeList.numberOfModes; i++)
            {
                DbgMsg(_T("  HDMI Stereo Mode id=%d, HVisible=%d, VVisible=%d, HActive=%d, VActive=%d VActiveSpace=(%d,%d), refresh rate=%d, StereoType=%d"),
                    i,
                    ModeList.modeList[i].HVisible,
                    ModeList.modeList[i].VVisible,
                    ModeList.modeList[i].HActive,
                    ModeList.modeList[i].VActive,
                    ModeList.modeList[i].VActiveSpace[0],ModeList.modeList[i].VActiveSpace[1],
                    ModeList.modeList[i].rr,
                    ModeList.modeList[i].stereoType);

                (*ppCaps)[i].uHeight = ModeList.modeList[i].VVisible;
                (*ppCaps)[i].uWidth = ModeList.modeList[i].HVisible;
                (*ppCaps)[i].uRefreshRate = ModeList.modeList[i].rr;
                if (ModeList.modeList[i].stereoType == NV_HDMI_STEREO_3D_ANY) //all mode support
                {
                    (*ppCaps)[i].dwStereoMode = HDMI_STEREO_ALL;
                }
                else if ((ModeList.modeList[i].stereoType) > (sizeof(ModeMap) / sizeof(DWORD))) // out of range,set to disabled.
                {
                    DbgMsg(_T("NvAPI_DISP_EnumHDMIStereoModes , StereoType is out of map range value=%d"), ModeList.modeList[i].stereoType);
                    (*ppCaps)[i].dwStereoMode = HDMI_STEREO_NONE;
                }
                else
                {
                    (*ppCaps)[i].dwStereoMode = ModeMap[ModeList.modeList[i].stereoType];
                }
            }
            hr = S_OK;
        }
        else
        {
            (*puCount) = 0;
            (*ppCaps) = NULL;
            DbgMsg(_T("NvAPI_DISP_EnumHDMIStereoModes Fail, nvret=%d"), nvret);
        }
    }
    return hr;
}

HRESULT CNvAPIDeviceExtensionAdapter::EnableHDMIStereoMode(BOOL bEnable, DriverExtHDMIStereoModeCap *pCap, BOOL *pbReCreateDevice)
{
    CHECK_POINTER(pCap);
    CHECK_POINTER(pbReCreateDevice);

    if (!m_pDevice9)
    {
        return E_UNEXPECTED;
    }

    if (bEnable) // to turn off HDMI Stereo output, just re-create device with default display mode as like 1080p@60hz,etc.
    {
        // test vectors
        GUID testGUID  = {0x4189ef25, 0xec67, 0x4979, {0x8c, 0x75, 0x48, 0x48, 0x94, 0xe6, 0xfa, 0xca}};
        BYTE testKEY[] = {0x33, 0x9d, 0x3f, 0xff, 0xf1, 0x79, 0xdb, 0xc9, 0x17, 0xb4, 0xeb, 0xea, 0x2f, 0x3e, 0x84, 0x81};

        // step 1 - obtain challenge
        NVAPI_D3D9_ENABLE_STEREO_PARAMS params = {0};
        BYTE challenge[NVAPI_D3D9_ENABLE_STEREO_CHALLENGE_SIZE] = {0};
        params.version = NVAPI_D3D9_ENABLE_STEREO_PARAMS_VER;
        params.dwCommand = NVAPI_D3D9_ENABLE_STEREO_CMD_CHALLENGE;
        if (FAILED(NvAPI_D3D9_EnableStereo(m_pDevice9, &params)))
        {
            DbgMsg ("CNvAPIDeviceExtensionAdapter::EnableHDMIStereoMode - NvAPI Enable Stereo API: challenge failed\n");
            return E_FAIL;
        }
        else
        {
            memcpy(challenge, params.ChallengeParams.challenge, NVAPI_D3D9_ENABLE_STEREO_CHALLENGE_SIZE);
            DbgMsg ("CNvAPIDeviceExtensionAdapter::EnableHDMIStereoMode - NvAPI Enable Stereo API: challenge succeeded.\n");
            DbgMsg ("CNvAPIDeviceExtensionAdapter::EnableHDMIStereoMode - Challenge = ");
            for (int i = 0; i < NVAPI_D3D9_ENABLE_STEREO_CHALLENGE_SIZE; i++)
            {
                DbgMsg ("%02x ", challenge[i]);
            }
        }

        // step 2 - calculate response
        BYTE message[sizeof(GUID) + NVAPI_D3D9_ENABLE_STEREO_CHALLENGE_SIZE] = {0};
        BYTE response[NVAPI_D3D9_ENABLE_STEREO_RESPONSE_SIZE] = {0};
        memcpy(message, &testGUID, sizeof(GUID));
        memcpy(message + sizeof(GUID), challenge, NVAPI_D3D9_ENABLE_STEREO_CHALLENGE_SIZE);
        HMAC_SHA1(response, message, sizeof(message), testKEY, sizeof(testKEY));
        DbgMsg ("CNvAPIDeviceExtensionAdapter::EnableHDMIStereoMode - Response = ");
        for (int i = 0; i < NVAPI_D3D9_ENABLE_STEREO_RESPONSE_SIZE; i++)
        {
            DbgMsg ("%02x ", response[i]);
        }

        params.dwCommand = NVAPI_D3D9_ENABLE_STEREO_CMD_RESPONSE;
        memcpy(&params.ResponseParams.vendorGUID, &testGUID, sizeof(GUID));
        memcpy(params.ResponseParams.response, response, NVAPI_D3D9_ENABLE_STEREO_RESPONSE_SIZE);
        if (FAILED(NvAPI_D3D9_EnableStereo(m_pDevice9, &params)))
        {
            DbgMsg ("CNvAPIDeviceExtensionAdapter::EnableHDMIStereoMode - NvAPI Enable Stereo API: response failed\n");
            return E_FAIL;
        }
        else
        {
			DEVMODE devMode;
			DWORD dwFlag = CDS_RESET | CDS_FULLSCREEN;
			memset(&devMode, 0, sizeof(devMode));
			devMode.dmSize = sizeof(devMode);
			devMode.dmPelsWidth = pCap->uWidth;
			devMode.dmPelsHeight = pCap->uHeight;
			devMode.dmDisplayFrequency = pCap->uRefreshRate;
			devMode.dmBitsPerPel = 32;   // exclusive mode only supports 32 bits color depth.
			devMode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY | DM_BITSPERPEL;
			ChangeDisplaySettings(&devMode, dwFlag);

			DbgMsg ("CNvAPIDeviceExtensionAdapter::EnableHDMIStereoMode - NvAPI Enable Stereo API: response succeeded.\n");
        }
    } 
	/*
	else
	{
	   Bug#110879 & 110880 : Here we should handle disable cases, and change display setting to original status.
	   When D3D device been closed, it will also change display settings, so we don't have to do it again.
	}
	*/

	(*pbReCreateDevice) = TRUE;

    return S_OK;
}

HRESULT CNvAPIDeviceExtensionAdapter::QueryAdapterInfo(HWND hWnd, HMONITOR hMonitor, DispSvr::DrvExtAdapterInfo *pInfo)
{
	CHECK_POINTER(pInfo);
	HRESULT hr = E_FAIL;

	MONITORINFOEX MonInfo;
	ZeroMemory(&MonInfo, sizeof(MONITORINFOEX));
	MonInfo.cbSize = sizeof(MONITORINFOEX);
	NV_HDMI_SUPPORT_INFO HDMIInfo = {0};

	pInfo->bIsXVYCCSupported = FALSE;
	
	NvAPI_Status nvret = NVAPI_OK;
	if (GetMonitorInfo(hMonitor, &MonInfo))
	{
		NvDisplayHandle hNvDisp = NULL;
		nvret = NvAPI_GetAssociatedNvidiaDisplayHandle(T2A(MonInfo.szDevice), &hNvDisp);
		if (NVAPI_OK  == nvret)
		{
			HDMIInfo.version = NV_HDMI_SUPPORT_INFO_VER;
			// outputId = "0", the default outputId from NvAPI_GetAssociatedDisplayOutputId() will be used.
			nvret = NvAPI_GetHDMISupportInfo(hNvDisp, 0, &HDMIInfo);
			if (nvret == NVAPI_OK)
			{
				pInfo->bIsXVYCCSupported = ((HDMIInfo.isMonxvYCC601Capable || HDMIInfo.isMonxvYCC709Capable)) ? TRUE : FALSE;
				hr = S_OK;
			}
		}
	}

	NvU8 IsStereoEnabled;
	nvret = NvAPI_Stereo_IsEnabled(&IsStereoEnabled);
	if (nvret == NVAPI_OK)
	{
		if (IsStereoEnabled > 0)
			pInfo->dwSupportStereoFormats |= (STEREO_VISION_NV_PRIVATE | STEREO_VISION_NV_STEREO_API);
		else
			pInfo->dwSupportStereoFormats &= ~(STEREO_VISION_NV_PRIVATE | STEREO_VISION_NV_STEREO_API);
	}


	return hr;
}

HRESULT CNvAPIDeviceExtensionAdapter::Clear()
{
    m_SurfaceHandleLookupTable.clear();
    return S_OK;
}

#if 1
// The simple surface handle lookup table is not reliable especially when video size changes. Same IDirect3DSurface9 surface pointer address may
// be returned by the DX runtime but points to different surface (handle). As a result NvAPI_D3D9_VideoSetStereoInfo() can fail with NVAPI_INVALID_ARGUMENT.
HRESULT CNvAPIDeviceExtensionAdapter::LockupSurfaceHandle(IDirect3DSurface9 *pSurface, NVDX_ObjectHandle *pHandle)
{
	CHECK_POINTER(pSurface);
	CHECK_POINTER(pHandle);

	SurfaceHandleMap::const_iterator iter = m_SurfaceHandleLookupTable.find(pSurface);

	if (iter != m_SurfaceHandleLookupTable.end())
	{
		*pHandle = iter->second;
		return S_FALSE;
	}
	
	if (NVAPI_OK != NvAPI_D3D9_GetSurfaceHandle(pSurface, pHandle))
	{
		*pHandle = 0;
		return E_FAIL;
	}

	if (m_SurfaceHandleLookupTable.size() > 512)	// if the table grows too big, clean it up
		m_SurfaceHandleLookupTable.clear();

	m_SurfaceHandleLookupTable[pSurface] = *pHandle;
	return S_OK;
}
#else
// There may be potential performance problem if we always look up surface handles.
HRESULT CNvAPIDeviceExtensionAdapter::LockupSurfaceHandle(IDirect3DSurface9 *pSurface, NVDX_ObjectHandle *pHandle)
{
	CHECK_POINTER(pSurface);
	CHECK_POINTER(pHandle);

	return NvAPI_D3D9_GetSurfaceHandle(pSurface, pHandle) == NVAPI_OK ? S_OK : E_FAIL;
}
#endif

#define SHA1_BLOCK_LENGTH               64
#define SHA1_DIGEST_LENGTH              20

typedef struct {
    DWORD       state[5];
    ULONGLONG   count;
    BYTE        buffer[SHA1_BLOCK_LENGTH];
} SHA1_CTX;

#define rol(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))
#define blk0(i) (block->l[i] = (rol(block->l[i],24)&0xFF00FF00) \
    |(rol(block->l[i],8)&0x00FF00FF))
#define blk(i) (block->l[i&15] = rol(block->l[(i+13)&15]^block->l[(i+8)&15] \
    ^block->l[(i+2)&15]^block->l[i&15],1))

/* (R0+R1), R2, R3, R4 are the different operations used in SHA1 */
#define R0(v,w,x,y,z,i) z+=((w&(x^y))^y)+blk0(i)+0x5A827999+rol(v,5);w=rol(w,30);
#define R1(v,w,x,y,z,i) z+=((w&(x^y))^y)+blk(i)+0x5A827999+rol(v,5);w=rol(w,30);
#define R2(v,w,x,y,z,i) z+=(w^x^y)+blk(i)+0x6ED9EBA1+rol(v,5);w=rol(w,30);
#define R3(v,w,x,y,z,i) z+=(((w|x)&y)|(w&x))+blk(i)+0x8F1BBCDC+rol(v,5);w=rol(w,30);
#define R4(v,w,x,y,z,i) z+=(w^x^y)+blk(i)+0xCA62C1D6+rol(v,5);w=rol(w,30);

/* Hash a single 512-bit block. This is the core of the algorithm. */
static void SHA1Transform(DWORD state[5], const BYTE buffer[SHA1_BLOCK_LENGTH])
{
    DWORD a, b, c, d, e;
    typedef union {
        BYTE c[64];
        DWORD  l[16];
    } CHAR64LONG16;
    CHAR64LONG16* block;
    block = (CHAR64LONG16 *)buffer;

    /* Copy context->state[] to working vars */
    a = state[0];
    b = state[1];
    c = state[2];
    d = state[3];
    e = state[4];

    /* 4 rounds of 20 operations each. Loop unrolled. */
    R0(a,b,c,d,e, 0); R0(e,a,b,c,d, 1); R0(d,e,a,b,c, 2); R0(c,d,e,a,b, 3);
    R0(b,c,d,e,a, 4); R0(a,b,c,d,e, 5); R0(e,a,b,c,d, 6); R0(d,e,a,b,c, 7);
    R0(c,d,e,a,b, 8); R0(b,c,d,e,a, 9); R0(a,b,c,d,e,10); R0(e,a,b,c,d,11);
    R0(d,e,a,b,c,12); R0(c,d,e,a,b,13); R0(b,c,d,e,a,14); R0(a,b,c,d,e,15);
    R1(e,a,b,c,d,16); R1(d,e,a,b,c,17); R1(c,d,e,a,b,18); R1(b,c,d,e,a,19);
    R2(a,b,c,d,e,20); R2(e,a,b,c,d,21); R2(d,e,a,b,c,22); R2(c,d,e,a,b,23);
    R2(b,c,d,e,a,24); R2(a,b,c,d,e,25); R2(e,a,b,c,d,26); R2(d,e,a,b,c,27);
    R2(c,d,e,a,b,28); R2(b,c,d,e,a,29); R2(a,b,c,d,e,30); R2(e,a,b,c,d,31);
    R2(d,e,a,b,c,32); R2(c,d,e,a,b,33); R2(b,c,d,e,a,34); R2(a,b,c,d,e,35);
    R2(e,a,b,c,d,36); R2(d,e,a,b,c,37); R2(c,d,e,a,b,38); R2(b,c,d,e,a,39);
    R3(a,b,c,d,e,40); R3(e,a,b,c,d,41); R3(d,e,a,b,c,42); R3(c,d,e,a,b,43);
    R3(b,c,d,e,a,44); R3(a,b,c,d,e,45); R3(e,a,b,c,d,46); R3(d,e,a,b,c,47);
    R3(c,d,e,a,b,48); R3(b,c,d,e,a,49); R3(a,b,c,d,e,50); R3(e,a,b,c,d,51);
    R3(d,e,a,b,c,52); R3(c,d,e,a,b,53); R3(b,c,d,e,a,54); R3(a,b,c,d,e,55);
    R3(e,a,b,c,d,56); R3(d,e,a,b,c,57); R3(c,d,e,a,b,58); R3(b,c,d,e,a,59);
    R4(a,b,c,d,e,60); R4(e,a,b,c,d,61); R4(d,e,a,b,c,62); R4(c,d,e,a,b,63);
    R4(b,c,d,e,a,64); R4(a,b,c,d,e,65); R4(e,a,b,c,d,66); R4(d,e,a,b,c,67);
    R4(c,d,e,a,b,68); R4(b,c,d,e,a,69); R4(a,b,c,d,e,70); R4(e,a,b,c,d,71);
    R4(d,e,a,b,c,72); R4(c,d,e,a,b,73); R4(b,c,d,e,a,74); R4(a,b,c,d,e,75);
    R4(e,a,b,c,d,76); R4(d,e,a,b,c,77); R4(c,d,e,a,b,78); R4(b,c,d,e,a,79);

    /* Add the working vars back into context.state[] */
    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
    state[4] += e;
    /* Wipe variables */
    a = b = c = d = e = 0;
}


/* SHA1Init - Initialize new context */
static void SHA1Init(SHA1_CTX *context)
{
    /* SHA1 initialization constants */
    context->count = 0;
    context->state[0] = 0x67452301;
    context->state[1] = 0xEFCDAB89;
    context->state[2] = 0x98BADCFE;
    context->state[3] = 0x10325476;
    context->state[4] = 0xC3D2E1F0;
}


/* Run your data through this. */
static void SHA1Update(SHA1_CTX *context, const BYTE *data, DWORD len)
{
    DWORD i, j;

    j = (DWORD)((context->count >> 3) & 63);
    context->count += (len << 3);
    if ((j + len) > 63) {
        memcpy(&context->buffer[j], data, (i = 64 - j));
        SHA1Transform(context->state, context->buffer);
        for ( ; i + 63 < len; i += 64) {
            memcpy(context->buffer, &data[i], 64);
            SHA1Transform(context->state, context->buffer);
        }
        j = 0;
    }
    else i = 0;
    memcpy(&context->buffer[j], &data[i], len - i);
}


/* Add padding and return the message digest. */
static void SHA1Final(BYTE digest[SHA1_DIGEST_LENGTH], SHA1_CTX *context)
{
    DWORD i;
    BYTE finalcount[8];

    for (i = 0; i < 8; i++) {
        finalcount[i] = (BYTE)((context->count >>
            ((7 - (i & 7)) * 8)) & 255);  /* Endian independent */
    }
    SHA1Update(context, (BYTE *)"\200", 1);
    while ((context->count & 504) != 448) {
        SHA1Update(context, (BYTE *)"\0", 1);
    }
    SHA1Update(context, finalcount, 8);  /* Should cause a SHA1Transform() */

    if (digest)
    {
        for (i = 0; i < SHA1_DIGEST_LENGTH; i++) {
            digest[i] = (BYTE)((context->state[i >> 2] >>
                ((3 - (i & 3)) * 8)) & 255);
        }
    }
}

void CNvAPIDeviceExtensionAdapter::SHA1(BYTE* pHash, const BYTE* pData, const DWORD size)
{
    SHA1_CTX context;
    SHA1Init(&context);
    SHA1Update(&context, pData, size);
    SHA1Final(pHash, &context);
}

// **************************************************************************
// Crypto helper functions - HMAC-SHA-1
// Based on FIPS Publication 198: The Keyed-Hash Message Authentication Code
// This C implementation is completely written from scratch.
// **************************************************************************
void CNvAPIDeviceExtensionAdapter::HMAC_SHA1(BYTE* pMac, const BYTE* pData, const DWORD dataSize, const BYTE* pKey, const DWORD keySize)
{
    BYTE K0[SHA1_BLOCK_LENGTH], ipad[SHA1_BLOCK_LENGTH], opad[SHA1_BLOCK_LENGTH];
    BYTE tmp_hash[SHA1_DIGEST_LENGTH], *tmp_msg1, tmp_msg2[SHA1_BLOCK_LENGTH + SHA1_DIGEST_LENGTH];

    // FIPS Publication 198 Section 5: HMAC Specification
    // Step 1-3: Determine K0
    memset(K0, 0, SHA1_BLOCK_LENGTH);
    if (keySize <= SHA1_BLOCK_LENGTH)
    {
        memcpy(K0, pKey, keySize);
    }
    else
    {
        SHA1(K0, pKey, keySize);
    }

    // Step 4: K0 ^ ipad
    // Step 7: K0 ^ opad
    for (int i = 0; i < SHA1_BLOCK_LENGTH; i++)
    {
        ipad[i] = K0[i] ^ 0x36;
        opad[i] = K0[i] ^ 0x5c;
    }

    // Step 5: Append the stream data to the result of Step 4
    tmp_msg1 = new BYTE[SHA1_BLOCK_LENGTH + dataSize];
    memcpy(tmp_msg1, ipad, SHA1_BLOCK_LENGTH);
    memcpy(tmp_msg1 + SHA1_BLOCK_LENGTH, pData, dataSize);

    // Step 6: Apply SHA-1 hash to the stream generated in Step 5
    SHA1(tmp_hash, tmp_msg1, SHA1_BLOCK_LENGTH + dataSize);
    delete[] tmp_msg1;

    // Step 8: Append the result 
    memcpy(tmp_msg2, opad, SHA1_BLOCK_LENGTH);
    memcpy(tmp_msg2 + SHA1_BLOCK_LENGTH, tmp_hash, SHA1_DIGEST_LENGTH);

    // Step 9: Apply SHA-1 hash to the result from Step 8
    SHA1(pMac, tmp_msg2, SHA1_BLOCK_LENGTH + SHA1_DIGEST_LENGTH);
}

///////////////////////////////////////////////////////////////////////////////
/// NV Coproc related

// Before calling NvAPI_Coproc_GetApplicationCoprocInfo, application should check whether there is
// coproc adapter.
bool IsNVCoprocDevicePresent(DWORD &dwDeviceId)
{
	DWORD iDevNum = 0;
	DISPLAY_DEVICE DisplayDevice;

	ZeroMemory(&DisplayDevice, sizeof(DisplayDevice));
	DisplayDevice.cb = sizeof(DisplayDevice);

	while (EnumDisplayDevices(NULL, iDevNum, &DisplayDevice, 0))
	{
		if (!(DisplayDevice.StateFlags & (DISPLAY_DEVICE_PRIMARY_DEVICE | DISPLAY_DEVICE_MIRRORING_DRIVER)))
		{
			std::string devstr = DisplayDevice.DeviceString;
			std::string devid = DisplayDevice.DeviceID;

			// sample string of DisplayDevice.DeviceID looks like "PCI\VEN_10DE&DEV_0A60&SUBSYS_075910DE&REV_A2"
			if (std::string::npos != devstr.find("NVIDIA") && std::string::npos != devid.find("VEN_10DE"))
			{
				std::istringstream iss(devid.substr(17, 4));
				iss >> std::hex >> dwDeviceId;
				return true;
			}
		}
		iDevNum++;
	}
	return false;
}

// NvAPI_Coproc_GetApplicationCoprocInfo.appStatus == NV_COPROC_APP_STATUS_ENABLED indicates the application
// is listed in the driver's white list. By making the call, application will comply coproc requiremnents by
// avoid calling any intel private API such as PAVP, etc.
//
// Calling NvAPI_Coproc_GetApplicationCoprocInfo also serves as an hint to the driver.
// It must be called *BEFORE* CreateDevice so that the driver can initialize the new device correctly.
bool IsNVCoprocEnabledForApp()
{
	DWORD dwDeviceId = 0;
    NvAPI_Status status = NvAPI_Initialize();

	if (NVAPI_OK == status)
	{
		NV_COPROC_APP_INFO CoprocAppInfo = { 0 }; 
		CoprocAppInfo.version = NV_COPROC_APP_INFO_VER;

		status = NvAPI_Coproc_GetApplicationCoprocInfo(&CoprocAppInfo);

		if (NVAPI_OK == status && CoprocAppInfo.appStatus == NV_COPROC_APP_STATUS_ENABLED)
			return true;
	}

    return false;
}
