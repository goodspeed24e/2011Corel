// AmdPCOMRender.cpp : Implementation of CAMDPCOMMixerPresenter
#include "stdafx.h"

#include "AmdPCOMMixerPresenter.h"
#include "D3D9VideoEffect3DManager.h"
#include <map>

using namespace DispSvr;
#define PRESENT_INTERVAL	1
#define D3DUSAGE_RESTRICTED_CONTENT              (0x00000800L)

enum  // From D3DHelper.cpp
{
	BACKBUFFER_COUNT_FLIP_MODE = 3,
	BACKBUFFER_COUNT_DEFAULT = 0
};

enum
{
    PCOM_V0 = 0,
    PCOM_V1 = 1,
    PCOM_V2 = 2,
    PCOM_V3 = 3,
};

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

static DWORD ToStereoVision(DWORD dwAMDStereoMode)
{
    std::map<const DWORD, DWORD> Map;
    
    Map[PCOM_STEREOSCOPIC_FORMAT_FRAME_SEQUENTIAL]                        = STEREO_VISION_AMD_FRAME_SEQENTIAL;
    Map[PCOM_STEREOSCOPIC_FORMAT_PIXEL_INTERLEAVED_ROW]                   = STEREO_VISION_ROW_INTERLEAVED;
    Map[PCOM_STEREOSCOPIC_FORMAT_PIXEL_INTERLEAVED_COLUMN]                = STEREO_VISION_COLUMN_INTERLEAVED;
    Map[PCOM_STEREOSCOPIC_FORMAT_PIXEL_INTERLEAVED_CHECKERBOARD]          = STEREO_VISION_CHECHERBOARD;
    Map[PCOM_STEREOSCOPIC_FORMAT_VERTICALLY_STACKED_HALF_RESOLUTION]      = STEREO_VISION_HALF_TOP_BOTTOM_LR;  //?
    Map[PCOM_STEREOSCOPIC_FORMAT_HORIZONTAL_SIDE_BY_SIDE_HALF_RESOLUTION] = STEREO_VISION_SIDE_BY_SIDE;  //?
    Map[PCOM_STEREOSCOPIC_FORMAT_HDMI_14]                                 = STEREO_VISION_HDMI_STEREO;
    Map[PCOM_STEREOSCOPIC_FORMAT_DP_12]                                   = STEREO_VISION_DP_STEREO;
    Map[PCOM_STEREOSCOPIC_FORMAT_ANAGLYPH_REDCYAN]                        = STEREO_VISION_ANAGLYPH;

    std::map<const DWORD, DWORD>::iterator iter = Map.find(dwAMDStereoMode);
    if (iter != Map.end())
        return iter->second;
    else
        return NULL;
}

static PCOM_STEREOSCOPIC_FORMAT ToAMDStereoMode(DWORD dwStereoMode)
{
    std::map<const DWORD, PCOM_STEREOSCOPIC_FORMAT> Map;
    
    Map[MIXER_STEREO_MODE_DISABLED]            = PCOM_STEREOSCOPIC_FORMAT_INVALID;
    Map[MIXER_STEREO_MODE_ANAGLYPH]            = PCOM_STEREOSCOPIC_FORMAT_ANAGLYPH_REDCYAN;  //?
    Map[MIXER_STEREO_MODE_SIDEBYSIDE]          = PCOM_STEREOSCOPIC_FORMAT_HORIZONTAL_SIDE_BY_SIDE_HALF_RESOLUTION; //?
    Map[MIXER_STEREO_MODE_CHECKERBOARD]        = PCOM_STEREOSCOPIC_FORMAT_PIXEL_INTERLEAVED_CHECKERBOARD;
    Map[MIXER_STEREO_MODE_OPTIMIZED_ANAGLYPH]  = PCOM_STEREOSCOPIC_FORMAT_ANAGLYPH_REDCYAN; //?
    Map[MIXER_STEREO_MODE_HALFCOLOR_ANAGLYPH]  = PCOM_STEREOSCOPIC_FORMAT_ANAGLYPH_REDCYAN; //?
    Map[MIXER_STEREO_MODE_ROW_INTERLEAVED]     = PCOM_STEREOSCOPIC_FORMAT_PIXEL_INTERLEAVED_ROW;
    Map[MIXER_STEREO_MODE_HALFCOLOR2_ANAGLYPH] = PCOM_STEREOSCOPIC_FORMAT_ANAGLYPH_REDCYAN; //?
    Map[MIXER_STEREO_MODE_COLUMN_INTERLEAVED]  = PCOM_STEREOSCOPIC_FORMAT_PIXEL_INTERLEAVED_COLUMN;
    Map[MIXER_STEREO_MODE_COLOR_ANAGLYPH]      = PCOM_STEREOSCOPIC_FORMAT_ANAGLYPH_REDCYAN; //?
    Map[MIXER_STEREO_MODE_AMD_ACTIVE_STEREO]   = PCOM_STEREOSCOPIC_FORMAT_FRAME_SEQUENTIAL;
    Map[MIXER_STEREO_MODE_HALF_SIDEBYSIDE_LR]  = PCOM_STEREOSCOPIC_FORMAT_HORIZONTAL_SIDE_BY_SIDE_HALF_RESOLUTION; //?
    Map[MIXER_STEREO_MODE_HALF_SIDEBYSIDE_RL]  = PCOM_STEREOSCOPIC_FORMAT_HORIZONTAL_SIDE_BY_SIDE_HALF_RESOLUTION; //?
    Map[MIXER_STEREO_MODE_HALF_TOPBOTTOM_LR]   = PCOM_STEREOSCOPIC_FORMAT_VERTICALLY_STACKED_HALF_RESOLUTION; //?
    Map[MIXER_STEREO_MODE_HALF_TOPBOTTOM_RL]   = PCOM_STEREOSCOPIC_FORMAT_VERTICALLY_STACKED_HALF_RESOLUTION; //?
    Map[MIXER_STEREO_MODE_HDMI_STEREO]         = PCOM_STEREOSCOPIC_FORMAT_HDMI_14;
    Map[MIXER_STEREO_MODE_DP_STEREO]           = PCOM_STEREOSCOPIC_FORMAT_DP_12;
    
    std::map<const DWORD, PCOM_STEREOSCOPIC_FORMAT>::iterator iter = Map.find(dwStereoMode);
    if (iter != Map.end())
        return iter->second;
    else
        return PCOM_STEREOSCOPIC_FORMAT_INVALID;
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
	m_bTemporalDeinterlacing = false;
	m_bAsyncPresent = true;
	// MIXER_CAP_CAN_CHANGE_DESTINATION, MIXER_CAP_3D_RENDERTARGET, MIXER_CAP_VIRTUALIZE_FROM_ORIGIN are not supported.
	m_MixerCaps.dwFlags = 0;
	m_pSwapChain = NULL;
	m_ePresentMode = D3D9_PRESENT_MODE_WINDOWED;
	m_iRenderTargetIndex = 0;
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
	::OffsetRect(&m_rcDst, GetRegistry(REG_DISPLAY_X, 0) - pt.x, GetRegistry(REG_DISPLAY_Y, 0) - pt.y);
	
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

	if(m_ePresentMode == D3D9_PRESENT_MODE_FLIPEX)
		return PresentEx();
	else
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
		hr = DestroyPCOM();
		hr = CreatePCOM();
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
			DbgMsg("AMDPCOM: XvYcc OFF.");
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
			DbgMsg("AMDPCOM: XvYcc ON.");
			return S_OK;
		}
	}
    else
    {
        DbgMsg("AMDPCOM: No XvYcc supports.");
    }

	return E_NOTIMPL;
}

