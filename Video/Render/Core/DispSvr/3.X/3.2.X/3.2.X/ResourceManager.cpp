#include "stdafx.h"
#include "D3D9VideoMixer.h"
#include "StereoVideoMixer.h"
#include "IntelFastCompositingMixer.h"
#include "AMDPCOMMixerPresenter.h"
#include "AMDSORTPresenter.h"
#include "NvAPIPresenter.h"
#include "NvDxva2HDMixerRender.h"
#include "S3APIPresenter.h"
#include "D3D9VideoPresenter.h"
#include "D3D9OverlayPresenter.h"
#include "D3D9VideoEffect3DManager.h"
#include "XVEManagerWrapper.h"
#include "PersistentInterfaceProxy.h"
#include "DriverExtensionPlugin.h"

using namespace DispSvr;

enum RESOURCE_TYPE_MASK
{
	RESOURCE_TYPE_MASK_VIDEOMIXER = (1 << 0),
	RESOURCE_TYPE_MASK_VIDEOPRESENTER = (1 << 1),
	RESOURCE_TYPE_MASK_VIDEOEFFECT3DMANAGER = (1 << 2),
    RESOURCE_TYPE_MASK_XVIDEOEFFECTMANAGER = (1 << 3),
	RESOURCE_TYPE_MASK_TEXTUREPOOL = (1 << 4),
	RESOURCE_TYPE_MASK_DRIVEREXTENSION = (1 << 5),
    RESOURCE_TYPE_MASK_VIDEOMIXERMODEL = (1 << 6)
};

static bool operator< (REFGUID a, REFGUID b)
{
	return memcmp(&a, &b, sizeof(GUID)) < 0;
}

namespace PresetEnvPredicate
{
	bool NoRequirement(DWORD dwVendorID, DWORD dwDeviceID) { return true; }
	bool NvAPI(DWORD dwVendorID, DWORD dwDeviceID) { return PCI_VENDOR_ID_NVIDIA == dwVendorID; }
	bool AMDSORT(DWORD dwVendorID, DWORD dwDeviceID) { return PCI_VENDOR_ID_ATI == dwVendorID; }
	bool AMDPCOM(DWORD dwVendorID, DWORD dwDeviceID) { return PCI_VENDOR_ID_ATI == dwVendorID && !UTIL_PCI_IsATIX1200SERIES(dwDeviceID); }
	bool IntelFC(DWORD dwVendorID, DWORD dwDeviceID) { return PCI_VENDOR_ID_INTEL == dwVendorID; }
	bool S3API(DWORD dwVendorID, DWORD dwDeviceID) { return PCI_VENDOR_ID_S3 == dwVendorID; }
	bool SisWorkaround(DWORD dwVendorID, DWORD dwDeviceID) { return PCI_VENDOR_ID_SIS != dwVendorID; }
    bool NvWorkaround(DWORD dwVendorID, DWORD dwDeviceID) { return PCI_VENDOR_ID_NVIDIA != dwVendorID; }
}

