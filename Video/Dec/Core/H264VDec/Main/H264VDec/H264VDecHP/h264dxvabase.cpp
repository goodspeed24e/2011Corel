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

#include "h264dxvabase.h"
#include "h264dxva1.h"
#include "h264dxva2.h"
#include "mmintrin.h"
#include "xmmintrin.h"
#include "emmintrin.h"
#include "image.h"

#define _MMX_IDCT
#define DQ_SHIFT 6
#define DQ_ADD   32
#define Transpose(mm0,mm1,mm2,mm3)  {	\
	\
	tmp0	= _mm_unpackhi_pi16(mm0,mm1);	\
	tmp2	= _mm_unpacklo_pi16(mm0,mm1);	\
	tmp1	= _mm_unpackhi_pi16(mm2,mm3);	\
	tmp3	= _mm_unpacklo_pi16(mm2,mm3);	\
	\
	(mm0)	= _mm_unpacklo_pi32(tmp2,tmp3);	\
	(mm1)	= _mm_unpackhi_pi32(tmp2,tmp3);	\
	(mm2)	= _mm_unpacklo_pi32(tmp0,tmp1);	\
	(mm3)	= _mm_unpackhi_pi32(tmp0,tmp1);	\
}

void iDCT_4x4_2_ATI(short* dest, short *src, short dest_stride)
{	
	__m128i mm0,  mm1,  mm2,  mm3;
	__m128i tmp0, tmp1, tmp2, tmp3;

	mm0 = _mm_load_si128((__m128i*)&src[0]);
	mm1 = _mm_load_si128((__m128i*)&src[8]);
	mm2 = _mm_load_si128((__m128i*)&src[16]);
	mm3 = _mm_load_si128((__m128i*)&src[24]);

	memset(src, 0, 32*sizeof(short));

	tmp0=_mm_unpacklo_epi64(mm0,mm2);	//m0 m0'
	tmp2=_mm_unpacklo_epi64(mm1,mm3);	//m2 m2'
	tmp1=_mm_unpackhi_epi64(mm0,mm2);	//m1 m1'
	tmp3=_mm_unpackhi_epi64(mm1,mm3);	//m3 m3'

	// horizontal
	mm0 = _mm_adds_epi16(tmp0,tmp2);	//m0+m2
	mm2 = _mm_subs_epi16(_mm_srai_epi16(tmp1,1),tmp3); //  1/2m1-m3
	mm1 = _mm_subs_epi16(tmp0,tmp2);	//m0-m2
	mm3 = _mm_adds_epi16(tmp1,_mm_srai_epi16(tmp3,1)); //  m1+1/2m3

	tmp0 = _mm_adds_epi16(mm0,mm3);		//m0+m1+m2+1/2m3 11115555
	tmp1 = _mm_adds_epi16(mm1,mm2);		//m0+1/2m1-m2-m3 22226666	
	tmp3 = _mm_subs_epi16(mm0,mm3);     //m0-m1+m2-1/2m3 44448888
	tmp2 = _mm_subs_epi16(mm1,mm2);		//m0-1/2m1-m2+m3 33337777

	//transpose
	mm0 =_mm_unpacklo_epi16(tmp0,tmp1);	// 12121212
	mm2 =_mm_unpacklo_epi16(tmp2,tmp3); // 34343434
	mm1 =_mm_unpackhi_epi16(tmp0,tmp1);	// 56565656 
	mm3 =_mm_unpackhi_epi16(tmp2,tmp3);	// 78787878

	tmp0 =_mm_unpacklo_epi32(mm0,mm2);	// 12341234
	tmp2 =_mm_unpacklo_epi32(mm1,mm3);  // 56785678
	tmp1 =_mm_unpackhi_epi32(mm0,mm2);	// 12341234 
	tmp3 =_mm_unpackhi_epi32(mm1,mm3);	// 56785678

	mm0 =_mm_unpacklo_epi64(tmp0,tmp2);	// 12345678
	mm2 =_mm_unpacklo_epi64(tmp1,tmp3); // 12345678
	mm1 =_mm_unpackhi_epi64(tmp0,tmp2);	// 12345678 
	mm3 =_mm_unpackhi_epi64(tmp1,tmp3);	// 12345678

	//vertical
	tmp0 = _mm_adds_epi16(mm0,mm2);
	tmp2 = _mm_subs_epi16(_mm_srai_epi16(mm1,1),mm3);
	tmp1 = _mm_subs_epi16(mm0,mm2);
	tmp3 = _mm_adds_epi16(mm1,_mm_srai_epi16(mm3,1));


	mm0 = _mm_adds_epi16(tmp0,tmp3);
	mm1 = _mm_adds_epi16(tmp1,tmp2);
	mm3 = _mm_subs_epi16(tmp0,tmp3);
	mm2 = _mm_subs_epi16(tmp1,tmp2);

	_mm_storeu_si128(((__m128i*) dest), mm0);
	_mm_storeu_si128(((__m128i*)(dest+(dest_stride ))), mm1);
	_mm_storeu_si128(((__m128i*)(dest+(dest_stride << 1))), mm2);
	_mm_storeu_si128(((__m128i*)(dest+(dest_stride << 1)+(dest_stride ))), mm3);

}

int CUncompressBufferQueue::GetItem()
{
	HANDLE handle[2] = { mutex, semaphore_get };
	int item;

#if defined (_SHOW_THREAD_TIME_)
	LARGE_INTEGER liStartTime, liEndTime;
	QueryPerformanceCounter(&liStartTime);
#endif

	DP_QUEUE("Wait for mutex & get, count: %d\n", count);
	WaitForMultipleObjects(2,handle,TRUE,INFINITE);

#if defined (_SHOW_THREAD_TIME_)
	QueryPerformanceCounter(&liEndTime);
	DP("GetItem() Time: %I64d, count: %d", (1000 * (liEndTime.QuadPart - liStartTime.QuadPart) / m_freq.QuadPart), count);
#endif

	item = queue[idx_get];
	if(++idx_get==max)
		idx_get = 0;
	count--;
	if(init_status==2)
	{
		init_status = 0;
		DP_QUEUE("Init status change (%d)\n", init_status);
	}
	DP_QUEUE("Release mutex & put, count: %d, item %d\n", count, item);
	ReleaseSemaphore(semaphore_put,1,NULL);
	ReleaseMutex(mutex);

	DP_QUEUE("[UnCompressedQueue] GetItem(): %d", item);

	return item;
}

int CUncompressBufferQueue::PeekItem(unsigned long offset)
{
	unsigned long idx;
	int result;

	DP_QUEUE(("Wait for mutex\n"));
	WaitForSingleObject(mutex,INFINITE);
	if(offset>=count)
	{
		ReleaseMutex(mutex);
		return 0;
	}
	idx = idx_get+offset;
	if(idx>=max)
		idx -= max;
	result = queue[idx];
	DP_QUEUE(("Release mutex\n"));
	ReleaseMutex(mutex);

	return result;
}

void CUncompressBufferQueue::PutItem(int item)
{
	HANDLE handle[2] = { mutex, semaphore_put };

	if(count==max)
		return;

	if(init_status)
	{
		if(init_status>1)
		{
			DP_QUEUE("Init status (%d) return\n", init_status);
			return;
		}
		else if(init_status==1 && item!=count)
		{
			DP_QUEUE("Init status (%d) return \n", init_status);
			return;
		}
	}

	DP_QUEUE("[UnCompressedQueue] PutItem(): %d", item);

#if defined (_SHOW_THREAD_TIME_)
	LARGE_INTEGER liStartTime, liEndTime;
	QueryPerformanceCounter(&liStartTime);
#endif

	DP_QUEUE("Wait for mutex & put, count: %d, item %d\n", count, item);
	WaitForMultipleObjects(2,handle,TRUE,INFINITE);

#if defined (_SHOW_THREAD_TIME_)
	QueryPerformanceCounter(&liEndTime);
	DP("PutItem() Time: %I64d, count: %d", (1000 * (liEndTime.QuadPart - liStartTime.QuadPart) / m_freq.QuadPart), count);
#endif

	queue[idx_put] = item;
	if(++idx_put==max)
		idx_put = 0;
	count++;
	if(init_status && count==max)
	{
		init_status = 2;
		DP_QUEUE("Init status change (%d)\n", init_status);
	}
	DP_QUEUE("Release mutex & get, count: %d\n", count);
	ReleaseSemaphore(semaphore_get,1,NULL);
	ReleaseMutex(mutex);
}

//////////////////////////////////////////////////////////////////////////
// CH264DXVABase implementation
CH264DXVABase::CH264DXVABase()
{
	InitializeCriticalSection(&m_cs);

	m_nSurfaceFrame = 0;
	m_nFrameCounter = 0;

	m_pIHVDService = NULL;
	m_pIHVDServiceDxva = NULL;
	m_nVGAType = -1;
	m_nDXVAMode = -1;

	m_bConfigBitstreamRaw = -1;
	m_bResolutionChange = FALSE;

	memset(m_RefList, 0xFF, 20);
	memset(m_RefInfo, 0, sizeof(MSBA_REF_INFO)*20);

	m_pSurfaceInfo = NULL;
	m_pUncompBufQueue = NULL;

	memset(m_bCompBufStaus, 0, sizeof(BYTE)*MAX_COMP_BUF);
	m_nLastCompBufIdx = -1;

	m_lpPictureParamBuf = NULL;
	m_lpMacroblockCtrlBuf = NULL;
	m_lpResidualDiffBuf = NULL;
	m_lpDeblockingCtrlBuf = NULL;
	m_lpInverseQuantMatrixBuf = NULL;
	m_lpSliceCtrlBuf = NULL;
	m_lpBitstreamBuf = NULL;
	m_lpMotionVectorBuf = NULL;

	m_lpnalu = NULL;
	m_nNALUSkipByte = 0;

	m_pIviCP = NULL;

	//////////////////////////////////////////////////////////////////////////
	iDCT_4x4_fcn = NULL;
	iDCT_8x8_fcn = NULL;
	iDCT_4x4_UV_fcn = NULL;
}

CH264DXVABase::~CH264DXVABase()
{
	DeleteCriticalSection(&m_cs);

}

void CH264DXVABase::SetIHVDService(IHVDService *pIHVDService, H264VDecHP_OpenOptionsEx *pOptions)
{
	m_pIHVDService = pIHVDService;
	m_pIHVDService->QueryInterface(__uuidof(IHVDServiceDxva), (void**)&m_pIHVDServiceDxva);

	HVDService::HVDDXVAConfigPictureDecode DxvaConfigPictureDecode;
	m_pIHVDServiceDxva->GetDxvaConfigPictureDecode(&DxvaConfigPictureDecode);

	m_nVGAType = pOptions->uiH264VGACard;
	m_nDXVAMode = pOptions->uiH264DXVAMode;

	m_bConfigBitstreamRaw = DxvaConfigPictureDecode.bConfigBitstreamRaw;
	m_guidConfigBitstreamEncryption = DxvaConfigPictureDecode.guidConfigBitstreamEncryption;

	m_pIviCP = pOptions->pIviCP;

}

int CH264DXVABase::Open PARGS1(int nSurfaceFrame)
{
	EnterCriticalSection(&m_cs);

	m_nSurfaceFrame = nSurfaceFrame;
	m_nFrameCounter = 0;

	m_pSurfaceInfo = new SURFACE_INFO[128];
	InitialUnCompressBufferQueue();

	if (m_nDXVAMode != E_H264_DXVA_MODE_E && m_nDXVAMode != E_H264_DXVA_MODE_F && m_nDXVAMode != E_H264_DXVA_INTEL_MODE_E) //MC and IT mode
	{
		if (m_nDXVAMode==E_H264_DXVA_ATI_PROPRIETARY_A) 
		{
			iDCT_4x4_fcn = iDCT_4x4_ATI;
			iDCT_8x8_fcn = iDCT_8x8_ATI;
			iDCT_4x4_UV_fcn = iDCT_4x4_UV_ATI;
		} 
		else 
		{
			iDCT_4x4_fcn = iDCT_4x4_nonATI;
			iDCT_8x8_fcn = iDCT_8x8_nonATI;
			iDCT_4x4_UV_fcn = iDCT_4x4_UV_nonATI;
		}
	}

	m_bResolutionChange = TRUE;

	LeaveCriticalSection(&m_cs);
	return 0;
}

int CH264DXVABase::Close PARGS0()
{
	if(m_pUncompBufQueue)
	{
		delete m_pUncompBufQueue;
		m_pUncompBufQueue = NULL;
	}

	if (m_pSurfaceInfo)
	{
		delete [] m_pSurfaceInfo;
		m_pSurfaceInfo = NULL;
	}

	if (m_pIHVDServiceDxva)
		m_pIHVDServiceDxva->Release();
	return 0;
}

void CH264DXVABase::ResetDXVABuffers()
{
	memset(m_RefList, 0xFF, 20);
	memset(m_RefInfo, 0, sizeof(MSBA_REF_INFO)*20);

	HVDService::HVDDXVAConfigPictureDecode DxvaConfigPictureDecode;
	m_pIHVDServiceDxva->GetDxvaConfigPictureDecode(&DxvaConfigPictureDecode);

	m_bConfigBitstreamRaw = DxvaConfigPictureDecode.bConfigBitstreamRaw;
	m_guidConfigBitstreamEncryption = DxvaConfigPictureDecode.guidConfigBitstreamEncryption;

}

void CH264DXVABase::BeginDecodeFrame PARGS0()
{

}

void CH264DXVABase::EndDecodeFrame PARGS0()
{

}

void CH264DXVABase::ReleaseDecodeFrame PARGS1(int nFrameIndex)
{
	if(m_pUncompBufQueue)
		m_pUncompBufQueue->PutItem(nFrameIndex);
}

void CH264DXVABase::InitialUnCompressBufferQueue()
{
	if(m_pUncompBufQueue)
	{
		delete m_pUncompBufQueue;
		m_pUncompBufQueue = NULL;
	}

	m_pUncompBufQueue = new CUncompressBufferQueue(m_nSurfaceFrame);
	for (int i=0; i<m_nSurfaceFrame; i++)
		m_pUncompBufQueue->PutItem(i);
}

