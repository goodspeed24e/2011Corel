#include "stdafx.h"
#include "DriverExtensionHelper.h"
#include "DXVAHDVideoProcessor.h"
#include "D3D9Dxva2HDVideoMixer.h"
#include "D3D9VideoEffect3DManager.h"

using namespace DispSvr;

CD3D9Dxva2HDVideoMixer::CD3D9Dxva2HDVideoMixer() :
	m_pVP(NULL)
{
	m_GUID = DISPSVR_RESOURCE_D3DDXVAHDVIDEOMIXER;
    ZeroMemory(&m_VPInitParam, sizeof(m_VPInitParam));

    // if DXVAHD mixer is used, we only create DXVAHD when there is driver support.
    m_VPInitParam.bUseNativeDXVAHDOnly = true;

	// make GUI to be blended as one of the DXVAHD stream.
	m_MixerCaps.dwFlags &= ~(MIXER_CAP_3D_RENDERTARGET);
}

CD3D9Dxva2HDVideoMixer::~CD3D9Dxva2HDVideoMixer()
{
}

STDMETHODIMP CD3D9Dxva2HDVideoMixer::_SetDevice(IUnknown *pDevice)
{
	HRESULT hr = CD3D9VideoMixerBase::_SetDevice(pDevice);
	if (SUCCEEDED(hr))
	{
		hr = CDXVAHDVP::CreateVP(m_pDeviceEx, &m_VPInitParam, &m_pVP);
		if (FAILED(hr))
		{
            DbgMsg("CDXVAHDVP unable to create DXVAHD device. hr = 0x%x", hr);
			return hr;
		}

		GenerateDxva2VPList();

		VideoProcessorStub vp = {0};

		vp.guidVP = DispSvr_VideoProcDxvaHD;
		vp.RenderTargetFormat = D3DFMT_X8R8G8B8;

		// Look for one VP in the list which provides VBltFactory in case we want to separate video
		// processing from DXVA HD.
		for (VideoProcessorList::iterator vi = m_VideoProcessorList.begin(); vi != m_VideoProcessorList.end(); ++vi)
			if (vi->pfnVBltFactory)
			{
				vp.pDelegateVPStub = &*vi;
				break;
			}

		hr = m_pVP->GetVPCaps(vp.sCaps);
		for (UINT i = 0; i < DXVAHDVP_MAX_FILTER; i++)
		{
			if (vp.sCaps.uFilterCaps & (1 << i))
			{
				m_pVP->GetFilterValueRange(static_cast<VIDEO_FILTER> (i), vp.FilterRanges[i]);
				
				// Update default filter values as current values
				vp.fFilterValue[i] = vp.FilterRanges[i].fDefaultValue;
			}
		}

		m_VideoProcessorList.push_back(vp);
		ASSERT(SUCCEEDED(hr));
		SelectVideoProcessor();

		// native DXVAHD can handle surface directly without needing texture container.
		if (m_pVP->IsNativeDXVAHD())
			hr = m_pTexturePool->SetPoolUsage(TEXTURE_POOL_USAGE_OFFSCREENPLANESURFACE);
	}

	return hr;
}

STDMETHODIMP CD3D9Dxva2HDVideoMixer::_ReleaseDevice()
{
	SAFE_DELETE(m_pVP);

	return CD3D9VideoMixerBase::_ReleaseDevice();
}

STDMETHODIMP CD3D9Dxva2HDVideoMixer::_QueryPlaneCaps(PLANE_ID PlaneID, PLANE_FORMAT Format, PlaneCaps *pCap)
{
	// DXVA HD does not seem to provide according functionality.
	pCap->dwFlags &= ~PLANE_CAP_HW_PARTIAL_BLENDING;
	return S_OK;
}

HRESULT CD3D9Dxva2HDVideoMixer::_Execute(IDirect3DSurface9 *pDestSurface, const RECT &rcDst, const RECT &rcDstClip)
{
	return VideoProcessBltHD(pDestSurface, rcDst, rcDstClip);
}

