#include "stdafx.h"
#include "ChartView.h"
#include "MyVariant.h"

#include "HolderView.h"

#include <vector>
#include <limits>
#include <boost/format.hpp>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const double DOUBLE_LIMIT = (std::numeric_limits<double>::max)() / 10.0;


IMPLEMENT_DYNCREATE(CChartView, CWnd)

// CChildView

CChartView::CChartView() : m_pEventCollection(NULL), m_RefreshRate(DefaultRefreshRate), m_bAutoView(TRUE), m_ViewTimeRangeMin(0)
{
}

CChartView::~CChartView()
{
	Clear();
}


BEGIN_MESSAGE_MAP(CChartView, CWnd)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_TIMER()
END_MESSAGE_MAP()



// CChildView message handlers

BOOL CChartView::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS,
		::LoadCursor(NULL, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW+1), NULL);

	return TRUE;
}

int CChartView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	CRect rect;
	GetClientRect (&rect);
	const UINT width = rect.right - rect.left;
	const UINT height = rect.bottom - rect.top;


	// create splitter
	m_cSplitter.CreateStatic(this, 2, 1);
	m_cSplitter.ModifyStyleEx(WS_EX_CLIENTEDGE | WS_THICKFRAME, 0, SWP_NOSIZE|SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);

	CCreateContext context;
	context.m_pNewViewClass = RUNTIME_CLASS(CHolderView);
	m_cSplitter.CreateView(0,0, RUNTIME_CLASS(CHolderView), CSize(0, 200), &context);
	m_cSplitter.CreateView(1,0, RUNTIME_CLASS(CHolderView), CSize(0, 200), &context);
	CHolderView* pWnd0 = (CHolderView*)m_cSplitter.GetPane(0,0);
	CHolderView* pWnd1 = (CHolderView*)m_cSplitter.GetPane(1,0);


	// create chart view
	m_ChartCtrl.Create(pWnd0, rect, 0, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS);
	CChartStandardAxis* pBottomAxis = m_ChartCtrl.CreateStandardAxis(CChartCtrl::BottomAxis);
	pBottomAxis->SetMinMax(0, 10);
	CChartStandardAxis* pLeftAxis = m_ChartCtrl.CreateStandardAxis(CChartCtrl::LeftAxis);
	pLeftAxis->SetMinMax(0, 10);

	m_ChartCtrl.GetLegend()->SetVisible(true);
	m_ChartCtrl.GetLegend()->UndockLegend(40,20);


	// create list view
	m_EventListView.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | WS_CLIPSIBLINGS | LVS_REPORT | LVS_NOSORTHEADER | LVS_ALIGNLEFT, rect, pWnd1, 0);
	m_EventListView.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER| m_EventListView.GetExtendedStyle());
	m_EventListView.GetWindowRect(&rect);
	{
		m_EventListView.InsertColumn(0, _T("Legend"), LVCFMT_LEFT, 60);
		m_EventListView.InsertColumn(1, _T("Id"),     LVCFMT_LEFT, 60);
		m_EventListView.InsertColumn(2, _T("Name"),   LVCFMT_LEFT, 120);
		m_EventListView.InsertColumn(3, _T("Value"),  LVCFMT_LEFT, 70);
		m_EventListView.InsertColumn(4, _T("Min"),  LVCFMT_LEFT, 70);
		m_EventListView.InsertColumn(5, _T("Max"),  LVCFMT_LEFT, 70);
		m_EventListView.InsertColumn(6, _T("Avg"),  LVCFMT_LEFT, 70);
		m_EventListView.InsertColumn(7, _T("Data Rate (In)"),   LVCFMT_LEFT, 100);
		m_EventListView.InsertColumn(8, _T("Data Rate (Out)"),  LVCFMT_LEFT, 100);
	}


	// attach to CSplitterWnd
	pWnd0->AttachWnd(&m_ChartCtrl, this);
	pWnd1->AttachWnd(&m_EventListView, this);

	SetTimer(TimerID, m_RefreshRate, NULL);
	return 0;
}


