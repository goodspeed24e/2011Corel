#ifndef _DISPSVR_AMD_PCOM_MIXER_PRESENTER_H_
#define _DISPSVR_AMD_PCOM_MIXER_PRESENTER_H_

#include "Imports/ThirdParty/AMD/PCOM/amdpcom.h"
#include "Imports/ThirdParty/AMD/MCOM/amdmcom.h"
#include "D3D9VideoMixerBase.h"
#include "D3D9VideoPresenterBase.h"
#include "DriverExtensionHelper.h"

/////////////////////////////////////////////////////////////////////////////

namespace DispSvr
{
	class CAMDDeviceExtensionAdapter : public IDriverExtensionAdapter
	{
	public:
		virtual ~CAMDDeviceExtensionAdapter();
		static HRESULT GetAdapter(IDriverExtensionAdapter **ppDeviceCapAdapter);

		// IDriverExtensionAdapter
		virtual HRESULT SetDevice(IDirect3DDevice9 *pDevice9);
		virtual HRESULT QueryPresenterCaps(DWORD VideoDecodeCaps, PresenterCaps* pCaps);
		virtual HRESULT QueryHDMIStereoModeCaps(HWND hWnd, DriverExtHDMIStereoModeCap **ppCaps, UINT *puCount);
		virtual HRESULT EnableHDMIStereoMode(BOOL bEnable, DriverExtHDMIStereoModeCap *pCap, BOOL *pbReCreateDevice);
		virtual HRESULT QueryAdapterInfo(HWND hWnd, HMONITOR hMonitor, DispSvr::DrvExtAdapterInfo *pInfo);

	private:
		CAMDDeviceExtensionAdapter();
		CAMDDeviceExtensionAdapter(IDirect3DDevice9 *pDevice);        
		HRESULT CreateMCOM(HWND hwnd);
		HRESULT DestroyMCOM();
		HRESULT QueryDecodeStreamCaps();
		HRESULT GetPCOMCaps(HWND hwnd);
		HRESULT CreateFakeD3DDevice(HWND hwnd, IDirect3DDevice9 **pDevice);
	private:        
		IDirect3DDevice9 *m_pDevice9;
		HWND m_hWnd;
        HMONITOR m_hMonitor;
		// MCOM session handle
		void* m_pMcomSession;
		MCOM_DECODE_STREAM_CAPS m_DecodeStreamCaps;
		// PCOM Caps
		PCOM_GET_CAPS_OUTPUT m_PCOMCapsOutput;
		DrvExtAdapterInfo m_AdapterInfo;
	};

	class CAMDPCOMMixerPresenter :
		public virtual CD3D9VideoPresenterBase,
		public virtual CD3D9VideoMixerBase
	{
	public:
		CAMDPCOMMixerPresenter();
		virtual ~CAMDPCOMMixerPresenter();

		// IUnkonwn
		STDMETHOD(QueryInterface)(REFIID riid, void **ppv);
		STDMETHOD_(ULONG, AddRef)() { return CD3D9PluginBase::AddRef();	}
		STDMETHOD_(ULONG, Release)() { return CD3D9PluginBase::Release(); }

		// IDispSvrPlugin
		STDMETHOD(ProcessMessage)(RESOURCE_MESSAGE_TYPE msg, LPVOID ulParam);

		// CD3D9VideoPresenterBase
		STDMETHOD(SetDisplayRect)(const RECT *rcDst, const RECT *rcSrc);
		STDMETHOD(Present)(const PresentHints *pHints);
		STDMETHOD(Clear)();
		STDMETHOD(SetColorKey)(const DWORD dwColorKey);
		STDMETHOD(SetGamutMetadata)(const DWORD dwFormat, void *pGamutMetadata);
		STDMETHOD(SetScreenCaptureDefense)(BOOL bEnable);

	private:

		// CD3D9VideoMixerBase & CD3D9VideoPresenterBase
        STDMETHOD(SetProperty)(const MixerProperty *pProperty);
		STDMETHOD(_Execute)(IDirect3DSurface9 *pDestSurface, const RECT &rcDst, const RECT &rcDstClip);
		STDMETHOD(_SetDevice)(IUnknown *pDevice);
		STDMETHOD(_ReleaseDevice)();
		STDMETHOD(_QueryPlaneCaps)(PLANE_ID PlaneID, PLANE_FORMAT Format, PlaneCaps *pCap);

		HRESULT CreatePCOM();
		HRESULT DestroyPCOM();
		HRESULT BeginFramePCOM(const RECT &rcDst);
		HRESULT ExecutePCOM(PPCOM_EXECUTE_INPUT pExecuteIn);
		HRESULT	EndFramePCOM();
		HRESULT PresentPCOM();
		HRESULT ClearPCOM();
		HRESULT PlaneToPCOMPlane(PLANE_ID id, PPCOM_EXECUTE_INPUT pExecuteIn, RECT &rcDst, const RECT &rcDstClip, const float fWindowAspectRatio);
		HRESULT PresentEx();
		HRESULT InsertPCOMVP();
        HRESULT CalculatePCOMRect(UINT uViewID, PCOM_RECT &pcomrcSrc, PCOM_RECT &pcomrcDst, const D3D9Plane &plane, RECT &rcSrc, const RECT &rcMixingDst, const RECT &rcDstClip, float fWindowAspectRatio);

	private:
		enum D3D9_PRESENT_MODE
		{
			D3D9_PRESENT_MODE_WINDOWED   = 0,
			D3D9_PRESENT_MODE_FULLSCREEN = 1,
			D3D9_PRESENT_MODE_FLIPEX     = 2
		} m_ePresentMode;

		/* SwapEffects */
		typedef enum _D3DSWAPEFFECT
		{
			D3DSWAPEFFECT_DISCARD           = 1,
			D3DSWAPEFFECT_FLIP              = 2,
			D3DSWAPEFFECT_COPY              = 3,
			D3DSWAPEFFECT_OVERLAY           = 4,
			D3DSWAPEFFECT_FLIPEX            = 5,

			D3DSWAPEFFECT_FORCE_DWORD       = 0x7fffffff
		} D3DSWAPEFFECT;

		IDirect3DSwapChain9 *m_pSwapChain;
		DWORD m_dwFlipChainSize;
		DWORD m_dwQueuedFrameCount;
		bool m_bFullScreen;

		// PCOM session handle
		PCOM_PTR m_pSession;
		XVYCC_GAMUT_METADATA m_XvYCCGamutMetaData;
		// PCOM clear rectangle list
		DWORD m_dwPCOMClearRecCount;
		PCOM_CLEAR_RECTANGLE m_PCOMClearRecList[PCOM_MAX_CLEAR_RECTANGLES_COUNT];
		bool m_bSpatialDeinterlacing;
		bool m_bTemporalDeinterlacing;
		bool m_bAsyncPresent;
		DWORD m_iRenderTargetIndex;
	};
}

#endif //_AMDPCOMMIXERRENDER_H_