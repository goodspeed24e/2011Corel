#include "stdafx.h"
#include <windows.h>
#include <streams.h>
#include <d3d9.h>
#include <dxva.h>
#include <dxva2api.h>
#include <mfidl.h>
#include <evr.h>
#include <amva.h>
#include "Imports/LibGPU/GPUID.h"
#include "HVDGuids.h"
#include "HVDSelector.h"
#include "HVDServiceDxva2.h"
#include <InitGuid.h>
#include "Imports/Inc/cpinterfaces.h"

#define DP_ERR(x)	DP x
#ifndef D3DUSAGE_RESTRICTED_CONTENT
#define D3DUSAGE_RESTRICTED_CONTENT      	    0x00000800L
#endif

static const DXVA2_ExtendedFormat g_Dxva2DefaultSampleFormat =
{
	DXVA2_SampleFieldInterleavedEvenFirst,	// we may switch between interleaved or progressive frames during decoding.
	DXVA2_VideoChromaSubsampling_MPEG2,
	DXVA2_NominalRange_16_235,
	DXVA2_VideoTransferMatrix_Unknown,	// = DXVA2_VideoTransferMatrix_BT601 for SD, DXVA2_VideoTransferMatrix_BT709 for HD
	DXVA2_VideoLighting_Unknown,		// = DXVA2_VideoLighting_dim
	DXVA2_VideoPrimaries_Unknown,		// = DXVA2_VideoPrimaries_BT709
	DXVA2_VideoTransFunc_Unknown		// = DXVA2_VideoTransFunc_22_709
};

using namespace HVDService;

CSurfacePool::CSurfacePool(IDirectXVideoDecoderService* pVidDecService)
	: m_pVidDecService(pVidDecService), m_ppSurfaces(0)
{
	if (!m_pVidDecService)
	{
		delete this;
		return;
	}

	m_pVidDecService->AddRef();
	m_dwSurfaceCount = 0;
	m_DecoderGuid = GUID_NULL;
	m_D3DFormat = D3DFMT_UNKNOWN;
}

CSurfacePool::~CSurfacePool()
{
	Release();
	SAFE_RELEASE(m_pVidDecService);
}

HRESULT CSurfacePool::Allocate(SurfaceInfo* pSurfInfo)
{
	CHECK_POINTER(pSurfInfo);
	if (pSurfInfo->nDecoderGuids==0 || pSurfInfo->pDecoderGuids==0)
		return E_INVALIDARG;

	HRESULT hr = E_FAIL;
	GUID* pDecoderGuidsSupported = 0;
	UINT nDecoderGuidsSupported = 0;
	D3DFORMAT* pD3DFormatsSupported = 0;
	UINT nD3DFormats = 0;
	UINT i=0, j=0, k=0, l=0;

	Release();

	hr = m_pVidDecService->GetDecoderDeviceGuids(&nDecoderGuidsSupported,&pDecoderGuidsSupported);	// Get support decoder device guilds
	if(FAILED(hr))
		return hr;

	for (i = 0; i < pSurfInfo->nDecoderGuids; i++)
	{
		for(k = 0; k < nDecoderGuidsSupported; k++)	// look up GUID pairs from HW side and user's side.
			if(pDecoderGuidsSupported[k] == pSurfInfo->pDecoderGuids[i])
				break;
		if(k == nDecoderGuidsSupported)	// no match found
			continue;

		hr = m_pVidDecService->GetDecoderRenderTargets(pDecoderGuidsSupported[k], &nD3DFormats, &pD3DFormatsSupported);	// Get support render targets
		if(FAILED(hr))
			continue;

		// try preference FourCC first
		if(pSurfInfo->dwPrefFourcc)
		{
			for(j = 0; j < nD3DFormats; j++)
			{
				if(pD3DFormatsSupported[j]==pSurfInfo->dwPrefFourcc)
				{
					for (l = pSurfInfo->dwMaxCount, hr = E_FAIL; l >= pSurfInfo->dwMinCount && FAILED(hr); l--)
						hr = _Allocate(pSurfInfo->dwWidth, pSurfInfo->dwHeight, l, pD3DFormatsSupported[j], pSurfInfo->bRestrictedContent);
					if(SUCCEEDED(hr))
					{
						m_DecoderGuid = pDecoderGuidsSupported[k];
						CoTaskMemFree(pD3DFormatsSupported);
						goto FreeDecGuid;
					}
				}
			}
		}

		for(j = 0; j < nD3DFormats; j++)
		{
#if 0
			if(m_pD3DDev9Man)
			{
				FOURCCMap mapVideoSubType(&m_guidVideoSubType);
				if(mapVideoSubType.GetFOURCC() != pD3FormatsSupported[j])
					continue;
			}
#endif
			for (l = pSurfInfo->dwMaxCount, hr = E_FAIL; l >= pSurfInfo->dwMinCount && FAILED(hr); l--)
				hr = _Allocate(pSurfInfo->dwWidth, pSurfInfo->dwHeight, l, pD3DFormatsSupported[j], pSurfInfo->bRestrictedContent);

			if(SUCCEEDED(hr))
			{
				m_DecoderGuid = pDecoderGuidsSupported[k];
				CoTaskMemFree(pD3DFormatsSupported);
				goto FreeDecGuid;
			}
		}
		CoTaskMemFree(pD3DFormatsSupported);
	}

FreeDecGuid:
	if (pDecoderGuidsSupported)
		CoTaskMemFree(pDecoderGuidsSupported);
	if(i == pSurfInfo->nDecoderGuids)
		hr = E_INVALIDARG;

	return hr;
}

