#include "StdAfx.h"
#include "DispSvr.h"
#include <dxva2api.h>
#include "IntelDxva2Device.h"
#include "DynLibManager.h"


using namespace DispSvr;

CIntelAuxiliaryDevice::CIntelAuxiliaryDevice(IDirect3DDevice9 *pDevice) : m_pDevice(pDevice)
{
	ASSERT(m_pDevice);
	m_pDevice->AddRef();
	m_VideoDesc.SampleWidth                         = 64;
	m_VideoDesc.SampleHeight                        = 64;
	m_VideoDesc.SampleFormat.VideoChromaSubsampling = DXVA2_VideoChromaSubsampling_MPEG2;
	m_VideoDesc.SampleFormat.NominalRange           = DXVA2_NominalRange_16_235;
	m_VideoDesc.SampleFormat.VideoTransferMatrix    = DXVA2_VideoTransferMatrix_BT709;
	m_VideoDesc.SampleFormat.VideoLighting          = DXVA2_VideoLighting_dim;
	m_VideoDesc.SampleFormat.VideoPrimaries         = DXVA2_VideoPrimaries_BT709;
	m_VideoDesc.SampleFormat.VideoTransferFunction  = DXVA2_VideoTransFunc_709;
	m_VideoDesc.SampleFormat.SampleFormat           = DXVA2_SampleProgressiveFrame;
	m_VideoDesc.Format                              = D3DFMT_YUY2;
	m_VideoDesc.InputSampleFreq.Numerator           = m_VideoDesc.OutputFrameFreq.Numerator   = 30000;
	m_VideoDesc.InputSampleFreq.Denominator         = m_VideoDesc.OutputFrameFreq.Denominator = 1001;
	m_VideoDesc.UABProtectionLevel                  = 0;
	m_VideoDesc.Reserved                            = 0;
	m_pRenderTarget = 0;
	m_pAuxiliaryDevice = 0;
}

CIntelAuxiliaryDevice::~CIntelAuxiliaryDevice()
{
	int ref = 0;

	SAFE_RELEASE(m_pDevice);

	if (m_pAuxiliaryDevice)
	{
		ref = m_pAuxiliaryDevice->Release();		ASSERT(ref == 0);
		m_pAuxiliaryDevice = 0;
	}

	if (m_pRenderTarget)
	{
		ref = m_pRenderTarget->Release();		ASSERT(ref == 0);
		m_pRenderTarget = 0;
	}
}

HRESULT CIntelAuxiliaryDevice::CreateAuxiliaryDevice()
{
	CHECK_POINTER(CDynLibManager::GetInstance()->pfnDXVA2CreateVideoService);

	D3DFORMAT *pFormats = NULL;
	DXVA2_ConfigPictureDecode *pPictureDecode = NULL;
	DXVA2_ConfigPictureDecode PictureDecode;
	CComPtr<IDirectXVideoDecoderService> pVideoDecoderService;
	GUID *pGUIDs = NULL;
	UINT i, uGuids, uiFormats, uiConfigurations;
	HRESULT hr;

	hr = CDynLibManager::GetInstance()->pfnDXVA2CreateVideoService(m_pDevice, __uuidof(IDirectXVideoDecoderService), (VOID **)&pVideoDecoderService);
	if (FAILED(hr))
		return hr;

	hr = pVideoDecoderService->GetDecoderDeviceGuids(&uGuids, &pGUIDs);
	if (FAILED(hr))
		return hr;

	for (i = 0; i < uGuids; i++)
	{
		if (IsEqualGUID(pGUIDs[i], DXVA2_Intel_Auxiliary_Device))
			break;
	}

	CoTaskMemFree(pGUIDs);

	if (i >= uGuids)
		return E_FAIL;

	hr = pVideoDecoderService->GetDecoderRenderTargets(DXVA2_Intel_Auxiliary_Device, &uiFormats, &pFormats);
	if (SUCCEEDED(hr))
	{
		m_VideoDesc.Format = pFormats[0];
		CoTaskMemFree(pFormats);
	}
	else
	{
		return hr;
	}

	hr = pVideoDecoderService->GetDecoderConfigurations(DXVA2_Intel_Auxiliary_Device,
		&m_VideoDesc,
		NULL,
		&uiConfigurations,
		&pPictureDecode);
	if (FAILED(hr))
		return hr;

	hr = pVideoDecoderService->CreateSurface(64, 64, 0, (D3DFORMAT)MAKEFOURCC('I', 'M', 'C', '3'),
		D3DPOOL_DEFAULT, 0, DXVA2_VideoDecoderRenderTarget, &m_pRenderTarget, NULL);
	if (FAILED(hr))
		return hr;

	PictureDecode = *pPictureDecode;
	CoTaskMemFree(pPictureDecode);
	if (uiConfigurations != 1)
		return E_FAIL;

	hr = pVideoDecoderService->CreateVideoDecoder(DXVA2_Intel_Auxiliary_Device,
		&m_VideoDesc,
		&PictureDecode,
		&m_pRenderTarget,
		1,
		&m_pAuxiliaryDevice);

	return hr;
}

