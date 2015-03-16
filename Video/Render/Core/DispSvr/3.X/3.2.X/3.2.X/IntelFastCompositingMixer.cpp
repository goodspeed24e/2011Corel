#include "StdAfx.h"
#include "IntelFastCompositingMixer.h"
#include "D3D9VideoEffect3DManager.h"
#include "DriverExtensionHelper.h"

#define MAX_BACKWARD_SAMPLES 1

using namespace DispSvr;

// ITextureFilter
class CIntelScrambleFilter : public ITextureFilter
{
public:
	CIntelScrambleFilter(D3D9Plane *pPlane) : m_pPlane(pPlane)
	{
	}

	virtual ~CIntelScrambleFilter()
	{
	}

	virtual HRESULT Process(const RECT &, const LockedRect &lockedBuffer)
	{
		DWORD dwPattern = 0;
		DWORD *pBuf;

		ASSERT(m_pPlane);
		m_pPlane->Protection.eType = PROTECTION_SCRAMBLE_XOR;
		m_pPlane->Protection.dwScrambleSeed = timeGetTime();

		BOOLEAN ret = GetScramblePattern(COREL_APPLICATION_ID, m_pPlane->Protection.dwScrambleSeed, &dwPattern);
		if (ret != 1)
			return E_FAIL;

		switch (m_pPlane->Format)
		{
		case PLANE_FORMAT_YUY2:
			{
				UCHAR *pLine = static_cast<UCHAR *> (lockedBuffer.pBuffer);
				for (UINT i = 0; i < m_pPlane->uHeight / 4; i++)
				{
					pBuf = (DWORD *) pLine;
					for (DWORD j = 0; j < m_pPlane->uWidth * 2 / 4; j++)
						*pBuf++ ^= dwPattern;
					pLine += lockedBuffer.uPitch;
				}
				m_pPlane->Protection.dwSizeScrambled = (lockedBuffer.uPitch * m_pPlane->uHeight / 4) & (~0xf);
			}
			break;
		default:
			{
				pBuf = static_cast<DWORD *> (lockedBuffer.pBuffer);
				m_pPlane->Protection.dwSizeScrambled = m_pPlane->uWidth * m_pPlane->uHeight / 2;
				for (DWORD j = 0; j < m_pPlane->Protection.dwSizeScrambled / 4; j++)
					*pBuf++ ^= dwPattern;
			}
		}
		return S_OK;
 	}

private:
	D3D9Plane *m_pPlane;
};

CIntelFastCompositingMixer::CIntelFastCompositingMixer()
{
	m_GUID = DISPSVR_RESOURCE_INTELFASTCOMPVIDEOMIXER;
	m_pFastCompService = 0;
	m_pSCDService = 0;
	m_pRegistrationDevice = 0;
	m_pClearRect = 0;
	m_uClearRect = 0;
	ZeroMemory(&m_rcClip, sizeof(m_rcClip));
}

CIntelFastCompositingMixer::~CIntelFastCompositingMixer()
{
	m_uClearRect = 0;
	SAFE_DELETE_ARRAY(m_pClearRect);
}

STDMETHODIMP CIntelFastCompositingMixer::QueryInterface(REFIID riid, void **ppv)
{
	HRESULT hr = E_NOINTERFACE;

	if (riid == __uuidof(IDispSvrVideoPresenter))
	{
		hr = GetInterface((IDispSvrVideoPresenter *)this, ppv);
	}
	else
	{
		hr = BASE_VIDEO_MIXER::QueryInterface(riid, ppv);
	}
	return hr;
}

STDMETHODIMP CIntelFastCompositingMixer::ProcessMessage(RESOURCE_MESSAGE_TYPE eMessage, LPVOID ulParam)
{
	return BASE_VIDEO_MIXER::ProcessMessage(eMessage, ulParam);
}

static inline bool VidProcIfHWPredicate(VideoProcessorStub &vp)
{
	return vp.sCaps.eType == PROCESSOR_TYPE_HARDWARE;
}

