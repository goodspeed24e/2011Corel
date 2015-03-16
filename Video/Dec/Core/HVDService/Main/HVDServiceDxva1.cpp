#include "stdafx.h"
#include <windows.h>
#include <streams.h>
#include <dvdmedia.h>
#include <d3d9.h>
#include <dxva.h>
#include <dxva2api.h>
#include <vmr9.h>
#include <mfidl.h>
#include "Imports/LibGPU/GPUID.h"
#include "HVDGuids.h"
#include "HVDSelector.h"
#include "HVDServiceDxva1.h"
#include "HVDDxva1Binder.h"
#include "HVDDSOutPin.h"
#include "HVDDSHelper.h"
#include "HVDDSAMVANotify.h"

#define DXVA_BUF_128MB   8
#define DXVA_REFRESH_TIME 333666

#define HVD_DEFAULT_TRACE_LEVEL 3
#define HVD_DEFAULT_ERROR_LEVEL 3

enum CGPIsampleproperty
{
	GPI_SAMPLE_NOTASYNCPOINT = 0x1,
	GPI_SAMPLE_LAST_TS		= 0x2,
	GPI_SAMPLE_NO_TS		= 0x4,
	GPI_SAMPLE_INVALID		= 0x8,
	// used internally by the video renderer
	GPI_SAMPLE_FIELD1FIRST	= 0x10,
	GPI_SAMPLE_FIELD2FIRST	= 0x20,
	GPI_SAMPLE_USEBUFFERSYNC = 0x40,
	GPI_SAMPLE_WAITFORDXVASAMPLE = 0x100	// used for DXVA and VMR9 with DXVA
};

using namespace HVDService;

CHVDServiceDxva1::CHVDServiceDxva1()
{
	m_hWnd = NULL;

	m_dwService = HVD_SERVICE_DXVA1;
	m_pVideoAccel = 0;
	m_pBinder = NULL;
	m_pGraph = NULL;

	m_DecoderGuid = GUID_NULL;

	memset(&m_DxvaConfigPictureDecode, 0, sizeof(m_DxvaConfigPictureDecode));
	memset(&m_DxvaConfigAlphaLoad, 0, sizeof(m_DxvaConfigAlphaLoad));
	memset(&m_DxvaConfigAlphaCombine, 0, sizeof(m_DxvaConfigAlphaCombine));
	memset(&m_UncompSurfacesConfig, 0, sizeof(m_UncompSurfacesConfig));

	m_pGraph = 0;
	m_pVideoRender = 0;
	m_pClock = 0;
	m_dwRotReg = 0;

	m_dwVideoRendererType = HVD_DXVA1_RENDERER_OVERLAY;
	m_dwSurfaceCount = 12;

	m_rtStart = 0;
	m_rtAvgTime = 0;
	m_bSyncStreamTime = FALSE;

	m_dwSurfaceType = 0;
	m_bConfigDecoder = FALSE;
	m_dwAlphaBufWidth = 0;
	m_dwAlphaBufHeight = 0;
	m_dwAlphaBufSize = 0;

	m_bDecodeBufHold = FALSE;
	m_dwDecodeFrame = 0;
	m_dwDecodeBufNum = 0;
	m_bUseEncryption = FALSE;
	m_dwSWDeinterlace = 0;

	m_dwVideoOutputPinBuffer = 6;
	m_dwSubOutputPinBuffer = 2;

	m_dwMinSurfaceCount = 5;
	m_bFIXFPS24Enabled = FALSE;
	m_bRecommendSurfCount = TRUE;
	m_bYUVMixing = TRUE;
	m_bYUVDecimateBy2 = FALSE;
	m_bSubPicPin = FALSE;
	m_bInterlaceSample = TRUE;
	m_bNVIDCT = TRUE;

}

CHVDServiceDxva1::~CHVDServiceDxva1()
{
	CAutoLock lock(&m_csObj);
	if (m_pVideoAccel)
	{
		m_pVideoAccel->Release();
		m_pVideoAccel = NULL;
	}

	if (m_pBinder)
	{
		m_pBinder->Release();
		m_pBinder = NULL;
	}

	if (m_pGraph)
	{
		m_pGraph->Release();
		m_pGraph = NULL;
	}

	SAFE_DELETE_ARRAY(m_pSupportedDecoderGuids);
}

//IUnknown
STDMETHODIMP CHVDServiceDxva1::QueryInterface(REFIID riid, void **ppv)
{
	HRESULT hr = E_NOINTERFACE;

	if (riid == __uuidof(IHVDServiceDxva))
	{
		hr = GetInterface((IHVDServiceDxva *)this, ppv);
	}
	else if (riid == __uuidof(IHVDServiceDxva1))
	{
		hr = GetInterface((IHVDServiceDxva1 *)this, ppv);
	}
	else
	{
		hr = CHVDServiceBase::QueryInterface(riid, ppv);
	}

	return hr;
}

STDMETHODIMP_(ULONG) CHVDServiceDxva1::AddRef()
{
	LONG lRef = InterlockedIncrement(&m_cRef);
	ASSERT(lRef > 0);
	return lRef;
}

STDMETHODIMP_(ULONG) CHVDServiceDxva1::Release()
{
	LONG lRef = InterlockedDecrement(&m_cRef);
	ASSERT(lRef >= 0);
	if (lRef == 0)
		delete this;
	return lRef;
}

HRESULT CHVDServiceDxva1::_Initialize(HVDInitConfig* pInitConfig)
{
	CAutoLock lock(&m_csObj);

	CHECK_POINTER(pInitConfig);

	HRESULT hr= E_FAIL;
	m_hWnd = pInitConfig->hwnd;

	if (pInitConfig->dwFlags & HVD_INIT_BASEFILTER)
	{
		if (!pInitConfig->pExternalDevice)
			return E_INVALIDARG;

		IBaseFilter *pFilter = NULL;
		hr = pInitConfig->pExternalDevice->QueryInterface(IID_IBaseFilter,(void **)&pFilter);
		if (SUCCEEDED(hr) && pFilter)
		{
			if (m_pVideoRender)
			{
				m_pVideoRender->Release();
				m_pVideoRender = NULL;
			}
			m_pVideoRender = pFilter;
			m_pVideoRender->AddRef();
			pFilter->Release();

			m_dwVideoRendererType = HVD_DXVA1_RENDERER_EXTERNAL;
		}
		return hr;
	}

	if (!((pInitConfig->dwFlags & HVD_INIT_DXVA1_FLAGS)))
		return E_INVALIDARG;

	m_dwVideoRendererType = (pInitConfig->m_Dxva1Flags.dwFlags & HVD_INIT_DXVA1_RENDERER_TYPE)
		? pInitConfig->m_Dxva1Flags.dwDxva1RendererType : HVD_DXVA1_RENDERER_VMR9CUSTOM;
	switch (m_dwVideoRendererType)
	{
		case HVD_DXVA1_RENDERER_VMR9CUSTOM:
			m_dwSurfaceCount = 12;
			break;
		case HVD_DXVA1_RENDERER_VMR9WIDNOWLESS:
			m_dwSurfaceCount = 12;
			break;
		case HVD_DXVA1_RENDERER_VMR7:
			m_dwSurfaceCount = 15;
			break;
		case HVD_DXVA1_RENDERER_OVERLAY:
			m_dwSurfaceCount = 15;
			break;
		default:
			m_dwSurfaceCount = 12;
			break;
	}

	if(pInitConfig->m_Dxva1Flags.dwFlags & HVD_INIT_DXVA1_VIDEO_OUTPUT_BUFFER)
		m_dwVideoOutputPinBuffer = pInitConfig->m_Dxva1Flags.dwVideoOutputPinBuf;
	if(pInitConfig->m_Dxva1Flags.dwFlags & HVD_INIT_DXVA1_SUB_OUTPUT_BUFFER)
		m_dwSubOutputPinBuffer = pInitConfig->m_Dxva1Flags.dwSubOutputPinBuf;
	if(pInitConfig->m_Dxva1Flags.dwFlags & HVD_INIT_DXVA1_FIXED_24FPS)
		m_bFIXFPS24Enabled = pInitConfig->m_Dxva1Flags.bFixFPS24Enabled;
	if(pInitConfig->m_Dxva1Flags.dwFlags & HVD_INIT_DXVA1_RECOMMEND_SURFACE_COUNT)
		m_bRecommendSurfCount = pInitConfig->m_Dxva1Flags.bRecommendSurfCount;
	if(pInitConfig->m_Dxva1Flags.dwFlags & HVD_INIT_DXVA1_YUV_MIXING)
		m_bYUVMixing = pInitConfig->m_Dxva1Flags.bYUVMixing;
	if(pInitConfig->m_Dxva1Flags.dwFlags & HVD_INIT_DXVA1_YUV_DECIMATE_BY_2)
		m_bYUVDecimateBy2 = pInitConfig->m_Dxva1Flags.bYUVDecimateBy2;
	if(pInitConfig->m_Dxva1Flags.dwFlags & HVD_INIT_DXVA1_SUB_PIC_PIN)
		m_bSubPicPin = pInitConfig->m_Dxva1Flags.bSubPicPin;
	if(pInitConfig->m_Dxva1Flags.dwFlags & HVD_INIT_DXVA1_INTERLACE_SAMPLE)
		m_bInterlaceSample = pInitConfig->m_Dxva1Flags.bInterlaceSample;
	if(pInitConfig->m_Dxva1Flags.dwFlags & HVD_INIT_DXVA1_NVIDIA_IDCT)
		m_bNVIDCT = pInitConfig->m_Dxva1Flags.bNVIDCT;

	hr = CreateVideoRenderer();
	if (FAILED(hr))
	{
		DbgLog((LOG_ERROR, HVD_DEFAULT_ERROR_LEVEL,"CHVDServiceDxva1::_Initialize: unable to create Video Renderer, hr=0x%08x\n", hr));
	}
	return hr;
}

// CHVDServiceBase
HRESULT CHVDServiceDxva1::_Uninitialize()
{
	CAutoLock lock(&m_csObj);
	HRESULT hr;
	hr = DestroyGraph();
	//ReleaseVideoService();
	return S_OK;
}

