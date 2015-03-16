#ifndef _TRSDK_H
#define _TRSDK_H

#include <windows.h>
#include <stdio.h>
#include "BaseTR\IVITrWinDef.h"
#include "BaseTR\IVITRUtil.h"
#include "BaseTR\IVICallStackVerify.h" // keep iviTR_INIT_DETECT_CALLER_VALIDATION, iviTR_VERIFY_CALLER_REGISTER always exist

#define TR_TREXE_ENABLE
//--------------------------------------------------------------------------------
//To enable the TR on your build, uncomment out the following lines:
//#define TR_ENABLE_NEWMACROS
//--------------------------------------------------------------------------------

// add the advanced tamper resistance code
#if (defined(TR_ENABLE_NEWMACROS) || defined(TR_ENABLE_NEWMACROS_WITHOUT_ANTIDEBUG))
#include "BaseTR.h"

#ifndef TR_TRACE_ENABLE
#pragma message("[TRSDK] ++++++++++++++ TR macro is turned ON ++++++++++++++ >>> TR_TRACE_ENABLE OFF <<<")
#else
#pragma message("[TRSDK] ++++++++++++++ TR macro is turned ON ++++++++++++++ >>> TR_TRACE_ENABLE ON  <<<")
#endif

NS_TRLIB_USING

//--------------------------------------------------------------------------------
// For partial disable AntiDebug request, if you wish to DISABLE AntiDebug, UNCOMMENT the following lines:
	#ifdef TR_ENABLE_NEWMACROS_WITHOUT_ANTIDEBUG

    #pragma message("[TRSDK] ++++++++++++++ TR macro for anti-debugger is turned OFF ++++++++++++++")

	#include "AntiDebugOff.h"
	#endif
//--------------------------------------------------------------------------------
#else
#pragma message("[TRSDK] ++++++++++++++ TR macro is totally turned OFF ++++++++++++++")

#include "BaseTROff.h"
#endif

#if defined(TR_TREXE_ENABLE)
#pragma message("[TRSDK] ++++++++++++++ TREXE macro is turned ON ++++++++++++++")
#include "IVIScramble.h"

NS_TRLIB_USING

#else
#pragma message("[TRSDK] ++++++++++++++ TREXE macro is totally turned OFF ++++++++++++++")
#include "IVIScrambleOff.h"
#endif

#if (defined(TR_ENABLE) || defined(TR_ENABLE_NEWMACROS) || defined(TR_ENABLE_NEWMACROS_WITHOUT_ANTIDEBUG)) && !defined (__linux__)
#include "BaseTR\DetectTREnable.h"
#else
#include "DetectTREnableOff.h"
#endif

#if (defined(TR_ENABLE) || defined(TR_TREXE_ENABLE)) && !defined (__linux__)
#include "BaseTR\DetectTRExe.h"
#else
#include "DetectTRExeOff.h"
#endif

#if (defined(TR_ENABLE_NEWMACROS) || defined(TR_ENABLE_NEWMACROS_WITHOUT_ANTIDEBUG))
// Following definition used to debug TR macro -- below
#ifdef TR_DEBUG_CC_DISABLE
// IVICheckReturnAddr
#undef  iviTR_SAVE_RETURN_ADDR
#undef  iviTR_RACHECK_BEGIN
#undef  iviTR_RACHECK_END
#undef  iviTR_CHECK_RETURN_ADDR

#define iviTR_SAVE_RETURN_ADDR {}
#define iviTR_RACHECK_BEGIN(FNUM)
#define iviTR_RACHECK_END(FNUM)
#define iviTR_CHECK_RETURN_ADDR(FNUM, bool_variable) {bool_variable = true;}

#undef  iviTR_INIT_DETECT_CALLER_VALIDATION
#undef  iviTR_DETECT_CALLER_VALIDATION
#define iviTR_INIT_DETECT_CALLER_VALIDATION(sz_clsid) {}
#define iviTR_DETECT_CALLER_VALIDATION(bool_return, sz_clsid) {bool_return = false;}

// IVICallStackVerify
#undef  iviTR_VERIFY_CALLER_FRAMEPOINTER
#undef  iviTR_VERIFY_CALLER_PARAM
#define iviTR_VERIFY_CALLER_FRAMEPOINTER(pFuncAddrs, cbNumFuncAddrs, bResult) {bResult = true;}
#define iviTR_VERIFY_CALLER_PARAM(firstparam, pFuncAddrs, cbNumFuncAddrs, bResult) {bResult = true;}

#undef  iviTR_VERIFY_CALLER_REGISTER
#undef  iviTR_VERIFY_CALLER
#define iviTR_VERIFY_CALLER_REGISTER(GUID, INDEX) {}
#define iviTR_VERIFY_CALLER(bInvalidCaller, GUID) {}

//IVIDetectHookCOM - RegisterCallerHandle/DetectHookCOM(Need SQPlus)
#undef  iviTR_REGISTER_CALLER_HANDLE
#undef  iviTR_DETECT_HOOK_COM

#define iviTR_REGISTER_CALLER_HANDLE(sz_value, h_handle, byte_pattern) {}
#define iviTR_DETECT_HOOK_COM(sz_clsid, sz_path, byte_pattern, bool_return){bool_return = false;}
#endif // TR_DEBUG_CC_DISABLE

#ifdef TR_DEBUG_ANTI_MEMDUMP_DISABLE
// IVIAntiMemoryDump
#undef  iviTR_REGISTER_IMAGESIZE
#undef  iviTR_UNREGISTER_IMAGESIZE
#undef  iviTR_START_IMAGESIZE_CHECKER
#undef  iviTR_STOP_IMAGESIZE_CHECKER
#undef  iviTR_MODULE_HIDING

#define iviTR_REGISTER_IMAGESIZE(hModule) {}
#define iviTR_UNREGISTER_IMAGESIZE(hModule) {}
#define iviTR_START_IMAGESIZE_CHECKER {}
#define iviTR_STOP_IMAGESIZE_CHECKER {}
#define iviTR_MODULE_HIDING(hModule) {}
#endif // TR_DEBUG_ANTI_MEMDUMP_DISABLE

#ifdef TR_DEBUG_TIMING_CHECK_DISABLE
// AnitDebug
#undef	iviTR_GETRDTSC
#define iviTR_GETRDTSC(dword_variable) {dword_variable=0;}

// TimingMacros
#undef  iviTR_SET_TIMEOUT_COUNT
#undef  iviTR_SET_INITIAL_TIMESTAMP
#undef  iviTR_SET_FINAL_TIMESTAMP
#undef  iviTR_TIMESTAMP_CHECK

#define iviTR_SET_TIMEOUT_COUNT(int_variable) {}
#define iviTR_SET_INITIAL_TIMESTAMP(t1) {}
#define iviTR_SET_FINAL_TIMESTAMP(t2) {}
#define iviTR_TIMESTAMP_CHECK(t1, t2, dword_variable, bool_variable) {bool_variable = false;}
#endif // TR_DEBUG_TIMING_CHECK_DISABLE
// Following definition used to debug TR macro -- above
#endif

#endif