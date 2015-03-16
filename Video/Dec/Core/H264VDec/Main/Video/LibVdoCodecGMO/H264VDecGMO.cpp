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
#define iviINITGUID

#include <windows.h>
#include <malloc.h>
#include "../../Imports/Inc/TRService.h"
#include "H264VDecGMO.h"
#include "GMO/GMediaBuffer.h"
#include "IviMediatypeGuids.h"
#if defined(_HW_ACCEL_)
#include <H264VDec\H264VDecHP\h264dxvabase.h>
#endif

#if defined(TR_TREXE_ENABLE) && !defined(TRSDK_VER)
volatile DWORD	TR_ScrambleEnable_95 = TR_SCRAMBLE_ENABLE_95^1;
volatile DWORD	TR_ChecksumEnable_95	= TR_CHECKSUM_ENABLE_95^1;
DWORD	TR_RelocateSize_95 = 1000;
WORD	TR_RelocateTable_95[1000] = {0xffff};
volatile DWORD	TR_ChecksumValue_95 = TR_CHECKSUM_VALUE_95;

// the following two functions are used as place markers.
// they are reordered by the fcnorder.txt file.
void TR_RegionBegin_95() 
{	// needs to have some meat otherwise it compresses with below.
	static int play;
	for (int i=0;i<8;i++) play+=play;
}

void TR_RegionEnd_95() 
{	// needs to have some meat otherwise it compresses with above.
	static int play;
	for (int i=0;i<16;i++) play^=play;
}

// the following function is used for descrambling.
void __fastcall TR_Descramble_95()
{
	if (TR_ScrambleEnable_95 == TR_SCRAMBLE_ENABLE_95)
	{
		TR_ScrambleEnable_95 ^= 1;	// ensure we don't do this twice!
		CTR::Descramble((void *)TR_RegionBegin_95, (void *)TR_RegionEnd_95, TR_SCRAMBLE_SEED_95, TR_RelocateTable_95);
	}
}
#elif defined(TR_TREXE_ENABLE) && TRSDK_VER >= 1000 && TRSDK_VER < 2000
iviTR_TREXE2_DECLARE(H264VDec, 46, 1500);
#endif //TR_TREXE_ENABLE

CH264VDecGMO::CH264VDecGMO(IUnknown* pUnkOuter) : CVdoCodecGMO(pUnkOuter)
{
	iviTR_VERIFY_ATTACHED_DEBUGGER(iviTR_DEFAULT_ACTION);
#if defined(TR_TREXE_ENABLE) && !defined(TRSDK_VER)
	TR_Descramble_95();
#endif
	m_pH264Dec = new CH264VDec;
	m_pCollectOneNalu = new CCollectOneNalu;
	m_dwInputTypeIndex = 2; //HL_H264
	m_dwOutputTypeIndex[0] = E_VDO_OUTPUT_TYPE_IVIH264; //IVIH264
	m_dwOutputTypeIndex[1] = E_VDO_OUTPUT_TYPE_OFFSET_METADATA; //Offset metadata
	m_bOpenByGMO = FALSE;
#if defined(TR_TREXE_ENABLE) && TRSDK_VER >= 1000 && TRSDK_VER < 2000
	iviTR_TREXE2_CallDescramble(H264VDec, 46, 1);
#endif
	m_bGlobal_nalu_exist = 0;
	m_dwStopFlag = 1;
	m_bReadyToDecode = FALSE;

	m_pVdoDropManager = new CVideoDropManager;

	m_bSkipNextFrame = FALSE;

	m_pPerfBase = NULL;

	m_bIsFastSeek = FALSE;
	m_dwFastSeekTargetFrameNum = 0;
	m_dwFastSeekDisplayCount = 0;
	m_dwFastSeekDropCount = 0;
	m_dwFastSeekDecodeCount = 0;
}

CH264VDecGMO::~CH264VDecGMO()
{
	if (m_bOpenByGMO)
	{
		m_pH264Dec->CloseH264VDec();

		m_bOpenByGMO = FALSE;
	}

	if (m_pVdoDropManager)
	{
		delete m_pVdoDropManager;
		m_pVdoDropManager = NULL;
	}

	if (m_pH264Dec)
	{
		delete(m_pH264Dec);
		m_pH264Dec = NULL;
	}

	if (m_pCollectOneNalu)
	{
		delete(m_pCollectOneNalu);
		m_pCollectOneNalu = NULL;
	}

}

int CH264VDecGMO::MapFrameTypeForDropManager(int nImgType)
{
	if (nImgType == B_SLICE)
		return E_VDO_FRAME_TYPE_B;
	else if (nImgType == P_SLICE)
		return E_VDO_FRAME_TYPE_P;
	else if (nImgType == I_SLICE)
		return E_VDO_FRAME_TYPE_I;
	else
		return E_VDO_FRAME_TYPE_INVALID;
}

HRESULT STDMETHODCALLTYPE CH264VDecGMO::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
	CheckPointer(ppv, E_POINTER);

	if (riid == IID_IVdoParameter)
		return GetInterface((IUnknown*)(IVdoParameter*) this, ppv);
	else if (riid == IID_IH264VdoParameter)
		return GetInterface((IUnknown*)(IH264VdoParameter*) this, ppv);
	else if (riid == IID_IH264VDecDllAPI)
		return GetInterface((IUnknown*)(IH264VDecDllAPI*) this, ppv);
	else if (riid == IID_IH264VDecDllDXVA)
		return GetInterface((IUnknown*)(IH264VDecDllDXVA*) this, ppv);
	else if (riid == IID_IH264VDecDllAPI2)
		return GetInterface((IUnknown*)(IH264VDecDllAPI2*) this, ppv);
	return CVdoCodecGMO::NonDelegatingQueryInterface(riid, ppv);
}