HRESULT CIntelAuxiliaryDevice::CreateRegistrationDevice(CIntelRegistrationDevice **ppDevice)
{
	HRESULT hr;
	CComPtr<IDirectXVideoProcessorService> pVPservice;

	ASSERT(ppDevice);

	hr = CDynLibManager::GetInstance()->pfnDXVA2CreateVideoService(m_pDevice, __uuidof(IDirectXVideoProcessorService), (VOID**)&pVPservice);
	if (FAILED(hr))
		return hr;

	*ppDevice = new CIntelRegistrationDevice();
	hr = pVPservice->CreateVideoProcessor(DXVA2_Registration_Device, &m_VideoDesc, D3DFMT_YUY2, 0, &(*ppDevice)->m_pRegistrationDevice);
	if (FAILED(hr))
	{
		delete *ppDevice;
		*ppDevice = 0;
	}

	return hr;
}

HRESULT CIntelAuxiliaryDevice::HasAccelGuids(const GUID &guid, BOOL *pbHasIt)
{
	HRESULT hr = E_FAIL;
	UINT uGuidCount = 0;
	GUID *pGuids = NULL;

	*pbHasIt = FALSE;
	// Check the decoder object
	if (!m_pAuxiliaryDevice)
		return hr;

	hr = Execute(AUXDEV_GET_ACCEL_GUID_COUNT, NULL, 0, &uGuidCount, sizeof(int));
	if (SUCCEEDED(hr))
	{
		// Allocate array of GUIDs
		pGuids = (GUID *) malloc(uGuidCount * sizeof(GUID));
		if (!pGuids)
			return E_OUTOFMEMORY;

		// Get Acceleration Service Guids
		hr = Execute(AUXDEV_GET_ACCEL_GUIDS, NULL, 0, pGuids, uGuidCount * sizeof(GUID));

		if (SUCCEEDED(hr))
		{
			for (UINT i = 0; i < uGuidCount; i++)
			{
				if (pGuids[i] == guid)
				{
					*pbHasIt = TRUE;
					break;
				}
			}
		}
		free(pGuids);
	}

	return hr;
}

HRESULT CIntelAuxiliaryDevice::QueryAccelGuids(GUID **ppAccelGuids, UINT *puAccelGuidCount)
{
	HRESULT hr = E_FAIL;
	UINT uGuidCount = 0;
	GUID *pGuids = NULL;

	ASSERT(ppAccelGuids && puAccelGuidCount);

	// Check the decoder object
	if (!m_pAuxiliaryDevice)
		return hr;

	hr = Execute(AUXDEV_GET_ACCEL_GUID_COUNT, NULL, 0, &uGuidCount, sizeof(int));
	if (SUCCEEDED(hr))
	{
		// Allocate array of GUIDs
		pGuids = (GUID *) malloc(uGuidCount * sizeof(GUID));
		if (!pGuids)
			return E_OUTOFMEMORY;

		// Get Acceleration Service Guids
		hr = Execute(AUXDEV_GET_ACCEL_GUIDS, NULL, 0, pGuids, uGuidCount * sizeof(GUID));
	}

	if (FAILED(hr))
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
	return hr;
}

HRESULT CIntelAuxiliaryDevice::QueryAccelRTFormats(CONST GUID *pAccelGuid, D3DFORMAT **ppFormats, UINT *puCount)
{
	HRESULT    hr     = E_INVALIDARG;
	UINT       uCount   = 0;
	D3DFORMAT *pFormats = NULL;

	ASSERT(pAccelGuid && ppFormats && puCount);
	ASSERT(m_pAuxiliaryDevice);

	hr = Execute(AUXDEV_GET_ACCEL_RT_FORMAT_COUNT, (PVOID)pAccelGuid, sizeof(GUID), &uCount, sizeof(int));
	if (SUCCEEDED(hr))
	{
		// Allocate array of formats
		pFormats = (D3DFORMAT *) malloc(uCount * sizeof(D3DFORMAT));
		if (!pFormats)
			return E_OUTOFMEMORY;

		// Get Guids
		hr = Execute(AUXDEV_GET_ACCEL_RT_FORMATS, (PVOID)pAccelGuid, sizeof(GUID), pFormats, uCount * sizeof(D3DFORMAT));
	}

	if (FAILED(hr))
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

	return hr;
}

