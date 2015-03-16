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
#ifndef _IVI_ANTIDEBUG2_H
#define _IVI_ANTIDEBUG2_H

#include "IVITrWinDef.h"
#include "IVITRComm.h"
#include "IVIDummyCall.h"

#ifdef __linux__
	#include <sys/ptrace.h>
	#include <signal.h>
	#include <setjmp.h>
	#include <bitset>
	using std::bitset;
	//static sigjmp_buf sigsegvbuffer1;
	//static sigjmp_buf sigsegvbuffer2;
	//static sigjmp_buf sigsegvbuffer3;
#endif // __linux__

NS_TRLIB_BEGIN

///*!
// [Internal function. Do not call it.]
//
// Detect Soft-Ice exist, return true is exist.
//
// Return value - [bool] true is exist, false is not.
// Supported platform: Windows NT/2K
//*/
// Mark off as the code is moved to the header.
//bool __cdecl TR_Verify_SoftICE_SIWVIDSTART_A();

///*!
// [Internal function. Do not call it.]
//
// Detect Soft-Ice exist (Dynamic link version), return true is exist.
//
// Return value - [bool] true is exist, false is not.
// Supported platform: Windows NT/2K
//*/
bool __cdecl TR_Verify_SoftICE_SIWVIDSTART_DD_A();

///*!
// [Internal function. Do not call it.]
//
// Detect Soft-Ice exist, return true is exist.
//
// Return value - [bool] true is exist, false is not.
// Supported platform: Windows NT/2K
//*/
bool __cdecl TR_Verify_SoftICE_SICE_A();

///*!
// [Internal function. Do not call it.]
//
// Detect Soft-Ice exist (Dynamic link version), return true is exist.
//
// Return value - [bool] true is exist, false is not.
// Supported platform: Windows NT/2K
//*/
bool __cdecl TR_Verify_SoftICE_SICE_DD_A();

///*!
// [Internal function. Do not call it.]
//
// Detect Soft-Ice exist, return true is exist.
//
// Return value - [bool] true is exist, false is not.
// Supported platform: Windows NT/2K
//*/
// Mark off as the code is moved to the header.
//bool __cdecl TR_Verify_SoftICE_NTICE_A();

///*!
// [Internal function. Do not call it.]
//
// Detect Soft-Ice exist (Dynamic link version), return true is exist.
//
// Return value - [bool] true is exist, false is not.
// Supported platform: Windows NT/2K
//*/
bool __cdecl TR_Verify_SoftICE_NTICE_DD_A();

///*!
// [Internal function. Do not call it.]
//
// Detect Soft-Ice exist, return true is exist.
//
// Return value - [bool] true is exist, false is not.
// Supported platform: Windows 95/98
//*/
bool __cdecl TR_Verify_SoftICE_SIWVID_A();

///*!
// [Internal function. Do not call it.]
//
// Detect Soft-Ice exist (Dynamic link version), return true is exist.
//
// Return value - [bool] true is exist, false is not.
// Supported platform: Windows 95/98
//*/
bool __cdecl TR_Verify_SoftICE_SIWVID_DD_A();

///*!
// [Internal function. Do not call it.]
//
// Detect Soft-Ice exist, return true is exist.
//
// Return value - [bool] true is exist, false is not.
// Supported platform: Windows 95/98\n
//*/
bool __cdecl TR_Verify_SoftICE_SIWDEBUG_A();

///*!
// [Internal function. Do not call it.]
//
// Detect Soft-Ice exist (Dynamic link version), return true is exist.
//
// Return value - [bool] true is exist, false is not.
// Supported platform: Windows 95/98
//*/
bool __cdecl TR_Verify_SoftICE_SIWDEBUG_DD_A();

///*!
// [Internal function. Do not call it.]
//
// Detect Soft-Ice exist (Dynamic link version), return false is exist.
//
// Return value - [bool] false is exist, true is not.
// Supported platform: Windows NT/2K
//*/
// Mark off as the code is moved to the header.
//bool __cdecl TR_Verify_SoftICE_SIWVIDSTART_A_INV();

///*!
// [Internal function. Do not call it.]
//
// Detect Soft-Ice exist (Dynamic link version), return false is exist.
//
// Return value - [bool] false is exist, true is not.
// Supported platform: Windows NT/2K
//*/
bool __cdecl TR_Verify_SoftICE_SIWVIDSTART_DD_A_INV();

///*!
// [Internal function. Do not call it.]
//
// Detect Soft-Ice exist, return false is exist.
//
// Return value - [bool] false is exist, true is not.
// Supported platform: Windows NT/2K
//*/
bool __cdecl TR_Verify_SoftICE_SICE_A_INV();

///*!
// [Internal function. Do not call it.]
//
// Detect Soft-Ice exist (Dynamic link version), return false is exist.
//
// Return value - [bool] false is exist, true is not.
// Supported platform: Windows NT/2K
//*/
bool __cdecl TR_Verify_SoftICE_SICE_DD_A_INV();

///*!
// [Internal function. Do not call it.]
//
// Detect Soft-Ice exist, return false is exist.
//
// Return value - [bool] false is exist, true is not.
// Supported platform: Windows NT/2K
//*/
// Mark off as the code is moved to the header.
//bool __cdecl TR_Verify_SoftICE_NTICE_A_INV();

///*!
// [Internal function. Do not call it.]
//
// Detect Soft-Ice exist (Dynamic link version), return false is exist.
//
// Return value - [bool] false is exist, true is not.
// Supported platform: Windows NT/2K
//*/
bool __cdecl TR_Verify_SoftICE_NTICE_DD_A_INV();

///*!
// [Internal function. Do not call it.]
//
// Detect Soft-Ice exist, return false is exist.
//
// Return value - [bool] false is exist, true is not.
// Supported platform: Windows 95/98
//*/
bool __cdecl TR_Verify_SoftICE_SIWVID_A_INV();

///*!
// [Internal function. Do not call it.]
//
// Detect Soft-Ice exist (Dynamic link version), return false is exist.
//
// Return value - [bool] false is exist, true is not.
// Supported platform: Windows 95/98
//*/
bool __cdecl TR_Verify_SoftICE_SIWVID_DD_A_INV();

///*!
// [Internal function. Do not call it.]
//
// Detect Soft-Ice exist, return false is exist.
//
// Return value - [bool] false is exist, true is not.
// Supported platform: Windows 95/98
//*/
bool __cdecl TR_Verify_SoftICE_SIWDEBUG_A_INV();

///*!
// [Internal function. Do not call it.]
//
// Detect Soft-Ice exist (Dynamic link version), return false is exist.
//
// Return value - [bool] false is exist, true is not.
// Supported platform: Windows 95/98
//*/
bool __cdecl TR_Verify_SoftICE_SIWDEBUG_DD_A_INV();

///*!
// [Internal function. Do not call it.]
//
// Detect Soft-Ice exist (Wide-charactor version), return true is exist.
//
// Return value - [bool] true is exist, false is not.
// Supported platform: Windows NT/2K
//*/
bool __cdecl TR_Verify_SoftICE_SIWVIDSTART_W();

///*!
// [Internal function. Do not call it.]
//
// Detect Soft-Ice exist (Wide-charactor/Dynamic link version), return true is exist.
//
// Return value - [bool] true is exist, false is not.
// Supported platform: Windows NT/2K
//*/
bool __cdecl TR_Verify_SoftICE_SIWVIDSTART_DD_W();

///*!
// [Internal function. Do not call it.]
//
// Detect Soft-Ice exist (Wide-charactor version), return true is exist.
//
// Return value - [bool] true is exist, false is not.
// Supported platform: Windows 95/98
//*/
bool __cdecl TR_Verify_SoftICE_SICE_W();

///*!
// [Internal function. Do not call it.]
//
// Detect Soft-Ice exist (Wide-charactor/Dynamic link version), return true is exist.
//
// Return value - [bool] true is exist, false is not.
// Supported platform: Windows 95/98
//*/
bool __cdecl TR_Verify_SoftICE_SICE_DD_W();

///*!
// [Internal function. Do not call it.]
//
// Detect Soft-Ice exist (Wide-charactor version), return true is exist.
//
// Return value - [bool] true is exist, false is not.
// Supported platform: Windows NT/2K
//*/
bool __cdecl TR_Verify_SoftICE_NTICE_W();

///*!
// [Internal function. Do not call it.]
//
// Detect Soft-Ice exist (Wide-charactor/Dynamic link version), return true is exist.
//
// Return value - [bool] true is exist, false is not.
// Supported platform: Windows NT/2K
//*/
bool __cdecl TR_Verify_SoftICE_NTICE_DD_W();

///*!
// [Internal function. Do not call it.]
//
// Detect Soft-Ice exist (Wide-charactor version), return true is exist.
//
// Return value - [bool] true is exist, false is not.
// Supported platform: Windows 95/98
//*/
bool __cdecl TR_Verify_SoftICE_SIWVID_W();

///*!
// [Internal function. Do not call it.]
//
// Detect Soft-Ice exist (Wide-charactor/Dynamic link version), return true is exist.
//
// Return value - [bool] true is exist, false is not.
// Supported platform: Windows 95/98
//*/
bool __cdecl TR_Verify_SoftICE_SIWVID_DD_W();

///*!
// [Internal function. Do not call it.]
//
// Detect Soft-Ice exist (Wide-charactor version), return true is exist.
//
// Return value - [bool] true is exist, false is not.
// Supported platform: Windows 95/98
//*/
bool __cdecl TR_Verify_SoftICE_SIWDEBUG_W();

///*!
// [Internal function. Do not call it.]
//
// Detect Soft-Ice exist (Wide-charactor/Dynamic link version), return true is exist.
//
// Return value - [bool] true is exist, false is not.
// Supported platform: Windows 95/98
//*/
bool __cdecl TR_Verify_SoftICE_SIWDEBUG_DD_W();

///*!
// [Internal function. Do not call it.]
//
// Detect Soft-Ice exist (Wide-charactor version), return false is exist.
//
// Return value - [bool] false is exist, true is not.
// Supported platform: Windows NT/2K
//*/
bool __cdecl TR_Verify_SoftICE_SIWVIDSTART_W_INV();

///*!
// [Internal function. Do not call it.]
//
// Detect Soft-Ice exist (Wide-charactor/Dynamic link version), return false is exist.
//
// Return value - [bool] false is exist, true is not.
// Supported platform: Windows NT/2K
//*/
bool __cdecl TR_Verify_SoftICE_SIWVIDSTART_DD_W_INV();

///*!
// [Internal function. Do not call it.]
//
// Detect Soft-Ice exist (Wide-charactor version), return false is exist.
//
// Return value - [bool] false is exist, true is not.
// Supported platform: Windows NT/2K
//*/
bool __cdecl TR_Verify_SoftICE_SICE_W_INV();

///*!
// [Internal function. Do not call it.]
//
// Detect Soft-Ice exist (Wide-charactor/Dynamic link version), return false is exist.
//
// Return value - [bool] false is exist, true is not.
// Supported platform: Windows NT/2K
//*/
bool __cdecl TR_Verify_SoftICE_SICE_DD_W_INV();

///*!
// [Internal function. Do not call it.]
//
// Detect Soft-Ice exist (Wide-charactor version), return false is exist.
//
// Return value - [bool] false is exist, true is not.
// Supported platform: Windows NT/2K
//*/
bool __cdecl TR_Verify_SoftICE_NTICE_W_INV();

///*!
// [Internal function. Do not call it.]
//
// Detect Soft-Ice exist (Wide-charactor/Dynamic link version), return false is exist.
//
// Return value - [bool] false is exist, true is not.
// Supported platform: Windows NT/2K
//*/
bool __cdecl TR_Verify_SoftICE_NTICE_DD_W_INV();

///*!
// [Internal function. Do not call it.]
//
// Detect Soft-Ice exist (Wide-charactor version), return false is exist.
//
// Return value - [bool] false is exist, true is not.
// Supported platform: Windows 95/98
//*/
bool __cdecl TR_Verify_SoftICE_SIWVID_W_INV();

///*!
// [Internal function. Do not call it.]
//
// Detect Soft-Ice exist (Wide-charactor/Dynamic link version), return false is exist.
//
// Return value - [bool] false is exist, true is not.
// Supported platform: Windows 95/98
//*/
bool __cdecl TR_Verify_SoftICE_SIWVID_DD_W_INV();

///*!
// [Internal function. Do not call it.]
//
// Detect Soft-Ice exist (Wide-charactor version), return false is exist.
//
// Return value - [bool] false is exist, true is not.
// Supported platform: Windows 95/98
//*/
bool __cdecl TR_Verify_SoftICE_SIWDEBUG_W_INV();

///*!
// [Internal function. Do not call it.]
//
// Detect Soft-Ice exist (Wide-charactor/Dynamic link version), return false is exist.
//
// Return value - [bool] false is exist, true is not.
// Supported platform: Windows 95/98
//*/
bool __cdecl TR_Verify_SoftICE_SIWDEBUG_DD_W_INV();

///*!
// [Internal function. Do not call it.]
//
// Detect Soft-Ice exist, return true is exist.
//
// Return value - [bool] true is exist, false is not.
// Supported platform: Windows NT/2K
//*/
// Mark off as the code is moved to the header.
//bool __cdecl TR_Verify_SoftICE_LCREAT();

///*!
// [Internal function. Do not call it.]
//
// Detect Soft-Ice exist, return true is exist.
//
// Return value - [bool] true is exist, false is not.
// Supported platform: Windows 95/98
//*/
bool __cdecl TR_Verify_SoftICE_LCREAT1();

///*!
// [Internal function. Do not call it.]
//
// Detect Soft-Ice exist, return true is exist.
//
// Return value - [bool] true is exist, false is not.
// Supported platform: Windows NT/2K
//*/
// Mark off as the code is moved to the header.
//bool __cdecl TR_Verify_SoftICE_LCREAT2();

///*!
// [Internal function. Do not call it.]
//
// Detect Soft-Ice exist, return true is exist.
//
// Return value - [bool] true is exist, false is not.
// Supported platform: Windows 95/98
//*/
bool __cdecl TR_Verify_SoftICE_LCREAT3();

///*!
// [Internal function. Do not call it.]
//
// Detect Soft-Ice exist, return true is exist.
//
// Return value - [bool] true is exist, false is not.
// Supported platform: Windows 95/98
//*/
bool __cdecl TR_Verify_SoftICE_LCREAT4();

///*!
// [Internal function. Do not call it.]
//
// Detect Soft-Ice exist, return false is exist.
//
// Return value - [bool] false is exist, true is not.
// Supported platform: Windows NT/2K
//*/
// Mark off as the code is moved to the header.
//bool __cdecl TR_Verify_SoftICE_LCREAT_INV();

///*!
// [Internal function. Do not call it.]
//
// Detect Soft-Ice exist, return false is exist.
//
// Return value - [bool] false is exist, true is not.
// Supported platform: Windows 95/98
//*/
bool __cdecl TR_Verify_SoftICE_LCREAT1_INV();

///*!
// [Internal function. Do not call it.]
//
// Detect Soft-Ice exist, return false is exist.
//
// Return value - [bool] false is exist, true is not.
// Supported platform: Windows NT/2K
//*/
// Mark off as the code is moved to the header.
//bool __cdecl TR_Verify_SoftICE_LCREAT2_INV();

///*!
// [Internal function. Do not call it.]
//
// Detect Soft-Ice exist, return false is exist.
//
// Return value - [bool] false is exist, true is not.
// Supported platform: Windows 95/98
//*/
bool __cdecl TR_Verify_SoftICE_LCREAT3_INV();

///*!
// [Internal function. Do not call it.]
//
// Detect Soft-Ice exist, return false is exist.
//
// Return value - [bool] false is exist, true is not.
// Supported platform: Windows 95/98
//*/
bool __cdecl TR_Verify_SoftICE_LCREAT4_INV();

///*!
// [Internal function. Do not call it.]
//
// Detect Soft-Ice exist, return true is exist.
//
// Return value - [bool] true is exist, false is not.
// Supported platform: Windows NT/2K
//*/
// Mark off as the code is moved to the header.
//bool __cdecl TR_Verify_SoftICE_LOPEN();

