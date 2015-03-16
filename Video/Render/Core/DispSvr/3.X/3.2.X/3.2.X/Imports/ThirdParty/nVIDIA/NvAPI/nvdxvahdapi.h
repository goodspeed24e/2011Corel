#ifndef __dxvahdapi_h__
#define __dxvahdapi_h__

#ifdef __cplusplus
extern "C"{
#endif 

typedef 
enum _DXVAHD_FRAME_FORMAT
    {	DXVAHD_FRAME_FORMAT_PROGRESSIVE	= 0,
	DXVAHD_FRAME_FORMAT_INTERLACED_TOP_FIELD_FIRST	= 1,
	DXVAHD_FRAME_FORMAT_INTERLACED_BOTTOM_FIELD_FIRST	= 2
    } 	DXVAHD_FRAME_FORMAT;

typedef 
enum _DXVAHD_DEVICE_USAGE
    {	DXVAHD_DEVICE_USAGE_PLAYBACK_NORMAL	= 0,
	DXVAHD_DEVICE_USAGE_OPTIMAL_SPEED	= 1,
	DXVAHD_DEVICE_USAGE_OPTIMAL_QUALITY	= 2
    } 	DXVAHD_DEVICE_USAGE;

typedef 
enum _DXVAHD_SURFACE_TYPE
    {	DXVAHD_SURFACE_TYPE_VIDEO_INPUT	= 0,
	DXVAHD_SURFACE_TYPE_VIDEO_INPUT_PRIVATE	= 1,
	DXVAHD_SURFACE_TYPE_VIDEO_OUTPUT	= 2
    } 	DXVAHD_SURFACE_TYPE;

typedef 
enum _DXVAHD_DEVICE_TYPE
    {	DXVAHD_DEVICE_TYPE_HARDWARE	= 0,
	DXVAHD_DEVICE_TYPE_SOFTWARE	= 1,
	DXVAHD_DEVICE_TYPE_REFERENCE	= 2,
	DXVAHD_DEVICE_TYPE_OTHER	= 3
    } 	DXVAHD_DEVICE_TYPE;

typedef 
enum _DXVAHD_DEVICE_CAPS
    {	DXVAHD_DEVICE_CAPS_LINEAR_SPACE	= 0x1,
	DXVAHD_DEVICE_CAPS_xvYCC	= 0x2
    } 	DXVAHD_DEVICE_CAPS;

typedef 
enum _DXVAHD_FEATURE_CAPS
    {	DXVAHD_FEATURE_CAPS_CONSTRICTION	= 0x1,
	DXVAHD_FEATURE_CAPS_CLEAR_RECT	= 0x2,
	DXVAHD_FEATURE_CAPS_LUMA_KEY	= 0x4
    } 	DXVAHD_FEATURE_CAPS;

typedef 
enum _DXVAHD_FILTER_CAPS
    {	DXVAHD_FILTER_CAPS_BRIGHTNESS	= 0x1,
	DXVAHD_FILTER_CAPS_CONTRAST	= 0x2,
	DXVAHD_FILTER_CAPS_HUE	= 0x4,
	DXVAHD_FILTER_CAPS_SATURATION	= 0x8,
	DXVAHD_FILTER_CAPS_NOISE_REDUCTION	= 0x10,
	DXVAHD_FILTER_CAPS_EDGE_ENHANCEMENT	= 0x20,
	DXVAHD_FILTER_CAPS_ANAMORPHIC_SCALING	= 0x40
    } 	DXVAHD_FILTER_CAPS;

typedef 
enum _DXVAHD_INPUT_FORMAT_CAPS
    {	DXVAHD_INPUT_FORMAT_CAPS_RGB_INTERLACED	= 0x1,
	DXVAHD_INPUT_FORMAT_CAPS_RGB_LIMITED_RANGE	= 0x2,
	DXVAHD_INPUT_FORMAT_CAPS_RGB_PROCAMP	= 0x4,
	DXVAHD_INPUT_FORMAT_CAPS_RGB_LUMA_KEY	= 0x8
    } 	DXVAHD_INPUT_FORMAT_CAPS;

typedef 
enum _DXVAHD_PROCESSOR_CAPS
    {	DXVAHD_PROCESSOR_CAPS_DEINTERLACE_BLEND	= 0x1,
	DXVAHD_PROCESSOR_CAPS_DEINTERLACE_BOB	= 0x2,
	DXVAHD_PROCESSOR_CAPS_DEINTERLACE_ADAPTIVE	= 0x4,
	DXVAHD_PROCESSOR_CAPS_DEINTERLACE_MOTION_COMPENSATION	= 0x8,
	DXVAHD_PROCESSOR_CAPS_INVERSE_TELECINE	= 0x10,
	DXVAHD_PROCESSOR_CAPS_FRAME_RATE_CONVERSION	= 0x20
    } 	DXVAHD_PROCESSOR_CAPS;

typedef 
enum _DXVAHD_ITELECINE_CAPS
    {	DXVAHD_ITELECINE_CAPS_32	= 0x1,
	DXVAHD_ITELECINE_CAPS_22	= 0x2,
	DXVAHD_ITELECINE_CAPS_2224	= 0x4,
	DXVAHD_ITELECINE_CAPS_2332	= 0x8,
	DXVAHD_ITELECINE_CAPS_32322	= 0x10,
	DXVAHD_ITELECINE_CAPS_55	= 0x20,
	DXVAHD_ITELECINE_CAPS_64	= 0x40,
	DXVAHD_ITELECINE_CAPS_87	= 0x80,
	DXVAHD_ITELECINE_CAPS_222222222223	= 0x100,
	DXVAHD_ITELECINE_CAPS_OTHER	= 0x80000000
    } 	DXVAHD_ITELECINE_CAPS;

typedef 
enum _DXVAHD_FILTER
    {	DXVAHD_FILTER_BRIGHTNESS	= 0,
	DXVAHD_FILTER_CONTRAST	= 1,
	DXVAHD_FILTER_HUE	= 2,
	DXVAHD_FILTER_SATURATION	= 3,
	DXVAHD_FILTER_NOISE_REDUCTION	= 4,
	DXVAHD_FILTER_EDGE_ENHANCEMENT	= 5,
	DXVAHD_FILTER_ANAMORPHIC_SCALING	= 6
    } 	DXVAHD_FILTER;

typedef 
enum _DXVAHD_BLT_STATE
    {	DXVAHD_BLT_STATE_TARGET_RECT	= 0,
	DXVAHD_BLT_STATE_BACKGROUND_COLOR	= 1,
	DXVAHD_BLT_STATE_OUTPUT_COLOR_SPACE	= 2,
	DXVAHD_BLT_STATE_ALPHA_FILL	= 3,
	DXVAHD_BLT_STATE_CONSTRICTION	= 4,
	DXVAHD_BLT_STATE_CLEAR_RECT	= 5,
	DXVAHD_BLT_STATE_PRIVATE	= 1000
    } 	DXVAHD_BLT_STATE;

typedef 
enum _DXVAHD_ALPHA_FILL_MODE
    {	DXVAHD_ALPHA_FILL_MODE_BACKGROUND	= 0,
	DXVAHD_ALPHA_FILL_MODE_DESTINATION	= 1,
	DXVAHD_ALPHA_FILL_MODE_SOURCE_STREAM	= 2
    } 	DXVAHD_ALPHA_FILL_MODE;

typedef 
enum _DXVAHD_STREAM_STATE
    {	DXVAHD_STREAM_STATE_D3DFORMAT	= 0,
	DXVAHD_STREAM_STATE_FRAME_FORMAT	= 1,
	DXVAHD_STREAM_STATE_INPUT_COLOR_SPACE	= 2,
	DXVAHD_STREAM_STATE_OUTPUT_RATE	= 3,
	DXVAHD_STREAM_STATE_SOURCE_RECT	= 4,
	DXVAHD_STREAM_STATE_DESTINATION_RECT	= 5,
	DXVAHD_STREAM_STATE_ALPHA	= 6,
	DXVAHD_STREAM_STATE_PALETTE	= 7,
	DXVAHD_STREAM_STATE_CLEAR_RECT	= 8,
	DXVAHD_STREAM_STATE_LUMA_KEY	= 9,
	DXVAHD_STREAM_STATE_FILTER_BRIGHTNESS	= 10,
	DXVAHD_STREAM_STATE_FILTER_CONTRAST	= 11,
	DXVAHD_STREAM_STATE_FILTER_HUE	= 12,
	DXVAHD_STREAM_STATE_FILTER_SATURATION	= 13,
	DXVAHD_STREAM_STATE_FILTER_NOISE_REDUCTION	= 14,
	DXVAHD_STREAM_STATE_FILTER_EDGE_ENHANCEMENT	= 15,
	DXVAHD_STREAM_STATE_FILTER_ANAMORPHIC_SCALING	= 16,
	DXVAHD_STREAM_STATE_PRIVATE	= 1000
    } 	DXVAHD_STREAM_STATE;

typedef 
enum _DXVAHD_OUTPUT_RATE
    {	DXVAHD_OUTPUT_RATE_NORMAL	= 0,
	DXVAHD_OUTPUT_RATE_HALF	= 1,
	DXVAHD_OUTPUT_RATE_CUSTOM	= 2
    } 	DXVAHD_OUTPUT_RATE;

typedef struct _DXVAHD_RATIONAL
    {
    UINT Numerator;
    UINT Denominator;
    } 	DXVAHD_RATIONAL;

typedef struct _DXVAHD_COLOR_RGBA
    {
    FLOAT R;
    FLOAT G;
    FLOAT B;
    FLOAT A;
    } 	DXVAHD_COLOR_RGBA;

typedef struct _DXVAHD_COLOR_YCbCrA
    {
    FLOAT Y;
    FLOAT Cb;
    FLOAT Cr;
    FLOAT A;
    } 	DXVAHD_COLOR_YCbCrA;

typedef union _DXVAHD_COLOR
    {
    DXVAHD_COLOR_RGBA RGB;
    DXVAHD_COLOR_YCbCrA YCbCr;
    } 	DXVAHD_COLOR;

typedef struct _DXVAHD_CONTENT_DESC
    {
    DXVAHD_FRAME_FORMAT InputFrameFormat;
    DXVAHD_RATIONAL InputFrameRate;
    UINT InputWidth;
    UINT InputHeight;
    DXVAHD_RATIONAL OutputFrameRate;
    UINT OutputWidth;
    UINT OutputHeight;
    } 	DXVAHD_CONTENT_DESC;

typedef struct _DXVAHD_VPDEVCAPS
    {
    DXVAHD_DEVICE_TYPE DeviceType;
    UINT DeviceCaps;
    UINT FeatureCaps;
    UINT FilterCaps;
    UINT InputFormatCaps;
    D3DPOOL InputPool;
    UINT OutputFormatCount;
    UINT InputFormatCount;
    UINT VideoProcessorCount;
    UINT MaxInputStreams;
    UINT MaxStreamStates;
    } 	DXVAHD_VPDEVCAPS;

typedef struct _DXVAHD_VPCAPS
    {
    GUID VPGuid;
    UINT PastFrames;
    UINT FutureFrames;
    UINT ProcessorCaps;
    UINT ITelecineCaps;
    UINT CustomRateCount;
    } 	DXVAHD_VPCAPS;

typedef struct _DXVAHD_CUSTOM_RATE_DATA
    {
    DXVAHD_RATIONAL CustomRate;
    UINT OutputFrames;
    BOOL InputInterlaced;
    UINT InputFramesOrFields;
    } 	DXVAHD_CUSTOM_RATE_DATA;

typedef struct _DXVAHD_FILTER_RANGE_DATA
    {
    INT Minimum;
    INT Maximum;
    INT Default;
    FLOAT Multiplier;
    } 	DXVAHD_FILTER_RANGE_DATA;

typedef struct _DXVAHD_BLT_STATE_TARGET_RECT_DATA
    {
    BOOL Enable;
    RECT TargetRect;
    } 	DXVAHD_BLT_STATE_TARGET_RECT_DATA;

typedef struct _DXVAHD_BLT_STATE_BACKGROUND_COLOR_DATA
    {
    BOOL YCbCr;
    DXVAHD_COLOR BackgroundColor;
    } 	DXVAHD_BLT_STATE_BACKGROUND_COLOR_DATA;

typedef struct _DXVAHD_BLT_STATE_OUTPUT_COLOR_SPACE_DATA
    {
    UINT Usage	: 1;
    UINT RGB_Range	: 1;
    UINT YCbCr_Matrix	: 1;
    UINT YCbCr_xvYCC	: 1;
    } 	DXVAHD_BLT_STATE_OUTPUT_COLOR_SPACE_DATA;

typedef struct _DXVAHD_BLT_STATE_ALPHA_FILL_DATA
    {
    DXVAHD_ALPHA_FILL_MODE Mode;
    UINT StreamNumber;
    } 	DXVAHD_BLT_STATE_ALPHA_FILL_DATA;

typedef struct _DXVAHD_BLT_STATE_CONSTRICTION_DATA
    {
    BOOL Enable;
    SIZE Size;
    } 	DXVAHD_BLT_STATE_CONSTRICTION_DATA;

typedef struct _DXVAHD_BLT_STATE_CLEAR_RECT_DATA
    {
    BOOL Enable;
    RECT ClearRect[ 32 ];
    } 	DXVAHD_BLT_STATE_CLEAR_RECT_DATA;

typedef struct _DXVAHD_BLT_STATE_PRIVATE_DATA
    {
    GUID Guid;
    UINT DataSize;
    void *pData;
    } 	DXVAHD_BLT_STATE_PRIVATE_DATA;

typedef struct _DXVAHD_STREAM_STATE_D3DFORMAT_DATA
    {
    D3DFORMAT Format;
    } 	DXVAHD_STREAM_STATE_D3DFORMAT_DATA;

typedef struct _DXVAHD_STREAM_STATE_FRAME_FORMAT_DATA
    {
    DXVAHD_FRAME_FORMAT FrameFormat;
    } 	DXVAHD_STREAM_STATE_FRAME_FORMAT_DATA;

typedef struct _DXVAHD_STREAM_STATE_INPUT_COLOR_SPACE_DATA
    {
    UINT Type	: 1;
    UINT RGB_Range	: 1;
    UINT YCbCr_Matrix	: 1;
    UINT YCbCr_xvYCC	: 1;
    } 	DXVAHD_STREAM_STATE_INPUT_COLOR_SPACE_DATA;

typedef struct _DXVAHD_STREAM_STATE_OUTPUT_RATE_DATA
    {
    BOOL RepeatFrame;
    DXVAHD_OUTPUT_RATE OutputRate;
    DXVAHD_RATIONAL CustomRate;
    } 	DXVAHD_STREAM_STATE_OUTPUT_RATE_DATA;

typedef struct _DXVAHD_STREAM_STATE_SOURCE_RECT_DATA
    {
    BOOL Enable;
    RECT SourceRect;
    } 	DXVAHD_STREAM_STATE_SOURCE_RECT_DATA;

typedef struct _DXVAHD_STREAM_STATE_DESTINATION_RECT_DATA
    {
    BOOL Enable;
    RECT DestinationRect;
    } 	DXVAHD_STREAM_STATE_DESTINATION_RECT_DATA;

typedef struct _DXVAHD_STREAM_STATE_ALPHA_DATA
    {
    BOOL Enable;
    FLOAT Alpha;
    } 	DXVAHD_STREAM_STATE_ALPHA_DATA;

typedef struct _DXVAHD_STREAM_STATE_PALETTE_DATA
    {
    UINT Count;
    D3DCOLOR *pEntries;
    } 	DXVAHD_STREAM_STATE_PALETTE_DATA;

typedef struct _DXVAHD_STREAM_STATE_CLEAR_RECT_DATA
    {
    DWORD ClearRectMask;
    } 	DXVAHD_STREAM_STATE_CLEAR_RECT_DATA;

typedef struct _DXVAHD_STREAM_STATE_LUMA_KEY_DATA
    {
    BOOL Enable;
    FLOAT Lower;
    FLOAT Upper;
    } 	DXVAHD_STREAM_STATE_LUMA_KEY_DATA;

typedef struct _DXVAHD_STREAM_STATE_FILTER_DATA
    {
    BOOL Enable;
    INT Level;
    } 	DXVAHD_STREAM_STATE_FILTER_DATA;

typedef struct _DXVAHD_STREAM_STATE_PRIVATE_DATA
    {
    GUID Guid;
    UINT DataSize;
    void *pData;
    } 	DXVAHD_STREAM_STATE_PRIVATE_DATA;

typedef struct _DXVAHD_STREAM_DATA
    {
    BOOL Enable;
    UINT OutputIndex;
    UINT InputFrameOrField;
    UINT PastFrames;
    UINT FutureFrames;
    IDirect3DSurface9 **ppPastSurfaces;
    IDirect3DSurface9 *pInputSurface;
    IDirect3DSurface9 **ppFutureSurfaces;
    } 	DXVAHD_STREAM_DATA;


// type definitions used in IDXVAHD_VideoProcessor
typedef struct _DXVAHD_CAPS_DATA
{
    UINT ProcessorCaps;
    UINT ITelecineCaps;
} DXVAHD_CAPS_DATA;

typedef struct _DXVAHD_STREAM_STATE_DATA
{	// struct size: about 80 bytes	
	DXVAHD_STREAM_STATE_FRAME_FORMAT_DATA FrameFormat;
	DXVAHD_STREAM_STATE_INPUT_COLOR_SPACE_DATA InputColorSpace;
    DXVAHD_STREAM_STATE_OUTPUT_RATE_DATA OutputRate;
    DXVAHD_STREAM_STATE_SOURCE_RECT_DATA SourceRect;
    DXVAHD_STREAM_STATE_DESTINATION_RECT_DATA DestinationRect;
	DXVAHD_STREAM_STATE_ALPHA_DATA Alpha;
	DXVAHD_STREAM_STATE_PALETTE_DATA Palette;
	DXVAHD_STREAM_STATE_CLEAR_RECT_DATA ClearRect;
	DXVAHD_STREAM_STATE_LUMA_KEY_DATA LumaKey;
	DXVAHD_STREAM_STATE_FILTER_DATA Brightness;
	DXVAHD_STREAM_STATE_FILTER_DATA Contrast;
	DXVAHD_STREAM_STATE_FILTER_DATA Hue;
	DXVAHD_STREAM_STATE_FILTER_DATA Saturation;
	DXVAHD_STREAM_STATE_FILTER_DATA NoiseReduction;
	DXVAHD_STREAM_STATE_FILTER_DATA EdgeEnhancement;
	DXVAHD_STREAM_STATE_FILTER_DATA AnamorphicScaling;
	DXVAHD_STREAM_STATE_PRIVATE_DATA Private;
	D3DCOLOR pPaletteEntries[256];
} DXVAHD_STREAM_STATE_DATA;

typedef struct _DXVAHD_BLT_STATE_DATA
{	// struct size: about 32 bytes
	DXVAHD_BLT_STATE_TARGET_RECT_DATA TargetRect;
	DXVAHD_BLT_STATE_BACKGROUND_COLOR_DATA BackgroundColor;
	DXVAHD_BLT_STATE_OUTPUT_COLOR_SPACE_DATA OutputColorSpace;
	DXVAHD_BLT_STATE_ALPHA_FILL_DATA AlphaFill;
	DXVAHD_BLT_STATE_CONSTRICTION_DATA Constriction;
	DXVAHD_BLT_STATE_CLEAR_RECT_DATA ClearRect;
	DXVAHD_BLT_STATE_PRIVATE_DATA Private;
} DXVAHD_BLT_STATE_DATA;

typedef struct _DXVAHD_STREAM_DATA_TO_COPY
{
    BOOL                Enable;
	UINT				OutputIndex;
    UINT				InputFrameOrField;
    UINT                PastFrames;
    UINT                FutureFrames;
    IDirect3DSurface9*  ppPastSurfaces[4];
    IDirect3DSurface9*  pInputSurface;
    IDirect3DSurface9*  ppFutureSurfaces[4];
} DXVAHD_STREAM_DATA_TO_COPY;

#define DXVAHD_MAX_NUM_STREAMS	12
#if defined(__cplusplus) && !defined(CINTERFACE)
class IDXVAHD_Device;

class IDXVAHD_VideoProcessor
{
public:
    // Members for DXVAHD support over DXVA2
	
	IDXVAHD_Device*     pDevice;

    // blt_state
	DXVAHD_BLT_STATE_DATA BltState;

	// stream_state
	DXVAHD_STREAM_STATE_DATA         pStreamState[DXVAHD_MAX_NUM_STREAMS];
	DXVAHD_STREAM_STATE_D3DFORMAT_DATA  D3DFormat[DXVAHD_MAX_NUM_STREAMS];

	// to borrow VideoProcessBlt
    IDirectXVideoProcessor*             m_pVideodProcessDevice;    
    IDirect3DSurface9*                  m_pSysMemControl;

	HRESULT OnCreate();
    HRESULT CreateControlSurface(IDirectXVideoProcessorService *pVPService);

    // DXVAHD functions
	HRESULT SetVideoProcessBltState(
		DXVAHD_BLT_STATE State,
		UINT             DataSize,
		const VOID*            pData
	);

	HRESULT GetVideoProcessBltState(
		DXVAHD_BLT_STATE State,
		UINT             DataSize,
		VOID*            pData
	);

	HRESULT SetVideoProcessStreamState(
		UINT              StreamNumber,
		DXVAHD_STREAM_STATE State,
		UINT              DataSize,
		const VOID*       pData
	);

	HRESULT GetVideoProcessStreamState(
		UINT              StreamNumber,
		DXVAHD_STREAM_STATE State,
		UINT              DataSize,
		VOID*             pData
	);	

	HRESULT VideoProcessBltHD(
		IDirect3DSurface9* pOutputSurface,
		UINT               OutputFrame,
		UINT               StreamCount,
		DXVAHD_STREAM_DATA*  pData
	);

    // Function to support DXVAHD over DXVA2
	HRESULT StealSurface(IDirect3DSurface9* lpDecode, DXVAHD_STREAM_DATA *pStreamData, UINT NumStreams);
};

#define DXVAHD_MAX_INPUT_FORMATS    32
#define DXVAHD_MAX_OUTPUT_FORMATS    32
#define DXVAHD_NUM_FILTERS	7

class IDXVAHD_Device
{
public:
    // Member for DXVAHD support over DXVA2
	IDirectXVideoProcessorService*      m_pAccelServices;    

    DXVAHD_VPDEVCAPS DevCaps;
    D3DFORMAT pInputFormats[DXVAHD_MAX_INPUT_FORMATS];
    D3DFORMAT pOutputFormats[DXVAHD_MAX_OUTPUT_FORMATS];
    DXVAHD_VPCAPS VPCaps;
    DXVAHD_FILTER_RANGE_DATA pFilterRanges[7];

    //DXVAHD functions
    HRESULT CreateVideoSurface(
		UINT                Width,
		UINT                Height,
		D3DFORMAT           Format,
        D3DPOOL             Pool,
        DWORD               Usage,         // reserved
        DXVAHD_SURFACE_TYPE Type,
        UINT                NumSurfaces,
		IDirect3DSurface9** ppSurface,
        HANDLE              *pSharedHandle // reserved
	);

    HRESULT GetVideoProcessorDeviceCaps(
		DXVAHD_VPDEVCAPS* pCaps
	);

    HRESULT GetVideoProcessorOutputFormats(
		UINT Count,
		D3DFORMAT* pFormat
	);

    HRESULT GetVideoProcessorInputFormats(
		UINT Count,
		D3DFORMAT* pFormat
	);

    HRESULT GetVideoProcessorCaps(
		UINT Count,
		DXVAHD_VPCAPS* pCaps
	);

    HRESULT IDXVAHD_Device::GetVideoProcessorCustomRates(
	    const GUID*                 pVPGuid,
	    UINT                        Count,
	    DXVAHD_CUSTOM_RATE_DATA*    pFrameRate
    );

    HRESULT GetVideoProcessorFilterRange(
		DXVAHD_FILTER Filter,
		DXVAHD_FILTER_RANGE_DATA* pData
	);

    HRESULT CreateVideoProcessor(
		GUID                  VPGuid,
		IDXVAHD_VideoProcessor** ppVideoProcessor
	);
};
	
#else 	/* C style interface */
#endif 	/* C style interface */

HRESULT DXVAHD_CreateDevice(
    IDirect3DDevice9Ex*  pD3DDevice,
    DXVAHD_CONTENT_DESC* pContentDesc,
    DXVAHD_DEVICE_USAGE   Flags,
    HMODULE              VideoProcessorPlugin,
	IDXVAHD_Device**     ppDevice
);

#ifdef __cplusplus
}
#endif

#endif