HRESULT CSurfacePool::Release()
{
	while(m_dwSurfaceCount)
	{
		m_dwSurfaceCount--;
		ULONG nRef = m_ppSurfaces[m_dwSurfaceCount]->Release(); ASSERT(nRef == 0);
#if 0
		if(m_aEvent && m_aEvent[m_nSurfAllocated])
			CloseHandle(m_aEvent[m_nSurfAllocated]);
#endif
	}

	SAFE_DELETE_ARRAY(m_ppSurfaces);
	m_D3DFormat = D3DFMT_UNKNOWN;
	m_dwSurfaceCount = 0;
	m_DecoderGuid = GUID_NULL;

#if 0
	if(m_aEvent)
	{
		delete[] m_aEvent; m_aEvent = 0;
	}
#endif
	return S_OK;
}

HRESULT CSurfacePool::GetSurface(DWORD dwIndex, IDirect3DSurface9** ppSurface)
{
	CHECK_POINTER(ppSurface);
	if (dwIndex >= m_dwSurfaceCount)
		return E_INVALIDARG;

	if (m_ppSurfaces)
	{
		*ppSurface = m_ppSurfaces[dwIndex];
		return S_OK;
	}

	return E_ABORT;
}

HRESULT CSurfacePool::_Allocate(DWORD dwWidth, DWORD dwHeight, UINT nSurface, D3DFORMAT D3DFormat, BOOL bRestrictedContent)
{
	SAFE_DELETE_ARRAY(m_ppSurfaces);
	m_ppSurfaces = new IDirect3DSurface9*[nSurface];
	if (!m_ppSurfaces)
		return E_OUTOFMEMORY;

	DWORD dwUsage = bRestrictedContent ? D3DUSAGE_RESTRICTED_CONTENT : 0;
	HRESULT hr = m_pVidDecService->CreateSurface(dwWidth, dwHeight, nSurface - 1, // # of buffers = front + # of back buf
		D3DFormat, D3DPOOL_DEFAULT, dwUsage, DXVA2_VideoDecoderRenderTarget, m_ppSurfaces, NULL);

	if(SUCCEEDED(hr))
	{
		m_dwSurfaceCount = nSurface;
		m_D3DFormat = D3DFormat;
	}
	else
		Release();

	return hr;
}

CHVDServiceDxva2::CHVDServiceDxva2()
{
	m_hDXVA2 = NULL;
	m_dwService = HVD_SERVICE_DXVA2;
	m_pfnDXVA2CreateVideoService = 0;
	m_pVidDecService = 0;
	m_pVidDec = 0;
	m_pDeviceManager = 0;
	m_pDevice = 0;
	m_pMFGetService = 0;
	m_hDevice = 0;
	m_pSurfacePool = 0;
	m_pCP = NULL;
	m_hWnd = NULL;
	m_pFastReadback = NULL;
    m_dwLockSurfaceFirst = FALSE;

	ZeroMemory(&m_Dxva2ConfigPictureDecode, sizeof(m_Dxva2ConfigPictureDecode));

	m_hDXVA2 = LoadLibrary(TEXT("dxva2.dll"));
	if (m_hDXVA2)
	{
		m_pfnDXVA2CreateVideoService = (TpfnDXVA2CreateVideoService)GetProcAddress(m_hDXVA2, "DXVA2CreateVideoService");
	}
	
	if (!m_pfnDXVA2CreateVideoService)
	{
		if (m_hDXVA2)
		{
			FreeLibrary(m_hDXVA2);
			m_hDXVA2 = NULL;
		}
		return;
	}
}

CHVDServiceDxva2::~CHVDServiceDxva2()
{
	CAutoLock lock(&m_csObj);
	SAFE_RELEASE(m_pVidDec);
	ReleaseSurfaces();
	ReleaseVideoService();
	SAFE_DELETE_ARRAY(m_pSupportedDecoderGuids);
	if (m_hDXVA2)
	{
		FreeLibrary(m_hDXVA2);
		m_hDXVA2 = NULL;
	}
}

STDMETHODIMP CHVDServiceDxva2::QueryInterface(REFIID riid, void **ppv)
{
	HRESULT hr = E_NOINTERFACE;

	if (!m_hDXVA2)
	{
		*ppv = NULL;
		return E_UNEXPECTED;
	}
	if (riid == __uuidof(IHVDServiceDxva))
	{
		hr = GetInterface((IHVDServiceDxva *)this, ppv);
	}
	else if (riid == __uuidof(IHVDServiceDxva2))
	{
		hr = GetInterface((IHVDServiceDxva2 *)this, ppv);
	}
    else if (riid == __uuidof(IHVDServiceFastReadBack))
    {
        hr = GetInterface((IHVDServiceFastReadBack *)this, ppv);
    }
	else
	{
		hr = CHVDServiceBase::QueryInterface(riid, ppv);
	}

	return hr;
}

STDMETHODIMP_(ULONG) CHVDServiceDxva2::AddRef()
{
	LONG lRef = InterlockedIncrement(&m_cRef);
	ASSERT(lRef > 0);
	return lRef;
}

STDMETHODIMP_(ULONG) CHVDServiceDxva2::Release()
{
	LONG lRef = InterlockedDecrement(&m_cRef);
	ASSERT(lRef >= 0);
	if (lRef == 0)
		delete this;
	return lRef;
}

