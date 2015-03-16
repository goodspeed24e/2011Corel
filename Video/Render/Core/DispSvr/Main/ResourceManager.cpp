#include "stdafx.h"
#include <InitGuid.h>
#include "DynLibManager.h"
#include "ResourceManager.h"
#include "RegistryService.h"
#include "D3D9VideoMixer.h"
#include "IntelFastCompositingMixer.h"
#include "AMDPCOMMixerPresenter.h"
#include "AMDSORTPresenter.h"
#include "NvAPIPresenter.h"
#include "NvDxva2HDMixerRender.h"
#include "S3APIPresenter.h"
#include "D3D9VideoPresenter.h"
#include "D3D9OverlayPresenter.h"
#include "D3D9VideoEffect3DManager.h"
#include "Imports/LibGPU/GPUID.h"
#include "XVEManagerWrapper.h"

using namespace DispSvr;

CResourceManager::CResourceManager() :
	m_pActiveVideoMixer(NULL), 
	m_pActiveVideoPresenter(NULL),
	m_pVideoEffect3DManager(NULL),
	m_guidPreferredVideoMixer(GUID_NULL),
	m_guidPreferredVideoPresenter(GUID_NULL),
	m_bDwmCompositionChanged(false),
	m_bDwmCompositionStatus(false)
{
	LoadStaticPlugins();
}

CResourceManager::~CResourceManager()
{
	// restore to original DWM status.
	if (m_bDwmCompositionChanged)
	{
		SetDwmComposition((m_bDwmCompositionStatus) ? FALSE : TRUE);
	}

	while(!m_listResourcePlugin.empty())
	{
		Uninstall(m_listResourcePlugin.back().pInstance);
	}
}

HRESULT CResourceManager::GetInterface(REFGUID guidResourceID, REFIID riid, void **ppv)
{
	HRESULT hr = E_NOINTERFACE;

	if (guidResourceID == DISPSVR_RESOURCE_VIDEOMIXER)
	{
		CAutoLock lock(&m_csObj);
		if (m_pActiveVideoMixer)
			hr = m_pActiveVideoMixer->pInstance->QueryInterface(riid, ppv);
	}
	else if (guidResourceID == DISPSVR_RESOURCE_VIDEOPRESENTER)
	{
		CAutoLock lock(&m_csObj);
		if (m_pActiveVideoPresenter)
			hr = m_pActiveVideoPresenter->pInstance->QueryInterface(riid, ppv);
	}
	else if (guidResourceID == DISPSVR_RESOURCE_VIDEOEFFECT3DMANAGER)
	{
		CAutoLock lock(&m_csObj);
		if (m_pVideoEffect3DManager)
			hr = m_pVideoEffect3DManager->pInstance->QueryInterface(riid, ppv);
	}

    else if (guidResourceID == DISPSVR_RESOURCE_XVIDEOEFFECTMANAGER)
    {
        CAutoLock lock(&m_csObj);
        if (m_pXVideoEffectManager)
            hr = m_pXVideoEffectManager->pInstance->QueryInterface(riid, ppv);
    }
	return hr;
}

HRESULT CResourceManager::Install(IDispSvrPlugin *pObj)
{
	if (!pObj)
		return E_INVALIDARG;

	HRESULT hr;
	Plugin plugin = {0};
	CAutoLock lock(&m_csObj);
    BOOL bloadXVEManager = FALSE;

	hr = pObj->GetResourceId(&plugin.guidGroup);
	if (SUCCEEDED(hr))
	{
		hr = pObj->QueryInterface(__uuidof(IDispSvrPlugin), reinterpret_cast<void **> (&plugin.pInstance));
		if (SUCCEEDED(hr))
		{
			IUnknown *pTemp = 0;

			hr = pObj->QueryInterface(__uuidof(IDispSvrVideoMixer), (void**)&pTemp);
			if (SUCCEEDED(hr))
			{
				plugin.dwType |= RESOURCE_TYPE_VIDEOMIXER;
				pTemp->Release();
			}

			hr = pObj->QueryInterface(__uuidof(IDispSvrVideoPresenter), (void**)&pTemp);
			if (SUCCEEDED(hr))
			{
				plugin.dwType |= RESOURCE_TYPE_VIDEOPRESENTER;
				pTemp->Release();
			}

			hr = pObj->QueryInterface(__uuidof(IDispSvrVideoEffect3DManager), (void **)&pTemp);
			if (SUCCEEDED(hr))
			{
				plugin.dwType |= RESOURCE_TYPE_VIDEOEFFECT3DMANAGER;
				pTemp->Release();
			}

            hr = pObj->QueryInterface(__uuidof(IDispSvrXVEManagerWrapper), (void **)&pTemp);
            if (SUCCEEDED(hr))
            {
                plugin.dwType |= RESOURCE_TYPE_XVIDEOEFFECTMANAGER;
                pTemp->Release();

                if (m_pXVideoEffectManager)
                    Uninstall(m_pXVideoEffectManager->pInstance);
                bloadXVEManager = TRUE;
            }

			m_listResourcePlugin.push_back(plugin);
            if (bloadXVEManager)
            {
                for (PluginList::const_reverse_iterator i = m_listResourcePlugin.rbegin(); i != m_listResourcePlugin.rend(); ++i)
                {
                    if (i->dwType & RESOURCE_TYPE_XVIDEOEFFECTMANAGER)
                    {
                        m_pXVideoEffectManager = &*i;
                        break;
                    }
                }
            }
			hr = S_OK;
		}
	}
	return hr;
}

