#ifndef _CONTENT_PROTECTION_SERVIC_ATI_MPEG2_
#define _CONTENT_PROTECTION_SERVIC_ATI_MPEG2_

#include "CPService.h"
#include "d3d9.h"
#include "r128auth.h"
#include <dxva.h>

typedef struct
{
	DXVA_EncryptProtocolHeader header;
	DWORD		dwCommand;
	DWORD		dwData;
} ATIEncryptionCommand;

class CoCPServiceAtiMp2IDCT : public CoCPService, public ICPServiceAtiMp2IDCT
{
public:
	CoCPServiceAtiMp2IDCT();
	~CoCPServiceAtiMp2IDCT();

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

	//ICPServiceAtiMp2		
	STDMETHODIMP			SkipMBlock(DWORD dwDeviceID);
	STDMETHODIMP			RescrambleDXVACoeff(DWORD dwDeviceID, LPVOID pSrc, LPVOID pDest);
	STDMETHODIMP			RescrambleDXVABlock(DWORD dwDeviceID, LPVOID pSrc, LPVOID pDest, DWORD dwNumOfBytes);	

private:
	
	STDMETHODIMP			SetDXEncryption(BOOL bOn);
    HRESULT			        EnableGPUCPSCD(IUnknown* pD3D9);
	
	DXVA_EncryptProtocolHeader m_DXVAEncryptProtocolHdr;
	BOOL  m_bMultiChannelEncrypt;
	BOOL  m_dwDeviceID;
	BYTE  m_byChannelNum;
	HANDLE m_hEncrypt;

    //Enable Screen Capture Defense.
    IDirect3DAuthenticatedChannel9 *m_pD3D9AuthChannel;
    HANDLE m_hD3D9AuthChannel;
	
};

#endif