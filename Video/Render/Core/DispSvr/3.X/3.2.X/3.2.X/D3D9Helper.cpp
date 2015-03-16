#include "stdafx.h"
#include "HDMIStereoModeHelper.h"
#include "D3D9Helper.h"

using namespace DispSvr;

enum
{
    BACKBUFFER_COUNT_FLIP_MODE = 3,
    BACKBUFFER_COUNT_DEFAULT = 0
};

static inline bool UseIntelD3D9Overlay(DWORD d)
{
   return d != PCI_DEVICE_ID_CANTIGA || d != PCI_DEVICE_ID_EAGLELAKE;
}

bool IsNVCoprocDevicePresent(DWORD &dwDeviceId);
bool IsNVCoprocEnabledForApp();

///////////////////////////////////////////////////////////////////////////////
/// D3D9 Responder
struct CD3D9ResponderBase : public CD3D9Helper::ICreationChainResponder
{
    virtual HRESULT Create(const CD3D9Helper::ConfigParam &cfg, const CD3D9Helper::CreationParam &p, CD3D9Helper::CreationOutput &out) const
    {
        HRESULT hr = CustomParam(cfg, p, out);
        if (SUCCEEDED(hr))
            hr = p.pD3D9->CreateDevice(p.uAdapter, p.DeviceType, p.PresentParam.hDeviceWindow, p.dwBehaviorFlag, &out.PresentParam, &out.pDevice);
        return hr;
    }

    virtual HRESULT Reset(const CD3D9Helper::ConfigParam &cfg, const CD3D9Helper::CreationParam &p, CD3D9Helper::CreationOutput &out) const
    {
        HRESULT hr = CustomParam(cfg, p, out);
        if (SUCCEEDED(hr))
            hr = p.pDevice->Reset(&out.PresentParam);
        return hr;
    }

    virtual HRESULT CustomParam(const CD3D9Helper::ConfigParam &c, const CD3D9Helper::CreationParam &p, CD3D9Helper::CreationOutput &output) const = 0;
};

struct CD3D9Responder : public CD3D9ResponderBase
{
    virtual HRESULT CustomParam(const CD3D9Helper::ConfigParam &cfg, const CD3D9Helper::CreationParam &p, CD3D9Helper::CreationOutput &out) const
    {
        ASSERT(p.pD3D9);
        return S_OK;
    }
};

struct CD3D9ExclusiveModeResponder : public CD3D9ResponderBase
{
    virtual HRESULT CustomParam(const CD3D9Helper::ConfigParam &cfg, const CD3D9Helper::CreationParam &p, CD3D9Helper::CreationOutput &out) const
    {
        ASSERT(p.pD3D9);
        ASSERT(!GetParent(p.PresentParam.hDeviceWindow));   // window handle must be top-level if entering exclusive mode

        out.PresentParam.Windowed = FALSE;
        out.PresentParam.FullScreen_RefreshRateInHz = p.DisplayMode.RefreshRate;
        return S_OK;
    }
};


///////////////////////////////////////////////////////////////////////////////
/// D3D9Ex Responder
struct CD3D9ExResponderBase : public CD3D9Helper::ICreationChainResponder
{
    virtual HRESULT Create(const CD3D9Helper::ConfigParam &cfg, const CD3D9Helper::CreationParam &p, CD3D9Helper::CreationOutput &out) const
    {
        HRESULT hr = CustomParam(cfg, p, out);
        if (SUCCEEDED(hr))
            hr = p.pD3D9Ex->CreateDeviceEx(p.uAdapter, p.DeviceType, p.PresentParam.hDeviceWindow, p.dwBehaviorFlag, &out.PresentParam, NULL, &out.pDeviceEx);
        return hr;
    }

    virtual HRESULT Reset(const CD3D9Helper::ConfigParam &cfg, const CD3D9Helper::CreationParam &p, CD3D9Helper::CreationOutput &out) const
    {
        HRESULT hr = CustomParam(cfg, p, out);
        if (SUCCEEDED(hr))
            hr = p.pDeviceEx->ResetEx(&out.PresentParam, NULL);
        return hr;
    }

    virtual HRESULT CustomParam(const CD3D9Helper::ConfigParam &c, const CD3D9Helper::CreationParam &p, CD3D9Helper::CreationOutput &output) const = 0;
};

struct CD3D9ExResponder : public CD3D9ExResponderBase
{
    virtual HRESULT CustomParam(const CD3D9Helper::ConfigParam &cfg, const CD3D9Helper::CreationParam &p, CD3D9Helper::CreationOutput &out) const
    {
        ASSERT(p.pD3D9Ex);
        DWORD dwOSVersion = GetRegistry(REG_OS_VERSION, 0);
        if(dwOSVersion < OS_WIN7)
        {
            out.PresentParam.Flags &= ~D3DPRESENTFLAG_RESTRICTED_CONTENT;
        }
        return S_OK;
    }
};

struct CD3D9ExExclusiveModeResponder : public CD3D9Helper::ICreationChainResponder
{
    virtual HRESULT Create(const CD3D9Helper::ConfigParam &cfg, const CD3D9Helper::CreationParam &p, CD3D9Helper::CreationOutput &out) const
    {
        HRESULT hr = CustomParam(cfg, p, out);
        if (SUCCEEDED(hr))
        {
            D3DDISPLAYMODEEX DisplayModeEx = p.DisplayModeEx;   // display mode can be changed by CreateDeviceEx

            // p.PresentParam.hDeviceWindow must be the top window when entering exclusive mode, can't be NULL.
            hr = p.pD3D9Ex->CreateDeviceEx(p.uAdapter, p.DeviceType, p.PresentParam.hDeviceWindow, p.dwBehaviorFlag, &out.PresentParam, &DisplayModeEx, &out.pDeviceEx);
        }
        return hr;
    }

