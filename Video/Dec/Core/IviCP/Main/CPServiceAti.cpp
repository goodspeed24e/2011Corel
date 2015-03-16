
#include "CPServiceAti.h"
#include "HSPLib.h" //for CATIHSP_Authentication
#include "hsp-ss.h" //for ATIDRM_SS_PARAM_RECVKS2
#include <InitGuid.h> //for DEFINE_GUID
#include <d3d9.h> //for dxva2api.h
#include <dxva2api.h> // for DXVA2_ModeMPEG2_VLD
#include <dxva.h> //for DXVA_ModeH264_ATI_A...
#include "RandomNumber16Byte.h" //for CRandomNumber16Byte
#include "aes.h" //for AesEncrypt
#include "uuids.h" //for MEDIASUBTYPE_NV12

DEFINE_GUID(DXVA_ATI_BA_H264, 0x4f3c94d, 0x2485, 0x45b3, 0x9e, 0x28, 0x4d, 0xd7, 0x4d, 0xfb, 0x6e, 0xc4);
DEFINE_GUID(DXVA_ModeH264_AMD_MVC, 0x9901ccd3, 0xca12, 0x4b7e, 0x86, 0x7a, 0xe2, 0x22, 0x3d, 0x92, 0x55, 0xc3);

#define CHECK_ATI_SCRAMBLE_SUPPORTED(id) ((id == DXVA_ATI_BA_H264) || (id == DXVA_ModeH264_E) || (id == DXVA_ModeH264_F) || (id == DXVA_ModeVC1_D) || (id == DXVA_ModeMPEG2_C) || (id == DXVA_ModeMPEG2_D) || (id == DXVA2_ModeMPEG2_IDCT) || (id == DXVA_ModeH264_AMD_MVC) || (id == DXVA2_ModeMPEG2_VLD))
#define HSPTEST_APPID	      0x00030001
#define HSPTEST_APPID_VISTA   0x00038002 //The specific vendor id is for Aero on case under Vista (PCOM for DWM)
#define SD_WIDTH		720
#define BPP				2

CoCPServiceAti::CoCPServiceAti() : m_dwCipherType(IVI_ALG_NONE),
m_bProtectedBlt(FALSE), m_dwFrameWidth(SD_WIDTH)
{	
	memset(&m_Session, 0, sizeof(m_Session));	
}

CoCPServiceAti::~CoCPServiceAti()
{	
	Close();
}

STDMETHODIMP CoCPServiceAti::QueryInterface(REFIID riid, void**ppInterface)
{
	if (IID_IUnknown == riid)
	{
		*ppInterface = static_cast<IUnknown *>(this);
	}
	else if (IID_ICPService == riid)
	{
		*ppInterface = static_cast<ICPService *>(this);
	}	
	else
	{
		*ppInterface = NULL;
		return E_NOINTERFACE;
	}

	static_cast<IUnknown *>(*ppInterface)->AddRef();

	return S_OK;
}

STDMETHODIMP_(ULONG) CoCPServiceAti::AddRef()
{
	return InterlockedIncrement(&m_lRefCount);
}

STDMETHODIMP_(ULONG) CoCPServiceAti::Release()
{
	LONG lRefCount = InterlockedDecrement(&m_lRefCount);

	if (0 == lRefCount)	
		delete this;

	return lRefCount;
}

STDMETHODIMP_(VOID) CoCPServiceAti::XOR128bits(const BYTE *pIn1, const BYTE *pIn2, BYTE *pOut)
{	
	for(int i=0; i<16; i++)
		pOut[i] = pIn1[i] ^ pIn2[i];
}

