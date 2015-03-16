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

#ifndef _IVICHECKPROCESSMEMORY_H
#define _IVICHECKPROCESSMEMORY_H

#include "IVITrWinDef.h"
#include "IVITRComm.h"

#ifdef _WIN32
	#include "IVICheck.h"

NS_TRLIB_BEGIN

	///*!
	// [Internal function. Do not call it.]
	//
	// get check checksum value of a piece of memory.
	//
	// Return value - [DWORD] return checksum value.
	//
	// -#pstart_addr - [IN] can not be NULL, start address of memory.
	// -#pend_addr - [IN] can not be NULL, end address of memory.
	// -#dw_seed - [IN] seed number in DWORD value.
	//
	// Supported platform: All Windows
	//*/
	DWORD __cdecl TR_GetCheckSum(LPVOID pstart_addr, LPVOID pend_addr, DWORD dw_seed);

	///*!
	// [Internal function. Do not call it.]
	//
	// create thread for background checking checksum value of a piece of memory.
	//
	// Return value - [LPVOID] internal thread instance, NULL will be a error.
	//
	// -#timer - [IN] can not be NULL, Mini-sec, return value in DWORD.
	// -#addr_start - [IN] can not be NULL, start address of memory.
	// -#addr_end - [IN] can not be NULL, end address of memory.
	// -#same - [IN] can not be NULL, if no any change callback function.
	// -#changed - [IN] can not be NULL, if any changed callback function.
	// -#pArg - [IN] user-defined parameter, in LPVOID.
	//
	// Supported platform: All Windows
	//*/
	LPVOID WINAPI CreateProcessCheckThread(
		DWORD timer, 
		LPVOID addr_start, 
		LPVOID addr_end, 
		CALLBACK_SAME same, 
		CALLBACK_CHANGED changed, 
		LPVOID pArg);

	///*!
	// [Internal function. Do not call it.]
	//
	// delete process checking thread instance 
	//
	// Return value - [HRESULT] S_OK will be success.
	//
	// -#pArg - [IN] instance of checking process thread, in LPVOID.
	//
	// Supported platform: All Windows
	//*/
	HRESULT WINAPI DeleteProcessCheckThread(LPVOID pArg);

NS_TRLIB_END

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	#define TR_CHECKSUM_SEED_0	0x1f78cfad

	///*!
	// get check checksum value of a piece of memory.
	//
	// Return value - None
	//
	// -#val - [IN] can not be NULL, return value in DWORD.
	// -#start_addr - [IN] can not be NULL, start address of memory.
	// -#end_addr - [IN] can not be NULL, end address of memory.
	// -#seed - [IN] seed number in DWORD value.
	//
	// Supported platform:		All Windows
	// Performance overhead:	Low
	// Security level:			Low
	// Usage scope:				Macro scope
	//*/
	#define iviTR_MEM_CHECKSUM(val, start_addr, end_addr, seed)\
		val = TR_GetCheckSum((LPVOID)start_addr, (LPVOID)end_addr, (DWORD)seed);

	///*!
	// [this macro is unstable so far. do not use it !!]
	//
	// Create thread for background checking checksum value of a piece of memory.
	//
	// Return value - [LPVOID] internal thread instance, NULL will be a error.
	//
	// -#timer - [IN] can not be NULL, Mini-sec, return value in DWORD.
	// -#s_addr - [IN] can not be NULL, start address of memory.
	// -#e_addr - [IN] can not be NULL, end address of memory.
	// -#call1 - [IN] can not be NULL, if no any change callback function.
	// -#call2 - [IN] can not be NULL, if any changed callback function.
	// -#pArg - [IN] user-defined parameter, in LPVOID.
	//
	// Supported platform:		All Windows
	// Performance overhead:	Medium
	// Security level:			Low
	// Usage scope:				Macro scope
	//
	// Note:\n
	// [This macro is not stable. Please do not use it so far.]
	//
	// Remember use iviTR_DELETE_CHECK_PROCESS_MEMORY_THREAD to delete thread internal.
	// call1 must be type "void (CALLBACK * CALLBACK_SAME)(LPVOID);"
	// call2 must be type "void (CALLBACK * CALLBACK_CHANGED)(LPVOID);"
	//*/
	/*
	#define iviTR_CREATE_CHECK_PROCESS_MEMORY_THREAD(timer, s_addr, e_addr, call1, call2, parg) \
		CreateProcessCheckThread((timer), (s_addr), (e_addr), (call1), (call2), (parg))
	*/
	#define iviTR_CREATE_CHECK_PROCESS_MEMORY_THREAD(timer, s_addr, e_addr, call1, call2, parg)


	///*!
	// [this macro is unstable so far. do not use it !!]
	//
	// Delete thread instance which created by iviTR_DELETE_CHECK_PROCESS_MEMORY_THREAD.
	//
	// Return value - [HRESULT] S_OK will be success.
	//
	// -#pmt - [IN] thread instance get by iviTR_CREATE_CHECK_PROCESS_MEMORY_THREAD.
	//
	// Supported platform:		All Windows
	// Performance overhead:	Medium
	// Security level:			Low
	// Usage scope:				Macro scope
	//
	// Note:\n
	// [This macro is not stable. Please do not use it so far.]
	//*/
	/*
	#define iviTR_DELETE_CHECK_PROCESS_MEMORY_THREAD(pmt)\
		DeleteProcessCheckThread((pmt));
	*/
	#define iviTR_DELETE_CHECK_PROCESS_MEMORY_THREAD(pmt)


#elif defined(__linux__)
	#define iviTR_MEM_CHECKSUM(val, start_addr, end_addr, seed)
	#define iviTR_CREATE_CHECK_PROCESS_MEMORY_THREAD(timer, s_addr, e_addr, call1, call2, parg)
	#define iviTR_DELETE_CHECK_PROCESS_MEMORY_THREAD(pmt)
#endif // _WIN32

#endif // _IVICHECKPROCESSMEMORY_H
