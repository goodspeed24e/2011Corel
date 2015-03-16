#ifndef _LIBTR_H
#define _LIBTR_H

#include "IVITRUtil.h"
#include "IVICallStackVerify.h" // keep iviTR_INIT_DETECT_CALLER_VALIDATION, iviTR_VERIFY_CALLER_REGISTER always exist
#include "tr.h"

//--------------------------------------------------------------------------------
//To enable the TR on your build, uncomment out the following lines:
//#define TR_ENABLE_NEWMACROS
//--------------------------------------------------------------------------------

// add the advanced tamper resistance code
#if (defined(TR_ENABLE_NEWMACROS) || defined(TR_ENABLE_NEWMACROS_WITHOUT_ANTIDEBUG))
#include "BaseTR.h"

#ifndef TR_TRACE_ENABLE
#pragma message("++++++++++++++ TR macro is turned ON ++++++++++++++ >>> TR_TRACE_ENABLE OFF <<<")
#else
#pragma message("++++++++++++++ TR macro is turned ON ++++++++++++++ >>> TR_TRACE_ENABLE ON  <<<")
#endif

//--------------------------------------------------------------------------------
// For partial disable AntiDebug request, if you wish to DISABLE AntiDebug, UNCOMMENT the following lines:
	#ifdef TR_ENABLE_NEWMACROS_WITHOUT_ANTIDEBUG

    #pragma message("++++++++++++++ TR macro for anti-debugger is turned OFF ++++++++++++++")

	#include "AntiDebugOff.h"
	#endif
//--------------------------------------------------------------------------------

//--------------------------------------------------------------------------------
//if you wish to DISABLE checking of macro names, UNCOMMENT the following lines:
    // because iviTR_CHECK_ALL_PROCESS_NAMES can not work correctly in WinDVD (incorrect detection), I temporarily disable
    // all macro name checking functions ! It will disable iviTR_CHECK_ALL_PROCESS_NAMES/iviTR_CHECK_ALL_MODULE_NAMES/iviTR_CHECK_ALL_WINDOW_NAMES in WinDVD !
	#ifdef iviTR_ENABLE_CHECK_MACRO_NAMES
	#undef iviTR_ENABLE_CHECK_MACRO_NAMES
	#endif
//--------------------------------------------------------------------------------

//--------------------------------------------------------------------------------
//if you wish to DISABLE timing macros, UNCOMMENT the following line:
	//#include "TimingMacrosOff.h"
//--------------------------------------------------------------------------------

//--------------------------------------------------------------------------------
//if you wish to USE the debugger WITH TR ENABLED, UNCOMMENT the following lines:
	//#ifdef iviTR_ENABLE_ANTIDEBUG
	//#undef iviTR_ENABLE_ANTIDEBUG
	//#endif
//--------------------------------------------------------------------------------

//--------------------------------------------------------------------------------
//if you wish to DISABLE the MultiThreadedRegion, UNCOMMENT the following lines:
	#ifdef iviTR_ENABLE_MULTI_THREADED_REGION
	#undef iviTR_ENABLE_MULTI_THREADED_REGION
	#endif
//--------------------------------------------------------------------------------

//--------------------------------------------------------------------------------
//if you wish to DISABLE the Memory Integrity Checking, UNCOMMENT the following lines:
    // because iviTR_CREATE_CHECK_PROCESS_MEMORY_THREAD/iviTR_DELETE_CHECK_PROCESS_MEMORY_THREAD can not work correctly in WinDVD8 (fail to playback), I temporarily disable
    // all memory integrity checking functions ! It will disable iviTR_CREATE_CHECK_PROCESS_MEMORY_THREAD/iviTR_DELETE_CHECK_PROCESS_MEMORY_THREAD in WinDVD8 !
    // (please refer to WinDVD8 bug3379)
	#ifdef iviTR_ENABLE_MEMORY_INTEGRITY_CHECKING
	#undef iviTR_ENABLE_MEMORY_INTEGRITY_CHECKING
	#endif
//--------------------------------------------------------------------------------
#else
#pragma message("++++++++++++++ TR macro is totally turned OFF ++++++++++++++")

#include "BaseTROff.h"
#endif


#if (defined(TR_ENABLE) || defined(TR_ENABLE_NEWMACROS) || defined(TR_ENABLE_NEWMACROS_WITHOUT_ANTIDEBUG)) && !defined (__linux__)
#include "DetectTREnable.h"
#else
#include "DetectTREnableOff.h"
#endif

#if (defined(TR_ENABLE) || defined(TR_TREXE_ENABLE)) && !defined (__linux__)
#include "DetectTRExe.h"
#else
#include "DetectTRExeOff.h"
#endif

