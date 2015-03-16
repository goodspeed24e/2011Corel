#ifndef _DISPSVR_D3D9_VIDEO_MIXER_BASE_H_
#define _DISPSVR_D3D9_VIDEO_MIXER_BASE_H_

#include <deque>
#include "Exports/Inc/VideoMixer.h"
#include "D3D9PluginBase.h"
#include "D3D9TexturePool.h"
#include "D3D9VBlt.h"

interface ID3D9VideoEffect3DProcessor;
interface IXVideoEffectManager;

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

        /// VideoSamples maintains a list of video samples
        /// including forward reference samples, current video sample and backward reference samples
		/// in descending order by reference time.
        VideoSampleList VideoSamples;
        // uiPlaneSampleIndex stands for the index of current video sample (plane sample) in VideoSamples
        UINT uiPlaneSampleIndex;

		ContentProtection Protection;
		/// Post texture buffer processing filter to be applied before d3d unlock is called.
		ITextureFilter *pPostTextureFilter;
		/// Custom video blit.
		ID3D9VBlt *pVBlt;
	};

	struct VideoProcessorStub
	{
		GUID guidVP;
		D3DFORMAT RenderTargetFormat;
		VideoProcessorCaps sCaps;
		ValueRange FilterRanges[6];
		float fFilterValue[6];
		TpfnVBltFactoryMethod pfnVBltFactory;
		VideoProcessorStub *pDelegateVPStub;
	};

	/// D3D9PlaneConfig is a static storage to save settings that do not change when planes are created or destroyed.
	struct D3D9PlaneConfig
	{
		VideoProcessorStub *pVideoProcessorStub;
	};

	/// Basic mixer implementation for all other D3D9 related mixers.
	class DECLSPEC_NOVTABLE CD3D9VideoMixerBase :
		public virtual CD3D9PluginBase,
		public virtual IDispSvrVideoMixer,
		public virtual IDispSvrVideoProcessor,
        public virtual IDispSvrVideoEffectManager
	{
	public:
		friend class CD3D9VideoMixerPlane;

	public:
		STDMETHOD(QueryInterface)(REFIID riid, void **ppv);
		STDMETHOD_(ULONG, AddRef)()	{ return CD3D9PluginBase::AddRef();	}
		STDMETHOD_(ULONG, Release)() { return CD3D9PluginBase::Release(); }

		STDMETHOD(ProcessMessage)(RESOURCE_MESSAGE_TYPE msg, LPVOID ulParam);

	protected:
		// IDispSvrVideoMixer interfaces
		STDMETHOD(SetDestination)(IUnknown *pSurface, const NORMALIZEDRECT *pDest);
		STDMETHOD(Execute)();
		STDMETHOD(QueryCaps)(MixerCaps *pCaps);
		STDMETHOD(CreatePlane)(PlaneInit *pInit, REFIID riid, void **ppPlane);
		STDMETHOD(QueryPlaneFormatCount)(PLANE_ID PlaneID, UINT *pCount);
		STDMETHOD(QueryPlaneFormat)(PLANE_ID PlaneID, PLANE_FORMAT *pFormats);
		STDMETHOD(QueryPlaneCaps)(PLANE_ID PlaneID, PLANE_FORMAT Format, PlaneCaps *pCap);
		STDMETHOD(SetClearRectangles)(UINT uCount, ClearRect *pRects);
		STDMETHOD(SetBackgroundColor)(COLORREF Color);
		STDMETHOD(GetBackgroundColor)(COLORREF *pColor);
		STDMETHOD(SetLumaKey)(const LumaKey *pLumaKey);
		STDMETHOD(GetLumaKey)(LumaKey *pLumaKey);
		STDMETHOD(SetProperty)(const MixerProperty *pProperty);
		STDMETHOD(GetProperty)(MixerProperty *pProperty);

		// IDispSvrVideoProcessor interfaces
		STDMETHOD(GetAvailableVideoProcessorModeCount)(UINT *pCount);
		STDMETHOD(GetAvailableVideoProcessorModes)(GUID *pGUID);
		STDMETHOD(GetVideoProcessorCaps)(LPCGUID lpGUID, DispSvr::VideoProcessorCaps *pCaps);
		STDMETHOD(GetVideoProcessorMode)(LPGUID lpGUID);
		STDMETHOD(SetVideoProcessorMode)(LPCGUID lpGUID);
		STDMETHOD(GetFilterValueRange)(DispSvr::VIDEO_FILTER eFilter, DispSvr::ValueRange *pValueRange);
		STDMETHOD(SetFilterValue)(DispSvr::VIDEO_FILTER eFilter, float fValue);
		STDMETHOD(GetFilterValue)(DispSvr::VIDEO_FILTER eFilter, float *pfValue);

        // IDispSvrVideoEffectManager
        STDMETHOD(GetVideoEffectManager)(IUnknown **ppManager);
        STDMETHOD(SetVideoEffectManager)(IUnknown *pManager);

	protected:
		// methods for overwriting by derived classes.
		STDMETHOD(_Execute)(IDirect3DSurface9 *pDestSurface, const RECT &rcDst, const RECT &rcDstClip) = 0;
		STDMETHOD(_SetDevice)(IUnknown *pDevice);
		STDMETHOD(_ReleaseDevice)();
		STDMETHOD(_CreatePlane)(DispSvr::PlaneInit *pInit);
		STDMETHOD(_DestroyPlane)(PLANE_ID PlaneID);
		STDMETHOD(_GetPlaneDesc)(PLANE_ID PlaneID, PlaneDesc *pDesc);
		STDMETHOD(_SetExternalSurface)(PLANE_ID PlaneID, const ExternalSurfaceDesc *pEx);
		STDMETHOD(_QueryStatusOfExternalSurface)(PLANE_ID PlaneID, const ExternalSurfaceDesc *pEx);
		STDMETHOD(_UpdateFromBuffer)(PLANE_ID PlaneID, UpdateBuffer *pBuffer);
		STDMETHOD(_LockBuffer)(PLANE_ID PlaneID, LockedRect *pOut, const RECT *rcSrc, DWORD dwFlags);
		STDMETHOD(_UnlockBuffer)(PLANE_ID PlaneID);
		STDMETHOD(_SetPlanarAlpha)(PLANE_ID PlaneID, float fAlpha);
		STDMETHOD(_GetPlanarAlpha)(PLANE_ID PlaneID, float *pfAlpha);
		STDMETHOD(_SetPosition)(PLANE_ID PlaneID, const NORMALIZEDRECT *rcDst, const RECT *rcSrc, const NORMALIZEDRECT *rcCrop, float fAspectRatio);
		STDMETHOD(_GetPosition)(PLANE_ID PlaneID, NORMALIZEDRECT *rcDst, RECT *rcSrc, NORMALIZEDRECT *rcCrop, float *pfAspectRatio);
		STDMETHOD(_SetDirtyRect)(PLANE_ID PlaneID, const RECT *rcDirty);
		STDMETHOD(_SetVideoProperty)(PLANE_ID PlaneID, const VideoProperty *pProperty);
		STDMETHOD(_GetVideoProperty)(PLANE_ID PlaneID, VideoProperty *pProperty);
		STDMETHOD(_QueryPlaneCaps)(PLANE_ID PlaneID, PLANE_FORMAT Format, PlaneCaps *pCap);
		STDMETHOD(_OnMoveWindow)();

		HRESULT NotifyListeners(DispSvr::EVENT_VIDEO_MIXING event, DWORD dwParam1, DWORD dwParam2);
		bool IsBackgroundVisible() const;
		void SelectVideoProcessor(VideoProcessorStub *pMainVP = NULL, VideoProcessorStub *pSubVP = NULL);
		HRESULT GenerateDxva2VPList();
		HRESULT PlaneToScreen(const D3D9Plane &plane, RECT &rcSrc, RECT &rcDst, const RECT &rcDstClip);
		void ClearNonVideoArea();
        HRESULT ProcessVideoEffect(PLANE_ID PlaneID, IDirect3DSurface9 *pDestSurface, RECT &rcSrc, const RECT &rcDst, PLANE_FORMAT IntermediateFormat, IUnknown **ppDestUnk); 
        void ReleaseVideoSamples(PLANE_ID PlaneID);

	public:
		CD3D9VideoMixerBase();
		virtual ~CD3D9VideoMixerBase();

	protected:
		typedef std::list<ClearRect> ClearRectList;
		typedef std::list<VideoProcessorStub> VideoProcessorList;

		CCritSec m_csObj;
		COLORREF m_colorBackGround;
		LumaKey m_LumaKey;
		D3D9Plane m_Planes[PLANE_MAX];
		D3D9PlaneConfig m_PlaneConfigs[PLANE_MAX];
		NORMALIZEDRECT m_nrcDst;
		ClearRectList m_ClearRectList;
		CD3D9TexturePool *m_pTexturePool;
		MixerProperty m_Property;
		CCritSec m_csExecuting;
		HANDLE m_hSurfaceReadyEvent;
		LONG m_lLockingCount;
		D3DRECT m_ClearNonVideoAreaList[4];
		UINT m_uClearNonVideoAreaListCount;
		VideoProcessorList m_VideoProcessorList;
		ID3D9VideoEffect3DProcessor *m_pVideoEffect3DBlt;
        IXVideoEffectManager *m_pXVEManager;

		MixerCaps m_MixerCaps;
		RECT m_rcMixingDstClip;
		RECT m_rcMixingDst;
		float m_fWindowAspectRatio;
		bool m_bRecalculateVideoPosition;
	};

	/// Plane object represents IDispSvrVideoMixerPlane and IDispSvrVideoMixerVideoPlane and
	/// delegates commands to the underlying mixer implementation.
	class CD3D9VideoMixerPlane : public IDispSvrVideoMixerVideoPlane
	{
	public:
		CD3D9VideoMixerPlane(const PlaneInit &InitOpt, CD3D9VideoMixerBase *pMixer);
		~CD3D9VideoMixerPlane();

		STDMETHOD(QueryInterface)(REFIID riid, void **ppv);
		STDMETHOD_(ULONG, AddRef)();
		STDMETHOD_(ULONG, Release)();

		// IDispSvrVideoMixerPlane
		STDMETHOD(GetPlaneDesc)(PlaneDesc *pDesc);
		STDMETHOD(SetExternalSurface)(const ExternalSurfaceDesc *pEx);
		STDMETHOD(UpdateFromBuffer)(UpdateBuffer *pBuffer);
		STDMETHOD(LockBuffer)(LockedRect *pOut, const RECT *rcSrc, DWORD dwFlags);
		STDMETHOD(UnlockBuffer)();
		STDMETHOD(SetPlanarAlpha)(float fAlpha);
		STDMETHOD(GetPlanarAlpha)(float *pfAlpha);
		STDMETHOD(SetPosition)(const NORMALIZEDRECT *rcDst, const RECT *rcSrc, const NORMALIZEDRECT *rcCrop, float fAspectRatio);
		STDMETHOD(GetPosition)(NORMALIZEDRECT *rcDst, RECT *rcSrc, NORMALIZEDRECT *rcCrop, float *pfAspectRatio);
		STDMETHOD(SetDirtyRect)(const RECT *rcDirty);

		// IDispSvrVideoMixerVideoPlane
		STDMETHOD(SetVideoProperty)(const VideoProperty *pProperty);
		STDMETHOD(GetVideoProperty)(VideoProperty *pProperty);
		STDMETHOD(SetColorControl)(const ColorControl *pCC);
		STDMETHOD(GetColorControl)(ColorControl *pCC);

	protected:
		CD3D9VideoMixerBase *m_pMixer;
		LONG m_cRef;
		PlaneInit m_InitOpt;
		bool m_bExternalSurface;
	};

	static inline bool IsD3D9PlaneValid(const D3D9Plane *plane)
	{
		return plane->bValid && plane->fAlpha > 0;
	}

	static inline bool IsHDVideo(int Width, int Height)
	{
		return Height >= 720;
	}
}

#endif	// _DISPSVR_D3D9_VIDEO_MIXER_BASE_H_
