
#include "CPServiceS3.h"
#include "S3DXVAEncrypt.h"
#include <dxva.h>
#include <InitGuid.h> //for DEFINE_GUID

DEFINE_GUID(GUID_S3DXVA_Encrypt, 0x1ff1beD1,0xa0c7,0x11d3,0xb9,0x84,0x00,0xc0,0x4f,0x2e,0x73,0xc5);


CoCPServiceS3::CoCPServiceS3() : m_pS3Encrypt(NULL),
m_bEnhanced(FALSE), m_cdwDecoderID('0IVI'),
m_cdwS3ID('S3AC'), m_cdwEnhancedApp(0x08000000), m_cdwEnhancedDriver(0x80000000),
m_cdwEncrypHost(DXVA_ENCRYPTPROTOCOLFUNCFLAG_HOST << 8),
m_cdwEncrypAccel(DXVA_ENCRYPTPROTOCOLFUNCFLAG_ACCEL << 8)

{	
	ZeroMemory(&m_CurrContentKey, sizeof(m_CurrContentKey));
	ZeroMemory(&m_NewContentKey, sizeof(m_NewContentKey));
	ZeroMemory(&m_EncryptedContentKey, sizeof(m_EncryptedContentKey));	
}

CoCPServiceS3::~CoCPServiceS3()
{	
	Close();
}

STDMETHODIMP CoCPServiceS3::QueryInterface(REFIID riid, void**ppInterface)
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

STDMETHODIMP_(ULONG) CoCPServiceS3::AddRef()
{
	return InterlockedIncrement(&m_lRefCount);
}

STDMETHODIMP_(ULONG) CoCPServiceS3::Release()
{
	LONG lRefCount = InterlockedDecrement(&m_lRefCount);

	if (0 == lRefCount)	
		delete this;

	return lRefCount;
}


STDMETHODIMP CoCPServiceS3::Open(CPOpenOptions *pOpenOptions)
{
	HRESULT hr = E_FAIL;		

	hr = CoCPService::Open(pOpenOptions);
	if(FAILED(hr))
		return hr;

	m_pEncryptInput = new DXVA_EncryptProtocolHeader;
	m_pEncryptOutput = new DXVA_EncryptProtocolHeader;
	ZeroMemory(m_pEncryptInput, sizeof(*m_pEncryptInput));
	ZeroMemory(m_pEncryptOutput, sizeof(*m_pEncryptOutput));

	m_pEncryptInput->dwFunction = m_cdwEncrypHost;
	setDXVA_EncryptProtocolFuncFunc( &m_pEncryptInput->dwFunction, 0 ); 

	hr =  DXVA_Execute(m_pEncryptInput->dwFunction, m_pEncryptInput, sizeof(*m_pEncryptInput),
		m_pEncryptOutput, sizeof(*m_pEncryptOutput));
	if (S_OK != hr)
		return hr;
	m_pS3Encrypt = reinterpret_cast<S3DXVA_Encrypt*>(m_pEncryptOutput->ReservedBits[0]);

	if (!m_pS3Encrypt || m_pS3Encrypt->dwAcceleratorID != m_cdwS3ID)
		return E_FAIL;
	if (m_pEncryptOutput->ReservedBits[2] & m_cdwEnhancedDriver)
	{		
		S3CryptoInput s3CryptoInput;	
		s3CryptoInput.dwISVId = m_cdwDecoderID;
		s3CryptoInput.dwKeySize = 8;
		m_eKeySize = KEY_SIZE_64BIT;		

		for (int i = 0; i < 2; i++)
		{
			hr = S3VideoExtEscape(S3_CRYPTO_FUNCTION, sizeof(s3CryptoInput),
				(LPCSTR) &s3CryptoInput, s3CryptoInput.dwKeySize, (LPSTR) m_SessionKey);

			if(S3_CRYPTO_SUCCESS != hr)
			{
				m_eKeySize = KEY_SIZE_32BIT; s3CryptoInput.dwKeySize = 4;

				if(S3_CRYPTO_TOO_LARGE_KEYSIZE == hr && 0 == i)
					continue;
			}
			else 
				m_bEnhanced = FALSE;
			break;
		}
	}
	m_pEncryptInput->dwFunction = m_cdwEncrypHost;
	setDXVA_EncryptProtocolFuncFunc( &m_pEncryptInput->dwFunction, 0);
	m_pEncryptInput->ReservedBits[0]= reinterpret_cast<DWORD>(m_pS3Encrypt);
	memcpy(&m_pEncryptInput->guidEncryptProtocol, &GUID_S3DXVA_Encrypt, sizeof(GUID));    
	m_pS3Encrypt->dwDecoderID = m_cdwDecoderID;
	m_pS3Encrypt->dwAcceleratorID = m_cdwS3ID;
	m_pS3Encrypt->dwMethod = METHOD_DXVA_1;
	memcpy(&m_pEncryptOutput->ReservedBits[0], &m_pS3Encrypt, sizeof(ULONG_PTR));

	if (m_bEnhanced)
		m_pEncryptInput->ReservedBits[2] = m_cdwEnhancedApp;

	hr =  DXVA_Execute(m_pEncryptInput->dwFunction, m_pEncryptInput, sizeof(*m_pEncryptInput),
		m_pEncryptOutput, sizeof(*m_pEncryptOutput));
	if (S_OK != hr || m_bEnhanced && m_cdwEncrypAccel != m_pEncryptOutput->dwFunction)
		return E_FAIL;	

	DP("Encrypt Mode = S3");
	
	return S_OK;
}

