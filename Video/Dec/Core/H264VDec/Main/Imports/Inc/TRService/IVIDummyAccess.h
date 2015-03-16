// IVIDummyAccess.h: interface for the IVIDummyAccess class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IVIDUMMYACCESS_H__0454D94D_C00F_4982_9788_8C77B02D9572__INCLUDED_)
#define AFX_IVIDUMMYACCESS_H__0454D94D_C00F_4982_9788_8C77B02D9572__INCLUDED_

#include "IVITRComm.h"

#ifdef _WIN32
	///*!
	// [Internal function. Do not call it.]
	//
	// start to perform dummy memory write
	//
    // -# nBlockSize - [IN] The block unit in bytes to be allocated for dummy write. Should be greater or equal to 1.
    // -# nTotalBlockPerThread - [IN] the total count of the blocks allocated by one working thread. Should be greater or equal to 1.
    // -# nTotalThread - [IN] the total working threads for dummy write. Should be greater or equal to 1.
    // -# nSleepTimePerCycle - [IN] the sleep time in 'ms' when the working thread completes one cycle to dummy write on the total blocks. 0 means no delay.
    // -# pszHeadPattern - [IN] the specific patterns in case-insensitive little-endian HEX string (like "001b2E") encrypted by iviTR_ENCRYPT_CONTENT(byte pattern = 0x71, encrypt type = 1). The decrypted binary pattern (0x001B2E not "001b2E") will be put at the head of each dummy write block. NULL if no need to insert specific pattern at head of the dummy write block.
    // -# pszMidPattern - [IN] the specific patterns in case-insensitive little-endian HEX string (like "001b2E") encrypted by iviTR_ENCRYPT_CONTENT(byte pattern = 0x71, encrypt type = 1). The decrypted binary pattern (0x001B2E not "001b2E") will be put at the middle of each dummy write block. NULL if no need to insert specific pattern at middle of the dummy write block.
    // -# pszTailPattern - [IN] the specific patterns in case-insensitive little-endian HEX string (like "001b2E") encrypted by iviTR_ENCRYPT_CONTENT(byte pattern = 0x71, encrypt type = 1). The decrypted binary pattern (0x001B2E not "001b2E") will be put at the tail of each dummy write block. NULL if no need to insert specific pattern at tail of the dummy write block.
    //
	// Return value - [bool] true if OK, false if error found.\n
	// Supported platform: All Windows\n
	//*/
	bool __cdecl trStartDummyWrite(unsigned nBlockSize, unsigned nTotalBlockPerThread, unsigned nTotalThread, unsigned nSleepTimePerCycle, char *pszHeadPattern, char *pszMidPattern, char *pszTailPattern);

	///*!
	// [Internal function. Do not call it.]
	//
	// stop dummy memory write process
	//
	// Return value - [bool] true if OK, false if error found.\n
	// Supported platform: All Windows\n
	//*/
	bool __cdecl trEndDummyWrite(void);
	
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	///*!
	// Start to perform dummy memory write. This is useful to defend plain-text memory dump attack.
	//
    // -# nBlockSize - [IN] The block unit in bytes to be allocated for dummy write. Should be greater or equal to 1.
    // -# nTotalBlockPerThread - [IN] the total count of the blocks allocated by one working thread. Should be greater or equal to 1.
    // -# nTotalThread - [IN] the total working threads for dummy write. Should be greater or equal to 1.
    // -# nSleepTimePerCycle - [IN] the sleep time in 'ms' when the working thread completes one cycle to dummy write on the total blocks. 0 means no delay.
    // -# pszHeadPattern - [IN] the specific patterns in case-insensitive little-endian HEX string (like "001b2E") encrypted by iviTR_ENCRYPT_CONTENT(byte pattern = 0x71, encrypt type = 1). The decrypted binary pattern (0x001B2E not "001b2E") will be put at the head of each dummy write block. NULL if no need to insert specific pattern at head of the dummy write block.
    // -# pszMidPattern - [IN] the specific patterns in case-insensitive little-endian HEX string (like "001b2E") encrypted by iviTR_ENCRYPT_CONTENT(byte pattern = 0x71, encrypt type = 1). The decrypted binary pattern (0x001B2E not "001b2E") will be put at the middle of each dummy write block. NULL if no need to insert specific pattern at middle of the dummy write block.
    // -# pszTailPattern - [IN] the specific patterns in case-insensitive little-endian HEX string (like "001b2E") encrypted by iviTR_ENCRYPT_CONTENT(byte pattern = 0x71, encrypt type = 1). The decrypted binary pattern (0x001B2E not "001b2E") will be put at the tail of each dummy write block. NULL if no need to insert specific pattern at tail of the dummy write block.
    //
	// Return value - None\n
	//
	// Supported platform:		All Windows\n
	// Performance overhead:	High
	// Security level:			High
	// Usage scope:				Macro scope
	//
	// Note:\n
	//
	// (1) The caller thread may be blocked about 'nTotalThread * nSleepTimePerCycle' ms when call iviTR_END_DUMMY_WRITE().
	// (2) the nBlockSize depends on your critical memory write data size which you want to protect. For AACS key, it should be 16.
	// (3) the nTotalBlockPerThread depends on the hacker verification speed for each memory block. For AACS key, suppose that
	//     it will take 1 ms for hacker to verify if the memory block is a correct key. Then, for each memory dump sample, hacker will spend
	//     about (nTotalBlockPerThread * nTotalThread) * 1 ms. If nTotalThread is 4 and nTotalBlockPerThread is 500000, it will take about
	//     500000 * 4 * 1 ms = 2000 sec = 33 min = 0.5 hr. If hacker needs to sample 1000 times to catch the memory snapshot containing the real key,
	//     it will take 0.5 hr * 1000 = 500 hr = about 21 days. It will frustrate hacker a lot.
	//
	// Usage:\n
	//
    // iviTR_START_DUMMY_WRITE(16, 500000, 4, 0, NULL, NULL, NULL)
	//
	// [start to do critical but time-insensitive memory write job]
	//
	// iviTR_END_DUMMY_WRITE()
	//*/
	#define iviTR_START_DUMMY_WRITE(nBlockSize, nTotalBlockPerThread, nTotalThread, nSleepTimePerCycle, pszHeadPattern, pszMidPattern, pszTailPattern) \
		{ \
		    trStartDummyWrite(nBlockSize, nTotalBlockPerThread, nTotalThread, nSleepTimePerCycle, pszHeadPattern, pszMidPattern, pszTailPattern); \
		}


	///*!
	// Stop dummy memory write process
	//
	// Return value - None\n
	//
	// Supported platform:		All Windows\n
	// Performance overhead:	High
	// Security level:			High
	// Usage scope:				Macro scope
	//
	// Usage:\n
	//
    // iviTR_START_DUMMY_WRITE()
	//
	// [start to do critical but time-insensitive memory write job]
	//
	// iviTR_END_DUMMY_WRITE()
	//*/
	#define iviTR_END_DUMMY_WRITE() \
		{ \
		    trEndDummyWrite(); \
		}

#elif defined(__linux__)
	#define iviTR_START_DUMMY_WRITE(nBlockSize, nTotalBlockPerThread, nTotalThread, nSleepTimePerCycle, pszHeadPattern, pszMidPattern, pszTailPattern)
	#define iviTR_END_DUMMY_WRITE()
#endif

#endif // !defined(AFX_IVIDUMMYACCESS_H__0454D94D_C00F_4982_9788_8C77B02D9572__INCLUDED_)
