
#include <InitGuid.h>
#include "CPService.h"

#include <strmif.h> // for videoaac.h
#include <videoacc.h> // for IID_IAMVideoAccelerator
#include <d3d9.h> //for dxva2api.h
#include <dxva2api.h> // for DXVA2_ModeMPEG2_VLD, IID_IDirectXVideoDecoder
#include <dxva.h> //for DXVA_ModeH264_ATI_A...
#include <stdio.h> //for _vsnprintf_s

DEFINE_GUID(DXVA2_Intel_ModeVC1_D,	0xbcc5db6d, 0xa2b6,0x4af0,0xac,0xe4,0xad,0xb1,0xf7,0x87,0xbc,0x89);
DEFINE_GUID(DXVA_ModeH264_ATI_A, 0x5cd11ee4, 0xdac9, 0x4449, 0xa0, 0x17, 0x44, 0xb, 0x36, 0xc0, 0xde, 0x3d);
DEFINE_GUID(DXVA_ModeH264_ATI_B, 0xe5de3a2, 0xc4d2, 0x4c2b, 0x85, 0xf0, 0x8c, 0x30, 0x90, 0xfd, 0xae, 0x4b);
DEFINE_GUID(DXVA_ATI_BA_H264, 0x4f3c94d, 0x2485, 0x45b3, 0x9e, 0x28, 0x4d, 0xd7, 0x4d, 0xfb, 0x6e, 0xc4);
DEFINE_GUID(DXVA_ModeH264_VP1,0x70174349, 0xd888,0x408a,0xbb,0x1d,0x9e,0x18,0x07,0x5e,0x5a,0x5a);
DEFINE_GUID(DXVA_ModeH264_AMD_MVC, 0x9901ccd3, 0xca12, 0x4b7e, 0x86, 0x7a, 0xe2, 0x22, 0x3d, 0x92, 0x55, 0xc3);
DEFINE_GUID(DXVADDI_Intel_ModeH264_E, 0x604f8e68, 0x4951, 0x4c54, 0x88, 0xfe, 0xab, 0xd2, 0x5c, 0x15, 0xb3, 0xd6);
DEFINE_GUID(DXVADDI_Intel_ModeH264_A, 0x604f8e64, 0x4951, 0x4c54, 0x88, 0xfe, 0xab, 0xd2, 0x5c, 0x15, 0xb3, 0xd6);
DEFINE_GUID(DXVADDI_Intel_ModeH264_C, 0x604f8e66, 0x4951, 0x4c54, 0x88, 0xfe, 0xab, 0xd2, 0x5c, 0x15, 0xb3, 0xd6);

struct DecGuidPair
{
	const GUID *guid;
	char *name;
};

static const DecGuidPair DecProfileList[] =
{	
	{&DXVA_ModeMPEG1_A,			"DXVA_ModeMPEG1_A"},
	{&DXVA_ModeMPEG2_A,			"DXVA_ModeMPEG2_A"},
	{&DXVA_ModeMPEG2_B,			"DXVA_ModeMPEG2_B"},
	{&DXVA_ModeMPEG2_C,			"DXVA_ModeMPEG2_C"},
	{&DXVA_ModeMPEG2_D,			"DXVA_ModeMPEG2_D"},
	{&DXVA2_ModeMPEG2_VLD,		"DXVA2_ModeMPEG2_VLD"},
	{&DXVA2_ModeMPEG2_IDCT,		"DXVA2_ModeMPEG2_IDCT"},
	{&DXVA2_ModeMPEG2_MOCOMP,	"DXVA2_ModeMPEG2_MOCOMP"},		
	{&DXVA_ModeVC1_D,			"DXVA_ModeVC1_D"},
	{&DXVA_ModeVC1_C,			"DXVA_ModeVC1_C"},
	{&DXVA_ModeVC1_B,			"DXVA_ModeVC1_B"},	
	{&DXVA_ModeVC1_A,			"DXVA_ModeVC1_A"},	
	{&DXVA2_Intel_ModeVC1_D,	"DXVA2_Intel_ModeVC1_D"},
	{&DXVA_ModeH264_F,			"DXVA_ModeH264_F"},
	{&DXVA_ModeH264_E,			"DXVA_ModeH264_E"},	
	{&DXVA_ModeH264_C,			"DXVA2_ModeH264_C"},
	{&DXVA_ModeH264_A,			"DXVA2_ModeH264_A"},	
	{&DXVA_ModeH264_VP1,			"DXVA_ModeH264_VP1"},
	{&DXVA_ModeH264_ATI_A,		"DXVA_ModeH264_ATI_A"},	
	{&DXVA_ModeH264_ATI_B,		"DXVA_ModeH264_ATI_B"},	
	{&DXVA_ATI_BA_H264,			"DXVA_ATI_BA_H264"},
	{&DXVADDI_Intel_ModeH264_E,	"DXVADDI_Intel_ModeH264_E"},	
	{&DXVADDI_Intel_ModeH264_C,	"DXVADDI_Intel_ModeH264_C"},
	{&DXVADDI_Intel_ModeH264_A,	"DXVADDI_Intel_ModeH264_A"},
	{&DXVA2_ModeWMV8_A,			"DXVA2_ModeWMV8_A"},	
	{&DXVA2_ModeWMV8_B,			"DXVA2_ModeWMV8_B"},	
	{&DXVA2_ModeWMV9_A,			"DXVA2_ModeWMV9_A"},	
	{&DXVA2_ModeWMV9_B,			"DXVA2_ModeWMV9_B"},	
	{&DXVA2_ModeWMV9_C,			"DXVA2_ModeWMV9_C"},
    {&DXVA_ModeH264_AMD_MVC,	"DXVA2_ModeH264_AMD_MVC"}
};