STDMETHODIMP CIntelFastCompositingMixer::_SetDevice(IUnknown *pDevice)
{
	HRESULT hr = BASE_VIDEO_MIXER::_SetDevice(pDevice);
	if (FAILED(hr))
		return hr;

	hr = CD3D9VideoPresenter::_SetDevice(pDevice);
	if (FAILED(hr))
		return hr;

	hr = CIntelFastCompositingService::Create(m_pDevice, &m_pFastCompService);
	if (SUCCEEDED(hr))
	{
        DbgMsg("Creating Intel FastCOM OK!");
		// only one registration device is needed, register surfaces to different
		// services with individual registration handles.
		hr = m_pFastCompService->CreateRegistrationDevice(&m_pRegistrationDevice);
		if (FAILED(hr))
        {
            DbgMsg("CreateRegistrationDevice fail");
			return hr;
        }
	
		m_PresenterCaps.dwFPS = m_pFastCompService->GetMaxFlipRate();
        m_PresenterCaps.bCanVirtualizeFromOrigin = TRUE;
        //m_PresenterCaps.bSupportXvYCC = m_pFastCompService->CanSupportExtendedGamut();

		m_pTexturePool->SetPoolUsage(TEXTURE_POOL_USAGE_OFFSCREENPLANESURFACE);
		GenerateDxva2VPList();

		VideoProcessorStub vp = {0};
		vp.guidVP = DispSvr_VideoProcFastComp;
		vp.RenderTargetFormat = D3DFMT_X8R8G8B8;
		vp.sCaps.uNumBackwardSamples = m_pFastCompService->GetNumBackwardSamples();
		vp.sCaps.uNumForwardSamples = m_pFastCompService->GetNumForwardSamples();
		if (vp.sCaps.uNumBackwardSamples + vp.sCaps.uNumForwardSamples > 0)
			vp.sCaps.uProcessorCaps = PROCESSOR_CAPS_DEINTERLACE_ADAPTIVE;
		else
			vp.sCaps.uProcessorCaps = PROCESSOR_CAPS_DEINTERLACE_BOB;

		for (VideoProcessorList::iterator vi = m_VideoProcessorList.begin(); vi != m_VideoProcessorList.end(); ++vi)
			if (vi->pfnVBltFactory)
			{
				vp.pDelegateVPStub = &*vi;
				break;
			}
		m_VideoProcessorList.push_back(vp);
		SelectVideoProcessor();

		hr = CIntelSCDService::Create(m_pDevice, &m_pSCDService);
        if (FAILED(hr))
        {
            DbgMsg("Creating Intel SCD fail");
        }
        else
        {
            DbgMsg("Creating Intel SCD OK!");
        }
	}
	else
	{
		// on some system, there is no FastComp but SCD is available.
		hr = CIntelSCDService::Create(m_pDevice, &m_pSCDService);
		if (FAILED(hr))
        {
            DbgMsg("Creating FastCOM and SCD fail");
			return hr;
        }
        else
        {
            DbgMsg("Creating Intel SCD OK!");
        }

		// if SCD extension, we need the registration device in order to register unscrambling surfaces.
		if (m_pSCDService->IsSCDExtension())
		{
			hr = m_pSCDService->CreateRegistrationDevice(&m_pRegistrationDevice);
			if (FAILED(hr))
            {
                DbgMsg("CreateRegistrationDevice fail");
				return hr;
            }
		}
	}

	if (m_pSCDService)
	{
		CD3D9VideoPresenterBase::UpdateScanLineProperty();
		m_PresenterCaps.bIsOverlay = FALSE;
		m_PresenterCaps.dwPresenterInfo = PRESENTER_PROPRIETARYOVERLAY;
	}

	return hr;
}

STDMETHODIMP CIntelFastCompositingMixer::_ReleaseDevice()
{
	SAFE_DELETE(m_pFastCompService);
	SAFE_DELETE(m_pSCDService);
	SAFE_DELETE(m_pRegistrationDevice);

	CD3D9VideoPresenter::_ReleaseDevice();
	return BASE_VIDEO_MIXER::_ReleaseDevice();
}

