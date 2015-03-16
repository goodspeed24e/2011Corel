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

#include <stdlib.h>
#include <stdio.h>
#include <streams.h>
#include <dvdmedia.h>
#include <initguid.h>
#include "GMO/GMO_i.h"
#include "GMO/GMOBase.h"
#include "GMO/GMOImpl.h"
#include "GMO/GMediaBuffer.h"
#include "GMO/GMOHelper.h"

#include "IH264VDec.h"
#include "IviMediatypeGuids.h"
#include "IVdoParameter.h"
#if defined(_MT_FOR_MULTI_SLICE_)
#include <process.h>
#endif
#if defined (_USE_SCRAMBLE_DATA_)
#include "DataEncryption.h"
#endif

//#define _ENABLE_ISMPGMO_ //for GMO interface Data Scramble
#define H264_ISMPGMO_SIZE   64
#define H264_ISMPGMO_OFFSET 0

#if defined(_ENABLE_ISMPGMO_)
#include "./ISMP/ISMPGMOGUID.h"
#include "./ISMP/ISMPGMOBRIDGE.h"

#if !defined(TRSDK_VER) && !defined(_DEBUG)
#pragma comment(lib,"../../Exports/Lib/LIBISMP_VC8.lib")
#pragma message("[H264VDec]...link LIBISMP_VC8.lib")
#elif !defined(TRSDK_VER) && defined(_DEBUG)
#pragma comment(lib,"../../Exports/Lib/LIBISMP_VC8_D.lib")
#pragma message("[H264VDec]...link LIBISMP_VC8_D.lib")
#elif TRSDK_VER >= 1000 && _MSC_VER >= 1400 && !defined(_DEBUG)
iviTR_TREXE2_SPECIFY_LIB
#pragma comment(lib,"../../Exports/Lib/LIBISMP_VC8_TRSDK.lib")
#pragma message("[H264VDec]...link LIBISMP_VC8_TRSDK.lib")
#endif
#endif

#define STREAM_CAPACITY 5
#if defined(__GNUC__) && !defined(__INTEL_COMPILER)
static unsigned char buffer[STREAM_CAPACITY][4096] __attribute__((aligned(32)));
#else
static unsigned char __declspec(align(32)) buffer[STREAM_CAPACITY][40960];
#endif

#define _CALL_CONV __cdecl

CGMOPtr<IGMediaObject> m_pDecGMO;
CGMOPtr<IH264VdoParameter> m_pIParam;
CGMOPtr<IH264VDecDllAPI2> m_pIAPI;
CGMOPtr<IH264VDecDllDXVA> m_pIDXVA;

static HRESULT hr[STREAM_CAPACITY];
static H264VDecParam::H264VDecHP_OpenOptions OpenOptions[STREAM_CAPACITY];
static H264VDecParam::H264VDecHP_OpenOptionsEx OpenOptionsEx[STREAM_CAPACITY];
static H264VDecParam::H264VDecHP_DecodeOptions DecodeOptions[STREAM_CAPACITY];
static H264VDecParam::H264VDecHP_GetFrameOptions GetFrameOptions[STREAM_CAPACITY];
static H264VDecParam::H264VDecHP_Frame H264Frame[STREAM_CAPACITY];
static H264VDecParam::H264VDecHP_FrameEx H264FrameEx[STREAM_CAPACITY];
static H264VDecHP *pDecoder[STREAM_CAPACITY];
static DWORD dwFramesDecoded[STREAM_CAPACITY], dwSkippedFrames[STREAM_CAPACITY];
static DWORD dwFrameCounter[STREAM_CAPACITY], dwNumberOfFrames[STREAM_CAPACITY];
static FILE * fp[STREAM_CAPACITY];
static void *img[STREAM_CAPACITY];
static int thread_id[STREAM_CAPACITY];

//static HANDLE event_finish_stream[STREAM_CAPACITY];
static HANDLE thread_h_stream[STREAM_CAPACITY];
static unsigned int thread_id_stream[STREAM_CAPACITY];

static DWORD dwStartTime[STREAM_CAPACITY], dwStopTime[STREAM_CAPACITY], dwElapsedTime[STREAM_CAPACITY];

#if defined(_ENABLE_ISMPGMO_)
CGMOPtr<ISMPGMO> g_pISMPGMO;
ISMPGMOBridge *m_pISMP = NULL;
#endif

static HRESULT _CALL_CONV H264HP_GetDataGMO(
	IN PVOID pvContext,
	OUT const BYTE **ppbOutBuffer,
	OUT DWORD *pcbNumberOfBytesRead,
	OUT BOOL *pbHasPTS,
	OUT H264VDecParam::H264_TS *pTimeStamp
	)
{
	int read; 
	static int bLastSlice = 0;

	read = fread((void *)*ppbOutBuffer,1,sizeof(buffer[0]),(FILE *)pvContext);

	if(read<=0)
	{
		char *pOutBuffer = (char *)*ppbOutBuffer;
		if ( feof((FILE *)pvContext) && (!bLastSlice) ) {
			pOutBuffer[0] = 0;
			pOutBuffer[1] = 0;
			pOutBuffer[2] = 0;
			pOutBuffer[3] = 1;
			pOutBuffer[4] = 0xB;
			read = 5;
			bLastSlice = 1;

		} else {
			*pcbNumberOfBytesRead = 0;

			if(*pbHasPTS)
				*pbHasPTS = FALSE;
			return -1;
		}		
	}

	*pcbNumberOfBytesRead = read;
	if(*pbHasPTS)
		*pbHasPTS = FALSE;
	return S_OK;
}

