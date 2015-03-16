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

// DebugDetectExcept.h: Detect debugger through use of structured exception handling
//
/////////////////////////////////////////////////////////////////////////////////
#if !defined(DEBUG_DETECT_EXCEPT__H__0EB7A1C3_64B7_4DB6_A9CE_F8DF5866E94B__INCLUDED_)
#define DEBUG_DETECT_EXCEPT__H__0EB7A1C3_64B7_4DB6_A9CE_F8DF5866E94B__INCLUDED_

#include "IVITrWinDef.h"
#include "IVITRComm.h"

#ifdef _WIN32

NS_TRLIB_BEGIN

#ifdef __cplusplus
	#pragma pack(push, 1)

	///*!
	// [Internal structure. Do not call it.]
	//
	// this is error handle code internal defined.
	//*/
	struct CEHCode
	{
		BYTE m_Prolog1;
		BYTE m_Prolog2[2];
		BYTE m_Prolog3[3];

		struct 
		{
			BYTE	m_Inst;
			BYTE	m_Opr1[2];
			LPDWORD m_Opr2;
		} m_CmpAddr;

		struct 
		{
			BYTE	m_Inst;
			BYTE	m_Opr1[2];
			LPDWORD m_Opr2;
		} m_RetAddr;

		BYTE m_ResolveArg1_1[3];
		BYTE m_ResolveArg2_1[3];
		BYTE m_ResolveArg3_1[6];
		BYTE m_CmpBeginAddr[3]; 
		BYTE m_Jbe[2];
		BYTE m_ResolveArg1_2[3];
		BYTE m_ResolveArg2_2[3];
		BYTE m_ResolveArg3_2[3];
		BYTE m_AddEndOffset[3];
		BYTE m_CmpEndAddr[6];
		BYTE m_Jae[2];
		BYTE m_ResolveArg1_3[3];
		BYTE m_ResolveArg2_3[3];
		BYTE m_ResolveArg3_3[3];
		BYTE m_SetRetAddr[6];
		BYTE m_SetRetVal1[3];
		BYTE m_JmpToRet[2]; 
		BYTE m_SetRetVal2[2];
		BYTE m_Epilog1[2];
		BYTE m_Epilog2;
		BYTE m_Ret[3];
	};

	#pragma pack(pop)
	///*!
	// [Internal structure. Do not call it.]
	//
	// this is thread parameter #1 internal defined.
	//*/
	struct CThdParams1
	{
		int		m_SomeMember1;
		double	m_SomeMember2;
		char	*m_pCrashPtr;
	};
	///*!
	// [Internal structure. Do not call it.]
	//
	// this is thread parameter #2 internal defined.
	//*/
	struct CThdParams2
	{
		double			m_SomeMember1;
		double			m_SomeMember2;
		double			m_SomeMember3;
		CThdParams1		*m_pTheCrasher;
	};
	///*!
	// [Internal structure. Do not call it.]
	//
	// this is thread parameter #3 internal defined.
	//*/
	struct CThdParams3
	{
		int			m_SomeMember1;
		int			(*m_pCrashFunc)(char*);
		int			m_SomeMember3;
		int			m_SomeMember4;
		CThdParams2	*m_pTheCrasher;
	};
	///*!
	// [Internal structure. Do not call it.]
	//
	// this is thread top parameter internal defined.
	//*/
	struct CThdParamsTop
	{
		int 		m_nRetVal;
		HANDLE      m_hThdSignal;
		void*       m_pEHReloc;
		void*       m_pLastExceptionFilter;
		CThdParams3	*m_pTheCrasher;
	};

	///*!
	// [Internal structure. Do not call it.]
	//
	// this is thread class, use for detect debugger.
	//*/
	class CCrasherThread
	{
	public:
		CCrasherThread(CEHCode *pehCode);
		~CCrasherThread();

		BOOL fnIsThreadCloseSignal();
		int  fnGetReturnValue();

	protected:
		CThdParams1 m_params1;
		CThdParams2 m_params2;
		CThdParams3 m_params3;
		CThdParamsTop m_paramTop;

		HANDLE m_hThd;
	};

	///*!
	// [Internal structure. Do not call it.]
	//
	// Initialize data structure.
	//
	// -# bEncKey - [IN] cipher key, assigned by one const BYTE.
	// -# pEHCODE - [IN/OUT]can not be NULL, (*pEHCODE) will return initialized value.
	// 
	// Return value - None
	//*/
	void __cdecl TR_fnInitCEHCode(const BYTE bEncKey, CEHCode *pEHCODE);

