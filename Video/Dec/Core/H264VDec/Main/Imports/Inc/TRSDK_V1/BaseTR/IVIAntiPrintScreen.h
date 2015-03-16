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
#ifndef _IVI_ANTIPRINTSCREEN_H
#define _IVI_ANTIPRINTSCREEN_H

#include "IVITrWinDef.h"
#include "IVITRComm.h"

NS_TRLIB_BEGIN

///*!
// [Internal function. Do not call it.]
//
// Enable print screen lock.
//
// Return value - None
// Supported platform: All Windows
//*/
void __cdecl TR_EnablePrintScreen();

///*!
// [Internal function. Do not call it.]
//
// Disable print screen lock.
//
// Return value - None
// Supported platform: All Windows
//*/
void __cdecl TR_DisablePrintScreen();

NS_TRLIB_END

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifdef _WIN32
	///*!
	// Disable print screen lock.
	//
	// Return value - None
	//
	// Supported platform:		All Windows
	// Performance overhead:	Low
	// Security level:			Low
	// Usage scope:				Macro scope
	//*/
	#define iviTR_DISABLE_PRINT_SCREEN() {TR_DisablePrintScreen();}

	///*!
	// Enable print screen lock.
	//
	// Return value - None
	//
	// Supported platform:		All Windows
	// Performance overhead:	Low
	// Security level:			Low
	// Usage scope:				Macro scope
	//*/
	#define iviTR_ENABLE_PRINT_SCREEN() {TR_EnablePrintScreen();}
#elif defined(__linux__)
	#define iviTR_DISABLE_PRINT_SCREEN()
	#define iviTR_ENABLE_PRINT_SCREEN()
#endif // _WIN32

#endif // _IVI_ANTIPRINTSCREEN_H
