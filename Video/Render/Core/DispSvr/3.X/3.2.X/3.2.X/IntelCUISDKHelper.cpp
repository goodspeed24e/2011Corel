#include "stdafx.h"
#include "Imports/ThirdParty/Intel/CUISDK/Common.h"
#include "Imports/ThirdParty/Intel/CUISDK/IgfxExt.h"
#include "Imports/ThirdParty/Intel/CUISDK/IgfxExt.h"
#include "Imports/ThirdParty/Intel/CUISDK/IgfxExt_i.c"

#include "IntelCUISDKHelper.h"

using namespace DispSvr;


static inline HRESULT GetMonitorUID(ICUIExternal8 *pCUI, DWORD dwDeviceTypeReq, DWORD *uidMonitorReq)
{
    DWORD uidMonitor = NULL;
    DWORD dwDeviceType = NULL;
    DWORD dwStatus = NULL;
    HRESULT hr = E_FAIL;

    if (NULL != pCUI)
    {
        BSTR bstrDev1 = SysAllocString(L"\\\\.\\Display1");
        BSTR bstrDev2 = SysAllocString(L"\\\\.\\Display2");

        for (int i = 0; i < 6; i++)
        {
            hr = pCUI->EnumAttachableDevices(bstrDev1, i, &uidMonitor, &dwDeviceType, &dwStatus); 
            if((dwDeviceTypeReq == dwDeviceType) && (IGFX_DISPLAY_DEVICE_NOTATTACHED != dwStatus))
            {
                *uidMonitorReq = uidMonitor;
                break;
            }
        }

        if (NULL == *uidMonitorReq)
        {
            for (int j = 0; j < 4; j++)
            {
                hr = pCUI->EnumAttachableDevices(bstrDev2, j, &uidMonitor, &dwDeviceType, &dwStatus); 
                if((dwDeviceTypeReq == dwDeviceType) && (IGFX_DISPLAY_DEVICE_NOTATTACHED != dwStatus))
                {
                    *uidMonitorReq = uidMonitor;
                    break;
                }
            }
        }

        SysFreeString(bstrDev1);
        SysFreeString(bstrDev2);
        if (NULL == *uidMonitorReq)
        {
            hr = E_FAIL;
        }
    }

    return hr;

#if 0  

	// updated from latest Intel CUI Util sample code, without tested.

	DWORD uidMonitor = NULL;
	DWORD dwDeviceType = NULL;
	DWORD dwStatus = NULL;
	HRESULT hr = E_FAIL;

	//Check if the SDK interface is valid
	if(NULL != pCUI)
	{
		BSTR bstrDev1 = SysAllocString(L"\\\\.\\Display1");
		BSTR bstrDev2 = SysAllocString(L"\\\\.\\Display2");
		BSTR bstrDev[2] = {bstrDev1, bstrDev2};

		for(int AdapterIndex = 0; AdapterIndex < 2 ; AdapterIndex++)
		{
			//Currently CUI supports 7 device types
			for(int index = 0; index < 7 ; index++)
			{
				//The tweek to get the HDMI and DP support on this API
				if(0 == index)
				{
					dwDeviceType = IGFX_FLAG_HDMI_DEIVCE_SUPPORT | IGFX_FLAG_DP_DEVICE_SUPPORT;
				}

				hr = pCUI->EnumAttachableDevices(bstrDev[AdapterIndex], index, &uidMonitor, &dwDeviceType, &dwStatus); 
				if(SUCCEEDED(hr))
				{
					//Check if the Device type matches and is it attached
					if((dwDeviceTypeReq == dwDeviceType) && (IGFX_DISPLAY_DEVICE_NOTATTACHED != dwStatus))
					{
						//yes got the monitor ID of the required device type
						*uidMonitorReq = uidMonitor;
						break;
					}
				}
			}

			//Break if we already got a valid devce ID
			if(NULL != *uidMonitorReq)
			{
				break;
			}
		}		
		
		SysFreeString(bstrDev1);
		SysFreeString(bstrDev2);

		//Check if we got a valid device ID
		if (NULL == *uidMonitorReq)
		{
			hr = E_FAIL;
		}

	}
	return hr;
#endif

}

CIntelCUIHelper *CIntelCUIHelper::GetHelper()
{
	static CIntelCUIHelper s_Helper;
	return &s_Helper;
}

CIntelCUIHelper::CIntelCUIHelper()
{
}

CIntelCUIHelper::~CIntelCUIHelper()
{
}

