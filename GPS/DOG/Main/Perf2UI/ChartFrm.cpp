// ChartFrm.cpp : implementation of the CChildFrame class
//
#include "stdafx.h"
#include "MyApp.h"

#include "ChartFrm.h"
#include "InputBox.h"
#include <set>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CChildFrame

IMPLEMENT_DYNCREATE(CChartFrame, CMDIChildWnd)

BEGIN_MESSAGE_MAP(CChartFrame, CMDIChildWnd)
	ON_COMMAND(ID_FILE_CLOSE, &CChartFrame::OnFileClose)
	ON_COMMAND(ID_ADD_REMOVE_EVENT, &CChartFrame::OnAddRemoveEvent)
	ON_COMMAND(ID_VIEW_LEGEND, &CChartFrame::OnViewLegend)
	ON_COMMAND(ID_VIEW_AUTO, &CChartFrame::OnViewAuto)
	ON_COMMAND(ID_VIEW_SET_TITLE, &CChartFrame::OnSetTitle)
	ON_COMMAND(ID_EVENT_CLEAR_DATA, &CChartFrame::OnClearEventData)	
	ON_COMMAND(ID_VIEW_SET_CHART_DISPLAY_RANGE, &CChartFrame::OnSetDisplayRange)	
	ON_UPDATE_COMMAND_UI(ID_VIEW_LEGEND, &CChartFrame::OnViewLegend)
	ON_UPDATE_COMMAND_UI(ID_VIEW_AUTO, &CChartFrame::OnViewAuto)
	ON_COMMAND(ID_VIEW_SET_REFRESHTIME, &CChartFrame::OnSetRefreshTime)	
	ON_WM_SETFOCUS()
	ON_WM_CREATE()
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CChildFrame construction/destruction

CChartFrame::CChartFrame()
{
	// TODO: add member initialization code here
}

CChartFrame::~CChartFrame()
{

}


BOOL CChartFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying the CREATESTRUCT cs
	if( !CMDIChildWnd::PreCreateWindow(cs) )
		return FALSE;

	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	cs.lpszClass = AfxRegisterWndClass(0);
	return TRUE;
}


// CChildFrame diagnostics

#ifdef _DEBUG
void CChartFrame::AssertValid() const
{
	CMDIChildWnd::AssertValid();
}

void CChartFrame::Dump(CDumpContext& dc) const
{
	CMDIChildWnd::Dump(dc);
}

#endif //_DEBUG


// CChildFrame message handlers
int CChartFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CChildFrame::OnCreate(lpCreateStruct) == -1)
		return -1;

	// create a view to occupy the client area of the frame
	if (!m_chartView.Create(NULL, NULL, AFX_WS_DEFAULT_VIEW,
		CRect(0, 0, 0, 0), this, AFX_IDW_PANE_FIRST, NULL))
	{
		TRACE0("Failed to create view window\n");
		return -1;
	}

	SetTitle(_T("Chart View"));

	return 0;
}

void CChartFrame::OnSetFocus(CWnd* pOldWnd)
{
	CMDIChildWnd::OnSetFocus(pOldWnd);

	m_chartView.SetFocus();
}

BOOL CChartFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// let the view have first crack at the command
	if (m_chartView.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	// otherwise, do default handling
	return CMDIChildWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CChartFrame::OnViewLegend()
{
	BOOL bShow = m_chartView.IsLegendShown();
	m_chartView.ShowLegend(!bShow);
}

void CChartFrame::OnViewAuto()
{
	BOOL bAuto = m_chartView.GetAutoView();
	m_chartView.SetAutoView(!bAuto);
}

void CChartFrame::OnViewLegend(CCmdUI* pCmdUI)
{
	BOOL bShow = m_chartView.IsLegendShown();
	pCmdUI->SetCheck(bShow);
}

void CChartFrame::OnViewAuto(CCmdUI* pCmdUI)
{
	BOOL bAuto = m_chartView.GetAutoView();
	pCmdUI->SetCheck(bAuto);
}

void CChartFrame::OnSetRefreshTime()
{
	CInputBox inputbox(GetSafeHwnd());
	int interval = m_chartView.GetRefreshRate();
	CString strInterval;
	strInterval.Format(_T("%d"), interval);
	if (inputbox.DoModal(_T("Set Refresh Time"), _T("Set view refresh interval (in millisecond)"), strInterval.GetBuffer() ))
	{
		interval = _ttoi(inputbox.Text);
		if(interval >0) {
			m_chartView.SetRefreshRate(interval);
		}
	}
}

void CChartFrame::OnSetDisplayRange()
{
	CInputBox inputbox(GetSafeHwnd());
	int rangeMin = m_chartView.GetViewRangeMin()*1e4;
	CString strRangeMin;
	strRangeMin.Format(_T("%d"), rangeMin);
	if (inputbox.DoModal(_T("Set Display Range"), _T("Sepecify display range of time line (in millisecond)\n(0 means unlimited)"), strRangeMin.GetBuffer() ))
	{
		rangeMin = _ttoi(inputbox.Text);
		if(rangeMin >=0) {
			m_chartView.SetViewRangeMin((double)rangeMin/1e4);
		}
	}
}
