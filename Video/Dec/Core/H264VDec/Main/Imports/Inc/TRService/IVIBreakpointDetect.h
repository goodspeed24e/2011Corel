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
#ifndef _IVI_BREAKPOINT_DETECT_H
#define _IVI_BREAKPOINT_DETECT_H

#include "IVITRComm.h"

///*!
// [Internal function. Do not call it.]
//
// Check has break point at specified memory address.
//
// Return value - [bool] true is exist, false is not.
//
// -#address - [IN] function memory address for checking break point.
// 
// Supported platform: All Windows/Linux
//*/
bool __cdecl IsBPX(void* address);

///*!
// [Internal function. Do not call it.]
//
// Check has break point at specified memory address.
//
// Return value - [bool] true is exist, false is not.
//
// -#address - [IN] function memory address for checking break point.
// 
// Supported platform: All Windows/Linux
//*/
bool __cdecl IsBPX_v2(void* address);



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

///*!
// Check has break point at specified memory address. This can use to check if a potential debugger is debugging current program.
//
// Return value - None
//
// -#bool_variable - [IN/OUT] get return value, true is exist, or not.
// -#dword_variable - [IN] function memory address for checking break point.
// 
// Supported platform:		All Windows/Linux
// Performance overhead:	Low
// Security level:			Low
// Usage scope:				Macro scope
//*/
#define iviTR_MEMORY_ADDRESS_BREAKPOINT_DETECT(bool_variable, dword_variable)\
{bool_variable =  IsBPX_v2(dword_variable); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bool_variable, iviTR_MEMORY_ADDRESS_BREAKPOINT_DETECT)}

#endif // _IVI_BREAKPOINT_DETECT_H
