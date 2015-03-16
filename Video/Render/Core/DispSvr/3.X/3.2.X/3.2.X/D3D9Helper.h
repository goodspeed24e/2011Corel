#ifndef _DISPSVR_D3D9_HELPER_H_
#define _DISPSVR_D3D9_HELPER_H_

class CHDMIStereoModeHelper;
class CFlipExWindow;

typedef enum _D3D9HELPER_RENDERSTATE_TYPE {
    D3D9HELPER_RENDERSTATE_NO_CHANGE,
    D3D9HELPER_RENDERSTATE_NO_WINDOW_OUTPUT,
    D3D9HELPER_RENDERSTATE_RTV,
    D3D9HELPER_RENDERSTATE_RTV_FROM_ORIGIN
} D3D9HELPER_RENDERSTATE_TYPE;

class CD3D9Helper : public DispSvr::IRegistryInitializer
{
public:
    struct ConfigParam
    {
        bool bUseD3D9Ex;            // use D3D9Ex whenever possible
        bool bUseExclusiveMode;
        bool bPreserveFPU;
        bool bUseProtectedOutput;
        bool bUseStencilDepthBuffer;
        bool bWaitForVsync;
        bool bUseHDMIStereoMode;
    };

    struct CreationParam
    {
        DWORD dwOSVersion;
        MONITORINFOEX MonitorInfoEx;
        D3DPRESENT_PARAMETERS PresentParam;
        D3DDISPLAYROTATION DisplayRotation;
        D3DDISPLAYMODE DisplayMode;
        D3DDISPLAYMODEEX DisplayModeEx;
        D3DDISPLAYROTATION Rotation;
        D3DDEVTYPE DeviceType;
        DWORD dwBehaviorFlag;
        D3DADAPTER_IDENTIFIER9 AdapterIdentifier;
        D3DCAPS9 Caps;
        UINT uAdapter;
        UINT uD3D9OverlayCaps;
        IDirect3D9 *pD3D9;
        IDirect3D9Ex *pD3D9Ex;
        IDirect3DDevice9 *pDevice;
        IDirect3DDevice9Ex *pDeviceEx;
        bool bIsXvYCCMonitor;
        bool bIsCoprocEnabledForApp;        //< coproc driver maintains a white list, we need to know if driver enables app or not
        DWORD dwCoprocDeviceId;
    };

    struct CreationOutput
    {
        IDirect3DDevice9 *pDevice;
        IDirect3DDevice9Ex *pDeviceEx;
        D3DPRESENT_PARAMETERS PresentParam;
        UINT uD3D9OverlayCaps;
    };

    interface ICreationChainResponder
    {
        virtual HRESULT Create(const ConfigParam &c, const CreationParam &p, CreationOutput &output) const = 0;
        virtual HRESULT Reset(const ConfigParam &c, const CreationParam &p, CreationOutput &output) const = 0;
        virtual HRESULT CustomParam(const ConfigParam &c, const CreationParam &p, CreationOutput &output) const = 0;
    };

public:
    CD3D9Helper();
    virtual ~CD3D9Helper();

    // IRegistryInitializer
    virtual HRESULT GetRegistryValue(DispSvr::REGISTRY_TYPE eType, DispSvr::registry_t &dwValue);

    HRESULT CreateDevice(HWND hwnd, const ConfigParam &p);
    HRESULT ResetDevice(const ConfigParam &p);
    HRESULT ReleaseDevice();
    HRESULT CheckDeviceState();
    HRESULT CheckModeSwitch();
    HRESULT GetAdapterEx(UINT *pAdapter, D3DDEVTYPE *pType, HMONITOR hMon);

    HRESULT GetBackBufferSize(UINT* BackBufferWidth, UINT* BackBufferHeight);
    HRESULT SetBackgroundColor(COLORREF Color);
    HRESULT GetBackgroundColor(COLORREF* pColor);
    HRESULT GetPresentParam(D3DPRESENT_PARAMETERS &pp);

    IUnknown *GetDevice() { return m_pDevice; }
    HRESULT SetDisplayWindow(HWND hwnd, RECT *pRect);
    HRESULT SetDevice(IUnknown *p)  { return E_NOTIMPL; }
    HRESULT Clear();
    HRESULT SetRenderStates(D3D9HELPER_RENDERSTATE_TYPE);
    HRESULT ResetRenderStates(D3D9HELPER_RENDERSTATE_TYPE);
    HRESULT InitRenderStates(D3D9HELPER_RENDERSTATE_TYPE, RECT &rcSrc);

    bool IsD3D9Ex() const { return !!m_pD3D9Ex; }
    bool IsExclusiveMode() const { return !m_CreationParam.PresentParam.Windowed; }
    bool IsD3D9Overlay() const { return m_CreationParam.PresentParam.SwapEffect == D3DSWAPEFFECT_OVERLAY; }
    bool IsFlipEx() const { return m_CreationParam.PresentParam.SwapEffect == D3DSWAPEFFECT_FLIPEX; }

protected:
    HRESULT Create();
    HRESULT Reset();
    HRESULT CreateD3D9();
    HRESULT GetAdapterIdentifier();
    HRESULT InitCreationParam();
    HRESULT SetupResponderChain();
    HRESULT InitDevice();

protected:
    CComPtr<IDirect3D9> m_pD3D9;
    CComPtr<IDirect3D9Ex> m_pD3D9Ex;
    CComPtr<IDirect3DDevice9> m_pDevice;
    CComPtr<IDirect3DDevice9Ex> m_pDeviceEx;

    CreationParam m_CreationParam;
    ConfigParam m_ConfigParam;

    D3DCOLOR m_d3dBackgroundColor;
    D3DXMATRIX m_matrixWorld;
    HMONITOR m_hMonitor;
    HWND m_hwnVideoWindow;
    DWORD m_dwNumberOfMonitor;

    CHDMIStereoModeHelper *m_pHDMIHelper;
    CFlipExWindow *m_pFlipExWindow;

    const ICreationChainResponder **m_ppCreationChain;
    static const DispSvr::REGISTRY_TYPE s_pInitializersByD3D9Helper[];
};

/// http://msdn.microsoft.com/en-us/library/ee890072(VS.85).aspx#Design_Guide_D3D_Flip_Mode
/// Applications should use Direct3D 9Ex Flip Mode Present in an HWND that is not also targeted by other APIs,
/// including Blt Mode Present Direct3D 9Ex, other versions of Direct3D, or GDI. Flip Mode Present can be used
/// to present to child windows; that is, applications can use Flip Model when it is not mixed with Blt Model in
/// the same HWND, as shown in the following illustrations.
class CFlipExWindow
{
public:
    explicit CFlipExWindow(HWND hwnd);
    ~CFlipExWindow();

    void UpdatePosition(const RECT &rect);
    void UpdateWindow(HWND hWnd);
    void HideWindow();
    HWND GetHWND() const { return m_hwnWindow; }

protected:
    HWND m_hwnWindow;
};

#endif  // _DISPSVR_D3D9_HELPER_H_
