#include "stdafx.h"

//#define DEBUG_DWM_PROTECTION

static const GUID DXVA2_SCD_WITH_DWM = 
{ 0x9e5a0dca, 0x2b9c, 0x414e, { 0x93, 0xfb, 0xd2, 0xcf, 0xae, 0x7e, 0x8e, 0x63 } };

static const GUID DXVA2_SCD_WITHOUT_DWM = 
{ 0x7460004, 0x7533, 0x4e1a, { 0xbd, 0xe3, 0xff, 0x20, 0x6b, 0xf5, 0xce, 0x47 } };

using namespace DispSvr;

static inline HRESULT Enable(IDirect3DDevice9 *pDevice, GUID guid, BOOL bEnableProtection)
{
	HRESULT hr;
	CComPtr<IDirectXVideoDecoderService> pVideoDecoderService;
	CComPtr<IDirectXVideoDecoder> pDecoder;
	CComPtr<IDirect3DSurface9> pRenderTarget;
	UINT			nRenderTargetFormats	= 0;
	DXVA2_VideoDesc	dxva2Desc				= {0};
	UINT			nConfig					= 0;
	D3DFORMAT*		pD3FormatsSupported		= NULL;
	DXVA2_ConfigPictureDecode *pConfigs		= NULL;

#ifdef DEBUG_DWM_PROTECTION
	guid = DXVA2_ModeMPEG2_IDCT;
#endif

	hr = CDynLibManager::GetInstance()->pfnDXVA2CreateVideoService(pDevice, __uuidof(IDirectXVideoDecoderService), (void**) &pVideoDecoderService);
	if (FAILED(hr))
		return hr;

	// Get support render targets
	hr = pVideoDecoderService->GetDecoderRenderTargets(guid, &nRenderTargetFormats, &pD3FormatsSupported);
	if (FAILED(hr))
		return hr;

	ASSERT(pRenderTarget == 0);
	ASSERT(nRenderTargetFormats > 0 && pD3FormatsSupported);
	for(UINT i = 0; i < nRenderTargetFormats; i++)
	{
		hr = pVideoDecoderService->CreateSurface(16, 16, 0,
			pD3FormatsSupported[i], D3DPOOL_DEFAULT, 0, DXVA2_VideoDecoderRenderTarget, &pRenderTarget, NULL);
		if (SUCCEEDED(hr))
		{
			dxva2Desc.SampleWidth = 16;
			dxva2Desc.SampleHeight = 16;
			dxva2Desc.Format = pD3FormatsSupported[i];
			break;
		}
	}

	CoTaskMemFree(pD3FormatsSupported);
	if (FAILED(hr))
		return hr;

	hr = pVideoDecoderService->GetDecoderConfigurations(guid, &dxva2Desc, 0, &nConfig, &pConfigs);
	if (FAILED(hr))
		return hr;

	hr = pVideoDecoderService->CreateVideoDecoder(guid, &dxva2Desc, pConfigs, &pRenderTarget.p, 1, &pDecoder);
	CoTaskMemFree(pConfigs);

	if (SUCCEEDED(hr))
	{
		DXVA2_DecodeExtensionData ExtData;
		DXVA2_DecodeExecuteParams Params = {0, NULL, &ExtData};
		BOOL bRestrictAccess = bEnableProtection;
		HRESULT hResult = E_FAIL;

		Params.pExtensionData->Function = 1;
		Params.pExtensionData->pPrivateInputData = &bRestrictAccess;
		Params.pExtensionData->PrivateInputDataSize = sizeof(bRestrictAccess);
		Params.pExtensionData->pPrivateOutputData = &hResult;
		Params.pExtensionData->PrivateOutputDataSize = sizeof(hResult);

		hr = pDecoder->BeginFrame(pRenderTarget, NULL);
		if (SUCCEEDED(hr))
		{
			hr = pDecoder->Execute(&Params);
			hr = pDecoder->EndFrame(NULL);
		}
	}
	return hr;
}

HRESULT EnableScreenCaptureDefense(IDirect3DDevice9 *pDevice, BOOL bEnableScreenCaptureDefense)
{
	BOOL bCompositionEnabled = FALSE;
	HRESULT hr;

	if (!pDevice)
		return E_INVALIDARG;

	if (CDynLibManager::GetInstance()->pfnDwmIsCompositionEnabled == NULL
		|| CDynLibManager::GetInstance()->pfnDXVA2CreateVideoService == NULL)
		return E_FAIL;

	hr = CDynLibManager::GetInstance()->pfnDwmIsCompositionEnabled(&bCompositionEnabled);
	if (FAILED(hr))
		bCompositionEnabled = FALSE;

	hr = Enable(pDevice, bCompositionEnabled ? DXVA2_SCD_WITH_DWM : DXVA2_SCD_WITHOUT_DWM, bEnableScreenCaptureDefense);
	return hr;
}