//IVdoParameter
HRESULT CH264VDecGMO::GetParameter(void* parameter, H264VdoGMOParam::VdoParamType pType /* = H264VdoGMOParam::VDO_PARAM_UNKOWN */) const
{
	HRESULT hr = S_OK;

	if (!m_pH264Dec || !parameter)
		return E_POINTER;

	INT *pValue;
	H264VDecHP_Frame *pFrame;
	switch(pType)
	{
	case H264VdoGMOParam::VDO_PARAM_GET_CURRENT_IMG_TYPE:
		pValue = (INT*)parameter;
		if (m_bReadyToDecode)
			*pValue = m_pH264Dec->GetCurrentImgType();
		else
			*pValue = -1;
		break;
	case H264VdoGMOParam::VDO_PARAM_GET_DISPLAY_IMG_INFORMATION:
		pFrame = (H264VDecHP_Frame*)parameter;
		memcpy(pFrame, &m_StoredOutFrame, sizeof(H264VDecHP_Frame));
		break;
	case H264VdoGMOParam::VDO_PARAM_GET_DISPLAYABLE_FRAME_NUM:
		pValue = (INT*)parameter;
		*pValue = m_pH264Dec->GetDisplayCount();
		break;
	case H264VdoGMOParam::VDO_PARAM_SET_FAST_SEEKING:
		pValue = (INT*)parameter;
		*pValue = m_bIsFastSeek;
		break;
	case H264VdoGMOParam::VDO_PARAM_SET_PREDICT_NEXT_SEEKING_COST:
		break;
	case H264VdoGMOParam::VDO_PARAM_GET_BYTECOUNT:
		pValue = (INT*)parameter;
		*pValue = m_pH264Dec->GetCurrentTotalByteCount_SECOP();
		break;
	case H264VdoGMOParam::VDO_PARAM_UNKOWN:
		break;
	}

	return hr;
}

HRESULT CH264VDecGMO::SetParameter(void* parameter, H264VdoGMOParam::VdoParamType pType /* = H264VdoGMOParam::VDO_PARAM_UNKOWN */)
{
	HRESULT hr = S_OK;

	if (!m_pH264Dec || !parameter)
		return E_POINTER;

	switch(pType)
	{
	case H264VdoGMOParam::VDO_PARAM_OPERATION_RELEASEFRAME:
		hr = m_pH264Dec->ReleaseH264VFrame((DWORD)parameter);
		break;
	case H264VdoGMOParam::VDO_PARAM_SET_OPEN_OPTION:
		m_pH264Dec->UpdateOpenOption(parameter);
		break;
	case H264VdoGMOParam::VDO_PARAM_SET_SMART_DECODE:
		hr = m_pH264Dec->SetH264VDec(H264_PROPID_SMART_DECODE, parameter);
		break;
	case H264VdoGMOParam::VDO_PARAM_SET_NVCTRL_SPEED:
		hr = m_pH264Dec->SetH264VDec(H264_PROPID_NVCTRL_SPEED, parameter);
		break;
	case H264VdoGMOParam::VDO_PARAM_SET_DOWNSAMPLE:
		hr = m_pH264Dec->SetH264VDec(H264_PROPID_DOWNSAMPLE, parameter);
		break;
	case H264VdoGMOParam::VDO_PARAM_SET_EOS:
		hr = m_pH264Dec->SetH264VDec(H264_PROPID_EOS, parameter);
		break;
	case H264VdoGMOParam::VDO_PARAM_SET_DECODER_HURRYUP:
		if (m_pVdoDropManager)
			m_pVdoDropManager->SetSkipLevel(*reinterpret_cast<int *>(parameter));
		break;
	case H264VdoGMOParam::VDO_PARAM_SET_DISPLAY_STATUS:
		hr = m_pH264Dec->GetDisplayStatus(*((DWORD*)parameter));
		break;
	case H264VdoGMOParam::VDO_PARAM_SET_ESTIMATE_NEXT_DISPLAY_TIME:
		hr = m_pH264Dec->FinishDisplay(*((DWORD*)parameter));
		break;
	case H264VdoGMOParam::VDO_PARAM_SET_HVDSERVICE:
		m_pH264Dec->SetIHVDService(parameter);
		break;
	case H264VdoGMOParam::VDO_PARAM_SET_PERF_BASE_POINTER:
		m_pPerfBase = (DWORD*)parameter;
		if (m_pPerfBase)
		{
			m_pPerfBase[PERF_VIDEO_DROPPED_FRAMES] = 0;
			m_pPerfBase[PERF_VIDEO_CODEC_MSEC] = 0;
		}
		break;
	case H264VdoGMOParam::VDO_PARAM_SET_OPEN_OPTION_EX:
		m_pH264Dec->UpdateOpenOptionEx(parameter);
		break;
	case H264VdoGMOParam::VDO_PARAM_SET_FAST_SEEKING:
		if (m_pVdoDropManager)
		{
			DEBUG_INFO("Set VDO_PARAM_SET_FAST_SEEKING!");
			m_bIsFastSeek = TRUE;
			m_dwFastSeekTargetFrameNum = (*((DWORD*)parameter));
			m_dwFastSeekDisplayCount = 0;
			m_dwFastSeekDropCount = 0;
			m_dwFastSeekDecodeCount = 0;
			m_pVdoDropManager->SetSkipLevel(VDO_FRAME_DROP_LEVEL_SKIP_ALL_B);
		}
		break;
	case H264VdoGMOParam::VDO_PARAM_SET_PREDICT_NEXT_SEEKING_COST:
		break;
	case H264VdoGMOParam::VDO_PARAM_SET_APP_FLAG:
		hr = m_pH264Dec->SetAppConstrants(parameter);
		break;
	case H264VdoGMOParam::VDO_PARAM_UNKOWN:
		break;
	}
	return hr;
}

//IMethodImpl Methods
HRESULT CH264VDecGMO::InternalAllocateStreamingResources(void)
{
	m_pH264Dec->OpenH264VDec();
	m_bOpenByGMO = TRUE;

#ifdef TR_ENABLE_NEWMACROS
	bool bDaemonfound = false, bINT1HModify = false;
	iviTR_DAEMON_FINDER(bDaemonfound);
	iviTR_INT1H_CHECK(bINT1HModify);
	if(bDaemonfound || bINT1HModify)
		iviTR_CRASH();
#endif
	//m_pCollectOneNalu = new CCollectOneNalu;

	if (m_pVdoDropManager == NULL)
		m_pVdoDropManager = new CVideoDropManager;

	m_pCollectOneNalu->m_pnalu = AllocNALU(m_pH264Dec->GetStreamBufferSize());	

#if defined(TR_TREXE_ENABLE) && defined(TRSDK_VER) && TRSDK_VER >= 1000 && TRSDK_VER < 2000
	bool t_bChecksum = true;
	iviTR_TREXE2_CallChecksum(H264VDec, 46, 1, t_bChecksum);
	if (!t_bChecksum)
		iviTR_CRASH();
#endif

#if defined(TR_ENABLE_NEWMACROS) && defined(TRSDK_VER) && TRSDK_VER >= 1000 && TRSDK_VER < 2000
	bool bVerified = false, bIntegrated = false ;
	iviTR_VERIFY_SYSER_SYSERBOOT(bVerified);
	iviTR_INT2D_CHECK(bIntegrated);
	if(bVerified||bIntegrated)
		iviTR_EXIT_PROCESS();
#endif

	return S_OK;
}

