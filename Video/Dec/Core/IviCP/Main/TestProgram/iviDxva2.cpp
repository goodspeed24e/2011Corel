#include <streams.h>
#include <windows.h>
#include <tchar.h>
#include <d3d9.h>
#include <InitGuid.h>
#include <dxva2api.h>
#include <mfidl.h>
#include <evr.h>
#include "iviDxva2.h"

CiviDxva2::CiviDxva2(bool bRenderTargetSynch): m_cRef(1), m_pD3DDev9(0), m_hD3DDevice(0),
	m_pD3DDev9Man(0), m_ppRenderTarget(0), m_guidVideoSubType(GUID_NULL), m_pguidDecGuids(NULL),
	m_hDxva2Lib(0), pfnDXVA2CreateVideoService(0), m_aEvent(0), m_bRenderTargetSynch(bRenderTargetSynch)
{
	m_nSurfAllocated = m_nLastSurfAllocated = 0;
	m_nWidth = m_nHeight = 0;
	m_nFormat = m_nLastFormat = D3DFMT_UNKNOWN;
	m_DeviceGuid = GUID_NULL;
}

void CiviDxva2::CleanUpLastAllocInfo()
{
	if(!m_nLastSurfAllocated)
		return;
	m_nLastSurfAllocated = 0; m_nLastFormat = D3DFMT_UNKNOWN;
}

void CiviDxva2::CleanUp()
{
	CiviDxva2::ReleaseRenderTargets();
	CiviDxva2::CleanUpLastAllocInfo();
	m_guidVideoSubType = GUID_NULL;
	m_pguidDecGuids = NULL;
	m_nWidth = m_nHeight = 0;
	m_nNumOfDecGuids =0;
	return;
}

CiviDxva2::~CiviDxva2()
{
	BeginDeviceLoss();
	CleanUp();

	if(m_hDxva2Lib)
		FreeLibrary(m_hDxva2Lib);
}

STDMETHODIMP CiviDxva2::BeginDeviceLoss()
{
	ReleaseRenderTargets();
	if (m_pD3DDev9)
	{
		m_pD3DDev9->Release();
		m_pD3DDev9 = 0;
	}
	return S_OK;
}

STDMETHODIMP CiviDxva2::EndDeviceLoss(IDirect3DDevice9 *pDevice9)
{
	if (pDevice9)
	{
		m_pD3DDev9 = pDevice9;
		m_pD3DDev9->AddRef();
		return S_OK;
	}
	return E_FAIL;
}

void CiviDxva2::ReleaseRenderTargets()
{
	while(m_nSurfAllocated)
	{
		m_nSurfAllocated--;
		ULONG nRef = m_ppRenderTarget[m_nSurfAllocated]->Release(); //ASSERT(nRef == 0);

		if(m_aEvent && m_aEvent[m_nSurfAllocated])
			CloseHandle(m_aEvent[m_nSurfAllocated]);
	}
	if(m_ppRenderTarget)
	{
		delete[] m_ppRenderTarget; m_ppRenderTarget = 0;
	}
	if(m_aEvent)
	{
		delete[] m_aEvent; m_aEvent = 0;
	}
	m_nFormat = D3DFMT_UNKNOWN;
}

STDMETHODIMP CiviDxva2::SetRenderTargetInfo(UINT  nWidth, UINT  nHeight, GUID* pVideoSubType, GUID* pDxvaGuid, int iNumOfGuid)
{
	if(pVideoSubType)
		m_guidVideoSubType = *pVideoSubType;
	if(pDxvaGuid)
	{
		m_pguidDecGuids = pDxvaGuid; m_nNumOfDecGuids = iNumOfGuid;
	}
	m_nWidth = nWidth;
	m_nHeight = nHeight;

#ifdef Graphic_Vendors_Didnt_Waive_Padding_Spec_Requirement
	const UINT nDoubleMacroBlockSize = 32;
	UINT nModulus = m_nWidth % nDoubleMacroBlockSize; // per DXVA2 spec requirement
	if(nModulus)
		m_nWidth += nDoubleMacroBlockSize - nModulus;
	nModulus = m_nHeight % nDoubleMacroBlockSize;
	if(nModulus)
		m_nHeight += nDoubleMacroBlockSize - nModulus;
#endif
	return S_OK;
}

