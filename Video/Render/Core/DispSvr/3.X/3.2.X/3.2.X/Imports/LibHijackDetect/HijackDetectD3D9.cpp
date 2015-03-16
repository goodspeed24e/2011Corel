#include <windows.h>
#include <d3d9.h>
#include "HijackDetectD3D9.h"

typedef IDirect3D9* (WINAPI *LPDIRECT3DCREATE9)(UINT);

DWORD g_HiJackFunctionsD3D9[] = 
{
	D3D9_OFFSET_ADDREF,
	D3D9_OFFSET_RELEASE,
	D3D9_OFFSET_RESET,
	D3D9_OFFSET_PRESENT,
	D3D9_OFFSET_BEGINSCENE,
	D3D9_OFFSET_ENDSCENE,
};

INT GetDetFuncNumD3D9()
{
	return sizeof(g_HiJackFunctionsD3D9)/sizeof(g_HiJackFunctionsD3D9[0]);
}

HRESULT LoadDeviceVTableD3D9(LPDWORD pVTableDetect, LPDWORD pDevice)
{
	if (!pVTableDetect || !pDevice)
		return E_POINTER;

	DWORD* pVTable = (DWORD*)(*((DWORD*)pDevice));
	for (INT i=0; i<sizeof(g_HiJackFunctionsD3D9)/sizeof(g_HiJackFunctionsD3D9[0]); i++)
	{
		pVTableDetect[i] = pVTable[g_HiJackFunctionsD3D9[i]];
	}

	return S_OK;
}

HRESULT LoadDeviceVTableOffsetD3D9(LPDWORD pVTableOffset, HWND hwnd)
{
	HRESULT hr = E_FAIL;

	if (!pVTableOffset)
		return E_POINTER;

	HMODULE hD3D9 = LoadLibrary(TEXT("d3d9.dll"));
	if (hD3D9 == NULL) 
	{
		return E_FAIL;
	}

	LPDIRECT3DCREATE9 lpfnDirect3DCreate9 = (LPDIRECT3DCREATE9)GetProcAddress(hD3D9, "Direct3DCreate9");
	if (lpfnDirect3DCreate9)
	{
		IDirect3D9* pD3D9 = Direct3DCreate9(D3D_SDK_VERSION);
		if (pD3D9)
		{
			D3DDISPLAYMODE d3ddm;
			hr = pD3D9->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3ddm);
			if (SUCCEEDED(hr))
			{
				D3DPRESENT_PARAMETERS d3dpp; 
				ZeroMemory(&d3dpp, sizeof(D3DPRESENT_PARAMETERS));
				d3dpp.Windowed = TRUE;
				d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
				d3dpp.BackBufferFormat = d3ddm.Format;

				IDirect3DDevice9* pDevice9 = NULL;
				hr = pD3D9->CreateDevice(
					D3DADAPTER_DEFAULT, 
					D3DDEVTYPE_HAL, 
					hwnd, 
					D3DCREATE_SOFTWARE_VERTEXPROCESSING, 
					&d3dpp, 
					&pDevice9);
				if (SUCCEEDED(hr))
				{
					DWORD* pVTable = (DWORD*)(*((DWORD*)pDevice9));
					for (INT i=0; i<sizeof(g_HiJackFunctionsD3D9)/sizeof(g_HiJackFunctionsD3D9[0]); i++)
					{
						pVTableOffset[i] = pVTable[g_HiJackFunctionsD3D9[i]] - (DWORD)hD3D9;
					}

					pDevice9->Release();
				}
			}
			pD3D9->Release();
		}
	}

	FreeLibrary(hD3D9);
	return S_OK;
}