#ifdef DUMP_FASTCOMP_BLT_PARAMS
static inline void DumpFastCompBltParams(const FASTCOMP_BLT_PARAMS &b)
{
	DbgMsg("FastCompBlt: count: %d, TF: %I64d", b.SampleCount, b.TargetFrame);
	for (unsigned int i = 0; i < b.SampleCount; ++i)
	{
		DbgMsg("\tDepth: %d, Start: %I64d, End: %I64d, SrSurface: %x",
			b.pSamples[i].Depth, b.pSamples[i].Start, b.pSamples[i].End, b.pSamples[i].SrcSurface);
	}
}
#endif // DUMP_FASTCOMP_BLT_PARAMS

// TODO: we do not need to register and unregister surfaces each time.
HRESULT CIntelFastCompositingMixer::FastCompositingBlt(const FASTCOMP_BLT_PARAMS &blt)
{
	UINT i;
	HRESULT hr;
	DXVA2_SURFACE_REGISTRATION RegRequest;
	DXVA2_SAMPLE_REG RegEntry[PLANE_MAX + 1 + MAX_BACKWARD_SAMPLES];	// PLANE_MAX + render target

	ASSERT(m_pFastCompService && m_pRegistrationDevice);
	CHECK_POINTER(blt.pRenderTarget);

	RegRequest.RegHandle = m_pFastCompService->GetRegistrationHandle();
	RegRequest.pRenderTargetRequest = &RegEntry[0];
	RegRequest.nSamples             = 0;
	RegRequest.pSamplesRequest      = &RegEntry[1];

	RegEntry[0].pD3DSurface = blt.pRenderTarget;
	RegEntry[0].Operation   = REG_REGISTER;

	for (i = 0; i < blt.SampleCount; ++i)
	{
		RegRequest.nSamples++;
		RegEntry[RegRequest.nSamples].pD3DSurface = blt.pSamples[i].SrcSurface;
		RegEntry[RegRequest.nSamples].Operation   = REG_REGISTER;
	}

	hr = m_pRegistrationDevice->RegisterSurfaces(&RegRequest);
	if (FAILED(hr))
	{
		ASSERT(0 && "m_pRegistrationDevice->RegisterSurfaces failed!");
		return hr;
	}

#ifdef DUMP_FASTCOMP_BLT_PARAMS
	DumpFastCompBltParams(blt);
#endif // DUMP_FASTCOMP_BLT_PARAMS

	hr = m_pFastCompService->FastCompositingBlt(blt);
	if (FAILED(hr))
	{
		DbgMsg("m_pFastCompService->FastCompositingBlt(blt) = 0x%x", hr);
	}

	for (i = 0; i <= RegRequest.nSamples; i++)
	{
		RegEntry[i].Operation = REG_UNREGISTER;
	}
	m_pRegistrationDevice->RegisterSurfaces(&RegRequest);
	return hr;
}

STDMETHODIMP CIntelFastCompositingMixer::_Execute(IDirect3DSurface9 *pDestSurface, const RECT &rcDst, const RECT &rcDstClip)
{
	HRESULT hr = E_FAIL;

	if (m_pFastCompService)
		hr = ExecuteFastCompositing(pDestSurface, rcDst, rcDstClip);

	if (FAILED(hr))
		hr = ExecuteD3DCompositing(pDestSurface, rcDst, rcDstClip);

	return hr;
}

HRESULT CIntelFastCompositingMixer::ExecuteD3DCompositing(IDirect3DSurface9 *pDestSurface, const RECT &rcDst, const RECT &rcDstClip)
{
	HRESULT hr = S_OK;
	D3D9Plane &plane = m_pModel->GetPlane(PLANE_MAINVIDEO);

    if (plane.IsValid() && plane.Protection.eType == PROTECTION_SCRAMBLE_XOR)
	{
		CComPtr<IDirect3DSurface9> pSurface;
		hr = m_pTexturePool->GetRepresentation(plane.hTexture, IID_IDirect3DSurface9, (void **)&pSurface);
		if (SUCCEEDED(hr))
			hr = UnscrambleSurface(pDestSurface, pSurface, plane.Protection);
		ASSERT(plane.Protection.eType == PROTECTION_NONE);
	}
	hr = BASE_VIDEO_MIXER::_Execute(pDestSurface, rcDst, rcDstClip);
	return hr;
}

