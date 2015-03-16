#pragma warning(disable:4018)  // warning C4018: '<' : signed/unsigned mismatch

#include "stdafx.h"
#include <windows.h>
#include <streams.h>
#include <dvdmedia.h>
#include <d3d9.h>
#include <vmr9.h>
#include "HVDDxva1Binder.h"
#include <dxva.h>
#include <atlbase.h>
#include "HVDGuids.h"
#include "Exports/Inc/HVDService.h"
#include "HVDDSOutPin.h"
#include "HVDDSAMVANotify.h"
#include "HVDDSVPConfig.h"
#include "HVDDSMedia.h"

using namespace HVDService;

AMOVIESETUP_MEDIATYPE CHVDDxva1Binder::m_pVidOutType[] =
{	
    {
		&MEDIATYPE_Video,
		&DXVA_ModeH264_E
	},
    {
        &MEDIATYPE_Video,
        &DXVA_ModeH264_VP1
    },
	{
		&MEDIATYPE_Video,
		&DXVA_ATI_BA_H264
	},
	{
		&MEDIATYPE_Video,
		&DXVA_ModeH264_ATI_A
	},
// 	{
// 		&MEDIATYPE_Video,
// 		&DXVA_ModeVC1_A
// 	},
	{
		&MEDIATYPE_Video,
		&DXVA_ModeVC1_D
	},
	{
		&MEDIATYPE_Video,
		&DXVA_ModeVC1_C
	},
	{
		&MEDIATYPE_Video,
		&DXVA_ModeVC1_B
	},
	{
		&MEDIATYPE_Video,
		&DXVA2_Intel_ModeVC1_D
	},
	{
		&MEDIATYPE_Video,
		&DXVA_ModeMPEG2_VLD
	},
    {
        &MEDIATYPE_Video,
        &DXVA_ModeMPEG2_B
    },
    {
        &MEDIATYPE_Video,
        &DXVA_ModeMPEG2_D
    },
    {
        &MEDIATYPE_Video,
        &DXVA_ModeMPEG2_A
    },
    {
        &MEDIATYPE_Video,
        &DXVA_ModeMPEG2_C
    },
};

AMOVIESETUP_MEDIATYPE CHVDDxva1Binder::m_pSubOutType[] =
	{
#if 0
		{
		&MEDIATYPE_Video,
		&MEDIASUBTYPE_AI44
		},
		{
		&MEDIATYPE_Video,
		&MEDIASUBTYPE_IA44
		},
#endif
		{
		&MEDIATYPE_Video,
		&MEDIASUBTYPE_AYUV
		},
#if 0
		{
		&MEDIATYPE_Video,
		&MEDIASUBTYPE_ARGB4444
		},
#endif
		{
		&MEDIATYPE_Video,
		&MEDIASUBTYPE_ARGB32
		},
		{
		&MEDIATYPE_Video,
		&MEDIASUBTYPE_RGB32
		},
	};

const AMOVIESETUP_PIN CHVDDxva1Binder::m_pPinInfo[] =
{
    {
        VIDEO_VIDOUT_LNAME,     // Pin's string name
        FALSE,      			// Is it rendered
        TRUE,      				// Is it an output
        FALSE,      			// Allowed none
        FALSE,      			// Allowed many
        &CLSID_NULL,      		// Connects to filter
        NULL,      				// Connects to pin
        sizeof(m_pVidOutType) / sizeof(AMOVIESETUP_MEDIATYPE),
        CHVDDxva1Binder::m_pVidOutType	// Pin information
    },
	{
		VIDEO_SUBOUT_LNAME,		// Pin's string name
		FALSE,				// Is it rendered
		TRUE,				// Is it an output
		FALSE,				// Allowed none
		FALSE,				// Allowed many
		&CLSID_NULL,			// Connects to filter
		NULL,			// Connects to pin
		sizeof(m_pSubOutType)/sizeof(AMOVIESETUP_MEDIATYPE),
		CHVDDxva1Binder::m_pSubOutType	// Pin information
	}
};

