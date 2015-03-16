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

#include "global.h"
#include "h264dxva1.h"
#include "image.h"
#include "mb_access.h"
#include <mmintrin.h>
#include <xmmintrin.h>
#include <emmintrin.h>

#include "loopfilter.h"

#include "amddecext.h"

#define CQPOF(qp, uv) (__fast_iclip0_X(51, qp + p->chroma_qp_offset[uv]))
extern byte QP_SCALE_CR[52] ;

extern byte ALPHA_TABLE[52];
extern byte BETA_TABLE[52];
extern byte CLIP_TAB[52][5];
extern int STRENGTH4[5]; 
extern byte blkQ_0[16];
extern byte blkP_[4][16];
extern byte lp_mb_offset[3][16];

extern unsigned char cbp_blk_chroma[8][4];
extern void calc_chroma_vector_adjustment PARGS2(int list_offset, int curr_mb_field);
extern CREL_RETURN record_reference_picIds PARGS2(int list_offset, Macroblock_s *currMB_s);
extern CREL_RETURN reorder_lists PARGS2(int currSliceType, Slice * currSlice);
extern CREL_RETURN check_lists PARGS0();

extern void iDCT_4x4_2_ATI(short* dest, short *src, short dest_stride);

extern void mem16_2( LPBYTE d1, LPBYTE d2, const BYTE* s, int _size );
extern void mem8_2( LPBYTE dUV, LPBYTE dU, LPBYTE dV, const BYTE* sU, const BYTE* sV, int _size );

extern BOOL	m_bLast_Slice;

extern void UpdatePTS PARGS0();

#define use_tmp
#define Alignment16(a) ((int)((a+15)>>4)<<4)

CH264DXVA1::CH264DXVA1()
{
	stream_global = NULL;

	InitializeCriticalSection( &crit_PICDEC );
	InitializeCriticalSection( &crit_COMP );
	InitializeCriticalSection( &crit_EXECUTE );
	InitializeCriticalSection( &crit_GetCompBuf );

	memset(m_pAMVACompBufInfo, 0, sizeof(AMVACompBufferInfo)*23);
	m_dwNumTypesCompBuffers = sizeof(m_pAMVACompBufInfo)/sizeof(m_pAMVACompBufInfo[0]);
	m_bNotGetCompBuf = TRUE;
	m_bDeblockingFlag = FALSE;

}

CH264DXVA1::~CH264DXVA1()
{
	DeleteCriticalSection( &crit_COMP );
	DeleteCriticalSection( &crit_PICDEC );
	DeleteCriticalSection( &crit_EXECUTE );
	DeleteCriticalSection( &crit_GetCompBuf );

}

void CH264DXVA1::SetIHVDService(IHVDService *pIHVDService, H264VDecHP_OpenOptionsEx *pOptions)
{
	pIHVDService->QueryInterface(__uuidof(IHVDServiceDxva1), (void**)&m_pIHVDServiceDxva1);
	return CH264DXVABase::SetIHVDService(pIHVDService, pOptions);
}

int CH264DXVA1::Open PARGS1(int nSurfaceFrame)
{
	stream_global = IMGPAR stream_global;

	CH264DXVABase::Open ARGS1(nSurfaceFrame);

	EnterCriticalSection(&m_cs);
	DXVA_ConfigPictureDecode t_sConfigPictureDecode;
	ZeroMemory(&t_sConfigPictureDecode, sizeof(DXVA_ConfigPictureDecode));

	if (m_nDXVAMode == E_H264_DXVA_MODE_E || m_nDXVAMode == E_H264_DXVA_MODE_F) //VLD mode
	{
		//We must notify driver first that we want to use short slice format.
		t_sConfigPictureDecode.bConfigBitstreamRaw = E_BA_RAW_SHORTFORMAT;
	}

	t_sConfigPictureDecode.dwFunction = (DXVA_QUERYORREPLYFUNCFLAG_DECODER_PROBE_QUERY<<8) | 1;
	Execute(t_sConfigPictureDecode.dwFunction, &t_sConfigPictureDecode, sizeof(DXVA_ConfigPictureDecode), &t_sConfigPictureDecode, sizeof(DXVA_ConfigPictureDecode), 0, NULL);

	t_sConfigPictureDecode.guidConfigBitstreamEncryption = m_guidConfigBitstreamEncryption;

	t_sConfigPictureDecode.dwFunction = (DXVA_QUERYORREPLYFUNCFLAG_DECODER_LOCK_QUERY<<8) | 1;
	Execute(t_sConfigPictureDecode.dwFunction, &t_sConfigPictureDecode, sizeof(DXVA_ConfigPictureDecode), &t_sConfigPictureDecode, sizeof(DXVA_ConfigPictureDecode), 0, NULL);

	m_bConfigBitstreamRaw = (int)t_sConfigPictureDecode.bConfigBitstreamRaw;

	LeaveCriticalSection(&m_cs);
	return S_OK;
}

int CH264DXVA1::Close PARGS0()
{	
	if (m_pIHVDServiceDxva1)
		m_pIHVDServiceDxva1->Release();
	return CH264DXVABase::Close ARGS0();
}

HRESULT CH264DXVA1::GetDXVABuffers()
{
	HRESULT hr = S_OK;
	unsigned int i;

 	if(m_bNotGetCompBuf)
	{
		hr = m_pIHVDServiceDxva1->GetInternalCompBufferInfo(&m_dwNumTypesCompBuffers, m_pAMVACompBufInfo);
		if(SUCCEEDED(hr))
		{
			if(m_bConfigBitstreamRaw)
			{
				m_dwNumCompBuffers = __min(MAX_COMP_BUF, __min(m_pAMVACompBufInfo[DXVA_PICTURE_DECODE_BUFFER].dwNumCompBuffers, m_pAMVACompBufInfo[DXVA_INVERSE_QUANTIZATION_MATRIX_BUFFER].dwNumCompBuffers));
				m_dwNumCompBuffers = __min(m_dwNumCompBuffers, __min(m_pAMVACompBufInfo[DXVA_SLICE_CONTROL_BUFFER].dwNumCompBuffers, m_pAMVACompBufInfo[DXVA_BITSTREAM_DATA_BUFFER].dwNumCompBuffers));

				for(i=0; i<(int)m_dwNumCompBuffers; i++)
					m_bCompBufStaus[i] = TRUE;
			}

			m_bNotGetCompBuf = FALSE;
		}
	}

	return hr;
}

void CH264DXVA1::ReleaseDXVABuffers()
{
	unsigned int i;

	if(!m_bNotGetCompBuf)
	{
		if(m_bConfigBitstreamRaw)
		{
			for(i=0; i<(int)m_dwNumCompBuffers; i++)
			{
				m_pbPictureParamBuf[i] = NULL;
				m_pbBitstreamBuf[i] = NULL;
				m_pbSliceCtrlBuf[i] = NULL;
				m_pbInverseQuantMatrixBuf[i] = NULL;	
			}
		}

		m_bNotGetCompBuf=TRUE;
	}
}

void CH264DXVA1::ResetDXVABuffers()
{
	CH264DXVABase::ResetDXVABuffers();

	ReleaseDXVABuffers();
	if (m_bConfigBitstreamRaw)
	{
		DXVA_ConfigPictureDecode t_sConfigPictureDecode;
		ZeroMemory(&t_sConfigPictureDecode, sizeof(DXVA_ConfigPictureDecode));

		t_sConfigPictureDecode.bConfigBitstreamRaw = E_BA_RAW_SHORTFORMAT;

		t_sConfigPictureDecode.dwFunction = (DXVA_QUERYORREPLYFUNCFLAG_DECODER_PROBE_QUERY<<8) | 1;
		Execute(t_sConfigPictureDecode.dwFunction, &t_sConfigPictureDecode, sizeof(DXVA_ConfigPictureDecode), &t_sConfigPictureDecode, sizeof(DXVA_ConfigPictureDecode), 0, NULL);

		t_sConfigPictureDecode.guidConfigBitstreamEncryption = m_guidConfigBitstreamEncryption;

		t_sConfigPictureDecode.dwFunction = (DXVA_QUERYORREPLYFUNCFLAG_DECODER_LOCK_QUERY<<8) | 1;
		Execute(t_sConfigPictureDecode.dwFunction, &t_sConfigPictureDecode, sizeof(DXVA_ConfigPictureDecode), &t_sConfigPictureDecode, sizeof(DXVA_ConfigPictureDecode), 0, NULL);

		m_bConfigBitstreamRaw = (int)t_sConfigPictureDecode.bConfigBitstreamRaw;
	}
	GetDXVABuffers();
}

void CH264DXVA1::BA_ResetLastCompBufStaus()
{
	DEBUG_SHOW_HW_INFO("[CH264DXVA1] BA_ResetLastCompBufStaus");
	img_par *img = stream_global->m_img[0];

	AMVABUFFERINFO *t_psBufferInfo = IMGPAR m_pBufferInfo;
	for(UINT i=0;i<4;i++)
	{
		if(FAILED(ReleaseBuffer(t_psBufferInfo[i].dwTypeIndex, t_psBufferInfo[i].dwBufferIndex)))
			break;
	}
	m_bCompBufStaus[m_nLastCompBufIdx]=TRUE;
}

HRESULT CH264DXVA1::GetBuffer(DWORD dwBufferType, DWORD dwBufferIndex, BOOL bReadOnly, LPVOID *ppBuffer, LONG *pBufferStride)
{
	HRESULT hr = S_OK;

	if(!m_pIHVDServiceDxva1)
		return E_POINTER;

	HVDService::HVDDxvaCompBufLockInfo HVDDxvaCompInfo;
	ZeroMemory(&HVDDxvaCompInfo, sizeof(HVDService::HVDDxvaCompBufLockInfo));
	hr = m_pIHVDServiceDxva1->LockCompressBuffer(dwBufferType, dwBufferIndex, &HVDDxvaCompInfo, TRUE);
	if (SUCCEEDED(hr))
	{
		if (ppBuffer)
			(*ppBuffer) = HVDDxvaCompInfo.pBuffer;
		if (pBufferStride)
			(*pBufferStride) = HVDDxvaCompInfo.lStride;
	}

	return hr;
}

HRESULT CH264DXVA1::ReleaseBuffer(DWORD dwBufferType, DWORD dwBufferIndex)
{
	HRESULT hr = S_OK;

	if(!m_pIHVDServiceDxva1)
		return E_POINTER;

	hr = m_pIHVDServiceDxva1->UnlockCompressBuffer(dwBufferType, dwBufferIndex);

	return hr;
}

HRESULT CH264DXVA1::BeginFrame(DWORD dwDestSurfaceIndex, DWORD dwFlags)
{
	HRESULT hr = S_OK;

	while(1)
	{
		hr = m_pIHVDServiceDxva1->BeginFrame(dwDestSurfaceIndex);
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

HRESULT CH264DXVA1::Execute(DWORD dwFunction, LPVOID lpPrivateInputData, DWORD cbPrivateInputData, LPVOID lpPrivateOutputData, DWORD cbPrivateOutputData, DWORD dwNumBuffers, AMVABUFFERINFO *pamvaBufferInfo)
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

	return m_pIHVDServiceDxva1->Execute(&HVDDxvaExecute);
}

HRESULT CH264DXVA1::EndFrame(DWORD dwDestSurfaceIndex)
{
	return m_pIHVDServiceDxva1->EndFrame();
}

CREL_RETURN CH264DXVA1::decode_one_picture_short PARGS2(int* header, BOOL bSkip)
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

		// Frame management: This is the part we have to reset pic_combine_status
		//IMGPAR idr_flag = (nalu->nal_unit_type == NALU_TYPE_IDR);
		if(nalu->nal_unit_type == NALU_TYPE_IDR)
			IMGPAR idr_flag = newSlice->idr_flag = 1;
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

		if (stream_global->bSeekToOpenGOP)
		{
			if (IMGPAR currentSlice->picture_type != B_SLICE)
			{
				stream_global->bSeekToOpenGOP = 0;
				next_image_type = IMGPAR type;
				bSkip = 0; //reset skip flag
			}
			else
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

		seq_parameter_set_rbsp_t *sps;
		if(g_bRewindDecoder == FALSE)
		{
			sps =  (IMGPAR currentSlice->bIsBaseView) ? &SeqParSet[PicParSet[IMGPAR currentSlice->pic_parameter_set_id].seq_parameter_set_id] :
				&SeqParSubset[PicParSet[IMGPAR currentSlice->pic_parameter_set_id].seq_parameter_set_id];
			activate_sps ARGS2(sps, 0);

			if(newSlice!=NULL && IMGPAR Hybrid_Decoding == 5 
				&& (IMGPAR structure != FRAME || IMGPAR currentSlice->MbaffFrameFlag))
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
				IMGPAR m_Intra_lCompBufIndex = NULL;
				m_lpBADATA = NULL;
				m_lpBADATACTRL = NULL;

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

			//Does Nvidia need this or not?
			if(stream_global->m_bTrickEOS)
			{
				nalu_global_available = 0;
				*header = EOS;
				return CREL_OK;
			}
			break;
		}

		if(is_new_picture ARGS0())
		{
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
						BA_DATA_CTRL = (DXVA_Slice_H264_Short *)m_lpBADATACTRL;
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
					IMGPAR m_slice_number_in_field = 1;

					IMGPAR currentSlice->m_nDispPicStructure = sps->vui_seq_parameters.picture_structure;

					DEBUG_SHOW_HW_INFO("##bSkip: %d##", bSkip);

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
						BA_DATA_CTRL = (DXVA_Slice_H264_Short *)m_lpBADATACTRL;					
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
					IMGPAR m_slice_number_in_field = 1;

					//store these parameters to next collect_pic					
					stream_global->PreviousFrameNum[0]				= IMGPAR PreviousFrameNum;
					stream_global->PreviousFrameNumOffset[0]	= IMGPAR PreviousFrameNumOffset;
					stream_global->PreviousPOC[0]						= IMGPAR PreviousPOC;
					stream_global->ThisPOC[0]								= IMGPAR ThisPOC; 
					stream_global->PrevPicOrderCntLsb[0]			= IMGPAR PrevPicOrderCntLsb;
					stream_global->PrevPicOrderCntMsb[0]			= IMGPAR PrevPicOrderCntMsb;	

					IMGPAR currentSlice->m_nDispPicStructure = sps->vui_seq_parameters.picture_structure;

					DEBUG_SHOW_HW_INFO("##bSkip: %d##", bSkip);

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
						BA_DATA_CTRL = (DXVA_Slice_H264_Short *)m_lpBADATACTRL;

						g_bReceivedFirst = (!g_bReceivedFirst) ? 1 : 2;
					}
				}
			}												
		}
		else
		{   //Slice
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

			BA_DATA_CTRL = (DXVA_Slice_H264_Short *)m_lpBADATACTRL;
		}

		if(!bSkip)
		{
			nalu_size = nalu->len+3+m_nNALUSkipByte;

			BA_DATA_CTRL->BSNALunitDataLocation = (int)(m_lpBADATA - m_pbBitstreamBuf[IMGPAR m_Intra_lCompBufIndex]);

			//ATi driver need to align to 128 (0x80).
			shiftoffset = (nalu_size & 127);
			if(shiftoffset)
			{
				memset((m_lpnalu+nalu_size), 0,(128-shiftoffset));
				nalu_size += (128-shiftoffset);
			}
			memcpy(m_lpBADATA, m_lpnalu, nalu_size);
			//DUMP_NVIDIA(m_lpnalu, nalu_size, 1, "BITSTREAMDATA-bxva1");
			//DUMP_NVIDIA(m_lpBADATA, nalu_size, 2, "BITSTREAMDATA-bxva1");
			m_lpBADATA += nalu_size;
			BA_DATA_CTRL->SliceBytesInBuffer = nalu_size;
			BA_DATA_CTRL->wBadSliceChopping = 0;
			m_lpBADATACTRL += sizeof(DXVA_Slice_H264_Short);
			BA_DATA_CTRL = (DXVA_Slice_H264_Short *)m_lpBADATACTRL;
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

CREL_RETURN CH264DXVA1::decode_one_picture_long PARGS2(int* header, BOOL bSkip) 
{
	return CREL_OK;
}

int CH264DXVA1::BA_build_picture_decode_buffer PARGS0()
{
	DEBUG_SHOW_HW_INFO("[CH264DXVA1] BA_build_picture_decode_buffer");
	int i, j;
	StreamParameters *stream_global = IMGPAR stream_global;
	//pic_parameter_set_rbsp_t *pps = IMGPAR currentSlice->m_active_pps;
	pic_parameter_set_rbsp_t *pps = &active_pps;
	seq_parameter_set_rbsp_t *sps = &active_sps;
	if(!active_sps.Valid)
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

	IMGPAR m_Intra_lCompBufIndex = BA_GetCompBufferIndex();
	if (IMGPAR m_Intra_lCompBufIndex < 0)
	{
		stream_global->m_iStop_Decode = 1;
		return -1;
	}

	if(IMGPAR UnCompress_Idx == -1)
		IMGPAR UnCompress_Idx = (int)m_pUncompBufQueue->GetItem();

	IMGPAR m_lFrame_Counter = m_nFrameCounter++;

	DXVA_BufferDescription *m_pDxvaBufferDescription = IMGPAR m_pDxvaBufferDescription;
	AMVABUFFERINFO         *m_pBufferInfo            = IMGPAR m_pBufferInfo;
	DXVA_PicParams_H264* m_pPictureDecode = (DXVA_PicParams_H264*)m_pbPictureParamBuf[IMGPAR m_Intra_lCompBufIndex];
	DXVA_Qmatrix_H264 *m_pQpMatrix = (DXVA_Qmatrix_H264*)m_pbInverseQuantMatrixBuf[IMGPAR m_Intra_lCompBufIndex];
	memset(m_pPictureDecode, 0, sizeof(DXVA_PicParams_H264));

	m_pPictureDecode->CurrPic.Index7Bits		= IMGPAR UnCompress_Idx;
	//m_pPictureDecode->CurrPic.AssociatedFlag = (IMGPAR currentSlice->structure != 0);
	m_pPictureDecode->CurrPic.AssociatedFlag = (IMGPAR field_pic_flag & (IMGPAR currentSlice->structure >> 1));
	m_pPictureDecode->wFrameWidthInMbsMinus1		= sps->pic_width_in_mbs_minus1;
	m_pPictureDecode->wFrameHeightInMbsMinus1		= IMGPAR FrameHeightInMbs-1;

	m_pPictureDecode->num_ref_frames = sps->num_ref_frames;

	m_pPictureDecode->field_pic_flag = IMGPAR field_pic_flag;   //field_pic_flag
	m_pPictureDecode->MbaffFrameFlag = IMGPAR currentSlice->MbaffFrameFlag;  //MbaffFrameFlag
	m_pPictureDecode->residual_colour_transform_flag = 0;  //residual_colour_transform_flag
	m_pPictureDecode->sp_for_switch_flag =  0;  //sp_for_switch_flag
	m_pPictureDecode->chroma_format_idc = sps->chroma_format_idc;  //chroma_format_idc
	//m_pPictureDecode->RefPicFlag = (IMGPAR currentSlice->picture_type != B_SLICE);//IMGPAR currentSlice->nal_reference_idc;  //RefPicFlag //hard coded for ATi
	m_pPictureDecode->RefPicFlag = dec_picture->used_for_reference;
	m_pPictureDecode->constrained_intra_pred_flag = pps->constrained_intra_pred_flag; //constrained_intra_pred_flag
	m_pPictureDecode->weighted_pred_flag = pps->weighted_pred_flag;  //weighted_pred_flag
	m_pPictureDecode->weighted_bipred_idc = pps->weighted_bipred_idc;  //weighted_bipred_idc
	m_pPictureDecode->MbsConsecutiveFlag = 1; //restricted mode profile is 0. //MbsConsecutiveFlag
	m_pPictureDecode->frame_mbs_only_flag = sps->frame_mbs_only_flag;  //frame_mbs_only_flag
	m_pPictureDecode->transform_8x8_mode_flag = pps->transform_8x8_mode_flag;  //transform_8x8_mode_flag
	m_pPictureDecode->MinLumaBipredSize8x8Flag = 1; //set to 1 for Main,High,High 10,High 422,or High 444 of levels3.1 and higher.
	m_pPictureDecode->IntraPicFlag = (IMGPAR currentSlice->picture_type == I_SLICE);  //IntraPicFlag

	m_pPictureDecode->bit_depth_luma_minus8 = sps->bit_depth_luma_minus8;
	m_pPictureDecode->bit_depth_chroma_minus8 = sps->bit_depth_chroma_minus8;
	m_pPictureDecode->StatusReportFeedbackNumber = 0;

	if(m_nVGAType==E_H264_VGACARD_NVIDIA)
	{
		if(m_pPictureDecode->field_pic_flag)
		{
			if(m_pPictureDecode->CurrPic.AssociatedFlag)
			{
				m_RefInfo[IMGPAR UnCompress_Idx].top_poc = m_pPictureDecode->CurrFieldOrderCnt[0] = dec_picture->top_poc = 0;
				m_RefInfo[IMGPAR UnCompress_Idx].bot_poc = m_pPictureDecode->CurrFieldOrderCnt[1] = dec_picture->bottom_poc = IMGPAR bottompoc;
			}
			else
			{	
				m_RefInfo[IMGPAR UnCompress_Idx].top_poc = m_pPictureDecode->CurrFieldOrderCnt[0] = dec_picture->top_poc = IMGPAR toppoc;
				m_RefInfo[IMGPAR UnCompress_Idx].bot_poc = m_pPictureDecode->CurrFieldOrderCnt[1] = dec_picture->bottom_poc = 0;
			}
		}
		else
		{
			m_RefInfo[IMGPAR UnCompress_Idx].top_poc = m_pPictureDecode->CurrFieldOrderCnt[0] = dec_picture->top_poc = IMGPAR toppoc;
			m_RefInfo[IMGPAR UnCompress_Idx].bot_poc = m_pPictureDecode->CurrFieldOrderCnt[1] = dec_picture->bottom_poc = IMGPAR bottompoc;
		}
	} 
	else if (m_nVGAType==E_H264_VGACARD_ATI)
	{
		if(m_pPictureDecode->field_pic_flag)
		{
			if(m_pPictureDecode->CurrPic.AssociatedFlag)
			{
				m_RefInfo[IMGPAR UnCompress_Idx].top_poc = m_pPictureDecode->CurrFieldOrderCnt[0] = 0;
				m_RefInfo[IMGPAR UnCompress_Idx].bot_poc = m_pPictureDecode->CurrFieldOrderCnt[1] = IMGPAR bottompoc;
			}
			else
			{	
				m_RefInfo[IMGPAR UnCompress_Idx].top_poc = m_pPictureDecode->CurrFieldOrderCnt[0] = IMGPAR toppoc;
				m_RefInfo[IMGPAR UnCompress_Idx].bot_poc = m_pPictureDecode->CurrFieldOrderCnt[1] = 0;
			}
		}
		else
		{
			m_RefInfo[IMGPAR UnCompress_Idx].top_poc = m_pPictureDecode->CurrFieldOrderCnt[0] = IMGPAR toppoc;
			m_RefInfo[IMGPAR UnCompress_Idx].bot_poc = m_pPictureDecode->CurrFieldOrderCnt[1] = IMGPAR bottompoc;
		}
	}
	else if(m_nVGAType==E_H264_VGACARD_S3)
	{
		if(m_pPictureDecode->field_pic_flag)
		{
			if(m_pPictureDecode->CurrPic.AssociatedFlag)
			{
				m_RefInfo[IMGPAR UnCompress_Idx].top_poc = m_pPictureDecode->CurrFieldOrderCnt[0] = dec_picture->top_poc = 0;
				m_RefInfo[IMGPAR UnCompress_Idx].bot_poc = m_pPictureDecode->CurrFieldOrderCnt[1] = dec_picture->bottom_poc = IMGPAR bottompoc;
			}
			else
			{	
				m_RefInfo[IMGPAR UnCompress_Idx].top_poc = m_pPictureDecode->CurrFieldOrderCnt[0] = dec_picture->top_poc = IMGPAR toppoc;
				m_RefInfo[IMGPAR UnCompress_Idx].bot_poc = m_pPictureDecode->CurrFieldOrderCnt[1] = dec_picture->bottom_poc = 0;
			}
		}
		else
		{
			m_RefInfo[IMGPAR UnCompress_Idx].top_poc = m_pPictureDecode->CurrFieldOrderCnt[0] = dec_picture->top_poc = IMGPAR toppoc;
			m_RefInfo[IMGPAR UnCompress_Idx].bot_poc = m_pPictureDecode->CurrFieldOrderCnt[1] = dec_picture->bottom_poc = IMGPAR bottompoc;
		}
	} 

	DEBUG_SHOW_HW_INFO("poc:%d", m_pPictureDecode->CurrFieldOrderCnt[0]);

	m_pPictureDecode->pic_init_qs_minus26 = pps->pic_init_qs_minus26;
	m_pPictureDecode->chroma_qp_index_offset = pps->chroma_qp_index_offset;
	m_pPictureDecode->second_chroma_qp_index_offset = pps->second_chroma_qp_index_offset;
	m_pPictureDecode->ContinuationFlag = 1;
	m_pPictureDecode->pic_init_qp_minus26 = pps->pic_init_qp_minus26;
	m_pPictureDecode->num_ref_idx_l0_active_minus1 = pps->num_ref_idx_l0_active_minus1;
	m_pPictureDecode->num_ref_idx_l1_active_minus1 = pps->num_ref_idx_l1_active_minus1;
	m_pPictureDecode->UsedForReferenceFlags = 0;
	m_pPictureDecode->NonExistingFrameFlags = 0;
	m_RefInfo[IMGPAR UnCompress_Idx].frame_num = m_pPictureDecode->frame_num = IMGPAR currentSlice->frame_num;
	m_pPictureDecode->log2_max_frame_num_minus4 = sps->log2_max_frame_num_minus4;
	m_pPictureDecode->pic_order_cnt_type = sps->pic_order_cnt_type;
	m_pPictureDecode->log2_max_pic_order_cnt_lsb_minus4 = sps->log2_max_pic_order_cnt_lsb_minus4;
	m_pPictureDecode->delta_pic_order_always_zero_flag = sps->delta_pic_order_always_zero_flag;
	m_pPictureDecode->direct_8x8_inference_flag = sps->direct_8x8_inference_flag;
	m_pPictureDecode->entropy_coding_mode_flag = pps->entropy_coding_mode_flag;
	m_pPictureDecode->pic_order_present_flag = pps->pic_order_present_flag;
	m_pPictureDecode->num_slice_groups_minus1 = pps->num_slice_groups_minus1;
	m_pPictureDecode->slice_group_map_type = pps->slice_group_map_type;
	m_pPictureDecode->deblocking_filter_control_present_flag = pps->deblocking_filter_control_present_flag;
	m_pPictureDecode->redundant_pic_cnt_present_flag = pps->redundant_pic_cnt_present_flag;
	m_pPictureDecode->slice_group_change_rate_minus1 = pps->slice_group_change_rate_minus1;
	//m_pPictureDecode->SliceGroupMap =;

	memset(m_pPictureDecode->RefFrameList,255,16);

	if(m_nVGAType==E_H264_VGACARD_NVIDIA)
	{
		int dbp_loop_size = min(dpb.used_size_on_view[0], active_sps.num_ref_frames);
		StorablePicture *RefFrame;
		for(j=0, i=0; i<dpb.used_size_on_view[0]; i++)
		{
			if(dpb.fs_on_view[0][i]->is_reference)
			{
				if(dpb.fs_on_view[0][i]->is_reference == (TOP_FIELD|BOTTOM_FIELD))
					RefFrame = dpb.fs_on_view[0][i]->frame;
				else
				{
					if(dpb.fs_on_view[0][i]->is_reference == TOP_FIELD)
						RefFrame = dpb.fs_on_view[0][i]->top_field;
					else
						RefFrame = dpb.fs_on_view[0][i]->bottom_field;
				}
				m_pPictureDecode->RefFrameList[j].Index7Bits	= RefFrame->pic_store_idx;	//*(byte*)&m_pSurfaceinfo[listX[0][i]->unique_id].SInfo[0];
				m_pPictureDecode->RefFrameList[j].AssociatedFlag = 0;

				m_pPictureDecode->FieldOrderCntList[j][0] = RefFrame->top_poc;
				m_pPictureDecode->FieldOrderCntList[j][1] = RefFrame->bottom_poc;
				m_pPictureDecode->FrameNumList[j] = RefFrame->frame_num;
				m_pPictureDecode->UsedForReferenceFlags |= (dpb.fs_on_view[0][i]->is_reference << (j*2));

				j++;
			}
		}
		//copy QP matrix
		for(i = 0; i<6; i++)
			for(j = 0; j<16; j++)
				m_pQpMatrix->bScalingLists4x4[i][j] = qmatrix[i][SNGL_SCAN_2D[j]];
		for(i = 6; i<8; i++)
			for(j = 0; j<64; j++)
				m_pQpMatrix->bScalingLists8x8[i-6][j] = qmatrix[i][SNGL_SCAN8x8_2D[j]];
	} 
	else if (m_nVGAType==E_H264_VGACARD_ATI)
	{
		m_pPictureDecode->Reserved8BitsA = 0;
		amddecext_h264setlevel(sps->level_idc,m_pPictureDecode);
		if (sps->profile_idc == 100)
			amddecext_h264setprofile(HIGH,m_pPictureDecode);		
		else if(sps->profile_idc == 77)
			amddecext_h264setprofile(MAIN,m_pPictureDecode);		
		else if(sps->profile_idc == 66)
			amddecext_h264setprofile(BASELINE,m_pPictureDecode);		
		else
			amddecext_h264setprofile(_TOO_BIG_,m_pPictureDecode);		

		m_pPictureDecode->Reserved8BitsB = 0;
		amddecext_h264setextsupport(1,m_pPictureDecode);
		if (sps->gaps_in_frame_num_value_allowed_flag)
			amddecext_h264setgap(1,m_pPictureDecode);
		if (!g_bReceivedFirst)
			amddecext_h264setseek(1,m_pPictureDecode);

		if(IMGPAR currentSlice->picture_type == P_SLICE)
		{
			for(i = 0; i < listXsize[0]; i++)
			{
				m_pPictureDecode->RefFrameList[i].Index7Bits		= *(byte*)&m_pSurfaceInfo[listX[0][i]->unique_id].SInfo[0];
				m_pPictureDecode->RefFrameList[i].AssociatedFlag = 0;
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
				//if(m_pSurfaceinfo[listX[0][i]->unique_id].SInfo[1])	//Skip Bottom filed
				//	continue;
				m_pPictureDecode->RefFrameList[i].Index7Bits = *(byte*)&m_pSurfaceInfo[listX[0][i]->unique_id].SInfo[0];
				m_pPictureDecode->RefFrameList[i].AssociatedFlag = 0;
			}
			for(j = 0; j < listXsize[1]; j++)
			{
				//if(m_pSurfaceinfo[listX[1][j]->unique_id].SInfo[1])	//Skip Bottom filed
				//	continue;
				for(i = 0; i < listXsize[0]; i++)
				{
					if(listX[0][i]->unique_id == listX[1][j]->unique_id)
					{
						IMGPAR m_L0L1Map[j] = i;
						break;
					}
				}
				if(i == listXsize[0])	//not found matching index
				{
					m_pPictureDecode->RefFrameList[listXsize[0]].Index7Bits = *(byte*)&m_pSurfaceInfo[listX[1][j]->unique_id].SInfo[0];
					m_pPictureDecode->RefFrameList[listXsize[0]].AssociatedFlag = 0;
					IMGPAR m_L0L1Map[j] = listXsize[0];
				}
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

		for(i=0 ; i<16 ; i++)
		{
			if(m_pPictureDecode->RefFrameList[i].bPicEntry != 0xFF)
			{
				m_pPictureDecode->FieldOrderCntList[i][0] = m_RefInfo[m_pPictureDecode->RefFrameList[i].bPicEntry].top_poc;
				m_pPictureDecode->FieldOrderCntList[i][1] = m_RefInfo[m_pPictureDecode->RefFrameList[i].bPicEntry].bot_poc;
				m_pPictureDecode->FrameNumList[i] = m_RefInfo[m_pPictureDecode->RefFrameList[i].bPicEntry].frame_num;
			}
		}


		//copy QP matrix
		for(i = 0; i<6; i++)
			memcpy(&m_pQpMatrix->bScalingLists4x4[i][0], qmatrix[i], 16);
		for(i = 6; i<8; i++)
			memcpy(&m_pQpMatrix->bScalingLists8x8[i-6][0], qmatrix[i], 64);
	}
	else if(m_nVGAType==E_H264_VGACARD_S3)
	{
		int dbp_loop_size = min(dpb.used_size_on_view[0], active_sps.num_ref_frames);
		StorablePicture *RefFrame;
		for(j=0, i=0; i<dpb.used_size_on_view[0]; i++)
		{
			if(dpb.fs_on_view[0][i]->is_reference)
			{
				if(dpb.fs_on_view[0][i]->is_reference == (TOP_FIELD|BOTTOM_FIELD))
					RefFrame = dpb.fs_on_view[0][i]->frame;
				else
				{
					if(dpb.fs_on_view[0][i]->is_reference == TOP_FIELD)
						RefFrame = dpb.fs_on_view[0][i]->top_field;
					else
						RefFrame = dpb.fs_on_view[0][i]->bottom_field;
				}
				m_pPictureDecode->RefFrameList[j].Index7Bits	= RefFrame->pic_store_idx;	//*(byte*)&m_pSurfaceinfo[listX[0][i]->unique_id].SInfo[0];
				m_pPictureDecode->RefFrameList[j].AssociatedFlag = 0;

				m_pPictureDecode->FieldOrderCntList[j][0] = RefFrame->top_poc;
				m_pPictureDecode->FieldOrderCntList[j][1] = RefFrame->bottom_poc;
				m_pPictureDecode->FrameNumList[j] = RefFrame->frame_num;
				m_pPictureDecode->UsedForReferenceFlags |= (dpb.fs_on_view[0][i]->is_reference << (j*2));

				j++;
			}
		}
		//copy QP matrix
		for(i = 0; i<6; i++)
			for(j = 0; j<16; j++)
				m_pQpMatrix->bScalingLists4x4[i][j] = qmatrix[i][SNGL_SCAN_2D[j]];
		for(i = 6; i<8; i++)
			for(j = 0; j<64; j++)
				m_pQpMatrix->bScalingLists8x8[i-6][j] = qmatrix[i][SNGL_SCAN8x8_2D[j]];
	} 

	SetSurfaceInfo ARGS3(dec_picture->unique_id,IMGPAR currentSlice->structure ==2?1:0,1);

	//fill out buffer description table for DXVA_PICTURE_DECODE_BUFFER
	memset(&m_pDxvaBufferDescription[0], 0, 4*sizeof(DXVA_BufferDescription));
	m_pDxvaBufferDescription[0].dwTypeIndex		= m_pBufferInfo[0].dwTypeIndex	= DXVA_PICTURE_DECODE_BUFFER;
	m_pDxvaBufferDescription[0].dwBufferIndex	= m_pBufferInfo[0].dwBufferIndex = IMGPAR m_Intra_lCompBufIndex;
	m_pDxvaBufferDescription[0].dwDataOffset	= m_pBufferInfo[0].dwDataOffset		= 0;
	m_pDxvaBufferDescription[0].dwDataSize		= m_pBufferInfo[0].dwDataSize	= sizeof(DXVA_PicParams_H264);
	m_pDxvaBufferDescription[0].dwNumMBsInBuffer	= 0;

	//fill out buffer description table for DXVA_INVERSE_QUANTIZATION_MATRIX_BUFFER
	m_pDxvaBufferDescription[1].dwTypeIndex		= m_pBufferInfo[1].dwTypeIndex	= DXVA_INVERSE_QUANTIZATION_MATRIX_BUFFER;
	m_pDxvaBufferDescription[1].dwBufferIndex	= m_pBufferInfo[1].dwBufferIndex = IMGPAR m_Intra_lCompBufIndex;
	m_pDxvaBufferDescription[1].dwDataOffset	= m_pBufferInfo[1].dwDataOffset		= 0;
	m_pDxvaBufferDescription[1].dwDataSize		= m_pBufferInfo[1].dwDataSize	= sizeof(DXVA_Qmatrix_H264);
	m_pDxvaBufferDescription[1].dwNumMBsInBuffer	= 0;

	return 0;
}

CREL_RETURN CH264DXVA1::BA_ExecuteBuffers PARGS0()
{
	DEBUG_SHOW_HW_INFO("[CH264DXVA1] BA_ExecuteBuffers");
	DWORD dwRetValue;
	HRESULT	hr = S_OK;

	if(!stream_global->m_iStop_Decode)
	{
		if(IMGPAR m_iIntraMCBufUsage != 0)
		{
			DXVA_BufferDescription *psDxvaBufferDescription = IMGPAR m_pDxvaBufferDescription;
			AMVABUFFERINFO* psBufferInfo			= IMGPAR m_pBufferInfo;
			psDxvaBufferDescription[2].dwTypeIndex		= psBufferInfo[2].dwTypeIndex	= DXVA_SLICE_CONTROL_BUFFER;
			psDxvaBufferDescription[2].dwBufferIndex	= psBufferInfo[2].dwBufferIndex = IMGPAR m_Intra_lCompBufIndex;			
			psDxvaBufferDescription[2].dwDataOffset		= psBufferInfo[2].dwDataOffset= 0;
			psDxvaBufferDescription[2].dwNumMBsInBuffer	= IMGPAR slice_number;
			if(m_bConfigBitstreamRaw==E_BA_RAW_LONGFORMAT)
				psDxvaBufferDescription[2].dwDataSize	= psBufferInfo[2].dwDataSize	= IMGPAR m_slice_number_in_field * sizeof(DXVA_Slice_H264_Long);
			else
				psDxvaBufferDescription[2].dwDataSize	= psBufferInfo[2].dwDataSize	= IMGPAR m_slice_number_in_field * sizeof(DXVA_Slice_H264_Short);

			psDxvaBufferDescription[3].dwTypeIndex		= psBufferInfo[3].dwTypeIndex	= DXVA_BITSTREAM_DATA_BUFFER;
			psDxvaBufferDescription[3].dwBufferIndex	= psBufferInfo[3].dwBufferIndex = IMGPAR m_Intra_lCompBufIndex;			
			psDxvaBufferDescription[3].dwDataOffset		= psBufferInfo[3].dwDataOffset= 0;
			psDxvaBufferDescription[3].dwNumMBsInBuffer	= IMGPAR slice_number;
			psDxvaBufferDescription[3].dwDataSize			= psBufferInfo[3].dwDataSize	= IMGPAR m_iIntraMCBufUsage;

			DUMP_NVIDIA(m_pbPictureParamBuf[IMGPAR m_Intra_lCompBufIndex], psDxvaBufferDescription[0].dwDataSize, IMGPAR m_lFrame_Counter, "PICPAR-bxva1");
			DUMP_NVIDIA(m_pbInverseQuantMatrixBuf[IMGPAR m_Intra_lCompBufIndex], psDxvaBufferDescription[1].dwDataSize, IMGPAR m_lFrame_Counter, "IQM-bxva1");
			DUMP_NVIDIA(m_pbSliceCtrlBuf[IMGPAR m_Intra_lCompBufIndex], psDxvaBufferDescription[2].dwDataSize, IMGPAR m_lFrame_Counter, "SLICECTRL-bxva1");
			DUMP_NVIDIA(m_pbBitstreamBuf[IMGPAR m_Intra_lCompBufIndex], psDxvaBufferDescription[3].dwDataSize, IMGPAR m_lFrame_Counter, "BITSTREAMDATA-bxva1");
			DUMP_NVIDIA((BYTE *)psDxvaBufferDescription, 4*sizeof(DXVA_BufferDescription), IMGPAR m_lFrame_Counter, "BUFDESC-bxva1");
			DUMP_NVIDIA((BYTE *)psBufferInfo, 4*sizeof(AMVABUFFERINFO), IMGPAR m_lFrame_Counter, "BUFINFO-bxva1");

			if(m_pIviCP != NULL)
			{			
				if(m_pIviCP->IsScramblingRequired(MAP_CP_PIC_TYPE(IMGPAR firstSlice->picture_type)))
				{
					hr = m_pIviCP->EnableScrambling();
					if(SUCCEEDED(hr))
					{					
						hr = m_pIviCP->ScrambleData(m_pbBitstreamBuf[IMGPAR m_Intra_lCompBufIndex], m_pbBitstreamBuf[IMGPAR m_Intra_lCompBufIndex], IMGPAR m_iIntraMCBufUsage);
					}
				}
				else
				{
					hr = m_pIviCP->DisableScrambling();
				}
			}

			hr = BeginFrame(IMGPAR UnCompress_Idx, 0);
			DEBUG_SHOW_HW_INFO("This is the %d Frame", IMGPAR m_lFrame_Counter);
			DEBUG_SHOW_HW_INFO("BeginFrame() return value = %d", hr);

			if(SUCCEEDED(hr))
			{
				hr |= Execute(0x01000000, psDxvaBufferDescription, 4*sizeof(DXVA_BufferDescription), &dwRetValue, 4, 4, psBufferInfo);
				DEBUG_SHOW_HW_INFO("Execute() return value = %d", hr);
				for(UINT i=0;i<4;i++)
				{
					if(FAILED(ReleaseBuffer(psBufferInfo[i].dwTypeIndex, psBufferInfo[i].dwBufferIndex)))
						return CREL_ERROR_H264_UNDEFINED;
				}
				hr |= EndFrame(IMGPAR UnCompress_Idx);
				DEBUG_SHOW_HW_INFO("EndFrame() return value = %d", hr);
			}
		}
	}

	m_bCompBufStaus[IMGPAR m_Intra_lCompBufIndex] = TRUE;

	return SUCCEEDED(hr) ? CREL_OK : CREL_ERROR_H264_UNDEFINED;
}

int CH264DXVA1::BA_GetCompBufferIndex()
{
	DEBUG_SHOW_HW_INFO("[CH264DXVA1] BA_GetCompBufferIndex");
	HRESULT hr;
	while(1)
	{
		if(++m_nLastCompBufIdx==m_dwNumCompBuffers)
			m_nLastCompBufIdx=0;

		if(m_bCompBufStaus[m_nLastCompBufIdx])
		{
			hr = m_pIHVDServiceDxva1->QueryRenderStatus(DXVA_PICTURE_DECODE_BUFFER, m_nLastCompBufIdx, 0);
			hr |= m_pIHVDServiceDxva1->QueryRenderStatus(DXVA_INVERSE_QUANTIZATION_MATRIX_BUFFER, m_nLastCompBufIdx, 0);
			hr |= m_pIHVDServiceDxva1->QueryRenderStatus(DXVA_SLICE_CONTROL_BUFFER, m_nLastCompBufIdx, 0);
			hr |= m_pIHVDServiceDxva1->QueryRenderStatus(DXVA_BITSTREAM_DATA_BUFFER, m_nLastCompBufIdx, 0);

			if(checkDDError(hr))
				Sleep(2);
			else
			{
				m_bCompBufStaus[m_nLastCompBufIdx]=false;
				break;
			}
		}
	}

	LONG lStride;
	/*hr |= */GetBuffer(DXVA_PICTURE_DECODE_BUFFER, m_nLastCompBufIdx, FALSE, (void**)&m_pbPictureParamBuf[m_nLastCompBufIdx], &lStride);
	/*hr |= */GetBuffer(DXVA_INVERSE_QUANTIZATION_MATRIX_BUFFER, m_nLastCompBufIdx, FALSE, (void**)&m_pbInverseQuantMatrixBuf[m_nLastCompBufIdx], &lStride);
	/*hr |= */GetBuffer(DXVA_SLICE_CONTROL_BUFFER, m_nLastCompBufIdx, FALSE, (void**)&m_pbSliceCtrlBuf[m_nLastCompBufIdx], &lStride);
	hr = GetBuffer(DXVA_BITSTREAM_DATA_BUFFER, m_nLastCompBufIdx, FALSE, (void**)&m_pbBitstreamBuf[m_nLastCompBufIdx], &lStride);
	m_lpBADATA = m_pbBitstreamBuf[m_nLastCompBufIdx];
	m_lpBADATACTRL = m_pbSliceCtrlBuf[m_nLastCompBufIdx];

	return ((hr==DDERR_SURFACELOST) ? -1 : m_nLastCompBufIdx);
}

//////////////////////////////////////////////////////////////////////////
// CH264DXVA1_NV Implementation
CH264DXVA1_NV::CH264DXVA1_NV()
{
	m_nFrameIndex = 0;
	m_nFirstDecBuf = 0;

	m_dwNumPicDecBuffers = 0;
	memset(m_bPicCompBufStaus, 0, sizeof(m_bPicCompBufStaus));
	m_nLastPicBufIdx = -1;

	m_bFirst_Slice = FALSE;
}

CH264DXVA1_NV::~CH264DXVA1_NV()
{

}

int CH264DXVA1_NV::Open PARGS1(int nSurfaceFrame)
{
	CH264DXVA1::Open ARGS1(nSurfaceFrame);

	build_macroblock_buffer_Inter[0] = &CH264DXVA1_NV::build_macroblock_buffer_Inter_Ori;
	build_macroblock_buffer_Inter[1] = &CH264DXVA1_NV::build_macroblock_buffer_Inter_Int;

	GetDXVABuffers();

	if (m_nDXVAMode == E_H264_DXVA_MODE_E || m_nDXVAMode == E_H264_DXVA_MODE_F) //VLD mode
		m_lpnalu = (BYTE*)_aligned_malloc((int)m_pAMVACompBufInfo[DXVA_BITSTREAM_DATA_BUFFER].dwBytesToAllocate, 16);

	return 0;
}

int CH264DXVA1_NV::Close PARGS0()
{
	ReleaseDXVABuffers();

	if (m_nDXVAMode == E_H264_DXVA_MODE_E || m_nDXVAMode == E_H264_DXVA_MODE_F) //VLD mode
	{
		_aligned_free(m_lpnalu);
		m_lpnalu = NULL;
	}

	return CH264DXVA1::Close ARGS0();
}

void CH264DXVA1_NV::ResetDXVABuffers()
{
	ReleaseDXVABuffers();
	GetDXVABuffers();
	return CH264DXVA1::ResetDXVABuffers();
}

int CH264DXVA1_NV::GetCompBufferIndex()
{
	HRESULT hr = S_OK;

	EnterCriticalSection( &crit_COMP );
	int i = m_nLastCompBufIdx+1;
	if(i == m_dwNumCompBuffers)
		i=0;
	m_nQueryCount = 0;

	DEBUG_SHOW_HW_INFO("GetCompBufferIndex : Start1");

	while(1)
	{
		if(m_bCompBufStaus[i])
			hr = m_pIHVDServiceDxva1->QueryRenderStatus(DXVA_RESIDUAL_DIFFERENCE_BUFFER,i,0);
		if(hr == S_OK)
		{
			m_bCompBufStaus[i] = FALSE;
			m_nLastCompBufIdx = i;
			LeaveCriticalSection( &crit_COMP );

			DEBUG_SHOW_HW_INFO("GetCompBufferIndex : Finish1");

			return i;
		}
		else
			m_nQueryCount++;
		if(++i== m_dwNumCompBuffers)
			i=0;
		if(m_nQueryCount == m_dwNumCompBuffers)
		{
			m_nQueryCount = 0;
			Sleep(0);
		}
	}

	return m_nLastCompBufIdx;
}

HRESULT CH264DXVA1_NV::GetDXVABuffers()
{
	HRESULT hr = S_OK;
	unsigned int i;
	long lStride;

	hr = CH264DXVA1::GetDXVABuffers();
	if (SUCCEEDED(hr))
	{
		if (m_bConfigBitstreamRaw==0)
		{
			m_dwNumCompBuffers = __min(MAX_COMP_BUF, __min(m_pAMVACompBufInfo[DXVA_MACROBLOCK_CONTROL_BUFFER].dwNumCompBuffers, m_pAMVACompBufInfo[DXVA_RESIDUAL_DIFFERENCE_BUFFER].dwNumCompBuffers));
			m_dwNumPicDecBuffers = __min(MAX_PICDEC_Buffer, m_pAMVACompBufInfo[DXVA_PICTURE_DECODE_BUFFER].dwNumCompBuffers);

			if(m_bDeblockingFlag)
			{
				m_dwNumPicDecBuffers = __min(m_dwNumPicDecBuffers, m_pAMVACompBufInfo[DXVA_DEBLOCKING_CONTROL_BUFFER].dwNumCompBuffers);
				for(i=0; i<m_dwNumPicDecBuffers ; i++)
					hr |= GetBuffer(DXVA_DEBLOCKING_CONTROL_BUFFER, i, FALSE, (void**)&m_pbDeblockingCtrlBuf[i], &lStride);
			}
			for(i=0; i<m_dwNumPicDecBuffers ; i++)
			{
				m_bPicCompBufStaus[i] = TRUE;
				hr |= GetBuffer(DXVA_PICTURE_DECODE_BUFFER, i, FALSE, (void**)&m_pbPictureParamBuf[i], &lStride);
			}
			for(i=0; i<m_dwNumCompBuffers; i++)
			{
				m_bCompBufStaus[i] = TRUE;
				hr |= GetBuffer(DXVA_MACROBLOCK_CONTROL_BUFFER, i, FALSE, (void**)&m_pbMacroblockCtrlBuf[i], &lStride);
				hr |= GetBuffer(DXVA_RESIDUAL_DIFFERENCE_BUFFER, i, FALSE, (void**)&m_pbResidualDiffBuf[i], &lStride);
			}
		}
	}

	return hr;
}

void CH264DXVA1_NV::ReleaseDXVABuffers()
{
	unsigned int i;

	CH264DXVA1::ReleaseDXVABuffers();

	if (m_bConfigBitstreamRaw==0)
	{
		for(i=0; i<m_dwNumPicDecBuffers; i++)
		{
			ReleaseBuffer(DXVA_PICTURE_DECODE_BUFFER, i);
			m_pbPictureParamBuf[i] = NULL;
			m_bPicCompBufStaus[i] = FALSE;
		}
		if(m_bDeblockingFlag)
		{
			for(i=0; i<m_dwNumPicDecBuffers; i++)
			{
				ReleaseBuffer(DXVA_DEBLOCKING_CONTROL_BUFFER, i);
				m_pbDeblockingCtrlBuf[i] = NULL;
			}
		}
		for(i=0; i<m_dwNumCompBuffers; i++)
		{
			ReleaseBuffer(DXVA_MACROBLOCK_CONTROL_BUFFER, i);
			m_pbMacroblockCtrlBuf[i] = NULL;
			ReleaseBuffer(DXVA_RESIDUAL_DIFFERENCE_BUFFER, i); 
			m_pbResidualDiffBuf[i] = NULL;
			m_bCompBufStaus[i] = FALSE;
		}
	}
}

int CH264DXVA1_NV::I_Pred_luma_16x16 PARGS1(int predmode)
{
	switch(predmode)
	{
	case VERT_PRED_16:
		return NVH264VP1_INTRA_16X16_VERTICAL;

	case HOR_PRED_16:
		return NVH264VP1_INTRA_16X16_HORIZONTAL;

	case PLANE_16:
		return NVH264VP1_INTRA_16X16_PLANE;

	case DC_PRED_16:
		{
			int i;	
			int mb_nr=IMGPAR current_mb_nr_d;		
			PixelPos up;					//!< pixel position p(0,-1)
			PixelPos left[2];		//!< pixel positions p(-1, -1..15)
			PixelPos left_up;				
			int up_avail, left_avail, left_up_avail;

			//left-up
			getCurrNeighbourD_Luma ARGS3(-1, -1, &left_up);			

			//left
			getCurrNeighbourA_Luma ARGS3(-1, 0, &left[0]);
			getCurrNeighbourA_Luma ARGS3(-1, 8, &left[1]);	

			//up	
			getCurrNeighbourB_Luma ARGS3(0, -1, &up);	
			if (!active_pps.constrained_intra_pred_flag)
			{
				up_avail	 = (up.pMB!=NULL);
				left_avail = (left[0].pMB!=NULL);
				left_up_avail = (left_up.pMB!=NULL);
			}
			else
			{
				up_avail      = (up.pMB != NULL);
				for (i=0, left_avail=1; i<2;i++)
					left_avail  &= (left[i].pMB != NULL);
				left_up_avail = (left_up.pMB != NULL);
			}			
			// DC prediction		
			if (up_avail && left_avail)
				return NVH264VP1_INTRA_16X16_DC_BOTH;			 // no edge
			else if (!up_avail && left_avail)
				return NVH264VP1_INTRA_16X16_DC_LEFT;			 // upper edge
			else if (up_avail && !left_avail)
				return NVH264VP1_INTRA_16X16_DC_TOP;				// left edge
			else //if (!up_avail && !left_avail)
				return NVH264VP1_INTRA_16X16_DC_NONE;			// HP restriction	// top left corner, nothing to predict from
		}		
	default:
		return 0;
	}
}

int CH264DXVA1_NV::I_Pred_luma_4x4 PARGS4(
	int ioff,						 //!< pixel offset X within MB
	int joff,						 //!< pixel offset Y within MB
	int img_block_x,			//!< location of block X, multiples of 4
	int img_block_y)			//!< location of block Y, multiples of 4
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

	switch(predmode)
	{
	case VERT_PRED:											 /* vertical prediction from block above */
		return NVH264VP1_INTRA_4X4_VERTICAL;

	case HOR_PRED:												/* horizontal prediction from left block */
		return NVH264VP1_INTRA_4X4_HORIZONTAL;

	case DIAG_DOWN_RIGHT_PRED:
		return NVH264VP1_INTRA_4X4_DIAGONAL_DOWN_RIGHT;

	case DIAG_DOWN_LEFT_PRED:
		if(pix_c.pMB)
			return NVH264VP1_INTRA_4X4_DIAGONAL_DOWN_LEFT;
		else
			return NVH264VP1_INTRA_4X4_DIAGONAL_DOWN_LEFT_NO_TOP_RIGHT;

	case	VERT_RIGHT_PRED:/* diagonal prediction -22.5 deg to horizontal plane */
		return NVH264VP1_INTRA_4X4_VERTICAL_RIGHT;

	case	VERT_LEFT_PRED:/* diagonal prediction -22.5 deg to horizontal plane */
		if(pix_c.pMB)
			return NVH264VP1_INTRA_4X4_VERTICAL_LEFT;
		else
			return NVH264VP1_INTRA_4X4_VERTICAL_LEFT_NO_TOP_RIGHT;

	case	HOR_UP_PRED:/* diagonal prediction -22.5 deg to horizontal plane */
		return NVH264VP1_INTRA_4X4_HORIZONTAL_UP;

	case	HOR_DOWN_PRED:/* diagonal prediction -22.5 deg to horizontal plane */
		return NVH264VP1_INTRA_4X4_HORIZONTAL_DOWN;

	case DC_PRED:												 /* DC prediction */
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
				getCurrNeighbourA_Luma ARGS3(ioff-1, joff,	 &pix_a);

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

			block_available_up			 = (pix_b.pMB!=NULL);
			block_available_up_right = (pix_c.pMB!=NULL);
			block_available_up_left	= (pix_d.pMB!=NULL);

			if (block_available_up && block_available_left)
				return NVH264VP1_INTRA_4X4_DC_BOTH;

			else if (!block_available_up && block_available_left)
				return NVH264VP1_INTRA_4X4_DC_LEFT;

			else if (block_available_up && !block_available_left)
				return NVH264VP1_INTRA_4X4_DC_TOP;

			else //if (!block_available_up && !block_available_left)
				return NVH264VP1_INTRA_4X4_DC_NONE;
		}

	default:
		return 0;
	}
}

int CH264DXVA1_NV::I_Pred_luma_8x8 PARGS1(int b8)
{
	int img_block_x = (IMGPAR mb_x_d)*4 + 2*(b8%2);
	int img_block_y = (IMGPAR mb_y_d)*4 + 2*(b8/2);
	int ioff = (b8%2)*8;
	int joff = (b8/2)*8;
	int mb_nr=IMGPAR current_mb_nr_d;
	byte predmode = currMB_d->ipredmode[(img_block_y&3)*4+(img_block_x&3)];
	PixelPos pix_c, pix_d;
	int block_available_up_left,block_available_up_right;
	int high_bits = 0x00;

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
	block_available_up_left	= (pix_d.pMB!=NULL);

	if(block_available_up_left)
		high_bits |= 0x10;
	if(block_available_up_right)
		high_bits |= 0x20;

	switch(predmode)
	{
	case VERT_PRED:
		return (high_bits | NVH264VP1_INTRA_8X8_VERTICAL);

	case HOR_PRED:
		return (high_bits | NVH264VP1_INTRA_8X8_HORIZONTAL);

	case DIAG_DOWN_LEFT_PRED:
		return (high_bits | NVH264VP1_INTRA_8X8_DIAGONAL_DOWN_LEFT);

	case VERT_LEFT_PRED:
		return (high_bits | NVH264VP1_INTRA_8X8_VERTICAL_LEFT);

	case DIAG_DOWN_RIGHT_PRED:
		return (high_bits | NVH264VP1_INTRA_8X8_DIAGONAL_DOWN_RIGHT);

	case	VERT_RIGHT_PRED:/* diagonal prediction -22.5 deg to horizontal plane */
		return (high_bits | NVH264VP1_INTRA_8X8_VERTICAL_RIGHT);

	case	HOR_DOWN_PRED:/* diagonal prediction -22.5 deg to horizontal plane */
		return (high_bits | NVH264VP1_INTRA_8X8_HORIZONTAL_DOWN);

	case	HOR_UP_PRED:/* diagonal prediction -22.5 deg to horizontal plane */
		return (high_bits | NVH264VP1_INTRA_8X8_HORIZONTAL_UP);

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
				getCurrNeighbourA_Luma ARGS3(ioff-1, joff,	 &pix_a);
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

			block_available_up			 = (pix_b.pMB!=NULL);

			if (block_available_up && block_available_left)
				return (high_bits | NVH264VP1_INTRA_8X8_DC_BOTH);

			else if (!block_available_up && block_available_left)
				return (high_bits | NVH264VP1_INTRA_8X8_DC_LEFT);

			else if (block_available_up && !block_available_left)
				return (high_bits | NVH264VP1_INTRA_8X8_DC_TOP);

			else //if (!block_available_up && !block_available_left)
				return (high_bits | NVH264VP1_INTRA_8X8_DC_NONE);
		}

	default:
		return 0;
	}
}

int CH264DXVA1_NV::I_Pred_chroma PARGS1(int uv)
{
	switch (currMB_d->c_ipred_mode)
	{
	case PLANE_8:
		return NVH264VP1_INTRA_CHROMA_PLANE;

	case HOR_PRED_8:
		return NVH264VP1_INTRA_CHROMA_HORIZONTAL;

	case VERT_PRED_8:
		return NVH264VP1_INTRA_CHROMA_VERTICAL;

	case DC_PRED_8:
		{
			int mb_nr=IMGPAR current_mb_nr_d;				

			PixelPos up;				//!< pixel position	p(0,-1)
			PixelPos left[2];	//!< pixel positions p(-1, -1..7), YUV4:2:0 restriction

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

			up_avail			= (up.pMB!=NULL);					
			left_up_avail = (left[0].pMB!=NULL);				

			//===== get prediction value =====
			if(up_avail)
			{
				if (!left_avail[0] && !left_avail[1])
					return NVH264VP1_INTRA_CHROMA_DC_TOP;

				else if (left_avail[0] && left_avail[1])
					return NVH264VP1_INTRA_CHROMA_DC_LEFT_TOP;

				else if(left_avail[0])
					return NVH264VP1_INTRA_CHROMA_DC_UPPER_LEFT_TOP;

				else	//if(left_avail[1])
					return NVH264VP1_INTRA_CHROMA_DC_LOWER_LEFT_TOP;
			}
			else
			{
				if (!left_avail[0] && !left_avail[1])
					return NVH264VP1_INTRA_CHROMA_DC_NONE;

				else if (left_avail[0] && left_avail[1])
					return NVH264VP1_INTRA_CHROMA_DC_LEFT;

				else if(left_avail[0])
					return NVH264VP1_INTRA_CHROMA_DC_UPPER_LEFT;

				else	//if(left_avail[1])
					return NVH264VP1_INTRA_CHROMA_DC_LOWER_LEFT;
			}
		}

	default:
		return 0;
	}
}

int CH264DXVA1_NV::decode_one_macroblock_Intra PARGS0()
{	
	int cbp_blk = currMB_s_d->cbp_blk;
	int current_mb_nr = IMGPAR current_mb_nr_d;
#if defined(ONE_COF)
	short *pcof = (short*)IMGPAR cof;
#else
	short *pcof = (short*)IMGPAR cof_d;
#endif
	int i=0, j=0, i4=0, j4=0;
	int ioff, joff; 
	int b4, b8;
	short* Residual_Y;
	short* Residual_UV;
	nvh264vp1MacroblockControlIntra* Mb_partition;
	nvh264vp1MacroblockControlIntra* Mb_partition_c;
	int Numpartition = 0;
	int luma_type;
	int pred_flag;
	int offset;

	if(IMGPAR m_bFirstMB)
		build_picture_decode_buffer ARGS0();

	if(	((IMGPAR m_iIntraRESBufUsage + (512<<(1-(current_mb_nr&1)))) > (int)m_pAMVACompBufInfo[3].dwBytesToAllocate) || 
		((IMGPAR m_iIntraMCBufUsage + (sizeof(nvh264vp1MacroblockControlIntra)<<((current_mb_nr&1) ? 4 : 5)) > (int)m_pAMVACompBufInfo[2].dwBytesToAllocate)))
	{
		EnterCriticalSection( &crit_EXECUTE );
		CH264DXVA1_NV::ExecuteBuffers ARGS1(E_OUTPUT_INTRA_MBS);
		LeaveCriticalSection( &crit_EXECUTE );
	}

	if(IMGPAR m_lpMBLK_Intra_Luma == NULL)
	{
		int iCompBufIndex = GetCompBufferIndex();
		IMGPAR m_lpMBLK_Intra_Luma = m_pbMacroblockCtrlBuf[iCompBufIndex];
		IMGPAR m_lpRESD_Intra_Luma = m_pbResidualDiffBuf[iCompBufIndex];
		IMGPAR m_IntraL_lCompBufIndex = iCompBufIndex;

		iCompBufIndex = GetCompBufferIndex();
		IMGPAR m_lpMBLK_Intra_Chroma = m_pbMacroblockCtrlBuf[iCompBufIndex];
		IMGPAR m_lpRESD_Intra_Chroma = m_pbResidualDiffBuf[iCompBufIndex];
		IMGPAR m_IntraC_lCompBufIndex = iCompBufIndex;
	}

	Residual_Y = (short*)IMGPAR m_lpRESD_Intra_Luma;
	Residual_UV = (short*)IMGPAR m_lpRESD_Intra_Chroma;
	Mb_partition = (nvh264vp1MacroblockControlIntra*)IMGPAR m_lpMBLK_Intra_Luma;
	Mb_partition_c = (nvh264vp1MacroblockControlIntra*)IMGPAR m_lpMBLK_Intra_Chroma;

	pred_flag = IMGPAR m_bLastIntraMB ? 0x08: 0x00;

	if(IMGPAR MbaffFrameFlag)
	{
		pred_flag |= IMGPAR m_bLastPairIntraMBs ? 0x40: 0x00;

		if(current_mb_nr&1)
		{
			if(IMGPAR m_bLastIntraMB)
				IMGPAR m_bLastPairIntraMBs = 1;
			else
				IMGPAR m_bLastPairIntraMBs = 0;
		}
		if(current_mb_nr&1)
		{
			if(currMB_d->mb_field)
			{
				if(IMGPAR m_lPrev_Field_pair)
					pred_flag |= 0xB0;
				else
					pred_flag |= 0x30;
				IMGPAR m_lPrev_Field_pair = 1;	
			}
			else
			{
				if(IMGPAR m_lPrev_Field_pair)
					pred_flag |= 0xA0;
				else
					pred_flag |= 0x20;
				IMGPAR m_lPrev_Field_pair = 0;
			}
		}
		else
		{
			if(currMB_d->mb_field)
			{
				if(IMGPAR m_lPrev_Field_pair)
					pred_flag |= 0x90;
				else
					pred_flag |= 0x10;
			}
			else
			{
				if(IMGPAR m_lPrev_Field_pair)
					pred_flag |= 0x80;
				else
					pred_flag |= 0x00;
			}		
		}
	}

	if (currMB_d->mb_type == I16MB)
	{
		luma_type					= NVH264VP1_INTRA_MBTYPE_16x16;
		Mb_partition->predFlags		= pred_flag;
		Mb_partition->partIndex		= luma_type;	// = luma_type | (0x00<<4);
		Mb_partition->partOffset	= 0x00;
		Mb_partition->predMode		= I_Pred_luma_16x16 ARGS1(currMB_d->i16mode);
		Mb_partition->uX			= (IMGPAR mb_x_d<<4);
		if(IMGPAR MbaffFrameFlag && currMB_d->mb_field)
			Mb_partition->uY = ((IMGPAR mb_y_d>>1)<<4);
		else
			Mb_partition->uY			= (IMGPAR mb_y_d<<4);

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
				cbp_blk >>= 1;
				pcof += 16;
			}
		}
		Numpartition++; Mb_partition++;
	}
	else if(currMB_d->mb_type == I8MB)
	{
		luma_type = NVH264VP1_INTRA_MBTYPE_8x8;
		for(b8=0; b8<4; b8++)
		{
			ioff = (b8&1)<<3;	//0 8 0 8
			joff = (b8>>1)<<3;	//0 0 8 8

			Mb_partition->predFlags		= pred_flag;
			Mb_partition->partIndex		= luma_type | ((0x3-b8)<<4);
			Mb_partition->partOffset	= ioff | (joff<<4);
			Mb_partition->predMode		= I_Pred_luma_8x8 ARGS1(b8);
			Mb_partition->uX			= (IMGPAR mb_x_d<<4);
			if(IMGPAR MbaffFrameFlag && currMB_d->mb_field)
				Mb_partition->uY = ((IMGPAR mb_y_d>>1)<<4);
			else
				Mb_partition->uY			= (IMGPAR mb_y_d<<4);

			if (currMB_d->cbp&(1<<b8))
#if defined(ONE_COF)
				iDCT_8x8_fcn(&Residual_Y[(joff<<4)+ioff], &IMGPAR cof[b8][0][0][0],16);
#else
				iDCT_8x8_fcn(&Residual_Y[(joff<<4)+ioff], IMGPAR cof_d + (b8<<6),16);
#endif
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
			Numpartition++; Mb_partition++;
		}
	}
	else if(currMB_d->mb_type == I4MB)
	{
		luma_type = NVH264VP1_INTRA_MBTYPE_4x4;
		int idx1[4] = {0,4,8,12};

		for(b8=0; b8<4 ; b8++)
		{
			for(b4=0; b4<4; b4++)
			{
				ioff = idx1[(b4&1)+((b8&1)<<1)];					//0 4 0 4 8 12 8 12 0 4	0	4 8 12 8 12 
				joff = idx1[(b4>>1)+((b8>>1)<<1)];				//0 0 4 4 0	0 4	4 8 8 12 12 8 8 12 12
				i = ioff>>2;
				j = joff>>2;

				Mb_partition->predFlags		= pred_flag;
				Mb_partition->partIndex		= luma_type | ((0xf-((b8<<2)+b4))<<4);
				Mb_partition->partOffset	= ioff | (joff<<4);
				Mb_partition->predMode		= I_Pred_luma_4x4 ARGS4(ioff, joff, (IMGPAR block_x_d+i), (IMGPAR block_y_d+j));
				Mb_partition->uX			= (IMGPAR mb_x_d<<4);
				if(IMGPAR MbaffFrameFlag && currMB_d->mb_field)
					Mb_partition->uY = ((IMGPAR mb_y_d>>1)<<4);
				else
					Mb_partition->uY = (IMGPAR mb_y_d<<4);

				if( currMB_s_d->cbp_blk & (1<<(joff+i)) )
#if defined(ONE_COF)
					iDCT_4x4_fcn(&Residual_Y[(joff<<4)+ioff], &IMGPAR cof[j][i][0][0], 16);
#else
					iDCT_4x4_fcn(&Residual_Y[(joff<<4)+ioff], (IMGPAR cof_d + ((j<<6)+(i<<4))), 16);
#endif
				else
				{
					offset = (joff<<4)+ioff;
					memset(&Residual_Y[offset], 0, 8);
					memset(&Residual_Y[offset+16], 0, 8);
					memset(&Residual_Y[offset+32], 0, 8);
					memset(&Residual_Y[offset+48], 0, 8);
				}

				Numpartition++; Mb_partition++;
			}
		}
	}
	else if(currMB_d->mb_type == IPCM)
	{
		luma_type					= NVH264VP1_INTRA_MBTYPE_PCM;
		Mb_partition->predFlags		= pred_flag;
		Mb_partition->partIndex		= luma_type;	// = luma_type | (0x00<<4);
		Mb_partition->partOffset	= 0x00;
		Mb_partition->predMode		= 0;
		Mb_partition->uX			= (IMGPAR mb_x_d<<4);
		if(IMGPAR MbaffFrameFlag && currMB_d->mb_field)
			Mb_partition->uY = ((IMGPAR mb_y_d>>1)<<4);
		else
			Mb_partition->uY			= (IMGPAR mb_y_d<<4);

//#if	 !defined(H264_ENABLE_INTRINSICS)
		if((cpu_type == CPU_LEVEL_SSE2) || (cpu_type == CPU_LEVEL_SSE3))
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
		else
		{
			for(i=0 ; i<128 ; i++)
			{
				*(Residual_Y+i*2) = ((*(pcof+i))&0xFF);
				*(Residual_Y+i*2+1) = ((*(unsigned short*)(pcof+i))>>8);
			}	
		}
//#endif

#if defined(ONE_COF)
		memset((unsigned char *) &IMGPAR cof[0][0][0][0], 0, 4*BLOCK_SIZE*BLOCK_SIZE*BLOCK_SIZE*sizeof(short));
#else
		memset((unsigned char *) IMGPAR cof_d, 0, 4*BLOCK_SIZE*BLOCK_SIZE*BLOCK_SIZE*sizeof(short));
#endif
		Numpartition++; Mb_partition++;
	}

	IMGPAR m_lpMBLK_Intra_Luma += sizeof(nvh264vp1MacroblockControlIntra)*Numpartition;
	IMGPAR m_iIntraMCBufUsage += sizeof(nvh264vp1MacroblockControlIntra)*Numpartition;

	IMGPAR m_lpRESD_Intra_Luma += 512;
	IMGPAR m_iIntraRESBufUsage += 512;

#ifdef __SUPPORT_YUV400__
	if (dec_picture->chroma_format_idc != YUV400)
	{
#endif
		Mb_partition_c->predFlags = pred_flag | NVH264VP1_PRED_FLAG_CHROMA;
		Mb_partition_c->partIndex = luma_type | 0;
		Mb_partition_c->partOffset = 0;
		Mb_partition_c->predMode = I_Pred_chroma ARGS1(0);
		Mb_partition_c->uX = (IMGPAR mb_x_d<<4);
		if(IMGPAR MbaffFrameFlag && currMB_d->mb_field)
			Mb_partition_c->uY = ((IMGPAR mb_y_d>>1)<<4);
		else
			Mb_partition_c->uY = (IMGPAR mb_y_d<<4);
		Numpartition++; 

		IMGPAR m_lpMBLK_Intra_Chroma += sizeof(nvh264vp1MacroblockControlIntra);

		if(currMB_d->mb_type != IPCM)
		{
#if	 !defined(H264_ENABLE_INTRINSICS)
			short tmp4[4][4];
			for(int uv=0;uv<2;uv++)
			{

				for(b8=0;b8<4;b8++)
				{

					ioff = (b8&1)<<2;	//0 4 0 4
					joff = (b8&2)<<1;	//0 0 4 4
					if( currMB_s_d->cbp_blk & (1<<cbp_blk_chroma[uv][b8]))//else means residual = 0
#if defined(ONE_COF)
						iDCT_4x4_fcn(&tmp4[0][0], &IMGPAR cof[4+uv][b8][0][0],4);
#else
						iDCT_4x4_fcn(&tmp4[0][0], (IMGPAR cof_d + ((4+uv<<6)+(b8<<4))),4);
#endif
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
				ioff = (b8&1)<<3;	//0 4 0 4
				joff = (b8&2)<<5;	//0 0 4 4

				iDCT_4x4_UV_fcn(&Residual_UV[(joff+ioff)]
#if defined(ONE_COF)
				, &IMGPAR cof[4][b8][0][0]
#else
				, (IMGPAR cof_d + ((4<<6)+(b8<<4)))
#endif
					,( currMB_s_d->cbp_blk & (1<<cbp_blk_chroma[0][b8])) // residual U
					,( currMB_s_d->cbp_blk & (1<<cbp_blk_chroma[1][b8])) // residual V
					, 16);
			}
#endif
		}
		else
		{
#if defined(ONE_COF)
			pcof = &IMGPAR cof[4][0][0][0];
#else
			pcof = (IMGPAR cof_d+256);
#endif

//#if	 !defined(H264_ENABLE_INTRINSICS)
			if((cpu_type == CPU_LEVEL_SSE2) || (cpu_type == CPU_LEVEL_SSE3))
			{
				__m128i xmm0, xmm1, xmm2, xmm3, xmm5, xmm7;
				short *dest = Residual_UV, *pcof_v = pcof+64;

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

#if defined(ONE_COF)
			memset((unsigned char *) &IMGPAR cof[4][0][0][0], 0, 2*BLOCK_SIZE*BLOCK_SIZE*BLOCK_SIZE*sizeof(short));
#else
			memset((unsigned char *) (IMGPAR cof_d+256), 0, 2*BLOCK_SIZE*BLOCK_SIZE*BLOCK_SIZE*sizeof(short));
#endif
			//For CABAC decoding of Dquant
			last_dquant=0;
		}
		IMGPAR m_lpRESD_Intra_Chroma += 256;
#ifdef __SUPPORT_YUV400__
	}
#endif



	IMGPAR m_lmbCount_Intra++;
	if(IMGPAR m_lFirstMBAddress_Intra == -1)
		IMGPAR m_lFirstMBAddress_Intra = IMGPAR current_mb_nr_d;

	if(!stream_global->m_is_MTMS && (!((IMGPAR smart_dec & SMART_DEC_NODEBLK_ALL) || ((IMGPAR smart_dec & SMART_DEC_NODEBLK_B) && (IMGPAR currentSlice->picture_type == B_SLICE)))))
		DeblockMacroblock ARGS2(dec_picture,current_mb_nr);

	if(current_mb_nr == (IMGPAR structure ? ((IMGPAR FrameSizeInMbs-1)/2) : (IMGPAR FrameSizeInMbs -1)))
	{
		EnterCriticalSection( &crit_EXECUTE );
		CH264DXVA1_NV::ExecuteBuffers ARGS1(E_USING_DEBLOCK);
		LeaveCriticalSection( &crit_EXECUTE );
	}
	IMGPAR m_bLastIntraMB = TRUE;

	return 0;
}

int CH264DXVA1_NV::decode_one_macroblock_Inter PARGS0()
{
	int fw_refframe=-1, bw_refframe=-1;
	int mb_nr = IMGPAR current_mb_nr_d;
	int list_offset;
	int vec_x_base, vec_y_base;
	int curr_mb_field = ((IMGPAR MbaffFrameFlag)&&(currMB_s_d->mb_field));

	if(IMGPAR m_bFirstMB)
		build_picture_decode_buffer ARGS0();

	if(	((IMGPAR m_iInterRESBufUsage_L + (512<<(1-(mb_nr&1)))) > (int)m_pAMVACompBufInfo[3].dwBytesToAllocate) || 
		((IMGPAR m_iInterRESBufUsage_C + (512<<(1-(mb_nr&1)))) > (int)m_pAMVACompBufInfo[3].dwBytesToAllocate) ||
		((IMGPAR m_iInterMCBufUsage + (sizeof(nvh264vp1MacroblockControlInterChroma)<<((mb_nr&1) ? 4 : 5)) > (int)m_pAMVACompBufInfo[2].dwBytesToAllocate)))
	{
		EnterCriticalSection( &crit_EXECUTE );
		CH264DXVA1_NV::ExecuteBuffers ARGS1(E_USING_NO_FLAGS);
		LeaveCriticalSection( &crit_EXECUTE );
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
			list_offset				= 4; // top field mb			
			vec_y_base -= 8;
		}
		else
		{
			list_offset				= 2; // bottom field mb
		}
		clip_max_y		= (dec_picture->size_y>>1)-1;
		clip_max_y_cr = (dec_picture->size_y_cr>>1)-1;
	}
	else
	{
		list_offset				= 0;	// no mb aff or frame mb
		clip_max_y		= dec_picture->size_y-1;
		clip_max_y_cr = dec_picture->size_y_cr-1;
	}
	clip_max_x		= dec_picture->size_x+1;
	clip_max_x_cr = dec_picture->size_x_cr-1;	

	calc_chroma_vector_adjustment ARGS2(list_offset, curr_mb_field);

	// luma decoding **************************************************	
	if(IMGPAR m_lpMBLK_Inter_Luma == NULL)
	{
		int temp;
		temp = GetCompBufferIndex();
		IMGPAR m_lpMBLK_Inter_Luma = m_pbMacroblockCtrlBuf[temp];
		IMGPAR m_lpRESD_Inter_Luma = m_pbResidualDiffBuf[temp];
		IMGPAR m_InterL_lCompBufIndex = temp;

		temp = GetCompBufferIndex();
		IMGPAR m_lpMBLK_Inter_Chroma = m_pbMacroblockCtrlBuf[temp];
		IMGPAR m_lpRESD_Inter_Chroma = m_pbResidualDiffBuf[temp];
		IMGPAR m_InterC_lCompBufIndex = temp;
	}

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

	build_residual_buffer_Inter_Luma ARGS0();
#ifdef __SUPPORT_YUV400__
	if (dec_picture->chroma_format_idc != YUV400)
#endif
		build_residual_buffer_Inter_Chroma ARGS0();

	IMGPAR m_lmbCount_Inter++;
	if(IMGPAR m_lFirstMBAddress_Inter == -1)
		IMGPAR m_lFirstMBAddress_Inter = IMGPAR current_mb_nr_d;

	if(!stream_global->m_is_MTMS && (!((IMGPAR smart_dec & SMART_DEC_NODEBLK_ALL) || ((IMGPAR smart_dec & SMART_DEC_NODEBLK_B) && (IMGPAR currentSlice->picture_type == B_SLICE)))))
		DeblockMacroblock ARGS2(dec_picture,mb_nr);

	if(mb_nr == (IMGPAR structure ? ((IMGPAR FrameSizeInMbs-1)/2) : (IMGPAR FrameSizeInMbs -1)))
	{
		EnterCriticalSection( &crit_EXECUTE );
		CH264DXVA1_NV::ExecuteBuffers ARGS1(E_USING_DEBLOCK);
		LeaveCriticalSection( &crit_EXECUTE );
	}

	IMGPAR m_bLastIntraMB = FALSE;
	if(IMGPAR MbaffFrameFlag && (IMGPAR current_mb_nr_d&1))
		IMGPAR m_bLastPairIntraMBs = FALSE;

	return 0;
}

void CH264DXVA1_NV::build_macroblock_buffer_Inter_16x16 PARGS1(int list_offset)
{
	struct nvh264vp1MacroblockControlInterLuma* MB_Partition = (nvh264vp1MacroblockControlInterLuma*)IMGPAR m_lpMBLK_Inter_Luma;
	struct nvh264vp1MacroblockControlInterChroma* MB_Partition_C = (nvh264vp1MacroblockControlInterChroma*)IMGPAR m_lpMBLK_Inter_Chroma;
	int pred_dir = currMB_d->b8pdir[0];
	int cbp_total = currMB_d->cbp;
	int cbp=0;


	(this->*build_macroblock_buffer_Inter[IMGPAR m_iFp_Idx]) ARGS5(pred_dir,0,list_offset,MB_Partition,MB_Partition_C);

	MB_Partition->partIndex	= (currMB_d->cbp&0x0F); //(currMB_d->cbp&0x0F) | (0x00<<4)
	MB_Partition->partOffset = 0;
	MB_Partition->partSize	 = 0x88;	//(16/2) | ((16/2)<<4)

#ifdef __SUPPORT_YUV400__
	if (dec_picture->chroma_format_idc != YUV400)
	{
#endif
		cbp_total = (currMB_s_d->cbp_blk>>16);
		if((cbp_total&0x0F)>0)
			cbp |= 0x01;
		if((cbp_total&0xF0)>0)
			cbp |= 0x02;

		MB_Partition_C->predFlags	= MB_Partition->predFlags | NVH264VP1_PRED_FLAG_CHROMA;
		MB_Partition_C->partIndex	= cbp; //currMB_d->cbp | (0x00<<4)
		MB_Partition_C->partOffset = 0;
		MB_Partition_C->partSize	 = 0x88;	//(16/2) | ((16/2)<<4)

#ifdef __SUPPORT_YUV400__
	}
#endif

	IMGPAR m_lpMBLK_Inter_Luma += sizeof(struct nvh264vp1MacroblockControlInterLuma);
	IMGPAR m_lpMBLK_Inter_Chroma += sizeof(struct nvh264vp1MacroblockControlInterChroma);
	IMGPAR m_iInterMCBufUsage += sizeof(struct nvh264vp1MacroblockControlInterChroma);
}

void CH264DXVA1_NV::build_macroblock_buffer_Inter_16x8 PARGS1(int list_offset)
{
	struct nvh264vp1MacroblockControlInterLuma* MB_Partition = (nvh264vp1MacroblockControlInterLuma*)IMGPAR m_lpMBLK_Inter_Luma;
	struct nvh264vp1MacroblockControlInterChroma* MB_Partition_C = (nvh264vp1MacroblockControlInterChroma*)IMGPAR m_lpMBLK_Inter_Chroma;
	int joff, k;
	int pred_dir;
	int cbp_total = currMB_d->cbp;
	int cbp=0;

	for(k=0;k<2;k++,MB_Partition++,MB_Partition_C++)
	{
		joff = (k<<3);
		pred_dir = currMB_d->b8pdir[k<<1];
		(this->*build_macroblock_buffer_Inter[IMGPAR m_iFp_Idx]) ARGS5(pred_dir,joff,list_offset,MB_Partition,MB_Partition_C);
		if(k==0)
			MB_Partition->partIndex	= (currMB_d->cbp&0x0F) | (0x10);//(currMB_d->cbp&0x0F) | (0x01<<4)
		else
			MB_Partition->partIndex	= 0;
		MB_Partition->partOffset = (k<<7); // 0x00 | ((k<<3)<<4)
		MB_Partition->partSize	 = 0x48;	//(16/2) | ((8/2)<<4)


#ifdef __SUPPORT_YUV400__
		if (dec_picture->chroma_format_idc != YUV400)
		{
#endif
			cbp_total = (currMB_s_d->cbp_blk>>16);
			if((cbp_total&0x0F)>0)
				cbp |= 0x01;
			if((cbp_total&0xF0)>0)
				cbp |= 0x02;		
			MB_Partition_C->predFlags	= MB_Partition->predFlags | NVH264VP1_PRED_FLAG_CHROMA;
			if(k==0)
				MB_Partition_C->partIndex	= cbp | (0x10);//cbp | (0x01<<4)
			else
				MB_Partition_C->partIndex	= 0;
			MB_Partition_C->partOffset = (k<<7); // 0x00 | ((k<<3)<<4)
			MB_Partition_C->partSize	 = 0x48;	//(16/2) | ((16/2)<<4)

#ifdef __SUPPORT_YUV400__
		}
#endif
	}

	IMGPAR m_lpMBLK_Inter_Luma += (2*sizeof(struct nvh264vp1MacroblockControlInterLuma));
	IMGPAR m_lpMBLK_Inter_Chroma += (2*sizeof(struct nvh264vp1MacroblockControlInterChroma));
	IMGPAR m_iInterMCBufUsage += (2*sizeof(struct nvh264vp1MacroblockControlInterChroma));
}

void CH264DXVA1_NV::build_macroblock_buffer_Inter_8x16 PARGS1(int list_offset)
{
	struct nvh264vp1MacroblockControlInterLuma* MB_Partition = (nvh264vp1MacroblockControlInterLuma*)IMGPAR m_lpMBLK_Inter_Luma;
	struct nvh264vp1MacroblockControlInterChroma* MB_Partition_C = (nvh264vp1MacroblockControlInterChroma*)IMGPAR m_lpMBLK_Inter_Chroma;
	int i4, k;
	int pred_dir;
	int cbp_total = currMB_d->cbp;
	int cbp=0;

	for(k=0;k<2;k++,MB_Partition++,MB_Partition_C++)
	{
		i4 = (k<<1);
		pred_dir = currMB_d->b8pdir[k];
		(this->*build_macroblock_buffer_Inter[IMGPAR m_iFp_Idx]) ARGS5(pred_dir,i4,list_offset,MB_Partition,MB_Partition_C);
		if(k==0)
			MB_Partition->partIndex	= (currMB_d->cbp&0x0F) | (0x10);//(currMB_d->cbp&0x0F) | (0x01<<4)
		else
			MB_Partition->partIndex	= 0;
		MB_Partition->partOffset = (k<<3); // (k<<3) | (0x00<<4)
		MB_Partition->partSize	 = 0x84;	//(8/2) | ((16/2)<<4)

#ifdef __SUPPORT_YUV400__
		if (dec_picture->chroma_format_idc != YUV400)
		{
#endif
			cbp_total = (currMB_s_d->cbp_blk>>16);
			if((cbp_total&0x0F)>0)
				cbp |= 0x01;
			if((cbp_total&0xF0)>0)
				cbp |= 0x02;
			MB_Partition_C->predFlags	= MB_Partition->predFlags | NVH264VP1_PRED_FLAG_CHROMA;
			if(k==0)
				MB_Partition_C->partIndex	= cbp | (0x10);//cbp | (0x01<<4);
			else
				MB_Partition_C->partIndex	= 0;
			MB_Partition_C->partOffset = (k<<3); // (k<<3) | (0x00<<4)
			MB_Partition_C->partSize	 = 0x84;	//(8/2) | ((16/2)<<4)

#ifdef __SUPPORT_YUV400__
		}
#endif

	}
	IMGPAR m_lpMBLK_Inter_Luma += (2*sizeof(struct nvh264vp1MacroblockControlInterLuma));
	IMGPAR m_lpMBLK_Inter_Chroma += (2*sizeof(struct nvh264vp1MacroblockControlInterChroma));
	IMGPAR m_iInterMCBufUsage += (2*sizeof(struct nvh264vp1MacroblockControlInterChroma));
}

void CH264DXVA1_NV::build_macroblock_buffer_Inter_8x8 PARGS1(int list_offset)
{
	struct nvh264vp1MacroblockControlInterLuma* MB_Partition = (nvh264vp1MacroblockControlInterLuma*)IMGPAR m_lpMBLK_Inter_Luma;
	struct nvh264vp1MacroblockControlInterChroma* MB_Partition_C = (nvh264vp1MacroblockControlInterChroma*)IMGPAR m_lpMBLK_Inter_Chroma;
	int i,b8,count,idx,cbp;
	int cbp_total = currMB_d->cbp;
	int temp=0;
	int pred_dir,mode;
	int fw_refframe,bw_refframe;
#if 0
	int table[2] = {0,8};
	int table1[2] = {0,4};
	int b84_idx[4][2] = { {0,4} , {2,6} , {8,12} , {10,14} };
	int b48_idx[4][2] = { {0,1} , {2,3} , {8,9} , {10,11} };
	int b44_idx[4][4] = { {0,1,4,5} , {2,3,6,7} , {8,9,12,13} , {10,11,14,15} };

	cbp = count = 0;
	for (b8=0; b8<4; b8++)
	{
		switch(currMB_d->b8mode[b8])
		{
		case 0:
			if(IMGPAR type==B_SLICE)
			{
				if(active_sps->direct_8x8_inference_flag)
					count++;
				else
					count+=4;
			}
			else
				count++;
			break;
		case 4:
			count++;
			break;
		case 5:
		case 6:
			count += 2;
			break;
		case 7:
			count += 4;
			break;
		}
	}
#else
	const static int	table[2] = {0,8},
		table1[2] = {0,4},
		b84_idx[4][2] = { {0,4} , {2,6} , {8,12} , {10,14} },
		b48_idx[4][2] = { {0,1} , {2,3} , {8,9} , {10,11} },
		b44_idx[4][4] = { {0,1,4,5} , {2,3,6,7} , {8,9,12,13} , {10,11,14,15} },
		b8modes_count_tab[] =	{ 0, //-1
		1, 0, 0, 0, 1, 2, 2, 4//,	//0-7
		//0, 0, 0, 0, 0, 0, 0, 0,	//8-15
		//0, 0, 0, 0, 0, 0, 0, 0,	//16-23
		//0, 0, 0, 0, 0, 0, 0, 0,	//24-31
		//0 //32
	},
	b8modes_count_tab2[] =	{ 0, //-1
	4, 0, 0, 0, 1, 2, 2, 4//,	//0-7
	//0, 0, 0, 0, 0, 0, 0, 0,	//8-15
	//0, 0, 0, 0, 0, 0, 0, 0,	//16-23
	//0, 0, 0, 0, 0, 0, 0, 0,	//24-31
	//0 //32
	};
	if(IMGPAR type!=B_SLICE || (IMGPAR type==B_SLICE && active_sps.direct_8x8_inference_flag))
		count = 
		b8modes_count_tab[((int) currMB_d->b8mode[0]) + 1]
	+ b8modes_count_tab[((int) currMB_d->b8mode[1]) + 1]
	+ b8modes_count_tab[((int) currMB_d->b8mode[2]) + 1]
	+ b8modes_count_tab[((int) currMB_d->b8mode[3]) + 1];
	else
		count = 
		b8modes_count_tab2[((int) currMB_d->b8mode[0]) + 1]
	+ b8modes_count_tab2[((int) currMB_d->b8mode[1]) + 1]
	+ b8modes_count_tab2[((int) currMB_d->b8mode[2]) + 1]
	+ b8modes_count_tab2[((int) currMB_d->b8mode[3]) + 1];
#endif

	cbp = idx=0;
	for (b8=0; b8<4; b8++)
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

		if(mode==PB_8x8 || mode==0)
		{
			count--;
			(this->*build_macroblock_buffer_Inter[IMGPAR m_iFp_Idx]) ARGS5(pred_dir,b8_idx[b8],list_offset,MB_Partition,MB_Partition_C);
			if(b8==0)
				MB_Partition->partIndex	= (currMB_d->cbp&0x0F) | (count<<4);
			else
				MB_Partition->partIndex	= (count<<4);
			MB_Partition->partOffset = (table[b8&1]) | ((table[b8>>1])<<4); 
			MB_Partition->partSize	 = 0x44;	//(8/2) | ((8/2)<<4)
#ifdef __SUPPORT_YUV400__
			if (dec_picture->chroma_format_idc != YUV400)
			{
#endif
				cbp_total = (currMB_s_d->cbp_blk>>16);
				if((cbp_total&0x0F)>0)
					cbp |= 0x01;
				if((cbp_total&0xF0)>0)
					cbp |= 0x02;

				MB_Partition_C->predFlags	= MB_Partition->predFlags | NVH264VP1_PRED_FLAG_CHROMA;
				if(b8==0)
					MB_Partition_C->partIndex	= cbp | (count<<4);
				else
					MB_Partition_C->partIndex	= (count<<4);
				MB_Partition_C->partOffset = (table[b8&1]) | ((table[b8>>1])<<4); 
				MB_Partition_C->partSize	 = 0x44;	//(8/2) | ((8/2)<<4)
#ifdef __SUPPORT_YUV400__
			}
#endif			
			idx++; MB_Partition_C++; MB_Partition++;
		}

		else if(mode==PB_8x4)
		{
			for(i=0 ; i<2 ; i++)
			{
				count--;
				(this->*build_macroblock_buffer_Inter[IMGPAR m_iFp_Idx]) ARGS5(pred_dir,b84_idx[b8][i],list_offset,MB_Partition,MB_Partition_C);
				if(b8==0 && i==0)
					MB_Partition->partIndex	= (currMB_d->cbp&0x0F) | (count<<4);
				else
					MB_Partition->partIndex	= (count<<4);
				MB_Partition->partOffset = (table[b8&1]) | ((table[b8>>1]+table1[i])<<4); 
				MB_Partition->partSize	 = 0x24;	//(8/2) | ((4/2)<<4)

#ifdef __SUPPORT_YUV400__
				if (dec_picture->chroma_format_idc != YUV400)
				{
#endif
					cbp_total = (currMB_s_d->cbp_blk>>16);
					cbp=0;
					if((cbp_total&0x0F)>0)
						cbp |= 0x01;
					if((cbp_total&0xF0)>0)
						cbp |= 0x02;

					MB_Partition_C->predFlags	= MB_Partition->predFlags | NVH264VP1_PRED_FLAG_CHROMA;
					if(b8==0 && i==0)
						MB_Partition_C->partIndex	= cbp | (count<<4);
					else
						MB_Partition_C->partIndex	= (count<<4);
					MB_Partition_C->partOffset = (table[b8&1]) | ((table[b8>>1]+table1[i])<<4); 
					MB_Partition_C->partSize	 = 0x24;	//(8/2) | ((4/2)<<4)
#ifdef __SUPPORT_YUV400__
				}
#endif			
				idx++; MB_Partition_C++; MB_Partition++;
			}
		}
		else if(mode==PB_4x8)
		{
			for(i=0 ; i<2 ; i++)
			{
				count--;
				(this->*build_macroblock_buffer_Inter[IMGPAR m_iFp_Idx]) ARGS5(pred_dir,b48_idx[b8][i],list_offset,MB_Partition,MB_Partition_C);
				if(b8==0 && i==0)
					MB_Partition->partIndex	= (currMB_d->cbp&0x0F) | (count<<4);
				else
					MB_Partition->partIndex	= (count<<4);
				MB_Partition->partOffset = (table[b8&1]+table1[i]) | ((table[b8>>1])<<4); 
				MB_Partition->partSize	 = 0x42;	//(4/2) | ((8/2)<<4)

#ifdef __SUPPORT_YUV400__
				if (dec_picture->chroma_format_idc != YUV400)
				{
#endif
					cbp_total = (currMB_s_d->cbp_blk>>16);
					cbp=0;
					if((cbp_total&0x0F)>0)
						cbp |= 0x01;
					if((cbp_total&0xF0)>0)
						cbp |= 0x02;

					MB_Partition_C->predFlags	= MB_Partition->predFlags | NVH264VP1_PRED_FLAG_CHROMA;
					if(b8==0 && i==0)
						MB_Partition_C->partIndex	= cbp | (count<<4);
					else
						MB_Partition_C->partIndex	= (count<<4);
					MB_Partition_C->partOffset = (table[b8&1]+table1[i]) | ((table[b8>>1])<<4); 
					MB_Partition_C->partSize	 = 0x42;	//(4/2) | ((8/2)<<4)
#ifdef __SUPPORT_YUV400__
				}
#endif			
				idx++; MB_Partition_C++; MB_Partition++;
			}
		}
		else
		{
			for(i=0 ; i<4 ; i++)
			{
				count--;
				(this->*build_macroblock_buffer_Inter[IMGPAR m_iFp_Idx]) ARGS5(pred_dir,b44_idx[b8][i],list_offset,MB_Partition,MB_Partition_C);
				if(b8==0 && i==0)
					MB_Partition->partIndex	= (currMB_d->cbp&0x0F) | (count<<4);
				else
					MB_Partition->partIndex	= (count<<4);
				MB_Partition->partOffset = (table[b8&1]+table1[i&1]) | ((table[b8>>1]+table1[i>>1])<<4); 
				MB_Partition->partSize	 = 0x22;	//(4/2) | ((4/2)<<4)

#ifdef __SUPPORT_YUV400__
				if (dec_picture->chroma_format_idc != YUV400)
				{
#endif
					cbp_total = (currMB_s_d->cbp_blk>>16);
					int cbp=0;
					if((cbp_total&0x0F)>0)
						cbp |= 0x01;
					if((cbp_total&0xF0)>0)
						cbp |= 0x02;

					MB_Partition_C->predFlags	= MB_Partition->predFlags | NVH264VP1_PRED_FLAG_CHROMA;
					if(b8==0 && i==0)
						MB_Partition_C->partIndex	= cbp | (count<<4);
					else
						MB_Partition_C->partIndex	= (count<<4);
					MB_Partition_C->partOffset = (table[b8&1]+table1[i&1]) | ((table[b8>>1]+table1[i>>1])<<4); 
					MB_Partition_C->partSize	 = 0x22;	//(4/2) | ((4/2)<<4)
#ifdef __SUPPORT_YUV400__
				}
#endif			
				idx++; MB_Partition_C++; MB_Partition++;
			}
		}
	}
	IMGPAR m_lpMBLK_Inter_Luma += (idx*sizeof(struct nvh264vp1MacroblockControlInterLuma));
	IMGPAR m_lpMBLK_Inter_Chroma += (idx*sizeof(struct nvh264vp1MacroblockControlInterChroma));
	IMGPAR m_iInterMCBufUsage += (idx*sizeof(struct nvh264vp1MacroblockControlInterChroma));
}

void CH264DXVA1_NV::build_residual_buffer_Inter_Luma PARGS0()
{
	int b8,b4;
#if defined(ONE_COF)
	short *pcof = (short*)IMGPAR cof;
#else
	short *pcof = (short*) IMGPAR cof_d;
#endif
	int cbp_8x8 = currMB_d->cbp;
	int cbp_blk = currMB_s_d->cbp_blk;
	int idx[2] = {0,8};
	int offset;

	static const int b44_idx[4][4] = { {0,1,4,5} , {2,3,6,7} , {8,9,12,13} , {10,11,14,15} };
	static const int b44_offset[4][4] = { {0,16,64,80} , {32,48,96,112} , {128,144,192,208} , {160,176,224,240} };

	for(b8=0 ; b8<4 ; b8++)
	{
		if(1 & (cbp_8x8>>b8))
		{
			if (!currMB_d->luma_transform_size_8x8_flag)
			{
				for(b4=0 ; b4<4 ; b4++)
				{		
					if( (cbp_blk>>b44_idx[b8][b4]) & 1 )
						iDCT_4x4_fcn((short*)&IMGPAR m_lpRESD_Inter_Luma[((b4>>1)<<6)+((b4&1)<<3)], pcof+b44_offset[b8][b4],8);
					else
					{
						offset = (((b4>>1)<<6)+((b4&1)<<3));
						memset(&IMGPAR m_lpRESD_Inter_Luma[offset], 0, 8);
						memset(&IMGPAR m_lpRESD_Inter_Luma[offset+16], 0, 8);
						memset(&IMGPAR m_lpRESD_Inter_Luma[offset+32], 0, 8);
						memset(&IMGPAR m_lpRESD_Inter_Luma[offset+48], 0, 8);
					}
				} 
			}
			else
#if defined(ONE_COF)
				iDCT_8x8_fcn((short*)IMGPAR m_lpRESD_Inter_Luma, &IMGPAR cof[b8][0][0][0],8);
#else
				iDCT_8x8_fcn((short*)IMGPAR m_lpRESD_Inter_Luma, IMGPAR cof_d + (b8<<6),8);
#endif

			IMGPAR m_lpRESD_Inter_Luma += 128;
			IMGPAR m_iInterRESBufUsage_L += 128;
		}
	}
}

void CH264DXVA1_NV::build_residual_buffer_Inter_Chroma PARGS0()
{
	int uv,b4;
	int cbp_total;
	int flag[2];
	int tmp88[4] = {0,0,0,0};
	int offset;

	cbp_total= (currMB_s_d->cbp_blk>>16);
	flag[0] = ((cbp_total&0x0F)>0);
	flag[1] = ((cbp_total&0xF0)>0);

	for(uv=0 ; uv<2 ; uv++)	 //for u and V
	{
		if(flag[uv])
		{
			for(b4=0 ; b4<4 ; b4++)
			{
				if(currMB_s_d->cbp_blk & (1<<cbp_blk_chroma[uv][b4]))
#if defined(ONE_COF)
					iDCT_4x4_fcn((short*)&IMGPAR m_lpRESD_Inter_Chroma[((b4>>1)<<6)+((b4&1)<<3)], &IMGPAR cof[4+uv][b4][0][0],8);
#else
					iDCT_4x4_fcn((short*)&IMGPAR m_lpRESD_Inter_Chroma[((b4>>1)<<6)+((b4&1)<<3)], (IMGPAR cof_d + ( ((4+uv)<<6) + (b4<<4) ) ),8);
#endif
				else
				{
					offset = (((b4>>1)<<6)+((b4&1)<<3));
					memset(&IMGPAR m_lpRESD_Inter_Chroma[offset], 0, 8);
					memset(&IMGPAR m_lpRESD_Inter_Chroma[offset+16], 0, 8);
					memset(&IMGPAR m_lpRESD_Inter_Chroma[offset+32], 0, 8);
					memset(&IMGPAR m_lpRESD_Inter_Chroma[offset+48], 0, 8);
				}

				cbp_total>>=1;
			}
			IMGPAR m_lpRESD_Inter_Chroma += 128;
			IMGPAR m_iInterRESBufUsage_C += 128; 
		}
	}
}

void CH264DXVA1_NV::BeginDecodeFrame PARGS0()
{
	if(!m_pUncompBufQueue)
		return;

	m_bFirst_Slice = TRUE;

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

	IMGPAR m_lFrame_Counter = ++m_nFrameCounter;

	if(!stream_global->m_is_MTMS && (!((IMGPAR smart_dec & SMART_DEC_NODEBLK_ALL) || ((IMGPAR smart_dec & SMART_DEC_NODEBLK_B) && (IMGPAR currentSlice->picture_type == B_SLICE)))))
	{
		if(IMGPAR type == B_SLICE)
		{
			fp_GetStrength_v			= GetStrength_v;
			fp_GetStrengthInternal_v	= GetStrengthInternal_v;
			fp_GetStrength_h			= GetStrength_h;
			fp_GetStrengthInternal_h	= GetStrengthInternal_h;
//#if	 defined(H264_ENABLE_INTRINSICS)
			if((cpu_type == CPU_LEVEL_SSE2) || (cpu_type == CPU_LEVEL_SSE3))
			{
				fp_GetStrengthInternal_v_3 = GetStrengthInternal_v_3;
				fp_GetStrengthInternal_h_3 = GetStrengthInternal_h_3;
			}
//#endif 
		}
		else
		{
			fp_GetStrength_v			= GetStrength_v_P;
			fp_GetStrength_h			= GetStrength_h_P;
			
//#if	 defined(H264_ENABLE_INTRINSICS)
			if((cpu_type == CPU_LEVEL_SSE2) || (cpu_type == CPU_LEVEL_SSE3))
			{
				fp_GetStrengthInternal_v_3 = GetStrengthInternal_v_P_3;
				fp_GetStrengthInternal_h_3 = GetStrengthInternal_h_P_3;
				fp_GetStrengthInternal_v	= GetStrengthInternal_v_P;
				fp_GetStrengthInternal_h	= GetStrengthInternal_h_P;
			}
			else
			{
				fp_GetStrengthInternal_v	= GetStrengthInternal_v_P_c;
				fp_GetStrengthInternal_h	= GetStrengthInternal_h_P_c;
			}
//#endif
		}
	}

	IMGPAR m_bFirstMB = TRUE;
}

void CH264DXVA1_NV::EndDecodeFrame PARGS0()
{
	// This frame is splited into two fields
	switch(dec_picture->structure)
	{
	case FRAME:
		if(dec_picture->used_for_reference && !dec_picture->frame_mbs_only_flag)
		{
			SetSurfaceInfo ARGS3(dpb.fs_on_view[0][dpb.used_size_on_view[0]-1]->top_field->unique_id,0,0);
			SetSurfaceInfo ARGS3(dpb.fs_on_view[0][dpb.used_size_on_view[0]-1]->bottom_field->unique_id,1,0);
		}
		break;
	case TOP_FIELD:
	case BOTTOM_FIELD:
		if(dpb.fs_on_view[0][dpb.used_size_on_view[0]-1]->is_used==(TOP_FIELD|BOTTOM_FIELD))
			SetSurfaceInfo ARGS3(dpb.fs_on_view[0][dpb.used_size_on_view[0]-1]->frame->unique_id,0,0);
		break;
	}
}

void CH264DXVA1_NV::ReleaseDecodeFrame PARGS1(int frame_index)
{
	if(m_pUncompBufQueue)
		m_pUncompBufQueue->PutItem(frame_index);
}

int CH264DXVA1_NV::build_picture_decode_buffer PARGS0()
{
	if(!IMGPAR m_bFirstMB)
		return 0;
	IMGPAR m_bFirstMB = FALSE;
	IMGPAR m_bLastIntraMB = IMGPAR m_bLastPairIntraMBs = IMGPAR m_lPrev_Field_pair = FALSE;
	IMGPAR m_lpMBLK_Inter_Luma = IMGPAR m_lpMBLK_Intra_Luma = NULL;
	IMGPAR m_lFirstMBAddress_Inter = IMGPAR m_lFirstMBAddress_Intra = -1;
	IMGPAR m_lmbCount_Inter = IMGPAR m_lmbCount_Intra = 0;
	IMGPAR m_iIntraMCBufUsage = IMGPAR m_iInterMCBufUsage = IMGPAR m_iIntraRESBufUsage = IMGPAR m_iInterRESBufUsage_L = IMGPAR m_iInterRESBufUsage_C = 0;
	IMGPAR m_bLastIntraMB = IMGPAR m_lPrev_Field_pair = IMGPAR m_bLastPairIntraMBs = FALSE;
	nvh264vp1PictureDecode* m_pnv1PictureDecode;
	DXVA_BufferDescription *m_pDxvaBufferDescription = IMGPAR m_pDxvaBufferDescription;
	AMVABUFFERINFO				 *m_pBufferInfo						= IMGPAR m_pBufferInfo;		

#if defined(_HW_Int_Mv_)
	if(IMGPAR type == B_SLICE && ((IMGPAR smart_dec & SMART_DEC_INT_PEL_Y) || (IMGPAR smart_dec & SMART_DEC_INT_PEL_UV)))
		IMGPAR m_iFp_Idx = 1;
	else
#endif
		IMGPAR m_iFp_Idx = 0;
	EnterCriticalSection( &crit_PICDEC );

	DEBUG_SHOW_HW_INFO("build_picture_decode_buffer : Start1");

	if(stream_global->m_is_MTMS)
	{
		if(m_bFirst_Slice && (IMGPAR structure == FRAME || pic_combine_status!=0))
			m_nFrameIndex = (int)m_pUncompBufQueue->GetItem();
		if(!m_bFirst_Slice)
		{
			IMGPAR m_pnv1PictureDecode = m_pbPictureParamBuf[m_nLastPicBufIdx];
			IMGPAR pic_dec_buf_idx = m_nLastPicBufIdx;
			if(!((IMGPAR smart_dec & SMART_DEC_NODEBLK_ALL) || ((IMGPAR smart_dec & SMART_DEC_NODEBLK_B) && (IMGPAR currentSlice->picture_type == B_SLICE))))
			{
				IMGPAR m_lpDEBLK_Luma = m_pbDeblockingCtrlBuf[m_nLastPicBufIdx];
				if(IMGPAR MbaffFrameFlag)
					IMGPAR m_lpDEBLK_Chroma = m_pbDeblockingCtrlBuf[m_nLastPicBufIdx] + IMGPAR PicSizeInMbs * sizeof(EdgeDataLumaMBAFF) * 2;
				else
					IMGPAR m_lpDEBLK_Chroma = m_pbDeblockingCtrlBuf[m_nLastPicBufIdx] + IMGPAR PicSizeInMbs * sizeof(EdgeDataLuma) * 2;
			}
		}
		IMGPAR UnCompress_Idx = m_nFrameIndex;
	}
	else
	{
		if(IMGPAR structure == FRAME || pic_combine_status!=0)
		{
			m_nFrameIndex = (int)m_pUncompBufQueue->GetItem();
			IMGPAR UnCompress_Idx = m_nFrameIndex;
		}
	}

	DEBUG_SHOW_HW_INFO("build_picture_decode_buffer : Finish1");

	if(m_bFirst_Slice)
	{
		if(stream_global->m_is_MTMS)
			m_nFirstDecBuf = 0;

		HRESULT hr = S_FALSE;
		int i = m_nLastPicBufIdx + 1;
		if(i == m_dwNumPicDecBuffers)
			i = 0;

		DEBUG_SHOW_HW_INFO("build_picture_decode_buffer : Start2");

		while(1)
		{
			if(m_bPicCompBufStaus[i])
				hr = m_pIHVDServiceDxva1->QueryRenderStatus(DXVA_PICTURE_DECODE_BUFFER,i,0);
			if(hr == S_OK)
			{
				IMGPAR m_pnv1PictureDecode = m_pbPictureParamBuf[i];
				IMGPAR pic_dec_buf_idx = i;
				if(!((IMGPAR smart_dec & SMART_DEC_NODEBLK_ALL) || ((IMGPAR smart_dec & SMART_DEC_NODEBLK_B) && (IMGPAR currentSlice->picture_type == B_SLICE))))
				{
					IMGPAR m_lpDEBLK_Luma = m_pbDeblockingCtrlBuf[i];
					if(IMGPAR MbaffFrameFlag)
						IMGPAR m_lpDEBLK_Chroma = m_pbDeblockingCtrlBuf[i] + IMGPAR PicSizeInMbs * sizeof(EdgeDataLumaMBAFF) * 2;
					else
						IMGPAR m_lpDEBLK_Chroma = m_pbDeblockingCtrlBuf[i] + IMGPAR PicSizeInMbs * sizeof(EdgeDataLuma) * 2;
				}
				m_bPicCompBufStaus[i] = FALSE;
				m_nLastPicBufIdx = i;

				if(!stream_global->m_is_MTMS)
					LeaveCriticalSection( &crit_PICDEC );

				break;
			}
			else if(++i == m_dwNumPicDecBuffers)
				i = 0;
		}

		DEBUG_SHOW_HW_INFO("build_picture_decode_buffer : Finish2");

		m_pnv1PictureDecode = (nvh264vp1PictureDecode*)IMGPAR m_pnv1PictureDecode;

		// fill out NV1 picture decode buffer 
		if(!m_pnv1PictureDecode || !dec_picture)
			return -1;
		memset(m_pnv1PictureDecode,0,sizeof(*m_pnv1PictureDecode));
		m_pnv1PictureDecode->ulFrameIndex	= IMGPAR UnCompress_Idx;
		m_pnv1PictureDecode->ulFlags		= 0;
		m_pnv1PictureDecode->ulPicHeight	= IMGPAR FrameHeightInMbs*16;
		m_pnv1PictureDecode->ulPicWidth		= active_sps.Valid?((int)active_sps.pic_width_in_mbs_minus1+1)*16:480;
		// bit 0 = frame(0) or field(1)
		// bit 1 = top(0) or bottom(1) field
		// bit 2 = MBAFF(1)
		switch(dec_picture->structure)
		{
		case FRAME:
			m_pnv1PictureDecode->ulFlags	= 0;
			break;
		case TOP_FIELD:
			m_pnv1PictureDecode->ulFlags	= 1; 
			m_pnv1PictureDecode->ulPicHeight >>=1;
			break;
		case BOTTOM_FIELD:
			m_pnv1PictureDecode->ulFlags	= 3;
			m_pnv1PictureDecode->ulPicHeight >>=1;
			break;
		}
		if(dec_picture->MbaffFrameFlag)
			m_pnv1PictureDecode->ulFlags	|= 4;

		// backup surface infomation
		SetSurfaceInfo ARGS3(dec_picture->unique_id,m_pnv1PictureDecode->ulFlags==3?1:0,1);
	}

	if(stream_global->m_is_MTMS)
	{
		m_bFirst_Slice = FALSE;
		LeaveCriticalSection( &crit_PICDEC );
	}

	dec_picture->pic_store_idx = IMGPAR UnCompress_Idx;	
	// fill out buffer description table for DXVA_PICTURE_DECODE_BUFFER
	memset(&m_pDxvaBufferDescription[0], 0, sizeof(DXVA_BufferDescription));
	m_pDxvaBufferDescription[0].dwTypeIndex		= m_pBufferInfo[0].dwTypeIndex	= DXVA_PICTURE_DECODE_BUFFER;
	m_pDxvaBufferDescription[0].dwBufferIndex	= m_pBufferInfo[0].dwBufferIndex= IMGPAR pic_dec_buf_idx;
	m_pDxvaBufferDescription[0].dwDataSize		= m_pBufferInfo[0].dwDataSize	= sizeof(*m_pnv1PictureDecode);
	return 0;
}

int CH264DXVA1_NV::build_slice_parameter_buffer PARGS0()
{
	return 0;
}

int CH264DXVA1_NV::ExecuteBuffers PARGS1(DWORD lInputFlag)
{
	DWORD m_dwRetValue;
	HRESULT	hr=0;
	int smart_execute =(SMART_DEC_SKIP_FIL_B | SMART_DEC_SKIP_HW_CR);
	int smart_execute_luma = (IMGPAR smart_dec & smart_execute);
	int smart_execute_chroma = (smart_execute_luma == SMART_DEC_SKIP_FIL_B ||(smart_execute_luma == SMART_DEC_SKIP_HW_CR && IMGPAR m_lFrame_Counter < (m_nSurfaceFrame<<2)));
	int	bf_num = 3-m_nFirstDecBuf;

	DXVA_BufferDescription *m_pDxvaBufferDescription = IMGPAR m_pDxvaBufferDescription;
	AMVABUFFERINFO				 *m_pBufferInfo						= IMGPAR m_pBufferInfo;							

	if(!stream_global->m_iStop_Decode)
	{
		hr = BeginFrame(IMGPAR UnCompress_Idx, 0);
		if(FAILED(hr))
			stream_global->m_iStop_Decode = 1;
	}

	if(!stream_global->m_iStop_Decode)
	{
		if((m_pIviCP != NULL) && (m_pIviCP->IsScramblingRequired(MAP_CP_PIC_TYPE(IMGPAR firstSlice->picture_type))))
		{
			m_pIviCP->EnableScrambling();
			if(IMGPAR m_lmbCount_Inter)
			{
				//for luma
				memset(&m_pDxvaBufferDescription[1], 0, 2*sizeof(DXVA_BufferDescription));
				// Macroblock control buffer description
				m_pDxvaBufferDescription[1].dwTypeIndex			= m_pBufferInfo[1].dwTypeIndex	= DXVA_MACROBLOCK_CONTROL_BUFFER;
				m_pDxvaBufferDescription[1].dwBufferIndex		= m_pBufferInfo[1].dwBufferIndex= IMGPAR m_InterL_lCompBufIndex;
				m_pDxvaBufferDescription[1].dwDataSize			= m_pBufferInfo[1].dwDataSize	= (int)(IMGPAR m_lpMBLK_Inter_Luma - m_pbMacroblockCtrlBuf[IMGPAR m_InterL_lCompBufIndex]);
				m_pDxvaBufferDescription[1].dwFirstMBaddress	= IMGPAR m_lFirstMBAddress_Inter;
				m_pDxvaBufferDescription[1].dwNumMBsInBuffer	= (unsigned short)IMGPAR m_lmbCount_Inter;
				// Block data buffer description
				m_pDxvaBufferDescription[2].dwTypeIndex			= m_pBufferInfo[2].dwTypeIndex	= DXVA_RESIDUAL_DIFFERENCE_BUFFER;
				m_pDxvaBufferDescription[2].dwBufferIndex		= m_pBufferInfo[2].dwBufferIndex= IMGPAR m_InterL_lCompBufIndex;
				m_pDxvaBufferDescription[2].dwDataSize			= m_pBufferInfo[2].dwDataSize	= (int)(IMGPAR m_lpRESD_Inter_Luma - m_pbResidualDiffBuf[IMGPAR m_InterL_lCompBufIndex]);
				m_pDxvaBufferDescription[2].dwFirstMBaddress	= IMGPAR m_lFirstMBAddress_Inter;
				m_pDxvaBufferDescription[2].dwNumMBsInBuffer	= (unsigned short)IMGPAR m_lmbCount_Inter;
				// call Execute and send buffers to driver
				//__memset(IMGPAR m_lpMBLK_Inter_Luma,0,(m_pCompBufferInfo[2].dwBytesToAllocate-m_pBufferInfo[1].dwDataSize));
				//__memset(IMGPAR m_lpRESD_Inter_Luma,0,(m_pCompBufferInfo[3].dwBytesToAllocate-m_pBufferInfo[2].dwDataSize));
				DUMP_NVIDIA((BYTE*)m_pDxvaBufferDescription, 3*sizeof(DXVA_BufferDescription), IMGPAR m_lFrame_Counter, "FAL_0");
				DUMP_NVIDIA(m_pbMacroblockCtrlBuf[IMGPAR m_InterL_lCompBufIndex], m_pDxvaBufferDescription[1].dwDataSize, IMGPAR m_lFrame_Counter, "InterL-MC_0");
				DUMP_NVIDIA(m_pbResidualDiffBuf[IMGPAR m_InterL_lCompBufIndex], m_pDxvaBufferDescription[2].dwDataSize, IMGPAR m_lFrame_Counter, "InterL-RES_0");
				m_pIviCP->ScrambleData(m_pbResidualDiffBuf[IMGPAR m_InterL_lCompBufIndex], m_pbResidualDiffBuf[IMGPAR m_InterL_lCompBufIndex], m_pDxvaBufferDescription[2].dwDataSize);

				hr |= Execute(0x01000000, &m_pDxvaBufferDescription[m_nFirstDecBuf], bf_num*sizeof(DXVA_BufferDescription), &m_dwRetValue, 4, bf_num, &m_pBufferInfo[m_nFirstDecBuf]);

				if(smart_execute_luma && IMGPAR currentSlice->picture_type == B_SLICE && IMGPAR currentSlice->structure != FRAME)
				{
					((nvh264vp1PictureDecode*)IMGPAR m_pnv1PictureDecode)->ulFlags = (pic_combine_status == TOP_FIELD ? 3 :1);
					hr |= Execute(0x01000000, &m_pDxvaBufferDescription[m_nFirstDecBuf], bf_num*sizeof(DXVA_BufferDescription), &m_dwRetValue, 4, bf_num, &m_pBufferInfo[m_nFirstDecBuf]);
					((nvh264vp1PictureDecode*)IMGPAR m_pnv1PictureDecode)->ulFlags = (pic_combine_status == TOP_FIELD ? 1 : 3);
					m_nFirstDecBuf = 0;
				}
				else
					m_nFirstDecBuf = 1;
				bf_num = 3-m_nFirstDecBuf;

				//for chroma
				memset(&m_pDxvaBufferDescription[1], 0, 2*sizeof(DXVA_BufferDescription));
				// Macroblock control buffer description
				m_pDxvaBufferDescription[1].dwTypeIndex			= m_pBufferInfo[1].dwTypeIndex	= DXVA_MACROBLOCK_CONTROL_BUFFER;
				m_pDxvaBufferDescription[1].dwBufferIndex		= m_pBufferInfo[1].dwBufferIndex= IMGPAR m_InterC_lCompBufIndex;
				m_pDxvaBufferDescription[1].dwDataSize			= m_pBufferInfo[1].dwDataSize	= (int)(IMGPAR m_lpMBLK_Inter_Chroma - m_pbMacroblockCtrlBuf[IMGPAR m_InterC_lCompBufIndex]);
				m_pDxvaBufferDescription[1].dwFirstMBaddress	= IMGPAR m_lFirstMBAddress_Inter;
				m_pDxvaBufferDescription[1].dwNumMBsInBuffer	= (unsigned short)IMGPAR m_lmbCount_Inter;
				// Block data buffer description
				m_pDxvaBufferDescription[2].dwTypeIndex			= m_pBufferInfo[2].dwTypeIndex	= DXVA_RESIDUAL_DIFFERENCE_BUFFER;
				m_pDxvaBufferDescription[2].dwBufferIndex		= m_pBufferInfo[2].dwBufferIndex= IMGPAR m_InterC_lCompBufIndex;
				m_pDxvaBufferDescription[2].dwDataSize			= m_pBufferInfo[2].dwDataSize	= (int)(IMGPAR m_lpRESD_Inter_Chroma - m_pbResidualDiffBuf[IMGPAR m_InterC_lCompBufIndex]);
				m_pDxvaBufferDescription[2].dwFirstMBaddress	= IMGPAR m_lFirstMBAddress_Inter;
				m_pDxvaBufferDescription[2].dwNumMBsInBuffer	= (unsigned short)IMGPAR m_lmbCount_Inter;
				// call Execute and send buffers to driver
				//__memset(IMGPAR m_lpMBLK_Inter_Chroma,0,(m_pCompBufferInfo[2].dwBytesToAllocate-m_pBufferInfo[1].dwDataSize));
				//__memset(IMGPAR m_lpRESD_Inter_Chroma,0,(m_pCompBufferInfo[3].dwBytesToAllocate-m_pBufferInfo[2].dwDataSize));
				DUMP_NVIDIA((BYTE*)m_pDxvaBufferDescription, 3*sizeof(DXVA_BufferDescription), IMGPAR m_lFrame_Counter, "FAL_1");
				DUMP_NVIDIA(m_pbMacroblockCtrlBuf[IMGPAR m_InterC_lCompBufIndex], m_pDxvaBufferDescription[1].dwDataSize, IMGPAR m_lFrame_Counter, "InterC-MC_1");
				DUMP_NVIDIA(m_pbResidualDiffBuf[IMGPAR m_InterC_lCompBufIndex], m_pDxvaBufferDescription[2].dwDataSize, IMGPAR m_lFrame_Counter, "InterC-RES_1");
				m_pIviCP->ScrambleData(m_pbResidualDiffBuf[IMGPAR m_InterC_lCompBufIndex], m_pbResidualDiffBuf[IMGPAR m_InterC_lCompBufIndex], m_pDxvaBufferDescription[2].dwDataSize);

				hr |= Execute(0x01000000, &m_pDxvaBufferDescription[m_nFirstDecBuf], bf_num*sizeof(DXVA_BufferDescription), &m_dwRetValue, 4, bf_num, &m_pBufferInfo[m_nFirstDecBuf]);

				if(smart_execute_chroma && (IMGPAR currentSlice->picture_type == B_SLICE && IMGPAR currentSlice->structure != FRAME))
				{
					((nvh264vp1PictureDecode*)IMGPAR m_pnv1PictureDecode)->ulFlags = (pic_combine_status == TOP_FIELD ? 3 :1);
					hr |= Execute(0x01000000, &m_pDxvaBufferDescription[m_nFirstDecBuf], bf_num*sizeof(DXVA_BufferDescription), &m_dwRetValue, 4, bf_num, &m_pBufferInfo[m_nFirstDecBuf]);
					((nvh264vp1PictureDecode*)IMGPAR m_pnv1PictureDecode)->ulFlags = (pic_combine_status == TOP_FIELD ? 1 : 3);
				}
				m_bCompBufStaus[IMGPAR m_InterC_lCompBufIndex] = TRUE;

				IMGPAR m_lmbCount_Inter = 0;
				IMGPAR m_lFirstMBAddress_Inter = -1;
				m_bCompBufStaus[IMGPAR m_InterL_lCompBufIndex] = TRUE;
				IMGPAR m_lpMBLK_Inter_Luma = NULL;
				IMGPAR m_iInterMCBufUsage = IMGPAR m_iInterRESBufUsage_L = IMGPAR m_iInterRESBufUsage_C = 0;
			}

			if(IMGPAR m_lmbCount_Intra && lInputFlag)
			{
				//for luma
				memset(&m_pDxvaBufferDescription[1], 0, 2*sizeof(DXVA_BufferDescription));
				// Macroblock control buffer description
				m_pDxvaBufferDescription[1].dwTypeIndex			= m_pBufferInfo[1].dwTypeIndex	= DXVA_MACROBLOCK_CONTROL_BUFFER;
				m_pDxvaBufferDescription[1].dwBufferIndex		= m_pBufferInfo[1].dwBufferIndex= IMGPAR m_IntraL_lCompBufIndex;
				m_pDxvaBufferDescription[1].dwDataSize			= m_pBufferInfo[1].dwDataSize	= (int)(IMGPAR m_lpMBLK_Intra_Luma - m_pbMacroblockCtrlBuf[IMGPAR m_IntraL_lCompBufIndex]);
				m_pDxvaBufferDescription[1].dwFirstMBaddress	= IMGPAR m_lFirstMBAddress_Intra;
				m_pDxvaBufferDescription[1].dwNumMBsInBuffer	= (unsigned short)IMGPAR m_lmbCount_Intra;
				// Block data buffer description
				m_pDxvaBufferDescription[2].dwTypeIndex			= m_pBufferInfo[2].dwTypeIndex	= DXVA_RESIDUAL_DIFFERENCE_BUFFER;
				m_pDxvaBufferDescription[2].dwBufferIndex		= m_pBufferInfo[2].dwBufferIndex= IMGPAR m_IntraL_lCompBufIndex;
				m_pDxvaBufferDescription[2].dwDataSize			= m_pBufferInfo[2].dwDataSize	= (int)(IMGPAR m_lpRESD_Intra_Luma - m_pbResidualDiffBuf[IMGPAR m_IntraL_lCompBufIndex]);
				m_pDxvaBufferDescription[2].dwFirstMBaddress	= IMGPAR m_lFirstMBAddress_Intra;
				m_pDxvaBufferDescription[2].dwNumMBsInBuffer	= (unsigned short)IMGPAR m_lmbCount_Intra;
				// call Execute and send buffers to driver
				//	__memset(IMGPAR m_lpMBLK_Intra_Luma,0,(m_pCompBufferInfo[2].dwBytesToAllocate-m_pBufferInfo[1].dwDataSize));
				//	__memset(IMGPAR m_lpRESD_Intra_Luma,0,(m_pCompBufferInfo[3].dwBytesToAllocate-m_pBufferInfo[2].dwDataSize));
				DUMP_NVIDIA((BYTE*)m_pDxvaBufferDescription, 3*sizeof(DXVA_BufferDescription), IMGPAR m_lFrame_Counter, "FAL_2");
				DUMP_NVIDIA(m_pbMacroblockCtrlBuf[IMGPAR m_IntraL_lCompBufIndex], m_pDxvaBufferDescription[1].dwDataSize, IMGPAR m_lFrame_Counter, "InterL-MC_2");
				DUMP_NVIDIA(m_pbResidualDiffBuf[IMGPAR m_IntraL_lCompBufIndex], m_pDxvaBufferDescription[2].dwDataSize, IMGPAR m_lFrame_Counter, "InterL-RES_2");
				m_pIviCP->ScrambleData(m_pbResidualDiffBuf[IMGPAR m_IntraL_lCompBufIndex], m_pbResidualDiffBuf[IMGPAR m_IntraL_lCompBufIndex], m_pDxvaBufferDescription[2].dwDataSize);

				hr |= Execute(0x01000000, &m_pDxvaBufferDescription[m_nFirstDecBuf], bf_num*sizeof(DXVA_BufferDescription), &m_dwRetValue, 4, bf_num, &m_pBufferInfo[m_nFirstDecBuf]);

				if(smart_execute_luma && IMGPAR currentSlice->picture_type == B_SLICE && IMGPAR currentSlice->structure != FRAME)
				{
					((nvh264vp1PictureDecode*)IMGPAR m_pnv1PictureDecode)->ulFlags = (pic_combine_status == TOP_FIELD ? 3 :1);
					hr |= Execute(0x01000000, &m_pDxvaBufferDescription[m_nFirstDecBuf], bf_num*sizeof(DXVA_BufferDescription), &m_dwRetValue, 4, bf_num, &m_pBufferInfo[m_nFirstDecBuf]);
					((nvh264vp1PictureDecode*)IMGPAR m_pnv1PictureDecode)->ulFlags = (pic_combine_status == TOP_FIELD ? 1 : 3);
					m_nFirstDecBuf = 0;
				}
				else
					m_nFirstDecBuf = 1;

				bf_num = 3-m_nFirstDecBuf;

				//for chroma
				memset(&m_pDxvaBufferDescription[1], 0, 2*sizeof(DXVA_BufferDescription));
				// Macroblock control buffer description
				m_pDxvaBufferDescription[1].dwTypeIndex			= m_pBufferInfo[1].dwTypeIndex	= DXVA_MACROBLOCK_CONTROL_BUFFER;
				m_pDxvaBufferDescription[1].dwBufferIndex		= m_pBufferInfo[1].dwBufferIndex= IMGPAR m_IntraC_lCompBufIndex;
				m_pDxvaBufferDescription[1].dwDataSize			= m_pBufferInfo[1].dwDataSize	= (int)(IMGPAR m_lpMBLK_Intra_Chroma - m_pbMacroblockCtrlBuf[IMGPAR m_IntraC_lCompBufIndex]);
				m_pDxvaBufferDescription[1].dwFirstMBaddress	= IMGPAR m_lFirstMBAddress_Intra;
				m_pDxvaBufferDescription[1].dwNumMBsInBuffer	= (unsigned short)IMGPAR m_lmbCount_Intra;
				// Block data buffer description
				m_pDxvaBufferDescription[2].dwTypeIndex			= m_pBufferInfo[2].dwTypeIndex	= DXVA_RESIDUAL_DIFFERENCE_BUFFER;
				m_pDxvaBufferDescription[2].dwBufferIndex		= m_pBufferInfo[2].dwBufferIndex= IMGPAR m_IntraC_lCompBufIndex;
				m_pDxvaBufferDescription[2].dwDataSize			= m_pBufferInfo[2].dwDataSize	= (int)(IMGPAR m_lpRESD_Intra_Chroma - m_pbResidualDiffBuf[IMGPAR m_IntraC_lCompBufIndex]);
				m_pDxvaBufferDescription[2].dwFirstMBaddress	= IMGPAR m_lFirstMBAddress_Intra;
				m_pDxvaBufferDescription[2].dwNumMBsInBuffer	= (unsigned short)IMGPAR m_lmbCount_Intra;
				// call Execute and send buffers to driver
				//	__memset(IMGPAR m_lpMBLK_Intra_Chroma,0,(m_pCompBufferInfo[2].dwBytesToAllocate-m_pBufferInfo[1].dwDataSize));
				//	__memset(IMGPAR m_lpRESD_Intra_Chroma,0,(m_pCompBufferInfo[3].dwBytesToAllocate-m_pBufferInfo[2].dwDataSize));
				DUMP_NVIDIA((BYTE*)m_pDxvaBufferDescription, 3*sizeof(DXVA_BufferDescription), IMGPAR m_lFrame_Counter, "FAL_3");
				DUMP_NVIDIA(m_pbMacroblockCtrlBuf[IMGPAR m_IntraC_lCompBufIndex], m_pDxvaBufferDescription[1].dwDataSize, IMGPAR m_lFrame_Counter, "InterC-MC_3");
				DUMP_NVIDIA(m_pbResidualDiffBuf[IMGPAR m_IntraC_lCompBufIndex], m_pDxvaBufferDescription[2].dwDataSize, IMGPAR m_lFrame_Counter, "InterC-RES_3");
				m_pIviCP->ScrambleData(m_pbResidualDiffBuf[IMGPAR m_IntraC_lCompBufIndex], m_pbResidualDiffBuf[IMGPAR m_IntraC_lCompBufIndex], m_pDxvaBufferDescription[2].dwDataSize);

				hr |= Execute(0x01000000, &m_pDxvaBufferDescription[m_nFirstDecBuf], bf_num*sizeof(DXVA_BufferDescription), &m_dwRetValue, 4, bf_num, &m_pBufferInfo[m_nFirstDecBuf]);

				if(smart_execute_chroma && (IMGPAR currentSlice->picture_type == B_SLICE && IMGPAR currentSlice->structure != FRAME))
				{
					((nvh264vp1PictureDecode*)IMGPAR m_pnv1PictureDecode)->ulFlags = (pic_combine_status == TOP_FIELD ? 3 :1);
					hr |= Execute(0x01000000, &m_pDxvaBufferDescription[m_nFirstDecBuf], bf_num*sizeof(DXVA_BufferDescription), &m_dwRetValue, 4, bf_num, &m_pBufferInfo[m_nFirstDecBuf]);
					((nvh264vp1PictureDecode*)IMGPAR m_pnv1PictureDecode)->ulFlags = (pic_combine_status == TOP_FIELD ? 1: 3);
				}
				m_bCompBufStaus[IMGPAR m_IntraC_lCompBufIndex] = TRUE;

				IMGPAR m_lmbCount_Intra = 0;
				IMGPAR m_lFirstMBAddress_Intra = -1;
				m_bCompBufStaus[IMGPAR m_IntraL_lCompBufIndex] = TRUE;
				IMGPAR m_lpMBLK_Intra_Luma = NULL;
				IMGPAR m_iIntraMCBufUsage = IMGPAR m_iIntraRESBufUsage = 0;
			}

			//Deblocking begin
			if(lInputFlag&E_USING_DEBLOCK)
			{
				if((IMGPAR smart_dec & SMART_DEC_NODEBLK_ALL) || ((IMGPAR smart_dec & SMART_DEC_NODEBLK_B) && (IMGPAR currentSlice->picture_type == B_SLICE)))
					m_bPicCompBufStaus[IMGPAR pic_dec_buf_idx] = TRUE;
				else if(!stream_global->m_is_MTMS)
				{
					memset(&m_pDxvaBufferDescription[3], 0, sizeof(DXVA_BufferDescription));
					m_pDxvaBufferDescription[3].dwBufferIndex = m_pBufferInfo[3].dwBufferIndex = IMGPAR pic_dec_buf_idx;
					m_pDxvaBufferDescription[3].dwDataSize = m_pBufferInfo[3].dwDataSize = IMGPAR m_lpDEBLK_Chroma - m_pbDeblockingCtrlBuf[IMGPAR pic_dec_buf_idx];
					m_pDxvaBufferDescription[3].dwDataOffset = 0;
					m_pDxvaBufferDescription[3].dwFirstMBaddress = 0;
					m_pDxvaBufferDescription[3].dwNumMBsInBuffer = 0;
					m_pDxvaBufferDescription[3].dwStride = 0;
					m_pDxvaBufferDescription[3].dwTypeIndex = m_pBufferInfo[3].dwTypeIndex = DXVA_DEBLOCKING_CONTROL_BUFFER;
					m_pDxvaBufferDescription[3].dwWidth = 0;
					m_pDxvaBufferDescription[3].dwHeight = IMGPAR FrameHeightInMbs*16;
					m_pBufferInfo[3].dwDataOffset = 0;

					//if(dec_picture->structure == TOP_FIELD || dec_picture->structure == BOTTOM_FIELD)
					//	m_pDxvaBufferDescription[3].dwHeight >>= 1;

					//Dump ARGS3(m_lpDEBLKBuf[IMGPAR pic_dec_buf_idx], m_pDxvaBufferDescription[3].dwDataSize, "DEBLOCK.dat");
					hr = Execute(DXVA_PICTURE_DECODING_FUNCTION << 24, &m_pDxvaBufferDescription[3], sizeof(DXVA_BufferDescription), &m_dwRetValue, 4, 1, &m_pBufferInfo[3]);
					m_bPicCompBufStaus[IMGPAR pic_dec_buf_idx] = TRUE;
				}
			}

			if(!stream_global->m_is_MTMS)
				m_nFirstDecBuf = 0;

			EndFrame(IMGPAR UnCompress_Idx);
		}
		else
		{
			if(m_pIviCP != NULL)
				m_pIviCP->DisableScrambling();

			if(IMGPAR m_lmbCount_Inter)
			{
				//for luma
				memset(&m_pDxvaBufferDescription[1], 0, 2*sizeof(DXVA_BufferDescription));
				// Macroblock control buffer description
				m_pDxvaBufferDescription[1].dwTypeIndex			= m_pBufferInfo[1].dwTypeIndex	= DXVA_MACROBLOCK_CONTROL_BUFFER;
				m_pDxvaBufferDescription[1].dwBufferIndex		= m_pBufferInfo[1].dwBufferIndex= IMGPAR m_InterL_lCompBufIndex;
				m_pDxvaBufferDescription[1].dwDataSize			= m_pBufferInfo[1].dwDataSize	= (int)(IMGPAR m_lpMBLK_Inter_Luma - m_pbMacroblockCtrlBuf[IMGPAR m_InterL_lCompBufIndex]);
				m_pDxvaBufferDescription[1].dwFirstMBaddress	= IMGPAR m_lFirstMBAddress_Inter;
				m_pDxvaBufferDescription[1].dwNumMBsInBuffer	= (unsigned short)IMGPAR m_lmbCount_Inter;
				// Block data buffer description
				m_pDxvaBufferDescription[2].dwTypeIndex			= m_pBufferInfo[2].dwTypeIndex	= DXVA_RESIDUAL_DIFFERENCE_BUFFER;
				m_pDxvaBufferDescription[2].dwBufferIndex		= m_pBufferInfo[2].dwBufferIndex= IMGPAR m_InterL_lCompBufIndex;
				m_pDxvaBufferDescription[2].dwDataSize			= m_pBufferInfo[2].dwDataSize	= (int)(IMGPAR m_lpRESD_Inter_Luma - m_pbResidualDiffBuf[IMGPAR m_InterL_lCompBufIndex]);
				m_pDxvaBufferDescription[2].dwFirstMBaddress	= IMGPAR m_lFirstMBAddress_Inter;
				m_pDxvaBufferDescription[2].dwNumMBsInBuffer	= (unsigned short)IMGPAR m_lmbCount_Inter;
				// call Execute and send buffers to driver
				//__memset(IMGPAR m_lpMBLK_Inter_Luma,0,(m_pCompBufferInfo[2].dwBytesToAllocate-m_pBufferInfo[1].dwDataSize));
				//__memset(IMGPAR m_lpRESD_Inter_Luma,0,(m_pCompBufferInfo[3].dwBytesToAllocate-m_pBufferInfo[2].dwDataSize));
				DUMP_NVIDIA((BYTE*)m_pDxvaBufferDescription, 3*sizeof(DXVA_BufferDescription), IMGPAR m_lFrame_Counter, "FAL_0");
				DUMP_NVIDIA(m_pbMacroblockCtrlBuf[IMGPAR m_InterL_lCompBufIndex], m_pDxvaBufferDescription[1].dwDataSize, IMGPAR m_lFrame_Counter, "InterL-MC_0");
				DUMP_NVIDIA(m_pbResidualDiffBuf[IMGPAR m_InterL_lCompBufIndex], m_pDxvaBufferDescription[2].dwDataSize, IMGPAR m_lFrame_Counter, "InterL-RES_0");

				hr |= Execute(0x01000000, &m_pDxvaBufferDescription[m_nFirstDecBuf], bf_num*sizeof(DXVA_BufferDescription), &m_dwRetValue, 4, bf_num, &m_pBufferInfo[m_nFirstDecBuf]);

				if(smart_execute_luma && IMGPAR currentSlice->picture_type == B_SLICE && IMGPAR currentSlice->structure != FRAME)
				{
					((nvh264vp1PictureDecode*)IMGPAR m_pnv1PictureDecode)->ulFlags = (pic_combine_status == TOP_FIELD ? 3 :1);
					hr |= Execute(0x01000000, &m_pDxvaBufferDescription[m_nFirstDecBuf], bf_num*sizeof(DXVA_BufferDescription), &m_dwRetValue, 4, bf_num, &m_pBufferInfo[m_nFirstDecBuf]);
					((nvh264vp1PictureDecode*)IMGPAR m_pnv1PictureDecode)->ulFlags = (pic_combine_status == TOP_FIELD ? 1 : 3);
					m_nFirstDecBuf = 0;
				}
				else
					m_nFirstDecBuf = 1;

				bf_num = 3-m_nFirstDecBuf;


				//for chroma
				memset(&m_pDxvaBufferDescription[1], 0, 2*sizeof(DXVA_BufferDescription));
				// Macroblock control buffer description
				m_pDxvaBufferDescription[1].dwTypeIndex			= m_pBufferInfo[1].dwTypeIndex	= DXVA_MACROBLOCK_CONTROL_BUFFER;
				m_pDxvaBufferDescription[1].dwBufferIndex		= m_pBufferInfo[1].dwBufferIndex= IMGPAR m_InterC_lCompBufIndex;
				m_pDxvaBufferDescription[1].dwDataSize			= m_pBufferInfo[1].dwDataSize	= (int)(IMGPAR m_lpMBLK_Inter_Chroma - m_pbMacroblockCtrlBuf[IMGPAR m_InterC_lCompBufIndex]);
				m_pDxvaBufferDescription[1].dwFirstMBaddress	= IMGPAR m_lFirstMBAddress_Inter;
				m_pDxvaBufferDescription[1].dwNumMBsInBuffer	= (unsigned short)IMGPAR m_lmbCount_Inter;
				// Block data buffer description
				m_pDxvaBufferDescription[2].dwTypeIndex			= m_pBufferInfo[2].dwTypeIndex	= DXVA_RESIDUAL_DIFFERENCE_BUFFER;
				m_pDxvaBufferDescription[2].dwBufferIndex		= m_pBufferInfo[2].dwBufferIndex= IMGPAR m_InterC_lCompBufIndex;
				m_pDxvaBufferDescription[2].dwDataSize			= m_pBufferInfo[2].dwDataSize	= (int)(IMGPAR m_lpRESD_Inter_Chroma - m_pbResidualDiffBuf[IMGPAR m_InterC_lCompBufIndex]);
				m_pDxvaBufferDescription[2].dwFirstMBaddress	= IMGPAR m_lFirstMBAddress_Inter;
				m_pDxvaBufferDescription[2].dwNumMBsInBuffer	= (unsigned short)IMGPAR m_lmbCount_Inter;
				// call Execute and send buffers to driver
				//__memset(IMGPAR m_lpMBLK_Inter_Chroma,0,(m_pCompBufferInfo[2].dwBytesToAllocate-m_pBufferInfo[1].dwDataSize));
				//__memset(IMGPAR m_lpRESD_Inter_Chroma,0,(m_pCompBufferInfo[3].dwBytesToAllocate-m_pBufferInfo[2].dwDataSize));
				DUMP_NVIDIA((BYTE*)m_pDxvaBufferDescription, 3*sizeof(DXVA_BufferDescription), IMGPAR m_lFrame_Counter, "FAL_1");
				DUMP_NVIDIA(m_pbMacroblockCtrlBuf[IMGPAR m_InterC_lCompBufIndex], m_pDxvaBufferDescription[1].dwDataSize, IMGPAR m_lFrame_Counter, "InterC-MC_1");
				DUMP_NVIDIA(m_pbResidualDiffBuf[IMGPAR m_InterC_lCompBufIndex], m_pDxvaBufferDescription[2].dwDataSize, IMGPAR m_lFrame_Counter, "InterC-RES_1");

				hr |= Execute(0x01000000, &m_pDxvaBufferDescription[m_nFirstDecBuf], bf_num*sizeof(DXVA_BufferDescription), &m_dwRetValue, 4, bf_num, &m_pBufferInfo[m_nFirstDecBuf]);

				if(smart_execute_chroma && (IMGPAR currentSlice->picture_type == B_SLICE && IMGPAR currentSlice->structure != FRAME))
				{
					((nvh264vp1PictureDecode*)IMGPAR m_pnv1PictureDecode)->ulFlags = (pic_combine_status == TOP_FIELD ? 3 :1);
					hr |= Execute(0x01000000, &m_pDxvaBufferDescription[m_nFirstDecBuf], bf_num*sizeof(DXVA_BufferDescription), &m_dwRetValue, 4, bf_num, &m_pBufferInfo[m_nFirstDecBuf]);
					((nvh264vp1PictureDecode*)IMGPAR m_pnv1PictureDecode)->ulFlags = (pic_combine_status == TOP_FIELD ? 1 : 3);
				}
				m_bCompBufStaus[IMGPAR m_InterC_lCompBufIndex] = TRUE;


				IMGPAR m_lmbCount_Inter = 0;
				IMGPAR m_lFirstMBAddress_Inter = -1;
				m_bCompBufStaus[IMGPAR m_InterL_lCompBufIndex] = TRUE;
				IMGPAR m_lpMBLK_Inter_Luma = NULL;
				IMGPAR m_iInterMCBufUsage = IMGPAR m_iInterRESBufUsage_L = IMGPAR m_iInterRESBufUsage_C = 0;
			}

			if(IMGPAR m_lmbCount_Intra && lInputFlag)
			{
				//for luma
				memset(&m_pDxvaBufferDescription[1], 0, 2*sizeof(DXVA_BufferDescription));
				// Macroblock control buffer description
				m_pDxvaBufferDescription[1].dwTypeIndex			= m_pBufferInfo[1].dwTypeIndex	= DXVA_MACROBLOCK_CONTROL_BUFFER;
				m_pDxvaBufferDescription[1].dwBufferIndex		= m_pBufferInfo[1].dwBufferIndex= IMGPAR m_IntraL_lCompBufIndex;
				m_pDxvaBufferDescription[1].dwDataSize			= m_pBufferInfo[1].dwDataSize	= (int)(IMGPAR m_lpMBLK_Intra_Luma - m_pbMacroblockCtrlBuf[IMGPAR m_IntraL_lCompBufIndex]);
				m_pDxvaBufferDescription[1].dwFirstMBaddress	= IMGPAR m_lFirstMBAddress_Intra;
				m_pDxvaBufferDescription[1].dwNumMBsInBuffer	= (unsigned short)IMGPAR m_lmbCount_Intra;
				// Block data buffer description
				m_pDxvaBufferDescription[2].dwTypeIndex			= m_pBufferInfo[2].dwTypeIndex	= DXVA_RESIDUAL_DIFFERENCE_BUFFER;
				m_pDxvaBufferDescription[2].dwBufferIndex		= m_pBufferInfo[2].dwBufferIndex= IMGPAR m_IntraL_lCompBufIndex;
				m_pDxvaBufferDescription[2].dwDataSize			= m_pBufferInfo[2].dwDataSize	= (int)(IMGPAR m_lpRESD_Intra_Luma - m_pbResidualDiffBuf[IMGPAR m_IntraL_lCompBufIndex]);
				m_pDxvaBufferDescription[2].dwFirstMBaddress	= IMGPAR m_lFirstMBAddress_Intra;
				m_pDxvaBufferDescription[2].dwNumMBsInBuffer	= (unsigned short)IMGPAR m_lmbCount_Intra;
				// call Execute and send buffers to driver
				//	__memset(IMGPAR m_lpMBLK_Intra_Luma,0,(m_pCompBufferInfo[2].dwBytesToAllocate-m_pBufferInfo[1].dwDataSize));
				//	__memset(IMGPAR m_lpRESD_Intra_Luma,0,(m_pCompBufferInfo[3].dwBytesToAllocate-m_pBufferInfo[2].dwDataSize));
				DUMP_NVIDIA((BYTE*)m_pDxvaBufferDescription, 3*sizeof(DXVA_BufferDescription), IMGPAR m_lFrame_Counter, "FAL_2");
				DUMP_NVIDIA(m_pbMacroblockCtrlBuf[IMGPAR m_IntraL_lCompBufIndex], m_pDxvaBufferDescription[1].dwDataSize, IMGPAR m_lFrame_Counter, "InterL-MC_2");
				DUMP_NVIDIA(m_pbResidualDiffBuf[IMGPAR m_IntraL_lCompBufIndex], m_pDxvaBufferDescription[2].dwDataSize, IMGPAR m_lFrame_Counter, "InterL-RES_2");
				hr |= Execute(0x01000000, &m_pDxvaBufferDescription[m_nFirstDecBuf], bf_num*sizeof(DXVA_BufferDescription), &m_dwRetValue, 4, bf_num, &m_pBufferInfo[m_nFirstDecBuf]);

				if(smart_execute_luma && IMGPAR currentSlice->picture_type == B_SLICE && IMGPAR currentSlice->structure != FRAME)
				{
					((nvh264vp1PictureDecode*)IMGPAR m_pnv1PictureDecode)->ulFlags = (pic_combine_status == TOP_FIELD ? 3 :1);
					hr |= Execute(0x01000000, &m_pDxvaBufferDescription[m_nFirstDecBuf], bf_num*sizeof(DXVA_BufferDescription), &m_dwRetValue, 4, bf_num, &m_pBufferInfo[m_nFirstDecBuf]);
					((nvh264vp1PictureDecode*)IMGPAR m_pnv1PictureDecode)->ulFlags = (pic_combine_status == TOP_FIELD ? 1 : 3);
					m_nFirstDecBuf = 0;
				}
				else
					m_nFirstDecBuf = 1;

				bf_num = 3-m_nFirstDecBuf;

				//for chroma
				memset(&m_pDxvaBufferDescription[1], 0, 2*sizeof(DXVA_BufferDescription));
				// Macroblock control buffer description
				m_pDxvaBufferDescription[1].dwTypeIndex			= m_pBufferInfo[1].dwTypeIndex	= DXVA_MACROBLOCK_CONTROL_BUFFER;
				m_pDxvaBufferDescription[1].dwBufferIndex		= m_pBufferInfo[1].dwBufferIndex= IMGPAR m_IntraC_lCompBufIndex;
				m_pDxvaBufferDescription[1].dwDataSize			= m_pBufferInfo[1].dwDataSize	= (int)(IMGPAR m_lpMBLK_Intra_Chroma - m_pbMacroblockCtrlBuf[IMGPAR m_IntraC_lCompBufIndex]);
				m_pDxvaBufferDescription[1].dwFirstMBaddress	= IMGPAR m_lFirstMBAddress_Intra;
				m_pDxvaBufferDescription[1].dwNumMBsInBuffer	= (unsigned short)IMGPAR m_lmbCount_Intra;
				// Block data buffer description
				m_pDxvaBufferDescription[2].dwTypeIndex			= m_pBufferInfo[2].dwTypeIndex	= DXVA_RESIDUAL_DIFFERENCE_BUFFER;
				m_pDxvaBufferDescription[2].dwBufferIndex		= m_pBufferInfo[2].dwBufferIndex= IMGPAR m_IntraC_lCompBufIndex;
				m_pDxvaBufferDescription[2].dwDataSize			= m_pBufferInfo[2].dwDataSize	= (int)(IMGPAR m_lpRESD_Intra_Chroma - m_pbResidualDiffBuf[IMGPAR m_IntraC_lCompBufIndex]);
				m_pDxvaBufferDescription[2].dwFirstMBaddress	= IMGPAR m_lFirstMBAddress_Intra;
				m_pDxvaBufferDescription[2].dwNumMBsInBuffer	= (unsigned short)IMGPAR m_lmbCount_Intra;
				// call Execute and send buffers to driver
				//	__memset(IMGPAR m_lpMBLK_Intra_Chroma,0,(m_pCompBufferInfo[2].dwBytesToAllocate-m_pBufferInfo[1].dwDataSize));
				//	__memset(IMGPAR m_lpRESD_Intra_Chroma,0,(m_pCompBufferInfo[3].dwBytesToAllocate-m_pBufferInfo[2].dwDataSize));
				DUMP_NVIDIA((BYTE*)m_pDxvaBufferDescription, 3*sizeof(DXVA_BufferDescription), IMGPAR m_lFrame_Counter, "FAL_3");
				DUMP_NVIDIA(m_pbMacroblockCtrlBuf[IMGPAR m_IntraC_lCompBufIndex], m_pDxvaBufferDescription[1].dwDataSize, IMGPAR m_lFrame_Counter, "InterC-MC_3");
				DUMP_NVIDIA(m_pbResidualDiffBuf[IMGPAR m_IntraC_lCompBufIndex], m_pDxvaBufferDescription[2].dwDataSize, IMGPAR m_lFrame_Counter, "InterC-RES_3");

				hr |= Execute(0x01000000, &m_pDxvaBufferDescription[m_nFirstDecBuf], bf_num*sizeof(DXVA_BufferDescription), &m_dwRetValue, 4, bf_num, &m_pBufferInfo[m_nFirstDecBuf]);

				if(smart_execute_chroma && (IMGPAR currentSlice->picture_type == B_SLICE && IMGPAR currentSlice->structure != FRAME))
				{
					((nvh264vp1PictureDecode*)IMGPAR m_pnv1PictureDecode)->ulFlags = (pic_combine_status == TOP_FIELD ? 3 :1);
					hr |= Execute(0x01000000, &m_pDxvaBufferDescription[m_nFirstDecBuf], bf_num*sizeof(DXVA_BufferDescription), &m_dwRetValue, 4, bf_num, &m_pBufferInfo[m_nFirstDecBuf]);
					((nvh264vp1PictureDecode*)IMGPAR m_pnv1PictureDecode)->ulFlags = (pic_combine_status == TOP_FIELD ? 1: 3);
				}
				m_bCompBufStaus[IMGPAR m_IntraC_lCompBufIndex] = TRUE;

				IMGPAR m_lmbCount_Intra = 0;
				IMGPAR m_lFirstMBAddress_Intra = -1;
				m_bCompBufStaus[IMGPAR m_IntraL_lCompBufIndex] = TRUE;
				IMGPAR m_lpMBLK_Intra_Luma = NULL;
				IMGPAR m_iIntraMCBufUsage = IMGPAR m_iIntraRESBufUsage = 0;
			}

			if(lInputFlag & E_USING_DEBLOCK)
			{
				if((IMGPAR smart_dec & SMART_DEC_NODEBLK_ALL) || ((IMGPAR smart_dec & SMART_DEC_NODEBLK_B) && (IMGPAR currentSlice->picture_type == B_SLICE)))
					m_bPicCompBufStaus[IMGPAR pic_dec_buf_idx] = TRUE;
				else if(!stream_global->m_is_MTMS)
				{
					memset(&m_pDxvaBufferDescription[3], 0, sizeof(DXVA_BufferDescription));
					m_pDxvaBufferDescription[3].dwBufferIndex = m_pBufferInfo[3].dwBufferIndex = IMGPAR pic_dec_buf_idx;
					m_pDxvaBufferDescription[3].dwDataSize = m_pBufferInfo[3].dwDataSize = IMGPAR m_lpDEBLK_Chroma - m_pbDeblockingCtrlBuf[IMGPAR pic_dec_buf_idx];
					m_pDxvaBufferDescription[3].dwDataOffset = 0;
					m_pDxvaBufferDescription[3].dwFirstMBaddress = 0;
					m_pDxvaBufferDescription[3].dwNumMBsInBuffer = 0;
					m_pDxvaBufferDescription[3].dwStride = 0;
					m_pDxvaBufferDescription[3].dwTypeIndex = m_pBufferInfo[3].dwTypeIndex = DXVA_DEBLOCKING_CONTROL_BUFFER;
					m_pDxvaBufferDescription[3].dwWidth = 0;
					m_pDxvaBufferDescription[3].dwHeight = IMGPAR FrameHeightInMbs*16;
					m_pBufferInfo[3].dwDataOffset = 0;

					hr = Execute(DXVA_PICTURE_DECODING_FUNCTION << 24, &m_pDxvaBufferDescription[3], sizeof(DXVA_BufferDescription), &m_dwRetValue, 4, 1, &m_pBufferInfo[3]);
					m_bPicCompBufStaus[IMGPAR pic_dec_buf_idx] = TRUE;
				}
			}

			if(!stream_global->m_is_MTMS)
				m_nFirstDecBuf = 0;

			EndFrame(IMGPAR UnCompress_Idx);
		}
	}
	else
	{
		if(IMGPAR m_lmbCount_Inter)
		{
			IMGPAR m_lmbCount_Inter = 0;
			IMGPAR m_lFirstMBAddress_Inter = -1;
			m_bCompBufStaus[IMGPAR m_InterL_lCompBufIndex] = TRUE;
			IMGPAR m_lpMBLK_Inter_Luma = NULL;
			IMGPAR m_iInterMCBufUsage = IMGPAR m_iInterRESBufUsage_L = IMGPAR m_iInterRESBufUsage_C = 0;
		}

		if(IMGPAR m_lmbCount_Intra && lInputFlag)
		{
			//for luma
			IMGPAR m_lmbCount_Intra = 0;
			IMGPAR m_lFirstMBAddress_Intra = -1;
			m_bCompBufStaus[IMGPAR m_IntraL_lCompBufIndex] = TRUE;
			IMGPAR m_lpMBLK_Intra_Luma = NULL;
			IMGPAR m_iIntraMCBufUsage = IMGPAR m_iIntraRESBufUsage = 0;
		}

		if(lInputFlag & E_USING_DEBLOCK)
			m_bPicCompBufStaus[IMGPAR pic_dec_buf_idx] = TRUE;
	}

	return SUCCEEDED(hr)?0:-1;
}

void CH264DXVA1_NV::DMA_Transfer PARGS0()
{
	int i;
	int width,offset;
	int height = (!IMGPAR structure ? IMGPAR height : IMGPAR height>>1);
	BYTE *raw;
	imgpel *imgy = dec_picture->imgY;
	imgpel *imguv = dec_picture->imgUV;
	HRESULT	hr=0;

	long l_lStride;

	if(IMGPAR structure == FRAME || pic_combine_status!=0)
		m_nFrameIndex = (int)m_pUncompBufQueue->GetItem();
	dec_picture->pic_store_idx = IMGPAR UnCompress_Idx = m_nFrameIndex;
	SetSurfaceInfo ARGS3(dec_picture->unique_id, (IMGPAR structure== BOTTOM_FIELD?1:0),1 );

	while(1)
	{
		hr=BeginFrame(IMGPAR UnCompress_Idx, 0);
		if(checkDDError(hr))
			Sleep(2);
		else
			break;
	}
	if(SUCCEEDED(hr))
	{
		hr |= GetBuffer(-1, IMGPAR UnCompress_Idx, FALSE, (void**)&raw, &l_lStride);	 
		if(IMGPAR structure)
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

		for(i=0 ; i<height; i++)
		{
			memcpy(raw,imgy,IMGPAR width);
			raw += width;
			imgy += dec_picture->Y_stride;
		}

		for(i=0 ; i<(height>>1); i++)
		{
			memcpy(raw,imguv,IMGPAR width);
			raw += (offset + l_lStride);
			imguv += dec_picture->UV_stride;			
		}

		hr |= ReleaseBuffer(-1, IMGPAR UnCompress_Idx);
		hr |= EndFrame(IMGPAR UnCompress_Idx);
	}
}

void CH264DXVA1_NV::DeblockSlice PARGS3(StorablePicture *p, int start_mb, int num_mb)
{
	int i, mbx, mby, PicHeightInMbs=p->PicSizeInMbs/p->PicWidthInMbs;
	Macroblock_s	*MbQ, *MbTop, *MbLeft;
	HRESULT hr;
	//long l_lStride;

	//	for(i=0; i<(int)m_pCompBufferInfo[DXVA_DEBLOCKING_CONTROL_BUFFER].dwNumCompBuffers; i++)
	//		hr = GetBuffer(DXVA_DEBLOCKING_CONTROL_BUFFER, i, FALSE, (void**)&m_lpDEBLKBuf[i], &l_lStride);

	//	if(deblocking_flag)
	//	{
	//		IMGPAR m_lpDEBLK_Luma = m_lpDEBLKBuf[0];
	//	if(dec_picture->MbaffFrameFlag)
	//		IMGPAR m_lpDEBLK_Chroma = m_lpDEBLKBuf[0] + p->PicSizeInMbs * sizeof(EdgeDataLumaMBAFF) * 2;
	//	else
	//		IMGPAR m_lpDEBLK_Chroma = m_lpDEBLKBuf[0] + p->PicSizeInMbs * sizeof(EdgeDataLuma) * 2;

	if(IMGPAR type == B_SLICE)
	{
		fp_GetStrength_v			= GetStrength_v;
		fp_GetStrengthInternal_v	= GetStrengthInternal_v;
		fp_GetStrength_h			= GetStrength_h;
		fp_GetStrengthInternal_h	= GetStrengthInternal_h;
//#if	 defined(H264_ENABLE_INTRINSICS)
		if((cpu_type == CPU_LEVEL_SSE2) || (cpu_type == CPU_LEVEL_SSE3))
		{
			fp_GetStrengthInternal_v_3 = GetStrengthInternal_v_3;
			fp_GetStrengthInternal_h_3 = GetStrengthInternal_h_3;
		}
//#endif
	}
	else
	{
		fp_GetStrength_v			= GetStrength_v_P;
		
		fp_GetStrength_h			= GetStrength_h_P;
		
//#if	 defined(H264_ENABLE_INTRINSICS)
		if((cpu_type == CPU_LEVEL_SSE2) || (cpu_type == CPU_LEVEL_SSE3))
		{
			fp_GetStrengthInternal_v_3 = GetStrengthInternal_v_P_3;
			fp_GetStrengthInternal_h_3 = GetStrengthInternal_h_P_3;
			fp_GetStrengthInternal_v	= GetStrengthInternal_v_P;
			fp_GetStrengthInternal_h	= GetStrengthInternal_h_P;
		}
		else
		{
			fp_GetStrengthInternal_v	= GetStrengthInternal_v_P_c;
			fp_GetStrengthInternal_h	= GetStrengthInternal_h_P_c;
		}
//#endif
	}

	MbQ	= &(p->mb_data[start_mb]);
	if(dec_picture->MbaffFrameFlag)
	{	
		MbLeft = MbQ-2;
		MbTop	= MbQ-(p->PicWidthInMbs<<1);
		for(i=start_mb;i<start_mb+num_mb;i+=2)
		{
			mby = ((i>>1)/p->PicWidthInMbs)<<1;
			mbx = (i>>1)%p->PicWidthInMbs;
			if((cpu_type == CPU_LEVEL_SSE2) || (cpu_type == CPU_LEVEL_SSE3))
			{
				build_deblocking_control_buffer_mbaff_sse2 ARGS7( p,	 i, mbx,	 mby, MbLeft, MbTop, MbQ++);
				build_deblocking_control_buffer_mbaff_sse2 ARGS7( p, i+1, mbx, mby+1, MbLeft, MbTop, MbQ++);
			}
			else
			{
				build_deblocking_control_buffer_mbaff_c ARGS7( p,	 i, mbx,	 mby, MbLeft, MbTop, MbQ++);
				build_deblocking_control_buffer_mbaff_c ARGS7( p, i+1, mbx, mby+1, MbLeft, MbTop, MbQ++);
			}
			MbLeft+=2;
			MbTop +=2;
		}
	}
	else
	{
		MbLeft = MbQ-1;
		MbTop	= MbQ-p->PicWidthInMbs;

		for(i=start_mb;i<start_mb+num_mb;i++)
		{
			mby = i/p->PicWidthInMbs;
			mbx = i%p->PicWidthInMbs;
			build_deblocking_control_buffer ARGS7( p, i, mbx, mby, MbLeft, MbTop, MbQ++);
			MbLeft++;
			MbTop++;
		}
	}

	if(m_bLast_Slice)
	{
		DWORD m_dwRetValue;
		DXVA_BufferDescription *m_pDxvaBufferDescription = IMGPAR m_pDxvaBufferDescription;
		AMVABUFFERINFO				 *m_pBufferInfo						= IMGPAR m_pBufferInfo;							
		memset(&m_pDxvaBufferDescription[3], 0, sizeof(DXVA_BufferDescription));
		m_pDxvaBufferDescription[3].dwBufferIndex = m_pBufferInfo[3].dwBufferIndex = IMGPAR pic_dec_buf_idx;
		m_pDxvaBufferDescription[3].dwDataSize = m_pBufferInfo[3].dwDataSize = IMGPAR m_lpDEBLK_Chroma - m_pbDeblockingCtrlBuf[IMGPAR pic_dec_buf_idx];
		m_pDxvaBufferDescription[3].dwDataOffset = 0;
		m_pDxvaBufferDescription[3].dwFirstMBaddress = 0;
		m_pDxvaBufferDescription[3].dwNumMBsInBuffer = 0;
		m_pDxvaBufferDescription[3].dwStride = 0;
		m_pDxvaBufferDescription[3].dwTypeIndex = m_pBufferInfo[3].dwTypeIndex = DXVA_DEBLOCKING_CONTROL_BUFFER;
		m_pDxvaBufferDescription[3].dwWidth = 0;
		m_pDxvaBufferDescription[3].dwHeight = IMGPAR FrameHeightInMbs*16;
		m_pBufferInfo[3].dwDataOffset = 0;

		//		if(dec_picture->structure == TOP_FIELD || dec_picture->structure == BOTTOM_FIELD)
		//			m_pDxvaBufferDescription[3].dwHeight >>= 1;

		hr = Execute(DXVA_PICTURE_DECODING_FUNCTION << 24, &m_pDxvaBufferDescription[3], sizeof(DXVA_BufferDescription), &m_dwRetValue, 4, 1, &m_pBufferInfo[3]);
		m_bPicCompBufStaus[IMGPAR pic_dec_buf_idx] = TRUE;
	}

}

void CH264DXVA1_NV::DeblockMacroblock PARGS2(StorablePicture *p, int mb_nr)
{
	int mbx, mby;
	Macroblock_s	*MbQ, *MbTop, *MbLeft;

	MbQ	= &(p->mb_data[mb_nr]);
	mby	= IMGPAR mb_y_d;
	mbx	= IMGPAR mb_x_d;

	if(dec_picture->MbaffFrameFlag)
	{
		if(mb_nr&1)
		{
			MbLeft = (&(p->mb_data[mb_nr-1]))-2;
			MbTop	= (&(p->mb_data[mb_nr-1]))-(p->PicWidthInMbs<<1);
		}
		else
		{
			MbLeft = MbQ-2;
			MbTop	= MbQ-(p->PicWidthInMbs<<1);
		}
		if((cpu_type == CPU_LEVEL_SSE2) || (cpu_type == CPU_LEVEL_SSE3))
			build_deblocking_control_buffer_mbaff_sse2 ARGS7( p, mb_nr, mbx,	 mby, MbLeft, MbTop, MbQ);
		else
			build_deblocking_control_buffer_mbaff_c ARGS7( p, mb_nr, mbx,	 mby, MbLeft, MbTop, MbQ);
	}
	else
	{
		MbLeft = MbQ-1;
		MbTop	= MbQ-p->PicWidthInMbs;
		build_deblocking_control_buffer ARGS7( p, mb_nr, mbx, mby, MbLeft, MbTop, MbQ);
	}
}

void CH264DXVA1_NV::build_deblocking_control_buffer PARGS7(StorablePicture *p, int MbQAddr, int mbx, int mby, Macroblock_s *Mb_left, Macroblock_s *Mb_top, Macroblock_s	*MbQ)
{
	bool		mixedModeEdgeFlagV=0;
	int				 mvlimit, bExtraEdge=0;
	int			LEAoffset, LEBoffset;
	int				 filterLeftMbEdgeFlag, filterTopMbEdgeFlag;
	int			fieldQ, QpQ;
	int			QP, IndexA[3], IndexB[3], IndexA4[6], IndexB4[6];
	byte				StrengthV[4][16],StrengthH[5][4];
	Macroblock_s	*MbPV=0,*MbPH=0;

	EdgeDataLuma* pLuma = (EdgeDataLuma*)IMGPAR m_lpDEBLK_Luma;
	EdgeDataChroma* pChroma = (EdgeDataChroma*)IMGPAR m_lpDEBLK_Chroma;
	memset(StrengthV,0,64);
	memset(StrengthH,0,20);

	QpQ = MbQ->qp;
	fieldQ = MbQ->mb_field;
	LEAoffset = IMGPAR currentSlice->LFAlphaC0Offset;
	LEBoffset = IMGPAR currentSlice->LFBetaOffset;
	mvlimit = 4 >> (int) ((p->structure!=FRAME) || (p->MbaffFrameFlag && MbQ->mb_field)); // 2 if field mode

	filterLeftMbEdgeFlag	= (mbx != 0);
	filterTopMbEdgeFlag	 = (mby != 0) && !(p->MbaffFrameFlag && mby==1 && MbQ->mb_field);

	if (IMGPAR currentSlice->LFDisableIdc==2)
	{
		filterLeftMbEdgeFlag = MbQ->filterLeftMbEdgeFlag;
		filterTopMbEdgeFlag	= MbQ->filterTopMbEdgeFlag;
	}

	// Start getting strength
	// Vertical external strength
	if(filterLeftMbEdgeFlag)
	{
		MbPV = Mb_left;	// temporary one
		if(fieldQ != MbPV->mb_field)
		{
			// mixed edge
			MbPV=Mb_left;
			mixedModeEdgeFlagV=1;
		}
		else
			MbPV=Mb_left+(p->MbaffFrameFlag&&(MbQAddr&1));		
		fp_GetStrength_v(StrengthV[0],MbQ,MbQAddr,mbx,mby,MbPV, 0, mvlimit, mixedModeEdgeFlagV); // Strength for 4 blks in 1 stripe
	}
	// Vertical internal strength
	if(MbQ->luma_transform_size_8x8_flag)		
		fp_GetStrengthInternal_v(StrengthV[2],MbQ,2, mvlimit); // Strength for 4 blks in 1 stripe
	else
	{
//#if	 defined(H264_ENABLE_INTRINSICS)
		if((cpu_type == CPU_LEVEL_SSE2) || (cpu_type == CPU_LEVEL_SSE3))
		{
			static int	StrengthSumV[4];
			fp_GetStrengthInternal_v_3(StrengthSumV, StrengthV[0], MbQ, mvlimit); // Strength for 4 blks in 1 stripe
		}
		else
		{
			fp_GetStrengthInternal_v(StrengthV[1],MbQ,1, mvlimit); // Strength for 4 blks in 1 stripe
			fp_GetStrengthInternal_v(StrengthV[2],MbQ,2, mvlimit); // Strength for 4 blks in 1 stripe
			fp_GetStrengthInternal_v(StrengthV[3],MbQ,3, mvlimit); // Strength for 4 blks in 1 stripe
		}
//#endif
	}

	// Horizontal external strength
	if(filterTopMbEdgeFlag)
	{
		MbPH = Mb_top;	// temporary one
		if(!p->MbaffFrameFlag)
			MbPH = Mb_top;
		else if(fieldQ)
			MbPH = Mb_top + ((MbQAddr&1) || (!MbPH->mb_field));
		else if(MbQAddr&1)
			MbPH = MbQ-1;
		else
		{
			bExtraEdge = MbPH->mb_field;
			MbPH = Mb_top + !bExtraEdge;
		}
		fp_GetStrength_h ARGS9(StrengthH[0],MbQ,MbQAddr,mbx,mby,MbPH, 0, mvlimit, p); // Strength for 4 blks in 1 stripe
		if(bExtraEdge)	// this is the extra horizontal edge between a frame macroblock pair and a field above it
			fp_GetStrength_h ARGS9(StrengthH[4],MbQ,MbQAddr,mbx,mby,MbPH+1, 4, mvlimit, p); // Strength for 4 blks in 1 stripe
	}
	// Horizontal internal strength
	if(MbQ->luma_transform_size_8x8_flag)
		fp_GetStrengthInternal_h(StrengthH[2],MbQ,2, mvlimit); // Strength for 4 blks in 1 stripe
	else
	{
//#if	 defined(H264_ENABLE_INTRINSICS)
		if((cpu_type == CPU_LEVEL_SSE2) || (cpu_type == CPU_LEVEL_SSE3))
		{
			static int StrengthSumH[5];
			fp_GetStrengthInternal_h_3(StrengthSumH, StrengthH[0], MbQ, mvlimit); // Strength for 4 blks in 1 stripe
		}
		else
		{
			fp_GetStrengthInternal_h(StrengthH[1],MbQ,1, mvlimit); // Strength for 4 blks in 1 stripe
			fp_GetStrengthInternal_h(StrengthH[2],MbQ,2, mvlimit); // Strength for 4 blks in 1 stripe
			fp_GetStrengthInternal_h(StrengthH[3],MbQ,3, mvlimit); // Strength for 4 blks in 1 stripe
		}
//#endif
	}//end horizontal edge

	// Start Deblocking
	// get internal index first
	IndexA[0] = __fast_iclip0_X(51, QpQ + LEAoffset);	
	IndexB[0] = __fast_iclip0_X(51, QpQ + LEBoffset);
	pLuma->AlphaInner = ALPHA_TABLE[IndexA[0]];
	pLuma->BetaInner	= BETA_TABLE[IndexB[0]];

	QP = QP_SCALE_CR[CQPOF(QpQ,0)];			// we do not handle UV offset, not in High profile
	IndexA[1] = __fast_iclip0_X(51, QP + LEAoffset);	
	IndexB[1] = __fast_iclip0_X(51, QP + LEBoffset);
	pChroma->AlphaInner_Cb = ALPHA_TABLE[IndexA[1]];
	pChroma->BetaInner_Cb	= BETA_TABLE[IndexB[1]];
	pChroma->AlphaInner_Cr = ALPHA_TABLE[IndexA[1]];
	pChroma->BetaInner_Cr	= BETA_TABLE[IndexB[1]];

	//Vertical external deblocking
	//	if(StrengthSumV[0])	// only if one of the 16 Strength bytes is != 0
	if(filterLeftMbEdgeFlag)
	{
		QP = (QpQ+MbPV->qp+1)>>1;
		IndexA4[0] = __fast_iclip0_X(51, QP + LEAoffset);	
		IndexB4[0] = __fast_iclip0_X(51, QP + LEBoffset);
		pLuma->AlphaEdge = ALPHA_TABLE[IndexA4[0]];
		pLuma->BetaEdge	= BETA_TABLE[IndexB4[0]];

		QP = (QP_SCALE_CR[CQPOF(QpQ,0)] + QP_SCALE_CR[CQPOF(MbPV->qp,0)] + 1) >> 1;
		IndexA4[2] = __fast_iclip0_X(51, QP + LEAoffset);	
		IndexB4[2] = __fast_iclip0_X(51, QP + LEBoffset);
		pChroma->AlphaEdge_Cb = ALPHA_TABLE[IndexA4[2]];
		pChroma->BetaEdge_Cb	= BETA_TABLE[IndexB4[2]];
		pChroma->AlphaEdge_Cr = ALPHA_TABLE[IndexA4[2]];
		pChroma->BetaEdge_Cr	= BETA_TABLE[IndexB4[2]];
		/*
		if(mixedModeEdgeFlagV)
		{
		QP = (QpQ+(MbPV+1)->qp+1)>>1;
		IndexA4[1] = __fast_iclip0_X(51, QP + LEAoffset);	
		IndexB4[1] = __fast_iclip0_X(51, QP + LEBoffset);
		QP = (QP_SCALE_CR[CQPOF(QpQ,0)] + QP_SCALE_CR[CQPOF((MbPV+1)->qp,0)] + 1) >> 1;
		IndexA4[3] = __fast_iclip0_X(51, QP + LEAoffset);	
		IndexB4[3] = __fast_iclip0_X(51, QP + LEBoffset);
		}
		*/
		//EdgeLoop_luma_v(SrcYQ, StrengthV[0], fieldQ, IndexA4, IndexB4, inc_y, mixedModeEdgeFlagV);
		pLuma->eFlagTC0[0].tC0[0] = CLIP_TAB[IndexA4[0]][StrengthV[0][0]];
		pLuma->eFlagTC0[0].tC0[1] = CLIP_TAB[IndexA4[0]][StrengthV[0][1]];
		pLuma->eFlagTC0[0].tC0[2] = CLIP_TAB[IndexA4[0]][StrengthV[0][2]];
		pLuma->eFlagTC0[0].tC0[3] = CLIP_TAB[IndexA4[0]][StrengthV[0][3]];
		pLuma->eFlagTC0[0].edgeFlag.eFlag0 = StrengthV[0][0] == 1 ? StrengthV[0][0] : (StrengthV[0][0] >> 1);
		pLuma->eFlagTC0[0].edgeFlag.eFlag1 = StrengthV[0][1] == 1 ? StrengthV[0][1] : (StrengthV[0][1] >> 1);
		pLuma->eFlagTC0[0].edgeFlag.eFlag2 = StrengthV[0][2] == 1 ? StrengthV[0][2] : (StrengthV[0][2] >> 1);
		pLuma->eFlagTC0[0].edgeFlag.eFlag3 = StrengthV[0][3] == 1 ? StrengthV[0][3] : (StrengthV[0][3] >> 1);
		pLuma->eFlagTC0[0].edgeFlag.b0.lineEdgeFlag = (	pLuma->eFlagTC0[0].edgeFlag.eFlag0 | 
			pLuma->eFlagTC0[0].edgeFlag.eFlag1 | 
			pLuma->eFlagTC0[0].edgeFlag.eFlag2 | 
			pLuma->eFlagTC0[0].edgeFlag.eFlag3 );

		//if( SrcUQ )	// check imgU for both UV
		//EdgeLoop_chromaUV_v(SrcUQ, SrcVQ, StrengthV[0], fieldQ, IndexA4+2, IndexB4+2, inc_ch, mixedModeEdgeFlagV) ;
		pChroma->tC0_Cb[0][0] = CLIP_TAB[IndexA4[2]][StrengthV[0][0]];
		pChroma->tC0_Cb[0][1] = CLIP_TAB[IndexA4[2]][StrengthV[0][1]];
		pChroma->tC0_Cb[0][2] = CLIP_TAB[IndexA4[2]][StrengthV[0][2]];
		pChroma->tC0_Cb[0][3] = CLIP_TAB[IndexA4[2]][StrengthV[0][3]];
		pChroma->tC0_Cr[0][0] = CLIP_TAB[IndexA4[2]][StrengthV[0][0]];
		pChroma->tC0_Cr[0][1] = CLIP_TAB[IndexA4[2]][StrengthV[0][1]];
		pChroma->tC0_Cr[0][2] = CLIP_TAB[IndexA4[2]][StrengthV[0][2]];
		pChroma->tC0_Cr[0][3] = CLIP_TAB[IndexA4[2]][StrengthV[0][3]];
		pChroma->edgeFlag[0].eFlag0 = StrengthV[0][0] == 1 ? StrengthV[0][0] : (StrengthV[0][0] >> 1);
		pChroma->edgeFlag[0].eFlag1 = StrengthV[0][1] == 1 ? StrengthV[0][1] : (StrengthV[0][1] >> 1);
		pChroma->edgeFlag[0].eFlag2 = StrengthV[0][2] == 1 ? StrengthV[0][2] : (StrengthV[0][2] >> 1);
		pChroma->edgeFlag[0].eFlag3 = StrengthV[0][3] == 1 ? StrengthV[0][3] : (StrengthV[0][3] >> 1);
		pChroma->edgeFlag[0].b0.lineEdgeFlag = (	pChroma->edgeFlag[0].eFlag0 |
			pChroma->edgeFlag[0].eFlag1 |
			pChroma->edgeFlag[0].eFlag2 |
			pChroma->edgeFlag[0].eFlag3 );
	}
	//Vertical internal deblocking
	//	if( !MbQ->luma_transform_size_8x8_flag && StrengthSumV[1] )	// only if one of the 16 Strength bytes is != 0
	//EdgeLoop_luma_v(SrcYQ+4, StrengthV[1], fieldQ, IndexA, IndexB, inc_y, 0);
	pLuma->eFlagTC0[1].tC0[0] = CLIP_TAB[IndexA[0]][StrengthV[1][0]];
	pLuma->eFlagTC0[1].tC0[1] = CLIP_TAB[IndexA[0]][StrengthV[1][1]];
	pLuma->eFlagTC0[1].tC0[2] = CLIP_TAB[IndexA[0]][StrengthV[1][2]];
	pLuma->eFlagTC0[1].tC0[3] = CLIP_TAB[IndexA[0]][StrengthV[1][3]];
	pLuma->eFlagTC0[1].edgeFlag.eFlag0 = StrengthV[1][0] == 1 ? StrengthV[1][0] : (StrengthV[1][0] >> 1);
	pLuma->eFlagTC0[1].edgeFlag.eFlag1 = StrengthV[1][1] == 1 ? StrengthV[1][1] : (StrengthV[1][1] >> 1);
	pLuma->eFlagTC0[1].edgeFlag.eFlag2 = StrengthV[1][2] == 1 ? StrengthV[1][2] : (StrengthV[1][2] >> 1);
	pLuma->eFlagTC0[1].edgeFlag.eFlag3 = StrengthV[1][3] == 1 ? StrengthV[1][3] : (StrengthV[1][3] >> 1);
	pLuma->eFlagTC0[1].edgeFlag.b0.lineEdgeFlag = (	pLuma->eFlagTC0[1].edgeFlag.eFlag0 | 
		pLuma->eFlagTC0[1].edgeFlag.eFlag1 | 
		pLuma->eFlagTC0[1].edgeFlag.eFlag2 | 
		pLuma->eFlagTC0[1].edgeFlag.eFlag3 );
	//	if( StrengthSumV[2])	// only if one of the 16 Strength bytes is != 0
	//	{
	//EdgeLoop_luma_v(SrcYQ+8, StrengthV[2], fieldQ, IndexA, IndexB, inc_y, 0);
	pLuma->eFlagTC0[2].tC0[0] = CLIP_TAB[IndexA[0]][StrengthV[2][0]];
	pLuma->eFlagTC0[2].tC0[1] = CLIP_TAB[IndexA[0]][StrengthV[2][1]];
	pLuma->eFlagTC0[2].tC0[2] = CLIP_TAB[IndexA[0]][StrengthV[2][2]];
	pLuma->eFlagTC0[2].tC0[3] = CLIP_TAB[IndexA[0]][StrengthV[2][3]];
	pLuma->eFlagTC0[2].edgeFlag.eFlag0 = StrengthV[2][0] == 1 ? StrengthV[2][0] : (StrengthV[2][0] >> 1);
	pLuma->eFlagTC0[2].edgeFlag.eFlag1 = StrengthV[2][1] == 1 ? StrengthV[2][1] : (StrengthV[2][1] >> 1);
	pLuma->eFlagTC0[2].edgeFlag.eFlag2 = StrengthV[2][2] == 1 ? StrengthV[2][2] : (StrengthV[2][2] >> 1);
	pLuma->eFlagTC0[2].edgeFlag.eFlag3 = StrengthV[2][3] == 1 ? StrengthV[2][3] : (StrengthV[2][3] >> 1);
	pLuma->eFlagTC0[2].edgeFlag.b0.lineEdgeFlag = (	pLuma->eFlagTC0[2].edgeFlag.eFlag0 | 
		pLuma->eFlagTC0[2].edgeFlag.eFlag1 | 
		pLuma->eFlagTC0[2].edgeFlag.eFlag2 | 
		pLuma->eFlagTC0[2].edgeFlag.eFlag3 );
	//if( SrcUQ )	// check imgU for both UV
	//EdgeLoop_chromaUV_v(SrcUQ+4, SrcVQ+4, StrengthV[2], fieldQ, IndexA+1, IndexB+1, inc_ch, 0) ;
	pChroma->tC0_Cb[1][0] = CLIP_TAB[IndexA[1]][StrengthV[2][0]];
	pChroma->tC0_Cb[1][1] = CLIP_TAB[IndexA[1]][StrengthV[2][1]];
	pChroma->tC0_Cb[1][2] = CLIP_TAB[IndexA[1]][StrengthV[2][2]];
	pChroma->tC0_Cb[1][3] = CLIP_TAB[IndexA[1]][StrengthV[2][3]];
	pChroma->tC0_Cr[1][0] = CLIP_TAB[IndexA[1]][StrengthV[2][0]];
	pChroma->tC0_Cr[1][1] = CLIP_TAB[IndexA[1]][StrengthV[2][1]];
	pChroma->tC0_Cr[1][2] = CLIP_TAB[IndexA[1]][StrengthV[2][2]];
	pChroma->tC0_Cr[1][3] = CLIP_TAB[IndexA[1]][StrengthV[2][3]];
	pChroma->edgeFlag[1].eFlag0 = StrengthV[2][0] == 1 ? StrengthV[2][0] : (StrengthV[2][0] >> 1);
	pChroma->edgeFlag[1].eFlag1 = StrengthV[2][1] == 1 ? StrengthV[2][1] : (StrengthV[2][1] >> 1);
	pChroma->edgeFlag[1].eFlag2 = StrengthV[2][2] == 1 ? StrengthV[2][2] : (StrengthV[2][2] >> 1);
	pChroma->edgeFlag[1].eFlag3 = StrengthV[2][3] == 1 ? StrengthV[2][3] : (StrengthV[2][3] >> 1);
	pChroma->edgeFlag[1].b0.lineEdgeFlag = (	pChroma->edgeFlag[1].eFlag0 |
		pChroma->edgeFlag[1].eFlag1 |
		pChroma->edgeFlag[1].eFlag2 |
		pChroma->edgeFlag[1].eFlag3 );
	//	}
	//	if( !MbQ->luma_transform_size_8x8_flag && StrengthSumV[3] )	// only if one of the 16 Strength bytes is != 0
	//EdgeLoop_luma_v(SrcYQ+12, StrengthV[3], fieldQ, IndexA, IndexB, inc_y, 0);
	pLuma->eFlagTC0[3].tC0[0] = CLIP_TAB[IndexA[0]][StrengthV[3][0]];
	pLuma->eFlagTC0[3].tC0[1] = CLIP_TAB[IndexA[0]][StrengthV[3][1]];
	pLuma->eFlagTC0[3].tC0[2] = CLIP_TAB[IndexA[0]][StrengthV[3][2]];
	pLuma->eFlagTC0[3].tC0[3] = CLIP_TAB[IndexA[0]][StrengthV[3][3]];
	pLuma->eFlagTC0[3].edgeFlag.eFlag0 = StrengthV[3][0] == 1 ? StrengthV[3][0] : (StrengthV[3][0] >> 1);
	pLuma->eFlagTC0[3].edgeFlag.eFlag1 = StrengthV[3][1] == 1 ? StrengthV[3][1] : (StrengthV[3][1] >> 1);
	pLuma->eFlagTC0[3].edgeFlag.eFlag2 = StrengthV[3][2] == 1 ? StrengthV[3][2] : (StrengthV[3][2] >> 1);
	pLuma->eFlagTC0[3].edgeFlag.eFlag3 = StrengthV[3][3] == 1 ? StrengthV[3][3] : (StrengthV[3][3] >> 1);
	pLuma->eFlagTC0[3].edgeFlag.b0.lineEdgeFlag = (	pLuma->eFlagTC0[3].edgeFlag.eFlag0 | 
		pLuma->eFlagTC0[3].edgeFlag.eFlag1 | 
		pLuma->eFlagTC0[3].edgeFlag.eFlag2 | 
		pLuma->eFlagTC0[3].edgeFlag.eFlag3 );
	//end Vertical deblocking

	//Dump(m_lpDEBLK_Luma, sizeof(EdgeDataLuma), "DEBLK_Luma.dat");
	//Dump(m_lpDEBLK_Chroma, sizeof(EdgeDataChroma), "DEBLK_Chroma.dat");
	IMGPAR m_lpDEBLK_Luma += sizeof(EdgeDataLuma);
	IMGPAR m_lpDEBLK_Chroma += sizeof(EdgeDataChroma);
	pLuma = (EdgeDataLuma*)IMGPAR m_lpDEBLK_Luma;
	pChroma = (EdgeDataChroma*)IMGPAR m_lpDEBLK_Chroma;


	//Horizontal external deblocking
	// get internal index first
	IndexA[0] = __fast_iclip0_X(51, QpQ + LEAoffset);	
	IndexB[0] = __fast_iclip0_X(51, QpQ + LEBoffset);
	pLuma->AlphaInner = ALPHA_TABLE[IndexA[0]];
	pLuma->BetaInner	= BETA_TABLE[IndexB[0]];

	QP = QP_SCALE_CR[CQPOF(QpQ,0)];			// we do not handle UV offset, not in High profile
	IndexA[1] = __fast_iclip0_X(51, QP + LEAoffset);	
	IndexB[1] = __fast_iclip0_X(51, QP + LEBoffset);
	pChroma->AlphaInner_Cb = ALPHA_TABLE[IndexA[1]];
	pChroma->BetaInner_Cb	= BETA_TABLE[IndexB[1]];
	pChroma->AlphaInner_Cr = ALPHA_TABLE[IndexA[1]];
	pChroma->BetaInner_Cr	= BETA_TABLE[IndexB[1]];
	//	if( StrengthSumH[0])
	if(filterTopMbEdgeFlag)
	{
		QP = (QpQ+MbPH->qp+1)>>1;
		IndexA4[0] = __fast_iclip0_X(51, QP + LEAoffset);	
		IndexB4[0] = __fast_iclip0_X(51, QP + LEBoffset);
		pLuma->AlphaEdge = ALPHA_TABLE[IndexA4[0]];
		pLuma->BetaEdge	= BETA_TABLE[IndexB4[0]];

		QP = (QP_SCALE_CR[CQPOF(QpQ,0)] + QP_SCALE_CR[CQPOF(MbPH->qp,0)] + 1) >> 1;
		IndexA4[1] = __fast_iclip0_X(51, QP + LEAoffset);	
		IndexB4[1] = __fast_iclip0_X(51, QP + LEBoffset);
		pChroma->AlphaEdge_Cb = ALPHA_TABLE[IndexA4[1]];
		pChroma->BetaEdge_Cb	= BETA_TABLE[IndexB4[1]];
		pChroma->AlphaEdge_Cr = ALPHA_TABLE[IndexA4[1]];
		pChroma->BetaEdge_Cr	= BETA_TABLE[IndexB4[1]];

		//EdgeLoop_luma_h(SrcYQ, StrengthH[0], IndexA4, IndexB4, inc_y<<bExtraEdge);
		pLuma->eFlagTC0[0].tC0[0] = CLIP_TAB[IndexA4[0]][StrengthH[0][0]];
		pLuma->eFlagTC0[0].tC0[1] = CLIP_TAB[IndexA4[0]][StrengthH[0][1]];
		pLuma->eFlagTC0[0].tC0[2] = CLIP_TAB[IndexA4[0]][StrengthH[0][2]];
		pLuma->eFlagTC0[0].tC0[3] = CLIP_TAB[IndexA4[0]][StrengthH[0][3]];
		pLuma->eFlagTC0[0].edgeFlag.eFlag0 = StrengthH[0][0] == 1 ? StrengthH[0][0] : (StrengthH[0][0] >> 1);
		pLuma->eFlagTC0[0].edgeFlag.eFlag1 = StrengthH[0][1] == 1 ? StrengthH[0][1] : (StrengthH[0][1] >> 1);
		pLuma->eFlagTC0[0].edgeFlag.eFlag2 = StrengthH[0][2] == 1 ? StrengthH[0][2] : (StrengthH[0][2] >> 1);
		pLuma->eFlagTC0[0].edgeFlag.eFlag3 = StrengthH[0][3] == 1 ? StrengthH[0][3] : (StrengthH[0][3] >> 1);
		pLuma->eFlagTC0[0].edgeFlag.b0.lineEdgeFlag = (	pLuma->eFlagTC0[0].edgeFlag.eFlag0 | 
			pLuma->eFlagTC0[0].edgeFlag.eFlag1 | 
			pLuma->eFlagTC0[0].edgeFlag.eFlag2 | 
			pLuma->eFlagTC0[0].edgeFlag.eFlag3 );

		//if( SrcUQ )	// check imgU for both UV
		//EdgeLoop_chromaUV_h(SrcUQ, SrcVQ, StrengthH[0], IndexA4+1, IndexB4+1, inc_ch<<bExtraEdge) ;
		pChroma->tC0_Cb[0][0] = CLIP_TAB[IndexA4[1]][StrengthH[0][0]];
		pChroma->tC0_Cb[0][1] = CLIP_TAB[IndexA4[1]][StrengthH[0][1]];
		pChroma->tC0_Cb[0][2] = CLIP_TAB[IndexA4[1]][StrengthH[0][2]];
		pChroma->tC0_Cb[0][3] = CLIP_TAB[IndexA4[1]][StrengthH[0][3]];
		pChroma->tC0_Cr[0][0] = CLIP_TAB[IndexA4[1]][StrengthH[0][0]];
		pChroma->tC0_Cr[0][1] = CLIP_TAB[IndexA4[1]][StrengthH[0][1]];
		pChroma->tC0_Cr[0][2] = CLIP_TAB[IndexA4[1]][StrengthH[0][2]];
		pChroma->tC0_Cr[0][3] = CLIP_TAB[IndexA4[1]][StrengthH[0][3]];
		pChroma->edgeFlag[0].eFlag0 = StrengthH[0][0] == 1 ? StrengthH[0][0] : (StrengthH[0][0] >> 1);
		pChroma->edgeFlag[0].eFlag1 = StrengthH[0][1] == 1 ? StrengthH[0][1] : (StrengthH[0][1] >> 1);
		pChroma->edgeFlag[0].eFlag2 = StrengthH[0][2] == 1 ? StrengthH[0][2] : (StrengthH[0][2] >> 1);
		pChroma->edgeFlag[0].eFlag3 = StrengthH[0][3] == 1 ? StrengthH[0][3] : (StrengthH[0][3] >> 1);
		pChroma->edgeFlag[0].b0.lineEdgeFlag = (	pChroma->edgeFlag[0].eFlag0 |
			pChroma->edgeFlag[0].eFlag1 |
			pChroma->edgeFlag[0].eFlag2 |
			pChroma->edgeFlag[0].eFlag3 );
	}
	/*
	// Extra horizontal edge between a frame macroblock pair and a field above it
	if( StrengthSumH[4] )
	{
	MbPH++;
	QP = (QpQ+MbPH->qp+1)>>1;
	IndexA4[0] = __fast_iclip0_X(51, QP + LEAoffset);	
	IndexB4[0] = __fast_iclip0_X(51, QP + LEBoffset);
	QP = (QP_SCALE_CR[CQPOF(QpQ,0)] + QP_SCALE_CR[CQPOF(MbPH->qp,0)] + 1) >> 1;
	IndexA4[1] = __fast_iclip0_X(51, QP + LEAoffset);	
	IndexB4[1] = __fast_iclip0_X(51, QP + LEBoffset);

	EdgeLoop_luma_h(SrcYQ+inc_y, StrengthH[4], IndexA4, IndexB4, inc_y<<bExtraEdge) ; 
	if( SrcUQ )	// check imgU for both UV
	EdgeLoop_chromaUV_h(SrcUQ+inc_ch, SrcVQ+inc_ch, StrengthH[4], IndexA4+1, IndexB4+1, inc_ch<<bExtraEdge) ;
	*/
	//Horizontal internal deblocking
	//	SrcYQ += inc_y4;
	//	if( !MbQ->luma_transform_size_8x8_flag && StrengthSumH[1] )
	//EdgeLoop_luma_h(SrcYQ, StrengthH[1], IndexA, IndexB, inc_y);
	pLuma->eFlagTC0[1].tC0[0] = CLIP_TAB[IndexA[0]][StrengthH[1][0]];
	pLuma->eFlagTC0[1].tC0[1] = CLIP_TAB[IndexA[0]][StrengthH[1][1]];
	pLuma->eFlagTC0[1].tC0[2] = CLIP_TAB[IndexA[0]][StrengthH[1][2]];
	pLuma->eFlagTC0[1].tC0[3] = CLIP_TAB[IndexA[0]][StrengthH[1][3]];
	pLuma->eFlagTC0[1].edgeFlag.eFlag0 = StrengthH[1][0] == 1 ? StrengthH[1][0] : (StrengthH[1][0] >> 1);
	pLuma->eFlagTC0[1].edgeFlag.eFlag1 = StrengthH[1][1] == 1 ? StrengthH[1][1] : (StrengthH[1][1] >> 1);
	pLuma->eFlagTC0[1].edgeFlag.eFlag2 = StrengthH[1][2] == 1 ? StrengthH[1][2] : (StrengthH[1][2] >> 1);
	pLuma->eFlagTC0[1].edgeFlag.eFlag3 = StrengthH[1][3] == 1 ? StrengthH[1][3] : (StrengthH[1][3] >> 1);
	pLuma->eFlagTC0[1].edgeFlag.b0.lineEdgeFlag = (	pLuma->eFlagTC0[1].edgeFlag.eFlag0 | 
		pLuma->eFlagTC0[1].edgeFlag.eFlag1 | 
		pLuma->eFlagTC0[1].edgeFlag.eFlag2 | 
		pLuma->eFlagTC0[1].edgeFlag.eFlag3 );
	//	SrcYQ += inc_y4;
	//	if( StrengthSumH[2])
	//	{
	//EdgeLoop_luma_h(SrcYQ, StrengthH[2], IndexA, IndexB, inc_y);
	pLuma->eFlagTC0[2].tC0[0] = CLIP_TAB[IndexA[0]][StrengthH[2][0]];
	pLuma->eFlagTC0[2].tC0[1] = CLIP_TAB[IndexA[0]][StrengthH[2][1]];
	pLuma->eFlagTC0[2].tC0[2] = CLIP_TAB[IndexA[0]][StrengthH[2][2]];
	pLuma->eFlagTC0[2].tC0[3] = CLIP_TAB[IndexA[0]][StrengthH[2][3]];
	pLuma->eFlagTC0[2].edgeFlag.eFlag0 = StrengthH[2][0] == 1 ? StrengthH[2][0] : (StrengthH[2][0] >> 1);
	pLuma->eFlagTC0[2].edgeFlag.eFlag1 = StrengthH[2][1] == 1 ? StrengthH[2][1] : (StrengthH[2][1] >> 1);
	pLuma->eFlagTC0[2].edgeFlag.eFlag2 = StrengthH[2][2] == 1 ? StrengthH[2][2] : (StrengthH[2][2] >> 1);
	pLuma->eFlagTC0[2].edgeFlag.eFlag3 = StrengthH[2][3] == 1 ? StrengthH[2][3] : (StrengthH[2][3] >> 1);
	pLuma->eFlagTC0[2].edgeFlag.b0.lineEdgeFlag = (	pLuma->eFlagTC0[2].edgeFlag.eFlag0 | 
		pLuma->eFlagTC0[2].edgeFlag.eFlag1 | 
		pLuma->eFlagTC0[2].edgeFlag.eFlag2 | 
		pLuma->eFlagTC0[2].edgeFlag.eFlag3 );
	//if( SrcUQ )	// check imgU for both UV
	//EdgeLoop_chromaUV_h(SrcUQ+(inc_ch<<2), SrcVQ+(inc_ch<<2), StrengthH[2], IndexA+1, IndexB+1, inc_ch) ;
	pChroma->tC0_Cb[1][0] = CLIP_TAB[IndexA[1]][StrengthH[2][0]];
	pChroma->tC0_Cb[1][1] = CLIP_TAB[IndexA[1]][StrengthH[2][1]];
	pChroma->tC0_Cb[1][2] = CLIP_TAB[IndexA[1]][StrengthH[2][2]];
	pChroma->tC0_Cb[1][3] = CLIP_TAB[IndexA[1]][StrengthH[2][3]];
	pChroma->tC0_Cr[1][0] = CLIP_TAB[IndexA[1]][StrengthH[2][0]];
	pChroma->tC0_Cr[1][1] = CLIP_TAB[IndexA[1]][StrengthH[2][1]];
	pChroma->tC0_Cr[1][2] = CLIP_TAB[IndexA[1]][StrengthH[2][2]];
	pChroma->tC0_Cr[1][3] = CLIP_TAB[IndexA[1]][StrengthH[2][3]];
	pChroma->edgeFlag[1].eFlag0 = StrengthH[2][0] == 1 ? StrengthH[2][0] : (StrengthH[2][0] >> 1);
	pChroma->edgeFlag[1].eFlag1 = StrengthH[2][1] == 1 ? StrengthH[2][1] : (StrengthH[2][1] >> 1);
	pChroma->edgeFlag[1].eFlag2 = StrengthH[2][2] == 1 ? StrengthH[2][2] : (StrengthH[2][2] >> 1);
	pChroma->edgeFlag[1].eFlag3 = StrengthH[2][3] == 1 ? StrengthH[2][3] : (StrengthH[2][3] >> 1);
	pChroma->edgeFlag[1].b0.lineEdgeFlag = (	pChroma->edgeFlag[1].eFlag0 |
		pChroma->edgeFlag[1].eFlag1 |
		pChroma->edgeFlag[1].eFlag2 |
		pChroma->edgeFlag[1].eFlag3 );
	//	}
	//	SrcYQ += inc_y4;
	//	if( !MbQ->luma_transform_size_8x8_flag && StrengthSumH[3] )
	//EdgeLoop_luma_h(SrcYQ, StrengthH[3], IndexA, IndexB, inc_y);
	pLuma->eFlagTC0[3].tC0[0] = CLIP_TAB[IndexA[0]][StrengthH[3][0]];
	pLuma->eFlagTC0[3].tC0[1] = CLIP_TAB[IndexA[0]][StrengthH[3][1]];
	pLuma->eFlagTC0[3].tC0[2] = CLIP_TAB[IndexA[0]][StrengthH[3][2]];
	pLuma->eFlagTC0[3].tC0[3] = CLIP_TAB[IndexA[0]][StrengthH[3][3]];
	pLuma->eFlagTC0[3].edgeFlag.eFlag0 = StrengthH[3][0] == 1 ? StrengthH[3][0] : (StrengthH[3][0] >> 1);
	pLuma->eFlagTC0[3].edgeFlag.eFlag1 = StrengthH[3][1] == 1 ? StrengthH[3][1] : (StrengthH[3][1] >> 1);
	pLuma->eFlagTC0[3].edgeFlag.eFlag2 = StrengthH[3][2] == 1 ? StrengthH[3][2] : (StrengthH[3][2] >> 1);
	pLuma->eFlagTC0[3].edgeFlag.eFlag3 = StrengthH[3][3] == 1 ? StrengthH[3][3] : (StrengthH[3][3] >> 1);
	pLuma->eFlagTC0[3].edgeFlag.b0.lineEdgeFlag = (	pLuma->eFlagTC0[3].edgeFlag.eFlag0 | 
		pLuma->eFlagTC0[3].edgeFlag.eFlag1 | 
		pLuma->eFlagTC0[3].edgeFlag.eFlag2 | 
		pLuma->eFlagTC0[3].edgeFlag.eFlag3 );
	//end horizontal deblocking
	//Dump(m_lpDEBLK_Luma, sizeof(EdgeDataLuma), "DEBLK_Luma.dat");
	//Dump(m_lpDEBLK_Chroma, sizeof(EdgeDataChroma), "DEBLK_Chroma.dat");
	IMGPAR m_lpDEBLK_Luma += sizeof(EdgeDataLuma);
	IMGPAR m_lpDEBLK_Chroma += sizeof(EdgeDataChroma);
}

//#if	 !defined(H264_ENABLE_INTRINSICS)
void CH264DXVA1_NV::build_deblocking_control_buffer_mbaff_c PARGS7(StorablePicture *p, int MbQAddr, int mbx, int mby, Macroblock_s *Mb_left, Macroblock_s *Mb_top, Macroblock_s	*MbQ)
{
	bool		mixedModeEdgeFlagV=0;
	int				 mvlimit, bExtraEdge=0;
	int			LEAoffset, LEBoffset;
	int				 filterLeftMbEdgeFlag, filterTopMbEdgeFlag;
	int			fieldQ, QpQ;
	int			QP, IndexA[3], IndexB[3], IndexA4[6], IndexB4[6];
	byte				StrengthV[4][16],StrengthH[5][4];
	Macroblock_s	*MbPV=0,*MbPH=0;

	EdgeDataLumaMBAFF* pLuma = (EdgeDataLumaMBAFF*)IMGPAR m_lpDEBLK_Luma;
	EdgeDataChromaMBAFF* pChroma = (EdgeDataChromaMBAFF*)IMGPAR m_lpDEBLK_Chroma;
	memset(StrengthV,0,64);
	memset(StrengthH,0,20);

	QpQ = MbQ->qp;
	fieldQ = MbQ->mb_field;
	LEAoffset = IMGPAR currentSlice->LFAlphaC0Offset;
	LEBoffset = IMGPAR currentSlice->LFBetaOffset;
	mvlimit = 4 >> (int) ((p->structure!=FRAME) || (p->MbaffFrameFlag && MbQ->mb_field)); // 2 if field mode

	filterLeftMbEdgeFlag	= (mbx != 0);
	filterTopMbEdgeFlag	 = (mby != 0) && !(p->MbaffFrameFlag && mby==1 && MbQ->mb_field);

	if (IMGPAR currentSlice->LFDisableIdc==2)
	{
		filterLeftMbEdgeFlag = MbQ->filterLeftMbEdgeFlag;
		filterTopMbEdgeFlag	= MbQ->filterTopMbEdgeFlag;
	}

	// Start getting strength
	// Vertical external strength
	if(filterLeftMbEdgeFlag)
	{
		MbPV = Mb_left;	// temporary one
		if(fieldQ != MbPV->mb_field)
		{
			// mixed edge
			MbPV=Mb_left;
			mixedModeEdgeFlagV=1;
		}
		else
			MbPV=Mb_left+(p->MbaffFrameFlag&&(MbQAddr&1));		
		fp_GetStrength_v(StrengthV[0],MbQ,MbQAddr,mbx,mby,MbPV, 0, mvlimit, mixedModeEdgeFlagV); // Strength for 4 blks in 1 stripe
	}
	// Vertical internal strength
	if(MbQ->luma_transform_size_8x8_flag)		
		fp_GetStrengthInternal_v(StrengthV[2],MbQ,2, mvlimit); // Strength for 4 blks in 1 stripe
	else
	{
		fp_GetStrengthInternal_v(StrengthV[1],MbQ,1, mvlimit); // Strength for 4 blks in 1 stripe
		fp_GetStrengthInternal_v(StrengthV[2],MbQ,2, mvlimit); // Strength for 4 blks in 1 stripe
		fp_GetStrengthInternal_v(StrengthV[3],MbQ,3, mvlimit); // Strength for 4 blks in 1 stripe
	}

	// Horizontal external strength
	if(filterTopMbEdgeFlag)
	{
		MbPH = Mb_top;	// temporary one
		if(!p->MbaffFrameFlag)
			MbPH = Mb_top;
		else if(fieldQ)
			MbPH = Mb_top + ((MbQAddr&1) || (!MbPH->mb_field));
		else if(MbQAddr&1)
			MbPH = MbQ-1;
		else
		{
			bExtraEdge = MbPH->mb_field;
			MbPH = Mb_top + !bExtraEdge;
		}
		fp_GetStrength_h ARGS9(StrengthH[0],MbQ,MbQAddr,mbx,mby,MbPH, 0, mvlimit, p); // Strength for 4 blks in 1 stripe
		if (bExtraEdge)	// this is the extra horizontal edge between a frame macroblock pair and a field above it
			fp_GetStrength_h ARGS9(StrengthH[4],MbQ,MbQAddr,mbx,mby,MbPH+1, 4, mvlimit, p); // Strength for 4 blks in 1 stripe
	}
	// Horizontal internal strength
	if(MbQ->luma_transform_size_8x8_flag)
		fp_GetStrengthInternal_h(StrengthH[2],MbQ,2, mvlimit); // Strength for 4 blks in 1 stripe
	else
	{
		fp_GetStrengthInternal_h(StrengthH[1],MbQ,1, mvlimit); // Strength for 4 blks in 1 stripe
		fp_GetStrengthInternal_h(StrengthH[2],MbQ,2, mvlimit); // Strength for 4 blks in 1 stripe
		fp_GetStrengthInternal_h(StrengthH[3],MbQ,3, mvlimit); // Strength for 4 blks in 1 stripe
	}//end horizontal edge

	// Start Deblocking
	// get internal index first
	IndexA[0] = __fast_iclip0_X(51, QpQ + LEAoffset);	
	IndexB[0] = __fast_iclip0_X(51, QpQ + LEBoffset);
	pLuma->AlphaInner = ALPHA_TABLE[IndexA[0]];
	pLuma->BetaInner	= BETA_TABLE[IndexB[0]];

	QP = QP_SCALE_CR[CQPOF(QpQ,0)];			// we do not handle UV offset, not in High profile
	IndexA[1] = __fast_iclip0_X(51, QP + LEAoffset);	
	IndexB[1] = __fast_iclip0_X(51, QP + LEBoffset);
	pChroma->AlphaInner_Cb = ALPHA_TABLE[IndexA[1]];
	pChroma->BetaInner_Cb	= BETA_TABLE[IndexB[1]];
	pChroma->AlphaInner_Cr = ALPHA_TABLE[IndexA[1]];
	pChroma->BetaInner_Cr	= BETA_TABLE[IndexB[1]];

	//Vertical external deblocking
	//	if(StrengthSumV[0])	// only if one of the 16 Strength bytes is != 0
	if(filterLeftMbEdgeFlag)
	{
		QP = (QpQ+MbPV->qp+1)>>1;
		IndexA4[0] = __fast_iclip0_X(51, QP + LEAoffset);	
		IndexB4[0] = __fast_iclip0_X(51, QP + LEBoffset);
		pLuma->AlphaEdge = ALPHA_TABLE[IndexA4[0]];
		pLuma->BetaEdge	= BETA_TABLE[IndexB4[0]];

		QP = (QP_SCALE_CR[CQPOF(QpQ,0)] + QP_SCALE_CR[CQPOF(MbPV->qp,0)] + 1) >> 1;
		IndexA4[2] = __fast_iclip0_X(51, QP + LEAoffset);	
		IndexB4[2] = __fast_iclip0_X(51, QP + LEBoffset);
		pChroma->AlphaEdge_Cb = ALPHA_TABLE[IndexA4[2]];
		pChroma->BetaEdge_Cb	= BETA_TABLE[IndexB4[2]];
		pChroma->AlphaEdge_Cr = ALPHA_TABLE[IndexA4[2]];
		pChroma->BetaEdge_Cr	= BETA_TABLE[IndexB4[2]];

		if(mixedModeEdgeFlagV)
		{
			QP = (QpQ+(MbPV+1)->qp+1)>>1;
			IndexA4[1] = __fast_iclip0_X(51, QP + LEAoffset);	
			IndexB4[1] = __fast_iclip0_X(51, QP + LEBoffset);
			pLuma->AlphaEdge2 = ALPHA_TABLE[IndexA4[1]];
			pLuma->BetaEdge2	= BETA_TABLE[IndexB4[1]];

			QP = (QP_SCALE_CR[CQPOF(QpQ,0)] + QP_SCALE_CR[CQPOF((MbPV+1)->qp,0)] + 1) >> 1;
			IndexA4[3] = __fast_iclip0_X(51, QP + LEAoffset);	
			IndexB4[3] = __fast_iclip0_X(51, QP + LEBoffset);
			pChroma->AlphaEdge2_Cb = ALPHA_TABLE[IndexA4[3]];
			pChroma->BetaEdge2_Cb	= BETA_TABLE[IndexB4[3]];
			pChroma->AlphaEdge2_Cr = ALPHA_TABLE[IndexA4[3]];
			pChroma->BetaEdge2_Cr	= BETA_TABLE[IndexB4[3]];
		}

		//EdgeLoop_luma_v(SrcYQ, StrengthV[0], fieldQ, IndexA4, IndexB4, inc_y, mixedModeEdgeFlagV);
		pLuma->eFlagTC0[1].tC0[0] = CLIP_TAB[IndexA4[0]][StrengthV[0][0]];
		pLuma->eFlagTC0[1].tC0[1] = CLIP_TAB[IndexA4[0]][StrengthV[0][1]];
		pLuma->eFlagTC0[1].tC0[2] = CLIP_TAB[IndexA4[0]][StrengthV[0][2]];
		pLuma->eFlagTC0[1].tC0[3] = CLIP_TAB[IndexA4[0]][StrengthV[0][3]];
		pLuma->eFlagTC0[1].edgeFlag.eFlag0 = StrengthV[0][0] == 1 ? StrengthV[0][0] : (StrengthV[0][0] >> 1);
		pLuma->eFlagTC0[1].edgeFlag.eFlag1 = StrengthV[0][1] == 1 ? StrengthV[0][1] : (StrengthV[0][1] >> 1);
		pLuma->eFlagTC0[1].edgeFlag.eFlag2 = StrengthV[0][2] == 1 ? StrengthV[0][2] : (StrengthV[0][2] >> 1);
		pLuma->eFlagTC0[1].edgeFlag.eFlag3 = StrengthV[0][3] == 1 ? StrengthV[0][3] : (StrengthV[0][3] >> 1);
		pLuma->eFlagTC0[1].edgeFlag.b0.lineEdgeFlag = (	pLuma->eFlagTC0[1].edgeFlag.eFlag0 | 
			pLuma->eFlagTC0[1].edgeFlag.eFlag1 | 
			pLuma->eFlagTC0[1].edgeFlag.eFlag2 | 
			pLuma->eFlagTC0[1].edgeFlag.eFlag3 );
		pLuma->eFlagTC0[1].edgeFlag.b0.mbField = fieldQ;

		//if( SrcUQ )	// check imgU for both UV
		//EdgeLoop_chromaUV_v(SrcUQ, SrcVQ, StrengthV[0], fieldQ, IndexA4+2, IndexB4+2, inc_ch, mixedModeEdgeFlagV) ;
		pChroma->tC0_Cb[1][0] = CLIP_TAB[IndexA4[2]][StrengthV[0][0]];
		pChroma->tC0_Cb[1][1] = CLIP_TAB[IndexA4[2]][StrengthV[0][1]];
		pChroma->tC0_Cb[1][2] = CLIP_TAB[IndexA4[2]][StrengthV[0][2]];
		pChroma->tC0_Cb[1][3] = CLIP_TAB[IndexA4[2]][StrengthV[0][3]];
		pChroma->tC0_Cr[1][0] = CLIP_TAB[IndexA4[2]][StrengthV[0][0]];
		pChroma->tC0_Cr[1][1] = CLIP_TAB[IndexA4[2]][StrengthV[0][1]];
		pChroma->tC0_Cr[1][2] = CLIP_TAB[IndexA4[2]][StrengthV[0][2]];
		pChroma->tC0_Cr[1][3] = CLIP_TAB[IndexA4[2]][StrengthV[0][3]];
		pChroma->edgeFlag[1].eFlag0 = StrengthV[0][0] == 1 ? StrengthV[0][0] : (StrengthV[0][0] >> 1);
		pChroma->edgeFlag[1].eFlag1 = StrengthV[0][1] == 1 ? StrengthV[0][1] : (StrengthV[0][1] >> 1);
		pChroma->edgeFlag[1].eFlag2 = StrengthV[0][2] == 1 ? StrengthV[0][2] : (StrengthV[0][2] >> 1);
		pChroma->edgeFlag[1].eFlag3 = StrengthV[0][3] == 1 ? StrengthV[0][3] : (StrengthV[0][3] >> 1);
		pChroma->edgeFlag[1].b0.lineEdgeFlag = (	pChroma->edgeFlag[1].eFlag0 |
			pChroma->edgeFlag[1].eFlag1 |
			pChroma->edgeFlag[1].eFlag2 |
			pChroma->edgeFlag[1].eFlag3 );
		pChroma->edgeFlag[1].b0.mbField = fieldQ;
	}
	//Vertical internal deblocking
	//	if( !MbQ->luma_transform_size_8x8_flag && StrengthSumV[1] )	// only if one of the 16 Strength bytes is != 0
	//EdgeLoop_luma_v(SrcYQ+4, StrengthV[1], fieldQ, IndexA, IndexB, inc_y, 0);
	pLuma->eFlagTC0[2].tC0[0] = CLIP_TAB[IndexA[0]][StrengthV[1][0]];
	pLuma->eFlagTC0[2].tC0[1] = CLIP_TAB[IndexA[0]][StrengthV[1][1]];
	pLuma->eFlagTC0[2].tC0[2] = CLIP_TAB[IndexA[0]][StrengthV[1][2]];
	pLuma->eFlagTC0[2].tC0[3] = CLIP_TAB[IndexA[0]][StrengthV[1][3]];
	pLuma->eFlagTC0[2].edgeFlag.eFlag0 = StrengthV[1][0] == 1 ? StrengthV[1][0] : (StrengthV[1][0] >> 1);
	pLuma->eFlagTC0[2].edgeFlag.eFlag1 = StrengthV[1][1] == 1 ? StrengthV[1][1] : (StrengthV[1][1] >> 1);
	pLuma->eFlagTC0[2].edgeFlag.eFlag2 = StrengthV[1][2] == 1 ? StrengthV[1][2] : (StrengthV[1][2] >> 1);
	pLuma->eFlagTC0[2].edgeFlag.eFlag3 = StrengthV[1][3] == 1 ? StrengthV[1][3] : (StrengthV[1][3] >> 1);
	pLuma->eFlagTC0[2].edgeFlag.b0.lineEdgeFlag = (	pLuma->eFlagTC0[2].edgeFlag.eFlag0 | 
		pLuma->eFlagTC0[2].edgeFlag.eFlag1 | 
		pLuma->eFlagTC0[2].edgeFlag.eFlag2 | 
		pLuma->eFlagTC0[2].edgeFlag.eFlag3 );
	pLuma->eFlagTC0[2].edgeFlag.b0.mbField = fieldQ;
	//	if( StrengthSumV[2])	// only if one of the 16 Strength bytes is != 0
	//	{
	//EdgeLoop_luma_v(SrcYQ+8, StrengthV[2], fieldQ, IndexA, IndexB, inc_y, 0);
	pLuma->eFlagTC0[3].tC0[0] = CLIP_TAB[IndexA[0]][StrengthV[2][0]];
	pLuma->eFlagTC0[3].tC0[1] = CLIP_TAB[IndexA[0]][StrengthV[2][1]];
	pLuma->eFlagTC0[3].tC0[2] = CLIP_TAB[IndexA[0]][StrengthV[2][2]];
	pLuma->eFlagTC0[3].tC0[3] = CLIP_TAB[IndexA[0]][StrengthV[2][3]];
	pLuma->eFlagTC0[3].edgeFlag.eFlag0 = StrengthV[2][0] == 1 ? StrengthV[2][0] : (StrengthV[2][0] >> 1);
	pLuma->eFlagTC0[3].edgeFlag.eFlag1 = StrengthV[2][1] == 1 ? StrengthV[2][1] : (StrengthV[2][1] >> 1);
	pLuma->eFlagTC0[3].edgeFlag.eFlag2 = StrengthV[2][2] == 1 ? StrengthV[2][2] : (StrengthV[2][2] >> 1);
	pLuma->eFlagTC0[3].edgeFlag.eFlag3 = StrengthV[2][3] == 1 ? StrengthV[2][3] : (StrengthV[2][3] >> 1);
	pLuma->eFlagTC0[3].edgeFlag.b0.lineEdgeFlag = (	pLuma->eFlagTC0[3].edgeFlag.eFlag0 | 
		pLuma->eFlagTC0[3].edgeFlag.eFlag1 | 
		pLuma->eFlagTC0[3].edgeFlag.eFlag2 | 
		pLuma->eFlagTC0[3].edgeFlag.eFlag3 );
	pLuma->eFlagTC0[3].edgeFlag.b0.mbField = fieldQ;
	//if( SrcUQ )	// check imgU for both UV
	//EdgeLoop_chromaUV_v(SrcUQ+4, SrcVQ+4, StrengthV[2], fieldQ, IndexA+1, IndexB+1, inc_ch, 0) ;
	pChroma->tC0_Cb[2][0] = CLIP_TAB[IndexA[1]][StrengthV[2][0]];
	pChroma->tC0_Cb[2][1] = CLIP_TAB[IndexA[1]][StrengthV[2][1]];
	pChroma->tC0_Cb[2][2] = CLIP_TAB[IndexA[1]][StrengthV[2][2]];
	pChroma->tC0_Cb[2][3] = CLIP_TAB[IndexA[1]][StrengthV[2][3]];
	pChroma->tC0_Cr[2][0] = CLIP_TAB[IndexA[1]][StrengthV[2][0]];
	pChroma->tC0_Cr[2][1] = CLIP_TAB[IndexA[1]][StrengthV[2][1]];
	pChroma->tC0_Cr[2][2] = CLIP_TAB[IndexA[1]][StrengthV[2][2]];
	pChroma->tC0_Cr[2][3] = CLIP_TAB[IndexA[1]][StrengthV[2][3]];
	pChroma->edgeFlag[2].eFlag0 = StrengthV[2][0] == 1 ? StrengthV[2][0] : (StrengthV[2][0] >> 1);
	pChroma->edgeFlag[2].eFlag1 = StrengthV[2][1] == 1 ? StrengthV[2][1] : (StrengthV[2][1] >> 1);
	pChroma->edgeFlag[2].eFlag2 = StrengthV[2][2] == 1 ? StrengthV[2][2] : (StrengthV[2][2] >> 1);
	pChroma->edgeFlag[2].eFlag3 = StrengthV[2][3] == 1 ? StrengthV[2][3] : (StrengthV[2][3] >> 1);
	pChroma->edgeFlag[2].b0.lineEdgeFlag = (	pChroma->edgeFlag[2].eFlag0 |
		pChroma->edgeFlag[2].eFlag1 |
		pChroma->edgeFlag[2].eFlag2 |
		pChroma->edgeFlag[2].eFlag3 );
	pChroma->edgeFlag[2].b0.mbField = fieldQ;

	//	}
	//	if( !MbQ->luma_transform_size_8x8_flag && StrengthSumV[3] )	// only if one of the 16 Strength bytes is != 0
	//EdgeLoop_luma_v(SrcYQ+12, StrengthV[3], fieldQ, IndexA, IndexB, inc_y, 0);
	pLuma->eFlagTC0[4].tC0[0] = CLIP_TAB[IndexA[0]][StrengthV[3][0]];
	pLuma->eFlagTC0[4].tC0[1] = CLIP_TAB[IndexA[0]][StrengthV[3][1]];
	pLuma->eFlagTC0[4].tC0[2] = CLIP_TAB[IndexA[0]][StrengthV[3][2]];
	pLuma->eFlagTC0[4].tC0[3] = CLIP_TAB[IndexA[0]][StrengthV[3][3]];
	pLuma->eFlagTC0[4].edgeFlag.eFlag0 = StrengthV[3][0] == 1 ? StrengthV[3][0] : (StrengthV[3][0] >> 1);
	pLuma->eFlagTC0[4].edgeFlag.eFlag1 = StrengthV[3][1] == 1 ? StrengthV[3][1] : (StrengthV[3][1] >> 1);
	pLuma->eFlagTC0[4].edgeFlag.eFlag2 = StrengthV[3][2] == 1 ? StrengthV[3][2] : (StrengthV[3][2] >> 1);
	pLuma->eFlagTC0[4].edgeFlag.eFlag3 = StrengthV[3][3] == 1 ? StrengthV[3][3] : (StrengthV[3][3] >> 1);
	pLuma->eFlagTC0[4].edgeFlag.b0.lineEdgeFlag = (	pLuma->eFlagTC0[4].edgeFlag.eFlag0 | 
		pLuma->eFlagTC0[4].edgeFlag.eFlag1 | 
		pLuma->eFlagTC0[4].edgeFlag.eFlag2 | 
		pLuma->eFlagTC0[4].edgeFlag.eFlag3 );
	pLuma->eFlagTC0[4].edgeFlag.b0.mbField = fieldQ;
	//end Vertical deblocking

	//Dump(m_lpDEBLK_Luma, sizeof(EdgeDataLumaMBAFF), "DEBLK_Luma_mbaff.dat");
	//Dump(m_lpDEBLK_Chroma, sizeof(EdgeDataChromaMBAFF), "DEBLK_Chroma_mbaff.dat");
	IMGPAR m_lpDEBLK_Luma += sizeof(EdgeDataLumaMBAFF);
	IMGPAR m_lpDEBLK_Chroma += sizeof(EdgeDataChromaMBAFF);
	pLuma = (EdgeDataLumaMBAFF*)IMGPAR m_lpDEBLK_Luma;
	pChroma = (EdgeDataChromaMBAFF*)IMGPAR m_lpDEBLK_Chroma;

	//Horizontal external deblocking
	// get internal index first
	IndexA[0] = __fast_iclip0_X(51, QpQ + LEAoffset);	
	IndexB[0] = __fast_iclip0_X(51, QpQ + LEBoffset);
	pLuma->AlphaInner = ALPHA_TABLE[IndexA[0]];
	pLuma->BetaInner	= BETA_TABLE[IndexB[0]];

	QP = QP_SCALE_CR[CQPOF(QpQ,0)];			// we do not handle UV offset, not in High profile
	IndexA[1] = __fast_iclip0_X(51, QP + LEAoffset);	
	IndexB[1] = __fast_iclip0_X(51, QP + LEBoffset);
	pChroma->AlphaInner_Cb = ALPHA_TABLE[IndexA[1]];
	pChroma->BetaInner_Cb	= BETA_TABLE[IndexB[1]];
	pChroma->AlphaInner_Cr = ALPHA_TABLE[IndexA[1]];
	pChroma->BetaInner_Cr	= BETA_TABLE[IndexB[1]];
	//	if( StrengthSumH[0])
	if(filterTopMbEdgeFlag)
	{
		QP = (QpQ+MbPH->qp+1)>>1;
		IndexA4[0] = __fast_iclip0_X(51, QP + LEAoffset);	
		IndexB4[0] = __fast_iclip0_X(51, QP + LEBoffset);
		pLuma->AlphaEdge = ALPHA_TABLE[IndexA4[0]];
		pLuma->BetaEdge	= BETA_TABLE[IndexB4[0]];

		QP = (QP_SCALE_CR[CQPOF(QpQ,0)] + QP_SCALE_CR[CQPOF(MbPH->qp,0)] + 1) >> 1;
		IndexA4[1] = __fast_iclip0_X(51, QP + LEAoffset);	
		IndexB4[1] = __fast_iclip0_X(51, QP + LEBoffset);
		pChroma->AlphaEdge_Cb = ALPHA_TABLE[IndexA4[1]];
		pChroma->BetaEdge_Cb	= BETA_TABLE[IndexB4[1]];
		pChroma->AlphaEdge_Cr = ALPHA_TABLE[IndexA4[1]];
		pChroma->BetaEdge_Cr	= BETA_TABLE[IndexB4[1]];

		//EdgeLoop_luma_h(SrcYQ, StrengthH[0], IndexA4, IndexB4, inc_y<<bExtraEdge);
		pLuma->eFlagTC0[1].tC0[0] = CLIP_TAB[IndexA4[0]][StrengthH[0][0]];
		pLuma->eFlagTC0[1].tC0[1] = CLIP_TAB[IndexA4[0]][StrengthH[0][1]];
		pLuma->eFlagTC0[1].tC0[2] = CLIP_TAB[IndexA4[0]][StrengthH[0][2]];
		pLuma->eFlagTC0[1].tC0[3] = CLIP_TAB[IndexA4[0]][StrengthH[0][3]];
		pLuma->eFlagTC0[1].edgeFlag.eFlag0 = StrengthH[0][0] == 1 ? StrengthH[0][0] : (StrengthH[0][0] >> 1);
		pLuma->eFlagTC0[1].edgeFlag.eFlag1 = StrengthH[0][1] == 1 ? StrengthH[0][1] : (StrengthH[0][1] >> 1);
		pLuma->eFlagTC0[1].edgeFlag.eFlag2 = StrengthH[0][2] == 1 ? StrengthH[0][2] : (StrengthH[0][2] >> 1);
		pLuma->eFlagTC0[1].edgeFlag.eFlag3 = StrengthH[0][3] == 1 ? StrengthH[0][3] : (StrengthH[0][3] >> 1);
		pLuma->eFlagTC0[1].edgeFlag.b0.lineEdgeFlag = (	pLuma->eFlagTC0[1].edgeFlag.eFlag0 | 
			pLuma->eFlagTC0[1].edgeFlag.eFlag1 | 
			pLuma->eFlagTC0[1].edgeFlag.eFlag2 | 
			pLuma->eFlagTC0[1].edgeFlag.eFlag3 );
		pLuma->eFlagTC0[1].edgeFlag.b0.mbField = fieldQ;

		//if( SrcUQ )	// check imgU for both UV
		//EdgeLoop_chromaUV_h(SrcUQ, SrcVQ, StrengthH[0], IndexA4+1, IndexB4+1, inc_ch<<bExtraEdge) ;
		pChroma->tC0_Cb[1][0] = CLIP_TAB[IndexA4[1]][StrengthH[0][0]];
		pChroma->tC0_Cb[1][1] = CLIP_TAB[IndexA4[1]][StrengthH[0][1]];
		pChroma->tC0_Cb[1][2] = CLIP_TAB[IndexA4[1]][StrengthH[0][2]];
		pChroma->tC0_Cb[1][3] = CLIP_TAB[IndexA4[1]][StrengthH[0][3]];
		pChroma->tC0_Cr[1][0] = CLIP_TAB[IndexA4[1]][StrengthH[0][0]];
		pChroma->tC0_Cr[1][1] = CLIP_TAB[IndexA4[1]][StrengthH[0][1]];
		pChroma->tC0_Cr[1][2] = CLIP_TAB[IndexA4[1]][StrengthH[0][2]];
		pChroma->tC0_Cr[1][3] = CLIP_TAB[IndexA4[1]][StrengthH[0][3]];
		pChroma->edgeFlag[1].eFlag0 = StrengthH[0][0] == 1 ? StrengthH[0][0] : (StrengthH[0][0] >> 1);
		pChroma->edgeFlag[1].eFlag1 = StrengthH[0][1] == 1 ? StrengthH[0][1] : (StrengthH[0][1] >> 1);
		pChroma->edgeFlag[1].eFlag2 = StrengthH[0][2] == 1 ? StrengthH[0][2] : (StrengthH[0][2] >> 1);
		pChroma->edgeFlag[1].eFlag3 = StrengthH[0][3] == 1 ? StrengthH[0][3] : (StrengthH[0][3] >> 1);
		pChroma->edgeFlag[1].b0.lineEdgeFlag = (	pChroma->edgeFlag[1].eFlag0 |
			pChroma->edgeFlag[1].eFlag1 |
			pChroma->edgeFlag[1].eFlag2 |
			pChroma->edgeFlag[1].eFlag3 );
		pChroma->edgeFlag[1].b0.mbField = fieldQ;
	}

	// Extra horizontal edge between a frame macroblock pair and a field above it
	//if( StrengthSumH[4] )
	if(bExtraEdge)
	{
		MbPH++;
		QP = (QpQ+MbPH->qp+1)>>1;
		IndexA4[0] = __fast_iclip0_X(51, QP + LEAoffset);	
		IndexB4[0] = __fast_iclip0_X(51, QP + LEBoffset);
		pLuma->AlphaEdge2 = ALPHA_TABLE[IndexA4[0]];
		pLuma->BetaEdge2	= BETA_TABLE[IndexB4[0]];

		QP = (QP_SCALE_CR[CQPOF(QpQ,0)] + QP_SCALE_CR[CQPOF(MbPH->qp,0)] + 1) >> 1;
		IndexA4[1] = __fast_iclip0_X(51, QP + LEAoffset);	
		IndexB4[1] = __fast_iclip0_X(51, QP + LEBoffset);
		pChroma->AlphaEdge2_Cb = ALPHA_TABLE[IndexA4[1]];
		pChroma->BetaEdge2_Cb	= BETA_TABLE[IndexB4[1]];
		pChroma->AlphaEdge2_Cr = ALPHA_TABLE[IndexA4[1]];
		pChroma->BetaEdge2_Cr	= BETA_TABLE[IndexB4[1]];

		//EdgeLoop_luma_h(SrcYQ+inc_y, StrengthH[4], IndexA4, IndexB4, inc_y<<bExtraEdge) ;
		pLuma->eFlagTC0[0].tC0[0] = CLIP_TAB[IndexA4[0]][StrengthH[4][0]];
		pLuma->eFlagTC0[0].tC0[1] = CLIP_TAB[IndexA4[0]][StrengthH[4][1]];
		pLuma->eFlagTC0[0].tC0[2] = CLIP_TAB[IndexA4[0]][StrengthH[4][2]];
		pLuma->eFlagTC0[0].tC0[3] = CLIP_TAB[IndexA4[0]][StrengthH[4][3]];
		pLuma->eFlagTC0[0].edgeFlag.eFlag0 = StrengthH[4][0] == 1 ? StrengthH[4][0] : (StrengthH[4][0] >> 1);
		pLuma->eFlagTC0[0].edgeFlag.eFlag1 = StrengthH[4][1] == 1 ? StrengthH[4][1] : (StrengthH[4][1] >> 1);
		pLuma->eFlagTC0[0].edgeFlag.eFlag2 = StrengthH[4][2] == 1 ? StrengthH[4][2] : (StrengthH[4][2] >> 1);
		pLuma->eFlagTC0[0].edgeFlag.eFlag3 = StrengthH[4][3] == 1 ? StrengthH[4][3] : (StrengthH[4][3] >> 1);
		pLuma->eFlagTC0[0].edgeFlag.b0.lineEdgeFlag = (	pLuma->eFlagTC0[0].edgeFlag.eFlag0 | 
			pLuma->eFlagTC0[0].edgeFlag.eFlag1 | 
			pLuma->eFlagTC0[0].edgeFlag.eFlag2 | 
			pLuma->eFlagTC0[0].edgeFlag.eFlag3 );
		pLuma->eFlagTC0[0].edgeFlag.b0.mbField = fieldQ;
		pLuma->eFlagTC0[0].edgeFlag.b0.twoTopEdges = 1;
		//if( SrcUQ )	// check imgU for both UV
		//EdgeLoop_chromaUV_h(SrcUQ+inc_ch, SrcVQ+inc_ch, StrengthH[4], IndexA4+1, IndexB4+1, inc_ch<<bExtraEdge) ;
		pChroma->tC0_Cb[0][0] = CLIP_TAB[IndexA4[1]][StrengthH[4][0]];
		pChroma->tC0_Cb[0][1] = CLIP_TAB[IndexA4[1]][StrengthH[4][1]];
		pChroma->tC0_Cb[0][2] = CLIP_TAB[IndexA4[1]][StrengthH[4][2]];
		pChroma->tC0_Cb[0][3] = CLIP_TAB[IndexA4[1]][StrengthH[4][3]];
		pChroma->tC0_Cr[0][0] = CLIP_TAB[IndexA4[1]][StrengthH[4][0]];
		pChroma->tC0_Cr[0][1] = CLIP_TAB[IndexA4[1]][StrengthH[4][1]];
		pChroma->tC0_Cr[0][2] = CLIP_TAB[IndexA4[1]][StrengthH[4][2]];
		pChroma->tC0_Cr[0][3] = CLIP_TAB[IndexA4[1]][StrengthH[4][3]];
		pChroma->edgeFlag[0].eFlag0 = StrengthH[4][0] == 1 ? StrengthH[4][0] : (StrengthH[4][0] >> 1);
		pChroma->edgeFlag[0].eFlag1 = StrengthH[4][1] == 1 ? StrengthH[4][1] : (StrengthH[4][1] >> 1);
		pChroma->edgeFlag[0].eFlag2 = StrengthH[4][2] == 1 ? StrengthH[4][2] : (StrengthH[4][2] >> 1);
		pChroma->edgeFlag[0].eFlag3 = StrengthH[4][3] == 1 ? StrengthH[4][3] : (StrengthH[4][3] >> 1);
		pChroma->edgeFlag[0].b0.lineEdgeFlag = (	pChroma->edgeFlag[0].eFlag0 |
			pChroma->edgeFlag[0].eFlag1 |
			pChroma->edgeFlag[0].eFlag2 |
			pChroma->edgeFlag[0].eFlag3 );
		pChroma->edgeFlag[0].b0.mbField = fieldQ;
		pChroma->edgeFlag[0].b0.twoTopEdges = 1;
	}

	//Horizontal internal deblocking
	//	SrcYQ += inc_y4;
	//	if( !MbQ->luma_transform_size_8x8_flag && StrengthSumH[1] )
	//EdgeLoop_luma_h(SrcYQ, StrengthH[1], IndexA, IndexB, inc_y);
	pLuma->eFlagTC0[2].tC0[0] = CLIP_TAB[IndexA[0]][StrengthH[1][0]];
	pLuma->eFlagTC0[2].tC0[1] = CLIP_TAB[IndexA[0]][StrengthH[1][1]];
	pLuma->eFlagTC0[2].tC0[2] = CLIP_TAB[IndexA[0]][StrengthH[1][2]];
	pLuma->eFlagTC0[2].tC0[3] = CLIP_TAB[IndexA[0]][StrengthH[1][3]];
	pLuma->eFlagTC0[2].edgeFlag.eFlag0 = StrengthH[1][0] == 1 ? StrengthH[1][0] : (StrengthH[1][0] >> 1);
	pLuma->eFlagTC0[2].edgeFlag.eFlag1 = StrengthH[1][1] == 1 ? StrengthH[1][1] : (StrengthH[1][1] >> 1);
	pLuma->eFlagTC0[2].edgeFlag.eFlag2 = StrengthH[1][2] == 1 ? StrengthH[1][2] : (StrengthH[1][2] >> 1);
	pLuma->eFlagTC0[2].edgeFlag.eFlag3 = StrengthH[1][3] == 1 ? StrengthH[1][3] : (StrengthH[1][3] >> 1);
	pLuma->eFlagTC0[2].edgeFlag.b0.lineEdgeFlag = (	pLuma->eFlagTC0[2].edgeFlag.eFlag0 | 
		pLuma->eFlagTC0[2].edgeFlag.eFlag1 | 
		pLuma->eFlagTC0[2].edgeFlag.eFlag2 | 
		pLuma->eFlagTC0[2].edgeFlag.eFlag3 );
	pLuma->eFlagTC0[2].edgeFlag.b0.mbField = fieldQ; 
	//	SrcYQ += inc_y4;
	//	if( StrengthSumH[2])
	//	{
	//EdgeLoop_luma_h(SrcYQ, StrengthH[2], IndexA, IndexB, inc_y);
	pLuma->eFlagTC0[3].tC0[0] = CLIP_TAB[IndexA[0]][StrengthH[2][0]];
	pLuma->eFlagTC0[3].tC0[1] = CLIP_TAB[IndexA[0]][StrengthH[2][1]];
	pLuma->eFlagTC0[3].tC0[2] = CLIP_TAB[IndexA[0]][StrengthH[2][2]];
	pLuma->eFlagTC0[3].tC0[3] = CLIP_TAB[IndexA[0]][StrengthH[2][3]];
	pLuma->eFlagTC0[3].edgeFlag.eFlag0 = StrengthH[2][0] == 1 ? StrengthH[2][0] : (StrengthH[2][0] >> 1);
	pLuma->eFlagTC0[3].edgeFlag.eFlag1 = StrengthH[2][1] == 1 ? StrengthH[2][1] : (StrengthH[2][1] >> 1);
	pLuma->eFlagTC0[3].edgeFlag.eFlag2 = StrengthH[2][2] == 1 ? StrengthH[2][2] : (StrengthH[2][2] >> 1);
	pLuma->eFlagTC0[3].edgeFlag.eFlag3 = StrengthH[2][3] == 1 ? StrengthH[2][3] : (StrengthH[2][3] >> 1);
	pLuma->eFlagTC0[3].edgeFlag.b0.lineEdgeFlag = (	pLuma->eFlagTC0[3].edgeFlag.eFlag0 | 
		pLuma->eFlagTC0[3].edgeFlag.eFlag1 | 
		pLuma->eFlagTC0[3].edgeFlag.eFlag2 | 
		pLuma->eFlagTC0[3].edgeFlag.eFlag3 );
	pLuma->eFlagTC0[3].edgeFlag.b0.mbField = fieldQ;
	//if( SrcUQ )	// check imgU for both UV
	//EdgeLoop_chromaUV_h(SrcUQ+(inc_ch<<2), SrcVQ+(inc_ch<<2), StrengthH[2], IndexA+1, IndexB+1, inc_ch) ;
	pChroma->tC0_Cb[2][0] = CLIP_TAB[IndexA[1]][StrengthH[2][0]];
	pChroma->tC0_Cb[2][1] = CLIP_TAB[IndexA[1]][StrengthH[2][1]];
	pChroma->tC0_Cb[2][2] = CLIP_TAB[IndexA[1]][StrengthH[2][2]];
	pChroma->tC0_Cb[2][3] = CLIP_TAB[IndexA[1]][StrengthH[2][3]];
	pChroma->tC0_Cr[2][0] = CLIP_TAB[IndexA[1]][StrengthH[2][0]];
	pChroma->tC0_Cr[2][1] = CLIP_TAB[IndexA[1]][StrengthH[2][1]];
	pChroma->tC0_Cr[2][2] = CLIP_TAB[IndexA[1]][StrengthH[2][2]];
	pChroma->tC0_Cr[2][3] = CLIP_TAB[IndexA[1]][StrengthH[2][3]];
	pChroma->edgeFlag[2].eFlag0 = StrengthH[2][0] == 1 ? StrengthH[2][0] : (StrengthH[2][0] >> 1);
	pChroma->edgeFlag[2].eFlag1 = StrengthH[2][1] == 1 ? StrengthH[2][1] : (StrengthH[2][1] >> 1);
	pChroma->edgeFlag[2].eFlag2 = StrengthH[2][2] == 1 ? StrengthH[2][2] : (StrengthH[2][2] >> 1);
	pChroma->edgeFlag[2].eFlag3 = StrengthH[2][3] == 1 ? StrengthH[2][3] : (StrengthH[2][3] >> 1);
	pChroma->edgeFlag[2].b0.lineEdgeFlag = (	pChroma->edgeFlag[2].eFlag0 |
		pChroma->edgeFlag[2].eFlag1 |
		pChroma->edgeFlag[2].eFlag2 |
		pChroma->edgeFlag[2].eFlag3 );
	pChroma->edgeFlag[2].b0.mbField = fieldQ;
	//	}
	//	SrcYQ += inc_y4;
	//	if( !MbQ->luma_transform_size_8x8_flag && StrengthSumH[3] )
	//EdgeLoop_luma_h(SrcYQ, StrengthH[3], IndexA, IndexB, inc_y);
	pLuma->eFlagTC0[4].tC0[0] = CLIP_TAB[IndexA[0]][StrengthH[3][0]];
	pLuma->eFlagTC0[4].tC0[1] = CLIP_TAB[IndexA[0]][StrengthH[3][1]];
	pLuma->eFlagTC0[4].tC0[2] = CLIP_TAB[IndexA[0]][StrengthH[3][2]];
	pLuma->eFlagTC0[4].tC0[3] = CLIP_TAB[IndexA[0]][StrengthH[3][3]];
	pLuma->eFlagTC0[4].edgeFlag.eFlag0 = StrengthH[3][0] == 1 ? StrengthH[3][0] : (StrengthH[3][0] >> 1);
	pLuma->eFlagTC0[4].edgeFlag.eFlag1 = StrengthH[3][1] == 1 ? StrengthH[3][1] : (StrengthH[3][1] >> 1);
	pLuma->eFlagTC0[4].edgeFlag.eFlag2 = StrengthH[3][2] == 1 ? StrengthH[3][2] : (StrengthH[3][2] >> 1);
	pLuma->eFlagTC0[4].edgeFlag.eFlag3 = StrengthH[3][3] == 1 ? StrengthH[3][3] : (StrengthH[3][3] >> 1);
	pLuma->eFlagTC0[4].edgeFlag.b0.lineEdgeFlag = (	pLuma->eFlagTC0[4].edgeFlag.eFlag0 | 
		pLuma->eFlagTC0[4].edgeFlag.eFlag1 | 
		pLuma->eFlagTC0[4].edgeFlag.eFlag2 | 
		pLuma->eFlagTC0[4].edgeFlag.eFlag3 );
	pLuma->eFlagTC0[4].edgeFlag.b0.mbField = fieldQ;
	//end horizontal deblocking
	//Dump(m_lpDEBLK_Luma, sizeof(EdgeDataLumaMBAFF), "DEBLK_Luma_mbaff.dat");
	//Dump(m_lpDEBLK_Chroma, sizeof(EdgeDataChromaMBAFF), "DEBLK_Chroma_mbaff.dat");
	IMGPAR m_lpDEBLK_Luma += sizeof(EdgeDataLumaMBAFF);
	IMGPAR m_lpDEBLK_Chroma += sizeof(EdgeDataChromaMBAFF);
}
//#else
void CH264DXVA1_NV::build_deblocking_control_buffer_mbaff_sse2 PARGS7(StorablePicture *p, int MbQAddr, int mbx, int mby, Macroblock_s *Mb_left, Macroblock_s *Mb_top, Macroblock_s	*MbQ)
{
	bool		mixedModeEdgeFlagV=0;
	int				 mvlimit, bExtraEdge=0;
	int			LEAoffset, LEBoffset;
	int				 filterLeftMbEdgeFlag, filterTopMbEdgeFlag;
	int			fieldQ, QpQ;
	int			QP, IndexA[3], IndexB[3], IndexA4[6], IndexB4[6];
	byte				StrengthV[4][16],StrengthH[5][4];
	Macroblock_s	*MbPV=0,*MbPH=0;

	EdgeDataLumaMBAFF* pLuma = (EdgeDataLumaMBAFF*)IMGPAR m_lpDEBLK_Luma;
	EdgeDataChromaMBAFF* pChroma = (EdgeDataChromaMBAFF*)IMGPAR m_lpDEBLK_Chroma;
	memset(StrengthV,0,64);
	memset(StrengthH,0,20);

	QpQ = MbQ->qp;
	fieldQ = MbQ->mb_field;
	LEAoffset = IMGPAR currentSlice->LFAlphaC0Offset;
	LEBoffset = IMGPAR currentSlice->LFBetaOffset;
	mvlimit = 4 >> (int) ((p->structure!=FRAME) || (p->MbaffFrameFlag && MbQ->mb_field)); // 2 if field mode

	filterLeftMbEdgeFlag	= (mbx != 0);
	filterTopMbEdgeFlag	 = (mby != 0) && !(p->MbaffFrameFlag && mby==1 && MbQ->mb_field);

	if (IMGPAR currentSlice->LFDisableIdc==2)
	{
		filterLeftMbEdgeFlag = MbQ->filterLeftMbEdgeFlag;
		filterTopMbEdgeFlag	= MbQ->filterTopMbEdgeFlag;
	}

	// Start getting strength
	// Vertical external strength
	if(filterLeftMbEdgeFlag)
	{
		MbPV = Mb_left;	// temporary one
		if(fieldQ != MbPV->mb_field)
		{
			// mixed edge
			MbPV=Mb_left;
			mixedModeEdgeFlagV=1;
		}
		else
			MbPV=Mb_left+(p->MbaffFrameFlag&&(MbQAddr&1));		
		fp_GetStrength_v(StrengthV[0],MbQ,MbQAddr,mbx,mby,MbPV, 0, mvlimit, mixedModeEdgeFlagV); // Strength for 4 blks in 1 stripe
	}
	// Vertical internal strength
	if(MbQ->luma_transform_size_8x8_flag)		
		fp_GetStrengthInternal_v(StrengthV[2],MbQ,2, mvlimit); // Strength for 4 blks in 1 stripe
	else
	{
		static int	StrengthSumV[4];
		fp_GetStrengthInternal_v_3(StrengthSumV, StrengthV[0], MbQ, mvlimit); // Strength for 4 blks in 1 stripe
	}

	//use StrengthV[];
	{	
		__m64 mm0,	mm1,	mm2,	mm3, mm4, mm_1_const;
		__m128i xmm0, xmm1, xmm2;
		__declspec(align(16)) const static short ncoeff_1_const[] = {1, 1, 1, 1};
		__declspec(align(16)) const static unsigned short ncoeff_EdgeFlagByte[] = {0xFF1F, 0xFFFF, 0xFF1F, 0xFFFF, 0xFF1F, 0xFFFF, 0xFF1F, 0xFFFF};
		__declspec(align(16)) const static unsigned short ncoeff_mbField[] = {0x0020, 0x0000, 0x0020, 0x0000, 0x0020, 0x0000, 0x0020, 0x0000};
		__declspec(align(16)) const static unsigned short ncoeff_2bits[] = {0x0003, 0x0000, 0x0003, 0x0000, 0x0003, 0x0000, 0x0003, 0x0000};

		mm4 = _mm_setzero_si64();
		mm_1_const = *((__m64*)&ncoeff_1_const[0]);
		mm0 = *((__m64*)&StrengthV[0][0]);
		mm1 = *((__m64*)&StrengthV[1][0]);
		mm2 = *((__m64*)&StrengthV[2][0]);
		mm3 = *((__m64*)&StrengthV[3][0]);

		mm0 = _mm_unpacklo_pi8(mm0, mm4);
		mm1 = _mm_unpacklo_pi8(mm1, mm4);
		mm2 = _mm_unpacklo_pi8(mm2, mm4);
		mm3 = _mm_unpacklo_pi8(mm3, mm4);

		mm4 = _mm_and_si64(mm0, mm_1_const);
		mm0 = _mm_srli_pi16 (mm0, 1);
		mm0 = _mm_or_si64(mm0, mm4);

		mm4 = _mm_and_si64(mm1, mm_1_const);
		mm1 = _mm_srli_pi16 (mm1, 1);
		mm1 = _mm_or_si64(mm1, mm4);

		mm4 = _mm_and_si64(mm2, mm_1_const);
		mm2 = _mm_srli_pi16 (mm2, 1);
		mm2 = _mm_or_si64(mm2, mm4);

		mm4 = _mm_and_si64(mm3, mm_1_const);
		mm3 = _mm_srli_pi16 (mm3, 1);
		mm3 = _mm_or_si64(mm3, mm4);

		mm4 = _mm_packs_pi16(mm0, mm1);
		mm0 = _mm_packs_pi16(mm2, mm3);

		//get 4 array into xmm.
		xmm0 = _mm_movpi64_epi64(mm4);
		xmm1 = _mm_movpi64_epi64(mm0);
		xmm1 = _mm_slli_si128(xmm1, 8);
		xmm0 = _mm_or_si128(xmm0, xmm1); 

		//get "edgeFlag.b0.lineEdgeFlag".
		xmm1 = _mm_srli_si128(xmm0, 2);
		xmm1 = _mm_or_si128(xmm1, xmm0);
		xmm2 = _mm_srli_si128(xmm1, 1);
		xmm2 = _mm_or_si128(xmm1, xmm2);
		xmm1 = *((__m128i*)&ncoeff_2bits[0]);
		xmm1 = _mm_and_si128(xmm1, xmm2);
		xmm1 = _mm_slli_epi32(xmm1, 6); //shift 6-bits 

		//get original flag without "lineEdgeFlag" and "mbField"
		xmm2 = *((__m128i*)&ncoeff_EdgeFlagByte[0]);
		xmm2 = _mm_and_si128(xmm2, xmm0);

		//combine "lineEdgeFlag" to original flag.
		xmm2 = _mm_or_si128(xmm2, xmm1);

		//set mbField value.
		xmm1 = *((__m128i*)&ncoeff_mbField[0]);
		xmm1 = _mm_srli_epi32(xmm1, ((!fieldQ)<<3));

		//combine "mbField" to original flag.
		xmm2 = _mm_or_si128(xmm2, xmm1);

		//get lower 64-bits.
		mm0 = _mm_movepi64_pi64(xmm2);
		mm1 = _mm_srli_si64(mm0, 32);

		//get higher 64-bits.
		xmm2 = _mm_srli_si128(xmm2, 8);
		mm2 = _mm_movepi64_pi64(xmm2);
		mm3 = _mm_srli_si64(mm2, 32);

		*((__m64*)&(pLuma->eFlagTC0[1].edgeFlag.eFlag0)) = mm0;
		*((__m64*)&(pLuma->eFlagTC0[2].edgeFlag.eFlag0)) = mm1;
		*((__m64*)&(pLuma->eFlagTC0[3].edgeFlag.eFlag0)) = mm2;
		*((__m64*)&(pLuma->eFlagTC0[4].edgeFlag.eFlag0)) = mm3;
		*((__m64*)&(pChroma->edgeFlag[1].eFlag0)) = mm0;
		*((__m64*)&(pChroma->edgeFlag[2].eFlag0)) = mm2;
	}

	// Horizontal external strength
	if(filterTopMbEdgeFlag)
	{
		MbPH = Mb_top;	// temporary one
		if(!p->MbaffFrameFlag)
			MbPH = Mb_top;
		else if(fieldQ)
			MbPH = Mb_top + ((MbQAddr&1) || (!MbPH->mb_field));
		else if(MbQAddr&1)
			MbPH = MbQ-1;
		else
		{
			bExtraEdge = MbPH->mb_field;
			MbPH = Mb_top + !bExtraEdge;
		}
		fp_GetStrength_h ARGS9(StrengthH[0],MbQ,MbQAddr,mbx,mby,MbPH, 0, mvlimit, p); // Strength for 4 blks in 1 stripe
		if (bExtraEdge)	// this is the extra horizontal edge between a frame macroblock pair and a field above it
			fp_GetStrength_h ARGS9(StrengthH[4],MbQ,MbQAddr,mbx,mby,MbPH+1, 4, mvlimit, p); // Strength for 4 blks in 1 stripe
	}
	// Horizontal internal strength
	if(MbQ->luma_transform_size_8x8_flag)
		fp_GetStrengthInternal_h(StrengthH[2],MbQ,2, mvlimit); // Strength for 4 blks in 1 stripe
	else
	{
		static int StrengthSumH[5];
		fp_GetStrengthInternal_h_3(StrengthSumH, StrengthH[0], MbQ, mvlimit); // Strength for 4 blks in 1 stripe
	}//end horizontal edge

	// Start Deblocking
	// get internal index first
	IndexA[0] = __fast_iclip0_X(51, QpQ + LEAoffset);	
	IndexB[0] = __fast_iclip0_X(51, QpQ + LEBoffset);
	pLuma->AlphaInner = ALPHA_TABLE[IndexA[0]];
	pLuma->BetaInner	= BETA_TABLE[IndexB[0]];

	QP = QP_SCALE_CR[CQPOF(QpQ,0)];			// we do not handle UV offset, not in High profile
	IndexA[1] = __fast_iclip0_X(51, QP + LEAoffset);	
	IndexB[1] = __fast_iclip0_X(51, QP + LEBoffset);
	pChroma->AlphaInner_Cb = ALPHA_TABLE[IndexA[1]];
	pChroma->BetaInner_Cb	= BETA_TABLE[IndexB[1]];
	pChroma->AlphaInner_Cr = ALPHA_TABLE[IndexA[1]];
	pChroma->BetaInner_Cr	= BETA_TABLE[IndexB[1]];

	//Vertical external deblocking
	//	if(StrengthSumV[0])	// only if one of the 16 Strength bytes is != 0
	if(filterLeftMbEdgeFlag)
	{
		QP = (QpQ+MbPV->qp+1)>>1;
		IndexA4[0] = __fast_iclip0_X(51, QP + LEAoffset);	
		IndexB4[0] = __fast_iclip0_X(51, QP + LEBoffset);
		pLuma->AlphaEdge = ALPHA_TABLE[IndexA4[0]];
		pLuma->BetaEdge	= BETA_TABLE[IndexB4[0]];

		QP = (QP_SCALE_CR[CQPOF(QpQ,0)] + QP_SCALE_CR[CQPOF(MbPV->qp,0)] + 1) >> 1;
		IndexA4[2] = __fast_iclip0_X(51, QP + LEAoffset);	
		IndexB4[2] = __fast_iclip0_X(51, QP + LEBoffset);
		pChroma->AlphaEdge_Cb = ALPHA_TABLE[IndexA4[2]];
		pChroma->BetaEdge_Cb	= BETA_TABLE[IndexB4[2]];
		pChroma->AlphaEdge_Cr = ALPHA_TABLE[IndexA4[2]];
		pChroma->BetaEdge_Cr	= BETA_TABLE[IndexB4[2]];

		if(mixedModeEdgeFlagV)
		{
			QP = (QpQ+(MbPV+1)->qp+1)>>1;
			IndexA4[1] = __fast_iclip0_X(51, QP + LEAoffset);	
			IndexB4[1] = __fast_iclip0_X(51, QP + LEBoffset);
			pLuma->AlphaEdge2 = ALPHA_TABLE[IndexA4[1]];
			pLuma->BetaEdge2	= BETA_TABLE[IndexB4[1]];

			QP = (QP_SCALE_CR[CQPOF(QpQ,0)] + QP_SCALE_CR[CQPOF((MbPV+1)->qp,0)] + 1) >> 1;
			IndexA4[3] = __fast_iclip0_X(51, QP + LEAoffset);	
			IndexB4[3] = __fast_iclip0_X(51, QP + LEBoffset);
			pChroma->AlphaEdge2_Cb = ALPHA_TABLE[IndexA4[3]];
			pChroma->BetaEdge2_Cb	= BETA_TABLE[IndexB4[3]];
			pChroma->AlphaEdge2_Cr = ALPHA_TABLE[IndexA4[3]];
			pChroma->BetaEdge2_Cr	= BETA_TABLE[IndexB4[3]];
		}

		//EdgeLoop_luma_v(SrcYQ, StrengthV[0], fieldQ, IndexA4, IndexB4, inc_y, mixedModeEdgeFlagV);
		pLuma->eFlagTC0[1].tC0[0] = CLIP_TAB[IndexA4[0]][StrengthV[0][0]];
		pLuma->eFlagTC0[1].tC0[1] = CLIP_TAB[IndexA4[0]][StrengthV[0][1]];
		pLuma->eFlagTC0[1].tC0[2] = CLIP_TAB[IndexA4[0]][StrengthV[0][2]];
		pLuma->eFlagTC0[1].tC0[3] = CLIP_TAB[IndexA4[0]][StrengthV[0][3]];

		//if( SrcUQ )	// check imgU for both UV
		//EdgeLoop_chromaUV_v(SrcUQ, SrcVQ, StrengthV[0], fieldQ, IndexA4+2, IndexB4+2, inc_ch, mixedModeEdgeFlagV) ;
		pChroma->tC0_Cb[1][0] = CLIP_TAB[IndexA4[2]][StrengthV[0][0]];
		pChroma->tC0_Cb[1][1] = CLIP_TAB[IndexA4[2]][StrengthV[0][1]];
		pChroma->tC0_Cb[1][2] = CLIP_TAB[IndexA4[2]][StrengthV[0][2]];
		pChroma->tC0_Cb[1][3] = CLIP_TAB[IndexA4[2]][StrengthV[0][3]];
		pChroma->tC0_Cr[1][0] = CLIP_TAB[IndexA4[2]][StrengthV[0][0]];
		pChroma->tC0_Cr[1][1] = CLIP_TAB[IndexA4[2]][StrengthV[0][1]];
		pChroma->tC0_Cr[1][2] = CLIP_TAB[IndexA4[2]][StrengthV[0][2]];
		pChroma->tC0_Cr[1][3] = CLIP_TAB[IndexA4[2]][StrengthV[0][3]];
	}
	//Vertical internal deblocking
	//	if( !MbQ->luma_transform_size_8x8_flag && StrengthSumV[1] )	// only if one of the 16 Strength bytes is != 0
	//EdgeLoop_luma_v(SrcYQ+4, StrengthV[1], fieldQ, IndexA, IndexB, inc_y, 0);
	pLuma->eFlagTC0[2].tC0[0] = CLIP_TAB[IndexA[0]][StrengthV[1][0]];
	pLuma->eFlagTC0[2].tC0[1] = CLIP_TAB[IndexA[0]][StrengthV[1][1]];
	pLuma->eFlagTC0[2].tC0[2] = CLIP_TAB[IndexA[0]][StrengthV[1][2]];
	pLuma->eFlagTC0[2].tC0[3] = CLIP_TAB[IndexA[0]][StrengthV[1][3]];
	//	if( StrengthSumV[2])	// only if one of the 16 Strength bytes is != 0
	//	{
	//EdgeLoop_luma_v(SrcYQ+8, StrengthV[2], fieldQ, IndexA, IndexB, inc_y, 0);
	pLuma->eFlagTC0[3].tC0[0] = CLIP_TAB[IndexA[0]][StrengthV[2][0]];
	pLuma->eFlagTC0[3].tC0[1] = CLIP_TAB[IndexA[0]][StrengthV[2][1]];
	pLuma->eFlagTC0[3].tC0[2] = CLIP_TAB[IndexA[0]][StrengthV[2][2]];
	pLuma->eFlagTC0[3].tC0[3] = CLIP_TAB[IndexA[0]][StrengthV[2][3]];
	//if( SrcUQ )	// check imgU for both UV
	//EdgeLoop_chromaUV_v(SrcUQ+4, SrcVQ+4, StrengthV[2], fieldQ, IndexA+1, IndexB+1, inc_ch, 0) ;
	pChroma->tC0_Cb[2][0] = CLIP_TAB[IndexA[1]][StrengthV[2][0]];
	pChroma->tC0_Cb[2][1] = CLIP_TAB[IndexA[1]][StrengthV[2][1]];
	pChroma->tC0_Cb[2][2] = CLIP_TAB[IndexA[1]][StrengthV[2][2]];
	pChroma->tC0_Cb[2][3] = CLIP_TAB[IndexA[1]][StrengthV[2][3]];
	pChroma->tC0_Cr[2][0] = CLIP_TAB[IndexA[1]][StrengthV[2][0]];
	pChroma->tC0_Cr[2][1] = CLIP_TAB[IndexA[1]][StrengthV[2][1]];
	pChroma->tC0_Cr[2][2] = CLIP_TAB[IndexA[1]][StrengthV[2][2]];
	pChroma->tC0_Cr[2][3] = CLIP_TAB[IndexA[1]][StrengthV[2][3]];
	//	}
	//	if( !MbQ->luma_transform_size_8x8_flag && StrengthSumV[3] )	// only if one of the 16 Strength bytes is != 0
	//EdgeLoop_luma_v(SrcYQ+12, StrengthV[3], fieldQ, IndexA, IndexB, inc_y, 0);
	pLuma->eFlagTC0[4].tC0[0] = CLIP_TAB[IndexA[0]][StrengthV[3][0]];
	pLuma->eFlagTC0[4].tC0[1] = CLIP_TAB[IndexA[0]][StrengthV[3][1]];
	pLuma->eFlagTC0[4].tC0[2] = CLIP_TAB[IndexA[0]][StrengthV[3][2]];
	pLuma->eFlagTC0[4].tC0[3] = CLIP_TAB[IndexA[0]][StrengthV[3][3]];
	//end Vertical deblocking

	//Dump(m_lpDEBLK_Luma, sizeof(EdgeDataLumaMBAFF), "DEBLK_Luma_mbaff.dat");
	//Dump(m_lpDEBLK_Chroma, sizeof(EdgeDataChromaMBAFF), "DEBLK_Chroma_mbaff.dat");
	IMGPAR m_lpDEBLK_Luma += sizeof(EdgeDataLumaMBAFF);
	IMGPAR m_lpDEBLK_Chroma += sizeof(EdgeDataChromaMBAFF);
	pLuma = (EdgeDataLumaMBAFF*)IMGPAR m_lpDEBLK_Luma;
	pChroma = (EdgeDataChromaMBAFF*)IMGPAR m_lpDEBLK_Chroma;

	//use StrengthH[];
	{	
		__m64 mm0,	mm1,	mm2,	mm3, mm4, mm5, mm_1_const;
		__m128i xmm0, xmm1, xmm2;
		__declspec(align(16)) const static short ncoeff_1_const[] = {1, 1, 1, 1};
		__declspec(align(16)) const static unsigned short ncoeff_EdgeFlagByte[] = {0xFF1F, 0xFFFF, 0xFF1F, 0xFFFF, 0xFF1F, 0xFFFF, 0xFF1F, 0xFFFF};
		__declspec(align(16)) const static unsigned short ncoeff_EdgeFlagByte_one[] = {0xFF1F, 0xFFFF, 0x0000, 0x0000};
		__declspec(align(16)) const static unsigned short ncoeff_mbField[] = {0x0020, 0x0000, 0x0020, 0x0000, 0x0020, 0x0000, 0x0020, 0x0000};
		__declspec(align(16)) const static unsigned short ncoeff_mbField_one[] = {0x0020, 0x0000, 0x0000, 0x0000};
		__declspec(align(16)) const static unsigned short ncoeff_2bits[] = {0x0003, 0x0000, 0x0003, 0x0000, 0x0003, 0x0000, 0x0003, 0x0000};
		__declspec(align(16)) const static unsigned short ncoeff_2bits_one[] = {0x0003, 0x0000, 0x0000, 0x0000};

		mm4 = _mm_setzero_si64();
		mm_1_const = *((__m64*)&ncoeff_1_const[0]);
		mm0 = *((__m64*)&StrengthH[0][0]);
		mm1 = *((__m64*)&StrengthH[1][0]);
		mm2 = *((__m64*)&StrengthH[2][0]);
		mm3 = *((__m64*)&StrengthH[3][0]);
		mm5 = *((__m64*)&StrengthH[4][0]);

		mm0 = _mm_unpacklo_pi8(mm0, mm4);
		mm1 = _mm_unpacklo_pi8(mm1, mm4);
		mm2 = _mm_unpacklo_pi8(mm2, mm4);
		mm3 = _mm_unpacklo_pi8(mm3, mm4);
		mm5 = _mm_unpacklo_pi8(mm5, mm4);

		mm4 = _mm_and_si64(mm0, mm_1_const);
		mm0 = _mm_srli_pi16 (mm0, 1);
		mm0 = _mm_or_si64(mm0, mm4);

		mm4 = _mm_and_si64(mm1, mm_1_const);
		mm1 = _mm_srli_pi16 (mm1, 1);
		mm1 = _mm_or_si64(mm1, mm4);

		mm4 = _mm_and_si64(mm2, mm_1_const);
		mm2 = _mm_srli_pi16 (mm2, 1);
		mm2 = _mm_or_si64(mm2, mm4);

		mm4 = _mm_and_si64(mm3, mm_1_const);
		mm3 = _mm_srli_pi16 (mm3, 1);
		mm3 = _mm_or_si64(mm3, mm4);

		mm4 = _mm_and_si64(mm5, mm_1_const);
		mm5 = _mm_srli_pi16 (mm5, 1);
		mm5 = _mm_or_si64(mm5, mm4);

		mm4 = _mm_packs_pi16(mm0, mm1);
		mm0 = _mm_packs_pi16(mm2, mm3);
		mm5 = _mm_packs_pi16(mm5, mm3);

		//get "edgeFlag.b0.lineEdgeFlag" for one data.
		mm1 = _mm_srli_si64(mm5, 16);
		mm1 = _mm_or_si64(mm1, mm5);
		mm2 = _mm_srli_si64(mm1, 8);
		mm2 = _mm_or_si64(mm1, mm2);
		mm1 = *((__m64*)&ncoeff_2bits_one[0]);
		mm1 = _mm_and_si64(mm1, mm2);
		mm1 = _mm_slli_pi32(mm1, 6); //shift 6-bits 

		//get original flag without "lineEdgeFlag" and "mbField"
		mm2 = *((__m64*)&ncoeff_EdgeFlagByte_one[0]);
		mm2 = _mm_and_si64(mm2, mm5);

		//combine "lineEdgeFlag" to original flag.
		mm2 = _mm_or_si64(mm2, mm1);

		//set mbField value.
		mm1 = *((__m64*)&ncoeff_mbField_one[0]);
		mm1 = _mm_srli_pi32(mm1, ((!fieldQ)<<3));

		//combine "mbField" to original flag.
		mm5 = _mm_or_si64(mm1, mm2);

		//get 4 array into xmm.
		xmm0 = _mm_movpi64_epi64(mm4);
		xmm1 = _mm_movpi64_epi64(mm0);
		xmm1 = _mm_slli_si128(xmm1, 8);
		xmm0 = _mm_or_si128(xmm0, xmm1); 

		//get "edgeFlag.b0.lineEdgeFlag".
		xmm1 = _mm_srli_si128(xmm0, 2);
		xmm1 = _mm_or_si128(xmm1, xmm0);
		xmm2 = _mm_srli_si128(xmm1, 1);
		xmm2 = _mm_or_si128(xmm1, xmm2);
		xmm1 = *((__m128i*)&ncoeff_2bits[0]);
		xmm1 = _mm_and_si128(xmm1, xmm2);
		xmm1 = _mm_slli_epi32(xmm1, 6); //shift 6-bits 

		//get original flag without "lineEdgeFlag" and "mbField"
		xmm2 = *((__m128i*)&ncoeff_EdgeFlagByte[0]);
		xmm2 = _mm_and_si128(xmm2, xmm0);

		//combine "lineEdgeFlag" to original flag.
		xmm2 = _mm_or_si128(xmm2, xmm1);

		//set mbField value.
		xmm1 = *((__m128i*)&ncoeff_mbField[0]);
		xmm1 = _mm_srli_epi32(xmm1, ((!fieldQ)<<3));

		//combine "mbField" to original flag.
		xmm2 = _mm_or_si128(xmm2, xmm1);

		//get lower 64-bits.
		mm0 = _mm_movepi64_pi64(xmm2);
		mm1 = _mm_srli_si64(mm0, 32);

		//get higher 64-bits.
		xmm2 = _mm_srli_si128(xmm2, 8);
		mm2 = _mm_movepi64_pi64(xmm2);
		mm3 = _mm_srli_si64(mm2, 32);

		*((__m64*)&(pLuma->eFlagTC0[0].edgeFlag.eFlag0)) = mm5;
		*((__m64*)&(pLuma->eFlagTC0[1].edgeFlag.eFlag0)) = mm0;
		*((__m64*)&(pLuma->eFlagTC0[2].edgeFlag.eFlag0)) = mm1;
		*((__m64*)&(pLuma->eFlagTC0[3].edgeFlag.eFlag0)) = mm2;
		*((__m64*)&(pLuma->eFlagTC0[4].edgeFlag.eFlag0)) = mm3;
		*((__m64*)&(pChroma->edgeFlag[0].eFlag0)) = mm5;
		*((__m64*)&(pChroma->edgeFlag[1].eFlag0)) = mm0;
		*((__m64*)&(pChroma->edgeFlag[2].eFlag0)) = mm2;
	}

	//Horizontal external deblocking
	// get internal index first
	IndexA[0] = __fast_iclip0_X(51, QpQ + LEAoffset);	
	IndexB[0] = __fast_iclip0_X(51, QpQ + LEBoffset);
	pLuma->AlphaInner = ALPHA_TABLE[IndexA[0]];
	pLuma->BetaInner	= BETA_TABLE[IndexB[0]];

	QP = QP_SCALE_CR[CQPOF(QpQ,0)];			// we do not handle UV offset, not in High profile
	IndexA[1] = __fast_iclip0_X(51, QP + LEAoffset);	
	IndexB[1] = __fast_iclip0_X(51, QP + LEBoffset);
	pChroma->AlphaInner_Cb = ALPHA_TABLE[IndexA[1]];
	pChroma->BetaInner_Cb	= BETA_TABLE[IndexB[1]];
	pChroma->AlphaInner_Cr = ALPHA_TABLE[IndexA[1]];
	pChroma->BetaInner_Cr	= BETA_TABLE[IndexB[1]];
	//	if( StrengthSumH[0])
	if(filterTopMbEdgeFlag)
	{
		QP = (QpQ+MbPH->qp+1)>>1;
		IndexA4[0] = __fast_iclip0_X(51, QP + LEAoffset);	
		IndexB4[0] = __fast_iclip0_X(51, QP + LEBoffset);
		pLuma->AlphaEdge = ALPHA_TABLE[IndexA4[0]];
		pLuma->BetaEdge	= BETA_TABLE[IndexB4[0]];

		QP = (QP_SCALE_CR[CQPOF(QpQ,0)] + QP_SCALE_CR[CQPOF(MbPH->qp,0)] + 1) >> 1;
		IndexA4[1] = __fast_iclip0_X(51, QP + LEAoffset);	
		IndexB4[1] = __fast_iclip0_X(51, QP + LEBoffset);
		pChroma->AlphaEdge_Cb = ALPHA_TABLE[IndexA4[1]];
		pChroma->BetaEdge_Cb	= BETA_TABLE[IndexB4[1]];
		pChroma->AlphaEdge_Cr = ALPHA_TABLE[IndexA4[1]];
		pChroma->BetaEdge_Cr	= BETA_TABLE[IndexB4[1]];

		//EdgeLoop_luma_h(SrcYQ, StrengthH[0], IndexA4, IndexB4, inc_y<<bExtraEdge);
		pLuma->eFlagTC0[1].tC0[0] = CLIP_TAB[IndexA4[0]][StrengthH[0][0]];
		pLuma->eFlagTC0[1].tC0[1] = CLIP_TAB[IndexA4[0]][StrengthH[0][1]];
		pLuma->eFlagTC0[1].tC0[2] = CLIP_TAB[IndexA4[0]][StrengthH[0][2]];
		pLuma->eFlagTC0[1].tC0[3] = CLIP_TAB[IndexA4[0]][StrengthH[0][3]];

		//if( SrcUQ )	// check imgU for both UV
		//EdgeLoop_chromaUV_h(SrcUQ, SrcVQ, StrengthH[0], IndexA4+1, IndexB4+1, inc_ch<<bExtraEdge) ;
		pChroma->tC0_Cb[1][0] = CLIP_TAB[IndexA4[1]][StrengthH[0][0]];
		pChroma->tC0_Cb[1][1] = CLIP_TAB[IndexA4[1]][StrengthH[0][1]];
		pChroma->tC0_Cb[1][2] = CLIP_TAB[IndexA4[1]][StrengthH[0][2]];
		pChroma->tC0_Cb[1][3] = CLIP_TAB[IndexA4[1]][StrengthH[0][3]];
		pChroma->tC0_Cr[1][0] = CLIP_TAB[IndexA4[1]][StrengthH[0][0]];
		pChroma->tC0_Cr[1][1] = CLIP_TAB[IndexA4[1]][StrengthH[0][1]];
		pChroma->tC0_Cr[1][2] = CLIP_TAB[IndexA4[1]][StrengthH[0][2]];
		pChroma->tC0_Cr[1][3] = CLIP_TAB[IndexA4[1]][StrengthH[0][3]];
	}

	// Extra horizontal edge between a frame macroblock pair and a field above it
	//if( StrengthSumH[4] )
	if(bExtraEdge)
	{
		MbPH++;
		QP = (QpQ+MbPH->qp+1)>>1;
		IndexA4[0] = __fast_iclip0_X(51, QP + LEAoffset);	
		IndexB4[0] = __fast_iclip0_X(51, QP + LEBoffset);
		pLuma->AlphaEdge2 = ALPHA_TABLE[IndexA4[0]];
		pLuma->BetaEdge2	= BETA_TABLE[IndexB4[0]];

		QP = (QP_SCALE_CR[CQPOF(QpQ,0)] + QP_SCALE_CR[CQPOF(MbPH->qp,0)] + 1) >> 1;
		IndexA4[1] = __fast_iclip0_X(51, QP + LEAoffset);	
		IndexB4[1] = __fast_iclip0_X(51, QP + LEBoffset);
		pChroma->AlphaEdge2_Cb = ALPHA_TABLE[IndexA4[1]];
		pChroma->BetaEdge2_Cb	= BETA_TABLE[IndexB4[1]];
		pChroma->AlphaEdge2_Cr = ALPHA_TABLE[IndexA4[1]];
		pChroma->BetaEdge2_Cr	= BETA_TABLE[IndexB4[1]];

		//EdgeLoop_luma_h(SrcYQ+inc_y, StrengthH[4], IndexA4, IndexB4, inc_y<<bExtraEdge) ;
		pLuma->eFlagTC0[0].tC0[0] = CLIP_TAB[IndexA4[0]][StrengthH[4][0]];
		pLuma->eFlagTC0[0].tC0[1] = CLIP_TAB[IndexA4[0]][StrengthH[4][1]];
		pLuma->eFlagTC0[0].tC0[2] = CLIP_TAB[IndexA4[0]][StrengthH[4][2]];
		pLuma->eFlagTC0[0].tC0[3] = CLIP_TAB[IndexA4[0]][StrengthH[4][3]];
		pLuma->eFlagTC0[0].edgeFlag.b0.twoTopEdges = 1;
		//if( SrcUQ )	// check imgU for both UV
		//EdgeLoop_chromaUV_h(SrcUQ+inc_ch, SrcVQ+inc_ch, StrengthH[4], IndexA4+1, IndexB4+1, inc_ch<<bExtraEdge) ;
		pChroma->tC0_Cb[0][0] = CLIP_TAB[IndexA4[1]][StrengthH[4][0]];
		pChroma->tC0_Cb[0][1] = CLIP_TAB[IndexA4[1]][StrengthH[4][1]];
		pChroma->tC0_Cb[0][2] = CLIP_TAB[IndexA4[1]][StrengthH[4][2]];
		pChroma->tC0_Cb[0][3] = CLIP_TAB[IndexA4[1]][StrengthH[4][3]];
		pChroma->tC0_Cr[0][0] = CLIP_TAB[IndexA4[1]][StrengthH[4][0]];
		pChroma->tC0_Cr[0][1] = CLIP_TAB[IndexA4[1]][StrengthH[4][1]];
		pChroma->tC0_Cr[0][2] = CLIP_TAB[IndexA4[1]][StrengthH[4][2]];
		pChroma->tC0_Cr[0][3] = CLIP_TAB[IndexA4[1]][StrengthH[4][3]];
		pChroma->edgeFlag[0].b0.twoTopEdges = 1;
	}

	//Horizontal internal deblocking
	//	SrcYQ += inc_y4;
	//	if( !MbQ->luma_transform_size_8x8_flag && StrengthSumH[1] )
	//EdgeLoop_luma_h(SrcYQ, StrengthH[1], IndexA, IndexB, inc_y);
	pLuma->eFlagTC0[2].tC0[0] = CLIP_TAB[IndexA[0]][StrengthH[1][0]];
	pLuma->eFlagTC0[2].tC0[1] = CLIP_TAB[IndexA[0]][StrengthH[1][1]];
	pLuma->eFlagTC0[2].tC0[2] = CLIP_TAB[IndexA[0]][StrengthH[1][2]];
	pLuma->eFlagTC0[2].tC0[3] = CLIP_TAB[IndexA[0]][StrengthH[1][3]];
	//	SrcYQ += inc_y4;
	//	if( StrengthSumH[2])
	//	{
	//EdgeLoop_luma_h(SrcYQ, StrengthH[2], IndexA, IndexB, inc_y);
	pLuma->eFlagTC0[3].tC0[0] = CLIP_TAB[IndexA[0]][StrengthH[2][0]];
	pLuma->eFlagTC0[3].tC0[1] = CLIP_TAB[IndexA[0]][StrengthH[2][1]];
	pLuma->eFlagTC0[3].tC0[2] = CLIP_TAB[IndexA[0]][StrengthH[2][2]];
	pLuma->eFlagTC0[3].tC0[3] = CLIP_TAB[IndexA[0]][StrengthH[2][3]];
	//if( SrcUQ )	// check imgU for both UV
	//EdgeLoop_chromaUV_h(SrcUQ+(inc_ch<<2), SrcVQ+(inc_ch<<2), StrengthH[2], IndexA+1, IndexB+1, inc_ch) ;
	pChroma->tC0_Cb[2][0] = CLIP_TAB[IndexA[1]][StrengthH[2][0]];
	pChroma->tC0_Cb[2][1] = CLIP_TAB[IndexA[1]][StrengthH[2][1]];
	pChroma->tC0_Cb[2][2] = CLIP_TAB[IndexA[1]][StrengthH[2][2]];
	pChroma->tC0_Cb[2][3] = CLIP_TAB[IndexA[1]][StrengthH[2][3]];
	pChroma->tC0_Cr[2][0] = CLIP_TAB[IndexA[1]][StrengthH[2][0]];
	pChroma->tC0_Cr[2][1] = CLIP_TAB[IndexA[1]][StrengthH[2][1]];
	pChroma->tC0_Cr[2][2] = CLIP_TAB[IndexA[1]][StrengthH[2][2]];
	pChroma->tC0_Cr[2][3] = CLIP_TAB[IndexA[1]][StrengthH[2][3]];
	//	}
	//	SrcYQ += inc_y4;
	//	if( !MbQ->luma_transform_size_8x8_flag && StrengthSumH[3] )
	//EdgeLoop_luma_h(SrcYQ, StrengthH[3], IndexA, IndexB, inc_y);
	pLuma->eFlagTC0[4].tC0[0] = CLIP_TAB[IndexA[0]][StrengthH[3][0]];
	pLuma->eFlagTC0[4].tC0[1] = CLIP_TAB[IndexA[0]][StrengthH[3][1]];
	pLuma->eFlagTC0[4].tC0[2] = CLIP_TAB[IndexA[0]][StrengthH[3][2]];
	pLuma->eFlagTC0[4].tC0[3] = CLIP_TAB[IndexA[0]][StrengthH[3][3]];
	//end horizontal deblocking
	//Dump(m_lpDEBLK_Luma, sizeof(EdgeDataLumaMBAFF), "DEBLK_Luma_mbaff.dat");
	//Dump(m_lpDEBLK_Chroma, sizeof(EdgeDataChromaMBAFF), "DEBLK_Chroma_mbaff.dat");
	IMGPAR m_lpDEBLK_Luma += sizeof(EdgeDataLumaMBAFF);
	IMGPAR m_lpDEBLK_Chroma += sizeof(EdgeDataChromaMBAFF);
}
//#endif

void CH264DXVA1_NV::build_macroblock_buffer_Inter_Int PARGS5(BYTE pred_dir,BYTE index,BYTE list_offset,struct nvh264vp1MacroblockControlInterLuma* MB_Partition,struct nvh264vp1MacroblockControlInterChroma* MB_Partition_C)
{
	int ref_idx, ref_idx_bw, ref_picid, ref_picid_bw;
	int current_mb_nr = IMGPAR current_mb_nr_d;
	int cbp_total = currMB_d->cbp;
	int temp=0;
	int wt_list_offset = (active_pps.weighted_bipred_idc==2)?list_offset:0;
	short mv[2],mv_bw[2];
	short weight_info[2],weight_info_bw[2];
	short weight_info_C[4],weight_info_C_bw[4];
	weight_info[1]	 = (short) 0xFF00;
	weight_info_C[3] = (short) 0xFFFF;
	weight_info[0]	= 0;
	weight_info_C[0] = weight_info_C[1] = weight_info_C[2] = 0;

	weight_info_bw[0]	= weight_info_bw[1]= 0;
	weight_info_C_bw[0] = weight_info_C_bw[1] = weight_info_C_bw[2] = weight_info_C_bw[3] = 0;	

	if(pred_dir!=2)
	{
		MB_Partition->predFlags	= NVH264VP1_PRED_FLAG_INTER;
		ref_idx = currMB_s_d->pred_info.ref_idx[pred_dir][l_16_4[index]];
		ref_picid = currMB_s_d->pred_info.ref_pic_id[pred_dir][l_16_4[index]];
		*(int*)&mv[0] = *(int*)&currMB_s_d->pred_info.mv[pred_dir][index].x;

		mv[0] = ((mv[0]+2) & 0xFFFC);
		mv[1] = ((mv[1]+2) & 0xFFFC);

		ref_idx_bw = 0;
		ref_picid_bw = 0;
		*(int*)&mv_bw[0] = 0;
		if (IMGPAR apply_weights)
		{
			weight_info[0] = (*(IMGPAR wp_weight+(pred_dir*MAX_REFERENCE_PICTURES+ref_idx)*3+0));
			weight_info[1] = (IMGPAR luma_log2_weight_denom<<8);
			weight_info[1] |= ((*(IMGPAR wp_offset+(pred_dir*MAX_REFERENCE_PICTURES+ref_idx)*3+0)) & 0xFF);
		}
	}
	else //(pred_dir==2)
	{
		MB_Partition->predFlags	= NVH264VP1_PRED_FLAG_BI_PRED|NVH264VP1_PRED_FLAG_INTER;
		ref_idx = currMB_s_d->pred_info.ref_idx[LIST_0][l_16_4[index]];
		ref_picid = currMB_s_d->pred_info.ref_pic_id[LIST_0][l_16_4[index]];
		*(int*)&mv[0] = *(int*)&currMB_s_d->pred_info.mv[LIST_0][index].x;
		ref_idx_bw = currMB_s_d->pred_info.ref_idx[LIST_1][l_16_4[index]];
		ref_picid_bw = currMB_s_d->pred_info.ref_pic_id[LIST_1][l_16_4[index]];
		*(int*)&mv_bw[0] = *(int*)&currMB_s_d->pred_info.mv[LIST_1][index].x;

		mv[0] = ((mv[0]+2) & 0xFFFC);
		mv[1] = ((mv[1]+2) & 0xFFFC);
		mv_bw[0] = ((mv_bw[0]+2) & 0xFFFC);
		mv_bw[1] = ((mv_bw[1]+2) & 0xFFFC);

		if (IMGPAR apply_weights)
		{
			weight_info[0] = (*(IMGPAR wbp_weight+(((0+wt_list_offset)*MAX_REFERENCE_PICTURES+ref_idx)*MAX_REFERENCE_PICTURES+ref_idx_bw)*3+0));
			weight_info[1] = (IMGPAR luma_log2_weight_denom<<8);
			weight_info[1] |= ((*(IMGPAR wp_offset+((wt_list_offset+0)*MAX_REFERENCE_PICTURES+ref_idx)*3+0)) & 0xFF);
			weight_info_bw[0] = (*(IMGPAR wbp_weight+(((1+wt_list_offset)*MAX_REFERENCE_PICTURES+ref_idx)*MAX_REFERENCE_PICTURES+ref_idx_bw)*3+0));
			weight_info_bw[1] =(*(IMGPAR wp_offset+((wt_list_offset+1)*MAX_REFERENCE_PICTURES+ref_idx_bw)*3+0));
		}
	}

	if(IMGPAR MbaffFrameFlag)
	{
		if(current_mb_nr&1)
		{
			if(currMB_d->mb_field)
			{
				MB_Partition->predFlags |= 0x30;
				IMGPAR m_lPrev_Field_pair = 1;	
			}
			else
			{
				MB_Partition->predFlags |= 0x20;
				IMGPAR m_lPrev_Field_pair = 0;
			}		
		}
		else
		{
			if(currMB_d->mb_field)
				MB_Partition->predFlags |= 0x10;
			else
				MB_Partition->predFlags |= 0x00;
		}
	}

	MB_Partition->uX				 = (IMGPAR mb_x_d<<4);
	if(IMGPAR MbaffFrameFlag && currMB_d->mb_field)
		MB_Partition->uY		= ((IMGPAR mb_y_d>>1)<<4);
	else
		MB_Partition->uY				 = (IMGPAR mb_y_d<<4);
	*(short*)&MB_Partition->refIndex	 = *(short*)&m_pSurfaceInfo[ref_picid].SInfo[0];
	if(MB_Partition->predFlags & 0x04)
		*(short*)&MB_Partition->refIndex2	= *(short*)&m_pSurfaceInfo[ref_picid_bw].SInfo[0];
	else
		*(short*)&MB_Partition->refIndex2	= 0;
	*(int*)&MB_Partition->mvx= *(int*)&mv[0];
	*(int*)&MB_Partition->wL = *(int*)&weight_info[0];
	*(int*)&MB_Partition->mvx2 = *(int*)&mv_bw[0];
	*(int*)&MB_Partition->wL2 = *(int*)&weight_info_bw[0];

#ifdef __SUPPORT_YUV400__
	if (dec_picture->chroma_format_idc != YUV400)
	{
#endif
		if(pred_dir!=2)
		{
			mv[1] += IMGPAR cr_vector_adjustment[list_offset+pred_dir][ref_idx];

			mv[1] = ((mv[1]+2) & 0xFFFC);

			if(IMGPAR apply_weights)
			{
				weight_info_C[0] = (*(IMGPAR wp_weight+(pred_dir*MAX_REFERENCE_PICTURES+ref_idx)*3+0+1));
				weight_info_C[1] = (*(IMGPAR wp_weight+(pred_dir*MAX_REFERENCE_PICTURES+ref_idx)*3+1+1));
				weight_info_C[2] = ((*(IMGPAR wp_offset+(pred_dir*MAX_REFERENCE_PICTURES+ref_idx)*3+1+1))<<8);
				weight_info_C[2] |= ((*(IMGPAR wp_offset+(pred_dir*MAX_REFERENCE_PICTURES+ref_idx)*3+0+1)) & 0xFF);
				weight_info_C[3] = IMGPAR chroma_log2_weight_denom;
			}
		}
		else //(pred_dir==2)
		{
			mv[1] += IMGPAR cr_vector_adjustment[list_offset+LIST_0][ref_idx];
			mv_bw[1] += IMGPAR cr_vector_adjustment[list_offset+LIST_1][ref_idx_bw];

			mv[1] = ((mv[1]+2) & 0xFFFC);
			mv_bw[1] = ((mv_bw[1]+2) & 0xFFFC);

			if(IMGPAR apply_weights)
			{
				weight_info_C[0] = (*(IMGPAR wbp_weight+(((0+wt_list_offset)*MAX_REFERENCE_PICTURES+ref_idx)*MAX_REFERENCE_PICTURES+ref_idx_bw)*3+0+1));
				weight_info_C[1] = (*(IMGPAR wbp_weight+(((0+wt_list_offset)*MAX_REFERENCE_PICTURES+ref_idx)*MAX_REFERENCE_PICTURES+ref_idx_bw)*3+1+1));
				weight_info_C[2] = ((*(IMGPAR wp_offset+((wt_list_offset + 0)*MAX_REFERENCE_PICTURES+ref_idx)*3+1+1))<<8);
				weight_info_C[2] |= ((*(IMGPAR wp_offset+((wt_list_offset + 0)*MAX_REFERENCE_PICTURES+ref_idx)*3+0+1)) & 0xFF);
				weight_info_C[3] = IMGPAR chroma_log2_weight_denom;
				weight_info_C_bw[0] = (*(IMGPAR wbp_weight+(((1+wt_list_offset)*MAX_REFERENCE_PICTURES+ref_idx)*MAX_REFERENCE_PICTURES+ref_idx_bw)*3+0+1));
				weight_info_C_bw[1] = (*(IMGPAR wbp_weight+(((1+wt_list_offset)*MAX_REFERENCE_PICTURES+ref_idx)*MAX_REFERENCE_PICTURES+ref_idx_bw)*3+1+1));
				weight_info_C_bw[2] = (*(IMGPAR wp_offset+((wt_list_offset + 1)*MAX_REFERENCE_PICTURES+ref_idx_bw)*3+0+1));
				weight_info_C_bw[3] = (*(IMGPAR wp_offset+((wt_list_offset + 1)*MAX_REFERENCE_PICTURES+ref_idx_bw)*3+1+1));
			}
		}
		MB_Partition_C->uX				 = (IMGPAR mb_x_d<<4);
		if(IMGPAR MbaffFrameFlag && currMB_d->mb_field)
			MB_Partition_C->uY		= ((IMGPAR mb_y_d>>1)<<4);
		else
			MB_Partition_C->uY				 = (IMGPAR mb_y_d<<4);
		*(short*)&MB_Partition_C->refIndex	 = *(short*)&m_pSurfaceInfo[ref_picid].SInfo[0];
		if(MB_Partition->predFlags & 0x04)
			*(short*)&MB_Partition_C->refIndex2	= *(short*)&m_pSurfaceInfo[ref_picid_bw].SInfo[0];
		else
			*(short*)&MB_Partition_C->refIndex2	= 0;
		*(int*)&MB_Partition_C->mvx = *(int*)&mv[0];
		*(LONGLONG*)&MB_Partition_C->wCb				= *(LONGLONG*)&weight_info_C[0];
		*(int*)&MB_Partition_C->mvx2 = *(int*)&mv_bw[0];
		*(LONGLONG*)&MB_Partition_C->wCb2			 = *(LONGLONG*)&weight_info_C_bw[0];


#ifdef __SUPPORT_YUV400__
	}
#endif
}

void CH264DXVA1_NV::build_macroblock_buffer_Inter_Ori PARGS5(BYTE pred_dir,BYTE index,BYTE list_offset,struct nvh264vp1MacroblockControlInterLuma* MB_Partition,struct nvh264vp1MacroblockControlInterChroma* MB_Partition_C)
{
	int ref_idx, ref_idx_bw, ref_picid, ref_picid_bw;
	int current_mb_nr = IMGPAR current_mb_nr_d;
	int cbp_total = currMB_d->cbp;
	int temp=0;
	int wt_list_offset = (active_pps.weighted_bipred_idc==2)?list_offset:0;
	short mv[2],mv_bw[2];
	short weight_info[2],weight_info_bw[2];
	short weight_info_C[4],weight_info_C_bw[4];
	weight_info[1]	 = (short) 0xFF00;
	weight_info_C[3] = (short) 0xFFFF;
	weight_info[0]	= 0;
	weight_info_C[0] = weight_info_C[1] = weight_info_C[2] = 0;

	weight_info_bw[0]	= weight_info_bw[1]= 0;
	weight_info_C_bw[0] = weight_info_C_bw[1] = weight_info_C_bw[2] = weight_info_C_bw[3] = 0;	

	if(pred_dir!=2)
	{
		MB_Partition->predFlags	= NVH264VP1_PRED_FLAG_INTER;
		ref_idx = currMB_s_d->pred_info.ref_idx[pred_dir][l_16_4[index]];
		ref_picid = currMB_s_d->pred_info.ref_pic_id[pred_dir][l_16_4[index]];
		*(int*)&mv[0] = *(int*)&currMB_s_d->pred_info.mv[pred_dir][index].x;

		ref_idx_bw = 0;
		ref_picid_bw = 0;
		*(int*)&mv_bw[0] = 0;
		if (IMGPAR apply_weights)
		{
			weight_info[0] = (*(IMGPAR wp_weight+(pred_dir*MAX_REFERENCE_PICTURES+ref_idx)*3+0));
			weight_info[1] = (IMGPAR luma_log2_weight_denom<<8);
			weight_info[1] |= ((*(IMGPAR wp_offset+(pred_dir*MAX_REFERENCE_PICTURES+ref_idx)*3+0)) & 0xFF);
		}
	}
	else //(pred_dir==2)
	{
		MB_Partition->predFlags	= NVH264VP1_PRED_FLAG_BI_PRED|NVH264VP1_PRED_FLAG_INTER;
		ref_idx = currMB_s_d->pred_info.ref_idx[LIST_0][l_16_4[index]];
		ref_picid = currMB_s_d->pred_info.ref_pic_id[LIST_0][l_16_4[index]];
		*(int*)&mv[0] = *(int*)&currMB_s_d->pred_info.mv[LIST_0][index].x;
		ref_idx_bw = currMB_s_d->pred_info.ref_idx[LIST_1][l_16_4[index]];
		ref_picid_bw = currMB_s_d->pred_info.ref_pic_id[LIST_1][l_16_4[index]];
		*(int*)&mv_bw[0] = *(int*)&currMB_s_d->pred_info.mv[LIST_1][index].x;

		if (IMGPAR apply_weights)
		{
			weight_info[0] = (*(IMGPAR wbp_weight+(((0+wt_list_offset)*MAX_REFERENCE_PICTURES+ref_idx)*MAX_REFERENCE_PICTURES+ref_idx_bw)*3+0));
			weight_info[1] = (IMGPAR luma_log2_weight_denom<<8);
			weight_info[1] |= ((*(IMGPAR wp_offset+((wt_list_offset+0)*MAX_REFERENCE_PICTURES+ref_idx)*3+0)) & 0xFF);
			weight_info_bw[0] = (*(IMGPAR wbp_weight+(((1+wt_list_offset)*MAX_REFERENCE_PICTURES+ref_idx)*MAX_REFERENCE_PICTURES+ref_idx_bw)*3+0));
			weight_info_bw[1] =(*(IMGPAR wp_offset+((wt_list_offset+1)*MAX_REFERENCE_PICTURES+ref_idx_bw)*3+0));
		}
	}

	if(IMGPAR MbaffFrameFlag)
	{
		if(current_mb_nr&1)
		{
			if(currMB_d->mb_field)
			{
				MB_Partition->predFlags |= 0x30;
				IMGPAR m_lPrev_Field_pair = 1;	
			}
			else
			{
				MB_Partition->predFlags |= 0x20;
				IMGPAR m_lPrev_Field_pair = 0;
			}		
		}
		else
		{
			if(currMB_d->mb_field)
				MB_Partition->predFlags |= 0x10;
			else
				MB_Partition->predFlags |= 0x00;
		}
	}

	MB_Partition->uX				 = (IMGPAR mb_x_d<<4);
	if(IMGPAR MbaffFrameFlag && currMB_d->mb_field)
		MB_Partition->uY		= ((IMGPAR mb_y_d>>1)<<4);
	else
		MB_Partition->uY				 = (IMGPAR mb_y_d<<4);
	*(short*)&MB_Partition->refIndex	 = *(short*)&m_pSurfaceInfo[ref_picid].SInfo[0];
	if(MB_Partition->predFlags & 0x04)
		*(short*)&MB_Partition->refIndex2	= *(short*)&m_pSurfaceInfo[ref_picid_bw].SInfo[0];
	else
		*(short*)&MB_Partition->refIndex2	= 0;
	*(int*)&MB_Partition->mvx= *(int*)&mv[0];
	*(int*)&MB_Partition->wL = *(int*)&weight_info[0];
	*(int*)&MB_Partition->mvx2 = *(int*)&mv_bw[0];
	*(int*)&MB_Partition->wL2 = *(int*)&weight_info_bw[0];

#ifdef __SUPPORT_YUV400__
	if (dec_picture->chroma_format_idc != YUV400)
	{
#endif
		if(pred_dir!=2)
		{
			mv[1] += IMGPAR cr_vector_adjustment[list_offset+pred_dir][ref_idx];

			if(IMGPAR apply_weights)
			{
				weight_info_C[0] = (*(IMGPAR wp_weight+(pred_dir*MAX_REFERENCE_PICTURES+ref_idx)*3+0+1));
				weight_info_C[1] = (*(IMGPAR wp_weight+(pred_dir*MAX_REFERENCE_PICTURES+ref_idx)*3+1+1));
				weight_info_C[2] = ((*(IMGPAR wp_offset+(pred_dir*MAX_REFERENCE_PICTURES+ref_idx)*3+1+1))<<8);
				weight_info_C[2] |= ((*(IMGPAR wp_offset+(pred_dir*MAX_REFERENCE_PICTURES+ref_idx)*3+0+1)) & 0xFF);
				weight_info_C[3] = IMGPAR chroma_log2_weight_denom;
			}
		}
		else //(pred_dir==2)
		{
			mv[1] += IMGPAR cr_vector_adjustment[list_offset+LIST_0][ref_idx];
			mv_bw[1] += IMGPAR cr_vector_adjustment[list_offset+LIST_1][ref_idx_bw];

			if(IMGPAR apply_weights)
			{
				weight_info_C[0] = (*(IMGPAR wbp_weight+(((0+wt_list_offset)*MAX_REFERENCE_PICTURES+ref_idx)*MAX_REFERENCE_PICTURES+ref_idx_bw)*3+0+1));
				weight_info_C[1] = (*(IMGPAR wbp_weight+(((0+wt_list_offset)*MAX_REFERENCE_PICTURES+ref_idx)*MAX_REFERENCE_PICTURES+ref_idx_bw)*3+1+1));
				weight_info_C[2] = ((*(IMGPAR wp_offset+((wt_list_offset + 0)*MAX_REFERENCE_PICTURES+ref_idx)*3+1+1))<<8);
				weight_info_C[2] |= ((*(IMGPAR wp_offset+((wt_list_offset + 0)*MAX_REFERENCE_PICTURES+ref_idx)*3+0+1)) & 0xFF);
				weight_info_C[3] = IMGPAR chroma_log2_weight_denom;
				weight_info_C_bw[0] = (*(IMGPAR wbp_weight+(((1+wt_list_offset)*MAX_REFERENCE_PICTURES+ref_idx)*MAX_REFERENCE_PICTURES+ref_idx_bw)*3+0+1));
				weight_info_C_bw[1] = (*(IMGPAR wbp_weight+(((1+wt_list_offset)*MAX_REFERENCE_PICTURES+ref_idx)*MAX_REFERENCE_PICTURES+ref_idx_bw)*3+1+1));
				weight_info_C_bw[2] = (*(IMGPAR wp_offset+((wt_list_offset + 1)*MAX_REFERENCE_PICTURES+ref_idx_bw)*3+0+1));
				weight_info_C_bw[3] = (*(IMGPAR wp_offset+((wt_list_offset + 1)*MAX_REFERENCE_PICTURES+ref_idx_bw)*3+1+1));
			}
		}
		MB_Partition_C->uX				 = (IMGPAR mb_x_d<<4);
		if(IMGPAR MbaffFrameFlag && currMB_d->mb_field)
			MB_Partition_C->uY		= ((IMGPAR mb_y_d>>1)<<4);
		else
			MB_Partition_C->uY				 = (IMGPAR mb_y_d<<4);
		*(short*)&MB_Partition_C->refIndex	 = *(short*)&m_pSurfaceInfo[ref_picid].SInfo[0];
		if(MB_Partition->predFlags & 0x04)
			*(short*)&MB_Partition_C->refIndex2	= *(short*)&m_pSurfaceInfo[ref_picid_bw].SInfo[0];
		else
			*(short*)&MB_Partition_C->refIndex2	= 0;
		*(int*)&MB_Partition_C->mvx = *(int*)&mv[0];
		*(LONGLONG*)&MB_Partition_C->wCb				= *(LONGLONG*)&weight_info_C[0];
		*(int*)&MB_Partition_C->mvx2 = *(int*)&mv_bw[0];
		*(LONGLONG*)&MB_Partition_C->wCb2			 = *(LONGLONG*)&weight_info_C_bw[0];


#ifdef __SUPPORT_YUV400__
	}
#endif
}

void CH264DXVA1_NV::build_macroblock_buffer_Inter_16x16_Luma PARGS1(int list_offset)
{
	struct nvh264vp1MacroblockControlInterLuma* MB_Partition = (nvh264vp1MacroblockControlInterLuma*)IMGPAR m_lpMBLK_Inter_Luma;
	int pred_dir = currMB_d->b8pdir[0];
	int cbp_total = currMB_d->cbp;
	int cbp=0;

	build_macroblock_buffer_Inter_Luma ARGS4(pred_dir,0,list_offset,MB_Partition);
	MB_Partition->partIndex	= (currMB_d->cbp&0x0F); //(currMB_d->cbp&0x0F) | (0x00<<4)
	MB_Partition->partOffset = 0;
	MB_Partition->partSize	 = 0x88;	//(16/2) | ((16/2)<<4)

	IMGPAR m_lpMBLK_Inter_Luma += sizeof(struct nvh264vp1MacroblockControlInterLuma);
	IMGPAR m_iInterMCBufUsage += sizeof(struct nvh264vp1MacroblockControlInterLuma);
}

void CH264DXVA1_NV::build_macroblock_buffer_Inter_16x8_Luma PARGS1(int list_offset)
{
	struct nvh264vp1MacroblockControlInterLuma* MB_Partition = (nvh264vp1MacroblockControlInterLuma*)IMGPAR m_lpMBLK_Inter_Luma;
	int joff, k;
	int pred_dir;
	int cbp_total = currMB_d->cbp;
	int cbp=0;

	for(k=0;k<2;k++,MB_Partition++)
	{
		joff = (k<<3);
		pred_dir = currMB_d->b8pdir[k<<1];
		build_macroblock_buffer_Inter_Luma ARGS4(pred_dir,joff,list_offset,MB_Partition);
		if(k==0)
			MB_Partition->partIndex	= (currMB_d->cbp&0x0F) | (0x10);//(currMB_d->cbp&0x0F) | (0x01<<4)
		else
			MB_Partition->partIndex	= 0;
		MB_Partition->partOffset = (k<<7); // 0x00 | ((k<<3)<<4)
		MB_Partition->partSize	 = 0x48;	//(16/2) | ((8/2)<<4)
	}

	IMGPAR m_lpMBLK_Inter_Luma += (2*sizeof(struct nvh264vp1MacroblockControlInterLuma));
	IMGPAR m_iInterMCBufUsage += (2*sizeof(struct nvh264vp1MacroblockControlInterLuma));
}

void CH264DXVA1_NV::build_macroblock_buffer_Inter_8x16_Luma PARGS1(int list_offset)
{
	struct nvh264vp1MacroblockControlInterLuma* MB_Partition = (nvh264vp1MacroblockControlInterLuma*)IMGPAR m_lpMBLK_Inter_Luma;
	int i4, k;
	int pred_dir;
	int cbp_total = currMB_d->cbp;
	int cbp=0;

	for(k=0;k<2;k++,MB_Partition++)
	{
		i4 = (k<<1);
		pred_dir = currMB_d->b8pdir[k];
		build_macroblock_buffer_Inter_Luma ARGS4(pred_dir,i4,list_offset,MB_Partition);
		if(k==0)
			MB_Partition->partIndex	= (currMB_d->cbp&0x0F) | (0x10);//(currMB_d->cbp&0x0F) | (0x01<<4)
		else
			MB_Partition->partIndex	= 0;
		MB_Partition->partOffset = (k<<3); // (k<<3) | (0x00<<4)
		MB_Partition->partSize	 = 0x84;	//(8/2) | ((16/2)<<4)
	}
	IMGPAR m_lpMBLK_Inter_Luma += (2*sizeof(struct nvh264vp1MacroblockControlInterLuma));
	IMGPAR m_iInterMCBufUsage += (2*sizeof(struct nvh264vp1MacroblockControlInterLuma));
}

void CH264DXVA1_NV::build_macroblock_buffer_Inter_8x8_Luma PARGS1(int list_offset)
{
	struct nvh264vp1MacroblockControlInterLuma* MB_Partition = (nvh264vp1MacroblockControlInterLuma*)IMGPAR m_lpMBLK_Inter_Luma;
	int i,b8,count,idx,cbp;
	int cbp_total = currMB_d->cbp;
	int temp=0;
	int pred_dir,mode;
	int fw_refframe,bw_refframe;
	const static int	table[2] = {0,8},
		table1[2] = {0,4},
		b84_idx[4][2] = { {0,4} , {2,6} , {8,12} , {10,14} },
		b48_idx[4][2] = { {0,1} , {2,3} , {8,9} , {10,11} },
		b44_idx[4][4] = { {0,1,4,5} , {2,3,6,7} , {8,9,12,13} , {10,11,14,15} },
		b8modes_count_tab[] =	{ 0, //-1
		1, 0, 0, 0, 1, 2, 2, 4//,	//0-7
		//0, 0, 0, 0, 0, 0, 0, 0,	//8-15
		//0, 0, 0, 0, 0, 0, 0, 0,	//16-23
		//0, 0, 0, 0, 0, 0, 0, 0,	//24-31
		//0 //32
	},
	b8modes_count_tab2[] =	{ 0, //-1
	4, 0, 0, 0, 1, 2, 2, 4//,	//0-7
	//0, 0, 0, 0, 0, 0, 0, 0,	//8-15
	//0, 0, 0, 0, 0, 0, 0, 0,	//16-23
	//0, 0, 0, 0, 0, 0, 0, 0,	//24-31
	//0 //32
	};
	if(IMGPAR type!=B_SLICE || (IMGPAR type==B_SLICE && active_sps.direct_8x8_inference_flag))
		count = 
		b8modes_count_tab[((int) currMB_d->b8mode[0]) + 1]
	+ b8modes_count_tab[((int) currMB_d->b8mode[1]) + 1]
	+ b8modes_count_tab[((int) currMB_d->b8mode[2]) + 1]
	+ b8modes_count_tab[((int) currMB_d->b8mode[3]) + 1];
	else
		count = 
		b8modes_count_tab2[((int) currMB_d->b8mode[0]) + 1]
	+ b8modes_count_tab2[((int) currMB_d->b8mode[1]) + 1]
	+ b8modes_count_tab2[((int) currMB_d->b8mode[2]) + 1]
	+ b8modes_count_tab2[((int) currMB_d->b8mode[3]) + 1];

	cbp = idx=0;
	for (b8=0; b8<4; b8++)
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

		if(mode==PB_8x8 || mode==0)
		{
			count--;
			build_macroblock_buffer_Inter_Luma ARGS4(pred_dir,b8_idx[b8],list_offset,MB_Partition);
			if(b8==0)
				MB_Partition->partIndex	= (currMB_d->cbp&0x0F) | (count<<4);
			else
				MB_Partition->partIndex	= (count<<4);
			MB_Partition->partOffset = (table[b8&1]) | ((table[b8>>1])<<4); 
			MB_Partition->partSize	 = 0x44;	//(8/2) | ((8/2)<<4)
			idx++; MB_Partition++;
		}

		else if(mode==PB_8x4)
		{
			for(i=0 ; i<2 ; i++)
			{
				count--;
				build_macroblock_buffer_Inter_Luma ARGS4(pred_dir,b84_idx[b8][i],list_offset,MB_Partition);
				if(b8==0 && i==0)
					MB_Partition->partIndex	= (currMB_d->cbp&0x0F) | (count<<4);
				else
					MB_Partition->partIndex	= (count<<4);
				MB_Partition->partOffset = (table[b8&1]) | ((table[b8>>1]+table1[i])<<4); 
				MB_Partition->partSize	 = 0x24;	//(8/2) | ((4/2)<<4)
				idx++; MB_Partition++;
			}
		}
		else if(mode==PB_4x8)
		{
			for(i=0 ; i<2 ; i++)
			{
				count--;
				build_macroblock_buffer_Inter_Luma ARGS4(pred_dir,b48_idx[b8][i],list_offset,MB_Partition);
				if(b8==0 && i==0)
					MB_Partition->partIndex	= (currMB_d->cbp&0x0F) | (count<<4);
				else
					MB_Partition->partIndex	= (count<<4);
				MB_Partition->partOffset = (table[b8&1]+table1[i]) | ((table[b8>>1])<<4); 
				MB_Partition->partSize	 = 0x42;	//(4/2) | ((8/2)<<4)

				idx++; MB_Partition++;
			}
		}
		else
		{
			for(i=0 ; i<4 ; i++)
			{
				count--;
				build_macroblock_buffer_Inter_Luma ARGS4(pred_dir,b44_idx[b8][i],list_offset,MB_Partition);
				if(b8==0 && i==0)
					MB_Partition->partIndex	= (currMB_d->cbp&0x0F) | (count<<4);
				else
					MB_Partition->partIndex	= (count<<4);
				MB_Partition->partOffset = (table[b8&1]+table1[i&1]) | ((table[b8>>1]+table1[i>>1])<<4); 
				MB_Partition->partSize	 = 0x22;	//(4/2) | ((4/2)<<4)

				idx++; MB_Partition++;
			}
		}
	}
	IMGPAR m_lpMBLK_Inter_Luma += (idx*sizeof(struct nvh264vp1MacroblockControlInterLuma));
	IMGPAR m_iInterMCBufUsage += (idx*sizeof(struct nvh264vp1MacroblockControlInterLuma));
}

void CH264DXVA1_NV::build_macroblock_buffer_Inter_Luma PARGS4(BYTE pred_dir,BYTE index,BYTE list_offset,struct nvh264vp1MacroblockControlInterLuma* MB_Partition)
{
	int ref_idx, ref_idx_bw, ref_picid, ref_picid_bw;
	int current_mb_nr = IMGPAR current_mb_nr_d;
	int cbp_total = currMB_d->cbp;
	int temp=0;
	int wt_list_offset = (active_pps.weighted_bipred_idc==2)?list_offset:0;
	short mv[2],mv_bw[2];
	short weight_info[2],weight_info_bw[2];
	weight_info[1]	 = (short) 0xFF00;
	weight_info[0]	= 0;

	weight_info_bw[0]	= weight_info_bw[1]= 0;
	if(pred_dir!=2)
	{
		MB_Partition->predFlags	= NVH264VP1_PRED_FLAG_INTER;
		ref_idx = currMB_s_d->pred_info.ref_idx[pred_dir][l_16_4[index]];
		ref_picid = currMB_s_d->pred_info.ref_pic_id[pred_dir][l_16_4[index]];
		*(int*)&mv[0] = *(int*)&currMB_s_d->pred_info.mv[pred_dir][index].x;
		ref_idx_bw = 0;
		ref_picid_bw = 0;
		*(int*)&mv_bw[0] = 0;
		if (IMGPAR apply_weights)
		{
			weight_info[0] = (*(IMGPAR wp_weight+(pred_dir*MAX_REFERENCE_PICTURES+ref_idx)*3+0));
			weight_info[1] = (IMGPAR luma_log2_weight_denom<<8);
			weight_info[1] |= ((*(IMGPAR wp_offset+(pred_dir*MAX_REFERENCE_PICTURES+ref_idx)*3+0)) & 0xFF);
		}
	}
	else //(pred_dir==2)
	{
		MB_Partition->predFlags	= NVH264VP1_PRED_FLAG_BI_PRED|NVH264VP1_PRED_FLAG_INTER;
		ref_idx = currMB_s_d->pred_info.ref_idx[LIST_0][l_16_4[index]];
		ref_picid = currMB_s_d->pred_info.ref_pic_id[LIST_0][l_16_4[index]];
		*(int*)&mv[0] = *(int*)&currMB_s_d->pred_info.mv[LIST_0][index].x;
		ref_idx_bw = currMB_s_d->pred_info.ref_idx[LIST_1][l_16_4[index]];
		ref_picid_bw = currMB_s_d->pred_info.ref_pic_id[LIST_1][l_16_4[index]];
		*(int*)&mv_bw[0] = *(int*)&currMB_s_d->pred_info.mv[LIST_1][index].x;

		if (IMGPAR apply_weights)
		{
			weight_info[0] = (*(IMGPAR wbp_weight+(((0+wt_list_offset)*MAX_REFERENCE_PICTURES+ref_idx)*MAX_REFERENCE_PICTURES+ref_idx_bw)*3+0));
			weight_info[1] = (IMGPAR luma_log2_weight_denom<<8);
			weight_info[1] |= ((*(IMGPAR wp_offset+((wt_list_offset+0)*MAX_REFERENCE_PICTURES+ref_idx)*3+0)) & 0xFF);
			weight_info_bw[0] = (*(IMGPAR wbp_weight+(((1+wt_list_offset)*MAX_REFERENCE_PICTURES+ref_idx)*MAX_REFERENCE_PICTURES+ref_idx_bw)*3+0));
			weight_info_bw[1] =(*(IMGPAR wp_offset+((wt_list_offset+1)*MAX_REFERENCE_PICTURES+ref_idx_bw)*3+0));
		}
	}

	if(IMGPAR MbaffFrameFlag)
	{
		if(current_mb_nr&1)
		{
			if(currMB_d->mb_field)
			{
				MB_Partition->predFlags |= 0x30;
				IMGPAR m_lPrev_Field_pair = 1;	
			}
			else
			{
				MB_Partition->predFlags |= 0x20;
				IMGPAR m_lPrev_Field_pair = 0;
			}		
		}
		else
		{
			if(currMB_d->mb_field)
				MB_Partition->predFlags |= 0x10;
			else
				MB_Partition->predFlags |= 0x00;
		}
	}

	MB_Partition->uX				 = (IMGPAR mb_x_d<<4);
	if(IMGPAR MbaffFrameFlag && currMB_d->mb_field)
		MB_Partition->uY		= ((IMGPAR mb_y_d>>1)<<4);
	else
		MB_Partition->uY				 = (IMGPAR mb_y_d<<4);
	*(short*)&MB_Partition->refIndex	 = *(short*)&m_pSurfaceInfo[ref_picid].SInfo[0];
	if(MB_Partition->predFlags & 0x04)
		*(short*)&MB_Partition->refIndex2	= *(short*)&m_pSurfaceInfo[ref_picid_bw].SInfo[0];
	else
		*(short*)&MB_Partition->refIndex2	= 0;
	*(int*)&MB_Partition->mvx= *(int*)&mv[0];
	*(int*)&MB_Partition->wL = *(int*)&weight_info[0];
	*(int*)&MB_Partition->mvx2 = *(int*)&mv_bw[0];
	*(int*)&MB_Partition->wL2 = *(int*)&weight_info_bw[0];
}

//////////////////////////////////////////////////////////////////////////
// CH264DXVA1_ATI Implementation
CH264DXVA1_ATI::CH264DXVA1_ATI()
{

}

CH264DXVA1_ATI::~CH264DXVA1_ATI()
{

}

int CH264DXVA1_ATI::Open PARGS1(int nSurfaceFrame)
{
	CH264DXVA1::Open ARGS1(nSurfaceFrame);

	GetDXVABuffers();

	if (m_nDXVAMode == E_H264_DXVA_MODE_E || m_nDXVAMode == E_H264_DXVA_MODE_F) //VLD mode
	{
		m_lpnalu = (BYTE*)_aligned_malloc((int)m_pAMVACompBufInfo[DXVA_BITSTREAM_DATA_BUFFER].dwBytesToAllocate, 16);
	}
	else //MC mode
	{
		//Use Queue for DXVA1 MC mode
		unsigned int id;
		char tmp_event_handle_name[128];

		bRunThread = TRUE;
		m_Queue_DXVA1 = new queue <ExecuteBuffersStruct_DXVA1>;
		InitializeCriticalSection( &crit_ACCESS_QUEUE );

		stream_global->m_queue_semaphore = CreateSemaphore(
			NULL, 
			0,
			20,
			NULL
			); 
		hEndCpyThread = (HANDLE) CreateEvent(NULL, true, false, NULL);
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

		m_ThreadExecuteHandle_DXVA1 = _beginthreadex(0, 0, ThreadExecute_DXVA1, (void *) this, 0, &id);
	}

	return 0;
}

int CH264DXVA1_ATI::Close PARGS0()
{
	ReleaseDXVABuffers();

	if (m_nDXVAMode == E_H264_DXVA_MODE_E || m_nDXVAMode == E_H264_DXVA_MODE_F) //VLD mode
	{
		_aligned_free(m_lpnalu);
		m_lpnalu = NULL;
	}
	else //MC mode
	{
		int i=20;
		bRunThread = FALSE;
		ReleaseSemaphore(stream_global->m_queue_semaphore, 1, NULL);
		WaitForSingleObject(hEndCpyThread, INFINITE);
		CloseHandle(hEndCpyThread);
		CloseHandle(stream_global->m_queue_semaphore);
		CloseHandle(stream_global->h_dxva_queue_reset_done);
		while(i)
			CloseHandle(stream_global->hReadyForRender[--i]);
		DeleteCriticalSection( &crit_ACCESS_QUEUE );

		CloseHandle((HANDLE)m_ThreadExecuteHandle_DXVA1);
	}

	return CH264DXVA1::Close ARGS0();
}

void CH264DXVA1_ATI::ResetDXVABuffers()
{
	ReleaseDXVABuffers();
	GetDXVABuffers();
	return CH264DXVA1::ResetDXVABuffers();
}

int CH264DXVA1_ATI::GetCompBufferIndex()
{
	int hr;
	while(1)
	{
		if(++m_nLastCompBufIdx == m_dwNumCompBuffers)
			m_nLastCompBufIdx = 0;
		if(m_bCompBufStaus[m_nLastCompBufIdx])
		{
			hr = m_pIHVDServiceDxva1->QueryRenderStatus(DXVA_RESIDUAL_DIFFERENCE_BUFFER, m_nLastCompBufIdx, 0);
			/*
			hr |= m_pIHVDServiceDxva1->QueryRenderStatus(DXVA_PICTURE_DECODE_BUFFER, m_iLastCompBufIdx, 0);
			hr |= m_pIHVDServiceDxva1->QueryRenderStatus(DXVA_SLICE_PARAMETER_BUFFER_264EXT, m_iLastCompBufIdx, 0);
			hr |= m_pIHVDServiceDxva1->QueryRenderStatus(DXVA_MACROBLOCK_CONTROL_BUFFER, m_iLastCompBufIdx, 0);
			hr |= m_pIHVDServiceDxva1->QueryRenderStatus(DXVA_MOTION_VECTOR_BUFFER_264EXT, m_iLastCompBufIdx, 0);
			*/
			if(checkDDError(hr))
				Sleep(2);
			else
			{
				m_bCompBufStaus[m_nLastCompBufIdx] = FALSE;
				break;
			}
		}
	}

	return m_nLastCompBufIdx;
}

HRESULT CH264DXVA1_ATI::GetDXVABuffers()
{
	HRESULT hr = S_OK;
	unsigned int i;
	long lStride;

	hr = CH264DXVA1::GetDXVABuffers();
	if (SUCCEEDED(hr))
	{
		if (m_bConfigBitstreamRaw==0)
		{
			m_dwNumCompBuffers = __min(MAX_COMP_BUF,  __min(m_pAMVACompBufInfo[DXVA_PICTURE_DECODE_BUFFER].dwNumCompBuffers, m_pAMVACompBufInfo[DXVA_SLICE_PARAMETER_BUFFER_264EXT].dwNumCompBuffers));
			m_dwNumCompBuffers = __min(m_dwNumCompBuffers, __min(m_pAMVACompBufInfo[DXVA_MACROBLOCK_CONTROL_BUFFER].dwNumCompBuffers, m_pAMVACompBufInfo[DXVA_MOTION_VECTOR_BUFFER_264EXT].dwNumCompBuffers));
			m_dwNumCompBuffers = __min(m_dwNumCompBuffers, m_pAMVACompBufInfo[DXVA_RESIDUAL_DIFFERENCE_BUFFER].dwNumCompBuffers);

			for(i=0; i<(int)m_dwNumCompBuffers; i++)
			{
				hr |= GetBuffer(DXVA_PICTURE_DECODE_BUFFER, i, FALSE, (void**)&m_pbPictureParamBuf[i], &lStride);
				hr |= GetBuffer(DXVA_SLICE_PARAMETER_BUFFER_264EXT, i, FALSE, (void**)&m_pbSliceCtrlBuf[i], &lStride);
				hr |= GetBuffer(DXVA_MACROBLOCK_CONTROL_BUFFER, i, FALSE, (void**)&m_pbMacroblockCtrlBuf[i], (LONG*)&m_MbBufStride);
				hr |= GetBuffer(DXVA_MOTION_VECTOR_BUFFER_264EXT, i, FALSE, (void**)&m_pbMotionVectorBuf[i], (LONG*)&m_MVBufStride);
				hr |= GetBuffer(DXVA_RESIDUAL_DIFFERENCE_BUFFER, i, FALSE, (void**)&m_pbResidualDiffBuf[i], (LONG*)&m_ReBufStride);
				m_bCompBufStaus[i] = TRUE;
#ifdef use_tmp
				m_pMVtmp[i] = (BYTE*)_aligned_malloc(m_pAMVACompBufInfo[DXVA_MOTION_VECTOR_BUFFER_264EXT].dwBytesToAllocate, 16);
				m_pREtmp[i] = (BYTE*)_aligned_malloc(m_pAMVACompBufInfo[DXVA_RESIDUAL_DIFFERENCE_BUFFER].dwBytesToAllocate*2, 16);
#endif
			}
		}
	}

	return hr;
}

void CH264DXVA1_ATI::ReleaseDXVABuffers()
{
	unsigned int i;

	CH264DXVA1::ReleaseDXVABuffers();

	if (m_bConfigBitstreamRaw==0)
	{
		if(m_bDeblockingFlag)
		{
			for(i=0; i<m_dwNumCompBuffers; i++)
			{
				ReleaseBuffer(DXVA_DEBLOCKING_CONTROL_BUFFER, i);
				m_pbDeblockingCtrlBuf[i] = NULL;
			}
		}
		for(i=0; i<m_dwNumCompBuffers; i++)
		{
			ReleaseBuffer(DXVA_PICTURE_DECODE_BUFFER, i);
			m_pbPictureParamBuf[i] = NULL;
			ReleaseBuffer(DXVA_MACROBLOCK_CONTROL_BUFFER, i);
			m_pbMacroblockCtrlBuf[i] = NULL;
			ReleaseBuffer(DXVA_RESIDUAL_DIFFERENCE_BUFFER, i); 
			m_pbResidualDiffBuf[i] = NULL;
			ReleaseBuffer(DXVA_MOTION_VECTOR_BUFFER_264EXT, i);
			m_pbMotionVectorBuf[i] = NULL;
			ReleaseBuffer(DXVA_SLICE_PARAMETER_BUFFER_264EXT, i); 
			m_pbSliceCtrlBuf[i] = NULL;

			m_bCompBufStaus[i] = FALSE;
#ifdef use_tmp
			_aligned_free(m_pMVtmp[i]);
			m_pMVtmp[i]=0;
			_aligned_free(m_pREtmp[i]);
			m_pREtmp[i]=0;
#endif
		}
	}
}

int CH264DXVA1_ATI::I_Pred_luma_16x16 PARGS1(int predmode)
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
			int mode = 0;	
			int up_avail, left_avail, left_up_avail;

			//left-up
			getCurrNeighbourD_Luma ARGS3(-1, -1, &left_up);

			//left
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
				up_avail      = (up.pMB != NULL);
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

int CH264DXVA1_ATI::I_Pred_luma_4x4 PARGS4(
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

int CH264DXVA1_ATI::I_Pred_luma_8x8 PARGS1(int b8)
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

int CH264DXVA1_ATI::I_Pred_chroma PARGS1(int uv)
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

void CH264DXVA1_ATI::StoreImgRowToDXVACompressedBuffer PARGS2(int start_x, int end_x) 
{
	byte* pDXVAResidueY;
	byte* pDXVAResidueUV;
	BYTE* pResidueRowBuffer = IMGPAR m_lpRESD_Intra_Luma;	// 2 Rows (MBAFF)/1 Row (Non-MBAFF)

	int iWidth = (end_x - start_x + 1) << 5;
	int mb_y = IMGPAR mb_y_d;
	int iPicStride = (IMGPAR PicWidthInMbs) << 5;		
	// 1) 16 bits per pixel 2) No boundary required for compressed residue buffer
	int iPicHeight = IMGPAR m_PicHeight;
	int i;	

	if((m_pIviCP != NULL)&&(m_pIviCP->IsScramblingRequired(MAP_CP_PIC_TYPE(IMGPAR firstSlice->picture_type))))
	{		
		m_pIviCP->EnableScrambling();
		if(IMGPAR MbaffFrameFlag) 
		{
			if(iWidth == iPicStride) 
			{
				//For Luminance
				pDXVAResidueY = IMGPAR m_lpRESD_DXVA + (mb_y >> 1) * iPicStride * 32;	// 32 = 16  * 2 (Rows)
				//For Chorminance
				pDXVAResidueUV = IMGPAR m_lpRESD_DXVA +  iPicStride * iPicHeight + (mb_y >> 1) * iPicStride * 16; // 16 = 8 * 2 (Rows)
				m_pIviCP->ScrambleData(pDXVAResidueY, pResidueRowBuffer, iPicStride*32);
				m_pIviCP->ScrambleData(pDXVAResidueUV, pResidueRowBuffer + iPicStride * 32, iPicStride*16);
			} 
			else
			{
				byte * pSrcResidueY;
				byte * pSrcResidueUV;

				//For Luminance
				pDXVAResidueY = IMGPAR m_lpRESD_DXVA + (mb_y >> 1) * iPicStride * 32 + (start_x<< 5);	
				pDXVAResidueUV = IMGPAR m_lpRESD_DXVA +  iPicStride * iPicHeight + (mb_y >> 1) * iPicStride * 16  + (start_x<< 5);
				pSrcResidueY = pResidueRowBuffer + (start_x << 5);
				pSrcResidueUV = pResidueRowBuffer + iPicStride * 32 + (start_x << 5);

				for(i = 0; i < 32; i++ )
				{
					m_pIviCP->ScrambleData(pDXVAResidueY + iPicStride*i, pSrcResidueY + iPicStride*i, iWidth );						

					if(i < 16)
						m_pIviCP->ScrambleData(pDXVAResidueUV + iPicStride*i, pSrcResidueUV + iPicStride*i, iWidth );
				}
			}
		} 
		else 
		{
			if( iWidth == iPicStride ) 
			{
				//For Luminance
				pDXVAResidueY = IMGPAR m_lpRESD_DXVA + mb_y * iPicStride * 16;	// 1 Row
				//For Chorminance
				pDXVAResidueUV = IMGPAR m_lpRESD_DXVA +  iPicStride * iPicHeight + mb_y * iPicStride * 8; // 1 Row
				m_pIviCP->ScrambleData(pDXVAResidueY, pResidueRowBuffer, iPicStride*16);
				m_pIviCP->ScrambleData(pDXVAResidueUV, pResidueRowBuffer + iPicStride * 16, iPicStride*8);
			}
			else
			{
				byte * pSrcResidueY;
				byte * pSrcResidueUV;

				//For Luminance
				pDXVAResidueY = IMGPAR m_lpRESD_DXVA + mb_y * iPicStride * 16 + (start_x<< 5);	
				pDXVAResidueUV = IMGPAR m_lpRESD_DXVA +  iPicStride * iPicHeight + mb_y * iPicStride * 8  + (start_x<< 5);
				pSrcResidueY = pResidueRowBuffer + (start_x << 5);
				pSrcResidueUV = pResidueRowBuffer + iPicStride * 16 + (start_x << 5);

				for(i = 0; i < 16; i++ )
				{
					m_pIviCP->ScrambleData(pDXVAResidueY + iPicStride*i, pSrcResidueY + iPicStride*i, iWidth );

					if(i < 8)
						m_pIviCP->ScrambleData(pDXVAResidueUV + iPicStride*i, pSrcResidueUV + iPicStride*i, iWidth );
				}							
			}
		}
	} 
	else 
	{
		if(m_pIviCP != NULL)
			m_pIviCP->DisableScrambling();

		if( IMGPAR MbaffFrameFlag ) 
		{
			if(iWidth == iPicStride) 
			{
				//For Luminance
				pDXVAResidueY = IMGPAR m_lpRESD_DXVA + (mb_y >> 1) * iPicStride * 32;	// 32 = 16  * 2 (Rows)
				memcpy(pDXVAResidueY, pResidueRowBuffer, iPicStride*32);
				//For Chorminance
				pDXVAResidueUV = IMGPAR m_lpRESD_DXVA +  iPicStride * iPicHeight + (mb_y >> 1) * iPicStride * 16; // 16 = 8 * 2 (Rows)
				memcpy(pDXVAResidueUV, pResidueRowBuffer + iPicStride * 32, iPicStride*16);
			} 
			else 
			{
				byte * pSrcResidueY;
				byte * pSrcResidueUV;
				//For Luminance
				pDXVAResidueY = IMGPAR m_lpRESD_DXVA + (mb_y >> 1) * iPicStride * 32 + (start_x<< 5);	
				pDXVAResidueUV = IMGPAR m_lpRESD_DXVA +  iPicStride * iPicHeight + (mb_y >> 1) * iPicStride * 16  + (start_x<< 5);
				pSrcResidueY = pResidueRowBuffer + (start_x << 5);
				pSrcResidueUV = pResidueRowBuffer + iPicStride * 32 + (start_x << 5);
				for(i = 0; i < 32; i++ )
					memcpy(pDXVAResidueY + iPicStride*i, pSrcResidueY + iPicStride*i, iWidth );

				//For Chorminance
				for(i = 0; i < 16; i++ )
					memcpy(pDXVAResidueUV + iPicStride*i, pSrcResidueUV + iPicStride*i, iWidth );
			}
		} 
		else 
		{
			if( iWidth == iPicStride ) 
			{
				//For Luminance
				pDXVAResidueY = IMGPAR m_lpRESD_DXVA + mb_y * iPicStride * 16;	// 1 Row
				memcpy(pDXVAResidueY, pResidueRowBuffer, iPicStride*16);
				//For Chorminance
				pDXVAResidueUV = IMGPAR m_lpRESD_DXVA +  iPicStride * iPicHeight + mb_y * iPicStride * 8; // 1 Row
				memcpy(pDXVAResidueUV, pResidueRowBuffer + iPicStride * 16, iPicStride*8);
			} 
			else 
			{
				byte * pSrcResidueY;
				byte * pSrcResidueUV;

				pDXVAResidueY = IMGPAR m_lpRESD_DXVA + mb_y * iPicStride * 16 + (start_x<< 5);	
				pDXVAResidueUV = IMGPAR m_lpRESD_DXVA +  iPicStride * iPicHeight + mb_y * iPicStride * 8  + (start_x<< 5);
				pSrcResidueY = pResidueRowBuffer + (start_x << 5);
				pSrcResidueUV = pResidueRowBuffer + iPicStride * 16 + (start_x << 5);
				//For Luminance
				for(i = 0; i < 16; i++ )
					memcpy(pDXVAResidueY + iPicStride*i, pSrcResidueY + iPicStride*i, iWidth );
				//For Chorminance
				for(i = 0; i < 8; i++ ) 
					memcpy(pDXVAResidueUV + iPicStride*i, pSrcResidueUV + iPicStride*i, iWidth );
			}
		}
	}

	return;
}

int CH264DXVA1_ATI::decode_one_macroblock_Intra PARGS0()
{
	int cbp_blk = currMB_s_d->cbp_blk;
	int i=0, j=0;
	//int ioff, joff;
	int b8;
	short* pos_plane;
	//__declspec(align(16)) short Residual[256];
#if   !defined(H264_ENABLE_INTRINSICS)
	__declspec(align(16)) short Residual_UV[128];
#endif
	//int offset;	
	WORD temp1,temp2;
	__m64 mm0;
#if defined(ONE_COF)
	short *pcof = (short*)IMGPAR cof;
#else
	short *pcof = (short*)IMGPAR cof_d;
#endif
	int slice_start_mb_nr = IMGPAR MbaffFrameFlag ? (IMGPAR currentSlice->start_mb_nr<<1) : IMGPAR currentSlice->start_mb_nr;

	if(IMGPAR current_mb_nr_d == slice_start_mb_nr)
	{
		if (stream_global->m_is_MTMS)
		{
			int iPicWidthInMBminus1 = (int)active_sps.pic_width_in_mbs_minus1;

			m_PicWidth = (iPicWidthInMBminus1+1)*16;
			IMGPAR m_PicHeight = IMGPAR PicHeightInMbs*16;
			IMGPAR m_MVBufSwitch = (IMGPAR m_PicHeight>>2)*(m_MVBufStride>>2);
			IMGPAR m_SBBufOffset = (IMGPAR m_PicHeight>>4)*(m_PicWidth>>2);
			m_MbBufStride = (m_PicWidth>>1);
			m_ReBufStride = (m_PicWidth<<1);
		}

		build_slice_parameter_buffer ARGS0();
	}

	H264_MC_CTRLMAP* MB_MC_Ctrl = (H264_MC_CTRLMAP*)(IMGPAR m_lpMBLK_Intra_Luma+IMGPAR mb_y_d*(m_PicWidth>>2)+(IMGPAR mb_x_d<<2));
	H264_SB_CTRLMAP* MB_SB_Ctrl = (H264_SB_CTRLMAP*)(IMGPAR m_lpMBLK_Intra_Luma+IMGPAR m_SBBufOffset+(IMGPAR mb_y_d<<2)*m_MbBufStride+(IMGPAR mb_x_d<<3));
	//H264_RD* MB_Residual = (H264_RD*)(IMGPAR m_lpRESD_Intra_Luma+(IMGPAR mb_y_d<<5)*m_PicWidth+(IMGPAR mb_x_d<<5));
	//H264_RD* MB_Residual = (H264_RD*)(IMGPAR m_lpRESD_Intra_Luma+((IMGPAR mb_y_d & 1)<<5)*m_PicWidth+(IMGPAR mb_x_d<<5));
	H264_RD* MB_Residual;
	if ( IMGPAR MbaffFrameFlag ) {
		MB_Residual = (H264_RD*)(IMGPAR m_lpRESD_Intra_Luma + ((IMGPAR mb_y_d & 1)<<5)*m_PicWidth + (IMGPAR mb_x_d<<5));	
	} else {
		MB_Residual = (H264_RD*)(IMGPAR m_lpRESD_Intra_Luma + (IMGPAR mb_x_d<<5));	
	}

	*MB_MC_Ctrl  = H264_INTRA_PREDICTION;
	*MB_MC_Ctrl |= ((currMB_d->mb_field)&&(dec_picture->MbaffFrameFlag))? H264_FIELD_MB:0;
	*MB_MC_Ctrl |= (IMGPAR current_slice_nr << 8);

	if (currMB_d->mb_type == I16MB)
	{
		temp1 = (I_Pred_luma_16x16 ARGS1(currMB_d->i16mode) << 2);
		temp1 |= (I_Pred_chroma ARGS1(0) << 12);
		mm0 = _mm_set1_pi16(temp1);
		for(j=0; j<4 ; j++)
		{
			*((__m64*)MB_SB_Ctrl) = mm0;
			MB_SB_Ctrl += (m_MbBufStride>>1);
		}

		for(i=0; i<16; i+=2)
		{	
			pos_plane = (short*)(MB_Residual + ((i&12)*m_PicWidth) + ((i&3)<<2));
			iDCT_4x4_2_ATI(pos_plane, pcof, m_PicWidth);
			//cbp_blk >>= 2; 
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
			*((__m64*)(MB_SB_Ctrl+(m_MbBufStride>>1))) = mm0;
			MB_SB_Ctrl += ((m_MbBufStride>>1)<<1);  
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
			pos_plane = (short*)(MB_Residual + ((b8>>1)*m_PicWidth<<3) + ((b8&1)<<3));

			if (currMB_d->cbp&(1<<b8))
#if defined(ONE_COF)
				iDCT_8x8_fcn(pos_plane, &IMGPAR cof[b8][0][0][0],m_PicWidth);
#else				
				iDCT_8x8_fcn(pos_plane, IMGPAR cof_d + (b8<<6),m_PicWidth);
#endif
			else {

				memset(pos_plane, 0, 16);
				memset(pos_plane+m_PicWidth, 0, 16);
				memset(pos_plane+(m_PicWidth<<1), 0, 16);
				memset(pos_plane+(m_PicWidth<<1)+m_PicWidth, 0, 16);
				memset(pos_plane+(m_PicWidth<<2), 0, 16);
				memset(pos_plane+(m_PicWidth<<2)+m_PicWidth, 0, 16);
				memset(pos_plane+(m_PicWidth<<2)+(m_PicWidth<<1), 0, 16);
				memset(pos_plane+(m_PicWidth<<3)-m_PicWidth, 0, 16);
			}
		}		
	} else if(currMB_d->mb_type == I4MB) {	
		for(j=0; j<4 ; j++)
		{
			for(i=0; i<4; i++)
			{
				MB_SB_Ctrl[i]  = (I_Pred_luma_4x4 ARGS4((i<<2), (j<<2), (IMGPAR block_x_d+i), (IMGPAR block_y_d+j)) << 2);
				MB_SB_Ctrl[i] |= (IMGPAR m_UpRight << 1);
				MB_SB_Ctrl[i] |= (I_Pred_chroma ARGS1(0) << 12);
			}
			MB_SB_Ctrl += (m_MbBufStride>>1);
		}
		for(i=0; i<16; i+=2) {
			pos_plane = (short*)(MB_Residual + ((i&12)*m_PicWidth) + ((i&3)<<2));
			iDCT_4x4_2_ATI(pos_plane, pcof, m_PicWidth);
			//cbp_blk >>= 2;
			pcof += 32;
		}		
	}
	else if(currMB_d->mb_type == IPCM) {
		//For CABAC decoding of Dquant
		last_dquant=0;
	}

#ifdef __SUPPORT_YUV400__
	if (dec_picture->chroma_format_idc != YUV400)
	{
#endif
		//MB_Residual = (H264_RD*)(IMGPAR m_lpRESD_Intra_Luma+((IMGPAR mb_y_d<<4)+IMGPAR m_PicHeight*2)*m_PicWidth+(IMGPAR mb_x_d<<5));
		//MB_Residual = (H264_RD*)(IMGPAR m_lpRESD_Intra_Luma+(((IMGPAR mb_y_d & 1)<<4)+64)*m_PicWidth+(IMGPAR mb_x_d<<5));
		if ( IMGPAR MbaffFrameFlag ) {
			MB_Residual = (H264_RD*)(IMGPAR m_lpRESD_Intra_Luma+(((IMGPAR mb_y_d & 1)<<4)+64)*m_PicWidth+(IMGPAR mb_x_d<<5));	
		} else {
			MB_Residual = (H264_RD*)(IMGPAR m_lpRESD_Intra_Luma + m_PicWidth*32 + (IMGPAR mb_x_d<<5));	
		}

		pcof = (short*)IMGPAR cof_d;
		cbp_blk = currMB_s_d->cbp_blk;
		if(currMB_d->mb_type != IPCM)
		{
#if   !defined(H264_ENABLE_INTRINSICS)
			short tmp4[4][4];
			int ioff,joff;
			for(int uv=0;uv<2;uv++)
			{			
				for(b8=0;b8<4;b8++)
				{				
					ioff = (b8&1)<<2;	//0 4 0 4
					joff = (b8&2)<<1;	//0 0 4 4
					if( cbp_blk & (1<<cbp_blk_chroma[uv][b8]))
#if defined(ONE_COF)
						iDCT_4x4_fcn(&tmp4[0][0], &IMGPAR cof[4+uv][b8][0][0],4);
#else
						iDCT_4x4_fcn(&tmp4[0][0], (IMGPAR cof_d + ((4+uv<<6)+(b8<<4))),4);
#endif
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
				pos_plane = (short*)( MB_Residual + ((b8>>1)*m_PicWidth<<2) + ((b8&1)<<3) ) ;

				iDCT_4x4_UV_fcn(//&Residual_UV[(((b8&1)<<3)+((b8&2)<<5))]
					pos_plane
#if defined(ONE_COF)
					, &pcof[4][b8][0][0]
#else
					, (pcof + ((4<<6)+(b8<<4)))
#endif
					,( cbp_blk & (1<<cbp_blk_chroma[0][b8])) // residual U
					,( cbp_blk & (1<<cbp_blk_chroma[1][b8])) // residual V
					, m_PicWidth/*16 */);
			}
#endif
		}		

#ifdef __SUPPORT_YUV400__
	}
#endif

	if ( ( IMGPAR mb_x_d == IMGPAR PicWidthInMbs - 1) && ( (IMGPAR mb_y_d & 1) || (IMGPAR MbaffFrameFlag == 0)) ) {
		StoreImgRowToDXVACompressedBuffer ARGS2(IMGPAR start_mb_x, IMGPAR mb_x_d);
	}
	return 0;
}

int CH264DXVA1_ATI::decode_one_macroblock_Inter PARGS0()
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
			int iPicWidthInMBminus1 = (int)active_sps.pic_width_in_mbs_minus1;

			m_PicWidth = (iPicWidthInMBminus1+1)*16;
			IMGPAR m_PicHeight = IMGPAR PicHeightInMbs*16;
			IMGPAR m_MVBufSwitch = (IMGPAR m_PicHeight>>2)*(m_MVBufStride>>2);
			IMGPAR m_SBBufOffset = (IMGPAR m_PicHeight>>4)*(m_PicWidth>>2);
			m_MbBufStride = (m_PicWidth>>1);
			m_ReBufStride = (m_PicWidth<<1);
		}

		build_slice_parameter_buffer ARGS0();
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

	if ( ( IMGPAR mb_x_d == IMGPAR PicWidthInMbs - 1) && ( (IMGPAR mb_y_d & 1) || (IMGPAR MbaffFrameFlag == 0)) ) {
		StoreImgRowToDXVACompressedBuffer ARGS2(IMGPAR start_mb_x, IMGPAR mb_x_d);
	}

	return 0;
}

void CH264DXVA1_ATI::build_macroblock_buffer_Inter_16x16 PARGS1(int list_offset)
{
	int pred_dir = currMB_d->b8pdir[0];
	int cbp_blk = currMB_d->cbp;
	int cbp=0, j, ref_picid, ref_picid_se;
	int curr_mb_field = ((IMGPAR MbaffFrameFlag)&&(currMB_s_d->mb_field));
	int current_mb_nr = IMGPAR current_mb_nr_d;

	H264_MC_CTRLMAP* MB_MC_Ctrl = (H264_MC_CTRLMAP*)(IMGPAR m_lpMBLK_Intra_Luma+IMGPAR mb_y_d*(m_PicWidth>>2)+(IMGPAR mb_x_d<<2));
	H264_SB_CTRLMAP* MB_SB_Ctrl = (H264_SB_CTRLMAP*)(IMGPAR m_lpMBLK_Intra_Luma+IMGPAR m_SBBufOffset+(IMGPAR mb_y_d<<2)*m_MbBufStride+(IMGPAR mb_x_d<<3));
	*MB_MC_Ctrl  = curr_mb_field ? H264_FIELD_MB:0;
	*MB_MC_Ctrl |= (IMGPAR current_slice_nr << 8);

	H264_MVMAP* MB_MV = (H264_MVMAP*)(IMGPAR m_lpMV+(IMGPAR mb_y_d<<2)*m_MVBufStride+(IMGPAR mb_x_d<<4));
	MB_MV += IMGPAR m_MVBufSwitch;

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

		MB_SB_Ctrl += (m_MbBufStride>>1);
		MB_MV += (m_MVBufStride>>2);
	}
}

void CH264DXVA1_ATI::build_macroblock_buffer_Inter_16x8 PARGS1(int list_offset)
{
	int pred_dir;
	int cbp_blk = currMB_d->cbp;
	int cbp=0, joff, j, k,  ref_picid, ref_picid_se;
	int curr_mb_field = ((IMGPAR MbaffFrameFlag)&&(currMB_s_d->mb_field));
	int current_mb_nr = IMGPAR current_mb_nr_d;

	H264_MC_CTRLMAP* MB_MC_Ctrl = (H264_MC_CTRLMAP*)(IMGPAR m_lpMBLK_Intra_Luma+IMGPAR mb_y_d*(m_PicWidth>>2)+(IMGPAR mb_x_d<<2));
	H264_SB_CTRLMAP* MB_SB_Ctrl = (H264_SB_CTRLMAP*)(IMGPAR m_lpMBLK_Intra_Luma+IMGPAR m_SBBufOffset+(IMGPAR mb_y_d<<2)*m_MbBufStride+(IMGPAR mb_x_d<<3));
	*MB_MC_Ctrl  = curr_mb_field ? H264_FIELD_MB:0;
	*MB_MC_Ctrl |= (IMGPAR current_slice_nr << 8);

	H264_MVMAP* MB_MV = (H264_MVMAP*)(IMGPAR m_lpMV+(IMGPAR mb_y_d<<2)*m_MVBufStride+(IMGPAR mb_x_d<<4));
	MB_MV += IMGPAR m_MVBufSwitch;

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

			MB_SB_Ctrl += (m_MbBufStride>>1);
			MB_MV += (m_MVBufStride>>2);
		}
	}
}

void CH264DXVA1_ATI::build_macroblock_buffer_Inter_8x16 PARGS1(int list_offset)
{
	int pred_dir;
	int cbp_blk = currMB_d->cbp;
	int cbp=0, i4, j, k, ref_picid, ref_picid_se;	
	int curr_mb_field = ((IMGPAR MbaffFrameFlag)&&(currMB_s_d->mb_field));
	int current_mb_nr = IMGPAR current_mb_nr_d;

	H264_MC_CTRLMAP* MB_MC_Ctrl = (H264_MC_CTRLMAP*)(IMGPAR m_lpMBLK_Intra_Luma+IMGPAR mb_y_d*(m_PicWidth>>2)+(IMGPAR mb_x_d<<2));
	H264_SB_CTRLMAP* MB_SB_Ctrl = (H264_SB_CTRLMAP*)(IMGPAR m_lpMBLK_Intra_Luma+IMGPAR m_SBBufOffset+(IMGPAR mb_y_d<<2)*m_MbBufStride+(IMGPAR mb_x_d<<3));
	*MB_MC_Ctrl  = curr_mb_field ? H264_FIELD_MB:0;
	*MB_MC_Ctrl |= (IMGPAR current_slice_nr << 8);

	H264_MVMAP* MB_MV = (H264_MVMAP*)(IMGPAR m_lpMV+(IMGPAR mb_y_d<<2)*m_MVBufStride+(IMGPAR mb_x_d<<4));
	MB_MV += IMGPAR m_MVBufSwitch;

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

			MB_SB_Ctrl += (m_MbBufStride>>1);
			MB_MV += (m_MVBufStride>>2);
		}

		MB_SB_Ctrl -= (m_MbBufStride<<1);
		MB_SB_Ctrl += 2;
		MB_MV -= m_MVBufStride;
		MB_MV += 2;
	}
}

void CH264DXVA1_ATI::build_macroblock_buffer_Inter_8x8 PARGS1(int list_offset)
{	
	int cbp_blk = currMB_d->cbp;
	int temp=0, i, b4, b8, ref_picid, ref_picid_se;
	int pred_dir,mode;
	int fw_refframe,bw_refframe;
	static const int b84_idx[4][2] = { {0,4} , {2,6} , {8,12} , {10,14} };
	static const int b48_idx[4][2] = { {0,1} , {2,3} , {8,9} , {10,11} };
	static const int b44_idx[4][4] = { {0,1,4,5} , {2,3,6,7} , {8,9,12,13} , {10,11,14,15} };
	int curr_mb_field = ((IMGPAR MbaffFrameFlag)&&(currMB_s_d->mb_field));
	int current_mb_nr = IMGPAR current_mb_nr_d;

	H264_MC_CTRLMAP* MB_MC_Ctrl = (H264_MC_CTRLMAP*)(IMGPAR m_lpMBLK_Intra_Luma+IMGPAR mb_y_d*(m_PicWidth>>2)+(IMGPAR mb_x_d<<2));
	H264_SB_CTRLMAP* MB_SB_Ctrl = (H264_SB_CTRLMAP*)(IMGPAR m_lpMBLK_Intra_Luma+IMGPAR m_SBBufOffset+(IMGPAR mb_y_d<<2)*m_MbBufStride+(IMGPAR mb_x_d<<3));
	*MB_MC_Ctrl  = curr_mb_field ? H264_FIELD_MB:0;
	*MB_MC_Ctrl |= (IMGPAR current_slice_nr << 8);

	H264_MVMAP* MB_MV;
	H264_SB_CTRLMAP* MB_SB_Ctrl_o = MB_SB_Ctrl;
	H264_MVMAP* MB_MV_o = (H264_MVMAP*)(IMGPAR m_lpMV+(IMGPAR mb_y_d<<2)*m_MVBufStride+(IMGPAR mb_x_d<<4));
	MB_MV_o += IMGPAR m_MVBufSwitch;

	H264_SB_CTRLMAP MB_SB_Ctrl_temp;
	H264_MVMAP MB_MV_0, MB_MV_1;

	for (b8=0; b8<4; b8++)
	{
		MB_SB_Ctrl = MB_SB_Ctrl_o + ((b8&1)<<1) + m_MbBufStride*((b8&2)>>1);
		MB_MV = MB_MV_o + ((b8&1)<<1) + (m_MVBufStride>>1)*((b8&2)>>1);

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
				MB_SB_Ctrl[m_MbBufStride>>1] = MB_SB_Ctrl[(m_MbBufStride>>1)+1] = MB_SB_Ctrl_temp;

			if(pred_dir != 2)
			{
				MB_MV[0] = MB_MV[1] = 
					MB_MV[(m_MVBufStride>>2)] = MB_MV[(m_MVBufStride>>2)+1] = MB_MV_0;
			}
			else
			{
				MB_MV[0] = MB_MV[1] = 
					MB_MV[(m_MVBufStride>>2)] = MB_MV[(m_MVBufStride>>2)+1] = MB_MV_1;

				MB_MV[0-IMGPAR m_MVBufSwitch] = MB_MV[1-IMGPAR m_MVBufSwitch] = 
					MB_MV[(m_MVBufStride>>2)-IMGPAR m_MVBufSwitch] = MB_MV[(m_MVBufStride>>2)+1-IMGPAR m_MVBufSwitch] = MB_MV_0;				
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
				MB_SB_Ctrl += (m_MbBufStride>>1);
				MB_MV += (m_MVBufStride>>2);
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
					MB_SB_Ctrl[i+(m_MbBufStride>>1)] = 0;
					MB_SB_Ctrl[i+(m_MbBufStride>>1)] |= (IMGPAR apply_weights ? (1<<1):0);
					MB_SB_Ctrl[i+(m_MbBufStride>>1)] |= (pred_dir ? (1<<10):0);
					MB_SB_Ctrl[i+(m_MbBufStride>>1)] |= (m_pSurfaceInfo[ref_picid].SInfo[1] ? (1<<11):0);
					if(!curr_mb_field)
					{
						if(pred_dir)
							MB_SB_Ctrl[i+(m_MbBufStride>>1)] |= (WORD)((IMGPAR m_L0L1Map[currMB_s_d->pred_info.ref_idx[pred_dir][b8]]) << 12);
						else
							MB_SB_Ctrl[i+(m_MbBufStride>>1)] |= (WORD)(currMB_s_d->pred_info.ref_idx[pred_dir][b8] << 12);
					}
					else
					{
						if(current_mb_nr&1)
						{
							if(pred_dir)
								MB_SB_Ctrl[i+(m_MbBufStride>>1)] |= (WORD)((IMGPAR m_BotL1Map[currMB_s_d->pred_info.ref_idx[pred_dir][b8]]) << 12);
							else
								MB_SB_Ctrl[i+(m_MbBufStride>>1)] |= (WORD)((IMGPAR m_BotL0Map[currMB_s_d->pred_info.ref_idx[pred_dir][b8]]) << 12);
						}
						else
						{
							if(pred_dir)
								MB_SB_Ctrl[i+(m_MbBufStride>>1)] |= (WORD)((IMGPAR m_TopL1Map[currMB_s_d->pred_info.ref_idx[pred_dir][b8]]) << 12);
							else
								MB_SB_Ctrl[i+(m_MbBufStride>>1)] |= (WORD)((IMGPAR m_TopL0Map[currMB_s_d->pred_info.ref_idx[pred_dir][b8]]) << 12);
						}
					}

					MB_MV_0 = ((DWORD)(currMB_s_d->pred_info.mv[pred_dir][b48_idx[b8][i]].x) & 0x0000ffff);
					MB_MV_0 |= (DWORD)(currMB_s_d->pred_info.mv[pred_dir][b48_idx[b8][i]].y << 16);

					MB_MV[i] = 
						MB_MV[i+(m_MVBufStride>>2)] = MB_MV_0;	
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
					MB_SB_Ctrl[i+(m_MbBufStride>>1)] = 1;
					MB_SB_Ctrl[i+(m_MbBufStride>>1)] |= (IMGPAR apply_weights ? (1<<1):0);
					MB_SB_Ctrl[i+(m_MbBufStride>>1)] |= (m_pSurfaceInfo[ref_picid].SInfo[1] ? (1<<3):0);
					MB_SB_Ctrl[i+(m_MbBufStride>>1)] |= (m_pSurfaceInfo[ref_picid_se].SInfo[1] ? (1<<11):0);	
					if(!curr_mb_field)
					{
						MB_SB_Ctrl[i+(m_MbBufStride>>1)] |= (WORD)(currMB_s_d->pred_info.ref_idx[LIST_0][b8] << 4);
						MB_SB_Ctrl[i+(m_MbBufStride>>1)] |= (WORD)((IMGPAR m_L0L1Map[currMB_s_d->pred_info.ref_idx[LIST_1][b8]]) << 12);
					}
					else
					{
						if(current_mb_nr&1)
						{
							MB_SB_Ctrl[i+(m_MbBufStride>>1)] |= (WORD)((IMGPAR m_BotL0Map[currMB_s_d->pred_info.ref_idx[LIST_0][b8]]) << 4);
							MB_SB_Ctrl[i+(m_MbBufStride>>1)] |= (WORD)((IMGPAR m_BotL1Map[currMB_s_d->pred_info.ref_idx[LIST_1][b8]]) << 12);
						}
						else
						{
							MB_SB_Ctrl[i+(m_MbBufStride>>1)] |= (WORD)((IMGPAR m_TopL0Map[currMB_s_d->pred_info.ref_idx[LIST_0][b8]]) << 4);
							MB_SB_Ctrl[i+(m_MbBufStride>>1)] |= (WORD)((IMGPAR m_TopL1Map[currMB_s_d->pred_info.ref_idx[LIST_1][b8]]) << 12);
						}
					}

					MB_MV_0 = ((DWORD)(currMB_s_d->pred_info.mv[LIST_0][b48_idx[b8][i]].x) & 0x0000ffff);
					MB_MV_0 |= (DWORD)(currMB_s_d->pred_info.mv[LIST_0][b48_idx[b8][i]].y << 16);

					MB_MV[i-IMGPAR m_MVBufSwitch] = 
						MB_MV[i+(m_MVBufStride>>2)-IMGPAR m_MVBufSwitch] = MB_MV_0;

					MB_MV_1 = ((DWORD)(currMB_s_d->pred_info.mv[LIST_1][b48_idx[b8][i]].x) & 0x0000ffff);
					MB_MV_1 |= (DWORD)(currMB_s_d->pred_info.mv[LIST_1][b48_idx[b8][i]].y << 16);

					MB_MV[i] = 
						MB_MV[i+(m_MVBufStride>>2)] = MB_MV_1;	
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
					MB_SB_Ctrl += (m_MbBufStride>>1);
					MB_MV += (m_MVBufStride>>2);
				}
			}
		}
	}
}	

void CH264DXVA1_ATI::build_residual_buffer_Inter PARGS0()
{
	int b8, b4;	
#if defined(ONE_COF)
	short *pcof = (short*)IMGPAR cof;
#else
	short *pcof = (short*) IMGPAR cof_d;
#endif
	int cbp_8x8 = currMB_d->cbp;
	int cbp_blk = currMB_s_d->cbp_blk;
	static const int b44_idx[4][4] = { {0,1,4,5} , {2,3,6,7} , {8,9,12,13} , {10,11,14,15} };
	static const int b44_offset[4][4] = { {0,16,64,80} , {32,48,96,112} , {128,144,192,208} , {160,176,224,240} };
	int iPicWidth = m_PicWidth;
	int iPicHeight = IMGPAR m_PicHeight;
	int mb_y = IMGPAR mb_y_d;
	int mb_x = IMGPAR mb_x_d;
	BYTE* pResiduePlane = IMGPAR m_lpRESD_Intra_Luma;

	//H264_RD* MB_Residual = (H264_RD*)(pResiduePlane+(mb_y<<5)*iPicWidth+(mb_x<<5));
	H264_RD* MB_Residual;

	if ( IMGPAR MbaffFrameFlag ) {
		MB_Residual = (H264_RD*)(pResiduePlane + ((mb_y & 1)<<5)*iPicWidth + (mb_x<<5));	
	} else {
		MB_Residual = (H264_RD*)(pResiduePlane + (mb_x<<5));	
	}

	__m64 mm0 = _mm_setzero_si64();
	__m128i xmm0 = _mm_setzero_si128(); // all 0s

	//luma
	for(b8=0 ; b8<4 ; b8++)
	{
		register short* b8off_plane = (short*)(MB_Residual + ((b8>>1)*iPicWidth<<3) + ((b8&1)<<3));
		if(1 & (cbp_8x8>>b8))
		{
			if (!currMB_d->luma_transform_size_8x8_flag)
			{
				for(b4=0 ; b4<4 ; b4+=2)
				{		
					register short*  offset_plane = b8off_plane + ((b4>>1)*iPicWidth<<2) + ((b4&1)<<2) ;
					iDCT_4x4_2_ATI(offset_plane, pcof+b44_offset[b8][b4], iPicWidth);
				} 
			}
			else
#if defined(ONE_COF)
				iDCT_8x8_fcn(b8off_plane, &IMGPAR cof[b8][0][0][0], iPicWidth);
#else				
				iDCT_8x8_fcn(b8off_plane, IMGPAR cof_d + (b8<<6), iPicWidth);
#endif
		} else {			
			_mm_store_si128((__m128i*)b8off_plane, xmm0); //0
			b8off_plane += iPicWidth;			
			_mm_store_si128((__m128i*)b8off_plane, xmm0); //0
			b8off_plane += iPicWidth;			
			_mm_store_si128((__m128i*)b8off_plane, xmm0); //0
			b8off_plane += iPicWidth;			
			_mm_store_si128((__m128i*)b8off_plane, xmm0); //0
			b8off_plane += iPicWidth;			
			_mm_store_si128((__m128i*)b8off_plane, xmm0); //0
			b8off_plane += iPicWidth;			
			_mm_store_si128((__m128i*)b8off_plane, xmm0); //0
			b8off_plane += iPicWidth;			
			_mm_store_si128((__m128i*)b8off_plane, xmm0); //0
			b8off_plane += iPicWidth;			
			_mm_store_si128((__m128i*)b8off_plane, xmm0); //0

			/*
			memset(b8off_plane, 0, 16);
			memset(b8off_plane + iPicWidth, 0, 16);
			memset(b8off_plane + (iPicWidth<<1), 0, 16);
			memset(b8off_plane + (iPicWidth<<1)+iPicWidth, 0, 16);
			memset(b8off_plane + (iPicWidth<<2), 0, 16);
			memset(b8off_plane + (iPicWidth<<2)+iPicWidth, 0, 16);
			memset(b8off_plane + (iPicWidth<<2)+(iPicWidth<<1), 0, 16);
			memset(b8off_plane + (iPicWidth<<3)-iPicWidth, 0, 16);
			*/

		}
	}

	//chroma
	//MB_Residual = (H264_RD*)(pResiduePlane+((mb_y<<4)+ iPicHeight*2)*iPicWidth+(mb_x<<5));	
	if ( IMGPAR MbaffFrameFlag ) {
		MB_Residual = (H264_RD*)(pResiduePlane+(((mb_y & 1)<<4)+64)*iPicWidth+(mb_x<<5));	
	} else {
		MB_Residual = (H264_RD*)(pResiduePlane + iPicWidth*32 + (mb_x<<5));	
	}

#if   !defined(H264_ENABLE_INTRINSICS)
	short tmp4[4][4];
	int uv, j, i;
	int ioff,joff;
	__declspec(align(16)) short Residual_UV[128];

	for(uv=0;uv<2;uv++)
	{			
		for(b8=0;b8<4;b8++)
		{				
			 ioff = (b8&1)<<2;	//0 4 0 4
			 joff = (b8&2)<<1;	//0 0 4 4
			if( cbp_blk & (1<<cbp_blk_chroma[uv][b8]))
#if defined(ONE_COF)
				iDCT_4x4_fcn(&tmp4[0][0], &pcof[4+uv][b8][0][0],4);
#else
				iDCT_4x4_fcn(&tmp4[0][0], (pcof + ((4+uv<<6)+(b8<<4))),4);
#endif
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
		register short* offset_plane = (short*)( MB_Residual + ((b8>>1)*iPicWidth<<2) + ((b8&1)<<3) ) ;

		//		ioff = (b8&1)<<3;	//0 8 0  8
		//		joff = (b8&2)<<5;	//0 0 64 64

		iDCT_4x4_UV_fcn(//&Residual_UV[(joff+ioff)]
			offset_plane
#if defined(ONE_COF)
			, &IMGPAR cof[4][b8][0][0]
#else
			, (pcof + ((4<<6)+(b8<<4)))
#endif
			,( cbp_blk & (1<<cbp_blk_chroma[0][b8])) // residual U
			,( cbp_blk & (1<<cbp_blk_chroma[1][b8])) // residual V
			, iPicWidth);
	}
#endif

}

void CH264DXVA1_ATI::BeginDecodeFrame PARGS0()
{
	if(!m_pUncompBufQueue)
		return;

	HRESULT hr = S_OK;
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
			return;

		ResetDXVABuffers();
#endif

		m_bResolutionChange = FALSE;
	}

	if(m_nDXVAMode == E_H264_DXVA_ATI_PROPRIETARY_A || IMGPAR Hybrid_Decoding == 5)
	{
		m_MVBufStride = ((m_nDXVAMode==E_H264_DXVA_ATI_PROPRIETARY_A) ? (m_pAMVACompBufInfo[DXVA_MOTION_VECTOR_BUFFER_264EXT].dwBytesToAllocate>>1)/(Alignment16(IMGPAR height)>>2) : (IMGPAR m_pic_width_in_mbs<<2) * sizeof(DWORD));
		IMGPAR m_lFrame_Counter = ++m_nFrameCounter;

		if ( ((IMGPAR Hybrid_Decoding==2) && (IMGPAR type == B_SLICE)) ||
			(IMGPAR Hybrid_Decoding==0) || 
			((IMGPAR Hybrid_Decoding==1) && (IMGPAR type != I_SLICE)) )
		{
			if (!((IMGPAR smart_dec & SMART_DEC_SKIP_FIL_B) && IMGPAR currentSlice->picture_type == B_SLICE && IMGPAR currentSlice->structure != FRAME && IMGPAR currentSlice->m_pic_combine_status == 0 && !IMGPAR nal_reference_idc))
			{
				EnterCriticalSection( &crit_GetCompBuf );

				DEBUG_SHOW_HW_INFO("ATI DXVA: Begin decode frame\n", IMGPAR UnCompress_Idx);
				IMGPAR m_Intra_lCompBufIndex = GetCompBufferIndex();
				DEBUG_SHOW_HW_INFO("ATI DXVA: Get Index %d form compressed buffer\n", IMGPAR m_Intra_lCompBufIndex);

				//First Field
				if (IMGPAR currentSlice->structure != FRAME && IMGPAR currentSlice->m_pic_combine_status)
					IMGPAR m_iFirstCompBufIndex = IMGPAR m_Intra_lCompBufIndex;

				IMGPAR m_lpMBLK_Intra_Luma = m_pbMacroblockCtrlBuf[IMGPAR m_Intra_lCompBufIndex];
				IMGPAR m_lpRESD_DXVA = m_pbResidualDiffBuf[IMGPAR m_Intra_lCompBufIndex];
#ifdef use_tmp
				IMGPAR m_lpMV = m_pMVtmp[IMGPAR m_Intra_lCompBufIndex];
				//IMGPAR m_lpRESD_Intra_Luma = m_pREtmp[IMGPAR m_Intra_lCompBufIndex];	//Use Row Level DXVA Residue transfer
#else
				IMGPAR m_lpMV = m_lpMVBuf[IMGPAR m_Intra_lCompBufIndex];
				IMGPAR m_lpRESD_Intra_Luma = m_lpRESDBuf[IMGPAR m_Intra_lCompBufIndex];
#endif

				LeaveCriticalSection( &crit_GetCompBuf );
			}

			int iPicWidthInMBminus1 = (int)active_sps.pic_width_in_mbs_minus1;

			m_PicWidth = (iPicWidthInMBminus1+1)*16;
			IMGPAR m_PicHeight = IMGPAR PicHeightInMbs*16;
			IMGPAR m_MVBufSwitch = (IMGPAR m_PicHeight>>2)*(m_MVBufStride>>2);
			IMGPAR m_SBBufOffset = (IMGPAR m_PicHeight>>4)*(m_PicWidth>>2);
			m_MbBufStride = (m_PicWidth>>1);
			m_ReBufStride = ((m_nDXVAMode==E_H264_DXVA_ATI_PROPRIETARY_A) ? (m_pAMVACompBufInfo[DXVA_RESIDUAL_DIFFERENCE_BUFFER].dwBytesToAllocate*2/3)/Alignment16(IMGPAR height) : (m_PicWidth<<1));
		}
		else
		{
			if(IMGPAR currentSlice->structure == FRAME || IMGPAR currentSlice->m_pic_combine_status)
			{
				IMGPAR UnCompress_Idx = (int)m_pUncompBufQueue->GetItem();
#if defined(_USE_QUEUE_FOR_DXVA1_)
				WaitForSingleObject(stream_global->hReadyForRender[IMGPAR UnCompress_Idx], INFINITE);
				ResetEvent(stream_global->hReadyForRender[IMGPAR UnCompress_Idx]);
#endif
			}

			hr = BeginFrame(IMGPAR UnCompress_Idx, 0);
			hr |= GetBuffer(-1, IMGPAR UnCompress_Idx, FALSE, (void**)&(IMGPAR m_pUnCompressedBuffer), &(IMGPAR m_UnCompressedBufferStride));

			SetSurfaceInfo ARGS3(dec_picture->unique_id,IMGPAR structure==2?1:0,1);
		}
	}
}

void CH264DXVA1_ATI::EndDecodeFrame PARGS0()
{	
	// This frame is splited into two fields
	switch(dec_picture->structure)
	{
	case FRAME:
		if(dec_picture->used_for_reference && !dec_picture->frame_mbs_only_flag)
		{
			SetSurfaceInfo ARGS3(dpb.fs_on_view[0][dpb.used_size_on_view[0]-1]->top_field->unique_id,0,0);
			SetSurfaceInfo ARGS3(dpb.fs_on_view[0][dpb.used_size_on_view[0]-1]->bottom_field->unique_id,1,0);
		}
		break;
	case TOP_FIELD:
	case BOTTOM_FIELD:
		if(dpb.fs_on_view[0][dpb.used_size_on_view[0]-1]->is_used==(TOP_FIELD|BOTTOM_FIELD))
			SetSurfaceInfo ARGS3(dpb.fs_on_view[0][dpb.used_size_on_view[0]-1]->frame->unique_id,0,0);
		break;
	}
	//EndFrame(IMGPAR UnCompress_Idx);

	DEBUG_SHOW_HW_INFO("ATI DXVA: Frame %d %d field Return Index %d from compressed buffer\n", IMGPAR m_lFrame_Counter,IMGPAR structure,IMGPAR m_Intra_lCompBufIndex);
	//m_lFrame_Counter++;
}

void CH264DXVA1_ATI::ReleaseDecodeFrame PARGS1(int frame_index)
{
	if(m_pUncompBufQueue)
		m_pUncompBufQueue->PutItem(frame_index);
}

int CH264DXVA1_ATI::build_picture_decode_buffer PARGS0()
{
	int i, j;

	if(!dec_picture)
		return -1;

	if(IMGPAR currentSlice->structure == FRAME || IMGPAR currentSlice->m_pic_combine_status)
	{
		IMGPAR UnCompress_Idx = (int)m_pUncompBufQueue->GetItem();
#if defined(_USE_QUEUE_FOR_DXVA1_)
		WaitForSingleObject(stream_global->hReadyForRender[IMGPAR UnCompress_Idx], INFINITE);
		ResetEvent(stream_global->hReadyForRender[IMGPAR UnCompress_Idx]);
#endif
	}

	EnterCriticalSection( &crit_PICDEC );

	DXVA_BufferDescription *m_pDxvaBufferDescription = IMGPAR m_pDxvaBufferDescription;
	AMVABUFFERINFO         *m_pBufferInfo            = IMGPAR m_pBufferInfo;
	DXVA_PictureParameters_H264* m_patiPictureDecode = (DXVA_PictureParameters_H264*)m_pbPictureParamBuf[IMGPAR m_Intra_lCompBufIndex];
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
	SetSurfaceInfo ARGS3(dec_picture->unique_id,m_patiPictureDecode->bPicStructure==2?1:0,1);

	DEBUG_SHOW_HW_INFO("ATI DXVA: End of build pic buffer \n");

	LeaveCriticalSection( &crit_PICDEC );	

	return 0;
}

int CH264DXVA1_ATI::build_slice_parameter_buffer PARGS0()
{
	int i, j, offset;
	HRESULT hr = S_FALSE;
	LPDXVA_SliceParameter_H264 pSlice = (LPDXVA_SliceParameter_H264)m_pbSliceCtrlBuf[IMGPAR m_Intra_lCompBufIndex];
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
		int nIndex = listXsize[0];
		for(j = 0; j < listXsize[1]; j++)
		{
			//if(m_pSurfaceInfo[listX[1][j]->unique_id].SInfo[1])	//Skip Bottom filed
			//	continue;
			for(i = 0; i < listXsize[0]; i++)
			{
				if(listX[0][i]->unique_id == listX[1][j]->unique_id)
				{
					IMGPAR m_L0L1Map[j] = i;
					break;
				}
			}
			if(i == listXsize[0])	//not found matching index
				IMGPAR m_L0L1Map[j] = nIndex++;
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
	DEBUG_SHOW_HW_INFO("ATI DXVA: End of build slice buffer\n");
	return 0;
}

int CH264DXVA1_ATI::ExecuteBuffers PARGS1(DWORD lInputFlag)
{
	ExecuteBuffersStruct_DXVA1 cur_img;// = new ExecuteBuffersStruct; //need to be change from outside.

	build_picture_decode_buffer ARGS0();
	dec_picture->pic_store_idx = IMGPAR UnCompress_Idx;

	if (IMGPAR currentSlice->structure!=FRAME && IMGPAR currentSlice->m_pic_combine_status)
		cur_img.m_Intra_lCompBufIndex = IMGPAR m_iFirstCompBufIndex;
	else
		cur_img.m_Intra_lCompBufIndex = IMGPAR m_Intra_lCompBufIndex;
	cur_img.UnCompress_Idx = IMGPAR UnCompress_Idx;
	cur_img.m_MVBufStride = m_MVBufStride;
	cur_img.m_PicWidth = m_PicWidth;
	cur_img.m_PicHeight = IMGPAR m_PicHeight;
	cur_img.m_ReBufStride = m_ReBufStride;
	cur_img.m_SBBufOffset = IMGPAR m_SBBufOffset;
	cur_img.m_lpMVBuf = m_pbMotionVectorBuf[cur_img.m_Intra_lCompBufIndex];
	cur_img.m_lpMV = IMGPAR m_lpMV;
	cur_img.m_lpRESDBuf = m_pbResidualDiffBuf[cur_img.m_Intra_lCompBufIndex];
	cur_img.m_lpRESD_Intra_Luma = IMGPAR m_lpRESD_Intra_Luma;
	memcpy(cur_img.m_pDxvaBufferDescription, IMGPAR m_pDxvaBufferDescription, sizeof(DXVA_BufferDescription));
	memcpy(cur_img.m_pBufferInfo, IMGPAR m_pBufferInfo, sizeof(AMVABUFFERINFO));
	cur_img.bPicStructure = ((DXVA_PictureParameters_H264*)m_pbPictureParamBuf[cur_img.m_Intra_lCompBufIndex])->bPicStructure;
	cur_img.smart_dec = IMGPAR smart_dec;
	cur_img.slice_struct = IMGPAR currentSlice->structure;
	cur_img.slice_number = IMGPAR slice_number;
	cur_img.combine_status = IMGPAR currentSlice->m_pic_combine_status;
	cur_img.picture_type = IMGPAR currentSlice->picture_type;
	cur_img.type = IMGPAR currentSlice->picture_type;
	cur_img.Hybrid_Decoding = IMGPAR Hybrid_Decoding;
	cur_img.m_pIviCP = m_pIviCP;
	cur_img.framepoc = IMGPAR framepoc;
	cur_img.m_is_MTMS = stream_global->m_is_MTMS;

	//ResetEvent(stream_global->hReadyForRender[IMGPAR UnCompress_Idx]);
	EnterCriticalSection( &crit_ACCESS_QUEUE );
	m_Queue_DXVA1->push(cur_img);	
	long count;
	ReleaseSemaphore(
		stream_global->m_queue_semaphore, 
		1, 
		&count
		);
	LeaveCriticalSection( &crit_ACCESS_QUEUE );

	return 0;
}

void CH264DXVA1_ATI::DMA_Transfer PARGS0()
{

	HRESULT	hr=0;
	if(IMGPAR Hybrid_Decoding == 5)
	{
		int i;
		imgpel *imgYDXVA;
		int stride = dec_picture->Y_stride;
		int stride_UV = dec_picture->UV_stride;
		int dxva_stride;
		int dxva_offset;	//For field scenario
		int dxva_height = (!IMGPAR structure ? IMGPAR height : IMGPAR height>>1);
		imgpel *imgY;
		imgpel *imgUV;

		imgY = dec_picture->imgY;
		imgUV = dec_picture->imgUV; 
		imgYDXVA = IMGPAR m_pUnCompressedBuffer;

		if(IMGPAR structure) 
		{	//Field
			dxva_stride = (IMGPAR m_UnCompressedBufferStride<<1);
			dxva_offset= IMGPAR m_UnCompressedBufferStride;

			if(IMGPAR structure==2) 
			{	//Bottom Field
				imgYDXVA += dxva_offset;
			}

		} 
		else 
		{	//Frame
			dxva_stride = IMGPAR m_UnCompressedBufferStride;
			dxva_offset = 0;

		}

		for(i=0;i<dxva_height;i++)//Y
		{
			memcpy(imgYDXVA, imgY, IMGPAR width);
			imgYDXVA += dxva_stride;
			imgY += stride;
		}

		for(i=0;i<(dxva_height>>1)-1;i++)//UV 
		{
			memcpy(imgYDXVA, imgUV, IMGPAR width);			
			imgYDXVA += (dxva_offset + IMGPAR m_UnCompressedBufferStride);
			imgUV += stride_UV;
		}		
	}

	dec_picture->pic_store_idx = IMGPAR UnCompress_Idx;

	hr |= ReleaseBuffer(-1, IMGPAR UnCompress_Idx);
	hr |= EndFrame(IMGPAR UnCompress_Idx);

#if defined(_USE_QUEUE_FOR_DXVA1_)
	SetEvent(stream_global->hReadyForRender[IMGPAR UnCompress_Idx]);
#endif

}

unsigned int CH264DXVA1_ATI::ThreadExecute_DXVA1(void * arg)
{
	CH264DXVA1_ATI* pATIdxva = (CH264DXVA1_ATI *)arg;
	ExecuteBuffersStruct_DXVA1 cur_img;
	queue <ExecuteBuffersStruct_DXVA1> *MemQueue = (queue <ExecuteBuffersStruct_DXVA1> *) pATIdxva->m_Queue_DXVA1;
	StreamParameters *stream_global = pATIdxva->stream_global;
	static int nCount = 10;
	static char FrameTypeStringTbl[3][12] = {{'P','F','r','a','m','e',' ','P','O','C',' ','='},
	{'B','F','r','a','m','e',' ','P','O','C',' ','='},
	{'I','F','r','a','m','e',' ','P','O','C',' ','='}};


	HRESULT	hr;
	DWORD m_dwRetValue;

	int t_iDiffTime;
	BOOL t_bDropBottomB;	

	while(1)
	{
		WaitForSingleObject(stream_global->m_queue_semaphore, INFINITE);

		if(pATIdxva->bRunThread == FALSE)
		{
			SetEvent(pATIdxva->hEndCpyThread);
			break;
		}

		if(stream_global->m_dxva_queue_reset) //reset queue
		{	DEBUG_SHOW_SW_INFO("m_dxva_queue_reset");
		stream_global->m_dxva_queue_reset = 0;
		EnterCriticalSection( &(pATIdxva->crit_ACCESS_QUEUE) );
		int size  = MemQueue->size();

		while(size--)
		{
			WaitForSingleObject(stream_global->m_queue_semaphore, 0);
			cur_img = MemQueue->front();
			MemQueue->pop();
			pATIdxva->m_bCompBufStaus[cur_img.m_Intra_lCompBufIndex] = true;
		}
		LeaveCriticalSection( &(pATIdxva->crit_ACCESS_QUEUE) );
		SetEvent(stream_global->h_dxva_queue_reset_done);
		continue;
		}

		EnterCriticalSection( &(pATIdxva->crit_ACCESS_QUEUE) );
		//KevinChien: 0722
		if(MemQueue->size())
		{
			cur_img = MemQueue->front();
			MemQueue->pop();	
			LeaveCriticalSection( &(pATIdxva->crit_ACCESS_QUEUE) );
		}
		else
		{
			DEBUG_SHOW_ERROR_INFO("ExeQueue Size Error");
			LeaveCriticalSection( &(pATIdxva->crit_ACCESS_QUEUE) );
			continue;
		}
		//End: 0722

		DXVA_BufferDescription *m_pDxvaBufferDescription = cur_img.m_pDxvaBufferDescription;
		AMVABUFFERINFO         *m_pBufferInfo            = cur_img.m_pBufferInfo;

#ifdef DUMPDXVA_PICTURE
		fwrite(FrameTypeStringTbl[cur_img.type], 1, 12, m_pDumpFilePicture);
		fwrite(&cur_img.framepoc, 4, 1, m_pDumpFilePicture);
		fwrite((DXVA_PictureParameters_H264*)m_PictureDecode[cur_img.m_Intra_lCompBufIndex], sizeof(DXVA_PictureParameters_H264), 1,  m_pDumpFilePicture);		
#endif

#ifdef use_tmp
		// For DXVA Data Dump Only
#ifdef DUMPDXVA_MV
		fwrite(FrameTypeStringTbl[cur_img.type], 1, 12, m_pDumpFileMV);
		fwrite(&cur_img.framepoc, 4, 1, m_pDumpFileMV);
		fwrite(cur_img.m_lpMV, 4, cur_img.m_MVBufStride*cur_img.m_PicHeight/8,  m_pDumpFileMV);
#endif
		///////////////////

		t_bDropBottomB = FALSE;

		//debug_value =  *(int*)(cur_img.m_lpMV + cur_img.m_MVBufStride*cur_img.m_PicHeight/2);

		if(cur_img.type == B_SLICE) {
			memcpy(cur_img.m_lpMVBuf, cur_img.m_lpMV, cur_img.m_MVBufStride*cur_img.m_PicHeight/2);
		} else if(cur_img.type == P_SLICE) {
			memcpy(cur_img.m_lpMVBuf+cur_img.m_MVBufStride*cur_img.m_PicHeight/4, cur_img.m_lpMV+cur_img.m_MVBufStride*cur_img.m_PicHeight/4, cur_img.m_MVBufStride*cur_img.m_PicHeight/4);
		}
#endif

#ifdef DUMPDXVA_RESIDUE
		fwrite(FrameTypeStringTbl[cur_img.type], 1, 12, m_pDumpFileResidue);
		fwrite(&cur_img.framepoc, 4, 1, m_pDumpFileResidue);
		//fwrite(cur_img.m_lpRESD_Intra_Luma, 1, cur_img.m_ReBufStride*cur_img.m_PicHeight*3/2,  m_pDumpFileResidue);
		fwrite(cur_img.m_lpRESDBuf, 1, cur_img.m_ReBufStride*cur_img.m_PicHeight*3/2,  m_pDumpFileResidue);
#endif


		//Dump ARGS3((byte*)m_PictureDecode[IMGPAR m_Intra_lCompBufIndex], m_pBufferInfo[0].dwDataSize, "dxva_Pic");

		//fill out buffer description table for DXVA_PICTURE_DECODE_BUFFER
		memset(&m_pDxvaBufferDescription[0], 0, sizeof(DXVA_BufferDescription));
		m_pDxvaBufferDescription[0].dwTypeIndex		= m_pBufferInfo[0].dwTypeIndex	= DXVA_PICTURE_DECODE_BUFFER;
		m_pDxvaBufferDescription[0].dwBufferIndex	= m_pBufferInfo[0].dwBufferIndex= cur_img.m_Intra_lCompBufIndex;
		m_pDxvaBufferDescription[0].dwDataSize		= m_pBufferInfo[0].dwDataSize	= sizeof(DXVA_PictureParameters_H264);

		memset(&m_pDxvaBufferDescription[1], 0, 4*sizeof(DXVA_BufferDescription));
		// Macroblock control buffer description
		m_pDxvaBufferDescription[1].dwTypeIndex			= m_pBufferInfo[1].dwTypeIndex	= DXVA_MACROBLOCK_CONTROL_BUFFER;
		m_pDxvaBufferDescription[1].dwBufferIndex		= m_pBufferInfo[1].dwBufferIndex= cur_img.m_Intra_lCompBufIndex;
		m_pDxvaBufferDescription[1].dwDataSize			= m_pBufferInfo[1].dwDataSize	= cur_img.m_SBBufOffset+cur_img.m_PicWidth*cur_img.m_PicHeight/8;
		//m_pDxvaBufferDescription[1].dwFirstMBaddress	= 0;
		m_pDxvaBufferDescription[1].dwNumMBsInBuffer	= cur_img.m_PicWidth*cur_img.m_PicHeight/256;
		//m_pDxvaBufferDescription[1].dwStride			= m_MbBufStride;
		//Dump ARGS3((byte*)m_lpMBLKBuf[IMGPAR m_Intra_lCompBufIndex], m_pBufferInfo[1].dwDataSize, "dxva_MB");

		// Residual data buffer description
		m_pDxvaBufferDescription[2].dwTypeIndex			= m_pBufferInfo[2].dwTypeIndex	= DXVA_RESIDUAL_DIFFERENCE_BUFFER;
		m_pDxvaBufferDescription[2].dwBufferIndex		= m_pBufferInfo[2].dwBufferIndex= cur_img.m_Intra_lCompBufIndex;
		m_pDxvaBufferDescription[2].dwDataSize			= m_pBufferInfo[2].dwDataSize	= cur_img.m_ReBufStride*cur_img.m_PicHeight*3/2;
		//m_pDxvaBufferDescription[2].dwFirstMBaddress	= 0;
		m_pDxvaBufferDescription[2].dwNumMBsInBuffer	= cur_img.m_PicWidth*cur_img.m_PicHeight/256;
		m_pDxvaBufferDescription[2].dwHeight			= cur_img.m_PicHeight;	
		m_pDxvaBufferDescription[2].dwStride			= cur_img.m_PicWidth*2;//m_ReBufStride;
		//Dump ARGS3((byte*)m_lpRESDBuf[IMGPAR m_Intra_lCompBufIndex], m_pBufferInfo[2].dwDataSize, "dxva_dct");

		// MV description
		m_pDxvaBufferDescription[3].dwTypeIndex			= m_pBufferInfo[3].dwTypeIndex	= DXVA_MOTION_VECTOR_BUFFER_264EXT;
		m_pDxvaBufferDescription[3].dwBufferIndex		= m_pBufferInfo[3].dwBufferIndex= cur_img.m_Intra_lCompBufIndex;
		m_pDxvaBufferDescription[3].dwDataSize			= m_pBufferInfo[3].dwDataSize	= cur_img.m_MVBufStride*cur_img.m_PicHeight/2;
		//m_pDxvaBufferDescription[3].dwFirstMBaddress	= 0;
		m_pDxvaBufferDescription[3].dwNumMBsInBuffer	= cur_img.m_PicWidth*cur_img.m_PicHeight/256;
		m_pDxvaBufferDescription[3].dwStride			= cur_img.m_MVBufStride;
		//Dump ARGS3((byte*)m_lpMVBuf[IMGPAR m_Intra_lCompBufIndex], m_pBufferInfo[3].dwDataSize, "dxva_MVector");

		//Slice Parameter buffer
		m_pDxvaBufferDescription[4].dwTypeIndex			= m_pBufferInfo[4].dwTypeIndex	= DXVA_SLICE_PARAMETER_BUFFER_264EXT;
		m_pDxvaBufferDescription[4].dwBufferIndex		= m_pBufferInfo[4].dwBufferIndex= cur_img.m_Intra_lCompBufIndex;
		m_pDxvaBufferDescription[4].dwDataSize			= m_pBufferInfo[4].dwDataSize	= sizeof(DXVA_SliceParameter_H264);
		//m_pDxvaBufferDescription[4].dwFirstMBaddress	= 0;
		m_pDxvaBufferDescription[4].dwNumMBsInBuffer	= cur_img.m_PicWidth*cur_img.m_PicHeight/256;
		//Dump ARGS3((byte*)m_lpSLICEBuf[IMGPAR m_Intra_lCompBufIndex], m_pBufferInfo[4].dwDataSize, "dxva_SliceParam");

		if (g_framerate1000<30000)
		{
			if (nCount<0 && (cur_img.slice_struct==FRAME || cur_img.combine_status))
			{
				if (pATIdxva->m_pUncompBufQueue->count > 8) //Display Queue <= 3
					WaitForSingleObject(stream_global->hFinishDisplaySemaphore, (30*g_PulldownRate)/1000);
				else
					WaitForSingleObject(stream_global->hFinishDisplaySemaphore, (50*g_PulldownRate)/1000);

				QueryPerformanceCounter(&stream_global->m_liRecodeTime1);
				t_iDiffTime = (stream_global->m_iEstimateNextFrame - ((int)(1000 * (stream_global->m_liRecodeTime1.QuadPart - stream_global->m_liRecodeTime0.QuadPart))/pATIdxva->m_pUncompBufQueue->m_freq.QuadPart));
				DEBUG_SHOW_HW_INFO("Diff Time: %d, count: %d queue size: %d", t_iDiffTime, pATIdxva->m_pUncompBufQueue->count, MemQueue->size());

				if (t_iDiffTime > 50)
					nCount++;
				else if (t_iDiffTime < 10)
				{
					if (pATIdxva->m_pUncompBufQueue->count > 8) //Display Queue <= 3
						WaitForSingleObject(stream_global->hFinishDisplaySemaphore, (30*g_PulldownRate)/1000);
					else
						WaitForSingleObject(stream_global->hFinishDisplaySemaphore, (50*g_PulldownRate)/1000);
				}
			}
			else
				nCount--;
		}

		hr = pATIdxva->BeginFrame(cur_img.UnCompress_Idx, 0);
		hr = pATIdxva->Execute(0x01000000, &m_pDxvaBufferDescription[0], 5*sizeof(DXVA_BufferDescription), &m_dwRetValue, 4, 5, &m_pBufferInfo[0]);
		pATIdxva->EndFrame(cur_img.UnCompress_Idx);

		if((cur_img.smart_dec & SMART_DEC_SKIP_FIL_B) && cur_img.type == B_SLICE && cur_img.slice_struct != FRAME)
		{
			t_bDropBottomB = TRUE;

			((DXVA_PictureParameters_H264*)pATIdxva->m_pbPictureParamBuf[cur_img.m_Intra_lCompBufIndex])->bPicStructure = (cur_img.bPicStructure == 1 ? 2 :1);

			hr = pATIdxva->BeginFrame(cur_img.UnCompress_Idx, 0);
			hr = pATIdxva->Execute(0x01000000, &m_pDxvaBufferDescription[0], 5*sizeof(DXVA_BufferDescription), &m_dwRetValue, 4, 5, &m_pBufferInfo[0]);			
			pATIdxva->EndFrame(cur_img.UnCompress_Idx);
		}

		pATIdxva->m_bCompBufStaus[cur_img.m_Intra_lCompBufIndex] = true;

		if (cur_img.slice_struct==FRAME || cur_img.combine_status==0 || t_bDropBottomB)
			SetEvent(stream_global->hReadyForRender[cur_img.UnCompress_Idx]);
	}
	return 0;
}

void CH264DXVA1_ATI::TransferData_at_SliceEnd PARGS0() {

	if ( ((IMGPAR Hybrid_Decoding==2) && (IMGPAR type == B_SLICE)) ||
		(IMGPAR Hybrid_Decoding==0) || 
		((IMGPAR Hybrid_Decoding==1) && (IMGPAR type != I_SLICE)) )
	{
		if (!((IMGPAR smart_dec & SMART_DEC_SKIP_FIL_B) && IMGPAR currentSlice->picture_type == B_SLICE && IMGPAR currentSlice->structure != FRAME && IMGPAR currentSlice->m_pic_combine_status == 0 && !IMGPAR nal_reference_idc))
		{					
			StoreImgRowToDXVACompressedBuffer ARGS2(IMGPAR start_mb_x, IMGPAR mb_x_d);					
		}
	} else {
		StoreImgRowToImgPic ARGS2(IMGPAR start_mb_x, IMGPAR mb_x_d);
	}
}

void CH264DXVA1_ATI::StoreImgRowToImgPic PARGS2(int start_x, int end_x) 
{

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
