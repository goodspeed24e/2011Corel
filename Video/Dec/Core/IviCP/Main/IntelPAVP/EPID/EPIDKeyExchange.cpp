
#include "EPIDKeyExchange.h"
#include <stdio.h> //for _vsnprintf_s
#include "crypt_data_gen.h"
#include "Crypto.h" //for CryptographicLibrary
#include <Process.h> // for thread
#include "../../CPDefines.h"

#define DW_SwapEndian(dw_value)	( (((dw_value) & 0x000000ff) << 24) |  (((dw_value) & 0x0000ff00) << 8) | (((dw_value) & 0x00ff0000) >> 8) | (((dw_value) & 0xff000000) >> 24) )
#define APP_ID 0x00000001

#define WAIT_TIME_FOR_CREATE_SESSION INFINITE 

CEPIDKeyExchange* CEPIDKeyExchange::m_pEPID = NULL;
CUTILcrit CEPIDKeyExchange::m_csEPID;

CEPIDKeyExchange* CEPIDKeyExchange::GetInstance(BOOL bUseProxyThread)
{
	CUTILautolock lock(&m_csEPID);
	if (m_pEPID == NULL) 
	{
		m_pEPID = new CEPIDKeyExchange(bUseProxyThread);
	}

	m_pEPID->AddRef();

	return m_pEPID;
}

ULONG CEPIDKeyExchange::AddRef()
{
	return InterlockedIncrement(&m_lRefCount);
}

ULONG CEPIDKeyExchange::Release(PVOID pvDataContext)
{	
	CUTILautolock lock(&m_csEPID);

	if(pvDataContext!=NULL)
		m_pvDataContext = pvDataContext;

	LONG lRefCount = InterlockedDecrement(&m_lRefCount);	

	if (0 == lRefCount)	
		delete this;

	return lRefCount;
}

HRESULT CEPIDKeyExchange::_ResetParams()
{
	m_hProxyThread = NULL;
	m_hProcessEvent = NULL;
	m_hWaitEvent = NULL;
	m_threadstatus = PROXY_THREAD_CLOSE;

	m_pvDataContext = NULL;
	m_pfnEPIDExecCallback = NULL;
	m_bShowMsg = FALSE;
	m_bAudio = FALSE;	

	m_bSessionIsCreated = FALSE;		
	m_bGetVideoStreamKey = FALSE; 
	m_bGetAudioStreamKey = FALSE; 	
	m_bChangeVideoStreamKey = FALSE;

	m_hSessionIsCreated = NULL;		
	m_hGotVideoStreamKey = NULL;
	m_hGotAudioStreamKey = NULL;

	m_SigmaSessionId = 0;
	m_AudioStreamId = 0;

	ZeroMemory(&m_SessionKey,	sizeof(SigmaSessionKey));
	ZeroMemory(&m_StreamKey,	sizeof(StreamKey));
	ZeroMemory(&m_SigningKey,	sizeof(SigmaMacKey));
	ZeroMemory(&m_VideoStreamKey,	sizeof(StreamKey));
	ZeroMemory(&m_AudioStreamKey,	sizeof(StreamKey));

	m_pSharedData = NULL;
	m_pExtObj = NULL;	

	return S_OK;

}

CEPIDKeyExchange::CEPIDKeyExchange(BOOL bUseProxyThread)
{
	m_lRefCount = 0;
	
	m_bUseProxyThread = bUseProxyThread;

	_ResetParams();	
	
	if(m_bUseProxyThread)
	{	
		m_hProcessEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
		m_hWaitEvent = CreateEvent(NULL,FALSE,FALSE,NULL);		
		m_threadstatus = PROXY_THREAD_CLOSE;
	}

	m_hSessionIsCreated = CreateEvent(NULL,TRUE,FALSE,NULL);		
	m_hGotVideoStreamKey = CreateEvent(NULL,TRUE,FALSE,NULL);
	m_hGotAudioStreamKey = CreateEvent(NULL,TRUE,FALSE,NULL);
			
}

CEPIDKeyExchange::~CEPIDKeyExchange()
{	
	HRESULT hr = S_OK;

	if(m_bUseProxyThread)
	{	
		if(m_threadstatus!=PROXY_THREAD_CLOSE)
		{	
			m_threadstatus = PROXY_THREAD_CLOSE;
			SetEvent(m_hProcessEvent);
			WaitForSingleObject(m_hWaitEvent, INFINITE);

			if(m_threadstatus == PROXY_THREAD_ERROR)
				hr = E_FAIL;

			CloseHandle(m_hProxyThread);
		}
		CloseHandle(m_hProcessEvent);
		CloseHandle(m_hWaitEvent);	
	}
	else
	{
		if(IsSessionCreated())
		{
			hr = _Close();					
		}
	}

	if(FAILED(hr))
	{
		DP("failed to close");	
	}
	else
	{
		DP("succeeded to close");
	}

	m_pEPID = NULL;	
}


