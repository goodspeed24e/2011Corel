#include "stdafx.h"
#include "FastCompositingDevice.h"
#include<string.h>
#include <windows.h>
#include <stdio.h>
#include <Psapi.h>

static const FASTCOMP_CAPS2 Caps2_Default = 
{
    0,              // 0 used to indicate defaults
    0x01,           // TargetInterlacingModes  = progressive output only
    FALSE,          // bTargetInSysMemory
    FALSE,          // bProcampControl
    FALSE,          // bDeinterlacingControl
    FALSE,          // bDenoiseControl
    FALSE,          // bDetailControl
    FALSE,          // bFmdControl
    FALSE,          // bVariances
    FALSE,          // bSceneDetection
    0
};

static const DXVA2_AYUVSample16 Background_default =
{   // Black Bg
    0x8000,         // Cr
    0x8000,         // Cb
    0x0000,         // Y
    0xffff          // Alpha
    };

static const DXVA2_ProcAmpValues ProcAmpValues_default =
{ //Fraction,Value
    { 0x0000, 0x00 },       // Brightness
    { 0x0000, 0x01 },       // Contrast
    { 0x0000, 0x00 },       // Hue
    { 0x0000, 0x01 }        // Saturation
};

static const DXVA2_ExtendedFormat DXVA2_ExtendedFormat_default = 
{
    DXVA2_SampleProgressiveFrame,           // SampleFormat
    DXVA2_VideoChromaSubsampling_MPEG2,     // VideoChromaSubsampling
    DXVA2_NominalRange_Normal,              // NominalRange
    DXVA2_VideoTransferMatrix_BT709,        // VideoTransferMatrix
    DXVA2_VideoLighting_dim,                // VideoLighting
    DXVA2_VideoPrimaries_BT709,             // VideoPrimaries
    DXVA2_VideoTransFunc_709                // VideoTransferFunction
};

static const FASTCOMP_BLT_PARAMS FastCompBlt_default =
{
    NULL,                               // pRenderTarget
    0,                                  // SampleCount
    NULL,                               // pSamples
    0,                                  // TargetFrame
    { 0, 0, 0, 0 },                     // TargetRect
    FALSE,                              // ReservedCR
    DXVA2_VideoTransferMatrix_BT709,    // TargetMatrix
    FALSE,                              // TargetExtGamut
    FASTCOMP_TARGET_PROGRESSIVE,        // TargetIntMode
    FALSE,                              // TargetInSysMem
    0,                                  // Reserved1
    sizeof(FASTCOMP_BLT_PARAMS),        // iSizeOfStructure
    0,                                  // Reserved2
    NULL,                               // pReserved1
    Background_default,                 // BackgroundColor
    NULL,                               // pReserved2
    // Rev 1.4 parameters
    ProcAmpValues_default,              // ProcAmpValues
    FALSE,                              // bDenoiseAutoAdjust
    FALSE,                              // bFMDEnable
    FALSE,                              // bSceneDetectionEnable
    FASTCOMP_DI_NONE,                   // iDeinterlacingAlgorithm
    0,                                  // Reserved3
    0,                                  // wDenoiseFactor
    0,                                  // wDetailFactor
    0,                                  // wSpatialComplexity
    0,                                  // wTemporalComplexity
    0,                                  // iVarianceType
    NULL,                               // pVariances
    0,                                  // iCadence
    FALSE,                              // bRepeatFrame
    0,                                  // Reserved4
    0                                   // iFrameNum
};

static DWORD ReadiEncoderSettingFromInI(char*  key, DWORD dwDefault);

