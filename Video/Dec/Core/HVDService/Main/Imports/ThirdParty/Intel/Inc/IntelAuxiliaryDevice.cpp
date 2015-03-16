#include "stdafx.h"
#include "IntelAuxiliaryDevice.h"

CIntelAuxiliaryDevice::CIntelAuxiliaryDevice(IDirect3DDevice9 *pD3DDevice9, HMODULE hAuxDXVA2):
    m_pVideoDecoderService(NULL),
    m_pAuxiliaryDevice(NULL),
    m_pRenderTarget(NULL)
{
    HRESULT    hRes = S_OK;
    UINT       i, uiDecoders, uiFormats, uiConfigurations;
    GUID      *pGuidDecoders = NULL;
    D3DFORMAT *pFormats = NULL;

    DXVA2_ConfigPictureDecode *pPictureDecode = NULL;

#if 1	
	TpfnDXVA2CreateVideoService pfnAuxDXVA2CreateVideoService = (TpfnDXVA2CreateVideoService)GetProcAddress(hAuxDXVA2, "DXVA2CreateVideoService");
	hRes = pfnAuxDXVA2CreateVideoService(pD3DDevice9, IID_IDirectXVideoDecoderService, (VOID **)&m_pVideoDecoderService);
#else
    hRes = DXVA2CreateVideoService(pD3DDevice9,
                                   IID_IDirectXVideoDecoderService,
                                   (VOID **)&m_pVideoDecoderService);
#endif
	if (FAILED(hRes))
    {
        DBGMSG((TEXT("Failed to create VideoDecoderService (error 0x%x)\n"), hRes));
        return;
    }

    hRes = m_pVideoDecoderService->GetDecoderDeviceGuids(&uiDecoders, &pGuidDecoders);
    if (FAILED(hRes))
    {
        DBGMSG((TEXT("Failed to enumerate decoder GUIDs (error 0x%x)\n"), hRes));
        return;
    }

    for (i = 0; i < uiDecoders; i++)
    {
        if (IsEqualGUID(pGuidDecoders[i], DXVA2_Intel_Auxiliary_Device)) break;
    }

    CoTaskMemFree(pGuidDecoders);

    if (i >= uiDecoders)
    {
        DBGMSG((TEXT("Failed to find Intel Auxiliary Device Interface (error 0x%x)\n"), hRes));
        return;
    }

    hRes = m_pVideoDecoderService->GetDecoderRenderTargets(DXVA2_Intel_Auxiliary_Device,
                                                           &uiFormats,
                                                           &pFormats);
    if (FAILED(hRes))
    {
        DBGMSG((TEXT("GetDecoderRenderTargets failed (error 0x%x)\n"), hRes));
        return;
    }

    CoTaskMemFree(pFormats);

    m_sVideoDescr.SampleWidth                         = 64;
    m_sVideoDescr.SampleHeight                        = 64;
    m_sVideoDescr.SampleFormat.VideoChromaSubsampling = DXVA2_VideoChromaSubsampling_MPEG2;
    m_sVideoDescr.SampleFormat.NominalRange           = DXVA2_NominalRange_16_235;
    m_sVideoDescr.SampleFormat.VideoTransferMatrix    = DXVA2_VideoTransferMatrix_BT709;
    m_sVideoDescr.SampleFormat.VideoLighting          = DXVA2_VideoLighting_dim;
    m_sVideoDescr.SampleFormat.VideoPrimaries         = DXVA2_VideoPrimaries_BT709;
    m_sVideoDescr.SampleFormat.VideoTransferFunction  = DXVA2_VideoTransFunc_709;
    m_sVideoDescr.SampleFormat.SampleFormat           = DXVA2_SampleProgressiveFrame;
    m_sVideoDescr.Format                              = pFormats[0];
    m_sVideoDescr.InputSampleFreq.Numerator           = m_sVideoDescr.OutputFrameFreq.Numerator   = 60000;
    m_sVideoDescr.InputSampleFreq.Denominator         = m_sVideoDescr.OutputFrameFreq.Denominator = 1001;
    m_sVideoDescr.UABProtectionLevel                  = 0;
    m_sVideoDescr.Reserved                            = 0;

    hRes = m_pVideoDecoderService->CreateSurface(64,
                                                 64,
                                                 0,
                                                 (D3DFORMAT)MAKEFOURCC('I', 'M', 'C', '3'),
                                                 D3DPOOL_DEFAULT,
                                                 0,
                                                 DXVA2_VideoDecoderRenderTarget,
                                                 &m_pRenderTarget,
                                                 NULL);
    if (FAILED(hRes))
    {
        DBGMSG((TEXT("Failed to create output surface (error 0x%x)\n"), hRes));
        return;
    }

    hRes = m_pVideoDecoderService->GetDecoderConfigurations(DXVA2_Intel_Auxiliary_Device,
                                                            &m_sVideoDescr,
                                                            NULL,
                                                            &uiConfigurations,
                                                            &pPictureDecode);
    if (FAILED(hRes))
    {
        DBGMSG((TEXT("Failed to obtain decoder configuration (error 0x%x)\n"), hRes));
        return;
    }

    if (uiConfigurations != 1)
    {
        DBGMSG((TEXT("Obtained more than one configuration\n")));
        return;
    }
    m_sPictureDecode = *pPictureDecode;
    CoTaskMemFree(pPictureDecode);

    hRes = m_pVideoDecoderService->CreateVideoDecoder(DXVA2_Intel_Auxiliary_Device,
                                                      &m_sVideoDescr,
                                                      &m_sPictureDecode,
                                                      &m_pRenderTarget,
                                                      1,
                                                      &m_pAuxiliaryDevice);
    if (FAILED(hRes))
    {
        DBGMSG((TEXT("Failed to create the auxiliary device (error 0x%x)\n"), hRes));
        return;
    }

    return;
}


