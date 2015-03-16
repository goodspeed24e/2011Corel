#include <mmintrin.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include "PavpDevice.h"
#include "assert.h"
#include "Crypto.h"
#include <InitGuid.h>
#include "crypt_data_gen.h"
#include <Process.h>

#define PAVEDPMESSAGE DP

DEFINE_GUID(DXVA2_Intel_Pavp, 0x7460004, 0x7533, 0x4e1a, 0xbd, 0xe3, 0xff, 0x20, 0x6b, 0xf5, 0xce, 0x47);

CPavpDevice* CPavpDevice::m_PavpDevice = NULL;
CUTILcrit CPavpDevice::m_PavpDeviceCS;

CPavpDevice* CPavpDevice::GetInstance(IDirect3DDevice9 *pD3DDevice9)
{
	CUTILautolock lock(&m_PavpDeviceCS);
	if(m_PavpDevice==NULL) 
	{
		m_PavpDevice = new CPavpDevice(pD3DDevice9);
	}

    m_PavpDevice->AddRef();

	return m_PavpDevice;
}

CPavpDevice* CPavpDevice::GetInstance(IMFGetService *pMFGetService)
{
	CUTILautolock lock(&m_PavpDeviceCS);
	if(m_PavpDevice==NULL) 
	{
		m_PavpDevice = new CPavpDevice(pMFGetService);
	}

    m_PavpDevice->AddRef();

	return m_PavpDevice;
}

CPavpDevice::CPavpDevice(IMFGetService *pMFGetService) :
	CIntelAuxiliaryDevice(pMFGetService), 
	m_bIsPresent(FALSE), 
	m_bKeysExchanged(FALSE), 
	m_pCryptImg(NULL), 
	m_pEPID(NULL),
	m_dwAesCounter(0), 
	m_lRefCount(0), 
	m_AudioStreamId(0),
	m_bSessionEstablished(FALSE), 
	m_hProxyThread(NULL), 	
	m_hProcessEvent(NULL), 
	m_hWaitEvent(NULL), 
	m_hhr(E_FAIL),		
	m_bIsCapQueried(FALSE),
	m_bAudio(FALSE)
{
	PAVEDPMESSAGE("Create\n");

	HRESULT hRes   = S_OK;
	UINT	i, uGuids;
	GUID   *pGUIDs = NULL;
	
	ZeroMemory(&m_sPavpCreateDevice, sizeof(PAVP_CREATE_DEVICE));

	m_hProcessEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
	m_hWaitEvent = CreateEvent(NULL,FALSE,FALSE,NULL);

	assert(m_hProxyThread == NULL);

	m_threadstatus = PAVP_PROXY_THREAD_INIT;
	m_hProxyThread=(HANDLE)_beginthreadex(0,0,ProxyThread,(void *)this,0,&m_uiThreadId);
	
	// Query Acceleration GUIDs
	hRes = QueryAccelGuids(&pGUIDs, &uGuids);
	if (FAILED(hRes))
	{
		PAVEDPMESSAGE("Failed to obtain auxiliary device GUIDs code 0x%x.\n", hRes);
		goto cleanup;
	}

	// Check if PAVP GUID is present in the system
	for (i = 0; i < uGuids; i++)
	{
		if (IsEqualGUID(pGUIDs[i], DXVA2_Intel_Pavp))
		{
			m_bIsPresent = TRUE;
			break;
		}
	}

	if (!m_bIsPresent)
	{
		PAVEDPMESSAGE("CPavpDevice: Service not available");
	}	

cleanup:
	if (pGUIDs) free(pGUIDs);
}

