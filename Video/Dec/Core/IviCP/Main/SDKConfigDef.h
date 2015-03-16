#ifndef _SDK_CONFIG_DEF_H_
#define _SDK_CONFIG_DEF_H_
	//////////////////////////////////////////////////////////////////
	// All of code for definition MUST include this header.
	// The header include all of macro header for all products.
	//
	// #define SET_DEF_HERE

	#ifdef VIDEOIVICPSDK_QA_TR_RELEASE
		#include "BuildConfig/VideoIVICPSDK_QA_TR_Release.h"
		#pragma message("include BuildConfig/VideoIVICPSDK_QA_TR_Release.h")
	#endif
	
	#ifdef VIDEOIVICPSDK_QA_TR_DEBUG
		#include "BuildConfig/VideoIVICPSDK_QA_TR_Debug.h"
		#pragma message("include BuildConfig/VideoIVICPSDK_QA_TR_Debug.h")
	#endif

	#ifdef VIDEOIVICPSDK_QA_NOTR_RELEASE
		#include "BuildConfig/VIDEOIVICPSDK_QA_NOTR_Release.h"
		#pragma message("include BuildConfig/VIDEOIVICPSDK_QA_NOTR_Release.h")
	#endif

	#ifdef VIDEOIVICPSDK_QA_NOTR_DEBUG
		#include "BuildConfig/VIDEOIVICPSDK_QA_NOTR_Debug.h"
		#pragma message("include BuildConfig/VIDEOIVICPSDK_QA_NOTR_Release.h")
	#endif
#endif	// _SDK_CONFIG_DEF_H_
