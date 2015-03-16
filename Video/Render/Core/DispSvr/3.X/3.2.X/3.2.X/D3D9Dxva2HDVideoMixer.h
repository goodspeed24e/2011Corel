#ifndef _DISPSVR_D3D9_DXVA2HD_VIDEOMIXER_H_
#define _DISPSVR_D3D9_DXVA2HD_VIDEOMIXER_H_

#include <set>
#include <dxva2api.h>
#include "DXVAHDVideoProcessor.h"
#include "D3D9VideoMixer.h"

namespace DispSvr
{
	class CD3D9Dxva2HDVideoMixer : public CD3D9VideoMixerBase
	{
	public:
		CD3D9Dxva2HDVideoMixer();
		virtual ~CD3D9Dxva2HDVideoMixer();

		// DXVAHD does not support clear rectangles
		STDMETHOD(SetClearRectangles)(UINT uCount, ClearRect *pRects) { return E_NOTIMPL; }

	protected:
		STDMETHOD(_SetDevice)(IUnknown *pDevice);
		STDMETHOD(_ReleaseDevice)();
		STDMETHOD(_QueryPlaneCaps)(PLANE_ID PlaneID, PLANE_FORMAT Format, PlaneCaps *pCap);
		STDMETHOD(_Execute)(IDirect3DSurface9 *pDestSurface, const RECT &rcDst, const RECT &rcDstClip);

		HRESULT VideoProcessBltHD(IDirect3DSurface9 *pDst, const RECT &rcDst, const RECT &rcDstClip);
		HRESULT SetSingleStreamStates(PLANE_ID PlaneID, DXVAHDVP_StreamState &State);
		HRESULT SetSingleVideoStreamStates(PLANE_ID PlaneID, DXVAHDVP_StreamState &State, UINT uViewID = 0);
		HRESULT SetSingleStereoStreamStates(PLANE_ID PlaneID, DXVAHDVP_StreamState &State);

	protected:
		CDXVAHDVP *m_pVP;

		DXVAHDVP_INIT m_VPInitParam;
	};
}

#endif	// _DISPSVR_D3D9_DXVA2HD_VIDEOMIXER_H_