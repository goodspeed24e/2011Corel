#pragma once

#include <windows.h>
#include <uuids.h>
#include <strmif.h>
#include <dxva.h>
#include "CertHelper.h"
#include "AuthHelper.h"

class COPPCore
{
public:
	COPPCore(IBaseFilter* pUnk, HRESULT* phr);
	~COPPCore();

	HRESULT Initialize();

	HRESULT GetStatusSeqIdxAndSessionKey(DWORD* pdwSeqIdx, BYTE* pbSessionKey);

public:
	IAMCertifiedOutputProtection*   m_pCOPP;

	COPPAuthHelper                  m_AuthHelper;

private:

	DWORD                           m_dwStatusSeqIdx;
	DWORD                           m_dwCommandSeqIdx;
	BYTE                            m_dwSessionKey[16];	
};