CoCPService::CoCPService() : m_bEncryptOn(FALSE), m_pVideoAccl(NULL), m_pDirectXVideoDec(NULL), m_lRefCount(0),
m_bEncryptIFrameOnly(TRUE), m_bShowMsg(FALSE)
{
	
}

CoCPService::~CoCPService()
{
	CoCPService::Close();
}

STDMETHODIMP CoCPService::QueryInterface(REFIID riid, void**ppInterface)
{
	if (IID_IUnknown == riid)
	{
		*ppInterface = static_cast<IUnknown *>(this);
	}
	else
	{
		*ppInterface = NULL;
		return E_NOINTERFACE;
	}
	
	static_cast<IUnknown *>(*ppInterface)->AddRef();

	return S_OK;
}

STDMETHODIMP_(ULONG) CoCPService::AddRef()
{
	return InterlockedIncrement(&m_lRefCount);
}

STDMETHODIMP_(ULONG) CoCPService::Release()
{
	LONG lRefCount = InterlockedDecrement(&m_lRefCount);
	
	if (0 == lRefCount)	
		delete this;
	
	return lRefCount;
}

STDMETHODIMP CoCPService::Open(CPOpenOptions *pOpenOptions)
{
	if(pOpenOptions->dwCPOption & E_CP_OPTION_AUDIO)
	{
		//Audio

	}
	else
	{	
		//Video
		IUnknown *pVA = pOpenOptions->pVA;
		GUID *pDecodeProfile = pOpenOptions->pDecodeProfile;

		m_bEncryptOn = FALSE;	
		m_bEncryptIFrameOnly = FALSE;
		m_bShowMsg = FALSE;
		m_pVideoAccl = NULL;
		m_pDirectXVideoDec = NULL;

		if (pOpenOptions->pVA==NULL)
			return E_INVALIDARG;

		CoCPService::Close();	

		m_bEncryptIFrameOnly = ((pOpenOptions->dwCPOption&E_CP_OPTION_I_ONLY)!=0);
		m_bShowMsg = ((pOpenOptions->dwCPOption&E_CP_OPTION_DBG_MSG)!=0);

		DP("OpenOption(): EncryptIFrameOnly=%d, ShowMsg=%d", m_bEncryptIFrameOnly, m_bShowMsg);

		//show decode profile
		if(pDecodeProfile!=NULL)
		{	
			int i, iIdx = 0, iNum = sizeof(DecProfileList)/sizeof(DecProfileList[0]);
			for(i=0; i<iNum; i++)	
			{
				if(*pDecodeProfile==*DecProfileList[i].guid)
				{
					iIdx = i;				
					break;
				}
			}

			if(i==iNum)
			{
				BYTE *pPtr = (BYTE *)pDecodeProfile;

				DP("decode profile = {%x%x%x%x,%x%x,%x%x,%x,%x,%x,%x,%x,%x,%x,%x}",
				pPtr[3],pPtr[2],pPtr[1],pPtr[0],pPtr[5],pPtr[4],pPtr[7],pPtr[6],			
				pPtr[8],pPtr[9],pPtr[10],pPtr[11],pPtr[12],pPtr[13],pPtr[14],pPtr[15]);
				
			}
			else
				DP("decode profile = %s", DecProfileList[iIdx].name);
		}

		if (S_OK != pVA->QueryInterface(IID_IAMVideoAccelerator, (void**)&m_pVideoAccl))
			if (S_OK != pVA->QueryInterface(IID_IDirectXVideoDecoder, (void**)&m_pDirectXVideoDec))
				return E_FAIL;

	}

	return S_OK;	
}

