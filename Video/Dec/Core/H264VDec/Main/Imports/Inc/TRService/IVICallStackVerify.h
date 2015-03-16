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
#ifndef _IVI_CALLSTACKVERIFY_H
#define _IVI_CALLSTACKVERIFY_H

/* TODO 
 * 1. hide function pointers
 */

#include <windows.h>
#include "IVITRComm.h"
#include "IVICheckReturnAddr.h"

/*****************************************************************************
	Call Stack Verification
	------------------------

	Description:
		These macros check the frame pointer and return address of a function stack.
		The return addresses are used to trace back to the previous function callee stack.
		We can verify if the callee is geniune in this manner.

	Cautions:
		1.	must disable Incremental Linking
		2.  When using both macros below, you must enclose your callee function with:
				#pragma optimize("y", off)
					... your function ...
				#pragma optimize("", on)

	Main Macro(s):
			1. iviTRVerifyCallerWithFramePointer
			2. iviTRVerifyCallerWithParam
		
******************************************************************************/

#define MAX_FUNCTION_SIZE	(1<<13)	//in code bytes

#ifdef _WIN32

	///*!
	// [Internal function. Do not call it.]
	//
	// To get the return address by frame pointer, it is available if there is no /Oy flag.\n
	//
	// Return value - [LPBYTE] the return address.\n
	// Supported platform: All Windows\n
	//*/
  LPBYTE GetReturnAddr(LPDWORD pdwFramePointer);
  

	///*!
	// [Internal function. Do not call it.]
	//
	// To get the previous level frame pointer by the current frame pointer, it is available if there is no /Oy flag.\n
	//
	// Return value - [LPDWORD] the previous frame pointer.\n
	// Supported platform: All Windows\n
	//*/
	LPDWORD GetPreviousFramePointer(LPDWORD pdwFramePointer);
	

	///*!
	// [Internal function. Do not call it.]
	//
	// To determine if it is the start point of function, it is available if there is no /Oy flag.\n
	//
	// Return value - [bool] return true if it is the start point, else false.\n
	// Supported platform: All Windows\n
	//*/
	bool isFunctionAddressStart(LPBYTE pbRetAddr);
	

	///*!
	// [Internal function. Do not call it.]
	//
	// To determine if the return address is legit.\n
	//
	// Return value - [bool] return true if it is legit, else false.\n
	// Supported platform: All Windows\n
	//*/
	bool isReturnAddrLegit(LPBYTE pbRetAddr, LPBYTE funcAddr);

	///*!
	// [Internal function. Do not call it.]
	//
	// To determine if the caller is legal, it is available if there is no /Oy flag.\n
	//
	//-# pbRetAddr - [IN] the return address within the caller function\n
	//-# funcAddr - [IN] the array of function pointers to be checked.\n
	//-# numFuncAddrs - [IN] number of function addrs to check\n
	//-# max_size - [IN] the max size of the caller function.\n
	// Return value - [bool] return true if one of callers within the array, funcAddr, is legal, else false.\n
	// Supported platform: All Windows\n
	//*/
	bool CheckCaller(LPBYTE pbRetAddr, LPVOID *funcAddr, DWORD numFuncAddrs, DWORD max_size);

	///*!
	// [Internal function. Do not call it.]
	//
	// To determine if the caller is legal.\n
	//
	//-# pbRetAddr - [IN] the return address within the caller function\n
	//-# funcAddr - [IN] the array of function pointers to be checked.\n
	//-# numFuncAddrs - [IN] number of function addrs to check\n
	//-# max_size - [IN] the max size of the caller function.\n
	// Return value - [bool] return true if one of callers within the array, funcAddr, is legal, else false.\n
	// Supported platform: All Windows\n
	//*/
	bool CheckCallerSimple(LPBYTE pbRetAddr, LPVOID *funcAddrs, DWORD numFuncAddrs, DWORD max_size);

	///*!
	// [Internal macro. Do not call it.]
	//
	// To get the current frame pointer, it is available if there is no /Oy flag.\n
	//
	//-# pdwFramePointer - [IN/OUT] the current frame pointer.\n
	// Return value - None\n
	//
	// Supported platform: All Windows\n
	// Note:\n
	// 
	//*/
	#define GetCurrentFramePointer(/*out*/pdwFramePointer)		\
		__asm {	mov	pdwFramePointer, ebp}	

	///*!
	// [Internal macro. Do not call it.]
	//
	// To get the current stack pointer.\n
	//
	//-# pdwStackPointer - [IN/OUT] the current stack pointer.\n
	// Return value - None\n
	//
	// Supported platform: All Windows\n
	// Note:\n
	// 
	//*/
	#define GetCurrentStackPointer(/*out*/pdwStackPointer)		\
		__asm {	mov	pdwStackPointer, esp}	


	///*!
	// [Internal use. Do not call it.]
	//
	// To Check authenticity of caller using frame pointer, it is available if there is no /Oy flag.\n
	//
	//-# pFuncAddrs - [IN] the array of function pointers to be checked.\n
	//-# cbNumFuncAddrs - [IN] number of function addrs to check\n
	//-# bResult - [IN/OUT] it is true if the caller detected is legal, else false.\n
	// Supported platform: All Windows\n
	//
	// Note:\n
	// The functions pointed in the function pointer array, pFuncAddrs must be either traditional 
	// C functions or static member functions if in classes.  For example, in the following class
	// and member declarations:
	// class cA
	// {
	//		void fncA1();
	//		static void fncA2();
	// };
	// 
	// The function fncA() pointer assignment to the pFuncAddrs array is not allowed.
	// pFuncAddrs[1] = { &cA::fncA1 };		// not allowed
	//
	// On other other hand, the fncA2() is allowed.
	// pFuncAddrs[1] = { &cA::fncA2 };		// allowed
	//
	//*/
	#define iviTR_VERIFY_CALLER_FRAMEPOINTER(pFuncAddrs, cbNumFuncAddrs, bResult)	\
		iviTRVerifyCaller(1, pFuncAddrs, cbNumFuncAddrs, bResult)

	///*!
	// [Internal use. Do not call it.]
	//
	// To verify the legality of caller using first parameter to function.\n
	//
	//-# firstparam - [IN] the first parameter of callee.\n
	//-# pFuncAddrs - [IN] the array of function pointers to be checked.\n
	//-# cbNumFuncAddrs - [IN] number of function addrs to check\n
	//-# bResult - [IN/OUT] it is true if the caller detected is legal, else false.\n
	// Supported platform: All Windows\n
	//
	// Note:\n
	// The functions pointed in the function pointer array, pFuncAddrs must be either traditional 
	// C functions or static member functions if in classes.  For example, in the following class
	// and member declarations:
	// class cA
	// {
	//		void fncA1();
	//		static void fncA2();
	// };
	// 
	// The function fncA() pointer assignment to the pFuncAddrs array is not allowed.
	// pFuncAddrs[1] = { &cA::fncA1 };		// not allowed
	//
	// On other other hand, the fncA2() is allowed.
	// pFuncAddrs[1] = { &cA::fncA2 };		// allowed
	//
	//*/
	#define iviTR_VERIFY_CALLER_PARAM(firstparam, pFuncAddrs, cbNumFuncAddrs, bResult)	\
		LPBYTE pbRetAddr;													\
		LPDWORD	retAddr = LPDWORD(&firstparam) - 1;							\
																		\
		pbRetAddr = reinterpret_cast<LPBYTE>(retAddr[0]);					\
																		\
		if(pbRetAddr)														\
		{																	\
			bResult = CheckCallerSimple(pbRetAddr, pFuncAddrs, cbNumFuncAddrs, MAX_FUNCTION_SIZE);\
		}

	///*!
	// [Internal macro. Do not call it.]
	//
	// To Check authenticity of caller using frame pointer, it is available if there is no /Oy flag.\n
	//
	// --------------------------------------------------------------------------------------------------------
	// caller0													  																--------dwLevel = 4
	//		|-------->caller1                                     					--------dwLevel = 3
	//                   |---------->caller2                      				--------dwLevel = 2
	//																	|-------->caller3         				--------dwLevel = 1
	//												 												|---------->callee
	//-# dwLevel - [IN] the level of call stack, it can be used to appoint which level caller will be checked.\n
	//-# pFuncAddrs - [IN] the array of function pointers to be checked.\n
	//-# cbNumFuncAddrs - [IN] number of function addrs to check\n
	//-# bResult - [IN/OUT] it is true if the caller detected is legal, else false.\n
	// Supported platform: All Windows\n
	//*/
	#define iviTRVerifyCaller(dwLevel, pFuncAddrs, cbNumFuncAddrs, bResult)	\
		DWORD iviTR_i;															\
		LPBYTE pbRetAddr;													\
		LPDWORD	pdwFramePointer;											\
																		\
		GetCurrentFramePointer(pdwFramePointer);							\
		for(iviTR_i=(dwLevel)>0?(dwLevel):1; iviTR_i>0; iviTR_i--)							\
		{																	\
			if(pdwFramePointer)												\
			{																\
				pbRetAddr = GetReturnAddr(pdwFramePointer);					\
				pdwFramePointer = GetPreviousFramePointer(pdwFramePointer);	\
			}																\
		}																	\
		if(pbRetAddr)														\
		{																	\
			bResult = CheckCallerSimple(pbRetAddr, pFuncAddrs, cbNumFuncAddrs, MAX_FUNCTION_SIZE);\
		}

