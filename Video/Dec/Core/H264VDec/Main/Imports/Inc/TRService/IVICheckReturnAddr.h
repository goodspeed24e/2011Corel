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
#ifndef _IVI_CHECKRETURNADDR_H
#define _IVI_CHECKRETURNADDR_H

#include "IVITRComm.h"

#ifdef _WIN32
	#include "CtrRetAddrObject.h"
	#include "CtrHandshakObject.h"

	///*!
	// [Internal function. Do not call it.]
	//
	// Check if function return address is within the specified region.
	//
	// Return value - [bool] true if within, false is not within.\n
	// Supported platform: All Windows\n
	//*/
	bool __cdecl gtrCheck_Return_Addr(DWORD dwFunctionBegin, DWORD dwFunctionEnd);

	///*!
	// [Internal function. Do not call it.]
	//
	// call this to initialize detection with a name.\n
	//
	//-#szName - [IN] a specific name to identify the caller\n
	//
	// Return value - [bool] not use\n
	// Supported platform: All Windows\n
	//*/
	bool __cdecl trInitDetectCallerValidation(char *szName, CtrHandshakObject *pObject);

	///*!
	// [Internal function. Do not call it.]
	//
	// detect if com module has been hooked with a name.\n
	//
	//-#szName - [IN] a specific name to identify the caller\n
	//
	// Return value - [bool] true is the hook action exists, or not.\n
	// Supported platform: All Windows\n
	//*/
	bool __cdecl trDetectCallerValidation(char *szName);

	//*!
	// [Internal macro. Do not call it.]
	//*/
	#define iviTR_RACHECK_BEGIN_NAME(FNUM) TR_RARegion_Begin_##FNUM

	//*!
	// [Internal macro. Do not call it.]
	//*/
	#define iviTR_RACHECK_END_NAME(FNUM) TR_RARegion_End_##FNUM


	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	///*!
	// Save function return address.
	//
	// Return value - None
	//
	// Supported platform:		All Windows
	// Performance overhead:	Medium
	// Security level:			Medium
	// Usage scope:				Function scope
	//
	// Note:\n
	// iviTR_SAVE_RETURN_ADDR must be placed at the beginning of a function.  
	// Use this with iviTR_RACHECK_BEGIN(FNUM), iviTR_RACHECK_END(FNUM) and
	// iviTR_CHECK_RETURN_ADDR(FNUM) all together to perform return address
	// checking.  iviTR_SAVE_RETURN_ADDR should be placed in the function 
	// (callee) that will be called from a caller function (caller) where 
	// iviTR_CHECK_RETURN_ADDR(FNUM) is in.  iviTR_RACHECK_BEGIN(FNUM) needs 
	// to be placed before such caller function and iviTR_RACHECK_END(FNUM) is 
	// placed after the function.  Every return address checking set is 
	// identified by FNUM and each set's FNUM should be unique within DLL scope.
	// 
	// If the DLL that contains the callee function is being hooked by attackers,
	// iviTR_CHECK_RETURN_ADDR(FNUM) will return false.
	//
	// To insure the macros work properly, please turn off global optimization
	// using #pragma optimize("g", off) before the functions that include the
	// macros.  Use #pragma optimize("", on) to restore the original settings.
	// In addition, SQPlus.dll has to be present under the calling directories.
	//
	// Usage:\n
	//
	// In callee function:
	//=============================================================================
	//	#pragma optimize("g", off)
	//	void calleefunction()
	//	{
	//		iviTR_SAVE_RETURN_ADDR
	//       ...
	//	}
	//	#pragma optimize("", on)
	//
	// In caller function's .cpp:
	//=============================================================================
	//	...
	//	#pragma optimize("g", off)
	//	iviTR_RACHECK_BEGIN(1)
	//	#pragma optimize("", on)
	//	...
	//	#pragma optimize("g", off)
	//	void callerfunction()
	//	{
	//		...
	//		calleefunction();
	//		...
	//		bool bRetValue = true;
	//		iviTR_CHECK_RETURN_ADDR(1, bRetValue)
	//		if (!bRetValue)
	//			{ cause instability }
	//		...
	//	}
	//	#pragma optimize("", on)
	//	...
	//	#pragma optimize("g", off)
	//	iviTR_RACHECK_END(1)
	//	#pragma optimize("", on)
	//	...
	//*/
	#define iviTR_SAVE_RETURN_ADDR				\
		DWORD __dwRetAddr__;					\
		__asm {									\
			__asm push eax						\
			__asm mov  eax, dword ptr [ebp+4]	\
			__asm mov  __dwRetAddr__, eax		\
			__asm pop  eax						\
		}										\
		CtrRetAddrObject RetAddrObject(__dwRetAddr__);


	///*!
	// Specify the beginning of return address checking region.
	//
	// Return value - None
	//
	// Supported platform:		All Windows
	// Performance overhead:	Medium
	// Security level:			Medium
	// Usage scope:				Function scope
	//
	// Note:\n
	// Use this with iviTR_SAVE_RETURN_ADDR, iviTR_RACHECK_BEGIN(FNUM)
	// iviTR_RACHECK_END(FNUM) and iviTR_CHECK_RETURN_ADDR(FNUM) all together to
	// perform return address checking.  iviTR_SAVE_RETURN_ADDR should be placed 
	// in the function (callee) that will be called from a caller function 
	// (caller) where iviTR_CHECK_RETURN_ADDR(FNUM) is in.  
	// iviTR_RACHECK_BEGIN(FNUM) needs to be placed before such caller function 
	// and iviTR_RACHECK_END(FNUM) is placed after the function.  Every return 
	// address checking set is identified by FNUM and each set's FNUM should be 
	// unique within DLL scope.
	// 
	// If the DLL that contains the callee function is being hooked by attackers,
	// iviTR_CHECK_RETURN_ADDR(FNUM) will return false.
	//
	// To insure the macros work properly, please turn off global optimization
	// using #pragma optimize("g", off) before the functions that include the
	// macros.  Use #pragma optimize("", on) to restore the original settings.
	// In addition, SQPlus.dll has to be present under the calling directories.
	//
	// Usage:\n
	//
	// In callee function:
	//=============================================================================
	//	#pragma optimize("g", off)
	//	void calleefunction()
	//	{
	//		iviTR_SAVE_RETURN_ADDR
	//       ...
	//	}
	//	#pragma optimize("", on)
	//
	// In caller function's .cpp:
	//=============================================================================
	//	...
	//	#pragma optimize("g", off)
	//	iviTR_RACHECK_BEGIN(1)
	//	#pragma optimize("", on)
	//	...
	//	#pragma optimize("g", off)
	//	void callerfunction()
	//	{
	//		...
	//		calleefunction();
	//		...
	//		bool bRetValue = true;
	//		iviTR_CHECK_RETURN_ADDR(1, bRetValue)
	//		if (!bRetValue)
	//			{ cause instability }
	//		...
	//	}
	//	#pragma optimize("", on)
	//	...
	//	#pragma optimize("g", off)
	//	iviTR_RACHECK_END(1)
	//	#pragma optimize("", on)
	//	...
	//*/
	#define iviTR_RACHECK_BEGIN(FNUM) \
		void iviTR_RACHECK_BEGIN_NAME(FNUM)()\
	{\
		char* cNum = ""#FNUM; \
		int n = atoi(cNum);\
		for (int i = 0; i < 10; i++) n += 1;\
	}\
		void iviTR_RACHECK_END_NAME(FNUM)();

	///*!
	// Specify the end of return address checking region.
	//
	// Return value - None
	//
	// Supported platform:		All Windows
	// Performance overhead:	Medium
	// Security level:			Medium
	// Usage scope:				Function scope
	//
	// Note:\n
	// Use this with iviTR_SAVE_RETURN_ADDR, iviTR_RACHECK_END(FNUM)
	// iviTR_RACHECK_END(FNUM) and iviTR_CHECK_RETURN_ADDR(FNUM) all together to
	// perform return address checking.  iviTR_SAVE_RETURN_ADDR should be placed
	// in the function (callee) that will be called from a caller function 
	// (caller) where iviTR_CHECK_RETURN_ADDR(FNUM) is in.
	// iviTR_RACHECK_BEGIN(FNUM) needs to be placed before such caller function 
	// and iviTR_RACHECK_END(FNUM) is placed after the function.  Every return 
	// address checking set is identified by FNUM and each set's FNUM should be 
	// unique within DLL scope.
	// 
	// If the DLL that contains the callee function is being hooked by attackers,
	// iviTR_CHECK_RETURN_ADDR(FNUM) will return false.
	//
	// To insure the macros work properly, please turn off global optimization
	// using #pragma optimize("g", off) before the functions that include the
	// macros.  Use #pragma optimize("", on) to restore the original settings.
	// In addition, SQPlus.dll has to be present under the calling directories.
	//
	// Usage:\n
	//
	// In callee function:
	//=============================================================================
	//	#pragma optimize("g", off)
	//	void calleefunction()
	//	{
	//		iviTR_SAVE_RETURN_ADDR
	//       ...
	//	}
	//	#pragma optimize("", on)
	//
	// In caller function's .cpp:
	//=============================================================================
	//	...
	//	#pragma optimize("g", off)
	//	iviTR_RACHECK_BEGIN(1)
	//	#pragma optimize("", on)
	//	...
	//	#pragma optimize("g", off)
	//	void callerfunction()
	//	{
	//		...
	//		calleefunction();
	//		...
	//		bool bRetValue = true;
	//		iviTR_CHECK_RETURN_ADDR(1, bRetValue)
	//		if (!bRetValue)
	//			{ cause instability }
	//		...
	//	}
	//	#pragma optimize("", on)
	//	...
	//	#pragma optimize("g", off)
	//	iviTR_RACHECK_END(1)
	//	#pragma optimize("", on)
	//	...
	//*/
	#define iviTR_RACHECK_END(FNUM) \
		void iviTR_RACHECK_END_NAME(FNUM)()\
	{\
		char* cNum = ""#FNUM; \
		int m = atoi(cNum);\
		for (int j = 0; j < 5; j++) m += 1;\
	}

	///*!
	// Check if function return address is within the specified region.
	//
	// Return value - bool
	//
	// -#bRetValue - [OUT] return value in bool.
	//
	// Supported platform:		All Windows
	// Performance overhead:	Medium
	// Security level:			Medium
	// Usage scope:				Macro scope
	//
	// Note:\n
	// iviTR_CHECK_RETURN_ADDR must be placed in the same function scope where
	// the function (callee) that contains iviTR_SAVE_RETURN_ADDR is called.
	// Use this with iviTR_SAVE_RETURN_ADDR(FNUM), iviTR_RACHECK_BEGIN(FNUM)
	// and iviTR_RACHECK_END(FNUM) and all together to perform return address
	// checking.  iviTR_SAVE_RETURN_ADDR should be placed in the function 
	// (callee) that will be called from a caller function (caller) where 
	// iviTR_CHECK_RETURN_ADDR(FNUM) is in.  iviTR_RACHECK_BEGIN(FNUM) needs to 
	// be placed before such caller function and iviTR_RACHECK_END(FNUM) is 
	// placed after the function.  Every return address checking set is 
	// identified by FNUM and each set's FNUM should be unique within DLL scope.
	// 
	// If the DLL that contains the callee function is being hooked by attackers,
	// iviTR_CHECK_RETURN_ADDR(FNUM) will return false.
	//
	// To insure the macros work properly, please turn off global optimization
	// using #pragma optimize("g", off) before the functions that include the
	// macros.  Use #pragma optimize("", on) to restore the original settings.
	// In addition, SQPlus.dll has to be present under the calling directories.
	//
	// Usage:\n
	//
	// In callee function:
	//=============================================================================
	//	#pragma optimize("g", off)
	//	void calleefunction()
	//	{
	//		iviTR_SAVE_RETURN_ADDR
	//       ...
	//	}
	//	#pragma optimize("", on)
	//
	// In caller function's .cpp:
	//=============================================================================
	//	...
	//	#pragma optimize("g", off)
	//	iviTR_RACHECK_BEGIN(1)
	//	#pragma optimize("", on)
	//	...
	//	#pragma optimize("g", off)
	//	void callerfunction()
	//	{
	//		...
	//		calleefunction();
	//		...
	//		bool bRetValue = true;
	//		iviTR_CHECK_RETURN_ADDR(1, bRetValue)
	//		if (!bRetValue)
	//			{ cause instability }
	//		...
	//	}
	//	#pragma optimize("", on)
	//	...
	//	#pragma optimize("g", off)
	//	iviTR_RACHECK_END(1)
	//	#pragma optimize("", on)
	//	...
	//*/
    #define iviTR_CHECK_RETURN_ADDR(FNUM, bRetValue) \
    { \
        bRetValue = gtrCheck_Return_Addr((DWORD)iviTR_RACHECK_BEGIN_NAME(FNUM), (DWORD)iviTR_RACHECK_END_NAME(FNUM)); \
        iviTR_DUMP_ERROR_TR_MACRO_ENTRY(!bRetValue, iviTR_CHECK_RETURN_ADDR) \
    }


	///*!
	// initilize hand-shaking detection, the caller calls this to register itself is a valid caller.\n
	//-#sz_Name - [IN] a unique token to identify\n
	//
	// Return value - None\n
	//
	// Supported platform: All Windows\n
	// Note:\n
	//*/
	#define iviTR_INIT_DETECT_CALLER_VALIDATION(sz_Name, int_index) \
	CtrHandshakObject obHandshak##int_index; \
	trInitDetectCallerValidation(sz_Name, &obHandshak##int_index);

	///*!
	// hand-shaking detection, the callee calls this to detect if there is any valid caller exists.\n
	//-#bool_return - [IN] can not be NULL, return check result. true if caller is invalid.
	//-#sz_Name - [IN] a unique token to identify, it must be same as the parameter of iviTR_INIT_DETECT_CALLER_VALIDATION\n
	//
	// Return value - All Windows\n
	//
	// Supported platform: None\n
	// Note:\n
	//*/
	#define iviTR_DETECT_CALLER_VALIDATION(bool_return, sz_Name) \
	{\
		bool_return = trDetectCallerValidation(sz_Name);\
        iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bool_return, iviTR_DETECT_CALLER_VALIDATION) \
	}

#elif defined(__linux__)
	#define iviTR_SAVE_RETURN_ADDR
	#define iviTR_RACHECK_BEGIN(FNUM)
	#define iviTR_RACHECK_END(FNUM)
	#define iviTR_CHECK_RETURN_ADDR(FNUM, bool_variable) {bool_variable = true;}
	#define iviTR_INIT_DETECT_CALLER_VALIDATION(sz_Name, int_index)
	#define iviTR_DETECT_CALLER_VALIDATION(bool_return, sz_Name) {bool_return = false;}
#endif	// _WIN32

#endif	//_IVI_CHECKRETURNADDR_H