static HRESULT _CALL_CONV H264HP_GetData0(
	IN PVOID pvContext,
	OUT const BYTE **ppbOutBuffer,
	OUT DWORD *pcbNumberOfBytesRead,
	OUT BOOL *pbHasPTS,
	OUT H264VDecParam::H264_TS *pTimeStamp
	)
{
	int read;
	static int bLastSlice = 0;

	read = fread(&buffer[0][0],1,sizeof(buffer[0]),(FILE *)pvContext);

	if(read<=0)
	{
		if ( feof((FILE *)pvContext) && (!bLastSlice) ) {
			buffer[0][0] = 0;
			buffer[0][1] = 0;
			buffer[0][2] = 0;
			buffer[0][3] = 1;
			buffer[0][4] = 0xB;
			read = 5;
			bLastSlice = 1;

		} else {
			*pcbNumberOfBytesRead = 0;

			if(*pbHasPTS)
				*pbHasPTS = FALSE;
			return -1;
		}		
	}
#if defined (_USE_SCRAMBLE_DATA_)
	// PERFORM_BUFFER_SCRAMBLING
	DataEncryption((unsigned char *)&buffer[0][0], read < H264_SCRAMBLE_DATA_LEN ? read : H264_SCRAMBLE_DATA_LEN);
#endif
	// PERFORM_FRAME_POINTER_SCRAMBLING
	DWORD dwScramblingPt = (DWORD)&buffer[0][0];
	dwScramblingPt = INLINE_SCRAMBLE(dwScramblingPt,SCRAMBLE_PARAM_1,SCRAMBLE_PARAM_2,SCRAMBLE_PARAM_3);
	*ppbOutBuffer = (unsigned char *)dwScramblingPt;

	*pcbNumberOfBytesRead = read;
	if(*pbHasPTS)
		*pbHasPTS = FALSE;
	return S_OK;
}

static HRESULT _CALL_CONV H264HP_GetData1(
	IN PVOID pvContext,
	OUT const BYTE **ppbOutBuffer,
	OUT DWORD *pcbNumberOfBytesRead,
	OUT BOOL *pbHasPTS,
	OUT H264VDecParam::H264_TS *pTimeStamp
	)
{
	int read;

	read = fread(&buffer[1][0],1,sizeof(buffer[1]),(FILE *)pvContext);
	if(read<=0)
	{
		*pcbNumberOfBytesRead = 0;
		if(*pbHasPTS)
			*pbHasPTS = FALSE;
		return -1;
	}
#if defined (_USE_SCRAMBLE_DATA_)
	// PERFORM_BUFFER_SCRAMBLING
	DataEncryption((unsigned char *)&buffer[1][0], read < H264_SCRAMBLE_DATA_LEN ? read : H264_SCRAMBLE_DATA_LEN);
#endif
	// PERFORM_FRAME_POINTER_SCRAMBLING
	DWORD dwScramblingPt = (DWORD)&buffer[1][0];
	dwScramblingPt = INLINE_SCRAMBLE(dwScramblingPt,SCRAMBLE_PARAM_1,SCRAMBLE_PARAM_2,SCRAMBLE_PARAM_3);
	*ppbOutBuffer = (unsigned char *)dwScramblingPt;

	*pcbNumberOfBytesRead = read;
	if(*pbHasPTS)
		*pbHasPTS = FALSE;
	return S_OK;
}

static HRESULT _CALL_CONV H264HP_GetData2(
	IN PVOID pvContext,
	OUT const BYTE **ppbOutBuffer,
	OUT DWORD *pcbNumberOfBytesRead,
	OUT BOOL *pbHasPTS,
	OUT H264VDecParam::H264_TS *pTimeStamp
	)
{
	int read;

	read = fread(&buffer[2][0],1,sizeof(buffer[2]),(FILE *)pvContext);
	if(read<=0)
	{
		*pcbNumberOfBytesRead = 0;
		if(*pbHasPTS)
			*pbHasPTS = FALSE;
		return -1;
	}
#if defined (_USE_SCRAMBLE_DATA_)
	// PERFORM_BUFFER_SCRAMBLING
	DataEncryption((unsigned char *)&buffer[2][0], read < H264_SCRAMBLE_DATA_LEN ? read : H264_SCRAMBLE_DATA_LEN);
#endif
	// PERFORM_FRAME_POINTER_SCRAMBLING
	DWORD dwScramblingPt = (DWORD)&buffer[2][0];
	dwScramblingPt = INLINE_SCRAMBLE(dwScramblingPt,SCRAMBLE_PARAM_1,SCRAMBLE_PARAM_2,SCRAMBLE_PARAM_3);
	*ppbOutBuffer = (unsigned char *)dwScramblingPt;

	*pcbNumberOfBytesRead = read;
	if(*pbHasPTS)
		*pbHasPTS = FALSE;
	return S_OK;
}

