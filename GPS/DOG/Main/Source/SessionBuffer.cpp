#include "SessionBuffer.h"
#include "FastRingBufferTemplate.h"
#include "DOG.h"
#include <algorithm>
#include <limits>
#include <new>

// namespace Diagnosis-of-GPS
namespace dog
{

#define SUPPORTED_SESSION_VERSION {1,0, 0,0}
CSessionBuffer::Version CSessionBuffer::SupportedVersion = SUPPORTED_SESSION_VERSION;

#define __offsetof(type, field) ((size_t)(&((type *)0)->field))

//-----------------------------------------------------------------------------
// internal class CEventSession::CMemoryMapping
class CSessionBuffer::CMemoryMapping : public TFastRingBuffer<CSessionBuffer::TDataPacket>
{
	friend class CSessionBuffer;
public:
	void Init(DWORD nSizeOfArray)
	{
		TFastRingBuffer::Init(nSizeOfArray);
		CHAR typeIndicator[16] = "DogSession";
		Version ver = SUPPORTED_SESSION_VERSION;
		size_t nSizeOfStr = (std::min)(sizeof(m_TypeIndicator), strlen(typeIndicator));
		memcpy_s(m_TypeIndicator, sizeof(m_TypeIndicator), typeIndicator, nSizeOfStr);
		memcpy_s(&m_Version, sizeof(m_Version), &ver, sizeof(ver));
	}
};

//-----------------------------------------------------------------------------
// class CEventSession
CSessionBuffer::CSessionBuffer() : m_pMem(NULL), m_nReadIndex(0), m_nMissCount(0), m_nSkippedIndex(0)
{
	Version zeroVer = {0,0,0,0};
	m_Version = zeroVer;
	LPCTSTR szMmfName = L"DOG-EventSession-{97F28020-C89F-44ab-A9D2-0CD4B032CC48}";
	if(!Open(szMmfName))
	{
		Create(szMmfName);
	}
}

CSessionBuffer::~CSessionBuffer()
{
	Close();
}

bool CSessionBuffer::Create(LPCTSTR lpMmfName)
{
	if(m_pMem)
	{
		Close();
	}

	const size_t nSizeOfArray = 1024 * 16;
	const size_t nSizeOfHeader = sizeof(CMemoryMapping);
	const size_t nMmfSize = nSizeOfHeader + sizeof(CMemoryMapping::TDataSlot) * nSizeOfArray;


	bool ret = m_MemMapFile.Create(lpMmfName, (DWORD)nMmfSize);
	if(ret && m_MemMapFile.GetData())
	{
		// placement new
		m_pMem = new (m_MemMapFile.GetData()) CMemoryMapping;
		m_pMem->Init(nSizeOfArray);

		m_nReadIndex = m_pMem->GetLowRange();
		m_nMissCount = m_nReadIndex;
		CMemoryMapping::Version bufVer = m_pMem->GetVersion();
		memcpy_s(&m_Version, sizeof(m_Version), &bufVer, sizeof(bufVer));
		m_bIsSupportedMMF = (0==memcmp(&m_Version, &SupportedVersion, sizeof(m_Version)));

		// VirtualLock(m_MemMapFile.GetData());
		return true;
	}
	return false;
}

bool CSessionBuffer::Open(LPCTSTR lpMmfName)
{
	if(m_pMem)
	{
		Close();
	}

	bool ret = m_MemMapFile.Open(lpMmfName);
	if(ret && m_MemMapFile.GetData())
	{
		// placement new
		m_pMem = new (m_MemMapFile.GetData()) CMemoryMapping;
		m_nReadIndex = m_pMem->GetLowRange();
		m_nMissCount = m_nReadIndex;
		CMemoryMapping::Version bufVer = m_pMem->GetVersion();
		memcpy_s(&m_Version, sizeof(m_Version), &bufVer, sizeof(bufVer));
		m_bIsSupportedMMF = (0==memcmp(&m_Version, &SupportedVersion, sizeof(m_Version)));

		return true;
	}
	return false;
}

void CSessionBuffer::Close()
{
	if(m_pMem)
	{
		m_pMem->~CMemoryMapping();
		m_pMem = NULL;
	}
	m_MemMapFile.Close();
}

//-----------------------------------------------------------------------------
HRESULT CSessionBuffer::GetAvailableRange(LONGLONG* pUpperBound, LONGLONG* pLowerBound) const
{
	if(m_pMem && m_bIsSupportedMMF)
	{
		if(pUpperBound)
		{
			*pUpperBound = m_pMem->GetWriteIndex();
		}
		if(pLowerBound)
		{
			*pLowerBound = m_pMem->GetLowRange();
		}
		return S_OK;
	}
	return E_FAIL;
}

LONGLONG CSessionBuffer::SetReadIndex(LONGLONG readIdx)
{
	if(m_pMem && m_bIsSupportedMMF)
	{
		LONGLONG upperBound = m_pMem->GetWriteIndex();
		LONGLONG lowerBound = m_pMem->GetLowRange();
		if(upperBound >= readIdx && readIdx >= lowerBound)
		{
			InterlockedExchange((LONG*)&m_nReadIndex, readIdx);
			return m_nReadIndex;
		}
	}
	return 0;
}

//-----------------------------------------------------------------------------
LONGLONG CSessionBuffer::Push(const DogEvent& dogEvent, ULONG threadId, ULONG processId, LARGE_INTEGER timeStamp, LARGE_INTEGER baseTime)
{
	if(NULL==m_pMem || !m_bIsSupportedMMF)
	{
		// error: no memory-mapping file allocate or version mismatching
		return (std::numeric_limits<LONGLONG>::max)();
	}

	const size_t sizeOfpayload = sizeof(TDataPacket) - __offsetof(TDataPacket, payload);
	DWORD totalPackets = 1;
	if(dogEvent.Size > sizeOfpayload)
	{
		totalPackets = dogEvent.Size/sizeOfpayload + (dogEvent.Size%sizeOfpayload ? 1 : 0);
	}
	const LONGLONG nWriteIdx = m_pMem->Allocate(totalPackets);

	CMemoryMapping::TDataSlot* pDataSlot= m_pMem->GetSlot(nWriteIdx);
	pDataSlot->Index = nWriteIdx;
	TDataPacket& pack = pDataSlot->Data;

	DWORD payload_size = (std::min<size_t>)(dogEvent.Size, sizeof(pack.payload));
	pack.packet_size = payload_size + __offsetof(TDataPacket, payload);
	pack.pack_header.continuity = 0;
	pack.pack_header.total_packets = totalPackets;
	pack.pack_header.transport_id = nWriteIdx;
	memcpy_s(&pack.payload, sizeof(pack.payload), &dogEvent, payload_size);

	DogEvent& evt = *((DogEvent*)(&pack.payload));
	evt.ProcessId = threadId;
	evt.ThreadId  = processId;
	evt.Index     = (DWORD)nWriteIdx;
	evt.TimeStamp = timeStamp;
	evt.BaseTime  = baseTime;

	pDataSlot->Signature = nWriteIdx;

	if(1==totalPackets) {
		return nWriteIdx;
	}

	//--------------------------------------------
	// for multiple packets

	size_t nRemaningData = (std::max<size_t>)(dogEvent.Size-sizeof(pack.payload), 0);
	for(DWORD continuity=1; continuity<totalPackets; ++continuity)
	{
		CMemoryMapping::TDataSlot* pDataSlot= m_pMem->GetSlot(nWriteIdx + continuity);
		pDataSlot->Index = nWriteIdx + continuity;
		TDataPacket& pack = pDataSlot->Data;

		payload_size = (std::min<size_t>)(nRemaningData, sizeof(pack.payload));
		pack.packet_size = payload_size + __offsetof(TDataPacket, payload);
		pack.pack_header.continuity = continuity;
		pack.pack_header.total_packets = totalPackets;
		pack.pack_header.transport_id = nWriteIdx;
		BYTE* curPtr = (BYTE*)&dogEvent+(dogEvent.Size-nRemaningData);
		memcpy_s(&pack.payload, sizeof(pack.payload), curPtr, payload_size);

		pDataSlot->Signature = nWriteIdx + continuity;

		nRemaningData -= sizeof(pack.payload);
	}

	return nWriteIdx;
}

CSessionBuffer::PopErrorEnum CSessionBuffer::Pop(TDataPacket& slot)
{
	if(m_pMem && m_bIsSupportedMMF)
	{
		const DWORD nWriteIdx = m_pMem->GetWriteIndex();
		if(nWriteIdx <= m_nReadIndex)
		{
			// no available data
			return POP_ERROR_NO_DATA_AVAILABLE;
		}

		const DWORD nPreReadIdx = m_nReadIndex+1;
		CMemoryMapping::TDataSlot* pSlot = m_pMem->GetSlot(nPreReadIdx);
		if(pSlot->Index==nPreReadIdx && pSlot->Index != pSlot->Signature)
		{
			// data is still writing into buffer
			return POP_ERROR_NO_DATA_AVAILABLE;
		}

		const DWORD nReadIdx = InterlockedIncrement((LONG*)&m_nReadIndex) -1;
		bool ret = m_pMem->Read(nReadIdx, slot);
		if(ret)
		{
			return POP_OK;
		}
		else // data lost, update m_nMissCount
		{
			for(;;)
			{
				const DWORD nLowRange = m_pMem->GetLowRange();
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
					// There may be many threads call Pop(). m_nSkippedIndex helps to identify
					// whose "nReadIdx" are calculated into miss count.
					// For multi-threading issue, we will keep nOldReadIdx to m_nSkippedIndex
					// to indicate that we have calculated miss count between nOldReadIdx
					// and nNewReadIdx.
					InterlockedExchange((LONG*)&m_nSkippedIndex, nOldReadIdx);

					// update m_nMissCount
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
			return POP_ERROR_DATA_LOST;
		}
	}
	return POP_ERROR_UNKNOWN;
}

} // namespace dog