//#define EmitFunctionList(cbNumFuncAddrs,...)\
//	LPVOID func_list[cbNumFuncAddrs] = {__VA_ARGS__}

	///*!
	// Register the call to be valid before invoking the callee function that
	// contains caller checking macro.
	//
	// Return value - None
	//
	//-#GUID - [IN] a unique GUID token that identifies this validated caller checking.
	//-#INDEX - [IN] a unique index number within a function scope.  It's recommended
	//          to always use preprocessor macro __LINE__ as the parameter.
	//
	// Supported platform:		All Windows\n
	// Performance overhead:	Medium
	// Security level:			Medium
	// Usage scope:				Block scope 
	//
	// Note:\n
	// iviTR_VERIFY_CALLER_REGISTER(GUID, INDEX) and iviTR_VERIFY_CALLER(bInvalidCaller, GUID)
	// must be used in pair to correctly check if a caller being valid.
	// iviTR_VERIFY_CALLER_REGISTER(GUID, INDEX) must be placed right before calling
	// the function (callee) that contains the validated caller checking macro,
	// iviTR_VERIFY_CALLER(bInvalidCaller, GUID).  The caller and callee pair that registers
	// carries out the checking must use the same GUID to identify themselves to each
	// other.  The GUID must be input by users as a string type and its value should
	// be generated via GUID generation tools, e.g. guidgen.exe.  The GUID input must
	// use the following format:
	//		XXXXXXXX_XXXX_XXXX_XXXX_XXXXXXXXXXXX
	// The dash symbol "-" in commonly seen format needs to be replaced with underscore "_"
	//
	// When using iviTR_VERIFY_CALLER_REGISTER(GUID, INDEX), only GUID needs to be specified
	// by users.  The INDEX parameter will always be __LINE__
	//
	// Usage:\n
	//
	// In caller function:
	// ============================================================================
	// void callerfunction()
	// {
	//		...
	//		iviTR_VERIFY_CALLER_REGISTER("10F4EB1F_BE6C_44e8_A8D8_FD0A7E009497", __LINE__)
	//		calleefunction();
	//		...
	// }
	//
	// In callee function:
	// ============================================================================
	// void calleefunction()
	// {
	//		...
	//		bool bInvalidCaller = false;
	//		iviTR_VERIFY_CALLER(bInvalidCaller, "10F4EB1F_BE6C_44e8_A8D8_FD0A7E009497")
	//		if (bInvalidCaller)
	//			{ // cause instability }
	// }
	//
	//*/
    #define iviTR_VERIFY_CALLER_REGISTER(GUID, INDEX) \
        iviTR_INIT_DETECT_CALLER_VALIDATION(GUID, INDEX)

	///*!
	// Check if the function is being called from a valid (registered) caller.
	//
	// Return value - bool
	//
	//-#GUID - [IN] a unique GUID token that identifies this validated caller checking.
	//-#bInvalidCaller - [OUT] boolean type return value.  false if the caller is valid,
	//                   true if invalid.
	//
	// Supported platform:		All Windows\n
	// Performance overhead:	Medium
	// Security level:			Medium
	// Usage scope:				Macro scope
	//
	// Note:\n
	// iviTR_VERIFY_CALLER_REGISTER(GUID, INDEX) and iviTR_VERIFY_CALLER(bInvalidCaller, GUID)
	// must be used in pair to correctly check if a caller being valid.
	// iviTR_VERIFY_CALLER_REGISTER(GUID, INDEX) must be placed right before calling
	// the function (callee) that contains the validated caller checking macro,
	// iviTR_VERIFY_CALLER(bInvalidCaller, GUID).  The caller and callee pair that registers
	// carries out the checking must use the same GUID to identify themselves to each
	// other.  The GUID must be input by users as a string type and its value should
	// be generated via GUID generation tools, e.g. guidgen.exe.  The GUID input must
	// use the following format:
	//		XXXXXXXX_XXXX_XXXX_XXXX_XXXXXXXXXXXX
	// The dash symbol "-" in commonly seen format needs to be replaced with underscore "_"
	//
	// When using iviTR_VERIFY_CALLER_REGISTER(GUID, INDEX), only GUID needs to be specified
	// by users.  The INDEX parameter will always be __LINE__
	//
	// Usage:\n
	//
	// In caller function:
	// ============================================================================
	// void callerfunction()
	// {
	//		...
	//		iviTR_VERIFY_CALLER_REGISTER("10F4EB1F_BE6C_44e8_A8D8_FD0A7E009497", __LINE__)
	//		calleefunction();
	//		...
	// }
	//
	// In callee function:
	// ============================================================================
	// void calleefunction()
	// {
	//		...
	//		bool bInvalidCaller = false;
	//		iviTR_VERIFY_CALLER(bInvalidCaller, "10F4EB1F_BE6C_44e8_A8D8_FD0A7E009497")
	//		if (bInvalidCaller)
	//			{ // cause instability }
	// }
	//
	//*/
    #define iviTR_VERIFY_CALLER(bInvalidCaller, GUID) \
		iviTR_DETECT_CALLER_VALIDATION(bInvalidCaller, GUID)

#elif defined(__linux__)

#define iviTR_VERIFY_CALLER_FRAMEPOINTER(pFuncAddrs, cbNumFuncAddrs, bResult)
#define iviTR_VERIFY_CALLER_PARAM(firstparam, pFuncAddrs, cbNumFuncAddrs, bResult)
#define iviTR_VERIFY_CALLER_REGISTER(GUID, INDEX)
#define iviTR_VERIFY_CALLER(bInvalidCaller, GUID) { bInvalidCaller = false; }

#endif	// _WIN32

#endif	//_IVI_CALLSTACKVERIFY_H