CPavpDevice::CPavpDevice(IDirect3DDevice9 *pD3DDevice9) :
	CIntelAuxiliaryDevice(pD3DDevice9), 
	m_bIsPresent(FALSE), 
	m_bKeysExchanged(FALSE), 
	m_pCryptImg(NULL), 
	m_pEPID(NULL),
	m_dwAesCounter(0), 
	m_lRefCount(0), 
	m_AudioStreamId(0),
	m_bSessionEstablished(FALSE), 
	m_hProxyThread(NULL), 	
	m_hProcessEvent(NULL), 
	m_hWaitEvent(NULL), 
	m_hhr(E_FAIL),		
	m_bIsCapQueried(FALSE),	
	m_bAudio(FALSE)
{
	HRESULT hRes   = S_OK;
	UINT	i, uGuids;
	GUID   *pGUIDs = NULL;
	
	ZeroMemory(&m_sPavpCreateDevice, sizeof(PAVP_CREATE_DEVICE));

	m_hProcessEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
	m_hWaitEvent = CreateEvent(NULL,FALSE,FALSE,NULL);

	assert(m_hProxyThread == NULL);

	m_threadstatus = PAVP_PROXY_THREAD_INIT;
	m_hProxyThread=(HANDLE)_beginthreadex(0,0,ProxyThread,(void *)this,0,&m_uiThreadId);
	
	// Query Acceleration GUIDs
	hRes = QueryAccelGuids(&pGUIDs, &uGuids);
	if (FAILED(hRes))
	{
		PAVEDPMESSAGE("Failed to obtain auxiliary device GUIDs code 0x%x.\n", hRes);
		goto cleanup;
	}

	// Check if PAVP GUID is present in the system
	for (i = 0; i < uGuids; i++)
	{
		if (IsEqualGUID(pGUIDs[i], DXVA2_Intel_Pavp))
		{
			m_bIsPresent = TRUE;
			break;
		}
	}

	if (!m_bIsPresent)
	{
		PAVEDPMESSAGE("CPavpDevice: Service not available");
	}	

cleanup:
	if (pGUIDs) free(pGUIDs);
}

CPavpDevice::~CPavpDevice()
{
	PAVEDPMESSAGE("Destroy Begin\n");
	m_PavpDevice = NULL;

	SAFE_DELETE(m_pCryptImg);		

	m_threadstatus = PAVP_PROXY_THREAD_CLEANUP_EPID;
	SetEvent(m_hProcessEvent);
	WaitForSingleObject(m_hWaitEvent, INFINITE);

	m_threadstatus = PAVP_PROXY_THREAD_CLEAN_AUXILIARY_DEVICE;
	SetEvent(m_hProcessEvent);
	WaitForSingleObject(m_hProxyThread, INFINITE);

	CloseHandle(m_hProxyThread);
	CloseHandle(m_hProcessEvent);
	CloseHandle(m_hWaitEvent);
	PAVEDPMESSAGE("Destroy End\n");
}

ULONG CPavpDevice::AddRef()
{
	return InterlockedIncrement(&m_lRefCount);
}

ULONG CPavpDevice::Release()
{
	LONG lRefCount = InterlockedDecrement(&m_lRefCount);
	assert(lRefCount >= 0);

	if (0 == lRefCount)	
		delete this;

	return lRefCount;
}

unsigned int CPavpDevice::ProxyThread(void *arg)
{
	CPavpDevice	*pPAVPDevice = (CPavpDevice *)arg;

	HRESULT hr = E_FAIL;
	BOOL flag = TRUE;

	while(flag)
	{
		WaitForSingleObject(pPAVPDevice->m_hProcessEvent, INFINITE);
		switch(pPAVPDevice->m_threadstatus)
		{		
			case PAVP_PROXY_THREAD_OPEN_KEY_EXCHANGE:
				//pPAVPDevice->DP("ThreadTest():_OpenKeyExchange()");
				pPAVPDevice->m_hhr = pPAVPDevice->_OpenKeyExchange();
				break;
			case PAVP_PROXY_THREAD_CLEANUP_EPID:
				//pPAVPDevice->DP("ThreadTest():CLEANUP_EPID");
				if(pPAVPDevice->m_bSessionEstablished && pPAVPDevice->IsPAVP15())
				{
					SAFE_DELETE(pPAVPDevice->m_pEPID);	
				}
				break;
			case PAVP_PROXY_THREAD_UPDATE_STREAM_KEY:
				//pPAVPDevice->DP("ThreadTest():_UpdateStreamKey()");
				pPAVPDevice->m_hhr = pPAVPDevice->_UpdateStreamKey(pPAVPDevice->m_bAudio);
				break;
			case PAVP_PROXY_THREAD_INVALIDATE_STREAM_KEY:
				//pPAVPDevice->DP("ThreadTest():_InvalidateStreamKey()");
				pPAVPDevice->m_hhr = pPAVPDevice->_InvalidateStreamKey(pPAVPDevice->m_bAudio);
				break;				
			case PAVP_PROXY_THREAD_CLEAN_AUXILIARY_DEVICE:
				//pPAVPDevice->DP("ThreadTest():_CleanAuxiliaryDevice()");
				pPAVPDevice->_CleanAuxiliaryDevice();
				flag = FALSE;
				break;
			default:
				break;
		}
		SetEvent(pPAVPDevice->m_hWaitEvent);
	}

	return 0;
}

