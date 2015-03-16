
#include "CPServiceWin7.h"
#include <atlbase.h>
#include "Crypto.h" //for CryptographicLibrary
#include <dxva2api.h>
#include <InitGuid.h>
#include "crypt_data_gen.h" //for EPID key exchange
#include "intel_pavp_api.h"
#include ".\Imports\GPUCPService\GPUCPGuids.h"

DEFINE_GUID(D3DKEYEXCHANGE_CANTIGA, 
0xB15AFE59, 0x70BF, 0x4063, 0xA7, 0x33, 0xEA, 0xE1, 0xA2, 0xE7, 0x82, 0x42);

DEFINE_GUID(D3DKEYEXCHANGE_EAGLELAKE,
0xA75334C7, 0x080C, 0x4539, 0x8E, 0x2D, 0xE6, 0xA0, 0xD2, 0xB1, 0x0F, 0xF0);

DEFINE_GUID(D3DKEYEXCHANGE_EPID, 
0xD13D3283, 0x9154, 0x43A2, 0x90, 0x3A, 0xF7, 0xE0, 0xF9, 0x2D, 0x0A, 0xB5);

DEFINE_GUID(D3DCRYPTOTYPE_INTEL_AES128_CTR, 
0xE29FAF83, 0xAABF, 0x48C8, 0xA3, 0xAB, 0x85, 0xFE, 0xDD, 0x5E, 0x60, 0xC4);

#define DXVA2_DECODE_SPECIFY_ENCRYPTED_BLOCKS	0x724
#define DXVA2_DECODE_GET_DRIVER_HANDLE          0x725

CUTILcrit CoCPServiceWin7::m_csEPID;

CoCPServiceWin7::CoCPServiceWin7() : m_pD3D9Video(NULL), m_pAuthChannel(NULL), m_pCryptoSession(NULL), m_hAuthChannel(NULL), m_pD3D9AuthChannel(NULL),
m_hCryptoSession(NULL), m_hDXVA2Dec(NULL), m_hDevice(NULL), m_bOpened(FALSE), m_bCryptoSessionIsCreated(FALSE), m_bAudio(FALSE),
m_bStreamKeyIsGotten(FALSE), m_dwVenderID(0), m_bIsOverlay(FALSE), m_bIsFlipEx(FALSE), m_bDShowMode(FALSE)
{	
		m_bWindowed = TRUE;
		m_eSwapEffect = D3DSWAPEFFECT_FORCE_DWORD;
}

CoCPServiceWin7::~CoCPServiceWin7()
{
	Close();
}

STDMETHODIMP CoCPServiceWin7::QueryInterface(REFIID riid, void**ppInterface)
{
	if (IID_IUnknown == riid)
	{
		*ppInterface = static_cast<IUnknown *>(static_cast<ICPService *>(this));
	}
	else if (IID_ICPService == riid)
	{
		*ppInterface = static_cast<ICPService *>(this);
	}
	else if (IID_IGetParams == riid)
	{
		*ppInterface = static_cast<IGetParams *>(this);
	}
	else
	{
		*ppInterface = NULL;
		return E_NOINTERFACE;
	}

	static_cast<IUnknown *>(*ppInterface)->AddRef();

	return S_OK;
}

STDMETHODIMP_(ULONG) CoCPServiceWin7::AddRef()
{
	return InterlockedIncrement(&m_lRefCount);
}

STDMETHODIMP_(ULONG) CoCPServiceWin7::Release()
{
	LONG lRefCount = InterlockedDecrement(&m_lRefCount);

	if (0 == lRefCount)	
		delete this;

	return lRefCount;
}

STDMETHODIMP CoCPServiceWin7::GetProtectionCapabilities(GUID *pDecodeProfile)
{	
	HRESULT hr = E_FAIL;

	if(m_dwVenderID == VENDER_ID_INTEL)
	{	
		m_guidCryptoType = D3DCRYPTOTYPE_INTEL_AES128_CTR;
		hr = m_pD3D9Video->GetContentProtectionCaps(&m_guidCryptoType, pDecodeProfile, &m_CPCaps);
		if(SUCCEEDED(hr))
		{
			m_eWIN7_EncryptMode = E_WIN7_Encrypt_INTEL_AESCTR;
		}
	}
	else
	{		
		m_guidCryptoType = D3DCRYPTOTYPE_AES128_CTR;		
		hr = m_pD3D9Video->GetContentProtectionCaps(&m_guidCryptoType, pDecodeProfile, &m_CPCaps);
		if(SUCCEEDED(hr))
		{
			m_eWIN7_EncryptMode = E_WIN7_Encrypt_AESCTR;
		}
	}

	if(FAILED(hr)||(m_CPCaps.Caps==0))
	{		
		m_guidCryptoType = D3DCRYPTOTYPE_PROPRIETARY;
		hr = m_pD3D9Video->GetContentProtectionCaps(&m_guidCryptoType, pDecodeProfile, &m_CPCaps);
		if(SUCCEEDED(hr))
		{
			m_eWIN7_EncryptMode = E_WIN7_Encrypt_PROPRIETARY;
			DP("AESCTR encryption isn't be supported");
			return E_FAIL;
		}
		else
		{			
			m_eWIN7_EncryptMode = E_WIN7_Encrypt_NONE;
			DP("all crypto types aren't be supported");
            return E_FAIL;
		}			
	}		

	DP("Capabilities for Protection", m_CPCaps.Caps);					
	if(m_CPCaps.Caps & D3DCPCAPS_HARDWARE)	
	{
		m_eChannelType = D3DAUTHENTICATEDCHANNEL_DRIVER_HARDWARE;	
		DP("[Channel Type = DRIVER_HARDWARE]");
	}
	else if(m_CPCaps.Caps & D3DCPCAPS_SOFTWARE)	
	{
		m_eChannelType = D3DAUTHENTICATEDCHANNEL_DRIVER_SOFTWARE;	
		DP("[Channel Type = DRIVER_SOFTWARE]");
	}

	if(m_CPCaps.Caps & D3DCPCAPS_PROTECTIONALWAYSON)
		DP("[Protection Always On]");

	if(m_CPCaps.Caps & D3DCPCAPS_PARTIALDECRYPTION)
		DP("[Partial Decryption]");
	else
		m_bPartialEncryption = FALSE;

	if(m_CPCaps.Caps & D3DCPCAPS_CONTENTKEY)
		DP("[Content Key]");
	else
		m_bUseContentKey = FALSE;

	if(m_CPCaps.Caps & D3DCPCAPS_FRESHENSESSIONKEY)
		DP("[Freshen Session Key]");
	else
		m_bRefreshSessionKey = FALSE;

	if(m_CPCaps.Caps & D3DCPCAPS_ENCRYPTEDREADBACK)
		DP("[Encrypted Read Back]");

	if(m_CPCaps.Caps & D3DCPCAPS_ENCRYPTEDREADBACKKEY)
		DP("[Encrypted Read Back Key]");

	if(m_CPCaps.KeyExchangeType == D3DKEYEXCHANGE_RSAES_OAEP)
	{
		DP("[Key Exchange Type = RSAES OAEP]");
	}
	else if(m_CPCaps.KeyExchangeType == D3DKEYEXCHANGE_CANTIGA)
	{
		DP("[Key Exchange Type = CANTIGA]");
	}
	else if(m_CPCaps.KeyExchangeType == D3DKEYEXCHANGE_EAGLELAKE)
	{
		DP("[Key Exchange Type = EAGLELAKE]");
	}
	else if(m_CPCaps.KeyExchangeType == D3DKEYEXCHANGE_EPID)
	{
		DP("[Key Exchange Type = EPID]");
	}

	m_dwBufferAlignmentStart = m_CPCaps.BufferAlignmentStart; 
	m_dwBlockAlignmentSize = m_CPCaps.BlockAlignmentSize;	
	

	return S_OK;
}

