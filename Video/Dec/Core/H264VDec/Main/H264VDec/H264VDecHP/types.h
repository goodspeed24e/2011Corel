
/**
* @file types.h
* Basic data types definitions
*
* - Copyright (c) 2002-2004 Videosoft, Inc.
* - Project:	Videosoft H.264 Codec
* - Module:	Common
*/

#ifndef __TYPES_H__
#define __TYPES_H__

#include "vssh_types.h"

/* for proper compiling (Compiler dependent) */
#if defined(WIN32)
#define INLINE		__forceinline
#define FASTCALL	__fastcall
#elif _WIN32_WCE
#define INLINE		__forceinline
#define FASTCALL
#else	//Linux:
#define INLINE		inline
#define FASTCALL
#endif

typedef int             int32;
typedef unsigned int    uint32;

typedef int t_bool;
typedef int t_si32;
typedef unsigned int t_ui32;
typedef char t_si8;
typedef unsigned char t_ui8;
typedef short MV_COMP_TYPE;
typedef short PIX_COEFF_TYPE;

#endif //__TYPES_H__