#endif // __cplusplus

NS_TRLIB_END

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	///*!
	// Initialization macro for detecting debugger
	//
	// Return value - None
	//
	// Supported platform:		All Windows
	// Performance overhead:	Low
	// Security level:			Medium
	// Usage scope:				Block scope
	//
	// Note:\n
	// Use with iviTR_DEBUG_DETECT_INIT, iviTR_DEBUG_DETECT_START, iviTR_DEBUG_DETECT_WAIT, iviTR_DEBUG_DETECT_END.
	//
	// Usage:\n
	// void Sample() 
	// {
	// 	 iviTR_DEBUG_DETECT_INIT
	// 		iviTR_DEBUG_DETECT_START
	//
	//		// put real working code here
	//		// and expect that the debug detection will be complete
	//		// before we get to the iviTR_DEBUG_DETECT_WAIT loop.
	//
	// 		iviTR_DEBUG_DETECT_WAIT(50);
	//
	// 	BOOL bDebuggerDetected = iviTR_DEBUG_DETECT_RES;
	// 	// do something heinous (but untraceable)
	//
	//	iviTR_DEBUG_DETECT_END
	// }
	//*/
#ifdef __cplusplus
	#define iviTR_DEBUG_DETECT_INIT \
	const BYTE kKey1 = 0xab; \
	CEHCode ehCode; \
	TR_fnInitCEHCode(kKey1, &ehCode);
#else
	#define iviTR_DEBUG_DETECT_INIT 
#endif // __cplusplus

	///*!
	// Start macro for detecting debugger
	//
	// Return value - None
	// 
	// Supported platform:		All Windows
	// Performance overhead:	Medium
	// Security level:			Medium
	// Usage scope:				Block scope
	//
	// Note:\n
	// Use with iviTR_DEBUG_DETECT_INIT, iviTR_DEBUG_DETECT_END, iviTR_DEBUG_DETECT_WAIT, iviTR_DEBUG_DETECT_RES.
	//
	// Usage:\n
	// void Sample() 
	// {
	// 	 iviTR_DEBUG_DETECT_INIT
	// 		iviTR_DEBUG_DETECT_START
	//
	//		// put real working code here
	//		// and expect that the debug detection will be complete
	//		// before we get to the iviTR_DEBUG_DETECT_WAIT loop.
	//
	// 		iviTR_DEBUG_DETECT_WAIT(50);
	//
	// 	BOOL bDebuggerDetected = iviTR_DEBUG_DETECT_RES;
	// 	// do something heinous (but untraceable)
	//
	//	iviTR_DEBUG_DETECT_END
	// }
	//*/
#ifdef __cplusplus
	#define iviTR_DEBUG_DETECT_START \
		CCrasherThread *pcrasherthd = new CCrasherThread(&ehCode);
#else
	#define iviTR_DEBUG_DETECT_START  
#endif // __cplusplus


	///*!
	// Waiting macro for detecting debugger
	//
	// Return value - None
	//
	// -#dwWaitMilliSec - [IN] the wait time for detecting debugger thread, in milli-seconds.
	//
	// Supported platform:		All Windows
	// Performance overhead:	Low
	// Security level:			Medium
	// Usage scope:				Block scope
	//
	// Note:\n
	// Use with iviTR_DEBUG_DETECT_INIT, iviTR_DEBUG_DETECT_START, iviTR_DEBUG_DETECT_END, iviTR_DEBUG_DETECT_RES.
	// please close to iviTR_DEBUG_DETECT_INIT.
	//
	// Usage:\n
	// void Sample() 
	// {
	// 	 iviTR_DEBUG_DETECT_INIT
	// 		iviTR_DEBUG_DETECT_START
	//
	//		// put real working code here
	//		// and expect that the debug detection will be complete
	//		// before we get to the iviTR_DEBUG_DETECT_WAIT loop.
	//
	// 		iviTR_DEBUG_DETECT_WAIT(50);
	//
	// 	BOOL bDebuggerDetected = iviTR_DEBUG_DETECT_RES;
	// 	// do something heinous (but untraceable)
	//
	//	iviTR_DEBUG_DETECT_END
	// }
	//*/