void CH264DXVABase::SetSurfaceInfo PARGS3(BYTE id, BYTE flag, BYTE overwrite)
{
	if (!m_pSurfaceInfo)
		return;

	if(m_pSurfaceInfo[id].SInfo[0] != 0xFF && overwrite)
		m_pSurfaceInfo[id].Set();

	m_pSurfaceInfo[id].SInfo[0] = IMGPAR UnCompress_Idx;
	m_pSurfaceInfo[id].SInfo[1] = flag;
}

CREL_RETURN HW_DecodeOnePicture PARGS2(int *header, BOOL bSkip)
{
	CH264DXVABase *pH264DXVA = (CH264DXVABase*)((IMGPAR stream_global)->m_pH264DXVA);
	stream_par *stream_global = IMGPAR stream_global;
	int ret = 0;

	if (!g_has_pts && nalu_global->pts.ts)
	{
		g_pts = nalu_global->pts;
		g_has_pts = 1;
	}

	if (pH264DXVA)
	{
		if(pH264DXVA->GetBitstreamRawConfig()==E_BA_RAW_SHORTFORMAT)
			ret = pH264DXVA->decode_one_picture_short ARGS2(header, bSkip);
		else
			ret = pH264DXVA->decode_one_picture_long ARGS2(header, bSkip);
	}

	return ret;
}

//////////////////////////////////////////////////////////////////////////
// HW related function
void* HW_Open PARGS4(int nframe, int iVGAType, int iDXVAMode, ICPService *m_pIviCP)
{
	stream_par *stream_global = IMGPAR stream_global;

	g_pH264DXVA = NULL;
	if (IviDxva1 == g_DXVAVer)
	{
		if (iVGAType == E_H264_VGACARD_ATI)
		{
			g_pH264DXVA = (CH264DXVABase*)(new CH264DXVA1_ATI());
		}
		else if (iVGAType == E_H264_VGACARD_NVIDIA)
		{
			g_pH264DXVA = (CH264DXVABase*)(new CH264DXVA1_NV());
		}
		else if (iVGAType == E_H264_VGACARD_S3)
		{
			g_pH264DXVA = (CH264DXVABase*)(new CH264DXVA1_NV());
		}
	}
	else if (IviDxva2 == g_DXVAVer)
	{
		if (iVGAType == E_H264_VGACARD_ATI)
		{
			g_pH264DXVA = (CH264DXVABase*)(new CH264DXVA2_ATI());
		}
		else //General VGA
		{
			g_pH264DXVA = (CH264DXVABase*)(new CH264DXVA2());
		}
	}

	return NULL;
}

CREL_RETURN HW_Close PARGS0()
{
	CH264DXVABase *pH264DXVA = (CH264DXVABase*)((IMGPAR stream_global)->m_pH264DXVA);

	if (pH264DXVA)
	{
		pH264DXVA->Close ARGS0();
		delete pH264DXVA;
		(IMGPAR stream_global)->m_pH264DXVA = NULL;
	}

	return CREL_OK;
}

CREL_RETURN HW_BeginDecodeFrame PARGS0()
{
	CH264DXVABase *pH264DXVA = (CH264DXVABase*)((IMGPAR stream_global)->m_pH264DXVA);

	if(pH264DXVA)	
		pH264DXVA->BeginDecodeFrame ARGS0();

	return CREL_OK;
}

CREL_RETURN HW_EndDecodeFrame PARGS0()
{
	CH264DXVABase *pH264DXVA = (CH264DXVABase*)((IMGPAR stream_global)->m_pH264DXVA);

	if(pH264DXVA)	
		pH264DXVA->EndDecodeFrame ARGS0();

	return CREL_OK;
}

CREL_RETURN HW_ReleaseDecodeFrame PARGS1(int frame_index)
{
	CH264DXVABase *pH264DXVA = (CH264DXVABase*)((IMGPAR stream_global)->m_pH264DXVA);

	if(pH264DXVA)			
		pH264DXVA->ReleaseDecodeFrame ARGS1(frame_index);

	return CREL_OK;
}

CREL_RETURN HW_decode_one_macroblock_Intra PARGS0()
{
	CH264DXVABase *pH264DXVA = (CH264DXVABase*)((IMGPAR stream_global)->m_pH264DXVA);

	if (pH264DXVA)
		pH264DXVA->decode_one_macroblock_Intra ARGS0();

	return CREL_OK;	
}

CREL_RETURN HW_decode_one_macroblock_Inter PARGS0()
{
	CH264DXVABase *pH264DXVA = (CH264DXVABase*)((IMGPAR stream_global)->m_pH264DXVA);

	if (pH264DXVA)
		pH264DXVA->decode_one_macroblock_Inter ARGS0();

	return CREL_OK;
}

CREL_RETURN HW_StoreImgRowToImgPic PARGS2(int start_x, int end_x) {

	CH264DXVABase *pH264DXVA = (CH264DXVABase*)((IMGPAR stream_global)->m_pH264DXVA);

	if(pH264DXVA) 
	{
		if( (IviDxva1==(IMGPAR stream_global)->lDXVAVer && pH264DXVA->GetDXVAMode()==E_H264_DXVA_ATI_PROPRIETARY_A) ||
			(IviDxva2==(IMGPAR stream_global)->lDXVAVer))
			pH264DXVA->StoreImgRowToImgPic ARGS2(start_x, end_x);
		else 
			StoreImgRowToImgPic ARGS2(start_x, end_x);
	}

	return CREL_OK;
}

CREL_RETURN HW_TransferData_at_SliceEnd PARGS0() {

	CH264DXVABase *pH264DXVA = (CH264DXVABase*)((IMGPAR stream_global)->m_pH264DXVA);

	if(pH264DXVA) 
	{
		if( (IviDxva1==(IMGPAR stream_global)->lDXVAVer && pH264DXVA->GetDXVAMode()==E_H264_DXVA_ATI_PROPRIETARY_A) ||
			(IviDxva2==(IMGPAR stream_global)->lDXVAVer))
			pH264DXVA->TransferData_at_SliceEnd ARGS0();		
		else
			TransferData_at_SliceEnd ARGS0();
	}

	return CREL_OK;
}

void EXECUTE PARGS0()
{
	CH264DXVABase *pH264DXVA = (CH264DXVABase*)((IMGPAR stream_global)->m_pH264DXVA);

	if (pH264DXVA)
	{
		if (IviDxva1==(IMGPAR stream_global)->lDXVAVer)
		{
			if (pH264DXVA->GetDXVAMode()==E_H264_DXVA_NVIDIA_PROPRIETARY_A)
				pH264DXVA->ExecuteBuffers ARGS1(E_USING_DEBLOCK);
			else if (pH264DXVA->GetDXVAMode()==E_H264_DXVA_ATI_PROPRIETARY_A)
				pH264DXVA->ExecuteBuffers ARGS1(E_OUTPUT_INTRA_MBS);
		}
		else if (IviDxva2==(IMGPAR stream_global)->lDXVAVer)
			pH264DXVA->ExecuteBuffers ARGS1(0);
	}
}

void DMA_Transfer PARGS0()
{
	CH264DXVABase *pH264DXVA = (CH264DXVABase*)((IMGPAR stream_global)->m_pH264DXVA);

	if (pH264DXVA)	
		pH264DXVA->DMA_Transfer ARGS0();
}

void HW_DeblockSlice PARGS3(StorablePicture *p, int start_mb, int num_mb)
{
	//CH264DXVABase *pH264DXVA = (CH264DXVABase*)((IMGPAR stream_global)->m_pH264DXVA);

	//if(pH264DXVA)			
	//	pH264DXVA->DeblockSlice ARGS3(p, start_mb, num_mb);
}

#if !defined(_MMX_IDCT)
void CH264dxva::iDCT_4x4(short dest[4][4], short *src, BYTE shift_off)
{
	int m6[4];
	int m7[16];
	int * mm;
	int * mm_stop;
	int i;

	// horizontal
	m6[0] = src[0]+src[2];
	m6[1] = src[0]-src[2];
	m6[2] = (src[1]>>1)-src[3];
	m6[3] = src[1]+(src[3]>>1);
	m7[0] = m6[0]+m6[3];
	m7[12] = m6[0]-m6[3];
	m7[4] = m6[1]+m6[2];
	m7[8] = m6[1]-m6[2];

	m6[0] = src[4]+src[6];
	m6[1] = src[4]-src[6];
	m6[2] = (src[5]>>1)-src[7];
	m6[3] = src[5]+(src[7]>>1);
	m7[1] = m6[0]+m6[3];
	m7[13] = m6[0]-m6[3];
	m7[5] = m6[1]+m6[2];
	m7[9] = m6[1]-m6[2];

	m6[0] = src[8]+src[10];
	m6[1] = src[8]-src[10];
	m6[2] = (src[9]>>1)-src[11];
	m6[3] = src[9]+(src[11]>>1);
	m7[2] = m6[0]+m6[3];
	m7[14] = m6[0]-m6[3];
	m7[6] = m6[1]+m6[2];
	m7[10] = m6[1]-m6[2];

	m6[0] = src[12]+src[14];
	m6[1] = src[12]-src[14];
	m6[2] = (src[13]>>1)-src[15];
	m6[3] = src[13]+(src[15]>>1);
	m7[3] = m6[0]+m6[3];
	m7[15] = m6[0]-m6[3];
	m7[7] = m6[1]+m6[2];
	m7[11] = m6[1]-m6[2];

	memset(src, 0, 16*sizeof(short));

	// vertical
	for (i = 0, mm = m7, mm_stop = m7 + 16; mm < mm_stop; mm+=4)
	{
		m6[0] = mm[0]+mm[2];
		m6[1] = mm[0]-mm[2];
		m6[2] = (mm[1]>>1)-mm[3];
		m6[3] = mm[1]+(mm[3]>>1);

		dest[0][i] = ((m6[0]+m6[3]+DQ_ADD)>>DQ_SHIFT);
		dest[1][i] = ((m6[1]+m6[2]+DQ_ADD)>>DQ_SHIFT);
		dest[2][i] = ((m6[1]-m6[2]+DQ_ADD)>>DQ_SHIFT);
		dest[3][i] = ((m6[0]-m6[3]+DQ_ADD)>>DQ_SHIFT);

		i++;
	}
}

void CH264dxva::iDCT_8x8(short dest[8][8], short *src,BYTE shift_off)
{
	int i;
	short m7[64];
	short * mm, * mm_stop;

	// horizontal
	short a[8], b[8];
	for(i=0, mm = m7; i < 8; i++,src+=8, mm++)
	{
		a[0] =  src[0]+src[4];
		a[4] =  src[0]-src[4];
		a[2] = (src[2]>>1)-src[6];
		a[6] =  src[2]+(src[6]>>1);
		a[1] = -src[3]+src[5]-src[7]-(src[7]>>1);
		a[3] =  src[1]+src[7]-src[3]-(src[3]>>1);
		a[5] = -src[1]+src[7]+src[5]+(src[5]>>1);
		a[7] =  src[3]+src[5]+src[1]+(src[1]>>1);

		b[0] = a[0] + a[6];
		b[2] = a[4] + a[2];
		b[4] = a[4] - a[2];
		b[6] = a[0] - a[6];
		b[1] = a[1] + (a[7]>>2);
		b[7] = -(a[1]>>2) + a[7];
		b[3] = a[3] + (a[5]>>2);
		b[5] = (a[3]>>2) - a[5];

		mm[0]  = b[0] + b[7];
		mm[8]  = b[2] + b[5];
		mm[16] = b[4] + b[3];
		mm[24] = b[6] + b[1];
		mm[32] = b[6] - b[1];
		mm[40] = b[4] - b[3];
		mm[48] = b[2] - b[5];
		mm[56] = b[0] - b[7];
	}

	memset(src-64, 0, 8*8*sizeof(short));

	// vertical
	for(i = 0, mm = m7, mm_stop = m7 + 64; mm < mm_stop; mm += 8)
	{
		a[0] =  mm[0] + mm[4];
		a[4] =  mm[0] - mm[4];
		a[2] = (mm[2]>>1) - mm[6];
		a[6] =  mm[2] + (mm[6]>>1);
		a[1] = -mm[3] + mm[5] - mm[7] - (mm[7]>>1);
		a[3] =  mm[1] + mm[7] - mm[3] - (mm[3]>>1);
		a[5] = -mm[1] + mm[7] + mm[5] + (mm[5]>>1);
		a[7] =  mm[3] + mm[5] + mm[1] + (mm[1]>>1);

		b[0] = a[0] + a[6];
		b[2] = a[4] + a[2];
		b[4] = a[4] - a[2];
		b[6] = a[0] - a[6];
		b[1] = a[1] + (a[7]>>2);
		b[7] = -(a[1]>>2) + a[7];
		b[3] = a[3] + (a[5]>>2);
		b[5] = (a[3]>>2) - a[5];

		mm[0] = b[0] + b[7];
		mm[1] = b[2] + b[5];
		mm[2] = b[4] + b[3];
		mm[3] = b[6] + b[1];
		mm[4] = b[6] - b[1];
		mm[5] = b[4] - b[3];
		mm[6] = b[2] - b[5];
		mm[7] = b[0] - b[7];


		dest[0][i] = mm[0];
		dest[1][i] = mm[1];
		dest[2][i] = mm[2];
		dest[3][i] = mm[3];
		dest[4][i] = mm[4];
		dest[5][i] = mm[5];
		dest[6][i] = mm[6];
		dest[7][i] = mm[7];

		i++;
	}
}

