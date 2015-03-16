#include "StdAfx.h"

#define iviINITGUID

#include "DXVATestProgram.h"
#include <streams.h>
#include "IviMediatypeGuids.h"
#include <dvdmedia.h>
#include <initguid.h>
#include <uuids.h>

//#define _ENABLE_ISMPGMO_ //for GMO interface Data Scramble
#define H264_ISMPGMO_SIZE   64
#define H264_ISMPGMO_OFFSET 0

#if defined(_ENABLE_ISMPGMO_)
#include "./Imports/Inc/ISMP/ISMPGMOGUID.h"
#include "./Imports/Inc/ISMP/ISMPGMOBRIDGE.h"

#if !defined(TRSDK_VER) && !defined(_DEBUG)
#pragma comment(lib,"./Imports/Lib/LIBISMP_VC8.lib")
#pragma message("[H264VDec]...link LIBISMP_VC8.lib")
#elif !defined(TRSDK_VER) && defined(_DEBUG)
#pragma comment(lib,"./Imports/Lib/LIBISMP_VC8_D.lib")
#pragma message("[H264VDec]...link LIBISMP_VC8_D.lib")
#elif TRSDK_VER >= 1000 && _MSC_VER >= 1400 && !defined(_DEBUG)
iviTR_TREXE2_SPECIFY_LIB
#pragma comment(lib,"./Imports/Lib/LIBISMP_VC8_TRSDK.lib")
#pragma message("[H264VDec]...link LIBISMP_VC8_TRSDK.lib")
#endif
#endif

#if defined(_ENABLE_ISMPGMO_)
CGMOPtr<ISMPGMO> g_pISMPGMO;
ISMPGMOBridge *m_pISMP = NULL;
#endif

CDXVATestProgram::CDXVATestProgram(void)
{
	m_pService = NULL;
	
	ZeroMemory(&m_tcConfigFile, sizeof(m_tcConfigFile));

}

CDXVATestProgram::~CDXVATestProgram(void)
{

}

UINT CDXVATestProgram::GetClipNum()
{
	return GetPrivateProfileInt(m_tcKeyName, _T("TOTALCLIPNUM"), 0, m_tcConfigFile);
}

UINT CDXVATestProgram::GetClipWidth(TCHAR *ptcClipFile)
{
	return GetPrivateProfileInt(ptcClipFile, _T("WIDTH"), 1920, m_tcConfigFile);
}

UINT CDXVATestProgram::GetClipHeight(TCHAR *ptcClipFile)
{
	return GetPrivateProfileInt(ptcClipFile, _T("HEIGHT"), 1088, m_tcConfigFile);
}
/*void CDXVATestProgram::GetClipInputDir(TCHAR *ptcClipFile, TCHAR *InputDir)
{
	GetPrivateProfileString(m_tcKeyName, _T("INPUTDIR"), _T("D:\\"), InputDir, 256, m_tcConfigFile);
}
void CDXVATestProgram::GetClipOutputDir(TCHAR *ptcClipFile, TCHAR *OutputDir)
{
	GetPrivateProfileString(m_tcKeyName, _T("OUTPUTDIR"), _T("D:\\"), OutputDir, 256, m_tcConfigFile);
}*/
void CDXVATestProgram::NV24WriteFrame(BYTE* pbYBufferStart, BYTE* pbUVBufferStart, DWORD dwWidth, DWORD dwHeight, DWORD dwStride, FILE *fp)
{
	byte* pBuf;
	//int y_height = dwHeight;
	int y_height = (((dwHeight+15)>>4)<<4);
	int y_width = dwWidth;
	//int uv_height = dwHeight>>1;
	int uv_height = ((((dwHeight>>1)+7)>>3)<<3);
	int uv_width = dwWidth>>1;
	int LH = (((dwHeight+31)>>5)<<5);
	int CH = ((((LH>>1)+31)>>5)<<5);
	unsigned char *uv_temp = new unsigned char[uv_width*2];

	//Luma
	for(int lines=0; lines<y_height/2; lines++)
	{
		fwrite(pbYBufferStart+lines*dwStride, 1, y_width, fp);
		fwrite(pbYBufferStart+(LH/2+lines)*dwStride, 1, y_width, fp);
	}

	//V
	pBuf = pbUVBufferStart;
	for(int lines=0;lines<uv_height/2;lines++)
	{
		for(int i=0;i<uv_width;i++)
			uv_temp[i]=pBuf[i<<1];

		pBuf +=  (CH/2)*dwStride;
		for(int i=0;i<uv_width;i++)
			uv_temp[i+uv_width]=pBuf[i<<1];

		fwrite(uv_temp, 1, uv_width*2, fp);

		pBuf = pBuf-(CH/2)*dwStride + dwStride;
	}

	//U
	pBuf = pbUVBufferStart;
	pBuf++;
	for(int lines=0;lines<uv_height/2;lines++)
	{
		for(int i=0;i<uv_width;i++)
			uv_temp[i]=pBuf[i<<1];

		pBuf +=  (CH/2)*dwStride;
		for(int i=0;i<uv_width;i++)
			uv_temp[i+uv_width]=pBuf[i<<1];

		fwrite(uv_temp, 1, uv_width*2, fp);

		pBuf = pBuf-(CH/2)*dwStride + dwStride;
	}
	delete[] uv_temp;
}
void CDXVATestProgram::NV12WriteFrame(BYTE* pbYBufferStart, BYTE* pbUVBufferStart, DWORD dwWidth, DWORD dwHeight, DWORD dwStride, FILE *fp)
{
	byte *pBuf;
	int y_height = (((dwHeight+15)>>4)<<4);
	int y_width = dwWidth;
	int uv_height = ((((dwHeight>>1)+7)>>3)<<3);
	int uv_width = dwWidth>>1;
	unsigned char *uv_temp = new unsigned char[uv_width];

	//Luma
	for(int lines=0;lines<y_height;lines++)
		fwrite(pbYBufferStart+lines*dwStride, 1, dwWidth, fp);

	//U
	pBuf = pbUVBufferStart;
	for(int lines=0;lines<uv_height;lines++)
	{
		for(int i=0;i<uv_width;i++)
			uv_temp[i] = pBuf[i<<1];
		fwrite(uv_temp, 1, uv_width, fp);
		//fwrite(&pBuf[i<<1], 1, sizeof(byte), write_fp);
		pBuf +=  dwStride;
	}

	//V
	pBuf = pbUVBufferStart;
	pBuf++;
	for(int lines=0;lines<uv_height;lines++)
	{
		for(int i=0;i<uv_width;i++)
			uv_temp[i] = pBuf[i<<1];
		fwrite(uv_temp, 1, uv_width, fp);
		//fwrite(&pBuf[i<<1], 1, sizeof(byte), write_fp);
		pBuf +=  dwStride;
	}
	delete[] uv_temp;
}
void CDXVATestProgram::IMC3WriteFrame(BYTE* pbYBufferStart, BYTE* pbUVBufferStart, DWORD dwWidth, DWORD dwHeight, DWORD dwStride, FILE *fp)
{
	byte* pBuf;
	int y_height = (((dwHeight+15)>>4)<<4);
	int y_width = dwWidth;
	int uv_height = y_height;
	int uv_width = dwWidth>>1;
	unsigned char *uv_temp = new unsigned char[uv_width];

	//Luma
	for(int lines=0;lines<y_height;lines++)
		fwrite(pbYBufferStart+lines*dwStride, 1, y_width, fp);

	//V
	pBuf = pbUVBufferStart+uv_height/2*dwStride;
	for(int lines=0;lines<uv_height/2;lines++)
	{
		for(int i=0;i<uv_width;i++)
			uv_temp[i] = pBuf[i];
		fwrite(uv_temp, 1, uv_width, fp);
		pBuf +=  dwStride;
	}

	//U
	pBuf = pbUVBufferStart;
	for(int lines=0;lines<uv_height/2;lines++)
	{
		for(int i=0;i<uv_width;i++)
			uv_temp[i] = pBuf[i];
		fwrite(uv_temp, 1, uv_width, fp);
		pBuf +=  dwStride;
	}
	delete[] uv_temp;
}
void CDXVATestProgram::YV12WriteFrame(BYTE* pbYBufferStart, BYTE* pbUVBufferStart, DWORD dwWidth, DWORD dwHeight, DWORD dwStride, FILE *fp)
{
	unsigned char *pBuf;
	int y_height = dwHeight;
//	int y_height = (((dwHeight+15)>>4)<<4);
	int y_width = dwWidth;
	int uv_height = dwHeight>>1;
	int uv_width = dwWidth>>1;
	int uv_stride = dwStride>>1;
	unsigned char *uv_temp = new unsigned char[uv_width];

	//Y
	for(int lines=0; lines<y_height; lines++)
		fwrite(pbYBufferStart+lines*dwStride, 1, y_width, fp);

	//V
	pBuf = pbUVBufferStart;
	for(int lines=0; lines<uv_height; lines++)
	{
		for(int i=0;i<uv_width;i++)
			uv_temp[i] = pBuf[i];
		fwrite(uv_temp, 1, uv_width, fp);
		//fwrite(&pBuf[i<<1], 1, 1, fp);	
		pBuf += uv_stride;
	}

	//U
	for(int lines=0;lines<uv_height;lines++)
	{
		for(int i=0;i<uv_width;i++)
			uv_temp[i] = pBuf[i];
		fwrite(uv_temp, 1, uv_width, fp);
		//fwrite(&pBuf[i<<1], 1, 1, fp);

		pBuf += uv_stride;
	}
	delete[] uv_temp;
}

enum VC1_PROPID_CB
{
	VC1_PROPID_CB_CHANGE_RESOLUTION = 1,
};

HRESULT CDXVATestProgram::GetParamFromHVDTest(DWORD dwPropID, PVOID pvContext, LPVOID *ppOutBuffer, DWORD *pdwOutBufferLen, LPVOID pInBuffer, DWORD dwInBufferLen)
{
	HRESULT hr = S_OK;
	CDXVATestProgram *pd;
	pd = reinterpret_cast<CDXVATestProgram *>(pvContext);
	if (!pd)
		return E_FAIL;

	
	HVDService::HVDDecodeConfig *pDecodeConfig = NULL;
	switch(dwPropID)
	{
	case VC1_PROPID_CB_CHANGE_RESOLUTION:
		
		if (dwInBufferLen != sizeof(HVDService::HVDDecodeConfig))
			return E_FAIL;
		else
			pDecodeConfig = reinterpret_cast<HVDService::HVDDecodeConfig*>(pInBuffer);

		
		hr = pd->m_pService->StartService(pDecodeConfig);
		
		break;
	default:
		break;
	}

	return S_OK;
}


