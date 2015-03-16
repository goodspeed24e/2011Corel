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

#ifndef _IVICHECK_H
#define _IVICHECK_H

#include "IVITrWinDef.h"
#include "IVITRComm.h"

///*!
// define a callback function type for found the same result.
//*/
typedef void (CALLBACK * CALLBACK_SAME)(LPVOID);

///*!
// define a callback function type for the result changed.
//*/
typedef void (CALLBACK * CALLBACK_CHANGED)(LPVOID);

///*!
// define a callback function type for if match the result.
//*/
typedef void (CALLBACK * CALLBACK_MATCH)(LPVOID);

///*!
// define a callback function type for if not match the result.
//*/
typedef void (CALLBACK * CALLBACK_NOMATCH)(LPVOID);

NS_TRLIB_BEGIN

///*!
// [Internal function. Do not call it.]
//
// Wild-charactor string compare function.
// Parsing '*' and '|' and '?' charactors.
//
// Return value - [int] position of string match.
//*/
int __cdecl striexpcmp(char *wildcard, char *string);

///*!
// [Internal function. Do not call it.]
//
// Tokenize string based on delimiter.
//
// Return value - [int] position of string match.
//*/
int __cdecl tokenize(char *buffer, char **arglist, char delimter);

///*!
// [Internal function. Do not call it.]
//
// simple encryption and decryption using vigenere cipher given KEY above
//
// Return value - None
//*/
void __fastcall decrypt_string_macro(const char * str, char *out);

NS_TRLIB_END

#endif // _IVICHECK_H
