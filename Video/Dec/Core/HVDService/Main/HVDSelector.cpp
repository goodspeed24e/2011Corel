#include "stdafx.h"
#include <windows.h>
#include <d3d9.h>
#include <initguid.h>
#include <dxva.h>
#include <dxva2api.h>
#include <ddraw.h>
#include "HVDGuids.h"
#include "HVDSelector.h"
#include "Imports/LibGPU/UtilGPU.h"

#define DXVA2_DEFAULT_BUFFER_NUM_MAX 12
#define DXVA2_DEFAULT_SMALLBUFFER_NUM_MAX 16
#define DXVA2_DEFAULT_BUFFER_NUM_MIN 5
#define DXVA2_DEFAULT_BUFFER_NUM_MAX_128 8

#define DXVA1_MEMORY_THRESHLOD 192
#define DXVA1_DEFAULT_BUFFER_NUM_MAX DXVA2_DEFAULT_BUFFER_NUM_MAX
#define DXVA1_DEFAULT_BUFFER_NUM_MIN DXVA2_DEFAULT_BUFFER_NUM_MAX_128

using namespace HVDService;

const GUID CHVDSelector::m_sOthers[] =
{
    DXVA_ATIDRM_Profile
};

// DXVA2 MPEG2
const GUID CHVDSelector::m_sDxva2MPEG2MC[] = 
{
    DXVA2_ModeMPEG2_MoComp,
};
const GUID CHVDSelector::m_sDxva2MPEG2IDCT[] = 
{
    DXVA2_ModeMPEG2_IDCT,
};
const GUID CHVDSelector::m_sDxva2MPEG2VLD[] = 
{
    DXVA2_ModeMPEG2_VLD,
};

// DXVA2 VC1
const GUID CHVDSelector::m_sDxva2VC1MC[] = 
{
    DXVA2_ModeVC1_B,
    DXVA2_ModeVC1_A, // ??
};
const GUID CHVDSelector::m_sDxva2VC1IDCT[] = 
{
    DXVA2_ModeVC1_C,
};
const GUID CHVDSelector::m_sDxva2VC1VLD[] = 
{
    DXVADDI_Intel_ModeVC1_D_2ndCtxt,
    DXVA2_Intel_ModeVC1_D,
    DXVA2_ModeVC1_D,
};

// DXVA2 H264
const GUID CHVDSelector::m_sDxva2H264MC[] = 
{
    DXVA_ModeH264_ATI_A,
    DXVADDI_Intel_ModeH264_A,
    DXVA2_ModeH264_A,
};
const GUID CHVDSelector::m_sDxva2H264IDCT[] = 
{
    DXVADDI_Intel_ModeH264_C,
    DXVA2_ModeH264_C,
};
const GUID CHVDSelector::m_sDxva2H264VLD[] = 
{
    DXVADDI_Intel_ModeH264_E_2ndCtxt,
    DXVADDI_Intel_ModeH264_E,
    DXVA_ModeH264_E,
};

const GUID CHVDSelector::m_sDxva2H264MVCVLD[] = 
{
    DXVA_ModeH264_MVC,
    DXVADDI_Intel_ModeH264_E_2ndCtxt,
    DXVADDI_Intel_ModeH264_E,
    DXVA_ModeH264_E,
};

// DXVA2 MPEG4
const GUID CHVDSelector::m_sDxva2MP4VLD[] = 
{
    DXVA_ModeMPEG4_VLD,
    AMD_MPEG4_PART2_ASP,
};

/*
// DXVA MPEG2
const GUID CHVDSelector::m_sDxvaMPEG2MC[] = 
{
};
const GUID CHVDSelector::m_sDxvaMPEG2IDCT[] = 
{
};
const GUID CHVDSelector::m_sDxvaMPEG2VLD[] = 
{
};

// DXVA2 VC1
const GUID CHVDSelector::m_sDxvaVC1MC[] = 
{
};
const GUID CHVDSelector::m_sDxvaVC1IDCT[] = 
{
};
const GUID CHVDSelector::m_sDxvaVC1VLD[] = 
{
};

// DXVA2 H264
const GUID CHVDSelector::m_sDxvaH264MC[] = 
{
};
const GUID CHVDSelector::m_sDxvaH264IDCT[] = 
{
};
const GUID CHVDSelector::m_sDxvaH264VLD[] = 
{
};

// DXVA1 GUID array
const GUID* CHVDSelector::m_sDxvaMPEG2[] = 
{
CHVDSelector::m_sDxvaMPEG2MC,
CHVDSelector::m_sDxvaMPEG2IDCT,
CHVDSelector::m_sDxvaMPEG2VLD,
};
const GUID* CHVDSelector::m_sDxvaVC1[] = 
{
CHVDSelector::m_sDxvaVC1MC,
CHVDSelector::m_sDxvaVC1IDCT,
CHVDSelector::m_sDxvaVC1VLD,
};
const GUID* CHVDSelector::m_sDxvaH264[] = 
{
CHVDSelector::m_sDxvaVC1MC,
CHVDSelector::m_sDxvaVC1IDCT,
CHVDSelector::m_sDxvaVC1VLD,
};
const GUID** CHVDSelector::m_sDxva[] = 
{
CHVDSelector::m_sDxvaMPEG2,
CHVDSelector::m_sDxvaVC1,
CHVDSelector::m_sDxvaH264,
};
*/

