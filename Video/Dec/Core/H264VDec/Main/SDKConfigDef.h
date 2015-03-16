#ifndef _SDK_CONFIG_DEF_H_
#define _SDK_CONFIG_DEF_H_
	//////////////////////////////////////////////////////////////////
	// All of code for definition MUST include this header.
	// The header include all of macro header for all products.
	//
	// #define SET_DEF_HERE

	#ifdef VIDEOH264VDECSDK_QA_NOISMP_TR_RELEASE
		#include "BuildConfig/VideoH264VDecSDK_QA_NoISMP_TR_Release.h"
		#pragma message("include BuildConfig/VideoH264VDecSDK_QA_NoISMP_TR_Release.h")
	#endif

	#ifdef VIDEOH264VDECSDK_QA_NOISMP_TRSDK_V1X_RELEASE
		#include "BuildConfig/VideoH264VDecSDK_QA_NoISMP_TRSDK_V1X_Release.h"
		#pragma message("include BuildConfig/VideoH264VDecSDK_QA_NoISMP_TRSDK_V1X_Release.h")
	#endif

	#ifdef VIDEOH264VDECSDK_QA_NOISMP_NOTR_RELEASE
		#include "BuildConfig/VideoH264VDecSDK_QA_NoISMP_NOTR_Release.h"
		#pragma message("include BuildConfig/VideoH264VDecSDK_QA_NoISMP_NOTR_Release.h")
	#endif

	#ifdef VIDEOH264VDECSDK_QA_ISMP_TR_RELEASE
		#include "BuildConfig/VideoH264VDecSDK_QA_ISMP_TR_Release.h"
		#pragma message("include BuildConfig/VideoH264VDecSDK_QA_ISMP_TR_Release.h")
	#endif

	#ifdef VIDEOH264VDECSDK_QA_ISMP_TRSDK_V1X_RELEASE
		#include "BuildConfig/VideoH264VDecSDK_QA_ISMP_TRSDK_V1X_Release.h"
		#pragma message("include BuildConfig/VideoH264VDecSDK_QA_ISMP_TRSDK_V1X_Release.h")
	#endif

	#ifdef VIDEOH264VDECSDK_QA_ISMP_NOTR_RELEASE
		#include "BuildConfig/VideoH264VDecSDK_QA_ISMP_NOTR_Release.h"
		#pragma message("include BuildConfig/VideoH264VDecSDK_QA_ISMP_NOTR_Release.h")
	#endif

	#ifdef VIDEOH264VDECSDK_QA_NOTR_PDB_RELEASE
		#include "BuildConfig/VideoH264VDecSDK_QA_NOTR_PDB_Release.h"
		#pragma message("include BuildConfig/VideoH264VDecSDK_QA_NOTR_PDB_Release.h")
	#endif

	#ifdef VIDEOH264VDECSDK_QA_ISMP_NOTR_DEBUG
		#include "BuildConfig/VideoH264VDecSDK_QA_ISMP_NOTR_Debug.h"
		#pragma message("include BuildConfig/VideoH264VDecSDK_QA_ISMP_NOTR_Debug.h")
	#endif
#endif	// _SDK_CONFIG_DEF_H_
