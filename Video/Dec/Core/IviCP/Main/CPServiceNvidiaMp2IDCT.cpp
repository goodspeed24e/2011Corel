
#include "CPServiceNvidiaMp2IDCT.h"
#include <dxva.h>
#include "nvencrlib.h"
#include <InitGuid.h>
#include "nvencrypt.h"


#define ENCRYPT_FUNC ((0xFFFF00<<8)|1)

CoCPServiceNvidiaMp2IDCT::CoCPServiceNvidiaMp2IDCT() : m_wFrameKey(1),
m_wHostSKey(0x6e68)
{
	

}

CoCPServiceNvidiaMp2IDCT::~CoCPServiceNvidiaMp2IDCT()
{	
	Close();
}

STDMETHODIMP CoCPServiceNvidiaMp2IDCT::QueryInterface(REFIID riid, void**ppInterface)
{
	if (IID_IUnknown == riid)
	{
		*ppInterface = static_cast<IUnknown *>(static_cast<ICPServiceNvidiaMp2IDCT *>(this));
	}
	else if (IID_ICPService == riid)
	{
		*ppInterface = static_cast<ICPService *>(static_cast<ICPServiceNvidiaMp2IDCT *>(this));
	}	
	else if (IID_ICPServiceNvidiaMp2IDCT == riid)
	{
		*ppInterface = static_cast<ICPServiceNvidiaMp2IDCT *>(this);
	}
	else
	{
		*ppInterface = NULL;
		return E_NOINTERFACE;
	}

	static_cast<IUnknown *>(*ppInterface)->AddRef();

	return S_OK;
}

STDMETHODIMP_(ULONG) CoCPServiceNvidiaMp2IDCT::AddRef()
{
	return InterlockedIncrement(&m_lRefCount);
}

STDMETHODIMP_(ULONG) CoCPServiceNvidiaMp2IDCT::Release()
{
	LONG lRefCount = InterlockedDecrement(&m_lRefCount);

	if (0 == lRefCount)	
		delete this;

	return lRefCount;
}


STDMETHODIMP CoCPServiceNvidiaMp2IDCT::Open(CPOpenOptions *pOpenOptions)
{
	HRESULT hr = E_FAIL;		
		
	hr = CoCPService::Open(pOpenOptions);
	if(FAILED(hr))
		return hr;
			
	// Decoder calls drivers to pass host key, get h/w key
	m_dwH2MKey = CreateH2MKey(m_wHostSKey);

	hr = SetAuth(m_dwH2MKey, &m_dwM2HKey);	
	if(FAILED(hr))
		return hr;

	DP("Encrypt Mode = NVIDIA_MP2_IDCT");

	return S_OK;
}

STDMETHODIMP CoCPServiceNvidiaMp2IDCT::Close()
{	
	HRESULT hr;

	if(m_bEncryptOn)
	{	
		//Turn off encryption in DirectX VA Driver
		hr = SetDXEncryption(FALSE);
		if(FAILED(hr))
		{
			DP("failed to turn off encryption!!");
			return hr;
		}
		else
		{
			DP("turn off encryption!!");
			m_bEncryptOn = FALSE;
		}
	}

	CoCPService::Close();

	return S_OK;
}

STDMETHODIMP_(BOOL) CoCPServiceNvidiaMp2IDCT::IsScramblingRequired(ECPFrameType eImgType)
{	
	return CoCPService::IsScramblingRequired(eImgType);
}