///*!
// [Internal function. Do not call it.]
//
// Detect Soft-Ice exist, return true is exist.
//
// Return value - [bool] true is exist, false is not.
// Supported platform: Windows 95/98
//*/
bool __cdecl TR_Verify_SoftICE_LOPEN1();

///*!
// [Internal function. Do not call it.]
//
// Detect Soft-Ice exist, return true is exist.
//
// Return value - [bool] true is exist, false is not.
// Supported platform: Windows NT/2K
//*/
// Mark off as the code is moved to the header.
//bool __cdecl TR_Verify_SoftICE_LOPEN2();

///*!
// [Internal function. Do not call it.]
//
// Detect Soft-Ice exist, return true is exist.
//
// Return value - [bool] true is exist, false is not.
// Supported platform: Windows 95/98
//*/
bool __cdecl TR_Verify_SoftICE_LOPEN3();

///*!
// [Internal function. Do not call it.]
//
// Detect Soft-Ice exist, return true is exist.
//
// Return value - [bool] true is exist, false is not.
// Supported platform: Windows 95/98
//*/
bool __cdecl TR_Verify_SoftICE_LOPEN4();

///*!
// [Internal function. Do not call it.]
//
// Detect Soft-Ice exist, return false is exist.
//
// Return value - [bool] false is exist, true is not.
// Supported platform: Windows NT/2K
//*/
// Mark off as the code is moved to the header.
//bool __cdecl TR_Verify_SoftICE_LOPEN_INV();

///*!
// [Internal function. Do not call it.]
//
// Detect Soft-Ice exist, return false is exist.
//
// Return value - [bool] false is exist, true is not.
// Supported platform: Windows 95/98
//*/
bool __cdecl TR_Verify_SoftICE_LOPEN1_INV();

///*!
// [Internal function. Do not call it.]
//
// Detect Soft-Ice exist, return false is exist.
//
// Return value - [bool] false is exist, true is not.
// Supported platform: Windows 95/98
//*/
// Mark off as the code is moved to the header.
//bool __cdecl TR_Verify_SoftICE_LOPEN2_INV();

///*!
// [Internal function. Do not call it.]
//
// Detect Soft-Ice exist, return false is exist.
//
// Return value - [bool] false is exist, true is not.
// Supported platform: Windows NT/2K
//*/
bool __cdecl TR_Verify_SoftICE_LOPEN3_INV();

///*!
// [Internal function. Do not call it.]
//
// Detect Soft-Ice exist, return false is exist.
//
// Return value - [bool] false is exist, true is not.
// Supported platform: Windows 95/98
//*/
bool __cdecl TR_Verify_SoftICE_LOPEN4_INV();

///*!
// [Internal function. Do not call it.]
//
// Detect Soft-Ice presence, return true when exists.
//
// Return value - [bool] true is present, false is not.
// Supported platform: Windows NT/2K
//*/
// Mark off as the code is moved to the header.
//bool __cdecl TR_Verify_SoftICE_NTICE_SERVICE();

///*!
// [Internal function. Do not call it.]
//
// Detect Soft-Ice presence, return false when exists.
//
// Return value - [bool] false is present, true is not.
// Supported platform: Windows NT/2K
//*/
// Mark off as the code is moved to the header.
//bool __cdecl TR_Verify_SoftICE_NTICE_SERVICE_INV();

///*!
// [Internal function. Do not call it.]
//
// Detect Syser Debugger exist, return true is exist.
//
// Return value - [bool] true is exist, false is not.
// Supported platform: Windows NT/2K
//*/
// Mark off as the code is moved to the header.
//bool __cdecl TR_Verify_Syser_A();

///*!
// [Internal function. Do not call it.]
//
// Detect Syser Debugger exist (Dynamic link version), return true is exist.
//
// Return value - [bool] true is exist, false is not.
// Supported platform: Windows NT/2K
//*/
bool __cdecl TR_Verify_Syser_DD_A();

///*!
// [Internal function. Do not call it.]
//
// Detect Syser Debugger exist (Wide-charactor version), return true is exist.
//
// Return value - [bool] true is exist, false is not.
// Supported platform: Windows NT/2K
//*/
bool __cdecl TR_Verify_Syser_W();

///*!
// [Internal function. Do not call it.]
//
// Detect Syser Debugger exist (Wide-charactor/Dynamic link version), return true is exist.
//
// Return value - [bool] true is exist, false is not.
// Supported platform: Windows NT/2K
//*/
bool __cdecl TR_Verify_Syser_DD_W();

///*!
// [Internal function. Do not call it.]
//
// Detect Syser Debugger exist, return true is exist.
//
// Return value - [bool] true is exist, false is not.
// Supported platform: Windows NT/2K
//*/
bool __cdecl TR_Verify_Syser_LCREATE();

///*!
// [Internal function. Do not call it.]
//
// Detect Syser Debugger exist, return true is exist.
//
// Return value - [bool] true is exist, false is not.
// Supported platform: Windows NT/2K
//*/
bool __cdecl TR_Verify_Syser_LOPEN();

///*!
// [Internal function. Do not call it.]
//
// Detect Int3 un-handled excepter filter, return true is exist.
//
// Return value - [DWORD] 4 is exist, or is not.
// Supported platform: Windows NT/2K
//*/
#ifdef _WIN32
	__forceinline DWORD __cdecl TR_Find_Int3_UnhandledExecptionFilter(void);
#elif defined(__linux__)
	///*!
	// [Internal function. Do not call it.]
	//
	// Detect Int3 un-handled excepter filter, return true is exist.
	//
	// Return value - [bool] true is exist, false is not.
	// Supported platform: Linux
	//*/
	bool TR_Int3Check(void);
#endif // _WIN32


///*!
// [Internal function. Do not call it.]
//
// Detect tracer trap flag exist, return not zero is exist.
//
// Return value - [DWORD] 0(zero) is debugger exist, or is not.
// Supported platform: All Windows
//*/
#ifdef _WIN32
	DWORD __cdecl TR_Detect_Tracer_Trap_Flag(void);
#elif defined(__linux__)
	///*!
	// [Internal function. Do not call it.]
	//
	// Detect tracer trap flag exist, return false is exist.
	//
	// Return value - [bool] false is debugger exist, true is not.
	// Supported platform: Linux
	//*/
	bool TR_Detect_Tracer_Trap_Flag(void);
#endif // _WIN32

#ifdef _WIN32
	///*!
	// [Internal function. Do not call it.]
	//
	// Detect ICE break point, return true is exist.
	//
	// Return value - [bool] true is debugger exist, or not.
	// Supported platform: Windows NT/2K
	//*/
	bool __stdcall TR_DetectICEBP(void);
#elif defined(__linux__)
	///*!
	// [Internal function. Do not call it.]
	//
	// Detect ICE break point, return true is exist.
	//
	// Return value - [bool] true is debugger exist, or not.
	// Supported platform:linux
	//*/
	bool __stdcall TR_DetectICEBP(void) {return false;}
#endif

///*!
// [Internal function. Do not call it.]
//
// Detect int 1 flag, return true is exist.
//
// Return value - [bool] true is debugger exist, or not.
// Supported platform: Windows NT/2K and linux
//*/
bool __stdcall TR_DetectInt1h(void);

///*!
// [Internal function. Do not call it.]
//
// Standard system check API, return true is exist.
//
// Return value - [bool] true is debugger exist, or not.
// Supported platform: All Windows
//*/
#ifdef _WIN32
	// Mark off as the code is moved to the header.
	//bool __cdecl TR_API_IsDebuggerPresent(void);
#elif defined(__linux__)
	///*!
	// [Internal function. Do not call it.]
	//
	// Standard system check API, return true is exist.
	//
	// Return value - [bool] true is debugger exist, or not.
	// Supported platform: Linux
	//*/
	bool TR_IsDebuggerPresent(void);
#endif // _WIN32

///*!
// [Internal function. Do not call it.]
//
// Detect OllyDbg exist, return true is exist.
//
// Return value - [bool] true is debugger exist, or not.
// Supported platform: All Windows
//*/
__forceinline bool __cdecl TR_IsODBGLoaded(void);

///*!
// [Internal function. Do not call it.]
//
// Lock compare register, if Soft-ICE exist, hang.
//
// Return value - None
// Supported platform: None
//
// Note:\n
// Some CPU will cause unstable,
// Not available for current Soft-ICE. Don't use it now.
//*/
void __cdecl TR_Lock_Cmpxchg8b(void);

///*!
// [Internal function. Do not call it.]
//
// Detect Int 3 eax flag, return 4 if OK, or Soft-ICE present.
//
// Return value - [DWORD] return 4 for normal case, or Soft-ICE present.
// Supported platform: None
// 
// Note:\n
// Not available for current Soft-ICE. Don't use it now.
//*/
DWORD __cdecl TR_INT3_EAX_Check(void);

///*!
// [Internal function. Do not call it.]
//
// Detect debugger break exception, return true is exist.
//
// Return value - [bool] true is debugger exist, or not.
// Supported platform: All Windows
//*/
bool __cdecl TR_Debugbreak_Check(void);

///*!
// [Internal function. Do not call it.]
//
// Detect SoftICE register key, return true is exist.
//
// Return value - [bool] true is debugger exist, or not.
// Supported platform: All Windows
//*/
// Mark off as the code is moved to the header.
//bool __cdecl TR_RegKeyCheck_SoftICE(void);

///*!
// [Internal function. Do not call it.]
//
// Detect Syser register key, return true is exist.
//
// Return value - [bool] true is debugger exist, or not.
// Supported platform: All Windows
//*/
// Mark off as the code is moved to the header.
//bool __cdecl TR_RegKeyCheck_Syser_1(void);

///*!
// [Internal function. Do not call it.]
//
// Detect Syser register key, return true is exist.
//
// Return value - [bool] true is debugger exist, or not.
// Supported platform: All Windows
//*/
// Mark off as the code is moved to the header.
//bool __cdecl TR_RegKeyCheck_Syser_2(void);

///*!
// [Internal function. Do not call it.]
//
// Detect Syser register key, return true is exist.
//
// Return value - [bool] true is debugger exist, or not.
// Supported platform: All Windows
//*/
// Mark off as the code is moved to the header.
//bool __cdecl TR_RegKeyCheck_Syser_3(void);

///*!
// [Internal function. Do not call it.]
//
// Detect Syser register key, return true is exist.
//
// Return value - [bool] true is debugger exist, or not.
// Supported platform: All Windows
//*/
// Mark off as the code is moved to the header.
//bool __cdecl TR_RegKeyCheck_Syser_4(void);

///*!
// [Internal function. Do not call it.]
//
// Detect Syser register key, return true is exist.
//
// Return value - [bool] true is debugger exist, or not.
// Supported platform: All Windows
//*/
// Mark off as the code is moved to the header.
//bool __cdecl TR_RegKeyCheck_Syser_5(void);

///*!
// [Internal function. Do not call it.]
//
// Detect debugger register key, return true is exist.
//
// Return value - [bool] true is debugger exist, or not.
// Supported platform: All Windows
// Note:\n
// Because internal create thread, has performance-related issue.
//*/
bool __cdecl TR_Debug_Register_Check(void);

///*!
// [Internal function. Do not call it.]
//
// Clear all Debug registers in the current thread, return true is successful.
//
// Return value - [bool] true is debugger exist, or not.
// Supported platform: All Windows
// Note:\n
// Because internal create thread, has performance-related issue.
//*/
bool __cdecl TR_Debug_Register_Clear(void);

///*!
// [Internal function. Do not call it.]
//
// Detect Int 3 eax flag, return true is debugger exist.
//
// Return value - [bool] true is debugger exist, or not.
// Supported platform: Linux
//*/
bool __cdecl TR_Int3Check2(void);

///*!
// [Internal function. Do not call it.]
//
// Detect debugger break exception, return true is exist.
//
// Return value - [bool] true is debugger exist, or not.
// Supported platform: Linux
//*/
bool __cdecl TR_Int3Check3(void);

///*!
// [Internal use]
//
// the structure for Detect/Clear Debug register.
// m_hCurThread : HANDLE, the handle of thread that should detect/clear the Debug register within it.
// m_bReturn : bool, the return value of Detection.
//*/
struct trDETECTDR
{
	HANDLE m_hCurThread;
	bool m_bReturn;
};

///*!
// [Internal function. Do not call it.]
//
// Thread procedure. To check if Debug register is set.
//
// Return value - [bool] true if it is successful.
// Supported platform: Windows
//*/
DWORD WINAPI gtrDetectDRFunc(LPVOID pArg);

///*!
// [Internal function. Do not call it.]
//
// Thread procedure. To clear all of  Debug registers.
//
// Return value - [bool] true if it is successful.
// Supported platform: Windows
//*/
DWORD WINAPI gtrClearDRFunc(LPVOID pArg);

///*!
// [Internal function.  Do not call it.]
//
// Check if the system is running in debug mode,
// return true if it is.
//
// Return value - [bool] true if it is, false if not.
// Supported platform: Windows
//*/
bool __cdecl gtrSystemDebugRegistryCheck(void);

///*!
// [Internal function.  Do not call it.]
//
// Detect if debugger is debugging current program, true is debugger exist.
//
// return true if the debugger is existed.
//
// Supported platform:		Windows NT/2K
// Performance overhead:	Low
// Security level:			High
// Usage scope:				Macro scope
// 
// Note:\n
//*/
bool __cdecl gtrINT2DCheck(void);

///*!
// [Internal function.  Do not call it.]
//
// Detect if the program is being debugged by user-mode debugger. true if exist.
//
// return true if the debugger is existed.
//
// Supported platform:		Windows NT/2K
// Performance overhead:	Low
// Security level:			High
// Usage scope:				Macro scope
// 
// Note:\n
//*/
bool __cdecl gtrDetectPrintException(void);

