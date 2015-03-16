#ifndef _DISPSVR_D3D9_VIDEO_MIXER_MODEL_H_
#define _DISPSVR_D3D9_VIDEO_MIXER_MODEL_H_

#include <deque>
#include "ReaderWriterLock.h"
#include "D3D9TexturePool.h"
#include "D3D9VBlt.h"

MIDL_INTERFACE("F782676B-A5EF-4807-AD8D-6AC4A20D6AEC") IDispSvrVideoMixerModel : public IUnknown
{
};

namespace DispSvr
{
	struct VideoReferenceSample
	{
		VideoProperty VideoProperty;
		HANDLE hTexture;
	};

	typedef std::deque<VideoReferenceSample> VideoSampleList;

	enum PROTECTION_TYPE
	{
		PROTECTION_NONE = 0,
		PROTECTION_SCRAMBLE_XOR
	};

	struct ContentProtection
	{
		PROTECTION_TYPE eType;
		DWORD dwSizeScrambled;
		DWORD dwScrambleSeed;
	};

	/// Plane properties needed for a D3D9 plane compositing.
	/// Dynamic data will be released when the plane is destroyed.
	struct D3D9Plane
	{
		UINT uWidth;	//< actual width of the plane
		UINT uHeight;	//< actual height of the plane
		UINT uPitch;
		PLANE_FORMAT Format;
		AYUVSample8	Palette[256];
		bool bExternal;
		bool bCreated;
		bool bValid;
		bool bPartialBlending;
		bool bPalettized;
		bool bHasBackingStore;	//< the plane has system/video memory pair.
		bool bHDVideo;
		bool bFullScreenMixing;
        bool bStereoSxSVideoDetected;
		NORMALIZEDRECT nrcDst;
		NORMALIZEDRECT nrcCrop;
		RECT rcSrc;	//< source rectangle to be displayed
		RECT rcDirty;	//< optional dirty rectangle for future optimization
		float fAspectRatio;
		float fAlpha;
		DWORD dwLastUpdateTS;	//< record the last update time stamp
		IDispSvrVideoMixerEventListener *pListener;

		VideoProperty VideoProperty;
		HANDLE hTexture;

        HANDLE hDeptViewTexture;
		/// BackwardSamples maintains a list of reference sample when D3D9PlaneConfig.pVideoProcessorStub->uNumBackwardSamples is not zero
		/// in descending order by reference time.
		VideoSampleList BackwardSamples;

		ContentProtection Protection;
		/// Post texture buffer processing filter to be applied before d3d unlock is called.
		ITextureFilter *pPostTextureFilter;
		/// Custom video blit.
		ID3D9VBlt *pVBlt;

        UINT uStereoMetadataMaxLength;
        UINT uStereoMetadataLength;
        INT8 *pStereoMetadata;
		STEREO_STREAM_MODE eStereoStreamMode;
        STEREO_OFFSET_MODE eStereoOffsetMode;
		LONG lStereoDisplayOffset;		// offset value is only used when eStereoStreamMode == STEREO_STREAM_OFFSET
                                                        // offset value could be fixed offset value or sequence id,it depends on offset mode
                                                        // for sequence id, mixer should get offset value from metadata of main video every frame.
		HANDLE GetViewTextureHandle(UINT uView) const
        {
			if (eStereoStreamMode != STEREO_STREAM_DUALVIEW || uView == 0)
				return hTexture;
			else if (uView == 1)
				return hDeptViewTexture;
			return NULL;
		}

        bool IsValid() const
        {
            return bValid && fAlpha > 0;
        }

        bool IsValidView(UINT uView) const
        {
            return IsValid() && GetViewTextureHandle(uView);
        }

        float GetAspectRatio() const
        {
            if (bStereoSxSVideoDetected)
            {
                // Some side by side videos do not set double width aspect ratio.
                // Heuristically 4:3 * 2 seems to be a good question for the threshold but may be wrong.
                if (fAspectRatio > 2.665f)   // 4/3.0*2 = 2.66666665, 2.665f magic number is selected to include 4:3 * 2
                    return fAspectRatio / 2.f;
            }
            return fAspectRatio;
        }
	};
	typedef std::list<ClearRect> ClearRectList;

    class CD3D9VideoMixerModel : public CD3D9PluginBase, public CCritSec, public CReaderWriterLock
    {
    public:
        CD3D9VideoMixerModel();
        virtual ~CD3D9VideoMixerModel();

  		// IUnknown
		STDMETHOD(QueryInterface)(REFIID riid, void **ppv);

        D3D9Plane &GetPlane(UINT uPlaneID) { return m_Planes[uPlaneID]; }
        const D3D9Plane &GetPlane(UINT uPlaneID) const  { return m_Planes[uPlaneID]; }

        void SetDestination(const NORMALIZEDRECT &DestRect);
		const NORMALIZEDRECT &GetDestination() const  { return m_nrcDst; }
        const ClearRectList &GetClearRectangles() const { return m_ClearRectList; }
		STDMETHOD(SetClearRectangles)(UINT uCount, ClearRect *pRects);
		STDMETHOD(SetBackgroundColor)(COLORREF Color);
		STDMETHOD(GetBackgroundColor)(COLORREF *pColor);
		STDMETHOD(SetLumaKey)(const LumaKey *pLumaKey);
		STDMETHOD(GetLumaKey)(LumaKey *pLumaKey);
        const DXVA2_AYUVSample16 &GetBackgroundColor() const { return m_ayuv16Background; }

    protected:
        // CD3D9PluginBase
		STDMETHOD(_SetDevice)(IUnknown *pDevice);
		STDMETHOD(_ReleaseDevice)();
//		STDMETHOD(_SetWindow)(HWND hwnd);

    protected:
		ClearRectList m_ClearRectList;
		D3D9Plane m_Planes[PLANE_MAX];
		COLORREF m_colorBackGround;
		LumaKey m_LumaKey;
		NORMALIZEDRECT m_nrcDst;
        DXVA2_AYUVSample16 m_ayuv16Background;
    };

	static inline bool IsHDVideo(int Width, int Height)
	{
		return Height >= 720;
	}
}

#endif  // _DISPSVR_D3D9_VIDEO_MIXER_MODEL_H_