STDMETHODIMP CoCPServiceAti::TriggerContentKeySwitching(AtiSession* pSession)
{
	DWORD dwKeyIndex = 0;
	HRESULT hr = E_FAIL;
	
	hr = pSession->pAuthObj->SetNextDecryptionKey(&dwKeyIndex);
	if (FAILED(hr))
		return hr;
	
	pSession->iIndexOfCurrentKey = dwKeyIndex;
	if (m_dwCipherType == IVI_ALG_AESLITE || m_dwCipherType == IVI_ALG_AESCTR)
	{
		hr = pSession->pEncrypt->SetEnryptionKey(pSession->ContentKeys[dwKeyIndex], 
			pSession->iContentKeySize, 
			pSession->ContentKeyCounter[dwKeyIndex], 
			16);	
	}
	else
	{
		hr = pSession->pEncrypt->SetEnryptionKey(pSession->ContentKeys[dwKeyIndex], 
			pSession->iContentKeySize, 
			NULL,
			NULL);
	}

	if(SUCCEEDED(hr))
	{
		pSession->iUsedCount[dwKeyIndex]++;	
		m_dwNonUsedKeys--;
	}
	
	return hr;
}

STDMETHODIMP CoCPServiceAti::CreateNewContentKey(AtiSession* pSession)
{
	HRESULT hr = E_FAIL;
	NLibDisplay::CRandomNumber16Byte R1;
	BYTE contentkey[MAX_CONTENT_KEY_SIZE];
	BYTE encryptedcontentkey[MAX_CONTENT_KEY_SIZE];

	R1.GenerateRandomNumber();
	memcpy(contentkey,    R1.RandNumber, KEY_SIZE);
	R1.GenerateRandomNumber();
	memcpy(contentkey+KEY_SIZE, R1.RandNumber, KEY_SIZE);

	NLibDisplay::AesEncrypt(pSession->KeySession, contentkey,    encryptedcontentkey);
	NLibDisplay::AesEncrypt(pSession->KeySession, contentkey+KEY_SIZE, encryptedcontentkey+KEY_SIZE);

	DWORD dwKeyIndex = 0;
	if (m_dwCipherType == IVI_ALG_XOR)
	{
		hr = pSession->pAuthObj->AddDecryptionKey( ATIDRM_ALG_XOR,
			encryptedcontentkey, 
			pSession->iContentKeySize,
			NULL, 
			NULL, 
			&dwKeyIndex);
	}
	else if (m_dwCipherType == IVI_ALG_AESLITE || m_dwCipherType == IVI_ALG_AESCTR)
	{
		BYTE initialcounter[KEY_SIZE];
		R1.GenerateRandomNumber();
		memcpy(initialcounter, R1.RandNumber, KEY_SIZE);

		hr = pSession->pAuthObj->AddDecryptionKey(((m_dwCipherType==IVI_ALG_AESLITE) ? ATIDRM_ALG_AESLITE : ATIDRM_ALG_AESCTR),
			encryptedcontentkey,
			pSession->iContentKeySize,
			initialcounter,
			KEY_SIZE,
			&dwKeyIndex);
		
		if (SUCCEEDED(hr))
			memcpy(pSession->ContentKeyCounter[dwKeyIndex], initialcounter, KEY_SIZE);
	}

	if (SUCCEEDED(hr))
	{
		memcpy(pSession->ContentKeys[dwKeyIndex], contentkey, MAX_CONTENT_KEY_SIZE);
		pSession->iUsedCount[dwKeyIndex] = 0;
		m_dwNonUsedKeys++;
	}

	return hr;
		
}

