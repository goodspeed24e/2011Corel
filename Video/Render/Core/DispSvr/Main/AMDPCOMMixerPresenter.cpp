// AmdPCOMRender.cpp : Implementation of CAMDPCOMMixerPresenter
#include "stdafx.h"

#include "DispSvr.h"
#include "DynLibManager.h"
#include "ResourceManager.h"
#include "RegistryService.h"
#include "MathVideoMixing.h"
#include "AmdPCOMMixerPresenter.h"
#include "D3D9VideoEffect3DManager.h"

using namespace DispSvr;
#define PRESENT_INTERVAL	1

static inline bool ClientToScreen(PCOM_RECT &PCOMRect, const HWND hwnd, const RECT &dstClip)
{
	int ret;
	POINT pt = {0};
	RECT Rect, WindowRect = {PCOMRect.left, PCOMRect.top, PCOMRect.right, PCOMRect.bottom};

	ret = ::ClientToScreen(hwnd, &pt); ASSERT(ret != 0);
	ret = ::OffsetRect(&WindowRect, pt.x, pt.y); ASSERT(ret != 0);
	IntersectRect(&Rect, &WindowRect, &dstClip);

	if (!IsRectEmpty(&Rect))
	{
		PCOMRect.left = Rect.left;
		PCOMRect.top = Rect.top;
		PCOMRect.right = Rect.right;
		PCOMRect.bottom = Rect.bottom;
		return true;
	}
	return false;	// invisible
}

static inline void NRectToPCOMRect(PCOM_RECT &PCOMRectClear, const NORMALIZEDRECT &NRectClear)
{
	RECT rcClear = {PCOMRectClear.left, PCOMRectClear.top, PCOMRectClear.right, PCOMRectClear.bottom};
	NRectToRect(rcClear, NRectClear);
	PCOMRectClear.left = rcClear.left;
	PCOMRectClear.top = rcClear.top;
	PCOMRectClear.right = rcClear.right;
	PCOMRectClear.bottom = rcClear.bottom;
}

static inline void SelectDisplayField(PCOM_FIELD_SELECT &Field, const VideoProperty &Property)
{
	if (Property.dwFieldSelect == FIELD_SELECT_FIRST)
	{
		Field = (Property.Format == VIDEO_FORMAT_FIELD_INTERLEAVED_EVEN_FIRST) ? PCOM_InterlaceFieldEven : PCOM_InterlaceFieldOdd;
	}
	else if (Property.dwFieldSelect == FIELD_SELECT_SECOND)
	{
		Field = (Property.Format == VIDEO_FORMAT_FIELD_INTERLEAVED_EVEN_FIRST) ? PCOM_InterlaceFieldOdd : PCOM_InterlaceFieldEven;
	}

	if (Property.Format == VIDEO_FORMAT_PROGRESSIVE)
		Field = PCOM_ProgressiveFrame;
}

static inline void CalculatePCOMRect(PCOM_RECT &pcomrcSrc, PCOM_RECT &pcomrcDst, const D3D9Plane &plane, RECT &rcSrc, const RECT &rcMixingDst, const RECT &rcDstClip, float fWindowAspectRatio)
{
	NORMALIZEDRECT nrcOutput = plane.nrcDst;
	RECT rcDst = rcMixingDst;

	CorrectAspectRatio(nrcOutput, plane.fAspectRatio / fWindowAspectRatio);
	CropRect(nrcOutput, plane.nrcCrop);
	CropRect(rcSrc, plane.nrcCrop);
	NRectToRect(rcDst, nrcOutput);
	ClipRect(rcDst, rcSrc, rcDstClip);

	pcomrcSrc.left = rcSrc.left;
	pcomrcSrc.top = rcSrc.top;
	pcomrcSrc.right = rcSrc.right;
	pcomrcSrc.bottom = rcSrc.bottom;

	pcomrcDst.left = rcDst.left;
	pcomrcDst.top  = rcDst.top;
	pcomrcDst.right = rcDst.right;
	pcomrcDst.bottom = rcDst.bottom;
}

CAMDPCOMMixerPresenter::CAMDPCOMMixerPresenter()
{
	m_GUID = DISPSVR_RESOURCE_AMDPCOMVIDEOMIXERPRESENTER;
	m_dwFlipChainSize = 0;
	m_dwQueuedFrameCount = 0;
	m_bFullScreen = false;
	m_pSession = NULL;
	ZeroMemory(&m_XvYCCGamutMetaData, sizeof(XVYCC_GAMUT_METADATA));
	m_dwPCOMClearRecCount = 0;
	ZeroMemory(&m_PCOMClearRecList, sizeof(m_PCOMClearRecList));

	m_bSpatialDeinterlacing = true;
	m_bAsyncPresent = true;
	m_pMcomSession = NULL;
	ZeroMemory(&m_DecodeStreamCaps, sizeof(MCOM_DECODE_STREAM_CAPS));
	m_DecodeStreamCaps.CAP_2HD_OTHER = m_DecodeStreamCaps.CAP_2HD_PROGRESSIVE = m_DecodeStreamCaps.CAP_1HD_1SD = 0x80000000;
	// MIXER_CAP_CAN_CHANGE_DESTINATION, MIXER_CAP_3D_RENDERTARGET, MIXER_CAP_VIRTUALIZE_FROM_ORIGIN are not supported.
	m_MixerCaps.dwFlags = 0;
}

