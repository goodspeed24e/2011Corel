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

#include "IVITRComm.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
	#include "IVITrWinDef.h"
	//#include "..\..\..\..\Shared\Lib\LibOS\Psapi.h"
	#include "list"
	#include "vector"
	using namespace std;

	///*!
	// [Internal use. Do not call it.]
	//
	// Define standard enumerate callback function.
	//*/
	//typedef BOOL (WINAPI *pfnEPM)(HANDLE hProcess, HMODULE* lphModule, DWORD cb, LPDWORD lpcbNeeded);

	///*!
	// [Internal use. Do not call it.]
	//
	// Define standard enumerate callback function.
	//*/
	//typedef BOOL (WINAPI *pFnGMI)(HANDLE, HMODULE, LPMODULEINFO, DWORD);

	///*!
	// [Internal use. Do not call it.]
	//
	// Initial function.
	//*/
	//void __cdecl InitPSAPIFuncs();

	///*!
	// [Internal use. Do not call it.]
	//
	// Uninitial function.
	//*/
	//void __cdecl CleanPSAPIFuncs();

	///*!
	// [Internal use. Do not call it.]
	// Uninitial function.
	//*/
	LPBYTE WINAPI GetRealAddr(PBYTE pbCode, BOOL fSkipJmp);

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

	///*!
	// [Internal class. Do not call it.]
	//
	// Determine is the API address has has been hacked.
	// still need test.
	//*/
	class CHijackList : public vector <CHijackApi>
	{
	public:

		CHijackList();

		virtual ~CHijackList();

		int Find(LPBYTE pApiAddr);
		bool AddAndHijack(LPBYTE pApiAddr, LPBYTE pHijackAddr);
		void Clear();
	};

#endif // !defined(_INJECTOR_H_INCLUDED_)