NS_TRLIB_END

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifdef _WIN32

	///*!
	// Detect if Soft-Ice kernel debugger is installed in the system. true if exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		Windows NT/2K
	// Performance overhead:	Low
	// Security level:			Low
	// Usage scope:				Macro scope
	//*/
	//#define iviTR_VERIFY_SOFTICE_SIWVIDSTART_A(bRetValue)\
	//{   bRetValue = TR_Verify_SoftICE_SIWVIDSTART_A(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bRetValue, iviTR_VERIFY_SOFTICE_SIWVIDSTART_A)}
	#define iviTR_VERIFY_SOFTICE_SIWVIDSTART_A(bRetValue) \
	{ \
		char name1[16]; HANDLE hDevice; \
		iviTR_DUMMY_CALL_3(78) \
		bRetValue = false; \
		name1[0]='\\'; name1[1]=name1[0]; name1[3]=name1[0]; name1[2]='.'; \
		name1[8]='I'; name1[9]='D'; name1[4]='S'; name1[5]='I'; name1[6]='W'; name1[7]='V'; \
		iviTR_DUMMY_CALL_8(15) \
		name1[10]='S'; name1[11]='T'; name1[12]='A'; name1[13]='R'; name1[14]='T'; name1[15]=0; \
		iviTR_DUMMY_CALL_1(18); \
		if ((hDevice = ::CreateFileA(name1,FILE_FLAG_WRITE_THROUGH,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0)) != INVALID_HANDLE_VALUE) \
		{ \
			::CloseHandle(hDevice); \
			bRetValue = true; \
		} \
		else \
		{ \
			iviTR_DUMMY_CALL_2(17) \
			bRetValue = false; \
		} \
		iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bRetValue, iviTR_VERIFY_SOFTICE_SIWVIDSTART_A) \
	}

	///*!
	// Detect if Soft-Ice kernel debugger is installed in the system. true if exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		Windows NT/2K
	// Performance overhead:	Medium
	// Security level:			Low
	// Usage scope:				Macro scope
	// 
	// Note:\n
	// dynamic link version, may cause performance low, use carefully.
	//*/
	#define iviTR_VERIFY_SOFTICE_SIWVIDSTART_DD_A(bRetValue)\
	{   bRetValue = TR_Verify_SoftICE_SIWVIDSTART_DD_A(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bRetValue, iviTR_VERIFY_SOFTICE_SIWVIDSTART_DD_A)}

	///*!
	// Detect if Soft-Ice kernel debugger is installed and activated in the system. true if exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		Windows NT/2K
	// Performance overhead:	Low
	// Security level:			Low
	// Usage scope:				Macro scope
	//*/
	#define iviTR_VERIFY_SOFTICE_SICE_A(bRetValue)\
	{   bRetValue = TR_Verify_SoftICE_SICE_A(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bRetValue, iviTR_VERIFY_SOFTICE_SICE_A)}

	///*!
	// Detect if Soft-Ice kernel debugger is installed and activated in the system. true if exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		Windows NT/2K
	// Performance overhead:	Medium
	// Security level:			Low
	// Usage scope:				Macro scope
	//
	// Note:\n
	// dynamic link version, may cause performance low, use carefully.
	//*/
	#define iviTR_VERIFY_SOFTICE_SICE_DD_A(bRetValue)\
	{   bRetValue = TR_Verify_SoftICE_SICE_DD_A(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bRetValue, iviTR_VERIFY_SOFTICE_SICE_DD_A)}

	///*!
	// Detect if Soft-Ice kernel debugger is installed and activated in the system. true if exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		Windows NT/2K
	// Performance overhead:	Low
	// Security level:			Low
	// Usage scope:				Macro scope
	//*/
	//#define iviTR_VERIFY_SOFTICE_NTICE_A(bRetValue)\
	//{   bRetValue = TR_Verify_SoftICE_NTICE_A(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bRetValue, iviTR_VERIFY_SOFTICE_NTICE_A)}
	#define iviTR_VERIFY_SOFTICE_NTICE_A(bRetValue) \
	{ \
		bRetValue = false; \
		char name1[10]; HANDLE hDevice; \
		name1[0]='\\'; name1[1]=name1[0]; name1[3]=name1[0]; name1[2]='.'; \
		name1[8]='E'; name1[9]=0; name1[4]='N'; name1[5]='T'; name1[6]='I'; name1[7]='C'; \
		if ((hDevice = ::CreateFileA(name1,FILE_FLAG_WRITE_THROUGH,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0)) != INVALID_HANDLE_VALUE) \
		{ \
			iviTR_DUMMY_CALL_4(3) \
			::CloseHandle(hDevice); \
			bRetValue = true; \
		} \
		else \
		{ \
			iviTR_DUMMY_CALL_1(85) \
			bRetValue = false; \
		} \
		iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bRetValue, iviTR_VERIFY_SOFTICE_NTICE_A) \
	}

	///*!
	// Detect if Soft-Ice kernel debugger is installed and activated in the system. true if exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		Windows NT/2K
	// Performance overhead:	Medium
	// Security level:			Low
	// Usage scope:				Macro scope
	//
	// Note:\n
	// dynamic link version, may cause performance low, use carefully.
	//*/
	#define iviTR_VERIFY_SOFTICE_NTICE_DD_A(bRetValue)\
	{   bRetValue = TR_Verify_SoftICE_NTICE_DD_A(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bRetValue, iviTR_VERIFY_SOFTICE_NTICE_DD_A)}

	///*!
	// Detect if Soft-Ice kernel debugger is installed and activated in the system. true if exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		Windows 95/98
	// Performance overhead:	Low
	// Security level:			Low
	// Usage scope:				Macro scope
	//*/
	#define iviTR_VERIFY_SOFTICE_SIWVID_A(bRetValue)\
	{   bRetValue = TR_Verify_SoftICE_SIWVID_A(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bRetValue, iviTR_VERIFY_SOFTICE_SIWVID_A)}

	///*!
	// Detect if Soft-Ice kernel debugger is installed and activated in the system. true if exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		Windows 95/98
	// Performance overhead:	Medium
	// Security level:			Low
	// Usage scope:				Macro scope
	//
	// Note:\n
	// dynamic link version, may cause performance low, use carefully.
	//*/
	#define iviTR_VERIFY_SOFTICE_SIWVID_DD_A(bRetValue)\
	{   bRetValue = TR_Verify_SoftICE_SIWVID_DD_A(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bRetValue, iviTR_VERIFY_SOFTICE_SIWVID_DD_A)}

	///*!
	// Detect if Soft-Ice kernel debugger is installed and activated in the system. true if exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		Windows 95/98
	// Performance overhead:	Low
	// Security level:			Low
	// Usage scope:				Macro scope
	//*/
	#define iviTR_VERIFY_SOFTICE_SIWDEBUG_A(bRetValue)\
	{	bRetValue = TR_Verify_SoftICE_SIWDEBUG_A(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bRetValue, iviTR_VERIFY_SOFTICE_SIWDEBUG_A)}

	///*!
	// Detect if Soft-Ice kernel debugger is installed and activated in the system. true if exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		Windows 95/98
	// Performance overhead:	Medium
	// Security level:			Low
	// Usage scope:				Macro scope
	//
	// Note:\n
	// dynamic link version, may cause performance low, use carefully.
	//*/
	#define iviTR_VERIFY_SOFTICE_SIWDEBUG_DD_A(bRetValue)\
	{	bRetValue = TR_Verify_SoftICE_SIWDEBUG_DD_A(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bRetValue, iviTR_VERIFY_SOFTICE_SIWDEBUG_DD_A)}

	///*!
	// Detect if Soft-Ice kernel debugger is installed in the system. false if exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		Windows NT/2K
	// Performance overhead:	Low
	// Security level:			Low
	// Usage scope:				Macro scope
	//*/
	//#define iviTR_VERIFY_SOFTICE_SIWVIDSTART_A_INV(bRetValue)\
	//{	bRetValue = TR_Verify_SoftICE_SIWVIDSTART_A_INV(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(!bRetValue, iviTR_VERIFY_SOFTICE_SIWVIDSTART_A_INV)}
	#define iviTR_VERIFY_SOFTICE_SIWVIDSTART_A_INV(bRetValue) \
	{ \
		bRetValue = true; \
		iviTR_DUMMY_CALL_5(10) \
		char name1[16]; HANDLE hDevice; \
		name1[0]='\\'; name1[1]=name1[0]; name1[3]=name1[0]; name1[2]='.'; \
		iviTR_DUMMY_CALL_9(21) \
		name1[8]='I'; name1[9]='D'; name1[4]='S'; name1[5]='I'; name1[6]='W'; name1[7]='V'; \
		name1[10]='S'; name1[11]='T'; name1[12]='A'; name1[13]='R'; name1[14]='T'; name1[15]=0; \
		if ((hDevice = ::CreateFileA(name1,FILE_FLAG_WRITE_THROUGH,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0)) != INVALID_HANDLE_VALUE) \
		{ \
			iviTR_DUMMY_CALL_3(50) \
			::CloseHandle(hDevice); \
			bRetValue = false; \
		} \
		else \
		{ \
			iviTR_DUMMY_CALL_1(11) \
			bRetValue = true; \
		} \
		iviTR_DUMP_ERROR_TR_MACRO_ENTRY(!bRetValue, iviTR_VERIFY_SOFTICE_SIWVIDSTART_A_INV) \
	}

	///*!
	// Detect if Soft-Ice kernel debugger is installed in the system. false if exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		Windows NT/2K
	// Performance overhead:	Medium
	// Security level:			Low
	// Usage scope:				Macro scope
	// 
	// Note:\n
	// dynamic link version, may cause performance low, use carefully.
	//*/
	#define iviTR_VERIFY_SOFTICE_SIWVIDSTART_DD_A_INV(bRetValue)\
	{   bRetValue = TR_Verify_SoftICE_SIWVIDSTART_DD_A_INV(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(!bRetValue, iviTR_VERIFY_SOFTICE_SIWVIDSTART_DD_A_INV)}

	///*!
	// Detect if Soft-Ice kernel debugger is installed and activated in the system. false if exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		Windows NT/2K
	// Performance overhead:	Low
	// Security level:			Low
	// Usage scope:				Macro scope
	//*/
	#define iviTR_VERIFY_SOFTICE_SICE_A_INV(bRetValue)\
	{   bRetValue = TR_Verify_SoftICE_SICE_A_INV(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(!bRetValue, iviTR_VERIFY_SOFTICE_SICE_A_INV)}

	///*!
	// Detect if Soft-Ice kernel debugger is installed and activated in the system. false if exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		Windows NT/2K
	// Performance overhead:	Medium
	// Security level:			Low
	// Usage scope:				Macro scope
	//
	// Note:\n
	// dynamic link version, may cause performance low, use carefully.
	//*/
	#define iviTR_VERIFY_SOFTICE_SICE_DD_A_INV(bRetValue)\
	{   bRetValue = TR_Verify_SoftICE_SICE_DD_A_INV(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(!bRetValue, iviTR_VERIFY_SOFTICE_SICE_DD_A_INV)}

	///*!
	// Detect if Soft-Ice kernel debugger is installed and activated in the system. false if exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		Windows NT/2K
	// Performance overhead:	Low
	// Security level:			Low
	// Usage scope:				Macro scope
	//*/
	//#define iviTR_VERIFY_SOFTICE_NTICE_A_INV(bRetValue)\
	//{   bRetValue = TR_Verify_SoftICE_NTICE_A_INV(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(!bRetValue, iviTR_VERIFY_SOFTICE_NTICE_A_INV)}
	// Mark off as the code is moved to the header.
	#define iviTR_VERIFY_SOFTICE_NTICE_A_INV(bRetValue) \
	{ \
		bRetValue = true; \
		iviTR_DUMMY_CALL_2(19) \
		char name1[10]; HANDLE hDevice; \
		iviTR_DUMMY_CALL_6(51) \
		name1[0]='\\'; name1[1]=name1[0]; name1[3]=name1[0]; name1[2]='.'; \
		name1[8]='E'; name1[9]=0; name1[4]='N'; name1[5]='T'; name1[6]='I'; name1[7]='C'; \
		if ((hDevice = ::CreateFileA(name1,FILE_FLAG_WRITE_THROUGH,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0)) != INVALID_HANDLE_VALUE) \
		{ \
		iviTR_DUMMY_CALL_1(35) \
			::CloseHandle(hDevice); \
			bRetValue = false; \
		} \
		else \
		{ \
			iviTR_DUMMY_CALL_8(81) \
			bRetValue = true; \
		} \
		iviTR_DUMP_ERROR_TR_MACRO_ENTRY(!bRetValue, iviTR_VERIFY_SOFTICE_NTICE_A_INV) \
	}

	///*!
	// Detect if Soft-Ice kernel debugger is installed and activated in the system. false if exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		Windows NT/2K
	// Performance overhead:	Medium
	// Security level:			Low
	// Usage scope:				Macro scope
	//
	// Note:\n
	// dynamic link version, may cause performance low, use carefully.
	//*/
	#define iviTR_VERIFY_SOFTICE_NTICE_DD_A_INV(bRetValue)\
	{   bRetValue = TR_Verify_SoftICE_NTICE_DD_A_INV(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(!bRetValue, iviTR_VERIFY_SOFTICE_NTICE_DD_A_INV)}

	///*!
	// Detect if Soft-Ice kernel debugger is installed and activated in the system. false if exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		Windows 95/98
	// Performance overhead:	Low
	// Security level:			Low
	// Usage scope:				Macro scope
	//*/
	#define iviTR_VERIFY_SOFTICE_SIWVID_A_INV(bRetValue)\
	{   bRetValue = TR_Verify_SoftICE_SIWVID_A_INV(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(!bRetValue, iviTR_VERIFY_SOFTICE_SIWVID_A_INV)}

	///*!
	// Detect if Soft-Ice kernel debugger is installed and activated in the system. false if exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		Windows 95/98
	// Performance overhead:	Low
	// Security level:			Low
	// Usage scope:				Macro scope
	//
	// Note:\n
	// dynamic link version, may cause performance low, use carefully.
	//*/
	#define iviTR_VERIFY_SOFTICE_SIWVID_DD_A_INV(bRetValue)\
	{   bRetValue = TR_Verify_SoftICE_SIWVID_DD_A_INV(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(!bRetValue, iviTR_VERIFY_SOFTICE_SIWVID_DD_A_INV)}

	///*!
	// Detect if Soft-Ice kernel debugger is installed and activated in the system. false if exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		Windows 95/98
	// Performance overhead:	Low
	// Security level:			Low
	// Usage scope:				Macro scope
	//*/
	#define iviTR_VERIFY_SOFTICE_SIWDEBUG_A_INV(bRetValue)\
	{   bRetValue = TR_Verify_SoftICE_SIWDEBUG_A_INV(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(!bRetValue, iviTR_VERIFY_SOFTICE_SIWDEBUG_A_INV)}

	///*!
	// Detect if Soft-Ice kernel debugger is installed and activated in the system. false if exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		Windows 95/98
	// Performance overhead:	Medium
	// Security level:			Low
	// Usage scope:				Macro scope
	//
	// Note:\n
	// dynamic link version, may cause performance low, use carefully.
	//*/
	#define iviTR_VERIFY_SOFTICE_SIWDEBUG_DD_A_INV(bRetValue)\
	{   bRetValue = TR_Verify_SoftICE_SIWDEBUG_DD_A_INV(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(!bRetValue, iviTR_VERIFY_SOFTICE_SIWDEBUG_DD_A_INV)}

	///*!
	// Detect if Soft-Ice kernel debugger is installed in the system. true if exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		Windows NT/2K
	// Performance overhead:	Low
	// Security level:			Low
	// Usage scope:				Macro scope
	//*/
	#define iviTR_VERIFY_SOFTICE_SIWVIDSTART_W(bRetValue)\
	{   bRetValue = TR_Verify_SoftICE_SIWVIDSTART_W(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bRetValue, iviTR_VERIFY_SOFTICE_SIWVIDSTART_W)}

	///*!
	// Detect if Soft-Ice kernel debugger is installed in the system. false if exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		Windows NT/2K
	// Performance overhead:	Medium
	// Security level:			Low
	// Usage scope:				Macro scope
	// 
	// Note:\n
	// dynamic link version, may cause performance low, use carefully.
	//*/
	#define iviTR_VERIFY_SOFTICE_SIWVIDSTART_DD_W(bRetValue)\
	{   bRetValue = TR_Verify_SoftICE_SIWVIDSTART_DD_W(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(!bRetValue, iviTR_VERIFY_SOFTICE_SIWVIDSTART_DD_W)}

	///*!
	// Detect if Soft-Ice kernel debugger is installed and activated in the system. true if exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		Windows 95/98
	// Performance overhead:	Low
	// Security level:			Low
	// Usage scope:				Macro scope
	//*/
	#define iviTR_VERIFY_SOFTICE_SICE_W(bRetValue)\
	{   bRetValue = TR_Verify_SoftICE_SICE_W(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bRetValue, iviTR_VERIFY_SOFTICE_SICE_W)}

	///*!
	// Detect if Soft-Ice kernel debugger is installed and activated in the system. true if exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		Windows 95/98
	// Performance overhead:	Medium
	// Security level:			Low
	// Usage scope:				Macro scope
	//
	// Note:\n
	// dynamic link version, may cause performance low, use carefully.
	//*/
	#define iviTR_VERIFY_SOFTICE_SICE_DD_W(bRetValue)\
	{   bRetValue = TR_Verify_SoftICE_SICE_DD_W(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bRetValue, iviTR_VERIFY_SOFTICE_SICE_DD_W)}

	///*!
	// Detect if Soft-Ice kernel debugger is installed and activated in the system. true if exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		Windows NT/2K
	// Performance overhead:	Low
	// Security level:			Low
	// Usage scope:				Macro scope
	//*/
	#define iviTR_VERIFY_SOFTICE_NTICE_W(bRetValue)\
	{   bRetValue = TR_Verify_SoftICE_NTICE_W(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bRetValue, iviTR_VERIFY_SOFTICE_NTICE_W)}

	///*!
	// Detect if Soft-Ice kernel debugger is installed and activated in the system. true if exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		Windows NT/2K
	// Performance overhead:	Medium
	// Security level:			Low
	// Usage scope:				Macro scope
	//
	// Note:\n
	// dynamic link version, may cause performance low, use carefully.
	//*/
	#define iviTR_VERIFY_SOFTICE_NTICE_DD_W(bRetValue)\
	{   bRetValue = TR_Verify_SoftICE_NTICE_DD_W(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bRetValue, iviTR_VERIFY_SOFTICE_NTICE_DD_W)}

	///*!
	// Detect if Soft-Ice kernel debugger is installed and activated in the system. true if exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		Windows 95/98
	// Performance overhead:	Low
	// Security level:			Low
	// Usage scope:				Macro scope
	//*/
	#define iviTR_VERIFY_SOFTICE_SIWVID_W(bool_variable)\
	{   bool_variable = TR_Verify_SoftICE_SIWVID_W(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bool_variable, iviTR_VERIFY_SOFTICE_SIWVID_W)}

	///*!
	// Detect if Soft-Ice kernel debugger is installed and activated in the system. true if exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		Windows 95/98
	// Performance overhead:	Medium
	// Security level:			Low
	// Usage scope:				Macro scope
	//
	// Note:\n
	// dynamic link version, may cause performance low, use carefully.
	//*/
	#define iviTR_VERIFY_SOFTICE_SIWVID_DD_W(bRetValue)\
	{   bRetValue = TR_Verify_SoftICE_SIWVID_DD_W(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bRetValue, iviTR_VERIFY_SOFTICE_SIWVID_DD_W)}

	///*!
	// Detect if Soft-Ice kernel debugger is installed and activated in the system. true if exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		Windows 95/98
	// Performance overhead:	Low
	// Security level:			Low
	// Usage scope:				Macro scope
	//*/
	#define iviTR_VERIFY_SOFTICE_SIWDEBUG_W(bRetValue)\
	{   bRetValue = TR_Verify_SoftICE_SIWDEBUG_W(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bRetValue, iviTR_VERIFY_SOFTICE_SIWDEBUG_W)}

	///*!
	// Detect if Soft-Ice kernel debugger is installed and activated in the system. true if exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		Windows 95/98
	// Performance overhead:	Medium
	// Security level:			Low
	// Usage scope:				Macro scope
	//
	// Note:\n
	// dynamic link version, may cause performance low, use carefully.
	//*/
	#define iviTR_VERIFY_SOFTICE_SIWDEBUG_DD_W(bool_variable)\
	{   bool_variable = TR_Verify_SoftICE_SIWDEBUG_DD_W(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bool_variable, iviTR_VERIFY_SOFTICE_SIWDEBUG_DD_W)}

	///*!
	// Detect if Soft-Ice kernel debugger is installed in the system. false if exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		Windows NT/2K
	// Performance overhead:	Low
	// Security level:			Low
	// Usage scope:				Macro scope
	//*/
	#define iviTR_VERIFY_SOFTICE_SIWVIDSTART_W_INV(bRetValue)\
	{   bRetValue = TR_Verify_SoftICE_SIWVIDSTART_W_INV(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(!bRetValue, iviTR_VERIFY_SOFTICE_SIWVIDSTART_W_INV)}

	///*!
	// Detect if Soft-Ice kernel debugger is installed in the system. false if exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		Windows NT/2K
	// Performance overhead:	Medium
	// Security level:			Low
	// Usage scope:				Macro scope
	//
	// Note:\n
	// dynamic link version, may cause performance low, use carefully.
	//*/
	#define iviTR_VERIFY_SOFTICE_SIWVIDSTART_DD_W_INV(bRetValue)\
	{   bRetValue = TR_Verify_SoftICE_SIWVIDSTART_DD_W_INV(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bRetValue, iviTR_VERIFY_SOFTICE_SIWVIDSTART_DD_W_INV)}

	///*!
	// Detect if Soft-Ice kernel debugger is installed in the system. false if exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		Windows NT/2K
	// Performance overhead:	Low
	// Security level:			Low
	// Usage scope:				Macro scope
	//*/
	#define iviTR_VERIFY_SOFTICE_SICE_W_INV(bRetValue)\
	{   bRetValue = TR_Verify_SoftICE_SICE_W_INV(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(!bRetValue, iviTR_VERIFY_SOFTICE_SICE_W_INV)}

	///*!
	// Detect if Soft-Ice kernel debugger is installed and activated in the system. false if exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		Windows NT/2K
	// Performance overhead:	Medium
	// Security level:			Low
	// Usage scope:				Macro scope
	//
	// Note:\n
	// dynamic link version, may cause performance low, use carefully.
	//*/
	#define iviTR_VERIFY_SOFTICE_SICE_DD_W_INV(bRetValue)\
	{   bRetValue = TR_Verify_SoftICE_SICE_DD_W_INV(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(!bRetValue, iviTR_VERIFY_SOFTICE_SICE_DD_W_INV)}

	///*!
	// Detect if Soft-Ice kernel debugger is installed and activated in the system. false if exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		Windows NT/2K
	// Performance overhead:	Low
	// Security level:			Low
	// Usage scope:				Macro scope
	//*/
	#define iviTR_VERIFY_SOFTICE_NTICE_W_INV(bRetValue)\
	{   bRetValue = TR_Verify_SoftICE_NTICE_W_INV(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(!bRetValue, iviTR_VERIFY_SOFTICE_NTICE_W_INV)}

	///*!
	// Detect if Soft-Ice kernel debugger is installed and activated in the system. false if exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		Windows NT/2K
	// Performance overhead:	Medium
	// Security level:			Low
	// Usage scope:				Macro scope
	//
	// Note:\n
	// dynamic link version, may cause performance low, use carefully.
	//*/
	#define iviTR_VERIFY_SOFTICE_NTICE_DD_W_INV(bRetValue)\
	{   bRetValue = TR_Verify_SoftICE_NTICE_DD_W_INV(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(!bRetValue, iviTR_VERIFY_SOFTICE_NTICE_DD_W_INV)}

	///*!
	// Detect if Soft-Ice kernel debugger is installed and activated in the system. false if exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		Windows 95/98
	// Performance overhead:	Low
	// Security level:			Low
	// Usage scope:				Macro scope
	//*/
	#define iviTR_VERIFY_SOFTICE_SIWVID_W_INV(bRetValue)\
	{   bRetValue = TR_Verify_SoftICE_SIWVID_W_INV(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(!bRetValue, iviTR_VERIFY_SOFTICE_SIWVID_W_INV)}

	///*!
	// Detect if Soft-Ice kernel debugger is installed and activated in the system. false if exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		Windows 95/98
	// Performance overhead:	Medium
	// Security level:			Low
	// Usage scope:				Macro scope
	//
	// Note:\n
	// dynamic link version, may cause performance low, use carefully.
	//*/
	#define iviTR_VERIFY_SOFTICE_SIWVID_DD_W_INV(bRetValue)\
	{   bRetValue = TR_Verify_SoftICE_SIWVID_DD_W_INV(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(!bRetValue, iviTR_VERIFY_SOFTICE_SIWVID_DD_W_INV)}

	///*!
	// Detect if Soft-Ice kernel debugger is installed and activated in the system. false if exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		Windows 95/98
	// Performance overhead:	Low
	// Security level:			Low
	// Usage scope:				Macro scope
	//*/
	#define iviTR_VERIFY_SOFTICE_SIWDEBUG_W_INV(bRetValue)\
	{   bRetValue = TR_Verify_SoftICE_SIWDEBUG_W_INV(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(!bRetValue, iviTR_VERIFY_SOFTICE_SIWDEBUG_W_INV)}

	///*!
	// Detect if Soft-Ice kernel debugger is installed and activated in the system. false if exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		Windows 95/98
	// Performance overhead:	Medium
	// Security level:			Low
	// Usage scope:				Macro scope
	//
	// Note:\n
	// dynamic link version, may cause performance low, use carefully.
	//*/
	#define iviTR_VERIFY_SOFTICE_SIWDEBUG_DD_W_INV(bRetValue)\
	{   bRetValue = TR_Verify_SoftICE_SIWDEBUG_DD_W_INV(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(!bRetValue, iviTR_VERIFY_SOFTICE_SIWDEBUG_DD_W_INV)}

	///*!
	// Detect if Soft-Ice kernel debugger is installed and activated in the system. true if exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		Windows NT/2K
	// Performance overhead:	Low
	// Security level:			Low
	// Usage scope:				Macro scope
	//*/
	// Mark off as the code is moved to the header.
	//#define iviTR_VERIFY_SOFTICE_LCREAT(bRetValue)\
	//{   bRetValue = TR_Verify_SoftICE_LCREAT(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bRetValue, iviTR_VERIFY_SOFTICE_LCREAT)}
	#define iviTR_VERIFY_SOFTICE_LCREAT(bRetValue) \
	{ \
		char mystring[16]; \
		mystring[0] = '\\'; mystring[1] = '\\'; mystring[2] = '.'; mystring[3] = '\\'; \
		iviTR_DUMMY_CALL_9(37) \
		mystring[4] = 'S'; mystring[5] = 'I'; mystring[6] = 'W'; mystring[7] = 'V'; \
		mystring[8] = 'I'; mystring[9] = 'D'; mystring[10] = 'S'; mystring[11] = 'T'; \
		mystring[12] = 'A'; mystring[13] = 'R'; mystring[14] = 'T'; mystring[15] = '\0'; \
		iviTR_DUMMY_CALL_5(5) \
		bRetValue = false; \
		if (::_lcreat(mystring,0) != HFILE_ERROR) \
		{ \
			bRetValue = true; \
			iviTR_DUMMY_CALL_1(38) \
		} \
		else \
		{ \
			bRetValue = false; \
			iviTR_DUMMY_CALL_10(17) \
		} \
		iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bRetValue, iviTR_VERIFY_SOFTICE_LCREAT) \
	}

	///*!
	// Detect if Soft-Ice kernel debugger is installed and activated in the system. true if exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		Windows 95/98
	// Performance overhead:	Low
	// Security level:			Low
	// Usage scope:				Macro scope
	//*/
	#define iviTR_VERIFY_SOFTICE_LCREAT1(bRetValue)\
	{   bRetValue = TR_Verify_SoftICE_LCREAT1(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bRetValue, iviTR_VERIFY_SOFTICE_LCREAT1)}

	///*!
	// Detect if Soft-Ice kernel debugger is installed and activated in the system. true if exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		Windows NT/2K
	// Performance overhead:	Low
	// Security level:			Low
	// Usage scope:				Macro scope
	//*/
	// Mark off as the code is moved to the header.
	//#define iviTR_VERIFY_SOFTICE_LCREAT2(bRetValue)\
	//{   bRetValue = TR_Verify_SoftICE_LCREAT2(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bRetValue, iviTR_VERIFY_SOFTICE_LCREAT2)}
	#define iviTR_VERIFY_SOFTICE_LCREAT2(bRetValue) \
	{ \
		iviTR_DUMMY_CALL_3(9) \
		char mystring[10]; \
		mystring[0] = '\\'; mystring[1] = '\\'; mystring[2] = '.'; mystring[3] = '\\'; \
		iviTR_DUMMY_CALL_5(74) \
		mystring[4] = 'N'; mystring[5] = 'T'; mystring[6] = 'I'; mystring[7] = 'C'; \
		mystring[8] = 'E';	mystring[9] = '\0'; \
		bRetValue = false; \
		if (::_lcreat(mystring,0) != HFILE_ERROR) \
		{ \
			iviTR_DUMMY_CALL_6(27) \
			bRetValue = true; \
		} \
		else \
		{ \
			iviTR_DUMMY_CALL_1(88) \
			bRetValue = false; \
		} \
		iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bRetValue, iviTR_VERIFY_SOFTICE_LCREAT2) \
	}

	///*!
	// Detect if Soft-Ice kernel debugger is installed and activated in the system. true if exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		Windows 95/98
	// Performance overhead:	Low
	// Security level:			Low
	// Usage scope:				Macro scope
	//*/
	#define iviTR_VERIFY_SOFTICE_LCREAT3(bRetValue)\
	{   bRetValue = TR_Verify_SoftICE_LCREAT3(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bRetValue, iviTR_VERIFY_SOFTICE_LCREAT3)}

	///*!
	// Detect if Soft-Ice kernel debugger is installed and activated in the system. true if exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		Windows 95/98
	// Performance overhead:	Low
	// Security level:			Low
	// Usage scope:				Macro scope
	//*/
	#define iviTR_VERIFY_SOFTICE_LCREAT4(bRetValue)\
	{   bRetValue = TR_Verify_SoftICE_LCREAT4(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bRetValue, iviTR_VERIFY_SOFTICE_LCREAT4)}

	///*!
	// Detect if Soft-Ice kernel debugger is installed and activated in the system. false if exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		Windows NT/2K
	// Performance overhead:	Low
	// Security level:			Low
	// Usage scope:				Macro scope
	//*/
	// Mark off as the code is moved to the header.
	//#define iviTR_VERIFY_SOFTICE_LCREAT_INV(bRetValue)\
	//{   bRetValue = TR_Verify_SoftICE_LCREAT_INV(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(!bRetValue, iviTR_VERIFY_SOFTICE_LCREAT_INV)}
	#define iviTR_VERIFY_SOFTICE_LCREAT_INV(bRetValue) \
	{ \
		char mystring[16]; \
		iviTR_DUMMY_CALL_4(41) \
		mystring[0] = '\\'; mystring[1] = '\\'; mystring[2] = '.'; mystring[3] = '\\'; \
		mystring[4] = 'S'; mystring[5] = 'I'; mystring[6] = 'W'; mystring[7] = 'V'; \
		mystring[8] = 'I'; mystring[9] = 'D'; mystring[10] = 'S'; mystring[11] = 'T'; \
		iviTR_DUMMY_CALL_7(51) \
		bRetValue = true; \
		mystring[12] = 'A'; mystring[13] = 'R'; mystring[14] = 'T'; mystring[15] = '\0'; \
		if (::_lcreat(mystring,0) != HFILE_ERROR) \
		{ \
			iviTR_DUMMY_CALL_1(91) \
			bRetValue = false; \
		} \
		else \
		{ \
			bRetValue = true; \
			iviTR_DUMMY_CALL_8(68) \
		} \
		iviTR_DUMP_ERROR_TR_MACRO_ENTRY(!bRetValue, iviTR_VERIFY_SOFTICE_LCREAT_INV) \
	}

	///*!
	// Detect if Soft-Ice kernel debugger is installed and activated in the system. false if exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		Windows 95/98
	// Performance overhead:	Low
	// Security level:			Low
	// Usage scope:				Macro scope
	//*/
	#define iviTR_VERIFY_SOFTICE_LCREAT1_INV(bRetValue)\
	{   bRetValue = TR_Verify_SoftICE_LCREAT1_INV(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(!bRetValue, iviTR_VERIFY_SOFTICE_LCREAT1_INV)}

	///*!
	// Detect if Soft-Ice kernel debugger is installed and activated in the system. false if exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		Windows NT/2K
	// Performance overhead:	Low
	// Security level:			Low
	// Usage scope:				Macro scope
	//*/
	// Mark off as the code is moved to the header.
	//#define iviTR_VERIFY_SOFTICE_LCREAT2_INV(bRetValue)\
	//{   bRetValue = TR_Verify_SoftICE_LCREAT2_INV(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(!bRetValue, iviTR_VERIFY_SOFTICE_LCREAT2_INV)}
	#define iviTR_VERIFY_SOFTICE_LCREAT2_INV(bRetValue) \
	{ \
		char mystring[10]; \
		mystring[0] = '\\'; mystring[1] = '\\'; mystring[2] = '.'; mystring[3] = '\\'; \
		iviTR_DUMMY_CALL_5(93) \
		bRetValue = true; \
		mystring[4] = 'N'; mystring[5] = 'T'; mystring[6] = 'I'; mystring[7] = 'C'; \
		mystring[8] = 'E';	mystring[9] = '\0'; \
		iviTR_DUMMY_CALL_9(47) \
		if (::_lcreat(mystring,0) != HFILE_ERROR) \
		{ \
			bRetValue = false; \
		} \
		else \
		{ \
			iviTR_DUMMY_CALL_3(81) \
			bRetValue = true; \
		} \
		iviTR_DUMP_ERROR_TR_MACRO_ENTRY(!bRetValue, iviTR_VERIFY_SOFTICE_LCREAT2_INV) \
	}

	///*!
	// Detect if Soft-Ice kernel debugger is installed and activated in the system. false if exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		Windows 95/98
	// Performance overhead:	Low
	// Security level:			Low
	// Usage scope:				Macro scope
	//*/
	#define iviTR_VERIFY_SOFTICE_LCREAT3_INV(bRetValue)\
	{   bRetValue = TR_Verify_SoftICE_LCREAT3_INV(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(!bRetValue, iviTR_VERIFY_SOFTICE_LCREAT3_INV)}

	///*!
	// Detect if Soft-Ice kernel debugger is installed and activated in the system. false if exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		Windows 95/98
	// Performance overhead:	Low
	// Security level:			Low
	// Usage scope:				Macro scope
	//*/
	#define iviTR_VERIFY_SOFTICE_LCREAT4_INV(bRetValue)\
	{   bRetValue = TR_Verify_SoftICE_LCREAT4_INV(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(!bRetValue, iviTR_VERIFY_SOFTICE_LCREAT4_INV)}

	///*!
	// Detect if Soft-Ice kernel debugger is installed and activated in the system. true if exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		Windows NT/2K
	// Performance overhead:	Low
	// Security level:			Low
	// Usage scope:				Macro scope
	//*/
	//#define iviTR_VERIFY_SOFTICE_LOPEN(bRetValue)\
	//{   bRetValue = TR_Verify_SoftICE_LOPEN(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bRetValue, iviTR_VERIFY_SOFTICE_LOPEN)}
	// Mark off as the code is moved to the header.
	#define iviTR_VERIFY_SOFTICE_LOPEN(bRetValue) \
	{ \
		iviTR_DUMMY_CALL_6(54) \
		char mystring[16]; \
		mystring[0] = '\\'; mystring[1] = '\\'; mystring[2] = '.'; mystring[3] = '\\'; \
		mystring[4] = 'S'; mystring[5] = 'I'; mystring[6] = 'W'; mystring[7] = 'V'; \
		iviTR_DUMMY_CALL_7(18) \
		bRetValue = false; \
		mystring[8] = 'I'; mystring[9] = 'D'; mystring[10] = 'S'; mystring[11] = 'T'; \
		mystring[12] = 'A'; mystring[13] = 'R'; mystring[14] = 'T'; mystring[15] = '\0'; \
		if (::_lopen(mystring,0) != HFILE_ERROR) \
		{ \
			iviTR_DUMMY_CALL_4(25) \
			bRetValue = true; \
		} \
		else \
		{ \
			bRetValue = false; \
			iviTR_DUMMY_CALL_3(76) \
		} \
		iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bRetValue, iviTR_VERIFY_SOFTICE_LOPEN) \
	}

	///*!
	// Detect if Soft-Ice kernel debugger is installed and activated in the system. true if exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		Windows 95/98
	// Performance overhead:	Low
	// Security level:			Low
	// Usage scope:				Macro scope
	//*/
	#define iviTR_VERIFY_SOFTICE_LOPEN1(bRetValue)\
	{   bRetValue = TR_Verify_SoftICE_LOPEN1(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bRetValue, iviTR_VERIFY_SOFTICE_LOPEN1)}

	///*!
	// Detect if Soft-Ice kernel debugger is installed and activated in the system. true if exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		Windows NT/2K
	// Performance overhead:	Low
	// Security level:			Low
	// Usage scope:				Macro scope
	//*/
	// Mark off as the code is moved to the header.
	//#define iviTR_VERIFY_SOFTICE_LOPEN2(bRetValue)\
	//{   bRetValue = TR_Verify_SoftICE_LOPEN2(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bRetValue, iviTR_VERIFY_SOFTICE_LOPEN2)}
	#define iviTR_VERIFY_SOFTICE_LOPEN2(bRetValue) \
	{ \
		char mystring[10]; \
		mystring[0] = '\\'; mystring[1] = '\\'; mystring[2] = '.'; mystring[3] = '\\'; \
		iviTR_DUMMY_CALL_3(13) \
		mystring[4] = 'N'; mystring[5] = 'T'; mystring[6] = 'I'; mystring[7] = 'C'; \
		mystring[8] = 'E';	mystring[9] = '\0'; \
		iviTR_DUMMY_CALL_10(72) \
		bRetValue = false; \
		if (::_lopen(mystring,0) != HFILE_ERROR) \
		{ \
			iviTR_DUMMY_CALL_5(7) \
			bRetValue = true; \
		} \
		else \
		{ \
			bRetValue = false; \
		} \
		iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bRetValue, iviTR_VERIFY_SOFTICE_LOPEN2) \
	}


	///*!
	// Detect if Soft-Ice kernel debugger is installed and activated in the system. true if exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		Windows 95/98
	// Performance overhead:	Low
	// Security level:			Low
	// Usage scope:				Macro scope
	//*/
	#define iviTR_VERIFY_SOFTICE_LOPEN3(bRetValue)\
	{   bRetValue = TR_Verify_SoftICE_LOPEN3(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bRetValue, iviTR_VERIFY_SOFTICE_LOPEN3)}

	///*!
	// Detect if Soft-Ice kernel debugger is installed and activated in the system. true if exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		Windows 95/98
	// Performance overhead:	Low
	// Security level:			Low
	// Usage scope:				Macro scope
	//*/
	#define iviTR_VERIFY_SOFTICE_LOPEN4(bRetValue)\
	{   bRetValue = TR_Verify_SoftICE_LOPEN4(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bRetValue, iviTR_VERIFY_SOFTICE_LOPEN4)}

	///*!
	// Detect if Soft-Ice kernel debugger is installed and activated in the system. false if exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		Windows NT/2K
	// Performance overhead:	Low
	// Security level:			Low
	// Usage scope:				Macro scope
	//*/
	// Mark off as the code is moved to the header.
	//#define iviTR_VERIFY_SOFTICE_LOPEN_INV(bRetValue)\
	//{   bRetValue = TR_Verify_SoftICE_LOPEN_INV(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(!bRetValue, iviTR_VERIFY_SOFTICE_LOPEN_INV)}
	#define iviTR_VERIFY_SOFTICE_LOPEN_INV(bRetValue) \
	{ \
		char mystring[16]; \
		mystring[0] = '\\'; mystring[1] = '\\'; mystring[2] = '.'; mystring[3] = '\\'; \
		iviTR_DUMMY_CALL_6(49) \
		bRetValue = true; \
		mystring[4] = 'S'; mystring[5] = 'I'; mystring[6] = 'W'; mystring[7] = 'V'; \
		mystring[8] = 'I'; mystring[9] = 'D'; mystring[10] = 'S'; mystring[11] = 'T'; \
		iviTR_DUMMY_CALL_2(33) \
		mystring[12] = 'A'; mystring[13] = 'R'; mystring[14] = 'T'; mystring[15] = '\0'; \
		if (::_lopen(mystring,0) != HFILE_ERROR) \
		{ \
			iviTR_DUMMY_CALL_4(73) \
			bRetValue = false; \
		} \
		else \
		{ \
			bRetValue = true; \
			iviTR_DUMMY_CALL_8(96) \
		} \
		iviTR_DUMP_ERROR_TR_MACRO_ENTRY(!bRetValue, iviTR_VERIFY_SOFTICE_LOPEN_INV) \
	}

	///*!
	// Detect if Soft-Ice kernel debugger is installed and activated in the system. false if exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		Windows 95/98
	// Performance overhead:	Low
	// Security level:			Low
	// Usage scope:				Macro scope
	//*/
	#define iviTR_VERIFY_SOFTICE_LOPEN1_INV(bRetValue)\
	{   bRetValue = TR_Verify_SoftICE_LOPEN1_INV(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(!bRetValue, iviTR_VERIFY_SOFTICE_LOPEN1_INV)}

	///*!
	// Detect if Soft-Ice kernel debugger is installed and activated in the system. false if exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		Windows 95/98
	// Performance overhead:	Low
	// Security level:			Low
	// Usage scope:				Macro scope
	//*/
	//#define iviTR_VERIFY_SOFTICE_LOPEN2_INV(bRetValue)\
	//{   bRetValue = TR_Verify_SoftICE_LOPEN2_INV(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(!bRetValue, iviTR_VERIFY_SOFTICE_LOPEN2_INV)}
	#define iviTR_VERIFY_SOFTICE_LOPEN2_INV(bRetValue) \
	{ \
		iviTR_DUMMY_CALL_4(32) \
		char mystring[10]; \
		mystring[0] = '\\'; mystring[1] = '\\'; mystring[2] = '.'; mystring[3] = '\\'; \
		mystring[4] = 'N'; mystring[5] = 'T'; mystring[6] = 'I'; mystring[7] = 'C'; \
		mystring[8] = 'E';	mystring[9] = '\0'; \
		iviTR_DUMMY_CALL_1(82) \
		bRetValue = true; \
		if (::_lopen(mystring,0) != HFILE_ERROR) \
		{ \
			iviTR_DUMMY_CALL_5(63) \
			bRetValue = false; \
		} \
		else \
		{ \
			iviTR_DUMMY_CALL_7(63) \
			bRetValue = true; \
		} \
		iviTR_DUMP_ERROR_TR_MACRO_ENTRY(!bRetValue, iviTR_VERIFY_SOFTICE_LOPEN2_INV) \
	}

	///*!
	// Detect if Soft-Ice kernel debugger is installed and activated in the system. false if exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		Windows NT/2K
	// Performance overhead:	Low
	// Security level:			Low
	// Usage scope:				Macro scope
	//*/
	#define iviTR_VERIFY_SOFTICE_LOPEN3_INV(bRetValue)\
	{   bRetValue = TR_Verify_SoftICE_LOPEN3_INV(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(!bRetValue, iviTR_VERIFY_SOFTICE_LOPEN3_INV)}

	///*!
	// Detect if Soft-Ice kernel debugger is installed and activated in the system. false if exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		Windows 95/98
	// Performance overhead:	Low
	// Security level:			Low
	// Usage scope:				Macro scope
	//*/
	#define iviTR_VERIFY_SOFTICE_LOPEN4_INV(bRetValue)\
	{   bRetValue = TR_Verify_SoftICE_LOPEN4_INV(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(!bRetValue, iviTR_VERIFY_SOFTICE_LOPEN4_INV)}

	#include "WINSVC.H"

	///*!
	// Detect if Soft-Ice kernel debugger is installed and activated. true if exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		Windows NT/2K
	// Performance overhead:	High
	// Security level:			Low
	// Usage scope:				Macro scope
	//*/
	// Mark off as the code is moved to the header.
	//#define iviTR_VERIFY_SOFTICE_NTICE_SERVICE(bRetValue)\
	//{	bRetValue = TR_Verify_SoftICE_NTICE_SERVICE(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bRetValue, iviTR_VERIFY_SOFTICE_NTICE_SERVICE)}
	#define iviTR_VERIFY_SOFTICE_NTICE_SERVICE(bRetValue) \
	{ \
		char szName[6]; \
		iviTR_DUMMY_CALL_5(21) \
		bRetValue = false; \
		SERVICE_STATUS ssStatus; \
   		szName[3]='C'; szName[0]='N'; szName[2]='I'; szName[1]='T'; szName[4]='E'; szName[5]='\0'; \
		SC_HANDLE schSCManager = ::OpenSCManagerA(NULL, SERVICES_ACTIVE_DATABASEA, SC_MANAGER_ALL_ACCESS); \
		iviTR_DUMMY_CALL_9(58) \
		if (schSCManager != NULL) \
		{ \
			iviTR_DUMMY_CALL_10(83) \
			SC_HANDLE schService  = ::OpenServiceA(schSCManager, szName, SERVICE_ALL_ACCESS); \
			if (schService == NULL) \
			{ \
				::CloseServiceHandle(schSCManager); \
				iviTR_DUMMY_CALL_2(10) \
				bRetValue = false; \
			} \
			else \
			{ \
				iviTR_DUMMY_CALL_3(53) \
				if (((::QueryServiceStatus(schService, &ssStatus)) != 0) && (ssStatus.dwCurrentState == SERVICE_RUNNING)) \
				{ \
					::CloseServiceHandle(schService); \
					::CloseServiceHandle(schSCManager); \
					iviTR_DUMMY_CALL_6(58) \
					bRetValue = true; \
				} \
				::CloseServiceHandle(schService); \
				::CloseServiceHandle(schSCManager); \
			} \
		} \
		else \
		{ \
			bRetValue = false; \
		} \
		iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bRetValue, iviTR_VERIFY_SOFTICE_NTICE_SERVICE) \
	}

	///*!
	// Detect if Soft-Ice kernel debugger is installed and activated. false if exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		Windows NT/2K
	// Performance overhead:	High
	// Security level:			Low
	// Usage scope:				Macro scope
	//*/
	// Mark off as the code is moved to the header.
	//#define iviTR_VERIFY_SOFTICE_NTICE_SERVICE_INV(bRetValue)\
	//{	bRetValue = TR_Verify_SoftICE_NTICE_SERVICE_INV(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(!bRetValue, iviTR_VERIFY_SOFTICE_NTICE_SERVICE_INV)}
	#define iviTR_VERIFY_SOFTICE_NTICE_SERVICE_INV(bRetValue) \
	{ \
		char szName[6]; \
		SERVICE_STATUS ssStatus; \
   		szName[3]='C'; szName[0]='N'; szName[2]='I'; szName[1]='T'; szName[4]='E'; szName[5]='\0'; \
		SC_HANDLE schSCManager = ::OpenSCManagerA(NULL, SERVICES_ACTIVE_DATABASE, SC_MANAGER_ALL_ACCESS); \
		iviTR_DUMMY_CALL_1(55) \
		bRetValue = true; \
		if (schSCManager != NULL) \
		{ \
			SC_HANDLE schService = ::OpenServiceA(schSCManager, szName, SERVICE_ALL_ACCESS); \
			if (schService == NULL) \
			{ \
				::CloseServiceHandle(schSCManager); \
				iviTR_DUMMY_CALL_4(39) \
				bRetValue = true; \
			} \
			else \
			{ \
				if (((::QueryServiceStatus(schService, &ssStatus)) != 0) && (ssStatus.dwCurrentState == SERVICE_RUNNING)) \
				{ \
					::CloseServiceHandle(schService); \
					iviTR_DUMMY_CALL_2(45) \
					::CloseServiceHandle(schSCManager); \
					bRetValue = false; \
				} \
				::CloseServiceHandle(schService); \
				iviTR_DUMMY_CALL_8(41) \
				::CloseServiceHandle(schSCManager); \
			} \
		} \
		else \
		{ \
			bRetValue = true; \
		} \
		iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bRetValue, iviTR_VERIFY_SOFTICE_NTICE_SERVICE) \
	}

	///*!
	// Detect if Syser kernel debugger is installed and activated. true if exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		Windows NT/2K
	// Performance overhead:	Low
	// Security level:			Low
	// Usage scope:				Macro scope
	//*/
	// Mark off as the code is moved to the header.
	//#define iviTR_VERIFY_SYSER_A(bRetValue)\
	//{   bRetValue = TR_Verify_Syser_A(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bRetValue, iviTR_VERIFY_SYSER_A)}
	#define iviTR_VERIFY_SYSER_A(bRetValue) \
	{ \
		char name1[10]; HANDLE hDevice; \
		iviTR_DUMMY_CALL_4(81) \
		name1[0]='\\'; name1[1]=name1[0]; name1[3]=name1[0]; name1[2]='.'; \
		name1[9]=0; name1[4]='S'; name1[5]='Y'; name1[6]='S'; name1[7]='E'; name1[8]='R'; \
		bRetValue = false; \
		iviTR_DUMMY_CALL_7(31) \
		if ((hDevice = ::CreateFileA(name1,FILE_FLAG_WRITE_THROUGH,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0)) != INVALID_HANDLE_VALUE) \
		{ \
			iviTR_DUMMY_CALL_5(49) \
			::CloseHandle(hDevice); \
			bRetValue = true; \
		} \
		else \
		{ \
			iviTR_DUMMY_CALL_3(12) \
			bRetValue = false; \
		} \
		iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bRetValue, iviTR_VERIFY_SYSER_A) \
	}

	///*!
	// Detect if Syser kernel debugger is installed and activated. true if exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		Windows NT/2K
	// Performance overhead:	Medium
	// Security level:			Low
	// Usage scope:				Macro scope
	//
	// Note:\n
	// dynamic link version, may cause performance low, use carefully.
	//*/
	#define iviTR_VERIFY_SYSER_DD_A(bRetValue)\
	{   bRetValue = TR_Verify_Syser_DD_A(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bRetValue, iviTR_VERIFY_SYSER_DD_A)}

	///*!
	// Detect if Syser kernel debugger is installed and activated. true if exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		Windows NT/2K
	// Performance overhead:	Low
	// Security level:			Low
	// Usage scope:				Macro scope
	//*/
	#define iviTR_VERIFY_SYSER_W(bRetValue)\
	{   bRetValue = TR_Verify_Syser_W(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bRetValue, iviTR_VERIFY_SYSER_W)}

	///*!
	// Detect if Syser kernel debugger is installed and activated. true if exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		Windows NT/2K
	// Performance overhead:	Medium
	// Security level:			Low
	// Usage scope:				Macro scope
	//
	// Note:\n
	// dynamic link version, may cause performance low, use carefully.
	//*/
	#define iviTR_VERIFY_SYSER_DD_W(bRetValue)\
	{   bRetValue = TR_Verify_Syser_DD_W(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bRetValue, iviTR_VERIFY_SYSER_DD_W)}

	///*!
	// Detect if Syser kernel debugger installed and activated. true if exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		Windows NT/2K
	// Performance overhead:	Low
	// Security level:			Low
	// Usage scope:				Macro scope
	//*/
	#define iviTR_VERIFY_SYSER_LCREAT(bRetValue)\
	{   bRetValue = TR_Verify_Syser_LCREATE(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bRetValue, iviTR_VERIFY_SYSER_LCREAT)}

	///*!
	// Detect if Syser kernel debugger is installed and activated. true if exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		Windows NT/2K
	// Performance overhead:	Low
	// Security level:			Low
	// Usage scope:				Macro scope
	//*/
	#define iviTR_VERIFY_SYSER_LOPEN(bRetValue)\
	{   bRetValue = TR_Verify_Syser_LOPEN(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bRetValue, iviTR_VERIFY_SYSER_LOPEN)}

	///*!
	// Detect if debugger is debugging current program, return '4' is exist.
	//
	// Return value - None
	//
	// -#dwRetValue - [IN] can not be NULL, return value in DWORD.
	//
	// Supported platform:		Windows NT/2K
	// Performance overhead:	Low
	// Security level:			High
	// Usage scope:				Macro scope
	//*/
	#define iviTR_FIND_INT_3H_UNHANDLEDEXCEPTIONFILTER(dwRetValue) \
	{ \
		dwRetValue = 0; \
		iviTR_DUMMY_CALL_3(46) \
		LPBYTE lpbAddr = (LPBYTE)::UnhandledExceptionFilter; \
		iviTR_DUMMY_CALL_10(55) \
		if (0xCC == *lpbAddr) \
			dwRetValue = 4; \
		iviTR_DUMP_ERROR_TR_MACRO_ENTRY((4 == dwRetValue), iviTR_FIND_INT_3H_UNHANDLEDEXCEPTIONFILTER) \
	}

	/* -- remove in TRSDK -- begin
	__forceinline DWORD __cdecl TR_Find_Int3_UnhandledExecptionFilter()
	{
		DWORD dwRet = 0;

		iviTR_FIND_INT_3H_UNHANDLEDEXCEPTIONFILTER(dwRet)

		return dwRet;
	}
	----- remove in TRSDK -- end */

	///*!
	// Detect if debugger is debugging current program, true is exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		All Windows
	// Performance overhead:	Low
	// Security level:			Medium
	// Usage scope:				Macro scope
	//*/
	#define iviTR_DAEMON_FINDER(bRetValue) \
	{ \
		bRetValue = false; \
		iviTR_DUMMY_CALL_4(72) \
		BYTE bChecker = 0; \
		__asm\
		{\
			__asm push		eax\
			__asm xor		eax, eax\
			__asm mov		eax, fs:[30h]\
			__asm movzx		eax, byte ptr [eax + 2h]\
			__asm mov		bChecker, al\
			__asm pop		eax\
		} \
		iviTR_DUMMY_CALL_7(29) \
		if (0 != bChecker) \
			bRetValue = true; \
		iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bRetValue, iviTR_DAEMON_FINDER) \
	}
	
	/* -- remove in TRSDK -- begin	
	__forceinline bool __cdecl TR_IsODBGLoaded()
	{
		bool bDebug = false;

		iviTR_DAEMON_FINDER(bDebug)

		return bDebug;
	}
	----- remove in TRSDK -- end */	

	///*!
	// Check if debugger is debugging current program, return true is exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		All Windows
	// Performance overhead:	Low
	// Security level:			High
	// Usage scope:				Macro scope
	//*/
	#define iviTR_DEBUG_REGISTER_CHECK(bRetValue) \
	{	bRetValue = TR_Debug_Register_Check(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bRetValue, iviTR_DEBUG_REGISTER_CHECK)}

	///*!
	// Check if debugger is debugging current program, return true is exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		All Windows
	// Performance overhead:	Low
	// Security level:			High
	// Usage scope:				Macro scope
	//*/
	#define iviTR_DEBUG_REGISTER_CHECK2(bRetValue) \
	{	\
		bRetValue = false; \
		DWORD dwCurThreadID = ::GetCurrentThreadId();	\
		HANDLE hCurThread = ::OpenThread(	\
			THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT | THREAD_SET_CONTEXT,	\
			TRUE, \
			dwCurThreadID);	\
		if (hCurThread)	\
		{					\
			DWORD thID;	\
			trDETECTDR detectDR = {0, 0};	\
			detectDR.m_hCurThread = hCurThread;	\
			HANDLE hDetThread = ::CreateThread(0, 512, gtrDetectDRFunc, &detectDR, 0, &thID);	\
			if (hDetThread)	\
			{	\
				if (WAIT_TIMEOUT == ::WaitForSingleObject(hDetThread, 200))	\
					bRetValue = false;	\
				else	\
					bRetValue = detectDR.m_bReturn;	\
				::CloseHandle(hDetThread);	\
			}	\
			else	\
				bRetValue = false;	\
			::CloseHandle(hCurThread);	\
		}	\
		else	\
			bRetValue = false;	\
		iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bRetValue, iviTR_DEBUG_REGISTER_CHECK2);	\
	}

	///*!
	// Clear all Debug registers in the current thread, return true is successful.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		All Windows
	// Performance overhead:	Low
	// Security level:			High
	// Usage scope:				Macro scope
	//*/
	#define iviTR_DEBUG_REGISTER_CLEAR(bRetValue) \
	{	\
		bRetValue = true; \
		DWORD dwCurThreadID = ::GetCurrentThreadId();	\
		HANDLE hCurThread = ::OpenThread(	\
			THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT | THREAD_SET_CONTEXT,	\
			TRUE, \
			dwCurThreadID);	\
		if (hCurThread)	\
		{					\
			DWORD thID;	\
			trDETECTDR detectDR = {0, 0};	\
			detectDR.m_hCurThread = hCurThread;	\
			HANDLE hClrThread = ::CreateThread(0, 512, gtrClearDRFunc, &detectDR, 0, &thID);	\
			if (hClrThread)	\
			{	\
				if (WAIT_TIMEOUT == ::WaitForSingleObject(hClrThread, 200))	\
					bRetValue = false;	\
				else	\
					bRetValue = detectDR.m_bReturn;	\
				::CloseHandle(hClrThread);	\
			}	\
			else	\
				bRetValue = false;	\
			::CloseHandle(hCurThread);	\
		}	\
		else	\
			bRetValue = false;	\
		iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bRetValue, iviTR_DEBUG_REGISTER_CLEAR);	\
	}

	///*!
	// Detect if debugger is debugging current program, return zero if debugger found.
	//
	// Return value - None
	//
	// -#dwRetValue - [IN] can not be NULL, return value in DWORD.
	//
	// Supported platform:		All Windows
	// Performance overhead:	Low
	// Security level:			High
	// Usage scope:				Macro scope
	//*/
	
	#define iviTR_DETECT_TRACER_TRAP_FLAG(dwRetValue)\
	{	\
		dwRetValue = (TR_Detect_Tracer_Trap_Flag() ? 1 : 0); \
		iviTR_DUMP_ERROR_TR_MACRO_ENTRY((0 == dwRetValue), iviTR_DETECT_TRACER_TRAP_FLAG) \
	}
	
	///*!
	// Detect if debugger is debugging current program by using single
	// stepping (e.g. SoftICE), true is debugger exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		Windows NT/2K
	// Performance overhead:	Low
	// Security level:			High
	// Usage scope:				Macro scope
	//*/
	#define iviTR_INT1H_CHECK(bRetValue)			\
	{												\
		bRetValue = TR_DetectInt1h();				\
		iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bRetValue, iviTR_INT1H_CHECK) \
	}

	///*!
	// Detect if debugger is debugging current program by using single
	// stepping (e.g. SoftICE), false is debugger exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		Windows NT/2K
	// Performance overhead:	Low
	// Security level:			High
	// Usage scope:				Macro scope
	//*/
	#define iviTR_ICEBP_CHECK(bRetValue)			\
	{												\
		bRetValue = TR_DetectICEBP();				\
		iviTR_DUMP_ERROR_TR_MACRO_ENTRY((false == bRetValue), iviTR_ICEBP_CHECK) \
	}
	
	///*!
	// Detect if debugger is debugging current program, true is debugger exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		All Windows
	// Performance overhead:	Low
	// Security level:			Medium
	// Usage scope:				Macro scope
	//*/
	// Mark off as the code is moved to the header.
	//#define iviTR_API_ISDEBUGGERPRESENT_CHECK(bRetValue) {bRetValue = TR_API_IsDebuggerPresent(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bRetValue, iviTR_API_ISDEBUGGERPRESENT_CHECK)}

	#define iviTR_API_ISDEBUGGERPRESENT_CHECK(bRetValue) \
	{ \
		iviTR_DUMMY_CALL_2(48) \
		typedef bool (WINAPI *APIIsDebuggerPresent)(VOID); \
		APIIsDebuggerPresent debuggerPresent = NULL; \
		char szkernel32[13]; \
		szkernel32[0] = 'k'; szkernel32[1] = 'e'; szkernel32[2] = 'r'; szkernel32[3] = 'n'; \
		iviTR_DUMMY_CALL_8(47) \
		szkernel32[4] = 'e'; szkernel32[5] = 'l'; szkernel32[6] = '3'; szkernel32[7] = '2'; \
		szkernel32[8] = '.'; szkernel32[9] = 'd'; szkernel32[10] = 'l'; szkernel32[11] = 'l'; \
		szkernel32[12] = '\0'; \
		iviTR_DUMMY_CALL_10(42) \
		char szIsDebugger[18]; \
		szIsDebugger[0] = 'I'; szIsDebugger[1] = 's'; szIsDebugger[2] = 'D'; szIsDebugger[3] = 'e'; \
		szIsDebugger[4] = 'b'; szIsDebugger[5] = 'u'; szIsDebugger[6] = 'g'; szIsDebugger[7] = 'g'; \
		iviTR_DUMMY_CALL_8(74) \
		szIsDebugger[8] = 'e'; szIsDebugger[9] = 'r'; szIsDebugger[10] = 'P'; szIsDebugger[11] = 'r'; \
		szIsDebugger[12] = 'e'; szIsDebugger[13] = 's'; szIsDebugger[14] = 'e'; szIsDebugger[15] = 'n'; \
		szIsDebugger[16] = 't'; szIsDebugger[17] = '\0'; \
		bRetValue = FALSE; \
		iviTR_DUMMY_CALL_5(13) \
		HMODULE hLib = ::LoadLibraryA(szkernel32); \
		if(hLib != 0) { \
			APIIsDebuggerPresent debuggerPresent = NULL; \
			iviTR_DUMMY_CALL_3(59) \
			debuggerPresent = (APIIsDebuggerPresent)::GetProcAddress(hLib,szIsDebugger); \
			if(debuggerPresent != NULL)	{ \
				bRetValue = debuggerPresent(); \
			} \
			iviTR_DUMMY_CALL_9(22) \
			::FreeLibrary(hLib); \
			hLib = NULL; \
		} \
		iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bRetValue, iviTR_API_ISDEBUGGERPRESENT_CHECK) \
	}

	///*!
	// [this macro is unstable so far. do not use it !!]
	//
	// Check if SoftIce kernel debugger is installed in the system, if Soft-ICE exist, hang.
	//
	// Return value - None
	//
	// Supported platform:		All Windows/Linux
	// Performance overhead:	Low
	// Security level:			Low
	// Usage scope:				Macro scope
	//
	// Note:\n
	// Some CPU will cause unstable,
	// Not available for current Soft-ICE. Don't use it now.
	//*/
	//#define iviTR_LOCK_CMPXCHG8B() {iviTR_DUMP_ERROR_TR_MACRO_ENTRY(true, Begin_for_iviTR_LOCK_CMPXCHG8B) TR_Lock_Cmpxchg8b(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(true, End_for_iviTR_LOCK_CMPXCHG8B)}
	#define iviTR_LOCK_CMPXCHG8B()

	///*!
	// [this macro is unstable so far. do not use it !!]
	//
	// Check if SoftIce kernel debugger is installed in the system, return 4 if OK, or Soft-ICE present.
	//
	// Return value - None
	//
	// Supported platform:		All Windows/Linux
	// Performance overhead:	Low
	// Security level:			Low
	// Usage scope:				Macro scope
	//
	// Note:\n
	// Not available for current Soft-ICE. Don't use it now.
	//*/
	//#define iviTR_INT3_EAX_CHECK(dword_variable) {dword_variable = TR_INT3_EAX_Check(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY((4 != dword_variable), iviTR_INT3_EAX_CHECK)}
	#define iviTR_INT3_EAX_CHECK(dword_variable) {dword_variable = 4;}

	///*!
	// Check if debugger is debugging current program, return true is exist.
	//
	// Return value - None
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		All Windows
	// Performance overhead:	Low
	// Security level:			High
	// Usage scope:				Macro scope
	//*/
	#define iviTR_DEBUGBREAK_CHECK(bRetValue) {\
		bRetValue = TR_Debugbreak_Check();\
		iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bRetValue, iviTR_DEBUGBREAK_CHECK) \
	}

	///*!
	// Check if SoftIce kernel debugger is installed in the system, return true is exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		All Windows
	// Performance overhead:	Low
	// Security level:			Low
	// Usage scope:				Macro scope
	//*/
	// Mark off as the code is moved to the header.
	//#define iviTR_SOFTWARE_NUMEGA_SOFTICE_REGISTRY_CHECK(bRetValue)\
	//{	bRetValue = TR_RegKeyCheck_SoftICE(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bRetValue, iviTR_SOFTWARE_NUMEGA_SOFTICE_REGISTRY_CHECK)}
	#define iviTR_SOFTWARE_NUMEGA_SOFTICE_REGISTRY_CHECK(bRetValue) \
	{ \
		char mystring[24]; \
		mystring[0] = 'S'; mystring[1] = 'o'; mystring[2] = 'f'; mystring[3] = 't'; \
		mystring[4] = 'w'; mystring[5] = 'a'; mystring[6] = 'r'; mystring[7] = 'e'; \
		mystring[8] = '\\'; mystring[9] = 'N'; mystring[10] = 'u'; mystring[11] = 'm'; \
		iviTR_DUMMY_CALL_4(63) \
		mystring[12] = 'e'; mystring[13] = 'g'; mystring[14] = 'a'; mystring[15] = '\\'; \
		mystring[16] = 'S'; mystring[17] = 'o'; mystring[18] = 'f'; mystring[19] = 't'; \
		iviTR_DUMMY_CALL_2(23) \
		mystring[20] = 'I'; mystring[21] = 'C'; mystring[22] = 'E'; mystring[23] = '\0'; \
		bRetValue = true; \
		HKEY hkey; \
		iviTR_DUMMY_CALL_8(49) \
		long temp = ::RegOpenKeyA(HKEY_LOCAL_MACHINE,mystring,&hkey); \
		if(temp!= ERROR_SUCCESS) \
		{ \
			bRetValue = false; \
		} \
		else \
		{ \
			bRetValue = true; \
			iviTR_DUMMY_CALL_2(82) \
			::RegCloseKey(hkey); \
		} \
		iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bRetValue, iviTR_SOFTWARE_NUMEGA_SOFTICE_REGISTRY_CHECK) \
	}

	///*!
	// Check if Syser kernel debugger is installed in the system, return true is exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		All Windows
	// Performance overhead:	Low
	// Security level:			Low
	// Usage scope:				Macro scope
	//*/
	// Mark off as the code is moved to the header.
	//#define iviTR_SOFTWARE_SYSERSOFT_SYSER_REGISTRY_CHECK1(bRetValue)\
	//{	bRetValue = TR_RegKeyCheck_Syser_1(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bRetValue, iviTR_SOFTWARE_SYSERSOFT_SYSER_REGISTRY_CHECK1)}
	#define iviTR_SOFTWARE_SYSERSOFT_SYSER_REGISTRY_CHECK1(bRetValue) \
	{ \
		char mystring[26]; \
		mystring[0] = 'S'; mystring[1] = 'o'; mystring[2] = 'f'; mystring[3] = 't'; \
		mystring[4] = 'w'; mystring[5] = 'a'; mystring[6] = 'r'; mystring[7] = 'e'; \
		iviTR_DUMMY_CALL_4(10) \
		mystring[8] = '\\'; mystring[9] = 'S'; mystring[10] = 'y'; mystring[11] = 's'; \
		mystring[12] = 'e'; mystring[13] = 'r'; mystring[14] = ' '; mystring[15] = 'S'; \
		mystring[16] = 'o'; mystring[17] = 'f'; mystring[18] = 't'; mystring[19] = '\\'; \
		mystring[20] = 'S'; mystring[21] = 'y'; mystring[22] = 's'; mystring[23] = 'e'; \
		mystring[24] = 'r'; mystring[25] = '\0'; \
		bRetValue = true; \
		iviTR_DUMMY_CALL_1(72) \
		HKEY hkey; \
		long temp = ::RegOpenKeyA(HKEY_CURRENT_USER,mystring,&hkey); \
		if(temp!= ERROR_SUCCESS) \
		{ \
			bRetValue = false; \
			iviTR_DUMMY_CALL_3(87) \
		} \
		else \
		{ \
			bRetValue = true; \
			iviTR_DUMMY_CALL_9(28) \
			::RegCloseKey(hkey); \
		} \
		iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bRetValue, iviTR_SOFTWARE_SYSERSOFT_SYSER_REGISTRY_CHECK1) \
	}

	///*!
	// Check if Syser kernel debugger is installed in the system, return true is exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		All Windows
	// Performance overhead:	Low
	// Security level:			Low
	// Usage scope:				Macro scope
	//*/
	// Mark off as the code is moved to the header.
	//#define iviTR_SOFTWARE_SYSERSOFT_SYSER_REGISTRY_CHECK2(bRetValue)\
	//{	bRetValue = TR_RegKeyCheck_Syser_2(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bRetValue, iviTR_SOFTWARE_SYSERSOFT_SYSER_REGISTRY_CHECK2)}
	#define iviTR_SOFTWARE_SYSERSOFT_SYSER_REGISTRY_CHECK2(bRetValue) \
	{ \
		char mystring[26]; \
		iviTR_DUMMY_CALL_8(52) \
		mystring[0] = 'S'; mystring[1] = 'o'; mystring[2] = 'f'; mystring[3] = 't'; \
		mystring[4] = 'w'; mystring[5] = 'a'; mystring[6] = 'r'; mystring[7] = 'e'; \
		iviTR_DUMMY_CALL_10(7) \
		mystring[8] = '\\'; mystring[9] = 'S'; mystring[10] = 'y'; mystring[11] = 's'; \
		mystring[12] = 'e'; mystring[13] = 'r'; mystring[14] = ' '; mystring[15] = 'S'; \
		mystring[16] = 'o'; mystring[17] = 'f'; mystring[18] = 't'; mystring[19] = '\\'; \
		mystring[20] = 'S'; mystring[21] = 'y'; mystring[22] = 's'; mystring[23] = 'e'; \
		iviTR_DUMMY_CALL_2(63) \
		mystring[24] = 'r'; mystring[25] = '\0'; \
		bRetValue = true; \
		HKEY hkey; \
		long temp = ::RegOpenKeyA(HKEY_LOCAL_MACHINE,mystring,&hkey); \
		if(temp!= ERROR_SUCCESS) \
		{ \
			iviTR_DUMMY_CALL_3(42) \
			bRetValue = false; \
		} \
		else \
		{ \
			bRetValue = true; \
			iviTR_DUMMY_CALL_2(74) \
			::RegCloseKey(hkey); \
		} \
		iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bRetValue, iviTR_SOFTWARE_SYSERSOFT_SYSER_REGISTRY_CHECK2) \
	}

	///*!
	// Check if Syser kernel debugger is installed in the system, return true is exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		All Windows
	// Performance overhead:	Low
	// Security level:			Low
	// Usage scope:				Macro scope
	//*/
	// Mark off as the code is moved to the header.
	//#define iviTR_SOFTWARE_SYSERSOFT_SYSER_REGISTRY_CHECK3(bRetValue)\
	//{	bRetValue = TR_RegKeyCheck_Syser_3(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bRetValue, iviTR_SOFTWARE_SYSERSOFT_SYSER_REGISTRY_CHECK3)}
	#define iviTR_SOFTWARE_SYSERSOFT_SYSER_REGISTRY_CHECK3(bRetValue) \
	{ \
		char mystring[35]; \
		mystring[0] = 'S'; mystring[1] = 'o'; mystring[2] = 'f'; mystring[3] = 't'; \
		iviTR_DUMMY_CALL_7(38) \
		mystring[4] = 'w'; mystring[5] = 'a'; mystring[6] = 'r'; mystring[7] = 'e'; \
		mystring[8] = '\\'; mystring[9] = 'S'; mystring[10] = 'y'; mystring[11] = 's'; \
		mystring[12] = 'e'; mystring[13] = 'r'; mystring[14] = ' '; mystring[15] = 'S'; \
		iviTR_DUMMY_CALL_2(57) \
		mystring[16] = 'o'; mystring[17] = 'f'; mystring[18] = 't'; mystring[19] = '\\'; \
		mystring[20] = 'S'; mystring[21] = 'y'; mystring[22] = 's'; mystring[23] = 'e'; \
		mystring[24] = 'r'; mystring[25] = '\\'; mystring[26] = 'I'; mystring[27] = 'n'; \
		mystring[28] = 's'; mystring[29] = 't'; mystring[30] = 'P'; mystring[31] = 'a'; \
		mystring[32] = 't'; mystring[33] = 'h'; mystring[34] = '\0'; \
		bRetValue = true; \
		HKEY hkey; \
		long temp = ::RegOpenKeyA(HKEY_LOCAL_MACHINE,mystring,&hkey); \
		iviTR_DUMMY_CALL_8(24) \
		if(temp!= ERROR_SUCCESS) \
		{ \
			bRetValue = false; \
		} \
		else \
		{ \
			iviTR_DUMMY_CALL_5(53) \
			bRetValue = true; \
			::RegCloseKey(hkey); \
		} \
		iviTR_DUMMY_CALL_1(31) \
		iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bRetValue, iviTR_SOFTWARE_SYSERSOFT_SYSER_REGISTRY_CHECK3) \
	}

	///*!
	// Check if Syser kernel debugger is installed in the system, return true is exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		All Windows
	// Performance overhead:	Low
	// Security level:			Low
	// Usage scope:				Macro scope
	//*/
	// Mark off as the code is moved to the header.
	//#define iviTR_SOFTWARE_SYSERSOFT_SYSER_REGISTRY_CHECK4(bRetValue)\
	//{	bRetValue = TR_RegKeyCheck_Syser_4(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bRetValue, iviTR_SOFTWARE_SYSERSOFT_SYSER_REGISTRY_CHECK4)}
	#define iviTR_SOFTWARE_SYSERSOFT_SYSER_REGISTRY_CHECK4(bRetValue) \
	{ \
		char mystring[20]; \
		mystring[0] = 'S'; mystring[1] = 'o'; mystring[2] = 'f'; mystring[3] = 't'; \
		iviTR_DUMMY_CALL_2(29) \
		mystring[4] = 'w'; mystring[5] = 'a'; mystring[6] = 'r'; mystring[7] = 'e'; \
		mystring[8] = '\\'; mystring[9] = 'S'; mystring[10] = 'y'; mystring[11] = 's'; \
		mystring[12] = 'e'; mystring[13] = 'r'; mystring[14] = ' '; mystring[15] = 'S'; \
		mystring[16] = 'o'; mystring[17] = 'f'; mystring[18] = 't'; mystring[19] = '\0'; \
		iviTR_DUMMY_CALL_9(53) \
		bRetValue = true; \
		HKEY hkey; \
		long temp = ::RegOpenKeyA(HKEY_LOCAL_MACHINE,mystring,&hkey); \
		iviTR_DUMMY_CALL_5(60) \
		if(temp!= ERROR_SUCCESS) \
		{ \
			bRetValue = false; \
		} \
		else \
		{ \
			bRetValue = true; \
			iviTR_DUMMY_CALL_1(21) \
			::RegCloseKey(hkey); \
		} \
		iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bRetValue, iviTR_SOFTWARE_SYSERSOFT_SYSER_REGISTRY_CHECK4) \
	}

	///*!
	// Check if Syser kernel debugger is installed in the system, return true is exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		All Windows
	// Performance overhead:	Low
	// Security level:			Low
	// Usage scope:				Macro scope
	//*/
	// Mark off as the code is moved to the header.
	//#define iviTR_SOFTWARE_SYSERSOFT_SYSER_REGISTRY_CHECK5(bRetValue)\
	//{	bRetValue = TR_RegKeyCheck_Syser_5(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bRetValue, iviTR_SOFTWARE_SYSERSOFT_SYSER_REGISTRY_CHECK5)}
	#define iviTR_SOFTWARE_SYSERSOFT_SYSER_REGISTRY_CHECK5(bRetValue) \
	{ \
		iviTR_DUMMY_CALL_6(42) \
		char mystring[20]; \
		mystring[0] = 'S'; mystring[1] = 'o'; mystring[2] = 'f'; mystring[3] = 't'; \
		mystring[4] = 'w'; mystring[5] = 'a'; mystring[6] = 'r'; mystring[7] = 'e'; \
		mystring[8] = '\\'; mystring[9] = 'S'; mystring[10] = 'y'; mystring[11] = 's'; \
		iviTR_DUMMY_CALL_3(18) \
		mystring[12] = 'e'; mystring[13] = 'r'; mystring[14] = ' '; mystring[15] = 'S'; \
		mystring[16] = 'o'; mystring[17] = 'f'; mystring[18] = 't'; mystring[19] = '\0'; \
		bRetValue = true; \
		HKEY hkey; \
		iviTR_DUMMY_CALL_7(45) \
		long temp = ::RegOpenKeyA(HKEY_CURRENT_USER,mystring,&hkey); \
		if(temp!= ERROR_SUCCESS) \
		{ \
			bRetValue = false; \
			iviTR_DUMMY_CALL_1(62) \
		} \
		else \
		{ \
			bRetValue = true; \
			::RegCloseKey(hkey); \
		} \
		iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bRetValue, iviTR_SOFTWARE_SYSERSOFT_SYSER_REGISTRY_CHECK5) \
	}

	///*!
	// Check if the system is running in debug mode, return true if it is.
	// Return value - None
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		All Windows
	// Performance overhead:	Low
	// Security level:			Low
	// Usage scope:				Macro scope
	//*/
	#define iviTR_SYSTEM_DEBUG_REGISTRY_CHECK(bRetValue) \
	{ \
		bRetValue = gtrSystemDebugRegistryCheck(); \
		iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bRetValue, iviTR_SYSTEM_DEBUG_REGISTRY_CHECK) \
	}

	///*!
	// Detect if Syser kernel debugger is running in the system. true if exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		Windows NT/2K
	// Performance overhead:	Low
	// Security level:			High
	// Usage scope:				Macro scope
	//*/
	#define iviTR_VERIFY_SYSER_ENGINE(bRetValue) \
	{ \
		char szDriverName[10]; \
		HANDLE hDevice; \
		szDriverName[0] = '\\'; szDriverName[1] = szDriverName[0]; szDriverName[3] = szDriverName[0]; \
		bRetValue = false; \
		szDriverName[2] = '.'; szDriverName[4] = 'S'; szDriverName[6] = szDriverName[4]; \
		iviTR_DUMMY_CALL_5(14) \
		szDriverName[5] = 'Y'; szDriverName[7] = 'E'; szDriverName[8] = 'R'; szDriverName[9] = '\0'; \
		if ((hDevice = CreateFileA(szDriverName, FILE_FLAG_WRITE_THROUGH, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0)) != INVALID_HANDLE_VALUE) \
		{ \
			CloseHandle(hDevice); \
			iviTR_DUMMY_CALL_9(33) \
			bRetValue = true; \
		} \
		iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bRetValue, iviTR_VERIFY_SYSER_ENGINE) \
	}

	///*!
	// Detect if Syser kernel debugger is running in the system. true if exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		Windows NT/2K
	// Performance overhead:	Low
	// Security level:			High
	// Usage scope:				Macro scope
	//*/
	#define iviTR_VERIFY_SYSER_SYSERBOOT(bRetValue) \
	{ \
		char szDriverName[14]; \
		HANDLE hDevice; \
		szDriverName[0] = '\\'; szDriverName[1] = szDriverName[0]; szDriverName[7] = 'E'; \
		szDriverName[2] = '.'; szDriverName[4] = 'S'; szDriverName[12] = 'T'; szDriverName[10] = 'O'; \
		iviTR_DUMMY_CALL_8(43) \
		szDriverName[6] = szDriverName[4]; szDriverName[5] = 'Y'; szDriverName[13] = '\0'; \
		szDriverName[9] = 'B'; szDriverName[3] = szDriverName[0]; szDriverName[11] = szDriverName[10]; \
		bRetValue = false; \
		szDriverName[8] = 'R'; \
		if ((hDevice = CreateFileA(szDriverName, FILE_FLAG_WRITE_THROUGH, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0)) != INVALID_HANDLE_VALUE) \
		{ \
			bRetValue = true; \
			iviTR_DUMMY_CALL_2(11) \
			CloseHandle(hDevice); \
		} \
		iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bRetValue, iviTR_VERIFY_SYSER_SYSERBOOT) \
	}

	///*!
	// Detect if the program is being debugged by user-mode debugger. true if exist.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		Windows NT/2K
	// Performance overhead:	Low
	// Security level:			High
	// Usage scope:				Macro scope
	// 
	// Note:\n
	//*/
	#define iviTR_DETECT_PRINTEXCEPTION(bRetValue) \
	{ \
		bRetValue = gtrDetectPrintException(); \
		iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bRetValue, iviTR_DETECT_PRINTEXCEPTION) \
	}

