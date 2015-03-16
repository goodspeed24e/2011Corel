
#include "CPServiceIntel.h"
//#include <InitGuid.h>
#include <d3d9.h> //for dxva2api.h
#include <dxva2api.h>
#include <dxva.h>
#include "igfxscramble.h"

CoCPServiceIntel::CoCPServiceIntel()
{	
}

CoCPServiceIntel::~CoCPServiceIntel()
{	
	Close();
}

STDMETHODIMP CoCPServiceIntel::QueryInterface(REFIID riid, void**ppInterface)
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

STDMETHODIMP_(ULONG) CoCPServiceIntel::AddRef()
{
	return InterlockedIncrement(&m_lRefCount);
}

STDMETHODIMP_(ULONG) CoCPServiceIntel::Release()
{
	LONG lRefCount = InterlockedDecrement(&m_lRefCount);

	if (0 == lRefCount)	
		delete this;

	return lRefCount;
}

STDMETHODIMP CoCPServiceIntel::Open(CPOpenOptions *pOpenOptions)
{
	HRESULT hr = E_FAIL;		

	hr = CoCPService::Open(pOpenOptions);
	if(FAILED(hr))
		return hr;

	if((*(pOpenOptions->pDecodeProfile)!=DXVA2_ModeMPEG2_MoComp) &&
		(*(pOpenOptions->pDecodeProfile)!=DXVA2_ModeMPEG2_IDCT) &&
		(*(pOpenOptions->pDecodeProfile)!=DXVA2_ModeMPEG2_VLD) &&
		(*(pOpenOptions->pDecodeProfile)!=DXVA_ModeMPEG2_B) &&
		(*(pOpenOptions->pDecodeProfile)!=DXVA_ModeMPEG2_C) &&
		(*(pOpenOptions->pDecodeProfile)!=DXVA_ModeMPEG2_D))
		return E_FAIL;

	m_guidKey = DXVA_Intel_Encryption;

	DP("Encrypt Mode = INTEL");
	
	return S_OK;
}

STDMETHODIMP CoCPServiceIntel::Close()
{	
	CoCPService::Close();
	
	return S_OK;
}

STDMETHODIMP_(BOOL) CoCPServiceIntel::IsScramblingRequired(ECPFrameType eImgType)
{	
	return CoCPService::IsScramblingRequired(eImgType);
}

STDMETHODIMP CoCPServiceIntel::EnableScrambling()
{	
	HRESULT hr = S_OK;

	if(!m_bEncryptOn)
	{		
		//Turn on encryption in DirectX VA Driver
		hr = SetDXEncryption(TRUE);	
		if(SUCCEEDED(hr))
		{			
			if(SetScrambleKey((BYTE*)&m_guidKey)!=csdStsNoErr)
			{			
				DP("failed to set scramble key!!");
				hr = SetDXEncryption(FALSE);
				if(FAILED(hr))
					return hr;				
			}

			hr = ChangeKey();
			if(FAILED(hr))
			{
				DP("failed to change key!!");					
				return hr;
			}

			m_bEncryptOn = TRUE;	
		}	
		else
		{			
			DP("failed to enable scrambling!!!");
		}
	}

	return S_OK;
}

STDMETHODIMP CoCPServiceIntel::DisableScrambling()
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

STDMETHODIMP CoCPServiceIntel::ScrambleData(BYTE *pDataOut, const BYTE *pDataIn, DWORD dwDataLen)
{		
	csdStatus status = csdStsNoErr;
	if(m_bEncryptOn)
	{	
		status = EncryptData(pDataIn, pDataOut, dwDataLen);	
	}
	else
	{
		memcpy(pDataOut, pDataIn, dwDataLen);
	}

	if(status==csdStsNoErr)
		return S_OK;
	else
		return E_FAIL;
}

STDMETHODIMP_(BYTE)	CoCPServiceIntel::GetEncryptMode()
{
	return 0; 
}

STDMETHODIMP CoCPServiceIntel::ProtectedBlt(DWORD dwWidth, DWORD dwHeight, DWORD dwPitch, BYTE *pSrc)
{		
	return S_OK;
}

STDMETHODIMP CoCPServiceIntel::SetDXEncryption(BOOL bOn)
{
	DXVA_Intel_Encryption_Protocol encProtocol, encProtocoloOut;
	ULONG ulVer;

	GetScrambleLibVersion(&ulVer);
	encProtocol.header.dwFunction = 0xffff0001;
	encProtocol.header.guidEncryptProtocol = bOn ? DXVA_Intel_Encryption : DXVA_NoEncrypt;
	encProtocol.scrambleKey = m_guidKey;
	encProtocol.scrambleLibVer = ulVer;

	return DXVA_Execute(0xffff0001,&encProtocol,sizeof(DXVA_Intel_Encryption_Protocol), &encProtocoloOut, sizeof(DXVA_Intel_Encryption_Protocol));	
}

STDMETHODIMP CoCPServiceIntel::ChangeKey()
{		
	DWORD *pKey=(DWORD*)&m_guidKey, dwRand;	

	for(int i=0; i<4; i++)
	{
		dwRand = rand();
		memcpy((void*)(pKey+i), &dwRand, 4);
	}

	return S_OK;
}

STDMETHODIMP_(ECPObjID)	CoCPServiceIntel::GetObjID()
{
	return E_CP_ID_INTEL;
}