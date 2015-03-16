// MessageFrm.cpp : implementation of the CChildFrame class
//
#include "stdafx.h"
#include "MyApp.h"

#include "MessageFrm.h"
#include "Inputbox.h"
#include <set>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CChildFrame

IMPLEMENT_DYNCREATE(CMessageFrame, CMDIChildWnd)

BEGIN_MESSAGE_MAP(CMessageFrame, CMDIChildWnd)
	ON_COMMAND(ID_FILE_CLOSE, &CMessageFrame::OnFileClose)
	ON_COMMAND(ID_ADD_REMOVE_EVENT, &CMessageFrame::OnAddRemoveEvent)
	ON_COMMAND(ID_VIEW_SET_TITLE, &CMessageFrame::OnSetTitle)
	ON_COMMAND(ID_EVENT_CLEAR_DATA, &CMessageFrame::OnClearEventData)
	ON_COMMAND(ID_VIEW_SET_REFRESHTIME, &CMessageFrame::OnSetRefreshTime)
	ON_WM_SETFOCUS()
	ON_WM_CREATE()
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CChildFrame construction/destruction

CMessageFrame::CMessageFrame()
{
	// TODO: add member initialization code here
}

CMessageFrame::~CMessageFrame()
{
}


BOOL CMessageFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying the CREATESTRUCT cs
	if( !CChildFrame::PreCreateWindow(cs) )
		return FALSE;

	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	cs.lpszClass = AfxRegisterWndClass(0);
	return TRUE;
}


// CChildFrame diagnostics

#ifdef _DEBUG
void CMessageFrame::AssertValid() const
{
	CMDIChildWnd::AssertValid();
}

void CMessageFrame::Dump(CDumpContext& dc) const
{
	CMDIChildWnd::Dump(dc);
}

#endif //_DEBUG


// CChildFrame message handlers
int CMessageFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CChildFrame::OnCreate(lpCreateStruct) == -1)
		return -1;

	// create a view to occupy the client area of the frame
	if (!m_messageView.Create(NULL, NULL, AFX_WS_DEFAULT_VIEW, 
		CRect(0, 0, 0, 0), this, AFX_IDW_PANE_FIRST, NULL))
	{
		TRACE0("Failed to create view window\n");
		return -1;
	}

	SetTitle(_T("Chart View"));

	return 0;
}

void CMessageFrame::OnSetFocus(CWnd* pOldWnd) 
{
	CMDIChildWnd::OnSetFocus(pOldWnd);

	m_messageView.SetFocus();
}

BOOL CMessageFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo) 
{
	// let the view have first crack at the command
	if (m_messageView.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	// otherwise, do default handling
	return CMDIChildWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CMessageFrame::OnSetRefreshTime()
{
	CInputBox inputbox(GetSafeHwnd());
	int interval = m_messageView.GetRefreshRate();
	CString strInterval;
	strInterval.Format(_T("%d"), interval);
	if (inputbox.DoModal(_T("Set Refresh Time"), _T("Set view refresh interval (in millisecond)"), strInterval.GetBuffer() ))
	{
		interval = _ttoi(inputbox.Text);
		if(interval >0) {
			m_messageView.SetRefreshRate(interval);
		}
	}
}
