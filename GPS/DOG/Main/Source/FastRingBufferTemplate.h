#pragma once

#include <windows.h>
#include <stddef.h>

namespace dog {

/**
* For performance reasons, we do not want to use any critical sections, mutex's,
* or other thread sync just like the existing implementation. This means, all
* operations to move the pointers must be performed atomically. I.e., they appear
* to complete in a single instruction from all other threads' perspective.
*/
template<typename T>
class TFastRingBuffer
{
public:
	/// version number: x.x.x.x
	struct Version
	{
		WORD MajorMain;
		WORD MajorSub;
		WORD MinorMain;
		WORD MinorSub;
	};

	struct TDataSlot
	{
		LONGLONG Index;
		T Data;
		LONGLONG Signature;
	};

	/// Initialize the header part of the buffer
	inline void Init(DWORD nSizeOfArray)
	{
		memset(this, 0, sizeof(TFastRingBuffer));

		m_SizeOfHeader = sizeof(TFastRingBuffer);
		m_nSizeOfArray = nSizeOfArray;
		m_nSizeOfBuffer = nSizeOfArray * sizeof(TDataSlot);
	}

	Version GetVersion() const { return m_Version; }

	/// Get the current write index of the buffer, the index is mono-increased.
	inline LONGLONG GetWriteIndex() const { return m_nWriteIndex.QuadPart; }

	/// Get the low range index. Data is only available between the write index and low range index.
	inline LONGLONG GetLowRange() const { return m_nWriteIndex.QuadPart < m_nSizeOfArray ? 0 : m_nWriteIndex.QuadPart - m_nSizeOfArray; }
	
	/// Get the size of the buffer in data unit.
	inline DWORD GetSizeOfArray() const { return m_nSizeOfArray; }

	/// Reserves the specified number of TDataSlot with continuous indices.
	/// @param numOfSlots
	/// @return
	///     the index of the beginning reserved TDataSlot.
	inline LONGLONG Allocate(DWORD numOfSlots)
	{
		if(1==numOfSlots)
		{
			const LONGLONG nWriteIdx = InterlockedIncrement((LONG*)&m_nWriteIndex.LowPart);
			return nWriteIdx-1;
		}
		else
		{
			const LONGLONG nWriteIdx = InterlockedExchangeAdd((LONG*)&m_nWriteIndex.LowPart, numOfSlots);
			return nWriteIdx;
		}
	}

	/// Returns the address of the specified TDataSlot
	inline TDataSlot* GetSlot(const LONGLONG index)
	{
		const LONGLONG bufIdx = index % m_nSizeOfArray;
		TDataSlot* pSlot = (TDataSlot*)((BYTE*)this + m_SizeOfHeader);
		return &pSlot[bufIdx];
	}

	/// Writes data to the buffer. Write() is a very fast method.
	/// It doesn't need to wait for free space, so it will not be blocked.
	/// After Write() is called, the write index will be increased.
	/// @return
	///     the buffer index of the data. 
	inline LONGLONG Write(const T& data)
	{
		const DWORD nWriteIdx = Allocate(1);
		const DWORD bufIdx = (DWORD)(nWriteIdx % m_nSizeOfArray);

		TDataSlot* pSlot = (TDataSlot*)((BYTE*)this + m_SizeOfHeader);
		pSlot[bufIdx].Index = nWriteIdx;
		memcpy_s(&pSlot[bufIdx].Data, sizeof(T), &data, data.GetSize());
		pSlot[bufIdx].Signature = nWriteIdx;
		return nWriteIdx;
	}

	/// Get data from the buffer
	/// @param index: the index of the requested data. Data is only available between the write index and low range index.
	/// @param dataOut [out] : reference for outputting data
	/// @return 
	///     true:  operation succeeded.
	///     false: the requested data doesn't exist.
	inline bool Read(const LONGLONG index, T& dataOut)
	{
		TDataSlot* pSlot = GetSlot(index);
		if(index == pSlot->Index)
		{
			memcpy_s(&dataOut, sizeof(T), &pSlot->Data, pSlot->Data.GetSize());
		}
		// compare signature. if signature doesn't match, the data may be corrupted by Write(). i.e. race condition
		return (index == pSlot->Index &&  pSlot->Index == pSlot->Signature);
	}

protected:
	CHAR m_TypeIndicator[16];

	/// TFastRingBuffer gets buffer address with the equation below:
	///   Buffer Start Address = this + m_SizeOfHeader
	DWORD m_SizeOfHeader;

	Version m_Version;

	DWORD m_nSizeOfArray;  ///< capacity of the ring buffer
	DWORD m_nSizeOfBuffer; ///< m_nSizeOfBuffer = m_nSizeOfArray * sizeof(TSlot);
	volatile LARGE_INTEGER m_nWriteIndex; ///< m_nWriteIndex indicates the next writing position

	BYTE reserved[128-(sizeof(CHAR[16])+sizeof(Version)+sizeof(DWORD)*3 + sizeof(LARGE_INTEGER))];

	// TSlot m_Buffer[m_nSizeOfArray];
};


} // namespace dog