HRESULT CEPIDKeyExchange::Open(PVOID pvDataContext, PFN_EPID_EXEC pfnEPIDExecCallback, IUnknown *pExtObj, LPVOID pSharedData, BOOL bShowMsg)
{
	HRESULT hr = S_OK;	
	CUTILautolock lock(&m_csEPID);	

	if(pExtObj != NULL)
	{
		pExtObj->QueryInterface(IID_IUnknown, (void**)&m_pExtObj);
	}

	m_pvDataContext = pvDataContext;
	m_pfnEPIDExecCallback = pfnEPIDExecCallback;	
	m_pSharedData = pSharedData;	
	m_bShowMsg = bShowMsg;

	if(m_bUseProxyThread)
	{	
		UINT32 m_uiThreadId;
		m_hProxyThread = (HANDLE)_beginthreadex(0, 0, ProxyThread, (void *)this, 0, &m_uiThreadId);

		m_threadstatus = PROXY_THREAD_OPEN;
		SetEvent(m_hProcessEvent);
		WaitForSingleObject(m_hWaitEvent, INFINITE);

		if(m_threadstatus == PROXY_THREAD_ERROR)
			hr = E_FAIL;
	}
	else
	{
		CdgStatus status = CdgStsOk;

		status = InitializeCdg();
		if( CdgStsOk != status )
		{
			hr = E_FAIL;
		}
	}

	if(FAILED(hr))
	{
		DP("failed to open");	
	}
	else
	{
		DP("succeeded to open");
	}

	return hr;
}

unsigned int CEPIDKeyExchange::ProxyThread(void *arg)
{
	CEPIDKeyExchange *pEPID = (CEPIDKeyExchange *)arg;
	BOOL bExit = FALSE;
	CdgStatus status = CdgStsOk;
	HRESULT hr = E_FAIL;

	do
	{
		WaitForSingleObject(pEPID->m_hProcessEvent, INFINITE);
		switch (pEPID->m_threadstatus)
		{
		case PROXY_THREAD_OPEN:
			status = InitializeCdg();
			if( CdgStsOk != status )
			{
				pEPID->m_threadstatus = PROXY_THREAD_ERROR;
			}
			
			break;

		case PROXY_THREAD_CREATE_SESSION:
			
			hr = pEPID->_CreateSession();
			if(FAILED(hr))
			{				
				pEPID->m_threadstatus = PROXY_THREAD_ERROR;
			}	
			
			break;

		case PROXY_THREAD_UPDATE_KEY:

			hr = pEPID->_UpdateStreamKey();
			if(FAILED(hr))
			{				
				pEPID->m_threadstatus = PROXY_THREAD_ERROR;
			}	
			
			break;

		case PROXY_THREAD_INVALIDATE_KEY:

			hr = pEPID->_InvalidateStreamKey();
			if(FAILED(hr))
			{				
				pEPID->m_threadstatus = PROXY_THREAD_ERROR;
			}	

			break;

		case PROXY_THREAD_CLOSE:
					
			if(pEPID->IsSessionCreated())
			{
				hr = pEPID->_Close();		
				if(FAILED(hr))
				{
					pEPID->m_threadstatus = PROXY_THREAD_ERROR;
				}	
			}

			bExit = TRUE;
			break;		
		default:
			break;
		}
		
		SetEvent(pEPID->m_hWaitEvent);

	}while(!bExit);

	return 0;
}

HRESULT CEPIDKeyExchange::CreateSession()
{
	HRESULT hr = S_OK;
	CUTILautolock lock(&m_csEPID);
	
	if(m_bUseProxyThread)
	{	
		m_threadstatus = PROXY_THREAD_CREATE_SESSION;
		SetEvent(m_hProcessEvent);
		WaitForSingleObject(m_hWaitEvent, INFINITE);

		if(m_threadstatus == PROXY_THREAD_ERROR)
			hr = E_FAIL;
	}
	else
	{
		hr = _CreateSession();		
	}

	if(FAILED(hr))
	{
		DP("failed to create session!!!");
		hr = E_FAIL;
	}	
	else
	{
		DP("succeeded to create session!!!");
	}

	return hr;
}

