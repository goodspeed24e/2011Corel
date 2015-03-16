#ifndef _DISPSVR_VIDEO_EFFECT_3D_H_
#define _DISPSVR_VIDEO_EFFECT_3D_H_

MIDL_INTERFACE("2B6D902E-A17E-41d9-B5F7-C7FEBD6F3C03") IDispSvrVideoEffect3DPlugin;
MIDL_INTERFACE("934F6239-8416-4ccd-A8A7-788EA0F5E103") IDispSvrVideoEffect3DManager;

//Default Video Effect GUIDs
// {F76491A9-57AA-4400-AA5F-B83B6E1EBC6D}
DEFINE_GUID(DispSvr_VideoEffectDownsampling, 
            0xf76491a9, 0x57aa, 0x4400, 0xaa, 0x5f, 0xb8, 0x3b, 0x6e, 0x1e, 0xbc, 0x6d);


namespace DispSvr
{
	enum VE3D_MESSAGE_TYPE
	{
		/// Called when plugin is installed by the effect manager.
		/// ulParam: IDispSvrVideoEffect3DManager, the effect manager pointer.
		VE3D_MESSAGE_TYPE_INITIALIZE = 0,

		/// Set device pointer in the ulParam.
		/// ulParam: IUnknown, the device pointer.
		VE3D_MESSAGE_TYPE_SETDEVICE = 1,

		/// The plugin should release device and resource.
		/// ulParam is not used and should be zero.
		VE3D_MESSAGE_TYPE_RELEASEDEVICE = 2,

		/// The plugin is enabled or disabled.
		/// ulParam: BOOL *, pointer to bEnable.
		VE3D_MESSAGE_TYPE_ENABLE = 3
	};

	enum EFFECT_MANAGER_TYPE
	{
		EFFECT_MANAGER_TYPE_UNKNOWN			= 0,
		EFFECT_MANAGER_TYPE_SHADER_BASED	= 1
	};

	enum EFFECT_BUFFER_CAPS
	{
		EFFECT_BUFFER_CAPS_TEXTURE		= 1 << 0,		//< surface is a texture
		EFFECT_BUFFER_CAPS_LOCKABLE		= 1 << 1		//< surface should be lockable
	};

	struct VE3DBuffer
	{
		IUnknown *pSurface;		//< pSurface should point to a valid surface or null on errors.
		UINT uWidth;			//< width of the buffer
		UINT uHeight;			//< height of the buffer
		RECT rcSource;			//< source rectangle
		RECT rcTarget;			//< hint of the target rectangle
		LONGLONG rtStart;
		LONGLONG rtEnd;
		UINT bInterlaced : 1;
		UINT bBottomFieldFirst : 1;
		UINT bSelectSecondField : 1;
		UINT dwReserved : 29;
	};

	struct VE3DManagerCaps
	{
		EFFECT_MANAGER_TYPE eType;		/// see EFFECT_MANAGER_TYPE
		UINT uInputBufferCaps;			/// see EFFECT_BUFFER_CAPS
		UINT uOutputBufferCaps;			/// see EFFECT_BUFFER_CAPS
		DWORD dwReserved[13];
	};

	struct VE3DManagerProperty
	{
		DWORD dwReserved[16];
	};
}

/// IDispSvrVideoEffect3DPlugin is implemented by plugin.
interface IDispSvrVideoEffect3DPlugin : IUnknown
{
	/// Process message from effect manager.
	STDMETHOD(ProcessMessage)(DispSvr::VE3D_MESSAGE_TYPE eMessage, LPVOID ulParam);

	/// Get the plugin's GUID to identify itself.
	STDMETHOD(GetResourceId)(GUID *pGUID) = 0;

	/// Set one parameter to the plugin. The parameters can be set are defined by the plugin.
	STDMETHOD(SetParam)(DWORD dwParam, DWORD dwValue) = 0;

	/// Get one parameter from the plugin. The parameters can be set are defined by the plugin.
	STDMETHOD(GetParam)(DWORD dwParam, DWORD *pdwValue) = 0;

	/// ProcessEffect is called when the plugin is enabled by effect manager.
	/// Plugin should respond the call by setting pOutput buffer with a valid surface or
	/// failing the call to break the processing pipeline.
	STDMETHOD(ProcessEffect)(DispSvr::VE3DBuffer *pInput, DispSvr::VE3DBuffer *pOutput) = 0;
};

/// IDispSvrVideoEffect3DManager is exposed by IDispSvrVideoMixer instance.
interface IDispSvrVideoEffect3DManager : IUnknown
{
	/// Install a plugin to effect manager.
	STDMETHOD(Install)(IDispSvrVideoEffect3DPlugin *pPlugin) = 0;

	/// Uninstall a plugin from effect manager.
	STDMETHOD(Uninstall)(IDispSvrVideoEffect3DPlugin *pPlugin) = 0;

	/// Enable or disable a plugin to process video effect.
	STDMETHOD(Enable)(const IDispSvrVideoEffect3DPlugin *pPlugin, BOOL bEnable) = 0;

	/// Check if a plugin is currently being enabled.
	/// @return TRUE if a plugin is enabled.
	STDMETHOD(IsEnabled)(const IDispSvrVideoEffect3DPlugin *pPlugin, BOOL *pbEnabled) = 0;

	/// Get current installed effect count.
	STDMETHOD(GetEffectCount)(UINT *puCount) = 0;

	/// Get a plugin interface from installed effect at indexed location.
	STDMETHOD(GetEffectAt)(UINT uIndex, IDispSvrVideoEffect3DPlugin **ppPlugin) = 0;

	/// Get capabilities about effect manager and its input/output buffer requirements.
	STDMETHOD(GetCaps)(DispSvr::VE3DManagerCaps *pCaps) = 0;

	/// Set effect manager property.
	STDMETHOD(SetProperty)(const DispSvr::VE3DManagerProperty *pProperty) = 0;

	/// Get property from effect manger.
	STDMETHOD(GetProperty)(DispSvr::VE3DManagerProperty *pProperty) = 0;

	/// Get array of available effect guids and count from effect manager
	/// The caller must release memory for the array by calling CoTaskMemFree
	STDMETHOD(GetAvailableEffectGUID)(UINT *puCount, GUID **ppAvailabeEffectGUID) = 0;

	/// Get a plugin interface from installed effect by GUID.
	STDMETHOD(GetEffectByGUID)(const GUID *pGUID, IDispSvrVideoEffect3DPlugin **ppPlugin) = 0;
};

typedef int (__cdecl *TpfnCreateVideoEffect3DPlugin)(IDispSvrVideoEffect3DPlugin **ppPlugin);

#ifdef __cplusplus
extern "C" {
#endif

/// the prototype for implementing video effect plugin loadable by DispSvr.
__declspec(dllexport) int __cdecl CreatePlugin(IDispSvrVideoEffect3DPlugin **ppPlugin);

#ifdef __cplusplus
}	// extern "C"
#endif

#endif	// _DISPSVR_VIDEO_EFFECT_3D_H_