HRESULT CPavpDevice::_OpenKeyExchange()
{
	HRESULT hr=E_FAIL;
	CdgStatus CryptoLibStatus = CdgStsOk;

	DP("QueryCaps()\n", hr);
	hr = _QueryCaps();	
	if(FAILED(hr))
	{
		PAVEDPMESSAGE("QueryCaps failed error : 0x%x", hr);
		return hr;
	}	

	DP("CreateService()\n", hr);
	hr = _CreateService();		
	if(FAILED(hr))
	{
		PAVEDPMESSAGE("CreateService failed error : 0x%x", hr);
		return hr;
	}

	DP("DoKeyExchange()\n", hr);
	hr = _DoKeyExchange();	
	if (FAILED(hr))
	{
		PAVEDPMESSAGE("DoKeyExchange failed error : 0x%x", hr);
		return hr;
	}

	return hr;

}

HRESULT CPavpDevice::Open(BOOL bAudio, BOOL bShowMsg)
{
	PAVEDPMESSAGE("Open()");

	HRESULT hr = S_OK;	

	m_bAudio = bAudio;
	m_bShowMsg = bShowMsg;
	m_pCryptImg = new CryptographicLibrary;	

	m_threadstatus = PAVP_PROXY_THREAD_OPEN_KEY_EXCHANGE;
	SetEvent(m_hProcessEvent);
	WaitForSingleObject(m_hWaitEvent, INFINITE);

	if(FAILED(m_hhr))
		return m_hhr;

	/*hr = _QueryCaps();
	DP("QueryCaps()\n", hr);
	if(FAILED(hr))
	{
		PAVEDPMESSAGE("QueryCaps failed error : 0x%x", hr);
		return hr;
	}	

	hr = _CreateService();	
	DP("CreateService()\n", hr);
	if(FAILED(hr))
	{
		PAVEDPMESSAGE("CreateService failed error : 0x%x", hr);
		return hr;
	}

	hr = _DoKeyExchange();
	DP("DoKeyExchange()\n", hr);
	if (FAILED(hr))
	{
		PAVEDPMESSAGE("DoKeyExchange failed error : 0x%x", hr);
		return hr;
	}*/

	return hr;
}

HRESULT CPavpDevice::_QueryCaps()
{	
	if (!m_bIsCapQueried)
	{
		HRESULT hRes;
		UINT	uQuerySize;
		PAVP_QUERY_CAPS sQuery;
		uQuerySize  = sizeof(sQuery);

		hRes = QueryAccelCaps(&DXVA2_Intel_Pavp, &sQuery, &uQuerySize);
		//PAVEDPMESSAGE("QueryCaps: AvailableKeyExchangeProtocols = 0x%x", sQuery.AvailableKeyExchangeProtocols);
		if (FAILED(hRes))
		{
			PAVEDPMESSAGE("QueryCaps: Failed to query caps");
			return hRes;
		}

		// Verify the key exchange protocol that will be used is supported
		if (sQuery.AvailableKeyExchangeProtocols & PAVP_KEY_EXCHANGE_DAA)
		{			
			PAVP_CREATE_DEVICE sPavpCreateDevice = {PAVP_KEY_EXCHANGE_DAA};
			_SetPavpDevice(&sPavpCreateDevice);
			PAVEDPMESSAGE("Key Exchange: DAA(PAVP 1.5)");
		}
		else if (sQuery.AvailableKeyExchangeProtocols & PAVP_KEY_EXCHANGE_CANTIGA)
		{
			PAVP_CREATE_DEVICE sPavpCreateDevice = {PAVP_KEY_EXCHANGE_CANTIGA};
			_SetPavpDevice(&sPavpCreateDevice);
			PAVEDPMESSAGE("Key Exchange: Cantiga");
		}
		else if (sQuery.AvailableKeyExchangeProtocols & PAVP_KEY_EXCHANGE_EAGLELAKE)
		{
			PAVP_CREATE_DEVICE sPavpCreateDevice = {PAVP_KEY_EXCHANGE_EAGLELAKE};
			_SetPavpDevice(&sPavpCreateDevice);
			PAVEDPMESSAGE("Key Exchange: Eaglelake");
		}
		else if (sQuery.AvailableKeyExchangeProtocols & PAVP_KEY_EXCHANGE_LARRABEE)
		{
			PAVP_CREATE_DEVICE sPavpCreateDevice = {PAVP_KEY_EXCHANGE_LARRABEE};
			_SetPavpDevice(&sPavpCreateDevice);
			PAVEDPMESSAGE("Key Exchange: Larrabee");
		}
		else if (sQuery.AvailableKeyExchangeProtocols & PAVP_KEY_EXCHANGE_IRONLAKE)
		{			
			PAVP_CREATE_DEVICE sPavpCreateDevice = {PAVP_KEY_EXCHANGE_IRONLAKE};
			_SetPavpDevice(&sPavpCreateDevice);
			PAVEDPMESSAGE("Key Exchange: Ironlake");
		}
		else
		{
			PAVEDPMESSAGE("Required key exchange protocol unavailable.");
			return E_FAIL;
		}

		m_bIsCapQueried = TRUE;

		return hRes;
	}
	else
	{
		return S_OK;
	}
}

