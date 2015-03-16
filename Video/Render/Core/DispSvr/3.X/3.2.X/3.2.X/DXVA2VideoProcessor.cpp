#include "stdafx.h"
#include "DXVA2VideoProcessor.h"

#define MAX_DXVA2_BACKWARD_SAMPLES 2
#define MAX_DXVA2_FORWARD_SAMPLES 0		// we don't handle forward reference sample now.

#define DXVA2_DI_MONTION_COMPENSATION_MASK DXVA2_DeinterlaceTech_MotionVectorSteered
#define DXVA2_DI_ADAPTIVE_MASK (DXVA2_DeinterlaceTech_EdgeFiltering | DXVA2_DeinterlaceTech_FieldAdaptive | DXVA2_DeinterlaceTech_PixelAdaptive)
#define DXVA2_DI_BLEND_MASK (DXVA2_DeinterlaceTech_BOBVerticalStretch | DXVA2_DeinterlaceTech_BOBVerticalStretch4Tap | DXVA2_DeinterlaceTech_MedianFiltering)
#define DXVA2_DI_BOB_MASK DXVA2_DeinterlaceTech_BOBLineReplicate

using namespace DispSvr;

static const DXVA2_ExtendedFormat s_dxva2DefaultSampleFormat =
{
	DXVA2_SampleFieldInterleavedEvenFirst,	// we may switch between interleaved or progressive frames during decoding.
	DXVA2_VideoChromaSubsampling_MPEG2,
	DXVA2_NominalRange_16_235,
	DXVA2_VideoTransferMatrix_Unknown,	// = DXVA2_VideoTransferMatrix_BT601 for SD, DXVA2_VideoTransferMatrix_BT709 for HD
	DXVA2_VideoLighting_Unknown,		// = DXVA2_VideoLighting_dim
	DXVA2_VideoPrimaries_Unknown,		// = DXVA2_VideoPrimaries_BT709
	DXVA2_VideoTransFunc_Unknown		// = DXVA2_VideoTransFunc_22_709
};

static const DXVA2_AYUVSample16 s_dxva2DefaultBackgroundColor =
{
	32640,	// Cr
	32640,	// Cb
	4080,	// Y
	65535	// Alpha
};

static const DXVA2_Fixed32 s_dxva2AlphaOpaque =
{
	0,	// fraction
	1	// value
};

static void DXVA2_ValueRange2ValueRange(const DXVA2_ValueRange &d, ValueRange &v)
{
	v.fDefaultValue = DXVA2FixedToFloat(d.DefaultValue);
	v.fMaxValue = DXVA2FixedToFloat(d.MaxValue);
	v.fMinValue = DXVA2FixedToFloat(d.MinValue);
	v.fStepSize = DXVA2FixedToFloat(d.StepSize);
}

static DXVA2_Fixed32 LinearFromDXVA2_ValueRange(const DXVA2_ValueRange &d, float v)
{
	return DXVA2FloatToFixed(DXVA2FixedToFloat(d.MinValue) + (DXVA2FixedToFloat(d.MaxValue) - DXVA2FixedToFloat(d.MinValue)) * v);
}

CDXVA2VideoProcessor::CDXVA2VideoProcessor(REFGUID refguid, D3DFORMAT rtFormat) : m_pVP(0), m_guidDesired(refguid), m_renderTargetFormat(rtFormat)
{
	Release();
}

CDXVA2VideoProcessor::~CDXVA2VideoProcessor()
{
	Release();
}

void CDXVA2VideoProcessor::Release()
{
	ZeroMemory(&m_VideoDesc, sizeof(m_VideoDesc));
	ZeroMemory(&m_rangeFilter, sizeof(m_rangeFilter));
	if (m_pVP)
	{
		int ref = m_pVP->Release();
		ASSERT(ref == 0);
		m_pVP = 0;
	}
}

