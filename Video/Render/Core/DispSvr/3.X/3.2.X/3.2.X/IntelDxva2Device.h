#ifndef __DISPSVR_INTEL_DXVA2_DEVICE_H_
#define __DISPSVR_INTEL_DXVA2_DEVICE_H_

#include "Imports/ThirdParty/Intel/FastComp/FastCompositingAPI.h"
#include "Imports/ThirdParty/Intel/SCD/ScreenCapDef.h"
#include "Imports/ThirdParty/Intel/SCD/SCD_Auth.h"
#include "Imports/ThirdParty/Intel/SCD/IntelPavpDeviceType.h"
#include "DriverExtensionHelper.h"

#ifndef COREL_APPLICATION_ID
#define COREL_APPLICATION_ID	COREL_TEST_APPLICATION_ID
#endif	// COREL_APPLICATION_ID

// FastCom Spec Revision 1.0
// The fast compositing device currently recommends rendering at 60 fps in the following configurations.
// 666MHz GPU core frequency: 1 to 3 full HD layers, full HD output
// 800MHz GPU core frequency: 1 to 4 full HD layers, full HD output
# define FASTCOM_MAX_INPUT_LAYERS    3
# define FASTCOM_MAX_SRC_WIDTH       1920
# define FASTCOM_MAX_SRC_HEIGHT      1080

enum
{
    INTEL_STREAM_FORMAT_MPEG2 = 1,
    INTEL_STREAM_FORMAT_VC1 = 2,
    INTEL_STREAM_FORMAT_H264 = 3,
};

interface ICUIExternal8;

namespace DispSvr
{
	class CIntelRegistrationDevice;

	/// One service must have one auxiliary device. Otherwise it may fail or BSOD.
	class CIntelAuxiliaryDevice
	{
	public:
		virtual ~CIntelAuxiliaryDevice();

		HRESULT CreateRegistrationDevice(CIntelRegistrationDevice **ppDevice);

		HRESULT Execute(UINT Function, PVOID pInput, UINT uSizeInput, PVOID pOutput, UINT uSizeOutput);

		/// Check if a GUID is supported by the auxiliary device.
		HRESULT HasAccelGuids(const GUID &guid, BOOL *pbHasIt);
		HRESULT QueryAccelGuids(GUID **ppAccelGuids, UINT *puAccelGuidCount);
		HRESULT QueryAccelRTFormats(CONST GUID *pAccelGuid, D3DFORMAT **ppFormats, UINT *puCount);
		HRESULT QueryAccelFormats(CONST GUID *pAccelGuid, D3DFORMAT **ppFormats, UINT *puCount);
		HRESULT QueryAccelCaps(CONST GUID *pAccelGuid, void *pCaps, UINT *puCapSize);

	protected:
        CIntelAuxiliaryDevice();
		CIntelAuxiliaryDevice(IDirect3DDevice9 *pDevice);
		HRESULT CreateAuxiliaryDevice();

	protected:
		DXVA2_VideoDesc m_VideoDesc;
		IDirect3DDevice9 *m_pDevice;
		IDirect3DSurface9 *m_pRenderTarget;	//< the render target for DXVA2 video decoder.
		IDirectXVideoDecoder *m_pAuxiliaryDevice;
	};


	/// Implementation of fast compositing interface through DXVA2 video decoder.
	class CIntelFastCompositingService : public CIntelAuxiliaryDevice
	{
	public:
		static HRESULT Create(IDirect3DDevice9 *pD3D9Device, CIntelFastCompositingService **ppService);
		virtual ~CIntelFastCompositingService();

		int GetNumBackwardSamples() const { return m_iNumBackwardSamples; }
		int GetNumForwardSamples() const { return m_iNumForwardSamples; }
		HANDLE GetRegistrationHandle() const { return m_hRegistration; }
		/// If the destination rectangle is larger than max width/height, the driver returns failure.
		/// In order to at least show the video, we center the output to comply with the restriction.
		void ApplyTargetRectRestriction(RECT &rcDst);
		HRESULT FastCompositingBlt(const FASTCOMP_BLT_PARAMS &blt);
		int GetMaxFlipRate() const { return m_dwMaxFlipRate; }
		bool CanSupportExtendedGamut() const { return m_bCanSupportExtendedGamut; }

	protected:
		CIntelFastCompositingService(IDirect3DDevice9 *pD3D9Device);
		HRESULT Init();
		HRESULT QueryFormats(FASTCOMP_SAMPLE_FORMATS **ppFormats);
		HRESULT QueryFrameRate(int iMaxLayers, int iMaxSrcWidth, int iMaxSrcHeight, int iMaxDstWidth, int iMaxDstHeight, DWORD &dwFrameRate);

