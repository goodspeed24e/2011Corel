#ifndef INTEL_FAST_COMPOSITING_DEVICE_INCLUDED
#define INTEL_FAST_COMPOSITING_DEVICE_INCLUDED

#include <dxva2api.h>

// Intel Auxiliary Device
// {A74CCAE2-F466-45ae-86F5-AB8BE8AF8483}
DEFINE_GUID(DXVA2_Intel_Auxiliary_Device, 0xa74ccae2, 0xf466, 0x45ae, 0x86, 0xf5, 0xab, 0x8b, 0xe8, 0xaf, 0x84, 0x83);

// DXVA2 Intel Auxiliary Device Function IDs
typedef enum tagAUXILIARY_DEVICE_FUNCTION_ID
{
	AUXDEV_GET_ACCEL_GUID_COUNT         = 1,
	AUXDEV_GET_ACCEL_GUIDS	            = 2,
	AUXDEV_GET_ACCEL_RT_FORMAT_COUNT	= 3,
	AUXDEV_GET_ACCEL_RT_FORMATS         = 4,
	AUXDEV_GET_ACCEL_FORMAT_COUNT       = 5,
	AUXDEV_GET_ACCEL_FORMATS            = 6,
	AUXDEV_QUERY_ACCEL_CAPS             = 7,
	AUXDEV_CREATE_ACCEL_SERVICE         = 8,
	AUXDEV_DESTROY_ACCEL_SERVICE        = 9
} AUXILIARY_DEVICE_FUNCTION_ID;


// Fast Video Compositing Device for HD DVD and Blu-Ray
// {7DD2D599-41DB-4456-9395-8B3D18CEDCAE}
DEFINE_GUID(DXVA2_FastCompositing, 0x7dd2d599, 0x41db, 0x4456, 0x93, 0x95, 0x8b, 0x3d, 0x18, 0xce, 0xdc, 0xae);

// Surface Registration Device
// {44DEE63B-77E6-4DD2-B2E1-443B130F2E2F}
DEFINE_GUID(DXVA2_Registration_Device, 0x44dee63b, 0x77e6, 0x4dd2, 0xb2, 0xe1, 0x44, 0x3b, 0x13, 0x0f, 0x2e, 0x2f);

// Private Query Device
DEFINE_GUID(DXVA2_Intel_PrivateQuery_Device, 0x56d269f0, 0xc210, 0x421c, 0x85, 0x38, 0xbe, 0x4e, 0xcb, 0x7c, 0x0a, 0xe2);

// Source Caps
typedef struct tagFASTCOMP_SRC_CAPS
{
    int     iDepth;                   // Depth of the source plane

    UINT    bSimpleDI           :1;   // Simple DI (BOB) is available
    UINT    bAdvancedDI         :1;   // Advanced DI is available
    UINT    bProcAmp            :1;   // ProcAmp (color control) is available
    UINT    bInverseTelecine    :1;   // Inverse telecine is available
    UINT    bDenoiseFilter      :1;   // Denoise filter is available
    UINT    bInverseTelecineTS  :1;   // Inverse telecine timestamping is available
    UINT    bVariance1          :1;
    UINT    bVariance2          :1;
    UINT    bSceneDetection     :1;
    UINT    Reserved1           :23;  // Reserved

    UINT    bSourceBlending     :1;   // Source blending is available  D = A * S + (1-A) * RT
    UINT    bPartialBlending    :1;   // Partial blending is available D = S     + (1-A) * RT
    UINT    uReserved2          :30;  // Reserved

    UINT    bConstantBlending   :1;   // Constant blending is available
    UINT                        :31;  // Reserved

    UINT    bLumaKeying         :1;   // Luma keying is available
    UINT    bExtendedGamut      :1;
    UINT    Reserved2           :30;  // Reserved

    int     iMaxWidth;                // Max source width
    int     iMaxHeight;               // Max source height
    int     iNumBackwardSamples;      // Backward Samples
    int     iNumForwardSamples;       // Forward Samples
} FASTCOMP_SRC_CAPS;

