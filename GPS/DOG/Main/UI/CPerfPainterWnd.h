// ChildView.h : interface of the CChildView class
//

#pragma once

#include "CPerfWnd.h"
#include "ChartCtrl.h"
#include "ChartLineSerie.h"
#include "ChartPointsSerie.h"

#include "DataSource.h"
#include <string>
#include <map>

// CChildView window

class CPerfPainterWnd : public CPerfWnd
{
	DECLARE_DYNCREATE(CPerfPainterWnd)
// Construction
public:
	CPerfPainterWnd();
	virtual ~CPerfPainterWnd();

// Operations
public:
	virtual void AddDataSource(DWORD id, std::basic_string<TCHAR> name, CDataSource* pDataSource);
	virtual void RemoveDataSource(DWORD id);
	virtual void Clear();
	virtual void Update();

// Overrides
protected:
	virtual int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual void OnDestroy();

// Generated message map functions
protected:
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	DECLARE_MESSAGE_MAP()

	struct DATA
	{
		DWORD id;
		DWORD eventType;
		std::basic_string<TCHAR> name;
		CDataSource* pDataSource;
		CChartSerie* pChartXYSerie;
	};

	struct STAT
	{
		double Xmin;
		double Xmax;
		double Ymin;
		double Ymax;
	};

	std::map<DWORD, DATA> m_DataMap;

	CChartCtrl m_ChartCtrl;

	template<typename EventType, class EventToXY>
	inline void UpdateData(const EventToXY& converter, const CPerfPainterWnd::DATA& dat, CPerfPainterWnd::STAT* pStat);

	enum {
		eTimerID = 1,
		eDefRefreshTimer = 1000,
	};
};