    virtual HRESULT Reset(const CD3D9Helper::ConfigParam &cfg, const CD3D9Helper::CreationParam &p, CD3D9Helper::CreationOutput &out) const
    {
        HRESULT hr = CustomParam(cfg, p, out);
        if (SUCCEEDED(hr))
        {
            D3DDISPLAYMODEEX DisplayModeEx = p.DisplayModeEx;   // display mode can be changed by CreateDeviceEx
            hr = p.pDeviceEx->ResetEx(&out.PresentParam, &DisplayModeEx);
        }
        return hr;
    }

    virtual HRESULT CustomParam(const CD3D9Helper::ConfigParam &c, const CD3D9Helper::CreationParam &p, CD3D9Helper::CreationOutput &out) const
    {
        ASSERT(p.pD3D9Ex);
        ASSERT(!GetParent(p.PresentParam.hDeviceWindow));   // window handle must be top-level if entering exclusive mode

        out.PresentParam.Windowed = FALSE;
        out.PresentParam.FullScreen_RefreshRateInHz = p.DisplayModeEx.RefreshRate;
        out.PresentParam.SwapEffect = D3DSWAPEFFECT_FLIP;
        out.PresentParam.BackBufferCount = BACKBUFFER_COUNT_FLIP_MODE;
        out.PresentParam.BackBufferFormat = p.DisplayModeEx.Format;
        return S_OK;
    }
};

template<D3DFORMAT BackBufferFormat>
struct CD3D9ExOverlayResponder : public CD3D9ExResponderBase
{
    virtual HRESULT CustomParam(const CD3D9Helper::ConfigParam &cfg, const CD3D9Helper::CreationParam &p, CD3D9Helper::CreationOutput &out) const
    {
        ASSERT(p.pD3D9Ex);

        HRESULT hr = E_FAIL;
        if (0 == (p.Caps.Caps & D3DCAPS_OVERLAY))
            return hr;

        CComQIPtr<IDirect3D9ExOverlayExtension> pOverlayExtension = p.pD3D9Ex;
        if (!pOverlayExtension)
            return hr;

        out.PresentParam.BackBufferFormat = BackBufferFormat;   // overlay format must be explicitly specified

        D3DOVERLAYCAPS overlayCaps;
        hr = pOverlayExtension->CheckDeviceOverlayType(p.uAdapter, p.DeviceType,
            out.PresentParam.BackBufferWidth, out.PresentParam.BackBufferHeight, out.PresentParam.BackBufferFormat,
            NULL, p.Rotation, &overlayCaps);
        if (FAILED(hr))
            return hr;

        out.uD3D9OverlayCaps = overlayCaps.Caps;
        // for some back buffer format, eg. YUY2, AutoDepthStencil may fail CreateDevice()
        if (out.PresentParam.EnableAutoDepthStencil)
        {
            hr = p.pD3D9Ex->CheckDepthStencilMatch(p.uAdapter, p.DeviceType, p.DisplayModeEx.Format, out.PresentParam.BackBufferFormat, p.PresentParam.AutoDepthStencilFormat);
            if (FAILED(hr))
                out.PresentParam.EnableAutoDepthStencil = FALSE;
        }

        // According to NV's suggestion, we set to 3 or more for better performance.
        // #80137, inverse telecine operation may be backed up waiting for other operations to continue and VPBltHD calls can be blocked.
        // MSDN : http://msdn.microsoft.com/en-us/library/bb172588(VS.85).aspx
        out.PresentParam.BackBufferCount = BACKBUFFER_COUNT_FLIP_MODE;
        out.PresentParam.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
        out.PresentParam.SwapEffect = D3DSWAPEFFECT_OVERLAY;

        if (PCI_VENDOR_ID_NVIDIA == p.AdapterIdentifier.VendorId)
            out.PresentParam.Flags |= D3DPRESENTFLAG_VIDEO;
        if (p.bIsXvYCCMonitor && ((D3DOVERLAYCAPS_YCbCr_BT709_xvYCC | D3DOVERLAYCAPS_YCbCr_BT601_xvYCC) & overlayCaps.Caps) != 0)
            out.PresentParam.Flags |= D3DPRESENTFLAG_OVERLAY_YCbCr_xvYCC;

        return hr;
    }
};

struct CD3D9ExFlipExResponder : public CD3D9ExResponderBase
{
    virtual HRESULT CustomParam(const CD3D9Helper::ConfigParam &cfg, const CD3D9Helper::CreationParam &p, CD3D9Helper::CreationOutput &out) const
    {
        ASSERT(p.pD3D9Ex);

        out.PresentParam.SwapEffect = D3DSWAPEFFECT_FLIPEX;
        out.PresentParam.BackBufferCount = BACKBUFFER_COUNT_FLIP_MODE;
        out.PresentParam.Flags &= ~D3DPRESENTFLAG_DEVICECLIP; // FlipEx fails if D3DPRESENTFLAG_DEVICECLIP is set.
		out.PresentParam.Flags &= ~D3DPRESENTFLAG_RESTRICT_SHARED_RESOURCE_DRIVER; //AP will hang on calling Present if adding this flag on AMD case (PCOM+FlipEx). Don't know root cause yet.
        return S_OK;
    }
};


// functor instances.
static const CD3D9Responder s_D3D9CreationResponder;
static const CD3D9ExResponder s_D3D9ExCreationResponder;
static const CD3D9ExExclusiveModeResponder s_D3D9ExExclusiveModeResponder;
static const CD3D9ExOverlayResponder<D3DFMT_X8R8G8B8> s_D3D9ExOverlayCreationResponder;
static const CD3D9ExOverlayResponder<D3DFMT_YUY2> s_D3D9ExYuy2OverlayCreationResponder;
static const CD3D9ExFlipExResponder s_D3D9ExFlipExCreationResponder;

