#pragma once

class CShineDO :
    public DispSvr::CObjectCounter<CShineDO>,
    public CUnknown,
    public IDisplayObject
{
public:
    CShineDO(LPUNKNOWN pUnk, HRESULT *phr);
    virtual ~CShineDO();

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
    STDMETHOD(EndDeviceLoss)(IUnknown* pDevice );
	STDMETHOD(GetCLSID)(CLSID* pClsid);
	STDMETHOD(NotifyEvent)(DWORD event, DWORD param1, DWORD param2, LPVOID pInstance) { return S_OK; }

private:

    HRESULT AllocDeviceResources(IDirect3DDevice9* pDevice);
    HRESULT FreeDeviceResources();

	HRESULT OnMouseMove(int x, int y);

private:
    
    CCritSec m_ObjectLock;
    CComPtr<IDisplayRenderEngine> m_pOwner;
#pragma warning(disable: 4324)  // structure was padded due to __declspec( align())
    CComPtr<IDirect3DVertexBuffer9> m_pVertexBuffer;
    CComPtr<IDirect3DTexture9> m_pShine[6];
    DWORD m_dwLastTextureTime;
    int m_iTexture;
};
