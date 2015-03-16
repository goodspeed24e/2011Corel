#ifndef _DISPSVR_D3D9_VIDEO_PLANE_H_
#define _DISPSVR_D3D9_VIDEO_PLANE_H_

MIDL_INTERFACE("B05B4BC5-3BB3-4341-BE96-5215B9D5F99D") IDispSvrVideoPlaneCallback;

namespace DispSvr
{
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
		TEXTURE_USAGE_FLAG eTextureUsage;
	};

	/// Plane object represents IDispSvrVideoMixerPlane and IDispSvrVideoMixerVideoPlane and
	/// delegates commands to the underlying mixer implementation.
	class CD3D9VideoMixerPlane : public IDispSvrVideoMixerVideoPlane, 
        public IDispSvrVideoMixerDependentView,
        public IDispSvrVideoMixerPlaneStereoControl
	{
	public:
        static HRESULT Create(const PlaneInit &InitOpt, CD3D9VideoMixerPlane **ppPlane);
		virtual ~CD3D9VideoMixerPlane();

        // IUnknown
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

        //IDispSvrVideoMixerDependentView
        STDMETHOD(SetDeptViewExternalSurface)(UINT uViewID,  const DispSvr::ExternalSurfaceDesc *pEx);
        STDMETHOD(UpdateDeptViewFromBuffer)(UINT uViewID,  DispSvr::UpdateBuffer *pBuffer);
        STDMETHOD(LockDeptViewBuffer)(UINT uViewID,  DispSvr::LockedRect *pOut, const RECT *rcSrc, DWORD dwFlags);
        STDMETHOD(UnlockDeptViewBuffer)(UINT uViewID);

        // IDispSvrVideoMixerPlaneStereoControl
        STDMETHOD(SetStereoStreamMode)(STEREO_STREAM_MODE eMode);
        STDMETHOD(GetStereoStreamMode)(STEREO_STREAM_MODE *peMode);
        STDMETHOD(SetPlaneMetaData)(const LPBYTE pMetaData, DWORD dwSize) ;
        STDMETHOD(GetPlaneMetaData)(LPBYTE *ppMetaData, LPDWORD pdwSize);
        STDMETHOD(SetOffsetProperty)(const DispSvr::StereoOffsetProperty *pProperty);
        STDMETHOD(GetOffsetProperty)(DispSvr::StereoOffsetProperty *pProperty);

	protected:
		CD3D9VideoMixerPlane(const PlaneInit &InitOpt);
		STDMETHOD(_QueryStatusOfExternalSurface)(PLANE_ID PlaneID, const ExternalSurfaceDesc *pEx, UINT uDeptViewID = NULL);
		STDMETHOD(_SetExternalSurface)(PLANE_ID PlaneID, const ExternalSurfaceDesc *pEx, UINT uDeptViewID = NULL);
        bool IsVideoPlane() const { return m_InitOpt.PlaneID == PLANE_MAINVIDEO || m_InitOpt.PlaneID == PLANE_SUBVIDEO; };
        HRESULT PrepareDeptViewBuffer(PLANE_ID PlaneID, UINT uViewID);
        HRESULT Init();

    protected:
        IDispSvrVideoPlaneCallback *m_pPlaneCB;
        IDispSvrVideoMixer *m_pMixer;
        CD3D9VideoMixerModel *m_pModel;
        CD3D9TexturePool *m_pTexturePool;
		LONG m_cRef;
		PlaneInit m_InitOpt;
		bool m_bExternalSurface;
	};
}

interface IDispSvrVideoPlaneCallback : public IUnknown
{
    virtual const DispSvr::D3D9PlaneConfig &GetPlaneConfig(DispSvr::PLANE_ID PlaneID) = 0;
    STDMETHOD_(void, OnVideoPositionChange)() = 0;
	STDMETHOD_(void, OnDestroyPlane)(DispSvr::PLANE_ID PlaneID) = 0;
	STDMETHOD(OnCreatePlane)(const DispSvr::PlaneInit &pInit) = 0;
};

#endif  // _DISPSVR_D3D9_VIDEO_PLANE_H_