HRESULT CIntelAuxiliaryDevice::QueryAccelFormats(CONST GUID *pAccelGuid, D3DFORMAT **ppFormats, UINT *puCount)
{
	HRESULT    hr     = E_INVALIDARG;
	UINT       uCount   = 0;
	D3DFORMAT *pFormats = NULL;

	ASSERT(pAccelGuid && ppFormats && puCount);
	ASSERT(m_pAuxiliaryDevice);

	// Get GUID count
	hr = Execute(AUXDEV_GET_ACCEL_FORMAT_COUNT, (PVOID)pAccelGuid, sizeof(GUID), &uCount, sizeof(int));
	if (SUCCEEDED(hr))
	{
		// Allocate array of formats
		pFormats = (D3DFORMAT *) malloc(uCount * sizeof(D3DFORMAT));
		if (!pFormats)
			return E_OUTOFMEMORY;

		// Get Guids
		hr = Execute(AUXDEV_GET_ACCEL_FORMATS, (PVOID)pAccelGuid, sizeof(GUID), pFormats, uCount * sizeof(D3DFORMAT));
	}

	if (FAILED(hr))
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

	return hr;
}


HRESULT CIntelAuxiliaryDevice::QueryAccelCaps(CONST GUID *pAccelGuid, void *pCaps, UINT *puCapSize)
{
	HRESULT hr;

	ASSERT(pAccelGuid && puCapSize);
	ASSERT(m_pAuxiliaryDevice);

	hr = Execute(AUXDEV_QUERY_ACCEL_CAPS, (PVOID)pAccelGuid, sizeof(GUID), pCaps, *puCapSize);
	return hr;
}

HRESULT CIntelAuxiliaryDevice::Execute(UINT Function, PVOID pInput, UINT uSizeInput, PVOID pOutput, UINT uSizeOutput)
{
	DXVA2_DecodeExecuteParams sExecute;
	DXVA2_DecodeExtensionData sExtension;
	HRESULT hr;
	
	sExtension.Function              = Function;
	sExtension.pPrivateInputData     = pInput;
	sExtension.PrivateInputDataSize  = uSizeInput;
	sExtension.pPrivateOutputData    = pOutput;
	sExtension.PrivateOutputDataSize = uSizeOutput;

	sExecute.NumCompBuffers     = 0;
	sExecute.pCompressedBuffers = 0;
	sExecute.pExtensionData     = &sExtension;

	hr = m_pAuxiliaryDevice->Execute(&sExecute);

	return hr;
}

/////////////////////////////////////////////////////////////////////////////////////

CIntelFastCompositingService::CIntelFastCompositingService(IDirect3DDevice9 *pD3D9Device)
: CIntelAuxiliaryDevice(pD3D9Device)
{
	m_hRegistration = INVALID_HANDLE_VALUE;
	m_bCreated = false;
	m_iMaxDstWidth = 1920;
	m_iMaxDstHeight = 1080;
	m_iNumBackwardSamples = 0;
	m_iNumForwardSamples = 0;
	m_dwMaxFlipRate = 30;
	m_bCanSupportExtendedGamut = false;
}

CIntelFastCompositingService::~CIntelFastCompositingService()
{
	if (m_bCreated)
	{
		HRESULT hr = Execute(AUXDEV_DESTROY_ACCEL_SERVICE, (PVOID)&DXVA2_FastCompositing, sizeof(GUID), NULL, 0);
	}
}

HRESULT CIntelFastCompositingService::Create(IDirect3DDevice9 *pD3D9Device, CIntelFastCompositingService **ppService)
{
	CIntelFastCompositingService *pService = new CIntelFastCompositingService(pD3D9Device); ASSERT(pService);
	HRESULT hr = pService->CreateAuxiliaryDevice();
	if (SUCCEEDED(hr))
		hr = pService->Init();

	if (FAILED(hr))
	{
		delete pService;
		pService = 0;
	}

	*ppService = pService;
	return hr;
}

