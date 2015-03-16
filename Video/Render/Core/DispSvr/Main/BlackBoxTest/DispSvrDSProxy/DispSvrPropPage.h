#pragma once

class CDispSvrPropPage : public CBasePropertyPage
{
public:
	CDispSvrPropPage(IUnknown *pUnk);
	~CDispSvrPropPage();

	static CUnknown * WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT *pHr);
	virtual HRESULT OnActivate();
	virtual HRESULT OnConnect(IUnknown *pUnknown);
    virtual INT_PTR OnReceiveMessage(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);

protected:
	//WM Message
	//BOOL OnInitDialog();
protected:
	IKsPropertySet *m_pKsSet;
	DWORD m_DispSvrInitFlags;
};