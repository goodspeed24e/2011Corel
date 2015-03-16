#include "stdafx.h"
#include "DispSvr.h"
#include "ResourceManager.h"
#include "DynLibManager.h"
#include "MathVideoMixing.h"
#include "RegistryService.h"
#include "NvDxva2HDMixerRender.h"

CNvDxva2HDMixerRender::CNvDxva2HDMixerRender()
{
	m_GUID = DISPSVR_RESOURCE_NVAPIVIDEOPRESENTER;
	// Because the mixing target format is YUY2, it is not a D3D9 render target.
	// MIXER_CAP_CAN_CHANGE_DESTINATION, MIXER_CAP_3D_RENDERTARGET, MIXER_CAP_VIRTUALIZE_FROM_ORIGIN are not supported.
	m_MixerCaps.dwFlags = 0;
}

CNvDxva2HDMixerRender::~CNvDxva2HDMixerRender()
{

}

STDMETHODIMP CNvDxva2HDMixerRender::_SetDevice(IUnknown *pDevice)
{
	HRESULT hr = E_FAIL;

	// It is an ugly way to avoid CreateNvAPI which may take a few hundred milliseconds.
	// hr = CNvAPIPresenter::_SetDevice(pDevice);
	hr = CD3D9VideoPresenterBase::_SetDevice(pDevice);
	if (SUCCEEDED(hr))
	{	
		hr = InitNvAPI();
		if (FAILED(hr))
			return hr;

		CAutoLock lock(&m_csLock);
		m_PresenterCaps.bSupportXvYCC = TRUE;
		hr = UpdateDisplayProperty();
		if (FAILED(hr))
			return hr;

		hr = CreateNvAPI();
		if (SUCCEEDED(hr))
		{
			// Update PresenterCaps
			m_PresenterCaps.bIsOverlay = TRUE;
		}
		else
		{
			DestroyNvAPI();
		}
	}
	
	// m_dwMaxUseQueueSize is zero if fall back to original RGBoverlay
	if(FAILED(hr) || m_dwMaxUseQueueSize == 0)
		return E_FAIL;

	hr = CD3D9Dxva2HDVideoMixer::_SetDevice(pDevice);
	if (FAILED(hr))
		return hr;

	if (m_HDMIInfo.isMonxvYCC709Capable)
		SetRegistry(REG_DISPLAY_XVYCC_MONITOR_TYPE, DISPLAY_XVYCC_MONITOR_BT709);
	else
		SetRegistry(REG_DISPLAY_XVYCC_MONITOR_TYPE, DISPLAY_XVYCC_MONITOR_BT601);

	ASSERT(m_PresenterCaps.bSupportXvYCC);
	return hr;
}

STDMETHODIMP CNvDxva2HDMixerRender::_ReleaseDevice()
{
	HRESULT hr = E_FAIL;

	SetRegistry(REG_DISPLAY_XVYCC_MONITOR_TYPE, DISPLAY_XVYCC_MONITOR_NOT_SUPPORT);
	hr = CNvAPIPresenter::_ReleaseDevice();
	hr = CD3D9Dxva2HDVideoMixer::_ReleaseDevice();
	return hr;
}

STDMETHODIMP CNvDxva2HDMixerRender::QueryInterface(REFIID riid, void **ppv)
{
	HRESULT hr = E_NOINTERFACE;

	if (riid == __uuidof(IDispSvrVideoPresenter))
	{
		hr = GetInterface((IDispSvrVideoPresenter *)this, ppv);
	}
	else
	{
		hr = CD3D9Dxva2HDVideoMixer::QueryInterface(riid, ppv);
	}
	return hr;
}

STDMETHODIMP CNvDxva2HDMixerRender::ProcessMessage(RESOURCE_MESSAGE_TYPE eMessage, LPVOID ulParam)
{
	return CD3D9Dxva2HDVideoMixer::ProcessMessage(eMessage, ulParam);
}

STDMETHODIMP CNvDxva2HDMixerRender::BeginRender()
{
	HRESULT hr = CD3D9VideoPresenterBase::BeginRender();
	if (SUCCEEDED(hr))
	{
		CAutoLock lock(&m_csLock);
		if(m_dwMaxUseQueueSize)
		{
			if(m_bNvPresentBusy)
			{
				m_bNvPresentBusy = false;
				return hr;
			}

			m_ObjHandle = m_hObj[m_dwQueueIdx];
			m_dwQueueIdx = (++m_dwQueueIdx) % m_dwMaxUseQueueSize;
		}
	}
	return hr;
}

HRESULT CNvDxva2HDMixerRender::CreateOverlay(D3DSURFACE_DESC *desc, NV_DX_CREATE_VIDEO_PARAMS *CVParams)
{
	NvAPI_Status nvret = NVAPI_ERROR;
	HRESULT hr = S_OK;
	CComPtr<IDirectXVideoAccelerationService> pAccelerationService;

	CHECK_POINTER(CDynLibManager::GetInstance()->pfnDXVA2CreateVideoService);
	hr = CDynLibManager::GetInstance()->pfnDXVA2CreateVideoService(m_pDevice, __uuidof(IDirectXVideoAccelerationService), (VOID**)&pAccelerationService);
	if (FAILED(hr))
		return hr;

	for (DWORD i = 0; i < m_dwMaxUseQueueSize; i++)
	{
		hr = pAccelerationService->CreateSurface( 
			desc->Width,
			desc->Height,
			0, 
			D3DFMT_YUY2, 
			D3DPOOL_DEFAULT, 
			0, 
			DXVA2_VideoProcessorRenderTarget, 
			&m_pRT[i], 
			NULL);

		// fill with color black
		if (SUCCEEDED(hr))
			hr = m_pDevice->ColorFill(m_pRT[i], NULL, D3DCOLOR_XYUV(0, 128, 128));
		else
			return S_FALSE;
	}

	// Fill in the overlay surface handles
	NV_OVLY_SURFS_INFO	surfInfo;
	nvret = NvAPI_D3D9_GetOverlaySurfaceHandles(m_pDevice, &surfInfo);
	if (nvret != NVAPI_OK)
		return E_FAIL;

	for (UINT i = 0; i < surfInfo.numSurfs; i++)
		m_hObj[i] = CVParams->hOvlSurfs[i] = surfInfo.handle[i];

	return S_OK;
}

HRESULT CNvDxva2HDMixerRender::_Execute(IDirect3DSurface9 *, const RECT &rcDst, const RECT &rcDstClip)
{
	CAutoLock lock(&m_csLock);
	DWORD dwDrawingIdx = (m_dwQueueIdx - 1 + m_dwMaxUseQueueSize) % m_dwMaxUseQueueSize;
	IDirect3DSurface9 *pDestSurface = m_pRT[dwDrawingIdx];

	HRESULT hr = VideoProcessBltHD(pDestSurface, rcDst, rcDstClip);
	return hr;
}
