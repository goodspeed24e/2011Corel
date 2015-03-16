#pragma once

#include "./DriverExtensionHelper.h"
//struct HDMIDisplayMode
//{
//    UINT uWidth;
//    UINT uHeight;
//    UINT uRefreshRate;
//};

using namespace DispSvr;

class CHDMIStereoModeHelper
{
    struct sContentFormat
    {
        UINT uWidth;
        UINT uHeight;
        UINT uRefreshRate;
    };

public:
    static HRESULT GetHelper(CHDMIStereoModeHelper **ppHelper);
    ULONG AddRef();
    ULONG Release();

public:
    CHDMIStereoModeHelper();
    virtual ~CHDMIStereoModeHelper();
public:
    HRESULT UpdateVideoWindow(HWND hWnd);
    BOOL IsHDMIStereoModeSupported(DriverExtHDMIStereoModeCap *pHDMIDisplayMode); //return false means it is not HDMI1.4 output
    BOOL IsHDMIStereoModeEnabled(); // return true to indicate that HDMI 1.4 stereo mode is enabled.
    HRESULT GetSelectedHDMIDisplayMode(DriverExtHDMIStereoModeCap *pHDMIDisplayMode);
    HRESULT GetAppropriateHDMIDisplayMode(DriverExtHDMIStereoModeCap *pHDMIDisplayMode);
    HRESULT EnableHDMIStereoMode(BOOL bEnable, DriverExtHDMIStereoModeCap *pHDMIDisplayMode, BOOL *pbReCreateDevice);
    HRESULT UpdateContentFormat(UINT uWidth, UINT uHeight, UINT uRefreshRate);
    HRESULT GetContentFormat(UINT *puWidth, UINT *puHeight, UINT *puRefreshRate);
    HRESULT SetDefaultDisplayMode(DriverExtHDMIStereoModeCap *pHDMIDisplayMode);
    HRESULT GetDefaultDisplayMode(DriverExtHDMIStereoModeCap *pHDMIDisplayMode);
protected:
    HRESULT ReleaseHDMIStereoCaps();
    HRESULT QueryHDMIStereoMode();
protected:
    LONG m_RefCount;
    HWND m_hWnd;
    DriverExtHDMIStereoModeCap m_DefaultDisplayMode;
    DriverExtHDMIStereoModeCap m_StereoMode;
    DriverExtHDMIStereoModeCap *m_pHDMIStereoCaps;
    UINT m_uHDMIModeCount;
    BOOL m_bEnableHDMIStereoMode;
    sContentFormat m_ContentFormat;
};