static HRESULT _CALL_CONV H264HP_GetData3(
	IN PVOID pvContext,
	OUT const BYTE **ppbOutBuffer,
	OUT DWORD *pcbNumberOfBytesRead,
	OUT BOOL *pbHasPTS,
	OUT H264VDecParam::H264_TS *pTimeStamp
	)
{
	int read;

	read = fread(&buffer[3][0],1,sizeof(buffer[3]),(FILE *)pvContext);
	if(read<=0)
	{
		*pcbNumberOfBytesRead = 0;
		if(*pbHasPTS)
			*pbHasPTS = FALSE;
		return -1;
	}
#if defined (_USE_SCRAMBLE_DATA_)
	// PERFORM_BUFFER_SCRAMBLING
	DataEncryption((unsigned char *)&buffer[3][0], read < H264_SCRAMBLE_DATA_LEN ? read : H264_SCRAMBLE_DATA_LEN);
#endif
	// PERFORM_FRAME_POINTER_SCRAMBLING
	DWORD dwScramblingPt = (DWORD)&buffer[3][0];
	dwScramblingPt = INLINE_SCRAMBLE(dwScramblingPt,SCRAMBLE_PARAM_1,SCRAMBLE_PARAM_2,SCRAMBLE_PARAM_3);
	*ppbOutBuffer = (unsigned char *)dwScramblingPt;

	*pcbNumberOfBytesRead = read;
	if(*pbHasPTS)
		*pbHasPTS = FALSE;
	return S_OK;
}

static HRESULT _CALL_CONV H264HP_GetData4(
	IN PVOID pvContext,
	OUT const BYTE **ppbOutBuffer,
	OUT DWORD *pcbNumberOfBytesRead,
	OUT BOOL *pbHasPTS,
	OUT H264VDecParam::H264_TS *pTimeStamp
	)
{
	int read;

	read = fread(&buffer[4][0],1,sizeof(buffer[4]),(FILE *)pvContext);
	if(read<=0)
	{
		*pcbNumberOfBytesRead = 0;
		if(*pbHasPTS)
			*pbHasPTS = FALSE;
		return -1;
	}
#if defined (_USE_SCRAMBLE_DATA_)
	// PERFORM_BUFFER_SCRAMBLING
	DataEncryption((unsigned char *)&buffer[4][0], read < H264_SCRAMBLE_DATA_LEN ? read : H264_SCRAMBLE_DATA_LEN);
#endif
	// PERFORM_FRAME_POINTER_SCRAMBLING
	DWORD dwScramblingPt = (DWORD)&buffer[4][0];
	dwScramblingPt = INLINE_SCRAMBLE(dwScramblingPt,SCRAMBLE_PARAM_1,SCRAMBLE_PARAM_2,SCRAMBLE_PARAM_3);
	*ppbOutBuffer = (unsigned char *)dwScramblingPt;

	*pcbNumberOfBytesRead = read;
	if(*pbHasPTS)
		*pbHasPTS = FALSE;
	return S_OK;
}

static H264VDecParam::PFN_H264HP_GET_DATA H264HP_GetData[STREAM_CAPACITY] = 
{
	H264HP_GetData0, H264HP_GetData1, H264HP_GetData2, H264HP_GetData3, H264HP_GetData4
};