AMOVIESETUP_FILTER CHVDDxva1Binder::m_FilterInfo =
{
    &GUID_NULL,      				// clsID
    VIDEO_FILTER_LNAME,      		// strName
    VIDEO_FILTER_MERIT,
    sizeof(m_pPinInfo) / sizeof(AMOVIESETUP_PIN),
    CHVDDxva1Binder::m_pPinInfo		// lPin
};

CHVDDxva1Binder::CHVDDxva1Binder(): CBaseFilter(_T("DXVA Binder"), NULL, &m_CritSec, *(&m_FilterInfo)->clsID),
//: CBasefilter("DXVA binder", NULL, &m_FilterInfo)
m_OutputPinList(NAME("CHVDDxva1Binder::m_OutputPinList"))
{
	m_NumOutputPins = 0;		
	m_NextOutputPinNumber = 0;
	ULONG ulref = NULL;

    m_pVidOut = CreateNextOutputPin(VIDEO_VIDOUT_NAME, m_pPinInfo);
    if (m_pVidOut)
    {
		m_pVidOut->AddRef();

		CComPtr<IUnknown> pUnkOuter;

        if (SUCCEEDED(m_pVidOut->QueryInterface(IID_IUnknown, (void **)&pUnkOuter)))
        {
            m_pVidOut->AddObject(new CHVDDSAMVANotify(m_pVidOut, pUnkOuter));
            m_pVidOut->AddObject(new CHVDDSVPConfig(&m_CritSec, pUnkOuter));
        }
    }

	m_pSubOut = CreateNextOutputPin(VIDEO_SUBOUT_NAME, m_pPinInfo + 1);
	if (m_pSubOut)
		m_pSubOut->AddRef();
    
	m_width_vid = m_height_vid = 0;
	m_height_spic = m_width_spic = 0;
	m_dwDecoderId = HVDService::HVD_MODE_UNKNOWN; 
	m_pVideoRender = NULL;
	m_dwVideoOutputBufCount = 6;
	m_dwSubOutputBufCount = 2;
    m_rtAvgTimeIn = 0;
	m_rtStartTime = 0;
	m_InterlaceSample = 1;//GetRegInt("INTERLACESAMPLE", 1);
}

CHVDDxva1Binder::~CHVDDxva1Binder()
{
	Destroy();
}

HRESULT CHVDDxva1Binder::GetMediaType(CBasePin *pPin, int pos, CMediaType *pmt)
{
    HRESULT hr = S_OK;

    if (pPin == m_pVidOut)
    {
        BOOL bDXVA, bMPEG2, bMPEG4, bVC1, bH264, bDXVA_h264, bDXVA_VC1;

        // Check input type
		bMPEG2 = (m_dwDecoderId == HVDService::HVD_MODE_MPEG1 || m_dwDecoderId == HVDService::HVD_MODE_MPEG2);
        bMPEG4 = (m_dwDecoderId == HVDService::HVD_MODE_MPEG4);
        bVC1 = (m_dwDecoderId == HVDService::HVD_MODE_VC1);
        bH264 = (m_dwDecoderId == HVDService::HVD_MODE_H264);

        // Check output type
        bDXVA = CHVDDSMedia::IsDXVA(*pmt->Subtype());				
		bDXVA_h264 = CHVDDSMedia::IsDXVAH264(*pmt->Subtype());
		bDXVA_VC1 = CHVDDSMedia::IsDXVAVC1(*pmt->Subtype());

        if (bDXVA)
        {
            if (!bMPEG2 )
                return S_FALSE;	// S_FALSE: non-mpeg2 input
        }
		if (bDXVA_h264)
		{
            if (!bH264 )
                return S_FALSE;	// S_FALSE: non-H264 input
		}
		if (bDXVA_VC1)
		{
			if (!bVC1 )
				return S_FALSE;	// S_FALSE: non-VC1 input
		}

        pmt->SetFormatType(&FORMAT_VideoInfo2);
        hr = SetVideoInfo(m_width_vid, m_height_vid, pmt, pPin, pos);
    }
	else if (pPin==m_pSubOut)
	{
		pmt->SetFormatType(&FORMAT_VideoInfo2);
		hr = SetVideoInfo(m_width_spic, m_height_spic, pmt, pPin, pos);
	}
    return hr;
}

