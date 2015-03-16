#ifndef _DISPSVR_REGISTRYSERVICE_H_
#define _DISPSVR_REGISTRYSERVICE_H_

#include <vector>

namespace DispSvr
{
	enum REGISTRY_TYPE
	{
		REG_VENDOR_ID = 0,
		REG_DEVICE_ID,
		REG_LOCAL_VIDMEM,
		REG_OS_MAJOR_VERSION,
		REG_OS_MINOR_VERSION,
		REG_OS_LOGPIXELX,
		REG_OS_LOGPIXELY,
		REG_BACKBUFFER_WIDTH,
		REG_BACKBUFFER_HEIGHT,
		REG_DISPLAY_WIDTH,
		REG_DISPLAY_HEIGHT,
		REG_DISPLAY_REFRESH_RATE,
		REG_OS_VERSION,						//< OS_VERSION
		REG_DISPLAY_XVYCC_MONITOR_TYPE,		//< DISPLAY_XVYCC_MONITOR_TYPE
		REG_DISPLAY_XVYCC_GAMUT,			//< using extended gamut -> TRUE: enable, FALSE: disable
		REG_MAX
	};

	enum OS_VERSION
	{
		OS_OLDER = 0,
		OS_2000,
		OS_XP,
		OS_VISTA,
		OS_WIN7
	};

	enum DISPLAY_XVYCC_MONITOR_TYPE
	{
		DISPLAY_XVYCC_MONITOR_NOT_SUPPORT,
		DISPLAY_XVYCC_MONITOR_BT601,
		DISPLAY_XVYCC_MONITOR_BT709
	};

	/// A registry service to store and retrieve information across DispSvr.
	/// we may store the type "any" later.
	class CRegistryService :  public DispSvr::Singleton<CRegistryService>
	{
	public:
		CRegistryService () : m_Registry(REG_MAX)
		{
			OSVERSIONINFOEX osvi;

			ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
			osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
			if (!GetVersionEx((OSVERSIONINFO *) &osvi))
				return;

			m_Registry[REG_OS_MAJOR_VERSION] = osvi.dwMajorVersion;
			m_Registry[REG_OS_MINOR_VERSION] = osvi.dwMinorVersion;
			m_Registry[REG_OS_VERSION] = GetOSVersion(osvi.dwMajorVersion, osvi.dwMinorVersion);
		}

		// OS version reference page : http://msdn.microsoft.com/en-us/library/ms724832(VS.85).aspx
		DWORD GetOSVersion(DWORD dwMajorVersion, DWORD dwMinorVersion)
		{
			DWORD dwOsVersoin = OS_OLDER;

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

		HRESULT Set(REGISTRY_TYPE eType, DWORD dwValue)
		{
			ASSERT(static_cast<UINT> (eType) < m_Registry.size());
			m_Registry[eType] = dwValue;
			return S_OK;
		}

		HRESULT Get(REGISTRY_TYPE eType, DWORD *pValue) const
		{
			ASSERT(static_cast<UINT> (eType) < m_Registry.size());
			ASSERT(pValue != 0);
			if (pValue == 0)
				return E_FAIL;

			*pValue = m_Registry[static_cast<UINT> (eType)];
			return S_OK;
		}

	private:
		std::vector<DWORD> m_Registry;
	};

	// Helper functions for easy use of RegistryService.
	static inline DWORD GetRegistry(REGISTRY_TYPE eType, DWORD dwDefault)
	{
		DWORD dwValue;
		if (FAILED(CRegistryService::GetInstance()->Get(eType, &dwValue)))
		{
			return dwDefault;
		}
		return dwValue;
	}

	static inline HRESULT SetRegistry(REGISTRY_TYPE eType, DWORD dwValue)
	{
		return CRegistryService::GetInstance()->Set(eType, dwValue);
	}
}

#endif	// _DISPSVR_REGISTRYSERVICE_H_