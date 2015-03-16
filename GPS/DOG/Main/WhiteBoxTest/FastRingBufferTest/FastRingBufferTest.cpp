#pragma warning (disable:4819)
//boost_1_35_0\include\boost\utility\enable_if.hpp : warning C4819: The file contains a character that cannot be represented in the current code page (950). Save the file in Unicode format to prevent data loss

#include "Logger.h"
#include <tchar.h>

#include <boost/test/unit_test.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/format.hpp>

#include "ReadWriteTester.h"


namespace FastRingBufferTest {


class CFreqTimer
{
public:
	CFreqTimer() { QueryPerformanceFrequency(&m_liPerfFreq); }

	/// Returns the current time in milliseconds
	DWORD now() const
	{
		LARGE_INTEGER liPerfNow;
		QueryPerformanceCounter(&liPerfNow);
		return(DWORD)(((liPerfNow.QuadPart) * 1000) / m_liPerfFreq.QuadPart);
	}

	/// Returns the current time in seconds
	double nowInSec() const
	{
		return (double)nowInMicro() / 1e6;
	}

	/// Returns the current time in microseconds
	LONGLONG nowInMicro() const
	{
		LARGE_INTEGER liPerfNow;
		QueryPerformanceCounter(&liPerfNow);
		return(((liPerfNow.QuadPart) * 1000000) / m_liPerfFreq.QuadPart);
	}