HRESULT CResourceManager::Uninstall(IDispSvrPlugin *pObj)
{
	HRESULT hr = S_FALSE;
	CAutoLock lock(&m_csObj);
	Plugin plugin = {0};

	hr = pObj->GetResourceId(&plugin.guidGroup);
	if (SUCCEEDED(hr))
	{
		if (m_pActiveVideoMixer && m_pActiveVideoMixer->pInstance == pObj)
			m_pActiveVideoMixer = NULL;

		if (m_pActiveVideoPresenter && m_pActiveVideoPresenter->pInstance == pObj)
			m_pActiveVideoPresenter = NULL;

		if (m_pVideoEffect3DManager && m_pVideoEffect3DManager->pInstance == pObj)
			m_pVideoEffect3DManager = NULL;

        if (m_pXVideoEffectManager && m_pXVideoEffectManager->pInstance == pObj)
            m_pXVideoEffectManager = NULL;

		for (PluginList::iterator it = m_listResourcePlugin.begin(); it != m_listResourcePlugin.end(); ++it)
		{
			 if (it->pInstance == pObj)
			 {
				 hr = it->pInstance->ProcessMessage(RESOURCE_MESSAGE_RELEASEDEVICE, 0);
				 int ref = it->pInstance->Release();
				 m_listResourcePlugin.erase(it);
				 return S_OK;
			 }
		}
	}

	return S_FALSE;
}

HRESULT CResourceManager::SetPreferredResource(REFGUID guidResourceID, RESOURCE_TYPE type)
{
	CAutoLock lock(&m_csObj);
	HRESULT hr = S_OK;

	if (guidResourceID != GUID_NULL)
	{
		hr = E_FAIL;
		// Find the preferred resource.
		for (PluginList::iterator it = m_listResourcePlugin.begin(); it != m_listResourcePlugin.end(); ++it)
		{
			if (IsEqualGUID(it->guidGroup, guidResourceID) && (it->dwType & type))
			{
				hr = S_OK;
				break;
			}
		}
	}

	if (SUCCEEDED(hr))
	{
		switch (type)
		{
		case RESOURCE_TYPE_VIDEOMIXER:
			m_guidPreferredVideoMixer = guidResourceID;
			DeactivePlugin(m_pActiveVideoMixer);
			break;
		case RESOURCE_TYPE_VIDEOPRESENTER:
			m_guidPreferredVideoPresenter = guidResourceID;
			DeactivePlugin(m_pActiveVideoPresenter);
			break;
		default:
			return E_FAIL;
		}
	}
	return hr;
}

HRESULT CResourceManager::ProcessMessage(RESOURCE_MESSAGE_TYPE msg, LPVOID ulParam)
{
	HRESULT hr = S_OK;

	CAutoLock lock(&m_csObj);
	if ((m_pActiveVideoMixer == NULL || m_pActiveVideoPresenter == NULL) && msg == RESOURCE_MESSAGE_SETDEVICE)
	{
		hr |= ActivatePlugin(ulParam);
	}
	else
	{
		if (m_pActiveVideoMixer)
			hr |= m_pActiveVideoMixer->pInstance->ProcessMessage(msg, ulParam);

		// resource manager is responsible to send message to plugin once and only once.
		if (m_pActiveVideoPresenter && m_pActiveVideoMixer != m_pActiveVideoPresenter)
			hr |= m_pActiveVideoPresenter->pInstance->ProcessMessage(msg, ulParam);
	}

	if (m_pVideoEffect3DManager)
		hr |= m_pVideoEffect3DManager->pInstance->ProcessMessage(msg, ulParam);

    if (m_pXVideoEffectManager)
        hr |= m_pXVideoEffectManager->pInstance->ProcessMessage(msg, ulParam);
	return hr;
}