CIntelAuxiliaryDevice::~CIntelAuxiliaryDevice()
{
    if (m_pAuxiliaryDevice)
    {
        m_pAuxiliaryDevice->Release();
        m_pAuxiliaryDevice = NULL;
    }

    if (m_pRenderTarget)
    {
        m_pRenderTarget->Release();
        m_pRenderTarget = NULL;
    }

    if (m_pVideoDecoderService)
    {
        m_pVideoDecoderService->Release();
        m_pVideoDecoderService = NULL;
    }
}

HRESULT CIntelAuxiliaryDevice::QueryAccelGuids(GUID **ppAccelGuids, UINT *puAccelGuidCount)
{
    DXVA2_DecodeExecuteParams sExecute;
    DXVA2_DecodeExtensionData sExtension;

    HRESULT hRes       = E_INVALIDARG;
    UINT    uGuidCount = 0;
    GUID   *pGuids     = NULL;

    // Check parameters
    if (!puAccelGuidCount || !ppAccelGuids)
    {
        DBGMSG((TEXT("QueryAccelGuids: Invalid parameters\n")));
        goto cleanup;
    }

    // Check the decoder object
    if (!m_pAuxiliaryDevice)
    {
        DBGMSG((TEXT("QueryAccelGuids: Auxiliary Device is invalid\n")));
        goto cleanup;
    }

    // Get GUID count
    sExtension.Function              = AUXDEV_GET_ACCEL_GUID_COUNT;
    sExtension.pPrivateInputData     = NULL;
    sExtension.PrivateInputDataSize  = 0;
    sExtension.pPrivateOutputData    = &uGuidCount;
    sExtension.PrivateOutputDataSize = sizeof(int);

    sExecute.NumCompBuffers     = 0;
    sExecute.pCompressedBuffers = 0;
    sExecute.pExtensionData     = &sExtension;

    hRes = m_pAuxiliaryDevice->Execute(&sExecute);
    if (FAILED(hRes))
    {
        DBGMSG((TEXT("QueryAccelGuids: Failed to obtain GUID Count (error 0x%x)\n"), hRes));
        goto cleanup;
    }

    // Allocate array of GUIDs
    pGuids = (GUID *) malloc(uGuidCount * sizeof(GUID));
    if (!pGuids)
    {
        DBGMSG((TEXT("QueryAccelGuids: Failed to allocate array of GUIDs\n")));
        hRes = E_FAIL;
        goto cleanup;
    }

    // Get Acceleration Service Guids
    sExtension.Function              = AUXDEV_GET_ACCEL_GUIDS;
    sExtension.pPrivateInputData     = NULL;
    sExtension.PrivateInputDataSize  = 0;
    sExtension.pPrivateOutputData    = pGuids;
    sExtension.PrivateOutputDataSize = uGuidCount * sizeof(GUID);

    hRes = m_pAuxiliaryDevice->Execute(&sExecute);
    if (FAILED(hRes))
    {
        DBGMSG((TEXT("QueryAccelGuids: Failed to obtain GUIDs (error 0x%x)\n"), hRes));
        goto cleanup;
    }

cleanup:
    if (FAILED(hRes))
    {
        uGuidCount = 0;
        if (pGuids)
        {
            free(pGuids);
            pGuids = NULL;
        }
    }

    *puAccelGuidCount = uGuidCount;
    *ppAccelGuids     = pGuids;

    return hRes;
}