#else
void iDCT_4x4_ATI(short* dest, short *src, short dest_stride)
{	
	__m64 mm0,  mm1,  mm2,  mm3;
	__m64 tmp0, tmp1, tmp2, tmp3;

	mm0 = *((__m64*)&src[0]);
	mm1 = *((__m64*)&src[4]);
	mm2 = *((__m64*)&src[8]);
	mm3 = *((__m64*)&src[12]);
	memset(src, 0, 16*sizeof(short));
#if !defined(_PRE_TRANSPOSE_)
	Transpose(mm0,mm1,mm2,mm3);
#endif

	// horizontal
	tmp0 = _m_paddw(mm0,mm2);
	tmp2 = _m_psubw(_m_psrawi(mm1,1),mm3);
	tmp1 = _m_psubw(mm0,mm2);
	tmp3 = _m_paddw(mm1,_m_psrawi(mm3,1));

	mm1 = _m_paddw(tmp1,tmp2);
	mm0 = _m_paddw(tmp0,tmp3);
	mm2 = _m_psubw(tmp1,tmp2);
	mm3 = _m_psubw(tmp0,tmp3);

	Transpose(mm0,mm1,mm2,mm3);

	//vertical
	tmp0 = _m_paddw(mm0,mm2);
	tmp2 = _m_psubw(_m_psrawi(mm1,1),mm3);
	tmp1 = _m_psubw(mm0,mm2);
	tmp3 = _m_paddw(mm1,_m_psrawi(mm3,1));

	{
		mm0 = _m_paddw(tmp0,tmp3);
		mm1 = _m_paddw(tmp1,tmp2);
		mm2 = _m_psubw(tmp1,tmp2);
		mm3 = _m_psubw(tmp0,tmp3);
	}

	*((__m64*)dest) = mm0;
	*((__m64*)(dest+dest_stride)) = mm1;
	*((__m64*)(dest+(dest_stride << 1))) = mm2;
	*((__m64*)(dest+(dest_stride << 1) + dest_stride)) = mm3;
}

void iDCT_4x4_nonATI(short* dest, short *src, short dest_stride)
{	
	__m64 mm0,  mm1,  mm2,  mm3, mm4, mm5;
	__m64 tmp0, tmp1, tmp2, tmp3;
	static __m64  ncoeff_32  = _mm_set1_pi16(32);

	mm0 = *((__m64*)&src[0]);
	mm1 = *((__m64*)&src[4]);
	mm2 = *((__m64*)&src[8]);
	mm3 = *((__m64*)&src[12]);
	memset(src, 0, 16*sizeof(short));
#if !defined(_PRE_TRANSPOSE_)
	Transpose(mm0,mm1,mm2,mm3);
#endif

	// horizontal
	tmp0 = _m_paddw(mm0,mm2);
	tmp1 = _m_psubw(mm0,mm2);
	tmp2 = _m_psubw(_m_psrawi(mm1,1),mm3);
	tmp3 = _m_paddw(mm1,_m_psrawi(mm3,1));

	mm0 = _m_paddw(tmp0,tmp3);
	mm1 = _m_paddw(tmp1,tmp2);
	mm2 = _m_psubw(tmp1,tmp2);
	mm3 = _m_psubw(tmp0,tmp3);

	Transpose(mm0,mm1,mm2,mm3);

	//vertical
	tmp0 = _m_paddw(mm0,mm2);
	tmp1 = _m_psubw(mm0,mm2);
	tmp2 = _m_psubw(_m_psrawi(mm1,1),mm3);
	tmp3 = _m_paddw(mm1,_m_psrawi(mm3,1));
	{
		mm0 = _m_psrawi(_m_paddw(_m_paddw(tmp0,tmp3),ncoeff_32),6);
		mm1 = _m_psrawi(_m_paddw(_m_paddw(tmp1,tmp2),ncoeff_32),6);
		mm2 = _m_psrawi(_m_paddw(_m_psubw(tmp1,tmp2),ncoeff_32),6);
		mm3 = _m_psrawi(_m_paddw(_m_psubw(tmp0,tmp3),ncoeff_32),6);
		mm4 = _mm_set1_pi16(-256);
		mm5 = _mm_set1_pi16(255);

		mm0 = _m_pmaxsw(mm0, mm4);
		mm0 = _m_pminsw(mm0, mm5);

		mm1 = _m_pmaxsw(mm1, mm4);
		mm1 = _m_pminsw(mm1, mm5);

		mm2 = _m_pmaxsw(mm2, mm4);
		mm2 = _m_pminsw(mm2, mm5);

		mm3 = _m_pmaxsw(mm3, mm4);
		mm3 = _m_pminsw(mm3, mm5);

	}

	*((__m64*)dest) = mm0;
	*((__m64*)(dest+dest_stride)) = mm1;
	*((__m64*)(dest+(dest_stride << 1))) = mm2;
	*((__m64*)(dest+(dest_stride << 1) + dest_stride)) = mm3;
}