STDMETHODIMP CiviDxva2::GetRenderTargetInfo(UINT* pWidth, UINT* pHeight, GUID* pVideoSubType)
{
	if(!pWidth || !pHeight)
		return E_POINTER;
	*pWidth = m_nWidth; *pHeight = m_nHeight;
	if(pVideoSubType)
		*pVideoSubType = m_guidVideoSubType;
	return S_OK;
}

STDMETHODIMP CiviDxva2::GetSurface(WORD index, IDirect3DSurface9** ppD3DSurf9, TSurfaceLock lock) const
{
	if(!m_nSurfAllocated)
		return E_OUTOFMEMORY;
	if(index < 0 || index >= m_nSurfAllocated)
		return E_INVALIDARG;
	if(m_aEvent && NoLock != lock)
	{
		if(Wait == lock)
#if 1
			WaitForSingleObject(m_aEvent[index], INFINITE);
#else
	HRESULT hr = S_OK; // move hr definition up
		{
			static DWORD nTimeOut = 40;
			if(WAIT_TIMEOUT == WaitForSingleObject(m_aEvent[index], nTimeOut))
				hr = D3DERR_WASSTILLDRAWING;
		}
#endif
		else
			ResetEvent(m_aEvent[index]);
	}
	*ppD3DSurf9 = m_ppRenderTarget[index];
	return S_OK;
}

STDMETHODIMP CiviDxva2::ReleaseSurface(WORD index) const
{
	if(!m_nSurfAllocated)
		return E_OUTOFMEMORY;
	if(index < 0 || index >= m_nSurfAllocated)
		return E_INVALIDARG;
	if(m_aEvent)
		SetEvent(m_aEvent[index]);
	return S_OK;
}

STDMETHODIMP CiviDxva2::ReSize(long nWidth, long nHeight)
{
	HRESULT hr = SetRenderTargetInfo(nWidth, nHeight);
	if(FAILED(hr))
		return hr;
	hr = ReAllocRenderTargets();
	return hr;
}

STDMETHODIMP CiviDxva2::ReAllocRenderTargets(WORD nExraTargets)
{
	if(!m_nLastSurfAllocated)
		return E_OUTOFMEMORY;
	ReleaseRenderTargets();
	return AllocRenderTargets(m_nLastSurfAllocated + nExraTargets, m_nLastFormat);
}

STDMETHODIMP CiviDxva2::LoadDxva2Lib()
{
	if(m_hDxva2Lib)
		return S_OK;
	m_hDxva2Lib = LoadLibrary(_T("dxva2.dll"));
	if(!m_hDxva2Lib)
		return HRESULT_FROM_WIN32(GetLastError());
	pfnDXVA2CreateVideoService = (TpIviFnDXVA2CreateVideoService)
		GetProcAddress(m_hDxva2Lib, "DXVA2CreateVideoService");
	if(pfnDXVA2CreateVideoService)
		return S_OK;
	FreeLibrary(m_hDxva2Lib); m_hDxva2Lib = 0;
	return HRESULT_FROM_WIN32(GetLastError());
}

STDMETHODIMP_(ULONG) CiviDxva2::AddRef()
{
	LONG lRef = InterlockedIncrement(&m_cRef);    
	return lRef;
}

STDMETHODIMP_(ULONG) CiviDxva2::Release()
{
    LONG lRef = InterlockedDecrement(&m_cRef);
    
    if (lRef == 0)
	{
        delete this;
        return 0;
	}
	else
        return lRef;
}

CiviDxva2Decoder::CiviDxva2Decoder(IMFGetService* pMfGetService) :
		CiviDxva2(true), m_pDxVideoMemConfig(0), m_pVidDecService(0)
{
	if(!pMfGetService)
		return;
	if(FAILED(pMfGetService->GetService(MR_VIDEO_ACCELERATION_SERVICE, IID_IDirect3DDeviceManager9, (void**)&m_pD3DDev9Man)))
		return;
	pMfGetService->GetService(MR_VIDEO_ACCELERATION_SERVICE, IID_IDirectXVideoMemoryConfiguration, (void**)&m_pDxVideoMemConfig);
}

CiviDxva2Decoder::CiviDxva2Decoder(IDirect3DDevice9* pD3DDev9) : m_pDxVideoMemConfig(0), m_pVidDecService(0)
{
	if(!pD3DDev9)
		return;
	if(FAILED(LoadDxva2Lib()))
		return;
	m_pD3DDev9 = pD3DDev9; m_pD3DDev9->AddRef();
}

