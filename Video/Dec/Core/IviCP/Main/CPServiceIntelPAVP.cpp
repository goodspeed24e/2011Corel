
#include "CPServiceIntelPAVP.h"
#include "PAVPDevice.h"
#include <InitGuid.h>

DEFINE_GUID(DXVA2_Intel_Pavp, 0x7460004, 0x7533, 0x4e1a, 0xbd, 0xe3, 0xff, 0x20, 0x6b, 0xf5, 0xce, 0x47);

CoCPServiceIntelPAVP::CoCPServiceIntelPAVP() : m_pPavpDevice(NULL), m_bAudio(FALSE), m_bHasGotAudioStreamKey(FALSE)
{	
	
}

CoCPServiceIntelPAVP::~CoCPServiceIntelPAVP()
{	
	Close();
}

STDMETHODIMP CoCPServiceIntelPAVP::QueryInterface(REFIID riid, void**ppInterface)
{
	if (IID_IUnknown == riid)
	{
		*ppInterface = static_cast<IUnknown *>(static_cast<ICPService *>(this));
	}
	else if (IID_ICPService == riid)
	{
		*ppInterface = static_cast<ICPService *>(this);
	}	
	else if (IID_ICheckPavpDevice == riid)
	{
		*ppInterface = static_cast<ICheckPavpDevice *>(this);
	}	
	else
	{
		*ppInterface = NULL;
		return E_NOINTERFACE;
	}

	static_cast<IUnknown *>(*ppInterface)->AddRef();

	return S_OK;
}

STDMETHODIMP_(ULONG) CoCPServiceIntelPAVP::AddRef()
{
	return InterlockedIncrement(&m_lRefCount);
}

STDMETHODIMP_(ULONG) CoCPServiceIntelPAVP::Release()
{
	LONG lRefCount = InterlockedDecrement(&m_lRefCount);

	if (0 == lRefCount)	
		delete this;

	return lRefCount;
}

STDMETHODIMP CoCPServiceIntelPAVP::Open(CPOpenOptions *pOpenOptions)
{
	HRESULT hr = E_FAIL;			

	m_bShowMsg = ((pOpenOptions->dwCPOption&E_CP_OPTION_DBG_MSG)!=0);
	m_bAudio = ((pOpenOptions->dwCPOption&E_CP_OPTION_AUDIO)!=0);

	DP("Encrypt Mode = INTEL_PAVP");
	
	DP("CoCPServiceIntelPAVP Open() Begin");

	if(!m_bAudio) // for video
	{	
		//Intel PAVP doesn't support scrambling for MoComp profile
		if((*(pOpenOptions->pDecodeProfile) == DXVA2_ModeMPEG2_MoComp) || (*(pOpenOptions->pDecodeProfile) == DXVA2_ModeH264_A) ||
		   (*(pOpenOptions->pDecodeProfile) == DXVA2_ModeH264_B) || (*(pOpenOptions->pDecodeProfile) == DXVA2_ModeVC1_A) || 
		   (*(pOpenOptions->pDecodeProfile) == DXVA2_ModeVC1_B))
			return E_FAIL;
	}

	hr = CoCPService::Open(pOpenOptions);
	if(FAILED(hr))
		return hr;									

	m_pPavpDevice = CPavpDevice::GetInstance((IDirect3DDevice9 *)pOpenOptions->pD3D9);
	
	CUTILautolock lock(&CPavpDevice::m_PavpDeviceCS);

	if (m_bAudio)
		m_pPavpDevice->SetAudioStreamId(pOpenOptions->dwAudioStreamId);

	if(!m_pPavpDevice->IsSessionEstablished())
	{		
		hr = m_pPavpDevice->Open(m_bAudio, m_bShowMsg);
		if(FAILED(hr))
		{
			DP("failed to open PAVP Device");
			return hr;		
		}
		else
		{
			DP("succeeded to open PAVP Device");				
		}
	}
	else
	{
		DP("PAVP Device is opened already!");				
	}

	DP("CoCPServiceIntelPAVP Open() End");

	return S_OK;
}

STDMETHODIMP CoCPServiceIntelPAVP::Close()
{		
	DP("CoCPServiceIntelPAVP Close() Begin");

	if(m_pPavpDevice!=NULL)
	{
		m_pPavpDevice->Close(m_bAudio);
	}

	SAFE_RELEASE(m_pPavpDevice);

	CoCPService::Close();

	DP("CoCPServiceIntelPAVP Close() End");
	
	return S_OK;
}