STDMETHODIMP CHVDServiceDxva2::GetAccel(IUnknown** ppAccel)
{
	CHECK_POINTER(ppAccel);
	if (m_pVidDec)
	{
		*ppAccel = m_pVidDec;
		(*ppAccel)->AddRef();
		return S_OK;
	}
	return E_FAIL;
}

STDMETHODIMP CHVDServiceDxva2::GetDxvaConfigPictureDecode(HVDDXVAConfigPictureDecode* pDxvaConfigPictureDecode)
{
	// Encryption GUIDs
	pDxvaConfigPictureDecode->guidConfigBitstreamEncryption	 = m_Dxva2ConfigPictureDecode.guidConfigBitstreamEncryption;
	pDxvaConfigPictureDecode->guidConfigMBcontrolEncryption	 = m_Dxva2ConfigPictureDecode.guidConfigMBcontrolEncryption; 
	pDxvaConfigPictureDecode->guidConfigResidDiffEncryption	 = m_Dxva2ConfigPictureDecode.guidConfigResidDiffEncryption; 
	// Bitstream Processing Indicator
	pDxvaConfigPictureDecode->bConfigBitstreamRaw            = m_Dxva2ConfigPictureDecode.ConfigBitstreamRaw; 
	// Macroblock Control Config
	pDxvaConfigPictureDecode->bConfigMBcontrolRasterOrder		 = m_Dxva2ConfigPictureDecode.ConfigMBcontrolRasterOrder; 
	// Host Residue Diff Config
	pDxvaConfigPictureDecode->bConfigResidDiffHost           = m_Dxva2ConfigPictureDecode.ConfigResidDiffHost; 
	pDxvaConfigPictureDecode->bConfigSpatialResid8           = m_Dxva2ConfigPictureDecode.ConfigSpatialResid8; 
	pDxvaConfigPictureDecode->bConfigResid8Subtraction       = m_Dxva2ConfigPictureDecode.ConfigResid8Subtraction; 
	pDxvaConfigPictureDecode->bConfigSpatialHost8or9Clipping = m_Dxva2ConfigPictureDecode.ConfigSpatialHost8or9Clipping; 
	pDxvaConfigPictureDecode->bConfigSpatialResidInterleaved = m_Dxva2ConfigPictureDecode.ConfigSpatialResidInterleaved; 
	pDxvaConfigPictureDecode->bConfigIntraResidUnsigned      = m_Dxva2ConfigPictureDecode.ConfigIntraResidUnsigned; 
	// Accelerator Residue Diff Config
	pDxvaConfigPictureDecode->bConfigResidDiffAccelerator		 = m_Dxva2ConfigPictureDecode.ConfigResidDiffAccelerator; 
	pDxvaConfigPictureDecode->bConfigHostInverseScan         = m_Dxva2ConfigPictureDecode.ConfigHostInverseScan; 
	pDxvaConfigPictureDecode->bConfigSpecificIDCT            = m_Dxva2ConfigPictureDecode.ConfigSpecificIDCT; 
	pDxvaConfigPictureDecode->bConfig4GroupedCoefs           = m_Dxva2ConfigPictureDecode.Config4GroupedCoefs; 

	return S_OK;
}

HRESULT CHVDServiceDxva2::_Initialize(HVDInitConfig* pInitConfig)
{
	CAutoLock lock(&m_csObj);
	CHECK_POINTER(pInitConfig);

	m_hWnd = pInitConfig->hwnd;

	return CreateVideoService(pInitConfig->pExternalDevice);
}

HRESULT CHVDServiceDxva2::_Uninitialize()
{
	CAutoLock lock(&m_csObj);
	ReleaseVideoService();

	return S_OK;
}

