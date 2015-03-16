#pragma once

#include "D3D9Helper.h"

class CHijackDetect;
struct DisplayDeviceEnv;
interface IDispSvrVideoPresenter;
interface IDisplayRenderEngineProperty;


/**
* Upon initialization, this object creates a separate thread for rendering scene asynchronously.
*/
class CRenderEngine :
            public DispSvr::CObjectCounter<CRenderEngine>,
            public CUnknown,
            public IDisplayRenderEngine,
			public IDisplayRenderEngineProperty,
			public IDisplayEventHost
{
public:
    CRenderEngine(LPUNKNOWN pUnk, HRESULT *phr);
    virtual ~CRenderEngine();

    // IUnknown implementation
    DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID, void**);

    // IDisplayRenderEngine implementation
    STDMETHOD(Initialize)(HWND hWnd, UINT BackBufferWidth, UINT BackBufferHeight, IDisplayLock* pLock, DWORD dwFlags);
    STDMETHOD(Terminate)();
    STDMETHOD(SetBackBufferSize)(UINT BackBufferWidth, UINT BackBufferHeight);
    STDMETHOD(GetBackBufferSize)(UINT* BackBufferWidth, UINT* BackBufferHeight);
    STDMETHOD(Render)();
    STDMETHOD(UpdateMainVideoTime)(void *pObject);
    STDMETHOD(ProcessLostDevice)();
    STDMETHOD(GetRootObject)(REFIID riid, void** ppvObject);
    STDMETHOD(SetRootObject)(IDisplayObject* pObject);
    STDMETHOD(SetFrameRate)(int nFramesPerSecBy100);
    STDMETHOD(GetFrameRate)(int* pnFramesPerSecBy100);
    STDMETHOD(GetFrameRateAvg)(int* pnFramesPerSecBy100);
    STDMETHOD(GetMixingPrefs)(DWORD* pdwPrefs);
    STDMETHOD(SetDisplayServer)(IDisplayServer* pDisplayServer);
    STDMETHOD(GetDisplayServer)(IDisplayServer** ppDisplayServer);
    STDMETHOD(Get3DDevice)(IUnknown** ppDevice);
    STDMETHOD(Set3DDevice)(IUnknown* pDevice);
    STDMETHOD(GetDisplayWindow)(HWND *phwnd, RECT *pRect);
    STDMETHOD(SetDisplayWindow)(HWND hwnd, RECT *pRect);
    STDMETHOD(GetBackgroundColor)(COLORREF* pColor);
    STDMETHOD(SetBackgroundColor)(COLORREF Color);
    STDMETHOD(GetLock)(IDisplayLock** ppLock);
    STDMETHOD(EnableRendering)(BOOL bEnableRendering);
    STDMETHOD(StartRenderingThread)();
    STDMETHOD(StopRenderingThread)();
    STDMETHOD(NodeRequest)(DWORD request, DWORD param1, DWORD param2, IDisplayObject* pObject);
    STDMETHOD(SetColorKey)(DWORD dwColorKey);
    STDMETHOD(GetColorKey)(DWORD* pdwColorKey);
    STDMETHOD(GetMessageWindow)(HWND* phwnd);
    STDMETHOD(SetMessageWindow)(HWND hwnd);
    STDMETHOD(GetConfigFlags)(DWORD *pFlags);
    STDMETHOD(GetRecomFPS)(DWORD *pdwRecomFPS);
    STDMETHOD(GetRecomVideoPixelArea)(DWORD* pdwRecomVideoPixelArea);
    STDMETHOD(AdviseEventNotify)(IDispSvrRenderEngineNotify *pEventNotify);
    STDMETHOD(UnAdviseEventNotify)(IDispSvrRenderEngineNotify *pEventNotify);
    STDMETHOD(ProcessWindowMessage)(UINT uMsg, WPARAM wParam, LPARAM lParam);
    STDMETHOD(ProcessRequest)(DWORD request, DWORD param1, DWORD param2);

    // IDisplayEventHost implementation
    STDMETHOD(Register)(IDisplayEventHandler* pHandler, LPVOID pInstance);
    STDMETHOD(Unregister)(IDisplayEventHandler* pHandler);

	// IDipslayRenderEngineProperty
	STDMETHOD(SetProperty)(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData);
	STDMETHOD(GetProperty)(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData, DWORD *pcbReturned);
	STDMETHOD(QueryPropertySupported)(REFGUID guidPropSet, DWORD dwPropID, DWORD *pTypeSupport);

private:
    enum RenderThreadStatus
    {
        eNotStarted = 0x0,
        eRunning,
        eWaitingToStop,
        eFinished,
    };

    enum DeviceStatus
    {
        eDeviceReady = 0x0,
        eDeviceChanging,
        eDeviceLost,
        eDeviceRecoverable,
        eDeviceLostDetected,
		eDeviceNotReset,
    };

    struct EventHandler
    {
        IDisplayEventHandler*   pHandler;
        LPVOID                  pInstance;
    };

	struct DeviceChangingInfo
	{
		BOOL bMonitorChanges;							// TRUE when monitor handle has changed. 
		BOOL bVGAChanges;								// TRUE when VGA has changed.
		BOOL bExclusiveWindowMode;					// TRUE when needs to switch between exclusive and window mode
		BOOL bCustomizedOutputChanges;			// TRUE when needs to change customized output.
		BOOL bStereoModeChanges;					// TRUE when needs to switch between 2D and 3D mode.
	};

    typedef std::vector<EventHandler> EventHandlers;
    typedef std::vector<IDispSvrRenderEngineNotify *> RenderEngineEventNotify;