STDMETHODIMP CAMDPCOMMixerPresenter::SetScreenCaptureDefense(BOOL bEnable)
{
    CAutoLock lock(&m_csLock);

    if (m_PresenterCaps.bIsOverlay || (m_ePresentMode == D3D9_PRESENT_MODE_FLIPEX))
        return S_OK;
    else
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

	m_pDevice->GetSwapChain(0, &m_pSwapChain);
	m_ePresentMode = D3D9_PRESENT_MODE_WINDOWED;

	if (m_pSwapChain)
	{
		D3DPRESENT_PARAMETERS sPresentParam;
		hr = m_pSwapChain->GetPresentParameters(&sPresentParam);
		if(SUCCEEDED(hr))
		{
			if (sPresentParam.Windowed == FALSE)
				m_ePresentMode = D3D9_PRESENT_MODE_FULLSCREEN;
			else if (sPresentParam.SwapEffect == D3DSWAPEFFECT_FLIPEX)
				m_ePresentMode = D3D9_PRESENT_MODE_FLIPEX;
			// Get the video hwnd for PCOM
			CD3D9PluginBase::_SetWindow(sPresentParam.hDeviceWindow);
		}
	}

	DbgMsg("PCOM value = %d", m_ePresentMode);

	if (SUCCEEDED(hr))
	{
		DestroyPCOM();
		// Create PCOM related resource with D3D device
		hr = CreatePCOM();
		if (SUCCEEDED(hr))
		{
			hr = InsertPCOMVP();
		}
	}

	return hr;
}

STDMETHODIMP CAMDPCOMMixerPresenter::_ReleaseDevice()
{
	CAutoLock lock(&m_csLock);
	HRESULT hr;

	DestroyPCOM();

	SAFE_RELEASE(m_pSwapChain);
	hr = CD3D9VideoMixerBase::_ReleaseDevice();
	hr |= CD3D9VideoPresenterBase::_ReleaseDevice();
	return hr;
}