HRESULT CEPIDKeyExchange::_CreateSession()
{
	HRESULT hRes = S_OK;

	SigmaPubKey					HwPublicEccKey = {0};			// g^a
	SigmaPubKey					AppPublicEccKey = {0};			// g^b
	BYTE						GaGb[2 * SIGMA_PUBLIC_KEY_LEN];	// g^a || g^b
	BOOL						bGaRecieved = FALSE;
	CryptographicLibrary        Cryptographic;

	// For PAVP SDK Crypto Utility:
	CdgStatus Status = CdgStsOk;
	CdgResult VerifRes = CdgInvalid;

	// For Step 1:
	GetHwEccPubKeyInBuff		sGetHwEccPubKeyIn = {0};
	GetHwEccPubKeyOutBuff		sGetHwEccPubKeyOut = {0};

	// For Step 2:
	HwAppExchgCertsInBuff		sExchgCertsIn = {0};
	HwAppExchgCertsOutBuff		sExchgCertsOut = {0};
	//unsigned char				PrivKey3p[ECDSA_PRIVKEY_LEN];
	Cert3p						Certificate3p = {0};

	Certificate3p.SignBy3p.CertificateType   = DW_SwapEndian(PAVP_EPID_PUBCERT3P_TYPE_PAVP);
	Certificate3p.SignBy3p.IssuerId          = DW_SwapEndian(PAVP_EPID_PUBCERT3P_ISSUER_ID);
	Certificate3p.SignBy3p.Id3p              = DW_SwapEndian(APP_ID);

	EcDsaPubKey		PubKey3pTemp = {	0x4b,   0x42,   0x88,   0x31,	0x1c,   0x81,   0xc1,   0x36,   
										0xae,   0x1a,   0x1b,   0xbf,   0x78,   0xae,	0x44,   0x41,   
										0x41,   0x2a,   0x5f,   0xda,	0x8d,   0x15,   0x3f,   0xb3,   
										0x90,   0x75,   0x66,   0x91,   0xe5,   0x79,	0x0f,   0x96,   
										0x21,   0xf6,   0x1c,   0x96,	0x62,   0xfa,   0xd4,   0xc8,   
										0xbd,   0xa8,   0x63,   0x7a,   0x17,   0xcc,	0x7b,   0xa6,   
										0x4c,   0x0a,   0xb1,   0x97,	0xba,   0x11,   0xaa,   0x90,   
										0x89,   0x53,   0x26,   0x99,   0xc0,   0xf2,	0x58,   0x3f	};

	memcpy(Certificate3p.SignBy3p.PubKey3p, PubKey3pTemp, 64);

	EcDsaSig		Sign3pTemp = {	0x62,   0x98,   0xb0,   0x15,   0x47,   0x2a,   0xe3,   0x9f,
									0x8a,   0x94,   0x67,   0x12,   0x98,   0x71,	0xa8,   0xff,   
									0x70,   0x2e,   0xa9,   0x53,	0x32,   0xf2,   0xd7,   0xe6,   
									0xc7,   0x0c,   0xb3,   0xcb,   0xce,   0x19,	0x3a,   0x82,   
									0xb3,   0x60,   0x6f,   0x0a,	0x00,   0x2a,   0xab,   0x5a,   
									0x20,   0x3a,   0xe9,   0x5b,   0x1e,   0xa6,	0xb5,   0x69,   
								  	0x2a,   0x92,   0x26,   0xb1,	0x06,   0x6f,   0x66,   0x82,   
								 	0x0f,   0xfb,   0x5a,   0x81,   0x19,   0x59,	0x73,   0x6e	};

	memcpy(Certificate3p.Sign3p, Sign3pTemp, 64);

	EcDsaPubKey		PubKeyVerify3pTemp = {	0x94,   0x3c,   0x20,   0x1e,	0xd2,   0xeb,   0x60,   0xa1,
											0xba,   0xc6,   0x57,   0xdd,   0xc1,   0x1d,	0xfc,   0x6d,   
											0x3f,   0xdf,   0xe0,   0x18,	0x56,   0xb3,   0xe8,   0x03,   
											0x6c,   0x51,   0x19,   0xa1,   0x9b,   0x11,	0x50,   0xee,   
											0x1f,   0x14,   0xcb,   0xa9,	0x51,   0x32,   0x77,   0x6d,   
											0x5e,   0xd6,   0xa6,   0x23,   0x73,   0x00,	0x33,   0xb0,   
											0xf8,   0x6b,   0x33,   0x52,	0x98,   0x73,   0xb4,   0x3d,   
											0x08,   0x11,   0xef,   0xed,   0x28,   0x18,	0x71,   0x83	};

	memcpy(Certificate3p.SignByIntel.PubKeyVerify3p, PubKeyVerify3pTemp, 64);

	EcDsaSig		SignIntelTemp = {	0xEE, 0x65, 0xAF, 0x41, 0x11, 0xFF, 0xD3, 0x45, 
										0x6A, 0x55, 0x18, 0x36, 0x06, 0x78, 0xB2, 0x69,
										0x33, 0xD3, 0x94, 0x54, 0xA2, 0xFE, 0xF3, 0x05, 
										0x4D, 0xE9, 0xA8, 0xEE, 0x5E, 0x81, 0xC1, 0x5B,
										0xCA, 0x40, 0x8C, 0xBF, 0x60, 0x95, 0x0F, 0xE2, 
										0xD5, 0xEB, 0x59, 0xE6, 0x0B, 0xDC, 0x42, 0xFD,
										0x0F, 0xBD, 0x6B, 0x78, 0xC2, 0xC0, 0x7E, 0x41, 
										0xF3, 0xAB, 0x98, 0xC3, 0x74, 0xD1, 0x2A, 0xFF	};

	memcpy(Certificate3p.SignIntel, SignIntelTemp, 64);

	do
	{
		// This sample is generating a certificate for test purposes. In reality an application will 
		// only generate a certificate 1 time and then use it after it's been signed by Intel. 
		// This also generates the corresponding private key that we need later. 
		/*Status = GenPrivKeyAndCert3p(PrivKey3p, ECDSA_PRIVKEY_LEN, (unsigned char*)(&Certificate3p), sizeof(Certificate3p));
		if(Status != CdgStsOk) 
		{
			DP("EPID Key Exchange: Failed to generate an application certificate");
			hRes = E_FAIL;
			break;
		}*/

		sGetHwEccPubKeyIn.Header.ApiVersion		= PAVP_EPID_API_VERSION;
		sGetHwEccPubKeyIn.Header.BufferLength	= 0;
		sGetHwEccPubKeyIn.Header.CommandId		= CMD_GET_HW_ECC_PUBKEY;
		sGetHwEccPubKeyIn.Header.Status			= PAVP_STATUS_SUCCESS;

		hRes = (*m_pfnEPIDExecCallback)(m_pvDataContext,
			(LPVOID)&sGetHwEccPubKeyIn,
			sizeof(sGetHwEccPubKeyIn),
			(LPVOID)&sGetHwEccPubKeyOut,
			sizeof(sGetHwEccPubKeyOut));

		if( FAILED(hRes) || !PAVP_EPID_SUCCESS(sGetHwEccPubKeyOut.Header.Status) )
		{
			DP("Failed to get the hardware's ECC public key");
			hRes = E_FAIL;
			break;
		}

		bGaRecieved = TRUE;

		// Copy out g^a
		memcpy(HwPublicEccKey, sGetHwEccPubKeyOut.Ga, SIGMA_PUBLIC_KEY_LEN);

		// Copy out the SIGMA session ID
		m_SigmaSessionId = sGetHwEccPubKeyOut.SigmaSessionId;

		// Derive the session key, Sk and message signing key, Mk using the PAVP EPID SDK code:
		// This will also give g^b back to send to the hardware
		Status = DeriveSigmaKeys(	(unsigned char*)HwPublicEccKey, SIGMA_PUBLIC_KEY_LEN,
			(unsigned char*)AppPublicEccKey, SIGMA_PUBLIC_KEY_LEN,
			(unsigned char*)m_SessionKey, SIGMA_SESSION_KEY_LEN,
			(unsigned char*)m_SigningKey, SIGMA_MAC_KEY_LEN);
		if(Status != CdgStsOk)
		{
			DP("Error deriving signing and session keys");
			hRes = E_FAIL;
			break;
		}

		// Create an HMAC using Mk this application's certificate.
		Status = CreateHmac(	(unsigned char*)(&Certificate3p), sizeof(Cert3p),
			(unsigned char*)m_SigningKey, SIGMA_MAC_KEY_LEN,
			(unsigned char*)&(sExchgCertsIn.Certificate3pHmac), SIGMA_MAC_LEN);
		if(Status != CdgStsOk) 
		{
			DP("Error creating HMAC of app certificate");
			hRes = E_FAIL;
			break;
		}

		// Copy over g^a || g^b to the struct to give to the hardware
		memcpy(GaGb, HwPublicEccKey, SIGMA_PUBLIC_KEY_LEN);
		memcpy(GaGb + SIGMA_PUBLIC_KEY_LEN, AppPublicEccKey, SIGMA_PUBLIC_KEY_LEN);

		//Test
		/*unsigned char				RTestEn[ECDSA_PRIVKEY_LEN] = {	0x6e, 0x0c, 0x2c, 0xa2, 0x5c, 0xec, 0x83, 0xd1,
			0xd7, 0xeb, 0xee, 0xdc, 0x79, 0xb0, 0x7e, 0x7a, 
			0x49, 0x0f, 0xf8, 0xe8, 0xff, 0xdc, 0xa0, 0x7c,
			0xf7, 0x36, 0x33, 0xb8, 0x49, 0xdd, 0xab, 0x2e	
		};
		unsigned char				RTestDe[ECDSA_PRIVKEY_LEN] = {0};*/

		unsigned char				PrivKey3pEn[ECDSA_PRIVKEY_LEN] = {	0x8b, 0xbd, 0xd0, 0x45, 0x50, 0x75, 0xd3, 0x1b,
			0x75, 0x3d, 0x82, 0x3d, 0x0f, 0x46, 0xe2, 0xac,
			0x57, 0xdd, 0xa8, 0x22, 0x9d, 0x0e, 0x13, 0x41,
			0xb6, 0xfb, 0xe4, 0xb7, 0x81, 0xb3, 0xed, 0x4d	
		};
		unsigned char				PrivKey3p[ECDSA_PRIVKEY_LEN] = {0};
		DWORD  dwTemp[4] = { 0x2c2bee72, 0x61e14d2b, 0x89987f10, 0xee79fb92 };

		//m_pCryptImg->AES_128E((BYTE*)&dwTemp, (BYTE*)&Root, (BYTE*)&RootEn); //For generate
		//m_pCryptImg->AES_128E((BYTE*)&dwTemp, (BYTE*)&Root+16, (BYTE*)&RootEn+16); //For generate
		//m_pCryptImg->AES_128D((BYTE*)&dwTemp, (BYTE*)&RTestEn, (BYTE*)&RTestDe);
		//m_pCryptImg->AES_128D((BYTE*)&dwTemp, (BYTE*)&RTestEn+16, (BYTE*)&RTestDe+16);

		//m_pCryptImg->AES_128E((BYTE*)&dwTemp, (BYTE*)&PrivKey3p, (BYTE*)&PrivKey3pEn); //For generate
		//m_pCryptImg->AES_128E((BYTE*)&dwTemp, (BYTE*)&PrivKey3p+16, (BYTE*)&PrivKey3pEn+16); //For generate
		Cryptographic.AES_128D((BYTE*)&dwTemp, (BYTE*)&PrivKey3pEn, (BYTE*)&PrivKey3p);
		Cryptographic.AES_128D((BYTE*)&dwTemp, (BYTE*)&PrivKey3pEn+16, (BYTE*)&PrivKey3p+16);
		//Test

		// Sign the concatenation of g^a || g^b
		Status = MessageSign3p(PrivKey3p, ECDSA_PRIVKEY_LEN,
			(unsigned char*)GaGb, (2 * SIGMA_PUBLIC_KEY_LEN),
			(unsigned char*)&(sExchgCertsIn.EcDsaSigGaGb), ECDSA_SIGNATURE_LEN);
		memset(PrivKey3p, 0, 32);
		if(Status != CdgStsOk) 
		{
			DP("Failed to sign public keys");
			hRes = E_FAIL;
			break;
		}

		sExchgCertsIn.Header.ApiVersion		= PAVP_EPID_API_VERSION;
		sExchgCertsIn.Header.BufferLength	= sizeof(HwAppExchgCertsInBuff) - sizeof(PAVPCmdHdr);
		sExchgCertsIn.Header.CommandId		= CMD_EXCHG_HW_APP_CERT;
		sExchgCertsIn.Header.Status			= PAVP_STATUS_SUCCESS;
		sExchgCertsIn.SigmaSessionId		= m_SigmaSessionId;

		memcpy(sExchgCertsIn.Gb, AppPublicEccKey, SIGMA_PUBLIC_KEY_LEN);
		memcpy(&sExchgCertsIn.Certificate3p, &Certificate3p, sizeof(Cert3p));

		hRes = (*m_pfnEPIDExecCallback)(m_pvDataContext,
			(LPVOID)&sExchgCertsIn, sizeof(sExchgCertsIn), 
			(LPVOID)&sExchgCertsOut, sizeof(sExchgCertsOut) );

		if( FAILED(hRes) || !PAVP_EPID_SUCCESS(sExchgCertsOut.Header.Status) )
		{
			DP("Failed to exchange hardware/app certificates");
			hRes = E_FAIL;
			break;
		}

		// Verify the HMAC at sExchgCertsOut.CertificatePchHmac
		Status = VerifyHmac((unsigned char*)&(sExchgCertsOut.CertificatePch), sizeof(EpidCert),
			(unsigned char*)sExchgCertsOut.CertificatePchHmac, SIGMA_MAC_LEN,
			(unsigned char*)m_SigningKey, SIGMA_MAC_KEY_LEN,
			&VerifRes);
		if(Status != CdgStsOk) 
		{
			DP("Failed to verify PCH certificate");
			hRes = E_FAIL;
			break;
		}
		if(VerifRes != CdgValid) 
		{
			DP("HW certificate is invalid!");
			hRes = E_FAIL;
			break;
		}

		// Verify Intel root signature of certificate PCH (sExchgCertsOut.CertificatePchHmac)
		VerifRes = CdgInvalid;
		Status = VerifyCertPchSigIntel((unsigned char*)&(sExchgCertsOut.CertificatePch), EPID_CERT_LEN, &VerifRes);
		if(Status != CdgStsOk) 
		{
			DP("Failed to verify Intel signature over PCH's certificate");
			hRes = E_FAIL;
			break;
		}
		if(VerifRes != CdgValid)
		{
			DP("HW certificate is invalid!");
			hRes = E_FAIL;
			break;
		}

		// Use EPID to verify g^a || g^b using the hardware's EPID public key from the certificate.
		VerifRes = CdgInvalid;
		Status = MessageVerifyPch(	(unsigned char*)&(sExchgCertsOut.CertificatePch), (sizeof(EpidCert) - ECDSA_SIGNATURE_LEN),
			(unsigned char*)GaGb, (2 * SIGMA_PUBLIC_KEY_LEN),
			NULL,
			0,
			(unsigned char*)sExchgCertsOut.EpidSigGaGb, EPID_SIG_LEN,
			&VerifRes);
		if(Status != CdgStsOk) 
		{
			DP("Failed to verify PCH signature of g^a|g^b");
			hRes = E_FAIL;
			break;
		}
		if(VerifRes == CdgInvalid) 
		{
			DP("PCH signature of g^a|g^b is invalid!");
			hRes = E_FAIL;
			break;
		}	
		
		m_bSessionIsCreated = TRUE;
		SetEvent(m_hSessionIsCreated);		

	} while(FALSE);

	return hRes;
}

