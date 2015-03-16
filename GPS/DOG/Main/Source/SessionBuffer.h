#ifndef _DIAGNOSIS_OF_GPS_EVENT_SESSION_BUFFER_H_
#define _DIAGNOSIS_OF_GPS_EVENT_SESSION_BUFFER_H_

#include "MemMapFile.h"

#include <boost/utility.hpp>

// namespace Diagnosis-of-GPS
namespace dog
{

struct DogEvent;

/**
 * CSessionBuffer encapsulates the operations of memory mapping file for DOG.
 * A DogEvent will be wrapped as TDataPacket, and then stored to the CSessionBuffer.
 *
 * Version Number:
 * CSessionBuffer has its version number, which is got from the header part of
 * memory mapping file. If the version of CSessionBuffer doesn't agree with the
 * memory mapping file, which means the layout of data is different, CSessionBuffer
 * will reject it.
 */
class CSessionBuffer : public boost::noncopyable
{
public:
	CSessionBuffer();
	~CSessionBuffer();

	/// Create a memory mapping file with the specified name.
	bool Create(LPCTSTR lpMmfName);

	/// Open a existing memory mapping file with the specified name.
	bool Open(LPCTSTR lpMmfName);

	/// Close the memory mapping file.
	void Close();

	/// version number: x.x.x.x
	struct Version
	{
		WORD MajorMain;
		WORD MajorSub;
		WORD MinorMain;
		WORD MinorSub;
	};
	Version GetVersion() const { return m_Version; }
	static Version GetSupportedVersion() { return SupportedVersion; }

	LONGLONG GetReadIndex() const { return m_nReadIndex; }
	LONGLONG GetMissCount() const { return m_nMissCount; }
	HRESULT  GetAvailableRange(LONGLONG* pUpperBound, LONGLONG* pLowerBound) const;
	LONGLONG SetReadIndex(LONGLONG readIdx);


	/// A DogEvent will be wrapped as TDataPacket.
	/// If a DogEvent is larger than the payload. The DogEvent will be divided into several TDataPacket.
	/// If a DogEvent is divided into several TDataPacket, the packets should be transmitted with adjacent packets.
	/// If the packets are not transmitted adjacently, the situation will be considered as packet lost.
	///
	/// For example: a DogEvent is divided into 5 TDataPacket. If the DogEvent is transmitted twice,
	/// they will be transmitted as following:
	/// transport_id=100, continuity=0, total_packets=5 <--- first packet of transport_id=100
	/// transport_id=100, continuity=1, total_packets=5
	/// transport_id=100, continuity=2, total_packets=5
	/// transport_id=100, continuity=3, total_packets=5
	/// transport_id=100, continuity=4, total_packets=5 <--- last
	/// transport_id=105, continuity=0, total_packets=5 <--- first packet of transport_id=105
	/// transport_id=105, continuity=1, total_packets=5
	/// transport_id=105, continuity=2, total_packets=5
	/// transport_id=105, continuity=3, total_packets=5
	/// transport_id=105, continuity=4, total_packets=5 <--- last
	///
	struct TDataPacket
	{
		DWORD packet_size; ///< size of packet's valid data
		DWORD reserved;    // Reserved. Make pack_header aligns with 8.
		struct THeader
		{
			DWORD continuity;       ///< packet counter, starts from 0 to total_packets-1
			DWORD total_packets;    ///< number of packets of the DogEvent is divided into.
			LONGLONG transport_id;  ///< each DogEvent will be marked as unique transport_id
			THeader() :	continuity(0), total_packets(0), transport_id(0) {}
		};
		THeader pack_header;
		BYTE payload[512 - sizeof(DWORD)*2 - sizeof(THeader)];

		size_t GetSize() const { return packet_size; }
	};

	/// Push a DogEvent to the CSessionBuffer. CSessionBuffer will wrap it as TDataPacket.
	/// @param dogEvent
	/// @param threadId  : Thread Id of the event source
	/// @param processId : Process Id of the event source
	/// @param timeStamp : Time in 100-nanoseconds when event happens
	/// @return
	///     the index of the TDataPacket
	LONGLONG Push(const DogEvent& dogEvent, ULONG threadId, ULONG processId, LARGE_INTEGER timeStamp, LARGE_INTEGER baseTime);

	enum PopErrorEnum
	{
		POP_OK = 0,
		POP_ERROR_UNKNOWN = 0x80004005,
		POP_ERROR_NO_DATA_AVAILABLE,
		POP_ERROR_DATA_LOST,
	};

	/// Pop a TDataPacket from the CSessionBuffer
	PopErrorEnum Pop(TDataPacket& slot);

protected:
	static Version SupportedVersion; ///< version of the CSessionBuffer implementation supporting
	Version m_Version; ///< version got from memory mapping file
	bool m_bIsSupportedMMF; //flag for MMF's version supporting

	volatile DWORD m_nReadIndex;
	volatile DWORD m_nMissCount;
	volatile DWORD m_nSkippedIndex; // for miss count calculation under multi-thread situation

	 CMemMapFile m_MemMapFile;

	 class CMemoryMapping;
	 CMemoryMapping* m_pMem;
};


} // namespace dog

#endif // _DIAGNOSIS_OF_GPS_EVENT_SESSION_BUFFER_H_