STDMETHODCALLTYPE CiviDxva2Decoder::~CiviDxva2Decoder()
{
	if(m_pDxVideoMemConfig)
		m_pDxVideoMemConfig->Release();
	if(m_pD3DDev9Man)
		m_pD3DDev9Man->Release();
	else if(m_pD3DDev9)
	{
		m_pD3DDev9->Release();
		m_pD3DDev9 = 0;
	}
	CiviDxva2Decoder::ReleaseVideoAccelService();
}

STDMETHODIMP CiviDxva2Decoder::ConfigureVideoMemory()
{
	if(!m_pDxVideoMemConfig)
		return E_POINTER;
	HRESULT hr;
	DXVA2_SurfaceType surfaceType;

	for (DWORD iTypeIndex = 0; ; iTypeIndex++)
	{
		hr = m_pDxVideoMemConfig->GetAvailableSurfaceTypeByIndex(iTypeIndex, &surfaceType);

		if (FAILED(hr))
		{
			break;
		}

		if (surfaceType == DXVA2_SurfaceType_DecoderRenderTarget)
		{
			hr = m_pDxVideoMemConfig->SetSurfaceType(DXVA2_SurfaceType_DecoderRenderTarget);
			break;
		}
	}
	return hr;
}

STDMETHODIMP CiviDxva2Decoder::GetVideoAccelService()
{
	if(!m_pD3DDev9 && !m_pD3DDev9Man)
		return E_POINTER;
	HRESULT hr;
	ReleaseVideoAccelService();
	if(m_pD3DDev9)
		hr = pfnDXVA2CreateVideoService(m_pD3DDev9, IID_IDirectXVideoDecoderService, (void**) &m_pVidDecService);
	else
	{
		hr = m_pD3DDev9Man->OpenDeviceHandle(&m_hD3DDevice);
		if(SUCCEEDED(hr))
			hr = m_pD3DDev9Man->GetVideoService(m_hD3DDevice, IID_IDirectXVideoDecoderService, (void**) &m_pVidDecService);
		if(FAILED(hr))
			ReleaseVideoAccelService();
	}
	return hr;
}

STDMETHODIMP_(void) CiviDxva2Decoder::ReleaseVideoAccelService()
{
	if(m_hD3DDevice)
	{
		m_pD3DDev9Man->CloseDeviceHandle(m_hD3DDevice); m_hD3DDevice = 0;
	}
	if(m_pVidDecService)
	{
		m_pVidDecService->Release(); m_pVidDecService = 0;
	}
}

STDMETHODIMP CiviDxva2Decoder::AllocRenderTargets(UINT nSurf, enum _D3DFORMAT n3DFormat)
{
	HRESULT hr; UINT i;
	if(!m_pVidDecService)
		return E_POINTER;
	m_ppRenderTarget = new IDirect3DSurface9* [nSurf];
	if(!m_ppRenderTarget)
		return E_OUTOFMEMORY;

	hr = m_pVidDecService->CreateSurface(m_nWidth, m_nHeight, nSurf - 1,// # of buffers = front + # of back buf
			 n3DFormat, D3DPOOL_DEFAULT, 0, DXVA2_VideoDecoderRenderTarget,
									m_ppRenderTarget, NULL);
	while(SUCCEEDED(hr) && m_bRenderTargetSynch)
	{
		m_aEvent = new HANDLE[nSurf];
		if(!m_aEvent)
		{
			hr = E_OUTOFMEMORY; goto CreateEventArrayFailed;
		}
		for (i = 0; i < nSurf; i++)
			m_aEvent[i] = 0;

		for (i = 0; i < nSurf; i++)
			if(!(m_aEvent[i] = CreateEvent(0, FALSE, TRUE, 0)))
				break;
		if(i == nSurf)
			break; //success
		hr = HRESULT_FROM_WIN32(GetLastError());
CreateEventArrayFailed:
		m_nSurfAllocated = nSurf;
		ReleaseRenderTargets();
		break;
	}
	if(SUCCEEDED(hr))
	{
		m_nSurfAllocated = m_nLastSurfAllocated = nSurf;
		m_nFormat = m_nLastFormat = n3DFormat;
	}
	else
	{
		delete[] m_ppRenderTarget; m_ppRenderTarget = 0;
	}
	return hr;
}