HRESULT CHVDServiceDxva1::_StartService()
{
	CAutoLock lock(&m_csObj);
	HRESULT hr = E_FAIL;

	if ((!m_pVideoRender) || (!m_pGraph && !(m_dwInitFlags & HVD_INIT_BASEFILTER)))
		return hr;

 	if (m_HVDDecodeConfig.dwMode == HVD_MODE_MPEG1)
 		return E_INVALIDARG;

	m_dwWidth = m_HVDDecodeConfig.dwWidth;
	m_dwHeight = m_HVDDecodeConfig.dwHeight;

	if (m_HVDDecodeConfig.dwFlags & HVD_DECODE_MODE)
		m_dwHVDMode = m_HVDDecodeConfig.dwMode;

	if (m_HVDDecodeConfig.dwFlags & HVD_DECODE_MAXSURFACECOUNT)
		m_dwSurfaceCount = m_HVDDecodeConfig.dwMaxSurfaceCount;
	if (m_HVDDecodeConfig.dwFlags & HVD_DECODE_MINSURFACECOUNT)
		m_dwMinSurfaceCount = m_HVDDecodeConfig.dwMinSurfaceCount;

	if (m_dwInitFlags & HVD_INIT_BASEFILTER) // HVDService can ignore config parameter,since these config is used for binder.
	{
		hr = UpdateAMVideoAccelerator();
		if (FAILED(hr))
			return hr;

		hr = InitializeDXVAConfig();
		if (FAILED(hr))
			return hr;

		DWORD dwNumFormatsSupported = 1;
		DDPIXELFORMAT ddFormatsSupported;
		hr = m_pVideoAccel->GetUncompFormatsSupported(&m_DecoderGuid, &dwNumFormatsSupported, &ddFormatsSupported);
		if (FAILED(hr))
		{
			DbgLog((LOG_ERROR, HVD_DEFAULT_ERROR_LEVEL,"CHVDServiceDxva1::RunDXVAGraph: GetUncompFormatsSupported failed, hr=0x%08x\n", hr));
			return hr;
		}

		m_dwSurfaceType = ddFormatsSupported.dwFourCC;
		m_bConfigDecoder = TRUE;
		if (m_dwSurfaceCount < m_dwMinSurfaceCount)
		{
			DbgLog((LOG_ERROR, HVD_DEFAULT_ERROR_LEVEL,"CHVDServiceDxva1::RunDXVAGraph: insufficient surfaces"));
			return E_FAIL;
		}
		return S_OK;
	}

	if (!m_pBinder)
	{
		m_pBinder = new CHVDDxva1Binder();

		if (!m_pBinder)
			return E_OUTOFMEMORY;

		m_pBinder->AddRef();

		m_pBinder->SetVideoRender(m_pVideoRender);

		hr = m_pGraph->AddFilter(m_pBinder, L"Video Renderer Binder");
		if (FAILED(hr))
		{
			DbgLog((LOG_ERROR, HVD_DEFAULT_ERROR_LEVEL,"CHVDServiceDxva1::_StartService: unable to add Video Renderer Binder, hr=0x%08x\n", hr));
			return hr;
		}
	}
	m_pBinder->SetOutputPinBufferCount( 0, m_dwVideoOutputPinBuffer);
	m_pBinder->SetOutputPinBufferCount( 1, m_dwSubOutputPinBuffer);

	m_pBinder->SetVideoDecodeInfo(m_dwWidth, m_dwHeight, (HVDService::HVD_MODE)m_dwHVDMode);

	if(m_bRecommendSurfCount)
	{
		CHVDSelector::RecommendSurfaceCount(m_dwService, m_dwHVDMode, m_HVDDecodeConfig.dwLevel,
			&m_UncompSurfacesConfig.dwMaxSurfaceCount, &m_UncompSurfacesConfig.dwMinSurfaceCount, m_dwWidth, m_dwHeight);
		m_pBinder->SetUncompSurfacesConfig(&m_UncompSurfacesConfig);
	}

#ifdef _DEBUG
	if (!m_dwRotReg)
		HVDService::AddIntoROT(m_pGraph, &m_dwRotReg);
#endif

	if(	m_dwVideoRendererType == HVD_DXVA1_RENDERER_VMR9CUSTOM || 
		m_dwVideoRendererType == HVD_DXVA1_RENDERER_VMR9WIDNOWLESS ||
		m_dwVideoRendererType == HVD_DXVA1_RENDERER_VMR7)
		hr = ConnectGraph();
	
	if (FAILED(hr))
		return hr;

	if (!m_pClock)
	{
		// Set the graph clock as system clock. we need to do it before graph running.
		hr = CoCreateInstance(CLSID_SystemClock, NULL, CLSCTX_INPROC_SERVER, IID_IReferenceClock, (void**)&m_pClock);
		if (SUCCEEDED(hr))
		{
			IMediaFilter *pMediaFilter = 0;
			hr = m_pGraph->QueryInterface(IID_IMediaFilter, (void**)&pMediaFilter);
			hr = pMediaFilter->SetSyncSource(m_pClock);
			pMediaFilter->Release();
		}
	}

	hr = RunDXVAGraph();

	if (FAILED(hr))
		return hr;

	CHVDServiceBase::_StartService();
	return hr;
}

HRESULT CHVDServiceDxva1::_StopService()
{
	CAutoLock lock(&m_csObj);

	Stop();
	HRESULT hr;
	hr = DisconnectGraph();
	//Send event notification for stop service
	CHVDServiceBase::_StopService();

	return S_OK;
}

//IHVDServiceDxva
STDMETHODIMP CHVDServiceDxva1::LockCompressBuffer(DWORD dwType, DWORD dwIndex, HVDDxvaCompBufLockInfo *pInfo, BOOL bReadOnly)
{
	CHECK_POINTER(pInfo);
	if (!m_pVideoAccel)
		return E_ABORT;

	CAutoLock lock(&m_csObj);
	HRESULT hr = m_pVideoAccel->GetBuffer(dwType, dwIndex, bReadOnly, &(pInfo->pBuffer), &(pInfo->lStride));

	return hr;
}

STDMETHODIMP CHVDServiceDxva1::UnlockCompressBuffer(DWORD dwType, DWORD dwIndex)
{
	if (!m_pVideoAccel)
		return E_ABORT;

	CAutoLock lock(&m_csObj);
	HRESULT hr = m_pVideoAccel->ReleaseBuffer(dwType, dwIndex);

	return hr;
}
STDMETHODIMP CHVDServiceDxva1::BeginFrame(DWORD dwDstSurfIndex)
{
	CAutoLock lock(&m_csObj);
	HRESULT hr = E_FAIL;
	AMVABeginFrameInfo l_beginFrameInfo;
	ZeroMemory(&l_beginFrameInfo, sizeof(l_beginFrameInfo));
	l_beginFrameInfo.dwDestSurfaceIndex	= dwDstSurfIndex;
	l_beginFrameInfo.pInputData	= &dwDstSurfIndex;
	l_beginFrameInfo.dwSizeInputData	= 2;

	for (int cnAttempts = 20; cnAttempts > 0; cnAttempts--)
	{
		hr = m_pVideoAccel->BeginFrame(&l_beginFrameInfo);
		if (hr == E_PENDING || hr == DDERR_WASSTILLDRAWING)
			Sleep(2);
		else
			break;
	}
	return hr;
}

STDMETHODIMP CHVDServiceDxva1::Execute(HVDDxvaExecuteConfig* pExecuteConfig)
{
	CHECK_POINTER(pExecuteConfig);

	if (!m_pVideoAccel)
		return E_ABORT;

	CAutoLock lock(&m_csObj);
	HRESULT hr = m_pVideoAccel->Execute(pExecuteConfig->dwFunction, pExecuteConfig->lpPrivateInputData, 
		pExecuteConfig->cbPrivateInputData, pExecuteConfig->lpPrivateOutputData, 
		pExecuteConfig->cbPrivateOutputData, pExecuteConfig->dwNumBuffers, ((AMVABUFFERINFO *)pExecuteConfig->lpAmvaBufferInfo));	
	
	return hr;
}

STDMETHODIMP CHVDServiceDxva1::RunDXVAGraph()
{
	CAutoLock lock(&m_csObj);
	HRESULT hr = E_FAIL;

	if (m_bYUVMixing)
		hr = SetMixingPref(MixerPref9_RenderTargetYUV | MixerPref9_NonSquareMixing);

	if (!m_pVideoAccel)
	{
		return E_FAIL;
	}

	hr = InitializeDXVAConfig();
	if (FAILED(hr))
	{
		DbgLog((LOG_ERROR, HVD_DEFAULT_ERROR_LEVEL,"CHVDServiceDxva1::RunDXVAGraph: InitializeDXVAConfig failed, hr=0x%08x\n", hr));
		return hr;
	}

	GetUncompSurfNum();

	DbgLog((LOG_TRACE, HVD_DEFAULT_TRACE_LEVEL,"CHVDServiceDxva1::RunDXVAGraph: DXVA buffer number: %d\n", m_dwSurfaceCount));

	if (m_bYUVDecimateBy2)
	{
		VIDEOINFOHEADER2 *h = (VIDEOINFOHEADER2 *)m_MediaType.Format();
		if (h->bmiHeader.biWidth > 720)
			hr = SetMixingPref(MixerPref9_DynamicDecimateBy2);
	}

#ifdef _DEBUG
	hr = CheckDeinterlaceMode();
	if (FAILED(hr))
	{
		DbgLog((LOG_ERROR, HVD_DEFAULT_ERROR_LEVEL,"CHVDServiceDxva1::RunDXVAGraph: CheckDeinterlaceMode failed, hr=0x%08x\n", hr));
	}
#endif

	DWORD dwNumFormatsSupported = 1;
	DDPIXELFORMAT ddFormatsSupported;
	hr = m_pVideoAccel->GetUncompFormatsSupported(&m_DecoderGuid, &dwNumFormatsSupported, &ddFormatsSupported);
	if (FAILED(hr))
	{
		DbgLog((LOG_ERROR, HVD_DEFAULT_ERROR_LEVEL,"CHVDServiceDxva1::RunDXVAGraph: GetUncompFormatsSupported failed, hr=0x%08x\n", hr));
		return hr;
	}

	m_dwSurfaceType = ddFormatsSupported.dwFourCC;
	m_bConfigDecoder = TRUE;
	if (m_dwSurfaceCount < m_dwMinSurfaceCount)
	{
		DbgLog((LOG_ERROR, HVD_DEFAULT_ERROR_LEVEL,"CHVDServiceDxva1::RunDXVAGraph: insufficient surfaces"));
		return E_FAIL;
	}

	hr = Run();

	if (FAILED(hr))
	{
		DbgLog((LOG_ERROR, HVD_DEFAULT_ERROR_LEVEL,"CHVDServiceDxva1::RunDXVAGraph: unable to run graph, hr = 0x%08x\n", hr));
		return E_FAIL;
	}
	m_rtStart = 0;
	m_bSyncStreamTime = FALSE;

	return hr;
}

