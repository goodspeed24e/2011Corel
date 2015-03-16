
#ifndef _CONTENT_PROTECTION_SERVIC_INTEL_
#define _CONTENT_PROTECTION_SERVIC_INTEL_

#include "CPService.h"

class CoCPServiceIntel : public CoCPService
{
public:
	CoCPServiceIntel();
	~CoCPServiceIntel();

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
	STDMETHODIMP			ChangeKey();

	GUID m_guidKey;
	
};

#endif