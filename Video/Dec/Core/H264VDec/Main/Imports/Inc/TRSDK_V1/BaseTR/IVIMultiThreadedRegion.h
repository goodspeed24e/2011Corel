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

#ifndef _IVIMULTITHREADEDREGION_H
#define _IVIMULTITHREADEDREGION_H

#include "IVITrWinDef.h"
#include "IVITRComm.h"

#ifdef _WIN32

	#define MAX_NUMBER_THREADS		25
	#define MIN_NUMBER_THREADS		1

	#define MAX_NUMBERS_REGION		100

	// MTR TYPES
	#define	TR_MTR_SEMAPHORE_RING 0
	#define	TR_MTR_EVENT 1
	#define	TR_MTR_SEMAPHORE_PUMP 2

	#define TR_MTR_EXIT break;
	#define TR_MTR_HANG WaitForSingleObject(pThis->m_hExitEvent,INFINITE);

	#define FALSE_ALARM_COUNT 2

NS_TRLIB_BEGIN

	///*!
	// [Internal function. Do not call it.]
	//
	// Output debug format string
	//
	// Return value - None
	// 
	//*/
	void __cdecl TR_DP(char *szFormat, ...);

#ifdef __cplusplus
	class CTRRegionCollector;
	extern CTRRegionCollector *g_pRegionCollector;
#endif

	///*!
	// [Internal function. Do not call it.]
	//
	// Register a region with its information to MTR
	//
	//*/
	HRESULT WINAPI RegisterMultiThreadedRegion(
		LPVOID pfnStart, 
		LPVOID pfnEnd, 
		volatile DWORD *trChecksumEnable, 
		volatile DWORD *trChecksumValue,
		LPWORD trRelocateTable,
		volatile DWORD *trScrambleEnable,
		int nRegNum);

	///*!
	// [Internal function. Do not call it.]
	//
	// Unregister a region with its number from MTR
	//
	//*/
	HRESULT WINAPI UnregisterMultiThreadRegion(int nRegNum);

	///*!
	// [Internal function. Do not call it.]
	//
	// Create multi-thread region handle.
	//
	// Return value - [LPVOID] return create multi-thread region handle.
	//
	// -#dwMtrType - [IN] create type, would be 0, 1 or 2.
	// -#dwThreads - [IN] can not be zero, number of threads to create, 
	// -#dwTimeout - [IN] can not be zero, time out value (suggested 5000).
	// -#dwLoopSleep - [IN] sleep time between each loop, milli-second.
	// -#dwCheckPeriod - [IN] amount of loops before checking debugger (a value of 3 means check 1 of 3 times).
	// -#phWait - [IN/OUT] a returned handle to wait on.  if there is a problem, this will halt a wait.
	// -#hControl - [IN] a input control handle - this is an run/pause event - when set to inactive, will stop the MTR from running.
	//
	// Supported platform: All Windows
	//
	//*/
	LPVOID WINAPI CreateMultiThreadRegion(
		DWORD dwMtrType,	// type, see above
		DWORD dwThreads,	// number of threads to create
		DWORD dwTimeout,	// timeout value (suggested 5000)
		DWORD dwLoopSleep,	// a sleep period per loop
		DWORD dwCheckPeriod,// amount of loops before executing debug check (a value of 3 means check 1 of 3 times)
		HANDLE *phWait,		// a returned handle to wait on.  if there is a problem, this will halt a wait.
		HANDLE hControl,	// a input control handle - this is an run/pause event - when set to inactive, will stop the MTR from running.
		LPVOID pExtraData, // a pointer to extra data if needed 
		bool bIsAntiDebug);

	///*!
	// [Internal function. Do not call it.]
	//
	// Delete multi-thread region handle.
	//
	// Return value - [HRESULT] S_OK will be success.\n
	//
	// -#pArg - [IN] multi-thread region handle.
	//
	// Supported platform: All Windows\n
	//
	//*/
	HRESULT WINAPI DeleteMultiThreadRegion(LPVOID pArg);

	///*!
	// [Internal function. Do not call it.]
	//
	// Trigger a signal of multi-thread region handle to run/pause.
	//
	// Return value - [HRESULT] S_OK will be success.
	//
	// -#pArg - [IN] multi-thread region handle, this is an run/pause thread.
	//
	// Supported platform: All Windows
	//
	//*/
	HRESULT WINAPI SignalMultiThreadRegion(LPVOID pArg);

