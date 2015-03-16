#include "stdafx.h"
#include <windows.h>
#include <streams.h>
#include <d3d9.h>
#include <dxva.h>
#include <dxva2api.h>
#include "Imports/LibGPU/GPUID.h"
#include "HVDFastReadback.h"
#include <string.h>

#define	DXVA2FASTCOMPOSITING 1
#define DXVALockSurfaceFirstNum 8
#define ALIGN16(X) ((((X+15)>>4)<<4))

//#define		TEST_READBACK_TIME
#ifdef TEST_READBACK_TIME
#define COMPUTE_READBACK_TIME_INIT() static DWORD total_time=0, total_count=0; DWORD atime = timeGetTime();
#define COMPUTE_READBACK_TIME(str)	{\
    total_time+=timeGetTime()-atime; total_count++;\
    if(total_count>=100)\
        {\
        char s[256];	_sprintf_p(s, sizeof(s), "HVDService: average %s readback time=%d\n", str, total_time/total_count);\
        OutputDebugStringA(s);	total_time = total_count = 0;\
        }\
    }
#else
#define COMPUTE_READBACK_TIME_INIT()
#define COMPUTE_READBACK_TIME(str)
#endif

#ifdef _DEBUG
#define DP_DBG(x)   DP x
#else
#define DP_DBG(x)   //DP x
#endif

#define DP_ERR DP 

using namespace HVDService;

#define CopyNV12(pDst0, pSrc0, width0, height0, src_pitch0)\
{\
    char* pDst=(char*)pDst0, *pSrc=(char*)pSrc0;\
    int width=(int)(width0), height=(int)(height0), src_pitch=(int)(src_pitch0);\
    int i, len=(height*3)>>1;\
    for(i=0; i<len; i++, pSrc+=src_pitch, pDst+=width)\
    memcpy(pDst, pSrc, width);\
}

CFastReadback::CFastReadback()
{
    m_uBufHeight = 0;
    m_uBufWidth = 0;
    m_uExpectedWidth = 0;
    m_uExpectedHeight = 0;
    m_d3dSurfaceFormat = D3DFMT_UNKNOWN;
    m_pDevice = NULL;
    m_bUsePicthCopy = FALSE;
}

CFastReadback::~CFastReadback()
{
}

HRESULT CFastReadback::Open(FastReadbackInit *pReadbackInit)
{
    return E_NOTIMPL;
}

HRESULT CFastReadback::Readback(IDirect3DSurface9* pSurface, DWORD dwIndex, HVDService::HVDDxva2UncompBufLockInfo* pInfo, HVDService::HVDFastReadbackOption* pReadbackOption)
{
    return E_NOTIMPL;
}

HRESULT CFastReadback::Close()
{
    return E_NOTIMPL;
}

int CFastReadback::GetID()
{
    return FAST_READBACK_UNKNOWN;
}

HRESULT CFastReadback::GetCaps(DWORD dwCapType, void* pvdCaps)
{
    return E_NOTIMPL;
}

// D3D FastReadback
CD3DFastReadback::CD3DFastReadback()
{
    m_pRenderSurface = NULL;
}

CD3DFastReadback::~CD3DFastReadback()
{
    m_pRenderSurface.Release();
}

HRESULT CD3DFastReadback::Open(FastReadbackInit *pReadbackInit)
{
    CHECK_POINTER(pReadbackInit);

    DP_DBG(("CD3DFastReadback::Open()"));

    HRESULT hr = E_FAIL;
    IDirect3D9 *pD3D9 = Direct3DCreate9(D3D_SDK_VERSION);

    m_pDevice = pReadbackInit->pDevice;
    m_uBufHeight = pReadbackInit->uBufHeight;
    m_uBufWidth = pReadbackInit->uBufWidth;
    m_d3dSurfaceFormat = pReadbackInit->d3dSurfaceFormat;

    CHECK_POINTER(m_pDevice);

    hr = pD3D9->CheckDeviceFormatConversion(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, m_d3dSurfaceFormat, D3DFMT_X8R8G8B8);// check device support format conversion
    if (FAILED(hr))
    {
        DP_FASTREADBACK("DeviceFormatConversion don't support videoFormat to D3DFMT_X8R8G8B8");
    }	

    SAFE_RELEASE(pD3D9) ;

    return hr;
}

HRESULT CD3DFastReadback::Readback(IDirect3DSurface9* pSurface, DWORD dwIndex, HVDService::HVDDxva2UncompBufLockInfo* pInfo, HVDService::HVDFastReadbackOption* pReadbackOption)
{
    CHECK_POINTER(pSurface);

    CAutoLock lock(&m_csObj);	

    HRESULT hr = E_FAIL;

    if(m_pTexturePriv==0)
        hr = m_pDevice->CreateTexture(m_uBufWidth,m_uBufHeight, 1, D3DUSAGE_RENDERTARGET, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &m_pTexturePriv, NULL);

    if(m_pTexturePrivSurf==0 && m_pTexturePriv)
        hr = m_pTexturePriv->GetSurfaceLevel(0, &m_pTexturePrivSurf);

    if (!m_pTexturePrivSurf)
        return E_POINTER;

    hr = m_pDevice->StretchRect(pSurface, NULL, m_pTexturePrivSurf, NULL, D3DTEXF_POINT);	// color space conversion (YUV to RGB)
    if(FAILED(hr))
        return hr;

    if(m_pRenderSurface==0)
        hr = m_pDevice->CreateOffscreenPlainSurface(m_uBufWidth, m_uBufHeight, D3DFMT_X8R8G8B8, D3DPOOL_SYSTEMMEM, &m_pRenderSurface, NULL);

    if(m_pRenderSurface==0 || m_pTexturePrivSurf==0)
        return E_POINTER;

    hr = m_pDevice->GetRenderTargetData(m_pTexturePrivSurf, m_pRenderSurface);	// move data from video memory to system memory
    if(FAILED(hr))
    {
        DP_FASTREADBACK("GetRenderTargetData failed");
        return hr;
    }

    pReadbackOption->wHeight = m_uBufHeight;
    pReadbackOption->wWidth = m_uBufWidth;

    if (pReadbackOption != NULL && pReadbackOption->dwReturnFourcc == FALSE)
        pReadbackOption->pReturnSurface = m_pRenderSurface;
    else
        if(pReadbackOption->pReturnSurface) // copy data into the surface
        {
            CHECK_POINTER(pInfo);
            D3DLOCKED_RECT d3dLockRect = {0};

            hr = m_pRenderSurface->LockRect(&d3dLockRect, NULL, 0);
            if(FAILED(hr))
            {
                DP_FASTREADBACK("LockRect failed");
                return hr;
            }
            pInfo->pBits = d3dLockRect.pBits;
            pInfo->uPitch = d3dLockRect.Pitch;
            pInfo->dwFourCC = D3DFMT_X8R8G8B8;

            if(pReadbackOption->dwReturnFourcc == MAKEFOURCC('N','V','1','2'))
            {
                int stride = d3dLockRect.Pitch;
                byte* pRGBARawdata = (byte*)d3dLockRect.pBits;
                unsigned char R=0, G=0, B=0;
                DWORD i=0, j=0, tempcal=0, y_height = m_uBufHeight;
                BYTE* m_pYUVData = (BYTE*)pReadbackOption->pReturnSurface;

                for (i=0 ; i < y_height ; i++)
                {
                    for(j=0 ; j < m_uBufWidth ; j++)
                    {
                        R = pRGBARawdata[j*4+2+i*stride]; G = pRGBARawdata[j*4+1+i*stride]; B = pRGBARawdata[j*4+0+i*stride];
                        //A = pRGBARawdata[j*4+3+i*stride];

                        m_pYUVData[j+i*m_uBufWidth] =  (byte)((float)0.299*R + (float)0.587*G + (float)0.114*B);
                        if ( (i%2 == 0) && (j%2 == 0)) // to NV12
                        {
                            tempcal = y_height*m_uBufWidth+j+(i>>1)*m_uBufWidth;
                            m_pYUVData[tempcal] = (byte)(128.0f-((float)0.147*R) - ((float)0.289*G) + ((float)0.436*B));//U
                            m_pYUVData[tempcal+1] =(byte)(128.0f+((float)0.615*R) - ((float)0.515*G) - ((float)0.100*B));//V
                        }
                    }				
                }
            }
            pInfo->pBits = 0;
            hr = m_pRenderSurface->UnlockRect();
        }
        else
        {
            CHECK_POINTER(pInfo);
            D3DLOCKED_RECT d3dLockRect = {0};

            hr = m_pRenderSurface->LockRect(&d3dLockRect, NULL, 0);
            if(FAILED(hr))
            {
                DP_FASTREADBACK("LockRect failed");
                return hr;
            }
            pInfo->pBits = d3dLockRect.pBits;
            pInfo->uPitch = d3dLockRect.Pitch;
            pInfo->dwFourCC = D3DFMT_X8R8G8B8;

            hr = m_pRenderSurface->UnlockRect();
        }

        return hr;
}