HRESULT CPavpDevice::_CreateService()
{
	HRESULT hRes;
	UINT  uCreateSize = sizeof(PAVP_CREATE_DEVICE);
	hRes = CreateAccelService(&DXVA2_Intel_Pavp, &m_sPavpCreateDevice, &uCreateSize);
	if (FAILED(hRes))
	{
		PAVEDPMESSAGE("CreateService: Failed to create service code 0x%x\n", hRes);
	}

	return hRes;
}

HRESULT CPavpDevice::_DoKeyExchange()
{
	HRESULT hr = S_OK;

	if (IsPAVP15())
	{		
		hr = _DoEPIDKeyExchange();
	}
	else
	{
		hr = _DoFixedKeyExchange();
	}

	return hr;
}


HRESULT CPavpDevice::_DoFixedKeyExchange()
{
	HRESULT hRes;
	DXVA2_DecodeExecuteParams sExecute;
	DXVA2_DecodeExtensionData sExtension;
	PAVP_FIXED_EXCHANGE_PARAMS KeyExchangeParams;

	DWORD  dwFixedKey[4];
	DWORD  dwEncryptedFixedKey[4];

	if(m_sPavpCreateDevice.eDesiredKeyExchangeMode & PAVP_KEY_EXCHANGE_CANTIGA)
	{
		//These keys are provided by Intel.  They need to be protected
		//	DWORD dwFixedKey[4]				= {0x2c2b4115, 0x6441122b, 0x042c5505, 0x04171b2e}
		//	DWORD dwEncryptedFixedKey[4]	= {0x9df76f5e, 0xee8576f0, 0x2cc0c3cb, 0xc1ce796d}; 
		dwEncryptedFixedKey[0]	= 0x9df76f5e;
		dwEncryptedFixedKey[1]	= 0xee8576f0;
		dwEncryptedFixedKey[2]	= 0x2cc0c3cb;
		dwEncryptedFixedKey[3]	= 0xc1ce796d; 

		DWORD IviKey[4] = {0x939d007b, 0x5685eab9, 0xde170be5, 0x9771bac0};
		BYTE IviCypherText[16] = {0x3e, 0xc4, 0x45, 0x00, 0x01, 0x9b, 0x86, 0x4d, 0xc5, 0xb8, 0x03, 0xe5, 0x84, 0x55, 0xb8, 0x5a};

		// Decrypt IviEncryptedKey to obtain Intel real FixedKey {0x2c2b4115, 0x6441122b, 0x042c5505, 0x04171b2e}
		m_pCryptImg->AES_128D((BYTE*)&IviKey, (BYTE*)&IviCypherText, (BYTE*)&dwFixedKey);
	}
	else if(m_sPavpCreateDevice.eDesiredKeyExchangeMode & PAVP_KEY_EXCHANGE_EAGLELAKE)
	{

		//These keys are provided by Intel.  They need to be protected
		//	DWORD  dwFixedKeyEglA2[4]			= {0x3b14e7e6, 0x9a925bad, 0xaa6e7172, 0x55ef99a5};
		//	DWORD  dwEncryptedFixedKeyEglA2[4]	= {0xd5cb367f, 0x1a96aab7, 0x63d3cba7, 0xfd62d894};
		dwEncryptedFixedKey[0]		= 0xd5cb367f;
		dwEncryptedFixedKey[1]		= 0x1a96aab7;
		dwEncryptedFixedKey[2]		= 0x63d3cba7;
		dwEncryptedFixedKey[3]		= 0xfd62d894;

		DWORD IviKey[4] = {0xd0da671d, 0x5b67fc88, 0x89acb5ca, 0xa5cea2a1};
		BYTE IviCypherText[16] = {0x4e, 0xcc, 0x33, 0x21, 0x99, 0x55, 0xed, 0x91, 0xe7, 0x45, 0x79, 0x97, 0x81, 0x16, 0xa2, 0xb5};

		// Decrypt IviEncryptedKey to obtain Intel real FixedKey {0x3b14e7e6, 0x9a925bad, 0xaa6e7172, 0x55ef99a5}
		m_pCryptImg->AES_128D((BYTE*)&IviKey, (BYTE*)&IviCypherText, (BYTE*)&dwFixedKey);
	}
	else if(m_sPavpCreateDevice.eDesiredKeyExchangeMode & PAVP_KEY_EXCHANGE_IRONLAKE)
	{
		//These keys are provided by Intel.  They need to be protected
		//	DWORD dwFixedKey[4]				= {0xb4141f96, 0x156b491f, 0xdbe38305, 0xb2ab8f24}
		//	DWORD dwEncryptedFixedKey[4]	= {0xfcebdb4a, 0xc311d5d9, 0x96a6aa05, 0x9cb2d98b}; 
		dwEncryptedFixedKey[0]	= 0xfcebdb4a;
		dwEncryptedFixedKey[1]	= 0xc311d5d9;
		dwEncryptedFixedKey[2]	= 0x96a6aa05;
		dwEncryptedFixedKey[3]	= 0x9cb2d98b; 

		DWORD IviKey[4] = {0x8962e610, 0x8de24f86, 0xbc6f8d78, 0x5f337b98};
		BYTE IviCypherText[16] = {0x43, 0x12, 0x8d, 0xe2, 0xfe, 0xeb, 0xd5, 0x9f, 0xef, 0xe4, 0xb5, 0x82, 0xd9, 0xfc, 0xb7, 0xff};

		// Decrypt IviEncryptedKey to obtain Intel real FixedKey {0xb4141f96, 0x156b491f, 0xdbe38305, 0xb2ab8f24}
		m_pCryptImg->AES_128D((BYTE*)&IviKey, (BYTE*)&IviCypherText, (BYTE*)&dwFixedKey);
	}

	// Create a random 128-bit key and encrypt it with the fixed key
	BYTE EncryptedSessionKey[16];
	memset(&EncryptedSessionKey, 0, 16);

	m_pCryptImg->RNG_ANSI_X931_128((BYTE*)&dwFixedKey, (BYTE*)&m_dwSessionKey);
	m_pCryptImg->AES_128E((BYTE*)&dwFixedKey, (BYTE*)&m_dwSessionKey, (BYTE*)&EncryptedSessionKey);
	memset(&dwFixedKey, 0, 16);

	memcpy(&KeyExchangeParams.SessionKey, EncryptedSessionKey, sizeof(EncryptedSessionKey));
	memcpy(&KeyExchangeParams.FixedKey, dwEncryptedFixedKey, sizeof(dwEncryptedFixedKey));

	// PAVP Key exchange
	sExtension.Function			  = PAVP_KEY_EXCHANGE;
	sExtension.pPrivateInputData	 = (PVOID)&KeyExchangeParams;
	sExtension.PrivateInputDataSize  = sizeof(KeyExchangeParams);
	sExtension.pPrivateOutputData	= NULL;
	sExtension.PrivateOutputDataSize = 0;

	sExecute.NumCompBuffers	 = 0;
	sExecute.pCompressedBuffers = 0;
	sExecute.pExtensionData	 = &sExtension;

	hRes = m_pAuxiliaryDevice->Execute(&sExecute);

	PAVEDPMESSAGE("AuxiliaryDevice::Execute() :PAVP_KEY_EXCHANGE, ret-0x%x\n", hRes);

	if (SUCCEEDED(hRes))
	{
		PAVEDPMESSAGE("Do Key Exchanged");
		m_bKeysExchanged = TRUE;
	}

	return hRes;
}

