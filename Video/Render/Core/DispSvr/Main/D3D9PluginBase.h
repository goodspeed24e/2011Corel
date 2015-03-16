#ifndef _DISPSVR_D3D9PLUGINBASE_H_
#define _DISPSVR_D3D9PLUGINBASE_H_

namespace DispSvr
{
	/// A basic implementation for D3D9 plug-in.
	class DECLSPEC_NOVTABLE CD3D9PluginBase : public IDispSvrPlugin
	{
	public:
		// IDispSvrPlugin
		STDMETHOD(GetResourceId)(GUID *pGUID);
		STDMETHOD(ProcessMessage)(RESOURCE_MESSAGE_TYPE msg, LPVOID ulParam);

		// IUnknown
		STDMETHOD(QueryInterface)(REFIID riid, void **ppv);
		STDMETHOD_(ULONG, AddRef)();
		STDMETHOD_(ULONG, Release)();

	protected:
		CD3D9PluginBase();
		virtual ~CD3D9PluginBase();

		STDMETHOD(_SetDevice)(IUnknown *pDevice);
		STDMETHOD(_ReleaseDevice)();
		STDMETHOD(_SetWindow)(HWND hwnd);

	protected:
		GUID m_GUID;
		HWND m_hwnd;
		DWORD m_eWindowState;
		IDirect3DDevice9* m_pDevice;
		IDirect3DDevice9Ex* m_pDeviceEx;
		RECT m_rcMonitor;

	private:
		LONG m_cRef;
	};
}

#endif	// _DISPSVR_D3D9PLUGINBASE_H_