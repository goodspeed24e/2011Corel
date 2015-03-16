#include "HVDServiceBase.h"
#include "Imports\ThirdParty\Intel\Inc\HVDIntelFastReadback.h"
#include "Imports\ThirdParty\Intel\Inc\FastCompositingDevice.h"
#include "Imports\ThirdParty\AMD\Inc\amdmcom.h"
#include "Imports\ThirdParty\AMD\Inc\amdmcompriv.h"
#include "Imports\ThirdParty\NVidia\Inc\nvapi.h"
#include "Imports\Inc\msdk.h"
#include <atlbase.h>

typedef HRESULT  (__stdcall *TpfnDXVA2CreateVideoService)(IDirect3DDevice9* pDD, REFIID riid, void** ppService);

namespace HVDService
{
    struct FastReadbackInit
    {
        DWORD dwSupportedDecoderCount;
        GUID* pSupportedDecoderGuids;
        DWORD dwSurfaceCount;
        HMODULE hDXVA2;
        IDirectXVideoDecoder* pVidDec;
        IDirect3DDevice9* pDevice;
        UINT uBufHeight;
        UINT uBufWidth;
        D3DFORMAT d3dSurfaceFormat;
        HWND hWnd;
        IDirect3DSurface9** ppSurfaces;
        UINT uSrcHeight;
        UINT uSrcWidth;
        DWORD dwInputInterlaceFlag;
        DWORD dwOutputInterlaceFlag;
        DWORD dwExpectedReadbackWidth;
        DWORD dwExpectedReadbackHeight;
        DWORD dwLockSurfaceFirst;
    };

    enum{
        FAST_READBACK_UNKNOWN = 0,
        FAST_READBACK_D3D,
        FAST_READBACK_NV,
        FAST_READBACK_AMD,
        FAST_READBACK_INTEL,
    };

    enum
    {
        E_CAPS_TYPE_SCALING = 0,
    };

    class CFastReadback
    {
    public:
        CFastReadback();
        virtual ~CFastReadback();
        virtual int GetID() = 0;

    public:
        virtual HRESULT Open(FastReadbackInit *pReadbackInit);
        virtual HRESULT Readback(IDirect3DSurface9* pSurface, DWORD dwIndex, HVDService::HVDDxva2UncompBufLockInfo* pInfo, HVDService::HVDFastReadbackOption* pReadbackOption);
        virtual HRESULT Close();
        virtual HRESULT GetCaps(DWORD dwCapType, void* pvdCaps);
        virtual void SetPitchCopy(BOOL bUsePitchCopy=TRUE) {m_bUsePicthCopy = bUsePitchCopy;}
        virtual HRESULT InstallMediaAllocator(void* pAllocater) { return S_OK;}
        virtual HRESULT SetDisplayArea(DWORD dwWitdh, DWORD dwHeight) { return S_OK;}
        virtual void EnableLockSource() { return; }

    protected:
        CCritSec m_csObj;

        // CSurfacePool
        UINT m_uBufHeight;
        UINT m_uBufWidth;
        UINT m_uSrcHeight;
        UINT m_uSrcWidth;
        UINT m_uExpectedWidth;  // for demand setting, do not need 16 align.
        UINT m_uExpectedHeight; // for demand setting, do not need 16 align.
        D3DFORMAT m_d3dSurfaceFormat;

        //HVDServiceDXVA2
        IDirect3DDevice9* m_pDevice;

        //General flag setting;
        BOOL m_bUsePicthCopy;
    };

    class CD3DFastReadback : public CFastReadback
    {
    public:
        CD3DFastReadback();
        ~CD3DFastReadback();
        virtual int GetID() {return FAST_READBACK_D3D;}

    public:
        HRESULT Open(FastReadbackInit *pReadbackInit);
        HRESULT Readback(IDirect3DSurface9* pSurface, DWORD dwIndex, HVDService::HVDDxva2UncompBufLockInfo* pInfo, HVDService::HVDFastReadbackOption* pReadbackOption);
        HRESULT Close();

    protected:
        CComPtr<IDirect3DSurface9> m_pRenderSurface;
        CComPtr<IDirect3DTexture9> m_pTexturePriv;
        CComPtr<IDirect3DSurface9> m_pTexturePrivSurf;
    };

    class CNVFastReadback: public CFastReadback
    {
    public:
        CNVFastReadback();
        ~CNVFastReadback();
        virtual int GetID() {return FAST_READBACK_NV;}

    public:
        HRESULT Open(FastReadbackInit *pReadbackInit);
        HRESULT Readback(IDirect3DSurface9* pSurface, DWORD dwIndex, HVDService::HVDDxva2UncompBufLockInfo* pInfo, HVDService::HVDFastReadbackOption* pReadbackOption);
        HRESULT Close();

    protected:
        BYTE* pSysMem, *pVideoStartAddress;
        NVDX_ObjectHandle m_hRenderTarget;
        NVAPI_D3D9_DMA_DESCRIBE_PARAMS m_SysBufInfo;
    };