HRESULT CPavpDevice::_DoEPIDKeyExchange()
{
	HRESULT hr = S_OK;

	m_pEPID = new CEPIDKeyExchange(FALSE);

	if (m_bAudio)
		SetAudioStreamId(m_AudioStreamId);

	hr = m_pEPID->Open(this, &CPavpDevice::EPIDExec, NULL, NULL, m_bShowMsg);
	if(FAILED(hr))
		return hr;

	hr = m_pEPID->CreateSession();
	if(FAILED(hr))
		return hr;

	m_bSessionEstablished = TRUE;

	if(m_bAudio)
	{
		hr = m_pEPID->UpdateStreamKey((StreamKey*)m_AudioStreamKey, NULL, m_bAudio);	
	}
	else
	{
		hr = m_pEPID->UpdateStreamKey((StreamKey*)m_VideoStreamKey, NULL, m_bAudio);	
	}	

	return hr;
}

// Obtains a new stream key and invalidates the previous one if necessary
HRESULT CPavpDevice::UpdateStreamKey(BOOL bAudio)
{
	HRESULT hr = S_OK;

	m_bAudio = bAudio;
	m_threadstatus = PAVP_PROXY_THREAD_UPDATE_STREAM_KEY;
	SetEvent(m_hProcessEvent);
	WaitForSingleObject(m_hWaitEvent, INFINITE);

	if(FAILED(m_hhr))
		return m_hhr;

	return hr;
}

