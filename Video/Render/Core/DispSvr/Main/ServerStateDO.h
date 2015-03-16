#pragma once
#include "DispSvr.h"
#include "Imports/LibD3D/d3dfont.h"


class CServerStateDO :
    public CUnknown,
    public IDisplayObject
{
public:
    CServerStateDO(LPUNKNOWN pUnk, HRESULT *phr);
    virtual ~CServerStateDO();

    // IUnknown implementation
    DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID, void**);

    // IDisplayObject implementation
    STDMETHOD(Initialize)(IDisplayRenderEngine* pRenderEngine);
    STDMETHOD(GetRenderEngineOwner)(IDisplayRenderEngine** ppRenderEngine);
    STDMETHOD(Terminate)();
    STDMETHOD(ProcessMessage)(HWND hWnd, UINT msg, UINT wParam, LONG lParam);
    STDMETHOD(Render)(IUnknown *pDevice, const NORMALIZEDRECT* lpParentRect);
    STDMETHOD(BeginDeviceLoss)(void);
    STDMETHOD(EndDeviceLoss)(IUnknown* pDevice);
	STDMETHOD(GetCLSID)(CLSID* pClsid);
	STDMETHOD(NotifyEvent)(DWORD event, DWORD param1, DWORD param2, LPVOID pInstance);

private:

    HRESULT InitFont(IDirect3DDevice9* pDevice);
    HRESULT UninitFont();

private:
    
    CComPtr<IDisplayRenderEngine> m_pOwner;
#pragma warning(disable: 4324)  // structure was padded due to __declspec( align())
    CD3DFont* m_pFont;
    D3DCAPS9 m_d3dCaps;
    D3DSURFACE_DESC m_d3dsdBackBuffer;
    D3DDISPLAYMODE m_d3dDisplayMode;
    CComPtr<IDisplayServerStateEventSink> m_pStateEventSink;
	DWORD m_dwLastUpdate;
	TCHAR m_szDispSvr[256];
	TCHAR m_szRuntime[1024];
    CCritSec m_ObjectLock;
};