STDMETHODIMP CHVDServiceDxva1::EndFrame(DWORD dwDstSurfIndex)
{
	HRESULT hr = E_FAIL;
	CAutoLock lock(&m_csObj);
	AMVAEndFrameInfo l_endFrameInfo;
	l_endFrameInfo.dwSizeMiscData = 2;
	l_endFrameInfo.pMiscData = &dwDstSurfIndex;
	hr = m_pVideoAccel->EndFrame(&l_endFrameInfo);
	return hr;
}

STDMETHODIMP CHVDServiceDxva1::GetUncompressedBufferCount(DWORD *pdwCount)
{
	CHECK_POINTER(pdwCount);
	*pdwCount = m_dwSurfaceCount;
	return S_OK;
}

STDMETHODIMP CHVDServiceDxva1::GetUncompresesdBufferFormat(DWORD* pdwFourCC)
{
	CHECK_POINTER(pdwFourCC);

	CAutoLock lock(&m_csObj);
	*pdwFourCC = m_dwSurfaceType;
	return S_OK;
}

STDMETHODIMP CHVDServiceDxva1::GetAccel(IUnknown** ppAccel)
{
	CHECK_POINTER(ppAccel);
	if (m_pVideoAccel)
	{
		*ppAccel = m_pVideoAccel;
		(*ppAccel)->AddRef();
		return S_OK;
	}
	return E_FAIL;
}

STDMETHODIMP CHVDServiceDxva1::GetDxvaConfigPictureDecode(HVDDXVAConfigPictureDecode* pDxvaConfigPictureDecode)
{
	// Encryption GUIDs
	pDxvaConfigPictureDecode->guidConfigBitstreamEncryption	 = m_DxvaConfigPictureDecode.guidConfigBitstreamEncryption;
	pDxvaConfigPictureDecode->guidConfigMBcontrolEncryption	 = m_DxvaConfigPictureDecode.guidConfigMBcontrolEncryption; 
	pDxvaConfigPictureDecode->guidConfigResidDiffEncryption	 = m_DxvaConfigPictureDecode.guidConfigResidDiffEncryption; 
	// Bitstream Processing Indicator
	pDxvaConfigPictureDecode->bConfigBitstreamRaw            = m_DxvaConfigPictureDecode.bConfigBitstreamRaw; 
	// Macroblock Control Config
	pDxvaConfigPictureDecode->bConfigMBcontrolRasterOrder		 = m_DxvaConfigPictureDecode.bConfigMBcontrolRasterOrder; 
	// Host Residue Diff Config
	pDxvaConfigPictureDecode->bConfigResidDiffHost           = m_DxvaConfigPictureDecode.bConfigResidDiffHost; 
	pDxvaConfigPictureDecode->bConfigSpatialResid8           = m_DxvaConfigPictureDecode.bConfigSpatialResid8; 
	pDxvaConfigPictureDecode->bConfigResid8Subtraction       = m_DxvaConfigPictureDecode.bConfigResid8Subtraction; 
	pDxvaConfigPictureDecode->bConfigSpatialHost8or9Clipping = m_DxvaConfigPictureDecode.bConfigSpatialHost8or9Clipping; 
	pDxvaConfigPictureDecode->bConfigSpatialResidInterleaved = m_DxvaConfigPictureDecode.bConfigSpatialResidInterleaved; 
	pDxvaConfigPictureDecode->bConfigIntraResidUnsigned      = m_DxvaConfigPictureDecode.bConfigIntraResidUnsigned; 
	// Accelerator Residue Diff Config
	pDxvaConfigPictureDecode->bConfigResidDiffAccelerator		 = m_DxvaConfigPictureDecode.bConfigResidDiffAccelerator; 
	pDxvaConfigPictureDecode->bConfigHostInverseScan         = m_DxvaConfigPictureDecode.bConfigHostInverseScan; 
	pDxvaConfigPictureDecode->bConfigSpecificIDCT            = m_DxvaConfigPictureDecode.bConfigSpecificIDCT; 
	pDxvaConfigPictureDecode->bConfig4GroupedCoefs           = m_DxvaConfigPictureDecode.bConfig4GroupedCoefs; 

	return S_OK;
}

//IHVDServiceDxva1
STDMETHODIMP CHVDServiceDxva1::GetRendererType(DWORD *dwRendererType)
{
	CHECK_POINTER(dwRendererType);
	*dwRendererType = (DWORD)m_dwVideoRendererType;
	return S_OK;
}

STDMETHODIMP CHVDServiceDxva1::GetVideoRenderer(IBaseFilter** ppFilter)
{
	CHECK_POINTER(ppFilter);

	HRESULT hr = S_OK;

	if (!m_pVideoRender)
	{
		hr = CreateVideoRenderer();
		if (FAILED(hr))
		{
			*ppFilter = NULL;
			return hr;
		}
	}

	return m_pVideoRender->QueryInterface(ppFilter);

}

STDMETHODIMP CHVDServiceDxva1::SetDisplaySize(const LPRECT pRect)
{
	CHECK_POINTER(pRect);

	m_dwWidth = pRect->right - pRect->left;
	m_dwHeight = pRect->bottom - pRect->top;
	return S_OK;
}

STDMETHODIMP CHVDServiceDxva1::SetVideoWindow(HWND hWnd)
{
	m_hWnd = hWnd;

	HRESULT hr;
	IVMRWindowlessControl *pWC;
	IVMRWindowlessControl9 *pWC9;

	if (m_dwVideoRendererType == HVD_DXVA1_RENDERER_VMR9CUSTOM || m_dwVideoRendererType == HVD_DXVA1_RENDERER_OVERLAY) {
		return E_UNEXPECTED;
	}

	if (m_hWnd != hWnd)
		m_hWnd = hWnd;

	if(m_dwVideoRendererType == HVD_DXVA1_RENDERER_VMR7)
	{
		hr = m_pVideoRender->QueryInterface(IID_IVMRWindowlessControl, (void**)&pWC);
		if (SUCCEEDED(hr)) 
		{
			pWC->SetVideoClippingWindow(m_hWnd);
			pWC->SetBorderColor(RGB(0,0,0));
		}
		long lWidth, lHeight; 
		hr = pWC->GetNativeVideoSize(&lWidth, &lHeight, NULL, NULL); 

		if (SUCCEEDED(hr))
		{
			RECT rcSrc, rcDest; 
			// Set the source rectangle.
			SetRect(&rcSrc, 0, 0, lWidth, lHeight); 

			// Get the window client area.
			GetClientRect(m_hWnd, &rcDest); 
			// Set the destination rectangle.
			SetRect(&rcDest, 0, 0, rcDest.right, rcDest.bottom); 

			// Set the video position.
			hr = pWC->SetVideoPosition(&rcSrc, &rcDest); 
		}    
		pWC->Release();
	}
	else if(m_dwVideoRendererType == HVD_DXVA1_RENDERER_VMR9WIDNOWLESS)
	{
		hr = m_pVideoRender->QueryInterface(IID_IVMRWindowlessControl9, (void**)&pWC9);

		if (SUCCEEDED(hr)) 
		{
			pWC9->SetVideoClippingWindow(m_hWnd);
			pWC9->SetBorderColor(RGB(0,0,0));
		}
		long lWidth, lHeight; 
		hr = pWC9->GetNativeVideoSize(&lWidth, &lHeight, NULL, NULL); 

		if (SUCCEEDED(hr))
		{
			RECT rcSrc, rcDest; 
			// Set the source rectangle.
			SetRect(&rcSrc, 0, 0, lWidth, lHeight); 

			// Get the window client area.
			GetClientRect(m_hWnd, &rcDest); 

			// Set the destination rectangle.
			SetRect(&rcDest, 0, 0, rcDest.right, rcDest.bottom); 

			// Set the video position.
			hr = pWC9->SetVideoPosition(&rcSrc, &rcDest); 
		}    
		pWC9->Release();
	}
	return S_OK;
}