HRESULT CH264VDecGMO::InternalDiscontinuity(DWORD dwInputStreamIndex)
{
	return S_OK;
}

HRESULT CH264VDecGMO::InternalFlush(void)
{
	if (!m_bOpenByGMO)
		return E_POINTER;

	m_pCollectOneNalu->Reset();
	m_bGlobal_nalu_exist = 0;

	m_bSkipNextFrame = FALSE;

	m_bIsFastSeek = FALSE;
	m_dwFastSeekTargetFrameNum = 0;
	m_dwFastSeekDisplayCount = 0;
	m_dwFastSeekDropCount = 0;
	m_dwFastSeekDecodeCount = 0;

	if (m_pPerfBase)
		m_pPerfBase[PERF_VIDEO_DROPPED_FRAMES] = 0;

	return m_pH264Dec->StopH264VDec(m_dwStopFlag);
}

HRESULT CH264VDecGMO::InternalFreeStreamingResources(void)
{
	m_pH264Dec->CloseH264VDec();

	m_bOpenByGMO = FALSE;

	if (m_pVdoDropManager)
	{
		delete m_pVdoDropManager;
		m_pVdoDropManager = NULL;
	}

	return S_OK;
}

HRESULT CH264VDecGMO::InternalGetInputMaxLatency(DWORD dwInputStreamIndex, REFERENCE_TIME *prtMaxLatency)
{
	return E_NOTIMPL;
}

HRESULT CH264VDecGMO::InternalSetInputMaxLatency(DWORD dwInputStreamIndex, REFERENCE_TIME rtMaxLatency)
{
	return E_NOTIMPL;
}

HRESULT CH264VDecGMO::InternalGetInputSizeInfo(DWORD dwInputStreamIndex, DWORD *pcbSize, DWORD *pulSizeMaxLookahead, DWORD *pulSizeAlignment)
{
	if (dwInputStreamIndex != 0)
		return GMO_E_INVALIDSTREAMINDEX;

	if (!pcbSize || !pulSizeMaxLookahead || !pulSizeAlignment)
		return E_POINTER;

	*pcbSize = 4096; //minimum buffer size
	*pulSizeMaxLookahead = 0; //no lookahead method
	*pulSizeAlignment = 1;

	return S_OK;
}

HRESULT CH264VDecGMO::InternalGetOutputSizeInfo(DWORD dwOutputStreamIndex, DWORD *pcbSize, DWORD *pulSizeAlignment)
{
	if(dwOutputStreamIndex > 1)
		return GMO_E_INVALIDSTREAMINDEX;

	if (!pcbSize || !pulSizeAlignment)
		return E_POINTER;

	
	if (m_dwOutputTypeIndex[dwOutputStreamIndex] == E_VDO_OUTPUT_TYPE_NV12) //NV12
	{
		*pcbSize = 4147200;//1920*1080*2; //minimum buffer size
		*pulSizeAlignment = 16;
	}
	else if (m_dwOutputTypeIndex[dwOutputStreamIndex] == E_VDO_OUTPUT_TYPE_IVIH264) //IVIH264
	{
		*pcbSize = sizeof(H264VDecHP_Frame); //minimum buffer size
		*pulSizeAlignment = 1;
	}
	else if (m_dwOutputTypeIndex[dwOutputStreamIndex] == E_VDO_OUTPUT_TYPE_IVIH264_EX) //IVIH264_EX
	{
		*pcbSize = sizeof(H264VDecHP_FrameEx); //minimum buffer size
		*pulSizeAlignment = 1;
	}
	else if(m_dwOutputTypeIndex[dwOutputStreamIndex] == E_VDO_OUTPUT_TYPE_OFFSET_METADATA)
	{
		*pcbSize = sizeof(H264VDecHP_Offset_Metadata); //minimum buffer size
		*pulSizeAlignment = 1;
	}

	return S_OK;
}

HRESULT CH264VDecGMO::InternalGetInputStreamInfo(DWORD dwInputStreamIndex, DWORD *pdwFlags)
{
	*pdwFlags = GMO_INPUT_STREAMF_WHOLE_SAMPLES;

	return S_OK;
}
HRESULT CH264VDecGMO::InternalGetOutputStreamInfo(DWORD dwOutputStreamIndex, DWORD *pdwFlags)
{
	*pdwFlags = GMO_OUTPUT_STREAMF_OPTIONAL;

	return S_OK;
}
HRESULT CH264VDecGMO::InternalGetInputType(DWORD dwInputStreamIndex, DWORD dwTypeIndex, GMO_MEDIA_TYPE *pmt)
{
	if(dwTypeIndex > 3)
	{
		return GMO_E_NO_MORE_ITEMS;
	}

	//If pmt is NULL, and the type index is in range, we return S_OK
	if( NULL == pmt)
	{
		return S_OK;
	}

	pmt->majortype = MEDIATYPE_Video;
	pmt->subtype = GUID_NULL;
	pmt->formattype = GUID_NULL;	
	pmt->bFixedSizeSamples = 0;
	pmt->bTemporalCompression = false;
	pmt->lSampleSize = 0;
	pmt->pUnk = NULL;
	pmt->cbFormat = 0;
	pmt->pbFormat = NULL;

	switch(dwTypeIndex) {
	case E_VDO_INPUT_TYPE_BLH264:
		pmt->subtype = MEDIASUBTYPE_BLH264;
		break;
	case E_VDO_INPUT_TYPE_MLH264:
		pmt->subtype = MEDIASUBTYPE_MLH264;
		break;
	case E_VDO_INPUT_TYPE_HLH264:
		pmt->subtype = MEDIASUBTYPE_HLH264;
		break;
	case E_VDO_INPUT_TYPE_ELH264:
		pmt->subtype = MEDIASUBTYPE_ELH264;
		break;
	}	

	return S_OK;
}

