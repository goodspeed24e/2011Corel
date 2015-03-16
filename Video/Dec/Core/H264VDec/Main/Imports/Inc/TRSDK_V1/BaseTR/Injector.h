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
#if !defined(_INJECTOR_H_INCLUDED_)
#define _INJECTOR_H_INCLUDED_

#include "IVITrWinDef.h"
#include "IVITRComm.h"

NS_TRLIB_BEGIN

	///*!
	// [Internal use. Do not call it.]
	// Uninitial function.
	//*/
	LPBYTE WINAPI GetRealAddr(PBYTE pbCode, BOOL fSkipJmp);

#ifdef __cplusplus
	///*!
	// [Internal class. Do not call it.]
	//
	// Determine is the API address has has been hacked.
	// still need test.
	//*/
	class CHijackApi
	{
	public:
		CHijackApi(LPBYTE pApiAddr, LPBYTE pHijackAddr);
		CHijackApi(const CHijackApi &copyObj){*this = copyObj;}
		~CHijackApi();

		CHijackApi& operator=(const CHijackApi &copyObj);

		bool RemoveHijack();
		bool RestoreHijack();
		static bool IsHijacked(LPBYTE pApiAddr);
		//static bool IsHijackedByUs(LPBYTE pApiAddr);

		LPBYTE		m_pApiAddr;
		LPBYTE		m_pRealAddr;
		LPBYTE		m_pHijackAddr;
		BYTE		m_DispBytes[5]; // jmp takes 5 bytes
	};
#endif

NS_TRLIB_END

#endif // !defined(_INJECTOR_H_INCLUDED_)