// if you wish to DISABLE specific macros, uncomment the lines below accordingly
/*
#ifdef	iviTR_NULL_PAD
#undef	iviTR_NULL_PAD
#endif
#define iviTR_NULL_PAD

#ifdef	iviTR_MISALIGNMENT
#undef	iviTR_MISALIGNMENT
#endif
#define iviTR_MISALIGNMENT(ll, reg, pad)

#ifdef	iviTR_MISALIGNMENT_1
#undef	iviTR_MISALIGNMENT_1
#endif
#define iviTR_MISALIGNMENT_1(ll)

#ifdef	iviTR_MISALIGNMENT_2
#undef	iviTR_MISALIGNMENT_2
#endif
#define iviTR_MISALIGNMENT_2(ll)

#ifdef	iviTR_FALSE_RETURN
#undef	iviTR_FALSE_RETURN
#endif
#define iviTR_FALSE_RETURN(ll, reg, pad)

#ifdef	iviTR_FALSE_RETURN_1
#undef	iviTR_FALSE_RETURN_1
#endif
#define iviTR_FALSE_RETURN_1(ll)

#ifdef	iviTR_FALSE_RETURN_2
#undef	iviTR_FALSE_RETURN_2
#endif
#define iviTR_FALSE_RETURN_2(ll)

#ifdef	iviTR_FALSE_CALL
#undef	iviTR_FALSE_CALL
#endif
#define iviTR_FALSE_CALL(ll, reg, pad)

#ifdef	iviTR_FALSE_CALL_1
#undef	iviTR_FALSE_CALL_1
#endif
#define iviTR_FALSE_CALL_1(ll)

#ifdef	iviTR_FIND_INT_3H_UNHANDLEDEXCEPTIONFILTER
#undef	iviTR_FIND_INT_3H_UNHANDLEDEXCEPTIONFILTER
#endif
#define iviTR_FIND_INT_3H_UNHANDLEDEXCEPTIONFILTER(dword_variable)

#ifdef	iviTR_DETECT_TRACER_TRAP_FLAG
#undef	iviTR_DETECT_TRACER_TRAP_FLAG
#endif
#define iviTR_DETECT_TRACER_TRAP_FLAG(dword_variable)

#ifdef	iviTR_INT3_EAX_CHECK
#undef	iviTR_INT3_EAX_CHECK
#endif
#define iviTR_INT3_EAX_CHECK(dword_variable)

#ifdef	iviTR_DAEMON_FINDER
#undef	iviTR_DAEMON_FINDER
#endif
#define iviTR_DAEMON_FINDER(bool_variable)

#ifdef	iviTR_INT1H_CHECK
#undef	iviTR_INT1H_CHECK
#endif
#define iviTR_INT1H_CHECK(bool_variable)

#ifdef	iviTR_API_ISDEBUGGERPRESENT_CHECK
#undef	iviTR_API_ISDEBUGGERPRESENT_CHECK
#endif
#define iviTR_API_ISDEBUGGERPRESENT_CHECK(bool_variable)

#ifdef	iviTR_LOCK_CMPXCHG8B
#undef	iviTR_LOCK_CMPXCHG8B
#endif
#define iviTR_LOCK_CMPXCHG8B()

#ifdef	iviTR_SOFTWARE_NUMEGA_SOFTICE_REGISTRY_CHECK
#undef	iviTR_SOFTWARE_NUMEGA_SOFTICE_REGISTRY_CHECK
#endif
#define iviTR_SOFTWARE_NUMEGA_SOFTICE_REGISTRY_CHECK(bool_variable)

#ifdef	iviTR_SOFTWARE_SYSERSOFT_SYSER_REGISTRY_CHECK1
#undef	iviTR_SOFTWARE_SYSERSOFT_SYSER_REGISTRY_CHECK1
#endif
#define iviTR_SOFTWARE_SYSERSOFT_SYSER_REGISTRY_CHECK1(bool_variable)

#ifdef	iviTR_SOFTWARE_SYSERSOFT_SYSER_REGISTRY_CHECK2
#undef	iviTR_SOFTWARE_SYSERSOFT_SYSER_REGISTRY_CHECK2
#endif
#define iviTR_SOFTWARE_SYSERSOFT_SYSER_REGISTRY_CHECK2(bool_variable)

#ifdef	iviTR_SOFTWARE_SYSERSOFT_SYSER_REGISTRY_CHECK3
#undef	iviTR_SOFTWARE_SYSERSOFT_SYSER_REGISTRY_CHECK3
#endif
#define iviTR_SOFTWARE_SYSERSOFT_SYSER_REGISTRY_CHECK3(bool_variable)

#ifdef	iviTR_SOFTWARE_SYSERSOFT_SYSER_REGISTRY_CHECK4
#undef	iviTR_SOFTWARE_SYSERSOFT_SYSER_REGISTRY_CHECK4
#endif
#define iviTR_SOFTWARE_SYSERSOFT_SYSER_REGISTRY_CHECK4(bool_variable)

#ifdef	iviTR_SOFTWARE_SYSERSOFT_SYSER_REGISTRY_CHECK5
#undef	iviTR_SOFTWARE_SYSERSOFT_SYSER_REGISTRY_CHECK5
#endif
#define iviTR_SOFTWARE_SYSERSOFT_SYSER_REGISTRY_CHECK5(bool_variable)

#ifdef	iviTR_DEBUGBREAK_CHECK
#undef	iviTR_DEBUGBREAK_CHECK
#endif
#define iviTR_DEBUGBREAK_CHECK(bool_variable)

#ifdef	iviTR_SOFTWARE_WINDBG_REGISTRY_CHECK
#undef	iviTR_SOFTWARE_WINDBG_REGISTRY_CHECK
#endif
#define	iviTR_SOFTWARE_WINDBG_REGISTRY_CHECK(bool_variable)

#ifdef	iviTR_VERIFY_WINDBG_KERNEL_MODE
#undef	iviTR_VERIFY_WINDBG_KERNEL_MODE
#endif
#define iviTR_VERIFY_WINDBG_KERNEL_MODE(bool_variable)

//Hibernate Detection Macros
#ifdef	iviTR_CREATE_HIBERNATE_DETECT_WINDOW
#undef	iviTR_CREATE_HIBERNATE_DETECT_WINDOW
#endif
#define iviTR_CREATE_HIBERNATE_DETECT_WINDOW(instance_variable)

#ifdef	iviTR_HIBERNATE_DETECTED
#undef	iviTR_HIBERNATE_DETECTED
#endif
#define iviTR_HIBERNATE_DETECTED(bool_variable)

#ifdef	iviTR_HIBERNATE_RESET
#undef	iviTR_HIBERNATE_RESET
#endif
#define iviTR_HIBERNATE_RESET()

//This one probably shouldn't be used
#ifdef	iviTR_POLL_FOR_HIBERNATE
#undef	iviTR_POLL_FOR_HIBERNATE
#endif
#define iviTR_POLL_FOR_HIBERNATE()

//Anti Print Screen Macros
#ifdef	iviTR_DISABLE_PRINT_SCREEN
#undef	iviTR_DISABLE_PRINT_SCREEN
#endif
#define iviTR_DISABLE_PRINT_SCREEN()

#ifdef	iviTR_ENABLE_PRINT_SCREEN
#undef	iviTR_ENABLE_PRINT_SCREEN
#endif
#define iviTR_ENABLE_PRINT_SCREEN()

//Breakpoint detection macro
#ifdef	iviTR_MEMORY_ADDRESS_BREAKPOINT_DETECT
#undef	iviTR_MEMORY_ADDRESS_BREAKPOINT_DETECT
#endif
#define iviTR_MEMORY_ADDRESS_BREAKPOINT_DETECT(bool_variable, dword_variable)

// {Process Name | Window Name | Module Name} Checking
#ifdef	iviTR_CHECK_WINDOW_NAMES
#undef	iviTR_CHECK_WINDOW_NAMES
#endif
#define iviTR_CHECK_WINDOW_NAMES(match, call1, call2, parg)

#ifdef	iviTR_CHECK_PROCESS_NAMES
#undef	iviTR_CHECK_PROCESS_NAMES
#endif
#define iviTR_CHECK_PROCESS_NAMES(match, call1, call2, type, parg)

#ifdef	iviTR_CHECK_PROCESS_MODULES
#undef	iviTR_CHECK_PROCESS_MODULES
#endif
#define iviTR_CHECK_PROCESS_MODULES(match, call1, call2, type, parg)

#ifdef	iviTR_CHECK_ALL_PROCESS_NAMES
#undef	iviTR_CHECK_ALL_PROCESS_NAMES
#endif
#define iviTR_CHECK_ALL_PROCESS_NAMES(call1, call2, parg)

#ifdef	iviTR_CHECK_ALL_MODULE_NAMES
#undef	iviTR_CHECK_ALL_MODULE_NAMES
#endif
#define iviTR_CHECK_ALL_MODULE_NAMES(call1, call2, parg)

#ifdef	iviTR_CHECK_ALL_WINDOW_NAMES
#undef	iviTR_CHECK_ALL_WINDOW_NAMES
#endif
#define iviTR_CHECK_ALL_WINDOW_NAMES(call1, call2, parg)

// memory checksumming
#ifdef	iviTR_CREATE_CHECK_PROCESS_MEMORY_THREAD
#undef	iviTR_CREATE_CHECK_PROCESS_MEMORY_THREAD
#endif
#define iviTR_CREATE_CHECK_PROCESS_MEMORY_THREAD(timer, s_addr, e_addr, call1, call2, parg, event)

//Code Timing Macros
#ifdef	iviTR_SET_TIMEOUT_COUNT
#undef	iviTR_SET_TIMEOUT_COUNT
#endif
#define iviTR_SET_TIMEOUT_COUNT(int_variable)

#ifdef	iviTR_SET_INITIAL_TIMESTAMP
#undef	iviTR_SET_INITIAL_TIMESTAMP
#endif
#define iviTR_SET_INITIAL_TIMESTAMP(t1)

#ifdef	iviTR_SET_FINAL_TIMESTAMP
#undef	iviTR_SET_FINAL_TIMESTAMP
#endif
#define iviTR_SET_FINAL_TIMESTAMP(t2)

#ifdef	iviTR_TIMESTAMP_CHECK
#undef	iviTR_TIMESTAMP_CHECK
#endif
#define iviTR_TIMESTAMP_CHECK(t1, t2, dword_variable, bool_variable)

//Debug Register checks
#ifdef	iviTR_DEBUG_REGISTER_CHECK
#undef	iviTR_DEBUG_REGISTER_CHECK
#endif
#define iviTR_DEBUG_REGISTER_CHECK(bool_variable)

// IVIAntiDebug
#ifdef	iviTR_VERIFY_SOFTICE
#undef	iviTR_VERIFY_SOFTICE
#endif
#define iviTR_VERIFY_SOFTICE(proc)
	
#ifdef	iviTR_VERIFY_ATTACHED_DEBUGGER
#undef	iviTR_VERIFY_ATTACHED_DEBUGGER
#endif
#define iviTR_VERIFY_ATTACHED_DEBUGGER(proc)

#ifdef	iviTR_VERIFY_VTUNE_30
#undef	iviTR_VERIFY_VTUNE_30
#endif
#define iviTR_VERIFY_VTUNE_30(proc)

#ifdef	iviTR_HALT_THIS_PROCESS
#undef	iviTR_HALT_THIS_PROCESS
#endif
#define iviTR_HALT_THIS_PROCESS(x) ExitProcess(0)

#ifdef	iviTR_CHECK_MICROSOFT_PIN
#undef	iviTR_CHECK_MICROSOFT_PIN
#endif
#define iviTR_CHECK_MICROSOFT_PIN(testpin, flag) {(flag)=0;}

#ifdef	iviTR_CHECK_THREAD_TIMER
#undef	iviTR_CHECK_THREAD_TIMER
#endif
#define iviTR_CHECK_THREAD_TIMER(timeout_sec,proc)

#ifdef	iviTR_READ_RETURN_ADDR
#undef	iviTR_READ_RETURN_ADDR
#endif
#define iviTR_READ_RETURN_ADDR

#ifdef	iviTR_CHECK_RETURN_ADDR
#undef	iviTR_CHECK_RETURN_ADDR
#endif
#define iviTR_CHECK_RETURN_ADDR

#ifdef	iviTR_GET_RDTSC
#undef	iviTR_GET_RDTSC
#endif
#define iviTR_GET_RDTSC(dword_variable)

#ifdef	iviTR_HIDDEN_GET_RDTSC1
#undef	iviTR_HIDDEN_GET_RDTSC1
#endif
#define iviTR_HIDDEN_GET_RDTSC1(linenumber, dword_variable)

#ifdef	iviTR_HIDDEN_GET_RDTSC2
#undef	iviTR_HIDDEN_GET_RDTSC2
#endif
#define iviTR_HIDDEN_GET_RDTSC2(linenumber, dword_variable)

#ifdef	iviTR_HIDDEN_GET_RDTSC3
#undef	iviTR_HIDDEN_GET_RDTSC3
#endif
#define iviTR_HIDDEN_GET_RDTSC3(linenumber, dword_variable)

#ifdef	iviTR_HIDDEN_GET_RDTSC4
#undef	iviTR_HIDDEN_GET_RDTSC4
#endif
#define iviTR_HIDDEN_GET_RDTSC4(linenumber, dword_variable)

#ifdef	iviTR_GETRDTSC
#undef	iviTR_GETRDTSC
#endif
#define iviTR_GETRDTSC(var)

// IVIAntiDebug2
#ifdef	iviTR_VERIFY_SOFTICE_SIWVIDSTART_A
#undef	iviTR_VERIFY_SOFTICE_SIWVIDSTART_A
#endif
#define iviTR_VERIFY_SOFTICE_SIWVIDSTART_A(bool_variable){}

#ifdef	iviTR_VERIFY_SOFTICE_SIWVIDSTART_DD_A
#undef	iviTR_VERIFY_SOFTICE_SIWVIDSTART_DD_A
#endif
#define iviTR_VERIFY_SOFTICE_SIWVIDSTART_DD_A(bool_variable){}

#ifdef	iviTR_VERIFY_SOFTICE_SICE_A
#undef	iviTR_VERIFY_SOFTICE_SICE_A
#endif
#define iviTR_VERIFY_SOFTICE_SICE_A(bool_variable){}

#ifdef	iviTR_VERIFY_SOFTICE_SICE_DD_A
#undef	iviTR_VERIFY_SOFTICE_SICE_DD_A
#endif
#define iviTR_VERIFY_SOFTICE_SICE_DD_A(bool_variable){}

#ifdef	iviTR_VERIFY_SOFTICE_NTICE_A
#undef	iviTR_VERIFY_SOFTICE_NTICE_A
#endif
#define iviTR_VERIFY_SOFTICE_NTICE_A(bool_variable){}

#ifdef	iviTR_VERIFY_SOFTICE_NTICE_DD_A
#undef	iviTR_VERIFY_SOFTICE_NTICE_DD_A
#endif
#define iviTR_VERIFY_SOFTICE_NTICE_DD_A(bool_variable){}

#ifdef	iviTR_VERIFY_SOFTICE_SIWVID_A
#undef	iviTR_VERIFY_SOFTICE_SIWVID_A
#endif
#define iviTR_VERIFY_SOFTICE_SIWVID_A(bool_variable){}

#ifdef	iviTR_VERIFY_SOFTICE_SIWVID_DD_A
#undef	iviTR_VERIFY_SOFTICE_SIWVID_DD_A
#endif
#define iviTR_VERIFY_SOFTICE_SIWVID_DD_A(bool_variable){}

#ifdef	iviTR_VERIFY_SOFTICE_SIWDEBUG_A
#undef	iviTR_VERIFY_SOFTICE_SIWDEBUG_A
#endif
#define iviTR_VERIFY_SOFTICE_SIWDEBUG_A(bool_variable){}

#ifdef	iviTR_VERIFY_SOFTICE_SIWDEBUG_DD_A
#undef	iviTR_VERIFY_SOFTICE_SIWDEBUG_DD_A
#endif
#define iviTR_VERIFY_SOFTICE_SIWDEBUG_DD_A(bool_variable){}

#ifdef	iviTR_VERIFY_SOFTICE_SIWVIDSTART_A_INV
#undef	iviTR_VERIFY_SOFTICE_SIWVIDSTART_A_INV
#endif
#define iviTR_VERIFY_SOFTICE_SIWVIDSTART_A_INV(bool_variable){}

#ifdef	iviTR_VERIFY_SOFTICE_SIWVIDSTART_DD_A_INV
#undef	iviTR_VERIFY_SOFTICE_SIWVIDSTART_DD_A_INV
#endif
#define iviTR_VERIFY_SOFTICE_SIWVIDSTART_DD_A_INV(bool_variable){}

#ifdef	iviTR_VERIFY_SOFTICE_SICE_A_INV
#undef	iviTR_VERIFY_SOFTICE_SICE_A_INV
#endif
#define iviTR_VERIFY_SOFTICE_SICE_A_INV(bool_variable){}

#ifdef	iviTR_VERIFY_SOFTICE_SICE_DD_A_INV
#undef	iviTR_VERIFY_SOFTICE_SICE_DD_A_INV
#endif
#define iviTR_VERIFY_SOFTICE_SICE_DD_A_INV(bool_variable){}

#ifdef	iviTR_VERIFY_SOFTICE_NTICE_A_INV
#undef	iviTR_VERIFY_SOFTICE_NTICE_A_INV
#endif
#define iviTR_VERIFY_SOFTICE_NTICE_A_INV(bool_variable){}

#ifdef	iviTR_VERIFY_SOFTICE_NTICE_DD_A_INV
#undef	iviTR_VERIFY_SOFTICE_NTICE_DD_A_INV
#endif
#define iviTR_VERIFY_SOFTICE_NTICE_DD_A_INV(bool_variable){}

#ifdef	iviTR_VERIFY_SOFTICE_SIWVID_A_INV
#undef	iviTR_VERIFY_SOFTICE_SIWVID_A_INV
#endif
#define iviTR_VERIFY_SOFTICE_SIWVID_A_INV(bool_variable){}

#ifdef	iviTR_VERIFY_SOFTICE_SIWVID_DD_A_INV
#undef	iviTR_VERIFY_SOFTICE_SIWVID_DD_A_INV
#endif
#define iviTR_VERIFY_SOFTICE_SIWVID_DD_A_INV(bool_variable){}

#ifdef	iviTR_VERIFY_SOFTICE_SIWDEBUG_A_INV
#undef	iviTR_VERIFY_SOFTICE_SIWDEBUG_A_INV
#endif
#define iviTR_VERIFY_SOFTICE_SIWDEBUG_A_INV(bool_variable){}

#ifdef	iviTR_VERIFY_SOFTICE_SIWDEBUG_DD_A_INV
#undef	iviTR_VERIFY_SOFTICE_SIWDEBUG_DD_A_INV
#endif
#define iviTR_VERIFY_SOFTICE_SIWDEBUG_DD_A_INV(bool_variable){}

#ifdef	iviTR_VERIFY_SOFTICE_SIWVIDSTART_W
#undef	iviTR_VERIFY_SOFTICE_SIWVIDSTART_W
#endif
#define iviTR_VERIFY_SOFTICE_SIWVIDSTART_W(bool_variable){}

#ifdef	iviTR_VERIFY_SOFTICE_SIWVIDSTART_DD_W
#undef	iviTR_VERIFY_SOFTICE_SIWVIDSTART_DD_W
#endif
#define iviTR_VERIFY_SOFTICE_SIWVIDSTART_DD_W(bool_variable){}

#ifdef	iviTR_VERIFY_SOFTICE_SICE_W
#undef	iviTR_VERIFY_SOFTICE_SICE_W
#endif
#define iviTR_VERIFY_SOFTICE_SICE_W(bool_variable){}

#ifdef	iviTR_VERIFY_SOFTICE_SICE_DD_W
#undef	iviTR_VERIFY_SOFTICE_SICE_DD_W
#endif
#define iviTR_VERIFY_SOFTICE_SICE_DD_W(bool_variable){}

#ifdef	iviTR_VERIFY_SOFTICE_NTICE_W
#undef	iviTR_VERIFY_SOFTICE_NTICE_W
#endif
#define iviTR_VERIFY_SOFTICE_NTICE_W(bool_variable){}

#ifdef	iviTR_VERIFY_SOFTICE_NTICE_DD_W
#undef	iviTR_VERIFY_SOFTICE_NTICE_DD_W
#endif
#define iviTR_VERIFY_SOFTICE_NTICE_DD_W(bool_variable){}

#ifdef	iviTR_VERIFY_SOFTICE_SIWVID_W
#undef	iviTR_VERIFY_SOFTICE_SIWVID_W
#endif
#define iviTR_VERIFY_SOFTICE_SIWVID_W(bool_variable){}

#ifdef	iviTR_VERIFY_SOFTICE_SIWVID_DD_W
#undef	iviTR_VERIFY_SOFTICE_SIWVID_DD_W
#endif
#define iviTR_VERIFY_SOFTICE_SIWVID_DD_W(bool_variable){}

#ifdef	iviTR_VERIFY_SOFTICE_SIWDEBUG_W
#undef	iviTR_VERIFY_SOFTICE_SIWDEBUG_W
#endif
#define iviTR_VERIFY_SOFTICE_SIWDEBUG_W(bool_variable){}

#ifdef	iviTR_VERIFY_SOFTICE_SIWDEBUG_DD_W
#undef	iviTR_VERIFY_SOFTICE_SIWDEBUG_DD_W
#endif
#define iviTR_VERIFY_SOFTICE_SIWDEBUG_DD_W(bool_variable){}

#ifdef	iviTR_VERIFY_SOFTICE_SIWVIDSTART_W_INV
#undef	iviTR_VERIFY_SOFTICE_SIWVIDSTART_W_INV
#endif
#define iviTR_VERIFY_SOFTICE_SIWVIDSTART_W_INV(bool_variable){}

#ifdef	iviTR_VERIFY_SOFTICE_SIWVIDSTART_DD_W_INV
#undef	iviTR_VERIFY_SOFTICE_SIWVIDSTART_DD_W_INV
#endif
#define iviTR_VERIFY_SOFTICE_SIWVIDSTART_DD_W_INV(bool_variable){}

#ifdef	iviTR_VERIFY_SOFTICE_SICE_W_INV
#undef	iviTR_VERIFY_SOFTICE_SICE_W_INV
#endif
#define iviTR_VERIFY_SOFTICE_SICE_W_INV(bool_variable){}

#ifdef	iviTR_VERIFY_SOFTICE_SICE_DD_W_INV
#undef	iviTR_VERIFY_SOFTICE_SICE_DD_W_INV
#endif
#define iviTR_VERIFY_SOFTICE_SICE_DD_W_INV(bool_variable){}

#ifdef	iviTR_VERIFY_SOFTICE_NTICE_W_INV
#undef	iviTR_VERIFY_SOFTICE_NTICE_W_INV
#endif
#define iviTR_VERIFY_SOFTICE_NTICE_W_INV(bool_variable){}

#ifdef	iviTR_VERIFY_SOFTICE_NTICE_DD_W_INV
#undef	iviTR_VERIFY_SOFTICE_NTICE_DD_W_INV
#endif
#define iviTR_VERIFY_SOFTICE_NTICE_DD_W_INV(bool_variable){}

#ifdef	iviTR_VERIFY_SOFTICE_SIWVID_W_INV
#undef	iviTR_VERIFY_SOFTICE_SIWVID_W_INV
#endif
#define iviTR_VERIFY_SOFTICE_SIWVID_W_INV(bool_variable){}

#ifdef	iviTR_VERIFY_SOFTICE_SIWVID_DD_W_INV
#undef	iviTR_VERIFY_SOFTICE_SIWVID_DD_W_INV
#endif
#define iviTR_VERIFY_SOFTICE_SIWVID_DD_W_INV(bool_variable){}

#ifdef	iviTR_VERIFY_SOFTICE_SIWDEBUG_W_INV
#undef	iviTR_VERIFY_SOFTICE_SIWDEBUG_W_INV
#endif
#define iviTR_VERIFY_SOFTICE_SIWDEBUG_W_INV(bool_variable){}

#ifdef	iviTR_VERIFY_SOFTICE_SIWDEBUG_DD_W_INV
#undef	iviTR_VERIFY_SOFTICE_SIWDEBUG_DD_W_INV
#endif
#define iviTR_VERIFY_SOFTICE_SIWDEBUG_DD_W_INV(bool_variable){}

#ifdef	iviTR_VERIFY_SOFTICE_LCREAT
#undef	iviTR_VERIFY_SOFTICE_LCREAT
#endif
#define iviTR_VERIFY_SOFTICE_LCREAT(bool_variable){}

#ifdef	iviTR_VERIFY_SOFTICE_LCREAT
#undef	iviTR_VERIFY_SOFTICE_LCREAT1
#endif
#define iviTR_VERIFY_SOFTICE_LCREAT1(bool_variable){}

#ifdef	iviTR_VERIFY_SOFTICE_LCREAT2
#undef	iviTR_VERIFY_SOFTICE_LCREAT2
#endif
#define iviTR_VERIFY_SOFTICE_LCREAT2(bool_variable){}

#ifdef	iviTR_VERIFY_SOFTICE_LCREAT3
#undef	iviTR_VERIFY_SOFTICE_LCREAT3
#endif
#define iviTR_VERIFY_SOFTICE_LCREAT3(bool_variable){}

#ifdef	iviTR_VERIFY_SOFTICE_LCREAT4
#undef	iviTR_VERIFY_SOFTICE_LCREAT4
#endif
#define iviTR_VERIFY_SOFTICE_LCREAT4(bool_variable){}

#ifdef	iviTR_VERIFY_SOFTICE_LCREAT_INV
#undef	iviTR_VERIFY_SOFTICE_LCREAT_INV
#endif
#define iviTR_VERIFY_SOFTICE_LCREAT_INV(bool_variable){}

#ifdef	iviTR_VERIFY_SOFTICE_LCREAT1_INV
#undef	iviTR_VERIFY_SOFTICE_LCREAT1_INV
#endif
#define iviTR_VERIFY_SOFTICE_LCREAT1_INV(bool_variable){}

#ifdef	iviTR_VERIFY_SOFTICE_LCREAT2_INV
#undef	iviTR_VERIFY_SOFTICE_LCREAT2_INV
#endif
#define iviTR_VERIFY_SOFTICE_LCREAT2_INV(bool_variable){}

#ifdef	iviTR_VERIFY_SOFTICE_LCREAT3_INV
#undef	iviTR_VERIFY_SOFTICE_LCREAT3_INV
#endif
#define iviTR_VERIFY_SOFTICE_LCREAT3_INV(bool_variable){}

#ifdef	iviTR_VERIFY_SOFTICE_LCREAT4_INV
#undef	iviTR_VERIFY_SOFTICE_LCREAT4_INV
#endif
#define iviTR_VERIFY_SOFTICE_LCREAT4_INV(bool_variable){}

#ifdef	iviTR_VERIFY_SOFTICE_LOPEN
#undef	iviTR_VERIFY_SOFTICE_LOPEN
#endif
#define iviTR_VERIFY_SOFTICE_LOPEN(bool_variable){}

#ifdef	iviTR_VERIFY_SOFTICE_LOPEN1
#undef	iviTR_VERIFY_SOFTICE_LOPEN1
#endif
#define iviTR_VERIFY_SOFTICE_LOPEN1(bool_variable){}

#ifdef	iviTR_VERIFY_SOFTICE_LOPEN2
#undef	iviTR_VERIFY_SOFTICE_LOPEN2
#endif
#define iviTR_VERIFY_SOFTICE_LOPEN2(bool_variable){}

#ifdef	iviTR_VERIFY_SOFTICE_LOPEN3
#undef	iviTR_VERIFY_SOFTICE_LOPEN3
#endif
#define iviTR_VERIFY_SOFTICE_LOPEN3(bool_variable){}

#ifdef	iviTR_VERIFY_SOFTICE_LOPEN4
#undef	iviTR_VERIFY_SOFTICE_LOPEN4
#endif
#define iviTR_VERIFY_SOFTICE_LOPEN4(bool_variable){}

#ifdef	iviTR_VERIFY_SOFTICE_LOPEN_INV
#undef	iviTR_VERIFY_SOFTICE_LOPEN_INV
#endif
#define iviTR_VERIFY_SOFTICE_LOPEN_INV(bool_variable){}

#ifdef	iviTR_VERIFY_SOFTICE_LOPEN1_INV
#undef	iviTR_VERIFY_SOFTICE_LOPEN1_INV
#endif
#define iviTR_VERIFY_SOFTICE_LOPEN1_INV(bool_variable){}

#ifdef	iviTR_VERIFY_SOFTICE_LOPEN2_INV
#undef	iviTR_VERIFY_SOFTICE_LOPEN2_INV
#endif
#define iviTR_VERIFY_SOFTICE_LOPEN2_INV(bool_variable){}

#ifdef	iviTR_VERIFY_SOFTICE_LOPEN3_INV
#undef	iviTR_VERIFY_SOFTICE_LOPEN3_INV
#endif
#define iviTR_VERIFY_SOFTICE_LOPEN3_INV(bool_variable){}

#ifdef	iviTR_VERIFY_SOFTICE_LOPEN4_INV
#undef	iviTR_VERIFY_SOFTICE_LOPEN4_INV
#endif
#define iviTR_VERIFY_SOFTICE_LOPEN4_INV(bool_variable){}

#ifdef	iviTR_VERIFY_SYSER_A
#undef	iviTR_VERIFY_SYSER_A
#endif
#define iviTR_VERIFY_SYSER_A(bool_variable){}

#ifdef	iviTR_VERIFY_SYSER_DD_A
#undef	iviTR_VERIFY_SYSER_DD_A
#endif
#define iviTR_VERIFY_SYSER_DD_A(bool_variable){}

#ifdef	iviTR_VERIFY_SYSER_W
#undef	iviTR_VERIFY_SYSER_W
#endif
#define iviTR_VERIFY_SYSER_W(bool_variable){}

#ifdef	iviTR_VERIFY_SYSER_DD_W
#undef	iviTR_VERIFY_SYSER_DD_W
#endif
#define iviTR_VERIFY_SYSER_DD_W(bool_variable){}

#ifdef	iviTR_VERIFY_SYSER_LCREAT
#undef	iviTR_VERIFY_SYSER_LCREAT
#endif
#define iviTR_VERIFY_SYSER_LCREAT(bool_variable){}

#ifdef	iviTR_VERIFY_SYSER_LOPEN
#undef	iviTR_VERIFY_SYSER_LOPEN
#endif
#define iviTR_VERIFY_SYSER_LOPEN(bool_variable){}

// IVIKernelUserDebuggerCofusion
#ifdef	iviTR_BLOCK_USER_INPUT
#undef	iviTR_BLOCK_USER_INPUT
#endif
#define iviTR_BLOCK_USER_INPUT(BOOLEAN_VAL) true

#ifdef	iviTR_CHECK_MULTIPLE_RUNNING_INSTANCE
#undef	iviTR_CHECK_MULTIPLE_RUNNING_INSTANCE
#endif
#define iviTR_CHECK_MULTIPLE_RUNNING_INSTANCE() true

// IVIOsFunctionCallHackingDetect
#ifdef	iviTR_GET_IAT_CRC32
#undef	iviTR_GET_IAT_CRC32
#endif
#define iviTR_GET_IAT_CRC32(rdwReturnedChecksum, bool_variable){}

#ifdef	iviTR_GET_IAT_CRC32_BY_MODULE_HANDLE
#undef	iviTR_GET_IAT_CRC32_BY_MODULE_HANDLE
#endif
#define iviTR_GET_IAT_CRC32_BY_MODULE_HANDLE(hModule, rdwChecksum, bool_variable){}

// IVIVirtualMachineDetect
#ifdef	iviTR_INSIDEVIRTUALPC_CHECK
#undef	iviTR_INSIDEVIRTUALPC_CHECK
#endif
#define iviTR_INSIDEVIRTUALPC_CHECK(bool_variable)

#ifdef	iviTR_INSIDEVMWARE_CHECK
#undef	iviTR_INSIDEVMWARE_CHECK
#endif
#define iviTR_INSIDEVMWARE_CHECK(bool_variable)

#ifdef	iviTR_CRASH
#undef	iviTR_CRASH
#endif

// IVICheckReturnAddr
#ifdef iviTR_SAVE_RETURN_ADDR
#undef iviTR_SAVE_RETURN_ADDR
#endif
#define iviTR_SAVE_RETURN_ADDR

#ifdef iviTR_RACHECK_BEGIN
#undef iviTR_RACHECK_BEGIN
#endif
#define iviTR_RACHECK_BEGIN(FNUM)

#ifdef iviTR_RACHECK_END
#undef iviTR_RACHECK_END
#endif
#define iviTR_RACHECK_END(FNUM)

#ifdef iviTR_CHECK_RETURN_ADDR
#undef iviTR_CHECK_RETURN_ADDR
#endif
#define iviTR_CHECK_RETURN_ADDR(FNUM, bool_variable)

#ifdef iviTR_INIT_DETECT_CALLER_VALIDATION
#undef iviTR_INIT_DETECT_CALLER_VALIDATION
#endif
#define iviTR_INIT_DETECT_CALLER_VALIDATION(sz_Name, int_index)

#ifdef iviTR_DETECT_CALLER_VALIDATION
#undef iviTR_DETECT_CALLER_VALIDATION
#endif
#define iviTR_DETECT_CALLER_VALIDATION(bool_return, sz_Name)

// IVICallStackVerify
#ifdef iviTR_VERIFY_CALLER_FRAMEPOINTER
#undef iviTR_VERIFY_CALLER_FRAMEPOINTER
#endif
#define iviTR_VERIFY_CALLER_FRAMEPOINTER

#ifdef iviTR_VERIFY_CALLER_PARAM
#undef iviTR_VERIFY_CALLER_PARAM
#endif
#define iviTR_VERIFY_CALLER_PARAM

#ifdef iviTR_VERIFY_CALLER_REGISTER
#undef iviTR_VERIFY_CALLER_REGISTER
#endif
#define iviTR_VERIFY_CALLER_REGISTER(GUID, INDEX)

#ifdef iviTR_VERIFY_CALLER
#undef iviTR_VERIFY_CALLER
#endif
#define iviTR_VERIFY_CALLER(bInvalidCaller, GUID)

// IVIAntiMemoryDump
#ifdef iviTR_REGISTER_IMAGESIZE
#undef iviTR_REGISTER_IMAGESIZE
#endif
#define iviTR_REGISTER_IMAGESIZE(hModule)

#ifdef iviTR_UNREGISTER_IMAGESIZE
#undef iviTR_UNREGISTER_IMAGESIZE
#endif
#define iviTR_UNREGISTER_IMAGESIZE(hModule)

#ifdef iviTR_START_IMAGESIZE_CHECKER
#undef iviTR_START_IMAGESIZE_CHECKER
#endif
#define iviTR_START_IMAGESIZE_CHECKER

#ifdef iviTR_STOP_IMAGESIZE_CHECKER
#undef iviTR_STOP_IMAGESIZE_CHECKER
#endif
#define iviTR_STOP_IMAGESIZE_CHECKER

// IVIDummyCall
#ifdef iviTR_DUMMY_CALL_1
	#undef iviTR_DUMMY_CALL_1
#endif
#define iviTR_DUMMY_CALL_1(Num)

#ifdef iviTR_DUMMY_CALL_2
	#undef iviTR_DUMMY_CALL_2
#endif
#define iviTR_DUMMY_CALL_2(Num)

#ifdef iviTR_DUMMY_CALL_3
	#undef iviTR_DUMMY_CALL_3
#endif
#define iviTR_DUMMY_CALL_3(Num)

#ifdef iviTR_DUMMY_CALL_4
	#undef iviTR_DUMMY_CALL_4
#endif
#define iviTR_DUMMY_CALL_4(Num)

#ifdef iviTR_DUMMY_CALL_5
	#undef iviTR_DUMMY_CALL_5
#endif
#define iviTR_DUMMY_CALL_5(Num)

#ifdef iviTR_DUMMY_CALL_6
	#undef iviTR_DUMMY_CALL_6
#endif
#define iviTR_DUMMY_CALL_6(Num)

#ifdef iviTR_DUMMY_CALL_7
	#undef iviTR_DUMMY_CALL_7
#endif
#define iviTR_DUMMY_CALL_7(Num)

#ifdef iviTR_DUMMY_CALL_8
	#undef iviTR_DUMMY_CALL_8
#endif
#define iviTR_DUMMY_CALL_8(Num)

#ifdef iviTR_DUMMY_CALL_9
	#undef iviTR_DUMMY_CALL_9
#endif
#define iviTR_DUMMY_CALL_9(Num)

#ifdef iviTR_DUMMY_CALL_10
	#undef iviTR_DUMMY_CALL_10
#endif
#define iviTR_DUMMY_CALL_10(Num)

//checksum
#ifdef iviTR_DUMP_CHECKSUM_LOG
#undef iviTR_DUMP_CHECKSUM_LOG
#endif
#define iviTR_DUMP_CHECKSUM_LOG(val, seed) {}

*/

#endif