HRESULT CHVDDxva1Binder::DecideBufferSize(CBasePin *pPin, IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pProperties)
{
	if (pPin == m_pVidOut)
	{
		LPBITMAPINFOHEADER lpbi;
		CMediaType cmt;

		//        ((CHVDDSOutPin *) pPin)->CurrentMediaType(&cmt);
		pPin->GetMediaType(0, &cmt);
		CHVDDSMedia::GetMediaPtrs(&cmt, &lpbi, 0, 0);
		if (lpbi)
			pProperties->cbBuffer = GetBitmapSize(lpbi);
		else
			pProperties->cbBuffer = m_width_vid * m_height_vid * 4;
		pProperties->cBuffers = m_dwVideoOutputBufCount; //GetRegInt(TEXT("DsVideoBuf"), 6);
		pProperties->cbAlign = 8;
		pProperties->cbAlign = 1;
		pProperties->cbPrefix = 0;
	}
	else if (pPin==m_pSubOut)
	{
		LPBITMAPINFOHEADER lpbi;
		CMediaType cmt;		
		//		((CHVDDSOutPin *) pPin)->CurrentMediaType(&cmt);
		pPin->GetMediaType(0, &cmt);
		CHVDDSMedia::GetMediaPtrs(&cmt, &lpbi, 0, 0);
		if (lpbi)
			pProperties->cbBuffer = GetBitmapSize(lpbi);
		else
			pProperties->cbBuffer = m_width_spic*m_height_spic*4;
		pProperties->cBuffers = m_dwSubOutputBufCount;//GetRegInt(TEXT("DsSpicBuf"), 2);	// overlay mixer provides only 1 buffer
		pProperties->cbAlign = 1;	// overlay mixer alignment must be 1
		pProperties->cbPrefix = 0;
	}
	return S_OK;
}

HRESULT CHVDDxva1Binder::CompleteConnect(CBasePin *pPin, IPin *pReceivePin)
{
	return S_OK;
}

HRESULT CHVDDxva1Binder::BreakConnect(CBasePin *pPin)
{
	return S_OK;
}

int CHVDDxva1Binder::GetPinCount()
{
	return 2;
}

CBasePin* CHVDDxva1Binder::GetPin(int n)
{
	if (n < 0)
		return NULL;
	else if (n == 0)
		return m_pVidOut;
	else if (n == 1)
		return m_pSubOut;
	else
		return NULL;
}

STDMETHODIMP CHVDDxva1Binder::Run(REFERENCE_TIME tStart)
{
	HRESULT hr = CBaseFilter::Run(tStart);
	if (SUCCEEDED(hr))
	{
		m_rtStartTime = tStart;
	}
	return hr;
}

STDMETHODIMP CHVDDxva1Binder::Pause()
{
	HRESULT hr = CBaseFilter::Pause();
	return hr;
}

STDMETHODIMP CHVDDxva1Binder::Stop()
{
	HRESULT hr = CBaseFilter::Stop();
	if (SUCCEEDED(hr))
	{
		m_rtStartTime = 0;
	}
	return hr;
}

CHVDDSOutPin *CHVDDxva1Binder::CreateNextOutputPin(TCHAR *pName, const AMOVIESETUP_PIN *pPinInfo, LPWSTR pPinName)
{
	CAutoLock lock_it(m_pLock);
	WCHAR szbuf[20];             // Temporary scratch buffer

	m_NextOutputPinNumber++;     // Next number to use for pin
	if(pName==0)
		pName = TEXT("DShow filter Output");
	if((pPinInfo==0 || pPinInfo->strName==0) && pPinName==0)
	{
		pPinName = szbuf;
		swprintf_s(pPinName, 20, L"Output%d", m_NextOutputPinNumber);
	}
	CHVDDSOutPin *pPin = new CHVDDSOutPin(pName, this, pPinInfo, pPinName);
	if(pPin==NULL) 
	{
		m_NextOutputPinNumber--;
		return NULL;
	}
	m_NumOutputPins++;
	IncrementPinVersion();
	//pPin->AddRef();
	return pPin;
}