///*!
// Detect if debugger is debugging current program, true is debugger exist.
//
// Return value - None
//
// -#bRetValue - [IN] can not be NULL, return value in bool.
//
// Supported platform:		Windows NT/2K
// Performance overhead:	Low
// Security level:			High
// Usage scope:				Macro scope
// 
// Note:\n
//*/
#define iviTR_INT2D_CHECK(bRetValue)	\
{	\
	bRetValue = gtrINT2DCheck();	\
	iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bRetValue, iviTR_INT2D_CHECK)	\
}

	///*!
	// Check if SoftICE's extension IceExt is installed in the system, return true if it is.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		All Windows
	// Performance overhead:	Low
	// Security level:			Medium
	// Usage scope:				Macro scope
	//*/
	#define iviTR_SOFTICE_ICEEXT_REGISTRY_CHECK(bRetValue) \
	{ \
		char mystring[40]; \
		iviTR_DUMMY_CALL_5(16) \
		mystring[8] = 'u'; mystring[0] = 'S'; mystring[9] = 'r'; mystring[34] = 'N'; \
		HKEY hkey; \
		mystring[4] = 'E'; mystring[5] = 'M'; mystring[6] = '\\'; mystring[7] = 'C'; \
		mystring[18] = 'r'; mystring[10] = 'r'; mystring[28] = 'v'; mystring[17] = 't'; \
		iviTR_DUMMY_CALL_3(21) \
		mystring[30] = 'c'; mystring[12] = 'n'; mystring[37] = 'c'; mystring[20] = 'l'; \
		mystring[39] = '\0'; mystring[16] = 'n'; mystring[32] = 's'; mystring[19] = 'o'; \
		mystring[21] = 'S'; mystring[13] = 't'; mystring[22] = 'e'; mystring[1] = 'Y'; \
		bRetValue = false; \
		mystring[24] = '\\'; mystring[38] = 'e'; mystring[11] = 'e'; mystring[3] = 'T'; \
		mystring[23] = 't'; mystring[29] = 'i'; mystring[33] = '\\'; mystring[25] = 'S'; \
		mystring[27] = 'r';mystring[2] = 'S'; mystring[15] = 'o'; mystring[31] = 'e'; \
		iviTR_DUMMY_CALL_3(14) \
		mystring[26] = 'e';  mystring[36] = 'i'; mystring[14] = 'C';mystring[35] = 'T'; \
		long temp = ::RegOpenKeyA(HKEY_LOCAL_MACHINE, mystring, &hkey); \
		iviTR_DUMMY_CALL_1(12) \
		if (temp == ERROR_SUCCESS) \
		{ \
			bRetValue = true; \
			::RegCloseKey(hkey); \
			iviTR_DUMMY_CALL_8(11) \
		} \
		iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bRetValue, iviTR_SOFTICE_ICEEXT_REGISTRY_CHECK) \
	}

	///*!
	// Check if SoftICE's extension IceExt is installed in the system, return false if it is.
	//
	// Return value - None
	//
	// -#bRetValue - [IN] can not be NULL, return value in bool.
	//
	// Supported platform:		All Windows
	// Performance overhead:	Low
	// Security level:			Medium
	// Usage scope:				Macro scope
	//*/
	#define iviTR_SOFTICE_ICEEXT_REGISTRY_CHECK_INV(bRetValue) \
	{ \
		char mystring[40]; \
		mystring[37] = 'c'; mystring[20] = 'l'; mystring[9] = 'r'; mystring[34] = 'N'; \
		iviTR_DUMMY_CALL_1(86) \
		mystring[4] = 'E'; mystring[5] = 'M'; mystring[6] = '\\'; mystring[7] = 'C'; \
		HKEY hkey; \
		iviTR_DUMMY_CALL_10(18) \
		mystring[8] = 'u'; mystring[10] = 'r'; mystring[15] = 'o';mystring[28] = 'v'; \
		mystring[30] = 'c'; mystring[12] = 'n'; mystring[23] = 't'; mystring[38] = 'e'; \
		mystring[39] = '\0'; mystring[16] = 'n'; mystring[32] = 's'; mystring[19] = 'o'; \
		mystring[21] = 'S'; mystring[22] = 'e'; mystring[0] = 'S'; mystring[13] = 't'; \
		iviTR_DUMMY_CALL_7(33) \
		mystring[24] = '\\'; mystring[11] = 'e'; mystring[26] = 'e';  mystring[3] = 'T'; \
		mystring[29] = 'i'; mystring[31] = 'e'; mystring[25] = 'S'; mystring[1] = 'Y'; \
		mystring[33] = '\\'; mystring[27] = 'r';mystring[35] = 'T'; mystring[2] = 'S'; \
		mystring[36] = 'i'; mystring[18] = 'r'; mystring[14] = 'C';mystring[17] = 't'; \
		long temp = ::RegOpenKeyA(HKEY_LOCAL_MACHINE, mystring, &hkey); \
		iviTR_DUMMY_CALL_3(31) \
		if (temp!= ERROR_SUCCESS) \
		{ \
			bRetValue = true; \
		} \
		else \
		{ \
			iviTR_DUMMY_CALL_9(11) \
			bRetValue = false; \
			::RegCloseKey(hkey); \
		} \
		iviTR_DUMP_ERROR_TR_MACRO_ENTRY(!bRetValue, iviTR_SOFTICE_ICEEXT_REGISTRY_CHECK_INV) \
	}

