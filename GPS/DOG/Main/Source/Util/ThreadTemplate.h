#ifndef _Thread_Template_H_
#define _Thread_Template_H_

#include <windows.h>

// namespace Diagnosis-of-GPS
namespace dog {

/**
 * Template TThread encapsulates a Windows thread and is intended to be derived.
 * A class derived from the template TThread must provide Run() function, which
 * will be invoked in ThreadProc() function of the template TThread.
 */
template< typename _DERIVED_ >
class TThread
{
public:
	TThread() : m_hThread(NULL), m_ThreadId(0), m_bRunFlag(0)
	{
		m_hEvent = CreateEvent(0, FALSE, FALSE, 0);
	}

	~TThread()
	{
		HANDLE hThread = StopThread();
		if(hThread) {
			WaitForSingleObject(hThread, INFINITE);
			CloseHandle(hThread);
		}
		if(m_hEvent) {
			CloseHandle(m_hEvent);
			m_hEvent = NULL;
		}
	}

	/// Starts thread
	/// If the thread is started, calling StartThread() has no effect.
	void StartThread()
	{
		if(NULL == m_hThread)
		{
			_DERIVED_* self = static_cast<_DERIVED_*>(this);
			InterlockedIncrement(&m_bRunFlag);
			m_hThread = CreateThread(NULL, 0, ThreadProc, self, 0, NULL);
		}
	}

	/// Stops thread.
	/// If the thread is stopped, calling StopThread() has no effect.
	/// @return : thread handle
	HANDLE StopThread()
	{
		HANDLE hThread = NULL;
		if(m_hThread)
		{
			InterlockedDecrement(&m_bRunFlag);
			hThread = m_hThread;
			m_hThread = NULL;
			SetEvent(m_hEvent);
		}
		return hThread;
	}

	/// Returns if the thread is running
	bool IsThreadActive() const { return NULL != m_hThread; }

	/// A class derived from the template TThread must provide Run() function,
	/// which will be invoked in ThreadProc()
	//
	// void Run()
	// {
	//     while( 0 != m_bRunFlag )
	//     {
	//         WaitForSingleObject(m_hEvent, INFINITE);
	//         Sleep(100);
	//     }
	// }
	//
	static DWORD WINAPI ThreadProc(LPVOID pParam)
	{
		_DERIVED_* self = static_cast<_DERIVED_*>(pParam);
		self->m_ThreadId = GetCurrentThreadId();
		self->Run();
		return 0;
	}

protected:
	HANDLE m_hThread;
	DWORD  m_ThreadId;
	HANDLE m_hEvent;
	LONG m_bRunFlag;

	// disable copy constructor and operator assignment
	TThread(const TThread&) {}
	TThread& operator = (const TThread&) {}
};


} //namespace dog

#endif //_Thread_Template_H_