HRESULT CHVDServiceDxva2::_StartService()
{
	CAutoLock lock(&m_csObj);
	HRESULT hr = E_FAIL;
	INT HVD_Start_Level = HVD_LEVEL_VLD;

	if(m_HVDDecodeConfig.dwMode == HVD_MODE_MPEG1)
		return E_INVALIDARG;

	if (m_HVDDecodeConfig.dwFlags & HVD_DECODE_MODE)
		m_dwHVDMode = m_HVDDecodeConfig.dwMode;

	if(m_dwVendorID == PCI_VENDOR_ID_INTEL && m_HVDDecodeConfig.dwMaxSurfaceCount>=16)
		m_HVDDecodeConfig.dwMaxSurfaceCount =16; // Intel HW cannot use more than 16 uncompression buffers

	SurfaceInfo SurfInfo = {0};
	SurfInfo.dwWidth = m_HVDDecodeConfig.dwWidth;
	SurfInfo.dwHeight = m_HVDDecodeConfig.dwHeight;
	SurfInfo.dwPrefFourcc = (m_dwInitFlags & HVD_INIT_FASTREADBACK)?MAKEFOURCC('N','V','1','2'):0;
	if (m_HVDDecodeConfig.dwFlags & HVD_DECODE_ENCRYPTION_GPUCP)
		SurfInfo.bRestrictedContent = TRUE;
	if (m_HVDDecodeConfig.dwFlags & HVD_DECODE_MAXSURFACECOUNT)
		SurfInfo.dwMaxCount = m_HVDDecodeConfig.dwMaxSurfaceCount;

	if (HVD_LEVEL_AUTO == m_HVDDecodeConfig.dwLevel || HVD_LEVEL_AUTO_CONSTRAIN_SURFACE_NUM == m_HVDDecodeConfig.dwLevel)
	{
		for (INT i = HVD_Start_Level; i>= HVD_LEVEL_MC; i--)
		{
			SurfInfo.nDecoderGuids = 0;
			SurfInfo.pDecoderGuids = 0;
			if(HVD_LEVEL_AUTO == m_HVDDecodeConfig.dwLevel)
				CHVDSelector::RecommendSurfaceCount(m_dwService, m_dwHVDMode, i, &SurfInfo.dwMaxCount, &SurfInfo.dwMinCount, SurfInfo.dwWidth, SurfInfo.dwHeight);
			CHVDSelector::GetHVDGuids(m_dwService, m_dwHVDMode, i, (const GUID**)&(SurfInfo.pDecoderGuids), &(SurfInfo.nDecoderGuids));
			hr = CreateSurfaces(&SurfInfo);
			if (SUCCEEDED(hr))
			{
				m_HVDDecodeConfig.dwLevel = i;
				break;
			}
		}
	}
	else
	{
		CHVDSelector::RecommendSurfaceCount(m_dwService, m_dwHVDMode, m_HVDDecodeConfig.dwLevel, &SurfInfo.dwMaxCount, &SurfInfo.dwMinCount, SurfInfo.dwWidth, SurfInfo.dwHeight);
		CHVDSelector::GetHVDGuids(m_dwService, m_dwHVDMode, m_HVDDecodeConfig.dwLevel, (const GUID**)&(SurfInfo.pDecoderGuids), &(SurfInfo.nDecoderGuids));
		hr = CreateSurfaces(&SurfInfo);
	}

	if (FAILED(hr))
		return hr;

	hr = CreateDxva2Decoder();
	if (FAILED(hr))
	{
		m_DecoderGuid = GUID_NULL;
		return hr;
	}

	if (m_dwInitFlags & HVD_INIT_FASTREADBACK)
	{
		IDirect3DSurface9** ppSurfaces= m_pSurfacePool->GetSurfaces();		
		FastReadbackInit stReadbackInit = {0};
		stReadbackInit.dwSupportedDecoderCount = m_dwSupportedDecoderCount;
		stReadbackInit.pSupportedDecoderGuids = m_pSupportedDecoderGuids;
		stReadbackInit.dwSurfaceCount = m_dwSurfaceCount;
		stReadbackInit.hDXVA2 = m_hDXVA2;
		stReadbackInit.pVidDec = m_pVidDec;
		stReadbackInit.pDevice = m_pDevice;
		stReadbackInit.d3dSurfaceFormat = m_pSurfacePool->GetFormat();
		stReadbackInit.hWnd = m_hWnd;
		stReadbackInit.ppSurfaces = ppSurfaces;
		// set source surface size
		stReadbackInit.uSrcWidth = SurfInfo.dwWidth;
		stReadbackInit.uSrcHeight = SurfInfo.dwHeight;
		// use the default readback size
		stReadbackInit.uBufWidth = SurfInfo.dwWidth;
		stReadbackInit.uBufHeight = SurfInfo.dwHeight;
        stReadbackInit.dwInputInterlaceFlag = m_FastCopyConfig.dwInputInterlaceFlag;
        stReadbackInit.dwOutputInterlaceFlag = m_FastCopyConfig.dwOutputInterlaceFlag;
		if(m_FastCopyConfig.dwHeightReadback>0 && m_FastCopyConfig.dwHeightReadback<1440 && 
			m_FastCopyConfig.dwWidthReadback>0 && m_FastCopyConfig.dwWidthReadback < 2048) // avoid un-initialize value
		{
			// specify the size of fast read back
			stReadbackInit.uBufWidth = m_FastCopyConfig.dwWidthReadback;
			stReadbackInit.uBufHeight = m_FastCopyConfig.dwHeightReadback;
		}

        stReadbackInit.dwExpectedReadbackWidth = m_FastCopyConfig.dwExpectedReadbackWidth;
        stReadbackInit.dwExpectedReadbackHeight = m_FastCopyConfig.dwExpectedReadbackHeight;

		if (m_dwVendorID == PCI_VENDOR_ID_ATI)
		{
			m_pFastReadback = new CATIFastReadback();
		}
		else if (m_dwVendorID == PCI_VENDOR_ID_INTEL)
		{
			m_pFastReadback = new CIntelFastReadback();
		}
		else if (m_dwVendorID == PCI_VENDOR_ID_NVIDIA)
		{
			m_pFastReadback = new CNVFastReadback();
		}

		hr = m_pFastReadback->Open(&stReadbackInit);
		if (FAILED(hr))
		{
			m_pFastReadback->Close();
			SAFE_DELETE(m_pFastReadback);
			DP_FASTREADBACK("FastReadback create failed");

			//Bug ID: 113073 & 116803: For AMD VGA card, if MCOM open failed and use D3D read back, the resizing will disable.
			//Because D3D can't do de-interlacing.
			if(m_dwVendorID == PCI_VENDOR_ID_ATI)
			{
				stReadbackInit.uBufWidth = SurfInfo.dwWidth;
				stReadbackInit.uBufHeight = SurfInfo.dwHeight;
			}

			// D3D method
			m_pFastReadback = new CD3DFastReadback();
			hr = m_pFastReadback->Open(&stReadbackInit);
		}

        // Configure memento flag.
        if(m_dwLockSurfaceFirst)
            m_pFastReadback->EnableLockSource();
	}

	// Send event notification for start service
	CHVDServiceBase::_StartService();
	return hr;
}

