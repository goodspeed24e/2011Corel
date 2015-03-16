#include "stdafx.h"
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
   m_eCurrentDispFrameRateMode = DISPFRAMERATE_NONE;
   m_dwDefaultMaxDisplayFrameRate = m_dwMaxDisplayFrameRate = 60;
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
                BOOL bIsGeneralD3DMode = FALSE;
                GUID guidResID;
                hr = CResourceManager::GetInstance()->GetActiveResrouceGUID(DISPSVR_RESOURCE_VIDEOMIXER, &guidResID);
                if (SUCCEEDED(hr))
                {
                    if (IsEqualGUID(guidResID, DISPSVR_RESOURCE_INTELFASTCOMPVIDEOMIXER))
                        m_dwMaxDisplayFrameRate = 60;
                    else
                        m_dwMaxDisplayFrameRate = Caps.dwFPS;
                }
            }
        }

        PresenterProperty PresenterProp = {0};
		PresenterProp.dwSize = sizeof(PresenterProperty);
		hr = pDispSvrVideoPresenter->GetProperty(&PresenterProp);
		if (SUCCEEDED(hr))
		{
			PresenterProp.dwFlags &= ~PRESENTER_PROPERTY_WAITUNTILPRESENTABLE;
			hr = pDispSvrVideoPresenter->SetProperty(&PresenterProp);
		}
    }

    m_llMinDisplayDuration = 10000000LL/(m_dwMaxDisplayFrameRate-1);

	SetDispFrameRateMode(m_dwMaxDisplayFrameRate);

	m_eDefaultDispFrameRateMode = m_eCurrentDispFrameRateMode;
	m_dwDefaultMaxDisplayFrameRate = m_dwMaxDisplayFrameRate;

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

HRESULT CVideoSourceDisplayHelper::ApplyDispFrameRateMode(DWORD dwFrameRate)
{
	DISPFRAMERATE_TYPE eDispFrameRateMode = m_eCurrentDispFrameRateMode;

	SetDispFrameRateMode(dwFrameRate);

	if (m_eCurrentDispFrameRateMode == DISPFRAMERATE_UNKNOWN)
	{
		return E_FAIL;
	}
	else if (m_eCurrentDispFrameRateMode == eDispFrameRateMode)
	{
		return S_OK;
	}
	else if (m_eCurrentDispFrameRateMode == DISPFRAMERATE_NONE)
	{
		RestoreFrameRate();
		return S_OK;
	}
	else
	{
		SetDispFrameRate(dwFrameRate);
		return S_OK;
	}
}

void  CVideoSourceDisplayHelper::SetDispFrameRate(DWORD dwFrameRate)
{
	m_dwMaxDisplayFrameRate = dwFrameRate;
	m_llMinDisplayDuration  = 10000000LL/(m_dwMaxDisplayFrameRate-1);
}

void  CVideoSourceDisplayHelper::RestoreFrameRate()
{
	m_dwMaxDisplayFrameRate = m_dwDefaultMaxDisplayFrameRate;
	m_llMinDisplayDuration  = 10000000LL/(m_dwMaxDisplayFrameRate-1);
}

void CVideoSourceDisplayHelper::SetDispFrameRateMode(DWORD dwDispFrameRate)
{
	switch (dwDispFrameRate)
	{
	case 0:
		m_eCurrentDispFrameRateMode = DISPFRAMERATE_NONE;
		break;
	case 24:
		m_eCurrentDispFrameRateMode = DISPFRAMERATE_24FPS;
		break;
	case 25:      
		m_eCurrentDispFrameRateMode = DISPFRAMERATE_25FPS;
		break;
	case 30:   
		m_eCurrentDispFrameRateMode = DISPFRAMERATE_30FPS;
		break;
	case 48:   
		m_eCurrentDispFrameRateMode = DISPFRAMERATE_48FPS;
		break;
	case 50:   
		m_eCurrentDispFrameRateMode = DISPFRAMERATE_50FPS;
		break;
	case 60:   
		m_eCurrentDispFrameRateMode = DISPFRAMERATE_60FPS;
		break;
	case 120:   
		m_eCurrentDispFrameRateMode = DISPFRAMERATE_120FPS;
		break;
	default:
		m_eCurrentDispFrameRateMode = DISPFRAMERATE_UNKNOWN;
		break;
	}
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