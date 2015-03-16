#ifndef _DISPSVR_NVAPI_DXVA2HD_MIXER_RENDER_H_
#define _DISPSVR_NVAPI_DXVA2HD_MIXER_RENDER_H_

#include "NvAPIPresenter.h"
#include "D3D9Dxva2HDVideoMixer.h"

// ******************************************************************
// 1. CNvDxva2HDMixerRender would be used based on following criteria.
//    - DXVAHD supported VGA driver
//    - playback on xvYcc supported HDMI monitor.
//    Otherwise, we would fall back to use DXVA2 RGB overlay
// 2. While playing a xvYcc content, App have to make sure the HDMI 
//    output format is YCbCr.
// ******************************************************************

namespace DispSvr
{
	class CNvDxva2HDMixerRender :
		public virtual CNvAPIPresenter,
		public virtual CD3D9Dxva2HDVideoMixer
	{
	public:
		CNvDxva2HDMixerRender();
		virtual ~CNvDxva2HDMixerRender();

		// IUnkonwn
		STDMETHOD(QueryInterface)(REFIID riid, void **ppv);
		STDMETHOD_(ULONG, AddRef)() { return CD3D9PluginBase::AddRef();	}
		STDMETHOD_(ULONG, Release)() { return CD3D9PluginBase::Release(); }

		// IDispSvrPlugin
		STDMETHOD(ProcessMessage)(RESOURCE_MESSAGE_TYPE msg, LPVOID ulParam);

		// IDispSvrPrivateRender
		STDMETHOD(BeginRender)();

	protected:
		STDMETHOD(_Execute)(IDirect3DSurface9 *pDestSurface, const RECT &rcDst, const RECT &rcDstClip);
		STDMETHOD(_SetDevice)(IUnknown *pDevice);
		STDMETHOD(_ReleaseDevice)();
		virtual HRESULT CreateOverlay(D3DSURFACE_DESC *desc, NV_DX_CREATE_VIDEO_PARAMS *CVParams);
	};
}

#endif	// _DISPSVR_NVAPI_DXVA2HD_MIXER_RENDER_H_