#ifndef _TCircularBuffer_Template_H_
#define _TCircularBuffer_Template_H_

#include <windows.h>
#include <boost/utility.hpp>

/**
 * Template TCircularBuffer implements a fast FIFO queue. (Non-thread-safe implementation)
 * Pushing data to a full queue will force queue to pop it's front element.
 */
template <typename EventT>
class TCircularBuffer : boost::noncopyable
{
public:
	typedef EventT value_type;

	TCircularBuffer(size_t qsize) : m_nQueueSize(qsize), m_FrontIdx(0), m_Count(0)
	{
		const size_t nSize = m_nQueueSize*sizeof(EventT);
		m_Container = (EventT*)malloc(nSize);
		memset(m_Container, 0, nSize);
	}

	~TCircularBuffer()
	{
		free(m_Container);
	}


	/// Returns a const reference to the element at the front of the TCircularBuffer.
	const EventT& Front() const
	{
		if(m_Count>0)
		{
			return m_Container[m_FrontIdx];
		}
		else
		{
			throw std::out_of_range("Get element from a empty TCircularBuffer");
		}
	}

	/// Returns a const reference to the element at position idx in the TCircularBuffer.
	/// Returning the element at position 0 is the same with Front().
	const EventT& operator[](int idx) const
	{
		if(idx < 0 || (size_t)idx > m_Count-1 || m_Count == 0)
		{
			throw std::out_of_range("TCircularBuffer subscript out of range");
		}

		size_t pos = WarpIndex(m_FrontIdx + idx);
		return m_Container[pos];
	}

	/// Push data to the TCircularBuffer.
	/// If the TCircularBuffer is full, Pushing data will force TCircularBuffer to pop it's front element.
	void Push(const EventT& val)
	{
		if(m_Count > m_nQueueSize-1)
		{
			// Removes the front element when the TCircularBuffer is full
			Pop();
		}

		size_t rearIdx = WarpIndex(m_FrontIdx + m_Count);
		m_Container[rearIdx] = val;
		++m_Count;
	}

	/// Removes the front element in the TCircularBuffer.
	/// If the TCircularBuffer is empty, calling Pop() has no effect.
	void Pop()
	{
		m_FrontIdx = WarpIndex(m_FrontIdx + 1);
		if(m_Count>0)
		{
			--m_Count;
		}
	}

	/// Returns the size of the allocated storage space for the elements of the TCircularBuffer.
	size_t GetCapacity() const
	{
		return m_nQueueSize;
	}

	/// Returns the number of elements in the TCircularBuffer.
	size_t GetSize() const
	{
		return m_Count;
	}

	/// Empty the TCircularBuffer, and reset it's internal states.
	void Clear()
	{
		m_FrontIdx = 0;
		m_Count = 0;
	}

protected:
	EventT* m_Container;
	size_t m_nQueueSize;

	size_t m_FrontIdx;   ///< The index of the first element.
	size_t m_Count;      ///< number of elements in the TCircularBuffer.

	/// Returns a warp around index value
	inline size_t WarpIndex(size_t idx) const
	{
		return (idx > m_nQueueSize-1) ? idx % m_nQueueSize : idx;
	}
};


#endif //_TCircularBuffer_Template_H_