CFastCompositingDevice::CFastCompositingDevice(
	HMODULE hComDXVA2,
    IDirect3DDevice9 *pD3DDevice9,
    FASTCOMP_MODE    iMode) :
    CIntelAuxiliaryDevice(pD3DDevice9, hComDXVA2),
    bIsPresent(FALSE),
    bIsRunning(FALSE),
    m_pVPservice(NULL),
    m_pRegistrationDevice(NULL),
    m_pDummySurface(NULL)
{
    HRESULT hRes   = S_OK;
    UINT    i, uGuids;
    GUID   *pGUIDs = NULL;
    DXVA2_VideoDesc VideoDesc;

    // Query Acceleration GUIDs
    hRes = QueryAccelGuids(&pGUIDs, &uGuids);
    if (FAILED(hRes))
    {
        DBGMSG((TEXT("Failed to obtain acceleration devices GUIDs (code 0x%x).\n"), hRes));
        goto cleanup;
    }

    // Check if Fast Compositing GUID is present in the system
    for (i = 0; i < uGuids; i++)
    {
        if (IsEqualGUID(pGUIDs[i], DXVA2_FastCompositing))
        {
            bIsPresent = TRUE;
            break;
        }
    }

    // Validate mode
    if (iMode != FASTCOMP_MODE_COMPOSITING &&
        iMode != FASTCOMP_MODE_PRE_PROCESS)
    {
        hRes = E_FAIL;
        DBGMSG((TEXT("Invalid Fast Compositing mode\n")));
        goto cleanup;
    }
    m_iMode = iMode;

    // Create Video Processor Service
    if (bIsPresent)
    {
#if 1
		TpfnDXVA2CreateVideoService pfnComDXVA2CreateVideoService = (TpfnDXVA2CreateVideoService)GetProcAddress(hComDXVA2, "DXVA2CreateVideoService");

		hRes = pfnComDXVA2CreateVideoService(pD3DDevice9, IID_IDirectXVideoProcessorService, (VOID**)&m_pVPservice);
#else
        hRes = DXVA2CreateVideoService(pD3DDevice9,
                                       IID_IDirectXVideoProcessorService,
                                       (VOID**)&m_pVPservice);
#endif
        if (FAILED(hRes))
        {
            DBGMSG((TEXT("DXVA2CreateVideoService failed with error 0x%x.\n"), hRes));
            bIsPresent = FALSE;
        }
    }

    VideoDesc.SampleWidth                         = 16;
    VideoDesc.SampleHeight                        = 16;
    VideoDesc.SampleFormat.VideoChromaSubsampling = DXVA2_VideoChromaSubsampling_MPEG2;
    VideoDesc.SampleFormat.NominalRange           = DXVA2_NominalRange_16_235;
    VideoDesc.SampleFormat.VideoTransferMatrix    = DXVA2_VideoTransferMatrix_BT709;
    VideoDesc.SampleFormat.VideoLighting          = DXVA2_VideoLighting_dim;
    VideoDesc.SampleFormat.VideoPrimaries         = DXVA2_VideoPrimaries_BT709;
    VideoDesc.SampleFormat.VideoTransferFunction  = DXVA2_VideoTransFunc_709;
    VideoDesc.SampleFormat.SampleFormat           = DXVA2_SampleProgressiveFrame;
    VideoDesc.Format                              = D3DFMT_YUY2;
    VideoDesc.InputSampleFreq.Numerator           = 60;
    VideoDesc.InputSampleFreq.Denominator         = 1;
    VideoDesc.OutputFrameFreq.Numerator           = 60;
    VideoDesc.OutputFrameFreq.Denominator         = 1;

    // Create Registration Device
    if (bIsPresent)
    {
        hRes = m_pVPservice->CreateVideoProcessor(DXVA2_Registration_Device,
                                                 &VideoDesc,
                                                  D3DFMT_YUY2,
                                                  1,
                                                 &m_pRegistrationDevice);

        if (FAILED(hRes))
        {
            DBGMSG((TEXT("CreateVideoProcessor failed with error 0x%x.\n"), hRes));
            bIsPresent = FALSE;
        }
    }

    if (!bIsPresent)
    {
        DBGMSG((TEXT("CFastCompositingDevice::CFastCompositingDevice: Service not available\n")));
    }
    else
    {
        FASTCOMP_CAPS  *pCaps  = NULL;
        FASTCOMP_CAPS2 *pCaps2 = NULL;

        // Get Caps
        if (!FAILED(QueryCaps(&pCaps)) && pCaps != NULL)
        {
            m_Caps = *pCaps;
            free(pCaps);
        }

        // Get Caps2
        if (!FAILED(QueryCaps2(&pCaps2)) && pCaps2 != NULL)
        {
            m_Caps2 = *pCaps2;
            free(pCaps2);
        }
    }

        // Create dummy surface for registration
    hRes = m_pVPservice->CreateSurface(
                          100,
                          100,
                          0,
                          D3DFMT_YUY2,
                          D3DPOOL_DEFAULT,
                          0,
                          DXVA2_VideoProcessorRenderTarget,
                          &m_pDummySurface,
                          NULL);

    m_DiAlgorithm = (FASTCOMP_DI_ALGORITHM)ReadiEncoderSettingFromInI("DIMODE", FASTCOMP_DI_BOB);

cleanup:
    if (pGUIDs) free(pGUIDs);
}

