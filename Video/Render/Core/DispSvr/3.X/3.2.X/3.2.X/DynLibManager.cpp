#include "stdafx.h"
#include "Exports/Inc/VideoEffect3D.h"
#include "VideoEffect3D/Downsampling/Downsampling.h"

using namespace DispSvr;

CDynLibManager::CDynLibManager() :
	pfnCreateDXGIFactory(0),
	pfnDirect3DCreate9Ex(0),
	pfnDwmIsCompositionEnabled(0),
	pfnDwmGetCompositionTimingInfo(0),
	pfnDwmSetPresentParameters(0),
	pfnDwmEnableComposition(0),
	pfnDwmGetWindowAttribute(0),
	pfnDwmSetWindowAttribute(0),
	pfnDwmSetDxFrameDuration(0),
	pfnDwmModifyPreviousDxFrameDuration(0),
	pfnDXVA2CreateVideoService(0),
	pfnDXVAHD_CreateDevice(0),
	pfnDXVA2CreateDirect3DDeviceManager9(0),
	pfnMFCreateVideoSampleFromSurface(0),
	pfnMFCreateMediaType(0),
	pfnCreateVideoProcessor(0),
	pfnGetNumberOfPhysicalMonitorsFromHMONITOR(0)
{
	HMODULE hD3D9 = NULL;
	HMODULE hDWMAPI = NULL;
	HMODULE hDXVA2 = NULL;
	HMODULE hEVR = NULL;
	HMODULE hMFPlat = NULL;
	HMODULE hVideoProcessor = NULL;
	HMODULE hDXGI = NULL;

	hD3D9 = LoadLibrary(TEXT("d3d9.dll"));
	hDWMAPI = LoadLibrary(TEXT("dwmapi.dll"));
	hMFPlat = LoadLibrary(TEXT("mfplat.dll"));
	hVideoProcessor = LoadLibrary(TEXT("videoprocessor.dll"));
	hDXGI = LoadLibrary(TEXT("dxgi.dll"));

	DWORD dwOSVersion = GetRegistry(REG_OS_VERSION, 0);
	if (dwOSVersion >= OS_VISTA) // window vista or later
	{ 
		hDXVA2 = LoadLibrary(TEXT("dxva2.dll"));
		hEVR = LoadLibrary(TEXT("evr.dll"));
	}

	if (hD3D9)
	{
		pfnDirect3DCreate9Ex = (TpfnDirect3DCreate9Ex) GetProcAddress(hD3D9, "Direct3DCreate9Ex");
		m_listLoadedLibraries.push_back(hD3D9);
	}

	if (hDWMAPI)
	{
		pfnDwmIsCompositionEnabled = (TpfnDwmIsCompositionEnabled) GetProcAddress(hDWMAPI, "DwmIsCompositionEnabled");
		pfnDwmGetCompositionTimingInfo = (TpfnDwmGetCompositionTimingInfo) GetProcAddress(hDWMAPI, "DwmGetCompositionTimingInfo");
		pfnDwmSetPresentParameters = (TpfnDwmSetPresentParameters) GetProcAddress(hDWMAPI, "DwmSetPresentParameters");
		pfnDwmEnableComposition = (TpfnDwmEnableComposition) GetProcAddress(hDWMAPI, "DwmEnableComposition");
		pfnDwmGetWindowAttribute = (TpfnDwmGetWindowAttribute) GetProcAddress(hDWMAPI, "DwmGetWindowAttribute");
		pfnDwmSetWindowAttribute = (TpfnDwmSetWindowAttribute) GetProcAddress(hDWMAPI, "DwmSetWindowAttribute");
		pfnDwmSetDxFrameDuration = (TpfnDwmSetDxFrameDuration) GetProcAddress(hDWMAPI, "DwmSetDxFrameDuration");
		pfnDwmModifyPreviousDxFrameDuration = (TpfnDwmModifyPreviousDxFrameDuration) GetProcAddress(hDWMAPI, "DwmModifyPreviousDxFrameDuration");
		pfnDwmEnableMMCSS = (TpfnDwmEnableMMCSS) GetProcAddress(hDWMAPI, "DwmEnableMMCSS");
		m_listLoadedLibraries.push_back(hDWMAPI);
	}

	if (hDXVA2)
	{
		pfnDXVA2CreateVideoService = (TpIviFnDXVA2CreateVideoService) GetProcAddress(hDXVA2, "DXVA2CreateVideoService");
		pfnDXVA2CreateDirect3DDeviceManager9 = (TpfnDXVA2CreateDirect3DDeviceManager9) GetProcAddress(hDXVA2, "DXVA2CreateDirect3DDeviceManager9");
		pfnDXVAHD_CreateDevice = (PDXVAHD_CreateDevice) GetProcAddress(hDXVA2, "DXVAHD_CreateDevice");
		pfnGetNumberOfPhysicalMonitorsFromHMONITOR = (TpfnGetNumberOfPhysicalMonitorsFromHMONITOR) GetProcAddress(hDXVA2, "GetNumberOfPhysicalMonitorsFromHMONITOR");
		m_listLoadedLibraries.push_back(hDXVA2);
	}

	if (hEVR)
	{
		pfnMFCreateVideoSampleFromSurface = (TpfnMFCreateVideoSampleFromSurface) GetProcAddress(hEVR, "MFCreateVideoSampleFromSurface");
		m_listLoadedLibraries.push_back(hEVR);
	}

	if (hMFPlat)
	{
		pfnMFCreateMediaType = (TpfnMFCreateMediaType) GetProcAddress(hMFPlat, "MFCreateMediaType");
		m_listLoadedLibraries.push_back(hMFPlat);
	}

	if (hVideoProcessor)
	{
		pfnCreateVideoProcessor = (TpfnCreateVideoProcessor) GetProcAddress(hVideoProcessor, "CreateVideoProcessor");
		m_listLoadedLibraries.push_back(hVideoProcessor);
	}

	if (hDXGI)
	{
		pfnCreateDXGIFactory = (TpfnCreateDXGIFactory) GetProcAddress(hDXGI, "CreateDXGIFactory");
		m_listLoadedLibraries.push_back(hDXGI);
	}
}