STDMETHODIMP CiviDxva2Decoder::AllocRenderTargets(ULONG nSurfMin, ULONG nSurfMax)
{
	if(!m_pVidDecService || GUID_NULL == m_guidVideoSubType && m_pD3DDev9Man || 0 == m_nWidth || 0 == m_nHeight)
		return E_POINTER;
	
	HRESULT hr;
	UINT			nDecGuidFormats, nRenderTargetFormats, i, j, k, l;
	D3DFORMAT*		pD3FormatsSupported = 0;
	GUID*			pDecGuids = 0;

	ReleaseRenderTargets();
	CleanUpLastAllocInfo();

	hr = m_pVidDecService->GetDecoderDeviceGuids(&nDecGuidFormats,&pDecGuids);	// Get support decoder device guilds
	if(FAILED(hr))
		return hr;
	
	for (i = 0; i < m_nNumOfDecGuids; i++)
	{
 		for(k = 0; k < nDecGuidFormats; k++)	// look up GUID pairs from HW side and user's side.
			if(pDecGuids[k] == m_pguidDecGuids[i])
				break;
		if(k == nDecGuidFormats)	// no match found
  			continue;

		hr = m_pVidDecService->GetDecoderRenderTargets(pDecGuids[k], &nRenderTargetFormats, &pD3FormatsSupported);	// Get support render targets
		if(FAILED(hr))
			continue;
		
		for(j = 0; j < nRenderTargetFormats; j++)
		{
			if(m_pD3DDev9Man)
			{
				FOURCCMap mapVideoSubType(&m_guidVideoSubType);
				if(mapVideoSubType.GetFOURCC() != pD3FormatsSupported[j])
					continue;
			}
			for (l = nSurfMax, hr = E_FAIL; l >= nSurfMin && FAILED(hr); l--)
				hr = AllocRenderTargets(l, pD3FormatsSupported[j]);
			
			if(SUCCEEDED(hr))
			{
				m_DeviceGuid = pDecGuids[k];
				CoTaskMemFree(pD3FormatsSupported);
				goto FreeDecGuid;
			}
		}
		CoTaskMemFree(pD3FormatsSupported);
	}
FreeDecGuid:
	if (pDecGuids)
		CoTaskMemFree(pDecGuids);
	if(i == m_nNumOfDecGuids)
		hr = E_INVALIDARG;
 	return hr;
}

STDMETHODIMP CiviDxva2Decoder::GetDecoderConfigurations(	const struct _DXVA2_VideoDesc *pVideoDesc, UINT *pCount, struct _DXVA2_ConfigPictureDecode **ppConfigs)
{
	if(!m_pVidDecService)
		return E_POINTER;
	return m_pVidDecService->GetDecoderConfigurations(m_DeviceGuid, pVideoDesc, 0, pCount, ppConfigs);
}

STDMETHODIMP CiviDxva2Decoder::CreateVideoDecoder(const struct _DXVA2_VideoDesc *pVideoDesc, const DXVA2_ConfigPictureDecode *pConfig, IDirectXVideoDecoder **ppDecode)
{
	if(!m_pVidDecService)
		return E_POINTER;
	return m_pVidDecService->CreateVideoDecoder(m_DeviceGuid, pVideoDesc, pConfig, m_ppRenderTarget, m_nSurfAllocated, ppDecode);
}

STDMETHODIMP CiviDxva2Decoder::RecoverFromD3DeviceChange()
{
	HRESULT hr = GetVideoAccelService();
	if(SUCCEEDED(hr))
		hr = ReAllocRenderTargets();
	return hr;
}

STDMETHODIMP CiviDxva2Decoder::BeginDeviceLoss()
{
	ReleaseVideoAccelService();
	return CiviDxva2::BeginDeviceLoss();
}

STDMETHODIMP CiviDxva2Decoder::EndDeviceLoss(IDirect3DDevice9* pDevice)
{
	return CiviDxva2::EndDeviceLoss(pDevice);
}

STDMETHODIMP CiviDxva2Decoder::GetDecoderDeviceSupportCount(UINT* pnCount)
{
	if(!m_pVidDecService)
		return E_POINTER;

	if (!pnCount)
		return E_INVALIDARG;

	UINT nDecGuidFormats;
	GUID* pDecGuids = 0;
	HRESULT hr;

	hr = m_pVidDecService->GetDecoderDeviceGuids(&nDecGuidFormats,&pDecGuids);	// Get support decoder device guilds
	if(FAILED(hr))
		return hr;

	*pnCount = nDecGuidFormats;
	if (pDecGuids)
		CoTaskMemFree(pDecGuids);

	return hr;
}