CH264DXVATestProgram::CH264DXVATestProgram()
{
	_tcscpy_s(m_tcKeyName, 50, _T("H264_REG_KEY"));
}
CH264DXVATestProgram::~CH264DXVATestProgram()
{

}

CMP2DXVATestProgram::CMP2DXVATestProgram()
{
	read_fp = write_fp = NULL;
	_tcscpy_s(m_tcKeyName, 50, _T("MP2V_REG_KEY"));
}

CMP2DXVATestProgram::~CMP2DXVATestProgram()
{

}

CVC1DXVATestProgram::CVC1DXVATestProgram()
{
	read_fp = write_fp = NULL;
	_tcscpy_s(m_tcKeyName, 50, _T("VC1_REG_KEY"));
}

CVC1DXVATestProgram::~CVC1DXVATestProgram()
{

}

CMP4DXVATestProgram::CMP4DXVATestProgram()
{
    read_fp = write_fp = NULL;
    _tcscpy_s(m_tcKeyName, 50, _T("MP4V_REG_KEY"));
}

CMP4DXVATestProgram::~CMP4DXVATestProgram()
{

}

HRESULT CMP2DXVATestProgram::GetDataFromStream(
	IN PVOID pvContext,
	OUT const BYTE **ppbOutBuffer,
	OUT DWORD *pcbNumberOfBytesRead,
	OUT BOOL *pbHasPTS,
	OUT MP2V_TS *pTimeStamp
	)
{
	*pcbNumberOfBytesRead = fread((void *)*ppbOutBuffer, 1, STREAM_PACKET, (FILE *)pvContext);

	if(*pbHasPTS)
		*pbHasPTS = FALSE;

	return S_OK;
}

int CMP2DXVATestProgram::MP2DecodeGMO(int DXVASelect)
{
	HRESULT hr = S_OK;
	BOOL bReadEnd = FALSE;
	DWORD dwInputStatus = 0;
	long lRecordTime =0, lDecodeTime=0, lTotalTime=0;
	GMO_OUTPUT_DATA_BUFFER gmoDecoderOutputBuf={0};
	DWORD dwStatus = 0;
	BYTE *pbDecoderOutBuf = NULL;
	DWORD cbDecoderOutput = 0;
	DWORD cbOutputSize=0 , nAlignment=1;
	#if defined(_ENABLE_ISMPGMO_)
	int nISMPCnt = 0, cbISMP = 0;
	#endif

	MP2VDecMP_Frame MP2VFrame;
	BYTE *pbData = NULL;
	DWORD cbData;
	DWORD dwCBReadLength;
	BOOL bHasPTS;
	MP2V_TS tsNALU;
	CGMediaBufferMgr m_GMediaBufferMgr;
	HRESULT hrRetFromGMO = S_OK;
	DWORD dwGMOOutputResult = 0, dwInputBufferState = E_DEC_BUFFER_STATE_STOCK;

	dwFrameCounter = 0;
	while(dwFrameCounter != dwNumberOfFrames)
	{
		CGMOPtr<IGMediaBuffer> pDecoderOutputBuf;
		cbOutputSize = sizeof(MP2VDecMP_Frame);
		if(FAILED(m_GMediaBufferMgr.GetGMediaBuffer(&pDecoderOutputBuf, cbOutputSize, nAlignment)))
			return 0;
		gmoDecoderOutputBuf.pBuffer = pDecoderOutputBuf;

		lRecordTime	= GetTickCount();
		do
		{
			dwGMOOutputResult = m_pDecGMO->ProcessOutput(bReadEnd ? GMO_PROCESS_OUTPUT_DRAIN_WHEN_NO_INPUT:0, 1, &gmoDecoderOutputBuf, &dwStatus);
			int iDropPolicy = 0;
			m_pIParam->SetParameter((void*)&iDropPolicy, VDO_PARAM_SET_DECODER_HURRYUP);

			if(dwGMOOutputResult==S_OK)
			{
				lDecodeTime = GetTickCount() - lRecordTime;
				lTotalTime += lDecodeTime;

				DP("Frame[%4d]  DecTime: %3d msec, Total: %d mse, AverageTime: %.2f\n", 
					dwFrameCounter+1, lDecodeTime,  lTotalTime, (double)((double)lTotalTime/(double)(dwFrameCounter+1)));

				pDecoderOutputBuf->GetBufferAndLength(&pbDecoderOutBuf, &cbDecoderOutput);
	#if defined(_ENABLE_ISMPGMO_)
				cbISMP = m_pISMP->DescrambleGMediaBuffer(pDecoderOutputBuf);

				if(cbISMP != TRUE)
					return E_FAIL;
	#endif
				if(cbDecoderOutput>0)
				{
					memcpy(&MP2VFrame, pbDecoderOutBuf, cbDecoderOutput);

					if (write_fp)
					{
						if(DXVASelect==1)
						{	//DXVA1 
							IHVDServiceDxva1* pIHVDServiceDxva1;
							HVDService::HVDDxvaCompBufLockInfo pInfo;
							DWORD dwType=-1;
							BOOL bRead=1;
							m_pService->QueryInterface(__uuidof(IHVDServiceDxva1), (void**)&pIHVDServiceDxva1);

							pIHVDServiceDxva1->LockCompressBuffer(dwType,MP2VFrame.frame_index,&pInfo,bRead);

							byte* pYRawdata = (byte*)pInfo.pBuffer;
							DWORD FourCC;

							pIHVDServiceDxva1->GetUncompresesdBufferFormat(&FourCC);

							if(FourCC == MAKEFOURCC('N','V','1','2'))
							{
								if(dwFrameCounter > 0)
								{	
									int y_height = (((MP2VFrame.adwHeight[0]+15)>>4)<<4);
									byte* pUVRawdata = pYRawdata+y_height*pInfo.lStride; 
									NV12WriteFrame(pYRawdata,pUVRawdata,MP2VFrame.adwWidth[0],MP2VFrame.adwHeight[0],pInfo.lStride,write_fp);
								}	
							}

							pIHVDServiceDxva1->UnlockCompressBuffer(dwType,MP2VFrame.frame_index);

						}
						else if(DXVASelect==2)
						{
							//DXVA2 
							IHVDServiceDxva2* pIHVDServiceDxva2;
							HVDService::HVDDxva2UncompBufLockInfo pInfo;
							m_pService->QueryInterface(__uuidof(IHVDServiceDxva2), (void**)&pIHVDServiceDxva2);

							pIHVDServiceDxva2->LockUncompressedBuffer(MP2VFrame.frame_index, &pInfo);
							byte* pYRawdata = (byte*)pInfo.pBits;
							DWORD FourCC;
							pIHVDServiceDxva2->GetUncompresesdBufferFormat(&FourCC);
							if(FourCC == MAKEFOURCC('N','V','1','2'))
							{
								if(dwFrameCounter > 0)
								{
									int y_height = (((MP2VFrame.adwHeight[0]+15)>>4)<<4);
									byte* pUVRawdata = pYRawdata+y_height*pInfo.uPitch; 
									NV12WriteFrame(pYRawdata,pUVRawdata,MP2VFrame.adwWidth[0],MP2VFrame.adwHeight[0],pInfo.uPitch,write_fp);
								}
							}

							pIHVDServiceDxva2->UnlockUncompressedBuffer(MP2VFrame.frame_index);
						}
						else
						{	//DisableDXVA 
							if(dwFrameCounter > 0)
							{
								byte* pYRawdata = (byte *)MP2VFrame.apbFrame[0];
								byte* pUVRawdata = (byte *)MP2VFrame.apbFrame[1];
								NV12WriteFrame(pYRawdata,pUVRawdata,MP2VFrame.adwWidth[0],MP2VFrame.adwHeight[0],MP2VFrame.adwStride[0],write_fp);
							}
						}
						
					}
					m_pIParam->SetParameter((void *)MP2VFrame.dwCookie, VDO_PARAM_OPERATION_RELEASEFRAME);
					dwFrameCounter++;
				}
				else
				{
					DP("Frame Drop\n");
					m_pIParam->SetParameter((void *)MP2VFrame.dwCookie, VDO_PARAM_OPERATION_RELEASEFRAME);
				}

				lRecordTime = GetTickCount();
			}
			else if(bReadEnd)
				return -1;

			if (gmoDecoderOutputBuf.dwStatus & GMO_OUTPUT_DATA_BUFFERF_INCOMPLETE)
				break;

			CGMOPtr<IGMediaBuffer> pDecoderInputBuf;
			hr = m_pDecGMO->GetInputStatus(0, &dwInputStatus);
			if (SUCCEEDED(hr) && (dwInputStatus == GMO_INPUT_STATUSF_ACCEPT_DATA))
			{
				if(FAILED(m_GMediaBufferMgr.GetGMediaBuffer(&pDecoderInputBuf, STREAM_PACKET)))
					return 0;
				pDecoderInputBuf->GetBufferAndLength(&pbData, &cbData);

				if(!feof(read_fp))
				{
					GetDataFromStream(read_fp, (const BYTE**)&pbData, &dwCBReadLength, &bHasPTS, &tsNALU);
					pDecoderInputBuf->SetLength(dwCBReadLength);
				}
				else
				{
					bReadEnd = TRUE;
					dwInputBufferState = E_DEC_BUFFER_STATE_EOF;
					pDecoderInputBuf->SetLength(0);
				}
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
			}
		}while(1);
		//}while(((dwGMOOutputResult==S_OK)|| !(dwInputBufferState==E_DEC_BUFFER_STATE_EOF)) && dwFrameCounter <= dwNumberOfFrames);
	}
	return 0;

}

