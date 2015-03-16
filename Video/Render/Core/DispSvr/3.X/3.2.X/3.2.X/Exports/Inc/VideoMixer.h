#ifndef _DISPSVR_VIDEO_MIXER_IMPL_H_
#define _DISPSVR_VIDEO_MIXER_IMPL_H_

MIDL_INTERFACE("EF5A7D54-56D5-435e-88D5-4AD80744AEE8") IDispSvrVideoMixer;
MIDL_INTERFACE("53355341-027B-4279-98CE-921999F7263A") IDispSvrVideoMixerPlane;
MIDL_INTERFACE("D8368FAF-80A2-438d-921C-742BDAEFF4B2") IDispSvrVideoMixerVideoPlane;
MIDL_INTERFACE("A20F8554-43A1-4436-93B4-01738475005C") IDispSvrVideoMixerEventListener;
MIDL_INTERFACE("C69C4CFA-2592-4e65-9C25-76F8062EEE47") IDispSvrVideoProcessor;
MIDL_INTERFACE("F12010D7-7DE3-4d34-BED4-895F38A7A067") IDispSvrVideoEffectManager;
MIDL_INTERFACE("840A6B96-8D4E-4f6f-B313-2B280E7BC93C") IDispSvrVideoMixerDependentView;
MIDL_INTERFACE("9CD52B55-027A-4807-866E-D793E086C8C7") IDispSvrVideoMixerPlaneStereoControl;

DEFINE_GUID(DispSvr_VideoProcStretchRect, 0xca906726, 0x30e0, 0x44e5, 0xb2, 0x8c, 0xba, 0x73, 0x10, 0x7b, 0x32, 0x55);
DEFINE_GUID(DispSvr_VideoProcPCOM, 0x8238195a, 0xf3ac, 0x41c7, 0xab, 0xe9, 0x19, 0x70, 0x37, 0x5, 0xc4, 0xd);
DEFINE_GUID(DispSvr_VideoProcAMDProprietary, 0x3c5323c1, 0x6fb7, 0x44f5, 0x90, 0x81, 0x05, 0x6b, 0xf2, 0xee, 0x44, 0x9d);
DEFINE_GUID(DispSvr_VideoProcFastComp, 0xae277730, 0xc587, 0x4c5c, 0x8f, 0x14, 0x39, 0x28, 0x29, 0xdc, 0x9b, 0xb3);
DEFINE_GUID(DispSvr_VideoProcDxvaHD, 0x6fb3b824, 0xb1b4, 0x4eb2, 0x90, 0xc4, 0x2a, 0x96, 0xb6, 0xdc, 0x1, 0x8c);

namespace DispSvr
{
    typedef int PLANE_ID;

    enum _PLANE_ID
    {
        PLANE_UNKNOWN = -1,
        PLANE_BACKGROUND = 0,
        PLANE_MAINVIDEO = 1,
        PLANE_SUBVIDEO = 2,
        PLANE_GRAPHICS = 3,
        PLANE_INTERACTIVE = 4,
        PLANE_OTHER = 5,
        PLANE_MAX = 6,

        // aliasing names
        PLANE_PG = PLANE_GRAPHICS,
        PLANE_SPIC = PLANE_GRAPHICS,
        PLANE_TEXT = PLANE_GRAPHICS,
        PLANE_BDJGRAPHICS = PLANE_INTERACTIVE,
        PLANE_IG = PLANE_INTERACTIVE,
        PLANE_IHD = PLANE_INTERACTIVE,
        PLANE_OSD = PLANE_OTHER
    };

    enum PLANE_INIT_FLAG
    {
        PLANE_INIT_EXTERNAL_SURFACE			= 1 << 0,
        /// D = S + (1 - A) * D, instead of D = A*S + (1 - A) * D, the default
        /// Used when the source colors are already multiplied by alpha.
        PLANE_INIT_PARTIAL_BLENDING			= 1 << 1,
        PLANE_INIT_FULLSCREEN_MIXING			= 1 << 2
    };

