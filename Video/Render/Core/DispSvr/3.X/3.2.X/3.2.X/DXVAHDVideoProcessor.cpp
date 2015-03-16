#include "stdafx.h"
#include "DriverExtensionHelper.h"
#include "DXVA2VideoProcessor.h"
#include "DXVAHDVideoProcessor.h"

namespace NVDxvaHD { 
	#include "Imports/ThirdParty/NVIDIA/NvAPI/nvdxvahdapi.h"
};
using namespace DispSvr;

#define SET_STREAM_STATE(uiStream, stateType, dataPointer)	\
	SetSS(m_pHDVP, uiStream, stateType, sizeof(stateType ## _DATA), dataPointer)
#define SET_STREAM_FILTER_STATE(uiStream, stateType, dataPointer)	\
	SetFS(m_pHDVP, uiStream, stateType, dataPointer)
#define SET_BLT_STATE(stateType, dataPointer) \
	SetBS(m_pHDVP, stateType, sizeof(stateType ## _DATA), dataPointer)
#define NV_SET_STREAM_STATE(uiStream, stateType, dataSize, dataPointer)	\
	SetSS(m_pHDVP, uiStream, stateType, sizeof(dataSize), dataPointer)

#ifdef DUMP_DXVA_HD

static inline HRESULT SetSS(IDXVAHD_VideoProcessor *pDev, UINT Stream, DXVAHD_STREAM_STATE State, UINT uSize, const void *pData)
{
	HRESULT hr = pDev->SetVideoProcessStreamState(Stream, State, uSize, pData);
	DbgMsg("SetVideoProcessStreamState(StreamNumber=%d, State=%d, Size=%d, Data=0x%p) = 0x%x", Stream, State, uSize, pData, hr);
	return hr;
}

static inline HRESULT SetFS(IDXVAHD_VideoProcessor *pDev, UINT Stream, DXVAHD_STREAM_STATE State, const void *pData)
{
	HRESULT hr = pDev->SetVideoProcessStreamState(Stream, State, sizeof(DXVAHD_STREAM_STATE_FILTER_DATA), pData);
	DbgMsg("SetVideoProcessStreamState(StreamNumber=%d, State=%d, Size=%d, Data=0x%p) = 0x%x", Stream, State, sizeof(DXVAHD_STREAM_STATE_FILTER_DATA), pData, hr);
	return hr;
}

static inline HRESULT SetBS(IDXVAHD_VideoProcessor *pDev, DXVAHD_BLT_STATE State, UINT uSize, const void *pData)
{
	HRESULT hr = pDev->SetVideoProcessBltState(State, uSize, pData);
	DbgMsg("SetVideoProcessBltState(State=%d, Size=%d, Data=0x%p) = 0x%x", State, uSize, pData, hr);
	return hr;
}
#else
static inline HRESULT SetSS(IDXVAHD_VideoProcessor *pDev, UINT Stream, DXVAHD_STREAM_STATE State, UINT uSize, const void *pData)
{
	if (pDev)
		return pDev->SetVideoProcessStreamState(Stream, State, uSize, pData);
	return S_FALSE;
}

static inline HRESULT SetFS(IDXVAHD_VideoProcessor *pDev, UINT Stream, DXVAHD_STREAM_STATE State, const void *pData)
{
	if (pDev)
		return pDev->SetVideoProcessStreamState(Stream, State, sizeof(DXVAHD_STREAM_STATE_FILTER_DATA), pData);
	return S_FALSE;
}

static inline HRESULT SetBS(IDXVAHD_VideoProcessor *pDev, DXVAHD_BLT_STATE State, UINT uSize, const void *pData)
{
	if (pDev)
		return pDev->SetVideoProcessBltState(State, uSize, pData);
	return S_FALSE;
}
#endif

struct CDXVAHDVP::DXVAHDVP_StreamStateCache : DXVAHDVP_StreamState
{
	DXVAHDVP_StreamStateCache() : pVP(0) { }
	~DXVAHDVP_StreamStateCache()
	{
		SAFE_DELETE(pVP);
	}

	D3DCOLOR pPaletteEntries[DXVAHDVP_MAX_PALETTE];
	UINT uInputFrameOrField;

	// used by D3D9 implementation
	D3DSURFACE_DESC descInputSurface;
	CDXVA2VideoProcessor *pVP;
	DXVA2VP_Caps sDXVA2VPCaps;
	float pfFilterRanges[6];
};

static void DXVAHD_ValueRange2ValueRange(DXVAHD_FILTER_RANGE_DATA &d, ValueRange &v)
{
	v.fDefaultValue = float(d.Default);
	v.fMaxValue = float(d.Maximum);
	v.fMinValue = float(d.Minimum);
	v.fStepSize = float(d.Multiplier);
}

// NVAPI DXVA HD via DXVA2
extern "C" HRESULT WINAPI
NV_DXVAHD_CreateDevice(IDirect3DDevice9Ex* pD3DDevice, const DXVAHD_CONTENT_DESC* pContentDesc, DXVAHD_DEVICE_USAGE Usage, void** ppDevice);

CDXVAHDVP::CDXVAHDVP()
: m_pHDDev(NULL), m_pHDVP(NULL), m_pStreamStateCache(NULL),
m_uMaxInputStream(0), m_pDXVAHDStreamData(NULL),
m_uChangedStream(0), m_uOutputFrame(0), m_pDriverExtension(NULL),
m_pDevice(NULL)
{
	ZeroMemory(&m_InitParam, sizeof(m_InitParam));
	ZeroMemory(&m_BltStateCache, sizeof(m_BltStateCache));
	ZeroMemory(&m_VPCaps, sizeof(m_VPCaps));
	ZeroMemory(m_VPFilterRange, sizeof(m_VPFilterRange));
	CResourceManager::GetInstance()->GetInterface(DISPSVR_RESOURCE_DRIVEREXTENSION, __uuidof(IDispSvrDriverExtension), (void **)&m_pDriverExtension);
}

CDXVAHDVP::~CDXVAHDVP()
{
	SAFE_RELEASE(m_pHDDev);
	SAFE_RELEASE(m_pHDVP);
	SAFE_RELEASE(m_pDriverExtension);
	SAFE_RELEASE(m_pDevice);
	SAFE_DELETE_ARRAY(m_pDXVAHDStreamData);
	SAFE_DELETE_ARRAY(m_pStreamStateCache);
}

HRESULT CDXVAHDVP::CreateVP(IDirect3DDevice9Ex *pDeviceEx, DXVAHDVP_INIT *pInit, CDXVAHDVP **ppVP)
{
	CHECK_POINTER(pDeviceEx);	// DXVA HD must use IDirect3DDevice9Ex
	CHECK_POINTER(ppVP);

	*ppVP = new CDXVAHDVP;
	if (*ppVP == NULL)
		return E_OUTOFMEMORY;

	HRESULT hr = (*ppVP)->CreateVP(pDeviceEx, pInit);
	if (FAILED(hr))
	{
		delete *ppVP;
		*ppVP = NULL;
	}

	return hr;
}

HRESULT CDXVAHDVP::CreateVP(IDirect3DDevice9Ex *pDeviceEx, DXVAHDVP_INIT *pInit)
{
	DXVAHD_VPDEVCAPS VPDevCaps;
	DXVAHD_CONTENT_DESC contentDesc;
	D3DCAPS9 caps9;
	HRESULT hr = E_NOTIMPL;

	ZeroMemory(&VPDevCaps, sizeof(VPDevCaps));
	ZeroMemory(&contentDesc, sizeof(contentDesc));
	ZeroMemory(&caps9, sizeof(caps9));
	ZeroMemory(&m_InitParam, sizeof(m_InitParam));

	contentDesc.InputFrameFormat = DXVAHD_FRAME_FORMAT_PROGRESSIVE;
	contentDesc.InputFrameRate.Numerator = 0;
	contentDesc.InputFrameRate.Denominator = 0;
	contentDesc.InputWidth = 1920;
	contentDesc.InputHeight = 1080;
	contentDesc.OutputFrameRate.Numerator = 0;
	contentDesc.OutputFrameRate.Denominator = 0;
	contentDesc.OutputWidth = 1920;
	contentDesc.OutputHeight = 1080;

	if (pInit)
		memcpy(&m_InitParam, pInit, sizeof(m_InitParam));

    if (m_InitParam.bNvDXVAHD)// create NV DXVAHD device for CNvDxva2HDMixerRender.
    {
        hr = NV_DXVAHD_CreateDevice(pDeviceEx, &contentDesc, DXVAHD_DEVICE_USAGE_PLAYBACK_NORMAL, (void **)&m_pHDDev);
    }
	// on windows 7, we can check if DXVAHD is supported via D3D9 device cap.
    else if (SUCCEEDED(pDeviceEx->GetDeviceCaps(&caps9)) && (D3DCAPS3_DXVAHD & caps9.Caps3) != 0)
    {
        if (DispSvr::CDynLibManager::GetInstance()->pfnDXVAHD_CreateDevice)
		{
	        hr = DispSvr::CDynLibManager::GetInstance()->pfnDXVAHD_CreateDevice(
		        pDeviceEx, &contentDesc, DXVAHD_DEVICE_USAGE_PLAYBACK_NORMAL, NULL, &m_pHDDev
		        );
            if (SUCCEEDED(hr))
            {
                DbgMsg("Creating DXVAHD OK!");
            }
		}
    }

    if (FAILED(hr))
	{
		// don't try to create d3d9 + dxva2 implementation
		if (m_InitParam.bUseNativeDXVAHDOnly)
			return hr;

		hr = CreateD3D9VP(pDeviceEx);
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

	D3DFORMAT *pFormats = new D3DFORMAT[max(VPDevCaps.InputFormatCount, VPDevCaps.OutputFormatCount)];
	hr = m_pHDDev->GetVideoProcessorInputFormats(VPDevCaps.InputFormatCount, pFormats);	ASSERT(SUCCEEDED(hr));
	m_setValidInputFormat = std::set<D3DFORMAT>(pFormats, pFormats + VPDevCaps.InputFormatCount);

	hr = m_pHDDev->GetVideoProcessorOutputFormats(VPDevCaps.OutputFormatCount, pFormats);	ASSERT(SUCCEEDED(hr));
	m_setValidOutputFormat = std::set<D3DFORMAT>(pFormats, pFormats + VPDevCaps.OutputFormatCount);
	delete [] pFormats;

//	enum VIDEO_FILTER
//	{
//		VIDEO_FILTER_BRIGHTNESS			= 0,
//		VIDEO_FILTER_CONTRAST			= 1,
//		VIDEO_FILTER_HUE				= 2,
//		VIDEO_FILTER_SATURATION			= 3,
//		VIDEO_FILTER_NOISE_REDUCTION	= 4,
//		VIDEO_FILTER_EDGE_ENHANCEMENT	= 5
//	};
//
//	enum _DXVAHD_FILTER_CAPS
//    {	DXVAHD_FILTER_CAPS_BRIGHTNESS	= 0x1,
//	DXVAHD_FILTER_CAPS_CONTRAST	= 0x2,
//	DXVAHD_FILTER_CAPS_HUE	= 0x4,
//	DXVAHD_FILTER_CAPS_SATURATION	= 0x8,
//	DXVAHD_FILTER_CAPS_NOISE_REDUCTION	= 0x10,
//	DXVAHD_FILTER_CAPS_EDGE_ENHANCEMENT	= 0x20,
//	DXVAHD_FILTER_CAPS_ANAMORPHIC_SCALING	= 0x40
//    } 	DXVAHD_FILTER_CAPS;
	DXVAHD_FILTER_RANGE_DATA range;
	for (UINT i = 0; i < DXVAHDVP_MAX_FILTER; i++)
	{
		UINT uFilterBit = (1 << i);
		if (VPDevCaps.FilterCaps & uFilterBit)
		{
			ZeroMemory(&range, sizeof(range));
			hr = m_pHDDev->GetVideoProcessorFilterRange(static_cast<DXVAHD_FILTER> (i), &range);
			if (SUCCEEDED(hr))
			{
				m_VPCaps.uFilterCaps |= uFilterBit;
				DXVAHD_ValueRange2ValueRange(range, m_VPFilterRange[i]);
			}
		}
	}

	m_VPCaps.eType = PROCESSOR_TYPE_HARDWARE;
	m_VPCaps.uNumBackwardSamples = VPCaps.PastFrames > DXVAHDVP_MAX_PAST_FRAMES ? DXVAHDVP_MAX_PAST_FRAMES : VPCaps.PastFrames;
	m_VPCaps.uNumForwardSamples = VPCaps.FutureFrames;

	if (VPCaps.ProcessorCaps & DXVAHD_PROCESSOR_CAPS_DEINTERLACE_MOTION_COMPENSATION)
		m_VPCaps.uProcessorCaps |= PROCESSOR_CAPS_DEINTERLACE_MOTION_COMPENSATION;
	if (VPCaps.ProcessorCaps & DXVAHD_PROCESSOR_CAPS_DEINTERLACE_ADAPTIVE)
		m_VPCaps.uProcessorCaps |= PROCESSOR_CAPS_DEINTERLACE_ADAPTIVE;
	if (VPCaps.ProcessorCaps & DXVAHD_PROCESSOR_CAPS_DEINTERLACE_BLEND)
		m_VPCaps.uProcessorCaps |= PROCESSOR_CAPS_DEINTERLACE_BLEND;
	if (VPCaps.ProcessorCaps & DXVAHD_PROCESSOR_CAPS_DEINTERLACE_BOB)
		m_VPCaps.uProcessorCaps |= PROCESSOR_CAPS_DEINTERLACE_BOB;

	m_pStreamStateCache = new DXVAHDVP_StreamStateCache[m_uMaxInputStream];
	ZeroMemory(m_pStreamStateCache, sizeof(DXVAHDVP_StreamStateCache) * m_uMaxInputStream);
	m_pDXVAHDStreamData = new DXVAHD_STREAM_DATA[m_uMaxInputStream];
	ZeroMemory(m_pDXVAHDStreamData, sizeof(DXVAHD_STREAM_DATA) * m_uMaxInputStream);
	return hr;
}

HRESULT CDXVAHDVP::CreateD3D9VP(IDirect3DDevice9Ex *pDeviceEx)
{
	const D3DFORMAT InputFormat[] = { D3DFMT_A8R8G8B8, D3DFMT_YUY2, D3DFORMAT(MAKEFOURCC('N', 'V', '1', '2')) };
	const D3DFORMAT OutputFormat[] = { D3DFMT_A8R8G8B8, D3DFMT_X8R8G8B8 };

	m_pDevice = pDeviceEx;
	m_pDevice->AddRef();

	m_uMaxInputStream = 5;

	m_setValidInputFormat = std::set<D3DFORMAT>(InputFormat, InputFormat + sizeof(InputFormat) / sizeof(InputFormat[0]));
	m_setValidOutputFormat = std::set<D3DFORMAT>(OutputFormat, OutputFormat + sizeof(OutputFormat) / sizeof(OutputFormat[0]));

	m_pStreamStateCache = new DXVAHDVP_StreamStateCache[m_uMaxInputStream];
	ZeroMemory(m_pStreamStateCache, sizeof(DXVAHDVP_StreamStateCache) * m_uMaxInputStream);
	m_pDXVAHDStreamData = new DXVAHD_STREAM_DATA[m_uMaxInputStream];
	ZeroMemory(m_pDXVAHDStreamData, sizeof(DXVAHD_STREAM_DATA) * m_uMaxInputStream);

	UINT uCount = 0;
	DXVA2VP_Caps *pCaps = 0;
	CDXVA2VideoProcessor::GetDXVA2VPCaps(m_pDevice, &uCount, &pCaps);
	if (uCount > 0)
	{
		const DXVA2VP_Caps &desiredCap = pCaps[0];
		float pfDefaultFilterRanges[6];

		for (UINT i = 0; i < 6; i++)
			pfDefaultFilterRanges[i] = desiredCap.FilterRanges[i].fDefaultValue;

		// For DXVA2 case (ex AMD VGA), set default filter ranges
		for (UINT i = 0; i < DXVAHDVP_MAX_FILTER; i++)
		{	
			m_VPFilterRange[i] = desiredCap.FilterRanges[i];
		}

		for (UINT i = 0; i < m_uMaxInputStream; i++)
		{
            memcpy(&m_pStreamStateCache[i].sDXVA2VPCaps.guidVP, &desiredCap.guidVP,sizeof(m_pStreamStateCache[i].sDXVA2VPCaps.guidVP));
            m_pStreamStateCache[i].sDXVA2VPCaps.RenderTargetFormat = desiredCap.RenderTargetFormat;

			memcpy(m_pStreamStateCache[i].pfFilterRanges, pfDefaultFilterRanges, sizeof(m_pStreamStateCache[i].pfFilterRanges));
		}

		memcpy(&m_VPCaps, &desiredCap.sCaps, sizeof(m_VPCaps));
		SAFE_DELETE_ARRAY(pCaps);
	}
	return S_OK;
}

HRESULT CDXVAHDVP::GetVPCaps(VideoProcessorCaps &sCaps)
{
	memcpy(&sCaps, &m_VPCaps, sizeof(VideoProcessorCaps));
	return S_OK;
}

HRESULT CDXVAHDVP::GetFilterValueRange(VIDEO_FILTER eFilter, ValueRange &range)
{
	if (m_VPCaps.uFilterCaps & (1 << eFilter))
	{
		memcpy(&range, &m_VPFilterRange[eFilter], sizeof(ValueRange));
		return S_OK;
	}
	return E_NOTIMPL;
}

HRESULT CDXVAHDVP::SetStreamState(UINT uiStreamNum, const DXVAHDVP_StreamState &s, bool ForceUpdate)
{
	HRESULT hr = S_OK;

	if (uiStreamNum >= m_uMaxInputStream
		|| s.uPaletteCount > DXVAHDVP_MAX_PALETTE
		|| (s.bEnable && s.pInputSurface == NULL)
		)
		return E_INVALIDARG;

	// Sync stream states
	DXVAHDVP_StreamStateCache &c = m_pStreamStateCache[uiStreamNum];

	if (s.bEnable || ForceUpdate)
	{
		if (s.pInputSurface)
		{
			D3DSURFACE_DESC desc;
			hr = s.pInputSurface->GetDesc(&desc);
			if (FAILED(hr))
				return hr;

			if (c.descInputSurface.Format != desc.Format)
			{
				DXVAHD_STREAM_STATE_D3DFORMAT_DATA data = { desc.Format };
				hr |= SET_STREAM_STATE(uiStreamNum, DXVAHD_STREAM_STATE_D3DFORMAT, &data);
			}
			c.descInputSurface = desc;
		}

		if (c.InputColorSpace.value != s.InputColorSpace.value)
		{
			DXVAHD_STREAM_STATE_INPUT_COLOR_SPACE_DATA data;

			ZeroMemory(&data, sizeof(data));
			data.Type = s.InputColorSpace.bGraphics;
			data.RGB_Range = s.InputColorSpace.bLimitedRange;
			data.YCbCr_Matrix = s.InputColorSpace.bBT709;
			data.YCbCr_xvYCC = s.InputColorSpace.bXvYCC;

			// Only either RGB or YCbCr flags corresponding to the color space of the input format will be referred. 
			hr |= SET_STREAM_STATE(uiStreamNum, DXVAHD_STREAM_STATE_INPUT_COLOR_SPACE, &data);
			c.InputColorSpace.value = s.InputColorSpace.value;

			if (s.InputColorSpace.bGraphics)
				SAFE_DELETE(c.pVP);
		}

		if (!EqualRect(&c.SourceRect, &s.SourceRect))
		{
			DXVAHD_STREAM_STATE_SOURCE_RECT_DATA data = { TRUE, s.SourceRect };
			hr |= SET_STREAM_STATE(uiStreamNum, DXVAHD_STREAM_STATE_SOURCE_RECT, &data);
			c.SourceRect = s.SourceRect;
		}

		ASSERT(!IsRectEmpty(&s.DestinationRect));	// Empty DestinationRect may cause crash in some DXVAHD implementation.
		if (!EqualRect(&c.DestinationRect, &s.DestinationRect))
		{
			DXVAHD_STREAM_STATE_DESTINATION_RECT_DATA data = { TRUE, s.DestinationRect };
			hr |= SET_STREAM_STATE(uiStreamNum, DXVAHD_STREAM_STATE_DESTINATION_RECT, &data);
			c.DestinationRect = s.DestinationRect;
		}

		if (!__FloatEqual(c.fAlpha, s.fAlpha))
		{
			DXVAHD_STREAM_STATE_ALPHA_DATA data = { c.fAlpha < 1.0 ? TRUE : FALSE, s.fAlpha};
			hr |= SET_STREAM_STATE(uiStreamNum, DXVAHD_STREAM_STATE_ALPHA, &data);
			c.fAlpha = s.fAlpha;
		}

		if (memcmp(&c.LumaKey, &s.LumaKey, sizeof(s.LumaKey)) != 0)
		{
			DXVAHD_STREAM_STATE_LUMA_KEY_DATA data = {
				s.LumaKey.bEnable,
				FLOAT(s.LumaKey.uLower) / 255,
				FLOAT(s.LumaKey.uUpper) / 255
			};

			if (m_InitParam.bNvDXVAHD)
				hr |= NV_SET_STREAM_STATE(uiStreamNum, static_cast<DXVAHD_STREAM_STATE>(NVDxvaHD::DXVAHD_STREAM_STATE_LUMA_KEY), NVDxvaHD::DXVAHD_STREAM_STATE_LUMA_KEY_DATA, &data);
			else
				hr |= SET_STREAM_STATE(uiStreamNum, DXVAHD_STREAM_STATE_LUMA_KEY, &data);
			c.LumaKey = s.LumaKey;
		}

		if (c.uPaletteCount != s.uPaletteCount
			|| (s.uPaletteCount > 0 && memcmp(c.Palette, s.Palette, sizeof(s.Palette[0]) * s.uPaletteCount) != 0))
		{
			DXVAHD_STREAM_STATE_PALETTE_DATA data = { s.uPaletteCount, s.uPaletteCount > 0 ? c.pPaletteEntries : NULL };
			for (UINT i = 0; i < s.uPaletteCount; i++)
			{
				data.pEntries[i] = AYUVSample8ToD3DCOLOR(s.Palette[i]);
			}
			hr |= SET_STREAM_STATE(uiStreamNum, DXVAHD_STREAM_STATE_PALETTE, &data);
			c.uPaletteCount = s.uPaletteCount;
			memcpy(c.Palette, s.Palette, sizeof(s.Palette[0]) * s.uPaletteCount);
		}

		if (c.FrameFormat != s.FrameFormat)
		{
			DXVAHD_STREAM_STATE_FRAME_FORMAT_DATA data = { static_cast<DXVAHD_FRAME_FORMAT> (s.FrameFormat) };
			hr |= SET_STREAM_STATE(uiStreamNum, DXVAHD_STREAM_STATE_FRAME_FORMAT, &data);
			c.FrameFormat = s.FrameFormat;
		}

		for (UINT i = 0; i < DXVAHDVP_MAX_FILTER; i++)
		{
			if (c.Filter[i].bEnable != s.Filter[i].bEnable || c.Filter[i].fLevel != s.Filter[i].fLevel)
			{
				DXVAHD_STREAM_STATE_FILTER_DATA data = { c.Filter[i].bEnable, (int)s.Filter[i].fLevel };
				if (m_InitParam.bNvDXVAHD)
					hr |= SET_STREAM_FILTER_STATE(uiStreamNum, static_cast<DXVAHD_STREAM_STATE> (i+NVDxvaHD::DXVAHD_STREAM_STATE_FILTER_BRIGHTNESS), &data);
				else
					hr |= SET_STREAM_FILTER_STATE(uiStreamNum, static_cast<DXVAHD_STREAM_STATE> (i+DXVAHD_STREAM_STATE_FILTER_BRIGHTNESS), &data);
				c.Filter[i] = s.Filter[i];
			}
		}
	}

	// prepare DXVAHD stream data
	DXVAHD_STREAM_DATA &d = m_pDXVAHDStreamData[uiStreamNum];

    BOOL bStereoEnable = (s.eStereoMixingMode != MIXER_STEREO_MODE_DISABLED);
	c.eStereoMixingMode = s.eStereoMixingMode;
	if (m_pDriverExtension && s.pInputSurface && bStereoEnable)
	{
		m_pDriverExtension->SetStereoInfo(s.pInputSurface, s.pDependentSurface, s.lStereoDisplayOffset, bStereoEnable, s.eStereoMixingMode);
		c.pDependentSurface = s.pDependentSurface;
		c.lStereoDisplayOffset = s.lStereoDisplayOffset;
	}
	else
	{
		c.pDependentSurface = NULL;
		c.lStereoDisplayOffset = 0;
	}

	bool bEnableStateChanged = (d.Enable != s.bEnable);
	d.Enable = s.bEnable;
	d.pInputSurface = s.pInputSurface;
	d.OutputIndex = s.uOutputIndex;
	d.PastFrames = s.uPastFrameCount;
	d.ppPastSurfaces = NULL;
	for (UINT i = 0; i < d.PastFrames; i++)
	{
		c.ppPastSurfaces[i] = s.ppPastSurfaces[i];
		d.ppPastSurfaces = c.ppPastSurfaces;
	}

	if (bEnableStateChanged)
	{
		m_uChangedStream++;
		c.uInputFrameOrField = 0;
		c.dwLastUpdateTS = s.dwLastUpdateTS;
	}
	else
	{
		if (c.dwLastUpdateTS != s.dwLastUpdateTS)
		{
			if (s.dwLastUpdateTS < c.dwLastUpdateTS)
				c.uInputFrameOrField = 0;
			else
				c.uInputFrameOrField++;

			c.dwLastUpdateTS = s.dwLastUpdateTS;
			m_uChangedStream++;
		}

	}

	// uInputFrameOrField is calculated internally instead.
	d.InputFrameOrField = c.uInputFrameOrField;
	return hr;
}

HRESULT CDXVAHDVP::VBltHD(IDirect3DSurface9 *pDestSurface, const DXVAHDVP_BltState &bltState)
{
	HRESULT hr;

	hr = SyncBltState(bltState);

	if (m_uChangedStream)
		m_uOutputFrame++;

#ifdef _DEBUG
	for (UINT i = 0; i < m_uMaxInputStream; i++)
	{
		if (m_pDXVAHDStreamData[i].Enable)
			ASSERT(m_pDXVAHDStreamData[i].pInputSurface);
	}
#endif

	if (m_pHDVP)
		hr = m_pHDVP->VideoProcessBltHD(pDestSurface, m_uOutputFrame, m_uMaxInputStream, m_pDXVAHDStreamData);
	else
		hr = D3D9VBltHD(pDestSurface, m_uOutputFrame, m_uMaxInputStream, m_pDXVAHDStreamData);
	if (SUCCEEDED(hr))
		m_uChangedStream = 0;

	// unlink any surface on a per blit basis.
	if (m_pDriverExtension)
	{
		for (UINT i = 0; i < m_uMaxInputStream; i++)
		{
			if (m_pStreamStateCache[i].eStereoMixingMode != MIXER_STEREO_MODE_DISABLED && m_pDXVAHDStreamData[i].pInputSurface)
			{
				ASSERT(m_pDXVAHDStreamData[i].Enable == TRUE);
				m_pDriverExtension->SetStereoInfo(m_pDXVAHDStreamData[i].pInputSurface, NULL, 0, FALSE, m_pStreamStateCache[i].eStereoMixingMode);
			}
		}
	}
	return hr;
}

HRESULT CDXVAHDVP::SyncBltState(const DXVAHDVP_BltState &bltState)
{
	HRESULT hr = 0;

	if (!EqualRect(&m_BltStateCache.TargetRect, &bltState.TargetRect))
	{
		// Set Blt State (Target RECT)
		DXVAHD_BLT_STATE_TARGET_RECT_DATA data = { TRUE, bltState.TargetRect };

		hr |= SET_BLT_STATE(DXVAHD_BLT_STATE_TARGET_RECT, &data);

		m_BltStateCache.TargetRect = bltState.TargetRect;
	}

	if (m_BltStateCache.OutputColorSpace.value != bltState.OutputColorSpace.value)
	{
		DXVAHD_BLT_STATE_OUTPUT_COLOR_SPACE_DATA data;

		ZeroMemory(&data, sizeof(data));
		data.Usage = bltState.OutputColorSpace.bVideoProcessing;
		data.RGB_Range = bltState.OutputColorSpace.bLimitedRange;
		data.YCbCr_Matrix = bltState.OutputColorSpace.bBT709;
		data.YCbCr_xvYCC = bltState.OutputColorSpace.bXvYCC;

		hr |= SET_BLT_STATE(DXVAHD_BLT_STATE_OUTPUT_COLOR_SPACE, &data);

		m_BltStateCache.OutputColorSpace = bltState.OutputColorSpace;
	}
	return hr;
}

bool CDXVAHDVP::IsValidInputFormat(PLANE_FORMAT fmt) const
{
	// ARGB support is required by DXVAHD implementation.
	if (PLANE_FORMAT_ARGB == fmt)
		return true;
	return m_setValidInputFormat.find(static_cast<D3DFORMAT> (fmt)) != m_setValidInputFormat.end();
}

HRESULT CDXVAHDVP::D3D9VBltHD(IDirect3DSurface9 *pOutputSurface, UINT OutputFrame, UINT StreamCount, const DXVAHD_STREAM_DATA *pStreams)
{
	HRESULT hr = S_FALSE;

	CComPtr<IDirect3DSurface9> pRT;
	D3DSURFACE_DESC desc;

	hr = pOutputSurface->GetDesc(&desc);
	if (FAILED(hr))
		return hr;

	D3DXMATRIX mxOrigWorld;
	D3DVIEWPORT9 sOrigViewport, sViewport = { 0, 0, desc.Width, desc.Height, 0.f, 1.f };
	const D3DXMATRIX mxWorld(
		2.f / desc.Width, 0, 0, 0,
		0, -2.f / desc.Height, 0, 0,
		0, 0, 1, 0,
		-1, 1, 0, 1
	);

	hr = m_pDevice->GetRenderTarget(0, &pRT);
	hr = m_pDevice->GetTransform(D3DTS_WORLD, &mxOrigWorld);
	hr = m_pDevice->GetViewport(&sOrigViewport);

	hr = m_pDevice->SetRenderTarget(0, pOutputSurface);
	if (FAILED(hr))
		return hr;

	hr = m_pDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	hr = m_pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	hr = m_pDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	hr = m_pDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
	hr = m_pDevice->SetSamplerState(0, D3DSAMP_BORDERCOLOR, 0x00000000);

	hr = m_pDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
	hr = m_pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	hr = m_pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

	hr = m_pDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	hr = m_pDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
	hr = m_pDevice->SetRenderState(D3DRS_ALPHAREF, 0x0);
	hr = m_pDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
	hr = m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	hr = m_pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);

	hr = m_pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	hr = m_pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	
	hr = m_pDevice->SetViewport(&sViewport);
	hr = m_pDevice->SetTransform(D3DTS_WORLD, &mxWorld);

	hr = m_pDevice->ColorFill(pOutputSurface, &m_BltStateCache.TargetRect, 0);
	for (UINT i = 0; i < StreamCount; i++)
	{
		DXVAHDVP_StreamStateCache &c = m_pStreamStateCache[i];
		const DXVAHD_STREAM_DATA &d = pStreams[i];

		if (d.Enable)
		{
			if (c.descInputSurface.Format != D3DFMT_A8R8G8B8)
				hr = DXVA2Blt(pOutputSurface, c, d, m_BltStateCache);
			else
				hr = D3D9Blt(pOutputSurface, c, d, m_BltStateCache);
		}
	}

	hr = m_pDevice->SetRenderTarget(0, pRT);
	hr = m_pDevice->SetViewport(&sOrigViewport);
	hr = m_pDevice->SetTransform(D3DTS_WORLD, &mxOrigWorld);

	return S_OK;
}

HRESULT CDXVAHDVP::DXVA2Blt(IDirect3DSurface9 *pOutputSurface, DXVAHDVP_StreamStateCache &c, const DXVAHD_STREAM_DATA &d, const DXVAHDVP_BltState &blt)
{
	HRESULT hr = E_FAIL;

	if (!c.pVP)
		c.pVP = new CDXVA2VideoProcessor(c.sDXVA2VPCaps.guidVP, c.sDXVA2VPCaps.RenderTargetFormat);

	if (c.pVP)
	{
		const DWORD dwFrameRate1000 = 24000;
		const LONGLONG llPTS = 10000000000 / dwFrameRate1000;
		VideoProperty vprop = {0};

		if (DXVAHDVP_FRAME_FORMAT_INTERLACED_TOP_FIELD_FIRST == c.FrameFormat)
			vprop.Format = VIDEO_FORMAT_FIELD_INTERLEAVED_EVEN_FIRST;
		else if (DXVAHDVP_FRAME_FORMAT_INTERLACED_BOTTOM_FIELD_FIRST == c.FrameFormat)
			vprop.Format = VIDEO_FORMAT_FIELD_INTERLEAVED_ODD_FIRST;
		else
			vprop.Format = VIDEO_FORMAT_PROGRESSIVE;

		vprop.uWidth = c.descInputSurface.Width;
		vprop.uHeight = c.descInputSurface.Height;
		vprop.dwFieldSelect = (c.uOutputIndex == 0) ? FIELD_SELECT_FIRST : FIELD_SELECT_SECOND;
		vprop.dwFrameRate1000 = dwFrameRate1000;
		vprop.rtStart = d.InputFrameOrField * llPTS;
		vprop.rtEnd = (d.InputFrameOrField + 1) * llPTS;

		DXVA2VP_RefSample *pSamples = new DXVA2VP_RefSample[1 + c.sDXVA2VPCaps.sCaps.uNumBackwardSamples];
		UINT uSamples = 0;
/*
// There are still some problems with AMD DXVA2VP when there are backward reference samples.
		for (UINT i = 0; i < d.PastFrames; i++)
		{
			pSamples[uSamples].pSurface = d.ppPastSurfaces[i];
			pSamples[uSamples].VideoProperty = vprop;
			pSamples[uSamples].VideoProperty.dwFieldSelect = FIELD_SELECT_FIRST;
			pSamples[uSamples].VideoProperty.rtStart -= (d.PastFrames - i) * llPTS;
			pSamples[uSamples].VideoProperty.rtEnd -= (d.PastFrames - i) * llPTS;
			uSamples++;
		}
*/
		pSamples[uSamples].pSurface = d.pInputSurface;
		pSamples[uSamples].VideoProperty = vprop;
		uSamples++;
	
		// Without VideoEffect, update filter values for DXVA2 blt
		for (int i = 0; i < DXVAHDVP_MAX_FILTER; i++)
		{
			if (c.Filter[i].bEnable)
			{
				c.pfFilterRanges[i] = c.Filter[i].fLevel;
			}
		}
		
		hr = c.pVP->VideoProcessBlt(d.pInputSurface, c.SourceRect, pOutputSurface, c.DestinationRect, vprop, pSamples, uSamples, c.pfFilterRanges);

		SAFE_DELETE_ARRAY(pSamples);
	}

	if (FAILED(hr))
		hr = m_pDevice->StretchRect(d.pInputSurface, &c.SourceRect, pOutputSurface, &c.DestinationRect, D3DTEXF_LINEAR);
	return hr;
}

HRESULT CDXVAHDVP::D3D9Blt(IDirect3DSurface9 *pOutputSurface, DXVAHDVP_StreamStateCache &c, const DXVAHD_STREAM_DATA &d, const DXVAHDVP_BltState &blt)
{
	CComPtr<IDirect3DTexture9> pTexture;
	HRESULT hr = d.pInputSurface->GetContainer(IID_IDirect3DTexture9, (void **)&pTexture);

	if (FAILED(hr))
		return hr;

	ASSERT(c.descInputSurface.Width > 0 && c.descInputSurface.Height > 0);
	const NORMALIZEDRECT DstRect = {
		float(c.DestinationRect.left),
		float(c.DestinationRect.top),
		float(c.DestinationRect.right),
		float(c.DestinationRect.bottom)
	};
	const NORMALIZEDRECT SrcRect = {
		float(c.SourceRect.left) / c.descInputSurface.Width,
		float(c.SourceRect.top) / c.descInputSurface.Height,
		float(c.SourceRect.right) / c.descInputSurface.Width,
		float(c.SourceRect.bottom) / c.descInputSurface.Height
	};

	const float z = 0.5f;
	struct {
		float x, y, z;
		float u, v;
	} vb[4] = {
		{ DstRect.left, DstRect.top, z, SrcRect.left, SrcRect.top },
		{ DstRect.left, DstRect.bottom, z, SrcRect.left, SrcRect.bottom },
		{ DstRect.right, DstRect.bottom, z, SrcRect.right, SrcRect.bottom },
		{ DstRect.right, DstRect.top, z, SrcRect.right, SrcRect.top }
	};

	hr = m_pDevice->SetFVF(D3DFVF_XYZ | D3DFVF_TEX1);
	hr = m_pDevice->SetTexture(0, pTexture);

	if (c.InputColorSpace.bGraphics)
	{
		hr = m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		hr = m_pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
		hr = m_pDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, &vb[0], sizeof(vb[0]));
		hr = m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		hr = m_pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	}
	else
	{
		hr = m_pDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, &vb[0], sizeof(vb[0]));
	}
	return hr;
}