STDMETHODIMP_(BOOL) CoCPServiceIntelPAVP::IsScramblingRequired(ECPFrameType eImgType)
{	
	return CoCPService::IsScramblingRequired(eImgType);
}

STDMETHODIMP CoCPServiceIntelPAVP::EnableScrambling()
{	
	return CoCPService::EnableScrambling();
}

STDMETHODIMP CoCPServiceIntelPAVP::DisableScrambling()
{
	HRESULT hr = S_OK;	

	if(m_bEncryptOn)
	{
		m_bEncryptOn = FALSE;	
		hr = _DisableScrambling();	
		if(FAILED(hr))
			DP("failed to disable scrambling!!!");
	}

	return hr;
}

STDMETHODIMP CoCPServiceIntelPAVP::ScrambleData(BYTE *pDataOut, const BYTE *pDataIn, DWORD dwDataLen)
{	
	if(!m_bAudio) // for Video
	{		
		if(m_bEncryptOn)
		{			
			m_pPavpDevice->ScrambleData((PVOID*)m_pDirectXVideoDec, pDataIn, pDataOut, dwDataLen);
		}
		else
		{
			memcpy(pDataOut, pDataIn, dwDataLen);
		}				
	}
	else // for Audio
	{			
		if(!m_bHasGotAudioStreamKey)
		{
			m_pPavpDevice->UpdateStreamKey(m_bAudio);

			m_bHasGotAudioStreamKey = TRUE;
		}

		m_pPavpDevice->ScrambleDataAudio(pDataIn, pDataOut, dwDataLen);
	}
		
	return S_OK;
}

STDMETHODIMP_(BYTE)	CoCPServiceIntelPAVP::GetEncryptMode()
{	
	if(m_pPavpDevice->IsPAVP10())
		return 1; //PAVP 1.0
	else if (m_pPavpDevice->IsPAVP15())
		return 2; //PAVP 1.5
	else
		return 0;
}

STDMETHODIMP CoCPServiceIntelPAVP::ProtectedBlt(DWORD dwWidth, DWORD dwHeight, DWORD dwPitch, BYTE *pSrc)
{		
	

	return S_OK;
}

STDMETHODIMP_(BOOL) CoCPServiceIntelPAVP::IsPAVPDevice(BOOL bDShow, IUnknown *pUnknown, BOOL bIsAudio)
{
	CUTILautolock lock(&CPavpDevice::m_PavpDeviceCS);

	BOOL bPavpDevice = FALSE;
	CPavpDevice *pPavpDevice = NULL;
	PAVP_CREATE_DEVICE pPavpCreateDevice = {PAVP_KEY_EXCHANGE_DEFAULT};
	
	if(bDShow)
		pPavpDevice = CPavpDevice::GetInstance((IMFGetService *)pUnknown);	
	else
		pPavpDevice = CPavpDevice::GetInstance((IDirect3DDevice9 *)pUnknown);
		
	
	if(pPavpDevice == NULL)
	{
		DP("Can not Create Pavp Device");
		return bPavpDevice;
	}	
	
	if(pPavpDevice->IsPresent())
	{
		if(bIsAudio)
		{			
			bPavpDevice = pPavpDevice->IsPAVP15();			
		}
		else
			bPavpDevice = TRUE;
	}
	else
		bPavpDevice = FALSE;

	SAFE_RELEASE(pPavpDevice);

	return bPavpDevice;
}

STDMETHODIMP_(ECPObjID)	CoCPServiceIntelPAVP::GetObjID()
{
	return E_CP_ID_INTEL_PAVP;
}

STDMETHODIMP CoCPServiceIntelPAVP::_DisableScrambling()
{
	CUTILautolock lock(&CPavpDevice::m_PavpDeviceCS);

	DXVA_EncryptProtocolHeader EncryptHdrIn;
	DXVA_EncryptProtocolHeader EncryptHdrOut;

	memset(&EncryptHdrIn, 0, sizeof(EncryptHdrIn));
	memset(&EncryptHdrOut, 0, sizeof(EncryptHdrOut));
	EncryptHdrIn.dwFunction = 0xffff0001;
	EncryptHdrIn.guidEncryptProtocol = DXVA2_Intel_Pavp;

	return DXVA_Execute(NULL, &EncryptHdrIn, sizeof(EncryptHdrIn), &EncryptHdrOut, sizeof(EncryptHdrOut));
}