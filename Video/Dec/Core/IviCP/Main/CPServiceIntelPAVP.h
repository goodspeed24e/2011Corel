
#ifndef _CONTENT_PROTECTION_SERVIC_INTEL_PAVP_
#define _CONTENT_PROTECTION_SERVIC_INTEL_PAVP_

#include "CPService.h"

class CPavpDevice;

class CoCPServiceIntelPAVP : public CoCPService, public ICheckPavpDevice
{
public:
	CoCPServiceIntelPAVP();
	~CoCPServiceIntelPAVP();

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

	//ICheckPavpDevice					
	STDMETHODIMP_(BOOL)		IsPAVPDevice(BOOL bDShow, IUnknown *pUnknown, BOOL bIsAudio=FALSE);		
	
private:

	STDMETHODIMP _DisableScrambling();

	CPavpDevice *m_pPavpDevice;	
	
	BOOL m_bAudio;
	BOOL m_bHasGotAudioStreamKey;	
	
};

#endif