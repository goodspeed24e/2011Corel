
#include "CPServiceAtiMp2IDCT.h"

#define DRIVER_ID			0x0000ff00
#define ENCRYPT_FUNC		0xffff0001
#define ATIAUTH_CMD_OFF		0x61746900
#define ATIAUTH_CMD_ON		0x61746901
#define ATIAUTH_CMD_MULTION	0x61746902

enum EWin7CPType
{
    E_CPWIN7_PROTECTIONENABLED = 1,
    E_CPWIN7_OVERLAYORFULLSCREENREQUIRED = 1<<1,
};

CoCPServiceAtiMp2IDCT::CoCPServiceAtiMp2IDCT() : m_bMultiChannelEncrypt(FALSE), m_dwDeviceID(0),
m_byChannelNum(0), m_hEncrypt(NULL), m_pD3D9AuthChannel(NULL), m_hD3D9AuthChannel(NULL)
{
	ZeroMemory(&m_DXVAEncryptProtocolHdr,sizeof(DXVA_EncryptProtocolHeader));
}

CoCPServiceAtiMp2IDCT::~CoCPServiceAtiMp2IDCT()
{	
	Close();
}

STDMETHODIMP CoCPServiceAtiMp2IDCT::QueryInterface(REFIID riid, void**ppInterface)
{
	if (IID_IUnknown == riid)
	{
		*ppInterface = static_cast<IUnknown *>(static_cast<ICPServiceAtiMp2IDCT *>(this));
	}
	else if (IID_ICPService == riid)
	{
		*ppInterface = static_cast<ICPService *>(static_cast<ICPServiceAtiMp2IDCT *>(this));
	}	
	else if (IID_ICPServiceAtiMp2IDCT == riid)
	{
		*ppInterface = static_cast<ICPServiceAtiMp2IDCT *>(this);
	}
	else
	{
		*ppInterface = NULL;
		return E_NOINTERFACE;
	}

	static_cast<IUnknown *>(*ppInterface)->AddRef();

	return S_OK;
}

STDMETHODIMP_(ULONG) CoCPServiceAtiMp2IDCT::AddRef()
{
	return InterlockedIncrement(&m_lRefCount);
}

STDMETHODIMP_(ULONG) CoCPServiceAtiMp2IDCT::Release()
{
	LONG lRefCount = InterlockedDecrement(&m_lRefCount);

	if (0 == lRefCount)	
		delete this;

	return lRefCount;
}


STDMETHODIMP CoCPServiceAtiMp2IDCT::Open(CPOpenOptions *pOpenOptions)
{
	HRESULT hr = E_FAIL;		
	DWORD dwRet = 0;

	hr = CoCPService::Open(pOpenOptions);
	if(FAILED(hr))
		return hr;
	
	//Get Encryption GUID 
	hr = DXVA_Execute(0xFFFF0000, NULL, 0, &m_DXVAEncryptProtocolHdr, sizeof(DXVA_EncryptProtocolHeader));
	if(FAILED(hr))
		return hr;	

	//Turn on authentication
	dwRet = Rage128_InitAuthentication(pOpenOptions->dwDeviceID, DRIVER_ID, pOpenOptions->pD3D9, &m_hEncrypt);
	m_dwDeviceID = pOpenOptions->dwDeviceID;
	if(dwRet==RAGE128AUTH_OK)
	{
		DP("Single Channel Encrypt");
		m_bMultiChannelEncrypt = FALSE;
	}
	else if((dwRet&0xffffff00)==RAGE128AUTH_MULTICHANNEL_OK)
	{
		DP("Multi Channel Encrypt");
		m_bMultiChannelEncrypt = TRUE;
		m_byChannelNum = static_cast<BYTE>(dwRet&0x000000ff);
	}
	else
	{
		Rage128_EndAuthentication(m_dwDeviceID, m_hEncrypt);
		return E_FAIL;
	}

	DP("Encrypt Mode = ATI_MP2_IDCT");

    if(pOpenOptions->bIsFlipEx)
    {
        hr = EnableGPUCPSCD(pOpenOptions->pD3D9);

        if(FAILED(hr))
        {
            DP("failed to init GPUCP, hr=0x%x", hr);
            return E_FAIL;
        }
    }

	return S_OK;
}

STDMETHODIMP CoCPServiceAtiMp2IDCT::Close()
{				
	HRESULT hr=E_FAIL;

	if(m_bEncryptOn)
	{	
		//Turn off encryption in DirectX VA Driver
		hr = SetDXEncryption(FALSE);
		if(FAILED(hr))
		{
			DP("failed to turn off encryption!!");
		}
		else
		{			
			DP("turn off encryption!!");
			m_bEncryptOn = FALSE;
		}
	}

	if (m_hEncrypt!=NULL) 
	{ 
		//Turn off authentication
		Rage128_EndAuthentication(m_dwDeviceID, m_hEncrypt);
		m_hEncrypt = NULL;
	}

    SAFE_RELEASE(m_pD3D9AuthChannel);
	CoCPService::Close();

	return S_OK;
}

STDMETHODIMP_(BOOL) CoCPServiceAtiMp2IDCT::IsScramblingRequired(ECPFrameType eImgType)
{	
	return CoCPService::IsScramblingRequired(eImgType);
}

STDMETHODIMP CoCPServiceAtiMp2IDCT::EnableScrambling()
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

STDMETHODIMP CoCPServiceAtiMp2IDCT::DisableScrambling()
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

STDMETHODIMP CoCPServiceAtiMp2IDCT::ScrambleData(BYTE *pDataOut, const BYTE *pDataIn, DWORD dwDataLen)
{	

	return S_OK;
}

