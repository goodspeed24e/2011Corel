#ifndef FAST_COMPOSITING_DEVICE_INCLUDE
#define FAST_COMPOSITING_DEVICE_INCLUDE

#include "IntelAuxiliaryDevice.h"

// Fast Video Compositing Device for Blu-Ray
// {7DD2D599-41DB-4456-9395-8B3D18CEDCAE}
static const GUID DXVA2_FastCompositing = 
{ 0x7dd2d599, 0x41db, 0x4456, { 0x93, 0x95, 0x8b, 0x3d, 0x18, 0xce, 0xdc, 0xae } };

// Surface Registration Device
// {44DEE63B-77E6-4DD2-B2E1-443B130F2E2F}
static const GUID DXVA2_Registration_Device = 
{ 0x44dee63b, 0x77e6, 0x4dd2, { 0xb2, 0xe1, 0x44, 0x3b, 0x13, 0x0f, 0x2e, 0x2f } };

// Structures use 64-bit alignment
#pragma pack(8)

/*---------------------------------------------\
|  SURFACE REGISTRATION STRUCTURES AND ENUMS   |
\---------------------------------------------*/
// Surface registration operation
typedef enum _REGISTRATION_OP
{
    REG_IGNORE      = 0,    // Do nothing
    REG_REGISTER    = 1,    // Register surface
    REG_UNREGISTER  = 2     // Unregister surface
} REGISTRATION_OP;

// Surface registration entry
typedef struct _DXVA2_SAMPLE_REG
{
    REGISTRATION_OP     Operation;
    IDirect3DSurface9   *pD3DSurface;
} DXVA2_SAMPLE_REG, *PDXVA2_SAMPLE_REG;

// Surface registration request
typedef struct _DXVA2_SURFACE_REGISTRATION
{
    HANDLE              RegHandle;
    DXVA2_SAMPLE_REG    *pRenderTargetRequest;
    UINT                nSamples;
    DXVA2_SAMPLE_REG    *pSamplesRequest;
} DXVA2_SURFACE_REGISTRATION, *PDXVA2_SURFACE_REGISTRATION;

// Fast Compositing modes
typedef enum _FASTCOMP_MODE
{
    FASTCOMP_MODE_COMPOSITING = 0,
    FASTCOMP_MODE_PRE_PROCESS = 1
} FASTCOMP_MODE;


/*-----------------------------------------\
|  FAST COMPOSITING STRUCTURES AND ENUMS   |
\-----------------------------------------*/

// Fast Compositing DDI Function ID
#define FASTCOMP_BLT 0x100

#define FASTCOMP_DEPTH_BACKGROUND  -1
#define FASTCOMP_DEPTH_MAIN_VIDEO   0
#define FASTCOMP_DEPTH_SUB_VIDEO    1
#define FASTCOMP_DEPTH_SUBSTREAM    2
#define FASTCOMP_DEPTH_GRAPHICS     3

// Query Caps Types
typedef enum _FASTCOMP_QUERYTYPE
{
    FASTCOMP_QUERY_CAPS                = 1,
    FASTCOMP_QUERY_FORMAT_COUNT        = 2,
    FASTCOMP_QUERY_FORMATS             = 3,
    FASTCOMP_QUERY_REGISTRATION_HANDLE = 4,
    FASTCOMP_QUERY_FRAME_RATE          = 5,
    FASTCOMP_QUERY_CAPS2               = 6,
    FASTCOMP_QUERY_VPP_CAPS            = 7,
    FASTCOMP_QUERY_VPP_CAPS2           = 8,
    FASTCOMP_QUERY_VPP_FORMAT_COUNT    = 9,
    FASTCOMP_QUERY_VPP_FORMATS         = 10
} FASTCOMP_QUERYTYPE;

// Target Deinterlacing Modes
typedef enum _FASTCOMP_TARGET_MODE
{
    FASTCOMP_TARGET_PROGRESSIVE        = 0,
    FASTCOMP_TARGET_NO_DEINTERLACING   = 1
} FASTCOMP_TARGET_MODE;

#define FASTCOMP_TARGET_PROGRESSIVE_MASK        (0x01 << FASTCOMP_TARGET_PROGRESSIVE)
#define FASTCOMP_TARGET_NO_DEINTERLACING_MASK   (0x01 << FASTCOMP_TARGET_NO_DEINTERLACING)