VOID CEPIDKeyExchange::DP(char* szMsg, ...)
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
		strcpy_s(szFullMessage, MAX_PATH, "[EPID] ");
		strcat_s(szFullMessage, MAX_PATH, szFormatMessage);		
		OutputDebugStringA(szFullMessage);
	}
}

HRESULT CEPIDKeyExchange::_CloseSession()
{
	HRESULT						hr = S_OK;
	CloseSigmaSessionInBuff		sCloseSessionIn = {0};
	CloseSigmaSessionOutBuff	sCloseSessionOut = {0};

	sCloseSessionIn.Header.ApiVersion	= PAVP_EPID_API_VERSION;
	sCloseSessionIn.Header.BufferLength	= sizeof(CloseSigmaSessionInBuff) - sizeof(PAVPCmdHdr);
	sCloseSessionIn.Header.CommandId	= CMD_CLOSE_SIGMA_SESSION;
	sCloseSessionIn.Header.Status		= PAVP_STATUS_SUCCESS;
	sCloseSessionIn.SigmaSessionId		= m_SigmaSessionId;

	hr = (*m_pfnEPIDExecCallback)(m_pvDataContext,
		(LPVOID)&sCloseSessionIn,
		sizeof(sCloseSessionIn),
		(LPVOID)&sCloseSessionOut,
		sizeof(sCloseSessionOut));

	if( FAILED(hr) || !PAVP_EPID_SUCCESS(sCloseSessionOut.Header.Status) )
	{
		hr = E_FAIL;
	}

	return hr;
}