// Creation chain must be null terminated.
static const CD3D9Helper::ICreationChainResponder *s_D3D9CreationChain[] = { &s_D3D9CreationResponder, NULL };
static const CD3D9Helper::ICreationChainResponder *s_D3D9ExCreationChain[] = { &s_D3D9ExCreationResponder, &s_D3D9CreationResponder, NULL };
static const CD3D9Helper::ICreationChainResponder *s_D3D9ExFlipExCreationChain[] =
{
    &s_D3D9ExFlipExCreationResponder,
    &s_D3D9ExCreationResponder,
    &s_D3D9CreationResponder,
    NULL
};

static const CD3D9Helper::ICreationChainResponder *s_D3D9ExOverlayCreationChain[] =
{
    &s_D3D9ExOverlayCreationResponder,
    &s_D3D9ExCreationResponder,
    &s_D3D9CreationResponder,
    NULL
};

static const CD3D9Helper::ICreationChainResponder *s_D3D9ExYuy2OverlayCreationChain[] =
{
    &s_D3D9ExYuy2OverlayCreationResponder,
    &s_D3D9ExCreationResponder,
    &s_D3D9CreationResponder,
    NULL
};

static const CD3D9Helper::ICreationChainResponder *s_D3D9ExExclusiveCreationChain[] =
{
    &s_D3D9ExExclusiveModeResponder,
    &s_D3D9CreationResponder,
    NULL
};

const REGISTRY_TYPE CD3D9Helper::s_pInitializersByD3D9Helper[] = {
    REG_VENDOR_ID, REG_DEVICE_ID,
    REG_BACKBUFFER_WIDTH, REG_BACKBUFFER_HEIGHT,
    REG_DISPLAY_X, REG_DISPLAY_Y,
    REG_DISPLAY_WIDTH, REG_DISPLAY_HEIGHT, REG_DISPLAY_REFRESH_RATE,
    REG_DISPLAY_XVYCC_MONITOR_TYPE,
    REG_COPROC_VENDOR_ID, REG_COPROC_DEVICE_ID, REG_COPROC_ACTIVE_VENDOR_ID
};

CD3D9Helper::CD3D9Helper()
{
    ZeroMemory(&m_CreationParam, sizeof(m_CreationParam));
    ZeroMemory(&m_ConfigParam, sizeof(m_ConfigParam));
    m_ppCreationChain = NULL;
    m_hwnVideoWindow = NULL;
    m_pFlipExWindow = NULL;
    CHDMIStereoModeHelper::GetHelper(&m_pHDMIHelper);
    ASSERT(m_pHDMIHelper);

    for (int i = 0; i < sizeof(s_pInitializersByD3D9Helper) / sizeof(REGISTRY_TYPE); i++)
        CRegistryService::GetInstance()->BindInitializer(s_pInitializersByD3D9Helper[i], this);
}

CD3D9Helper::~CD3D9Helper()
{
    for (int i = 0; i < sizeof(s_pInitializersByD3D9Helper) / sizeof(REGISTRY_TYPE); i++)
        CRegistryService::GetInstance()->BindInitializer(s_pInitializersByD3D9Helper[i], NULL);

    SAFE_RELEASE(m_pHDMIHelper);
    SAFE_DELETE(m_pFlipExWindow);
}

HRESULT CD3D9Helper::GetRegistryValue(REGISTRY_TYPE eType, registry_t &value)
{
    const CreationParam &cp = m_CreationParam;
    switch (eType)
    {
    case REG_VENDOR_ID:
        ASSERT(cp.AdapterIdentifier.VendorId);
        value = cp.AdapterIdentifier.VendorId;
        break;
    case REG_DEVICE_ID:
        ASSERT(cp.AdapterIdentifier.DeviceId);
        value = cp.AdapterIdentifier.DeviceId;
        break;
    case REG_BACKBUFFER_WIDTH:
        ASSERT(cp.PresentParam.BackBufferWidth);
        value = cp.PresentParam.BackBufferWidth;
        break;
    case REG_BACKBUFFER_HEIGHT:
        ASSERT(cp.PresentParam.BackBufferHeight);
        value = cp.PresentParam.BackBufferHeight;
        break;
    case REG_DISPLAY_X:
        value = cp.MonitorInfoEx.rcMonitor.left;
        break;
    case REG_DISPLAY_Y:
        value = cp.MonitorInfoEx.rcMonitor.top;
        break;
    case REG_DISPLAY_WIDTH:
        ASSERT(cp.DisplayMode.Width);
        value = cp.DisplayMode.Width;
        break;
    case REG_DISPLAY_HEIGHT:
        ASSERT(cp.DisplayMode.Height);
        value = cp.DisplayMode.Height;
        break;
    case REG_DISPLAY_REFRESH_RATE:
        ASSERT(cp.DisplayMode.RefreshRate);
        value = cp.DisplayMode.RefreshRate;
        break;
    case REG_DISPLAY_XVYCC_MONITOR_TYPE:
        ASSERT(cp.PresentParam.Flags);
        if (cp.PresentParam.Flags & D3DPRESENTFLAG_OVERLAY_YCbCr_xvYCC)
        {
            if (cp.uD3D9OverlayCaps & D3DOVERLAYCAPS_YCbCr_BT709_xvYCC)
                value = DISPLAY_XVYCC_MONITOR_BT709;
            else
                value = DISPLAY_XVYCC_MONITOR_BT601;
        }
        else
        {
            value = DISPLAY_XVYCC_MONITOR_NOT_SUPPORT;
        }
        break;
    case REG_COPROC_VENDOR_ID:
        if (cp.bIsCoprocEnabledForApp)
        {
            value = PCI_VENDOR_ID_NVIDIA;
            break;
        }
        return E_FAIL;
    case REG_COPROC_DEVICE_ID:
        if (cp.bIsCoprocEnabledForApp)
        {
            value = cp.dwCoprocDeviceId;
            break;
        }
        return E_FAIL;
    case REG_COPROC_ACTIVE_VENDOR_ID:
        if (cp.bIsCoprocEnabledForApp && IsD3D9Overlay())
        {
            value = PCI_VENDOR_ID_NVIDIA;
            break;
        }
        return E_FAIL;
    default:
        ASSERT(0);  // shouldn't happen if the REGISTRY_TYPE is not binded first.
        return E_UNEXPECTED;
    }

    return S_OK;
}