    enum PLANE_CAP_FLAG
    {
        PLANE_CAP_MAX_SIZE					= 1 << 0,	//< if the plane restricts max size
        PLANE_CAP_MIN_SIZE					= 1 << 1,	//< if the plane restricts min size
        PLANE_CAP_VIDEO_PROCAMP				= 1 << 2,	//< DEPRECATED, use IDispSvrVideoProcessor instead
        PLANE_CAP_VIDEO_DETAILFILTER		= 1 << 3,	//< DEPRECATED, use IDispSvrVideoProcessor instead
        PLANE_CAP_VIDEO_NOISEFILTER			= 1 << 4,	//< DEPRECATED, use IDispSvrVideoProcessor instead
        PLANE_CAP_VIDEO_DEINTERLACE			= 1 << 5,	//< DEPRECATED, use IDispSvrVideoProcessor instead
        PLANE_CAP_VIDEO_SCRAMBLE			= 1 << 6,
        PLANE_CAP_HW_PARTIAL_BLENDING		= 1 << 7
    };

    enum PLANE_FORMAT
    {
        PLANE_FORMAT_UNKNOWN = 0,
        PLANE_FORMAT_XRGB = MAKEFOURCC('X', 'R', 'G', 'B'),	//< 32 bit, X8R8G8B8
        PLANE_FORMAT_ARGB = MAKEFOURCC('A', 'R', 'G', 'B'),	//< 32 bit, A8R8G8B8
        PLANE_FORMAT_AYUV = MAKEFOURCC('A', 'Y', 'U', 'V'),	//< 32 bit, A8Y8U8V8
        PLANE_FORMAT_AV12 = MAKEFOURCC('A', 'V', '1', '2'),	//< 20 bit, NV12 4:2:0 + A8, or ANV12
        PLANE_FORMAT_NV12 = MAKEFOURCC('N', 'V', '1', '2'),	//< 12 bit, 4:2:0
        PLANE_FORMAT_NV24 = MAKEFOURCC('N', 'V', '2', '4'),	//< 12 bit, 4:2:0
        PLANE_FORMAT_IMC3 = MAKEFOURCC('I', 'M', 'C', '3'),	//< 12 bit, 4:2:0
        PLANE_FORMAT_YV12 = MAKEFOURCC('Y', 'V', '1', '2'),	//< 12 bit, 4:2:0
        PLANE_FORMAT_YUY2 = MAKEFOURCC('Y', 'U', 'Y', '2'),	//< 16 bit, YUV 4:2:2
        PLANE_FORMAT_P8	  = 41								//< same as D3DFMT_P8
    };

    enum PLANE_LOCK_FLAG
    {
        PLANE_LOCK_READONLY	=		1 << 0
    };

    enum VIDEO_FORMAT
    {
        VIDEO_FORMAT_PROGRESSIVE						= 0,
        VIDEO_FORMAT_FIELD_INTERLEAVED_EVEN_FIRST		= 1,
        VIDEO_FORMAT_FIELD_INTERLEAVED_ODD_FIRST		= 2
    };

    /// Flags to select field/frame according to VIDEO_FORMAT
    enum FIELD_SELECT
    {
        FIELD_SELECT_FIRST				= 0,
        FIELD_SELECT_SECOND				= 1,
        FIELD_SELECT_REPEAT_FIRST		= 2
    };

    enum EVENT_VIDEO_MIXING
    {
        EVENT_VIDEO_MIXING_BEGIN				= 0,
        EVENT_VIDEO_MIXING_END					= 1,
		EVENT_VIDEO_MIXING_CHANGE_DESTINATION	= 2,	//< param1: IUknown *pDestSurface, param2: NORMALIZEDRECT *nrcDst
        EVENT_VIDEO_MIXING_FRAMERATE_CONVERSION = 3,    //< param1: Enable = TRUE or FALSE.
    };

    enum MIXER_CAP_FLAG
    {
        /// if the mixer is capable of changing destination surface by SetDestination.
        MIXER_CAP_CAN_CHANGE_DESTINATION	= 1 << 0,
        /// if the mixer uses 3D render target. If not using 3D render target, no 3D drawing can be performed on
        /// top of mixing result.
        MIXER_CAP_3D_RENDERTARGET			= 1 << 1,
        /// The mixer can virtualize from origin point if the bit is on.
        MIXER_CAP_CAN_VIRTUALIZE_FROM_ORIGIN	= 1 << 2
    };