STDMETHODIMP CoCPServiceAti::GetDeviceCaps(AtiSession* pSession, GUID *pguidDec)
{
	HRESULT hr=E_FAIL;

	//Get device capabilities
	ATIDRM_DEVICE_CAPS devicecaps;
	ZeroMemory(&devicecaps, sizeof(devicecaps));
	devicecaps.dwSize = sizeof(devicecaps);
	memset(&devicecaps.RequestID, 3, 16);
	hr = pSession->pAuthObj->GetCaps(&devicecaps);
	if (FAILED(hr))
	{
		DP("failed to get device capabilities, hr=%x", hr);
		return hr;
	}

	if(CHECK_ATI_SCRAMBLE_SUPPORTED(*pguidDec))	
	{
		if ((devicecaps.dwCipherCaps & ATIDRM_ALG_AESLITE) != 0)
			m_dwCipherType = IVI_ALG_AESLITE;
		if ((devicecaps.dwCipherCaps & ATIDRM_ALG_AESCTR) != 0)		
			m_dwCipherType = IVI_ALG_AESCTR;					
	}
	else
	{
		if ((devicecaps.dwCipherCaps & ATIDRM_ALG_XOR) != 0)
			m_dwCipherType = IVI_ALG_XOR;
	}

	if (m_dwCipherType == IVI_ALG_NONE)
		return E_FAIL;
	
	DP("Cipher Type = %s",(m_dwCipherType==IVI_ALG_AESCTR)?"AESCTR":(m_dwCipherType==IVI_ALG_AESLITE)?"ATSLITE":"XOR");

	return S_OK;
}

STDMETHODIMP CoCPServiceAti::ShareAuthProtocol(AtiSession* pSession)
{
	HRESULT hr=E_FAIL;

	// Perform Step1
	ATIDRM_SS_PARAM_RECVKS2 step1Params;
	ZeroMemory(&step1Params, sizeof (step1Params));

	// Generate R1: Ideally it should be seeded at different execution points
	NLibDisplay::CRandomNumber16Byte R1;
	R1.PrepareRandSeed1();
	R1.PrepareRandSeed2();
	R1.PrepareRandSeed3();
	R1.GenerateRandomNumber();
	memcpy(step1Params.R1, R1.RandNumber, 16);

	hr = pSession->pAuthObj->SendAuthCommand(ATIDRM_SS_CMD_RECVKS2, &step1Params, sizeof(ATIDRM_SS_PARAM_RECVKS2));	
	if (FAILED(hr))
		return hr;

	BYTE DeviceID_R2[16];
	NLibDisplay::AesDecryptKeyCommon(step1Params.e_DeviceID_R2, DeviceID_R2);

	BYTE DeviceID[16];
	XOR128bits(DeviceID_R2, step1Params.R2, DeviceID);

	// Normally we'd look up Kx1 & Kx2 using Device ID 
	// but the this test code uses fixed values for now...

	// Verify R1 & Extract Ks2 
	BYTE R1_Ks2[32];
	NLibDisplay::AesDecryptKeyKx2(step1Params.e_R1_Ks2, R1_Ks2);
	NLibDisplay::AesDecryptKeyKx2(step1Params.e_R1_Ks2+16, R1_Ks2+16);

	// Verify our R1 matches R1 inside R1_Ks2 from Device:
	if (memcmp(R1_Ks2, R1.RandNumber, 16) != 0)	
		return E_FAIL;	

	// Ks2 will be used latter to derive session key:
	BYTE KeyKs2[16];
	memcpy(KeyKs2, R1_Ks2+16, 16);

	// Perform Step 2
	ATIDRM_SS_PARAM_SENDKS1 step2Params;
	ZeroMemory(&step2Params, sizeof (step2Params));

	// Generate KeyKs1: Ideally it should be seeded at different execution points
	NLibDisplay::CRandomNumber16Byte KeyKs1;
	KeyKs1.PrepareRandSeed1();
	KeyKs1.PrepareRandSeed2();
	KeyKs1.PrepareRandSeed3();
	KeyKs1.GenerateRandomNumber();

	NLibDisplay::AesEncryptKeyKx1(step1Params.R2, step2Params.e_R2_Ks1);
	NLibDisplay::AesEncryptKeyKx1(KeyKs1.RandNumber, step2Params.e_R2_Ks1+16);

	hr = pSession->pAuthObj->SendAuthCommand(ATIDRM_SS_CMD_SENDKS1, &step2Params, sizeof (ATIDRM_SS_PARAM_SENDKS1));

	if (FAILED(hr))
		return hr;

	// Now we can use KeyKs1 & KeyKs2 to generate the session key:
	NLibDisplay::AesEncrypt(KeyKs1.RandNumber, KeyKs2, pSession->KeySession);

	return S_OK;
}