HRESULT CResourceManager::ActivatePlugin(LPVOID ulParam)
{
	DWORD dwVendorID = 0;
	HRESULT hr = CRegistryService::GetInstance()->Get(REG_VENDOR_ID, &dwVendorID);

	if (FAILED(hr))
		return hr;

	for (PluginList::iterator it = m_listResourcePlugin.begin(); it != m_listResourcePlugin.end(); ++it)
	{
		if (IsSuitableResource(&(it->guidGroup), dwVendorID) == FALSE)
			continue;

		hr = E_FAIL;
		bool bPreferredMixer = IsEqualGUID(m_guidPreferredVideoMixer, GUID_NULL)
			|| IsEqualGUID(m_guidPreferredVideoMixer, it->guidGroup);
		bool bPreferredPresenter = IsEqualGUID(m_guidPreferredVideoPresenter, GUID_NULL)
			|| IsEqualGUID(m_guidPreferredVideoPresenter, it->guidGroup);

		// Check if the resource is used for both mixer and presenter
		// Release procedure should be done inside.
		// Otherwise resource which we do not focus will also be released
		if ( (it->dwType & RESOURCE_TYPE_VIDEOMIXER) 
			&& (it->dwType & RESOURCE_TYPE_VIDEOPRESENTER)
			&& bPreferredMixer!=bPreferredPresenter)
		{
			// The preferred mixer and the preferred presenter can not be used together.
			// Fall back to auto detect if this situation happens.
			//m_guidPreferredVideoMixer = DISPSVR_RESOURCE_VIDEOMIXER;
			//m_guidPreferredVideoPresenter  = DISPSVR_RESOURCE_VIDEOPRESENTER;
//			ASSERT(0 && "PreferredMixer != PreferredPresenter");
			
			// release this plugin
			it->pInstance->ProcessMessage(RESOURCE_MESSAGE_RELEASEDEVICE, 0);
			hr = E_FAIL;
		}
		else
		{
			if ((it->dwType & RESOURCE_TYPE_VIDEOMIXER && !m_pActiveVideoMixer && bPreferredMixer) || 
				(it->dwType & RESOURCE_TYPE_VIDEOPRESENTER && !m_pActiveVideoPresenter && bPreferredPresenter))
			{
				hr = it->pInstance->ProcessMessage(RESOURCE_MESSAGE_SETDEVICE, ulParam);
				if (SUCCEEDED(hr))
				{
					if (it->dwType & RESOURCE_TYPE_VIDEOMIXER)
					{
						ASSERT(m_pActiveVideoMixer == 0);
						m_pActiveVideoMixer = &*it;
					}

					if (it->dwType & RESOURCE_TYPE_VIDEOPRESENTER)
					{
						ASSERT(m_pActiveVideoPresenter == 0);
						m_pActiveVideoPresenter = &*it;
					}
				}
				else
				{
					it->pInstance->ProcessMessage(RESOURCE_MESSAGE_RELEASEDEVICE, 0);
				}
			}
		}

		// We get all necessary resources.
		if (m_pActiveVideoMixer && m_pActiveVideoPresenter)
		{
			hr = ControlDwmComposition();
			return S_OK;
		}
	}

	return E_FAIL;
}

// Install order is crucial for correct plug-in selection.
// Because some systems may have more than one VGA installed, for example Intel integrated + AMD discrete,
// it is not safe that we install only specific plugins by current device vendor ID.
void CResourceManager::LoadStaticPlugins()
{
	DWORD dwOSVersion = 0;

	CRegistryService::GetInstance()->Get(REG_OS_VERSION, &dwOSVersion);

	Install(new CD3D9VideoEffect3DManager);
	for (PluginList::const_iterator i = m_listResourcePlugin.begin(); i != m_listResourcePlugin.end(); ++i)
	{
		if (i->dwType & RESOURCE_TYPE_VIDEOEFFECT3DMANAGER)
		{
			m_pVideoEffect3DManager = &*i;
			break;
		}
	}
	CDynLibManager::GetInstance()->LoadVideoEffect();

	// Fast compositing and Nv DXVA HD use DXVA2, we only take this path on Vista.
	// Intel will use DXVA HD + D3D9 Overlay on Windows 7 or later.
	if (dwOSVersion >= OS_VISTA)
	{
		Install(new CIntelFastCompositingMixer);
		Install(new CNvDxva2HDMixerRender);
	}

	Install(new CAMDPCOMMixerPresenter);

	// install DXVA HD mixer in windows 7 or later by default.
	if (dwOSVersion >= OS_WIN7)
		Install(new CD3D9Dxva2HDVideoMixer);

	if (dwOSVersion >= OS_VISTA)
		Install(new CD3D9Dxva2VideoMixer);

	Install(new CD3D9VideoMixer);

	Install(new CAMDSORTVideoPresenter);
	Install(new CNvAPIPresenter);
	Install(new CS3APIPresenter);
	Install(new CD3D9VideoPresenter);

	if (dwOSVersion >= OS_WIN7)
		Install(new CD3D9OverlayPresenter);
}