CFastCompositingDevice::~CFastCompositingDevice()
{
    if (m_pDummySurface)
    {
        m_pDummySurface->Release();
    }

    if (m_pRegistrationDevice)
    {
        m_pRegistrationDevice->Release();
        m_pRegistrationDevice = NULL;
    }

    if (m_pVPservice)
    {
        m_pVPservice->Release();
        m_pVPservice = NULL;
    }
}

HRESULT CFastCompositingDevice::CreateSurface(
    UINT                Width,
    UINT                Height,
    UINT                BackBuffers, 
    D3DFORMAT           Format,
    D3DPOOL             Pool,
    DWORD               Usage,
    DWORD               DxvaType,
    IDirect3DSurface9** ppSurface,
    HANDLE            * pSharedHandle)
{
    if (m_pVPservice)
    {
        return m_pVPservice->CreateSurface(Width, Height, BackBuffers,
                                           Format, Pool, Usage, DxvaType,
                                           ppSurface, pSharedHandle);
    }

    return S_FALSE;
}

HRESULT CFastCompositingDevice::QueryCaps(FASTCOMP_CAPS **ppCaps)
{
    HRESULT hRes;
    UINT    uQuerySize;
    FASTCOMP_QUERYCAPS sQuery;

    if ((!bIsPresent) || bIsRunning)
    {
        if (bIsRunning)
        {
            DBGMSG((TEXT("CFastCompositingDevice::QueryCaps: Service already running\n")));
        }
        else
        {
            DBGMSG((TEXT("CFastCompositingDevice::QueryCaps: Service not available\n")));
        }

        return E_FAIL;
    }

    if (!ppCaps)
    {
        return E_INVALIDARG;
    }

    if (m_iMode == FASTCOMP_MODE_PRE_PROCESS)
    {
        sQuery.Type = FASTCOMP_QUERY_VPP_CAPS;
    }
    else
    {
        sQuery.Type = FASTCOMP_QUERY_CAPS;
    }

    uQuerySize  = sizeof(sQuery);
    hRes = QueryAccelCaps(&DXVA2_FastCompositing, &sQuery, &uQuerySize);
    if (!FAILED(hRes))
    {
        *ppCaps   = (FASTCOMP_CAPS *) malloc(sizeof(FASTCOMP_CAPS));
        if (*ppCaps == NULL)
        {
            DBGMSG((TEXT("CFastCompositingDevice::QueryCaps: Failed to allocate memory\n")));
            return E_FAIL;
        }

        memcpy(*ppCaps, &sQuery.sCaps, sizeof(FASTCOMP_CAPS));
    }

    return hRes;
}

HRESULT CFastCompositingDevice::QueryCaps2(FASTCOMP_CAPS2 **ppCaps2)
{
    HRESULT             hRes;
    UINT                uQuerySize;
    FASTCOMP_QUERYCAPS  sQuery;

    if ((!bIsPresent) || bIsRunning)
    {
        if (bIsRunning)
        {
            DBGMSG((TEXT("CFastCompositingDevice::QueryCaps: Service already running\n")));
        }
        else
        {
            DBGMSG((TEXT("CFastCompositingDevice::QueryCaps: Service not available\n")));
        }

        return E_FAIL;
    }

    if (!ppCaps2)
    {
        return E_INVALIDARG;
    }

    if (m_iMode == FASTCOMP_MODE_PRE_PROCESS)
    {
        sQuery.Type = FASTCOMP_QUERY_VPP_CAPS2;
    }
    else
    {
        sQuery.Type = FASTCOMP_QUERY_CAPS2;
    }

    // Set query size, for compatibility
    sQuery.sCaps2.dwSize = uQuerySize = sizeof(sQuery);

    // Query caps2
    hRes = QueryAccelCaps(&DXVA2_FastCompositing, &sQuery, &uQuerySize);

    // Allocate caps2
    *ppCaps2  = (FASTCOMP_CAPS2 *) malloc(sizeof(FASTCOMP_CAPS2));
    if (*ppCaps2 == NULL)
    {
        DBGMSG((TEXT("CFastCompositingDevice::QueryCaps: Failed to allocate memory\n")));
        return E_FAIL;
    }

    // Get query data
    if (!FAILED(hRes))
    {
        memcpy(*ppCaps2, &sQuery.sCaps2, sizeof(FASTCOMP_CAPS2));
    }
    else
    {
        (*ppCaps2)->dwSize = 0;
    }

    // Set defaults values
    if ((*ppCaps2)->dwSize < sizeof(FASTCOMP_CAPS2))
    {
        **ppCaps2 = Caps2_Default;
    }

    return S_OK;
}

