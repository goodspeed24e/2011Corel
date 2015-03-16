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

#ifndef _H264_VDEC_GMO_H_
#define _H264_VDEC_GMO_H_

#include <windows.h>
#include "GMO/GMO_i.h"
#include "GMO/GMOBase.h"
#include "GMO/GMOImpl.h"
#include "VdoCodecGMO.h"
#include "IVdoParameter.h"
#include "IH264VDec.h"
#include "../../H264VDec/H264VDecHP/H264VDecHP_impl.h"
#include "vdodropmanager.h"


enum E_VDO_SUPPORT_INPUT_TYPE_H264
{
	E_VDO_INPUT_TYPE_BLH264 = 0,
	E_VDO_INPUT_TYPE_MLH264 = 1,
	E_VDO_INPUT_TYPE_HLH264 = 2,
	E_VDO_INPUT_TYPE_ELH264 = 3,
};

enum E_VDO_SUPPORT_OUTPUT_TYPE_H264
{
	E_VDO_OUTPUT_TYPE_NV12 = 0,
	E_VDO_OUTPUT_TYPE_IVIH264 = 1,
	E_VDO_OUTPUT_TYPE_IVIH264_EX = 2,
	E_VDO_OUTPUT_TYPE_OFFSET_METADATA = 3,
};

//for PERF, the same with utilperf.h
#define PERF_VIDEO_BYTECOUNT           0
#define PERF_VIDEO_DROPPED_FRAMES      1
#define PERF_VIDEO_TOTAL_FRAMES        2
#define PERF_VIDEO_I_FRAMES            3
#define PERF_VIDEO_P_FRAMES            4
#define PERF_VIDEO_B_FRAMES            5
#define PERF_VIDEO_INTERLACED_FRAMES   6
#define PERF_VIDEO_PRESENTATION_JITTER 7
#define PERF_AUDIO_BYTECOUNT           8
#define PERF_VIDEO_DECODE_MSEC         9
#define PERF_VIDEO_DISPLAY_MSEC        10
#define PERF_AUDIO_FRAME_MSEC          11

#define PERF_VIDEO_CODEC_MSEC          22

