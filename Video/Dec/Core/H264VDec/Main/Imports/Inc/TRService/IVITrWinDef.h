#ifndef _IVI_TRWINDEF_H
#define _IVI_TRWINDEF_H
#ifndef __cplusplus
	#ifndef DWORD
		typedef unsigned long       DWORD;
	#endif
	#ifndef WORD
		typedef unsigned short       WORD;
	#endif
	#ifndef BYTE
		typedef unsigned char       BYTE;
	#endif
	#ifndef bool
		typedef	short				bool;
	#endif
	#ifndef BOOL
		typedef int                 BOOL;
	#endif
#endif

#include "IVITRComm.h"

#ifdef _WIN32
	#include <windows.h>
#endif // _WIN32

#ifdef __linux__
	#ifndef __cdecl
	#define __cdecl
	#endif // __cdecl

	#ifndef __stdcall
	#define __stdcall
	#endif // __stdcall

	#ifndef __fastcall
	#define __fastcall
	#endif // __fastcall
	
	#ifndef CALLBACK
	#define CALLBACK
	#endif // CALLBACK

	#ifndef WINAPI
	#define WINAPI
	#endif // WINAPI

	#ifndef __forceinline
	#define __forceinline inline
	#endif // __forceinline

	#ifndef FALSE
	#define FALSE false
	#endif // FALSE

	#ifndef TRUE
	#define TRUE true
	#endif // TRUE

	typedef unsigned long DWORD;
	typedef void* HANDLE;
	typedef void* LPVOID;
	typedef long LONG;
	typedef LONG HRESULT;
	typedef bool BOOL;
	typedef unsigned short WORD;
#endif // __linux__

#endif // _IVI_TRWINDEF_H