HRESULT CIntelFastCompositingService::Init()
{
	HRESULT hr;
	DXVA2_VideoDesc desc = {0};
	FASTCOMP_CREATEDEVICE sCreateStruct = {
		&desc,
		D3DFMT_A8R8G8B8,	// Render Target Format, usually D3DFMT_A8R8G8B8
		1			// substream should be set to 1 unless otherwise specified.
	};
	UINT i = 0;
	FASTCOMP_QUERYCAPS sQuery;
	UINT uQuerySize = sizeof(sQuery);
	BOOL bHasGuid = FALSE;

	// check if the driver reports DXVA2_FastCompositing
	hr = HasAccelGuids(DXVA2_FastCompositing, &bHasGuid);
	if (FAILED(hr))
		return hr;

	if (bHasGuid != TRUE)
		return E_FAIL;

	sQuery.Type = FASTCOMP_QUERY_CAPS;
	hr = QueryAccelCaps(&DXVA2_FastCompositing, &sQuery, &uQuerySize);
	if (SUCCEEDED(hr))
	{
		m_iMaxDstWidth = sQuery.sCaps.sRenderTargetCaps.iMaxWidth;
		m_iMaxDstHeight = sQuery.sCaps.sRenderTargetCaps.iMaxHeight;
		m_iNumBackwardSamples = sQuery.sCaps.sPrimaryVideoCaps.iNumBackwardSamples;
		m_iNumForwardSamples = sQuery.sCaps.sPrimaryVideoCaps.iNumForwardSamples;
		m_bCanSupportExtendedGamut = sQuery.sCaps.sPrimaryVideoCaps.bExtendedGamut;
	}

	DWORD dwFrameRate = 0;
	hr = QueryFrameRate(FASTCOM_MAX_INPUT_LAYERS, FASTCOM_MAX_SRC_WIDTH, 
		FASTCOM_MAX_SRC_HEIGHT, m_iMaxDstWidth, m_iMaxDstHeight, dwFrameRate);
	if (SUCCEEDED(hr))
		m_dwMaxFlipRate = dwFrameRate;

//	FASTCOMP_SAMPLE_FORMATS *pSampleFormats = NULL;
//	hr = QueryFormats(&pSampleFormats);
//	if (SUCCEEDED(hr))
//		free(pSampleFormats);

	// Initially, we set video format to yuy2. Driver checks the input surface format
	// at per execution basis so it is not necessary to recreate service when input
	// video format changes.
	// If using m_iMaxDstWidth/m_iMaxDstHeight (1920/1200), E_INVALIDARG will be returned.
	// E_INVALIDARG might be return by MS instead of driver. (Driver usually return E_FAIL)
	// So we use fixed 1920x1080 to create Fast Composition service.
	desc.Format = D3DFMT_YUY2;
	desc.SampleWidth = 1920;
	desc.SampleHeight = 1080;

	// create FastCompositing service
	hr = Execute(AUXDEV_CREATE_ACCEL_SERVICE, (PVOID)&DXVA2_FastCompositing, sizeof(GUID), &sCreateStruct, sizeof(sCreateStruct));
	if (FAILED(hr))
		return hr;

	m_bCreated = true;

	// Obtain registration handle
	uQuerySize = sizeof(sQuery);
	sQuery.Type = FASTCOMP_QUERY_REGISTRATION_HANDLE;

	hr = QueryAccelCaps(&DXVA2_FastCompositing, &sQuery, &uQuerySize);
	if (SUCCEEDED(hr))
		m_hRegistration = sQuery.hRegistration;

	return hr;
}

HRESULT CIntelFastCompositingService::FastCompositingBlt(const FASTCOMP_BLT_PARAMS &blt)
{
	return Execute(FASTCOMP_BLT, (PVOID)&blt, sizeof(blt), NULL, 0);
}

void CIntelFastCompositingService::ApplyTargetRectRestriction(RECT &rcDst)
{
	int iWidth = rcDst.right - rcDst.left;
	int iHeight = rcDst.bottom - rcDst.top;

	if (iWidth > m_iMaxDstWidth)
	{
		rcDst.left += (iWidth - m_iMaxDstWidth) >> 1;
		rcDst.right = rcDst.left + m_iMaxDstWidth;
	}

	if (iHeight > m_iMaxDstHeight)
	{
		rcDst.top += (iHeight - m_iMaxDstHeight) >> 1;
		rcDst.bottom = rcDst.top + m_iMaxDstHeight;
	}
}