CResourceManager::CResourceManager() :
	m_bDwmCompositionChanged(false),
	m_bDwmCompositionStatus(false),
    m_pInterfaceProxy(new CPersistentInterfaceProxy)
{
	// initialize active plugin item requirements.
	{
		ResourceGroup g = {0};
		g.dwTypeMask = RESOURCE_TYPE_MASK_VIDEOMIXER;
		g.iidRequiredInterface = __uuidof(IDispSvrVideoMixer);
		g.bMandatory = true;
        g.pInterfaceProxy = m_pInterfaceProxy;
		m_mapResourceGroup[DISPSVR_RESOURCE_VIDEOMIXER] = g;
	}

	{
		ResourceGroup g = {0};
		g.dwTypeMask = RESOURCE_TYPE_MASK_VIDEOPRESENTER;
		g.iidRequiredInterface = __uuidof(IDispSvrVideoPresenter);
		g.bMandatory = true;
		m_mapResourceGroup[DISPSVR_RESOURCE_VIDEOPRESENTER] = g;
	}

	{
		ResourceGroup g = {0};
		g.dwTypeMask = RESOURCE_TYPE_MASK_VIDEOEFFECT3DMANAGER;
		g.iidRequiredInterface = __uuidof(IDispSvrVideoEffect3DManager);
		m_mapResourceGroup[DISPSVR_RESOURCE_VIDEOEFFECT3DMANAGER] = g;
	}

	{
		ResourceGroup g = {0};
		g.dwTypeMask = RESOURCE_TYPE_MASK_XVIDEOEFFECTMANAGER;
		g.iidRequiredInterface = __uuidof(IDispSvrXVEManagerWrapper);
		m_mapResourceGroup[DISPSVR_RESOURCE_XVIDEOEFFECTMANAGER] = g;
	}

	{
		ResourceGroup g = {0};
		g.iidRequiredInterface = __uuidof(IDispSvrTexturePool);
		g.dwTypeMask = RESOURCE_TYPE_MASK_TEXTUREPOOL;
        g.bMandatory = true;
		m_mapResourceGroup[DISPSVR_RESOURCE_TEXTUREPOOL] = g;
	}

	{
		ResourceGroup g = {0};
		g.iidRequiredInterface = __uuidof(IDispSvrDriverExtension);
		g.dwTypeMask = RESOURCE_TYPE_MASK_DRIVEREXTENSION;
		m_mapResourceGroup[DISPSVR_RESOURCE_DRIVEREXTENSION] = g;
	}

    {
		ResourceGroup g = {0};
		g.iidRequiredInterface = __uuidof(IDispSvrVideoMixerModel);
		g.dwTypeMask = RESOURCE_TYPE_MASK_VIDEOMIXERMODEL;
        g.bMandatory = true;
		m_mapResourceGroup[DISPSVR_RESOURCE_VIDEOMIXERMODEL] = g;
	}

	LoadStaticPlugins();
}