HRESULT CD3D9Helper::CreateDevice(HWND hwnd, const ConfigParam &p)
{
    HRESULT hr = E_FAIL;

    m_hwnVideoWindow = hwnd;
    m_ConfigParam = p;

    hr = CreateD3D9();
    if (FAILED(hr))
        return hr;

    hr = GetAdapterIdentifier();
    if (FAILED(hr))
        return hr;

    hr = InitCreationParam();
    if (FAILED(hr))
        return hr;

    hr = SetupResponderChain();
    if (FAILED(hr))
        return hr;

    hr = Create();
    if (SUCCEEDED(hr))
        InitDevice();
    return hr;
}

HRESULT CD3D9Helper::ResetDevice(const ConfigParam &p)
{
	HRESULT hr = E_FAIL;

    m_ConfigParam = p;
    // ResetDevice should be called only when there is a device.
    // We do not handle non-D3D9Ex Reset.
    if (!m_pDevice || !m_pDeviceEx)
        return hr;

    hr = GetAdapterIdentifier();
    if (FAILED(hr))
        return hr;

    hr = InitCreationParam();
    if (FAILED(hr))
        return hr;

    hr = SetupResponderChain();
    if (FAILED(hr))
        return hr;

    hr = Reset();
	return hr;
}

HRESULT CD3D9Helper::ReleaseDevice()
{
    ZeroMemory(&m_CreationParam, sizeof(m_CreationParam));
    ZeroMemory(&m_ConfigParam, sizeof(m_ConfigParam));
	m_pDeviceEx.Release();

    if (m_pDevice)
    {
        IDirect3DDevice9 *pDevice = m_pDevice.Detach();
        int ref = pDevice->Release();
        DbgMsg("CD3D9Helper::ReleaseDevice() IDirect3DDevice9 *m_pDevice refcount = %d, better be 0.", ref);
//        ASSERT(ref == 0);	// this may be a fatal error.
    }

    m_pD3D9.Release();
    m_pD3D9Ex.Release();

    return S_OK;
}

HRESULT CD3D9Helper::Create()
{
    HRESULT hr = E_FAIL;
    CreationOutput out;

    ASSERT(!m_pDevice);
    ASSERT(!m_pDeviceEx);
    for (const ICreationChainResponder **p = m_ppCreationChain; *p; p++)
    {
        ZeroMemory(&out, sizeof(out));
        // make a copy because CreateDevice can change PresentParam.
        out.PresentParam = m_CreationParam.PresentParam;

        hr = (*p)->Create(m_ConfigParam, m_CreationParam, out);
        if (SUCCEEDED(hr))
        {
            // copy the final result and store.
            m_CreationParam.PresentParam = out.PresentParam;
            m_CreationParam.uD3D9OverlayCaps = out.uD3D9OverlayCaps;
            m_pDevice = out.pDevice;
            m_pDeviceEx = out.pDeviceEx;
            if (out.pDevice)
                out.pDevice->Release();
            if (out.pDeviceEx)
                out.pDeviceEx->Release();
            break;
        }
        ASSERT(!out.pDevice);
        ASSERT(!out.pDeviceEx);
    }

    return hr;
}

HRESULT CD3D9Helper::Reset()
{
    HRESULT hr = E_FAIL;
    CreationOutput out;

    for (const ICreationChainResponder **p = m_ppCreationChain; *p; p++)
    {
        ZeroMemory(&out, sizeof(out));
        // make a copy because CreateDevice can change PresentParam.
        out.PresentParam = m_CreationParam.PresentParam;

        hr = (*p)->Reset(m_ConfigParam, m_CreationParam, out);
        if (SUCCEEDED(hr))
        {
            // copy the final result and store.
            m_CreationParam.PresentParam = out.PresentParam;
            m_CreationParam.uD3D9OverlayCaps = out.uD3D9OverlayCaps;
            break;
        }
    }

    return hr;
}

HRESULT CD3D9Helper::CreateD3D9()
{
    ASSERT(!m_pD3D9Ex);
    ASSERT(!m_pD3D9);
    HRESULT hr = E_FAIL;

    // m_CreationParam gets cleared in ReleaseDevice()
    m_CreationParam.dwOSVersion = GetRegistry(REG_OS_VERSION, 0);

    if (m_ConfigParam.bUseD3D9Ex && CDynLibManager::GetInstance()->pfnDirect3DCreate9Ex)
    {
        // IsNVCoprocEnabledForApp must be called before D3D9Ex is created.
        // IsNVCoprocDevicePresent should be relative light process since it only enumerates non-primary display.
        // On most of systems, it should take no time.
        if (m_CreationParam.dwOSVersion >= OS_WIN7 && IsNVCoprocDevicePresent(m_CreationParam.dwCoprocDeviceId))
            m_CreationParam.bIsCoprocEnabledForApp = IsNVCoprocEnabledForApp();

        hr = CDynLibManager::GetInstance()->pfnDirect3DCreate9Ex(D3D_SDK_VERSION, &m_pD3D9Ex);
    }

	if (m_pD3D9 = Direct3DCreate9(D3D_SDK_VERSION))
        hr = S_OK;

    return hr;
}