// Destination Caps
typedef struct tagFASTCOMP_DST_CAPS
{
	int     iMaxWidth;
	int     iMaxHeight;
} FASTCOMP_DST_CAPS;

// Fast Compositing Source/Destination Caps
typedef struct tagFASTCOMP_CAPS
{
	int                iMaxClearRects;
	int                iMaxSubstreams;
	FASTCOMP_SRC_CAPS  sPrimaryVideoCaps;
	FASTCOMP_SRC_CAPS  sSecondaryVideoCaps;
	FASTCOMP_SRC_CAPS  sSubstreamCaps;
	FASTCOMP_SRC_CAPS  sGraphicsCaps;
	FASTCOMP_DST_CAPS  sRenderTargetCaps;
} FASTCOMP_CAPS;

// Fast Compositing Sample Formats (User output)
typedef struct tagFASTCOMP_SAMPLE_FORMATS
{
	int          iPrimaryVideoFormatCount;
	int          iSecondaryVideoFormatCount;
	int          iSubstreamFormatCount;		// Presentation Graphics (BD)
	int          iGraphicsFormatCount;		// Interactive Graphics (BD)
	int          iRenderTargetFormatCount;
	int			 iBackgroundFormatCount;
	D3DFORMAT *  pPrimaryVideoFormats;
	D3DFORMAT *  pSecondaryVideoFormats;
	D3DFORMAT *  pSubstreamFormats;			// Presentation Graphics (BD)
	D3DFORMAT *  pGraphicsFormats;			// Interactive Graphics (BD)
	D3DFORMAT *  pRenderTargetFormats;
	D3DFORMAT *  pBackgroundFormats;
} FASTCOMP_SAMPLE_FORMATS;

// Fast Composition device creation parameters
typedef struct tagFASTCOMP_CREATEDEVICE
{
	DXVA2_VideoDesc     *pVideoDesc;
	D3DFORMAT            TargetFormat;
	UINT                 iSubstreams;
} FASTCOMP_CREATEDEVICE;

// Fast Composition ClearRect structure
typedef struct tagFASTCOMP_ClearRect
{
	RECT        Rect;
	UINT        Depth;
} FASTCOMP_ClearRect;

// Fast Composition video sample
typedef struct tagFASTCOMP_VideoSample
{
	REFERENCE_TIME       Start;
	REFERENCE_TIME       End;
	DXVA2_ExtendedFormat SampleFormat;
	IDirect3DSurface9 *  SrcSurface;
	RECT                 SrcRect;
	RECT                 DstRect;
	DXVA2_AYUVSample8    Palette[256];
	FLOAT                Alpha;
	int                  Depth;
	UINT                 bLumaKey			: 1;
	UINT				 bPartialBlending	: 1;
	UINT                 bExtendedGamut		: 1;
	UINT				 uReserved			: 29;	// Reserved.
	UINT                 iLumaLow;
	UINT                 iLumaHigh;
} FASTCOMP_VideoSample;

// Fast Compositing Blt request
typedef struct tagFASTCOMP_BLT_PARAMS
{
	IDirect3DSurface9    *pRenderTarget;
	UINT                  SampleCount;
	FASTCOMP_VideoSample *pSamples;
	REFERENCE_TIME        TargetFrame;
	RECT                  TargetRect;
	UINT                  bClearRectChanged	: 1;	// LSB
	UINT                  TargetMatrix : 3;   // Render Target Matrix
	UINT                  TargetExtGamut : 1;   // Render Target Extended Gamut
	UINT				  Reserved1 : 27;	// Reserved for extensions
	UINT                  ClearRectCount;	// clear rect is no longer supported, must be 0
	FASTCOMP_ClearRect   *pClearRect;		// clear rect is no longer supported, must be NULL
	DXVA2_AYUVSample16    BackgroundColor;
	void				 *pReserved2;		// Reserved for extensions
} FASTCOMP_BLT_PARAMS;

