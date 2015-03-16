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

// #define iviTR_ENABLE_MULTI_THREADED_REGION
// #define iviTR_ENABLE_ANTIDEBUG
// #define iviTR_ENABLE_MEMORY_INTEGRITY_CHECKING
// #define iviTR_ENABLE_CHECK_MACRO_NAMES

#ifdef _WIN32
	// the following is a 3rd party file used for morphing code
    #include "BaseTR\EXECryptor.h"
    #include "BaseTR\ThemidaSDK.h"
#endif // _WIN32

#include "BaseTR\Injector.h"
#include "BaseTR\IVIAntiDebug.h"
#include "BaseTR\IVIAntiDebug2.h"
#include "BaseTR\IVIAntiDisassemble.h"

#include "BaseTR\IVIAntiPrintScreen.h"

#include "BaseTR\IVIBreakpointDetect.h"
#include "BaseTR\IVICheck.h"
#include "BaseTR\IVICheckProcessInfo.h"
#include "BaseTR\IVICheckProcessMemory.h"

#include "BaseTR\IVICheckWindowNames.h"
#include "BaseTR\IVIMultiThreadedRegion.h"
#include "BaseTR\IVIHibernateDetect.h"
#include "BaseTR\IVIKernelUserDebuggerCofusion.h"

#include "BaseTR\IVIOsFunctionCallHackingDetect.h"

#include "BaseTR\IVIVirtualMachineDetect.h"

#include "BaseTR\TimingMacros.h"
#include "BaseTR\DelayedInstability.h"
#include "BaseTR\DebugDetectExcept.h"

#include "BaseTR\IVICallStackVerify.h"
#include "BaseTR\IVICheckReturnAddr.h"
#include "BaseTR\IVIDetectHookCom.h"
#include "BaseTR\IVICheckVtableHook.h"
#include "BaseTR\IVIDetectDebuggerAttach.h"
#include "BaseTR\IVIVerifyCheckSumRegion.h"
#include "BaseTR\IVIDummyAccess.h"

#include "BaseTR\IVIAntiMemoryDump.h"

#include "BaseTR\IVIDummyCall.h"

#endif //_BASETR_H_
