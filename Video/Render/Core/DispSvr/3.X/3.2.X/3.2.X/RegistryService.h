#ifndef _DISPSVR_REGISTRYSERVICE_H_
#define _DISPSVR_REGISTRYSERVICE_H_


namespace DispSvr
{
	enum REGISTRY_TYPE
	{
		REG_VENDOR_ID = 0,
		REG_DEVICE_ID,
		REG_BACKBUFFER_WIDTH,
		REG_BACKBUFFER_HEIGHT,
        REG_DISPLAY_X,                      //< monitor rect.left
        REG_DISPLAY_Y,                      //< monitor rect.top
		REG_DISPLAY_WIDTH,                  //< monitor rect.right - rect.left
		REG_DISPLAY_HEIGHT,                 //< monitor rect.bottom - rect.top
		REG_DISPLAY_REFRESH_RATE,
		REG_OS_VERSION,						//< OS_VERSION
		REG_DISPLAY_XVYCC_MONITOR_TYPE,		//< DISPLAY_XVYCC_MONITOR_TYPE
		REG_DISPLAY_XVYCC_GAMUT,			//< using extended gamut -> TRUE: enable, FALSE: disable
		REG_COPROC_VENDOR_ID,
		REG_COPROC_DEVICE_ID,
		REG_COPROC_ACTIVE_VENDOR_ID,
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

    typedef LONG registry_t;

    interface IRegistryInitializer
    {
        virtual HRESULT GetRegistryValue(REGISTRY_TYPE eType, registry_t &value) = 0;
    };

	/// A registry service to store and retrieve information across DispSvr.
	/// we may store the type "any" later.
	class CRegistryService :  public DispSvr::Singleton<CRegistryService>
	{
	public:
        CRegistryService();

		HRESULT Set(REGISTRY_TYPE eType, registry_t value);
		HRESULT Get(REGISTRY_TYPE eType, registry_t *pValue);
        HRESULT Clear(REGISTRY_TYPE eType);
        HRESULT BindInitializer(REGISTRY_TYPE eType, IRegistryInitializer *pInitializer);

    private:
        struct _Registry
        {
            registry_t value;
            IRegistryInitializer *pInitializer;
            bool bInitialized;
        };

    private:
        std::vector<_Registry> m_Registry;
	};

	// Helper functions for easy use of RegistryService.
	static inline registry_t GetRegistry(REGISTRY_TYPE eType, registry_t defaultvalue)
	{
		registry_t value;
		if (FAILED(CRegistryService::GetInstance()->Get(eType, &value)))
		{
			return defaultvalue;
		}
		return value;
	}

	static inline HRESULT SetRegistry(REGISTRY_TYPE eType, registry_t value)
	{
		return CRegistryService::GetInstance()->Set(eType, value);
	}

    static inline HRESULT ClearRegistry(REGISTRY_TYPE eType)
    {
        return CRegistryService::GetInstance()->Clear(eType);
    }
}

#endif	// _DISPSVR_REGISTRYSERVICE_H_
