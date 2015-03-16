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
#ifndef _IVI_ANTIDEBUG_H
#define _IVI_ANTIDEBUG_H

#include "IVITRComm.h"

///*!
// [Internal function. Do not call it.]
//
// Define char change to dword
//*/
#define TR_CHAR2DWORD(a,b,c,d) ((a)|((b)<<8)|((c)<<16)|((d)<<24))

///*!
// [Internal function. Do not call it.]
//
// Verify SoftIce exist.
//
// Return value - [bool] true is exist, false is not.\n
// Supported platform: Win98/Linux\n
//*/
bool __cdecl TR_fnVerify_SoftIce();

///*!
// [Internal function. Do not call it.]
//
// Verify is there attached debugger exist.
//
// Return value - [bool] true is exist, false is not.
// Supported platform: Win98/Linux
//*/
bool __cdecl TR_fnVerify_Attached_Debugger();

///*!
// [Internal function. Do not call it.]
//
// Verify is there Vtune 3.0 profiler exist.
//
// Return value - [bool] true is exist, false is not.
// Supported platform: All Windows
//*/
bool __cdecl TR_fnVerify_VTune_30();

///*!
// [Internal function. Do not call it.]
//
// Force halt this process.
//
// Return value - None
// Supported platform: All Windows/Linux
//*/
void __cdecl TR_fnStdHaltThisProcess(int nExitCode = 0);

#ifdef _WIN32
	//#ifndef CLSID_DirectDraw
	//#include <strmif.h>
	//#endif // CLSID_DirectDraw

	interface IPin;

	///*!
	// [Internal function. Do not call it.]
	//
	// Test upper stream filter is a microsoft navigator pin or ivi navigator pin.
	//
	// Return value - [bool] true is exist, false is not.
	// Supported platform: All Windows(DirectX)
	//*/
	bool __cdecl TR_fnCheckIPin(IPin *pITestpin);
#endif // _WIN32

///*!
// [Internal class. Do not call it.]
//
// This is watch thread class that looks for the presence of a debugger.
//
// Supported platform: All Windows
//*/
class CTrWatchThd
{
public:
	CTrWatchThd();
	~CTrWatchThd();
	static DWORD __stdcall _TrWatchThd(LPVOID pArg);

private:
	HANDLE m_hThd, m_hTermEvt;
};

///*!
// [Internal function. Do not call it.]
//
// Get x86 system time clock cycles.
// for a 2GHz CPU, the result of this macro should increment by 1 for every 2 seconds.
//
// Supported platform: All Windows/Linux
//*/
DWORD __cdecl TR_GetRDTSC();

///*!
// [Internal function. Do not call it.]
//
// Get Pentium system time clock cycles.
// The variable returned is a 32 bit unsigned integer of the #cycles executed.
// divided by 16M. Hence, it increments by ~25 every second on a PII-400.
//
// Supported platform: All Windows/Linux
//*/
DWORD __cdecl TR_GetPentiumRDTSC();

///*!
// [Internal function. Do not call it.]
//
// Get x86 system time clock cycles.
//
// Return value - None
//
// -#dwRDTSC - [IN] can not be NULL, return RDTSC in DWORD.
//
// Supported platform:		All Windows/Linux
// Performance overhead:	Low
// Security level:			Low
// Usage scope:				Macro scope
//
// Note:\n
// for a 2GHz CPU, the result of this macro should increment by 1 for every 2 seconds.
//*/
#define GET_RDTSC(dwRDTSC) { dwRDTSC = TR_GetRDTSC(); }

///*!
// [Internal function. Do not call it.]
//
// Get x86 system time clock cycles. [Hidden version #1]
//
// Return value - None
//
// -#linenumber - [IN] can not be NULL, any number to assign a label name.
// -#dwRDTSC - [IN] can not be NULL, return RDTSC in DWORD.
//
// Supported platform:		All Windows/Linux
// Performance overhead:	Low
// Security level:			Low
// Usage scope:				Macro scope
//
// Note:\n
// for VC environment linenumber can use __LINE__ instead.
// please refer to GET_RDTSC macro.
//*/
#define HIDDEN_GET_RDTSC1(linenumber, dwRDTSC) \
	iviTR_FALSE_RETURN_1(linenumber)                     \
	GET_RDTSC(dwRDTSC)

///*!
// [Internal function. Do not call it.]
//
// Get x86 system time clock cycles. [Hidden version #2]
//
// Return value - None
//
// -#linenumber - [IN] can not be NULL, any number to assign a label name.
// -#dwRDTSC - [IN] can not be NULL, return RDTSC in DWORD.
//
// Supported platform:		All Windows/Linux
// Performance overhead:	Low
// Security level:			Low
// Usage scope:				Macro scope
// 
// Note:\n
// for VC environment linenumber can use __LINE__ instead.
// please refer to GET_RDTSC macro.
//*/
#define HIDDEN_GET_RDTSC2(linenumber, dwRDTSC) \
	iviTR_FALSE_RETURN_2(linenumber)                     \
	GET_RDTSC(dwRDTSC)