HRESULT CD3D9Dxva2HDVideoMixer::VideoProcessBltHD(IDirect3DSurface9 *pDst, const RECT &rcDst, const RECT &rcDstClip)
{
	HRESULT hr = S_OK;
	UINT uStreamID = 0, uMaxInputStream = 0;
	DXVAHDVP_StreamState state;
	DXVAHDVP_BltState bltState;
	bool pSkipStream[PLANE_MAX] = {0};

	if (!m_pVP || !pDst)
		return E_FAIL;

	ZeroMemory(&bltState, sizeof(bltState));
    uMaxInputStream = m_pVP->GetMaxInputStream();

	// Calculating the number of visible plane.
    UINT uVisiblePlaneNumber = 0;
    for (UINT PlaneID = 0; PlaneID < PLANE_MAX ; PlaneID++)
    {
        const D3D9Plane &plane = m_pModel->GetPlane(PlaneID);
        if (plane.IsValid())
            uVisiblePlaneNumber++;
        else
            pSkipStream[PlaneID] = true;
    }
    // Known issue - If the number of visible stream is larger than the MaxInputStream number, we have to drop one of them at least.
    if(uMaxInputStream < uVisiblePlaneNumber)
        pSkipStream[PLANE_BACKGROUND] = true;

	for (UINT PlaneID = 0; PlaneID < PLANE_MAX && uStreamID < uMaxInputStream; PlaneID++)
	{
		if (pSkipStream[PlaneID])
			continue;

		const D3D9Plane &plane = m_pModel->GetPlane(PlaneID);
		ZeroMemory(&state, sizeof(state));

		state.dwLastUpdateTS = plane.dwLastUpdateTS;
		state.SourceRect = plane.rcSrc;
		if (plane.bFullScreenMixing && m_eWindowState == RESOURCE_WINDOW_STATE_FULLSCREEN)
			state.DestinationRect = m_rcMixingDstClip;
		else
			state.DestinationRect = rcDst;

        if (SUCCEEDED(m_pTexturePool->GetRepresentation(plane.hTexture, IID_IDirect3DSurface9, (void **)&state.pInputSurface)))
		{
			state.pInputSurface->Release();

			if (PlaneID == PLANE_MAINVIDEO || PlaneID == PLANE_SUBVIDEO)
				hr = SetSingleVideoStreamStates(PlaneID, state);
			else
				hr = SetSingleStreamStates(PlaneID, state);

			if ((GetMixerStereoMode() == MIXER_STEREO_MODE_NV_STEREOAPI) ||
                (GetMixerStereoMode() == MIXER_STEREO_MODE_HDMI_STEREO))
				SetSingleStereoStreamStates(PlaneID, state);

			hr |= PlaneToScreen(plane, state.SourceRect, state.DestinationRect, rcDstClip);

            if (SUCCEEDED(hr))
				state.bEnable = TRUE;
		}

		hr = m_pVP->SetStreamState(uStreamID, state);
		uStreamID++;
	}

    ZeroMemory(&state, sizeof(state));
    for(UINT uClearStreamID = uStreamID ; uClearStreamID < uMaxInputStream ; uClearStreamID++)
    {
        // Workaround for NV DXVA HD implementation, because DXVA2 runtime
        // checks input surface, source/destination rectangle on Vista.
        if (m_VPInitParam.bNvDXVAHD)
        {
            D3DSURFACE_DESC DummyDesc;
            if (FAILED(pDst->GetDesc(&DummyDesc)))
                ASSERT(0);

            // If no surface is available, we still need to assign a valid surface for VPBltHD 
            // with some necessary stream state 
            // on the other hand, DXVAHD won't composite because one of the streams has a invalid surface.

            SetRect(&state.SourceRect, 0, 0, DummyDesc.Width, DummyDesc.Height);
            SetRect(&state.DestinationRect, 0, 0, DummyDesc.Width, DummyDesc.Height);

            state.pInputSurface = pDst;

            // Force to update stream state whether the stream is valid or not
            hr = m_pVP->SetStreamState(uClearStreamID, state, true);
        }
        else
            hr = m_pVP->SetStreamState(uClearStreamID, state);
    }

	// We use DXVAHD to clear non-video area so the TargetRect is rcDstClip instead of rcDst.
	bltState.TargetRect = rcDstClip;

//	bltState.OutputColorSpace.bVideoProcessing = 0;
//	bltState.OutputColorSpace.bLimitedRange = 0;
	if (GetRegistry(REG_DISPLAY_XVYCC_MONITOR_TYPE, DISPLAY_XVYCC_MONITOR_NOT_SUPPORT) == DISPLAY_XVYCC_MONITOR_BT709)
		bltState.OutputColorSpace.bBT709 = 1;
	if (GetRegistry(REG_DISPLAY_XVYCC_GAMUT, FALSE))
		bltState.OutputColorSpace.bXvYCC = 1;

	hr = m_pVP->VBltHD(pDst, bltState);
	return hr;
}