CAMDPCOMMixerPresenter::~CAMDPCOMMixerPresenter()
{
}

STDMETHODIMP CAMDPCOMMixerPresenter::QueryInterface(REFIID riid, void **ppv)
{
	HRESULT hr = E_NOINTERFACE;

	if (riid == __uuidof(IDispSvrVideoPresenter))
	{
		hr = GetInterface((IDispSvrVideoPresenter *)this, ppv);
	}
	else
	{
		hr = CD3D9VideoMixerBase::QueryInterface(riid, ppv);
	}
	return hr;
}

STDMETHODIMP CAMDPCOMMixerPresenter::ProcessMessage(RESOURCE_MESSAGE_TYPE eMessage, LPVOID ulParam)
{
	return CD3D9VideoMixerBase::ProcessMessage(eMessage, ulParam);
}

STDMETHODIMP CAMDPCOMMixerPresenter::_QueryPlaneCaps(PLANE_ID PlaneID, PLANE_FORMAT Format, PlaneCaps *pCap)
{
	// PCOM_NV12, PCOM_YUY2, PCOM_ARGB, PCOM_AYUV
	if(Format != PLANE_FORMAT_ARGB && Format != PLANE_FORMAT_YUY2 && Format != PLANE_FORMAT_NV12 && Format != PLANE_FORMAT_AYUV)
		return E_FAIL;

	return S_OK;
}

STDMETHODIMP CAMDPCOMMixerPresenter::SetDisplayRect(const RECT *rcDst, const RECT *rcSrc)
{
	CAutoLock lock(&m_csLock);

	HRESULT hr = E_FAIL;
	hr = CD3D9VideoPresenterBase::SetDisplayRect(rcDst, rcSrc);
	if (FAILED(hr))
		return hr;

	// PCOM will calculate the window position internally.
	// AP only needs to pass destination window size to PCOM.
	POINT pt = {0};
	::ClientToScreen(m_hwnd, &pt);
	::OffsetRect(&m_rcDst, m_rcMonitor.left - pt.x, m_rcMonitor.top - pt.y);

	m_bFullScreen = CD3D9VideoPresenterBase::IsWindowFullScreen();
	return hr;
}

STDMETHODIMP CAMDPCOMMixerPresenter::Present(const PresentHints *pHints)
{
	CAutoLock lock(&m_csLock);
	// If PCOM queue is not full yet, keep waiting until queue is full.
	if (m_dwQueuedFrameCount < m_dwFlipChainSize)
	{
		DbgMsg("Fill up the display queue!!");
		return S_FALSE;
	}

	return PresentPCOM();
}

STDMETHODIMP CAMDPCOMMixerPresenter::Clear()
{
	CAutoLock lock(&m_csLock);
	return ClearPCOM();
}

STDMETHODIMP CAMDPCOMMixerPresenter::SetColorKey(const DWORD dwColorKey)
{
	CAutoLock lock(&m_csLock);

	DWORD dwOldColorKey = m_dwColorKey;
	HRESULT hr = CD3D9VideoPresenterBase::SetColorKey(dwColorKey);
	if (SUCCEEDED(hr) && m_dwColorKey != dwOldColorKey)
	{
		if (m_pMcomSession)
		{
			hr = DestroyMCOM();
			hr = CreateMCOM();
		}
		if (m_pSession)
		{
			hr = DestroyPCOM();
			hr = CreatePCOM();
		}
	}

	return hr;
}

STDMETHODIMP CAMDPCOMMixerPresenter::SetGamutMetadata(const DWORD dwFormat, void *pGamutMetadata)
{
	if (m_PresenterCaps.bSupportXvYCC)
	{
		m_dwGamutFormat = dwFormat;
		if (m_dwGamutFormat == GAMUT_METADATA_NONE)
		{
			ZeroMemory(&m_GamutRange, sizeof(GamutMetadataRange));
			ZeroMemory(&m_GamutVertices, sizeof(GamutMetadataVertices));
			DbgMsg("DispSvr: XvYcc OFF.");
			return S_OK;
		}

		if (m_dwGamutFormat == GAMUT_METADATA_RANGE && memcmp(&m_GamutRange, pGamutMetadata, sizeof(m_GamutRange)) )
		{
			memcpy(&m_GamutRange, pGamutMetadata, sizeof(GamutMetadataRange));
			m_XvYCCGamutMetaData.size = sizeof(m_XvYCCGamutMetaData);
			m_XvYCCGamutMetaData.Format_Flag = m_GamutRange.Format_Flag;
			m_XvYCCGamutMetaData.GBD_Color_Precision = m_GamutRange.GBD_Color_Precision;
			m_XvYCCGamutMetaData.GBD_Color_Space = m_GamutRange.GBD_Color_Space;
			m_XvYCCGamutMetaData.Max_Red_Data = m_GamutRange.Max_Red_Data;
			m_XvYCCGamutMetaData.Min_Red_Data = m_GamutRange.Min_Red_Data;
			m_XvYCCGamutMetaData.Max_Green_Data = m_GamutRange.Max_Green_Data;
			m_XvYCCGamutMetaData.Min_Green_Data = m_GamutRange.Min_Green_Data;
			m_XvYCCGamutMetaData.Max_Blue_Data = m_GamutRange.Max_Blue_Data;
			m_XvYCCGamutMetaData.Min_Blue_Data = m_GamutRange.Min_Blue_Data;
			DbgMsg("DispSvr: XvYcc ON.");
			return S_OK;
		}
	}

	return E_NOTIMPL;
}