    enum MIXER_PROPERTY_FLAG
    {
        /// Virtualize video to the position of window related to desktop.
        /// Use virtualization can avoid extra scaling from render target size to video window size because
        /// the video is drawn to render target the same size of video window size.
        MIXER_PROPERTY_VIRTUALIZATION	= 1 << 0,
        /// Clear non-video area only
        MIXER_PROPERTY_CLEARNONVIDEOAREA   = 1 << 1,
        /// Virtualize video to the size of window and place it from origin (upper left corner, as x=0, y=0).
        MIXER_PROPERTY_VIRTUALIZE_FROM_ORIGIN	= 1 << 2,
		/// Retrieves a back buffer from the device's swap chain as the destination surface
		/// MIXER_PROPERTY_SWAP_CHAIN and MIXER_PROPERTY_RENDER_TARGET are mutually exclusive.
		MIXER_PROPERTY_SWAP_CHAIN = 1 << 3,
		/// Retrieves a render-target surface as the destination surface, 
		/// MIXER_PROPERTY_RENDER_TARGET and MIXER_PROPERTY_SWAP_CHAIN are mutually exclusive.
		MIXER_PROPERTY_RENDER_TARGET = 1 << 4,
    };


    enum MIXER_STEREO_MODE  
    { 
        MIXER_STEREO_MODE_DISABLED = 0, 
        MIXER_STEREO_MODE_ANAGLYPH = 1,                 //< default is MIXER_STEREO_MODE_ANAGLYPH
        MIXER_STEREO_MODE_NV_PRIVATE = 2, 
        MIXER_STEREO_MODE_SIDEBYSIDE = 3, 
        MIXER_STEREO_MODE_NV_STEREOAPI = 4,
        MIXER_STEREO_MODE_CHECKERBOARD = 5,
        MIXER_STEREO_MODE_OPTIMIZED_ANAGLYPH = 6,		//< reduce binocular rivalry by balancing luminance but partial color reproduction.
        MIXER_STEREO_MODE_HALFCOLOR_ANAGLYPH = 7,
        MIXER_STEREO_MODE_ROW_INTERLEAVED = 8,
        MIXER_STEREO_MODE_HALFCOLOR2_ANAGLYPH = 9,		//< increased saturation after half color
        MIXER_STEREO_MODE_COLUMN_INTERLEAVED = 10,
        MIXER_STEREO_MODE_COLOR_ANAGLYPH = 11,          //< original color anaglyph
		MIXER_STEREO_MODE_AMD_ACTIVE_STEREO = 12,     //< AMD active stereo mode
		MIXER_STEREO_MODE_HALF_SIDEBYSIDE_LR = 13,
		MIXER_STEREO_MODE_HALF_SIDEBYSIDE_RL = 14,
		MIXER_STEREO_MODE_HALF_TOPBOTTOM_LR = 15,
		MIXER_STEREO_MODE_HALF_TOPBOTTOM_RL = 16,
		MIXER_STEREO_MODE_HDMI_STEREO = 17, 
		MIXER_STEREO_MODE_DP_STEREO            = 18,
    }; 

    enum VIDEO_FILTER
    {
        VIDEO_FILTER_BRIGHTNESS			= 0,
        VIDEO_FILTER_CONTRAST			= 1,
        VIDEO_FILTER_HUE				= 2,
        VIDEO_FILTER_SATURATION			= 3,
        VIDEO_FILTER_NOISE_REDUCTION	= 4,
        VIDEO_FILTER_EDGE_ENHANCEMENT	= 5
    };

    enum PROCESSOR_TYPE
    {
        PROCESSOR_TYPE_HARDWARE = 0,
        PROCESSOR_TYPE_SOFTWARE = 1
    };

    enum PROCESSOR_CAPS
    {
        PROCESSOR_CAPS_DEINTERLACE_BLEND	= 1 << 0,
        PROCESSOR_CAPS_DEINTERLACE_BOB		= 1 << 1,
        PROCESSOR_CAPS_DEINTERLACE_ADAPTIVE	= 1 << 2,
        PROCESSOR_CAPS_DEINTERLACE_MOTION_COMPENSATION	= 1 << 3
    };

