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
#ifndef _IVI_TIMINGMACROS_H
#define _IVI_TIMINGMACROS_H

#include "IVITrWinDef.h"
#include "IVITRComm.h"
#include "IVIDummyCall.h"

///*!
// [Internal function. Do not call it.]
//
// Init the max count for checking time-out, should be 1 at least.
//
// Return value - None
//
// -#nMaxCount - [IN] Set the maximum millisecond of time out.
//
// Supported platform: All Windows/Linux
//*/
// Mark off as the code is moved to the header.
//void __cdecl TR_SetTimeoutCount(int nMaxCount);

///*!
// [Internal function. Do not call it.]
//
// current time tick from system.
//
// Return value - [DWORD] time tick of system.
//
// Supported platform: All Windows/Linux
//*/
// Mark off as the code is moved to the header.
//DWORD __cdecl TR_GetTimeTick();

///*!
// [Internal function. Do not call it.]
//
// retun checking result, boolean value, true is pass, or not.
//
// Return value - [bool] true is checking pass, or not.
//
// -#dwDiff - [IN] set time different in DWORD.
// -#dwLimit - [IN] set time different limit value in DWORD.
//
// Supported platform: All Windows/Linux
//*/
// Mark off as the code is moved to the header.
//bool __cdecl TR_CheckTimeStamp(DWORD dwDiff, DWORD dwLimit);

NS_TRLIB_BEGIN

extern int timeoutcount;
extern int maxtimeoutvalue;

NS_TRLIB_END

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

///*!
// Init the max count for checking time-out, should be 1 at least.
//
// Return value - None
//
// -#int_variable - [IN] can not be NULL, the maximum millisecond of time out.
//
// Supported platform:		All Windows/Linux
// Performance overhead:	Low
// Security level:			Medium
// Usage scope:				Block scope			
//
// Note:\n
// use with iviTR_SET_INITIAL_TIMESTAMP, iviTR_SET_FINAL_TIMESTAMP, iviTR_TIMESTAMP_CHECK
// depends on CPU clock, please set proper time gap.
//
// Usage:\n
// void Sample() 
// {
// 	 iviTR_SET_TIMEOUT_COUNT(5000)
//   ...
//   DWORD t1, t2;
// 		iviTR_SET_INITIAL_TIMESTAMP(t1)
//
//		// put real working code here
//
// 		iviTR_SET_FINAL_TIMESTAMP(t2)
//   ...
//   bool bcheck = false;
//   iviTR_TIMESTAMP_CHECK(t1, t2, 5000, bcheck)
//   if (bcheck)
// 		// do something heinous (but untraceable)
// }
//
//*/
// Mark off as the code is moved to the header.
//#define iviTR_SET_TIMEOUT_COUNT(int_variable) { TR_SetTimeoutCount(int_variable); }
#define iviTR_SET_TIMEOUT_COUNT(int_variable) { timeoutcount = int_variable; maxtimeoutvalue = int_variable; }

///*!
// get first time tick from system.
//
// Return value - None
//
// -#t1 - [OUT] can not be NULL, get current system tick value in DWORD..
//
// Supported platform:		All Windows/Linux
// Performance overhead:	Low
// Security level:			Medium
// Usage scope:				Macro scope
//
// Note:\n
// use with iviTR_SET_TIMEOUT_COUNT, iviTR_SET_FINAL_TIMESTAMP, iviTR_TIMESTAMP_CHECK
// depends on CPU clock, please set proper time gap.
//
// Usage:\n
// void Sample() 
// {
// 	 iviTR_SET_TIMEOUT_COUNT(5000)
//   ...
//   DWORD t1, t2;
// 		iviTR_SET_INITIAL_TIMESTAMP(t1)
//
//		// put real working code here
//
// 		iviTR_SET_FINAL_TIMESTAMP(t2)
//   ...
//   bool bcheck = false;
//   iviTR_TIMESTAMP_CHECK(t1, t2, 5000, bcheck)
//   if (bcheck)
// 		// do something heinous (but untraceable)
// }
//*/
// Mark off as the code is moved to the header.
//#define iviTR_SET_INITIAL_TIMESTAMP(t1) { t1 = TR_GetTimeTick(); }
#ifdef _WIN32
	#define iviTR_SET_INITIAL_TIMESTAMP(t1) { t1 = GetTickCount(); }
#elif defined(__linux__)
	#define iviTR_SET_INITIAL_TIMESTAMP(t1) { t1 = 0; }
#endif //_WIN32

///*!
// get second time tick from system.
//
// Return value - None
//
// -#t2 - [OUT] can not be NULL, get current system tick value in DWORD.
//
// Supported platform:		All Windows/Linux
// Performance overhead:	Low
// Security level:			Medium
// Usage scope:				Macro scope
//
// Note:\n
// use with iviTR_SET_TIMEOUT_COUNT, iviTR_SET_INITIAL_TIMESTAMP, iviTR_TIMESTAMP_CHECK
// depends on CPU clock, please set proper time gap.
//
// Usage:\n
// void Sample() 
// {
// 	 iviTR_SET_TIMEOUT_COUNT(5000)
//   ...
//   DWORD t1, t2;
// 		iviTR_SET_INITIAL_TIMESTAMP(t1)
//
//		// put real working code here
//
// 		iviTR_SET_FINAL_TIMESTAMP(t2)
//   ...
//   bool bcheck = false;
//   iviTR_TIMESTAMP_CHECK(t1, t2, 5000, bcheck)
//   if (bcheck)
// 		// do something heinous (but untraceable)
// }
//*/
// Mark off as the code is moved to the header.
//#define iviTR_SET_FINAL_TIMESTAMP(t2) { t2 = TR_GetTimeTick(); }
#ifdef _WIN32
	#define iviTR_SET_FINAL_TIMESTAMP(t2) {t2 = GetTickCount(); }