HRESULT CIntelFastCompositingMixer::ExecuteFastCompositing(IDirect3DSurface9 *pDestSurface, const RECT &rcMixingDst, const RECT &rcDstClip)
{
	HRESULT hr = S_OK;
	RECT rcDst(rcMixingDst);
	FASTCOMP_BLT_PARAMS blt;
	FASTCOMP_VideoSample Samples[PLANE_MAX + MAX_BACKWARD_SAMPLES];

	ZeroMemory(&blt, sizeof(blt));
	ZeroMemory(&Samples, sizeof(Samples));

	// FastComp may have max width/height and will fail if destination is larger,
	// we place a check/workaround here to at least show the video when that happens.
	m_pFastCompService->ApplyTargetRectRestriction(rcDst);

	blt.pRenderTarget = pDestSurface;
	// We should make sure TargetRect falls into render target, or the driver may do a colorfill
	// in the area outside of render target and causes corruption.
	IntersectRect(&blt.TargetRect, &rcDst, &rcDstClip);
	if (IsRectEmpty(&blt.TargetRect))
		return E_FAIL;

	blt.bClearRectChanged = CheckAndUpdateClearRects(rcDst, rcDstClip) == S_OK ? true : false;
	blt.ClearRectCount = m_uClearRect;
	blt.pClearRect = m_pClearRect;
    blt.BackgroundColor = m_pModel->GetBackgroundColor();

	if (IsBackgroundVisible() && PlaneToSample(PLANE_BACKGROUND, Samples[blt.SampleCount], rcDst, rcDstClip) == S_OK)
		blt.SampleCount++;

	if (PlaneToSample(PLANE_MAINVIDEO, Samples[blt.SampleCount], rcDst, rcDstClip) == S_OK)
	{
		D3D9Plane &plane = m_pModel->GetPlane(PLANE_MAINVIDEO);
		FASTCOMP_VideoSample &sampleMainVideo = Samples[blt.SampleCount];
		blt.TargetFrame = plane.VideoProperty.rtStart;
		blt.SampleCount++;

		if (plane.Protection.eType == PROTECTION_SCRAMBLE_XOR)
		{
			hr = UnscrambleSurface(pDestSurface, sampleMainVideo.SrcSurface, plane.Protection);
		}

		if (sampleMainVideo.SampleFormat.SampleFormat != DXVA2_SampleProgressiveFrame)
		{
			if (plane.VideoProperty.dwFieldSelect == FIELD_SELECT_SECOND)
				blt.TargetFrame = (sampleMainVideo.Start + sampleMainVideo.End) / 2;

			for (VideoSampleList::const_iterator it = plane.BackwardSamples.begin();
				it != plane.BackwardSamples.end();
				++it)
			{
				Samples[blt.SampleCount] = sampleMainVideo;
				Samples[blt.SampleCount].SrcSurface = NULL;
				if (SUCCEEDED(m_pTexturePool->GetRepresentation(it->hTexture, IID_IDirect3DSurface9,
					(void **) &Samples[blt.SampleCount].SrcSurface)))
				{
					Samples[blt.SampleCount].Start = it->VideoProperty.rtStart;
					Samples[blt.SampleCount].End = it->VideoProperty.rtEnd;
					Samples[blt.SampleCount].SrcSurface->Release();
					blt.SampleCount++;
				}
			}
		}
	}

	if (PlaneToSample(PLANE_SUBVIDEO, Samples[blt.SampleCount], rcDst, rcDstClip) == S_OK)
	{
        const D3D9Plane &plane = m_pModel->GetPlane(PLANE_SUBVIDEO);
		if (plane.VideoProperty.Format == VIDEO_FORMAT_FIELD_INTERLEAVED_EVEN_FIRST)
			Samples[blt.SampleCount].SampleFormat.SampleFormat = DXVA2_SampleFieldInterleavedEvenFirst;
		else if (plane.VideoProperty.Format == VIDEO_FORMAT_FIELD_INTERLEAVED_ODD_FIRST)
			Samples[blt.SampleCount].SampleFormat.SampleFormat = DXVA2_SampleFieldInterleavedOddFirst;

        LumaKey lumaKey = {0};
        m_pModel->GetLumaKey(&lumaKey);
		if (lumaKey.bEnable)
		{
			Samples[blt.SampleCount].bLumaKey = true;
			Samples[blt.SampleCount].iLumaHigh = lumaKey.uUpper;
			Samples[blt.SampleCount].iLumaLow = lumaKey.uLower;
		}

		blt.SampleCount++;
	}

	for (int i = PLANE_GRAPHICS; i < PLANE_MAX; i++)
	{
		if (PlaneToSample(i, Samples[blt.SampleCount], rcDst, rcDstClip) == S_OK)
			blt.SampleCount++;
	}

	if (blt.SampleCount > 0)
	{
		blt.pSamples = Samples;
        /*if (m_dwGamutFormat == GAMUT_METADATA_RANGE)
        {
            blt.TargetExtGamut = true;
            // When render target is YUY2 and if main video is missing or not in YUV format, TargetTransferMatrix is used.
            // We just set a valid value for it or BT709 is used by the driver when the value is invalid.
            blt.TargetMatrix = DXVA2_VideoTransferMatrix_BT709;
        }*/
		hr = FastCompositingBlt(blt);
	}

	return hr;
}