// Deinterlacing Algorithm
typedef enum _FASTCOMP_DI_ALGORITHM
{
    FASTCOMP_DI_NONE = 0,
    FASTCOMP_DI_BOB  = 1,
    FASTCOMP_DI_ADI  = 2
} FASTCOMP_DI_ALGORITHM;

#define FASTCOMP_TOP_FIELD      0
#define FASTCOMP_BOTTOM_FIELD   1

// Source Caps
typedef struct _FASTCOMP_SRC_CAPS
{
    INT     iDepth;                   // Depth of the source plane

    UINT    bSimpleDI           :1;   // Simple DI (BOB) is available
    UINT    bAdvancedDI         :1;   // Advanced DI is available
    UINT    bProcAmp            :1;   // ProcAmp (color control) is available
    UINT    bInverseTelecine    :1;   // Inverse telecine is available
    UINT    bDenoiseFilter      :1;   // Denoise filter is available
    UINT    bInverseTelecineTS  :1;   // Inverse telecine timestamping is available
    UINT    bVariance1          :1;   // GT Variance is provided
    UINT    bVariance2          :1;   // Larabee Variance is provided
    UINT    bSceneDetection     :1;   // Scene Detection is provided
    UINT    bInterlacedScaling  :1;   // Interlaced Scaling is supported
    UINT    uReserved1          :22;  // Reserved

    UINT    bSourceBlending     :1;   // Source blending is available  D = A * S + (1-A) * RT
    UINT    bPartialBlending    :1;   // Partial blending is available D = S     + (1-A) * RT
    UINT    uReserved2          :30;  // Reserved

    UINT    bConstantBlending   :1;   // Constant blending is available
    UINT    uReserved3          :31;  // Reserved

    UINT    bLumaKeying         :1;   // Luma keying is available
    UINT    bExtendedGamut      :1;   // Source may be extended gamut xvYCC
    UINT    uReserved4          :30;  // Reserved

    INT     iMaxWidth;                // Max source width
    INT     iMaxHeight;               // Max source height
    INT     iNumBackwardSamples;      // Backward Samples
    INT     iNumForwardSamples;       // Forward Samples
} FASTCOMP_SRC_CAPS;

// Destination Caps
typedef struct _FASTCOMP_DST_CAPS
{
    INT     iMaxWidth;
    INT     iMaxHeight;
} FASTCOMP_DST_CAPS;

// Fast Compositing Source/Destination Caps
typedef struct _FASTCOMP_CAPS
{
    INT                 iMaxClearRects;
    INT                 iMaxSubstreams;
    FASTCOMP_SRC_CAPS   sPrimaryVideoCaps;
    FASTCOMP_SRC_CAPS   sSecondaryVideoCaps;
    FASTCOMP_SRC_CAPS   sSubstreamCaps;
    FASTCOMP_SRC_CAPS   sGraphicsCaps;
    FASTCOMP_DST_CAPS   sRenderTargetCaps;
} FASTCOMP_CAPS;

// Fast Compositing Extra Caps
typedef struct _FASTCOMP_CAPS2
{
    UINT            dwSize;
    UINT            TargetInterlacingModes  : 8;
    UINT            bTargetInSysMemory      : 1;
    UINT            bProcampControl         : 1;
    UINT            bDeinterlacingControl   : 1;
    UINT            bDenoiseControl         : 1;
    UINT            bDetailControl          : 1;
    UINT            bFmdControl             : 1;
    UINT            bVariances              : 1;
    UINT            bSceneDetection         : 1;
    UINT            Reserved                : 16;
} FASTCOMP_CAPS2;

// Fast Compositing Sample Formats (User output)
typedef struct _FASTCOMP_SAMPLE_FORMATS
{
    INT         iPrimaryVideoFormatCount;   // Primary Video
    INT         iSecondaryVideoFormatCount;
    INT         iSubstreamFormatCount;     // Presentation Graphics (BD)
    INT         iGraphicsFormatCount;      // Interactive Graphics (BD)
    INT         iRenderTargetFormatCount;
    INT         iBackgroundFormatCount;
    D3DFORMAT   *pPrimaryVideoFormats;
    D3DFORMAT   *pSecondaryVideoFormats;
    D3DFORMAT   *pSubstreamFormats;         // Presentation Graphics (BD)
    D3DFORMAT   *pGraphicsFormats;          // Interactive Graphics (BD)
    D3DFORMAT   *pRenderTargetFormats;
    D3DFORMAT   *pBackgroundFormats;
} FASTCOMP_SAMPLE_FORMATS;