STDMETHODIMP CiviDxva2Decoder::GetDecoderDeviceSupportGUIDs(GUID *pGUIDs)
{
	if(!m_pVidDecService)
		return E_FAIL;

	if (!pGUIDs)
		return E_INVALIDARG;

	UINT nDecGuidFormats;
	GUID* pDecGuids = 0;
	HRESULT hr;

	hr = m_pVidDecService->GetDecoderDeviceGuids(&nDecGuidFormats,&pDecGuids);	// Get support decoder device guilds
	if(FAILED(hr))
		return hr;

	if (pDecGuids)
	{
		for (UINT i=0; i<nDecGuidFormats; i++)
			pGUIDs[i] = pDecGuids[i];

		CoTaskMemFree(pDecGuids);
	}

	return hr;
}

STDMETHODIMP CiviDxva2Decoder::GetDecoderDeviceGUIDs(UINT* pnCount, GUID **ppGuids)
{
	if(!m_pVidDecService)
		return E_FAIL;

	HRESULT hr = m_pVidDecService->GetDecoderDeviceGuids(pnCount, ppGuids);	// Get support decoder device guilds

	if(FAILED(hr))
		return hr;

	return hr;
}

CiviDxva2VidProc::CiviDxva2VidProc(IDirect3DDevice9 *pDevice)
	: m_pVidProcService(0)
{
	if(FAILED(LoadDxva2Lib()))
		return;
	m_pD3DDev9 = pDevice;
	m_pD3DDev9->AddRef();
}

CiviDxva2VidProc::~CiviDxva2VidProc()
{
	ReleaseVideoAccelService();
}

STDMETHODIMP CiviDxva2VidProc::BeginDeviceLoss()
{
	ReleaseVideoAccelService();
	return CiviDxva2::BeginDeviceLoss();
}

STDMETHODIMP CiviDxva2VidProc::EndDeviceLoss(IDirect3DDevice9* pDevice)
{
	return CiviDxva2::EndDeviceLoss(pDevice);
}

STDMETHODIMP CiviDxva2VidProc::GetVideoAccelService()
{
	if (m_pVidProcService)
		return S_FALSE;

	ReleaseVideoAccelService();
	if(m_pD3DDev9 && pfnDXVA2CreateVideoService)
		return pfnDXVA2CreateVideoService(m_pD3DDev9, IID_IDirectXVideoProcessorService, (void**) &m_pVidProcService);
	return E_FAIL;
}

STDMETHODIMP_(void) CiviDxva2VidProc::ReleaseVideoAccelService()
{
	if (m_pVidProcService)
	{
		m_pVidProcService->Release();
		m_pVidProcService = 0;
	}
}

STDMETHODIMP CiviDxva2VidProc::CreateSurface(UINT width, UINT height, enum _D3DFORMAT format, enum _D3DPOOL Pool, IDirect3DSurface9 **ppSurface)
{
	if (ppSurface && m_pVidProcService)
	{
		return m_pVidProcService->CreateSurface(width, height, 0, format, Pool, 0, DXVA2_VideoProcessorRenderTarget, ppSurface, NULL);
	}
	return E_FAIL;
}

STDMETHODIMP CiviDxva2VidProc::AllocRenderTargets(UINT nSurf, enum _D3DFORMAT n3DFormat)
{
	if(!m_pVidProcService)
		return E_POINTER;

	ReleaseRenderTargets();
	CleanUpLastAllocInfo();
	m_ppRenderTarget = new IDirect3DSurface9*[nSurf];
	if (!m_ppRenderTarget)
		return E_OUTOFMEMORY;

	HRESULT hr = m_pVidProcService->CreateSurface(m_nWidth, m_nHeight, nSurf - 1,// # of buffers = front + # of back buf
		n3DFormat, D3DPOOL_DEFAULT, 0, DXVA2_VideoProcessorRenderTarget,
		m_ppRenderTarget, NULL);
	if(SUCCEEDED(hr))
	{
		m_nSurfAllocated = m_nLastSurfAllocated = nSurf;
		m_nFormat = m_nLastFormat = n3DFormat;
	} else {
		ReleaseRenderTargets();
	}
	return hr;
}

static inline bool IsRgbRenderTarget(D3DFORMAT nFormat)
{
	switch (nFormat) {
	case D3DFMT_X8R8G8B8:
	case D3DFMT_A8R8G8B8:
		return true;
	}
	return false;
}
	
static UINT uPreferredProcAmp = DXVA2_ProcAmp_Brightness | DXVA2_ProcAmp_Contrast | DXVA2_ProcAmp_Hue | DXVA2_ProcAmp_Saturation;
static UINT uPreferredDevice = DXVA2_VPDev_HardwareDevice;