STDMETHODIMP CoCPServiceS3::Close()
{	
	SAFE_DELETE(m_pEncryptInput); 
	SAFE_DELETE(m_pEncryptOutput);

	CoCPService::Close();
	
	return S_OK;
}

STDMETHODIMP_(BOOL) CoCPServiceS3::IsScramblingRequired(ECPFrameType eImgType)
{	
	return CoCPService::IsScramblingRequired(eImgType);
}

STDMETHODIMP CoCPServiceS3::EnableScrambling()
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

STDMETHODIMP CoCPServiceS3::DisableScrambling()
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

STDMETHODIMP CoCPServiceS3::SetDXEncryption(BOOL bOn)
{
	HRESULT hr = E_FAIL;	

	m_pEncryptInput->dwFunction = m_cdwEncrypHost;
	setDXVA_EncryptProtocolFuncFunc(&m_pEncryptInput->dwFunction, 1 );

	m_pEncryptInput->ReservedBits[0]= reinterpret_cast<DWORD>(m_pS3Encrypt);
	memcpy(&m_pEncryptInput->guidEncryptProtocol, &GUID_S3DXVA_Encrypt, sizeof(GUID));    
	m_pS3Encrypt->dwDecoderID = m_cdwDecoderID;
	m_pS3Encrypt->dwAcceleratorID = m_cdwS3ID;
	m_pS3Encrypt->dwMethod = METHOD_DXVA_1;   // dwMethod to set    	
	m_pS3Encrypt->dwValidation  = bOn ? (m_pS3Encrypt->Encrypt.dwKey ? CURR_KEY_INVALID : CURR_KEY_SET) : NO_ENCRYPTION_NEEDED;
	m_pEncryptOutput->ReservedBits[0]= reinterpret_cast<DWORD>(m_pS3Encrypt);

	if(NO_ENCRYPTION_NEEDED != m_pS3Encrypt->dwValidation)
	{
		if (m_bEnhanced)
		{
			m_pEncryptInput->ReservedBits[2] = m_cdwEnhancedApp;
			*reinterpret_cast<int*>(m_NewContentKey) = rand();
			S3EncryptKey(m_SessionKey, m_CurrContentKey, m_NewContentKey,  m_EncryptedContentKey, m_eKeySize);
			memcpy(m_pS3Encrypt->EnhancedEncrypt.Key, m_EncryptedContentKey, m_cnSessionKey << 1);
		}
		else
			m_pS3Encrypt->Encrypt.dwNewKey = rand();
	}
	hr =  DXVA_Execute(m_pEncryptInput->dwFunction, m_pEncryptInput, sizeof(*m_pEncryptInput),
		m_pEncryptOutput, sizeof(*m_pEncryptOutput));
	if(S_OK != hr)
		return E_FAIL;
	if(NO_ENCRYPTION_NEEDED != m_pS3Encrypt->dwValidation)
	{
		if(m_bEnhanced)
			m_pS3Encrypt->Encrypt.dwKey = 1;
		else
			m_pS3Encrypt->Encrypt.dwKey = m_pS3Encrypt->Encrypt.dwNewKey;
	}
	return S_OK;
}

STDMETHODIMP CoCPServiceS3::ScrambleData(BYTE *pDataOut, const BYTE *pDataIn, DWORD dwDataLen)
{	
	if(m_bEncryptOn)
	{	
		if (m_bEnhanced)
			S3EnhancedEncryptData(const_cast<BYTE*>(pDataIn), pDataOut, m_NewContentKey, dwDataLen, m_eKeySize); 
		else
			S3EncryptData((DWORD*)pDataIn, (DWORD*)pDataOut, m_pS3Encrypt->Encrypt.dwKey, dwDataLen >> 2);	
	}
	else
	{
		memcpy(pDataOut, pDataIn, dwDataLen);
	}
		
	return S_OK;
}

STDMETHODIMP_(BYTE)	CoCPServiceS3::GetEncryptMode()
{
	return m_bEnhanced;		
}

STDMETHODIMP CoCPServiceS3::ProtectedBlt(DWORD dwWidth, DWORD dwHeight, DWORD dwPitch, BYTE *pSrc)
{		
	return S_OK;
}

STDMETHODIMP_(ECPObjID)	CoCPServiceS3::GetObjID()
{
	return E_CP_ID_S3;
}