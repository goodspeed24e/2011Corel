#include "stdafx.h"
#include "DirectDrawHelper.h"

using namespace DispSvr;

HRESULT CDirectDrawHelper::QueryVideoMemorySize(DWORD* pdwTotalMem, DWORD* pdwFreeMem)
{
	LPDIRECTDRAW lpdd = NULL;
	HRESULT hr = DirectDrawCreate(NULL, &lpdd, NULL);
	if(SUCCEEDED(hr))
	{
		LPDIRECTDRAW7 lpdd7 = NULL;
		hr = lpdd->QueryInterface(IID_IDirectDraw7, (LPVOID*)&lpdd7);
		if(SUCCEEDED(hr))
		{
			DDSCAPS2 ddsCaps2;
			DWORD dwTotal=0, dwFree=0;
			// Initialize the structure.
			ZeroMemory(&ddsCaps2, sizeof(ddsCaps2));
			ddsCaps2.dwCaps = DDSCAPS_LOCALVIDMEM | DDSCAPS_HWCODEC;
			hr = lpdd7->GetAvailableVidMem(&ddsCaps2, &dwTotal, &dwFree);
			if(SUCCEEDED(hr))
			{
				if(pdwTotalMem)
					*pdwTotalMem =(dwTotal>>20); //rounded to MB
				if(pdwFreeMem)
					*pdwFreeMem = (dwFree>>20); //rounded to MB
			}
			lpdd7->Release();
		}
		lpdd->Release();
	}
	return hr;
}