HRESULT CH264VDecGMO::InternalGetOutputType(DWORD dwOutputStreamIndex, DWORD dwTypeIndex, GMO_MEDIA_TYPE *pmt)
{
	if(dwTypeIndex > 1)
	{
		return GMO_E_NO_MORE_ITEMS;
	}

	if(pmt == NULL)
	{
		return S_OK;
	}

	pmt->majortype = MEDIATYPE_Video;
	pmt->subtype = GUID_NULL;
	pmt->formattype = GUID_NULL;	
	pmt->bFixedSizeSamples = 0;
	pmt->bTemporalCompression = false;
	pmt->lSampleSize = 0;
	pmt->pUnk = NULL;
	pmt->cbFormat = 0;
	pmt->pbFormat = NULL;

	switch(dwTypeIndex) {
	case E_VDO_OUTPUT_TYPE_NV12:
		pmt->subtype = MEDIASUBTYPE_NV12;
		break;
	case E_VDO_OUTPUT_TYPE_IVIH264:
		pmt->subtype = MEDIASUBTYPE_IVIH264;
		break;
	}	

	return S_OK;
}

HRESULT CH264VDecGMO::InternalProcessInput(DWORD dwInputStreamIndex, IGMediaBuffer *pBuffer, DWORD dwFlags, REFERENCE_TIME rtTimeStamp, REFERENCE_TIME rtTimeLength)
{
	HRESULT hr = S_OK;
#if defined(TR_TREXE_ENABLE) && TRSDK_VER >= 1000 && TRSDK_VER < 2000
	iviTR_TREXE2_REGION_DECLARE(H264VDec, 46);
#endif
	hr = pBuffer->GetBufferAndLength(&m_pbInputData, &m_cbInputLength);
	if (FAILED(hr))
		return E_FAIL;

	if (m_cbInputLength)
	{
		m_pCollectOneNalu->SetBufferPointer(m_pbInputData, m_cbInputLength);
		if (rtTimeStamp)
			m_pCollectOneNalu->SetGlobalTimeStamp(rtTimeStamp, rtTimeLength);
	}
	else if (dwFlags & GMO_INPUT_DATA_BUFFERF_SYNCPOINT)
		m_pCollectOneNalu->ChangeDataStatus(E_H264_DATA_STATUS_DATA_DISCONTINUITY);

	return S_OK;
}

