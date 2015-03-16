// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "HVDServiceTester.h"

#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_COMMAND(ID_WINDOW_VIDEO, &CMainFrame::OnWindowVideo)
	ON_COMMAND(ID_WINDOW_CTRLPANEL, &CMainFrame::OnWindowCtrlpanel)
	ON_MESSAGE(WM_APP_HVDTEST_IN_PROGRESS, &CMainFrame::OnHVDTestInProgress)
	ON_MESSAGE(WM_APP_HVDTEST_FINISH, &CMainFrame::OnHVDTestFinish)
	ON_MESSAGE(WM_APP_HVDTEST_CLOSE, &CMainFrame::OnHVDTestClose)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};


// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
}

CMainFrame::~CMainFrame()
{
}


int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}
	return 0;
}

void CMainFrame::OnClose()
{
	m_TestCenter.StopTest(FALSE);
	m_TestCenter.CloseHVDTestCenter();
	m_wndVideo.DestroyWindow();
	m_wndCtrlPanel.DestroyWindow();
	CWnd::OnClose();
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CMDIFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}


// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWnd::Dump(dc);
}

#endif //_DEBUG


BOOL CMainFrame::SetupHVDTesterItem()
{
	m_TestCenter.SetParentWnd(this);

	OnWindowVideo();
	OnWindowCtrlpanel();
	m_TestCenter.SetVideoWnd(&m_wndVideo);
	m_TestCenter.SetupHVDTestCenter();
	return TRUE;
}

BOOL CMainFrame::OpenHVDTestConfigFile(TCHAR *ptcConfigFile)
{
	m_TestCenter.LaunchTest(ptcConfigFile);
	return TRUE;
}

// CMainFrame message handlers


void CMainFrame::OnWindowVideo()
{
	// TODO: Add your command handler code here

	if(!::IsWindow(m_wndVideo.GetSafeHwnd()))
	{
		CRect ParentRect;
		GetClientRect(&ParentRect);
		ClientToScreen(&ParentRect);
		m_wndVideo.Create(this, ParentRect.TopLeft());
	}
	else
		m_wndVideo.ShowWindow(SW_SHOW);
	m_wndVideo.SetForegroundWindow();
}

void CMainFrame::OnWindowCtrlpanel()
{
	// TODO: Add your command handler code here
	if(!::IsWindow(m_wndCtrlPanel.GetSafeHwnd()))
		m_wndCtrlPanel.Create(this, &m_TestCenter);
	else
		m_wndCtrlPanel.ShowWindow(SW_SHOW);
	m_wndCtrlPanel.SetForegroundWindow();
}

LRESULT CMainFrame::OnHVDTestInProgress(WPARAM wParam, LPARAM lParam)
{
	m_wndCtrlPanel.SetCtrlPanelItemState(ePanel_InProgress_State);
	return S_OK;
}

LRESULT CMainFrame::OnHVDTestFinish(WPARAM wParam, LPARAM lParam)
{
	m_wndCtrlPanel.SetCtrlPanelItemState(ePanel_Default_State);
	return S_OK;
}

LRESULT CMainFrame::OnHVDTestClose(WPARAM wParam, LPARAM lParam)
{
	PostMessage(WM_CLOSE, NULL, NULL);
	return S_OK;
}