void CResourceManager::DeactivePlugin(const Plugin *pPlugin)
{
	if (pPlugin == 0)
		return;

	pPlugin->pInstance->ProcessMessage(RESOURCE_MESSAGE_RELEASEDEVICE, 0);
	if (m_pActiveVideoMixer == pPlugin)
	{
		ASSERT(pPlugin->dwType & RESOURCE_TYPE_VIDEOMIXER);
		m_pActiveVideoMixer = 0;
	}

	if (m_pActiveVideoPresenter == pPlugin)
	{
		ASSERT(pPlugin->dwType & RESOURCE_TYPE_VIDEOPRESENTER);
		m_pActiveVideoPresenter = 0;
	}
}

BOOL CResourceManager::IsSuitableResource(const GUID *pGuid, DWORD dwVendorID)
{
	if (PCI_VENDOR_ID_ATI == dwVendorID)
	{
		if (IsEqualGUID(*pGuid, DISPSVR_RESOURCE_AMDPCOMVIDEOMIXERPRESENTER) || 
			IsEqualGUID(*pGuid, DISPSVR_RESOURCE_AMDSORTVIDEOPRESENTER))
			return TRUE;
	}
	
	if (PCI_VENDOR_ID_INTEL == dwVendorID)
	{
		if (IsEqualGUID(*pGuid, DISPSVR_RESOURCE_INTELFASTCOMPVIDEOMIXER))
			return TRUE;
	}

	if (PCI_VENDOR_ID_NVIDIA == dwVendorID)
	{
		if (IsEqualGUID(*pGuid, DISPSVR_RESOURCE_NVAPIVIDEOPRESENTER))
			return TRUE;
	}

	if (PCI_VENDOR_ID_S3 == dwVendorID)
	{
		if (IsEqualGUID(*pGuid, DISPSVR_RESOURCE_S3APIVIDEOPRESENTER))
			return TRUE;
	}

	/* 
	this workaround for SIS, it can't open the dxva2 videoprocessor, if 
	you open it that you can see green screen on the windowless mode. [Bugzilla# 46649]
	*/
	if(PCI_VENDOR_ID_SIS == dwVendorID)
	{  
		if(IsEqualGUID(*pGuid, DISPSVR_RESOURCE_D3DDXVA2VIDEOMIXER))
			return FALSE;
	}

	if (IsEqualGUID(*pGuid, DISPSVR_RESOURCE_D3DVIDEOMIXER) ||
		(IsEqualGUID(*pGuid, DISPSVR_RESOURCE_D3DDXVA2VIDEOMIXER) && CDynLibManager::GetInstance()->pfnDXVA2CreateVideoService) ||
		(IsEqualGUID(*pGuid, DISPSVR_RESOURCE_D3DDXVAHDVIDEOMIXER) && CDynLibManager::GetInstance()->pfnDXVA2CreateVideoService) ||
		IsEqualGUID(*pGuid, DISPSVR_RESOURCE_D3DVIDEOPRESENTER) ||
		IsEqualGUID(*pGuid, DISPSVR_RESOURCE_D3DOVERALYPRESENTER))
		return TRUE;

	return FALSE;
}

HRESULT CResourceManager::ControlDwmComposition()
{
	HRESULT hr = E_FAIL;
	bool bDWMOff = false;

	if (IsEqualGUID(m_pActiveVideoPresenter->guidGroup, DISPSVR_RESOURCE_AMDSORTVIDEOPRESENTER))
	{
		bDWMOff = true;
	}

	if(bDWMOff)
		hr = SetDwmComposition(FALSE);

	return hr;
}

HRESULT CResourceManager::SetDwmComposition(BOOL bEnable)
{
	HRESULT hr = E_FAIL;

	BOOL bIsEnabled = FALSE;
	if (CDynLibManager::GetInstance()->pfnDwmIsCompositionEnabled)
	{
		hr = CDynLibManager::GetInstance()->pfnDwmIsCompositionEnabled(&bIsEnabled);
		ASSERT(SUCCEEDED(hr));
	}

	if ((bIsEnabled != bEnable) && CDynLibManager::GetInstance()->pfnDwmEnableComposition)
	{
		hr = CDynLibManager::GetInstance()->pfnDwmEnableComposition(bEnable);
		if (SUCCEEDED(hr))
		{
			m_bDwmCompositionChanged = true;
			m_bDwmCompositionStatus = !!bEnable;
		}
	}

	return hr;
}