#pragma warning (disable:4819)
//boost_1_35_0\include\boost\utility\enable_if.hpp : warning C4819: The file contains a character that cannot be represented in the current code page (950). Save the file in Unicode format to prevent data loss
#pragma warning (disable:4267)
// warning C4267: '=' : conversion from 'size_t' to 'DWORD', possible loss of data

#include <tchar.h>

#include <boost/test/unit_test.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/condition.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/format.hpp>

#include <vector>
#include <string>

#include "FastRingBufferTemplate.h"

#include "DogCategory.h"
#include "SubCategory.h"
#include "DOGSession.h"
#include "DogUserEvent.h"
using namespace dog;
using namespace dogsub;


namespace DogSessionTest {

//-----------------------------------------------------------------------------
class CMemoryMappingTest : public TFastRingBuffer<CSessionBuffer::TDataPacket>
{
public:
	void SetVersion(CSessionBuffer::Version ver)
	{
		m_Version.MajorMain = ver.MajorMain;
		m_Version.MajorSub  = ver.MajorSub;
		m_Version.MinorMain = ver.MinorMain;
		m_Version.MinorSub  = ver.MinorSub;
	}
};

class CSessionBufferTest : public CSessionBuffer
{
public:
	void SetVersion(CSessionBuffer::Version ver) { ((CMemoryMappingTest*)m_pMem)->SetVersion(ver); }
};

class CDOGSessionTest : public CDOGSession
{
public:
	CSessionBuffer::Version GetVersion() const { return  m_SessionBuf.GetVersion(); }
	void SetVersion(CSessionBuffer::Version ver) { ((CSessionBufferTest*)&m_SessionBuf)->SetVersion(ver); }
};

//-----------------------------------------------------------------------------
template< typename T >
bool CompareResult(const std::vector<T>& src, const std::vector<T>& dest)
{
	bool bEqualSize = src.size() == dest.size();
	if(!bEqualSize) {
		return false;
	}

	const size_t sizeOfT = sizeof(T);

	for(size_t i=0; i<src.size(); ++i)
	{
		if(src[i].Size != dest[i].Size) {
			return false;
		}

		int ret = memcmp(&src[i], &dest[i], src[i].Size);
		if(ret != 0) {
			return false;
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
void BasicTest()
{
	printf("run %s\n", __FUNCTION__);
	printf("sizeof(DogEvent)=%d\n", sizeof(DogEvent));

	CDOGSessionTest dogSession1;
	CDOGSessionTest dogSession2;

	CSessionBuffer::Version zeroVer = {0,0,0,0};
	CSessionBuffer::Version dogSessionVer1 = dogSession1.GetVersion();
	CSessionBuffer::Version dogSessionVer2 = dogSession2.GetVersion();

	BOOST_CHECK( 0 != memcmp(&zeroVer, &dogSessionVer1, sizeof(dogSessionVer1)) ); // must have a version number
	BOOST_CHECK( 0 == memcmp(&dogSessionVer2, &dogSessionVer1, sizeof(dogSessionVer1)) );


	boost::mutex mtx;
	const size_t numOfEvents = 100;

	class MyEventListener : public IEventListener
	{
	public:
		MyEventListener(size_t n) : expectedNumOfEvents(n)
		{
			receivedEvents.reserve(expectedNumOfEvents);
		}

		const size_t expectedNumOfEvents;
		std::vector<DogEvent> receivedEvents;
		boost::condition receivedAllCond;

		void STDCALL NotifyEvent(const DogEvent& event)
		{
			//printf("received event id=%d, time=%f \n", event.Id, event.TimeStamp.QuadPart/1e7);

			DogEvent evt = event;
			evt.ProcessId = 0;
			evt.ThreadId = 0;
			evt.TimeStamp.QuadPart = 0;
			receivedEvents.push_back(evt);
			if(receivedEvents.size() >= expectedNumOfEvents)
			{
				receivedAllCond.notify_all();
			}
		}
	};

	MyEventListener myEventListener(numOfEvents);

	// write events from session1, but receive events from session2

	dogSession2.SetupDogConfigByString("log = all");
	dogSession2.RegisterListener(&myEventListener);

	std::vector<DogEvent> myEvents;
	myEvents.resize(numOfEvents);
	for(size_t i=0; i<myEvents.size(); ++i)
	{
		myEvents[i].Id = i;
		myEvents[i].TimeStamp.QuadPart = 0;
		dogSession1.WriteEvent(myEvents[i]);
		//Sleep(1);
	}

	myEventListener.receivedAllCond.timed_wait(mtx, boost::posix_time::seconds(1));

	BOOST_CHECK( !myEventListener.receivedEvents.empty());
	if(!myEventListener.receivedEvents.empty())
	{
		int ret = memcmp(&myEvents[0], &myEventListener.receivedEvents[0], sizeof(DogEvent));
		BOOST_CHECK( CompareResult(myEvents, myEventListener.receivedEvents));
	}

}

void VersionCheckingTest()
{
	printf("run %s\n", __FUNCTION__);

	CDOGSessionTest dogSession1;
	CSessionBuffer::Version settingVer = {0xffff, 0xffff, 0xffff, 0xffff};
	dogSession1.SetVersion(settingVer);

	// session2 attachs the session buffer created by session1
	CDOGSessionTest dogSession2;

	CSessionBuffer::Version zeroVer = {0,0,0,0};
	CSessionBuffer::Version dogSessionVer1 = dogSession1.GetVersion();
	CSessionBuffer::Version dogSessionVer2 = dogSession2.GetVersion();

	BOOST_CHECK( 0 != memcmp(&zeroVer, &dogSessionVer1, sizeof(dogSessionVer1)) ); // must have a version number
	BOOST_CHECK( 0 == memcmp(&dogSessionVer2, &settingVer, sizeof(dogSessionVer1)) );

	boost::mutex mtx;
	const size_t numOfEvents = 100;

	class MyEventListener : public IEventListener
	{
	public:
		MyEventListener(size_t n) : expectedNumOfEvents(n)
		{
			receivedEvents.reserve(expectedNumOfEvents);
		}

		const size_t expectedNumOfEvents;
		std::vector<DogEvent> receivedEvents;
		boost::condition receivedAllCond;

		void STDCALL NotifyEvent(const DogEvent& event)
		{
			DogEvent evt = event;
			evt.ProcessId = 0;
			evt.ThreadId = 0;
			evt.TimeStamp.QuadPart = 0;
			receivedEvents.push_back(event);
			if(receivedEvents.size() >= expectedNumOfEvents)
			{
				receivedAllCond.notify_all();
			}
		}
	};

	MyEventListener myEventListener(numOfEvents);

	// write events from session1, but receive events from session2

	dogSession2.SetupDogConfigByString("log = all");
	dogSession2.RegisterListener(&myEventListener);

	std::vector<DogEvent> myEvents;
	myEvents.resize(numOfEvents);
	for(size_t i=0; i<myEvents.size(); ++i)
	{
		myEvents[i].Id = i;
		myEvents[i].TimeStamp.QuadPart = 0;
		dogSession1.WriteEvent(myEvents[i]);
	}

	myEventListener.receivedAllCond.timed_wait(mtx, boost::posix_time::seconds(1));

	// Because session buffer's version doesn't be supported by the CSessionBuffer,
	// The listener expects to receive no event.
	BOOST_CHECK( myEventListener.receivedEvents.empty());
}

void CustomEventTest()
{
	printf("run %s\n", __FUNCTION__);

	boost::mutex mtx;
	const size_t numOfEvents = 100;

	enum MyEventType
	{
		MY_EVENT_TYPE = 123,
	};

	struct MyEvent : public DogEvent
	{
		DWORD Data1;
		DWORD Data2;
		DWORD Data3;
		DWORD Data4;

		MyEvent() : DogEvent(0, 0, 0, MY_EVENT_TYPE, sizeof(MyEvent)), Data1(0), Data2(0) {}
	};

	class MyEventListener : public IEventListener
	{
	public:
		MyEventListener(size_t n) : expectedNumOfEvents(n)
		{
			receivedEvents.reserve(expectedNumOfEvents);
		}

		const size_t expectedNumOfEvents;
		std::vector<MyEvent> receivedEvents;
		boost::condition receivedAllCond;

		void STDCALL NotifyEvent(const DogEvent& event)
		{
			if(MY_EVENT_TYPE == event.DataType && event.Size == sizeof(MyEvent))
			{
				const MyEvent& myTypeEvent = (MyEvent&)(event);
				//printf("received event id=%d, data1=%d, data2=%x, time=%f\n", myTypeEvent.Id, myTypeEvent.Data1, myTypeEvent.Data2, myTypeEvent.TimeStamp.QuadPart/1e7);

				MyEvent evt = myTypeEvent;
				evt.ProcessId = 0;
				evt.ThreadId = 0;
				evt.TimeStamp.QuadPart = 0;
				receivedEvents.push_back(evt);
			}

			if(receivedEvents.size() >= expectedNumOfEvents)
			{
				receivedAllCond.notify_all();
			}
		}
	};

	MyEventListener myEventListener(numOfEvents);

	CDOGSession dogSession;
	dogSession.SetupDogConfigByString("log = all");
	dogSession.RegisterListener(&myEventListener);

	std::vector<MyEvent> myEvents;
	myEvents.resize(numOfEvents);
	for(size_t i=0; i<myEvents.size(); ++i)
	{
		myEvents[i].Id = i;
		myEvents[i].Data1 = i*2;
		myEvents[i].Data2 = 0xaabb;
		myEvents[i].Data3 = i*3;
		myEvents[i].Data4 = 0xffdd;
		myEvents[i].TimeStamp.QuadPart = 0;
		dogSession.WriteEvent(myEvents[i]);
	}

	myEventListener.receivedAllCond.timed_wait(mtx, boost::posix_time::seconds(1));

	BOOST_CHECK( CompareResult(myEvents, myEventListener.receivedEvents));
}

// test maximum event size
void StringEventTest()
{
	printf("run %s\n", __FUNCTION__);

	CDOGSessionTest dogSession1;
	CDOGSessionTest dogSession2;

	boost::mutex mtx;
	const size_t numOfEvents = 100;

	class MyStringEvent : public StringEvent
	{
	public:
		MyStringEvent() : StringEvent(0, 0, 0, "") {}
		MyStringEvent(const char* msg)  : StringEvent(0, 0, 0, msg) {}
		MyStringEvent(const wchar_t* msgW)  : StringEvent(0, 0, 0, msgW) {}
	};

	class MyEventListener : public IEventListener
	{
	public:
		MyEventListener(size_t n) : expectedNumOfEvents(n)
		{
			receivedEvents.reserve(expectedNumOfEvents);
		}

		const size_t expectedNumOfEvents;
		std::vector<MyStringEvent> receivedEvents;
		boost::condition receivedAllCond;

		void STDCALL NotifyEvent(const DogEvent& event)
		{
			switch(event.DataType)
			{
			case STRING_MESSAGE_EVENT_TYPE:
				{
					MyStringEvent& strEvent = (MyStringEvent&)(event);
					strEvent.ProcessId = 0;
					strEvent.ThreadId = 0;
					strEvent.TimeStamp.QuadPart = 0;
					receivedEvents.push_back(strEvent);
				}
				break;
			default:
				break;
			}

			if(receivedEvents.size() >= expectedNumOfEvents)
			{
				receivedAllCond.notify_all();
			}
		}
	};

	MyEventListener myEventListener(numOfEvents);

	// write events from session1, but receive events from session2

	dogSession2.SetupDogConfigByString("log = all");
	dogSession2.RegisterListener(&myEventListener);

	char strMsg1[] = "Test Message: MyStringEvent. Stress Test. ";
	char szMessage1[512];
	for(size_t i=0; i<sizeof(szMessage1); ++i)
	{
		szMessage1[i] = strMsg1[i%(sizeof(strMsg1)-1)];
	}
	szMessage1[sizeof(szMessage1)-1] = 0;

	wchar_t strMsg2[] = L"COREL Perf2 stress test. ";
	wchar_t szMessage2[256];
	for(size_t i=0; i<sizeof(szMessage2)/sizeof(wchar_t); ++i)
	{
		szMessage2[i] = strMsg2[i%(sizeof(strMsg2)/sizeof(wchar_t) -1)];
	}
	szMessage2[sizeof(szMessage2)/sizeof(wchar_t) -1] = 0;

	std::vector<MyStringEvent> myStrEvents;
	myStrEvents.resize(numOfEvents);
	for(size_t i=0; i<myStrEvents.size(); ++i)
	{
		if(0 == i%2)
			myStrEvents[i] = MyStringEvent(szMessage1);
		else
			myStrEvents[i] = MyStringEvent(szMessage2);

		myStrEvents[i].Id = i;
		myStrEvents[i].TimeStamp.QuadPart = 0;
		dogSession1.WriteEvent(myStrEvents[i]);
	}

	myEventListener.receivedAllCond.timed_wait(mtx, boost::posix_time::seconds(1));

	BOOST_CHECK( !myEventListener.receivedEvents.empty());
	if(!myEventListener.receivedEvents.empty())
	{
		int ret = memcmp(&myStrEvents[0], &myEventListener.receivedEvents[0], sizeof(MyStringEvent));
		BOOST_CHECK( CompareResult(myStrEvents, myEventListener.receivedEvents));
	}
}

DWORD GenSequence()
{
	static DWORD i=0;
	return i++;
}

void LargeEventTest()
{
	printf("run %s\n", __FUNCTION__);

	boost::mutex mtx;
	const size_t numOfEvents = 100;

	enum MyEventType
	{
		MY_EVENT_TYPE = 123,
	};

	struct MyLargeEvent : public DogEvent
	{
		DWORD Data[4096];
		MyLargeEvent() : DogEvent(0, 0, 0, MY_EVENT_TYPE, sizeof(MyLargeEvent)) {}
	};

	class MyEventListener : public IEventListener
	{
	public:
		MyEventListener(size_t n) : expectedNumOfEvents(n)
		{
			receivedEvents.reserve(expectedNumOfEvents);
		}

		const size_t expectedNumOfEvents;
		std::vector<MyLargeEvent> receivedEvents;
		boost::condition receivedAllCond;

		void STDCALL NotifyEvent(const DogEvent& event)
		{
			if(MY_EVENT_TYPE == event.DataType && event.Size == sizeof(MyLargeEvent))
			{
				const MyLargeEvent& myTypeEvent = (MyLargeEvent&)(event);
				//printf("received event id=%d, data1=%d, data2=%x, time=%f\n", myTypeEvent.Id, myTypeEvent.Data1, myTypeEvent.Data2, myTypeEvent.TimeStamp.QuadPart/1e7);

				MyLargeEvent evt = myTypeEvent;
				evt.ProcessId = 0;
				evt.ThreadId = 0;
				evt.TimeStamp.QuadPart = 0;
				receivedEvents.push_back(evt);
			}

			if(receivedEvents.size() >= expectedNumOfEvents)
			{
				receivedAllCond.notify_all();
			}
		}
	};

	MyEventListener myEventListener(numOfEvents);

	CDOGSession dogSession;
	dogSession.SetupDogConfigByString("log = all");
	dogSession.RegisterListener(&myEventListener);

	std::vector<MyLargeEvent> myEvents;
	myEvents.resize(numOfEvents);
	for(size_t i=0; i<myEvents.size(); ++i)
	{
		myEvents[i].Id = i;
		myEvents[i].TimeStamp.QuadPart = 0;

		const int N = sizeof(myEvents[i].Data)/sizeof(DWORD);

		switch(i%4)
		{
		case 0:
		case 1:
		case 2:
			std::generate(myEvents[i].Data, myEvents[i].Data+N, GenSequence);
			break;

		case 3:
			std::generate(myEvents[i].Data, myEvents[i].Data+N, rand);
			break;
		}

		dogSession.WriteEvent(myEvents[i]);
	}

	myEventListener.receivedAllCond.timed_wait(mtx, boost::posix_time::seconds(1));

	BOOST_CHECK( CompareResult(myEvents, myEventListener.receivedEvents));
}


inline std::string VariantToString(Variant var)
{
	switch(var.type)
	{
	case Variant::VT_INT:       return (boost::format("VT_INT %d") % var.iVal).str();
	case Variant::VT_UINT:      return (boost::format("VT_UINT %d") % var.uiVal).str();
	case Variant::VT_LONGLONG:  return (boost::format("VT_LONGLONG %d") % var.llVal).str();
	case Variant::VT_ULONGLONG: return (boost::format("VT_ULONGLONG %d") % var.ullVal).str();
	case Variant::VT_FLOAT:     return (boost::format("VT_FLOAT %f") % var.fVal).str();
	case Variant::VT_DOUBLE:    return (boost::format("VT_DOUBLE %f") % var.dblVal).str();
	default:
		return "";
	}
	return "";
}

void UserEventTest()
{
	printf("run %s\n", __FUNCTION__);

	boost::mutex mtx;

	class MyEventListener : public IEventListener
	{
	public:
		size_t m_nEventCount;
		boost::condition receivedAllCond;

		MyEventListener(): m_nEventCount(0) {}

		void STDCALL NotifyEvent(const DogEvent& event)
		{
			++m_nEventCount;

			switch(event.DataType)
			{
			case STRING_MESSAGE_EVENT_TYPE:
				{
					const StringEvent& strEvent = (StringEvent&)(event);

					if(1 == strEvent.nBytesOfChar) {
						printf("received StringEvent: time=%f, %s \n", event.TimeStamp.QuadPart/1e7, strEvent.szMessage);
					} else  {
						wprintf(L"received StringEvent: time=%f, %s \n", event.TimeStamp.QuadPart/1e7, strEvent.szMessageW);
					}
				}
				break;
			case VARIANT_DATA_EVENT_TYPE:
				if(sizeof(VariantDataEvent) == event.Size)
				{
					const VariantDataEvent& varEvent = (VariantDataEvent&)(event);
					printf("received VariantDataEvent: time=%f, data=%s \n", event.TimeStamp.QuadPart/1e7, VariantToString(varEvent.Data).c_str());
				}
				break;
			case DATA_PROCESS_EVENT_TYPE:
			case BUFFER_EVENT_TYPE:
				if(sizeof(DataProcessEvent) == event.Size)
				{
					const DataProcessEvent& bufEvent = (DataProcessEvent&)(event);
					char* szOperation = "UPDATE_STATUS";
					switch(bufEvent.Operation)
					{
					default:
					case DataProcessEvent::UPDATE_STATUS:  szOperation = "UPDATE_STATUS"; break;
					case DataProcessEvent::DATA_IN: 	szOperation = "DATA_IN";  break;
					case DataProcessEvent::DATA_OUT: szOperation = "DATA_OUT"; break;
					}
					printf("received BufferEvent: buf=%d, (%d/%d), op=%s, PTS=%f, duration=%f, size=%d\n", bufEvent.Id, bufEvent.BufferSize, bufEvent.Capacity,
						szOperation, (bufEvent.DataPTS/1e7), (bufEvent.DataDuration/1e7), bufEvent.DataSize);
				}
				break;
			default:
				break;
			}

			if(7 == m_nEventCount)
			{
				receivedAllCond.notify_all();
			}
		}
	};

	MyEventListener myEventListener;

	CDOGSession dogSession;
	dogSession.SetupDogConfigByString("log = all");
	dogSession.RegisterListener(&myEventListener);

	dogSession.WriteEvent(DebugMessageEvent(0,0, "HelloDog"));
	dogSession.WriteEvent(DebugMessageEvent(0,0, (boost::format("StringEvent Test %d") % 100).str().c_str()));
	dogSession.WriteEvent(DebugMessageEvent(0,0, L"WCHAR: HelloDog"));
	dogSession.WriteEvent(VariantDataEvent(0,0, 123, Variant(0xff)));
	dogSession.WriteEvent(VariantDataEvent(0,0, 123, Variant(3.14159f)));
	dogSession.WriteEvent(DataProcessEvent(0,0, 1, DataProcessEvent::DATA_IN,  1e7, (1536.0f/48000)*1e7, 1536*4, 1536*4, 1920000));
	dogSession.WriteEvent(DataProcessEvent(0,0, 1, DataProcessEvent::DATA_OUT, 1e7, (1536.0f/48000)*1e7, 1536*4, 0, 1920000));

	myEventListener.receivedAllCond.timed_wait(mtx, boost::posix_time::seconds(1));
}

void SessionBandwidthTest()
{
	printf("run %s\n", __FUNCTION__);

	boost::mutex mtx;
	const size_t numOfEvents = 64 * (16*1024);

	enum MyEventType
	{
		MY_EVENT_TYPE = 123,
	};

	struct MyLargeDataEvent : public DogEvent
	{
		//static const size_t ReservedSize = 64;
		BYTE Data[512-sizeof(DogEvent)-64];

		MyLargeDataEvent() : DogEvent(0, 0, 0, MY_EVENT_TYPE, sizeof(MyLargeDataEvent)) {}
	};

	class MyEventListener : public IEventListener
	{
		CFreqTimer m_FreqTimer;
	public:
		DWORD m_NumOfReceivedEvents;
		LONGLONG m_TimeWhenReceivedAll;

		MyEventListener(size_t n) : expectedNumOfEvents(n), m_NumOfReceivedEvents(0), m_TimeWhenReceivedAll(0)
		{}

		const size_t expectedNumOfEvents;
		boost::condition receivedAllCond;

		void STDCALL NotifyEvent(const DogEvent& event)
		{
			if(MY_EVENT_TYPE == event.DataType && event.Size == sizeof(MyLargeDataEvent))
			{
				const MyLargeDataEvent& myTypeEvent = (MyLargeDataEvent&)(event);
				++m_NumOfReceivedEvents;
			}

			if(m_NumOfReceivedEvents >= expectedNumOfEvents)
			{
				m_TimeWhenReceivedAll =  m_FreqTimer.Now();
				receivedAllCond.notify_all();
			}
		}
	};

	MyEventListener myEventListener(numOfEvents);

	CDOGSession dogSession;
	dogSession.SetupDogConfigByString("log = all");
	dogSession.RegisterListener(&myEventListener);

	//SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

	CFreqTimer m_FreqTimer;
	const LONGLONG startT = m_FreqTimer.Now();

	MyLargeDataEvent myEvent;
	for(size_t i=0; i<numOfEvents; ++i)
	{
		myEvent.Id = i;
		myEvent.TimeStamp.QuadPart = 0;
		dogSession.WriteEvent(myEvent);
	}

	const LONGLONG timeOfSendEnd = m_FreqTimer.Now();
	DWORD nSendRate = sizeof(myEvent.Data)*numOfEvents / ((timeOfSendEnd-startT)/1e7);

	const DWORD nReceivedEvents = myEventListener.m_NumOfReceivedEvents;
	const LONGLONG timeOfRecvEnd = myEventListener.m_TimeWhenReceivedAll!=0 ? myEventListener.m_TimeWhenReceivedAll : m_FreqTimer.Now();
	DWORD nRecvRate = sizeof(myEvent.Data)*nReceivedEvents / ((timeOfRecvEnd-startT)/1e7);

	myEventListener.receivedAllCond.timed_wait(mtx, boost::posix_time::seconds(2));

	printf("send=%d (bytes/sec) = %f (MB/sec)\n", nSendRate, nSendRate/1e6);
	printf("recv=%d (bytes/sec) = %f (MB/sec)\n", nRecvRate, nRecvRate/1e6);
	printf(__FUNCTION__": miss count=%d\n", dogSession.GetMissCount());
	//BOOST_CHECK(0 == dogSession.GetMissCount());
}

void TimerTest()
{
	CFreqTimer m_FreqTimer;
	CRdtscTimer m_RdtscTimer;
	const size_t count = 1000000;

	const LONGLONG startFreq = m_FreqTimer.Now();
	for(size_t i=0; i<count; ++i)
	{
		m_FreqTimer.Now();
	}
	const LONGLONG endFreq = m_FreqTimer.Now();
	const double fFreqTimerPerCall = (double)(endFreq - startFreq) / count;

	const LONGLONG startRdtsc = m_FreqTimer.Now();
	for(size_t i=0; i<count; ++i)
	{
		m_RdtscTimer.Now();
	}
	const LONGLONG endRdtsc = m_FreqTimer.Now();
	const double fRdtscTimerPerCall = (double)(endRdtsc - startRdtsc) / count;

	const LONGLONG startTGT = m_FreqTimer.Now();
	for(size_t i=0; i<count; ++i)
	{
		timeGetTime();
	}
	const LONGLONG endTGT = m_FreqTimer.Now();
	const double fTGTPerCall = (double)(endTGT - startTGT) / count;

	printf("Timer performance comparison:\n"
		   "CFreqTimer takes %f (100-nano) per call\n"
		   "CRdtscTimer takes %f (100-nano) per call\n"
		   "timeGetTime takes %f (100-nano) per call\n"
		   ,fFreqTimerPerCall, fRdtscTimerPerCall, fTGTPerCall);
}

// RdtscTimerTest
// - Test if CRdtscTimer is mono-increased
void RdtscTimerTest()
{
	printf("run %s\n", __FUNCTION__);
	CFreqTimer freqTimer;
	LONGLONG elapsePreT = 0;

	CRdtscTimer rdtscTimer;
	LONGLONG preT = 0;
	for(;;)
	{
		LONGLONG nowT = rdtscTimer.Elapse();
		if(!(nowT >= preT))
		{
			printf("RdtscTimerTest Error: nowT (%f) < preT (%f)\n", nowT/1e7, preT/1e7);
		}
		preT = nowT;

		LONGLONG elapseNowT = freqTimer.Elapse();
		if(elapseNowT - elapsePreT > 10000000)
		{
			elapsePreT = elapseNowT;
			printf("RdtscTimerTest Elapse %d sec RDTSC=%f\r", (int)(elapseNowT/1e7), nowT/1e7);
		}
		Sleep(10);
	}
}


} // namespace DogEventTest

//============================================================================
boost::unit_test::test_suite* make_DogEventTest_suite()
{
	using namespace DogSessionTest;

	boost::unit_test_framework::test_suite* suite = NULL;
	suite = BOOST_TEST_SUITE("DogSessionTest");
	{
		suite->add( BOOST_TEST_CASE( BasicTest ));
		suite->add( BOOST_TEST_CASE( VersionCheckingTest ));
		suite->add( BOOST_TEST_CASE( CustomEventTest ));
		suite->add( BOOST_TEST_CASE( LargeEventTest ));
		suite->add( BOOST_TEST_CASE( StringEventTest ));
		suite->add( BOOST_TEST_CASE( UserEventTest ));

		suite->add( BOOST_TEST_CASE( TimerTest ));

//		suite->add( BOOST_TEST_CASE( SessionBandwidthTest ));

//		suite->add( BOOST_TEST_CASE( RdtscTimerTest ));
	}
	return suite;
}
