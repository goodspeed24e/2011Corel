// MainFrm.h : interface of the CMainFrame class
//
#pragma once

#include <list>
#include "tinyxml.h"

class CChildFrame;

class CMainFrame : public CMDIFrameWnd
{
	DECLARE_DYNAMIC(CMainFrame)
public:
	CMainFrame();
	virtual ~CMainFrame();

	/// Save layout/setting to a xml file
	void SaveConfig(const TCHAR* szConfigFileName);

	/// Load layout/setting from a xml file
	void LoadConfig(const TCHAR* szConfigFileName);

	void OnChildDestory(CChildFrame* child)
	{
		m_ChildFrameList.remove(child);
	}

	// Overrides
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	HACCEL m_hMDIAccel;
	HMENU m_hMDIMenu;
	HMENU m_hChartViewMenu;
	HMENU m_hMessageViewMenu;


	// control bar embedded members
	CStatusBar  m_wndStatusBar;
	CToolBar    m_wndToolBar;

	DWORD GetToolBarHeight()
	{
		RECT rect;
		m_wndToolBar.GetWindowRect(&rect);
		return rect.bottom - rect.top;
	}

	// Generated message map functions
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSaveSetting();
	afx_msg void OnLoadSetting();
	afx_msg void OnChartNew();
	afx_msg void OnMessageViewNew();
	DECLARE_MESSAGE_MAP()

	void CloseAllWindows();

	template<typename T>
	CChildFrame* CreateChild(CString strTitle, UINT nResource, HMENU hMenu = NULL, HACCEL hAccel = NULL);
	CChildFrame* CreateChartViewFrame();
	CChildFrame* CreateMessageViewFrame();


	std::list<CChildFrame*> m_ChildFrameList;
	TiXmlDocument m_ConfigXML;
};


