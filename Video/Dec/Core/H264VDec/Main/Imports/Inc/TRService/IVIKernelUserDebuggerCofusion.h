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

#if !defined(__IVI_KERNEL_USER_DEBUGGER_CONFUSION_H__)
#define __IVI_KERNEL_USER_DEBUGGER_CONFUSION_H__

#include "IVITRComm.h"

///*!
// [Internal function. Do not call it.]
//
// Block user any keyboard input.
//
// Return value - [bool] if success, return true.
//
// -#bIs2LockUserInput - [IN] enable block or not.
//
// Supported platform: All Windows
// Note:\n
// If input is already blocked, the return value is false.
//*/
bool __cdecl TR_BlockUserInput(bool bIs2LockUserInput);

///*!
// [Internal function. Do not call it.]
//
// Check multiple running instance exist.
//
// Return value - [bool] if success, return true.
//
// Supported platform: All Windows
// Note:\n
//*/
bool __cdecl TR_CheckMultipleRunningInstance();



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#if !defined(__USER_INPUT_BLOCKER_H__)
#define __USER_INPUT_BLOCKER_H__

///*!
// Block user any keyboard input.
//
// Return value - [bool] if success, return true.
//
// -#BOOLEAN_VAL - [IN] can not be NULL, enable block or not.
//
// Supported platform:		All Windows
// Performance overhead:	Low
// Security level:			Low
// Usage scope:				Global/process scope
//
//*/
#ifdef _WIN32
	#define iviTR_BLOCK_USER_INPUT(BOOLEAN_VAL) TR_BlockUserInput(BOOLEAN_VAL)
#elif defined(__linux__)
	#define iviTR_BLOCK_USER_INPUT(BOOLEAN_VAL) true
#endif // _WIN32

#endif //__USER_INPUT_BLOCKER_H__

#if !defined(__MULTIPLE_RUNNING_INSTANCE_CHECKER_H__)
#define __MULTIPLE_RUNNING_INSTANCE_CHECKER_H__

///*!
// Check if current module(dll/exe) has been load from other un-approved instance(program).
// If there is more than one instance to use this protected function,
// it can be used in illegal instance(process).
// it may mean that it is under debugging.
//
// Return value - [bool] if success, return true if it has multiple instances.
//
// Supported platform:		All Windows
// Performance overhead:	Low
// Security level:			Low
// Usage scope:				Global/process scope
//
// Note:\n
// You have to well-know your function in module, 
// place this macro in some procedure that only run once in one instance.
// Do not use this macro in function that be called frequently.
// Do not use this macro twice in module scope(DLL/EXE).
//*/
#ifdef _WIN32
	#define iviTR_CHECK_MULTIPLE_RUNNING_INSTANCE() TR_CheckMultipleRunningInstance()
#elif defined(__linux__)
	#define iviTR_CHECK_MULTIPLE_RUNNING_INSTANCE() false
#endif // _WIN32

#endif //__MULTIPLE_RUNNING_INSTANCE_CHECKER_H__

#endif //__IVI_KERNEL_USER_DEBUGGER_CONFUSION_H__