HRESULT CEPIDKeyExchange::_Close()
{		
	HRESULT hr = S_OK;

	//invalidate audio stream key
	m_bAudio = TRUE;
	hr = _InvalidateStreamKey();
 	if(FAILED(hr))
 	{
		DP("Failed to _InvalidateStreamKey() m_bAudio");
 	}

	//invalidate video stream key
	m_bAudio = FALSE;
	hr = _InvalidateStreamKey();
 	if(FAILED(hr))
 	{
		DP("Failed to _InvalidateStreamKey() m_bVideo");
 	}

	//close session
	hr = _CloseSession();

	//release resource
	SAFE_RELEASE(m_pExtObj);

	CloseHandle(m_hSessionIsCreated);		
	CloseHandle(m_hGotVideoStreamKey);
	CloseHandle(m_hGotAudioStreamKey);

	return hr;
}

HRESULT CEPIDKeyExchange::GetCurrStreamKey(StreamKey *pStreamKey, BOOL bAudio)
{	
	if(bAudio)
	{
		WaitForSingleObject(m_hGotAudioStreamKey, INFINITE);

		memcpy(pStreamKey, m_AudioStreamKey, 16);

		DP("get new audio stream key");		
	}
	else
	{	
		WaitForSingleObject(m_hGotVideoStreamKey, INFINITE);

		CUTILautolock lock(&m_csEPID);

		memcpy(pStreamKey, m_VideoStreamKey, 16);

		DP("get new video stream key");		

		m_bChangeVideoStreamKey = FALSE;
	}

	return S_OK;

}