STDMETHODIMP CAMDPCOMMixerPresenter::_SetDevice(IUnknown *pDevice)
{
	CAutoLock lock(&m_csLock);
	HRESULT hr = E_FAIL;

	hr = CD3D9VideoMixerBase::_SetDevice(pDevice);
	if (FAILED(hr))
		return hr;

	hr = CD3D9VideoPresenterBase::_SetDevice(pDevice);
	if (SUCCEEDED(hr))
	{
		DestroyMCOM();
		hr = CreateMCOM();

		DestroyPCOM();

		// Create PCOM related resource with D3D device
		hr = CreatePCOM();
		if (SUCCEEDED(hr))
		{
			hr = m_pTexturePool->SetPoolUsage(TEXTURE_POOL_USAGE_OFFSCREENPLANESURFACE);
			GenerateDxva2VPList();

			VideoProcessorStub vp = {0};
			vp.guidVP = DispSvr_VideoProcPCOM;
			vp.RenderTargetFormat = D3DFMT_X8R8G8B8;
			vp.sCaps.eType = PROCESSOR_TYPE_HARDWARE;
			vp.sCaps.uProcessorCaps = PROCESSOR_CAPS_DEINTERLACE_BOB;

			for (VideoProcessorList::iterator vi = m_VideoProcessorList.begin(); vi != m_VideoProcessorList.end(); ++vi)
				if (vi->pfnVBltFactory)
				{
					vp.pDelegateVPStub = &*vi;
					break;
				}

			m_VideoProcessorList.push_back(vp);
			SelectVideoProcessor();
		}
	}

	return hr;
}

STDMETHODIMP CAMDPCOMMixerPresenter::_ReleaseDevice()
{
	CAutoLock lock(&m_csLock);
	HRESULT hr;

	DestroyPCOM();
	DestroyMCOM();

	hr = CD3D9VideoMixerBase::_ReleaseDevice();
	hr |= CD3D9VideoPresenterBase::_ReleaseDevice();
	return hr;
}

STDMETHODIMP CAMDPCOMMixerPresenter::_SetWindow(HWND hwnd)
{
	CAutoLock lock(&m_csLock);

	HRESULT hr = CD3D9PluginBase::_SetWindow(hwnd);

	if (hr == S_OK)
	{
		if(m_pMcomSession)
		{
			DestroyMCOM();
			hr = CreateMCOM();
		}
		// If PCOM is initialized, we need to re-initialize PCOM with new window handle.
		if (m_pSession)
		{
			DestroyPCOM();
			hr = CreatePCOM();
		}
	}
	return hr;
}