STDMETHODIMP_(bool) CiviDxva2VidProc::IsPreferredVideoCaps(const DXVA2_VideoProcessorCaps &c) const
{
	return (c.DeviceCaps & uPreferredDevice) != 0
		&& ((c.ProcAmpControlCaps & uPreferredProcAmp) == uPreferredProcAmp);
}

STDMETHODIMP CiviDxva2VidProc::CreateVideoProcessor(const _DXVA2_VideoDesc *pVideoDesc, IDirectXVideoProcessor **ppVidProcess)
{
	return CreateVideoProcessor(pVideoDesc, ppVidProcess, GUID_NULL, 0);
}

STDMETHODIMP CiviDxva2VidProc::CreateVideoProcessor(const _DXVA2_VideoDesc *pVideoDesc, IDirectXVideoProcessor **ppVidProcess, const GUID guidDesired, UINT nSubStream)
{
	if (!m_pVidProcService)
		return E_FAIL;

	HRESULT hr = E_FAIL;
	UINT uDeviceCount, uRtCount;
	GUID SelectedGuid = GUID_NULL, *pDeviceGuids = NULL;
	D3DFORMAT nFormat = D3DFMT_UNKNOWN, *RtFormats = NULL;
	DXVA2_VideoProcessorCaps VidProcCaps;
	UINT i = 0;

	m_DeviceGuid = GUID_NULL;
	while (FAILED(hr) && i < 2)
	{
		hr = m_pVidProcService->GetVideoProcessorDeviceGuids(pVideoDesc, &uDeviceCount, &pDeviceGuids);
		if (FAILED(hr))
			return hr;

		i++;
		// this is a NVIDIA G7x workaround when driver returns all GUID_NULL guids
		for (UINT k = 0; k < uDeviceCount; k++)
		{
			if (pDeviceGuids[k] == GUID_NULL)
			{
				CoTaskMemFree(pDeviceGuids);
				hr = E_FAIL;
				break;
			}
		}
	}

	for (i = 0; i < uDeviceCount && nFormat == D3DFMT_UNKNOWN; i++)
	{
		if (guidDesired != GUID_NULL && guidDesired != pDeviceGuids[i])
			continue;

		SelectedGuid = pDeviceGuids[i];
		hr = m_pVidProcService->GetVideoProcessorRenderTargets(SelectedGuid, pVideoDesc, &uRtCount, &RtFormats);
		if (FAILED(hr))
			continue;

		for (UINT j = 0; j < uRtCount; j++)
		{
			hr = m_pVidProcService->GetVideoProcessorCaps(SelectedGuid, pVideoDesc, RtFormats[j], &VidProcCaps);
			if (SUCCEEDED(hr) && ::IsRgbRenderTarget(RtFormats[j]) && IsPreferredVideoCaps(VidProcCaps))
			{
				nFormat = RtFormats[j];
				break;
			}
		}
		CoTaskMemFree(RtFormats);
	}
	CoTaskMemFree(pDeviceGuids);

	// if no preferred device, choose bob device which is available on every platform.
	if (nFormat == D3DFMT_UNKNOWN)
	{
		SelectedGuid = DXVA2_VideoProcBobDevice;
		hr = m_pVidProcService->GetVideoProcessorRenderTargets(SelectedGuid, pVideoDesc, &uRtCount, &RtFormats);
		if (FAILED(hr))
			return hr;
		nFormat = RtFormats[0];
		CoTaskMemFree(RtFormats);
	}

	SetRenderTargetInfo(pVideoDesc->SampleWidth, pVideoDesc->SampleHeight, NULL);

	// if it is a rgb render target, we can directly blit onto the texture allocated in DispObj.
	if (::IsRgbRenderTarget(nFormat))
	{
		ReleaseRenderTargets();
		m_nFormat = m_nLastFormat = nFormat;
	}
	else
	{
		// we only need one render target for video processor regardless how many reference samples are needed.
		hr = AllocRenderTargets(1, nFormat);
	}

	if (SUCCEEDED(hr))
	{
		m_DeviceGuid = SelectedGuid;
		hr = m_pVidProcService->CreateVideoProcessor(SelectedGuid, pVideoDesc, nFormat, nSubStream, ppVidProcess);
		if (FAILED(hr))
			ReleaseRenderTargets();
	}

	return hr;
}

bool CiviDxva2VidProc::IsRgbRenderTarget() const
{
	return ::IsRgbRenderTarget(m_nFormat);
}

