#include "HijackDetect.h"
#include "HijackDetectDD.h"
#include "HijackDetectD3D9.h"

#define IsOutOfRange(p, base, range) ((p) < (base) || (p) > (base + range))

extern DWORD g_HiJackFunctionsD3D9[];
extern DWORD g_HiJackFunctionsDD[];

CHijackDetect::CHijackDetect()
{
	m_Targets = 0;
	m_pInterface = NULL;
	m_pVTable = NULL;
	m_dwVTableSize = 0;
	m_pDLLBase = NULL;
	m_dwDLLSize = 0;
	m_iIsHiJackedCounter = 0;
	m_hrLastIsHiJackedStatus = E_FAIL;
	m_lpfGetModuleInformation = NULL;
	m_hmPSAPI = LoadLibrary(TEXT("psapi.dll"));
	if (m_hmPSAPI)
	{
		m_lpfGetModuleInformation = (LPGETMODULEINFORMATION)GetProcAddress(m_hmPSAPI, "GetModuleInformation");
	}
}

CHijackDetect::~CHijackDetect()
{
	if (m_hmPSAPI)
		FreeLibrary(m_hmPSAPI);

	if (m_pVTable)
	{
		delete [] m_pVTable;
		m_pVTable = NULL;
	}
	m_dwVTableSize = 0;
}

HRESULT CHijackDetect::SetDesc(LPHJDETDESC lpHJDD)
{
	HRESULT hr = S_OK;

	if (!lpHJDD)
		return E_INVALIDARG;

	HMODULE hmDLL = NULL; 
	MODULEINFO MInfo;

	if (m_pVTable)
	{
		delete [] m_pVTable;
		m_pVTable = NULL;
	}
	switch (lpHJDD->Targets)
	{
	case HJ_TARGET_D3D9:
		m_dwVTableSize = GetDetFuncNumD3D9();
		// Get basic module information.
		hmDLL = GetModuleHandle(TEXT("d3d9.dll"));
		m_dwDLLSize = D3D9_IMAGESIZE;
		break;
	case HJ_TARGET_DDRAW:
		m_dwVTableSize = GetDetFuncNumDD();
		// Get basic module information.
		hmDLL = GetModuleHandle(TEXT("ddraw.dll"));
		m_dwDLLSize = DD_IMAGESIZE;
		break;
	default:
		break;
	}
	m_pInterface = lpHJDD->pInterface;
	m_pVTable = new DWORD[m_dwVTableSize];

	if (hmDLL)
	{
		// Get module handle base address.
		if (m_lpfGetModuleInformation && 
			m_lpfGetModuleInformation(GetCurrentProcess(), hmDLL, &MInfo, sizeof(MODULEINFO)))
		{
			m_pDLLBase = (LPBYTE)MInfo.lpBaseOfDll;
			m_dwDLLSize = MInfo.SizeOfImage;
		}
		else
		{
			m_pDLLBase = (LPBYTE)hmDLL;
		}
	}
	
	if (m_pInterface)
	{
		switch(lpHJDD->Targets)
		{
		case HJ_TARGET_D3D9:
			hr = LoadDeviceVTableD3D9(m_pVTable, m_pInterface);
			break;
		case HJ_TARGET_DDRAW:
			hr = LoadSurfaceVTableDD(m_pVTable, m_pInterface);
			break;
		default:
			break;
		}
	}
	else
	{
		switch(lpHJDD->Targets)
		{
		case HJ_TARGET_D3D9:
			// Get VTable functions offset (opposite to module handle).
			hr = LoadDeviceVTableOffsetD3D9(m_pVTable, lpHJDD->hWnd);
			break;
		case HJ_TARGET_DDRAW:
			hr = LoadSurfaceVTableOffsetDD(m_pVTable);
			break;
		default:
			break;
		}
		// Get VTable functions pointer
		for (DWORD i=0; i<m_dwVTableSize; i++)
			m_pVTable[i] = m_pVTable[i] + (DWORD)(m_pDLLBase);
	}

	m_Targets = lpHJDD->Targets;
	return hr;
}

HRESULT CHijackDetect::IsHiJacked(BOOL& bHiJacked)
{
	HRESULT hr = E_FAIL;

	if (m_Targets && ((m_iIsHiJackedCounter%IsHiJackedCounter)==0))
	{
		if (m_iIsHiJackedCounter==IsHiJackedCounterReset)
			m_iIsHiJackedCounter = 0;

		LPDWORD pHJFuncs;
		switch (m_Targets)
		{
		case HJ_TARGET_D3D9:
			pHJFuncs = g_HiJackFunctionsD3D9;
			break;
		case HJ_TARGET_DDRAW:
			pHJFuncs = g_HiJackFunctionsDD;
			break;
		default:
			break;
		}

		// Check 1. Check if function are modified to jump to outside of dll.
		hr = DetectHiJackJMP(m_pVTable, m_dwVTableSize, m_pDLLBase, m_dwDLLSize, bHiJacked);
		m_hrLastIsHiJackedStatus = hr;
		if (SUCCEEDED(hr) && bHiJacked)
			return m_hrLastIsHiJackedStatus;

		// Check 2. Check if functions in vtable are modified.
		hr = DetectHiJackVT(m_pInterface, pHJFuncs, m_dwVTableSize, m_pDLLBase, m_dwDLLSize, bHiJacked);
		m_hrLastIsHiJackedStatus = hr;
		if (SUCCEEDED(hr) && bHiJacked)
			return m_hrLastIsHiJackedStatus;
	}

	m_iIsHiJackedCounter++;
	return m_hrLastIsHiJackedStatus;
}

HRESULT CHijackDetect::DetectHiJackJMP(LPDWORD pVTable, DWORD dwVTableSize, LPBYTE pBase, DWORD dwRange, BOOL& bHiJacked)
{
	if (!pVTable || !pBase || dwRange==0)
		return E_INVALIDARG;

	for (DWORD i=0; i<dwVTableSize; i++)
	{
		LPBYTE pAPI = (LPBYTE)pVTable[i];
		if (*pAPI == 0xe9)
		{
			DWORD offset = (DWORD)(*((LPBYTE*)(pAPI + 1)));
			LPBYTE pAddr = (pAPI + 5) + offset;
			if (IsOutOfRange(pAddr, pBase, dwRange))
			{
				bHiJacked = TRUE;
				break;
			}
		}
	}

	return S_OK;
}

HRESULT CHijackDetect::DetectHiJackVT(LPDWORD pInterface, LPDWORD pHJFuncs, DWORD dwVTableSize, LPBYTE pBase, DWORD dwRange, BOOL& bHiJacked)
{
	if (!pInterface || !pHJFuncs || !pBase || dwRange==0)
		return E_INVALIDARG;

	DWORD* pVTable = (DWORD*)(*pInterface);
	for (DWORD i=0; i<dwVTableSize; i++)
	{
		LPBYTE pAPI = (LPBYTE)pVTable[pHJFuncs[i]];
		if (IsOutOfRange(pAPI, pBase, dwRange))
		{
			bHiJacked = TRUE;
			break;
		}
	}

	return S_OK;
}