HRESULT CD3D9Dxva2HDVideoMixer::SetSingleVideoStreamStates(PLANE_ID PlaneID, DXVAHDVP_StreamState &state, UINT uViewID)
{
	D3D9Plane &plane = m_pModel->GetPlane(PlaneID);
	HRESULT hr = S_OK;

	CHECK_POINTER(state.pInputSurface);

	if (plane.VideoProperty.Format == VIDEO_FORMAT_PROGRESSIVE)
		state.FrameFormat = DXVAHDVP_FRAME_FORMAT_PROGRESSIVE;
	else if (plane.VideoProperty.Format == VIDEO_FORMAT_FIELD_INTERLEAVED_EVEN_FIRST)
		state.FrameFormat = DXVAHDVP_FRAME_FORMAT_INTERLACED_TOP_FIELD_FIRST;
	else if (plane.VideoProperty.Format == VIDEO_FORMAT_FIELD_INTERLEAVED_ODD_FIRST)
		state.FrameFormat = DXVAHDVP_FRAME_FORMAT_INTERLACED_BOTTOM_FIELD_FIRST;

	state.InputColorSpace.bBT709 = plane.bHDVideo ? 1 : 0;	// 0:BT.601(SDTV), 1:BT.709(HDTV)
	state.InputColorSpace.bXvYCC = GetRegistry(REG_DISPLAY_XVYCC_GAMUT, FALSE) ? 1 : 0;	// 0:Conventional, 1:Expanded(xvYCC)

	if (PLANE_SUBVIDEO == PlaneID)
	{
		state.fAlpha = plane.fAlpha;
        LumaKey lumaKey = {0};
        m_pModel->GetLumaKey(&lumaKey);
		if (lumaKey.bEnable)
			state.LumaKey = lumaKey;
	}

	if (PLANE_MAINVIDEO == PlaneID)
	{
		const VideoProcessorStub *pVP = m_PlaneConfigs[PLANE_MAINVIDEO].pVideoProcessorStub;
		for (int i = 0; i <= DXVAHDVP_MAX_FILTER; i++)
		{
			if ((1 << i) & pVP->sCaps.uFilterCaps)
			{
				state.Filter[i].bEnable = TRUE;
				state.Filter[i].fLevel = pVP->fFilterValue[i];
			}
		}

		// video effect pipeline
		CComPtr<IUnknown> pUnk;
        // cached the base view rectangels, source/destination rectangles may be changed in the first ProcessVideoEffect() call.
        // If we pass changed rectangles again to ProcessVideoEffect(), some effects may reset internally due to width/height mismatch.
        RECT rcDepSource = state.SourceRect;
        RECT rcDepDest = state.DestinationRect;

        if (ProcessVideoEffect(PLANE_MAINVIDEO, &plane.bStereoSxSVideoDetected, state.SourceRect, state.DestinationRect, plane.Format, &pUnk, uViewID) != S_FALSE && pUnk)
		{
			CComQIPtr<IDirect3DSurface9> pSurface = pUnk;
            if (CComQIPtr<IDirect3DTexture9> pTexture = pUnk)
            {
                hr = pTexture->GetSurfaceLevel(0, &pSurface);
				if (FAILED(hr))
					return hr;
            }
			ASSERT(pSurface);
			state.pInputSurface = pSurface;
            state.FrameFormat = DXVAHDVP_FRAME_FORMAT_PROGRESSIVE;

			state.eStereoMixingMode = GetMixerStereoMode();
            if ( state.eStereoMixingMode == MIXER_STEREO_MODE_NV_STEREOAPI || 
				( state.eStereoMixingMode == MIXER_STEREO_MODE_HDMI_STEREO && (GetRegistry(REG_VENDOR_ID, 0) == PCI_VENDOR_ID_NVIDIA) ))
            {
                pUnk.Release();

                // process effect again for the dependent view.
                if (ProcessVideoEffect(PLANE_MAINVIDEO, NULL, rcDepSource, rcDepDest, plane.Format, &pUnk, uViewID + 1) != S_FALSE && pUnk)
                {
                    CComQIPtr<IDirect3DSurface9> pSurface = pUnk;
                    if (CComQIPtr<IDirect3DTexture9> pTexture = pUnk)
                    {
                        hr = pTexture->GetSurfaceLevel(0, &pSurface);
                        if (FAILED(hr))
                           return hr;
                    }
	                state.pDependentSurface = pSurface;
                }
            }
		}
		else
		{
			// process de-interlacing and color space conversion by calling VideoProcessBlt()
			if ((m_VPInitParam.bNvDXVAHD && plane.VideoProperty.Format != VIDEO_FORMAT_PROGRESSIVE)
				|| !m_pVP->IsValidInputFormat(plane.Format))
			{
				CComPtr<IDirect3DSurface9> pIntermediateSurface;
				hr = plane.pVBlt->IntermediateVBlt(state.pInputSurface, state.SourceRect, PLANE_FORMAT_NV12, &pIntermediateSurface);
				if (FAILED(hr))
					return hr;

				state.pInputSurface = pIntermediateSurface;
				state.FrameFormat = DXVAHDVP_FRAME_FORMAT_PROGRESSIVE;
			}

			ASSERT(state.uPastFrameCount == 0);
			for (VideoSampleList::const_iterator it = plane.BackwardSamples.begin();
				it != plane.BackwardSamples.end();
				++it)
			{
				if (SUCCEEDED(m_pTexturePool->GetRepresentation(it->hTexture, __uuidof(IDirect3DSurface9),
					(void **) &state.ppPastSurfaces[state.uPastFrameCount])))
				{
					state.ppPastSurfaces[state.uPastFrameCount]->Release();
					++state.uPastFrameCount;
				}
			}
		}
	}

	// OutputIndex selects the result from deinterlacing.
	// When the FrameFormat is progressive, we should always select the index 0.
	// Otherwise black or undefined video output may be selected and shown.
	if (DXVAHDVP_FRAME_FORMAT_PROGRESSIVE != state.FrameFormat)
	{
		if (FIELD_SELECT_SECOND == plane.VideoProperty.dwFieldSelect)
			state.uOutputIndex = 1;
		else if (FIELD_SELECT_REPEAT_FIRST == plane.VideoProperty.dwFieldSelect)
			state.uOutputIndex = 2;
	}
    D3DSURFACE_DESC desc;
    if (state.pInputSurface && SUCCEEDED(state.pInputSurface->GetDesc(&desc)))
    {
        if (desc.Format == PLANE_FORMAT_ARGB || desc.Format == D3DFMT_A8R8G8B8 || 
            desc.Format == PLANE_FORMAT_XRGB || desc.Format == D3DFMT_X8B8G8R8)
        {
            state.InputColorSpace.bGraphics = 1;		// 0:Video, 1:Graphics  
        }
    }
	return hr;
}