HRESULT CD3DFastReadback::Close()
{
    DP_DBG(("CD3DFastReadback::Close()"));

    m_pRenderSurface.Release();
    return S_OK;
}

//NVidia fast read back: Only r187 above of drivers can support this feature
CNVFastReadback::CNVFastReadback()
:CFastReadback()
{
    pSysMem = NULL;
}

CNVFastReadback::~CNVFastReadback()
{
    if(pSysMem)
        delete[]pSysMem;
    pSysMem = 0;
}

HRESULT CNVFastReadback::Open(FastReadbackInit *pReadbackInit)
{
    DP_DBG(("CNVFastReadback::Open()"));

    HRESULT hr;

    m_pDevice = pReadbackInit->pDevice;
    m_uBufHeight = pReadbackInit->uBufHeight;
    m_uBufWidth = pReadbackInit->uBufWidth;
    m_d3dSurfaceFormat = pReadbackInit->d3dSurfaceFormat;

    if(NVAPI_OK!=NvAPI_Initialize())
        return E_FAIL;

    NVAPI_D3D9_DMA_PARAMS p1; memset(&p1, 0, sizeof(p1));
    p1.version	= NVAPI_D3D9_DMA_PARAMS_VER;
    p1.dwCommand = NVAPI_D3D9_DMA_CMD_DESCRIBE;
    p1.DescribeParams.dwWidth = m_uBufWidth;
    p1.DescribeParams.dwHeight = m_uBufHeight;
    p1.DescribeParams.dwFormat = MAKEFOURCC('N','V','1','2');
    hr = NvAPI_D3D9_DMA(m_pDevice, &p1);
    if(hr!=S_OK)
        return hr;
    m_SysBufInfo = p1.DescribeParams;

    pSysMem = new BYTE[p1.DescribeParams.dwSize+p1.DescribeParams.dwAlignment];
    pVideoStartAddress = (BYTE*)(((DWORD)pSysMem+p1.DescribeParams.dwAlignment)&(~(p1.DescribeParams.dwAlignment-1)));

    NVAPI_D3D9_DMA_PARAMS p2; memset(&p2, 0, sizeof(p2));
    p2.version	= NVAPI_D3D9_DMA_PARAMS_VER;
    p2.dwCommand = NVAPI_D3D9_DMA_CMD_MAP;
    p2.MapParams.dwFormat	= MAKEFOURCC('N','V','1','2');
    p2.MapParams.dwWidth	= m_uBufWidth;
    p2.MapParams.dwHeight	= m_uBufHeight;
    p2.MapParams.pMemory	= pVideoStartAddress;
    hr = NvAPI_D3D9_DMA(m_pDevice, &p2);

    if(hr!=S_OK)
    {
        if(pSysMem)
            delete[]pSysMem;
        pSysMem = 0;
        return hr;
    }
    m_hRenderTarget = p2.MapParams.hSysmemSurface;
    return S_OK;
}


HRESULT CNVFastReadback::Readback(IDirect3DSurface9* pSurface, DWORD dwIndex, HVDService::HVDDxva2UncompBufLockInfo* pInfo, HVDService::HVDFastReadbackOption* pReadbackOption)
{
    COMPUTE_READBACK_TIME_INIT();
    HRESULT hr;
    NVDX_ObjectHandle hD3d9;

    hr = NvAPI_D3D9_GetSurfaceHandle(pSurface, &hD3d9);
    if(hr!=S_OK)
        return hr;

    NVAPI_D3D9_DMA_PARAMS params; memset(&params, 0, sizeof(params));
    params.version	= NVAPI_D3D9_DMA_PARAMS_VER;
    params.dwCommand = NVAPI_D3D9_DMA_CMD_TRANSFER;
    params.TransferParams.direction = NVAPI_D3D9_DMA_TRANSFER_DIR_DOWNLOAD;
    params.TransferParams.hSysmemSurface = m_hRenderTarget;
    params.TransferParams.hD3D9Surface = hD3d9;
    hr=NvAPI_D3D9_DMA(m_pDevice, &params);

    if(hr==S_OK)
    {
        pReadbackOption->bOutputProgressive = pReadbackOption->bInputProgressive;
        if(m_bUsePicthCopy && m_SysBufInfo.dwFormat==MAKEFOURCC('N','V','1','2'))
        {
            CopyNV12((char*)pInfo->pBits, (char*)pVideoStartAddress, m_uBufWidth, m_uBufHeight, m_SysBufInfo.dwPitch);
            pInfo->dwFourCC = m_SysBufInfo.dwFormat;
            pInfo->uPitch	= m_uBufWidth;
        }
        else
        {
            pInfo->pBits	= pVideoStartAddress;
            pInfo->dwFourCC = m_SysBufInfo.dwFormat;
            pInfo->uPitch	= m_SysBufInfo.dwPitch;
        }
    }
    COMPUTE_READBACK_TIME("NVapi DMA");
    return hr;
}

HRESULT CNVFastReadback::Close()
{
    DP_DBG(("CNVFastReadback::Close()"));

    HRESULT hr;
    NVAPI_D3D9_DMA_PARAMS p;

    memset(&p, 0, sizeof(p));
    p.version	= NVAPI_D3D9_DMA_PARAMS_VER;
    p.dwCommand = NVAPI_D3D9_DMA_CMD_UNMAP;
    p.UnmapParams.hSysmemSurface = m_hRenderTarget;
    if(pSysMem)
        delete[]pSysMem;
    pSysMem = 0;

    return hr=NvAPI_D3D9_DMA(m_pDevice, &p);
}



// Intel FastReadback
CIntelFastReadback::CIntelFastReadback()
{
    m_pCaps = 0;
    m_pCaps2 = 0;
    m_pRegVidPro = 0;
    m_pRegVidProService = 0;
    m_dwSupportedDecoderCount = 0; 
    m_pSupportedDecoderGuids = NULL;
    m_dwSurfaceCount = 0;
    m_hDXVA2 = NULL;
    m_ppSurfaces = NULL;
    m_hRegistration = NULL;
    m_pfnAuxDXVA2CreateVideoService = NULL;
    m_pfnRegDXVA2CreateVideoService = NULL;
    m_pDstFormats = NULL;
    m_pAuxVidDecService = NULL;
    m_pAuxVidDec = NULL;
    m_pDummySurface = NULL;
    m_DstInSystem = TRUE;
    m_pComp = NULL;
    m_pAllocator = NULL;

    m_dwDisplayWidth = 0;
    m_dwDisplayHeight = 0;

    m_bLockSourceFirst = FALSE;
    m_dwReadbackCounter = 0;
#if defined DELAYACCEL	
    m_dwWriteframe = 0;
    memset(m_pSourceSurface, 0, sizeof(m_pSourceSurface)); 
#endif
    memset(m_pDestSurface, 0, sizeof(m_pDestSurface));  
    ZeroMemory(&m_AuxDxva2ConfigPictureDecode, sizeof(m_AuxDxva2ConfigPictureDecode));
}

CIntelFastReadback::~CIntelFastReadback()
{
    SAFE_DELETE_ARRAY(m_pSupportedDecoderGuids);
    SAFE_DELETE_ARRAY(m_ppSurfaces);
    SAFE_RELEASE(m_pAuxVidDecService);		
    SAFE_RELEASE(m_pAuxVidDec);
    SAFE_RELEASE(m_pDummySurface);
    SAFE_RELEASE(m_pRegVidProService);
    SAFE_RELEASE(m_pRegVidPro);
    SAFE_DELETE (m_pDstFormats);
    for (int j = 0; j < DST_COUNT; j++)
        SAFE_RELEASE (m_pDestSurface[j]); 
#if defined DELAYACCEL
    for (int j = 0; j < SRC_COUNT; j++)
        SAFE_RELEASE(m_pSourceSurface[j]);
#endif 
    if(m_pAllocator)
    {
        delete [] m_pAllocator;
        m_pAllocator = NULL;
    }

    if(m_pCaps)
        free(m_pCaps);
    m_pCaps = 0;
    if(m_pCaps2)
        free(m_pCaps2);
    m_pCaps2 = 0;
}