STDMETHODIMP CHVDServiceDxva1::DisplayFrame(HVDService::HVDDxva1VideoFrame *pFrame, HVDService::HVDDxva1SubFrame *pSub)
{
	if (!m_pVideoAccel)
		return E_FAIL;

	BOOL bDiscontinuity = FALSE, bSyncPoint = TRUE;
	// check the navigation from GUI side
	if (pFrame->dwDecodeStatus == HVD_DXVA1_DECODE_STOP)
	{
		m_rtStart = 0;
		bDiscontinuity = TRUE;
		m_bSyncStreamTime = FALSE;
	}
/*
	HVDDxva1QueryBuffer l_qb;
	l_qb.dwTypeIndex	= 0xffffffff;
	l_qb.dwBufferIndex	= pFrame->dwFrameIndex;
*/
	////new method for H264
	//if(f->dwVideoType == VIDEO_H264)
	//	f->getH264displaystatus_fcn(f);

/*
	HRESULT hr = QueryStatus(&l_qb, 1, HVA_QUERY_READ);
	if (FAILED(hr))
		return hr;
*/
	HRESULT hr = E_FAIL;
	for (int nQuery = 0; nQuery < 20; nQuery++)
	{
		hr = QueryRenderStatus(0xffffffff, pFrame->dwFrameIndex, AMVA_QUERYRENDERSTATUSF_READ);
		if (hr != S_OK && hr != E_NOTIMPL)
		{
			DbgLog((LOG_TRACE, HVD_DEFAULT_TRACE_LEVEL,"QuaryStatus: sleep 2ms Count=%d\n", nQuery));
			Sleep(2);
		}
		else
			break;
	}
	if (FAILED(hr))
		return hr;

	// Acquire a deliver buffer
	IMediaSample *pSample;
	if (m_pBinder)
	{
		hr = m_pBinder->GetOutputPin(0)->GetDeliveryBuffer(&pSample, NULL, NULL, 0);
		if (FAILED(hr) || !pSample)
		{
			return E_FAIL;
		}
	}
	else if (m_dwInitFlags & HVD_INIT_BASEFILTER)
	{
		CHVDDSHelper helper(m_pGraph);
		IPin *pVmrInput = NULL;
		IMemAllocator *pMemAllocator = NULL;
		hr = helper.FindPin(m_pVideoRender, &pVmrInput, PINDIR_INPUT);
		if (SUCCEEDED(hr) && pVmrInput)
		{
			hr = pVmrInput->QueryInterface(&pMemAllocator);
			if (SUCCEEDED(hr) && pMemAllocator)
			{
				pMemAllocator->GetBuffer(&pSample, NULL, NULL, 0);
				if (FAILED(hr) || !pSample)
				{
					return E_FAIL;
				}
			}
		}
	}

	if(m_bInterlaceSample)
	{
		//		pFrame->sample.msp = pSample;
		pFrame->pMediaSample = pSample;
		DSUpdateOutputSample(pFrame);
	}

	if (pFrame->dwFrameRate)
		m_rtAvgTime = LONGLONG(10000000000.0 / pFrame->dwFrameRate);
	else
		m_rtAvgTime = NULL;
	//if (m_bSubPicPin)
	//	BlendSpic(f, pSub);

	if (pFrame->bNeedSyncStreamTime)
		m_bSyncStreamTime = FALSE;

	if (!m_bSyncStreamTime)
	{
		//Sync. to filter StreamTime.
		REFERENCE_TIME rtGraphStartTime;
		hr = m_pBinder->GetStartTime(&rtGraphStartTime);
		if (SUCCEEDED (hr))
		{
			REFERENCE_TIME rtSystemTime, rtStreamTime;
			hr = m_pClock->GetTime(&rtSystemTime);
			if (SUCCEEDED(hr))
			{
				rtStreamTime = rtSystemTime - rtGraphStartTime;
				//ASSERT(rtStreamTime > 0);
				if (m_rtStart > rtStreamTime + (DXVA_REFRESH_TIME<<1) || m_rtStart < rtStreamTime)
				{
#ifdef _DEBUG
					DbgLog((LOG_ERROR, HVD_DEFAULT_ERROR_LEVEL,"CHVDServiceDxva1::DisplayFrame, m_rtStart=%I64d, rtStreamTime=%I64d, diff=%I64d\n", m_rtStart, rtStreamTime, rtStreamTime-m_rtStart));
#endif
					m_rtStart = rtStreamTime + DXVA_REFRESH_TIME;
					bDiscontinuity = TRUE;
				}
				m_bSyncStreamTime = TRUE;
			}
		}
	}

	REFERENCE_TIME rtEnd;
	if (pFrame->bRefresh)
	{
		rtEnd = m_rtStart + 1;
		bSyncPoint = FALSE;
		bDiscontinuity = TRUE;
	}
	else if (pFrame->bNormalPlayback)
	{
		rtEnd = m_rtStart + m_rtAvgTime;
		if (pFrame->dwRepeatFirstField)
			rtEnd += (m_rtAvgTime>>1);
	}
	else //f->speed != GPI_SPEED_NORMAL, FF or FR playback mode.
	{
		rtEnd = m_rtStart + 1;
		bDiscontinuity = TRUE;
	}
	
	hr = pSample->SetTime(&m_rtStart, &rtEnd);
	hr = pSample->SetSyncPoint(bSyncPoint);
	hr = pSample->SetDiscontinuity(bDiscontinuity);

	DWORD dwTime = NULL;

#ifdef _DEBUG
	DebugPrintMediaSample(pSample);
	DbgLog((LOG_TRACE, HVD_DEFAULT_TRACE_LEVEL,"before m_pVideoAccel->DisplayFrame[%2d], rt %I64d -> %I64d, diff: %d\n", pFrame->dwFrameIndex,
		m_rtStart, rtEnd, rtEnd-m_rtStart));
	dwTime = timeGetTime();
#endif

	hr = m_pVideoAccel->DisplayFrame(pFrame->dwFrameIndex, pSample);


	DbgLog((LOG_TRACE, HVD_DEFAULT_TRACE_LEVEL,"after m_pVideoAccel->DisplayFrame, took %d\n", timeGetTime() - dwTime));

	if (!pFrame->bRefresh)
		m_rtStart += (pFrame->dwRepeatFirstField) ? (m_rtAvgTime + (m_rtAvgTime>>1)) : m_rtAvgTime;

	pSample->Release();

	return hr;
}

STDMETHODIMP CHVDServiceDxva1::Repaint(HDC hdc, LPRECT pRect)
{
	HRESULT hr;
	IVMRWindowlessControl *pWC;
	IVMRWindowlessControl9 *pWC9;

	if (m_dwVideoRendererType == HVD_DXVA1_RENDERER_VMR9CUSTOM ||(m_pVideoRender == NULL) || m_dwVideoRendererType == HVD_DXVA1_RENDERER_OVERLAY) {
		return E_UNEXPECTED;
	}

	if(m_dwVideoRendererType == HVD_DXVA1_RENDERER_VMR7)
	{
		hr = m_pVideoRender->QueryInterface(IID_IVMRWindowlessControl, (void**)&pWC);
		if (SUCCEEDED(hr) && pWC)
		{
			hr = pWC->RepaintVideo(m_hWnd, hdc);  
			pWC->Release();
		}
	}
	else if(m_dwVideoRendererType == HVD_DXVA1_RENDERER_VMR9WIDNOWLESS)
	{
		hr = m_pVideoRender->QueryInterface(IID_IVMRWindowlessControl9, (void**)&pWC9);
		if (SUCCEEDED(hr) && pWC9)
		{
			hr = pWC9->RepaintVideo(m_hWnd, hdc);  
			pWC9->Release();
		}
	}
	return 0;
}

//Internal Function
HRESULT CHVDServiceDxva1::CreateGraphBuilder()
{
	HRESULT hr = E_FAIL;

	if (!m_pGraph)
	{
		//		IGraphBuilder *pGB;
		hr = CoCreateInstance(CLSID_FilterGraph,NULL,CLSCTX_INPROC,IID_IGraphBuilder,(LPVOID *)&m_pGraph);
		//hr = pGB.CoCreateInstance(CLSID_FilterGraph);
		if (FAILED(hr))
		{
			DbgLog((LOG_ERROR, HVD_DEFAULT_ERROR_LEVEL,"CHVDServiceDxva1::CreateGraphBuilder: unable to create filter graph, hr=0x%08x\n", hr));
			return hr;
		}
		//m_pGraph->AddRef();
	}
	return S_OK;
	//return hr;
}