HRESULT CHVDServiceDxva2::_StopService()
{
	CAutoLock lock(&m_csObj);

	// Send event notification for stop service
	CHVDServiceBase::_StopService();

	if (m_pFastReadback)
	{
		m_pFastReadback->Close();
	}

	SAFE_DELETE(m_pFastReadback);
	SAFE_RELEASE(m_pVidDec);
	ReleaseSurfaces();
	return S_OK;
}

STDMETHODIMP CHVDServiceDxva2::LockCompressBuffer(DWORD dwType, DWORD dwIndex, HVDDxvaCompBufLockInfo *pInfo, BOOL bReadOnly)
{
	CAutoLock lock(&m_csObj);

	CHECK_POINTER(pInfo);
	if (!m_pVidDec)
		return E_ABORT;

	HRESULT hr = m_pVidDec->GetBuffer(dwType, &(pInfo->pBuffer), &(pInfo->uSize));
	return hr;
}

STDMETHODIMP CHVDServiceDxva2::UnlockCompressBuffer(DWORD dwType, DWORD dwIndex)
{
	CAutoLock lock(&m_csObj);

	if (!m_pVidDec)
		return E_ABORT;

	HRESULT hr = m_pVidDec->ReleaseBuffer(dwType);
	return hr;
}

STDMETHODIMP CHVDServiceDxva2::BeginFrame(DWORD dwDstSurfIndex)
{
	CAutoLock lock(&m_csObj);

	if (!m_pSurfacePool || !m_pVidDec)
		return E_ABORT;

	IDirect3DSurface9 *pD3DSurf9 = 0;
	HRESULT hr = m_pSurfacePool->GetSurface(dwDstSurfIndex, &pD3DSurf9);

	hr = CHVDServiceBase::_NotifyEvent(HVD_EVENT_UNCOMPBUF_AVAILABILITY_QUERY, (DWORD)pD3DSurf9, 0);

	if (SUCCEEDED(hr))
	{
		if((m_pCP!=NULL)&&(m_pCP->GetObjID()==E_CP_ID_WIN7))
		{
			IGetParams *pGetParams;
			m_pCP->QueryInterface(IID_IGetParams, (LPVOID*)&pGetParams);
			hr = m_pVidDec->BeginFrame(pD3DSurf9, pGetParams->GetContentKey());
			pGetParams->Release();
		}
		else
		hr = m_pVidDec->BeginFrame(pD3DSurf9, 0);
	}
	return hr;
}

STDMETHODIMP CHVDServiceDxva2::Execute(HVDDxvaExecuteConfig* pExecuteConfig)
{
	CAutoLock lock(&m_csObj);

	CHECK_POINTER(pExecuteConfig);
	if (!m_pVidDec)
		return E_ABORT;

	HRESULT hr = E_FAIL;

	DXVA2_DecodeBufferDesc		DecodeBufferDesc[10];
	DXVA2_DecodeExecuteParams Params = {0};

	memset(&DecodeBufferDesc[0], 0, 10*sizeof(DecodeBufferDesc[0]));

	Params.NumCompBuffers = pExecuteConfig->dwNumBuffers;
	Params.pCompressedBuffers = &DecodeBufferDesc[0];

	DXVA2_DecodeExtensionData ExtData = {0};
	if (pExecuteConfig->dwFlags & HVD_DXVA2_EXECUTE_EXTDATA)
	{
		ExtData.Function = pExecuteConfig->dwFunction;
		ExtData.pPrivateInputData = pExecuteConfig->lpPrivateInputData;
		ExtData.PrivateInputDataSize = pExecuteConfig->cbPrivateInputData;
		ExtData.pPrivateOutputData = pExecuteConfig->lpPrivateOutputData;
		ExtData.PrivateOutputDataSize = pExecuteConfig->cbPrivateOutputData;
		Params.pExtensionData = &ExtData;
	}
	else
		Params.pExtensionData = NULL;

	AMVABUFFERINFO*          pBufferInfo = (AMVABUFFERINFO*)pExecuteConfig->lpAmvaBufferInfo;
	DXVA_BufferDescription*  pBufferDest = (DXVA_BufferDescription*)pExecuteConfig->lpPrivateInputData;

	if (pExecuteConfig->cbPrivateInputData != sizeof(DXVA_BufferDescription)*Params.NumCompBuffers)
		return E_FAIL;

	LPVOID pIV=NULL;
	if((m_pCP!=NULL)&&(m_pCP->GetObjID()==E_CP_ID_WIN7))
	{
		IGetParams *pGetParams;
		m_pCP->QueryInterface(IID_IGetParams, (LPVOID*)&pGetParams);
		pIV = pGetParams->GetIV();
		pGetParams->Release();
	}

	for (UINT i=0; i<Params.NumCompBuffers; i++)
	{
		DecodeBufferDesc[i].CompressedBufferType = pBufferInfo[i].dwTypeIndex;
		DecodeBufferDesc[i].DataOffset           = pBufferDest[i].dwDataOffset;
		DecodeBufferDesc[i].DataSize             = pBufferDest[i].dwDataSize;
		DecodeBufferDesc[i].ReservedBits         = 0;
		if(pIV!=NULL)
		{	
			if ((DXVA2_ResidualDifferenceBufferType==pBufferInfo[i].dwTypeIndex) ||
			    (DXVA2_BitStreamDateBufferType==pBufferInfo[i].dwTypeIndex))				
			DecodeBufferDesc[i].pvPVPState           = pIV;
		}	
		DecodeBufferDesc[i].FirstMBaddress       = pBufferDest[i].dwFirstMBaddress;
		DecodeBufferDesc[i].NumMBsInBuffer       = pBufferDest[i].dwNumMBsInBuffer;
		DecodeBufferDesc[i].Width                = pBufferDest[i].dwWidth;
		DecodeBufferDesc[i].Height               = pBufferDest[i].dwHeight;
		DecodeBufferDesc[i].Stride               = pBufferDest[i].dwStride;
	}

	hr = m_pVidDec->Execute(&Params);
	return hr;
}
STDMETHODIMP CHVDServiceDxva2::EndFrame(DWORD dwDstSurfIndex)
{
	CAutoLock lock(&m_csObj);
	if (!m_pSurfacePool || !m_pVidDec)
		return E_ABORT;

	return m_pVidDec->EndFrame(0);
}