HRESULT CHVDSelector::GetHVDGuids(DWORD dwService, DWORD dwMode, DWORD dwLevel, const GUID** ppGuids, DWORD *pdwCount)
{
    HRESULT hr = E_INVALIDARG;

    if (HVD_LEVEL_AUTO == dwLevel)
        return E_INVALIDARG;

    if (HVD_SERVICE_DXVA1 == dwService)
    {
    }
    else if (HVD_SERVICE_DXVA2 == dwService)
    {
        switch(dwMode)
        {
        case HVD_MODE_MPEG1:
        case HVD_MODE_MPEG2:
            if (HVD_LEVEL_MC == dwLevel)
            {
                *ppGuids = m_sDxva2MPEG2MC;
                *pdwCount = (DWORD)sizeof(m_sDxva2MPEG2MC) / sizeof(GUID);
            }
            else if (HVD_LEVEL_IDCT == dwLevel)
            {
                *ppGuids = m_sDxva2MPEG2IDCT;
                *pdwCount = (DWORD)sizeof(m_sDxva2MPEG2IDCT) / sizeof(GUID);
            }
            else if (HVD_LEVEL_VLD ==  dwLevel)
            {
                *ppGuids = m_sDxva2MPEG2VLD;
                *pdwCount = (DWORD)sizeof(m_sDxva2MPEG2VLD) / sizeof(GUID);
            }
            break;
        case HVD_MODE_VC1:
            if (HVD_LEVEL_MC == dwLevel)
            {
                *ppGuids = m_sDxva2VC1MC;
                *pdwCount = (DWORD)sizeof(m_sDxva2VC1MC) / sizeof(GUID);
            }
            else if (HVD_LEVEL_IDCT == dwLevel)
            {
                *ppGuids = m_sDxva2VC1IDCT;
                *pdwCount = (DWORD)sizeof(m_sDxva2VC1IDCT) / sizeof(GUID);
            }
            else if (HVD_LEVEL_VLD ==  dwLevel)
            {
                *ppGuids = m_sDxva2VC1VLD;
                *pdwCount = (DWORD)sizeof(m_sDxva2VC1VLD) / sizeof(GUID);
            }
            break;
        case HVD_MODE_H264:
            if (HVD_LEVEL_MC == dwLevel)
            {
                *ppGuids = m_sDxva2H264MC;
                *pdwCount = (DWORD)sizeof(m_sDxva2H264MC) / sizeof(GUID);
            }
            else if (HVD_LEVEL_IDCT == dwLevel)
            {
                *ppGuids = m_sDxva2H264IDCT;
                *pdwCount = (DWORD)sizeof(m_sDxva2H264IDCT) / sizeof(GUID);
            }
            else if (HVD_LEVEL_VLD ==  dwLevel)
            {
                *ppGuids = m_sDxva2H264VLD;
                *pdwCount = (DWORD)sizeof(m_sDxva2H264VLD) / sizeof(GUID);
            }
            else if(HVD_LEVEL_AUTO_CONSTRAIN_SURFACE_NUM ==  dwLevel)
            {
                *ppGuids = m_sDxva2H264MVCVLD;
                *pdwCount = (DWORD)sizeof(m_sDxva2H264MVCVLD) / sizeof(GUID);
            }
            break;
        case HVD_MODE_MPEG4:
            if (HVD_LEVEL_VLD ==  dwLevel)
            {
                *ppGuids = m_sDxva2MP4VLD;
                *pdwCount = (DWORD)sizeof(m_sDxva2MP4VLD) / sizeof(GUID);
            }
            break;
        case HVD_MODE_OTHERS:
            *ppGuids = m_sOthers;
            *pdwCount = sizeof(m_sOthers) / sizeof(GUID);
            break;
        }

        hr = S_OK;
    }

    return hr;
}

HRESULT CHVDSelector::RecommendSurfaceCount(DWORD dwService, DWORD dwMode, DWORD dwLevel, DWORD *pdwMax, DWORD *pdwMin, DWORD dwWidth, DWORD dwHeight)
{
    CHECK_POINTER(pdwMax);
    CHECK_POINTER(pdwMin);

    DWORD dwCaps = DDSCAPS_LOCALVIDMEM|DDSCAPS_HWCODEC;
    DWORD dwLocalMB = 0, dwFreeMB = 0;
    HRESULT hr = CUtilGPU::QueryVideoMemorySize(dwCaps, &dwLocalMB, &dwFreeMB);

    if (HVD_SERVICE_DXVA1 == dwService)
    {
        if (SUCCEEDED(hr))
        {
            if (dwLocalMB > DXVA1_MEMORY_THRESHLOD)
            {
                *pdwMax = DXVA1_DEFAULT_BUFFER_NUM_MAX;
                *pdwMin = DXVA1_DEFAULT_BUFFER_NUM_MAX;
            }
            else
            {
                *pdwMax = DXVA1_DEFAULT_BUFFER_NUM_MIN;
                *pdwMin = DXVA1_DEFAULT_BUFFER_NUM_MIN;
            } 
        }
        else
        {
            *pdwMax = DXVA1_DEFAULT_BUFFER_NUM_MIN;
            *pdwMin = DXVA1_DEFAULT_BUFFER_NUM_MIN;
        }
    }
    else if (HVD_SERVICE_DXVA2 == dwService)
    {
		if (*pdwMax == 0)
		{		
        //Bug#100341
        //Create max 16 surface for small size case.
        if(dwHeight*dwWidth > 345600) // > 720*480
           *pdwMax = DXVA2_DEFAULT_BUFFER_NUM_MAX;
         else
            *pdwMax = DXVA2_DEFAULT_SMALLBUFFER_NUM_MAX;
		}

        *pdwMin = DXVA2_DEFAULT_BUFFER_NUM_MIN;
        if (SUCCEEDED(hr))
        {
			if ((dwLocalMB < 128) & (*pdwMax == 0))
                *pdwMax	= DXVA2_DEFAULT_BUFFER_NUM_MAX_128;
        }
    }

    return S_OK;
}