STDMETHODIMP CoCPServiceWin7::Init(IUnknown* pD3D9, GUID *pDecodeProfile)
{
	HRESULT hr = E_FAIL;			
	
	if((pD3D9==NULL)||(*pDecodeProfile==GUID_NULL))
		return E_FAIL;

	m_dwEncryptedFramesWithSessionKey = 0;
	m_dwEncryptedFramesWithContentKey = 0;
		
	memset(m_ContentKey, 0, AES_KEY_LEN);
	memset(m_EncryptedContentKey, 0, AES_KEY_LEN);
	memset(m_AESKeyOfAuthChannel, 0, AES_KEY_LEN);
	memset(m_AESKeyOfCryptoSession, 0, AES_KEY_LEN);
	memset(&m_IV, 0, sizeof(m_IV));	
	memset(&m_SwappedIV, 0, sizeof(m_SwappedIV));	
		
	hr = pD3D9->QueryInterface(IID_IDirect3DDevice9Video, (void**)&m_pD3D9Video);
	if(FAILED(hr)||(m_pD3D9Video==NULL))
	{
		DP("failed to get IDirect3DDevice9Video, hr=0x%x", hr);
		return hr;
	}

	CComPtr<IDirect3DDevice9> pDevice;
	hr = pD3D9->QueryInterface(IID_IDirect3DDevice9, (void**)&pDevice);

	if(FAILED(hr)||(pDevice==NULL))
	{
		DP("failed to get IDirect3DDevice9, hr=0x%x", hr);
		return hr;
	}
	CComPtr<IDirect3DSwapChain9> pSwapChain;
	hr = pDevice->GetSwapChain(0, &pSwapChain);

	if(FAILED(hr)||(pSwapChain==NULL))
	{
		DP("failed to GetSwapChain, hr=0x%x", hr);
		return hr;
	}

	D3DPRESENT_PARAMETERS sPresentParam;
	ZeroMemory(&sPresentParam, sizeof(D3DPRESENT_PARAMETERS));
	hr = pSwapChain->GetPresentParameters(&sPresentParam);
	if(FAILED(hr))
	{
		DP("failed to GetPresentParameters, hr=0x%x", hr);
		return hr;
	}
	m_bWindowed = sPresentParam.Windowed;
	m_eSwapEffect = sPresentParam.SwapEffect;

	hr = GetProtectionCapabilities(pDecodeProfile);
	if(FAILED(hr)||(m_pD3D9Video==NULL))
	{
		DP("failed to get protection capabilities, hr=0x%x", hr);
		return hr;
	}	

	if(m_eWIN7_EncryptMode == E_WIN7_Encrypt_INTEL_AESCTR)
	{
		//Intel doesn't support scrambling for MoComp profile
		if((*(pDecodeProfile) == DXVA2_ModeMPEG2_MoComp) || (*(pDecodeProfile) == DXVA2_ModeH264_A) ||
			(*(pDecodeProfile) == DXVA2_ModeH264_B) || (*(pDecodeProfile) == DXVA2_ModeVC1_A) || 
			(*(pDecodeProfile) == DXVA2_ModeVC1_B))
			return E_FAIL;
	}

    if(!m_bDShowMode)
    {
		hr = m_pD3D9Video->CreateAuthenticatedChannel(D3DAUTHENTICATEDCHANNEL_D3D9, &m_pD3D9AuthChannel, &m_hD3D9AuthChannel);
		if(FAILED(hr))
		{
			DP("failed to create authenticated channel for D3D9/DWM, hr=0x%x", hr);
			return hr;
		}
    }

	hr = m_pD3D9Video->CreateAuthenticatedChannel(m_eChannelType, &m_pAuthChannel, &m_hAuthChannel);
	if(FAILED(hr))
	{
		DP("failed to create authenticated channel, hr=0x%x", hr);
		return hr;
	}

	if(m_CPCaps.KeyExchangeType != D3DKEYEXCHANGE_EPID)
	{
		hr = m_pD3D9Video->CreateCryptoSession(&m_guidCryptoType, pDecodeProfile, &m_pCryptoSession, &m_hCryptoSession);
		if(FAILED(hr))
		{
			DP("failed to create crypto session, hr=0x%x", hr);
			return hr;
		}
	}
	else
	{	
		CUTILautolock lock(&m_csEPID);
		m_pEPID = CEPIDKeyExchange::GetInstance();
		
		if(!m_pEPID->IsSessionCreated())				
		{
			hr = m_pD3D9Video->CreateCryptoSession(&m_guidCryptoType, pDecodeProfile, &m_pCryptoSession, &m_hCryptoSession);
			if(FAILED(hr))
			{				
				DP("failed to create crypto session, hr=0x%x", hr);
				return hr;
			}

			hr = m_pEPID->Open(this, &CoCPServiceWin7::EPIDExec, m_pCryptoSession, (LPVOID) m_hCryptoSession, m_bShowMsg);
			if(FAILED(hr))
			{
				DP("failed to open EPID!!");
				SAFE_RELEASE(m_pEPID);
				return hr;
			}

			hr = m_pEPID->CreateSession();
			if(FAILED(hr))
			{
				DP("failed to create session!!");
				SAFE_RELEASE(m_pEPID);
				return hr;
			}
		}
		else
		{
			m_bCryptoSessionIsCreated = TRUE;

			m_pCryptoSession = (IDirect3DCryptoSession9*) m_pEPID->GetExtObj();
		}
	}

	// Seed the random-number generator with the current time so that
	// the numbers will be different every time we run.
	srand(timeGetTime());	

	m_IV.IV = rand();		

	return S_OK;
}