///*!
// [Internal function. Do not call it.]
//
// Get x86 system time clock cycles. [Hidden version #3]
//
// Return value - None
//
// -#linenumber - [IN] can not be NULL, any number to assign a label name.
// -#dwRDTSC - [IN] can not be NULL, return RDTSC in DWORD.
//
// Supported platform:		All Windows/Linux
// Performance overhead:	Low
// Security level:			Low
// Usage scope:				Macro scope
// 
// Note:\n
// for VC environment linenumber can use __LINE__ instead.
// please refer to GET_RDTSC macro.
//*/
#define HIDDEN_GET_RDTSC3(linenumber, dwRDTSC) \
	iviTR_MISALIGNMENT_1(linenumber)                     \
	GET_RDTSC(dwRDTSC)

///*!
// [Internal function. Do not call it.]
//
// Get x86 system time clock cycles. [Hidden version #3]
//
// Return value - None
//
// -#linenumber - [IN] can not be NULL, any number to assign a label name.
// -#dwRDTSC - [IN] can not be NULL, return RDTSC in DWORD.
//
// Supported platform:		All Windows/Linux
// Performance overhead:	Low
// Security level:			Low
// Usage scope:				Macro scope
// 
// Note:\n
// for VC environment linenumber can use __LINE__ instead.
// please refer to GET_RDTSC macro.
//*/
#define HIDDEN_GET_RDTSC4(linenumber, dwRDTSC) \
	iviTR_MISALIGNMENT_2(linenumber)                     \
	GET_RDTSC(dwRDTSC)



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

///*!
// Verify if SoftIce kernel debugger is installed in the system.
//
// Return value - None
// 
// -#proc - [IN] operation if Soft-Ice exist.
//
// Supported platform:		Win95/98
// Performance overhead:	Low
// Security level:			Low
// Usage scope:				Macro scope
//*/
#ifdef _WIN32
	#define iviTR_VERIFY_SOFTICE(proc) { if (TR_fnVerify_SoftIce()) { iviTR_DUMP_ERROR_TR_MACRO_ENTRY(true, iviTR_VERIFY_SOFTICE) proc; }}
#elif defined(__linux__)
	// Linux does not need to test for soft ice
	#define iviTR_VERIFY_SOFTICE(proc)
#endif //_WIN32

///*!
// Verify if debugger is debugging current program.
//
// Return value - None
// 
// -#proc - [IN] operation if debugger attached exist.
//
// Supported platform:		Win95/98
// Performance overhead:	Low
// Security level:			Low
// Usage scope:				Macro scope
//*/
#define iviTR_VERIFY_ATTACHED_DEBUGGER(proc) { if (TR_fnVerify_Attached_Debugger()) { iviTR_DUMP_ERROR_TR_MACRO_ENTRY(true, iviTR_VERIFY_ATTACHED_DEBUGGER) proc; }}

///*!
// Verify if Vtune 3.0 profiler is analyzing current program
//
// Return value - None
// 
// -#proc - [IN] operation if Vtune 3.0 run-time exist.
//
// Supported platform:		All Windows
// Performance overhead:	Low
// Security level:			Low
// Usage scope:				Macro scope
//*/
#ifdef _WIN32
	#define iviTR_VERIFY_VTUNE_30(proc) { if (TR_fnVerify_VTune_30()) { iviTR_DUMP_ERROR_TR_MACRO_ENTRY(true, iviTR_VERIFY_VTUNE_30) proc; }}
#elif defined(__linux__)
	#define iviTR_VERIFY_VTUNE_30(proc)
#endif // _WIN32

///*!
// Check if time expired reported from the watchdog thread.
//
// Return value - None
// 
// -#timeout_sec - [IN] specify time out limit in sec.
// -#proc - [IN] operation if exceed timer limit.
//
// Supported platform:		All Windows/Linux
// Performance overhead:	Low
// Security level:			Low
// Usage scope:				Macro scope
//
// Note:\n
// this macro is incomplete. Do not use it.
//*/
#ifdef _WIN32
	#define iviTR_CHECK_THREAD_TIMER(timeout_sec,proc) /* need win32 implementation */
#elif defined(__linux__)
	#define iviTR_CHECK_THREAD_TIMER(timeout_sec,proc)\
	{   extern volatile unsigned long TR_max_expire_delay; /* linuxtimer.cpp */ \
		if (TR_max_expire_delay > (timeout_sec)*1000) \
		{  /*printf("exiting!\n");*/ iviTR_DUMP_ERROR_TR_MACRO_ENTRY(true, iviTR_CHECK_THREAD_TIMER) proc; }\
	}
#endif // _WIN32

///*!
// Halt current process.
//
// Return value - None
//
// -#x - [IN] exit process return code, int value.
// 
// Supported platform:		All Windows/Linux
// Performance overhead:	Low
// Security level:			Low
// Usage scope:				Macro scope
//*/
#ifdef _WIN32
	#define iviTR_HALT_THIS_PROCESS(x) TR_fnStdHaltThisProcess(x)
#elif !defined(USE_MIPS) && !defined(NO_TR) && defined(__linux__)
	/* Linux */
	#define iviTR_HALT_THIS_PROCESS(x) TR_fnStdHaltThisProcess(x)