CDynLibManager::~CDynLibManager()
{
	for (std::list<HINSTANCE>::const_iterator i = m_listLoadedLibraries.begin();
		i != m_listLoadedLibraries.end();
		++i)
	{
		if (*i)
			FreeLibrary(*i);
	}
}

#ifdef ENABLE_VIDEO_EFFECT_DLL

#include <string>
typedef std::basic_string<TCHAR> String;
#define EFFECT_PLUGIN_SUFFIX	"ve3d"

void CDynLibManager::LoadVideoEffect()
{
	CComPtr<IDispSvrVideoEffect3DManager> pManager = 0;
	HRESULT hr = CResourceManager::GetInstance()->GetInterface(DISPSVR_RESOURCE_VIDEOEFFECT3DMANAGER,
		__uuidof(IDispSvrVideoEffect3DManager),
		(void **) &pManager);
	if (FAILED(hr))
		return;

	TCHAR szFilename[MAX_PATH] = "";

	if (0 == ::GetModuleFileName(GetModuleHandle(_T("dispsvr.dll")), szFilename, MAX_PATH))
		return;

	IDispSvrVideoEffect3DPlugin *pPlugin;
	HMODULE hPlugin;
	TpfnCreateVideoEffect3DPlugin pProc;
	WIN32_FIND_DATA ffd;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	String strFullPath;
	String strModulePath;

	try
	{
		strFullPath = szFilename;
		strModulePath = strFullPath.substr(0, strFullPath.find_last_of('\\') + 1);
		hFind = FindFirstFile(String(strModulePath + _T("*." EFFECT_PLUGIN_SUFFIX)).c_str(), &ffd);
	}
	catch (...)	// std::string may throw std::exception instance.
	{
	}

	if (INVALID_HANDLE_VALUE == hFind)
		return;

	do
	{
		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			continue;

		try
		{
			strFullPath = strModulePath + ffd.cFileName;
			hPlugin = LoadLibrary(strFullPath.c_str());
			if (hPlugin == NULL)
				continue;
		}
		catch (...)	// std::string may throw std::exception instance.
		{
			continue;
		}

		pProc = (TpfnCreateVideoEffect3DPlugin) GetProcAddress(hPlugin, _T("CreatePlugin"));
		if (pProc)
		{
			hr = pProc(&pPlugin);
			if (SUCCEEDED(hr))
			{
				hr = pManager->Install(pPlugin);
				pPlugin->Release();
				// keep the loaded library alive until DynLibManager is released.
				m_listLoadedLibraries.push_back(hPlugin);
				continue;
			}
		}

		FreeLibrary(hPlugin);
	} while (FindNextFile(hFind, &ffd) != 0);

	FindClose(hFind);
}
#else

void CDynLibManager::LoadVideoEffect()
{
    CComPtr<IDispSvrVideoEffect3DManager> pManager;
    HRESULT hr = E_FAIL;
    hr = CResourceManager::GetInstance()->GetInterface(DISPSVR_RESOURCE_VIDEOEFFECT3DMANAGER,
                                                       __uuidof(IDispSvrVideoEffect3DManager),
                                                       (void **) &pManager);

    if (FAILED(hr))
        return;
    
    // Load Default Effect, the first one is downsampling
	IDispSvrVideoEffect3DPlugin *pPlugin = NULL;
    hr = CDownsamplingEffect::Create(&pPlugin);
    if (SUCCEEDED(hr) && pPlugin)
    {
        hr = pManager->Install(pPlugin);
        pPlugin->Release();
    }
}

#endif	// ENABLE_VIDEO_EFFECT_DLL
