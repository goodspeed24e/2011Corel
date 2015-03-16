#ifndef __DISPSVR_DRIVER_EXTENSION_HELPER_H__
#define __DISPSVR_DRIVER_EXTENSION_HELPER_H__


namespace DispSvr
{
	enum DRIVER_EXT_CP_TYPE
	{
		DRIVER_EXT_CP_PAVP = 1
	};

	struct DriverExtContentProtectionCaps
	{
		DRIVER_EXT_CP_TYPE eType;
		union
		{
			struct
			{
				BOOL bSupportAudio;
			} sPavpCaps;
			DWORD dwPadding[31];
		};
	};

	struct DrvExtAdapterInfo
	{
		BOOL bIsXVYCCSupported;
		DWORD dwSupportStereoFormats;  // STEREO_VISION_MODE_FLAG
	};

	enum HDMI_STEREO_MODE_FLAG
	{
		HDMI_STEREO_NONE                    = 0,
		HDMI_STEREO_FRAME_PACKING           = 1 << 0,
		HDMI_STEREO_FIELD_ALTERNATIVE       = 1 << 1,
		HDMI_STEREO_LINE_ALTERNATIVE        = 1 << 2,
		HDMI_STEREO_SIDE_BY_SIDE_FULL       = 1 << 3,
		HDMI_STEREO_L_DEPTH                 = 1 << 4,
		HDMI_STEREO_L_DEPTH_GFX             = 1 << 5,
		HDMI_STEREO_SIDE_BY_SIDE_HALF       = 1 << 6,
		HDMI_STEREO_TOP_BOTTOM              = 1 << 7, //doesn't exist in HDMI 1.4 spec.
		HDMI_STEREO_FRAME_PACKING_INT       = 1 << 8, // for nVIDIA
		HDMI_STEREO_NV_3D_VISION            = 1 << 9, // for nVIDIA
		HDMI_STEREO_ALL                     = 0xffffffff
	};

	enum HDMI_STEREO_MODE_EXT_FLAG // for Side-by-Side half ext data
	{
		//HDMI_STEREO_EXT_NONE                              = 0,
		HDMI_STEREO_EXT_HORIZONTAIL_ODD_LEFT_ODD_RIGHT      = 1 << 20,
		HDMI_STEREO_EXT_HORIZONTAIL_ODD_LEFT_EVEN_RIGHT     = 1 << 21,
		HDMI_STEREO_EXT_HORIZONTAIL_EVEN_LEFT_ODD_RIGHT     = 1 << 22,
		HDMI_STEREO_EXT_HORIZONTAIL_EVEN_LEFT_EVEN_RIGHT    = 1 << 23,

		HDMI_STEREO_EXT_QUINCUNX_ODD_LEFT_ODD_RIGHT      = 1 << 24,
		HDMI_STEREO_EXT_QUINCUNX_ODD_LEFT_EVEN_RIGHT     = 1 << 25,
		HDMI_STEREO_EXT_QUINCUNX_EVEN_LEFT_ODD_RIGHT     = 1 << 26,
		HDMI_STEREO_EXT_QUINCUNX_EVEN_LEFT_EVEN_RIGHT    = 1 << 27,
		//HDMI_STEREO_EXT_ALL                            = 0xffffffff
	};

    struct DriverExtHDMIStereoModeCap
    {
        UINT uWidth;
        UINT uHeight;
        UINT uRefreshRate;
        DWORD dwStereoMode; //mask of (HDMI_STEREO_MODE_FLAG) OR (HDMI_STEREO_MODE_EXT_FLAG)
        //        DWORD eStereoModeExt; // mask of HDMI_STEREO_MODE_EXT_FLAG
    };

	/// Adapter interface to convert given device to extension query interface.
	interface IDriverExtensionAdapter
	{
		virtual ~IDriverExtensionAdapter() { }
		virtual HRESULT QueryPresenterCaps(DWORD VideoDecodeCaps, PresenterCaps* pCaps) = 0;
		// optional interfaces
        virtual HRESULT SetDevice(IDirect3DDevice9 *pDevice9) { return E_NOTIMPL; }
		virtual HRESULT SetStereoInfo(IDirect3DSurface9 *pBaseView, IDirect3DSurface9 *pDependentView, INT iOffset, BOOL bStereoEnable, MIXER_STEREO_MODE stereoMixingMode) { return E_NOTIMPL; }
		virtual HRESULT QueryContentProtectionCaps(DriverExtContentProtectionCaps *pCaps) { return E_NOTIMPL; }
        virtual HRESULT QueryHDMIStereoModeCaps(HWND hWnd, DriverExtHDMIStereoModeCap **ppCaps, UINT *puCount) { return E_NOTIMPL; }
        virtual HRESULT EnableHDMIStereoMode(BOOL bEnable, DriverExtHDMIStereoModeCap *pCap, BOOL *pbReCreateDevice){ return E_NOTIMPL; }
        virtual HRESULT QueryAdapterInfo(HWND hWnd, HMONITOR hMonitor, DrvExtAdapterInfo *pInfo) { return E_NOTIMPL; }
        virtual HRESULT Clear() { return E_NOTIMPL; }
	};
}

// internal driver extension plugin
MIDL_INTERFACE("0E8E49AB-D028-400b-937F-ECEC989C2FB5") IDispSvrDriverExtension : public IUnknown
{
	STDMETHOD(QueryPresenterCaps)(DWORD VideoDecodeCaps, DispSvr::PresenterCaps* pCaps) = 0;
	STDMETHOD(SetStereoInfo)(IDirect3DSurface9 *pBaseView, IDirect3DSurface9 *pDependentView, INT iOffset, BOOL bStereoEnable, DispSvr::MIXER_STEREO_MODE stereoMixingMode) = 0;
	STDMETHOD(QueryContentProtectionCaps)(DispSvr::DriverExtContentProtectionCaps *pCaps) = 0;
    STDMETHOD(QueryHDMIStereoModeCaps)(HWND hWnd, DispSvr::DriverExtHDMIStereoModeCap **ppCaps, UINT *puCount) = 0;
    STDMETHOD(EnableHDMIStereoMode)(BOOL bEnable, DispSvr::DriverExtHDMIStereoModeCap *pCap, BOOL *pbReCreateDevice) = 0;
    STDMETHOD(QueryAdapterInfo)(HWND hWnd, HMONITOR hMonitor, DispSvr::DrvExtAdapterInfo *pInfo) = 0;
    STDMETHOD(Clear)() = 0;
};

#endif	// __DISPSVR_DRIVER_EXTENSION_HELPER_H__