HRESULT CD3D9Dxva2HDVideoMixer::SetSingleStreamStates(PLANE_ID PlaneID, DXVAHDVP_StreamState &state)
{
	const D3D9Plane &plane = m_pModel->GetPlane(PlaneID);

	CHECK_POINTER(state.pInputSurface);

	state.InputColorSpace.bGraphics = 1;
	state.fAlpha = plane.fAlpha;

    HRESULT hr = E_FAIL;
	CComPtr<IUnknown> pUnk;

	if (PLANE_GRAPHICS == PlaneID && (ProcessVideoEffect(PLANE_GRAPHICS, NULL, state.SourceRect, state.DestinationRect, plane.Format, &pUnk) != S_FALSE))
	{
		/* if FAILED(ProcessVideoEffect()) 
		   we expect "pUnk = NULL"
		*/

		// video effect pipeline
		CComQIPtr<IDirect3DTexture9> pTexture = pUnk;
		if (pTexture)
		{
            CComPtr<IDirect3DSurface9> pSurface;
            hr = pTexture->GetSurfaceLevel(0, &pSurface);	ASSERT(SUCCEEDED(hr));
            state.pInputSurface = pSurface;
#ifndef _NO_USE_D3DXDLL
            if (0)
            {
                hr = D3DXSaveSurfaceToFile(_T("C:\\reRT.png"), D3DXIFF_PNG, state.pInputSurface, NULL, NULL);
            }
#endif
        }
        else
        {
            CComQIPtr<IDirect3DSurface9> pSurface = pUnk;
            state.pInputSurface = pSurface;
#ifndef _NO_USE_D3DXDLL
            if (0)
            {
                hr = D3DXSaveSurfaceToFile(_T("C:\\doSurface.bmp"), D3DXIFF_BMP, state.pInputSurface, NULL, NULL);
            }
#endif
		}
        state.FrameFormat = DXVAHDVP_FRAME_FORMAT_PROGRESSIVE;
	}

	if (plane.bPalettized)
	{
		state.uPaletteCount = 256;
		ASSERT(sizeof(plane.Palette) == sizeof(state.Palette));
		memcpy(state.Palette, plane.Palette, sizeof(state.Palette));
	}
	return S_OK;
}