NS_TRLIB_END

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	///*!
	// Register a region to MTR
	//
	// -#regNum - [IN] the number of region.
	//
	// -#tr_ScrambleEnable - [IN] a pointer to TR_ScrambleEnable_xx if this region needs to be Scrambled/Descrambled, else set it 0.
	//
	// Supported platform:		All Windows
	// Performance overhead:	Low
	// Security level:			None
	// Usage scope:				Global/process scope
	//
	//*/
 	#define iviTR_REGISTER_MULTI_THREADED_REGION(regNum, tr_ScrambleEnable)\
			RegisterMultiThreadedRegion(TR_RegionBegin_##regNum, TR_RegionEnd_##regNum, &TR_ChecksumEnable_##regNum, &TR_ChecksumValue_##regNum, TR_RelocateTable_##regNum, tr_ScrambleEnable, regNum)

	///*!
	// -#regNum - [IN] the number of region from MTR.
	//
	// Supported platform:		All Windows
	// Performance overhead:	Low
	// Security level:			None
	// Usage scope:				Global/process scope
	//
	//*/
	#define iviTR_UNREGISTER_MULTI_THREADED_REGION(regNum)\
		UnregisterMultiThreadRegion(regNum)

	///*!
	//
	// Create multi-thread region handle.
	//
	// Return value - [LPVOID] return create multi-thread region handle.
	//
	// -#mtrtype - [IN] create type, would be 0, 1 or 2.
	//                  mtrtype 0 - use semaphore ring
	//                  mtrtype 1 - use event
	//                  mtrtype 2 - use semaphore pump
	// -#numthreads - [IN] can not be zero, number of threads to create, 
	// -#timeout - [IN] can not be zero, time out value (suggested 5000).
	// -#loopsleep - [IN] sleep time between each loop, milli-second.
	// -#checkperiod - [IN] amount of loops before checking debugger (a value of 3 means check 1 of 3 times).
	// -#p_wait_handle - [IN/OUT] a returned handle to wait on.  if there is a problem, this will halt a wait.
	// -#ctrl_event - [IN] a input control handle - this is an run/pause event - when set to inactive, will stop the MTR from running.
	// -#extra_data - [IN] a pointer to extra data - multiple uses, currently used for checksums inside threads
	//
	// Supported platform:		All Windows
	// Performance overhead:	High
	// Security level:			Medium
	// Usage scope:				Global/process scope
	//
	//*/
	#ifdef TR_ENABLE_NEWMACROS_WITHOUT_ANTIDEBUG
	#define iviTR_CREATE_MULTI_THREADED_REGION(mtrtype, numthreads, timeout, loopsleep, checkperiod, p_wait_handle, ctrl_event, extra_data) \
		CreateMultiThreadRegion((mtrtype),(numthreads),(timeout),(loopsleep),(checkperiod),(p_wait_handle),(ctrl_event), (extra_data), false)
	#else
	#define iviTR_CREATE_MULTI_THREADED_REGION(mtrtype, numthreads, timeout, loopsleep, checkperiod, p_wait_handle, ctrl_event, extra_data) \
		CreateMultiThreadRegion((mtrtype),(numthreads),(timeout),(loopsleep),(checkperiod),(p_wait_handle),(ctrl_event), (extra_data), true)
	#endif
	
	///*!
	//
	// Delete multi-thread region handle.
	//
	// Return value - [HRESULT] S_OK will be success.
	//
	// -#pArg - [IN] multi-thread region handle.
	//
	// Supported platform:		All Windows
	// Performance overhead:	Low
	// Security level:			None
	// Usage scope:				Global/process scope
	//
	//*/
	
	#define iviTR_DELETE_MULTI_THREADED_REGION(pArg) \
		DeleteMultiThreadRegion((pArg))
    
	///*!
	//
	// Trigger a signal of multi-thread region handle to run/pause.
	//
	// Return value - [HRESULT] S_OK will be success.
	//
	// -#pArg - [IN] multi-thread region handle.
	//
	// Supported platform:		All Windows
	// Performance overhead:	Low
	// Security level:			Medium
	// Usage scope:				Macro scope
	//
	//*/
	
	#define iviTR_SIGNAL_MULTI_THREADED_REGION(pArg) \
		SignalMultiThreadRegion((pArg))

#elif defined(__linux__)
 	#define iviTR_REGISTER_MULTI_THREADED_REGION(regNum, tr_ScrambleEnable)
 	#define iviTR_UNREGISTER_MULTI_THREADED_REGION(regNum)
	#define iviTR_CREATE_MULTI_THREADED_REGION(mtrtype, numthreads, timeout, loopsleep, checkperiod, p_wait_handle, ctrl_event)
	#define iviTR_DELETE_MULTI_THREADED_REGION(pArg)
	#define iviTR_SIGNAL_MULTI_THREADED_REGION(pArg)
#endif // _WIN32

#endif // _IVIMULTITHREADEDREGION_H