void CChartView::SetRefreshRate(DWORD milliseconds)
{
	m_RefreshRate = milliseconds;
	if(::IsWindow(m_hWnd))
	{
		SetTimer(TimerID, m_RefreshRate, NULL);
	}
}

DWORD CChartView::GetRefreshRate()
{
	return m_RefreshRate;
}

void CChartView::ShowLegend(BOOL bShow)
{
	m_ChartCtrl.GetLegend()->SetVisible(bShow);
}

BOOL CChartView::IsLegendShown()
{
	return m_ChartCtrl.GetLegend()->IsVisible();
}

void CChartView::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	// TODO: Add your message handler code here

	// Do not call CWnd::OnPaint() for painting messages
}

void CChartView::OnSize(UINT nType, int cx, int cy)
{
	CRect rect;
	GetClientRect (&rect);
//	m_ChartCtrl.MoveWindow(rect);
	m_cSplitter.MoveWindow(rect);
}

BOOL CChartView::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	if (m_cSplitter.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	// otherwise, do default handling
	return CWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}


void CChartView::SetDataSource(CEventCollection* pEventCollection)
{
	m_pEventCollection = pEventCollection;
}

void CChartView::AddEvent(DWORD id, std::basic_string<TCHAR> friendlyName)
{
	std::map<DWORD, DATA>::iterator iter = m_DataMap.find(id);
	if(iter == m_DataMap.end())
	{
		DATA dat = { id, 0, friendlyName, NULL, NULL, -1 };
		m_DataMap[id] = dat;
	}
}

void CChartView::RemoveEvent(DWORD id)
{
	std::map<DWORD, DATA>::iterator iter = m_DataMap.find(id);
	if(iter != m_DataMap.end())
	{
		//remove exist line
		if(iter->second.pChartXYSerie)
		{
			unsigned uSerieId = iter->second.pChartXYSerie->GetSerieId();
			m_ChartCtrl.RemoveSerie(uSerieId);
		}
	}
	m_DataMap.erase(id);
}

std::vector<DWORD> CChartView::GetEventList()
{
	std::vector<DWORD> eventList;

	const size_t n = m_DataMap.size();
	eventList.reserve(n);
	std::map<DWORD, DATA>::iterator iter = m_DataMap.begin();
	for(; iter!=m_DataMap.end(); ++iter)
	{
		eventList.push_back(iter->first);
	}

	return eventList;
}

void CChartView::Clear()
{
	m_DataMap.clear();
	m_ChartCtrl.RemoveAllSeries();
}

inline void CChartView::UpdateDataSource(DWORD id, CDataSource* pDataSource)
{
	if(NULL == pDataSource)
	{
		return;
	}

	DATA dat = { id, 0, _T(""), pDataSource, NULL, -1 };

	std::map<DWORD, DATA>::iterator iter = m_DataMap.find(id);
	if(iter != m_DataMap.end())
	{
		dat = iter->second;
	}

	dat.pDataSource = pDataSource;
	dat.eventType = pDataSource->GetEventType();

	if(NULL == dat.pChartXYSerie)
	{
		using namespace dog;

		// create a new line
		switch(dat.eventType)
		{
		default:
		case BASIC_EVENT_TYPE:
			{
				CChartPointsSerie* pPointsSeries = m_ChartCtrl.CreatePointsSerie();
				pPointsSeries->SetName(dat.name);
				dat.pChartXYSerie = pPointsSeries;
			}
			break;

		case VARIANT_DATA_EVENT_TYPE:
		case DATA_PROCESS_EVENT_TYPE:
		case BUFFER_EVENT_TYPE:
			{
				CChartLineSerie* pLineSeries = m_ChartCtrl.CreateLineSerie();
				pLineSeries->SetName(dat.name);
				dat.pChartXYSerie = pLineSeries;

				int nItmCount = m_EventListView.GetItemCount();
				LVITEM item = {LVIF_TEXT, nItmCount, 0, 0, 0, _T(""), 0, 0};
				int iItem = m_EventListView.InsertItem(&item);
				dat.iListViewItem = iItem;

				typedef boost::basic_format<TCHAR> tformat;
				m_EventListView.SetItemText(iItem, 1, (tformat(_T("%d")) % dat.id).str().c_str());
				m_EventListView.SetItemText(iItem, 2, dat.name.c_str());
			}
			break;
		}
	}
	m_DataMap[id] = dat;
}