HRESULT CFastCompositingDevice::QueryFormats(FASTCOMP_SAMPLE_FORMATS **ppFormats)
{
    HRESULT hRes;

    UINT                uTotal;
    D3DFORMAT          *pFormatArray;
    FASTCOMP_QUERYCAPS  sQuery;
    UINT                uQuerySize;
    FASTCOMP_SAMPLE_FORMATS *pFormats = NULL;

    if ((!bIsPresent) || bIsRunning)
    {
        return E_FAIL;
    }

    if (!ppFormats)
    {
        return E_INVALIDARG;
    }

    // Query format counts
    if (m_iMode == FASTCOMP_MODE_PRE_PROCESS)
    {
        sQuery.Type = FASTCOMP_QUERY_VPP_FORMAT_COUNT;
    }
    else
    {
        sQuery.Type = FASTCOMP_QUERY_FORMAT_COUNT;
    }

    uQuerySize  = sizeof(sQuery);
    hRes = QueryAccelCaps(&DXVA2_FastCompositing, &sQuery, &uQuerySize);
    if (FAILED(hRes))
    {
        DBGMSG((TEXT("CFastCompositingDevice::QueryFormats: Failed to obtain remaining video format count\n")));
        goto cleanup;
    }

    // Allocate a structure and extra area to hold all format arrays
    uTotal  = sQuery.sFmtCount.iPrimaryFormats;
    uTotal += sQuery.sFmtCount.iSecondaryFormats;
    uTotal += sQuery.sFmtCount.iSubstreamFormats;
    uTotal += sQuery.sFmtCount.iGraphicsFormats;
    uTotal += sQuery.sFmtCount.iRenderTargetFormats;
    uTotal += sQuery.sFmtCount.iBackgroundFormats;
    pFormats = (FASTCOMP_SAMPLE_FORMATS *) malloc(sizeof(FASTCOMP_SAMPLE_FORMATS) + uTotal * sizeof(D3DFORMAT) + 8);
   
    if (!pFormats)
    {
        DBGMSG((TEXT("CFastCompositingDevice::QueryFormats: Failed to allocate memory\n")));
        hRes = E_FAIL;
        goto cleanup;
    }

    // Initialize the output structure
    pFormatArray = (D3DFORMAT *) (pFormats + 1);

    pFormats->iPrimaryVideoFormatCount   = sQuery.sFmtCount.iPrimaryFormats;
    pFormats->pPrimaryVideoFormats       = (sQuery.sFmtCount.iPrimaryFormats) ? pFormatArray : NULL;
    pFormatArray += sQuery.sFmtCount.iSecondaryFormats;

    pFormats->iSecondaryVideoFormatCount = sQuery.sFmtCount.iSecondaryFormats;
    pFormats->pSecondaryVideoFormats     = (sQuery.sFmtCount.iSecondaryFormats) ? pFormatArray : NULL;
    pFormatArray += sQuery.sFmtCount.iSecondaryFormats;

    pFormats->iSubstreamFormatCount      = sQuery.sFmtCount.iSubstreamFormats;
    pFormats->pSubstreamFormats          = (sQuery.sFmtCount.iSubstreamFormats) ? pFormatArray : NULL;
    pFormatArray += sQuery.sFmtCount.iSubstreamFormats;

    pFormats->iGraphicsFormatCount       = sQuery.sFmtCount.iGraphicsFormats;
    pFormats->pGraphicsFormats           = (sQuery.sFmtCount.iGraphicsFormats) ? pFormatArray : NULL;
    pFormatArray += sQuery.sFmtCount.iGraphicsFormats;

    pFormats->iRenderTargetFormatCount   = sQuery.sFmtCount.iRenderTargetFormats;
    pFormats->pRenderTargetFormats       = (sQuery.sFmtCount.iRenderTargetFormats) ? pFormatArray : NULL;
    pFormatArray += sQuery.sFmtCount.iRenderTargetFormats;

    pFormats->iBackgroundFormatCount     = sQuery.sFmtCount.iBackgroundFormats;
    pFormats->pBackgroundFormats         = (sQuery.sFmtCount.iBackgroundFormats) ? pFormatArray : NULL;

    // Query remaining formats
    if (m_iMode == FASTCOMP_MODE_PRE_PROCESS)
    {
        sQuery.Type = FASTCOMP_QUERY_VPP_FORMATS;
    }
    else
    {
        sQuery.Type = FASTCOMP_QUERY_FORMATS;
    }

    sQuery.sFormats.iPrimaryFormatSize      = pFormats->iPrimaryVideoFormatCount   * sizeof(D3DFORMAT);
    sQuery.sFormats.iSecondaryFormatSize    = pFormats->iSecondaryVideoFormatCount * sizeof(D3DFORMAT);
    sQuery.sFormats.iSubstreamFormatSize    = pFormats->iSubstreamFormatCount      * sizeof(D3DFORMAT);
    sQuery.sFormats.iGraphicsFormatSize     = pFormats->iGraphicsFormatCount       * sizeof(D3DFORMAT);
    sQuery.sFormats.iRenderTargetFormatSize = pFormats->iRenderTargetFormatCount   * sizeof(D3DFORMAT);
    sQuery.sFormats.iBackgroundFormatSize   = pFormats->iBackgroundFormatCount     * sizeof(D3DFORMAT);
    sQuery.sFormats.pPrimaryFormats         = pFormats->pPrimaryVideoFormats;
    sQuery.sFormats.pSecondaryFormats       = pFormats->pSecondaryVideoFormats;
    sQuery.sFormats.pSubstreamFormats       = pFormats->pSubstreamFormats;
    sQuery.sFormats.pGraphicsFormats        = pFormats->pGraphicsFormats;
    sQuery.sFormats.pRenderTargetFormats    = pFormats->pRenderTargetFormats;
    sQuery.sFormats.pBackgroundFormats      = pFormats->pBackgroundFormats;
    uQuerySize  = sizeof(sQuery);

    hRes = QueryAccelCaps(&DXVA2_FastCompositing, &sQuery, &uQuerySize);
    if (FAILED(hRes))
    {
        DBGMSG((TEXT("CFastCompositingDevice::QueryFormats: Failed to obtain remaining video formats\n")));
    }

cleanup:
    if (FAILED(hRes))
    {
        if (pFormats) free(pFormats);
        pFormats = NULL;
    }

    *ppFormats = pFormats;
    return hRes;
}