STDMETHODIMP CoCPServiceAti::SetEncryptionFormat(AtiSession* pSession)
{
	HRESULT hr=E_FAIL;
	DWORD dwContentKeySize = 0;
	BOOL bUseATIEncrypt = 0;

	hr = pSession->pEncrypt->SetEncryptionFormat(m_dwCipherType, &dwContentKeySize, &bUseATIEncrypt);
	if (FAILED(hr))
		return hr;

	pSession->iContentKeySize = static_cast<int>(dwContentKeySize);

	return S_OK;
}

STDMETHODIMP CoCPServiceAti::CreateNewSession(AtiSession* pSession, IUnknown *pVA, GUID *pguidDec)
{
	HRESULT hr = E_FAIL;

	// Create HSP Object & Open Session
	pSession->pAuthObj = new CATIHSP_Authentication;

	if (!pSession->pAuthObj)	
	{
		DP("failed to create CATIHSP_Authentication");
		return E_FAIL;	
	}

	pSession->pEncrypt = new CATIHSP_Encryption;
	if (!pSession->pEncrypt)			
	{
		DP("failed to create CATIHSP_Encryption");
		return E_FAIL;	
	}
	
    hr = pSession->pAuthObj->Open(pVA, HSPTEST_APPID);
	if (FAILED(hr))
	{
		DP("failed to CATIHSP_Authentication::Open(), hr=%x", hr);
		return hr;
	}

	hr = GetDeviceCaps(pSession, pguidDec);
	if (FAILED(hr))
	{
		DP("failed to get device capabilities, hr=%x", hr);
		return hr;
	}

	hr = ShareAuthProtocol(pSession);
	if (FAILED(hr))
	{
		DP("failed to share secret authentication protocol with graphics device, hr=%x", hr);
		return hr;
	}
	
	SetEncryptionFormat(pSession);
	if (FAILED(hr))
	{
		DP("failed to set encryption format, hr=%x", hr);
		return hr;
	}		

	return  S_OK;
}

STDMETHODIMP CoCPServiceAti::Open(CPOpenOptions *pOpenOptions)
{
	HRESULT hr = E_FAIL;		

	m_dwFrameWidth = pOpenOptions->dwFrameWidth;
	m_bProtectedBlt = pOpenOptions->bProtectedBlt;
	m_dwNonUsedKeys = 0;
	m_dwCipherType = IVI_ALG_NONE;
	memset(&m_Session, 0, sizeof(m_Session));	

	hr = CoCPService::Open(pOpenOptions);
	if(FAILED(hr))
		return hr;
	
	hr = CreateNewSession(&m_Session, pOpenOptions->pVA, pOpenOptions->pDecodeProfile);
	if (FAILED(hr))
		return hr;	
	
	// ProtectedBlt doesn't need to create content key
	if (!m_bProtectedBlt)
	{		
		for (int i=0; i<MAX_NUM_CONTENT_KEYS; i++)
		{			
			if (CreateNewContentKey(&m_Session) < 0)
				return E_FAIL;
		}
	}			

	DP("Encrypt Mode = ATI");

	return S_OK;
}

STDMETHODIMP CoCPServiceAti::Close()
{	
	if (m_Session.pAuthObj)
	{
		m_Session.pAuthObj->Close();
		SAFE_DELETE(m_Session.pAuthObj);
		SAFE_DELETE(m_Session.pEncrypt);
	}

	CoCPService::Close();
	
	return S_OK;
}

STDMETHODIMP_(BOOL) CoCPServiceAti::IsScramblingRequired(ECPFrameType eImgType)
{	
	return CoCPService::IsScramblingRequired(eImgType);
}

