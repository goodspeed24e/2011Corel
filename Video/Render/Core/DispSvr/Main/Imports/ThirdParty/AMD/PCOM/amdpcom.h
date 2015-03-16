/*******************************************************************************
*
*
* Copyright (c) 2007 Advanced Micro Devices, Inc. (unpublished)
*
* All rights reserved. This notice is intended as a precaution against
* inadvertent publication and does not imply publication or any waiver of
* confidentiality. The year included in the foregoing notice is the year of
* creation of the work.
*
*
*******************************************************************************/

#ifndef _AMDPCOM_H
#define _AMDPCOM_H

#include "XvYCCMetaData.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PCOM_API_CALL __stdcall

#define PCOM_MAX_COMPOSITION_PLANES         16
#define PCOM_MAX_CLEAR_RECTANGLES_COUNT     32
#define PCOM_MAX_PLANE_SURFACE_FORMATS      16
#define PCOM_MAX_PLANE_TYPE                 8

// A minor revision change indicates a backward-compatible change; a major revision change indicates a backward-incompatible 
#define PCOM_VERSION_MAJOR         1
#define PCOM_VERSION_MINOR         1
#define PCOM_VERSION ((PCOM_VERSION_MAJOR << 16) | PCOM_VERSION_MINOR)

/*  Four-character-code (FOURCC) */
#define PCOM_FOURCC(a,b,c,d)\
        (((unsigned int)(a)<<0) |\
         ((unsigned int)(b)<<8) |\
         ((unsigned int)(c)<<16)|\
         ((unsigned int)(d)<<24))

typedef enum _PCOM_SURFACE_FORMAT
{
    PCOM_NV12       = PCOM_FOURCC('N','V','1','2'), /* 12bit  Y/CbCr 4:2:0 planar  */
    PCOM_YUY2       = PCOM_FOURCC('Y','U','Y','2'), /* 16bit  YUV 4:2:2 */
    PCOM_ARGB       = PCOM_FOURCC('A','R','G','B'), /* 32bit  ARGB-8-8-8-8  */
    PCOM_AYUV       = PCOM_FOURCC('A','Y','U','V'), /* 32bit  AYUV-8-8-8-8  */

} PCOM_SURFACE_FORMAT;

// Rectangle identified by the upper-left and lower-right corners. It does not include the right column and bottom row of pixels.
typedef struct _PCOM_RECT
{
    int left;
    int top;
    int right;
    int bottom;

} PCOM_RECT, *PPCOM_RECT;

typedef enum _PCOM_PLANE_TYPE
{
    PCOM_MainVideo_Plane    = 0,
    PCOM_SubVideo_Plane     = 1,
    PCOM_Generic_Plane      = 2,

} PCOM_PLANE_TYPE;

typedef enum _PCOM_FIELD_SELECT
{
    PCOM_ProgressiveFrame   = 0,
    PCOM_InterlaceFieldEven = 1,
    PCOM_InterlaceFieldOdd  = 2,

} PCOM_FIELD_SELECT;

typedef enum _PCOM_CSC_MATRIX
{
    PCOM_CSC_Matrix_Undefined = 0,
    PCOM_CSC_Matrix_BT601 = 1,
    PCOM_CSC_Matrix_BT709 = 2,

} PCOM_CSC_MATRIX;

typedef enum _PCOM_CSC_NOMINAL_RANGE
{
    PCOM_CSC_Nominal_Range_0_255 = 0,
    PCOM_CSC_Nominal_Range_16_235 = 1,

} PCOM_CSC_NOMINAL_RANGE;

typedef struct _PCOM_CSC_TYPE
{
    PCOM_CSC_MATRIX         cscMatrix;
    PCOM_CSC_NOMINAL_RANGE  cscRange;

} PCOM_CSC_TYPE;

typedef enum _PCOM_CLEAR_RECT_TYPE
{
    PCOM_MainVideo_ClearRect    = 0,
    PCOM_SubVideo_ClearRect     = 1,

} PCOM_CLEAR_RECT_TYPE;

typedef struct _PCOM_CLEAR_RECTANGLE
{
    PCOM_CLEAR_RECT_TYPE    type;
    PCOM_RECT               clearRect;

} PCOM_CLEAR_RECTANGLE;

typedef struct _PCOM_VALID_PLANE_FIELDS
{
   union
   {
       struct
       {
           unsigned int hasPlanarAlpha      : 1;    // Valid for subvideo plane only
           unsigned int hasLumaKey          : 1;    // Valid for subvideo plane only
           unsigned int hasBackgroundColor  : 1;    // Valid for background plane only
           unsigned int hasClearRectangles  : 1;    // Valid for generic planes only
           unsigned int hasDirtyRect        : 1;    // Valid for any plane
           unsigned int hasFps              : 1;    // Valid for any plane
           unsigned int hasCsc              : 1;    // Valid for video plane only

           unsigned int reserved            :25;
       };
       unsigned int value;
    };
} PCOM_VALID_PLANE_FIELDS;