HRESULT CAMDPCOMMixerPresenter::_Execute(IDirect3DSurface9 *pDestSurface, const RECT &, const RECT &rcDstClip)
{
	CAutoLock lock(&m_csLock);
	HRESULT hr = E_FAIL;
	RECT rcDst(m_rcDst); // The rect of destination.

    NRectToRect(rcDst, m_pModel->GetDestination());
	if (rcDst.top > rcDst.bottom || rcDst.left > rcDst.right)
		return E_FAIL;

	PCOM_EXECUTE_INPUT ExecuteIn = {0};
	ExecuteIn.size = sizeof(PCOM_EXECUTE_INPUT);
	ExecuteIn.planeCount = 0;

    //Each plane should refer to the their own mixing area's aspect ratio, not whole mixing area(Window)
    // mixing area's aspect ratio = (window(whole mixing area) width * viewport's normalized width) /
    //                                             (window(whole mixing area) height * viewport's normalized height)
    float fMixingAreaAspectRatio[PLANE_MAX];
    for (int i = 0; i < DispSvr::PLANE_MAX; i++)
    {
        fMixingAreaAspectRatio[i] = m_fWindowAspectRatio;
        const D3D9Plane &plane = m_pModel->GetPlane(i);

        if (plane.bValid)
            fMixingAreaAspectRatio[i] = m_fWindowAspectRatio * (plane.nrcDst.right - plane.nrcDst.left) / (plane.nrcDst.bottom - plane.nrcDst.top);
    }

	// PCOM clear rectangle will apply current and below layers except main video & background.
	// So we can only apply clear rectangle to SubVidoe & SPIC layer.
	// We need to check HDi layer in order to pass clear rectangle & coordinates to SubVideo & SPIC layer,
    const ClearRectList &ClearRectList = m_pModel->GetClearRectangles();
	if (ClearRectList.size() > 0 && ClearRectList.size() <= PCOM_MAX_CLEAR_RECTANGLES_COUNT)
	{
        const D3D9Plane &plane = m_pModel->GetPlane(PLANE_INTERACTIVE);
		ASSERT(plane.bValid == true);
		m_dwPCOMClearRecCount = 0;
		PCOM_RECT srcRect, dstRect;
		RECT rcSrc = plane.rcSrc;
		CalculatePCOMRect(0, srcRect, dstRect, plane, rcSrc, rcDst, rcDstClip, fMixingAreaAspectRatio[PLANE_INTERACTIVE]);

		PCOM_CLEAR_RECTANGLE *pCR = m_PCOMClearRecList;
		for (ClearRectList::const_iterator it = ClearRectList.begin(); it != ClearRectList.end(); ++it)
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
		hr = PlaneToPCOMPlane(i, &ExecuteIn, rcDst, rcDstClip, fMixingAreaAspectRatio[i]);
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

HRESULT CAMDPCOMMixerPresenter::SetProperty(const MixerProperty *pProperty)
{
    CAutoLock lock(&m_csLock);
    HRESULT hr = E_FAIL;
    MixerProperty OriProperty;

    hr = CD3D9VideoMixerBase::GetProperty(&OriProperty);
    hr = CD3D9VideoMixerBase::SetProperty(pProperty);
    if((OriProperty.bStereoEnable != pProperty->bStereoEnable) || 
        ((OriProperty.eStereoMode != pProperty->eStereoMode) && pProperty->bStereoEnable))  //Enable Stereo mode or Change Stereo mode
    {
        hr = DestroyPCOM();
        hr = CreatePCOM();
    }
    return hr;
}

HRESULT CAMDPCOMMixerPresenter::CreatePCOM()
{
	PCOM_STATUS ret = PCOM_OK;
	HRESULT hr = E_FAIL;
	PCOM_GET_CAPS_INPUT GetCapsIn = {0};
	PCOM_GET_CAPS_OUTPUT GetCapsOut = {0};

	GetCapsOut.size = sizeof(PCOM_GET_CAPS_OUTPUT);
	GetCapsIn.size = sizeof(PCOM_GET_CAPS_INPUT);
	GetCapsIn.GfxDevice = m_pDevice;

	ret = PCOMGetCaps(&GetCapsIn, &GetCapsOut);

	unsigned int uVersionMajor = GetCapsOut.revision >> 16;
	unsigned int uVersionMinor = GetCapsOut.revision & 0x00000001;

	if((m_ePresentMode == D3D9_PRESENT_MODE_FLIPEX) && (!GetCapsOut.flags.supportAppTargets || !m_pDeviceEx))
	{
		DbgMsg("PCOM Driver does not support PCOM DWM");
		return E_FAIL;
	}

	DWORD dwBBWidth = GetRegistry(REG_BACKBUFFER_WIDTH, 0);
	DWORD dwBBHeight = GetRegistry(REG_BACKBUFFER_HEIGHT, 0);

	PCOM_CREATE_INPUT CreateIn = {0};
	PCOM_CREATE_OUTPUT CreateOut = {0};

	CreateOut.size = sizeof(PCOM_CREATE_OUTPUT);
	CreateIn.size = sizeof(PCOM_CREATE_INPUT);

	CreateIn.flags.value = 0;

	if(m_ePresentMode == D3D9_PRESENT_MODE_FLIPEX)
    {
		CreateIn.flags.useOnlyAppTargets = 1;
    }
	else
	{
		CreateIn.flags.useOverlayPresent = 1;
		CreateIn.flags.useAsyncPresent = (m_bAsyncPresent) ? 1 : 0;
	}

	CreateIn.overlayColorKey = m_dwColorKey;
	CreateIn.mainVideoWidth = dwBBWidth;
	CreateIn.mainVideoHeight = dwBBHeight;
	CreateIn.windowHandle = m_hwnd;
	CreateIn.GfxDevice = m_pDevice;

    // For PCOMv3
    MIXER_STEREO_MODE eStereoMode = (MIXER_STEREO_MODE) GetRegInt(TEXT("StereoMixerMode"), GetMixerStereoMode());
    CreateIn.stereoscopicFormat = ToAMDStereoMode(eStereoMode);
    CreateIn.flags.useStereoscopicMode = (m_Property.bStereoEnable) ? 1 : 0;
    // Could not create PCOM with stereo mode if the stereoscopic format is invalid.
    if(PCOM_STEREOSCOPIC_FORMAT_INVALID == CreateIn.stereoscopicFormat)
        CreateIn.flags.useStereoscopicMode = 0;
    DbgMsg("Enable Stereo mode = %d, Stereo mode is %d,", CreateIn.flags.useStereoscopicMode, CreateIn.stereoscopicFormat);

	ret = PCOMCreate(&CreateIn, &CreateOut);

	if (ret != PCOM_OK)
	{
        ASSERT(0);
        DbgMsg("PCOMCreate return value: 0x%08X", ret);
        return E_FAIL;
	}
    else
    {
        DbgMsg("Creating PCOM OK!");
    }

	m_pSession = CreateOut.PCOMSession;
	// Update presenter caps
	m_PresenterCaps.dwFPS = GetCapsOut.recomFps;
	m_PresenterCaps.dwResPixels = GetCapsOut.recomWindowPixelCnt;
	m_PresenterCaps.dwBandWidth = GetCapsOut.systemToVideoBandwidth;
	m_PresenterCaps.bSupportXvYCC = (GetCapsOut.flags.supportXvYCCWideGamutDisplay) ? TRUE : FALSE;
	m_PresenterCaps.bIsOverlay = (m_ePresentMode == D3D9_PRESENT_MODE_FLIPEX) ? FALSE : TRUE;
	m_bSpatialDeinterlacing = (GetCapsOut.flags.supportSpatialDeinterlacing) ? true : false;
	m_bTemporalDeinterlacing = (GetCapsOut.flags.supportTemporalDeinterlacing) ? true : false;
	//if(uVersionMajor < PCOM_V2 || (uVersionMajor == PCOM_V2 && uVersionMinor == 0))  //Could not use TemporalDeinterlacing if the version of PCOM is under 2.1
    // Turn off PCOMv2 first because bug#111341, we have to report it to AMD.
	m_bTemporalDeinterlacing = false;

	if(m_ePresentMode == D3D9_PRESENT_MODE_FLIPEX)
    {
		m_PresenterCaps.dwPresenterInfo = (PRESENTER_FLIPEX | PRESENTER_D3D);
        m_dwFlipChainSize = BACKBUFFER_COUNT_FLIP_MODE;
    }
    else
    {
		m_PresenterCaps.dwPresenterInfo = PRESENTER_PROPRIETARYOVERLAY;
        m_dwFlipChainSize = static_cast<DWORD>(CreateOut.flipChainSize);
    }

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

HRESULT CAMDPCOMMixerPresenter::InsertPCOMVP()
{
	m_PlaneConfigs[PLANE_GRAPHICS].eTextureUsage	= TEXTURE_USAGE_LOCKABLE_BACKSTORE;
	m_PlaneConfigs[PLANE_INTERACTIVE].eTextureUsage = TEXTURE_USAGE_LOCKABLE_BACKSTORE;

	m_pTexturePool->SetPoolUsage(TEXTURE_POOL_USAGE_OFFSCREENPLANESURFACE);
	GenerateDxva2VPList();

	// Bug#83555, remove PCOM from preferred mixer selection because PCOM 1.0 only support BOB deinterlace.
	// 1 possible risk is that if the machine don't support both PCOM v2 and DXVA VP,
	// there will no video processer for de-interlace.
	if(m_bTemporalDeinterlacing)
	{
		VideoProcessorStub vp = {0};
		vp.guidVP = DispSvr_VideoProcPCOM;
		vp.RenderTargetFormat = D3DFMT_X8R8G8B8;
		vp.sCaps.eType = PROCESSOR_TYPE_HARDWARE;
		vp.sCaps.uProcessorCaps = PROCESSOR_CAPS_DEINTERLACE_ADAPTIVE;

		for(VideoProcessorList::iterator vi = m_VideoProcessorList.begin(); vi != m_VideoProcessorList.end(); ++vi)
		{
			if (vi->pfnVBltFactory)
			{
				vp.pDelegateVPStub = &*vi;
				break;
			}
		}

		m_VideoProcessorList.push_back(vp);
		DbgMsg("Creating PCOM VP OK - PCOM temporal");
	}
	else
	{
		DbgMsg("Creating PCOM with DXVA2 VP OK");
	}            
	SelectVideoProcessor();
	return S_OK;
}

HRESULT CAMDPCOMMixerPresenter::BeginFramePCOM(const RECT &rcDst)
{
	PCOM_STATUS ret = PCOM_OK;
	PCOM_BEGIN_FRAME_INPUT BeginFrameIn = {0};
	HRESULT hr = E_FAIL;
	BeginFrameIn.size = sizeof(PCOM_BEGIN_FRAME_INPUT);
	BeginFrameIn.flags.enableFullScreenMode = (m_bFullScreen) ? 1 : 0;
	BeginFrameIn.flags.enableXvYCCMetaData = (m_dwGamutFormat == GAMUT_METADATA_RANGE) ? 1 : 0;
	BeginFrameIn.targetRect.left = rcDst.left;
	BeginFrameIn.targetRect.top = rcDst.top;
	BeginFrameIn.targetRect.right = rcDst.right;
	BeginFrameIn.targetRect.bottom = rcDst.bottom;
	BeginFrameIn.xvYCCMetaData = (m_dwGamutFormat == GAMUT_METADATA_RANGE) ? &m_XvYCCGamutMetaData : 0;
    BeginFrameIn.flags.outputLeftView = 1;
    BeginFrameIn.flags.outputRightView = (m_Property.bStereoEnable) ? 1 : 0;
	
	if (m_ePresentMode == D3D9_PRESENT_MODE_FLIPEX)
	{
		CComPtr<IDirect3DSurface9> pBackBuffer;

		hr = m_pDeviceEx->GetRenderTarget(0, &pBackBuffer);
		BeginFrameIn.targetSurface = pBackBuffer;
        BeginFrameIn.flags.enableFullScreenMode = 0; //Using PCOM for DWM, we have to set this flag as 0.
		if(m_iRenderTargetIndex < (m_dwFlipChainSize-1))
			m_iRenderTargetIndex++;

		while(1)
		{
			if(m_dwQueuedFrameCount == m_dwFlipChainSize)
			{
				hr = PresentEx();
			}
			ret = PCOMBeginFrame(m_pSession, &BeginFrameIn);
			break;
		};
	}
	else
	{
		while(1)
		{
			ret = PCOMBeginFrame(m_pSession, &BeginFrameIn);
			if (ret == PCOM_QUEUE_FULL)
			{
				hr = PresentPCOM();
				if (SUCCEEDED(hr))
					continue;
			}
			break;
		};
	}

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

HRESULT CAMDPCOMMixerPresenter::PresentEx()
{
	HRESULT hr = E_FAIL;
	
	if(0)
	{
#ifndef _NO_USE_D3DXDLL
		CComPtr<IDirect3DSwapChain9> pSwapChain;
		CComPtr<IDirect3DSurface9> pBackBuffer2;

		hr = m_pDeviceEx->GetSwapChain(0, &pSwapChain);
		hr = m_pDeviceEx->GetBackBuffer(0, m_iRenderTargetIndex, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer2);
		hr = D3DXSaveSurfaceToFile(_T("C:\\doSurface.bmp"), D3DXIFF_BMP, pBackBuffer2, NULL, NULL);
#endif
	}

	while ((hr = m_pSwapChain->Present(NULL, NULL, NULL, NULL, D3DPRESENT_DONOTWAIT)) == D3DERR_WASSTILLDRAWING)
		Sleep(PRESENT_INTERVAL);

	m_dwQueuedFrameCount--;

	return hr;
}

HRESULT CAMDPCOMMixerPresenter::ClearPCOM()
{
	HRESULT hr = E_FAIL;
	PCOM_STATUS ret = PCOM_OK;

	if(m_ePresentMode == D3D9_PRESENT_MODE_WINDOWED)
	{
		ret = PCOMResetQueue(m_pSession);
		if (ret == PCOM_OK)
		{
			m_dwQueuedFrameCount = 0;
		}
	}
	else
	{
		m_dwQueuedFrameCount = 0;
		m_iRenderTargetIndex = 0;
	}
	return hr;
}

HRESULT CAMDPCOMMixerPresenter::CalculatePCOMRect(UINT uViewID, PCOM_RECT &pcomrcSrc, PCOM_RECT &pcomrcDst, const D3D9Plane &plane, RECT &rcSrc, const RECT &rcMixingDst, const RECT &rcDstClip, float fWindowAspectRatio)
{
    NORMALIZEDRECT nrcOutput = plane.nrcDst;
    RECT rcDst = rcMixingDst;

    CorrectAspectRatio(nrcOutput, plane.fAspectRatio / fWindowAspectRatio);
    // cropping applies to both source and destination.
    CropRect(nrcOutput, plane.nrcCrop);
    CropRect(rcSrc, plane.nrcCrop);
    // Calculate the offset of Base view and depend view.
    INT iOffset = GetStereoOffset(plane);
    if (iOffset != 0 && plane.uWidth != 0)
    {
        float fNormalizedOffset = float(iOffset) / plane.uWidth;
        if (uViewID == 0)	// base view shifts right if offset is positive.
        {
            nrcOutput.left += fNormalizedOffset;
            nrcOutput.right += fNormalizedOffset;
        }
        else if (uViewID == 1)	// depend view shifts left if offset is positive.
        {
            nrcOutput.left -= fNormalizedOffset;
            nrcOutput.right -= fNormalizedOffset;
        }
    }
    // map normailzed destination rectangle to actual rectangle in pixel.
    NRectToRect(rcDst, nrcOutput);
    // check and clip source/destination rectangles when rcDst is outside of rcDstClip.
    ClipRect(rcDst, rcSrc, rcDstClip);

    pcomrcSrc.left = rcSrc.left;
    pcomrcSrc.top = rcSrc.top;
    pcomrcSrc.right = rcSrc.right;
    pcomrcSrc.bottom = rcSrc.bottom;

    pcomrcDst.left = rcDst.left;
    pcomrcDst.top  = rcDst.top;
    pcomrcDst.right = rcDst.right;
    pcomrcDst.bottom = rcDst.bottom;

    return S_OK;
}

HRESULT CAMDPCOMMixerPresenter::PlaneToPCOMPlane(PLANE_ID id, PPCOM_EXECUTE_INPUT pExecuteIn, RECT &rcDst, const RECT &rcDstClip, const float fWindowAspectRatio)
{
	D3D9Plane &plane = m_pModel->GetPlane(id);
	CComPtr<IDirect3DSurface9> pSurface;

	HRESULT hr = E_FAIL;

	// According to AMD's suggestion, PCOM must set background color if there's null plane or only 1 graphic plane.
	// So we bypass the background plane even it's not a used.
    if (!plane.IsValid() && id != PLANE_BACKGROUND)
		return S_FALSE;

    UINT uView = 0;
    hr = m_pTexturePool->GetRepresentation(plane.GetViewTextureHandle(uView), __uuidof(IDirect3DSurface9), (void **)&pSurface);
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

    PCOM_PLANE PCOMPlane = {0};
	PCOMPlane.size = sizeof(PCOM_PLANE);
	PCOMPlane.planeType = PCOM_Generic_Plane;
	PCOMPlane.fieldSelect = PCOM_ProgressiveFrame;
	PCOMPlane.flags.value = 0;
	PCOMPlane.alpha = (unsigned int)(plane.fAlpha * 255.0f);
	PCOMPlane.clearRectCount = 0;

	// It's dangerous to use main video frame rate as reference always.
	PCOMPlane.fps = (float)m_pModel->GetPlane(PLANE_MAINVIDEO).VideoProperty.dwFrameRate1000 / 1000;
	PCOMPlane.plane = pSurface;

	if (id == PLANE_BACKGROUND)
	{
		//If there has nothing to render, set ValidPlaneFields.hasBackgroundColor = 1. Default of value is 0.
		if(!pSurface || !IsBackgroundVisible())
		{
			// Set default value to normalized RECT before CalculatePCOMRect()
            plane.nrcDst = plane.nrcCrop = m_pModel->GetDestination();

			ValidPlaneFields.hasBackgroundColor = 1;
            COLORREF bkgColor = 0;
            m_pModel->GetBackgroundColor(&bkgColor);

            PCOMPlane.backgroundColor = bkgColor;
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
        else if (m_pVideoEffect3DBlt && m_pVideoEffect3DBlt->IsEffectEnabled())
        {
            hr = m_pTexturePool->GetRepresentation(plane.hTexture, __uuidof(IDirect3DTexture9), (void **)&pTexture);

            if (FAILED(hr))
            {
                hr = plane.pVBlt->IntermediateTextureVBlt(pSurface, rcSrc, &pTexture);
            }

            if (SUCCEEDED(hr))
            {
                CComPtr<IUnknown> pOutputSurface;
                ProcessEffectRequest Request = {0};
                Request.lpSourceRect = &rcSrc;
                Request.lpTargetRect = &rcDst;
                Request.pVideoProperty = &plane.VideoProperty;
                Request.pInput = pTexture;
                Request.ppOutput = &pOutputSurface;
                hr = m_pVideoEffect3DBlt->ProcessEffect(&Request);
                if (SUCCEEDED(hr))
                {
                    pTexture.Release();
                    hr = pOutputSurface->QueryInterface(IID_IDirect3DTexture9, (void **)&pTexture);
                    if (SUCCEEDED(hr))
                    {
                        pSurface.Release();
                        hr = pTexture->GetSurfaceLevel(0, &pSurface);	ASSERT(SUCCEEDED(hr));
                        PCOMPlane.plane = pSurface;
                    }
                }
            }
        }
    	else if (plane.Format != PLANE_FORMAT_ARGB)
		{
			PCOMPlane.planeType = PCOM_MainVideo_Plane;
			ValidPlaneFields.hasCsc = 1;
			ValidPlaneFields.hasStreamID = 1;
			PCOMPlane.streamID = PLANE_MAINVIDEO;

			PCOMPlane.csc.cscRange = PCOM_CSC_Nominal_Range_0_255;
			PCOMPlane.csc.cscMatrix = plane.bHDVideo ? PCOM_CSC_Matrix_BT709 : PCOM_CSC_Matrix_BT601;

			// if temporally de-interlaced, PCOMPlane.fieldSelect sets to PCOM_ProgressiveFrame.
			if (!m_bTemporalDeinterlacing && !m_bSpatialDeinterlacing && plane.pVBlt && plane.VideoProperty.Format != VIDEO_FORMAT_PROGRESSIVE)
			{
				CComPtr<IDirect3DSurface9> pIntermediateSurface;
				const RECT rcSrc = {0, 0, plane.uWidth, plane.uHeight};

                // we should remove this once GPI can set correct flags.
                if(GetRegistry(REG_OS_VERSION, 0) >= OS_WIN7) // We have to creating this surface with D3DUSAGE_RESTRICTED_CONTENT when using PreventRead on GPUCP
                    plane.VideoProperty.bRestrictedContent = TRUE;
				if (SUCCEEDED(plane.pVBlt->IntermediateVBlt(pSurface, rcSrc, plane.Format, &pIntermediateSurface)))
					PCOMPlane.plane = pIntermediateSurface;	// use the intermediate result for PCOM input.
			}
			else
			{
				// use PCOM de-interlacer
				SelectDisplayField(PCOMPlane.fieldSelect, plane.VideoProperty);
				if(m_bTemporalDeinterlacing)
				{
					ValidPlaneFields.hasTimestamp = 1;
                    PCOMPlane.startTimestamp = plane.VideoProperty.rtStart;
                    PCOMPlane.endTimestamp = plane.VideoProperty.rtEnd;
				}
			}
		}
	}
	else if (id == PLANE_SUBVIDEO)
	{
		PCOMPlane.planeType = PCOM_SubVideo_Plane;
		SelectDisplayField(PCOMPlane.fieldSelect, plane.VideoProperty);

        LumaKey lumaKey = {0};
        m_pModel->GetLumaKey(&lumaKey);
		if (lumaKey.bEnable)
		{
			ValidPlaneFields.hasLumaKey = 1;
			PCOMPlane.minLumaKey = lumaKey.uLower;
			PCOMPlane.maxLumaKey = lumaKey.uUpper;
		}

		PCOMPlane.fps = (float)m_pModel->GetPlane(PLANE_SUBVIDEO).VideoProperty.dwFrameRate1000 / 1000;

		ValidPlaneFields.hasCsc = 1;
		PCOMPlane.csc.cscRange = PCOM_CSC_Nominal_Range_0_255;
		PCOMPlane.csc.cscMatrix = plane.bHDVideo ? PCOM_CSC_Matrix_BT709 : PCOM_CSC_Matrix_BT601;
		ValidPlaneFields.hasStreamID = 1;
		PCOMPlane.streamID = PLANE_SUBVIDEO;

		if(m_bTemporalDeinterlacing)
		{
			ValidPlaneFields.hasTimestamp = 1;
			PCOMPlane.startTimestamp = plane.VideoProperty.rtStart;
			PCOMPlane.endTimestamp = plane.VideoProperty.rtEnd;
		}
	}

    if (!m_pModel->GetClearRectangles().empty())
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

    PCOMPlane.validFields = ValidPlaneFields;

    for(uView = 0; uView < 2; uView++)
    {
        // If the plane is created for XGUI, the rect of destination should be equal to screen size(m_rcDst) in full screen mode.
        if (plane.bFullScreenMixing && m_bFullScreen)
            CalculatePCOMRect(uView, PCOMPlane.srcRect, PCOMPlane.dstRect, plane, rcSrc, m_rcDst, rcDstClip, fWindowAspectRatio);
        else if(m_bFullScreen || m_ePresentMode != D3D9_PRESENT_MODE_FLIPEX)
            CalculatePCOMRect(uView, PCOMPlane.srcRect, PCOMPlane.dstRect, plane, rcSrc, rcDst, rcDstClip, fWindowAspectRatio);
        else
        {
            rcDst.top = rcDstClip.top;
            rcDst.left = rcDstClip.left;
            rcDst.right = rcDstClip.right;
            rcDst.bottom = rcDstClip.bottom;

            CalculatePCOMRect(uView, PCOMPlane.srcRect, PCOMPlane.dstRect, plane, rcSrc, rcDst, rcDstClip, fWindowAspectRatio);
        }

        if(uView == 0)
        {
            pExecuteIn->planeList[pExecuteIn->planeCount] = PCOMPlane;
        }
        else  //if(uView == 1); if(m_Property.bStereoEnable), Enable Stereo Video playback
        {
            CComPtr<IDirect3DSurface9> pDependSurface;
            if(SUCCEEDED(m_pTexturePool->GetRepresentation(plane.GetViewTextureHandle(uView), __uuidof(IDirect3DSurface9), (void **)&pDependSurface)))
                PCOMPlane.plane = pDependSurface;

            pExecuteIn->planeListRightView[pExecuteIn->planeCount] = PCOMPlane;
        }
    }

	return S_OK;
}

CAMDDeviceExtensionAdapter::CAMDDeviceExtensionAdapter()
{
	m_pDevice9 = NULL;
	m_pMcomSession = NULL;
    m_hWnd = NULL;
    m_hMonitor = NULL;
	ZeroMemory(&m_pMcomSession, sizeof(MCOM_DECODE_STREAM_CAPS));
	ZeroMemory(&m_PCOMCapsOutput,sizeof(PCOM_GET_CAPS_OUTPUT));
	ZeroMemory(&m_AdapterInfo,sizeof(DrvExtAdapterInfo));
	m_DecodeStreamCaps.CAP_2HD_OTHER = m_DecodeStreamCaps.CAP_2HD_PROGRESSIVE = m_DecodeStreamCaps.CAP_1HD_1SD = 0x80000000;
}

CAMDDeviceExtensionAdapter::CAMDDeviceExtensionAdapter(IDirect3DDevice9 *pDevice)
{
	m_pDevice9 = pDevice;
	m_pDevice9->AddRef();
    m_pMcomSession = NULL;
    m_hWnd = NULL;
    m_hMonitor = NULL;
    ZeroMemory(&m_pMcomSession, sizeof(MCOM_DECODE_STREAM_CAPS));
    ZeroMemory(&m_PCOMCapsOutput,sizeof(PCOM_GET_CAPS_OUTPUT));
    ZeroMemory(&m_AdapterInfo,sizeof(DrvExtAdapterInfo));
    m_DecodeStreamCaps.CAP_2HD_OTHER = m_DecodeStreamCaps.CAP_2HD_PROGRESSIVE = m_DecodeStreamCaps.CAP_1HD_1SD = 0x80000000;
}

CAMDDeviceExtensionAdapter::~CAMDDeviceExtensionAdapter()
{
	DestroyMCOM();
	SAFE_RELEASE(m_pDevice9);
}

HRESULT CAMDDeviceExtensionAdapter::SetDevice(IDirect3DDevice9 *pDevice9)
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


HRESULT CAMDDeviceExtensionAdapter::CreateMCOM(HWND hwnd)
{
	HRESULT hr = E_FAIL;
	MCOM_STATUS ret = MCOM_FAIL;
	MCOM_CREATE_INPUT CreateIn = {0};
	MCOM_CREATE_OUTPUT CreateOut = {0};

	CreateIn.size = sizeof(MCOM_CREATE_INPUT);
	CreateOut.size = sizeof(MCOM_CREATE_OUTPUT);

	CreateIn.flags = 0;
	CreateIn.windowHandle = hwnd;
	CreateIn.GfxDevice = m_pDevice9;

	ret = MCOMCreate(&CreateIn, &CreateOut);
	if (ret != MCOM_OK)
	{
		DbgMsg("MCOMCreate return value: 0x%08X", ret);
		return hr;
	}
    else
        hr = S_OK;
	m_pMcomSession = CreateOut.MCOMSession;
	hr = QueryDecodeStreamCaps();

	return hr;
}

HRESULT CAMDDeviceExtensionAdapter::DestroyMCOM()
{
	if (m_pMcomSession)
	{
		MCOM_STATUS ret = MCOM_FAIL;
		ret = MCOMDestroy(m_pMcomSession);
		m_pMcomSession = NULL;
	}
	return S_OK;
}

HRESULT CAMDDeviceExtensionAdapter::QueryDecodeStreamCaps()
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

HRESULT CAMDDeviceExtensionAdapter::GetAdapter(IDriverExtensionAdapter **ppAdapter)
{
	*ppAdapter = 0;
	*ppAdapter = new CAMDDeviceExtensionAdapter();

	return S_OK;
}

HRESULT CAMDDeviceExtensionAdapter::QueryPresenterCaps(DWORD VideoDecodeCaps, PresenterCaps* pCaps)
{
    HRESULT hr = E_FAIL;
	if(!m_pMcomSession && m_pDevice9)
	{
		CComPtr<IDirect3DSwapChain9> pSwapChain;
		D3DPRESENT_PARAMETERS sPresentParam;

		m_pDevice9->GetSwapChain(0, &pSwapChain);
		if (SUCCEEDED(pSwapChain->GetPresentParameters(&sPresentParam)))
		{
			hr = CreateMCOM(sPresentParam.hDeviceWindow);
		}
	}

	if(VideoDecodeCaps > 0 && m_pMcomSession)
	{
		if(m_DecodeStreamCaps.CAP_2HD_OTHER == 0) // no restriction
		{
			pCaps->bHwDecode = TRUE;
		}
		else if(m_DecodeStreamCaps.CAP_2HD_PROGRESSIVE == 0) //both HD streams in progressive format only
		{
			if((VideoDecodeCaps & VIDEO_CAP_STREAM_SUB) && 
				(VideoDecodeCaps & VIDEO_CAP_INTERLACE) && 
				(VideoDecodeCaps & (VIDEO_CAP_FORMAT_1080|VIDEO_CAP_FORMAT_720)))
				pCaps->bHwDecode = FALSE;
			else
				pCaps->bHwDecode = TRUE;
		}
		else if(m_DecodeStreamCaps.CAP_1HD_1SD == 0) //1 SD + 1 HD streams in both interlaced and progressive format
		{
			if((VideoDecodeCaps & VIDEO_CAP_STREAM_SUB) && 
				(VideoDecodeCaps & (VIDEO_CAP_FORMAT_1080|VIDEO_CAP_FORMAT_720)))
				pCaps->bHwDecode = FALSE;
			else
				pCaps->bHwDecode = TRUE;
		}
		else if(m_DecodeStreamCaps.CAP_1HD_OTHER == 0) //one HD stream in either interlaced or progressive format
		{
			pCaps->bHwDecode = (VideoDecodeCaps & VIDEO_CAP_STREAM_SUB) ? FALSE : TRUE;
		}
		else if(m_DecodeStreamCaps.CAP_1HD_PROGRESSIVE == 0) //one HD in progressive format (for instance, some RV610 only supports this)
		{
			if(!(VideoDecodeCaps & VIDEO_CAP_STREAM_SUB) && 
				!(VideoDecodeCaps & VIDEO_CAP_INTERLACE))
				pCaps->bHwDecode = TRUE;
			else
				pCaps->bHwDecode = FALSE;
		}
		else //this ASIC don't support DXVA.
		{
			pCaps->bHwDecode = FALSE;
		}
	}

	return hr;
}

HRESULT CAMDDeviceExtensionAdapter::QueryHDMIStereoModeCaps(HWND hWnd, DriverExtHDMIStereoModeCap **ppCaps, UINT *puCount)
{
	CHECK_POINTER(ppCaps);
	CHECK_POINTER(puCount);

	HRESULT hr = E_FAIL;

	hr = GetPCOMCaps(hWnd);

    //Query all display mode of HDMI1.4
    int iNumHDMI14 = 0;
    for(unsigned int i = 0; i < m_PCOMCapsOutput.supportedStereoscopicDisplayModeCount; i++)
    {
        if(m_PCOMCapsOutput.supportedStereoscopicDisplayModes[i].stereoscopicFormat == PCOM_STEREOSCOPIC_FORMAT_HDMI_14)
            iNumHDMI14++;
    }

    if(iNumHDMI14 > 0)
    {
        (*ppCaps) = new DriverExtHDMIStereoModeCap[iNumHDMI14];
        ZeroMemory((*ppCaps), sizeof(DriverExtHDMIStereoModeCap) * iNumHDMI14);
        (*puCount) = iNumHDMI14;

        int j = 0;
        for(unsigned int i = 0; i < m_PCOMCapsOutput.supportedStereoscopicDisplayModeCount; i++)
        {
            if(m_PCOMCapsOutput.supportedStereoscopicDisplayModes[i].stereoscopicFormat == PCOM_STEREOSCOPIC_FORMAT_HDMI_14)
            {
                (*ppCaps)[j].uWidth = m_PCOMCapsOutput.supportedStereoscopicDisplayModes[i].width;
                (*ppCaps)[j].uHeight = m_PCOMCapsOutput.supportedStereoscopicDisplayModes[i].height;
                (*ppCaps)[j].uRefreshRate = m_PCOMCapsOutput.supportedStereoscopicDisplayModes[i].refreshRate;
                (*ppCaps)[j].dwStereoMode = HDMI_STEREO_FRAME_PACKING;
                j++;
            }
        }
    }

	return S_OK;
}

HRESULT CAMDDeviceExtensionAdapter::EnableHDMIStereoMode(BOOL bEnable, DriverExtHDMIStereoModeCap *pCap, BOOL *pbReCreateDevice)
{
	CHECK_POINTER(pCap);
	CHECK_POINTER(pbReCreateDevice);

	HRESULT hr = E_FAIL;
	MixerProperty MixerProp;

	(*pbReCreateDevice) = TRUE;

	CComPtr<IDispSvrVideoMixer> pDispSvrVideoMixer;
	if(SUCCEEDED(CResourceManager::GetInstance()->GetInterface(DISPSVR_RESOURCE_VIDEOMIXER, __uuidof(IDispSvrVideoMixer), (void **)&pDispSvrVideoMixer)))
	{
		pDispSvrVideoMixer->GetProperty(&MixerProp);
		MixerProp.eStereoMode = (bEnable) ? MIXER_STEREO_MODE_HDMI_STEREO : MIXER_STEREO_MODE_DISABLED;

		//Need to change display setting
		DEVMODE devMode;
		DWORD dwFlag = CDS_RESET;
		memset(&devMode, 0, sizeof(devMode));
		devMode.dmSize = sizeof(devMode);
		devMode.dmPelsWidth = pCap->uWidth;
		devMode.dmPelsHeight = pCap->uHeight;
		devMode.dmDisplayFrequency = pCap->uRefreshRate;
		devMode.dmBitsPerPel = 32;   // exclusive mode only supports 32 bits color depth.
		devMode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY | DM_BITSPERPEL;
		ChangeDisplaySettings(&devMode, dwFlag);

		//Re-creating PCOM
		if(SUCCEEDED(pDispSvrVideoMixer->SetProperty(&MixerProp)))
		{
			DbgMsg ("CAMDDeviceExtensionAdapter::EnableHDMIStereoMode - Success.\n");
		}
		else
		{
			DbgMsg ("CAMDDeviceExtensionAdapter::EnableHDMIStereoMode - Fail.\n");
		}
	}

	return S_OK;
}

HRESULT CAMDDeviceExtensionAdapter::QueryAdapterInfo(HWND hWnd, HMONITOR hMonitor, DispSvr::DrvExtAdapterInfo *pInfo)
{
	CHECK_POINTER(pInfo);
	HRESULT hr = E_FAIL;

	hr = GetPCOMCaps(hWnd);
	memcpy(pInfo, &m_AdapterInfo, sizeof(DrvExtAdapterInfo));

	return hr;
}

HRESULT CAMDDeviceExtensionAdapter::GetPCOMCaps(HWND hwnd)
{
    HRESULT hr = E_FAIL;
	IDirect3DDevice9 *pDevice = NULL;
	PCOM_STATUS ret = PCOM_OK;
	PCOM_GET_CAPS_INPUT GetCapsIn = {0};

    HMONITOR hMonitor = MonitorFromWindow(hwnd, (hwnd) ? MONITOR_DEFAULTTONEAREST : MONITOR_DEFAULTTOPRIMARY);
	if(m_hMonitor == hMonitor && m_hWnd == hwnd)  // Don't query PCOMcaps again if video window handler and monitor hanlder are the same.
		return S_FALSE;

	ZeroMemory(&m_PCOMCapsOutput,sizeof(PCOM_GET_CAPS_OUTPUT));
	m_PCOMCapsOutput.size = sizeof(PCOM_GET_CAPS_OUTPUT);
	GetCapsIn.size = sizeof(PCOM_GET_CAPS_INPUT);

	if(!m_pDevice9)
	{
		if(FAILED(CreateFakeD3DDevice(hwnd, &pDevice)))
			return hr;

        GetCapsIn.GfxDevice = pDevice;
	}
	else
	{
		GetCapsIn.GfxDevice = m_pDevice9;
	}

	ret = PCOMGetCaps(&GetCapsIn, &m_PCOMCapsOutput);
	if (ret != PCOM_OK)
		return hr;
    else
        hr = S_OK;

	m_AdapterInfo.bIsXVYCCSupported = m_PCOMCapsOutput.flags.supportXvYCCWideGamutDisplay;

    if((m_PCOMCapsOutput.revision>>16) >= PCOM_V3)  //PCOMv3 Caps
    {
        //Query all supported format and save them.
        for(unsigned int i = 0; i < m_PCOMCapsOutput.supportedStereoscopicFormatCount; i++)
        {
            m_AdapterInfo.dwSupportStereoFormats |= ToStereoVision(m_PCOMCapsOutput.supportedStereoscopicFormats[i]);
        }
        //Query all display mode of HDMI1.4
        int iNumHDMI14 = 0;
        int iNumDP12 = 0;
        int iNumFrameSequential = 0;
        for(unsigned int i = 0; i < m_PCOMCapsOutput.supportedStereoscopicDisplayModeCount; i++)
        {
            switch (m_PCOMCapsOutput.supportedStereoscopicDisplayModes[i].stereoscopicFormat)
            {
            case PCOM_STEREOSCOPIC_FORMAT_HDMI_14:
                iNumHDMI14++;
                break;
            case PCOM_STEREOSCOPIC_FORMAT_FRAME_SEQUENTIAL:
                iNumFrameSequential++;
                break;
            case PCOM_STEREOSCOPIC_FORMAT_DP_12:
                iNumDP12++;
                break;
            default:
                break;
            }
        }

        if(iNumHDMI14 == 0) // There is no display mode about HDMI1.4
        {
            m_AdapterInfo.dwSupportStereoFormats &= ~STEREO_VISION_HDMI_STEREO;
            DbgMsg("PCOMv3 does not support HDMI1.4, no HDMI1.4 display format");
        }
        if(iNumFrameSequential == 0) // There is no display mode about frame sequential
        {
            m_AdapterInfo.dwSupportStereoFormats &= ~STEREO_VISION_AMD_FRAME_SEQENTIAL;
            DbgMsg("PCOMv3 does not support frame seqential, no frame seqential display format");
        }
        if(iNumDP12 == 0) // There is no display mode about DP1.2
        {
            m_AdapterInfo.dwSupportStereoFormats &= ~STEREO_VISION_DP_STEREO;
            DbgMsg("PCOMv3 does not support DP1.2, no DP1.2 display format");
        }
    }

	m_hWnd = hwnd;
    m_hMonitor = hMonitor;
    SAFE_RELEASE(pDevice);

	return hr;
}

HRESULT CAMDDeviceExtensionAdapter::CreateFakeD3DDevice(HWND hwnd, IDirect3DDevice9 **pDevice)
{
	HRESULT hr = E_FAIL;

	// Creating a temp d3d device. That is a workaround solution due to we don't have current device when calling this function.
	CComPtr<IDirect3D9> pD3D9;

	pD3D9 = Direct3DCreate9(D3D_SDK_VERSION);

	if (!pD3D9)
	{
		DbgMsg("CAMDDeviceExtensionAdapter::CreateFakeD3DDevice: failed to create Direct3D9 object");
		return hr;
	}

	D3DPRESENT_PARAMETERS pp = {0};
	pp.Windowed = TRUE;
	pp.hDeviceWindow = hwnd;
	pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	pp.BackBufferCount = 0;
	pp.Flags = D3DPRESENTFLAG_DEVICECLIP;
	pp.BackBufferFormat = D3DFMT_X8R8G8B8;

	hr = pD3D9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd, D3DCREATE_DISABLE_DRIVER_MANAGEMENT_EX|D3DCREATE_SOFTWARE_VERTEXPROCESSING, &pp, pDevice);
	if (FAILED(hr))
	{
		DbgMsg("CAMDDeviceExtensionAdapter::CreateD3DDevice9Ex: failed to create Direct3D9Ex Device, version = %d", D3D_SDK_VERSION);
		return hr;
	}

	return hr;
}