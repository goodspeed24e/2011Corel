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

#ifndef _H264VDECHP_IMPL_H_
#define _H264VDECHP_IMPL_H_

#include "global.h"
#include "nalucommon.h"
#include "HVDService.h"
#include "GMO/GMO_i.h"
#include "GMO/GMOBase.h"
#include "GMO/GMOImpl.h"

#include <queue>
#include <list>

class CH264VideoFrameMgr;
typedef struct storable_picture StorablePicture;


class CH264VDecHP
{
public:
	CH264VDecHP();
	~CH264VDecHP();
	void *operator new(size_t s);
	void operator delete(void *p);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();

#if !defined(_COLLECT_PIC_)
#ifdef _GLOBAL_IMG_
	STDMETHODIMP Open(IN const H264VDecHP_OpenOptionsEx *pOptions, IN const DWORD dwSize);
#else
	STDMETHODIMP Open(IN const H264VDecHP_OpenOptionsEx *pOptions, IN const DWORD dwSize, ImageParameters **imgp);
#endif

#else
	STDMETHODIMP Open(IN const H264VDecHP_OpenOptionsEx *pOptions, IN const DWORD dwSize, void **imgp);

#endif

#if defined (_USE_SCRAMBLE_DATA_)
	STDMETHODIMP OpenKey(IN H264VDec_OnOpenKeyFcn pCallBack, IN LPVOID pParam);
#endif

	STDMETHODIMP Close PARGS0();
	STDMETHODIMP DecodeFrame PARGS4(IN const H264VDecHP_DecodeOptions *pOptions,
		IN const DWORD dwSize,
		OUT DWORD *pdwNumberOfDisplayableFrames,
		OUT DWORD *pdwNumberOfSkippedFrames);
	STDMETHODIMP GetFrame PARGS5(IN const H264VDecHP_GetFrameOptions *pOptions,
		IN const DWORD dwSize,
		OUT H264VDecHP_Frame *pFrame,
		IN const DWORD dwFrameSize,
		IN unsigned int view_id);

	STDMETHODIMP_(BOOL) GetDisplayStatus PARGS1(IN const DWORD dwIdx);
	STDMETHODIMP_(BOOL) FinishDisplay PARGS1(IN DWORD dwEstimateNextFrame);

	STDMETHODIMP Stop PARGS1(IN const DWORD dwStopFlags);
	STDMETHODIMP_(ULONG) AddRefFrame(IN const DWORD dwCookie);
	STDMETHODIMP_(ULONG) ReleaseFrame PARGS1(IN const DWORD dwCookie);
	STDMETHODIMP ReleaseBuffer (IN const DWORD dwReleaseFlags, OUT LPBYTE *ppBufferPtr);
	STDMETHODIMP Get(DWORD dwPropID, OUT LPVOID pPropData, IN DWORD cbPropData, OUT DWORD *pcbReturned);
	STDMETHODIMP Set(DWORD dwPropID, IN LPVOID pPropData, IN DWORD cbPropData);

	CPU_LEVEL GetCPULevel();				//for detecting the type of CPU (support MMX, SSE, SSE2 and SSE3)
	void InitializeVideoDecoder();			//decide the decoder implementation by CPU types

	int GetPicture PARGS2(H264VDecHP_Frame *pFrame, unsigned int view_index);
	

protected:

	int Reset PARGS0();

	CRITICAL_SECTION m_csDecode;	
	LONG m_lRefCount;				// counts references by the decoder

	BOOL m_bSetNormalSpeed;

#if defined (_COLLECT_PIC_)
	stream_par                *stream_global;
#endif

	void *m_pIHVDService;

};

class CCollectOneNalu
{
public:
	CCollectOneNalu();
	~CCollectOneNalu();

	NALU_t *m_pnalu;

	void SetBufferPointer(BYTE *pbDataBuffer, DWORD dwInputLength);
	void SetGlobalTimeStamp(REFERENCE_TIME rtTimeStamp, REFERENCE_TIME rtTimeLength);

	HRESULT CollectOneNALU(BOOL bEOS, int* output_pos);
	CREL_RETURN GetAnnexbNALU(NALU_t *one_nalu, BOOL bEOS, int* output_pos);
	CREL_RETURN AllocNALUDecodingBuffer(NALU_t *one_nalu);
	CREL_RETURN ConsumeAnnexbNALStartCode(NALU_t *one_nalu);