STDMETHODIMP CoCPServiceAti::EnableScrambling()
{
	HRESULT hr = S_OK;

	if(m_bProtectedBlt)
	{
		DP("EnableScrambling() can't be used in ProtectedBlt mode!!!");
		return E_FAIL;
	}		

	if(!m_bEncryptOn)
	{
		m_bEncryptOn = TRUE;				
	}

	if(m_dwNonUsedKeys<MAX_NUM_CONTENT_KEYS-1)
	{		
		hr = CreateNewContentKey(&m_Session);
		if(FAILED(hr))
		{
			hr = CreateNewContentKey(&m_Session);    // workaround for bug#109512, AVCREC corruption issue
			if(FAILED(hr))
			{
				DP("failed to add content key!!!");
				return hr;
			}
		}
	}

	//Use new key
	hr = TriggerContentKeySwitching(&m_Session);
	if(FAILED(hr))
	{
		DP("failed to set content key!!!");	
		return hr;
	}	

	return hr;
}

STDMETHODIMP CoCPServiceAti::DisableScrambling()
{
	HRESULT hr = S_OK;

	if(m_bProtectedBlt)
	{
		DP("DisableScrambling() can't be used in ProtectedBlt mode!!!");
		return E_FAIL;
	}

	if(m_bEncryptOn)
	{
		m_bEncryptOn = FALSE;	
		hr = m_Session.pAuthObj->SetNullDecryptionKey();
		if(FAILED(hr))
			DP("failed to disable scrambling!!!");
	}

	return hr;
}

STDMETHODIMP CoCPServiceAti::ScrambleData(BYTE *pDataOut, const BYTE *pDataIn, DWORD dwDataLen)
{
	HRESULT hr = E_FAIL;							
	
	if(m_bProtectedBlt)
	{
		DP("ScrambleData() can't be used in ProtectedBlt mode!!!");
		return E_FAIL;
	}

	if(m_bEncryptOn)
	{								
		hr = m_Session.pEncrypt->Encrypt((LPVOID)pDataIn, pDataOut, dwDataLen, (m_dwCipherType==IVI_ALG_XOR)? m_dwFrameWidth*BPP :0);
		if(FAILED(hr))
		{
			DP("failed to encrypt data, hr=%x", hr);
			return hr;
		}
	}
	else
	{
		memcpy(pDataOut, pDataIn, dwDataLen);
	}
		
	return S_OK;
}

STDMETHODIMP_(BYTE)	CoCPServiceAti::GetEncryptMode()
{
	return m_dwCipherType; 
}

// Currently, driver only support NV12 as input format.
// other format needs to convert first
STDMETHODIMP CoCPServiceAti::ProtectedBlt(DWORD dwWidth, DWORD dwHeight, DWORD dwPitch, BYTE *pSrc)
{		
	RECT rcSrc = {0, 0, dwWidth, dwHeight};
	ATIDRM_PROTBLT_PARAMS PBPARAMS;
	ZeroMemory(&PBPARAMS, sizeof(PBPARAMS));
	PBPARAMS.dwSize = sizeof(PBPARAMS);
	PBPARAMS.bltOP = ATIDRM_PROTBLT_RENDERTGT;
	PBPARAMS.srcRect = rcSrc;
	PBPARAMS.srcPitch = dwPitch;
	PBPARAMS.srcWidth = dwWidth;
	PBPARAMS.srcHeight = dwHeight;
	PBPARAMS.dstOffset = 0;	
	PBPARAMS.srcFormat = MEDIASUBTYPE_NV12;				
	memcpy(PBPARAMS.srcAddr, &pSrc, 4);
		
	NLibDisplay::AesEncrypt(m_Session.KeySession, PBPARAMS.srcAddr, PBPARAMS.srcAddr);
	HRESULT hr = m_Session.pAuthObj->ProtectedBlt(&PBPARAMS);

	return hr;
}

STDMETHODIMP_(ECPObjID)	CoCPServiceAti::GetObjID()
{
	return E_CP_ID_ATI;
}