HRESULT CIntelFastReadback::CreateRenderTarget(D3DFORMAT fourcc)
{
    // Create NV12 Render Target Surface
    if(m_pCaps==0)
        return E_POINTER;

    HRESULT hr = m_pDevice->CreateOffscreenPlainSurface(
        m_uBufWidth,
        m_uBufHeight, 
        fourcc, 
        m_DstInSystem?D3DPOOL_SYSTEMMEM:D3DPOOL_DEFAULT, 
        &m_pDestSurface[0], 
        NULL);

    if (FAILED(hr))
        DP_FASTREADBACK("Fast Compositing failed to create render target surface with error 0x%x.\n", hr);
    return hr;
}

HRESULT CIntelFastReadback::Open(FastReadbackInit *pReadbackInit)
{
    DP_DBG(("CIntelFastReadback::Open()"));

    CHECK_POINTER(pReadbackInit);

    HRESULT hr = E_FAIL;	

    m_hDXVA2 = pReadbackInit->hDXVA2;
    m_pDevice = pReadbackInit->pDevice;
    m_uBufHeight = pReadbackInit->uBufHeight;
    m_uBufWidth = pReadbackInit->uBufWidth;
    m_uSrcHeight = pReadbackInit->uSrcHeight;
    m_uSrcWidth = pReadbackInit->uSrcWidth;
    m_d3dSurfaceFormat = pReadbackInit->d3dSurfaceFormat;
    m_uExpectedWidth = pReadbackInit->dwExpectedReadbackWidth;
    m_uExpectedHeight = pReadbackInit->dwExpectedReadbackHeight;
    m_dwReadbackCounter = 0;

    CHECK_POINTER(m_pDevice);

#if defined DXVA2FASTCOMPOSITING

    FASTCOMP_SAMPLE_FORMATS *pFormats = NULL;
    FASTCOMP_CAPS           *pCaps  = NULL;
    FASTCOMP_CAPS2          *pCaps2 = NULL;
    DXVA2_VideoDesc          VideoDesc={0};
    IDirect3DSurface9       *pRenderTarget = NULL;

    // Create Compositing device
    m_pComp = new CFastCompositingDevice(m_hDXVA2, m_pDevice, FASTCOMP_MODE_PRE_PROCESS);

    //// Check if the Fast Compositing device is present in the system
    if (!m_pComp->IsPresent())
    {
        DP_FASTREADBACK("Failed to create the Fast Compositing device\n");
        return E_FAIL;
    }

    // Query Formats
    hr = m_pComp->QueryFormats(&pFormats);
    if (FAILED(hr))
    {
        DP_FASTREADBACK("Failed to obtain Fast Compositing formats with error 0x%x.\n", hr);
        return E_FAIL;
    }

    // Query Device Caps
    hr = m_pComp->QueryCaps(&pCaps);
    if (FAILED(hr))
    {
        DP_FASTREADBACK("Failed to obtain Fast Compositing caps with error 0x%x.\n", hr);
        return E_FAIL;
    }
    m_pCaps = pCaps;

    // Make sure the system can support fast readback
    hr = m_pComp->QueryCaps2(&pCaps2);
    if (FAILED(hr))
    {
        DP_FASTREADBACK("Failed to obtain Fast Compositing caps2 with error 0x%x.\n", hr);
        return E_FAIL;
    }
    m_pCaps2 = pCaps2;
    if(!m_pCaps2->bTargetInSysMemory)
    {
        DP_FASTREADBACK("CFastCompositingDevice::FastCopy: No support for Target in system memory\n", hr);
#if 0
        return E_FAIL;
#endif
    }

    // Create the Fast Compositing Device
    VideoDesc.SampleWidth                         = pCaps->sPrimaryVideoCaps.iMaxWidth;//m_uBufWidth;         // Width
    VideoDesc.SampleHeight                        = pCaps->sPrimaryVideoCaps.iMaxHeight;//m_uBufHeight;        // Height
    VideoDesc.SampleFormat.VideoChromaSubsampling = DXVA2_VideoChromaSubsampling_MPEG2;
    VideoDesc.SampleFormat.NominalRange           = DXVA2_NominalRange_Normal;
    VideoDesc.SampleFormat.VideoTransferMatrix    = DXVA2_VideoTransferMatrix_BT709;
    VideoDesc.SampleFormat.VideoLighting          = DXVA2_VideoLighting_dim;
    VideoDesc.SampleFormat.VideoPrimaries         = DXVA2_VideoPrimaries_BT709;
    VideoDesc.SampleFormat.VideoTransferFunction  = DXVA2_VideoTransFunc_709;
    VideoDesc.SampleFormat.SampleFormat           = DXVA2_SampleFieldInterleavedOddFirst;       // Interlaced
    VideoDesc.Format                              = (D3DFORMAT)MAKEFOURCC('N','V','1','2');     // NV12
    VideoDesc.InputSampleFreq.Numerator           = 30000; 
    VideoDesc.InputSampleFreq.Denominator         = 1001;
    VideoDesc.OutputFrameFreq.Numerator           = 60000;
    VideoDesc.OutputFrameFreq.Denominator         = 1001;
    VideoDesc.UABProtectionLevel                  = 0;
    VideoDesc.Reserved                            = 0;

    hr = m_pComp->CreateService(&VideoDesc, (D3DFORMAT)MAKEFOURCC('N','V','1','2'), 0);
    if (FAILED(hr))
    {
        DP_FASTREADBACK("failed to create Fast Compositing service with error 0x%x.\n", hr);
        return E_FAIL;
    }

    // Create NV12 Render Target Surface
    hr = CreateRenderTarget();
    if (FAILED(hr))
    {
        DP_FASTREADBACK("Fast Compositing failed to create render target surface with error 0x%x.\n", hr);
    }

#else
    m_dwSupportedDecoderCount = pReadbackInit->dwSupportedDecoderCount;
    m_pSupportedDecoderGuids = pReadbackInit->pSupportedDecoderGuids;
    m_dwSurfaceCount = pReadbackInit->dwSurfaceCount;
    m_ppSurfaces = pReadbackInit->ppSurfaces;
    CHECK_POINTER(m_ppSurfaces);
    hr = Intel_Init();
    if (FAILED(hr))
        return E_FAIL;

    // Create Auxiliary Device
    hr = CreateAuxiliaryDevice();
    if (FAILED(hr))
        return E_FAIL;

    // Create Fast Copy Device
    hr = QueryFastcopy();
    if (FAILED(hr))
        return E_FAIL;

    hr = CreateFastCopyDevice();
    if (FAILED(hr))
        return E_FAIL;

    // Query Caps
    hr = QueryAccelRTFormats(&m_pDstFormats, &m_uDstFormatCount);
    if (FAILED(hr))
        return E_FAIL;

    hr = Intel_CreateService();
    if (SUCCEEDED(hr))
    {
        // Register Source surfaces
        hr = FastCopyRegister(m_dwSurfaceCount,m_ppSurfaces);
        if (FAILED(hr))
            return E_FAIL;

        // Register destination surfaces
        for (int j = 0; j < DST_COUNT; j++)
        {
            hr = m_pDevice->CreateOffscreenPlainSurface(m_uBufWidth, m_uBufHeight, m_d3dSurfaceFormat, D3DPOOL_SYSTEMMEM, &m_pDestSurface[j], NULL);
            if (FAILED(hr))
            {
                DP_FASTREADBACK("CreateOffscreenPlainSurface failed m_uBufWidth:%d m_uBufHeight:%d",m_uBufWidth,m_uBufHeight);
                return E_FAIL;
            }
        }

        hr = FastCopyRegister(DST_COUNT, m_pDestSurface);
    }
#endif
    return hr;
}

HRESULT CIntelFastReadback::InstallMediaAllocator(void* pAllocater)
{
    if(m_pAllocator)
    {
        delete [] m_pAllocator;
        m_pAllocator = NULL;
    }

    if(pAllocater)
    {
        if(!m_pAllocator)
            m_pAllocator = new BYTE [sizeof(MSDK_MEM_ALLOCATOR)];
        memcpy(m_pAllocator, pAllocater, sizeof(MSDK_MEM_ALLOCATOR));
    }

    return S_OK;
}

HRESULT CIntelFastReadback::SetDisplayArea(DWORD dwWitdh, DWORD dwHeight)
{
    m_dwDisplayWidth = dwWitdh;
    m_dwDisplayHeight = dwHeight;
    return S_OK;
}

