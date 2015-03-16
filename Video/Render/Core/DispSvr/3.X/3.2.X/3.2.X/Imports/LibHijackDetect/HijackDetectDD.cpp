#include <windows.h>
#include <ddraw.h>
#include "HijackDetectDD.h"

DWORD g_HiJackFunctionsDD[] = 
{
	DD_OFFSET_ADDREF,
	DD_OFFSET_RELEASE,
	DD_OFFSET_FLIP,
};

INT GetDetFuncNumDD()
{
	return sizeof(g_HiJackFunctionsDD)/sizeof(g_HiJackFunctionsDD[0]);
}

HRESULT LoadSurfaceVTableDD(LPDWORD pVTableDetect, LPDWORD pSurface)
{
	if (!pVTableDetect || !pSurface)
		return E_POINTER;

	DWORD* pVTable = (DWORD*)(*((DWORD*)pSurface));
	for (INT i=0; i<sizeof(g_HiJackFunctionsDD)/sizeof(g_HiJackFunctionsDD[0]); i++)
	{
		pVTableDetect[i] = pVTable[g_HiJackFunctionsDD[i]];
	}

	return S_OK;
}

HRESULT LoadSurfaceVTableOffsetDD(LPDWORD pVTableOffset)
{
	HRESULT hr = E_FAIL;

	if (!pVTableOffset)
		return E_POINTER;

	HMODULE hDD = LoadLibrary(TEXT("ddraw.dll"));
	if (hDD == NULL) 
	{
		return E_FAIL;
	}

	LPDIRECTDRAW lpfnDirectDraw = (LPDIRECTDRAW)GetProcAddress(hDD, "DirectDraw");
	if (lpfnDirectDraw)
	{
		IDirectDraw* pDD;
		if (DirectDrawCreate(NULL, &pDD, NULL) == DD_OK)
		{
			LPDIRECTDRAW4 lpDD4;
			if (pDD->QueryInterface(IID_IDirectDraw4,(void **)&lpDD4) != S_OK)
			{
				pDD->Release();
				return E_FAIL;
			}
			pDD->Release();	

			DDSURFACEDESC2 ddsd2;
			LPDIRECTDRAWSURFACE4 lpDDSurface4;
			ZeroMemory(&ddsd2,sizeof(DDSURFACEDESC2));
			ddsd2.dwSize = sizeof(ddsd2);
			ddsd2.dwFlags = DDSD_CAPS;
			ddsd2.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
			if (lpDD4->CreateSurface(&ddsd2, &lpDDSurface4, NULL) != DD_OK)
			{
				lpDD4->Release();
				return E_FAIL;
			}
			DWORD* pVTable = (DWORD*)(*((DWORD*)lpDD4));
			for (INT i=0; i<sizeof(g_HiJackFunctionsDD)/sizeof(g_HiJackFunctionsDD[0]); i++)
			{
				pVTableOffset[i] = pVTable[g_HiJackFunctionsDD[i]] - (DWORD)hDD;
			}
			lpDD4->Release();
		}
	}
	else
	{
		LPDIRECTDRAW7 lpfnDirectDraw7 = (LPDIRECTDRAW7)GetProcAddress(hDD, "DirectDraw");
		if (lpfnDirectDraw7)
		{
			IDirectDrawSurface7* pDDSurface7;
			if (DirectDrawCreateEx(NULL, (void**)&pDDSurface7, IID_IDirectDraw7, NULL) == DD_OK)
			{
				DWORD* pVTable = (DWORD*)(*((DWORD*)pDDSurface7));
				for (INT i=0; i<sizeof(g_HiJackFunctionsDD)/sizeof(g_HiJackFunctionsDD[0]); i++)
				{
					pVTableOffset[i] = pVTable[g_HiJackFunctionsDD[i]] - (DWORD)hDD;
				}
				pDDSurface7->Release();
			}
		}
	}
	FreeLibrary(hDD);
	return S_OK;
}
