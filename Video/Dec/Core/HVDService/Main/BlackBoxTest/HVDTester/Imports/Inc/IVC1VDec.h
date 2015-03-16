//=============================================================================
//	THIS SOURCE CODE IS PROPRIETARY INFORMATION BELONGING TO INTERVIDEO, INC.
// 	ANY USE INCLUDING BUT NOT LIMITED TO COPYING OF CODE, CONCEPTS, AND/OR
//	ALGORITHMS IS PROHIBITED EXCEPT WITH EXPRESS WRITTEN PERMISSION BY THE 
//	COMPANY.
//
// 	THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
// 	KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// 	IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
// 	PURPOSE.
//
// 	Copyright (c) 2007 InterVideo Corporation.  All Rights Reserved.
//
//-----------------------------------------------------------------------------

#ifndef _IVC1VDECDLL_H_
#define _IVC1VDECDLL_H_

#include <windows.h>
#include "VC1TS.h"
#include "VC1Debug.h"
#include "VC1VDecDef.h"

#if !defined(E_PROP_ID_UNSUPPORTED)
#define E_PROP_ID_UNSUPPORTED     ((HRESULT)0x80070490L)
#endif

#define VC1_SCRAMBLE_DATA_LEN 256

struct IAMVideoAccelerator;
struct IDirectXVideoDecoder;
class DXVA_ENCRYPTBase;

EXTERN_GUID(IID_IVC1VDecDllAPI, 0x5ec7e3d5, 0x5117, 0x4542, 0x90, 0x4a, 0x5f, 0x2f, 0x4f, 0xb2, 0x64, 0xcc);
class IVC1VDecDllAPI : public IUnknown
{
public:
	virtual HRESULT _CALL_CONV VC1VDec_Create(
		OUT LPVOID *ppDecoder
		) = 0;

	virtual HRESULT _CALL_CONV VC1VDec_Release(
		IN LPVOID pDecoder
		) = 0 ;

	virtual HRESULT _CALL_CONV VC1VDec_Open(
		IN LPVOID pDecoder,
		IN const VC1VDecParam::VC1VDec_OpenOptions *pOptions,
		IN const DWORD dwSize
		) = 0;

	virtual HRESULT _CALL_CONV VC1VDec_Close(
		IN LPVOID pDecoder
		) = 0;

	virtual HRESULT _CALL_CONV VC1VDec_Stop(
		IN LPVOID pDecoder,
		IN const DWORD dwStopFlags
		) = 0;

	virtual HRESULT _CALL_CONV VC1VDec_DecodeFrame(
		IN LPVOID pDecoder,
		IN const VC1VDecParam::VC1VDec_DecodeOptions *pOptions,
		IN const DWORD dwSize,
		OUT DWORD *pdwNumberOfDecodedFrames,
		OUT DWORD *pdwNumberOfSkippedFrames
		) = 0;

	virtual HRESULT _CALL_CONV VC1VDec_GetFrame(
		IN LPVOID pDecoder,
		IN const VC1VDecParam::VC1VDec_GetFrameOptions *pOptions,
		IN const DWORD dwSize,
		OUT VC1VDecParam::VC1VDec_Frame *pFrame,
		IN const DWORD dwFrameSize
		) = 0;

	virtual HRESULT _CALL_CONV VC1VDec_ReleaseFrame(
		IN LPVOID pDecoder,
		IN DWORD dwCookie
		) = 0;

	virtual HRESULT _CALL_CONV VC1VDec_Get(
		IN LPVOID pDecoder,
		IN DWORD dwPropID,
		OUT LPVOID pPropData,
		IN DWORD cbPropData,
		OUT DWORD *pcbReturned
		) = 0;

	virtual HRESULT _CALL_CONV VC1VDec_Set(
		IN LPVOID pDecoder,
		IN DWORD dwPropID,
		IN LPVOID pPropData,
		IN DWORD cbPropData
		) = 0;

	virtual HRESULT _CALL_CONV VC1VDec_ReleaseBuffer(
		IN LPVOID pDecoder,
		IN DWORD dwReleaseFlags,
		OUT LPBYTE *ppBufferPtr
		) = 0;

	virtual HRESULT _CALL_CONV VC1VDec_OpenKey(
		IN LPVOID pDecoder,
		IN VC1VDecParam::VC1VDec_OnOpenKeyFcn pCallBack,
		IN LPVOID pParam
		) = 0;

	virtual HRESULT _CALL_CONV VC1VDec_SetVideoAccel(
		IN LPVOID pDecoder,
		IN LPVOID lpvVideoAccel
		) = 0;

	virtual HRESULT _CALL_CONV VC1VDec_ReleaseAccelBuffer(
		IN LPVOID pDecoder
		) = 0;
};
#endif