HRESULT CPavpDevice::Close(BOOL bAudio)
{
	HRESULT hr = S_OK;
	
	if((IsPAVP15()) && (m_pEPID!=NULL) &&(bAudio))
	{			
		m_bAudio = bAudio;
		m_threadstatus = PAVP_PROXY_THREAD_INVALIDATE_STREAM_KEY;
		SetEvent(m_hProcessEvent);
		WaitForSingleObject(m_hWaitEvent, INFINITE);

		if(FAILED(m_hhr))
			return m_hhr;		
	}

	return hr;
}

HRESULT CPavpDevice::_InvalidateStreamKey(BOOL bAudio)
{
	HRESULT hr = S_OK;

	if(IsPAVP15() && (m_pEPID!=NULL))
	{
		m_pEPID->InvalidateStreamKey(NULL, bAudio);	
	}
	
	return hr;
}

// Obtains a new stream key and invalidates the previous one if necessary
HRESULT CPavpDevice::_UpdateStreamKey(BOOL bAudio)
{
	HRESULT hr = S_OK;

	if (bAudio)
	{
		m_pEPID->UpdateStreamKey((StreamKey*)m_AudioStreamKey, this, bAudio);	
	}
	else
	{
		m_pEPID->UpdateStreamKey((StreamKey*)m_VideoStreamKey, this, bAudio);	
	}

	return hr;
}

void CPavpDevice::_CleanAuxiliaryDevice()
{
	if (m_pAuxiliaryDevice)
	{
		m_pAuxiliaryDevice->Release();
		m_pAuxiliaryDevice = NULL;
	}
}

HRESULT CPavpDevice::_SetAESCounter(PVOID pAccel, DWORD dwBufferSize, DWORD dwAesCounter)
{
	IDirectXVideoDecoder* pTemp = (IDirectXVideoDecoder*)pAccel;
	HRESULT hRes;
	DXVA2_DecodeExecuteParams sExecute;
	DXVA2_DecodeExtensionData sExtension;
	DXVA_Intel_Pavp_Protocol PavpProtocolInput;
	DXVA_Intel_Pavp_Protocol PavpProtocolOutput;

	PavpProtocolInput.EncryptProtocolHeader.dwFunction = 0xffff0001;
	PavpProtocolInput.EncryptProtocolHeader.guidEncryptProtocol = DXVA2_Intel_Pavp;
	PavpProtocolInput.dwBufferSize = dwBufferSize;
	PavpProtocolInput.dwAesCounter = dwAesCounter;

	sExtension.Function			  = 0;
	
	sExtension.pPrivateInputData	 = (PVOID)&PavpProtocolInput;
	sExtension.PrivateInputDataSize  = sizeof(DXVA_Intel_Pavp_Protocol);
	
	sExtension.pPrivateOutputData	= (PVOID)&PavpProtocolOutput.EncryptProtocolHeader;
	sExtension.PrivateOutputDataSize = sizeof(DXVA_EncryptProtocolHeader);

	sExecute.NumCompBuffers	 = 0;
	sExecute.pCompressedBuffers = 0;
	sExecute.pExtensionData	 = &sExtension;

	hRes = pTemp->Execute(&sExecute);	// Intel's workaround: Should use PAVP device?
	if(FAILED(hRes))
		hRes = pTemp->Execute(&sExecute);

	PAVEDPMESSAGE("AuxiliaryDevice::Execute() :PAVP_PROTOCOL, ret-0x%x\n", hRes);

	return hRes;
}

