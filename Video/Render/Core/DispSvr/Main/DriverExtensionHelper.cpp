#include "stdafx.h"
#include "Imports/LibGPU/GPUID.h"
#include "ResourceManager.h"
#include "RegistryService.h"
#include "DynLibManager.h"
#include "DriverExtensionHelper.h"
#include "NvAPIPresenter.h"
#include "IntelDxva2Device.h"

HRESULT CDriverExtensionHelper::GetAdapter(IDirect3DDevice9 *pDevice, DispSvr::IDriverExtensionAdapter **ppAdapter)
{
	HRESULT hr = E_FAIL;

	CHECK_POINTER(pDevice);
	CHECK_POINTER(ppAdapter);

	switch (GetRegistry(REG_VENDOR_ID, 0))
	{
	case PCI_VENDOR_ID_NVIDIA:
		hr = CNvAPIDeviceExtensionAdapter::GetAdapter(pDevice, ppAdapter);
		break;

	case PCI_VENDOR_ID_INTEL:
		hr = CIntelDxva2DriverExtAdapter::GetAdapter(pDevice, ppAdapter);
		break;

	default:
		break;
	}

	return hr;
}