/// UpdateData<EventType, EventToXY>
/// Generic method for updating data to chart view
/// Caller needs to provide an EventToXY converter to provide x,y values for chart view
template<typename EventType, class EventToXY>
inline void CChartView::UpdateData(const EventToXY& converter, const CChartView::DATA& dat, CChartView::STAT* pStat)
{
	using namespace dog;

	if(NULL == dat.pDataSource || NULL == dat.pChartXYSerie ) {
		return;
	}

	size_t count = dat.pDataSource->GetCount();
	if(count <=1) {
		return;
	}

	double fXmin= DOUBLE_LIMIT;
	double fXmax= -DOUBLE_LIMIT;
	double fYmin= DOUBLE_LIMIT;
	double fYmax= -DOUBLE_LIMIT;
	double total = 0;

	std::vector<double> X;
	std::vector<double> Y;
	X.reserve(count);
	Y.reserve(count);

	for(size_t i=0; i<count; ++i)
	{
		double _x_=0, _y_=0;
		converter.GetXY((EventType&)dat.pDataSource->GetEvent((int)i), _x_, _y_);

		X.push_back(_x_);
		Y.push_back(_y_);

		if(pStat)
		{
			fXmin = (std::min)(fXmin, _x_);
			fXmax = (std::max)(fXmax, _x_);
			fYmin = (std::min)(fYmin, _y_);
			fYmax = (std::max)(fYmax, _y_);
			total += _y_;
		}
	}

	dat.pChartXYSerie->ClearSerie();
	dat.pChartXYSerie->AddPoints(&X[0], &Y[0], (unsigned int)count);

	if(pStat)
	{
		int curVal = 0;
		double avg = 0;
		if(count>0)
		{
			double _x_=0, _y_=0;
			converter.GetXY((EventType&)dat.pDataSource->GetEvent((int) count-1), _x_, _y_);
			curVal = _y_;

			LARGE_INTEGER BeginTime = dat.pDataSource->GetEvent(0).TimeStamp;
			LARGE_INTEGER EndTime = dat.pDataSource->GetEvent((int)(count-1)).TimeStamp;
			avg = total / count;
		}

		pStat->CurValue = curVal;
		pStat->Xmin = fXmin;
		pStat->Xmax = fXmax;
		pStat->Ymin = fYmin;
		pStat->Ymax = fYmax;
		pStat->Average = avg;
	}
}