HRESULT CIntelFastCompositingService::QueryFormats(FASTCOMP_SAMPLE_FORMATS **ppFormats)
{
	HRESULT hr;
	UINT                uTotal;
	D3DFORMAT		   *pFormatArray;
	FASTCOMP_QUERYCAPS  sQuery;
	UINT                uQuerySize;
	FASTCOMP_SAMPLE_FORMATS *pFormats = NULL;

	ASSERT(ppFormats);

	// Query format counts
	sQuery.Type = FASTCOMP_QUERY_FORMAT_COUNT;
	uQuerySize  = sizeof(sQuery);
	hr = QueryAccelCaps(&DXVA2_FastCompositing, &sQuery, &uQuerySize);
	if (FAILED(hr))
		return hr;

	// Allocate a structure and extra area to hold all format arrays
	uTotal  = sQuery.sFmtCount.iPrimaryFormats;
	uTotal += sQuery.sFmtCount.iSecondaryFormats;
	uTotal += sQuery.sFmtCount.iSubstreamFormats;
	uTotal += sQuery.sFmtCount.iGraphicsFormats;
	uTotal += sQuery.sFmtCount.iRenderTargetFormats;
	uTotal += sQuery.sFmtCount.iBackgroundFormats;
	pFormats = (FASTCOMP_SAMPLE_FORMATS *) malloc(sizeof(FASTCOMP_SAMPLE_FORMATS) + uTotal * sizeof(D3DFORMAT) + 8);
	if (!pFormats)
		return E_OUTOFMEMORY;

	// Initialize the output structure
	pFormatArray = (D3DFORMAT *) (pFormats + 1);

	pFormats->iPrimaryVideoFormatCount   = sQuery.sFmtCount.iPrimaryFormats;
	pFormats->pPrimaryVideoFormats       = pFormatArray;
	pFormatArray += sQuery.sFmtCount.iSecondaryFormats;

	pFormats->iSecondaryVideoFormatCount = sQuery.sFmtCount.iSecondaryFormats;
	pFormats->pSecondaryVideoFormats     = pFormatArray;
	pFormatArray += sQuery.sFmtCount.iSecondaryFormats;

	pFormats->iSubstreamFormatCount      = sQuery.sFmtCount.iSubstreamFormats;
	pFormats->pSubstreamFormats          = pFormatArray;
	pFormatArray += sQuery.sFmtCount.iSubstreamFormats;

	pFormats->iGraphicsFormatCount       = sQuery.sFmtCount.iGraphicsFormats;
	pFormats->pGraphicsFormats           = pFormatArray;
	pFormatArray += sQuery.sFmtCount.iGraphicsFormats;

	pFormats->iRenderTargetFormatCount   = sQuery.sFmtCount.iRenderTargetFormats;
	pFormats->pRenderTargetFormats       = pFormatArray;
	pFormatArray += sQuery.sFmtCount.iRenderTargetFormats;

	pFormats->iBackgroundFormatCount     = sQuery.sFmtCount.iBackgroundFormats;
	pFormats->pBackgroundFormats         = pFormatArray;

	// Query remaining formats
	sQuery.Type = FASTCOMP_QUERY_FORMATS;
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

	hr = QueryAccelCaps(&DXVA2_FastCompositing, &sQuery, &uQuerySize);
	if (FAILED(hr))
	{
		if (pFormats) free(pFormats);
		pFormats = NULL;
	}

	*ppFormats = pFormats;
	return hr;
}

HRESULT CIntelFastCompositingService::QueryFrameRate(int iMaxLayers, int iMaxSrcWidth, int iMaxSrcHeight, int iMaxDstWidth, int iMaxDstHeight, DWORD &dwFrameRate)
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

	HRESULT hr = QueryAccelCaps(&DXVA2_FastCompositing, &sQuery, &uQuerySize);
	if (FAILED(hr))
	{
		DbgMsg("CIntelFastCompositingService::QueryFrameRate: Failed to query frame rate\n");
	}
	dwFrameRate = sQuery.sFrameRate.iFrameRate;

	return hr;
}

/////////////////////////////////////////////////////////////////////////////////////

CIntelSCDService::CIntelSCDService(IDirect3DDevice9 *pDevice)
: CIntelAuxiliaryDevice (pDevice)
{
	m_hRegistration = INVALID_HANDLE_VALUE;
	m_bCreated = false;
	m_bCompositionEnabled = FALSE;
	m_bPavpLiteMode = true;	// default to treat as lite mode.
	m_bNeedEnableComposition = false;
}

CIntelSCDService::~CIntelSCDService()
{
	if (m_bCreated)
	{
		Execute(AUXDEV_DESTROY_ACCEL_SERVICE, (PVOID) (m_bPavpLiteMode ? &DXVA2_Intel_SCD : &DXVA2_Intel_Pavp), sizeof(GUID), NULL, 0);
	}

	if (m_bNeedEnableComposition)
	{
		CDynLibManager::GetInstance()->pfnDwmEnableComposition(TRUE);
	}
}

