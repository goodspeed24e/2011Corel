#ifndef __DISPSVR_INTEL_FAST_COMPOSITING_VIDEO_MIXER_H__
#define __DISPSVR_INTEL_FAST_COMPOSITING_VIDEO_MIXER_H__

#include "IntelDxva2Device.h"
#include "D3D9VideoMixer.h"
#include "D3D9VideoPresenter.h"

#define BASE_VIDEO_MIXER	CD3D9VideoMixer	//CD3D9Dxva2VideoMixer

namespace DispSvr
{
	class CIntelFastCompositingMixer :
		public virtual BASE_VIDEO_MIXER,
		public virtual CD3D9VideoPresenter
	{
	public:
		CIntelFastCompositingMixer();
		virtual ~CIntelFastCompositingMixer();

		// IUnkonwn
		STDMETHOD(QueryInterface)(REFIID riid, void **ppv);
		STDMETHOD_(ULONG, AddRef)() { return CD3D9PluginBase::AddRef();	}
		STDMETHOD_(ULONG, Release)() { return CD3D9PluginBase::Release(); }

		// IDispSvrPlugin
		STDMETHOD(ProcessMessage)(RESOURCE_MESSAGE_TYPE msg, LPVOID ulParam);

		// CD3D9VideoPresenterBase
		STDMETHOD(SetDisplayRect)(const RECT *rcDst, const RECT *rcSrc);
		STDMETHOD(Present)(const PresentHints *pHints);
		STDMETHOD(SetScreenCaptureDefense)(BOOL bEnable);
		STDMETHOD(SetBackgroundColor)(COLORREF Color);
		STDMETHOD(SetGamutMetadata)(const DWORD dwFormat, void *pGamutMetadata);

	private:
		// CD3D9VideoMixerBase
		STDMETHOD(_QueryPlaneCaps)(PLANE_ID PlaneID, PLANE_FORMAT Format, PlaneCaps *pCap);
		STDMETHOD(_CreatePlane)(DispSvr::PlaneInit *pInit);
		STDMETHOD(_Execute)(IDirect3DSurface9 *pDestSurface, const RECT &rcDst, const RECT &rcDstClip);
		STDMETHOD(_SetDevice)(IUnknown *pDevice);
		STDMETHOD(_ReleaseDevice)();
		STDMETHOD(_QueryCaps)(PresenterCaps* pCaps);

		HRESULT ExecuteFastCompositing(IDirect3DSurface9 *pDestSurface, const RECT &rcDst, const RECT &rcDstClip);
		HRESULT ExecuteD3DCompositing(IDirect3DSurface9 *pDestSurface, const RECT &rcDst, const RECT &rcDstClip);
		HRESULT QueryCaps(FASTCOMP_CAPS **ppCaps);
		HRESULT QueryFormats(FASTCOMP_SAMPLE_FORMATS **ppFormats);

		HRESULT FastCompositingBlt(const FASTCOMP_BLT_PARAMS &blt);
		HRESULT PlaneToSample(PLANE_ID id, FASTCOMP_VideoSample &sample, const RECT &rcDst, const RECT &rcDstClip);
		HRESULT CheckAndUpdateClearRects(const RECT &rcDst, const RECT &rcDstClip);
		HRESULT UnscrambleSurface(IDirect3DSurface9 *pRT, IDirect3DSurface9 *pSurface, ContentProtection &protection);

	protected:
		FASTCOMP_ClearRect *m_pClearRect;
		UINT m_uClearRect;
		RECT m_rcClip;
		DXVA2_AYUVSample16 m_ayuv16BackgroundColor;

		CIntelSCDService *m_pSCDService;
		CIntelFastCompositingService *m_pFastCompService;
		CIntelRegistrationDevice *m_pRegistrationDevice;
		IDriverExtensionAdapter *m_pDeviceCap;
	};
}

#endif	// __DISPSVR_INTEL_FAST_COMPOSITING_VIDEO_MIXER_H__