/// Calculate data rate with the specified number of latest samples
inline std::pair<double, double> CalcBufferDataRate(CDataSource* pDataSource, size_t nProfileSamples)
{
	using namespace dog;

	if(NULL == pDataSource) {
		return std::pair<double, double>(0,0);
	}

	const DWORD eventType = pDataSource->GetEventType();
	if( eventType != DATA_PROCESS_EVENT_TYPE &&
		eventType != BUFFER_EVENT_TYPE)
	{
		return std::pair<double, double>(0,0);
	}

	size_t count = pDataSource->GetCount();
	if(count <=1) {
		return std::pair<double, double>(0,0);
	}

	struct DataRateStat
	{
		size_t Samples;
		size_t SumOfData;
		LARGE_INTEGER BeginTime;
		double DataRate;

		DataRateStat() : Samples(0), SumOfData(0), DataRate(0) {}
	};

	DataRateStat DataIn_Stat;
	DataRateStat DataOut_Stat;

	// calculate data rate from the latest samples
	for(int i=count-1; i>=0 ; --i)
	{
		const BufferEvent& bufEvent = (BufferEvent&)pDataSource->GetEvent(i);
// 		if(DataIn_Stat.Samples >= nProfileSamples && DataOut_Stat.Samples>= nProfileSamples) {
// 			break;
// 		}

		switch(bufEvent.Operation)
		{
		case BufferEvent::DATA_IN:
			++DataIn_Stat.Samples;
			DataIn_Stat.SumOfData += bufEvent.DataSize;
			DataIn_Stat.BeginTime = bufEvent.TimeStamp;
			break;
		case BufferEvent::DATA_OUT:
			++DataOut_Stat.Samples;
			DataOut_Stat.SumOfData += bufEvent.DataSize;
			DataOut_Stat.BeginTime = bufEvent.TimeStamp;
			break;
		default:
			continue;
		}
	}

	LARGE_INTEGER EndTime = pDataSource->GetEvent((int)(count-1)).TimeStamp;
	DataIn_Stat.DataRate  = (double)DataIn_Stat.SumOfData  / ((double)(EndTime.QuadPart-DataIn_Stat.BeginTime.QuadPart) /1e7);
	DataOut_Stat.DataRate = (double)DataOut_Stat.SumOfData / ((double)(EndTime.QuadPart-DataOut_Stat.BeginTime.QuadPart) /1e7);
	return std::pair<double, double>(DataIn_Stat.DataRate, DataOut_Stat.DataRate);
}


inline bool OutOfRange(double x)
{
	return !(-DOUBLE_LIMIT < x && x < DOUBLE_LIMIT);
}

inline void MinMaxAdjustment(double& fXmin, double& fXmax, double& fYmin, double& fYmax)
{
	fXmin= OutOfRange(fXmin) ?  0 : fXmin;
	fXmax= OutOfRange(fXmax) ? 10 : fXmax;
	fYmin= OutOfRange(fYmin) ?  0 : fYmin;
	fYmax= OutOfRange(fYmax) ? 10 : fYmax;

	// if min, max are the same
	if(fXmin == fXmax)
	{
		fXmin -=5;
		fXmax +=5;
	}
	if(fYmin == fYmax)
	{
		fYmin -=5;
		fYmax +=5;
	}
}

inline void CChartView::UpdateListView(int iItem, const CChartView::STAT& stat)
{
	if(iItem >= 0)
	{
		typedef boost::basic_format<TCHAR> tformat;
		m_EventListView.SetItemText(iItem, 3, (tformat(_T("%d")) % stat.CurValue).str().c_str());
		m_EventListView.SetItemText(iItem, 4, (tformat(_T("%d")) % stat.Ymin).str().c_str());
		m_EventListView.SetItemText(iItem, 5, (tformat(_T("%d")) % stat.Ymax).str().c_str());
		m_EventListView.SetItemText(iItem, 6, (tformat(_T("%.3f")) % stat.Average).str().c_str());
	}
}

inline void CChartView::UpdateListView_DataRate(int iItem, double dataRate_in, double dataRate_out)
{
	if(iItem >= 0)
	{
		typedef boost::basic_format<TCHAR> tformat;
		m_EventListView.SetItemText(iItem, 7, (tformat(_T("%.3f")) % dataRate_in).str().c_str());
		m_EventListView.SetItemText(iItem, 8, (tformat(_T("%.3f")) % dataRate_out).str().c_str());
	}
}