HRESULT CIntelFastReadback::Readback(IDirect3DSurface9* pSurface, DWORD dwIndex, HVDService::HVDDxva2UncompBufLockInfo* pInfo, HVDService::HVDFastReadbackOption* pReadbackOption)
{
    CHECK_POINTER(pSurface);
    CAutoLock lock(&m_csObj);
    HRESULT hr = S_OK;

    if(pReadbackOption== NULL)
        return E_POINTER;

    if(m_pComp==NULL) // cannot use FC
    {
        pReadbackOption->pReturnSurface = pSurface;
        return S_OK;
    }

#if defined DELAYACCEL
    if (pReadbackOption->dwDelayPeriod > 1 && m_dwWriteframe > 0) // Enable delay accelerate
    {
        if (dwIndex < 100)	// Record the index of frame
        {
            m_uRecordFrameIndex[m_dwWriteframe%dwDelayPeriod]=dwIndex;	
            //DP_FASTREADBACK("Get frame index:%d",dwIndex);
            m_dwWriteframe++;
        }
        else	// Copy frame from video memory to system memory
        {
            int CurrentFrameIndex = m_uRecordFrameIndex[dwIndex-100];
            START_TICKS
                // Fast Copy blt
                hr = FastCopyBlt(m_pSourceSurface[CurrentFrameIndex], m_pDestSurface[CurrentFrameIndex]);
            if (FAILED(hr))
                return E_FAIL;		
            STOP_TICKS
                // Lock the destination surface and read data
                hr = m_pDestSurface[CurrentFrameIndex]->LockRect(&d3dLockRect, NULL, 0);
            if (SUCCEEDED(hr))
            {
                pInfo->pBits = (byte*)d3dLockRect.pBits;
                pInfo->uPitch = d3dLockRect.Pitch;
                pInfo->dwFourCC = m_d3dSurfaceFormat;	
            }
            hr = m_pDestSurface[CurrentFrameIndex]->UnlockRect();
        }
    }
    else
#endif
    {
        if(m_bLockSourceFirst || m_dwReadbackCounter<DXVALockSurfaceFirstNum)
        {
            D3DLOCKED_RECT  d3dLockRect = {0};
            hr = pSurface->LockRect(&d3dLockRect, NULL, 0);
            if (FAILED(hr))
                DP_FASTREADBACK("Lock Src failed with error 0x%x.\n", hr);
            pSurface->UnlockRect();
            
            if(m_dwReadbackCounter<DXVALockSurfaceFirstNum)
                m_dwReadbackCounter++;
        }

        if(pReadbackOption->dwReturnFourcc==FALSE && m_DstInSystem)
        {
            m_DstInSystem = FALSE;
            m_pDestSurface[0]->Release();
            CreateRenderTarget();
        }

        COMPUTE_READBACK_TIME_INIT();
        RECT srcRect, dstRect;
        IDirect3DSurface9* pDst = NULL;

        MSDK_MEM_ALLOCATOR* pAllocator = NULL;
        pAllocator = (pReadbackOption->pMskMemAllocator) ? (MSDK_MEM_ALLOCATOR*)pReadbackOption->pMskMemAllocator : (MSDK_MEM_ALLOCATOR*)m_pAllocator;
        if(pAllocator)
        {
            while(pDst==NULL)
            {
                pDst = (IDirect3DSurface9*)pAllocator->MSDK_GetFreeSurface(pAllocator->pEncInst);
                if(!pDst)
                    DP_ERR("CIntelFastReadback::Readback()- Fail to get free surface");
            }
            
            srcRect.left   = 0;
            srcRect.top    = 0;
            srcRect.right  = m_dwDisplayWidth ? m_dwDisplayWidth:m_uSrcWidth;
            srcRect.bottom = m_dwDisplayHeight ? m_dwDisplayHeight:m_uSrcHeight;

            dstRect.left   = 0;
            dstRect.top    = 0;
            dstRect.right  = m_uBufWidth;
            // bug#115159, make output video's height the same as SW mode in resizing case
            // 1081 is workaround magic number.
            dstRect.bottom = (m_uBufHeight==1088 ? (pReadbackOption->bInputProgressive==1?1080:1081): m_uBufHeight);

            pReadbackOption->bOutputProgressive = ((pReadbackOption->bInputProgressive) ? 1:pReadbackOption->bOutputProgressive);
        }
        else
        {
            pDst = m_pDestSurface[0];

            srcRect.left   = 0;
            srcRect.top    = 0;
            srcRect.right  = m_dwDisplayWidth ? m_dwDisplayWidth:m_uSrcWidth;
            srcRect.bottom = m_dwDisplayHeight ? m_dwDisplayHeight:m_uSrcHeight;

            dstRect.left   = 0;
            dstRect.top    = 0;
            dstRect.right  = m_uExpectedWidth ? m_uExpectedWidth : m_uBufWidth;
            dstRect.bottom = m_uExpectedHeight ? m_uExpectedHeight : m_uBufHeight;
        }

        hr = m_pComp->FastCopy(pDst, 
            pSurface, 
            &dstRect, 
            &srcRect, 
            !pReadbackOption->bInputProgressive,	//Is the input interlaced?
            !pReadbackOption->bOutputProgressive,	// always output progressive
            FASTCOMP_TOP_FIELD);

        if(FAILED(hr) && pReadbackOption->bInputProgressive && pReadbackOption->bOutputProgressive==0)
        {
            pReadbackOption->bOutputProgressive = 1;
            // try again
            hr = m_pComp->FastCopy(pDst, 
                pSurface, 
                &dstRect, 
                &srcRect, 
                !pReadbackOption->bInputProgressive,	//Is the input interlaced?
                !pReadbackOption->bOutputProgressive,	// always output progressive
            FASTCOMP_TOP_FIELD);
        }

        if (FAILED(hr))
        {
            // no need to use FC. We can just return the DXVA surface to the caller
            if(pReadbackOption->dwReturnFourcc==FALSE && m_DstInSystem==0)
            {
                Close();
                pReadbackOption->pReturnSurface = pSurface;
                return S_OK;
            }
            DP_FASTREADBACK("Fast Compositing failed with error 0x%x.\n", hr);
            return E_FAIL;		
        }

        if (pReadbackOption->dwReturnFourcc == FALSE)
        {
            pReadbackOption->wWidth = m_uBufWidth;
            pReadbackOption->wHeight = m_uBufHeight;
            pReadbackOption->pReturnSurface = pDst;
        }
        else
        {		
            CHECK_POINTER(pInfo);
            D3DLOCKED_RECT  d3dLockRect = {0};
            hr = pDst->LockRect(&d3dLockRect, NULL, 0);
            if (SUCCEEDED(hr))
            {
                UINT nD3DPitch = (m_DstInSystem ? d3dLockRect.Pitch>>1 : d3dLockRect.Pitch); // Intel will report double pitch, if bilit to system memory.

                if(m_bUsePicthCopy && m_d3dSurfaceFormat==MAKEFOURCC('N','V','1','2'))
                {
                    pReadbackOption->wHeight = m_uBufHeight;
                    pReadbackOption->wWidth = m_uBufWidth;
                    CopyNV12((char*)pInfo->pBits, (char*)d3dLockRect.pBits, m_uBufWidth, m_uBufHeight, nD3DPitch);
                    pInfo->uPitch		= m_uBufWidth;
                    pInfo->dwFourCC		= m_d3dSurfaceFormat;
                }
                else
                {
                    pReadbackOption->wHeight = m_uBufHeight;
                    pReadbackOption->wWidth = m_uBufWidth;
                    pInfo->pBits = (byte*)d3dLockRect.pBits;
                    pInfo->uPitch = nD3DPitch;
                    pInfo->dwFourCC = m_d3dSurfaceFormat;
                }
                hr = pDst->UnlockRect();
            }
        }
        COMPUTE_READBACK_TIME("Intel FC");
    }

    return hr;
}

HRESULT CIntelFastReadback::Close()
{
    DP_DBG(("CIntelFastReadback::Close()"));

    SAFE_RELEASE(m_pAuxVidDecService);		
    SAFE_RELEASE(m_pAuxVidDec);
    SAFE_RELEASE(m_pDummySurface);
    SAFE_RELEASE(m_pRegVidProService);
    SAFE_RELEASE(m_pRegVidPro);
    SAFE_DELETE (m_pDstFormats);
    SAFE_DELETE (m_pComp);
    for (int j = 0; j < DST_COUNT; j++)
        SAFE_RELEASE (m_pDestSurface[j]);
#if defined DELAYACCEL
    for (int j = 0; j < SRC_COUNT; j++)
        SAFE_RELEASE(m_pSourceSurface[j]);
#endif

    return S_OK;
}