HRESULT CAMDPCOMMixerPresenter::_Execute(IDirect3DSurface9 *pDestSurface, const RECT &, const RECT &rcDstClip)
{
	CAutoLock lock(&m_csLock);
	HRESULT hr = E_FAIL;
	RECT rcDst(m_rcDst); // The rect of destination.

	NRectToRect(rcDst, m_nrcDst);
	if (rcDst.top > rcDst.bottom || rcDst.left > rcDst.right)
		return E_FAIL;

	PCOM_EXECUTE_INPUT ExecuteIn = {0};
	ExecuteIn.size = sizeof(PCOM_EXECUTE_INPUT);
	ExecuteIn.planeCount = 0;

    //Each plane should refer to the their own mixing area's aspect ratio, not whole mixing area(Window)
    // mixing area's aspect ratio = (window(whole mixing area) width * viewport's normalized width) /
    //                                             (window(whole mixing area) height * viewport's normalized height)
    float fWindowWidth = (float)(m_rcMixingDst.right - m_rcMixingDst.left);
    float fWindowHeight = (float)(m_rcMixingDst.bottom - m_rcMixingDst.top);
    float fMixingAreaAspectRatio[PLANE_MAX];
    for (int i = 0; i < DispSvr::PLANE_MAX; i++)
    {
        fMixingAreaAspectRatio[i] = m_fWindowAspectRatio;

        if (m_Planes[i].bValid)
        {
            fMixingAreaAspectRatio[i] = (fWindowWidth * (m_Planes[i].nrcDst.right - m_Planes[i].nrcDst.left)) /
                (fWindowHeight * (m_Planes[i].nrcDst.bottom - m_Planes[i].nrcDst.top));
        }
    }

	// PCOM clear rectangle will apply current and below layers except main video & background.
	// So we can only apply clear rectangle to SubVidoe & SPIC layer.
	// We need to check HDi layer in order to pass clear rectangle & coordinates to SubVideo & SPIC layer,
	if (m_ClearRectList.size() > 0 && m_ClearRectList.size() <= PCOM_MAX_CLEAR_RECTANGLES_COUNT)
	{
		ASSERT(m_Planes[PLANE_INTERACTIVE].bValid == true);
		m_dwPCOMClearRecCount = 0;
		PCOM_RECT srcRect, dstRect;
		RECT rcSrc = m_Planes[PLANE_INTERACTIVE].rcSrc;
		CalculatePCOMRect(srcRect, dstRect, m_Planes[PLANE_INTERACTIVE], rcSrc, rcDst, rcDstClip, fMixingAreaAspectRatio[PLANE_INTERACTIVE]);

		PCOM_CLEAR_RECTANGLE *pCR = m_PCOMClearRecList;
		for (ClearRectList::iterator it = m_ClearRectList.begin(); it != m_ClearRectList.end(); ++it)
		{
			pCR->type = (it->Target == CLEAR_RECT_TARGET_MAIN) ? PCOM_MainVideo_ClearRect : PCOM_SubVideo_ClearRect;
			pCR->clearRect = dstRect;
			NRectToPCOMRect(pCR->clearRect, it->Rect);
			// The coordinates of PCOM clear rectangle Rect is opposite to desktop, not client window.
			if (ClientToScreen(pCR->clearRect, m_hwnd, rcDstClip))
			{
				++m_dwPCOMClearRecCount;
				++pCR;
			}
		}
	}

	for (int i = 0; i < DispSvr::PLANE_MAX; i++)
	{
		hr = PlaneToPCOMPlane(i, ExecuteIn.planeList[ExecuteIn.planeCount], rcDst, rcDstClip, fMixingAreaAspectRatio[i]);
		if (hr == S_OK)
			ExecuteIn.planeCount++;
	}

	if (ExecuteIn.planeCount)
	{
		// rect of destination(rcDst) should be equal to screen size in full screen mode,
		// or unexpected color would shown outside the PCOM planes.
		if(m_bFullScreen)
			SetRect(&rcDst, 0, 0, m_szSrc.cx, m_szSrc.cy);

		hr = BeginFramePCOM(rcDst);
		if (SUCCEEDED(hr))
		{
			hr = ExecutePCOM(&ExecuteIn); ASSERT(SUCCEEDED(hr));
			hr = EndFramePCOM(); ASSERT(SUCCEEDED(hr));
		}
	}

	return hr;
}

HRESULT CAMDPCOMMixerPresenter::CreatePCOM()
{
	PCOM_STATUS ret = PCOM_OK;
	PCOM_GET_CAPS_INPUT GetCapsIn = {0};
	PCOM_GET_CAPS_OUTPUT GetCapsOut = {0};

	GetCapsOut.size = sizeof(PCOM_GET_CAPS_OUTPUT);
	GetCapsIn.size = sizeof(PCOM_GET_CAPS_INPUT);
	GetCapsIn.GfxDevice = m_pDevice;

	ret = PCOMGetCaps(&GetCapsIn, &GetCapsOut);
	if (ret != PCOM_OK)
		return E_FAIL;

	DWORD dwBBWidth = GetRegistry(REG_BACKBUFFER_WIDTH, m_rcMonitor.right - m_rcMonitor.left);
	DWORD dwBBHeight = GetRegistry(REG_BACKBUFFER_HEIGHT, m_rcMonitor.bottom - m_rcMonitor.top);

	PCOM_CREATE_INPUT CreateIn = {0};
	PCOM_CREATE_OUTPUT CreateOut = {0};

	CreateOut.size = sizeof(PCOM_CREATE_OUTPUT);
	CreateIn.size = sizeof(PCOM_CREATE_INPUT);

	CreateIn.flags.value = 0;
	CreateIn.flags.useOverlayPresent = 1;
	CreateIn.flags.useAsyncPresent = (m_bAsyncPresent) ? 1 : 0;

	CreateIn.overlayColorKey = m_dwColorKey;
	CreateIn.mainVideoWidth = dwBBWidth;
	CreateIn.mainVideoHeight = dwBBHeight;
	CreateIn.windowHandle = m_hwnd;
	CreateIn.GfxDevice = m_pDevice;

	ret = PCOMCreate(&CreateIn, &CreateOut);
	if (ret != PCOM_OK)
	{
		ASSERT(0);
		DbgMsg("PCOMCreate return value: 0x%08X", ret);
		return E_FAIL;
	}

	m_pSession = CreateOut.PCOMSession;
	m_dwFlipChainSize = static_cast<DWORD>(CreateOut.flipChainSize);

	// Update presenter caps
	m_PresenterCaps.dwFPS = GetCapsOut.recomFps;
	m_PresenterCaps.dwResPixels = GetCapsOut.recomWindowPixelCnt;
	m_PresenterCaps.dwBandWidth = GetCapsOut.systemToVideoBandwidth;
	m_PresenterCaps.bSupportXvYCC = (GetCapsOut.flags.supportXvYCCWideGamutDisplay) ? TRUE : FALSE;
	m_PresenterCaps.bIsOverlay = TRUE;
	m_bSpatialDeinterlacing = (GetCapsOut.flags.supportSpatialDeinterlacing) ? true : false ;

	return S_OK;
}