CHVDDSOutPin* CHVDDxva1Binder::GetOutputPin(int n)
{
	if (n < 0)
		return NULL;
	else if (n == 0)
		return m_pVidOut;
	else if (n == 1)
		return m_pSubOut;
	else
		return NULL;
}

HRESULT CHVDDxva1Binder::GetStartTime(REFERENCE_TIME *pStartTime)
{
	if (m_State == State_Stopped || m_rtStartTime == 0)
		return E_FAIL;
	*pStartTime = m_rtStartTime;

	return S_OK;
}

void CHVDDxva1Binder::SetVideoDecodeInfo(int width, int height, HVDService::HVD_MODE dwDecoderId)
{
	m_width_vid = width;
	m_height_vid = height;
	m_width_spic  = width;
	m_height_spic = height;
	m_dwDecoderId = dwDecoderId;
}

HRESULT CHVDDxva1Binder::SetVideoRender(IBaseFilter *pVideoRender)
{
	CHECK_POINTER(pVideoRender)

		SAFE_RELEASE(m_pVideoRender);

	m_pVideoRender = pVideoRender;
	if (m_pVideoRender)
		m_pVideoRender->AddRef();

	return S_OK;
}

HRESULT CHVDDxva1Binder::SetUncompSurfacesConfig(HVDService::HVDDxva1UncompSurfConfig *pUncompSurfacesConfig)
{
	IHVDDXVA1AMVANofityConfig *pAMVANotifyConfig;
	HRESULT hr = m_pVidOut->QueryInterface(IID_IHVDDXVA1AMVANofityConfig, (void **)&pAMVANotifyConfig);
	if (SUCCEEDED(hr))
	{
		pAMVANotifyConfig->SetUncompSurfacesConfig(pUncompSurfacesConfig);
		pAMVANotifyConfig->Release();
	}
	return hr;
}

HRESULT CHVDDxva1Binder::SetOutputPinBufferCount(int nPinID, DWORD dwBufCount)
{
	if (nPinID < 0 || nPinID > 1)
		return E_INVALIDARG;

	if (nPinID == 0)
	{
		m_dwVideoOutputBufCount = dwBufCount;
		return S_OK;
	}
	else if (nPinID == 1)
	{
		m_dwSubOutputBufCount = dwBufCount;
		return S_OK;
	}
	else
		return E_FAIL;
}

HRESULT CHVDDxva1Binder::Destroy()
{
	ULONG ulref;

	if (m_pVidOut)
	{
		m_pVidOut->RemoveAllObject();
		ulref = m_pVidOut->Release();
		delete m_pVidOut;
		m_pVidOut = NULL;
	}

	if (m_pSubOut)
	{
		m_pSubOut->RemoveAllObject();
		ulref = m_pSubOut->Release();
		delete m_pSubOut;
		m_pSubOut = NULL;
	}

	if (m_pVideoRender)
	{
		ulref = m_pVideoRender->Release();
		m_pVideoRender = NULL;
	}

	return S_OK;
}