int CFastCompositingDevice::QueryFrameRate(int iMaxLayers, int iMaxSrcWidth, int iMaxSrcHeight, int iMaxDstWidth, int iMaxDstHeight)
{
    FASTCOMP_QUERYCAPS  sQuery;
    UINT                uQuerySize;

    sQuery.Type = FASTCOMP_QUERY_FRAME_RATE;
    sQuery.sFrameRate.iMaxSrcWidth  = iMaxSrcWidth;
    sQuery.sFrameRate.iMaxSrcHeight = iMaxSrcHeight;
    sQuery.sFrameRate.iMaxDstWidth  = iMaxDstWidth;
    sQuery.sFrameRate.iMaxDstHeight = iMaxDstHeight; 
    sQuery.sFrameRate.iMaxLayers    = iMaxLayers;    
    sQuery.sFrameRate.iFrameRate    = -1;  // Output

    uQuerySize = sizeof(sQuery);

    HRESULT hRes = QueryAccelCaps(&DXVA2_FastCompositing, &sQuery, &uQuerySize);
    if (FAILED(hRes))
    {
        DBGMSG((TEXT("CFastCompositingDevice::QueryFrameRate: Failed to query frame rate\n")));
    }

    return (sQuery.sFrameRate.iFrameRate);
}

HRESULT CFastCompositingDevice::CreateService(
    DXVA2_VideoDesc *pVideoDesc,
    D3DFORMAT       TargetFormat,
    UINT            iSubstreams)
{
    HRESULT hRes;
    FASTCOMP_CREATEDEVICE  sCreateStruct = { pVideoDesc, TargetFormat, iSubstreams, m_iMode };
    UINT  uCreateSize = sizeof(sCreateStruct);

    if ((!bIsPresent) || bIsRunning)
    {
        if (bIsRunning)
        {
            DBGMSG((TEXT("CFastCompositingDevice::CreateService: Service already running\n")));
        }
        else
        {
            DBGMSG((TEXT("CFastCompositingDevice::CreateService: Service not available\n")));
        }

        return E_FAIL;
    }

    hRes = CreateAccelService(&DXVA2_FastCompositing, &sCreateStruct, &uCreateSize);
    if (FAILED(hRes))
    {
        DBGMSG((TEXT("CFastCompositingDevice::CreateService: Failed to create service (code 0x%x).\n"), hRes));
    }
    else
    {
        bIsRunning = TRUE;
    }

    // Obtain registration handle
    FASTCOMP_QUERYCAPS sQuery;
    UINT                 uQuerySize;

    sQuery.Type = FASTCOMP_QUERY_REGISTRATION_HANDLE;
    uQuerySize  = sizeof(sQuery);

    hRes = QueryAccelCaps(&DXVA2_FastCompositing, &sQuery, &uQuerySize);
    m_hRegistration = FAILED(hRes) ? NULL : sQuery.hRegistration;

    return hRes;
}