STDMETHODIMP CoCPServiceNvidiaMp2IDCT::EnableScrambling()
{	
	HRESULT hr = S_OK;

	hr = PassFrameKey(m_wFrameKey);
	if(FAILED(hr))
		return hr;

	m_wFrameKey++;

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

STDMETHODIMP CoCPServiceNvidiaMp2IDCT::DisableScrambling()
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

STDMETHODIMP CoCPServiceNvidiaMp2IDCT::ScrambleData(BYTE *pDataOut, const BYTE *pDataIn, DWORD dwDataLen)
{	

	return S_OK;
}

STDMETHODIMP_(BYTE)	CoCPServiceNvidiaMp2IDCT::GetEncryptMode()
{
	return 0;
}


STDMETHODIMP CoCPServiceNvidiaMp2IDCT::ProtectedBlt(DWORD dwWidth, DWORD dwHeight, DWORD dwPitch, BYTE *pSrc)
{		

	return S_OK;
}

STDMETHODIMP CoCPServiceNvidiaMp2IDCT::UpdatelfsrIB(WORD *wPadBlock)
{
	Updatelfsr(&m_dwlfsrB, &m_dwlfsrI, wPadBlock, m_wHostSKey, m_dwM2HKey);

	return S_OK;
}

STDMETHODIMP_(WORD) CoCPServiceNvidiaMp2IDCT::ScrambleDataIDCT(WORD wPadBlock, BYTE byIndex, WORD wData)
{
	return CodingData(m_wHostSKey, (WORD)m_dwlfsrI, wPadBlock, byIndex, wData);		
}

STDMETHODIMP CoCPServiceNvidiaMp2IDCT::SetDXEncryption(BOOL bOn)
{	
	DXVA_EncryptProtocolHeader	EncryptHdrIn, EncryptHdrOut;
		
	memcpy((GUID *)&EncryptHdrIn.guidEncryptProtocol, (GUID *)&Nvidia_DXVA_IDCT_Encryption, sizeof(GUID));

	if (bOn)
		EncryptHdrIn.dwFunction = NV_ENCRYPT_ENABLE;   
	else
		EncryptHdrIn.dwFunction = NV_ENCRYPT_DISABLE;  

	return DXVA_Execute(ENCRYPT_FUNC, &EncryptHdrIn, sizeof(DXVA_EncryptProtocolHeader), &EncryptHdrOut, sizeof(DXVA_EncryptProtocolHeader));	
}


STDMETHODIMP CoCPServiceNvidiaMp2IDCT::PassFrameKey(WORD wFrameKey)
{
	HRESULT hr;
	NV_EncryptFrameKey  EncryptFrameKey;
	DXVA_EncryptProtocolHeader	EncryptResult;


	EncryptFrameKey.EncryptHeader.dwFunction = NV_ENCRYPT_FRAMEKEY;
	EncryptFrameKey.wFrameKey = m_wFrameKey;

	memcpy((GUID *)&EncryptFrameKey.EncryptHeader.guidEncryptProtocol,
		(GUID *)&Nvidia_DXVA_IDCT_Encryption,
		sizeof(GUID));

	hr =  DXVA_Execute(ENCRYPT_FUNC, &EncryptFrameKey, sizeof(EncryptFrameKey), &EncryptResult, sizeof(DXVA_EncryptProtocolHeader));
	if(FAILED(hr))
		return hr;

	//init dwlfsrB and dwlfsrI for new wFrameKey
	m_dwlfsrB = InitlfsrB(  m_wFrameKey,  m_wHostSKey,  m_dwM2HKey);
	m_dwlfsrI = InitlfsrI(  m_wFrameKey,  m_wHostSKey,  m_dwM2HKey);	

	return S_OK;
}

STDMETHODIMP CoCPServiceNvidiaMp2IDCT::SetAuth(DWORD dwH2MKey, DWORD *pdwM2HKey)
{
	HRESULT hr = E_FAIL;
	NV_EncryptAuthenticate EncryptAuthIn;
	NV_EncryptAuthenticate EncryptAuthOut;
	EncryptAuthIn.EncryptHeader.dwFunction = NV_ENCRYPT_AUTHENTICATE;
	EncryptAuthIn.dwH2MKey = dwH2MKey;
	memcpy((GUID *)&EncryptAuthIn.EncryptHeader.guidEncryptProtocol,
		   (GUID *)&Nvidia_DXVA_IDCT_Encryption,sizeof(GUID));

	hr =  DXVA_Execute(ENCRYPT_FUNC, &EncryptAuthIn, sizeof(NV_EncryptAuthenticate), &EncryptAuthOut, sizeof(NV_EncryptAuthenticate));
	if(FAILED(hr)||(EncryptAuthOut.dwM2HKey==0))
		return E_FAIL;
	
	if(InitKeys(m_wHostSKey, EncryptAuthOut.dwM2HKey)==0)
		return E_FAIL;       //not a valid M2HKey
	
	*pdwM2HKey = EncryptAuthOut.dwM2HKey;

	return S_OK;
}

STDMETHODIMP_(ECPObjID)	CoCPServiceNvidiaMp2IDCT::GetObjID()
{
	return E_CP_ID_NVIDIA_MP2IDCT;
}