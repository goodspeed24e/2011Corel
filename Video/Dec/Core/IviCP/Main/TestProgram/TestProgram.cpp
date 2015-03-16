#include <stdio.h>
#include <tchar.h>
#include <objbase.h>
#include <InitGuid.h>
#include "..\CPGuids.h"
#include "..\CPInterfaces.h"
#include "..\Win7\d3d9.h"
#include <dxva2api.h>
#include <atlbase.h>

#define VIDEO_WIDTH 1920
#define VIDEO_HEIGHT 1088

#include "dispsvr.h"
#include "dispsvr_i.c"
#include "HVDService.h"

//DEFINE_GUID(DXVADDI_Intel_ModeH264_E, 0x604f8e68, 0x4951, 0x4c54, 0x88, 0xfe, 0xab, 0xd2, 0x5c, 0x15, 0xb3, 0xd6);

// The window's message handler
static LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

HWND CreateAWindow()
{
	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L,
		GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
		"Ivicp Test", NULL };
	RegisterClassEx( &wc );

	DWORD dwWindowStyle = WS_DISABLED;//WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
	// Create the application's window
	return CreateWindow( wc.lpszClassName, "Test Window",
		dwWindowStyle,
		0, 0, 
		VIDEO_WIDTH, 
		VIDEO_HEIGHT,
		NULL, NULL, wc.hInstance, NULL );
}

unsigned int __stdcall CreateD3DDevice(void *arg)
{
	HRESULT hr = E_FAIL;

	IDirect3DDevice9Ex **ppD3DDevice = static_cast<IDirect3DDevice9Ex**>(arg);

	CComPtr<IDisplayServer> pDispSvr;		
	CComPtr<IDisplayRenderEngine> pRenderEngine;

	CoInitialize(NULL);

	hr = pDispSvr.CoCreateInstance(CLSID_DisplayServer);
	
	hr = pDispSvr->Initialize(0x1da0, CreateAWindow(), NULL);				
	
	hr = pDispSvr->GetRenderEngine(&pRenderEngine);
	
	hr = pRenderEngine->Get3DDevice((IUnknown**)ppD3DDevice);

	CoUninitialize();
	
	return 0;
}

typedef int (__cdecl *HVDService_CreateFcn)(DWORD dwService, void** ppHVDService);
HRESULT CreateHVDService(IHVDService **ppIHVDService, IDirect3DDevice9Ex *pD3DDevice)
{
	HRESULT hr = E_FAIL;
	HMODULE hLib;
	HVDService_CreateFcn m_lpHVDServiceCreate;

	hLib = LoadLibrary("HVDService.dll");

	if (hLib)
	{
		m_lpHVDServiceCreate = (HVDService_CreateFcn)GetProcAddress(hLib, "CreateHVDService");
		if (m_lpHVDServiceCreate)
		{
			hr = (*m_lpHVDServiceCreate)(1/*HVD_SERVICE_DXVA2*/, (void**)ppIHVDService);
		}
	}	

	if ((*ppIHVDService)!=NULL)
	{
		HVDService::HVDInitConfig HVDInit;
		ZeroMemory(&HVDInit, sizeof(HVDService::HVDInitConfig));

		HVDInit.dwFlags = HVDService::HVD_INIT_D3DDEVICE;		
		HVDInit.pExternalDevice = pD3DDevice;		
		hr = (*ppIHVDService)->Initialize(&HVDInit);

		HVDService::HVDDecodeConfig m_HVDDecodeConfig;
		if (SUCCEEDED(hr))
		{
			//HVD Start Service Default
			ZeroMemory(&m_HVDDecodeConfig, sizeof(HVDService::HVDDecodeConfig));
			m_HVDDecodeConfig.dwFlags = HVDService::HVD_DECODE_MAXSURFACECOUNT | HVDService::HVD_DECODE_MODE | HVDService::HVD_DECODE_UABPROTECTLEVEL;		
			m_HVDDecodeConfig.dwFlags |= HVDService::HVD_DECODE_ENCRYPTION_GPUCP;													
			m_HVDDecodeConfig.dwMode = HVDService::HVD_MODE_MPEG2;
			m_HVDDecodeConfig.dwLevel = HVDService::HVD_LEVEL_AUTO;
			m_HVDDecodeConfig.dwWidth = VIDEO_WIDTH;
			m_HVDDecodeConfig.dwHeight = VIDEO_HEIGHT;
			m_HVDDecodeConfig.dwMaxSurfaceCount = 12;
			m_HVDDecodeConfig.dwMinSurfaceCount = 5;
			m_HVDDecodeConfig.dwUABProtectionLevel = 0;

			hr = (*ppIHVDService)->StartService(&m_HVDDecodeConfig);		
		}		
	}
	else
	{
		return E_FAIL;
	}

	return hr;
}