HRESULT CDXVA2VideoProcessor::CreateVideoProcessor(IDirect3DSurface9 *pSrc, IDirect3DSurface9 *pDst, const VideoProperty &prop)
{
	CHECK_POINTER(CDynLibManager::GetInstance()->pfnDXVA2CreateVideoService);

	DXVA2_VideoDesc desc = {0};
	CComPtr<IDirectXVideoProcessorService> pVPService;
	CComPtr<IDirect3DDevice9> pDevice;
	D3DSURFACE_DESC d3dSurfaceDesc;
	HRESULT hr = pSrc->GetDevice(&pDevice);
	if (FAILED(hr))
		return hr;

	hr = pSrc->GetDesc(&d3dSurfaceDesc);
	if (FAILED(hr))
		return hr;

	desc.SampleWidth = prop.uWidth ? prop.uWidth : d3dSurfaceDesc.Width;
	desc.SampleHeight = prop.uHeight ? prop.uHeight : d3dSurfaceDesc.Height;
	desc.Format = d3dSurfaceDesc.Format;
	desc.SampleFormat = s_dxva2DefaultSampleFormat;
	if (prop.Format == VIDEO_FORMAT_PROGRESSIVE)
		desc.SampleFormat.SampleFormat = DXVA2_SampleProgressiveFrame;
	else if (prop.Format == VIDEO_FORMAT_FIELD_INTERLEAVED_ODD_FIRST)
		desc.SampleFormat.SampleFormat = DXVA2_SampleFieldInterleavedOddFirst;
	desc.InputSampleFreq.Numerator = prop.dwFrameRate1000;
	desc.InputSampleFreq.Denominator = 1000;
	desc.OutputFrameFreq = desc.InputSampleFreq;
	if (prop.Format != VIDEO_FORMAT_PROGRESSIVE)
		desc.OutputFrameFreq.Numerator *= 2;
	desc.UABProtectionLevel = 0;

	hr = CDynLibManager::GetInstance()->pfnDXVA2CreateVideoService(pDevice,	IID_IDirectXVideoProcessorService, (VOID**)&pVPService);
	if (FAILED(hr))
		return hr;

	hr = pVPService->CreateVideoProcessor(m_guidDesired, &desc, m_renderTargetFormat, 0, &m_pVP);
	// if failed, fallback to DXVA2_VideoProcBobDevice.
	if (FAILED(hr))
		hr = pVPService->CreateVideoProcessor(DXVA2_VideoProcBobDevice, &desc, m_renderTargetFormat, 0, &m_pVP);

	if (SUCCEEDED(hr))
	{
		for (int i = 1; i <= 12; i++)
			m_pVP->GetFilterPropertyRange(i, &m_rangeFilter[i]);

		m_VideoDesc = desc;
	}
	
	return hr;
}

