#include "stdafx.h"

#include "DriverExtensionHelper.h"

#include "HDMIStereoModeHelper.h"

using namespace DispSvr;

CHDMIStereoModeHelper *s_pHDMIHelper = NULL;

HRESULT CHDMIStereoModeHelper::GetHelper(CHDMIStereoModeHelper **ppHelper)
{

    CHECK_POINTER(ppHelper);

    if (!s_pHDMIHelper)
    {
        s_pHDMIHelper = new CHDMIStereoModeHelper();
        if (!s_pHDMIHelper)
            return E_OUTOFMEMORY;
    }
    (*ppHelper) = s_pHDMIHelper;
    (*ppHelper)->AddRef();

    return S_OK;
}

ULONG CHDMIStereoModeHelper::AddRef()
{
    return InterlockedIncrement(&m_RefCount);
}

ULONG CHDMIStereoModeHelper::Release()
{
    LONG ref = InterlockedDecrement(&m_RefCount);
    if (0 == ref)
    {
        s_pHDMIHelper = NULL;
        delete this;
    }
    return ref;
}

CHDMIStereoModeHelper::CHDMIStereoModeHelper()
{
    DbgMsg("CHDMIStereoModeHelper::CHDMIStereoModeHelper");

    m_RefCount = 0;
    m_hWnd = NULL;
    ZeroMemory(&m_DefaultDisplayMode, sizeof(DriverExtHDMIStereoModeCap));
    ZeroMemory(&m_StereoMode, sizeof(DriverExtHDMIStereoModeCap));
    ZeroMemory(&m_ContentFormat, sizeof(sContentFormat));
    m_pHDMIStereoCaps = NULL;
    m_uHDMIModeCount = 0;
    m_bEnableHDMIStereoMode = FALSE;
}

CHDMIStereoModeHelper::~CHDMIStereoModeHelper()
{
    DbgMsg("CHDMIStereoModeHelper::~CHDMIStereoModeHelper");

    ReleaseHDMIStereoCaps();
}

HRESULT CHDMIStereoModeHelper::UpdateVideoWindow(HWND hWnd)
{
    DbgMsg("CHDMIStereoModeHelper::UpdateVideoWindow");

    m_hWnd = hWnd;
    return QueryHDMIStereoMode();
}

BOOL CHDMIStereoModeHelper::IsHDMIStereoModeSupported(DriverExtHDMIStereoModeCap *pHDMIDisplayMode)
{
    DbgMsg("CHDMIStereoModeHelper::IsHDMIStereoModeSupported");

//    SetVideoWindow(hWnd);
    if (m_pHDMIStereoCaps && m_uHDMIModeCount)
    {
        if (!pHDMIDisplayMode) // simply check HDMI Stereo Mode supported or not.
            return TRUE;

        for(UINT i = 0; i < m_uHDMIModeCount; i++)
        {
            if ((m_pHDMIStereoCaps[i].uWidth == pHDMIDisplayMode->uWidth) &&
                (m_pHDMIStereoCaps[i].uHeight == pHDMIDisplayMode->uHeight) &&
                (m_pHDMIStereoCaps[i].uRefreshRate == pHDMIDisplayMode->uRefreshRate) &&
                (m_pHDMIStereoCaps[i].dwStereoMode & pHDMIDisplayMode->dwStereoMode)
                )
            {
                return TRUE;
            }
        }
    }
     return FALSE;
}

BOOL CHDMIStereoModeHelper::IsHDMIStereoModeEnabled()
{
//    DbgMsg("CHDMIStereoModeHelper::IsHDMIStereoModeEnabled - Enable=%d", m_bEnableHDMIStereoMode);

    return m_bEnableHDMIStereoMode;
}

HRESULT CHDMIStereoModeHelper::GetSelectedHDMIDisplayMode(DriverExtHDMIStereoModeCap *pHDMIDisplayMode)
{
    DbgMsg("CHDMIStereoModeHelper::GetSelectedHDMIDisplayMode");

    CHECK_POINTER(pHDMIDisplayMode);

    memcpy(pHDMIDisplayMode, &m_StereoMode, sizeof(DriverExtHDMIStereoModeCap));
    return S_OK;
}

