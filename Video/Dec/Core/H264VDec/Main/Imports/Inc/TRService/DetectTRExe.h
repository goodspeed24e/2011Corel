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

#if !defined(_H__DETECT_TREXE_)
#define _H__DETECT_TREXE_

#include "IVITRComm.h"

///*!
// [Internal definition. Do not call it.]
//
// this is TR_EXE mutex key.
//*/
#define MUTEX_NAME_FOR_TR_EXE_APPLIED         "CAEB1D98-BD1B-4b1a-A118-A05A1B616D56"

///*!
// [Internal definition. Do not call it.]
//
// Check and set checkpoint of TREXE.
//
// Return value - None
//
// Supported platform: All Windows
//*/
void __cdecl gfnCHECK_TREXE_APPLIED();


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

///*!
// Check if TREXE is applied on current modules. If applied, the corresponding mutex will be created
// for later check.
//
// Return value - None
//
// Supported platform:		All Windows
// Performance overhead:	Low
// Security level:			None
// Usage scope:				Global/process scope
// 
// Note:\n
// call this macro at the beginning of AP, from UI side.
// such like in 'CWinDVDApp::CWinDVDApp()' of 'DVD\Source\WinDVD\WinDVD.cpp'.
// internal use TR_CHECKSUM_ENABLE_55 for scramble, other modules can not use the same key, otherwise, it will have link error.
//*/
#define iviTR_CHECK_TREXE_APPLIED() { gfnCHECK_TREXE_APPLIED(); } // check function

#endif // !defined(_H__DETECT_TREXE_)