HRESULT CHVDServiceDxva1::CreateVideoRenderer()
{
	if (m_pVideoRender)
		return S_FALSE;

	HRESULT hr = E_FAIL;
	//IBaseFilter *pVideoRender = NULL;
	IVMRWindowlessControl *pWC = NULL;
	IVMRWindowlessControl9 *pWC9 = NULL;
	WCHAR wVideoRenderName[50];


	if (m_dwVideoRendererType == HVD_DXVA1_RENDERER_VMR7)
	{
		hr = CoCreateInstance(CLSID_VideoMixingRenderer, NULL, CLSCTX_INPROC, IID_IBaseFilter, (void**)&m_pVideoRender); 
		wcscpy_s(wVideoRenderName, L"Video Mixing Renderer");
	}
	else if (m_dwVideoRendererType == HVD_DXVA1_RENDERER_VMR9CUSTOM || m_dwVideoRendererType == HVD_DXVA1_RENDERER_VMR9WIDNOWLESS)
	{
		hr = CoCreateInstance(CLSID_VideoMixingRenderer9, NULL, CLSCTX_INPROC, IID_IBaseFilter, (void**)&m_pVideoRender); 
		wcscpy_s(wVideoRenderName, L"Video Mixing Renderer 9");
	}
	else
	{
		hr = E_FAIL;
	}

	if (FAILED(hr))
	{
		DbgLog((LOG_ERROR, HVD_DEFAULT_ERROR_LEVEL,"CHVDServiceDxva1::CreateVideoRenderer: unable to create video renderer (type = %d), hr=0x%08x\n", m_dwVideoRendererType, hr));
		return hr;
	}

	hr = CreateGraphBuilder();
	if (FAILED(hr))
	{
		DbgLog((LOG_ERROR, HVD_DEFAULT_ERROR_LEVEL,"CHVDServiceDxva1::CreateVideoRenderer: unable to get filter graph, hr=0x%08x\n", hr));
		return hr;
	}

	// It is necessary to add this VMR to filter graph.
	hr = m_pGraph->AddFilter(m_pVideoRender, wVideoRenderName);
	if (FAILED(hr))
	{
		DbgLog((LOG_ERROR, HVD_DEFAULT_ERROR_LEVEL,"CHVDServiceDxva1::CreateVideoRenderer: unable to add VMR9 into filter graph, hr=0x%08x\n", hr));
		return hr;
	}

	if (m_dwVideoRendererType == HVD_DXVA1_RENDERER_VMR7)
	{
		// Set the rendering mode and number of streams.  
		IVMRFilterConfig* pConfig;

		hr = m_pVideoRender->QueryInterface(IID_IVMRFilterConfig, (void**)&pConfig);
		if (SUCCEEDED(hr)) 
		{
			pConfig->SetRenderingMode(VMRMode_Windowless);
			if (m_bSubPicPin)	// disable sub-picture pin, only need 1 pin in VMR
				pConfig->SetNumberOfStreams(2);
			pConfig->Release();
		}

		hr = m_pVideoRender->QueryInterface(IID_IVMRWindowlessControl, (void**)&pWC);
		if (SUCCEEDED(hr)) 
		{
			pWC->SetVideoClippingWindow(m_hWnd);
			pWC->SetBorderColor(RGB(0,0,0));
		}
		long lWidth, lHeight; 
		hr = pWC->GetNativeVideoSize(&lWidth, &lHeight, NULL, NULL); 
		if (SUCCEEDED(hr))
		{
			RECT rcSrc, rcDest; 
			// Set the source rectangle.
			SetRect(&rcSrc, 0, 0, lWidth, lHeight); 

			// Get the window client area.
			GetClientRect(m_hWnd, &rcDest); 
			// Set the destination rectangle.
			SetRect(&rcDest, 0, 0, rcDest.right, rcDest.bottom); 

			// Set the video position.
			hr = pWC->SetVideoPosition(&rcSrc, &rcDest); 
		}
		pWC->Release();
	}
	else if(m_dwVideoRendererType == HVD_DXVA1_RENDERER_VMR9WIDNOWLESS)
	{
		// Set the rendering mode and number of streams.  
		IVMRFilterConfig9* pConfig9;

		hr = m_pVideoRender->QueryInterface(IID_IVMRFilterConfig9, (void**)&pConfig9);
		if (SUCCEEDED(hr)) 
		{
			pConfig9->SetRenderingMode(VMRMode_Windowless);
			if (m_bSubPicPin)	// disable sub-picture pin, only need 1 pin in VMR
				pConfig9->SetNumberOfStreams(2);
			pConfig9->Release();
		}

		hr = m_pVideoRender->QueryInterface(IID_IVMRWindowlessControl9, (void**)&pWC9);
		if (SUCCEEDED(hr)) 
		{
			pWC9->SetVideoClippingWindow(m_hWnd);
			pWC9->SetBorderColor(RGB(0,0,0));
		}
		long lWidth, lHeight; 
		hr = pWC9->GetNativeVideoSize(&lWidth, &lHeight, NULL, NULL); 
		if (SUCCEEDED(hr))
		{
			RECT rcSrc, rcDest; 
			// Set the source rectangle.
			SetRect(&rcSrc, 0, 0, lWidth, lHeight); 

			// Get the window client area.
			GetClientRect(m_hWnd, &rcDest); 
			// Set the destination rectangle.
			SetRect(&rcDest, 0, 0, rcDest.right, rcDest.bottom); 

			// Set the video position.
			hr = pWC9->SetVideoPosition(&rcSrc, &rcDest); 
		}
		pWC9->Release();
	}
	else if (m_dwVideoRendererType == HVD_DXVA1_RENDERER_VMR9CUSTOM)
	{
		IVMRFilterConfig9* pConfig9;
		hr = m_pVideoRender->QueryInterface(&pConfig9);
		if (FAILED(hr))
			return hr;
		if (m_bSubPicPin)	// disable sub-picture pin, only need 1 pin in VMR
			hr = pConfig9->SetNumberOfStreams(2);
		//Set Renderless mode
		hr = pConfig9->SetRenderingMode(VMRMode_Renderless);

		pConfig9->Release();
	}

	m_rtStart = 0;
	m_bSyncStreamTime = FALSE;
	return S_OK;
}

HRESULT CHVDServiceDxva1::DestroyGraph()
{
	if (m_dwRotReg != 0)
	{
		HVDService::RemoveFromROT(m_dwRotReg);
		m_dwRotReg = 0;
	}
	DestroyBinder();

	if (m_pVideoRender && m_pGraph)
	{
		m_pGraph->RemoveFilter(m_pVideoRender);
	}

	SAFE_RELEASE(m_pClock);
	SAFE_RELEASE(m_pGraph);
	SAFE_RELEASE(m_pVideoRender);
	return S_OK;
}

HRESULT CHVDServiceDxva1::ConnectGraph()
{
	HRESULT hr = E_FAIL;
	BOOL bReset_dxva_buffer = FALSE;

	if (!m_pGraph)
		return hr;

	CHVDDSHelper helper(m_pGraph);

	int count = 0;
	// <KLUDGE> Sometimes we need to try multiple times to connect two filters...
	do
	{
		hr = helper.ConnectPins(m_pBinder, VIDEO_VIDOUT_LNAME, m_pVideoRender);		
		if (SUCCEEDED(hr)) {
			if (m_bSubPicPin) {
				for (int i=0; i<10; i++) {
					hr = helper.ConnectPins(m_pBinder, VIDEO_SUBOUT_LNAME, m_pVideoRender);
					if (SUCCEEDED(hr))
						break;
					Sleep(100);
				}
			}
			break;
		}
		else if (hr == VFW_E_VMR_NO_AP_SUPPLIED)
		{
			DbgLog((LOG_ERROR, HVD_DEFAULT_ERROR_LEVEL,"CHVDServiceDxva1::ConnectGraph: unable to connect pins due to no custom allocator presenter, hr=0x%08x(VFW_E_NO_ACCEPTABLE_TYPES)\n", hr));
			return hr;
		}
		Sleep(100);
		count ++;

		//some VGA cards could not allocate 15 buffers even the video memory is large than 192 MB, lower to 8 buffers
		if(count == 5 && bReset_dxva_buffer == FALSE /*&& m_dwVideoMemorySize > MEMORY_THRESHLOD*/)
		{
			count = 0;
			m_UncompSurfacesConfig.dwMaxSurfaceCount = DXVA_BUF_128MB;
			m_UncompSurfacesConfig.dwMinSurfaceCount = DXVA_BUF_128MB;
			m_pBinder->SetUncompSurfacesConfig(&m_UncompSurfacesConfig);
			bReset_dxva_buffer = TRUE;
		}
	}
	while(count < 5);

	if(hr == VFW_E_NO_ACCEPTABLE_TYPES /*DIERR_NOTBUFFERED*/)
	{
		DbgLog((LOG_ERROR, HVD_DEFAULT_ERROR_LEVEL,"CHVDServiceDxva1::ConnectGraph: unable to connect pins due to no acceptable type, hr=0x%08x(VFW_E_NO_ACCEPTABLE_TYPES)\n", hr));
		return hr;
	}

	if (FAILED(hr))
	{
		DbgLog((LOG_ERROR, HVD_DEFAULT_ERROR_LEVEL,"CHVDServiceDxva1::ConnectGraph: unable to connect VMR binder and VMR9, hr=0x%08x\n", hr));
		return hr;
	}

	hr = UpdateAMVideoAccelerator();
	if (FAILED(hr))
	{
		DbgLog((LOG_ERROR, HVD_DEFAULT_ERROR_LEVEL,"CHVDServiceDxva1::ConnectGraph: unable to update AMVideoAccelerator, hr=0x%08x\n", hr));
		return hr;
	}
	return hr;
}

HRESULT CHVDServiceDxva1::DisconnectGraph()
{
	Stop();

	FreeMediaType(m_MediaType);

	if (m_pVideoAccel)
	{
		m_pVideoAccel->Release();
		m_pVideoAccel = NULL;
	}
	if (m_pBinder)
	{
		// For safety, disconnect the connection between Binder and VMR manually.
		CHVDDSHelper helper(m_pGraph);
		helper.DisconnectAllPins(m_pBinder);
	}

	return S_OK;
}

HRESULT CHVDServiceDxva1::DestroyBinder()
{
	Stop();

	if (m_pBinder)
	{
		// For safety, disconnect the connection between Binder and VMR manually.
		CHVDDSHelper helper(m_pGraph);
		helper.DisconnectAllPins(m_pBinder);
		m_pGraph->RemoveFilter(m_pBinder);
		m_pBinder->Destroy();
		m_pBinder->Release();
		m_pBinder = NULL;
	}
//	m_pVideoAccel->Release();
	SAFE_RELEASE(m_pVideoAccel);

	return S_OK;
}

