#ifndef _DISPSVR_DRIVER_EXTENSION_PLUGIN_H__
#define _DISPSVR_DRIVER_EXTENSION_PLUGIN_H__

#include "D3D9PluginBase.h"
#include "DriverExtensionHelper.h"


namespace DispSvr
{
	class CD3D9DriverExtensionPlugin : public CD3D9PluginBase, public IDispSvrDriverExtension
	{
	public:
		CD3D9DriverExtensionPlugin();
		virtual ~CD3D9DriverExtensionPlugin();

		// IUnknown
		STDMETHOD(QueryInterface)(REFIID riid, void **ppv);
        STDMETHOD_(ULONG, AddRef)() { return CD3D9PluginBase::AddRef(); }
        STDMETHOD_(ULONG, Release)() { return CD3D9PluginBase::Release(); }

		// IDispSvrDriverExtension
		STDMETHOD(QueryPresenterCaps)(DWORD VideoDecodeCaps, DispSvr::PresenterCaps* pCaps);
		STDMETHOD(SetStereoInfo)(IDirect3DSurface9 *pBaseView, IDirect3DSurface9 *pDependentView, INT iOffset, BOOL bStereoEnable, MIXER_STEREO_MODE stereoMixingMode);
		STDMETHOD(QueryContentProtectionCaps)(DriverExtContentProtectionCaps *pCaps);
        STDMETHOD(QueryHDMIStereoModeCaps)(HWND hWnd, DispSvr::DriverExtHDMIStereoModeCap **ppCaps, UINT *puCount);
        STDMETHOD(EnableHDMIStereoMode)( BOOL bEnable, DispSvr::DriverExtHDMIStereoModeCap *pCap, BOOL *pbReCreateDevice);
        STDMETHOD(QueryAdapterInfo)(HWND hWnd, HMONITOR hMonitor, DispSvr::DrvExtAdapterInfo *pInfo);
        STDMETHOD(Clear)();

	protected:
		STDMETHOD(_SetDevice)(IUnknown *pDevice);
		STDMETHOD(_ReleaseDevice)();
        HRESULT CheckAdapter();

	protected:
		IDriverExtensionAdapter *m_pAdapter;
	};
}
#endif // _DISPSVR_DRIVER_EXTENSION_PLUGIN_H__