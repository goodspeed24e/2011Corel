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

#ifndef _IVICHECKWINDOWNAMES_H
#define _IVICHECKWINDOWNAMES_H

#include "IVITRComm.h"

#ifdef _WIN32
	#include "IVICheck.h"

	///*!
	// [Internal function. Do not call it.]
	//
	// Check current running windows name, is possible a debugger exist 
	//
	// Return value - None
	//
	// -#fnMatchCallBack - [IN] can not be NULL, if match callback function.
	// -#fnNoMatchCallBack - [IN] can not be NULL, if not match callback function.
	// -#parg - [IN] user-defined parameter, in LPVOID.
	//
	// Supported platform: All Windows
	//*/
	void __cdecl CheckAllWindowNames(CALLBACK_MATCH fnMatchCallBack, CALLBACK_NOMATCH fnNoMatchCallBack, LPVOID parg);

	///*!
	// [Internal function. Do not call it.]
	//
	// Check current running windows name by a string list
	//
	// Return value - [DWORD] S_OK will be success.
	//
	// -#match_list - [IN] can not be NULL, a compared string list of windows name.
	// -#is_match - [IN] can not be NULL, if match callback function.
	// -#is_notmatch - [IN] can not be NULL, if not match callback function.
	// -#pArg - [IN] user-defined parameter, in LPVOID.
	//
	// Supported platform: All Windows
	//*/
	DWORD WINAPI CheckWindowNames(char *match_list, CALLBACK_MATCH is_match, CALLBACK_NOMATCH is_notmatch, LPVOID pArg);



	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	///*!
	// [this macro is unstable so far. do not use it !!]
	//
	// Check current running windows name by a string list
	//
	// -#call1 - [IN] can not be NULL, if match callback function.
	// -#call2 - [IN] can not be NULL, if not match callback function.
	// -#parg - [IN] user-defined parameter, in LPVOID.
	//
	// Supported platform:		All Windows
	// Performance overhead:	High
	// Security level:			Low
	// Usage scope:				Macro scope
	//
	// Note:\n
	// call1 must be type "void (CALLBACK * CALLBACK_MATCH)(LPVOID);"
	// call2 must be type "void (CALLBACK * CALLBACK_NOMATCH)(LPVOID);"
	// Not useful for anti-hacker, mark off currently.
	//*/
	// #define iviTR_CHECK_ALL_WINDOW_NAMES(call1,call2, parg) {CheckAllWindowNames((call1), (call2),(parg));}
	#define iviTR_CHECK_ALL_WINDOW_NAMES(call1,call2, parg) {}

	///*!
	// [this macro is unstable so far. do not use it !!]
	//
	// Check current running windows name by a string list
	//
	// -#match - [IN] can not be NULL, a compared string point of windows name.
	// -#call1 - [IN] can not be NULL, if match callback function.
	// -#call2 - [IN] can not be NULL, if not match callback function.
	// -#parg - [IN] user-defined parameter, in LPVOID.
	//
	// Supported platform:		All Windows
	// Performance overhead:	High
	// Security level:			Low
	// Usage scope:				Macro scope
	//
	// Note:\n
	// call1 must be type "void (CALLBACK * CALLBACK_MATCH)(LPVOID);"
	// call2 must be type "void (CALLBACK * CALLBACK_NOMATCH)(LPVOID);"
	// Not useful for anti-hacker, mark off currently.
	//*/
	// #define iviTR_CHECK_WINDOW_NAMES(match, call1, call2, parg) {CheckWindowNames((match), (call1), (call2),(parg));}
	#define iviTR_CHECK_WINDOW_NAMES(match, call1, call2, parg) {}

#elif defined(__linux__)
	#define iviTR_CHECK_ALL_WINDOW_NAMES(call1,call2, parg)
	#define iviTR_CHECK_WINDOW_NAMES(match, call1, call2, parg)
#endif // _WIN32

#endif // _IVICHECKWINDOWNAMES_H