STDMETHODIMP CoCPServiceWin7::Open(CPOpenOptions *pOpenOptions)
{	
	HRESULT hr = E_FAIL;	

	m_bShowMsg = ((pOpenOptions->dwCPOption&E_CP_OPTION_DBG_MSG)!=0);
	m_bAudio = ((pOpenOptions->dwCPOption&E_CP_OPTION_AUDIO)!=0);
	m_bRefreshSessionKey = ((pOpenOptions->dwCPOption&E_CP_WIN7_DISABLE_REFRESHED_SESSION_KEY)==0);
	m_dwVenderID = pOpenOptions->dwVenderID;
	m_bIsOverlay = pOpenOptions->bIsOverlay;
    m_bIsFlipEx = pOpenOptions->bIsFlipEx;
    m_bDShowMode = pOpenOptions->bDShowMode;

	DP("CoCPServiceWin7::Open() Begin");

	DP("Encrypt Mode = Win7_GPUCP");

	if(!m_bAudio) // for video
	{	
		hr = CoCPService::Open(pOpenOptions);
		if(FAILED(hr))
			return hr;		

		m_bUseContentKey = ((pOpenOptions->dwCPOption&E_CP_WIN7_DISABLE_CONTENT_KEY)==0);		
		m_bPartialEncryption = ((pOpenOptions->dwCPOption&E_CP_WIN7_DISABLE_PARTIAL_ENCRYPTION)==0);
		m_bProtectedBlt = ((pOpenOptions->dwCPOption&E_CP_WIN7_DISABLE_PROTECTED_BLT)==0);	

		hr = Init(pOpenOptions->pD3D9, pOpenOptions->pDecodeProfile);
		if(FAILED(hr))
		{
			DP("failed to init, hr=0x%x", hr);
			return hr;
		}		

		hr = NegotiateKeyWithAuthChannel();
		if(FAILED(hr))
			return hr;	

		if(m_eWIN7_EncryptMode == E_WIN7_Encrypt_INTEL_AESCTR)
		{
			if(m_CPCaps.KeyExchangeType != D3DKEYEXCHANGE_EPID)
			{
				hr = PAVPFixedKeyExchange();
				if(FAILED(hr))
				{
					DP("failed to do PAVPFixedKeyExchange()!!");
				}
				else
				{
					DP("succeeded to do PAVPFixedKeyExchange!!");
				}
			}
			else 
			{	
				if(!m_bCryptoSessionIsCreated)
				{																		
					hr = PAVPEPIDKeyExchange();
					if(FAILED(hr))
					{
						DP("failed to do EPID key exchange!!");
						SAFE_RELEASE(m_pEPID);
					}
					else
					{
						DP("succeeded to do EPID key exchange!!");
					}
					
				}
				else
				{			
					hr = m_pEPID->GetCurrStreamKey((StreamKey*)m_AESKeyOfCryptoSession, FALSE);
					if(FAILED(hr))
					{
						DP("failed to do EPID by created crypto session!!");
						SAFE_RELEASE(m_pEPID);
					}
					else
					{
						DP("succeeded to do EPID by created crypto session!!");
					}
				}
			}
			
		}
		else
		{
			hr = NegotiateKeyWithCryptoSession();
			if(FAILED(hr))
			{
				DP("failed to do NegotiateKeyWithCryptoSession()!!");
			}
			else
			{
				DP("succeeded to do NegotiateKeyWithCryptoSession!!");
			}
		}		
		if(FAILED(hr))
		{
			DP("failed to negotiate key, hr=0x%x", hr);
			return hr;
		}		
		
		hr = Configure();
		if(FAILED(hr))
		{
			DP("failed to configure, hr=0x%x", hr);
			return hr;
		}
	}
	else // for audio
	{				
		m_dwEncryptedFramesWithSessionKey = 0;
		m_AudioStreamId = pOpenOptions->dwAudioStreamId;
		m_CPCaps.KeyExchangeType = D3DKEYEXCHANGE_EPID;

		m_pEPID = CEPIDKeyExchange::GetInstance();

		m_pEPID->SetAudioStreamID(m_AudioStreamId);

		memset(m_AESKeyOfCryptoSession, 0, AES_KEY_LEN);					
	}

	m_bOpened = TRUE;

	DP("CoCPServiceWin7::Open() End");

	return S_OK;
}

STDMETHODIMP CoCPServiceWin7::Close()
{		
	HRESULT hr = E_FAIL;	

	DP("CoCPServiceWin7::Close() Begin");

	if((m_bOpened) && (m_CPCaps.KeyExchangeType == D3DKEYEXCHANGE_EPID))
	{				
		if(m_bAudio)
			m_pEPID->InvalidateStreamKey(this, m_bAudio);

		// begin of CryptoSession release cycle for EPID
		// warning: please don't set m_pCryptoSession = NULL before m_pEPID has been released
		if(m_pCryptoSession!=NULL)
			m_pCryptoSession->Release();				

		EPID_SAFE_RELEASE(m_pEPID, this);
		m_pCryptoSession = NULL;		
		// end of CryptoSession release cycle for EPID
							
		m_bCryptoSessionIsCreated = FALSE;		
		m_bOpened = FALSE;		
	}
		
	if(m_pD3D9AuthChannel)
	{
		D3DAUTHENTICATEDCHANNEL_CONFIGURE_OUTPUT cfgOutput;	
		D3DAUTHENTICATEDCHANNEL_CONFIGUREPROTECTION cfgD3D9Protect;
		cfgD3D9Protect.Parameters.ConfigureType = D3DAUTHENTICATEDCONFIGURE_PROTECTION;
		cfgD3D9Protect.Parameters.hChannel = m_hD3D9AuthChannel;
		cfgD3D9Protect.Parameters.SequenceNumber = 0;		
		cfgD3D9Protect.Protections.Value = 0; // disable protection	
		memset(&(cfgD3D9Protect.Parameters.omac), 0, D3D_OMAC_SIZE);	

		hr = m_pD3D9AuthChannel->Configure(sizeof(cfgD3D9Protect), &cfgD3D9Protect, &cfgOutput);	
		if(FAILED(hr))
		{
			DP("failed to disable protection for D3D9/DWM, hr=0x%x", hr);	
			return hr;
		}
		else
		{				
			DP("succeed to disable protection for D3D9/DWM");	
		}
	}	

	SAFE_RELEASE(m_pD3D9Video);
	SAFE_RELEASE(m_pAuthChannel);
    SAFE_RELEASE(m_pD3D9AuthChannel);
	SAFE_RELEASE(m_pCryptoSession);	
	
	CoCPService::Close();

	DP("CoCPServiceWin7::Close() End");

	return hr;
}