void CMP2DXVATestProgram::MP2Decoder(int uiClipCounter)
{
	HRESULT hr = E_FAIL;
	TCHAR pInputFile[256];
	TCHAR pOutputFile[256];
	int DXVASelect = 0;
	TCHAR InputName[256];
	TCHAR OutputName[256];

	if(uiClipCounter == 1)
	{
		HMODULE hDecoderModule = LoadLibrary(_T("MP2VDec.dll"));
		typedef void* (STDAPICALLTYPE * LPFNDLLCREATECOMPONENT)(void);
		LPFNDLLCREATECOMPONENT lpDllCreateComponent = reinterpret_cast<LPFNDLLCREATECOMPONENT>(GetProcAddress(hDecoderModule, "CreateComponent"));
		if ( lpDllCreateComponent )
			m_pDecGMO = reinterpret_cast<IGMediaObject*>(lpDllCreateComponent());
		
		if (m_pDecGMO)
			hr = m_pDecGMO->QueryInterface(IID_IVdoParameter, reinterpret_cast<void **>(&m_pIParam));
	}
#if defined(_ENABLE_ISMPGMO_)
	m_pDecGMO->QueryInterface(IID_ISMPGMO, reinterpret_cast<void **>(&g_pISMPGMO));
	if(g_pISMPGMO)
	{
		m_pISMP = new ISMPGMOBridge();
		HRESULT hr = g_pISMPGMO->SPConnect(m_pISMP);
		if(FAILED(hr))
			return;
	}
	else
	{
		printf("Can't create ISMP\n");
		return;
	}
#endif

	dwNumberOfFrames = GetPrivateProfileInt(_T("MP2V_REG_KEY"), _T("TOTALDECODEFRAME"), 300, m_tcConfigFile);

	TCHAR sClipName[256];
	_stprintf_s(sClipName, 256, _T("CLIP%d"), uiClipCounter);

	GetPrivateProfileString(m_tcKeyName, _T("INPUTDIR"), _T("D:\\"), pInputFile, 256, m_tcConfigFile);
	GetPrivateProfileString(m_tcKeyName, _T("OUTPUTDIR"), _T("D:\\"), pOutputFile, 256, m_tcConfigFile);

	GetPrivateProfileString(sClipName, _T("INPUTFILENAME"), _T("123.m2v"), InputName, 256, m_tcConfigFile);
	GetPrivateProfileString(sClipName, _T("OUTPUTFILENAME"), _T("123.yuv"), OutputName, 256, m_tcConfigFile);

	_tcscat_s(pInputFile, 256, InputName);
	_tcscat_s(pOutputFile, 256, OutputName);


	if(GetPrivateProfileInt(_T("MP2V_REG_KEY"), _T("NEEDOUTPUTYUVFILE"), 1, m_tcConfigFile))
		_tfopen_s(&write_fp, pOutputFile, _T("wb"));

	_tfopen_s(&read_fp, pInputFile, _T("rb"));

	MP2VDecMP_OpenOptions OpenOptions;
	ZeroMemory(&OpenOptions,sizeof(OpenOptions));
	OpenOptions.dwThreads = 1;
	OpenOptions.dwThreadAffinity = 0;
	OpenOptions.dwBuffers = 12;	// gpidisplay has the actual buffers
	OpenOptions.pfnDataCallback = NULL;
	OpenOptions.pvDataContext = this;
	OpenOptions.pfnSkipFrameCallback = NULL;
	OpenOptions.pvSkipFrameContext = this;

	OpenOptions.m_dwVendorID = NULL;//m_dwVendorID;
	OpenOptions.m_dwDeviceID = NULL;//m_dwDeviceId;
	OpenOptions.pdxvaConfigPictureDecode = NULL;
	OpenOptions.pdxvaAlphaLoadConfig = NULL;
	OpenOptions.pdxvaAlphaCombConfig = NULL;
	OpenOptions.pdxvaConfigEncryption = NULL;//m_pEncrypt;
	OpenOptions.pdxvaGeneralInfo = NULL;
	//OpenOptions.pAccel.pDxva = CGPIvideodecoder::GetDxvaFromUnknown(m_DxvaVer, disp->GetVideoAccel());
	OpenOptions.pdisp_instance = NULL;
	OpenOptions.pfnDXVA_BeginFrame = NULL;
	OpenOptions.pfnDXVA_EndFrame = NULL;
	OpenOptions.pfnDXVA_DrawSetParameters = NULL;
	OpenOptions.pfnDXVA_DrawGetResolutionCallback = DXVA_Get_Resolution;

	if (GetPrivateProfileInt(_T("MP2V_REG_KEY"), _T("DXVASELECT"), 0, m_tcConfigFile)==1)
	{
		OpenOptions.pAccel.dxvaVer = IviDxva1;
		DXVASelect = 1;
		m_pIParam->SetParameter((void*)m_pService, VDO_PARAM_SET_HVDSERVICE);
	}
	else if(GetPrivateProfileInt(_T("MP2V_REG_KEY"), _T("DXVASELECT"), 0, m_tcConfigFile)==2)
	{
		OpenOptions.pAccel.dxvaVer = IviDxva2;
		DXVASelect = 2;
		m_pIParam->SetParameter((void*)m_pService, VDO_PARAM_SET_HVDSERVICE);
	}
	else
		OpenOptions.pAccel.dxvaVer = IviNotDxva;

	/*
	OpenOptions.pbVobuBegin = &m_bVobuBegin;
	OpenOptions.pbSurfaceLost = &m_bSurfaceLost;
	OpenOptions.ppLine21 = &m_pline21;
	OpenOptions.pcsLine21 = &m_csLine21;
	OpenOptions.pbLine21 = &m_bLine21;
	OpenOptions.pbIsValidSpeedLine21 = &m_bIsValidSpeedLine21;
	OpenOptions.pSeqHdr = m_pSeqHeader;
	OpenOptions.iSeqHdrSize = m_iSeqHeaderSize;
	OpenOptions.dwDXVAERR = GetRegInt(TEXT("DXVAERR"), 0);
	OpenOptions.dwEncryptForced = GetRegInt(TEXT("FORCECSS"), 0);
	OpenOptions.bSingleSliceDecode = GetRegInt("DXVASINGLESLICE", IsWin2KCompatible(UTIL_GetOS()));
	*/

	m_pIParam->SetParameter((void*)&OpenOptions, VDO_PARAM_SET_OPEN_OPTION);
	

	GMediaObjectHelper::CGMO_MEDIA_TYPE tmpGMT;
	GMediaObjectHelper::MoInitMediaType(&tmpGMT, 8192);
	tmpGMT.majortype = MEDIATYPE_Video;
	tmpGMT.subtype = MEDIASUBTYPE_MPEG2_VIDEO;
	m_pDecGMO->SetInputType(0, &tmpGMT, 0);

	tmpGMT.subtype = MEDIASUBTYPE_IVIMP2V;
	tmpGMT.formattype = FORMAT_VideoInfo;
	tmpGMT.cbFormat= sizeof(MP2VDecMP_Frame);
	m_pDecGMO->SetOutputType(0, &tmpGMT, 0);

	char pTempString[256]; 
	USES_CONVERSION;
	sprintf(pTempString, "CLIP%d: %s start to decode !\r\n", uiClipCounter, T2A(pInputFile));
	fwrite(pTempString, strlen(pTempString), 1, m_fpDecodeLog);

	DWORD dwTime = GetTickCount();
	MP2DecodeGMO(DXVASelect);
	dwTime = GetTickCount()-dwTime;

	sprintf(pTempString, "The total frame number is %5d; Total msec: %5d; FPS: %5.2f!\r\n\r\n", dwFrameCounter, dwTime, (double)(((double)dwFrameCounter*1000.0)/(double)dwTime));
	fwrite(pTempString, strlen(pTempString), 1, m_fpDecodeLog);

	if(read_fp)
		fclose(read_fp);
	if(write_fp)
		fclose(write_fp);

	if (m_pDecGMO)
	{
		m_pDecGMO->Flush();
		m_pDecGMO->FreeStreamingResources();
	}

#if defined(_ENABLE_ISMPGMO_)
	if(m_pISMP)
	{
		delete m_pISMP;
		m_pISMP = NULL;
	}
#endif
}

HRESULT CH264DXVATestProgram::H264HP_GetDataGMO(
	IN PVOID pvContext,
	OUT const BYTE **ppbOutBuffer,
	OUT DWORD *pcbNumberOfBytesRead,
	OUT BOOL *pbHasPTS,
	OUT H264VDecParam::H264_TS *pTimeStamp
	)
{
	int read; 
	static int bLastSlice = 0;

	read = (int)fread((void *)*ppbOutBuffer,1,PACKET_SIZE,(FILE *)pvContext);

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

			bLastSlice = 0;
			return -1;
		}		
	}

	*pcbNumberOfBytesRead = read;
	if(*pbHasPTS)
		*pbHasPTS = FALSE;
	return S_OK;
}