IUnknown* GetAccel(IHVDService *pIHVDService)
{
	//get video accelerator
	IUnknown *pVideoAccel = NULL;
	IHVDServiceDxva *pIHVDServiceDxva;
	pIHVDService->QueryInterface(__uuidof(IHVDServiceDxva), (void**)&pIHVDServiceDxva);
	if (pIHVDServiceDxva)
	{
		pIHVDServiceDxva->GetAccel(&pVideoAccel);			
		pIHVDServiceDxva->Release();		
	}

	return pVideoAccel;
}

unsigned int __stdcall DoEPID(void *arg)
{
	HRESULT hr = E_FAIL;

	GUID guidTest = DXVA2_ModeMPEG2_VLD;	

	IDirect3DDevice9Ex *pD3DDevice = static_cast<IDirect3DDevice9Ex*>(arg);
	CComPtr<IDirectXVideoDecoder> pDXVDec;
	CComPtr<IHVDService> pIHVDService;				
	CComPtr<ICPService> pCP;				

	CoInitialize(NULL);

	do
	{

	hr = CreateHVDService(&pIHVDService, pD3DDevice);
	if(FAILED(hr))
	break;

	pDXVDec = (IDirectXVideoDecoder*)GetAccel(pIHVDService);

	hr = CoCreateInstance(CLSID_IVICP_WIN7, NULL, CLSCTX_INPROC_SERVER,IID_ICPService, (void**)&pCP);							
	if(FAILED(hr))
	break;

	CPOpenOptions openoption;
	openoption.dwCPOption = E_CP_OPTION_ON + E_CP_OPTION_DBG_MSG + E_CP_OPTION_I_ONLY;
	openoption.pD3D9 = pD3DDevice;
	openoption.pVA = pDXVDec;
	openoption.bProtectedBlt = FALSE;
	openoption.dwFrameWidth = 0;
	openoption.dwDeviceID = 0;
	openoption.pDecodeProfile = &guidTest;

	hr = pCP->Open(&openoption);
	if(SUCCEEDED(hr))
		printf("succeed to open ivicp!");
	else
		printf("failed to open ivicp!");
	
	}while(0);

	hr = pCP->Close(); 	
	hr = pIHVDService->Uninitialize();

	CoUninitialize();

	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
	HRESULT hr = E_FAIL;	

	//GUID guidTest = DXVA2_ModeMPEG2_VLD;	
	
	CComPtr<IDisplayServer> pDispSvr;
	CComPtr<IDirect3DDevice9Ex> pD3DDevice;
	//CComPtr<IDirectXVideoDecoder> pDXVDec;
	//CComPtr<IHVDService> pIHVDService;				
	//CComPtr<ICPService> pCP;				

	CComPtr<IDisplayRenderEngine> pRenderEngine;					

	CoInitialize(NULL);

	do 
	{	
		/*hr = pDispSvr.CoCreateInstance(CLSID_DisplayServer);
		if(FAILED(hr))
			break;
		
		hr = pDispSvr->Initialize(0x1da0, CreateAWindow(), NULL);				
		if(FAILED(hr))
			break;
		
		hr = pDispSvr->GetRenderEngine(&pRenderEngine);
		if(FAILED(hr))
			break;

		hr = pRenderEngine->Get3DDevice((IUnknown**)&pD3DDevice);
		if(FAILED(hr))
			break;*/

		HANDLE hThread;
		unsigned int dwThreadID;
		hThread = (HANDLE)_beginthreadex(NULL, 0, CreateD3DDevice, &pD3DDevice, 0, &dwThreadID);
		WaitForSingleObject(hThread, INFINITE);

		HANDLE hThread2;
		unsigned int dwThreadID2;
		hThread2 = (HANDLE)_beginthreadex(NULL, 0, DoEPID, pD3DDevice, 0, &dwThreadID2);
		WaitForSingleObject(hThread2, INFINITE);
				
		/*hr = CreateHVDService(&pIHVDService, pD3DDevice);
		if(FAILED(hr))
			break;

		pDXVDec = (IDirectXVideoDecoder*)GetAccel(pIHVDService);
		
		hr = CoCreateInstance(CLSID_IVICP_WIN7, NULL, CLSCTX_INPROC_SERVER,IID_ICPService, (void**)&pCP);							
		if(FAILED(hr))
			break;
						
		CPOpenOptions openoption;
		openoption.dwCPOption = E_CP_OPTION_ON + E_CP_OPTION_DBG_MSG + E_CP_OPTION_I_ONLY;
		openoption.pD3D9 = pD3DDevice;
		openoption.pVA = pDXVDec;
		openoption.bProtectedBlt = FALSE;
		openoption.dwFrameWidth = 0;
		openoption.dwDeviceID = 0;
		openoption.pDecodeProfile = &guidTest;

		hr = pCP->Open(&openoption);
		if(SUCCEEDED(hr))
			printf("succeed to open ivicp!");
		else
			printf("failed to open ivicp!");*/

		break;

	} while (0);
	
	CoUninitialize();

	return 0;
}