HRESULT CHVDDxva1Binder::SetVideoInfo(int iWidth, int iHeight, CMediaType *pmt, CBasePin *pPin, int pos)
{
	LPBITMAPINFOHEADER lpbi;
	DWORD *pdwBitfields;
	REFGUID guid = *pmt->Subtype();
	VIDEOINFOHEADER *pVideoInfo;
	LARGE_INTEGER li;

	if (*pmt->FormatType() == FORMAT_VideoInfo2)
	{
		VIDEOINFOHEADER2 *pVideoInfo2 = (VIDEOINFOHEADER2 *)pmt->ReallocFormatBuffer(sizeof(VIDEOINFOHEADER2) + SIZE_PALETTE);
		if (pVideoInfo2 == 0)
			return E_OUTOFMEMORY;
		ZeroMemory(pVideoInfo2, sizeof(VIDEOINFOHEADER2));
		SetRect(&pVideoInfo2->rcSource, 0, 0, 0, 0);
		SetRect(&pVideoInfo2->rcTarget, 0, 0, iWidth, iHeight);
		pVideoInfo2->dwPictAspectRatioX = 4;
		pVideoInfo2->dwPictAspectRatioY = 3;
		if(pPin!=m_pSubOut && m_InterlaceSample)
			pVideoInfo2->dwInterlaceFlags = AMINTERLACE_IsInterlaced | AMINTERLACE_DisplayModeBobOrWeave;  
		lpbi = &pVideoInfo2->bmiHeader;
		lpbi->biSize = sizeof(BITMAPINFOHEADER); // must be before BITMASKS() macro
		pdwBitfields = BITMASKS(pVideoInfo2);
		pVideoInfo2->AvgTimePerFrame = m_rtAvgTimeIn ? m_rtAvgTimeIn : 333667; // 100-ns unit, default to 30 Hz
		if (guid == MEDIASUBTYPE_IA44 || guid == MEDIASUBTYPE_AI44)
			memset(COLORS(pVideoInfo2), 0, 64);	// set palette to zero
	}
	else
		return E_FAIL;
	lpbi->biHeight = iHeight;
	lpbi->biWidth = iWidth;
	lpbi->biPlanes = 1;
	lpbi->biXPelsPerMeter = 0;
	lpbi->biYPelsPerMeter = 0;
	if (guid == MEDIASUBTYPE_RGB8)
	{
		lpbi->biClrUsed = 1;
		lpbi->biBitCount = 8;
		lpbi->biCompression = BI_RGB;
	}
	else if (guid == MEDIASUBTYPE_AI44)
	{
		lpbi->biClrUsed = 16;
		lpbi->biBitCount = 8;
		lpbi->biCompression = MAKEFOURCC('A','I','4','4');
	}
	else if (guid == MEDIASUBTYPE_IA44)
	{
		lpbi->biClrUsed = 16;
		lpbi->biBitCount = 8;
		lpbi->biCompression = MAKEFOURCC('I','A','4','4');
	}
	else if (guid == MEDIASUBTYPE_ARGB4444)
	{
		lpbi->biPlanes      = 1;
		lpbi->biBitCount	= 16;
		lpbi->biCompression = BI_BITFIELDS;
		pdwBitfields[0] = 0x0f00;	// red mask
		pdwBitfields[1] = 0x00f0;	// green mask
		pdwBitfields[2] = 0x000f;	// blue mask
	}
	else if (guid == MEDIASUBTYPE_ARGB32)
	{
		lpbi->biPlanes      = 1;
		lpbi->biBitCount	= 32;
		lpbi->biCompression = BI_BITFIELDS;
		pdwBitfields[0] = 0x0f00;	// red mask
		pdwBitfields[1] = 0x00f0;	// green mask
		pdwBitfields[2] = 0x000f;	// blue mask
	}
	else if (guid == MEDIASUBTYPE_RGB555)
	{
		lpbi->biBitCount	= 16;
		lpbi->biCompression = BI_BITFIELDS;
		pdwBitfields[0] = 0x7c00;	// red mask
		pdwBitfields[1] = 0x03e0;	// green mask
		pdwBitfields[2] = 0x001f;	// blue mask
	}
	else if (guid == MEDIASUBTYPE_RGB565)
	{
		lpbi->biBitCount	= 16;
		lpbi->biCompression = BI_BITFIELDS;
		pdwBitfields[0] = 0xf800;	// red mask
		pdwBitfields[1] = 0x07e0;	// green mask
		pdwBitfields[2] = 0x001f;	// blue mask
	}
	else if (guid == MEDIASUBTYPE_RGB24)
	{
		lpbi->biBitCount      = 24;
		lpbi->biCompression = BI_RGB;
	}
	else if (guid == MEDIASUBTYPE_RGB32)
	{
		lpbi->biBitCount      = 32;
		lpbi->biCompression = BI_RGB;
	}
	else if (guid == MEDIASUBTYPE_AYUV)
	{
		lpbi->biBitCount = 32;
		lpbi->biCompression = MAKEFOURCC('A','Y','U','V');
	}
	else if(CHVDDSMedia::IsDXVAVC1(guid))
	{
		lpbi->biCompression = MAKEFOURCC('D','X','V','A');
		lpbi->biBitCount	= 16;
		lpbi->biHeight		= iHeight;
	}
	else if(guid == DXVA_ModeH264_VP1)
	{
		lpbi->biBitCount = 12;
		lpbi->biCompression = MAKEFOURCC('N','V','1','2');
	}
	else if(guid == DXVA_ModeH264_ATI_A || guid == DXVA_ModeH264_ATI_B)
	{
		lpbi->biCompression = MAKEFOURCC('D','X','V','A');
		lpbi->biBitCount	= 16;
		lpbi->biHeight		= iHeight;
	}
	else if(guid == DXVA_ATI_BA_H264 || guid == DXVA_ModeH264_ATI_RESTRICTED)
	{
		lpbi->biCompression = MAKEFOURCC('D','X','V','A');
		lpbi->biBitCount	= 16;
		lpbi->biHeight		= iHeight;
	}
	else if(guid == DXVA_ModeH264_E)
	{
		lpbi->biCompression = MAKEFOURCC('D','X','V','A');
		lpbi->biBitCount	= 16;
		lpbi->biHeight		= iHeight;
	}
	else if(guid == DXVA_ModeH264_F)
	{
		lpbi->biCompression = MAKEFOURCC('D','X','V','A');
		lpbi->biBitCount	= 16;
		lpbi->biHeight		= iHeight;
	}

#if 0 //def USE_HVA
	else if (guid==MEDIASUBTYPE_I81X)
	{
		// note: this is not fourcc, but seems does not matter
		FOURCCMap fccmap(pmt->Subtype());
		lpbi->biCompression = fccmap.GetFOURCC();
		lpbi->biBitCount      = 12;
		lpbi->biHeight        = iHeight;
	}
#endif
	else if (CHVDDSMedia::IsDXVA(guid))
	{
		lpbi->biCompression =	MAKEFOURCC('D','X','V','A');
		lpbi->biBitCount =		16;
		lpbi->biHeight =		iHeight;
	}
	else
	{	// must be fourcc - can also be true guid
		FOURCCMap fccmap(pmt->Subtype());

		lpbi->biCompression = fccmap.GetFOURCC();
		if (guid == MEDIASUBTYPE_YV12)
			lpbi->biBitCount = 12;	// YV12 must set exactly otherwise problems with sink filters.
		else
			lpbi->biBitCount = 16;	// assume YUY2
		lpbi->biHeight = iHeight;
	}
	lpbi->biSizeImage = GetBitmapSize(lpbi);
	lpbi->biClrImportant = 0;
	pVideoInfo = (VIDEOINFOHEADER*)pmt->Format();
	li.QuadPart = pVideoInfo->AvgTimePerFrame;
	pVideoInfo->dwBitRate = MulDiv(GetBitmapSize(lpbi), 80000000, li.LowPart);
	pVideoInfo->dwBitErrorRate = 0L;
	pmt->SetSampleSize(lpbi->biSizeImage);
	pmt->SetTemporalCompression(FALSE);

	SetDeinterlaceMode((VIDEOINFOHEADER2 *)pmt->pbFormat);
	return S_OK;
}