STDMETHODIMP_(BOOL)	CoCPServiceWin7::IsScramblingRequired(ECPFrameType eImgType)
{		
	return CoCPService::IsScramblingRequired(eImgType);
}
		
STDMETHODIMP CoCPServiceWin7::EnableScrambling()
{
	HRESULT hr = S_OK;
	
	if(m_eWIN7_EncryptMode != E_WIN7_Encrypt_INTEL_AESCTR)
	{	
		if((m_dwEncryptedFramesWithSessionKey==SESSION_KEY_REFRESH_LEVEL) && m_bRefreshSessionKey)
		{		
			if (!m_bCryptoSessionIsCreated)
			{
				hr = RefreshSessionKey();			
				if(FAILED(hr))
					DP("failed to refresh session key");
				else
					DP("succeeded to refresh session key");
			}

			m_dwEncryptedFramesWithSessionKey = 0;
		}
		else if(m_dwEncryptedFramesWithContentKey==CONTENT_KEY_REFRESH_LEVEL)
		{		
			hr = UpdateContentKey();		

			if(FAILED(hr))
				DP("failed to update content key");
			else
				DP("succeeded to update content key");

			m_dwEncryptedFramesWithContentKey = 0;
		}
	}

	return CoCPService::EnableScrambling();
}
	
STDMETHODIMP CoCPServiceWin7::DisableScrambling()
{
	return CoCPService::DisableScrambling();
}


STDMETHODIMP_(UINT64) CoCPServiceWin7::SwapByteOrder(UINT64 ui64Val)
{
	UINT64 ui64Origin = (UINT64)ui64Val;

	UINT64 ui64Swapped = ( (0x00000000000000FF) & (ui64Origin >> 56)

		| (0x000000000000FF00) & (ui64Origin >> 40)

		| (0x0000000000FF0000) & (ui64Origin >> 24)

		| (0x00000000FF000000) & (ui64Origin >> 8)

		| (0x000000FF00000000) & (ui64Origin << 8)

		| (0x0000FF0000000000) & (ui64Origin << 24)

		| (0x00FF000000000000) & (ui64Origin << 40)

		| (0xFF00000000000000) & (ui64Origin << 56));

	return ui64Swapped;
}

STDMETHODIMP CoCPServiceWin7::ScrambleData(BYTE *pDataOut, const BYTE *pDataIn, DWORD dwDataLen)
{		
	HRESULT hr = S_OK;			

	if(!m_bAudio) // for video
	{
		if(m_bEncryptOn)
		{				
			CryptographicLibrary Cryptographic;

			m_dwEncryptedFramesWithSessionKey++;		           

            if(m_eWIN7_EncryptMode == E_WIN7_Encrypt_INTEL_AESCTR)
			{				
				m_IV.IV = 0;
				m_IV.Count = 1;		
			}
			else
			{                
				++m_IV.IV;
				m_IV.Count = rand();		
			}

			m_SwappedIV.IV = SwapByteOrder(m_IV.IV);	
			m_SwappedIV.Count = SwapByteOrder(m_IV.Count);

			if((m_bCryptoSessionIsCreated) && (m_pEPID->IsVideoStreamKeyChanged()))
			{
				hr = m_pEPID->GetCurrStreamKey((StreamKey*)m_AESKeyOfCryptoSession, FALSE);
				if(FAILED(hr))
				{
					DP("failed to do EPID by created crypto session!!");
					SAFE_RELEASE(m_pEPID);
				}
				else
				{
					DP("succeeded to do EPID by created crypto session!!");
				}				
			}
            
            if((m_CPCaps.Caps & D3DCPCAPS_CONTENTKEY) && m_bUseContentKey)
            {		
                m_dwEncryptedFramesWithContentKey++;
                Cryptographic.AES_128CTR_GPUCP(m_ContentKey, pDataIn, pDataOut, dwDataLen, (BYTE*)&m_SwappedIV);		
            }
            else
            {						
                Cryptographic.AES_128CTR_GPUCP(m_AESKeyOfCryptoSession, pDataIn, pDataOut, dwDataLen, (BYTE*)&m_SwappedIV);
            }

            m_SwappedIV.Count = SwapByteOrder(m_IV.Count);             
		}
		else
		{
			memcpy(pDataOut, pDataIn, dwDataLen);
		}
	}
	else // for audio
	{		
		CdgStatus Status = CdgStsOk;		

		if(!m_bStreamKeyIsGotten)
		{		
			m_pCryptoSession = (IDirect3DCryptoSession9*) m_pEPID->GetExtObj();

			hr = m_pEPID->UpdateStreamKey((StreamKey*)m_AESKeyOfCryptoSession, this, m_bAudio);
			if(FAILED(hr))
			{
				DP("failed to update audio stream key!!");			
			}
			else
			{
				DP("succeeded to update audio stream key!!");				
			}	

			m_bStreamKeyIsGotten = TRUE;
		}		

		/*if((++m_dwEncryptedFramesWithSessionKey==AUDIO_SESSION_KEY_REFRESH_LEVEL) && m_bRefreshSessionKey)
		{					
			hr = RefreshSessionKey();			
			if(FAILED(hr))
				DP("failed to refresh audio session key");
			else
				DP("succeeded to refresh audio session key");
			
			m_dwEncryptedFramesWithSessionKey = 0;
		}*/

		Status = Aes128EcbEncrypt((BYTE*)pDataIn, dwDataLen, pDataOut, dwDataLen, 
			m_AESKeyOfCryptoSession, PAVP_EPID_STREAM_KEY_LEN);

		if(Status != CdgStsOk) 
		{
			DP("failed to AES Encrypt");
			hr = E_FAIL;			
		}			
	}
	
	return hr;
}

STDMETHODIMP_(BYTE) CoCPServiceWin7::GetEncryptMode()
{
	return m_eWIN7_EncryptMode;
}