HRESULT CDXVA2VideoProcessor::VideoProcessBlt(
	IDirect3DSurface9 *pSrc, const RECT &rcSrc,
	IDirect3DSurface9 *pDst, const RECT &rcDst,
	const VideoProperty &prop,
	DXVA2VP_RefSample *pRefSamples,
	UINT nRefSamples,
	float *pfFilterRange)
{
	DXVA2_VideoProcessBltParams BltParams;
	DXVA2_VideoSample Samples[1 + MAX_DXVA2_BACKWARD_SAMPLES + MAX_DXVA2_FORWARD_SAMPLES];
	UINT nSamples = 0;
	HRESULT hr = E_FAIL;

	ASSERT(pSrc && pDst);

	// Release dxva2 video processor if 
	// 1. video resolution change.
	// 2. video frame rate change.
	// 3. video format change.
	if ((m_VideoDesc.SampleWidth != prop.uWidth || m_VideoDesc.SampleHeight != prop.uHeight)
		|| (m_VideoDesc.InputSampleFreq.Numerator != prop.dwFrameRate1000)
		|| (m_VideoDesc.SampleFormat.SampleFormat == DXVA2_SampleProgressiveFrame && prop.Format != VIDEO_FORMAT_PROGRESSIVE)
		|| (m_VideoDesc.SampleFormat.SampleFormat != DXVA2_SampleProgressiveFrame && prop.Format == VIDEO_FORMAT_PROGRESSIVE))
		Release();

	if (!m_pVP)
	{
		hr = CreateVideoProcessor(pSrc, pDst, prop);
		if (FAILED(hr))
		{
			DbgMsg("CreateVideoProcessor failed. hr=0x%x", hr);
			ASSERT(0 && "CDXVA2VideoProcessor::CreateVideoProcessor failed.");
			return hr;
		}
	}

	ASSERT(m_pVP);

	// Prepare VideoProcessBlt parameters to the registration device
	memset(&BltParams, 0, sizeof(BltParams));
	memset(Samples, 0, sizeof(Samples));

	if (prop.Format != VIDEO_FORMAT_PROGRESSIVE && prop.dwFieldSelect == FIELD_SELECT_SECOND)
		BltParams.TargetFrame = (prop.rtStart + prop.rtEnd) / 2;
	else
		BltParams.TargetFrame = prop.rtStart;

	BltParams.TargetRect = rcDst;
	BltParams.StreamingFlags = 0;
	BltParams.ConstrictionSize.cx = 0;//rcDst.right - rcDst.left;
	BltParams.ConstrictionSize.cy = 0;//rcDst.bottom - rcDst.top;
	BltParams.BackgroundColor = s_dxva2DefaultBackgroundColor;
	BltParams.DestFormat = s_dxva2DefaultSampleFormat;
	//BltParams.DestFormat.SampleFormat = DXVA2_SampleProgressiveFrame;
	BltParams.Alpha = s_dxva2AlphaOpaque;
	BltParams.DestData = 0;

	BltParams.ProcAmpValues.Brightness = DXVA2FloatToFixed(pfFilterRange[VIDEO_FILTER_BRIGHTNESS]);
	BltParams.ProcAmpValues.Contrast = DXVA2FloatToFixed(pfFilterRange[VIDEO_FILTER_CONTRAST]);
	BltParams.ProcAmpValues.Hue = DXVA2FloatToFixed(pfFilterRange[VIDEO_FILTER_HUE]);
	BltParams.ProcAmpValues.Saturation = DXVA2FloatToFixed(pfFilterRange[VIDEO_FILTER_SATURATION]);

	BltParams.DetailFilterLuma.Level = LinearFromDXVA2_ValueRange(m_rangeFilter[DXVA2_DetailFilterLumaLevel], pfFilterRange[VIDEO_FILTER_EDGE_ENHANCEMENT]);
	BltParams.DetailFilterLuma.Radius = m_rangeFilter[DXVA2_DetailFilterLumaRadius].DefaultValue;
	BltParams.DetailFilterLuma.Threshold = m_rangeFilter[DXVA2_DetailFilterLumaThreshold].DefaultValue;
	BltParams.DetailFilterChroma.Level = m_rangeFilter[DXVA2_DetailFilterChromaLevel].DefaultValue;
	BltParams.DetailFilterChroma.Radius = m_rangeFilter[DXVA2_DetailFilterChromaRadius].DefaultValue;
	BltParams.DetailFilterChroma.Threshold = m_rangeFilter[DXVA2_DetailFilterChromaThreshold].DefaultValue;

	BltParams.NoiseFilterLuma.Level = LinearFromDXVA2_ValueRange(m_rangeFilter[DXVA2_NoiseFilterLumaLevel], pfFilterRange[VIDEO_FILTER_EDGE_ENHANCEMENT]);
	BltParams.NoiseFilterLuma.Radius = m_rangeFilter[DXVA2_NoiseFilterLumaRadius].DefaultValue;
	BltParams.NoiseFilterLuma.Threshold = m_rangeFilter[DXVA2_NoiseFilterLumaThreshold].DefaultValue;
	BltParams.NoiseFilterChroma.Level = m_rangeFilter[DXVA2_NoiseFilterChromaLevel].DefaultValue;
	BltParams.NoiseFilterChroma.Radius = m_rangeFilter[DXVA2_NoiseFilterChromaRadius].DefaultValue;
	BltParams.NoiseFilterChroma.Threshold = m_rangeFilter[DXVA2_NoiseFilterChromaThreshold].DefaultValue;

	// filling the backward reference sample(s) if any
    for (; nSamples < nRefSamples; nSamples++)
    {
        DXVA2_VideoSample &s = Samples[nSamples];
        const DXVA2VP_RefSample &ref = pRefSamples[nSamples];

        s.SampleFormat = BltParams.DestFormat;
        if (DXVA2VP_REFSAMPLE_TYPE_VIDEO == ref.eType)
        {
            s.Start        = ref.VideoProperty.rtStart;
            s.End          = ref.VideoProperty.rtEnd;
            if (ref.VideoProperty.Format == VIDEO_FORMAT_PROGRESSIVE)
                s.SampleFormat.SampleFormat = DXVA2_SampleProgressiveFrame;
            else if (ref.VideoProperty.Format == VIDEO_FORMAT_FIELD_INTERLEAVED_ODD_FIRST)
                s.SampleFormat.SampleFormat = DXVA2_SampleFieldInterleavedOddFirst;
            s.SrcRect      = rcSrc;
            s.DstRect      = BltParams.TargetRect;
        }
        else
        {
            s.Start        = prop.rtStart;
            s.End          = prop.rtEnd;
            s.SampleFormat.SampleFormat = DXVA2_SampleSubStream;
            s.SrcRect = ref.GraphicsProperty.rcSrc;
            s.DstRect = ref.GraphicsProperty.rcDst;
        }
        s.SrcSurface   = ref.pSurface;
        s.SampleData   = 0;
        s.PlanarAlpha  = s_dxva2AlphaOpaque;
    }

	//		DbgMsg("VideoProcessBlt TargetFrame: %I64d, Start: %I64d, End: %I64d",
	//			BltParams.TargetFrame, Samples[0].Start, Samples[0].End);

	hr = m_pVP->VideoProcessBlt(pDst, &BltParams, Samples, nSamples, NULL);

	// we should try to fix all failure cases
	ASSERT(SUCCEEDED(hr) && "CDXVA2VideoProcessor::VideoProcessBlt failed!!");
	return hr;
}

