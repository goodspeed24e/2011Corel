
#include "COPP.h"
#include <InitGuid.h>

DEFINE_GUID(IID_IAMCertifiedOutputProtection_Local, 
	0x6feded3e, 0x0ff1, 0x4901, 0xa2, 0xf1, 0x43, 0xf7, 0x01, 0x2c, 0x85, 0x15);

#define SAFE_RELEASE(p) if((p)!=NULL) {(p)->Release(); (p)=NULL;}

COPPCore::COPPCore(IBaseFilter *pFilter, HRESULT *phr) : m_pCOPP(0), m_dwStatusSeqIdx(0),
m_dwCommandSeqIdx(0)
{
	HRESULT hr = S_OK;

	do
	{
		if (0 == pFilter)
		{
			hr = E_INVALIDARG;
			break;
		}

		if (FAILED(hr = pFilter->QueryInterface(IID_IAMCertifiedOutputProtection_Local, 
			(void**)&m_pCOPP)))
			break;

		hr = Initialize();

	} while (FALSE);

	if (0 != phr)
		*phr = hr;

	return;
}   //  COPPCore::COPPCore


COPPCore::~COPPCore()
{
	SAFE_RELEASE(m_pCOPP);
}


HRESULT COPPCore::Initialize()
{
	HRESULT         hr = S_OK;
	GUID            gGHRandom = {0};
	BYTE*           pbGHCert = 0;
	DWORD           dwGHCertLen = 0;
	AMCOPPSignature sig = {0};	
	
	do
	{
		//  key exchange
		if (FAILED(hr = m_pCOPP->KeyExchange(&gGHRandom, &pbGHCert, &dwGHCertLen)))
			break;

		//  initialize
		if (FAILED(hr = m_AuthHelper.Initialize(pbGHCert, dwGHCertLen)))
			break;

		if (FAILED(hr = m_AuthHelper.GenerateRandomNumber((BYTE*)&m_dwStatusSeqIdx, sizeof DWORD)))
			break;

		if (FAILED(hr = m_AuthHelper.GenerateRandomNumber((BYTE*)&m_dwCommandSeqIdx, sizeof DWORD)))
			break;

		//  start session
		if (FAILED(hr = m_AuthHelper.PrepareSignature((BYTE*)&sig, sizeof AMCOPPSignature,
			&gGHRandom, m_dwStatusSeqIdx, m_dwCommandSeqIdx, m_dwSessionKey)))
			break;

		hr = m_pCOPP->SessionSequenceStart(&sig);

	}   while (FALSE);

	if (0 != pbGHCert)
		CoTaskMemFree(pbGHCert);

	return hr;
}   //  COPPCore::Initialize


HRESULT COPPCore::GetStatusSeqIdxAndSessionKey(DWORD* pdwSeqIdx, BYTE* pbSessionKey)
{
	if(pdwSeqIdx==NULL || pbSessionKey==NULL)
		return E_OUTOFMEMORY;

	if(pdwSeqIdx)
		*pdwSeqIdx = m_dwCommandSeqIdx;

	if(pbSessionKey)
		memcpy(pbSessionKey, m_dwSessionKey, sizeof(BYTE)*16);

	return S_OK;
}