HRESULT CD3D9Helper::GetAdapterIdentifier()
{
    IDirect3D9 *pD3D9 = m_pD3D9Ex.p ? m_pD3D9Ex.p : m_pD3D9.p;
    const UINT uAdapterCount = pD3D9->GetAdapterCount();

    CreationParam &cp = m_CreationParam;
    cp.DeviceType = D3DDEVTYPE_HAL;
    cp.uAdapter = D3DADAPTER_DEFAULT;
	m_hMonitor = MonitorFromWindow(m_hwnVideoWindow, (m_hwnVideoWindow) ? MONITOR_DEFAULTTONEAREST : MONITOR_DEFAULTTOPRIMARY);
    ZeroMemory(&cp.MonitorInfoEx, sizeof(cp.MonitorInfoEx));
    cp.MonitorInfoEx.cbSize = sizeof(cp.MonitorInfoEx);
    GetMonitorInfo(m_hMonitor, &cp.MonitorInfoEx);

    for (int i = 0; i < sizeof(s_pInitializersByD3D9Helper) / sizeof(REGISTRY_TYPE); i++)
        ClearRegistry(s_pInitializersByD3D9Helper[i]);

    for (UINT Adapter = 0; Adapter < uAdapterCount; Adapter++)
	{
		HMONITOR hmAdapter = pD3D9->GetAdapterMonitor(Adapter);

        if (hmAdapter == m_hMonitor)
        {
			cp.uAdapter = Adapter;
            pD3D9->GetAdapterIdentifier(Adapter, 0, &cp.AdapterIdentifier);
            DbgMsg("Adapter: %s attached!", cp.AdapterIdentifier.Description);

        	pD3D9->GetDeviceCaps(Adapter, cp.DeviceType, &cp.Caps);
        	pD3D9->GetAdapterDisplayMode(Adapter, &cp.DisplayMode);
            if (m_pD3D9Ex)
            {
                cp.DisplayModeEx.Size = sizeof(cp.DisplayModeEx);
                m_pD3D9Ex->GetAdapterDisplayModeEx(Adapter, &cp.DisplayModeEx, &cp.Rotation);
            }
            return S_OK;
        }
	}

    return E_FAIL;
}