HRESULT CH264VDecGMO::InternalProcessOutput(DWORD dwFlags, DWORD OutputBufferCount, GMO_OUTPUT_DATA_BUFFER *pOutputBuffers, DWORD *pdwStatus)
{
	HRESULT hr = S_OK;
	BYTE    *pbData[2]; 
	DWORD   cbData[2];
	DWORD   cbOutputLength = 0;
	CGMOPtr<IGMediaBuffer> pOutputBuffer[2];
	BOOL    bEOS = (dwFlags==GMO_PROCESS_OUTPUT_DRAIN_WHEN_NO_INPUT);

	pbData[0] = NULL;
	cbData[0] = 0;
	pOutputBuffer[0] = pOutputBuffers[0].pBuffer;

	if(OutputBufferCount == 2)
	{
		pbData[1] = NULL;
		cbData[1] = 0;
		pOutputBuffer[1] = pOutputBuffers[1].pBuffer;
	}

#if defined(TR_TREXE_ENABLE) && TRSDK_VER >= 1000 && TRSDK_VER < 2000
	iviTR_TREXE2_REGION_DECLARE(H264VDec, 46);
#endif
	hr = pOutputBuffer[0]->GetBufferAndLength(&pbData[0], &cbData[0]);
	if(SUCCEEDED(hr))
	{
		hr = pOutputBuffer[0]->GetMaxLength(&cbOutputLength);
		if(SUCCEEDED(hr) && (cbOutputLength>=sizeof(H264VDecHP_Frame)))
		{
			if (m_pH264Dec->GetDXVAMode() == E_H264_DXVA_MODE_E || m_pH264Dec->GetDXVAMode() == E_H264_DXVA_INTEL_MODE_E)
				DecodeFrame_BA(bEOS);
			else
				DecodeFrame(bEOS);

			if(m_bIsFastSeek == TRUE)
			{
				while(m_pH264Dec->GetDisplayCount())
				{
					BOOL bIsDropFrame = FALSE;					
					H264VDecHP_Frame *pOutH264VFrame = (H264VDecHP_Frame *)pbData[0];
					int index = m_pH264Dec->GetCurrentOutputViewIndex();
					if(ERROR_SUCCESS != m_pH264Dec->GetH264VFrame(pOutH264VFrame, index))
						hr = pOutputBuffer[0]->SetLength(0);
					else
					{
						if((m_dwFastSeekDisplayCount+m_dwFastSeekDropCount) == m_dwFastSeekTargetFrameNum)
						{
							bIsDropFrame = FALSE;
							m_bIsFastSeek = FALSE;
							m_dwFastSeekDisplayCount = 0;
							m_dwFastSeekTargetFrameNum = 0;
							m_dwFastSeekDropCount = 0;
							m_dwFastSeekDecodeCount = 0;
						}
						else
						{
							bIsDropFrame = TRUE;
							m_dwFastSeekDisplayCount++;
						}

						if(bIsDropFrame == FALSE)
						{
							pOutputBuffer[0]->SetLength(sizeof(H264VDecHP_Frame));
							pOutputBuffers[0].rtTimestamp = pOutH264VFrame->pts.ts;
							pOutputBuffers[0].rtTimelength = pOutH264VFrame->pts.tslength;
							break;
						}
						else
						{
							pOutputBuffer[0]->SetLength(0);
							m_pH264Dec->ReleaseH264VFrame(pOutH264VFrame->dwCookie);
						}
						m_pH264Dec->SetNextOutputViewIndex();
					}
				}
			}
			else
			{
				if (m_pH264Dec->GetDisplayCount())
				{
					if (m_dwOutputTypeIndex[0] == E_VDO_OUTPUT_TYPE_NV12)
					{
						H264VDecHP_Frame *tFrame = &m_StoredOutFrame;
						BYTE *pbRawBuffer = pbData[0];
						memset(tFrame,0,sizeof(H264VDecHP_Frame));
						int index = m_pH264Dec->GetCurrentOutputViewIndex();
						if(ERROR_SUCCESS != m_pH264Dec->GetH264VFrame(tFrame, index))
							hr = pOutputBuffer[0]->SetLength(0);
						else
						{
							DWORD dwLumaSize =  tFrame->adwHeight[0] * tFrame->adwStride[0];
							DWORD dwChromaSize = tFrame->adwHeight[1] * tFrame->adwStride[1];
							if(pbData[0] != NULL)
							{
								memcpy(pbRawBuffer, tFrame->apbFrame[0], dwLumaSize);//Luma
								pbRawBuffer += dwLumaSize;
								memcpy(pbRawBuffer, tFrame->apbFrame[1], dwChromaSize);//Chroma
								tFrame->apbFrame[2] = NULL;
								pOutputBuffer[0]->SetLength(dwLumaSize + dwChromaSize);
								pOutputBuffers[0].rtTimestamp = tFrame->pts.ts;
								pOutputBuffers[0].rtTimelength = tFrame->pts.tslength;
							}
							m_pH264Dec->ReleaseH264VFrame(tFrame->dwCookie); 
							tFrame->dwCookie = 0;
							m_pH264Dec->SetNextOutputViewIndex();
						}
					}
					else if (m_dwOutputTypeIndex[0] == E_VDO_OUTPUT_TYPE_IVIH264)
					{
						H264VDecHP_Frame *pOutH264VFrame = (H264VDecHP_Frame *)pbData[0];
						int index = m_pH264Dec->GetCurrentOutputViewIndex();
						if(ERROR_SUCCESS != m_pH264Dec->GetH264VFrame(pOutH264VFrame, index))
							hr = pOutputBuffer[0]->SetLength(0);
						else
						{
							pOutputBuffer[0]->SetLength(sizeof(H264VDecHP_Frame));
							pOutputBuffers[0].rtTimestamp = pOutH264VFrame->pts.ts;
							pOutputBuffers[0].rtTimelength = pOutH264VFrame->pts.tslength;
							m_pH264Dec->SetNextOutputViewIndex();
						}
					}
					else if (m_dwOutputTypeIndex[0] == E_VDO_OUTPUT_TYPE_IVIH264_EX)
					{
						H264VDecHP_FrameEx *pOutH264VFrame = (H264VDecHP_FrameEx *)pbData[0];
						int index = m_pH264Dec->GetCurrentOutputViewIndex();
						if(ERROR_SUCCESS != m_pH264Dec->GetH264VFrameEx(pOutH264VFrame, index))
							hr = pOutputBuffer[0]->SetLength(0);
						else
						{
							pOutputBuffer[0]->SetLength(sizeof(H264VDecHP_FrameEx));
							pOutputBuffers[0].rtTimestamp = pOutH264VFrame->pts.ts;
							pOutputBuffers[0].rtTimelength = pOutH264VFrame->pts.tslength;
							if(index != 0)
							{	
								if(OutputBufferCount == 2)
								{
									//offset meta data
									hr = pOutputBuffer[1]->GetBufferAndLength(&pbData[1], &cbData[1]);
									if(SUCCEEDED(hr))
									{
										if (m_dwOutputTypeIndex[1] == E_VDO_OUTPUT_TYPE_OFFSET_METADATA)
										{
											H264VDecHP_Offset_Metadata *pOffsetMetadata = (H264VDecHP_Offset_Metadata *)pbData[1];
											hr = m_pH264Dec->GetOffsetMetadata(pOffsetMetadata, pOutH264VFrame->pts.ts, pOutH264VFrame->dwCookie);
											if(hr == S_OK)
											{
												pOutputBuffer[1]->SetLength(sizeof(H264VDecHP_Offset_Metadata));
												pOutputBuffers[1].rtTimestamp = pOutH264VFrame->pts.ts;
												pOutputBuffers[1].rtTimelength = pOutH264VFrame->pts.tslength;
											}
											else
												pOutputBuffer[1]->SetLength(0);
										}
									}
									else
										pOutputBuffer[1]->SetLength(0);
								}
								else
								{
									H264VDecHP_Offset_Metadata OffsetMetadata;
									hr = m_pH264Dec->GetOffsetMetadata(&OffsetMetadata, pOutH264VFrame->pts.ts, pOutH264VFrame->dwCookie);
									if(hr == S_OK)
									{
										m_pH264Dec->Release_Metadata(pOutH264VFrame->dwCookie);
									}
								}
								
							}
							
							m_pH264Dec->SetNextOutputViewIndex();
						}
					}
				}
				else
					hr = pOutputBuffer[0]->SetLength(0);
			}

			if (m_pCollectOneNalu->GetRemainBufferLength())
				pOutputBuffers[0].dwStatus |= GMO_OUTPUT_DATA_BUFFERF_INCOMPLETE;
		}
		else
			return E_FAIL;
	}
	else
		return E_FAIL;

	return S_OK;
}

//HRESULT CH264VDecGMO::SetInputType(DWORD dwInputStreamIndex, IGMediaBuffer *pBuffer, DWORD dwFlags, REFERENCE_TIME rtTimeStamp, REFERENCE_TIME rtTimeLength)
//{
//	return E_NOTIMPL;
//}
//
//HRESULT CH264VDecGMO::SetOutputType(DWORD dwFlags, DWORD OutputBufferCount, GMO_OUTPUT_DATA_BUFFER *pOutputBuffers, DWORD *pdwStatus)
//{
//	return E_NOTIMPL;
//}

