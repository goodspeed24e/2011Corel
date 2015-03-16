// HVDService.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include <windows.h>
#include <streams.h>
#include <d3d9.h>
#include <dxva2api.h>
#include <evr.h>
#include "HVDSelector.h"
#include "Exports\Inc\HVDService.h"
#include "HVDServiceDxva1.h"
#include "HVDServiceDxva2.h"


#ifdef _MANAGED
#pragma managed(push, off)
#endif

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
			DisableThreadLibraryCalls(hModule);
		}
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
    return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif

HVDSERVICE_API int CreateHVDService(DWORD dwService, void** ppHVDService)
{
	if (!ppHVDService)
		return -1;

	if (HVDService::HVD_SERVICE_DXVA1 == dwService)
	{
		HVDService::CHVDServiceDxva1* pService = 0;
		pService = new HVDService::CHVDServiceDxva1;
		if (pService)
		{
			HRESULT hr = pService->QueryInterface(__uuidof(IHVDService), ppHVDService);
			if (SUCCEEDED(hr) && *ppHVDService)
				return 0;
		}
	}
	else if (HVDService::HVD_SERVICE_DXVA2 == dwService)
	{
		HVDService::CHVDServiceDxva2* pService = 0;
		pService = new HVDService::CHVDServiceDxva2;
		if (pService)
		{
			HRESULT hr = pService->QueryInterface(__uuidof(IHVDService), ppHVDService);
			if (SUCCEEDED(hr) && *ppHVDService)
				return 0;
			else
				SAFE_DELETE(pService);
		}
	}
	return -1;
}