HRESULT CEPIDKeyExchange::InvalidateStreamKey(PVOID pvDataContext, BOOL bAudio)
{
	HRESULT hr = S_OK;

	if((bAudio)&&(!m_bGetAudioStreamKey))
	{
		DP("no audio stream key need to be invalidated");
		return S_FALSE;
	}
	else if((!bAudio)&&(!m_bGetVideoStreamKey))
	{
		DP("no video stream key need to be invalidated");
		return S_FALSE;
	}

	CUTILautolock lock(&m_csEPID);
	m_bAudio = bAudio;
	if(pvDataContext!=NULL)
		m_pvDataContext = pvDataContext;

	if(m_bUseProxyThread)
	{
		if (m_threadstatus != PROXY_THREAD_CLOSE)
		{	
			m_threadstatus = PROXY_THREAD_INVALIDATE_KEY;
			SetEvent(m_hProcessEvent);
			WaitForSingleObject(m_hWaitEvent, INFINITE);

			if(m_threadstatus == PROXY_THREAD_ERROR)
				hr = E_FAIL;
		}
	}
	else
	{
		hr = _InvalidateStreamKey();
	}

	return hr;
}

HRESULT CEPIDKeyExchange::UpdateStreamKey(StreamKey *pStreamKey, PVOID pvDataContext, BOOL bAudio)
{
	HRESULT hr = S_OK;

	WaitForSingleObject(m_hSessionIsCreated, WAIT_TIME_FOR_CREATE_SESSION);

	CUTILautolock lock(&m_csEPID);
	m_bAudio = bAudio;
	if(pvDataContext!=NULL)
		m_pvDataContext = pvDataContext;

	if(m_bUseProxyThread)
	{
		if (m_threadstatus != PROXY_THREAD_CLOSE)
		{	
			m_threadstatus = PROXY_THREAD_UPDATE_KEY;
			SetEvent(m_hProcessEvent);
			WaitForSingleObject(m_hWaitEvent, INFINITE);

			if(m_threadstatus == PROXY_THREAD_ERROR)
			{
				hr = E_FAIL;
			}
		}
	}
	else
	{
		hr = _UpdateStreamKey();
	}

	if(SUCCEEDED(hr))
	{
		DP("succeeded to update stream key");
		if(bAudio)
		{			
			memcpy(pStreamKey, m_AudioStreamKey, 16);
		}
		else
		{			
			memcpy(pStreamKey, m_VideoStreamKey, 16);
			m_bChangeVideoStreamKey = TRUE;
		}
	}	
	else
	{
		DP("failed to update stream key");
	}

	return hr;
}