unsigned __stdcall decode_streams(void *par)
{
	int *tmp = (int*)par;
	int sn = *tmp;

	dwStartTime[sn] = GetTickCount();
	for (dwFrameCounter[sn]=0;;)
	{
		dwFramesDecoded[sn] = 0;
		hr[sn] = m_pIAPI->H264VDec_DecodeFrame(pDecoder[sn],
			&DecodeOptions[sn],
			sizeof(DecodeOptions),
			&dwFramesDecoded[sn],
			&dwSkippedFrames[sn],
			img[sn]);

		if(dwFramesDecoded[sn]==0)
			break;

		while(dwFramesDecoded[sn])
		{
			// PERFORM_FRAME_POINTER_SCRAMBLING
			DWORD dwFrame = (DWORD)&H264Frame[sn];
			dwFrame = INLINE_SCRAMBLE(dwFrame,SCRAMBLE_PARAM_1,SCRAMBLE_PARAM_2,SCRAMBLE_PARAM_3);

			hr[sn] = m_pIAPI->H264VDec_GetFrame(pDecoder[sn],
				&GetFrameOptions[sn],
				sizeof(GetFrameOptions),
				(H264VDecParam::H264VDecHP_Frame *)dwFrame,
				sizeof(H264Frame),
				img[sn]);
			if(FAILED(hr[sn]))
				break;
			if(fp[sn])
			{
				/*
				if((reinterpret_cast<int>(H264Frame[sn].apbFrame[0])%16)!=0 ||
				(reinterpret_cast<int>(H264Frame[sn].apbFrame[1])%16)!=0 ||
				(reinterpret_cast<int>(H264Frame[sn].apbFrame[2])%16)!=0)
				{
				printf("[ALARM] address is not aligned: %p %p %p\n",H264Frame[sn].apbFrame[0],H264Frame[sn].apbFrame[1],H264Frame[sn].apbFrame[2]);
				}
				for(int YUV=0; YUV<3; YUV++)
				{
				unsigned char *pBuf;
				pBuf = H264Frame[sn].apbFrame[YUV]+H264Frame[sn].adwTop[YUV]*H264Frame[sn].adwStride[YUV]+H264Frame[sn].adwLeft[YUV];
				for(unsigned int lines=0;lines<H264Frame[sn].adwHeight[YUV];lines++)
				fwrite(pBuf+lines*H264Frame[sn].adwStride[YUV], 1, H264Frame[sn].adwWidth[YUV], fp[sn]);
				}
				*/

				if((reinterpret_cast<int>(H264Frame[sn].apbFrame[0])%16)!=0 ||
					(reinterpret_cast<int>(H264Frame[sn].apbFrame[1])%16)!=0 )
				{
					printf("[ALARM] address is not aligned: %p %p %p\n",H264Frame[sn].apbFrame[0],H264Frame[sn].apbFrame[1],H264Frame[sn].apbFrame[2]);
				}

				//Luma
				for(int YUV=0; YUV<1; YUV++)
				{
					unsigned char *pBuf;
					pBuf = H264Frame[sn].apbFrame[YUV]+H264Frame[sn].adwTop[YUV]*H264Frame[sn].adwStride[YUV]+H264Frame[sn].adwLeft[YUV];
					for(unsigned int lines=0;lines<H264Frame[sn].adwHeight[YUV];lines++)
						fwrite(pBuf+lines*H264Frame[sn].adwStride[YUV], 1, H264Frame[sn].adwWidth[YUV], fp[sn]);
				}

				int i;
				unsigned char *pBuf;
				unsigned char uv_temp[1920];
				int uv_width = H264Frame[sn].adwWidth[1];
				int uv_stride = H264Frame[sn].adwStride[1];

				//U
				pBuf = H264Frame[sn].apbFrame[1]+H264Frame[sn].adwTop[1]*uv_stride+H264Frame[sn].adwLeft[1];
				for(unsigned int lines=0;lines<H264Frame[sn].adwHeight[1];lines++)
				{
					for(i=0;i<uv_width;i++)
						uv_temp[i] = pBuf[i<<1];
					fwrite(uv_temp, 1, uv_width, fp[sn]);
					pBuf += uv_stride;
				}

				//V
				pBuf = H264Frame[sn].apbFrame[1]+H264Frame[sn].adwTop[1]*uv_stride+H264Frame[sn].adwLeft[1];
				pBuf++;
				for(unsigned int lines=0;lines<H264Frame[sn].adwHeight[1];lines++)
				{
					for(i=0;i<uv_width;i++)
						uv_temp[i] = pBuf[i<<1];
					fwrite(uv_temp, 1, uv_width, fp[sn]);
					pBuf += uv_stride;
				}
			}
			hr[sn] = m_pIAPI->H264VDec_ReleaseFrame(pDecoder[sn],H264Frame[sn].dwCookie, img[sn]);
			dwFramesDecoded[sn]--;
			dwFrameCounter[sn]++;
			if(dwNumberOfFrames[sn] && dwNumberOfFrames[sn]==dwFrameCounter[sn])
				goto end_decode;
		}
	}
end_decode:
	dwStopTime[sn] = GetTickCount();
	dwElapsedTime[sn] = dwStopTime[sn] - dwStartTime[sn];

	dwFrameCounter[sn] += dwSkippedFrames[sn];
	printf("Elapsed: %lu ms, Avg: %.2f ms (%.2f fr/s)\n#skipped frames: %lu, #decoded frames: %lu, #total frames: %lu\n",
		dwElapsedTime[sn],
		(float)dwElapsedTime[sn]/(float)dwFrameCounter[sn],
		(float)(dwFrameCounter[sn]*1000.)/(float)dwElapsedTime[sn],
		dwSkippedFrames[sn], dwFrameCounter[sn]-dwSkippedFrames[sn], dwFrameCounter[sn]);
	m_pIAPI->H264VDec_Close(pDecoder[sn], img[sn]);
	m_pIAPI->H264VDec_Release(pDecoder[sn]);
	if(OpenOptions[sn].pvDataContext)
	{
		fclose((FILE *)OpenOptions[sn].pvDataContext);
		OpenOptions[sn].pvDataContext = 0;
	}
	if(fp[sn])
	{
		fclose(fp[sn]);
		fp[sn] = 0;
	}

#if defined(_MT_FOR_MULTI_SLICE_)
	_endthreadex(0);
#endif
	return 1;
}

