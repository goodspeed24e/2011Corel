#ifndef _CONTENT_PROTECTION_SERVIC_ATI_
#define _CONTENT_PROTECTION_SERVIC_ATI_

#include "CPService.h"

#define KEY_SIZE 16
#define MAX_NUM_CONTENT_KEYS 16
#define MAX_CONTENT_KEY_SIZE 32 

#pragma warning(disable:4819)

// Foward Declarations
class CATIHSP_Authentication;
class CATIHSP_Encryption;

enum EIviSecureProtocolCipherType
{
	IVI_ALG_NONE    = 0,
	IVI_ALG_LEGACY  = 1,
	IVI_ALG_XOR     = 2,
	IVI_ALG_AESLITE = 4,
	IVI_ALG_AESCTR  = 8
};

struct AtiSession
{
	CATIHSP_Authentication* pAuthObj;
	CATIHSP_Encryption* pEncrypt;
	BYTE KeySession[KEY_SIZE];
	BYTE ContentKeys[MAX_NUM_CONTENT_KEYS][MAX_CONTENT_KEY_SIZE];
	BYTE ContentKeyCounter[MAX_NUM_CONTENT_KEYS][KEY_SIZE];
	int iIndexOfCurrentKey;
	int iContentKeySize;
	int iUsedCount[MAX_NUM_CONTENT_KEYS];
};

class CoCPServiceAti : public CoCPService
{
public:
	CoCPServiceAti();
	~CoCPServiceAti();

	//IUnknown
	STDMETHODIMP			QueryInterface(REFIID riid, void**ppInterface);
	STDMETHODIMP_(ULONG)	AddRef();
	STDMETHODIMP_(ULONG)	Release();

	//ICPService
	STDMETHODIMP			Open(CPOpenOptions *pOpenOptions);
	STDMETHODIMP			Close();
	STDMETHODIMP_(BOOL)		IsScramblingRequired(ECPFrameType eImgType=CP_I_TYPE);	
	STDMETHODIMP            EnableScrambling();
	STDMETHODIMP            DisableScrambling();
	STDMETHODIMP			ScrambleData(BYTE *pDataOut, const BYTE *pDataIn, DWORD dwDataLen);	
	STDMETHODIMP_(BYTE)		GetEncryptMode();	
	STDMETHODIMP			ProtectedBlt(DWORD dwWidth, DWORD dwHeight, DWORD dwPitch, BYTE *pSrc);	
	STDMETHODIMP_(ECPObjID)	GetObjID();
	
private:
	STDMETHODIMP CreateNewSession(AtiSession* pSession, IUnknown *pVA, GUID *pguidDec);
	STDMETHODIMP CreateNewContentKey(AtiSession* pSession);
	STDMETHODIMP TriggerContentKeySwitching(AtiSession* pSession);
	STDMETHODIMP_(VOID) XOR128bits(const BYTE *pIn1, const BYTE *pIn2, BYTE *pOut);
	STDMETHODIMP GetDeviceCaps(AtiSession* pSession, GUID *pguidDec);
	STDMETHODIMP ShareAuthProtocol(AtiSession* pSession);
	STDMETHODIMP SetEncryptionFormat(AtiSession* pSession);

	EIviSecureProtocolCipherType m_dwCipherType;
	AtiSession m_Session;	
	BOOL m_bProtectedBlt;
	DWORD m_dwFrameWidth;	
	DWORD m_dwNonUsedKeys;
	
};

#endif