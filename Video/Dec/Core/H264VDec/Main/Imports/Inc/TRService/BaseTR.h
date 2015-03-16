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


#ifndef _BASETR_H_
#define _BASETR_H_

#define iviTR_MORPH_BEGIN_0 CRYPT_START
#define iviTR_MORPH_BEGIN_1 CRYPT_START
#define iviTR_MORPH_BEGIN_2 CRYPT_START
#define iviTR_MORPH_BEGIN_3 CRYPT_START
#define iviTR_MORPH_BEGIN_4 CRYPT_START
#define iviTR_MORPH_BEGIN_5 CRYPT_START
#define iviTR_MORPH_BEGIN_6 CRYPT_START
#define iviTR_MORPH_BEGIN_7 CRYPT_START
#define iviTR_MORPH_BEGIN_8 CRYPT_START
#define iviTR_MORPH_BEGIN_9 CRYPT_START

#define iviTR_MORPH_END_0 CRYPT_END
#define iviTR_MORPH_END_1 CRYPT_END
#define iviTR_MORPH_END_2 CRYPT_END
#define iviTR_MORPH_END_3 CRYPT_END
#define iviTR_MORPH_END_4 CRYPT_END
#define iviTR_MORPH_END_5 CRYPT_END
#define iviTR_MORPH_END_6 CRYPT_END
#define iviTR_MORPH_END_7 CRYPT_END
#define iviTR_MORPH_END_8 CRYPT_END
#define iviTR_MORPH_END_9 CRYPT_END

// adopt 3rd-party VM protection (refer to ThemidaSDK.h)
#define iviTR_VIRTUAL_MACHINE_BEGIN	VM_START
#define iviTR_VIRTUAL_MACHINE_END	VM_END

#define iviTR_ENABLE_MULTI_THREADED_REGION
#define iviTR_ENABLE_ANTIDEBUG
#define iviTR_ENABLE_MEMORY_INTEGRITY_CHECKING
#define iviTR_ENABLE_CHECK_MACRO_NAMES

#ifdef _WIN32
	// the following is a 3rd party file used for morphing code
    #include "EXECryptor.h"
    #include "ThemidaSDK.h"
#endif // _WIN32

#include "IVIAntiDebug.h"
#include "IVIAntiDebug2.h"
#include "IVIAntiDisassemble.h"

#include "IVIAntiPrintScreen.h"

#include "IVIBreakpointDetect.h"
#include "IVICheck.h"
#include "IVICheckProcessInfo.h"
#include "IVICheckProcessMemory.h"

#include "IVICheckWindowNames.h"
#include "IVIMultiThreadedRegion.h"
#include "IVIHibernateDetect.h"
#include "IVIKernelUserDebuggerCofusion.h"

#include "IVIOsFunctionCallHackingDetect.h"

#include "IVIVirtualMachineDetect.h"

#include "TimingMacros.h"
#include "DelayedInstability.h"
#include "DebugDetectExcept.h"

#include "IVICallStackVerify.h"
#include "IVICheckReturnAddr.h"
#include "IVIDetectHookCom.h"
#include "IVICheckVtableHook.h"
#include "IVIDetectDebuggerAttach.h"
#include "IVIVerifyCheckSumRegion.h"
#include "IVIDummyAccess.h"
#include "IVIAntiMemoryDump.h"

#include "IVIDummyCall.h"

// Added by Johnny (2007/03/14)
// Temporarily mark the following macros empty
// IVICallStackVerify
#undef iviTR_VERIFY_CALLER_FRAMEPOINTER
#define iviTR_VERIFY_CALLER_FRAMEPOINTER(pFuncAddrs, cbNumFuncAddrs, bResult) {bResult = true;}

#undef iviTR_VERIFY_CALLER_PARAM
#define iviTR_VERIFY_CALLER_PARAM(firstparam, pFuncAddrs, cbNumFuncAddrs, bResult) {bResult = true;}

#endif //_BASETR_H_
