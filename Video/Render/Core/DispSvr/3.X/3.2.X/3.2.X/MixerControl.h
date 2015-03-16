#pragma once
#include "VideoSource.h"

class CVideoSourceManager;
class CVideoFrame;
typedef std::vector<CVideoFrame *> VideoFrames;

class CVideoMixer :
            public DispSvr::CObjectCounter<CVideoMixer>,
            public CUnknown,
            public IDisplayObject,
            public IDisplayVideoMixer,
            public IDisplayVideoSink,
			public IDisplayOptimizedRender,
            public IDisplayProperties,
            public IDisplayLock
{
public:
	
    CVideoMixer(LPUNKNOWN pUnk, HRESULT *phr);
    virtual ~CVideoMixer();

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
    STDMETHOD(EndDeviceLoss)( IUnknown* pDevice );
	STDMETHOD(GetCLSID)(CLSID* pClsid);
	STDMETHOD(NotifyEvent)(DWORD event, DWORD param1, DWORD param2, LPVOID pInstance);

	// IDisplayOptimizedRender implementation
	STDMETHOD(MultiTextureRender)(IUnknown *pDevice, LONG lStageIdx, LONG lTexIdx, const NORMALIZEDRECT* lpParentRect);
	STDMETHOD(GetTextureCount)(LONG* plCount);
	STDMETHOD(GetTextureCoord)(LONG lIndex, NORMALIZEDRECT* lpNormRect);

    // IDisplayLock implementation
    STDMETHOD(Lock)();
    STDMETHOD(Unlock)();
	STDMETHOD(TryLock)();

	// IDisplayProperties
	STDMETHOD(GetOutputRect)(NORMALIZEDRECT* lpNormRect);
	STDMETHOD(SetOutputRect)(NORMALIZEDRECT* lpNormRect);
	STDMETHOD(GetCropRect)(NORMALIZEDRECT* lpNormRect);
	STDMETHOD(SetCropRect)(NORMALIZEDRECT* lpNormRect);						
	STDMETHOD(GetVertexBufferRect)(NORMALIZEDRECT* lpNormRect)				{ return E_NOTIMPL; }
	STDMETHOD(GetFrameColor)(COLORREF* pColor);
	STDMETHOD(SetFrameColor)(COLORREF color);
	STDMETHOD(CaptureFrame)(DWORD dwFormat, BYTE** ppFrame, UINT* pSize);
	STDMETHOD(GetZoom)(NORMALIZEDRECT* lpNormRect);
	STDMETHOD(SetZoom)(NORMALIZEDRECT* lpNormRect);
	STDMETHOD(SetShow)(BOOL bShow);	
	STDMETHOD(GetShow)(BOOL* bShow);
	STDMETHOD(ClearFrame)();
	STDMETHOD(ClientToDO)(POINT* pPt){return E_NOTIMPL;}
	STDMETHOD(SetDispFrameRate)(DWORD dwFrameRate)					{return E_NOTIMPL;}

    // private methods
private:

	CVideoFrame* GetVideoFrame(IDisplayVideoSource* pVidSrc);
	void BackupFrameState();
	
    // private data
private:

    CCritSec m_csObj; // this object has to be thread-safe
    
    VideoFrames m_listFrames;
	VideoFrames m_listBackup;	/// Remember current frame state when device lost.
    CComPtr<IDisplayRenderEngine> m_pOwner;
	D3DXCOLOR				m_cAlpha;
	BOOL					m_bOptimizedRenderSupport;
	BOOL					m_bShow;
	BOOL					m_bLockRatio;

    CVideoSourceManager*	m_pSourceMgr;

#ifdef _DEBUG
    CComPtr<IDisplayVideoSource> m_pMovingVidSrc;
	float m_xDrag, m_yDrag;
#endif
};
