#include "stdafx.h"
#include "DispSvr.h"
#include "ResourceManager.h"
#include "D3D9VideoMixer.h"


using namespace DispSvr;

CD3D9Dxva2VideoMixer::CD3D9Dxva2VideoMixer()
{
	m_GUID = DISPSVR_RESOURCE_D3DDXVA2VIDEOMIXER;
}

CD3D9Dxva2VideoMixer::~CD3D9Dxva2VideoMixer()
{

}

STDMETHODIMP CD3D9Dxva2VideoMixer::_SetDevice(IUnknown *pDevice)
{
	HRESULT hr = CD3D9VideoMixer::_SetDevice(pDevice);
	if (SUCCEEDED(hr))
	{
		hr = GenerateDxva2VPList();
		SelectVideoProcessor();
	}
	return hr;
}