STDMETHODIMP CHVDServiceDxva2::GetUncompressedBufferCount(DWORD *pdwCount)
{
	CHECK_POINTER(pdwCount);
	*pdwCount = m_dwSurfaceCount;
	return S_OK;
}

STDMETHODIMP CHVDServiceDxva2::GetUncompresesdBufferFormat(DWORD* pdwFourCC)
{
	CAutoLock lock(&m_csObj);

	CHECK_POINTER(pdwFourCC);
	if (!m_pSurfacePool)
		return E_ABORT;

	*pdwFourCC = static_cast<DWORD>(m_pSurfacePool->GetFormat());
	return S_OK;
}

STDMETHODIMP CHVDServiceDxva2::GetUncompressedBuffer(DWORD dwIndex, IDirect3DSurface9 **ppSurface)
{
	CAutoLock lock(&m_csObj);

	CHECK_POINTER(ppSurface);
	if (!m_pSurfacePool)
		return E_ABORT;

	HRESULT hr = m_pSurfacePool->GetSurface(dwIndex, ppSurface);
	return hr;
}

STDMETHODIMP CHVDServiceDxva2::LockUncompressedBuffer(DWORD dwIndex, HVDDxva2UncompBufLockInfo *pInfo)
{
	CAutoLock lock(&m_csObj);

	CHECK_POINTER(pInfo);
	if (!m_pSurfacePool)
		return E_ABORT;

	IDirect3DSurface9 *pD3DSurf9 = 0;
	HRESULT hr = m_pSurfacePool->GetSurface(dwIndex, &pD3DSurf9);
	if (SUCCEEDED(hr))
	{				 
		D3DLOCKED_RECT d3dLockRect = {0};
		hr = pD3DSurf9->LockRect(&d3dLockRect, NULL, 0);
		if (SUCCEEDED(hr))
		{
			pInfo->pBits = d3dLockRect.pBits;
			pInfo->uPitch = d3dLockRect.Pitch;
			pInfo->dwFourCC = m_pSurfacePool->GetFormat();	
		}
	}
			
	return hr;
}

STDMETHODIMP CHVDServiceDxva2::UnlockUncompressedBuffer(DWORD dwIndex)
{
	CAutoLock lock(&m_csObj);

	if (!m_pSurfacePool)
		return E_ABORT;

	IDirect3DSurface9 *pD3DSurf9 = 0;
	HRESULT hr = m_pSurfacePool->GetSurface(dwIndex, &pD3DSurf9);
	if (SUCCEEDED(hr))
		hr = pD3DSurf9->UnlockRect();
	return hr;
}

STDMETHODIMP CHVDServiceDxva2::SetCP(LPVOID pIviCP)
{
	m_pCP = (ICPService*)pIviCP;

	return S_OK;
}

STDMETHODIMP CHVDServiceDxva2::FastReadback(DWORD dwIndex, HVDDxva2UncompBufLockInfo* pInfo, HVDFastReadbackOption* pReadbackOption)
{
	HRESULT hr = E_FAIL;
	IDirect3DSurface9 *pD3DSurf9 = 0;

	hr = m_pSurfacePool->GetSurface(dwIndex, &pD3DSurf9);
	if (SUCCEEDED(hr))
	{
		if (m_pFastReadback && pReadbackOption->dwReturnFourcc)
		{
			hr = m_pFastReadback->Readback(pD3DSurf9, dwIndex, pInfo, pReadbackOption);
		}
		else
		{
			if(m_pFastReadback->GetID()==FAST_READBACK_INTEL) // for SNB, FC can do re-scaling or de-interlacing
			{
				hr = m_pFastReadback->Readback(pD3DSurf9, dwIndex, pInfo, pReadbackOption);
				if(SUCCEEDED(hr))
					return hr;
			}

			if (pReadbackOption->dwReturnFourcc == FALSE)
			{
				pReadbackOption->pReturnSurface = pD3DSurf9;
			}
			else
			{
				hr = LockUncompressedBuffer(dwIndex, pInfo);
				if ( SUCCEEDED(hr))
				{
					hr = UnlockUncompressedBuffer(dwIndex);
				}
			}
		}
	}
	return hr;
}

