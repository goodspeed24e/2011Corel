
#include "CPServiceNvidia.h"
#include "CPCoppManager.hpp"
#include "nvscrambler.h"
#include "nvencrypt.h"
#include "aes128.h"


#define ENCRYPT_FUNC ((0xFFFF00<<8)|1)

CoCPServiceNvidia::CoCPServiceNvidia() : m_pCPCOPPMgr(NULL), m_pScrambler(NULL),
m_pAESScrambler(NULL), m_eNV_EncryptMode(E_NV_Encrypt_NONE), m_updateIVCount(0)
{		
	
}

CoCPServiceNvidia::~CoCPServiceNvidia()
{	
	Close();
}

STDMETHODIMP CoCPServiceNvidia::QueryInterface(REFIID riid, void**ppInterface)
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

STDMETHODIMP_(ULONG) CoCPServiceNvidia::AddRef()
{
	return InterlockedIncrement(&m_lRefCount);
}

STDMETHODIMP_(ULONG) CoCPServiceNvidia::Release()
{
	LONG lRefCount = InterlockedDecrement(&m_lRefCount);

	if (0 == lRefCount)	
		delete this;

	return lRefCount;
}


STDMETHODIMP CoCPServiceNvidia::Open(CPOpenOptions *pOpenOptions)
{
	HRESULT hr = E_FAIL;		
	
	hr = CoCPService::Open(pOpenOptions);
	if(FAILED(hr))
		return hr;
		
	m_pCPCOPPMgr = NULL;
	m_pScrambler = NULL;
	m_pAESScrambler = NULL;
	m_eNV_EncryptMode = E_NV_Encrypt_NONE;
	m_updateIVCount = 0;

	DXVA_EncryptProtocolHeader	EncryptHdrIn;	
	DXVA_EncryptProtocolHeader	EncryptHdrOut;	

	EncryptHdrIn.dwFunction = NV_ENCRYPT_GET_GUID;
	hr =  DXVA_Execute(ENCRYPT_FUNC, &EncryptHdrIn, sizeof(DXVA_EncryptProtocolHeader), &EncryptHdrOut, sizeof(DXVA_EncryptProtocolHeader));
	if(FAILED(hr)) 
		return hr;

	m_EncryptHdr.guidEncryptProtocol = EncryptHdrOut.guidEncryptProtocol;
	if(m_EncryptHdr.guidEncryptProtocol==Nvidia_DXVA_AES128_Encryption)
	{
		m_eNV_EncryptMode = E_NV_Encrypt_AESCTR;		
		m_pCPCOPPMgr = new CPCoppManager;		
		m_pAESScrambler = new AES128;
	}
	else
	{
		m_eNV_EncryptMode = E_NV_Encrypt_SCRAMBLE;		
		m_pScrambler = new NvScrambler;
	}	

	hr = GenerateKey();
	if(FAILED(hr)) 
		return hr;

	DP("Encrypt Mode = NVIDIA");

	return S_OK;
}

STDMETHODIMP CoCPServiceNvidia::Close()
{	
	SAFE_DELETE(m_pCPCOPPMgr);
	SAFE_DELETE(m_pScrambler);
	SAFE_DELETE(m_pAESScrambler);
		
	CoCPService::Close();

	return S_OK;
}

STDMETHODIMP_(BOOL) CoCPServiceNvidia::IsScramblingRequired(ECPFrameType eImgType)
{	
	return CoCPService::IsScramblingRequired(eImgType);
}

STDMETHODIMP CoCPServiceNvidia::EnableScrambling()
{	
	HRESULT hr = S_OK;

	if(!m_bEncryptOn)
	{
		m_bEncryptOn = TRUE;	
		//Turn on encryption in DirectX VA Driver
		hr = SetDXEncryption(TRUE);	
		if(FAILED(hr))
			DP("failed to enable scrambling!!!");
	}

	return hr;
}

STDMETHODIMP CoCPServiceNvidia::DisableScrambling()
{
	HRESULT hr = S_OK;	

	if(m_bEncryptOn)
	{
		m_bEncryptOn = FALSE;	
		//Turn off encryption in DirectX VA Driver
		hr = SetDXEncryption(FALSE);
		if(FAILED(hr))
			DP("failed to disable scrambling!!!");
	}

	return hr;
}

