#pragma once

#include "DOG.h"
#include "DogUserEvent.h"
#include "AsyncCircularBuffer.h"
#include "EventCollection.h"
#include <map>
#include <set>

#include <windows.h>

/**
 * CEventListener listens interested events and push them into circular buffers.
 */
class CEventListener : public CEventCollection, public dog::IEventListener
{
public:
	CEventListener() : m_StartTime(0)
	{
		InitializeCriticalSection(&m_cs);
	}
	~CEventListener()
	{
		DeleteCriticalSection(&m_cs);
	}

	void STDCALL NotifyEvent(const dog::DogEvent& event)
	{
		using namespace dog;
		{
			CAutoLock loc(&m_cs);
			if(0 == m_StartTime)
			{
				m_StartTime = event.TimeStamp.QuadPart;
			}

			const TInterestedTable::const_iterator iter = m_InterestedTable.find(event.Id);
			if(iter == m_InterestedTable.end())
			{
				// not register in interested event table
				return;
			}
		}

		event.BaseTime.QuadPart = m_StartTime;
		// push event into CEventCollection
		Push(event);
	}

	void AddInterestedEventId(DWORD eventId)
	{
		CAutoLock loc(&m_cs);
		const TInterestedTable::const_iterator iter = m_InterestedTable.find(eventId);
		if(iter == m_InterestedTable.end())
		{
			m_InterestedTable[eventId] = 1;
		}
		else
		{
			++m_InterestedTable[eventId];
		}
	}

	void RemoveInterestedEventId(DWORD eventId)
	{
		bool bRemoveBuffer = false;
		{
			CAutoLock loc(&m_cs);
			const TInterestedTable::const_iterator iter = m_InterestedTable.find(eventId);
			if(iter != m_InterestedTable.end())
			{
				int refCount = m_InterestedTable[eventId];
				--refCount;
				if(refCount <= 0)
				{
					m_InterestedTable.erase(eventId);
					bRemoveBuffer = true;
				}
				else
				{
					m_InterestedTable[eventId] = refCount;
				}
			}
		}

		if(bRemoveBuffer)
		{
			Erase(eventId);
		}
	}

	void ClearInterestedTable()
	{
		{
			CAutoLock loc(&m_cs);
			m_InterestedTable.clear();
		}
		Clear();
	}

protected:
	mutable CRITICAL_SECTION m_cs;
	LONGLONG m_StartTime; // Timestamp of the first event

	class CAutoLock
	{
		CRITICAL_SECTION* m_pcs;
	public:
		CAutoLock(CRITICAL_SECTION* cs) : m_pcs(cs) { EnterCriticalSection(m_pcs); }
		~CAutoLock() { LeaveCriticalSection(m_pcs); }
	};


	///  Interested event table, which records interested event Id.
	typedef std::map<DWORD, int> TInterestedTable;
	TInterestedTable m_InterestedTable;
};