HRESULT CIntelFastReadback::Intel_Init()
{	
    if (m_hDXVA2)
    {
        m_pfnAuxDXVA2CreateVideoService = (TpfnDXVA2CreateVideoService)GetProcAddress(m_hDXVA2, "DXVA2CreateVideoService");
        m_pfnRegDXVA2CreateVideoService = (TpfnDXVA2CreateVideoService)GetProcAddress(m_hDXVA2, "DXVA2CreateVideoService");
    }

    if (!m_pfnAuxDXVA2CreateVideoService || !m_pfnRegDXVA2CreateVideoService)
    {
        m_hDXVA2 = NULL;
        m_pfnAuxDXVA2CreateVideoService = NULL;
        m_pfnRegDXVA2CreateVideoService = NULL;
        return E_FAIL;
    }
    return S_OK;
}

HRESULT CIntelFastReadback::CreateAuxiliaryDevice()
{
    CHECK_POINTER(m_pDevice);

    HRESULT hr = E_FAIL;
    DXVA2_VideoDesc dxva2Desc = {0};
    DXVA2_ConfigPictureDecode *pConfigs;
    UINT nConfig=0;	

    IDirect3DSurface9** ppAuxSurfaces = NULL;
    ppAuxSurfaces = new IDirect3DSurface9*[1];
    if (!ppAuxSurfaces)
        return E_OUTOFMEMORY;

    m_pfnAuxDXVA2CreateVideoService(m_pDevice, __uuidof(IDirectXVideoDecoderService), (void**) &m_pAuxVidDecService);
    if (m_pAuxVidDecService)
    {
        for (UINT j=0; j < m_dwSupportedDecoderCount ; j++)
        {
            if (IsEqualGUID(m_pSupportedDecoderGuids[j], DXVA2_Intel_Auxiliary_Device))
            {
                hr = m_pAuxVidDecService->CreateSurface(64,64,0,(D3DFORMAT)MAKEFOURCC('I', 'M', 'C', '3'), D3DPOOL_DEFAULT, 0, DXVA2_VideoDecoderRenderTarget, ppAuxSurfaces, NULL);
                if (FAILED(hr))
                {
                    break;
                }

                dxva2Desc.SampleWidth                         = 64;
                dxva2Desc.SampleHeight                        = 64;
                dxva2Desc.SampleFormat.VideoChromaSubsampling = DXVA2_VideoChromaSubsampling_MPEG2;
                dxva2Desc.SampleFormat.NominalRange           = DXVA2_NominalRange_16_235;
                dxva2Desc.SampleFormat.VideoTransferMatrix    = DXVA2_VideoTransferMatrix_BT709;
                dxva2Desc.SampleFormat.VideoLighting          = DXVA2_VideoLighting_dim;
                dxva2Desc.SampleFormat.VideoPrimaries         = DXVA2_VideoPrimaries_BT709;
                dxva2Desc.SampleFormat.VideoTransferFunction  = DXVA2_VideoTransFunc_709;
                dxva2Desc.SampleFormat.SampleFormat           = DXVA2_SampleProgressiveFrame;
                dxva2Desc.Format                              = (D3DFORMAT)MAKEFOURCC('I', 'M', 'C', '3');
                dxva2Desc.InputSampleFreq.Numerator           = dxva2Desc.OutputFrameFreq.Numerator   = 60000;
                dxva2Desc.InputSampleFreq.Denominator         = dxva2Desc.OutputFrameFreq.Denominator = 1001;
                dxva2Desc.UABProtectionLevel                  = 0;
                dxva2Desc.Reserved                            = 0;

                hr = m_pAuxVidDecService->GetDecoderConfigurations(m_pSupportedDecoderGuids[j], &dxva2Desc, NULL, &nConfig, &pConfigs);
                if (SUCCEEDED(hr))
                {
                    for (UINT i = 0; i < nConfig ; i++)
                    {
                        hr = m_pAuxVidDecService->CreateVideoDecoder(m_pSupportedDecoderGuids[j], &dxva2Desc, &pConfigs[i], ppAuxSurfaces, 1 , &m_pAuxVidDec);
                        if (SUCCEEDED(hr))
                        {
                            m_AuxDxva2ConfigPictureDecode = pConfigs[i];				
                            CoTaskMemFree(pConfigs);
                            SAFE_DELETE_ARRAY(ppAuxSurfaces);
                            return S_OK;
                        }
                    }
                }
            }
        }			
    }

    if (pConfigs)
    {
        CoTaskMemFree(pConfigs);
    }

    SAFE_DELETE_ARRAY(ppAuxSurfaces);

    return E_FAIL;
}

HRESULT CIntelFastReadback::QueryFastcopy()
{
    // query fastcopy device
    DXVA2_DecodeExtensionData AuxExdata;
    DXVA2_DecodeExecuteParams AuxEXParam;
    HRESULT hr = E_FAIL;
    UINT iGuidCount = 0;

    if (!m_pAuxVidDec)
    {
        DP_FASTREADBACK("Auxiliary Device is invalid");
        return E_FAIL;
    }

    AuxExdata.Function = AUXDEV_GET_ACCEL_GUID_COUNT;	
    AuxExdata.pPrivateInputData = NULL;
    AuxExdata.PrivateInputDataSize = 0;
    AuxExdata.pPrivateOutputData = &iGuidCount;
    AuxExdata.PrivateOutputDataSize = sizeof(int);

    AuxEXParam.NumCompBuffers     = 0;
    AuxEXParam.pCompressedBuffers = 0;
    AuxEXParam.pExtensionData     = &AuxExdata;

    hr = m_pAuxVidDec->Execute(&AuxEXParam);
    if (FAILED(hr))
    {
        DP_FASTREADBACK("Query guid count failed");
        return E_FAIL;
    }

    // Get Acceleration Service Guids
    GUID *pGuids = NULL;
    pGuids = new GUID[iGuidCount];
    if (!pGuids)
        return E_OUTOFMEMORY;

    AuxExdata.Function              = AUXDEV_GET_ACCEL_GUIDS;
    AuxExdata.pPrivateInputData     = NULL;
    AuxExdata.PrivateInputDataSize  = 0;
    AuxExdata.pPrivateOutputData    = pGuids;
    AuxExdata.PrivateOutputDataSize = iGuidCount * sizeof(GUID);

    hr = m_pAuxVidDec->Execute(&AuxEXParam);
    if (SUCCEEDED(hr))
    {
        for (UINT j = 0; j < iGuidCount ; j++)
        {	
            if (IsEqualGUID(pGuids[j], DXVA2_Intel_FastCopy_Device))
            {
                SAFE_DELETE_ARRAY(pGuids);
                return S_OK;
            }
        }
    }

    SAFE_DELETE_ARRAY(pGuids);
    return E_FAIL;
}

HRESULT CIntelFastReadback::CreateFastCopyDevice()
{
    CHECK_POINTER(m_pDevice);
    // Create Registration device
    DXVA2_VideoDesc dxva2Desc = {0};
    HRESULT hr = E_FAIL;

    hr = m_pfnRegDXVA2CreateVideoService(m_pDevice,__uuidof(IDirectXVideoProcessorService), (void**) &m_pRegVidProService);
    if (FAILED(hr))
    {
        DP_FASTREADBACK("Create Register service failed");
        return E_FAIL;
    }

    dxva2Desc.SampleWidth                         = 16;
    dxva2Desc.SampleHeight                        = 16;
    dxva2Desc.SampleFormat.VideoChromaSubsampling = DXVA2_VideoChromaSubsampling_MPEG2;
    dxva2Desc.SampleFormat.NominalRange           = DXVA2_NominalRange_16_235;
    dxva2Desc.SampleFormat.VideoTransferMatrix    = DXVA2_VideoTransferMatrix_BT709;
    dxva2Desc.SampleFormat.VideoLighting          = DXVA2_VideoLighting_dim;
    dxva2Desc.SampleFormat.VideoPrimaries         = DXVA2_VideoPrimaries_BT709;
    dxva2Desc.SampleFormat.VideoTransferFunction  = DXVA2_VideoTransFunc_709;
    dxva2Desc.SampleFormat.SampleFormat           = DXVA2_SampleProgressiveFrame;
    dxva2Desc.Format                              = D3DFMT_YUY2;
    dxva2Desc.InputSampleFreq.Numerator           = 60;
    dxva2Desc.InputSampleFreq.Denominator         = 1;
    dxva2Desc.OutputFrameFreq.Numerator           = 60;
    dxva2Desc.OutputFrameFreq.Denominator         = 1;

    hr = m_pRegVidProService->CreateVideoProcessor(DXVA2_Registration_Device,&dxva2Desc,D3DFMT_YUY2,1,&m_pRegVidPro);

    return hr;
}