HRESULT CIntelSCDService::Create(IDirect3DDevice9 *pD3D9Device, CIntelSCDService **ppService)
{
	CIntelSCDService *pService = new CIntelSCDService(pD3D9Device); ASSERT(pService);
	HRESULT hr = pService->CreateAuxiliaryDevice();
	if (FAILED(hr))
		return hr;

	hr = pService->Init();
	if (FAILED(hr))
	{
		delete pService;
		pService = 0;
	}

	*ppService = pService;
	return hr;
}

HRESULT CIntelSCDService::Init()
{
	HRESULT hr;
	BOOL bHasGuid = FALSE;
	PAVP_QUERY_CAPS sQuery = {0};
	UINT uPavpCapSize = sizeof(PAVP_QUERY_CAPS);

	m_bPavpLiteMode = true;
	hr = QueryAccelCaps(&DXVA2_Intel_Pavp, &sQuery, &uPavpCapSize);
	if (SUCCEEDED(hr))
		m_bPavpLiteMode = (sQuery.eCurrentMemoryMode == PAVP_MEMORY_PROTECTION_LITE) != 0;

	hr = CDynLibManager::GetInstance()->pfnDwmIsCompositionEnabled(&m_bCompositionEnabled);
	if (FAILED(hr))
		return hr;

	// if BIOS is set to PAVP heavy, we should not use SCD because PAVP automatically enables SCD.
	if (m_bPavpLiteMode)
	{
		// check if the driver reports DXVA2_Intel_SCD
		hr = HasAccelGuids(DXVA2_Intel_SCD, &bHasGuid);
		if (FAILED(hr))
			return hr;

		if (bHasGuid != TRUE)
			return E_FAIL;

		// create Screen Capture Defense service
		SCD_CREATE_DEVICE sSCDCreateStruct;
		hr = Execute(AUXDEV_CREATE_ACCEL_SERVICE, (PVOID)&DXVA2_Intel_SCD, sizeof(GUID), &sSCDCreateStruct, sizeof(sSCDCreateStruct));
	}
	else
	{
		// 	Under PAVP heavy mode, we need to disable DWM for protected surfaces to blt onto proprietary overlay.
		if (m_bCompositionEnabled)
		{
			hr = CDynLibManager::GetInstance()->pfnDwmEnableComposition(FALSE);
			if (FAILED(hr))
				return hr;
			m_bNeedEnableComposition = true;
		}

		PAVP_CREATE_DEVICE sPAVPCreateStruct;
		sPAVPCreateStruct.eDesiredKeyExchangeMode = PAVP_KEY_EXCHANGE_CANTIGA;
		hr = Execute(AUXDEV_CREATE_ACCEL_SERVICE, (PVOID)&DXVA2_Intel_Pavp, sizeof(GUID), &sPAVPCreateStruct, sizeof(sPAVPCreateStruct));
	}

	if (FAILED(hr))
		return hr;

	m_bCreated = true;

	if (m_bPavpLiteMode)
	{
		HRESULT hRes = E_FAIL;
		SCD_MODE sScdInitMode;
		sScdInitMode.ScdMode = m_bCompositionEnabled ? SCD_DWM_BASED : SCD_OVL_BASED;

		// begin screen capture defense.
		hr = Execute(SCD_BEGIN, (PVOID)&sScdInitMode, sizeof(sScdInitMode), &hRes, sizeof(hRes));
		if (FAILED(hr))
			return hr;

		// authentication extension to SCD DDI, older driver may not have this feature.
		SCD_INIT_AUTHENTICATION_PARAMS sInitAuth;
		sInitAuth.dwAppId = COREL_APPLICATION_ID;
		if (SUCCEEDED(Execute(SCD_INIT_AUTHENTICATION, (PVOID)&sInitAuth, sizeof(sInitAuth), (PVOID)&sInitAuth, sizeof(sInitAuth))))
		{
			m_hRegistration = sInitAuth.hRegistration;
			DbgMsg("CIntelSCDService::Init: SCD_INIT_AUTHENTICATION_PARAMS is used, handle = %x", m_hRegistration);
		}
	}
	return hr;
}