HRESULT CIntelFastCompositingMixer::PlaneToSample(PLANE_ID id, FASTCOMP_VideoSample &s, const RECT &rcDst, const RECT &rcDstClip)
{
	HRESULT hr;
	const D3D9Plane &plane = m_pModel->GetPlane(id);

    if (!plane.IsValid())
		return E_FAIL;

	s.DstRect = rcDst;
	s.SrcRect = plane.rcSrc;
	hr = PlaneToScreen(plane, s.SrcRect, s.DstRect, rcDstClip);
	if (FAILED(hr))
		return E_FAIL;

	hr = m_pTexturePool->GetRepresentation(plane.hTexture, __uuidof(IDirect3DSurface9), (void **)&s.SrcSurface);
	if (!s.SrcSurface)
		return E_FAIL;

	s.SrcSurface->Release();
	s.Depth = id - 1;
	s.Alpha = plane.fAlpha;
	s.Start = plane.VideoProperty.rtStart;
	s.End = plane.VideoProperty.rtEnd;
	s.SampleFormat.VideoChromaSubsampling = DXVA2_VideoChromaSubsampling_MPEG2;
	s.SampleFormat.NominalRange           = DXVA2_NominalRange_Normal;
	s.SampleFormat.VideoTransferMatrix    = DXVA2_VideoTransferMatrix_BT709;
    //s.SampleFormat.VideoTransferMatrix    = plane.bHDVideo ? DXVA2_VideoTransferMatrix_BT709 : DXVA2_VideoTransferMatrix_BT601;
	s.SampleFormat.VideoLighting          = DXVA2_VideoLighting_dim;
	s.SampleFormat.VideoPrimaries         = DXVA2_VideoPrimaries_BT709;
	s.SampleFormat.VideoTransferFunction  = DXVA2_VideoTransFunc_709;
	s.SampleFormat.SampleFormat           = DXVA2_SampleProgressiveFrame;
	//Workaround : partial blending can't work well on new Intel driver under Vista so we change to do it on SW mode.
	//s.bPartialBlending = plane.bPartialBlending;
	if (plane.bPalettized)
	{
		for (int i = 0; i < 256; i++)
		{
			s.Palette[i].Y = plane.Palette[i].Y;
			s.Palette[i].Cr = plane.Palette[i].Cr;
			s.Palette[i].Cb = plane.Palette[i].Cb;
			s.Palette[i].Alpha = plane.Palette[i].Alpha;
		}
	}

	if (PLANE_MAINVIDEO == id)
	{
        CComPtr<IDirect3DTexture9> pTexture;
        CComPtr<IUnknown> pUnk;
        if (ProcessVideoEffect(PLANE_MAINVIDEO, NULL, s.SrcRect,s.DstRect, /*plane.Format*/ PLANE_FORMAT_YUY2, &pUnk) != S_FALSE)
        {
            if (pUnk)
            {
                hr = pUnk->QueryInterface(IID_IDirect3DTexture9, (void **)&pTexture);
                if (SUCCEEDED(hr))
                {
                    hr = pTexture->GetSurfaceLevel(0, &s.SrcSurface);	ASSERT(SUCCEEDED(hr));
#ifndef _NO_USE_D3DXDLL
                    if (0)
                    {
                        hr = D3DXSaveTextureToFile(_T("C:\\doTexture.bmp"), D3DXIFF_BMP, pTexture, NULL);
                    }
#endif
                }
                else
                {
                    hr = pUnk->QueryInterface(IID_IDirect3DSurface9, (void **)&s.SrcSurface);
#ifndef _NO_USE_D3DXDLL
                    if (0)
                    {
                         hr = D3DXSaveSurfaceToFile(_T("C:\\doSurface.bmp"), D3DXIFF_BMP, s.SrcSurface, NULL, NULL);
                    }
#endif
                }
                s.SrcSurface->Release();
            }
        }
        else if (m_pVideoEffect3DBlt && m_pVideoEffect3DBlt->IsEffectEnabled())
        {
            hr = plane.pVBlt->IntermediateTextureVBlt(s.SrcSurface, s.SrcRect, &pTexture);
            if (SUCCEEDED(hr))
            {
                CComPtr<IUnknown> pOutputSurface;
                ProcessEffectRequest Request = {0};
                Request.lpSourceRect = &s.SrcRect;
                Request.lpTargetRect = &s.DstRect;
                Request.pVideoProperty = &plane.VideoProperty;
                Request.pInput = pTexture;
                Request.ppOutput = &pOutputSurface;
                hr = m_pVideoEffect3DBlt->ProcessEffect(&Request);
                if (SUCCEEDED(hr))
                {
                    pTexture.Release();
                    hr = pOutputSurface->QueryInterface(IID_IDirect3DTexture9, (void **)&pTexture);
                    if (SUCCEEDED(hr))
                    {
                        hr = pTexture->GetSurfaceLevel(0, &s.SrcSurface);	ASSERT(SUCCEEDED(hr));
                        s.SrcSurface->Release();
                    }
                }
            }
        }
		else
		{
			if (plane.VideoProperty.Format == VIDEO_FORMAT_FIELD_INTERLEAVED_EVEN_FIRST)
				s.SampleFormat.SampleFormat = DXVA2_SampleFieldInterleavedEvenFirst;
			else if (plane.VideoProperty.Format == VIDEO_FORMAT_FIELD_INTERLEAVED_ODD_FIRST)
				s.SampleFormat.SampleFormat = DXVA2_SampleFieldInterleavedOddFirst;
		}

        /*if (m_dwGamutFormat == GAMUT_METADATA_RANGE)
        {
            s.bExtendedGamut = true;
        }*/
	}

	return S_OK;
}

