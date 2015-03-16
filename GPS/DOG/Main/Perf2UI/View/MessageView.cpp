#include "stdafx.h"
#include "MessageView.h"
#include "MyVariant.h"

#include "AudioTypeDef.h"
#include "GuidUtil.h"
#include "WinUnicodeUtil.h"

#include <vector>
#include <limits>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CMessageView, CWnd)

// CChildView

CMessageView::CMessageView() : m_pEventCollection(NULL), m_RefreshRate(DefaultRefreshRate)
{
}

CMessageView::~CMessageView()
{
	Clear();
}


BEGIN_MESSAGE_MAP(CMessageView, CWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_TIMER()
END_MESSAGE_MAP()



// CChildView message handlers

BOOL CMessageView::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS,
		::LoadCursor(NULL, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW+1), NULL);

	return TRUE;
}

int CMessageView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	CRect rect;
	GetClientRect (&rect);
	const UINT width = rect.right - rect.left;
	const UINT height = rect.bottom - rect.top;

	m_MessageList.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | WS_CLIPSIBLINGS | LVS_REPORT | LVS_NOSORTHEADER | LVS_ALIGNLEFT, rect, this, 0);

	m_MessageList.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER| m_MessageList.GetExtendedStyle());
	m_MessageList.GetWindowRect(&rect);
	m_MessageList.InsertColumn(0, _T("Time"), LVCFMT_LEFT, 100);
	m_MessageList.InsertColumn(1, _T("Message"), LVCFMT_LEFT, 650);


	SetTimer(TimerID, m_RefreshRate, NULL);
	return 0;
}


void CMessageView::SetRefreshRate(DWORD milliseconds)
{
	m_RefreshRate = milliseconds;
	if(::IsWindow(m_hWnd))
	{
		SetTimer(TimerID, m_RefreshRate, NULL);
	}
}

DWORD CMessageView::GetRefreshRate()
{
	return m_RefreshRate;
}

void CMessageView::OnSize(UINT nType, int cx, int cy)
{
	CRect rect;
	GetClientRect (&rect);
	m_MessageList.MoveWindow(rect);
}


void CMessageView::SetDataSource(CEventCollection* pEventCollection)
{
	m_pEventCollection = pEventCollection;
}

void CMessageView::AddEvent(DWORD id, std::basic_string<TCHAR> friendlyName)
{
	std::map<DWORD, DATA>::iterator iter = m_DataMap.find(id);
	if(iter == m_DataMap.end()) 
	{
		DATA dat = { id, 0, friendlyName, NULL };
		m_DataMap[id] = dat;
	}
}

void CMessageView::RemoveEvent(DWORD id)
{
	std::map<DWORD, DATA>::iterator iter = m_DataMap.find(id);
	if(iter != m_DataMap.end())
	{

	}
	m_DataMap.erase(id);
}

std::vector<DWORD> CMessageView::GetEventList()
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

void CMessageView::Clear()
{
	m_DataMap.clear();
}


inline void CMessageView::UpdateDataSource(DWORD id, CDataSource* pDataSource)
{
	if(NULL == pDataSource)
	{
		return;
	}

	DATA dat = { id, 0, _T(""), pDataSource };

	std::map<DWORD, DATA>::iterator iter = m_DataMap.find(id);
	if(iter != m_DataMap.end()) 
	{
		dat = iter->second;
	}

	dat.pDataSource = pDataSource;
	dat.eventType = pDataSource->GetEventType();
	m_DataMap[id] = dat;
}

inline void CMessageView::AppendMessage(LONGLONG time, std::basic_string<TCHAR> message)
{
	int iItem = m_MessageList.GetItemCount();
	TCHAR strTime[32];
	_stprintf_s(strTime, sizeof(strTime)/sizeof(TCHAR), _T("%.3f"), time/1e7);

	LVITEM item = {LVIF_TEXT, iItem, 0, 0, 0, strTime, 0, 0};
	iItem = m_MessageList.InsertItem(&item);
	m_MessageList.SetItemText(iItem, 1, message.c_str());
}

inline std::basic_string<TCHAR> MediaTypeToString(const GUID& guid)
{
	std::basic_string<TCHAR> strSubType =_T("MEDIASUBTYPE_PCM");

	if(GUID_NULL == guid) { strSubType =_T("MEDIASUBTYPE_PCM");}
	else if(MEDIASUBTYPE_PCM == guid) { strSubType =_T("MEDIASUBTYPE_PCM"); }
	else if (MEDIASUBTYPE_DOLBY_AC3 == guid) {  strSubType =_T("MEDIASUBTYPE_DOLBY_AC3"); }
	else if (MEDIASUBTYPE_DTS == guid)    {  strSubType =_T("MEDIASUBTYPE_DTS");    }
	else if (MEDIASUBTYPE_DTSHD == guid)  {  strSubType =_T("MEDIASUBTYPE_DTSHD");  }
	else if (MEDIASUBTYPE_DTSLBR == guid) {  strSubType =_T("MEDIASUBTYPE_DTSLBR"); }
// 	else if (KSDATAFORMAT_RT_BITSTREAM_AUDIO == guid) {  strSubType =_T("KSDATAFORMAT_RT_BITSTREAM_AUDIO"); }
// 	else if (KSDATAFORMAT_SUBTYPE_RT_2ND_STREAM_AUDIO == guid) {  strSubType =_T("KSDATAFORMAT_SUBTYPE_RT_2ND_STREAM_AUDIO"); }
	else {  strSubType = GuidToString(guid); }

	return strSubType;
}