void iDCT_8x8_ATI(short* dest, short *src, short dest_stride)
{
	// New code, SSE2 intrinsics
	__m128i xmm0,xmm1,xmm2,xmm3,xmm4,xmm5,xmm6,xmm7;
	const static unsigned short __declspec(align(16)) const_32[8] = { 32, 32, 32, 32, 32, 32, 32, 32 };

#ifdef _PRE_TRANSPOSE_
	xmm3 = _mm_load_si128((__m128i*)&src[0 ]); // a0 b0 c0 d0 e0 f0 g0 h0
	xmm6 = _mm_load_si128((__m128i*)&src[16]); // a2 b2 c2 d2 e2 f2 g2 h2
	xmm0 = _mm_load_si128((__m128i*)&src[32]); // a4 b4 c4 d4 e4 f4 g4 h4
	xmm5 = _mm_load_si128((__m128i*)&src[40]); // a5 b5 c5 d5 e5 f5 g5 h5
	xmm1 = _mm_load_si128((__m128i*)&src[48]); // a6 b6 c6 d6 e6 f6 g6 h6
	xmm2 = _mm_load_si128((__m128i*)&src[56]); // a7 b7 c7 d7 e7 f7 g7 h7
	// 6
#else
	// Transpose 8x8 matrix
	//start	
	//__asm int 3;
	xmm0 = _mm_load_si128((__m128i*)&src[0 ]); //a 0 1 2 3 4 5 6 7
	xmm1 = _mm_load_si128((__m128i*)&src[8 ]); //b 0 1 2 3 4 5 6 7
	xmm2 = _mm_load_si128((__m128i*)&src[16]); //c 0 1 2 3 4 5 6 7
	xmm3 = _mm_load_si128((__m128i*)&src[24]); //d 0 1 2 3 4 5 6 7

	xmm6 = xmm0; //a 0 1 2 3 4 5 6 7
	xmm7 = xmm2; //c 0 1 2 3 4 5 6 7

	xmm0 = _mm_unpacklo_epi16(xmm0,xmm1); // a0 b0 a1 b1 a2 b2 a3 b3
	xmm2 = _mm_unpacklo_epi16(xmm2,xmm3); // c0 d0 c1 d1 c2 d2 c3 d3
	xmm6 = _mm_unpackhi_epi16(xmm6,xmm1); // a4 b4 a5 b5 a6 b6 a7 b7
	xmm7 = _mm_unpackhi_epi16(xmm7,xmm3); // c4 d4 c5 d5 c6 d6 c7 d7

	xmm1 = xmm0; // a0 b0 a1 b1 a2 b2 a3 b3
	xmm3 = xmm6; // a4 b4 a5 b5 a6 b6 a7 b7

	xmm0 = _mm_unpacklo_epi32(xmm0,xmm2); // a0 b0 c0 d0 a1 b1 c1 d1
	xmm6 = _mm_unpacklo_epi32(xmm6,xmm7); // a4 b4 c4 d4 a5 b5 c5 d5
	xmm1 = _mm_unpackhi_epi32(xmm1,xmm2); // a2 b2 c2 d2 a3 b3 c3 d3
	xmm3 = _mm_unpackhi_epi32(xmm3,xmm7); // a6 b6 c6 d6 a7 b7 c7 d7
	// 16
	// xmm0 = abcd-0/1, xmm1 = abcd-2/3, xmm3 = abcd-6/7, xmm6 = abcd-4/5
	_mm_store_si128((__m128i*)&src[0 ], xmm6); // a4 b4 c4 d4 a5 b5 c5 d5
	_mm_store_si128((__m128i*)&src[16], xmm3); // a6 b6 c6 d6 a7 b7 c7 d7

	xmm4 = _mm_load_si128((__m128i*)&src[32]); //e 0 1 2 3 4 5 6 7
	xmm5 = _mm_load_si128((__m128i*)&src[40]); //f 0 1 2 3 4 5 6 7
	xmm6 = _mm_load_si128((__m128i*)&src[48]); //g 0 1 2 3 4 5 6 7
	xmm7 = _mm_load_si128((__m128i*)&src[56]); //h 0 1 2 3 4 5 6 7

	xmm2 = xmm4; //e 0 1 2 3 4 5 6 7
	xmm3 = xmm6; //g 0 1 2 3 4 5 6 7

	xmm2 = _mm_unpacklo_epi16(xmm2,xmm5); // e0 f0 e1 f1 e2 f2 e3 f3
	xmm3 = _mm_unpacklo_epi16(xmm3,xmm7); // g0 h0 g1 h1 g2 h2 g3 h3
	xmm4 = _mm_unpackhi_epi16(xmm4,xmm5); // e4 f4 e5 f5 e6 f6 e7 f7
	xmm6 = _mm_unpackhi_epi16(xmm6,xmm7); // g4 h4 g5 h5 g6 h6 g7 h7

	xmm5 = xmm2; // e0 f0 e1 f1 e2 f2 e3 f3
	xmm7 = xmm4; // e4 f4 e5 f5 e6 f6 e7 f7

	xmm5 = _mm_unpacklo_epi32(xmm5,xmm3); // e0 f0 g0 h0 e1 f1 g1 h1
	xmm7 = _mm_unpacklo_epi32(xmm7,xmm6); // e4 f4 g4 h4 e5 f5 g5 h5
	xmm2 = _mm_unpackhi_epi32(xmm2,xmm3); // e2 f2 g2 h2 e3 f3 g3 h3
	xmm4 = _mm_unpackhi_epi32(xmm4,xmm6); // e6 f6 g6 h6 e7 f7 g7 h7

	xmm3 = xmm0; // a0 b0 c0 d0 a1 b1 c1 d1
	xmm6 = xmm1; // a2 b2 c2 d2 a3 b3 c3 d3
	// 36
	// xmm0= abcd-0/1, xmm1= abcd-2/3, xmm2= efgh-2/3, xmm3= abcd-0/1, xmm4= efgh-6/7, xmm5= efgh-0/1, xmm6= abcd-2/3, xmm7= efgh-4/5
	// src[0]= abcd-4/5, src[8]= abcd-6/7
	xmm3 = _mm_unpacklo_epi64(xmm3,xmm5); // a0 b0 c0 d0 e0 f0 g0 h0
	xmm6 = _mm_unpacklo_epi64(xmm6,xmm2); // a2 b2 c2 d2 e2 f2 g2 h2
	xmm0 = _mm_unpackhi_epi64(xmm0,xmm5); // a1 b1 c1 d1 e1 f1 g1 h1
	xmm1 = _mm_unpackhi_epi64(xmm1,xmm2); // a3 b3 c3 d3 e3 f3 g3 h3

	_mm_store_si128((__m128i*)&src[8 ], xmm0); // a1 b1 c1 d1 e1 f1 g1 h1
	_mm_store_si128((__m128i*)&src[24], xmm1); // a3 b3 c3 d3 e3 f3 g3 h3

	xmm5 = _mm_load_si128((__m128i*)&src[0 ]); // a4 b4 c4 d4 a5 b5 c5 d5
	xmm2 = _mm_load_si128((__m128i*)&src[16]); // a6 b6 c6 d6 a7 b7 c7 d7

	xmm0 = xmm5; // a4 b4 c4 d4 a5 b5 c5 d5
	xmm1 = xmm2; // a6 b6 c6 d6 a7 b7 c7 d7

	xmm0 = _mm_unpacklo_epi64(xmm0,xmm7); // a4 b4 c4 d4 e4 f4 g4 h4
	xmm1 = _mm_unpacklo_epi64(xmm1,xmm4); // a6 b6 c6 d6 e6 f6 g6 h6
	xmm5 = _mm_unpackhi_epi64(xmm5,xmm7); // a5 b5 c5 d5 e5 f5 g5 h5
	xmm2 = _mm_unpackhi_epi64(xmm2,xmm4); // a7 b7 c7 d7 e7 f7 g7 h7
	// 50
#endif
	// xmm0= a4, xmm1= a6, xmm2= a7, xmm3= a0, xmm5= a5, xmm6= a2, src[16] = a1, src[24] = a3
	xmm4 = xmm3; // a0
	xmm7 = xmm6; // a2

	xmm3 = _mm_adds_epi16(xmm3, xmm0); // a0+a4 = a[0]
	xmm4 = _mm_subs_epi16(xmm4, xmm0); // a0-a4 = a[4]
	xmm7 = _mm_srai_epi16(xmm7, 1); // a2>>1
	xmm7 = _mm_subs_epi16(xmm7, xmm1); // (a2>>1) - a6 = a[2]
	xmm1 = _mm_srai_epi16(xmm1, 1); // a6>>1
	xmm6 = _mm_adds_epi16(xmm6, xmm1); // a2 + (a6>>1) = a[6]

	xmm0 = xmm3; // a[0]
	xmm1 = xmm4; // a[4]

	xmm0 = _mm_adds_epi16(xmm0, xmm6); // a[0] + a[6] = b[0]
	xmm1 = _mm_adds_epi16(xmm1, xmm7); // a[4] + a[2] = b[2]
	xmm3 = _mm_subs_epi16(xmm3, xmm6); // a[0] - a[6] = b[6]
	xmm4 = _mm_subs_epi16(xmm4, xmm7); // a[4] - a[2] = b[4]
	// 64
	// xmm0= b[0], xmm1= b[2], xmm2= a7, xmm3= b[6], xmm4 = b[4], xmm5= a5, src[16] = a1, src[24] = a3
	_mm_store_si128((__m128i*)&src[32], xmm0); // b[0]
	_mm_store_si128((__m128i*)&src[40], xmm1); // b[2]
	_mm_store_si128((__m128i*)&src[48], xmm4); // b[4]
	_mm_store_si128((__m128i*)&src[56], xmm3); // b[6]

	xmm6 = _mm_load_si128((__m128i*)&src[8 ]); // a1
	xmm7 = _mm_load_si128((__m128i*)&src[24]); // a3

	xmm0 = xmm6; // a1
	xmm1 = xmm7; // a3
	xmm3 = xmm5; // a5
	xmm4 = xmm2; // a7

	xmm0 = _mm_srai_epi16(xmm0, 1); // a1>>1
	xmm1 = _mm_srai_epi16(xmm1, 1); // a3>>1
	xmm3 = _mm_srai_epi16(xmm3, 1); // a5>>1
	xmm4 = _mm_srai_epi16(xmm4, 1); // a7>>1

	xmm0 = _mm_adds_epi16(xmm0, xmm6); // a1 + (a1>>1)
	xmm1 = _mm_adds_epi16(xmm1, xmm7); // a3 + (a3>>1)
	xmm3 = _mm_adds_epi16(xmm3, xmm5); // a5 + (a5>>1)
	xmm4 = _mm_adds_epi16(xmm4, xmm2); // a7 + (a7>>1)

	xmm0 = _mm_adds_epi16(xmm0, xmm5); //  a5 + a1 + (a1>>1)
	xmm1 = _mm_subs_epi16(xmm1, xmm2); // -a7 + a3 + (a3>>1)
	xmm3 = _mm_subs_epi16(xmm3, xmm6); // -a1 + a5 + (a5>>1)
	xmm4 = _mm_adds_epi16(xmm4, xmm7); //  a3 + a7 + (a7>>1)

	xmm7 = _mm_adds_epi16(xmm7, xmm0); //  a3 + a5 + a1 + (a1>>1) = a[7]
	xmm6 = _mm_subs_epi16(xmm6, xmm1); //  a1 + a7 - a3 - (a3>>1) = a[3]
	xmm2 = _mm_adds_epi16(xmm2, xmm3); //  a7 - a1 + a5 + (a5>>1) = a[5]
	xmm5 = _mm_subs_epi16(xmm5, xmm4); //  a5 - a3 - a7 - (a7>>1) = a[1]
	// 90
	// xmm2= a[5], xmm5= a[1], xmm6 = a[3], xmm7= a[7], src[32] = b[0], src[40] = b[2], src[48] = b[4], src[56] = b[6]
	xmm0 = xmm7; // a[7]
	xmm1 = xmm2; // a[5]

	xmm0 = _mm_srai_epi16(xmm0, 2); // a[7]>>2
	xmm1 = _mm_srai_epi16(xmm1, 2); // a[5]>>2
	xmm0 = _mm_adds_epi16(xmm0, xmm5); //  a[1] + (a[7]>>2) = b[1]
	xmm1 = _mm_adds_epi16(xmm1, xmm6); //  a[3] + (a[5]>>2) = b[3]

	xmm6 = _mm_srai_epi16(xmm6, 2); // a[3]>>2
	xmm5 = _mm_srai_epi16(xmm5, 2); // a[1]>>2
	xmm6 = _mm_subs_epi16(xmm6, xmm2); //  (a[3]>>2) - a[5] = b[5]
	xmm7 = _mm_subs_epi16(xmm7, xmm5); //  a[7] - (a[1]>>2) = b[7]
	// 100
	// xmm0 = b[1], xmm1 = b[3], xmm6 = b[5], xmm7= b[7], src[32] = b[0], src[40] = b[2], src[48] = b[4], src[56] = b[6]
	xmm2 = _mm_load_si128((__m128i*)&src[32]); // b[0]
	xmm3 = _mm_load_si128((__m128i*)&src[40]); // b[2]
	xmm4 = xmm2; // b[0]
	xmm5 = xmm3; // b[2]

	xmm2 = _mm_adds_epi16(xmm2, xmm7); //  b[0] + b[7]
	xmm4 = _mm_subs_epi16(xmm4, xmm7); //  b[0] - b[7]
	xmm3 = _mm_adds_epi16(xmm3, xmm6); //  b[2] + b[5]
	xmm5 = _mm_subs_epi16(xmm5, xmm6); //  b[2] - b[5]

	_mm_store_si128((__m128i*)&src[0 ], xmm2); // MM0
	_mm_store_si128((__m128i*)&src[8 ], xmm3); // MM1

	xmm6 = _mm_load_si128((__m128i*)&src[48]); // b[4]
	xmm7 = _mm_load_si128((__m128i*)&src[56]); // b[6]
	xmm2 = xmm6; // b[4]
	xmm3 = xmm7; // b[6]

	xmm2 = _mm_adds_epi16(xmm2, xmm1); //  b[4] + b[3]
	xmm6 = _mm_subs_epi16(xmm6, xmm1); //  b[4] - b[3]
	xmm3 = _mm_adds_epi16(xmm3, xmm0); //  b[6] + b[1]
	xmm7 = _mm_subs_epi16(xmm7, xmm0); //  b[6] - b[1]
	// 118
	// xmm2 = MM2, xmm3 = MM3, xmm4 = MM7, xmm5 = MM6, xmm6 = MM5, xmm7 = MM4, src[0] = MM0, src[8] = MM1
	xmm0 = xmm7; //e 0 1 2 3 4 5 6 7
	xmm1 = xmm5; //g 0 1 2 3 4 5 6 7

	xmm0 = _mm_unpacklo_epi16(xmm0,xmm6); // e0 f0 e1 f1 e2 f2 e3 f3
	xmm1 = _mm_unpacklo_epi16(xmm1,xmm4); // g0 h0 g1 h1 g2 h2 g3 h3
	xmm7 = _mm_unpackhi_epi16(xmm7,xmm6); // e4 f4 e5 f5 e6 f6 e7 f7
	xmm5 = _mm_unpackhi_epi16(xmm5,xmm4); // g4 h4 g5 h5 g6 h6 g7 h7

	xmm6 = xmm0; // e0 f0 e1 f1 e2 f2 e3 f3
	xmm4 = xmm7; // e4 f4 e5 f5 e6 f6 e7 f7

	xmm6 = _mm_unpacklo_epi32(xmm6,xmm1); // e0 f0 g0 h0 e1 f1 g1 h1
	xmm4 = _mm_unpacklo_epi32(xmm4,xmm5); // e4 f4 g4 h4 e5 f5 g5 h5
	xmm0 = _mm_unpackhi_epi32(xmm0,xmm1); // e2 f2 g2 h2 e3 f3 g3 h3
	xmm7 = _mm_unpackhi_epi32(xmm7,xmm5); // e6 f6 g6 h6 e7 f7 g7 h7
	// 130
	// xmm0 = efgh-2/3, xmm2 = MM2, xmm3 = MM3, xmm4 = efgh-4/5, xmm6 = efgh-0/1, xmm7 = efgh-6/7, src[0] = MM0, src[8] = MM1
	_mm_store_si128((__m128i*)&src[16], xmm4); // e4 f4 g4 h4 e5 f5 g5 h5
	_mm_store_si128((__m128i*)&src[24], xmm7); // e6 f6 g6 h6 e7 f7 g7 h7

	xmm1 = _mm_load_si128((__m128i*)&src[0 ]); // a 0 1 2 3 4 5 6 7
	xmm5 = _mm_load_si128((__m128i*)&src[8 ]); // b 0 1 2 3 4 5 6 7
	xmm4 = xmm1; // a 0 1 2 3 4 5 6 7
	xmm7 = xmm2; // c 0 1 2 3 4 5 6 7

	xmm1 = _mm_unpacklo_epi16(xmm1,xmm5); // a0 b0 a1 b1 a2 b2 a3 b3
	xmm2 = _mm_unpacklo_epi16(xmm2,xmm3); // c0 d0 c1 d1 c2 d2 c3 d3
	xmm4 = _mm_unpackhi_epi16(xmm4,xmm5); // a4 b4 a5 b5 a6 b6 a7 b7
	xmm7 = _mm_unpackhi_epi16(xmm7,xmm3); // c4 d4 c5 d5 c6 d6 c7 d7

	xmm5 = xmm1; // a0 b0 a1 b1 a2 b2 a3 b3
	xmm3 = xmm4; // a4 b4 a5 b5 a6 b6 a7 b7

	xmm1 = _mm_unpacklo_epi32(xmm1,xmm2); // a0 b0 c0 d0 a1 b1 c1 d1
	xmm4 = _mm_unpacklo_epi32(xmm4,xmm7); // a4 b4 c4 d4 a5 b5 c5 d5
	xmm5 = _mm_unpackhi_epi32(xmm5,xmm2); // a2 b2 c2 d2 a3 b3 c3 d3
	xmm3 = _mm_unpackhi_epi32(xmm3,xmm7); // a6 b6 c6 d6 a7 b7 c7 d7
	// 146
	// xmm0 = efgh-2/3, xmm1 = abcd-0/1, xmm3 = abcd-6/7, xmm4 = abcd-4/5, xmm5 = abcd-2/3, xmm6 = efgh-0/1, src[16] = efgh-4/5, src[24] = efgh-6/7
	xmm2 = xmm1; // a0 b0 c0 d0 a1 b1 c1 d1
	xmm7 = xmm5; // a2 b2 c2 d2 a3 b3 c3 d3

	xmm1 = _mm_unpacklo_epi64(xmm1,xmm6); // a0 b0 c0 d0 e0 f0 g0 h0
	xmm5 = _mm_unpacklo_epi64(xmm5,xmm0); // a2 b2 c2 d2 e2 f2 g2 h2
	xmm2 = _mm_unpackhi_epi64(xmm2,xmm6); // a1 b1 c1 d1 e1 f1 g1 h1
	xmm7 = _mm_unpackhi_epi64(xmm7,xmm0); // a3 b3 c3 d3 e3 f3 g3 h3

	_mm_store_si128((__m128i*)&src[32], xmm2); // a1 b1 c1 d1 e1 f1 g1 h1
	_mm_store_si128((__m128i*)&src[40], xmm7); // a3 b3 c3 d3 e3 f3 g3 h3

	xmm6 = _mm_load_si128((__m128i*)&src[16]); // e4 f4 g4 h4 e5 f5 g5 h5
	xmm0 = _mm_load_si128((__m128i*)&src[24]); // e6 f6 g6 h6 e7 f7 g7 h7
	xmm2 = xmm4; // a4 b4 c4 d4 a5 b5 c5 d5
	xmm7 = xmm3; // a6 b6 c6 d6 a7 b7 c7 d7

	xmm4 = _mm_unpacklo_epi64(xmm4,xmm6); // a4 b4 c4 d4 e4 f4 g4 h4
	xmm3 = _mm_unpacklo_epi64(xmm3,xmm0); // a6 b6 c6 d6 e6 f6 g6 h6
	xmm2 = _mm_unpackhi_epi64(xmm2,xmm6); // a5 b5 c5 d5 e5 f5 g5 h5
	xmm7 = _mm_unpackhi_epi64(xmm7,xmm0); // a7 b7 c7 d7 e7 f7 g7 h7
	// 162
	// xmm1 = mm[0], xmm2 = mm[5], xmm3 = mm[6], xmm4 = mm[4], xmm5 = mm[2], xmm7 = mm[7], src[32] = mm[1], src[40] = mm[3]
	// ********************* //
	xmm6 = xmm1; //mm[0]
	xmm0 = xmm5; //mm[2]

	xmm1 = _mm_adds_epi16(xmm1, xmm4); // mm[0] + mm[4] = a[0]
	xmm6 = _mm_subs_epi16(xmm6, xmm4); // mm[0] - mm[4] = a[4]
	xmm5 = _mm_srai_epi16(xmm5, 1); // mm[2]>>1
	xmm5 = _mm_subs_epi16(xmm5, xmm3); // (mm[2]>>1) - mm[6] = a[2]
	xmm3 = _mm_srai_epi16(xmm3, 1); // mm[6]>>1
	xmm0 = _mm_adds_epi16(xmm0, xmm3); // mm[2] + (mm[6]>>1) = a[6]
	// 170
	// xmm0 = a[6], xmm1 = a[0], xmm2 = mm[5], xmm5 = a[2], xmm6 = a[4], xmm7 = mm[7], src[32] = mm[1], src[40] = mm[3]
	xmm4 = xmm1; // a[0]
	xmm3 = xmm6; // a[4]

	xmm4 = _mm_adds_epi16(xmm4, xmm0); // a[0] + a[6] = b[0]
	xmm3 = _mm_adds_epi16(xmm3, xmm5); // a[4] + a[2] = b[2]
	xmm1 = _mm_subs_epi16(xmm1, xmm0); // a[0] - a[6] = b[6]
	xmm6 = _mm_subs_epi16(xmm6, xmm5); // a[4] - a[2] = b[4]

	_mm_store_si128((__m128i*)&src[0 ], xmm4); // b[0]
	_mm_store_si128((__m128i*)&src[8 ], xmm3); // b[2]
	_mm_store_si128((__m128i*)&src[16], xmm6); // b[4]
	_mm_store_si128((__m128i*)&src[24], xmm1); // b[6]

	xmm0 = _mm_load_si128((__m128i*)&src[32]); // mm[1]
	xmm5 = _mm_load_si128((__m128i*)&src[40]); // mm[3]
	// 182
	// xmm0 = mm[1], xmm2 = mm[5], xmm5 = mm[3], xmm7 = mm[7], src[0] = b[0], src[8] = b[2], src[16] = b[4], src[24] = b[6]
	xmm1 = xmm0; // a1
	xmm3 = xmm5; // a3
	xmm4 = xmm2; // a5
	xmm6 = xmm7; // a7

	xmm1 = _mm_srai_epi16(xmm1, 1); // a1>>1
	xmm3 = _mm_srai_epi16(xmm3, 1); // a3>>1
	xmm4 = _mm_srai_epi16(xmm4, 1); // a5>>1
	xmm6 = _mm_srai_epi16(xmm6, 1); // a7>>1

	xmm1 = _mm_adds_epi16(xmm1, xmm0); // a1 + (a1>>1)
	xmm3 = _mm_adds_epi16(xmm3, xmm5); // a3 + (a3>>1)
	xmm4 = _mm_adds_epi16(xmm4, xmm2); // a5 + (a5>>1)
	xmm6 = _mm_adds_epi16(xmm6, xmm7); // a7 + (a7>>1)

	xmm1 = _mm_adds_epi16(xmm1, xmm2); //  a5 + a1 + (a1>>1)
	xmm3 = _mm_subs_epi16(xmm3, xmm7); // -a7 + a3 + (a3>>1)
	xmm4 = _mm_subs_epi16(xmm4, xmm0); // -a1 + a5 + (a5>>1)
	xmm6 = _mm_adds_epi16(xmm6, xmm5); //  a3 + a7 + (a7>>1)

	xmm5 = _mm_adds_epi16(xmm5, xmm1); //  a3 + a5 + a1 + (a1>>1) = a[7]
	xmm0 = _mm_subs_epi16(xmm0, xmm3); //  a1 + a7 - a3 - (a3>>1) = a[3]
	xmm7 = _mm_adds_epi16(xmm7, xmm4); //  a7 - a1 + a5 + (a5>>1) = a[5]
	xmm2 = _mm_subs_epi16(xmm2, xmm6); //  a5 - a3 - a7 - (a7>>1) = a[1]
	// 202
	// xmm0 = a[3], xmm2 = a[1], xmm5 = a[7], xmm7 = a[5], src[0] = b[0], src[8] = b[2], src[16] = b[4], src[24] = b[6]
	xmm1 = xmm5; // a[7]
	xmm3 = xmm7; // a[5]

	xmm1 = _mm_srai_epi16(xmm1, 2); // a[7]>>2
	xmm3 = _mm_srai_epi16(xmm3, 2); // a[5]>>2
	xmm1 = _mm_adds_epi16(xmm1, xmm2); //  a[1] + (a[7]>>2) = b[1]
	xmm3 = _mm_adds_epi16(xmm3, xmm0); //  a[3] + (a[5]>>2) = b[3]

	xmm2 = _mm_srai_epi16(xmm2, 2); // a[1]>>2
	xmm0 = _mm_srai_epi16(xmm0, 2); // a[3]>>2
	xmm5 = _mm_subs_epi16(xmm5, xmm2); //  a[7] - (a[1]>>2) = b[7]
	xmm0 = _mm_subs_epi16(xmm0, xmm7); //  (a[3]>>2) - a[5] = b[5]
	// 212
	// xmm0 = b[5], xmm1 = b[1], xmm3 = b[3], xmm5 = b[7], src[0] = b[0], src[8] = b[2], src[16] = b[4], src[24] = b[6]
	xmm2 = _mm_load_si128((__m128i*)&src[0 ]); // b[0]
	xmm4 = _mm_load_si128((__m128i*)&src[8 ]); // b[2]
	xmm6 = xmm2; // b[0]
	xmm7 = xmm4; // b[2]

	xmm2 = _mm_adds_epi16(xmm2, xmm5); //  b[0] + b[7]
	xmm6 = _mm_subs_epi16(xmm6, xmm5); //  b[0] - b[7]
	xmm4 = _mm_adds_epi16(xmm4, xmm0); //  b[2] + b[5]
	xmm7 = _mm_subs_epi16(xmm7, xmm0); //  b[2] - b[5]
	{
		// 220
		_mm_store_si128((__m128i*)dest, xmm2); // (b[0] + b[7])
		_mm_store_si128((__m128i*)(dest + dest_stride), xmm4); // (b[2] + b[5])
		// 240
		xmm2 = _mm_load_si128((__m128i*)&src[16]); // b[4]
		xmm0 = xmm2; // b[4]
		xmm2 = _mm_adds_epi16(xmm2, xmm3); //  b[4] + b[3]
		xmm0 = _mm_subs_epi16(xmm0, xmm3); //  b[4] - b[3]

		_mm_store_si128((__m128i*)(dest+ (dest_stride << 1)), xmm2); // (b[4] + b[3])
		// 254
		xmm3 = _mm_load_si128((__m128i*)&src[24]); // b[6]
		xmm4 = xmm3; // b[6]
		xmm4 = _mm_adds_epi16(xmm4, xmm1); //  b[6] + b[1]
		xmm3 = _mm_subs_epi16(xmm3, xmm1); //  b[6] - b[1]

		_mm_store_si128((__m128i*)(dest+(dest_stride << 1) + dest_stride), xmm4); // (b[6] + b[1])
		// 267

		_mm_store_si128((__m128i*)(dest+(dest_stride << 2)), xmm3); // (b[6] - b[1])
		_mm_store_si128((__m128i*)(dest+(dest_stride << 2) + dest_stride), xmm0); // (b[4] - b[3])
		_mm_store_si128((__m128i*)(dest+(dest_stride << 2) + (dest_stride << 1)), xmm7); // (b[2] - b[5])
		_mm_store_si128((__m128i*)(dest+(dest_stride << 3) - dest_stride), xmm6); // (b[0] - b[7])
	}

	// 287
	xmm5 = _mm_setzero_si128(); // all 0s
	_mm_store_si128((__m128i*)&src[0 ], xmm5); //0
	_mm_store_si128((__m128i*)&src[8 ], xmm5); //0
	_mm_store_si128((__m128i*)&src[16], xmm5); //0
	_mm_store_si128((__m128i*)&src[24], xmm5); //0
	_mm_store_si128((__m128i*)&src[32], xmm5); //0
	_mm_store_si128((__m128i*)&src[40], xmm5); //0
	_mm_store_si128((__m128i*)&src[48], xmm5); //0
	_mm_store_si128((__m128i*)&src[56], xmm5); //0
	// 295 - 44 if _PRE_TRANSPOSE_
}

