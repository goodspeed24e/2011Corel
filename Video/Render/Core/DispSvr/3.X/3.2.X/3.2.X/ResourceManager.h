#ifndef _DISPSVR_RESOURCE_MANAGER_H_
#define _DISPSVR_RESOURCE_MANAGER_H_

#include <list>
#include <map>
#include "Singleton.h"
#include "Exports/Inc/VideoMixer.h"
#include "Exports/Inc/EngineProperty.h"

// video mixer group: DISPSVR_RESOURCE_VIDEOMIXER
// video presenter group: DISPSVR_RESOURCE_VIDEOPRESENTER
// video effect manager group
DEFINE_GUID(DISPSVR_RESOURCE_VIDEOEFFECT3DMANAGER, 0x45153063, 0xc925, 0x451c, 0xb9, 0xbb, 0x13, 0x83, 0x2, 0xb9, 0xfe, 0x37);
// X Video Effect Manager
DEFINE_GUID(DISPSVR_RESOURCE_XVIDEOEFFECTMANAGER, 0x4ef12394, 0xf97c, 0x4942, 0x87, 0x33, 0xa4, 0x47, 0x19, 0x3a, 0x39, 0x1c);
// {C155CAFE-FBA0-44b2-9FC6-FC2FC1496653}
DEFINE_GUID(DISPSVR_RESOURCE_TEXTUREPOOL, 
0xc155cafe, 0xfba0, 0x44b2, 0x9f, 0xc6, 0xfc, 0x2f, 0xc1, 0x49, 0x66, 0x53);
// {C0577478-CA74-4ee8-9D32-C0C59D80129C}
DEFINE_GUID(DISPSVR_RESOURCE_DRIVEREXTENSION, 
0xc0577478, 0xca74, 0x4ee8, 0x9d, 0x32, 0xc0, 0xc5, 0x9d, 0x80, 0x12, 0x9c);
// {AD99760C-8399-445c-8AD8-27B7CBD75BE9}
DEFINE_GUID(DISPSVR_RESOURCE_VIDEOMIXERMODEL, 
0xad99760c, 0x8399, 0x445c, 0x8a, 0xd8, 0x27, 0xb7, 0xcb, 0xd7, 0x5b, 0xe9);


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

	/// Notify all resources that the state of window.
	/// ulParam: RESOURCE_WINDOW_STATE *, pointer to type RESOURCE_WINDOW_STATE.
	RESOURCE_MESSAGE_SETWINDOWSTATE,

	/// Notify all resources that the device has been reset.
	RESOURCE_MESSAGE_RESETDEVICE,
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

const int _BACK_BUFFER_COUNT_ = 3;
const int _FRONT_BUFFER_COUNT_ = 1;

namespace DispSvr
{
    class CPersistentInterfaceProxy;

	class CResourceManager :
		public Singleton<CResourceManager>
	{
	public:
		typedef bool (*EnvPredicate) (DWORD dwVendorID, DWORD dwDeviceID);
		CResourceManager();
		~CResourceManager();

		HRESULT GetInterface(REFGUID guidGroupId, REFIID riid, void **ppv);
		HRESULT ProcessMessage(RESOURCE_MESSAGE_TYPE msg, LPVOID ulParam);
		HRESULT Install(IDispSvrPlugin *pPlugin, EnvPredicate pPredicate = NULL);
		HRESULT Uninstall(IDispSvrPlugin *pPlugin);
		HRESULT SetPreferredResource(REFGUID guidResourceID, REFGUID guidGroupId);
        HRESULT GetActiveResrouceGUID(REFGUID guidGroupId, GUID *pGUID);

	private:
		struct Plugin
		{
			IDispSvrPlugin *pInstance;
			GUID guidId;					//< the unique ID for the plugin
			DWORD dwTypeMask;
			EnvPredicate InRequiredEnv;
		};
		typedef std::list<Plugin> PluginList;

		struct ResourceGroup
		{
			const Plugin *pPlugin;
			GUID guidPreferredPlugin;		//< the preferred resource ID.
			DWORD dwTypeMask;
			IID iidRequiredInterface;       //< if exposing required interface, we put the resource to this group.
			bool bMandatory;				//< resource manager must have an available active plugin in this group.
            CPersistentInterfaceProxy *pInterfaceProxy;
		};
		typedef std::map<GUID, ResourceGroup> ResourceGroupMap;

	private:
		void LoadStaticPlugins();
		HRESULT ActivatePlugin(LPVOID ulParam, ResourceGroup &group);
		HRESULT ActivatePluginWithoutDevice(REFGUID guidGroupId);
		HRESULT ActivateMixerPresenter(LPVOID ulParam);
		void DeactivatePlugin(const Plugin *pPlugin);
		HRESULT ControlDwmComposition();
		HRESULT SetDwmComposition(BOOL bEnable);

	private:
		CCritSec m_csObj;
		PluginList m_listResourcePlugin;
		ResourceGroupMap m_mapResourceGroup;
        CPersistentInterfaceProxy *m_pInterfaceProxy;

		bool m_bDwmCompositionChanged;
		bool m_bDwmCompositionStatus;
	};
}

#endif	// _DISPSVR_RESOURCE_MANAGER_H_