inline void CMessageView::UpdateMessage()
{
	std::map<DWORD, DATA>::iterator iter = m_DataMap.begin();
	for(; iter!=m_DataMap.end(); ++iter)
	{
		const DATA& dat = iter->second;

		using namespace dog;

		switch(dat.eventType)
		{
		default:
		case BASIC_EVENT_TYPE:
			break;

		case STRING_MESSAGE_EVENT_TYPE:
			if(dat.pDataSource) 
			{				
				size_t count = dat.pDataSource->GetCount();
				for(size_t i=0; i<count; ++i)
				{
					const StringEvent& strEvent = (StringEvent&)dat.pDataSource->GetEvent(0);
					LONGLONG time = strEvent.TimeStamp.QuadPart - strEvent.BaseTime.QuadPart;

					if(strEvent.nBytesOfChar == sizeof(TCHAR))
					{
						AppendMessage(time, (const TCHAR*)strEvent.szMessage);
					}
					else if( 1 == strEvent.nBytesOfChar && 2 == sizeof(TCHAR))
					{
						AppendMessage(time, (const TCHAR*)AnsiToUnicode(strEvent.szMessage).c_str());
					}
					else if (2 == strEvent.nBytesOfChar && 1 == sizeof(TCHAR))
					{	
						AppendMessage(time, (const TCHAR*)UnicodeToAnsi(strEvent.szMessageW).c_str());
					}

					dat.pDataSource->Pop();
				}
			}
			break;			

		case VARIANT_DATA_EVENT_TYPE:
			break;

		case DATA_PROCESS_EVENT_TYPE:
		case BUFFER_EVENT_TYPE:

			break;

		case VIDEO_FRAME_EVENT_TYPE:
			break;

		case AUDIO_STREAM_INFO_UPDATE_EVENT_TYPE:
			{				
				size_t count = dat.pDataSource->GetCount();
				for(size_t i=0; i<count; ++i)
				{
					const AudioStreamInfoUpdateEvent& infoEvent = (AudioStreamInfoUpdateEvent&)dat.pDataSource->GetEvent(0);
					LONGLONG time = infoEvent.TimeStamp.QuadPart - infoEvent.BaseTime.QuadPart;

					TCHAR strMsg[512];
					_stprintf_s(strMsg, sizeof(strMsg)/sizeof(TCHAR), _T("AudioStream= %s (%d Hz, %d ch, %d bits), Frame=%d, BitRate=%d"), 
						MediaTypeToString(infoEvent.StreamType).c_str(),
						infoEvent.SamplesPerSec, infoEvent.Channels, infoEvent.BitsPerSample,
						infoEvent.SamplesPerFrame,
						infoEvent.BitRate);

					AppendMessage(time, strMsg);
					dat.pDataSource->Pop();
				}
			}
			break;
		case AUDIO_OUTPUT_UPDATE_EVENT_TYPE:
			{
				size_t count = dat.pDataSource->GetCount();
				for(size_t i=0; i<count; ++i)
				{
					const AudioOutputUpdateEvent& outputEvent = (AudioOutputUpdateEvent&)dat.pDataSource->GetEvent(0);
					LONGLONG time = outputEvent.TimeStamp.QuadPart - outputEvent.BaseTime.QuadPart;

					TCHAR strMsg[512];
					_stprintf_s(strMsg, sizeof(strMsg)/sizeof(TCHAR), _T("AudioOutput= %s (%d Hz, %d ch, %d bits), renderer:%s"), 
						MediaTypeToString(outputEvent.OutputType).c_str(),
						outputEvent.SamplesPerSec, outputEvent.Channels, outputEvent.BitsPerSample,
						ToTcharString(outputEvent.RendererInfo).c_str());
					
					AppendMessage(time, strMsg);
					dat.pDataSource->Pop();
				}
			}
			break;
		case AUDIO_DEVICE_CAPABILITY_UPDATE_EVENT_TYPE:
			{
				size_t count = dat.pDataSource->GetCount();
				for(size_t i=0; i<count; ++i)
				{
					const AudioDevCapUpdateEvent& capEvent = (AudioDevCapUpdateEvent&)dat.pDataSource->GetEvent(0);
					LONGLONG time = capEvent.TimeStamp.QuadPart - capEvent.BaseTime.QuadPart;

					TCHAR strMsg[512];
					_stprintf_s(strMsg, sizeof(strMsg)/sizeof(TCHAR), _T("Audio Device Cap= DevType=%d (%d Hz, %d ch, %d bits) Speaker config=%d, Encoded support=%#x"),
						capEvent.IviAdoDevType,
						capEvent.SamplesPerSec, capEvent.Channels, capEvent.BitsPerSample,
						capEvent.SpeakerConfig,
						capEvent.SupportedEncodedFormats);

					AppendMessage(time, strMsg);
					dat.pDataSource->Pop();
				}
			}
			break;
		}
	}
}

void CMessageView::Update()
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

	UpdateMessage();
}

void  CMessageView::OnTimer(UINT_PTR nIDEvent)
{
	Update();
}