unsigned __stdcall decode_gmo(void *par)
{
	int *tmp = (int*)par;
	int sn = *tmp;
	BOOL bReadEnd = FALSE;
	HRESULT hr = S_OK;
#if defined(_ENABLE_ISMPGMO_)
	int nISMPCnt = 0, cbISMP = 0;
#endif

	GMO_OUTPUT_DATA_BUFFER gmoDecoderOutputBuf[2] = {0};
	DWORD	dwStatus = 0;
	BYTE *pbDecoderOutBuf[2];
	DWORD cbDecoderOutput[2] = {0};
	DWORD cbOutputSize[2] , nAlignment=1;

	const DWORD m_nDecoderReadInSize = 40960;
	BYTE *pbData = NULL;
	DWORD cbData;
	DWORD dwCBReadLength;
	BOOL bHasPTS;
	H264VDecParam::H264_TS tsNALU;

	CGMediaBufferMgr m_GMediaBufferMgr;

	for(int i=0 ; i<2; i++)
		m_pDecGMO->GetOutputSizeInfo(i, &cbOutputSize[i], &nAlignment);

	BOOL bAccepted = TRUE;
	DWORD dwInputStatus;
	while(dwFrameCounter[sn] != dwNumberOfFrames[sn])
	{
		CGMOPtr<IGMediaBuffer> pDecoderOutputBuf[2];
		for(int i=0 ; i<2; i++)
		{
			if(FAILED(m_GMediaBufferMgr.GetGMediaBuffer(&pDecoderOutputBuf[i], cbOutputSize[i], nAlignment)))
				return 0;
			gmoDecoderOutputBuf[i].pBuffer = pDecoderOutputBuf[i];
		}

		do 
		{
			m_pDecGMO->ProcessOutput(bReadEnd ? GMO_PROCESS_OUTPUT_DRAIN_WHEN_NO_INPUT:0, 2, gmoDecoderOutputBuf, &dwStatus);
			pDecoderOutputBuf[0]->GetBufferAndLength(&pbDecoderOutBuf[0], &cbDecoderOutput[0]);
			if ( cbDecoderOutput[0] > 0 )
			{
#if defined(_ENABLE_ISMPGMO_)
				cbISMP = m_pISMP->DescrambleGMediaBuffer(pDecoderOutputBuf[0]);

				if(cbISMP != TRUE)
					return E_FAIL;
#endif
				if (GetPrivateProfileInt("H264_REG_KEY", "OUTPUTNV12", 0, "C:\\H264VDecHP.ini"))
				{
					m_pIParam->GetParameter(&H264FrameEx[sn], H264VdoGMOParam::VDO_PARAM_GET_DISPLAY_IMG_INFORMATION);
					H264FrameEx[sn].dwCookie = 0;
					H264FrameEx[sn].apbFrame[0] = pbDecoderOutBuf[0];
					H264FrameEx[sn].apbFrame[1] = (PBYTE)(pbDecoderOutBuf[0] + H264FrameEx[sn].adwHeight[0] * H264FrameEx[sn].adwStride[0]);
					H264FrameEx[sn].apbFrame[2] = NULL;
				}
				else
					memcpy(&H264FrameEx[sn], pbDecoderOutBuf[0], cbDecoderOutput[0]);
				
				if (fp[sn])
				{
					if((reinterpret_cast<int>(H264FrameEx[sn].apbFrame[0])%16)!=0 ||
						(reinterpret_cast<int>(H264FrameEx[sn].apbFrame[1])%16)!=0 )
					{
						printf("[ALARM] address is not aligned: %p %p %p\n",H264FrameEx[sn].apbFrame[0],H264FrameEx[sn].apbFrame[1],H264FrameEx[sn].apbFrame[2]);
					}

					int nDisplayWidth = H264FrameEx[sn].dwDisplayWidth;
					int nDisplayHeight = H264FrameEx[sn].dwDisplayHeight;

					//Luma
					for(int YUV=0; YUV<1; YUV++)
					{
						unsigned char *pBuf;
						pBuf = H264FrameEx[sn].apbFrame[YUV]+H264FrameEx[sn].adwTop[YUV]*H264FrameEx[sn].adwStride[YUV]+H264FrameEx[sn].adwLeft[YUV];
						for(unsigned int lines=0; lines < nDisplayHeight; lines++)
							fwrite(pBuf+lines*H264FrameEx[sn].adwStride[YUV], 1, nDisplayWidth, fp[sn]);
					}

					int i;
					unsigned char *pBuf;
					unsigned char uv_temp[1920];
					int uv_width = nDisplayWidth>>1;
					int uv_height = nDisplayHeight>>1;
					int uv_stride = H264FrameEx[sn].adwStride[1];

					//U
					pBuf = H264FrameEx[sn].apbFrame[1]+H264FrameEx[sn].adwTop[1]*uv_stride+H264FrameEx[sn].adwLeft[1];
					for(unsigned int lines=0; lines < uv_height; lines++)
					{
						for(i=0; i < uv_width; i++)
							uv_temp[i] = pBuf[i<<1];
						fwrite(uv_temp, 1, uv_width, fp[sn]);
						pBuf += uv_stride;
					}

					//V
					pBuf = H264FrameEx[sn].apbFrame[1]+H264FrameEx[sn].adwTop[1]*uv_stride+H264FrameEx[sn].adwLeft[1];
					pBuf++;
					for(unsigned int lines=0; lines < uv_height; lines++)
					{
						for(i=0; i < uv_width; i++)
							uv_temp[i] = pBuf[i<<1];
						fwrite(uv_temp, 1, uv_width, fp[sn]);
						pBuf += uv_stride;
					}
				}

				pDecoderOutputBuf[1]->GetBufferAndLength(&pbDecoderOutBuf[1], &cbDecoderOutput[1]);
				if(cbDecoderOutput[1] > 0)
				{	// Get offset meta data
					H264VDecParam::H264VDecHP_Offset_Metadata H264Offset_Metadata;
					memcpy(&H264Offset_Metadata, pbDecoderOutBuf[1], cbDecoderOutput[1]);
				}
				m_pIParam->SetParameter((void *)H264FrameEx[sn].dwCookie, H264VdoGMOParam::VDO_PARAM_OPERATION_RELEASEFRAME);

				dwFrameCounter[sn]++;
				break;
			}
			else if (bReadEnd)
				return -1;

			if (gmoDecoderOutputBuf[0].dwStatus & GMO_OUTPUT_DATA_BUFFERF_INCOMPLETE)
				break;

			CGMOPtr<IGMediaBuffer> pDecoderInputBuf;
			if(FAILED(m_GMediaBufferMgr.GetGMediaBuffer(&pDecoderInputBuf, m_nDecoderReadInSize)))
				return 0;

			hr = m_pDecGMO->GetInputStatus(0, &dwInputStatus);
			if (SUCCEEDED(hr) && (dwInputStatus == GMO_INPUT_STATUSF_ACCEPT_DATA))
			{
				pDecoderInputBuf->GetBufferAndLength(&pbData, &cbData);
				H264HP_GetDataGMO(OpenOptionsEx[sn].pvDataContext, (const BYTE**)&pbData, &dwCBReadLength, &bHasPTS, &tsNALU);

				pDecoderInputBuf->SetLength(dwCBReadLength);
#if defined(_ENABLE_ISMPGMO_)
				if(nISMPCnt++ > 10)
				{
					cbISMP = m_pISMP->ScrambleGMediaBuffer(pDecoderInputBuf, ISMPGMO_DATA_ENCRYPT, H264_ISMPGMO_SIZE, H264_ISMPGMO_OFFSET);
					nISMPCnt = 0;
				}
				else
					cbISMP = m_pISMP->ScrambleGMediaBuffer(pDecoderInputBuf, ISMPGMO_DATA_NO_ENCRYPT, 0, 0);

				if(cbISMP  != TRUE)
					return E_FAIL;
#endif
				m_pDecGMO->ProcessInput(0, pDecoderInputBuf, 0, 0, 0);

				if (dwCBReadLength==0) //EOS
				{
					bReadEnd = TRUE;
					continue;
				}
			}
		} while(1);
	}

}

