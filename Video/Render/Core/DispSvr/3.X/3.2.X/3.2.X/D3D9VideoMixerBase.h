#ifndef _DISPSVR_D3D9_VIDEO_MIXER_BASE_H_
#define _DISPSVR_D3D9_VIDEO_MIXER_BASE_H_

#include "Imports/XVE/Inc/XVE.h"
#include "D3D9PluginBase.h"
#include "D3D9VideoMixerModel.h"
#include "D3D9VideoPlane.h"

interface ID3D9VideoEffect3DProcessor;
interface IXVideoEffectManager;
interface IDispSvrDriverExtension;

namespace DispSvr
{
	/// Basic mixer implementation for all other D3D9 related mixers.
	class DECLSPEC_NOVTABLE CD3D9VideoMixerBase :
		public virtual CD3D9PluginBase,
		public virtual IDispSvrVideoMixer,
		public virtual IDispSvrVideoProcessor,
        public virtual IDispSvrVideoEffectManager,
        public virtual IXVEManagerNotify,
        public virtual IDispSvrVideoPlaneCallback
	{
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
		STDMETHOD(GetMappedFilterValue)(DispSvr::VIDEO_FILTER eFilter, float *pfValue);

        // IDispSvrVideoEffectManager
        STDMETHOD(GetVideoEffectManager)(IUnknown **ppManager);
        STDMETHOD(SetVideoEffectManager)(IUnknown *pManager);

        // IXVEManagerNotify
        STDMETHOD(OnNotify)(DWORD dwStreamID, REFGUID rguidEffectID, XVE::XVE_EVENT_TYPE dwEvent, DWORD dwParams);

        // IDispSvrVideoPlaneCallback
        virtual const D3D9PlaneConfig &GetPlaneConfig(PLANE_ID PlaneID);
        STDMETHOD_(void, OnVideoPositionChange)();
	    STDMETHOD_(void, OnDestroyPlane)(PLANE_ID PlaneID);
		STDMETHOD(OnCreatePlane)(const DispSvr::PlaneInit &pInit);

    protected:
		// methods for overwriting by derived classes.
		STDMETHOD(_Execute)(IDirect3DSurface9 *pDestSurface, const RECT &rcDst, const RECT &rcDstClip) = 0;
		STDMETHOD(_SetDevice)(IUnknown *pDevice);
		STDMETHOD(_ReleaseDevice)();
		STDMETHOD(_QueryPlaneCaps)(PLANE_ID PlaneID, PLANE_FORMAT Format, PlaneCaps *pCap);
		STDMETHOD(_OnMoveWindow)();
		STDMETHOD(_ResetDevice)();

        HRESULT NotifyListeners(DispSvr::EVENT_VIDEO_MIXING event, DWORD dwParam1, DWORD dwParam2);
		bool IsBackgroundVisible() const;
		void SelectVideoProcessor(VideoProcessorStub *pMainVP = NULL, VideoProcessorStub *pSubVP = NULL);
		HRESULT GenerateDxva2VPList();
		HRESULT PlaneToScreen(const D3D9Plane &plane, RECT &rcSrc, RECT &rcDst, const RECT &rcDstClip);
		HRESULT StereoPlaneToScreen(UINT uViewID, const D3D9Plane &plane, RECT &rcSrc, RECT &rcDst, const RECT &rcDstClip);
		void ClearNonVideoArea();
		HRESULT CheckXVEManager();
        HRESULT ProcessVideoEffect(PLANE_ID PlaneID, bool *pbStereoDetected, RECT &rcSrc, const RECT &rcDst, PLANE_FORMAT IntermediateFormat, IUnknown **ppDestUnk, UINT uViewID = 0); 
        HRESULT getFilterSaturationValue(float *pfValue);
		MIXER_STEREO_MODE GetMixerStereoMode() const { return m_Property.bStereoEnable ? m_Property.eStereoMode : MIXER_STEREO_MODE_DISABLED; }
        INT GetStereoOffset(const D3D9Plane &plane) const;

	public:
		CD3D9VideoMixerBase();
		virtual ~CD3D9VideoMixerBase();

	protected:
		typedef std::list<VideoProcessorStub> VideoProcessorList;

		CCritSec m_csObj;

        D3D9PlaneConfig m_PlaneConfigs[PLANE_MAX];
		D3DRECT m_ClearNonVideoAreaList[4];
		UINT m_uClearNonVideoAreaListCount;
		VideoProcessorList m_VideoProcessorList;
		ID3D9VideoEffect3DProcessor *m_pVideoEffect3DBlt;
        IXVideoEffectManager *m_pXVEManager;
        CD3D9VideoMixerModel *m_pModel;
		CD3D9TexturePool *m_pTexturePool;

		MixerProperty m_Property;
		MixerCaps m_MixerCaps;
		RECT m_rcMixingDstClip;
		RECT m_rcMixingDst;
		float m_fWindowAspectRatio;
		bool m_bRecalculateVideoPosition;
        bool m_bClearFullRenderTarget;
        IDispSvrDriverExtension *m_pDriverExtension;
	};

    ID3D9VBlt *CreateVBltFromVPStub(D3D9Plane *pPlane, VideoProcessorStub *pVPStub, CD3D9TexturePool *pPool);
}

#endif	// _DISPSVR_D3D9_VIDEO_MIXER_BASE_H_