    enum FILTER_CAPS
    {
        FILTER_CAPS_BRIGHTNESS			= 1 << VIDEO_FILTER_BRIGHTNESS,
        FILTER_CAPS_CONTRAST			= 1 << VIDEO_FILTER_CONTRAST,
        FILTER_CAPS_HUE					= 1 << VIDEO_FILTER_HUE,
        FILTER_CAPS_SATURATION			= 1 << VIDEO_FILTER_SATURATION,
        FILTER_CAPS_NOISE_REDUCTION		= 1 << VIDEO_FILTER_NOISE_REDUCTION,
        FILTER_CAPS_EDGE_ENHANCEMENT	= 1 << VIDEO_FILTER_EDGE_ENHANCEMENT
    };

    enum STEREO_STREAM_MODE
    {
        STEREO_STREAM_NONE		= 0,		//< the same base view for left/right views
        STEREO_STREAM_OFFSET	 = 1,		//< offset to base view to produce left/right views
        STEREO_STREAM_DUALVIEW	= 2,			//< use base view and dependent view
        STEREO_STREAM_SIDEBYSIDE_LR = 3,    // use side by side buffer with left/right view
        STEREO_STREAM_SIDEBYSIDE_RL = 4,    // use side by side buffer with right/left view
        STEREO_STREAM_TOPBOTTOM_LR = 5,        //  use over-under buffer for left(top)/right(bottom) view
        STEREO_STREAM_TOPBOTTOM_RL = 6,         //  use under-over buffer for right(top)/left(bottom) view
        STEREO_STREAM_AUTO = 7                 //  auto-detected LR view by DispSvr for side-by side and top-bottom buffer format
    };

    enum STEREO_OFFSET_MODE
    {
        OFFSET_MODE_DISABLED = 0,
        OFFSET_MODE_FIXED_OFFSET = 1,
        OFFSET_MODE_SEQUENCE_ID = 2,
    };

    enum CLEAR_RECT_TARGET
    {
        CLEAR_RECT_TARGET_MAIN	= 0,
        CLEAR_RECT_TARGET_SUB	= 1
    };

    struct ClearRect
    {
        CLEAR_RECT_TARGET Target;
        NORMALIZEDRECT Rect;
    };

    // deprecated
    struct ColorControl
    {
        // ProcAmp
        float fBrightness;
        float fContrast;
        float fHue;
        float fSaturation;

        // Noise filter
        float fNoiseLumaLevel;
        float fNoiseLumaRadius;
        float fNoiseLumaThreshold;
        float fNoiseChromaLevel;
        float fNoiseChromaRadius;
        float fNoiseChromaThreshold;

        // Detail filter
        float fDetailLumaLevel;
        float fDetailLumaRadius;
        float fDetailLumaThreshold;
        float fDetailChromaLevel;
        float fDetailChromaRadius;
        float fDetailChromaThreshold;

        int reserved[16];
    };

    struct LumaKey
    {
        BOOL bEnable;
        UINT uLower;
        UINT uUpper;
    };

    struct AYUVSample8
    {
        UCHAR Cr;
        UCHAR Cb;
        UCHAR Y;
        UCHAR Alpha;
    };

    struct ValueRange
    {
        float fMinValue;		//< Minimum supported value.
        float fMaxValue;		//< Maximum supported value.
        float fDefaultValue;	//< Default value.
        float fStepSize;		//< Minimum increment between values.
    };

    struct PlaneInit
    {
        PLANE_ID PlaneID;
        UINT uWidth;
        UINT uHeight;
        DWORD dwFlags;			//< bitwise OR of PLANE_INIT_FLAG
        PLANE_FORMAT Format;
        AYUVSample8	Palette[256];
        IDispSvrVideoMixerEventListener *pListener;	//< Register to listen to events
    };

    struct DependentViewInit
    {
        UINT uViewID;
        DWORD dwReserved[7];
    };

    struct LockedRect
    {
        UINT uPitch;
        void *pBuffer;
    };