int CH264DXVATestProgram::H264DecodeGMO(int DXVASelect)
{
	BOOL bReadEnd = FALSE;
	HRESULT hr = S_OK;
	GMO_OUTPUT_DATA_BUFFER gmoDecoderOutputBuf={0};
	DWORD	dwStatus = 0;
	BYTE *pbDecoderOutBuf = NULL;
	DWORD cbDecoderOutput = 0;
	DWORD cbOutputSize=0 , nAlignment=1;
#if defined(_ENABLE_ISMPGMO_)
	int nISMPCnt = 0, cbISMP = 0;
#endif

	const DWORD m_nDecoderReadInSize = PACKET_SIZE;
	BYTE *pbData = NULL;
	DWORD cbData;
	DWORD dwCBReadLength;
	BOOL bHasPTS;
	H264VDecParam::H264_TS tsNALU;

	CGMediaBufferMgr m_GMediaBufferMgr;

	m_pDecGMO->GetOutputSizeInfo(0, &cbOutputSize, &nAlignment);

	BOOL bAccepted = TRUE;
	DWORD dwInputStatus;

	while(dwFrameCounter != dwNumberOfFrames)
	{
		CGMOPtr<IGMediaBuffer> pDecoderOutputBuf;
		if(FAILED(m_GMediaBufferMgr.GetGMediaBuffer(&pDecoderOutputBuf, cbOutputSize, nAlignment)))
			return 0;
		gmoDecoderOutputBuf.pBuffer = pDecoderOutputBuf;

		do 
		{
			m_pDecGMO->ProcessOutput(bReadEnd ? GMO_PROCESS_OUTPUT_DRAIN_WHEN_NO_INPUT:0, 1, &gmoDecoderOutputBuf, &dwStatus);
			pDecoderOutputBuf->GetBufferAndLength(&pbDecoderOutBuf, &cbDecoderOutput);
			if ( cbDecoderOutput > 0 )
			{
#if defined(_ENABLE_ISMPGMO_)
				cbISMP = m_pISMP->DescrambleGMediaBuffer(pDecoderOutputBuf);

				if(cbISMP != TRUE)
					return E_FAIL;
#endif
				memcpy(&H264Frame, pbDecoderOutBuf, cbDecoderOutput);

				if (fp)
				{
					if((reinterpret_cast<int>(H264Frame.apbFrame[0])%16)!=0 ||
						(reinterpret_cast<int>(H264Frame.apbFrame[1])%16)!=0 )
					{
						printf("[ALARM] address is not aligned: %p %p %p\n",H264Frame.apbFrame[0],H264Frame.apbFrame[1],H264Frame.apbFrame[2]);
					}

					if(DXVASelect==1)
					{	//DXVA1
						IHVDServiceDxva1* pIHVDServiceDxva1;
						HVDService::HVDDxvaCompBufLockInfo pInfo;
						DWORD dwType=-1;
						BOOL bRead=1;
						m_pService->QueryInterface(__uuidof(IHVDServiceDxva1), (void**)&pIHVDServiceDxva1);

						pIHVDServiceDxva1->LockCompressBuffer(dwType,H264Frame.frame_index,&pInfo,bRead);

						byte* pYRawdata = (byte*)pInfo.pBuffer;
						DWORD FourCC;

						pIHVDServiceDxva1->GetUncompresesdBufferFormat(&FourCC);

						if(FourCC == MAKEFOURCC('N','V','1','2'))
						{
							int y_height = (((H264Frame.adwHeight[0]+15)>>4)<<4);
							byte* pUVRawdata = pYRawdata+y_height*pInfo.lStride; 
							//byte* pUVRawdata = pYRawdata+H264Frame.adwHeight[0]*pInfo.lStride; 
							NV12WriteFrame(pYRawdata,pUVRawdata,H264Frame.adwWidth[0],H264Frame.adwHeight[0],pInfo.lStride,fp);
						}
						else if(FourCC == MAKEFOURCC('N','V','2','4'))
						{
							int y_height=(((H264Frame.adwHeight[0]+31)>>5)<<5);
							byte* pUVRawdata = pYRawdata+y_height*pInfo.lStride; 
							NV24WriteFrame(pYRawdata,pUVRawdata,H264Frame.adwWidth[0],H264Frame.adwHeight[0],pInfo.lStride,fp);
						}

						pIHVDServiceDxva1->UnlockCompressBuffer(dwType,H264Frame.frame_index);
					}
					else if(DXVASelect==2)
					{	//DXVA2
						IHVDServiceDxva2* pIHVDServiceDxva2;
						HVDService::HVDDxva2UncompBufLockInfo pInfo;
						m_pService->QueryInterface(__uuidof(IHVDServiceDxva2), (void**)&pIHVDServiceDxva2);

						pIHVDServiceDxva2->LockUncompressedBuffer(H264Frame.frame_index, &pInfo);

						DWORD FourCC;
						byte* pYRawdata = (byte*)pInfo.pBits;
						pIHVDServiceDxva2->GetUncompresesdBufferFormat(&FourCC);
						
						byte *pBuf;
				
						if(FourCC == MAKEFOURCC('N','V','1','2'))
						{
							int y_height = (((H264Frame.adwHeight[0]+15)>>4)<<4);
							byte* pUVRawdata = pYRawdata+y_height*pInfo.uPitch; 
							//byte* pUVRawdata = pYRawdata+H264Frame.adwHeight[0]*pInfo.uPitch; 
							NV12WriteFrame(pYRawdata,pUVRawdata,H264Frame.adwWidth[0],H264Frame.adwHeight[0],pInfo.uPitch,fp);
						}
						else if(FourCC == MAKEFOURCC('N','V','2','4'))
						{
							int y_height=(((H264Frame.adwHeight[0]+31)>>5)<<5);
							byte* pUVRawdata = pYRawdata+y_height*pInfo.uPitch; 
							NV24WriteFrame(pYRawdata,pUVRawdata,H264Frame.adwWidth[0],H264Frame.adwHeight[0],pInfo.uPitch,fp);
						}
						pIHVDServiceDxva2->UnlockUncompressedBuffer(H264Frame.frame_index);
		
					}
					else
					{	//DisbaleDXVA
						byte* pYRawdata = H264Frame.apbFrame[0]+H264Frame.adwTop[0]*H264Frame.adwStride[0]+H264Frame.adwLeft[0];
						byte* pUVRawdata = H264Frame.apbFrame[1]+H264Frame.adwTop[1]*H264Frame.adwStride[1]+H264Frame.adwLeft[1];
						NV12WriteFrame(pYRawdata,pUVRawdata,H264Frame.adwWidth[0],H264Frame.adwHeight[0],H264Frame.adwStride[0],fp);
					}
					
				}

				m_pIParam->SetParameter((void *)H264Frame.dwCookie, VDO_PARAM_OPERATION_RELEASEFRAME);

				dwFrameCounter++;
				break;
			}
			else if (bReadEnd)
				return -1;

			if (gmoDecoderOutputBuf.dwStatus & GMO_OUTPUT_DATA_BUFFERF_INCOMPLETE)
				break;

			CGMOPtr<IGMediaBuffer> pDecoderInputBuf;
			if(FAILED(m_GMediaBufferMgr.GetGMediaBuffer(&pDecoderInputBuf, m_nDecoderReadInSize)))
				return 0;

			hr = m_pDecGMO->GetInputStatus(0, &dwInputStatus);
			if (SUCCEEDED(hr) && (dwInputStatus == GMO_INPUT_STATUSF_ACCEPT_DATA))
			{
				pDecoderInputBuf->GetBufferAndLength(&pbData, &cbData);
				H264HP_GetDataGMO(OpenOptions.pvDataContext, (const BYTE**)&pbData, &dwCBReadLength, &bHasPTS, &tsNALU);

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
	return 1;
}

void CH264DXVATestProgram::H264Decoder(int uiClipCounter)
{
	HRESULT ret = S_OK;
	TCHAR pInputFile[256];
	TCHAR pOutputFile[256];
	int DXVASelect = 0;
	TCHAR InputName[256];
	TCHAR OutputName[256];

	if(uiClipCounter == 1)
	{
		HMODULE hDecoderModule = LoadLibrary(_T("H264VDec.dll"));
		typedef void* (STDAPICALLTYPE * LPFNDLLCREATECOMPONENT)(void);
		LPFNDLLCREATECOMPONENT lpDllCreateComponent = reinterpret_cast<LPFNDLLCREATECOMPONENT>(GetProcAddress(hDecoderModule, "CreateComponent"));
		if ( lpDllCreateComponent )
			m_pDecGMO = reinterpret_cast<IGMediaObject*>(lpDllCreateComponent());
		if (m_pDecGMO)
			ret = m_pDecGMO->QueryInterface(IID_IVdoParameter, reinterpret_cast<void **>(&m_pIParam));
	}
#if defined(_ENABLE_ISMPGMO_)		
	m_pDecGMO->QueryInterface(IID_ISMPGMO, reinterpret_cast<void **>(&g_pISMPGMO));
	if(g_pISMPGMO)
	{
		m_pISMP = new ISMPGMOBridge();
		HRESULT hr = g_pISMPGMO->SPConnect(m_pISMP);
		if(FAILED(hr))
			return;
	}
	else
	{
		printf("Can't create ISMP\n");
		return;
	}
#endif

	fp = 0;
	dwNumberOfFrames = GetPrivateProfileInt(_T("H264_REG_KEY"), _T("TOTALDECODEFRAME"), 300, m_tcConfigFile);

	TCHAR sClipName[256];
	_stprintf_s(sClipName, 256, _T("CLIP%d"), uiClipCounter);

	

	GetPrivateProfileString(m_tcKeyName, _T("INPUTDIR"), _T("D:\\"), pInputFile, 256, m_tcConfigFile);
	GetPrivateProfileString(m_tcKeyName, _T("OUTPUTDIR"), _T("D:\\"), pOutputFile, 256, m_tcConfigFile);
	
	GetPrivateProfileString(sClipName, _T("INPUTFILENAME"), _T("123.264"), InputName, 256, m_tcConfigFile);
	GetPrivateProfileString(sClipName, _T("OUTPUTFILENAME"), _T("123.yuv"), OutputName, 256, m_tcConfigFile);

	_tcscat_s(pInputFile, 256, InputName);
	_tcscat_s(pOutputFile, 256, OutputName);
	
	if(GetPrivateProfileInt(_T("H264_REG_KEY"), _T("NEEDOUTPUTYUVFILE"), 1, m_tcConfigFile))
		_tfopen_s( &fp, pOutputFile, _T("wb"));

	ZeroMemory(&OpenOptions, sizeof(H264VDecParam::H264VDecHP_OpenOptions));
	ZeroMemory(&H264Frame, sizeof(H264VDecParam::H264VDecHP_Frame));
	ZeroMemory(&DecodeOptions,sizeof(H264VDecParam::H264VDecHP_DecodeOptions));
	ZeroMemory(&GetFrameOptions,sizeof(H264VDecParam::H264VDecHP_GetFrameOptions));

	OpenOptions.dwH264RegKey = GetPrivateProfileInt(_T("H264_REG_KEY"), _T("SMARTDEC"), 0, m_tcConfigFile);
	if(GetPrivateProfileInt(_T("H264_REG_KEY"), _T("DEBLOCKING"), 0, m_tcConfigFile))
		OpenOptions.dwH264RegKey |= H264VDecParam::H264_REG_DEBLOCKING;
	if(!GetPrivateProfileInt(_T("H264_REG_KEY"), _T("NOVIDEODROP"), 1, m_tcConfigFile))
		OpenOptions.dwH264RegKey |= H264VDecParam::H264_REG_DROP_FRAME;

	OpenOptions.pfnDataCallback = NULL;
	OpenOptions.pvDataContext   = (void *)_tfopen(pInputFile,_T("rb"));  
	OpenOptions.dwBuffers		  = 12; // display Q size.
	OpenOptions.dwFillFrameNumGap	= 0;

	/*
	if(GetRegInt("DXVA2UAB", 1))
	{
	DWORD temp = disp->GetDeviceInfo();
	if(temp==PCI_VENDOR_ID_NVIDIA)
	OpenOptions.dwH264RegKey |= H264_REG_NVFORMAT;
	else if(temp==PCI_VENDOR_ID_INTEL)
	OpenOptions.dwH264RegKey |= H264_REG_INTELFORMAT;
	}
	*/

	OpenOptions.pIviCP = NULL; 

	//OpenOptions.pAccel.pDxva = CGPIvideodecoder::GetDxvaFromUnknown(m_DxvaVer, disp->GetVideoAccel());
	OpenOptions.pdisp_instance = NULL;//this;
	OpenOptions.pfnDXVA_BeginFrame = NULL;//DXVA_BeginFrame;
	OpenOptions.pfnDXVA_EndFrame = NULL;//DXVA_EndFrame;
	OpenOptions.pfnDXVA_LockSurface = NULL;//DXVA_LockUncompSurf;
	OpenOptions.pfnDXVA_UnLockSurface = NULL;//DXVA_UnlockUncompSurf;
	OpenOptions.pfnDXVA_DrawSetParameters = NULL;
	OpenOptions.pdxvaConfigPictureDecode = NULL;//disp->GetDxvaConfigPictureDecode();
	OpenOptions.pdxvaAlphaLoadConfig = NULL;//disp->GetDxvaConfigAlphaLoad();
	OpenOptions.pdxvaAlphaCombConfig = NULL;//disp->GetDxvaConfigAlphaCombine();
	OpenOptions.pdxvaGeneralInfo = NULL;//disp->GetDxvaGeneralInfo();
	OpenOptions.pdxvaConfigEncryption = NULL;//disp->GetDxvaConfigEncryption();
	OpenOptions.pfnDXVA_DrawGetResolutionCallback = DXVA_Get_Resolution;

	if (GetPrivateProfileInt(_T("H264_REG_KEY"), _T("DXVASELECT"), 0, m_tcConfigFile)==1)
	{
		OpenOptions.pAccel.dxvaVer = IviDxva1;
		OpenOptions.uiH264VGACard = E_H264_VGACARD_NVIDIA;
		OpenOptions.uiH264DXVAMode = E_H264_DXVA_MODE_E;
		DXVASelect = 1;
		m_pIParam->SetParameter((void*)m_pService, VDO_PARAM_SET_HVDSERVICE);
	}
	else if(GetPrivateProfileInt(_T("H264_REG_KEY"), _T("DXVASELECT"), 0, m_tcConfigFile)==2)
	{
		OpenOptions.pAccel.dxvaVer = IviDxva2;
		OpenOptions.uiH264VGACard = E_H264_VGACARD_NVIDIA;
		OpenOptions.uiH264DXVAMode = E_H264_DXVA_MODE_E;
		DXVASelect = 2;
		m_pIParam->SetParameter((void*)m_pService, VDO_PARAM_SET_HVDSERVICE);
	}
	else if(GetPrivateProfileInt(_T("H264_REG_KEY"), _T("DXVASELECT"), 0, m_tcConfigFile)==3)
	{
		OpenOptions.pAccel.dxvaVer = IviDxva2;
		OpenOptions.uiH264VGACard = E_H264_VGACARD_INTEL;
		OpenOptions.uiH264DXVAMode = E_H264_DXVA_INTEL_MODE_E;
		DXVASelect = 2;
		m_pIParam->SetParameter((void*)m_pService, VDO_PARAM_SET_HVDSERVICE);
		DWORD APP_ID = H264VDecParam::MVC_3DBD;
		m_pIParam->SetParameter(&APP_ID, VDO_PARAM_SET_APP_FLAG);
	}
	else
	{
		OpenOptions.pAccel.dxvaVer = IviNotDxva;
		OpenOptions.uiH264VGACard = 0;
		OpenOptions.uiH264DXVAMode = 0;
	}


	/*
	if(m_GUID==DXVA_ModeH264_VP1)
	{
	OpenOptions.uiH264VGACard = E_H264_VGACARD_NVIDIA;
	OpenOptions.uiH264DXVAMode = E_H264_DXVA_NVIDIA_PROPRIETARY_A;
	}
	else if(m_GUID==DXVA_ModeH264_ATI_A)
	{
	OpenOptions.uiH264VGACard = E_H264_VGACARD_ATI;
	OpenOptions.uiH264DXVAMode = E_H264_DXVA_ATI_PROPRIETARY_A;
	}
	else if(m_GUID==DXVA_ATI_BA_H264)
	{
	OpenOptions.uiH264VGACard = E_H264_VGACARD_ATI;
	OpenOptions.uiH264DXVAMode = E_H264_DXVA_ATI_PROPRIETARY_E;
	}
	else
	{
	if(m_dwVendorID==PCI_VENDOR_ID_NVIDIA)
	OpenOptions.uiH264VGACard = E_H264_VGACARD_NVIDIA;
	if(m_dwVendorID==PCI_VENDOR_ID_ATI)
	OpenOptions.uiH264VGACard = E_H264_VGACARD_ATI;
	if(m_dwVendorID==PCI_VENDOR_ID_INTEL)
	OpenOptions.uiH264VGACard = E_H264_VGACARD_INTEL;
	if (m_dwVendorID==PCI_VENDOR_ID_S3)
	OpenOptions.uiH264VGACard = E_H264_VGACARD_S3;

	OpenOptions.uiH264DXVAMode = ((m_GUID==DXVA_ModeH264_A) ? E_H264_DXVA_MODE_A : OpenOptions.uiH264DXVAMode);
	OpenOptions.uiH264DXVAMode = ((m_GUID==DXVA_ModeH264_C) ? E_H264_DXVA_MODE_C : OpenOptions.uiH264DXVAMode);
	OpenOptions.uiH264DXVAMode = ((m_GUID==DXVA_ModeH264_E) ? E_H264_DXVA_MODE_E : OpenOptions.uiH264DXVAMode);
	OpenOptions.uiH264DXVAMode = ((m_GUID==DXVA_ModeH264_F) ? E_H264_DXVA_MODE_F : OpenOptions.uiH264DXVAMode);
	OpenOptions.uiH264DXVAMode = ((m_GUID==DXVADDI_Intel_ModeH264_A) ? E_H264_DXVA_MODE_A : OpenOptions.uiH264DXVAMode);
	OpenOptions.uiH264DXVAMode = ((m_GUID==DXVADDI_Intel_ModeH264_C) ? E_H264_DXVA_MODE_C : OpenOptions.uiH264DXVAMode);
	OpenOptions.uiH264DXVAMode = ((m_GUID==DXVADDI_Intel_ModeH264_E) ? E_H264_DXVA_MODE_E : OpenOptions.uiH264DXVAMode);
	}

	if(OpenOptions.uiH264DXVAMode==E_H264_DXVA_ATI_PROPRIETARY_E)
	m_iDecFrameMode = 1;
	*/

	dwFrameCounter=0;

	m_pIParam->SetParameter(&OpenOptions, VDO_PARAM_SET_OPEN_OPTION);

	GMediaObjectHelper::CGMO_MEDIA_TYPE tmpGMT;
	GMediaObjectHelper::MoInitMediaType(&tmpGMT, sizeof(H264VDecParam::H264VDecHP_Frame));
	tmpGMT.majortype = MEDIATYPE_Video;
	tmpGMT.subtype = MEDIASUBTYPE_HLH264;
	m_pDecGMO->SetInputType(0, &tmpGMT, 0);

	tmpGMT.subtype = GetPrivateProfileInt(_T("H264_REG_KEY"), _T("OUTPUTNV12"), 0, m_tcConfigFile) ? MEDIASUBTYPE_NV12 : MEDIASUBTYPE_IVIH264;
	tmpGMT.cbFormat= sizeof(H264VDecParam::H264VDecHP_Frame);
	m_pDecGMO->SetOutputType(0, &tmpGMT, 0);

	char pTempString[256]; 

	USES_CONVERSION;
	sprintf(pTempString, "CLIP%d: %s start to decode !\r\n", uiClipCounter, T2A(pInputFile));
	fwrite(pTempString, strlen(pTempString), 1, m_fpDecodeLog);

	DWORD dwTime = GetTickCount();
	H264DecodeGMO(DXVASelect);
	dwTime = GetTickCount()-dwTime;

	sprintf(pTempString, "The total frame number is %5d; Total msec: %5d; FPS: %5.2f!\r\n\r\n", dwFrameCounter, dwTime, (double)(((double)dwFrameCounter*1000.0)/(double)dwTime));
	fwrite(pTempString, strlen(pTempString), 1, m_fpDecodeLog);

	if(OpenOptions.pvDataContext)
	{
		fclose((FILE *)OpenOptions.pvDataContext);
		OpenOptions.pvDataContext = 0;
	}
	if(fp)
	{
		fclose(fp);
		fp = 0;
	}

	if (m_pDecGMO)
	{
		m_pDecGMO->Flush();
		m_pDecGMO->FreeStreamingResources();
	}

#if defined(_ENABLE_ISMPGMO_)
	if(m_pISMP)
	{
		delete m_pISMP;
		m_pISMP = NULL;
	}
#endif
}

void CVC1DXVATestProgram::ReadStream(BYTE** pbBuffer, DWORD* pdwBufferSize)
{
	if(read_fp)
		*pdwBufferSize = fread(*pbBuffer, 1, STREAM_PACKET, read_fp);
}


void CVC1DXVATestProgram::VC1Decoder(int uiClipCounter)
{
	HRESULT hr = E_FAIL;
	TCHAR pInputFile[256];
	TCHAR pOutputFile[256];
	int DXVASelect = 0;
	TCHAR InputName[256];
	TCHAR OutputName[256];

	if(uiClipCounter == 1)
	{
		HMODULE hDecoderModule = LoadLibrary(_T("VC1VDec.dll"));
		typedef void* (STDAPICALLTYPE * LPFNDLLCREATECOMPONENT)(void);
		LPFNDLLCREATECOMPONENT lpDllCreateComponent = reinterpret_cast<LPFNDLLCREATECOMPONENT>(GetProcAddress(hDecoderModule, "CreateComponent"));
		if ( lpDllCreateComponent )
			m_pDecGMO = reinterpret_cast<IGMediaObject*>(lpDllCreateComponent());

		if (m_pDecGMO)
			hr = m_pDecGMO->QueryInterface(IID_IVdoParameter, reinterpret_cast<void **>(&m_pIParam));
	}
#if defined(_ENABLE_ISMPGMO_)		
	m_pDecGMO->QueryInterface(IID_ISMPGMO, reinterpret_cast<void **>(&g_pISMPGMO));
	if(g_pISMPGMO)
	{
		m_pISMP = new ISMPGMOBridge();
		HRESULT hr = g_pISMPGMO->SPConnect(m_pISMP);
		if(FAILED(hr))
			return;
	}
	else
	{
		printf("Can't create ISMP\n");
		return;
	}
#endif

	dwNumberOfFrames = GetPrivateProfileInt(_T("VC1_REG_KEY"), _T("TOTALDECODEFRAME"), 300, m_tcConfigFile);

	TCHAR sClipName[256];
	_stprintf_s(sClipName, 256, _T("CLIP%d"), uiClipCounter);

	GetPrivateProfileString(m_tcKeyName, _T("INPUTDIR"), _T("D:\\"), pInputFile, 256, m_tcConfigFile);
	GetPrivateProfileString(m_tcKeyName, _T("OUTPUTDIR"), _T("D:\\"), pOutputFile, 256, m_tcConfigFile);

	GetPrivateProfileString(sClipName, _T("INPUTFILENAME"), _T("123.vc1"), InputName, 256, m_tcConfigFile);
	GetPrivateProfileString(sClipName, _T("OUTPUTFILENAME"), _T("123.yuv"), OutputName, 256, m_tcConfigFile);

	_tcscat_s(pInputFile, 256, InputName);
	_tcscat_s(pOutputFile, 256, OutputName);

	if(GetPrivateProfileInt(_T("VC1_REG_KEY"), _T("NEEDOUTPUTYUVFILE"), 1, m_tcConfigFile))
		_tfopen_s(&write_fp, pOutputFile, _T("wb"));

	_tfopen_s(&read_fp, pInputFile, _T("rb"));

	VC1VDecParam::VC1VDec_OpenOptions OpenOptions;
	ZeroMemory(&OpenOptions,sizeof(OpenOptions));

	OpenOptions.dwThreads = 2;
	OpenOptions.dwThreadAffinity = 0;
	OpenOptions.dwBuffers = 12;	// gpidisplay has the actual buffers

	OpenOptions.pvDataContext = this;

	//OpenOptions.pAccel.pDxva = CGPIvideodecoder::GetDxvaFromUnknown(m_DxvaVer, disp->GetVideoAccel());

	/*
	OpenOptions.DXVAOpen_Option.pdisp_instance = NULL;// this;
	OpenOptions.DXVAOpen_Option.pfnDXVA_BeginFrame = NULL;//CVC1GMOdecoder::DXVA_BeginFrame;
	OpenOptions.DXVAOpen_Option.pfnDXVA_EndFrame = NULL;//CVC1GMOdecoder::DXVA_EndFrame;
	OpenOptions.DXVAOpen_Option.pfnDXVA_DrawSetParameters = NULL;	// instate of "pfnDXVA_DrawGetResolutionCallback" 
	OpenOptions.DXVAOpen_Option.pdxvaConfigPictureDecode =NULL;// disp->GetDxvaConfigPictureDecode();
	OpenOptions.DXVAOpen_Option.pdxvaAlphaLoadConfig = NULL;//disp->GetDxvaConfigAlphaLoad();
	OpenOptions.DXVAOpen_Option.pdxvaAlphaCombConfig = NULL;//disp->GetDxvaConfigAlphaCombine();
	OpenOptions.DXVAOpen_Option.pdxvaGeneralInfo = NULL;//disp->GetDxvaGeneralInfo();
	OpenOptions.DXVAOpen_Option.pdxvaConfigEncryption = NULL;//disp->GetDxvaConfigEncryption();
	*/

	/*
	OpenOptions.dwVendorID = ((m_dwVendorID==PCI_VENDOR_ID_ATI)	? E_VC1_VGACARD_ATI : E_VC1_VGACARD_NotSupport);
	OpenOptions.dwVendorID = ((m_dwVendorID==PCI_VENDOR_ID_INTEL) ? E_VC1_VGACARD_INTEL : OpenOptions.dwVendorID);
	OpenOptions.dwVendorID = ((m_dwVendorID==PCI_VENDOR_ID_NVIDIA) ? E_VC1_VGACARD_NVIDIA : OpenOptions.dwVendorID);
	OpenOptions.dwVideoProfileID = ((m_GUID==DXVA_ModeVC1_B) ? E_VC1_DXVA_PROFILE_B : E_VC1_DXVA_PROFILE_N);
	OpenOptions.dwVideoProfileID = ((m_GUID==DXVA_ModeVC1_D) ? E_VC1_DXVA_PROFILE_D : OpenOptions.dwVideoProfileID);
	OpenOptions.dwVideoProfileID = ((m_GUID==DXVA_ModeVC1_C) ? E_VC1_DXVA_PROFILE_C : OpenOptions.dwVideoProfileID);
	OpenOptions.dwVideoProfileID = ((m_GUID==DXVA2_Intel_ModeVC1_D) ? E_INTEL_VC1_DXVA_PROFILE_D : OpenOptions.dwVideoProfileID);
	OpenOptions.dwVideoProfileID = ((m_GUID==DXVADDI_Intel_ModeVC1_D_2ndCtxt) ? E_INTEL_VC1_DXVA_PROFILE_D : OpenOptions.dwVideoProfileID);
	*/

	//OpenOptions.pfnDXVA_DrawGetResolutionCallback = DXVA_Get_Resolution;

	OpenOptions.pIviCP = NULL; 
	OpenOptions.pfnGetParamCallback = CDXVATestProgram::GetParamFromHVDTest;

	if (GetPrivateProfileInt(_T("VC1_REG_KEY"), _T("DXVASELECT"), 0, m_tcConfigFile)==1)
	{
		OpenOptions.dxvaVer = IviDxva1;
		//OpenOptions.DXVAOpen_Option.dwVendorID = E_VC1_VGACARD_NVIDIA;
		OpenOptions.dwVendorID =  E_VC1_VGACARD_NVIDIA;
		OpenOptions.dwVideoProfileID = E_VC1_DXVA_PROFILE_C;
		DXVASelect = 1;
		m_pIParam->SetParameter((void*)m_pService, VDO_PARAM_SET_HVDSERVICE);
	}
	else if (GetPrivateProfileInt(_T("VC1_REG_KEY"), _T("DXVASELECT"), 0, m_tcConfigFile)==2)
	{
		OpenOptions.dxvaVer = IviDxva2;
		//OpenOptions.DXVAOpen_Option.dwVendorID = E_VC1_VGACARD_NVIDIA;
		OpenOptions.dwVendorID =  E_VC1_VGACARD_NVIDIA;
		OpenOptions.dwVideoProfileID = E_VC1_DXVA_PROFILE_C;
		DXVASelect = 2;
		m_pIParam->SetParameter((void*)m_pService, VDO_PARAM_SET_HVDSERVICE);
	}
	else
	{
		OpenOptions.dxvaVer = IviNotDxva;
		//OpenOptions.DXVAOpen_Option.dwVendorID = 0;
		OpenOptions.dwVendorID =  0;
		OpenOptions.dwVideoProfileID = 0;
		m_pService = NULL;
	}

	m_pIParam->SetParameter((void*)&OpenOptions, VDO_PARAM_SET_OPEN_OPTION);

	dwFrameCounter=0;

	GMediaObjectHelper::CGMO_MEDIA_TYPE cInputGMediaType;
	GMediaObjectHelper::MoInitMediaType(&cInputGMediaType, STREAM_PACKET);
	GMediaObjectHelper::CGMO_MEDIA_TYPE cOutputGMediaType;

	cInputGMediaType.majortype = MEDIATYPE_Video;
	cInputGMediaType.subtype = MEDIASUBTYPE_VC1;
	cInputGMediaType.cbFormat = STREAM_PACKET;
	m_pDecGMO->SetInputType(0, &cInputGMediaType, 0);

	cOutputGMediaType.subtype = MEDIASUBTYPE_YUY2;
	cOutputGMediaType.formattype = FORMAT_VideoInfo;
	m_pDecGMO->SetOutputType(0, &cOutputGMediaType, 0);
	char pTempString[256]; 
	USES_CONVERSION;
	sprintf(pTempString, "CLIP%d: %s start to decode !\r\n", uiClipCounter, T2A(pInputFile));
	fwrite(pTempString, strlen(pTempString), 1, m_fpDecodeLog);

	DWORD dwTime = GetTickCount();
	VC1DecodeGMO(DXVASelect);
	dwTime = GetTickCount()-dwTime;

	sprintf(pTempString, "The total frame number is %5d; Total msec: %5d; FPS: %5.2f!\r\n\r\n", dwFrameCounter, dwTime, (double)(((double)dwFrameCounter*1000.0)/(double)dwTime));
	fwrite(pTempString, strlen(pTempString), 1, m_fpDecodeLog);

	if(read_fp)
		fclose(read_fp);
	if(write_fp)
		fclose(write_fp);

	if (m_pDecGMO)
	{
		m_pDecGMO->Flush();
		m_pDecGMO->FreeStreamingResources();
	}

#if defined(_ENABLE_ISMPGMO_)
	if(m_pISMP)
	{
		delete m_pISMP;
		m_pISMP = NULL;
	}
#endif
}

int CVC1DXVATestProgram::VC1DecodeGMO(int DXVASelect)
{
	BOOL bReadEnd = FALSE;
	DWORD dwInputStatus = 0;
	HRESULT hr = S_OK;
	BYTE *pbInputBuffer = NULL, *pbOutputBuffer = NULL;
	DWORD dwBufferSize=(1920 * 1088 * 3) / 2, dwAlignment=1;
	CGMediaBufferMgr cGMediaBufferMgr;
	GMO_OUTPUT_DATA_BUFFER gmoDecoderOutputBuf = {0};
	DWORD dwGMOOutputState = 0, dwGMOOutputResult = 0;
	DWORD dwTotalDecodedFrames =0;
	DWORD dwInputBufferState = E_DEC_BUFFER_STATE_STOCK;
#if defined(_ENABLE_ISMPGMO_)
	int nISMPCnt = 0, cbISMP = 0;
#endif

	while(dwTotalDecodedFrames != dwNumberOfFrames)
	{
		CGMOPtr<IGMediaBuffer> pDecoderOutputBuf;
		if(FAILED(cGMediaBufferMgr.GetGMediaBuffer(&pDecoderOutputBuf, dwBufferSize, dwAlignment)))
			return 0;
		gmoDecoderOutputBuf.pBuffer = pDecoderOutputBuf;

		do
		{
			dwGMOOutputResult = m_pDecGMO->ProcessOutput(bReadEnd ? GMO_PROCESS_OUTPUT_DRAIN_WHEN_NO_INPUT:0, 1, &gmoDecoderOutputBuf, &dwGMOOutputState);

			if(dwGMOOutputResult==S_OK)
			{
				dwTotalDecodedFrames++;

				if(dwTotalDecodedFrames < 5)
					continue;

				pDecoderOutputBuf->GetBufferAndLength(&pbOutputBuffer, &dwBufferSize);
				if(dwBufferSize>0)
				{
	#if defined(_ENABLE_ISMPGMO_)
					cbISMP = m_pISMP->DescrambleGMediaBuffer(pDecoderOutputBuf);

					if(cbISMP != TRUE)
						return E_FAIL;
	#endif
					VC1VDecParam::VC1VDecGMO_Frame sVC1VDecGMOFrame;
					memset(&sVC1VDecGMOFrame, sizeof(VC1VDecParam::VC1VDecGMO_Frame), 1);
					m_pIParam->GetParameter(&sVC1VDecGMOFrame, VDO_PARAM_GET_DISPLAY_IMG_INFORMATION);
					m_pIParam->SetParameter(&sVC1VDecGMOFrame.frame_index, VDO_PARAM_SET_DISPLAY_STATUS);

					if(dwTotalDecodedFrames > 5)
					{
						if(write_fp)
						{
							if(DXVASelect==1)
							{	//DXVA1
								IHVDServiceDxva1* pIHVDServiceDxva1;
								HVDService::HVDDxvaCompBufLockInfo pInfo;
								DWORD dwType=-1;
								BOOL bRead=1;
								m_pService->QueryInterface(__uuidof(IHVDServiceDxva1), (void**)&pIHVDServiceDxva1);

								pIHVDServiceDxva1->LockCompressBuffer(dwType,sVC1VDecGMOFrame.frame_index,&pInfo,bRead);

								byte* pYRawdata = (byte*)pInfo.pBuffer;
								DWORD FourCC;

								pIHVDServiceDxva1->GetUncompresesdBufferFormat(&FourCC);

								if(FourCC == MAKEFOURCC('N','V','2','4'))
								{
									int y_height=(((sVC1VDecGMOFrame.dwHeight[0]+31)>>5)<<5);
									byte* pUVRawdata = pYRawdata+y_height*pInfo.lStride; 
									NV24WriteFrame(pYRawdata,pUVRawdata,sVC1VDecGMOFrame.dwWidth[0],sVC1VDecGMOFrame.dwHeight[0],pInfo.lStride,write_fp);		
								}
								else if(FourCC == MAKEFOURCC('N','V','1','2'))
								{
									int y_height = (((sVC1VDecGMOFrame.dwHeight[0]+15)>>4)<<4);
									byte* pUVRawdata = pYRawdata+y_height*pInfo.lStride; 
									NV12WriteFrame(pYRawdata,pUVRawdata,sVC1VDecGMOFrame.dwWidth[0],sVC1VDecGMOFrame.dwHeight[0],pInfo.lStride,write_fp);
								}

								pIHVDServiceDxva1->UnlockCompressBuffer(dwType,sVC1VDecGMOFrame.frame_index);
							}
							else if(DXVASelect==2)
							{	//DXVA2
								IHVDServiceDxva2* pIHVDServiceDxva2;
								HVDService::HVDDxva2UncompBufLockInfo pInfo;
								m_pService->QueryInterface(__uuidof(IHVDServiceDxva2), (void**)&pIHVDServiceDxva2);

								pIHVDServiceDxva2->LockUncompressedBuffer(sVC1VDecGMOFrame.frame_index, &pInfo);

								DWORD FourCC;
								byte* pYRawdata = (byte*)pInfo.pBits;
								pIHVDServiceDxva2->GetUncompresesdBufferFormat(&FourCC);
								
								if(FourCC == MAKEFOURCC('N','V','2','4'))
								{
									int y_height=(((sVC1VDecGMOFrame.dwHeight[0]+31)>>5)<<5);
									byte* pUVRawdata = pYRawdata+y_height*pInfo.uPitch; 
									NV24WriteFrame(pYRawdata,pUVRawdata,sVC1VDecGMOFrame.dwWidth[0],sVC1VDecGMOFrame.dwHeight[0],pInfo.uPitch,write_fp);
								}
								else if(FourCC == MAKEFOURCC('N','V','1','2'))
								{
									int y_height = (((sVC1VDecGMOFrame.dwHeight[0]+15)>>4)<<4);
									byte* pUVRawdata = pYRawdata+y_height*pInfo.uPitch; 
									NV12WriteFrame(pYRawdata,pUVRawdata,sVC1VDecGMOFrame.dwWidth[0],sVC1VDecGMOFrame.dwHeight[0],pInfo.uPitch,write_fp);
								}

								pIHVDServiceDxva2->UnlockUncompressedBuffer(sVC1VDecGMOFrame.frame_index);
							}
							else
							{	//DisableDXVA
								byte* pYRawdata = (byte *)pbOutputBuffer;
								byte* pUVRawdata = (byte *)pbOutputBuffer+sVC1VDecGMOFrame.dwHeight[0]*sVC1VDecGMOFrame.dwStride[0];
								YV12WriteFrame(pYRawdata,pUVRawdata,sVC1VDecGMOFrame.dwWidth[0],sVC1VDecGMOFrame.dwHeight[0],sVC1VDecGMOFrame.dwStride[0],write_fp);
							}
						}
					}
					if (m_pService)
						m_pIParam->SetParameter((void *)sVC1VDecGMOFrame.frame_index, VDO_PARAM_OPERATION_RELEASEFRAME);
					dwFrameCounter++;
					break;
				}
			}
			else if(bReadEnd)
				return -1;

			if (gmoDecoderOutputBuf.dwStatus & GMO_OUTPUT_DATA_BUFFERF_INCOMPLETE)
				break;

			CGMOPtr<IGMediaBuffer> pDecoderInputBuf;
			hr = m_pDecGMO->GetInputStatus(0, &dwInputStatus);
			if (SUCCEEDED(hr) && (dwInputStatus == GMO_INPUT_STATUSF_ACCEPT_DATA))
			{
				if(FAILED(cGMediaBufferMgr.GetGMediaBuffer(&pDecoderInputBuf, STREAM_PACKET, 1 << 2)))
					return 0;

				if(!feof(read_fp))
				{
					dwBufferSize = STREAM_PACKET;
					pDecoderInputBuf->GetBufferAndLength(&pbInputBuffer, &dwBufferSize);
					ReadStream(&pbInputBuffer, &dwBufferSize);
					pDecoderInputBuf->SetLength(dwBufferSize);
				}
				else
				{
					bReadEnd = TRUE;
					dwInputBufferState = E_DEC_BUFFER_STATE_EOF;
					pDecoderInputBuf->SetLength(0);
				}
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
			}

		}while(1); // while(dwGMOOutputResult!=S_FALSE || dwInputBufferState!=E_DEC_BUFFER_STATE_EOF);
	}
	return S_OK;

}

void CMP4DXVATestProgram::GetDataGMO(FILE *pfInputFile, const BYTE **ppbOutBuffer, DWORD *pdwNumberOfBytesRead)
{
    int nReadSize = fread((void *)*ppbOutBuffer, 1, BITSTREAM_BUFFER_SIZE, pfInputFile);	
    *pdwNumberOfBytesRead = (nReadSize<=0) ? 0 : nReadSize;
}

int CMP4DXVATestProgram::MP4DecodeGMO(int DXVASelect)
{
    HRESULT hr = S_OK;
    BOOL bReadEnd = FALSE;

    GMO_OUTPUT_DATA_BUFFER gmoDecoderOutBuf={0};
    DWORD dwStatus = 0;
    BYTE *pbDecoderOutBuf = NULL;
    DWORD dwDecoderOutBufSize = 0;

    BYTE *pbDecoderInBuf = NULL;
    DWORD dwDecoderInBufSize;
    DWORD dwReadFileLength=0;
    DWORD dwInputStatus;

    CGMediaBufferMgr sGMediaBufferMgr;
    MP4VDecASP_Frame_ MP4VFrame;

    DWORD dwDecoderOutputBufSize=0 , nAlignment=1;

    m_pDecGMO->GetOutputSizeInfo(0, &dwDecoderOutputBufSize, &nAlignment);

    while(dwFrameCounter != dwNumberOfFrames)
    {
        CGMOPtr<IGMediaBuffer> pDecoderOutputBuf;
        if(FAILED(sGMediaBufferMgr.GetGMediaBuffer(&pDecoderOutputBuf, dwDecoderOutputBufSize, nAlignment)))
            return 0;
        gmoDecoderOutBuf.pBuffer = pDecoderOutputBuf;

        do 
        {
            m_pDecGMO->ProcessOutput(bReadEnd ? GMO_PROCESS_OUTPUT_DRAIN_WHEN_NO_INPUT:0, 1, &gmoDecoderOutBuf, &dwStatus);
            pDecoderOutputBuf->GetBufferAndLength(&pbDecoderOutBuf, &dwDecoderOutBufSize);
            if(dwDecoderOutBufSize>0)
            {
                if(write_fp)
                {
                    //Decoder MP4Decoder[1];	//We should use another structure instead of Decoder.
                    //memcpy(MP4Decoder, pbDecoderOutBuf, dwDecoderOutBufSize);
                    //fwrite(MP4Decoder->display_vop, sizeof(BYTE), MP4Decoder->frame_size, pfOutputFile );
                    if(DXVASelect == 2)
                    {
                        HRESULT Lockhr = S_OK;
                        DWORD FourCC;
                        IHVDServiceDxva2* pIHVDServiceDxva2;
                        HVDService::HVDDxva2UncompBufLockInfo pInfo;
                        m_pService->QueryInterface(__uuidof(IHVDServiceDxva2), (void**)&pIHVDServiceDxva2);

                        memcpy(&MP4VFrame, pbDecoderOutBuf, dwDecoderOutBufSize);

                        Lockhr = pIHVDServiceDxva2->LockUncompressedBuffer(MP4VFrame.nFrameIndex, &pInfo);
                        byte* pYRawdata = (byte*)pInfo.pBits;
                        DP("MPEG4 WirteOut, index = %d, pInfo.pBits = %d", MP4VFrame.nFrameIndex, pInfo.pBits);

                        pIHVDServiceDxva2->GetUncompresesdBufferFormat(&FourCC);
                        if(FourCC == MAKEFOURCC('N','V','1','2'))
                        {
                            int y_height = (((MP4VFrame.nHeight + 15) >> 4) << 4);
                            byte* pUVRawdata = pYRawdata + y_height * pInfo.uPitch; 
                            NV12WriteFrame(pYRawdata, pUVRawdata, MP4VFrame.nWidth, MP4VFrame.nHeight, pInfo.uPitch, write_fp);

                            if (dwFrameCounter == 0)
                                NV12WriteFrame(pYRawdata, pUVRawdata, MP4VFrame.nWidth, MP4VFrame.nHeight, pInfo.uPitch, write_fp);   
                        }

                        Lockhr = pIHVDServiceDxva2->UnlockUncompressedBuffer(MP4VFrame.nFrameIndex);

                        if (MP4VFrame.bCurrentFrameVOPCoded)
                            m_pIParam->SetParameter((void *)&MP4VFrame, VDO_PARAM_OPERATION_RELEASEFRAME);
                    }
                    else
                        fwrite(pbDecoderOutBuf, sizeof(BYTE), frame_size, write_fp );

                    dwFrameCounter++;
                }

                break;
            }
            else if(bReadEnd)
                return 0;

            if (gmoDecoderOutBuf.dwStatus & GMO_OUTPUT_DATA_BUFFERF_INCOMPLETE)
                break;

            CGMOPtr<IGMediaBuffer> pDecoderInputBuf;
            if(FAILED(sGMediaBufferMgr.GetGMediaBuffer(&pDecoderInputBuf, BITSTREAM_BUFFER_SIZE)))
                return 0;

            HRESULT hRet = m_pDecGMO->GetInputStatus(0, &dwInputStatus);
            if (SUCCEEDED(hRet) && (dwInputStatus == GMO_INPUT_STATUSF_ACCEPT_DATA))
            {
                pDecoderInputBuf->GetBufferAndLength(&pbDecoderInBuf, &dwDecoderInBufSize);
                if(!bReadEnd)
                    GetDataGMO(read_fp, (const BYTE**)&pbDecoderInBuf, &dwReadFileLength);

                pDecoderInputBuf->SetLength(dwReadFileLength);

                m_pDecGMO->ProcessInput(0, pDecoderInputBuf, 0, 0, 0);

                if(dwReadFileLength==0) //EOS
                {
                    bReadEnd = TRUE;
                    continue;
                }
            }
        } while(1);
    }

    return 0;
}

void CMP4DXVATestProgram::MP4Decoder(int uiClipCounter)
{
    HRESULT hr = E_FAIL;
    TCHAR pInputFile[256];
    TCHAR pOutputFile[256];
    int DXVASelect = 0;
    TCHAR InputName[256];
    TCHAR OutputName[256];

    DWORD nOutputWidth = 0;
    DWORD nOutputHeight = 0;
    DWORD nMP4Variant = 0;
    dwFrameCounter = 0;

    if(uiClipCounter == 1)
    {
        HMODULE hDecoderModule = LoadLibrary(_T("MP4VDecASP.dll"));
        typedef void* (STDAPICALLTYPE * LPFNDLLCREATECOMPONENT)(void);
        LPFNDLLCREATECOMPONENT lpDllCreateComponent = reinterpret_cast<LPFNDLLCREATECOMPONENT>(GetProcAddress(hDecoderModule, "CreateComponent"));
        if ( lpDllCreateComponent )
            m_pDecGMO = reinterpret_cast<IGMediaObject*>(lpDllCreateComponent());

        if (m_pDecGMO)
            hr = m_pDecGMO->QueryInterface(IID_IVdoParameter, reinterpret_cast<void **>(&m_pIParam));
    }
#if defined(_ENABLE_ISMPGMO_)
    m_pDecGMO->QueryInterface(IID_ISMPGMO, reinterpret_cast<void **>(&g_pISMPGMO));
    if(g_pISMPGMO)
    {
        m_pISMP = new ISMPGMOBridge();
        HRESULT hr = g_pISMPGMO->SPConnect(m_pISMP);
        if(FAILED(hr))
            return;
    }
    else
    {
        printf("Can't create ISMP\n");
        return;
    }
#endif

    dwNumberOfFrames = GetPrivateProfileInt(_T("MP4V_REG_KEY"), _T("TOTALDECODEFRAME"), 300, m_tcConfigFile);

    TCHAR sClipName[256];
    _stprintf_s(sClipName, 256, _T("CLIP%d"), uiClipCounter);

    GetPrivateProfileString(m_tcKeyName, _T("INPUTDIR"), _T("D:\\"), pInputFile, 256, m_tcConfigFile);
    GetPrivateProfileString(m_tcKeyName, _T("OUTPUTDIR"), _T("D:\\"), pOutputFile, 256, m_tcConfigFile);

    GetPrivateProfileString(sClipName, _T("INPUTFILENAME"), _T("123.m2v"), InputName, 256, m_tcConfigFile);
    GetPrivateProfileString(sClipName, _T("OUTPUTFILENAME"), _T("123.yuv"), OutputName, 256, m_tcConfigFile);

    _tcscat_s(pInputFile, 256, InputName);
    _tcscat_s(pOutputFile, 256, OutputName);


    if(GetPrivateProfileInt(_T("MP4V_REG_KEY"), _T("NEEDOUTPUTYUVFILE"), 1, m_tcConfigFile))
        _tfopen_s(&write_fp, pOutputFile, _T("wb"));

    _tfopen_s(&read_fp, pInputFile, _T("rb"));

    MP4VDecASP_OpenOptions OpenOptions;
    ZeroMemory(&OpenOptions,sizeof(OpenOptions));

    OpenOptions.uiMP4VGACard = NULL;
    OpenOptions.uiMP4DXVAMode = NULL;     
    OpenOptions.dwBuffers = 12;

    nMP4Variant = GetPrivateProfileInt(_T("MP4V_REG_KEY"), _T("MPEG4VARIANT"), 0, m_tcConfigFile);

    OpenOptions.dwMP4Variant = MPEG4_ASP_GENERIC + nMP4Variant;

    if (GetPrivateProfileInt(_T("MP4V_REG_KEY"), _T("DXVASELECT"), 0, m_tcConfigFile)==1)
    {
        OpenOptions.lDxvaVer = IviDxva1;
        DXVASelect = 1;
        m_pIParam->SetParameter((void*)m_pService, VDO_PARAM_SET_HVDSERVICE);
    }
    else if(GetPrivateProfileInt(_T("MP4V_REG_KEY"), _T("DXVASELECT"), 0, m_tcConfigFile)==2)
    {
        OpenOptions.lDxvaVer = IviDxva2;
        DXVASelect = 2;
        m_pIParam->SetParameter((void*)m_pService, VDO_PARAM_SET_HVDSERVICE);
    }
    else
    {
        OpenOptions.lDxvaVer = IviNotDxva;
    }

    if (OpenOptions.lDxvaVer != IviNotDxva)
    {
        if (GetPrivateProfileInt(_T("MP4V_REG_KEY"), _T("SUPPORT"), 0, m_tcConfigFile)==999)
        {
            OpenOptions.uiMP4VGACard = E_MP4_VGACARD_ATI;         
        }
        else if (GetPrivateProfileInt(_T("MP4V_REG_KEY"), _T("SUPPORT"), 0, m_tcConfigFile)==96)
        {
            OpenOptions.uiMP4VGACard = E_MP4_VGACARD_NVIDIA;
        }
        else
        {
            OpenOptions.lDxvaVer = IviNotDxva;
            OpenOptions.uiMP4VGACard = NULL;
        }
    }

    m_pIParam->SetParameter((void*)&OpenOptions, VDO_PARAM_SET_OPEN_OPTION);

    nOutputWidth = GetPrivateProfileInt(sClipName, _T("WIDTH"), 1920, m_tcConfigFile);
    nOutputHeight = GetPrivateProfileInt(sClipName, _T("HEIGHT"), 1080, m_tcConfigFile);
    frame_size = nOutputWidth * nOutputHeight * 3 / 2;

    GMediaObjectHelper::CGMO_MEDIA_TYPE GMType;
    GMediaObjectHelper::MoInitMediaType(&GMType, frame_size);
    GMType.majortype = MEDIATYPE_Video;
    GMType.subtype = MEDIASUBTYPE_MPEG4_VIDEO;
    m_pDecGMO->SetInputType(0, &GMType, 0);


    if (OpenOptions.lDxvaVer == IviNotDxva)
    {
        GMType.subtype = MEDIASUBTYPE_IYUV;
        GMType.cbFormat= frame_size; //sizeof(Decoder); //We should use another structure instead of Decoder.
    }
    else
    {
        GMType.subtype = MEDIASUBTYPE_IVIMP4V;
        GMType.cbFormat= sizeof(MP4VDecASP_Frame); //We should use another structure instead of Decoder.
    }
    m_pDecGMO->SetOutputType(0, &GMType, 0);

    char pTempString[256]; 
    USES_CONVERSION;
    sprintf(pTempString, "CLIP%d: %s start to decode !\r\n", uiClipCounter, T2A(pInputFile));
    fwrite(pTempString, strlen(pTempString), 1, m_fpDecodeLog);

    DWORD dwTime = GetTickCount();
    MP4DecodeGMO(DXVASelect);
    dwTime = GetTickCount()-dwTime;

    sprintf(pTempString, "The total frame number is %5d; Total msec: %5d; FPS: %5.2f!\r\n\r\n", dwFrameCounter, dwTime, (double)(((double)dwFrameCounter*1000.0)/(double)dwTime));
    fwrite(pTempString, strlen(pTempString), 1, m_fpDecodeLog);

    if(read_fp)
        fclose(read_fp);
    if(write_fp)
        fclose(write_fp);

    if (m_pDecGMO)
    {
        m_pDecGMO->Flush();
        m_pDecGMO->FreeStreamingResources();
    }

#if defined(_ENABLE_ISMPGMO_)
    if(m_pISMP)
    {
        delete m_pISMP;
        m_pISMP = NULL;
    }
#endif

}

void CMP4DXVATestProgram::NV12WriteFrame(BYTE* pbYBufferStart, BYTE* pbUVBufferStart, DWORD dwWidth, DWORD dwHeight, DWORD dwStride, FILE *fp)
{
    byte *pBuf;
    int y_height = dwHeight;
    int y_width = dwWidth;
    int uv_height = dwHeight>>1;
    int uv_width = dwWidth>>1;
    unsigned char *uv_temp = new unsigned char[uv_width];

    //Luma
    for(int lines=0;lines<y_height;lines++)
        fwrite(pbYBufferStart+lines*dwStride, 1, dwWidth, fp);

    //U
    pBuf = pbUVBufferStart;
    for(int lines=0;lines<uv_height;lines++)
    {
        for(int i=0;i<uv_width;i++)
            uv_temp[i] = pBuf[i<<1];
        fwrite(uv_temp, 1, uv_width, fp);
        //fwrite(&pBuf[i<<1], 1, sizeof(byte), write_fp);
        pBuf +=  dwStride;
    }

    //V
    pBuf = pbUVBufferStart;
    pBuf++;
    for(int lines=0;lines<uv_height;lines++)
    {
        for(int i=0;i<uv_width;i++)
            uv_temp[i] = pBuf[i<<1];
        fwrite(uv_temp, 1, uv_width, fp);
        //fwrite(&pBuf[i<<1], 1, sizeof(byte), write_fp);
        pBuf +=  dwStride;
    }
    delete[] uv_temp;
}