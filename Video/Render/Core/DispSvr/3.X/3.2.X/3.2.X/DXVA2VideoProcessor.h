#ifndef _DISPSVR_DXVA2VIDEOPROCESSOR_H_
#define _DISPSVR_DXVA2VIDEOPROCESSOR_H_

#include "Exports/Inc/VideoMixer.h"
#include <dxva2api.h>

namespace DispSvr
{
	enum DXVA2VP_REFSAMPLE_TYPE
	{
		DXVA2VP_REFSAMPLE_TYPE_VIDEO = 0,
		DXVA2VP_REFSAMPLE_TYPE_GRAPHICS = 1
	};

	struct DXVA2VP_RefSample
	{
		DXVA2VP_RefSample() : eType(DXVA2VP_REFSAMPLE_TYPE_VIDEO), pSurface(NULL) { }

		DXVA2VP_REFSAMPLE_TYPE eType;
		IDirect3DSurface9 *pSurface;
		union
		{
		VideoProperty VideoProperty;
			struct
			{
				RECT rcSrc;
				RECT rcDst;
			} GraphicsProperty;
		};
	};

	struct DXVA2VP_Caps
	{
		GUID guidVP;
		D3DFORMAT RenderTargetFormat;
		VideoProcessorCaps sCaps;
		ValueRange FilterRanges[6];
	};

	/// CDXVA2VideoProcessor only provides a wrapper class to DXVA2 video processor related
	/// APIs, and is not thread-safe.
	class CDXVA2VideoProcessor
	{
	public:
		CDXVA2VideoProcessor(REFGUID refguid, D3DFORMAT rtFormat);
		~CDXVA2VideoProcessor();

		/// Releases all held interfaces and initialize internal structures.
		void Release();

		/// Create a DXVA2 video processor with the given surface, video property.
		HRESULT CreateVideoProcessor(IDirect3DSurface9 *pSrc, IDirect3DSurface9 *pDst, const VideoProperty &prop);

		/// Calls DXVA2 VideoProcessBlt and creates video processor device when necessary.
		/// @return E_INVALIDARG when input arguments are not valid.
		///			S_OK when succeeds.
		///         and other errors returned from creating device and DXVA2 VideoProcessBlt.
		HRESULT VideoProcessBlt(
			IDirect3DSurface9 *pSrc, const RECT &rcSrc,
			IDirect3DSurface9 *pDst, const RECT &rcDst,
			const VideoProperty &prop,
			DXVA2VP_RefSample *pSamples,
			UINT nSamples,
			float *pfFilterRange
			);

		// Get a list of DXVA2VP_Caps, caller is responsible for releasing memory returned by ppCaps.
		static HRESULT GetDXVA2VPCaps(IDirect3DDevice9 *pDevice, UINT *puCount, DXVA2VP_Caps **ppCaps);

	protected:
		IDirectXVideoProcessor *m_pVP;
		DXVA2_VideoDesc m_VideoDesc;
		GUID m_guidDesired;
		D3DFORMAT m_renderTargetFormat;
		DXVA2_ValueRange m_rangeFilter[13];		// DXVA2_NoiseFilterLumaLevel = 1 ... DXVA2_DetailFilterChromaRadius = 12
	};
}

#endif	// _DISPSVR_DXVA2VIDEOPROCESSOR_H_