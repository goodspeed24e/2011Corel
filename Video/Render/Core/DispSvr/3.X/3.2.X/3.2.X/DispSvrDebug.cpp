#include "stdafx.h"
#include "DebugExports\Inc\DispSvrDbgDef.h"
#include "DispSvrDebug.h"
#include "DispSvrDbg_i.c"

//==========================================================================================================
CDispSvrDebug::CDispSvrDebug(LPUNKNOWN pUnk, HRESULT *phr)
	: CUnknown(NAME("Display Server Debug"), pUnk)
	,m_dwDispSvrDbgRegID(NULL)
	,m_InitFlags(NULL)
{
	AddRef();
}

CDispSvrDebug::~CDispSvrDebug()
{

}
//==========================================================================================================
//IUnknown implementation
STDMETHODIMP CDispSvrDebug::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
	HRESULT hr = E_NOINTERFACE;
	*ppv = NULL;

	if (riid == IID_IDispSvrDbg)
	{
		hr = GetInterface((IDispSvrDbg*) this, ppv);
	}
	else
	{
		hr = CUnknown::NonDelegatingQueryInterface(riid, ppv);
	}
	return hr;
}

//IDispSvrDbg implementation
STDMETHODIMP CDispSvrDebug::GetDispSvrVersion(BSTR *bstrVer)
{
	USES_CONVERSION;
    TCHAR achModule[MAX_PATH];
	TCHAR FileVersion[MAX_PATH];
	DWORD dwTemp;
	VS_FIXEDFILEINFO FixedFileInfo;
	BYTE *pBuffer;
	HMODULE hModule = GetModuleHandle(_T("DispSvr.dll"));
	GetModuleFileName( hModule, achModule, sizeof(achModule));
	
	UINT size = GetFileVersionInfoSize(achModule,&dwTemp);
	size;
	pBuffer = new byte[size+1];

	if (GetFileVersionInfo(achModule, 0, size, pBuffer))
	{
		void *pBufHead;
		VerQueryValue( pBuffer, _T("\\"),&pBufHead,&size);
		CopyMemory( &FixedFileInfo, pBufHead, sizeof(VS_FIXEDFILEINFO));
	}
	_stprintf_s(FileVersion, MAX_PATH, _T("%d.%d.%d.%d"),
		HIWORD(FixedFileInfo.dwFileVersionMS),
		LOWORD(FixedFileInfo.dwFileVersionMS),
		HIWORD(FixedFileInfo.dwFileVersionLS),
		LOWORD(FixedFileInfo.dwFileVersionLS)
		);
	*bstrVer = SysAllocString(CT2W(FileVersion));
	SAFE_DELETE_ARRAY(pBuffer);


	return S_OK;
}

STDMETHODIMP CDispSvrDebug::SetInitFlags(DWORD dwFlags)
{
	m_InitFlags = dwFlags;
	return S_OK;
}

STDMETHODIMP CDispSvrDebug::GetInitFlags(DWORD *dwFlags)
{
	CHECK_POINTER(dwFlags)

	*dwFlags = m_InitFlags;
	return S_OK;
}
//==========================================================================================================
//ROT Related implemenation
HRESULT CDispSvrDebug::AddIntoROT()
{
	WCHAR wszMonikerName[100];
	swprintf(wszMonikerName, sizeof(wszMonikerName), L"%s:%08x", DispSvrDbgMonikerID, GetCurrentProcessId());

	IMoniker *pMoniker;
	IRunningObjectTable *pROT;
	HRESULT hr;

	hr = GetRunningObjectTable(0, &pROT);
	if(SUCCEEDED(hr))
	{
		hr = CreateItemMoniker(MonikerDelimeter, wszMonikerName, &pMoniker);
		if(SUCCEEDED(hr))
		{
			hr = pROT->Register(ROTFLAGS_REGISTRATIONKEEPSALIVE, this,
				pMoniker, &m_dwDispSvrDbgRegID);
			pMoniker->Release();
		}
		pROT->Release();
	}
	return hr;
}

HRESULT CDispSvrDebug::RemoveFromROT()
{
	IRunningObjectTable *pROT;

	if(SUCCEEDED(GetRunningObjectTable(0, &pROT)))
	{
		pROT->Revoke(m_dwDispSvrDbgRegID);
		pROT->Release();
		return S_OK;
	}
	else
		return S_FALSE;
}

HRESULT CDispSvrDebug::SetRenderEngine(IDisplayRenderEngine *pRenderEngine)
{
	m_pRenderEngine = pRenderEngine;
	return S_OK;
}
