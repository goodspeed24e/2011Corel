#include "stdafx.h"
#include "RegistryService.h"

using namespace DispSvr;

///////////////////////////////////////////////////////////////////////////////
/// CRegistryImpl
struct COSVersionInitializer : public IRegistryInitializer
{
    virtual HRESULT GetRegistryValue(REGISTRY_TYPE eType, registry_t &value)
    {
        OSVERSIONINFOEX osvi;

        ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
        osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
        if (GetVersionEx((OSVERSIONINFO *) &osvi))
        {
            value = GetOSVersion(osvi.dwMajorVersion, osvi.dwMinorVersion);
            return S_OK;
        }
        return E_FAIL;
    }

    // OS version reference page : http://msdn.microsoft.com/en-us/library/ms724832(VS.85).aspx
    static inline DWORD GetOSVersion(DWORD dwMajorVersion, DWORD dwMinorVersion)
    {
        OS_VERSION dwOsVersoin = OS_OLDER;

        if (dwMajorVersion > 6 || (dwMajorVersion == 6 && dwMinorVersion > 0))
            dwOsVersoin = OS_WIN7;
        else if (dwMajorVersion == 6 && dwMinorVersion == 0)
            dwOsVersoin = OS_VISTA;
        else if (dwMajorVersion > 5 || (dwMajorVersion == 5 && dwMinorVersion > 0))
            dwOsVersoin = OS_XP;
        else if (dwMajorVersion == 5 && dwMinorVersion == 0)
            dwOsVersoin = OS_2000;
        else
            dwOsVersoin = OS_OLDER;

        return dwOsVersoin;
    }
};

///////////////////////////////////////////////////////////////////////////////
/// CRegistryService
CRegistryService::CRegistryService() : m_Registry(REG_MAX)
{
    static COSVersionInitializer s_RegOSInitializer;
    BindInitializer(REG_OS_VERSION, &s_RegOSInitializer);
}

HRESULT CRegistryService::Set(REGISTRY_TYPE eType, registry_t value)
{
    ASSERT(static_cast<UINT> (eType) < m_Registry.size());
    CRegistryService::_Registry &reg = m_Registry[eType];
    
    reg.value = value;
    reg.bInitialized = true;
    return S_OK;
}

HRESULT CRegistryService::Get(REGISTRY_TYPE eType, registry_t *pValue)
{
    ASSERT(static_cast<UINT> (eType) < m_Registry.size());
    ASSERT(pValue != 0);

    HRESULT hr = E_FAIL;

    CRegistryService::_Registry &reg = m_Registry[static_cast<UINT> (eType)];
    if (reg.bInitialized)
    {
        *pValue = reg.value;
        return S_OK;
    }

    if (reg.pInitializer)
    {
        hr = reg.pInitializer->GetRegistryValue(eType, reg.value);
        if (SUCCEEDED(hr))
        {
            reg.bInitialized = true;
            *pValue = reg.value;
        }
    }

    return hr;
}

HRESULT CRegistryService::Clear(REGISTRY_TYPE eType)
{
    ASSERT(static_cast<UINT> (eType) < m_Registry.size());
    m_Registry[static_cast<UINT> (eType)].bInitialized = false;
    return S_OK;
}

HRESULT CRegistryService::BindInitializer(REGISTRY_TYPE eType, IRegistryInitializer *pInitializer)
{
    ASSERT(static_cast<UINT> (eType) < m_Registry.size());
    m_Registry[static_cast<UINT> (eType)].pInitializer = pInitializer;
    return S_OK;
}