HRESULT CHVDServiceDxva2::SetParameter(DWORD dwParamType, void* pvdParam, DWORD* pdwParamSize)
{
    HRESULT hRet = S_OK;
    switch (dwParamType)
    {
    case HVD_FB_PARAM_TYPE_SET_PICH_COPY:
        m_pFastReadback->SetPitchCopy(TRUE);
        break;
    case HVD_FB_PARAM_TYPE_SET_MSDK_MEM_ALLOCATOR:
        m_pFastReadback->InstallMediaAllocator(pvdParam);
        break;
    case HVD_FB_PARAM_TYPE_SET_DISPLAY_RECT:
        {
            RECT* pDisplayRect = (RECT*)pvdParam;
            m_pFastReadback->SetDisplayArea(pDisplayRect->right, pDisplayRect->bottom);
        }
        break;
    case HVD_FB_PARAM_TYPE_SET_LOCK_SURFACE:
        {
            m_pFastReadback->EnableLockSource();
            m_dwLockSurfaceFirst=TRUE; // if resolution change, readback will reopen. This flag need to set again.
        }
        break;
    default: hRet = S_FALSE; 
        break;
    }
    return hRet;
}

HRESULT CHVDServiceDxva2::GetParameter(DWORD dwParamType, void* pvdParam, DWORD* pdwParamSize)
{
    HRESULT hRet = S_OK;
    switch(dwParamType)
    {
    case HVD_FB_PARAM_TYPE_GET_NVIDIA_CAPS:
    case HVD_FB_PARAM_TYPE_GET_INTEL_CAPS:
        break;
    case HVD_FB_PARAM_TYPE_GET_AMD_SCALING_CAPS:
        {
            if(pvdParam && m_pFastReadback->GetID()==FAST_READBACK_AMD)
            {
                MCOM_DECODE_TARGET_ACCESS_SCALING_CAPS mcomCapsOutput = {0};
                if((hRet=m_pFastReadback->GetCaps(E_CAPS_TYPE_SCALING, &mcomCapsOutput))==S_OK)
                {
                    *pdwParamSize = (*pdwParamSize<sizeof(MCOM_DECODE_TARGET_ACCESS_SCALING_CAPS))? *pdwParamSize:sizeof(MCOM_DECODE_TARGET_ACCESS_SCALING_CAPS);
                    memcpy(pvdParam, &mcomCapsOutput, *pdwParamSize);
                }
                else
                {
                    *pdwParamSize = 0;			
                    hRet=E_FAIL;
                }
            }
            else
                hRet=E_FAIL;
        }
        break;
    default:
        DP_ERR(("Not support ParamType- %d\n", dwParamType));
        break;
    }

    return hRet;
}

HRESULT CHVDServiceDxva2::CreateVideoService(IUnknown *pDevice)
{
	CHECK_POINTER(pDevice);

	HRESULT hr = E_FAIL;
	if (m_dwInitFlags & HVD_INIT_D3DDEVICE)
		hr = pDevice->QueryInterface(__uuidof(IDirect3DDevice9), (void **)&m_pDevice);
	else if (m_dwInitFlags & HVD_INIT_MFGETSERVICE)
		hr = pDevice->QueryInterface(__uuidof(IMFGetService), (void**)&m_pMFGetService);

	if (FAILED(hr))
		return E_INVALIDARG;

	if (m_pDevice)
		hr = m_pfnDXVA2CreateVideoService(m_pDevice, __uuidof(IDirectXVideoDecoderService), (void**) &m_pVidDecService);
	else if (m_pMFGetService)
	{
		hr = m_pMFGetService->GetService(MR_VIDEO_ACCELERATION_SERVICE,__uuidof(IDirect3DDeviceManager9), (void**)&m_pDeviceManager);
		if (SUCCEEDED(hr))
		{
			hr = m_pDeviceManager->OpenDeviceHandle(&m_hDevice);
			if(SUCCEEDED(hr))
				hr = m_pDeviceManager->GetVideoService(m_hDevice, __uuidof(IDirectXVideoDecoderService), (void**) &m_pVidDecService);
		}
		//m_pMFGetService->GetService(MR_VIDEO_ACCELERATION_SERVICE, IID_IDirectXVideoMemoryConfiguration, (void**)&m_pDxVideoMemConfig);
	}
	else
		return E_FAIL;

	if (m_pVidDecService)
	{
		UINT nCount = 0;
		GUID* pGuids = 0;
		hr = m_pVidDecService->GetDecoderDeviceGuids(&nCount,&pGuids);	// Get support decoder device guilds
		if (SUCCEEDED(hr))
		{
			m_dwSupportedDecoderCount = nCount;
			if (pGuids)
			{
				SAFE_DELETE_ARRAY(m_pSupportedDecoderGuids);
				m_pSupportedDecoderGuids = new GUID[nCount];
				for (UINT i=0; i<nCount; i++)
					m_pSupportedDecoderGuids[i] = pGuids[i];

				CoTaskMemFree(pGuids);
			}
		}
	}
	return hr;
}

HRESULT CHVDServiceDxva2::ReleaseVideoService()
{
	if(m_hDevice)
	{
		m_pDeviceManager->CloseDeviceHandle(m_hDevice); 
		m_hDevice = 0;
	}

	SAFE_RELEASE(m_pVidDecService);
	SAFE_RELEASE(m_pDevice);
	SAFE_RELEASE(m_pDeviceManager);
	SAFE_RELEASE(m_pMFGetService);

	m_dwHVDMode = HVD_MODE_UNKNOWN;
	m_pCP = NULL;
	return S_OK;
}