HRESULT CIntelSCDService::SetWindowPosition(const RECT &rWindowPosition, const RECT &rVisibleContent, HDC hdcMonitorId)
{
	SCD_SET_WINDOW_POSITION_PARAMS sScdSetWindowPositionParams = {rWindowPosition, rVisibleContent, hdcMonitorId};
	HRESULT hRes = E_FAIL;
	HRESULT hr = Execute(
		m_bPavpLiteMode ? SCD_SET_WINDOW_POSITION : PAVP_SET_WINDOW_POSITION,
		&sScdSetWindowPositionParams, sizeof(sScdSetWindowPositionParams), &hRes, sizeof(hRes));
	return SUCCEEDED(hr) ? hRes : hr;
}

HRESULT CIntelSCDService::UnscrambleSurface(void *pSurface, DWORD dwSizeScrambled, DWORD dwScrambleSeed)
{
	ASSERT(IsSCDExtension() && m_bPavpLiteMode == true);
	SCD_UNSCRAMBLE_SURFACE_PARAMS param = { pSurface, dwSizeScrambled, dwScrambleSeed };
	HRESULT hr = Execute(SCD_UNSCRAMBLE_SURFACE, &param, sizeof(param), NULL, 0);
	return hr;
}

/////////////////////////////////////////////////////////////////////////////////////

CIntelRegistrationDevice::CIntelRegistrationDevice()
{
	m_pRegistrationDevice = 0;

	memset(&m_BltParams, 0, sizeof(m_BltParams));
	memset(m_Samples   , 0, sizeof(m_Samples));
	SetRect(&m_BltParams.TargetRect, 0, 0, 16, 16);
	m_BltParams.StreamingFlags    = 0;
	m_BltParams.BackgroundColor.Alpha = 65535;
	m_BltParams.DestFormat.VideoChromaSubsampling = DXVA2_VideoChromaSubsampling_MPEG2;
	m_BltParams.DestFormat.NominalRange           = DXVA2_NominalRange_Normal;
	m_BltParams.DestFormat.VideoTransferMatrix    = DXVA2_VideoTransferMatrix_BT709;
	m_BltParams.DestFormat.VideoLighting          = DXVA2_VideoLighting_dim;
	m_BltParams.DestFormat.VideoPrimaries         = DXVA2_VideoPrimaries_BT709;
	m_BltParams.DestFormat.VideoTransferFunction  = DXVA2_VideoTransFunc_709;
	m_BltParams.DestFormat.SampleFormat           = DXVA2_SampleFieldInterleavedEvenFirst;
	m_BltParams.Alpha.Value       = 1;
	m_BltParams.Alpha.Fraction    = 0;
	m_BltParams.DestData          = 0;

	for (int i = 0; i < MAX_REGISTRATION_SAMPLES; i++)
	{
		m_Samples[1].SrcRect = m_Samples[i].DstRect = m_BltParams.TargetRect;
		m_Samples[i].SampleFormat = m_BltParams.DestFormat;
	}
}

CIntelRegistrationDevice::~CIntelRegistrationDevice()
{
	if (m_pRegistrationDevice)
	{
		m_pRegistrationDevice->Release();
		m_pRegistrationDevice = 0;
	}
}

HRESULT CIntelRegistrationDevice::RegisterSurfaces(const DXVA2_SURFACE_REGISTRATION *Reg)
{
	CHECK_POINTER(Reg);
	CHECK_POINTER(m_pRegistrationDevice);
	CHECK_POINTER(Reg->pRenderTargetRequest->pD3DSurface);
	if (Reg->RegHandle == INVALID_HANDLE_VALUE)
		return E_INVALIDARG;

	m_BltParams.TargetFrame = (REFERENCE_TIME) Reg;

	for (UINT i = 0; i < Reg->nSamples; i++)
	{
		m_Samples[i].Start = m_BltParams.TargetFrame;
		m_Samples[i].End = m_BltParams.TargetFrame + 1;
		m_Samples[i].SrcSurface = Reg->pSamplesRequest[i].pD3DSurface;
	}

	// Register IDirect3DSurface9 surface pointers. Driver communicates with d3d runtime by surface
	// handles, and our app talks with d3d runtime by IDirect3DSurface9 pointers. Since fastcomp
	// will bypass d3d runtime, driver uses this way to map IDirect3DSurface9 to surface handles.
	return m_pRegistrationDevice->VideoProcessBlt(Reg->pRenderTargetRequest->pD3DSurface, &m_BltParams, m_Samples, Reg->nSamples, NULL);
}

/////////////////////////////////////////////////////////////////////////////////////

CIntelDxva2DriverExtAdapter::CIntelDxva2DriverExtAdapter(IDirect3DDevice9 *pDevice)
: CIntelAuxiliaryDevice(pDevice), m_uMainVideoWidth(0), m_uMainVideoHeight(0)
{
}

