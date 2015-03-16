//
// DispSvrVideoEffectProbe is used to probe and list video effect plugins which can be used by DispSvr.
//
#include <tchar.h>
#include <atlbase.h>
#include "../DispSvr.h"
#include <initguid.h>
#include <dxva2api.h>
#include "../Exports/Inc/VideoMixer.h"
#include "../Exports/Inc/VideoEffect3D.h"

#define P(fmt, ...) printf(fmt, __VA_ARGS__)
using namespace DispSvr;

void PrintBufferCaps(UINT c)
{
	P("\t");
	if (c & EFFECT_BUFFER_CAPS_TEXTURE)
		P("EFFECT_BUFFER_CAPS_TEXTURE ");
	if (c & EFFECT_BUFFER_CAPS_LOCKABLE)
		P("EFFECT_BUFFER_CAPS_LOCKABLE ");
	P("\n");
}

void PrintCaps(const VE3DManagerCaps &c)
{
	P("IDispSvrVideoEffect3DManager capabilities:\n");
	P("Type: %s\n", c.eType == EFFECT_MANAGER_TYPE_SHADER_BASED ? "EFFECT_MANAGER_TYPE_SHADER_BASED" : "UNKNOWN");
	P("uInputBufferCaps = %d:\n", c.uInputBufferCaps);
	PrintBufferCaps(c.uInputBufferCaps);
	P("uOutputBufferCaps = %d:\n", c.uOutputBufferCaps);
	PrintBufferCaps(c.uOutputBufferCaps);
	P("\n");
}

void ListVideoEffects(IDispSvrVideoEffect3DManager *pManager)
{
	HRESULT hr;
	UINT uCount = 0;
	IDispSvrVideoEffect3DPlugin *pPlugin;
	GUID guidPlugin;
	BOOL bEnable;
	OLECHAR strBuffer[MAX_PATH];
	VE3DManagerCaps caps;

	ZeroMemory(&caps, sizeof(VE3DManagerCaps));
	hr = pManager->GetCaps(&caps);
	if (FAILED(hr))
	{
		P("IDispSvrVideoEffect3DManager::GetCaps() failed, hr = 0x%x\n", hr);
	}

	PrintCaps(caps);

	hr = pManager->GetEffectCount(&uCount);
	if (FAILED(hr))
	{
		P("IDispSvrVideoEffect3DManager::GetEffectCount() failed, hr = 0x%x\n", hr);
		return;
	}

	P("--------------------------------------------------------------------------\n");
	P("IDispSvrVideoEffect3DManager has %d video effect plugin(s).\n", uCount);
	for (UINT i = 0; i < uCount; i++)
	{
		pPlugin = 0;
		hr = pManager->GetEffectAt(i, &pPlugin);
		if (FAILED(hr))
		{
			P("IDispSvrVideoEffect3DManager::GetEffectAt(%d) failed, hr = 0x%x\n", i, hr);
			continue;
		}

		guidPlugin = GUID_NULL;
		hr = pPlugin->GetResourceId(&guidPlugin);
		if (FAILED(hr))
		{
			P("Index %d IDispSvrVideoEffect3DPlugin::GetResourceId() failed, hr = 0x%x\n", i, hr);
		}

		bEnable = FALSE;
		hr = pManager->IsEnabled(pPlugin, &bEnable);
		if (FAILED(hr))
		{
			P("Index %d IDispSvrVideoEffect3DManager::IsEnabled() failed, hr = 0x%x\n", i, hr);
		}

		hr = StringFromGUID2(guidPlugin, strBuffer, sizeof(strBuffer));
		P("index %d: %S, %s\n", i, strBuffer, bEnable ? "*ENABLED*" : "*disabled*");

		pPlugin->Release();
	}
}

int main(int argc, char* argv[])
{
	CoInitialize(NULL);
	{

		DWORD dwDispSvrFlags = DISPSVR_USE_D3D9EX | DISPSVR_NO_RENDER_THREAD;
		CComPtr<IDisplayServer> pDispSvr;
		HWND hwndDevice = CreateWindow(_T("STATIC"), _T("DispSvr"), 0, 0, 0, 16, 16, 0, 0, 0, 0);
		HRESULT hr = pDispSvr.CoCreateInstance(CLSID_DisplayServer);

		if (FAILED(hr))
		{
			P("CoCreateInstance of CLSID_DisplayServer failed. hr=0x%x\n", hr);
			return hr;
		}

		hr = pDispSvr->Initialize(dwDispSvrFlags, hwndDevice, NULL);
		if (FAILED(hr))
		{
			P("pDispSvr->Initialize failed. hr=0x%x\n", hr);
			return hr;
		}

		CComPtr<IDisplayRenderEngine> pRenderEngine;
		hr = pDispSvr->GetRenderEngine(&pRenderEngine);
		if (FAILED(hr))
		{
			P("pDispSvr->GetRenderEngine failed. hr=0x%x\n", hr);
			return hr;
		}

		CComQIPtr<IDispSvrVideoMixer> pVideoMixer = pRenderEngine;
		if (CComQIPtr<IDispSvrVideoEffect3DManager> pManager = pVideoMixer)
		{
			ListVideoEffects(pManager);
		}
		else
		{
			P("this DispSvr does not support IDispSvrVideoEffect3DManager interface.\n");
		}

		pDispSvr->Terminate();
		CloseWindow(hwndDevice);

	}
	CoUninitialize();

	return 0;
}

