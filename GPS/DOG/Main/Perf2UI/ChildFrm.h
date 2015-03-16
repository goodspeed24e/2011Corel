// ChartFrm.h : interface of the CChildFrame class
//


#pragma once


#include "ChartView.h"
#include "EventView.h"

class CChildFrame : public CMDIChildWnd
{
public:
	CChildFrame();
	virtual ~CChildFrame();

public:
	enum
	{
		NONE = 0,
		CHART_WND,
		MESSAGE_WND,
	};
	virtual int GetWndType() = 0;
	virtual CEventView* GetEventView() = 0;

	// Generated message map functions
protected:
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnFileClose();
	afx_msg void OnAddRemoveEvent();
	afx_msg void OnSetTitle();
	afx_msg void OnClearEventData();
	DECLARE_MESSAGE_MAP()
};
