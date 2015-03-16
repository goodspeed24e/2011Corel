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
// 	Copyright (c) 2004 - 2005  InterVideo Corporation.  All Rights Reserved.
//
//-----------------------------------------------------------------------------

#ifndef _MP2VDECMP_H_
#define _MP2VDECMP_H_

#include "IMP2VDec.h"

//
// Creates a new video decoder object with a reference count
// of 1 and stores a pointer to it in ppDecoder.
//
// This object needs to be freed by MP2VDecMP_Release().
//
HRESULT
MP2VDecMP_Create(
	OUT PVOID *ppDecoder,
	IN const bool IsEnableDXVA
    );
typedef HRESULT (*MP2VDecMP_CreateFcn)(
	OUT PVOID *ppDecoder,
	IN const bool IsEnableDXVA = false
	);

//
// Decrements reference count of the decoder object. 
// If the reference count goes to 0, frees up all
// resources used by video decoder.
//
// pDecoder - Decoder object created with MP2VDecMP_Create()
//
ULONG
MP2VDecMP_Release(
    IN PVOID pDecoder
    );
typedef ULONG (*MP2VDecMP_ReleaseFcn)(
	IN PVOID pDecoder
	);

//
// Adds a reference count to the decoder.
//
// pDecoder - Decoder object created with MP2VDecMP_Create()
//
ULONG
MP2VDecMP_AddRef(
    IN PVOID pDecoder
    );
typedef ULONG (*MP2VDecMP_AddRefFcn)(
	IN PVOID pDecoder
	);

//
// Initializes the decoder with the OpenOptions.
// This is required after Creation of the decoder 
// object, before any decoding occurs.
// 
HRESULT
MP2VDecMP_Open(
    IN PVOID pDecoder,
	IN const MP2VDecMP_OpenOptions *pOptions,
	IN const DWORD dwSize
	);
typedef HRESULT (*MP2VDecMP_OpenFcn)(
	IN PVOID pDecoder,
	IN const MP2VDecMP_OpenOptions *pOptions,
	IN const DWORD dwSize
	);

//
// Deallocates any resources held by the decoder
// as part of decoding and displaying.  This frees
// up the runtime resources used by the decoder.
// 
HRESULT
MP2VDecMP_Close(
    IN PVOID pDecoder
	);
typedef HRESULT (*MP2VDecMP_CloseFcn)(
	IN PVOID pDecoder
	);

//
// Halts current decoding, flushes all frames
// and buffers required for decoding.  The
// display frames are kept however.
// 
HRESULT
MP2VDecMP_Stop(
    IN PVOID pDecoder,
	IN const DWORD dwStopFlags
	);
typedef HRESULT (*MP2VDecMP_StopFcn)(
	IN PVOID pDecoder,
	IN const DWORD dwStopFlags
	);

//
// Decodes data into a frame.  In general, one decoded frame is generated
// and placed into the display queue, although this is not necessarily always
// the case, sometimes 0 or multiple frames may be decoded.
//
// The callback pfnDataCallback is used to obtain data.
// The callback pfnSkipFrameCallback is used to determine whether to skip a frame
//
// pDecoder - Decoder object created with MP2VDecMP_Create()
// pdwNumberOfDecodedFrames - Number of frames decoded (retrieved by MP2VDecMP_GetFrame())
//
HRESULT
MP2VDecMP_DecodeFrame(
    IN PVOID pDecoder,
	IN const MP2VDecMP_DecodeOptions *pOptions,
	IN const DWORD dwSize,
    OUT DWORD *pdwNumberOfDecodedFrames	
    );
typedef HRESULT (*MP2VDecMP_DecodeFrameFcn)(
	IN PVOID pDecoder,
	IN const MP2VDecMP_DecodeOptions *pOptions,
	IN const DWORD dwSize,
	OUT DWORD *pdwNumberOfDecodedFrames	
	);

//
// Retrieves a frame from the display queue. The structure pFrame is a caller
// allocated structure, but the buffer pointers within that structure will 
// be returned pointing to decoder allocated buffers.
// Thus, the frame must be released through MP2VDecMP_ReleaseFrame() function below.
// This function returns E_FAIL when there are no more frames left.
//
HRESULT
MP2VDecMP_GetFrame(
    IN PVOID pDecoder,
	IN const MP2VDecMP_GetFrameOptions *pOptions,
	IN const DWORD dwSize,
	OUT MP2VDecMP_Frame *pFrame,
	IN const DWORD dwFrameSize
	);