HRESULT CPavpDevice::GetFreshnessValue()
{
	HRESULT hRes;
	DXVA2_DecodeExecuteParams sExecute;
	DXVA2_DecodeExtensionData sExtension;
	PAVP_GET_FRESHNESS_PARAMS GetFreshnessParams;

	if (!m_bKeysExchanged)
	{
		PAVEDPMESSAGE("GetFreshnessValue: Service has not been initialized");
		return E_FAIL;
	}

	// PAVP get freshness
	sExtension.Function			  = PAVP_GET_FRESHNESS;
	sExtension.pPrivateInputData	 = NULL;
	sExtension.PrivateInputDataSize  = 0;
	sExtension.pPrivateOutputData	= (PVOID)&GetFreshnessParams;
	sExtension.PrivateOutputDataSize = sizeof(GetFreshnessParams);

	sExecute.NumCompBuffers	 = 0;
	sExecute.pCompressedBuffers = 0;
	sExecute.pExtensionData	 = &sExtension;

	hRes = m_pAuxiliaryDevice->Execute(&sExecute);

	return hRes;
}

HRESULT CPavpDevice::UseFreshnessValue()
{
	HRESULT hRes;
	DXVA2_DecodeExecuteParams sExecute;
	DXVA2_DecodeExtensionData sExtension;

	if (!m_bKeysExchanged)
	{
		PAVEDPMESSAGE("SetFreshnessValue: Service has not been initialized");
		return E_FAIL;
	}

	// PAVP set freshness
	sExtension.Function			  = PAVP_SET_FRESHNESS;
	sExtension.pPrivateInputData	 = NULL;
	sExtension.PrivateInputDataSize  = 0;
	sExtension.pPrivateOutputData	= NULL;
	sExtension.PrivateOutputDataSize = 0;

	sExecute.NumCompBuffers	 = 0;
	sExecute.pCompressedBuffers = 0;
	sExecute.pExtensionData	 = &sExtension;

	hRes = m_pAuxiliaryDevice->Execute(&sExecute);

	return hRes;
}

HRESULT CPavpDevice::GetConnectionState()
{
	HRESULT hRes;
	DXVA2_DecodeExecuteParams sExecute;
	DXVA2_DecodeExtensionData sExtension;
	PAVP_GET_CONNECTION_STATE_PARAMS GetConnectionStateParams;

	// until hardware is available, this is hardcoded and doesn't really
	// provide useful information
	const DWORD dwExpectedOutput[] = { 0, 0, 0, 1 };

	if (!m_bKeysExchanged)
	{
		PAVEDPMESSAGE("GetConnectionState: Service has not been initialized");
		return E_FAIL;
	}

	// PAVP get hardware display output connection state
	sExtension.Function			  = PAVP_GET_CONNECTION_STATE;
	sExtension.pPrivateInputData	 = NULL;
	sExtension.PrivateInputDataSize  = 0;
	sExtension.pPrivateOutputData	= (PVOID)&GetConnectionStateParams;
	sExtension.PrivateOutputDataSize = sizeof(GetConnectionStateParams);

	sExecute.NumCompBuffers	 = 0;
	sExecute.pCompressedBuffers = 0;
	sExecute.pExtensionData	 = &sExtension;

	hRes = m_pAuxiliaryDevice->Execute(&sExecute);

	// Verify the protected memory is still in Paranoid mode
	if (memcmp(dwExpectedOutput, GetConnectionStateParams.ProtectedMemoryStatus, sizeof(dwExpectedOutput)))
	{
		PAVEDPMESSAGE("GetConnectionState: hardware mode invalid");
		return E_FAIL;
	}

	// Verify the display output port configuration is still as expected
	if (GetConnectionStateParams.PortStatusType == 1)
	{
	}
	else
	{
		PAVEDPMESSAGE("GetConnectionState: hardware port status type invalid");
		return E_FAIL;
	}
	return hRes;
}

