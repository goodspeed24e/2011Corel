#include "stdafx.h"
#include "MyApp.h"
#include "CPerfPainterWnd.h"
#include "MainFrame.h"
#include "MyVariant.h"

#include <vector>
#include <limits>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CPerfPainterWnd, CWnd)

// CChildView

CPerfPainterWnd::CPerfPainterWnd()
{
}

CPerfPainterWnd::~CPerfPainterWnd()
{
	Clear();
}


BEGIN_MESSAGE_MAP(CPerfPainterWnd, CWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_TIMER()
END_MESSAGE_MAP()



// CChildView message handlers

int CPerfPainterWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CPerfWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rect;
	GetClientRect (&rect);
	m_ChartCtrl.MoveWindow(rect);

	m_ChartCtrl.Create(this, rect, 0, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS);

	CChartStandardAxis* pBottomAxis = m_ChartCtrl.CreateStandardAxis(CChartCtrl::BottomAxis);
	pBottomAxis->SetMinMax(0, 10);
	CChartStandardAxis* pLeftAxis = m_ChartCtrl.CreateStandardAxis(CChartCtrl::LeftAxis);
	pLeftAxis->SetMinMax(0, 10);

//	m_ChartCtrl.GetLegend()->SetVisible(true);
//	m_ChartCtrl.GetLegend()->UndockLegend(40,20);


	SetTimer(eTimerID, eDefRefreshTimer, NULL);

	return 0;
}

void CPerfPainterWnd::OnDestroy()
{
	KillTimer(eTimerID);

	//Only destroy this window, not destroy all app
	if (((MainFrame*)GetParent())->m_DestroyMainFrame == false)
	{
		//Remove from MainFrame's list
		((MainFrame*)GetParent())->RemoveWindowFromList(this);
	}

	CPerfWnd::OnDestroy();
}

void CPerfPainterWnd::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	// TODO: Add your message handler code here

	// Do not call CWnd::OnPaint() for painting messages
}

void CPerfPainterWnd::OnSize(UINT nType, int cx, int cy)
{
	CRect rect;
	GetClientRect (&rect);
	m_ChartCtrl.MoveWindow(rect);
}

void CPerfPainterWnd::OnTimer(UINT_PTR nIDEvent)
{
	Update();
}


void CPerfPainterWnd::AddDataSource(DWORD id, std::basic_string<TCHAR> name, CDataSource* pDataSource)
{
	if(NULL == pDataSource) {
		return;
	}

	DATA dat = { id, pDataSource->GetEventType(), name, pDataSource, NULL };

	std::map<DWORD, DATA>::iterator iter = m_DataMap.find(id);
	if(iter == m_DataMap.end())
	{
		using namespace dog;

		// create a new line
		switch(dat.eventType)
		{
		default:
		case BASIC_EVENT_TYPE:
			{
				CChartPointsSerie* pPointsSeries = m_ChartCtrl.CreatePointsSerie();
				pPointsSeries->SetName(name);
				dat.pChartXYSerie = pPointsSeries;
			}
			break;

		case VARIANT_DATA_EVENT_TYPE:
		case DATA_PROCESS_EVENT_TYPE:
		case BUFFER_EVENT_TYPE:
			{
				CChartLineSerie* pLineSeries = m_ChartCtrl.CreateLineSerie();
				pLineSeries->SetName(name);
				dat.pChartXYSerie = pLineSeries;
			}
			break;
		}
	}
	else
	{
		dat.pChartXYSerie = iter->second.pChartXYSerie;
	}
	m_DataMap[id] = dat;
}

void CPerfPainterWnd::RemoveDataSource(DWORD id)
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

void CPerfPainterWnd::Clear()
{
	m_DataMap.clear();
	m_ChartCtrl.RemoveAllSeries();
}


template<typename EventType, class EventToXY>
inline void CPerfPainterWnd::UpdateData(const EventToXY& converter, const CPerfPainterWnd::DATA& dat, CPerfPainterWnd::STAT* pStat)
{
	using namespace dog;

	if(NULL == dat.pDataSource || NULL == dat.pChartXYSerie ) {
		return;
	}

	size_t count = dat.pDataSource->GetCount();
	if(count <=1) {
		return;
	}

	double fXmin= (std::numeric_limits<double>::max)();
	double fXmax= -(std::numeric_limits<double>::max)();
	double fYmin= (std::numeric_limits<double>::max)();
	double fYmax= -(std::numeric_limits<double>::max)();

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
		}
	}

	dat.pChartXYSerie->ClearSerie();
	dat.pChartXYSerie->AddPoints(&X[0], &Y[0], (unsigned int)count);

	if(pStat)
	{
		pStat->Xmin = fXmin;
		pStat->Xmax = fXmax;
		pStat->Ymin = fYmin;
		pStat->Ymax = fYmax;
	}
}

inline void MinMaxAdjustment(double& fXmin, double& fXmax, double& fYmin, double& fYmax)
{
	// if no min, max update
	fXmin = fXmin ==  (std::numeric_limits<double>::max)() ?  0 : fXmin;
	fXmax = fXmax == -(std::numeric_limits<double>::max)() ? 10 : fXmax;
	fYmin = fYmin ==  (std::numeric_limits<double>::max)() ?  0 : fYmin;
	fYmax = fYmax == -(std::numeric_limits<double>::max)() ? 10 : fYmax;

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

void CPerfPainterWnd::Update()
{
	m_ChartCtrl.EnableRefresh(false);

	double fXmin= (std::numeric_limits<double>::max)();
	double fXmax= -(std::numeric_limits<double>::max)();
	double fYmin= (std::numeric_limits<double>::max)();
	double fYmax= -(std::numeric_limits<double>::max)();

	std::map<DWORD, DATA>::iterator iter = m_DataMap.begin();
	if(iter == m_DataMap.end())
	{
		fXmin=0;
		fXmax=10;
		fYmin=0;
		fYmax=10;
	}

	for(; iter!=m_DataMap.end(); ++iter)
	{
		const DATA& dat = iter->second;
		STAT stat;
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


	MinMaxAdjustment(fXmin, fXmax, fYmin, fYmax);

	m_ChartCtrl.GetLeftAxis()->SetMinMax(fYmin, fYmax);
	m_ChartCtrl.GetBottomAxis()->SetMinMax(fXmin, fXmax);

	m_ChartCtrl.EnableRefresh(true);
}