HRESULT CIntelCUIHelper::IsXVYCCMonitor(HMONITOR hMonitor, BOOL *pXVYCCMonitor)
{
	if (!pXVYCCMonitor)
		return E_FAIL;

	IGFX_SOURCE_HDMI_GBD_DATA sIntelData;
	DWORD dwError = 0;
	HRESULT hr = E_FAIL;
    CComPtr<ICUIExternal8> pCUI;

	*pXVYCCMonitor = FALSE;
    hr = CoCreateInstance(CLSID_CUIExternal, NULL, CLSCTX_SERVER, IID_ICUIExternal8, (void **)&pCUI);
    if (FAILED(hr))
        return hr;

	ZeroMemory(&sIntelData, sizeof(sIntelData));
	hr = GetMonitorUID(pCUI, IGFX_ExternalFP, &sIntelData.dwSourceID);
	if (FAILED(hr))
		return hr;

	hr = pCUI->GetDeviceData(IGFX_SOURCE_HDMI_GBD_GUID, sizeof(sIntelData), (BYTE *) &sIntelData, &dwError);
	if (SUCCEEDED(hr))
		*pXVYCCMonitor = sIntelData.IsXVYCCSupported;
	DbgMsg("0x%x = CUI::GetDeviceData(IGFX_SOURCE_HDMI_GBD_GUID: xvycc supported = %d, enabled = %d, error = 0x%x",
		hr, sIntelData.IsXVYCCSupported, sIntelData.IsXVYCCEnabled, dwError);
	return hr;
}

HRESULT CIntelCUIHelper::SetGamutMetadata(void *pGamutMetadata, DWORD dwSize)
{
	HRESULT hr = E_FAIL;
	IGFX_SOURCE_HDMI_GBD_DATA sIntelData;
	DWORD dwError = 0;
    CComPtr<ICUIExternal8> pCUI;

    hr = CoCreateInstance(CLSID_CUIExternal, NULL, CLSCTX_SERVER, IID_ICUIExternal8, (void **)&pCUI);
    if (FAILED(hr))
        return hr;

	ZeroMemory(&sIntelData, sizeof(sIntelData));
	hr = GetMonitorUID(pCUI, IGFX_ExternalFP, &sIntelData.dwSourceID);
	if (FAILED(hr))
		return hr;

	hr = pCUI->GetDeviceData(IGFX_SOURCE_HDMI_GBD_GUID, sizeof(sIntelData), (BYTE *) &sIntelData, &dwError);
	DbgMsg("0x%x = CUI::GetDeviceData(IGFX_SOURCE_HDMI_GBD_GUID: xvycc supported = %d, enabled = %d, error = 0x%x",
		hr, sIntelData.IsXVYCCSupported, sIntelData.IsXVYCCEnabled, dwError);
	if (SUCCEEDED(hr) && sIntelData.IsXVYCCSupported)
	{
		//High byte = 1, Low byte = 3 to indicate HDMI 1.3
		sIntelData.MediaSourceHDMIGBD.Version = 0x0103;
		sIntelData.MediaSourceHDMIGBD.Size = GBD_DATA_SIZE;

        if (sIntelData.IsXVYCCEnabled)
        {
            if (pGamutMetadata == 0)	// disable
            {
                sIntelData.IsXVYCCEnabled = FALSE;
                memset(sIntelData.MediaSourceHDMIGBD.GBDPayLoad, 0, GBD_DATA_SIZE);
            }
            else
            {
                memcpy(sIntelData.MediaSourceHDMIGBD.GBDPayLoad, pGamutMetadata, dwSize);
            }
            hr = pCUI->SetDeviceData(IGFX_SOURCE_HDMI_GBD_GUID, sizeof(sIntelData), (BYTE *) &sIntelData, &dwError);
        }
        else
        {
            if (pGamutMetadata != 0)	// enable
            {
                sIntelData.IsXVYCCEnabled = TRUE;	// to enable xvycc if not already on.
                memcpy(sIntelData.MediaSourceHDMIGBD.GBDPayLoad, pGamutMetadata, dwSize);
                hr = pCUI->SetDeviceData(IGFX_SOURCE_HDMI_GBD_GUID, sizeof(sIntelData), (BYTE *) &sIntelData, &dwError);
            }
        }

		DbgMsg("0x%x = CUI::SetDeviceData(IGFX_SOURCE_HDMI_GBD_GUID", hr);
	}
	
	return hr;
}