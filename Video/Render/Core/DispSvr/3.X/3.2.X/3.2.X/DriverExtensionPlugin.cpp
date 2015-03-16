#include "stdafx.h"
#include "DriverExtensionPlugin.h"
#include "NvAPIPresenter.h"
#include "IntelDxva2Device.h"
#include "AMDPCOMMixerPresenter.h"

using namespace DispSvr;

CD3D9DriverExtensionPlugin::CD3D9DriverExtensionPlugin() : m_pAdapter(NULL)
{
}

CD3D9DriverExtensionPlugin::~CD3D9DriverExtensionPlugin()
{
	SAFE_DELETE(m_pAdapter);
}

// IUnknown
STDMETHODIMP CD3D9DriverExtensionPlugin::QueryInterface(REFIID riid, void **ppv)
{
	HRESULT hr = E_NOINTERFACE;

	if (riid == __uuidof(IDispSvrDriverExtension))
	{
		hr = GetInterface((IDispSvrDriverExtension *)this, ppv);
	}
	else
	{
		hr = CD3D9PluginBase::QueryInterface(riid, ppv);
	}
	return hr;
}

STDMETHODIMP CD3D9DriverExtensionPlugin::QueryPresenterCaps(DWORD VideoDecodeCaps, DispSvr::PresenterCaps* pCaps)
{
	HRESULT hr = E_FAIL;
	if (VideoDecodeCaps > 0 && SUCCEEDED(CheckAdapter()))
		hr = m_pAdapter->QueryPresenterCaps(VideoDecodeCaps, pCaps);
	return hr;
}

STDMETHODIMP CD3D9DriverExtensionPlugin::SetStereoInfo(IDirect3DSurface9 *pBaseView, IDirect3DSurface9 *pDependentView, INT iOffset, BOOL bStereoEnable, MIXER_STEREO_MODE stereoMixingMode)
{
	HRESULT hr = E_FAIL;
	if (SUCCEEDED(CheckAdapter()))
		hr = m_pAdapter->SetStereoInfo(pBaseView, pDependentView, iOffset, bStereoEnable, stereoMixingMode);
	return hr;
}

STDMETHODIMP CD3D9DriverExtensionPlugin::QueryContentProtectionCaps(DriverExtContentProtectionCaps *pCaps)
{
	HRESULT hr = E_FAIL;
	if (SUCCEEDED(CheckAdapter()))
		hr = m_pAdapter->QueryContentProtectionCaps(pCaps);
	return hr;
}

STDMETHODIMP CD3D9DriverExtensionPlugin::QueryHDMIStereoModeCaps(HWND hWnd, DispSvr::DriverExtHDMIStereoModeCap **ppCaps, UINT *puCount)
{
    HRESULT hr = E_FAIL;

    if (SUCCEEDED(CheckAdapter()))
        hr = m_pAdapter->QueryHDMIStereoModeCaps( hWnd, ppCaps, puCount);

    return hr;
}

STDMETHODIMP CD3D9DriverExtensionPlugin::EnableHDMIStereoMode(BOOL bEnable, DispSvr::DriverExtHDMIStereoModeCap *pCap, BOOL *pbReCreateDevice)
{
    HRESULT hr = E_FAIL;

    if (SUCCEEDED(CheckAdapter()))
        hr = m_pAdapter->EnableHDMIStereoMode( bEnable, pCap, pbReCreateDevice);

    return hr;
}

STDMETHODIMP CD3D9DriverExtensionPlugin::QueryAdapterInfo(HWND hWnd, HMONITOR hMonitor, DispSvr::DrvExtAdapterInfo *pInfo)
{
    HRESULT hr = E_FAIL;

    if (SUCCEEDED(CheckAdapter()))
        hr = m_pAdapter->QueryAdapterInfo(hWnd, hMonitor, pInfo);

    return hr;
}

STDMETHODIMP CD3D9DriverExtensionPlugin::Clear()
{
    HRESULT hr = E_FAIL;
    if (SUCCEEDED(CheckAdapter()))
        hr = m_pAdapter->Clear();
    return hr;
}

STDMETHODIMP CD3D9DriverExtensionPlugin::_SetDevice(IUnknown *pDevice)
{
    HRESULT hr = CD3D9PluginBase::_SetDevice(pDevice); // driver extension is optional, we do not report error if there is no available extension.

    if (m_pAdapter)
    {
        delete m_pAdapter;
        m_pAdapter = NULL;
        CheckAdapter();
    }
    return hr;
}

STDMETHODIMP CD3D9DriverExtensionPlugin::_ReleaseDevice()
{
	SAFE_DELETE(m_pAdapter);
    return CD3D9PluginBase::_ReleaseDevice();
}

HRESULT CD3D9DriverExtensionPlugin::CheckAdapter()
{
    HRESULT hr = E_FAIL;

    if (m_pAdapter)
        return S_FALSE;

//    CHECK_POINTER(m_pDevice);

	switch (GetRegistry(REG_VENDOR_ID, 0))
	{
	case PCI_VENDOR_ID_NVIDIA:
		hr = CNvAPIDeviceExtensionAdapter::GetAdapter(&m_pAdapter);
		break;

	case PCI_VENDOR_ID_INTEL:
		// in coproc mode, only route BD playback that uses overlay to DGPU.
		// NVIDIA uses overlay or NVAPI_D3D9_xx calls as triggers to make transition.
		if (GetRegistry(REG_COPROC_ACTIVE_VENDOR_ID, 0) == PCI_VENDOR_ID_NVIDIA)
			hr = CNvAPIDeviceExtensionAdapter::GetAdapter(&m_pAdapter);
		else
			hr = CIntelDxva2DriverExtAdapter::GetAdapter(&m_pAdapter);
		break;
	case PCI_VENDOR_ID_ATI:
		hr = CAMDDeviceExtensionAdapter::GetAdapter(&m_pAdapter);
		break;
	}

    if (m_pAdapter && m_pDevice)
        m_pAdapter->SetDevice(m_pDevice);
    ASSERT(FAILED(hr) || m_pAdapter);
    return hr;
}