#elif defined(__linux__)
	#define iviTR_SET_FINAL_TIMESTAMP(t2) { t2 = 0; }
#endif //_WIN32

///*!
// check if time difference exceeds the limitation. bool_variable == true if expires. This can be used to check
// if any debugger is debugging current program.
//
// Return value - None
//
// -#t1 - [IN] can not be NULL, set first get system tick value in DWORD.
// -#t2 - [IN] can not be NULL, set second get system tick value in DWORD.
// -#dword_variable - [IN] set time different limit value in DWORD.
// -#bool_variable - [OUT] return value, assign a boolean variable prepared by caller.
//
// Supported platform:		All Windows/Linux
// Performance overhead:	Low
// Security level:			Medium
// Usage scope:				Macro scope
//
// Note:\n
// use use with iviTR_SET_TIMEOUT_COUNT, iviTR_SET_INITIAL_TIMESTAMP, iviTR_TIMESTAMP_CHECK
// depends on CPU clock, please set proper time gap.
//
// [2006/11/17] A bug fix to deal with the situation that time count warps around to zero
// after the system being up for more than 49.7 days.
//
// Usage:\n
// void Sample() 
// {
// 	 iviTR_SET_TIMEOUT_COUNT(5000)
//   ...
//   DWORD t1, t2;
// 		iviTR_SET_INITIAL_TIMESTAMP(t1)
//
//		// put real working code here
//
// 		iviTR_SET_FINAL_TIMESTAMP(t2)
//   ...
//   bool bcheck = false;
//   iviTR_TIMESTAMP_CHECK(t1, t2, 5000, bcheck)
//   if (bcheck)
// 		// do something heinous (but untraceable)
// }
//*/
// Mark off as the code is moved to the header.
/*
#define iviTR_TIMESTAMP_CHECK(t1, t2, dword_variable, bool_variable)\
{\
	if (t1 > t2)															\
		{ bool_variable = false; }											\
	else																	\
	{																		\
		DWORD temptimecheck = t2 - t1;										\
		bool_variable = TR_CheckTimeStamp(temptimecheck, dword_variable);	\
	}																		\
	\
    iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bool_variable, iviTR_TIMESTAMP_CHECK) \
}
*/

#define iviTR_TIMESTAMP_CHECK(t1, t2, dword_variable, bool_variable) \
{ \
	DWORD temptimecheck = t2 - t1; \
	iviTR_DUMMY_CALL_1(41) \
	bool_variable = false;	\
	if (temptimecheck < 0 || temptimecheck > dword_variable) \
	{ \
		timeoutcount--; \
	} \
	if (timeoutcount <= 0 || timeoutcount > maxtimeoutvalue) \
	{ \
		iviTR_DUMMY_CALL_3(20) \
		bool_variable = true; \
	} \
	else \
	{ \
		bool_variable = false; \
	} \
}

/******************
 *ReadRDTSC USAGE*
 ******************

ULONGLONG ullStart, ullEnd;
iviTR_READ_RDTSC(1,ullStart);

...do some work...

iviTR_READ_RDTSC(2,ullEnd);

...check delta...
***************************************/

#define iviTR_READ_RDTSC(n, ts)	{}
// 	static int virtualCount##n;										\
// 	DWORD oldPtr##n;													\
// 	LPDWORD instrPtr##n;												\
// 	LPBYTE p##n = (LPBYTE)&ts;											\
// 	DWORD oldMask##n = ::SetThreadAffinityMask(::GetCurrentThread(), 1);	\
// 																		\
// 	if(virtualCount##n == 0)											\
// 	{																	\
// 		_asm															\
// 		{																\
// 			_asm push eax												\
// 			_asm call next_line##n										\
// 			_asm next_line##n:											\
// 			_asm pop eax												\
// 			_asm mov instrPtr##n, eax									\
// 			_asm pop eax												\
// 		}																\
// 																		\
// 		::VirtualProtect(instrPtr##n, 1<<5, PAGE_READWRITE, &oldPtr##n);	\
// 		virtualCount##n = 1;														\
// 	}																	\
// 	_asm																\
// 	{																	\
// 		_asm push eax													\
// 		_asm push ebx													\
// 		_asm push edx													\
// 		_asm mov ebx, RDT##n											\
// 		_asm xor byte ptr [ebx], 0x0b									\
// 		_asm xor byte ptr [ebx+1], 0xa0									\
// 		_asm RDT##n:													\
// 		_asm add al, 0x91												\
// 		_asm xor byte ptr [ebx], 0x0b									\
// 		_asm xor byte ptr [ebx+1], 0xa0									\
// 		_asm mov ebx, p##n												\
// 		_asm mov  dword ptr [ebx], eax									\
// 		_asm mov  dword ptr [ebx+4], edx								\
// 		_asm pop edx													\
// 		_asm pop ebx													\
// 		_asm pop eax													\
// 	}																	\
// 	::SetThreadAffinityMask(GetCurrentThread(), oldMask##n); 

#endif // _IVI_TIMINGMACROS_H