STDMETHODIMP CoCPServiceWin7::Configure()
{
	HRESULT hr = E_FAIL;
	CryptographicLibrary Cryptographic;		

	//initialize
	D3DAUTHENTICATEDCHANNEL_CONFIGUREINITIALIZE cfgInit;
	D3DAUTHENTICATEDCHANNEL_CONFIGURE_OUTPUT cfgOutput;	
	cfgInit.Parameters.ConfigureType = D3DAUTHENTICATEDCONFIGURE_INITIALIZE;
	cfgInit.Parameters.hChannel = m_hAuthChannel;
	cfgInit.StartSequenceConfigure = m_uiStartSequenceConfigure = rand();
	cfgInit.StartSequenceQuery = m_uiStartSequenceQuery = rand();

	Cryptographic.OMAC1_128(m_AESKeyOfAuthChannel, (BYTE*)&(cfgInit)+D3D_OMAC_SIZE, sizeof(cfgInit)-D3D_OMAC_SIZE, (BYTE*)&(cfgInit.Parameters.omac));

	hr = m_pAuthChannel->Configure(sizeof(cfgInit), &cfgInit, &cfgOutput);
	if(FAILED(hr))
	{
		DP("failed to initialize, hr=0x%x", hr);
		return hr;
	}

	//get device handle
	D3DAUTHENTICATEDCHANNEL_QUERY_INPUT queryInput;
	D3DAUTHENTICATEDCHANNEL_QUERYDEVICEHANDLE_OUTPUT queryDevice;

	queryInput.QueryType = D3DAUTHENTICATEDQUERY_DEVICEHANDLE;
	queryInput.hChannel = m_hAuthChannel;
	queryInput.SequenceNumber = m_uiStartSequenceQuery++;

	hr = m_pAuthChannel->Query(sizeof(queryInput), &queryInput, sizeof(queryDevice), &queryDevice);
	if(FAILED(hr))
	{
		DP("failed to query device handle, hr=0x%x", hr);
		return hr;
	}

	m_hDevice = queryDevice.DeviceHandle;

	UINT uProtectionValue = E_CPWIN7_PROTECTIONENABLED;

	//We should set OverlayOrFullscreenRequired because it is full-screen exclusive or overlay. Please refer to http://msdn.microsoft.com/en-us/library/dd317950(VS.85).aspx
	if(m_bIsOverlay) //ProtectionEnabled + OverlayOrFullscreenRequired 
		uProtectionValue |= E_CPWIN7_OVERLAYORFULLSCREENREQUIRED;

	if(!m_bWindowed && m_dwVenderID == VENDER_ID_NVIDIA)
		uProtectionValue |= E_CPWIN7_OVERLAYORFULLSCREENREQUIRED;

	// D3DAUTHENTICATEDCHANNEL_D3D9 is controlled by the D3D9 run-ime and not by the driver. => from NV's comment
	// D3D9/DWM channel should be used on pure D3D presenter or FlipEx mode and we have to recreate the protection channel if D3D device reset/recreate.
	// If enable D3D9/DWM channel and using D3D9Overlay, the video is black.	
	if(!(m_dwVenderID == VENDER_ID_NVIDIA) && !m_bIsOverlay && !m_bDShowMode)   //For D3D Presenter and FlipEx
	{
		//enable protection for D3D9/DWM 
		D3DAUTHENTICATEDCHANNEL_CONFIGUREPROTECTION cfgD3D9Protect;
		cfgD3D9Protect.Parameters.ConfigureType = D3DAUTHENTICATEDCONFIGURE_PROTECTION;
		cfgD3D9Protect.Parameters.hChannel = m_hD3D9AuthChannel;
		cfgD3D9Protect.Parameters.SequenceNumber = 0;		
		cfgD3D9Protect.Protections.Value = uProtectionValue;
		memset(&(cfgD3D9Protect.Parameters.omac), 0, D3D_OMAC_SIZE);	

		hr = m_pD3D9AuthChannel->Configure(sizeof(cfgD3D9Protect), &cfgD3D9Protect, &cfgOutput);	
		if(FAILED(hr))
		{
			DP("failed to enable protection for D3D9/DWM, hr=0x%x", hr);	
			return hr;
		}
		else
		{				
			DP("succeed to enable protection for D3D9/DWM, Protections.Value = 0x%08x, Overlay = %d, windowed = %d", cfgD3D9Protect.Protections.Value, m_bIsOverlay, m_bWindowed);
		}
	}

	//enable protection		
	D3DAUTHENTICATEDCHANNEL_CONFIGUREPROTECTION cfgProtect;
	cfgProtect.Parameters.ConfigureType = D3DAUTHENTICATEDCONFIGURE_PROTECTION;
	cfgProtect.Parameters.hChannel = m_hAuthChannel;
	cfgProtect.Parameters.SequenceNumber = ++m_uiStartSequenceConfigure;
	cfgProtect.Protections.Value = uProtectionValue;
	Cryptographic.OMAC1_128(m_AESKeyOfAuthChannel, (BYTE*)&(cfgProtect)+D3D_OMAC_SIZE, sizeof(cfgProtect)-D3D_OMAC_SIZE, (BYTE*)&(cfgProtect.Parameters.omac));	

	hr = m_pAuthChannel->Configure(sizeof(cfgProtect), &cfgProtect, &cfgOutput);	
	if(FAILED(hr))
	{
		DP("failed to enable protection, hr=0x%x", hr);	
		return hr;
	}
	else
	{		
		m_bEncryptOn = TRUE;
		DP("succeed to enable protection, Protections.Value = 0x%08x, Overlay = %d, windowed = %d", cfgProtect.Protections.Value, m_bIsOverlay, m_bWindowed);
	}	
	
	//get handle of dxva2 decoder
	hr = DXVA_Execute(DXVA2_DECODE_GET_DRIVER_HANDLE, NULL, 0, &m_hDXVA2Dec, sizeof(HANDLE));
	if(FAILED(hr))
	{
		DP("failed to get handle of dxva2 decoder, hr=0x%x", hr);
		return hr;
	}	
	
	//associate crypto session with dxva2 decoder and d3d9 device
	D3DAUTHENTICATEDCHANNEL_CONFIGURECRYPTOSESSION cfgCrypto;
	cfgCrypto.Parameters.ConfigureType = D3DAUTHENTICATEDCONFIGURE_CRYPTOSESSION;
	cfgCrypto.Parameters.hChannel = m_hAuthChannel;
	cfgCrypto.Parameters.SequenceNumber = ++m_uiStartSequenceConfigure;	
	if(!m_bCryptoSessionIsCreated)
		cfgCrypto.CryptoSessionHandle = m_hCryptoSession;	
	else		
		cfgCrypto.CryptoSessionHandle = (HANDLE) m_pEPID->GetSharedData();				
	cfgCrypto.DeviceHandle = m_hDevice;
	cfgCrypto.DXVA2DecodeHandle = m_hDXVA2Dec;
	Cryptographic.OMAC1_128(m_AESKeyOfAuthChannel, (BYTE*)&(cfgCrypto)+D3D_OMAC_SIZE, sizeof(cfgCrypto)-D3D_OMAC_SIZE, (BYTE*)&(cfgCrypto.Parameters.omac));

	DP("(D3D Device:0x%x, DXVA2 decoder:0x%x, CryptoSession:0x%x)\n",m_hDevice, m_hDXVA2Dec, cfgCrypto.CryptoSessionHandle);

	hr = m_pAuthChannel->Configure(sizeof(cfgCrypto), &cfgCrypto, &cfgOutput);
	if(FAILED(hr))
	{
		DP("failed to associate crypto session, hr=0x%x", hr);	
		return hr;
	}	
	

	//partial decryption is not implemented yet
	/*if((m_CPCaps.Caps & D3DCPCAPS_PARTIALDECRYPTION) && m_bPartialEncryption)
	{	
		//get handle of dxva2 decoder
		D3DENCRYPTED_BLOCK_INFO EncBlkInfo;
		memset(&EncBlkInfo, 0, sizeof(D3DENCRYPTED_BLOCK_INFO));

		EncBlkInfo.NumEncryptedBytesAtBeginning = xxx;
		EncBlkInfo.NumBytesInSkipPattern = xxx;
		EncBlkInfo.NumBytesInEncryptPattern = xxx;

		hr = DXVA_Execute(DXVA2_DECODE_SPECIFY_ENCRYPTED_BLOCKS, NULL, 0, &EncBlkInfo, sizeof(D3DENCRYPTED_BLOCK_INFO));
		if(FAILED(hr))
		{
			DP("failed to set partial decryption, hr=0x%x", hr);
			return hr;
		}
	}*/

	return S_OK;
}