HRESULT CAMDPCOMMixerPresenter::DestroyPCOM()
{
	if (m_pSession)
	{
		PCOM_STATUS ret = PCOM_OK;
		ret = PCOMDestroy(m_pSession);
		ASSERT(ret == PCOM_OK);
		m_pSession = NULL;
	}

	m_dwFlipChainSize = 0;
	m_dwQueuedFrameCount = 0;
	return S_OK;
}

HRESULT CAMDPCOMMixerPresenter::BeginFramePCOM(const RECT &rcDst)
{
	PCOM_STATUS ret = PCOM_OK;
	PCOM_BEGIN_FRAME_INPUT BeginFrameIn = {0};
	BeginFrameIn.size = sizeof(PCOM_BEGIN_FRAME_INPUT);
	BeginFrameIn.flags.enableFullScreenMode = (m_bFullScreen) ? 1 : 0;
	BeginFrameIn.flags.enableXvYCCMetaData = (m_dwGamutFormat == GAMUT_METADATA_RANGE) ? 1 : 0;
	BeginFrameIn.targetRect.left = rcDst.left;
	BeginFrameIn.targetRect.top = rcDst.top;
	BeginFrameIn.targetRect.right = rcDst.right;
	BeginFrameIn.targetRect.bottom = rcDst.bottom;
	BeginFrameIn.xvYCCMetaData = (m_dwGamutFormat == GAMUT_METADATA_RANGE) ? &m_XvYCCGamutMetaData : 0;

	while(1)
	{
		ret = PCOMBeginFrame(m_pSession, &BeginFrameIn);
		if (ret == PCOM_QUEUE_FULL)
		{
			HRESULT hr = PresentPCOM();
			if (SUCCEEDED(hr))
				continue;
		}
		break;
	};

	return (ret == PCOM_OK) ? S_OK : E_FAIL;
}

HRESULT CAMDPCOMMixerPresenter::ExecutePCOM(PPCOM_EXECUTE_INPUT pExecuteIn)
{
	HRESULT hr = S_OK;
	if (pExecuteIn->planeCount > 0)
	{
		PCOM_STATUS ret = PCOMExecute(m_pSession, pExecuteIn);
		// QueuedFrameCound should increase by 1 if PCOMExecute successfully.
		if (ret == PCOM_OK)
		{
			m_dwQueuedFrameCount++;
		}
		else
		{
			// If assert happens with DXVA2 enabled, it could causes AP hang.
			ASSERT(0);
			hr = E_FAIL;
		}
	}

	return hr;
}

HRESULT CAMDPCOMMixerPresenter::EndFramePCOM()
{
	PCOM_STATUS ret = PCOM_OK;
	PCOM_END_FRAME_INPUT EndFrameIn = {0};
	EndFrameIn.size = sizeof(PCOM_END_FRAME_INPUT);
	EndFrameIn.flags.value = 0;

	ret = PCOMEndFrame(m_pSession, &EndFrameIn);
	return (ret == PCOM_OK) ? S_OK : E_FAIL;
}

HRESULT CAMDPCOMMixerPresenter::PresentPCOM()
{
	HRESULT hr = E_FAIL;
	PCOM_STATUS ret = PCOM_OK;
	PCOM_PRESENT_OUTPUT PresentOut = {0};
	PresentOut.size = sizeof(PCOM_PRESENT_OUTPUT);

	while(1)
	{
		ret = PCOMPresent(m_pSession, &PresentOut);
		if (ret != PCOM_OK)
		{
			DbgMsg("PCOMPresent is failed due to error: 0x%08X", ret);
			if (m_bAsyncPresent && (ret==PCOM_FLIP_PENDING || ret==PCOM_COMP_NOT_READY))
			{
				Sleep(PRESENT_INTERVAL);
				continue;
			}
		}

		break;
	}

	// m_dwQueuedFrameCount should be decreased by 1 after PCOMPresent.
	if (ret == PCOM_OK)
	{
		m_dwQueuedFrameCount = PresentOut.numOfQueuedUpFrames;
		hr = S_OK;
	}

	return hr;
}


