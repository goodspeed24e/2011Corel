//=============================================================================
//  THIS SOURCE CODE IS PROPRIETARY INFORMATION BELONGING TO INTERVIDEO, INC.
//  ANY USE INCLUDING BUT NOT LIMITED TO COPYING OF CODE, CONCEPTS, AND/OR
//  ALGORITHMS IS PROHIBITED EXCEPT WITH EXPRESS WRITTEN PERMISSION BY THE
//  COMPANY.
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
//  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
//  PURPOSE.
//
//  Copyright (c) 1998 - 2007  InterVideo Corporation.  All Rights Reserved.
//
//-----------------------------------------------------------------------------

#ifndef _IVIVERIFYCHECKSUMREGION_H
#define _IVIVERIFYCHECKSUMREGION_H

#include "IVITrWinDef.h"
#include "IVITRComm.h"

#ifdef _WIN32
	#include "IVICheck.h"
	
NS_TRLIB_BEGIN
	
	///*!
	// [Internal function. Do not call it.]
	//
	// To create a thread to verify the checksum of specific region.\n
	//
	//-# pfnStart - [IN] a pointer to region begin\n
	//-# pfnEnd - [IN] a pointer to region end.\n
	//-# trRelocatTable - [IN] a table point to memory that can be relocated\n
	//-# trChecksumValue - [IN] checksum value in DWORD.\n
	//-# trChecksumValueSeed - [IN] checksum seed number in DWORD.\n
	//-# trChecksumEnable - [IN] checksum enable value in DWORD.\n
	//-# trChecksumEnableSeed - [IN] checksum enable seed number in DWORD.\n
	//-# pfnChangeCallback - [IN] a callback function implemented by caller.\n
	//-# pArg - [IN] a pointer to caller self.\n
	// Return value - [LPVOID] a pointer to a specific thread object.\n
	// Supported platform: All Windows\n
	//*/
	LPVOID WINAPI CreateVerifyCheckSumThread(
		LPVOID pfnStart,					// i.e TR_RegionBegin_57
		LPVOID pfnEnd,						// i.e TR_RegionEnd_57
		LPWORD trRelocatTable,				// i.e TR_RelocateTable_57
		DWORD trChecksumValue,				// i.e TR_ChecksumValue_57
		DWORD trChecksumSeed,				// i.e TR_CHECKSUM_SEED_57
		DWORD trChecksumValueSeed,			// i.e TR_CHECKSUM_VALUE_57
		DWORD trChecksumEnable,				// i.e TR_ChecksumEnable_57
		DWORD trChecksumEnableSeed,			// i.e TR_CHECKSUM_ENABLE_57,
		CALLBACK_CHANGED pfnChangeCallback, // i.e crashApp
		LPVOID pArg);						// i.e this

	///*!
	// [Internal function. Do not call it.]
	//
	// To create a thread to verify the checksum of specific region.\n
	//
	//-# pfnStart - [IN] a pointer to region begin\n
	//-# pfnEnd - [IN] a pointer to region end.\n
	//-# trRelocatTable - [IN] a table point to memory that can be relocated\n
	//-# trChecksumValue - [IN] checksum value in DWORD.\n
	//-# trChecksumValueSeed - [IN] checksum seed number in DWORD.\n
	//-# trChecksumEnable - [IN] checksum enable value in DWORD.\n
	//-# trChecksumEnableSeed - [IN] checksum enable seed number in DWORD.\n
	//-# pfnChangeCallback - [IN] a callback function implemented by caller.\n
	//-# pArg - [IN] a pointer to caller self.\n
	// Return value - [LPVOID] a pointer to a specific thread object.\n
	// Supported platform: All Windows\n
	//*/
	LPVOID WINAPI CreateVerifyCheckSumThreadEX(
		LPVOID pfnStart,					// i.e TR_RegionBegin_57
		LPVOID pfnEnd,						// i.e TR_RegionEnd_57
		LPWORD trRelocatTable,				// i.e TR_RelocateTable_57
		DWORD trChecksumValue,				// i.e TR_ChecksumValue_57
		DWORD trChecksumSeed,				// i.e TR_CHECKSUM_SEED_57
		DWORD trChecksumValueSeed,			// i.e TR_CHECKSUM_VALUE_57
		DWORD trChecksumEnable,				// i.e TR_ChecksumEnable_57
		DWORD trChecksumEnableSeed,			// i.e TR_CHECKSUM_ENABLE_57,
		DWORD trImageBaseOffset, 
		CALLBACK_CHANGED pfnChangeCallback, // i.e crashApp
		LPVOID pArg);						// i.e this

	///*!
	// [Internal function. Do not call it.]
	//
	// To delete a thread created to verify checksum.\n
	//
	//-# pArg - [IN] a object created to detect checksum before\n
	// Return value - [HRESULT] return S_OK if success else E_POINTER.\n
	// Supported platform: All Windows\n
	//*/
	HRESULT WINAPI DeleteVerifyCheckSumThread(LPVOID pArg);

	///*!
	// [Internal function. Do not call it.]
	//
	// To delete a thread created to verify checksum.\n
	//
	//-# pArg - [IN] a object created to detect checksum before\n
	// Return value - [HRESULT] return S_OK if success else E_POINTER.\n
	// Supported platform: All Windows\n
	//*/
	HRESULT WINAPI DeleteVerifyCheckSumThreadEX(LPVOID pArg);
	
