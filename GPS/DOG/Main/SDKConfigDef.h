#ifndef _SDK_CONFIG_DEF_H_
#define _SDK_CONFIG_DEF_H_
	//////////////////////////////////////////////////////////////////
	// All of code for definition MUST include this header.
	// The header include all of macro header for all products.
	//
	#define SET_DEF_HERE
		
	#ifdef PERFIISDK_QA_RELEASE
		#include "BuildConfig/PerfIISDK_QA_Release.h"
		#pragma message("include BuildConfig/PerfIISDK_QA_Release.h")
	#endif
	
#endif	// _SDK_CONFIG_DEF_H_
