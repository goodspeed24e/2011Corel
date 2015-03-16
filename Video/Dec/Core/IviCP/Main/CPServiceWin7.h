
#ifndef _CONTENT_PROTECTION_SERVICE_WIN7_
#define _CONTENT_PROTECTION_SERVICE_WIN7_

#include "CPService.h"
#include "d3d9.h"
#include "dxva.h"
#include "EPIDKeyExchange.h"
#include ".\Imports\GPUCPService\GPUCPInterfaces.h"

#define AES_KEY_LEN 16
#define CIPHER_TEXT_LEN 256
#define SESSION_KEY_REFRESH_LEVEL 300 //about 150sec for encrypting I frame only at 30fps 
#define CONTENT_KEY_REFRESH_LEVEL 100 //about 50sec for encrypting I frame only at 30fps 
#define AUDIO_SESSION_KEY_REFRESH_LEVEL 500

enum EWIN7EncryptMode
{
	E_WIN7_Encrypt_NONE	= 0,
	E_WIN7_Encrypt_PROPRIETARY	= 1,
	E_WIN7_Encrypt_AESCTR	= 2,
	E_WIN7_Encrypt_INTEL_AESCTR = 3
};

enum EWin7CPOption
{
	E_CP_WIN7_OFF = 0,
	E_CP_WIN7_ON = 1,
	E_CP_WIN7_AUTO = 1<<1,
	E_CP_WIN7_DBG_MSG = 1<<2,
	E_CP_WIN7_I_ONLY = 1<<3,	
	E_CP_WIN7_DISABLE_CONTENT_KEY = 1<<4,
	E_CP_WIN7_DISABLE_REFRESHED_SESSION_KEY = 1<<5,
	E_CP_WIN7_DISABLE_PARTIAL_ENCRYPTION = 1<<6,
	E_CP_WIN7_DISABLE_PROTECTED_BLT = 1<<7
};

enum EWin7CPType
{
    E_CPWIN7_PROTECTIONENABLED = 1,
    E_CPWIN7_OVERLAYORFULLSCREENREQUIRED = 1<<1,
};

typedef struct tagPAVP_EPID_EXCHANGE_PARAMS
{
	BYTE *pInput;
	ULONG ulInputSize;
	BYTE *pOutput;
	ULONG ulOutputSize;
} PAVP_EPID_EXCHANGE_PARAMS;

typedef struct _tagGPUCP_CRYPTOSESSION_PAVP_KEYEXCHANGE
{
	GUID  guidPAVPKeyExchange;
	UINT  uiDataSize;
	VOID* pKeyExchangeParams;
}GPUCP_CRYPTOSESSION_PAVP_KEYEXCHANGE;

class CoCPServiceWin7 : public CoCPService, IGetParams
{
public:
	CoCPServiceWin7();
	~CoCPServiceWin7();

	//IUnknown
	STDMETHODIMP			QueryInterface(REFIID riid, void**ppInterface);
	STDMETHODIMP_(ULONG)	AddRef();
	STDMETHODIMP_(ULONG)	Release();

	//ICPService
	STDMETHODIMP			Open(CPOpenOptions *pOpenOptions);
	STDMETHODIMP			Close();
	STDMETHODIMP_(BOOL)		IsScramblingRequired(ECPFrameType eImgType=CP_I_TYPE);	
	STDMETHODIMP			EnableScrambling();
	STDMETHODIMP			DisableScrambling();	
	STDMETHODIMP			ScrambleData(BYTE *pDataOut, const BYTE *pDataIn, DWORD dwDataLen);	
	STDMETHODIMP_(BYTE)		GetEncryptMode();
	STDMETHODIMP			ProtectedBlt(DWORD dwWidth, DWORD dwHeight, DWORD dwPitch, BYTE *pSrc);	
	STDMETHODIMP_(ECPObjID)	GetObjID();

	//IGetParams
	STDMETHODIMP_(LPVOID)   GetIV();
	STDMETHODIMP_(LPVOID)   GetContentKey();

protected:
	STDMETHODIMP Init(IUnknown* pD3D9, GUID *pDecodeProfile);
	STDMETHODIMP NegotiateKeyWithAuthChannel();
	STDMETHODIMP NegotiateKeyWithCryptoSession();
	STDMETHODIMP GenerateKey(BYTE *pKey, DWORD dwKeySize);
	STDMETHODIMP EncryptAESKey(BYTE *pCert, DWORD dwCertSize, BYTE *pAESKey, DWORD dwAESKeySize, BYTE *pEncryptedAESKey, UINT uiEncryptedAESKeySize);
	STDMETHODIMP GetProtectionCapabilities(GUID *pDecodeProfile);

