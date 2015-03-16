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
#ifndef _IVI_VIRTUALMACHINE_DETECT_H
#define _IVI_VIRTUALMACHINE_DETECT_H

#include "IVITrWinDef.h"
#include "IVITRComm.h"

NS_TRLIB_BEGIN

///*!
// [Internal function. Do not call it.]
//
// Check OS environment is in Microsoft Virtual PC/Server.
//
// Return value - [bool] return true is exist.
//
// Supported platform: All Windows/Linux
//*/
bool __cdecl TR_IsInsideVPC();

///*!
// [Internal function. Do not call it.]
//
// Check OS environment is in Virtual Machine.
//
// Return value - [bool] return true is exist.
//
// Supported platform: All Windows/Linux
//*/
bool __cdecl TR_IsInsideVMWare_();

///*!
// [Internal function. Do not call it.]
//
// Check OS environment is in Virtual Machine.
//
// Return value - [bool] return true is exist.
//
// Supported platform: All Windows/Linux
//*/
bool __cdecl TR_IsInsideVMWare();

NS_TRLIB_END

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

///*!
// Check if OS environment is in Microsoft Virtual PC/Server.
//
// Return value - None
//
// -#bool_variable - [OUT] can not be NULL, return true is exist.
//
// Supported platform:		All Windows/Linux
// Performance overhead:	Low
// Security level:			Low
// Usage scope:				Macro scope
//
//*/
#define iviTR_INSIDEVIRTUALPC_CHECK(bool_variable) {bool_variable = TR_IsInsideVPC(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bool_variable, iviTR_INSIDEVIRTUALPC_CHECK)}

///*!
// Check if OS environment is in Virtual Machine.
//
// Return value - None
//
// -#bool_variable - [OUT] can not be NULL, return true is exist.
//
// Supported platform:		All Windows/Linux
// Performance overhead:	Low
// Security level:			Low
// Usage scope:				Macro scope
//
//*/
#define iviTR_INSIDEVMWARE_CHECK(bool_variable) {bool_variable = TR_IsInsideVMWare(); iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bool_variable, iviTR_INSIDEVMWARE_CHECK)}

#endif // _IVI_VIRTUALMACHINE_DETECT_H