HRESULT CHDMIStereoModeHelper::GetAppropriateHDMIDisplayMode(DriverExtHDMIStereoModeCap *pHDMIDisplayMode)
{
    DbgMsg("CHDMIStereoModeHelper::GetAppropriateHDMIDisplayMode");

    CHECK_POINTER(pHDMIDisplayMode);

    //if (!IsHDMIStereoModeEnabled()) // back up current display mode as default display mode. 
    //{
    //    memcpy(&m_DefaultMode, pHDMIDisplayMode, sizeof(HDMIStereoModeCap));
    //}

    DriverExtHDMIStereoModeCap ModeSelect[] =
    {
        //Use content format as the first one.
        { m_ContentFormat.uWidth, m_ContentFormat.uHeight, m_ContentFormat.uRefreshRate, HDMI_STEREO_FRAME_PACKING},
        { 1920, 1080, 24,  HDMI_STEREO_FRAME_PACKING},
        { 1280,   720, 60,  HDMI_STEREO_FRAME_PACKING},
        { 1280,   720, 50,  HDMI_STEREO_FRAME_PACKING},
        {   720,   576, 60,  HDMI_STEREO_FRAME_PACKING},
        {   720,   576, 50,  HDMI_STEREO_FRAME_PACKING},
        {   720,   480, 60,  HDMI_STEREO_FRAME_PACKING},
        {   720,   480, 50,  HDMI_STEREO_FRAME_PACKING},
    };

    UINT uCount = sizeof(ModeSelect) / sizeof(DriverExtHDMIStereoModeCap);
    for (UINT i = 0 ; i < uCount;i++)
    {
        if ((m_ContentFormat.uRefreshRate > 0) && (m_ContentFormat.uRefreshRate > ModeSelect[i].uRefreshRate)) 
        {
            DbgMsg("Ignore display mode Width=%d, Height=%d, RefreshRate=%d due to refresh rate is less than content",
                ModeSelect[i].uWidth,
                ModeSelect[i].uHeight,
                ModeSelect[i].uRefreshRate);
            continue;
        }

        if (IsHDMIStereoModeSupported(&ModeSelect[i]))
        {
            memcpy(pHDMIDisplayMode, &ModeSelect[i], sizeof(DriverExtHDMIStereoModeCap));
            DbgMsg("CHDMIStereoModeHelper::GetAppropriateHDMIDisplayMode - Selected Display mode : Width=%d, Height=%d, RefreshRate=%d",
                pHDMIDisplayMode->uWidth,
                pHDMIDisplayMode->uHeight,
                pHDMIDisplayMode->uRefreshRate);
            return S_OK;
        }
    }
    return E_FAIL;
}

HRESULT CHDMIStereoModeHelper::EnableHDMIStereoMode(BOOL bEnable, DriverExtHDMIStereoModeCap *pHDMIDisplayMode, BOOL *pbReCreateDevice)
{
    DbgMsg("CHDMIStereoModeHelper::EnableHDMIStereoMode");
    HRESULT hr = E_FAIL;
    CComPtr<IDispSvrDriverExtension> pDrvExt;

    hr = CResourceManager::GetInstance()->GetInterface(DISPSVR_RESOURCE_DRIVEREXTENSION, __uuidof(IDispSvrDriverExtension), (void **)&pDrvExt);
    if (FAILED(hr))
    {
        return hr;
    }

    if (bEnable)
    {
        CHECK_POINTER(pHDMIDisplayMode);
        CHECK_POINTER(pbReCreateDevice);

        if (memcmp(pHDMIDisplayMode, &m_StereoMode,  sizeof(DriverExtHDMIStereoModeCap)) || !IsHDMIStereoModeEnabled()) //different stereo mode.
        {
            if (IsHDMIStereoModeSupported(pHDMIDisplayMode)) // can switch to new stereo mode
            {
                memcpy(&m_StereoMode, pHDMIDisplayMode, sizeof(DriverExtHDMIStereoModeCap));
                // Do Set HDMI Stereo Mode...
                hr = pDrvExt->EnableHDMIStereoMode(bEnable, &m_StereoMode, pbReCreateDevice);
                if (SUCCEEDED(hr)) 
                {
                    DbgMsg("CHDMIStereoModeHelper::EnableHDMIStereoMode - Enable HDMIStereoMode Succeeded ReCreateDevice=%d", (*pbReCreateDevice));
                    m_bEnableHDMIStereoMode = TRUE;
                }
                else
                {
                    DbgMsg("CHDMIStereoModeHelper::EnableHDMIStereoMode - Enable HDMIStereoMode failed");
                }
            }
            else
            {
                DbgMsg("CHDMIStereoModeHelper::EnableHDMIStereoMode - IsHDMIStereoModeSupported failed");
                return E_INVALIDARG;
            }
        }
        else
        {
            DbgMsg("CHDMIStereoModeHelper::EnableHDMIStereoMode - ignore since same display mode");
            return S_FALSE;
        }
    }
    else
    {
        //turn off HDMI Stereo Mode
        ZeroMemory(&m_StereoMode, sizeof(DriverExtHDMIStereoModeCap));
        if (IsHDMIStereoModeEnabled())
        {
            m_bEnableHDMIStereoMode = FALSE;
            hr = pDrvExt->EnableHDMIStereoMode(bEnable, &m_DefaultDisplayMode, pbReCreateDevice); // switch back to default display mode
        }
    }

    return hr;
}

