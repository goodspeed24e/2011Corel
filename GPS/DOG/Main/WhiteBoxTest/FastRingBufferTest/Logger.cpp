#include "Logger.h"
#include "FastRingBufferTemplate.h"
#include <algorithm>
#include <new>


//-----------------------------------------------------------------------------
// internal class CLogger::CLogBuffer
class CLogger::CLogBuffer : public dog::TFastRingBuffer<CLogger::LogEntry>
{
	friend class CLogger;
public:
	void Init(DWORD nSizeOfArray)
	{
		TFastRingBuffer::Init(nSizeOfArray);
		CHAR typeIndicator[16] = "Logger";
		Version ver = {0,0, 1,0};
		size_t nSizeOfStr = (std::min)(sizeof(m_TypeIndicator), strlen(typeIndicator));
		memcpy_s(m_TypeIndicator, sizeof(m_TypeIndicator), typeIndicator, nSizeOfStr);
	}
};


//-----------------------------------------------------------------------------
// class CLogger
CLogger::CLogger() : m_pBuffer(NULL), m_dwStartTime(0), m_dwIndex(0), m_nReadIndex(0), m_nMissCount(0), m_nSkippedIndex(0)
{
}

CLogger::~CLogger()
{
}

bool CLogger::Create(LPCTSTR lpMmfName)
{
	if(m_pBuffer)
	{
		Close();
	}

	const size_t nSizeOfArray = 4096;
	const size_t nSizeOfHeader = sizeof(CLogBuffer);
	const size_t nMmfSize = nSizeOfHeader + sizeof(CLogBuffer::TDataSlot) * nSizeOfArray;

	bool ret = m_MemMapFile.Create(lpMmfName, (DWORD)nMmfSize);
	if(ret && m_MemMapFile.GetData())
	{
		m_dwStartTime = timeGetTime();

		// placement new
		m_pBuffer = new (m_MemMapFile.GetData()) CLogBuffer;
		m_pBuffer->Init(nSizeOfArray);

		m_nReadIndex = m_pBuffer->GetLowRange();
		m_nMissCount = m_nReadIndex;

		// VirtualLock(m_MemMapFile.GetData());
		return true;
	}
	return false;
}

bool CLogger::Open(LPCTSTR lpMmfName)
{
	if(m_pBuffer)
	{
		Close();
	}

	bool ret = m_MemMapFile.Open(lpMmfName);
	if(ret && m_MemMapFile.GetData())
	{
		// placement new
		m_pBuffer = new (m_MemMapFile.GetData()) CLogBuffer;
		m_nReadIndex = m_pBuffer->GetLowRange();
		m_nMissCount = m_nReadIndex;
		return true;
	}
	return false;
}

void CLogger::Close()
{
	if(m_pBuffer)
	{
		m_pBuffer->~CLogBuffer();
		m_pBuffer = NULL;
	}
	m_MemMapFile.Close();
}

void CLogger::Output(DWORD dwCategory, DWORD dwSubCategory, const char* szMsg)
{
	if(m_pBuffer)
	{
		LogEntry entry = { 0, timeGetTime()-m_dwStartTime, dwCategory, dwSubCategory, 0};
		InterlockedIncrement((LONG*)&m_dwIndex);
		entry.dwIndex = m_dwIndex;
		strcpy_s(entry.szMessage, _countof(entry.szMessage), szMsg);
		m_pBuffer->Write(entry);
	}
}


bool CLogger::Receive(CLogger::LogEntry& entry)
{
	if(m_pBuffer)
	{
		const DWORD nWriteIdx = m_pBuffer->GetWriteIndex();
		if(nWriteIdx <= m_nReadIndex)
		{
			// no available data
			return false;
		}

		const DWORD nReadIdx = InterlockedIncrement((LONG*)&m_nReadIndex) -1;
		bool ret = m_pBuffer->Read(nReadIdx, entry);
		if(!ret)
		{
			for(;;)
			{
				const DWORD nLowRange = m_pBuffer->GetLowRange();
				const DWORD nNewReadIdx = nLowRange;
				const DWORD nOldReadIdx = m_nReadIndex;
				if(nNewReadIdx < nOldReadIdx)
				{
					if(nReadIdx > m_nSkippedIndex) 
					{
						// m_nSkippedIndex may not be updated
						// sleep a while to wait for m_nSkippedIndex being updated
						Sleep(1); 
					}

					if(nReadIdx < m_nSkippedIndex) {
						InterlockedIncrement((LONG*)&m_nMissCount);
					}
					break;
				}

				if(InterlockedCompareExchange((LONG*)&m_nReadIndex, nNewReadIdx, nOldReadIdx)  == nOldReadIdx)
				{
					InterlockedExchange((LONG*)&m_nSkippedIndex, nOldReadIdx);
					for(;;)
					{
						const DWORD nNewMissCount = m_nMissCount + ((nNewReadIdx-nOldReadIdx) + 1);
						const DWORD nOldMissCount = m_nMissCount;
						if(InterlockedCompareExchange((LONG*)&m_nMissCount, nNewMissCount, nOldMissCount) == nOldMissCount)
						{
							break;
						}
					}
					break;
				}
			}

			// data lost
			return false;
		}
		return ret;
	}
	return false;
}

void CLogger::SetSizeOfArray(DWORD nSize)
{
	if(m_pBuffer && nSize <= m_pBuffer->GetSizeOfArray())
	{
		m_pBuffer->m_nSizeOfArray = nSize;
	}
}

size_t CLogger::GetSizeOfArray()
{
	if(m_pBuffer)
	{
		return m_pBuffer->m_nSizeOfArray;
	}
	return 0;
}


DWORD CLogger::GetMissCount()
{
	return m_nMissCount;
}

DWORD CLogger::GetWriteIndex()
{
	if(m_pBuffer)
	{
		return m_pBuffer->m_nWriteIndex.QuadPart;
	}
	return 0;
}

DWORD CLogger::GeReadIndex()
{
	return m_nReadIndex;
}