#endif // _WIN32

///*!
// Default halt current process.
//
// Return value - None
//
// -#x - [IN] exit process return code, int value.
// 
// Supported platform:		All Windows
// Performance overhead:	Low
// Security level:			Low
// Usage scope:				Macro scope
//
// Note:\n
// Linux: does not exit process immediately after checking for debugger
// since it's too easy to see how debugger was detected by looking
// at the output of the "strace" command.
//*/
#ifdef _WIN32
	#define iviTR_DEFAULT_ACTION iviTR_HALT_THIS_PROCESS(2)
#elif defined(__linux__)
	#define iviTR_DEFAULT_ACTION 
#endif // _WIN32

///*!
// Check if the testpin is connected to an upstream filter that IS NOT
// a Microsoft navigator pin or IVI navigator pin.
//
// Return value - None
//
// -#testpin - [IN] interface IPin point for test.
// -#flag - [OUT] return bool value, true if the testpin is cracked.
// 
// Supported platform:		All Windows
// Performance overhead:	Low
// Security level:			Medium
// Usage scope:				Macro scope
//*/
#ifdef _WIN32
	#define iviTR_CHECK_MICROSOFT_PIN(testpin, flag) { flag = TR_fnCheckIPin((IPin*)testpin); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(flag, iviTR_CHECK_MICROSOFT_PIN)}
#elif defined(__linux__)
	#define iviTR_CHECK_MICROSOFT_PIN(testpin, flag) {(flag)=0;}
#endif

/*
//
// Keep function return address.
//
// Return value - None
//
// Supported platform:		All Windows
// Performance overhead:	Medium
// Security level:			Medium
// Usage scope:				Function scope
//
// Note:\n
// READ_RETURN_ADDR must be placed near the beginning of the function.
// Use this with CHECK_RETURN_ADDR together.
// When you use these macros, you should check the compiled code to make
// sure there is no explicit operations on EBP (such as sub ebp,30h) in the function
// before the macros are called.
//
// emms instruction is only needed if you are not sure whether any MMX codes
// forgot to put emms at the end of the block.
//
// For safety, you should enclose your function with:
// #pragma optimize("y", off)
// ... your function ...
// #pragma optimize("", on)
//
// Usage:\n
//    void somefunction()
//    {
//       READ_RETURN_ADDR
//       ...
//       ...
//     	 __asm emms
//       if(!CHECK_RETURN_ADDR(TR_RegionBegin_xx, TR_RegionEnd_xx)) 
//          cause_delayed_instability;
//
// #ifndef READ_RETURN_ADDR
// #define READ_RETURN_ADDR              \
//     DWORD __dwRetAddr__;              \
// 	__asm push eax                    \
//     __asm mov  eax, dword ptr [ebp+4] \
//     __asm mov  __dwRetAddr__, eax     \
// 	__asm pop  eax
// #endif

//
// Check function real body by entry address distance.
//
// Return value - None
//
// Supported platform:		All Windows
// Performance overhead:	Medium
// Security level:			Medium
// Usage scope:				Macro scope
// 
// Note:\n
// Use this with READ_RETURN_ADDR together.
// When you use these macros, you should check the compiled code to make
// sure there is no explicit operations on EBP (such as sub ebp,30h) in the function
// before the macros are called.
//
// emms instruction is only needed if you are not sure whether any MMX codes
// forgot to put emms at the end of the block.
//
// For safety, you should enclose your function with:
// #pragma optimize("y", off)
// ... your function ...
// #pragma optimize("", on)
//
// Usage:\n
//    void somefunction()
//    {
//       READ_RETURN_ADDR
//       ...
//       ...
//     	 __asm emms
//       if(!CHECK_RETURN_ADDR(TR_RegionBegin_xx, TR_RegionEnd_xx)) 
//          cause_delayed_instability;
//
// #ifndef CHECK_RETURN_ADDR
// #define CHECK_RETURN_ADDR(regbegin, regend) \
// 	(((float)__dwRetAddr__ - (float)regbegin) * ((float)__dwRetAddr__ - (float)regend) <= 0.05f)
// #endif
*/


///*!
// The following macro gets the internal Pentium/PPro/PII timestamp counter
// (RDTSC) divided by 16Mcycles. The purpose of the macro is to be used in the synchronization code to
// detect whether or not play has been stopped in the middle of execution due
// to a kernel or system debugger.
//
// Return value - None
//
// -#dwRDTSC - [IN] can not be NULL, return RDTSC in DWORD.
//
// Supported platform:		All Windows/Linux
// Performance overhead:	Low
// Security level:			None
// Usage scope:				Macro scope
//
// Note:\n
// The variable returned is a 32 bit unsigned integer of the #cycles executed.
// divided by 16M. Hence, it increments by ~25 every second on a PII-400.
// Please take care about the overflow issue by yourself.
//*/
#define iviTR_GETRDTSC(dwRDTSC) { dwRDTSC = TR_GetPentiumRDTSC(); }

#endif // _IVI_ANTIDEBUG_H
