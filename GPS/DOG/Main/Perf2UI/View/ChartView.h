#pragma once

#include "ChartCtrl.h"
#include "ChartLineSerie.h"
#include "ChartPointsSerie.h"

#include "EventCollection.h"
#include "DataSource.h"
#include "EventView.h"
#include <string>
#include <map>

// CChildView window

class CChartView : public CWnd, public CEventView
{
	DECLARE_DYNCREATE(CChartView)
// Construction
public:
	CChartView();
	virtual ~CChartView();

public:
	virtual void SetDataSource(CEventCollection* pEventCollection);
	virtual void AddEvent(DWORD id, std::basic_string<TCHAR> friendlyName);
	virtual void RemoveEvent(DWORD id);
	virtual std::vector<DWORD> GetEventList();
	virtual void Clear();
	virtual void Update();

public:
	void  SetRefreshRate(DWORD milliseconds);
	DWORD GetRefreshRate();
	void  ShowLegend(BOOL bShow);
	BOOL  IsLegendShown();
	void   SetViewRangeMin(double time_min) { m_ViewTimeRangeMin = time_min; }
	double GetViewRangeMin() { return m_ViewTimeRangeMin; }

	void SetAutoView(bool bAuto) { m_bAutoView = bAuto; }
	BOOL GetAutoView() { return m_bAutoView; }

	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

protected:
	// Overrides
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual int  OnCreate(LPCREATESTRUCT lpCreateStruct);

	// Generated message map functions
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	DECLARE_MESSAGE_MAP()

	CEventCollection* m_pEventCollection;


	struct DATA
	{
		DWORD id;
		DWORD eventType;
		std::basic_string<TCHAR> name;
		CDataSource* pDataSource;
		CChartXYSerie* pChartXYSerie;
		int iListViewItem;
	};

	struct STAT
	{
		double CurValue;
		double Xmin;
		double Xmax;
		double Ymin;
		double Ymax;
 		double Average;
	};

	std::map<DWORD, DATA> m_DataMap;

	CSplitterWnd m_cSplitter;
	CChartCtrl m_ChartCtrl;
	CListCtrl m_EventListView;
	DWORD m_RefreshRate;
	BOOL  m_bAutoView;

	double m_ViewTimeRangeMin;

	template<typename EventType, class EventToXY>
	inline void UpdateData(const EventToXY& converter, const CChartView::DATA& dat, CChartView::STAT* pStat);

	inline void UpdateDataSource(DWORD id, CDataSource* pDataSource);
	inline void UpdateListView(int iItem, const STAT& stat);
	inline void UpdateListView_DataRate(int iItem, double dataRate_in, double dataRate_out);
	inline void UpdateChart();

	enum
	{
		TimerID = 1,
		DefaultRefreshRate = 500,
	};
};