HRESULT CAMDPCOMMixerPresenter::ClearPCOM()
{
	HRESULT hr = E_FAIL;
	PCOM_STATUS ret = PCOM_OK;

	ret = PCOMResetQueue(m_pSession);
	if (ret == PCOM_OK)
	{
		m_dwQueuedFrameCount = 0;
	}

	RECT rcDst = {0};
	CopyRect(&rcDst, &m_rcDst);
	NRectToRect(rcDst, m_nrcDst);

	PCOM_EXECUTE_INPUT ExecuteIn = {0};
	ExecuteIn.size = sizeof(PCOM_EXECUTE_INPUT);
	ExecuteIn.planeCount = 1;
	ExecuteIn.planeList[0].size = sizeof(PCOM_PLANE);
	ExecuteIn.planeList[0].validFields.value = 0;
	ExecuteIn.planeList[0].validFields.hasBackgroundColor = 1;
	ExecuteIn.planeList[0].planeType = PCOM_Generic_Plane;
	ExecuteIn.planeList[0].flags.value = 0;
	ExecuteIn.planeList[0].plane = NULL;
	ExecuteIn.planeList[0].backgroundColor = m_colorBackGround;
	ExecuteIn.planeList[0].dstRect.left = rcDst.left;
	ExecuteIn.planeList[0].dstRect.top = rcDst.top;
	ExecuteIn.planeList[0].dstRect.right = rcDst.right;
	ExecuteIn.planeList[0].dstRect.bottom = rcDst.bottom;

	//Clear desktop resolution
	SetRect(&rcDst, 0, 0, m_szSrc.cx, m_szSrc.cy);
	for (DWORD i=0; i<m_dwFlipChainSize ;i++)
	{
		hr = BeginFramePCOM(rcDst);
		if (SUCCEEDED(hr))
		{
			hr = ExecutePCOM(&ExecuteIn); ASSERT(SUCCEEDED(hr));
			hr = EndFramePCOM(); ASSERT(SUCCEEDED(hr));
		}
	}

	while(m_dwQueuedFrameCount)
	{
		hr = PresentPCOM();
		if (FAILED(hr))
			break;
	}

	return hr;
}