typedef struct _PCOM_PLANE_FLAGS
{
   union
   {
       struct
       {
           unsigned int applyConstriction   : 1;    // Valid only for video plane

           unsigned int reserved            :31;
       };
       unsigned int value;
    };
} PCOM_PLANE_FLAGS;

typedef struct _PCOM_PLANE
{
    unsigned int            size;               // structure size
    PCOM_PLANE_TYPE         planeType;          // plane type
    PCOM_FIELD_SELECT       fieldSelect;        // field selection (valid for video/subvideo planes only)
    PCOM_VALID_PLANE_FIELDS validFields;        // valid fields in the plane structure
    PCOM_PLANE_FLAGS        flags;              // plane flags

    float                   fps;                // update frame rate

    void*                   plane;              // HW surface

    PCOM_RECT               srcRect;            // source rectangle
    PCOM_RECT               dstRect;            // destination rectangle
    
    unsigned int            alpha;              // planar alpha

    // dirty rectangle
    PCOM_RECT               dirtyRect;          // Specify the dirty rectangle of the plane

    // subvideo plane only 
    unsigned char           minLumaKey;         // min value of luma key
    unsigned char           maxLumaKey;         // max value of luma key
    // background plane only
    unsigned int            backgroundColor;    // use a solid background color instead of a background image
    // clear rectangles ?for advanced application plane only
    unsigned int            clearRectCount;
    PCOM_CLEAR_RECTANGLE    clearRectList[PCOM_MAX_CLEAR_RECTANGLES_COUNT];
    // CSC
    PCOM_CSC_TYPE           csc;                // specify the CSC matrix and range used to convert from YUV to RGB    
} PCOM_PLANE, *PPCOM_PLANE;

typedef struct _PCOM_CAPS_FLAGS
{
   union
   {
       struct
       {
           // supported features
           unsigned int supportNoiseReduction           : 1;    // PCOM support noise reduction
           unsigned int supportColorEnhancement         : 1;    // PCOM support color enhancement
           unsigned int supportDetailEnhancement        : 1;    // PCOM support detail enhancement
           unsigned int supportAppTargets               : 1;    // PCOM support composition to application provided target surfaces
           unsigned int supportOverlayPresentMode       : 1;    // PCOM support presentation using HW overlay
           unsigned int supportFullscreenMode           : 1;    // PCOM support full screen mode
           unsigned int supportSpatialDeinterlacing     : 1;    // PCOM support spatial deinterlacing
           unsigned int supportTemporalDeinterlacing    : 1;    // PCOM support temporal deinterlacing
           unsigned int supportXvYCCWideGamutDisplay    : 1;    // PCOM support XvYCC Wide Gamut Display
           unsigned int reserved                        :23;
       };
       unsigned int value;
    };
} PCOM_CAPS_FLAGS;

typedef struct _PCOM_PLANE_FORMAT
{
    unsigned int        count;
    PCOM_SURFACE_FORMAT format[PCOM_MAX_PLANE_SURFACE_FORMATS];

} PCOM_PLANE_FORMAT;

typedef struct _PCOM_GET_CAPS_INPUT
{
    unsigned int        size;                   // structure size

    void*               GfxDevice;              // Gfx device

} PCOM_GET_CAPS_INPUT, *PPCOM_GET_CAPS_INPUT;

typedef struct _PCOM_GET_CAPS_OUTPUT
{
    unsigned int        size;                   // structure size

    unsigned int        revision;               // PCOM revision

    PCOM_CAPS_FLAGS     flags;                  // PCOM capabilities

    unsigned int        systemToVideoBandwidth; // Total number of bytes from system to video memory per second

    unsigned int        recomFps;               // Recommended output FPS

    unsigned int        recomWindowPixelCnt;    // Recommended output window pixel count (width * height)

    // list supported surface format for each plane type
    unsigned int        supportedFormatsCount;
    PCOM_PLANE_FORMAT   supportedFormats[PCOM_MAX_PLANE_TYPE];

} PCOM_GET_CAPS_OUTPUT, *PPCOM_GET_CAPS_OUTPUT;

typedef struct _PCOM_CREATE_FLAGS
{
   union
   {
       struct
       {
           // enable/disable features
           unsigned int useNoiseReduction       : 1;    // Enable noise reduction
           unsigned int useColorEnhancement     : 1;    // Enable color enhancement
           unsigned int useDetailEnhancement    : 1;    // Enable detail enhancement
           unsigned int useOnlyAppTargets       : 1;    // Composition will be done only to app provided target surfaces
           unsigned int useOverlayPresent       : 1;    // Presentation is done using HW overlay
           unsigned int useAsyncPresent         : 1;    // Present calls will wait and return after flip

           unsigned int reserved                :26;
       };
       unsigned int value;
    };
} PCOM_CREATE_FLAGS;

typedef struct _PCOM_CREATE_INPUT
{
    unsigned int        size;               // structure size

    // enable/disable any supported features
    PCOM_CREATE_FLAGS   flags;              // valid for current PCOM instance only

    unsigned int        overlayColorKey;    // overlay color key - used only in overlay present mode

    unsigned int        mainVideoWidth;     // main video width
    unsigned int        mainVideoHeight;    // main video height

    void*               windowHandle;       // handle to playback window
    void*               GfxDevice;          // Gfx device

} PCOM_CREATE_INPUT, *PPCOM_CREATE_INPUT;