void iDCT_8x8_nonATI(short* dest, short *src, short dest_stride)
{
	// New code, SSE2 intrinsics
	__m128i xmm0,xmm1,xmm2,xmm3,xmm4,xmm5,xmm6,xmm7;
	const static unsigned short __declspec(align(16)) const_32[8] = { 32, 32, 32, 32, 32, 32, 32, 32 };

#ifdef _PRE_TRANSPOSE_
	xmm3 = _mm_load_si128((__m128i*)&src[0 ]); // a0 b0 c0 d0 e0 f0 g0 h0
	xmm6 = _mm_load_si128((__m128i*)&src[16]); // a2 b2 c2 d2 e2 f2 g2 h2
	xmm0 = _mm_load_si128((__m128i*)&src[32]); // a4 b4 c4 d4 e4 f4 g4 h4
	xmm5 = _mm_load_si128((__m128i*)&src[40]); // a5 b5 c5 d5 e5 f5 g5 h5
	xmm1 = _mm_load_si128((__m128i*)&src[48]); // a6 b6 c6 d6 e6 f6 g6 h6
	xmm2 = _mm_load_si128((__m128i*)&src[56]); // a7 b7 c7 d7 e7 f7 g7 h7
	// 6
#else
	// Transpose 8x8 matrix
	//start	
	//__asm int 3;
	xmm0 = _mm_load_si128((__m128i*)&src[0 ]); //a 0 1 2 3 4 5 6 7
	xmm1 = _mm_load_si128((__m128i*)&src[8 ]); //b 0 1 2 3 4 5 6 7
	xmm2 = _mm_load_si128((__m128i*)&src[16]); //c 0 1 2 3 4 5 6 7
	xmm3 = _mm_load_si128((__m128i*)&src[24]); //d 0 1 2 3 4 5 6 7

	xmm6 = xmm0; //a 0 1 2 3 4 5 6 7
	xmm7 = xmm2; //c 0 1 2 3 4 5 6 7

	xmm0 = _mm_unpacklo_epi16(xmm0,xmm1); // a0 b0 a1 b1 a2 b2 a3 b3
	xmm2 = _mm_unpacklo_epi16(xmm2,xmm3); // c0 d0 c1 d1 c2 d2 c3 d3
	xmm6 = _mm_unpackhi_epi16(xmm6,xmm1); // a4 b4 a5 b5 a6 b6 a7 b7
	xmm7 = _mm_unpackhi_epi16(xmm7,xmm3); // c4 d4 c5 d5 c6 d6 c7 d7

	xmm1 = xmm0; // a0 b0 a1 b1 a2 b2 a3 b3
	xmm3 = xmm6; // a4 b4 a5 b5 a6 b6 a7 b7

	xmm0 = _mm_unpacklo_epi32(xmm0,xmm2); // a0 b0 c0 d0 a1 b1 c1 d1
	xmm6 = _mm_unpacklo_epi32(xmm6,xmm7); // a4 b4 c4 d4 a5 b5 c5 d5
	xmm1 = _mm_unpackhi_epi32(xmm1,xmm2); // a2 b2 c2 d2 a3 b3 c3 d3
	xmm3 = _mm_unpackhi_epi32(xmm3,xmm7); // a6 b6 c6 d6 a7 b7 c7 d7
	// 16
	// xmm0 = abcd-0/1, xmm1 = abcd-2/3, xmm3 = abcd-6/7, xmm6 = abcd-4/5
	_mm_store_si128((__m128i*)&src[0 ], xmm6); // a4 b4 c4 d4 a5 b5 c5 d5
	_mm_store_si128((__m128i*)&src[16], xmm3); // a6 b6 c6 d6 a7 b7 c7 d7

	xmm4 = _mm_load_si128((__m128i*)&src[32]); //e 0 1 2 3 4 5 6 7
	xmm5 = _mm_load_si128((__m128i*)&src[40]); //f 0 1 2 3 4 5 6 7
	xmm6 = _mm_load_si128((__m128i*)&src[48]); //g 0 1 2 3 4 5 6 7
	xmm7 = _mm_load_si128((__m128i*)&src[56]); //h 0 1 2 3 4 5 6 7

	xmm2 = xmm4; //e 0 1 2 3 4 5 6 7
	xmm3 = xmm6; //g 0 1 2 3 4 5 6 7

	xmm2 = _mm_unpacklo_epi16(xmm2,xmm5); // e0 f0 e1 f1 e2 f2 e3 f3
	xmm3 = _mm_unpacklo_epi16(xmm3,xmm7); // g0 h0 g1 h1 g2 h2 g3 h3
	xmm4 = _mm_unpackhi_epi16(xmm4,xmm5); // e4 f4 e5 f5 e6 f6 e7 f7
	xmm6 = _mm_unpackhi_epi16(xmm6,xmm7); // g4 h4 g5 h5 g6 h6 g7 h7

	xmm5 = xmm2; // e0 f0 e1 f1 e2 f2 e3 f3
	xmm7 = xmm4; // e4 f4 e5 f5 e6 f6 e7 f7

	xmm5 = _mm_unpacklo_epi32(xmm5,xmm3); // e0 f0 g0 h0 e1 f1 g1 h1
	xmm7 = _mm_unpacklo_epi32(xmm7,xmm6); // e4 f4 g4 h4 e5 f5 g5 h5
	xmm2 = _mm_unpackhi_epi32(xmm2,xmm3); // e2 f2 g2 h2 e3 f3 g3 h3
	xmm4 = _mm_unpackhi_epi32(xmm4,xmm6); // e6 f6 g6 h6 e7 f7 g7 h7

	xmm3 = xmm0; // a0 b0 c0 d0 a1 b1 c1 d1
	xmm6 = xmm1; // a2 b2 c2 d2 a3 b3 c3 d3
	// 36
	// xmm0= abcd-0/1, xmm1= abcd-2/3, xmm2= efgh-2/3, xmm3= abcd-0/1, xmm4= efgh-6/7, xmm5= efgh-0/1, xmm6= abcd-2/3, xmm7= efgh-4/5
	// src[0]= abcd-4/5, src[8]= abcd-6/7
	xmm3 = _mm_unpacklo_epi64(xmm3,xmm5); // a0 b0 c0 d0 e0 f0 g0 h0
	xmm6 = _mm_unpacklo_epi64(xmm6,xmm2); // a2 b2 c2 d2 e2 f2 g2 h2
	xmm0 = _mm_unpackhi_epi64(xmm0,xmm5); // a1 b1 c1 d1 e1 f1 g1 h1
	xmm1 = _mm_unpackhi_epi64(xmm1,xmm2); // a3 b3 c3 d3 e3 f3 g3 h3

	_mm_store_si128((__m128i*)&src[8 ], xmm0); // a1 b1 c1 d1 e1 f1 g1 h1
	_mm_store_si128((__m128i*)&src[24], xmm1); // a3 b3 c3 d3 e3 f3 g3 h3

	xmm5 = _mm_load_si128((__m128i*)&src[0 ]); // a4 b4 c4 d4 a5 b5 c5 d5
	xmm2 = _mm_load_si128((__m128i*)&src[16]); // a6 b6 c6 d6 a7 b7 c7 d7

	xmm0 = xmm5; // a4 b4 c4 d4 a5 b5 c5 d5
	xmm1 = xmm2; // a6 b6 c6 d6 a7 b7 c7 d7

	xmm0 = _mm_unpacklo_epi64(xmm0,xmm7); // a4 b4 c4 d4 e4 f4 g4 h4
	xmm1 = _mm_unpacklo_epi64(xmm1,xmm4); // a6 b6 c6 d6 e6 f6 g6 h6
	xmm5 = _mm_unpackhi_epi64(xmm5,xmm7); // a5 b5 c5 d5 e5 f5 g5 h5
	xmm2 = _mm_unpackhi_epi64(xmm2,xmm4); // a7 b7 c7 d7 e7 f7 g7 h7
	// 50
#endif
	// xmm0= a4, xmm1= a6, xmm2= a7, xmm3= a0, xmm5= a5, xmm6= a2, src[16] = a1, src[24] = a3
	xmm4 = xmm3; // a0
	xmm7 = xmm6; // a2

	xmm3 = _mm_adds_epi16(xmm3, xmm0); // a0+a4 = a[0]
	xmm4 = _mm_subs_epi16(xmm4, xmm0); // a0-a4 = a[4]
	xmm7 = _mm_srai_epi16(xmm7, 1); // a2>>1
	xmm7 = _mm_subs_epi16(xmm7, xmm1); // (a2>>1) - a6 = a[2]
	xmm1 = _mm_srai_epi16(xmm1, 1); // a6>>1
	xmm6 = _mm_adds_epi16(xmm6, xmm1); // a2 + (a6>>1) = a[6]

	xmm0 = xmm3; // a[0]
	xmm1 = xmm4; // a[4]

	xmm0 = _mm_adds_epi16(xmm0, xmm6); // a[0] + a[6] = b[0]
	xmm1 = _mm_adds_epi16(xmm1, xmm7); // a[4] + a[2] = b[2]
	xmm3 = _mm_subs_epi16(xmm3, xmm6); // a[0] - a[6] = b[6]
	xmm4 = _mm_subs_epi16(xmm4, xmm7); // a[4] - a[2] = b[4]
	// 64
	// xmm0= b[0], xmm1= b[2], xmm2= a7, xmm3= b[6], xmm4 = b[4], xmm5= a5, src[16] = a1, src[24] = a3
	_mm_store_si128((__m128i*)&src[32], xmm0); // b[0]
	_mm_store_si128((__m128i*)&src[40], xmm1); // b[2]
	_mm_store_si128((__m128i*)&src[48], xmm4); // b[4]
	_mm_store_si128((__m128i*)&src[56], xmm3); // b[6]

	xmm6 = _mm_load_si128((__m128i*)&src[8 ]); // a1
	xmm7 = _mm_load_si128((__m128i*)&src[24]); // a3

	xmm0 = xmm6; // a1
	xmm1 = xmm7; // a3
	xmm3 = xmm5; // a5
	xmm4 = xmm2; // a7

	xmm0 = _mm_srai_epi16(xmm0, 1); // a1>>1
	xmm1 = _mm_srai_epi16(xmm1, 1); // a3>>1
	xmm3 = _mm_srai_epi16(xmm3, 1); // a5>>1
	xmm4 = _mm_srai_epi16(xmm4, 1); // a7>>1

	xmm0 = _mm_adds_epi16(xmm0, xmm6); // a1 + (a1>>1)
	xmm1 = _mm_adds_epi16(xmm1, xmm7); // a3 + (a3>>1)
	xmm3 = _mm_adds_epi16(xmm3, xmm5); // a5 + (a5>>1)
	xmm4 = _mm_adds_epi16(xmm4, xmm2); // a7 + (a7>>1)

	xmm0 = _mm_adds_epi16(xmm0, xmm5); //  a5 + a1 + (a1>>1)
	xmm1 = _mm_subs_epi16(xmm1, xmm2); // -a7 + a3 + (a3>>1)
	xmm3 = _mm_subs_epi16(xmm3, xmm6); // -a1 + a5 + (a5>>1)
	xmm4 = _mm_adds_epi16(xmm4, xmm7); //  a3 + a7 + (a7>>1)

	xmm7 = _mm_adds_epi16(xmm7, xmm0); //  a3 + a5 + a1 + (a1>>1) = a[7]
	xmm6 = _mm_subs_epi16(xmm6, xmm1); //  a1 + a7 - a3 - (a3>>1) = a[3]
	xmm2 = _mm_adds_epi16(xmm2, xmm3); //  a7 - a1 + a5 + (a5>>1) = a[5]
	xmm5 = _mm_subs_epi16(xmm5, xmm4); //  a5 - a3 - a7 - (a7>>1) = a[1]
	// 90
	// xmm2= a[5], xmm5= a[1], xmm6 = a[3], xmm7= a[7], src[32] = b[0], src[40] = b[2], src[48] = b[4], src[56] = b[6]
	xmm0 = xmm7; // a[7]
	xmm1 = xmm2; // a[5]

	xmm0 = _mm_srai_epi16(xmm0, 2); // a[7]>>2
	xmm1 = _mm_srai_epi16(xmm1, 2); // a[5]>>2
	xmm0 = _mm_adds_epi16(xmm0, xmm5); //  a[1] + (a[7]>>2) = b[1]
	xmm1 = _mm_adds_epi16(xmm1, xmm6); //  a[3] + (a[5]>>2) = b[3]

	xmm6 = _mm_srai_epi16(xmm6, 2); // a[3]>>2
	xmm5 = _mm_srai_epi16(xmm5, 2); // a[1]>>2
	xmm6 = _mm_subs_epi16(xmm6, xmm2); //  (a[3]>>2) - a[5] = b[5]
	xmm7 = _mm_subs_epi16(xmm7, xmm5); //  a[7] - (a[1]>>2) = b[7]
	// 100
	// xmm0 = b[1], xmm1 = b[3], xmm6 = b[5], xmm7= b[7], src[32] = b[0], src[40] = b[2], src[48] = b[4], src[56] = b[6]
	xmm2 = _mm_load_si128((__m128i*)&src[32]); // b[0]
	xmm3 = _mm_load_si128((__m128i*)&src[40]); // b[2]
	xmm4 = xmm2; // b[0]
	xmm5 = xmm3; // b[2]

	xmm2 = _mm_adds_epi16(xmm2, xmm7); //  b[0] + b[7]
	xmm4 = _mm_subs_epi16(xmm4, xmm7); //  b[0] - b[7]
	xmm3 = _mm_adds_epi16(xmm3, xmm6); //  b[2] + b[5]
	xmm5 = _mm_subs_epi16(xmm5, xmm6); //  b[2] - b[5]

	_mm_store_si128((__m128i*)&src[0 ], xmm2); // MM0
	_mm_store_si128((__m128i*)&src[8 ], xmm3); // MM1

	xmm6 = _mm_load_si128((__m128i*)&src[48]); // b[4]
	xmm7 = _mm_load_si128((__m128i*)&src[56]); // b[6]
	xmm2 = xmm6; // b[4]
	xmm3 = xmm7; // b[6]

	xmm2 = _mm_adds_epi16(xmm2, xmm1); //  b[4] + b[3]
	xmm6 = _mm_subs_epi16(xmm6, xmm1); //  b[4] - b[3]
	xmm3 = _mm_adds_epi16(xmm3, xmm0); //  b[6] + b[1]
	xmm7 = _mm_subs_epi16(xmm7, xmm0); //  b[6] - b[1]
	// 118
	// xmm2 = MM2, xmm3 = MM3, xmm4 = MM7, xmm5 = MM6, xmm6 = MM5, xmm7 = MM4, src[0] = MM0, src[8] = MM1
	xmm0 = xmm7; //e 0 1 2 3 4 5 6 7
	xmm1 = xmm5; //g 0 1 2 3 4 5 6 7

	xmm0 = _mm_unpacklo_epi16(xmm0,xmm6); // e0 f0 e1 f1 e2 f2 e3 f3
	xmm1 = _mm_unpacklo_epi16(xmm1,xmm4); // g0 h0 g1 h1 g2 h2 g3 h3
	xmm7 = _mm_unpackhi_epi16(xmm7,xmm6); // e4 f4 e5 f5 e6 f6 e7 f7
	xmm5 = _mm_unpackhi_epi16(xmm5,xmm4); // g4 h4 g5 h5 g6 h6 g7 h7

	xmm6 = xmm0; // e0 f0 e1 f1 e2 f2 e3 f3
	xmm4 = xmm7; // e4 f4 e5 f5 e6 f6 e7 f7

	xmm6 = _mm_unpacklo_epi32(xmm6,xmm1); // e0 f0 g0 h0 e1 f1 g1 h1
	xmm4 = _mm_unpacklo_epi32(xmm4,xmm5); // e4 f4 g4 h4 e5 f5 g5 h5
	xmm0 = _mm_unpackhi_epi32(xmm0,xmm1); // e2 f2 g2 h2 e3 f3 g3 h3
	xmm7 = _mm_unpackhi_epi32(xmm7,xmm5); // e6 f6 g6 h6 e7 f7 g7 h7
	// 130
	// xmm0 = efgh-2/3, xmm2 = MM2, xmm3 = MM3, xmm4 = efgh-4/5, xmm6 = efgh-0/1, xmm7 = efgh-6/7, src[0] = MM0, src[8] = MM1
	_mm_store_si128((__m128i*)&src[16], xmm4); // e4 f4 g4 h4 e5 f5 g5 h5
	_mm_store_si128((__m128i*)&src[24], xmm7); // e6 f6 g6 h6 e7 f7 g7 h7

	xmm1 = _mm_load_si128((__m128i*)&src[0 ]); // a 0 1 2 3 4 5 6 7
	xmm5 = _mm_load_si128((__m128i*)&src[8 ]); // b 0 1 2 3 4 5 6 7
	xmm4 = xmm1; // a 0 1 2 3 4 5 6 7
	xmm7 = xmm2; // c 0 1 2 3 4 5 6 7

	xmm1 = _mm_unpacklo_epi16(xmm1,xmm5); // a0 b0 a1 b1 a2 b2 a3 b3
	xmm2 = _mm_unpacklo_epi16(xmm2,xmm3); // c0 d0 c1 d1 c2 d2 c3 d3
	xmm4 = _mm_unpackhi_epi16(xmm4,xmm5); // a4 b4 a5 b5 a6 b6 a7 b7
	xmm7 = _mm_unpackhi_epi16(xmm7,xmm3); // c4 d4 c5 d5 c6 d6 c7 d7

	xmm5 = xmm1; // a0 b0 a1 b1 a2 b2 a3 b3
	xmm3 = xmm4; // a4 b4 a5 b5 a6 b6 a7 b7

	xmm1 = _mm_unpacklo_epi32(xmm1,xmm2); // a0 b0 c0 d0 a1 b1 c1 d1
	xmm4 = _mm_unpacklo_epi32(xmm4,xmm7); // a4 b4 c4 d4 a5 b5 c5 d5
	xmm5 = _mm_unpackhi_epi32(xmm5,xmm2); // a2 b2 c2 d2 a3 b3 c3 d3
	xmm3 = _mm_unpackhi_epi32(xmm3,xmm7); // a6 b6 c6 d6 a7 b7 c7 d7
	// 146
	// xmm0 = efgh-2/3, xmm1 = abcd-0/1, xmm3 = abcd-6/7, xmm4 = abcd-4/5, xmm5 = abcd-2/3, xmm6 = efgh-0/1, src[16] = efgh-4/5, src[24] = efgh-6/7
	xmm2 = xmm1; // a0 b0 c0 d0 a1 b1 c1 d1
	xmm7 = xmm5; // a2 b2 c2 d2 a3 b3 c3 d3

	xmm1 = _mm_unpacklo_epi64(xmm1,xmm6); // a0 b0 c0 d0 e0 f0 g0 h0
	xmm5 = _mm_unpacklo_epi64(xmm5,xmm0); // a2 b2 c2 d2 e2 f2 g2 h2
	xmm2 = _mm_unpackhi_epi64(xmm2,xmm6); // a1 b1 c1 d1 e1 f1 g1 h1
	xmm7 = _mm_unpackhi_epi64(xmm7,xmm0); // a3 b3 c3 d3 e3 f3 g3 h3

	_mm_store_si128((__m128i*)&src[32], xmm2); // a1 b1 c1 d1 e1 f1 g1 h1
	_mm_store_si128((__m128i*)&src[40], xmm7); // a3 b3 c3 d3 e3 f3 g3 h3

	xmm6 = _mm_load_si128((__m128i*)&src[16]); // e4 f4 g4 h4 e5 f5 g5 h5
	xmm0 = _mm_load_si128((__m128i*)&src[24]); // e6 f6 g6 h6 e7 f7 g7 h7
	xmm2 = xmm4; // a4 b4 c4 d4 a5 b5 c5 d5
	xmm7 = xmm3; // a6 b6 c6 d6 a7 b7 c7 d7

	xmm4 = _mm_unpacklo_epi64(xmm4,xmm6); // a4 b4 c4 d4 e4 f4 g4 h4
	xmm3 = _mm_unpacklo_epi64(xmm3,xmm0); // a6 b6 c6 d6 e6 f6 g6 h6
	xmm2 = _mm_unpackhi_epi64(xmm2,xmm6); // a5 b5 c5 d5 e5 f5 g5 h5
	xmm7 = _mm_unpackhi_epi64(xmm7,xmm0); // a7 b7 c7 d7 e7 f7 g7 h7
	// 162
	// xmm1 = mm[0], xmm2 = mm[5], xmm3 = mm[6], xmm4 = mm[4], xmm5 = mm[2], xmm7 = mm[7], src[32] = mm[1], src[40] = mm[3]
	// ********************* //
	xmm6 = xmm1; //mm[0]
	xmm0 = xmm5; //mm[2]

	xmm1 = _mm_adds_epi16(xmm1, xmm4); // mm[0] + mm[4] = a[0]
	xmm6 = _mm_subs_epi16(xmm6, xmm4); // mm[0] - mm[4] = a[4]
	xmm5 = _mm_srai_epi16(xmm5, 1); // mm[2]>>1
	xmm5 = _mm_subs_epi16(xmm5, xmm3); // (mm[2]>>1) - mm[6] = a[2]
	xmm3 = _mm_srai_epi16(xmm3, 1); // mm[6]>>1
	xmm0 = _mm_adds_epi16(xmm0, xmm3); // mm[2] + (mm[6]>>1) = a[6]
	// 170
	// xmm0 = a[6], xmm1 = a[0], xmm2 = mm[5], xmm5 = a[2], xmm6 = a[4], xmm7 = mm[7], src[32] = mm[1], src[40] = mm[3]
	xmm4 = xmm1; // a[0]
	xmm3 = xmm6; // a[4]

	xmm4 = _mm_adds_epi16(xmm4, xmm0); // a[0] + a[6] = b[0]
	xmm3 = _mm_adds_epi16(xmm3, xmm5); // a[4] + a[2] = b[2]
	xmm1 = _mm_subs_epi16(xmm1, xmm0); // a[0] - a[6] = b[6]
	xmm6 = _mm_subs_epi16(xmm6, xmm5); // a[4] - a[2] = b[4]

	_mm_store_si128((__m128i*)&src[0 ], xmm4); // b[0]
	_mm_store_si128((__m128i*)&src[8 ], xmm3); // b[2]
	_mm_store_si128((__m128i*)&src[16], xmm6); // b[4]
	_mm_store_si128((__m128i*)&src[24], xmm1); // b[6]

	xmm0 = _mm_load_si128((__m128i*)&src[32]); // mm[1]
	xmm5 = _mm_load_si128((__m128i*)&src[40]); // mm[3]
	// 182
	// xmm0 = mm[1], xmm2 = mm[5], xmm5 = mm[3], xmm7 = mm[7], src[0] = b[0], src[8] = b[2], src[16] = b[4], src[24] = b[6]
	xmm1 = xmm0; // a1
	xmm3 = xmm5; // a3
	xmm4 = xmm2; // a5
	xmm6 = xmm7; // a7

	xmm1 = _mm_srai_epi16(xmm1, 1); // a1>>1
	xmm3 = _mm_srai_epi16(xmm3, 1); // a3>>1
	xmm4 = _mm_srai_epi16(xmm4, 1); // a5>>1
	xmm6 = _mm_srai_epi16(xmm6, 1); // a7>>1

	xmm1 = _mm_adds_epi16(xmm1, xmm0); // a1 + (a1>>1)
	xmm3 = _mm_adds_epi16(xmm3, xmm5); // a3 + (a3>>1)
	xmm4 = _mm_adds_epi16(xmm4, xmm2); // a5 + (a5>>1)
	xmm6 = _mm_adds_epi16(xmm6, xmm7); // a7 + (a7>>1)

	xmm1 = _mm_adds_epi16(xmm1, xmm2); //  a5 + a1 + (a1>>1)
	xmm3 = _mm_subs_epi16(xmm3, xmm7); // -a7 + a3 + (a3>>1)
	xmm4 = _mm_subs_epi16(xmm4, xmm0); // -a1 + a5 + (a5>>1)
	xmm6 = _mm_adds_epi16(xmm6, xmm5); //  a3 + a7 + (a7>>1)

	xmm5 = _mm_adds_epi16(xmm5, xmm1); //  a3 + a5 + a1 + (a1>>1) = a[7]
	xmm0 = _mm_subs_epi16(xmm0, xmm3); //  a1 + a7 - a3 - (a3>>1) = a[3]
	xmm7 = _mm_adds_epi16(xmm7, xmm4); //  a7 - a1 + a5 + (a5>>1) = a[5]
	xmm2 = _mm_subs_epi16(xmm2, xmm6); //  a5 - a3 - a7 - (a7>>1) = a[1]
	// 202
	// xmm0 = a[3], xmm2 = a[1], xmm5 = a[7], xmm7 = a[5], src[0] = b[0], src[8] = b[2], src[16] = b[4], src[24] = b[6]
	xmm1 = xmm5; // a[7]
	xmm3 = xmm7; // a[5]

	xmm1 = _mm_srai_epi16(xmm1, 2); // a[7]>>2
	xmm3 = _mm_srai_epi16(xmm3, 2); // a[5]>>2
	xmm1 = _mm_adds_epi16(xmm1, xmm2); //  a[1] + (a[7]>>2) = b[1]
	xmm3 = _mm_adds_epi16(xmm3, xmm0); //  a[3] + (a[5]>>2) = b[3]

	xmm2 = _mm_srai_epi16(xmm2, 2); // a[1]>>2
	xmm0 = _mm_srai_epi16(xmm0, 2); // a[3]>>2
	xmm5 = _mm_subs_epi16(xmm5, xmm2); //  a[7] - (a[1]>>2) = b[7]
	xmm0 = _mm_subs_epi16(xmm0, xmm7); //  (a[3]>>2) - a[5] = b[5]
	// 212
	// xmm0 = b[5], xmm1 = b[1], xmm3 = b[3], xmm5 = b[7], src[0] = b[0], src[8] = b[2], src[16] = b[4], src[24] = b[6]
	xmm2 = _mm_load_si128((__m128i*)&src[0 ]); // b[0]
	xmm4 = _mm_load_si128((__m128i*)&src[8 ]); // b[2]
	xmm6 = xmm2; // b[0]
	xmm7 = xmm4; // b[2]

	xmm2 = _mm_adds_epi16(xmm2, xmm5); //  b[0] + b[7]
	xmm6 = _mm_subs_epi16(xmm6, xmm5); //  b[0] - b[7]
	xmm4 = _mm_adds_epi16(xmm4, xmm0); //  b[2] + b[5]
	xmm7 = _mm_subs_epi16(xmm7, xmm0); //  b[2] - b[5]

	{

		__m128i rd_min, rd_max;
		rd_min = _mm_set1_epi16(-256);
		rd_max = _mm_set1_epi16(255);


		// 220
		// xmm1 = b[1], xmm2 = ROW[0], xmm3 = b[3], xmm4 = ROW[1], xmm6 = ROW[7], xmm7 = ROW[6], src[16] = b[4], src[24] = b[6]
		xmm5 = _mm_load_si128((__m128i*)&const_32); // all 32s
		xmm2 = _mm_adds_epi16(xmm2, xmm5); //  b[0] + b[7] + 32
		xmm2 = _mm_srai_epi16(xmm2, 6); // (b[0] + b[7] + 32)>>6
		xmm6 = _mm_adds_epi16(xmm6, xmm5); //  b[0] - b[7] + 32
		xmm6 = _mm_srai_epi16(xmm6, 6); // (b[0] - b[7] + 32)>>6

		xmm4 = _mm_adds_epi16(xmm4, xmm5); //  b[2] + b[5] + 32
		xmm4 = _mm_srai_epi16(xmm4, 6); // (b[2] + b[5] + 32)>>6
		xmm7 = _mm_adds_epi16(xmm7, xmm5); //  b[2] - b[5] + 32
		xmm7 = _mm_srai_epi16(xmm7, 6); // (b[2] - b[5] + 32)>>6
		// 230

		xmm2 = _mm_max_epi16(rd_min,xmm2);
		xmm2 = _mm_min_epi16(rd_max,xmm2);

		xmm4 = _mm_max_epi16(rd_min,xmm4);
		xmm4 = _mm_min_epi16(rd_max,xmm4);

		_mm_store_si128((__m128i*)dest, xmm2); // (b[0] + b[7] + 32)>>6
		_mm_store_si128((__m128i*)(dest+dest_stride), xmm4); // (b[2] + b[5] + 32)>>6
		// 240
		xmm2 = _mm_load_si128((__m128i*)&src[16]); // b[4]
		xmm0 = xmm2; // b[4]
		xmm2 = _mm_adds_epi16(xmm2, xmm3); //  b[4] + b[3]
		xmm0 = _mm_subs_epi16(xmm0, xmm3); //  b[4] - b[3]
		xmm2 = _mm_adds_epi16(xmm2, xmm5); //  b[4] + b[3] + 32
		xmm2 = _mm_srai_epi16(xmm2, 6); // (b[4] + b[3] + 32)>>6
		xmm0 = _mm_adds_epi16(xmm0, xmm5); //  b[4] - b[3] + 32
		xmm0 = _mm_srai_epi16(xmm0, 6); // (b[4] - b[3] + 32)>>6

		xmm2 = _mm_max_epi16(rd_min,xmm2);
		xmm2 = _mm_min_epi16(rd_max,xmm2);
		_mm_store_si128((__m128i*)(dest+(dest_stride<<1)), xmm2); // (b[4] + b[3] + 32)>>6
		// 254
		xmm3 = _mm_load_si128((__m128i*)&src[24]); // b[6]
		xmm4 = xmm3; // b[6]
		xmm4 = _mm_adds_epi16(xmm4, xmm1); //  b[6] + b[1]
		xmm3 = _mm_subs_epi16(xmm3, xmm1); //  b[6] - b[1]
		xmm4 = _mm_adds_epi16(xmm4, xmm5); //  b[6] + b[1] + 32
		xmm4 = _mm_srai_epi16(xmm4, 6); // (b[6] + b[1] + 32)>>6
		xmm3 = _mm_adds_epi16(xmm3, xmm5); //  b[6] - b[1] + 32
		xmm3 = _mm_srai_epi16(xmm3, 6); // (b[6] - b[1] + 32)>>6

		xmm4 = _mm_max_epi16(rd_min,xmm4);
		xmm4 = _mm_min_epi16(rd_max,xmm4);
		_mm_store_si128((__m128i*)(dest+ (dest_stride<<1) + dest_stride), xmm4); // (b[6] + b[1] + 32)>>6
		// 267
		xmm3 = _mm_max_epi16(rd_min,xmm3);
		xmm3 = _mm_min_epi16(rd_max,xmm3);

		xmm0 = _mm_max_epi16(rd_min,xmm0);
		xmm0 = _mm_min_epi16(rd_max,xmm0);

		xmm7 = _mm_max_epi16(rd_min,xmm7);
		xmm7 = _mm_min_epi16(rd_max,xmm7);

		xmm6 = _mm_max_epi16(rd_min,xmm6);
		xmm6 = _mm_min_epi16(rd_max,xmm6);

		_mm_store_si128((__m128i*)(dest+(dest_stride<<2)), xmm3); // (b[6] - b[1] + 32)>>6
		_mm_store_si128((__m128i*)(dest+ (dest_stride<<2) + dest_stride), xmm0); // (b[4] - b[3] + 32)>>6
		_mm_store_si128((__m128i*)(dest+ (dest_stride<<2) + (dest_stride<<1)), xmm7); // (b[2] - b[5] + 32)>>6
		_mm_store_si128((__m128i*)(dest+ (dest_stride<<3) - dest_stride), xmm6); // (b[0] - b[7] + 32)>>6
	}	

	// 287
	xmm5 = _mm_setzero_si128(); // all 0s
	_mm_store_si128((__m128i*)&src[0 ], xmm5); //0
	_mm_store_si128((__m128i*)&src[8 ], xmm5); //0
	_mm_store_si128((__m128i*)&src[16], xmm5); //0
	_mm_store_si128((__m128i*)&src[24], xmm5); //0
	_mm_store_si128((__m128i*)&src[32], xmm5); //0
	_mm_store_si128((__m128i*)&src[40], xmm5); //0
	_mm_store_si128((__m128i*)&src[48], xmm5); //0
	_mm_store_si128((__m128i*)&src[56], xmm5); //0
	// 295 - 44 if _PRE_TRANSPOSE_
}