HRESULT CHVDServiceDxva1::UpdateAMVideoAccelerator()
{
	HRESULT hr;

	CHVDDSHelper helper(m_pGraph);
	IPin *pVmrInput;
	hr = helper.FindPin(m_pVideoRender, &pVmrInput, PINDIR_INPUT);
	if (FAILED(hr))
	{
		DbgLog((LOG_ERROR, HVD_DEFAULT_ERROR_LEVEL,"CHVDServiceDxva1::UpdateAMVideoAccelerator: unable to find video input pin of VMR, hr=0x%08x\n", hr));
		return hr;
	}

	if (m_pBinder)
	{
		for (int i = 0; i < 5; i++)
		{
			// main video
			FreeMediaType(m_MediaType);
			hr = m_pBinder->GetOutputPin(0)->ConnectionMediaType(&m_MediaType);
			if (SUCCEEDED(hr))
			{
				break;
			}
			else
			{
				DbgLog((LOG_ERROR, HVD_DEFAULT_ERROR_LEVEL,"CHVDServiceDxva1::UpdateAMVideoAccelerator: fail to get connection media type, retry later, hr=0x%08x\n", hr));
			}
			Sleep(100);
		}
	}
	else if ((m_dwInitFlags & HVD_INIT_BASEFILTER) && (pVmrInput))
	{
		IPin *pVideoOutputPin;
		hr = pVmrInput->ConnectedTo(&pVideoOutputPin);
		if (SUCCEEDED(hr) && pVideoOutputPin)
		{
			for (int i = 0; i < 5; i++)
			{
				// main video
				FreeMediaType(m_MediaType);
				hr = pVideoOutputPin->ConnectionMediaType(&m_MediaType);
				if (SUCCEEDED(hr))
				{
					if (m_MediaType.formattype == FORMAT_VideoInfo)
					{
						VIDEOINFOHEADER* pVideoInfo = (VIDEOINFOHEADER*)m_MediaType.pbFormat;

						m_HVDDecodeConfig.dwWidth = pVideoInfo->bmiHeader.biWidth;
						m_HVDDecodeConfig.dwHeight = pVideoInfo->bmiHeader.biHeight;
					}
					else if (m_MediaType.formattype == FORMAT_VideoInfo2)
					{
						VIDEOINFOHEADER2* pVideoInfo = (VIDEOINFOHEADER2*)m_MediaType.pbFormat;

						m_HVDDecodeConfig.dwWidth = pVideoInfo->bmiHeader.biWidth;
						m_HVDDecodeConfig.dwHeight = pVideoInfo->bmiHeader.biHeight;
					}

					break;
				}
				else
				{
					DbgLog((LOG_ERROR, HVD_DEFAULT_ERROR_LEVEL,"CHVDServiceDxva1::UpdateAMVideoAccelerator: fail to get connection media type, retry later, hr=0x%08x\n", hr));
				}
				Sleep(100);
			}
			pVideoOutputPin->Release();
			pVideoOutputPin = NULL;
		}
	}
	else
		hr = E_FAIL;

	if (FAILED(hr))
	{
		DbgLog((LOG_ERROR, HVD_DEFAULT_ERROR_LEVEL,"CHVDServiceDxva1::UpdateAMVideoAccelerator: No connection media type is available, hr=0x%08x\n", hr));
		m_DecoderGuid = GUID_NULL;
		SAFE_RELEASE(m_pVideoAccel);
		SAFE_RELEASE(pVmrInput);
		return hr;
	}
	else
	{
		// Remember the connection type for later use.
		m_DecoderGuid = m_MediaType.subtype;
		// Get the IAMVideoAccelerator!
		pVmrInput->QueryInterface(&m_pVideoAccel);
		if (m_pVideoAccel)
		{
			if SUCCEEDED(m_pVideoAccel->GetVideoAcceleratorGUIDs( &m_dwSupportedDecoderCount, NULL) && m_dwSupportedDecoderCount > 0)
			{
				SAFE_DELETE_ARRAY(m_pSupportedDecoderGuids);
				m_pSupportedDecoderGuids = new GUID[m_dwSupportedDecoderCount];
				m_pVideoAccel->GetVideoAcceleratorGUIDs( &m_dwSupportedDecoderCount, m_pSupportedDecoderGuids);
			}
		}
		SAFE_RELEASE(pVmrInput);
	}
	return hr;
}

HRESULT CHVDServiceDxva1::CheckDeinterlaceMode()
{
	// if not FORMAT_VideoInfo2, we can't set deinterlace mode.
	if (m_MediaType.formattype != FORMAT_VideoInfo2) 
		return E_FAIL;

	IVMRDeinterlaceControl9 *pDIC9;
	VIDEOINFOHEADER2 *h = (VIDEOINFOHEADER2*)m_MediaType.pbFormat;
	VMR9VideoDesc vd;
	DWORD nMode;
	HRESULT hr = m_pVideoRender->QueryInterface(__uuidof(IVMRDeinterlaceControl9), (void**) &pDIC9);
	if(SUCCEEDED(hr))
	{
		VMR9DeinterlaceCaps Caps;

		// if not filled with size, GetDeinterlaceModeCaps returns E_INVALIDARG
		Caps.dwSize = sizeof(VMR9DeinterlaceCaps);

		vd.dwSize = sizeof(VMR9VideoDesc);
		vd.dwSampleWidth = h->bmiHeader.biWidth;
		vd.dwSampleHeight = h->bmiHeader.biHeight;
		vd.SampleFormat = VMR9_SampleFieldInterleavedEvenFirst;
		vd.dwFourCC = h->bmiHeader.biCompression;
		vd.InputSampleFreq.dwNumerator = 30000;
		vd.InputSampleFreq.dwDenominator = 1001;
		vd.OutputFrameFreq.dwNumerator = 60000;
		vd.OutputFrameFreq.dwDenominator = 1001;

		hr = pDIC9->GetNumberOfDeinterlaceModes(&vd, &nMode, NULL);
		// if more than one mode to choose from
		if(SUCCEEDED(hr) && nMode > 1)
		{
			GUID *pGuid = new GUID[nMode];

			hr = pDIC9->GetNumberOfDeinterlaceModes(&vd, &nMode, pGuid);
			if(SUCCEEDED(hr))
			{
				GUID *pNormalBobGuid = NULL;

				for (DWORD i = 0; i < nMode; i++)
				{
					hr = pDIC9->GetDeinterlaceModeCaps(pGuid + i, &vd, &Caps);

					if (SUCCEEDED(hr) &&
						(Caps.DeinterlaceTechnology == DeinterlaceTech9_BOBVerticalStretch
						|| Caps.DeinterlaceTechnology == DeinterlaceTech9_BOBLineReplicate)
						)
					{
						pNormalBobGuid = pGuid + i;
						break;
					}
				}
			}
			delete [] pGuid;
		}

		GUID actual;
		hr = pDIC9->GetActualDeinterlaceMode(0, &actual);
		hr = pDIC9->GetDeinterlaceModeCaps(&actual, &vd, &Caps);
		hr = pDIC9->GetDeinterlaceMode(0, &actual);
		hr = pDIC9->GetDeinterlaceModeCaps(&actual, &vd, &Caps);
		pDIC9->Release();
	}
	return S_OK;
}


HRESULT CHVDServiceDxva1::DSUpdateOutputSample(HVDService::HVDDxva1VideoFrame *pFrame)
{
	//	CGPIsample *psample = &f->sample;
	CHECK_POINTER(pFrame);

	BOOL bMs2InterlaceInfoNotSent = TRUE;
	HRESULT hr;

	//the time stamp is supposed to be valid
	//	if(psample->pts.IsValid())
	//	{

	if (m_bInterlaceSample) // && m_dwYuzu==0)
	{
		AM_SAMPLE2_PROPERTIES amProp;
		IMediaSample2 *i_ms2;

		hr = pFrame->pMediaSample->QueryInterface(IID_IMediaSample2, (void **)&i_ms2);
		if(SUCCEEDED(hr) && SUCCEEDED(i_ms2->GetProperties(sizeof(amProp), (BYTE *)&amProp)))
		{
			// Assume it's interlace frame first.
			amProp.dwTypeSpecificFlags = AM_VIDEO_FLAG_INTERLEAVED_FRAME;

			if (pFrame->dwRepeatFirstField)
				amProp.dwTypeSpecificFlags |= AM_VIDEO_FLAG_REPEAT_FIELD;

			if (pFrame->dwMediaSampleProperty & GPI_SAMPLE_FIELD1FIRST)
				amProp.dwTypeSpecificFlags |= AM_VIDEO_FLAG_FIELD1FIRST;
			else if (pFrame->dwMediaSampleProperty & GPI_SAMPLE_FIELD2FIRST)
				amProp.dwTypeSpecificFlags |= 0;
			else //progressive
				amProp.dwTypeSpecificFlags = AM_VIDEO_FLAG_WEAVE;

			bMs2InterlaceInfoNotSent = FAILED(i_ms2->SetProperties(sizeof(amProp), (BYTE *)&amProp));
			i_ms2->Release();
		}
	}

	if(m_MediaType.formattype == FORMAT_VideoInfo2
		&& m_MediaType.cbFormat >= sizeof(VIDEOINFOHEADER2))
	{
		VIDEOINFOHEADER2 *pvih2 = (VIDEOINFOHEADER2 *) m_MediaType.pbFormat;
		DWORD dwInterlaceFlags = pvih2->dwInterlaceFlags;
		REFERENCE_TIME rtAvgTime; 		//= f->GetFrameDuration10000();
		if (pFrame->dwFrameRate)
			rtAvgTime = LONGLONG(10000000000.0 / pFrame->dwFrameRate);
		else
			rtAvgTime = NULL;
		RECT rcSource, rcTarget;
		DWORD uDispWidth = pFrame->bDecodeHalfWidth ? pFrame->nHorizontalSize/2 : pFrame->nHorizontalSize;
		DWORD uDispHeight = pFrame->bDecodeHalfHeight ? pFrame->nVerticalSize/2 : pFrame->nVerticalSize;

		rcSource.left = 0;
		rcSource.top  = 0;
		rcSource.right = rcSource.left + pFrame->nHorizontalSize;
		rcSource.bottom = rcSource.top + pFrame->nVerticalSize;

		rcTarget.left = 0;
		rcTarget.top  = 0;
		rcTarget.right = rcTarget.left + uDispWidth;
		rcTarget.bottom = rcTarget.top + uDispHeight;

		if (m_bFIXFPS24Enabled)
			rtAvgTime = 400000;

		if (bMs2InterlaceInfoNotSent)
		{
			if (pFrame->dwMediaSampleProperty & (GPI_SAMPLE_FIELD1FIRST|GPI_SAMPLE_FIELD2FIRST))
			{
				dwInterlaceFlags = AMINTERLACE_IsInterlaced|AMINTERLACE_DisplayModeBobOrWeave;

				if (pFrame->dwMediaSampleProperty & GPI_SAMPLE_FIELD1FIRST)
					dwInterlaceFlags |= AMINTERLACE_Field1First;
			}
			else
			{
				if (dwInterlaceFlags & AMINTERLACE_IsInterlaced)
					dwInterlaceFlags &= ~AMINTERLACE_IsInterlaced;
			}
		}

		if (dwInterlaceFlags != pvih2->dwInterlaceFlags
			|| pFrame->dwPictAspectRatioX != pvih2->dwPictAspectRatioX
			|| pFrame->dwPictAspectRatioY != pvih2->dwPictAspectRatioY
			|| rtAvgTime != pvih2->AvgTimePerFrame
			|| memcmp(&rcSource, &pvih2->rcSource, sizeof(RECT))
			|| memcmp(&rcTarget, &pvih2->rcTarget, sizeof(RECT)))
		{
			pvih2->dwPictAspectRatioX = pFrame->dwPictAspectRatioX;
			pvih2->dwPictAspectRatioY = pFrame->dwPictAspectRatioY;
			pvih2->dwInterlaceFlags = dwInterlaceFlags;
			pvih2->AvgTimePerFrame = rtAvgTime;
			pvih2->rcSource = rcSource;
			pvih2->rcTarget = rcTarget;
			m_rtAvgTime = rtAvgTime;
			hr = pFrame->pMediaSample->SetMediaType(&m_MediaType);
		}

#ifdef _DEBUG
		AM_MEDIA_TYPE *type;
		hr = pFrame->pMediaSample->GetMediaType(&type);
		if (hr == S_OK)
		{
			VIDEOINFOHEADER2 *vh = (VIDEOINFOHEADER2 *)type->pbFormat;
			DeleteMediaType(type);
		}
#endif	
	}
	//	}

	return S_OK;
}