// Fast Composition device creation parameters
typedef struct _FASTCOMP_CREATEDEVICE
{
    DXVA2_VideoDesc      *pVideoDesc;            // Main video parameters
    D3DFORMAT            TargetFormat;           // Render Target Format
    UINT                 iSubstreams     : 8;    // Max number of substreams to use
    FASTCOMP_MODE        iMode           : 2;    // Creation mode
    UINT                 Reserved        : 21;   // Reserved - must be zero
    UINT                 MBZ             : 1;    // Must be zero
} FASTCOMP_CREATEDEVICE;

// Fast Composition video sample
typedef struct _FASTCOMP_VideoSample
{
    REFERENCE_TIME          Start;
    REFERENCE_TIME          End;
    DXVA2_ExtendedFormat    SampleFormat;
    IDirect3DSurface9       *SrcSurface;
    RECT                    SrcRect;
    RECT                    DstRect;
    DXVA2_AYUVSample8       Palette[256];
    FLOAT                   ConstantAlpha;
    INT                     Depth;
    UINT                    bLumaKey         : 1;
    UINT                    bPartialBlending : 1;
    UINT                    bExtendedGamut   : 1;
    UINT                    uReserved        : 29;
    UINT                    iLumaLow;
    UINT                    iLumaHigh;
} FASTCOMP_VideoSample;

// Fast Compositing Blt request
typedef struct _FASTCOMP_BLT_PARAMS
{
    IDirect3DSurface9       *pRenderTarget;
    UINT                    SampleCount;
    FASTCOMP_VideoSample    *pSamples;
    REFERENCE_TIME          TargetFrame;
    RECT                    TargetRect;
    UINT                    ReservedCR              : 1;    // Reserved for ClearRects, deprecated
    UINT                    TargetMatrix            : 3;    // Render Target Matrix
    UINT                    TargetExtGamut          : 1;    // Render Target Extended Gamut
    FASTCOMP_TARGET_MODE    TargetIntMode           : 3;    // Render Target Interlacing Mode
    UINT                    TargetInSysMem          : 1;    // Render Target In System Memory
    UINT                    Reserved1               : 23;
    UINT                    iSizeOfStructure        : 16;   // Size of the structure
    UINT                    Reserved2               : 16;
    void                    *pReserved1;
    DXVA2_AYUVSample16      BackgroundColor;
    void                    *pReserved2;

    // Rev 1.4 parameters
    DXVA2_ProcAmpValues     ProcAmpValues;                  // Procamp parameters
    UINT                    bDenoiseAutoAdjust      : 1;    // Denoise Auto Adjust
    UINT                    bFMDEnable              : 1;    // Enable FMD
    UINT                    bSceneDetectionEnable   : 1;    // Enable Scene Detection
    UINT                    iDeinterlacingAlgorithm : 4;    // DI algorithm
    UINT                    Reserved3               : 25;
    WORD                    wDenoiseFactor;                 // Denoise factor (same as CUI 2.75)
    WORD                    wDetailFactor;                  // Detail factor (same as CUI 2.75)
    WORD                    wSpatialComplexity;             // [OUT] Spatial complexity
    WORD                    wTemporalComplexity;            // [OUT] Temporal complexity
    UINT                    iVarianceType;                  // [OUT] Variance type
    VOID                    *pVariances;                    // [OUT] Array of variances
    UINT                    iCadence                : 12;   // [OUT] Cadence type
    UINT                    bRepeatFrame            : 1;    // [OUT] Repeat frame flag
    UINT                    Reserved4               : 19;
    UINT                    iFrameNum;                      // [OUT] Frame Number
} FASTCOMP_BLT_PARAMS;

// Fast Compositing Query Format Count (DDI)
typedef struct _FASTCOMP_FORMAT_COUNT
{
    INT          iPrimaryFormats;               // Main video
    INT          iSecondaryFormats;             // Sub-video
    INT          iSubstreamFormats;             // Presentation Graphics (BD)
    INT          iGraphicsFormats;              // Interactive Graphics (BD)
    INT          iRenderTargetFormats;          // Render Target
    INT          iBackgroundFormats;            // Background layer
} FASTCOMP_FORMAT_COUNT;

