#pragma once

#include "DispSvrDbg.h"

class CDispSvrDebug : 
	public CUnknown,
	IDispSvrDbg
{
public:
	CDispSvrDebug(LPUNKNOWN pUnk, HRESULT *phr);
	virtual ~CDispSvrDebug();

	// IUnknown implementation
	DECLARE_IUNKNOWN
	STDMETHODIMP NonDelegatingQueryInterface(REFIID, void**);

	//IDispSvrDbg Implementation
	STDMETHOD(GetDispSvrVersion)(BSTR *bstrVer);
	STDMETHOD(SetInitFlags)(DWORD dwFlags);
	STDMETHOD(GetInitFlags)(DWORD *dwFlags);
public:
	// ROT related method
	HRESULT AddIntoROT();
	HRESULT RemoveFromROT();
	// In-process control method
	HRESULT SetRenderEngine(IDisplayRenderEngine *pRenderEngine);
protected:
	DWORD m_dwDispSvrDbgRegID;
	DWORD m_InitFlags;

	CComPtr<IDisplayRenderEngine> m_pRenderEngine;    // render engine
};