#if 0
void iDCT_4x4_UV_ATI(short* dest, short *src_u, int residual_u, int residual_v, short stride)
{	
	short *src_v = src_u + 64;
	__m64 mm0,  mm1,  mm2,  mm3;
	__m64 tmp0, tmp1, tmp2, tmp3;

	const static unsigned short __declspec(align(16)) ncoeff_32_const[] = {32, 32, 32, 32};
	__m64 ncoeff_32 = *((__m64*)&ncoeff_32_const[0]);

	__m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7;

	if(residual_u)
	{
		// process U
		mm0 = *((__m64*)&src_u[0]);
		mm1 = *((__m64*)&src_u[4]);
		mm2 = *((__m64*)&src_u[8]);
		mm3 = *((__m64*)&src_u[12]);
		memset(src_u, 0, 16*sizeof(short));
#if !defined(_PRE_TRANSPOSE_)
		Transpose(mm0,mm1,mm2,mm3);
#endif

		// horizontal
		tmp0 = _m_paddw(mm0,mm2);
		tmp1 = _m_psubw(mm0,mm2);
		tmp2 = _m_psubw(_m_psrawi(mm1,1),mm3);
		tmp3 = _m_paddw(mm1,_m_psrawi(mm3,1));

		mm0 = _m_paddw(tmp0,tmp3);
		mm1 = _m_paddw(tmp1,tmp2);
		mm2 = _m_psubw(tmp1,tmp2);
		mm3 = _m_psubw(tmp0,tmp3);

		Transpose(mm0,mm1,mm2,mm3);

		//vertical
		tmp0 = _m_paddw(mm0,mm2);
		tmp1 = _m_psubw(mm0,mm2);
		tmp2 = _m_psubw(_m_psrawi(mm1,1),mm3);
		tmp3 = _m_paddw(mm1,_m_psrawi(mm3,1));

		{
			mm0 = _m_paddw(tmp0,tmp3);
			mm1 = _m_paddw(tmp1,tmp2);
			mm2 = _m_psubw(tmp1,tmp2);
			mm3 = _m_psubw(tmp0,tmp3);
		}

		xmm0 = _mm_movpi64_epi64(mm0);
		xmm1 = _mm_movpi64_epi64(mm1);
		xmm2 = _mm_movpi64_epi64(mm2);
		xmm3 = _mm_movpi64_epi64(mm3);

		//*((__m64*)dest) = mm0;
		//*((__m64*)(dest+(16>>shift_off))) = mm1;
		//*((__m64*)(dest+(32>>shift_off))) = mm2;
		//*((__m64*)(dest+(48>>shift_off))) = mm3;
	}
	else
	{
		xmm0 = //_mm_setzero_si128();
			xmm1 = //_mm_setzero_si128();
			xmm2 = //_mm_setzero_si128();
			xmm3 = _mm_setzero_si128();
	}


	if(residual_v)
	{
		// process V
		mm0 = *((__m64*)&src_v[0]);
		mm1 = *((__m64*)&src_v[4]);
		mm2 = *((__m64*)&src_v[8]);
		mm3 = *((__m64*)&src_v[12]);
		memset(src_v, 0, 16*sizeof(short));
#if !defined(_PRE_TRANSPOSE_)
		Transpose(mm0,mm1,mm2,mm3);
#endif
		// horizontal
		tmp0 = _m_paddw(mm0,mm2);
		tmp1 = _m_psubw(mm0,mm2);
		tmp2 = _m_psubw(_m_psrawi(mm1,1),mm3);
		tmp3 = _m_paddw(mm1,_m_psrawi(mm3,1));

		mm0 = _m_paddw(tmp0,tmp3);
		mm1 = _m_paddw(tmp1,tmp2);
		mm2 = _m_psubw(tmp1,tmp2);
		mm3 = _m_psubw(tmp0,tmp3);

		Transpose(mm0,mm1,mm2,mm3);

		//vertical
		tmp0 = _m_paddw(mm0,mm2);
		tmp1 = _m_psubw(mm0,mm2);
		tmp2 = _m_psubw(_m_psrawi(mm1,1),mm3);
		tmp3 = _m_paddw(mm1,_m_psrawi(mm3,1));

		{
			mm0 = _m_paddw(tmp0,tmp3);
			mm1 = _m_paddw(tmp1,tmp2);
			mm2 = _m_psubw(tmp1,tmp2);
			mm3 = _m_psubw(tmp0,tmp3);
		}

		xmm4 = _mm_movpi64_epi64(mm0);
		xmm5 = _mm_movpi64_epi64(mm1);
		xmm6 = _mm_movpi64_epi64(mm2);
		xmm7 = _mm_movpi64_epi64(mm3);
	}
	else
	{
		xmm4 = //_mm_setzero_si128();
			xmm5 = //_mm_setzero_si128();
			xmm6 = //_mm_setzero_si128();
			xmm7 = _mm_setzero_si128();
	}

	xmm0 = _mm_unpacklo_epi16(xmm0, xmm4);
	xmm1 = _mm_unpacklo_epi16(xmm1, xmm5);
	xmm2 = _mm_unpacklo_epi16(xmm2, xmm6);
	xmm3 = _mm_unpacklo_epi16(xmm3, xmm7);

	*((__m128i*)dest) = xmm0;
	//dest += stride;
	*((__m128i*)(dest+stride)) = xmm1; //*((__m128i*)(dest+(64>>shift_off))) = xmm1;
	//*((__m128i*)dest) = xmm1;
	//dest += stride;
	*((__m128i*)(dest+(stride << 1))) = xmm2; //*((__m128i*)(dest+(128>>shift_off))) = xmm2;
	//*((__m128i*)dest) = xmm2;
	//dest += stride;
	*((__m128i*)(dest+(stride << 1)+stride)) = xmm3; //*((__m128i*)(dest+(192>>shift_off))) = xmm3;
	//*((__m128i*)dest) = xmm3;	
}
#else
void iDCT_4x4_UV_ATI(short* dest, short *src_u, int residual_u, int residual_v, short stride)
{	
	short *src_v = src_u + 64;
	__m128i xmm0, xmm1, xmm2, xmm3,	xtmp0, xtmp1, xtmp2, xtmp3;

	if((residual_u==0) && (residual_v==0))
	{
		xtmp0 = 
			xtmp1 = 
			xtmp2 = 
			xtmp3 = _mm_setzero_si128();
	}else{
		// process U
		xmm0 = *((__m128i*)&src_u[0 ]); //11112222
		xmm1 = *((__m128i*)&src_u[8 ]); //33334444
		// process V
		xmm2 = *((__m128i*)&src_v[0 ]); //55556666
		xmm3 = *((__m128i*)&src_v[8 ]); //77778888

		xtmp0 = _mm_unpacklo_epi64(xmm0,xmm2);
		xtmp1 = _mm_unpackhi_epi64(xmm0,xmm2);
		xtmp2 = _mm_unpacklo_epi64(xmm1,xmm3);
		xtmp3 = _mm_unpackhi_epi64(xmm1,xmm3);

		// horizontal
		xmm0 = _mm_adds_epi16(xtmp0,xtmp2);	//m0+m2
		xmm2 = _mm_subs_epi16(_mm_srai_epi16(xtmp1,1),xtmp3); //  1/2m1-m3
		xmm1 = _mm_subs_epi16(xtmp0,xtmp2);	//m0-m2
		xmm3 = _mm_adds_epi16(xtmp1,_mm_srai_epi16(xtmp3,1)); //  m1+1/2m3

		xtmp0 = _mm_adds_epi16(xmm0,xmm3);		//m0+m1+m2+1/2m3 11115555
		xtmp1 = _mm_adds_epi16(xmm1,xmm2);		//m0+1/2m1-m2-m3 22226666	
		xtmp3 = _mm_subs_epi16(xmm0,xmm3);     //m0-m1+m2-1/2m3 44448888
		xtmp2 = _mm_subs_epi16(xmm1,xmm2);		//m0-1/2m1-m2+m3 33337777

		//transpose
		xmm0 =_mm_unpacklo_epi16(xtmp0,xtmp1);	// 12121212
		xmm2 =_mm_unpacklo_epi16(xtmp2,xtmp3); // 34343434
		xmm1 =_mm_unpackhi_epi16(xtmp0,xtmp1);	// 56565656 
		xmm3 =_mm_unpackhi_epi16(xtmp2,xtmp3);	// 78787878

		xtmp0 =_mm_unpacklo_epi32(xmm0,xmm2);	// 12341234
		xtmp2 =_mm_unpacklo_epi32(xmm1,xmm3);  // 56785678
		xtmp1 =_mm_unpackhi_epi32(xmm0,xmm2);	// 12341234 
		xtmp3 =_mm_unpackhi_epi32(xmm1,xmm3);	// 56785678

		xmm0 =_mm_unpacklo_epi64(xtmp0,xtmp2);	// 12345678
		xmm2 =_mm_unpacklo_epi64(xtmp1,xtmp3); // 12345678
		xmm1 =_mm_unpackhi_epi64(xtmp0,xtmp2);	// 12345678 
		xmm3 =_mm_unpackhi_epi64(xtmp1,xtmp3);	// 12345678

		//vertical
		xtmp0 = _mm_adds_epi16(xmm0,xmm2);
		xtmp2 = _mm_subs_epi16(_mm_srai_epi16(xmm1,1),xmm3);
		xtmp1 = _mm_subs_epi16(xmm0,xmm2);
		xtmp3 = _mm_adds_epi16(xmm1,_mm_srai_epi16(xmm3,1));


		xmm0 = _mm_adds_epi16(xtmp0,xtmp3); //lolololo hihihihi
		xmm1 = _mm_adds_epi16(xtmp1,xtmp2);
		xmm3 = _mm_subs_epi16(xtmp0,xtmp3);
		xmm2 = _mm_subs_epi16(xtmp1,xtmp2);

		xtmp0 = _mm_unpackhi_epi64(xmm0,xmm1);
		xtmp1 = _mm_unpackhi_epi64(xmm1,xmm0);
		xtmp2 = _mm_unpackhi_epi64(xmm2,xmm3);
		xtmp3 = _mm_unpackhi_epi64(xmm3,xmm2);

		xtmp0 = _mm_unpacklo_epi16(xmm0,xtmp0);
		xtmp1 = _mm_unpacklo_epi16(xmm1,xtmp1);
		xtmp2 = _mm_unpacklo_epi16(xmm2,xtmp2);
		xtmp3 = _mm_unpacklo_epi16(xmm3,xtmp3);
	}
	//lohilohilohi
	memset(src_v, 0, 16*sizeof(short));	
	memset(src_u, 0, 16*sizeof(short));

	_mm_store_si128((__m128i*)dest, xtmp0);
	_mm_store_si128((__m128i*)(dest+stride), xtmp1);
	_mm_store_si128((__m128i*)(dest+(stride << 1)), xtmp2);
	_mm_store_si128((__m128i*)(dest+(stride << 1)+stride), xtmp3);
}
#endif