HRESULT CD3D9Helper::InitCreationParam()
{
    CreationParam &cp = m_CreationParam;

    cp.pD3D9 = m_pD3D9.p;           // weak reference
    cp.pD3D9Ex = m_pD3D9Ex.p;       // weak reference
    cp.pDevice = m_pDevice.p;       // weak reference
    cp.pDeviceEx = m_pDeviceEx.p;   // weak reference

    // set device behavior flags
    cp.dwBehaviorFlag = D3DCREATE_MULTITHREADED;
    if (m_ConfigParam.bPreserveFPU)
		cp.dwBehaviorFlag |= D3DCREATE_FPU_PRESERVE;

    if (cp.Caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
		cp.dwBehaviorFlag |= D3DCREATE_HARDWARE_VERTEXPROCESSING;
	else
		cp.dwBehaviorFlag |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;

    if (cp.pD3D9Ex)
        cp.dwBehaviorFlag |= D3DCREATE_DISABLE_DRIVER_MANAGEMENT_EX;

    cp.PresentParam.hDeviceWindow = m_hwnVideoWindow;
    cp.PresentParam.BackBufferWidth = cp.DisplayMode.Width;
    cp.PresentParam.BackBufferHeight = cp.DisplayMode.Height;
    cp.PresentParam.BackBufferCount = BACKBUFFER_COUNT_DEFAULT;
    cp.PresentParam.SwapEffect = D3DSWAPEFFECT_DISCARD;

	m_pHDMIHelper->UpdateVideoWindow(m_hwnVideoWindow);

    //Query HDMI 1.4 Stereo Mode before creating Device.
    if (m_ConfigParam.bUseHDMIStereoMode)
    {
        DriverExtHDMIStereoModeCap StereoMode = {0};

        StereoMode.uWidth = cp.PresentParam.BackBufferWidth;
        StereoMode.uHeight = cp.PresentParam.BackBufferHeight;
        StereoMode.uRefreshRate = cp.DisplayMode.RefreshRate;

        if (!m_pHDMIHelper->IsHDMIStereoModeEnabled())
			m_pHDMIHelper->SetDefaultDisplayMode(&StereoMode);

		if (SUCCEEDED(m_pHDMIHelper->GetAppropriateHDMIDisplayMode(&StereoMode)))
        {
            DbgMsg("Dump default Display Mode before update to HDMI mode: Width = %d, Height = %d, RefreshRate = %d",
                cp.PresentParam.BackBufferWidth, cp.PresentParam.BackBufferHeight, cp.DisplayModeEx.RefreshRate);
            cp.DisplayModeEx.Width = cp.PresentParam.BackBufferWidth = StereoMode.uWidth;
            cp.DisplayModeEx.Height = cp.PresentParam.BackBufferHeight = StereoMode.uHeight;
            cp.DisplayModeEx.RefreshRate = StereoMode.uRefreshRate;
            DbgMsg("Dump seletect HDMI Display Mode: Width = %d, Height = %d, RefreshRate = %d",
                cp.PresentParam.BackBufferWidth, cp.PresentParam.BackBufferHeight, cp.DisplayModeEx.RefreshRate);
        }
    }

    cp.PresentParam.Windowed = TRUE;
    cp.PresentParam.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;

    if (m_ConfigParam.bUseStencilDepthBuffer)
    {
        cp.PresentParam.EnableAutoDepthStencil = TRUE;
        cp.PresentParam.AutoDepthStencilFormat = D3DFMT_D24S8;
    }

    if (m_ConfigParam.bWaitForVsync)
    {
        cp.PresentParam.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
    }
    else
    {
        // Workaround for Intel Cantiga/Eaglelake system for present time is too large with full screen when DWM is off.
	    if (PCI_VENDOR_ID_INTEL == cp.AdapterIdentifier.VendorId && !m_ConfigParam.bUseExclusiveMode)
		    cp.PresentParam.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
        else
            cp.PresentParam.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
    }

    cp.PresentParam.Flags = D3DPRESENTFLAG_DEVICECLIP;

    // S3 relies on D3DPRESENTFLAG_VIDEO to create overlay render-target
    if (PCI_VENDOR_ID_S3 == cp.AdapterIdentifier.VendorId)
		cp.PresentParam.Flags |= D3DPRESENTFLAG_VIDEO;

    if (cp.pD3D9Ex && m_ConfigParam.bUseProtectedOutput && !cp.bIsCoprocEnabledForApp)
        cp.PresentParam.Flags |= D3DPRESENTFLAG_RESTRICTED_CONTENT;
    //| D3DPRESENTFLAG_RESTRICT_SHARED_RESOURCE_DRIVER; (We have to add this flag on FlipEx mode with NV ASIC according to the comment from NV

    CComPtr<IDispSvrDriverExtension> pDrvExt;

    HRESULT hr = CResourceManager::GetInstance()->GetInterface(DISPSVR_RESOURCE_DRIVEREXTENSION, __uuidof(IDispSvrDriverExtension), (void **)&pDrvExt);
    if (SUCCEEDED(hr))
    {
		DrvExtAdapterInfo AdapterInfo = {0};
		hr = pDrvExt->QueryAdapterInfo(m_hwnVideoWindow, m_hMonitor, &AdapterInfo);
		if (SUCCEEDED(hr) && AdapterInfo.bIsXVYCCSupported)
			cp.bIsXvYCCMonitor = true;
    }

    return S_OK;
}

HRESULT CD3D9Helper::SetupResponderChain()
{
    CreationParam &cp = m_CreationParam;

    if (m_pD3D9Ex)
    {
        if (m_ConfigParam.bUseExclusiveMode)
        {
            m_ppCreationChain = s_D3D9ExExclusiveCreationChain;
        }
        else
        {
            DWORD dwVendorId = cp.AdapterIdentifier.VendorId;

            m_ppCreationChain = s_D3D9ExCreationChain;

            // When coproc is enabled, we should use nvidia path regardless current vendor id.
            // It is still up to the driver to decide whether to enable DGPU or not by the rule:
            // if D3D9Overlay == true:
            //     DGPU = true
            // else:
            //     if IGPU has VP: # Arrandale/Ironlake/later with IGPU VP
            //         if (codec == H264 or codec == VC1):
            //             DGPU = true
            //         else:
            //             DGPU = false
            //     else:
            //         DGPU = true
            if (cp.bIsCoprocEnabledForApp)
                dwVendorId = PCI_VENDOR_ID_NVIDIA;

            if (cp.dwOSVersion >= OS_WIN7)
            {
                switch (dwVendorId)
                {
                case PCI_VENDOR_ID_INTEL:
                    if (cp.bIsXvYCCMonitor)
                        m_ppCreationChain = s_D3D9ExYuy2OverlayCreationChain;
                    else if (m_ConfigParam.bUseProtectedOutput && UseIntelD3D9Overlay(cp.AdapterIdentifier.DeviceId))
                        m_ppCreationChain = s_D3D9ExOverlayCreationChain;
                    break;

                case PCI_VENDOR_ID_ATI:
                    {
                        BOOL bIsDWMEnabled = FALSE;

                        if (CDynLibManager::GetInstance()->pfnDwmIsCompositionEnabled)
                            CDynLibManager::GetInstance()->pfnDwmIsCompositionEnabled(&bIsDWMEnabled);

                        if (bIsDWMEnabled && m_ConfigParam.bUseProtectedOutput)
                        {
                            if (!m_pFlipExWindow)
                                m_pFlipExWindow = new CFlipExWindow(cp.PresentParam.hDeviceWindow);
                            else
                                m_pFlipExWindow->UpdateWindow(cp.PresentParam.hDeviceWindow);

                            cp.PresentParam.hDeviceWindow = m_pFlipExWindow->GetHWND();
                            m_ppCreationChain = s_D3D9ExFlipExCreationChain;
                        }
                        else
                        {
                            if(m_pFlipExWindow)
                                m_pFlipExWindow->HideWindow();
                        }
                    }
                    break;

                case PCI_VENDOR_ID_NVIDIA:
                    if (m_ConfigParam.bUseProtectedOutput && !cp.bIsXvYCCMonitor)
                        m_ppCreationChain = s_D3D9ExOverlayCreationChain;
                    // we should enable flipex on all platforms for better performance.
                    else if (0)
                        m_ppCreationChain = s_D3D9ExFlipExCreationChain;
                    break;

                case PCI_VENDOR_ID_S3:
                default:
                    break;
                }
            }
        }
    }
    else
    {
        m_ppCreationChain = s_D3D9CreationChain;
    }

    return S_OK;
}

HRESULT CD3D9Helper::InitDevice()
{
    HRESULT hr;

    if (m_pDeviceEx)
    {
        ASSERT(!m_pDevice);
        m_pDeviceEx.QueryInterface(&m_pDevice);
    }
    ASSERT(m_pDevice);

    // get display mode since it could be changed after using exclusive mode.
    if (m_pDevice)
        hr = m_pDevice->GetDisplayMode(0, &m_CreationParam.DisplayMode);
    if (m_pDeviceEx)
        hr = m_pDeviceEx->GetDisplayModeEx(0, &m_CreationParam.DisplayModeEx, &m_CreationParam.Rotation);

	hr = m_pDevice->SetRenderState(D3DRS_AMBIENT, D3DCOLOR_XRGB(255, 255, 255));
	hr = m_pDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	hr = m_pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	hr = m_pDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
	hr = m_pDevice->SetRenderState(D3DRS_DITHERENABLE, TRUE);
	hr = m_pDevice->SetRenderState(D3DRS_STENCILENABLE, FALSE);
	hr = m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	hr = m_pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	hr = m_pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	hr = m_pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	hr = m_pDevice->SetRenderState(D3DRS_ALPHAREF, 0x10);
	hr = m_pDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);

	hr = m_pDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_BORDER);
	hr = m_pDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_BORDER);
	hr = m_pDevice->SetSamplerState(0, D3DSAMP_BORDERCOLOR, 0x00000000);
	hr = m_pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	hr = m_pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);

    if (CDynLibManager::GetInstance()->pfnGetNumberOfPhysicalMonitorsFromHMONITOR)
	    CDynLibManager::GetInstance()->pfnGetNumberOfPhysicalMonitorsFromHMONITOR(m_hMonitor, &m_dwNumberOfMonitor);

    return hr;
}