STDMETHODIMP CoCPServiceWin7::EncryptAESKey(BYTE *pCert, DWORD dwCertSize, BYTE *pAESKey, DWORD dwAESKeySize, BYTE *pEncryptedAESKey, UINT uiEncryptedAESKeySize)
{
	PKCS7_st *p7_PEAuth;	//PKCS7 structure
	R_RSA_PUBLIC_KEY rsaPublicKey;	//rsa public key obtained from leaf certificate		
	DWORD dwRSAKeyLen = 0;				
	CryptographicLibrary Cryptographic;
	HRESULT hr = E_FAIL;

	//parse PKCS7 signature with a certificate chain
	p7_PEAuth = Cryptographic.GetPKCS7_CertificateChain(pCert,(int)dwCertSize);
	if(p7_PEAuth == NULL)
		return E_FAIL;	

	//verify certificate chain up to root
	hr = Cryptographic.PKCS7_CertChain_CHECK(p7_PEAuth);
	if(FAILED(hr))
		return E_FAIL;

	Cryptographic.PKCS7_Get_LeafCert_PubKey(p7_PEAuth, rsaPublicKey);			

	if(Cryptographic.RSA_Encryption(pEncryptedAESKey, &uiEncryptedAESKeySize, pAESKey, AES_KEY_LEN, &rsaPublicKey, RSA_OAEP_Padding, SHA512)==FALSE)	
		return E_FAIL;

	return S_OK;
}

STDMETHODIMP CoCPServiceWin7::EncryptAESKeyForIntel(BYTE *pAESKey, DWORD dwAESKeySize, BYTE *pEncryptedAESKey, UINT uiEncryptedAESKeySize)
{
	CryptographicLibrary Cryptographic;	
	DWORD  dwFixedKey[4];

	if(m_CPCaps.KeyExchangeType == D3DKEYEXCHANGE_CANTIGA) 
	{
		DWORD IviKey[4] = {0x939d007b, 0x5685eab9, 0xde170be5, 0x9771bac0};
		BYTE IviCypherText[16] = {0x3e, 0xc4, 0x45, 0x00, 0x01, 0x9b, 0x86, 0x4d, 0xc5, 0xb8, 0x03, 0xe5, 0x84, 0x55, 0xb8, 0x5a};

		// Decrypt IviCypherText to obtain Intel real FixedKey {0x2c2b4115, 0x6441122b, 0x042c5505, 0x04171b2e}
		Cryptographic.AES_128D((BYTE*)&IviKey, (BYTE*)&IviCypherText, (BYTE*)&dwFixedKey);
	}
	else if(m_CPCaps.KeyExchangeType == D3DKEYEXCHANGE_EAGLELAKE)
	{
		DWORD IviKey[4] = {0xd0da671d, 0x5b67fc88, 0x89acb5ca, 0xa5cea2a1};
		BYTE IviCypherText[16] = {0x4e, 0xcc, 0x33, 0x21, 0x99, 0x55, 0xed, 0x91, 0xe7, 0x45, 0x79, 0x97, 0x81, 0x16, 0xa2, 0xb5};

		// Decrypt IviEncryptedKey to obtain Intel real FixedKey {0x3b14e7e6, 0x9a925bad, 0xaa6e7172, 0x55ef99a5}
		Cryptographic.AES_128D((BYTE*)&IviKey, (BYTE*)&IviCypherText, (BYTE*)&dwFixedKey);
	}

	Cryptographic.AES_128E((BYTE*)&dwFixedKey, pAESKey, pEncryptedAESKey);

	memset(dwFixedKey, 0, AES_KEY_LEN);

	return S_OK;
}

STDMETHODIMP CoCPServiceWin7::NegotiateKeyWithAuthChannel()
{
	HRESULT hr = E_FAIL;
	
	BYTE EncryptedAESKey[CIPHER_TEXT_LEN];
	//get certificate
	BYTE *pCertificate = NULL;
	UINT uiCertificateLength;
	hr = m_pAuthChannel->GetCertificateSize(&uiCertificateLength);
	if(FAILED(hr)||(uiCertificateLength==0))
	{
		DP("failed to get certificate size for authenticated channel, hr=0x%x", hr);
		return E_FAIL;
	}

	pCertificate = new BYTE[uiCertificateLength];
	m_pAuthChannel->GetCertificate(uiCertificateLength, pCertificate);
	if(FAILED(hr))
	{
		DP("failed to get certificate for authenticated channel, hr=0x%x", hr);
		SAFE_DELETE_ARRAY(pCertificate);
		return E_FAIL;
	}

	GenerateKey(m_AESKeyOfAuthChannel, AES_KEY_LEN);
	hr = EncryptAESKey(pCertificate, uiCertificateLength, m_AESKeyOfAuthChannel, AES_KEY_LEN, EncryptedAESKey, CIPHER_TEXT_LEN);
	if(FAILED(hr))
	{
		DP("failed to encrypt aes key by certificate for authenticated channel, hr=0x%x", hr);
		return E_FAIL;
	}

	hr = m_pAuthChannel->NegotiateKeyExchange(CIPHER_TEXT_LEN, EncryptedAESKey);
	if(FAILED(hr))
	{
		DP("failed to negotiate key with authenticated channel, hr=0x%x", hr);
		return E_FAIL;
	}		
	

	return S_OK;
}

