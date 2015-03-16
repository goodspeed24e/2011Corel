#ifndef _DISPSVR_DYN_LIB_
#define _DISPSVR_DYN_LIB_

#include <list>
#include <d3dx9.h>
#include <dxgi.h>
#include <dwmapi.h>
#include "Imports/ThirdParty/Microsoft/DXVAHD/dxvahd.h"
#include "Imports/Inc/VideoProcessor.h"
#include "Singleton.h"

interface IDirect3DDeviceManager9;
interface IMFSample;
interface IMFMediaType;

namespace DispSvr
{
	/// Manage all dynamically loaded library and functions.
	class CDynLibManager : public DispSvr::Singleton<CDynLibManager>
	{
	public:

		typedef HRESULT (__stdcall *TpfnCreateDXGIFactory)(REFIID riid, void **ppFactory);
		typedef HRESULT (__stdcall *TpfnDirect3DCreate9Ex)(UINT uVersion, IDirect3D9Ex **ppService);
		typedef HRESULT (__stdcall *TpfnDwmIsCompositionEnabled)(BOOL *pfEnabled);
		typedef HRESULT (__stdcall *TpfnDwmGetCompositionTimingInfo)(HWND hwnd, DWM_TIMING_INFO *pTimingInfo);
		typedef HRESULT (__stdcall *TpfnDwmSetPresentParameters)(HWND hwnd, DWM_PRESENT_PARAMETERS *pPresentParams);
		typedef HRESULT (__stdcall *TpfnDwmSetDxFrameDuration)(HWND hwnd, INT cRefreshes);
		typedef HRESULT (__stdcall *TpfnDwmModifyPreviousDxFrameDuration)(HWND hwnd, INT cRefreshes, BOOL fRelative);
		typedef HRESULT (__stdcall *TpfnDwmEnableComposition)(UINT uCompositionAction);
		typedef HRESULT (__stdcall *TpfnDwmGetWindowAttribute)(HWND hwnd, DWORD dwAttribute, PVOID pvAttribute, DWORD cbAttribute);
		typedef HRESULT (__stdcall *TpfnDwmSetWindowAttribute)(HWND hwnd, DWORD dwAttribute, LPCVOID pvAttribute, DWORD cbAttribute);
		typedef HRESULT  (__stdcall *TpIviFnDXVA2CreateVideoService)(IDirect3DDevice9* pDD, REFIID riid, void** ppService);
		typedef HRESULT  (__stdcall *TpfnDXVA2CreateDirect3DDeviceManager9)(UINT *pToken, IDirect3DDeviceManager9 **);
		typedef HRESULT (__stdcall *TpfnMFCreateVideoSampleFromSurface)(IUnknown* pUnkSurface, IMFSample** ppSample);
		typedef HRESULT (__stdcall *TpfnMFCreateMediaType)(IMFMediaType **ppMFType);
		typedef HRESULT (__stdcall *TpfnDwmEnableMMCSS)(BOOL fEnableMMCSS);
		// CreateVideoProcessor as defined in Imports/Inc/VideoProcessor.h
		typedef int (*TpfnCreateVideoProcessor)(
			VideoProcessor::VIDEO_FORMAT InputVideoFormat,
			VideoProcessor::VIDEO_FORMAT RenderTargetFormat,
			IVideoProcessorEventListener *pEvenListener,
			void** ppVideoProcessor
		);

		TpfnCreateDXGIFactory pfnCreateDXGIFactory;
		TpfnDirect3DCreate9Ex pfnDirect3DCreate9Ex;
		TpfnDwmIsCompositionEnabled pfnDwmIsCompositionEnabled;
		TpfnDwmGetCompositionTimingInfo pfnDwmGetCompositionTimingInfo;
		TpfnDwmSetPresentParameters pfnDwmSetPresentParameters;
		TpfnDwmEnableComposition pfnDwmEnableComposition;
		TpfnDwmGetWindowAttribute pfnDwmGetWindowAttribute;
		TpfnDwmSetWindowAttribute pfnDwmSetWindowAttribute;
		TpfnDwmSetDxFrameDuration pfnDwmSetDxFrameDuration;
		TpfnDwmModifyPreviousDxFrameDuration pfnDwmModifyPreviousDxFrameDuration;
		TpIviFnDXVA2CreateVideoService pfnDXVA2CreateVideoService;
		PDXVAHD_CreateDevice pfnDXVAHD_CreateDevice;
		TpfnDXVA2CreateDirect3DDeviceManager9 pfnDXVA2CreateDirect3DDeviceManager9;
		TpfnMFCreateVideoSampleFromSurface pfnMFCreateVideoSampleFromSurface;
		TpfnMFCreateMediaType pfnMFCreateMediaType;
		TpfnDwmEnableMMCSS pfnDwmEnableMMCSS;
		TpfnCreateVideoProcessor pfnCreateVideoProcessor;

	public:
		CDynLibManager();
		~CDynLibManager();
		void LoadVideoEffect();

	protected:
		std::list<HMODULE> m_listLoadedLibraries;
	};

}

#endif	// _DISPSVR_DYN_LIB_