HRESULT CPavpDevice::ScrambleData(PVOID pAccel, const byte *input, byte *output, int bufLen)
{
	HRESULT hRes;

	CUTILautolock lock(&m_PavpDeviceCS);

	if (IsPAVP15())
	{
		CdgStatus Status = CdgStsOk;

		if(m_dwAesCounter > 0xEFFFFFFF)
			m_dwAesCounter = 0; //reset to zero when overflow

		hRes = _SetAESCounter(pAccel, (DWORD)bufLen, m_dwAesCounter);

		if (FAILED(hRes))
		{
			PAVEDPMESSAGE("ScrambleData: Failed to set AES counter");
			return hRes;
		}

		Status = Aes128CtrEncrypt((unsigned char *)input, bufLen, (unsigned char *)output, bufLen, 
			m_VideoStreamKey, PAVP_EPID_STREAM_KEY_LEN, (unsigned int)m_dwAesCounter);

		if(Status != CdgStsOk) 
		{
			PAVEDPMESSAGE("ScrambleData: Failed to AES Encrypt");
			hRes = E_FAIL;
			return hRes;
		}

		m_dwAesCounter += (bufLen/16);
	}
	else
	{
		if(m_dwAesCounter > 0xEFFFFFFF)
			m_dwAesCounter = 0; //reset to zero when overflow

		hRes = _SetAESCounter(pAccel, (DWORD)bufLen, m_dwAesCounter);
		if (FAILED(hRes))
		{
			PAVEDPMESSAGE("ScrambleData: Failed to set AES counter");
			return hRes;
		}

		m_pCryptImg->AES_128CTR_PAVP((BYTE*)&m_dwSessionKey, input, output, bufLen, m_dwAesCounter);

		m_dwAesCounter += (bufLen/16);
	}

	return S_OK;
}

HRESULT CPavpDevice::ScrambleDataAudio(const byte *input, byte *output, int bufLen)
{
	HRESULT hRes;
	CdgStatus Status = CdgStsOk;

	Status = Aes128EcbEncrypt((unsigned char *)input, bufLen, (unsigned char *)output, bufLen, 
		m_AudioStreamKey, PAVP_EPID_STREAM_KEY_LEN);

	if(Status != CdgStsOk) 
	{
		PAVEDPMESSAGE("ScrambleData: Failed to AES Encrypt");
		hRes = E_FAIL;
		return hRes;
	}

	return S_OK;
}

HRESULT CPavpDevice::EPIDExec(PVOID pvContext, LPVOID pInBuf, DWORD dwInBufLen, LPVOID pOutBuf, DWORD dwOutBufLen)
{
	HRESULT hr = S_OK;

	CPavpDevice *pd;
	pd = reinterpret_cast<CPavpDevice *>(pvContext);

	hr = pd->ExecuteFunction(PAVP_KEY_EXCHANGE, 
		pInBuf, dwInBufLen, 
		pOutBuf, dwOutBufLen );
	
	return hr;
}

BOOL CPavpDevice::IsPAVP15()
{
	HRESULT hr = E_FAIL;
	
	hr = _QueryCaps();	
	if(FAILED(hr))
	{
		PAVEDPMESSAGE("QueryCaps failed error : 0x%x", hr);		
	}

	if (m_sPavpCreateDevice.eDesiredKeyExchangeMode == PAVP_KEY_EXCHANGE_DAA)
		return TRUE;
	else 
		return FALSE;	
}

BOOL CPavpDevice::IsPAVP10()
{
	HRESULT hr = E_FAIL;

	hr = _QueryCaps();	
	if(FAILED(hr))
	{
		PAVEDPMESSAGE("QueryCaps failed error : 0x%x", hr);		
	}
	
	if ((m_sPavpCreateDevice.eDesiredKeyExchangeMode == PAVP_KEY_EXCHANGE_CANTIGA) ||
		(m_sPavpCreateDevice.eDesiredKeyExchangeMode == PAVP_KEY_EXCHANGE_EAGLELAKE) ||
		(m_sPavpCreateDevice.eDesiredKeyExchangeMode == PAVP_KEY_EXCHANGE_LARRABEE) ||
		(m_sPavpCreateDevice.eDesiredKeyExchangeMode == PAVP_KEY_EXCHANGE_IRONLAKE))
		return TRUE;
	else 
		return FALSE;	
}

void CPavpDevice::SetAudioStreamId(PAVPStreamId StreamId)
{
	m_AudioStreamId = StreamId;
	if (m_pEPID!=NULL)
		m_pEPID->SetAudioStreamID(StreamId);	
}	