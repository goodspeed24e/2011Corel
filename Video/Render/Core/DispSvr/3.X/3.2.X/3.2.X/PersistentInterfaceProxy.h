#ifndef _DISPSVR_PERSISTENT_INTERFACE_PROXY_H_
#define _DISPSVR_PERSISTENT_INTERFACE_PROXY_H_

namespace DispSvr
{
    class CPersistentInterfaceProxy
    {
    public:
        CPersistentInterfaceProxy();
        ~CPersistentInterfaceProxy();

        HRESULT GetInterface(REFGUID guidGroupId, REFIID riid, void **ppv);
        HRESULT SetInstance(REFGUID guidGroupId, IUnknown *pInstance);

    protected:
        class CVideoMixerInterfaceProxy :
            public IDispSvrVideoMixer,
            public IDispSvrVideoProcessor,
            public IDispSvrVideoEffectManager,
            public IDispSvrVideoPlaneCallback
        {
        public:
            CVideoMixerInterfaceProxy();
            virtual ~CVideoMixerInterfaceProxy();
            HRESULT SetInstance(IUnknown *pInstance);

            // IUnkonwn
	        STDMETHOD(QueryInterface)(REFIID riid, void **ppv);
            STDMETHOD_(ULONG, AddRef)();
            STDMETHOD_(ULONG, Release)();

            // IDispSvrVideoMixer
	        STDMETHOD(Execute)();
	        STDMETHOD(SetDestination)(IUnknown *pSurface, const NORMALIZEDRECT *pDest);
	        STDMETHOD(QueryCaps)(DispSvr::MixerCaps *pCaps);
	        STDMETHOD(CreatePlane)(DispSvr::PlaneInit *pInit, REFIID riid, void **ppPlane);
	        STDMETHOD(QueryPlaneFormatCount)(DispSvr::PLANE_ID PlaneID, UINT *pCount);
	        STDMETHOD(QueryPlaneFormat)(DispSvr::PLANE_ID PlaneID, DispSvr::PLANE_FORMAT *pFormats);
	        STDMETHOD(QueryPlaneCaps)(DispSvr::PLANE_ID PlaneID, DispSvr::PLANE_FORMAT Format, DispSvr::PlaneCaps *pCap);
	        STDMETHOD(SetClearRectangles)(UINT uCount, DispSvr::ClearRect *pRects);
	        STDMETHOD(SetBackgroundColor)(COLORREF Color);
	        STDMETHOD(GetBackgroundColor)(COLORREF *pColor);
	        STDMETHOD(SetLumaKey)(const DispSvr::LumaKey *pLumaKey);
	        STDMETHOD(GetLumaKey)(DispSvr::LumaKey *pLumaKey);
	        STDMETHOD(SetProperty)(const DispSvr::MixerProperty *pProperty);
	        STDMETHOD(GetProperty)(DispSvr::MixerProperty *pProperty);

            // IDispSvrVideoProcessor
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

            // IDispSvrVideoPlaneCallback
            virtual const D3D9PlaneConfig &GetPlaneConfig(PLANE_ID PlaneID);
            STDMETHOD_(void, OnVideoPositionChange)();
	        STDMETHOD_(void, OnDestroyPlane)(PLANE_ID PlaneID);
		    STDMETHOD(OnCreatePlane)(const DispSvr::PlaneInit &pInit);

        protected:
            // These are weak references to the object. ResourceManager holds the instances.
            IDispSvrVideoMixer *m_pMixer;
            IDispSvrVideoProcessor *m_pVideoProcessor;
            IDispSvrVideoEffectManager *m_pEffMgr;
            IDispSvrVideoPlaneCallback *m_pVideoPlaneCB;
            IUnknown *m_pInstance;
        };

    protected:
        CVideoMixerInterfaceProxy *m_pVideoMixerProxy;
    };

}

#endif  // _DISPSVR_PERSISTENT_INTERFACE_PROXY_H_