HRESULT CHVDServiceDxva1::SetMixingPref(DWORD dwPref, BOOL bBehavior)
{
	ASSERT(m_pVideoRender);
	HRESULT hr = E_FAIL;
	IVMRMixerControl9 *pMixerControl9;
	m_pVideoRender->QueryInterface(&pMixerControl9);
	if (pMixerControl9)
	{
		DWORD dwMixerPrefs = 0;
		hr = pMixerControl9->GetMixingPrefs(&dwMixerPrefs);
		if (SUCCEEDED(hr))
		{
			if (MixerPref9_RenderTargetMask & dwPref)
			{
				dwMixerPrefs &= ~MixerPref9_RenderTargetMask;
			}
			if (bBehavior)
				dwMixerPrefs |= dwPref;
			else
				dwMixerPrefs &= ~dwPref;

			hr = pMixerControl9->SetMixingPrefs(dwMixerPrefs);
			if (FAILED(hr))
			{
				//				DbgLog((LOG_ERROR, HVD_DEFAULT_ERROR_LEVEL,"CHVDServiceDxva1::SetMixingPref failed. pref = 0x%x, hr =0x%x\n", dwMixerPrefs, hr));
			}
		}
		pMixerControl9->Release();
	}
	return hr;
}

HRESULT CHVDServiceDxva1::Run()
{
	HRESULT hr = E_FAIL;
	IMediaControl *pMC = NULL;

	if (!m_pGraph)
		return S_FALSE;

	hr	= m_pGraph->QueryInterface(&pMC);

	if (!pMC)
	{
		return VFW_E_WRONG_STATE;
	}
	hr = pMC->Run();
	pMC->Release();

	return hr;
}

HRESULT CHVDServiceDxva1::Stop()
{
	HRESULT hr = E_FAIL;
	IVideoWindow *pVW = NULL;

	if (!m_pGraph)
		return S_FALSE;

	hr = m_pGraph->QueryInterface(&pVW);
	if (pVW) 
	{
		pVW->put_Owner(NULL);
		pVW->put_Visible(FALSE);
	}
	pVW->Release();

	IMediaControl *pMC = NULL;
	hr	= m_pGraph->QueryInterface(&pMC);

	if (!pMC)
	{
		return VFW_E_WRONG_STATE;
	}

	hr = pMC->Stop();
	pMC->Release();

	return hr;
}

DWORD CHVDServiceDxva1::GetUncompSurfNum()
{
	DWORD value = 0;
	IiviVideoAcceleratorNotify *pNotify = 0;

	if ((m_dwInitFlags & HVD_INIT_BASEFILTER) && m_pVideoRender)
	{
		CHVDDSHelper helper(m_pGraph);
		IPin *pVmrInput = NULL;
		IPin *pVideoOutputPin = NULL;
		HRESULT hr = E_FAIL;
		hr = helper.FindPin(m_pVideoRender, &pVmrInput, PINDIR_INPUT, NULL, FALSE ,Pin_Connected);
		if (SUCCEEDED(hr) && pVmrInput)
		{
			hr = pVmrInput->ConnectedTo(&pVideoOutputPin);
			if (SUCCEEDED(hr) && pVideoOutputPin)
			{
				hr = pVideoOutputPin->QueryInterface(IID_IiviVideoAcceleratorNotify,(void **)&pNotify);
				if (SUCCEEDED(hr) && pNotify)
				{
					pNotify->GetUncompSurfacesAllocated(&value);
					pNotify->Release();
					m_dwSurfaceCount=value;
				}
				pVideoOutputPin->Release();
			}
			pVmrInput->Release();
		}
		return m_dwSurfaceCount;
	}
	else if (!m_pBinder)
		return m_dwSurfaceCount;	// not built yet, return default value.

	if (m_pBinder->GetOutputPin(0)==0 || FAILED(m_pBinder->GetOutputPin(0)->QueryInterface(IID_IiviVideoAcceleratorNotify,(void **)&pNotify)))
		return 0;

	pNotify->GetUncompSurfacesAllocated(&value);
	pNotify->Release();
	m_dwSurfaceCount=value;
	return value;
}

BOOL CHVDServiceDxva1::GetRecommendedTextureSize(DWORD *pWidth, DWORD *pHeight)
{
	DWORD width = *pWidth, height = *pHeight;

	if (m_dwWidth < width / 2)
		*pWidth >>= 1;
	if (m_dwHeight < height / 2)
		*pHeight >>= 1;

	return width != *pWidth || height != *pHeight;
}

int CHVDServiceDxva1::InitializeDXVAConfig()
{
	int	retVal;
	BOOL bUseSpicPin = TRUE;
	DWORD dwNumTypesCompBuffers = NUM_COMP_BUFFER_TYPE;

	ZeroMemory(&m_DxvaConfigAlphaLoad, sizeof(m_DxvaConfigAlphaLoad));
	ZeroMemory(&m_DxvaConfigAlphaCombine, sizeof(m_DxvaConfigAlphaCombine));
	retVal = InitializeDXVAConfigPicture();
	if (FAILED(retVal))
		return retVal;

	if (m_DecoderGuid == DXVA_ModeMPEG2_B || m_DecoderGuid == DXVA_ModeMPEG2_D || m_DecoderGuid == DXVA_ModeMPEG2_VLD)
	{	// If config fails, go to sub-picture pin!
		if (InitializeDXVAConfigAlphaLoad() == 0 && InitializeDXVAConfigAlphaComb() == 0)
		{	// DXVA spic configuration succeeded
			if (m_DxvaConfigAlphaCombine.bConfigBlendType == 1 || m_dwSurfaceCount > (m_dwMinSurfaceCount))
				bUseSpicPin = FALSE; // enough frame buffers for DXVA front-end sub-picture
		}
		else
			return -1;
	}

//	DWORD dwBlendLast = GetRegInt("DXVABLENDLAST", 2);
	//if (m_dwBlendLast == 2)
	//	m_dwBlendLast = m_DxvaConfigAlphaCombine.bConfigBlendType == 1 && m_dwVendorID != PCI_VENDOR_ID_S3 && m_bFrontBlend != 1;
	//else
	//	m_dwBlendLast = (m_dwBlendLast != 0);
	// Signal decoder to reconfigure
	m_bConfigDecoder = TRUE;
	// Retrieve buffer information
	m_pVideoAccel->GetInternalCompBufferInfo(&dwNumTypesCompBuffers, m_pCompBufferInfo);
	if (m_DecoderGuid == DXVA_ModeMPEG2_B || m_DecoderGuid == DXVA_ModeMPEG2_D || m_DecoderGuid == DXVA_ModeMPEG2_VLD)
	{
		int	idxBuf = 0;
		switch (m_DxvaConfigAlphaLoad.bConfigDataType)
		{
		case DXVA_CONFIG_DATA_TYPE_IA44:
		case DXVA_CONFIG_DATA_TYPE_AI44:
			idxBuf	= DXVA_IA44_SURFACE_BUFFER;
			break;
		case DXVA_CONFIG_DATA_TYPE_DPXD:
			idxBuf	= DXVA_DPXD_SURFACE_BUFFER;
			break;
		case DXVA_CONFIG_DATA_TYPE_AYUV:
			idxBuf	= DXVA_AYUV_BUFFER;
			break;
		}
		if (idxBuf)
		{
			m_dwAlphaBufWidth	= m_pCompBufferInfo[idxBuf].dwWidthToCreate;
			m_dwAlphaBufHeight	= m_pCompBufferInfo[idxBuf].dwHeightToCreate;
			m_dwAlphaBufSize	= m_pCompBufferInfo[idxBuf].dwBytesToAllocate;
			// 576 is enough for most sub-picture
			if ( m_dwAlphaBufHeight > 576 )
				m_dwAlphaBufHeight = 576;
		}
	}
	return 0;

}

