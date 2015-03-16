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

#if !defined(_H__DETECT_TRENABLE_)
#define _H__DETECT_TRENABLE_

#include "IVITrWinDef.h"
#include "IVITRComm.h"

#ifdef TR_ENABLE
    #define TR_ENABLE_FLAG true
#else
    #define TR_ENABLE_FLAG false
#endif

#if (defined(TR_ENABLE_NEWMACROS) || defined(TR_ENABLE_NEWMACROS_WITHOUT_ANTIDEBUG))
    #define TR_ENABLE_NEWMACROS_FLAG true
#else
    #define TR_ENABLE_NEWMACROS_FLAG false
#endif

///*!
// [Internal definition. Do not call it.]
//
// this is TR_ENABLE mutex key for engine side.
//*/
#ifdef _UNICODE
#define ENGINE_MUTEX_NAME_FOR_TR_ENABLE           L"C9664A3D-C19B-4f52-A964-1977E03A8911"
#else
#define ENGINE_MUTEX_NAME_FOR_TR_ENABLE           "C9664A3D-C19B-4f52-A964-1977E03A8911"
#endif

///*!
// [Internal definition. Do not call it.]
//
// this is TR_ENABLE_NEWMACROS mutex key for engine side.
//*/
#ifdef _UNICODE
#define ENGINE_MUTEX_NAME_FOR_TR_ENABLE_NEWMACROS L"A95A112A-656E-4e10-867A-5431A8734D76"
#else
#define ENGINE_MUTEX_NAME_FOR_TR_ENABLE_NEWMACROS "A95A112A-656E-4e10-867A-5431A8734D76"
#endif

///*!
// [Internal definition. Do not call it.]
//
// this is TR_ENABLE mutex key for UI side.
//*/
#ifdef _UNICODE
#define UI_MUTEX_NAME_FOR_TR_ENABLE           L"0AD2CF15-1CC1-4fcb-8E8D-0232A29F5046"
#else
#define UI_MUTEX_NAME_FOR_TR_ENABLE           "0AD2CF15-1CC1-4fcb-8E8D-0232A29F5046"
#endif

///*!
// [Internal definition. Do not call it.]
//
// this is TR_ENABLE_NEWMACROS mutex key for UI side.
//*/
#ifdef _UNICODE
#define UI_MUTEX_NAME_FOR_TR_ENABLE_NEWMACROS L"9B3F1E5A-9D8A-4f3d-B4B9-9766AD15744A"
#else
#define UI_MUTEX_NAME_FOR_TR_ENABLE_NEWMACROS "9B3F1E5A-9D8A-4f3d-B4B9-9766AD15744A"
#endif

NS_TRLIB_BEGIN

///*!
// [Internal definition. Do not call it.]
//
// Check run-time product is enable TR_ENABLE and TR_ENABLE_NEWMACROS flag of engine/UI side.
//
// Return value - None
//
// -#bCheckEnginePart - [IN] check UI or engine side, true is engine side, or UI side.
// -#pbIsAppliedForTR_ENABLE - [IN] can not be NULL. return true means 'TR_ENABLE is applied'
// -#pbIsAppliedForTR_ENABLE_NEWMACROS - [IN] can not be NULL. return true means 'TR_ENABLE_NEWMACROS is applied'
//
// Supported platform: All Windows
//*/
void  __cdecl gfnIsAppliedWithTR_ENABLE(bool bCheckEnginePart, bool *pbIsAppliedForTR_ENABLE, bool *pbIsAppliedForTR_ENABLE_NEWMACROS);

NS_TRLIB_END

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

