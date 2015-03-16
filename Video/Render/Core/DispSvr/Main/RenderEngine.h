#pragma once

#include <vector>
#include <multimon.h>

class CHijackDetect;
struct DisplayDeviceEnv;
interface IDispSvrVideoPresenter;
interface IDisplayRenderEngineProperty;

/**
  * Upon initialization, this object creates a separate thread for rendering scene asynchronously.
  */
class CRenderEngine :
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
	};

	enum RenderTargetSize
	{
		eVideoResolution = 0x0,
		eScreenResolutionFixed,
		eScreenResolutionMax,
	};

    struct EventHandler
    {
        IDisplayEventHandler*   pHandler;
        LPVOID                  pInstance;
    };

	typedef std::vector<EventHandler> EventHandlers;
	typedef std::vector<IDispSvrRenderEngineNotify *> RenderEngineEventNotify;

private:
	void NotifyRenderEngineEvent(DispSvrEngineEvent dwEvent, DWORD param1, DWORD param2);
	void NotifyDisplayEvent(DisplayEvent event, DWORD param1, DWORD param2);
	static UINT WINAPI RenderThreadProc(LPVOID lpParameter);
	HRESULT RenderLoop();
	HRESULT CreateD3DDevice9(UINT uDeviceCreateFlag, const D3DFORMAT *pFmts, DisplayDeviceEnv *pEnv);
	HRESULT CreateD3DDevice9Ex(UINT uDeviceCreateFlag, const D3DFORMAT *pFmts, DisplayDeviceEnv *pEnv);
	HRESULT CreateD3DDevice();
	HRESULT ReleaseD3DDevice();
	HRESULT ConfigureResource();
	HRESULT SetPreferredMixerPresenter(GUID &guidMixer, GUID& guidPresenter);
	HRESULT RenderScene();
	HRESULT Present();
	HRESULT RenderNoWindowOutput();
	HRESULT RenderDispObj();
	HRESULT RenderCustomMixing();
	HRESULT CheckDeviceState();
	HRESULT CheckModeSwitch();
	HRESULT SetCustomizedOutput(BOOL bFlag);
	HRESULT WaitDeviceChange();
	HRESULT GetAdapterEx(UINT *pAdapter, D3DDEVTYPE *pType, HMONITOR hMon);

	// internal use for other functions
	HRESULT GetRecommendedBackBufferSize(UINT *pWidth, UINT *pHeight);

private:
	RenderThreadStatus m_RenderThreadStatus; // 0: not started, 1: running, 2: requested to stop, 3: stopped
	EventHandlers m_EventHandlers;			/// Registered handlers to receive display event
	RenderEngineEventNotify m_RenderEngineEventNotify; ///send render engine event notify to application.
	HWND m_hwndMessage;  // handle to the message window
	HWND m_hwnVideoWindow;
	SIZE m_sizeBackBuffer;

	HANDLE m_hRenderThread;

	CComPtr<IDisplayLock> m_pLock;
	CComPtr<IDisplayServer> m_pOwner;       // IDisplayServer that controls this render engine
	CComPtr<IDisplayObject> m_pRootDO;  // The root Display Object
	CComPtr<IDirect3DSurface9> m_pRenderTarget;
	CComPtr<IDirect3D9> m_pD3D9;
	CComPtr<IDirect3D9Ex> m_pD3D9Ex;
	CComPtr<IDirect3DDevice9> m_pDevice;
	CComPtr<IDirect3DDevice9Ex> m_pDeviceEx; 
	CComPtr<IDispSvrVideoPresenter> m_pDispSvrVideoPresenter;

	DWORD m_dwConfigFlags;   // configuration flags
	D3DCOLOR m_d3dBackgroundColor;

	RECT m_rectDisp;
	RECT m_rectSrc;		//< record render target source rectangle
	RECT m_rectWindow;	//< record window rectangle
	D3DXMATRIX m_matrixWorld;

	IDisplayServerStateEventSink *m_pStateEventSink;
	CHijackDetect* m_pHJDetect;

	BOOL m_bCheckDisplayChange;
	BOOL m_bExclusiveMode;
	D3DDISPLAYMODE m_d3dDisplayMode; // Display mode for current device.

	BOOL m_bInitialized; // true if Initialize() was called
	BOOL m_bUseMessageThread;
	BOOL m_bFreezeState;
	BOOL m_bAutoRenderThread; // For pause Auto Render,used in passive mode.
	RenderTargetSize m_eRTSize;
	volatile DeviceStatus m_eDeviceStatus; // 0: ready, 1: changing, 2: lost
	volatile BOOL m_bEnableRendering;
	HMONITOR m_hMonitor;
	HRESULT (CRenderEngine::*m_pfnRenderFunc)();
	BOOL m_bICT;
	BOOL m_bHide;		/// if the window is completely occluded.
	DWORD m_dwInterval;
	DWORD m_dwColorFillTimes;
	DWORD m_dwClearTimes;
	BOOL m_bEnableColorFill;
	DWORD m_dwFrameRate;
	DWORD m_dwVideoFormat;
	bool m_bIsD3D9RenderTarget;
	bool m_bUseD3D9Overlay;
	bool m_bUseRTV;
	bool m_bRTVFromOrigin;
	bool m_bIsOverlayPresenter;
    bool m_bFirstTimeDisp;
};