// Obtains a new stream key and invalidates the previous one if necessary
HRESULT CEPIDKeyExchange::_UpdateStreamKey()
{
	HRESULT hr = S_OK;

	CdgStatus Status = CdgStsOk;
	CdgResult VerifRes = CdgInvalid;

	// To get Stream keys:
	GetStreamKeyInBuff			sGetStreamKeyIn;
	GetStreamKeyOutBuff			sGetStreamKeyOut;

	DP("_UpdateStreamKey(): Begin");

	do
	{		
		if(!m_bAudio)
		{
			if(m_bGetVideoStreamKey)
				hr = _InvalidateStreamKey();
		}
		else
		{
			if(m_bGetAudioStreamKey)
				hr = _InvalidateStreamKey();
		}

		sGetStreamKeyIn.Header.ApiVersion		= PAVP_EPID_API_VERSION;
		sGetStreamKeyIn.Header.BufferLength		= sizeof(GetStreamKeyInBuff) - sizeof(PAVPCmdHdr);
		sGetStreamKeyIn.Header.CommandId		= CMD_GET_STREAM_KEY;
		sGetStreamKeyIn.Header.Status			= PAVP_STATUS_SUCCESS;
		sGetStreamKeyIn.SigmaSessionId			= m_SigmaSessionId;
		sGetStreamKeyIn.StreamId				= !m_bAudio ? 0 : m_AudioStreamId;		
		sGetStreamKeyIn.MediaPathId				= !m_bAudio ? PAVP_VIDEO_PATH : PAVP_AUDIO_PATH;

		DP("StreamId = %d, MediaPathId = %d", sGetStreamKeyIn.StreamId, sGetStreamKeyIn.MediaPathId);

		hr = (*m_pfnEPIDExecCallback)(m_pvDataContext,
			(LPVOID)&sGetStreamKeyIn,
			sizeof(sGetStreamKeyIn),
			(LPVOID)&sGetStreamKeyOut,
			sizeof(sGetStreamKeyOut));

		if( FAILED(hr) || !PAVP_EPID_SUCCESS(sGetStreamKeyOut.Header.Status) )
		{
			DP("Failed to obtain a stream key from the hardware");
			DP("hr = 0x%x, Status = 0x%x", hr, sGetStreamKeyOut.Header.Status);
			hr = E_FAIL;
			break;
		}

		// Verify the HMAC of the stream key at sGetStreamKeyOut.WrappedStreamKeyHmac using Mk
		VerifRes = CdgInvalid;
		Status = VerifyHmac((unsigned char*)(&sGetStreamKeyOut.WrappedStreamKey), PAVP_EPID_STREAM_KEY_LEN,
			(unsigned char*)sGetStreamKeyOut.WrappedStreamKeyHmac, SIGMA_MAC_LEN,
			(unsigned char*)m_SigningKey, SIGMA_MAC_KEY_LEN,
			&VerifRes);
		if(Status != CdgStsOk) 
		{
			DP("Failed to verify HMAC of encrypted stream key");
			hr = E_FAIL;
			break;
		}
		if(VerifRes != CdgValid) 
		{
			DP("Invalid MAC given for the encrypted stream key");
			hr = E_FAIL;
			break;
		}

		// Decrypt Stream Key using AES-ECB with m_SessionKey as the key
		Status = Aes128EcbDecrypt((unsigned char*)sGetStreamKeyOut.WrappedStreamKey, PAVP_EPID_STREAM_KEY_LEN,
			(unsigned char*)m_StreamKey, PAVP_EPID_STREAM_KEY_LEN,
			(unsigned char*)m_SessionKey, SIGMA_SESSION_KEY_LEN);

		DP("WrappedStreamKey = {%x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x}",
			sGetStreamKeyOut.WrappedStreamKey[0], sGetStreamKeyOut.WrappedStreamKey[1], sGetStreamKeyOut.WrappedStreamKey[2], sGetStreamKeyOut.WrappedStreamKey[3],
			sGetStreamKeyOut.WrappedStreamKey[4], sGetStreamKeyOut.WrappedStreamKey[5], sGetStreamKeyOut.WrappedStreamKey[6], sGetStreamKeyOut.WrappedStreamKey[7],
			sGetStreamKeyOut.WrappedStreamKey[8], sGetStreamKeyOut.WrappedStreamKey[9], sGetStreamKeyOut.WrappedStreamKey[10], sGetStreamKeyOut.WrappedStreamKey[11],
			sGetStreamKeyOut.WrappedStreamKey[12], sGetStreamKeyOut.WrappedStreamKey[13], sGetStreamKeyOut.WrappedStreamKey[14], sGetStreamKeyOut.WrappedStreamKey[15]);

		DP("m_StreamKey = {%x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x}",
			m_StreamKey[0], m_StreamKey[1], m_StreamKey[2], m_StreamKey[3],
			m_StreamKey[4], m_StreamKey[5], m_StreamKey[6], m_StreamKey[7],
			m_StreamKey[8], m_StreamKey[9], m_StreamKey[10], m_StreamKey[11],
			m_StreamKey[12], m_StreamKey[13], m_StreamKey[14], m_StreamKey[15]);

		DP("m_SessionKey = {%x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x}",
			m_SessionKey[0], m_SessionKey[1], m_SessionKey[2], m_SessionKey[3],
			m_SessionKey[4], m_SessionKey[5], m_SessionKey[6], m_SessionKey[7],
			m_SessionKey[8], m_SessionKey[9], m_SessionKey[10], m_SessionKey[11],
			m_SessionKey[12], m_SessionKey[13], m_SessionKey[14], m_SessionKey[15]);

		if(Status != CdgStsOk)
		{
			DP("failed to verify HMAC of encrypted stream key");
			hr = E_FAIL;
			break;
		}

		if(!m_bAudio)
		{			
			m_bGetVideoStreamKey = TRUE;
			memcpy(m_VideoStreamKey, m_StreamKey, 16);
			SetEvent(m_hGotVideoStreamKey);			
		}
		else
		{			
			m_bGetAudioStreamKey = TRUE;
			memcpy(m_AudioStreamKey, m_StreamKey, 16);
			SetEvent(m_hGotAudioStreamKey);			
		}		

	} while( FALSE );

	DP("_UpdateStreamKey(): End");

	return hr;
}