///*!
// Check if TR_ENABLE/TR_ENABLE_NEWMACROS are defined in the engine modules. If they are defined,
// the corresponding mutex will be created for later check by iviTR_IS_TR_ENABLE_APPLIED_FOR_ENGINE().
//
// Return value - None
//
// Supported platform:		All Windows
// Performance overhead:	Low
// Security level:			None
// Usage scope:				Global/process scope
//
// Note:\n
// call this macro only at the engine modules and should be called before iviTR_IS_TR_ENABLE_APPLIED_FOR_ENGINE() is called
// such like in 'CGPIControl::CGPIControl()' of 'Shared\Components\GPIProxy\GPIControl.cpp'.
//*/
#define iviTR_CHECK_ENGINE_TR_ENABLE_APPLIED()\
{   if (TR_ENABLE_FLAG){\
		::CreateMutex(NULL, FALSE, ENGINE_MUTEX_NAME_FOR_TR_ENABLE);\
    }\
    if (TR_ENABLE_NEWMACROS_FLAG){\
        ::CreateMutex(NULL, FALSE, ENGINE_MUTEX_NAME_FOR_TR_ENABLE_NEWMACROS);\
    }} 

///*!
// Check if TR_ENABLE/TR_ENABLE_NEWMACROS are defined in the UI modules. If they are defined,
// the corresponding mutex will be created for later check by iviTR_IS_TR_ENABLE_APPLIED_FOR_UI().
//
// Return value - None
//
// Supported platform:		All Windows
// Performance overhead:	Low
// Security level:			None
// Usage scope:				Global/process scope
//
// Note:\n
// call this macro only at the UI modules and should be called before iviTR_IS_TR_ENABLE_APPLIED_FOR_UI() is called
// such like in 'CWinDVDApp::CWinDVDApp()' of 'DVD\Source\WinDVD\WinDVD.cpp'.
//*/
#define iviTR_CHECK_UI_TR_ENABLE_APPLIED()\
{   if (TR_ENABLE_FLAG){\
        ::CreateMutex(NULL, FALSE, UI_MUTEX_NAME_FOR_TR_ENABLE);\
    }\
    if (TR_ENABLE_NEWMACROS_FLAG){\
        ::CreateMutex(NULL, FALSE, UI_MUTEX_NAME_FOR_TR_ENABLE_NEWMACROS);\
    }}

///*!
// Check if run-time product is enabled TR_ENABLE and TR_ENABLE_NEWMACROS flag of engine side.
//
// Return value - None
//
// -#pbTR_ENABLE - [IN] can not be NULL, check result of TR_ENABLE flag, BOOL type of variable prepared by caller.
// -#pbTR_ENABLE_NEWMACROS - [IN] can not be NULL, check result of TR_ENABLE_NEWMACROS flag, BOOL type of variable prepared by caller.
// 
// Supported platform:		All Windows
// Performance overhead:	Low
// Security level:			None
// Usage scope:				Macro scope
// 
// Note:\n
// Can use this macro from the same or difference process. iviTR_CHECK_ENGINE_TR_ENABLE_APPLIED() should be called in the checked product before this macro is called.
//*/
#define iviTR_IS_TR_ENABLE_APPLIED_FOR_ENGINE(pbTR_ENABLE, pbTR_ENABLE_NEWMACROS) { gfnIsAppliedWithTR_ENABLE(true, pbTR_ENABLE, pbTR_ENABLE_NEWMACROS); } // check function

///*!
// Check if run-time product is enabled TR_ENABLE and TR_ENABLE_NEWMACROS flag of UI side.
//
// Return value - None
//
// -#pbTR_ENABLE - [IN] can not be NULL, check result of TR_ENABLE flag, BOOL type of variable prepared by caller.
// -#pbTR_ENABLE_NEWMACROS - [IN] can not be NULL, check result of TR_ENABLE_NEWMACROS flag, BOOL type of variable prepared by caller.
// 
// Supported platform:		All Windows
// Performance overhead:	Low
// Security level:			None
// Usage scope:				Macro scope
// 
// Note:\n
// Can use this macro from the same or difference process. iviTR_CHECK_UI_TR_ENABLE_APPLIED() should be called in the checked product before this macro is called.
//*/
#define iviTR_IS_TR_ENABLE_APPLIED_FOR_UI(pbTR_ENABLE, pbTR_ENABLE_NEWMACROS) { gfnIsAppliedWithTR_ENABLE(false, pbTR_ENABLE, pbTR_ENABLE_NEWMACROS); } // check function

#endif // !defined(_H__DETECT_TRENABLE_)
