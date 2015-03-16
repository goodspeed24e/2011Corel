#ifndef _HARDWARE_DECODE_SERVICE_DXVA2_H_
#define _HARDWARE_DECODE_SERVICE_DXVA2_H_

#include "HVDServiceBase.h"
#include "HVDFastReadback.h"

typedef HRESULT  (__stdcall *TpfnDXVA2CreateVideoService)(IDirect3DDevice9* pDD, REFIID riid, void** ppService);

namespace HVDService
{
    struct SurfaceInfo
    {
        DWORD dwWidth;
        DWORD dwHeight;
        DWORD dwMaxCount;
        DWORD dwMinCount;
        GUID* pDecoderGuids;
        DWORD nDecoderGuids;
        DWORD dwPrefFourcc;
        BOOL bRestrictedContent;		
        DWORD dwReserved[24];
    };

    class CSurfacePool
    {
    public:
        CSurfacePool(IDirectXVideoDecoderService* pVidDecService);
        virtual ~CSurfacePool();

    public:
        HRESULT Allocate(SurfaceInfo* pSurfInfo);
        HRESULT Release();
        HRESULT GetSurface(DWORD dwIndex, IDirect3DSurface9** ppSurface);
        DWORD GetCount() { return m_dwSurfaceCount; }
        GUID GetDecodeGUID() {return m_DecoderGuid; }
        D3DFORMAT GetFormat() {return m_D3DFormat; }
        IDirect3DSurface9** GetSurfaces() { return m_ppSurfaces; }

    protected:
        HRESULT _Allocate(DWORD dwWidth, DWORD dwHeight, UINT nSurface, D3DFORMAT D3DFormat, BOOL bRestrictedContent);

    protected:
        IDirectXVideoDecoderService* m_pVidDecService;
        IDirect3DSurface9** m_ppSurfaces;
        DWORD m_dwSurfaceCount;
        D3DFORMAT m_D3DFormat;
        GUID m_DecoderGuid;
    };

    class CHVDServiceDxva2 : 
        public CHVDServiceBase,
        public IHVDServiceDxva2, 
        public IHVDServiceFastReadBack
    {
    public:
        CHVDServiceDxva2();
        virtual ~CHVDServiceDxva2();

    public:
        // IUnkonwn
        STDMETHOD(QueryInterface)(REFIID riid, void **ppv);
        STDMETHOD_(ULONG, AddRef)();
        STDMETHOD_(ULONG, Release)();

        // CHVDServiceBase
        HRESULT _Initialize(HVDInitConfig* pInitConfig);
        HRESULT _Uninitialize();
        HRESULT _StartService();
        HRESULT _StopService();

        // IHVDServiceDxva
        STDMETHOD(LockCompressBuffer)(DWORD dwType, DWORD dwIndex, HVDDxvaCompBufLockInfo *pInfo, BOOL bReadOnly=FALSE);
        STDMETHOD(UnlockCompressBuffer)(DWORD dwType, DWORD dwIndex);	
        STDMETHOD(BeginFrame)(DWORD dwDstSurfIndex);
        STDMETHOD(Execute)(HVDDxvaExecuteConfig* pExecuteConfig);
        STDMETHOD(EndFrame)(DWORD dwDstSurfIndex);
        STDMETHOD(GetUncompressedBufferCount)(DWORD *pdwCount);
        STDMETHOD(GetUncompresesdBufferFormat)(DWORD* pdwFourCC);
        STDMETHOD(GetAccel)(IUnknown** ppAccel);
        STDMETHOD(GetDxvaConfigPictureDecode)(HVDDXVAConfigPictureDecode* pDxvaConfigPictureDecode);

        // IHVDServiceDxva2
        STDMETHOD(GetUncompressedBuffer)(DWORD dwIndex, IDirect3DSurface9** ppSurface);
        STDMETHOD(LockUncompressedBuffer)(DWORD dwIndex, HVDDxva2UncompBufLockInfo* pInfo);
        STDMETHOD(UnlockUncompressedBuffer)(DWORD dwIndex);
        STDMETHOD(SetCP)(LPVOID pIviCP);

        // FastReadback
        STDMETHOD(FastReadback)(DWORD dwIndex, HVDDxva2UncompBufLockInfo* pInfo, HVDFastReadbackOption* pReadbackOption);

        // IHVDServiceFastReadBack
        STDMETHOD(SetParameter)(DWORD dwParamType, void* pvdParam, DWORD* pdwParamSize);
        STDMETHOD(GetParameter)(DWORD dwParamType, void* pvdParam, DWORD* pdwParamSize);

    protected:
        HRESULT CreateVideoService(IUnknown* pDevice);
        HRESULT ReleaseVideoService();
        HRESULT CreateSurfaces(SurfaceInfo* pSurfInfo);
        HRESULT ReleaseSurfaces();
        HRESULT CreateDxva2Decoder();

    protected:
        HMODULE m_hDXVA2;
        TpfnDXVA2CreateVideoService m_pfnDXVA2CreateVideoService;
        IDirectXVideoDecoderService* m_pVidDecService;
        IDirectXVideoDecoder* m_pVidDec;	
        IDirect3DDeviceManager9* m_pDeviceManager; // shared usage - Dshow or MF.
        IDirect3DDevice9* m_pDevice;
        IMFGetService* m_pMFGetService;
        HANDLE m_hDevice;
        CSurfacePool* m_pSurfacePool;
        //IDirectXVideoMemoryConfiguration* m_pDxVideoMemConfig

        DXVA2_ConfigPictureDecode m_Dxva2ConfigPictureDecode;
        ICPService *m_pCP;

        // FastReadback
        HWND m_hWnd;
        CFastReadback* m_pFastReadback;
        DWORD m_dwLockSurfaceFirst;
    };
}
#endif