    struct VideoProperty
    {
        UINT uWidth;
        UINT uHeight;
        VIDEO_FORMAT Format;
        DWORD dwFieldSelect;		//< bitwise OR of FIELD_SELECT
        DWORD dwFrameRate1000;		//< planar frame rate
        REFERENCE_TIME rtStart;
        REFERENCE_TIME rtEnd;
        BOOL bStillMenuHint;
        REFERENCE_TIME rtDisplayTarget;
        BOOL bRepeatFrame;          //< a repeat frame
        BOOL bRestrictedContent;    //< the video is created with restricted flag.
		DWORD dwReserved[3];
    };

    struct PlaneCaps
    {
        DWORD dwFlags;				//< bitwise OR of PLANE_CAP_FLAG
        UINT uMaxWidth;
        UINT uMaxHeight;
        UINT uMinWidth;
        UINT uMinHeight;

        /// procamp, noise, detail filters are deprecated
        ValueRange BrightnessRange;
        ValueRange ContrastRange;
        ValueRange HueRange;
        ValueRange SaturationRange;

        ValueRange NoiseLumaLevelRange;
        ValueRange NoiseLumaRadiusRange;
        ValueRange NoiseLumaThresholdRange;
        ValueRange NoiseChromaLevelRange;
        ValueRange NoiseChromaRadiusRange;
        ValueRange NoiseChromaThresholdRange;

        ValueRange DetailLumaLevelRange;
        ValueRange DetailLumaRadiusRange;
        ValueRange DetailLumaThresholdRange;
        ValueRange DetailChromaLevelRange;
        ValueRange DetailChromaRadiusRange;
        ValueRange DetailChromaThresholdRange;

        UINT uNumBackwardSamples;
        UINT uNumForwardSamples;
        DWORD dwReserved[125];
    };

    struct PlaneDesc
    {
        UINT uWidth;
        UINT uHeight;
        UINT uPitch;
        PLANE_FORMAT Format;
        DWORD dwReserved[16];
    };

    /// When setting external surface, the caller must ensure the surface exists
    /// and is not changed until the surface is unregistered.
    /// One can turn on bQueryStatusOnly to check the current status of pSurface
    /// if it is still being used.
    struct ExternalSurfaceDesc
    {
        UINT uWidth;
        UINT uHeight;
        PLANE_FORMAT Format;
        IUnknown *pSurface;
        BOOL bQueryStatusOnly; 
        DWORD dwReserved[14];
    };

    struct MixerCaps
    {
        UINT uMaxPlaneNumber;
        DWORD dwFlags;		//< bitwise OR of MIXER_CAP_FLAG
        DWORD dwReserved[30];
    };

    struct MixerProperty
    {
        DWORD dwFlags;		//< bitwise OR of MIXER_PROPERTY_FLAG
        BOOL bStereoEnable;
        MIXER_STEREO_MODE eStereoMode;
        DWORD dwReserved[29];
    };

    struct UpdateBuffer
    {
        DWORD dwReserved[128];
    };

    struct VideoProcessorCaps
    {
        PROCESSOR_TYPE eType;		//< see PROCESSOR_TYPE
        UINT uNumBackwardSamples;
        UINT uNumForwardSamples;
        UINT uProcessorCaps;		//< see PROCESSOR_CAPS
        UINT uFilterCaps;			//< see FILTER_CAPS
        DWORD dwReserved[3];
    };

    struct StereoOffsetProperty
    {
        //eMode = OFFSET_MODE_FIXED_OFFSET: use nFixedOffset as offset pixel
        //eMode = OFFSET_MODE_SEQUENCE_ID : use nOffsetSqeuenceID as Index,then get correct offset pixel from offset table of main video
        //usage : Left view + offset pixel;Right view - offset pixel
        STEREO_OFFSET_MODE  eMode; 
        union
        {
            INT nFixedOffset;
            INT nOffsetSequenceID;
            INT nValue;
        } ;
        DWORD dwReserved[14];
    };

    struct VideoTimeStamp
    {
        REFERENCE_TIME rtStart;
        REFERENCE_TIME rtEnd;
        DWORD dwReserved[8];
    };
}	// namespace DispSvr