typedef struct _PCOM_CREATE_OUTPUT
{
    unsigned int        size;               // structure size

    unsigned int        flipChainSize;      // total number of surfaces on the flip chain

    void*               PCOMSession;        // pointer to PCOM session data

} PCOM_CREATE_OUTPUT, *PPCOM_CREATE_OUTPUT;

typedef struct _PCOM_BEGIN_FRAME_FLAGS
{
   union
   {
       struct
       {
           unsigned int enableFullScreenMode    : 1;    // enable/disable fullscreen mode per frame
           unsigned int enableXvYCCMetaData     : 1;    // enable XvYCC mode
           unsigned int reserved                :30;
       };
       unsigned int value;
    };
} PCOM_BEGIN_FRAME_FLAGS;

typedef struct _PCOM_BEGIN_FRAME_INPUT
{
    unsigned int            size;           // structure size

    PCOM_BEGIN_FRAME_FLAGS  flags;          // begin frame flags

    // target
    PCOM_RECT               targetRect;     // relative to current desktop for PCOM selected target
    void*                   targetSurface;  // NULL = target surface selected by PCOM and added to present queue
    PXVYCC_GAMUT_METADATA   xvYCCMetaData;  // must be valid if flags.enableXvYCCMetaData true
} PCOM_BEGIN_FRAME_INPUT, *PPCOM_BEGIN_FRAME_INPUT;

typedef struct _PCOM_EXECUTE_INPUT
{
    unsigned int            size;           // structure size

    // input planes in Z order
    unsigned int            planeCount;     // input plane count
    PCOM_PLANE              planeList[PCOM_MAX_COMPOSITION_PLANES];

} PCOM_EXECUTE_INPUT, *PPCOM_EXECUTE_INPUT;

typedef struct _PCOM_END_FRAME_FLAGS
{
   union
   {
       struct
       {
           unsigned int discardCurrentFrame : 1;    // Discard all PCOM rendering for current frame

           unsigned int reserved            :31;
       };
       unsigned int value;
    };
} PCOM_END_FRAME_FLAGS;

typedef struct _PCOM_END_FRAME_INPUT
{
    unsigned int            size;           // structure size

    PCOM_END_FRAME_FLAGS    flags;          // end frame flags

} PCOM_END_FRAME_INPUT, *PPCOM_END_FRAME_INPUT;

typedef struct _PCOM_PRESENT_OUTPUT
{
    unsigned int    size;                   // structure size

    unsigned int    numOfQueuedUpFrames;    // Number of queued up frames left in the display queue

} PCOM_PRESENT_OUTPUT, *PPCOM_PRESENT_OUTPUT;

typedef enum _PCOM_STATUS
{
    // good status
    PCOM_OK                     = 0x00000000,

    // bad status
    PCOM_FAIL                   = 0x80000000,
    PCOM_OUT_OF_MEM             = 0x80000001,
    PCOM_INVALID_PARAM          = 0x80000002,
    PCOM_NOT_IMPLEMENTED        = 0x80000003,
    PCOM_QUEUE_FULL             = 0x80000004,   // used only by BeginFrame
    PCOM_COMP_NOT_READY         = 0x80000005,   // used only by Present on async mode
    PCOM_FLIP_PENDING           = 0x80000006,   // used only by Present on async mode
    PCOM_FEATURE_NOT_SUPPORTED  = 0x80000007,
    PCOM_INVALID_INPUT_SIZE     = 0x80000008,
    PCOM_INVALID_OUTPUT_SIZE    = 0x80000009,
} PCOM_STATUS;

// get API caps
PCOM_STATUS PCOM_API_CALL PCOMGetCaps (PPCOM_GET_CAPS_INPUT, PPCOM_GET_CAPS_OUTPUT);

// init PCOM and return PCOM state and handle
PCOM_STATUS PCOM_API_CALL PCOMCreate (PPCOM_CREATE_INPUT, PPCOM_CREATE_OUTPUT);

// notify about beginning of a new frame
PCOM_STATUS PCOM_API_CALL PCOMBeginFrame (void* PCOMSession, PPCOM_BEGIN_FRAME_INPUT);

// send info about planes used for current frame
PCOM_STATUS PCOM_API_CALL PCOMExecute (void* PCOMSession, PPCOM_EXECUTE_INPUT);

// notify about ending of a frame
PCOM_STATUS PCOM_API_CALL PCOMEndFrame (void* PCOMSession, PPCOM_END_FRAME_INPUT);

// present a previous composed frame
PCOM_STATUS PCOM_API_CALL PCOMPresent (void* PCOMSession, PPCOM_PRESENT_OUTPUT);

// Reset the display queue
PCOM_STATUS PCOM_API_CALL PCOMResetQueue (void* PCOMSession);

// release all driver resources related to current PCOM
PCOM_STATUS PCOM_API_CALL PCOMDestroy (void* PCOMSession);

#ifdef __cplusplus
}
#endif

#endif /*_AMDPCOM_H*/