	protected:
		HANDLE m_hRegistration;
		bool m_bCreated;
		int m_iMaxDstWidth;
		int m_iMaxDstHeight;
		int m_iNumBackwardSamples;
		int m_iNumForwardSamples;
		int m_dwMaxFlipRate;
		bool m_bCanSupportExtendedGamut;
	};

	/// Implementation of screen capture defense and additional security (extension).
	///
	/// If PAVP heavy mode, PAVP automatically enables SCD and replaces certain functions
	/// in SCD device.
	class CIntelSCDService : public CIntelAuxiliaryDevice
	{
	public:
		static HRESULT Create(IDirect3DDevice9 *pD3D9Device, CIntelSCDService **ppService);
		virtual ~CIntelSCDService();

		HANDLE GetRegistrationHandle() const { return m_hRegistration; }
		bool IsSCDOverlay() const { return m_bCompositionEnabled != TRUE; }
		bool IsSCDExtension() const { return m_hRegistration != INVALID_HANDLE_VALUE; }
		bool IsPAVPHeavy() const { return !m_bPavpLiteMode; }
		HRESULT SetWindowPosition(const RECT &rWindowPosition, const RECT &rVisibleContent, HDC hdcMonitorId);
		HRESULT UnscrambleSurface(void *pSurface, DWORD dwSizeScrambled, DWORD dwScrambleSeed);

	protected:
		CIntelSCDService(IDirect3DDevice9 *pD3D9Device);
		HRESULT Init();

	protected:
		HANDLE m_hRegistration;
		bool m_bCreated;
		BOOL m_bCompositionEnabled;
		bool m_bNeedEnableComposition;
		bool m_bPavpLiteMode;
	};


	#define MAX_REGISTRATION_SAMPLES	10
	/// Registration device is used to map IDirect3DSurface9 and surface handles between the app and driver.
	class CIntelRegistrationDevice
	{
	friend class CIntelAuxiliaryDevice;
	public:
		~CIntelRegistrationDevice();
		HRESULT RegisterSurfaces(const DXVA2_SURFACE_REGISTRATION *Reg);

	protected:
		CIntelRegistrationDevice();

	protected:
		IDirectXVideoProcessor *m_pRegistrationDevice;
		DXVA2_VideoProcessBltParams m_BltParams;
		DXVA2_VideoSample m_Samples[MAX_REGISTRATION_SAMPLES];
	};

	/// CIntelDxva2DeviceCapAdapter is used for querying only without actual creating fast compositing or other services.
	class CIntelDxva2DriverExtAdapter : public IDriverExtensionAdapter, public CIntelAuxiliaryDevice
	{
	public:
		virtual ~CIntelDxva2DriverExtAdapter();

    	static HRESULT GetAdapter(IDriverExtensionAdapter **ppDeviceCapAdapter);

		// IDriverExtensionAdapter
        virtual HRESULT SetDevice(IDirect3DDevice9 *pDevice9);
		virtual HRESULT QueryPresenterCaps(DWORD VideoDecodeCaps, PresenterCaps* pCaps);
		virtual HRESULT QueryContentProtectionCaps(DriverExtContentProtectionCaps *pCaps);
        virtual HRESULT QueryHDMIStereoModeCaps(HWND hWnd, DriverExtHDMIStereoModeCap **ppCaps, UINT *puCount);
        virtual HRESULT EnableHDMIStereoMode(BOOL bEnable, DriverExtHDMIStereoModeCap *pCap, BOOL *pbReCreateDevice);
		virtual HRESULT QueryAdapterInfo(HWND hWnd, HMONITOR hMonitor, DispSvr::DrvExtAdapterInfo *pInfo);

    protected:
        virtual HRESULT GetPrimaryMonitorUID(ICUIExternal8 *pCUI, DWORD *uidMonitorReq);

	private:
		CIntelDxva2DriverExtAdapter(IDirect3DDevice9 *pDevice);
        CIntelDxva2DriverExtAdapter();

	private:
		DWORD m_dwMainVideoDecodeCaps;
	};

    class CIntelAuxiliaryDeviceService : public CIntelAuxiliaryDevice
    {
    public:
        static HRESULT Create(IDirect3DDevice9 *pD3D9Device, CIntelAuxiliaryDeviceService **ppService);
        virtual ~CIntelAuxiliaryDeviceService();
    protected:
        CIntelAuxiliaryDeviceService(IDirect3DDevice9 *pD3D9Device);

    protected:
        bool m_bCreated;
    };
}

#endif	// __DISPSVR_INTEL_DXVA2_DEVICE_H_
