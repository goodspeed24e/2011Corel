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
#ifndef _IVI_ANTIDISASSEMBLE_H
#define _IVI_ANTIDISASSEMBLE_H

#include "IVITRComm.h"

///*!
// [Internal function. Do not call it.]
//
// Fake calling function;
//
// Return value - None
//
// Supported platform: All Windows/Linux
//*/
void __cdecl TR_fnFalseCall();


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


///*!
// Disturbing disassemble anaylsis by mis-alignment.
//
// Return value - None
//
// -#ll - [IN] can not be NULL, any number to assign a label name.
// -#reg - [IN] can not be NULL, stander register, such lik 'eax', 'ebx'.
// -#pad - [IN] can not be NULL, padding charactor in byte hex expression.
//
// Supported platform:		All Windows/Linux
// Performance overhead:	Low
// Security level:			High
// Usage scope:				Macro scope
//
// Note:\n
// please place closely and before your code need protection.
//*/
#ifdef _WIN32
	#define iviTR_MISALIGNMENT1(ll, reg, pad)  \
		_asm \
		{ \
			_asm cmp    reg, reg \
			_asm jz     misalign##ll \
			_asm _emit  pad \
			_asm misalign##ll: \
		}
#elif defined(__linux__)
	#define iviTR_MISALIGNMENT1(ll, reg, pad)
#endif // _WIN32

///*!
// Disturbing disassemble anaylsis by mis-alignment.
//
// Return value - None
//
// -#ll - [IN] can not be NULL, any number to assign a label name.
// -#reg - [IN] can not be NULL, stander register, such lik 'eax', 'ebx'.
// -#pad - [IN] can not be NULL, padding charactor in byte hex expression.
//
// Supported platform:		All Windows/Linux
// Performance overhead:	Low
// Security level:			High
// Usage scope:				Macro scope
//
// Note:\n
// please place closely and before your code need protection.
//*/
#ifdef _WIN32
	#define iviTR_MISALIGNMENT2(ll, reg, pad)  \
		_asm \
		{ \
			_asm cmp    reg, reg \
			_asm jz     misalign##ll \
			_asm _emit  pad \
			_asm misalign##ll: \
		}
#elif defined(__linux__)
	#define iviTR_MISALIGNMENT2(ll, reg, pad)
#endif // _WIN32

///*!
// Default #1 for disturbing disassemble anaylsis by mis-alignment.
//
// Return value - None
//
// -#ll - [IN] can not be NULL, any number to assign a label name.
//
// Supported platform:		All Windows/Linux
// Performance overhead:	Low
// Security level:			High
// Usage scope:				Macro scope
//
// Note:\n
// please place closely and before your code need protection.
//*/
#define iviTR_MISALIGNMENT_1(ll) iviTR_MISALIGNMENT1(ll, eax, 0x0F)

///*!
// Default #2 for disturbing disassemble anaylsis by mis-alignment.
//
// Return value - None
//
// -#ll - [IN] can not be NULL, any number to assign a label name.
//
// Supported platform:		All Windows/Linux
// Performance overhead:	Low
// Security level:			High
// Usage scope:				Macro scope
// 
// Note:\n
// please place closely and before your code need protection.
//*/
#define iviTR_MISALIGNMENT_2(ll) iviTR_MISALIGNMENT2(ll, ebx, 0xFF)

///*!
// Disturbing disassemble anaylsis on return value.
//
// Return value - None
//
// -#ll - [IN] can not be NULL, any number to assign a label name.
// -#reg - [IN] can not be NULL, stander register, such lik 'eax', 'ebx'.
// -#pad - [IN] can not be NULL, padding charactor in byte hex expression.
//
// Supported platform:		All Windows/Linux
// Performance overhead:	Low
// Security level:			High
// Usage scope:				Macro scope
//
// Note:\n
// please place bottom of your subroutine, closely and before your return code.
//*/
#ifdef _WIN32
	#define iviTR_FALSE_RETURN(ll, reg, pad)  \
		_asm \
		{ \
			_asm push	reg \
			_asm lea    reg, falseret##ll \
			_asm push   reg \
			_asm ret \
			_asm _emit  pad \
			_asm falseret##ll: \
			_asm pop    reg \
		}