HRESULT CIntelFastReadback::QueryAccelRTFormats(D3DFORMAT **ppFormats, UINT *puCount)
{
    DXVA2_DecodeExecuteParams sExecute;
    DXVA2_DecodeExtensionData sExtension;

    HRESULT    hr     = E_FAIL;
    UINT       uCount   = 0;

    // Get GUID count
    sExtension.Function              = AUXDEV_GET_ACCEL_RT_FORMAT_COUNT;
    sExtension.pPrivateInputData     = (PVOID)&DXVA2_Intel_FastCopy_Device;
    sExtension.PrivateInputDataSize  = sizeof(GUID);
    sExtension.pPrivateOutputData    = &uCount;
    sExtension.PrivateOutputDataSize = sizeof(int);

    sExecute.NumCompBuffers     = 0;
    sExecute.pCompressedBuffers = 0;
    sExecute.pExtensionData     = &sExtension;

    hr = m_pAuxVidDec->Execute(&sExecute);
    if (FAILED(hr))
    {
        DP_FASTREADBACK("Get acceleration RT count failed");
        return E_FAIL;
    }

    // Allocate array of formats
    D3DFORMAT *pFormats = NULL;
    pFormats = new D3DFORMAT[uCount];
    if (!pFormats)
        return E_OUTOFMEMORY;

    // Get Guids
    sExtension.Function              = AUXDEV_GET_ACCEL_RT_FORMATS;
    sExtension.pPrivateInputData     = (PVOID)&DXVA2_Intel_FastCopy_Device;
    sExtension.PrivateInputDataSize  = sizeof(GUID);
    sExtension.pPrivateOutputData    = pFormats;
    sExtension.PrivateOutputDataSize = uCount * sizeof(D3DFORMAT);

    hr = m_pAuxVidDec->Execute(&sExecute);
    if (FAILED(hr))
    {
        delete [] pFormats;
        DP_FASTREADBACK("Get acceleration RT format failed");
        return E_FAIL;
    }

    *puCount   = uCount;
    *ppFormats = pFormats;

    return S_OK;
}

HRESULT CIntelFastReadback::Intel_CreateService()
{
    HRESULT hr = E_FAIL;
    int    iCreateData = 0;
    UINT   uCreateSize = sizeof(int);

    hr = CreateAccelService(&DXVA2_Intel_FastCopy_Device, &iCreateData, &uCreateSize);
    if (FAILED(hr))
    {
        DP_FASTREADBACK("Create fastcopy service failed");
        return E_FAIL;
    }

    // Create dummy surface
    hr = m_pRegVidProService->CreateSurface(64, 64, 0,(D3DFORMAT)MAKEFOURCC('Y','U','Y','2'),D3DPOOL_DEFAULT,0,DXVA2_VideoProcessorRenderTarget,&m_pDummySurface,0);
    if (FAILED(hr))
    {
        DP_FASTREADBACK("Register create surface failed");
        return E_FAIL;
    }

    // Obtain registration handle
    FASTCOPY_QUERYCAPS sQuery;
    UINT               uQuerySize;

    sQuery.Type = FASTCOPY_QUERY_REGISTRATION_HANDLE;
    uQuerySize  = sizeof(sQuery);

    hr = QueryAccelCaps(&sQuery, &uQuerySize);
    if (SUCCEEDED(hr))
    {
        m_hRegistration = sQuery.hRegistration;
    }
    else
    {
        SAFE_RELEASE(m_pDummySurface);
        m_hRegistration = NULL;
    }

    return hr;
}

HRESULT CIntelFastReadback::CreateAccelService(CONST GUID *pAccelGuid, void *pCreateParams, UINT *puCreateParamSize)
{
    HRESULT hr = E_FAIL;
    DXVA2_DecodeExecuteParams sExecute;
    DXVA2_DecodeExtensionData sExtension;

    // Query Caps
    sExtension.Function              = AUXDEV_CREATE_ACCEL_SERVICE;
    sExtension.pPrivateInputData     = (PVOID)pAccelGuid;
    sExtension.PrivateInputDataSize  = sizeof(GUID);
    sExtension.pPrivateOutputData    = pCreateParams;
    sExtension.PrivateOutputDataSize = *puCreateParamSize;

    sExecute.NumCompBuffers     = 0;
    sExecute.pCompressedBuffers = 0;
    sExecute.pExtensionData     = &sExtension;

    hr = m_pAuxVidDec->Execute(&sExecute);

    return hr;
}

HRESULT CIntelFastReadback::QueryAccelCaps(void *pCaps, UINT *puCapSize)
{	
    HRESULT hr = E_FAIL;
    DXVA2_DecodeExecuteParams sExecute;
    DXVA2_DecodeExtensionData sExtension;

    // Query Caps
    sExtension.Function              = AUXDEV_QUERY_ACCEL_CAPS;
    sExtension.pPrivateInputData     = (PVOID)&DXVA2_Intel_FastCopy_Device;
    sExtension.PrivateInputDataSize  = sizeof(GUID);
    sExtension.pPrivateOutputData    = pCaps;
    sExtension.PrivateOutputDataSize = *puCapSize;

    sExecute.NumCompBuffers     = 0;
    sExecute.pCompressedBuffers = 0;
    sExecute.pExtensionData     = &sExtension;

    hr = m_pAuxVidDec->Execute(&sExecute);

    return hr;
}

HRESULT CIntelFastReadback::FastCopyRegister(int count, IDirect3DSurface9 *pSurface, ...)
{
    IDirect3DSurface9 *pList[32];
    int                n = 1;

    // Get first surface
    pList[0] = pSurface;

    // Get all surfaces
    va_list arg_list;
    va_start(arg_list, pSurface);
    while (n < count)
    {
        try
        {
            pList[n++] = (IDirect3DSurface9 *)va_arg(arg_list, LPVOID);
        }
        catch (...)
        {
            break;
        }
    }
    va_end(arg_list);

    return FastCopyRegister(count, pList);
}

HRESULT CIntelFastReadback::FastCopyRegister(int count, IDirect3DSurface9 **ppSurface)
{
    HRESULT hr = E_FAIL;

    DXVA2_SURFACE_REGISTRATION regRequest;
    DXVA2_SAMPLE_REG           regEntry[33];

    DXVA2_VideoProcessBltParams bltVidParams;
    DXVA2_VideoSample           bltSamples[32];

    RECT Rect = { 0, 0, 0, 0 };
    int n;

    // Setup registration request structure
    regRequest.RegHandle            = m_hRegistration;
    regRequest.pRenderTargetRequest = &regEntry[0];
    regRequest.nSamples             = count;
    regRequest.pSamplesRequest      = &regEntry[1];

    regEntry[0].pD3DSurface = m_pDummySurface;
    regEntry[0].Operation   = REG_IGNORE;
    for (n=0; n < count; n++)
    {
        regEntry[n+1].pD3DSurface = ppSurface[n];
        regEntry[n+1].Operation   = REG_REGISTER;
    }

    // Prepare VideoProcessBlt paramaters to the registration device
    memset(&bltVidParams, 0, sizeof(bltVidParams));
    memset(bltSamples   , 0, sizeof(bltSamples));
    bltVidParams.TargetFrame = (REFERENCE_TIME) (&regRequest);
    bltVidParams.TargetRect.left   = 0;
    bltVidParams.TargetRect.top    = 0;
    bltVidParams.TargetRect.right  = 64;
    bltVidParams.TargetRect.bottom = 64;
    bltVidParams.StreamingFlags    = 0;
    bltVidParams.BackgroundColor   = g_sBackground;
    bltVidParams.DestFormat.value  = 0;
    bltVidParams.DestFormat.VideoChromaSubsampling = DXVA2_VideoChromaSubsampling_MPEG2;
    bltVidParams.DestFormat.NominalRange           = DXVA2_NominalRange_Normal;
    bltVidParams.DestFormat.VideoTransferMatrix    = DXVA2_VideoTransferMatrix_BT709;
    bltVidParams.DestFormat.VideoLighting          = DXVA2_VideoLighting_dim;
    bltVidParams.DestFormat.VideoPrimaries         = DXVA2_VideoPrimaries_BT709;
    bltVidParams.DestFormat.VideoTransferFunction  = DXVA2_VideoTransFunc_709;
    bltVidParams.DestFormat.SampleFormat           = DXVA2_SampleProgressiveFrame;
    bltVidParams.Alpha.Value       = 0;
    bltVidParams.Alpha.Fraction    = 0;
    bltVidParams.DestData          = 0;

    // Setup sources
    for (n=0; n < count; n++)
    {
        bltSamples[n].Start        = (REFERENCE_TIME) (&regRequest);
        bltSamples[n].End          = bltSamples[0].Start + 1;
        bltSamples[n].SampleFormat = bltVidParams.DestFormat;
        bltSamples[n].SrcSurface   = ppSurface[n];
        bltSamples[n].SrcRect      = bltVidParams.TargetRect;
        bltSamples[n].DstRect      = bltVidParams.TargetRect;
        bltSamples[n].SampleData   = 0;
    }

    // Register surfaces
    hr = m_pRegVidPro->VideoProcessBlt(m_pDummySurface,&bltVidParams,bltSamples,count,NULL);

    return hr;
}