inline void CChartView::UpdateChart()
{
	m_ChartCtrl.EnableRefresh(false);

	double fXmin= DOUBLE_LIMIT;
	double fXmax= -DOUBLE_LIMIT;
	double fYmin= DOUBLE_LIMIT;
	double fYmax= -DOUBLE_LIMIT;

	std::map<DWORD, DATA>::iterator iter = m_DataMap.begin();
	for(; iter!=m_DataMap.end(); ++iter)
	{
		const DATA& dat = iter->second;
		STAT stat = { 0, fXmin, fXmax, fYmin, fYmax, 0 };
		size_t nBasicTypeCount = 1;

		using namespace dog;

		switch(dat.eventType)
		{
		default:
		case BASIC_EVENT_TYPE:
			{
				struct DogEventToXY
				{
					double fYVal;
					DogEventToXY(double y) : fYVal(y)
					{}

					void GetXY(const DogEvent& event, double& x, double& y) const
					{
						x = (double)(event.TimeStamp.QuadPart - event.BaseTime.QuadPart)/1e7;
						y = fYVal;
					}
				};
				UpdateData<DogEvent>(DogEventToXY(nBasicTypeCount), dat, &stat);
				++nBasicTypeCount;
			}
			break;

		case VARIANT_DATA_EVENT_TYPE:
			{
				struct VariantDataEventToXY
				{
					void GetXY(const VariantDataEvent& dataEvent, double& x, double& y) const
					{
						MyVariant myVar = dataEvent.Data;
						x = (double)(dataEvent.TimeStamp.QuadPart - dataEvent.BaseTime.QuadPart)/1e7;
						y = myVar.ToDouble();
					}
				};
				UpdateData<VariantDataEvent>(VariantDataEventToXY(), dat, &stat);
				UpdateListView(dat.iListViewItem, stat);
			}
			break;

		case DATA_PROCESS_EVENT_TYPE:
		case BUFFER_EVENT_TYPE:
			{
				struct BufferEventToXY
				{
					void GetXY(const BufferEvent& bufEvent, double& x, double& y) const
					{
						x = (double)(bufEvent.TimeStamp.QuadPart - bufEvent.BaseTime.QuadPart)/1e7;
						y = bufEvent.BufferSize;
					}
				};
				UpdateData<BufferEvent>(BufferEventToXY(), dat, &stat);
				UpdateListView(dat.iListViewItem, stat);

				const size_t nProfileSamples = 3; // calculate data rate with the latest 3 samples
				std::pair<double, double> dataRates = CalcBufferDataRate(dat.pDataSource, nProfileSamples);
				UpdateListView_DataRate(dat.iListViewItem, dataRates.first, dataRates.second);

				if(dat.pDataSource && dat.pDataSource->GetCount()>0)
				{
					BufferEvent& bufEvent = (BufferEvent&)dat.pDataSource->GetEvent(0);
					stat.Ymax = bufEvent.Capacity;
				}
				stat.Ymin = 0;
			}
			break;
		}

		fXmin = (std::min)(fXmin, stat.Xmin);
		fXmax = (std::max)(fXmax, stat.Xmax);
		fYmin = (std::min)(fYmin, stat.Ymin);
		fYmax = (std::max)(fYmax, stat.Ymax);
	}


	if(m_bAutoView)
	{
		// make sure min and max are not glued together
		MinMaxAdjustment(fXmin, fXmax, fYmin, fYmax);

		if(m_ViewTimeRangeMin > 0) 
		{
			fXmin = (std::max)(fXmin, fXmax-m_ViewTimeRangeMin);
		}

		m_ChartCtrl.GetLeftAxis()->SetMinMax(fYmin, fYmax);
		m_ChartCtrl.GetBottomAxis()->SetMinMax(fXmin, fXmax);
	}

	m_ChartCtrl.EnableRefresh(true);
}


void CChartView::Update()
{
	if(NULL == m_pEventCollection)
	{
		return;
	}

	std::map<DWORD, DATA>::iterator iter = m_DataMap.begin();
	for(; iter!=m_DataMap.end(); ++iter)
	{
		const DATA& dat = iter->second;
		UpdateDataSource(dat.id, m_pEventCollection->GetDataSource(dat.id));
	}
	UpdateChart();
}

void  CChartView::OnTimer(UINT_PTR nIDEvent)
{
	Update();
}