STDMETHODIMP CoCPServiceNvidia::ScrambleData(BYTE *pDataOut, const BYTE *pDataIn, DWORD dwDataLen)
{	
	if(m_bEncryptOn)
	{	
		if(m_updateIVCount%50 == 0)
		{
			UpdateIV();
			m_updateIVCount = 0;
		}
		else
			m_updateIVCount++;

		if(m_eNV_EncryptMode==E_NV_Encrypt_AESCTR)
			m_pAESScrambler->EncryptCTR64(pDataIn, pDataOut, dwDataLen);
		else
			m_pScrambler->Scramble(pDataIn, pDataOut, dwDataLen);	
	}
	else
	{
		memcpy(pDataOut, pDataIn, dwDataLen);
	}

	return S_OK;
}

STDMETHODIMP_(BYTE)	CoCPServiceNvidia::GetEncryptMode()
{
	return m_eNV_EncryptMode;
}


STDMETHODIMP CoCPServiceNvidia::ProtectedBlt(DWORD dwWidth, DWORD dwHeight, DWORD dwPitch, BYTE *pSrc)
{		

	return S_OK;
}

STDMETHODIMP CoCPServiceNvidia::SetDXEncryption(BOOL bOn)
{	
	DXVA_EncryptProtocolHeader EncryptHdrOut;
	
	if (bOn)
		m_EncryptHdr.dwFunction = NV_ENCRYPT_ENABLE;   
	else
		m_EncryptHdr.dwFunction = NV_ENCRYPT_DISABLE;  

	return DXVA_Execute(ENCRYPT_FUNC, &m_EncryptHdr, sizeof(DXVA_EncryptProtocolHeader), &EncryptHdrOut, sizeof(DXVA_EncryptProtocolHeader));	
}

STDMETHODIMP CoCPServiceNvidia::GenerateKey()
{
	HRESULT hr;	
	NV_EncryptSetProtectedKey	Contentkey, Sessionkey;	

	if(m_eNV_EncryptMode==E_NV_Encrypt_AESCTR)
	{
		memset(&Contentkey, 0, sizeof(NV_EncryptSetProtectedKey));
		memset(&Sessionkey, 0, sizeof(NV_EncryptSetProtectedKey));
		m_EncryptHdr.dwFunction = NV_ENCRYPT_SET_PROTECTED_KEY;		
		Contentkey.dwEncryptKeyProtectionMode = NV_ENCRYPTKEYPROTECTION_COPP;
		m_pAESScrambler->GenerateKey(Contentkey.dwKey);
		m_pCPCOPPMgr->QueryIdentifierAndSessionKey(&(Contentkey.dwEncryptKeyProtectionIdentifier), (BYTE*)(&(Sessionkey.dwKey[0])));
		m_pAESScrambler->EncryptECB((BYTE*)(&Sessionkey.dwKey[0]), (BYTE*)(&Contentkey.dwKey[0]), 16);
	}
	else
	{
		m_EncryptHdr.dwFunction = NV_ENCRYPT_SETSCRAMBLINGKEY;				
		Contentkey.dwEncryptKeyProtectionMode = NV_ENCRYPTKEYPROTECTION_NONE;
		m_pScrambler->GenerateKey(Contentkey.dwKey);
	}

	memcpy(&Contentkey.EncryptHeader, &m_EncryptHdr, sizeof(DXVA_EncryptProtocolHeader));
	hr =  DXVA_Execute(ENCRYPT_FUNC, &Contentkey, sizeof(NV_EncryptSetProtectedKey), &Sessionkey, sizeof(NV_EncryptSetProtectedKey));

	return hr;
}

STDMETHODIMP CoCPServiceNvidia::UpdateIV()
{
	HRESULT hr;
	
	NV_EncryptSetProtectedKey IVIn,IVOut;
	
	if(m_eNV_EncryptMode==E_NV_Encrypt_AESCTR)
	{
		m_pAESScrambler->UpdateIV(IVIn.dwKey);
		m_EncryptHdr.dwFunction = NV_ENCRYPT_SET_PROTECTED_IV;
		IVIn.dwEncryptKeyProtectionMode = NV_ENCRYPTKEYPROTECTION_NONE;
	}
	else
	{
		m_pScrambler->UpdateIV(IVIn.dwKey);
		m_EncryptHdr.dwFunction = NV_ENCRYPT_SETSCRAMBLINGIV;
		IVIn.dwEncryptKeyProtectionMode = NV_ENCRYPTKEYPROTECTION_NONE;
	}

	memcpy(&IVIn.EncryptHeader, &m_EncryptHdr, sizeof(DXVA_EncryptProtocolHeader));

	hr =  DXVA_Execute(ENCRYPT_FUNC, &IVIn, sizeof(NV_EncryptSetProtectedKey), &IVOut, sizeof(NV_EncryptSetProtectedKey));
	return hr;
}

STDMETHODIMP_(ECPObjID)	CoCPServiceNvidia::GetObjID()
{
	return E_CP_ID_NVIDIA;
}