HRESULT CIntelFastCompositingMixer::CheckAndUpdateClearRects(const RECT &rcDst, const RECT &rcDstClip)
{
    const ClearRectList &ClearRectList = m_pModel->GetClearRectangles();
	if (ClearRectList.size() > 0)
	{
		FASTCOMP_ClearRect *pClearRects = new FASTCOMP_ClearRect[ClearRectList.size()];
		FASTCOMP_ClearRect *pCR = pClearRects;
		if (!pClearRects)
			return E_OUTOFMEMORY;

		for (ClearRectList::const_iterator i = ClearRectList.begin(); i != ClearRectList.end(); ++i, ++pCR)
		{
			RECT rcOutput = rcDst;
			NRectToRect(rcOutput, i->Rect);
			IntersectRect(&pCR->Rect, &rcOutput, &rcDstClip);
			pCR->Depth = i->Target == CLEAR_RECT_TARGET_MAIN ? 0 : 1;
		}

		if (m_pClearRect)
		{
			if (m_uClearRect == ClearRectList.size()
				&& memcmp(m_pClearRect, pClearRects, sizeof(FASTCOMP_ClearRect) * ClearRectList.size()) == 0)
			{
				delete [] pClearRects;
				return S_FALSE;
			}

			delete [] m_pClearRect;
		}

		m_pClearRect = pClearRects;
		m_uClearRect = ClearRectList.size();
		return S_OK;
	}
	else if (m_uClearRect > 0)
	{
		delete [] m_pClearRect;
		m_pClearRect = 0;
		m_uClearRect = 0;
		return S_OK;
	}

	return S_FALSE;
}