HRESULT CAMDPCOMMixerPresenter::PlaneToPCOMPlane(PLANE_ID id, PCOM_PLANE &PCOMPlane, const RECT &rcDst, const RECT &rcDstClip, const float fWindowAspectRatio)
{
	D3D9Plane &plane = m_Planes[id];
	CComPtr<IDirect3DSurface9> pSurface;

	HRESULT hr = E_FAIL;

	// According to AMD's suggestion, PCOM must set background color if there's null plane or only 1 graphic plane.
	// So we bypass the background plane even it's not a used.
	if (!IsD3D9PlaneValid(&plane) && id != PLANE_BACKGROUND)
		return S_FALSE;

	hr = m_pTexturePool->GetRepresentation(plane.hTexture, __uuidof(IDirect3DSurface9), (void **)&pSurface);
	if (0)
	{
#ifndef _NO_USE_D3DXDLL
		hr = D3DXSaveSurfaceToFile(_T("C:\\do.bmp"), D3DXIFF_BMP, pSurface, NULL, NULL);

		CComPtr<IDirect3DTexture9> pTexturePriv;
		CComPtr<IDirect3DSurface9> pTexturePrivSurf;
		hr = m_pDevice->CreateTexture(1920, 1080, 1, D3DUSAGE_RENDERTARGET, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &pTexturePriv, NULL);
		hr = pTexturePriv->GetSurfaceLevel(0, &pTexturePrivSurf);
		hr = m_pDevice->StretchRect(pSurface, NULL, pTexturePrivSurf, NULL, D3DTEXF_POINT);
		hr = D3DXSaveSurfaceToFile(_T("C:\\reRT.bmp"), D3DXIFF_BMP, pTexturePrivSurf, NULL, NULL);
#endif
	}

	if ((!pSurface || id > 5) && id != PLANE_BACKGROUND)
		return E_FAIL;

	RECT rcSrc = plane.rcSrc;
	PCOM_VALID_PLANE_FIELDS ValidPlaneFields = {0};
	//If there don't have Surface, set ValidPlaneFields.hasPlanarAlpha = 0. Because there is nothing to render.
	ValidPlaneFields.hasPlanarAlpha = (pSurface && plane.fAlpha != 1.0f) ? 1 : 0;
	ValidPlaneFields.hasLumaKey = 0;
	ValidPlaneFields.hasBackgroundColor = 0;
	ValidPlaneFields.hasClearRectangles = 0;
	ValidPlaneFields.hasDirtyRect = 0;
	ValidPlaneFields.hasFps = 1;
	ValidPlaneFields.hasCsc = 0;

	PCOMPlane.size = sizeof(PCOM_PLANE);
	PCOMPlane.planeType = PCOM_Generic_Plane;
	PCOMPlane.fieldSelect = PCOM_ProgressiveFrame;
	PCOMPlane.flags.value = 0;
	PCOMPlane.alpha = (unsigned int)(plane.fAlpha * 255.0f);
	PCOMPlane.clearRectCount = 0;

	// It's dangerous to use main video frame rate as reference always.
	PCOMPlane.fps = (float)m_Planes[PLANE_MAINVIDEO].VideoProperty.dwFrameRate1000 / 1000;
	PCOMPlane.plane = pSurface;

	if (id == PLANE_BACKGROUND)
	{
		//If there has nothing to render, set ValidPlaneFields.hasBackgroundColor = 1. Default of value is 0.
		if(!pSurface || !IsBackgroundVisible())
		{
			ValidPlaneFields.hasBackgroundColor = 1;
			PCOMPlane.backgroundColor = m_colorBackGround;
			PCOMPlane.plane = 0;
		}
		else
		{
			PCOMPlane.fps = 1.0f;
		}
	}
	else if (id == PLANE_MAINVIDEO)
	{
        CComPtr<IDirect3DTexture9> pTexture;
        CComPtr<IUnknown> pUnk;

        if (ProcessVideoEffect(PLANE_MAINVIDEO, NULL, rcSrc,rcDst, plane.Format, &pUnk) != S_FALSE)
        {
            if (pUnk)
            {
                pSurface.Release();
                hr = pUnk->QueryInterface(IID_IDirect3DTexture9, (void **)&pTexture);
                if (SUCCEEDED(hr))
                {
                    hr = pTexture->GetSurfaceLevel(0, &pSurface);	ASSERT(SUCCEEDED(hr));
#ifndef _NO_USE_D3DXDLL
                    if (0)
                    {
                        hr = D3DXSaveTextureToFile(_T("C:\\doTexture.bmp"), D3DXIFF_BMP, pTexture, NULL);
                    }
#endif
                }
                else
                {
                    hr = pUnk->QueryInterface(IID_IDirect3DSurface9, (void **)&pSurface);
#ifndef _NO_USE_D3DXDLL
                    if (0)
                    {
                        hr = D3DXSaveSurfaceToFile(_T("C:\\doSurface.bmp"), D3DXIFF_BMP, pSurface, NULL, NULL);
                    }
#endif
                }
                PCOMPlane.plane = pSurface;
            }
        }
    	else if (plane.Format != PLANE_FORMAT_ARGB)
		{
			PCOMPlane.planeType = PCOM_MainVideo_Plane;
			ValidPlaneFields.hasCsc = 1;

			PCOMPlane.csc.cscRange = PCOM_CSC_Nominal_Range_0_255;
			PCOMPlane.csc.cscMatrix = plane.bHDVideo ? PCOM_CSC_Matrix_BT709 : PCOM_CSC_Matrix_BT601;

			// if temporally de-interlaced, PCOMPlane.fieldSelect sets to PCOM_ProgressiveFrame.
			if (!m_bSpatialDeinterlacing && plane.pVBlt && plane.VideoProperty.Format != VIDEO_FORMAT_PROGRESSIVE)
			{
				CComPtr<IDirect3DSurface9> pIntermediateSurface;
				const RECT rcSrc = {0, 0, plane.uWidth, plane.uHeight};

				if (SUCCEEDED(plane.pVBlt->IntermediateVBlt(pSurface, rcSrc, plane.Format, &pIntermediateSurface)))
					PCOMPlane.plane = pIntermediateSurface;	// use the intermediate result for PCOM input.
			}
			else
			{
				// use PCOM de-interlacer
				SelectDisplayField(PCOMPlane.fieldSelect, plane.VideoProperty);
			}
		}
	}
	else if (id == PLANE_SUBVIDEO)
	{
		PCOMPlane.planeType = PCOM_SubVideo_Plane;
		SelectDisplayField(PCOMPlane.fieldSelect, plane.VideoProperty);

		if (m_LumaKey.bEnable)
		{
			ValidPlaneFields.hasLumaKey = 1;
			PCOMPlane.minLumaKey = m_LumaKey.uLower;
			PCOMPlane.maxLumaKey = m_LumaKey.uUpper;
		}

		PCOMPlane.fps = (float)m_Planes[PLANE_SUBVIDEO].VideoProperty.dwFrameRate1000 / 1000;

		ValidPlaneFields.hasCsc = 1;
		PCOMPlane.csc.cscRange = PCOM_CSC_Nominal_Range_0_255;
		PCOMPlane.csc.cscMatrix = plane.bHDVideo ? PCOM_CSC_Matrix_BT709 : PCOM_CSC_Matrix_BT601;
	}

	if (!m_ClearRectList.empty())
	{
		if (id == PLANE_SUBVIDEO || id == PLANE_GRAPHICS)
		{
			// Maybe it's needed to clear the array here.
			//ZeroMemory(&PCOMPlane.clearRectList, sizeof(PCOM_CLEAR_RECTANGLE)*PCOM_MAX_CLEAR_RECTANGLES_COUNT);
			PCOMPlane.clearRectCount = m_dwPCOMClearRecCount;
			memcpy(&PCOMPlane.clearRectList, m_PCOMClearRecList, sizeof(PCOM_CLEAR_RECTANGLE)*m_dwPCOMClearRecCount);
			ValidPlaneFields.hasClearRectangles = PCOMPlane.clearRectCount > 0;
		}
	}

	// If the plane is created for XGUI, the rect of destination should be equal to screen size(m_rcDst) in full screen mode.
	if (plane.bFullScreenMixing && m_bFullScreen)
		CalculatePCOMRect(PCOMPlane.srcRect, PCOMPlane.dstRect, plane, rcSrc, m_rcDst, rcDstClip, fWindowAspectRatio);
	else
		CalculatePCOMRect(PCOMPlane.srcRect, PCOMPlane.dstRect, plane, rcSrc, rcDst, rcDstClip, fWindowAspectRatio);
	PCOMPlane.validFields = ValidPlaneFields;
	return S_OK;
}

