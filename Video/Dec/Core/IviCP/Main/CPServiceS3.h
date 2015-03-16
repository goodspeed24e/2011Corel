
#ifndef _CONTENT_PROTECTION_SERVIC_S3_
#define _CONTENT_PROTECTION_SERVIC_S3_

#include "CPService.h"

typedef struct _S3DXVA_Encrypt S3DXVA_Encrypt;
typedef struct _DXVA_EncryptProtocolHeader DXVA_EncryptProtocolHeader;
typedef enum _KEY_SIZE IVI_KEY_SIZE;

class CoCPServiceS3 : public CoCPService
{
public:
	CoCPServiceS3();
	~CoCPServiceS3();

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
	STDMETHODIMP			SetDXEncryption(BOOL bOn);	

	enum {m_cnSessionKey = 256};
	S3DXVA_Encrypt				*m_pS3Encrypt;
	DXVA_EncryptProtocolHeader	*m_pEncryptInput;
	DXVA_EncryptProtocolHeader  *m_pEncryptOutput;
	BOOL						m_bEnhanced;	
	const DWORD					m_cdwDecoderID, m_cdwS3ID, m_cdwEncrypHost, m_cdwEncrypAccel, m_cdwEnhancedApp, m_cdwEnhancedDriver;
	BYTE						m_SessionKey[m_cnSessionKey];
	BYTE						m_CurrContentKey[m_cnSessionKey];
	BYTE						m_EncryptedContentKey[m_cnSessionKey << 1];
	BYTE						m_NewContentKey[m_cnSessionKey];
	IVI_KEY_SIZE				m_eKeySize;	
};

#endif