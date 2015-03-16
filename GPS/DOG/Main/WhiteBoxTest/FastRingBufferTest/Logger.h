#pragma once

#include "MemMapFile.h"

/**
 * A simple logger implementation test
 */
class CLogger
{
public:
	CLogger();
	~CLogger();

	bool Create(LPCTSTR lpMmfName);
	bool Open(LPCTSTR lpMmfName);
	void Close();

	void Output(const char* szMsg) { Output(0,0, szMsg); }
	void Output(DWORD dwCategory, DWORD dwSubCategory, const char* szMsg);

	struct LogEntry
	{
		DWORD dwIndex;
		DWORD dwTime;
		DWORD dwCategory;
		DWORD dwSubCategory;
		CHAR  szMessage[512-(sizeof(DWORD)*4)];
		size_t GetSize() const { return sizeof(DWORD)*4+sizeof(szMessage); }
	};

	bool Receive(LogEntry& entry);

	void  SetSizeOfArray(DWORD nSize);
	size_t GetSizeOfArray();

	DWORD GetMissCount();
	DWORD GetWriteIndex();
	DWORD GeReadIndex();

protected:
	DWORD m_dwStartTime;
	DWORD m_dwIndex;

	volatile DWORD m_nReadIndex;
	volatile DWORD m_nMissCount;
	volatile DWORD m_nSkippedIndex;

	dog::CMemMapFile m_MemMapFile;
	
	class CLogBuffer;
	CLogBuffer* m_pBuffer;
};