HRESULT CIntelFastReadback::FastCopyBlt(IDirect3DSurface9 *pSrc,IDirect3DSurface9 *pDst)
{
    HRESULT hr = E_FAIL;
    DXVA2_DecodeExecuteParams sExecute;
    DXVA2_DecodeExtensionData sExtension;

    FASTCOPY_BLT_PARAMS       sFastCopyBlt;

    // Fast Copy Blt
    sFastCopyBlt.pSource = pSrc;
    sFastCopyBlt.pDest   = pDst;
    sFastCopyBlt.bPerformScaling = TRUE; // turn off scaling
    sFastCopyBlt.DstRect.top = 0;
    sFastCopyBlt.DstRect.left = 0;
    sFastCopyBlt.DstRect.bottom = m_uBufHeight;
    sFastCopyBlt.DstRect.right = m_uBufWidth;

    sExtension.Function              = FASTCOPY_BLT;
    sExtension.pPrivateInputData     = (PVOID)&sFastCopyBlt;
    sExtension.PrivateInputDataSize  = sizeof(FASTCOPY_BLT_PARAMS);
    sExtension.pPrivateOutputData    = NULL;
    sExtension.PrivateOutputDataSize = 0;

    sExecute.NumCompBuffers     = 0;
    sExecute.pCompressedBuffers = 0;
    sExecute.pExtensionData     = &sExtension;

    hr = m_pAuxVidDec->Execute(&sExecute);

    return hr;
}

// ATI FastReadback
CATIFastReadback::CATIFastReadback()
{
    m_pVidDec = NULL;
    m_pRenderWnd = NULL;
    m_pMcomSession = NULL;
    m_bMcomDecodeTargetAccessSupport = FALSE;
    m_bUseScaling = TRUE; //FALSE ;
    m_bMcomScalingSupported = FALSE;
    m_uScaledWidth = 1920;
    m_uScaledHeight = 1088;
    m_uScaledPitch = 0;
    memset(&m_mcomDecodeTargetAccessCapsOutput, 0, sizeof(MCOM_GET_DECODE_TARGET_ACCESS_CAPS_OUTPUT)); 
    memset(&m_mcomBeginDecodeTargetAccessInput, 0, sizeof(MCOM_BEGIN_DECODE_TARGET_ACCESS_INPUT)); 
    memset(&m_mcomBeginDecodeTargetAccessOutput, 0, sizeof(MCOM_BEGIN_DECODE_TARGET_ACCESS_OUTPUT)); 
}

CATIFastReadback::~CATIFastReadback()
{
}

HRESULT CATIFastReadback::Open(FastReadbackInit *pReadbackInit)
{
    CHECK_POINTER(pReadbackInit);

    DWORD dwProgressiveContent = !(pReadbackInit->dwInputInterlaceFlag);
    m_pVidDec = pReadbackInit->pVidDec;
    m_pDevice = pReadbackInit->pDevice;
    m_pRenderWnd = pReadbackInit->hWnd;


    m_uScaledWidth	= pReadbackInit->dwExpectedReadbackWidth; 
    m_uScaledHeight	= pReadbackInit->dwExpectedReadbackHeight; 
    m_uSrcWidth = pReadbackInit->uSrcWidth;
    m_uSrcHeight = pReadbackInit->uSrcHeight;

    CHECK_POINTER(m_pDevice);
    CHECK_POINTER(m_pVidDec);

    // Create MCOM Session 
    MCOM_CREATE_INPUT mcomCreateInput; 
    MCOM_CREATE_OUTPUT mcomCreateOutput; 
    MCOM_STATUS mcomStatus = MCOM_OK; 

    memset(&mcomCreateInput, 0, sizeof(MCOM_CREATE_INPUT)); 
    mcomCreateInput.size = sizeof(MCOM_CREATE_INPUT); 
    mcomCreateInput.flags = 0; 
    mcomCreateInput.GfxDevice = m_pDevice;
    mcomCreateInput.windowHandle = m_pRenderWnd; 

    memset(&mcomCreateOutput, 0, sizeof(MCOM_CREATE_OUTPUT)); 
    mcomCreateOutput.size = sizeof(MCOM_CREATE_OUTPUT); 

    mcomStatus = MCOMCreate(&mcomCreateInput, &mcomCreateOutput); 
    if(MCOM_OK == mcomStatus) 
    {	
        // MCOMDecodeTargetAccessCaps 
        m_pMcomSession = mcomCreateOutput.MCOMSession; 

        // Query decode target access caps 

        MCOM_GET_DECODE_TARGET_ACCESS_CAPS_INPUT mcomDecodeTargetAccessCapsInput = {0}; 

        mcomDecodeTargetAccessCapsInput.size = sizeof(MCOM_GET_DECODE_TARGET_ACCESS_CAPS_INPUT); 
        mcomDecodeTargetAccessCapsInput.MCOMSession = m_pMcomSession; 
        m_mcomDecodeTargetAccessCapsOutput.size = sizeof(MCOM_GET_DECODE_TARGET_ACCESS_CAPS_OUTPUT); 

        mcomStatus = MCOMDecodeTargetAccessCaps(&mcomDecodeTargetAccessCapsInput, &m_mcomDecodeTargetAccessCapsOutput); 
        if(mcomStatus == MCOM_OK) 
        {
            m_bMcomDecodeTargetAccessSupport = TRUE; 
        }
        else 
        { 
            m_bMcomDecodeTargetAccessSupport = FALSE; 
            return E_FAIL;
        } 
    } 
    else 
    { 
        m_bMcomDecodeTargetAccessSupport = FALSE; 
        return E_FAIL;
    } 

    if(m_bMcomDecodeTargetAccessSupport) 
    {
        m_mcomBeginDecodeTargetAccessInput.size = sizeof(MCOM_BEGIN_DECODE_TARGET_ACCESS_INPUT); 
        m_mcomBeginDecodeTargetAccessInput.MCOMSession = m_pMcomSession; 
        m_mcomBeginDecodeTargetAccessInput.DecodeSession = m_pVidDec;

        if(m_mcomDecodeTargetAccessCapsOutput.OutputCaps.CAP_NV12) 
        {
            m_mcomBeginDecodeTargetAccessInput.outputMode = MCOM_DECODE_TARGET_ACCESS_OUTPUT_MODE_NV12;
        }
        else if(m_mcomDecodeTargetAccessCapsOutput.OutputCaps.CAP_YV12) 
        {		
            m_mcomBeginDecodeTargetAccessInput.outputMode = MCOM_DECODE_TARGET_ACCESS_OUTPUT_MODE_YV12;
        }
        else if(m_mcomDecodeTargetAccessCapsOutput.OutputCaps.CAP_YUY2) 
        {
            m_mcomBeginDecodeTargetAccessInput.outputMode = MCOM_DECODE_TARGET_ACCESS_OUTPUT_MODE_YUY2; 
        }

		// In m_uSrcHeight == 1088 case, there is green line issue on the bottom if do MCOM, need to fix it
        if(m_bUseScaling /*&& m_uSrcHeight != 1088*/ && (m_uSrcWidth!=ALIGN16(m_uScaledWidth) || m_uSrcHeight!=ALIGN16(m_uScaledHeight)))
			m_bMcomScalingSupported = (dwProgressiveContent || m_mcomDecodeTargetAccessCapsOutput.ScalingCaps.CAP_INTERLACED);
        else
            m_bMcomScalingSupported = FALSE;

        if(m_bMcomScalingSupported)
        {
            if(m_mcomDecodeTargetAccessCapsOutput.ScalingCaps.CAP_BICUBIC)
            {
                m_mcomBeginDecodeTargetAccessInput.scalingMode = MCOM_DECODE_TARGET_ACCESS_SCALING_MODE_BICUBIC;
                m_mcomBeginDecodeTargetAccessInput.scaledWidth = m_uScaledWidth; 
                m_mcomBeginDecodeTargetAccessInput.scaledHeight = m_uScaledHeight;
            }
            else if(m_mcomDecodeTargetAccessCapsOutput.ScalingCaps.CAP_BILINEAR)
            {
                m_mcomBeginDecodeTargetAccessInput.scalingMode = MCOM_DECODE_TARGET_ACCESS_SCALING_MODE_BILINEAR;
                m_mcomBeginDecodeTargetAccessInput.scaledWidth = m_uScaledWidth; 
                m_mcomBeginDecodeTargetAccessInput.scaledHeight = m_uScaledHeight;
            }
            else
            {
                m_mcomBeginDecodeTargetAccessInput.scalingMode = MCOM_DECODE_TARGET_ACCESS_SCALING_MODE_OFF; 
				m_mcomBeginDecodeTargetAccessInput.scaledWidth = 0; 
				m_mcomBeginDecodeTargetAccessInput.scaledHeight = 0;
            }
        }
        else
		{
            m_mcomBeginDecodeTargetAccessInput.scalingMode = MCOM_DECODE_TARGET_ACCESS_SCALING_MODE_OFF; 
			m_mcomBeginDecodeTargetAccessInput.scaledWidth = 0; 
			m_mcomBeginDecodeTargetAccessInput.scaledHeight = 0;
			
		}

		
        DP_DBG(("CATIFastReadback::Open(): Src- [%d, %d], InputInterlaceFlags:[%d, %d], CapsIntelace- %d, DoScalingMode- [%d, %d, %d]\n", 
            m_uSrcWidth, m_uSrcHeight,
            pReadbackInit->dwInputInterlaceFlag,
            pReadbackInit->dwOutputInterlaceFlag,
            m_mcomDecodeTargetAccessCapsOutput.ScalingCaps.CAP_INTERLACED,
            m_mcomBeginDecodeTargetAccessInput.scalingMode, 
            m_mcomBeginDecodeTargetAccessInput.scaledWidth,
            m_mcomBeginDecodeTargetAccessInput.scaledHeight));

        m_mcomBeginDecodeTargetAccessOutput.size = sizeof(MCOM_BEGIN_DECODE_TARGET_ACCESS_OUTPUT); 

        mcomStatus = MCOMBeginDecodeTargetAccess(&m_mcomBeginDecodeTargetAccessInput, &m_mcomBeginDecodeTargetAccessOutput); 
        if(mcomStatus == MCOM_OK) 
        { 
            if(m_bUseScaling && m_bMcomScalingSupported) 
            {
                m_uScaledPitch = m_mcomBeginDecodeTargetAccessOutput.outputPitch; 	// Store the output pitch for use later 
            }

            return S_OK;
        } 
    }

    return E_FAIL;

}