	HRESULT DXVA_CollectOneNALU(BOOL bEOS, int* output_pos, LPVOID lpPrivateInputData, LPVOID lpPrivateOutputData);
	CREL_RETURN DXVA_GetAnnexbNALU(NALU_t *one_nalu, BOOL bEOS, int* output_pos, LPVOID lpPrivateInputData, LPVOID lpPrivateOutputData);

	DWORD GetRemainBufferLength();

	void Reset();

	void  ChangeDataStatus(DWORD dwStatus) {m_dwDataStatus = dwStatus;};
	DWORD GetDataStatus() {return m_dwDataStatus;};

protected:
	BYTE *buffer_begin, *buffer_end;
	AnnexbNALUDecodingState m_state_NAL;

	std::queue<H264_TS> m_QueueTS;

private:
	BOOL m_bReceivedEOS;
	DWORD m_dwDataStatus;
};

class CH264VDec
{
public:
	CH264VDec();
	~CH264VDec();

	void SetDefaultOpenOptions();

	//Used for ProcessOutput
	HRESULT CollectOneFrame(BOOL bEOS, NALU_t *one_nalu);
	HRESULT DoCollectOneFrame PARGS0();
	HRESULT DoProcessOneNALU PARGS1(int *header);
	HRESULT DecodeThisFrame(BOOL bEOS, BOOL &bSkip, BOOL bFastSeek);
	HRESULT DoEOSFlush PARGS0();

	//Used for Interface with CH264VDecHP
	HRESULT OpenH264VDec();
	HRESULT CloseH264VDec();
	HRESULT StopH264VDec(DWORD dwStopFlag);
	HRESULT SetH264VDec(DWORD dwPropID, void *pValue);

	HRESULT GetH264VFrame(H264VDecHP_Frame *pH264VFrame, unsigned int view_index);
	HRESULT GetH264VFrameEx(H264VDecHP_FrameEx *pH264VFrame, unsigned int view_index);
	HRESULT ReleaseH264VFrame(DWORD dwCookie);
	HRESULT ResetDataBuffer(DWORD *pValue);

	DWORD GetDisplayCount();
	DWORD GetDisplayFrame(int view_index);
	DWORD GetCurrentImgType();
	DWORD GetNextImgType();
	BOOLEAN IsNextBaseView();
	void SetNextSkipPOC();
	BOOLEAN IsNextDenpendentViewSkipped();
	DWORD GetRemainBufferLength();
	BOOL  IsSkipFirstB() { return (stream_global->m_bSkipFirstB>0)?TRUE:FALSE; };

	void UpdateOpenOption(void *pOpenOption);
	void UpdateOpenOptionEx(void *pOpenOption);

	//DXVA related
	DWORD GetDXVAMode();
	DWORD GetStreamBufferSize();
	HRESULT DecodeBistreamDXVA(CCollectOneNalu *pCollectNalu, BOOL bEOS, BOOL *pSkip);

	void SetIHVDService(void *pIHVDService) {m_pIHVDService = pIHVDService;};

	HRESULT DoEOSBistreamDXVA(BOOL bSkip, int nRetDecodeOnePicture, int *pHeader);

	//sync with render
	BOOL GetDisplayStatus(DWORD dwIndex);
	BOOL FinishDisplay(DWORD dwEstimateNextFrame);

	//FAST SEEK
	BOOL IsSeekToOpenGOP() { return stream_global->bSeekToOpenGOP;};

	// MVC
	int GetCurrentOutputViewIndex() { return stream_global->m_CurrOutputViewIndex; };
	void SetNextOutputViewIndex();

	//Get offset meta data
	HRESULT GetOffsetMetadata(H264VDecHP_Offset_Metadata *pOffsetMetadata, unsigned __int64 pts, DWORD dwCookie);
	void Release_Metadata(DWORD dwCookie);

	HRESULT SetAppConstrants(void *pValue);

	//SECOP
	DWORD GetCurrentTotalByteCount_SECOP();

	BOOL IsMVCFieldPicture();
	BOOL m_bMVCFieldEOS;

protected:
	stream_par *stream_global;

	CH264VDecHP *m_pDecImpl;
	void *m_pStream;

	H264VDecHP_OpenOptionsEx *m_pOpenOptions;
	ImageParameters *m_pActiveImg;
	DWORD m_dwNextImageType;

	//DXVA related
	void *m_pIHVDService;
	BYTE bGotNextNalu;

	std::list<H264_Offset_Metadata_Control> m_ListOffsetMetadata;

	BOOL m_bIsAppConstrantsBD3D;

	//For MVC filed coding EOS
	int m_iMVCFieldEOSActiveIndex;


};

#endif //_H264VDECHP_IMPL_H_
