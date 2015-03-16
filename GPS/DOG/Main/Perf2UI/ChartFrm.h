// ChartFrm.h : interface of the CChildFrame class
//

#pragma once


#include "ChildFrm.h"
#include "ChartView.h"
#include "EventView.h"

class CChartFrame : public CChildFrame
{
	DECLARE_DYNCREATE(CChartFrame)
public:
	CChartFrame();
	virtual ~CChartFrame();
	
public:
	virtual int GetWndType() 
	{
		return CChildFrame::CHART_WND;
	}

	virtual CEventView* GetEventView()
	{
		return &m_chartView;
	}

	// Overrides
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// Generated message map functions
protected:
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnViewLegend();
	afx_msg void OnViewAuto();
	afx_msg void OnViewLegend(CCmdUI* pCmdUI);
	afx_msg void OnViewAuto(CCmdUI* pCmdUI);
	afx_msg void OnSetRefreshTime();
	afx_msg void OnSetDisplayRange();
	DECLARE_MESSAGE_MAP()

	// view for the client area of the frame.
	CChartView m_chartView;
};