//IGMediaObjectImpl Required overides
HRESULT CH264VDecGMO::InternalAcceptingInput(DWORD dwInputStreamIndex)
{
	if (dwInputStreamIndex != 0)
		return GMO_E_INVALIDSTREAMINDEX;

	return (m_pCollectOneNalu->GetRemainBufferLength()) ? GMO_E_NOTACCEPTING:S_OK;
}
HRESULT CH264VDecGMO::InternalCheckInputType(DWORD dwInputStreamIndex, const GMO_MEDIA_TYPE *pmt)
{
	if (!IsEqualGUID(pmt->majortype, MEDIATYPE_Video))
		return E_FAIL;
	if (!IsEqualGUID(pmt->subtype, MEDIASUBTYPE_BLH264) &&
		  !IsEqualGUID(pmt->subtype, MEDIASUBTYPE_MLH264) &&
			!IsEqualGUID(pmt->subtype, MEDIASUBTYPE_HLH264) &&
			!IsEqualGUID(pmt->subtype, MEDIASUBTYPE_ELH264))
		return E_FAIL;

	if (dwInputStreamIndex != 0)
		return GMO_E_INVALIDSTREAMINDEX;

	if (IsEqualGUID(pmt->subtype, MEDIASUBTYPE_BLH264))
		m_dwInputTypeIndex = E_VDO_INPUT_TYPE_BLH264;
	else if (IsEqualGUID(pmt->subtype, MEDIASUBTYPE_MLH264))
		m_dwInputTypeIndex = E_VDO_INPUT_TYPE_MLH264;
	else if (IsEqualGUID(pmt->subtype, MEDIASUBTYPE_HLH264))
		m_dwInputTypeIndex = E_VDO_INPUT_TYPE_HLH264;
	else if (IsEqualGUID(pmt->subtype, MEDIASUBTYPE_ELH264))
		m_dwInputTypeIndex = E_VDO_INPUT_TYPE_ELH264;

	return S_OK;
}
HRESULT CH264VDecGMO::InternalCheckOutputType(DWORD dwOutputStreamIndex, const GMO_MEDIA_TYPE *pmt)
{
	if (!IsEqualGUID(pmt->majortype, MEDIATYPE_Video))
		return E_FAIL;
	if (!IsEqualGUID(pmt->subtype, MEDIASUBTYPE_NV12) &&
		!IsEqualGUID(pmt->subtype, MEDIASUBTYPE_IVIH264))
		return E_FAIL;

	if (dwOutputStreamIndex > 1)
		return GMO_E_INVALIDSTREAMINDEX;

	if (IsEqualGUID(pmt->subtype, MEDIASUBTYPE_NV12))
		m_dwOutputTypeIndex[dwOutputStreamIndex] = E_VDO_OUTPUT_TYPE_NV12;
	else if (IsEqualGUID(pmt->subtype, MEDIASUBTYPE_IVIH264))
	{
		if (pmt->cbFormat == sizeof(H264VDecHP_FrameEx))
			m_dwOutputTypeIndex[dwOutputStreamIndex] = E_VDO_OUTPUT_TYPE_IVIH264_EX;
		else if (pmt->cbFormat == sizeof(H264VDecHP_Frame))
			m_dwOutputTypeIndex[dwOutputStreamIndex] = E_VDO_OUTPUT_TYPE_IVIH264;
		else if(pmt->cbFormat == sizeof(H264VDecHP_Offset_Metadata))
			m_dwOutputTypeIndex[dwOutputStreamIndex] = E_VDO_OUTPUT_TYPE_OFFSET_METADATA;
		else
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CH264VDecGMO::DecodeFrame(BOOL bEOS)
{
	HRESULT hr = S_OK;

	if (!m_pH264Dec || !m_pCollectOneNalu)
		return E_POINTER;

	if(m_pH264Dec->m_bMVCFieldEOS == TRUE)
	{
		hr = m_pH264Dec->CollectOneFrame(bEOS, m_pCollectOneNalu->m_pnalu);

		if(hr == S_FALSE)
			bEOS = FALSE;
		else if(hr == S_OK)
			bEOS = TRUE;
		else
			m_pH264Dec->m_bMVCFieldEOS = FALSE;
	}
	else if(bEOS)
	{
		int pos;
		hr = m_pCollectOneNalu->CollectOneNALU(bEOS, &pos);
		if (SUCCEEDED(hr) && pos>0)
			m_pH264Dec->CollectOneFrame(FALSE, m_pCollectOneNalu->m_pnalu);
		
		hr = m_pH264Dec->CollectOneFrame(bEOS, m_pCollectOneNalu->m_pnalu);
	}
	else
	{
		while(1)
		{
			if (!m_bGlobal_nalu_exist)
			{
				int pos;
				hr = m_pCollectOneNalu->CollectOneNALU(bEOS, &pos);
				if (pos == 0)//Received EndOfStream NALU
					bEOS = TRUE;
			}
			else
			{
				m_bGlobal_nalu_exist = 0;
				hr = CREL_OK;
			}

			if (SUCCEEDED(hr)) //collected one NALU
			{
				hr = m_pH264Dec->CollectOneFrame(bEOS, m_pCollectOneNalu->m_pnalu);
				if (SUCCEEDED(hr)) //collected one Frame -> break to DecodeThisFrame()
					break;
			}
			else
				break;
		}
	}
	
	if(SUCCEEDED(hr) && bEOS == TRUE && m_pH264Dec->IsMVCFieldPicture() && m_pH264Dec->m_bMVCFieldEOS != TRUE)
	{	//For MVC field coding, triggering EOS must wait the last dependent frame decoded.
		m_pH264Dec->m_bMVCFieldEOS = TRUE;

		hr = m_pH264Dec->CollectOneFrame(bEOS, m_pCollectOneNalu->m_pnalu);

		if(hr == S_FALSE)
			bEOS = FALSE;
		else if(hr == S_OK)
			bEOS = TRUE;
		else
			m_pH264Dec->m_bMVCFieldEOS = FALSE;
	}

	if (SUCCEEDED(hr))
	{
		DWORD dwDecodeTimeForPERF = GetTickCount();
		hr = m_pH264Dec->DecodeThisFrame(bEOS, m_bSkipNextFrame, m_bIsFastSeek);
		dwDecodeTimeForPERF = GetTickCount()-dwDecodeTimeForPERF;
		if (m_pPerfBase)
			m_pPerfBase[PERF_VIDEO_CODEC_MSEC] += dwDecodeTimeForPERF;

		m_bGlobal_nalu_exist = 1;

		if (m_pPerfBase && m_bSkipNextFrame)
			m_pPerfBase[PERF_VIDEO_DROPPED_FRAMES] += 1;

		if(m_bIsFastSeek == TRUE)
		{	
			m_dwFastSeekDecodeCount++;

			if(m_dwFastSeekDecodeCount == m_dwFastSeekTargetFrameNum)
				m_pVdoDropManager->SetSkipLevel(VDO_FRAME_DROP_LEVEL_NO_SKIP);

			if(m_dwFastSeekDecodeCount <= m_dwFastSeekTargetFrameNum)
			{
				if(m_bSkipNextFrame == TRUE && m_pH264Dec->IsSeekToOpenGOP() == 0)
					m_dwFastSeekDropCount++;
			}
		}
			
		//drop frame decision
		if(m_pH264Dec->IsNextBaseView())
		{
			int nImgType = m_pH264Dec->GetNextImgType();
			m_bSkipNextFrame = m_pVdoDropManager->CheckFrameDrop(MapFrameTypeForDropManager(nImgType));
			if (nImgType == B_SLICE && m_pH264Dec->IsSkipFirstB() && m_pVdoDropManager->GetSkipLevel()==VDO_FRAME_DROP_LEVEL_SKIP_END_OF_B)
				m_bSkipNextFrame = TRUE;

			if(m_bSkipNextFrame)
				m_pH264Dec->SetNextSkipPOC();
			DEBUG_INFO("[CH264VDecGMO] DecodeFrame() SkipNextFrame: %d", m_bSkipNextFrame);
		}
		else
			m_bSkipNextFrame = m_pH264Dec->IsNextDenpendentViewSkipped();
	}

	return hr;
}

HRESULT CH264VDecGMO::DecodeFrame_BA(BOOL bEOS)
{
	if (!m_pH264Dec || !m_pCollectOneNalu)
		return E_POINTER;

	DWORD dwDecodeTimeForPERF = GetTickCount();
	HRESULT hr = m_pH264Dec->DecodeBistreamDXVA(m_pCollectOneNalu, bEOS, &m_bSkipNextFrame);
	if (SUCCEEDED(hr))
	{
		dwDecodeTimeForPERF = GetTickCount()-dwDecodeTimeForPERF;
		if (m_pPerfBase)
			m_pPerfBase[PERF_VIDEO_CODEC_MSEC] += dwDecodeTimeForPERF;

		if (m_pPerfBase && m_bSkipNextFrame)
			m_pPerfBase[PERF_VIDEO_DROPPED_FRAMES] += 1;

		//drop frame decision
		//mark it due to DXVA VLD mode no need to do drop frame, otherwise it will cause corruption issue by HW
		//int nImgType = m_pH264Dec->GetNextImgType();
		//m_bSkipNextFrame = m_pVdoDropManager->CheckFrameDrop(MapFrameTypeForDropManager(nImgType));
		DEBUG_INFO("[CH264VDecGMO] DecodeFrame_BA() SkipNextFrame: %d", m_bSkipNextFrame);
	}

	return hr;
}

//////////////////////////////////////////////////////////////////////////
//IH264VDecDllAPI
HRESULT CH264VDecGMO::H264VDec_Create(
	OUT H264VDecHP **ppDecoder
	)
{
	if(ppDecoder==NULL)
		return E_POINTER;

	*ppDecoder = new CH264VDecHP;

	return S_OK;
}

HRESULT CH264VDecGMO::H264VDec_Release(
	IN H264VDecHP *pDecoder
	)
{
	if(pDecoder==NULL)
		return E_POINTER;

	return reinterpret_cast<CH264VDecHP *>(pDecoder)->Release();
}

HRESULT CH264VDecGMO::H264VDec_Open(
	IN H264VDecHP *pDecoder,
	IN const H264VDecHP_OpenOptions *pOptions,
	IN const DWORD dwSize,
	OUT void **imgp
	)
{
	if(pDecoder==NULL)
		return E_POINTER;

	H264VDecHP_OpenOptionsEx OpenOptionsEx;
	memset(&OpenOptionsEx, 0, sizeof(H264VDecHP_OpenOptionsEx));
	OpenOptionsEx.dwThreads = pOptions->dwThreads;
	OpenOptionsEx.dwThreadAffinity = pOptions->dwThreadAffinity;
	OpenOptionsEx.dwBuffers = pOptions->dwBuffers;
	OpenOptionsEx.dwFillFrameNumGap = pOptions->dwFillFrameNumGap;

	OpenOptionsEx.pfnDataCallback = pOptions->pfnDataCallback;
	OpenOptionsEx.pvDataContext = pOptions->pvDataContext;
	OpenOptionsEx.dwH264RegKey = pOptions->dwH264RegKey;
	OpenOptionsEx.dxvaVer = pOptions->pAccel.dxvaVer;

	OpenOptionsEx.uiH264VGACard = pOptions->uiH264VGACard;
	OpenOptionsEx.uiH264DXVAMode = pOptions->uiH264DXVAMode;

	OpenOptionsEx.pfnGetParamCallback = NULL;
	OpenOptionsEx.pIviCP = pOptions->pIviCP;

#if !defined(_COLLECT_PIC_)
#ifdef _GLOBAL_IMG_
	return reinterpret_cast<CH264VDecHP *>(pDecoder)->Open(&OpenOptionsEx,dwSize);
#else
	return reinterpret_cast<CH264VDecHP *>(pDecoder)->Open(&OpenOptionsEx,dwSize, (ImageParameters **) imgp);
#endif

#else
	return reinterpret_cast<CH264VDecHP *>(pDecoder)->Open(&OpenOptionsEx, sizeof(H264VDecHP_OpenOptionsEx), imgp);
#endif
}

HRESULT CH264VDecGMO::H264VDec_Close(
	IN H264VDecHP *pDecoder,
	IN void *imgp
	)
{
#if !defined(_COLLECT_PIC_)
	ImageParameters *img = (ImageParameters*) imgp;
#else
	StreamParameters *stream = (StreamParameters*) imgp;
	if(!stream)
		return E_POINTER;

	ImageParameters *img = stream->m_img[0];
#endif

	if(pDecoder==NULL)
		return E_POINTER;

	return reinterpret_cast<CH264VDecHP *>(pDecoder)->Close ARGS0();
}

HRESULT CH264VDecGMO::H264VDec_Stop(
	IN H264VDecHP *pDecoder,
	IN const DWORD dwStopFlags,
	IN void *imgp
	)
{
#if !defined(_COLLECT_PIC_)
	ImageParameters *img = (ImageParameters *) imgp;
#else
	StreamParameters *stream_global = (StreamParameters*) imgp;
	if(!stream_global)
		return E_POINTER;

	ImageParameters *img = stream_global->m_img[0];//&tmp_img;
#endif

	//g_Initial_Flag |= (dwStopFlags==1);

	if(pDecoder==NULL)
		return E_POINTER;

	return reinterpret_cast<CH264VDecHP *>(pDecoder)->Stop ARGS1(dwStopFlags);
}

HRESULT CH264VDecGMO::H264VDec_DecodeFrame(
	IN H264VDecHP *pDecoder,
	IN const H264VDecHP_DecodeOptions *pOptions,
	IN const DWORD dwSize,
	OUT DWORD *pdwNumberOfDecodedFrames,
	OUT DWORD *pdwNumberOfSkippedFrames,
	IN void *imgp
	)
{
#if !defined(_COLLECT_PIC_)
	ImageParameters *img = (ImageParameters*) imgp;
#else	
	StreamParameters *stream_global = (StreamParameters*) imgp;
	if(!stream_global)
		return E_POINTER;

	ImageParameters *img = stream_global->m_img[0];
#endif

	if(g_Initial_Flag)
		g_Initial_Flag = FALSE;

	if(pDecoder==0)
		return E_POINTER;

	return reinterpret_cast<CH264VDecHP *>(pDecoder)->DecodeFrame ARGS4(pOptions,dwSize,pdwNumberOfDecodedFrames,pdwNumberOfSkippedFrames);
}

HRESULT CH264VDecGMO::H264VDec_GetFrame(
	IN H264VDecHP *pDecoder,
	IN const H264VDecHP_GetFrameOptions *pOptions,
	IN const DWORD dwSize,
	OUT H264VDecHP_Frame *pFrame,
	IN const DWORD dwFrameSize,
	IN void *imgp
	)
{
#if !defined(_COLLECT_PIC_)
	ImageParameters *img = (ImageParameters*) imgp;
#else
	StreamParameters *stream = (StreamParameters*) imgp;
	static unsigned int view_id = 0;
	if(!stream)
		return E_POINTER;

	ImageParameters *img = stream->m_img[0];
#endif

	if(pDecoder==NULL)
		return E_POINTER;

	return reinterpret_cast<CH264VDecHP *>(pDecoder)->GetFrame ARGS5(pOptions,dwSize,pFrame,dwFrameSize, view_id);
	view_id = 1 - view_id;
}

HRESULT CH264VDecGMO::H264VDec_ReleaseFrame(
	IN PVOID pDecoder,
	IN DWORD dwCookie,
	IN void *imgp
	)
{
#if !defined(_COLLECT_PIC_)
	ImageParameters *img = (ImageParameters *) imgp;
#else
	StreamParameters *stream = (StreamParameters*) imgp;
	if(!stream)
		return E_POINTER;

	ImageParameters *img = stream->m_img[0];
#endif

	if(pDecoder==NULL)
		return E_POINTER;

	return reinterpret_cast<CH264VDecHP *>(pDecoder)->ReleaseFrame ARGS1(dwCookie);
}

HRESULT CH264VDecGMO::H264VDec_ReleaseBuffer(
	IN PVOID pDecoder,
	IN const DWORD dwReleaseFlags,
	OUT LPBYTE *ppBufferPtr
	)
{
	if(pDecoder==NULL)
		return E_POINTER;

	return reinterpret_cast<CH264VDecHP *>(pDecoder)->ReleaseBuffer (dwReleaseFlags, ppBufferPtr);
}

HRESULT CH264VDecGMO::H264VDec_Get(
	IN H264VDecHP *pDecoder,
	IN DWORD dwPropID,
	IN LPVOID pPropData,
	IN DWORD cbPropData,
	OUT DWORD *pcbReturned
	)
{
	if(pDecoder==NULL)
		return E_POINTER;

	return reinterpret_cast<CH264VDecHP *>(pDecoder)->Get(dwPropID, pPropData, cbPropData, pcbReturned);
}

HRESULT CH264VDecGMO::H264VDec_Set(
	IN H264VDecHP *pDecoder,
	IN DWORD dwPropID,
	IN LPVOID pPropData,
	IN DWORD cbPropData
	)
{
	if(pDecoder==NULL)
		return E_POINTER;

	return reinterpret_cast<CH264VDecHP *>(pDecoder)->Set(dwPropID, pPropData, cbPropData);
}

HRESULT CH264VDecGMO::H264VDec_AddRef(
	IN H264VDecHP *pDecoder
	)
{
	if(pDecoder==NULL)
		return 0;

	return reinterpret_cast<CH264VDecHP *>(pDecoder)->AddRef();
}

HRESULT CH264VDecGMO::H264VDec_AddRefFrame(
	IN PVOID pDecoder,
	IN DWORD dwCookie
	)
{
	if(pDecoder==NULL)
		return 0;

	return reinterpret_cast<CH264VDecHP *>(pDecoder)->AddRefFrame(dwCookie);
}

//IH264VDecDllDXVA
HRESULT CH264VDecGMO::H264VDec_Reset_DXVA(
	IN LPVOID pVideoAccel,
	IN DWORD dwCookie,
	IN PVOID pDecoder,
	IN void *imgp
	)
{
	return 0;
}

HRESULT CH264VDecGMO::H264VDec_Release_DXVA(
	IN PVOID pDecoder,
	void *imgp
	)
{
	return 0;
}

//IH264VDecDllAPI2
HRESULT CH264VDecGMO::H264VDec_GetDisplayStatus(
	IN H264VDecHP *pDecoder,
	IN const DWORD dwIdx,
	IN void *imgp
	)
{
#if !defined(_COLLECT_PIC_)
	ImageParameters *img = (ImageParameters*) imgp;
#else
	StreamParameters *stream = (StreamParameters*) imgp;
	if(!stream)
		return E_POINTER;

	ImageParameters *img = stream->m_img[0];
#endif

	if(pDecoder==NULL)
		return E_POINTER;

	return reinterpret_cast<CH264VDecHP *>(pDecoder)->GetDisplayStatus ARGS1(dwIdx);
}

HRESULT CH264VDecGMO::H264VDec_FinishDisplay(
	IN H264VDecHP *pDecoder,
	IN DWORD dwEstimateNextFrame,
	IN void *imgp
	)
{
#if !defined(_COLLECT_PIC_)
#if !defined(_GLOBAL_IMG_)
	ImageParameters *img = (ImageParameters*) imgp;
#endif
#else
	StreamParameters *stream = (StreamParameters*) imgp;
	if(!stream)
		return E_POINTER;

	ImageParameters *img = stream->m_img[0];
#endif

	if(pDecoder==NULL)
		return E_POINTER;

	return reinterpret_cast<CH264VDecHP *>(pDecoder)->FinishDisplay ARGS1(dwEstimateNextFrame);
}

HRESULT CH264VDecGMO::H264VDec_OpenKey(
	IN H264VDecHP *pDecoder,
	IN H264VDec_OnOpenKeyFcn pCallBack,
	IN LPVOID pParam
	)
{
	if(pDecoder==NULL || pCallBack==NULL)
		return E_POINTER;

#if defined (_USE_SCRAMBLE_DATA_)
	return reinterpret_cast<CH264VDecHP *>(pDecoder)->OpenKey(pCallBack, pParam);
#endif

}