HRESULT CHVDDxva1Binder::SetDeinterlaceMode(VIDEOINFOHEADER2 *h)
{
	// we need to set deinterlace mode before making a connection.
	CComPtr<IVMRDeinterlaceControl9> pDIC9;

	HRESULT hr = m_pVideoRender->QueryInterface(__uuidof(IVMRDeinterlaceControl9), (void**) &pDIC9);
	if(SUCCEEDED(hr))
	{
		//We don't specify the deinterlace mode, the VMR defaults to the first mode reported by the driver.
		//If the VMR cannot use the preferred mode, it falls back to another mode as specified in the IVMRDeinterlaceControl9::SetDeinterlacePrefs method.
		hr = pDIC9->SetDeinterlacePrefs(DeinterlacePref9_NextBest);

		if(0) //For Debug
		{
			VMR9VideoDesc vd;
			DWORD nMode;
			vd.dwSize = sizeof(VMR9VideoDesc);
			vd.dwSampleWidth = h->bmiHeader.biWidth;
			vd.dwSampleHeight = h->bmiHeader.biHeight;
			vd.SampleFormat = VMR9_SampleFieldInterleavedEvenFirst;
			vd.dwFourCC = h->bmiHeader.biCompression;
			vd.InputSampleFreq.dwNumerator = 30000;
			vd.InputSampleFreq.dwDenominator = 1001;
			vd.OutputFrameFreq.dwNumerator = 60000;
			vd.OutputFrameFreq.dwDenominator = 1001;

			hr = pDIC9->GetNumberOfDeinterlaceModes(&vd, &nMode, NULL);
			// if more than one mode to choose from
			if(SUCCEEDED(hr) && nMode > 1)
			{
				GUID *pGuid = new GUID[nMode];

				hr = pDIC9->GetNumberOfDeinterlaceModes(&vd, &nMode, pGuid);
				if(SUCCEEDED(hr))
				{
					GUID *pDeinterlaceGuid[8] = {0};
					VMR9DeinterlaceCaps Caps;

					for (DWORD i = 0; i < nMode; i++)
					{
						// if not filled with size, GetDeinterlaceModeCaps returns E_INVALIDARG
						Caps.dwSize = sizeof(VMR9DeinterlaceCaps);
						hr = pDIC9->GetDeinterlaceModeCaps(pGuid + i, &vd, &Caps);
						if (SUCCEEDED(hr))
						{
							if (Caps.DeinterlaceTechnology == DeinterlaceTech9_Unknown)
								pDeinterlaceGuid[0] = pGuid + i;
							else if (Caps.DeinterlaceTechnology == DeinterlaceTech9_MotionVectorSteered)
								pDeinterlaceGuid[1] = pGuid + i;
							else if (Caps.DeinterlaceTechnology == DeinterlaceTech9_PixelAdaptive)
								pDeinterlaceGuid[2] = pGuid + i;
							else if (Caps.DeinterlaceTechnology == DeinterlaceTech9_FieldAdaptive)
								pDeinterlaceGuid[3] = pGuid + i;
							else if (Caps.DeinterlaceTechnology == DeinterlaceTech9_EdgeFiltering)
								pDeinterlaceGuid[4] = pGuid + i;
							else if (Caps.DeinterlaceTechnology == DeinterlaceTech9_MedianFiltering)
								pDeinterlaceGuid[5] = pGuid + i;
							else if (Caps.DeinterlaceTechnology == DeinterlaceTech9_BOBVerticalStretch)
								pDeinterlaceGuid[6] = pGuid + i;
							else if (Caps.DeinterlaceTechnology == DeinterlaceTech9_BOBLineReplicate)
								pDeinterlaceGuid[7] = pGuid + i;
						}
					}

					for (int i = 0; i < 8 ; i++)
					{
						if(pDeinterlaceGuid[i] != NULL)
						{
							hr = pDIC9->SetDeinterlaceMode(0, pDeinterlaceGuid[i]);
							if (SUCCEEDED(hr))
								break;
						}
					}

					GUID actual;
					hr = pDIC9->GetDeinterlaceMode(0, &actual);
					hr = pDIC9->GetDeinterlaceModeCaps(&actual, &vd, &Caps);
					hr = pDIC9->GetActualDeinterlaceMode(0, &actual);
					hr = pDIC9->GetDeinterlaceModeCaps(&actual, &vd, &Caps);
				}
				delete [] pGuid;
			}
		}
	}
	return hr;
}


