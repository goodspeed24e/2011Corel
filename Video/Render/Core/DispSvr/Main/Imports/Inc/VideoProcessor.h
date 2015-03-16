#ifndef _VIDEO_PROCESSOR_SERVICE_H_
#define _VIDEO_PROCESSOR_SERVICE_H_

struct __declspec(uuid("1528A4D4-1DA1-4228-B7BE-E13A6ED18970")) IVideoProcessor;
struct __declspec(uuid("F8754C9E-0965-49db-ADE6-16D5AFFC34CA")) IVideoProcessorEffect;
struct __declspec(uuid("2AEB3B70-CBBA-4372-B319-D55ADD4F7CE1")) IVideoProcessorTrimensionAll2HD;
struct __declspec(uuid("61C75C2D-E08D-44e2-98B8-80CC6D88D9D2")) IVideoProcessorEventListener;

#ifndef MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3) ((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) | ((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24 ))
#endif	// MAKEFOURCC

namespace VideoProcessor
{
	enum EVENT_VIDEOPROCESSOR
	{
		EVENT_VIDEOPROCESSORBLT_RENDERTARGET_READY,
		EVENT_VIDEOPROCESSORBLT_ERROR
	};

	enum VIDEO_FORMAT
	{
		VIDEO_FORMAT_UNKNOW = 0,
		VIDEO_FORMAT_8BPP,
		VIDEO_FORMAT_RGB555,
		VIDEO_FORMAT_RGB565,
		VIDEO_FORMAT_RGB888,
		VIDEO_FORMAT_RGB32,
		VIDEO_FORMAT_PRIVATE_IYUV, //< Y U V, I420 or YUV420
		VIDEO_FORMAT_PRIVATE_YV12, //< Y V U
		VIDEO_FORMAT_PRIVATE_NV12, //< Y UV
		VIDEO_FORMAT_PRIVATE_SUBPICTURE,
		VIDEO_FORMAT_ARGB = MAKEFOURCC('A', 'R', 'G', 'B'),	//< 32 bit, A8R8G8B8
		VIDEO_FORMAT_AYUV = MAKEFOURCC('A', 'Y', 'U', 'V'),	//< 32 bit, A8Y8U8V8
		VIDEO_FORMAT_AV12 = MAKEFOURCC('A', 'V', '1', '2'),	//< 20 bit, NV12 4:2:0 + A8, or ANV12
		VIDEO_FORMAT_NV12 = MAKEFOURCC('N', 'V', '1', '2'),	//< 12 bit, 4:2:0
		VIDEO_FORMAT_NV24 = MAKEFOURCC('N', 'V', '2', '4'),	//< 12 bit, 4:2:0
		VIDEO_FORMAT_IMC3 = MAKEFOURCC('I', 'M', 'C', '3'),	//< 12 bit, 4:2:0
		VIDEO_FORMAT_YV12 = MAKEFOURCC('Y', 'V', '1', '2'),	//< 12 bit, 4:2:0
		VIDEO_FORMAT_YUY2 = MAKEFOURCC('Y', 'U', 'Y', '2'),	//< 16 bit, YUV 4:2:2
		VIDEO_FORMAT_AIP8 = MAKEFOURCC('A', 'I', 'P', '8'),	//< 8 bit, I8 indexed
		VIDEO_FORMAT_PPA8 = MAKEFOURCC('P', 'P', 'A', '8')	//< 16 bit, A8I8 indexed
	};

	enum FRAME_STRUCTURE
	{
		FRAME_STRUCTURE_PROGRESSIVE				= 0,
		FRAME_STRUCTURE_INTERLACED_EVEN_FIRST	= 0x1,
		FRAME_STRUCTURE_INTERLACED_ODD_FIRST	= 0x2,
		FRAME_STRUCTURE_REPEAT_FIRST_FIELD		= 0x4
	};

	// 0: Auto, 1: Force interlaced, 2: Force progressive.
	enum PROCESS_MODE
	{
		PROCESS_MODE_AUTO,
		PROCESS_MODE_FORCE_DEINTERLACING,
		PROCESS_MODE_FORCE_PROGRESSIVE
	};

	enum NOISEFILTER_TECH
	{	
		NOISEFILTER_TECH_UNSUPPORTED	= 0,
		NOISEFILTER_TECH_Unknown		= 0x01,
		NOISEFILTER_TECH_Median			= 0x02,
		NOISEFILTER_TECH_Temporal		= 0x04,
		NOISEFILTER_TECH_BlockNoise		= 0x08,
		NOISEFILTER_TECH_MosquitoNoise	= 0x10,
		NOISEFILTER_TECH_Mask			= 0x1f
	};