    class CIntelFastReadback : public CFastReadback
    {
    public:
        CIntelFastReadback();
        ~CIntelFastReadback();
        virtual int GetID() {return FAST_READBACK_INTEL;}

    public:
        HRESULT Open(FastReadbackInit *pReadbackInit);
        HRESULT Readback(IDirect3DSurface9* pSurface, DWORD dwIndex, HVDService::HVDDxva2UncompBufLockInfo* pInfo, HVDService::HVDFastReadbackOption* pReadbackOption);
        HRESULT Close();

    protected:
        HRESULT Intel_Init();
        HRESULT CreateAuxiliaryDevice();
        HRESULT CreateFastCopyDevice();
        HRESULT QueryFastcopy();		
        HRESULT QueryAccelRTFormats(D3DFORMAT **ppFormats, UINT *puCount);
        HRESULT Intel_CreateService ();
        HRESULT CreateAccelService (CONST GUID *pAccelGuid, void *pCreateParams, UINT *puCreateParamSize);	
        HRESULT QueryAccelCaps     (void *pCaps, UINT *puCapSize);
        HRESULT FastCopyRegister(int count, IDirect3DSurface9 **ppSurface);
        HRESULT FastCopyRegister(int count, IDirect3DSurface9 *pSurface, ...);
        HRESULT FastCopyBlt(IDirect3DSurface9 *pSrc,IDirect3DSurface9 *pDst);
        HRESULT CreateRenderTarget(D3DFORMAT fourcc=(D3DFORMAT)MAKEFOURCC('N','V','1','2'));
        HRESULT InstallMediaAllocator(void* pAllocater);
        HRESULT SetDisplayArea(DWORD dwWitdh, DWORD dwHeight);
        void    EnableLockSource() { m_bLockSourceFirst=TRUE; }

    protected:
        //HVDServiceBase
        DWORD m_dwSupportedDecoderCount;
        GUID* m_pSupportedDecoderGuids;
        DWORD m_dwSurfaceCount;
        DWORD m_dwReadbackCounter;

        //HVDServiceDXVA2
        HMODULE m_hDXVA2;

        //SurfacePool
        IDirect3DSurface9** m_ppSurfaces;

        TpfnDXVA2CreateVideoService m_pfnAuxDXVA2CreateVideoService;
        TpfnDXVA2CreateVideoService m_pfnRegDXVA2CreateVideoService;
        IDirectXVideoDecoderService* m_pAuxVidDecService;
        IDirectXVideoDecoder* m_pAuxVidDec;
        IDirectXVideoProcessorService* m_pRegVidProService;
        IDirectXVideoProcessor* m_pRegVidPro;	
        IDirect3DSurface9 *m_pDummySurface;
        DXVA2_ConfigPictureDecode m_AuxDxva2ConfigPictureDecode;
        HANDLE m_hRegistration;
        D3DFORMAT *m_pDstFormats;
        UINT	   m_uDstFormatCount;
        IDirect3DSurface9 *m_pDestSurface[DST_COUNT] ;	// destination in system memory
        CFastCompositingDevice *m_pComp;
        BOOL			m_DstInSystem;
        FASTCOMP_CAPS   *m_pCaps;
        FASTCOMP_CAPS2   *m_pCaps2;

#if defined DELAYACCEL
        DWORD m_dwWriteframe;
        IDirect3DSurface9 *m_pSourceSurface[SRC_COUNT]     ;	// source in video memory
        UINT m_uRecordFrameIndex[DelayPeriod];
#endif
        BYTE* m_pAllocator;
        DWORD m_dwDisplayWidth;
        DWORD m_dwDisplayHeight;
        BOOL m_bLockSourceFirst;
    };

    class CATIFastReadback :  public CFastReadback
    {
    public:
        CATIFastReadback();
        ~CATIFastReadback();
        virtual int GetID() {return FAST_READBACK_AMD;}

    public:
        HRESULT Open(FastReadbackInit *pReadbackInit);
        HRESULT Readback(IDirect3DSurface9* pSurface, DWORD dwIndex, HVDService::HVDDxva2UncompBufLockInfo* pInfo, HVDService::HVDFastReadbackOption* pReadbackOption);
        HRESULT Close();
        HRESULT GetCaps(DWORD dwCapType, void* pvdCaps);

    protected:
        //HVDServiceDXVA2
        IDirectXVideoDecoder* m_pVidDec;
        HWND m_pRenderWnd;

        // Scaling
        BOOL m_bUseScaling ;
        BOOL m_bMcomScalingSupported;
        UINT m_uScaledWidth;
        UINT m_uScaledHeight;
        UINT m_uScaledPitch;

        void* m_pMcomSession;
        BOOL m_bMcomDecodeTargetAccessSupport;

        MCOM_GET_DECODE_TARGET_ACCESS_CAPS_OUTPUT m_mcomDecodeTargetAccessCapsOutput; 
        MCOM_BEGIN_DECODE_TARGET_ACCESS_INPUT m_mcomBeginDecodeTargetAccessInput; 
        MCOM_BEGIN_DECODE_TARGET_ACCESS_OUTPUT m_mcomBeginDecodeTargetAccessOutput; 
    };
}