HRESULT CIntelAuxiliaryDevice::QueryAccelRTFormats(CONST GUID *pAccelGuid, D3DFORMAT **ppFormats, UINT *puCount)
{
    DXVA2_DecodeExecuteParams sExecute;
    DXVA2_DecodeExtensionData sExtension;

    HRESULT    hRes     = E_INVALIDARG;
    UINT       uCount   = 0;
    D3DFORMAT *pFormats = NULL;

    // Check parameters
    if (!pAccelGuid || !ppFormats || !puCount)
    {
        DBGMSG((TEXT("QueryAccelRTFormats: Invalid parameters\n")));
        goto cleanup;
    }

    // Check the decoder object
    if (!m_pAuxiliaryDevice)
    {
        DBGMSG((TEXT("QueryAccelRTFormats: Auxiliary Device is invalid\n")));
        goto cleanup;
    }

    // Get GUID count
    sExtension.Function              = AUXDEV_GET_ACCEL_RT_FORMAT_COUNT;
    sExtension.pPrivateInputData     = (PVOID)pAccelGuid;
    sExtension.PrivateInputDataSize  = sizeof(GUID);
    sExtension.pPrivateOutputData    = &uCount;
    sExtension.PrivateOutputDataSize = sizeof(int);

    sExecute.NumCompBuffers     = 0;
    sExecute.pCompressedBuffers = 0;
    sExecute.pExtensionData     = &sExtension;

    hRes = m_pAuxiliaryDevice->Execute(&sExecute);
    if (FAILED(hRes))
    {
        DBGMSG((TEXT("QueryAccelRTFormats: Failed to obtain RT format count (error 0x%x)\n"), hRes));
        goto cleanup;
    }

    // Allocate array of formats
    pFormats = (D3DFORMAT *) malloc(uCount * sizeof(D3DFORMAT));
    if (!pFormats)
    {
        DBGMSG((TEXT("QueryAccelRTFormats: Failed to allocate array of RT formats\n")));
        hRes = E_FAIL;
        goto cleanup;
    }

    // Get Guids
    sExtension.Function              = AUXDEV_GET_ACCEL_RT_FORMATS;
    sExtension.pPrivateInputData     = (PVOID)pAccelGuid;
    sExtension.PrivateInputDataSize  = sizeof(GUID);
    sExtension.pPrivateOutputData    = pFormats;
    sExtension.PrivateOutputDataSize = uCount * sizeof(D3DFORMAT);

    hRes = m_pAuxiliaryDevice->Execute(&sExecute);
    if (FAILED(hRes))
    {
        DBGMSG((TEXT("QueryAccelRTFormats: Failed to obtain array of RT formats (error 0x%x)\n"), hRes));
        goto cleanup;
    }

cleanup:
    if (FAILED(hRes))
    {
        uCount = 0;
        if (pFormats)
        {
            free(pFormats);
            pFormats = NULL;
        }
    }

    *puCount   = uCount;
    *ppFormats = pFormats;

    return hRes;
}


HRESULT CIntelAuxiliaryDevice::QueryAccelFormats(CONST GUID *pAccelGuid, D3DFORMAT **ppFormats, UINT *puCount)
{
    DXVA2_DecodeExecuteParams sExecute;
    DXVA2_DecodeExtensionData sExtension;

    HRESULT    hRes     = E_INVALIDARG;
    UINT       uCount   = 0;
    D3DFORMAT *pFormats = NULL;


    // Check parameters
    if (!pAccelGuid || !ppFormats || !puCount)
    {
        DBGMSG((TEXT("QueryAccelFormats: Invalid parameters\n")));
        goto cleanup;
    }

    // Check the decoder object
    if (!m_pAuxiliaryDevice)
    {
        DBGMSG((TEXT("QueryAccelFormats: Auxiliary Device is invalid\n")));
        goto cleanup;
    }

    // Get GUID count
    sExtension.Function              = AUXDEV_GET_ACCEL_FORMAT_COUNT;
    sExtension.pPrivateInputData     = (PVOID)pAccelGuid;
    sExtension.PrivateInputDataSize  = sizeof(GUID);
    sExtension.pPrivateOutputData    = &uCount;
    sExtension.PrivateOutputDataSize = sizeof(int);

    sExecute.NumCompBuffers     = 0;
    sExecute.pCompressedBuffers = 0;
    sExecute.pExtensionData     = &sExtension;

    hRes = m_pAuxiliaryDevice->Execute(&sExecute);
    if (FAILED(hRes))
    {
        DBGMSG((TEXT("QueryAccelFormats: Failed to obtain format count (error 0x%x)\n"), hRes));
        goto cleanup;
    }

    // Allocate array of formats
    pFormats = (D3DFORMAT *) malloc(uCount * sizeof(D3DFORMAT));
    if (!pFormats)
    {
        DBGMSG((TEXT("QueryAccelFormats: Failed to allocate array of formats\n")));
        hRes = E_FAIL;
        goto cleanup;
    }

    // Get Guids
    sExtension.Function              = AUXDEV_GET_ACCEL_FORMATS;
    sExtension.pPrivateInputData     = (PVOID)pAccelGuid;
    sExtension.PrivateInputDataSize  = sizeof(GUID);
    sExtension.pPrivateOutputData    = pFormats;
    sExtension.PrivateOutputDataSize = uCount * sizeof(D3DFORMAT);

    hRes = m_pAuxiliaryDevice->Execute(&sExecute);
    if (FAILED(hRes))
    {
        DBGMSG((TEXT("QueryAccelFormats: Failed to obtain array of formats (error 0x%x)\n"), hRes));
        goto cleanup;
    }

cleanup:
    if (FAILED(hRes))
    {
        uCount = 0;
        if (pFormats)
        {
            free(pFormats);
            pFormats = NULL;
        }
    }

    *puCount   = uCount;
    *ppFormats = pFormats;

    return hRes;
}