#elif defined(__linux__)
	#define iviTR_FALSE_RETURN(ll, reg, pad)
#endif // _WIN32

///*!
// Disturbing disassemble anaylsis on return value.
//
// Return value - None
//
// -#ll - [IN] can not be NULL, any number to assign a label name.
// -#reg - [IN] can not be NULL, stander register, such lik 'eax', 'ebx'.
// -#pad - [IN] can not be NULL, padding charactor in byte hex expression.
//
// Supported platform:		All Windows/Linux
// Performance overhead:	Low
// Security level:			High
// Usage scope:				Macro scope
//
// Note:\n
// please place bottom of your subroutine, closely and before your return code.
//*/
#ifdef _WIN32
	#define iviTR_FALSE_RETURN2(ll, reg, pad)  \
		_asm \
		{ \
			_asm push   reg \
			_asm push   ebx \
			_asm push   edx \
			_asm mov    ebx, esp \
			_asm mov    esp, ebp \
			_asm pop    ebp \
			_asm pop    reg \
			_asm lea    edx, falseret##ll \
			_asm push   edx \
			_asm ret \
			_asm _emit  pad \
			_asm falseret##ll: \
			_asm push   reg \
			_asm push   ebp \
			_asm mov    ebp, esp \
			_asm mov    esp, ebx \
			_asm pop    edx \
			_asm pop    ebx \
			_asm pop    reg \
		}
#elif defined(__linux__)
	#define iviTR_FALSE_RETURN2(ll, reg, pad)
#endif // _WIN32

///*!
// Default #1 for disturbing disassemble anaylsis on return value.
//
// Return value - None
//
// -#ll - [IN] can not be NULL, any number to assign a label name.
//
// Supported platform:		All Windows/Linux
// Performance overhead:	Low
// Security level:			High
// Usage scope:				Macro scope
//
// Note:\n
// please place bottom of your subroutine, closely and before your return code.
//*/
#define iviTR_FALSE_RETURN_1(ll) iviTR_FALSE_RETURN(ll, edx, 0x0F)

///*!
// Default #1 for disturbing disassemble anaylsis on return value.
//
// Return value - None
//
// -#ll - [IN] can not be NULL, any number to assign a label name.
//
// Supported platform:		All Windows/Linux
// Performance overhead:	Low
// Security level:			High
// Usage scope:				Macro scope
//
// Note:\n
// please place bottom of your subroutine, closely and before your return code.
//*/
#define iviTR_FALSE_RETURN_2(ll) iviTR_FALSE_RETURN2(ll, ecx, 0x0F)

///*!
// Disturbing disassemble anaylsis on calling function.
//
// Return value - None
//
// -#ll - [IN] can not be NULL, any number to assign a label name.
// -#reg - [IN] can not be NULL, stander register, such lik 'eax', 'ebx'.
// -#pad - [IN] can not be NULL, padding charactor in byte hex expression.
//
// Supported platform:		All Windows/Linux
// Performance overhead:	Low
// Security level:			High
// Usage scope:				Macro scope
//
// Note:\n
// please place closely and before your calling function code.
//*/
#ifdef _WIN32
	#define iviTR_FALSE_CALL(ll, reg, pad)		\
		_asm \
		{ \
			_asm push	reg \
			_asm lea	reg, falsecall##ll \
			_asm add	reg, 1 \
			_asm push   reg \
			_asm call	TR_fnFalseCall \
			_asm xor	reg, reg \
			_asm ret \
			_asm falsecall##ll: \
			_asm _emit  pad \
			_asm pop    reg \
		}
#elif defined(__linux__)
	#define iviTR_FALSE_CALL(ll, reg, pad)
#endif // _WIN32
	
///*!
// Disturbing disassemble anaylsis on calling function.
//
// Return value - None
//
// -#ll - [IN] can not be NULL, any number to assign a label name.
//
// Supported platform:		All Windows/Linux
// Performance overhead:	Low
// Security level:			High
// Usage scope:				Macro scope
//
// Note:\n
// please place closely and before your calling function code.
//*/
#define iviTR_FALSE_CALL_1(ll) iviTR_FALSE_CALL(ll, ecx, 0x0F)

#endif
