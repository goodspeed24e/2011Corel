#ifndef _DISPSVR_DXVAHDVIDEOPROCESSOR_H_
#define _DISPSVR_DXVAHDVIDEOPROCESSOR_H_

#include "Exports/Inc/VideoMixer.h"
#include <set>

#define DXVAHDVP_MAX_STREAMS			5
#define DXVAHDVP_MAX_PAST_FRAMES		2		// too many past frames many decrease decoder performance
#define DXVAHDVP_MAX_FUTURE_FRAMES		2
#define DXVAHDVP_MAX_FILTER				6
#define DXVAHDVP_MAX_PALETTE			256

interface IDispSvrDriverExtension;

namespace DispSvr
{
	union DXVAHDVP_INPUT_COLORS_SPACE
	{
		struct
		{
			UINT bGraphics : 1;			// video or graphics
			UINT bLimitedRange : 1;		// full range [0 - 255] or limited range [16 - 235]
			UINT bBT709 : 1;			// ITU-R BT.601 or ITU-R BT.709
			UINT bXvYCC : 1;			// extended YCbCr or not
			UINT uPadding : 28;
		};

		UINT value;
	};

	union DXVAHDVP_OUTPUT_COLOR_SPACE
	{
		struct
		{
			UINT bVideoProcessing : 1;
			UINT bLimitedRange : 1;		// full range [0 - 255] or limited range [16 - 235]
			UINT bBT709 : 1;			// ITU-R BT.601 or ITU-R BT.709
			UINT bXvYCC : 1;			// extended YCbCr or not
			UINT uPadding : 28;
		};

		UINT value;
	};

	struct DXVAHDVP_FILTER
	{
		BOOL bEnable;
		FLOAT fLevel;
	};
	
	enum DXVAHDVP_FRAME_FORMAT
	{
		DXVAHDVP_FRAME_FORMAT_PROGRESSIVE = 0,
		DXVAHDVP_FRAME_FORMAT_INTERLACED_TOP_FIELD_FIRST = 1,
		DXVAHDVP_FRAME_FORMAT_INTERLACED_BOTTOM_FIELD_FIRST = 2
	};

	struct DXVAHDVP_StreamState
	{
		// stream data set
		BOOL bEnable;
		IDirect3DSurface9 *pInputSurface;							// pInputSurface is a weak reference.
		IDirect3DSurface9 *ppPastSurfaces[DXVAHDVP_MAX_PAST_FRAMES];	// pPastFrames are weak references.
		UINT uPastFrameCount;
		UINT uOutputIndex;

		// stream state
		RECT SourceRect;
		RECT DestinationRect;
		DXVAHDVP_FRAME_FORMAT FrameFormat;
		DXVAHDVP_INPUT_COLORS_SPACE InputColorSpace;
		DXVAHDVP_FILTER Filter[DXVAHDVP_MAX_FILTER];
		FLOAT fAlpha;
		LumaKey LumaKey;
		UINT uPaletteCount;
		AYUVSample8 Palette[DXVAHDVP_MAX_PALETTE];
		DWORD dwLastUpdateTS;

		// stream stereo data set, currently setting through driver extension only.
		// DXVAHD was designed to be extendable so stereo may be included in future DXVAHD spec.
		MIXER_STEREO_MODE eStereoMixingMode;
		IDirect3DSurface9 *pDependentSurface;	// pDependentSurface is a weak reference
		LONG lStereoDisplayOffset;
	};

	struct DXVAHDVP_BltState
	{
		RECT TargetRect;
		DXVAHDVP_OUTPUT_COLOR_SPACE OutputColorSpace;
	};

	struct DXVAHDVP_INIT
	{
		bool bNvDXVAHD;
		bool bUseNativeDXVAHDOnly;
	};

	class CDXVAHDVP
	{
	public:
		static HRESULT CreateVP(IDirect3DDevice9Ex *pDeviceEx, DXVAHDVP_INIT *pInit, CDXVAHDVP **ppVP);

		~CDXVAHDVP();
		HRESULT SetStreamState(UINT uStreamID, const DXVAHDVP_StreamState &state, bool ForceUpdate = 0);
		HRESULT VBltHD(IDirect3DSurface9 *pDestSurface, const DXVAHDVP_BltState &bltState);
		HRESULT GetVPCaps(VideoProcessorCaps &sCaps);
		HRESULT GetFilterValueRange(VIDEO_FILTER eFilter, ValueRange &range);
		UINT GetMaxInputStream() const { return m_uMaxInputStream; }
		bool IsValidInputFormat(PLANE_FORMAT fmt) const;
		bool IsNativeDXVAHD() const { return m_pHDVP != 0; }

	protected:
		struct DXVAHDVP_StreamStateCache;

		CDXVAHDVP();
		HRESULT CreateVP(IDirect3DDevice9Ex *pDeviceEx, DXVAHDVP_INIT *pInit);
		HRESULT SyncBltState(const DXVAHDVP_BltState &bltState);

		HRESULT D3D9VBltHD(IDirect3DSurface9 *pOutputSurface, UINT OutputFrame, UINT StreamCount, const DXVAHD_STREAM_DATA *pStreams);
		HRESULT CreateD3D9VP(IDirect3DDevice9Ex *pDeviceEx);
		HRESULT DXVA2Blt(IDirect3DSurface9 *pDestSurface, DXVAHDVP_StreamStateCache &c, const DXVAHD_STREAM_DATA &d, const DXVAHDVP_BltState &blt);
		HRESULT D3D9Blt(IDirect3DSurface9 *pDestSurface, DXVAHDVP_StreamStateCache &c, const DXVAHD_STREAM_DATA &d, const DXVAHDVP_BltState &blt);

	protected:
		// we must keep m_pHDDev alive while m_pHDVP is in use because a bug in NV DXVA HD implementation.
		IDXVAHD_Device *m_pHDDev;
		IDXVAHD_VideoProcessor *m_pHDVP;
		std::set<D3DFORMAT> m_setValidInputFormat;
		std::set<D3DFORMAT> m_setValidOutputFormat;
		UINT m_uMaxInputStream;
		UINT m_uChangedStream;
		UINT m_uOutputFrame;
		DXVAHDVP_INIT m_InitParam;
		VideoProcessorCaps m_VPCaps;
		ValueRange m_VPFilterRange[DXVAHDVP_MAX_FILTER];

		DXVAHDVP_BltState m_BltStateCache;
		DXVAHDVP_StreamStateCache *m_pStreamStateCache;
		DXVAHD_STREAM_DATA *m_pDXVAHDStreamData;
		IDispSvrDriverExtension *m_pDriverExtension;

		IDirect3DDevice9Ex *m_pDevice;		// we don't keep m_pDevice refcount unless it is not native DXVAHD
	};
}

#endif	// _DISPSVR_DXVAHDVIDEOPROCESSOR_H_