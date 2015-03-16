#ifndef _DISPSVR_INTEL_CUISDK_HELPER_H_
#define _DISPSVR_INTEL_CUISDK_HELPER_H_


interface ICUIExternal8;

namespace DispSvr
{
    class CIntelCUIHelper
    {
    public:
	    ~CIntelCUIHelper();
	    static CIntelCUIHelper *GetHelper();
	    HRESULT IsXVYCCMonitor(HMONITOR hMonitor, BOOL *pXVYCCMonitor);
	    HRESULT SetGamutMetadata(void *pGamutMetadata, DWORD dwSize);

    private:
	    CIntelCUIHelper();
    };
}

#endif  // _DISPSVR_INTEL_CUISDK_HELPER_H_