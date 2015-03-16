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
#ifndef _IVI_ANTIMEMORYDUMP_H
#define _IVI_ANTIMEMORYDUMP_H

///*!
// [Internal function. Do not call it.]
//
// Modify the SizeOfImage value of the specified module in 
// both PEB structure and PE header.  Save the handle and 
// SizeOfImage information to a watch list.
//
// Return value - [bool] true if successful, false if not
// Supported platform: Windows NT/2K
//*/
bool __cdecl gtrRegisterImagesize(HANDLE hModule);

///*!
// [Internal function. Do not call it.]
//
// Restore SizeOfImage vlaue of the target module in both 
// PEB and PE header.  Remove the handle of the module and
// its SizeOfImage information entry from the watch list.
//
// Return value - [bool] true if successful, false if not
// Supported platform: Windows NT/2K
//*/
bool __cdecl gtrUnregisterImagesize(HANDLE hModule);

///*!
// [Internal function. Do not call it.]
//
// Start to check periodically if any SizeOfImage values of 
// the modules in the watch list have been modified illegally.
//
// Return value - [bool] true if successful, false if not
// Supported platform: Windows NT/2K
//*/
bool __cdecl gtrStartImagesizeChecker();

///*!
// [Internal function. Do not call it.]
//
// Start to check periodically if any SizeOfImage values of 
// the modules in the watch list have been modified illegally.
//
// Supported platform: Windows NT/2K
//*/
bool __cdecl gtrStopImagesizeChecker();

#ifdef _WIN32

	///*!
	// The macro iviTR_REGISTER_IMAGESIZE modifies the SizeOfImage values of
	// the specified module, in both PEB structure and PE header.  If the
	// modification is successful, the information is then saved to a watch
	// list.
	//
	// Return value - None
	//
	// -#hModule - [IN] can not be NULL, the target module's handle
	//
	// Supported platform:		Windows NT/2K
	// Performance overhead:	Medium
	// Security level:			Low
	// Usage scope:				Global/process scope
	//
	// Usage:\n
	//
	// When the target module is a WIN32 executable, call the macro at
	// the application's initialization (e.g. InitInstance()).
	// ===================================================================
	//	iviTR_REGISTER_IMAGESIZE(::GetModuleHandle(NULL));
	//
	// When the target module is a DLL and will be loaded upon application's
	// launch (due to dependencies), call iviTR_REGISTER_IMAGESIZE within
	// DllMain.  Place the macro at the end of DLL_PROCESS_ATTACH case
	// statement.
	// ===================================================================
	// BOOL APIENTRY DllMain(HANDLE hModule, 
	//						 DWORD  ul_reason_for_call, 
	//						 LPVOID lpReserved)
	// {
	//		....
	//		if (DLL_PROCESS_ATTACH == ul_reason_for_call)
	//		{
	//			...
	//			iviTR_REGISTER_IMAGESIZE(hModule);
	//		}
	//
    //		return TRUE;
	//		....
	//	}
	//
	// When the target module is a DLL and loaded and unloaded dynamically
	// inside the application via LoadLibrary() and FreeLibrary().  Call
	// iviTR_REGISTER_IMAGESIZE right after LoadLibrary().
	// iviTR_REGISTER_IMAGESIZE & iviTR_UNREGISTER_IMAGESIZE macros should
	// be called in pair.
	// ===================================================================
	//	HINSTANCE hInstance = NULL;
	//	hInstance = ::LoadLibrary("MyDll.dll");
	//	if (NULL != hInstance)
	//	{
	//		iviTR_REGISTER_IMAGESIZE(hInstance);
	//		...
	//		...
	//		iviTR_UNREGISTER_IMAGESIZE(hInstance);
	//		FreeLibrary(hInstance);
	//	}
	// 
	//*/
	#define iviTR_REGISTER_IMAGESIZE(hModule)				\
	{ gtrRegisterImagesize(hModule); }

	///*!
	// The macro iviTR_UNREGISTER_IMAGESIZE removes the entry of 
	// the specified module from the checking list.
	//
	// Return value - None
	//
	// -#hModule - [IN] can not be NULL, the target module's handle
	//
	// Supported platform:		Windows NT/2K
	// Performance overhead:	Medium
	// Security level:			Low
	// Usage scope:				Global/process scope
	//
	// Usage:\n
	//
	// When the target module is a WIN32 executable, call the macro
	// at the application's termination (e.g. ExitInstance()).
	// ===================================================================
	//	iviTR_UNREGISTER_IMAGESIZE(::GetModuleHandle(NULL));
	//
	// When the target module is a DLL and loaded and unloaded dynamically
	// inside the application via LoadLibrary() and FreeLibrary().  Call
	// iviTR_UNREGISTER_IMAGESIZE before FreeLibrary().
	// iviTR_REGISTER_IMAGESIZE & iviTR_UNREGISTER_IMAGESIZE macros should
	// be called in pair.
	// ===================================================================
	//	HINSTANCE hInstance = NULL;
	//	hInstance = ::LoadLibrary("MyDll.dll");
	//	if (NULL != hInstance)
	//	{
	//		iviTR_REGISTER_IMAGESIZE(hInstance);
	//		...
	//		...
	//		iviTR_UNREGISTER_IMAGESIZE(hInstance);
	//		FreeLibrary(hInstance);
	//	}
	// 
	// Note:\n
	// Do not call iviTR_UNREGISTER_IMAGESIZE macro inside DllMain().
	//
	//*/
	#define iviTR_UNREGISTER_IMAGESIZE(hModule)	\
	{ gtrUnregisterImagesize(hModule); }

	///*!
	// The macro iviTR_START_IMAGESIZE_CHECKER starts an opertation
	// that constantly checks if the new SizeOfImage values of the modules
	// in the watch list are being changed illegally.  If yes, the
	// application crashes immediately.
	//
	// Return value - None
	//
	// Supported platform:		Windows NT/2K
	// Performance overhead:	Medium
	// Security level:			Low
	// Usage scope:				Global/process scope
	//
	// Note:\n
	// 
	// Insure the macro is called after the module loading process is fully
	// completely.  E.g. Do not call it in a constructor of a class or
	// DllMain() function of a DLL module.
	//
	//*/
	#define iviTR_START_IMAGESIZE_CHECKER \
	{ gtrStartImagesizeChecker(); }

	///*!
	// The macro iviTR_STOP_IMAGESIZE_CHECKER stops the opertation
	// that constantly checks if the new SizeOfImage values of the modules
	// in the watch list are being changed illegally.
	//
	// Return value - None
	//
	// Supported platform:		Windows NT/2K
	// Performance overhead:	Medium
	// Security level:			Low
	// Usage scope:				Global/process scope
	//
	// Note:\n
	// 
	// Always call this macro after iviTR_START_IMAGESIZE_CHECKER.
	//
	//*/
	#define iviTR_STOP_IMAGESIZE_CHECKER \
	{ gtrStopImagesizeChecker(); }	

#elif defined(__linux__)

	#define iviTR_REGISTER_IMAGESIZE(hModule)
	#define iviTR_UNREGISTER_IMAGESIZE(hModule)
	#define iviTR_START_IMAGESIZE_CHECKER
	#define iviTR_STOP_IMAGESIZE_CHECKER

#endif // _WIN32

#endif // _IVI_ANTIMEMORYDUMP_H