HRESULT CATIFastReadback::Readback(IDirect3DSurface9* pSurface, DWORD dwIndex, HVDService::HVDDxva2UncompBufLockInfo* pInfo, HVDService::HVDFastReadbackOption* pReadbackOption)
{
    // This works whether MCOM decode target access is supported or not 
    // It will just be slower without MCOM decode target access support 
    // Scaling will not be supported, but that is accounted for in m_bMcomScalingSupported 

    CHECK_POINTER(pSurface);
    CAutoLock lock(&m_csObj);
    HRESULT hr = S_OK;

    COMPUTE_READBACK_TIME_INIT();

    if (pReadbackOption != NULL && pReadbackOption->dwReturnFourcc == FALSE)
    {
        pReadbackOption->pReturnSurface = pSurface;
    }
    else
    {
        CHECK_POINTER(pInfo);
        D3DLOCKED_RECT d3dlockRect = {0}; 
        BOOL bUpSampling = FALSE;

        hr = pSurface->LockRect(&d3dlockRect, NULL, 0);
        if(SUCCEEDED(hr)) 
        { 
            UINT uBufPitch = 0; 

            if((m_bUseScaling && m_bMcomScalingSupported) || bUpSampling) 
            { 
                pReadbackOption->wWidth = m_uScaledWidth;
                pReadbackOption->wHeight = m_uScaledHeight;
                uBufPitch = m_uScaledPitch; 
            } 
            else  
            {
                pReadbackOption->wWidth = m_uSrcWidth;
				pReadbackOption->wHeight = m_uSrcHeight==1088 ? 1080 : m_uSrcHeight;
                uBufPitch = d3dlockRect.Pitch; 
            }

            if (m_mcomBeginDecodeTargetAccessInput.outputMode == MCOM_DECODE_TARGET_ACCESS_OUTPUT_MODE_NV12)
            {
                pInfo->dwFourCC = MAKEFOURCC('N','V','1','2');
            }
            else if (m_mcomBeginDecodeTargetAccessInput.outputMode == MCOM_DECODE_TARGET_ACCESS_OUTPUT_MODE_YV12)
            {
                pInfo->dwFourCC = MAKEFOURCC('Y','V','1','2');
            }
            else if (m_mcomBeginDecodeTargetAccessInput.outputMode == MCOM_DECODE_TARGET_ACCESS_OUTPUT_MODE_YUY2)
            {
                uBufPitch = m_mcomBeginDecodeTargetAccessOutput.outputPitch;
                pInfo->dwFourCC = MAKEFOURCC('Y','U','Y','2');	
            }

            if(m_bUsePicthCopy && pInfo->dwFourCC==MAKEFOURCC('N','V','1','2'))
            {
                CopyNV12((char*)pInfo->pBits, (char*)d3dlockRect.pBits, pReadbackOption->wWidth, pReadbackOption->wHeight, uBufPitch);
                pInfo->uPitch = pReadbackOption->wWidth;
            }
            else
            {
                pInfo->pBits = d3dlockRect.pBits;
                pInfo->uPitch = uBufPitch;
            }

            hr = pSurface->UnlockRect();
        } 
    }
    pReadbackOption->bOutputProgressive = pReadbackOption->bInputProgressive;

    COMPUTE_READBACK_TIME("ATI MCOM");

    return hr;
}

HRESULT CATIFastReadback::Close()
{
    MCOM_STATUS mcomStatus = MCOM_OK; 

    if(m_bMcomDecodeTargetAccessSupport) 
    { 
        // Configure decode target access 
        MCOM_END_DECODE_TARGET_ACCESS_INPUT mcomEndDecodeTargetAccessInput = {0}; 

        // pVideoDecoder is the IDirectXVideoDecoder* for DXVA2 
        mcomEndDecodeTargetAccessInput.size = sizeof(MCOM_END_DECODE_TARGET_ACCESS_INPUT); 
        mcomEndDecodeTargetAccessInput.MCOMSession = m_pMcomSession; 
        mcomEndDecodeTargetAccessInput.DecodeSession = m_pVidDec; 

        MCOM_END_DECODE_TARGET_ACCESS_OUTPUT mcomEndDecodeTargetAccessOutput = {0}; 
        mcomEndDecodeTargetAccessOutput.size = sizeof(MCOM_END_DECODE_TARGET_ACCESS_OUTPUT); 

        mcomStatus = MCOMEndDecodeTargetAccess( &mcomEndDecodeTargetAccessInput, &mcomEndDecodeTargetAccessOutput); 
        if(mcomStatus != MCOM_OK) 
        {
            DP_FASTREADBACK("MCOMEndDecodeTargetAccess failed");
        }
    } 

    if(m_pMcomSession != NULL) 
    { 
        // Destroy MCOM Session (Driver retains the settings) 
        mcomStatus = MCOMDestroy(m_pMcomSession); 
        if(mcomStatus != MCOM_OK) 
        {
            DP_FASTREADBACK("MCOMDestroy failed");
        }
        m_pMcomSession = NULL;
    } 
    return S_OK;
}

HRESULT CATIFastReadback::GetCaps(DWORD dwCapType, void* pvdCaps)
{
    HRESULT hRet = S_OK;

    if(!pvdCaps)
        return E_POINTER;

    if(dwCapType==E_CAPS_TYPE_SCALING)
        memcpy(pvdCaps, &(m_mcomDecodeTargetAccessCapsOutput.ScalingCaps), sizeof(MCOM_DECODE_TARGET_ACCESS_SCALING_CAPS));
    else
        hRet = E_FAIL;

    return hRet;
}