	enum DEINTERLACE_TECH
	{	
		DEINTERLACE_TECH_UNKNOW					= 0,
		DEINTERLACE_TECH_BOBLineReplicate		= 0x001,
		DEINTERLACE_TECH_BOBVerticalStretch		= 0x002,
		DEINTERLACE_TECH_BOBVerticalStretch4Tap	= 0x004,
		DEINTERLACE_TECH_MedianFiltering		= 0x008,
		DEINTERLACE_TECH_EdgeFiltering			= 0x010,
		DEINTERLACE_TECH_FieldAdaptive			= 0x020,
		DEINTERLACE_TECH_PixelAdaptive			= 0x040,
		DEINTERLACE_TECH_MotionVectorSteered	= 0x080,
		DEINTERLACE_TECH_InverseTelecine		= 0x100,
		DEINTERLACE_TECH_MASK					= 0x1ff
	};

	enum DETAILFILTER_TECH
	{	
		DETAILFILTER_TECH_UNSUPPORTED	= 0,
		DETAILFILTER_TECH_Unknown		= 0x1,
		DETAILFILTER_TECH_Edge			= 0x2,
		DETAILFILTER_TECH_Sharpening	= 0x4,
		DETAILFILTER_TECH_Mask			= 0x7
	};

	enum VIDEO_FILTER
	{	
		NoiseFilterLumaLevel		= 0,
		NoiseFilterLumaThreshold,
		NoiseFilterLumaRadius,
		NoiseFilterChromaLevel,
		NoiseFilterChromaThreshold,
		NoiseFilterChromaRadius,
		DetailFilterLumaLevel,
		DetailFilterLumaThreshold,
		DetailFilterLumaRadius,
		DetailFilterChromaLevel,
		DetailFilterChromaThreshold,
		DetailFilterChromaRadius
	};

	enum PROCAMP_CONTROL
	{
		PROCAMP_CONTROL_BRIGHTNESS	= 0x001,
		PROCAMP_CONTROL_CONTRAST	= 0x002,
		PROCAMP_CONTROL_SATURATION	= 0x004,
		PROCAMP_CONTROL_HUE			= 0x008,
		PROCAMP_CONTROL_GAMMA		= 0x010,
		PROCAMP_CONTROL_CGAMMA		= 0x020,
		PROCAMP_CONTROL_SHARPNESS	= 0x040,
		PROCAMP_CONTROL_UOFFSET		= 0x080,
		PROCAMP_CONTROL_VOFFSET		= 0x100
	};

	enum VIDEOEFFECT_TECH
	{
		VIDEOEFFECT_TECH_UNSUPPORTED		= 0,
		VIDEOEFFECT_TECH_VIDEOENHANCEMENT	= 0x1, // video effect dmo and video smart stretch dmo.
		VIDEOEFFECT_TECH_TRIMENSIONAll2HD	= 0x2  // trimension All2HD.
	};

	typedef LONGLONG VIDEO_TIME;

	struct VideoProcessorCaps
	{
		VIDEO_FORMAT	dwSrcFormat;
		VIDEO_FORMAT	dwRenderTargetFormat;
		DWORD			dwDeinterlaceTechnology;
		DWORD			dwProcAmpControlCaps;
		DWORD			dwNoiseFilterTechnology;
		DWORD			dwDetailFilterTechnology;
		DWORD			dwVideoEffectOperations;
		DWORD			uReserved[6];
	};

	struct VideoProcessorModes
	{
		DWORD	dwDeinterlaceTechnology;
		DWORD	dwNoiseFilterTechnology;	
		DWORD	dwDetailFilterTechnology;	
		DWORD	dwVideoEffectOperations;	// video effect.

		DWORD	dwReserved[11];
	};

	struct VideoBuffer
	{
		VIDEO_FORMAT VideoFormat;
		union
		{
			struct
			{
				UCHAR	*pVideoPlane[3];	// pointers to Y,U,V planes
				INT		iStride[3];			// input buffer stride
				INT		iWidth[3];			// decoded width
				INT		iHeight[3];			// decoded height
			} Video;

			struct
			{
				DWORD	dwFlags;			// status flags
				UCHAR	*pPixel;			// pixel array returned by GetSpic (short for YUY2, 16bpp, char for RGB 8bpp, 24bpp)
				UCHAR	*pAlpha;			// alpha array returned by GetSpic
				ULONG	*pPalette;			// yuv palette used (16 entries) -- useful for 8bpp.
			} Subtitle;
		};
	};