HRESULT CIntelAuxiliaryDevice::QueryAccelCaps(CONST GUID *pAccelGuid, void *pCaps, UINT *puCapSize)
{
    DXVA2_DecodeExecuteParams sExecute;
    DXVA2_DecodeExtensionData sExtension;

    HRESULT hRes = E_INVALIDARG;

    // Check parameters (pCaps may be NULL => puCapSize will receive the size of the device caps)
    if (!pAccelGuid || !puCapSize)
    {
        DBGMSG((TEXT("QueryAccelCaps: Invalid parameters\n")));
        goto cleanup;
    }

    // Check the decoder object
    if (!m_pAuxiliaryDevice)
    {
        DBGMSG((TEXT("QueryAccelCaps: Auxiliary Device is invalid\n")));
        goto cleanup;
    }

    // Query Caps
    sExtension.Function              = AUXDEV_QUERY_ACCEL_CAPS;
    sExtension.pPrivateInputData     = (PVOID)pAccelGuid;
    sExtension.PrivateInputDataSize  = sizeof(GUID);
    sExtension.pPrivateOutputData    = pCaps;
    sExtension.PrivateOutputDataSize = *puCapSize;

    sExecute.NumCompBuffers     = 0;
    sExecute.pCompressedBuffers = 0;
    sExecute.pExtensionData     = &sExtension;

    hRes = m_pAuxiliaryDevice->Execute(&sExecute);
    if (FAILED(hRes))
    {
        DBGMSG((TEXT("QueryAccelCaps: Failed to obtain caps (error 0x%x)\n"), hRes));
        goto cleanup;
    }

cleanup:
    *puCapSize = sExtension.PrivateOutputDataSize;
    return hRes;
}


HRESULT CIntelAuxiliaryDevice::CreateAccelService(CONST GUID *pAccelGuid, void *pCreateParams, UINT *puCreateParamSize)
{
    HRESULT hRes = E_INVALIDARG;
    DXVA2_DecodeExecuteParams sExecute;
    DXVA2_DecodeExtensionData sExtension;

    // Check parameters
    if (!pAccelGuid || !pCreateParams || !puCreateParamSize)
    {
        DBGMSG((TEXT("CreateAccelService: Invalid parameters\n")));
        goto cleanup;
    }

    // Check the decoder object
    if (!m_pAuxiliaryDevice)
    {
        DBGMSG((TEXT("CreateAccelService: Auxiliary Device is invalid\n")));
        goto cleanup;
    }

    // Query Caps
    sExtension.Function              = AUXDEV_CREATE_ACCEL_SERVICE;
    sExtension.pPrivateInputData     = (PVOID)pAccelGuid;
    sExtension.PrivateInputDataSize  = sizeof(GUID);
    sExtension.pPrivateOutputData    = pCreateParams;
    sExtension.PrivateOutputDataSize = *puCreateParamSize;

    sExecute.NumCompBuffers     = 0;
    sExecute.pCompressedBuffers = 0;
    sExecute.pExtensionData     = &sExtension;

    hRes = m_pAuxiliaryDevice->Execute(&sExecute);
    if (FAILED(hRes))
    {
        DBGMSG((TEXT("CreateAccelService: Failed to create the acceleration service (error 0x%x)\n"), hRes));
        goto cleanup;
    }

cleanup:
    *puCreateParamSize = sExtension.PrivateOutputDataSize;
    return hRes;
}