STDMETHODIMP CoCPServiceWin7::NegotiateKeyWithCryptoSession()
{
	HRESULT hr = E_FAIL;

	BYTE EncryptedAESKey[CIPHER_TEXT_LEN];
	//get certificate
	BYTE *pCertificate = NULL;
	UINT uiCertificateLength;
	hr = m_pCryptoSession->GetCertificateSize(&uiCertificateLength);
	if(FAILED(hr)||(uiCertificateLength==0))
	{
		DP("failed to get certificate size for crypto cession, hr=0x%x", hr);
		return E_FAIL;
	}

	pCertificate = new BYTE[uiCertificateLength];
	m_pCryptoSession->GetCertificate(uiCertificateLength, pCertificate);
	if(FAILED(hr))
	{
		DP("failed to get certificate for crypto cession, hr=0x%x", hr);
		SAFE_DELETE_ARRAY(pCertificate);
		return E_FAIL;
	}

	GenerateKey(m_AESKeyOfCryptoSession, AES_KEY_LEN);
	hr = EncryptAESKey(pCertificate, uiCertificateLength, m_AESKeyOfCryptoSession, AES_KEY_LEN, EncryptedAESKey, CIPHER_TEXT_LEN);
	if(FAILED(hr))
	{
		DP("failed to encrypt aes key by certificate for crypto cession, hr=0x%x", hr);
		return E_FAIL;
	}

	hr = m_pCryptoSession->NegotiateKeyExchange(CIPHER_TEXT_LEN, EncryptedAESKey);
	if(FAILED(hr))
	{
		DP("failed to negotiate key with crypto cession, hr=0x%x", hr);
		return E_FAIL;
	}	

	if((m_CPCaps.Caps & D3DCPCAPS_CONTENTKEY)&&(m_bUseContentKey))
		UpdateContentKey();
	

	return S_OK;
}

STDMETHODIMP CoCPServiceWin7::PAVPFixedKeyExchange()
{
	HRESULT hr = E_FAIL;
	GPUCP_CRYPTOSESSION_PAVP_KEYEXCHANGE PAVPKeys;
	PAVP_FIXED_EXCHANGE_PARAMS PAVPParams;		

	GenerateKey((BYTE*)&m_AESKeyOfCryptoSession, AES_KEY_LEN);	
	EncryptAESKeyForIntel((BYTE*)&m_AESKeyOfCryptoSession, AES_KEY_LEN, (BYTE*)&(PAVPParams.SessionKey), AES_KEY_LEN);

	if(m_CPCaps.KeyExchangeType == D3DKEYEXCHANGE_CANTIGA) 
	{	
		PAVPParams.FixedKey[0]	= 0x9df76f5e;
		PAVPParams.FixedKey[1]	= 0xee8576f0;
		PAVPParams.FixedKey[2]	= 0x2cc0c3cb;
		PAVPParams.FixedKey[3]	= 0xc1ce796d;
	}
	else if(m_CPCaps.KeyExchangeType == D3DKEYEXCHANGE_EAGLELAKE)
	{	
		PAVPParams.FixedKey[0]	= 0xd5cb367f;
		PAVPParams.FixedKey[1]	= 0x1a96aab7;
		PAVPParams.FixedKey[2]	= 0x63d3cba7;
		PAVPParams.FixedKey[3]	= 0xfd62d894;	
	}

	PAVPKeys.guidPAVPKeyExchange = m_CPCaps.KeyExchangeType;
	PAVPKeys.pKeyExchangeParams = &PAVPParams;
	PAVPKeys.uiDataSize	= sizeof(PAVPParams);

	hr = m_pCryptoSession->NegotiateKeyExchange(sizeof(PAVPKeys), &PAVPKeys);
	if(FAILED(hr))
	{
		DP("failed to negotiate key with crypto session, hr=0x%x", hr);
		return E_FAIL;
	}		

	if((m_CPCaps.Caps & D3DCPCAPS_CONTENTKEY)&&(m_bUseContentKey))
		UpdateContentKey();

	return S_OK;
}

STDMETHODIMP CoCPServiceWin7::PAVPEPIDKeyExchange()
{
	HRESULT hr = S_OK;	
	
	hr = m_pEPID->UpdateStreamKey((StreamKey*)m_AESKeyOfCryptoSession, this, m_bAudio);
	if(FAILED(hr))
	{
		DP("failed to update stream key!!");			
	}	

	return hr;
}

STDMETHODIMP CoCPServiceWin7::GenerateKey(BYTE *pKey, DWORD dwKeySize)
{
	//used for random number generator
	DWORD g_dwSeed[5] = {0x646991fd, 0x48b8098c, 0xfe5afee9, 0xb98f9220, 0x82ecd447};

	CryptographicLibrary Cryptographic;

	for(DWORD i = 0; i<dwKeySize; i+=16)
		Cryptographic.RNG_ANSI_X931_128((BYTE *)g_dwSeed, pKey+i);

	return S_OK;
}

STDMETHODIMP CoCPServiceWin7::ProtectedBlt(DWORD dwWidth, DWORD dwHeight, DWORD dwPitch, BYTE *pSrc)
{
	return S_OK;
}

STDMETHODIMP_(ECPObjID)	CoCPServiceWin7::GetObjID()
{
	return E_CP_ID_WIN7;
}

STDMETHODIMP_(LPVOID) CoCPServiceWin7::GetIV()
{	
	if(m_bEncryptOn)	
		return (LPVOID)&(m_SwappedIV);	
	else
		return NULL;
}

STDMETHODIMP_(LPVOID) CoCPServiceWin7::GetContentKey()
{
	if(m_bEncryptOn)
	{		
		if((m_CPCaps.Caps & D3DCPCAPS_CONTENTKEY)&&(m_bUseContentKey))							
			return ((LPVOID)&m_EncryptedContentKey);				
		else		
			return ((LPVOID)&m_ContentKey);		
	}
	else
		return NULL;
}

STDMETHODIMP CoCPServiceWin7::RefreshSessionKey()
{
	HRESULT hr = S_OK;				
	
	if(m_CPCaps.KeyExchangeType != D3DKEYEXCHANGE_EPID)
	{	
		BYTE RandomNum[AES_KEY_LEN];	

		memset(RandomNum, 0, AES_KEY_LEN);

		hr = m_pCryptoSession->StartSessionKeyRefresh(RandomNum, AES_KEY_LEN);
		
		for(int i=0;i<AES_KEY_LEN;i++)
			m_AESKeyOfCryptoSession[i] = m_AESKeyOfCryptoSession[i] ^ RandomNum[i];

		if((m_CPCaps.Caps & D3DCPCAPS_CONTENTKEY) && m_bUseContentKey)
			hr = UpdateContentKey(FALSE);

		hr = m_pCryptoSession->FinishSessionKeyRefresh();	
	}
	else
	{	
		hr = m_pEPID->UpdateStreamKey((StreamKey*)m_AESKeyOfCryptoSession, this, m_bAudio);
		if(FAILED(hr))
		{
			DP("failed to update stream key!!");			
		}			
	}

	return hr;
}