HRESULT CFastCompositingDevice::Register(
    IDirect3DSurface9   **pSources,
    INT                 iCount,
    BOOL                bRegister)  // True to Register; False for Unregister
{
    INT     i;
    HRESULT hRes;

    DXVA2_SURFACE_REGISTRATION RegRequest;
    DXVA2_SAMPLE_REG           RegEntry[33];

    DXVA2_VideoProcessBltParams BltParams;
    DXVA2_VideoSample           Samples[32];

    if (!bIsRunning)
    {
        DBGMSG((TEXT("CFastCompositingDevice::FastCompositingBlt: Service has not been initialized\n")));
        return E_FAIL;
    }

    // Create a surface registration request 
    // (note: this may be performed only once, when the surface is created, 
    //        before the first call to FastCompositingBlt)
    RegRequest.RegHandle            = m_hRegistration;
    RegRequest.pRenderTargetRequest = &RegEntry[0];
    RegRequest.nSamples             = iCount;
    RegRequest.pSamplesRequest      = &RegEntry[1];

    RegEntry[0].pD3DSurface = m_pDummySurface;
    RegEntry[0].Operation   = REG_IGNORE;
    for (i = 0; i < iCount; i++)
    {
        RegEntry[i + 1].pD3DSurface = pSources[i];
        RegEntry[i + 1].Operation   = (bRegister) ? REG_REGISTER : REG_UNREGISTER;
    }

    // Prepare VideoProcessBlt paramaters to the registration device
    memset(&BltParams, 0, sizeof(BltParams));
    memset(Samples   , 0, sizeof(Samples));
    BltParams.TargetFrame = (REFERENCE_TIME) (&RegRequest);
    BltParams.TargetRect.left   = 0;
    BltParams.TargetRect.top    = 0;
    BltParams.TargetRect.right  = 100;
    BltParams.TargetRect.bottom = 100;
    BltParams.StreamingFlags    = 0;
    BltParams.BackgroundColor   = Background_default;
    BltParams.DestFormat        = DXVA2_ExtendedFormat_default;
    BltParams.Alpha.Value       = 0;
    BltParams.Alpha.Fraction    = 0;
    BltParams.DestData          = 0;
    for (i = 0; i < iCount; i++)
    {
        Samples[i].Start        = (REFERENCE_TIME) (&RegRequest);  // pVideoCompositingBlt->pSamples[i].Start;
        Samples[i].End          = Samples[i].Start + 1;            // pVideoCompositingBlt->pSamples[i].End;
        Samples[i].SampleFormat = DXVA2_ExtendedFormat_default;
        Samples[i].SrcSurface   = pSources[i];
        Samples[i].SrcRect      = BltParams.TargetRect;
        Samples[i].DstRect      = BltParams.TargetRect;
        Samples[i].SampleData   = 0;
    }

    // Register surfaces for the FastCompositing device
    hRes = m_pRegistrationDevice->VideoProcessBlt(m_pDummySurface,
                                                  &BltParams,
                                                  Samples,
                                                  iCount,
                                                  NULL);

    return hRes;
}

VOID CFastCompositingDevice::ResetBltParams (FASTCOMP_BLT_PARAMS *pBlt)
{
    if (pBlt)
    {
        *pBlt = FastCompBlt_default;
    }
}

