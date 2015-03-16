#ifndef _CONTENT_PROTECTION_SERVIC_NVIDIA_
#define _CONTENT_PROTECTION_SERVIC_NVIDIA_

#include "CPService.h"
#include <dxva.h>

class NvScrambler;
class AES128;
class CPCoppManager;

enum E_NV_Encrypt_Mode
{
	E_NV_Encrypt_NONE	= 0,
	E_NV_Encrypt_SCRAMBLE	= 1,
	E_NV_Encrypt_AESCTR	= 2,
};

class CoCPServiceNvidia : public CoCPService
{
public:
	CoCPServiceNvidia();
	~CoCPServiceNvidia();

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
	STDMETHODIMP			GenerateKey();
	STDMETHODIMP			UpdateIV();

	NvScrambler *m_pScrambler;
	AES128 *m_pAESScrambler;
	CPCoppManager* m_pCPCOPPMgr;
	E_NV_Encrypt_Mode m_eNV_EncryptMode;
	int m_updateIVCount;		
	DWORD m_dwDeviceID;
	DXVA_EncryptProtocolHeader	m_EncryptHdr;

};

#endif