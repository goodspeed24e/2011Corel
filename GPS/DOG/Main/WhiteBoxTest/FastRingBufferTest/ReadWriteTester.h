#pragma once

#include "Logger.h"
#include <tchar.h>

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/format.hpp>


namespace FastRingBufferTest {


class CWriteTester
{
public:
	CWriteTester() : m_Id(0), m_pLogger(NULL)
	{}

	void Setup(int id, size_t nLoop, CLogger* logger)
	{
		m_Id = id;
		m_nLoopCount = nLoop;
		m_pLogger = logger;
	}

	void GenerateOutputData()
	{
		for(size_t i=0; i<m_nLoopCount; ++i)
		{
			std::string str = (boost::format("[writer=%d][msgIdx=%d] Write Test Message\n") % m_Id % i).str();
			if(m_pLogger) {
				m_pLogger->Output(str.c_str());
			}
		}
	}

	void Start()
	{
		if(NULL == m_pThread) {
			m_pThread = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&CWriteTester::GenerateOutputData, this)));
		}
	}

	void Stop()
	{
		if(m_pThread) {
			m_pThread->join();
		}
	}

protected:
	int m_Id;
	size_t m_nLoopCount;
	CLogger* m_pLogger;
	boost::shared_ptr<boost::thread> m_pThread;
};


class CReadTester
{
public:
	CReadTester() : m_Id(0), m_pLogger(NULL), m_bRunFlag(false), m_nTotalCount(0), m_CurIndex(0), m_bIncrementalOrder(true)
	{}

	void Setup(int id, CLogger* logger)
	{
		m_Id = id;
		m_pLogger = logger;
		m_nTotalCount = 0;
	}

	bool PopMessage()
	{
		if(m_pLogger)
		{
			const int previousIndex = m_CurIndex;

			CLogger::LogEntry msg;
			bool ret = m_pLogger->Receive(msg);
			if(ret)
			{
				m_CurIndex = msg.dwIndex;

				InterlockedIncrement(&m_nTotalCount);
				m_bIncrementalOrder &= (previousIndex < m_CurIndex);
				if(!(previousIndex < m_CurIndex)) {
					printf("[DEBUG]: previousIndex=%d, m_CurIndex=%d\n", previousIndex, m_CurIndex);
				}

				std::string str = (boost::format("%d:time=%f:%s") % msg.dwIndex % ((float)msg.dwTime/1e3) % msg.szMessage).str();
				//printf(str.c_str());
				OutputDebugStringA(str.c_str());
				return true;
			}
		}
		return false; // no data available
	}

	void MainLoop()
	{
		int nPreviousIndex = 0;
		while(m_bRunFlag)
		{
			bool ret = PopMessage();
			if(!ret) {
				boost::this_thread::sleep(boost::posix_time::milliseconds(1));
			}
		}
	}

	void Start()
	{
		m_bRunFlag = true;
		if(NULL == m_pThread) {
			m_pThread = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&CReadTester::MainLoop, this)));
		}
	}

	void Stop()
	{
		m_bRunFlag = false;
		if(m_pThread) {
			m_pThread->join();
		}
	}

	LONG GetTotalCount() { return m_nTotalCount; }
	bool CheckIncrementalOrder() { return m_bIncrementalOrder; }

protected:
	int m_Id;
	CLogger* m_pLogger;
	boost::shared_ptr<boost::thread> m_pThread;
	bool m_bRunFlag;

	LONG m_nTotalCount;	// total message count
	int  m_CurIndex;
	bool m_bIncrementalOrder;
};



} // namespace FastRingBufferTest