// Get a list of DXVA2VP_Caps, caller is responsible for releasing memory returned by ppCaps.
HRESULT CDXVA2VideoProcessor::GetDXVA2VPCaps(IDirect3DDevice9 *pDevice, UINT *puCount, DXVA2VP_Caps **ppCaps)
{
	HRESULT hr;
	DXVA2_VideoDesc desc = {0};
	D3DFORMAT fmtRenderTarget = D3DFMT_X8R8G8B8;
	CComPtr<IDirectXVideoProcessorService> pVPService;

	CHECK_POINTER(CDynLibManager::GetInstance()->pfnDXVA2CreateVideoService);
	CHECK_POINTER(puCount);
	CHECK_POINTER(ppCaps);

	*puCount = 0;
	*ppCaps = 0;

	CComPtr<IDirect3DSurface9> pRT;
	D3DSURFACE_DESC destSurfaceDesc;
	
	hr = pDevice->GetRenderTarget(0, &pRT);
	if (FAILED(hr))
		return hr;

	hr = pRT->GetDesc(&destSurfaceDesc);
	if (FAILED(hr))
		return hr;

	fmtRenderTarget = destSurfaceDesc.Format;

	// SD video may expose more VP with better capabilities while HD may not.
	// We fix render target format to NV12 becaue most VGAs support it.
	desc.SampleWidth = 720;
	desc.SampleHeight = 480;
	desc.Format = static_cast<D3DFORMAT> (PLANE_FORMAT_NV12);
	desc.SampleFormat = s_dxva2DefaultSampleFormat;
	desc.InputSampleFreq.Numerator = 30000;
	desc.InputSampleFreq.Denominator = 1001;
	desc.OutputFrameFreq.Numerator = 60000;
	desc.OutputFrameFreq.Denominator = 1001;
	desc.UABProtectionLevel = 0;

	hr = CDynLibManager::GetInstance()->pfnDXVA2CreateVideoService(pDevice,	IID_IDirectXVideoProcessorService, (VOID**)&pVPService);
	if (FAILED(hr))
		return hr;

	GUID *pDeviceGuids = NULL;
	UINT uCount = 0;

	hr = pVPService->GetVideoProcessorDeviceGuids(&desc, &uCount, &pDeviceGuids);
	if (FAILED(hr))
		return hr;

	ASSERT(uCount >= 2);	// at least progressive and bob devices, required by spec

	DXVA2VP_Caps *pCaps = *ppCaps = new DXVA2VP_Caps[uCount];
	if (pCaps)
	{
		DXVA2_VideoProcessorCaps caps;
		DXVA2_ValueRange range;

		ZeroMemory(pCaps, sizeof(DXVA2VP_Caps) * uCount);

		bool bGetVPCaps = false;
		while (!bGetVPCaps)
		{
			for (UINT i = 0; i < uCount; i++)
			{
				pCaps->guidVP = pDeviceGuids[i];
				// skip DXVA2_VideoProcSoftwareDevice or any failure
				if (pCaps->guidVP == DXVA2_VideoProcSoftwareDevice
					|| FAILED(pVPService->GetVideoProcessorCaps(pCaps->guidVP, &desc, fmtRenderTarget, &caps)))
					continue;
				pCaps->RenderTargetFormat = fmtRenderTarget;
				pCaps->sCaps.eType = caps.DeviceCaps == DXVA2_VPDev_SoftwareDevice ? PROCESSOR_TYPE_SOFTWARE : PROCESSOR_TYPE_HARDWARE;
				pCaps->sCaps.uNumBackwardSamples = caps.NumBackwardRefSamples;
				pCaps->sCaps.uNumForwardSamples = caps.NumForwardRefSamples;
				if (caps.DeinterlaceTechnology & DXVA2_DeinterlaceTech_MotionVectorSteered)
					pCaps->sCaps.uProcessorCaps = PROCESSOR_CAPS_DEINTERLACE_MOTION_COMPENSATION;
				else if (caps.DeinterlaceTechnology & DXVA2_DI_ADAPTIVE_MASK)
					pCaps->sCaps.uProcessorCaps = PROCESSOR_CAPS_DEINTERLACE_ADAPTIVE;
				else if (caps.DeinterlaceTechnology & DXVA2_DI_BLEND_MASK)
					pCaps->sCaps.uProcessorCaps = PROCESSOR_CAPS_DEINTERLACE_BLEND;
				else if (caps.DeinterlaceTechnology & DXVA2_DI_BOB_MASK)
					pCaps->sCaps.uProcessorCaps = PROCESSOR_CAPS_DEINTERLACE_BOB;

				if (caps.ProcAmpControlCaps & DXVA2_ProcAmp_Brightness)
				{
					hr = pVPService->GetProcAmpRange(pCaps->guidVP, &desc, fmtRenderTarget, DXVA2_ProcAmp_Brightness, &range);
					if (SUCCEEDED(hr))
					{
						pCaps->sCaps.uFilterCaps |= FILTER_CAPS_BRIGHTNESS;
						DXVA2_ValueRange2ValueRange(range, pCaps->FilterRanges[VIDEO_FILTER_BRIGHTNESS]);
					}
				}

				if (caps.ProcAmpControlCaps & DXVA2_ProcAmp_Contrast)
				{
					hr = pVPService->GetProcAmpRange(pCaps->guidVP, &desc, fmtRenderTarget, DXVA2_ProcAmp_Contrast, &range);
					if (SUCCEEDED(hr))
					{
						pCaps->sCaps.uFilterCaps |= FILTER_CAPS_CONTRAST;
						DXVA2_ValueRange2ValueRange(range, pCaps->FilterRanges[VIDEO_FILTER_CONTRAST]);
					}
				}

				if (caps.ProcAmpControlCaps & DXVA2_ProcAmp_Hue)
				{
					hr = pVPService->GetProcAmpRange(pCaps->guidVP, &desc, fmtRenderTarget, DXVA2_ProcAmp_Hue, &range);
					if (SUCCEEDED(hr))
					{
						pCaps->sCaps.uFilterCaps |= FILTER_CAPS_HUE;
						DXVA2_ValueRange2ValueRange(range, pCaps->FilterRanges[VIDEO_FILTER_HUE]);
					}
				}

				if (caps.ProcAmpControlCaps & DXVA2_ProcAmp_Saturation)
				{
					hr = pVPService->GetProcAmpRange(pCaps->guidVP, &desc, fmtRenderTarget, DXVA2_ProcAmp_Saturation, &range);
					if (SUCCEEDED(hr))
					{
						pCaps->sCaps.uFilterCaps |= FILTER_CAPS_SATURATION;
						DXVA2_ValueRange2ValueRange(range, pCaps->FilterRanges[VIDEO_FILTER_SATURATION]);
					}
				}

				if (DXVA2_NoiseFilterTech_Unsupported != caps.NoiseFilterTechnology)
				{
					pCaps->sCaps.uFilterCaps |= FILTER_CAPS_NOISE_REDUCTION;
					pCaps->FilterRanges[VIDEO_FILTER_NOISE_REDUCTION].fDefaultValue = 0;
					pCaps->FilterRanges[VIDEO_FILTER_NOISE_REDUCTION].fMaxValue = 1;
					pCaps->FilterRanges[VIDEO_FILTER_NOISE_REDUCTION].fMinValue = 0;
					pCaps->FilterRanges[VIDEO_FILTER_NOISE_REDUCTION].fStepSize = 0.1f;
				}

				if (DXVA2_DetailFilterTech_Unsupported != caps.DetailFilterTechnology)
				{
					pCaps->sCaps.uFilterCaps |= FILTER_CAPS_EDGE_ENHANCEMENT;
					pCaps->FilterRanges[VIDEO_FILTER_EDGE_ENHANCEMENT].fDefaultValue = 0;
					pCaps->FilterRanges[VIDEO_FILTER_EDGE_ENHANCEMENT].fMaxValue = 1;
					pCaps->FilterRanges[VIDEO_FILTER_EDGE_ENHANCEMENT].fMinValue = 0;
					pCaps->FilterRanges[VIDEO_FILTER_EDGE_ENHANCEMENT].fStepSize = 0.1f;
				}
				
				++*puCount;
				++pCaps;
			}

			if (*puCount > 0)
				bGetVPCaps = true;
			else
			{
				// query VPCaps by using D3DFMT_X8R8G8B8 format, if no available VP exist
				if (fmtRenderTarget != D3DFMT_X8R8G8B8)
					fmtRenderTarget = D3DFMT_X8R8G8B8;
				else
				{
					hr = E_FAIL;
					break;
				}
			}
		}
	}

	CoTaskMemFree(pDeviceGuids);
	
	return hr;
}