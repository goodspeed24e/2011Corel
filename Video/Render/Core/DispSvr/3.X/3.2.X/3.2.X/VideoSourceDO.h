#pragma once
#include "stdafx.h"

interface IDispSvrVideoMixerVideoPlane;
interface IDisplayVideoSourcePropertyExtension;

enum VIDEOSOURCEDO_REQUEST
{
    VIDEOSOURCEDO_REQUEST_QUERY_SURFACE_STATUS = WM_APP + 1,
    VIDEOSOURCEDO_REQUEST_RELEASE_SURFACE,
	VIDEOSOURCEDO_REQUEST_UPDATE_SURFACE
};

/// CVideoSourceDO is designed to support IDispSvrVideoMixer and only one video source (main video cases)
/// and CVideoSourceDO is currently a singleton the same with IDispSvrVideoMixer.
class CVideoSourceDO :
    public DispSvr::CObjectCounter<CVideoSourceDO>,
	public CUnknown,
	public IDisplayObject,
	public IDisplayVideoMixer,
	public IDisplayVideoSink,
	public IDisplayProperties,
	public IDisplayLock,
	public IDispSvrVideoMixerEventListener,
    public IDisplayStereoControl
{
public:

	CVideoSourceDO(LPUNKNOWN pUnk, HRESULT *phr);
    virtual ~CVideoSourceDO();

    // IUnknown implementation
    DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID, void**);

    // IDisplayVideoMixer implementation
    STDMETHOD(GetOutputRect)(IDisplayVideoSource* pVidSrc, NORMALIZEDRECT* lpNormRect);
    STDMETHOD(GetIdealOutputRect)(IDisplayVideoSource* pVidSrc, DWORD dwWidth, DWORD dwHeight, NORMALIZEDRECT* lpNormRect);
    STDMETHOD(SetOutputRect)(IDisplayVideoSource* pVidSrc, NORMALIZEDRECT* lpNormRect);
    STDMETHOD(GetZOrder)(IDisplayVideoSource* pVidSrc, DWORD *pdwZ);
    STDMETHOD(SetZOrder)(IDisplayVideoSource* pVidSrc, DWORD pdwZ);
    STDMETHOD(GetAlpha)(IDisplayVideoSource* pVidSrc, float* pAlpha);
    STDMETHOD(SetAlpha)(IDisplayVideoSource* pVidSrc, float Alpha);
    STDMETHOD(AddVideoSource)(IBaseFilter* pVMRFilter, IDisplayVideoSource** ppVidSrc);
    STDMETHOD(RemoveVideoSource)(IDisplayVideoSource* pVidSrc);
	STDMETHOD(GetVideoSourceCount)(LONG* plCount);
	STDMETHOD(GetVideoSourceByIndex)(LONG lIndex, IDisplayVideoSource** ppVideoSource);
	STDMETHOD(KeepAspectRatio)(IDisplayVideoSource* pVidSrc, BOOL bLockRatio);

    // IDisplayVideoSink
    STDMETHOD(OnVideoSourceAdded)(IDisplayVideoSource* pVidSrc, FLOAT fAspectRatio);
    STDMETHOD(OnVideoSourceRemoved)(IDisplayVideoSource* pVidSrc);
    STDMETHOD(Get3DDevice)(IUnknown** ppDevice);

    // IDisplayObject implementation
    STDMETHOD(Initialize)(IDisplayRenderEngine* pRenderEngine);
    STDMETHOD(GetRenderEngineOwner)(IDisplayRenderEngine** ppRenderEngine);
    STDMETHOD(Terminate)();
    STDMETHOD(ProcessMessage)(HWND hWnd, UINT msg, UINT wParam, LONG lParam);
    STDMETHOD(Render)(IUnknown *pDevice, const NORMALIZEDRECT* lpParentRect);
    STDMETHOD(BeginDeviceLoss)(void);
    STDMETHOD(EndDeviceLoss)( IUnknown* pDevice);
	STDMETHOD(GetCLSID)(CLSID* pClsid);
	STDMETHOD(NotifyEvent)(DWORD event, DWORD param1, DWORD param2, LPVOID pInstance);

    // IDisplayLock implementation
    STDMETHOD(Lock)();
    STDMETHOD(Unlock)();
	STDMETHOD(TryLock)();

	// IDisplayProperties
	STDMETHOD(GetOutputRect)(NORMALIZEDRECT* lpNormRect);
	STDMETHOD(SetOutputRect)(NORMALIZEDRECT* lpNormRect);
	STDMETHOD(GetCropRect)(NORMALIZEDRECT* lpNormRect);
	STDMETHOD(SetCropRect)(NORMALIZEDRECT* lpNormRect);
	STDMETHOD(GetVertexBufferRect)(NORMALIZEDRECT* lpNormRect)	{ return E_NOTIMPL; }
	STDMETHOD(GetFrameColor)(COLORREF* pColor);
	STDMETHOD(SetFrameColor)(COLORREF color);
	STDMETHOD(CaptureFrame)(DWORD dwFormat, BYTE** ppFrame, UINT* pSize);
	STDMETHOD(GetZoom)(NORMALIZEDRECT* lpNormRect);
	STDMETHOD(SetZoom)(NORMALIZEDRECT* lpNormRect);
	STDMETHOD(SetShow)(BOOL bShow);	
	STDMETHOD(GetShow)(BOOL* bShow);
	STDMETHOD(ClearFrame)();
	STDMETHOD(ClientToDO)(POINT* pPt){return E_NOTIMPL;}
	STDMETHOD(SetDispFrameRate)(DWORD dwFrameRate);

	// IDispSvrVideoMixerEventListener
	STDMETHOD(Notify)(DWORD dwEvent, DWORD dwParam1, DWORD dwParam2);

    // public IDisplayStereoControl
    STDMETHOD(SetStereoDisplayMode)(DWORD stereoDispProp);
    STDMETHOD(GetStereoDisplayMode)(DWORD *stereoDispProp);

private:
	HRESULT UpdateSurfaceFromVideoSource();
	void UpdatePosition();
	void UnregisterSurface();
    void EnableFrameRateConversion(BOOL bEnable, DWORD dwNumerator = 60000, DWORD dwDenominator = 1001);

private:

	CCritSec m_csObj; // this object has to be thread-safe
        CCritSec m_csObjForRender; // critical section for render request/terminate renderengine
	IDisplayVideoSource *m_pVideoSource;
	CComPtr<IDispSvrVideoMixer> m_pVideoMixer;
	CComPtr<IDisplayRenderEngine> m_pOwner;
	CComPtr<IDispSvrVideoMixerVideoPlane> m_pVideoPlane;
	CComPtr<IDisplayVideoSourcePropertyExtension> m_pVSProperty;
	DispSvr::ExternalSurfaceDesc m_ExternalSurfaceDesc;
	NORMALIZEDRECT m_nrcOutput;
	NORMALIZEDRECT m_nrcCrop;
	NORMALIZEDRECT m_nrcZoom;
	FLOAT m_fAspectRatio;
	FLOAT m_fAlpha;
	RECT m_rcSrc;
	bool m_bLockRatio;
	bool m_bShow;
	bool m_bNeedUpdatePosition;
	bool m_bEnableRendering;
	bool m_bPullSurfaceFromVideoSource;
    StereoDisplayProperties m_eStereoDispMode;
	UINT m_uiVideoWidth;
	UINT m_uiVideoHeight;
	DWORD m_dwFrameRate1000;
};