CIntelDxva2DriverExtAdapter::~CIntelDxva2DriverExtAdapter()
{
}

HRESULT CIntelDxva2DriverExtAdapter::GetAdapter(IDirect3DDevice9 *pDevice9, IDriverExtensionAdapter **ppDeviceCapAdapter)
{
	if (pDevice9 == 0 || ppDeviceCapAdapter == 0)
		return E_POINTER;

	CIntelDxva2DriverExtAdapter *p = new CIntelDxva2DriverExtAdapter(pDevice9);
	HRESULT hr = p->CreateAuxiliaryDevice();
	if (FAILED(hr))
	{
		delete p;
		p = 0;
	}

	*ppDeviceCapAdapter = p;
	return hr;
}

HRESULT CIntelDxva2DriverExtAdapter::QueryPresenterCaps(DWORD VideoDecodeCaps, PresenterCaps *pCaps)
{
	HRESULT hr = E_FAIL;
	DWORD dwFrameRate = 0;
	int iMaxDstWidth = 0;
	int iMaxDstHeight = 0;
	FASTCOMP_QUERYCAPS sQuery;
	UINT uQuerySize;

	sQuery.Type = FASTCOMP_QUERY_CAPS;
	uQuerySize = sizeof(sQuery);

	hr = QueryAccelCaps(&DXVA2_FastCompositing, &sQuery, &uQuerySize);
	if (SUCCEEDED(hr))
	{
		iMaxDstWidth = sQuery.sCaps.sRenderTargetCaps.iMaxWidth;
		iMaxDstHeight = sQuery.sCaps.sRenderTargetCaps.iMaxHeight;
	}

	sQuery.Type = FASTCOMP_QUERY_FRAME_RATE;
	sQuery.sFrameRate.iMaxSrcWidth  = FASTCOM_MAX_SRC_WIDTH;
	sQuery.sFrameRate.iMaxSrcHeight = FASTCOM_MAX_SRC_HEIGHT;
	sQuery.sFrameRate.iMaxDstWidth  = iMaxDstWidth;
	sQuery.sFrameRate.iMaxDstHeight = iMaxDstHeight; 
	sQuery.sFrameRate.iMaxLayers    = FASTCOM_MAX_INPUT_LAYERS;    
	sQuery.sFrameRate.iFrameRate    = -1;  // Output

	hr = QueryAccelCaps(&DXVA2_FastCompositing, &sQuery, &uQuerySize);
	if (SUCCEEDED(hr))
		pCaps->dwFPS = sQuery.sFrameRate.iFrameRate;
	else
		pCaps->dwFPS = 30;

	if (VideoDecodeCaps > 0)
	{
		UINT uSrcWidth = 0;
		UINT uSrcHeight = 0;

		if(VideoDecodeCaps & VIDEO_CAP_FORMAT_1080)
		{
			uSrcHeight = 1088;
			uSrcWidth = 1920;	//1440 or 1920
		}
		else if(VideoDecodeCaps & VIDEO_CAP_FORMAT_480)
		{
			uSrcHeight = 480;
			uSrcWidth = 720;
		}
		else if(VideoDecodeCaps & VIDEO_CAP_FORMAT_576)
		{
			uSrcHeight = 576;
			uSrcWidth = 720;
		}
		else if(VideoDecodeCaps & VIDEO_CAP_FORMAT_720)
		{
			uSrcHeight = 720;
			uSrcWidth = 1280;
		}

		ASSERT(uSrcHeight * uSrcWidth > 0);

		if (VideoDecodeCaps & VIDEO_CAP_STREAM_SUB)
		{
			PQUERY_DUAL_DXVA_DECODE sQuery = {0};
			UINT uQuerySize = sizeof(sQuery);

			ASSERT(m_uMainVideoWidth * m_uMainVideoHeight > 0);
			sQuery.iPrivateQueryID = PQID_DUAL_DXVA_DECODE;
			sQuery.Width1 = m_uMainVideoWidth;
			sQuery.Height1 = m_uMainVideoHeight;
			sQuery.Width2 = uSrcWidth;
			sQuery.Height2 = uSrcHeight;
			hr = QueryAccelCaps(&DXVA2_Intel_PrivateQuery_Device, &sQuery, &uQuerySize);
			if (SUCCEEDED(hr))
				pCaps->bHwDecode = sQuery.bSupported;
			else
				pCaps->bHwDecode = FALSE;
		}
		else 		// cache main video size for later.
		{
			m_uMainVideoWidth = uSrcWidth;
			m_uMainVideoHeight = uSrcHeight;
		}
	}

	return hr;
}
