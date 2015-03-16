#ifndef _DISPSVR_RESOURCE_MANAGER_H_
#define _DISPSVR_RESOURCE_MANAGER_H_

#include <list>
#include "Singleton.h"
#include "Exports/Inc/VideoMixer.h"
#include "Exports/Inc/EngineProperty.h"

// video effect manager group
DEFINE_GUID(DISPSVR_RESOURCE_VIDEOEFFECT3DMANAGER, 0x45153063, 0xc925, 0x451c, 0xb9, 0xbb, 0x13, 0x83, 0x2, 0xb9, 0xfe, 0x37);
// X Video Effect Manager
DEFINE_GUID(DISPSVR_RESOURCE_XVIDEOEFFECTMANAGER, 0x4ef12394, 0xf97c, 0x4942, 0x87, 0x33, 0xa4, 0x47, 0x19, 0x3a, 0x39, 0x1c);

enum RESOURCE_MESSAGE_TYPE
{
	/// Set device pointer in the ulParam.
	/// ulParam: IUnknown, the device pointer.
	RESOURCE_MESSAGE_SETDEVICE,

	/// Release all reference count in the plugin.
	/// ulParam is not used and should be zero.
	RESOURCE_MESSAGE_RELEASEDEVICE,

	/// Set window handle in the ulParam.
	/// ulParam: HANDLE, the window handle.
	RESOURCE_MESSAGE_SETWINDOWHANDLE,

	/// Evict all resources on default pool and restore it later.
	/// and restore it when another paired message bEvict is FALSE.
	/// ulParam: BOOL *, bEvict
	RESOURCE_MESSAGE_EVICTRESOURCES,

	/// Notify presenter that video window receives WM_PAINT message
	/// ulParam: Reserved.
	RESOURCE_MESSAGE_UPDATEWINDOW,

	/// Notify presenter that video window receives WM_MOVE/WM_MOVING message
	/// ulParam: Reserved.
	RESOURCE_MESSAGE_MOVEWINDOW,

	/// Notify presenter that video window is minimized
	/// Indicates that window was restored with (*ulParam) = FALSE
	/// ulParam: a BOOL pointer for switching show/hide
	RESOURCE_MESSAGE_HIDEWINDOW,

    /// Notify all resources that RenderEngine is teminated.
    /// ulParam: Reserved.
    RESOURCE_MESSAGE_TERMINATE,

    /// Notify all resources that RenderEngine is teminated.
    /// ulParam: RESOURCE_WINDOW_STATE *, pointer to type RESOURCE_WINDOW_STATE.
	RESOURCE_MESSAGE_SETWINDOWSTATE
};

enum RESOURCE_WINDOW_STATE
{
	RESOURCE_WINDOW_STATE_WINDOWED,
	RESOURCE_WINDOW_STATE_MINIMIZED,
	RESOURCE_WINDOW_STATE_FULLSCREEN
};

MIDL_INTERFACE("3CD08F98-9432-436d-95E6-0D24605BAE21") IDispSvrPlugin : public IUnknown
{
	STDMETHOD(GetResourceId)(GUID *pGUID) = 0;
	STDMETHOD(ProcessMessage)(RESOURCE_MESSAGE_TYPE msg, LPVOID ulParam) = 0;
};

namespace DispSvr
{

	enum RESOURCE_TYPE
	{
		RESOURCE_TYPE_UNKNOWN = 0,
		RESOURCE_TYPE_VIDEOMIXER = 1 << 0,
		RESOURCE_TYPE_VIDEOPRESENTER = 1 << 1,
		RESOURCE_TYPE_VIDEOEFFECT3DMANAGER = 1 << 2,
        RESOURCE_TYPE_XVIDEOEFFECTMANAGER = 1 << 3,
	};

	class CResourceManager :
		public Singleton<CResourceManager>
	{
	public:
		CResourceManager();
		~CResourceManager();

		HRESULT GetInterface(REFGUID guidResourceID, REFIID riid, void **ppv);
		HRESULT ProcessMessage(RESOURCE_MESSAGE_TYPE msg, LPVOID ulParam);
		HRESULT Install(IDispSvrPlugin *pPlugin);
		HRESULT Uninstall(IDispSvrPlugin *pPlugin);
		HRESULT SetPreferredResource(REFGUID guidResourceID, RESOURCE_TYPE type);

	private:
		struct Plugin
		{
			IDispSvrPlugin *pInstance;
			GUID guidGroup;
			DWORD dwType;
		};
		typedef std::list<Plugin> PluginList;

	private:
		void LoadStaticPlugins();
		HRESULT ActivatePlugin(LPVOID ulParam);
		void DeactivePlugin(const Plugin *pPlugin);
		BOOL IsSuitableResource(const GUID *pGuid, DWORD dwVendorID);
		HRESULT ControlDwmComposition();
		HRESULT SetDwmComposition(BOOL bEnable);

	private:
		CCritSec m_csObj;
		PluginList m_listResourcePlugin;

		const Plugin *m_pActiveVideoMixer;
		const Plugin *m_pActiveVideoPresenter;
		const Plugin *m_pVideoEffect3DManager;
        const Plugin *m_pXVideoEffectManager;
		GUID m_guidPreferredVideoMixer;
		GUID m_guidPreferredVideoPresenter;

		bool m_bDwmCompositionChanged;
		bool m_bDwmCompositionStatus;
	};
}

#endif	// _DISPSVR_RESOURCE_MANAGER_H_