STDMETHODIMP_(BYTE)	CoCPServiceAtiMp2IDCT::GetEncryptMode()
{
	return m_bMultiChannelEncrypt;
}


STDMETHODIMP CoCPServiceAtiMp2IDCT::ProtectedBlt(DWORD dwWidth, DWORD dwHeight, DWORD dwPitch, BYTE *pSrc)
{		
	
	return S_OK;
}


STDMETHODIMP CoCPServiceAtiMp2IDCT::SetDXEncryption(BOOL bOn)
{
	DWORD dwRet=0;
	ATIEncryptionCommand cmdEncrypt;
	cmdEncrypt.header.dwFunction = ENCRYPT_FUNC;
	memcpy(&cmdEncrypt.header,&m_DXVAEncryptProtocolHdr, sizeof(DXVA_EncryptProtocolHeader));

	if(bOn)
	{
		if(m_bMultiChannelEncrypt)
		{
			cmdEncrypt.dwCommand = ATIAUTH_CMD_MULTION;
			cmdEncrypt.dwData = m_byChannelNum;
		}
		else
		{		
			cmdEncrypt.dwCommand = ATIAUTH_CMD_ON;
		}
	}
	else
	{				
		cmdEncrypt.dwCommand = ATIAUTH_CMD_OFF;
	}
	
	return DXVA_Execute(ENCRYPT_FUNC, &cmdEncrypt, sizeof(ATIEncryptionCommand), &dwRet, sizeof(DWORD));

}

STDMETHODIMP CoCPServiceAtiMp2IDCT::SkipMBlock(DWORD dwDeviceID)
{
	if(m_bEncryptOn)
		return Rage128_SkipMBlock(dwDeviceID, m_hEncrypt);
	else
		return S_OK;
}

STDMETHODIMP CoCPServiceAtiMp2IDCT::RescrambleDXVACoeff(DWORD dwDeviceID, LPVOID pSrc, LPVOID pDest)
{
	if(m_bEncryptOn)
		return Rage128_RescrambleDXVACoeff(dwDeviceID, pSrc, pDest, m_hEncrypt);
	else
		return S_OK;
}

STDMETHODIMP CoCPServiceAtiMp2IDCT::RescrambleDXVABlock(DWORD dwDeviceID, LPVOID pSrc, LPVOID pDest, DWORD dwNumOfBytes)	
{
	if(m_bEncryptOn)
		return Rage128_RescrambleDXVABlock(dwDeviceID, pSrc, pDest, dwNumOfBytes, m_hEncrypt);
	else
		return S_OK;
}

STDMETHODIMP_(ECPObjID)	CoCPServiceAtiMp2IDCT::GetObjID()
{
	return E_CP_ID_ATI_MP2IDCT;
}

HRESULT CoCPServiceAtiMp2IDCT::EnableGPUCPSCD(IUnknown* pD3D9)
{
    HRESULT hr = S_OK;

    //Enable Screen Capture Defense on GPUCP.
    IDirect3DDevice9Video *pD3D9Video = NULL;
    D3DCONTENTPROTECTIONCAPS CPCaps;
    hr = pD3D9->QueryInterface(IID_IDirect3DDevice9Video, (void**)&pD3D9Video);
    if(FAILED(hr)||(pD3D9Video==NULL))
    {
        DP("failed to get IDirect3DDevice9Video, hr=0x%x", hr);
        return hr;
    }

    hr = pD3D9Video->GetContentProtectionCaps(NULL, NULL, &CPCaps);

    if(SUCCEEDED(hr) && (CPCaps.Caps & D3DCPCAPS_SOFTWARE)) // AMD case, ASIC does not support AESCTR_128 encryption but might support proprietary API - MPEG2
    {
        hr = pD3D9Video->CreateAuthenticatedChannel(D3DAUTHENTICATEDCHANNEL_D3D9, &m_pD3D9AuthChannel, &m_hD3D9AuthChannel);
        if(SUCCEEDED(hr))
        {
            //enable protection for D3D9/DWM 
            D3DAUTHENTICATEDCHANNEL_CONFIGUREPROTECTION cfgD3D9Protect;
            D3DAUTHENTICATEDCHANNEL_CONFIGURE_OUTPUT cfgOutput;
            cfgD3D9Protect.Parameters.ConfigureType = D3DAUTHENTICATEDCONFIGURE_PROTECTION;
            cfgD3D9Protect.Parameters.hChannel = m_hD3D9AuthChannel;
            cfgD3D9Protect.Parameters.SequenceNumber = 0;
            cfgD3D9Protect.Protections.Value = E_CPWIN7_PROTECTIONENABLED;
            memset(&(cfgD3D9Protect.Parameters.omac), 0, D3D_OMAC_SIZE);

            hr = m_pD3D9AuthChannel->Configure(sizeof(cfgD3D9Protect), &cfgD3D9Protect, &cfgOutput);
            if(FAILED(hr))
            {
                DP("failed to enable protection for D3D9/DWM, hr=0x%x", hr);
            }
            else
            {
                DP("succeed to enable protection for D3D9/DWM");
            }
        }
        else
        {
            DP("failed to create authenticated channel for D3D9/DWM, hr=0x%x", hr);
        }
    }
    else
    {
        DP("failed to get IDirect3DDevice9Video, hr=0x%x", hr);
        DP("GPUCP Driver Software Protection does not support");
        hr = E_FAIL;
    }

    SAFE_RELEASE(pD3D9Video);
    return hr;
}