HRESULT CAMDPCOMMixerPresenter::CreateMCOM()
{
	HRESULT hr = E_FAIL;
	MCOM_STATUS ret = MCOM_FAIL;
	MCOM_CREATE_INPUT CreateIn = {0};
	MCOM_CREATE_OUTPUT CreateOut = {0};

	CreateIn.size = sizeof(MCOM_CREATE_INPUT);
	CreateOut.size = sizeof(MCOM_CREATE_OUTPUT);

	CreateIn.flags = 0;
	CreateIn.windowHandle = m_hwnd;
	CreateIn.GfxDevice = m_pDevice;

	ret = MCOMCreate(&CreateIn, &CreateOut);
	if (ret != MCOM_OK)
	{
		DbgMsg("MCOMCreate return value: 0x%08X", ret);
		return E_FAIL;
	}
	m_pMcomSession = CreateOut.MCOMSession;
	hr = QueryDecodeStreamCaps();

	return hr;
}

HRESULT CAMDPCOMMixerPresenter::DestroyMCOM()
{
	if (m_pMcomSession)
	{
		MCOM_STATUS ret = MCOM_FAIL;
		ret = MCOMDestroy(m_pMcomSession);
		ASSERT(ret == MCOM_OK);
		m_pMcomSession = NULL;
	}
	return S_OK;
}

HRESULT CAMDPCOMMixerPresenter::QueryDecodeStreamCaps()
{
	MCOM_STATUS ret = MCOM_FAIL;
	MCOM_GET_BLURAY_DECODE_STREAM_INPUT DecodeStreamIn = {0};
	MCOM_GET_BLURAY_DECODE_STREAM_OUTPUT DecodeStreamOut = {0};

	DecodeStreamIn.size = sizeof(MCOM_GET_BLURAY_DECODE_STREAM_INPUT);
	DecodeStreamOut.size = sizeof(MCOM_GET_BLURAY_DECODE_STREAM_OUTPUT);

	DecodeStreamIn.MCOMSession = m_pMcomSession;

	ret = MCOMBluRayDecodeStreamCaps(&DecodeStreamIn, &DecodeStreamOut);
	if (ret != MCOM_OK)
	{
		DbgMsg("MCOMBluRayDecodeStreamCaps return value: 0x%08X", ret);
		return E_FAIL;
	}

	memcpy(&m_DecodeStreamCaps, &DecodeStreamOut.DecodeStreamCaps, sizeof(MCOM_DECODE_STREAM_CAPS));

	return S_OK;
}

STDMETHODIMP CAMDPCOMMixerPresenter::_QueryCaps(PresenterCaps* pCaps)
{
	if(pCaps->VideoDecodeCaps > 0)
	{
		if(m_DecodeStreamCaps.CAP_2HD_OTHER == 0) // no restriction
		{
			m_PresenterCaps.bHwDecode = TRUE;
		}
		else if(m_DecodeStreamCaps.CAP_2HD_PROGRESSIVE == 0) //both HD streams in progressive format only
		{
			if((pCaps->VideoDecodeCaps & VIDEO_CAP_STREAM_SUB) && 
				(pCaps->VideoDecodeCaps & VIDEO_CAP_INTERLACE) && 
				(pCaps->VideoDecodeCaps & (VIDEO_CAP_FORMAT_1080|VIDEO_CAP_FORMAT_720)))
				m_PresenterCaps.bHwDecode = FALSE;
			else
				m_PresenterCaps.bHwDecode = TRUE;
		}
		else if(m_DecodeStreamCaps.CAP_1HD_1SD == 0) //1 SD + 1 HD streams in both interlaced and progressive format
		{
			if((pCaps->VideoDecodeCaps & VIDEO_CAP_STREAM_SUB) && 
				(pCaps->VideoDecodeCaps & (VIDEO_CAP_FORMAT_1080|VIDEO_CAP_FORMAT_720)))
				m_PresenterCaps.bHwDecode = FALSE;
			else
				m_PresenterCaps.bHwDecode = TRUE;
		}
		else if(m_DecodeStreamCaps.CAP_1HD_OTHER == 0) //one HD stream in either interlaced or progressive format
		{
			m_PresenterCaps.bHwDecode = (pCaps->VideoDecodeCaps & VIDEO_CAP_STREAM_SUB) ? FALSE : TRUE;
		}
		else if(m_DecodeStreamCaps.CAP_1HD_PROGRESSIVE == 0) //one HD in progressive format (for instance, some RV610 only supports this)
		{
			if(!(pCaps->VideoDecodeCaps & VIDEO_CAP_STREAM_SUB) && 
				!(pCaps->VideoDecodeCaps & VIDEO_CAP_INTERLACE))
				m_PresenterCaps.bHwDecode = TRUE;
			else
				m_PresenterCaps.bHwDecode = FALSE;
		}
		else //this ASIC don't support DXVA.
		{
			m_PresenterCaps.bHwDecode = FALSE;
		}
	}

	return S_OK;
}