int CHVDServiceDxva1::InitializeDXVAConfigPicture()
{
	DXVA_ConfigPictureDecode	l_ConfigPictureDecode;
	HRESULT	hr;
	DWORD	dwFunc;
	int	size;

	// Initial configuration set for INTEL Almador
	dwFunc	= (DXVA_QUERYORREPLYFUNCFLAG_DECODER_PROBE_QUERY << 8) | 1;
	size	= sizeof(DXVA_ConfigPictureDecode);
	ZeroMemory(&m_DxvaConfigPictureDecode, size);
	m_DxvaConfigPictureDecode.dwFunction	= dwFunc;
	m_DxvaConfigPictureDecode.guidConfigBitstreamEncryption	= DXVA_NoEncrypt;
	m_DxvaConfigPictureDecode.guidConfigMBcontrolEncryption	= DXVA_NoEncrypt;
	m_DxvaConfigPictureDecode.guidConfigResidDiffEncryption	= DXVA_NoEncrypt;

	if (m_dwVendorID == PCI_VENDOR_ID_SIS && m_dwDeviceID == PCI_DEVICE_ID_630)
	{
		m_DxvaConfigPictureDecode.guidConfigBitstreamEncryption	= DXVA_EncryptProt1;
		m_DxvaConfigPictureDecode.guidConfigMBcontrolEncryption	= DXVA_NoEncrypt;
		m_DxvaConfigPictureDecode.guidConfigResidDiffEncryption	= DXVA_NoEncrypt;
		m_DxvaConfigPictureDecode.bConfigBitstreamRaw	= 1;
	}
	else if (UTIL_PCI_IsVIASliceCard(m_dwVendorID, m_dwDeviceID))
	{
		m_DxvaConfigPictureDecode.guidConfigBitstreamEncryption	= DXVA_EncryptProt1;
		m_DxvaConfigPictureDecode.guidConfigMBcontrolEncryption	= DXVA_NoEncrypt;
		m_DxvaConfigPictureDecode.guidConfigResidDiffEncryption	= DXVA_NoEncrypt;
		m_DxvaConfigPictureDecode.bConfigBitstreamRaw	= 1;
	}
	else if (m_dwVendorID == PCI_VENDOR_ID_S3)
	{	// special S3 mode
		m_DxvaConfigPictureDecode.bConfigMBcontrolRasterOrder	= 1;
		m_DxvaConfigPictureDecode.bConfigResidDiffHost	= 1;
		m_DxvaConfigPictureDecode.bConfigSpatialResid8 = 1;
		m_DxvaConfigPictureDecode.bConfigIntraResidUnsigned	= 1;
	}
	else if (m_dwVendorID == PCI_VENDOR_ID_NVIDIA && UTIL_PCI_IsNVIDCT(m_dwDeviceID))
	{
		m_DxvaConfigPictureDecode.bConfigResidDiffAccelerator = m_bNVIDCT; // do this later &&(!(base.option&GPI_OPTION_DXVAEFFECT))) ? 1 : 0;
		m_DxvaConfigPictureDecode.bConfigHostInverseScan	= 1;
	}
	else if (m_dwVendorID == PCI_VENDOR_ID_SIS)
	{
		m_DxvaConfigPictureDecode.bConfigMBcontrolRasterOrder	= 1;
		m_DxvaConfigPictureDecode.bConfigResidDiffHost	= 1;
		m_DxvaConfigPictureDecode.bConfigSpatialHost8or9Clipping = 1;
		m_DxvaConfigPictureDecode.bConfigIntraResidUnsigned	= 1;
	}
	else if (m_dwVendorID == PCI_VENDOR_ID_INTEL)
	{
		if ((m_DecoderGuid == DXVA_ModeMPEG2_B) && 
			(m_HVDDecodeConfig.dwMode == HVDService::HVD_MODE_MPEG1))
		{
			m_DxvaConfigPictureDecode.bConfigMBcontrolRasterOrder	= 1;
			m_DxvaConfigPictureDecode.bConfigResidDiffHost		 	= 1;
			m_DxvaConfigPictureDecode.bConfigSpatialHost8or9Clipping= 1;
		}
		else
			m_DxvaConfigPictureDecode.bConfigBitstreamRaw	= 1;
	}
	else
	{
		m_DxvaConfigPictureDecode.bConfigMBcontrolRasterOrder	= 1;
		m_DxvaConfigPictureDecode.bConfigResidDiffAccelerator = 1;
		m_DxvaConfigPictureDecode.bConfigHostInverseScan	= 1;
	}

	// probe picture config
	hr = m_pVideoAccel->Execute(dwFunc, &m_DxvaConfigPictureDecode, size, &l_ConfigPictureDecode, size, 0, NULL);

	if (hr != S_OK)
		memcpy(&m_DxvaConfigPictureDecode, &l_ConfigPictureDecode, sizeof(DXVA_ConfigPictureDecode));

	if(m_dwVendorID==PCI_VENDOR_ID_ATI && (m_HVDDecodeConfig.dwFlags&HVD_DECODE_ENCRYPTION_PRIVATE))
	{
		if(m_DecoderGuid==DXVA_ModeH264_ATI_A)
			m_DxvaConfigPictureDecode.guidConfigResidDiffEncryption	= DXVA_ATIDRM_Encrypt;
		else if(m_DecoderGuid==DXVA_ATI_BA_H264||
			m_DecoderGuid==DXVADDI_ModeH264_E||
			m_DecoderGuid==DXVADDI_ModeH264_F||
			m_DecoderGuid==DXVA2_ModeVC1_D)
			m_DxvaConfigPictureDecode.guidConfigBitstreamEncryption	= DXVA_ATIDRM_Encrypt;
	}

     if(m_dwVendorID == PCI_VENDOR_ID_S3 && (m_DecoderGuid==DXVADDI_ModeH264_E|| m_DecoderGuid==DXVADDI_ModeH264_F))
           m_DxvaConfigPictureDecode.bConfigSpecificIDCT = 2;

	dwFunc	= (DXVA_QUERYORREPLYFUNCFLAG_DECODER_LOCK_QUERY << 8) | 1;
	m_DxvaConfigPictureDecode.dwFunction	= dwFunc;
	// lock picture config
	hr = m_pVideoAccel->Execute(dwFunc, &m_DxvaConfigPictureDecode, size, &l_ConfigPictureDecode, size, 0, NULL);

	return hr;
}

int CHVDServiceDxva1::InitializeDXVAConfigAlphaLoad()
{
	DXVA_ConfigAlphaLoad	l_AlphaLoadConfig;
	HRESULT	hr;
	DWORD	dwFunc;
	int	size;

	dwFunc	= (DXVA_QUERYORREPLYFUNCFLAG_DECODER_PROBE_QUERY << 8) | 2;
	size	= sizeof(DXVA_ConfigAlphaLoad);
	ZeroMemory(&m_DxvaConfigAlphaLoad, size);
	m_DxvaConfigAlphaLoad.dwFunction	= dwFunc;
	m_DxvaConfigAlphaLoad.bConfigDataType	= 0;
	// probe alpha loading
	hr = m_pVideoAccel->Execute(dwFunc, &m_DxvaConfigAlphaLoad, size, &l_AlphaLoadConfig, size, 0, NULL);
	if (hr != S_OK)
		memcpy(&m_DxvaConfigAlphaLoad, &l_AlphaLoadConfig, size);
	dwFunc	= (DXVA_QUERYORREPLYFUNCFLAG_DECODER_LOCK_QUERY << 8) | 2;
	m_DxvaConfigAlphaLoad.dwFunction	= dwFunc;
	// lock alpha loading
	hr = m_pVideoAccel->Execute(dwFunc, &m_DxvaConfigAlphaLoad, size, &l_AlphaLoadConfig, size, 0, NULL);

	return hr;
}

int CHVDServiceDxva1::InitializeDXVAConfigAlphaComb()
{
	DXVA_ConfigAlphaCombine	l_AlphaCombConfig;
	HRESULT	hr;
	DWORD	dwFunc;
	int	size;

	dwFunc	= (DXVA_QUERYORREPLYFUNCFLAG_DECODER_PROBE_QUERY << 8) | 3;
	size	= sizeof(DXVA_ConfigAlphaCombine);
	ZeroMemory(&m_DxvaConfigAlphaCombine, size);
	m_DxvaConfigAlphaCombine.dwFunction	= dwFunc;
	m_DxvaConfigAlphaCombine.bConfigBlendType	= 0;
	m_DxvaConfigAlphaCombine.bConfigPictureResizing	= 0;
	m_DxvaConfigAlphaCombine.bConfigOnlyUsePicDestRectArea	= 1;
	m_DxvaConfigAlphaCombine.bConfigGraphicResizing	= 1;
	m_DxvaConfigAlphaCombine.bConfigWholePlaneAlpha	= 0;
	// probe alpha combination
	hr = m_pVideoAccel->Execute(dwFunc, &m_DxvaConfigAlphaCombine, size, &l_AlphaCombConfig, size, 0, NULL);
	if (hr != S_OK)
		memcpy(&m_DxvaConfigAlphaCombine, &l_AlphaCombConfig, size);
	dwFunc	= (DXVA_QUERYORREPLYFUNCFLAG_DECODER_LOCK_QUERY << 8) | 3;
	m_DxvaConfigAlphaCombine.dwFunction	= dwFunc;
	// lock alpha combination
	hr = m_pVideoAccel->Execute(dwFunc, &m_DxvaConfigAlphaCombine, size, &l_AlphaCombConfig, size, 0, NULL);

	// Work around for SIS driver. bConfigGraphicResizing should be set as 1.
	if (hr == S_OK && m_dwVendorID == PCI_VENDOR_ID_SIS)
		m_DxvaConfigAlphaCombine.bConfigGraphicResizing = 1;

	return hr;
}

VOID CHVDServiceDxva1::DebugPrintMediaSample(IMediaSample *pSample)
{
	LONG lDataLength = pSample->GetActualDataLength();
	long lSize = pSample->GetSize();
	REFERENCE_TIME rtStart, rtEnd;
	char cSt;

	switch (pSample->GetTime(&rtStart, &rtEnd)) {
	case VFW_E_SAMPLE_TIME_NOT_SET:
		cSt = 'i';
		break;
	case VFW_S_NO_STOP_TIME:
		cSt = 'n';
		break;
	case S_OK:
	default:
		cSt = 'v';
	}

	DbgLog((LOG_TRACE, HVD_DEFAULT_TRACE_LEVEL,
		"MSample:[%c%c%c] Length=%d/%d StreamTime[%c] %d->%d\n",
		pSample->IsDiscontinuity() ? 'D' : ' ',
		pSample->IsPreroll() ? 'P' : ' ',
		pSample->IsSyncPoint() ? 'S' : ' ',
		lSize, lDataLength,
		cSt, (DWORD)rtStart, (DWORD)rtEnd
		));
}

STDMETHODIMP CHVDServiceDxva1::GetInternalCompBufferInfo(DWORD* pdwNumTypesCompBuffers, struct _tag_AMVACompBufferInfo *pamvaCompBufferInfo)
{
	if (!m_pVideoAccel)
		return E_ABORT;

	HRESULT hr = m_pVideoAccel->GetInternalCompBufferInfo(pdwNumTypesCompBuffers, pamvaCompBufferInfo);

	return hr;
}

STDMETHODIMP CHVDServiceDxva1::QueryRenderStatus(DWORD dwTypeIndex, DWORD dwBufferIndex, DWORD dwFlags)
{
	if (!m_pVideoAccel)
		return E_ABORT;
	HRESULT hr = m_pVideoAccel->QueryRenderStatus(dwTypeIndex, dwBufferIndex, dwFlags);

	return hr;
}
