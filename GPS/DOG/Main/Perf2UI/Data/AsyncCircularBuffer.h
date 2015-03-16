#pragma once

#include "CircularBuffer.h"
#include <windows.h>

template <typename EventT>
class TAsyncCircularBuffer : public TCircularBuffer<EventT>
{
public:
	TAsyncCircularBuffer(size_t qsize) : TCircularBuffer(qsize)
	{
		InitializeCriticalSection(&m_cs);
	}
	~TAsyncCircularBuffer() 
	{
		DeleteCriticalSection(&m_cs);
	}

	const EventT& Front() const
	{
		CAutoLock loc(&m_cs); 
		return TCircularBuffer::Front();
	}

	const EventT& operator[](int idx) const
	{
		CAutoLock loc(&m_cs);  
		return TCircularBuffer::operator[](idx);
	}

	void Push(const EventT& val)
	{
		CAutoLock loc(&m_cs);
		TCircularBuffer::Push(val);
	}

	void Pop()
	{
		CAutoLock loc(&m_cs);
		TCircularBuffer::Pop();
	}

	size_t GetCapacity() const
	{
		CAutoLock loc(&m_cs);
		return TCircularBuffer::GetCapacity(val);
	}

	size_t GetSize() const
	{
		CAutoLock loc(&m_cs);
		return TCircularBuffer::GetSize();
	}

	void Clear()
	{
		CAutoLock loc(&m_cs);
		TCircularBuffer::Clear();
	}



protected:
	mutable CRITICAL_SECTION m_cs;

	class CAutoLock
	{
		CRITICAL_SECTION* m_pcs;
	public:
		CAutoLock(CRITICAL_SECTION* cs) : m_pcs(cs) { EnterCriticalSection(m_pcs); }
		~CAutoLock() { LeaveCriticalSection(m_pcs); }
	};
};