HRESULT CFastCompositingDevice::FastCopy(
    IDirect3DSurface9   *pTarget,
    IDirect3DSurface9   *pSource,
    RECT                *pTargetRect,
    RECT                *pSourceRect,
    BOOL                bInterlacedInput,
    BOOL                bInterlacedOutput,
    DWORD               dwField)
{
    HRESULT hRes;
    D3DSURFACE_DESC             SurfDesc;
    DXVA2_DecodeExecuteParams   sExecute;
    DXVA2_DecodeExtensionData   sExtension;
    FASTCOMP_BLT_PARAMS         VideoCompositingBlt = FastCompBlt_default;
    IDirect3DSurface9           *pSourceArray[]  = { pTarget, pSource };
    FASTCOMP_VideoSample        Samples[1];

    if (!bIsRunning)
    {
        DBGMSG((TEXT("CFastCompositingDevice::FastCopy: Service has not been initialized\n")));
        return E_FAIL;
    }

    // Check Caps
    if (!m_Caps2.bTargetInSysMemory)
    {
        DBGMSG((TEXT("CFastCompositingDevice::FastCopy: No support for Target in system memory\n")));
        return E_FAIL;
    }

    // Check target surface
    pTarget->GetDesc(&SurfDesc);
    if (SurfDesc.Pool != D3DPOOL_SYSTEMMEM)
    {
        DBGMSG((TEXT("CFastCompositingDevice::FastCopy: Target not in system memory\n")));
#if 0 // For SandyBridge, the result is put into VGA memory for it can support HW encoding. No need to send back to system memory
        return E_FAIL;
#endif
    }

    // Set output mode and deinterlacing algorithm
    if (m_Caps2.TargetInterlacingModes == FASTCOMP_TARGET_PROGRESSIVE_MASK)
    {
        VideoCompositingBlt.TargetIntMode = FASTCOMP_TARGET_PROGRESSIVE;
        if (bInterlacedInput)
        {
            VideoCompositingBlt.iDeinterlacingAlgorithm = m_DiAlgorithm;
        }
        else
        {
            VideoCompositingBlt.iDeinterlacingAlgorithm = FASTCOMP_DI_NONE;
        }
    }
    else if (m_Caps2.TargetInterlacingModes == FASTCOMP_TARGET_NO_DEINTERLACING_MASK)
    {
        VideoCompositingBlt.TargetIntMode = FASTCOMP_TARGET_NO_DEINTERLACING;
        VideoCompositingBlt.iDeinterlacingAlgorithm = FASTCOMP_DI_NONE;
    }
    else if (m_Caps2.TargetInterlacingModes ==
        (FASTCOMP_TARGET_PROGRESSIVE_MASK | FASTCOMP_TARGET_NO_DEINTERLACING_MASK))
    {
        if (bInterlacedOutput)
        {
            VideoCompositingBlt.TargetIntMode = FASTCOMP_TARGET_NO_DEINTERLACING;
            VideoCompositingBlt.iDeinterlacingAlgorithm = FASTCOMP_DI_NONE;
        }
        else
        {
            VideoCompositingBlt.TargetIntMode = FASTCOMP_TARGET_PROGRESSIVE;
            if (bInterlacedInput)
            {
                VideoCompositingBlt.iDeinterlacingAlgorithm = m_DiAlgorithm;
            }
            else
            {
                VideoCompositingBlt.iDeinterlacingAlgorithm = FASTCOMP_DI_NONE;
            }
        }
    }
    else
    {
        DBGMSG((TEXT("CFastCompositingDevice::FastCopy: Unknown Supported TargetInterlacingModes\n")));
        return E_FAIL;
    }

    // Check support for Bob or ADI Deinterlacing
    if (VideoCompositingBlt.iDeinterlacingAlgorithm == FASTCOMP_DI_BOB || VideoCompositingBlt.iDeinterlacingAlgorithm == FASTCOMP_DI_ADI)
    {
        if (!m_Caps.sPrimaryVideoCaps.bSimpleDI)
        {
            DBGMSG((TEXT("CFastCompositingDevice::FastCopy: No support for Bob Deinterlacing\n")));
            return E_FAIL;
        }
    }

    // Check support for interlaced to interlaced copy
    if (bInterlacedOutput)
    {
        if (VideoCompositingBlt.TargetIntMode == FASTCOMP_TARGET_PROGRESSIVE)
        {
            DBGMSG((TEXT("CFastCompositingDevice::FastCopy: No support for interlaced output\n")));
            return E_FAIL;
        }

        if (!bInterlacedInput)
        {
            DBGMSG((TEXT("CFastCompositingDevice::FastCopy: No support for progressive input and interlaced output\n")));
            return E_FAIL;
        }

        // Check for interlaced scaling
        if (memcmp(pTargetRect, pSourceRect, sizeof(RECT)) &&
            m_Caps.sPrimaryVideoCaps.bInterlacedScaling == 0)
        {
            DBGMSG((TEXT("CFastCompositingDevice::FastCopy: Interlaced scaling not supported\n")));
            return E_FAIL;
        }
    }

    // Register Source/Target
    Register(pSourceArray, 2, TRUE);

    // FastCompositing Params for FAST COPY
    VideoCompositingBlt.pRenderTarget  = pTarget;
    VideoCompositingBlt.SampleCount    = 1;
    VideoCompositingBlt.pSamples       = Samples;
    if (VideoCompositingBlt.iDeinterlacingAlgorithm == FASTCOMP_DI_BOB || VideoCompositingBlt.iDeinterlacingAlgorithm == FASTCOMP_DI_ADI)
    {
        if (dwField == FASTCOMP_TOP_FIELD)
        {
            VideoCompositingBlt.TargetFrame = 0;
        }
        else // (dwField == FASTCOMP_BOTTOM_FIELD)
        {
            VideoCompositingBlt.TargetFrame = (0 + 1000) / 2;
        }
    }
    else
    {
        VideoCompositingBlt.TargetFrame = 0;
    }
    VideoCompositingBlt.TargetRect     = *pTargetRect;
    VideoCompositingBlt.TargetInSysMem = (SurfDesc.Pool==D3DPOOL_SYSTEMMEM);


    // Set main video
    ZeroMemory(Samples, sizeof(Samples));
    Samples[0].Depth            = FASTCOMP_DEPTH_MAIN_VIDEO;
    Samples[0].SrcSurface       = pSource;
    Samples[0].SrcRect          = *pSourceRect;
    Samples[0].DstRect          = *pTargetRect;
    Samples[0].Start            = 0;        // Don't care
    Samples[0].End              = 1000;     // Don't care

    // Set interlaced/progressive sample flag
    Samples[0].SampleFormat     = DXVA2_ExtendedFormat_default;
    Samples[0].SampleFormat.SampleFormat = (bInterlacedInput) ?
                                               DXVA_SampleFieldInterleavedEvenFirst :
                                               DXVA_SampleProgressiveFrame;

    // FastCompositing Blt
    sExtension.Function              = FASTCOMP_BLT;
    sExtension.pPrivateInputData     = (PVOID)&VideoCompositingBlt;
    sExtension.PrivateInputDataSize  = sizeof(FASTCOMP_BLT_PARAMS);
    sExtension.pPrivateOutputData    = NULL;
    sExtension.PrivateOutputDataSize = 0;

    sExecute.NumCompBuffers     = 0;
    sExecute.pCompressedBuffers = 0;
    sExecute.pExtensionData     = &sExtension;

    hRes = m_pAuxiliaryDevice->Execute(&sExecute);

    return hRes;

}


