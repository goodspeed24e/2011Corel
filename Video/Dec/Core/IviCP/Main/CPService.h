
#ifndef _CONTENT_PROTECTION_SERVICE_
#define _CONTENT_PROTECTION_SERVICE_

#include "CPInterfaces.h"

// forward declarations
interface IAMVideoAccelerator;
interface IDirectXVideoDecoder;

class CoCPService : public ICPService
{
public:
	CoCPService();
	virtual ~CoCPService();

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

protected:

	HRESULT DXVA_Execute(DWORD dwFunction, LPVOID lpPrivateInputData, DWORD cbPrivateInputData, LPVOID lpPrivateOutputDat, DWORD cbPrivateOutputData);	
	VOID	DP(CHAR* szMsg, ...);

	BOOL m_bEncryptOn;	
	BOOL m_bEncryptIFrameOnly;
	BOOL m_bShowMsg;
	IAMVideoAccelerator *m_pVideoAccl;
	IDirectXVideoDecoder *m_pDirectXVideoDec;	
	LONG m_lRefCount;	
};



#endif