#pragma once

#include "DOG.h"
#include "SessionBuffer.h"
#include "ThreadTemplate.h"

#include <list>
#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>

// namespace Diagnosis-of-GPS
namespace dog
{

/**
 * CListenThread Pops data from session buffer, and dispatch them to listeners.
 */
class CListenThread : protected TThread< CListenThread >, public boost::noncopyable
{
	friend class TThread< CListenThread >;
public:
	CListenThread(CSessionBuffer& refSessionBuf) : m_refSessionBuf(refSessionBuf)
	{
		InitializeCriticalSection(&m_cs);
	}

	~CListenThread()
	{
		HANDLE hThread = StopThread();
		if(hThread) {
			WaitForSingleObject(hThread, INFINITE);
			CloseHandle(hThread);
		}

		DeleteCriticalSection(&m_cs);
	}

	void Start()   { StartThread(); }
	HANDLE Stop()  { return StopThread();  }

	/// Register a dog event listener
	void RegisterListener(IEventListener* listener);

	/// Remove the specified dog event listener
	void UnregisterListener(IEventListener* listener);

	/// Get number of registered listener
	size_t GetNumOfListener();

protected:
	CRITICAL_SECTION m_cs;

	// Thread's main body
	void Run();

	CSessionBuffer& m_refSessionBuf;
	std::list<IEventListener*> m_ListenerList;

	class TLargePack;
	boost::shared_ptr<TLargePack> m_pLargePackBuf;

	void CollectPackects(const CSessionBuffer::TDataPacket& pack);
	void NotifyLargePackEvent(LONGLONG transport_id);


	/// Automatically locking and unlocking of a critical section.
	class CAutoLock
	{
		CRITICAL_SECTION* m_cs;
	public:
		CAutoLock(CRITICAL_SECTION* cs) : m_cs(cs) { EnterCriticalSection(m_cs); }
		~CAutoLock() { LeaveCriticalSection(m_cs); }
	};
};


} // namespace dog