#elif defined(__linux__)
	#define iviTR_VERIFY_SOFTICE_SIWVIDSTART_A(bool_variable) {bool_variable = false;}
	#define iviTR_VERIFY_SOFTICE_SIWVIDSTART_DD_A(bool_variable) {bool_variable = false;}
	#define iviTR_VERIFY_SOFTICE_SICE_A(bool_variable) {bool_variable = false;}
	#define iviTR_VERIFY_SOFTICE_SICE_DD_A(bool_variable) {bool_variable = false;}
	#define iviTR_VERIFY_SOFTICE_NTICE_A(bool_variable) {bool_variable = false;}
	#define iviTR_VERIFY_SOFTICE_NTICE_DD_A(bool_variable) {bool_variable = false;}
	#define iviTR_VERIFY_SOFTICE_SIWVID_A(bool_variable) {bool_variable = false;}
	#define iviTR_VERIFY_SOFTICE_SIWVID_DD_A(bool_variable) {bool_variable = false;}
	#define iviTR_VERIFY_SOFTICE_SIWDEBUG_A(bool_variable) {bool_variable = false;}
	#define iviTR_VERIFY_SOFTICE_SIWDEBUG_DD_A(bool_variable) {bool_variable = false;}

	#define iviTR_VERIFY_SOFTICE_SIWVIDSTART_A_INV(bool_variable) {bool_variable = true;}
	#define iviTR_VERIFY_SOFTICE_SIWVIDSTART_DD_A_INV(bool_variable) {bool_variable = true;}
	#define iviTR_VERIFY_SOFTICE_SICE_A_INV(bool_variable) {bool_variable = true;}
	#define iviTR_VERIFY_SOFTICE_SICE_DD_A_INV(bool_variable) {bool_variable = true;}
	#define iviTR_VERIFY_SOFTICE_NTICE_A_INV(bool_variable) {bool_variable = true;}
	#define iviTR_VERIFY_SOFTICE_NTICE_DD_A_INV(bool_variable) {bool_variable = true;}
	#define iviTR_VERIFY_SOFTICE_SIWVID_A_INV(bool_variable) {bool_variable = true;}
	#define iviTR_VERIFY_SOFTICE_SIWVID_DD_A_INV(bool_variable) {bool_variable = true;}
	#define iviTR_VERIFY_SOFTICE_SIWDEBUG_A_INV(bool_variable) {bool_variable = true;}
	#define iviTR_VERIFY_SOFTICE_SIWDEBUG_DD_A_INV(bool_variable) {bool_variable = true;}

	#define iviTR_VERIFY_SOFTICE_SIWVIDSTART_W(bool_variable) {bool_variable = false;}
	#define iviTR_VERIFY_SOFTICE_SIWVIDSTART_DD_W(bool_variable) {bool_variable = false;}
	#define iviTR_VERIFY_SOFTICE_SICE_W(bool_variable) {bool_variable = false;}
	#define iviTR_VERIFY_SOFTICE_SICE_DD_W(bool_variable) {bool_variable = false;}
	#define iviTR_VERIFY_SOFTICE_NTICE_W(bool_variable) {bool_variable = false;}
	#define iviTR_VERIFY_SOFTICE_NTICE_DD_W(bool_variable) {bool_variable = false;}
	#define iviTR_VERIFY_SOFTICE_SIWVID_W(bool_variable) {bool_variable = false;}
	#define iviTR_VERIFY_SOFTICE_SIWVID_DD_W(bool_variable) {bool_variable = false;}
	#define iviTR_VERIFY_SOFTICE_SIWDEBUG_W(bool_variable) {bool_variable = false;}
	#define iviTR_VERIFY_SOFTICE_SIWDEBUG_DD_W(bool_variable) {bool_variable = false;}

	#define iviTR_VERIFY_SOFTICE_SIWVIDSTART_W_INV(bool_variable) {bool_variable = true;}
	#define iviTR_VERIFY_SOFTICE_SIWVIDSTART_DD_W_INV(bool_variable) {bool_variable = true;}
	#define iviTR_VERIFY_SOFTICE_SICE_W_INV(bool_variable) {bool_variable = true;}
	#define iviTR_VERIFY_SOFTICE_SICE_DD_W_INV(bool_variable) {bool_variable = true;}
	#define iviTR_VERIFY_SOFTICE_NTICE_W_INV(bool_variable) {bool_variable = true;}
	#define iviTR_VERIFY_SOFTICE_NTICE_DD_W_INV(bool_variable) {bool_variable = true;}
	#define iviTR_VERIFY_SOFTICE_SIWVID_W_INV(bool_variable) {bool_variable = true;}
	#define iviTR_VERIFY_SOFTICE_SIWVID_DD_W_INV(bool_variable) {bool_variable = true;}
	#define iviTR_VERIFY_SOFTICE_SIWDEBUG_W_INV(bool_variable) {bool_variable = true;}
	#define iviTR_VERIFY_SOFTICE_SIWDEBUG_DD_W_INV(bool_variable) {bool_variable = true;}

	#define iviTR_VERIFY_SOFTICE_LCREAT(bool_variable) {bool_variable = false;}
	#define iviTR_VERIFY_SOFTICE_LCREAT1(bool_variable) {bool_variable = false;}
	#define iviTR_VERIFY_SOFTICE_LCREAT2(bool_variable) {bool_variable = false;}
	#define iviTR_VERIFY_SOFTICE_LCREAT3(bool_variable) {bool_variable = false;}
	#define iviTR_VERIFY_SOFTICE_LCREAT4(bool_variable) {bool_variable = false;}

	#define iviTR_VERIFY_SOFTICE_LCREAT_INV(bool_variable) {bool_variable = true;}
	#define iviTR_VERIFY_SOFTICE_LCREAT1_INV(bool_variable) {bool_variable = true;}
	#define iviTR_VERIFY_SOFTICE_LCREAT2_INV(bool_variable) {bool_variable = true;}
	#define iviTR_VERIFY_SOFTICE_LCREAT3_INV(bool_variable) {bool_variable = true;}
	#define iviTR_VERIFY_SOFTICE_LCREAT4_INV(bool_variable) {bool_variable = true;}

	#define iviTR_VERIFY_SOFTICE_LOPEN(bool_variable) {bool_variable = false;}
	#define iviTR_VERIFY_SOFTICE_LOPEN1(bool_variable) {bool_variable = false;}
	#define iviTR_VERIFY_SOFTICE_LOPEN2(bool_variable) {bool_variable = false;}
	#define iviTR_VERIFY_SOFTICE_LOPEN3(bool_variable) {bool_variable = false;}
	#define iviTR_VERIFY_SOFTICE_LOPEN4(bool_variable) {bool_variable = false;}

	#define iviTR_VERIFY_SOFTICE_LOPEN_INV(bool_variable) {bool_variable = true;}
	#define iviTR_VERIFY_SOFTICE_LOPEN1_INV(bool_variable) {bool_variable = true;}
	#define iviTR_VERIFY_SOFTICE_LOPEN2_INV(bool_variable) {bool_variable = true;}
	#define iviTR_VERIFY_SOFTICE_LOPEN3_INV(bool_variable) {bool_variable = true;}
	#define iviTR_VERIFY_SOFTICE_LOPEN4_INV(bool_variable) {bool_variable = true;}

	#define iviTR_VERIFY_SOFTICE_NTICE_SERVICE(bool_variable) {bool_variable = false;}
	#define iviTR_VERIFY_SOFTICE_NTICE_SERVICE_INV(bool_variable) {bool_variable = true;}

	#define iviTR_VERIFY_SYSER_A(bool_variable) {bool_variable = false;}
	#define iviTR_VERIFY_SYSER_DD_A(bool_variable) {bool_variable = false;}
	#define iviTR_VERIFY_SYSER_W(bool_variable) {bool_variable = false;}
	#define iviTR_VERIFY_SYSER_DD_W(bool_variable) {bool_variable = false;}
	#define iviTR_VERIFY_SYSER_LCREAT(bool_variable) {bool_variable = false;}
	#define iviTR_VERIFY_SYSER_LOPEN(bool_variable) {bool_variable = false;}

	// static void sigsegvhandler1(int sig) { siglongjmp(sigsegvbuffer1, sig); }

	#define iviTR_FIND_INT_3H_UNHANDLEDEXCEPTIONFILTER(dword_variable)\
	{	dword_variable = TR_Int3Check(); }


	#define iviTR_DETECT_TRACER_TRAP_FLAG(dword_variable)\
	{	dword_variable = (TR_Detect_Tracer_Trap_Flag() ? 1 : 0); }

	#define iviTR_DAEMON_FINDER(bool_variable)\
	{	bool_variable = false; }

	#define iviTR_INT1H_CHECK(bool_variable) {bool_variable = TR_DetectInt1h();}
	#define iviTR_ICEBP_CHECK(bRetValue) {bool_variable = false;}

	#define iviTR_API_ISDEBUGGERPRESENT_CHECK(bool_variable) {bool_variable = TR_IsDebuggerPresent();}

	#define iviTR_LOCK_CMPXCHG8B(){}

	#define iviTR_INT3_EAX_CHECK(dword_variable) {dword_variable = TR_Int3Check2();}
	#define iviTR_DEBUGBREAK_CHECK(bool_variable) {bool_variable = TR_Int3Check3();}

	#define iviTR_SOFTWARE_NUMEGA_SOFTICE_REGISTRY_CHECK(bool_variable) {bool_variable = false;}
	#define iviTR_SOFTWARE_SYSERSOFT_SYSER_REGISTRY_CHECK1(bool_variable) {bool_variable = false;}
	#define iviTR_SOFTWARE_SYSERSOFT_SYSER_REGISTRY_CHECK2(bool_variable) {bool_variable = false;}
	#define iviTR_SOFTWARE_SYSERSOFT_SYSER_REGISTRY_CHECK3(bool_variable) {bool_variable = false;}
	#define iviTR_SOFTWARE_SYSERSOFT_SYSER_REGISTRY_CHECK4(bool_variable) {bool_variable = false;}
	#define iviTR_SOFTWARE_SYSERSOFT_SYSER_REGISTRY_CHECK5(bool_variable) {bool_variable = false;}
	//need to be implement!!
	#define iviTR_DEBUG_REGISTER_CHECK(bool_variable) {bool_variable = false;}
	#define iviTR_DEBUG_REGISTER_CHECK2(bool_variable) {bool_variable = false;}
	#define iviTR_DEBUG_REGISTER_CLEAR(bool_variable) {bool_variable = false;}
	#define iviTR_SYSTEM_DEBUG_REGISTRY_CHECK(bool_variable) {bool_variable = false;}
	#define iviTR_VERIFY_SYSER_ENGINE(bool_variable) {bool_variable = false;}
	#define iviTR_VERIFY_SYSER_SYSERBOOT(bool_variable) {bool_variable = false;}
	#define iviTR_DETECT_PRINTEXCEPTION(bool_variable) {bool_variable = false;}
	#define iviTR_INT2D_CHECK(bRetValue) {bRetValue = false;}
	#define iviTR_SOFTICE_ICEEXT_REGISTRY_CHECK(bool_variable) {bool_variable = false;}
	#define iviTR_SOFTICE_ICEEXT_REGISTRY_CHECK_INV(bool_variable) {bool_variable = true;}

#endif // _WIN32

#endif // _IVI_ANTIDEBUG2_H