NS_TRLIB_END

	///*!
	// [Internal use. Do not call it.]
	//
	// To create a thread to verify the checksum of specific region.\n
	//
	//-# pfnStart - [IN] a pointer to region begin\n
	//-# pfnEnd - [IN] a pointer to region end.\n
	//-# trRelocatTable - [IN] a table point to memory that can be relocated\n
	//-# trChecksumValue - [IN] checksum value in DWORD.\n
	//-# trChecksumValueSeed - [IN] checksum seed number in DWORD.\n
	//-# trChecksumEnable - [IN] checksum enable value in DWORD.\n
	//-# trChecksumEnableSeed - [IN] checksum enable seed number in DWORD.\n
	//-# pfnChangeCallback - [IN] a callback function implemented by caller.\n
	//-# pArg - [IN] a pointer to caller self.\n
	// Return value - [LPVOID] a pointer to a specific thread object.\n
	//
	// Supported platform:		All Windows
	// Performance overhead:	Low
	// Security level:			Medium
	// Usage scope:				Global/process scope
	//
	//Example of how to use: 
	//
	//In Constructor:
	//	LPVOID m_pVCT = iviTR_CREATE_VERIFY_CHECKSUM_REGION(
	//		TR_RegionBegin_57, 
	//		TR_RegionEnd_57, 
	//		TR_RelocateTable_57, 
	//		TR_ChecksumValue_57,
	//		TR_CHECKSUM_SEED_57,
	//		TR_CHECKSUM_VALUE_57,
	//		TR_ChecksumEnable_57,
	//		TR_CHECKSUM_ENABLE_57,
	//		crashApp,
	//		this);
  //
	//In Destructor:
	//	iviTR_DELETE_VERIFY_CHECKSUM_REGION(m_pVCT);
	//
	//*/
	#define iviTR_CREATE_VERIFY_CHECKSUM_REGION(pfnStart, pfnEnd, trRelocatTable, trChecksumValue,trChecksumSeed, trChecksumValueSeed, trChecksumEnable, trChecksumEnableSeed, pfnChangeCallback, parg) \
			CreateVerifyCheckSumThreadEX((pfnStart), (pfnEnd), (trRelocatTable), (trChecksumValue),(trChecksumSeed),(trChecksumValueSeed),(trChecksumEnable), (trChecksumEnableSeed), (DO_GETUNDOOFFSET), (pfnChangeCallback),(parg))


	///*!
	//
	// To create a thread to verify the checksum of specific region.\n
	//
	//-# num - [IN] the number of region in DWORD.\n
	//-# pfnChangeCallback - [IN] a callback function implemented by caller.\n
	//-# pArg - [IN] a pointer to caller self.\n
	// Return value - [LPVOID] a pointer to a specific thread object.\n
	//
	// Supported platform:		All Windows
	// Performance overhead:	Low
	// Security level:			Medium
	// Usage scope:				Global/process scope
	//
	//Example of how to use: 
	//
	//In Constructor:
	//	LPVOID m_pVCT = iviTR_CREATE_VERIFY_CHECKSUM_REGION_EX(
	//		57,
	//		crashApp,
	//		this);
	#define iviTR_CREATE_VERIFY_CHECKSUM_REGION_EX(region_num, pfnChangeCallback, parg) \
			iviTR_CREATE_VERIFY_CHECKSUM_REGION(TR_RegionBegin_##region_num, TR_RegionEnd_##region_num, TR_RelocateTable_##region_num, TR_ChecksumValue_##region_num, TR_CHECKSUM_SEED_##region_num, TR_CHECKSUM_VALUE_##region_num, TR_ChecksumEnable_##region_num, TR_CHECKSUM_ENABLE_##region_num, pfnChangeCallback, parg)

	///*!
	//
	// To delete a thread created to verify checksum.\n
	//
	//-# pVCT - [IN] a object created to detect checksum before\n
	// Return value - [HRESULT] return S_OK if success else E_POINTER.\n
	//
	// Supported platform:		All Windows
	// Performance overhead:	Low
	// Security level:			None
	// Usage scope:				Global/process scope
	//
	//*/
	#define iviTR_DELETE_VERIFY_CHECKSUM_REGION(pVCT) \
		DeleteVerifyCheckSumThreadEX(pVCT)



#elif defined(__linux__)
	#define iviTR_CREATE_VERIFY_CHECKSUM_REGION(pfnStart, pfnEnd, trRelocatTable, trChecksumValue,trChecksumSeed, trChecksumValueSeed, trChecksumEnable, trChecksumEnableSeed, pfnChangeCallback, parg) 0
	#define iviTR_DELETE_CHECK_PROCESS_MEMORY_THREAD(pmt)
#endif // _WIN32

#endif // _IVIVERIFYCHECKSUMREGION_H
