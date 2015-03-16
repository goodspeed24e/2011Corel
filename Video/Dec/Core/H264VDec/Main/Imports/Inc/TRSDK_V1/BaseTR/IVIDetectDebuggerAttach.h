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
#ifndef _IVI_DETECTDEBUGGERATTACH_H
#define _IVI_DETECTDEBUGAGERTTACH_H

#include "IVITrWinDef.h"
#include "IVITRComm.h"

#ifdef _WIN32

NS_TRLIB_BEGIN

	///*!
	// [Internal function. Do not call it.]
	//
	// Initialize detection for debugger process attaching.
	//
	// Return value - [bool] true if successful, false if not.\n
	// Supported platform: All Windows\n
	//*/
	bool __cdecl gtrDetectDebuggerAttach();

NS_TRLIB_END

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	///*!
	//
	// Initialize debugger process attaching detection.
	//
	// Return value - None
	//
	// Supported platform:		All Windows
	// Performance overhead:	Medium
	// Security level:			Medium
	// Usage scope:				Global/process scope
	//
	// Note:\n
	// Use iviTR_DETECT_DEBUGGER_ATTACH macro to detect if a user-mode
	// debugger is trying to debug a process by using process attaching
	// mechanism.  The detection enabled by the macro stays effective until
	// the process terminates.  As the debugger attaching is being detected,
	// the macro calls iviTR_CRASH to close the application immediately.
	//
	// It is recommended to use the macro as early as possible during
	// process' creation.  For example, with WinDVD it'd be appropriate to 
	// place the macro in CWinDVDApp::InitInstance().
	// 
	//*/
	#define iviTR_DETECT_DEBUGGER_ATTACH()	\
		{ gtrDetectDebuggerAttach(); }

#elif defined(__linux__)
	#define iviTR_DETECT_DEBUGGER_ATTACH()

#endif //_WIN32

#endif // _IVI_DETECTDEBUGGERATTACH_H