HRESULT CFastCompositingDevice::FastCompositingBlt(FASTCOMP_BLT_PARAMS *pVideoCompositingBlt)
{
    HRESULT hRes;
    DXVA2_DecodeExecuteParams sExecute;
    DXVA2_DecodeExtensionData sExtension;

    if (!bIsRunning)
    {
        DBGMSG((TEXT("CFastCompositingDevice::FastCompositingBlt: Service has not been initialized\n")));
        return E_FAIL;
    }

    // FastCompositing Blt
    sExtension.Function              = FASTCOMP_BLT;
    sExtension.pPrivateInputData     = (PVOID)pVideoCompositingBlt;
    sExtension.PrivateInputDataSize  = sizeof(FASTCOMP_BLT_PARAMS);
    sExtension.pPrivateOutputData    = NULL;
    sExtension.PrivateOutputDataSize = 0;

    sExecute.NumCompBuffers     = 0;
    sExecute.pCompressedBuffers = 0;
    sExecute.pExtensionData     = &sExtension;

    hRes = m_pAuxiliaryDevice->Execute(&sExecute);

    return hRes;
}

#pragma warning(disable : 4996)
#pragma warning(disable : 4995)
static DWORD ReadiEncoderSettingFromInI(char*  key, DWORD dwDefault)
{
    DWORD dwPathReturnSize = MAX_PATH;
    char ConfigFileName[] = "iEncoder.ini";
    char ConfigPathFileName[MAX_PATH];
    
    memset(ConfigPathFileName, 0, sizeof(ConfigPathFileName));
    HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION|PROCESS_VM_READ, FALSE, GetCurrentProcessId());
    if(hProcess)
	{
		DWORD ret = GetModuleFileNameEx(hProcess, 0, &ConfigPathFileName[0], dwPathReturnSize);
	    CloseHandle(hProcess);
		if(ret==0) 
			return dwDefault; 
	}
    int i;
    for(i=strlen(ConfigPathFileName)-1; i>=0; i--)
    {
        if(ConfigPathFileName[i]=='\\')
        {
            i++;
            break;
        }
    }
    if(i==0)
			return dwDefault; 

    strcpy(&ConfigPathFileName[i], ConfigFileName);

	// examine if INI exists or not
	FILE *fp = fopen(ConfigPathFileName, "rb");
	if(fp==0)
            return dwDefault;
	fclose(fp);
	
#define READKEY(key, d_value)		GetPrivateProfileInt("HWVDecConfig", (key), (d_value), ConfigPathFileName)
	 
    DWORD dwRet = READKEY(key, dwDefault);

    return dwRet;
}

