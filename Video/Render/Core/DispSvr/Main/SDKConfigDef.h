#ifndef _SDK_CONFIG_DEF_H_
#define _SDK_CONFIG_DEF_H_
	//////////////////////////////////////////////////////////////////
	// All of code for definition MUST include this header.
	// The header include all of macro header for all products.
	//
	// #define SET_DEF_HERE

	#ifdef DISPSVRSDK_QA_TR_RELEASE
		#include "BuildConfig/DispSvrSDK_QA_TR_Release.h"
		#pragma message("include BuildConfig/DispSvrSDK_QA_TR_Release.h")
	#endif

	#ifdef DISPSVRSDK_QA_NOTR_RELEASE
		#include "BuildConfig/DispSvrSDK_QA_NOTR_Release.h"
		#pragma message("include BuildConfig/DispSvrSDK_QA_NOTR_Release.h")
	#endif

	#ifdef DISPSVRSDK_QA_NOTR_PDB_RELEASE
		#include "BuildConfig/DispSvrSDK_QA_NOTR_PDB_Release.h"
		#pragma message("include BuildConfig/DispSvrSDK_QA_NOTR_PDB_Release.h")
	#endif

	#ifdef DISPSVRSDK_QA_NOTR_DEBUG
		#include "BuildConfig/DispSvrSDK_QA_NOTR_Debug.h"
		#pragma message("include BuildConfig/DispSvrSDK_QA_NOTR_Debug.h")
	#endif
#endif	// _SDK_CONFIG_DEF_H_