STDMETHODIMP CoCPService::Close()
{	
	m_bEncryptOn = FALSE;	
	m_bEncryptIFrameOnly = FALSE;
	m_bShowMsg = FALSE;		

	SAFE_RELEASE(m_pVideoAccl);
	SAFE_RELEASE(m_pDirectXVideoDec);

	return S_OK;
}

STDMETHODIMP_(BOOL)	CoCPService::IsScramblingRequired(ECPFrameType eImgType)
{
	if((eImgType != CP_I_TYPE) && (m_bEncryptIFrameOnly))
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

STDMETHODIMP CoCPService::EnableScrambling()
{
	if(!m_bEncryptOn)
	{
		m_bEncryptOn = TRUE;
	}

	return S_OK;
}

STDMETHODIMP CoCPService::DisableScrambling()
{
	if(m_bEncryptOn)
	{
		m_bEncryptOn = FALSE;
	}

	return S_OK;
}

STDMETHODIMP CoCPService::ScrambleData(BYTE *pDataOut, const BYTE *pDataIn, DWORD dwDataLen)
{
	return S_OK;
}

STDMETHODIMP_(BYTE)	CoCPService::GetEncryptMode()
{
	return 0;
}

STDMETHODIMP CoCPService::ProtectedBlt(DWORD dwWidth, DWORD dwHeight, DWORD dwPitch, BYTE *pSrc)
{
	return S_OK;
}

HRESULT CoCPService::DXVA_Execute(DWORD dwFunction, LPVOID lpPrivateInputData, DWORD cbPrivateInputData, LPVOID lpPrivateOutputDat, DWORD cbPrivateOutputData)
{
	if(m_pVideoAccl)
		return m_pVideoAccl->Execute(dwFunction, lpPrivateInputData, cbPrivateInputData, lpPrivateOutputDat, cbPrivateOutputData, 0, NULL);
	else if(!m_pDirectXVideoDec)
		return E_NOINTERFACE;

	DXVA2_DecodeExtensionData ExtData;
	DXVA2_DecodeExecuteParams Params = {0, NULL, &ExtData};
	Params.pExtensionData->Function = dwFunction;
	Params.pExtensionData->pPrivateInputData = lpPrivateInputData;
	Params.pExtensionData->PrivateInputDataSize = cbPrivateInputData;
	Params.pExtensionData->pPrivateOutputData = lpPrivateOutputDat;
	Params.pExtensionData->PrivateOutputDataSize = cbPrivateOutputData;

	return m_pDirectXVideoDec->Execute(&Params);
}

VOID CoCPService::DP(char* szMsg, ...)
{
	if(m_bShowMsg)
	{	
		char szFullMessage[MAX_PATH];
		char szFormatMessage[MAX_PATH];

		// format message
		va_list ap;
		va_start(ap, szMsg);
		_vsnprintf_s(szFormatMessage, MAX_PATH, _TRUNCATE, szMsg, ap);
		va_end(ap);
		strncat_s(szFormatMessage, MAX_PATH,"\n", MAX_PATH);				
		strcpy_s(szFullMessage, MAX_PATH, "[IviCP] ");
		strcat_s(szFullMessage, MAX_PATH, szFormatMessage);		
		OutputDebugStringA(szFullMessage);
	}
}

STDMETHODIMP_(ECPObjID)	CoCPService::GetObjID()
{
	return E_CP_ID_UNKNOWN;
}

