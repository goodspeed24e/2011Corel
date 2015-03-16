#pragma once

#include <vmr9.h>

class CVideoSource :
    public CUnknown,
    public IDisplayVideoSource,
    public IVMRSurfaceAllocator9,
    public IVMRImagePresenter9,
    public IVMRImageCompositor9
{
public:

    CVideoSource(IDisplayLock* pLock, IDisplayVideoSink* pVideoSink);
    virtual ~CVideoSource();

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

    // IVMRImageCompositor9 implementation
    STDMETHOD(InitCompositionDevice)(IUnknown *pD3DDevice);
    STDMETHOD(TermCompositionDevice)(IUnknown *pD3DDevice);
    STDMETHOD(SetStreamMediaType)(DWORD dwStrmID, AM_MEDIA_TYPE *pmt, BOOL fTexture);
    STDMETHOD(CompositeImage)(IUnknown *pD3DDevice, IDirect3DSurface9 *pddsRenderTarget,
        AM_MEDIA_TYPE *pmtRenderTarget, REFERENCE_TIME rtStart, REFERENCE_TIME rtEnd,
        D3DCOLOR dwClrBkGnd, VMR9VideoStreamInfo *pVideoStreamInfo, UINT cStreams);

    HRESULT VerifyID(DWORD_PTR dwID)
    {
        if (dwID != GetID())
        {
            return VFW_E_NOT_FOUND;
        }
        return S_OK;
    }

    DWORD_PTR GetID()
    {
        return (DWORD_PTR) this;
    }

private:
	void Cleanup();
	void DeleteSurfaces();
	HRESULT DisconnectPins();
	HRESULT AllocateSurfaceBuffer(DWORD dwN);
	HRESULT PrepareCompositorTexture(IDirect3DDevice9* pDevice, DWORD width, DWORD height);
	HRESULT ConfigureVMR(IBaseFilter* pVMR);

private:
    DWORD m_dwNumBuf;
    DWORD m_dwNumBufActuallyAllocated;

    LONG m_lImageWidth;
    LONG m_lImageHeight;
	FLOAT m_fApsectRatio;
 	NORMALIZEDRECT m_nrcTexture;
   
    IDirect3DSurface9** m_ppSurface;

    CComPtr<IDirect3DTexture9> m_pTexturePriv;
	CComPtr<IDirect3DSurface9> m_pTexturePrivSurf;
    CComPtr<IFilterGraph> m_pGraph;
    CComPtr<IVMRSurfaceAllocatorNotify9> m_pDefaultNotify;
    CComPtr<IBaseFilter> m_pVMR;
    CComPtr<IDisplayVideoSink> m_pVideoSink;
    CComPtr<IDisplayLock> m_pLock;
    CComPtr<IDirect3DTexture9> m_pTextureComp;

	BOOL m_bValid;
	bool m_bTextureSurface;
	bool m_bDecimateBy2;
	BOOL m_bInitiativeDisplay;
};
