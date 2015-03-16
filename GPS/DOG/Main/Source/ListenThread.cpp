#include "ListenThread.h"
#include <algorithm>

#define __offsetof(type, field) ((size_t)(&((type *)0)->field))

// namespace Diagnosis-of-GPS
namespace dog
{

//-----------------------------------------------------------------------------
// Internal class: TLargePack
// - Container for collecting large pack event
// - not implemented by thread-safe
class CListenThread::TLargePack : public boost::noncopyable
{
public:
	TLargePack() : m_Buf(NULL), m_BufSize(0), transport_id(0), total_packets(0), continuity(0) {}
	~TLargePack() { Free(); }

	void Reset()
	{
		transport_id = 0;
		total_packets = 0;
		continuity = 0;
	}

	bool Allocate(size_t size)
	{
		if(m_BufSize == size) {
			return true;
		}

		Free();
		m_Buf = (BYTE*)malloc(size);
		m_BufSize = size;
		return m_Buf != NULL;
	}

	void Free()
	{
		if(m_Buf)
		{
			free(m_Buf);
			m_Buf = NULL;
			m_BufSize = 0;
		}
	}

	BYTE* GetBuf() { return m_Buf; }
	size_t GetSize() const { return m_Buf!=NULL ? m_BufSize : 0; }

	LONGLONG transport_id;
	DWORD total_packets;
	DWORD continuity;

protected:
	BYTE* m_Buf;
	size_t m_BufSize;
};

//-----------------------------------------------------------------------------
void CListenThread::RegisterListener(IEventListener* listener)
{
	if(NULL == listener) {
		return;
	}

	CAutoLock loc(&m_cs);
	std::list<IEventListener*>::iterator pos = std::find(m_ListenerList.begin(), m_ListenerList.end(), listener);
	if(pos == m_ListenerList.end())
	{
		m_ListenerList.push_back(listener);
	}
}

void CListenThread::UnregisterListener(IEventListener* listener)
{
	CAutoLock loc(&m_cs);
	std::list<IEventListener*>::iterator pos = std::find(m_ListenerList.begin(), m_ListenerList.end(), listener);
	if(pos != m_ListenerList.end())
	{
		m_ListenerList.erase(pos);
	}
}

size_t CListenThread::GetNumOfListener()
{
	CAutoLock loc(&m_cs);
	return m_ListenerList.size();
}

void CListenThread::CollectPackects(const CSessionBuffer::TDataPacket& pack)
{
	const size_t header_size = __offsetof(CSessionBuffer::TDataPacket, payload);
	const DWORD payload_size = pack.packet_size - header_size;

	if(NULL == m_pLargePackBuf)
	{
		m_pLargePackBuf = boost::shared_ptr<TLargePack>(new TLargePack);
	}

	// If a DogEvent is divided into several TDataPacket, the packets should be transmitted with adjacent packets.
	// If the packets are not transmitted adjacently, the situation will be considered as packet lost.
	// For example: a DogEvent is divided into 5 TDataPacket. If the DogEvent is transmitted twice,
	// they will be transmitted as following:
	// transport_id=100, continuity=0, total_packets=5 <--- first packet of transport_id=100
	// transport_id=100, continuity=1, total_packets=5
	// transport_id=100, continuity=2, total_packets=5
	// transport_id=100, continuity=3, total_packets=5
	// transport_id=100, continuity=4, total_packets=5 <--- last
	// transport_id=105, continuity=0, total_packets=5 <--- first packet of transport_id=105
	// transport_id=105, continuity=1, total_packets=5
	// transport_id=105, continuity=2, total_packets=5
	// transport_id=105, continuity=3, total_packets=5
	// transport_id=105, continuity=4, total_packets=5 <--- last

	if(0 == pack.pack_header.continuity)
	{
		// the first packet, init m_pLargePackBuf for collecting data.

		DogEvent& event = *(DogEvent*)(&pack.payload);
		m_pLargePackBuf->Allocate(event.Size);

		m_pLargePackBuf->transport_id = pack.pack_header.transport_id;
		m_pLargePackBuf->total_packets = pack.pack_header.total_packets;
		m_pLargePackBuf->continuity = 0;
	}
	else if(m_pLargePackBuf->continuity+1 != pack.pack_header.continuity)
	{
		// error: packet lost. drop data
		// packets for the same event should be transmitted adjacently and continuously.
		m_pLargePackBuf->Reset();
		return;
	}

	if(m_pLargePackBuf->transport_id != pack.pack_header.transport_id)
	{
		// error: transport_id mismatch. cannot accept this packet.
		return;
	}

	BYTE* pBuf = m_pLargePackBuf->GetBuf();
	size_t nSize = m_pLargePackBuf->GetSize();
	size_t offset = pack.pack_header.continuity * sizeof(pack.payload);
	if(pBuf) {
		memcpy_s(pBuf+offset, nSize-offset, &pack.payload, payload_size);
	}

	// update continuity count
	m_pLargePackBuf->continuity = pack.pack_header.continuity;
}

void CListenThread::NotifyLargePackEvent(LONGLONG transport_id)
{
	if(NULL == m_pLargePackBuf)
	{
		return;
	}

	if(m_pLargePackBuf->transport_id == transport_id &&
		m_pLargePackBuf->continuity !=0 &&
		m_pLargePackBuf->continuity == m_pLargePackBuf->total_packets-1)
	{
		BYTE* pBuf = m_pLargePackBuf->GetBuf();

		CAutoLock loc(&m_cs);
		std::list<IEventListener*>::iterator iterListener;
		for(iterListener = m_ListenerList.begin(); iterListener!=m_ListenerList.end(); ++iterListener)
		{
			(*iterListener)->NotifyEvent( *(DogEvent*)(pBuf) );
		}
	}
}

void CListenThread::Run()
{
	while( 0 != m_bRunFlag )
	{
		CSessionBuffer::TDataPacket pack;
		CSessionBuffer::PopErrorEnum ret = m_refSessionBuf.Pop(pack);

		if(CSessionBuffer::POP_OK == ret)
		{
			if(pack.pack_header.total_packets > 1)
			{
				CollectPackects(pack);

				if(pack.pack_header.continuity == pack.pack_header.total_packets-1)
				{
					// the last packet, event transmission complete.
					NotifyLargePackEvent(pack.pack_header.transport_id);
				}
			}
			else
			{
				CAutoLock loc(&m_cs);
				std::list<IEventListener*>::iterator iter;
				for(iter = m_ListenerList.begin(); iter!=m_ListenerList.end(); ++iter)
				{
					(*iter)->NotifyEvent( *(DogEvent*)(&pack.payload) );
				}
			}
		}
		else if(CSessionBuffer::POP_ERROR_NO_DATA_AVAILABLE == ret)
		{
			// run out of queue. wait for a new event arriving.
			Sleep(10);
		}
	}
}


} // namespace dog