HRESULT CEPIDKeyExchange::_InvalidateStreamKey()
{
	HRESULT hr = S_OK;

	InvStreamKeyInBuff	sInvKeyIn;
	InvStreamKeyOutBuff	sInvkeyOut;

	DP("_InvalidateStreamKey() Begin");

	do
	{	
		if(!m_bAudio)
		{
			if(!m_bGetVideoStreamKey)
			{
				DP("no video stream key need to be invalidated");
				return S_FALSE;
			}
		}
		else
		{
			if(!m_bGetAudioStreamKey)
			{
				DP("no audio stream key need to be invalidated");
				return S_FALSE;
			}
		}

		sInvKeyIn.Header.ApiVersion		= PAVP_EPID_API_VERSION;
		sInvKeyIn.Header.BufferLength	= sizeof(InvStreamKeyInBuff) - sizeof(PAVPCmdHdr);
		sInvKeyIn.Header.CommandId		= CMD_INV_STREAM_KEY;
		sInvKeyIn.Header.Status			= PAVP_STATUS_SUCCESS;
		sInvKeyIn.SigmaSessionId		= m_SigmaSessionId;
		sInvKeyIn.StreamId				= !m_bAudio ? 0 : m_AudioStreamId;		
		sInvKeyIn.MediaPathId			= !m_bAudio ? PAVP_VIDEO_PATH : PAVP_AUDIO_PATH;

		DP("StreamId = %d, MediaPathId = %d", sInvKeyIn.StreamId, sInvKeyIn.MediaPathId);

		hr = (*m_pfnEPIDExecCallback)(m_pvDataContext,
			(LPVOID)&sInvKeyIn,
			sizeof(sInvKeyIn),
			(LPVOID)&sInvkeyOut,
			sizeof(sInvkeyOut));

		if( FAILED(hr) || !PAVP_EPID_SUCCESS(sInvkeyOut.Header.Status) )
		{
			if(!m_bAudio)
				DP("failed to invalidate video stream key");
			else
				DP("failed to invalidate audio stream key");
		}
		else
		{		
			if(!m_bAudio)
			{
				m_bGetVideoStreamKey = FALSE;				
				ResetEvent(m_hGotVideoStreamKey);

				DP("succeed to invalidate video stream key");
			}
			else
			{
				m_bGetAudioStreamKey = FALSE;				
				ResetEvent(m_hGotAudioStreamKey);

				DP("succeed to invalidate audio stream key");
			}
		}

	} while( FALSE );

	DP("_InvalidateStreamKey() End");

	return hr;
}

IUnknown* CEPIDKeyExchange::GetExtObj()
{
	if(m_pExtObj!=NULL)
		m_pExtObj->AddRef();

	return m_pExtObj;
}