// Surface registration operation
typedef enum REGISTRATION_OP
{
	REG_IGNORE      = 0,    // Do nothing
	REG_REGISTER    = 1,    // Register surface
	REG_UNREGISTER  = 2     // Unregister surface
};

// Surface registration entry
typedef struct tagDXVA2_SAMPLE_REG
{
	REGISTRATION_OP     Operation;
	IDirect3DSurface9  *pD3DSurface;
} DXVA2_SAMPLE_REG, *PDXVA2_SAMPLE_REG;

// Surface registration request
typedef struct tagDXVA2_SURFACE_REGISTRATION
{
	HANDLE             RegHandle;
	DXVA2_SAMPLE_REG  *pRenderTargetRequest;
	UINT               nSamples;
	DXVA2_SAMPLE_REG  *pSamplesRequest;
} DXVA2_SURFACE_REGISTRATION, *PDXVA2_SURFACE_REGISTRATION;

// Fast Compositing DDI Function IDs
#define FASTCOMP_BLT 0x100

// Query Caps Types
typedef enum FASTCOMP_QUERYTYPE
{
	FASTCOMP_QUERY_CAPS                = 1,
	FASTCOMP_QUERY_FORMAT_COUNT        = 2,
	FASTCOMP_QUERY_FORMATS             = 3,
	FASTCOMP_QUERY_REGISTRATION_HANDLE = 4,
	FASTCOMP_QUERY_FRAME_RATE          = 5
};

// Fast Compositing Query Format Count (DDI)
typedef struct tagFASTCOMP_FORMAT_COUNT
{
	int          iPrimaryFormats;
	int          iSecondaryFormats;
	int          iSubstreamFormats;			// Presentation Graphics (BD)
	int          iGraphicsFormats;			// Interactive Graphics (BD)
	int          iRenderTargetFormats;
	int			 iBackgroundFormats;
} FASTCOMP_FORMAT_COUNT;

// Fast Compositing Query Formats (DDI)
typedef struct tagFASTCOMP_FORMATS
{
	int          iPrimaryFormatSize;
	int          iSecondaryFormatSize;
	int          iSubstreamFormatSize;		// Presentation Graphics (BD)
	int          iGraphicsFormatSize;		// Interactive Graphics (BD)
	int          iRenderTargetFormatSize;
	int          iBackgroundFormatSize;
	D3DFORMAT *  pPrimaryFormats;
	D3DFORMAT *  pSecondaryFormats;
	D3DFORMAT *  pSubstreamFormats;			// Presentation Graphics (BD)
	D3DFORMAT *  pGraphicsFormats;			// Interactive Graphics (BD)
	D3DFORMAT *  pRenderTargetFormats;
	D3DFORMAT *  pBackgroundFormats;
} FASTCOMP_FORMATS;

// Fast Compositing Query Frame Rate
typedef struct tagFASTCOMP_FRAME_RATE
{
	int          iMaxSrcWidth;       // [in]
	int          iMaxSrcHeight;      // [in]
	int          iMaxDstWidth;       // [in]
	int          iMaxDstHeight;      // [in]
	int          iMaxLayers;         // [in]
	int          iFrameRate;         // [out]
} FASTCOMP_FRAME_RATE;

// Fast Composition Query Caps union
typedef struct tagFASTCOMP_QUERYCAPS
{
	FASTCOMP_QUERYTYPE    Type;
	union
	{
		FASTCOMP_CAPS         sCaps;
		FASTCOMP_FORMAT_COUNT sFmtCount;
		FASTCOMP_FORMATS      sFormats;
		HANDLE                hRegistration;
		FASTCOMP_FRAME_RATE   sFrameRate;
	};
} FASTCOMP_QUERYCAPS;