CResourceManager::~CResourceManager()
{
    SAFE_DELETE(m_pInterfaceProxy);
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

HRESULT CResourceManager::GetInterface(REFGUID guidGroupId, REFIID riid, void **ppv)
{
	HRESULT hr = E_NOINTERFACE;

	CAutoLock lock(&m_csObj);
	ResourceGroupMap::iterator i = m_mapResourceGroup.find(guidGroupId);
	if (i != m_mapResourceGroup.end())
	{
        ResourceGroup &group = i->second;
		if (group.pPlugin)
        {
            if (group.pInterfaceProxy)
                hr = group.pInterfaceProxy->GetInterface(guidGroupId, riid, ppv);
            else
                hr = group.pPlugin->pInstance->QueryInterface(riid, ppv);
        }
	}
	return hr;
}

HRESULT CResourceManager::Install(IDispSvrPlugin *pObj, EnvPredicate pPredicate)
{
	if (!pObj)
		return E_INVALIDARG;

	HRESULT hr;
	Plugin plugin = {0};
	plugin.InRequiredEnv = pPredicate ? pPredicate : PresetEnvPredicate::NoRequirement;
	CAutoLock lock(&m_csObj);

	hr = pObj->GetResourceId(&plugin.guidId);
	if (SUCCEEDED(hr))
	{
		hr = pObj->QueryInterface(__uuidof(IDispSvrPlugin), reinterpret_cast<void **> (&plugin.pInstance));
		if (SUCCEEDED(hr))
		{
			IUnknown *pTemp = 0;

			for (ResourceGroupMap::const_iterator i = m_mapResourceGroup.begin(); i != m_mapResourceGroup.end(); ++i)
			{
				hr = pObj->QueryInterface(i->second.iidRequiredInterface, (void **)&pTemp);
				if (SUCCEEDED(hr))
				{
					plugin.dwTypeMask |= i->second.dwTypeMask;
					pTemp->Release();
				}
			}

			if (plugin.dwTypeMask == 0)		// the plugin does not expose any usable interface
			{
				plugin.pInstance->Release();
				hr = E_FAIL;
				ASSERT(0 && "CResourceManager::Install fatal error");
			}
			else
			{
				m_listResourcePlugin.push_back(plugin);
				hr = S_OK;
			}
		}
	}
	return hr;
}

HRESULT CResourceManager::Uninstall(IDispSvrPlugin *pObj)
{
	CAutoLock lock(&m_csObj);

	for (ResourceGroupMap::iterator i = m_mapResourceGroup.begin(); i != m_mapResourceGroup.end(); ++i)
	{
		ResourceGroup &group = i->second;
		if (group.pPlugin && group.pPlugin->pInstance == pObj)
			group.pPlugin = NULL;
	}

	for (PluginList::iterator it = m_listResourcePlugin.begin(); it != m_listResourcePlugin.end(); ++it)
	{
		 if (it->pInstance == pObj)
		 {
			 it->pInstance->ProcessMessage(RESOURCE_MESSAGE_RELEASEDEVICE, 0);
			 int ref = it->pInstance->Release();
			 m_listResourcePlugin.erase(it);
			 return S_OK;
		 }
	}

	return S_FALSE;
}

HRESULT CResourceManager::SetPreferredResource(REFGUID guidResourceID, REFGUID guidGroupId)
{
	CAutoLock lock(&m_csObj);
	ResourceGroupMap::iterator i = m_mapResourceGroup.find(guidGroupId);
	if (i == m_mapResourceGroup.end())
		return E_INVALIDARG;

	ResourceGroup &group = i->second;

	DeactivatePlugin(group.pPlugin);
	if (guidResourceID == GUID_NULL)
	{
		group.guidPreferredPlugin = guidResourceID;
		return S_OK;
	}

	// Find the preferred resource.
	for (PluginList::iterator it = m_listResourcePlugin.begin(); it != m_listResourcePlugin.end(); ++it)
	{
		if (IsEqualGUID(it->guidId, guidResourceID) && (it->dwTypeMask & group.dwTypeMask) != 0)
		{
			group.guidPreferredPlugin = guidResourceID;
			return S_OK;
		}
	}
	return E_FAIL;
}

HRESULT CResourceManager::GetActiveResrouceGUID(REFGUID guidGroupId, GUID *pGUID)
{
    HRESULT hr = E_FAIL;
    CHECK_POINTER(pGUID);

    CAutoLock lock(&m_csObj);
    ResourceGroupMap::iterator i = m_mapResourceGroup.find(guidGroupId);
    if (i != m_mapResourceGroup.end())
    {
        ResourceGroup &group = i->second;
        if(group.pPlugin)
        {
            *pGUID = group.pPlugin->guidId;
            hr = S_OK;
        }
    }
    return hr;
}

// broadcast message to all active plugins once and only once per instance.
HRESULT CResourceManager::ProcessMessage(RESOURCE_MESSAGE_TYPE msg, LPVOID ulParam)
{
	HRESULT hr = S_OK;
	DWORD dwProcessedPluginMask = 0;

	CAutoLock lock(&m_csObj);
	for (ResourceGroupMap::iterator i = m_mapResourceGroup.begin(); i != m_mapResourceGroup.end(); ++i)
	{
		ResourceGroup &group = i->second;
		if (group.pPlugin)
		{
			if ((dwProcessedPluginMask & group.dwTypeMask) == 0)
			{
				hr |= group.pPlugin->pInstance->ProcessMessage(msg, ulParam);
				dwProcessedPluginMask |= group.pPlugin->dwTypeMask;
			}
		}
		// m_ActivePluginItem[i].pPlugin == NULL is not allowed.
		else if (group.bMandatory && (RESOURCE_MESSAGE_SETDEVICE == msg || RESOURCE_MESSAGE_RESETDEVICE == msg))
		{
			if (group.dwTypeMask & (RESOURCE_TYPE_MASK_VIDEOMIXER | RESOURCE_TYPE_MASK_VIDEOPRESENTER))
			{
				hr |= ActivateMixerPresenter(ulParam);
				dwProcessedPluginMask |= (RESOURCE_TYPE_MASK_VIDEOMIXER | RESOURCE_TYPE_MASK_VIDEOPRESENTER);
			}
			else
			{
				hr |= ActivatePlugin(ulParam, group);
				dwProcessedPluginMask |= group.dwTypeMask;
			}
		}
	}

	return hr;
}

HRESULT CResourceManager::ActivatePlugin(LPVOID ulParam, ResourceGroup &group)
{
	HRESULT hr = E_INVALIDARG;

	ASSERT(group.pPlugin == 0);
	if (group.pPlugin)
		return hr;

	for (PluginList::reverse_iterator it = m_listResourcePlugin.rbegin(); it != m_listResourcePlugin.rend(); ++it)
	{
		if (it->dwTypeMask & group.dwTypeMask)
		{
			hr = it->pInstance->ProcessMessage(RESOURCE_MESSAGE_SETDEVICE, ulParam);
			if (SUCCEEDED(hr))
			{
				group.pPlugin = &*it;
				break;
			}
			else
			{
				it->pInstance->ProcessMessage(RESOURCE_MESSAGE_RELEASEDEVICE, 0);
			}
		}
	}
	return hr;
}

// mixer and presenter plugins are the special cases.
HRESULT CResourceManager::ActivateMixerPresenter(LPVOID ulParam)
{
    HRESULT hr = S_OK;
	DWORD dwVendorID = GetRegistry(REG_VENDOR_ID, 0);
    DWORD dwDeviceID = GetRegistry(REG_DEVICE_ID, 0);

	ResourceGroup &groupMixer = m_mapResourceGroup[DISPSVR_RESOURCE_VIDEOMIXER];
	ResourceGroup &groupPresenter = m_mapResourceGroup[DISPSVR_RESOURCE_VIDEOPRESENTER];

	const BOOL bAnyMixer = groupMixer.guidPreferredPlugin == GUID_NULL;
	const BOOL bAnyPresenter = groupPresenter.guidPreferredPlugin == GUID_NULL;
	bool bPluginIsMixer, bPluginIsPresenter, bPreferredMixer, bPreferredPresenter;

	for (PluginList::iterator it = m_listResourcePlugin.begin(); it != m_listResourcePlugin.end(); ++it)
	{
		hr = E_FAIL;

		bPluginIsMixer = (it->dwTypeMask & groupMixer.dwTypeMask) != 0;
		bPluginIsPresenter = (it->dwTypeMask & groupPresenter.dwTypeMask) != 0;

		if (!(bPluginIsMixer || bPluginIsPresenter) || !it->InRequiredEnv(dwVendorID, dwDeviceID))
			continue;

		bPreferredMixer = bPluginIsMixer && bAnyMixer || groupMixer.guidPreferredPlugin == it->guidId;
		bPreferredPresenter = bPluginIsPresenter && bAnyPresenter || groupPresenter.guidPreferredPlugin == it->guidId;

		// if a plugin supplies both mixer and presenter, it should be both preferred mixer/presenter,
		// or we don't want the plugin.
		if (bPluginIsMixer && bPluginIsPresenter)
		{
			if (bPreferredMixer != bPreferredPresenter)
				continue;

//			// if a mixer or a presenter is already activated, try to find the next plugin which can be paired with.
//			if (groupMixer.pPlugin || groupPresenter.pPlugin)
//				continue;
		}

		if ((!groupMixer.pPlugin && bPreferredMixer) || (!groupPresenter.pPlugin && bPreferredPresenter))
		{
			hr = it->pInstance->ProcessMessage(RESOURCE_MESSAGE_SETDEVICE, ulParam);
			if (SUCCEEDED(hr))
			{
				if (bPluginIsMixer)
				{
					if (groupMixer.pPlugin)
						groupMixer.pPlugin->pInstance->ProcessMessage(RESOURCE_MESSAGE_RELEASEDEVICE, 0);
					groupMixer.pPlugin = &*it;
				}

				if (bPluginIsPresenter)
				{
					if (groupPresenter.pPlugin)
						groupPresenter.pPlugin->pInstance->ProcessMessage(RESOURCE_MESSAGE_RELEASEDEVICE, 0);
					groupPresenter.pPlugin = &*it;
				}
			}
			else
			{
				it->pInstance->ProcessMessage(RESOURCE_MESSAGE_RELEASEDEVICE, 0);
			}
		}

		if (groupMixer.pPlugin && groupPresenter.pPlugin)
		{
            m_pInterfaceProxy->SetInstance(DISPSVR_RESOURCE_VIDEOMIXER, groupMixer.pPlugin->pInstance);
            m_pInterfaceProxy->SetInstance(DISPSVR_RESOURCE_VIDEOPRESENTER, groupPresenter.pPlugin->pInstance);
			//hr = ControlDwmComposition(); We don't need it now due to control Aero On/Off by UI not DispSVR.
			return S_OK;
		}
	}

    ASSERT(0 && "it is fatal if we can't activate mixer&presenter");
	return E_FAIL;
}

HRESULT CResourceManager::ActivatePluginWithoutDevice(REFGUID guidGroupId)
{
	ResourceGroupMap::iterator itGroup = m_mapResourceGroup.find(guidGroupId);
	if (itGroup == m_mapResourceGroup.end())
		return E_INVALIDARG;

	ResourceGroup &group = itGroup->second;
	for (PluginList::reverse_iterator i = m_listResourcePlugin.rbegin(); i != m_listResourcePlugin.rend(); ++i)
	{
		if (i->dwTypeMask & group.dwTypeMask)
		{
			group.pPlugin = &*i;
            m_pInterfaceProxy->SetInstance(guidGroupId, group.pPlugin->pInstance);
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
	DWORD dwOSVersion = GetRegistry(REG_OS_VERSION, 0);

	Install(new CDispSvrXVEManagerWrapper);
	ActivatePluginWithoutDevice(DISPSVR_RESOURCE_XVIDEOEFFECTMANAGER);

	Install(new CD3D9VideoEffect3DManager);
	ActivatePluginWithoutDevice(DISPSVR_RESOURCE_VIDEOEFFECT3DMANAGER);
	CDynLibManager::GetInstance()->LoadVideoEffect();

	Install(new CD3D9DriverExtensionPlugin);
	ActivatePluginWithoutDevice(DISPSVR_RESOURCE_DRIVEREXTENSION);

    Install(new CD3D9TexturePool);
    ActivatePluginWithoutDevice(DISPSVR_RESOURCE_TEXTUREPOOL);

    Install(new CD3D9VideoMixerModel);
    ActivatePluginWithoutDevice(DISPSVR_RESOURCE_VIDEOMIXERMODEL);

	// Fast compositing and Nv DXVA HD use DXVA2, we only take this path on Vista.
	// Intel will use DXVA HD + D3D9 Overlay on Windows 7 or later.
	if (dwOSVersion >= OS_VISTA)
	{
		Install(new CIntelFastCompositingMixer, PresetEnvPredicate::IntelFC);
		Install(new CNvDxva2HDMixerRender, PresetEnvPredicate::NvAPI);
	}

	Install(new CAMDPCOMMixerPresenter, PresetEnvPredicate::AMDPCOM);

	// install DXVA HD mixer in windows 7 or later by default.
	if (dwOSVersion >= OS_WIN7)
        Install(new CD3D9Dxva2HDVideoMixer);

	if (dwOSVersion >= OS_VISTA)
		Install(new CD3D9Dxva2VideoMixer, PresetEnvPredicate::SisWorkaround);

	Install(new CD3D9VideoMixer);

	Install(new CAMDSORTVideoPresenter, PresetEnvPredicate::AMDSORT);
	Install(new CNvAPIPresenter, PresetEnvPredicate::NvAPI);
	Install(new CS3APIPresenter, PresetEnvPredicate::S3API);
	Install(new CD3D9VideoPresenter);

	if (dwOSVersion >= OS_WIN7)
    {
		Install(new CD3D9OverlayPresenter);
        Install(new CStereoVideoMixer);
    }
}

void CResourceManager::DeactivatePlugin(const Plugin *pPlugin)
{
	if (pPlugin == 0 || !pPlugin->pInstance)
		return;

	pPlugin->pInstance->ProcessMessage(RESOURCE_MESSAGE_RELEASEDEVICE, 0);

    // because a plugin can support multiple resource group, we should look for other
    // groups to see the deactivated plugin is being used as well.
	for (ResourceGroupMap::iterator i = m_mapResourceGroup.begin(); i != m_mapResourceGroup.end(); ++i)
	{
		if (i->second.pPlugin == pPlugin)
        {
            m_pInterfaceProxy->SetInstance(i->first, NULL);
			i->second.pPlugin = NULL;
        }
	}
}

HRESULT CResourceManager::ControlDwmComposition()
{
	HRESULT hr = E_FAIL;
	bool bDWMOff = false;

	const ResourceGroup &group = m_mapResourceGroup[DISPSVR_RESOURCE_VIDEOPRESENTER];
	if (group.pPlugin && group.pPlugin->guidId == DISPSVR_RESOURCE_AMDSORTVIDEOPRESENTER)
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