	STDMETHODIMP Configure();	
	STDMETHODIMP RefreshSessionKey();
	STDMETHODIMP UpdateContentKey(BOOL bNewKey=TRUE);
	STDMETHODIMP_(UINT64) SwapByteOrder(UINT64 ui64Val);

	//For Intel
	STDMETHODIMP EncryptAESKeyForIntel(BYTE *pAESKey, DWORD dwAESKeySize, BYTE *pEncryptedAESKey, UINT uiEncryptedAESKeySize);		
	STDMETHODIMP PAVPFixedKeyExchange();	
	STDMETHODIMP PAVPEPIDKeyExchange();	
	static HRESULT EPIDExec(PVOID pvContext, LPVOID pInBuf, DWORD dwInBufLen, LPVOID pOutBuf, DWORD dwOutBufLen);
	//static HRESULT EPIDRelease(PVOID pvContext);		
	
	CEPIDKeyExchange *m_pEPID;
	static CUTILcrit m_csEPID;
	
	BOOL m_bAudio;
	PAVPStreamId m_AudioStreamId;
	BOOL m_bCryptoSessionIsCreated;
	BOOL m_bStreamKeyIsGotten;
		
	IDirect3DDevice9Video *m_pD3D9Video;
	GUID m_guidCryptoType;	
	D3DCONTENTPROTECTIONCAPS m_CPCaps;

	IDirect3DAuthenticatedChannel9 *m_pAuthChannel;
	HANDLE m_hAuthChannel;	
	
	//Enable Screen Capture Defense.
	IDirect3DAuthenticatedChannel9 *m_pD3D9AuthChannel;
	HANDLE m_hD3D9AuthChannel;
	
	D3DAUTHENTICATEDCHANNELTYPE m_eChannelType;
	IDirect3DCryptoSession9 *m_pCryptoSession;
	HANDLE m_hCryptoSession;
	HANDLE m_hDXVA2Dec;
	HANDLE m_hDevice;

	DWORD m_dwBufferAlignmentStart;
	DWORD m_dwBlockAlignmentSize;	

	BYTE m_AESKeyOfAuthChannel[AES_KEY_LEN];
	BYTE m_AESKeyOfCryptoSession[AES_KEY_LEN];	
	BYTE m_ContentKey[AES_KEY_LEN];	
	BYTE m_EncryptedContentKey[AES_KEY_LEN];	

	EWIN7EncryptMode m_eWIN7_EncryptMode;

	UINT m_uiStartSequenceConfigure;
	UINT m_uiStartSequenceQuery;	
	
	D3DAES_CTR_IV m_IV;		
	D3DAES_CTR_IV m_SwappedIV;
	
	DWORD m_dwEncryptedFramesWithSessionKey;
	DWORD m_dwEncryptedFramesWithContentKey;

	BOOL m_bUseContentKey;
	BOOL m_bRefreshSessionKey;
	BOOL m_bPartialEncryption;
	BOOL m_bProtectedBlt;
	DWORD m_dwVenderID;
	BOOL m_bIsOverlay;
    BOOL m_bIsFlipEx;
    BOOL m_bDShowMode;

	BOOL m_bOpened;
	BOOL m_bWindowed;
	D3DSWAPEFFECT m_eSwapEffect;
};

class CoCPServiceWin7PAVP : public CoCPServiceWin7
{

public:
	CoCPServiceWin7PAVP();
	~CoCPServiceWin7PAVP();

	STDMETHODIMP			Open(CPOpenOptions *pOpenOptions);
	STDMETHODIMP			Close();
	STDMETHODIMP			EnableScrambling();
	STDMETHODIMP			DisableScrambling();	
	STDMETHODIMP			ScrambleData(BYTE *pDataOut, const BYTE *pDataIn, DWORD dwDataLen);	
	
	//IGetParams
	STDMETHODIMP_(LPVOID)   GetIV();
	STDMETHODIMP_(LPVOID)   GetContentKey();
	STDMETHODIMP_(BOOL)		IsPAVPSessionAvailable();

protected:

	IGPUCPService *m_pGPUCP;
};


#endif