// MainFrm.h : interface of the CMainFrame class
//


#pragma once
#include "HVDTestCenter.h"
#include "CtrlPanel.h"
#include "VideoWnd.h"

class CMainFrame : public CMDIFrameWnd
{
	DECLARE_DYNAMIC(CMainFrame)
public:
	CMainFrame();

// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	BOOL SetupHVDTesterItem();
	BOOL OpenHVDTestConfigFile(TCHAR *ptcConfigFile);

protected:  // control bar embedded members
	CStatusBar  m_wndStatusBar;

	CHVDTestCenter m_TestCenter;
	CCtrlPanel m_wndCtrlPanel;
	CVideoWnd  m_wndVideo;

// Generated message map functions
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnClose();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnWindowVideo();
public:
	afx_msg void OnWindowCtrlpanel();
	afx_msg LRESULT OnHVDTestFinish(WPARAM, LPARAM);
	afx_msg LRESULT OnHVDTestClose(WPARAM, LPARAM);
	afx_msg LRESULT OnHVDTestInProgress(WPARAM, LPARAM);
};