/*!
***********************************************************************
* \brief
*    main function for H264VDecHP interface.
***********************************************************************
*/
int __cdecl main(int argc, char **argv)
{
	int i, nUseGMO;
	char *input_file[STREAM_CAPACITY], *output_file[STREAM_CAPACITY];

	int sn;
	int sn_max = STREAM_CAPACITY;

	HRESULT ret = S_OK;

	HMODULE hDecoderModule = LoadLibrary("H264VDec.dll");
	typedef void* (STDAPICALLTYPE * LPFNDLLCREATECOMPONENT)(void);
	LPFNDLLCREATECOMPONENT lpDllCreateComponent = reinterpret_cast<LPFNDLLCREATECOMPONENT>(GetProcAddress(hDecoderModule, "CreateComponent"));
	IGMediaObject *pGMO = NULL;
	if ( lpDllCreateComponent )
		pGMO = reinterpret_cast<IGMediaObject*>(lpDllCreateComponent());

	if (pGMO)
	{
		m_pDecGMO = pGMO;
		pGMO->Release();
	}

	if (m_pDecGMO){
		ret = m_pDecGMO->QueryInterface(IID_IH264VdoParameter, reinterpret_cast<void **>(&m_pIParam));
		if (FAILED(ret))
			return 0;
		ret = m_pDecGMO->QueryInterface(IID_IH264VDecDllAPI2, reinterpret_cast<void **>(&m_pIAPI));
		if (FAILED(ret))
			return 0;
		ret = m_pDecGMO->QueryInterface(IID_IH264VDecDllDXVA, reinterpret_cast<void **>(&m_pIDXVA));
		if (FAILED(ret))
			return 0;

#if defined(_ENABLE_ISMPGMO_)		
		m_pDecGMO->QueryInterface(IID_ISMPGMO, reinterpret_cast<void **>(&g_pISMPGMO));
		if(g_pISMPGMO)
		{
			m_pISMP = new ISMPGMOBridge();
			HRESULT hr = g_pISMPGMO->SPConnect(m_pISMP);
			if(FAILED(hr))
				return E_FAIL;
		}
		else
		{
			printf("Can't create ISMP\n");
			return E_FAIL;
		}
#endif
	}

#if defined (_USE_GRAPH_)
	//into dshow mode
	if(_stricmp(argv[1],"-ds")==0)
	{
		BOOL bUseCLFilter,bVisiable,bUseClock;
		int iIdx=2,iRunTimes=1;

		if(_stricmp(argv[iIdx],"-cl")==0)
		{
			bUseCLFilter = TRUE;
			iIdx++;
		}
		else
			bUseCLFilter = FALSE;

		if(_stricmp(argv[iIdx],"-v")==0)
		{
			bVisiable = TRUE;
			iIdx++;
		}
		else
			bVisiable = FALSE;

		if(_stricmp(argv[iIdx],"-c")==0)
		{
			bUseClock = TRUE;
			iIdx++;
		}
		else
			bUseClock = FALSE;

		if(_stricmp(argv[iIdx],"-t")==0)
		{
			iRunTimes = atoi(argv[++iIdx]);
			iIdx++;
		}

		int iFPS=0,iJitter=0,iSumFPS=0,iSumJitter=0;
		unsigned long ulCostTime=0,ulSumCostTime=0;
		CGraphBuilder GB;
		if(FAILED(GB.Open(argv[iIdx],bUseClock,bUseCLFilter)))
		{
			printf("Failed to build graph!\n");
			GB.Close();
			return -1;
		}

		for (int i=0;i<iRunTimes;i++)
		{
			printf("\n[%d]Decoding Data...\n",i+1);

			GB.Run(bVisiable);	

			iFPS = GB.GetAverageFrameRate();			
			iJitter = GB.GetJitter();

			if(iFPS!=0)
			{
				iSumFPS += iFPS;
				printf("[%d]Frame Rate: %.2f fps, total %d frames\n",i+1,static_cast<float>(iFPS)/100,GB.GetDecodedFrameNum());
			}
			else
			{
				ulCostTime = GB.GetTotalCostTimeMs();
				ulSumCostTime += ulCostTime;
				printf("[%d]Cost Time: %d ms",i+1,ulCostTime);
			}

			if(iJitter!=0)
			{
				iSumJitter += iJitter;
				printf("[%d]Jitter: %d ms\n",i+1,iJitter);
			}

		}

		if(iRunTimes>1)
		{		
			printf("----- the Average of %d times playing -----\n",iRunTimes);
			if(iSumFPS!=0)
				printf("Average Frame Rate: %.2f fps\n",static_cast<float>(iSumFPS)/100/iRunTimes);
			else
				printf("Average Cost Time: %d ms\n",ulSumCostTime/iRunTimes);

			if(iSumJitter!=0)
				printf("Average Jitter: %d ms\n",iSumJitter/iRunTimes);
		}

		GB.Close();
		return 0;
	}
#endif

	nUseGMO = GetPrivateProfileInt("H264_REG_KEY", "H264GMODecoder", 1, "C:\\H264VDecHP.ini");

	if (nUseGMO)
	{
		for(sn=0,i=1;sn<STREAM_CAPACITY && i<argc;sn++)
		{
			pDecoder[sn] = 0;
			input_file[sn] = 0;
			output_file[sn] = 0;
			fp[sn] = 0;
			img[sn] = 0;
			dwNumberOfFrames[sn] = -1;
			thread_id[sn] = sn;
			for(;i<argc;i++)
			{
				if(stricmp(argv[i],"+")==0)
				{
					i++;
					break;
				}
				else if(stricmp(argv[i],"-nf")==0)
					dwNumberOfFrames[sn] = atoi(argv[++i]);
				else if(input_file[sn]==0)
					input_file[sn] = argv[i];
				else if(output_file[sn]==0)
					output_file[sn] = argv[i];
			}
			if(sn==0 && input_file[sn]==0)
			{
				printf("H264VDecHPTest [-nf number_of_frames] inputfile [outputfile] [+ inputfile ...]\n");
				printf("or dshow mode\n");
				printf("H264VDecHPTest -ds [-v -c -t n] inputfile\n");
				return -1;
			}
			else if(input_file[sn]==0)
			{
				break;
			}
			if(output_file[sn])
				fp[sn] = fopen(output_file[sn],"wb");
			ZeroMemory(&OpenOptionsEx[sn], sizeof(H264VDecParam::H264VDecHP_OpenOptionsEx));
			ZeroMemory(&H264FrameEx[sn], sizeof(H264VDecParam::H264VDecHP_FrameEx));
			ZeroMemory(&DecodeOptions[sn],sizeof(H264VDecParam::H264VDecHP_DecodeOptions));
			ZeroMemory(&GetFrameOptions[sn],sizeof(H264VDecParam::H264VDecHP_GetFrameOptions));

#ifdef _WIN32
			OpenOptionsEx[sn].dwH264RegKey = GetPrivateProfileInt("H264_REG_KEY", "SMARTDEC", 0, "C:\\H264VDecHP.ini");
			if(GetPrivateProfileInt("H264_REG_KEY", "DEBLOCKING", 1, "C:\\H264VDecHP.ini"))
				OpenOptionsEx[sn].dwH264RegKey |= H264VDecParam::H264_REG_DEBLOCKING;
			if(!GetPrivateProfileInt("H264_REG_KEY", "NOVIDEODROP", 1, "C:\\H264VDecHP.ini"))
				OpenOptionsEx[sn].dwH264RegKey |= H264VDecParam::H264_REG_DROP_FRAME;
#else
			OpenOptionsEx[sn].dwH264RegKey = 0;
#endif

			OpenOptionsEx[sn].pfnDataCallback = NULL;
			OpenOptionsEx[sn].pvDataContext   = (void *)fopen(input_file[sn],"rb");  
			OpenOptionsEx[sn].dwBuffers		  = 8; // display Q size.
			OpenOptionsEx[sn].dwFillFrameNumGap	= 1;

			OpenOptionsEx[sn].pfnGetParamCallback = NULL;

			dwFrameCounter[sn]=0;

			m_pIParam->SetParameter(&OpenOptionsEx[sn], H264VdoGMOParam::VDO_PARAM_SET_OPEN_OPTION_EX);

			GMediaObjectHelper::CGMO_MEDIA_TYPE tmpGMT;
			GMediaObjectHelper::MoInitMediaType(&tmpGMT, sizeof(H264VDecParam::H264VDecHP_FrameEx));
			tmpGMT.majortype = MEDIATYPE_Video;
			tmpGMT.subtype = MEDIASUBTYPE_HLH264;
			if (FAILED(m_pDecGMO->SetInputType(0, &tmpGMT, 0)))
				return 0;

			tmpGMT.subtype = GetPrivateProfileInt("H264_REG_KEY", "OUTPUTNV12", 0, "C:\\H264VDecHP.ini")?MEDIASUBTYPE_NV12 : MEDIASUBTYPE_IVIH264;
			tmpGMT.cbFormat= sizeof(H264VDecParam::H264VDecHP_FrameEx);
			if (FAILED(m_pDecGMO->SetOutputType(0, &tmpGMT, 0)))
				return 0;

			tmpGMT.cbFormat = sizeof(H264VDecParam::H264VDecHP_Offset_Metadata);
			if (FAILED(m_pDecGMO->SetOutputType(1, &tmpGMT, 0)))	//Index: 1 is offset meta data. It still haven't new defined subtype ID.
				return 0;

			dwStartTime[sn] = GetTickCount();

			decode_gmo(&(thread_id[sn]));

			dwStopTime[sn] = GetTickCount();
			dwElapsedTime[sn] = dwStopTime[sn] - dwStartTime[sn];

			dwFrameCounter[sn] += dwSkippedFrames[sn];
			printf("Elapsed: %lu ms, Avg: %.2f ms (%.2f fr/s)\n#skipped frames: %lu, #decoded frames: %lu, #total frames: %lu\n",
				dwElapsedTime[sn],
				(float)dwElapsedTime[sn]/(float)dwFrameCounter[sn],
				(float)(dwFrameCounter[sn]*1000.)/(float)dwElapsedTime[sn],
				dwSkippedFrames[sn], dwFrameCounter[sn]-dwSkippedFrames[sn], dwFrameCounter[sn]);

			if(OpenOptionsEx[sn].pvDataContext)
			{
				fclose((FILE *)OpenOptionsEx[sn].pvDataContext);
				OpenOptionsEx[sn].pvDataContext = 0;
			}
			if(fp[sn])
			{
				fclose(fp[sn]);
				fp[sn] = 0;
			}
		}

		if (m_pDecGMO)
			m_pDecGMO->Flush();

		m_pDecGMO->FreeStreamingResources();

#if defined(_ENABLE_ISMPGMO_)
		if(m_pISMP)
		{
			delete m_pISMP;
			m_pISMP = NULL;
		}
#endif
	}
	else
	{
		for(sn=0,i=1;sn<STREAM_CAPACITY && i<argc;sn++)
		{
			pDecoder[sn] = 0;
			input_file[sn] = 0;
			output_file[sn] = 0;
			fp[sn] = 0;
			img[sn] = 0;
			dwNumberOfFrames[sn] = 0;
			thread_id[sn] = sn;
			for(;i<argc;i++)
			{
				if(stricmp(argv[i],"+")==0)
				{
					i++;
					break;
				}
				else if(stricmp(argv[i],"-nf")==0)
					dwNumberOfFrames[sn] = atoi(argv[++i]);
				else if(input_file[sn]==0)
					input_file[sn] = argv[i];
				else if(output_file[sn]==0)
					output_file[sn] = argv[i];
			}
			if(sn==0 && input_file[sn]==0)
			{
				printf("H264VDecHPTest [-nf number_of_frames] inputfile [outputfile] [+ inputfile ...]\n");
				printf("or dshow mode\n");
				printf("H264VDecHPTest -ds [-v -c -t n] inputfile\n");
				return -1;
			}
			else if(input_file[sn]==0)
			{
				break;
			}
			if(output_file[sn])
				fp[sn] = fopen(output_file[sn],"wb");
			ZeroMemory(&OpenOptions[sn], sizeof(H264VDecParam::H264VDecHP_OpenOptions));
			ZeroMemory(&H264Frame[sn], sizeof(H264VDecParam::H264VDecHP_Frame));
			ZeroMemory(&DecodeOptions[sn],sizeof(H264VDecParam::H264VDecHP_DecodeOptions));
			ZeroMemory(&GetFrameOptions[sn],sizeof(H264VDecParam::H264VDecHP_GetFrameOptions));
			hr[sn] = m_pIAPI->H264VDec_Create(&pDecoder[sn]);
			if(FAILED(hr[sn]))
				return 0;

#ifdef _WIN32
			OpenOptions[sn].dwH264RegKey = GetPrivateProfileInt("H264_REG_KEY", "SMARTDEC", 0, "C:\\H264VDecHP.ini");
			if(GetPrivateProfileInt("H264_REG_KEY", "DEBLOCKING", 1, "C:\\H264VDecHP.ini"))
				OpenOptions[sn].dwH264RegKey |= H264VDecParam::H264_REG_DEBLOCKING;
			if(!GetPrivateProfileInt("H264_REG_KEY", "NOVIDEODROP", 1, "C:\\H264VDecHP.ini"))
				OpenOptions[sn].dwH264RegKey |= H264VDecParam::H264_REG_DROP_FRAME;
#else
			OpenOptions[sn].dwH264RegKey = 0;
#endif

			OpenOptions[sn].pfnDataCallback = H264HP_GetData[sn];
			OpenOptions[sn].pvDataContext   = (void *)fopen(input_file[sn],"rb");  
			OpenOptions[sn].dwBuffers		  = 8; // display Q size.
			OpenOptions[sn].dwFillFrameNumGap	= 1;

			hr[sn] = m_pIAPI->H264VDec_Open(pDecoder[sn], &OpenOptions[sn], sizeof(OpenOptions), &img[sn]);
			if(FAILED(hr[sn]))
				return 0;
			dwFrameCounter[sn]=0;
			#if defined(_MT_FOR_MULTI_SLICE_)
					thread_h_stream[sn] = (HANDLE) _beginthreadex(NULL, 0, decode_streams, &thread_id[sn], 0, &(thread_id_stream[sn]));
			#else
					decode_streams(&(thread_id[sn]));
			#endif
		}
	}

	sn_max = sn;
	printf("POC must = frame# or field# for SNRs to be correct\n");
	printf("----------------------------------------------------------\n");
	printf("STREAM  Frame       POC   Pic#   QP  Y:U:V  Time(ms)\n");
	printf("----------------------------------------------------------\n");

#if defined(_MT_FOR_MULTI_SLICE_)
	for (sn=0; sn<sn_max; sn++)
	{
		WaitForSingleObject(thread_h_stream[sn], INFINITE);
		CloseHandle(thread_h_stream[sn]);
	}
#endif

	return 0;
}