typedef HRESULT (*MP2VDecMP_GetFrameFcn)(
	IN PVOID pDecoder,
	IN const MP2VDecMP_GetFrameOptions *pOptions,
	IN const DWORD dwSize,
	OUT MP2VDecMP_Frame *pFrame,
	IN const DWORD dwFrameSize
	);

//
// Increments reference count of the display frame buffers returned from MP2VDecMP_GetFrame().
// Note that dwCookie is the frame cookie returned in MP2VDecMP_Frame structure.
//
ULONG
MP2VDecMP_AddRefFrame(
    IN PVOID pDecoder,
	IN DWORD dwCookie
	);
typedef ULONG (*MP2VDecMP_AddRefFrameFcn)(
	IN PVOID pDecoder,
	IN DWORD dwCookie);

//
// Decrements reference count of the display frame buffers returned from MP2VDecMP_GetFrame().
// Note that dwCookie is the frame cookie returned in MP2VDecMP_Frame structure.
//
// If the reference count goes to 0, it returns to the internal frame management
// queue for reuse in decoding.
//
ULONG
MP2VDecMP_ReleaseFrame(
    IN PVOID pDecoder,
	IN DWORD dwCookie
	);
typedef ULONG (*MP2VDecMP_ReleaseFrameFcn)(
	IN PVOID pDecoder,
	IN DWORD dwCookie
	);

//
// Releases the input bitstream buffer state.  This allows the decoder
// to load another buffer pointer set in the middle of decoding. 
// ppBufferPtr is the pointer in the last buffer returned by the 
// data callback function.  If ppBufferPtr is 0, then the buffer
// was exhausted and an end of sequence was generated internally.
//
HRESULT
MP2VDecMP_ReleaseBuffer(
    IN PVOID pDecoder,
	IN DWORD dwReleaseFlags,
	OUT LPBYTE *ppBufferPtr
	);
typedef HRESULT (*MP2VDecMP_ReleaseBufferFcn)(
	IN PVOID pDecoder,
	IN DWORD dwReleaseFlags,
	OUT LPBYTE *ppBufferPtr
	);

//
// Gets a property from the decoder. 
// This function returns E_PROP_ID_UNSUPPORTED
// if the property ID is unsupported for the Get function.  
// This returns E_FAIL if the property ID is supported
// but either the cbPropData and pPropData are unmatched or
// the Get fails.
//
// QuerySupported can be implemented by a "Get" with cbPropData and pPropData
// set to 0 and testing whether the return value is E_PROP_ID_UNSUPPORTED
// or E_FAIL.
//
// There is no guid propset.  This should be taken care of by
// routing in the application or system framework layer.
//
HRESULT
MP2VDecMP_Get(
    IN PVOID pDecoder,
	IN DWORD dwPropID,
	OUT LPVOID pPropData,
	IN DWORD cbPropData,
	OUT DWORD *pcbReturned
	);
typedef HRESULT (*MP2VDecMP_GetFcn)(
	IN PVOID pDecoder,
	IN DWORD dwPropID,
	OUT LPVOID pPropData,
	IN DWORD cbPropData,
	OUT DWORD *pcbReturned
	);

//
// Sets a property in the decoder. 
// This function returns E_PROP_ID_UNSUPPORTED
// if the property ID is unsupported for the Set function.  
// This returns E_FAIL if the property ID is supported
// but either the cbPropData and pPropData are unmatched or
// the Set fails.
//
// QuerySupported can be implemented by a "Set" with cbPropData and pPropData
// set to 0 and testing whether the return value is E_PROP_ID_UNSUPPORTED
// or E_FAIL.
//
// There is no guid propset.  This should be taken care of by
// routing in the application or system framework layer.
//
HRESULT
MP2VDecMP_Set(
    IN PVOID pDecoder,
	IN DWORD dwPropID,
	IN LPVOID pPropData,
	IN DWORD cbPropData
	);
typedef HRESULT (*MP2VDecMP_SetFcn)(
	IN PVOID pDecoder,
	IN DWORD dwPropID,
	IN LPVOID pPropData,
	IN DWORD cbPropData
	);

HRESULT 
MP2VDecMP_OpenKey(
				   IN PVOID pDecoder,
				   IN MP2VDec_OnOpenKeyFcn pCallBack,
				   IN LPVOID pParam
				   );

typedef HRESULT (*MP2VDecMP_OpenKeyFcn)(
	IN PVOID pDecoder,
	IN MP2VDec_OnOpenKeyFcn pCallBack,
	IN LPVOID pParam
	);

#endif // _MP2VDECMP_H_
