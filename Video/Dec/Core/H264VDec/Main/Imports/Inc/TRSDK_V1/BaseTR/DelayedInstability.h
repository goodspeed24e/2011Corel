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
#ifndef _IVI_DELAYEDINSTABILITY_H
#define _IVI_DELAYEDINSTABILITY_H

#include "IVITrWinDef.h"
#include "IVITRComm.h"

#ifdef _WIN32

NS_TRLIB_BEGIN

#ifdef __cplusplus
	///*!
	// [Internal structure. Do not call it.]
	//
	// this is base class, does not have correct behavior
	//*/
	class DelayedInstability
	{
	public:
		///*!
		// Erase stack history before AP crash.
		//*/
		void EraseStack();
		///*!
		// Execute instability function.
		// -# bCauseInstability - [IN] set instability, TRUE cause instability, or not.
		//*/	
		virtual void Update(BOOL bCauseInstability) = 0;
	};
	 
	///*!
	// [Internal structure. Do not call it.]
	//
	// cause delay hang class in AP process.
	//
	// Supported platform:		All Windows
	// Performance overhead:	Low
	// Security level:			Low
	// Usage scope:				Macro scope
	//
	// Usage:\n
	// void Sample()
	// {
	//    DelayedHang hang;
	//
	//    // do something working code here  
	// 
	//    if (bDebuggerExist) hang.Update(TRUE); // if any debugger exist, hang immediatly.
	// }
	//*/
	class DelayedHang : public DelayedInstability
	{
		HANDLE hSem;

	public:
		DelayedHang(int maxCount = 128);
		virtual ~DelayedHang();
		///*!
		// Execute instability function.
		// -# bCauseInstability - [IN] set instability, TRUE cause instability, or not.
		//*/		
		virtual void Update(BOOL bCauseInstability);
	};

	///*!
	// [Internal structure. Do not call it.]
	//
	// cause delay crash class in AP process.
	//
	// Supported platform:		All Windows
	// Performance overhead:	Low
	// Security level:			Low
	// Usage scope:				Macro scope
	//
	// Usage:\n
	// void Sample()
	// {
	//    DelayedCrash crash;
	//
	//    // do something working code here  
	// 
	//    if (bDebuggerExist) crash.Update(TRUE); // if any debugger exist, crash immediatly.
	// }
	//*/
	class DelayedCrash : public DelayedInstability
	{
		BYTE* p0;
		BYTE* p;
		
	public:
		DelayedCrash();
		virtual ~DelayedCrash();
		///*!
		// Execute instability function.
		// -# bCauseInstability - [IN] set instability, TRUE cause instability, or not.
		//*/		
		virtual void Update(BOOL bCauseInstability);
	};

	///*!
	// [Internal structure. Do not call it.]
	//
	// cause out of memory class in AP process.
	//
	// Supported platform:		All Windows
	// Performance overhead:	Low
	// Security level:			Low
	// Usage scope:				Macro scope
	//
	// Usage:\n
	// void Sample()
	// {
	//    DelayedOutOfMemory crash;
	//
	//    // do something working code here  
	// 
	//    if (bDebuggerExist) crash.Update(TRUE); // if any debugger exist, crash immediatly.
	// }
	//*/
	class DelayedOutOfMemory : public DelayedInstability
	{
	public:

		DelayedOutOfMemory();
		virtual ~DelayedOutOfMemory();
		///*!
		// Execute instability function.
		// -# bCauseInstability - [IN] set instability, TRUE cause instability, or not.
		//*/			
		virtual void Update(BOOL bCauseInstability);
	};

	///*!
	// [Internal structure. Do not call it.]
	//
	// cause stack overflow class in AP process.
	//
	// Supported platform:		All Windows
	// Performance overhead:	Low
	// Security level:			Low
	// Usage scope:				Macro scope
	//
	// Usage:\n
	// void Sample()
	// {
	//    DelayedStackOverflow crash;
	//
	//    // do something working code here  
	// 
	//    if (bDebuggerExist) crash.Update(TRUE); // if any debugger exist, crash immediatly.
	// }
	//*/
	class DelayedStackOverflow : public DelayedInstability
	{
		int nDepth;
	public:

		DelayedStackOverflow();
		~DelayedStackOverflow();
		///*!
		// Execute instability function.
		// -# bCauseInstability - [IN] set instability, TRUE cause instability, or not.
		//*/		
		virtual void Update(BOOL bCauseInstability);
	};

	class CtrExitProcess
	{
	public:
		CtrExitProcess();
		~CtrExitProcess();

		virtual void ExitNow();

	private:
		HANDLE m_hProcess;
		LPVOID m_pTermProcCode;
	};
	
#endif // __cplusplus
	
NS_TRLIB_END

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	///*!
	// Force program to crash after a random period of time. 
	//
	// Return value - None
	//
	// Supported platform:		All Windows
	// Performance overhead:	Low
	// Security level:			High
	// Usage scope:				Macro scope
	//*/
#ifdef __cplusplus	
	#define iviTR_CRASH()\
	{ iviTR_DUMP_ERROR_TR_MACRO_ENTRY(true, iviTR_CRASH); DelayedCrash ofCrash; ofCrash.Update(true); }
#else
	#define iviTR_CRASH() 
#endif // __cplusplus	

	///*!
	// Exit the process immediately.
	//
	// Return value - None
	//
	// Supported platform:		All Windows
	// Performance overhead:	Low
	// Security level:			Medium
	// Usage scope:				Macro scope
	//*/
	#define iviTR_EXIT_PROCESS()\
	{ iviTR_DUMP_ERROR_TR_MACRO_ENTRY(true, iviTR_EXIT_PROCESS); CtrExitProcess ofExitProcess; ofExitProcess.ExitNow(); }

#elif defined(__linux__)
	#define iviTR_CRASH()
	#define iviTR_EXIT_PROCESS()
#endif // _WIN32

#endif // _IVI_DELAYEDINSTABILITY_H
