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

#ifndef _IVICHECKVTABLEHOOK_H
#define _IVICHECKVTABLEHOOK_H

#include "IVITrWinDef.h"
#include "IVITRComm.h"

#ifdef _WIN32

NS_TRLIB_BEGIN

	///*!
	// [Internal function. Do not call it.]
	//
	// Check if specified function entries in virtual function table
	// is being intercepted by attackers.
	//
	// Return value - [bool] true if it is, false is not.\n
	// Supported platform: All Windows\n
	//*/
	bool __cdecl gtrCheck_Vtable_Hook_On_Callback(DWORD dwRetAddr, LPVOID pIf, INT32 iIndex, INT32 nCount);

	///*!
	// [Internal function. Do not call it.]
	//
	// detect the hook by checking vtable status.\n
	//
	//-# pInterface - [IN] pointer of the expected interface be detected\n
	//-# dwFuncCount - [IN] indicate the numbers of function within the expected interface.\n
	//
	// Return value - [bool] true is the hook action exists, or not.\n
	// Supported platform: All Windows\n
	//*/

	bool gtrDetectVTableHook(void *pInterface, DWORD dwFuncCount);

NS_TRLIB_END	

	///*!
	// [Internal testing macro. Do not use it.]
	//
	// Check if a virtual function table is being hooked.
	//
	// Return value - None
	//
	// Supported platform:		All Windows
	// Performance overhead:	Medium
	// Security level:			Medium
	// Usage scope:				Macro scope
	//
	// Note:\n
	// Use iviTR_CHECK_VTABLE_HOOK_ON_CALLBACK macro to perform virtual function table
	// hooking check on specific function entries.  pIF is the interface pointer of the
	// virtual function table.  Caller specifies the function entries that want to be
	// checked with iIndex and nCount.  iIndex is the index of the function in the
	// table to start checking and nCount is the number of function entries that should
	// be checked starting from the function denoted by index.  It is important to be
	// noted that this macro can only be used within the callback function scope. Such
	// callback functions have to reside in the same module as the interface pointer
	// (pIF) points to.  If the callback functions are indirect calls from another 
	// module, the macro will return positive hooking detection, which is an incorrect
	// result.
	// 
	//*/
	#define iviTR_CHECK_VTABLE_HOOK_ON_CALLBACK(ptr_interface, int_index, int_count, bool_variable)	\
		DWORD __dwRetAddr1__;														\
		__asm {																		\
			__asm push eax															\
			__asm mov  eax, dword ptr [ebp+4]										\
			__asm mov  __dwRetAddr1__, eax											\
			__asm pop  eax															\
		}																			\
		bool_variable = gtrCheck_Vtable_Hook_On_Callback(__dwRetAddr1__, ptr_interface, int_index, int_count);

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	///*!
	// detect the hook by checking vtable status.\n
	//
	//-# ptr_interface - [IN] pointer of the expected interface be detected.\n
	//-# int_count - [IN] indicate the numbers of function within the expected interface.\n
	//-# bool_variable - [IN/OUT] return value, true : it is hooked, false : it is normal.\n
	// Return value - None\n
	//
	// Supported platform: All Windows\n
	// Note:\n
	// 
	//*/
	#define iviTR_DETECT_VTABLE_HOOK(ptr_interface, int_count, bool_variable) \
		{ \
	        bool_variable = gtrDetectVTableHook(ptr_interface, int_count); \
			iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bool_variable, iviTR_DETECT_VTABLE_HOOK) \
		}


#elif defined(__linux__)
	#define iviTR_CHECK_VTABLE_HOOK_ON_CALLBACK(ptr_interface, int_index, int_count, bool_variable) {bool_variable = false;}

	#define iviTR_DETECT_VTABLE_HOOK(ptr_interface, int_count, bool_variable) {bool_variable = false;}
#endif	// _WIN32

#endif	//_IVICHECKVTABLEHOOK_H