#ifndef _CONTENT_PROTECTION_SERVIC_NVIDIA_MPEG2_
#define _CONTENT_PROTECTION_SERVIC_NVIDIA_MPEG2_

#include "CPService.h"

class CoCPServiceNvidiaMp2IDCT : public CoCPService, public ICPServiceNvidiaMp2IDCT
{
public:
	CoCPServiceNvidiaMp2IDCT();
	~CoCPServiceNvidiaMp2IDCT();

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

	//ICPServiceNvidiaMp2		
	STDMETHODIMP			UpdatelfsrIB(WORD *wPadBlock);
	STDMETHODIMP_(WORD)		ScrambleDataIDCT(WORD wPadBlock, BYTE byIndex, WORD wData);		

private:	
	STDMETHODIMP			SetDXEncryption(BOOL bOn);
	STDMETHODIMP			PassFrameKey(WORD wFrameKey);
	STDMETHODIMP			SetAuth(DWORD dwH2MKey, DWORD *pdwM2HKey);

	WORD m_wHostSKey;
	DWORD m_dwH2MKey, m_dwM2HKey;
	DWORD m_dwlfsrB, m_dwlfsrI;
	WORD m_wFrameKey;	
};

#endif