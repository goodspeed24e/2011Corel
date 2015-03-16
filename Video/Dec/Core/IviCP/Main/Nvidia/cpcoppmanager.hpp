#ifndef		_CPCoppManager_
#define		_CPCoppManager_

#include <atlbase.h>
#include "COPP.h"

class CPCoppManager
{
public:
	CPCoppManager() : m_pVMR (NULL), m_pCoppCore(NULL)
	{
		InitCoppManager();
	}

	~CPCoppManager()
	{
		CloseCoppManager();
	}

	void QueryIdentifierAndSessionKey(DWORD* pdwIdentifier, BYTE* pbSessionKey)
	{
		if(pdwIdentifier)
			*pdwIdentifier = m_dwIdentifier;

		if(pbSessionKey)
			memcpy(pbSessionKey, m_ucSessionKey, sizeof(BYTE)*16);
	}

private:
	CComPtr<IBaseFilter>	m_pVMR;
	COPPCore*				m_pCoppCore;
	DWORD					m_dwIdentifier;
	BYTE					m_ucSessionKey[16];


private:
	void InitCoppManager()
	{
		m_pVMR = CreateVMR();
		m_pCoppCore = new COPPCore(m_pVMR, NULL);
		if(m_pCoppCore)
			m_pCoppCore->GetStatusSeqIdxAndSessionKey(&m_dwIdentifier, &m_ucSessionKey[0]);
	}
	void CloseCoppManager()
	{
		m_pVMR = NULL;

		delete m_pCoppCore;
		m_pCoppCore = NULL;
	}
	static CComPtr<IBaseFilter> CreateVMR()
	{
		HRESULT hr;
		CComPtr<IBaseFilter> pVMR;
		hr = CoCreateInstance(CLSID_VideoMixingRenderer, NULL, CLSCTX_INPROC, IID_IBaseFilter, (void**)&pVMR);
 		if ( pVMR )
 		{
 			CComPtr<IVMRFilterConfig> pConfig;
 			pVMR->QueryInterface(IID_IVMRFilterConfig, (LPVOID *)&pConfig);
 			if ( pConfig )
 				hr = pConfig->SetRenderingMode(VMRMode_Windowless);
 		}
		return pVMR;
	}
};

#endif