#ifdef __cplusplus
	#define iviTR_DEBUG_DETECT_WAIT(dwWaitMilliSec) \
		while (!pcrasherthd->fnIsThreadCloseSignal()) \
		{ \
			::Sleep(dwWaitMilliSec); \
		}
#else	
	#define iviTR_DEBUG_DETECT_WAIT(dwWaitMilliSec) 
#endif	



	///*!
	// Get result macro for detecting debugger
	//
	// Return value - [BOOL] TRUE is debugger exist, or not.
	// 
	// Supported platform:		All Windows
	// Performance overhead:	Low
	// Security level:			Medium
	// Usage scope:				Macro scope
	//
	// Note:\n
	// Use with iviTR_DEBUG_DETECT_INIT, iviTR_DEBUG_DETECT_START, iviTR_DEBUG_DETECT_WAIT, iviTR_DEBUG_DETECT_END.
	//
	// Usage:\n
	// void Sample() 
	// {
	// 	 iviTR_DEBUG_DETECT_INIT
	// 		iviTR_DEBUG_DETECT_START
	//
	//		// put real working code here
	//		// and expect that the debug detection will be complete
	//		// before we get to the iviTR_DEBUG_DETECT_WAIT loop.
	//
	// 		iviTR_DEBUG_DETECT_WAIT(50);
	//
	// 	BOOL bDebuggerDetected = iviTR_DEBUG_DETECT_RES;
	// 	// do something heinous (but untraceable)
	//
	//	iviTR_DEBUG_DETECT_END
	// }
	//*/
#ifdef __cplusplus
	#define iviTR_DEBUG_DETECT_RES (-1 != pcrasherthd->fnGetReturnValue())
#else	
	#define iviTR_DEBUG_DETECT_RES 0 
#endif	

	///*!
	// End macro for detecting debugger
	//
	// Return value - None
	// 
	// Supported platform:		All Windows
	// Performance overhead:	Medium
	// Security level:			Medium
	// Usage scope:				Block scope
	//
	// Note:\n
	//
	// Use with iviTR_DEBUG_DETECT_INIT, iviTR_DEBUG_DETECT_START, iviTR_DEBUG_DETECT_WAIT, iviTR_DEBUG_DETECT_RES.
	//
	// Usage:\n
	// void Sample() 
	// {
	// 	 iviTR_DEBUG_DETECT_INIT
	// 		iviTR_DEBUG_DETECT_START
	//
	//		// put real working code here
	//		// and expect that the debug detection will be complete
	//		// before we get to the iviTR_DEBUG_DETECT_WAIT loop.
	//
	// 		iviTR_DEBUG_DETECT_WAIT(50);
	//
	// 	BOOL bDebuggerDetected = iviTR_DEBUG_DETECT_RES;
	// 	// do something heinous (but untraceable)
	//
	//	iviTR_DEBUG_DETECT_END
	// }
	//*/
#ifdef __cplusplus
	#define iviTR_DEBUG_DETECT_END \
	delete pcrasherthd; \
	pcrasherthd = NULL;
#else
	#define iviTR_DEBUG_DETECT_END 
#endif

#elif defined(__linux__)
	#define iviTR_DEBUG_DETECT_INIT
	#define iviTR_DEBUG_DETECT_START
	#define iviTR_DEBUG_DETECT_WAIT
	#define iviTR_DEBUG_DETECT_RES
	#define iviTR_DEBUG_DETECT_END
#endif // _WIN32

#endif // !defined(DEBUG_DETECT_EXCEPT__H__0EB7A1C3_64B7_4DB6_A9CE_F8DF5866E94B__INCLUDED_)
