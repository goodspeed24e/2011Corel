#pragma once

#include "Singleton.h"
#include "DispSvrDebug.h"

/// CWizard is a global singleton whose lifetime is the same as DispSvr.dll
class CWizard :
    public DispSvr::CObjectCounter<CWizard>,
    public CUnknown,
    public IDisplayServer,
    public IDisplayLock,
	public DispSvr::Singleton<CWizard>
{
public:

    CWizard(LPUNKNOWN pUnk, HRESULT *phr);
    virtual ~CWizard();

    // IUnknown implementation
    DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID, void**);

    // IDisplayServer implementation
    STDMETHOD(Initialize)(DWORD dwFlags, HWND hWnd, IDisplayRenderEngine *pRenderEngine);
    STDMETHOD(Terminate)();
	STDMETHOD(BeginDeviceLoss)();
    STDMETHOD(EndDeviceLoss)(IUnknown* pDevice);
    STDMETHOD(GetRenderEngine)(IDisplayRenderEngine** ppRenderEngine);
	STDMETHOD(GetMessageWindow)(HWND* phwnd);
	STDMETHOD(SetMessageWindow)(HWND hwnd);

    // IDisplayLock implementation
    STDMETHOD(Lock)();
    STDMETHOD(Unlock)();
    STDMETHOD(TryLock)();

private:
	HRESULT CreateDebugObject();
	HRESULT ReleaseDebugObject();

private:

	CRITICAL_SECTION m_ObjectLock;
    BOOL m_bInitialized;     // true if Initialize() was called and succeeded
	BOOL m_bExtRenderEng;	// Customized render engine

    CComPtr<IDisplayRenderEngine> m_pRenderEngine;    // render engine
	CDispSvrDebug *m_pDispSvrDebug;
};
