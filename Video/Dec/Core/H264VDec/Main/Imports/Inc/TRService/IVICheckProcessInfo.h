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

#ifndef _IVICHECKPROCESSINFO_H
#define _IVICHECKPROCESSINFO_H

#include "IVITRComm.h"

#ifdef _WIN32
	#include "IVICheck.h"

	#define CHECK_PROCESS_NAMES		0x00000000
	#define CHECK_PROCESS_MODULES	0x00000001

	// PSAPI declarations
	typedef BOOL (WINAPI *t_pfEnumProcesses)( DWORD *pidlist, DWORD bufSize, DWORD *bufNeeded );
	typedef BOOL (WINAPI *t_pfEnumProcessModules)( HANDLE hp, HMODULE *phm, DWORD cb, LPDWORD lpcbNeeded );
	typedef DWORD (WINAPI *t_pfGetModuleBaseName)( HANDLE hp, HMODULE hm, LPTSTR pName, DWORD bufSize );
	typedef DWORD (WINAPI *t_pfGetModuleFileNameEx)( HANDLE hp, HMODULE hm, LPTSTR pName, DWORD bufSize );

	///*!
	// [Internal function. Do not call it.]
	//
	// Check process information, if any match or not match, doing specified calling function.
	//
	// -#match_list - [IN] can not be NULL, match string list.
	// -#is_match - [IN] can not be NULL, matched callback function.
	// -#is_notmatch - [IN] can not be NULL, unmatched callback function.
	// -#flag - [IN] specified CHECK_PROCESS_NAMES or CHECK_PROCESS_MODULES
	// -#pArg - [IN] user-defined parameter, in LPVOID.
	// Return value - [bool] true is success or not.\n
	//
	// Supported platform: All Windows\n
	//*/
	bool WINAPI CheckProcessInfo(char *match_list, CALLBACK_MATCH is_match, CALLBACK_NOMATCH is_notmatch, DWORD flag, LPVOID pArg);

	///*!
	// [Internal function. Do not call it.]
	//
	// current-running process name match with specified string list.
	//
	// -#fnMatchCallBack - [IN] can not be NULL, matched callback function.
	// -#fnNoMatchCallBack - [IN] can not be NULL, unmatched callback function.
	// -#pArg - [IN] user-defined parameter, in LPVOID.
	//
	// Return value - None\n
	//
	// Supported platform: All Windows\n
	//*/
	void __cdecl CheckAllProcessNames(CALLBACK_MATCH fnMatchCallBack, CALLBACK_NOMATCH fnNoMatchCallBack, LPVOID pArg);

	///*!
	// [Internal function. Do not call it.]
	//
	// Check active process which loaded dll name possible is a debugger.
	//
	// -#fnMatchCallBack - [IN] can not be NULL, matched callback function.
	// -#fnNoMatchCallBack - [IN] can not be NULL, unmatched callback function.
	// -#pArg - [IN] user-defined parameter, in LPVOID.
	//
	// Return value - None\n
	//
	// Supported platform: All Windows\n
	//*/
	void __cdecl CheckAllModuleNames(CALLBACK_MATCH fnMatchCallBack, CALLBACK_NOMATCH fnNoMatchCallBack, LPVOID pArg);



	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	///*!
	// [this macro is unstable so far. do not use it !!]
	//
	// Check current-running process name match with specified string list.
	//
	// Return value - None
	//
	// -#match - [IN] can not be NULL, match string list.
	// -#call1 - [IN] can not be NULL, matched callback function.
	// -#call2 - [IN] can not be NULL, unmatched callback function.
	// -#parg - [IN] user-defined parameter, in LPVOID.
	//
	// Supported platform:		All Windows
	// Performance overhead:	High
	// Security level:			Low
	// Usage scope:				Macro scope
	//
	// Note:\n
	// Not useful for anti-hacker, mark off currently.
	//*/
	// #define iviTR_CHECK_PROCESS_NAMES(match, call1, call2, parg) {CheckProcessInfo((match), (call1), (call2), CHECK_PROCESS_NAMES, (parg));}
	#define iviTR_CHECK_PROCESS_NAMES(match, call1, call2, parg) {}

	///*!
	// [this macro is unstable so far. do not use it !!]
	//
	// Check active process which loaded dll name match with specified string list.
	//
	// Return value - None
	//
	// -#match - [IN] can not be NULL, match string list.
	// -#call1 - [IN] can not be NULL, matched callback function.
	// -#call2 - [IN] can not be NULL, unmatched callback function.
	// -#parg - [IN] user-defined parameter, in LPVOID.
	//
	// Supported platform:		All Windows
	// Performance overhead:	High
	// Security level:			Low
	// Usage scope:				Macro scope
	//
	// Note:\n
	// Not useful for anti-hacker, mark off currently.
	//*/
	// #define iviTR_CHECK_PROCESS_MODULES(match, call1, call2,parg) {CheckProcessInfo((match), (call1), (call2), CHECK_PROCESS_MODULES, (parg));}
	#define iviTR_CHECK_PROCESS_MODULES(match, call1, call2,parg) {}

	///*!
	// [this macro is unstable so far. do not use it !!]
	//
	// Check current-running process name possible is a debugger.
	//
	// Return value - None
	//
	// -#call1 - [IN] can not be NULL, matched callback function.
	// -#call2 - [IN] can not be NULL, unmatched callback function.
	// -#parg - [IN] user-defined parameter, in LPVOID.
	//
	// Supported platform:		All Windows
	// Performance overhead:	High
	// Security level:			Low
	// Usage scope:				Macro scope
	//
	// Note:\n
	// Not useful for anti-hacker, mark off currently.
	//*/
	// #define iviTR_CHECK_ALL_PROCESS_NAMES(call1, call2,parg) {CheckAllProcessNames((call1), (call2), (parg));}
	#define iviTR_CHECK_ALL_PROCESS_NAMES(call1, call2,parg) {}

	///*!
	// [this macro is unstable so far. do not use it !!]
	//
	// Check active process which loaded dll name possible is a debugger.
	//
	// Return value - None
	//
	// -#call1 - [IN] can not be NULL, matched callback function.
	// -#call2 - [IN] can not be NULL, unmatched callback function.
	// -#parg - [IN] user-defined parameter, in LPVOID.
	//
	// Supported platform:		All Windows
	// Performance overhead:	High
	// Security level:			Low
	// Usage scope:				Macro scope
	//
	// Note:\n
	// Not useful for anti-hacker, mark off currently.
	//*/
	// #define iviTR_CHECK_ALL_MODULE_NAMES(call1, call2,parg) {CheckAllModuleNames((call1), (call2), (parg));}
	#define iviTR_CHECK_ALL_MODULE_NAMES(call1, call2,parg) {}

#elif defined(__linux__)
	#define iviTR_CHECK_PROCESS_NAMES(match, call1, call2, parg)
	#define iviTR_CHECK_PROCESS_MODULES(match, call1, call2,parg)
	#define iviTR_CHECK_ALL_PROCESS_NAMES(call1, call2,parg)
	#define iviTR_CHECK_ALL_MODULE_NAMES(call1, call2,parg)
#endif // _WIN32

#endif // _IVICHECKPROCESSINFO_H