STDMETHODIMP CIntelFastCompositingMixer::_QueryPlaneCaps(PLANE_ID PlaneID, PLANE_FORMAT Format, PlaneCaps *pCap)
{
	if (m_pFastCompService)
	{
		pCap->dwFlags |= PLANE_CAP_HW_PARTIAL_BLENDING;
	}

	if (PlaneID == PLANE_MAINVIDEO && m_pSCDService && m_pSCDService->IsSCDExtension())
	{
		pCap->dwFlags |= PLANE_CAP_VIDEO_SCRAMBLE;
	}
	return S_OK;
}

STDMETHODIMP CIntelFastCompositingMixer::SetDisplayRect(const RECT *pDst, const RECT *pSrc)
{
	CAutoLock lock(&m_csLock);

	HRESULT hr;

	if (m_pSCDService)
	{
		hr = CD3D9VideoPresenterBase::SetDisplayRect(pDst, pSrc);
		if (SUCCEEDED(hr))
		{
    		SIZE szDisplay = {GetRegistry(REG_DISPLAY_WIDTH, 0), GetRegistry(REG_DISPLAY_HEIGHT, 0)};
			hr = CD3D9VideoPresenterBase::CaculateDstClipRect(&m_rcClip, &m_rcDst, &m_rcSrc, szDisplay);

			if (m_rcSrc.left >= m_rcSrc.right || m_rcSrc.top >= m_rcSrc.bottom)
				return S_OK;

			HMONITOR hMonitor = MonitorFromWindow(m_hwnd, (m_hwnd) ? MONITOR_DEFAULTTONEAREST : MONITOR_DEFAULTTOPRIMARY);
			MONITORINFOEX MonInfo;

			ZeroMemory(&MonInfo, sizeof(MONITORINFOEX));
			MonInfo.cbSize = sizeof(MONITORINFOEX);
			if (!GetMonitorInfo(hMonitor, &MonInfo))
				ASSERT(0);

			HDC hdc = CreateDC(NULL, MonInfo.szDevice, NULL, NULL);
			if (hdc)
			{
				hr = m_pSCDService->SetWindowPosition(m_rcDst, m_rcClip, hdc);
				DeleteDC(hdc);
			}
			else
				hr = E_FAIL;
		}
	}
	else
	{
		hr = CD3D9VideoPresenter::SetDisplayRect(pDst, pSrc);
	}

	return hr;
}

STDMETHODIMP CIntelFastCompositingMixer::Present(const PresentHints *pHints)
{
	CAutoLock lock(&m_csLock);
	HRESULT hr = E_FAIL;
	if (m_pDevice)
	{
		if (m_pSCDService)
		{
			RECT *pDstRect = m_rcDst.left == m_rcDst.right || m_rcDst.top == m_rcDst.bottom ? NULL : &m_rcDst;
			RECT *pSrcRect = m_rcSrc.left == m_rcSrc.right || m_rcSrc.top == m_rcSrc.bottom ? NULL : &m_rcClip;
			// We will have to send src/dst rectangles to Present() when dwm is on. When dwm is off, we use SCD_SET_WINDOWPOSITION.		
			// The reason is that dwm is basically a compositing manager which will compose all windows and call DDI Present at its own. 
			// Our Present call will not pass the src/dst information to the driver. We need to let DWM compose images correctly to the 
			// backbuffer then the driver will take it from there.
			if (m_pSCDService->IsSCDOverlay())
			{
				hr = D3D9Present(pSrcRect, NULL);
				//----------------------------------------------------------------------------------------
				// Workaround: Force post device lost message when the present mode is changed. 
				// Otherwise, AP will have no video if vga driver is Intel's 15.13 branch driver or later.
				if (hr == S_PRESENT_MODE_CHANGED)
				{
					hr = E_FAIL;
				}
				//----------------------------------------------------------------------------------------
			}
			else
			{
				RECT rcDst = m_rcDst;
				POINT pt = {0};
				::ClientToScreen(m_hwnd, &pt);
				::OffsetRect(&rcDst, GetRegistry(REG_DISPLAY_X, 0) - pt.x, GetRegistry(REG_DISPLAY_Y, 0) - pt.y);

				hr = D3D9Present(pSrcRect, &rcDst);
			}
		}
		else
		{
			hr = CD3D9VideoPresenter::Present(pHints);
		}
	}
	return hr;
}

