#ifndef _DISPSVR_D3D9_DXVA2HD_VIDEOMIXER_H_
#define _DISPSVR_D3D9_DXVA2HD_VIDEOMIXER_H_

#include <set>
#include <dxva2api.h>
#include "Imports/ThirdParty/Microsoft/DXVAHD/dxvahd.h"
#include "D3D9VideoMixer.h"

#define MAX_DXVAHD_STREAMS	5
#define MAX_PAST_FRAMES		2		// too many past frames many decrease decoder performance
#define MAX_FUTURE_FRAMES	2

namespace DispSvr
{
	class CD3D9Dxva2HDVideoMixer : public CD3D9VideoMixerBase
	{
	public:
		CD3D9Dxva2HDVideoMixer();
		virtual ~CD3D9Dxva2HDVideoMixer();

		STDMETHOD(_SetDevice)(IUnknown *pDevice);
		STDMETHOD(_ReleaseDevice)();
		STDMETHOD(_QueryPlaneCaps)(PLANE_ID PlaneID, PLANE_FORMAT Format, PlaneCaps *pCap);
		STDMETHOD(SetClearRectangles)(UINT uCount, ClearRect *pRects);

	protected:
		/// We cache certain paramters in stream state and avoid frequently updating to the runtime.
		/// Only update stream states when there is any change.
		struct StreamDataCache
		{
			D3DFORMAT d3dSurfaceFormat;
			DXVAHD_FRAME_FORMAT FrameFormat;
			RECT SourceRect;
			RECT DestinationRect;
			DXVAHD_STREAM_STATE_INPUT_COLOR_SPACE_DATA InputColorSpaceData;
			DXVAHD_STREAM_STATE_ALPHA_DATA AlphaData;
			DXVAHD_STREAM_STATE_FILTER_DATA FilterData[6];
			LumaKey LumaKey;
			UINT uPaletteCount;
			UINT uInputFrameOrField;
			AYUVSample8 Palette[256];
			DWORD dwLastUpdateTS;
		};

		/// Data store provides a storage for DXVAHD_STREAM_DATA ppPastFrames/ppFutureFrames to point to.
		/// So we can guarantee the surface pointers will remain accessible during VideoProcessBltHD() call.
		/// All pointers stored in the data store are weak references.
		struct StreamDataStore
		{
			IDirect3DSurface9 *pPastFrames[MAX_PAST_FRAMES];
			IDirect3DSurface9 *pFutureFrames[MAX_FUTURE_FRAMES];
		};

		/// Cache Blt state too to prevent from frequent updating.
		struct BltStateCache
		{
			RECT TargetRect;
			DXVAHD_BLT_STATE_OUTPUT_COLOR_SPACE_DATA OutputColorSpaceData;
		};

		typedef HRESULT (CD3D9Dxva2HDVideoMixer::*TpfnSetStreamState)(PLANE_ID PlaneID, StreamDataStore &sDataStore, DXVAHD_STREAM_DATA &sStreamData, StreamDataCache &cache);

	protected:
		STDMETHOD(_Execute)(IDirect3DSurface9 *pDestSurface, const RECT &rcDst, const RECT &rcDstClip);
		HRESULT VideoProcessBltHD(IDirect3DSurface9 *pDst, const RECT &rcDst, const RECT &rcDstClip);
		HRESULT SetSingleStreamStates(PLANE_ID PlaneID, StreamDataStore &sDataStore, DXVAHD_STREAM_DATA &sStreamData, StreamDataCache &cache);
		HRESULT SetSingleVideoStreamStates(PLANE_ID PlaneID, StreamDataStore &sDataStore, DXVAHD_STREAM_DATA &sStreamData, StreamDataCache &cache);
		HRESULT SyncStreamState(UINT uiStreamNum, const StreamDataCache &data);
		HRESULT SetVPBltHDStates(const RECT &rcDst, const RECT &rcDstClip);
		void SetDefaultStates();

	protected:
		// we must keep m_pHDDev alive while m_pHDVP is in use because a bug in NV DXVA HD implementation.
		IDXVAHD_Device *m_pHDDev;
		IDXVAHD_VideoProcessor* m_pHDVP;
		TpfnSetStreamState m_pfnStreamSetter[PLANE_MAX];
		DXVAHD_STREAM_DATA m_StreamData[PLANE_MAX];
		StreamDataCache m_StreamDataCaches[PLANE_MAX];
		StreamDataStore m_StreamDataStore[PLANE_MAX];
		BltStateCache m_BltStateCache;
		UINT m_uiFrameCount;
		UINT m_uMaxInputStream;
		bool m_bNvDxvaHD;
		std::set<D3DFORMAT> m_setValidInputFormat;
	};
}

#endif	// _DISPSVR_D3D9_DXVA2HD_VIDEOMIXER_H_