HRESULT CD3D9Dxva2HDVideoMixer::SetSingleStereoStreamStates(PLANE_ID PlaneID, DXVAHDVP_StreamState &state)
{
	const D3D9Plane &plane = m_pModel->GetPlane(PlaneID);

	CHECK_POINTER(state.pInputSurface);

	state.eStereoMixingMode = GetMixerStereoMode();
	if (STEREO_STREAM_OFFSET == plane.eStereoStreamMode)
	{
		// for one plane + offset case
		state.lStereoDisplayOffset = GetStereoOffset(plane);
	}
	else if (STEREO_STREAM_DUALVIEW == plane.eStereoStreamMode)
	{
		if (!state.pDependentSurface
			&& SUCCEEDED(m_pTexturePool->GetRepresentation(plane.GetViewTextureHandle(1), IID_IDirect3DSurface9, (void **)&state.pDependentSurface)))
        {
#ifndef _NO_USE_D3DXDLL
            HRESULT hr = E_FAIL;
            if (0)
            {
                hr = D3DXSaveSurfaceToFile(_T("C:\\doSurface.png"), D3DXIFF_PNG, state.pDependentSurface, NULL, NULL);
            }
#endif
			state.pDependentSurface->Release();
        }
		// for two planes + offset case
		state.lStereoDisplayOffset = GetStereoOffset(plane);
	}

	return S_OK;
}