STDMETHODIMP CIntelFastCompositingMixer::SetScreenCaptureDefense(BOOL bEnable)
{
	CAutoLock lock(&m_csLock);

	if (m_pSCDService)
		return S_OK;
	return E_FAIL;
}

STDMETHODIMP CIntelFastCompositingMixer::OnCreatePlane(const DispSvr::PlaneInit &init)
{
	if (init.PlaneID == PLANE_MAINVIDEO
		&& m_pSCDService && m_pSCDService->IsSCDExtension()
		&& (init.dwFlags & PLANE_INIT_EXTERNAL_SURFACE) == 0
		&& IsHDVideo(init.uWidth, init.uHeight))
	{
        // don't need to lock model as caller should have locked.
        D3D9Plane &plane = m_pModel->GetPlane(init.PlaneID);
		// must register post texture filter before calling _CreatePlane to the base to be effective.
		plane.pPostTextureFilter = new CIntelScrambleFilter(&plane);
	}
	return BASE_VIDEO_MIXER::OnCreatePlane(init);
}

HRESULT CIntelFastCompositingMixer::UnscrambleSurface(IDirect3DSurface9 *pRT, IDirect3DSurface9 *pSurface, ContentProtection &security)
{
	HRESULT hr;
	DXVA2_SURFACE_REGISTRATION RegRequest;
	DXVA2_SAMPLE_REG RegEntry[2];

	ASSERT(m_pRegistrationDevice);
	ASSERT(m_pSCDService && m_pSCDService->IsSCDExtension());
	CHECK_POINTER(m_pSCDService);

	RegRequest.RegHandle = m_pSCDService->GetRegistrationHandle();
	RegRequest.pRenderTargetRequest = &RegEntry[0];
	RegRequest.nSamples             = 1;
	RegRequest.pSamplesRequest      = &RegEntry[1];

	// render target must be set to get passed by microsoft runtime in videoprocessblt.
	RegEntry[0].pD3DSurface = pRT;
	RegEntry[0].Operation   = REG_REGISTER;
	RegEntry[1].pD3DSurface = pSurface;
	RegEntry[1].Operation   = REG_REGISTER;

	RegRequest.RegHandle = m_pSCDService->GetRegistrationHandle();
	hr = m_pRegistrationDevice->RegisterSurfaces(&RegRequest);
	if (FAILED(hr))
	{
		ASSERT(0 && "SCD register surface failed.");
		return hr;
	}

	hr = m_pSCDService->UnscrambleSurface(RegRequest.pSamplesRequest[0].pD3DSurface, security.dwSizeScrambled, security.dwScrambleSeed);
	if (SUCCEEDED(hr))
		security.eType = PROTECTION_NONE;	// now the surface is unscrambled.
	else
		ASSERT(0 && "SCD unscramble failed.");
	return hr;
}

/*STDMETHODIMP CIntelFastCompositingMixer::SetGamutMetadata(const DWORD dwFormat, void *pGamutMetadata)
{
    if (!m_PresenterCaps.bSupportXvYCC)
    {
        return S_FALSE;
    }

    HRESULT hr = E_INVALIDARG;
    m_dwGamutFormat = dwFormat;

    if (m_dwGamutFormat == GAMUT_METADATA_NONE)
    {
        memset(&m_GamutRange, 0, sizeof(m_GamutRange));
        memset(&m_GamutVertices, 0, sizeof(m_GamutVertices));
        hr = CIntelCUIHelper::GetHelper()->SetGamutMetadata(m_hwnd, NULL, 0);
    }
    else if (m_dwGamutFormat == GAMUT_METADATA_RANGE)
    {
        memcpy(&m_GamutRange, pGamutMetadata, sizeof(m_GamutRange));
        hr = CIntelCUIHelper::GetHelper()->SetGamutMetadata(m_hwnd, pGamutMetadata, sizeof(GamutMetadataRange));
    }

    return hr;
}*/