///////////////////////////////////////////////////////////////////////////////
/// Private Query Device

// Private Query Device ID
enum PrivateQueryID
{
	QUERY_ID_DUAL_DXVA_DECODE_V1	= 1,
	QUERY_ID_DUAL_DXVA_DECODE_V2	= 2,
};

// Query structure for Private Query Device with ID: QUERY_ID_DUAL_DXVA_DECODE_V1
typedef struct _QUERY_DUAL_DXVA_DECODE_V1
{
	int iPrivateQueryID;	// [IN] Must be QUERY_ID_DUAL_DXVA_DECODE_V1 (1)
	int Reserved;			// [IN] Must be zero
	UINT Width1;			// [IN] Width of first video stream
	UINT Height1;			// [IN] Height of first video stream
	UINT Width2;			// [IN] Width of second video stream
	UINT Height2;			// [IN] Height of second video stream
	BOOL bSupported;		// [OUT] = TRUE/FALSE
} QUERY_DUAL_DXVA_DECODE_V1;

typedef struct _STREAM_PARAMETERS
{
    UINT    Width;
    UINT    Height;
    UINT    VideoFormat     :4;
    UINT    SourceFrameRate :8;
    BOOL    bInterlaced     :1;
    UINT    Reserved1       :19;    // Reserved(MBZ)
    UINT    Reserved2;              // Reserved(MBZ)
} STREAM_PARAMETERS;

// Query structure for Private Query Device with ID: QUERY_ID_DUAL_DXVA_DECODE_V2
typedef struct _QUERY_DUAL_DXVA_DECODE_V2
{
    INT iPrivateQueryID;	    // [IN] Must be QUERY_ID_DUAL_DXVA_DECODE_V2 (2)
    INT Reserved;			    // [IN] Must be zero
    STREAM_PARAMETERS Stream1;	// [IN] Stream 1 parameters
    STREAM_PARAMETERS Stream2;	// [IN] Stream 2 parameters
    INT Reserved2[4];			// [IN] Reserved(MBZ)
    BOOL bSupported;            // [OUT] = TRUE/FALSE
} QUERY_DUAL_DXVA_DECODE_V2;

///////////////////////////////////////////////////////////////////////////////
/// PreP

//				Min		Max		Default		Increment
// Brightness	-100.0	100.0	0.0			0.1
// Contrast		0.0		10.0	1.0			0.01
// Saturation	0.0		10.0	1.0			0.01
// Hue			-180.0	180.0	0.0			0.1
typedef struct tagPREPROC_ONLY_PARAM
{
	UINT VarianceType;
	VOID *pVariances;
	UINT DenoiseAutoAdjust	: 1;
	UINT DetailOptimalValue	: 1;
	UINT FMDEnable			: 1;
	UINT ProcAmpEnable		: 1;
	UINT DeinterlacingAlgorithm	: 4;
	UINT uReserved1			: 24;
	WORD DenoiseFactor;
	WORD DetailFactor;
	DXVA2_ProcAmpValues ProcAmpValues;
	WORD SpatialComplexity;
	WORD TemporalComplexity;
	UINT SceneChangeRate	: 8;
	UINT Cadence			: 12;
	UINT RepeatFrame		: 1;
	UINT uReserved2			: 11;
	VOID *pReserved;
} PREPROC_ONLY_PARAM;

typedef struct tagPREPPROC_QUERY_SCENE_DETECTION
{
	UINT FrameNumber;
	SHORT SceneChangeRate;
} PREPROC_QUERY_SCENE_DETECTION;

enum SCENE_DETECTION_RETURN
{
	SCENE_DETECTION_ERROR		= -2,
	SCENE_DETECTION_NOTREADY	= -1,
	SCENE_DETECTION_NOCHANGE	= 0,
	SCENE_DETECTION_DETECTED	= 255
};

#endif	// INTEL_FAST_COMPOSITING_DEVICE_INCLUDED
