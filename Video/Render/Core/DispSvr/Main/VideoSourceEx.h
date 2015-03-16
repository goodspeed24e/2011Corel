#pragma once

#include "VideoSourcePropertyExtension.h"
#include <vmr9.h>

class CVideoSourceEx :
	public CUnknown,
	public IDisplayVideoSource,
	public IVMRSurfaceAllocator9,
	public IVMRImagePresenter9,
	public IDisplayEventHandler,
	public IVMRWindowlessControl9,
	public IDisplayVideoSourcePropertyExtension
{
public:

	CVideoSourceEx(IDisplayLock* pLock, IDisplayVideoSink* pVideoSink);
	virtual ~CVideoSourceEx();

	DECLARE_IUNKNOWN
	STDMETHODIMP NonDelegatingQueryInterface(REFIID, void**);

	// IDisplayVideoSource implementation
	STDMETHOD(GetGraph)(IFilterGraph** ppGraph);
	STDMETHOD(GetTexture)(IUnknown** ppTexture, NORMALIZEDRECT* lpNormRect);
	STDMETHOD(GetVideoSize)(LONG* plWidth, LONG* plHeight, float *pfAspectRatio);
	STDMETHOD(Attach)(IBaseFilter* pVMRFilter);
	STDMETHOD(Detach)();
	STDMETHOD(BeginDraw)();
	STDMETHOD(EndDraw)();
	STDMETHOD(IsValid)();
	STDMETHOD(ClearImage)();
	STDMETHOD(BeginDeviceLoss)();
	STDMETHOD(EndDeviceLoss)(IUnknown* pDevice);
	STDMETHOD(EnableInitiativeDisplay)(BOOL bEnable);

	// IVMRSurfaceAllocator9 implementation
	STDMETHOD(AdviseNotify)(IVMRSurfaceAllocatorNotify9* lpIVMRSurfAllocNotify);
	STDMETHOD(GetSurface)(DWORD_PTR dwUserID, DWORD SurfaceIndex, DWORD SurfaceFlags, IDirect3DSurface9** lplpSurface);
	STDMETHOD(InitializeDevice)(DWORD_PTR dwUserID, VMR9AllocationInfo* lpAllocInfo, DWORD* lpNumBuffers);
	STDMETHOD(TerminateDevice)(DWORD_PTR dwUserID);

	// IVMRImagePresenter9 implementation
	STDMETHOD(StartPresenting)(DWORD_PTR dwUserID);
	STDMETHOD(StopPresenting)(DWORD_PTR dwUserID);
	STDMETHOD(PresentImage)(DWORD_PTR dwUserID, VMR9PresentationInfo* lpPresInfo);

	// IDisplayEventHandler implementation
	STDMETHOD(NotifyEvent)(DWORD event, DWORD param1, DWORD param2, LPVOID pInstance);

	// IVMRWindowlessControl9 methods
	STDMETHOD(DisplayModeChanged)() { return E_NOTIMPL; }
	STDMETHOD(GetMaxIdealVideoSize)(LONG*  lpWidth, LONG*  lpHeight);
	STDMETHOD(GetMinIdealVideoSize)(LONG*  lpWidth, LONG*  lpHeight) { return E_NOTIMPL; }
	STDMETHOD(GetNativeVideoSize)(LONG*  lpWidth, LONG*  lpHeight, LONG*  lpARWidth, LONG*  lpARHeight);
	STDMETHOD(SetVideoPosition)(const LPRECT  lpSRCRect, const LPRECT  lpDSTRect);
	STDMETHOD(GetVideoPosition)(LPRECT  lpSRCRect, LPRECT  lpDSTRect);
	STDMETHOD(SetAspectRatioMode)(DWORD  AspectRatioMode);
	STDMETHOD(GetAspectRatioMode)(DWORD*  lpAspectRatioMode);
	STDMETHOD(RepaintVideo)(HWND  hwnd, HDC  hdc) { return E_NOTIMPL; }
	STDMETHOD(SetBorderColor)(COLORREF  Clr) { return E_NOTIMPL; }
	STDMETHOD(GetBorderColor)(COLORREF*  lpClr) { return E_NOTIMPL; }
	STDMETHOD(GetCurrentImage)(BYTE**  lpDib);
	STDMETHOD(SetVideoClippingWindow)(HWND  hwnd);

	// IDisplayVideoSourcePropertyExtension methods
	STDMETHOD(GetSampleProperty)(SampleProperty *pProperty);

	HRESULT VerifyID(DWORD_PTR dwID) const
	{
		if (dwID != GetID())
		{
			DbgMsg(("Failed to VerifyID()!"));
			return VFW_E_NOT_FOUND;
		}
		return S_OK;
	}
	HRESULT GetID() const { return (DWORD_PTR) this; }
	

private:
	void Cleanup();
	void DeleteSurfaces();
	HRESULT DisconnectPins();
	HRESULT AllocateSurfaceBuffer(DWORD dwN);
	HRESULT ConfigureVMR(IBaseFilter* pVMR);
	BOOL IsFilterStopped();
    HRESULT CheckMaxDisplayFrameRate();
    inline BOOL IsOverDisplayFrameRate(LONGLONG hnsDuration);

private:
	DWORD m_dwNumBuf;
	DWORD m_dwNumBufActuallyAllocated;

	LONG m_lImageWidth;
	LONG m_lImageHeight;
	FLOAT m_fAspectRatio;
	FLOAT m_fNativeAspectRatio;

	IDirect3DSurface9** m_ppSurface;

	CComPtr<IFilterGraph> m_pGraph;
	CComPtr<IVMRSurfaceAllocatorNotify9> m_pDefaultNotify;
	CComPtr<IBaseFilter> m_pVMR;
	CComPtr<IDisplayVideoSink> m_pVideoSink;
	CComPtr<IDisplayLock> m_pLock;

	BOOL m_bValid;
	bool m_bTextureSurface;
	BOOL m_bInitiativeDisplay;
	bool m_bImageAvailable;

	HWND m_hwndVideo;
	SIZE m_szNativeAspectRatio;
	DWORD  m_dwAspectRatioMode;
	RECT m_dstrect;
	volatile BOOL m_bStopState;
	REFERENCE_TIME m_rtStart;
	REFERENCE_TIME m_rtEnd;

    DWORD m_dwMaxDisplayFrameRate;
    REFERENCE_TIME m_rtLaststart;
    bool m_bFirstTimeDisp;
};