	struct VideoSample
	{
		DWORD				dwFrameStucture;
		VIDEO_TIME			vtStartTime;		// frame pts
		VIDEO_TIME			vtEndTime;			// frame pte
		VideoBuffer			Surface;
		RECT				rcDisplayRect;

		UINT				uReserved[16];
	};

	struct ProcAmpValueRange
	{
		INT iMinValue;			// Minimum supported value.
		INT iMaxValue;			// Maximum supported value.
		INT iDefaultValue;		// Default value.
		INT iStepSize;			// Minimum increment between values.
	};

	struct FilterValueRange
	{
		FLOAT fMinValue;		// Minimum supported value.
		FLOAT fMaxValue;		// Maximum supported value.
		FLOAT fDefaultValue;	// Default value.
		FLOAT fStepSize;		// Minimum increment between values.
	};

	struct ProcAmpValues
	{
		INT		iHue;
		INT		iSaturation;
		INT		iBrightness;
		INT		iContrast;
		INT		iSharpness;
		INT		iGamma;
		INT		iCGamma;
		INT		iUOffset;
		INT		iVOffset;
		UINT	uReserved[16];
	};

	struct FilterValues
	{
		INT iLevel;
		INT iThreshold;
		INT iRadius;
	};

	struct VideoProcessBltParams 
	{
		PROCESS_MODE	ProcessMode;
		VIDEO_TIME		vtTargetFrame;
		ProcAmpValues	sProcAmpValues;
		FilterValues	sNoiseFilterLuma;
		FilterValues	sNoiseFilterChroma;
		FilterValues	sDetailFilterLuma;
		FilterValues	sDetailFilterChroma;
		UINT			uReserved[16];
	};
}

// A callback interface for those who want to be notified.
interface IVideoProcessorEventListener : public IUnknown
{
	STDMETHOD(Notify)(DWORD dwEvent, DWORD dwParam1, DWORD dwParam2) = 0;
};

interface IVideoProcessor : public IUnknown
{
	STDMETHOD(QueryVideoProcessorCaps) ( 
		/* [out] */ 
		VideoProcessor::VideoProcessorCaps **ppCaps) = 0;

	STDMETHOD(GetFilterPropertyRange) ( 
		/* [in] */ 
		VideoProcessor::VIDEO_FILTER VideoFilter,
		/* [out] */ 
		VideoProcessor::FilterValueRange* pFilterRange) = 0;

	STDMETHOD(GetProcAmpRange) ( 
		/* [in] */ 
		VideoProcessor::PROCAMP_CONTROL ProcAmpControl,
		/* [out] */ 
		VideoProcessor::ProcAmpValueRange* pProcAmpRange) = 0;

	STDMETHOD(SetVideoProcessorMode) ( 
		/* [in] */ 
		const VideoProcessor::VideoProcessorModes *pVPMode) = 0;

	STDMETHOD(GetVideoProcessorMode) ( 
		/* [out] */ 
		VideoProcessor::VideoProcessorModes *pVPMode) = 0;

	STDMETHOD(GetNumReferentSamples) ( 
		/* [out] */ 
		DWORD	*pNumBackwardRefSamples,
		/* [out] */ 
		DWORD	*pNumForwardRefSamples) = 0;

	STDMETHOD(VideoProcessBlt) ( 
		/* [in] */ 
		VideoProcessor::VideoBuffer *pRenderTarget,
		/* [in] */ 
		const VideoProcessor::VideoProcessBltParams *pBltParams,
		/* [size_is][in] */ 
		const VideoProcessor::VideoSample *pVideoSamples,
		/* [in] */ 
		UINT uNumSamples) = 0;
};


#ifdef VIDEOPROCESSOR_EXPORTS
#define VIDEOPROCESSOR_API __declspec(dllexport)
#else
#define VIDEOPROCESSOR_API __declspec(dllimport)
#endif

extern "C" VIDEOPROCESSOR_API int CreateVideoProcessor(
	/*[in]*/	VideoProcessor::VIDEO_FORMAT InputVideoFormat,
	/*[in]*/	VideoProcessor::VIDEO_FORMAT RenderTargetFormat,
	/*[in]*/	IVideoProcessorEventListener *pEvenListener,
	/*[out]*/	void** ppVideoProcessor
	);


#endif