	LARGE_INTEGER m_liPerfFreq;   // Counts per second
};

//-----------------------------------------------------------------------------

// Check basic features
void BasicTest()
{
	CLogger Logger;
	Logger.Create(_T("BasicTest"));

	const size_t nSizeOfLogMsg = 512;
	char logMsg[][nSizeOfLogMsg] = {
		"Test Message 1",
		"Test Message 2",
		"Test Message 3",
		"Test Message 4",
		"Test Message 5",
	};
	const int nNumOfLogMsg = sizeof(logMsg)/nSizeOfLogMsg;

	for(int i=0; i<nNumOfLogMsg; ++i)
	{
		Logger.Output(logMsg[i]);
	}

	CLogger::LogEntry msg;

	CLogger receiver;
	receiver.Open(_T("BasicTest"));
	bool ret;

	for(int i=0; i<nNumOfLogMsg; ++i)
	{
		ret = receiver.Receive(msg);
		BOOST_CHECK(msg.dwIndex == i+1);
		BOOST_CHECK(ret);
		BOOST_CHECK(strcmp(msg.szMessage, logMsg[i]) == 0);
	}

	// check if there is no more message
	ret = receiver.Receive(msg);
	BOOST_CHECK(ret == false);

	receiver.Close();
	Logger.Close();
}

//-----------------------------------------------------------------------------
// Check behavior in case of multiple writing threads
// - check all data is received by a reader
void MultipleWritingThreadsTest()
{
	CLogger Logger;
	Logger.Create(_T("MultipleWritingThreadsTest"));
	const size_t nSizeOfArray = Logger.GetSizeOfArray();


	const size_t NumOfWriters = 20; // number of writing threads

	// This test case is only intended to test the simple case of multiple writing threads,
	// so we constrain the total number of messages to avoid buffer run out
	const size_t NumOfWriteLoop = nSizeOfArray / NumOfWriters;


	CReadTester  Readers;
	CWriteTester Writers[NumOfWriters];

	Readers.Setup(0, &Logger);
	Readers.Start();


	CFreqTimer timer;
	LONGLONG _startTime = timer.nowInMicro();

	for(int i=0; i<NumOfWriters; ++i)
	{
		Writers[i].Setup(i, NumOfWriteLoop, &Logger);
		Writers[i].Start();
	}

	for(int i=0; i<NumOfWriters; ++i) {
		Writers[i].Stop();
	}

	LONGLONG _endTime = timer.nowInMicro();
	//printf("Writing elapse = %f\n", (double)(_endTime - _startTime)/1e6);


	// wait for reader to receive all messages
	int time_out = 6000;
	while(time_out>0)
	{
		boost::this_thread::sleep(boost::posix_time::milliseconds(100));
		time_out -= 100;

		if(NumOfWriters * NumOfWriteLoop < (size_t)Readers.GetTotalCount())
		{
			break;
		}
	}
	Readers.Stop();


	// - check all data is received by a reader
	BOOST_CHECK(NumOfWriters * NumOfWriteLoop == (size_t)Readers.GetTotalCount());

	// - check the the order of output messages is kept
	//BOOST_CHECK(Readers.CheckIncrementalOrder()); // remove this check because CLogger::Output() is not an atomic operation

	Logger.Close();
}

//-----------------------------------------------------------------------------
// Check behavior in case of multiple reading and writing threads
// - check all data is received by a reader
void MultipleReadWriteThreadsTest()
{
	CLogger Logger;
	Logger.Create(_T("MultipleReadWriteThreadsTest"));
	const size_t nSizeOfArray = Logger.GetSizeOfArray();

	const size_t NumOfReaders = 20; // number of reading threads
	const size_t NumOfWriters = 20; // number of writing threads

	// This test case is only intended to test the simple case of multiple writing threads,
	// so we constrain the total number of messages to avoid buffer run out
	const size_t NumOfWriteLoop = nSizeOfArray / NumOfWriters;


	CReadTester  Readers[NumOfReaders];
	CWriteTester Writers[NumOfWriters];


	for(int i=0; i<NumOfReaders; ++i)
	{
		Readers[i].Setup(i, &Logger);
		Readers[i].Start();
	}


	CFreqTimer timer;
	LONGLONG _startTime = timer.nowInMicro();

	for(int i=0; i<NumOfWriters; ++i)
	{
		Writers[i].Setup(i, NumOfWriteLoop, &Logger);
		Writers[i].Start();
	}

	for(int i=0; i<NumOfWriters; ++i) {
		Writers[i].Stop();
	}

	LONGLONG _endTime = timer.nowInMicro();
	//printf("Writing elapse = %f\n", (double)(_endTime - _startTime)/1e6);


	boost::this_thread::sleep(boost::posix_time::milliseconds(6000));

	size_t nTotalReceived = 0;
	bool   bCheckIncrementalOrder = true;

	for(int i=0; i<NumOfReaders; ++i) 
	{
		Readers[i].Stop();
		nTotalReceived += Readers[i].GetTotalCount();
		bCheckIncrementalOrder &= Readers[i].CheckIncrementalOrder();
	}

	// - check all data is received by a reader
	BOOST_CHECK(NumOfWriters * NumOfWriteLoop == nTotalReceived);

	// - check the the order of output messages is kept
	//BOOST_CHECK(bCheckIncrementalOrder); // remove this check because CLogger::Output() is not an atomic operation

	Logger.Close();
}

//-----------------------------------------------------------------------------
// Writer run out buffer and causes skipping data when reader try to get data
// - check missing count
void BufferRunoutTest()
{
	CLogger Logger;
	Logger.Create(_T("MultipleReadWriteThreadsTest"));
	const size_t nSizeOfArray = 20;
	Logger.SetSizeOfArray(nSizeOfArray);

	const int nNumOfLogMsg = nSizeOfArray * 1.5f;
 	const size_t nSizeOfLogMsg = 512;
 	char logMsg[nNumOfLogMsg][nSizeOfLogMsg];

	for(int i=0; i<nNumOfLogMsg; ++i)
	{
		sprintf_s(logMsg[i], sizeof(logMsg[i]), "Test Message %d", i);
		Logger.Output(logMsg[i]);
	}

	CLogger::LogEntry msg;
	bool ret;

	ret = Logger.Receive(msg);
	DWORD nWriteIndex = Logger.GetWriteIndex();
	DWORD nMissCount = Logger.GetMissCount();
	DWORD nReadIndex = Logger.GeReadIndex();
	BOOST_CHECK(ret == false);
	BOOST_CHECK(nWriteIndex == nNumOfLogMsg);
	BOOST_CHECK(nMissCount == nNumOfLogMsg-nSizeOfArray);	
	BOOST_CHECK(nReadIndex == nWriteIndex-nSizeOfArray);

	ret = Logger.Receive(msg);
	BOOST_CHECK(ret);

	Logger.Close();
}

//-----------------------------------------------------------------------------
// Writer run out buffer and causes skipping data when reader try to get data
// - check if TFastRingBuffer update data correctly in multiple threads 
// - check missing count
void BufferRunoutAndMultipleReaderThreadsTest()
{
	const size_t NumOfReaders = 20; // number of reading threads

	CLogger Logger;
	Logger.Create(_T("MultipleReadWriteThreadsTest"));
	const size_t nSizeOfArray = 20;
	Logger.SetSizeOfArray(nSizeOfArray);

	const int nNumOfLogMsg = nSizeOfArray * 1.5f;
	const size_t nSizeOfLogMsg = 512;
	char logMsg[nNumOfLogMsg][nSizeOfLogMsg];

	for(int i=0; i<nNumOfLogMsg; ++i)
	{
		sprintf_s(logMsg[i], sizeof(logMsg[i]), "Test Message %d", i);
		Logger.Output(logMsg[i]);
	}

	CReadTester  Readers[NumOfReaders];

	for(int i=0; i<NumOfReaders; ++i)
	{
		Readers[i].Setup(i, &Logger);
		Readers[i].Start();
	}

	boost::this_thread::sleep(boost::posix_time::milliseconds(1000));

	for(int i=0; i<NumOfReaders; ++i) 
	{
		Readers[i].Stop();
	}

	DWORD nWriteIndex = Logger.GetWriteIndex();
	DWORD nMissCount = Logger.GetMissCount();	
	DWORD nReadIndex = Logger.GeReadIndex();

	BOOST_CHECK(nWriteIndex == nNumOfLogMsg);
	BOOST_CHECK(nMissCount == nWriteIndex-nSizeOfArray);	
	BOOST_CHECK(nReadIndex == nNumOfLogMsg);

	Logger.Close();
}


} // namespace FastRingBufferTest

//============================================================================
boost::unit_test::test_suite* make_FastRingBufferTest_suite()
{
	using namespace FastRingBufferTest;

	boost::unit_test_framework::test_suite* suite = NULL;
	suite = BOOST_TEST_SUITE("FastRingBufferTest");
	{
		suite->add( BOOST_TEST_CASE( BasicTest ));
		suite->add( BOOST_TEST_CASE( MultipleWritingThreadsTest ));
		suite->add( BOOST_TEST_CASE( MultipleReadWriteThreadsTest ));
		suite->add( BOOST_TEST_CASE( BufferRunoutTest ));
		suite->add( BOOST_TEST_CASE( BufferRunoutAndMultipleReaderThreadsTest ));

	}
	return suite;
}