// Fast Compositing Query Formats (DDI)
typedef struct _FASTCOMP_FORMATS
{
    INT          iPrimaryFormatSize;            // Main video
    INT          iSecondaryFormatSize;          // Sub-video
    INT          iSubstreamFormatSize;          // Presentation Graphics (BD)
    INT          iGraphicsFormatSize;           // Interactive Graphics (BD)
    INT          iRenderTargetFormatSize;       // Render Target
    INT          iBackgroundFormatSize;         // Background layer
    D3DFORMAT *  pPrimaryFormats;
    D3DFORMAT *  pSecondaryFormats;
    D3DFORMAT *  pSubstreamFormats;
    D3DFORMAT *  pGraphicsFormats;
    D3DFORMAT *  pRenderTargetFormats;
    D3DFORMAT *  pBackgroundFormats;
} FASTCOMP_FORMATS;

// Fast Compositing Query Frame Rate
typedef struct _FASTCOMP_FRAME_RATE
{
    INT          iMaxSrcWidth;       // [in]
    INT          iMaxSrcHeight;      // [in]
    INT          iMaxDstWidth;       // [in]
    INT          iMaxDstHeight;      // [in]
    INT          iMaxLayers;         // [in]
    INT          iFrameRate;         // [out]
} FASTCOMP_FRAME_RATE;

// Fast Composition Query Caps union
typedef struct _FASTCOMP_QUERYCAPS
{
    FASTCOMP_QUERYTYPE    Type;
    union
    {
        FASTCOMP_CAPS           sCaps;
        FASTCOMP_CAPS2          sCaps2;
        FASTCOMP_FORMAT_COUNT   sFmtCount;
        FASTCOMP_FORMATS        sFormats;
        HANDLE                  hRegistration;
        FASTCOMP_FRAME_RATE     sFrameRate;
    };
} FASTCOMP_QUERYCAPS;

#pragma pack()

class CFastCompositingDevice : public CIntelAuxiliaryDevice
{
private:
    BOOL    bIsPresent;
    BOOL    bIsRunning;
    FASTCOMP_DI_ALGORITHM m_DiAlgorithm; 

    IDirectXVideoProcessorService *m_pVPservice;
    IDirectXVideoProcessor        *m_pRegistrationDevice;
    HANDLE                         m_hRegistration;

    FASTCOMP_MODE                  m_iMode;
    FASTCOMP_CAPS                  m_Caps;
    FASTCOMP_CAPS2                 m_Caps2;

    IDirect3DSurface9             *m_pDummySurface;

public:
    CFastCompositingDevice  (HMODULE hComDXVA2,
							IDirect3DDevice9   *pD3DDevice9,
                            FASTCOMP_MODE       iMode = FASTCOMP_MODE_COMPOSITING);

    ~CFastCompositingDevice();

    HRESULT QueryCaps      (FASTCOMP_CAPS            **ppCaps);

    HRESULT QueryCaps2     (FASTCOMP_CAPS2           **ppCaps2);

    HRESULT QueryFormats   (FASTCOMP_SAMPLE_FORMATS  **ppFormats);

    static VOID ResetBltParams(FASTCOMP_BLT_PARAMS *pBlt);

    INT     QueryFrameRate (INT  iMaxLayers,
                            INT  iMaxSrcWidth,
                            INT  iMaxSrcHeight,
                            INT  iMaxDstWidth,
                            INT  iMaxDstHeight);

    HRESULT CreateService  (DXVA2_VideoDesc     *pVideoDesc,
                            D3DFORMAT           TargetFormat,
                            UINT                iSubstreams);

    HRESULT CreateSurface  (UINT                Width,
                            UINT                Height,
                            UINT                BackBuffers, 
                            D3DFORMAT           Format,
                            D3DPOOL             Pool,
                            DWORD               Usage,
                            DWORD               DxvaType,
                            IDirect3DSurface9   **ppSurface,
                            HANDLE              *pSharedHandle);

    HRESULT Register       (IDirect3DSurface9   **pSources,
                            INT                 iCount,
                            BOOL                bRegister);

    HRESULT FastCopy(IDirect3DSurface9  *pTarget,
                     IDirect3DSurface9  *pSource,
                     RECT               *pTargetRect,
                     RECT               *pSourceRect,
                     BOOL               bInterlacedInput,
                     BOOL               bInterlacedOutput,
                     DWORD              dwField);

    HRESULT FastCompositingBlt(FASTCOMP_BLT_PARAMS *pVideoCompositingBlt);

    BOOL    IsPresent() { return bIsPresent; }
    BOOL    IsRunning() { return bIsRunning; }
};


#endif // FAST_COMPOSITING_DEVICE_INCLUDE