HRESULT CHVDServiceDxva2::CreateSurfaces(SurfaceInfo* pSurfInfo)
{
	CHECK_POINTER(pSurfInfo);

	if (!m_pVidDecService)
		return E_ABORT;

	if (m_pSurfacePool)
		ReleaseSurfaces();

	m_pSurfacePool = new CSurfacePool(m_pVidDecService);
	if (!m_pSurfacePool)
		return E_FAIL;

	HRESULT hr = m_pSurfacePool->Allocate(pSurfInfo);
	if (SUCCEEDED(hr))
	{
		m_dwSurfaceCount = m_pSurfacePool->GetCount();
		m_DecoderGuid = m_pSurfacePool->GetDecodeGUID();
	}
	else
		ReleaseSurfaces();

	return hr;
}

HRESULT CHVDServiceDxva2::ReleaseSurfaces()
{
	m_dwSurfaceCount = 0;
	m_DecoderGuid = GUID_NULL;
	SAFE_DELETE(m_pSurfacePool);
	return S_OK;
}

HRESULT CHVDServiceDxva2::CreateDxva2Decoder()
{
	if (!m_pVidDecService || !m_pSurfacePool)
		return E_ABORT;

	SAFE_RELEASE(m_pVidDec);

	HRESULT hr = E_FAIL;
	DXVA2_VideoDesc dxva2Desc = {0};
	UINT nConfig=0, nUsedConfig=0;
	DXVA2_ConfigPictureDecode *pConfigs;

	dxva2Desc.SampleWidth = m_HVDDecodeConfig.dwWidth;
	dxva2Desc.SampleHeight = m_HVDDecodeConfig.dwHeight;
	dxva2Desc.SampleFormat = g_Dxva2DefaultSampleFormat;
	dxva2Desc.Format = m_pSurfacePool->GetFormat();
	dxva2Desc.InputSampleFreq.Numerator = dxva2Desc.OutputFrameFreq.Numerator = 30000;
	dxva2Desc.InputSampleFreq.Denominator = dxva2Desc.OutputFrameFreq.Denominator = 1001;
	dxva2Desc.UABProtectionLevel = (m_HVDDecodeConfig.dwFlags&HVD_DECODE_UABPROTECTLEVEL) ? m_HVDDecodeConfig.dwUABProtectionLevel : 0;

	hr = m_pVidDecService->GetDecoderConfigurations(m_DecoderGuid, &dxva2Desc, 0, &nConfig, &pConfigs);
	if(FAILED(hr))
	{
		m_DecoderGuid = GUID_NULL;
		return hr;
	}

	while(nConfig != nUsedConfig)
	{
		pConfigs[nUsedConfig].guidConfigBitstreamEncryption = GUID_NULL;
		pConfigs[nUsedConfig].guidConfigMBcontrolEncryption = GUID_NULL;
		pConfigs[nUsedConfig].guidConfigResidDiffEncryption = GUID_NULL;

		if (PCI_VENDOR_ID_ATI == m_dwVendorID)
		{
			if (m_HVDDecodeConfig.dwFlags&HVD_DECODE_ENCRYPTION_PRIVATE)
			{
				if (m_DecoderGuid == DXVA2_ModeVC1_D || m_DecoderGuid == DXVA_ModeH264_E)
					pConfigs[nUsedConfig].guidConfigBitstreamEncryption = DXVA_ATIDRM_Encrypt;
				else if (m_DecoderGuid == DXVA_ModeH264_ATI_A)
					pConfigs[nUsedConfig].guidConfigResidDiffEncryption = DXVA_ATIDRM_Encrypt;
			}
		}
		else if (PCI_VENDOR_ID_INTEL == m_dwVendorID)
		{
			if(m_HVDDecodeConfig.dwFlags&HVD_DECODE_ENCRYPTION_PRIVATE)
			{
				if(pConfigs[nUsedConfig].ConfigBitstreamRaw)
					pConfigs[nUsedConfig].guidConfigBitstreamEncryption = DXVA2_Intel_Pavp;
				if(pConfigs[nUsedConfig].ConfigResidDiffHost || pConfigs[nUsedConfig].ConfigResidDiffAccelerator)
					pConfigs[nUsedConfig].guidConfigResidDiffEncryption = DXVA2_Intel_Pavp;
			}
		}
		else if (PCI_VENDOR_ID_NVIDIA == m_dwVendorID)
		{
			if(pConfigs[nUsedConfig].ConfigResidDiffAccelerator==1)
				pConfigs[nUsedConfig].ConfigHostInverseScan	= 1;

			if(pConfigs[nUsedConfig].ConfigBitstreamRaw==1)
				pConfigs[nUsedConfig].ConfigBitstreamRaw = 2; //Force using short format for Bistream profile of Nvidia verdor.
		}

		hr = m_pVidDecService->CreateVideoDecoder(m_DecoderGuid, &dxva2Desc, &pConfigs[nUsedConfig], m_pSurfacePool->GetSurfaces(), m_pSurfacePool->GetCount(), &m_pVidDec);
		if (SUCCEEDED(hr))
		{
			m_Dxva2ConfigPictureDecode = pConfigs[nUsedConfig];
			break;
		}

		nUsedConfig++;
	}

	CoTaskMemFree(pConfigs);
	return hr;
}