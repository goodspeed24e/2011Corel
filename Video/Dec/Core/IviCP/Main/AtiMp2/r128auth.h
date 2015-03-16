/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*
 * File:        R128AUTH.H
 *
 * Purpose:     Class Header File for ATIIDCTEncryption.LIB
 *
 * Notes:	This file is provided under strict non-disclosure agreements
 *		it is and remains the property of ATI Technologies Inc.
 *		Any use of this file or the information it contains to
 *		develop products commercial or otherwise must be with the
 *		permission of ATI Technologies Inc.
 *
 * Copyright (C) 1998-2001, ATI Technologies Inc.
 *
 *^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
#include <windows.h>
#ifndef _R128AUTH_H
#define _R128AUTH_H

#define RAGE128AUTHHRESULT DWORD

#define RAGE128AUTH_OK	                    0x00000000
#define RAGE128AUTH_MULTICHANNEL_OK         0x0000ff00
#define RAGE128AUTH_USE_CHANNEL0            RAGE128AUTH_MULTICHANNEL_OK | 0x0
#define RAGE128AUTH_USE_CHANNEL1            RAGE128AUTH_MULTICHANNEL_OK | 0x1
#define RAGE128AUTH_USE_CHANNEL2            RAGE128AUTH_MULTICHANNEL_OK | 0x2
#define RAGE128AUTH_USE_CHANNEL3            RAGE128AUTH_MULTICHANNEL_OK | 0x3
#define RAGE128AUTH_USE_CHANNEL4            RAGE128AUTH_MULTICHANNEL_OK | 0x4
#define RAGE128AUTH_USE_CHANNEL5            RAGE128AUTH_MULTICHANNEL_OK | 0x5
#define RAGE128AUTH_USE_CHANNEL6            RAGE128AUTH_MULTICHANNEL_OK | 0x6
#define RAGE128AUTH_USE_CHANNEL7            RAGE128AUTH_MULTICHANNEL_OK | 0x7
#define RAGE128AUTH_USE_CHANNEL8            RAGE128AUTH_MULTICHANNEL_OK | 0x8
#define RAGE128AUTH_USE_CHANNEL9            RAGE128AUTH_MULTICHANNEL_OK | 0x9
#define RAGE128AUTH_USE_CHANNEL10           RAGE128AUTH_MULTICHANNEL_OK | 0xa
#define RAGE128AUTH_USE_CHANNEL11           RAGE128AUTH_MULTICHANNEL_OK | 0xb
#define RAGE128AUTH_USE_CHANNEL12           RAGE128AUTH_MULTICHANNEL_OK | 0xc
#define RAGE128AUTH_USE_CHANNEL13           RAGE128AUTH_MULTICHANNEL_OK | 0xd
#define RAGE128AUTH_USE_CHANNEL14           RAGE128AUTH_MULTICHANNEL_OK | 0xe
#define RAGE128AUTH_USE_CHANNEL15           RAGE128AUTH_MULTICHANNEL_OK | 0xf

#define RAGE128AUTHERR_FATAL				-1
#define RAGE128AUTHERR_DEVICE_NOT_AVAILABLE	-2
#define RAGE128AUTHERR_INVALID_PARAMETER	-3
#define RAGE128AUTHERR_AUTHENTICATE_FAIL	-4
#define RAGE128AUTHERR_RAGE_CHALLENGE_FAIL	-5
#define RAGE128AUTHERR_DRV_CHALLENGE_FAIL	-6
#define RAGE128AUTHERR_AUTHENTICATE_TIMEOUT -7
#define RAGE128AUTHERR_INVALID_DEVICEID		-8
#define RAGE128AUTHERR_NOT_INITIALIZED		-9

//The reason to define NULL handle to be -1 is trring to islolate it with the case that handle is 0.
//We use NULL_ENCRYPTION_HANDLE to make it back compatible with old interface which don't need 
//to pass hEncrytion handle to those functions.
#define NULL_ENCRYPTION_HANDLE  (HANDLE(-1))    
#ifdef __cplusplus
extern "C" {
#endif

RAGE128AUTHHRESULT __stdcall Rage128_InitAuthentication(DWORD DeviceID, DWORD DriverID, HANDLE hD3DDevice = NULL, HANDLE *phEncryption = NULL);
RAGE128AUTHHRESULT __stdcall Rage128_RescrambleBlock(DWORD DeviceID, LPVOID Src, LPVOID Dest, DWORD NumOfBytes, HANDLE hEncryption = NULL_ENCRYPTION_HANDLE);
RAGE128AUTHHRESULT __stdcall Rage128_RescrambleDXVABlock(DWORD DeviceID, LPVOID Src, LPVOID Dest, DWORD NumOfBytes, HANDLE hEncryption = NULL_ENCRYPTION_HANDLE);
RAGE128AUTHHRESULT __stdcall Rage128_RescrambleDXVACoeff(DWORD DeviceID, LPVOID Src, LPVOID Dest, HANDLE hEncryption = NULL_ENCRYPTION_HANDLE);
RAGE128AUTHHRESULT __stdcall Rage128_EndOfBlock(DWORD DeviceID, HANDLE hEncryption = NULL_ENCRYPTION_HANDLE);
RAGE128AUTHHRESULT __stdcall Rage128_RescrambleRuns(DWORD DeviceID, DWORD Src, LPDWORD Dest, HANDLE hEncryption = NULL_ENCRYPTION_HANDLE);
RAGE128AUTHHRESULT __stdcall Rage128_SkipMBlock(DWORD DeviceID, HANDLE hEncryption = NULL_ENCRYPTION_HANDLE);
RAGE128AUTHHRESULT __stdcall Rage128_EndAuthentication(DWORD DeviceID, HANDLE hEncryption = NULL_ENCRYPTION_HANDLE);
RAGE128AUTHHRESULT __stdcall Rage128_CheckLFSR(HANDLE hEncryption = NULL_ENCRYPTION_HANDLE);

#ifdef __cplusplus
}
#endif

#endif //R128AUTH_H