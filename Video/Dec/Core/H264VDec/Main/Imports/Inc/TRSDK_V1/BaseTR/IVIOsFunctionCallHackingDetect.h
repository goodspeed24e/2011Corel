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
//  Copyright (c) 1998 - 2006  InterVideo Corporation.  All Rights Reserved.
//
//-----------------------------------------------------------------------------
#if !defined(__IVI_OS_FUNCTION_CALL_HACKING_DETECT_H__)
#define __IVI_OS_FUNCTION_CALL_HACKING_DETECT_H__

#include "IVITrWinDef.h"
#include "IVITRComm.h"

NS_TRLIB_BEGIN

///*!
// [Internal function. Do not call it.]
//
// Get IAT checksume value for all current loaded modules
// for the calling process.
//
// -#rdwChecksum - [IN/OUT] multi-thread region handle.
// Return value - [bool] true will be success.
//
// Supported platform: All Windows
//*/
bool __cdecl GetIatChecksum(DWORD *pdwChecksum);

///*!
// [Internal function. Do not call it.]
//
// Get IAT checksum value for the specific module.
//
//-#hModule - [IN] handle of the desired module.
//-#rdwChecksum - [IN/OUT] multi-thread region handle.
// Return value - [bool] true will be success.
//
// Supported platform: All Windows
//*/
bool __cdecl GetIatChecksumByModuleHandle(HMODULE hModule, DWORD *pdwChecksum);

NS_TRLIB_END

#if !defined(__IMPORT_ADDRESS_TABLE_CHECKSUM_CALCULATOR_H__)
#define __IMPORT_ADDRESS_TABLE_CHECKSUM_CALCULATOR_H__


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifdef _WIN32
	///*!
	// Get IAT checksume value for all current loaded module for the calling process.
	//
	//-#rdwChecksum - [IN/OUT] multi-thread region handle in DWORD data type.
	//-#bool_variable - [IN/OUT] return value, true if successful.
	// Return value - None
	//
	// Supported platform:		All Windows
	// Performance overhead:	High
	// Security level:			Low
	// Usage scope:				Macro scope
	//
	// Note:
	// Do not use it in performance sensitive sections.
	//
	//*/
	#define iviTR_GET_IAT_CRC32(rdwChecksum, bool_variable)	\
		{ \
			bool_variable = GetIatChecksum(&rdwChecksum); \
			iviTR_DUMP_ERROR_TR_MACRO_ENTRY(!bool_variable, iviTR_GET_IAT_CRC32) \
		}

	///*!
	// Get IAT checksum value for the speicific module.
	//
	//-#hModule - [IN] handle of the desired module.
	//-#rdwChecksum - [IN/OUT] multi-thread region handle.
	//-#bool_variable - [IN/OUT] return value, true if successful.
	// Return value - None
	//
	// Supported platform:		All Windows
	// Performance overhead:	High
	// Security level:			Medium
	// Usage scope:				Macro scope
	//
	// Note:
	// Do not use it in performance sensitive sections.
	//
	// Usage:
	// 1. Call this macro in DLL's entry point (e.g. DllMain()) and
	//    store the checksum value in a global DWORD variable. 
	// 2. Call this macro again in any other locations within the same
	//    module and store the checksum in a local DWORD variable.
	// 3. Compare the global and local DWORD variables above.  If they
	//    are not the same, the module's has been hacked by attackers
	//    and it's recommended to call iviTR_CRASH() to terminate the
	//    application.
	//*/	
	#define iviTR_GET_IAT_CRC32_BY_MODULE_HANDLE(hModule, rdwChecksum, bool_variable) \
		{ \
			bool_variable = GetIatChecksumByModuleHandle(hModule, &rdwChecksum); \
			iviTR_DUMP_ERROR_TR_MACRO_ENTRY(!bool_variable, iviTR_GET_IAT_CRC32_BY_MODULE_HANDLE) \
		}

#elif defined(__linux__)
	#define iviTR_GET_IAT_CRC32(rdwChecksum, bool_variable) {bool_variable = true;}
	#define iviTR_GET_IAT_CRC32_BY_MODULE_HANDLE(hModule, rdwChecksum, bool_variable)	\
		{bool_variable = true;}
#endif // _WIN32

#endif // __IMPORT_ADDRESS_TABLE_CHECKSUM_CALCULATOR_H__

#endif // __IVI_OS_FUNCTION_CALL_HACKING_DETECT_H__
