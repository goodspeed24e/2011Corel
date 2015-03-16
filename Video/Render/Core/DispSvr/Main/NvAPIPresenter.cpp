#include "stdafx.h"
#include "DispSvr.h"
#include "ResourceManager.h"
#include "RegistryService.h"
#include "DynLibManager.h"
#include "RegistryService.h"
#include "NvAPIPresenter.h"
#include "Imports/LibGPU/GPUID.h"
#include "Imports/LibGPU/UtilGPU.h"

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
    m_pDeviceCap = 0;
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
            return hr;

        hr = UpdateDisplayProperty();
        if (FAILED(hr))
            return hr;

        hr = CreateNvAPI();
        if (SUCCEEDED(hr))
        {
            // Update PresenterCaps
            m_PresenterCaps.bIsOverlay = TRUE;
        }
        else
        {
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
        SAFE_DELETE(m_pDeviceCap);
    }
    return CD3D9VideoPresenterBase::_ReleaseDevice();
}

STDMETHODIMP CNvAPIPresenter::SetDisplayRect(const RECT *rcDst, const RECT *rcSrc)
{
    HRESULT hr = CD3D9VideoPresenterBase::SetDisplayRect(rcDst, rcSrc);
    if (SUCCEEDED(hr))
    {
        SIZE szDisplay = {m_rcMonitor.right - m_rcMonitor.left, m_rcMonitor.bottom - m_rcMonitor.top};
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

STDMETHODIMP CNvAPIPresenter::_QueryCaps(PresenterCaps* pCaps)
{
    return m_pDeviceCap->QueryPresenterCaps(pCaps->VideoDecodeCaps, &m_PresenterCaps);
}

HRESULT CNvAPIPresenter::InitNvAPI()
{
    CAutoLock lock(&m_csLock);

    HRESULT hr = CNvAPIDeviceExtensionAdapter::GetAdapter(m_pDevice, &m_pDeviceCap);
    if (FAILED(hr))
        return hr;

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

    return hr;
}

HRESULT CNvAPIPresenter::CreateNvAPI()
{
    m_dwFlipQueueHint  = m_bIsInterlacedDisplay ? 4 : 2;

    NvAPI_Status nvret = NVAPI_OK;
    HRESULT hr = E_FAIL;
    CComPtr<IDirect3DSurface9> pRenderTarget;
    D3DSURFACE_DESC desc;
    DWORD dwBBWidth = GetRegistry(REG_BACKBUFFER_WIDTH, m_rcMonitor.right - m_rcMonitor.left);
    DWORD dwBBHeight = GetRegistry(REG_BACKBUFFER_HEIGHT, m_rcMonitor.bottom - m_rcMonitor.top);

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
        DWORD dwTotal = GetRegistry(REG_LOCAL_VIDMEM, 256);

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
        m_bIsInterlacedDisplay = (INTERLACE_DISPLAY_THRESHOLD < m_fScanLineInterval * (m_rcMonitor.bottom - m_rcMonitor.top));
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

HRESULT CNvAPIDeviceExtensionAdapter::GetAdapter(IDirect3DDevice9 *pDevice9, IDriverExtensionAdapter **ppAdapter)
{
    CHECK_POINTER(pDevice9);
    NvAPI_Status nvret = NvAPI_Initialize();

    *ppAdapter = 0;
    if (nvret != NVAPI_OK)
        return E_NOTIMPL;

    *ppAdapter = new CNvAPIDeviceExtensionAdapter(pDevice9);
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
            if ( (packet.videoCaps[0].perfLevel & (1 << NV_PERF_LEVEL_FRUC_BIT)) != 0)
            {
                pCaps->bSupportFRUC = TRUE;
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