/// A callback interface for those who want to be notified.
interface IDispSvrVideoMixerEventListener : public IUnknown
{
    /// @param dwEvent EVENT_VIDEO_MIXING
    /// @param dwParam1 @see EVENT_VIDEO_MIXING
    /// @param dwParam2 @see EVENT_VIDEO_MIXING
    STDMETHOD(Notify)(DWORD dwEvent, DWORD dwParam1, DWORD dwParam2) = 0;
};

/// IDispSvrVideoMixerPlane represents one general plane in video mixer.
interface IDispSvrVideoMixerPlane : public IUnknown
{
    STDMETHOD(GetPlaneDesc)(DispSvr::PlaneDesc *pDesc) = 0;
    STDMETHOD(SetExternalSurface)(const DispSvr::ExternalSurfaceDesc *pEx) = 0;
    STDMETHOD(UpdateFromBuffer)(DispSvr::UpdateBuffer *pBuffer) = 0;
    STDMETHOD(LockBuffer)(DispSvr::LockedRect *pOut, const RECT *rcSrc, DWORD dwFlags) = 0;
    STDMETHOD(UnlockBuffer)() = 0;
    STDMETHOD(SetPlanarAlpha)(float fAlpha) = 0;
    STDMETHOD(GetPlanarAlpha)(float *pfAlpha) = 0;

    /// Set position of the plane.
    /// @param rcDst [0 - 1.0]
    /// @param rcSrc [0 - width] [0 - height]
    /// @param rcCrop [0 - 1.0]
    /// @param fAspectRatio [0 - 1.0]
    STDMETHOD(SetPosition)(const NORMALIZEDRECT *rcDst, const RECT *rcSrc, const NORMALIZEDRECT *rcCrop, float fAspectRatio) = 0;
    STDMETHOD(GetPosition)(NORMALIZEDRECT *rcDst, RECT *rcSrc, NORMALIZEDRECT *rcCrop, float *pfAspectRatio) = 0;
    STDMETHOD(SetDirtyRect)(const RECT *rcDirty) = 0;
};

/// IDispSvrVideoMixerVideoPlane represents one video plane in video mixer
/// In addition to IDispSvrVideoMixerPlane, it provides more video specific settings.
interface IDispSvrVideoMixerVideoPlane : public IDispSvrVideoMixerPlane
{
    STDMETHOD(SetVideoProperty)(const DispSvr::VideoProperty *pProperty) = 0;
    STDMETHOD(GetVideoProperty)(DispSvr::VideoProperty *pProperty) = 0;
    /// deprecated, use IDispSvrVideoProcessor
    STDMETHOD(SetColorControl)(const DispSvr::ColorControl *pCC) = 0;
    /// deprecated, use IDispSvrVideoProcessor
    STDMETHOD(GetColorControl)(DispSvr::ColorControl *pCC) = 0;
};

/// IDispSvrVideoMixerDependentView is created by queried from IDispSvrVideoMixerPlane
// Lock/UnlockDepViewBuffer should not be nested with IDispSvrVideoMixerPlane::Lock/UnlockBuffer calls to avoid deadlock.
interface IDispSvrVideoMixerDependentView : public IUnknown
{
    STDMETHOD(SetDeptViewExternalSurface)(UINT uViewID, const DispSvr::ExternalSurfaceDesc *pEx) = 0;
    STDMETHOD(UpdateDeptViewFromBuffer)(UINT uViewID, DispSvr::UpdateBuffer *pBuffer) = 0;
    STDMETHOD(LockDeptViewBuffer)(UINT uViewID, DispSvr::LockedRect *pOut, const RECT *rcSrc, DWORD dwFlags) = 0;
    STDMETHOD(UnlockDeptViewBuffer)(UINT uViewID) = 0;
};

/// IDispSvrVideoMixerPlaneStereoControl is queried from IDispSvrVideoMixerPlane
interface IDispSvrVideoMixerPlaneStereoControl : public IUnknown
{
    STDMETHOD(SetStereoStreamMode)(DispSvr::STEREO_STREAM_MODE eMode) = 0;
    STDMETHOD(GetStereoStreamMode)(DispSvr::STEREO_STREAM_MODE *peMode) = 0;
    STDMETHOD(SetPlaneMetaData)(const LPBYTE pMetaData, DWORD dwSize)  = 0;
    STDMETHOD(GetPlaneMetaData)(LPBYTE *ppMetaData, LPDWORD pdwSize) = 0;
    STDMETHOD(SetOffsetProperty)(const DispSvr::StereoOffsetProperty *pProperty) = 0;
    STDMETHOD(GetOffsetProperty)(DispSvr::StereoOffsetProperty *pProperty) = 0;
};