STDMETHODIMP CoCPServiceWin7::UpdateContentKey(BOOL bNewKey)
{
	CryptographicLibrary Cryptographic;
	if(bNewKey)
		GenerateKey(m_ContentKey, AES_KEY_LEN);
	Cryptographic.AES_128E(m_AESKeyOfCryptoSession, m_ContentKey, m_EncryptedContentKey);

	return S_OK;
}

HRESULT CoCPServiceWin7::EPIDExec(PVOID pvContext, LPVOID pInBuf, DWORD dwInBufLen, LPVOID pOutBuf, DWORD dwOutBufLen)
{
	GPUCP_CRYPTOSESSION_PAVP_KEYEXCHANGE PAVPKeys;
	PAVP_EPID_EXCHANGE_PARAMS EPIDParams;
	HRESULT hr = E_FAIL;

	CoCPServiceWin7 *pd = reinterpret_cast<CoCPServiceWin7 *>(pvContext);

	ZeroMemory(&EPIDParams , sizeof(PAVP_EPID_EXCHANGE_PARAMS));
	EPIDParams.pInput = (BYTE*)pInBuf;
	EPIDParams.ulInputSize = dwInBufLen;
	EPIDParams.pOutput = (BYTE*)pOutBuf;
	EPIDParams.ulOutputSize = dwOutBufLen;

	ZeroMemory(&PAVPKeys , sizeof(GPUCP_CRYPTOSESSION_PAVP_KEYEXCHANGE));
	PAVPKeys.guidPAVPKeyExchange = D3DKEYEXCHANGE_EPID;
	PAVPKeys.pKeyExchangeParams = (void*)&EPIDParams;
	PAVPKeys.uiDataSize	= sizeof(PAVP_EPID_EXCHANGE_PARAMS);

	if(pd->m_pCryptoSession!=NULL)
	{
		hr = pd->m_pCryptoSession->NegotiateKeyExchange(sizeof(PAVPKeys), (BYTE*)&PAVPKeys);
		if(FAILED(hr))
		{
			pd->DP("failed to do EPID key exchange step, hr=0x%x", hr);
			return E_FAIL;
		}	
	}

	return S_OK;
}

/*HRESULT CoCPServiceWin7::EPIDRelease(PVOID pvContext)
{	
	CoCPServiceWin7 *pd;
	pd = reinterpret_cast<CoCPServiceWin7 *>(pvContext);

	pd->m_pCryptoSession->Release();	

	return S_OK;
}*/CoCPServiceWin7PAVP::CoCPServiceWin7PAVP()
{
	m_pGPUCP = NULL;
}

CoCPServiceWin7PAVP::~CoCPServiceWin7PAVP()
{
	SAFE_RELEASE(m_pGPUCP);
}


STDMETHODIMP CoCPServiceWin7PAVP::Open(CPOpenOptions *pOpenOptions)
{	
	HRESULT hr = E_FAIL;	

	if(pOpenOptions->pVA == NULL)
		return E_INVALIDARG;

	hr = CoCreateInstance(CLSID_GPUCP_PAVP, NULL, CLSCTX_ALL, IID_IGPUCPService, (void**)&m_pGPUCP);
	DP("[QATEST]USE SNB HEAVY MODE");
	GPUCPOpenOptions temp;
	temp.bDShowMode = pOpenOptions->bDShowMode;
	temp.bIsOverlay = pOpenOptions->bIsOverlay;
	temp.dwCPOption = pOpenOptions->dwCPOption;
	temp.dwDeviceID = pOpenOptions->dwDeviceID;
	temp.dwVenderID = pOpenOptions->dwVenderID;
	temp.pD3D9		= pOpenOptions->pD3D9;
	temp.pDecodeProfile = pOpenOptions->pDecodeProfile;
	temp.pVA = pOpenOptions->pVA;
	
	//CComPtr<IGPUCPGetParams> ppv;
	//m_pGPUCP->QueryInterface(IID_IGPUCPGetParams,(void**)&ppv);
	
	if(hr == S_OK)// && ppv->IsCryptoSessionAvailable()==FALSE)
	{	
		hr = m_pGPUCP->Open(&temp);
	}
	/*else if(ppv->IsCryptoSessionAvailable())
	{
		hr = m_pGPUCP->Associate(&temp);
	}*/
	return hr;
}

STDMETHODIMP CoCPServiceWin7PAVP::Close()
{		
	DP("GPI cannot close PAVP");
	if(m_pGPUCP)
		m_pGPUCP->FreeResources();
	return S_OK;
}

STDMETHODIMP CoCPServiceWin7PAVP::ScrambleData(BYTE *pDataOut, const BYTE *pDataIn, DWORD dwDataLen)
{
	if(m_pGPUCP == NULL)
		return E_FAIL;
	HRESULT hr = m_pGPUCP->ScrambleData(pDataOut,pDataIn,dwDataLen);
	return hr;
}

STDMETHODIMP CoCPServiceWin7PAVP::EnableScrambling()
{
	if(m_pGPUCP == NULL)
		return E_FAIL;
	HRESULT hr = m_pGPUCP->EnableScrambling();
	return hr;
}

STDMETHODIMP CoCPServiceWin7PAVP::DisableScrambling()
{
	if(m_pGPUCP == NULL)
		return E_FAIL;
	HRESULT hr = m_pGPUCP->DisableScrambling();
	return hr;
}

STDMETHODIMP_(LPVOID) CoCPServiceWin7PAVP::GetIV()
{
	CComPtr<IGPUCPGetParams> pPa;
	HRESULT hr = m_pGPUCP->QueryInterface(IID_IGPUCPGetParams,(void**)&pPa);
	if(hr == S_OK && pPa->IsCryptoSessionAvailable())
	{
		return pPa->GetIV();
	}
	else
		return NULL;
}

STDMETHODIMP_(LPVOID) CoCPServiceWin7PAVP::GetContentKey()
{
	CComPtr<IGPUCPGetParams> pPa;
	HRESULT hr = m_pGPUCP->QueryInterface(IID_IGPUCPGetParams,(void**)&pPa);
	if(hr == S_OK && pPa->IsCryptoSessionAvailable())
	{
		return pPa->GetContentKey();
	}
	else
		return NULL;
}