class CH264VDecGMO :
	public CVdoCodecGMO,
	public IVdoParameter,
	public IH264VdoParameter,
	public IH264VDecDllAPI2,
	public IH264VDecDllDXVA
{
public:
	IMPLEMENT_MAKE(CH264VDecGMO);
	IMPLEMENT_CREATEINSTANCE(CH264VDecGMO, NULL);
	DECLARE_IUNKNOWN

protected:
	CH264VDecGMO(IUnknown* pUnkOuter= NULL);
	virtual ~CH264VDecGMO();

	int MapFrameTypeForDropManager(int nImgType);

	HRESULT DecodeFrame(BOOL bEOS);
	HRESULT DecodeFrame_BA(BOOL bEOS);

public:
	virtual STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

	//IH264VdoParameter
	virtual HRESULT GetParameter(void* parameter, H264VdoGMOParam::VdoParamType pType = H264VdoGMOParam::VDO_PARAM_UNKOWN) const;
	virtual HRESULT SetParameter(void* parameter, H264VdoGMOParam::VdoParamType pType = H264VdoGMOParam::VDO_PARAM_UNKOWN);

	//IMethodImpl Methods
	virtual STDMETHODIMP InternalAllocateStreamingResources(void);
	virtual STDMETHODIMP InternalDiscontinuity(DWORD dwInputStreamIndex);
	virtual STDMETHODIMP InternalFlush(void);
	virtual STDMETHODIMP InternalFreeStreamingResources(void);
	virtual STDMETHODIMP InternalGetInputMaxLatency(DWORD dwInputStreamIndex, REFERENCE_TIME *prtMaxLatency);
	virtual STDMETHODIMP InternalSetInputMaxLatency(DWORD dwInputStreamIndex, REFERENCE_TIME rtMaxLatency);
	virtual STDMETHODIMP InternalGetInputSizeInfo(DWORD dwInputStreamIndex, DWORD *pcbSize, DWORD *pulSizeMaxLookahead, DWORD *pulSizeAlignment);
	virtual STDMETHODIMP InternalGetOutputSizeInfo(DWORD dwOutputStreamIndex, DWORD *pcbSize, DWORD *pulSizeAlignment);
	virtual STDMETHODIMP InternalGetInputStreamInfo(DWORD dwInputStreamIndex, DWORD *pdwFlags);
	virtual STDMETHODIMP InternalGetOutputStreamInfo(DWORD dwOutputStreamIndex, DWORD *pdwFlags);
	virtual STDMETHODIMP InternalGetInputType(DWORD dwInputStreamIndex, DWORD dwTypeIndex, GMO_MEDIA_TYPE *pmt);
	virtual STDMETHODIMP InternalGetOutputType(DWORD dwOutputStreamIndex, DWORD dwTypeIndex, GMO_MEDIA_TYPE *pmt);
	virtual STDMETHODIMP InternalProcessInput(DWORD dwInputStreamIndex, IGMediaBuffer *pBuffer, DWORD dwFlags, REFERENCE_TIME rtTimeStamp, REFERENCE_TIME rtTimeLength);
	virtual STDMETHODIMP InternalProcessOutput(DWORD dwFlags, DWORD OutputBufferCount, GMO_OUTPUT_DATA_BUFFER *pOutputBuffers, DWORD *pdwStatus);
	//virtual STDMETHODIMP SetInputType(ULONG dwInputStreamIndex, const GMO_MEDIA_TYPE *pmt, DWORD dwFlags);
	//virtual STDMETHODIMP SetOutputType(ULONG dwOutputStreamIndex, const GMO_MEDIA_TYPE *pmt, DWORD dwFlags);

	//IGMediaObjectImpl Required overides
	virtual STDMETHODIMP InternalAcceptingInput(DWORD dwInputStreamIndex);
	virtual STDMETHODIMP InternalCheckInputType(DWORD dwInputStreamIndex, const GMO_MEDIA_TYPE *pmt);
	virtual STDMETHODIMP InternalCheckOutputType(DWORD dwOutputStreamIndex, const GMO_MEDIA_TYPE *pmt);

	//IH264VDecDllAPI
	HRESULT __stdcall H264VDec_Create(
		OUT H264VDecHP **ppDecoder
		);

	HRESULT __stdcall H264VDec_Release(
		IN H264VDecHP *pDecoder
		);

	HRESULT __stdcall H264VDec_Open(
		IN H264VDecHP *pDecoder,
		IN const H264VDecHP_OpenOptions *pOptions,
		IN const DWORD dwSize,
		OUT void **imgp
		);

	HRESULT __stdcall H264VDec_Close(
		IN H264VDecHP *pDecoder,
		IN void *imgp
		);

	HRESULT __stdcall H264VDec_Stop(
		IN H264VDecHP *pDecoder,
		IN const DWORD dwStopFlags,
		IN void *imgp
		);

	HRESULT __stdcall H264VDec_DecodeFrame(
		IN H264VDecHP *pDecoder,
		IN const H264VDecHP_DecodeOptions *pOptions,
		IN const DWORD dwSize,
		OUT DWORD *pdwNumberOfDecodedFrames,
		OUT DWORD *pdwNumberOfSkippedFrames,
		IN void *imgp
		);

	HRESULT __stdcall H264VDec_GetFrame(
		IN H264VDecHP *pDecoder,
		IN const H264VDecHP_GetFrameOptions *pOptions,
		IN const DWORD dwSize,
		OUT H264VDecHP_Frame *pFrame,
		IN const DWORD dwFrameSize,
		IN void *imgp
		);

	HRESULT __stdcall H264VDec_ReleaseFrame(
		IN PVOID pDecoder,
		IN DWORD dwCookie,
		IN void *imgp
		);

	HRESULT __stdcall H264VDec_ReleaseBuffer(
		IN PVOID pDecoder,
		IN const DWORD dwReleaseFlags,
		OUT LPBYTE *ppBufferPtr
		);

	HRESULT __stdcall H264VDec_Get(
		IN H264VDecHP *pDecoder,
		IN DWORD dwPropID,
		IN LPVOID pPropData,
		IN DWORD cbPropData,
		OUT DWORD *pcbReturned
		);

	HRESULT __stdcall H264VDec_Set(
		IN H264VDecHP *pDecoder,
		IN DWORD dwPropID,
		IN LPVOID pPropData,
		IN DWORD cbPropData
		);

	HRESULT __stdcall H264VDec_AddRef(
		IN H264VDecHP *pDecoder
		);

	HRESULT __stdcall H264VDec_AddRefFrame(
		IN PVOID pDecoder,
		IN DWORD dwCookie
		);

	//IH264VDecDllDXVA
	HRESULT __stdcall H264VDec_Reset_DXVA(
		IN LPVOID pVideoAccel,
		IN DWORD dwCookie,
		IN PVOID pDecoder,
		IN void *imgp
		);

	HRESULT __stdcall H264VDec_Release_DXVA(
		IN PVOID pDecoder,
		void *imgp
		);

	//IH264VDecDllAPI2
	HRESULT __stdcall H264VDec_GetDisplayStatus(
		IN H264VDecHP *pDecoder,
		IN const DWORD dwIdx,
		IN void *imgp
		);

	HRESULT __stdcall H264VDec_FinishDisplay(
		IN H264VDecHP *pDecoder,
		IN DWORD dwEstimateNextFrame,
		IN void *imgp
		);

	HRESULT __stdcall H264VDec_OpenKey(
		IN H264VDecHP *pDecoder,
		IN H264VDec_OnOpenKeyFcn pCallBack,
		IN LPVOID pParam
		);

protected:
	CH264VDec *m_pH264Dec;
	CCollectOneNalu *m_pCollectOneNalu;
	DWORD m_dwInputTypeIndex;
	DWORD m_dwOutputTypeIndex[2];
	BOOL m_bOpenByGMO;

	BOOL m_bGlobal_nalu_exist;

	DWORD m_dwStopFlag;
	BOOL m_bReadyToDecode;

	CVideoDropManager *m_pVdoDropManager;
	H264VDecHP_Frame m_StoredOutFrame;

	BOOL m_bSkipNextFrame;

	//for PERF
	DWORD *m_pPerfBase;

	//only for fast seeking
	BOOL m_bIsFastSeek;
	DWORD m_dwFastSeekTargetFrameNum;
	DWORD m_dwFastSeekDisplayCount;
	DWORD m_dwFastSeekDropCount;
	DWORD m_dwFastSeekDecodeCount;
};

#endif //_H264_VDEC_GMO_H_