/// Interfaces for VideoMixer
interface IDispSvrVideoMixer : public IUnknown
{
    STDMETHOD(Execute)() = 0;
    STDMETHOD(SetDestination)(IUnknown *pSurface, const NORMALIZEDRECT *pDest) = 0;

    /// Query video mixer's capability
    /// @param pCaps output of the video mixer's capability.
    STDMETHOD(QueryCaps)(DispSvr::MixerCaps *pCaps) = 0;

    /// Create the plane wrapper to allocate internal buffer or register outside buffer.
    STDMETHOD(CreatePlane)(DispSvr::PlaneInit *pInit, REFIID riid, void **ppPlane) = 0;
    STDMETHOD(QueryPlaneFormatCount)(DispSvr::PLANE_ID PlaneID, UINT *pCount) = 0;
    STDMETHOD(QueryPlaneFormat)(DispSvr::PLANE_ID PlaneID, DispSvr::PLANE_FORMAT *pFormats) = 0;
    /// Query plane capabilities. App can also use QueryPlaneCaps to check if desired Format is supported.
    /// Main or sub video is required to support PLANE_FORMAT_YUY2.
    STDMETHOD(QueryPlaneCaps)(DispSvr::PLANE_ID PlaneID, DispSvr::PLANE_FORMAT Format, DispSvr::PlaneCaps *pCap) = 0;
    /// Set clear rectangles to video mixer. Set uCount=0 to clear current rectangles.
    STDMETHOD(SetClearRectangles)(UINT uCount, DispSvr::ClearRect *pRects) = 0;
    STDMETHOD(SetBackgroundColor)(COLORREF Color) = 0;
    STDMETHOD(GetBackgroundColor)(COLORREF *pColor) = 0;

    /// Set luma key for sub-video plane.
    STDMETHOD(SetLumaKey)(const DispSvr::LumaKey *pLumaKey) = 0;

    /// Retrieve current luma key for sub-video plane.
    STDMETHOD(GetLumaKey)(DispSvr::LumaKey *pLumaKey) = 0;
    STDMETHOD(SetProperty)(const DispSvr::MixerProperty *pProperty) = 0;
    STDMETHOD(GetProperty)(DispSvr::MixerProperty *pProperty) = 0;
};

/// IDispSvrVideoProcessor is exposed from IDispSvrVideoMixer instance to configure the processor used for main video.
/// Software video processor is only used when calling IDispSvrVideoMixerPlane::UpdateFromBuffer() from main video planes.
interface IDispSvrVideoProcessor : public IUnknown
{
    STDMETHOD(GetAvailableVideoProcessorModeCount)(UINT *pCount) = 0;
    STDMETHOD(GetAvailableVideoProcessorModes)(GUID *pGUID) = 0;
    STDMETHOD(GetVideoProcessorCaps)(LPCGUID lpGUID, DispSvr::VideoProcessorCaps *pCaps) = 0;
    STDMETHOD(GetVideoProcessorMode)(LPGUID lpGUID) = 0;
    STDMETHOD(SetVideoProcessorMode)(LPCGUID lpGUID) = 0;
    STDMETHOD(GetFilterValueRange)(DispSvr::VIDEO_FILTER eFilter, DispSvr::ValueRange *pValueRange) = 0;
    STDMETHOD(SetFilterValue)(DispSvr::VIDEO_FILTER eFilter, float fValue) = 0;
    STDMETHOD(GetFilterValue)(DispSvr::VIDEO_FILTER eFilter, float *pfValue) = 0;
};

interface IDispSvrVideoEffectManager : public IUnknown
{
    STDMETHOD(GetVideoEffectManager)(IUnknown **ppManager) = 0;
    STDMETHOD(SetVideoEffectManager)(IUnknown *pManager) = 0;
};

#endif	// _DISPSVR_VIDEO_MIXER_IMPL_H_