HRESULT CD3D9Helper::CheckDeviceState()
{
	HRESULT hr = D3DERR_DEVICELOST;

	if (m_pDeviceEx)
	{
		// We do not consider occlusion here so that NULL is passed to CheckDeviceState.
		// If a window handle is passed in, CheckDeviceState can report occlusion but not mode switch.
		hr = m_pDeviceEx->CheckDeviceState(m_hwnVideoWindow);
	}
	else if (m_pDevice)
	{
        hr = m_pDevice->TestCooperativeLevel();
	}
	return hr;
}

HRESULT CD3D9Helper::CheckModeSwitch()
{
    HRESULT hr = S_OK;

    if (!IsD3D9Ex())
        return hr;

	D3DDISPLAYMODEEX DispModeEx = {0};
	D3DDISPLAYROTATION Rotation = D3DDISPLAYROTATION_IDENTITY;
	DispModeEx.Size = sizeof(D3DDISPLAYMODEEX);
	HMONITOR hMonitor = MonitorFromWindow(m_hwnVideoWindow, (m_hwnVideoWindow) ? MONITOR_DEFAULTTONEAREST : MONITOR_DEFAULTTOPRIMARY);

	DWORD dwNumberOfMonitor = 0;
	if (CDynLibManager::GetInstance()->pfnGetNumberOfPhysicalMonitorsFromHMONITOR)
		CDynLibManager::GetInstance()->pfnGetNumberOfPhysicalMonitorsFromHMONITOR(hMonitor, &dwNumberOfMonitor);

    hr = m_pD3D9Ex->GetAdapterDisplayModeEx(m_CreationParam.uAdapter, &DispModeEx, &Rotation);
    if (SUCCEEDED(hr))
    {
        // five situations should set present mode changed:
        // a. color depth changed, 
        // b. current display mode is lower than new resolution
        // c. Monitor handle has changed 
		// d. Display rotation has changed
		// e. Number of physical monitor has changed (ex. from single mode to clone mode)
        if (m_CreationParam.DisplayModeEx.Format != DispModeEx.Format
            || m_CreationParam.DisplayModeEx.Width != DispModeEx.Width
            || m_CreationParam.DisplayModeEx.Height != DispModeEx.Height
            || m_hMonitor != hMonitor
            || m_CreationParam.Rotation != Rotation
		    || m_dwNumberOfMonitor != dwNumberOfMonitor)
        {
            hr = S_PRESENT_MODE_CHANGED;
        }
    }
    return hr;
}

HRESULT CD3D9Helper::SetDisplayWindow(HWND hwnd, RECT *pRect)
{
	if(m_pFlipExWindow)
		m_pFlipExWindow->UpdatePosition(*pRect);
    return S_OK;
}

HRESULT CD3D9Helper::GetBackBufferSize(UINT* BackBufferWidth, UINT* BackBufferHeight)
{
    *BackBufferWidth = m_CreationParam.PresentParam.BackBufferWidth;
    *BackBufferHeight = m_CreationParam.PresentParam.BackBufferHeight;
    return S_OK;
}

HRESULT CD3D9Helper::GetBackgroundColor(COLORREF* pColor)
{
	*pColor = RGB(GetRValue_D3DCOLOR_XRGB(m_d3dBackgroundColor), GetGValue_D3DCOLOR_XRGB(m_d3dBackgroundColor), GetBValue_D3DCOLOR_XRGB(m_d3dBackgroundColor));
	return S_OK;
}

HRESULT CD3D9Helper::SetBackgroundColor(COLORREF Color)
{
	m_d3dBackgroundColor = D3DCOLOR_XRGB(GetRValue(Color), GetGValue(Color), GetBValue(Color));
	return S_OK;
}

HRESULT CD3D9Helper::Clear()
{
    DWORD dwFlags = m_CreationParam.PresentParam.EnableAutoDepthStencil ? D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL : D3DCLEAR_TARGET;

    return m_pDevice->Clear(0L,	    // no rects (clear all)
        NULL,					// clear entire viewport
        dwFlags,		        // clear render target
        m_d3dBackgroundColor,
        1.0f,					// z buffer depth
        0L);					// no stencil
}

