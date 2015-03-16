#include "stdafx.h"
#include "ResourceManager.h"
#include "Exports/Inc/VideoMixer.h"
#include "Exports/Inc/VideoPresenter.h"
#include "VideoSourcePropertyExtension.h"

using namespace DispSvr;

// At 2X, 4X, we may receive samples at 120Hz, 240Hz
// It does not make sense to display them all and some of the presenters can't handle
// display frequency so high. It is easier to drop the exceeding samples by not presenting
// all of the samples.

// (Duration - Threshold) must be lower than 4ms due to the duration might be 4ms while playing 240hz
//Therefore set 13ms for 60fps, and 30ms for 30fps
#define INITIATIVE_RENDERING_SAMPLE_DURATION_THRESHOLD_60FPS (130000LL)
#define INITIATIVE_RENDERING_SAMPLE_DURATION_THRESHOLD_30FPS  (300000LL)
#define VSYNC_DELAY_BOUND (333333LL) //use 10000000 / 30 vsync time, not 10000000 / 29.97

CVideoSourceDisplayHelper::CVideoSourceDisplayHelper()
{
    m_dwMaxDisplayFrameRate = 60;
    m_llMinDisplayDuration = 10000000LL/(m_dwMaxDisplayFrameRate-1);
}

CVideoSourceDisplayHelper::~CVideoSourceDisplayHelper()
{
}

HRESULT CVideoSourceDisplayHelper::UpdateMaxDisplayFrameRate(UINT uImageWidth, UINT uImageHeight)
{
    HRESULT hr = E_FAIL;
    CComPtr<IDispSvrVideoPresenter> pDispSvrVideoPresenter;

    hr = CResourceManager::GetInstance()->GetInterface(DISPSVR_RESOURCE_VIDEOPRESENTER, __uuidof(IDispSvrVideoPresenter), (void**)&pDispSvrVideoPresenter);
    if (SUCCEEDED(hr))
    {
        PresenterCaps Caps = {0};
        Caps.dwSize = sizeof(PresenterCaps);

        Caps.VideoDecodeCaps = VIDEO_CAP_CODEC_MPEG2; //default value
        if (uImageHeight > 720) // 1920x1080 and 1440x1080
        {
            Caps.VideoDecodeCaps |= VIDEO_CAP_FORMAT_1080;
        }
        else if (uImageHeight > 576) //1280x720
        {
            Caps.VideoDecodeCaps |= VIDEO_CAP_FORMAT_720;
        }
        else if (uImageHeight > 480)
        {
            Caps.VideoDecodeCaps |= VIDEO_CAP_FORMAT_576;
        }
        else
        {
            Caps.VideoDecodeCaps |= VIDEO_CAP_FORMAT_480;
        }

        hr = pDispSvrVideoPresenter->QueryCaps(&Caps);
        if (SUCCEEDED(hr))
        {
            if (Caps.bIsOverlay)
            {
                m_dwMaxDisplayFrameRate = Caps.dwFPS;
            }
            else
            {
                CComPtr<IDispSvrPlugin> pPlugIn;
                CComPtr<IDispSvrVideoMixer> pDispSvrVideoMixer;
                BOOL bIsGeneralD3DMode = TRUE;
                hr = CResourceManager::GetInstance()->GetInterface(DISPSVR_RESOURCE_VIDEOMIXER, __uuidof(IDispSvrVideoMixer), (void**)&pDispSvrVideoMixer);
                if (SUCCEEDED(hr) && pDispSvrVideoMixer)
                {
                    pDispSvrVideoMixer->QueryInterface(__uuidof(IDispSvrPlugin), (VOID **)&pPlugIn);
                    if (pPlugIn)
                    {
                        GUID guidResID;
                        if (SUCCEEDED(pPlugIn->GetResourceId(&guidResID)))
                        {
                            if (IsEqualGUID(guidResID, DISPSVR_RESOURCE_INTELFASTCOMPVIDEOMIXER))
                            {
                                bIsGeneralD3DMode = FALSE;
                            }
                        }
                    }
                }

                if (bIsGeneralD3DMode) // D3D mode, always set 60 fps since we cannot query max flip rate.
                    m_dwMaxDisplayFrameRate = 60;
                else
                    m_dwMaxDisplayFrameRate = Caps.dwFPS;
            }
        }
    }

    m_llMinDisplayDuration = 10000000LL/(m_dwMaxDisplayFrameRate-1);

    return hr;
}

BOOL CVideoSourceDisplayHelper::IsOverDisplayFrameRate(LONGLONG hnsDuration) const
{
    if (hnsDuration < 0)
        return FALSE;

    LONGLONG llThrethold = 0;
    if (m_dwMaxDisplayFrameRate == 30)
    {
        llThrethold = INITIATIVE_RENDERING_SAMPLE_DURATION_THRESHOLD_30FPS;
    }
    else
    {
        llThrethold = INITIATIVE_RENDERING_SAMPLE_DURATION_THRESHOLD_60FPS;
    }

    if (hnsDuration < llThrethold)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

BOOL CVideoSourceDisplayHelper::IsSkipFrameForVideoSync(LONGLONG hnsDuration, LONGLONG hnsDelta) const
{
    if (hnsDelta >= 0)
        return FALSE;

    //allow one vsync delay to decrease frequency of frame skip.
    LONGLONG DelayDuration = hnsDuration > VSYNC_DELAY_BOUND ? hnsDuration : VSYNC_DELAY_BOUND;

    if ((hnsDuration < m_llMinDisplayDuration) && (hnsDelta < -DelayDuration))
        return TRUE;

    return FALSE;
}

BOOL CVideoSourceDisplayHelper::IsYUVFormat(DWORD dwFormat)
{
	if (dwFormat == PLANE_FORMAT_NV12 || dwFormat == PLANE_FORMAT_NV24 ||
		dwFormat == PLANE_FORMAT_IMC3 || dwFormat == PLANE_FORMAT_YV12 ||
		dwFormat == PLANE_FORMAT_YUY2)
		return TRUE;
	else 
		return FALSE;
}