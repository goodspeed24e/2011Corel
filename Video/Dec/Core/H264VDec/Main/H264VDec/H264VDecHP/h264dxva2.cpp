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
// 	Copyright (c) 2008 InterVideo Corporation.  All Rights Reserved.
//
//-----------------------------------------------------------------------------

#include "h264dxva2.h"
#include "atih264.h"
#include "image.h"
#include "nalu.h"
#include "parset.h"
#include "header.h"
#include "sei.h"
#include "annexb.h"
#include "dxva_error.h"
#include "mb_access.h"
#include "amddecext.h"

#include "emmintrin.h"
#include <bitset>

using std::bitset;

extern CREL_RETURN reorder_lists PARGS2(int currSliceType, Slice * currSlice);
extern CREL_RETURN check_lists PARGS0();

#define Alignment16(a) ((int)((a+15)>>4)<<4)

static const byte peano_raster_single[4][4] = {
	0, 1, 4, 5,
	2, 3, 6, 7,
	8, 9, 12, 13,
	10, 11, 14, 15
};

static byte QP_SCALE_CR_Intel[52]=
{
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,
	12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,
	28,29,29,30,31,32,32,33,34,34,35,35,36,36,37,37,
	37,38,38,38,39,39,39,39
};

extern unsigned char cbp_blk_chroma[8][4];

extern void calc_chroma_vector_adjustment PARGS2(int list_offset, int curr_mb_field);

extern void mem16_2( LPBYTE d1, LPBYTE d2, const BYTE* s, int _size );
extern void mem8_2( LPBYTE dUV, LPBYTE dU, LPBYTE dV, const BYTE* sU, const BYTE* sV, int _size );

extern void iDCT_4x4_2_ATI(short* dest, short *src, short dest_stride);

extern void UpdatePTS PARGS0();

CH264DXVA2::CH264DXVA2()
{
	stream_global = NULL;

	memset(m_nCompBufSize, 0, sizeof(int)*23);
	memset(m_bCompBufUsed, 0, sizeof(m_bCompBufUsed));

	m_lpBitstreamBuf_Intel = NULL;
	m_nSliceNum = 0;

	m_dwTotalByteCount = 0;

}

CH264DXVA2::~CH264DXVA2()
{

}

void CH264DXVA2::SetIHVDService(IHVDService *pIHVDService, H264VDecHP_OpenOptionsEx *pOptions)
{
	pIHVDService->QueryInterface(__uuidof(IHVDServiceDxva2), (void**)&m_pIHVDServiceDxva2);
	return CH264DXVABase::SetIHVDService(pIHVDService, pOptions);
}

int CH264DXVA2::Open PARGS1(int nSurfaceFrame)
{
	stream_global = IMGPAR stream_global;
	m_bResidualDataFormat = IMGPAR bResidualDataFormat;

	m_iMVCFieldNeedExecuteIndex = -1;
	CH264DXVABase::Open ARGS1(nSurfaceFrame);

	EnterCriticalSection(&m_cs);
	GetAllCompressBufferSizeInfo();

	if (m_bConfigBitstreamRaw) //VLD mode
	{
		m_lpnalu = (BYTE*)_aligned_malloc(m_nCompBufSize[DXVA2_BitStreamDateBufferType], 16);

		if (nalu->max_size > m_nCompBufSize[DXVA2_BitStreamDateBufferType]) {
			for (int i = 0; i < IMG_NUM; i++) {				
				stream_global->m_img[i]->m_nalu->max_size = m_nCompBufSize[DXVA2_BitStreamDateBufferType];			
			}
			stream_global->m_nalu_global->max_size = m_nCompBufSize[DXVA2_BitStreamDateBufferType];
		}

		if(m_nVGAType == E_H264_VGACARD_INTEL)
		{
			m_lpBitstreamBuf_Intel = (BYTE*)_aligned_malloc(m_nCompBufSize[DXVA2_BitStreamDateBufferType], 16);
			m_lpBitstreamBuf = m_lpBitstreamBuf_Intel;
		}
	}
	else //MC and IT mode
	{
		for (int i=0; i<MAX_COMP_BUF_FOR_DXVA2; i++)
		{
			m_pbPictureParamBuf[i]   = (BYTE*)_aligned_malloc(m_nCompBufSize[DXVA2_PictureParametersBufferType], 16);
			m_pbSliceCtrlBuf[i]      = (BYTE*)_aligned_malloc(m_nCompBufSize[DXVA2_SliceControlBufferType], 16);
			if (m_nDXVAMode==E_H264_DXVA_MODE_C)
				m_pbIQMatrixBuf[i] = (BYTE*)_aligned_malloc(m_nCompBufSize[DXVA2_InverseQuantizationMatrixBufferType], 16);
			m_pbMacroblockCtrlBuf[i] = (BYTE*)_aligned_malloc(m_nCompBufSize[DXVA2_MacroBlockControlBufferType], 16);
			m_pbMotionVectorBuf[i]   = (BYTE*)_aligned_malloc(m_nCompBufSize[DXVA2_MotionVectorBuffer], 16);
			m_pbResidualDiffBuf[i]   = (BYTE*)_aligned_malloc(m_nCompBufSize[DXVA2_ResidualDifferenceBufferType], 16);
			m_bCompBufStaus[i] = TRUE;
		}

		//////////////////////////////////////////////////////////////////////////
		// Use Queue for DXVA2 MC and IT mode
		unsigned int id;
		char tmp_event_handle_name[128];

		m_bRunThread = TRUE;
		m_Queue = new queue <ExecuteBuffersStruct>;
		InitializeCriticalSection( &crit_ACCESS_QUEUE );

		stream_global->m_queue_semaphore = CreateSemaphore(
			NULL, 
			0,
			20,
			NULL
			);

		sprintf(tmp_event_handle_name, "event_for_end_thread\n");
		m_hEndCpyThread = (HANDLE) CreateEvent(NULL, true, false, NULL);
		stream_global->h_dxva_queue_reset_done  = (HANDLE) CreateEvent(NULL, false, false, tmp_event_handle_name);
		stream_global->m_dxva_queue_reset = 0;
		int i = 20;

		EnterCriticalSection( &crit_ACCESS_QUEUE );
		while(i)
		{
			sprintf(tmp_event_handle_name, "event_for_render_%d\n", --i);
			stream_global->hReadyForRender[i] = (HANDLE)CreateEvent(NULL, true, false, tmp_event_handle_name);
			SetEvent(stream_global->hReadyForRender[i]);
		}
		LeaveCriticalSection( &crit_ACCESS_QUEUE );

		m_dwThreadExecuteHandle = _beginthreadex(0, 0, ThreadExecute, (void *) this, 0, &id);
	}

	LeaveCriticalSection(&m_cs);
	return S_OK;
}

int CH264DXVA2::Close PARGS0()
{
	if (m_bConfigBitstreamRaw) //VLD mode
	{
		_aligned_free(m_lpnalu);
		m_lpnalu = NULL;

		if(m_nVGAType == E_H264_VGACARD_INTEL)
		{
			_aligned_free(m_lpBitstreamBuf_Intel);
			m_lpBitstreamBuf_Intel = NULL;
		}
	}
	else //MC and IT mode
	{
		for (int i=0; i<MAX_COMP_BUF_FOR_DXVA2; i++)
		{
			_aligned_free(m_pbPictureParamBuf[i]);
			_aligned_free(m_pbSliceCtrlBuf[i]);
			if (m_nDXVAMode==E_H264_DXVA_MODE_C)
				_aligned_free(m_pbIQMatrixBuf[i]);
			_aligned_free(m_pbMacroblockCtrlBuf[i]);
			_aligned_free(m_pbMotionVectorBuf[i]);
			_aligned_free(m_pbResidualDiffBuf[i]);
		}

		int i=20;
		m_bRunThread = FALSE;
		ReleaseSemaphore(stream_global->m_queue_semaphore, 1, NULL);
		WaitForSingleObject(m_hEndCpyThread, INFINITE);
		CloseHandle(m_hEndCpyThread);
		CloseHandle(stream_global->m_queue_semaphore);
		CloseHandle(stream_global->h_dxva_queue_reset_done);
		while(i)
			CloseHandle(stream_global->hReadyForRender[--i]);
		DeleteCriticalSection( &crit_ACCESS_QUEUE );

		CloseHandle((HANDLE)m_dwThreadExecuteHandle);
	}

	if (m_pIHVDServiceDxva2)
		m_pIHVDServiceDxva2->Release();

	m_iMVCFieldNeedExecuteIndex = -1;
	return CH264DXVABase::Close ARGS0();
}

void CH264DXVA2::ResetDXVABuffers()
{
	CH264DXVABase::ResetDXVABuffers();
	GetAllCompressBufferSizeInfo();
}

void CH264DXVA2::GetCompressBufferSizeInfo(int nBufferType)
{
	void *pBuffer;
	HVDService::HVDDxvaCompBufLockInfo HVDDxvaCompInfo;
	ZeroMemory(&HVDDxvaCompInfo, sizeof(HVDService::HVDDxvaCompBufLockInfo));
	HRESULT hr = m_pIHVDServiceDxva2->LockCompressBuffer(nBufferType, 0, &HVDDxvaCompInfo, TRUE);
	if(FAILED(hr)) 
		return;

	pBuffer = HVDDxvaCompInfo.pBuffer;
	m_nCompBufSize[nBufferType] = HVDDxvaCompInfo.uSize;

	m_pIHVDServiceDxva2->UnlockCompressBuffer(nBufferType, 0);
}

void CH264DXVA2::GetAllCompressBufferSizeInfo()
{
	GetCompressBufferSizeInfo(DXVA2_PictureParametersBufferType);
	GetCompressBufferSizeInfo(DXVA2_SliceControlBufferType);
	if(m_bConfigBitstreamRaw)
	{
		GetCompressBufferSizeInfo(DXVA2_InverseQuantizationMatrixBufferType);
		GetCompressBufferSizeInfo(DXVA2_BitStreamDateBufferType);
	}
	else
	{
		if (m_nDXVAMode==E_H264_DXVA_MODE_C)
			GetCompressBufferSizeInfo(DXVA2_InverseQuantizationMatrixBufferType);
		GetCompressBufferSizeInfo(DXVA2_MacroBlockControlBufferType);
		GetCompressBufferSizeInfo(DXVA2_ResidualDifferenceBufferType);
		GetCompressBufferSizeInfo(DXVA2_MotionVectorBuffer);
	}
}

void CH264DXVA2::BA_ResetLastCompBufStaus()
{
	DEBUG_SHOW_HW_INFO("[CH264DXVA2] BA_ResetLastCompBufStaus()");
	img_par *img = stream_global->m_img[0];

	AMVABUFFERINFO *t_psBufferInfo = IMGPAR m_pBufferInfo;
	for (UINT i=0; i<4; i++)
	{
		if (FAILED(ReleaseBuffer(t_psBufferInfo[i].dwTypeIndex)))
			break;
	}
}

HRESULT CH264DXVA2::GetBuffer(DWORD dwBufferType, LPVOID *ppBuffer, DWORD *pBufferSize)
{
	HRESULT hr = S_OK;

	if(!m_pIHVDServiceDxva2)
		return E_POINTER;

	m_bCompBufUsed[dwBufferType] = TRUE;

	HVDService::HVDDxvaCompBufLockInfo HVDDxvaCompInfo;
	ZeroMemory(&HVDDxvaCompInfo, sizeof(HVDService::HVDDxvaCompBufLockInfo));
	hr = m_pIHVDServiceDxva2->LockCompressBuffer(dwBufferType, 0, &HVDDxvaCompInfo, TRUE);
	DEBUG_SHOW_HW_INFO("[DXVA2] GetBuffer() BufferType = %d, return value = %d", dwBufferType, hr);
	if (SUCCEEDED(hr))
	{
		if (ppBuffer)
			(*ppBuffer) = HVDDxvaCompInfo.pBuffer;
		if (pBufferSize)
			(*pBufferSize) = HVDDxvaCompInfo.uSize;
	}

	return hr;
}

HRESULT CH264DXVA2::ReleaseBuffer(DWORD dwBufferType)
{
	HRESULT hr = S_OK;

	if(!m_pIHVDServiceDxva2)
		return E_POINTER;

	if (m_bCompBufUsed[dwBufferType])
	{
		hr = m_pIHVDServiceDxva2->UnlockCompressBuffer(dwBufferType, 0);
		DEBUG_SHOW_HW_INFO("[DXVA2] ReleaseBuffer() BufferType = %d, return value = %d", dwBufferType, hr);
		if (SUCCEEDED(hr))
			m_bCompBufUsed[dwBufferType] = FALSE;
	}

	return hr;
}

HRESULT CH264DXVA2::BeginFrame(DWORD dwDestSurfaceIndex, DWORD dwFlags)
{
	HRESULT hr = S_OK;

	while(1)
	{
		hr = m_pIHVDServiceDxva2->BeginFrame(dwDestSurfaceIndex);
		DEBUG_SHOW_HW_INFO("[DXVA2] BeginFrame() DestSurfaceIndex = %d, return value = %d", dwDestSurfaceIndex, hr);
		if (checkDDError(hr))
		{
			Sleep(2);
			if (dwFlags) //return to change "dwDestSurfaceIndex"
				break;
		}
		else
			break;
	}

	return hr;
}

HRESULT CH264DXVA2::Execute(DWORD dwFunction, LPVOID lpPrivateInputData, DWORD cbPrivateInputData, LPVOID lpPrivateOutputData, DWORD cbPrivateOutputData, DWORD dwNumBuffers, AMVABUFFERINFO *pamvaBufferInfo)
{
	HVDService::HVDDxvaExecuteConfig HVDDxvaExecute;
	ZeroMemory(&HVDDxvaExecute, sizeof(HVDService::HVDDxvaExecuteConfig));
	HVDDxvaExecute.dwNumBuffers = dwNumBuffers;
	HVDDxvaExecute.lpAmvaBufferInfo = (LPVOID)(pamvaBufferInfo);
	HVDDxvaExecute.dwFunction = dwFunction;
	HVDDxvaExecute.lpPrivateInputData = lpPrivateInputData;
	HVDDxvaExecute.cbPrivateInputData = cbPrivateInputData;
	HVDDxvaExecute.lpPrivateOutputData = lpPrivateOutputData;
	HVDDxvaExecute.cbPrivateOutputData = cbPrivateOutputData;

	DEBUG_SHOW_HW_INFO("[DXVA2] Execute()");

	return m_pIHVDServiceDxva2->Execute(&HVDDxvaExecute);
}

HRESULT CH264DXVA2::EndFrame()
{
	DEBUG_SHOW_HW_INFO("[DXVA2] EndFrame()");
	return m_pIHVDServiceDxva2->EndFrame();
}

CREL_RETURN CH264DXVA2::decode_one_picture_short PARGS2(int* header, BOOL bSkip)
{
	DecodingEnvironment *dep;
	DXVA_Slice_H264_Short *BA_DATA_CTRL;
	int AU_HasSPS = 0;
	int primary_pic_type = -1;
	int new_pps = 0;
	CREL_RETURN ret = CREL_OK;
	int shiftoffset;
	int nalu_size;
	Slice *newSlice;
	static BOOL bSeekIDR_backup;

	BOOL m_newpps = false;
	BOOL m_newsps = false;
	BOOL bFirstSlice = (IMGPAR slice_number==0) ? TRUE:FALSE;

	if (bFirstSlice)
		nalu->buf = IMGPAR ori_nalu_buf;

	AU_HasSPS	= nalu_global->AU_HasSPS;
	nalu->startcodeprefix_len	= nalu_global->startcodeprefix_len;
	nalu->len					= nalu_global->len;
	nalu->max_size				= nalu_global->max_size;
	nalu->nal_unit_type			= nalu_global->nal_unit_type;
	nalu->nal_reference_idc		= nalu_global->nal_reference_idc;
	nalu->forbidden_bit			= nalu_global->forbidden_bit;
	nalu->pts					= nalu_global->pts;
	memcpy(nalu->buf, nalu_global->buf, nalu_global->len);

	nalu_global_available = 0;			
	if(nalu->nal_unit_type == NALU_TYPE_SPS)
	{
		m_newsps = false;
		m_newpps = false;
	}
	else if(nalu->nal_unit_type == NALU_TYPE_PPS)
		m_newpps = false;

	unsigned char cNALUType = 0;
	unsigned char cMetaDataSize = 0;
	DWORD dwNALUByteCount = 0;
	DWORD dwSliceHandle = 0;
	DWORD dwOffset = 0;
	switch (nalu->nal_unit_type)
	{
	case NALU_TYPE_SECOP:
		DEBUG_SHOW_HW_INFO("-- !! NALU_TYPE_SECOP - ERASED_SLICE !! --");

		if (nalu->buf[0] == 0xff) //erased_slice_descriptor_start_code
		{
			if (bFirstSlice)
			{
				memset(&(IMGPAR sSliceInfoArray[0]), 0, 4096*sizeof(SECOP_SliceInfo));
				IMGPAR dwSliceCountForSECOP = 0;
			}
			//original nal_unit_type
			nalu->nal_unit_type = cNALUType = nalu->buf[1] & 0x1f;

			dwNALUByteCount |= nalu->buf[2]<<16;
			dwNALUByteCount |= nalu->buf[3]<<8;
			dwNALUByteCount |= nalu->buf[4];

			cMetaDataSize = nalu->buf[5]&0x7f;
			cMetaDataSize -= m_nNALUSkipByte;

			dwOffset = 5 + cMetaDataSize + 1;
			dwSliceHandle |= (dwOffset < nalu->len) ? (nalu->buf[dwOffset]):0;
			dwSliceHandle |= (dwOffset+1 < nalu->len) ? (nalu->buf[dwOffset+1]<<8):0;
			dwSliceHandle |= (dwOffset+2 < nalu->len) ? (nalu->buf[dwOffset+2]<<16):0;
			dwSliceHandle |= (dwOffset+3 < nalu->len) ? (nalu->buf[dwOffset+3]<<24):0;

			IMGPAR sSliceInfoArray[IMGPAR dwSliceCountForSECOP].dwSliceByteCount = dwNALUByteCount;
			IMGPAR sSliceInfoArray[IMGPAR dwSliceCountForSECOP].dwSliceHandle = dwSliceHandle;

			m_dwTotalByteCount += (dwNALUByteCount + cMetaDataSize + m_nNALUSkipByte);

			DP("[SECOP] Prevention Codes Skip Bytes: %d MetaDataSize: %d", m_nNALUSkipByte, cMetaDataSize);
			IMGPAR dwSliceCountForSECOP++;
		}
	case NALU_TYPE_SLICE:
	case NALU_TYPE_IDR:
	case NALU_TYPE_SLICE_EXT:
		
		DEBUG_SHOW_HW_INFO("-- NALU_TYPE_SLICE -- NALU_TYPE_IDR --");

		ret = malloc_new_slice(&newSlice);
		if (FAILED(ret)) {
			break;
		}

		if(nalu->nal_unit_type == NALU_TYPE_IDR)
		{
			DEBUG_SHOW_HW_INFO("-- This is IDR --");
			IMGPAR idr_flag = newSlice->idr_flag = 1;
		}
		else
			IMGPAR idr_flag = newSlice->idr_flag = 0;

		IMGPAR nal_reference_idc = newSlice->nal_reference_idc = nalu->nal_reference_idc;
		IMGPAR disposable_flag = newSlice->disposable_flag = (nalu->nal_reference_idc == NALU_PRIORITY_DISPOSABLE);

		newSlice->dp_mode = PAR_DP_1;
		newSlice->max_part_nr = 1;
		newSlice->ei_flag = 0;
		newSlice->nextSlice = NULL;
		dep              = newSlice->g_dep;
		dep->Dei_flag    = 0;
		dep->Dbits_to_go = 0;
		dep->Dbuffer     = 0;

		if (nalu->nal_unit_type == NALU_TYPE_SLICE_EXT) {

			ret = ProcessNaluExt ARGS1(nalu);

			dep->Dbasestrm = &(nalu->buf[4]);
			dep->Dstrmlength = nalu->len-4;
		} else 
		{
			if (cMetaDataSize)
			{
				dep->Dbasestrm = &(nalu->buf[6]);
				dep->Dstrmlength = cMetaDataSize;
			}
			else
			{
				dep->Dbasestrm = &(nalu->buf[1]);
				dep->Dstrmlength = nalu->len-1;
			}
		}
		ret = RBSPtoSODB(dep->Dbasestrm, &(dep->Dstrmlength));
		if (FAILED(ret)) {
			break;
		}
		dep->Dcodestrm   = dep->Dbasestrm;

		if (nalu->nal_unit_type == NALU_TYPE_SLICE_EXT) {

			CopyNaluExtToSlice ARGS1 (newSlice);
			newSlice->bIsBaseView = false;
			stream_global->nalu_mvc_extension.bIsPrefixNALU = FALSE;
			//stream_global->nalu_mvc_extension.valid = FALSE;

		}
		else
		{
			newSlice->bIsBaseView = true;
			if(stream_global->m_active_sps_on_view[1] == 0)
			{
				newSlice->viewId = GetBaseViewId ARGS0 ();
				if(stream_global->nalu_mvc_extension.valid && stream_global->nalu_mvc_extension.bIsPrefixNALU && newSlice->viewId == stream_global->nalu_mvc_extension.viewId)
				{
					newSlice->interViewFlag = stream_global->nalu_mvc_extension.interViewFlag;
					newSlice->anchorPicFlag = stream_global->nalu_mvc_extension.anchorPicFlag;
				}
				else
				{
					newSlice->interViewFlag = 1;
					newSlice->anchorPicFlag = 0;
				}
			}
			else
			{
				if(stream_global->nalu_mvc_extension.valid && stream_global->nalu_mvc_extension.bIsPrefixNALU)
				{
					newSlice->viewId = stream_global->nalu_mvc_extension.viewId;
					newSlice->interViewFlag = stream_global->nalu_mvc_extension.interViewFlag;
					newSlice->anchorPicFlag = stream_global->nalu_mvc_extension.anchorPicFlag;
				}
				else
				{
					newSlice->viewId = stream_global->m_active_sps_on_view[1]->view_id[0];
					newSlice->interViewFlag = 1;
					newSlice->anchorPicFlag = 0;
				}
			}
		}
		newSlice->viewIndex = GetViewIndex ARGS1 (newSlice->viewId);

		if(newSlice->viewIndex > 0)
		{
			if(img != img_array[newSlice->viewIndex])
			{
				BOOL bFieldPicFlag = (IMGPAR firstSlice)? IMGPAR firstSlice->field_pic_flag: FALSE;
				if(bFieldPicFlag)
				{
					img = img_array[newSlice->viewIndex];
					ret = decode_one_picture_short ARGS2(header, bSkip);
					return ret;
				}
			}
		}
		stream_global->m_pbValidViews[newSlice->viewIndex] = 1;
		if(stream_global->m_CurrOutputViewIndex == -1)
			stream_global->m_CurrOutputViewIndex = newSlice->viewIndex;

		if(bFirstSlice)
		{				
			IMGPAR prevSlice  = NULL;
			IMGPAR firstSlice = newSlice;
			IMGPAR currentSlice  = IMGPAR firstSlice;
		}
		else
		{
			IMGPAR prevSlice  = IMGPAR currentSlice;
			IMGPAR currentSlice->nextSlice = (void*)newSlice;
			IMGPAR currentSlice = (Slice*) (IMGPAR currentSlice->nextSlice);
		}

		if (bFirstSlice) {
			bSeekIDR_backup = stream_global->m_bSeekIDR;
		}	

		ret = ParseSliceHeader ARGS0();
		
		if ( FAILED(ret) ) {

			if (bSeekIDR_backup) {
				free_new_slice(newSlice);
				newSlice = NULL;		

				stream_global->m_bSeekIDR = TRUE;

				nalu->buf = IMGPAR ori_nalu_buf;					
				IMGPAR slice_number = 0;
				break;
			}

			free_new_slice(newSlice);
			newSlice = NULL;					

			if (stream_global->m_bSeekIDR) {
				nalu->buf = IMGPAR ori_nalu_buf;					
				IMGPAR slice_number = 0;

			} else if (nalu->nal_unit_type == NALU_TYPE_IDR) {
				stream_global->m_bSeekIDR = TRUE;
				nalu->buf = IMGPAR ori_nalu_buf;

			}				

			break;
			
		}		

		IMGPAR currentSlice->AU_type = IMGPAR currentSlice->picture_type;			
		if(AU_HasSPS && IMGPAR currentSlice->AU_type==I_SLICE)
			IMGPAR currentSlice->AU_type = I_GOP;			
		if(IMGPAR currentSlice->idr_flag)
			IMGPAR currentSlice->AU_type = IDR_SLICE;
		if(IMGPAR currentSlice->picture_type == B_SLICE && IMGPAR currentSlice->nal_reference_idc )
			IMGPAR currentSlice->AU_type = RB_SLICE;

		DEBUG_SHOW_HW_INFO("This Slice AU Type: %d", IMGPAR currentSlice->AU_type);

		if (stream_global->bSeekToOpenGOP)
		{
			if (IMGPAR currentSlice->picture_type != B_SLICE)
			{
				stream_global->bSeekToOpenGOP = 0;
				next_image_type = IMGPAR type;
				bSkip = 0; //reset skip flag
			}
			else// if (IMGPAR currentSlice->AU_type != RB_SLICE)
			{
				nalu->buf = IMGPAR ori_nalu_buf;
				if (IMGPAR prevSlice)
				{
					IMGPAR currentSlice = IMGPAR prevSlice;
					IMGPAR currentSlice->nextSlice = NULL;
				}
				free_new_slice(newSlice);
				newSlice = NULL;

				if (g_has_pts)
				{
					g_has_pts = 0;
					DEBUG_INFO("[Drop PTS]: ts: %I64d freq: %d, tslength: %d flags: %d unused1: %d unused2: %d", nalu->pts.ts, nalu->pts.freq, nalu->pts.tslength, nalu->pts.flags, nalu->pts.unused1, nalu->pts.unused2);
				}
				break;
			}

		}

		if ((stream_global->profile_idc == 0) || 
			((g_bReceivedFirst == 0) && (IMGPAR firstSlice->picture_type != I_SLICE) && !(stream_global->bMVC && IMGPAR firstSlice->viewIndex != 0 && IMGPAR firstSlice->field_pic_flag == TRUE))
			)
		{
			nalu->buf = IMGPAR ori_nalu_buf;
			free_new_slice(newSlice);
			newSlice = NULL;
			IMGPAR slice_number = 0;
			m_newpps = FALSE;
			m_newsps = FALSE;

			for ( unsigned int i = 0; i < stream_global->num_views; i++) {
				stream_global->m_active_sps_on_view[i]= 0;
			}

			if (g_has_pts)
			{
				g_has_pts = 0;
				DEBUG_INFO("[Drop PTS]: ts: %I64d freq: %d, tslength: %d flags: %d unused1: %d unused2: %d", nalu->pts.ts, nalu->pts.freq, nalu->pts.tslength, nalu->pts.flags, nalu->pts.unused1, nalu->pts.unused2);
			}
			break;
		}

		if(is_new_picture ARGS0())
		{
			seq_parameter_set_rbsp_t *sps;
			pic_parameter_set_rbsp_t *pps;
	
			ret = decode_poc ARGS0();

			if (FAILED(ret)) {

				if (bSeekIDR_backup) {							

					stream_global->m_bSeekIDR = TRUE;
					nalu->buf = IMGPAR ori_nalu_buf;					
					IMGPAR slice_number = 0;

				}

				free_new_slice(newSlice);
				newSlice = NULL;

				break;
			}

			IMGPAR currentSlice->framerate1000 = g_framerate1000;

			if (IMGPAR prevSlice)
				IMGPAR prevSlice->exit_flag = 1;

			*header = SOP;
			//Picture or Field
			if(IMGPAR currentSlice->field_pic_flag)
			{   //Field Picture

				if( (IMGPAR prevSlice!= NULL && IMGPAR prevSlice->field_pic_flag)
					&&					
					( (IMGPAR currentSlice->structure==BOTTOM_FIELD && IMGPAR prevSlice!=NULL && 
					IMGPAR prevSlice->frame_num == IMGPAR currentSlice->frame_num && IMGPAR prevSlice->m_pic_combine_status==TOP_FIELD)
					||
					(IMGPAR currentSlice->structure==TOP_FIELD && IMGPAR prevSlice!=NULL && 
					IMGPAR prevSlice->frame_num == IMGPAR currentSlice->frame_num && IMGPAR prevSlice->m_pic_combine_status==BOTTOM_FIELD) )
					)
				{
					//Second Filed of New Picture
					UpdatePTS ARGS0();

					sps =  (IMGPAR currentSlice->bIsBaseView) ? &SeqParSet[PicParSet[IMGPAR currentSlice->pic_parameter_set_id].seq_parameter_set_id] :
						&SeqParSubset[PicParSet[IMGPAR currentSlice->pic_parameter_set_id].seq_parameter_set_id];
					pps =  &PicParSet[IMGPAR currentSlice->pic_parameter_set_id];
					ret = activate_sps ARGS2(sps, IMGPAR currentSlice->viewId);
					if (FAILED(ret)) {
						free_new_slice(newSlice);
						newSlice = NULL;
						stream_global->m_bSeekIDR = TRUE;
						nalu->buf = IMGPAR ori_nalu_buf;					
						IMGPAR slice_number = 0;
						break;
					}
					activate_pps ARGS2(pps, IMGPAR currentSlice->viewIndex);

					if(!bSkip)
					{
						if (cMetaDataSize)
							IMGPAR dwSliceCountForSECOP--;

						if(stream_global->bMVC != TRUE)
						{
							ret = BA_ExecuteBuffers ARGS0();
							if (FAILED(ret)) {
								free_new_slice(newSlice);
								newSlice = NULL;
								stream_global->m_bSeekIDR = TRUE;
								nalu->buf = IMGPAR ori_nalu_buf;					
								IMGPAR slice_number = 0;
								break;
							}

							dec_picture->pic_store_idx = IMGPAR UnCompress_Idx;
							ret = store_picture_in_dpb ARGS1(dec_picture);
							if (FAILED(ret)) {
								free_new_slice(newSlice);
								newSlice = NULL;
								stream_global->m_bSeekIDR = TRUE;
								nalu->buf = IMGPAR ori_nalu_buf;					
								IMGPAR slice_number = 0;
								break;
							}
						}
						else
						{
							//MVC field coding
							if(m_iMVCFieldNeedExecuteIndex >= 0)
							{
								int current_view_index = (IMGPAR firstSlice)? IMGPAR firstSlice->viewIndex: 0;
								img = stream_global->m_img[m_iMVCFieldNeedExecuteIndex];

								ret = BA_ExecuteBuffers ARGS0();
								if (FAILED(ret)) {
									free_new_slice(newSlice);
									newSlice = NULL;
									stream_global->m_bSeekIDR = TRUE;
									nalu->buf = IMGPAR ori_nalu_buf;					
									IMGPAR slice_number = 0;
									break;
								}

								dec_picture->pic_store_idx = IMGPAR UnCompress_Idx;
								ret = store_picture_in_dpb ARGS1(dec_picture);
								if (FAILED(ret)) {
									free_new_slice(newSlice);
									newSlice = NULL;
									stream_global->m_bSeekIDR = TRUE;
									nalu->buf = IMGPAR ori_nalu_buf;					
									IMGPAR slice_number = 0;
									break;
								}

								img = stream_global->m_img[current_view_index];
								m_iMVCFieldNeedExecuteIndex = -1;
							}
						}

						if (cMetaDataSize)
						{
							IMGPAR sSliceInfoArray[0].dwSliceByteCount = IMGPAR sSliceInfoArray[IMGPAR dwSliceCountForSECOP].dwSliceByteCount;
							IMGPAR sSliceInfoArray[0].dwSliceHandle = IMGPAR sSliceInfoArray[IMGPAR dwSliceCountForSECOP].dwSliceHandle;
							memset(&(IMGPAR sSliceInfoArray[1]), 0, 4095*sizeof(SECOP_SliceInfo));
							IMGPAR dwSliceCountForSECOP = 1;
						}
					}
					//store these parameters to next collect_pic

					//For MVC field coding, different view index would be collected in different IMAGE array. 
					//So the following temp variables will be saved in different view index.
					//For non-MVC field coding, these will be saved in the index 0.
					int iMVCFieldViewindex = 0;
					if(stream_global->bMVC && IMGPAR firstSlice)
						iMVCFieldViewindex = IMGPAR firstSlice->viewIndex;
					stream_global->PreviousFrameNum[iMVCFieldViewindex]				= IMGPAR PreviousFrameNum;
					stream_global->PreviousFrameNumOffset[iMVCFieldViewindex]		= IMGPAR PreviousFrameNumOffset;
					stream_global->PreviousPOC[iMVCFieldViewindex]					= IMGPAR PreviousPOC;
					stream_global->ThisPOC[iMVCFieldViewindex]						= IMGPAR ThisPOC; 
					stream_global->PrevPicOrderCntLsb[iMVCFieldViewindex]			= IMGPAR PrevPicOrderCntLsb;
					stream_global->PrevPicOrderCntMsb[iMVCFieldViewindex]			= IMGPAR PrevPicOrderCntMsb;
					//Combine Two Filed
					IMGPAR currentSlice->header = SOP;
					IMGPAR currentSlice->m_pic_combine_status = 0; //Picture is Full
					IMGPAR slice_number++;
					nalu->buf += nalu->len;

					if (!bSkip)
					{
						ret = initial_image ARGS0();
						if (FAILED(ret)) {
							free_new_slice(newSlice);
							newSlice = NULL;
							stream_global->m_bSeekIDR = TRUE;
							nalu->buf = IMGPAR ori_nalu_buf;					
							IMGPAR slice_number = 0;
							break;
						}

						memcpy(&(dec_picture->m_CCCode), &(IMGPAR m_CCCode), sizeof(H264_CC));

						ret = init_lists ARGS2(IMGPAR type, IMGPAR currentSlice->structure);
						if (FAILED(ret)) {
							free_new_slice(newSlice);
							newSlice = NULL;
							stream_global->m_bSeekIDR = TRUE;
							nalu->buf = IMGPAR ori_nalu_buf;					
							IMGPAR slice_number = 0;
							break;
						}
						ret = reorder_lists ARGS2((IMGPAR type), (IMGPAR currentSlice));
						if (FAILED(ret)) {
							free_new_slice(newSlice);
							newSlice = NULL;
							stream_global->m_bSeekIDR = TRUE;
							nalu->buf = IMGPAR ori_nalu_buf;					
							IMGPAR slice_number = 0;
							break;
						}

						ret = check_lists ARGS0();
						if (FAILED(ret)) {
							free_new_slice(newSlice);
							newSlice = NULL;
							stream_global->m_bSeekIDR = TRUE;
							nalu->buf = IMGPAR ori_nalu_buf;					
							IMGPAR slice_number = 0;
							break;
						}

						g_bReceivedFirst = (!g_bReceivedFirst) ? 1 : 2;

						if (IMGPAR currentSlice->structure == FRAME)
							init_mbaff_lists ARGS0();
						free_ref_pic_list_reordering_buffer(IMGPAR currentSlice);

						IMGPAR m_Intra_lCompBufIndex = 0;
						IMGPAR m_lmbCount_Intra = 0;
						IMGPAR m_iIntraMCBufUsage = 0;
						IMGPAR m_iInterMCBufUsage = 0;
						IMGPAR m_slice_number_in_field = 1;

						if (cMetaDataSize)
							BA_build_picture_decode_buffer_SECOP ARGS0();
						else
							BA_build_picture_decode_buffer ARGS0();
						BA_DATA_CTRL = (DXVA_Slice_H264_Short *)m_lpSliceCtrlBuf;

						m_iMVCFieldNeedExecuteIndex = (IMGPAR firstSlice)? IMGPAR firstSlice->viewIndex: 0;
					}
				}
				else if (IMGPAR slice_number == 0)				
				{
					//First Filed of New Picture
					UpdatePTS ARGS0();

					ZeroMemory(&(IMGPAR m_CCCode), sizeof(H264_CC));
					SetH264CCCode ARGS0();

					sps =  (IMGPAR currentSlice->bIsBaseView) ? &SeqParSet[PicParSet[IMGPAR currentSlice->pic_parameter_set_id].seq_parameter_set_id] :
						&SeqParSubset[PicParSet[IMGPAR currentSlice->pic_parameter_set_id].seq_parameter_set_id];
					pps =  &PicParSet[IMGPAR currentSlice->pic_parameter_set_id];
					ret = activate_sps ARGS2(sps, IMGPAR currentSlice->viewId);
					if (FAILED(ret)) {
						free_new_slice(newSlice);
						newSlice = NULL;
						stream_global->m_bSeekIDR = TRUE;
						nalu->buf = IMGPAR ori_nalu_buf;					
						IMGPAR slice_number = 0;
						break;
					}
					activate_pps ARGS2(pps, IMGPAR currentSlice->viewIndex);

					IMGPAR currentSlice->header = SOP;
					IMGPAR currentSlice->m_pic_combine_status = IMGPAR currentSlice->structure; //first field
					IMGPAR slice_number++;
					nalu->buf += nalu->len;

					IMGPAR currentSlice->m_nDispPicStructure = sps->vui_seq_parameters.picture_structure;

					DEBUG_SHOW_HW_INFO("##bSkip: %d, Type = %d, POC = %d##", bSkip, IMGPAR firstSlice->AU_type, IMGPAR ThisPOC);

					IMGPAR UnCompress_Idx = -1;
					IMGPAR m_Intra_lCompBufIndex = 0;
					IMGPAR m_lmbCount_Intra = 0;
					IMGPAR m_iIntraMCBufUsage = 0;
					IMGPAR m_iInterMCBufUsage = 0;
					IMGPAR m_slice_number_in_field = 1;

					if(!bSkip)
					{
						//MVC field coding
						if(stream_global->bMVC == TRUE && m_iMVCFieldNeedExecuteIndex >= 0)
						{
							int current_view_index = (IMGPAR firstSlice)? IMGPAR firstSlice->viewIndex: 0;
							img = stream_global->m_img[m_iMVCFieldNeedExecuteIndex];

							ret = BA_ExecuteBuffers ARGS0();
							if (FAILED(ret)) {
								free_new_slice(newSlice);
								newSlice = NULL;
								stream_global->m_bSeekIDR = TRUE;
								nalu->buf = IMGPAR ori_nalu_buf;					
								IMGPAR slice_number = 0;
								break;
							}

							dec_picture->pic_store_idx = IMGPAR UnCompress_Idx;
							ret = store_picture_in_dpb ARGS1(dec_picture);
							if (FAILED(ret)) {
								free_new_slice(newSlice);
								newSlice = NULL;
								stream_global->m_bSeekIDR = TRUE;
								nalu->buf = IMGPAR ori_nalu_buf;					
								IMGPAR slice_number = 0;
								break;
							}

							img = stream_global->m_img[current_view_index];
							m_iMVCFieldNeedExecuteIndex = -1;
						}

						if ((IMGPAR firstSlice->picture_type==B_SLICE)  && dpb.used_size_on_view[0])
						{
							StorablePicture *pPreFrame = NULL;


							if ( dpb.fs_on_view[0][dpb.used_size_on_view[0]-1]->frame ) {	// Field Combined or Frame Split work may not be done if that picture has error.
								pPreFrame = dpb.fs_on_view[0][dpb.used_size_on_view[0]-1]->frame;
							} else if (dpb.fs_on_view[0][dpb.used_size_on_view[0]-1]->top_field) {
								pPreFrame = dpb.fs_on_view[0][dpb.used_size_on_view[0]-1]->top_field;
							} else {
								pPreFrame = dpb.fs_on_view[0][dpb.used_size_on_view[0]-1]->bottom_field;
							}

							if(pPreFrame && (pPreFrame->slice_type==P_SLICE || pPreFrame->slice_type==I_SLICE))
							{
								g_pArrangedPCCCode = &(pPreFrame->m_CCCode);
								g_pArrangedPCCCode->ccbuf[LINE21BUF_SIZE-1] = 0;
							}

							if (g_pArrangedPCCCode)
								RearrangePBCCBuf ARGS0();
						}

						ret = initial_image ARGS0();
						if (FAILED(ret)) {
							free_new_slice(newSlice);
							newSlice = NULL;
							stream_global->m_bSeekIDR = TRUE;
							nalu->buf = IMGPAR ori_nalu_buf;					
							IMGPAR slice_number = 0;
							break;
						}

						memcpy(&(dec_picture->m_CCCode), &(IMGPAR m_CCCode), sizeof(H264_CC));

						ret = init_lists ARGS2(IMGPAR type, IMGPAR currentSlice->structure);
						if (FAILED(ret)) {
							free_new_slice(newSlice);
							newSlice = NULL;
							stream_global->m_bSeekIDR = TRUE;
							nalu->buf = IMGPAR ori_nalu_buf;					
							IMGPAR slice_number = 0;
							break;
						}
						ret = reorder_lists ARGS2((IMGPAR type), (IMGPAR currentSlice));
						if (FAILED(ret)) {
							free_new_slice(newSlice);
							newSlice = NULL;
							stream_global->m_bSeekIDR = TRUE;
							nalu->buf = IMGPAR ori_nalu_buf;					
							IMGPAR slice_number = 0;
							break;
						}

						ret = check_lists ARGS0();
						if (FAILED(ret)) {
							free_new_slice(newSlice);
							newSlice = NULL;
							stream_global->m_bSeekIDR = TRUE;
							nalu->buf = IMGPAR ori_nalu_buf;					
							IMGPAR slice_number = 0;
							break;
						}

						if (IMGPAR currentSlice->structure == FRAME)
							init_mbaff_lists ARGS0();
						free_ref_pic_list_reordering_buffer(IMGPAR currentSlice);

						if (cMetaDataSize)
							BA_build_picture_decode_buffer_SECOP ARGS0();
						else
							BA_build_picture_decode_buffer ARGS0();
						BA_DATA_CTRL = (DXVA_Slice_H264_Short *)m_lpSliceCtrlBuf;

						m_iMVCFieldNeedExecuteIndex = (IMGPAR firstSlice)? IMGPAR firstSlice->viewIndex: 0;
					}
				}
				else
				{
					if (cMetaDataSize)
					{
						nalu->nal_unit_type = NALU_TYPE_SECOP;
						IMGPAR dwSliceCountForSECOP--;
					}

					//Cpoy naul to nalu_global
					nalu_global->AU_HasSPS	= AU_HasSPS;
					nalu_global->startcodeprefix_len	= nalu->startcodeprefix_len;
					nalu_global->len					= nalu->len;
					nalu_global->max_size				= nalu->max_size;
					nalu_global->nal_unit_type			= nalu->nal_unit_type;
					nalu_global->nal_reference_idc		= nalu->nal_reference_idc;
					nalu_global->forbidden_bit			= nalu->forbidden_bit;
					nalu_global->pts					= nalu->pts;
					memcpy(nalu_global->buf, nalu->buf, nalu->len);
					nalu_global_available = 1;

					//For MVC field coding, different view index would be collected in different IMAGE array. 
					//So the following temp variables will be saved in different view index.
					//For non-MVC field coding, these will be saved in the index 0.
					int iMVCFieldViewindex = 0;
					if(stream_global->bMVC && IMGPAR firstSlice)
						iMVCFieldViewindex = IMGPAR firstSlice->viewIndex;
					IMGPAR PreviousFrameNum				= stream_global->PreviousFrameNum[iMVCFieldViewindex];
					IMGPAR PreviousFrameNumOffset		= stream_global->PreviousFrameNumOffset[iMVCFieldViewindex];
					IMGPAR PreviousPOC					= stream_global->PreviousPOC[iMVCFieldViewindex];
					IMGPAR ThisPOC						= stream_global->ThisPOC[iMVCFieldViewindex]; 
					IMGPAR PrevPicOrderCntLsb			= stream_global->PrevPicOrderCntLsb[iMVCFieldViewindex];
					IMGPAR PrevPicOrderCntMsb			= stream_global->PrevPicOrderCntMsb[iMVCFieldViewindex];
					stream_global->m_iMVCFieldPicActiveIndex = iMVCFieldViewindex;

					if(stream_global->bMVC)
					{
						if(iMVCFieldViewindex == stream_global->num_views - 1)
							next_image_type = stream_global->m_img[0]->type;
						else
							next_image_type = stream_global->m_img[iMVCFieldViewindex+1]->type;
					}
					else
						next_image_type = IMGPAR type;

					IMGPAR currentSlice = IMGPAR prevSlice;
					IMGPAR currentSlice->nextSlice = NULL;

					if(!bSkip)
					{
						if(stream_global->bMVC != TRUE)
						{
							dec_picture->pic_store_idx = IMGPAR UnCompress_Idx;
							ret = store_picture_in_dpb ARGS1(dec_picture);
							if (FAILED(ret)) {
								free_new_slice(newSlice);
								newSlice = NULL;
								stream_global->m_bSeekIDR = TRUE;
								nalu->buf = IMGPAR ori_nalu_buf;					
								IMGPAR slice_number = 0;
								break;
							}
						}
						*header = SOP;
					}

					free_new_slice(newSlice);
					newSlice = NULL;

					if(stream_global->m_bTrickEOS)
					{
						DEBUG_SHOW_HW_INFO("-- TrickMode EOS after I_SLICE--");
						next_image_type = IMGPAR firstSlice->picture_type;
						nalu_global_available = 0;
						IMGPAR currentSlice->exit_flag = 1;
						*header = EOS;
					}

					return CREL_OK;
				}
			}
			else 
			{   //Frame Picture
				if(IMGPAR slice_number)
				{					
					if (cMetaDataSize)
					{
						nalu->nal_unit_type = NALU_TYPE_SECOP;
						IMGPAR dwSliceCountForSECOP--;
					}

					//Cpoy naul to nalu_global
					nalu_global->AU_HasSPS	= AU_HasSPS;
					nalu_global->startcodeprefix_len	= nalu->startcodeprefix_len;
					nalu_global->len					= nalu->len;
					nalu_global->max_size				= nalu->max_size;
					nalu_global->nal_unit_type			= nalu->nal_unit_type;
					nalu_global->nal_reference_idc		= nalu->nal_reference_idc;
					nalu_global->forbidden_bit			= nalu->forbidden_bit;
					nalu_global->pts					= nalu->pts;
					memcpy(nalu_global->buf, nalu->buf, nalu->len);
					nalu_global_available = 1;

					stream_global->m_iNextPOC = IMGPAR ThisPOC;
					stream_global->m_iNextViewIndex = IMGPAR currentSlice->viewIndex;

					IMGPAR PreviousFrameNum				= stream_global->PreviousFrameNum[0];
					IMGPAR PreviousFrameNumOffset = stream_global->PreviousFrameNumOffset[0];
					IMGPAR PreviousPOC						= stream_global->PreviousPOC[0];
					IMGPAR ThisPOC								= stream_global->ThisPOC[0]; 
					IMGPAR PrevPicOrderCntLsb			= stream_global->PrevPicOrderCntLsb[0];
					IMGPAR PrevPicOrderCntMsb			= stream_global->PrevPicOrderCntMsb[0];

					next_image_type = IMGPAR type;

					IMGPAR currentSlice = IMGPAR prevSlice;
					IMGPAR currentSlice->nextSlice = NULL;

					if(!bSkip)
					{
						dec_picture->pic_store_idx = IMGPAR UnCompress_Idx;
						ret = store_picture_in_dpb ARGS1(dec_picture);
						if (FAILED(ret)) {
							stream_global->m_bSeekIDR = TRUE;
							nalu->buf = IMGPAR ori_nalu_buf;					
							IMGPAR slice_number = 0;
							break;
						}
						*header = SOP;
					}

					free_new_slice(newSlice);
					newSlice = NULL;

					if(stream_global->m_bTrickEOS)
					{
						DEBUG_SHOW_HW_INFO("-- TrickMode EOS after I_SLICE--");
						next_image_type = IMGPAR firstSlice->picture_type;
						nalu_global_available = 0;
						IMGPAR currentSlice->exit_flag = 1;
						*header = EOS;							
					}

					return CREL_OK;
				}
				else
				{
					UpdatePTS ARGS0();

					ZeroMemory(&(IMGPAR m_CCCode), sizeof(H264_CC));
					SetH264CCCode ARGS0();

					sps =  (IMGPAR currentSlice->bIsBaseView) ? &SeqParSet[PicParSet[IMGPAR currentSlice->pic_parameter_set_id].seq_parameter_set_id] :
						&SeqParSubset[PicParSet[IMGPAR currentSlice->pic_parameter_set_id].seq_parameter_set_id];
					pps =  &PicParSet[IMGPAR currentSlice->pic_parameter_set_id];
					ret = activate_sps ARGS2(sps, IMGPAR currentSlice->viewId);
					if (FAILED(ret)) {
						break;
					}
					activate_pps ARGS2(pps, IMGPAR currentSlice->viewIndex);
					
					IMGPAR currentSlice->header = SOP;
					IMGPAR currentSlice->m_pic_combine_status = FRAME;
					IMGPAR slice_number++;
					nalu->buf += nalu->len;

					stream_global->PreviousFrameNum[0]				= IMGPAR PreviousFrameNum;
					stream_global->PreviousFrameNumOffset[0]	= IMGPAR PreviousFrameNumOffset;
					stream_global->PreviousPOC[0]						= IMGPAR PreviousPOC;
					stream_global->ThisPOC[0]								= IMGPAR ThisPOC; 
					stream_global->PrevPicOrderCntLsb[0]			= IMGPAR PrevPicOrderCntLsb;
					stream_global->PrevPicOrderCntMsb[0]			= IMGPAR PrevPicOrderCntMsb;

					IMGPAR currentSlice->m_nDispPicStructure = sps->vui_seq_parameters.picture_structure;

					DEBUG_SHOW_HW_INFO("##bSkip: %d, Type = %d, POC = %d##", bSkip, IMGPAR firstSlice->AU_type, IMGPAR ThisPOC);

					IMGPAR UnCompress_Idx = -1;
					IMGPAR m_Intra_lCompBufIndex = 0;
					IMGPAR m_lmbCount_Intra = 0;
					IMGPAR m_iIntraMCBufUsage = 0;
					IMGPAR m_iInterMCBufUsage = 0;
					IMGPAR m_slice_number_in_field = 1;

					if(!bSkip)
					{
						if ((IMGPAR firstSlice->picture_type==B_SLICE)  && dpb.used_size_on_view[0])
						{
							StorablePicture *pPreFrame = NULL;


							if ( dpb.fs_on_view[0][dpb.used_size_on_view[0]-1]->frame ) {	// Field Combined or Frame Split work may not be done if that picture has error.
								pPreFrame = dpb.fs_on_view[0][dpb.used_size_on_view[0]-1]->frame;
							} else if (dpb.fs_on_view[0][dpb.used_size_on_view[0]-1]->top_field) {
								pPreFrame = dpb.fs_on_view[0][dpb.used_size_on_view[0]-1]->top_field;
							} else {
								pPreFrame = dpb.fs_on_view[0][dpb.used_size_on_view[0]-1]->bottom_field;
							}

							if(pPreFrame && (pPreFrame->slice_type==P_SLICE || pPreFrame->slice_type==I_SLICE))
							{
								g_pArrangedPCCCode = &(pPreFrame->m_CCCode);
								g_pArrangedPCCCode->ccbuf[LINE21BUF_SIZE-1] = 0;
							}

							if (g_pArrangedPCCCode)
								RearrangePBCCBuf ARGS0();
						}

						ret = initial_image ARGS0();
						if (FAILED(ret)) {
							free_new_slice(newSlice);
							newSlice = NULL;
							stream_global->m_bSeekIDR = TRUE;
							nalu->buf = IMGPAR ori_nalu_buf;					
							IMGPAR slice_number = 0;
							break;
						}

						memcpy(&(dec_picture->m_CCCode), &(IMGPAR m_CCCode), sizeof(H264_CC));

						ret = init_lists ARGS2(IMGPAR type, IMGPAR currentSlice->structure);
						if (FAILED(ret)) {
							free_new_slice(newSlice);
							newSlice = NULL;
							stream_global->m_bSeekIDR = TRUE;
							nalu->buf = IMGPAR ori_nalu_buf;					
							IMGPAR slice_number = 0;
							break;
						}
						ret = reorder_lists ARGS2((IMGPAR type), (IMGPAR currentSlice));
						if (FAILED(ret)) {
							free_new_slice(newSlice);
							newSlice = NULL;
							stream_global->m_bSeekIDR = TRUE;
							nalu->buf = IMGPAR ori_nalu_buf;					
							IMGPAR slice_number = 0;
							break;
						}

						ret = check_lists ARGS0();
						if (FAILED(ret)) {
							free_new_slice(newSlice);
							newSlice = NULL;
							stream_global->m_bSeekIDR = TRUE;
							nalu->buf = IMGPAR ori_nalu_buf;					
							IMGPAR slice_number = 0;
							break;
						}

						if (IMGPAR currentSlice->structure == FRAME)
							init_mbaff_lists ARGS0();
						free_ref_pic_list_reordering_buffer(IMGPAR currentSlice);

						if (cMetaDataSize)
							BA_build_picture_decode_buffer_SECOP ARGS0();
						else
							BA_build_picture_decode_buffer ARGS0();
						BA_DATA_CTRL = (DXVA_Slice_H264_Short *)m_lpSliceCtrlBuf;

						g_bReceivedFirst = (!g_bReceivedFirst) ? 1 : 2;
					}
				}
			}												
		}
		else
		{
			IMGPAR currentSlice->header = *header = SOS;				
			IMGPAR currentSlice->m_pic_combine_status = IMGPAR prevSlice->m_pic_combine_status;
			IMGPAR slice_number++;
			IMGPAR m_slice_number_in_field++;

			if (IMGPAR prevSlice)
			{
				IMGPAR prevSlice->exit_flag = 0;
				IMGPAR currentSlice->pts = IMGPAR prevSlice->pts;
				IMGPAR currentSlice->dts = IMGPAR prevSlice->dts;
				IMGPAR currentSlice->has_pts = IMGPAR prevSlice->has_pts;
				IMGPAR currentSlice->framerate1000 = IMGPAR prevSlice->framerate1000;
				IMGPAR currentSlice->NumClockTs = IMGPAR prevSlice->NumClockTs;
			}
			nalu->buf += nalu->len;

			IMGPAR currentSlice->m_nDispPicStructure = IMGPAR prevSlice->m_nDispPicStructure;

			BA_DATA_CTRL = (DXVA_Slice_H264_Short *)m_lpSliceCtrlBuf;
		}

		if(!bSkip && !cMetaDataSize)
		{
			nalu_size = nalu->len+3 + m_nNALUSkipByte;
			BA_DATA_CTRL->BSNALunitDataLocation = (int)(m_lpBitstreamBuf - IMGPAR m_lpMV);
			shiftoffset = (nalu_size & 127);
			if(shiftoffset)
			{
				memset((m_lpnalu+nalu_size), 0,(128-shiftoffset));
				nalu_size += (128-shiftoffset);
			}
			memcpy(m_lpBitstreamBuf, m_lpnalu, nalu_size);
			m_lpBitstreamBuf += nalu_size;
			BA_DATA_CTRL->SliceBytesInBuffer = nalu_size;
			BA_DATA_CTRL->wBadSliceChopping = 0;
			m_lpSliceCtrlBuf += sizeof(DXVA_Slice_H264_Short);
			BA_DATA_CTRL = (DXVA_Slice_H264_Short *)m_lpSliceCtrlBuf;
			IMGPAR m_iInterMCBufUsage += sizeof(DXVA_Slice_H264_Short);
			IMGPAR m_iIntraMCBufUsage += nalu_size;
			IMGPAR m_lmbCount_Intra++;

			DEBUG_SHOW_HW_INFO("IMGPAR m_iInterMCBufUsage:%d, nalu_size:%d, nalu_len:%d, nalu_skipbyte:%d", IMGPAR m_iInterMCBufUsage, nalu_size, nalu->len, m_nNALUSkipByte);
		}

		break;
	case NALU_TYPE_DPA:
		return CREL_ERROR_H264_SYNCNOTFOUND;
		break;
	case NALU_TYPE_DPC:			
		return CREL_ERROR_H264_SYNCNOTFOUND;
		break;
	case NALU_TYPE_SEI:
		DEBUG_SHOW_HW_INFO("-- NALU_TYPE_SEI --");
		DEBUG_SHOW_HW_INFO ("read_new_slice: Found NALU_TYPE_SEI, len %d\n", nalu->len);
		ret = InterpretSEIMessage ARGS2(nalu->buf,nalu->len);
		if (FAILED(ret)) {

			if (ISWARNING(ret)) {

				if (ISWARNINGLEVEL_1(ret)) {
					stream_global->m_bSeekIDR = TRUE;
					nalu->buf = IMGPAR ori_nalu_buf;
					for ( unsigned int i = 0; i < stream_global->num_views; i++) {
						stream_global->m_active_sps_on_view[i]= 0;
					}
				}

				break;

			} else {
				return ret;
			}
		}			
		break;
	case NALU_TYPE_PPS:
		DEBUG_SHOW_HW_INFO("-- NALU_TYPE_PPS --");
		//if (m_newpps && ((IMGPAR field_pic_flag && new_picture==2) || (!IMGPAR field_pic_flag && new_picture)))
		//{
		//	nalu_global->AU_HasSPS	= AU_HasSPS;
		//	nalu_global->startcodeprefix_len	= nalu->startcodeprefix_len;
		//	nalu_global->len					= nalu->len;
		//	nalu_global->max_size				= nalu->max_size;
		//	nalu_global->nal_unit_type			= nalu->nal_unit_type;
		//	nalu_global->nal_reference_idc		= nalu->nal_reference_idc;
		//	nalu_global->forbidden_bit			= nalu->forbidden_bit;
		//	nalu_global->pts					= nalu->pts;
		//	memcpy(nalu_global->buf, nalu->buf, nalu->len);
		//	nalu_global_available = 1;

		//	if(!bSkip)
		//	{
		//		if (dec_picture)
		//		{
		//			dec_picture->pic_store_idx = IMGPAR UnCompress_Idx;
		//			ret = store_picture_in_dpb ARGS1(dec_picture);
		//			if (FAILED(ret)) {
		//				return ret;
		//			}
		//		}

		//		if (IMGPAR slice_number > 0)
		//			IMGPAR currentSlice->exit_flag = 1;

		//		if (stream_global->m_bTrickEOS)
		//		{
		//			DEBUG_SHOW_HW_INFO("-- TrickMode EOS after PPS--");
		//			next_image_type = IMGPAR firstSlice->picture_type;
		//			nalu_global_available = 0;
		//			*header = EOS;
		//			return CREL_OK;
		//		}
		//	}
		//	*header = SOP;
		//	return CREL_OK;
		//}
		m_newpps = true;
		ret = ProcessPPS ARGS1(nalu);		
		break;
	case NALU_TYPE_SPS:
		DEBUG_SHOW_HW_INFO("-- NALU_TYPE_SPS --");
		AU_HasSPS = 1;
		if ( CheckSPS ARGS2(nalu, 0) || (IMGPAR slice_number > 0 && IMGPAR firstSlice->idr_flag) || m_newsps)
		{
			nalu_global->AU_HasSPS	= AU_HasSPS;
			nalu_global->startcodeprefix_len	= nalu->startcodeprefix_len;
			nalu_global->len					= nalu->len;
			nalu_global->max_size				= nalu->max_size;
			nalu_global->nal_unit_type			= nalu->nal_unit_type;
			nalu_global->nal_reference_idc		= nalu->nal_reference_idc;
			nalu_global->forbidden_bit			= nalu->forbidden_bit;
			nalu_global->pts					= nalu->pts;
			memcpy(nalu_global->buf, nalu->buf, nalu->len);
			nalu_global_available = 1;

			if(!bSkip)
			{
				if (dec_picture)
				{
					dec_picture->pic_store_idx = IMGPAR UnCompress_Idx;
					ret = store_picture_in_dpb ARGS1(dec_picture);
					if (FAILED(ret)) {
						stream_global->m_bSeekIDR = TRUE;
						break;
					}
				}

				if (IMGPAR slice_number > 0)
					IMGPAR currentSlice->exit_flag = 1;

				if (stream_global->m_bTrickEOS)
				{
					DEBUG_SHOW_HW_INFO("-- TrickMode EOS after SPS--");
					next_image_type = IMGPAR firstSlice->picture_type;
					nalu_global_available = 0;
					*header = EOS;
					return CREL_OK;
				}
			}
			*header = (m_newsps ? SOP : NALU_TYPE_SPS);
			return CREL_OK;
		}

		m_newsps = true;
		ret = ProcessSPS ARGS1(nalu);
		break;
	case NALU_TYPE_PREFIX:

		ret = ProcessNaluExt ARGS1(nalu);
		stream_global->nalu_mvc_extension.bIsPrefixNALU = TRUE;

		if (FAILED(ret)) {

			if (ISWARNING(ret)) {

				if (ISWARNINGLEVEL_1(ret)) {
					stream_global->m_bSeekIDR = TRUE;
					nalu->buf = IMGPAR ori_nalu_buf;
				}

				break;

			} else {
				return ret;
			}
		}



		break;

	case NALU_TYPE_SPS_SUBSET:
		DEBUG_SHOW_SW_INFO("-- NALU_TYPE_SPS_SUBSET --");
/*
		//If bitstream is IDR only and IDR are all the same, decoder needs to return here.
		if (IMGPAR slice_number > 0 && IMGPAR firstSlice->idr_flag)
		{
			nalu_global->startcodeprefix_len = nalu->startcodeprefix_len;
			nalu_global->len                 = nalu->len;
			nalu_global->max_size            = nalu->max_size;
			nalu_global->nal_unit_type       = nalu->nal_unit_type;
			nalu_global->nal_reference_idc   = nalu->nal_reference_idc;
			nalu_global->forbidden_bit       = nalu->forbidden_bit;
			nalu_global->pts                 = nalu->pts;
			memcpy(nalu_global->buf, nalu->buf, nalu->len);
			nalu_global_available = 1;

			IMGPAR currentSlice->exit_flag = 1;

			return CREL_OK;
		}
*/
		ret = ProcessSPSSubset ARGS1(nalu);
		if (FAILED(ret)) {

			if (ISWARNING(ret)) {

				if (ISWARNINGLEVEL_1(ret)) {
					stream_global->m_bSeekIDR = TRUE;
					nalu->buf = IMGPAR ori_nalu_buf;
				}

				break;

			} else {
				return ret;
			}
		}

		break;
	case NALU_TYPE_AUD:
		AU_HasSPS = 0;
		ret = ProcessAUD ARGS2(nalu, &primary_pic_type);		
		break;
	case NALU_TYPE_EOSEQ:
		DEBUG_SHOW_HW_INFO ("read_new_slice: Found 'End of Sequence' NAL unit, len %d, ignored\n", nalu->len);
		break;
	case NALU_TYPE_EOSTREAM:
		DEBUG_SHOW_HW_INFO ("read_new_slice: Found 'End of Stream' NAL unit, len %d, ignored\n", nalu->len);
		break;
	case NALU_TYPE_FILL:
		DEBUG_SHOW_HW_INFO ("read_new_slice: Found NALU_TYPE_FILL, len %d\n", nalu->len);
		DEBUG_SHOW_HW_INFO ("Skipping these filling bits, proceeding w/ next NALU\n");
		break;
	case NALU_TYPE_DRD:
		break;
	default:
		DEBUG_SHOW_HW_INFO("-- NALU_TYPE_default --");
		DEBUG_SHOW_HW_INFO("Found NALU type %d, len %d undefined, ignore NALU, moving on\n", nalu->nal_unit_type, nalu->len);
	}

	if (FAILED(ret)) {

		if (g_has_pts)
		{
			g_has_pts = 0;
			DEBUG_INFO("[Drop PTS]: ts: %I64d freq: %d, tslength: %d flags: %d unused1: %d unused2: %d", nalu->pts.ts, nalu->pts.freq, nalu->pts.tslength, nalu->pts.flags, nalu->pts.unused1, nalu->pts.unused2);
		}

		if (IMGPAR structure == FRAME) {
			
			if(dec_picture)
			{
				if (dpb.used_size_on_view && ((dpb.used_size_on_view[0] == 0) ||dpb.fs_on_view[0][dpb.used_size_on_view[0] - 1]->frame != dec_picture)) {
					dec_picture->pic_store_idx = img->UnCompress_Idx;
					release_storable_picture ARGS2(dec_picture, 1);
					img->UnCompress_Idx = -1;
					dec_picture->pic_store_idx = -1;
				}
				dec_picture = NULL;
			}
			img->m_dec_picture_top = NULL;
			img->m_dec_picture_bottom = NULL;
			


		} else {

			if (img->m_dec_picture_top && (img->m_dec_picture_top->top_field_first)) {

				if(img->m_dec_picture_top && img->m_dec_picture_bottom)
				{
					if (dpb.used_size_on_view && ((dpb.used_size_on_view[0] == 0) ||dpb.fs_on_view[0][dpb.used_size_on_view[0] - 1]->bottom_field != img->m_dec_picture_bottom)) {

						if (dpb.used_size_on_view && (dpb.used_size_on_view[0] && dpb.fs_on_view[0][dpb.used_size_on_view[0] - 1]->top_field && (dpb.fs_on_view[0][dpb.used_size_on_view[0] - 1]->top_field->pic_store_idx == img->UnCompress_Idx))){
							img->m_dec_picture_bottom->pic_store_idx = -1;
						} else {
							img->m_dec_picture_bottom->pic_store_idx = img->UnCompress_Idx;
						}

						//img->m_dec_picture_bottom->pic_store_idx = img->UnCompress_Idx;

						release_storable_picture ARGS2(img->m_dec_picture_bottom, 1);
						img->UnCompress_Idx = -1;
						img->m_dec_picture_bottom->pic_store_idx = -1;
					}
					
				} else {

					if (dpb.used_size_on_view && ((dpb.used_size_on_view[0] == 0) ||dpb.fs_on_view[0][dpb.used_size_on_view[0] - 1]->top_field != img->m_dec_picture_top)) {
						if (img->m_dec_picture_top) {
							img->m_dec_picture_top->pic_store_idx = img->UnCompress_Idx;
						}
						release_storable_picture ARGS2(img->m_dec_picture_top, 1);	
						img->UnCompress_Idx = -1;
						
					}

				}
				img->m_dec_picture_top = NULL;
				img->m_dec_picture_bottom = NULL;


			} else {

				if(img->m_dec_picture_top && img->m_dec_picture_bottom)
				{
					if (dpb.used_size_on_view && ((dpb.used_size_on_view[0] == 0) ||dpb.fs_on_view[0][dpb.used_size_on_view[0] - 1]->top_field != img->m_dec_picture_top)) {

						if (dpb.used_size_on_view && (dpb.used_size_on_view[0] && dpb.fs_on_view[0][dpb.used_size_on_view[0] - 1]->top_field && (dpb.fs_on_view[0][dpb.used_size_on_view[0] - 1]->bottom_field->pic_store_idx == img->UnCompress_Idx))){
							img->m_dec_picture_top->pic_store_idx = -1;
						} else {
							img->m_dec_picture_top->pic_store_idx = img->UnCompress_Idx;
						}

						//img->m_dec_picture_top->pic_store_idx = img->UnCompress_Idx;

						release_storable_picture ARGS2(img->m_dec_picture_top, 1);
						img->UnCompress_Idx = -1;
						img->m_dec_picture_top->pic_store_idx = -1;
					}

				} else {

					if (dpb.used_size_on_view && ((dpb.used_size_on_view[0] == 0) ||dpb.fs_on_view[0][dpb.used_size_on_view[0] - 1]->bottom_field != img->m_dec_picture_bottom)) {
						if (img->m_dec_picture_bottom) {
							img->m_dec_picture_bottom->pic_store_idx = img->UnCompress_Idx;
						}
						release_storable_picture ARGS2(img->m_dec_picture_bottom, 1);
						img->UnCompress_Idx = -1;						
					}

				}
				img->m_dec_picture_top = NULL;
				img->m_dec_picture_bottom = NULL;


			}		


			dec_picture = NULL;			

		}	

		for (int i=0 ;i<1; i++)
		{
			img = img_array[i];
			if (i==0)
			{
				IMGPAR currentSlice = IMGPAR firstSlice;
				for (int j=0; j< IMGPAR slice_number; j++)
				{
					//active_pps = &PicParSet[IMGPAR currentSlice->pic_parameter_set_id];
					memcpy(&active_pps, &PicParSet[IMGPAR currentSlice->pic_parameter_set_id], sizeof(pic_parameter_set_rbsp_t));
					IMGPAR prevSlice = IMGPAR currentSlice;
					IMGPAR currentSlice = (Slice*) IMGPAR currentSlice->nextSlice;
					free_new_slice(IMGPAR prevSlice);
				}
			}
			IMGPAR firstSlice = IMGPAR currentSlice = IMGPAR prevSlice = NULL;
			IMGPAR current_mb_nr_r = IMGPAR current_mb_nr_d = -4712;   // impossible value for debugging, StW
			IMGPAR error_mb_nr = -4712;
			IMGPAR current_slice_nr = 0;
			IMGPAR slice_number = 0;
			img->m_active_pps[0].Valid = NULL;
			img->m_active_pps[1].Valid = NULL;
			dec_picture = NULL;
			img->m_dec_picture_top = NULL;
			img->m_dec_picture_bottom = NULL;

			if (IMGPAR structure != FRAME)
			{
				IMGPAR mb_decdata = IMGPAR mb_decdata_ori;
				IMGPAR cof_array = IMGPAR cof_array_ori;
			}
		}

		if (stream_global->m_bSeekIDR) {
			if(dpb.used_size_on_view[0])
			{
				for (unsigned int i=0; i <storable_picture_count; i++)				
					storable_picture_map[i]->used_for_first_field_reference = 0;

				
				if (img->structure  == FRAME) {

					if (dpb.fs_on_view[0][dpb.used_size_on_view[0]-1]->frame && dpb.fs_on_view[0][dpb.used_size_on_view[0]-1]->frame->pic_store_idx == img->UnCompress_Idx){
						img->UnCompress_Idx = -1;
					}

				} else {

					if ((dpb.fs_on_view[0][dpb.used_size_on_view[0]-1]->top_field && dpb.fs_on_view[0][dpb.used_size_on_view[0]-1]->top_field->pic_store_idx == img->UnCompress_Idx) ||
						(dpb.fs_on_view[0][dpb.used_size_on_view[0]-1]->bottom_field && dpb.fs_on_view[0][dpb.used_size_on_view[0]-1]->bottom_field->pic_store_idx == img->UnCompress_Idx)) {
							img->UnCompress_Idx = -1;
					}
				}				

				flush_dpb ARGS1(0);
				update_ref_list ARGS1(0);
				update_ltref_list ARGS1(0);		
			}

			nalu_global_available = 0;
		}

		return ret;
	}
	
	return CREL_ERROR_H264_SYNCNOTFOUND;
}

CREL_RETURN CH264DXVA2::decode_one_picture_long PARGS2(int* header, BOOL bSkip)
{
	DecodingEnvironment *dep;
	DXVA_Slice_H264_Long *BA_DATA_CTRL;
	int AU_HasSPS = 0;
	int primary_pic_type = -1;
	int new_pps = 0;
	CREL_RETURN ret = CREL_OK;
	int shiftoffset;
	int nalu_size;
	Slice *newSlice;
	static BOOL bSeekIDR_backup;

	BOOL m_newpps = false;
	BOOL m_newsps = false;
	BOOL bFirstSlice = (IMGPAR slice_number==0) ? TRUE:FALSE;

	if (bFirstSlice)
		nalu->buf = IMGPAR ori_nalu_buf;

	AU_HasSPS	= nalu_global->AU_HasSPS;
	nalu->startcodeprefix_len	= nalu_global->startcodeprefix_len;
	nalu->len					= nalu_global->len;
	nalu->max_size				= nalu_global->max_size;
	nalu->nal_unit_type			= nalu_global->nal_unit_type;
	nalu->nal_reference_idc		= nalu_global->nal_reference_idc;
	nalu->forbidden_bit			= nalu_global->forbidden_bit;
	nalu->pts					= nalu_global->pts;
	memcpy(nalu->buf, nalu_global->buf, nalu_global->len);

	nalu_global_available = 0;			
	if(nalu->nal_unit_type == NALU_TYPE_SPS)
	{
		m_newsps = false;
		m_newpps = false;
	}
	else if(nalu->nal_unit_type == NALU_TYPE_PPS)
		m_newpps = false;

	switch (nalu->nal_unit_type)
	{
	case NALU_TYPE_SLICE:
	case NALU_TYPE_IDR:
	case NALU_TYPE_SLICE_EXT:

		DEBUG_SHOW_HW_INFO("-- NALU_TYPE_SLICE -- NALU_TYPE_IDR --");

		ret = malloc_new_slice(&newSlice);
		if (FAILED(ret)) {
			break;
		}

		if(nalu->nal_unit_type == NALU_TYPE_IDR)
		{
			DEBUG_SHOW_HW_INFO("-- This is IDR --");
			IMGPAR idr_flag = newSlice->idr_flag = 1;
		}
		else
			IMGPAR idr_flag = newSlice->idr_flag = 0;

		IMGPAR nal_reference_idc = newSlice->nal_reference_idc = nalu->nal_reference_idc;
		IMGPAR disposable_flag = newSlice->disposable_flag = (nalu->nal_reference_idc == NALU_PRIORITY_DISPOSABLE);

		newSlice->dp_mode = PAR_DP_1;
		newSlice->max_part_nr = 1;
		newSlice->ei_flag = 0;
		newSlice->nextSlice = NULL;
		dep              = newSlice->g_dep;
		dep->Dei_flag    = 0;
		dep->Dbits_to_go = 0;
		dep->Dbuffer     = 0;	

		if (nalu->nal_unit_type == NALU_TYPE_SLICE_EXT) {

			ret = ProcessNaluExt ARGS1(nalu);

			dep->Dbasestrm = &(nalu->buf[4]);
			dep->Dstrmlength = nalu->len-4;
		} else 
		{
			dep->Dbasestrm = &(nalu->buf[1]);
			dep->Dstrmlength = nalu->len-1;
		}

		ret = RBSPtoSODB(dep->Dbasestrm, &(dep->Dstrmlength));
		if (FAILED(ret)) {
			break;
		}
		dep->Dcodestrm   = dep->Dbasestrm;

		if (nalu->nal_unit_type == NALU_TYPE_SLICE_EXT) {

			CopyNaluExtToSlice ARGS1 (newSlice);
			newSlice->bIsBaseView = false;
			stream_global->nalu_mvc_extension.bIsPrefixNALU = FALSE;
			//stream_global->nalu_mvc_extension.valid = FALSE;

		}
		else
		{
			newSlice->bIsBaseView = true;
			if(stream_global->m_active_sps_on_view[1] == 0)
			{
				newSlice->viewId = GetBaseViewId ARGS0 ();
				if(stream_global->nalu_mvc_extension.valid && stream_global->nalu_mvc_extension.bIsPrefixNALU && newSlice->viewId == stream_global->nalu_mvc_extension.viewId)
				{
					newSlice->interViewFlag = stream_global->nalu_mvc_extension.interViewFlag;
					newSlice->anchorPicFlag = stream_global->nalu_mvc_extension.anchorPicFlag;
				}
				else
				{
					newSlice->interViewFlag = 1;
					newSlice->anchorPicFlag = 0;
				}
			}
			else
			{
				if(stream_global->nalu_mvc_extension.valid && stream_global->nalu_mvc_extension.bIsPrefixNALU)
				{
					newSlice->viewId = stream_global->nalu_mvc_extension.viewId;
					newSlice->interViewFlag = stream_global->nalu_mvc_extension.interViewFlag;
					newSlice->anchorPicFlag = stream_global->nalu_mvc_extension.anchorPicFlag;
				}
				else
				{
					newSlice->viewId = stream_global->m_active_sps_on_view[1]->view_id[0];
					newSlice->interViewFlag = 1;
					newSlice->anchorPicFlag = 0;
				}
			}
		}
		newSlice->viewIndex = GetViewIndex ARGS1 (newSlice->viewId);

		stream_global->m_pbValidViews[newSlice->viewIndex] = 1;
		if(stream_global->m_CurrOutputViewIndex == -1)
			stream_global->m_CurrOutputViewIndex = newSlice->viewIndex;

		if(bFirstSlice)
		{				
			IMGPAR prevSlice  = NULL;
			IMGPAR firstSlice = newSlice;
			IMGPAR currentSlice  = IMGPAR firstSlice;
		}
		else
		{
			IMGPAR prevSlice  = IMGPAR currentSlice;
			IMGPAR currentSlice->nextSlice = (void*)newSlice;
			IMGPAR currentSlice = (Slice*) (IMGPAR currentSlice->nextSlice);
		}

		if (bFirstSlice) {
			bSeekIDR_backup = stream_global->m_bSeekIDR;
		}	

		ret = ParseSliceHeader ARGS0();
		if ( FAILED(ret) ) {

			if (bSeekIDR_backup) {
				free_new_slice(newSlice);
				newSlice = NULL;		

				stream_global->m_bSeekIDR = TRUE;

				nalu->buf = IMGPAR ori_nalu_buf;					
				IMGPAR slice_number = 0;
				break;

			}			

			free_new_slice(newSlice);
			newSlice = NULL;					

			if (stream_global->m_bSeekIDR) {
				nalu->buf = IMGPAR ori_nalu_buf;					
				IMGPAR slice_number = 0;

			} else if (nalu->nal_unit_type == NALU_TYPE_IDR) {
				stream_global->m_bSeekIDR = TRUE;
				nalu->buf = IMGPAR ori_nalu_buf;

			}				

			break;

		}		

		IMGPAR currentSlice->AU_type = IMGPAR currentSlice->picture_type;			
		if(AU_HasSPS && IMGPAR currentSlice->AU_type==I_SLICE)
			IMGPAR currentSlice->AU_type = I_GOP;			
		if(IMGPAR currentSlice->idr_flag)
			IMGPAR currentSlice->AU_type = IDR_SLICE;
		if(IMGPAR currentSlice->picture_type == B_SLICE && IMGPAR currentSlice->nal_reference_idc )
			IMGPAR currentSlice->AU_type = RB_SLICE;

		DEBUG_SHOW_HW_INFO("This Slice AU Type: %d", IMGPAR currentSlice->AU_type);

		if (stream_global->bSeekToOpenGOP)
		{
			if (IMGPAR currentSlice->picture_type != B_SLICE)
			{
				stream_global->bSeekToOpenGOP = 0;
				next_image_type = IMGPAR type;
				bSkip = 0; //reset skip flag
			}
			else/* if (IMGPAR currentSlice->AU_type != RB_SLICE)*/
			{
				nalu->buf = IMGPAR ori_nalu_buf;
				if (IMGPAR prevSlice)
				{
					IMGPAR currentSlice = IMGPAR prevSlice;
					IMGPAR currentSlice->nextSlice = NULL;
				}
				free_new_slice(newSlice);
				newSlice = NULL;

				if (g_has_pts)
				{
					g_has_pts = 0;
					DEBUG_INFO("[Drop PTS]: ts: %I64d freq: %d, tslength: %d flags: %d unused1: %d unused2: %d", nalu->pts.ts, nalu->pts.freq, nalu->pts.tslength, nalu->pts.flags, nalu->pts.unused1, nalu->pts.unused2);
				}
				break;
			}
		}

		if ((stream_global->profile_idc == 0) || 
			((g_bReceivedFirst == 0) && (IMGPAR firstSlice->picture_type != I_SLICE))
			)
		{
			nalu->buf = IMGPAR ori_nalu_buf;
			free_new_slice(newSlice);
			newSlice = NULL;
			IMGPAR slice_number = 0;
			m_newpps = FALSE;
			m_newsps = FALSE;

			for ( unsigned int i = 0; i < stream_global->num_views; i++) {
				stream_global->m_active_sps_on_view[i]= 0;
			}

			if (g_has_pts)
			{
				g_has_pts = 0;
				DEBUG_INFO("[Drop PTS]: ts: %I64d freq: %d, tslength: %d flags: %d unused1: %d unused2: %d", nalu->pts.ts, nalu->pts.freq, nalu->pts.tslength, nalu->pts.flags, nalu->pts.unused1, nalu->pts.unused2);
			}
			break;
		}

		if(is_new_picture ARGS0())
		{
			seq_parameter_set_rbsp_t *sps;
			pic_parameter_set_rbsp_t *pps;

			ret = decode_poc ARGS0();
			if (FAILED(ret)) {

				if (bSeekIDR_backup) {							

					stream_global->m_bSeekIDR = TRUE;
					nalu->buf = IMGPAR ori_nalu_buf;					
					IMGPAR slice_number = 0;

				}

				free_new_slice(newSlice);
				newSlice = NULL;

				break;
			}

			m_nSliceNum = 0;

			IMGPAR currentSlice->framerate1000 = g_framerate1000;

			if (IMGPAR prevSlice)
				IMGPAR prevSlice->exit_flag = 1;

			*header = SOP;
			//Picture or Field
			if(IMGPAR currentSlice->field_pic_flag)
			{   //Field Picture
				if( (IMGPAR prevSlice!= NULL && IMGPAR prevSlice->field_pic_flag)
					&&					
					( (IMGPAR currentSlice->structure==BOTTOM_FIELD && IMGPAR prevSlice!=NULL && 
					IMGPAR prevSlice->frame_num == IMGPAR currentSlice->frame_num && IMGPAR prevSlice->m_pic_combine_status==TOP_FIELD)
					||
					(IMGPAR currentSlice->structure==TOP_FIELD && IMGPAR prevSlice!=NULL && 
					IMGPAR prevSlice->frame_num == IMGPAR currentSlice->frame_num && IMGPAR prevSlice->m_pic_combine_status==BOTTOM_FIELD) )
					)
				{
					//Second Filed of New Picture
					UpdatePTS ARGS0();

					sps =  (IMGPAR currentSlice->bIsBaseView) ? &SeqParSet[PicParSet[IMGPAR currentSlice->pic_parameter_set_id].seq_parameter_set_id] :
						&SeqParSubset[PicParSet[IMGPAR currentSlice->pic_parameter_set_id].seq_parameter_set_id];
					pps =  &PicParSet[IMGPAR currentSlice->pic_parameter_set_id];
					ret = activate_sps ARGS2(sps, IMGPAR currentSlice->viewId);
					if (FAILED(ret)) {
						free_new_slice(newSlice);
						newSlice = NULL;
						stream_global->m_bSeekIDR = TRUE;
						nalu->buf = IMGPAR ori_nalu_buf;					
						IMGPAR slice_number = 0;
						break;
					}
					activate_pps ARGS2(pps, IMGPAR currentSlice->viewIndex);

					if(!bSkip)
					{
						ret = BA_ExecuteBuffers ARGS0();
						if (FAILED(ret)) {
							free_new_slice(newSlice);
							newSlice = NULL;
							stream_global->m_bSeekIDR = TRUE;
							nalu->buf = IMGPAR ori_nalu_buf;					
							IMGPAR slice_number = 0;
							break;
						}

						dec_picture->pic_store_idx = IMGPAR UnCompress_Idx;
						ret = store_picture_in_dpb ARGS1(dec_picture);
						if (FAILED(ret)) {
							free_new_slice(newSlice);
							newSlice = NULL;
							stream_global->m_bSeekIDR = TRUE;
							nalu->buf = IMGPAR ori_nalu_buf;					
							IMGPAR slice_number = 0;
							break;
						}
						EndDecodeFrame ARGS0();

						m_lpBitstreamBuf = m_lpBitstreamBuf_Intel;
					}
					//store these parameters to next collect_pic
					stream_global->PreviousFrameNum[0]				= IMGPAR PreviousFrameNum;
					stream_global->PreviousFrameNumOffset[0]	= IMGPAR PreviousFrameNumOffset;
					stream_global->PreviousPOC[0]						= IMGPAR PreviousPOC;
					stream_global->ThisPOC[0]								= IMGPAR ThisPOC; 
					stream_global->PrevPicOrderCntLsb[0]			= IMGPAR PrevPicOrderCntLsb;
					stream_global->PrevPicOrderCntMsb[0]			= IMGPAR PrevPicOrderCntMsb;
					//Combine Two Filed
					IMGPAR currentSlice->header = SOP;
					IMGPAR currentSlice->m_pic_combine_status = 0; //Picture is Full
					IMGPAR slice_number++;

					nalu->buf += nalu->len;
					if (!bSkip)
					{
						ret = initial_image ARGS0();
						if (FAILED(ret)) {
							free_new_slice(newSlice);
							newSlice = NULL;
							stream_global->m_bSeekIDR = TRUE;
							nalu->buf = IMGPAR ori_nalu_buf;					
							IMGPAR slice_number = 0;
							break;
						}

						memcpy(&(dec_picture->m_CCCode), &(IMGPAR m_CCCode), sizeof(H264_CC));

						ret = init_lists ARGS2(IMGPAR type, IMGPAR currentSlice->structure);
						if (FAILED(ret)) {
							free_new_slice(newSlice);
							newSlice = NULL;
							stream_global->m_bSeekIDR = TRUE;
							nalu->buf = IMGPAR ori_nalu_buf;					
							IMGPAR slice_number = 0;
							break;
						}
						ret = reorder_lists ARGS2((IMGPAR type), (IMGPAR currentSlice));
						if (FAILED(ret)) {
							free_new_slice(newSlice);
							newSlice = NULL;
							stream_global->m_bSeekIDR = TRUE;
							nalu->buf = IMGPAR ori_nalu_buf;					
							IMGPAR slice_number = 0;
							break;
						}

						ret = check_lists ARGS0();
						if (FAILED(ret)) {
							free_new_slice(newSlice);
							newSlice = NULL;
							stream_global->m_bSeekIDR = TRUE;
							nalu->buf = IMGPAR ori_nalu_buf;					
							IMGPAR slice_number = 0;
							break;
						}

						g_bReceivedFirst = (!g_bReceivedFirst) ? 1 : 2;

						if (IMGPAR currentSlice->structure == FRAME)
							init_mbaff_lists ARGS0();
						free_ref_pic_list_reordering_buffer(IMGPAR currentSlice);

						//For long format weight offset
						if ((active_pps.weighted_bipred_idc > 0 && IMGPAR type == B_SLICE) || (active_pps.weighted_pred_flag && IMGPAR type == P_SLICE))
							fill_wp_params ARGS0();

						IMGPAR m_Intra_lCompBufIndex = 0;
						IMGPAR m_lmbCount_Intra = 0;
						IMGPAR m_iIntraMCBufUsage = 0;
						IMGPAR m_iInterMCBufUsage = 0;
						IMGPAR m_slice_number_in_field = 1;

						BA_build_picture_decode_buffer ARGS0();
						BA_DATA_CTRL = (DXVA_Slice_H264_Long *)m_lpSliceCtrlBuf;
					}
				}
				else if (IMGPAR slice_number == 0)				
				{
					//First Filed of New Picture
					UpdatePTS ARGS0();

					ZeroMemory(&(IMGPAR m_CCCode), sizeof(H264_CC));
					SetH264CCCode ARGS0();

					sps =  (IMGPAR currentSlice->bIsBaseView) ? &SeqParSet[PicParSet[IMGPAR currentSlice->pic_parameter_set_id].seq_parameter_set_id] :
						&SeqParSubset[PicParSet[IMGPAR currentSlice->pic_parameter_set_id].seq_parameter_set_id];
					pps =  &PicParSet[IMGPAR currentSlice->pic_parameter_set_id];
					ret = activate_sps ARGS2(sps, IMGPAR currentSlice->viewId);
					if (FAILED(ret)) {
						free_new_slice(newSlice);
						newSlice = NULL;
						stream_global->m_bSeekIDR = TRUE;
						nalu->buf = IMGPAR ori_nalu_buf;					
						IMGPAR slice_number = 0;
						break;
					}
					activate_pps ARGS2(pps, IMGPAR currentSlice->viewIndex);

					IMGPAR currentSlice->header = SOP;
					IMGPAR currentSlice->m_pic_combine_status = IMGPAR currentSlice->structure; //first field
					IMGPAR slice_number++;
					nalu->buf += nalu->len;

					IMGPAR currentSlice->m_nDispPicStructure = sps->vui_seq_parameters.picture_structure;

					DEBUG_SHOW_HW_INFO("##bSkip: %d, Type = %d, POC = %d##", bSkip, IMGPAR firstSlice->AU_type, IMGPAR ThisPOC);

					IMGPAR UnCompress_Idx = -1;
					IMGPAR m_Intra_lCompBufIndex = 0;
					IMGPAR m_lmbCount_Intra = 0;
					IMGPAR m_iIntraMCBufUsage = 0;
					IMGPAR m_iInterMCBufUsage = 0;
					IMGPAR m_slice_number_in_field = 1;

					if(!bSkip)
					{
						unsigned int view_index = (IMGPAR currentSlice) ? (IMGPAR currentSlice->viewIndex) : 0;
						if ((IMGPAR firstSlice->picture_type==B_SLICE)  && dpb.used_size_on_view[view_index])
						{
							StorablePicture *pPreFrame = NULL;

							if ( dpb.fs_on_view[view_index][dpb.used_size_on_view[view_index]-1]->frame ) {	// Field Combined or Frame Split work may not be done if that picture has error.
								pPreFrame = dpb.fs_on_view[view_index][dpb.used_size_on_view[view_index]-1]->frame;
							} else if (dpb.fs_on_view[view_index][dpb.used_size_on_view[view_index]-1]->top_field) {
								pPreFrame = dpb.fs_on_view[view_index][dpb.used_size_on_view[view_index]-1]->top_field;
							} else {
								pPreFrame = dpb.fs_on_view[view_index][dpb.used_size_on_view[view_index]-1]->bottom_field;
							}

							if(pPreFrame && (pPreFrame->slice_type==P_SLICE || pPreFrame->slice_type==I_SLICE))
							{
								g_pArrangedPCCCode = &(pPreFrame->m_CCCode);
								g_pArrangedPCCCode->ccbuf[LINE21BUF_SIZE-1] = 0;
							}

							if (g_pArrangedPCCCode)
								RearrangePBCCBuf ARGS0();
						}

						ret = initial_image ARGS0();
						if (FAILED(ret)) {
							free_new_slice(newSlice);
							newSlice = NULL;
							stream_global->m_bSeekIDR = TRUE;
							nalu->buf = IMGPAR ori_nalu_buf;					
							IMGPAR slice_number = 0;
							break;
						}

						memcpy(&(dec_picture->m_CCCode), &(IMGPAR m_CCCode), sizeof(H264_CC));

						ret = init_lists ARGS2(IMGPAR type, IMGPAR currentSlice->structure);
						if (FAILED(ret)) {
							free_new_slice(newSlice);
							newSlice = NULL;
							stream_global->m_bSeekIDR = TRUE;
							nalu->buf = IMGPAR ori_nalu_buf;					
							IMGPAR slice_number = 0;
							break;
						}
						ret = reorder_lists ARGS2((IMGPAR type), (IMGPAR currentSlice));
						if (FAILED(ret)) {
							free_new_slice(newSlice);
							newSlice = NULL;
							stream_global->m_bSeekIDR = TRUE;
							nalu->buf = IMGPAR ori_nalu_buf;					
							IMGPAR slice_number = 0;
							break;
						}

						ret = check_lists ARGS0();
						if (FAILED(ret)) {
							free_new_slice(newSlice);
							newSlice = NULL;
							stream_global->m_bSeekIDR = TRUE;
							nalu->buf = IMGPAR ori_nalu_buf;					
							IMGPAR slice_number = 0;
							break;
						}

						if (IMGPAR currentSlice->structure == FRAME)
							init_mbaff_lists ARGS0();
						free_ref_pic_list_reordering_buffer(IMGPAR currentSlice);

						//For long format weight offset
						if ((active_pps.weighted_bipred_idc > 0 && IMGPAR type == B_SLICE) || (active_pps.weighted_pred_flag && IMGPAR type == P_SLICE))
							fill_wp_params ARGS0();

						BA_build_picture_decode_buffer ARGS0();
						BA_DATA_CTRL = (DXVA_Slice_H264_Long *)m_lpSliceCtrlBuf;
					}
				}
				else
				{
					//Cpoy naul to nalu_global
					nalu_global->AU_HasSPS	= AU_HasSPS;
					nalu_global->startcodeprefix_len	= nalu->startcodeprefix_len;
					nalu_global->len					= nalu->len;
					nalu_global->max_size				= nalu->max_size;
					nalu_global->nal_unit_type			= nalu->nal_unit_type;
					nalu_global->nal_reference_idc		= nalu->nal_reference_idc;
					nalu_global->forbidden_bit			= nalu->forbidden_bit;
					nalu_global->pts					= nalu->pts;
					memcpy(nalu_global->buf, nalu->buf, nalu->len);
					nalu_global_available = 1;

					IMGPAR PreviousFrameNum				= stream_global->PreviousFrameNum[0];
					IMGPAR PreviousFrameNumOffset = stream_global->PreviousFrameNumOffset[0];
					IMGPAR PreviousPOC						= stream_global->PreviousPOC[0];
					IMGPAR ThisPOC								= stream_global->ThisPOC[0]; 
					IMGPAR PrevPicOrderCntLsb			= stream_global->PrevPicOrderCntLsb[0];
					IMGPAR PrevPicOrderCntMsb			= stream_global->PrevPicOrderCntMsb[0];

					next_image_type = IMGPAR type;

					IMGPAR currentSlice = IMGPAR prevSlice;
					IMGPAR currentSlice->nextSlice = NULL;

					if(!bSkip)
					{
						dec_picture->pic_store_idx = IMGPAR UnCompress_Idx;
						ret = store_picture_in_dpb ARGS1(dec_picture);
						if (FAILED(ret)) {
							free_new_slice(newSlice);
							newSlice = NULL;
							stream_global->m_bSeekIDR = TRUE;
							nalu->buf = IMGPAR ori_nalu_buf;					
							IMGPAR slice_number = 0;
							break;
						}
						*header = SOP;
						EndDecodeFrame ARGS0();
					}

					free_new_slice(newSlice);
					newSlice = NULL;

					if(stream_global->m_bTrickEOS)
					{
						DEBUG_SHOW_HW_INFO("-- TrickMode EOS after I_SLICE--");
						next_image_type = IMGPAR firstSlice->picture_type;
						nalu_global_available = 0;
						IMGPAR currentSlice->exit_flag = 1;
						*header = EOS;
					}

					return CREL_OK;
				}
			}
			else 
			{   //Frame Picture
				if(IMGPAR slice_number)
				{					
					//Cpoy naul to nalu_global
					nalu_global->AU_HasSPS	= AU_HasSPS;
					nalu_global->startcodeprefix_len	= nalu->startcodeprefix_len;
					nalu_global->len					= nalu->len;
					nalu_global->max_size				= nalu->max_size;
					nalu_global->nal_unit_type			= nalu->nal_unit_type;
					nalu_global->nal_reference_idc		= nalu->nal_reference_idc;
					nalu_global->forbidden_bit			= nalu->forbidden_bit;
					nalu_global->pts					= nalu->pts;
					memcpy(nalu_global->buf, nalu->buf, nalu->len);
					nalu_global_available = 1;

					IMGPAR PreviousFrameNum				= stream_global->PreviousFrameNum[0];
					IMGPAR PreviousFrameNumOffset = stream_global->PreviousFrameNumOffset[0];
					IMGPAR PreviousPOC						= stream_global->PreviousPOC[0];
					IMGPAR ThisPOC								= stream_global->ThisPOC[0]; 
					IMGPAR PrevPicOrderCntLsb			= stream_global->PrevPicOrderCntLsb[0];
					IMGPAR PrevPicOrderCntMsb			= stream_global->PrevPicOrderCntMsb[0];

					next_image_type = IMGPAR type;

					IMGPAR currentSlice = IMGPAR prevSlice;
					IMGPAR currentSlice->nextSlice = NULL;

					if(!bSkip)
					{
						dec_picture->pic_store_idx = IMGPAR UnCompress_Idx;
						ret = store_picture_in_dpb ARGS1(dec_picture);
						if (FAILED(ret)) {
							stream_global->m_bSeekIDR = TRUE;
							nalu->buf = IMGPAR ori_nalu_buf;					
							IMGPAR slice_number = 0;
							break;
						}
						*header = SOP;
						EndDecodeFrame ARGS0();
					}

					free_new_slice(newSlice);
					newSlice = NULL;

					if(stream_global->m_bTrickEOS)
					{
						DEBUG_SHOW_HW_INFO("-- TrickMode EOS after I_SLICE--");
						next_image_type = IMGPAR firstSlice->picture_type;
						nalu_global_available = 0;
						IMGPAR currentSlice->exit_flag = 1;
						*header = EOS;
					}

					return CREL_OK;
				}
				else
				{
					UpdatePTS ARGS0();

					ZeroMemory(&(IMGPAR m_CCCode), sizeof(H264_CC));
					SetH264CCCode ARGS0();

					sps =  (IMGPAR currentSlice->bIsBaseView) ? &SeqParSet[PicParSet[IMGPAR currentSlice->pic_parameter_set_id].seq_parameter_set_id] :
						&SeqParSubset[PicParSet[IMGPAR currentSlice->pic_parameter_set_id].seq_parameter_set_id];
					pps =  &PicParSet[IMGPAR currentSlice->pic_parameter_set_id];
					ret = activate_sps ARGS2(sps, IMGPAR currentSlice->viewId);
					if (FAILED(ret)) {
						break;
					}
					activate_pps ARGS2(pps, IMGPAR currentSlice->viewIndex);

					IMGPAR currentSlice->header = SOP;
					IMGPAR currentSlice->m_pic_combine_status = FRAME;
					IMGPAR slice_number++;
					nalu->buf += nalu->len;

					stream_global->PreviousFrameNum[0]				= IMGPAR PreviousFrameNum;
					stream_global->PreviousFrameNumOffset[0]	= IMGPAR PreviousFrameNumOffset;
					stream_global->PreviousPOC[0]						= IMGPAR PreviousPOC;
					stream_global->ThisPOC[0]								= IMGPAR ThisPOC; 
					stream_global->PrevPicOrderCntLsb[0]			= IMGPAR PrevPicOrderCntLsb;
					stream_global->PrevPicOrderCntMsb[0]			= IMGPAR PrevPicOrderCntMsb;	

					IMGPAR currentSlice->m_nDispPicStructure = sps->vui_seq_parameters.picture_structure;

					DEBUG_SHOW_HW_INFO("##bSkip: %d, Type = %d, POC = %d##", bSkip, IMGPAR firstSlice->AU_type, IMGPAR ThisPOC);

					IMGPAR UnCompress_Idx = -1;
					IMGPAR m_Intra_lCompBufIndex = 0;
					IMGPAR m_lmbCount_Intra = 0;
					IMGPAR m_iIntraMCBufUsage = 0;
					IMGPAR m_iInterMCBufUsage = 0;
					IMGPAR m_slice_number_in_field = 1;

					if(!bSkip)
					{
						unsigned int view_index = (IMGPAR currentSlice) ? (IMGPAR currentSlice->viewIndex) : 0;
						if ((IMGPAR firstSlice->picture_type==B_SLICE)  && dpb.used_size_on_view[view_index])
						{
							StorablePicture *pPreFrame = NULL;


							if ( dpb.fs_on_view[view_index][dpb.used_size_on_view[view_index]-1]->frame ) {	// Field Combined or Frame Split work may not be done if that picture has error.
								pPreFrame = dpb.fs_on_view[view_index][dpb.used_size_on_view[view_index]-1]->frame;
							} else if (dpb.fs_on_view[view_index][dpb.used_size_on_view[view_index]-1]->top_field) {
								pPreFrame = dpb.fs_on_view[view_index][dpb.used_size_on_view[view_index]-1]->top_field;
							} else {
								pPreFrame = dpb.fs_on_view[view_index][dpb.used_size_on_view[view_index]-1]->bottom_field;
							}

							if(pPreFrame && (pPreFrame->slice_type==P_SLICE || pPreFrame->slice_type==I_SLICE))
							{
								g_pArrangedPCCCode = &(pPreFrame->m_CCCode);
								g_pArrangedPCCCode->ccbuf[LINE21BUF_SIZE-1] = 0;
							}

							if (g_pArrangedPCCCode)
								RearrangePBCCBuf ARGS0();
						}

						ret = initial_image ARGS0();
						if (FAILED(ret)) {
							free_new_slice(newSlice);
							newSlice = NULL;
							stream_global->m_bSeekIDR = TRUE;
							nalu->buf = IMGPAR ori_nalu_buf;					
							IMGPAR slice_number = 0;
							break;
						}

						memcpy(&(dec_picture->m_CCCode), &(IMGPAR m_CCCode), sizeof(H264_CC));

						ret = init_lists ARGS2(IMGPAR type, IMGPAR currentSlice->structure);
						if (FAILED(ret)) {
							free_new_slice(newSlice);
							newSlice = NULL;
							stream_global->m_bSeekIDR = TRUE;
							nalu->buf = IMGPAR ori_nalu_buf;					
							IMGPAR slice_number = 0;
							break;
						}
						ret = reorder_lists ARGS2((IMGPAR type), (IMGPAR currentSlice));
						if (FAILED(ret)) {
							free_new_slice(newSlice);
							newSlice = NULL;
							stream_global->m_bSeekIDR = TRUE;
							nalu->buf = IMGPAR ori_nalu_buf;					
							IMGPAR slice_number = 0;
							break;
						}

						ret = check_lists ARGS0();
						if (FAILED(ret)) {
							free_new_slice(newSlice);
							newSlice = NULL;
							stream_global->m_bSeekIDR = TRUE;
							nalu->buf = IMGPAR ori_nalu_buf;					
							IMGPAR slice_number = 0;
							break;
						}

						if (IMGPAR currentSlice->structure == FRAME)
							init_mbaff_lists ARGS0();
						free_ref_pic_list_reordering_buffer(IMGPAR currentSlice);

						//For long format weight offset
						if ((active_pps.weighted_bipred_idc > 0 && IMGPAR type == B_SLICE) || (active_pps.weighted_pred_flag && IMGPAR type == P_SLICE))
							fill_wp_params ARGS0();

						BA_build_picture_decode_buffer ARGS0();
						BA_DATA_CTRL = (DXVA_Slice_H264_Long *)m_lpSliceCtrlBuf;

						g_bReceivedFirst = (!g_bReceivedFirst) ? 1 : 2;
					}
				}
			}												
		}
		else
		{
			IMGPAR currentSlice->header = *header = SOS;				
			IMGPAR currentSlice->m_pic_combine_status = IMGPAR prevSlice->m_pic_combine_status;
			IMGPAR slice_number++;
			IMGPAR m_slice_number_in_field++;

			//In long format, for the second and above slice of multi-slice.
			if(!bSkip)
			{
				ret = initial_image ARGS0();
				if (FAILED(ret)) {
					return ret;
				}
				ret = init_lists ARGS2(IMGPAR type, IMGPAR currentSlice->structure);
				if (FAILED(ret)) {
					return ret;
				}
				ret = reorder_lists ARGS2((IMGPAR type), (IMGPAR currentSlice));
				if (FAILED(ret)) {
					return ret;
				}

				ret = check_lists ARGS0();
				if (FAILED(ret)) {
					return ret;
				}

				if (IMGPAR currentSlice->structure == FRAME)
					init_mbaff_lists ARGS0();
				free_ref_pic_list_reordering_buffer(IMGPAR currentSlice);

				//For long format weight offset
				if ((active_pps.weighted_bipred_idc > 0 && IMGPAR type == B_SLICE) || (active_pps.weighted_pred_flag && IMGPAR type == P_SLICE))
					fill_wp_params ARGS0();

				m_nSliceNum++;
			}
			//In long format, for the second and above slice of multi-slice.

			if (IMGPAR prevSlice)
			{
				IMGPAR prevSlice->exit_flag = 0;
				IMGPAR currentSlice->pts = IMGPAR prevSlice->pts;
				IMGPAR currentSlice->dts = IMGPAR prevSlice->dts;
				IMGPAR currentSlice->has_pts = IMGPAR prevSlice->has_pts;
				IMGPAR currentSlice->framerate1000 = IMGPAR prevSlice->framerate1000;
				IMGPAR currentSlice->NumClockTs = IMGPAR prevSlice->NumClockTs;
			}
			nalu->buf += nalu->len;

			IMGPAR currentSlice->m_nDispPicStructure = IMGPAR prevSlice->m_nDispPicStructure;

			BA_DATA_CTRL = (DXVA_Slice_H264_Long *)m_lpSliceCtrlBuf;
		}

		if(!bSkip)
		{
			int i, j;

			////For check prevention code
			//int data_pos, HW_pos=0 ,zero_count=0, prevention_code_num=0, temp_offset;
			//BYTE *temp = m_lpnalu, *temp_nalu;

			int offsetLength;
			int prevention_byte_cnt=0;
			BYTE *dataptr = m_lpnalu;
			DXVA_PicParams_H264* t_psPictureDecode = (DXVA_PicParams_H264*)IMGPAR m_pnv1PictureDecode;

			nalu_size = nalu->len+3+m_nNALUSkipByte;

			////check prevention code
			//data_pos = IMGPAR m_BitOffsetToSliceData >> 3;
			//if(IMGPAR m_BitOffsetToSliceData & 0x07)
			//	data_pos++;
			//temp+=4;

			//for(int m=0;m<(nalu_size-4);m++)
			//{
			//	HW_pos++;
			//	if(*temp==0)
			//		zero_count++;
			//	else
			//	{
			//		if(zero_count>1)
			//		{
			//			if(zero_count==ZEROBYTES_SHORTSTARTCODE && *temp==0x03)
			//			{	
			//				if(HW_pos<=data_pos)
			//				{
			//					prevention_code_num++;
			//					data_pos++;
			//				}
			//				else if((HW_pos-data_pos)==1)
			//				{
			//					temp_offset = IMGPAR m_BitOffsetToSliceData >> 3;
			//					if(IMGPAR m_BitOffsetToSliceData & 0x07)
			//						temp_offset++;
			//					temp_nalu = (BYTE*)_aligned_malloc(nalu_size-(4+temp_offset+1), 16);
			//					memcpy(temp_nalu, m_lpnalu+(4+temp_offset+1), nalu_size-(4+temp_offset+1));
			//					memcpy(m_lpnalu+(4+temp_offset), temp_nalu, nalu_size-(4+temp_offset+1));
			//					nalu_size--;
			//					_aligned_free(temp_nalu);

			//					break;
			//				}
			//				else if((HW_pos-data_pos)==2)
			//				{
			//					temp_offset = IMGPAR m_BitOffsetToSliceData >> 3;
			//					if(IMGPAR m_BitOffsetToSliceData & 0x07)
			//						temp_offset++;
			//					temp_nalu = (BYTE*)_aligned_malloc(nalu_size-(4+temp_offset+2), 16);
			//					memcpy(temp_nalu, m_lpnalu+(4+temp_offset+2), nalu_size-(4+temp_offset+2));
			//					memcpy(m_lpnalu+(4+temp_offset+1), temp_nalu, nalu_size-(4+temp_offset+2));
			//					nalu_size--;
			//					_aligned_free(temp_nalu);

			//					break;
			//				}
			//				else
			//					break;
			//			}
			//		}
			//		zero_count = 0;
			//	}
			//	temp++;
			//}
			//IMGPAR m_BitOffsetToSliceData += (prevention_code_num<<3);
			////check prevention code

			if (m_nDXVAMode == E_H264_DXVA_INTEL_MODE_E) { //Currently Intel platform would count prevention code

				offsetLength = IMGPAR m_BitOffsetToSliceData >> 3;
				if(IMGPAR m_BitOffsetToSliceData & 0x07)
					offsetLength++;
				dataptr += 4;

				for(int m = 0; m < offsetLength; m++)
				{
					if(dataptr[0] == 0 && dataptr[1] == 0 && dataptr[2] == 3)
					{
						dataptr += 2;
						if((m + 2) >= offsetLength)
							*dataptr = 0;
						prevention_byte_cnt++;
						offsetLength++;
						m += 2;
					}
					dataptr++;
				}
				IMGPAR m_BitOffsetToSliceData += (prevention_byte_cnt<<3);
			}		//S3 would not count prevention code, and no other platform support long format so far
			//check prevention code

			//BA_DATA_CTRL->BSNALunitDataLocation = (int)(m_lpBitstreamBuf - IMGPAR m_lpMV);
			BA_DATA_CTRL->BSNALunitDataLocation = IMGPAR m_iIntraMCBufUsage;
			BA_DATA_CTRL->SliceBytesInBuffer = nalu_size;
			shiftoffset = (nalu_size & 15);
			if(shiftoffset)
			{
				memset((m_lpnalu+nalu_size), 0,(16-shiftoffset));
				nalu_size += (16-shiftoffset);
			}
			memcpy(m_lpBitstreamBuf, m_lpnalu, nalu_size);
			m_lpBitstreamBuf += nalu_size;
			//BA_DATA_CTRL->SliceBytesInBuffer = nalu_size;
			BA_DATA_CTRL->wBadSliceChopping = 0;

			//*****For Intel only supports long format*****
			if (t_psPictureDecode->IntraPicFlag == 1 && (IMGPAR currentSlice->picture_type == P_SLICE 
				|| IMGPAR currentSlice->picture_type == B_SLICE || IMGPAR currentSlice->picture_type == RB_SLICE))
				t_psPictureDecode->IntraPicFlag = 0;  //For the first slice of P, B pictures is I slice  

			BA_DATA_CTRL->first_mb_in_slice = IMGPAR currentSlice->start_mb_nr;
			BA_DATA_CTRL->NumMbsForSlice = 0;
			BA_DATA_CTRL->BitOffsetToSliceData = IMGPAR m_BitOffsetToSliceData;

			if (m_nVGAType == E_H264_VGACARD_INTEL) {
				BA_DATA_CTRL->slice_type = (IMGPAR currentSlice->picture_type+5);
			} else if (m_nVGAType == E_H264_VGACARD_S3) {
				BA_DATA_CTRL->slice_type = IMGPAR currentSlice->picture_type;
			} else {
				BA_DATA_CTRL->slice_type = IMGPAR currentSlice->picture_type;
			}

			//BA_DATA_CTRL->luma_log2_weight_denom = IMGPAR luma_log2_weight_denom;
			//BA_DATA_CTRL->chroma_log2_weight_denom = IMGPAR chroma_log2_weight_denom;
			if(IMGPAR currentSlice->picture_type != I_SLICE && IMGPAR apply_weights)
			{
				BA_DATA_CTRL->luma_log2_weight_denom = IMGPAR luma_log2_weight_denom;
				BA_DATA_CTRL->chroma_log2_weight_denom = IMGPAR chroma_log2_weight_denom;
			}
			//BA_DATA_CTRL->num_ref_idx_l0_active_minus1 = IMGPAR currentSlice->m_active_pps->num_ref_idx_l0_active_minus1;
			//BA_DATA_CTRL->num_ref_idx_l1_active_minus1 = IMGPAR currentSlice->m_active_pps->num_ref_idx_l1_active_minus1;
			BA_DATA_CTRL->num_ref_idx_l0_active_minus1 = ((listXsize[0]-1)<0 ? 0 : listXsize[0]-1);
			BA_DATA_CTRL->num_ref_idx_l1_active_minus1 = ((listXsize[1]-1)<0 ? 0 : listXsize[1]-1);
			BA_DATA_CTRL->slice_alpha_c0_offset_div2 = IMGPAR currentSlice->LFAlphaC0Offset / 2;
			BA_DATA_CTRL->slice_beta_offset_div2 = IMGPAR currentSlice->LFBetaOffset /2;

			memset(BA_DATA_CTRL->RefPicList,255,64);
			if(IMGPAR currentSlice->picture_type == P_SLICE)
			{
				for(i = 0; i < listXsize[0]; i++)
				{
					BA_DATA_CTRL->RefPicList[0][i].Index7Bits	= *(byte*)&m_pSurfaceInfo[listX[0][i]->unique_id].SInfo[0];
					BA_DATA_CTRL->RefPicList[0][i].AssociatedFlag = m_pSurfaceInfo[listX[0][i]->unique_id].SInfo[1];
				}
				if(IMGPAR MbaffFrameFlag)
				{
					//Top L0 map to list index
					for(j = 0; j < listXsize[2]; j++)
						for(i = 0; i < listXsize[0]; i++)
							if(m_pSurfaceInfo[listX[0][i]->unique_id].SInfo[0] == m_pSurfaceInfo[listX[2][j]->unique_id].SInfo[0])
							{
								IMGPAR m_TopL0Map[j] = i;
								break;
							}
							//Bottom L0 map to list index
							for(j = 0; j < listXsize[4]; j++)
								for(i = 0; i < listXsize[0]; i++)
									if(m_pSurfaceInfo[listX[0][i]->unique_id].SInfo[0] == m_pSurfaceInfo[listX[4][j]->unique_id].SInfo[0])
									{
										IMGPAR m_BotL0Map[j] = i;
										break;
									}
				}
			}
			else if (IMGPAR currentSlice->picture_type == B_SLICE)
			{
				for(i = 0; i < listXsize[0]; i++)
				{
					BA_DATA_CTRL->RefPicList[0][i].Index7Bits	= *(byte*)&m_pSurfaceInfo[listX[0][i]->unique_id].SInfo[0];
					BA_DATA_CTRL->RefPicList[0][i].AssociatedFlag = m_pSurfaceInfo[listX[0][i]->unique_id].SInfo[1];
				}
				for(i = 0; i < listXsize[1]; i++)
				{
					BA_DATA_CTRL->RefPicList[1][i].Index7Bits	= *(byte*)&m_pSurfaceInfo[listX[1][i]->unique_id].SInfo[0];
					BA_DATA_CTRL->RefPicList[1][i].AssociatedFlag = m_pSurfaceInfo[listX[1][i]->unique_id].SInfo[1];
				}
				if(IMGPAR MbaffFrameFlag)
				{
					//Top L0 map to list index
					for(j = 0; j < listXsize[2]; j++)
						for(i = 0; i < listXsize[0]; i++)
							if(m_pSurfaceInfo[listX[0][i]->unique_id].SInfo[0] == m_pSurfaceInfo[listX[2][j]->unique_id].SInfo[0])
							{
								IMGPAR m_TopL0Map[j] = i;
								break;
							}
							//Top L1 map to list index
							for(j = 0; j < listXsize[3]; j++)
								for(i = 0; i < listXsize[1]; i++)
									if(m_pSurfaceInfo[listX[1][i]->unique_id].SInfo[0] == m_pSurfaceInfo[listX[3][j]->unique_id].SInfo[0])
									{
										IMGPAR m_TopL1Map[j] = i;
										break;
									}
									//Bottom L0 map to list index
									for(j = 0; j < listXsize[4]; j++)
										for(i = 0; i < listXsize[0]; i++)
											if(m_pSurfaceInfo[listX[0][i]->unique_id].SInfo[0] == m_pSurfaceInfo[listX[4][j]->unique_id].SInfo[0])
											{
												IMGPAR m_BotL0Map[j] = i;
												break;
											}
											//Bottom L1 map to list index
											for(j = 0; j < listXsize[5]; j++)
												for(i = 0; i < listXsize[1]; i++)
													if(m_pSurfaceInfo[listX[1][i]->unique_id].SInfo[0] == m_pSurfaceInfo[listX[5][j]->unique_id].SInfo[0])
													{
														IMGPAR m_BotL1Map[j] = i;
														break;
													}
				}
			}

			if(IMGPAR currentSlice->picture_type != I_SLICE && IMGPAR apply_weights)
			{
				for(i = 0; i < listXsize[0]; i++)
				{
					BA_DATA_CTRL->Weights[0][i][0][0] = *((IMGPAR wp_weight+(i+0)*3+0));
					BA_DATA_CTRL->Weights[0][i][0][1] = *((IMGPAR wp_offset+(i+0)*3+0));
					BA_DATA_CTRL->Weights[0][i][1][0] = *((IMGPAR wp_weight+(i+0)*3+1));
					BA_DATA_CTRL->Weights[0][i][1][1] = *((IMGPAR wp_offset+(i+0)*3+1));
					BA_DATA_CTRL->Weights[0][i][2][0] = *((IMGPAR wp_weight+(i+0)*3+2));
					BA_DATA_CTRL->Weights[0][i][2][1] = *((IMGPAR wp_offset+(i+0)*3+2));
				}
				if(IMGPAR currentSlice->picture_type == B_SLICE)
				{
					for(i = 0; i < listXsize[1]; i++)
					{
						BA_DATA_CTRL->Weights[1][i][0][0] = *((IMGPAR wp_weight+(MAX_REFERENCE_PICTURES+i)*3+0));
						BA_DATA_CTRL->Weights[1][i][0][1] = *((IMGPAR wp_offset+(MAX_REFERENCE_PICTURES+i)*3+0));
						BA_DATA_CTRL->Weights[1][i][1][0] = *((IMGPAR wp_weight+(MAX_REFERENCE_PICTURES+i)*3+1));
						BA_DATA_CTRL->Weights[1][i][1][1] = *((IMGPAR wp_offset+(MAX_REFERENCE_PICTURES+i)*3+1));
						BA_DATA_CTRL->Weights[1][i][2][0] = *((IMGPAR wp_weight+(MAX_REFERENCE_PICTURES+i)*3+2));
						BA_DATA_CTRL->Weights[1][i][2][1] = *((IMGPAR wp_offset+(MAX_REFERENCE_PICTURES+i)*3+2));
					}
				}
			}
			BA_DATA_CTRL->slice_qp_delta = IMGPAR currentSlice->slice_qp_delta;
			//BA_DATA_CTRL->redundant_pic_cnt = IMGPAR currentSlice->m_active_pps->redundant_pic_cnt_present_flag;
			BA_DATA_CTRL->redundant_pic_cnt = active_pps.redundant_pic_cnt_present_flag;
			BA_DATA_CTRL->direct_spatial_mv_pred_flag = IMGPAR direct_spatial_mv_pred_flag;
			BA_DATA_CTRL->cabac_init_idc = IMGPAR model_number;
			BA_DATA_CTRL->disable_deblocking_filter_idc = IMGPAR currentSlice->LFDisableIdc;
			BA_DATA_CTRL->slice_id = m_nSliceNum;//IMGPAR current_slice_nr;
			//*****For Intel only supports long format*****

			m_lpSliceCtrlBuf += sizeof(DXVA_Slice_H264_Long);
			BA_DATA_CTRL = (DXVA_Slice_H264_Long *)m_lpSliceCtrlBuf;
			IMGPAR m_iInterMCBufUsage += sizeof(DXVA_Slice_H264_Long);
			IMGPAR m_iIntraMCBufUsage += nalu_size;
			IMGPAR m_lmbCount_Intra++;

			DEBUG_SHOW_HW_INFO("IMGPAR m_iInterMCBufUsage:%d, nalu_size:%d, nalu_len:%d, nalu_skipbyte:%d", IMGPAR m_iInterMCBufUsage, nalu_size, nalu->len, m_nNALUSkipByte);
		}

		break;
	case NALU_TYPE_DPA:
		return CREL_ERROR_H264_SYNCNOTFOUND;
		break;
	case NALU_TYPE_DPC:			
		return CREL_ERROR_H264_SYNCNOTFOUND;
		break;
	case NALU_TYPE_SEI:
		DEBUG_SHOW_HW_INFO("-- NALU_TYPE_SEI --");
		DEBUG_SHOW_HW_INFO ("read_new_slice: Found NALU_TYPE_SEI, len %d\n", nalu->len);
		ret = InterpretSEIMessage ARGS2(nalu->buf,nalu->len);
		if (FAILED(ret)) {

			if (ISWARNING(ret)) {

				if (ISWARNINGLEVEL_1(ret)) {
					stream_global->m_bSeekIDR = TRUE;
					nalu->buf = IMGPAR ori_nalu_buf;
					for ( unsigned int i = 0; i < stream_global->num_views; i++) {
						stream_global->m_active_sps_on_view[i]= 0;
					}
				}

				break;

			} else {
				return ret;
			}
		}	
		break;
	case NALU_TYPE_PPS:
		DEBUG_SHOW_HW_INFO("-- NALU_TYPE_PPS --");
		//if (m_newpps && ((IMGPAR field_pic_flag && new_picture==2) || (!IMGPAR field_pic_flag && new_picture)))
		//{
		//	nalu_global->AU_HasSPS	= AU_HasSPS;
		//	nalu_global->startcodeprefix_len	= nalu->startcodeprefix_len;
		//	nalu_global->len					= nalu->len;
		//	nalu_global->max_size				= nalu->max_size;
		//	nalu_global->nal_unit_type			= nalu->nal_unit_type;
		//	nalu_global->nal_reference_idc		= nalu->nal_reference_idc;
		//	nalu_global->forbidden_bit			= nalu->forbidden_bit;
		//	nalu_global->pts					= nalu->pts;
		//	memcpy(nalu_global->buf, nalu->buf, nalu->len);
		//	nalu_global_available = 1;

		//	if(!bSkip)
		//	{
		//		if (dec_picture)
		//		{
		//			dec_picture->pic_store_idx = IMGPAR UnCompress_Idx;
		//			ret = store_picture_in_dpb ARGS1(dec_picture);
		//			if (FAILED(ret)) {
		//				return ret;
		//			}
		//			EndDecodeFrame ARGS0();
		//		}

		//		if (IMGPAR slice_number > 0)
		//			IMGPAR currentSlice->exit_flag = 1;

		//		if (stream_global->m_bTrickEOS)
		//		{
		//			DEBUG_SHOW_HW_INFO("-- TrickMode EOS after PPS--");
		//			next_image_type = IMGPAR firstSlice->picture_type;
		//			nalu_global_available = 0;
		//			*header = EOS;
		//			return CREL_OK;
		//		}
		//	}
		//	*header = SOP;
		//	return CREL_OK;
		//}

		m_newpps = true;
		ret = ProcessPPS ARGS1(nalu);			
		break;
	case NALU_TYPE_SPS:
		DEBUG_SHOW_HW_INFO("-- NALU_TYPE_SPS --");
		AU_HasSPS = 1;
		if ( CheckSPS ARGS2(nalu, 0) || (IMGPAR slice_number > 0 && IMGPAR firstSlice->idr_flag) || m_newsps)
		{
			nalu_global->AU_HasSPS	= AU_HasSPS;
			nalu_global->startcodeprefix_len	= nalu->startcodeprefix_len;
			nalu_global->len					= nalu->len;
			nalu_global->max_size				= nalu->max_size;
			nalu_global->nal_unit_type			= nalu->nal_unit_type;
			nalu_global->nal_reference_idc		= nalu->nal_reference_idc;
			nalu_global->forbidden_bit			= nalu->forbidden_bit;
			nalu_global->pts					= nalu->pts;
			memcpy(nalu_global->buf, nalu->buf, nalu->len);
			nalu_global_available = 1;

			if(!bSkip)
			{
				if (dec_picture)
				{
					dec_picture->pic_store_idx = IMGPAR UnCompress_Idx;
					ret = store_picture_in_dpb ARGS1(dec_picture);
					if (FAILED(ret)) {
						stream_global->m_bSeekIDR = TRUE;
						break;
					}
					EndDecodeFrame ARGS0();
				}

				if (IMGPAR slice_number > 0)
					IMGPAR currentSlice->exit_flag = 1;

				if (stream_global->m_bTrickEOS)
				{
					DEBUG_SHOW_HW_INFO("-- TrickMode EOS after SPS--");
					next_image_type = IMGPAR firstSlice->picture_type;
					nalu_global_available = 0;
					*header = EOS;
					return CREL_OK;
				}
			}
			*header = (m_newsps ? SOP : NALU_TYPE_SPS);
			return CREL_OK;
		}

		m_newsps = true;
		ret = ProcessSPS ARGS1(nalu);
		break;
	case NALU_TYPE_PREFIX:


		ret = ProcessNaluExt ARGS1(nalu);



		if (FAILED(ret)) {

			if (ISWARNING(ret)) {

				if (ISWARNINGLEVEL_1(ret)) {
					stream_global->m_bSeekIDR = TRUE;
					nalu->buf = IMGPAR ori_nalu_buf;
				}

				break;

			} else {
				return ret;
			}
		}



		break;

	case NALU_TYPE_SPS_SUBSET:
		DEBUG_SHOW_SW_INFO("-- NALU_TYPE_SPS_SUBSET --");

		//If bitstream is IDR only and IDR are all the same, decoder needs to return here.
/*		if (IMGPAR slice_number > 0 && IMGPAR firstSlice->idr_flag)
		{
			nalu_global->startcodeprefix_len = nalu->startcodeprefix_len;
			nalu_global->len                 = nalu->len;
			nalu_global->max_size            = nalu->max_size;
			nalu_global->nal_unit_type       = nalu->nal_unit_type;
			nalu_global->nal_reference_idc   = nalu->nal_reference_idc;
			nalu_global->forbidden_bit       = nalu->forbidden_bit;
			nalu_global->pts                 = nalu->pts;
			memcpy(nalu_global->buf, nalu->buf, nalu->len);
			nalu_global_available = 1;

			IMGPAR currentSlice->exit_flag = 1;

			return CREL_OK;
		}
*/
		ret = ProcessSPSSubset ARGS1(nalu);
		if (FAILED(ret)) {

			if (ISWARNING(ret)) {

				if (ISWARNINGLEVEL_1(ret)) {
					stream_global->m_bSeekIDR = TRUE;
					nalu->buf = IMGPAR ori_nalu_buf;
				}

				break;

			} else {
				return ret;
			}
		}

		break;
	case NALU_TYPE_AUD:
		AU_HasSPS = 0;
		ret = ProcessAUD ARGS2(nalu, &primary_pic_type);	
		break;
	case NALU_TYPE_EOSEQ:
		DEBUG_SHOW_HW_INFO ("read_new_slice: Found 'End of Sequence' NAL unit, len %d, ignored\n", nalu->len);
		break;
	case NALU_TYPE_EOSTREAM:
		DEBUG_SHOW_HW_INFO ("read_new_slice: Found 'End of Stream' NAL unit, len %d, ignored\n", nalu->len);
		break;
	case NALU_TYPE_FILL:
		DEBUG_SHOW_HW_INFO ("read_new_slice: Found NALU_TYPE_FILL, len %d\n", nalu->len);
		DEBUG_SHOW_HW_INFO ("Skipping these filling bits, proceeding w/ next NALU\n");
		break;
	default:
		DEBUG_SHOW_HW_INFO("-- NALU_TYPE_default --");
		DEBUG_SHOW_HW_INFO("Found NALU type %d, len %d undefined, ignore NALU, moving on\n", nalu->nal_unit_type, nalu->len);
	}

	if (FAILED(ret)) {

		if (g_has_pts)
		{
			g_has_pts = 0;
			DEBUG_INFO("[Drop PTS]: ts: %I64d freq: %d, tslength: %d flags: %d unused1: %d unused2: %d", nalu->pts.ts, nalu->pts.freq, nalu->pts.tslength, nalu->pts.flags, nalu->pts.unused1, nalu->pts.unused2);
		}

		if (IMGPAR structure == FRAME) {

			if(dec_picture)
			{
				if (dpb.used_size_on_view && ((dpb.used_size_on_view[0] == 0) ||dpb.fs_on_view[0][dpb.used_size_on_view[0] - 1]->frame != dec_picture)) {
					dec_picture->pic_store_idx = img->UnCompress_Idx;
					release_storable_picture ARGS2(dec_picture, 1);
					img->UnCompress_Idx = -1;
					dec_picture->pic_store_idx = -1;
				}
				dec_picture = NULL;
			}
			img->m_dec_picture_top = NULL;
			img->m_dec_picture_bottom = NULL;



		} else {

			if (img->m_dec_picture_top && (img->m_dec_picture_top->top_field_first)) {

				if(img->m_dec_picture_top && img->m_dec_picture_bottom)
				{
					if (dpb.used_size_on_view && ((dpb.used_size_on_view[0] == 0) ||dpb.fs_on_view[0][dpb.used_size_on_view[0] - 1]->bottom_field != img->m_dec_picture_bottom)) {

						if (dpb.used_size_on_view && (dpb.used_size_on_view[0] && dpb.fs_on_view[0][dpb.used_size_on_view[0] - 1]->top_field && (dpb.fs_on_view[0][dpb.used_size_on_view[0] - 1]->top_field->pic_store_idx == img->UnCompress_Idx))){
							img->m_dec_picture_bottom->pic_store_idx = -1;
						} else {
							img->m_dec_picture_bottom->pic_store_idx = img->UnCompress_Idx;
						}

						//img->m_dec_picture_bottom->pic_store_idx = img->UnCompress_Idx;

						release_storable_picture ARGS2(img->m_dec_picture_bottom, 1);
						img->UnCompress_Idx = -1;
						img->m_dec_picture_bottom->pic_store_idx = -1;
					}

				} else {

					if (dpb.used_size_on_view && ((dpb.used_size_on_view[0] == 0) ||dpb.fs_on_view[0][dpb.used_size_on_view[0] - 1]->top_field != img->m_dec_picture_top)) {
						if (img->m_dec_picture_top) {
							img->m_dec_picture_top->pic_store_idx = img->UnCompress_Idx;
						}
						release_storable_picture ARGS2(img->m_dec_picture_top, 1);	
						img->UnCompress_Idx = -1;

					}

				}
				img->m_dec_picture_top = NULL;
				img->m_dec_picture_bottom = NULL;


			} else {

				if(img->m_dec_picture_top && img->m_dec_picture_bottom)
				{
					if (dpb.used_size_on_view && ((dpb.used_size_on_view[0] == 0) ||dpb.fs_on_view[0][dpb.used_size_on_view[0] - 1]->top_field != img->m_dec_picture_top)) {

						if (dpb.used_size_on_view && (dpb.used_size_on_view[0] && dpb.fs_on_view[0][dpb.used_size_on_view[0] - 1]->top_field && (dpb.fs_on_view[0][dpb.used_size_on_view[0] - 1]->bottom_field->pic_store_idx == img->UnCompress_Idx))){
							img->m_dec_picture_top->pic_store_idx = -1;
						} else {
							img->m_dec_picture_top->pic_store_idx = img->UnCompress_Idx;
						}

						//img->m_dec_picture_top->pic_store_idx = img->UnCompress_Idx;

						release_storable_picture ARGS2(img->m_dec_picture_top, 1);
						img->UnCompress_Idx = -1;
						img->m_dec_picture_top->pic_store_idx = -1;
					}

				} else {

					if (dpb.used_size_on_view && ((dpb.used_size_on_view[0] == 0) ||dpb.fs_on_view[0][dpb.used_size_on_view[0] - 1]->bottom_field != img->m_dec_picture_bottom)) {
						if (img->m_dec_picture_bottom) {
							img->m_dec_picture_bottom->pic_store_idx = img->UnCompress_Idx;
						}
						release_storable_picture ARGS2(img->m_dec_picture_bottom, 1);
						img->UnCompress_Idx = -1;						
					}

				}
				img->m_dec_picture_top = NULL;
				img->m_dec_picture_bottom = NULL;


			}		


			dec_picture = NULL;			

		}	

		for (int i=0 ;i<1; i++)
		{
			img = img_array[i];
			if (i==0)
			{
				IMGPAR currentSlice = IMGPAR firstSlice;
				for (int j=0; j< IMGPAR slice_number; j++)
				{
					//active_pps = &PicParSet[IMGPAR currentSlice->pic_parameter_set_id];
					memcpy(&active_pps, &PicParSet[IMGPAR currentSlice->pic_parameter_set_id], sizeof(pic_parameter_set_rbsp_t));
					IMGPAR prevSlice = IMGPAR currentSlice;
					IMGPAR currentSlice = (Slice*) IMGPAR currentSlice->nextSlice;
					free_new_slice(IMGPAR prevSlice);
				}
			}
			IMGPAR firstSlice = IMGPAR currentSlice = IMGPAR prevSlice = NULL;
			IMGPAR current_mb_nr_r = IMGPAR current_mb_nr_d = -4712;   // impossible value for debugging, StW
			IMGPAR error_mb_nr = -4712;
			IMGPAR current_slice_nr = 0;
			IMGPAR slice_number = 0;
			img->m_active_pps[0].Valid = NULL;
			img->m_active_pps[1].Valid = NULL;
			dec_picture = NULL;
			img->m_dec_picture_top = NULL;
			img->m_dec_picture_bottom = NULL;

			if (IMGPAR structure != FRAME)
			{
				IMGPAR mb_decdata = IMGPAR mb_decdata_ori;
				IMGPAR cof_array = IMGPAR cof_array_ori;
			}
		}

		if (stream_global->m_bSeekIDR) {
			if(dpb.used_size_on_view[0])
			{
				for (unsigned int i=0; i <storable_picture_count; i++)				
					storable_picture_map[i]->used_for_first_field_reference = 0;


				if (img->structure  == FRAME) {

					if (dpb.fs_on_view[0][dpb.used_size_on_view[0]-1]->frame && dpb.fs_on_view[0][dpb.used_size_on_view[0]-1]->frame->pic_store_idx == img->UnCompress_Idx){
						img->UnCompress_Idx = -1;
					}

				} else {

					if ((dpb.fs_on_view[0][dpb.used_size_on_view[0]-1]->top_field && dpb.fs_on_view[0][dpb.used_size_on_view[0]-1]->top_field->pic_store_idx == img->UnCompress_Idx) ||
						(dpb.fs_on_view[0][dpb.used_size_on_view[0]-1]->bottom_field && dpb.fs_on_view[0][dpb.used_size_on_view[0]-1]->bottom_field->pic_store_idx == img->UnCompress_Idx)) {
							img->UnCompress_Idx = -1;
					}
				}				

				flush_dpb ARGS1(0);
				update_ref_list ARGS1(0);
				update_ltref_list ARGS1(0);		
			}

			nalu_global_available = 0;
		}

		return ret;
	}

	return  CREL_ERROR_H264_SYNCNOTFOUND;
}

CREL_RETURN CH264DXVA2::BA_build_picture_decode_buffer PARGS0()
{
	HRESULT	hr = S_OK;
	int i, j;
	int view_index = (IMGPAR currentSlice) ? (IMGPAR currentSlice->viewIndex) : 0;

	pic_parameter_set_rbsp_t *pps = &active_pps;
	seq_parameter_set_rbsp_t *sps = &active_sps;
	if(active_sps.Valid)
		sps =  (IMGPAR currentSlice->bIsBaseView) ? &SeqParSet[PicParSet[IMGPAR currentSlice->pic_parameter_set_id].seq_parameter_set_id] :
			&SeqParSubset[PicParSet[IMGPAR currentSlice->pic_parameter_set_id].seq_parameter_set_id];

	AssignQuantParam ARGS2(&active_pps, &active_sps);

	if(m_bResolutionChange && get_param_fcn != NULL)
	{
#if 1
		HVDService::HVDDecodeConfig DecodeConfig;
		ZeroMemory(&DecodeConfig, sizeof(HVDService::HVDDecodeConfig));
		m_pIHVDService->GetHVDDecodeConfig(&DecodeConfig);
		if (DecodeConfig.dwWidth != IMGPAR width || DecodeConfig.dwHeight != IMGPAR height)
		{
			DecodeConfig.dwWidth = IMGPAR width;
			DecodeConfig.dwHeight = IMGPAR height;

			//Change display resolution and Get new IVICP
			(*get_param_fcn)(H264_PROPID_CB_CHANGE_RESOLUTION, H264_pvDataContext, (LPVOID*)&m_pIviCP, NULL, &DecodeConfig, sizeof(HVDService::HVDDecodeConfig));

			ResetDXVABuffers();
		}
#else
		hr = Send_Resolution(H264_pvDataContext,IMGPAR width,IMGPAR height,(dec_picture->dwXAspect==16 ? 3 : 2), (LPVOID*)&m_pIviCP);
		if(FAILED(hr))
			return -1;

		ResetDXVABuffers();

		if(m_pIviCP)
			m_pIviCP->GenerateKey();
#endif

		m_bResolutionChange = FALSE;
	}

	hr = GetBuffer(DXVA2_PictureParametersBufferType, (LPVOID*)(&IMGPAR m_pnv1PictureDecode), NULL);
	DEBUG_SHOW_HW_INFO("GetBuffer() PicPar, return value = %d, address = %d", hr, IMGPAR m_pnv1PictureDecode);
	hr = GetBuffer(DXVA2_InverseQuantizationMatrixBufferType, (LPVOID*)(&IMGPAR m_lpMBLK_Intra_Luma), NULL);
	DEBUG_SHOW_HW_INFO("GetBuffer() IQM, return value = %d, address = %d", hr, IMGPAR m_lpMBLK_Intra_Luma);
	hr = GetBuffer(DXVA2_SliceControlBufferType, (LPVOID*)(&IMGPAR m_lpSLICE), NULL);
	DEBUG_SHOW_HW_INFO("GetBuffer() SliceCtrl, return value = %d, address = %d", hr, IMGPAR m_lpSLICE);
	hr = GetBuffer(DXVA2_BitStreamDateBufferType, (LPVOID*)(&IMGPAR m_lpMV), NULL);
	DEBUG_SHOW_HW_INFO("GetBuffer() BitstreamData, return value = %d, address = %d", hr, IMGPAR m_lpMV);

	if(m_nVGAType != E_H264_VGACARD_INTEL)
		m_lpBitstreamBuf = IMGPAR m_lpMV;
	else
		m_lpBitstreamBuf = m_lpBitstreamBuf_Intel;

	m_lpSliceCtrlBuf = IMGPAR m_lpSLICE;

	if(IMGPAR UnCompress_Idx == -1)
	{
		while (1)
		{
			IMGPAR UnCompress_Idx = (int)m_pUncompBufQueue->GetItem();

			hr = BeginFrame(IMGPAR UnCompress_Idx, 1);
			if(checkDDError(hr))
				m_pUncompBufQueue->PutItem(IMGPAR UnCompress_Idx);
			else
			{
				EndFrame();
				break;
			}
		}
	}

	IMGPAR m_lFrame_Counter = m_nFrameCounter++;

	DXVA_BufferDescription *t_psDxvaBufferDescription = IMGPAR m_pDxvaBufferDescription;
	AMVABUFFERINFO         *t_psBufferInfo            = IMGPAR m_pBufferInfo;
	DXVA_PicParams_H264* t_psPictureDecode = (DXVA_PicParams_H264*)IMGPAR m_pnv1PictureDecode;
	DXVA_Qmatrix_H264 *t_psQpMatrix = (DXVA_Qmatrix_H264*)IMGPAR m_lpMBLK_Intra_Luma;
	memset(t_psPictureDecode, 0, sizeof(DXVA_PicParams_H264));
	memset(t_psQpMatrix, 0, sizeof(DXVA_Qmatrix_H264));

	t_psPictureDecode->CurrPic.Index7Bits		= IMGPAR UnCompress_Idx;
	//t_psPictureDecode->CurrPic.AssociatedFlag = (IMGPAR currentSlice->structure != 0);
	t_psPictureDecode->CurrPic.AssociatedFlag = (IMGPAR field_pic_flag & (IMGPAR currentSlice->structure >> 1));
	t_psPictureDecode->wFrameWidthInMbsMinus1		= sps->pic_width_in_mbs_minus1;
	t_psPictureDecode->wFrameHeightInMbsMinus1		= IMGPAR FrameHeightInMbs-1;

	t_psPictureDecode->num_ref_frames = sps->num_ref_frames;
	t_psPictureDecode->field_pic_flag = IMGPAR field_pic_flag;   //field_pic_flag
	t_psPictureDecode->MbaffFrameFlag = IMGPAR currentSlice->MbaffFrameFlag;  //MbaffFrameFlag
	t_psPictureDecode->residual_colour_transform_flag = 0;  //residual_colour_transform_flag
	t_psPictureDecode->sp_for_switch_flag =  0;  //sp_for_switch_flag
	t_psPictureDecode->chroma_format_idc = sps->chroma_format_idc;  //chroma_format_idc
	t_psPictureDecode->RefPicFlag = dec_picture->used_for_reference;
	t_psPictureDecode->constrained_intra_pred_flag = pps->constrained_intra_pred_flag; //constrained_intra_pred_flag
	t_psPictureDecode->weighted_pred_flag = pps->weighted_pred_flag;  //weighted_pred_flag
	t_psPictureDecode->weighted_bipred_idc = pps->weighted_bipred_idc;  //weighted_bipred_idc
	t_psPictureDecode->MbsConsecutiveFlag = 1; //restricted mode profile is 0. //MbsConsecutiveFlag
	t_psPictureDecode->frame_mbs_only_flag = sps->frame_mbs_only_flag;  //frame_mbs_only_flag
	t_psPictureDecode->transform_8x8_mode_flag = pps->transform_8x8_mode_flag;  //transform_8x8_mode_flag
	t_psPictureDecode->MinLumaBipredSize8x8Flag = 1; //set to 1 for Main,High,High 10,High 422,or High 444 of levels3.1 and higher.
	t_psPictureDecode->IntraPicFlag = (IMGPAR currentSlice->picture_type == I_SLICE);  //IntraPicFlag
	t_psPictureDecode->bit_depth_luma_minus8 = sps->bit_depth_luma_minus8;
	t_psPictureDecode->bit_depth_chroma_minus8 = sps->bit_depth_chroma_minus8;
	t_psPictureDecode->StatusReportFeedbackNumber = 0;
	if(t_psPictureDecode->field_pic_flag)
	{
		if(t_psPictureDecode->CurrPic.AssociatedFlag)
		{
			m_RefInfo[IMGPAR UnCompress_Idx].top_poc = t_psPictureDecode->CurrFieldOrderCnt[0] = dec_picture->top_poc = 0;
			m_RefInfo[IMGPAR UnCompress_Idx].bot_poc = t_psPictureDecode->CurrFieldOrderCnt[1] = dec_picture->bottom_poc = IMGPAR bottompoc;
		}
		else
		{	
			m_RefInfo[IMGPAR UnCompress_Idx].top_poc = t_psPictureDecode->CurrFieldOrderCnt[0] = dec_picture->top_poc = IMGPAR toppoc;
			m_RefInfo[IMGPAR UnCompress_Idx].bot_poc = t_psPictureDecode->CurrFieldOrderCnt[1] = dec_picture->bottom_poc = 0;
		}
	}
	else
	{
		m_RefInfo[IMGPAR UnCompress_Idx].top_poc = t_psPictureDecode->CurrFieldOrderCnt[0] = dec_picture->top_poc = IMGPAR toppoc;
		m_RefInfo[IMGPAR UnCompress_Idx].bot_poc = t_psPictureDecode->CurrFieldOrderCnt[1] = dec_picture->bottom_poc = IMGPAR bottompoc;
	}

	DEBUG_SHOW_HW_INFO("poc:%d", t_psPictureDecode->CurrFieldOrderCnt[0]);

	t_psPictureDecode->pic_init_qs_minus26 = pps->pic_init_qs_minus26;
	t_psPictureDecode->chroma_qp_index_offset = pps->chroma_qp_index_offset;
	t_psPictureDecode->second_chroma_qp_index_offset = pps->second_chroma_qp_index_offset;
	t_psPictureDecode->ContinuationFlag = 1;
	t_psPictureDecode->pic_init_qp_minus26 = pps->pic_init_qp_minus26;
	t_psPictureDecode->num_ref_idx_l0_active_minus1 = pps->num_ref_idx_l0_active_minus1;
	t_psPictureDecode->num_ref_idx_l1_active_minus1 = pps->num_ref_idx_l1_active_minus1;
	t_psPictureDecode->UsedForReferenceFlags = 0;
	t_psPictureDecode->NonExistingFrameFlags = 0;
	m_RefInfo[IMGPAR UnCompress_Idx].frame_num = t_psPictureDecode->frame_num = IMGPAR currentSlice->frame_num;
	t_psPictureDecode->log2_max_frame_num_minus4 = sps->log2_max_frame_num_minus4;
	t_psPictureDecode->pic_order_cnt_type = sps->pic_order_cnt_type;
	t_psPictureDecode->log2_max_pic_order_cnt_lsb_minus4 = sps->log2_max_pic_order_cnt_lsb_minus4;
	t_psPictureDecode->delta_pic_order_always_zero_flag = sps->delta_pic_order_always_zero_flag;
	t_psPictureDecode->direct_8x8_inference_flag = sps->direct_8x8_inference_flag;
	t_psPictureDecode->entropy_coding_mode_flag = pps->entropy_coding_mode_flag;
	t_psPictureDecode->pic_order_present_flag = pps->pic_order_present_flag;
	t_psPictureDecode->num_slice_groups_minus1 = pps->num_slice_groups_minus1;
	t_psPictureDecode->slice_group_map_type = pps->slice_group_map_type;
	t_psPictureDecode->deblocking_filter_control_present_flag = pps->deblocking_filter_control_present_flag;
	t_psPictureDecode->redundant_pic_cnt_present_flag = pps->redundant_pic_cnt_present_flag;
	t_psPictureDecode->slice_group_change_rate_minus1 = pps->slice_group_change_rate_minus1;

	memset(t_psPictureDecode->RefFrameList,255,16);
	int dbp_loop_size = min(dpb.used_size_on_view[view_index], active_sps.num_ref_frames);
	StorablePicture *RefFrame;

	if(GetBitstreamRawConfig() == E_BA_RAW_SHORTFORMAT)
	{

		for(i=0, j = 0; i<dpb.used_size_on_view[view_index]; i++)
		{
			if(dpb.fs_on_view[view_index][i]->is_reference)
			{
				if(dpb.fs_on_view[view_index][i]->is_reference == 3)
				{
					RefFrame = dpb.fs_on_view[view_index][i]->frame;
					t_psPictureDecode->FieldOrderCntList[j][0] = RefFrame->top_poc;
					t_psPictureDecode->FieldOrderCntList[j][1] = RefFrame->bottom_poc;
				}
				else
				{
					if(dpb.fs_on_view[view_index][i]->is_reference == 1)
					{
						RefFrame = dpb.fs_on_view[view_index][i]->top_field;
						t_psPictureDecode->FieldOrderCntList[j][0] = RefFrame->top_poc;
					}
					else
					{
						RefFrame = dpb.fs_on_view[view_index][i]->bottom_field;
						t_psPictureDecode->FieldOrderCntList[j][1] = RefFrame->bottom_poc;
					}
				}
				t_psPictureDecode->RefFrameList[j].Index7Bits	= RefFrame->pic_store_idx;	//*(byte*)&m_pSurfaceinfo[listX[0][i]->unique_id].SInfo[0];
				t_psPictureDecode->RefFrameList[j].AssociatedFlag = 0;
				t_psPictureDecode->FrameNumList[j] = RefFrame->frame_num;

				t_psPictureDecode->UsedForReferenceFlags |= (dpb.fs_on_view[view_index][i]->is_reference << (j*2));

				j++;
			}
		}
	}
	else
	{
		for(i=0; i<dpb.used_size_on_view[view_index]; i++)
		{
			if(dpb.fs_on_view[view_index][i]->is_reference)
			{
				if(dpb.fs_on_view[view_index][i]->is_reference == 3)
				{
					RefFrame = dpb.fs_on_view[view_index][i]->frame;
					t_psPictureDecode->FieldOrderCntList[RefFrame->pic_store_idx][0] = RefFrame->top_poc;
					t_psPictureDecode->FieldOrderCntList[RefFrame->pic_store_idx][1] = RefFrame->bottom_poc;
				}
				else
				{
					if(dpb.fs_on_view[view_index][i]->is_reference == 1)
					{
						RefFrame = dpb.fs_on_view[view_index][i]->top_field;
						t_psPictureDecode->FieldOrderCntList[RefFrame->pic_store_idx][0] = RefFrame->top_poc;
					}
					else
					{
						RefFrame = dpb.fs_on_view[view_index][i]->bottom_field;
						t_psPictureDecode->FieldOrderCntList[RefFrame->pic_store_idx][1] = RefFrame->bottom_poc;
					}
				}
				t_psPictureDecode->RefFrameList[RefFrame->pic_store_idx].Index7Bits	= RefFrame->pic_store_idx;	//*(byte*)&m_pSurfaceinfo[listX[0][i]->unique_id].SInfo[0];
				t_psPictureDecode->RefFrameList[RefFrame->pic_store_idx].AssociatedFlag = 0;
				t_psPictureDecode->FrameNumList[RefFrame->pic_store_idx] = RefFrame->frame_num;
				t_psPictureDecode->UsedForReferenceFlags |= (dpb.fs_on_view[view_index][i]->is_reference << ((RefFrame->pic_store_idx)*2));
			}
		}
	}

	DXVA_PicParams_H264_MVC *mvc_param = (DXVA_PicParams_H264_MVC*)(&t_psPictureDecode->SliceGroupMap[0]);
	memset(mvc_param, 255, sizeof(DXVA_PicParams_H264_MVC));
	mvc_param->num_views_minus1 = stream_global->num_views - 1;
	mvc_param->view_id = IMGPAR currentSlice->viewId;
	mvc_param->inter_view_flag = IMGPAR currentSlice->interViewFlag;

	if(view_index <= 0 || IMGPAR type == I_SLICE)
	{
		mvc_param->num_inter_view_refs_l0 = 0;
		mvc_param->num_inter_view_refs_l1 = 0;
	}
	else if(IMGPAR type == P_SLICE)
	{
		int index = 0;
		if( IMGPAR currentSlice->structure == FRAME)
		{
			if ( img->currentSlice->anchorPicFlag )
			{
				int num_ref = active_sps.num_anchor_refs_l0[view_index];
				for ( i = 0; i < num_ref; i++) 
				{
					int anchor_refs_view_index = GetViewIndex ARGS1(active_sps.anchor_refs_l0[view_index][i]);
					int ref_dpb_size = dpb.used_size_on_view[anchor_refs_view_index];

					for ( j = 0; j < ref_dpb_size; j++ ) 
					{						
						if (( dpb.fs_on_view[anchor_refs_view_index][j]->poc == IMGPAR ThisPOC) 
							&& dpb.fs_on_view[anchor_refs_view_index][j]->is_used == 3 && dpb.fs_on_view[anchor_refs_view_index][j]->frame->inter_view_flag )
						{
							StorablePicture *RefFrame = dpb.fs_on_view[anchor_refs_view_index][j]->frame;
							mvc_param->InterViewRefsL0[index].Index7Bits = RefFrame->pic_store_idx;
							mvc_param->InterViewRefsL0[index++].AssociatedFlag = 0;
						}
					}
				}
				mvc_param->num_inter_view_refs_l0 = index;
			}
			else
			{
				int num_ref = active_sps.num_non_anchor_refs_l0[view_index];
				for ( i = 0; i < num_ref; i++) 
				{
					int non_anchor_refs_view_index = GetViewIndex ARGS1(active_sps.non_anchor_refs_l0[view_index][i]);
					int ref_dpb_size = dpb.used_size_on_view[non_anchor_refs_view_index];

					for ( j = 0; j < ref_dpb_size; j++ ) 
					{						
						if (( dpb.fs_on_view[non_anchor_refs_view_index][j]->poc == IMGPAR ThisPOC) 
							&& dpb.fs_on_view[non_anchor_refs_view_index][j]->is_used == 3 && dpb.fs_on_view[non_anchor_refs_view_index][j]->frame->inter_view_flag )
						{
							StorablePicture *RefFrame = dpb.fs_on_view[non_anchor_refs_view_index][j]->frame;
							mvc_param->InterViewRefsL0[index].Index7Bits = RefFrame->pic_store_idx;
							mvc_param->InterViewRefsL0[index++].AssociatedFlag = 0;
						}
					}
				}
				mvc_param->num_inter_view_refs_l0 = index;
			}

			mvc_param->num_inter_view_refs_l1 = 0;
		}
		else
		{
			if ( img->currentSlice->anchorPicFlag )
			{
				int num_ref = active_sps.num_anchor_refs_l0[view_index];
				for ( i = 0; i < num_ref; i++) 
				{
					int anchor_refs_view_index = GetViewIndex ARGS1(active_sps.anchor_refs_l0[view_index][i]);
					int ref_dpb_size = dpb.used_size_on_view[anchor_refs_view_index];

					for ( j = 0; j < ref_dpb_size; j++ ) 
					{
						if(IMGPAR currentSlice->structure == TOP_FIELD)
						{
							if (( dpb.fs_on_view[anchor_refs_view_index][j]->top_field->poc == IMGPAR ThisPOC) 
								&& (dpb.fs_on_view[anchor_refs_view_index][j]->is_used & TOP_FIELD) && dpb.fs_on_view[anchor_refs_view_index][j]->top_field->inter_view_flag )
							{
								StorablePicture *RefFrame = dpb.fs_on_view[anchor_refs_view_index][j]->top_field;
								mvc_param->InterViewRefsL0[index].Index7Bits = RefFrame->pic_store_idx;
								mvc_param->InterViewRefsL0[index++].AssociatedFlag = 0;
							}
						}
						else if(IMGPAR currentSlice->structure == BOTTOM_FIELD)
						{
							if (( dpb.fs_on_view[anchor_refs_view_index][j]->bottom_field->poc == IMGPAR ThisPOC) 
								&& (dpb.fs_on_view[anchor_refs_view_index][j]->is_used & BOTTOM_FIELD) && dpb.fs_on_view[anchor_refs_view_index][j]->bottom_field->inter_view_flag )
							{
								StorablePicture *RefFrame = dpb.fs_on_view[anchor_refs_view_index][j]->bottom_field;
								mvc_param->InterViewRefsL0[index].Index7Bits = RefFrame->pic_store_idx;
								mvc_param->InterViewRefsL0[index++].AssociatedFlag = 0;
							}
						}
						
					}
				}
				mvc_param->num_inter_view_refs_l0 = index;
			}
			else
			{
				int num_ref = active_sps.num_non_anchor_refs_l0[view_index];
				for ( i = 0; i < num_ref; i++) 
				{
					int non_anchor_refs_view_index = GetViewIndex ARGS1(active_sps.non_anchor_refs_l0[view_index][i]);
					int ref_dpb_size = dpb.used_size_on_view[non_anchor_refs_view_index];

					for ( j = 0; j < ref_dpb_size; j++ ) 
					{
						if(IMGPAR currentSlice->structure == TOP_FIELD)
						{
							if (( dpb.fs_on_view[non_anchor_refs_view_index][j]->top_field->poc == IMGPAR ThisPOC) 
								&& (dpb.fs_on_view[non_anchor_refs_view_index][j]->is_used & TOP_FIELD) && dpb.fs_on_view[non_anchor_refs_view_index][j]->top_field->inter_view_flag )
							{
								StorablePicture *RefFrame = dpb.fs_on_view[non_anchor_refs_view_index][j]->top_field;
								mvc_param->InterViewRefsL0[index].Index7Bits = RefFrame->pic_store_idx;
								mvc_param->InterViewRefsL0[index++].AssociatedFlag = 0;
							}
						}
						else if(IMGPAR currentSlice->structure == BOTTOM_FIELD)
						{
							if (( dpb.fs_on_view[non_anchor_refs_view_index][j]->bottom_field->poc == IMGPAR ThisPOC) 
								&& (dpb.fs_on_view[non_anchor_refs_view_index][j]->is_used & BOTTOM_FIELD) && dpb.fs_on_view[non_anchor_refs_view_index][j]->bottom_field->inter_view_flag )
							{
								StorablePicture *RefFrame = dpb.fs_on_view[non_anchor_refs_view_index][j]->bottom_field;
								mvc_param->InterViewRefsL0[index].Index7Bits = RefFrame->pic_store_idx;
								mvc_param->InterViewRefsL0[index++].AssociatedFlag = 0;
							}
						}
						
					}
				}
				mvc_param->num_inter_view_refs_l0 = index;
			}

			mvc_param->num_inter_view_refs_l1 = 0;
		}
		
	}
	else	// B_SLICE
	{
		int index_l0 = 0, index_l1 = 0;

		if( IMGPAR currentSlice->structure == FRAME)
		{
			if ( img->currentSlice->anchorPicFlag ) 
			{
				int num_ref = active_sps.num_anchor_refs_l0[view_index];
				for ( i = 0; i < num_ref; i++) {
					int anchor_refs_view_index = GetViewIndex ARGS1(active_sps.anchor_refs_l0[view_index][i]);
					int ref_dpb_size = dpb.used_size_on_view[anchor_refs_view_index];

					for ( j = 0; j < ref_dpb_size; j++ ) {
						if (( dpb.fs_on_view[anchor_refs_view_index][j]->poc == IMGPAR ThisPOC) 
							&& dpb.fs_on_view[anchor_refs_view_index][j]->is_used == 3 && dpb.fs_on_view[anchor_refs_view_index][j]->frame->inter_view_flag ){
								StorablePicture *RefFrame = dpb.fs_on_view[anchor_refs_view_index][j]->frame;
								mvc_param->InterViewRefsL0[index_l0].Index7Bits = RefFrame->pic_store_idx;
								mvc_param->InterViewRefsL0[index_l0++].AssociatedFlag = 0;
						}
					}
				}
				mvc_param->num_inter_view_refs_l0 = index_l0;

				num_ref = active_sps.num_anchor_refs_l1[view_index];
				for ( i = 0; i < num_ref; i++) {
					int anchor_refs_view_index = GetViewIndex ARGS1(active_sps.anchor_refs_l1[view_index][i]);
					int ref_dpb_size = dpb.used_size_on_view[anchor_refs_view_index];

					for ( j = 0; j < ref_dpb_size; j++ ) {
						if (( dpb.fs_on_view[anchor_refs_view_index][j]->poc == IMGPAR ThisPOC) 
							&& dpb.fs_on_view[anchor_refs_view_index][j]->is_used == 3 && dpb.fs_on_view[anchor_refs_view_index][j]->frame->inter_view_flag ){
								StorablePicture *RefFrame = dpb.fs_on_view[anchor_refs_view_index][j]->frame;
								mvc_param->InterViewRefsL1[index_l1].Index7Bits = RefFrame->pic_store_idx;
								mvc_param->InterViewRefsL1[index_l1++].AssociatedFlag = 0;
						}
					}
				}
			mvc_param->num_inter_view_refs_l1 = index_l1;
			}
			else
			{
				int num_ref = active_sps.num_non_anchor_refs_l0[view_index];
				for ( i = 0; i < num_ref; i++) {
					int non_anchor_refs_view_index = GetViewIndex ARGS1(active_sps.non_anchor_refs_l0[view_index][i]);
					int ref_dpb_size = dpb.used_size_on_view[non_anchor_refs_view_index];

					for ( j = 0; j < ref_dpb_size; j++ ) {
						if (( dpb.fs_on_view[non_anchor_refs_view_index][j]->poc == IMGPAR ThisPOC)
							&& dpb.fs_on_view[non_anchor_refs_view_index][j]->is_used == 3 && dpb.fs_on_view[non_anchor_refs_view_index][j]->frame->inter_view_flag ){
								StorablePicture *RefFrame = dpb.fs_on_view[non_anchor_refs_view_index][j]->frame;
								mvc_param->InterViewRefsL0[index_l0].Index7Bits = RefFrame->pic_store_idx;
								mvc_param->InterViewRefsL0[index_l0++].AssociatedFlag = 0;
						}
					}
				}
				mvc_param->num_inter_view_refs_l0 = index_l0;

				num_ref = active_sps.num_non_anchor_refs_l1[view_index];
				for ( i = 0; i < num_ref; i++) {
					int non_anchor_refs_view_index = GetViewIndex ARGS1(active_sps.non_anchor_refs_l1[view_index][i]);
					int ref_dpb_size = dpb.used_size_on_view[non_anchor_refs_view_index];
					for ( j = 0; j < ref_dpb_size; j++ ) {
						if (( dpb.fs_on_view[non_anchor_refs_view_index][j]->poc == IMGPAR ThisPOC)
							&& dpb.fs_on_view[non_anchor_refs_view_index][j]->is_used == 3 && dpb.fs_on_view[non_anchor_refs_view_index][j]->frame->inter_view_flag ){
								StorablePicture *RefFrame = dpb.fs_on_view[non_anchor_refs_view_index][j]->frame;
								mvc_param->InterViewRefsL1[index_l1].Index7Bits = RefFrame->pic_store_idx;
								mvc_param->InterViewRefsL1[index_l1++].AssociatedFlag = 0;
						}
					}
				}
				mvc_param->num_inter_view_refs_l1 = index_l1;
			}
		}
		else
		{
			if ( img->currentSlice->anchorPicFlag ) 
			{
				int num_ref = active_sps.num_anchor_refs_l0[view_index];
				for ( i = 0; i < num_ref; i++) {
					int anchor_refs_view_index = GetViewIndex ARGS1(active_sps.anchor_refs_l0[view_index][i]);
					int ref_dpb_size = dpb.used_size_on_view[anchor_refs_view_index];

					for ( j = 0; j < ref_dpb_size; j++ ) {

						if(IMGPAR currentSlice->structure == TOP_FIELD)
						{
							if (( dpb.fs_on_view[anchor_refs_view_index][j]->top_field->poc == IMGPAR ThisPOC) 
								&& (dpb.fs_on_view[anchor_refs_view_index][j]->is_used & TOP_FIELD) && dpb.fs_on_view[anchor_refs_view_index][j]->top_field->inter_view_flag )
							{
								StorablePicture *RefFrame = dpb.fs_on_view[anchor_refs_view_index][j]->top_field;
								mvc_param->InterViewRefsL0[index_l0].Index7Bits = RefFrame->pic_store_idx;
								mvc_param->InterViewRefsL0[index_l0++].AssociatedFlag = 0;
							}
						}
						else if(IMGPAR currentSlice->structure == BOTTOM_FIELD)
						{
							if (( dpb.fs_on_view[anchor_refs_view_index][j]->bottom_field->poc == IMGPAR ThisPOC) 
								&& (dpb.fs_on_view[anchor_refs_view_index][j]->is_used & BOTTOM_FIELD) && dpb.fs_on_view[anchor_refs_view_index][j]->bottom_field->inter_view_flag )
							{
								StorablePicture *RefFrame = dpb.fs_on_view[anchor_refs_view_index][j]->bottom_field;
								mvc_param->InterViewRefsL0[index_l0].Index7Bits = RefFrame->pic_store_idx;
								mvc_param->InterViewRefsL0[index_l0++].AssociatedFlag = 0;
							}
						}
						
					}
				}
				mvc_param->num_inter_view_refs_l0 = index_l0;

				num_ref = active_sps.num_anchor_refs_l1[view_index];
				for ( i = 0; i < num_ref; i++) {
					int anchor_refs_view_index = GetViewIndex ARGS1(active_sps.anchor_refs_l1[view_index][i]);
					int ref_dpb_size = dpb.used_size_on_view[anchor_refs_view_index];

					for ( j = 0; j < ref_dpb_size; j++ ) 
					{

						if(IMGPAR currentSlice->structure == TOP_FIELD)
						{
							if (( dpb.fs_on_view[anchor_refs_view_index][j]->top_field->poc == IMGPAR ThisPOC) 
								&& dpb.fs_on_view[anchor_refs_view_index][j]->is_used & TOP_FIELD && dpb.fs_on_view[anchor_refs_view_index][j]->top_field->inter_view_flag )
							{
								StorablePicture *RefFrame = dpb.fs_on_view[anchor_refs_view_index][j]->top_field;
								mvc_param->InterViewRefsL1[index_l1].Index7Bits = RefFrame->pic_store_idx;
								mvc_param->InterViewRefsL1[index_l1++].AssociatedFlag = 0;
							}
							
						}
						else if(IMGPAR currentSlice->structure == BOTTOM_FIELD)
						{
							if (( dpb.fs_on_view[anchor_refs_view_index][j]->bottom_field->poc == IMGPAR ThisPOC) 
								&& dpb.fs_on_view[anchor_refs_view_index][j]->is_used & BOTTOM_FIELD && dpb.fs_on_view[anchor_refs_view_index][j]->bottom_field->inter_view_flag )
							{
								StorablePicture *RefFrame = dpb.fs_on_view[anchor_refs_view_index][j]->bottom_field;
								mvc_param->InterViewRefsL1[index_l1].Index7Bits = RefFrame->pic_store_idx;
								mvc_param->InterViewRefsL1[index_l1++].AssociatedFlag = 0;
							}
						}
						
					}
				}
				mvc_param->num_inter_view_refs_l1 = index_l1;
			}
			else
			{
				int num_ref = active_sps.num_non_anchor_refs_l0[view_index];
				for ( i = 0; i < num_ref; i++) {
					int non_anchor_refs_view_index = GetViewIndex ARGS1(active_sps.non_anchor_refs_l0[view_index][i]);
					int ref_dpb_size = dpb.used_size_on_view[non_anchor_refs_view_index];

					for ( j = 0; j < ref_dpb_size; j++ ) {
						if(IMGPAR currentSlice->structure == TOP_FIELD)
						{
							if (( dpb.fs_on_view[non_anchor_refs_view_index][j]->top_field->poc == IMGPAR ThisPOC)
								&& dpb.fs_on_view[non_anchor_refs_view_index][j]->is_used & TOP_FIELD && dpb.fs_on_view[non_anchor_refs_view_index][j]->top_field->inter_view_flag )
							{
								StorablePicture *RefFrame = dpb.fs_on_view[non_anchor_refs_view_index][j]->top_field;
								mvc_param->InterViewRefsL0[index_l0].Index7Bits = RefFrame->pic_store_idx;
								mvc_param->InterViewRefsL0[index_l0++].AssociatedFlag = 0;
							}
						}
						else if(IMGPAR currentSlice->structure == BOTTOM_FIELD)
						{
							if (( dpb.fs_on_view[non_anchor_refs_view_index][j]->bottom_field->poc == IMGPAR ThisPOC)
								&& dpb.fs_on_view[non_anchor_refs_view_index][j]->is_used & BOTTOM_FIELD && dpb.fs_on_view[non_anchor_refs_view_index][j]->bottom_field->inter_view_flag )
							{
								StorablePicture *RefFrame = dpb.fs_on_view[non_anchor_refs_view_index][j]->bottom_field;
								mvc_param->InterViewRefsL0[index_l0].Index7Bits = RefFrame->pic_store_idx;
								mvc_param->InterViewRefsL0[index_l0++].AssociatedFlag = 0;
							}
						}
					}
				}
				mvc_param->num_inter_view_refs_l0 = index_l0;

				num_ref = active_sps.num_non_anchor_refs_l1[view_index];
				for ( i = 0; i < num_ref; i++) {
					int non_anchor_refs_view_index = GetViewIndex ARGS1(active_sps.non_anchor_refs_l1[view_index][i]);
					int ref_dpb_size = dpb.used_size_on_view[non_anchor_refs_view_index];
					for ( j = 0; j < ref_dpb_size; j++ ) {
						if(IMGPAR currentSlice->structure == TOP_FIELD)
						{
							if (( dpb.fs_on_view[non_anchor_refs_view_index][j]->top_field->poc == IMGPAR ThisPOC)
								&& dpb.fs_on_view[non_anchor_refs_view_index][j]->is_used & TOP_FIELD && dpb.fs_on_view[non_anchor_refs_view_index][j]->top_field->inter_view_flag )
							{
								StorablePicture *RefFrame = dpb.fs_on_view[non_anchor_refs_view_index][j]->top_field;
								mvc_param->InterViewRefsL1[index_l1].Index7Bits = RefFrame->pic_store_idx;
								mvc_param->InterViewRefsL1[index_l1++].AssociatedFlag = 0;
							}
						}
						else if(IMGPAR currentSlice->structure == BOTTOM_FIELD)
						{
							if (( dpb.fs_on_view[non_anchor_refs_view_index][j]->bottom_field->poc == IMGPAR ThisPOC)
								&& dpb.fs_on_view[non_anchor_refs_view_index][j]->is_used & BOTTOM_FIELD && dpb.fs_on_view[non_anchor_refs_view_index][j]->bottom_field->inter_view_flag )
							{
								StorablePicture *RefFrame = dpb.fs_on_view[non_anchor_refs_view_index][j]->bottom_field;
								mvc_param->InterViewRefsL1[index_l1].Index7Bits = RefFrame->pic_store_idx;
								mvc_param->InterViewRefsL1[index_l1++].AssociatedFlag = 0;
							}
						}
						
					}
				}
				mvc_param->num_inter_view_refs_l1 = index_l1;
			}
		}
	}

	//copy QP matrix
	for(i = 0; i<6; i++)
		for(j = 0; j<16; j++)
			t_psQpMatrix->bScalingLists4x4[i][j] = qmatrix[i][SNGL_SCAN_2D[j]];
	for(i = 6; i<8; i++)
		for(j = 0; j<64; j++)
			t_psQpMatrix->bScalingLists8x8[i-6][j] = qmatrix[i][SNGL_SCAN8x8_2D[j]];

	SetSurfaceInfo ARGS3(dec_picture->unique_id, IMGPAR currentSlice->structure==2?1:0, 1);

	memset(&t_psDxvaBufferDescription[0], 0, 4*sizeof(DXVA_BufferDescription));
	t_psDxvaBufferDescription[0].dwTypeIndex    = t_psBufferInfo[0].dwTypeIndex	= DXVA2_PictureParametersBufferType;
	t_psDxvaBufferDescription[0].dwBufferIndex  = t_psBufferInfo[0].dwBufferIndex = IMGPAR m_Intra_lCompBufIndex;
	t_psDxvaBufferDescription[0].dwDataOffset   = t_psBufferInfo[0].dwDataOffset = 0;
	t_psDxvaBufferDescription[0].dwDataSize     = t_psBufferInfo[0].dwDataSize = sizeof(DXVA_PicParams_H264);
	t_psDxvaBufferDescription[0].dwNumMBsInBuffer	= 0;

	t_psDxvaBufferDescription[1].dwTypeIndex    = t_psBufferInfo[1].dwTypeIndex	= DXVA2_InverseQuantizationMatrixBufferType;
	t_psDxvaBufferDescription[1].dwBufferIndex  = t_psBufferInfo[1].dwBufferIndex = IMGPAR m_Intra_lCompBufIndex;
	t_psDxvaBufferDescription[1].dwDataOffset   = t_psBufferInfo[1].dwDataOffset = 0;
	t_psDxvaBufferDescription[1].dwDataSize     = t_psBufferInfo[1].dwDataSize = sizeof(DXVA_Qmatrix_H264);
	t_psDxvaBufferDescription[1].dwNumMBsInBuffer	= 0;

	return 0;
}

CREL_RETURN CH264DXVA2::BA_build_picture_decode_buffer_SECOP PARGS0()
{
	int i, j;
	HRESULT	hr = S_OK;
	DEBUG_SHOW_HW_INFO("[NV_SECOP]build_picture_decode_buffer_BA_SECOP() BEGIN");

	int view_index = (IMGPAR currentSlice) ? (IMGPAR currentSlice->viewIndex) : 0;

	pic_parameter_set_rbsp_t *pps = &active_pps;
	seq_parameter_set_rbsp_t *sps = &active_sps;
	if(active_sps.Valid)
		sps =  (IMGPAR currentSlice->bIsBaseView) ? &SeqParSet[PicParSet[IMGPAR currentSlice->pic_parameter_set_id].seq_parameter_set_id] :
				&SeqParSubset[PicParSet[IMGPAR currentSlice->pic_parameter_set_id].seq_parameter_set_id];	

	AssignQuantParam ARGS2(&active_pps, &active_sps);

	if(m_bResolutionChange && get_param_fcn != NULL)
	{
#if 1
		HVDService::HVDDecodeConfig DecodeConfig;
		ZeroMemory(&DecodeConfig, sizeof(HVDService::HVDDecodeConfig));
		m_pIHVDService->GetHVDDecodeConfig(&DecodeConfig);
		if (DecodeConfig.dwWidth != IMGPAR width || DecodeConfig.dwHeight != IMGPAR height)
		{
			DecodeConfig.dwWidth = IMGPAR width;
			DecodeConfig.dwHeight = IMGPAR height;

			//Change display resolution and Get new IVICP
			(*get_param_fcn)(H264_PROPID_CB_CHANGE_RESOLUTION, H264_pvDataContext, NULL, NULL, &DecodeConfig, sizeof(HVDService::HVDDecodeConfig));

			ResetDXVABuffers();
		}
#else
		hr = Send_Resolution(H264_pvDataContext,IMGPAR width,IMGPAR height,(dec_picture->dwXAspect==16 ? 3 : 2), (LPVOID*)&m_pIviCP);
		if(FAILED(hr))
			return -1;

		ResetDXVABuffers();
#endif

		m_bResolutionChange = FALSE;
	}

	hr = GetBuffer(DXVA2_PictureParametersBufferType, (LPVOID*)(&IMGPAR m_pnv1PictureDecode), NULL);
	DEBUG_SHOW_HW_INFO("GetBuffer() PicPar, return value = %d, address = %d", hr, IMGPAR m_pnv1PictureDecode);
	hr = GetBuffer(DXVA2_InverseQuantizationMatrixBufferType, (LPVOID*)(&IMGPAR m_lpMBLK_Intra_Luma), NULL);
	DEBUG_SHOW_HW_INFO("GetBuffer() IQM, return value = %d, address = %d", hr, IMGPAR m_lpMBLK_Intra_Luma);
	hr = GetBuffer(DXVA2_SliceControlBufferType, (LPVOID*)(&IMGPAR m_lpSLICE), NULL);
	DEBUG_SHOW_HW_INFO("GetBuffer() SliceCtrl, return value = %d, address = %d", hr, IMGPAR m_lpSLICE);

	m_lpSliceCtrlBuf = IMGPAR m_lpSLICE;

	if(IMGPAR UnCompress_Idx == -1)
	{
		while (1)
		{
			IMGPAR UnCompress_Idx = (int)m_pUncompBufQueue->GetItem();

			hr = BeginFrame(IMGPAR UnCompress_Idx, 1);
			if(checkDDError(hr))
				m_pUncompBufQueue->PutItem(IMGPAR UnCompress_Idx);
			else 
			{
				EndFrame();
				break;
			}
		}
	}

	IMGPAR m_lFrame_Counter = m_nFrameCounter++;

	DXVA_PicParams_H264* t_psPictureDecode = (DXVA_PicParams_H264*)IMGPAR m_pnv1PictureDecode;
	memset(t_psPictureDecode, 0, sizeof(DXVA_PicParams_H264));

	t_psPictureDecode->CurrPic.Index7Bits		= IMGPAR UnCompress_Idx;
	//t_psPictureDecode->CurrPic.AssociatedFlag = (IMGPAR currentSlice->structure != 0);
	t_psPictureDecode->CurrPic.AssociatedFlag = (IMGPAR field_pic_flag & (IMGPAR currentSlice->structure >> 1));
	t_psPictureDecode->wFrameWidthInMbsMinus1		= sps->pic_width_in_mbs_minus1;
	t_psPictureDecode->wFrameHeightInMbsMinus1		= IMGPAR FrameHeightInMbs-1;

	t_psPictureDecode->num_ref_frames = sps->num_ref_frames;
	t_psPictureDecode->field_pic_flag = IMGPAR field_pic_flag;   //field_pic_flag
	t_psPictureDecode->MbaffFrameFlag = IMGPAR currentSlice->MbaffFrameFlag;  //MbaffFrameFlag
	t_psPictureDecode->residual_colour_transform_flag = 0;  //residual_colour_transform_flag
	t_psPictureDecode->sp_for_switch_flag =  0;  //sp_for_switch_flag
	t_psPictureDecode->chroma_format_idc = sps->chroma_format_idc;  //chroma_format_idc
	t_psPictureDecode->RefPicFlag = dec_picture->used_for_reference;
	t_psPictureDecode->constrained_intra_pred_flag = pps->constrained_intra_pred_flag; //constrained_intra_pred_flag
	t_psPictureDecode->weighted_pred_flag = pps->weighted_pred_flag;  //weighted_pred_flag
	t_psPictureDecode->weighted_bipred_idc = pps->weighted_bipred_idc;  //weighted_bipred_idc
	t_psPictureDecode->MbsConsecutiveFlag = 1; //restricted mode profile is 0. //MbsConsecutiveFlag
	t_psPictureDecode->frame_mbs_only_flag = sps->frame_mbs_only_flag;  //frame_mbs_only_flag
	t_psPictureDecode->transform_8x8_mode_flag = pps->transform_8x8_mode_flag;  //transform_8x8_mode_flag
	t_psPictureDecode->MinLumaBipredSize8x8Flag = 1; //set to 1 for Main,High,High 10,High 422,or High 444 of levels3.1 and higher.
	t_psPictureDecode->IntraPicFlag = (IMGPAR currentSlice->picture_type == I_SLICE);  //IntraPicFlag
	t_psPictureDecode->bit_depth_luma_minus8 = sps->bit_depth_luma_minus8;
	t_psPictureDecode->bit_depth_chroma_minus8 = sps->bit_depth_chroma_minus8;
	t_psPictureDecode->StatusReportFeedbackNumber = 0;
	if(t_psPictureDecode->field_pic_flag)
	{
		if(t_psPictureDecode->CurrPic.AssociatedFlag)
		{
			m_RefInfo[IMGPAR UnCompress_Idx].top_poc = t_psPictureDecode->CurrFieldOrderCnt[0] = dec_picture->top_poc = 0;
			m_RefInfo[IMGPAR UnCompress_Idx].bot_poc = t_psPictureDecode->CurrFieldOrderCnt[1] = dec_picture->bottom_poc = IMGPAR bottompoc;
		}
		else
		{	
			m_RefInfo[IMGPAR UnCompress_Idx].top_poc = t_psPictureDecode->CurrFieldOrderCnt[0] = dec_picture->top_poc = IMGPAR toppoc;
			m_RefInfo[IMGPAR UnCompress_Idx].bot_poc = t_psPictureDecode->CurrFieldOrderCnt[1] = dec_picture->bottom_poc = 0;
		}
	}
	else
	{
		m_RefInfo[IMGPAR UnCompress_Idx].top_poc = t_psPictureDecode->CurrFieldOrderCnt[0] = dec_picture->top_poc = IMGPAR toppoc;
		m_RefInfo[IMGPAR UnCompress_Idx].bot_poc = t_psPictureDecode->CurrFieldOrderCnt[1] = dec_picture->bottom_poc = IMGPAR bottompoc;
	}

	DEBUG_SHOW_HW_INFO("poc:%d", t_psPictureDecode->CurrFieldOrderCnt[0]);

	t_psPictureDecode->pic_init_qs_minus26 = pps->pic_init_qs_minus26;
	t_psPictureDecode->chroma_qp_index_offset = pps->chroma_qp_index_offset;
	t_psPictureDecode->second_chroma_qp_index_offset = pps->second_chroma_qp_index_offset;
	t_psPictureDecode->ContinuationFlag = 1;
	t_psPictureDecode->pic_init_qp_minus26 = pps->pic_init_qp_minus26;
	t_psPictureDecode->num_ref_idx_l0_active_minus1 = pps->num_ref_idx_l0_active_minus1;
	t_psPictureDecode->num_ref_idx_l1_active_minus1 = pps->num_ref_idx_l1_active_minus1;
	t_psPictureDecode->UsedForReferenceFlags = 0;
	t_psPictureDecode->NonExistingFrameFlags = 0;
	m_RefInfo[IMGPAR UnCompress_Idx].frame_num = t_psPictureDecode->frame_num = IMGPAR currentSlice->frame_num;
	t_psPictureDecode->log2_max_frame_num_minus4 = sps->log2_max_frame_num_minus4;
	t_psPictureDecode->pic_order_cnt_type = sps->pic_order_cnt_type;
	t_psPictureDecode->log2_max_pic_order_cnt_lsb_minus4 = sps->log2_max_pic_order_cnt_lsb_minus4;
	t_psPictureDecode->delta_pic_order_always_zero_flag = sps->delta_pic_order_always_zero_flag;
	t_psPictureDecode->direct_8x8_inference_flag = sps->direct_8x8_inference_flag;
	t_psPictureDecode->entropy_coding_mode_flag = pps->entropy_coding_mode_flag;
	t_psPictureDecode->pic_order_present_flag = pps->pic_order_present_flag;
	t_psPictureDecode->num_slice_groups_minus1 = pps->num_slice_groups_minus1;
	t_psPictureDecode->slice_group_map_type = pps->slice_group_map_type;
	t_psPictureDecode->deblocking_filter_control_present_flag = pps->deblocking_filter_control_present_flag;
	t_psPictureDecode->redundant_pic_cnt_present_flag = pps->redundant_pic_cnt_present_flag;
	t_psPictureDecode->slice_group_change_rate_minus1 = pps->slice_group_change_rate_minus1;

	memset(t_psPictureDecode->RefFrameList,255,16);
	int dbp_loop_size = min(dpb.used_size_on_view[view_index], active_sps.num_ref_frames);
	StorablePicture *RefFrame;

	if(GetBitstreamRawConfig() == E_BA_RAW_SHORTFORMAT)
	{
		for(i=0, j=0; i<dpb.used_size_on_view[view_index]; i++)
		{
			if(dpb.fs_on_view[view_index][i]->is_reference)
			{
				if(dpb.fs_on_view[view_index][i]->is_reference == 3)
				{
					RefFrame = dpb.fs_on_view[view_index][i]->frame;
					t_psPictureDecode->FieldOrderCntList[j][0] = RefFrame->top_poc;
					t_psPictureDecode->FieldOrderCntList[j][1] = RefFrame->bottom_poc;
				}
				else
				{
					if(dpb.fs_on_view[view_index][i]->is_reference == 1)
					{
						RefFrame = dpb.fs_on_view[view_index][i]->top_field;
						t_psPictureDecode->FieldOrderCntList[j][0] = RefFrame->top_poc;
					}
					else
					{
						RefFrame = dpb.fs_on_view[view_index][i]->bottom_field;
						t_psPictureDecode->FieldOrderCntList[j][1] = RefFrame->bottom_poc;
					}
				}
				t_psPictureDecode->RefFrameList[j].Index7Bits	= RefFrame->pic_store_idx;	//*(byte*)&m_pSurfaceinfo[listX[0][i]->unique_id].SInfo[0];
				t_psPictureDecode->RefFrameList[j].AssociatedFlag = 0;
				t_psPictureDecode->FrameNumList[j] = RefFrame->frame_num;
				t_psPictureDecode->UsedForReferenceFlags |= (dpb.fs_on_view[view_index][i]->is_reference << (j*2));

				j++;
			}
		}
	}
	else
	{
		for(i=0; i<dpb.used_size_on_view[view_index]; i++)
		{
			if(dpb.fs_on_view[view_index][i]->is_reference)
			{
				if(dpb.fs_on_view[view_index][i]->is_reference == 3)
				{
					RefFrame = dpb.fs_on_view[view_index][i]->frame;
					t_psPictureDecode->FieldOrderCntList[RefFrame->pic_store_idx][0] = RefFrame->top_poc;
					t_psPictureDecode->FieldOrderCntList[RefFrame->pic_store_idx][1] = RefFrame->bottom_poc;
				}
				else
				{
					if(dpb.fs_on_view[view_index][i]->is_reference == 1)
					{
						RefFrame = dpb.fs_on_view[view_index][i]->top_field;
						t_psPictureDecode->FieldOrderCntList[RefFrame->pic_store_idx][0] = RefFrame->top_poc;
					}
					else
					{
						RefFrame = dpb.fs_on_view[view_index][i]->bottom_field;
						t_psPictureDecode->FieldOrderCntList[RefFrame->pic_store_idx][1] = RefFrame->bottom_poc;
					}
				}
				t_psPictureDecode->RefFrameList[RefFrame->pic_store_idx].Index7Bits	= RefFrame->pic_store_idx;	//*(byte*)&m_pSurfaceinfo[listX[0][i]->unique_id].SInfo[0];
				t_psPictureDecode->RefFrameList[RefFrame->pic_store_idx].AssociatedFlag = 0;
				t_psPictureDecode->FrameNumList[RefFrame->pic_store_idx] = RefFrame->frame_num;
				t_psPictureDecode->UsedForReferenceFlags |= (dpb.fs_on_view[view_index][i]->is_reference << ((RefFrame->pic_store_idx)*2));
			}
		}
	}

	DXVA_Qmatrix_H264 *t_psQpMatrix = (DXVA_Qmatrix_H264*)IMGPAR m_lpMBLK_Intra_Luma;
	memset(t_psQpMatrix, 0, sizeof(DXVA_Qmatrix_H264));
	for(i = 0; i<6; i++)
		for(j = 0; j<16; j++)
			t_psQpMatrix->bScalingLists4x4[i][j] = qmatrix[i][SNGL_SCAN_2D[j]];
	for(i = 6; i<8; i++)
		for(j = 0; j<64; j++)
			t_psQpMatrix->bScalingLists8x8[i-6][j] = qmatrix[i][SNGL_SCAN8x8_2D[j]];

	SetSurfaceInfo ARGS3(dec_picture->unique_id,IMGPAR currentSlice->structure ==2?1:0,1);

	DXVA_BufferDescription *t_psDxvaBufferDescription = IMGPAR m_pDxvaBufferDescription;
	AMVABUFFERINFO         *t_psBufferInfo            = IMGPAR m_pBufferInfo;
	memset(&t_psDxvaBufferDescription[0], 0, 3*sizeof(DXVA_BufferDescription));
	t_psDxvaBufferDescription[0].dwTypeIndex		= t_psBufferInfo[0].dwTypeIndex	= DXVA2_PictureParametersBufferType;
	t_psDxvaBufferDescription[0].dwBufferIndex		= t_psBufferInfo[0].dwBufferIndex = IMGPAR m_Intra_lCompBufIndex;
	t_psDxvaBufferDescription[0].dwDataOffset		= t_psBufferInfo[0].dwDataOffset = 0;
	t_psDxvaBufferDescription[0].dwDataSize			= t_psBufferInfo[0].dwDataSize = sizeof(DXVA_PicParams_H264);
	t_psDxvaBufferDescription[0].dwNumMBsInBuffer	= 0;

	t_psDxvaBufferDescription[1].dwTypeIndex		= t_psBufferInfo[1].dwTypeIndex	= DXVA2_InverseQuantizationMatrixBufferType;
	t_psDxvaBufferDescription[1].dwBufferIndex		= t_psBufferInfo[1].dwBufferIndex = IMGPAR m_Intra_lCompBufIndex;
	t_psDxvaBufferDescription[1].dwDataOffset		= t_psBufferInfo[1].dwDataOffset = 0;
	t_psDxvaBufferDescription[1].dwDataSize			= t_psBufferInfo[1].dwDataSize = sizeof(DXVA_Qmatrix_H264);
	t_psDxvaBufferDescription[1].dwNumMBsInBuffer	= 0;

	DEBUG_SHOW_HW_INFO("[NV_SECOP]build_picture_decode_buffer_BA_SECOP() END");
	return 0;
}

CREL_RETURN CH264DXVA2::BA_ExecuteBuffers PARGS0()
{
	DEBUG_SHOW_HW_INFO("[CH264DXVA2] BA_ExecuteBuffers()");
	HRESULT	hr = S_OK;
	DWORD m_dwRetValue;

	if (IMGPAR dwSliceCountForSECOP > 0) //NV_SECOP
	{
		DEBUG_SHOW_HW_INFO("[NV_SECOP]BA_ExecuteBuffers() BEGIN dwSliceCount: %d", IMGPAR dwSliceCountForSECOP);
		DXVA_BufferDescription *t_psDxvaBufferDescription = IMGPAR m_pDxvaBufferDescription;
		AMVABUFFERINFO         *t_psBufferInfo            = IMGPAR m_pBufferInfo;

		memset(&t_psDxvaBufferDescription[2], 0, sizeof(DXVA_BufferDescription));
		memset(&t_psBufferInfo[2], 0, sizeof(AMVABUFFERINFO));
		t_psDxvaBufferDescription[2].dwTypeIndex = t_psBufferInfo[2].dwTypeIndex = DXVA2_SliceControlBufferType;
		t_psDxvaBufferDescription[2].dwDataSize  = t_psBufferInfo[2].dwDataSize = IMGPAR dwSliceCountForSECOP * sizeof(SECOP_SliceInfo);
		memcpy(IMGPAR m_lpSLICE, &(IMGPAR sSliceInfoArray[0]), t_psDxvaBufferDescription[2].dwDataSize);

		//for (int i=0; i<IMGPAR dwSliceCountForSECOP; i++)
		//	DP("[SECOP] Slice%2d Handle: %08x Size: %08x", i, IMGPAR sSliceInfoArray[i].dwSliceHandle, IMGPAR sSliceInfoArray[i].dwSliceByteCount);

		while(1)
		{
			hr = BeginFrame(IMGPAR UnCompress_Idx, 0);
			DEBUG_SHOW_HW_INFO_INTEL("This is the %d Frame", IMGPAR m_lFrame_Counter);
			DEBUG_SHOW_HW_INFO_INTEL("BeginFrame() return value = %d", hr);
			if(checkDDError(hr))
			{
				DEBUG_SHOW_HW_INFO_INTEL("DDERR_WASSTILLDRAWING or E_PENDING in BeginFrame(), return value = %d", hr);
				Sleep(2);
			}
			else if (hr == DDERR_SURFACELOST)
			{
				DEBUG_SHOW_HW_INFO_INTEL("DDERR_SURFACELOST in BeginFrame(), return value = %d", hr);
				Sleep(10);
			}
			else
				break;
		}

		for (UINT i=0; i<3; i++)
		{
			if (FAILED(ReleaseBuffer(t_psBufferInfo[i].dwTypeIndex)))
				break;
		}

		if (SUCCEEDED(hr))
		{
			hr |= Execute(0, t_psDxvaBufferDescription, 3*sizeof(DXVA_BufferDescription), &m_dwRetValue, sizeof(DWORD), 3, t_psBufferInfo);
			DEBUG_SHOW_HW_INFO_INTEL("Execute() return value = %d", hr);

			hr |= EndFrame();
			DEBUG_SHOW_HW_INFO_INTEL("EndFrame() return value = %d", hr);
		}

		DEBUG_SHOW_HW_INFO("[NV_SECOP]BA_ExecuteBuffers() END");
		return SUCCEEDED(hr)?0:-1;
	}

	if(!stream_global->m_iStop_Decode)
	{
		if(IMGPAR m_iIntraMCBufUsage != 0)
		{
			DXVA_BufferDescription *t_psDxvaBufferDescription = IMGPAR m_pDxvaBufferDescription;
			AMVABUFFERINFO         *t_psBufferInfo            = IMGPAR m_pBufferInfo;

			t_psDxvaBufferDescription[2].dwTypeIndex		= t_psBufferInfo[2].dwTypeIndex = DXVA2_SliceControlBufferType;
			t_psDxvaBufferDescription[2].dwBufferIndex		= t_psBufferInfo[2].dwBufferIndex = IMGPAR m_Intra_lCompBufIndex;
			t_psDxvaBufferDescription[2].dwDataOffset		= t_psBufferInfo[2].dwDataOffset = 0;
			t_psDxvaBufferDescription[2].dwNumMBsInBuffer	= IMGPAR PicSizeInMbs;
			if(m_bConfigBitstreamRaw==E_BA_RAW_LONGFORMAT)
				t_psDxvaBufferDescription[2].dwDataSize		= t_psBufferInfo[2].dwDataSize = IMGPAR m_slice_number_in_field * sizeof(DXVA_Slice_H264_Long);
			else
				t_psDxvaBufferDescription[2].dwDataSize		= t_psBufferInfo[2].dwDataSize = IMGPAR m_slice_number_in_field * sizeof(DXVA_Slice_H264_Short);

			t_psDxvaBufferDescription[3].dwTypeIndex		= t_psBufferInfo[3].dwTypeIndex = DXVA2_BitStreamDateBufferType;
			t_psDxvaBufferDescription[3].dwBufferIndex		= t_psBufferInfo[3].dwBufferIndex = IMGPAR m_Intra_lCompBufIndex;	
			t_psDxvaBufferDescription[3].dwDataOffset		= t_psBufferInfo[3].dwDataOffset = 0;
			t_psDxvaBufferDescription[3].dwNumMBsInBuffer	= IMGPAR PicSizeInMbs;
			t_psDxvaBufferDescription[3].dwDataSize			= t_psBufferInfo[3].dwDataSize = IMGPAR m_iIntraMCBufUsage;

			DUMP_NVIDIA(IMGPAR m_pnv1PictureDecode, t_psDxvaBufferDescription[0].dwDataSize, IMGPAR m_lFrame_Counter, "PICPAR");
			DUMP_NVIDIA(IMGPAR m_lpMBLK_Intra_Luma, t_psDxvaBufferDescription[1].dwDataSize, IMGPAR m_lFrame_Counter, "IQM");
			DUMP_NVIDIA(IMGPAR m_lpSLICE, t_psDxvaBufferDescription[2].dwDataSize, IMGPAR m_lFrame_Counter, "SLICECTRL");

			if(m_pIviCP != NULL)
			{
				if(m_nVGAType == E_H264_VGACARD_INTEL)
				{
					m_lpBitstreamBuf -= IMGPAR m_iIntraMCBufUsage;
					DUMP_NVIDIA(m_lpBitstreamBuf, t_psDxvaBufferDescription[3].dwDataSize, IMGPAR m_lFrame_Counter, "BITSTREAMDATA");
					if(m_pIviCP->IsScramblingRequired(MAP_CP_PIC_TYPE(IMGPAR firstSlice->picture_type)))
					{
						m_pIviCP->EnableScrambling();							
						m_pIviCP->ScrambleData(IMGPAR m_lpMV, m_lpBitstreamBuf, t_psBufferInfo[3].dwDataSize);						
					}
					else
					{
						m_pIviCP->DisableScrambling();
						memcpy(IMGPAR m_lpMV, m_lpBitstreamBuf, t_psBufferInfo[3].dwDataSize);
					}
				}
				else
				{
					DUMP_NVIDIA(IMGPAR m_lpMV, t_psDxvaBufferDescription[3].dwDataSize, IMGPAR m_lFrame_Counter, "BITSTREAMDATA");
					if(m_pIviCP->IsScramblingRequired(MAP_CP_PIC_TYPE(IMGPAR firstSlice->picture_type)))
					{
						m_pIviCP->EnableScrambling();							
						m_pIviCP->ScrambleData(IMGPAR m_lpMV, IMGPAR m_lpMV, t_psBufferInfo[3].dwDataSize);
					}
					else
					{
						m_pIviCP->DisableScrambling();
					}
				}
			}
			else if (m_nVGAType == E_H264_VGACARD_INTEL)
			{
				m_lpBitstreamBuf -= IMGPAR m_iIntraMCBufUsage;
				DUMP_NVIDIA(m_lpBitstreamBuf, t_psDxvaBufferDescription[3].dwDataSize, IMGPAR m_lFrame_Counter, "BITSTREAMDATA");
				memcpy(IMGPAR m_lpMV, m_lpBitstreamBuf, t_psBufferInfo[3].dwDataSize);
			}
			else
				DUMP_NVIDIA(IMGPAR m_lpMV, t_psDxvaBufferDescription[3].dwDataSize, IMGPAR m_lFrame_Counter, "BITSTREAMDATA");

			while(1)
			{
				hr = BeginFrame(IMGPAR UnCompress_Idx, 0);
				DEBUG_SHOW_HW_INFO("This is the %d Frame", IMGPAR m_lFrame_Counter);
				DEBUG_SHOW_HW_INFO("BeginFrame() return value = %d", hr);

				if (hr == DDERR_SURFACELOST)
				{
					DEBUG_SHOW_HW_INFO("DDERR_SURFACELOST in BeginFrame(), return value = %d", hr);
					Sleep(10);
				}
				else		
					break;		
			}

			for (UINT i=0; i<4; i++)
			{
				if (FAILED(ReleaseBuffer(t_psBufferInfo[i].dwTypeIndex)))
					break;
			}

			if (SUCCEEDED(hr))
			{
				/*if(m_bResidualDataFormat == E_RES_DATA_FORMAT_INTEL)
				{
					if (stream_global->frame_execute_count < 0 && (IMGPAR firstSlice->picture_type == I_SLICE || 
						stream_global->m_iExecute_Of_I_Frame == TRUE))
					{
						if(IMGPAR firstSlice->picture_type == I_SLICE)
							stream_global->m_iExecute_Of_I_Frame = TRUE;
						else
							stream_global->m_iExecute_Of_I_Frame = FALSE;

						WaitForSingleObject(stream_global->hFinishDisplaySemaphore, 0);
						WaitForSingleObject(stream_global->hFinishDisplaySemaphore, 52);
					}
					else if(stream_global->frame_execute_count >= 0)
					{
						stream_global->frame_execute_count--;
					}
				}*/
				if(m_bResidualDataFormat == E_RES_DATA_FORMAT_INTEL)
				{
					if (m_pUncompBufQueue->GetCount() < 2)
					{
						stream_global->m_CheckExecute = TRUE;
					}
					else if (m_pUncompBufQueue->GetCount() > m_pUncompBufQueue->GetMax()>>1)
					{
						stream_global->m_CheckExecute = FALSE;
					}

					if (stream_global->m_CheckExecute)
					{
						LARGE_INTEGER currTime, freq;
						DWORD frame_duration = 0;

						if (g_framerate1000 > 0)
							frame_duration = (DWORD)(1000000 / g_framerate1000);

						if (IMGPAR currentSlice->structure != FRAME)
							frame_duration >>= 1;

						frame_duration = min(frame_duration, 30);
						QueryPerformanceFrequency(&freq);
						QueryPerformanceCounter(&currTime);
						DWORD timediff = (DWORD)((currTime.QuadPart - stream_global->m_liRecodeTime1.QuadPart)*1000/freq.QuadPart);
						
						if (IMGPAR currentSlice->structure != FRAME || timediff > frame_duration)
						{
							WaitForSingleObject(stream_global->hFinishDisplaySemaphore, frame_duration);
						}
						else if (timediff < frame_duration)
						{
							Sleep(frame_duration - timediff);
						}
						QueryPerformanceCounter(&stream_global->m_liRecodeTime1);
					}
					else
					{
						DEBUG_SHOW_HW_INFO("[H264] not in 60 FPS mode");
					}

				}
				else
				{
					stream_global->m_CheckExecute = FALSE;
					DEBUG_SHOW_HW_INFO("[H264] not in 60 FPS mode");
				}
			
				hr |= Execute(0, t_psDxvaBufferDescription, 4*sizeof(DXVA_BufferDescription), &m_dwRetValue, 4, 4, t_psBufferInfo);
				DEBUG_SHOW_HW_INFO("Execute() return value = %d", hr);

				hr |= EndFrame();
				DEBUG_SHOW_HW_INFO("EndFrame() return value = %d", hr);

				//Workaround for some intel platform(GM45) on trick mode, use lock/Unlock to sync d3d surface
				if (m_bResidualDataFormat == E_RES_DATA_FORMAT_INTEL && stream_global->m_bTrickEOS)
				{
					HVDService::HVDDxva2UncompBufLockInfo tInfo;					
					m_pIHVDServiceDxva2->LockUncompressedBuffer(IMGPAR UnCompress_Idx, &tInfo);
					m_pIHVDServiceDxva2->UnlockUncompressedBuffer(IMGPAR UnCompress_Idx);
					DEBUG_SHOW_HW_INFO("Trick Mode in Intel!! Using Lock/Unlock to sync!");
				}
			}

			DEBUG_SHOW_HW_INFO("executed frame:%d", IMGPAR m_lFrame_Counter);
		}
	}

	return SUCCEEDED(hr)?0:-1;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

int CH264DXVA2::GetCompBufferIndex()
{
	while(1)
	{
		if(++m_nLastCompBufIdx == MAX_COMP_BUF_FOR_DXVA2)
			m_nLastCompBufIdx = 0;
		if(m_bCompBufStaus[m_nLastCompBufIdx])
		{
			m_bCompBufStaus[m_nLastCompBufIdx] = FALSE;
			break;
		}
		else
			Sleep(2);
	}

	return m_nLastCompBufIdx;
}

void CH264DXVA2::BeginDecodeFrame PARGS0()
{
	if(!m_pUncompBufQueue)
		return;

	IMGPAR m_lmbCount_Inter = 0;  //for slice number counting

	if(m_bResolutionChange && get_param_fcn != NULL)
	{
		HRESULT hr = S_OK;
#if 1
		HVDService::HVDDecodeConfig DecodeConfig;
		ZeroMemory(&DecodeConfig, sizeof(HVDService::HVDDecodeConfig));
		m_pIHVDService->GetHVDDecodeConfig(&DecodeConfig);
		if (DecodeConfig.dwWidth != IMGPAR width || DecodeConfig.dwHeight != IMGPAR height)
		{
			DecodeConfig.dwWidth = IMGPAR width;
			DecodeConfig.dwHeight = IMGPAR height;

			//Change display resolution and Get new IVICP
			(*get_param_fcn)(H264_PROPID_CB_CHANGE_RESOLUTION, H264_pvDataContext, (LPVOID*)&m_pIviCP, NULL, &DecodeConfig, sizeof(HVDService::HVDDecodeConfig));

			ResetDXVABuffers();
		}
#else
		hr = Send_Resolution(H264_pvDataContext,IMGPAR width,IMGPAR height,(dec_picture->dwXAspect==16 ? 3 : 2), (LPVOID*)&m_pIviCP);
		if(FAILED(hr))
			return;

		ResetDXVABuffers();
#endif

		m_bResolutionChange = FALSE;
	}
	m_nMVBufStride = ((m_nDXVAMode==E_H264_DXVA_ATI_PROPRIETARY_A) ? (m_nCompBufSize[DXVA2_MotionVectorBuffer]>>1)/(Alignment16(IMGPAR height)>>2) : (IMGPAR m_pic_width_in_mbs<<2) * sizeof(DWORD));

	if ((IMGPAR Hybrid_Decoding==0) ||
		((IMGPAR Hybrid_Decoding==1) && (IMGPAR type != I_SLICE)) ||
		((IMGPAR Hybrid_Decoding==2) && (IMGPAR type == B_SLICE)))
	{
		//Skip Bottom Field of B due to SmartDec_3
		if (!((IMGPAR smart_dec & SMART_DEC_SKIP_FIL_B) && IMGPAR currentSlice->picture_type == B_SLICE && IMGPAR currentSlice->structure != FRAME && IMGPAR currentSlice->m_pic_combine_status == 0 && !IMGPAR nal_reference_idc))
		{
			EnterCriticalSection(&m_cs);
			IMGPAR m_Intra_lCompBufIndex = GetCompBufferIndex();
			LeaveCriticalSection(&m_cs);
			//First Field
			if (IMGPAR currentSlice->structure != FRAME && IMGPAR currentSlice->m_pic_combine_status)
				IMGPAR m_iFirstCompBufIndex = IMGPAR m_Intra_lCompBufIndex;

			if (stream_global->m_is_MTMS)
			{
				m_lpPictureParamBuf       = m_pbPictureParamBuf[IMGPAR m_Intra_lCompBufIndex];
				m_lpInverseQuantMatrixBuf = m_pbIQMatrixBuf[IMGPAR m_Intra_lCompBufIndex];
				m_lpSliceCtrlBuf          = m_pbSliceCtrlBuf[IMGPAR m_Intra_lCompBufIndex];
				m_lpMacroblockCtrlBuf     = m_pbMacroblockCtrlBuf[IMGPAR m_Intra_lCompBufIndex];
				m_lpResidualDiffBuf       = m_pbResidualDiffBuf[IMGPAR m_Intra_lCompBufIndex];
				if (IMGPAR currentSlice->picture_type != I_SLICE)
					m_lpMotionVectorBuf     = m_pbMotionVectorBuf[IMGPAR m_Intra_lCompBufIndex];
			}
			else
			{
				IMGPAR m_pnv1PictureDecode   = m_pbPictureParamBuf[IMGPAR m_Intra_lCompBufIndex];
				IMGPAR m_lpQMatrix           = m_pbIQMatrixBuf[IMGPAR m_Intra_lCompBufIndex];
				IMGPAR m_lpSLICE             = m_pbSliceCtrlBuf[IMGPAR m_Intra_lCompBufIndex];
				IMGPAR m_lpMBLK_Intra_Luma   = m_pbMacroblockCtrlBuf[IMGPAR m_Intra_lCompBufIndex];
				if(m_bResidualDataFormat!=E_RES_DATA_FORMAT_NV)
					IMGPAR m_lpRESD_Intra_Luma = m_pbResidualDiffBuf[IMGPAR m_Intra_lCompBufIndex];
				if(IMGPAR currentSlice->picture_type != I_SLICE)
					IMGPAR m_lpMV              = m_pbMotionVectorBuf[IMGPAR m_Intra_lCompBufIndex];
			}			
		}

		IMGPAR m_lmbCount_Inter = 0;
		IMGPAR m_lmbCount_Intra = 0;

		IMGPAR m_iIntraRESBufUsage = 0;
		IMGPAR m_iIntraMCBufUsage = 0;
		IMGPAR m_iInterMCBufUsage = 0;

		IMGPAR m_iInterRESBufUsage_L = 0;
		IMGPAR m_iInterRESBufUsage_C = 0;

		//For ATI
		IMGPAR m_PicHeight = IMGPAR PicHeightInMbs*16;
		IMGPAR m_MVBufSwitch = (IMGPAR m_PicHeight>>2)*(m_nMVBufStride>>2);
		IMGPAR m_SBBufOffset = (IMGPAR m_PicHeight>>4)*(IMGPAR width>>2);
		m_nMBBufStride = (IMGPAR width>>1);
		m_nReBufStride = ((m_nDXVAMode==E_H264_DXVA_ATI_PROPRIETARY_A) ? (m_nCompBufSize[DXVA2_ResidualDifferenceBufferType]*2/3)/Alignment16(IMGPAR height) : (IMGPAR width<<1));
	}
	else
	{
		SurfaceInfo tSurfaceInfo;
		if(IMGPAR structure == FRAME || pic_combine_status)
		{
			IMGPAR UnCompress_Idx = (int)m_pUncompBufQueue->GetItem();

			if (IMGPAR Hybrid_Decoding != 5)
			{
				if (m_nVGAType == E_H264_VGACARD_ATI)	//ATI's DXVA needs to call BeginFrame and EndFrame when we accessing the surface.
				{
					HRESULT hr=E_FAIL;
					while(1)
					{					
						hr = BeginFrame(IMGPAR UnCompress_Idx, 1);
						if(checkDDError(hr))
						{
							m_pUncompBufQueue->PutItem(IMGPAR UnCompress_Idx);
							Sleep(2);
						}
						else
							break;
						IMGPAR UnCompress_Idx = (int)m_pUncompBufQueue->GetItem();
					}
				}
				m_pIHVDServiceDxva2->LockUncompressedBuffer(IMGPAR UnCompress_Idx, (HVDService::HVDDxva2UncompBufLockInfo*)&tSurfaceInfo);
				IMGPAR m_UnCompressedBufferStride = tSurfaceInfo.uPitch;
				IMGPAR m_pUnCompressedBuffer = (BYTE*)tSurfaceInfo.pBits;
			}

#if defined(_USE_QUEUE_FOR_DXVA2_)
			WaitForSingleObject(stream_global->hReadyForRender[IMGPAR UnCompress_Idx], INFINITE);
			ResetEvent(stream_global->hReadyForRender[IMGPAR UnCompress_Idx]);
#endif
		}
	}
}

void CH264DXVA2::EndDecodeFrame PARGS0()
{
	switch(dec_picture->structure)
	{
	case FRAME:
		if (dec_picture->used_for_reference && !dec_picture->frame_mbs_only_flag)
		{
			SetSurfaceInfo ARGS3(dpb.fs_on_view[0][dpb.used_size_on_view[0]-1]->top_field->unique_id,0,0);
			SetSurfaceInfo ARGS3(dpb.fs_on_view[0][dpb.used_size_on_view[0]-1]->bottom_field->unique_id,1,0);
		}
		break;
	case TOP_FIELD:
	case BOTTOM_FIELD:
		if (dpb.fs_on_view[0][dpb.used_size_on_view[0]-1]->is_used==(TOP_FIELD|BOTTOM_FIELD))
			SetSurfaceInfo ARGS3(dpb.fs_on_view[0][dpb.used_size_on_view[0]-1]->frame->unique_id,0,0);
		break;
	}
}

USHORT CH264DXVA2::getPatternCode PARGS1(int luma_transform_size_8x8_flag)
{
	USHORT usLumaPatternCode = 0;
	static const int idx[16] = {15, 14, 11, 10, 13, 12, 9, 8, 7, 6, 3, 2, 5, 4, 1, 0};	

	if(luma_transform_size_8x8_flag)
	{
		for(int b8 = 0; b8 < 4; b8++)
			if(currMB_d->cbp & (1<<b8))
				usLumaPatternCode |= (1<<(3-b8));
	}
	else
	{
		for(int i=0; i<16 ; i++)
		{
			if( currMB_s_d->cbp_blk & (1<<(idx[i])) )
				usLumaPatternCode |= (1<<i);
		}
	}

	return usLumaPatternCode;
}

UCHAR CH264DXVA2::getMBresidDataQuantity PARGS3(unsigned long cbp, int blk_8x8, int luma_transform_size_8x8_flag)
{
	bitset<16> cbp_luma = cbp;
	bitset<8> cbp_chroma = (cbp>>16);
	bitset<4> cbp_8x8 = blk_8x8;
	UCHAR totalBits = 0;

	if(luma_transform_size_8x8_flag)
		totalBits = (cbp_8x8.count()<<3);
	else
		totalBits = (cbp_luma.count()<<1);

	totalBits += (cbp_chroma.count()<<1);

	return totalBits;
}

int CH264DXVA2::getIntraPredAvailFlags PARGS1(int nIntraMode)
{
	UCHAR IntraPredAvail = 0;		
	int mb_nr=IMGPAR current_mb_nr_d;				
	PixelPos up;         //!< pixel position p(0,-1)
	PixelPos left[2];    //!< pixel positions p(-1, -1..15)
	PixelPos left_up;			
	PixelPos right_up;
	int up_avail, left_top_avail, left_bot_avail, left_up_avail, right_up_avail;
	int lsbZero = 0;

	if (nIntraMode == I16MB)
		lsbZero = 2;
	else if (nIntraMode == I8MB)
		lsbZero = 0;
	else if (nIntraMode == I4MB)
		lsbZero = 1;

	//left-up
	getCurrNeighbourD_Luma ARGS3(-1, -1, &left_up);

	getCurrNeighbourA_Luma ARGS3(-1, 0, &left[0]);
	getCurrNeighbourA_Luma ARGS3(-1, 8, &left[1]);

	//up  
	getCurrNeighbourB_Luma ARGS3(0, -1, &up);

	//right-up
	getCurrNeighbourC_Luma ARGS3(16, -1, &right_up);

	if (active_pps.constrained_intra_pred_flag)
	{
		left_top_avail	= (left[0].pMB != NULL);
		left_bot_avail	= (left[1].pMB != NULL);
		up_avail		= (up.pMB != NULL);	
		left_up_avail	= (left_up.pMB != NULL);
		right_up_avail	= (right_up.pMB != NULL);

		if(left_top_avail)
			IntraPredAvail |= 0x10;		// Bit 4
		if(left_bot_avail)
			IntraPredAvail |= 0x08;		// Bit 3
		if(up_avail)
			IntraPredAvail |= 0x04;		// Bit 2
		if(right_up_avail)
			IntraPredAvail |= 0x02;		// Bit 1
		if(left_up_avail)
			IntraPredAvail |= 0x01;		// Bit 0		
	}
	else
	{
		if(left[0].pMB)
			IntraPredAvail |= 0x18;		// Bit 4 and 3
		if(up.pMB)
			IntraPredAvail |= 0x04;		// Bit 2
		if(right_up.pMB)
			IntraPredAvail |= 0x02;		// Bit 1
		if(left_up.pMB)
			IntraPredAvail |= 0x01;		// Bit 0
	}

	if(m_bResidualDataFormat!=E_RES_DATA_FORMAT_INTEL)
	{
		if(lsbZero)
		{
			IntraPredAvail = ((IntraPredAvail>>lsbZero)<<lsbZero);
		}
	}

	return IntraPredAvail;
}

void CH264DXVA2::build_residual_buffer_Inter_Luma PARGS0()
{
	int b8,b4;
	short *pcof = (short*) IMGPAR cof_d;
	int cbp_8x8 = currMB_d->cbp;
	int cbp_blk = currMB_s_d->cbp_blk;
	static const int idx[4] = {0,4,8,12};
	int i_j;

	LPDXVA_MBctrl_H264 Mb_partition_Inter_Luma = (LPDXVA_MBctrl_H264)(IMGPAR m_lpMBLK_Intra_Luma + IMGPAR m_iIntraMCBufUsage);

	if(m_bResidualDataFormat==E_RES_DATA_FORMAT_NV)
	{
		int offset;
		BYTE *RESD;

		int InterRESBufUsage=0;
		RESD = IMGPAR m_lpRESD_Inter_Luma + IMGPAR m_iInterRESBufUsage_L;

		if (!currMB_d->luma_transform_size_8x8_flag)
		{
			for(b8=0 ; b8<4 ; b8++)
			{
				if(1 & (cbp_8x8>>b8))
				{
					for(b4=0 ; b4<4 ; b4++)
					{
						i_j = peano_raster_single[b8][b4];
						if( (cbp_blk>>i_j) & 1 )
							iDCT_4x4_fcn((short*)&RESD[((b4>>1)<<6)+((b4&1)<<3)], pcof+(i_j<<4), 8);
						else
						{
							offset = (((b4>>1)<<6)+((b4&1)<<3));
							memset(&RESD[offset], 0, 8);
							memset(&RESD[offset+16], 0, 8);
							memset(&RESD[offset+32], 0, 8);
							memset(&RESD[offset+48], 0, 8);
						}					
					}
					InterRESBufUsage += 128;
					RESD += 128;
				}
			} 
		}
		else
		{
			for(b8=0 ; b8<4 ; b8++)
			{
				if(1 & (cbp_8x8>>b8))
				{					
					iDCT_8x8_fcn((short*)(RESD), IMGPAR cof_d + (b8<<6),8);
					InterRESBufUsage += 128;
					RESD += 128;
				}
			}
		}

		IMGPAR m_iInterRESBufUsage_L += InterRESBufUsage;
		m_ResInfo[IMGPAR current_slice_nr].m_iInterRESBufUsage_L += InterRESBufUsage;
	}
	else //MS & INTEL
	{
		int i, j, ioff, joff;
		BYTE *m_lpResd = IMGPAR m_lpRESD_Intra_Luma + IMGPAR m_iIntraRESBufUsage;

		if (!currMB_d->luma_transform_size_8x8_flag)
		{
			if (m_nDXVAMode==E_H264_DXVA_MODE_A)
			{
				for(b8=0 ; b8<4 ; b8++)
				{
					USHORT pattern = 0xF000;
					if(Mb_partition_Inter_Luma->wPatternCode[0] & (pattern>>(b8<<2)))
					{
						//if(1 & (cbp_8x8>>b8))
						//{
						for(b4=0 ; b4<4 ; b4++)
						{		
							ioff = idx[(b4&1)+((b8&1)<<1)];	        //0 4 0 4 8 12 8 12 0 4  0  4 8 12 8 12 
							joff = idx[(b4>>1)+((b8>>1)<<1)];		    //0 0 4 4 0  0 4  4 8 8 12 12 8 8 12 12
							i = ioff>>2;
							j = joff>>2;

							if(m_bResidualDataFormat!=E_RES_DATA_FORMAT_INTEL) //MS
							{
								if( currMB_s_d->cbp_blk & (1<<(joff+i)) )
								{
									iDCT_4x4_fcn((short*)m_lpResd, (IMGPAR cof_d + ((j<<6)+(i<<4))), 4);
									m_lpResd += 32;
									IMGPAR m_iIntraRESBufUsage += 32;
								}
							} 
							else //Intel
							{
								if( currMB_s_d->cbp_blk & (1<<(joff+i)) )
								{
									iDCT_4x4_fcn((short*)m_lpResd, (IMGPAR cof_d + ((j<<6)+(i<<4))), 8);
								}
								else
								{
									memset(&m_lpResd[0],0,8);
									memset(&m_lpResd[16],0,8);
									memset(&m_lpResd[32],0,8);
									memset(&m_lpResd[48],0,8);
									Mb_partition_Inter_Luma->bMBresidDataQuantity += 2;
								}
								((b4&1)==1) ? m_lpResd += 56 : m_lpResd += 8;
								IMGPAR m_iIntraRESBufUsage += 32;
							}
						} 
					}
					//For 4x4 transform case if the cbp_8x8 is zero, host doesn't need to send the 8x8 block.
					//else if (m_bResidualDataFormat==E_RES_DATA_FORMAT_INTEL) //INTEL
					//{
					//	memset(&m_lpResd[0], 0, 128);
					//	m_lpResd += 128;
					//	IMGPAR m_iIntraRESBufUsage += 128;
					//} 
				}
			}
			else if (m_nDXVAMode==E_H264_DXVA_MODE_C)
			{
				LPDXVA_TCoefSingle lpCoefResidual = (LPDXVA_TCoefSingle)m_lpResd;
				int z, iLastNonZeroCoefIdx=0;

				for(b8=0 ; b8<4 ; b8++)
				{
					if(1 & (cbp_8x8>>b8))
					{
						for(b4=0 ; b4<4 ; b4++)
						{
							ioff = idx[(b4&1)+((b8&1)<<1)];	        //0 4 0 4 8 12 8 12 0 4  0  4 8 12 8 12 
							joff = idx[(b4>>1)+((b8>>1)<<1)];		    //0 0 4 4 0  0 4  4 8 8 12 12 8 8 12 12
							i = ioff>>2;
							j = joff>>2;

							if( currMB_s_d->cbp_blk & (1<<(joff+i)) )
							{
								int j_temp = 15-((b8<<2)+b4);
								pcof = IMGPAR cof_d + ((j<<6)+(i<<4));
								for (z=0; z<16; z++)
								{
									if ((*pcof)!=0)
									{
										writeDXVA_TCoefSingleIndexWithEOB(lpCoefResidual, z, 0);
										lpCoefResidual->TCoefValue = *pcof;
										lpCoefResidual++;
										IMGPAR m_iIntraRESBufUsage += 4;
										IMGPAR m_data_count_for_Mode_C += 4;
										iLastNonZeroCoefIdx = z;

										Mb_partition_Inter_Luma->wPatternCode[0] |= (1<<j_temp);
									}
									pcof++;
								}
								if(Mb_partition_Inter_Luma->wPatternCode[0] & (1<<j_temp))
								{
									lpCoefResidual--;
									writeDXVA_TCoefSingleIndexWithEOB(lpCoefResidual, iLastNonZeroCoefIdx, 1);
									lpCoefResidual++;
								}
							}
						}
					}
				}
			}
		}
		else
		{
			if (m_nDXVAMode==E_H264_DXVA_MODE_A)
			{
				for(b8=0 ; b8<4 ; b8++)
				{
					if(1 & (cbp_8x8>>b8))
					{
						iDCT_8x8_fcn((short*)m_lpResd, IMGPAR cof_d + (b8<<6),8);
						m_lpResd += 128;
						IMGPAR m_iIntraRESBufUsage += 128;
					}
				}
			}
			else if (m_nDXVAMode==E_H264_DXVA_MODE_C)
			{
				LPDXVA_TCoefSingle lpCoefResidual = (LPDXVA_TCoefSingle)m_lpResd;
				int z, iLastNonZeroCoefIdx=0;

				for(b8=0 ; b8<4 ; b8++)
				{
					if(1 & (cbp_8x8>>b8))
					{
						pcof = IMGPAR cof_d + (b8<<6);
						for (z=0; z<64; z++)
						{
							if ((*pcof)!=0)
							{
								writeDXVA_TCoefSingleIndexWithEOB(lpCoefResidual, z, 0);
								lpCoefResidual->TCoefValue = *pcof;
								lpCoefResidual++;
								IMGPAR m_iIntraRESBufUsage += 4;
								IMGPAR m_data_count_for_Mode_C += 4;
								iLastNonZeroCoefIdx = z;

								Mb_partition_Inter_Luma->wPatternCode[0] |= (1<<(3-b8));
							}
							pcof++;
						}
						if(Mb_partition_Inter_Luma->wPatternCode[0] & (1<<(3-b8)))
						{
							lpCoefResidual--;
							writeDXVA_TCoefSingleIndexWithEOB(lpCoefResidual, iLastNonZeroCoefIdx, 1);
							lpCoefResidual++;
						}
					}
				}
			}
		}
	}
}

void CH264DXVA2::build_residual_buffer_Chroma PARGS0()
{
	LPDXVA_MBctrl_H264 Mb_partition_Chroma = (LPDXVA_MBctrl_H264)(IMGPAR m_lpMBLK_Intra_Luma + IMGPAR m_iIntraMCBufUsage);
	if(m_bResidualDataFormat==E_RES_DATA_FORMAT_NV)
	{
		int uv,b4;
		int cbp_total;
		int flag[2];		
		int offset;
		BYTE *RESD;

		cbp_total= (currMB_s_d->cbp_blk>>16);
		flag[0] = ((cbp_total&0x0F)>0);
		flag[1] = ((cbp_total&0xF0)>0);

		for(uv=0 ; uv<2 ; uv++)   //for u and V
		{
			if(flag[uv])
			{
				RESD = IMGPAR m_lpRESD_Inter_Chroma + IMGPAR m_iInterRESBufUsage_C;
				for(b4=0 ; b4<4 ; b4++)
				{
					if(currMB_s_d->cbp_blk & (1<<cbp_blk_chroma[uv][b4]))
						iDCT_4x4_fcn((short*)&RESD[((b4>>1)<<6)+((b4&1)<<3)], (IMGPAR cof_d + ( ((4+uv)<<6) + (b4<<4) ) ),8);
					else
					{
						offset = (((b4>>1)<<6)+((b4&1)<<3));
						memset(&RESD[offset], 0, 8);
						memset(&RESD[offset+16], 0, 8);
						memset(&RESD[offset+32], 0, 8);
						memset(&RESD[offset+48], 0, 8);	
					}
					cbp_total>>=1;
				}
				IMGPAR m_iInterRESBufUsage_C += 128; 
				m_ResInfo[IMGPAR current_slice_nr].m_iInterRESBufUsage_C += 128;
			}
		}
	}
	else
	{
		int ioff, joff;
		int b8;
		int cbp_total;
		int flag[2];

		BYTE *m_lpResd = IMGPAR m_lpRESD_Intra_Luma + IMGPAR m_iIntraRESBufUsage;

		cbp_total= (currMB_s_d->cbp_blk>>16);
		flag[0] = ((cbp_total&0x0F)>0);
		flag[1] = ((cbp_total&0xF0)>0);

		if (m_nDXVAMode==E_H264_DXVA_MODE_A)
		{
			for(int uv=0;uv<2;uv++)
			{
				if(flag[uv]) // If all of 4 cbp of the 8x8 block of u or v component are equal to zero, host doesn't need to send the 8x8 block.
				{
					for(b8=0;b8<4;b8++)
					{
						ioff = (b8&1)<<2;	//0 4 0 4
						joff = (b8&2)<<1;	//0 0 4 4
						if( currMB_s_d->cbp_blk & (1<<cbp_blk_chroma[uv][b8]))//else means residual = 0
						{
							iDCT_4x4_fcn((short*)m_lpResd, (IMGPAR cof_d + (((4+uv)<<6)+(b8<<4))),8);
							IMGPAR m_iIntraRESBufUsage += 32;
						}
						else if (m_bResidualDataFormat==E_RES_DATA_FORMAT_INTEL) //INTEL
						{
							memset(&m_lpResd[0],0,8);
							memset(&m_lpResd[16],0,8);
							memset(&m_lpResd[32],0,8);
							memset(&m_lpResd[48],0,8);
							Mb_partition_Chroma->bMBresidDataQuantity += 2;
							IMGPAR m_iIntraRESBufUsage += 32;
						}
						((b8&1)==1) ? m_lpResd += 56 : m_lpResd += 8;
					}
				}
			}
		}
		else if (m_nDXVAMode==E_H264_DXVA_MODE_C)
		{
			LPDXVA_TCoefSingle lpCoefResidual = (LPDXVA_TCoefSingle)m_lpResd;
			int z, iLastNonZeroCoefIdx=-1, uv;
			short *pcof = IMGPAR cof_d;

			//fill DC
			for(uv=0;uv<2;uv++)
			{
				for(b8=0;b8<4;b8++)
				{
					ioff = (b8&1)<<2;	//0 4 0 4
					joff = (b8&2)<<1;	//0 0 4 4

					if( currMB_s_d->cbp_blk & (1<<cbp_blk_chroma[uv][b8]))//else means residual = 0
					{
						pcof = IMGPAR cof_d + (((4+uv)<<6)+(b8<<4));
						if ((*pcof)!=0)
						{
							writeDXVA_TCoefSingleIndexWithEOB(lpCoefResidual, b8, 0);
							lpCoefResidual->TCoefValue = *pcof;
							lpCoefResidual++;
							IMGPAR m_iIntraRESBufUsage += 4;
							IMGPAR m_data_count_for_Mode_C += 4;
							iLastNonZeroCoefIdx = b8;
							if(uv == 0)
							{
								if(Mb_partition_Chroma->DcBlockCodedCbFlag == 0)
									Mb_partition_Chroma->DcBlockCodedCbFlag = 1;
							}
							else
							{
								if(Mb_partition_Chroma->DcBlockCodedCrFlag == 0)
									Mb_partition_Chroma->DcBlockCodedCrFlag = 1;
							}
						}
					}
				}
				if((uv == 0 && Mb_partition_Chroma->DcBlockCodedCbFlag == 1) || (uv == 1 && Mb_partition_Chroma->DcBlockCodedCrFlag == 1))
				{
					lpCoefResidual--;
					writeDXVA_TCoefSingleIndexWithEOB(lpCoefResidual, iLastNonZeroCoefIdx, 1);
					lpCoefResidual++;
				}
			}

			//fill AC
			for(uv=0;uv<2;uv++)
			{
				for(b8=0;b8<4;b8++)
				{
					ioff = (b8&1)<<2;	//0 4 0 4
					joff = (b8&2)<<1;	//0 0 4 4

					if( currMB_s_d->cbp_blk & (1<<cbp_blk_chroma[uv][b8]))//else means residual = 0
					{
						pcof = IMGPAR cof_d + (((4+uv)<<6)+(b8<<4));
						pcof++;
						for (z=1; z<16; z++)
						{
							if ((*pcof)!=0)
							{
								writeDXVA_TCoefSingleIndexWithEOB(lpCoefResidual, z, 0);
								lpCoefResidual->TCoefValue = *pcof;
								lpCoefResidual++;
								IMGPAR m_iIntraRESBufUsage += 4;
								IMGPAR m_data_count_for_Mode_C += 4;
								iLastNonZeroCoefIdx = z;

								if(uv == 0)
									Mb_partition_Chroma->wPatternCode[1] |= (1<<(3-b8));
								else
									Mb_partition_Chroma->wPatternCode[2] |= (1<<(3-b8));
							}
							pcof++;
						}
						if((uv == 0 && (Mb_partition_Chroma->wPatternCode[1] & (1<<(3-b8)))) || 
							(uv == 1 && (Mb_partition_Chroma->wPatternCode[2] & (1<<(3-b8)))))
						{
							lpCoefResidual--;
							writeDXVA_TCoefSingleIndexWithEOB(lpCoefResidual, iLastNonZeroCoefIdx, 1);
							lpCoefResidual++;
						}
					}
				}
			}
			//For Intel  Mode C, the unit of bMBresidDataQuantity is changed to 8 byte, bMBresidDataQuantity of padding size must be double.
			Mb_partition_Chroma->bMBresidDataQuantity += 12;
			Mb_partition_Chroma->bMBresidDataQuantity += IMGPAR m_data_count_for_Mode_C >> 3;  //For Intel, change 16 byte unit to 8 byte.
			if(IMGPAR m_data_count_for_Mode_C&0x0007)
				Mb_partition_Chroma->ReservedBit = 1;  //For Intel data quantity calculation.
			//Mb_partition_aaa->bMBresidDataQuantity++;
		}
	}
}

int CH264DXVA2::decode_one_macroblock_Intra PARGS0()
{
	short *pcof = (short*)IMGPAR cof_d;

	int i=0, j=0, i4=0, j4=0;
	int ioff, joff; 
	int b4, b8;
	int i_j;
	int curr_mb_field = ((IMGPAR MbaffFrameFlag)&&(currMB_s_d->mb_field));
	USHORT predmode;
	BYTE *pResDiffBuf;

	short* Residual_Y;
	short* Residual_UV;
	int cbp_blk = currMB_s_d->cbp_blk;
	int offset;
	int slice_start_mb_nr = IMGPAR MbaffFrameFlag ? (IMGPAR currentSlice->start_mb_nr<<1) : IMGPAR currentSlice->start_mb_nr;
	IMGPAR m_data_count_for_Mode_C = 0;

	if(IMGPAR current_mb_nr_d == slice_start_mb_nr)
	{
		if (stream_global->m_is_MTMS)
		{
			IMGPAR m_pnv1PictureDecode = m_lpPictureParamBuf;
			IMGPAR m_lpQMatrix = m_lpInverseQuantMatrixBuf;
			IMGPAR m_lpMBLK_Intra_Luma = m_lpMacroblockCtrlBuf;
			if(m_bResidualDataFormat==E_RES_DATA_FORMAT_NV)
			{
				IMGPAR m_lpRESD_Intra_Luma = m_ResInfo[IMGPAR current_slice_nr].m_lpRESD_Intra_Luma = dec_picture->m_lpRESD_Intra_Luma + slice_start_mb_nr*512;
				IMGPAR m_lpRESD_Intra_Chroma = m_ResInfo[IMGPAR current_slice_nr].m_lpRESD_Intra_Chroma = dec_picture->m_lpRESD_Intra_Chroma + slice_start_mb_nr*256;
				IMGPAR m_lpRESD_Inter_Luma = m_ResInfo[IMGPAR current_slice_nr].m_lpRESD_Inter_Luma = dec_picture->m_lpRESD_Inter_Luma + slice_start_mb_nr*512;
				IMGPAR m_lpRESD_Inter_Chroma = m_ResInfo[IMGPAR current_slice_nr].m_lpRESD_Inter_Chroma = dec_picture->m_lpRESD_Inter_Chroma + slice_start_mb_nr*256;

				m_ResInfo[IMGPAR current_slice_nr].m_lmbCount_Inter = 0;
				m_ResInfo[IMGPAR current_slice_nr].m_lmbCount_Intra = 0;
				m_ResInfo[IMGPAR current_slice_nr].m_iInterRESBufUsage_L = IMGPAR m_iInterRESBufUsage_L = 0;
				m_ResInfo[IMGPAR current_slice_nr].m_iInterRESBufUsage_C = IMGPAR m_iInterRESBufUsage_C = 0;
			}
			else
				IMGPAR m_lpRESD_Intra_Luma = m_lpResidualDiffBuf;

			IMGPAR m_lpSLICE = m_lpSliceCtrlBuf + IMGPAR current_slice_nr * sizeof(DXVA_Slice_H264_Long);
			IMGPAR m_lpMV = m_lpMotionVectorBuf;

			IMGPAR m_lmbCount_Inter = 0;
			IMGPAR m_lmbCount_Intra = 0;

			if(m_bResidualDataFormat==E_RES_DATA_FORMAT_INTEL)
				IMGPAR m_iIntraRESBufUsage =  slice_start_mb_nr*960;
			else
				IMGPAR m_iIntraRESBufUsage =  slice_start_mb_nr*768;
			IMGPAR m_iIntraMCBufUsage = slice_start_mb_nr*sizeof(DXVA_MBctrl_H264);
			IMGPAR m_iInterMCBufUsage = slice_start_mb_nr*sizeof(DXVA_MVvalue)*8;
		}
		else
		{
			if(m_bResidualDataFormat==E_RES_DATA_FORMAT_NV)
			{
				IMGPAR m_lpRESD_Intra_Luma = dec_picture->m_lpRESD_Intra_Luma;
				IMGPAR m_lpRESD_Intra_Chroma = dec_picture->m_lpRESD_Intra_Chroma;
				IMGPAR m_lpRESD_Inter_Luma = dec_picture->m_lpRESD_Inter_Luma;
				IMGPAR m_lpRESD_Inter_Chroma = dec_picture->m_lpRESD_Inter_Chroma;
			}
		}

		build_slice_parameter_buffer ARGS0();
		if (m_bResidualDataFormat!=E_RES_DATA_FORMAT_NV)
			IMGPAR m_lmbCount_Inter++;
	}

	if (m_bResidualDataFormat==E_RES_DATA_FORMAT_INTEL)
	{
		pResDiffBuf = IMGPAR m_lpRESD_Intra_Luma + IMGPAR m_iIntraRESBufUsage;
		memset(pResDiffBuf, 0 ,192);
		pResDiffBuf += 192;
	}
	else
	{
		pResDiffBuf = IMGPAR m_lpRESD_Intra_Luma + IMGPAR m_iIntraRESBufUsage;
	}

	LPDXVA_MBctrl_H264 Mb_partition = (LPDXVA_MBctrl_H264)(IMGPAR m_lpMBLK_Intra_Luma + IMGPAR m_iIntraMCBufUsage);
	memset(Mb_partition, 0, sizeof(DXVA_MBctrl_H264));

	if(m_bResidualDataFormat==E_RES_DATA_FORMAT_NV)
		Residual_Y = (short*)(IMGPAR m_lpRESD_Intra_Luma + IMGPAR m_lmbCount_Intra * 512);

	if (currMB_d->mb_type == I16MB)
	{
		Mb_partition->bSliceID		= IMGPAR current_slice_nr;
		Mb_partition->MbType5Bits	= 1;
		Mb_partition->IntraMbFlag	= 1;
		if(m_bResidualDataFormat==E_RES_DATA_FORMAT_INTEL)
			Mb_partition->mb_field_decoding_flag = IMGPAR MbaffFrameFlag ? curr_mb_field : IMGPAR field_pic_flag;
		else
			Mb_partition->mb_field_decoding_flag = curr_mb_field;// ? IMGPAR MbaffFrameFlag : IMGPAR field_pic_flag;
		Mb_partition->transform_size_8x8_flag = 0;
		Mb_partition->HostResidDiff = (m_nDXVAMode==E_H264_DXVA_MODE_A) ? 1 : 0;
		Mb_partition->CurrMbAddr = IMGPAR current_mb_nr_d;
		Mb_partition->bQpPrime[0] = currMB_s_d->qp;
		Mb_partition->bQpPrime[1] = QP_SCALE_CR_Intel[Clip3(0, 51, currMB_s_d->qp + dec_picture->chroma_qp_offset[0])];
		Mb_partition->bQpPrime[2] = QP_SCALE_CR_Intel[Clip3(0, 51, currMB_s_d->qp + dec_picture->chroma_qp_offset[1])];
		if(m_nDXVAMode==E_H264_DXVA_MODE_A)
		{
			Mb_partition->wPatternCode[0] = getPatternCode ARGS1(currMB_d->luma_transform_size_8x8_flag);
			for(b8 = 0; b8 < 4; b8++)
			{
				if(currMB_s_d->cbp_blk & (1<<(b8+16)))
					Mb_partition->wPatternCode[1] |= (1<<(3-b8));
				if(currMB_s_d->cbp_blk & (1<<(b8+20)))
					Mb_partition->wPatternCode[2] |= (1<<(3-b8));
			}
			Mb_partition->bMBresidDataQuantity = getMBresidDataQuantity ARGS3(currMB_s_d->cbp_blk, currMB_d->cbp, 0);
		}
		Mb_partition->dwMBdataLocation = (IMGPAR m_iIntraRESBufUsage>>2);
		if(m_bResidualDataFormat==E_RES_DATA_FORMAT_INTEL)
		{
			Mb_partition->bMBresidDataQuantity += 12;
			IMGPAR m_iIntraRESBufUsage += 192;
		}
		Mb_partition->LumaIntraPredModes[0] = currMB_d->i16mode;
		Mb_partition->LumaIntraPredModes[1] = Mb_partition->LumaIntraPredModes[2] = Mb_partition->LumaIntraPredModes[3] = 0;
		Mb_partition->intra_chroma_pred_mode = currMB_d->c_ipred_mode;
		Mb_partition->IntraPredAvailFlags = getIntraPredAvailFlags ARGS1(I16MB);

		if(m_bResidualDataFormat==E_RES_DATA_FORMAT_NV)
		{
			for(j4=0; j4<16; j4+=4)
			{		
				for(i4=0; i4<16; i4+=4)
				{						
					if( cbp_blk & 1 )
						iDCT_4x4_fcn(&Residual_Y[(j4<<4)+i4], pcof, 16);
					else
					{						
						offset = (j4<<4)+i4;
						memset(&Residual_Y[offset],0,8);
						memset(&Residual_Y[offset+16],0,8);
						memset(&Residual_Y[offset+32],0,8);
						memset(&Residual_Y[offset+48],0,8);						
					}
					//memset(tmp4, 0, sizeof(short)*16);
					cbp_blk >>= 1;
					pcof += 16;
				}
			}
		}
		else //MS & INTEL
		{
			int idx1[4] = {0,4,8,12};

			if (m_bResidualDataFormat!=E_RES_DATA_FORMAT_INTEL)
			{
				for(b8=0; b8<4 ; b8++)
				{
					for(b4=0; b4<4; b4++)
					{
						ioff = idx1[(b4&1)+((b8&1)<<1)];	        //0 4 0 4 8 12 8 12 0 4  0  4 8 12 8 12 
						joff = idx1[(b4>>1)+((b8>>1)<<1)];		    //0 0 4 4 0  0 4  4 8 8 12 12 8 8 12 12
						i = ioff>>2;
						j = joff>>2;

						if( currMB_s_d->cbp_blk & (1<<(joff+i)) )
						{
							iDCT_4x4_fcn((short*)pResDiffBuf, (IMGPAR cof_d + ((j<<6)+(i<<4))), 4);
							pResDiffBuf += 32;
							IMGPAR m_iIntraRESBufUsage += 32;
						}
					}
				}
			}
			else //INTEL
			{
				if (m_nDXVAMode==E_H264_DXVA_MODE_A)
				{
					for(b8=0; b8<4 ; b8++)
					{
						USHORT pattern = 0xF000;
						if(Mb_partition->wPatternCode[0] & (pattern>>(b8<<2)))
						{
							for(b4=0; b4<4; b4++)
							{
								ioff = idx1[(b4&1)+((b8&1)<<1)];	        //0 4 0 4 8 12 8 12 0 4  0  4 8 12 8 12 
								joff = idx1[(b4>>1)+((b8>>1)<<1)];		    //0 0 4 4 0  0 4  4 8 8 12 12 8 8 12 12
								i = ioff>>2;
								j = joff>>2;

								if( currMB_s_d->cbp_blk & (1<<(joff+i)) )
								{
									iDCT_4x4_fcn((short*)pResDiffBuf, (IMGPAR cof_d + ((j<<6)+(i<<4))), 8); //For Intel, use 8x8 unit
								}
								else
								{
									memset(&pResDiffBuf[0],0,8);
									memset(&pResDiffBuf[16],0,8);
									memset(&pResDiffBuf[32],0,8);
									memset(&pResDiffBuf[48],0,8);
									Mb_partition->bMBresidDataQuantity += 2;  //Padding zero of 4x4 block should be added to quantity 
								}
								((b4&1)==1) ? pResDiffBuf += 56 : pResDiffBuf += 8; //For Intel, use 8x8 unit
								IMGPAR m_iIntraRESBufUsage += 32;
							}
						}
					}
				}
				else if (m_nDXVAMode==E_H264_DXVA_MODE_C)
				{
					LPDXVA_TCoefSingle lpCoefResidual = (LPDXVA_TCoefSingle)pResDiffBuf;
					int z, iLastNonZeroCoefIdx=0;

					//fill DC
					for(b8=0; b8<16; b8++)
					{
						if ((*pcof)!=0)
						{
							writeDXVA_TCoefSingleIndexWithEOB(lpCoefResidual, b8, 0);
							lpCoefResidual->TCoefValue = *pcof;
							lpCoefResidual++;
							IMGPAR m_iIntraRESBufUsage += 4;
							IMGPAR m_data_count_for_Mode_C += 4;
							iLastNonZeroCoefIdx = b8;
							if(Mb_partition->DcBlockCodedYFlag == 0)
								Mb_partition->DcBlockCodedYFlag = 1;
						}
						pcof += 16;
					}

					if(Mb_partition->DcBlockCodedYFlag)
					{
						lpCoefResidual--;
						writeDXVA_TCoefSingleIndexWithEOB(lpCoefResidual, iLastNonZeroCoefIdx, 1);
						lpCoefResidual++;
					}

					//fill AC
					pcof = IMGPAR cof_d;

					for(b8=0; b8<4 ; b8++)
					{
						for(b4=0; b4<4; b4++)
						{
							ioff = idx1[(b4&1)+((b8&1)<<1)];	        //0 4 0 4 8 12 8 12 0 4  0  4 8 12 8 12 
							joff = idx1[(b4>>1)+((b8>>1)<<1)];		    //0 0 4 4 0  0 4  4 8 8 12 12 8 8 12 12
							i = ioff>>2;
							j = joff>>2;

							if (currMB_s_d->cbp_blk & (1<<(joff+i)))
							{
								int j_temp = 15-((b8<<2)+b4);
								pcof = IMGPAR cof_d + ((j<<6)+(i<<4));
								pcof++;

								for (z=1; z<16; z++)
								{
									if ((*pcof)!=0)
									{
										writeDXVA_TCoefSingleIndexWithEOB(lpCoefResidual, z, 0);
										lpCoefResidual->TCoefValue = *pcof;
										lpCoefResidual++;
										IMGPAR m_iIntraRESBufUsage += 4;
										IMGPAR m_data_count_for_Mode_C += 4;
										iLastNonZeroCoefIdx = z;

										Mb_partition->wPatternCode[0] |= (1<<j_temp);
									}
									pcof++;
								}
								if(Mb_partition->wPatternCode[0] & (1<<j_temp))
								{
									lpCoefResidual--;
									writeDXVA_TCoefSingleIndexWithEOB(lpCoefResidual, iLastNonZeroCoefIdx, 1);
									lpCoefResidual++;
								}
							}
						}
					}
				}
			}
		}
	}
	else if(currMB_d->mb_type == I8MB)
	{
		Mb_partition->bSliceID		= IMGPAR current_slice_nr;
		Mb_partition->MbType5Bits	= 0;
		Mb_partition->IntraMbFlag	= 1;
		if(m_bResidualDataFormat==E_RES_DATA_FORMAT_INTEL)
			Mb_partition->mb_field_decoding_flag = IMGPAR MbaffFrameFlag ? curr_mb_field : IMGPAR field_pic_flag;
		else
			Mb_partition->mb_field_decoding_flag = curr_mb_field;// ? IMGPAR MbaffFrameFlag : IMGPAR field_pic_flag;
		Mb_partition->transform_size_8x8_flag = 1;
		Mb_partition->HostResidDiff = (m_nDXVAMode==E_H264_DXVA_MODE_A) ? 1 : 0;
		Mb_partition->CurrMbAddr = IMGPAR current_mb_nr_d;
		Mb_partition->wPatternCode[0] = Mb_partition->wPatternCode[1] = Mb_partition->wPatternCode[2] = 0;
		Mb_partition->bQpPrime[0] = currMB_s_d->qp;
		Mb_partition->bQpPrime[1] = QP_SCALE_CR_Intel[Clip3(0, 51, currMB_s_d->qp + dec_picture->chroma_qp_offset[0])];
		Mb_partition->bQpPrime[2] = QP_SCALE_CR_Intel[Clip3(0, 51, currMB_s_d->qp + dec_picture->chroma_qp_offset[1])];
		if(m_nDXVAMode==E_H264_DXVA_MODE_A)
		{		
			for(b8 = 0; b8 < 4; b8++)
			{
				if(currMB_d->cbp & (1<<b8))
					Mb_partition->wPatternCode[0] |= (1<<(3-b8));
				if(currMB_s_d->cbp_blk & (1<<(b8+16)))
					Mb_partition->wPatternCode[1] |= (1<<(3-b8));
				if(currMB_s_d->cbp_blk & (1<<(b8+20)))
					Mb_partition->wPatternCode[2] |= (1<<(3-b8));
			}
			Mb_partition->bMBresidDataQuantity = getMBresidDataQuantity ARGS3(currMB_s_d->cbp_blk, currMB_d->cbp, 1);
		}
		Mb_partition->dwMBdataLocation = (IMGPAR m_iIntraRESBufUsage>>2);
		if(m_bResidualDataFormat==E_RES_DATA_FORMAT_INTEL)
		{
			Mb_partition->bMBresidDataQuantity += 12;
			IMGPAR m_iIntraRESBufUsage += 192;
		}
		Mb_partition->LumaIntraPredModes[0] = Mb_partition->LumaIntraPredModes[1] = 
			Mb_partition->LumaIntraPredModes[2] = Mb_partition->LumaIntraPredModes[3] = 0;
		Mb_partition->intra_chroma_pred_mode = currMB_d->c_ipred_mode;
		Mb_partition->IntraPredAvailFlags = getIntraPredAvailFlags ARGS1(I8MB);

		for(b8=0; b8<4; b8++)
		{
			ioff = (b8&1)<<3;	//0 8 0 8
			joff = (b8>>1)<<3;	//0 0 8 8
			predmode = currMB_d->ipredmode[(((IMGPAR mb_y_d)*4 + 2*(b8/2))&3)*4+(((IMGPAR mb_x_d)*4 + 2*(b8%2))&3)];
			Mb_partition->LumaIntraPredModes[0] |= (predmode<<(b8<<2));

			if(m_bResidualDataFormat==E_RES_DATA_FORMAT_NV)
			{
				if (currMB_d->cbp&(1<<b8))
					iDCT_8x8_fcn(&Residual_Y[(joff<<4)+ioff], IMGPAR cof_d + (b8<<6),16);
				else
				{
					offset = (joff<<4)+ioff;
					memset(&Residual_Y[offset], 0, 16);
					memset(&Residual_Y[offset+16], 0, 16);
					memset(&Residual_Y[offset+32], 0, 16);
					memset(&Residual_Y[offset+48], 0, 16);
					memset(&Residual_Y[offset+64], 0, 16);
					memset(&Residual_Y[offset+80], 0, 16);
					memset(&Residual_Y[offset+96], 0, 16);
					memset(&Residual_Y[offset+112], 0, 16);					
				}			
			}
			else //MS & INTEL
			{
				if (m_nDXVAMode==E_H264_DXVA_MODE_A)
				{
					if (currMB_d->cbp&(1<<b8))
					{
						iDCT_8x8_fcn((short*)pResDiffBuf, IMGPAR cof_d + (b8<<6),8);
						pResDiffBuf += 128;
						IMGPAR m_iIntraRESBufUsage += 128;
					}		
				}
				else if (m_nDXVAMode==E_H264_DXVA_MODE_C)
				{
					LPDXVA_TCoefSingle lpCoefResidual = (LPDXVA_TCoefSingle)pResDiffBuf;
					int z, iLastNonZeroCoefIdx=0;

					if (currMB_d->cbp&(1<<b8))
					{
						pcof = IMGPAR cof_d + (b8<<6);

						for (z=0; z<64; z++)
						{
							if ((*pcof)!=0)
							{
								writeDXVA_TCoefSingleIndexWithEOB(lpCoefResidual, z, 0);
								lpCoefResidual->TCoefValue = *pcof;
								lpCoefResidual++;
								IMGPAR m_iIntraRESBufUsage += 4;
								IMGPAR m_data_count_for_Mode_C += 4;
								iLastNonZeroCoefIdx = z;
								pResDiffBuf += 4;

								Mb_partition->wPatternCode[0] |= (1<<(3-b8));
							}
							pcof++;
						}
						if(Mb_partition->wPatternCode[0] & (1<<(3-b8)))
						{
							lpCoefResidual--;
							writeDXVA_TCoefSingleIndexWithEOB(lpCoefResidual, iLastNonZeroCoefIdx, 1);
							lpCoefResidual++;
						}
					}
				}
			}
		}
	}
	else if (currMB_d->mb_type == I4MB)
	{
		Mb_partition->bSliceID		= IMGPAR current_slice_nr;
		Mb_partition->MbType5Bits	= 0;
		Mb_partition->IntraMbFlag	= 1;
		if(m_bResidualDataFormat==E_RES_DATA_FORMAT_INTEL)
			Mb_partition->mb_field_decoding_flag	= IMGPAR MbaffFrameFlag ? curr_mb_field : IMGPAR field_pic_flag;
		else
			Mb_partition->mb_field_decoding_flag = curr_mb_field;// ? IMGPAR MbaffFrameFlag : IMGPAR field_pic_flag;
		Mb_partition->transform_size_8x8_flag = 0;
		Mb_partition->HostResidDiff = (m_nDXVAMode==E_H264_DXVA_MODE_A) ? 1 : 0;
		Mb_partition->CurrMbAddr = IMGPAR current_mb_nr_d;
		Mb_partition->wPatternCode[0] = Mb_partition->wPatternCode[1] = Mb_partition->wPatternCode[2] = 0;
		Mb_partition->bQpPrime[0] = currMB_s_d->qp;
		Mb_partition->bQpPrime[1] = QP_SCALE_CR_Intel[Clip3(0, 51, currMB_s_d->qp + dec_picture->chroma_qp_offset[0])];
		Mb_partition->bQpPrime[2] = QP_SCALE_CR_Intel[Clip3(0, 51, currMB_s_d->qp + dec_picture->chroma_qp_offset[1])];

		if(m_nDXVAMode==E_H264_DXVA_MODE_A)
		{
			for(b8 = 0; b8 < 4; b8++)
			{
				if(currMB_s_d->cbp_blk & (1<<(b8+16)))
					Mb_partition->wPatternCode[1] |= (1<<(3-b8));
				if(currMB_s_d->cbp_blk & (1<<(b8+20)))
					Mb_partition->wPatternCode[2] |= (1<<(3-b8));
			}
			Mb_partition->bMBresidDataQuantity = getMBresidDataQuantity ARGS3(currMB_s_d->cbp_blk, currMB_d->cbp, 0);
		}
		Mb_partition->dwMBdataLocation = (IMGPAR m_iIntraRESBufUsage>>2);

		if(m_bResidualDataFormat==E_RES_DATA_FORMAT_INTEL)
		{
			//For Intel Mode A, move wPatternCode[0] assignment from loop to this block.
			if(m_nDXVAMode==E_H264_DXVA_MODE_A)
				Mb_partition->wPatternCode[0] = getPatternCode ARGS1(currMB_d->luma_transform_size_8x8_flag);
			Mb_partition->bMBresidDataQuantity += 12;
			IMGPAR m_iIntraRESBufUsage += 192;
		}
		Mb_partition->LumaIntraPredModes[0] = Mb_partition->LumaIntraPredModes[1] = 
			Mb_partition->LumaIntraPredModes[2] = Mb_partition->LumaIntraPredModes[3] = 0;
		Mb_partition->intra_chroma_pred_mode = currMB_d->c_ipred_mode;
		Mb_partition->IntraPredAvailFlags = getIntraPredAvailFlags ARGS1(I4MB);
		int idx1[4] = {0,4,8,12};

		if(m_bResidualDataFormat==E_RES_DATA_FORMAT_NV)
		{
			for(b8=0; b8<4 ; b8++)
			{
				for(b4=0; b4<4; b4++)
				{
					i_j = peano_raster_single[b8][b4];
					ioff = idx1[(b4&1)+((b8&1)<<1)];	        //0 4 0 4 8 12 8 12 0 4  0  4 8 12 8 12 
					joff = idx1[(b4>>1)+((b8>>1)<<1)];		    //0 0 4 4 0  0 4  4 8 8 12 12 8 8 12 12			
					predmode = currMB_d->ipredmode[i_j];
					Mb_partition->LumaIntraPredModes[b8] |= (predmode<<(b4<<2));					

					if( cbp_blk & (1<<i_j) )
					{
						Mb_partition->wPatternCode[0] |= (0x01<<(15-b8*4-b4));
						iDCT_4x4_fcn(&Residual_Y[(joff<<4)+ioff], (IMGPAR cof_d + (i_j<<4)), 16);
					}
					else
					{
						offset = (joff<<4)+ioff;
						memset(&Residual_Y[offset], 0, 8);
						memset(&Residual_Y[offset+16], 0, 8);
						memset(&Residual_Y[offset+32], 0, 8);
						memset(&Residual_Y[offset+48], 0, 8);
					}										
				}
			}
		}
		else //MS & INTEL
		{
			if (m_bResidualDataFormat!=E_RES_DATA_FORMAT_INTEL)
			{
				for(b8=0; b8<4 ; b8++)
				{
					for(b4=0; b4<4; b4++)
					{
						i_j = peano_raster_single[b8][b4];
						ioff = idx1[(b4&1)+((b8&1)<<1)];	        //0 4 0 4 8 12 8 12 0 4  0  4 8 12 8 12 
						joff = idx1[(b4>>1)+((b8>>1)<<1)];		    //0 0 4 4 0  0 4  4 8 8 12 12 8 8 12 12			
						predmode = currMB_d->ipredmode[i_j];
						Mb_partition->LumaIntraPredModes[b8] |= (predmode<<(b4<<2));

						if( cbp_blk & (1<<i_j) )
						{
							Mb_partition->wPatternCode[0] |= (0x01<<(15-b8*4-b4));
							iDCT_4x4_fcn((short*)pResDiffBuf, (IMGPAR cof_d + (i_j<<4)), 4);
							pResDiffBuf += 32;
							IMGPAR m_iIntraRESBufUsage += 32;
						}				
					}
				}
			}
			else //INTEL
			{
				if (m_nDXVAMode==E_H264_DXVA_MODE_A)
				{
					for(b8=0; b8<4 ; b8++)
					{
						USHORT pattern = 0xF000;
						if(Mb_partition->wPatternCode[0] & (pattern>>(b8<<2)))
						{
							for(b4=0; b4<4; b4++)
							{
								i_j = peano_raster_single[b8][b4];
								ioff = idx1[(b4&1)+((b8&1)<<1)];	        //0 4 0 4 8 12 8 12 0 4  0  4 8 12 8 12 
								joff = idx1[(b4>>1)+((b8>>1)<<1)];		    //0 0 4 4 0  0 4  4 8 8 12 12 8 8 12 12			
								predmode = currMB_d->ipredmode[i_j];
								Mb_partition->LumaIntraPredModes[b8] |= (predmode<<(b4<<2));

								if( cbp_blk & (1<<i_j) )
								{
									//Mb_partition->wPatternCode[0] |= (0x01<<(15-b8*4-b4));
									iDCT_4x4_fcn((short*)pResDiffBuf, (IMGPAR cof_d + (i_j<<4)), 8);
								}
								else
								{
									memset(&pResDiffBuf[0],0,8);
									memset(&pResDiffBuf[16],0,8);
									memset(&pResDiffBuf[32],0,8);
									memset(&pResDiffBuf[48],0,8);
									Mb_partition->bMBresidDataQuantity += 2;  //Padding zero of 4x4 block should be added to quantity 
								}
								((b4&1)==1) ? pResDiffBuf += 56 : pResDiffBuf += 8;
								IMGPAR m_iIntraRESBufUsage += 32;
							}
						}
						else
						{
							for(b4=0; b4<4; b4++)
							{
								i_j = peano_raster_single[b8][b4];
								predmode = currMB_d->ipredmode[i_j];
								Mb_partition->LumaIntraPredModes[b8] |= (predmode<<(b4<<2));
							}
						}
					}
				}
				else if (m_nDXVAMode==E_H264_DXVA_MODE_C)
				{
					LPDXVA_TCoefSingle lpCoefResidual = (LPDXVA_TCoefSingle)pResDiffBuf;
					int z, iLastNonZeroCoefIdx=0;

					for(b8=0; b8<4 ; b8++)
					{
						for(b4=0; b4<4; b4++)
						{
							i_j = peano_raster_single[b8][b4];
							ioff = idx1[(b4&1)+((b8&1)<<1)];	        //0 4 0 4 8 12 8 12 0 4  0  4 8 12 8 12
							joff = idx1[(b4>>1)+((b8>>1)<<1)];		    //0 0 4 4 0  0 4  4 8 8 12 12 8 8 12 12
							predmode = currMB_d->ipredmode[i_j];
							Mb_partition->LumaIntraPredModes[b8] |= (predmode<<(b4<<2));

							if( cbp_blk & (1<<i_j) )
							{
								int j_temp = 15-((b8<<2)+b4);
								//Mb_partition->wPatternCode[0] |= (0x01<<(15-b8*4-b4));
								pcof = IMGPAR cof_d + (i_j<<4);

								for (z=0; z<16; z++)
								{
									if ((*pcof)!=0)
									{
										writeDXVA_TCoefSingleIndexWithEOB(lpCoefResidual, z, 0);
										lpCoefResidual->TCoefValue = *pcof;
										lpCoefResidual++;
										IMGPAR m_iIntraRESBufUsage += 4;
										IMGPAR m_data_count_for_Mode_C += 4;
										iLastNonZeroCoefIdx = z;

										Mb_partition->wPatternCode[0] |= (1<<j_temp);
									}
									pcof++;
								}
								if(Mb_partition->wPatternCode[0] & (1<<j_temp))
								{
									lpCoefResidual--;
									writeDXVA_TCoefSingleIndexWithEOB(lpCoefResidual, iLastNonZeroCoefIdx, 1);
									lpCoefResidual++;
								}
							}
						}
					}
				}
			}
		}	
	}
	else if(currMB_d->mb_type == IPCM)
	{
		Mb_partition->bSliceID		= IMGPAR current_slice_nr;
		Mb_partition->MbType5Bits	= 25;
		Mb_partition->IntraMbFlag	= 1;
		if(m_bResidualDataFormat==E_RES_DATA_FORMAT_INTEL)
			Mb_partition->mb_field_decoding_flag	= IMGPAR MbaffFrameFlag ? curr_mb_field : IMGPAR field_pic_flag;
		else
			Mb_partition->mb_field_decoding_flag = curr_mb_field;// ? IMGPAR MbaffFrameFlag : IMGPAR field_pic_flag;
		Mb_partition->transform_size_8x8_flag = 0;
		Mb_partition->HostResidDiff = (m_nDXVAMode==E_H264_DXVA_MODE_A) ? 1 : 0;
		Mb_partition->CurrMbAddr = IMGPAR current_mb_nr_d;
		memset(&Mb_partition->wPatternCode[0],0xFF,6);		
		Mb_partition->bQpPrime[0] = currMB_s_d->qp;  //Should be removed.
		Mb_partition->bQpPrime[1] = QP_SCALE_CR_Intel[Clip3(0, 51, currMB_s_d->qp + dec_picture->chroma_qp_offset[0])];  //Should be removed.
		Mb_partition->bQpPrime[2] = QP_SCALE_CR_Intel[Clip3(0, 51, currMB_s_d->qp + dec_picture->chroma_qp_offset[1])];  //Should be removed.
		if(m_bResidualDataFormat!=E_RES_DATA_FORMAT_INTEL)
			Mb_partition->bMBresidDataQuantity = 48;
		Mb_partition->dwMBdataLocation = (IMGPAR m_iIntraRESBufUsage>>2);
		//Mb_partition->LumaIntraPredModes[0] = currMB_d->i16mode;
		//Mb_partition->LumaIntraPredModes[1] = Mb_partition->LumaIntraPredModes[2] = Mb_partition->LumaIntraPredModes[3] = 0;
		//Mb_partition->intra_chroma_pred_mode = currMB_d->c_ipred_mode;
		//Mb_partition->IntraPredAvailFlags = I_Pred_luma_16x16 ARGS1(2);
		if(m_bResidualDataFormat==E_RES_DATA_FORMAT_INTEL)
		{
			if(m_nDXVAMode==E_H264_DXVA_MODE_A)
				Mb_partition->bMBresidDataQuantity = 36; //12(192 padding)+24(data), 16 bytes unit for Intel
			else
				Mb_partition->bMBresidDataQuantity = 72; //Mode C, 24(192 padding)+48(data), 8 bytes unit for Intel

			IMGPAR m_iIntraRESBufUsage += 192;
		}

		//if (m_iDXVAMode==E_H264_DXVA_MODE_A || m_bResidualDataFormat==E_RES_DATA_FORMAT_INTEL) //Both A&C of Intel use this path.
		{
		//for luma residual
//#if  !defined(H264_ENABLE_INTRINSICS)
		if((cpu_type == CPU_LEVEL_SSE2) || (cpu_type == CPU_LEVEL_SSE3))
		{
			//for luma residual
			if(m_bResidualDataFormat==E_RES_DATA_FORMAT_NV)
			{
				__m128i xmm0, xmm1, xmm2, xmm3, xmm5, xmm6;
				short *dest = Residual_Y;

				for(i=0 ; i<8 ; i++)
				{
					xmm0 = *((__m128i*)&pcof[i<<4]);
					xmm1 = *((__m128i*)&pcof[(i<<4)+8]);

					xmm2 = xmm3 = xmm5 = xmm6 = _mm_setzero_si128();

					xmm2 = _mm_unpacklo_epi8(xmm0, xmm2);
					xmm3 = _mm_unpackhi_epi8(xmm0, xmm3);
					xmm5 = _mm_unpacklo_epi8(xmm1, xmm5);
					xmm6 = _mm_unpackhi_epi8(xmm1, xmm6);

					*((__m128i*)dest) = xmm2;
					*((__m128i*)(dest+8)) = xmm3;
					*((__m128i*)(dest+16)) = xmm5;
					*((__m128i*)(dest+24)) = xmm6;

					dest += 32;
				}
			}
			else //MS & Intel
			{
				memcpy(pResDiffBuf, pcof, 256);
			}
		}
		else
		{
			for(i=0 ; i<128 ; i++)
			{
				*(Residual_Y+i*2) = ((*(pcof+i))&0xFF);
				*(Residual_Y+i*2+1) = ((*(unsigned short*)(pcof+i))>>8);
			}	
		}
//#endif

		memset((unsigned char *) IMGPAR cof_d, 0, 4*BLOCK_SIZE*BLOCK_SIZE*BLOCK_SIZE*sizeof(short));

		//for chroma residual
		pcof = (IMGPAR cof_d+256);

//#if !defined(H264_ENABLE_INTRINSICS)
		if(cpu_type == CPU_LEVEL_MMX || CPU_LEVEL_SSE || CPU_LEVEL_SSE2 || CPU_LEVEL_SSE3)
		{
			if(m_bResidualDataFormat==E_RES_DATA_FORMAT_NV)
			{
				__m128i xmm0, xmm1, xmm2, xmm3, xmm5, xmm7;
				short *dest, *pcof_v;

				Residual_UV = (short*)(IMGPAR m_lpRESD_Intra_Chroma + IMGPAR m_lmbCount_Intra * 256);
				dest = Residual_UV;
				pcof_v = pcof+64;

				for(i=0 ; i<8 ; i++)
				{
					xmm0 = *((__m128i*)&pcof[i<<3]);
					xmm1 = *((__m128i*)&pcof_v[i<<3]);

					xmm2 = xmm3 = xmm5 = xmm7 = _mm_setzero_si128();

					xmm2 = _mm_unpacklo_epi8(xmm0, xmm2);
					xmm3 = _mm_unpackhi_epi8(xmm0, xmm3);
					xmm5 = _mm_unpacklo_epi8(xmm1, xmm5);
					xmm7 = _mm_unpackhi_epi8(xmm1, xmm7);

					xmm0 = _mm_unpacklo_epi16(xmm2, xmm5);
					xmm1 = _mm_unpackhi_epi16(xmm2, xmm5);
					xmm2 = _mm_unpacklo_epi16(xmm3, xmm7);
					xmm5 = _mm_unpackhi_epi16(xmm3, xmm7);

					*((__m128i*)dest) = xmm0;
					*((__m128i*)(dest+8)) = xmm1;
					*((__m128i*)(dest+16)) = xmm2;
					*((__m128i*)(dest+24)) = xmm5;

					dest += 32;
				}
			}
			else //MS & Intel
			{
				IMGPAR m_iIntraRESBufUsage += 384;
				pResDiffBuf+=256;
				memcpy(pResDiffBuf, pcof, 64);  //u
				pcof += 64;
				pResDiffBuf += 64;
				memcpy(pResDiffBuf, pcof, 64);  //v
			}
		}
		else
		{
			for(i=0 ; i<64 ; i++)
			{	
			//u
				*(Residual_UV+i*4) = ((*(pcof+i))&0xFF);
				*(Residual_UV+i*4+2) = ((*(unsigned short*)(pcof+i))>>8);

			//v
				*(Residual_UV+i*4+1) = ((*(pcof+64+i))&0xFF);
				*(Residual_UV+i*4+3) = ((*(unsigned short*)(pcof+64+i))>>8);
			}
		}
//#endif
		memset((unsigned char *) (IMGPAR cof_d+256), 0, 2*BLOCK_SIZE*BLOCK_SIZE*BLOCK_SIZE*sizeof(short));
		}

		//For CABAC decoding of Dquant
		last_dquant=0;
	}

	if(m_bResidualDataFormat==E_RES_DATA_FORMAT_NV)
		Residual_UV = (short*)(IMGPAR m_lpRESD_Intra_Chroma + IMGPAR m_lmbCount_Intra * 256);

#ifdef __SUPPORT_YUV400__
	if (dec_picture->chroma_format_idc != YUV400)
	{
#endif
		if(currMB_d->mb_type != IPCM)
		{
			if(m_bResidualDataFormat==E_RES_DATA_FORMAT_NV)
			{
				for(b8=0;b8<4;b8++)
				{
					ioff = (b8&1)<<3;	//0 4 0 4
					joff = (b8&2)<<5;	//0 0 4 4

					iDCT_4x4_UV_fcn(&Residual_UV[(joff+ioff)], (IMGPAR cof_d + ((4<<6)+(b8<<4))),
						( currMB_s_d->cbp_blk & (1<<cbp_blk_chroma[0][b8])),
						( currMB_s_d->cbp_blk & (1<<cbp_blk_chroma[1][b8])), 16);
				}
			}
			else
				build_residual_buffer_Chroma ARGS0();
		}
#ifdef __SUPPORT_YUV400__
	}
#endif

	IMGPAR m_iIntraMCBufUsage += sizeof(DXVA_MBctrl_H264);

	if(m_bResidualDataFormat==E_RES_DATA_FORMAT_NV)
	{
		IMGPAR m_lmbCount_Intra++;
		m_ResInfo[IMGPAR current_slice_nr].m_lmbCount_Intra++;
	}

	return 0;
}

int CH264DXVA2::decode_one_macroblock_Inter PARGS0()
{
	int fw_refframe=-1, bw_refframe=-1;
	int mb_nr = IMGPAR current_mb_nr_d;
	int list_offset;
	int vec_x_base, vec_y_base;
	int curr_mb_field = ((IMGPAR MbaffFrameFlag)&&(currMB_s_d->mb_field));
	int slice_start_mb_nr = IMGPAR MbaffFrameFlag ? (IMGPAR currentSlice->start_mb_nr<<1) : IMGPAR currentSlice->start_mb_nr;
	IMGPAR m_data_count_for_Mode_C = 0;

	if(IMGPAR current_mb_nr_d == slice_start_mb_nr)
	{
		if (stream_global->m_is_MTMS)
		{
			IMGPAR m_pnv1PictureDecode = m_lpPictureParamBuf;
			IMGPAR m_lpQMatrix = m_lpInverseQuantMatrixBuf;
			IMGPAR m_lpMBLK_Intra_Luma = m_lpMacroblockCtrlBuf;

			if(m_bResidualDataFormat==E_RES_DATA_FORMAT_NV)
			{
				IMGPAR m_lpRESD_Intra_Luma = m_ResInfo[IMGPAR current_slice_nr].m_lpRESD_Intra_Luma = dec_picture->m_lpRESD_Intra_Luma + mb_nr*512;
				IMGPAR m_lpRESD_Intra_Chroma = m_ResInfo[IMGPAR current_slice_nr].m_lpRESD_Intra_Chroma = dec_picture->m_lpRESD_Intra_Chroma + mb_nr*256;
				IMGPAR m_lpRESD_Inter_Luma = m_ResInfo[IMGPAR current_slice_nr].m_lpRESD_Inter_Luma = dec_picture->m_lpRESD_Inter_Luma + mb_nr*512;
				IMGPAR m_lpRESD_Inter_Chroma = m_ResInfo[IMGPAR current_slice_nr].m_lpRESD_Inter_Chroma = dec_picture->m_lpRESD_Inter_Chroma + mb_nr*256;

				m_ResInfo[IMGPAR current_slice_nr].m_lmbCount_Inter = 0;
				m_ResInfo[IMGPAR current_slice_nr].m_lmbCount_Intra = 0;
				m_ResInfo[IMGPAR current_slice_nr].m_iInterRESBufUsage_L = IMGPAR m_iInterRESBufUsage_L = 0;
				m_ResInfo[IMGPAR current_slice_nr].m_iInterRESBufUsage_C = IMGPAR m_iInterRESBufUsage_C = 0;
			}
			else
				IMGPAR m_lpRESD_Intra_Luma = m_lpResidualDiffBuf;

			IMGPAR m_lpSLICE = m_lpSliceCtrlBuf + IMGPAR current_slice_nr * sizeof(DXVA_Slice_H264_Long);
			IMGPAR m_lpMV = m_lpMotionVectorBuf;

			IMGPAR m_lmbCount_Inter = 0;
			IMGPAR m_lmbCount_Intra = 0;
			
			if (m_bResidualDataFormat==E_RES_DATA_FORMAT_INTEL)
				IMGPAR m_iIntraRESBufUsage =  slice_start_mb_nr*960;
			else
				IMGPAR m_iIntraRESBufUsage =  slice_start_mb_nr*768;
			IMGPAR m_iIntraMCBufUsage = slice_start_mb_nr*sizeof(DXVA_MBctrl_H264);
			IMGPAR m_iInterMCBufUsage = slice_start_mb_nr*sizeof(DXVA_MVvalue)*8;
		}
		else
		{
			if(m_bResidualDataFormat==E_RES_DATA_FORMAT_NV)
			{
				IMGPAR m_lpRESD_Intra_Luma = dec_picture->m_lpRESD_Intra_Luma;
				IMGPAR m_lpRESD_Intra_Chroma = dec_picture->m_lpRESD_Intra_Chroma;
				IMGPAR m_lpRESD_Inter_Luma = dec_picture->m_lpRESD_Inter_Luma;
				IMGPAR m_lpRESD_Inter_Chroma = dec_picture->m_lpRESD_Inter_Chroma;
			}
		}

		build_slice_parameter_buffer ARGS0();
		if(m_bResidualDataFormat!=E_RES_DATA_FORMAT_NV)
			IMGPAR m_lmbCount_Inter++;
	}

	// Base position for 0-motion vectors
	vec_x_base = IMGPAR pix_x_d;
	vec_y_base = IMGPAR pix_y_d;
	// find out the correct list offsets
	if (curr_mb_field)
	{
		vec_y_base >>= 1;
		if(mb_nr & 1)
		{
			list_offset = 4; // top field mb		  
			vec_y_base -= 8;
		}
		else
		{
			list_offset = 2; // bottom field mb
		}
		clip_max_y    = (dec_picture->size_y>>1)-1;
		clip_max_y_cr = (dec_picture->size_y_cr>>1)-1;
	}
	else
	{
		list_offset = 0;  // no mb aff or frame mb
		clip_max_y    = dec_picture->size_y-1;
		clip_max_y_cr = dec_picture->size_y_cr-1;
	}
	clip_max_x    = dec_picture->size_x+1;
	clip_max_x_cr = dec_picture->size_x_cr-1;	

	calc_chroma_vector_adjustment ARGS2(list_offset, curr_mb_field);

	if(!currMB_d->mb_type)
	{
		if(IMGPAR type==B_SLICE)
			build_macroblock_buffer_Inter_8x8 ARGS1(list_offset);
		else
			build_macroblock_buffer_Inter_16x16 ARGS1(list_offset);
	}
	else if(currMB_d->mb_type==PB_16x16)  
		build_macroblock_buffer_Inter_16x16 ARGS1(list_offset);
	else if(currMB_d->mb_type==PB_16x8) 
		build_macroblock_buffer_Inter_16x8 ARGS1(list_offset);
	else if(currMB_d->mb_type==PB_8x16) 
		build_macroblock_buffer_Inter_8x16 ARGS1(list_offset);
	else  
		build_macroblock_buffer_Inter_8x8 ARGS1(list_offset);

	if (m_bResidualDataFormat==E_RES_DATA_FORMAT_INTEL)
		IMGPAR m_iIntraRESBufUsage += 192;

	//IMGPAR m_iIntraMCBufUsage += sizeof(DXVA_MBctrl_H264);

	build_residual_buffer_Inter_Luma ARGS0();

#ifdef __SUPPORT_YUV400__
	if (dec_picture->chroma_format_idc != YUV400)
	{
#endif
		build_residual_buffer_Chroma ARGS0();
#ifdef __SUPPORT_YUV400__
	}
#endif

	IMGPAR m_iIntraMCBufUsage += sizeof(DXVA_MBctrl_H264);

	if(m_bResidualDataFormat==E_RES_DATA_FORMAT_NV)
	{
		IMGPAR m_lmbCount_Inter++;
		m_ResInfo[IMGPAR current_slice_nr].m_lmbCount_Inter++;
	}

	return 0;
}

int CH264DXVA2::build_picture_decode_buffer PARGS0()
{
	int i;
	pic_parameter_set_rbsp_t *pps = &active_pps;
	seq_parameter_set_rbsp_t *sps = &active_sps;

	if(active_sps.Valid)
		sps =  (IMGPAR currentSlice->bIsBaseView) ? &SeqParSet[PicParSet[IMGPAR currentSlice->pic_parameter_set_id].seq_parameter_set_id] :
			&SeqParSubset[PicParSet[IMGPAR currentSlice->pic_parameter_set_id].seq_parameter_set_id];

	if(IMGPAR structure == FRAME || pic_combine_status)
	{
		IMGPAR UnCompress_Idx = (int)m_pUncompBufQueue->GetItem();
#if defined(_USE_QUEUE_FOR_DXVA2_)
		WaitForSingleObject(stream_global->hReadyForRender[IMGPAR UnCompress_Idx], INFINITE);
		ResetEvent(stream_global->hReadyForRender[IMGPAR UnCompress_Idx]);
#endif
	}

	dec_picture->pic_store_idx = IMGPAR UnCompress_Idx;

	DXVA_PicParams_H264* m_patiPictureDecode = (DXVA_PicParams_H264*)IMGPAR m_pnv1PictureDecode;
	memset(m_patiPictureDecode, 0, sizeof(DXVA_PicParams_H264));

	m_patiPictureDecode->CurrPic.Index7Bits		= IMGPAR UnCompress_Idx;
	m_patiPictureDecode->CurrPic.AssociatedFlag = (IMGPAR structure == FRAME ? 0 :(IMGPAR currentSlice->structure - 1));
	m_patiPictureDecode->wFrameWidthInMbsMinus1		= (int)sps->pic_width_in_mbs_minus1;
	m_patiPictureDecode->wFrameHeightInMbsMinus1		= IMGPAR FrameHeightInMbs-1;
	m_patiPictureDecode->num_ref_frames = sps->num_ref_frames;
	m_patiPictureDecode->field_pic_flag = (IMGPAR currentSlice->structure !=0);   //field_pic_flag
	m_patiPictureDecode->MbaffFrameFlag = IMGPAR currentSlice->MbaffFrameFlag;  //MbaffFrameFlag
	m_patiPictureDecode->residual_colour_transform_flag = (sps->chroma_format_idc ==3);  //residual_colour_transform_flag
	m_patiPictureDecode->sp_for_switch_flag = 0;   //sp_for_switch_flag
	m_patiPictureDecode->chroma_format_idc = sps->chroma_format_idc;  //chroma_format_idc
	m_patiPictureDecode->RefPicFlag = dec_picture->used_for_reference;
	m_patiPictureDecode->constrained_intra_pred_flag = pps->constrained_intra_pred_flag;  //constrained_intra_pred_flag
	m_patiPictureDecode->weighted_pred_flag = pps->weighted_pred_flag;  //weighted_pred_flag
	m_patiPictureDecode->weighted_bipred_idc = pps->weighted_bipred_idc;  //weighted_bipred_idc
	m_patiPictureDecode->MbsConsecutiveFlag = 1;//!sps->gaps_in_frame_num_value_allowed_flag;  //MbsConsecutiveFlag
	m_patiPictureDecode->frame_mbs_only_flag = sps->frame_mbs_only_flag;  //frame_mbs_only_flag
	m_patiPictureDecode->transform_8x8_mode_flag = pps->transform_8x8_mode_flag;  //transform_8x8_mode_flag
	m_patiPictureDecode->MinLumaBipredSize8x8Flag = 0; //MinLumaBipredSize8x8Flag
	m_patiPictureDecode->IntraPicFlag = (IMGPAR currentSlice->picture_type == I_SLICE);  //IntraPicFlag
	m_patiPictureDecode->bit_depth_luma_minus8 = sps->bit_depth_luma_minus8;
	m_patiPictureDecode->bit_depth_chroma_minus8 = sps->bit_depth_chroma_minus8;
	if(m_bResidualDataFormat==E_RES_DATA_FORMAT_INTEL)
	{
		//For the last slice of P, B pictures is I slice, the IntraPicFlag should be reset to 0
		if((m_patiPictureDecode->IntraPicFlag == 1) && dec_picture->has_PB_slice)  
			m_patiPictureDecode->IntraPicFlag = 0;
		m_patiPictureDecode->Reserved16Bits = 0x4E49;
	}
	m_patiPictureDecode->StatusReportFeedbackNumber = 1;
	m_patiPictureDecode->pic_init_qs_minus26 = pps->pic_init_qs_minus26;
	m_patiPictureDecode->chroma_qp_index_offset = pps->chroma_qp_index_offset;
	m_patiPictureDecode->second_chroma_qp_index_offset = pps->second_chroma_qp_index_offset;
	m_patiPictureDecode->ContinuationFlag = 0;
	//m_patiPictureDecode->pic_init_qp_minus26 = pps->pic_init_qp_minus26;                    //**********************
	//m_patiPictureDecode->num_ref_idx_l0_active_minus1 = pps->num_ref_idx_l0_active_minus1;  //*****************
	//m_patiPictureDecode->num_ref_idx_l1_active_minus1 = pps->num_ref_idx_l1_active_minus1;  //*****************
	m_patiPictureDecode->UsedForReferenceFlags = 0;
	m_patiPictureDecode->NonExistingFrameFlags = 0;
	//m_patiPictureDecode->frame_num = IMGPAR currentSlice->frame_num;   //*************
	//m_patiPictureDecode->log2_max_frame_num_minus4 = sps->log2_max_frame_num_minus4;   //****************
	m_patiPictureDecode->pic_order_cnt_type = sps->pic_order_cnt_type;
	//*m_patiPictureDecode->log2_max_pic_order_cnt_lsb_minus4 = sps->log2_max_pic_order_cnt_lsb_minus4;
	//*m_patiPictureDecode->delta_pic_order_always_zero_flag = sps->delta_pic_order_always_zero_flag;
	//*m_patiPictureDecode->direct_8x8_inference_flag = sps->direct_8x8_inference_flag;
	//*m_patiPictureDecode->entropy_coding_mode_flag = pps->entropy_coding_mode_flag;
	m_patiPictureDecode->pic_order_present_flag = pps->pic_order_present_flag;
	m_patiPictureDecode->num_slice_groups_minus1 = pps->num_slice_groups_minus1;
	//*m_patiPictureDecode->slice_group_map_type = pps->slice_group_map_type;
	//*m_patiPictureDecode->deblocking_filter_control_present_flag = pps->deblocking_filter_control_present_flag;
	//*m_patiPictureDecode->redundant_pic_cnt_present_flag = pps->redundant_pic_cnt_present_flag;
	//*m_patiPictureDecode->slice_group_change_rate_minus1 = pps->slice_group_change_rate_minus1;
	//m_patiPictureDecode->SliceGroupMap =;

	memcpy(m_patiPictureDecode->RefFrameList, m_RefList, 16);

	if(IMGPAR structure == FRAME || pic_combine_status)
		m_patiPictureDecode->RefFrameList[IMGPAR UnCompress_Idx].bPicEntry = m_RefList[IMGPAR UnCompress_Idx].bPicEntry = 0xFF;
	if(dec_picture->used_for_reference)
		m_patiPictureDecode->RefFrameList[IMGPAR UnCompress_Idx].bPicEntry = m_RefList[IMGPAR UnCompress_Idx].bPicEntry = IMGPAR UnCompress_Idx;


	for(i=0 ; i<16 ; i++)
	{
		m_patiPictureDecode->FieldOrderCntList[i][0] = m_RefInfo[i].top_poc;
		m_patiPictureDecode->FieldOrderCntList[i][1] = m_RefInfo[i].bot_poc;
		//******m_patiPictureDecode->FrameNumList[i] = m_RefInfo[i].frame_num;        
	}

	if(IMGPAR currentSlice->structure == FRAME)
	{
		m_RefInfo[IMGPAR UnCompress_Idx].top_poc = m_patiPictureDecode->CurrFieldOrderCnt[0] = IMGPAR toppoc;
		m_RefInfo[IMGPAR UnCompress_Idx].bot_poc = m_patiPictureDecode->CurrFieldOrderCnt[1] = IMGPAR bottompoc;
	}
	else if(IMGPAR currentSlice->structure == TOP_FIELD)
		m_RefInfo[IMGPAR UnCompress_Idx].top_poc = m_patiPictureDecode->CurrFieldOrderCnt[0] = IMGPAR toppoc;
	else
		m_RefInfo[IMGPAR UnCompress_Idx].bot_poc = m_patiPictureDecode->CurrFieldOrderCnt[1] = IMGPAR bottompoc;

	m_RefInfo[IMGPAR UnCompress_Idx].frame_num = IMGPAR currentSlice->frame_num;

	SetSurfaceInfo ARGS3(dec_picture->unique_id, IMGPAR currentSlice->structure ==2?1:0,1);

	if (m_nDXVAMode==E_H264_DXVA_MODE_C)
	{
		int j;
		DXVA_Qmatrix_H264 *t_pQMatrix = (DXVA_Qmatrix_H264*)IMGPAR m_lpQMatrix;
		memset(t_pQMatrix, 0, sizeof(DXVA_Qmatrix_H264));

		//copy QP matrix
		for(i = 0; i<6; i++)
			for(j = 0; j<16; j++)
				t_pQMatrix->bScalingLists4x4[i][j] = qmatrix[i][SNGL_SCAN_2D[j]];
		for(i = 6; i<8; i++)
			for(j = 0; j<64; j++)
				t_pQMatrix->bScalingLists8x8[i-6][j] = qmatrix[i][SNGL_SCAN8x8_2D[j]];
	}

	return 0;
}

int CH264DXVA2::build_slice_parameter_buffer PARGS0()
{
	int i, j;
	DXVA_Slice_H264_Long *BA_DATA_CTRL = (DXVA_Slice_H264_Long *)IMGPAR m_lpSLICE;

	memset(BA_DATA_CTRL,0,sizeof(DXVA_Slice_H264_Long));
	BA_DATA_CTRL->SliceBytesInBuffer = 0;
	BA_DATA_CTRL->wBadSliceChopping = 0;
	BA_DATA_CTRL->first_mb_in_slice = IMGPAR currentSlice->start_mb_nr;
	BA_DATA_CTRL->NumMbsForSlice = IMGPAR currentSlice->read_mb_nr;
	//BA_DATA_CTRL->BitOffsetToSliceData =
	BA_DATA_CTRL->slice_type = (IMGPAR currentSlice->picture_type+5);
	if(IMGPAR currentSlice->picture_type != I_SLICE && IMGPAR apply_weights)
	{
		BA_DATA_CTRL->luma_log2_weight_denom = IMGPAR luma_log2_weight_denom;
		BA_DATA_CTRL->chroma_log2_weight_denom = IMGPAR chroma_log2_weight_denom;
	}
	//	BA_DATA_CTRL->num_ref_idx_l0_active_minus1 = IMGPAR currentSlice->m_active_pps->num_ref_idx_l0_active_minus1;
	//	BA_DATA_CTRL->num_ref_idx_l1_active_minus1 = IMGPAR currentSlice->m_active_pps->num_ref_idx_l1_active_minus1;
	BA_DATA_CTRL->num_ref_idx_l0_active_minus1 = ((listXsize[0]-1)<0 ? 0 : listXsize[0]-1);
	BA_DATA_CTRL->num_ref_idx_l1_active_minus1 = ((listXsize[1]-1)<0 ? 0 : listXsize[1]-1);
	BA_DATA_CTRL->slice_alpha_c0_offset_div2 = IMGPAR currentSlice->LFAlphaC0Offset / 2;
	BA_DATA_CTRL->slice_beta_offset_div2 = IMGPAR currentSlice->LFBetaOffset /2;

	if(m_bResidualDataFormat == E_RES_DATA_FORMAT_INTEL)
	{
		if (IMGPAR currentSlice->picture_type == P_SLICE 
			|| IMGPAR currentSlice->picture_type == B_SLICE || IMGPAR currentSlice->picture_type == RB_SLICE)
			dec_picture->has_PB_slice = 1; 
	}

	memset(BA_DATA_CTRL->RefPicList,255,64);
	if(IMGPAR currentSlice->picture_type == P_SLICE)
	{
		for(i = 0; i < listXsize[0]; i++)
		{
			BA_DATA_CTRL->RefPicList[0][i].Index7Bits	= *(byte*)&m_pSurfaceInfo[listX[0][i]->unique_id].SInfo[0];
			BA_DATA_CTRL->RefPicList[0][i].AssociatedFlag = m_pSurfaceInfo[listX[0][i]->unique_id].SInfo[1];
		}
		if(IMGPAR MbaffFrameFlag)
		{
			//Top L0 map to list index
			for(j = 0; j < listXsize[2]; j++)
				for(i = 0; i < listXsize[0]; i++)
					if(m_pSurfaceInfo[listX[0][i]->unique_id].SInfo[0] == m_pSurfaceInfo[listX[2][j]->unique_id].SInfo[0])
					{
						IMGPAR m_TopL0Map[j] = i;
						break;
					}
					//Bottom L0 map to list index
					for(j = 0; j < listXsize[4]; j++)
						for(i = 0; i < listXsize[0]; i++)
							if(m_pSurfaceInfo[listX[0][i]->unique_id].SInfo[0] == m_pSurfaceInfo[listX[4][j]->unique_id].SInfo[0])
							{
								IMGPAR m_BotL0Map[j] = i;
								break;
							}
		}
	}
	else if (IMGPAR currentSlice->picture_type == B_SLICE)
	{
		for(i = 0; i < listXsize[0]; i++)
		{
			BA_DATA_CTRL->RefPicList[0][i].Index7Bits	= *(byte*)&m_pSurfaceInfo[listX[0][i]->unique_id].SInfo[0];
			BA_DATA_CTRL->RefPicList[0][i].AssociatedFlag = m_pSurfaceInfo[listX[0][i]->unique_id].SInfo[1];
		}
		for(i = 0; i < listXsize[1]; i++)
		{
			BA_DATA_CTRL->RefPicList[1][i].Index7Bits	= *(byte*)&m_pSurfaceInfo[listX[1][i]->unique_id].SInfo[0];
			BA_DATA_CTRL->RefPicList[1][i].AssociatedFlag = m_pSurfaceInfo[listX[1][i]->unique_id].SInfo[1];
		}
		if(IMGPAR MbaffFrameFlag)
		{
			//Top L0 map to list index
			for(j = 0; j < listXsize[2]; j++)
				for(i = 0; i < listXsize[0]; i++)
					if(m_pSurfaceInfo[listX[0][i]->unique_id].SInfo[0] == m_pSurfaceInfo[listX[2][j]->unique_id].SInfo[0])
					{
						IMGPAR m_TopL0Map[j] = i;
						break;
					}
					//Top L1 map to list index
					for(j = 0; j < listXsize[3]; j++)
						for(i = 0; i < listXsize[1]; i++)
							if(m_pSurfaceInfo[listX[1][i]->unique_id].SInfo[0] == m_pSurfaceInfo[listX[3][j]->unique_id].SInfo[0])
							{
								IMGPAR m_TopL1Map[j] = i;
								break;
							}
							//Bottom L0 map to list index
							for(j = 0; j < listXsize[4]; j++)
								for(i = 0; i < listXsize[0]; i++)
									if(m_pSurfaceInfo[listX[0][i]->unique_id].SInfo[0] == m_pSurfaceInfo[listX[4][j]->unique_id].SInfo[0])
									{
										IMGPAR m_BotL0Map[j] = i;
										break;
									}
									//Bottom L1 map to list index
									for(j = 0; j < listXsize[5]; j++)
										for(i = 0; i < listXsize[1]; i++)
											if(m_pSurfaceInfo[listX[1][i]->unique_id].SInfo[0] == m_pSurfaceInfo[listX[5][j]->unique_id].SInfo[0])
											{
												IMGPAR m_BotL1Map[j] = i;
												break;
											}
		}
	}
	if (m_bResidualDataFormat==E_RES_DATA_FORMAT_INTEL && active_pps.weighted_bipred_idc != 2 && IMGPAR apply_weights)
	{												
		USHORT  m_wo_prevent_code_Intel = 0x0001;  //Intel driver can't distinguish 0x0000 from a real Reserved16bits number.
		DXVA_PicParams_H264* m_patiPictureDecode = (DXVA_PicParams_H264*)IMGPAR m_pnv1PictureDecode;

		if(IMGPAR currentSlice->picture_type != I_SLICE && IMGPAR apply_weights)
		{
			for(i = 0; i < listXsize[0]; i++)
			{
				BA_DATA_CTRL->Weights[0][i][0][0] = *((IMGPAR wp_weight+(i+0)*3+0));
				BA_DATA_CTRL->Weights[0][i][0][1] = *((IMGPAR wp_offset+(i+0)*3+0));
				if((*((IMGPAR wp_weight+(i+0)*3+0)) == (m_wo_prevent_code_Intel & 0x00ff)) 
					&& (*((IMGPAR wp_offset+(i+0)*3+0)) == (m_wo_prevent_code_Intel >> 8)))
				{
					m_wo_prevent_code_Intel++;
				}

				BA_DATA_CTRL->Weights[0][i][1][0] = *((IMGPAR wp_weight+(i+0)*3+1));
				BA_DATA_CTRL->Weights[0][i][1][1] = *((IMGPAR wp_offset+(i+0)*3+1));
				if((*((IMGPAR wp_weight+(i+0)*3+1)) == (m_wo_prevent_code_Intel & 0x00ff)) 
					&& (*((IMGPAR wp_offset+(i+0)*3+1)) == (m_wo_prevent_code_Intel >> 8)))
				{
					m_wo_prevent_code_Intel++;
				}

				BA_DATA_CTRL->Weights[0][i][2][0] = *((IMGPAR wp_weight+(i+0)*3+2));
				BA_DATA_CTRL->Weights[0][i][2][1] = *((IMGPAR wp_offset+(i+0)*3+2));
				if((*((IMGPAR wp_weight+(i+0)*3+2)) == (m_wo_prevent_code_Intel & 0x00ff)) 
					&& (*((IMGPAR wp_offset+(i+0)*3+2)) == (m_wo_prevent_code_Intel >> 8)))
				{
					m_wo_prevent_code_Intel++;
				}
			}
			if(IMGPAR currentSlice->picture_type == B_SLICE)
			{
				for(i = 0; i < listXsize[1]; i++)
				{
					BA_DATA_CTRL->Weights[1][i][0][0] = *((IMGPAR wp_weight+(MAX_REFERENCE_PICTURES+i)*3+0));
					BA_DATA_CTRL->Weights[1][i][0][1] = *((IMGPAR wp_offset+(MAX_REFERENCE_PICTURES+i)*3+0));
					if((*((IMGPAR wp_weight+(MAX_REFERENCE_PICTURES+i)*3+0)) == (m_wo_prevent_code_Intel & 0x00ff)) 
						&& (*((IMGPAR wp_offset+(MAX_REFERENCE_PICTURES+i)*3+0)) == (m_wo_prevent_code_Intel >> 8)))
					{
						m_wo_prevent_code_Intel++;
					}

					BA_DATA_CTRL->Weights[1][i][1][0] = *((IMGPAR wp_weight+(MAX_REFERENCE_PICTURES+i)*3+1));
					BA_DATA_CTRL->Weights[1][i][1][1] = *((IMGPAR wp_offset+(MAX_REFERENCE_PICTURES+i)*3+1));
					if((*((IMGPAR wp_weight+(MAX_REFERENCE_PICTURES+i)*3+1)) == (m_wo_prevent_code_Intel & 0x00ff)) 
						&& (*((IMGPAR wp_offset+(MAX_REFERENCE_PICTURES+i)*3+1)) == (m_wo_prevent_code_Intel >> 8)))
					{
						m_wo_prevent_code_Intel++;
					}

					BA_DATA_CTRL->Weights[1][i][2][0] = *((IMGPAR wp_weight+(MAX_REFERENCE_PICTURES+i)*3+2));
					BA_DATA_CTRL->Weights[1][i][2][1] = *((IMGPAR wp_offset+(MAX_REFERENCE_PICTURES+i)*3+2));
					if((*((IMGPAR wp_weight+(MAX_REFERENCE_PICTURES+i)*3+2)) == (m_wo_prevent_code_Intel & 0x00ff)) 
						&& (*((IMGPAR wp_offset+(MAX_REFERENCE_PICTURES+i)*3+2)) == (m_wo_prevent_code_Intel >> 8)))
					{
						m_wo_prevent_code_Intel++;
					}
				}
				//active_pps->weighted_bipred_idc != 2 (explicit), wbp is not used, so remove checking of wbp
			}
			m_patiPictureDecode->Reserved16Bits = m_wo_prevent_code_Intel;
		}

		//replace 0x0080
		if(IMGPAR currentSlice->picture_type != I_SLICE && IMGPAR apply_weights)
		{
			for(i = 0; i < listXsize[0]; i++)
			{
				if((*((IMGPAR wp_weight+(i+0)*3+0)) == 128) 
					&& (*((IMGPAR wp_offset+(i+0)*3+0)) == 0))
				{
					*((IMGPAR wp_weight+(i+0)*3+0)) = m_wo_prevent_code_Intel & 0x00ff;
					*((IMGPAR wp_offset+(i+0)*3+0)) = m_wo_prevent_code_Intel >> 8;
				}

				if((*((IMGPAR wp_weight+(i+0)*3+1)) == 128) 
					&& (*((IMGPAR wp_offset+(i+0)*3+1)) == 0))
				{
					*((IMGPAR wp_weight+(i+0)*3+1)) = m_wo_prevent_code_Intel & 0x00ff;
					*((IMGPAR wp_offset+(i+0)*3+1)) = m_wo_prevent_code_Intel >> 8;
				}

				if((*((IMGPAR wp_weight+(i+0)*3+2)) == 128) 
					&& (*((IMGPAR wp_offset+(i+0)*3+2)) == 0))
				{
					*((IMGPAR wp_weight+(i+0)*3+2)) = m_wo_prevent_code_Intel & 0x00ff;
					*((IMGPAR wp_offset+(i+0)*3+2)) = m_wo_prevent_code_Intel >> 8;
				}
			}
			if(IMGPAR currentSlice->picture_type == B_SLICE)
			{
				for(i = 0; i < listXsize[1]; i++)
				{
					if((*((IMGPAR wp_weight+(MAX_REFERENCE_PICTURES+i)*3+0)) == 128) 
						&& (*((IMGPAR wp_offset+(MAX_REFERENCE_PICTURES+i)*3+0)) == 0))
					{
						*((IMGPAR wp_weight+(MAX_REFERENCE_PICTURES+i)*3+0)) = m_wo_prevent_code_Intel & 0x00ff;
						*((IMGPAR wp_offset+(MAX_REFERENCE_PICTURES+i)*3+0)) = m_wo_prevent_code_Intel >> 8;
					}

					if((*((IMGPAR wp_weight+(MAX_REFERENCE_PICTURES+i)*3+1)) == 128) 
						&& (*((IMGPAR wp_offset+(MAX_REFERENCE_PICTURES+i)*3+1)) == 0))
					{
						*((IMGPAR wp_weight+(MAX_REFERENCE_PICTURES+i)*3+1)) = m_wo_prevent_code_Intel & 0x00ff;
						*((IMGPAR wp_offset+(MAX_REFERENCE_PICTURES+i)*3+1)) = m_wo_prevent_code_Intel >> 8;
					}

					if((*((IMGPAR wp_weight+(MAX_REFERENCE_PICTURES+i)*3+2)) == 128) 
						&& (*((IMGPAR wp_offset+(MAX_REFERENCE_PICTURES+i)*3+2)) == 0))
					{
						*((IMGPAR wp_weight+(MAX_REFERENCE_PICTURES+i)*3+2)) = m_wo_prevent_code_Intel & 0x00ff;
						*((IMGPAR wp_offset+(MAX_REFERENCE_PICTURES+i)*3+2)) = m_wo_prevent_code_Intel >> 8;
					}
				}
				//active_pps->weighted_bipred_idc != 2 (explicit), wbp is not used, so remove replacement for wbp
			}
		}

	}
	else if(IMGPAR currentSlice->picture_type != I_SLICE && IMGPAR apply_weights)
	{
		for(i = 0; i < listXsize[0]; i++)
		{
			BA_DATA_CTRL->Weights[0][i][0][0] = *((IMGPAR wp_weight+(i+0)*3+0));
			BA_DATA_CTRL->Weights[0][i][0][1] = *((IMGPAR wp_offset+(i+0)*3+0));
			BA_DATA_CTRL->Weights[0][i][1][0] = *((IMGPAR wp_weight+(i+0)*3+1));
			BA_DATA_CTRL->Weights[0][i][1][1] = *((IMGPAR wp_offset+(i+0)*3+1));
			BA_DATA_CTRL->Weights[0][i][2][0] = *((IMGPAR wp_weight+(i+0)*3+2));
			BA_DATA_CTRL->Weights[0][i][2][1] = *((IMGPAR wp_offset+(i+0)*3+2));
		}
		if(IMGPAR currentSlice->picture_type == B_SLICE)
		{
			for(i = 0; i < listXsize[1]; i++)
			{
				BA_DATA_CTRL->Weights[1][i][0][0] = *((IMGPAR wp_weight+(MAX_REFERENCE_PICTURES+i)*3+0));
				BA_DATA_CTRL->Weights[1][i][0][1] = *((IMGPAR wp_offset+(MAX_REFERENCE_PICTURES+i)*3+0));
				BA_DATA_CTRL->Weights[1][i][1][0] = *((IMGPAR wp_weight+(MAX_REFERENCE_PICTURES+i)*3+1));
				BA_DATA_CTRL->Weights[1][i][1][1] = *((IMGPAR wp_offset+(MAX_REFERENCE_PICTURES+i)*3+1));
				BA_DATA_CTRL->Weights[1][i][2][0] = *((IMGPAR wp_weight+(MAX_REFERENCE_PICTURES+i)*3+2));
				BA_DATA_CTRL->Weights[1][i][2][1] = *((IMGPAR wp_offset+(MAX_REFERENCE_PICTURES+i)*3+2));
			}
		}
	}

	//	BA_DATA_CTRL->slice_qs_delta = IMGPAR currentSlice->slice_qs_delta;
	BA_DATA_CTRL->slice_qp_delta = IMGPAR currentSlice->slice_qp_delta;
	//BA_DATA_CTRL->redundant_pic_cnt = IMGPAR currentSlice->m_active_pps->redundant_pic_cnt_present_flag;
	BA_DATA_CTRL->redundant_pic_cnt = active_pps.redundant_pic_cnt_present_flag;
	BA_DATA_CTRL->direct_spatial_mv_pred_flag = IMGPAR direct_spatial_mv_pred_flag;
	BA_DATA_CTRL->cabac_init_idc = IMGPAR model_number;
	//*BA_DATA_CTRL->disable_deblocking_filter_idc = IMGPAR currentSlice->LFDisableIdc;
	BA_DATA_CTRL->disable_deblocking_filter_idc = 0;
	BA_DATA_CTRL->slice_id = IMGPAR current_slice_nr;

	if(stream_global->m_is_MTMS)
		IMGPAR m_lpSLICE += sizeof(DXVA_Slice_H264_Long);

	return 0;
}

void CH264DXVA2::build_macroblock_buffer_Inter_16x16 PARGS1(int list_offset)
{
	int pred_dir = currMB_d->b8pdir[0];
	int cbp=0, b8, ref_picid, ref_picid_se, ref_bot, ref_bot_se;
	int curr_mb_field = ((IMGPAR MbaffFrameFlag)&&(currMB_s_d->mb_field));
	int current_mb_nr = IMGPAR current_mb_nr_d;

	LPDXVA_MBctrl_H264 Mb_partition = (LPDXVA_MBctrl_H264)(IMGPAR m_lpMBLK_Intra_Luma + IMGPAR m_iIntraMCBufUsage);
	memset(Mb_partition, 0, sizeof(DXVA_MBctrl_H264));
	LPDXVA_MVvalue Mv;
	if (m_bResidualDataFormat==E_RES_DATA_FORMAT_INTEL)
	{
		Mv = (LPDXVA_MVvalue)(IMGPAR m_lpRESD_Intra_Luma + IMGPAR m_iIntraRESBufUsage);
		memset(Mv, 0 ,192);
	}
	else
		Mv = (LPDXVA_MVvalue)(IMGPAR m_lpMV+ IMGPAR m_iInterMCBufUsage);

	Mb_partition->bSliceID		= IMGPAR current_slice_nr;
	Mb_partition->MbType5Bits	= pred_dir + 1;
	//Mb_partition->IntraMbFlag	= 0;
	if(m_bResidualDataFormat==E_RES_DATA_FORMAT_INTEL)
		Mb_partition->mb_field_decoding_flag	= IMGPAR MbaffFrameFlag ? curr_mb_field : IMGPAR field_pic_flag;
	else
		Mb_partition->mb_field_decoding_flag	= curr_mb_field;// ? IMGPAR MbaffFrameFlag : IMGPAR field_pic_flag;
	Mb_partition->transform_size_8x8_flag	= (m_bResidualDataFormat==E_RES_DATA_FORMAT_NV ? 1 : currMB_d->luma_transform_size_8x8_flag);
	Mb_partition->HostResidDiff = (m_nDXVAMode==E_H264_DXVA_MODE_A) ? 1 : 0;
	//Mb_partition->bMvQuantity = 0;
	Mb_partition->CurrMbAddr = IMGPAR current_mb_nr_d;
	Mb_partition->bQpPrime[0] = currMB_s_d->qp;
	Mb_partition->bQpPrime[1] = QP_SCALE_CR_Intel[Clip3(0, 51, currMB_s_d->qp + dec_picture->chroma_qp_offset[0])];
	Mb_partition->bQpPrime[2] = QP_SCALE_CR_Intel[Clip3(0, 51, currMB_s_d->qp + dec_picture->chroma_qp_offset[1])];


	if(m_nDXVAMode==E_H264_DXVA_MODE_A)
	{
		Mb_partition->wPatternCode[0] = getPatternCode ARGS1(Mb_partition->transform_size_8x8_flag);
		for(b8 = 3; b8 >= 0; b8--)
		{
			if(currMB_s_d->cbp_blk & (1<<(19-b8)))
				Mb_partition->wPatternCode[1] |= (1<<b8);
			if(currMB_s_d->cbp_blk & (1<<(23-b8)))
				Mb_partition->wPatternCode[2] |= (1<<b8);
		}
		Mb_partition->bMBresidDataQuantity = getMBresidDataQuantity ARGS3(currMB_s_d->cbp_blk, currMB_d->cbp, Mb_partition->transform_size_8x8_flag);
	}
	if(m_bResidualDataFormat==E_RES_DATA_FORMAT_INTEL)
		Mb_partition->bMBresidDataQuantity += 12;

	Mb_partition->dwMBdataLocation = (IMGPAR m_iIntraRESBufUsage>>2);
	Mb_partition->wMvBuffOffset = (IMGPAR m_iInterMCBufUsage>>2);

	if(pred_dir != 2)
	{
		//ref_picid = currMB_s_d->pred_info.ref_pic_id[pred_dir][0];		
		int ref_idx = currMB_s_d->pred_info.ref_idx[pred_dir][0];		
		if(!curr_mb_field)
		{
			if(pred_dir)
			{
				Mb_partition->bRefPicSelect[LIST_1][0] = currMB_s_d->pred_info.ref_idx[pred_dir][0];
				Mb_partition->bRefPicSelect[LIST_0][0] = 0xff;
			}
			else
			{
				Mb_partition->bRefPicSelect[LIST_0][0] = currMB_s_d->pred_info.ref_idx[pred_dir][0];
				Mb_partition->bRefPicSelect[LIST_1][0] = 0xff;
			}
		}
		else
		{
			ref_picid = currMB_s_d->pred_info.ref_pic_id[pred_dir][0];
			ref_bot = m_pSurfaceInfo[ref_picid].SInfo[1];
			if(current_mb_nr&1)
			{
				if(pred_dir)
				{
					Mb_partition->bRefPicSelect[LIST_1][0] = (IMGPAR m_BotL1Map[currMB_s_d->pred_info.ref_idx[pred_dir][0]]<<1);
					Mb_partition->bRefPicSelect[LIST_1][0] |= (ref_bot ? 0 : 1);
					Mb_partition->bRefPicSelect[LIST_0][0] = 0xff;
				}
				else
				{
					Mb_partition->bRefPicSelect[LIST_0][0] = (IMGPAR m_BotL0Map[currMB_s_d->pred_info.ref_idx[pred_dir][0]]<<1);
					Mb_partition->bRefPicSelect[LIST_0][0] |= (ref_bot ? 0 : 1);
					Mb_partition->bRefPicSelect[LIST_1][0] = 0xff;
				}
			}
			else
			{
				if(pred_dir)
				{
					Mb_partition->bRefPicSelect[LIST_1][0] = (IMGPAR m_TopL1Map[currMB_s_d->pred_info.ref_idx[pred_dir][0]]<<1);
					Mb_partition->bRefPicSelect[LIST_1][0] |= (ref_bot ? 1 : 0);
					Mb_partition->bRefPicSelect[LIST_0][0] = 0xff;
				}
				else
				{
					Mb_partition->bRefPicSelect[LIST_0][0] = (IMGPAR m_TopL0Map[currMB_s_d->pred_info.ref_idx[pred_dir][0]]<<1);
					Mb_partition->bRefPicSelect[LIST_0][0] |= (ref_bot ? 1 : 0);
					Mb_partition->bRefPicSelect[LIST_1][0] = 0xff;
				}
			}
		}
		SHORT sHorz = Mv->horz = currMB_s_d->pred_info.mv[pred_dir][0].x;
		SHORT sVert = Mv->vert = currMB_s_d->pred_info.mv[pred_dir][0].y;
		Mv++;
		Mb_partition->bMvQuantity++;
		IMGPAR m_iInterMCBufUsage += sizeof(DXVA_MVvalue); // It has no effect for Intel, do not increase the m_iIntraRESBufUsage for Intel
														   // because the value 192 added to m_iIntraRESBufUsage is the size of MV and weight-offset
		if (m_bResidualDataFormat==E_RES_DATA_FORMAT_INTEL)
		{
			Mv->horz = sHorz;
			Mv->vert = sVert;
			Mb_partition->bMvQuantity++;
			IMGPAR m_iInterMCBufUsage += sizeof(DXVA_MVvalue)*3;

			//build weight/offset data block
			//Mv+=2;
			Mv+=3; //L1 and two reserved DWord
			//Mv+=28; //28 n/a DWords  //Marked to Pack WO data right after MV data
			char *pData = (char*)Mv;

			if (active_pps.weighted_pred_flag && curr_mb_field)
				ref_idx >>=1;

			if (IMGPAR apply_weights)
			{
				build_weight_data_block_INTEL ARGS5(pred_dir, pData, ref_idx, ref_idx, list_offset);
			}
		}
	}
	else
	{
		//ref_picid = currMB_s_d->pred_info.ref_pic_id[LIST_0][0];
		//ref_picid_se = currMB_s_d->pred_info.ref_pic_id[LIST_1][0];
		int ref_idx_0 = currMB_s_d->pred_info.ref_idx[LIST_0][0];
		int ref_idx_1 = currMB_s_d->pred_info.ref_idx[LIST_1][0];
		if(!curr_mb_field)
		{
			Mb_partition->bRefPicSelect[LIST_0][0] = currMB_s_d->pred_info.ref_idx[LIST_0][0];
			Mb_partition->bRefPicSelect[LIST_1][0] = currMB_s_d->pred_info.ref_idx[LIST_1][0];
		}
		else
		{
			ref_picid = currMB_s_d->pred_info.ref_pic_id[LIST_0][0];
			ref_picid_se = currMB_s_d->pred_info.ref_pic_id[LIST_1][0];
			ref_bot = m_pSurfaceInfo[ref_picid].SInfo[1];
			ref_bot_se = m_pSurfaceInfo[ref_picid_se].SInfo[1];
			if(current_mb_nr&1)
			{
				Mb_partition->bRefPicSelect[LIST_0][0] = (IMGPAR m_BotL0Map[currMB_s_d->pred_info.ref_idx[LIST_0][0]]<<1);
				Mb_partition->bRefPicSelect[LIST_0][0] |= (ref_bot ? 0 : 1);
				Mb_partition->bRefPicSelect[LIST_1][0] = (IMGPAR m_BotL1Map[currMB_s_d->pred_info.ref_idx[LIST_1][0]]<<1);
				Mb_partition->bRefPicSelect[LIST_1][0] |= (ref_bot_se ? 0 : 1);
			}
			else
			{
				Mb_partition->bRefPicSelect[LIST_0][0] = (IMGPAR m_TopL0Map[currMB_s_d->pred_info.ref_idx[LIST_0][0]]<<1);
				Mb_partition->bRefPicSelect[LIST_0][0] |= (ref_bot ? 1 : 0);
				Mb_partition->bRefPicSelect[LIST_1][0] = (IMGPAR m_TopL1Map[currMB_s_d->pred_info.ref_idx[LIST_1][0]]<<1);
				Mb_partition->bRefPicSelect[LIST_1][0] |= (ref_bot_se ? 1 : 0);
			}
		}
		Mv->horz = currMB_s_d->pred_info.mv[LIST_0][0].x;
		Mv->vert = currMB_s_d->pred_info.mv[LIST_0][0].y;
		Mv++;
		Mv->horz = currMB_s_d->pred_info.mv[LIST_1][0].x;
		Mv->vert = currMB_s_d->pred_info.mv[LIST_1][0].y;
		Mv++;
		Mb_partition->bMvQuantity += 2;
		IMGPAR m_iInterMCBufUsage += sizeof(DXVA_MVvalue)*2;

		if (m_bResidualDataFormat==E_RES_DATA_FORMAT_INTEL)
		{
			IMGPAR m_iInterMCBufUsage += sizeof(DXVA_MVvalue)*2;

			//build weight/offset data block
			//Mv+=2;
			Mv+=2; //two reserved DWord
			//Mv+=28;	//28 n/a DWords  //Marked to Pack WO data right after MV data
			char *pData = (char*)Mv;

			if (active_pps.weighted_bipred_idc==1 && curr_mb_field)
			{
				ref_idx_0 >>=1;
				ref_idx_1 >>=1;
			}

			if (IMGPAR apply_weights)
			{
				build_weight_data_block_INTEL ARGS5(pred_dir, pData, ref_idx_0, ref_idx_1, list_offset);
			}
		}
	}
}

void CH264DXVA2::build_macroblock_buffer_Inter_16x8 PARGS1(int list_offset)
{
	int pred_dir;
	int cbp=0, joff, k, b8, ref_picid, ref_picid_se, ref_bot, ref_bot_se;
	int curr_mb_field = ((IMGPAR MbaffFrameFlag)&&(currMB_s_d->mb_field));
	int current_mb_nr = IMGPAR current_mb_nr_d;
	int block_index = 0;	

	LPDXVA_MBctrl_H264 Mb_partition = (LPDXVA_MBctrl_H264)(IMGPAR m_lpMBLK_Intra_Luma + IMGPAR m_iIntraMCBufUsage);
	memset(Mb_partition, 0, sizeof(DXVA_MBctrl_H264));
	LPDXVA_MVvalue Mv;
	if (m_bResidualDataFormat==E_RES_DATA_FORMAT_INTEL)
	{
		Mv = (LPDXVA_MVvalue)(IMGPAR m_lpRESD_Intra_Luma + IMGPAR m_iIntraRESBufUsage);
		memset(Mv, 0 ,192);
	}
	else
		Mv = (LPDXVA_MVvalue)(IMGPAR m_lpMV+ IMGPAR m_iInterMCBufUsage);

	Mb_partition->bSliceID		= IMGPAR current_slice_nr;
	//Mb_partition->IntraMbFlag	= 0;
	if(m_bResidualDataFormat==E_RES_DATA_FORMAT_INTEL)
		Mb_partition->mb_field_decoding_flag	= IMGPAR MbaffFrameFlag ? curr_mb_field : IMGPAR field_pic_flag;
	else
		Mb_partition->mb_field_decoding_flag	= curr_mb_field;// ? IMGPAR MbaffFrameFlag : IMGPAR field_pic_flag;
	Mb_partition->transform_size_8x8_flag	= (m_bResidualDataFormat==E_RES_DATA_FORMAT_NV ? 1 : currMB_d->luma_transform_size_8x8_flag);
	Mb_partition->HostResidDiff = (m_nDXVAMode==E_H264_DXVA_MODE_A) ? 1 : 0;
	//Mb_partition->bMvQuantity = 0;
	Mb_partition->CurrMbAddr = IMGPAR current_mb_nr_d;
	Mb_partition->bQpPrime[0] = currMB_s_d->qp;
	Mb_partition->bQpPrime[1] = QP_SCALE_CR_Intel[Clip3(0, 51, currMB_s_d->qp + dec_picture->chroma_qp_offset[0])];
	Mb_partition->bQpPrime[2] = QP_SCALE_CR_Intel[Clip3(0, 51, currMB_s_d->qp + dec_picture->chroma_qp_offset[1])];

	if(m_nDXVAMode==E_H264_DXVA_MODE_A)
	{
		Mb_partition->wPatternCode[0] = getPatternCode ARGS1(Mb_partition->transform_size_8x8_flag);
		for(b8 = 3; b8 >= 0; b8--)
		{
			if(currMB_s_d->cbp_blk & (1<<(19-b8)))
				Mb_partition->wPatternCode[1] |= (1<<b8);
			if(currMB_s_d->cbp_blk & (1<<(23-b8)))
				Mb_partition->wPatternCode[2] |= (1<<b8);
		}
		Mb_partition->bMBresidDataQuantity = getMBresidDataQuantity ARGS3(currMB_s_d->cbp_blk, currMB_d->cbp, Mb_partition->transform_size_8x8_flag);
	}
	if(m_bResidualDataFormat==E_RES_DATA_FORMAT_INTEL)
		Mb_partition->bMBresidDataQuantity += 12;

	Mb_partition->dwMBdataLocation = (IMGPAR m_iIntraRESBufUsage>>2);
	Mb_partition->wMvBuffOffset = (IMGPAR m_iInterMCBufUsage>>2);

	for(k=0; k<4; k+=2)
	{
		joff = (k<<2);
		pred_dir = currMB_d->b8pdir[k];
		block_index += ((pred_dir+1)<<k);

		if(pred_dir != 2)
		{
			//ref_picid = currMB_s_d->pred_info.ref_pic_id[pred_dir][k];
			MotionVector *pMV = &(currMB_s_d->pred_info.mv[pred_dir][joff]);
			if(!curr_mb_field)
			{
				if(pred_dir)
				{
					Mb_partition->bRefPicSelect[LIST_1][k>>1] = currMB_s_d->pred_info.ref_idx[pred_dir][k];
					Mb_partition->bRefPicSelect[LIST_0][k>>1] = 0xff;
				}
				else
				{
					Mb_partition->bRefPicSelect[LIST_0][k>>1] = currMB_s_d->pred_info.ref_idx[pred_dir][k];
					Mb_partition->bRefPicSelect[LIST_1][k>>1] = 0xff;
				}
			}
			else
			{
				ref_picid = currMB_s_d->pred_info.ref_pic_id[pred_dir][k];
				ref_bot = m_pSurfaceInfo[ref_picid].SInfo[1];
				if(current_mb_nr&1)
				{
					if(pred_dir)
					{
						Mb_partition->bRefPicSelect[LIST_1][k>>1] = (IMGPAR m_BotL1Map[currMB_s_d->pred_info.ref_idx[pred_dir][k]]<<1);
						Mb_partition->bRefPicSelect[LIST_1][k>>1] |= (ref_bot ? 0 : 1);
						Mb_partition->bRefPicSelect[LIST_0][k>>1] = 0xff;
					}
					else
					{
						Mb_partition->bRefPicSelect[LIST_0][k>>1] = (IMGPAR m_BotL0Map[currMB_s_d->pred_info.ref_idx[pred_dir][k]]<<1);
						Mb_partition->bRefPicSelect[LIST_0][k>>1] |= (ref_bot ? 0 : 1);
						Mb_partition->bRefPicSelect[LIST_1][k>>1] = 0xff;
					}
				}
				else
				{
					if(pred_dir)
					{
						Mb_partition->bRefPicSelect[LIST_1][k>>1] = (IMGPAR m_TopL1Map[currMB_s_d->pred_info.ref_idx[pred_dir][k]]<<1);
						Mb_partition->bRefPicSelect[LIST_1][k>>1] |= (ref_bot ? 1 : 0);
						Mb_partition->bRefPicSelect[LIST_0][k>>1] = 0xff;
					}
					else
					{
						Mb_partition->bRefPicSelect[LIST_0][k>>1] = (IMGPAR m_TopL0Map[currMB_s_d->pred_info.ref_idx[pred_dir][k]]<<1);
						Mb_partition->bRefPicSelect[LIST_0][k>>1] |= (ref_bot ? 1 : 0);
						Mb_partition->bRefPicSelect[LIST_1][k>>1] = 0xff;
					}
				}
			}	
			Mv->horz = pMV->x;
			Mv->vert = pMV->y;
			Mv++;
			Mb_partition->bMvQuantity++;
			IMGPAR m_iInterMCBufUsage += sizeof(DXVA_MVvalue);

			if (m_bResidualDataFormat==E_RES_DATA_FORMAT_INTEL)
			{
				Mv->horz = pMV->x;
				Mv->vert = pMV->y;
				Mv++;
				Mv->horz = pMV->x;
				Mv->vert = pMV->y;
				Mv++;
				Mv->horz = pMV->x;
				Mv->vert = pMV->y;
				Mv++;
				Mb_partition->bMvQuantity+=3;
				IMGPAR m_iInterMCBufUsage += sizeof(DXVA_MVvalue)*3;
			}
		}
		else
		{
			//ref_picid = currMB_s_d->pred_info.ref_pic_id[LIST_0][k];
			//ref_picid_se = currMB_s_d->pred_info.ref_pic_id[LIST_1][k];
			MotionVector *pMV0 = &(currMB_s_d->pred_info.mv[LIST_0][joff]);
			MotionVector *pMV1 = &(currMB_s_d->pred_info.mv[LIST_1][joff]);
			if(!curr_mb_field)
			{
				Mb_partition->bRefPicSelect[LIST_0][k>>1] = currMB_s_d->pred_info.ref_idx[LIST_0][k];
				Mb_partition->bRefPicSelect[LIST_1][k>>1] = currMB_s_d->pred_info.ref_idx[LIST_1][k];
			}
			else
			{
				ref_picid = currMB_s_d->pred_info.ref_pic_id[LIST_0][k];
				ref_picid_se = currMB_s_d->pred_info.ref_pic_id[LIST_1][k];
				ref_bot = m_pSurfaceInfo[ref_picid].SInfo[1];
				ref_bot_se = m_pSurfaceInfo[ref_picid_se].SInfo[1];
				if(current_mb_nr&1)
				{
					Mb_partition->bRefPicSelect[LIST_0][k>>1] = (IMGPAR m_BotL0Map[currMB_s_d->pred_info.ref_idx[LIST_0][k]]<<1);
					Mb_partition->bRefPicSelect[LIST_0][k>>1] |= (ref_bot ? 0 : 1);
					Mb_partition->bRefPicSelect[LIST_1][k>>1] = (IMGPAR m_BotL1Map[currMB_s_d->pred_info.ref_idx[LIST_1][k]]<<1);
					Mb_partition->bRefPicSelect[LIST_1][k>>1] |= (ref_bot_se ? 0 : 1);
				}
				else
				{
					Mb_partition->bRefPicSelect[LIST_0][k>>1] = (IMGPAR m_TopL0Map[currMB_s_d->pred_info.ref_idx[LIST_0][k]]<<1);
					Mb_partition->bRefPicSelect[LIST_0][k>>1] |= (ref_bot ? 1 : 0);
					Mb_partition->bRefPicSelect[LIST_1][k>>1] = (IMGPAR m_TopL1Map[currMB_s_d->pred_info.ref_idx[LIST_1][k]]<<1);
					Mb_partition->bRefPicSelect[LIST_1][k>>1] |= (ref_bot_se ? 1 : 0);
				}
			}		
			Mv->horz = pMV0->x;
			Mv->vert = pMV0->y;
			Mv++;
			Mv->horz = pMV1->x;
			Mv->vert = pMV1->y;
			Mv++;
			Mb_partition->bMvQuantity += 2;
			IMGPAR m_iInterMCBufUsage += sizeof(DXVA_MVvalue)*2;

			if (m_bResidualDataFormat==E_RES_DATA_FORMAT_INTEL)
			{
				Mv->horz = pMV0->x;
				Mv->vert = pMV0->y;
				Mv++;
				Mv->horz = pMV1->x;
				Mv->vert = pMV1->y;
				Mv++;
				Mb_partition->bMvQuantity += 2;
				IMGPAR m_iInterMCBufUsage += sizeof(DXVA_MVvalue)*2;
			}
		}
	}

	//build weight/offset data block
	if (m_bResidualDataFormat==E_RES_DATA_FORMAT_INTEL && IMGPAR apply_weights)
	{
		//Mv+=24; //24 n/a DWords  //Marked to Pack WO data right after MV data
		char *pData = (char*)Mv;
		for(k=0; k<4; k+=2)
		{
			pred_dir = currMB_d->b8pdir[k];
			if(pred_dir != 2)
			{
				int ref_idx = currMB_s_d->pred_info.ref_idx[pred_dir][k];

				if (active_pps.weighted_pred_flag && curr_mb_field)
					ref_idx >>=1;

				build_weight_data_block_INTEL ARGS5(pred_dir, pData, ref_idx, ref_idx, list_offset);
				pData+=16;
				build_weight_data_block_INTEL ARGS5(pred_dir, pData, ref_idx, ref_idx, list_offset);
			}
			else
			{
				int ref_idx_0 = currMB_s_d->pred_info.ref_idx[LIST_0][k];
				int ref_idx_1 = currMB_s_d->pred_info.ref_idx[LIST_1][k];

				if (active_pps.weighted_bipred_idc==1 && curr_mb_field)
				{
					ref_idx_0 >>=1;
					ref_idx_1 >>=1;
				}

				build_weight_data_block_INTEL ARGS5(pred_dir, pData, ref_idx_0, ref_idx_1, list_offset);
				pData+=16;
				build_weight_data_block_INTEL ARGS5(pred_dir, pData, ref_idx_0, ref_idx_1, list_offset);
			}
			pData+=16;
		}
	}

	switch(block_index)
	{
	case 5:			// L0_L0_16x8
		Mb_partition->MbType5Bits = 4;
		break;
	case 10:		// L1_L1_16x8
		Mb_partition->MbType5Bits = 6;
		break;
	case 9:			// L0_L1_16x8
		Mb_partition->MbType5Bits = 8;
		break;
	case 6:			// L1_L0_16x8
		Mb_partition->MbType5Bits = 10;
		break;
	case 13:		// L0_Bi_16x8
		Mb_partition->MbType5Bits = 12;
		break;
	case 14:		// L1_Bi_16x8
		Mb_partition->MbType5Bits = 14;
		break;
	case 7:			// Bi_L0_16x8
		Mb_partition->MbType5Bits = 16;
		break;
	case 11:		// Bi_L1_16x8
		Mb_partition->MbType5Bits = 18;
		break;
	case 15:		// Bi_Bi_16x8
		Mb_partition->MbType5Bits = 20;
		break;
	default:
		Mb_partition->MbType5Bits = 0;
		break;
	}
}

void CH264DXVA2::build_macroblock_buffer_Inter_8x16 PARGS1(int list_offset)
{
	int pred_dir;
	int cbp=0, i4, b8, k, ref_picid, ref_picid_se, ref_bot, ref_bot_se;
	int curr_mb_field = ((IMGPAR MbaffFrameFlag)&&(currMB_s_d->mb_field));
	int current_mb_nr = IMGPAR current_mb_nr_d;
	int block_index = 0;

	LPDXVA_MBctrl_H264 Mb_partition = (LPDXVA_MBctrl_H264)(IMGPAR m_lpMBLK_Intra_Luma + IMGPAR m_iIntraMCBufUsage);
	memset(Mb_partition, 0, sizeof(DXVA_MBctrl_H264));
	LPDXVA_MVvalue Mv;
	if (m_bResidualDataFormat==E_RES_DATA_FORMAT_INTEL)
	{
		Mv = (LPDXVA_MVvalue)(IMGPAR m_lpRESD_Intra_Luma + IMGPAR m_iIntraRESBufUsage);
		memset(Mv, 0 ,192);
	}
	else
		Mv = (LPDXVA_MVvalue)(IMGPAR m_lpMV+ IMGPAR m_iInterMCBufUsage);

	Mb_partition->bSliceID		= IMGPAR current_slice_nr;
	//Mb_partition->IntraMbFlag	= 0;
	if(m_bResidualDataFormat==E_RES_DATA_FORMAT_INTEL)
		Mb_partition->mb_field_decoding_flag	= IMGPAR MbaffFrameFlag ? curr_mb_field : IMGPAR field_pic_flag;
	else
		Mb_partition->mb_field_decoding_flag	=curr_mb_field;//? IMGPAR MbaffFrameFlag : IMGPAR field_pic_flag;
	Mb_partition->transform_size_8x8_flag	=(m_bResidualDataFormat==E_RES_DATA_FORMAT_NV ? 1 : currMB_d->luma_transform_size_8x8_flag);
	Mb_partition->HostResidDiff = (m_nDXVAMode==E_H264_DXVA_MODE_A) ? 1 : 0;
	//Mb_partition->bMvQuantity = 0;
	Mb_partition->CurrMbAddr = IMGPAR current_mb_nr_d;
	Mb_partition->bQpPrime[0] = currMB_s_d->qp;
	Mb_partition->bQpPrime[1] = QP_SCALE_CR_Intel[Clip3(0, 51, currMB_s_d->qp + dec_picture->chroma_qp_offset[0])];
	Mb_partition->bQpPrime[2] = QP_SCALE_CR_Intel[Clip3(0, 51, currMB_s_d->qp + dec_picture->chroma_qp_offset[1])];

	if(m_nDXVAMode==E_H264_DXVA_MODE_A)
	{
		Mb_partition->wPatternCode[0] = getPatternCode ARGS1(Mb_partition->transform_size_8x8_flag);
		for(b8 = 3; b8 >= 0; b8--)
		{
			if(currMB_s_d->cbp_blk & (1<<(19-b8)))
				Mb_partition->wPatternCode[1] |= (1<<b8);
			if(currMB_s_d->cbp_blk & (1<<(23-b8)))
				Mb_partition->wPatternCode[2] |= (1<<b8);
		}
		Mb_partition->bMBresidDataQuantity = getMBresidDataQuantity ARGS3(currMB_s_d->cbp_blk, currMB_d->cbp, Mb_partition->transform_size_8x8_flag);
	}
	if(m_bResidualDataFormat==E_RES_DATA_FORMAT_INTEL)
		Mb_partition->bMBresidDataQuantity += 12;

	Mb_partition->dwMBdataLocation = (IMGPAR m_iIntraRESBufUsage>>2);
	Mb_partition->wMvBuffOffset = (IMGPAR m_iInterMCBufUsage>>2);

	for(k=0; k<2; k++)
	{
		i4 = (k<<1);
		pred_dir = currMB_d->b8pdir[k];
		block_index += ((pred_dir+1)<<(k*2));
		if(pred_dir != 2)
		{
			//ref_picid = currMB_s_d->pred_info.ref_pic_id[pred_dir][k];
			MotionVector *pMV = &(currMB_s_d->pred_info.mv[pred_dir][i4]);
			if(!curr_mb_field)
			{
				if(pred_dir)
				{
					Mb_partition->bRefPicSelect[LIST_1][k] = currMB_s_d->pred_info.ref_idx[pred_dir][k];
					Mb_partition->bRefPicSelect[LIST_0][k] = 0xff;
				}
				else
				{
					Mb_partition->bRefPicSelect[LIST_0][k] = currMB_s_d->pred_info.ref_idx[pred_dir][k];
					Mb_partition->bRefPicSelect[LIST_1][k] = 0xff;
				}
			}
			else
			{
				ref_picid = currMB_s_d->pred_info.ref_pic_id[pred_dir][k];
				ref_bot = m_pSurfaceInfo[ref_picid].SInfo[1];
				if(current_mb_nr&1)
				{
					if(pred_dir)
					{
						Mb_partition->bRefPicSelect[LIST_1][k] = (IMGPAR m_BotL1Map[currMB_s_d->pred_info.ref_idx[pred_dir][k]]<<1);
						Mb_partition->bRefPicSelect[LIST_1][k] |= (ref_bot ? 0 : 1);
						Mb_partition->bRefPicSelect[LIST_0][k] = 0xff;
					}
					else
					{
						Mb_partition->bRefPicSelect[LIST_0][k] = (IMGPAR m_BotL0Map[currMB_s_d->pred_info.ref_idx[pred_dir][k]]<<1);
						Mb_partition->bRefPicSelect[LIST_0][k] |= (ref_bot ? 0 : 1);
						Mb_partition->bRefPicSelect[LIST_1][k] = 0xff;
					}
				}
				else
				{
					if(pred_dir)
					{
						Mb_partition->bRefPicSelect[LIST_1][k] = (IMGPAR m_TopL1Map[currMB_s_d->pred_info.ref_idx[pred_dir][k]]<<1);
						Mb_partition->bRefPicSelect[LIST_1][k] |= (ref_bot ? 1 : 0);
						Mb_partition->bRefPicSelect[LIST_0][k] = 0xff;
					}
					else
					{
						Mb_partition->bRefPicSelect[LIST_0][k] = (IMGPAR m_TopL0Map[currMB_s_d->pred_info.ref_idx[pred_dir][k]]<<1);
						Mb_partition->bRefPicSelect[LIST_0][k] |= (ref_bot ? 1 : 0);
						Mb_partition->bRefPicSelect[LIST_1][k] = 0xff;
					}
				}
			}
			Mv->horz = pMV->x;
			Mv->vert = pMV->y;
			Mv++;
			Mb_partition->bMvQuantity++;
			IMGPAR m_iInterMCBufUsage += sizeof(DXVA_MVvalue);
			if (m_bResidualDataFormat==E_RES_DATA_FORMAT_INTEL) // duplicate Mv
			{
				Mv->horz = pMV->x;
				Mv->vert = pMV->y;
				//Mv++;
				Mv+=3; // 0 1   For first block (k=0), duplicate the Mv to location 2 including L0 and L1
					   // 2 3   for second block (k=1), duplicate the Mv to location 3
				//L0
				Mv->horz = pMV->x;
				Mv->vert = pMV->y;
				Mv++;
				//L1
				Mv->horz = pMV->x;
				Mv->vert = pMV->y;
				//Mv++;
				(k==0) ? Mv-=3 : Mv++;  //if k=0, return the Mv pointer to location 1 for next block
				Mb_partition->bMvQuantity+=3;
				IMGPAR m_iInterMCBufUsage += sizeof(DXVA_MVvalue)*3;
			}
		}
		else
		{
			//ref_picid = currMB_s_d->pred_info.ref_pic_id[LIST_0][k];
			//ref_picid_se = currMB_s_d->pred_info.ref_pic_id[LIST_1][k];
			MotionVector *pMV0 = &(currMB_s_d->pred_info.mv[LIST_0][i4]);
			MotionVector *pMV1 = &(currMB_s_d->pred_info.mv[LIST_1][i4]);
			if(!curr_mb_field)
			{
				Mb_partition->bRefPicSelect[LIST_0][k] = currMB_s_d->pred_info.ref_idx[LIST_0][k];
				Mb_partition->bRefPicSelect[LIST_1][k] = currMB_s_d->pred_info.ref_idx[LIST_1][k];				
			}
			else
			{
				ref_picid = currMB_s_d->pred_info.ref_pic_id[LIST_0][k];
				ref_picid_se = currMB_s_d->pred_info.ref_pic_id[LIST_1][k];
				ref_bot = m_pSurfaceInfo[ref_picid].SInfo[1];
				ref_bot_se = m_pSurfaceInfo[ref_picid_se].SInfo[1];
				if(current_mb_nr&1)
				{
					Mb_partition->bRefPicSelect[LIST_0][k] = (IMGPAR m_BotL0Map[currMB_s_d->pred_info.ref_idx[LIST_0][k]]<<1);
					Mb_partition->bRefPicSelect[LIST_0][k] |= (ref_bot ? 0 : 1);
					Mb_partition->bRefPicSelect[LIST_1][k] = (IMGPAR m_BotL1Map[currMB_s_d->pred_info.ref_idx[LIST_1][k]]<<1);
					Mb_partition->bRefPicSelect[LIST_1][k] |= (ref_bot_se ? 0 : 1);
				}
				else
				{
					Mb_partition->bRefPicSelect[LIST_0][k] = (IMGPAR m_TopL0Map[currMB_s_d->pred_info.ref_idx[LIST_0][k]]<<1);
					Mb_partition->bRefPicSelect[LIST_0][k] |= (ref_bot ? 1 : 0);
					Mb_partition->bRefPicSelect[LIST_1][k] = (IMGPAR m_TopL1Map[currMB_s_d->pred_info.ref_idx[LIST_1][k]]<<1);
					Mb_partition->bRefPicSelect[LIST_1][k] |= (ref_bot_se ? 1 : 0);
				}
			}
			Mv->horz = pMV0->x;
			Mv->vert = pMV0->y;
			Mv++;
			Mv->horz = pMV1->x;
			Mv->vert = pMV1->y;
			Mv++;
			Mb_partition->bMvQuantity += 2;
			IMGPAR m_iInterMCBufUsage += sizeof(DXVA_MVvalue)*2;
			if (m_bResidualDataFormat==E_RES_DATA_FORMAT_INTEL) // duplicate Mv
			{
				Mv+=2; // 0 1   For first block (k=0), duplicate the Mv to location 2 including L0 and L1
					   // 2 3   for second block (k=1), duplicate the Mv to location 3
				//L0
				Mv->horz = pMV0->x;
				Mv->vert = pMV0->y;
				Mv++;
				//L1
				Mv->horz = pMV1->x;
				Mv->vert = pMV1->y;
				//Mv++;
				(k==0) ? Mv-=3 : Mv++;  //if k=0, return the Mv pointer to location 1 for next block
				Mb_partition->bMvQuantity += 2;
				IMGPAR m_iInterMCBufUsage += sizeof(DXVA_MVvalue)*2;
			}		
		}		
	}

	//build weight/offset data block
	if (m_bResidualDataFormat==E_RES_DATA_FORMAT_INTEL && IMGPAR apply_weights)
	{
		//Mv+=24; //24 n/a DWords  //Marked to Pack WO data right after MV data
		char *pData = (char*)Mv;
		for(k=0; k<2; k++)
		{
			pred_dir = currMB_d->b8pdir[k];
			if(pred_dir != 2)
			{
				int ref_idx = currMB_s_d->pred_info.ref_idx[pred_dir][k];

				if (active_pps.weighted_pred_flag && curr_mb_field)
					ref_idx >>=1;

				build_weight_data_block_INTEL ARGS5(pred_dir, pData, ref_idx, ref_idx, list_offset);
				pData+=32;
				build_weight_data_block_INTEL ARGS5(pred_dir, pData, ref_idx, ref_idx, list_offset);
			}		
			else
			{
				int ref_idx_0 = currMB_s_d->pred_info.ref_idx[LIST_0][k];
				int ref_idx_1 = currMB_s_d->pred_info.ref_idx[LIST_1][k];

				if (active_pps.weighted_bipred_idc==1 && curr_mb_field)
				{
					ref_idx_0 >>=1;
					ref_idx_1 >>=1;
				}

				build_weight_data_block_INTEL ARGS5(pred_dir, pData, ref_idx_0, ref_idx_1, list_offset);
				pData+=32;
				build_weight_data_block_INTEL ARGS5(pred_dir, pData, ref_idx_0, ref_idx_1, list_offset);
			}		
			pData-=16;
		}
	}

	switch(block_index)
	{
	case 5:			// L0_L0_8x16
		Mb_partition->MbType5Bits = 5;
		break;
	case 10:		// L1_L1_8x16
		Mb_partition->MbType5Bits = 7;
		break;
	case 9:			// L0_L1_8x16
		Mb_partition->MbType5Bits = 9;
		break;
	case 6:			// L1_L0_8x16
		Mb_partition->MbType5Bits = 11;
		break;
	case 13:		// L0_Bi_8x16
		Mb_partition->MbType5Bits = 13;
		break;
	case 14:		// L1_Bi_8x16
		Mb_partition->MbType5Bits = 15;
		break;
	case 7:			// Bi_L0_8x16
		Mb_partition->MbType5Bits = 17;
		break;
	case 11:		// Bi_L1_8x16
		Mb_partition->MbType5Bits = 19;
		break;
	case 15:		// Bi_Bi_8x16
		Mb_partition->MbType5Bits = 21;
		break;
	default:
		Mb_partition->MbType5Bits = 0;
		break;
	}
}

void CH264DXVA2::build_macroblock_buffer_Inter_8x8 PARGS1(int list_offset)
{
	int i, b4, b8, ref_picid, ref_picid_se, ref_bot, ref_bot_se;
	int pred_dir,mode;
	int fw_refframe,bw_refframe;
	static const int b84_idx[4][2] = { {0,4} , {2,6} , {8,12} , {10,14} };
	static const int b48_idx[4][2] = { {0,1} , {2,3} , {8,9} , {10,11} };
	static const int b44_idx[4][4] = { {0,1,4,5} , {2,3,6,7} , {8,9,12,13} , {10,11,14,15} };
	int curr_mb_field = ((IMGPAR MbaffFrameFlag)&&(currMB_s_d->mb_field));
	int current_mb_nr = IMGPAR current_mb_nr_d;

	BOOL Has_Sub_Partition;
	if(m_bResidualDataFormat==E_RES_DATA_FORMAT_INTEL)
	{
		//For Intel, 8 MV is used without sub-partition, 16 MV (P MB) or 32 MV (B MB) is used with sub-partition.
		Has_Sub_Partition = !((currMB_d->b8mode[0]==PB_8x8 || currMB_d->b8mode[0]==0) && (currMB_d->b8mode[1]==PB_8x8 || currMB_d->b8mode[1]==0)
			&& (currMB_d->b8mode[2]==PB_8x8 || currMB_d->b8mode[2]==0) && (currMB_d->b8mode[3]==PB_8x8 || currMB_d->b8mode[3]==0));
	}
	
	LPDXVA_MBctrl_H264 Mb_partition = (LPDXVA_MBctrl_H264)(IMGPAR m_lpMBLK_Intra_Luma + IMGPAR m_iIntraMCBufUsage);
	memset(Mb_partition, 0, sizeof(DXVA_MBctrl_H264));
	LPDXVA_MVvalue Mv;
	if (m_bResidualDataFormat==E_RES_DATA_FORMAT_INTEL)
	{
		Mv = (LPDXVA_MVvalue)(IMGPAR m_lpRESD_Intra_Luma + IMGPAR m_iIntraRESBufUsage);
		memset(Mv, 0 ,192);
	}
	else
		Mv = (LPDXVA_MVvalue)(IMGPAR m_lpMV+ IMGPAR m_iInterMCBufUsage);

	Mb_partition->bSliceID		= IMGPAR current_slice_nr;
	Mb_partition->MbType5Bits	= 22;	// B_8x8
	//Mb_partition->IntraMbFlag	= 0;
	if(m_bResidualDataFormat==E_RES_DATA_FORMAT_INTEL)
		Mb_partition->mb_field_decoding_flag	= IMGPAR MbaffFrameFlag ? curr_mb_field : IMGPAR field_pic_flag;
	else
		Mb_partition->mb_field_decoding_flag	= curr_mb_field;// ? IMGPAR MbaffFrameFlag : IMGPAR field_pic_flag;
	Mb_partition->transform_size_8x8_flag	= (m_bResidualDataFormat==E_RES_DATA_FORMAT_NV ? 1 : currMB_d->luma_transform_size_8x8_flag);
	Mb_partition->HostResidDiff = (m_nDXVAMode==E_H264_DXVA_MODE_A) ? 1 : 0;
	//Mb_partition->bMvQuantity = 0;
	Mb_partition->CurrMbAddr = IMGPAR current_mb_nr_d;
	Mb_partition->bQpPrime[0] = currMB_s_d->qp;
	Mb_partition->bQpPrime[1] = QP_SCALE_CR_Intel[Clip3(0, 51, currMB_s_d->qp + dec_picture->chroma_qp_offset[0])];
	Mb_partition->bQpPrime[2] = QP_SCALE_CR_Intel[Clip3(0, 51, currMB_s_d->qp + dec_picture->chroma_qp_offset[1])];

	if(m_nDXVAMode==E_H264_DXVA_MODE_A)
	{
		Mb_partition->wPatternCode[0] = getPatternCode ARGS1(Mb_partition->transform_size_8x8_flag);		
		for(b8 = 3; b8 >= 0; b8--){
			if(currMB_s_d->cbp_blk & (1<<(19-b8)))
				Mb_partition->wPatternCode[1] |= (1<<b8);
			if(currMB_s_d->cbp_blk & (1<<(23-b8)))
				Mb_partition->wPatternCode[2] |= (1<<b8);
		}
		Mb_partition->bMBresidDataQuantity = getMBresidDataQuantity ARGS3(currMB_s_d->cbp_blk, currMB_d->cbp, Mb_partition->transform_size_8x8_flag);
	}
	if(m_bResidualDataFormat==E_RES_DATA_FORMAT_INTEL)
		Mb_partition->bMBresidDataQuantity += 12;

	Mb_partition->dwMBdataLocation = (IMGPAR m_iIntraRESBufUsage>>2);
	Mb_partition->wMvBuffOffset = (IMGPAR m_iInterMCBufUsage>>2);
	//Mb_partition->bSubMbShapes = 0;
	//Mb_partition->bSubMbPredModes = 0;

	//Yolk: Merge to one MB
	byte bNotCombineMB = (m_bResidualDataFormat!=E_RES_DATA_FORMAT_NV);
	bNotCombineMB |= currMB_d->b8mode[0];
	bNotCombineMB |= (currMB_d->b8mode[0]!=currMB_d->b8mode[1]);
	bNotCombineMB |= (currMB_d->b8mode[1]!=currMB_d->b8mode[2]);
	bNotCombineMB |= (currMB_d->b8mode[2]!=currMB_d->b8mode[3]);
	bNotCombineMB |= (currMB_d->b8pdir[0]!=currMB_d->b8pdir[1]);
	bNotCombineMB |= (currMB_d->b8pdir[1]!=currMB_d->b8pdir[2]);
	bNotCombineMB |= (currMB_d->b8pdir[2]!=currMB_d->b8pdir[3]);
	bNotCombineMB |= (IMGPAR type!=B_SLICE);
	if (!bNotCombineMB)
	{
		if (IMGPAR direct_spatial_mv_pred_flag) //equal to direct_flag is true in uv loop
		{
			fw_refframe = currMB_s_d->pred_info.ref_idx[LIST_0][0];
			bw_refframe = currMB_s_d->pred_info.ref_idx[LIST_1][0];
			//direct spatial
			if (active_sps.direct_8x8_inference_flag)
			{
				mode=PB_8x8;
				if (bw_refframe==-1)
					pred_dir=0;
				else if (fw_refframe==-1)
					pred_dir=1;
				else
					pred_dir=2;
			}
			else
			{
				mode=PB_4x4;
				if (bw_refframe==-1)
					pred_dir=0;
				else if (fw_refframe==-1)
					pred_dir=1;
				else
					pred_dir=2;
			}
		}
		else
		{
			pred_dir=2;
			//direct temporal
			if (currMB_d->luma_transform_size_8x8_flag)
				mode=PB_8x8;
			else
				mode=PB_4x4;
		}

		if (pred_dir!=2)
		{
			//bNotCombineMB |= (fast_abs_short(currMB_s_d->pred_info.mv[pred_dir][b8_idx[0]].x - currMB_s_d->pred_info.mv[pred_dir][b8_idx[1]].x) != 0);
			//bNotCombineMB |= (fast_abs_short(currMB_s_d->pred_info.mv[pred_dir][b8_idx[0]].y - currMB_s_d->pred_info.mv[pred_dir][b8_idx[1]].y) != 0);
			//bNotCombineMB |= (fast_abs_short(currMB_s_d->pred_info.mv[pred_dir][b8_idx[1]].x - currMB_s_d->pred_info.mv[pred_dir][b8_idx[2]].x) != 0);
			//bNotCombineMB |= (fast_abs_short(currMB_s_d->pred_info.mv[pred_dir][b8_idx[1]].y - currMB_s_d->pred_info.mv[pred_dir][b8_idx[2]].y) != 0);
			//bNotCombineMB |= (fast_abs_short(currMB_s_d->pred_info.mv[pred_dir][b8_idx[2]].x - currMB_s_d->pred_info.mv[pred_dir][b8_idx[3]].x) != 0);
			//bNotCombineMB |= (fast_abs_short(currMB_s_d->pred_info.mv[pred_dir][b8_idx[2]].y - currMB_s_d->pred_info.mv[pred_dir][b8_idx[3]].y) != 0);
			bNotCombineMB |= (currMB_s_d->pred_info.mv[pred_dir][b8_idx[0]].x != currMB_s_d->pred_info.mv[pred_dir][b8_idx[1]].x);
			bNotCombineMB |= (currMB_s_d->pred_info.mv[pred_dir][b8_idx[0]].y != currMB_s_d->pred_info.mv[pred_dir][b8_idx[1]].y);
			bNotCombineMB |= (currMB_s_d->pred_info.mv[pred_dir][b8_idx[1]].x != currMB_s_d->pred_info.mv[pred_dir][b8_idx[2]].x);
			bNotCombineMB |= (currMB_s_d->pred_info.mv[pred_dir][b8_idx[1]].y != currMB_s_d->pred_info.mv[pred_dir][b8_idx[2]].y);
			bNotCombineMB |= (currMB_s_d->pred_info.mv[pred_dir][b8_idx[2]].x != currMB_s_d->pred_info.mv[pred_dir][b8_idx[3]].x);
			bNotCombineMB |= (currMB_s_d->pred_info.mv[pred_dir][b8_idx[2]].y != currMB_s_d->pred_info.mv[pred_dir][b8_idx[3]].y);
		}
		else
		{
			//bNotCombineMB |= (fast_abs_short(currMB_s_d->pred_info.mv[LIST_0][b8_idx[0]].x - currMB_s_d->pred_info.mv[LIST_0][b8_idx[1]].x) != 0);
			//bNotCombineMB |= (fast_abs_short(currMB_s_d->pred_info.mv[LIST_0][b8_idx[0]].y - currMB_s_d->pred_info.mv[LIST_0][b8_idx[1]].y) != 0);
			//bNotCombineMB |= (fast_abs_short(currMB_s_d->pred_info.mv[LIST_0][b8_idx[1]].x - currMB_s_d->pred_info.mv[LIST_0][b8_idx[2]].x) != 0);
			//bNotCombineMB |= (fast_abs_short(currMB_s_d->pred_info.mv[LIST_0][b8_idx[1]].y - currMB_s_d->pred_info.mv[LIST_0][b8_idx[2]].y) != 0);
			//bNotCombineMB |= (fast_abs_short(currMB_s_d->pred_info.mv[LIST_0][b8_idx[2]].x - currMB_s_d->pred_info.mv[LIST_0][b8_idx[3]].x) != 0);
			//bNotCombineMB |= (fast_abs_short(currMB_s_d->pred_info.mv[LIST_0][b8_idx[2]].y - currMB_s_d->pred_info.mv[LIST_0][b8_idx[3]].y) != 0);
			//bNotCombineMB |= (fast_abs_short(currMB_s_d->pred_info.mv[LIST_1][b8_idx[0]].x - currMB_s_d->pred_info.mv[LIST_1][b8_idx[1]].x) != 0);
			//bNotCombineMB |= (fast_abs_short(currMB_s_d->pred_info.mv[LIST_1][b8_idx[0]].y - currMB_s_d->pred_info.mv[LIST_1][b8_idx[1]].y) != 0);
			//bNotCombineMB |= (fast_abs_short(currMB_s_d->pred_info.mv[LIST_1][b8_idx[1]].x - currMB_s_d->pred_info.mv[LIST_1][b8_idx[2]].x) != 0);
			//bNotCombineMB |= (fast_abs_short(currMB_s_d->pred_info.mv[LIST_1][b8_idx[1]].y - currMB_s_d->pred_info.mv[LIST_1][b8_idx[2]].y) != 0);
			//bNotCombineMB |= (fast_abs_short(currMB_s_d->pred_info.mv[LIST_1][b8_idx[2]].x - currMB_s_d->pred_info.mv[LIST_1][b8_idx[3]].x) != 0);
			//bNotCombineMB |= (fast_abs_short(currMB_s_d->pred_info.mv[LIST_1][b8_idx[2]].y - currMB_s_d->pred_info.mv[LIST_1][b8_idx[3]].y) != 0);
			bNotCombineMB |= (currMB_s_d->pred_info.mv[LIST_0][b8_idx[0]].x != currMB_s_d->pred_info.mv[LIST_0][b8_idx[1]].x);
			bNotCombineMB |= (currMB_s_d->pred_info.mv[LIST_0][b8_idx[0]].y != currMB_s_d->pred_info.mv[LIST_0][b8_idx[1]].y);
			bNotCombineMB |= (currMB_s_d->pred_info.mv[LIST_0][b8_idx[1]].x != currMB_s_d->pred_info.mv[LIST_0][b8_idx[2]].x);
			bNotCombineMB |= (currMB_s_d->pred_info.mv[LIST_0][b8_idx[1]].y != currMB_s_d->pred_info.mv[LIST_0][b8_idx[2]].y);
			bNotCombineMB |= (currMB_s_d->pred_info.mv[LIST_0][b8_idx[2]].x != currMB_s_d->pred_info.mv[LIST_0][b8_idx[3]].x);
			bNotCombineMB |= (currMB_s_d->pred_info.mv[LIST_0][b8_idx[2]].y != currMB_s_d->pred_info.mv[LIST_0][b8_idx[3]].y);
			bNotCombineMB |= (currMB_s_d->pred_info.mv[LIST_1][b8_idx[0]].x != currMB_s_d->pred_info.mv[LIST_1][b8_idx[1]].x);
			bNotCombineMB |= (currMB_s_d->pred_info.mv[LIST_1][b8_idx[0]].y != currMB_s_d->pred_info.mv[LIST_1][b8_idx[1]].y);
			bNotCombineMB |= (currMB_s_d->pred_info.mv[LIST_1][b8_idx[1]].x != currMB_s_d->pred_info.mv[LIST_1][b8_idx[2]].x);
			bNotCombineMB |= (currMB_s_d->pred_info.mv[LIST_1][b8_idx[1]].y != currMB_s_d->pred_info.mv[LIST_1][b8_idx[2]].y);
			bNotCombineMB |= (currMB_s_d->pred_info.mv[LIST_1][b8_idx[2]].x != currMB_s_d->pred_info.mv[LIST_1][b8_idx[3]].x);
			bNotCombineMB |= (currMB_s_d->pred_info.mv[LIST_1][b8_idx[2]].y != currMB_s_d->pred_info.mv[LIST_1][b8_idx[3]].y);
		}
	}

	if(m_bResidualDataFormat==E_RES_DATA_FORMAT_INTEL)  //Has_Sub_Partition must be set before fill each 8x8 Mv, set the Has_Sub_Partition for direct mode
	{
		for (b8=0; b8<(bNotCombineMB ? 4:1); b8++)
		{
			if (bNotCombineMB)
			{
				//pred_dir = currMB_d->b8pdir[b8];
				mode = currMB_d->b8mode[b8];

				if(mode==0 && IMGPAR type == B_SLICE)
				{
					if (!active_sps.direct_8x8_inference_flag)
					{
						Has_Sub_Partition = TRUE;  //mode=PB_4x4
						break;
					}
				}
			}
		}
	}
	
	int iPredDirSet[4];
	for (b8=0; b8<(bNotCombineMB ? 4:1); b8++)
	{
		if (bNotCombineMB)
		{
			pred_dir = currMB_d->b8pdir[b8];
			mode = currMB_d->b8mode[b8];

			if(mode==0 && IMGPAR type == B_SLICE)
			{
				if (IMGPAR direct_spatial_mv_pred_flag) //equal to direct_flag is true in uv loop
				{
					fw_refframe = currMB_s_d->pred_info.ref_idx[LIST_0][b8];
					bw_refframe = currMB_s_d->pred_info.ref_idx[LIST_1][b8];
					//direct spatial
					if (active_sps.direct_8x8_inference_flag)
					{
						mode=PB_8x8;
						if (bw_refframe==-1)
							pred_dir=0;
						else if (fw_refframe==-1)
							pred_dir=1;
						else
							pred_dir=2;
					}
					else
					{
						mode=PB_4x4;
						if (bw_refframe==-1)
							pred_dir=0;
						else if (fw_refframe==-1)
							pred_dir=1;
						else
							pred_dir=2;
					}
				}
				else
				{
					pred_dir=2;
					//direct temporal
					if(active_sps.direct_8x8_inference_flag)
						mode=PB_8x8;
					else
						mode=PB_4x4;
				}
			}
		}
		else
			Mb_partition->MbType5Bits = pred_dir+1;

		Mb_partition->bSubMbPredModes |= (pred_dir << (b8<<1));
		if(pred_dir != 2)
		{
			if(!curr_mb_field)
			{
				if(pred_dir)
				{
					Mb_partition->bRefPicSelect[LIST_1][b8] = currMB_s_d->pred_info.ref_idx[pred_dir][b8];
					Mb_partition->bRefPicSelect[LIST_0][b8] = 0xff;
				}
				else
				{
					Mb_partition->bRefPicSelect[LIST_0][b8] = currMB_s_d->pred_info.ref_idx[pred_dir][b8];
					Mb_partition->bRefPicSelect[LIST_1][b8] = 0xff;
				}
			}
			else
			{
				ref_picid = currMB_s_d->pred_info.ref_pic_id[pred_dir][b8];
				ref_bot = m_pSurfaceInfo[ref_picid].SInfo[1];
				if(current_mb_nr&1)
				{
					if(pred_dir)
					{
						Mb_partition->bRefPicSelect[LIST_1][b8] = (IMGPAR m_BotL1Map[currMB_s_d->pred_info.ref_idx[pred_dir][b8]]<<1);
						Mb_partition->bRefPicSelect[LIST_1][b8] |= (ref_bot ? 0 : 1);
						Mb_partition->bRefPicSelect[LIST_0][b8] = 0xff;
					}
					else
					{
						Mb_partition->bRefPicSelect[LIST_0][b8] = (IMGPAR m_BotL0Map[currMB_s_d->pred_info.ref_idx[pred_dir][b8]]<<1);
						Mb_partition->bRefPicSelect[LIST_0][b8] |= (ref_bot ? 0 : 1);
						Mb_partition->bRefPicSelect[LIST_1][b8] = 0xff;
					}
				}
				else
				{
					if(pred_dir)
					{
						Mb_partition->bRefPicSelect[LIST_1][b8] = (IMGPAR m_TopL1Map[currMB_s_d->pred_info.ref_idx[pred_dir][b8]]<<1);
						Mb_partition->bRefPicSelect[LIST_1][b8] |= (ref_bot ? 1 : 0);
						Mb_partition->bRefPicSelect[LIST_0][b8] = 0xff;
					}
					else
					{
						Mb_partition->bRefPicSelect[LIST_0][b8] = (IMGPAR m_TopL0Map[currMB_s_d->pred_info.ref_idx[pred_dir][b8]]<<1);
						Mb_partition->bRefPicSelect[LIST_0][b8] |= (ref_bot ? 1 : 0);
						Mb_partition->bRefPicSelect[LIST_1][b8] = 0xff;
					}
				}
			}
		}
		else
		{
			if(!curr_mb_field)
			{
				Mb_partition->bRefPicSelect[LIST_0][b8] = currMB_s_d->pred_info.ref_idx[LIST_0][b8];
				Mb_partition->bRefPicSelect[LIST_1][b8] = currMB_s_d->pred_info.ref_idx[LIST_1][b8];
			}
			else
			{
				ref_picid = currMB_s_d->pred_info.ref_pic_id[LIST_0][b8];
				ref_picid_se = currMB_s_d->pred_info.ref_pic_id[LIST_1][b8];
				ref_bot = m_pSurfaceInfo[ref_picid].SInfo[1];
				ref_bot_se = m_pSurfaceInfo[ref_picid_se].SInfo[1];
				if(current_mb_nr&1)
				{
					Mb_partition->bRefPicSelect[LIST_0][b8] = (IMGPAR m_BotL0Map[currMB_s_d->pred_info.ref_idx[LIST_0][b8]]<<1);
					Mb_partition->bRefPicSelect[LIST_0][b8] |= (ref_bot ? 0 : 1);
					Mb_partition->bRefPicSelect[LIST_1][b8] = (IMGPAR m_BotL1Map[currMB_s_d->pred_info.ref_idx[LIST_1][b8]]<<1);
					Mb_partition->bRefPicSelect[LIST_1][b8] |= (ref_bot_se ? 0 : 1);
				}
				else
				{
					Mb_partition->bRefPicSelect[LIST_0][b8] = (IMGPAR m_TopL0Map[currMB_s_d->pred_info.ref_idx[LIST_0][b8]]<<1);
					Mb_partition->bRefPicSelect[LIST_0][b8] |= (ref_bot ? 1 : 0);
					Mb_partition->bRefPicSelect[LIST_1][b8] = (IMGPAR m_TopL1Map[currMB_s_d->pred_info.ref_idx[LIST_1][b8]]<<1);
					Mb_partition->bRefPicSelect[LIST_1][b8] |= (ref_bot_se ? 1 : 0);
				}
			}
		}

		iPredDirSet[b8] = pred_dir;
		if(mode==PB_8x8 || mode==0)	//8x8
		{
			Mb_partition->bSubMbShapes |= (0x00 << (b8*2));
			if(pred_dir != 2)
			{
				SHORT sHorz, sVert;
				sHorz = Mv->horz = currMB_s_d->pred_info.mv[pred_dir][b8_idx[b8]].x;
				sVert = Mv->vert = currMB_s_d->pred_info.mv[pred_dir][b8_idx[b8]].y;
				Mv++;
				Mb_partition->bMvQuantity++;
				IMGPAR m_iInterMCBufUsage += sizeof(DXVA_MVvalue);

				if (m_bResidualDataFormat==E_RES_DATA_FORMAT_INTEL)
				{
					Mv->horz = sHorz;
					Mv->vert = sVert;
					Mv++;
					Mb_partition->bMvQuantity++;
					IMGPAR m_iInterMCBufUsage += sizeof(DXVA_MVvalue);

					if(Has_Sub_Partition) //16 MV or 32 MV, else 8 MV
					{
						//if(IMGPAR type==P_SLICE) //P MB (16 MV)                  //Mark for Intel change 16MV to 32 MV
						//{
						//	for (int j=0;j<2;j++)
						//	{
						//		Mv->horz = sHorz;
						//		Mv->vert = sVert;
						//		Mv++;
						//		Mb_partition->bMvQuantity++;
						//		IMGPAR m_iInterMCBufUsage += sizeof(DXVA_MVvalue);
						//	}
						//} 
						//else //B MB (32 MV)
						{
							for (int j=0;j<6;j++)
							{
								Mv->horz = sHorz;
								Mv->vert = sVert;
								Mv++;
								Mb_partition->bMvQuantity++;
								IMGPAR m_iInterMCBufUsage += sizeof(DXVA_MVvalue);
							}
						}
					}
				}
			}
			else
			{
				SHORT sHorz_L0, sVert_L0, sHorz_L1, sVert_L1;
				//ref_picid = currMB_s_d->pred_info.ref_pic_id[LIST_0][b8];
				//ref_picid_se = currMB_s_d->pred_info.ref_pic_id[LIST_1][b8];
				Mv->horz = sHorz_L0 = currMB_s_d->pred_info.mv[LIST_0][b8_idx[b8]].x;
				Mv->vert = sVert_L0 = currMB_s_d->pred_info.mv[LIST_0][b8_idx[b8]].y;
				Mv++;
				Mv->horz = sHorz_L1 = currMB_s_d->pred_info.mv[LIST_1][b8_idx[b8]].x;
				Mv->vert = sVert_L1 = currMB_s_d->pred_info.mv[LIST_1][b8_idx[b8]].y;
				Mv++;
				Mb_partition->bMvQuantity +=2;
				IMGPAR m_iInterMCBufUsage += sizeof(DXVA_MVvalue)*2;

				if (m_bResidualDataFormat==E_RES_DATA_FORMAT_INTEL && Has_Sub_Partition) //B Mb (32 MV), else 8 MV
				{
					for (int j=0;j<3;j++)
					{
						Mv->horz = sHorz_L0;
						Mv->vert = sVert_L0;
						Mv++;
						Mb_partition->bMvQuantity++;
						IMGPAR m_iInterMCBufUsage += sizeof(DXVA_MVvalue);

						Mv->horz = sHorz_L1;
						Mv->vert = sVert_L1;
						Mv++;
						Mb_partition->bMvQuantity++;
						IMGPAR m_iInterMCBufUsage += sizeof(DXVA_MVvalue);
					}
				}
			}
		}			
		else if(mode==PB_8x4)	//8x4
		{
			Mb_partition->bSubMbShapes |= (0x01 << (b8*2));
			for(i=0 ; i<2 ; i++)
			{
				if(pred_dir != 2)
				{
					SHORT sHorz, sVert;
					sHorz = Mv->horz = currMB_s_d->pred_info.mv[pred_dir][b84_idx[b8][i]].x;
					sVert = Mv->vert = currMB_s_d->pred_info.mv[pred_dir][b84_idx[b8][i]].y;
					Mv++;
					Mb_partition->bMvQuantity++;
					IMGPAR m_iInterMCBufUsage += sizeof(DXVA_MVvalue);

					if (m_bResidualDataFormat==E_RES_DATA_FORMAT_INTEL)
					{
						Mv->horz = sHorz;
						Mv->vert = sVert;
						Mv++;
						Mb_partition->bMvQuantity++;
						IMGPAR m_iInterMCBufUsage += sizeof(DXVA_MVvalue);

						//if(IMGPAR type==B_SLICE)//B Mb (32 MV), else P MB (16 MV)        //Mark for Intel change 16MV to 32 MV
						//{
							Mv->horz = sHorz;
							Mv->vert = sVert;
							Mv++;
							Mb_partition->bMvQuantity++;
							IMGPAR m_iInterMCBufUsage += sizeof(DXVA_MVvalue);

							Mv->horz = sHorz;
							Mv->vert = sVert;
							Mv++;
							Mb_partition->bMvQuantity ++;
							IMGPAR m_iInterMCBufUsage += sizeof(DXVA_MVvalue);
						//}
					}
				}
				else
				{
					SHORT sHorz0, sVert0, sHorz1, sVert1;
					sHorz0 = Mv->horz = currMB_s_d->pred_info.mv[LIST_0][b84_idx[b8][i]].x;
					sVert0 = Mv->vert = currMB_s_d->pred_info.mv[LIST_0][b84_idx[b8][i]].y;
					Mv++;
					sHorz1 = Mv->horz = currMB_s_d->pred_info.mv[LIST_1][b84_idx[b8][i]].x;
					sVert1 = Mv->vert = currMB_s_d->pred_info.mv[LIST_1][b84_idx[b8][i]].y;
					Mv++;
					Mb_partition->bMvQuantity +=2;
					IMGPAR m_iInterMCBufUsage += sizeof(DXVA_MVvalue)*2;

					if (m_bResidualDataFormat==E_RES_DATA_FORMAT_INTEL)
					{
						Mv->horz = sHorz0;
						Mv->vert = sVert0;
						Mv++;
						Mv->horz = sHorz1;
						Mv->vert = sVert1;
						Mv++;
						Mb_partition->bMvQuantity +=2;
						IMGPAR m_iInterMCBufUsage += sizeof(DXVA_MVvalue)*2;
					}
				}
			}
		}
		else if(mode==PB_4x8)	//4x8
		{
			Mb_partition->bSubMbShapes |= (0x02 << (b8*2));
			for(i=0 ; i<2 ; i++)
			{
				if(pred_dir != 2)
				{
					SHORT sHorz, sVert;
					sHorz = Mv->horz = currMB_s_d->pred_info.mv[pred_dir][b48_idx[b8][i]].x;
					sVert = Mv->vert = currMB_s_d->pred_info.mv[pred_dir][b48_idx[b8][i]].y;
					Mv++;
					Mb_partition->bMvQuantity++;
					IMGPAR m_iInterMCBufUsage += sizeof(DXVA_MVvalue);

					if (m_bResidualDataFormat==E_RES_DATA_FORMAT_INTEL)
					{
						//if (IMGPAR type==P_SLICE) //P MB (16 MV)            //Mark for Intel change 16MV to 32 MV
						//{
						//	Mv++;
						//	Mv->horz = sHorz;
						//	Mv->vert = sVert;
						//	(i==0) ? Mv-- : Mv++;
						//	Mb_partition->bMvQuantity++;
						//	IMGPAR m_iInterMCBufUsage += sizeof(DXVA_MVvalue);
						//}
						//else //B MB (32 MV)
						{
							Mv->horz = sHorz;
							Mv->vert = sVert;
							Mv+=3;
							Mb_partition->bMvQuantity++;
							IMGPAR m_iInterMCBufUsage += sizeof(DXVA_MVvalue);

							Mv->horz = sHorz;
							Mv->vert = sVert;
							Mv++;
							Mv->horz = sHorz;
							Mv->vert = sVert;
							(i==0) ? Mv-=3 : Mv++;
							Mb_partition->bMvQuantity+=2;
							IMGPAR m_iInterMCBufUsage += 2*sizeof(DXVA_MVvalue);
						}
					}
				}
				else
				{
					SHORT sHorz0, sVert0, sHorz1, sVert1;
					sHorz0 = Mv->horz = currMB_s_d->pred_info.mv[LIST_0][b48_idx[b8][i]].x;
					sVert0 = Mv->vert = currMB_s_d->pred_info.mv[LIST_0][b48_idx[b8][i]].y;
					Mv++;
					sHorz1 = Mv->horz = currMB_s_d->pred_info.mv[LIST_1][b48_idx[b8][i]].x;
					sVert1 = Mv->vert = currMB_s_d->pred_info.mv[LIST_1][b48_idx[b8][i]].y;
					Mv++;
					Mb_partition->bMvQuantity +=2;
					IMGPAR m_iInterMCBufUsage += sizeof(DXVA_MVvalue)*2;

					if (m_bResidualDataFormat==E_RES_DATA_FORMAT_INTEL)
					{
						Mv+=2;
						Mv->horz = sHorz0;
						Mv->vert = sVert0;
						Mv++;
						Mv->horz = sHorz1;
						Mv->vert = sVert1;
						(i==0) ? Mv-=3 : Mv++;
						Mb_partition->bMvQuantity +=2;
						IMGPAR m_iInterMCBufUsage += sizeof(DXVA_MVvalue)*2;
					}
				}
			}
		}
		else		// PB_4x4
		{
			Mb_partition->bSubMbShapes |= (0x03 << (b8*2));
			for(b4=0 ; b4<4 ; b4++)
			{
				if(pred_dir != 2)
				{
					SHORT sHorz, sVert;
					//ref_picid = currMB_s_d->pred_info.ref_pic_id[pred_dir][b8];	
					sHorz = Mv->horz = currMB_s_d->pred_info.mv[pred_dir][b44_idx[b8][b4]].x;
					sVert = Mv->vert = currMB_s_d->pred_info.mv[pred_dir][b44_idx[b8][b4]].y;
					Mv++;
					Mb_partition->bMvQuantity++;
					IMGPAR m_iInterMCBufUsage += sizeof(DXVA_MVvalue);

					if (m_bResidualDataFormat==E_RES_DATA_FORMAT_INTEL /*&& IMGPAR type==B_SLICE*/) //B MB (32 MV)
					{													//Mark for Intel change 16MV to 32 MV
						Mv->horz = sHorz;
						Mv->vert = sVert;
						Mv++;
						Mb_partition->bMvQuantity++;
						IMGPAR m_iInterMCBufUsage += sizeof(DXVA_MVvalue);
					}
				}
				else
				{
					//ref_picid = currMB_s_d->pred_info.ref_pic_id[LIST_0][b8];
					//ref_picid_se = currMB_s_d->pred_info.ref_pic_id[LIST_1][b8];	
					Mv->horz = currMB_s_d->pred_info.mv[LIST_0][b44_idx[b8][b4]].x;
					Mv->vert = currMB_s_d->pred_info.mv[LIST_0][b44_idx[b8][b4]].y;
					Mv++;
					Mv->horz = currMB_s_d->pred_info.mv[LIST_1][b44_idx[b8][b4]].x;
					Mv->vert = currMB_s_d->pred_info.mv[LIST_1][b44_idx[b8][b4]].y;
					Mv++;
					Mb_partition->bMvQuantity +=2;
					IMGPAR m_iInterMCBufUsage += sizeof(DXVA_MVvalue)*2;
				}
			}
		}
	}

	//build weight/offset data block
	if (m_bResidualDataFormat==E_RES_DATA_FORMAT_INTEL && IMGPAR apply_weights)
	{
		//if(!Has_Sub_Partition) //8 MV
		//	Mv+=24; //24 n/a DWords     //Marked to Pack WO data right after MV data
		//else                               //Mark for Intel change 16MV to 32 MV
		//{
		//	if(IMGPAR type==P_SLICE) //16 MV
		//		Mv+=16; //16 n/a DWords
		//}

		int iPredDir;
		char *pData = (char*)Mv;

		for(b8=0; b8<4; b8++)
		{
			iPredDir = iPredDirSet[b8];
			//pred_dir = currMB_d->b8pdir[b8];

			if(iPredDir != 2)
			{
				int ref_idx = currMB_s_d->pred_info.ref_idx[iPredDir][b8];

				if (active_pps.weighted_pred_flag && curr_mb_field)
					ref_idx >>=1;

				build_weight_data_block_INTEL ARGS5(iPredDir, pData, ref_idx, ref_idx, list_offset);
			}
			else
			{
				int ref_idx_0 = currMB_s_d->pred_info.ref_idx[LIST_0][b8];
				int ref_idx_1 = currMB_s_d->pred_info.ref_idx[LIST_1][b8];

				if (active_pps.weighted_bipred_idc==1 && curr_mb_field)
				{
					ref_idx_0 >>=1;
					ref_idx_1 >>=1;
				}

				build_weight_data_block_INTEL ARGS5(iPredDir, pData, ref_idx_0, ref_idx_1, list_offset);
			}
			pData+=16;
		}
	}
}

void CH264DXVA2::build_weight_data_block_INTEL PARGS5(int pred_dir, char *pData, int ref_idx_0, int ref_idx_1, int list_offset)
{																				//Perhaps int pred_dir_0, int pred_dir_1 can be removed
	//There's a unit of Weight_Offset in each slice, and for each MB, Weight_Offset is found 
	//from associated Slice Parameter according to pred_dir_0(1) and ref_idx_0(1) of MB.
	//Weights[pred_dir][ref_idx][Y=0,Cb=1,Cr=2][weight=0,offset=1]

	DXVA_Slice_H264_Long *pSliceDataBuffer = (DXVA_Slice_H264_Long *)IMGPAR m_lpSLICE;
	int wt_list_offset = (active_pps.weighted_bipred_idc==2)?list_offset:0;

	if (active_pps.weighted_bipred_idc != 2)
	{
		char *pWeightOffsetValue = pData;

		if(pred_dir==2)
		{
			//Y
			pWeightOffsetValue[0] = (*(IMGPAR wbp_weight+(((0+wt_list_offset)*MAX_REFERENCE_PICTURES+ref_idx_0)*MAX_REFERENCE_PICTURES+ref_idx_1)*3+0));
			pWeightOffsetValue[1] = (*(IMGPAR wp_offset+((wt_list_offset+0)*MAX_REFERENCE_PICTURES+ref_idx_0)*3+0));
			pWeightOffsetValue[2] = (*(IMGPAR wbp_weight+(((1+wt_list_offset)*MAX_REFERENCE_PICTURES+ref_idx_0)*MAX_REFERENCE_PICTURES+ref_idx_1)*3+0));
			pWeightOffsetValue[3] = (*(IMGPAR wp_offset+((wt_list_offset+1)*MAX_REFERENCE_PICTURES+ref_idx_1)*3+0));

			//Cb
			pWeightOffsetValue[4] = (*(IMGPAR wbp_weight+(((0+wt_list_offset)*MAX_REFERENCE_PICTURES+ref_idx_0)*MAX_REFERENCE_PICTURES+ref_idx_1)*3+0+1));
			pWeightOffsetValue[5] = (*(IMGPAR wp_offset+((wt_list_offset + 0)*MAX_REFERENCE_PICTURES+ref_idx_0)*3+0+1));
			pWeightOffsetValue[6] = (*(IMGPAR wbp_weight+(((1+wt_list_offset)*MAX_REFERENCE_PICTURES+ref_idx_0)*MAX_REFERENCE_PICTURES+ref_idx_1)*3+0+1));
			pWeightOffsetValue[7] = (*(IMGPAR wp_offset+((wt_list_offset + 1)*MAX_REFERENCE_PICTURES+ref_idx_1)*3+0+1));

			//Cr
			pWeightOffsetValue[8] = (*(IMGPAR wbp_weight+(((0+wt_list_offset)*MAX_REFERENCE_PICTURES+ref_idx_0)*MAX_REFERENCE_PICTURES+ref_idx_1)*3+1+1));
			pWeightOffsetValue[9] = (*(IMGPAR wp_offset+((wt_list_offset + 0)*MAX_REFERENCE_PICTURES+ref_idx_0)*3+1+1));
			pWeightOffsetValue[10] = (*(IMGPAR wbp_weight+(((1+wt_list_offset)*MAX_REFERENCE_PICTURES+ref_idx_0)*MAX_REFERENCE_PICTURES+ref_idx_1)*3+1+1));
			pWeightOffsetValue[11] = (*(IMGPAR wp_offset+((wt_list_offset + 1)*MAX_REFERENCE_PICTURES+ref_idx_1)*3+1+1));
		}
		else
		{
			//Y
			if(pred_dir)
			{
				pWeightOffsetValue[0] = (*(IMGPAR wp_weight+(pred_dir*MAX_REFERENCE_PICTURES+ref_idx_1)*3+0));
				pWeightOffsetValue[1] = (*(IMGPAR wp_offset+(pred_dir*MAX_REFERENCE_PICTURES+ref_idx_1)*3+0));
				pWeightOffsetValue[2] = (*(IMGPAR wp_weight+(pred_dir*MAX_REFERENCE_PICTURES+ref_idx_1)*3+0));
				pWeightOffsetValue[3] = (*(IMGPAR wp_offset+(pred_dir*MAX_REFERENCE_PICTURES+ref_idx_1)*3+0));
			}
			else
			{
				pWeightOffsetValue[0] = (*(IMGPAR wp_weight+(pred_dir*MAX_REFERENCE_PICTURES+ref_idx_0)*3+0));
				pWeightOffsetValue[1] = (*(IMGPAR wp_offset+(pred_dir*MAX_REFERENCE_PICTURES+ref_idx_0)*3+0));
				pWeightOffsetValue[2] = (*(IMGPAR wp_weight+(pred_dir*MAX_REFERENCE_PICTURES+ref_idx_0)*3+0));
				pWeightOffsetValue[3] = (*(IMGPAR wp_offset+(pred_dir*MAX_REFERENCE_PICTURES+ref_idx_0)*3+0));
			}

			//Cb
			if(pred_dir)
			{
				pWeightOffsetValue[4] = (*(IMGPAR wp_weight+(pred_dir*MAX_REFERENCE_PICTURES+ref_idx_1)*3+0+1));
				pWeightOffsetValue[5] = (*(IMGPAR wp_offset+(pred_dir*MAX_REFERENCE_PICTURES+ref_idx_1)*3+0+1));
				pWeightOffsetValue[6] = (*(IMGPAR wp_weight+(pred_dir*MAX_REFERENCE_PICTURES+ref_idx_1)*3+0+1));
				pWeightOffsetValue[7] = (*(IMGPAR wp_offset+(pred_dir*MAX_REFERENCE_PICTURES+ref_idx_1)*3+0+1));
			}
			else
			{
				pWeightOffsetValue[4] = (*(IMGPAR wp_weight+(pred_dir*MAX_REFERENCE_PICTURES+ref_idx_0)*3+0+1));
				pWeightOffsetValue[5] = (*(IMGPAR wp_offset+(pred_dir*MAX_REFERENCE_PICTURES+ref_idx_0)*3+0+1));
				pWeightOffsetValue[6] = (*(IMGPAR wp_weight+(pred_dir*MAX_REFERENCE_PICTURES+ref_idx_0)*3+0+1));
				pWeightOffsetValue[7] = (*(IMGPAR wp_offset+(pred_dir*MAX_REFERENCE_PICTURES+ref_idx_0)*3+0+1));
			}

			//Cr
			if(pred_dir)
			{
				pWeightOffsetValue[8] = (*(IMGPAR wp_weight+(pred_dir*MAX_REFERENCE_PICTURES+ref_idx_1)*3+1+1));
				pWeightOffsetValue[9] = (*(IMGPAR wp_offset+(pred_dir*MAX_REFERENCE_PICTURES+ref_idx_1)*3+1+1));
				pWeightOffsetValue[10] = (*(IMGPAR wp_weight+(pred_dir*MAX_REFERENCE_PICTURES+ref_idx_1)*3+1+1));
				pWeightOffsetValue[11] = (*(IMGPAR wp_offset+(pred_dir*MAX_REFERENCE_PICTURES+ref_idx_1)*3+1+1));
			}
			else
			{
				pWeightOffsetValue[8] = (*(IMGPAR wp_weight+(pred_dir*MAX_REFERENCE_PICTURES+ref_idx_0)*3+1+1));
				pWeightOffsetValue[9] = (*(IMGPAR wp_offset+(pred_dir*MAX_REFERENCE_PICTURES+ref_idx_0)*3+1+1));
				pWeightOffsetValue[10] = (*(IMGPAR wp_weight+(pred_dir*MAX_REFERENCE_PICTURES+ref_idx_0)*3+1+1));
				pWeightOffsetValue[11] = (*(IMGPAR wp_offset+(pred_dir*MAX_REFERENCE_PICTURES+ref_idx_0)*3+1+1));
			}
		}
	}
	else
	{
		short *pWeightOffsetValue = (short*)(pData);

		if(pred_dir==2)
		{
			//Y
			pWeightOffsetValue[0] = (*(IMGPAR wbp_weight+(((0+wt_list_offset)*MAX_REFERENCE_PICTURES+ref_idx_0)*MAX_REFERENCE_PICTURES+ref_idx_1)*3+0));
			pWeightOffsetValue[1] = (*(IMGPAR wbp_weight+(((1+wt_list_offset)*MAX_REFERENCE_PICTURES+ref_idx_0)*MAX_REFERENCE_PICTURES+ref_idx_1)*3+0));

			//Cb
			pWeightOffsetValue[2] = (*(IMGPAR wbp_weight+(((0+wt_list_offset)*MAX_REFERENCE_PICTURES+ref_idx_0)*MAX_REFERENCE_PICTURES+ref_idx_1)*3+0+1));
			pWeightOffsetValue[3] = (*(IMGPAR wbp_weight+(((1+wt_list_offset)*MAX_REFERENCE_PICTURES+ref_idx_0)*MAX_REFERENCE_PICTURES+ref_idx_1)*3+0+1));

			//Cr
			pWeightOffsetValue[4] = (*(IMGPAR wbp_weight+(((0+wt_list_offset)*MAX_REFERENCE_PICTURES+ref_idx_0)*MAX_REFERENCE_PICTURES+ref_idx_1)*3+1+1));
			pWeightOffsetValue[5] = (*(IMGPAR wbp_weight+(((1+wt_list_offset)*MAX_REFERENCE_PICTURES+ref_idx_0)*MAX_REFERENCE_PICTURES+ref_idx_1)*3+1+1));
		}
		else
		{
			//Y
			if(pred_dir)
			{
				pWeightOffsetValue[0] = (*(IMGPAR wp_weight+(pred_dir*MAX_REFERENCE_PICTURES+ref_idx_1)*3+0));
				pWeightOffsetValue[1] = (*(IMGPAR wp_weight+(pred_dir*MAX_REFERENCE_PICTURES+ref_idx_1)*3+0));
			}
			else
			{
				pWeightOffsetValue[0] = (*(IMGPAR wp_weight+(pred_dir*MAX_REFERENCE_PICTURES+ref_idx_0)*3+0));
				pWeightOffsetValue[1] = (*(IMGPAR wp_weight+(pred_dir*MAX_REFERENCE_PICTURES+ref_idx_0)*3+0));
			}

			//Cb
			if(pred_dir)
			{
				pWeightOffsetValue[2] = (*(IMGPAR wp_weight+(pred_dir*MAX_REFERENCE_PICTURES+ref_idx_1)*3+0+1));
				pWeightOffsetValue[3] = (*(IMGPAR wp_weight+(pred_dir*MAX_REFERENCE_PICTURES+ref_idx_1)*3+0+1));
			}
			else
			{
				pWeightOffsetValue[2] = (*(IMGPAR wp_weight+(pred_dir*MAX_REFERENCE_PICTURES+ref_idx_0)*3+0+1));
				pWeightOffsetValue[3] = (*(IMGPAR wp_weight+(pred_dir*MAX_REFERENCE_PICTURES+ref_idx_0)*3+0+1));
			}

			//Cr
			if(pred_dir)
			{
				pWeightOffsetValue[4] = (*(IMGPAR wp_weight+(pred_dir*MAX_REFERENCE_PICTURES+ref_idx_1)*3+1+1));
				pWeightOffsetValue[5] = (*(IMGPAR wp_weight+(pred_dir*MAX_REFERENCE_PICTURES+ref_idx_1)*3+1+1));
			}
			else
			{
				pWeightOffsetValue[4] = (*(IMGPAR wp_weight+(pred_dir*MAX_REFERENCE_PICTURES+ref_idx_0)*3+1+1));
				pWeightOffsetValue[5] = (*(IMGPAR wp_weight+(pred_dir*MAX_REFERENCE_PICTURES+ref_idx_0)*3+1+1));
			}
		}
	}
}

int CH264DXVA2::ExecuteBuffers PARGS1(DWORD dwFlags)
{
	ExecuteBuffersStruct cur_img;// = new ExecuteBuffersStruct; //need to be change from outside.

	build_picture_decode_buffer ARGS0();

	cur_img.currentSlice_structure = IMGPAR currentSlice->structure;
	cur_img.currentSlice_picture_type = IMGPAR currentSlice->picture_type;
	cur_img.UnCompress_Idx = IMGPAR UnCompress_Idx;
	cur_img.m_iIntraMCBufUsage = IMGPAR m_iIntraMCBufUsage;
	cur_img.m_iIntraRESBufUsage = IMGPAR m_iIntraRESBufUsage;
	cur_img.m_iInterMCBufUsage = IMGPAR m_iInterMCBufUsage;
	cur_img.smart_dec = IMGPAR smart_dec;
	cur_img.slice_number = IMGPAR slice_number;
	cur_img.nal_reference_idc = IMGPAR nal_reference_idc;
	cur_img.m_is_MTMS = stream_global->m_is_MTMS;
	cur_img.FrameSizeInMbs = IMGPAR FrameSizeInMbs;
	cur_img.PicSizeInMbs = IMGPAR PicSizeInMbs;

	cur_img.m_iVGAType = m_nVGAType;
	cur_img.m_pic_combine_status = IMGPAR currentSlice->m_pic_combine_status;
	if (IMGPAR currentSlice->structure!=FRAME && IMGPAR currentSlice->m_pic_combine_status)
		cur_img.m_Intra_lCompBufIndex = IMGPAR m_iFirstCompBufIndex;
	else
		cur_img.m_Intra_lCompBufIndex = IMGPAR m_Intra_lCompBufIndex;
	cur_img.m_lFrame_Counter = m_nFrameCounter;

	BYTE* pRDBuffer = m_pbResidualDiffBuf[cur_img.m_Intra_lCompBufIndex];
	if (m_bResidualDataFormat==E_RES_DATA_FORMAT_NV && cur_img.m_is_MTMS)
	{
		int iSliceNum = (cur_img.slice_number>>(int)(IMGPAR structure != FRAME));
		memcpy(&(cur_img.m_sResinfo[0]), &(m_ResInfo[0]), sizeof(RESD_INFO)*iSliceNum);		
	}

	cur_img.m_pnv1PictureDecode = IMGPAR m_pnv1PictureDecode;
	if (m_nDXVAMode==E_H264_DXVA_MODE_C)
		cur_img.m_lpQMatrix = IMGPAR m_lpQMatrix;
	else
		cur_img.m_lpQMatrix = NULL;
	cur_img.m_lpMBLK_Intra_Luma = IMGPAR m_lpMBLK_Intra_Luma;
	cur_img.m_lpSLICE = IMGPAR m_lpSLICE;
	cur_img.m_lpMV	= IMGPAR m_lpMV;
	cur_img.m_lpRESD_Intra_Luma = IMGPAR m_lpRESD_Intra_Luma;
	cur_img.m_lpRESD_Intra_Chroma = IMGPAR m_lpRESD_Intra_Chroma;
	cur_img.m_lpRESD_Inter_Luma = IMGPAR m_lpRESD_Inter_Luma;
	cur_img.m_lpRESD_Inter_Chroma = IMGPAR m_lpRESD_Inter_Chroma;

	cur_img.m_lmbCount_Inter = IMGPAR m_lmbCount_Inter;
	cur_img.m_lmbCount_Intra = IMGPAR m_lmbCount_Intra;
	cur_img.m_iInterRESBufUsage_L = IMGPAR m_iInterRESBufUsage_L;
	cur_img.m_iInterRESBufUsage_C = IMGPAR m_iInterRESBufUsage_C;

	cur_img.m_lpPicDec = m_pbPictureParamBuf[IMGPAR m_Intra_lCompBufIndex];
	if (m_nDXVAMode==E_H264_DXVA_MODE_C)
		cur_img.m_lpQMatrixCtl = m_pbIQMatrixBuf[IMGPAR m_Intra_lCompBufIndex];
	else
		cur_img.m_lpQMatrixCtl = NULL;
	cur_img.m_lpSliCtl = m_pbSliceCtrlBuf[IMGPAR m_Intra_lCompBufIndex];
	cur_img.m_lpMbCtl = m_pbMacroblockCtrlBuf[IMGPAR m_Intra_lCompBufIndex];
	cur_img.m_lpMv = m_pbMotionVectorBuf[IMGPAR m_Intra_lCompBufIndex];
	cur_img.m_lpResdual = m_pbResidualDiffBuf[IMGPAR m_Intra_lCompBufIndex];

	m_lpPictureParamBuf = m_lpInverseQuantMatrixBuf = m_lpSliceCtrlBuf = m_lpMacroblockCtrlBuf = m_lpResidualDiffBuf = m_lpMotionVectorBuf = NULL;

	cur_img.m_unique_id = dec_picture->unique_id;
	cur_img.m_pIviCP = m_pIviCP;
	cur_img.m_bSkipedBFrame = dec_picture->SkipedBFrames[dec_picture->view_index][0];
	cur_img.m_bResidualDataFormat = m_bResidualDataFormat;
	cur_img.m_bDMATransfer = false;
	cur_img.m_iMbaffFrameFlag = IMGPAR MbaffFrameFlag;
	cur_img.m_iDXVAMode_temp = m_nDXVAMode;

	cur_img.poc = IMGPAR ThisPOC;

	if(m_bResidualDataFormat==E_RES_DATA_FORMAT_NV)
		memset(&m_ResInfo[0], 0, 20*sizeof(RESD_INFO));

	EnterCriticalSection( &crit_ACCESS_QUEUE );
	m_Queue->push(cur_img);	
	long count;
	ReleaseSemaphore(
		stream_global->m_queue_semaphore, 
		1, 
		&count
		);
	LeaveCriticalSection( &crit_ACCESS_QUEUE );

	m_nFrameCounter++;

	return 0;
}

void CH264DXVA2::DMA_Transfer PARGS0()
{
	HRESULT	hr=0;

	if (IMGPAR Hybrid_Decoding == 5)
	{
		int i;
		int width,offset;
		int height = (!IMGPAR structure ? IMGPAR height : IMGPAR height>>1);	
		BYTE *raw;
		imgpel *imgy = dec_picture->imgY;
		imgpel *imguv = dec_picture->imgUV;
		SurfaceInfo p_info;
		long l_lStride;

		if(m_nVGAType == E_H264_VGACARD_ATI)	//ATI's DXVA needs to call BeginFrame and EndFrame when we accessing the surface.
		{
			while(1)
			{
				hr = BeginFrame(IMGPAR UnCompress_Idx, 1);
				if(checkDDError(hr))
				{
					m_pUncompBufQueue->PutItem(IMGPAR UnCompress_Idx);
					Sleep(2);
				}
				else
					break;
				IMGPAR UnCompress_Idx = (int)m_pUncompBufQueue->GetItem();
			}
		}

		dec_picture->pic_store_idx = IMGPAR UnCompress_Idx;

		SetSurfaceInfo ARGS3(dec_picture->unique_id, (IMGPAR structure== BOTTOM_FIELD?1:0),1 );

		if (m_nVGAType!=E_H264_VGACARD_ATI)	// ATI not support IDR Seek.
		{
			if (dec_picture->used_for_reference)
				m_RefList[IMGPAR UnCompress_Idx].bPicEntry = IMGPAR UnCompress_Idx;
			if (IMGPAR currentSlice->structure == FRAME)
			{
				m_RefInfo[IMGPAR UnCompress_Idx].top_poc =  IMGPAR toppoc;
				m_RefInfo[IMGPAR UnCompress_Idx].bot_poc =  IMGPAR bottompoc;
			}
			else if (IMGPAR currentSlice->structure == TOP_FIELD)
				m_RefInfo[IMGPAR UnCompress_Idx].top_poc =  IMGPAR toppoc;
			else
				m_RefInfo[IMGPAR UnCompress_Idx].bot_poc =  IMGPAR bottompoc;

			m_RefInfo[IMGPAR UnCompress_Idx].frame_num = IMGPAR currentSlice->frame_num;
		}

		if (SUCCEEDED(hr))
		{
			hr = m_pIHVDServiceDxva2->LockUncompressedBuffer(IMGPAR UnCompress_Idx, (HVDService::HVDDxva2UncompBufLockInfo*)&p_info);
			l_lStride = p_info.uPitch;
			raw = (BYTE*)p_info.pBits;
			if (IMGPAR structure)
			{
				width = (l_lStride<<1);
				offset= l_lStride;
				if(IMGPAR structure==2)
					raw += offset;
			}
			else
			{
				width = l_lStride;
				offset = 0;
			}

			for (i=0 ; i<height; i++)
			{
				memcpy(raw,imgy,IMGPAR width);
				raw += width;
				imgy += dec_picture->Y_stride;
			}

			for (i=0 ; i<(height>>1); i++)
			{
				memcpy(raw,imguv,IMGPAR width);
				raw += (offset + l_lStride);
				imguv += dec_picture->UV_stride;
			}
			hr = m_pIHVDServiceDxva2->UnlockUncompressedBuffer(IMGPAR UnCompress_Idx);
			if (m_nVGAType == E_H264_VGACARD_ATI)
				hr = EndFrame();
		}
	}
	else
	{
		dec_picture->pic_store_idx = IMGPAR UnCompress_Idx;

		SetSurfaceInfo ARGS3(dec_picture->unique_id, (IMGPAR structure== BOTTOM_FIELD?1:0), 1);

		if (m_nVGAType!=E_H264_VGACARD_ATI)	// ATI not support IDR Seek.
		{
			if (dec_picture->used_for_reference)
				m_RefList[IMGPAR UnCompress_Idx].bPicEntry = IMGPAR UnCompress_Idx;
			if (IMGPAR currentSlice->structure == FRAME)
			{
				m_RefInfo[IMGPAR UnCompress_Idx].top_poc =  IMGPAR toppoc;
				m_RefInfo[IMGPAR UnCompress_Idx].bot_poc =  IMGPAR bottompoc;
			}
			else if (IMGPAR currentSlice->structure == TOP_FIELD)
				m_RefInfo[IMGPAR UnCompress_Idx].top_poc =  IMGPAR toppoc;
			else
				m_RefInfo[IMGPAR UnCompress_Idx].bot_poc =  IMGPAR bottompoc;

			m_RefInfo[IMGPAR UnCompress_Idx].frame_num = IMGPAR currentSlice->frame_num;
		}
	}

	if (((IMGPAR currentSlice->structure == FRAME) || (IMGPAR currentSlice->m_pic_combine_status == 0))
		&& (IMGPAR Hybrid_Decoding!=5))
	{
		hr = m_pIHVDServiceDxva2->UnlockUncompressedBuffer(IMGPAR UnCompress_Idx);
		if (m_nVGAType == E_H264_VGACARD_ATI)
			hr = EndFrame();
	}

#if defined(_USE_QUEUE_FOR_DXVA2_)
	SetEvent(stream_global->hReadyForRender[IMGPAR UnCompress_Idx]);
#endif
}

void CH264DXVA2::StoreImgRowToImgPic PARGS2(int start_x, int end_x)
{//Used for HW_PB
	//write back to picture	
	int i, j;
	imgpel *imgY;		
	imgpel *imgUV;
	imgpel *imgYDXVA;
	imgpel *imgUVDXVA;
	int stride = dec_picture->Y_stride;
	int stride_UV = dec_picture->UV_stride;
	int width;
	int dxva_stride;
	int dxva_offset;	//For field scenario
	int dxva_height = (!IMGPAR structure ? IMGPAR height : IMGPAR height>>1);

	imgpel *imgY_row;
	imgpel *imgUV_row;

	width = (end_x-start_x + 1) << 4;

	if ( IMGPAR MbaffFrameFlag ) 
	{
		if ( IMGPAR mb_y_d & 1 )
		{		
			imgY     = dec_picture->imgY + (IMGPAR pix_y_d-16)*stride+(start_x<<4);
			imgUV    = dec_picture->imgUV + (IMGPAR pix_c_y_d-8)*stride_UV+(start_x<<4); 

			imgY_row = IMGPAR m_imgY + (IMGPAR pix_y_rows-16)*stride+(start_x<<4);
			imgUV_row = IMGPAR m_imgUV + (IMGPAR pix_c_y_rows-8)*stride_UV+(start_x<<4); 

			if(IMGPAR structure) {	//Field
				dxva_stride = (IMGPAR m_UnCompressedBufferStride<<1);
				dxva_offset= IMGPAR m_UnCompressedBufferStride;
				imgYDXVA = IMGPAR m_pUnCompressedBuffer + (IMGPAR pix_y_d-16)*dxva_stride+(start_x<<4);;
				imgUVDXVA = IMGPAR m_pUnCompressedBuffer + dxva_stride * dxva_height + (IMGPAR pix_c_y_d-8)*dxva_stride+(start_x<<4);;

				if(IMGPAR structure==2) {	//Bottom Field
					imgYDXVA += dxva_offset;
					imgUVDXVA += dxva_offset;
				}

			} else {	//Frame
				dxva_stride = IMGPAR m_UnCompressedBufferStride;
				imgYDXVA = IMGPAR m_pUnCompressedBuffer + (IMGPAR pix_y_d-16)*dxva_stride+(start_x<<4);;
				imgUVDXVA = IMGPAR m_pUnCompressedBuffer + dxva_stride * dxva_height + (IMGPAR pix_c_y_d-8)*dxva_stride+(start_x<<4);;

				dxva_offset = 0;
			}	

			for(j=0;j<32;j++)
			{
				mem16_2(imgY, imgYDXVA, imgY_row, width);
				imgY += stride;
				imgYDXVA += dxva_stride;
				imgY_row += stride;
			}

			for(i=0;i<16;i++) {

				mem16_2(imgUV, imgUVDXVA, imgUV_row, width);   

				imgUVDXVA += (dxva_offset + IMGPAR m_UnCompressedBufferStride);

				imgUV += stride_UV;
				imgUV_row += stride_UV;
			}
		}
	} else {				

		imgY     = dec_picture->imgY + IMGPAR pix_y_d*stride+(start_x<<4);
		imgUV    = dec_picture->imgUV + IMGPAR pix_c_y_d*stride_UV+(start_x<<4);  

		imgY_row = IMGPAR m_imgY + IMGPAR pix_y_rows*stride+(start_x<<4);
		imgUV_row = IMGPAR m_imgUV + IMGPAR pix_c_y_rows*stride_UV+(start_x<<4); 

		if(IMGPAR structure) {	//Field
			dxva_stride = (IMGPAR m_UnCompressedBufferStride<<1);
			dxva_offset= IMGPAR m_UnCompressedBufferStride;
			imgYDXVA = IMGPAR m_pUnCompressedBuffer + IMGPAR pix_y_d*dxva_stride+(start_x<<4);;
			imgUVDXVA = IMGPAR m_pUnCompressedBuffer + dxva_stride * dxva_height + IMGPAR pix_c_y_d*dxva_stride+(start_x<<4);;

			if(IMGPAR structure==2) {	//Bottom Field
				imgYDXVA += dxva_offset;
				imgUVDXVA += dxva_offset;
			}

		} else {	//Frame
			dxva_stride = IMGPAR m_UnCompressedBufferStride;
			imgYDXVA = IMGPAR m_pUnCompressedBuffer + IMGPAR pix_y_d*dxva_stride+(start_x<<4);;
			imgUVDXVA = IMGPAR m_pUnCompressedBuffer + dxva_stride * dxva_height + IMGPAR pix_c_y_d*dxva_stride+(start_x<<4);;

			dxva_offset = 0;
		}

		for(j=0;j<16;j++)
		{
			mem16_2(imgY, imgYDXVA, imgY_row, width);
			imgY += stride;
			imgYDXVA += dxva_stride;
			imgY_row += stride;
		}

		for(i=0;i<8;i++)
		{
			mem16_2(imgUV, imgUVDXVA, imgUV_row, width);

			imgUVDXVA += (dxva_offset + IMGPAR m_UnCompressedBufferStride);

			imgUV += stride_UV;
			imgUV_row += stride_UV;
		}		
	}
}

void CH264DXVA2::TransferData_at_SliceEnd PARGS0()
{
	if ((IMGPAR Hybrid_Decoding==0) ||
		((IMGPAR Hybrid_Decoding==1) && (IMGPAR type != I_SLICE)) ||
		((IMGPAR Hybrid_Decoding==2) && (IMGPAR type == B_SLICE)))
	{}
	else
		StoreImgRowToImgPic ARGS2(IMGPAR start_mb_x, IMGPAR mb_x_d);
}

static int g_nCounter = 0;
unsigned int CH264DXVA2::ThreadExecute(void * arg)
{
	CH264DXVA2* t_pDxva2 = (CH264DXVA2 *)arg;
	ExecuteBuffersStruct cur_img;
	queue <ExecuteBuffersStruct> *MemQueue = (queue <ExecuteBuffersStruct> *) t_pDxva2->m_Queue;
	StreamParameters *stream_global = t_pDxva2->stream_global;
	static int t_iCount = 10;
	HRESULT hr;
	int i;
	BYTE *t_pTemp;
	int t_iNumSlice;
	BYTE *t_lpPicDec;
	BYTE *t_lpQMatrix;
	BYTE *t_lpSliCtl;
	BYTE *t_lpMbCtl;
	BYTE *t_lpResdual;
	BYTE *t_lpMv;

	int t_dwNumBuffers;
	DXVA_BufferDescription t_pDxvaBufferDescription[5];
	AMVABUFFERINFO         t_pBufferInfo[5];

	int t_iDiffTime;
	BOOL t_bDropBottomB;

	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

	while(1)
	{
		WaitForSingleObject(stream_global->m_queue_semaphore, INFINITE);

		if(t_pDxva2->m_bRunThread == FALSE)
		{
			SetEvent(t_pDxva2->m_hEndCpyThread);
			break;
		}

		if(stream_global->m_dxva_queue_reset) //reset queue
		{	
			DEBUG_SHOW_HW_INFO("m_dxva_queue_reset");

			stream_global->m_dxva_queue_reset = 0;
			EnterCriticalSection( &(t_pDxva2->crit_ACCESS_QUEUE) );
			int size  = MemQueue->size();

			while(size--)
			{
				WaitForSingleObject(stream_global->m_queue_semaphore, 0);
				cur_img = MemQueue->front();
				MemQueue->pop();
				t_pDxva2->m_bCompBufStaus[cur_img.m_Intra_lCompBufIndex] = TRUE;
			}
			LeaveCriticalSection( &(t_pDxva2->crit_ACCESS_QUEUE) );
			SetEvent(stream_global->h_dxva_queue_reset_done);
			continue;
		}
		EnterCriticalSection( &(t_pDxva2->crit_ACCESS_QUEUE) );
		if(MemQueue->size())
		{
			cur_img = MemQueue->front();
			MemQueue->pop();	
			DEBUG_SHOW_HW_INFO("pop from queue");
			LeaveCriticalSection( &(t_pDxva2->crit_ACCESS_QUEUE) );
		}
		else
		{
			DEBUG_SHOW_HW_INFO("ExeQueue Size Error");
			LeaveCriticalSection( &(t_pDxva2->crit_ACCESS_QUEUE) );
			continue;
		}

		memset(t_pDxvaBufferDescription, 0, 5*sizeof(DXVA_BufferDescription));
		memset(t_pBufferInfo, 0, 5*sizeof(AMVABUFFERINFO));
		t_iNumSlice = (cur_img.slice_number>>(int)(cur_img.currentSlice_structure != FRAME));

		t_bDropBottomB = FALSE;
		g_nCounter++;
		if(cur_img.m_iVGAType==E_H264_VGACARD_ATI)
		{
			DEBUG_SHOW_HW_INFO("Execute Threading on ATI");

			hr = t_pDxva2->BeginFrame(cur_img.UnCompress_Idx, 0);

			if(SUCCEEDED(hr))
			{
				t_pDxvaBufferDescription[0].dwTypeIndex				=  t_pBufferInfo[0].dwTypeIndex				=  DXVA2_PictureParametersBufferType;
				t_pDxvaBufferDescription[0].dwDataSize				=  t_pBufferInfo[0].dwDataSize					= sizeof(DXVA_PictureParameters_H264);
				t_pDxvaBufferDescription[0].dwFirstMBaddress		=  0;
				t_pDxvaBufferDescription[0].dwNumMBsInBuffer		=  (cur_img.currentSlice_structure ==0 ? cur_img.FrameSizeInMbs : (cur_img.FrameSizeInMbs>>1));
				t_pDxvaBufferDescription[0].dwHeight				= 0;	
				t_pDxvaBufferDescription[0].dwWidth					= 0;
				t_pDxvaBufferDescription[0].dwStride				= 0;

				t_pDxvaBufferDescription[1].dwTypeIndex				= t_pBufferInfo[1].dwTypeIndex				= DXVA2_SliceControlBufferType;
				t_pDxvaBufferDescription[1].dwDataSize				= t_pBufferInfo[1].dwDataSize					= sizeof(DXVA_SliceParameter_H264);
				t_pDxvaBufferDescription[1].dwFirstMBaddress		= 0;
				t_pDxvaBufferDescription[1].dwNumMBsInBuffer		= (cur_img.currentSlice_structure ==0 ? cur_img.FrameSizeInMbs : (cur_img.FrameSizeInMbs>>1));
				t_pDxvaBufferDescription[1].dwHeight				= 0;	
				t_pDxvaBufferDescription[1].dwWidth					= 0;
				t_pDxvaBufferDescription[1].dwStride				= 0;

				t_pDxvaBufferDescription[2].dwTypeIndex				= t_pBufferInfo[2].dwTypeIndex				= DXVA2_MacroBlockControlBufferType;
				t_pDxvaBufferDescription[2].dwDataSize				= t_pBufferInfo[2].dwDataSize				= cur_img.m_SBBufOffset+cur_img.width*cur_img.m_PicHeight/8;
				t_pDxvaBufferDescription[2].dwFirstMBaddress		= 0;
				t_pDxvaBufferDescription[2].dwNumMBsInBuffer		= (cur_img.currentSlice_structure ==0 ? cur_img.FrameSizeInMbs : (cur_img.FrameSizeInMbs>>1));
				t_pDxvaBufferDescription[2].dwHeight				= cur_img.m_PicHeight;	
				t_pDxvaBufferDescription[2].dwWidth					= cur_img.width;
				t_pDxvaBufferDescription[2].dwStride				= 0;

				t_pDxvaBufferDescription[3].dwTypeIndex				= t_pBufferInfo[3].dwTypeIndex				= DXVA2_ResidualDifferenceBufferType;
				t_pDxvaBufferDescription[3].dwDataSize				= t_pBufferInfo[3].dwDataSize				= cur_img.m_ReBufStride*cur_img.m_PicHeight*3/2;
				t_pDxvaBufferDescription[3].dwFirstMBaddress		= 0;
				t_pDxvaBufferDescription[3].dwNumMBsInBuffer		= (cur_img.currentSlice_structure ==0 ? cur_img.FrameSizeInMbs : (cur_img.FrameSizeInMbs>>1));
				t_pDxvaBufferDescription[3].dwHeight				= cur_img.m_PicHeight;	
				t_pDxvaBufferDescription[3].dwWidth					= cur_img.width;
				t_pDxvaBufferDescription[3].dwStride				= cur_img.m_ReBufStride;

				if(cur_img.currentSlice_picture_type != I_SLICE)
				{
					t_pDxvaBufferDescription[4].dwTypeIndex				= t_pBufferInfo[4].dwTypeIndex				= DXVA2_MotionVectorBuffer;
					t_pDxvaBufferDescription[4].dwDataSize				= t_pBufferInfo[4].dwDataSize				= cur_img.m_MVBufStride*cur_img.m_PicHeight/2;
					t_pDxvaBufferDescription[4].dwFirstMBaddress		= 0;
					t_pDxvaBufferDescription[4].dwNumMBsInBuffer		= (cur_img.currentSlice_structure ==0 ? cur_img.FrameSizeInMbs : (cur_img.FrameSizeInMbs>>1));
					t_pDxvaBufferDescription[4].dwHeight				= cur_img.m_PicHeight;	
					t_pDxvaBufferDescription[4].dwWidth					= cur_img.width;
					t_pDxvaBufferDescription[4].dwStride				= cur_img.m_MVBufStride;
					t_dwNumBuffers = 5;
				}
				else
					t_dwNumBuffers = 4;

				t_pDxva2->GetBuffer(DXVA2_PictureParametersBufferType, (LPVOID*)(&t_lpPicDec), 0);
				memcpy(t_lpPicDec, cur_img.m_pnv1PictureDecode, sizeof(DXVA_PictureParameters_H264));
				t_pDxva2->GetBuffer(DXVA2_MacroBlockControlBufferType, (LPVOID*)(&t_lpMbCtl), 0);
				memcpy(t_lpMbCtl, cur_img.m_lpMBLK_Intra_Luma, cur_img.m_SBBufOffset+cur_img.width*cur_img.m_PicHeight/8);
				t_pDxva2->GetBuffer(DXVA2_ResidualDifferenceBufferType, (LPVOID*)(&t_lpResdual), 0);
				//memcpy(t_lpResdual, cur_img.m_lpRESD_Intra_Luma, cur_img.m_ReBufStride*cur_img.m_PicHeight*3/2);
				{
					int i,j,k;
					int stride = cur_img.m_ReBufStride>>1;
					short *luma_s, *chroma_s;
					short *luma_d;
					short *chroma_d;

					luma_s   = (short*)cur_img.m_lpRESD_Intra_Luma;
					chroma_s = luma_s + cur_img.m_PicHeight*cur_img.width;

					/*
					To make writing to VGA memory most effecient, we have to write 64 bytes at a time (write-combined  buffer) 
					*/
					if(cur_img.m_iMbaffFrameFlag)
					{
						/*
						Mbaff case
						*/
						for(j=0;j<(cur_img.m_PicHeight>>4);j+=2)
						{
							luma_d   = (short*)t_lpResdual+j*16*stride;
							chroma_d = (short*)t_lpResdual+stride*cur_img.m_PicHeight+j*8*stride;

							for(i=0;i<(cur_img.width>>5);i++)
							{							
								for(k=0;k<16;k++)
								{
									memcpy(luma_d+stride*k, luma_s+16*k, 32);
									memcpy(luma_d+stride*k+16, luma_s+512+16*k, 32);

									memcpy(luma_d+stride*(k+16), luma_s+256+16*k, 32);
									memcpy(luma_d+stride*(k+16)+16, luma_s+256+512+16*k, 32);
								}

								for(k=0;k<8;k++)
								{
									memcpy(chroma_d+stride*k, chroma_s+16*k, 32);
									memcpy(chroma_d+stride*k+16, chroma_s+256+16*k, 32);

									memcpy(chroma_d+stride*(k+8), chroma_s+128+16*k, 32);
									memcpy(chroma_d+stride*(k+8)+16, chroma_s+256+128+16*k, 32);
								}
								luma_s += 512*2;
								chroma_s += 256*2;
								luma_d += 32;
								chroma_d += 32;
							}

							//mb width is odd number
							if((cur_img.width>>4)&1)
							{
								for(k=0;k<16;k++)
								{
									memcpy(luma_d+stride*k, luma_s+16*k, 32);	
									memcpy(luma_d+stride*(k+16), luma_s+256+16*k, 32);
								}

								for(k=0;k<8;k++)
								{
									memcpy(chroma_d+stride*k, chroma_s+16*k, 32);
									memcpy(chroma_d+stride*(k+8), chroma_s+128+16*k, 32);
								}
								luma_s += 256*2;
								chroma_s += 128*2;							
							}
						}
					}
					else
					{
						/*
						Non-Mbaff case
						*/
						for(j=0;j<(cur_img.m_PicHeight>>4);j++)
						{
							luma_d   = (short*)t_lpResdual+j*16*stride;
							chroma_d = (short*)t_lpResdual+stride*cur_img.m_PicHeight+j*8*stride;

							for(i=0;i<(cur_img.width>>5);i++)
							{									
								for(k=0;k<16;k++)
								{
									memcpy(luma_d+stride*k, luma_s+16*k, 32);
									memcpy(luma_d+stride*k+16, luma_s+256+16*k, 32);
								}								

								for(k=0;k<8;k++)
								{
									memcpy(chroma_d+stride*k, chroma_s+16*k, 32);
									memcpy(chroma_d+stride*k+16, chroma_s+128+16*k, 32);
								}
								luma_s += 512;
								chroma_s += 256;
								luma_d += 32;
								chroma_d += 32;
							}

							//mb width is odd number
							if((cur_img.width>>4)&1)
							{
								for(k=0;k<16;k++)
								{
									memcpy(luma_d+stride*k, luma_s+16*k, 32);								
								}

								for(k=0;k<8;k++)
								{
									memcpy(chroma_d+stride*k, chroma_s+16*k, 32);								
								}
								luma_s += 256;
								chroma_s += 128;							
							}
						}
					}
				}
				t_pDxva2->GetBuffer(DXVA2_SliceControlBufferType, (LPVOID*)(&t_lpSliCtl), 0);
				memcpy(t_lpSliCtl, cur_img.m_lpSLICE, sizeof(DXVA_SliceParameter_H264));

				if(t_dwNumBuffers==5)
				{
					t_pDxva2->GetBuffer(DXVA2_MotionVectorBuffer, (LPVOID*)(&t_lpMv), 0);
					memcpy(t_lpMv, cur_img.m_lpMV, cur_img.m_MVBufStride*cur_img.m_PicHeight/2);
				}
				if(cur_img.m_pIviCP != NULL)
				{
					int scramble_size_luma;
					int scramble_size_chroma;
					if(cur_img.m_PicHeight <=1024)
					{
						scramble_size_luma = cur_img.m_ReBufStride*cur_img.m_PicHeight;
						scramble_size_chroma = cur_img.m_ReBufStride*cur_img.m_PicHeight/2;
					}
					else
					{
						scramble_size_luma = cur_img.m_ReBufStride*1024;
						scramble_size_chroma = cur_img.m_ReBufStride*((cur_img.m_PicHeight-1024) + cur_img.m_PicHeight/2);
					}

					if(cur_img.m_pIviCP != NULL)
					{											
						if(cur_img.m_pIviCP->IsScramblingRequired(MAP_CP_PIC_TYPE(cur_img.currentSlice_picture_type)))
						{					
							cur_img.m_pIviCP->EnableScrambling();
							cur_img.m_pIviCP->ScrambleData(t_lpResdual, t_lpResdual, scramble_size_luma+scramble_size_chroma);												
						}
						else
						{
							cur_img.m_pIviCP->DisableScrambling();
						}

						if(cur_img.m_pIviCP->GetObjID()==E_CP_ID_WIN7)
						{
							t_pDxva2->BeginFrame(cur_img.UnCompress_Idx, 0);
							t_pDxva2->EndFrame();
						}
					}
				}

				if (g_framerate1000<30000)
				{
					if (t_iCount<0 && (cur_img.currentSlice_structure==FRAME || cur_img.m_pic_combine_status))
					{
						if (t_pDxva2->m_pUncompBufQueue->count > 8) //Display Queue <= 3
							WaitForSingleObject(stream_global->hFinishDisplaySemaphore, (30*g_PulldownRate)/1000);
						else
							WaitForSingleObject(stream_global->hFinishDisplaySemaphore, (50*g_PulldownRate)/1000);

						QueryPerformanceCounter(&stream_global->m_liRecodeTime1);
						t_iDiffTime = (stream_global->m_iEstimateNextFrame - ((int)(1000 * (stream_global->m_liRecodeTime1.QuadPart - stream_global->m_liRecodeTime0.QuadPart))/t_pDxva2->m_pUncompBufQueue->m_freq.QuadPart));
						DEBUG_SHOW_HW_INFO("Diff Time: %d, count: %d queue size: %d", t_iDiffTime, t_pDxva2->m_pUncompBufQueue->count, MemQueue->size());

						if (t_iDiffTime > 50)
							t_iCount++;
						else if (t_iDiffTime < 10)
						{
							if (t_pDxva2->m_pUncompBufQueue->count > 8) //Display Queue <= 3
								WaitForSingleObject(stream_global->hFinishDisplaySemaphore, (30*g_PulldownRate)/1000);
							else
								WaitForSingleObject(stream_global->hFinishDisplaySemaphore, (50*g_PulldownRate)/1000);
						}
					}
					else
						t_iCount--;
				}

				for (i=0; i<t_dwNumBuffers; i++)
				{
					if (FAILED(t_pDxva2->ReleaseBuffer(t_pBufferInfo[i].dwTypeIndex)))
						break;
				}

				hr |= t_pDxva2->Execute(0, &t_pDxvaBufferDescription[0], t_dwNumBuffers*sizeof(DXVA_BufferDescription), NULL, 0, t_dwNumBuffers, &t_pBufferInfo[0]);
				hr |= t_pDxva2->EndFrame();
			}

			//for smart decoding
			if(!cur_img.nal_reference_idc && cur_img.currentSlice_structure != FRAME && cur_img.currentSlice_picture_type == B_SLICE && (cur_img.smart_dec & SMART_DEC_SKIP_FIL_B))
			{
				DEBUG_SHOW_HW_INFO("Execute Threading on ATI SmartDec");
				t_bDropBottomB = TRUE;

				t_pDxva2->BeginFrame(cur_img.UnCompress_Idx, 0);

				((DXVA_PictureParameters_H264*)cur_img.m_pnv1PictureDecode)->bPicStructure = cur_img.currentSlice_structure==1 ? 0x02:0x01;
				t_pDxva2->GetBuffer(DXVA2_PictureParametersBufferType, (LPVOID*)(&t_lpPicDec), 0);
				memcpy(t_lpPicDec, cur_img.m_pnv1PictureDecode, sizeof(DXVA_PictureParameters_H264));
				t_pDxva2->GetBuffer(DXVA2_MacroBlockControlBufferType, (LPVOID*)(&t_lpMbCtl), 0);
				memcpy(t_lpMbCtl, cur_img.m_lpMBLK_Intra_Luma, cur_img.m_SBBufOffset+cur_img.width*cur_img.m_PicHeight/8);
				t_pDxva2->GetBuffer(DXVA2_ResidualDifferenceBufferType, (LPVOID*)(&t_lpResdual), 0);
				memcpy(t_lpResdual, cur_img.m_lpRESD_Intra_Luma, cur_img.m_ReBufStride*cur_img.m_PicHeight*3/2);
				t_pDxva2->GetBuffer(DXVA2_SliceControlBufferType, (LPVOID*)(&t_lpSliCtl), 0);
				memcpy(t_lpSliCtl, cur_img.m_lpSLICE, sizeof(DXVA_SliceParameter_H264));
				if(t_dwNumBuffers==5)
				{
					t_pDxva2->GetBuffer(DXVA2_MotionVectorBuffer, (LPVOID*)(&t_lpMv), 0);
					memcpy(t_lpMv, cur_img.m_lpMV, cur_img.m_MVBufStride*cur_img.m_PicHeight/2);
				}

				if(cur_img.m_pIviCP != NULL)
				{
					int scramble_size_luma;
					int scramble_size_chroma;
					if(cur_img.m_PicHeight <=1024)
					{
						scramble_size_luma = cur_img.m_ReBufStride*cur_img.m_PicHeight;
						scramble_size_chroma = cur_img.m_ReBufStride*cur_img.m_PicHeight/2;
					}
					else
					{
						scramble_size_luma = cur_img.m_ReBufStride*1024;
						scramble_size_chroma = cur_img.m_ReBufStride*((cur_img.m_PicHeight-1024) + cur_img.m_PicHeight/2);
					}

					if(cur_img.m_pIviCP != NULL)
					{					
						if(cur_img.m_pIviCP->IsScramblingRequired(MAP_CP_PIC_TYPE(cur_img.currentSlice_picture_type)))
						{
							cur_img.m_pIviCP->EnableScrambling();
							cur_img.m_pIviCP->ScrambleData(t_lpResdual, t_lpResdual, scramble_size_luma+scramble_size_chroma);												
						}
						else
						{
							cur_img.m_pIviCP->DisableScrambling();
						}

						if(cur_img.m_pIviCP->GetObjID()==E_CP_ID_WIN7)
						{
							t_pDxva2->BeginFrame(cur_img.UnCompress_Idx, 0);
							t_pDxva2->EndFrame();
						}
					}
				}

				for (i=0; i<t_dwNumBuffers; i++)
				{
					if (FAILED(t_pDxva2->ReleaseBuffer(t_pBufferInfo[i].dwTypeIndex)))
						break;
				}

				hr |= t_pDxva2->Execute(0, &t_pDxvaBufferDescription[0], t_dwNumBuffers*sizeof(DXVA_BufferDescription), NULL, 0, t_dwNumBuffers, &t_pBufferInfo[0]);
				hr |= t_pDxva2->EndFrame();
			}
		}
		else
		{
			DEBUG_SHOW_HW_INFO("Execute Threading on nVidia");

			hr = t_pDxva2->BeginFrame(cur_img.UnCompress_Idx, 0);

			if(SUCCEEDED(hr))
			{
				t_pDxvaBufferDescription[0].dwTypeIndex				= t_pBufferInfo[0].dwTypeIndex	= DXVA2_PictureParametersBufferType;
				t_pDxvaBufferDescription[0].dwDataSize				= t_pBufferInfo[0].dwDataSize		= sizeof(DXVA_PicParams_H264);
				t_pDxvaBufferDescription[0].dwFirstMBaddress	= 0;
				t_pDxvaBufferDescription[0].dwNumMBsInBuffer	= cur_img.PicSizeInMbs;//(cur_img.currentSlice_structure ==0 ? cur_img.FrameSizeInMbs : (cur_img.FrameSizeInMbs>>1));
				//t_pDxva2->Dump(cur_img.m_pnv1PictureDecode, sizeof(DXVA_PicParams_H264), cur_img.m_lFrame_Counter+1, "PIC.dat");

				t_pDxvaBufferDescription[1].dwTypeIndex				= t_pBufferInfo[1].dwTypeIndex	= DXVA2_SliceControlBufferType;
				t_pDxvaBufferDescription[1].dwDataSize				= t_pBufferInfo[1].dwDataSize		= t_iNumSlice*sizeof(DXVA_Slice_H264_Long);
				t_pDxvaBufferDescription[1].dwFirstMBaddress	= 0;
				t_pDxvaBufferDescription[1].dwNumMBsInBuffer	= cur_img.PicSizeInMbs;//(cur_img.currentSlice_structure ==0 ? cur_img.FrameSizeInMbs : (cur_img.FrameSizeInMbs>>1));
				//t_pDxva2->Dump(cur_img.m_lpSLICE, t_pDxvaBufferDescription[1].dwDataSize, cur_img.m_lFrame_Counter+1, "SLI");

				t_pDxvaBufferDescription[2].dwTypeIndex				= t_pBufferInfo[2].dwTypeIndex	= DXVA2_MacroBlockControlBufferType;
				t_pDxvaBufferDescription[2].dwDataSize				= t_pBufferInfo[2].dwDataSize		= cur_img.m_iIntraMCBufUsage;
				t_pDxvaBufferDescription[2].dwFirstMBaddress	= 0;
				t_pDxvaBufferDescription[2].dwNumMBsInBuffer	= cur_img.PicSizeInMbs;//(cur_img.currentSlice_structure ==0 ? cur_img.FrameSizeInMbs : (cur_img.FrameSizeInMbs>>1));
				// t_pDxva2->Dump(cur_img.m_lpMBLK_Intra_Luma, t_pDxvaBufferDescription[2].dwDataSize, cur_img.m_lFrame_Counter+1, "MC");

				t_pDxvaBufferDescription[3].dwTypeIndex				= t_pBufferInfo[3].dwTypeIndex	= DXVA2_ResidualDifferenceBufferType;
				if(cur_img.m_bResidualDataFormat!=E_RES_DATA_FORMAT_NV)
					t_pDxvaBufferDescription[3].dwDataSize			= t_pBufferInfo[3].dwDataSize		= cur_img.m_iIntraRESBufUsage;
				else
					t_pDxvaBufferDescription[3].dwDataSize			= t_pBufferInfo[3].dwDataSize		= cur_img.PicSizeInMbs*768;
				t_pDxvaBufferDescription[3].dwFirstMBaddress	= 0;
				t_pDxvaBufferDescription[3].dwNumMBsInBuffer	= cur_img.PicSizeInMbs;//(cur_img.currentSlice_structure ==0 ? cur_img.FrameSizeInMbs : (cur_img.FrameSizeInMbs>>1));
				//t_pDxva2->Dump(cur_img.m_lpRESD_Intra_Luma, t_pBufferInfo[3].dwDataSize, "RES");

				if(cur_img.currentSlice_picture_type != I_SLICE && cur_img.m_bResidualDataFormat!=E_RES_DATA_FORMAT_INTEL)
				{
					t_pDxvaBufferDescription[4].dwTypeIndex				= t_pBufferInfo[4].dwTypeIndex	= DXVA2_MotionVectorBuffer;
					t_pDxvaBufferDescription[4].dwDataSize				= t_pBufferInfo[4].dwDataSize		= cur_img.m_iInterMCBufUsage;
					t_pDxvaBufferDescription[4].dwFirstMBaddress	= 0;
					t_pDxvaBufferDescription[4].dwNumMBsInBuffer	= cur_img.PicSizeInMbs;//(cur_img.currentSlice_structure ==0 ? cur_img.FrameSizeInMbs : (cur_img.FrameSizeInMbs>>1));
					t_dwNumBuffers = 5;
					//		Dump ARGS3(cur_img.m_lpMV, cur_img.m_iInterMCBufUsage, "MV");
				}
				else
					t_dwNumBuffers = 4;

				if (cur_img.m_lpQMatrix)
				{
					t_pDxvaBufferDescription[t_dwNumBuffers].dwTypeIndex			= t_pBufferInfo[t_dwNumBuffers].dwTypeIndex	= DXVA2_InverseQuantizationMatrixBufferType;
					t_pDxvaBufferDescription[t_dwNumBuffers].dwDataSize				= t_pBufferInfo[t_dwNumBuffers].dwDataSize  = sizeof(DXVA_Qmatrix_H264);
					t_pDxvaBufferDescription[t_dwNumBuffers].dwFirstMBaddress	= 0;
					t_pDxvaBufferDescription[t_dwNumBuffers].dwNumMBsInBuffer	= 0;
					t_dwNumBuffers++;
				}

				if(!cur_img.m_is_MTMS)
				{
					t_pDxva2->GetBuffer(DXVA2_PictureParametersBufferType, (LPVOID*)(&t_lpPicDec), 0);
					memcpy(t_lpPicDec, cur_img.m_pnv1PictureDecode, sizeof(DXVA_PicParams_H264));
					DUMP_NVIDIA(t_lpPicDec, t_pDxvaBufferDescription[0].dwDataSize, /*m_lFrame_Counter*/cur_img.poc, "PICPAR");

					if (cur_img.m_lpQMatrix)
					{
						t_pDxva2->GetBuffer(DXVA2_InverseQuantizationMatrixBufferType, (LPVOID*)(&t_lpQMatrix), 0);
						memcpy(t_lpQMatrix, cur_img.m_lpQMatrix, sizeof(DXVA_Qmatrix_H264));
						int t_dwNumBuffers_temp = (t_dwNumBuffers-1);
						DUMP_NVIDIA(t_lpQMatrix, t_pDxvaBufferDescription[t_dwNumBuffers_temp].dwDataSize, /*m_lFrame_Counter*/cur_img.poc, "QMatrix");
					}

					t_pDxva2->GetBuffer(DXVA2_MacroBlockControlBufferType, (LPVOID*)(&t_lpMbCtl), 0);
					memcpy(t_lpMbCtl, cur_img.m_lpMBLK_Intra_Luma, cur_img.m_iIntraMCBufUsage);
					DUMP_NVIDIA(t_lpMbCtl, t_pDxvaBufferDescription[2].dwDataSize, /*m_lFrame_Counter*/cur_img.poc, "MBCtrl");

					t_pDxva2->GetBuffer(DXVA2_ResidualDifferenceBufferType, (LPVOID*)(&t_lpResdual), 0);
					if(cur_img.m_bResidualDataFormat!=E_RES_DATA_FORMAT_NV) {
						memcpy(t_lpResdual, cur_img.m_lpRESD_Intra_Luma, cur_img.m_iIntraRESBufUsage);
						DUMP_NVIDIA(t_lpResdual, t_pDxvaBufferDescription[3].dwDataSize, /*m_lFrame_Counter*/cur_img.poc, "ResDiff");
					} else {
						//						memcpy(t_lpResdual, cur_img.m_lpResdual, t_pBufferInfo[3].dwDataSize);
						memcpy(t_lpResdual, cur_img.m_lpRESD_Inter_Luma, cur_img.m_iInterRESBufUsage_L);
						memcpy(t_lpResdual + (cur_img.m_lmbCount_Inter*512), cur_img.m_lpRESD_Inter_Chroma, cur_img.m_iInterRESBufUsage_C);
						memcpy(t_lpResdual + (cur_img.m_lmbCount_Inter*768), cur_img.m_lpRESD_Intra_Luma, cur_img.m_lmbCount_Intra * 512);
						memcpy(t_lpResdual + (cur_img.m_lmbCount_Inter*768 + cur_img.m_lmbCount_Intra*512), cur_img.m_lpRESD_Intra_Chroma, cur_img.m_lmbCount_Intra * 256);

					}
					t_pDxva2->GetBuffer(DXVA2_SliceControlBufferType, (LPVOID*)(&t_lpSliCtl), 0);
					memcpy(t_lpSliCtl, cur_img.m_lpSLICE, t_iNumSlice*sizeof(DXVA_Slice_H264_Long));
					DUMP_NVIDIA(t_lpSliCtl, t_pDxvaBufferDescription[1].dwDataSize, /*m_lFrame_Counter*/cur_img.poc, "SliceCtrl");

					if((t_dwNumBuffers-(cur_img.m_lpQMatrix!=NULL))==5 && cur_img.m_bResidualDataFormat!=E_RES_DATA_FORMAT_INTEL)
					{
						t_pDxva2->GetBuffer(DXVA2_MotionVectorBuffer, (LPVOID*)(&t_lpMv), 0);
						memcpy(t_lpMv, cur_img.m_lpMV, cur_img.m_iInterMCBufUsage);
						DUMP_NVIDIA(t_lpMv, t_pDxvaBufferDescription[4].dwDataSize, /*m_lFrame_Counter*/cur_img.poc, "MV");
					}
				}
				else
				{
					t_pDxva2->GetBuffer(DXVA2_PictureParametersBufferType, (LPVOID*)(&t_pTemp), 0);
					memcpy(t_pTemp,cur_img.m_lpPicDec, sizeof(DXVA_PicParams_H264));
					DUMP_NVIDIA(t_pTemp, t_pDxvaBufferDescription[0].dwDataSize, /*m_lFrame_Counter*/cur_img.poc, "PICPAR");
					//DUMP_NVIDIA(t_pTemp, t_pBufferInfo[0].dwDataSize, g_nCounter, "PICPAR");

					if (cur_img.m_lpQMatrix)
					{
						t_pDxva2->GetBuffer(DXVA2_InverseQuantizationMatrixBufferType, (LPVOID*)(&t_pTemp), 0);
						memcpy(t_pTemp, cur_img.m_lpQMatrixCtl, sizeof(DXVA_Qmatrix_H264));
						int t_dwNumBuffers_temp = (t_dwNumBuffers-1);
						DUMP_NVIDIA(t_pTemp, t_pDxvaBufferDescription[t_dwNumBuffers_temp].dwDataSize, /*m_lFrame_Counter*/cur_img.poc, "QMatrix");
					}
					t_pDxva2->GetBuffer(DXVA2_MacroBlockControlBufferType, (LPVOID*)(&t_pTemp), 0);
					memcpy(t_pTemp,cur_img.m_lpMbCtl, cur_img.PicSizeInMbs * sizeof(DXVA_MBctrl_H264));
					DUMP_NVIDIA(t_pTemp, t_pDxvaBufferDescription[2].dwDataSize, /*m_lFrame_Counter*/cur_img.poc, "MBCtrl");
					//DUMP_NVIDIA(t_pTemp, t_pBufferInfo[2].dwDataSize, g_nCounter, "MBCTRL");

					t_pDxva2->GetBuffer(DXVA2_ResidualDifferenceBufferType, (LPVOID*)(&t_pTemp), 0);
					if(cur_img.m_pIviCP != NULL)
					{
						if(cur_img.m_pIviCP->IsScramblingRequired(MAP_CP_PIC_TYPE(cur_img.currentSlice_picture_type)))
						{			
							cur_img.m_pIviCP->EnableScrambling();
							if(cur_img.m_iVGAType==E_H264_VGACARD_INTEL)						
							{
								// Intel's workaround: Padding zero
								BYTE *t_lpRESDTmpBuf = cur_img.m_lpResdual + cur_img.m_iIntraRESBufUsage;
								UINT uiPadZeroSize = (16 - (cur_img.m_iIntraRESBufUsage % 16)) & 0x0f;
								for(UINT i=0; i<uiPadZeroSize; i++)
									*t_lpRESDTmpBuf++ = 0x00;

								cur_img.m_pIviCP->ScrambleData(cur_img.m_lpResdual, cur_img.m_lpResdual, cur_img.m_iIntraRESBufUsage+uiPadZeroSize);

							}
							else
							{
								cur_img.m_pIviCP->ScrambleData(cur_img.m_lpResdual, cur_img.m_lpResdual, cur_img.m_iIntraRESBufUsage);
							}						
						}
						else
						{
							cur_img.m_pIviCP->DisableScrambling();
						}

						if(cur_img.m_pIviCP->GetObjID()==E_CP_ID_WIN7)
						{
							t_pDxva2->BeginFrame(cur_img.UnCompress_Idx, 0);
							t_pDxva2->EndFrame();
						}
					}
					
					if(cur_img.m_bResidualDataFormat!=E_RES_DATA_FORMAT_NV)
					{
						if (cur_img.m_bResidualDataFormat==E_RES_DATA_FORMAT_INTEL)
						{
							if(cur_img.m_iDXVAMode_temp==E_H264_DXVA_MODE_A)
								memcpy(t_pTemp,cur_img.m_lpResdual, cur_img.PicSizeInMbs * 960);
							else
								memcpy(t_pTemp,cur_img.m_lpResdual, cur_img.PicSizeInMbs * 1728);
						}
						else
							memcpy(t_pTemp,cur_img.m_lpResdual, cur_img.PicSizeInMbs * 768);
					}
					else
					{

						int iNumMbInter = 0;
						int iNumMbIntra = 0;
						int iSliceNum = (cur_img.slice_number>>(int)(cur_img.currentSlice_structure != FRAME));
						BYTE *Inter_L, *Inter_C, *Intra_L, *Intra_C;
						for(i=0 ; i < iSliceNum; i++){
							iNumMbInter += cur_img.m_sResinfo[i].m_lmbCount_Inter;
							iNumMbIntra += cur_img.m_sResinfo[i].m_lmbCount_Intra;
						}

						Inter_L = t_pTemp;
						Inter_C = t_pTemp + (iNumMbInter*512);
						Intra_L = t_pTemp + (iNumMbInter*768);
						Intra_C = t_pTemp + (iNumMbInter*768 + iNumMbIntra*512);
						for(i=0 ; i < iSliceNum ; i++) {

							memcpy(Inter_L, cur_img.m_sResinfo[i].m_lpRESD_Inter_Luma, cur_img.m_sResinfo[i].m_iInterRESBufUsage_L);
							memcpy(Inter_C, cur_img.m_sResinfo[i].m_lpRESD_Inter_Chroma, cur_img.m_sResinfo[i].m_iInterRESBufUsage_C);
							memcpy(Intra_L, cur_img.m_sResinfo[i].m_lpRESD_Intra_Luma, cur_img.m_sResinfo[i].m_lmbCount_Intra * 512);
							memcpy(Intra_C, cur_img.m_sResinfo[i].m_lpRESD_Intra_Chroma, cur_img.m_sResinfo[i].m_lmbCount_Intra * 256);

							Inter_L += cur_img.m_sResinfo[i].m_iInterRESBufUsage_L;
							Inter_C += cur_img.m_sResinfo[i].m_iInterRESBufUsage_C;
							Intra_L += cur_img.m_sResinfo[i].m_lmbCount_Intra * 512;
							Intra_C += cur_img.m_sResinfo[i].m_lmbCount_Intra * 256;

						}

					}
					DUMP_NVIDIA(t_pTemp, t_pDxvaBufferDescription[3].dwDataSize, /*m_lFrame_Counter*/cur_img.poc, "ResDiff");
					//DUMP_NVIDIA(t_pTemp, t_pBufferInfo[3].dwDataSize, g_nCounter, "RESIDUAL");

					t_pDxva2->GetBuffer(DXVA2_SliceControlBufferType, (LPVOID*)(&t_pTemp), 0);
					memcpy(t_pTemp,cur_img.m_lpSliCtl, t_iNumSlice*sizeof(DXVA_Slice_H264_Long));
					DUMP_NVIDIA(t_pTemp, t_pDxvaBufferDescription[1].dwDataSize, /*m_lFrame_Counter*/cur_img.poc, "SliceCtrl");
					//DUMP_NVIDIA(t_pTemp, t_pBufferInfo[1].dwDataSize, g_nCounter, "SLICECTRL");

					if((t_dwNumBuffers-(cur_img.m_lpQMatrix!=NULL))==5 && cur_img.m_bResidualDataFormat!=E_RES_DATA_FORMAT_INTEL)
					{
						t_pDxva2->GetBuffer(DXVA2_MotionVectorBuffer, (LPVOID*)(&t_pTemp), 0);
						memcpy(t_pTemp, cur_img.m_lpMv,cur_img.PicSizeInMbs * 32);
						DUMP_NVIDIA(t_pTemp, t_pDxvaBufferDescription[4].dwDataSize, /*m_lFrame_Counter*/cur_img.poc, "MV");
						//DUMP_NVIDIA(t_pTemp, t_pBufferInfo[4].dwDataSize, g_nCounter, "MVBUFFER");
					}
				}				

				if (g_framerate1000<30000)
				{
					if (t_iCount<0 && (cur_img.currentSlice_structure==FRAME || cur_img.m_pic_combine_status))
					{
						if (t_pDxva2->m_pUncompBufQueue->count > 8) //Display Queue <= 3
							WaitForSingleObject(stream_global->hFinishDisplaySemaphore, (30*g_PulldownRate)/1000);
						else
							WaitForSingleObject(stream_global->hFinishDisplaySemaphore, (50*g_PulldownRate)/1000);

						QueryPerformanceCounter(&stream_global->m_liRecodeTime1);
						t_iDiffTime = (stream_global->m_iEstimateNextFrame - ((int)(1000 * (stream_global->m_liRecodeTime1.QuadPart - stream_global->m_liRecodeTime0.QuadPart))/t_pDxva2->m_pUncompBufQueue->m_freq.QuadPart));
						DEBUG_SHOW_HW_INFO("Diff Time: %d, count: %d queue size: %d", t_iDiffTime, t_pDxva2->m_pUncompBufQueue->count, MemQueue->size());

						if (t_iDiffTime > 50)
							t_iCount++;
						else if (t_iDiffTime < 10)
						{
							if (t_pDxva2->m_pUncompBufQueue->count > 8) //Display Queue <= 3
								WaitForSingleObject(stream_global->hFinishDisplaySemaphore, (30*g_PulldownRate)/1000);
							else
								WaitForSingleObject(stream_global->hFinishDisplaySemaphore, (50*g_PulldownRate)/1000);
						}
					}
					else
						t_iCount--;
				}

				for (i=0; i<t_dwNumBuffers; i++)
				{
					if (FAILED(t_pDxva2->ReleaseBuffer(t_pBufferInfo[i].dwTypeIndex)))
						break;
				}

				hr |= t_pDxva2->Execute(0, &t_pDxvaBufferDescription[0], t_dwNumBuffers*sizeof(DXVA_BufferDescription), NULL, 0, t_dwNumBuffers, &t_pBufferInfo[0]);
				hr |= t_pDxva2->EndFrame();
			}
			//for smart decoding
			if(!cur_img.nal_reference_idc && cur_img.currentSlice_structure != FRAME && cur_img.currentSlice_picture_type == B_SLICE && (cur_img.smart_dec & SMART_DEC_SKIP_FIL_B))
			{
				DEBUG_SHOW_HW_INFO("Execute Threading on nVidia SmartDec");
				t_bDropBottomB = TRUE;

				hr=t_pDxva2->BeginFrame(cur_img.UnCompress_Idx, 0);

				if(SUCCEEDED(hr))
				{
					if(!cur_img.m_is_MTMS)
					{
						((DXVA_PicParams_H264*)cur_img.m_pnv1PictureDecode)->CurrPic.AssociatedFlag = (cur_img.currentSlice_structure==1 ? 1 : 0);
						t_pDxva2->GetBuffer(DXVA2_PictureParametersBufferType, (LPVOID*)(&t_lpPicDec), 0);
						memcpy(t_lpPicDec, cur_img.m_pnv1PictureDecode, sizeof(DXVA_PicParams_H264));
						DUMP_NVIDIA(t_lpPicDec, t_pDxvaBufferDescription[0].dwDataSize, /*m_lFrame_Counter*/cur_img.poc, "PICPAR");

						if (cur_img.m_lpQMatrix)
						{
							t_pDxva2->GetBuffer(DXVA2_InverseQuantizationMatrixBufferType, (LPVOID*)(&t_lpQMatrix), 0);
							memcpy(t_lpQMatrix, cur_img.m_lpQMatrix, sizeof(DXVA_Qmatrix_H264));
							int t_dwNumBuffers_temp = (t_dwNumBuffers-1);
							DUMP_NVIDIA(t_lpQMatrix, t_pDxvaBufferDescription[t_dwNumBuffers_temp].dwDataSize, /*m_lFrame_Counter*/cur_img.poc, "QMatrix");
						}

						t_pDxva2->GetBuffer(DXVA2_MacroBlockControlBufferType, (LPVOID*)(&t_lpMbCtl), 0);
						memcpy(t_lpMbCtl, cur_img.m_lpMBLK_Intra_Luma, cur_img.m_iIntraMCBufUsage);
						DUMP_NVIDIA(t_lpMbCtl, t_pDxvaBufferDescription[2].dwDataSize, /*m_lFrame_Counter*/cur_img.poc, "MBCtrl");

						t_pDxva2->GetBuffer(DXVA2_ResidualDifferenceBufferType, (LPVOID*)(&t_lpResdual), 0);
						if(cur_img.m_bResidualDataFormat!=E_RES_DATA_FORMAT_NV) {
							memcpy(t_lpResdual, cur_img.m_lpRESD_Intra_Luma, cur_img.m_iIntraRESBufUsage);
							DUMP_NVIDIA(t_lpResdual, t_pDxvaBufferDescription[3].dwDataSize, /*m_lFrame_Counter*/cur_img.poc, "ResDiff");
						} else {
							//							memcpy(t_lpResdual, cur_img.m_lpResdual, t_pBufferInfo[3].dwDataSize);
							memcpy(t_lpResdual, cur_img.m_lpRESD_Inter_Luma, cur_img.m_iInterRESBufUsage_L);
							memcpy(t_lpResdual + (cur_img.m_lmbCount_Inter*512), cur_img.m_lpRESD_Inter_Chroma, cur_img.m_iInterRESBufUsage_C);
							memcpy(t_lpResdual + (cur_img.m_lmbCount_Inter*768), cur_img.m_lpRESD_Intra_Luma, cur_img.m_lmbCount_Intra * 512);
							memcpy(t_lpResdual + (cur_img.m_lmbCount_Inter*768 + cur_img.m_lmbCount_Intra*512), cur_img.m_lpRESD_Intra_Chroma, cur_img.m_lmbCount_Intra * 256);	

						}

						t_pDxva2->GetBuffer(DXVA2_SliceControlBufferType, (LPVOID*)(&t_lpSliCtl), 0);
						memcpy(t_lpSliCtl, cur_img.m_lpSLICE, t_iNumSlice*sizeof(DXVA_Slice_H264_Long));
						DUMP_NVIDIA(t_lpSliCtl, t_pDxvaBufferDescription[1].dwDataSize, /*m_lFrame_Counter*/cur_img.poc, "SliceCtrl");

						if((t_dwNumBuffers-(cur_img.m_lpQMatrix!=NULL))==5 && cur_img.m_bResidualDataFormat!=E_RES_DATA_FORMAT_INTEL)
						{
							t_pDxva2->GetBuffer(DXVA2_MotionVectorBuffer, (LPVOID*)(&t_lpMv), 0);
							memcpy(t_lpMv, cur_img.m_lpMV, cur_img.m_iInterMCBufUsage);
							DUMP_NVIDIA(t_lpMv, t_pDxvaBufferDescription[4].dwDataSize, /*m_lFrame_Counter*/cur_img.poc, "MV");
						}
					}
					else 
					{
						((DXVA_PicParams_H264*)cur_img.m_lpPicDec)->CurrPic.AssociatedFlag = (cur_img.currentSlice_structure == 1 ? 1 : 0);
						t_pDxva2->GetBuffer(DXVA2_PictureParametersBufferType, (LPVOID*)(&t_pTemp), 0);
						memcpy(t_pTemp,cur_img.m_lpPicDec, sizeof(DXVA_PicParams_H264));
						DUMP_NVIDIA(t_pTemp, t_pDxvaBufferDescription[0].dwDataSize, /*m_lFrame_Counter*/cur_img.poc, "PICPAR");

						if (cur_img.m_lpQMatrix)
						{
							t_pDxva2->GetBuffer(DXVA2_InverseQuantizationMatrixBufferType, (LPVOID*)(&t_pTemp), 0);
							memcpy(t_pTemp, cur_img.m_lpQMatrixCtl, sizeof(DXVA_Qmatrix_H264));
							int t_dwNumBuffers_temp = (t_dwNumBuffers-1);
							DUMP_NVIDIA(t_pTemp, t_pDxvaBufferDescription[t_dwNumBuffers_temp].dwDataSize, /*m_lFrame_Counter*/cur_img.poc, "QMatrix");
						}
						t_pDxva2->GetBuffer(DXVA2_MacroBlockControlBufferType, (LPVOID*)(&t_pTemp), 0);
						memcpy(t_pTemp,cur_img.m_lpMbCtl, cur_img.PicSizeInMbs * sizeof(DXVA_MBctrl_H264));
						DUMP_NVIDIA(t_pTemp, t_pDxvaBufferDescription[2].dwDataSize, /*m_lFrame_Counter*/cur_img.poc, "MBCtrl");

						t_pDxva2->GetBuffer(DXVA2_ResidualDifferenceBufferType, (LPVOID*)(&t_pTemp), 0);

						if(cur_img.m_pIviCP != NULL)
						{
							if(cur_img.m_pIviCP->IsScramblingRequired(MAP_CP_PIC_TYPE(cur_img.currentSlice_picture_type)))
							{
								cur_img.m_pIviCP->EnableScrambling();
								if(cur_img.m_iVGAType==E_H264_VGACARD_INTEL)
								{
									// Intel's workaround: Padding zero
									BYTE *t_lpRESDTmpBuf = cur_img.m_lpResdual + cur_img.m_iIntraRESBufUsage;
									UINT uiPadZeroSize = (16 - (cur_img.m_iIntraRESBufUsage % 16)) & 0x0f;
									for(UINT i=0; i<uiPadZeroSize; i++)
										*t_lpRESDTmpBuf++ = 0x00;

									cur_img.m_pIviCP->ScrambleData(cur_img.m_lpResdual, cur_img.m_lpResdual, cur_img.m_iIntraRESBufUsage+uiPadZeroSize);

								}
								else
								{
									cur_img.m_pIviCP->ScrambleData(cur_img.m_lpResdual, cur_img.m_lpResdual, cur_img.m_iIntraRESBufUsage);
								}
							}
							else
							{
								cur_img.m_pIviCP->DisableScrambling();
							}

							if(cur_img.m_pIviCP->GetObjID()==E_CP_ID_WIN7)
							{
								t_pDxva2->BeginFrame(cur_img.UnCompress_Idx, 0);
								t_pDxva2->EndFrame();
							}
						}

						if(cur_img.m_bResidualDataFormat!=E_RES_DATA_FORMAT_NV)
						{
							if (cur_img.m_bResidualDataFormat==E_RES_DATA_FORMAT_INTEL)
							{
								if(cur_img.m_iDXVAMode_temp==E_H264_DXVA_MODE_A)
									memcpy(t_pTemp,cur_img.m_lpResdual, cur_img.PicSizeInMbs * 960);
								else
									memcpy(t_pTemp,cur_img.m_lpResdual, cur_img.PicSizeInMbs * 1728);
							}
							else
								memcpy(t_pTemp,cur_img.m_lpResdual, cur_img.PicSizeInMbs * 768);
							DUMP_NVIDIA(t_pTemp, t_pDxvaBufferDescription[3].dwDataSize, /*m_lFrame_Counter*/cur_img.poc, "ResDiff");
						}
						else
						{

							int iNumMbInter = 0;
							int iNumMbIntra = 0;
							int iSliceNum = (cur_img.slice_number>>(int)(cur_img.currentSlice_structure != FRAME));
							BYTE *Inter_L, *Inter_C, *Intra_L, *Intra_C;
							for(i=0 ; i < iSliceNum; i++){
								iNumMbInter += cur_img.m_sResinfo[i].m_lmbCount_Inter;
								iNumMbIntra += cur_img.m_sResinfo[i].m_lmbCount_Intra;
							}

							Inter_L = t_pTemp;
							Inter_C = t_pTemp + (iNumMbInter*512);
							Intra_L = t_pTemp + (iNumMbInter*768);
							Intra_C = t_pTemp + (iNumMbInter*768 + iNumMbIntra*512);
							for(i=0 ; i < iSliceNum ; i++) {
								memcpy(Inter_L, cur_img.m_sResinfo[i].m_lpRESD_Inter_Luma, cur_img.m_sResinfo[i].m_iInterRESBufUsage_L);
								memcpy(Inter_C, cur_img.m_sResinfo[i].m_lpRESD_Inter_Chroma, cur_img.m_sResinfo[i].m_iInterRESBufUsage_C);								
								memcpy(Intra_L, cur_img.m_sResinfo[i].m_lpRESD_Intra_Luma, cur_img.m_sResinfo[i].m_lmbCount_Intra * 512);
								memcpy(Intra_C, cur_img.m_sResinfo[i].m_lpRESD_Intra_Chroma, cur_img.m_sResinfo[i].m_lmbCount_Intra * 256);

								Inter_L += cur_img.m_sResinfo[i].m_iInterRESBufUsage_L;
								Inter_C += cur_img.m_sResinfo[i].m_iInterRESBufUsage_C;
								Intra_L += cur_img.m_sResinfo[i].m_lmbCount_Intra * 512;
								Intra_C += cur_img.m_sResinfo[i].m_lmbCount_Intra * 256;

							}

						}
						t_pDxva2->GetBuffer(DXVA2_SliceControlBufferType, (LPVOID*)(&t_pTemp), 0);
						memcpy(t_pTemp,cur_img.m_lpSliCtl, t_iNumSlice*sizeof(DXVA_Slice_H264_Long));
						DUMP_NVIDIA(t_pTemp, t_pDxvaBufferDescription[1].dwDataSize, /*m_lFrame_Counter*/cur_img.poc, "SliceCtrl");

						if((t_dwNumBuffers-(cur_img.m_lpQMatrix!=NULL))==5 && cur_img.m_bResidualDataFormat!=E_RES_DATA_FORMAT_INTEL)
						{
							t_pDxva2->GetBuffer(DXVA2_MotionVectorBuffer, (LPVOID*)(&t_pTemp), 0);
							memcpy(t_pTemp, cur_img.m_lpMv,cur_img.PicSizeInMbs * 32);
							DUMP_NVIDIA(t_pTemp, t_pDxvaBufferDescription[4].dwDataSize, /*m_lFrame_Counter*/cur_img.poc, "MV");
						}
					}

					for (i=0; i<t_dwNumBuffers; i++)
					{
						if (FAILED(t_pDxva2->ReleaseBuffer(t_pBufferInfo[i].dwTypeIndex)))
							break;
					}

					hr |= t_pDxva2->Execute(0, &t_pDxvaBufferDescription[0], t_dwNumBuffers*sizeof(DXVA_BufferDescription), NULL, 0, t_dwNumBuffers, &t_pBufferInfo[0]);
					hr |= t_pDxva2->EndFrame();
				}
			} //end for smart decoding
		}

		t_pDxva2->m_bCompBufStaus[cur_img.m_Intra_lCompBufIndex] = TRUE;

		if (cur_img.currentSlice_structure==FRAME || cur_img.m_pic_combine_status==0 || t_bDropBottomB)
			SetEvent(stream_global->hReadyForRender[cur_img.UnCompress_Idx]);
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////
// CH264DXVA2_AMD Implementation
CH264DXVA2_ATI::CH264DXVA2_ATI()
{

}

CH264DXVA2_ATI::~CH264DXVA2_ATI()
{

}

CREL_RETURN CH264DXVA2_ATI::decode_one_picture_short PARGS2(int* header, BOOL bSkip)
{
	DecodingEnvironment *dep;
	DXVA_Slice_H264_Short *BA_DATA_CTRL;
	int AU_HasSPS = 0;
	int primary_pic_type = -1;
	int new_pps = 0;
	CREL_RETURN ret = CREL_OK;
	int shiftoffset;
	int nalu_size;
	Slice *newSlice;
	static BOOL bSeekIDR_backup;

	BOOL m_newpps = false;
	BOOL m_newsps = false;
	BOOL bFirstSlice = (IMGPAR slice_number==0) ? TRUE:FALSE;

	if (bFirstSlice)
		nalu->buf = IMGPAR ori_nalu_buf;

	AU_HasSPS	= nalu_global->AU_HasSPS;
	nalu->startcodeprefix_len	= nalu_global->startcodeprefix_len;
	nalu->len					= nalu_global->len;
	nalu->max_size				= nalu_global->max_size;
	nalu->nal_unit_type			= nalu_global->nal_unit_type;
	nalu->nal_reference_idc		= nalu_global->nal_reference_idc;
	nalu->forbidden_bit			= nalu_global->forbidden_bit;
	nalu->pts					= nalu_global->pts;
	memcpy(nalu->buf, nalu_global->buf, nalu_global->len);

	nalu_global_available = 0;			
	if(nalu->nal_unit_type == NALU_TYPE_SPS)
	{
		m_newsps = false;
		m_newpps = false;
	}
	else if(nalu->nal_unit_type == NALU_TYPE_PPS)
		m_newpps = false;

	switch (nalu->nal_unit_type)
	{
	case NALU_TYPE_SLICE:
	case NALU_TYPE_IDR:
		DEBUG_SHOW_HW_INFO("-- NALU_TYPE_SLICE -- NALU_TYPE_IDR --");

		ret = malloc_new_slice(&newSlice);
		if (FAILED(ret)) {
			break;
		}

		if(nalu->nal_unit_type == NALU_TYPE_IDR)
		{
			DEBUG_SHOW_HW_INFO("-- This is IDR --");
			IMGPAR idr_flag = newSlice->idr_flag = 1;
		}
		else
			IMGPAR idr_flag = newSlice->idr_flag = 0;

		IMGPAR nal_reference_idc = newSlice->nal_reference_idc = nalu->nal_reference_idc;
		IMGPAR disposable_flag = newSlice->disposable_flag = (nalu->nal_reference_idc == NALU_PRIORITY_DISPOSABLE);

		newSlice->dp_mode = PAR_DP_1;
		newSlice->max_part_nr = 1;
		newSlice->ei_flag = 0;
		newSlice->nextSlice = NULL;
		dep              = newSlice->g_dep;
		dep->Dei_flag    = 0;
		dep->Dbits_to_go = 0;
		dep->Dbuffer     = 0;						
		dep->Dbasestrm = &(nalu->buf[1]);
		dep->Dstrmlength = nalu->len-1;
		ret = RBSPtoSODB(dep->Dbasestrm, &(dep->Dstrmlength));
		if (FAILED(ret)) {
			break;
		}
		dep->Dcodestrm   = dep->Dbasestrm;

		if (nalu->nal_unit_type == NALU_TYPE_SLICE_EXT) {

			CopyNaluExtToSlice ARGS1 (newSlice);
			newSlice->bIsBaseView = false;
			stream_global->nalu_mvc_extension.bIsPrefixNALU = FALSE;
			//stream_global->nalu_mvc_extension.valid = FALSE;

		}
		else
		{
			newSlice->bIsBaseView = true;
			if(stream_global->m_active_sps_on_view[1] == 0)
			{
				newSlice->viewId = GetBaseViewId ARGS0 ();
				if(stream_global->nalu_mvc_extension.valid && stream_global->nalu_mvc_extension.bIsPrefixNALU && newSlice->viewId == stream_global->nalu_mvc_extension.viewId)
				{
					newSlice->interViewFlag = stream_global->nalu_mvc_extension.interViewFlag;
					newSlice->anchorPicFlag = stream_global->nalu_mvc_extension.anchorPicFlag;
				}
				else
				{
					newSlice->interViewFlag = 1;
					newSlice->anchorPicFlag = 0;
				}
			}
			else
			{
				if(stream_global->nalu_mvc_extension.valid && stream_global->nalu_mvc_extension.bIsPrefixNALU)
				{
					newSlice->viewId = stream_global->nalu_mvc_extension.viewId;
					newSlice->interViewFlag = stream_global->nalu_mvc_extension.interViewFlag;
					newSlice->anchorPicFlag = stream_global->nalu_mvc_extension.anchorPicFlag;
				}
				else
				{
					newSlice->viewId = stream_global->m_active_sps_on_view[1]->view_id[0];
					newSlice->interViewFlag = 1;
					newSlice->anchorPicFlag = 0;
				}
			}
		}
		newSlice->viewIndex = GetViewIndex ARGS1 (newSlice->viewId);

		stream_global->m_pbValidViews[newSlice->viewIndex] = 1;
		if(stream_global->m_CurrOutputViewIndex == -1)
			stream_global->m_CurrOutputViewIndex = newSlice->viewIndex;

		if(bFirstSlice)
		{				
			IMGPAR prevSlice  = NULL;
			IMGPAR firstSlice = newSlice;
			IMGPAR currentSlice  = IMGPAR firstSlice;

			//Only for ATI
			g_bRewindDecoder = FALSE;
		}
		else
		{
			IMGPAR prevSlice  = IMGPAR currentSlice;
			IMGPAR currentSlice->nextSlice = (void*)newSlice;
			IMGPAR currentSlice = (Slice*) (IMGPAR currentSlice->nextSlice);
		}

		if (bFirstSlice) {
			bSeekIDR_backup = stream_global->m_bSeekIDR;
		}	

		ret = ParseSliceHeader ARGS0();
		if ( FAILED(ret) ) {

			if (bSeekIDR_backup) {
				free_new_slice(newSlice);
				newSlice = NULL;		

				stream_global->m_bSeekIDR = TRUE;

				nalu->buf = IMGPAR ori_nalu_buf;					
				IMGPAR slice_number = 0;
				break;

			}			

			free_new_slice(newSlice);
			newSlice = NULL;					

			if (stream_global->m_bSeekIDR) {
				nalu->buf = IMGPAR ori_nalu_buf;					
				IMGPAR slice_number = 0;

			} else if (nalu->nal_unit_type == NALU_TYPE_IDR) {
				stream_global->m_bSeekIDR = TRUE;
				nalu->buf = IMGPAR ori_nalu_buf;

			}				

			break;

		}		

		IMGPAR currentSlice->AU_type = IMGPAR currentSlice->picture_type;			
		if(AU_HasSPS && IMGPAR currentSlice->AU_type==I_SLICE)
			IMGPAR currentSlice->AU_type = I_GOP;			
		if(IMGPAR currentSlice->idr_flag)
			IMGPAR currentSlice->AU_type = IDR_SLICE;
		if(IMGPAR currentSlice->picture_type == B_SLICE && IMGPAR currentSlice->nal_reference_idc )
			IMGPAR currentSlice->AU_type = RB_SLICE;

		DEBUG_SHOW_HW_INFO("This Slice AU Type: %d", IMGPAR currentSlice->AU_type);

		if (stream_global->bSeekToOpenGOP)
		{
			if (IMGPAR currentSlice->picture_type != B_SLICE)
			{
				stream_global->bSeekToOpenGOP = 0;
				next_image_type = IMGPAR type;
				bSkip = 0; //reset skip flag
			}
			else/* if (IMGPAR currentSlice->AU_type != RB_SLICE)*/
			{
				nalu->buf = IMGPAR ori_nalu_buf;
				if (IMGPAR prevSlice)
				{
					IMGPAR currentSlice = IMGPAR prevSlice;
					IMGPAR currentSlice->nextSlice = NULL;
				}
				free_new_slice(newSlice);
				newSlice = NULL;

				if (g_has_pts)
				{
					g_has_pts = 0;
					DEBUG_INFO("[Drop PTS]: ts: %I64d freq: %d, tslength: %d flags: %d unused1: %d unused2: %d", nalu->pts.ts, nalu->pts.freq, nalu->pts.tslength, nalu->pts.flags, nalu->pts.unused1, nalu->pts.unused2);
				}
				break;
			}
		}							

		//Only for ATI
		seq_parameter_set_rbsp_t *sps;
		if(g_bRewindDecoder == FALSE)
		{
			sps =  (IMGPAR currentSlice->bIsBaseView) ? &SeqParSet[PicParSet[IMGPAR currentSlice->pic_parameter_set_id].seq_parameter_set_id] :
				&SeqParSubset[PicParSet[IMGPAR currentSlice->pic_parameter_set_id].seq_parameter_set_id];
			activate_sps ARGS2(sps, 0);

			if(newSlice!=NULL && IMGPAR Hybrid_Decoding == 5 
				&& (!(IMGPAR structure == FRAME && IMGPAR height ==1088) || IMGPAR currentSlice->MbaffFrameFlag))
			{
				g_bRewindDecoder = TRUE;
#ifdef IP_RD_MERGE	
				g_pfnDecodePicture = DecodePicture_MultiThread_SingleSlice_IPRD_Merge;
#else
				g_pfnDecodePicture = DecodePicture_MultiThread_SingleSlice_IPRD_Seperate;
#endif

				memcpy(nalu_global->buf, nalu->buf, nalu->len);
				nalu_global->startcodeprefix_len	= nalu->startcodeprefix_len;
				nalu_global->len					= nalu->len;
				nalu_global->max_size				= nalu->max_size;
				nalu_global->nal_unit_type			= nalu->nal_unit_type;
				nalu_global->nal_reference_idc		= nalu->nal_reference_idc;
				nalu_global->forbidden_bit			= nalu->forbidden_bit;
				nalu_global->pts					= nalu->pts;
				nalu_global->AU_HasSPS			    = AU_HasSPS;
				nalu_global_available = 1;
				IMGPAR slice_number = 0;

				stream_global->m_is_MTMS = -1;

				//Yolk: Reset this value!
				stream_global->m_active_sps_on_view[0] = 0;
				free_new_slice(newSlice);
				newSlice = NULL;
				*header = 0;

				return CREL_OK; 
			}
		}

		if ((stream_global->profile_idc == 0) || 
			((g_bReceivedFirst == 0) && (IMGPAR firstSlice->picture_type != I_SLICE))
			)
		{
			nalu->buf = IMGPAR ori_nalu_buf;
			free_new_slice(newSlice);
			newSlice = NULL;
			IMGPAR slice_number = 0;
			m_newpps = FALSE;
			m_newsps = FALSE;
			stream_global->m_active_sps_on_view[0] = 0;

			if (g_has_pts)
			{
				g_has_pts = 0;
				DEBUG_INFO("[Drop PTS]: ts: %I64d freq: %d, tslength: %d flags: %d unused1: %d unused2: %d", nalu->pts.ts, nalu->pts.freq, nalu->pts.tslength, nalu->pts.flags, nalu->pts.unused1, nalu->pts.unused2);
			}
			break;
		}

		if(is_new_picture ARGS0())
		{
			//seq_parameter_set_rbsp_t *sps;
			pic_parameter_set_rbsp_t *pps;

			ret = decode_poc ARGS0();
			if (FAILED(ret)) {

				if (bSeekIDR_backup) {							

					stream_global->m_bSeekIDR = TRUE;
					nalu->buf = IMGPAR ori_nalu_buf;					
					IMGPAR slice_number = 0;

				}

				free_new_slice(newSlice);
				newSlice = NULL;

				break;
			}

			IMGPAR currentSlice->framerate1000 = g_framerate1000;

			if (IMGPAR prevSlice)
				IMGPAR prevSlice->exit_flag = 1;

			*header = SOP;
			//Picture or Field
			if(IMGPAR currentSlice->field_pic_flag)
			{   //Field Picture

				if( (IMGPAR prevSlice!= NULL && IMGPAR prevSlice->field_pic_flag)
					&&					
					( (IMGPAR currentSlice->structure==BOTTOM_FIELD && IMGPAR prevSlice!=NULL && 
					IMGPAR prevSlice->frame_num == IMGPAR currentSlice->frame_num && IMGPAR prevSlice->m_pic_combine_status==TOP_FIELD)
					||
					(IMGPAR currentSlice->structure==TOP_FIELD && IMGPAR prevSlice!=NULL && 
					IMGPAR prevSlice->frame_num == IMGPAR currentSlice->frame_num && IMGPAR prevSlice->m_pic_combine_status==BOTTOM_FIELD) )
					)
				{
					//Second Filed of New Picture
					UpdatePTS ARGS0();

					sps =  (IMGPAR currentSlice->bIsBaseView) ? &SeqParSet[PicParSet[IMGPAR currentSlice->pic_parameter_set_id].seq_parameter_set_id] :
						&SeqParSubset[PicParSet[IMGPAR currentSlice->pic_parameter_set_id].seq_parameter_set_id];
					pps =  &PicParSet[IMGPAR currentSlice->pic_parameter_set_id];
					ret = ret = activate_sps ARGS2(sps, 0);
					if (FAILED(ret)) {
						free_new_slice(newSlice);
						newSlice = NULL;
						stream_global->m_bSeekIDR = TRUE;
						nalu->buf = IMGPAR ori_nalu_buf;					
						IMGPAR slice_number = 0;
						break;
					}
					activate_pps ARGS2(pps, 0);

					if(!bSkip)
					{
						ret = BA_ExecuteBuffers ARGS0();
						if (FAILED(ret)) {
							free_new_slice(newSlice);
							newSlice = NULL;
							stream_global->m_bSeekIDR = TRUE;
							nalu->buf = IMGPAR ori_nalu_buf;					
							IMGPAR slice_number = 0;
							break;
						}

						dec_picture->pic_store_idx = IMGPAR UnCompress_Idx;
						ret = store_picture_in_dpb ARGS1(dec_picture);
						if (FAILED(ret)) {
							free_new_slice(newSlice);
							newSlice = NULL;
							stream_global->m_bSeekIDR = TRUE;
							nalu->buf = IMGPAR ori_nalu_buf;					
							IMGPAR slice_number = 0;
							break;
						}
					}
					//store these parameters to next collect_pic
					stream_global->PreviousFrameNum[0]				= IMGPAR PreviousFrameNum;
					stream_global->PreviousFrameNumOffset[0]	= IMGPAR PreviousFrameNumOffset;
					stream_global->PreviousPOC[0]						= IMGPAR PreviousPOC;
					stream_global->ThisPOC[0]								= IMGPAR ThisPOC; 
					stream_global->PrevPicOrderCntLsb[0]			= IMGPAR PrevPicOrderCntLsb;
					stream_global->PrevPicOrderCntMsb[0]			= IMGPAR PrevPicOrderCntMsb;
					//Combine Two Filed
					IMGPAR currentSlice->header = SOP;
					IMGPAR currentSlice->m_pic_combine_status = 0; //Picture is Full
					IMGPAR slice_number++;
					nalu->buf += nalu->len;

					if (!bSkip)
					{
						ret = initial_image ARGS0();
						if (FAILED(ret)) {
							free_new_slice(newSlice);
							newSlice = NULL;
							stream_global->m_bSeekIDR = TRUE;
							nalu->buf = IMGPAR ori_nalu_buf;					
							IMGPAR slice_number = 0;
							break;
						}

						memcpy(&(dec_picture->m_CCCode), &(IMGPAR m_CCCode), sizeof(H264_CC));

						ret = init_lists ARGS2(IMGPAR type, IMGPAR currentSlice->structure);
						if (FAILED(ret)) {
							free_new_slice(newSlice);
							newSlice = NULL;
							stream_global->m_bSeekIDR = TRUE;
							nalu->buf = IMGPAR ori_nalu_buf;					
							IMGPAR slice_number = 0;
							break;
						}
						ret = reorder_lists ARGS2((IMGPAR type), (IMGPAR currentSlice));
						if (FAILED(ret)) {
							free_new_slice(newSlice);
							newSlice = NULL;
							stream_global->m_bSeekIDR = TRUE;
							nalu->buf = IMGPAR ori_nalu_buf;					
							IMGPAR slice_number = 0;
							break;
						}

						ret = check_lists ARGS0();
						if (FAILED(ret)) {
							free_new_slice(newSlice);
							newSlice = NULL;
							stream_global->m_bSeekIDR = TRUE;
							nalu->buf = IMGPAR ori_nalu_buf;					
							IMGPAR slice_number = 0;
							break;
						}

						g_bReceivedFirst = (!g_bReceivedFirst) ? 1 : 2;

						if (IMGPAR currentSlice->structure == FRAME)
							init_mbaff_lists ARGS0();
						free_ref_pic_list_reordering_buffer(IMGPAR currentSlice);

						IMGPAR m_Intra_lCompBufIndex = 0;
						IMGPAR m_lmbCount_Intra = 0;
						IMGPAR m_iIntraMCBufUsage = 0;
						IMGPAR m_iInterMCBufUsage = 0;
						IMGPAR m_slice_number_in_field = 1;

						BA_build_picture_decode_buffer ARGS0();
						BA_DATA_CTRL = (DXVA_Slice_H264_Short *)m_lpSliceCtrlBuf;
					}
				}
				else if (IMGPAR slice_number == 0)				
				{
					//First Filed of New Picture
					UpdatePTS ARGS0();

					ZeroMemory(&(IMGPAR m_CCCode), sizeof(H264_CC));
					SetH264CCCode ARGS0();

					sps =  (IMGPAR currentSlice->bIsBaseView) ? &SeqParSet[PicParSet[IMGPAR currentSlice->pic_parameter_set_id].seq_parameter_set_id] :
						&SeqParSubset[PicParSet[IMGPAR currentSlice->pic_parameter_set_id].seq_parameter_set_id];
					pps =  &PicParSet[IMGPAR currentSlice->pic_parameter_set_id];
					ret = activate_sps ARGS2(sps, 0);
					if (FAILED(ret)) {
						free_new_slice(newSlice);
						newSlice = NULL;
						stream_global->m_bSeekIDR = TRUE;
						nalu->buf = IMGPAR ori_nalu_buf;					
						IMGPAR slice_number = 0;
						break;
					}
					activate_pps ARGS2(pps, 0);

					IMGPAR currentSlice->header = SOP;
					IMGPAR currentSlice->m_pic_combine_status = IMGPAR currentSlice->structure; //first field
					IMGPAR slice_number++;
					nalu->buf += nalu->len;

					IMGPAR currentSlice->m_nDispPicStructure = sps->vui_seq_parameters.picture_structure;

					DEBUG_SHOW_HW_INFO("##bSkip: %d, Type = %d, POC = %d##", bSkip, IMGPAR firstSlice->AU_type, IMGPAR ThisPOC);

					IMGPAR UnCompress_Idx = -1;
					IMGPAR m_Intra_lCompBufIndex = 0;
					IMGPAR m_lmbCount_Intra = 0;
					IMGPAR m_iIntraMCBufUsage = 0;
					IMGPAR m_iInterMCBufUsage = 0;
					IMGPAR m_slice_number_in_field = 1;

					if(!bSkip)
					{
						if ((IMGPAR firstSlice->picture_type==B_SLICE)  && dpb.used_size_on_view[0])
						{
							StorablePicture *pPreFrame = NULL;


							if ( dpb.fs_on_view[0][dpb.used_size_on_view[0]-1]->frame ) {	// Field Combined or Frame Split work may not be done if that picture has error.
								pPreFrame = dpb.fs_on_view[0][dpb.used_size_on_view[0]-1]->frame;
							} else if (dpb.fs_on_view[0][dpb.used_size_on_view[0]-1]->top_field) {
								pPreFrame = dpb.fs_on_view[0][dpb.used_size_on_view[0]-1]->top_field;
							} else {
								pPreFrame = dpb.fs_on_view[0][dpb.used_size_on_view[0]-1]->bottom_field;
							}

							if(pPreFrame && (pPreFrame->slice_type==P_SLICE || pPreFrame->slice_type==I_SLICE))
							{
								g_pArrangedPCCCode = &(pPreFrame->m_CCCode);
								g_pArrangedPCCCode->ccbuf[LINE21BUF_SIZE-1] = 0;
							}

							if (g_pArrangedPCCCode)
								RearrangePBCCBuf ARGS0();
						}

						ret = initial_image ARGS0();
						if (FAILED(ret)) {
							free_new_slice(newSlice);
							newSlice = NULL;
							stream_global->m_bSeekIDR = TRUE;
							nalu->buf = IMGPAR ori_nalu_buf;					
							IMGPAR slice_number = 0;
							break;
						}

						memcpy(&(dec_picture->m_CCCode), &(IMGPAR m_CCCode), sizeof(H264_CC));

						ret = init_lists ARGS2(IMGPAR type, IMGPAR currentSlice->structure);
						if (FAILED(ret)) {
							free_new_slice(newSlice);
							newSlice = NULL;
							stream_global->m_bSeekIDR = TRUE;
							nalu->buf = IMGPAR ori_nalu_buf;					
							IMGPAR slice_number = 0;
							break;
						}
						ret = reorder_lists ARGS2((IMGPAR type), (IMGPAR currentSlice));
						if (FAILED(ret)) {
							free_new_slice(newSlice);
							newSlice = NULL;
							stream_global->m_bSeekIDR = TRUE;
							nalu->buf = IMGPAR ori_nalu_buf;					
							IMGPAR slice_number = 0;
							break;
						}

						ret = check_lists ARGS0();
						if (FAILED(ret)) {
							free_new_slice(newSlice);
							newSlice = NULL;
							stream_global->m_bSeekIDR = TRUE;
							nalu->buf = IMGPAR ori_nalu_buf;					
							IMGPAR slice_number = 0;
							break;
						}

						if (IMGPAR currentSlice->structure == FRAME)
							init_mbaff_lists ARGS0();
						free_ref_pic_list_reordering_buffer(IMGPAR currentSlice);

						BA_build_picture_decode_buffer ARGS0();
						BA_DATA_CTRL = (DXVA_Slice_H264_Short *)m_lpSliceCtrlBuf;
					}
				}
				else
				{
					//Cpoy naul to nalu_global
					nalu_global->AU_HasSPS	= AU_HasSPS;
					nalu_global->startcodeprefix_len	= nalu->startcodeprefix_len;
					nalu_global->len					= nalu->len;
					nalu_global->max_size				= nalu->max_size;
					nalu_global->nal_unit_type			= nalu->nal_unit_type;
					nalu_global->nal_reference_idc		= nalu->nal_reference_idc;
					nalu_global->forbidden_bit			= nalu->forbidden_bit;
					nalu_global->pts					= nalu->pts;
					memcpy(nalu_global->buf, nalu->buf, nalu->len);
					nalu_global_available = 1;

					IMGPAR PreviousFrameNum				= stream_global->PreviousFrameNum[0];
					IMGPAR PreviousFrameNumOffset = stream_global->PreviousFrameNumOffset[0];
					IMGPAR PreviousPOC						= stream_global->PreviousPOC[0];
					IMGPAR ThisPOC								= stream_global->ThisPOC[0]; 
					IMGPAR PrevPicOrderCntLsb			= stream_global->PrevPicOrderCntLsb[0];
					IMGPAR PrevPicOrderCntMsb			= stream_global->PrevPicOrderCntMsb[0];

					next_image_type = IMGPAR type;

					IMGPAR currentSlice = IMGPAR prevSlice;
					IMGPAR currentSlice->nextSlice = NULL;

					if(!bSkip)
					{
						dec_picture->pic_store_idx = IMGPAR UnCompress_Idx;
						ret = store_picture_in_dpb ARGS1(dec_picture);
						if (FAILED(ret)) {
							free_new_slice(newSlice);
							newSlice = NULL;
							stream_global->m_bSeekIDR = TRUE;
							nalu->buf = IMGPAR ori_nalu_buf;					
							IMGPAR slice_number = 0;
							break;
						}
						*header = SOP;
					}

					free_new_slice(newSlice);
					newSlice = NULL;

					if(stream_global->m_bTrickEOS)
					{
						DEBUG_SHOW_HW_INFO("-- TrickMode EOS after I_SLICE--");
						next_image_type = IMGPAR firstSlice->picture_type;
						nalu_global_available = 0;
						IMGPAR currentSlice->exit_flag = 1;
						*header = EOS;
					}

					return CREL_OK;
				}
			}
			else 
			{   //Frame Picture
				if(IMGPAR slice_number)
				{					
					//Cpoy naul to nalu_global
					nalu_global->AU_HasSPS	= AU_HasSPS;
					nalu_global->startcodeprefix_len	= nalu->startcodeprefix_len;
					nalu_global->len					= nalu->len;
					nalu_global->max_size				= nalu->max_size;
					nalu_global->nal_unit_type			= nalu->nal_unit_type;
					nalu_global->nal_reference_idc		= nalu->nal_reference_idc;
					nalu_global->forbidden_bit			= nalu->forbidden_bit;
					nalu_global->pts					= nalu->pts;
					memcpy(nalu_global->buf, nalu->buf, nalu->len);
					nalu_global_available = 1;

					IMGPAR PreviousFrameNum				= stream_global->PreviousFrameNum[0];
					IMGPAR PreviousFrameNumOffset = stream_global->PreviousFrameNumOffset[0];
					IMGPAR PreviousPOC						= stream_global->PreviousPOC[0];
					IMGPAR ThisPOC								= stream_global->ThisPOC[0]; 
					IMGPAR PrevPicOrderCntLsb			= stream_global->PrevPicOrderCntLsb[0];
					IMGPAR PrevPicOrderCntMsb			= stream_global->PrevPicOrderCntMsb[0];

					next_image_type = IMGPAR type;

					IMGPAR currentSlice = IMGPAR prevSlice;
					IMGPAR currentSlice->nextSlice = NULL;

					if(!bSkip)
					{
						dec_picture->pic_store_idx = IMGPAR UnCompress_Idx;
						ret = store_picture_in_dpb ARGS1(dec_picture);
						if (FAILED(ret)) {
							stream_global->m_bSeekIDR = TRUE;
							nalu->buf = IMGPAR ori_nalu_buf;					
							IMGPAR slice_number = 0;
							break;
						}
						*header = SOP;
					}

					free_new_slice(newSlice);
					newSlice = NULL;

					if(stream_global->m_bTrickEOS)
					{
						DEBUG_SHOW_HW_INFO("-- TrickMode EOS after I_SLICE--");
						next_image_type = IMGPAR firstSlice->picture_type;
						nalu_global_available = 0;
						IMGPAR currentSlice->exit_flag = 1;
						*header = EOS;							
					}

					return CREL_OK;
				}
				else
				{
					UpdatePTS ARGS0();

					ZeroMemory(&(IMGPAR m_CCCode), sizeof(H264_CC));
					SetH264CCCode ARGS0();

					sps =  (IMGPAR currentSlice->bIsBaseView) ? &SeqParSet[PicParSet[IMGPAR currentSlice->pic_parameter_set_id].seq_parameter_set_id] :
						&SeqParSubset[PicParSet[IMGPAR currentSlice->pic_parameter_set_id].seq_parameter_set_id];
					pps =  &PicParSet[IMGPAR currentSlice->pic_parameter_set_id];
					ret = activate_sps ARGS2(sps, 0);
					if (FAILED(ret)) {
						break;
					}
					activate_pps ARGS2(pps, 0);

					IMGPAR currentSlice->header = SOP;		
					IMGPAR currentSlice->m_pic_combine_status = FRAME;
					IMGPAR slice_number++;
					nalu->buf += nalu->len;

					stream_global->PreviousFrameNum[0]				= IMGPAR PreviousFrameNum;
					stream_global->PreviousFrameNumOffset[0]	= IMGPAR PreviousFrameNumOffset;
					stream_global->PreviousPOC[0]						= IMGPAR PreviousPOC;
					stream_global->ThisPOC[0]								= IMGPAR ThisPOC; 
					stream_global->PrevPicOrderCntLsb[0]			= IMGPAR PrevPicOrderCntLsb;
					stream_global->PrevPicOrderCntMsb[0]			= IMGPAR PrevPicOrderCntMsb;	

					IMGPAR currentSlice->m_nDispPicStructure = sps->vui_seq_parameters.picture_structure;

					DEBUG_SHOW_HW_INFO("##bSkip: %d, Type = %d, POC = %d##", bSkip, IMGPAR firstSlice->AU_type, IMGPAR ThisPOC);

					IMGPAR UnCompress_Idx = -1;
					IMGPAR m_Intra_lCompBufIndex = 0;
					IMGPAR m_lmbCount_Intra = 0;
					IMGPAR m_iIntraMCBufUsage = 0;
					IMGPAR m_iInterMCBufUsage = 0;
					IMGPAR m_slice_number_in_field = 1;

					if(!bSkip)
					{
						if ((IMGPAR firstSlice->picture_type==B_SLICE)  && dpb.used_size_on_view[0])
						{
							StorablePicture *pPreFrame = NULL;


							if ( dpb.fs_on_view[0][dpb.used_size_on_view[0]-1]->frame ) {	// Field Combined or Frame Split work may not be done if that picture has error.
								pPreFrame = dpb.fs_on_view[0][dpb.used_size_on_view[0]-1]->frame;
							} else if (dpb.fs_on_view[0][dpb.used_size_on_view[0]-1]->top_field) {
								pPreFrame = dpb.fs_on_view[0][dpb.used_size_on_view[0]-1]->top_field;
							} else {
								pPreFrame = dpb.fs_on_view[0][dpb.used_size_on_view[0]-1]->bottom_field;
							}

							if(pPreFrame && (pPreFrame->slice_type==P_SLICE || pPreFrame->slice_type==I_SLICE))
							{
								g_pArrangedPCCCode = &(pPreFrame->m_CCCode);
								g_pArrangedPCCCode->ccbuf[LINE21BUF_SIZE-1] = 0;
							}

							if (g_pArrangedPCCCode)
								RearrangePBCCBuf ARGS0();
						}

						ret = initial_image ARGS0();
						if (FAILED(ret)) {
							free_new_slice(newSlice);
							newSlice = NULL;
							stream_global->m_bSeekIDR = TRUE;
							nalu->buf = IMGPAR ori_nalu_buf;					
							IMGPAR slice_number = 0;
							break;
						}

						memcpy(&(dec_picture->m_CCCode), &(IMGPAR m_CCCode), sizeof(H264_CC));

						ret = init_lists ARGS2(IMGPAR type, IMGPAR currentSlice->structure);
						if (FAILED(ret)) {
							free_new_slice(newSlice);
							newSlice = NULL;
							stream_global->m_bSeekIDR = TRUE;
							nalu->buf = IMGPAR ori_nalu_buf;					
							IMGPAR slice_number = 0;
							break;
						}
						ret = reorder_lists ARGS2((IMGPAR type), (IMGPAR currentSlice));
						if (FAILED(ret)) {
							free_new_slice(newSlice);
							newSlice = NULL;
							stream_global->m_bSeekIDR = TRUE;
							nalu->buf = IMGPAR ori_nalu_buf;					
							IMGPAR slice_number = 0;
							break;
						}

						ret = check_lists ARGS0();
						if (FAILED(ret)) {
							free_new_slice(newSlice);
							newSlice = NULL;
							stream_global->m_bSeekIDR = TRUE;
							nalu->buf = IMGPAR ori_nalu_buf;					
							IMGPAR slice_number = 0;
							break;
						}

						if (IMGPAR currentSlice->structure == FRAME)
							init_mbaff_lists ARGS0();
						free_ref_pic_list_reordering_buffer(IMGPAR currentSlice);

						BA_build_picture_decode_buffer ARGS0();
						BA_DATA_CTRL = (DXVA_Slice_H264_Short *)m_lpSliceCtrlBuf;

						g_bReceivedFirst = (!g_bReceivedFirst) ? 1 : 2;
					}
				}
			}												
		}
		else
		{
			IMGPAR currentSlice->header = *header = SOS;				
			IMGPAR currentSlice->m_pic_combine_status = IMGPAR prevSlice->m_pic_combine_status;
			IMGPAR slice_number++;
			IMGPAR m_slice_number_in_field++;

			if (IMGPAR prevSlice)
			{
				IMGPAR prevSlice->exit_flag = 0;
				IMGPAR currentSlice->pts = IMGPAR prevSlice->pts;
				IMGPAR currentSlice->dts = IMGPAR prevSlice->dts;
				IMGPAR currentSlice->has_pts = IMGPAR prevSlice->has_pts;
				IMGPAR currentSlice->framerate1000 = IMGPAR prevSlice->framerate1000;
				IMGPAR currentSlice->NumClockTs = IMGPAR prevSlice->NumClockTs;
			}
			nalu->buf += nalu->len;

			IMGPAR currentSlice->m_nDispPicStructure = IMGPAR prevSlice->m_nDispPicStructure;

			BA_DATA_CTRL = (DXVA_Slice_H264_Short *)m_lpSliceCtrlBuf;
		}

		if(!bSkip)
		{
			nalu_size = nalu->len+3 + m_nNALUSkipByte;
			BA_DATA_CTRL->BSNALunitDataLocation = (int)(m_lpBitstreamBuf - IMGPAR m_lpMV);
			shiftoffset = (nalu_size & 127);
			if(shiftoffset)
			{
				memset((m_lpnalu+nalu_size), 0,(128-shiftoffset));
				nalu_size += (128-shiftoffset);
			}
			memcpy(m_lpBitstreamBuf, m_lpnalu, nalu_size);
			m_lpBitstreamBuf += nalu_size;
			BA_DATA_CTRL->SliceBytesInBuffer = nalu_size;
			BA_DATA_CTRL->wBadSliceChopping = 0;
			m_lpSliceCtrlBuf += sizeof(DXVA_Slice_H264_Short);
			BA_DATA_CTRL = (DXVA_Slice_H264_Short *)m_lpSliceCtrlBuf;
			IMGPAR m_iInterMCBufUsage += sizeof(DXVA_Slice_H264_Short);
			IMGPAR m_iIntraMCBufUsage += nalu_size;
			IMGPAR m_lmbCount_Intra++;

			DEBUG_SHOW_HW_INFO("IMGPAR m_iInterMCBufUsage:%d, nalu_size:%d, nalu_len:%d, nalu_skipbyte:%d", IMGPAR m_iInterMCBufUsage, nalu_size, nalu->len, m_nNALUSkipByte);
		}

		break;
	case NALU_TYPE_DPA:
		return CREL_ERROR_H264_SYNCNOTFOUND;
		break;
	case NALU_TYPE_DPC:			
		return CREL_ERROR_H264_SYNCNOTFOUND;
		break;
	case NALU_TYPE_SEI:
		DEBUG_SHOW_HW_INFO("-- NALU_TYPE_SEI --");
		DEBUG_SHOW_HW_INFO ("read_new_slice: Found NALU_TYPE_SEI, len %d\n", nalu->len);
		ret = InterpretSEIMessage ARGS2(nalu->buf,nalu->len);
		break;
	case NALU_TYPE_PPS:
		DEBUG_SHOW_HW_INFO("-- NALU_TYPE_PPS --");
		//if (m_newpps && ((IMGPAR field_pic_flag && new_picture==2) || (!IMGPAR field_pic_flag && new_picture)))
		//{
		//	nalu_global->AU_HasSPS	= AU_HasSPS;
		//	nalu_global->startcodeprefix_len	= nalu->startcodeprefix_len;
		//	nalu_global->len					= nalu->len;
		//	nalu_global->max_size				= nalu->max_size;
		//	nalu_global->nal_unit_type			= nalu->nal_unit_type;
		//	nalu_global->nal_reference_idc		= nalu->nal_reference_idc;
		//	nalu_global->forbidden_bit			= nalu->forbidden_bit;
		//	nalu_global->pts					= nalu->pts;
		//	memcpy(nalu_global->buf, nalu->buf, nalu->len);
		//	nalu_global_available = 1;

		//	if(!bSkip)
		//	{
		//		if (dec_picture)
		//		{
		//			dec_picture->pic_store_idx = IMGPAR UnCompress_Idx;
		//			ret = store_picture_in_dpb ARGS1(dec_picture);
		//			if (FAILED(ret)) {
		//				return ret;
		//			}
		//		}

		//		if (IMGPAR slice_number > 0)
		//			IMGPAR currentSlice->exit_flag = 1;

		//		if (stream_global->m_bTrickEOS)
		//		{
		//			DEBUG_SHOW_HW_INFO("-- TrickMode EOS after PPS--");
		//			next_image_type = IMGPAR firstSlice->picture_type;
		//			nalu_global_available = 0;
		//			*header = EOS;
		//			return CREL_OK;
		//		}
		//	}
		//	*header = SOP;
		//	return CREL_OK;
		//}
		m_newpps = true;
		ret = ProcessPPS ARGS1(nalu);
		break;
	case NALU_TYPE_SPS:
		DEBUG_SHOW_HW_INFO("-- NALU_TYPE_SPS --");
		AU_HasSPS = 1;
		if ( CheckSPS ARGS2(nalu, 0) || (IMGPAR slice_number > 0 && IMGPAR firstSlice->idr_flag) || m_newsps)
		{
			nalu_global->AU_HasSPS	= AU_HasSPS;
			nalu_global->startcodeprefix_len	= nalu->startcodeprefix_len;
			nalu_global->len					= nalu->len;
			nalu_global->max_size				= nalu->max_size;
			nalu_global->nal_unit_type			= nalu->nal_unit_type;
			nalu_global->nal_reference_idc		= nalu->nal_reference_idc;
			nalu_global->forbidden_bit			= nalu->forbidden_bit;
			nalu_global->pts					= nalu->pts;
			memcpy(nalu_global->buf, nalu->buf, nalu->len);
			nalu_global_available = 1;

			if(!bSkip)
			{
				if (dec_picture)
				{
					dec_picture->pic_store_idx = IMGPAR UnCompress_Idx;
					ret = store_picture_in_dpb ARGS1(dec_picture);
					if (FAILED(ret)) {
						stream_global->m_bSeekIDR = TRUE;
						break;
					}
				}

				if (IMGPAR slice_number > 0)
					IMGPAR currentSlice->exit_flag = 1;

				if (stream_global->m_bTrickEOS)
				{
					DEBUG_SHOW_HW_INFO("-- TrickMode EOS after SPS--");
					next_image_type = IMGPAR firstSlice->picture_type;
					nalu_global_available = 0;
					*header = EOS;
					return CREL_OK;
				}
			}
			*header = (m_newsps ? SOP : NALU_TYPE_SPS);
			return CREL_OK;
		}

		m_newsps = true;
		ret = ProcessSPS ARGS1(nalu);
		break;
	case NALU_TYPE_AUD:
		AU_HasSPS = 0;
		ret = ProcessAUD ARGS2(nalu, &primary_pic_type);
		break;
	case NALU_TYPE_EOSEQ:
		DEBUG_SHOW_HW_INFO ("read_new_slice: Found 'End of Sequence' NAL unit, len %d, ignored\n", nalu->len);
		break;
	case NALU_TYPE_EOSTREAM:
		DEBUG_SHOW_HW_INFO ("read_new_slice: Found 'End of Stream' NAL unit, len %d, ignored\n", nalu->len);
		break;
	case NALU_TYPE_FILL:
		DEBUG_SHOW_HW_INFO ("read_new_slice: Found NALU_TYPE_FILL, len %d\n", nalu->len);
		DEBUG_SHOW_HW_INFO ("Skipping these filling bits, proceeding w/ next NALU\n");
		break;
	default:
		DEBUG_SHOW_HW_INFO("-- NALU_TYPE_default --");
		DEBUG_SHOW_HW_INFO("Found NALU type %d, len %d undefined, ignore NALU, moving on\n", nalu->nal_unit_type, nalu->len);
	}

	if (FAILED(ret)) {

		if (g_has_pts)
		{
			g_has_pts = 0;
			DEBUG_INFO("[Drop PTS]: ts: %I64d freq: %d, tslength: %d flags: %d unused1: %d unused2: %d", nalu->pts.ts, nalu->pts.freq, nalu->pts.tslength, nalu->pts.flags, nalu->pts.unused1, nalu->pts.unused2);
		}

		if (IMGPAR structure == FRAME) {

			if(dec_picture)
			{
				if (dpb.used_size_on_view && ((dpb.used_size_on_view[0] == 0) ||dpb.fs_on_view[0][dpb.used_size_on_view[0] - 1]->frame != dec_picture)) {
					dec_picture->pic_store_idx = img->UnCompress_Idx;
					release_storable_picture ARGS2(dec_picture, 1);
					img->UnCompress_Idx = -1;
					dec_picture->pic_store_idx = -1;
				}
				dec_picture = NULL;
			}
			img->m_dec_picture_top = NULL;
			img->m_dec_picture_bottom = NULL;



		} else {

			if (img->m_dec_picture_top && (img->m_dec_picture_top->top_field_first)) {

				if(img->m_dec_picture_top && img->m_dec_picture_bottom)
				{
					if (dpb.used_size_on_view && ((dpb.used_size_on_view[0] == 0) ||dpb.fs_on_view[0][dpb.used_size_on_view[0] - 1]->bottom_field != img->m_dec_picture_bottom)) {

						if (dpb.used_size_on_view[0] && dpb.fs_on_view[0][dpb.used_size_on_view[0] - 1]->top_field && (dpb.fs_on_view[0][dpb.used_size_on_view[0] - 1]->top_field->pic_store_idx == img->UnCompress_Idx)){
							img->m_dec_picture_bottom->pic_store_idx = -1;
						} else {
							img->m_dec_picture_bottom->pic_store_idx = img->UnCompress_Idx;
						}

						//img->m_dec_picture_bottom->pic_store_idx = img->UnCompress_Idx;

						release_storable_picture ARGS2(img->m_dec_picture_bottom, 1);
						img->UnCompress_Idx = -1;
						img->m_dec_picture_bottom->pic_store_idx = -1;
					}

				} else {

					if (dpb.used_size_on_view && ((dpb.used_size_on_view[0] == 0) ||dpb.fs_on_view[0][dpb.used_size_on_view[0] - 1]->top_field != img->m_dec_picture_top)) {
						if (img->m_dec_picture_top) {
							img->m_dec_picture_top->pic_store_idx = img->UnCompress_Idx;
						}
						release_storable_picture ARGS2(img->m_dec_picture_top, 1);	
						img->UnCompress_Idx = -1;

					}

				}
				img->m_dec_picture_top = NULL;
				img->m_dec_picture_bottom = NULL;


			} else {

				if(img->m_dec_picture_top && img->m_dec_picture_bottom)
				{
					if (dpb.used_size_on_view && ((dpb.used_size_on_view[0] == 0) ||dpb.fs_on_view[0][dpb.used_size_on_view[0] - 1]->top_field != img->m_dec_picture_top)) {

						if (dpb.used_size_on_view[0] && dpb.fs_on_view[0][dpb.used_size_on_view[0] - 1]->top_field && (dpb.fs_on_view[0][dpb.used_size_on_view[0] - 1]->bottom_field->pic_store_idx == img->UnCompress_Idx)){
							img->m_dec_picture_top->pic_store_idx = -1;
						} else {
							img->m_dec_picture_top->pic_store_idx = img->UnCompress_Idx;
						}

						//img->m_dec_picture_top->pic_store_idx = img->UnCompress_Idx;

						release_storable_picture ARGS2(img->m_dec_picture_top, 1);
						img->UnCompress_Idx = -1;
						img->m_dec_picture_top->pic_store_idx = -1;
					}

				} else {

					if (dpb.used_size_on_view && ((dpb.used_size_on_view[0] == 0) ||dpb.fs_on_view[0][dpb.used_size_on_view[0] - 1]->bottom_field != img->m_dec_picture_bottom)) {
						if (img->m_dec_picture_bottom) {
							img->m_dec_picture_bottom->pic_store_idx = img->UnCompress_Idx;
						}
						release_storable_picture ARGS2(img->m_dec_picture_bottom, 1);
						img->UnCompress_Idx = -1;						
					}

				}
				img->m_dec_picture_top = NULL;
				img->m_dec_picture_bottom = NULL;


			}		


			dec_picture = NULL;			

		}	

		for (int i=0 ;i<1; i++)
		{
			img = img_array[i];
			if (i==0)
			{
				IMGPAR currentSlice = IMGPAR firstSlice;
				for (int j=0; j< IMGPAR slice_number; j++)
				{
					//active_pps = &PicParSet[IMGPAR currentSlice->pic_parameter_set_id];
					memcpy(&active_pps, &PicParSet[IMGPAR currentSlice->pic_parameter_set_id], sizeof

						(pic_parameter_set_rbsp_t));
					IMGPAR prevSlice = IMGPAR currentSlice;
					IMGPAR currentSlice = (Slice*) IMGPAR currentSlice->nextSlice;
					free_new_slice(IMGPAR prevSlice);
				}
			}
			IMGPAR firstSlice = IMGPAR currentSlice = IMGPAR prevSlice = NULL;
			IMGPAR current_mb_nr_r = IMGPAR current_mb_nr_d = -4712;   // impossible value for debugging, StW
			IMGPAR error_mb_nr = -4712;
			IMGPAR current_slice_nr = 0;
			IMGPAR slice_number = 0;
			img->m_active_pps[0].Valid = NULL;
			img->m_active_pps[1].Valid = NULL;
			dec_picture = NULL;
			img->m_dec_picture_top = NULL;
			img->m_dec_picture_bottom = NULL;

			if (IMGPAR structure != FRAME)
			{
				IMGPAR mb_decdata = IMGPAR mb_decdata_ori;
				IMGPAR cof_array = IMGPAR cof_array_ori;
			}
		}

		if (stream_global->m_bSeekIDR) {
			if(dpb.used_size_on_view[0])
			{
				for (unsigned int i=0; i <storable_picture_count; i++)				
					storable_picture_map[i]->used_for_first_field_reference = 0;


				if (img->structure  == FRAME) {

					if (dpb.fs_on_view[0][dpb.used_size_on_view[0]-1]->frame && dpb.fs_on_view[0][dpb.used_size_on_view[0]-1]->frame->pic_store_idx == img->UnCompress_Idx){
						img->UnCompress_Idx = -1;
					}

				} else {

					if ((dpb.fs_on_view[0][dpb.used_size_on_view[0]-1]->top_field && dpb.fs_on_view[0][dpb.used_size_on_view[0]-1]->top_field->pic_store_idx == img->UnCompress_Idx) ||
				        (dpb.fs_on_view[0][dpb.used_size_on_view[0]-1]->bottom_field && dpb.fs_on_view[0][dpb.used_size_on_view[0]-1]->bottom_field->pic_store_idx == img->UnCompress_Idx)) {
						img->UnCompress_Idx = -1;
					}
				}				

				flush_dpb ARGS1(0);
				update_ref_list ARGS1(0);
				update_ltref_list ARGS1(0);		
			}

			nalu_global_available = 0;
		}

		return ret;
	}

	return CREL_ERROR_H264_SYNCNOTFOUND;
}

CREL_RETURN CH264DXVA2_ATI::BA_build_picture_decode_buffer PARGS0()
{
	HRESULT	hr = S_OK;
	int i, j;
	StreamParameters *stream_global = IMGPAR stream_global;
	pic_parameter_set_rbsp_t *pps = &active_pps;
	seq_parameter_set_rbsp_t *sps = &active_sps;
	if(active_sps.Valid)
		sps =  (IMGPAR currentSlice->bIsBaseView) ? &SeqParSet[PicParSet[IMGPAR currentSlice->pic_parameter_set_id].seq_parameter_set_id] :
			&SeqParSubset[PicParSet[IMGPAR currentSlice->pic_parameter_set_id].seq_parameter_set_id];

	AssignQuantParam ARGS2(&active_pps, &active_sps);

	if(m_bResolutionChange && get_param_fcn != NULL)
	{
		HRESULT hr = S_OK;
#if 1
		HVDService::HVDDecodeConfig DecodeConfig;
		ZeroMemory(&DecodeConfig, sizeof(HVDService::HVDDecodeConfig));
		m_pIHVDService->GetHVDDecodeConfig(&DecodeConfig);
		if (DecodeConfig.dwWidth != IMGPAR width || DecodeConfig.dwHeight != IMGPAR height)
		{
			DecodeConfig.dwWidth = IMGPAR width;
			DecodeConfig.dwHeight = IMGPAR height;

			//Change display resolution and Get new IVICP
			(*get_param_fcn)(H264_PROPID_CB_CHANGE_RESOLUTION, H264_pvDataContext, (LPVOID*)&m_pIviCP, NULL, &DecodeConfig, sizeof(HVDService::HVDDecodeConfig));

			ResetDXVABuffers();
		}
#else
		hr = Send_Resolution(H264_pvDataContext,IMGPAR width,IMGPAR height,(dec_picture->dwXAspect==16 ? 3 : 2), (LPVOID*)&m_pIviCP);
		if(FAILED(hr))
			return -1;

		ResetDXVABuffers();
#endif

		m_bResolutionChange = FALSE;
	}

	hr = GetBuffer(DXVA2_PictureParametersBufferType, (LPVOID*)(&IMGPAR m_pnv1PictureDecode), 0);
	DEBUG_SHOW_HW_INFO("GetBuffer() PicPar, return value = %d, address = %d", hr, IMGPAR m_pnv1PictureDecode);
	hr = GetBuffer(DXVA2_InverseQuantizationMatrixBufferType, (LPVOID*)(&IMGPAR m_lpMBLK_Intra_Luma), 0);
	DEBUG_SHOW_HW_INFO("GetBuffer() IQM, return value = %d, address = %d", hr, IMGPAR m_lpMBLK_Intra_Luma);
	hr = GetBuffer(DXVA2_SliceControlBufferType, (LPVOID*)(&IMGPAR m_lpSLICE), 0);
	DEBUG_SHOW_HW_INFO("GetBuffer() SliceCtrl, return value = %d, address = %d", hr, IMGPAR m_lpSLICE);
	hr = GetBuffer(DXVA2_BitStreamDateBufferType, (LPVOID*)(&IMGPAR m_lpMV), 0);
	DEBUG_SHOW_HW_INFO("GetBuffer() BitstreamData, return value = %d, address = %d", hr, IMGPAR m_lpMV);
	m_lpBitstreamBuf = IMGPAR m_lpMV;
	m_lpSliceCtrlBuf = IMGPAR m_lpSLICE;

	if(IMGPAR UnCompress_Idx == -1)
	{
		while (1)
		{
			IMGPAR UnCompress_Idx = (int)m_pUncompBufQueue->GetItem();

			hr = BeginFrame(IMGPAR UnCompress_Idx, 1);
			if(checkDDError(hr))
				m_pUncompBufQueue->PutItem(IMGPAR UnCompress_Idx);
			else
			{
				EndFrame();
				break;
			}
		}
	}

	IMGPAR m_lFrame_Counter = m_nFrameCounter++;

	DXVA_BufferDescription *m_pDxvaBufferDescription = IMGPAR m_pDxvaBufferDescription;
	AMVABUFFERINFO         *m_pBufferInfo            = IMGPAR m_pBufferInfo;
	DXVA_PicParams_H264* m_patiPictureDecode = (DXVA_PicParams_H264*)IMGPAR m_pnv1PictureDecode;
	DXVA_Qmatrix_H264 *m_pQpMatrix = (DXVA_Qmatrix_H264*)IMGPAR m_lpMBLK_Intra_Luma;
	memset(m_patiPictureDecode, 0, sizeof(DXVA_PicParams_H264));

	m_patiPictureDecode->CurrPic.Index7Bits		= IMGPAR UnCompress_Idx;
	//m_patiPictureDecode->CurrPic.AssociatedFlag = (IMGPAR currentSlice->structure != 0);
	m_patiPictureDecode->CurrPic.AssociatedFlag = (IMGPAR field_pic_flag & (IMGPAR currentSlice->structure >> 1));
	m_patiPictureDecode->wFrameWidthInMbsMinus1		= sps->pic_width_in_mbs_minus1;
	m_patiPictureDecode->wFrameHeightInMbsMinus1		= IMGPAR FrameHeightInMbs-1;

	m_patiPictureDecode->num_ref_frames = sps->num_ref_frames;
#if 1
	m_patiPictureDecode->field_pic_flag = IMGPAR field_pic_flag;   //field_pic_flag
	m_patiPictureDecode->MbaffFrameFlag = IMGPAR currentSlice->MbaffFrameFlag;  //MbaffFrameFlag
	m_patiPictureDecode->residual_colour_transform_flag = 0;  //residual_colour_transform_flag
	m_patiPictureDecode->sp_for_switch_flag =  0;  //sp_for_switch_flag
	m_patiPictureDecode->chroma_format_idc = sps->chroma_format_idc;  //chroma_format_idc
	//m_patiPictureDecode->RefPicFlag = IMGPAR currentSlice->nal_reference_idc;  //RefPicFlag
	m_patiPictureDecode->RefPicFlag = dec_picture->used_for_reference;
	m_patiPictureDecode->constrained_intra_pred_flag = pps->constrained_intra_pred_flag; //constrained_intra_pred_flag
	m_patiPictureDecode->weighted_pred_flag = pps->weighted_pred_flag;  //weighted_pred_flag
	m_patiPictureDecode->weighted_bipred_idc = pps->weighted_bipred_idc;  //weighted_bipred_idc
	m_patiPictureDecode->MbsConsecutiveFlag = 1; //restricted mode profile is 0. //MbsConsecutiveFlag
	m_patiPictureDecode->frame_mbs_only_flag = sps->frame_mbs_only_flag;  //frame_mbs_only_flag
	m_patiPictureDecode->transform_8x8_mode_flag = pps->transform_8x8_mode_flag;  //transform_8x8_mode_flag
	m_patiPictureDecode->MinLumaBipredSize8x8Flag = 1; //set to 1 for Main,High,High 10,High 422,or High 444 of levels3.1 and higher.
	m_patiPictureDecode->IntraPicFlag = (IMGPAR currentSlice->picture_type == I_SLICE);  //IntraPicFlag
#else
	m_patiPictureDecode->wBitFields |= (((IMGPAR currentSlice->structure !=0) & 0x01)<<15);   //field_pic_flag
	m_patiPictureDecode->wBitFields |= ((IMGPAR currentSlice->MbaffFrameFlag & 0x01) << 14);  //MbaffFrameFlag
	m_patiPictureDecode->wBitFields |= (((sps->chroma_format_idc ==3) & 0x01)<<13);  //residual_colour_transform_flag
	//m_patiPictureDecode->wBitFields |=    //sp_for_switch_flag
	m_patiPictureDecode->wBitFields |= ((sps->chroma_format_idc & 0x03)<<10);  //chroma_format_idc
	m_patiPictureDecode->wBitFields |= ((IMGPAR currentSlice->nal_reference_idc & 0x01)<<9);  //RefPicFlag
	m_patiPictureDecode->wBitFields |= ((pps->constrained_intra_pred_flag & 0x01)<<8);  //constrained_intra_pred_flag
	m_patiPictureDecode->wBitFields |= ((pps->weighted_pred_flag & 0x01)<<7);  //weighted_pred_flag
	m_patiPictureDecode->wBitFields |= ((pps->weighted_bipred_idc & 0x03)<<5);  //weighted_bipred_idc
	m_patiPictureDecode->wBitFields |= ((sps->gaps_in_frame_num_value_allowed_flag & 0x01)<<4);  //MbsConsecutiveFlag
	m_patiPictureDecode->wBitFields |= ((sps->frame_mbs_only_flag & 0x01)<<3);  //frame_mbs_only_flag
	m_patiPictureDecode->wBitFields |= ((pps->transform_8x8_mode_flag & 0x01)<<2);  //transform_8x8_mode_flag
	m_patiPictureDecode->wBitFields |= ((IMGPAR currentSlice->picture_type == I_SLICE) & 0x01);  //IntraPicFlag
#endif
	m_patiPictureDecode->bit_depth_luma_minus8 = sps->bit_depth_luma_minus8;
	m_patiPictureDecode->bit_depth_chroma_minus8 = sps->bit_depth_chroma_minus8;
	m_patiPictureDecode->StatusReportFeedbackNumber = 0;
	//if(IMGPAR currentSlice->structure == FRAME)
	//{
	//	m_refinfo[IMGPAR UnCompress_Idx].top_poc = m_patiPictureDecode->CurrFieldOrderCnt[0] = IMGPAR toppoc;
	//	m_refinfo[IMGPAR UnCompress_Idx].bot_poc = m_patiPictureDecode->CurrFieldOrderCnt[1] = IMGPAR bottompoc;
	//}
	//else if(IMGPAR currentSlice->structure == TOP_FIELD)
	//	m_refinfo[IMGPAR UnCompress_Idx].top_poc = m_patiPictureDecode->CurrFieldOrderCnt[0] = IMGPAR toppoc;
	//else
	//	m_refinfo[IMGPAR UnCompress_Idx].bot_poc = m_patiPictureDecode->CurrFieldOrderCnt[1] = IMGPAR bottompoc;
	if(m_patiPictureDecode->field_pic_flag)
	{
		if(m_patiPictureDecode->CurrPic.AssociatedFlag)
		{
			m_RefInfo[IMGPAR UnCompress_Idx].top_poc = m_patiPictureDecode->CurrFieldOrderCnt[0] = 0;
			m_RefInfo[IMGPAR UnCompress_Idx].bot_poc = m_patiPictureDecode->CurrFieldOrderCnt[1] = IMGPAR bottompoc;
		}
		else
		{
			m_RefInfo[IMGPAR UnCompress_Idx].top_poc = m_patiPictureDecode->CurrFieldOrderCnt[0] = IMGPAR toppoc;
			m_RefInfo[IMGPAR UnCompress_Idx].bot_poc = m_patiPictureDecode->CurrFieldOrderCnt[1] = 0;
		}
	}
	else
	{
		m_RefInfo[IMGPAR UnCompress_Idx].top_poc = m_patiPictureDecode->CurrFieldOrderCnt[0] = IMGPAR toppoc;
		m_RefInfo[IMGPAR UnCompress_Idx].bot_poc = m_patiPictureDecode->CurrFieldOrderCnt[1] = IMGPAR bottompoc;
	}

	DEBUG_SHOW_HW_INFO("poc:%d", m_patiPictureDecode->CurrFieldOrderCnt[0]);

	m_patiPictureDecode->pic_init_qs_minus26 = pps->pic_init_qs_minus26;
	m_patiPictureDecode->chroma_qp_index_offset = pps->chroma_qp_index_offset;
	m_patiPictureDecode->second_chroma_qp_index_offset = pps->second_chroma_qp_index_offset;
	m_patiPictureDecode->ContinuationFlag = 1;
	m_patiPictureDecode->pic_init_qp_minus26 = pps->pic_init_qp_minus26;
	m_patiPictureDecode->num_ref_idx_l0_active_minus1 = pps->num_ref_idx_l0_active_minus1;
	m_patiPictureDecode->num_ref_idx_l1_active_minus1 = pps->num_ref_idx_l1_active_minus1;
	m_patiPictureDecode->UsedForReferenceFlags = 0;
	m_patiPictureDecode->NonExistingFrameFlags = 0;
	m_RefInfo[IMGPAR UnCompress_Idx].frame_num = m_patiPictureDecode->frame_num = IMGPAR currentSlice->frame_num;
	m_patiPictureDecode->log2_max_frame_num_minus4 = sps->log2_max_frame_num_minus4;
	m_patiPictureDecode->pic_order_cnt_type = sps->pic_order_cnt_type;
	m_patiPictureDecode->log2_max_pic_order_cnt_lsb_minus4 = sps->log2_max_pic_order_cnt_lsb_minus4;
	m_patiPictureDecode->delta_pic_order_always_zero_flag = sps->delta_pic_order_always_zero_flag;
	m_patiPictureDecode->direct_8x8_inference_flag = sps->direct_8x8_inference_flag;
	m_patiPictureDecode->entropy_coding_mode_flag = pps->entropy_coding_mode_flag;
	m_patiPictureDecode->pic_order_present_flag = pps->pic_order_present_flag;
	m_patiPictureDecode->num_slice_groups_minus1 = pps->num_slice_groups_minus1;
	m_patiPictureDecode->slice_group_map_type = pps->slice_group_map_type;
	m_patiPictureDecode->deblocking_filter_control_present_flag = pps->deblocking_filter_control_present_flag;
	m_patiPictureDecode->redundant_pic_cnt_present_flag = pps->redundant_pic_cnt_present_flag;
	m_patiPictureDecode->slice_group_change_rate_minus1 = pps->slice_group_change_rate_minus1;
	//m_patiPictureDecode->SliceGroupMap =;

	memset(m_patiPictureDecode->RefFrameList,255,16);
	StorablePicture *RefFrame;

	for(unsigned int j=0, i=0; i<dpb.used_size_on_view[0]; i++)
	{
		if(dpb.fs_on_view[0][i]->is_reference)
		{
			if(dpb.fs_on_view[0][i]->is_reference == 3)
			{
				RefFrame = dpb.fs_on_view[0][i]->frame;
				m_patiPictureDecode->FieldOrderCntList[j][0] = RefFrame->top_poc;
				m_patiPictureDecode->FieldOrderCntList[j][1] = RefFrame->bottom_poc;
			}
			else
			{
				if(dpb.fs_on_view[0][i]->is_reference == 1)
				{
					RefFrame = dpb.fs_on_view[0][i]->top_field;
					m_patiPictureDecode->FieldOrderCntList[j][0] = RefFrame->top_poc;
				}
				else
				{
					RefFrame = dpb.fs_on_view[0][i]->bottom_field;
					m_patiPictureDecode->FieldOrderCntList[j][1] = RefFrame->bottom_poc;
				}
			}
			m_patiPictureDecode->RefFrameList[j].Index7Bits	= RefFrame->pic_store_idx;	//*(byte*)&m_pSurfaceinfo[listX[0][i]->unique_id].SInfo[0];
			m_patiPictureDecode->RefFrameList[j].AssociatedFlag = 0;		
			m_patiPictureDecode->FrameNumList[j] = RefFrame->frame_num;
			m_patiPictureDecode->UsedForReferenceFlags |= (dpb.fs_on_view[0][i]->is_reference << (j*2));

			j++;
		}
	}

	m_patiPictureDecode->Reserved8BitsA = 0;
	amddecext_h264setlevel(sps->level_idc,m_patiPictureDecode);
	if (sps->profile_idc == 100)
		amddecext_h264setprofile(HIGH,m_patiPictureDecode);		
	else if(sps->profile_idc == 77)
		amddecext_h264setprofile(MAIN,m_patiPictureDecode);		
	else if(sps->profile_idc == 66)
		amddecext_h264setprofile(BASELINE,m_patiPictureDecode);		
	else
		amddecext_h264setprofile(_TOO_BIG_,m_patiPictureDecode);		

	m_patiPictureDecode->Reserved8BitsB = 0;
	amddecext_h264setextsupport(1,m_patiPictureDecode);
	if (sps->gaps_in_frame_num_value_allowed_flag)
		amddecext_h264setgap(1,m_patiPictureDecode);
	if (!g_bReceivedFirst)
		amddecext_h264setseek(1,m_patiPictureDecode);

	//copy QP matrix
	for(i = 0; i<6; i++)
		memcpy(&m_pQpMatrix->bScalingLists4x4[i][0], qmatrix[i], 16);
	for(i = 6; i<8; i++)
		memcpy(&m_pQpMatrix->bScalingLists8x8[i-6][0], qmatrix[i], 64);

	SetSurfaceInfo ARGS3(dec_picture->unique_id, IMGPAR currentSlice->structure ==2?1:0, 1);

	return 0;
}

CREL_RETURN CH264DXVA2_ATI::BA_ExecuteBuffers PARGS0()
{
	DEBUG_SHOW_HW_INFO("[CH264DXVA2_ATI] BA_ExecuteBuffers()");
	HRESULT	hr = S_OK;

	hr = BeginFrame(IMGPAR UnCompress_Idx, 0);
	if(SUCCEEDED(hr))
	{
		if(!stream_global->m_iStop_Decode)
		{
			if(IMGPAR m_iIntraMCBufUsage != 0)
			{			
				if(m_pIviCP != NULL)
				{
					if(m_pIviCP->IsScramblingRequired(MAP_CP_PIC_TYPE(IMGPAR firstSlice->picture_type)))	
					{
						m_pIviCP->EnableScrambling();
						m_pIviCP->ScrambleData(IMGPAR m_lpMV, IMGPAR m_lpMV, IMGPAR m_iIntraMCBufUsage);
					}
					else
					{
						m_pIviCP->DisableScrambling();					
					}
					
					if(m_pIviCP->GetObjID()==E_CP_ID_WIN7)
					{
						BeginFrame(IMGPAR UnCompress_Idx, 0);
						EndFrame();
					}
				}

				DXVA_BufferDescription *m_pDxvaBufferDescription = IMGPAR m_pDxvaBufferDescription;
				AMVABUFFERINFO         *m_pBufferInfo            = IMGPAR m_pBufferInfo;

				memset(&m_pDxvaBufferDescription[0], 0, 4*sizeof(DXVA_BufferDescription));
				m_pDxvaBufferDescription[0].dwTypeIndex		= m_pBufferInfo[0].dwTypeIndex	= DXVA2_PictureParametersBufferType;
				m_pDxvaBufferDescription[0].dwBufferIndex = m_pBufferInfo[0].dwBufferIndex = 0;
				m_pDxvaBufferDescription[0].dwDataSize		= m_pBufferInfo[0].dwDataSize	= sizeof(DXVA_PicParams_H264);
				m_pDxvaBufferDescription[0].dwNumMBsInBuffer	= IMGPAR slice_number;

				m_pDxvaBufferDescription[1].dwTypeIndex		= m_pBufferInfo[1].dwTypeIndex	= DXVA2_InverseQuantizationMatrixBufferType;
				m_pDxvaBufferDescription[1].dwBufferIndex = m_pBufferInfo[1].dwBufferIndex = 0;
				m_pDxvaBufferDescription[1].dwDataSize		= m_pBufferInfo[1].dwDataSize	= 224;
				m_pDxvaBufferDescription[1].dwNumMBsInBuffer	= IMGPAR slice_number;

				m_pDxvaBufferDescription[2].dwTypeIndex			= m_pBufferInfo[2].dwTypeIndex	= DXVA2_SliceControlBufferType;
				m_pDxvaBufferDescription[2].dwBufferIndex		= m_pBufferInfo[2].dwBufferIndex = 0;
				m_pDxvaBufferDescription[2].dwDataSize			= m_pBufferInfo[2].dwDataSize	= IMGPAR m_slice_number_in_field * 10;
				m_pDxvaBufferDescription[2].dwNumMBsInBuffer	= IMGPAR slice_number;

				m_pDxvaBufferDescription[3].dwTypeIndex			= m_pBufferInfo[3].dwTypeIndex	= DXVA2_BitStreamDateBufferType;
				m_pDxvaBufferDescription[3].dwBufferIndex		= m_pBufferInfo[3].dwBufferIndex = 0;
				m_pDxvaBufferDescription[3].dwDataSize			= m_pBufferInfo[3].dwDataSize	= IMGPAR m_iIntraMCBufUsage;
				m_pDxvaBufferDescription[3].dwNumMBsInBuffer	= IMGPAR slice_number;

				for (UINT i=0; i<4; i++)
				{
					if (FAILED(ReleaseBuffer(m_pBufferInfo[i].dwTypeIndex)))
						break;
				}

				hr |= Execute(0, &m_pDxvaBufferDescription[0], 4*sizeof(DXVA_BufferDescription), NULL, 0, 4, &m_pBufferInfo[0]);
				hr |= EndFrame();
				DEBUG_SHOW_HW_INFO("executed frame:%d", IMGPAR m_lFrame_Counter);
			}
		}
	}

	return SUCCEEDED(hr)?CREL_OK:CREL_ERROR_H264_UNDEFINED;
}

int CH264DXVA2_ATI::decode_one_macroblock_Intra PARGS0()
{
	int cbp_blk = currMB_s_d->cbp_blk;
	int i=0, j=0, k=0, i4=0, j4=0;
	int ioff, joff;
	int b8;
	int width;	
	WORD temp1,temp2;
	__m64 mm0;

	short *pcof = (short*)IMGPAR cof_d;
	short *pos_plane;
	int slice_start_mb_nr = IMGPAR MbaffFrameFlag ? (IMGPAR currentSlice->start_mb_nr<<1) : IMGPAR currentSlice->start_mb_nr;

	if(IMGPAR current_mb_nr_d == slice_start_mb_nr)
	{
		if (stream_global->m_is_MTMS)
		{
			IMGPAR m_pnv1PictureDecode = m_lpPictureParamBuf;
			IMGPAR m_lpMBLK_Intra_Luma = m_lpMacroblockCtrlBuf;
			IMGPAR m_lpRESD_Intra_Luma = m_lpResidualDiffBuf;
			IMGPAR m_lpSLICE = m_lpSliceCtrlBuf;
			IMGPAR m_lpMV = m_lpMotionVectorBuf;

			IMGPAR m_PicHeight = IMGPAR PicHeightInMbs*16;
			IMGPAR m_MVBufSwitch = (IMGPAR m_PicHeight>>2)*(m_nMVBufStride>>2);
			IMGPAR m_SBBufOffset = (IMGPAR m_PicHeight>>4)*(IMGPAR width>>2);
			m_nMBBufStride = (IMGPAR width>>1);
			m_nReBufStride = (IMGPAR width<<1);	
		}

		build_slice_parameter_buffer ARGS0();
		IMGPAR m_lmbCount_Inter++;
	}

	H264_MC_CTRLMAP* MB_MC_Ctrl = (H264_MC_CTRLMAP*)(IMGPAR m_lpMBLK_Intra_Luma+IMGPAR mb_y_d*(IMGPAR width>>2)+(IMGPAR mb_x_d<<2));
	H264_SB_CTRLMAP* MB_SB_Ctrl = (H264_SB_CTRLMAP*)(IMGPAR m_lpMBLK_Intra_Luma+IMGPAR m_SBBufOffset+(IMGPAR mb_y_d<<2)*m_nMBBufStride+(IMGPAR mb_x_d<<3));	
	H264_RD* MB_Residual = (H264_RD*)IMGPAR m_lpRESD_Intra_Luma+(IMGPAR current_mb_nr_d<<8);

	*MB_MC_Ctrl  = H264_INTRA_PREDICTION;
	*MB_MC_Ctrl |= ((currMB_d->mb_field)&&(dec_picture->MbaffFrameFlag))? H264_FIELD_MB:0;
	*MB_MC_Ctrl |= (IMGPAR current_slice_nr << 8);

	width = IMGPAR width;

	if (currMB_d->mb_type == I16MB)
	{
#if 1
		temp1 = (I_Pred_luma_16x16 ARGS1(currMB_d->i16mode) << 2);
		temp1 |= (I_Pred_chroma ARGS1(0) << 12);
		mm0 = _mm_set1_pi16(temp1);
		for(j=0; j<4 ; j++)
		{
			*((__m64*)MB_SB_Ctrl) = mm0;
			MB_SB_Ctrl += (m_nMBBufStride>>1);
		}
#else
		for(j=0; j<4 ; j++)
		{
			for(i=0; i<4; i++)
			{	
				MB_SB_Ctrl[i]  = (I_Pred_luma_16x16 ARGS1(currMB_d->i16mode) << 2);
				MB_SB_Ctrl[i] |= (I_Pred_chroma ARGS1(0) << 12);
			}
			MB_SB_Ctrl += (m_nMBBufStride>>1);
		}
#endif
		for(i=0; i<16; i+=2)
		{	
			pos_plane = (short*)(MB_Residual + ((i&12)*16) + ((i&3)<<2));
			//if( cbp_blk & 1 )
			iDCT_4x4_2_ATI(pos_plane, pcof, 16);
			/*else
			{
			offset = idx2[i];
			memset(&Residual[offset],0,8);
			memset(&Residual[offset+16],0,8);
			memset(&Residual[offset+32],0,8);
			memset(&Residual[offset+48],0,8);
			}
			cbp_blk >>= 1;*/
			pcof += 32;
		}	
	}
	else if(currMB_d->mb_type == I8MB)
	{
#if 1
		for(j=0; j<2 ; j++)
		{
			b8 = (j<<1);
			temp1  = (I_Pred_luma_8x8 ARGS1(b8) << 2);
			temp1 |= IMGPAR m_UpLeft;
			temp1 |= (IMGPAR m_UpRight << 1);
			temp1 |= (I_Pred_chroma ARGS1(0) << 12);
			temp2  = (I_Pred_luma_8x8 ARGS1(b8+1) << 2);
			temp2 |= IMGPAR m_UpLeft;
			temp2 |= (IMGPAR m_UpRight << 1);
			temp2 |= (I_Pred_chroma ARGS1(0) << 12);
			mm0 = _mm_set_pi16(temp2, temp2, temp1, temp1);
			*((__m64*)MB_SB_Ctrl) = mm0;
			*((__m64*)(MB_SB_Ctrl+(m_nMBBufStride>>1))) = mm0;
			MB_SB_Ctrl += ((m_nMBBufStride>>1)<<1);  
		}
#else
		for(j=0; j<4 ; j++)
		{
			for(i=0; i<4; i++)
			{
				b8 = ((j>>1)<<1)+(i>>1);
				MB_SB_Ctrl[i]  = (I_Pred_luma_8x8 ARGS1(b8) << 2);
				MB_SB_Ctrl[i] |= IMGPAR m_UpLeft;
				MB_SB_Ctrl[i] |= (IMGPAR m_UpRight << 1);
				MB_SB_Ctrl[i] |= (I_Pred_chroma ARGS1(0) << 12);
			}
			MB_SB_Ctrl += (m_MbBufStride>>1);
		}
#endif		
		for(b8=0; b8<4; b8++)
		{
			ioff = (b8&1)<<3;	//0 8 0 8
			joff = (b8>>1)<<3;	//0 0 8 8

			pos_plane = (short*)(MB_Residual + ((b8>>1)*16<<3) + ((b8&1)<<3));

			if (currMB_d->cbp&(1<<b8))
				iDCT_8x8_fcn(pos_plane, IMGPAR cof_d + (b8<<6), 16);
			else
			{				
				memset(pos_plane, 0, 16);
				memset(pos_plane+16, 0, 16);
				memset(pos_plane+(16<<1), 0, 16);
				memset(pos_plane+(16<<1)+16, 0, 16);
				memset(pos_plane+(16<<2), 0, 16);
				memset(pos_plane+(16<<2)+16, 0, 16);
				memset(pos_plane+(16<<2)+(16<<1), 0, 16);
				memset(pos_plane+(16<<3)-16, 0, 16);
			}
		}		
	}
	else if(currMB_d->mb_type == I4MB)
	{	
		for(j=0; j<4 ; j++)
		{
			for(i=0; i<4; i++)
			{
				MB_SB_Ctrl[i]  = (I_Pred_luma_4x4 ARGS4((i<<2), (j<<2), (IMGPAR block_x_d+i), (IMGPAR block_y_d+j)) << 2);
				MB_SB_Ctrl[i] |= (IMGPAR m_UpRight << 1);
				MB_SB_Ctrl[i] |= (I_Pred_chroma ARGS1(0) << 12);
			}
			MB_SB_Ctrl += (m_nMBBufStride>>1);
		}
		for(i=0; i<16; i+=2)
		{
			pos_plane = (short*)(MB_Residual + ((i&12)*16) + ((i&3)<<2));
			//if( cbp_blk & 1 )
			iDCT_4x4_2_ATI(pos_plane, pcof, 16);
			/*else
			{
			offset = idx2[i];
			memset(&Residual[offset],0,8);
			memset(&Residual[offset+16],0,8);
			memset(&Residual[offset+32],0,8);
			memset(&Residual[offset+48],0,8);
			}
			cbp_blk >>= 1;*/
			pcof += 32;
		}		
	}
	/*
	else if(currMB_d->mb_type == IPCM)
	{
	for(j=0; j<4 ; j++)
	{
	for(i=0; i<4; i++)
	{			
	MB_SB_Ctrl[i]  = (LUMA_DC_PRED_16_IPCM << 2);
	MB_SB_Ctrl[i] |= (CHROMA_8_IPCM << 12);
	}
	MB_SB_Ctrl += (m_MbBufStride>>1);
	}
	for(i=0; i<16; i++)
	{	
	pcof += 16;
	}
	for(i = 0; i < 16; i++)
	memcpy(MB_Residual+i*IMGPAR width, &Residual[i<<4], 32);
	}
	*/	 
#ifdef __SUPPORT_YUV400__
	if (dec_picture->chroma_format_idc != YUV400)
	{
#endif
		MB_Residual = (H264_RD*)IMGPAR m_lpRESD_Intra_Luma+(IMGPAR m_PicHeight*IMGPAR width)+(IMGPAR current_mb_nr_d<<7);
		if(currMB_d->mb_type != IPCM)
		{
#if   !defined(H264_ENABLE_INTRINSICS)
			short tmp4[4][4];
			short* Residual_UV;
			for(int uv=0;uv<2;uv++)
			{			
				for(b8=0;b8<4;b8++)
				{				
					ioff = (b8&1)<<2;	//0 4 0 4
					joff = (b8&2)<<1;	//0 0 4 4
					if( currMB_s_d->cbp_blk & (1<<cbp_blk_chroma[uv][b8]))
						iDCT_4x4_fcn(&tmp4[0][0], (IMGPAR cof_d + ((4+uv<<6)+(b8<<4))),4);
					else
						memset(tmp4, 0, sizeof(short)*16);

					for(j = 0; j < 4; j++)
						for(i = 0; i < 4; i++)
							Residual_UV[(joff+j)*16+(ioff+i)*2+uv] = tmp4[j][i];			
				}
			}
#else
			for(b8=0;b8<4;b8++)
			{
				pos_plane = (short*)( MB_Residual + ((b8>>1)*16<<2) + ((b8&1)<<3) ) ;

				ioff = (b8&1)<<3;	//0 8 0  8
				joff = (b8&2)<<5;	//0 0 64 64

				iDCT_4x4_UV_fcn(pos_plane
					, (IMGPAR cof_d + ((4<<6)+(b8<<4)))
					,( currMB_s_d->cbp_blk & (1<<cbp_blk_chroma[0][b8])) // residual U
					,( currMB_s_d->cbp_blk & (1<<cbp_blk_chroma[1][b8])) // residual V
					, 16);
			}
#endif
		}	

#ifdef __SUPPORT_YUV400__
	}
#endif

	return 0;
}

int CH264DXVA2_ATI::decode_one_macroblock_Inter PARGS0()
{
	int fw_refframe=-1, bw_refframe=-1;
	int mb_nr = IMGPAR current_mb_nr_d;
	int list_offset;
	int vec_x_base, vec_y_base;
	int curr_mb_field = ((IMGPAR MbaffFrameFlag)&&(currMB_s_d->mb_field));
	int slice_start_mb_nr = IMGPAR MbaffFrameFlag ? (IMGPAR currentSlice->start_mb_nr<<1) : IMGPAR currentSlice->start_mb_nr;

	if(IMGPAR current_mb_nr_d == slice_start_mb_nr)
	{
		if (stream_global->m_is_MTMS)
		{
			IMGPAR m_pnv1PictureDecode = m_lpPictureParamBuf;
			IMGPAR m_lpMBLK_Intra_Luma = m_lpMacroblockCtrlBuf;
			IMGPAR m_lpRESD_Intra_Luma = m_lpResidualDiffBuf;
			IMGPAR m_lpSLICE = m_lpSliceCtrlBuf;
			IMGPAR m_lpMV = m_lpMotionVectorBuf;

			IMGPAR m_PicHeight = IMGPAR PicHeightInMbs*16;
			IMGPAR m_MVBufSwitch = (IMGPAR m_PicHeight>>2)*(m_nMVBufStride>>2);
			IMGPAR m_SBBufOffset = (IMGPAR m_PicHeight>>4)*(IMGPAR width>>2);
			m_nMBBufStride = (IMGPAR width>>1);
			m_nReBufStride = (IMGPAR width<<1);
		}

		build_slice_parameter_buffer ARGS0();
		IMGPAR m_lmbCount_Inter++;
	}


	// Base position for 0-motion vectors
	vec_x_base = IMGPAR pix_x_d;
	vec_y_base = IMGPAR pix_y_d;
	// find out the correct list offsets
	if (curr_mb_field)
	{
		vec_y_base >>= 1;
		if(mb_nr & 1)
		{
			list_offset = 4; // top field mb		  
			vec_y_base -= 8;
		}
		else
		{
			list_offset = 2; // bottom field mb
		}
		clip_max_y    = (dec_picture->size_y>>1)-1;
		clip_max_y_cr = (dec_picture->size_y_cr>>1)-1;
	}
	else
	{
		list_offset = 0;  // no mb aff or frame mb
		clip_max_y    = dec_picture->size_y-1;
		clip_max_y_cr = dec_picture->size_y_cr-1;
	}
	clip_max_x    = dec_picture->size_x+1;
	clip_max_x_cr = dec_picture->size_x_cr-1;	

	calc_chroma_vector_adjustment ARGS2(list_offset, curr_mb_field);

	// luma decoding **************************************************  
	if(!currMB_d->mb_type)
	{
		if(IMGPAR type==B_SLICE)
			build_macroblock_buffer_Inter_8x8 ARGS1(list_offset);
		else
			build_macroblock_buffer_Inter_16x16 ARGS1(list_offset);
	}
	else if(currMB_d->mb_type==PB_16x16)  
		build_macroblock_buffer_Inter_16x16 ARGS1(list_offset);
	else if(currMB_d->mb_type==PB_16x8) 
		build_macroblock_buffer_Inter_16x8 ARGS1(list_offset);
	else if(currMB_d->mb_type==PB_8x16) 
		build_macroblock_buffer_Inter_8x16 ARGS1(list_offset);
	else  
		build_macroblock_buffer_Inter_8x8 ARGS1(list_offset);

	build_residual_buffer_Inter ARGS0();

	return 0;
}

int CH264DXVA2_ATI::build_picture_decode_buffer PARGS0()
{
	int i, j;
	if(IMGPAR structure == FRAME || pic_combine_status)
	{
		dec_picture->pic_store_idx = IMGPAR UnCompress_Idx = (int)m_pUncompBufQueue->GetItem();
#if defined(_USE_QUEUE_FOR_DXVA2_)
		WaitForSingleObject(stream_global->hReadyForRender[IMGPAR UnCompress_Idx], INFINITE);
		ResetEvent(stream_global->hReadyForRender[IMGPAR UnCompress_Idx]);
#endif
	}

	DXVA_PictureParameters_H264* m_patiPictureDecode = (DXVA_PictureParameters_H264*)IMGPAR m_pnv1PictureDecode;
	memset(m_patiPictureDecode, 0, sizeof(DXVA_PictureParameters_H264));

	m_patiPictureDecode->wDecodedPictureIndex		= IMGPAR UnCompress_Idx;
	m_patiPictureDecode->wPicWidthInMBminus1		= (int)active_sps.pic_width_in_mbs_minus1;
	m_patiPictureDecode->wPicHeightInMBminus1		= IMGPAR PicHeightInMbs-1;
	m_patiPictureDecode->bBPPminus1					= 7;
	m_patiPictureDecode->bChromaFormat				= 1;
	m_patiPictureDecode->bMV_RPS					= 1;
	m_patiPictureDecode->bWeightedPrediction		= (IMGPAR apply_weights ? 1:0);
	m_patiPictureDecode->bDisableILD				= 1;

	if(IMGPAR currentSlice->picture_type == P_SLICE)
	{
		m_patiPictureDecode->bH264PicType			= H264PIC_PI;
		m_patiPictureDecode->wNumberOfReference		= listXsize[0];
		for(i = 0; i < listXsize[0]; i++)
			m_patiPictureDecode->bReference[i]		= *(byte*)&m_pSurfaceInfo[listX[0][i]->unique_id].SInfo[0];
	}
	else if (IMGPAR currentSlice->picture_type == B_SLICE)
	{
		m_patiPictureDecode->bH264PicType			= H264PIC_BI;
		m_patiPictureDecode->wNumberOfReference		= listXsize[0];
		for(i = 0; i < listXsize[0]; i++)
		{
			m_patiPictureDecode->bReference[i] = *(byte*)&m_pSurfaceInfo[listX[0][i]->unique_id].SInfo[0];
		}
		for(j = 0; j < listXsize[1]; j++)
		{
			for(i = 0; i < listXsize[0]; i++)
			{
				if(listX[0][i]->unique_id == listX[1][j]->unique_id)
					break;
			}
			if(i == listXsize[0])	//not found matching index
			{
				m_patiPictureDecode->bReference[m_patiPictureDecode->wNumberOfReference] = *(byte*)&m_pSurfaceInfo[listX[1][j]->unique_id].SInfo[0];
				m_patiPictureDecode->wNumberOfReference++;
			}
		}
	}
	else
	{
		m_patiPictureDecode->bH264PicType			= H264PIC_INTRA;
		m_patiPictureDecode->wNumberOfReference		= 0;
	}

	switch(dec_picture->structure)
	{
	case FRAME:
		m_patiPictureDecode->bPicStructure	= 0x00;
		break;
	case TOP_FIELD:
		m_patiPictureDecode->bPicStructure	= 0x01; 
		break;
	case BOTTOM_FIELD:
		m_patiPictureDecode->bPicStructure	= 0x02;
		break;
	}
	if(IMGPAR MbaffFrameFlag)
		m_patiPictureDecode->bPicStructure	= 0x03;

	//backup surface infomation
	SetSurfaceInfo ARGS3(dec_picture->unique_id, m_patiPictureDecode->bPicStructure==2?1:0, 1);

	return 0;
}

int CH264DXVA2_ATI::build_slice_parameter_buffer PARGS0()
{
	int i, j, offset;
	HRESULT hr = S_FALSE;
	LPDXVA_SliceParameter_H264 pSlice = (LPDXVA_SliceParameter_H264)IMGPAR m_lpSLICE;
	memset(pSlice,0,sizeof(LPDXVA_SliceParameter_H264));

	if(IMGPAR currentSlice->picture_type == P_SLICE)
	{
		if(IMGPAR MbaffFrameFlag)
		{
			//Top L0 map to list index
			for(j = 0; j < listXsize[2]; j++)
				for(i = 0; i < listXsize[0]; i++)
					if(m_pSurfaceInfo[listX[0][i]->unique_id].SInfo[0] == m_pSurfaceInfo[listX[2][j]->unique_id].SInfo[0])
					{
						IMGPAR m_TopL0Map[j] = i;
						break;
					}
					//Bottom L0 map to list index
					for(j = 0; j < listXsize[4]; j++)
						for(i = 0; i < listXsize[0]; i++)
							if(m_pSurfaceInfo[listX[0][i]->unique_id].SInfo[0] == m_pSurfaceInfo[listX[4][j]->unique_id].SInfo[0])
							{
								IMGPAR m_BotL0Map[j] = i;
								break;
							}	
		}
	}
	else if (IMGPAR currentSlice->picture_type == B_SLICE)
	{
		int index = listXsize[0];
		for(j = 0; j < listXsize[1]; j++)
		{
			for(i = 0; i < listXsize[0]; i++)
			{
				if(listX[0][i]->unique_id == listX[1][j]->unique_id)
				{
					IMGPAR m_L0L1Map[j] = i;
					break;
				}
			}
			if(i == listXsize[0])	//not found matching index
				IMGPAR m_L0L1Map[j] = index++;
		}
		if(IMGPAR MbaffFrameFlag)
		{
			//Top L0 map to list index
			for(j = 0; j < listXsize[2]; j++)
				for(i = 0; i < listXsize[0]; i++)
					if(m_pSurfaceInfo[listX[0][i]->unique_id].SInfo[0] == m_pSurfaceInfo[listX[2][j]->unique_id].SInfo[0])
					{
						IMGPAR m_TopL0Map[j] = i;
						break;
					}
					//Top L1 map to list index
					for(j = 0; j < listXsize[3]; j++)
						for(i = 0; i < listXsize[1]; i++)
							if(m_pSurfaceInfo[listX[1][i]->unique_id].SInfo[0] == m_pSurfaceInfo[listX[3][j]->unique_id].SInfo[0])
							{
								IMGPAR m_TopL1Map[j] = IMGPAR m_L0L1Map[i];
								break;
							}
							//Bottom L0 map to list index
							for(j = 0; j < listXsize[4]; j++)
								for(i = 0; i < listXsize[0]; i++)
									if(m_pSurfaceInfo[listX[0][i]->unique_id].SInfo[0] == m_pSurfaceInfo[listX[4][j]->unique_id].SInfo[0])
									{
										IMGPAR m_BotL0Map[j] = i;
										break;
									}
									//Bottom L1 map to list index
									for(j = 0; j < listXsize[5]; j++)
										for(i = 0; i < listXsize[1]; i++)
											if(m_pSurfaceInfo[listX[1][i]->unique_id].SInfo[0] == m_pSurfaceInfo[listX[5][j]->unique_id].SInfo[0])
											{
												IMGPAR m_BotL1Map[j] = IMGPAR m_L0L1Map[i];
												break;
											}
		}
	}

	if(stream_global->m_is_MTMS)
	{
		pSlice->nNumberOfSlices = __max(pSlice->nNumberOfSlices, (UINT)IMGPAR current_slice_nr+1);
		if(IMGPAR apply_weights)
		{
			j = IMGPAR current_slice_nr;
			if(IMGPAR luma_log2_weight_denom)
				pSlice->SliceCtrlMap1[j].mWeightDenomLuma = (1<<IMGPAR luma_log2_weight_denom);
			else
				pSlice->SliceCtrlMap1[j].mWeightDenomLuma = 0;

			if(IMGPAR chroma_log2_weight_denom)
				pSlice->SliceCtrlMap1[j].mWeightDenomChroma = (1<<IMGPAR chroma_log2_weight_denom);
			else
				pSlice->SliceCtrlMap1[j].mWeightDenomChroma = 0;

			for(i = 0; i < listXsize[0]; i++)
			{
				offset = m_pSurfaceInfo[listX[0][i]->unique_id].SInfo[1];	//Bottom filed
				pSlice->SliceCtrlMap2[j].WOffset_L0[i+offset*16].mWeightOffsetY = *((IMGPAR wp_offset+(i+0)*3+0));
				pSlice->SliceCtrlMap2[j].WOffset_L0[i+offset*16].mWeightOffsetU = *((IMGPAR wp_offset+(i+0)*3+1));
				pSlice->SliceCtrlMap2[j].WOffset_L0[i+offset*16].mWeightOffsetV = *((IMGPAR wp_offset+(i+0)*3+2));
				pSlice->ExpWMap[j].WPacket_L0[i+offset*16].mWeightY = *((IMGPAR wp_weight+(i+0)*3+0));
				pSlice->ExpWMap[j].WPacket_L0[i+offset*16].mWeightU = *((IMGPAR wp_weight+(i+0)*3+1));
				pSlice->ExpWMap[j].WPacket_L0[i+offset*16].mWeightV = *((IMGPAR wp_weight+(i+0)*3+2));	
			}
			if((IMGPAR currentSlice->picture_type == B_SLICE) && active_pps.weighted_bipred_idc > 0)
			{
				for(i = 0; i < listXsize[1]; i++)
				{
					offset = m_pSurfaceInfo[listX[1][i]->unique_id].SInfo[1];	//Bottom filed
					pSlice->SliceCtrlMap2[j].WOffset_L1[IMGPAR m_L0L1Map[i]+offset*16].mWeightOffsetY = *((IMGPAR wp_offset+(i+MAX_REFERENCE_PICTURES)*3+0));
					pSlice->SliceCtrlMap2[j].WOffset_L1[IMGPAR m_L0L1Map[i]+offset*16].mWeightOffsetU = *((IMGPAR wp_offset+(i+MAX_REFERENCE_PICTURES)*3+1));
					pSlice->SliceCtrlMap2[j].WOffset_L1[IMGPAR m_L0L1Map[i]+offset*16].mWeightOffsetV = *((IMGPAR wp_offset+(i+MAX_REFERENCE_PICTURES)*3+2));
					pSlice->ExpWMap[j].WPacket_L1[IMGPAR m_L0L1Map[i]+offset*16].mWeightY = *((IMGPAR wp_weight+(i+MAX_REFERENCE_PICTURES)*3+0));
					pSlice->ExpWMap[j].WPacket_L1[IMGPAR m_L0L1Map[i]+offset*16].mWeightU = *((IMGPAR wp_weight+(i+MAX_REFERENCE_PICTURES)*3+1));
					pSlice->ExpWMap[j].WPacket_L1[IMGPAR m_L0L1Map[i]+offset*16].mWeightV = *((IMGPAR wp_weight+(i+MAX_REFERENCE_PICTURES)*3+2));
				}
			}
		}
	}
	else
	{
		pSlice->nNumberOfSlices = IMGPAR current_slice_nr+1;
		if(IMGPAR apply_weights)
		{
			for(j = 0; j < (int)pSlice->nNumberOfSlices; j++)
			{
				if(IMGPAR luma_log2_weight_denom)
					pSlice->SliceCtrlMap1[j].mWeightDenomLuma = (1<<IMGPAR luma_log2_weight_denom);
				else
					pSlice->SliceCtrlMap1[j].mWeightDenomLuma = 0;

				if(IMGPAR chroma_log2_weight_denom)
					pSlice->SliceCtrlMap1[j].mWeightDenomChroma = (1<<IMGPAR chroma_log2_weight_denom);
				else
					pSlice->SliceCtrlMap1[j].mWeightDenomChroma = 0;

				for(i = 0; i < listXsize[0]; i++)
				{
					offset = m_pSurfaceInfo[listX[0][i]->unique_id].SInfo[1];	//Bottom filed
					pSlice->SliceCtrlMap2[j].WOffset_L0[i+offset*16].mWeightOffsetY = *((IMGPAR wp_offset+(i+0)*3+0));
					pSlice->SliceCtrlMap2[j].WOffset_L0[i+offset*16].mWeightOffsetU = *((IMGPAR wp_offset+(i+0)*3+1));
					pSlice->SliceCtrlMap2[j].WOffset_L0[i+offset*16].mWeightOffsetV = *((IMGPAR wp_offset+(i+0)*3+2));
					pSlice->ExpWMap[j].WPacket_L0[i+offset*16].mWeightY = *((IMGPAR wp_weight+(i+0)*3+0));
					pSlice->ExpWMap[j].WPacket_L0[i+offset*16].mWeightU = *((IMGPAR wp_weight+(i+0)*3+1));
					pSlice->ExpWMap[j].WPacket_L0[i+offset*16].mWeightV = *((IMGPAR wp_weight+(i+0)*3+2));	
				}
				if((IMGPAR currentSlice->picture_type == B_SLICE) && active_pps.weighted_bipred_idc > 0)
				{
					for(i = 0; i < listXsize[1]; i++)
					{
						offset = m_pSurfaceInfo[listX[1][i]->unique_id].SInfo[1];	//Bottom filed
						pSlice->SliceCtrlMap2[j].WOffset_L1[IMGPAR m_L0L1Map[i]+offset*16].mWeightOffsetY = *((IMGPAR wp_offset+(i+MAX_REFERENCE_PICTURES)*3+0));
						pSlice->SliceCtrlMap2[j].WOffset_L1[IMGPAR m_L0L1Map[i]+offset*16].mWeightOffsetU = *((IMGPAR wp_offset+(i+MAX_REFERENCE_PICTURES)*3+1));
						pSlice->SliceCtrlMap2[j].WOffset_L1[IMGPAR m_L0L1Map[i]+offset*16].mWeightOffsetV = *((IMGPAR wp_offset+(i+MAX_REFERENCE_PICTURES)*3+2));
						pSlice->ExpWMap[j].WPacket_L1[IMGPAR m_L0L1Map[i]+offset*16].mWeightY = *((IMGPAR wp_weight+(i+MAX_REFERENCE_PICTURES)*3+0));
						pSlice->ExpWMap[j].WPacket_L1[IMGPAR m_L0L1Map[i]+offset*16].mWeightU = *((IMGPAR wp_weight+(i+MAX_REFERENCE_PICTURES)*3+1));
						pSlice->ExpWMap[j].WPacket_L1[IMGPAR m_L0L1Map[i]+offset*16].mWeightV = *((IMGPAR wp_weight+(i+MAX_REFERENCE_PICTURES)*3+2));
					}
				}
			}
		}
	}	

	DEBUG_SHOW_HW_INFO("ATI DXVA: End of build slice buffer");

	return 0;
}

void CH264DXVA2_ATI::build_macroblock_buffer_Inter_16x16 PARGS1(int list_offset)
{
	int pred_dir = currMB_d->b8pdir[0];
	int cbp_blk = currMB_d->cbp;
	int cbp=0, j, ref_picid, ref_picid_se;
	int curr_mb_field = ((IMGPAR MbaffFrameFlag)&&(currMB_s_d->mb_field));
	int current_mb_nr = IMGPAR current_mb_nr_d;

	H264_MC_CTRLMAP* MB_MC_Ctrl = (H264_MC_CTRLMAP*)(IMGPAR m_lpMBLK_Intra_Luma+IMGPAR mb_y_d*(IMGPAR width>>2)+(IMGPAR mb_x_d<<2));
	H264_SB_CTRLMAP* MB_SB_Ctrl = (H264_SB_CTRLMAP*)(IMGPAR m_lpMBLK_Intra_Luma+IMGPAR m_SBBufOffset+(IMGPAR mb_y_d<<2)*m_nMBBufStride+(IMGPAR mb_x_d<<3));
	H264_MVMAP* MB_MV = (H264_MVMAP*)(IMGPAR m_lpMV+(IMGPAR mb_y_d<<2)*m_nMVBufStride+(IMGPAR mb_x_d<<4));
	MB_MV += IMGPAR m_MVBufSwitch;

	*MB_MC_Ctrl  = curr_mb_field ? H264_FIELD_MB:0;
	*MB_MC_Ctrl |= (IMGPAR current_slice_nr << 8);

	H264_SB_CTRLMAP MB_SB_Ctrl_temp;
	H264_MVMAP MB_MV_0, MB_MV_1;

	if(pred_dir != 2)
	{
		ref_picid = currMB_s_d->pred_info.ref_pic_id[pred_dir][0];
		MB_SB_Ctrl_temp = 0;
		MB_SB_Ctrl_temp |= (IMGPAR apply_weights ? (1<<1):0);
		MB_SB_Ctrl_temp |= (pred_dir ? (1<<10):0);
		MB_SB_Ctrl_temp |= (m_pSurfaceInfo[ref_picid].SInfo[1] ? (1<<11):0);
		if(!curr_mb_field)
		{
			if(pred_dir)
				MB_SB_Ctrl_temp |= (WORD)((IMGPAR m_L0L1Map[currMB_s_d->pred_info.ref_idx[pred_dir][0]]) << 12);
			else
				MB_SB_Ctrl_temp |= (WORD)(currMB_s_d->pred_info.ref_idx[pred_dir][0] << 12);
		}
		else
		{
			if(current_mb_nr&1)
			{
				if(pred_dir)
					MB_SB_Ctrl_temp |= (WORD)((IMGPAR m_BotL1Map[currMB_s_d->pred_info.ref_idx[pred_dir][0]]) << 12);
				else
					MB_SB_Ctrl_temp |= (WORD)((IMGPAR m_BotL0Map[currMB_s_d->pred_info.ref_idx[pred_dir][0]]) << 12);
			}
			else
			{
				if(pred_dir)
					MB_SB_Ctrl_temp |= (WORD)((IMGPAR m_TopL1Map[currMB_s_d->pred_info.ref_idx[pred_dir][0]]) << 12);
				else
					MB_SB_Ctrl_temp |= (WORD)((IMGPAR m_TopL0Map[currMB_s_d->pred_info.ref_idx[pred_dir][0]]) << 12);
			}
		}	
		MB_MV_0 = ((DWORD)(currMB_s_d->pred_info.mv[pred_dir][0].x) & 0x0000ffff);
		MB_MV_0 |= (DWORD)(currMB_s_d->pred_info.mv[pred_dir][0].y << 16);
	}
	else
	{
		ref_picid = currMB_s_d->pred_info.ref_pic_id[LIST_0][0];
		ref_picid_se = currMB_s_d->pred_info.ref_pic_id[LIST_1][0];
		MB_SB_Ctrl_temp = 1;
		MB_SB_Ctrl_temp |= (IMGPAR apply_weights ? (1<<1):0);
		MB_SB_Ctrl_temp |= (m_pSurfaceInfo[ref_picid].SInfo[1] ? (1<<3):0);
		MB_SB_Ctrl_temp |= (m_pSurfaceInfo[ref_picid_se].SInfo[1] ? (1<<11):0);
		if(!curr_mb_field)
		{
			MB_SB_Ctrl_temp |= (WORD)(currMB_s_d->pred_info.ref_idx[LIST_0][0] << 4);
			MB_SB_Ctrl_temp |= (WORD)((IMGPAR m_L0L1Map[currMB_s_d->pred_info.ref_idx[LIST_1][0]]) << 12);
		}
		else
		{
			if(current_mb_nr&1)
			{
				MB_SB_Ctrl_temp |= (WORD)((IMGPAR m_BotL0Map[currMB_s_d->pred_info.ref_idx[LIST_0][0]]) << 4);
				MB_SB_Ctrl_temp |= (WORD)((IMGPAR m_BotL1Map[currMB_s_d->pred_info.ref_idx[LIST_1][0]]) << 12);
			}
			else
			{
				MB_SB_Ctrl_temp |= (WORD)((IMGPAR m_TopL0Map[currMB_s_d->pred_info.ref_idx[LIST_0][0]]) << 4);
				MB_SB_Ctrl_temp |= (WORD)((IMGPAR m_TopL1Map[currMB_s_d->pred_info.ref_idx[LIST_1][0]]) << 12);
			}
		}

		MB_MV_0 = ((DWORD)(currMB_s_d->pred_info.mv[LIST_0][0].x) & 0x0000ffff);
		MB_MV_0 |= (DWORD)(currMB_s_d->pred_info.mv[LIST_0][0].y << 16);
		MB_MV_1 = ((DWORD)(currMB_s_d->pred_info.mv[LIST_1][0].x) & 0x0000ffff);
		MB_MV_1 |= (DWORD)(currMB_s_d->pred_info.mv[LIST_1][0].y << 16);		
	}

	for(j=0; j<4; j++)
	{
		MB_SB_Ctrl[0] = MB_SB_Ctrl[1] = MB_SB_Ctrl[2] = MB_SB_Ctrl[3] = MB_SB_Ctrl_temp;

		if(pred_dir != 2)
		{
			MB_MV[0] = MB_MV[1] = MB_MV[2] = MB_MV[3] = MB_MV_0;
		}
		else
		{
			MB_MV[0] = MB_MV[1] = MB_MV[2] = MB_MV[3] = MB_MV_1;
			MB_MV[0-IMGPAR m_MVBufSwitch] = MB_MV[1-IMGPAR m_MVBufSwitch] = MB_MV[2-IMGPAR m_MVBufSwitch] = MB_MV[3-IMGPAR m_MVBufSwitch] = MB_MV_0;
		}

		MB_SB_Ctrl += (m_nMBBufStride>>1);
		MB_MV += (m_nMVBufStride>>2);
	}	
}

void CH264DXVA2_ATI::build_macroblock_buffer_Inter_16x8 PARGS1(int list_offset)
{
	int pred_dir;
	int cbp_blk = currMB_d->cbp;
	int cbp=0, joff, j, k,  ref_picid, ref_picid_se;
	int curr_mb_field = ((IMGPAR MbaffFrameFlag)&&(currMB_s_d->mb_field));
	int current_mb_nr = IMGPAR current_mb_nr_d;

	H264_MC_CTRLMAP* MB_MC_Ctrl = (H264_MC_CTRLMAP*)(IMGPAR m_lpMBLK_Intra_Luma+IMGPAR mb_y_d*(IMGPAR width>>2)+(IMGPAR mb_x_d<<2));
	H264_SB_CTRLMAP* MB_SB_Ctrl = (H264_SB_CTRLMAP*)(IMGPAR m_lpMBLK_Intra_Luma+IMGPAR m_SBBufOffset+(IMGPAR mb_y_d<<2)*m_nMBBufStride+(IMGPAR mb_x_d<<3));
	H264_MVMAP* MB_MV = (H264_MVMAP*)(IMGPAR m_lpMV+(IMGPAR mb_y_d<<2)*m_nMVBufStride+(IMGPAR mb_x_d<<4));
	MB_MV += IMGPAR m_MVBufSwitch;

	*MB_MC_Ctrl  = curr_mb_field ? H264_FIELD_MB:0;
	*MB_MC_Ctrl |= (IMGPAR current_slice_nr << 8);

	H264_SB_CTRLMAP MB_SB_Ctrl_temp;
	H264_MVMAP MB_MV_0, MB_MV_1;

	for(k=0;k<2;k++)
	{
		joff = (k<<3);
		pred_dir = currMB_d->b8pdir[k<<1];

		if(pred_dir != 2)
		{
			ref_picid = currMB_s_d->pred_info.ref_pic_id[pred_dir][k<<1];
			MB_SB_Ctrl_temp = 0;
			MB_SB_Ctrl_temp |= (IMGPAR apply_weights ? (1<<1):0);
			MB_SB_Ctrl_temp |= (pred_dir ? (1<<10):0);
			MB_SB_Ctrl_temp |= (m_pSurfaceInfo[ref_picid].SInfo[1] ? (1<<11):0);
			if(!curr_mb_field)
			{
				if(pred_dir)
					MB_SB_Ctrl_temp |= ((IMGPAR m_L0L1Map[currMB_s_d->pred_info.ref_idx[pred_dir][k<<1]]) << 12);
				else
					MB_SB_Ctrl_temp |= (currMB_s_d->pred_info.ref_idx[pred_dir][k<<1] << 12);
			}
			else
			{
				if(current_mb_nr&1)
				{
					if(pred_dir)
						MB_SB_Ctrl_temp |= ((IMGPAR m_BotL1Map[currMB_s_d->pred_info.ref_idx[pred_dir][k<<1]]) << 12);
					else
						MB_SB_Ctrl_temp |= ((IMGPAR m_BotL0Map[currMB_s_d->pred_info.ref_idx[pred_dir][k<<1]]) << 12);
				}
				else
				{
					if(pred_dir)
						MB_SB_Ctrl_temp |= ((IMGPAR m_TopL1Map[currMB_s_d->pred_info.ref_idx[pred_dir][k<<1]]) << 12);
					else
						MB_SB_Ctrl_temp |= ((IMGPAR m_TopL0Map[currMB_s_d->pred_info.ref_idx[pred_dir][k<<1]]) << 12);
				}
			}	
			MB_MV_0 = ((DWORD)(currMB_s_d->pred_info.mv[pred_dir][joff].x) & 0x0000ffff);
			MB_MV_0 |= (DWORD)(currMB_s_d->pred_info.mv[pred_dir][joff].y << 16);
		}
		else
		{
			ref_picid = currMB_s_d->pred_info.ref_pic_id[LIST_0][k<<1];
			ref_picid_se = currMB_s_d->pred_info.ref_pic_id[LIST_1][k<<1];
			MB_SB_Ctrl_temp = 1;
			MB_SB_Ctrl_temp |= (IMGPAR apply_weights ? (1<<1):0);
			MB_SB_Ctrl_temp |= (m_pSurfaceInfo[ref_picid].SInfo[1] ? (1<<3):0);
			MB_SB_Ctrl_temp |= (m_pSurfaceInfo[ref_picid_se].SInfo[1] ? (1<<11):0);
			if(!curr_mb_field)
			{
				MB_SB_Ctrl_temp |= (WORD)(currMB_s_d->pred_info.ref_idx[LIST_0][k<<1] << 4);
				MB_SB_Ctrl_temp |= (WORD)((IMGPAR m_L0L1Map[currMB_s_d->pred_info.ref_idx[LIST_1][k<<1]]) << 12);
			}
			else
			{
				if(current_mb_nr&1)
				{
					MB_SB_Ctrl_temp |= (WORD)((IMGPAR m_BotL0Map[currMB_s_d->pred_info.ref_idx[LIST_0][k<<1]]) << 4);
					MB_SB_Ctrl_temp |= (WORD)((IMGPAR m_BotL1Map[currMB_s_d->pred_info.ref_idx[LIST_1][k<<1]]) << 12);
				}
				else
				{
					MB_SB_Ctrl_temp |= (WORD)((IMGPAR m_TopL0Map[currMB_s_d->pred_info.ref_idx[LIST_0][k<<1]]) << 4);
					MB_SB_Ctrl_temp |= (WORD)((IMGPAR m_TopL1Map[currMB_s_d->pred_info.ref_idx[LIST_1][k<<1]]) << 12);
				}
			}		
			MB_MV_0 = ((DWORD)(currMB_s_d->pred_info.mv[LIST_0][joff].x) & 0x0000ffff);
			MB_MV_0 |= (DWORD)(currMB_s_d->pred_info.mv[LIST_0][joff].y << 16);
			MB_MV_1 = ((DWORD)(currMB_s_d->pred_info.mv[LIST_1][joff].x) & 0x0000ffff);
			MB_MV_1 |= (DWORD)(currMB_s_d->pred_info.mv[LIST_1][joff].y << 16);	
		}

		for(j=0; j<2; j++)
		{
			MB_SB_Ctrl[0] = MB_SB_Ctrl[1] = MB_SB_Ctrl[2] = MB_SB_Ctrl[3] = MB_SB_Ctrl_temp;

			if(pred_dir != 2)
			{
				MB_MV[0] = MB_MV[1] = MB_MV[2] = MB_MV[3] = MB_MV_0;
			}
			else
			{
				MB_MV[0] = MB_MV[1] = MB_MV[2] = MB_MV[3] = MB_MV_1;
				MB_MV[0-IMGPAR m_MVBufSwitch] = MB_MV[1-IMGPAR m_MVBufSwitch] = MB_MV[2-IMGPAR m_MVBufSwitch] = MB_MV[3-IMGPAR m_MVBufSwitch] = MB_MV_0;
			}

			MB_SB_Ctrl += (m_nMBBufStride>>1);
			MB_MV += (m_nMVBufStride>>2);
		}
	}
}

void CH264DXVA2_ATI::build_macroblock_buffer_Inter_8x16 PARGS1(int list_offset)
{
	int pred_dir;
	int cbp_blk = currMB_d->cbp;
	int cbp=0, i4, j, k, ref_picid, ref_picid_se;
	int curr_mb_field = ((IMGPAR MbaffFrameFlag)&&(currMB_s_d->mb_field));
	int current_mb_nr = IMGPAR current_mb_nr_d;

	H264_MC_CTRLMAP* MB_MC_Ctrl = (H264_MC_CTRLMAP*)(IMGPAR m_lpMBLK_Intra_Luma+IMGPAR mb_y_d*(IMGPAR width>>2)+(IMGPAR mb_x_d<<2));
	H264_SB_CTRLMAP* MB_SB_Ctrl = (H264_SB_CTRLMAP*)(IMGPAR m_lpMBLK_Intra_Luma+IMGPAR m_SBBufOffset+(IMGPAR mb_y_d<<2)*m_nMBBufStride+(IMGPAR mb_x_d<<3));
	H264_MVMAP* MB_MV = (H264_MVMAP*)(IMGPAR m_lpMV+(IMGPAR mb_y_d<<2)*m_nMVBufStride+(IMGPAR mb_x_d<<4));
	MB_MV += IMGPAR m_MVBufSwitch;

	*MB_MC_Ctrl  = curr_mb_field ? H264_FIELD_MB:0;
	*MB_MC_Ctrl |= (IMGPAR current_slice_nr << 8);

	H264_SB_CTRLMAP MB_SB_Ctrl_temp;
	H264_MVMAP MB_MV_0, MB_MV_1;

	for(k=0;k<2;k++)
	{
		i4 = (k<<1);
		pred_dir = currMB_d->b8pdir[k];

		if(pred_dir != 2)
		{
			ref_picid = currMB_s_d->pred_info.ref_pic_id[pred_dir][k];
			MB_SB_Ctrl_temp = 0;
			MB_SB_Ctrl_temp |= (IMGPAR apply_weights ? (1<<1):0);
			MB_SB_Ctrl_temp |= (pred_dir ? (1<<10):0);
			MB_SB_Ctrl_temp |= (m_pSurfaceInfo[ref_picid].SInfo[1] ? (1<<11):0);
			if(!curr_mb_field)
			{
				if(pred_dir)
					MB_SB_Ctrl_temp |= (WORD)((IMGPAR m_L0L1Map[currMB_s_d->pred_info.ref_idx[pred_dir][k]]) << 12);
				else
					MB_SB_Ctrl_temp |= (WORD)(currMB_s_d->pred_info.ref_idx[pred_dir][k] << 12);
			}
			else
			{
				if(current_mb_nr&1)
				{
					if(pred_dir)
						MB_SB_Ctrl_temp |= (WORD)((IMGPAR m_BotL1Map[currMB_s_d->pred_info.ref_idx[pred_dir][k]]) << 12);
					else
						MB_SB_Ctrl_temp |= (WORD)((IMGPAR m_BotL0Map[currMB_s_d->pred_info.ref_idx[pred_dir][k]]) << 12);
				}
				else
				{
					if(pred_dir)
						MB_SB_Ctrl_temp |= (WORD)((IMGPAR m_TopL1Map[currMB_s_d->pred_info.ref_idx[pred_dir][k]]) << 12);
					else
						MB_SB_Ctrl_temp |= (WORD)((IMGPAR m_TopL0Map[currMB_s_d->pred_info.ref_idx[pred_dir][k]]) << 12);
				}
			}
			MB_MV_0 = ((DWORD)(currMB_s_d->pred_info.mv[pred_dir][i4].x) & 0x0000ffff);
			MB_MV_0 |= (DWORD)(currMB_s_d->pred_info.mv[pred_dir][i4].y << 16);
		}
		else
		{
			ref_picid = currMB_s_d->pred_info.ref_pic_id[LIST_0][k];
			ref_picid_se = currMB_s_d->pred_info.ref_pic_id[LIST_1][k];
			MB_SB_Ctrl_temp = 1;
			MB_SB_Ctrl_temp |= (IMGPAR apply_weights ? (1<<1):0);
			MB_SB_Ctrl_temp |= (m_pSurfaceInfo[ref_picid].SInfo[1] ? (1<<3):0);
			MB_SB_Ctrl_temp |= (m_pSurfaceInfo[ref_picid_se].SInfo[1] ? (1<<11):0);
			if(!curr_mb_field)
			{
				MB_SB_Ctrl_temp |= (WORD)(currMB_s_d->pred_info.ref_idx[LIST_0][k] << 4);
				MB_SB_Ctrl_temp |= (WORD)((IMGPAR m_L0L1Map[currMB_s_d->pred_info.ref_idx[LIST_1][k]]) << 12);
			}
			else
			{
				if(current_mb_nr&1)
				{
					MB_SB_Ctrl_temp |= (WORD)((IMGPAR m_BotL0Map[currMB_s_d->pred_info.ref_idx[LIST_0][k]]) << 4);
					MB_SB_Ctrl_temp |= (WORD)((IMGPAR m_BotL1Map[currMB_s_d->pred_info.ref_idx[LIST_1][k]]) << 12);
				}
				else
				{
					MB_SB_Ctrl_temp |= (WORD)((IMGPAR m_TopL0Map[currMB_s_d->pred_info.ref_idx[LIST_0][k]]) << 4);
					MB_SB_Ctrl_temp |= (WORD)((IMGPAR m_TopL1Map[currMB_s_d->pred_info.ref_idx[LIST_1][k]]) << 12);
				}
			}
			MB_MV_0 = ((DWORD)(currMB_s_d->pred_info.mv[LIST_0][i4].x) & 0x0000ffff);
			MB_MV_0 |= ((DWORD)currMB_s_d->pred_info.mv[LIST_0][i4].y << 16);
			MB_MV_1 = ((DWORD)(currMB_s_d->pred_info.mv[LIST_1][i4].x) & 0x0000ffff);
			MB_MV_1 |= ((DWORD)currMB_s_d->pred_info.mv[LIST_1][i4].y << 16);
		}

		for(j=0; j<4; j++)
		{
			MB_SB_Ctrl[0] = MB_SB_Ctrl[1] = MB_SB_Ctrl_temp;

			if(pred_dir != 2)
			{
				MB_MV[0] = MB_MV[1] = MB_MV_0;
			}
			else
			{
				MB_MV[0] = MB_MV[1] = MB_MV_1;
				MB_MV[0-IMGPAR m_MVBufSwitch] = MB_MV[1-IMGPAR m_MVBufSwitch] = MB_MV_0;
			}

			MB_SB_Ctrl += (m_nMBBufStride>>1);
			MB_MV += (m_nMVBufStride>>2);
		}

		MB_SB_Ctrl -= (m_nMBBufStride<<1);
		MB_SB_Ctrl += 2;
		MB_MV -= m_nMVBufStride;
		MB_MV += 2;
	}
}

void CH264DXVA2_ATI::build_macroblock_buffer_Inter_8x8 PARGS1(int list_offset)
{
	int cbp_blk = currMB_d->cbp;
	int i, b4, b8, ref_picid, ref_picid_se;
	int pred_dir,mode;
	int fw_refframe,bw_refframe;
	static const int b84_idx[4][2] = { {0,4} , {2,6} , {8,12} , {10,14} };
	static const int b48_idx[4][2] = { {0,1} , {2,3} , {8,9} , {10,11} };
	static const int b44_idx[4][4] = { {0,1,4,5} , {2,3,6,7} , {8,9,12,13} , {10,11,14,15} };
	int curr_mb_field = ((IMGPAR MbaffFrameFlag)&&(currMB_s_d->mb_field));
	int current_mb_nr = IMGPAR current_mb_nr_d;

	H264_SB_CTRLMAP* MB_SB_Ctrl;
	H264_MVMAP* MB_MV;

	H264_MC_CTRLMAP* MB_MC_Ctrl = (H264_MC_CTRLMAP*)(IMGPAR m_lpMBLK_Intra_Luma+IMGPAR mb_y_d*(IMGPAR width>>2)+(IMGPAR mb_x_d<<2));
	H264_SB_CTRLMAP* MB_SB_Ctrl_o = (H264_SB_CTRLMAP*)(IMGPAR m_lpMBLK_Intra_Luma+IMGPAR m_SBBufOffset+(IMGPAR mb_y_d<<2)*m_nMBBufStride+(IMGPAR mb_x_d<<3));
	H264_MVMAP* MB_MV_o = (H264_MVMAP*)(IMGPAR m_lpMV+(IMGPAR mb_y_d<<2)*m_nMVBufStride+(IMGPAR mb_x_d<<4));
	MB_MV_o += IMGPAR m_MVBufSwitch;

	*MB_MC_Ctrl  = curr_mb_field ? H264_FIELD_MB:0;
	*MB_MC_Ctrl |= (IMGPAR current_slice_nr << 8);

	H264_SB_CTRLMAP MB_SB_Ctrl_temp;
	H264_MVMAP MB_MV_0, MB_MV_1;

	for (b8=0; b8<4; b8++)
	{
		MB_SB_Ctrl = MB_SB_Ctrl_o + ((b8&1)<<1) + m_nMBBufStride*((b8&2)>>1);
		MB_MV = MB_MV_o + ((b8&1)<<1) + (m_nMVBufStride>>1)*((b8&2)>>1);

		pred_dir = currMB_d->b8pdir[b8];
		mode = currMB_d->b8mode[b8];

		if(mode==0 && IMGPAR type == B_SLICE)
		{
			if (IMGPAR direct_spatial_mv_pred_flag) //equal to direct_flag is true in uv loop
			{
				fw_refframe = currMB_s_d->pred_info.ref_idx[LIST_0][b8];
				bw_refframe = currMB_s_d->pred_info.ref_idx[LIST_1][b8];
				//direct spatial
				if (active_sps.direct_8x8_inference_flag)
				{
					mode=PB_8x8;
					if (bw_refframe==-1)
						pred_dir=0;
					else if (fw_refframe==-1)
						pred_dir=1;
					else
						pred_dir=2;
				}
				else
				{
					mode=PB_4x4;
					if (bw_refframe==-1)
						pred_dir=0;
					else if (fw_refframe==-1)
						pred_dir=1;
					else
						pred_dir=2;
				}
			}
			else
			{
				pred_dir=2;
				//direct temporal
				if(active_sps.direct_8x8_inference_flag)
					mode=PB_8x8;
				else
					mode=PB_4x4;
			}
		}

		if(mode==PB_8x8 || mode==0)
		{
			if(pred_dir != 2)
			{
				ref_picid = currMB_s_d->pred_info.ref_pic_id[pred_dir][b8];
				MB_SB_Ctrl_temp = 0;
				MB_SB_Ctrl_temp |= (IMGPAR apply_weights ? (1<<1):0);
				MB_SB_Ctrl_temp |= (pred_dir ? (1<<10):0);
				MB_SB_Ctrl_temp |= (m_pSurfaceInfo[ref_picid].SInfo[1] ? (1<<11):0);
				if(!curr_mb_field)
				{
					if(pred_dir)
						MB_SB_Ctrl_temp |= (WORD)((IMGPAR m_L0L1Map[currMB_s_d->pred_info.ref_idx[pred_dir][b8]]) << 12);
					else
						MB_SB_Ctrl_temp |= (WORD)(currMB_s_d->pred_info.ref_idx[pred_dir][b8] << 12);
				}
				else
				{
					if(current_mb_nr&1)
					{
						if(pred_dir)
							MB_SB_Ctrl_temp |= (WORD)((IMGPAR m_BotL1Map[currMB_s_d->pred_info.ref_idx[pred_dir][b8]]) << 12);
						else
							MB_SB_Ctrl_temp |= (WORD)((IMGPAR m_BotL0Map[currMB_s_d->pred_info.ref_idx[pred_dir][b8]]) << 12);
					}
					else
					{
						if(pred_dir)
							MB_SB_Ctrl_temp |= (WORD)((IMGPAR m_TopL1Map[currMB_s_d->pred_info.ref_idx[pred_dir][b8]]) << 12);
						else
							MB_SB_Ctrl_temp |= (WORD)((IMGPAR m_TopL0Map[currMB_s_d->pred_info.ref_idx[pred_dir][b8]]) << 12);
					}
				}
				MB_MV_0 = ((DWORD)(currMB_s_d->pred_info.mv[pred_dir][b8_idx[b8]].x) & 0x0000ffff);
				MB_MV_0 |= (DWORD)(currMB_s_d->pred_info.mv[pred_dir][b8_idx[b8]].y << 16);
			}
			else
			{
				ref_picid = currMB_s_d->pred_info.ref_pic_id[LIST_0][b8];
				ref_picid_se = currMB_s_d->pred_info.ref_pic_id[LIST_1][b8];
				MB_SB_Ctrl_temp = 1;
				MB_SB_Ctrl_temp |= (IMGPAR apply_weights ? (1<<1):0);
				MB_SB_Ctrl_temp |= (m_pSurfaceInfo[ref_picid].SInfo[1] ? (1<<3):0);
				MB_SB_Ctrl_temp |= (m_pSurfaceInfo[ref_picid_se].SInfo[1] ? (1<<11):0);
				if(!curr_mb_field)
				{
					MB_SB_Ctrl_temp |= (WORD)(currMB_s_d->pred_info.ref_idx[LIST_0][b8] << 4);
					MB_SB_Ctrl_temp |= (WORD)((IMGPAR m_L0L1Map[currMB_s_d->pred_info.ref_idx[LIST_1][b8]]) << 12);
				}
				else
				{
					if(current_mb_nr&1)
					{
						MB_SB_Ctrl_temp |= (WORD)((IMGPAR m_BotL0Map[currMB_s_d->pred_info.ref_idx[LIST_0][b8]]) << 4);
						MB_SB_Ctrl_temp |= (WORD)((IMGPAR m_BotL1Map[currMB_s_d->pred_info.ref_idx[LIST_1][b8]]) << 12);
					}
					else
					{
						MB_SB_Ctrl_temp |= (WORD)((IMGPAR m_TopL0Map[currMB_s_d->pred_info.ref_idx[LIST_0][b8]]) << 4);
						MB_SB_Ctrl_temp |= (WORD)((IMGPAR m_TopL1Map[currMB_s_d->pred_info.ref_idx[LIST_1][b8]]) << 12);
					}
				}
				MB_MV_0 = ((DWORD)(currMB_s_d->pred_info.mv[LIST_0][b8_idx[b8]].x) & 0x0000ffff);
				MB_MV_0 |= (DWORD)(currMB_s_d->pred_info.mv[LIST_0][b8_idx[b8]].y << 16);
				MB_MV_1 = ((DWORD)(currMB_s_d->pred_info.mv[LIST_1][b8_idx[b8]].x) & 0x0000ffff);
				MB_MV_1 |= (DWORD)(currMB_s_d->pred_info.mv[LIST_1][b8_idx[b8]].y << 16);
			}

			MB_SB_Ctrl[0] = MB_SB_Ctrl[1] = 
				MB_SB_Ctrl[m_nMBBufStride>>1] = MB_SB_Ctrl[(m_nMBBufStride>>1)+1] = MB_SB_Ctrl_temp;

			if(pred_dir != 2)
			{
				MB_MV[0] = MB_MV[1] = 
					MB_MV[(m_nMVBufStride>>2)] = MB_MV[(m_nMVBufStride>>2)+1] = MB_MV_0;
			}
			else
			{
				MB_MV[0] = MB_MV[1] = 
					MB_MV[(m_nMVBufStride>>2)] = MB_MV[(m_nMVBufStride>>2)+1] = MB_MV_1;

				MB_MV[0-IMGPAR m_MVBufSwitch] = MB_MV[1-IMGPAR m_MVBufSwitch] = 
					MB_MV[(m_nMVBufStride>>2)-IMGPAR m_MVBufSwitch] = MB_MV[(m_nMVBufStride>>2)+1-IMGPAR m_MVBufSwitch] = MB_MV_0;				
			}			
		}		
		else if(mode==PB_8x4)
		{
			for(i=0 ; i<2 ; i++)
			{
				if(pred_dir != 2)
				{
					ref_picid = currMB_s_d->pred_info.ref_pic_id[pred_dir][b8];
					MB_SB_Ctrl[0] = 0;
					MB_SB_Ctrl[0] |= (IMGPAR apply_weights ? (1<<1):0);
					MB_SB_Ctrl[0] |= (pred_dir ? (1<<10):0);
					MB_SB_Ctrl[0] |= (m_pSurfaceInfo[ref_picid].SInfo[1] ? (1<<11):0);

					ref_picid = currMB_s_d->pred_info.ref_pic_id[pred_dir][b8];
					MB_SB_Ctrl[1] = 0;
					MB_SB_Ctrl[1] |= (IMGPAR apply_weights ? (1<<1):0);
					MB_SB_Ctrl[1] |= (pred_dir ? (1<<10):0);
					MB_SB_Ctrl[1] |= (m_pSurfaceInfo[ref_picid].SInfo[1] ? (1<<11):0);
					if(!curr_mb_field)
					{
						if(pred_dir)
						{
							MB_SB_Ctrl[0] |= (WORD)((IMGPAR m_L0L1Map[currMB_s_d->pred_info.ref_idx[pred_dir][b8]]) << 12);
							MB_SB_Ctrl[1] |= (WORD)((IMGPAR m_L0L1Map[currMB_s_d->pred_info.ref_idx[pred_dir][b8]]) << 12);
						}
						else
						{
							MB_SB_Ctrl[0] |= (WORD)(currMB_s_d->pred_info.ref_idx[pred_dir][b8] << 12);
							MB_SB_Ctrl[1] |= (WORD)(currMB_s_d->pred_info.ref_idx[pred_dir][b8] << 12);
						}
					}
					else
					{
						if(current_mb_nr&1)
						{
							if(pred_dir)
							{
								MB_SB_Ctrl[0] |= (WORD)((IMGPAR m_BotL1Map[currMB_s_d->pred_info.ref_idx[pred_dir][b8]]) << 12);
								MB_SB_Ctrl[1] |= (WORD)((IMGPAR m_BotL1Map[currMB_s_d->pred_info.ref_idx[pred_dir][b8]]) << 12);
							}
							else
							{
								MB_SB_Ctrl[0] |= (WORD)((IMGPAR m_BotL0Map[currMB_s_d->pred_info.ref_idx[pred_dir][b8]]) << 12);
								MB_SB_Ctrl[1] |= (WORD)((IMGPAR m_BotL0Map[currMB_s_d->pred_info.ref_idx[pred_dir][b8]]) << 12);
							}
						}
						else
						{
							if(pred_dir)
							{
								MB_SB_Ctrl[0] |= (WORD)((IMGPAR m_TopL1Map[currMB_s_d->pred_info.ref_idx[pred_dir][b8]]) << 12);
								MB_SB_Ctrl[1] |= (WORD)((IMGPAR m_TopL1Map[currMB_s_d->pred_info.ref_idx[pred_dir][b8]]) << 12);
							}
							else
							{
								MB_SB_Ctrl[0] |= (WORD)((IMGPAR m_TopL0Map[currMB_s_d->pred_info.ref_idx[pred_dir][b8]]) << 12);
								MB_SB_Ctrl[1] |= (WORD)((IMGPAR m_TopL0Map[currMB_s_d->pred_info.ref_idx[pred_dir][b8]]) << 12);
							}
						}
					}
					MB_MV_0 = ((DWORD)(currMB_s_d->pred_info.mv[pred_dir][b84_idx[b8][i]].x) & 0x0000ffff);
					MB_MV_0 |= (DWORD)(currMB_s_d->pred_info.mv[pred_dir][b84_idx[b8][i]].y << 16);
					MB_MV[0] = MB_MV[1] = MB_MV_0;					
				}
				else
				{
					ref_picid = currMB_s_d->pred_info.ref_pic_id[LIST_0][b8];
					ref_picid_se = currMB_s_d->pred_info.ref_pic_id[LIST_1][b8];
					MB_SB_Ctrl[0] = 1;
					MB_SB_Ctrl[0] |= (IMGPAR apply_weights ? (1<<1):0);
					MB_SB_Ctrl[0] |= (m_pSurfaceInfo[ref_picid].SInfo[1] ? (1<<3):0);
					MB_SB_Ctrl[0] |= (m_pSurfaceInfo[ref_picid_se].SInfo[1] ? (1<<11):0);

					ref_picid = currMB_s_d->pred_info.ref_pic_id[LIST_0][b8];
					ref_picid_se = currMB_s_d->pred_info.ref_pic_id[LIST_1][b8];
					MB_SB_Ctrl[1] = 1;
					MB_SB_Ctrl[1] |= (IMGPAR apply_weights ? (1<<1):0);
					MB_SB_Ctrl[1] |= (m_pSurfaceInfo[ref_picid].SInfo[1] ? (1<<3):0);
					MB_SB_Ctrl[1] |= (m_pSurfaceInfo[ref_picid_se].SInfo[1] ? (1<<11):0);
					if(!curr_mb_field)
					{
						MB_SB_Ctrl[0] |= (WORD)(currMB_s_d->pred_info.ref_idx[LIST_0][b8] << 4);
						MB_SB_Ctrl[1] |= (WORD)(currMB_s_d->pred_info.ref_idx[LIST_0][b8] << 4);
						MB_SB_Ctrl[0] |= (WORD)((IMGPAR m_L0L1Map[currMB_s_d->pred_info.ref_idx[LIST_1][b8]]) << 12);
						MB_SB_Ctrl[1] |= (WORD)((IMGPAR m_L0L1Map[currMB_s_d->pred_info.ref_idx[LIST_1][b8]]) << 12);
					}
					else
					{
						if(current_mb_nr&1)
						{
							MB_SB_Ctrl[0] |= (WORD)((IMGPAR m_BotL0Map[currMB_s_d->pred_info.ref_idx[LIST_0][b8]]) << 4);
							MB_SB_Ctrl[1] |= (WORD)((IMGPAR m_BotL0Map[currMB_s_d->pred_info.ref_idx[LIST_0][b8]]) << 4);
							MB_SB_Ctrl[0] |= (WORD)((IMGPAR m_BotL1Map[currMB_s_d->pred_info.ref_idx[LIST_1][b8]]) << 12);
							MB_SB_Ctrl[1] |= (WORD)((IMGPAR m_BotL1Map[currMB_s_d->pred_info.ref_idx[LIST_1][b8]]) << 12);
						}
						else
						{
							MB_SB_Ctrl[0] |= (WORD)((IMGPAR m_TopL0Map[currMB_s_d->pred_info.ref_idx[LIST_0][b8]]) << 4);
							MB_SB_Ctrl[1] |= (WORD)((IMGPAR m_TopL0Map[currMB_s_d->pred_info.ref_idx[LIST_0][b8]]) << 4);
							MB_SB_Ctrl[0] |= (WORD)((IMGPAR m_TopL1Map[currMB_s_d->pred_info.ref_idx[LIST_1][b8]]) << 12);
							MB_SB_Ctrl[1] |= (WORD)((IMGPAR m_TopL1Map[currMB_s_d->pred_info.ref_idx[LIST_1][b8]]) << 12);
						}
					}	

					MB_MV_0 = ((DWORD)(currMB_s_d->pred_info.mv[LIST_0][b84_idx[b8][i]].x) & 0x0000ffff);
					MB_MV_0 |= (DWORD)(currMB_s_d->pred_info.mv[LIST_0][b84_idx[b8][i]].y << 16);

					MB_MV[0-IMGPAR m_MVBufSwitch] = 
						MB_MV[1-IMGPAR m_MVBufSwitch] = MB_MV_0;

					MB_MV_1 = ((DWORD)(currMB_s_d->pred_info.mv[LIST_1][b84_idx[b8][i]].x) & 0x0000ffff);
					MB_MV_1 |= (DWORD)(currMB_s_d->pred_info.mv[LIST_1][b84_idx[b8][i]].y << 16);

					MB_MV[0] = 
						MB_MV[1] = MB_MV_1;
				}
				MB_SB_Ctrl += (m_nMBBufStride>>1);
				MB_MV += (m_nMVBufStride>>2);
			}
		}
		else if(mode==PB_4x8)
		{
			for(i=0 ; i<2 ; i++)
			{
				if(pred_dir != 2)
				{
					ref_picid = currMB_s_d->pred_info.ref_pic_id[pred_dir][b8];
					MB_SB_Ctrl[i] = 0;
					MB_SB_Ctrl[i] |= (IMGPAR apply_weights ? (1<<1):0);
					MB_SB_Ctrl[i] |= (pred_dir ? (1<<10):0);
					MB_SB_Ctrl[i] |= (m_pSurfaceInfo[ref_picid].SInfo[1] ? (1<<11):0);
					if(!curr_mb_field)
					{
						if(pred_dir)
							MB_SB_Ctrl[i] |= (WORD)((IMGPAR m_L0L1Map[currMB_s_d->pred_info.ref_idx[pred_dir][b8]]) << 12);
						else
							MB_SB_Ctrl[i] |= (WORD)(currMB_s_d->pred_info.ref_idx[pred_dir][b8] << 12);
					}
					else
					{
						if(current_mb_nr&1)
						{
							if(pred_dir)
								MB_SB_Ctrl[i] |= (WORD)((IMGPAR m_BotL1Map[currMB_s_d->pred_info.ref_idx[pred_dir][b8]]) << 12);
							else
								MB_SB_Ctrl[i] |= (WORD)((IMGPAR m_BotL0Map[currMB_s_d->pred_info.ref_idx[pred_dir][b8]]) << 12);
						}
						else
						{
							if(pred_dir)
								MB_SB_Ctrl[i] |= (WORD)((IMGPAR m_TopL1Map[currMB_s_d->pred_info.ref_idx[pred_dir][b8]]) << 12);
							else
								MB_SB_Ctrl[i] |= (WORD)((IMGPAR m_TopL0Map[currMB_s_d->pred_info.ref_idx[pred_dir][b8]]) << 12);
						}
					}
					ref_picid = currMB_s_d->pred_info.ref_pic_id[pred_dir][b8];
					MB_SB_Ctrl[i+(m_nMBBufStride>>1)] = 0;
					MB_SB_Ctrl[i+(m_nMBBufStride>>1)] |= (IMGPAR apply_weights ? (1<<1):0);
					MB_SB_Ctrl[i+(m_nMBBufStride>>1)] |= (pred_dir ? (1<<10):0);
					MB_SB_Ctrl[i+(m_nMBBufStride>>1)] |= (m_pSurfaceInfo[ref_picid].SInfo[1] ? (1<<11):0);
					if(!curr_mb_field)
					{
						if(pred_dir)
							MB_SB_Ctrl[i+(m_nMBBufStride>>1)] |= (WORD)((IMGPAR m_L0L1Map[currMB_s_d->pred_info.ref_idx[pred_dir][b8]]) << 12);
						else
							MB_SB_Ctrl[i+(m_nMBBufStride>>1)] |= (WORD)(currMB_s_d->pred_info.ref_idx[pred_dir][b8] << 12);
					}
					else
					{
						if(current_mb_nr&1)
						{
							if(pred_dir)
								MB_SB_Ctrl[i+(m_nMBBufStride>>1)] |= (WORD)((IMGPAR m_BotL1Map[currMB_s_d->pred_info.ref_idx[pred_dir][b8]]) << 12);
							else
								MB_SB_Ctrl[i+(m_nMBBufStride>>1)] |= (WORD)((IMGPAR m_BotL0Map[currMB_s_d->pred_info.ref_idx[pred_dir][b8]]) << 12);
						}
						else
						{
							if(pred_dir)
								MB_SB_Ctrl[i+(m_nMBBufStride>>1)] |= (WORD)((IMGPAR m_TopL1Map[currMB_s_d->pred_info.ref_idx[pred_dir][b8]]) << 12);
							else
								MB_SB_Ctrl[i+(m_nMBBufStride>>1)] |= (WORD)((IMGPAR m_TopL0Map[currMB_s_d->pred_info.ref_idx[pred_dir][b8]]) << 12);
						}
					}

					MB_MV_0 = ((DWORD)(currMB_s_d->pred_info.mv[pred_dir][b48_idx[b8][i]].x) & 0x0000ffff);
					MB_MV_0 |= (DWORD)(currMB_s_d->pred_info.mv[pred_dir][b48_idx[b8][i]].y << 16);

					MB_MV[i] = 
						MB_MV[i+(m_nMVBufStride>>2)] = MB_MV_0;	
				}
				else
				{
					ref_picid = currMB_s_d->pred_info.ref_pic_id[LIST_0][b8];
					ref_picid_se = currMB_s_d->pred_info.ref_pic_id[LIST_1][b8];
					MB_SB_Ctrl[i] = 1;
					MB_SB_Ctrl[i] |= (IMGPAR apply_weights ? (1<<1):0);
					MB_SB_Ctrl[i] |= (m_pSurfaceInfo[ref_picid].SInfo[1] ? (1<<3):0);
					MB_SB_Ctrl[i] |= (m_pSurfaceInfo[ref_picid_se].SInfo[1] ? (1<<11):0);
					if(!curr_mb_field)
					{
						MB_SB_Ctrl[i] |= (WORD)(currMB_s_d->pred_info.ref_idx[LIST_0][b8] << 4);
						MB_SB_Ctrl[i] |= (WORD)((IMGPAR m_L0L1Map[currMB_s_d->pred_info.ref_idx[LIST_1][b8]]) << 12);
					}
					else
					{
						if(current_mb_nr&1)
						{
							MB_SB_Ctrl[i] |= (WORD)((IMGPAR m_BotL0Map[currMB_s_d->pred_info.ref_idx[LIST_0][b8]]) << 4);
							MB_SB_Ctrl[i] |= (WORD)((IMGPAR m_BotL1Map[currMB_s_d->pred_info.ref_idx[LIST_1][b8]]) << 12);
						}
						else
						{
							MB_SB_Ctrl[i] |= (WORD)((IMGPAR m_TopL0Map[currMB_s_d->pred_info.ref_idx[LIST_0][b8]]) << 4);
							MB_SB_Ctrl[i] |= (WORD)((IMGPAR m_TopL1Map[currMB_s_d->pred_info.ref_idx[LIST_1][b8]]) << 12);
						}
					}
					ref_picid = currMB_s_d->pred_info.ref_pic_id[LIST_0][b8];
					ref_picid_se = currMB_s_d->pred_info.ref_pic_id[LIST_1][b8];
					MB_SB_Ctrl[i+(m_nMBBufStride>>1)] = 1;
					MB_SB_Ctrl[i+(m_nMBBufStride>>1)] |= (IMGPAR apply_weights ? (1<<1):0);
					MB_SB_Ctrl[i+(m_nMBBufStride>>1)] |= (m_pSurfaceInfo[ref_picid].SInfo[1] ? (1<<3):0);
					MB_SB_Ctrl[i+(m_nMBBufStride>>1)] |= (m_pSurfaceInfo[ref_picid_se].SInfo[1] ? (1<<11):0);	
					if(!curr_mb_field)
					{
						MB_SB_Ctrl[i+(m_nMBBufStride>>1)] |= (WORD)(currMB_s_d->pred_info.ref_idx[LIST_0][b8] << 4);
						MB_SB_Ctrl[i+(m_nMBBufStride>>1)] |= (WORD)((IMGPAR m_L0L1Map[currMB_s_d->pred_info.ref_idx[LIST_1][b8]]) << 12);
					}
					else
					{
						if(current_mb_nr&1)
						{
							MB_SB_Ctrl[i+(m_nMBBufStride>>1)] |= (WORD)((IMGPAR m_BotL0Map[currMB_s_d->pred_info.ref_idx[LIST_0][b8]]) << 4);
							MB_SB_Ctrl[i+(m_nMBBufStride>>1)] |= (WORD)((IMGPAR m_BotL1Map[currMB_s_d->pred_info.ref_idx[LIST_1][b8]]) << 12);
						}
						else
						{
							MB_SB_Ctrl[i+(m_nMBBufStride>>1)] |= (WORD)((IMGPAR m_TopL0Map[currMB_s_d->pred_info.ref_idx[LIST_0][b8]]) << 4);
							MB_SB_Ctrl[i+(m_nMBBufStride>>1)] |= (WORD)((IMGPAR m_TopL1Map[currMB_s_d->pred_info.ref_idx[LIST_1][b8]]) << 12);
						}
					}

					MB_MV_0 = ((DWORD)(currMB_s_d->pred_info.mv[LIST_0][b48_idx[b8][i]].x) & 0x0000ffff);
					MB_MV_0 |= (DWORD)(currMB_s_d->pred_info.mv[LIST_0][b48_idx[b8][i]].y << 16);

					MB_MV[i-IMGPAR m_MVBufSwitch] = 
						MB_MV[i+(m_nMVBufStride>>2)-IMGPAR m_MVBufSwitch] = MB_MV_0;

					MB_MV_1 = ((DWORD)(currMB_s_d->pred_info.mv[LIST_1][b48_idx[b8][i]].x) & 0x0000ffff);
					MB_MV_1 |= (DWORD)(currMB_s_d->pred_info.mv[LIST_1][b48_idx[b8][i]].y << 16);

					MB_MV[i] = 
						MB_MV[i+(m_nMVBufStride>>2)] = MB_MV_1;	
				}
			}
		}
		else
		{		//PB_4x4
			for(b4=0 ; b4<4 ; b4++)
			{
				i = (b4&0x01);
				if(pred_dir != 2)
				{
					ref_picid = currMB_s_d->pred_info.ref_pic_id[pred_dir][b8];
					MB_SB_Ctrl[i] = 0;
					MB_SB_Ctrl[i] |= (IMGPAR apply_weights ? (1<<1):0);
					MB_SB_Ctrl[i] |= (pred_dir ? (1<<10):0);
					MB_SB_Ctrl[i] |= (m_pSurfaceInfo[ref_picid].SInfo[1] ? (1<<11):0);
					if(!curr_mb_field)
					{
						if(pred_dir)
							MB_SB_Ctrl[i] |= (WORD)((IMGPAR m_L0L1Map[currMB_s_d->pred_info.ref_idx[pred_dir][b8]]) << 12);
						else
							MB_SB_Ctrl[i] |= (WORD)(currMB_s_d->pred_info.ref_idx[pred_dir][b8] << 12);
					}
					else
					{
						if(current_mb_nr&1)
						{
							if(pred_dir)
								MB_SB_Ctrl[i] |= (WORD)((IMGPAR m_BotL1Map[currMB_s_d->pred_info.ref_idx[pred_dir][b8]]) << 12);
							else
								MB_SB_Ctrl[i] |= (WORD)((IMGPAR m_BotL0Map[currMB_s_d->pred_info.ref_idx[pred_dir][b8]]) << 12);
						}
						else
						{
							if(pred_dir)
								MB_SB_Ctrl[i] |= (WORD)((IMGPAR m_TopL1Map[currMB_s_d->pred_info.ref_idx[pred_dir][b8]]) << 12);
							else
								MB_SB_Ctrl[i] |= (WORD)((IMGPAR m_TopL0Map[currMB_s_d->pred_info.ref_idx[pred_dir][b8]]) << 12);
						}
					}
					MB_MV[i] = ((DWORD)(currMB_s_d->pred_info.mv[pred_dir][b44_idx[b8][b4]].x) & 0x0000ffff);
					MB_MV[i] |= (DWORD)(currMB_s_d->pred_info.mv[pred_dir][b44_idx[b8][b4]].y << 16);
				}
				else
				{
					ref_picid = currMB_s_d->pred_info.ref_pic_id[LIST_0][b8];
					ref_picid_se = currMB_s_d->pred_info.ref_pic_id[LIST_1][b8];
					MB_SB_Ctrl[i] = 1;
					MB_SB_Ctrl[i] |= (IMGPAR apply_weights ? (1<<1):0);
					MB_SB_Ctrl[i] |= (m_pSurfaceInfo[ref_picid].SInfo[1] ? (1<<3):0);
					MB_SB_Ctrl[i] |= (m_pSurfaceInfo[ref_picid_se].SInfo[1] ? (1<<11):0);
					if(!curr_mb_field)
					{
						MB_SB_Ctrl[i] |= (WORD)(currMB_s_d->pred_info.ref_idx[LIST_0][b8] << 4);
						MB_SB_Ctrl[i] |= (WORD)((IMGPAR m_L0L1Map[currMB_s_d->pred_info.ref_idx[LIST_1][b8]]) << 12);
					}
					else
					{
						if(current_mb_nr&1)
						{
							MB_SB_Ctrl[i] |= (WORD)((IMGPAR m_BotL0Map[currMB_s_d->pred_info.ref_idx[LIST_0][b8]]) << 4);
							MB_SB_Ctrl[i] |= (WORD)((IMGPAR m_BotL1Map[currMB_s_d->pred_info.ref_idx[LIST_1][b8]]) << 12);
						}
						else
						{
							MB_SB_Ctrl[i] |= (WORD)((IMGPAR m_TopL0Map[currMB_s_d->pred_info.ref_idx[LIST_0][b8]]) << 4);
							MB_SB_Ctrl[i] |= (WORD)((IMGPAR m_TopL1Map[currMB_s_d->pred_info.ref_idx[LIST_1][b8]]) << 12);
						}
					}			
					MB_MV[i-IMGPAR m_MVBufSwitch] = ((DWORD)(currMB_s_d->pred_info.mv[LIST_0][b44_idx[b8][b4]].x) & 0x0000ffff);
					MB_MV[i-IMGPAR m_MVBufSwitch] |= (DWORD)(currMB_s_d->pred_info.mv[LIST_0][b44_idx[b8][b4]].y << 16);
					MB_MV[i] = ((DWORD)(currMB_s_d->pred_info.mv[LIST_1][b44_idx[b8][b4]].x) & 0x0000ffff);
					MB_MV[i] |= (DWORD)(currMB_s_d->pred_info.mv[LIST_1][b44_idx[b8][b4]].y << 16);
				}
				if(b4==1)
				{
					MB_SB_Ctrl += (m_nMBBufStride>>1);
					MB_MV += (m_nMVBufStride>>2);
				}
			}
		}
	}
}

int CH264DXVA2_ATI::ExecuteBuffers PARGS1(DWORD dwFlags)
{
	ExecuteBuffersStruct cur_img;// = new ExecuteBuffersStruct; //need to be change from outside.

	build_picture_decode_buffer ARGS0();
	dec_picture->pic_store_idx = IMGPAR UnCompress_Idx;

	cur_img.currentSlice_picture_type = IMGPAR currentSlice->picture_type;
	cur_img.currentSlice_structure = IMGPAR currentSlice->structure;
	cur_img.UnCompress_Idx = IMGPAR UnCompress_Idx;
	cur_img.smart_dec = IMGPAR smart_dec;
	cur_img.slice_number = IMGPAR slice_number;
	cur_img.nal_reference_idc = IMGPAR nal_reference_idc;
	cur_img.m_is_MTMS = stream_global->m_is_MTMS;
	cur_img.FrameSizeInMbs = IMGPAR FrameSizeInMbs;
	cur_img.PicSizeInMbs = IMGPAR PicSizeInMbs;

	cur_img.m_iVGAType = m_nVGAType;
	cur_img.m_pic_combine_status = IMGPAR currentSlice->m_pic_combine_status;
	if (IMGPAR currentSlice->structure!=FRAME && IMGPAR currentSlice->m_pic_combine_status)
		cur_img.m_Intra_lCompBufIndex = IMGPAR m_iFirstCompBufIndex;
	else
		cur_img.m_Intra_lCompBufIndex = IMGPAR m_Intra_lCompBufIndex;
	cur_img.m_lFrame_Counter = m_nFrameCounter;

	cur_img.m_pnv1PictureDecode = m_pbPictureParamBuf[IMGPAR m_Intra_lCompBufIndex];
	cur_img.m_lpMBLK_Intra_Luma = m_pbMacroblockCtrlBuf[IMGPAR m_Intra_lCompBufIndex];
	cur_img.m_lpSLICE						= m_pbSliceCtrlBuf[IMGPAR m_Intra_lCompBufIndex];
	cur_img.m_lpMV							= m_pbMotionVectorBuf[IMGPAR m_Intra_lCompBufIndex];
	cur_img.m_lpRESD_Intra_Luma = m_pbResidualDiffBuf[IMGPAR m_Intra_lCompBufIndex];

	cur_img.width = IMGPAR width;
	cur_img.m_PicHeight = IMGPAR m_PicHeight;
	cur_img.m_SBBufOffset = IMGPAR m_SBBufOffset;
	cur_img.m_ReBufStride = m_nReBufStride;
	cur_img.m_MVBufStride = m_nMVBufStride;
	cur_img.m_unique_id = dec_picture->unique_id;
	cur_img.m_pIviCP = m_pIviCP;
	cur_img.m_bSkipedBFrame = dec_picture->SkipedBFrames[dec_picture->view_index][0];
	cur_img.m_bDMATransfer = false;
	cur_img.m_iMbaffFrameFlag = IMGPAR MbaffFrameFlag;

	EnterCriticalSection( &crit_ACCESS_QUEUE );
	m_Queue->push(cur_img);	
	long count;
	ReleaseSemaphore(
		stream_global->m_queue_semaphore, 
		1, 
		&count
		);
	LeaveCriticalSection( &crit_ACCESS_QUEUE );

	m_nFrameCounter++;

	return 0;
}

void CH264DXVA2_ATI::build_residual_buffer_Inter PARGS0()
{
	int b8, b4, ioff, joff, b8off;
	short *pcof = (short*) IMGPAR cof_d;
	int cbp_8x8 = currMB_d->cbp;
	int cbp_blk = currMB_s_d->cbp_blk;
	static const int b44_idx[4][4] = { {0,1,4,5} , {2,3,6,7} , {8,9,12,13} , {10,11,14,15} };
	static const int b44_offset[4][4] = { {0,16,64,80} , {32,48,96,112} , {128,144,192,208} , {160,176,224,240} };	
	H264_RD* MB_Residual = (H264_RD*)IMGPAR m_lpRESD_Intra_Luma+(IMGPAR current_mb_nr_d<<8);
	int iPicWidth = IMGPAR width;
	__m128i xmm0 = _mm_setzero_si128(); // all 0s

	//memset(Residual, 0, sizeof(short)*256);

	//luma
	for(b8=0 ; b8<4 ; b8++)
	{
		register short* b8off_plane = (short*)(MB_Residual + ((b8>>1)*16<<3) + ((b8&1)<<3));
		b8off = ((b8>>1)<<7)+((b8&1)<<3);	// 0 8 128 136
		if(1 & (cbp_8x8>>b8))
		{
			if (!currMB_d->luma_transform_size_8x8_flag)
			{
				for(b4=0 ; b4<4 ; b4+=2)
				{	
					register short*  offset_plane = b8off_plane + ((b4>>1)*16<<2) + ((b4&1)<<2) ;
					//if( (cbp_blk>>b44_idx[b8][b4]) & 1 )
					iDCT_4x4_2_ATI(offset_plane, pcof+b44_offset[b8][b4], 16);
					/*else
					{
					offset = b8off+(((b4>>1)<<6)+((b4&1)<<2));
					memset(&Residual[offset], 0, 8);
					memset(&Residual[offset+16], 0, 8);
					memset(&Residual[offset+32], 0, 8);
					memset(&Residual[offset+48], 0, 8);
					}*/
				} 
			}
			else
				iDCT_8x8_fcn(b8off_plane, IMGPAR cof_d + (b8<<6), 16);
		}
		else
		{
			/*
			memset(&Residual[b8off], 0, 16);
			memset(&Residual[b8off+16], 0, 16);
			memset(&Residual[b8off+32], 0, 16);
			memset(&Residual[b8off+48], 0, 16);
			memset(&Residual[b8off+64], 0, 16);
			memset(&Residual[b8off+80], 0, 16);
			memset(&Residual[b8off+96], 0, 16);
			memset(&Residual[b8off+112], 0, 16);
			*/
			_mm_store_si128((__m128i*)b8off_plane, xmm0); //0
			b8off_plane += 16;			
			_mm_store_si128((__m128i*)b8off_plane, xmm0); //0
			b8off_plane += 16;			
			_mm_store_si128((__m128i*)b8off_plane, xmm0); //0
			b8off_plane += 16;			
			_mm_store_si128((__m128i*)b8off_plane, xmm0); //0
			b8off_plane += 16;			
			_mm_store_si128((__m128i*)b8off_plane, xmm0); //0
			b8off_plane += 16;			
			_mm_store_si128((__m128i*)b8off_plane, xmm0); //0
			b8off_plane += 16;			
			_mm_store_si128((__m128i*)b8off_plane, xmm0); //0
			b8off_plane += 16;			
			_mm_store_si128((__m128i*)b8off_plane, xmm0); //0
		}
	}

	//chroma	
	MB_Residual = (H264_RD*)IMGPAR m_lpRESD_Intra_Luma+(IMGPAR m_PicHeight*IMGPAR width)+(IMGPAR current_mb_nr_d<<7);
#if   !defined(H264_ENABLE_INTRINSICS)
	short tmp4[4][4];
	int uv, j, i;
	short* Residual_UV;
	for(uv=0;uv<2;uv++)
	{			
		for(b8=0;b8<4;b8++)
		{				
			ioff = (b8&1)<<2;	//0 4 0 4
			joff = (b8&2)<<1;	//0 0 4 4
			if( currMB_s_d->cbp_blk & (1<<cbp_blk_chroma[uv][b8]))
				iDCT_4x4_fcn(&tmp4[0][0], (IMGPAR cof_d + ((4+uv<<6)+(b8<<4))),4);
			else
				memset(tmp4, 0, sizeof(short)*16);

			for(j = 0; j < 4; j++)
				for(i = 0; i < 4; i++)
					Residual_UV[(joff+j)*16+(ioff+i)*2+uv] = tmp4[j][i];			
		}
	}
#else
	for(b8=0;b8<4;b8++)
	{
		register short* offset_plane = (short*)( MB_Residual + ((b8>>1)*16<<2) + ((b8&1)<<3) ) ;

		ioff = (b8&1)<<3;	//0 8 0  8
		joff = (b8&2)<<5;	//0 0 64 64

		iDCT_4x4_UV_fcn(offset_plane
			, (IMGPAR cof_d + ((4<<6)+(b8<<4)))
			,( currMB_s_d->cbp_blk & (1<<cbp_blk_chroma[0][b8])) // residual U
			,( currMB_s_d->cbp_blk & (1<<cbp_blk_chroma[1][b8])) // residual V
			, 16);
	}
#endif
}

int CH264DXVA2_ATI::I_Pred_luma_16x16 PARGS1(int predmode)
{
	switch(predmode)
	{
	case VERT_PRED_16:
		return LUMA_VERT_PRED_16;

	case HOR_PRED_16:
		return LUMA_HOR_PRED_16;

	case PLANE_16:
		return LUMA_PLANE_16;

	case DC_PRED_16:
		{
			int i;			
			int mb_nr=IMGPAR current_mb_nr_d;		
			PixelPos up;          //!< pixel position p(0,-1)
			PixelPos left[2];    //!< pixel positions p(-1, -1..15)
			PixelPos left_up;				
			int up_avail, left_avail, left_up_avail;

			//left-up
			getCurrNeighbourD_Luma ARGS3(-1, -1, &left_up);

			getCurrNeighbourA_Luma ARGS3(-1, 0, &left[0]);
			getCurrNeighbourA_Luma ARGS3(-1, 8, &left[1]);

			//up  
			getCurrNeighbourB_Luma ARGS3(0, -1, &up);

			if (!active_pps.constrained_intra_pred_flag)
			{
				up_avail   = (up.pMB!=NULL);
				left_avail = (left[0].pMB!=NULL);
				left_up_avail = (left_up.pMB!=NULL);
			}
			else
			{
				up_avail      = (up.pMB!=NULL);
				for (i=0, left_avail=1; i<2;i++)
					left_avail  &= (left[i].pMB != NULL);
				left_up_avail = (left_up.pMB != NULL);
			}

			// DC prediction		
			if (up_avail && left_avail)
				return LUMA_DC_PRED_16_UP_LF;       // no edge
			else if (!up_avail && left_avail)
				return LUMA_DC_PRED_16_LF;			// upper edge
			else if (up_avail && !left_avail)
				return LUMA_DC_PRED_16_UP;			// left edge
			else //if (!up_avail && !left_avail)
				return LUMA_DC_PRED_16_NONE;		// top left corner, nothing to predict from
		}			
	default:
		return 0;
	}
}

int CH264DXVA2_ATI::I_Pred_luma_4x4 PARGS4(
																			 int ioff,             //!< pixel offset X within MB
																			 int joff,             //!< pixel offset Y within MB
																			 int img_block_x,      //!< location of block X, multiples of 4
																			 int img_block_y)      //!< location of block Y, multiples of 4
{
	byte predmode = currMB_d->ipredmode[(img_block_y&3)*4+(img_block_x&3)];
	PixelPos pix_c;
	int mb_nr=IMGPAR current_mb_nr_d;
	int block_available_up_right;

	if (ioff)
	{
		if (joff)
		{
			if ((ioff+4)<16)
				pix_c.pMB = currMB_d;
			else
				pix_c.pMB = NULL;
		}
		else
		{
			if ((ioff+4)<16)
				getCurrNeighbourB_Luma ARGS3(ioff+4, joff-1, &pix_c);
			else
				getCurrNeighbourC_Luma ARGS3(ioff+4, joff-1, &pix_c);
		}
	}
	else
	{
		if (joff)
			pix_c.pMB = currMB_d;
		else
			getCurrNeighbourB_Luma ARGS3(ioff+4, joff-1, &pix_c);
	}

	if (pix_c.pMB) {
		if (((ioff==4)||(ioff==12)) && ((joff==4)||(joff==12))) {
			pix_c.pMB = NULL;
		}
	}

	block_available_up_right = (pix_c.pMB!=NULL);

	IMGPAR m_UpRight = block_available_up_right ? 1:0;

	switch(predmode)
	{
	case VERT_PRED:                       /* vertical prediction from block above */
		return LUMA_VERT_PRED_4;

	case HOR_PRED:                        /* horizontal prediction from left block */
		return LUMA_HOR_PRED_4;

	case DIAG_DOWN_RIGHT_PRED:
		return LUMA_DIAG_DOWN_RIGHT_PRED_4;

	case DIAG_DOWN_LEFT_PRED:
		return LUMA_DIAG_DOWN_LEFT_PRED_4;

	case  VERT_RIGHT_PRED:/* diagonal prediction -22.5 deg to horizontal plane */
		return LUMA_VERT_RIGHT_PRED_4;

	case  VERT_LEFT_PRED:/* diagonal prediction -22.5 deg to horizontal plane */
		return LUMA_VERT_LEFT_PRED_4;

	case  HOR_UP_PRED:/* diagonal prediction -22.5 deg to horizontal plane */
		return LUMA_HOR_UP_PRED_4;

	case  HOR_DOWN_PRED:/* diagonal prediction -22.5 deg to horizontal plane */
		return LUMA_HOR_DOWN_PRED_4;

	case DC_PRED:                         /* DC prediction */
		{
			int img_y,img_x;	
			PixelPos pix_a;
			PixelPos pix_b, pix_d;	
			int block_available_up;
			int block_available_left;
			int block_available_up_left;	

			img_x=img_block_x*4;
			img_y=img_block_y*4;

			if (ioff)
			{	
				block_available_left = 1;

				if (joff)
				{
					pix_b.pMB = currMB_d;
					pix_d.pMB = currMB_d;
				}	
				else
				{
					getCurrNeighbourB_Luma ARGS3(ioff, joff-1, &pix_b);
					getCurrNeighbourB_Luma ARGS3(ioff-1, joff-1, &pix_d);
				}
			}
			else
			{
				getCurrNeighbourA_Luma ARGS3(ioff-1, joff,   &pix_a);

				if(pix_a.pMB)
				{
					block_available_left = 1;	

					if (active_pps.constrained_intra_pred_flag)
					{
						if (IMGPAR MbaffFrameFlag)
						{												
							switch(currMB_d->mbStatusA)
							{
							case 1:					
							case 2:			
								//need to check both odd and even macroblocks
								block_available_left = (currMB_d->mbIntraA==0x03);
								break;															
							default:								
								break;
							}			
						}						
					}
				}
				else
					block_available_left = 0;		

				if (joff)
				{
					pix_b.pMB = currMB_d;
					getCurrNeighbourA_Luma ARGS3(ioff-1, joff-1, &pix_d);
				}
				else
				{
					getCurrNeighbourB_Luma ARGS3(ioff, joff-1, &pix_b);
					getCurrNeighbourD_Luma ARGS3(ioff-1, joff-1, &pix_d);
				}
			}			

			block_available_up       = (pix_b.pMB!=NULL);
			block_available_up_right = (pix_c.pMB!=NULL);
			block_available_up_left  = (pix_d.pMB!=NULL);

			IMGPAR m_UpRight = block_available_up_right ? 1:0;

			if (block_available_up && block_available_left)
				return LUMA_DC_PRED_4_UP_LF;

			else if (!block_available_up && block_available_left)
				return LUMA_DC_PRED_4_LF;

			else if (block_available_up && !block_available_left)
				return LUMA_DC_PRED_4_UP;

			else //if (!block_available_up && !block_available_left)
				return LUMA_DC_PRED_4_NONE;
		}	  
	default:
		return 0;
	}
}

int CH264DXVA2_ATI::I_Pred_luma_8x8 PARGS1(int b8)
{
	int img_block_x = (IMGPAR mb_x_d)*4 + 2*(b8%2);
	int img_block_y = (IMGPAR mb_y_d)*4 + 2*(b8/2);
	int ioff = (b8%2)*8;
	int joff = (b8/2)*8;
	int mb_nr=IMGPAR current_mb_nr_d;
	byte predmode = currMB_d->ipredmode[(img_block_y&3)*4+(img_block_x&3)];
	PixelPos pix_c, pix_d;
	int block_available_up_left,block_available_up_right;

	if (ioff)
	{
		if (joff)
		{
			pix_d.pMB = currMB_d;
			if ((ioff+8)<16)
				pix_c.pMB = currMB_d;
			else
				pix_c.pMB = NULL;
		}
		else
		{
			getCurrNeighbourB_Luma ARGS3(ioff-1, joff-1, &pix_d);
			if ((ioff+8)<16)
				getCurrNeighbourB_Luma ARGS3(ioff+8, joff-1, &pix_c);
			else
				getCurrNeighbourC_Luma ARGS3(ioff+8, joff-1, &pix_c);
		}
	}
	else
	{
		if (joff)
		{
			getCurrNeighbourA_Luma ARGS3(ioff-1, joff-1, &pix_d);
			pix_c.pMB = currMB_d;
		}
		else
		{
			getCurrNeighbourD_Luma ARGS3(ioff-1, joff-1, &pix_d);
			getCurrNeighbourB_Luma ARGS3(ioff+8, joff-1, &pix_c);
		}
	}

	if (pix_c.pMB) {
		if (ioff == 8 && joff == 8) {
			pix_c.pMB = NULL;
		}
	}

	block_available_up_right = (pix_c.pMB!=NULL);
	block_available_up_left  = (pix_d.pMB!=NULL);

	IMGPAR m_UpRight = block_available_up_right? 1:0;
	IMGPAR m_UpLeft = block_available_up_left? 1:0;

	switch(predmode)
	{
	case VERT_PRED:
		return LUMA_VERT_PRED_8;

	case HOR_PRED:
		return LUMA_HOR_PRED_8;

	case DIAG_DOWN_LEFT_PRED:
		return LUMA_DIAG_DOWN_LEFT_PRED_8;

	case VERT_LEFT_PRED:
		return LUMA_VERT_LEFT_PRED_8;

	case DIAG_DOWN_RIGHT_PRED:
		return LUMA_DIAG_DOWN_RIGHT_PRED_8;

	case  VERT_RIGHT_PRED:/* diagonal prediction -22.5 deg to horizontal plane */
		return LUMA_VERT_RIGHT_PRED_8;

	case  HOR_DOWN_PRED:/* diagonal prediction -22.5 deg to horizontal plane */
		return LUMA_HOR_DOWN_PRED_8;

	case  HOR_UP_PRED:/* diagonal prediction -22.5 deg to horizontal plane */
		return LUMA_HOR_UP_PRED_8;

	case DC_PRED:
		{	
			PixelPos pix_a;
			PixelPos pix_b;

			int block_available_up;
			int block_available_left;

			if (ioff)
			{
				block_available_left = 1;

				if (joff)
					pix_b.pMB = currMB_d;
				else
					getCurrNeighbourB_Luma ARGS3(ioff, joff-1, &pix_b);
			}
			else
			{
				getCurrNeighbourA_Luma ARGS3(ioff-1, joff,   &pix_a);

				if(pix_a.pMB)
				{
					block_available_left = 1;	

					if (active_pps.constrained_intra_pred_flag)
					{
						if (IMGPAR MbaffFrameFlag)
						{												
							switch(currMB_d->mbStatusA)
							{
							case 1:					
							case 2:
								block_available_left = (currMB_d->mbIntraA==0x03);			
								break;							
							default:						
								break;
							}										
						}						
					}
				}
				else
					block_available_left = 0;	

				if (joff)
					pix_b.pMB = currMB_d;
				else
					getCurrNeighbourB_Luma ARGS3(ioff, joff-1, &pix_b);
			}

			block_available_up       = (pix_b.pMB!=NULL);

			IMGPAR m_UpRight = block_available_up_right? 1:0;
			IMGPAR m_UpLeft = block_available_up_left? 1:0;

			if (block_available_up && block_available_left)
				return LUMA_DC_PRED_8_UP_LF;

			else if (!block_available_up && block_available_left)
				return LUMA_DC_PRED_8_LF;

			else if (block_available_up && !block_available_left)
				return LUMA_DC_PRED_8_UP;

			else //if (!block_available_up && !block_available_left)
				return LUMA_DC_PRED_8_NONE;
		}

	default:
		return 0;
	}
}

int CH264DXVA2_ATI::I_Pred_chroma PARGS1(int uv)
{
	switch (currMB_d->c_ipred_mode)
	{
	case PLANE_8:
		return CHROMA_PLANE_8;

	case HOR_PRED_8:
		return CHROMA_HOR_PRED_8;

	case VERT_PRED_8:
		return CHROMA_VERT_PRED_8;

	case DC_PRED_8:
		{
			int mb_nr=IMGPAR current_mb_nr_d;	
			PixelPos up;        //!< pixel position  p(0,-1)
			PixelPos left[2];  //!< pixel positions p(-1, -1..7), YUV4:2:0 restriction
			int up_avail, left_avail[2], left_up_avail;

			getCurrNeighbourD_Chroma ARGS3(-1, -1, &left[0]);

			getCurrNeighbourA_Chroma ARGS3(-1, 0, &left[1]);
			if(left[1].pMB)
			{				
				left_avail[0] = left_avail[1]=1;

				if (active_pps.constrained_intra_pred_flag)
				{
					if (IMGPAR MbaffFrameFlag)
					{
						switch(currMB_d->mbStatusA)
						{
						case 1:					
						case 2:
							left_avail[0] = left_avail[1] = (currMB_d->mbIntraA==0x03);					
							break;
						case 3:					
						case 4:
							left_avail[0] = (currMB_d->mbIntraA&1);
							left_avail[1] = (currMB_d->mbIntraA>>1);
							break;				
						default:						
							break;
						}								
					}					
				}
			}
			else
			{
				left_avail[0] = left_avail[1]=0;
			}

			getCurrNeighbourB_Chroma ARGS3(0, -1, &up);			

			up_avail      = (up.pMB!=NULL);				
			left_up_avail = (left[0].pMB!=NULL);			


			//===== get prediction value =====
			if(up_avail)
			{
				if (!left_avail[0] && !left_avail[1])
					return CHROMA_DC_PRED_8_UP;			
				else
					return CHROMA_DC_PRED_8_UP_LF;
			}
			else
			{
				if (!left_avail[0] && !left_avail[1])
					return CHROMA_DC_PRED_8_NONE;
				else
					return CHROMA_DC_PRED_8_LF;
			}
		}

	default:
		return 0;
	}
}

