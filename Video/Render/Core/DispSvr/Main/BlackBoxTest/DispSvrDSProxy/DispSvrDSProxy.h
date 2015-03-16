#pragma once

enum RendererMode
{
	enRendererMode_None,
	enRendererMode_VMR9,
	enRendererMode_EVR,
};

class CDispSvrDSProxy : public CBaseFilter, public ISpecifyPropertyPages, public IKsPropertySet
{
public:
	CDispSvrDSProxy(
		const TCHAR *pName,
		LPUNKNOWN pUnk);
	~CDispSvrDSProxy();
	static CUnknown * WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT *pHr);
	static LRESULT CALLBACK VideoWndProc(HWND hwnd , UINT msg , WPARAM wp , LPARAM lp);

public:
	//CUnknown
	DECLARE_IUNKNOWN;
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);

	//IBaseFilter implemenation
	STDMETHOD(JoinFilterGraph)(__inout_opt IFilterGraph * pGraph, __in_opt LPCWSTR pName);

	//CBaseFilter implementation
	virtual int GetPinCount();
	virtual CBasePin* GetPin(int n);

	//ISpecifyPropertyPages Implementation
	STDMETHOD(GetPages)(CAUUID *pPages);

	// IKsPropertySet stuff 
	STDMETHOD(Set)(	REFGUID guidPropSet, 
		DWORD dwPropID, 
		LPVOID pInstanceData,
		DWORD cbInstanceData, 
		LPVOID pPropData, 
		DWORD cbPropData);

	STDMETHOD(Get)(	REFGUID guidPropSet, 
		DWORD dwPropID, 
		LPVOID pInstanceData,
		DWORD cbInstanceData, 
		LPVOID pPropData, 
		DWORD cbPropData,
		DWORD *pcbReturned);
	STDMETHOD(QuerySupported)(REFGUID guidPropSet, DWORD dwPropID, DWORD *pTypeSupport);

public:
	static AMOVIESETUP_FILTER m_FilterInfo;

public:
	HRESULT TeminateDispSvr();
	HRESULT DetachDispSvr();
	HRESULT RemoveRendererFilter();
	HRESULT GetRenderEngine(IDisplayRenderEngine **pEngine);
protected:
	HRESULT InitDispSvr();
	HRESULT InitMSCustomPresenter();
	HRESULT ChildObjectControl(IUnknown *pUnk, BOOL bInsert = FALSE, ULONG lZOrder = NULL);
	HRESULT InitVideoDialog();
	HRESULT InsertRendererFilter();
	HRESULT AttachDispSvr();
	HRESULT ConfigVideoRenderer();
	HRESULT ProcessDeviceLost();
//	HRESULT RefreshGraphEdit();

protected:
	HWND m_hGraphEdit;
	CCritSec m_CritSec;
	IDisplayServer *m_pDispSvr;
	IDisplayRenderEngine *m_pRenderEngine;
	IDisplayVideoSource *m_pVidSrc;
	IDisplayVideoMixer *m_pDispVidMixer;
	IBaseFilter *m_pVideoRenderer;
	HWND m_hVideo;
	DWORD m_dwRendererMode;
	DWORD m_dwDispSvrInitFlags;
    BOOL m_bForceVMR9;

protected:
	IMFVideoPresenter *m_pCustomPresenter;
};