void iDCT_4x4_UV_nonATI(short* dest, short *src_u, int residual_u, int residual_v, short stride)
{	
	short *src_v = src_u + 64;
	__m64 mm0,  mm1,  mm2,  mm3;
	__m64 tmp0, tmp1, tmp2, tmp3;

	const static unsigned short __declspec(align(16)) ncoeff_32_const[] = {32, 32, 32, 32};
	__m64 ncoeff_32 = *((__m64*)&ncoeff_32_const[0]);

	__m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7;

	if(residual_u)
	{
		// process U
		mm0 = *((__m64*)&src_u[0]);
		mm1 = *((__m64*)&src_u[4]);
		mm2 = *((__m64*)&src_u[8]);
		mm3 = *((__m64*)&src_u[12]);
		memset(src_u, 0, 16*sizeof(short));
#if !defined(_PRE_TRANSPOSE_)
		Transpose(mm0,mm1,mm2,mm3);
#endif

		// horizontal
		tmp0 = _m_paddw(mm0,mm2);
		tmp1 = _m_psubw(mm0,mm2);
		tmp2 = _m_psubw(_m_psrawi(mm1,1),mm3);
		tmp3 = _m_paddw(mm1,_m_psrawi(mm3,1));

		mm0 = _m_paddw(tmp0,tmp3);
		mm1 = _m_paddw(tmp1,tmp2);
		mm2 = _m_psubw(tmp1,tmp2);
		mm3 = _m_psubw(tmp0,tmp3);

		Transpose(mm0,mm1,mm2,mm3);

		//vertical
		tmp0 = _m_paddw(mm0,mm2);
		tmp1 = _m_psubw(mm0,mm2);
		tmp2 = _m_psubw(_m_psrawi(mm1,1),mm3);
		tmp3 = _m_paddw(mm1,_m_psrawi(mm3,1));

		{
			mm0 = _m_psrawi(_m_paddw(_m_paddw(tmp0,tmp3),ncoeff_32),6);
			mm1 = _m_psrawi(_m_paddw(_m_paddw(tmp1,tmp2),ncoeff_32),6);
			mm2 = _m_psrawi(_m_paddw(_m_psubw(tmp1,tmp2),ncoeff_32),6);
			mm3 = _m_psrawi(_m_paddw(_m_psubw(tmp0,tmp3),ncoeff_32),6);
		}

		xmm0 = _mm_movpi64_epi64(mm0);
		xmm1 = _mm_movpi64_epi64(mm1);
		xmm2 = _mm_movpi64_epi64(mm2);
		xmm3 = _mm_movpi64_epi64(mm3);

	} else {
		xmm0 = //_mm_setzero_si128();
			xmm1 = //_mm_setzero_si128();
			xmm2 = //_mm_setzero_si128();
			xmm3 = _mm_setzero_si128();
	}


	if(residual_v)
	{
		// process V
		mm0 = *((__m64*)&src_v[0]);
		mm1 = *((__m64*)&src_v[4]);
		mm2 = *((__m64*)&src_v[8]);
		mm3 = *((__m64*)&src_v[12]);
		memset(src_v, 0, 16*sizeof(short));
#if !defined(_PRE_TRANSPOSE_)
		Transpose(mm0,mm1,mm2,mm3);
#endif
		// horizontal
		tmp0 = _m_paddw(mm0,mm2);
		tmp1 = _m_psubw(mm0,mm2);
		tmp2 = _m_psubw(_m_psrawi(mm1,1),mm3);
		tmp3 = _m_paddw(mm1,_m_psrawi(mm3,1));

		mm0 = _m_paddw(tmp0,tmp3);
		mm1 = _m_paddw(tmp1,tmp2);
		mm2 = _m_psubw(tmp1,tmp2);
		mm3 = _m_psubw(tmp0,tmp3);

		Transpose(mm0,mm1,mm2,mm3);

		//vertical
		tmp0 = _m_paddw(mm0,mm2);
		tmp1 = _m_psubw(mm0,mm2);
		tmp2 = _m_psubw(_m_psrawi(mm1,1),mm3);
		tmp3 = _m_paddw(mm1,_m_psrawi(mm3,1));

		{
			mm0 = _m_psrawi(_m_paddw(_m_paddw(tmp0,tmp3),ncoeff_32),6);
			mm1 = _m_psrawi(_m_paddw(_m_paddw(tmp1,tmp2),ncoeff_32),6);
			mm2 = _m_psrawi(_m_paddw(_m_psubw(tmp1,tmp2),ncoeff_32),6);
			mm3 = _m_psrawi(_m_paddw(_m_psubw(tmp0,tmp3),ncoeff_32),6);
		}

		xmm4 = _mm_movpi64_epi64(mm0);
		xmm5 = _mm_movpi64_epi64(mm1);
		xmm6 = _mm_movpi64_epi64(mm2);
		xmm7 = _mm_movpi64_epi64(mm3);
	}
	else
	{
		xmm4 = //_mm_setzero_si128();
			xmm5 = //_mm_setzero_si128();
			xmm6 = //_mm_setzero_si128();
			xmm7 = _mm_setzero_si128();
	}

	xmm0 = _mm_unpacklo_epi16(xmm0, xmm4);
	xmm1 = _mm_unpacklo_epi16(xmm1, xmm5);
	xmm2 = _mm_unpacklo_epi16(xmm2, xmm6);
	xmm3 = _mm_unpacklo_epi16(xmm3, xmm7);

	{
		xmm4 = _mm_set1_epi16(-256);
		xmm5 = _mm_set1_epi16(255);

		xmm0 = _mm_max_epi16(xmm4,xmm0);
		xmm0 = _mm_min_epi16(xmm5,xmm0);

		xmm1 = _mm_max_epi16(xmm4,xmm1);
		xmm1 = _mm_min_epi16(xmm5,xmm1);

		xmm2 = _mm_max_epi16(xmm4,xmm2);
		xmm2 = _mm_min_epi16(xmm5,xmm2);

		xmm3 = _mm_max_epi16(xmm4,xmm3);
		xmm3 = _mm_min_epi16(xmm5,xmm3);
	}

	*((__m128i*)dest) = xmm0;
	*((__m128i*)(dest+stride)) = xmm1; //*((__m128i*)(dest+(64>>shift_off))) = xmm1;
	*((__m128i*)(dest+(stride << 1))) = xmm2; //*((__m128i*)(dest+(128>>shift_off))) = xmm2;
	*((__m128i*)(dest+(stride << 1)+stride)) = xmm3; //*((__m128i*)(dest+(192>>shift_off))) = xmm3;
}
#endif //!defined(_MMX_IDCT)
