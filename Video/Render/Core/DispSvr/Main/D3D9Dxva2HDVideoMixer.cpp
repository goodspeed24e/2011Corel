#include "Imports/ThirdParty/Microsoft/DXVAHD/d3d9.h"
#include "Imports/ThirdParty/Microsoft/DXVAHD/d3d9types.h"
#include "stdafx.h"
#include "DispSvr.h"
#include "ResourceManager.h"
#include "RegistryService.h"
#include "DynLibManager.h"
#include "MathVideoMixing.h"
#include "D3D9Dxva2HDVideoMixer.h"
#include "D3D9VideoEffect3DManager.h"

using namespace DispSvr;

#ifdef DUMP_DXVA_HD
#define SET_STREAM_STATE(uiStream, stateType, dataPointer)	\
	SetSS(m_pHDVP, uiStream, stateType, sizeof(stateType ## _DATA), dataPointer)
#define SET_BLT_STATE(stateType, dataPointer) \
	SetBS(m_pHDVP, stateType, sizeof(stateType ## _DATA), dataPointer)

static inline HRESULT SetSS(IDXVAHD_VideoProcessor *pDev, UINT Stream, DXVAHD_STREAM_STATE State, UINT uSize, const void *pData)
{
	HRESULT hr = pDev->SetVideoProcessStreamState(Stream, State, uSize, pData);
	DbgMsg("SetVideoProcessStreamState(StreamNumber=%d, State=%d, Size=%d, Data=0x%p) = 0x%x", Stream, State, uSize, pData, hr);
	return hr;
}

static inline HRESULT SetBS(IDXVAHD_VideoProcessor *pDev, DXVAHD_BLT_STATE State, UINT uSize, const void *pData)
{
	HRESULT hr = pDev->SetVideoProcessBltState(State, uSize, pData);
	DbgMsg("SetVideoProcessBltState(State=%d, Size=%d, Data=0x%p) = 0x%x", State, uSize, pData, hr);
	return hr;
}
#else
#define SET_STREAM_STATE(uiStream, stateType, dataPointer)	\
	m_pHDVP->SetVideoProcessStreamState(uiStream, stateType, sizeof(stateType ## _DATA), dataPointer)
#define SET_STREAM_FILTER_STATE(uiStream, stateType, dataPointer)	\
	m_pHDVP->SetVideoProcessStreamState(uiStream, stateType, sizeof(DXVAHD_STREAM_STATE_FILTER_DATA), dataPointer)
#define SET_BLT_STATE(stateType, dataPointer) \
	m_pHDVP->SetVideoProcessBltState(stateType, sizeof(stateType ## _DATA), dataPointer)
#endif

// NVAPI DXVA HD via DXVA2
extern "C" HRESULT WINAPI
NV_DXVAHD_CreateDevice(IDirect3DDevice9Ex* pD3DDevice, const DXVAHD_CONTENT_DESC* pContentDesc, DXVAHD_DEVICE_USAGE Usage, void** ppDevice);

static void DXVAHD_ValueRange2ValueRange(DXVAHD_FILTER_RANGE_DATA &d, ValueRange &v)
{
	v.fDefaultValue = float(d.Default);
	v.fMaxValue = float(d.Maximum);
	v.fMinValue = float(d.Minimum);
	v.fStepSize = float(d.Multiplier);
}

CD3D9Dxva2HDVideoMixer::CD3D9Dxva2HDVideoMixer() :
	m_pHDDev(0),
	m_pHDVP(0),
	m_uiFrameCount(0),
	m_uMaxInputStream(0),
	m_bNvDxvaHD(false)
{
	m_GUID = DISPSVR_RESOURCE_D3DDXVAHDVIDEOMIXER;
	ZeroMemory(m_StreamData, sizeof(m_StreamData));
	m_pfnStreamSetter[PLANE_BACKGROUND] = &CD3D9Dxva2HDVideoMixer::SetSingleStreamStates;
	m_pfnStreamSetter[PLANE_MAINVIDEO] = &CD3D9Dxva2HDVideoMixer::SetSingleVideoStreamStates;
	m_pfnStreamSetter[PLANE_SUBVIDEO] = &CD3D9Dxva2HDVideoMixer::SetSingleVideoStreamStates;
	m_pfnStreamSetter[PLANE_GRAPHICS] = &CD3D9Dxva2HDVideoMixer::SetSingleStreamStates;
	m_pfnStreamSetter[PLANE_INTERACTIVE] = &CD3D9Dxva2HDVideoMixer::SetSingleStreamStates;
	m_pfnStreamSetter[PLANE_OTHER] = &CD3D9Dxva2HDVideoMixer::SetSingleStreamStates;
	SetDefaultStates();

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
		// DXVA HD must use IDirect3DDevice9Ex
		if (m_pDeviceEx == 0)
			return E_FAIL;

		DXVAHD_VPDEVCAPS VPDevCaps;
		DXVAHD_CONTENT_DESC contentDesc;

		ZeroMemory(&VPDevCaps, sizeof(VPDevCaps));
		ZeroMemory(&contentDesc, sizeof(contentDesc));
		contentDesc.InputFrameFormat = DXVAHD_FRAME_FORMAT_PROGRESSIVE;
		contentDesc.InputFrameRate.Numerator = 0;
		contentDesc.InputFrameRate.Denominator = 0;
		contentDesc.InputWidth = 1920;
		contentDesc.InputHeight = 1080;
		contentDesc.OutputFrameRate.Numerator = 0;
		contentDesc.OutputFrameRate.Denominator = 0;
		contentDesc.OutputWidth = m_rcMonitor.right - m_rcMonitor.left;
		contentDesc.OutputHeight = m_rcMonitor.bottom - m_rcMonitor.top;

		D3DCAPS9 caps9;
		hr = m_pDeviceEx->GetDeviceCaps(&caps9);
		// on windows 7, we can check if DXVAHD is supported via D3D9 device cap.
		if (SUCCEEDED(hr) && (D3DCAPS3_DXVAHD & caps9.Caps3) != 0)
		{
			if (!DispSvr::CDynLibManager::GetInstance()->pfnDXVAHD_CreateDevice)
				return E_FAIL;

			hr = DispSvr::CDynLibManager::GetInstance()->pfnDXVAHD_CreateDevice(
				m_pDeviceEx, &contentDesc, DXVAHD_DEVICE_USAGE_PLAYBACK_NORMAL, NULL, &m_pHDDev
				);
			m_bNvDxvaHD = false;
		}
		else
		{
			hr = NV_DXVAHD_CreateDevice(m_pDeviceEx, &contentDesc, DXVAHD_DEVICE_USAGE_PLAYBACK_NORMAL, (void **)&m_pHDDev);
			m_bNvDxvaHD = true;
		}

		if (FAILED(hr))
		{
			DbgMsg("CD3D9Dxva2HDVideoMixer unable to create DXVA HD device. hr = 0x%x", hr);
			return hr;
		}

		hr = m_pHDDev->GetVideoProcessorDeviceCaps(&VPDevCaps);
		if (FAILED(hr))
			return hr;

		ASSERT(VPDevCaps.InputFormatCount > 0);
		ASSERT(VPDevCaps.VideoProcessorCount > 0);
		// we need at least 4 streams in current implementation.
		ASSERT(VPDevCaps.MaxInputStreams >= 4 && VPDevCaps.MaxStreamStates >= 4);

		m_uMaxInputStream = min(VPDevCaps.MaxInputStreams, PLANE_MAX);

		DXVAHD_VPCAPS VPCaps;
		DXVAHD_VPCAPS *pvpcaps = new DXVAHD_VPCAPS[VPDevCaps.VideoProcessorCount];
		hr = m_pHDDev->GetVideoProcessorCaps(VPDevCaps.VideoProcessorCount, pvpcaps);
		if (SUCCEEDED(hr))
		{
			// we use the first provided VP for now
			hr = m_pHDDev->CreateVideoProcessor(&pvpcaps[0].VPGuid, &m_pHDVP);
			VPCaps = pvpcaps[0];
		}

		delete [] pvpcaps;
		if (FAILED(hr))
			return hr;

		D3DFORMAT *pFormats = new D3DFORMAT[VPDevCaps.InputFormatCount];
		hr = m_pHDDev->GetVideoProcessorInputFormats(VPDevCaps.InputFormatCount, pFormats);
		m_setValidInputFormat = std::set<D3DFORMAT>(pFormats, pFormats + VPDevCaps.InputFormatCount);
		delete [] pFormats;

#ifdef _DEBUG
		pFormats = new D3DFORMAT[VPDevCaps.OutputFormatCount];
		hr = m_pHDDev->GetVideoProcessorOutputFormats(VPDevCaps.OutputFormatCount, pFormats);
		delete [] pFormats;
#endif

		GenerateDxva2VPList();

		VideoProcessorStub vp = {0};
		DXVAHD_FILTER_RANGE_DATA range;

		vp.sCaps.eType = PROCESSOR_TYPE_HARDWARE;
		vp.guidVP = DispSvr_VideoProcDxvaHD;
		vp.RenderTargetFormat = D3DFMT_X8R8G8B8;
		if (VPDevCaps.FilterCaps & DXVAHD_FILTER_CAPS_BRIGHTNESS)
		{			
			hr = m_pHDDev->GetVideoProcessorFilterRange(DXVAHD_FILTER_BRIGHTNESS, &range);
			if (SUCCEEDED(hr))
			{
				vp.sCaps.uFilterCaps |= FILTER_CAPS_BRIGHTNESS;
				DXVAHD_ValueRange2ValueRange(range, vp.FilterRanges[VIDEO_FILTER_BRIGHTNESS]);
			}
		}
		if (VPDevCaps.FilterCaps & DXVAHD_FILTER_CAPS_CONTRAST)
		{
			hr = m_pHDDev->GetVideoProcessorFilterRange(DXVAHD_FILTER_CONTRAST, &range);
			if (SUCCEEDED(hr))
			{
				vp.sCaps.uFilterCaps |= FILTER_CAPS_CONTRAST;
				DXVAHD_ValueRange2ValueRange(range, vp.FilterRanges[VIDEO_FILTER_CONTRAST]);
			}
		}
		if (VPDevCaps.FilterCaps & DXVAHD_FILTER_CAPS_HUE)
		{
			hr = m_pHDDev->GetVideoProcessorFilterRange(DXVAHD_FILTER_HUE, &range);
			if (SUCCEEDED(hr))
			{
				vp.sCaps.uFilterCaps |= FILTER_CAPS_HUE;
				DXVAHD_ValueRange2ValueRange(range, vp.FilterRanges[VIDEO_FILTER_HUE]);
			}
		}
		if (VPDevCaps.FilterCaps & DXVAHD_FILTER_CAPS_SATURATION)
		{
			hr = m_pHDDev->GetVideoProcessorFilterRange(DXVAHD_FILTER_SATURATION, &range);
			if (SUCCEEDED(hr))
			{
				vp.sCaps.uFilterCaps |= FILTER_CAPS_SATURATION;
				DXVAHD_ValueRange2ValueRange(range, vp.FilterRanges[VIDEO_FILTER_SATURATION]);
			}			
		}
		if (VPDevCaps.FilterCaps & DXVAHD_FILTER_CAPS_NOISE_REDUCTION)
			vp.sCaps.uFilterCaps |= FILTER_CAPS_NOISE_REDUCTION;
		if (VPDevCaps.FilterCaps & DXVAHD_FILTER_CAPS_EDGE_ENHANCEMENT)
			vp.sCaps.uFilterCaps |= FILTER_CAPS_EDGE_ENHANCEMENT;

		vp.sCaps.uNumBackwardSamples = VPCaps.PastFrames > MAX_PAST_FRAMES ? MAX_PAST_FRAMES : VPCaps.PastFrames;
		vp.sCaps.uNumForwardSamples = VPCaps.FutureFrames;

		if (VPCaps.ProcessorCaps & DXVAHD_PROCESSOR_CAPS_DEINTERLACE_MOTION_COMPENSATION)
			vp.sCaps.uProcessorCaps = PROCESSOR_CAPS_DEINTERLACE_MOTION_COMPENSATION;
		else if (VPCaps.ProcessorCaps & DXVAHD_PROCESSOR_CAPS_DEINTERLACE_ADAPTIVE)
			vp.sCaps.uProcessorCaps = PROCESSOR_CAPS_DEINTERLACE_ADAPTIVE;
		else if (VPCaps.ProcessorCaps & DXVAHD_PROCESSOR_CAPS_DEINTERLACE_BLEND)
			vp.sCaps.uProcessorCaps = PROCESSOR_CAPS_DEINTERLACE_BLEND;
		else if (VPCaps.ProcessorCaps & DXVAHD_PROCESSOR_CAPS_DEINTERLACE_BOB)
			vp.sCaps.uProcessorCaps = PROCESSOR_CAPS_DEINTERLACE_BOB;

		// Look for one VP in the list which provides VBltFactory in case we want to separate video
		// processing from DXVA HD.
		for (VideoProcessorList::iterator vi = m_VideoProcessorList.begin(); vi != m_VideoProcessorList.end(); ++vi)
			if (vi->pfnVBltFactory)
			{
				vp.pDelegateVPStub = &*vi;
				break;
			}

		m_VideoProcessorList.push_back(vp);
		ASSERT(SUCCEEDED(hr));
		SelectVideoProcessor();

		hr = m_pTexturePool->SetPoolUsage(TEXTURE_POOL_USAGE_OFFSCREENPLANESURFACE);
		SetDefaultStates();
	}

	return hr;
}

STDMETHODIMP CD3D9Dxva2HDVideoMixer::_ReleaseDevice()
{
	{
		CAutoLock selfLock(&m_csObj);
		SAFE_RELEASE(m_pHDVP);
		SAFE_RELEASE(m_pHDDev);
		ZeroMemory(m_StreamData, sizeof(m_StreamData));
		SetDefaultStates();
		m_uMaxInputStream = 0;
	}

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
	if (!m_pHDVP || !pDestSurface)
		return E_FAIL;

	HRESULT hr = VideoProcessBltHD(pDestSurface, rcDst, rcDstClip);
	return hr;
}

HRESULT CD3D9Dxva2HDVideoMixer::VideoProcessBltHD(IDirect3DSurface9 *pDst, const RECT &rcDst, const RECT &rcDstClip)
{
	HRESULT hr;
	StreamDataCache cache;
	D3DSURFACE_DESC desc;
	UINT uChangedStream = 0;
	UINT uStreamID = 0;
	bool pSkipStream[PLANE_MAX] = {0};

	// Background and sub-video can not co-exist, so we skip sub-video if background is visible, and vice versa.
	// This way we can use maximum 4 streams for BD and do not change stream states often.
	pSkipStream[PLANE_BACKGROUND] = !IsBackgroundVisible();
	pSkipStream[PLANE_SUBVIDEO] = !pSkipStream[PLANE_BACKGROUND];

	// stream data, and stream data store are cleared every frame while
	// stream data cache isn't.
	ZeroMemory(m_StreamData, sizeof(m_StreamData));
	ZeroMemory(m_StreamDataStore, sizeof(m_StreamDataStore));

	for (UINT PlaneID = 0; PlaneID < PLANE_MAX && uStreamID < m_uMaxInputStream; PlaneID++)
	{
		if (pSkipStream[PlaneID])
			continue;

		DXVAHD_STREAM_DATA &sStreamData = m_StreamData[uStreamID];
		const D3D9Plane &plane = m_Planes[PlaneID];

		hr = IsD3D9PlaneValid(&plane) ? S_OK : E_FAIL;
		while (SUCCEEDED(hr))
		{
			hr = m_pTexturePool->GetRepresentation(plane.hTexture, IID_IDirect3DSurface9, (void **)&sStreamData.pInputSurface);
			if (FAILED(hr))
				break;

			sStreamData.pInputSurface->Release();
			hr = sStreamData.pInputSurface->GetDesc(&desc);
			if (FAILED(hr))
				break;

			// make a local copy of stream data cache.
			memcpy(&cache, &m_StreamDataCaches[uStreamID], sizeof(cache));

			if (plane.bFullScreenMixing && m_eWindowState == RESOURCE_WINDOW_STATE_FULLSCREEN)
				cache.DestinationRect = m_rcMixingDstClip;
			else
				cache.DestinationRect = rcDst;
			cache.SourceRect = plane.rcSrc;
			cache.d3dSurfaceFormat = desc.Format;

			// Pass the plane ID, stream data store, stream data, and a local copy of stream data cache.
			// Once local cahce is filled by pfnStreamSetter function, we then compare with the stream data cache
			// with the local cache and synchronize with the runtime.
			hr = (this->*m_pfnStreamSetter[PlaneID])(PlaneID, m_StreamDataStore[uStreamID], sStreamData, cache);
			if (FAILED(hr))
				break;

			hr = PlaneToScreen(plane, cache.SourceRect, cache.DestinationRect, rcDstClip);
			break;
		}

		if (FAILED(hr))
		{
			// Workaround for NV DXVA HD implementation, because DXVA2 runtime
			// checks input surface, source/destination rectangle on Vista.
			if (m_bNvDxvaHD)
			{
				D3DSURFACE_DESC DummyDesc;
				if (FAILED(pDst->GetDesc(&DummyDesc)))
					ASSERT(0);

				memcpy(&cache, &m_StreamDataCaches[uStreamID], sizeof(cache));
				cache.d3dSurfaceFormat = DummyDesc.Format;
				SetRect(&cache.SourceRect, 0, 0, DummyDesc.Width, DummyDesc.Height);
				SetRect(&cache.DestinationRect, 0, 0, DummyDesc.Width, DummyDesc.Height);
				hr = SyncStreamState(uStreamID, cache);

				// If no surface is available, we still need to assign a valid surface for VPBltHD 
				// with some necessary stream state 
				// on the other hand, DXVAHD won't composite because one of the streams has a invalid surface.
				sStreamData.pInputSurface = pDst;
			}

			sStreamData.Enable = FALSE;
			sStreamData.InputFrameOrField = 0;
			m_StreamDataCaches[uStreamID].uInputFrameOrField = 0;
		}
		else
		{
			// InputFrameOrField indicates if the stream has new data to be processed.
			// We should reset it to zero when chapter changes.
			if (plane.dwLastUpdateTS != cache.dwLastUpdateTS)
			{
				m_StreamDataCaches[uStreamID].dwLastUpdateTS = plane.dwLastUpdateTS;
				if (plane.dwLastUpdateTS < cache.dwLastUpdateTS)
					m_StreamDataCaches[uStreamID].uInputFrameOrField = 0;
				else
					m_StreamDataCaches[uStreamID].uInputFrameOrField++;

				sStreamData.InputFrameOrField = m_StreamDataCaches[uStreamID].uInputFrameOrField;
				uChangedStream++;
			}
			sStreamData.Enable = TRUE;
			// synchronize between runtime and our cache if there is any change.
			hr = SyncStreamState(uStreamID, cache);
		}

		uStreamID++;
	}

	ASSERT(m_uMaxInputStream >= uStreamID);
	hr = SetVPBltHDStates(rcDst, rcDstClip);
	if (uChangedStream > 0)
		m_uiFrameCount++;
	hr = m_pHDVP->VideoProcessBltHD(pDst, m_uiFrameCount, uStreamID, m_StreamData);
	if (FAILED(hr))
		DbgMsg("CD3D9Dxva2HDVideoMixer::VideoProcessBltHD failed. hr=0x%x", hr);
	
	return hr;
}

HRESULT CD3D9Dxva2HDVideoMixer::SetSingleVideoStreamStates(PLANE_ID PlaneID, StreamDataStore &sDataStore, DXVAHD_STREAM_DATA &sStreamData, StreamDataCache &cache)
{
	HRESULT hr = S_OK;
	const D3D9Plane &plane = m_Planes[PlaneID];

	ZeroMemory(&cache.InputColorSpaceData, sizeof(cache.InputColorSpaceData));	// zero the reserved bits
	// Only either RGB or YCbCr flags corresponding to the color space of the input format will be referred. 
	cache.InputColorSpaceData.Type = 0;			// 0:Video, 1:Graphics
	cache.InputColorSpaceData.RGB_Range = 0;		// 0:Full(0-255),  1:Limited(16-235)
	cache.InputColorSpaceData.YCbCr_Matrix = plane.bHDVideo ? 1 : 0;	// 0:BT.601(SDTV), 1:BT.709(HDTV)
	cache.InputColorSpaceData.YCbCr_xvYCC = GetRegistry(REG_DISPLAY_XVYCC_GAMUT, FALSE) ? 1 : 0;	// 0:Conventional, 1:Expanded(xvYCC)

	if (PlaneID == PLANE_SUBVIDEO)
	{
		cache.AlphaData.Enable = plane.fAlpha < 1.0f ? TRUE : FALSE;
		cache.AlphaData.Alpha = plane.fAlpha;
		cache.LumaKey = m_LumaKey;
	}
	else
	{
		cache.AlphaData.Enable = FALSE;
		cache.AlphaData.Alpha = plane.fAlpha;
	}

	// Cache Color Control Settings
	if (PlaneID == PLANE_MAINVIDEO)
	{
		const VideoProcessorStub *pVP = m_PlaneConfigs[PLANE_MAINVIDEO].pVideoProcessorStub;
		for (int i=0; i<=VIDEO_FILTER_SATURATION; i++)
		{
			if (cache.FilterData[i].Level != INT(pVP->fFilterValue[i]))
			{
				cache.FilterData[i].Enable = ((1 << i) & pVP->sCaps.uFilterCaps)?TRUE:FALSE;
				cache.FilterData[i].Level = INT(pVP->fFilterValue[i]);				
			}
		}
	}

	if (plane.VideoProperty.Format == VIDEO_FORMAT_PROGRESSIVE)
		cache.FrameFormat = DXVAHD_FRAME_FORMAT_PROGRESSIVE;
	else if (plane.VideoProperty.Format == VIDEO_FORMAT_FIELD_INTERLEAVED_EVEN_FIRST)
		cache.FrameFormat = DXVAHD_FRAME_FORMAT_INTERLACED_TOP_FIELD_FIRST;
	else if (plane.VideoProperty.Format == VIDEO_FORMAT_FIELD_INTERLEAVED_ODD_FIRST)
		cache.FrameFormat = DXVAHD_FRAME_FORMAT_INTERLACED_BOTTOM_FIELD_FIRST;

	// video effect pipeline
    CComPtr<IDirect3DTexture9> pTexture;
    CComPtr<IUnknown> pUnk;
    if (PlaneID == PLANE_MAINVIDEO && (ProcessVideoEffect(PLANE_MAINVIDEO, NULL, cache.SourceRect, cache.DestinationRect, plane.Format, &pUnk) != S_FALSE))
    {
        if (pUnk)
        {
			D3DSURFACE_DESC desc;
            hr = pUnk->QueryInterface(IID_IDirect3DTexture9, (void **)&pTexture);
            if (SUCCEEDED(hr))
            {
                hr = pTexture->GetSurfaceLevel(0, &sStreamData.pInputSurface);	ASSERT(SUCCEEDED(hr));
#ifndef _NO_USE_D3DXDLL
                if (0)
                {
                    hr = D3DXSaveSurfaceToFile(_T("C:\\reRT.bmp"), D3DXIFF_BMP, sStreamData.pInputSurface, NULL, NULL);
                }
#endif
            }
            else
            {
                hr = pUnk->QueryInterface(IID_IDirect3DSurface9, (void **)&sStreamData.pInputSurface);
#ifndef _NO_USE_D3DXDLL
                if (0)
                {
                    hr = D3DXSaveSurfaceToFile(_T("C:\\doSurface.bmp"), D3DXIFF_BMP, sStreamData.pInputSurface, NULL, NULL);
                }
#endif
            }
            hr = sStreamData.pInputSurface->GetDesc(&desc);	ASSERT(SUCCEEDED(hr));
            cache.d3dSurfaceFormat = desc.Format;
            cache.FrameFormat = DXVAHD_FRAME_FORMAT_PROGRESSIVE;
        }
    }
	else
	{
		// process de-interlacing and color space conversion by calling VideoProcessBlt()
		if ((m_bNvDxvaHD && plane.VideoProperty.Format != VIDEO_FORMAT_PROGRESSIVE)
			|| m_setValidInputFormat.find(cache.d3dSurfaceFormat) == m_setValidInputFormat.end())
		{
			CComPtr<IDirect3DSurface9> pIntermediateSurface;
			hr = plane.pVBlt->IntermediateVBlt(sStreamData.pInputSurface, cache.SourceRect, PLANE_FORMAT_NV12, &pIntermediateSurface);
			if (FAILED(hr))
				return hr;

			sStreamData.pInputSurface = pIntermediateSurface;
			cache.d3dSurfaceFormat = static_cast<D3DFORMAT> (PLANE_FORMAT_NV12);
			cache.FrameFormat = DXVAHD_FRAME_FORMAT_PROGRESSIVE;
		}
		else if (PlaneID == PLANE_MAINVIDEO)
		{
			UINT Count = 0;
            // start from the first backward reference sample
            VideoSampleList::const_iterator it = plane.VideoSamples.begin() + plane.uiPlaneSampleIndex + 1;
            for (;it != plane.VideoSamples.end() && Count < MAX_PAST_FRAMES;++it)
			{
				if (SUCCEEDED(m_pTexturePool->GetRepresentation(it->hTexture, __uuidof(IDirect3DSurface9),
					(void **) &sDataStore.pPastFrames[Count])))
				{
					sDataStore.pPastFrames[Count]->Release();
					++Count;
				}
			}

			if (Count > 0)
			{
				sStreamData.ppPastSurfaces = sDataStore.pPastFrames;
				sStreamData.PastFrames = Count;
			}
		}
	}

	// OutputIndex selects the result from deinterlacing.
	// When the FrameFormat is progressive, we should always select the index 0.
	// Otherwise black or undefined video output may be selected and shown.
	if (DXVAHD_FRAME_FORMAT_PROGRESSIVE == cache.FrameFormat)
	{
		sStreamData.OutputIndex = 0;
	}
	else
	{
		if (FIELD_SELECT_SECOND == plane.VideoProperty.dwFieldSelect)
			sStreamData.OutputIndex = 1;
		else if (FIELD_SELECT_REPEAT_FIRST == plane.VideoProperty.dwFieldSelect)
			sStreamData.OutputIndex = 2;
		else
			sStreamData.OutputIndex = 0;
	}

#ifdef _DEBUG_OUTPUT
	DbgMsg("Video[%d] Format = %d, OutputIndex = %d, PastFrames = %d, FutureFrames = %d",
		PlaneID == PLANE_SUBVIDEO, cache.FrameFormat,
		pStreamData->OutputIndex, pStreamData->PastFrames, pStreamData->FutureFrames);
#endif // _DEBUG_OUTPUT
	return hr;
}

HRESULT CD3D9Dxva2HDVideoMixer::SetSingleStreamStates(PLANE_ID PlaneID, StreamDataStore &sDataStore, DXVAHD_STREAM_DATA &sStreamData, StreamDataCache &cache)
{
	HRESULT hr = S_OK;
	const D3D9Plane &plane = m_Planes[PlaneID];

	ZeroMemory(&cache.InputColorSpaceData, sizeof(cache.InputColorSpaceData));	// zero the reserved bits
	cache.InputColorSpaceData.Type = 1;	// 0:Video, 1:Graphics

	// Set Stream State (Alpha), default is FALSE, 1.0
	if (PlaneID != PLANE_BACKGROUND)
	{
		cache.AlphaData.Enable = TRUE;
		cache.AlphaData.Alpha = plane.fAlpha;
	}
	else
	{
		cache.AlphaData.Enable = FALSE;
		cache.AlphaData.Alpha = 0.f;
	}

	if (plane.bPalettized)
	{
		cache.uPaletteCount = 256;
		memcpy(cache.Palette, plane.Palette, sizeof(cache.Palette));
	}
	else
	{
		cache.uPaletteCount = 0;
	}

	return hr;
}

HRESULT CD3D9Dxva2HDVideoMixer::SyncStreamState(UINT uiStreamNum, const StreamDataCache &data)
{
	HRESULT hr = S_OK;
	UINT uStateSetTimes = 0;
	StreamDataCache &cache = m_StreamDataCaches[uiStreamNum];

	if (cache.d3dSurfaceFormat != data.d3dSurfaceFormat)
	{
		// Set Stream State (D3DFormat)
		DXVAHD_STREAM_STATE_D3DFORMAT_DATA strStateD3DFormat;
		strStateD3DFormat.Format = data.d3dSurfaceFormat;
		hr = SET_STREAM_STATE(uiStreamNum, DXVAHD_STREAM_STATE_D3DFORMAT, &strStateD3DFormat);
		uStateSetTimes++;
	}

	if (memcmp(&cache.InputColorSpaceData, &data.InputColorSpaceData, sizeof(data.InputColorSpaceData)) != 0)
	{
		// Set Stream State (Input Color Space)
		// Only either RGB or YCbCr flags corresponding to the color space of the input format will be referred. 
		hr = SET_STREAM_STATE(uiStreamNum, DXVAHD_STREAM_STATE_INPUT_COLOR_SPACE, &data.InputColorSpaceData);
		uStateSetTimes++;
	}

	if (!EqualRect(&cache.SourceRect, &data.SourceRect))
	{
		// Set Stream State (Source RECT)
		DXVAHD_STREAM_STATE_SOURCE_RECT_DATA strStateSourceRECT;
		strStateSourceRECT.Enable = TRUE;
		strStateSourceRECT.SourceRect = data.SourceRect;
		hr = SET_STREAM_STATE(uiStreamNum, DXVAHD_STREAM_STATE_SOURCE_RECT, &strStateSourceRECT);
		uStateSetTimes++;
	}
	
	if (!EqualRect(&cache.DestinationRect, &data.DestinationRect))
	{
		// Set Stream State (Destination RECT)
		DXVAHD_STREAM_STATE_DESTINATION_RECT_DATA dstRect;
		dstRect.Enable = TRUE;
		dstRect.DestinationRect = data.DestinationRect;
		hr = SET_STREAM_STATE(uiStreamNum, DXVAHD_STREAM_STATE_DESTINATION_RECT, &dstRect);
		uStateSetTimes++;
	}

	// Set Stream State (Alpha), default is FALSE, 1.0
	if (memcmp(&cache.AlphaData, &data.AlphaData, sizeof(data.AlphaData)) != 0)
	{
		hr = SET_STREAM_STATE(uiStreamNum, DXVAHD_STREAM_STATE_ALPHA, &data.AlphaData);
		uStateSetTimes++;
	}

	if (memcmp(&cache.LumaKey, &data.LumaKey, sizeof(data.LumaKey)) != 0)
	{
		// Set Stream State (Luma Key)
		DXVAHD_STREAM_STATE_LUMA_KEY_DATA strStateLumaKey;

		strStateLumaKey.Enable = data.LumaKey.bEnable;
		strStateLumaKey.Lower = FLOAT(data.LumaKey.uLower) / 255;
		strStateLumaKey.Upper = FLOAT(data.LumaKey.uUpper) / 255;
		hr = SET_STREAM_STATE(uiStreamNum, DXVAHD_STREAM_STATE_LUMA_KEY, &strStateLumaKey);
		uStateSetTimes++;
	}

	// Set Stream State (Palette data)
	if (data.uPaletteCount != cache.uPaletteCount
		|| (data.uPaletteCount > 0 && memcmp(cache.Palette, data.Palette, sizeof(cache.Palette[0]) * data.uPaletteCount) != 0))
	{
		DXVAHD_STREAM_STATE_PALETTE_DATA strStatePalette;
		strStatePalette.Count = data.uPaletteCount;
		if (strStatePalette.Count > 0)
		{
			strStatePalette.pEntries = new D3DCOLOR[strStatePalette.Count];
			for (UINT i = 0; i < strStatePalette.Count; i++)
			{
				strStatePalette.pEntries[i] = AYUVSample8ToD3DCOLOR(data.Palette[i]);
			}
			hr = SET_STREAM_STATE(uiStreamNum, DXVAHD_STREAM_STATE_PALETTE, &strStatePalette);
			delete [] strStatePalette.pEntries;
		}
		else
		{
			strStatePalette.pEntries = 0;
			hr = SET_STREAM_STATE(uiStreamNum, DXVAHD_STREAM_STATE_PALETTE, &strStatePalette);
		}

		uStateSetTimes++;
	}

	if (data.FrameFormat != cache.FrameFormat)
	{
		DXVAHD_STREAM_STATE_FRAME_FORMAT_DATA sFrameFormat = { data.FrameFormat };
		hr = SET_STREAM_STATE(uiStreamNum, DXVAHD_STREAM_STATE_FRAME_FORMAT, &sFrameFormat);
		uStateSetTimes++;
	}

	// Set Stream State (Brightness Filter)
	if (cache.FilterData[VIDEO_FILTER_BRIGHTNESS].Level != data.FilterData[VIDEO_FILTER_BRIGHTNESS].Level)
	{
	DXVAHD_STREAM_STATE_FILTER_DATA strStateBrightness;
	strStateBrightness.Enable = data.FilterData[VIDEO_FILTER_BRIGHTNESS].Enable;
	strStateBrightness.Level = data.FilterData[VIDEO_FILTER_BRIGHTNESS].Level;
	hr = SET_STREAM_FILTER_STATE(uiStreamNum, DXVAHD_STREAM_STATE_FILTER_BRIGHTNESS, &strStateBrightness);
	uStateSetTimes++;
	}

	// Set Stream State (Contrast Filter)
	if (cache.FilterData[VIDEO_FILTER_CONTRAST].Level != data.FilterData[VIDEO_FILTER_CONTRAST].Level)
	{
	DXVAHD_STREAM_STATE_FILTER_DATA strStateContrast;
	strStateContrast.Enable = data.FilterData[VIDEO_FILTER_CONTRAST].Enable;
	strStateContrast.Level = data.FilterData[VIDEO_FILTER_CONTRAST].Level;
	hr = SET_STREAM_FILTER_STATE(uiStreamNum, DXVAHD_STREAM_STATE_FILTER_CONTRAST, &strStateContrast);
	uStateSetTimes++;
	}

	// Set Stream State (Hue Filter)
	if (cache.FilterData[VIDEO_FILTER_HUE].Level != data.FilterData[VIDEO_FILTER_HUE].Level)
	{
	DXVAHD_STREAM_STATE_FILTER_DATA strStateHue;
	strStateHue.Enable = data.FilterData[VIDEO_FILTER_HUE].Enable;
	strStateHue.Level = data.FilterData[VIDEO_FILTER_HUE].Level;
	hr = SET_STREAM_FILTER_STATE(uiStreamNum, DXVAHD_STREAM_STATE_FILTER_HUE, &strStateHue);
	uStateSetTimes++;
	}

	// Set Stream State (Saturation Filter)
	if (cache.FilterData[VIDEO_FILTER_SATURATION].Level != data.FilterData[VIDEO_FILTER_SATURATION].Level)
	{
	DXVAHD_STREAM_STATE_FILTER_DATA strStateSaturation;
	strStateSaturation.Enable = data.FilterData[VIDEO_FILTER_SATURATION].Enable;
	strStateSaturation.Level = data.FilterData[VIDEO_FILTER_SATURATION].Level;
	hr = SET_STREAM_FILTER_STATE(uiStreamNum, DXVAHD_STREAM_STATE_FILTER_SATURATION, &strStateSaturation);
	uStateSetTimes++;
	}

	/*
	// Set Stream State (Noise Reduction Filter)
	DXVAHD_STREAM_STATE_FILTER_DATA strStateNoiseReduction;
	strStateNoiseReduction.Enable = FALSE;
	strStateNoiseReduction.Level = 0;
	hr = SET_STREAM_FILTER_STATE(uiStreamNum, DXVAHD_STREAM_STATE_FILTER_NOISE_REDUCTION, &strStateNoiseReduction);

	// Set Stream State (Edge Enhancement Filter)
	DXVAHD_STREAM_STATE_FILTER_DATA strStateEdgeEnhancement;
	strStateEdgeEnhancement.Enable = FALSE;
	strStateEdgeEnhancement.Level = 0;
	hr = SET_STREAM_FILTER_STATE(uiStreamNum, DXVAHD_STREAM_STATE_FILTER_EDGE_ENHANCEMENT, &strStateEdgeEnhancement);

	// Set Stream State (Anamorphic Scaling Filter)
	DXVAHD_STREAM_STATE_FILTER_DATA strStateAnamorphicsScaling;
	strStateAnamorphicsScaling.Enable = FALSE;
	strStateAnamorphicsScaling.Level = 32;
	hr = SET_STREAM_FILTER_STATE(uiStreamNum, DXVAHD_STREAM_STATE_FILTER_ANAMORPHIC_SCALING, &strStateAnamorphicsScaling);
	*/
	if (uStateSetTimes > 0)
		memcpy(&cache, &data, sizeof(data));
	return hr;
}

HRESULT CD3D9Dxva2HDVideoMixer::SetVPBltHDStates(const RECT &rcDst, const RECT &rcDstClip)
{
	HRESULT hr = S_OK;
	BltStateCache data = m_BltStateCache;
	UINT uStateSetTimes = 0;

	data.TargetRect = rcDstClip;
	ZeroMemory(&data.OutputColorSpaceData, sizeof(data.OutputColorSpaceData));
	data.OutputColorSpaceData.Usage = 0;				// 0:Playback,     1:Processing
	data.OutputColorSpaceData.RGB_Range = 0;			// 0:Full(0-255),  1:Limited(16-235)
	data.OutputColorSpaceData.YCbCr_Matrix =
		GetRegistry(REG_DISPLAY_XVYCC_MONITOR_TYPE, DISPLAY_XVYCC_MONITOR_NOT_SUPPORT) == DISPLAY_XVYCC_MONITOR_BT709 ? 1 : 0;		// 0:BT.601(SDTV), 1:BT.709(HDTV)
	data.OutputColorSpaceData.YCbCr_xvYCC = GetRegistry(REG_DISPLAY_XVYCC_GAMUT, FALSE) ? 1 : 0;		// 0:Conventional, 1:Expanded(xvYCC)

	if (memcmp(&m_BltStateCache.TargetRect, &data.TargetRect, sizeof(RECT)) != 0)
	{
		// Set Blt State (Target RECT)
		DXVAHD_BLT_STATE_TARGET_RECT_DATA bltStateTargetRECT;
		bltStateTargetRECT.Enable = TRUE;
		bltStateTargetRECT.TargetRect = data.TargetRect;
		hr = SET_BLT_STATE(DXVAHD_BLT_STATE_TARGET_RECT, &bltStateTargetRECT);
		uStateSetTimes++;
	}

	if (memcmp(&m_BltStateCache.OutputColorSpaceData, &data.OutputColorSpaceData, sizeof(data.OutputColorSpaceData)) != 0)
	{
		// Set Blt State (Output Color Space)
		// Only either RGB or YCbCr flag corresponding to the color space of the output format will be referred.
		hr = SET_BLT_STATE(DXVAHD_BLT_STATE_OUTPUT_COLOR_SPACE, &data.OutputColorSpaceData);
		uStateSetTimes++;
	}

	if (uStateSetTimes > 0)
		memcpy(&m_BltStateCache, &data, sizeof(data));
	return hr;
}

void CD3D9Dxva2HDVideoMixer::SetDefaultStates()
{
	ZeroMemory(&m_BltStateCache, sizeof(m_BltStateCache));
	ZeroMemory(m_StreamDataCaches, sizeof(m_StreamDataCaches));
}

STDMETHODIMP CD3D9Dxva2HDVideoMixer::SetClearRectangles(UINT uCount, ClearRect *pRects)
{
	return E_NOTIMPL;
}