HRESULT CD3D9Helper::SetRenderStates(D3D9HELPER_RENDERSTATE_TYPE e)
{
    ASSERT(m_pDevice);
    switch (e)
    {
    case D3D9HELPER_RENDERSTATE_NO_WINDOW_OUTPUT:
        {
            D3DXMATRIX idMatrix;
            D3DXMatrixIdentity(&idMatrix);
            m_pDevice->SetTransform(D3DTS_WORLD, &idMatrix);
            m_pDevice->SetTransform(D3DTS_VIEW, &idMatrix);
            m_pDevice->SetTransform(D3DTS_PROJECTION, &idMatrix);
            m_pDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
        }
        break;
    case D3D9HELPER_RENDERSTATE_RTV:
    case D3D9HELPER_RENDERSTATE_RTV_FROM_ORIGIN:
        m_pDevice->SetTransform(D3DTS_WORLD, &m_matrixWorld);
        break;
    }
    return S_OK;
}

HRESULT CD3D9Helper::ResetRenderStates(D3D9HELPER_RENDERSTATE_TYPE e)
{
    ASSERT(m_pDevice);
    switch (e)
    {
    case D3D9HELPER_RENDERSTATE_RTV:
    case D3D9HELPER_RENDERSTATE_RTV_FROM_ORIGIN:
        D3DXMATRIX idMatrix;
        D3DXMatrixIdentity(&idMatrix);
        m_pDevice->SetTransform(D3DTS_WORLD, &idMatrix);
        break;
    }
    return S_OK;
}

HRESULT CD3D9Helper::InitRenderStates(D3D9HELPER_RENDERSTATE_TYPE e, RECT &rcSrc)
{
    switch (e)
    {
    case D3D9HELPER_RENDERSTATE_RTV_FROM_ORIGIN:
        if (rcSrc.left > 0)
        {
	        rcSrc.right -= rcSrc.left;
	        rcSrc.left = 0;
        }

        if (rcSrc.top > 0)
        {
	        rcSrc.bottom -= rcSrc.top;
	        rcSrc.top = 0;
        }
        // pass through

    case D3D9HELPER_RENDERSTATE_RTV:
        {
	        // Produce the transformation matrix to transform vertices to window position
	        // Texture coordinates are measured from the center of the texel, while vertex
	        // coordinates are absolute.
	        // We should subtract all vertices by 0.5 to show texel in 2D correctly.
	        // For the sake of later vertex calculation, we offset the 0.5 in the translation.
	        //
	        // The simplified matrix is derived from:
	        //
	        // D3DXMatrixTranslation(m1, 1, -1, 0);
	        // D3DXMatrixScaling(m2,
	        //   FLOAT(rcSrc.right - rcSrc.left) / sizeRt.cx
	        //   FLOAT(rcSrc.bottom - rcSrc.top) / sizeRt.cy,
	        //   0);
	        // D3DXMatrixTranslation(m3,
	        //   FLOAT(rcSrc.left - 0.5) * 2 / sizeRt.cx - 1,
	        //   -FLOAT(rcSrc.top - 0.5) * 2 / sizeRt.cy + 1,
	        //   0);
	        // matrix = m1 * m2 * m3;
	        D3DXMatrixIdentity(&m_matrixWorld);
	        m_matrixWorld(0, 0) = FLOAT(rcSrc.right - rcSrc.left) / m_CreationParam.PresentParam.BackBufferWidth;
	        m_matrixWorld(1, 1) = FLOAT(rcSrc.bottom - rcSrc.top) / m_CreationParam.PresentParam.BackBufferHeight;
	        m_matrixWorld(3, 0) = m_matrixWorld(0, 0) + FLOAT(rcSrc.left - 0.5) * 2 / m_CreationParam.PresentParam.BackBufferWidth - 1;
	        m_matrixWorld(3, 1) = -m_matrixWorld(1, 1) - FLOAT(rcSrc.top - 0.5) * 2 / m_CreationParam.PresentParam.BackBufferHeight + 1;
        }
        break;

    default:
	    rcSrc.left = rcSrc.top = 0;
        rcSrc.right = m_CreationParam.PresentParam.BackBufferWidth;
	    rcSrc.bottom = m_CreationParam.PresentParam.BackBufferHeight;
        break;
    }

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
/// CFlipExWindow
CFlipExWindow::CFlipExWindow(HWND hwnd)
{
    RECT videoRect = {0};

	::GetWindowRect(hwnd, &videoRect);

    m_hwnWindow = CreateWindowEx(
	    GetWindowLong(hwnd, GWL_EXSTYLE),
	    TEXT("WinDVD"),
	    TEXT("FlipEX Window"),
	    GetWindowLong(hwnd, GWL_STYLE),
	    0,
	    0,
	    videoRect.right - videoRect.left,
	    videoRect.bottom - videoRect.top,
	    hwnd,					            // Handle of parent
	    NULL,								// Handle to menu
	    (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE),  // Application instance
	    NULL
    );

	// Set FlipEX window as bottom of the z-order, except its parent (m_hwnVideoWindow).
	::SetWindowPos(m_hwnWindow, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
}

CFlipExWindow::~CFlipExWindow()
{
    ::CloseWindow(m_hwnWindow);
}

void CFlipExWindow::UpdatePosition(const RECT &rect)
{
    ::MoveWindow(m_hwnWindow, 0, 0, rect.right - rect.left, rect.bottom - rect.top, FALSE);
}

void CFlipExWindow::UpdateWindow(HWND hwnd)
{
    RECT videoRect = {0};

	::GetWindowRect(hwnd, &videoRect);

	// update its size with original window size.
	::SetWindowPos(m_hwnWindow, NULL, 0, 0, videoRect.right - videoRect.left, videoRect.bottom - videoRect.top, SWP_NOZORDER|SWP_SHOWWINDOW);
}

void CFlipExWindow::HideWindow()
{
    ::ShowWindow(m_hwnWindow, SW_HIDE);
}
