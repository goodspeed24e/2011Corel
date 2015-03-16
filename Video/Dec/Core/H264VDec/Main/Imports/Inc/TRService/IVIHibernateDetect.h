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
#ifndef _IVI_HIBERNATE_DETECT_H
#define _IVI_HIBERNATE_DETECT_H

#include "IVITRComm.h"

#ifdef _WIN32
	#include <Pbt.h>

	#define TR_ONMSG_BATTERYLOW         PBT_APMBATTERYLOW
	#define TR_ONMSG_OEMEVNET           PBT_APMOEMEVENT
	#define TR_ONMSG_POWERSTATUSCHANGE  PBT_APMPOWERSTATUSCHANGE
	#define TR_ONMSG_QUERYSUSPEND       PBT_APMQUERYSUSPEND
	#define TR_ONMSG_QUERYSUSPENDFAILED PBT_APMQUERYSUSPENDFAILED
	#define TR_ONMSG_RESUMEAUTOMATIC    PBT_APMRESUMEAUTOMATIC
	#define TR_ONMSG_RESUMECRITICAL     PBT_APMRESUMECRITICAL
	#define TR_ONMSG_RESUMESUSPEND      PBT_APMRESUMESUSPEND
	#define TR_ONMSG_SUSPEND            PBT_APMSUSPEND

	///*!
	// The callback function proto-type to receive notify from system hibernating.
	//
	// Return value - 0, TRUE, BROADCAST_QUERY_DENY
	//
	// -#dwType - [OUT] TR_ONMSG_BATTERYLOW ... as above definition.
	// -#lParam - [OUT] Function-specific data, For most events, its reserved and not used.
	//
	// Supported platform:		All Windows
	// Performance overhead:	Low
	// Security level:			None
	// Usage scope:				Global/process scope
	// 
	// Note:\n
	// This macro must co-work with "SQPlus.dll", please put your module with "SQPlus.dll".
	// will help you receive hibernating message from Windows.
	// more details information, please see WM_POWERBROADCAST in MSDN.
	//*/
	typedef LRESULT (CALLBACK * TR_SYS_HibernateCallBack)(DWORD dwType, LPARAM lParam);

	///*!
	// [Internal function. Do not call it.]
	//
	//*/
	bool __cdecl TR_fnRegisterHibernateMonitor(TR_SYS_HibernateCallBack pfnCallback);

	///*!
	// Register a callback function to retrieve OS hibernating notify
	//
	// Return value - None
	//
	// -#pfn - [IN] can not be NULL, specify an user-define callback function as TR_SYS_HibernateCallBack.
	//
	// Supported platform:		All Windows
	// Performance overhead:	Low
	// Security level:			None
	// Usage scope:				Global/process scope
	// 
	// Note:\n
	// This macro must co-work with "SQPlus.dll", please put your module with "SQPlus.dll".
	// will help you receive hibernating message from Windows.
	// more details information, please see WM_POWERBROADCAST in MSDN.
	//*/
	#define iviTR_REGISTER_HIBERNATE_MONITOR(pfn) TR_fnRegisterHibernateMonitor(pfn)

	///*!
	// [Internal function. Do not call it.]
	//
	//*/
	bool __cdecl TR_fnUnRegisterHibernateMonitor(TR_SYS_HibernateCallBack pfnCallback);

	///*!
	// Unregister a callback function to retrieve OS hibernating notify
	//
	// Return value - None
	//
	// -#pfn - [IN] can not be NULL, specify an user-define callback function as TR_SYS_HibernateCallBack.
	//
	// Supported platform:		All Windows
	// Performance overhead:	Low
	// Security level:			None
	// Usage scope:				Global/process scope
	// 
	// Note:\n
	// This macro must co-work with "SQPlus.dll", please put your module with "SQPlus.dll".
	// will help you remove a receiver callback function.
	// Please call it if you don't want use anymore after use iviTR_REGISTER_HIBERNATE_MONITOR.
	//*/
	#define iviTR_UNREGISTER_HIBERNATE_MONITOR(pfn) TR_fnUnRegisterHibernateMonitor(pfn)

#elif defined(__linux__)
	#define iviTR_REGISTER_HIBERNATE_MONITOR(pfn) {}
	#define iviTR_UNREGISTER_HIBERNATE_MONITOR(pfn) {}
#endif // _WIN32

#endif // _IVI_HIBERNATE_DETECT_H