HRESULT CHDMIStereoModeHelper::ReleaseHDMIStereoCaps()
{
    DbgMsg("CHDMIStereoModeHelper::ReleaseHDMIStereoCaps");

    SAFE_DELETE_ARRAY(m_pHDMIStereoCaps);
    m_uHDMIModeCount = 0;
    return S_OK;
}

HRESULT CHDMIStereoModeHelper::QueryHDMIStereoMode()
{
    DbgMsg("CHDMIStereoModeHelper::QueryHDMIStereoMode");
    HRESULT hr = E_FAIL;
    CComPtr<IDispSvrDriverExtension> pDrvExt;
    ReleaseHDMIStereoCaps();
    hr = CResourceManager::GetInstance()->GetInterface(DISPSVR_RESOURCE_DRIVEREXTENSION, __uuidof(IDispSvrDriverExtension), (void **)&pDrvExt);
    if (SUCCEEDED(hr))
    {
        hr = pDrvExt->QueryHDMIStereoModeCaps(m_hWnd, &m_pHDMIStereoCaps, &m_uHDMIModeCount);
        if (SUCCEEDED(hr))
        {
            DbgMsg("Dump HDMIStereoModeCaps Count =%d", m_uHDMIModeCount);
            for (UINT i = 0 ; i < m_uHDMIModeCount ; i++)
            {
                DbgMsg("Count Id = %d:  Mode : Width=%d, Height=%d, RefreshRate=%d, StereoMode=%d",
                    i, m_pHDMIStereoCaps[i].uWidth, m_pHDMIStereoCaps[i].uHeight, m_pHDMIStereoCaps[i].uRefreshRate, m_pHDMIStereoCaps[i].dwStereoMode);
            }
        }
    }
    return hr;
}

HRESULT CHDMIStereoModeHelper::SetDefaultDisplayMode(DriverExtHDMIStereoModeCap *pHDMIDisplayMode)
{ 
    DbgMsg("CHDMIStereoModeHelper::SetDefaultDisplayMode");

    CHECK_POINTER(pHDMIDisplayMode);
    memcpy(&m_DefaultDisplayMode, pHDMIDisplayMode, sizeof(DriverExtHDMIStereoModeCap));
    return S_OK;
}

HRESULT CHDMIStereoModeHelper::GetDefaultDisplayMode(DriverExtHDMIStereoModeCap *pHDMIDisplayMode)
{
    DbgMsg("CHDMIStereoModeHelper::GetDefaultDisplayMode");

    CHECK_POINTER(pHDMIDisplayMode);
    memcpy(pHDMIDisplayMode, &m_DefaultDisplayMode, sizeof(DriverExtHDMIStereoModeCap));
    return S_OK;
}

HRESULT CHDMIStereoModeHelper::UpdateContentFormat(UINT uWidth, UINT uHeight, UINT uRefreshRate)
{
    DbgMsg("CHDMIStereoModeHelper::UpdateContentFormat Width=%d, Height=%d, refreshRate=%d", uWidth, uHeight, uRefreshRate);

    m_ContentFormat.uWidth = uWidth;
    m_ContentFormat.uHeight = uHeight;
    m_ContentFormat.uRefreshRate = uRefreshRate;

    return S_OK;
}

HRESULT CHDMIStereoModeHelper::GetContentFormat(UINT *puWidth, UINT *puHeight, UINT *puRefreshRate)
{
    DbgMsg("CHDMIStereoModeHelper::GetContentFormat Width=%d, Height=%d, refreshRate=%d", m_ContentFormat.uWidth, m_ContentFormat.uHeight, m_ContentFormat.uRefreshRate);

    CHECK_POINTER(puWidth);
    CHECK_POINTER(puHeight);
    CHECK_POINTER(puRefreshRate);

    (*puWidth) = m_ContentFormat.uWidth;
    (*puHeight) = m_ContentFormat.uHeight;
    (*puRefreshRate) = m_ContentFormat.uRefreshRate;

    return S_OK;
}