private:
    void NotifyRenderEngineEvent(DispSvrEngineEvent dwEvent, DWORD param1, DWORD param2);
    void NotifyDisplayEvent(DisplayEvent event, DWORD param1, DWORD param2);
    static UINT WINAPI RenderThreadProc(LPVOID lpParameter);
    HRESULT RenderLoop();
    HRESULT CreateD3DDevice();
    HRESULT ReleaseD3DDevice();
    HRESULT ConfigureResource(bool bResetCase = false);
    HRESULT SetPreferredMixerPresenter(GUID &guidMixer, GUID& guidPresenter, bool bResetCase);
    HRESULT RenderScene(DWORD dwTimeOut = 0);
    HRESULT Present();
    HRESULT ResetDevice();
    HRESULT GenerateConfigParamFromEngine(CD3D9Helper::ConfigParam &p);
    HRESULT InitResources(bool bResetCase = false);
    // unordered render/present do not guarantee render and present are paired.  
    HRESULT RenderSceneUnordered();  
    HRESULT PresentUnordered();
    HRESULT RenderNoWindowOutput();
    HRESULT RenderDispObj();
    HRESULT RenderCustomMixing();
    HRESULT SetCustomizedOutput(BOOL bFlag, DWORD dwExtOption);
	HRESULT WaitDeviceChange();
	HRESULT NotifyDWMQueuing(BOOL bEnable);
    HRESULT ProcessDWM(BOOL bEnable);
    HRESULT UpdateHJDetectDesc();
    HRESULT CheckDeviceState();
    HRESULT UpdateDisplayWindow();
	HRESULT ConfigureHDMIStereoBeforeDeviceReset();
	void UpdateOverlayStatus(BOOL bHide, long Status=0);


private:
    RenderThreadStatus m_RenderThreadStatus; // 0: not started, 1: running, 2: requested to stop, 3: stopped
    EventHandlers m_EventHandlers;			/// Registered handlers to receive display event
    RenderEngineEventNotify m_RenderEngineEventNotify; ///send render engine event notify to application.
    HWND m_hwnVideoWindow;

    HANDLE m_hRenderThread;

    CComPtr<IDisplayLock> m_pLock;
    CComPtr<IDisplayServer> m_pOwner;       // IDisplayServer that controls this render engine
    CComPtr<IDisplayObject> m_pRootDO;  // The root Display Object
    CComPtr<IDispSvrVideoPresenter> m_pDispSvrVideoPresenter;

    DWORD m_dwConfigFlags;   // configuration flags

    RECT m_rectDisp;
    RECT m_rectSrc;		//< record render target source rectangle
    RECT m_rectWindow;	//< record window rectangle

    IDisplayServerStateEventSink *m_pStateEventSink;
    CHijackDetect* m_pHJDetect;

    BOOL m_bCheckDisplayChange;
    BOOL m_bStereoMode;
    HANDLE m_hRenderPresentSemaphore;   // the semaphore is used to improve render/present call in pair.

    BOOL m_bInitialized; // true if Initialize() was called
    BOOL m_bUseMessageThread;
    BOOL m_bFreezeState;
    BOOL m_bAutoRenderThread; // For pause Auto Render,used in passive mode.
    BOOL m_bOutOfMemory;
    volatile DeviceStatus m_eDeviceStatus; // 0: ready, 1: changing, 2: lost
    volatile BOOL m_bEnableRendering;
    HMONITOR m_hMonitor;
    HRESULT (CRenderEngine::*m_pfnRenderFunc)();
    BOOL m_bHide;		/// if the window is completely occluded.
    DWORD m_dwInterval;
    BOOL m_bForceClearRenderTarget;
    DWORD m_dwClearRenderTargetTimes;
    DWORD m_dwFrameRate;
    DWORD m_dwVideoFormat;
    bool m_bRTVFromOrigin;
    bool m_bIsOverlayPresenter;
	bool m_bIsD3D9RenderTarget;
    bool m_bFirstTimeDisp;
	int  m_nMonPowerStatus;
	bool m_bUseRTV;
	BOOL m_bRestoreMixerStereoEnable;
    DWORD m_dwRestoreMixerStereoMode;
    BOOL m_bIsDWMEnabled;
    class CHDMIStereoModeHelper *m_pHDMIHelper;
    CD3D9Helper *m_pD3D9Helper;
	DeviceChangingInfo m_DeviceInfo;
    BOOL m_bExclusiveMode;
    BOOL m_bEnableHDMIStereoAfterDeviceReset;
    DWORD m_dwColorKey;
    RESOURCE_WINDOW_STATE m_eWindowState;
	BOOL m_bHideOverlay;

    D3D9HELPER_RENDERSTATE_TYPE m_eD3D9HelperRenderStateType;
};
