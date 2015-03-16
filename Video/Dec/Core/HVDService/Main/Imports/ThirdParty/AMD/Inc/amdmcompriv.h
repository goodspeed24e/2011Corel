/*******************************************************************************
*
*
* Copyright (c) 2008 Advanced Micro Devices, Inc. (unpublished)
*
* All rights reserved. This notice is intended as a precaution against
* inadvertent publication and does not imply publication or any waiver of
* confidentiality. The year included in the foregoing notice is the year of
* creation of the work.
*
*
*******************************************************************************/

#ifndef _AMDMCOMPRIV_H
#define _AMDMCOMPRIV_H
#ifdef __cplusplus
extern "C" {
#endif

#include "amdmcom.h"

//
// MCOMDecodeTargetAccessCaps
//
// Used to query supported capabilities from the driver for transcoding.  Specifically
// for access to the decode output target buffer.
//
typedef struct _MCOM_DECODE_TARGET_ACCESS_SCALING_CAPS
{
    unsigned int        CAP_BILINEAR   : 1;     // Set if bilinear scaling is supported
    unsigned int        CAP_BICUBIC    : 1;     // Set if bicubic scaling is supported
    unsigned int        reserved1      : 8;     // Reserved, these bits will be set to 0
    unsigned int        CAP_INTERLACED : 1;     // Reserved for future use in the consideration
                                                // of scaling interlaced content.  If this bit
                                                // is set to 0, it is recommended that the
                                                // client use software scaling.
    unsigned int        reserved2      : 21;    // Reserved, these bits will be set to 0
} MCOM_DECODE_TARGET_ACCESS_SCALING_CAPS;

typedef struct _MCOM_DECODE_TARGET_ACCESS_OUTPUT_CAPS
{
    unsigned int        CAP_NV12     : 1;       // Set if NV12 output is supported
    unsigned int        CAP_YV12     : 1;       // Set if YV12 output is supported
    unsigned int        CAP_YUY2     : 1;       // Set if YUY2 output is supported
    unsigned int        reserved     : 29;      // Reserved, these bits will be set to 0
} MCOM_DECODE_TARGET_ACCESS_OUTPUT_CAPS;

typedef struct _MCOM_GET_DECODE_TARGET_ACCESS_CAPS_INPUT
{
    unsigned int        size;                   // Size of the input structure
    void*               MCOMSession;            // MCOM session pointer from creation
} MCOM_GET_DECODE_TARGET_ACCESS_CAPS_INPUT, *PMCOM_GET_DECODE_TARGET_ACCESS_CAPS_INPUT;

typedef struct _MCOM_GET_DECODE_TARGET_ACCESS_CAPS_OUTPUT
{
    unsigned int                            size;                   // Size of the output structure
    MCOM_DECODE_TARGET_ACCESS_SCALING_CAPS  ScalingCaps;            // Scaling Caps
    MCOM_DECODE_TARGET_ACCESS_OUTPUT_CAPS   OutputCaps;             // Output caps
} MCOM_GET_DECODE_TARGET_ACCESS_CAPS_OUTPUT, *PMCOM_GET_DECODE_TARGET_ACCESS_CAPS_OUTPUT;

MCOM_STATUS MCOM_API_CALL MCOMDecodeTargetAccessCaps (PMCOM_GET_DECODE_TARGET_ACCESS_CAPS_INPUT, PMCOM_GET_DECODE_TARGET_ACCESS_CAPS_OUTPUT);

//
// MCOMBeginDecodeTargetAccess
//
// Indicates to the decoder to begin optimization for transcoding such that the render target
// from the decoder will be locked and accessed by the CPU.  The locked buffer will
// be scaled and in the output format specified in the MCOM_BEGIN_DECODE_TARGET_ACCESS_INPUT
// structure passed through the call.  The correct pitch will be returned through the
// appropriate mechanism depending on the API and OS being used.
//
typedef enum _MCOM_DECODE_TARGET_ACCESS_SCALING_MODE
{
    MCOM_DECODE_TARGET_ACCESS_SCALING_MODE_OFF       = 0,            // Disable scaling
    MCOM_DECODE_TARGET_ACCESS_SCALING_MODE_BILINEAR  = 1,            // Use bilinear scaling (if supported)
    MCOM_DECODE_TARGET_ACCESS_SCALING_MODE_BICUBIC   = 2             // Use bicubic scaling (if supported)
} MCOM_DECODE_TARGET_ACCESS_SCALING_MODE;

typedef enum _MCOM_DECODE_TARGET_ACCESS_OUTPUT_MODE
{
    MCOM_DECODE_TARGET_ACCESS_OUTPUT_MODE_NV12       = 0,            // Output target is in NV12 layout
    MCOM_DECODE_TARGET_ACCESS_OUTPUT_MODE_YV12       = 1,            // Output target is in YV12 layout
    MCOM_DECODE_TARGET_ACCESS_OUTPUT_MODE_YUY2       = 2             // Output target is in YUY2 layout
} MCOM_DECODE_TARGET_ACCESS_OUTPUT_MODE;

typedef struct _MCOM_BEGIN_DECODE_TARGET_ACCESS_INPUT
{
    unsigned int                            size;           // Size of the input structure
    void*                                   MCOMSession;    // MCOM session pointer from creation
    void*                                   DecodeSession;  // Pointer to the decode session (IAMVideoAccelerator(DXVA1)/IDirectXVideoDecoder(DXVA2))

    MCOM_DECODE_TARGET_ACCESS_SCALING_MODE  scalingMode;    // Scaling mode to use from MCOM_SCALING_MODE enum
    unsigned int                            scaledWidth;    // Scaled width of the output if scalingMode is specified and supported
    unsigned int                            scaledHeight;   // Scaled height of the output if scalingMode is specified and supported

    MCOM_DECODE_TARGET_ACCESS_OUTPUT_MODE   outputMode;     // Output mode to use from MCOM_OUTPUT_MODE
} MCOM_BEGIN_DECODE_TARGET_ACCESS_INPUT, *PMCOM_BEGIN_DECODE_TARGET_ACCESS_INPUT;

typedef struct _MCOM_BEGIN_DECODE_TARGET_ACCESS_OUTPUT
{
    unsigned int        size;                   // Size of the output structure

    unsigned int        outputPitch;            // New target pitch for locks on the output of the decode if scaling is used
} MCOM_BEGIN_DECODE_TARGET_ACCESS_OUTPUT, *PMCOM_BEGIN_DECODE_TARGET_ACCESS_OUTPUT;

MCOM_STATUS MCOM_API_CALL MCOMBeginDecodeTargetAccess (PMCOM_BEGIN_DECODE_TARGET_ACCESS_INPUT, PMCOM_BEGIN_DECODE_TARGET_ACCESS_OUTPUT);

//
// MCOMEndDecodeTargetAccess
//
// Indicates to the decoder to end optimization for transcoding and to clean up all
// transcoding resources.
//
typedef struct _MCOM_END_DECODE_TARGET_ACCESS_INPUT
{
    unsigned int        size;                   // Size of the input structure
    void*               MCOMSession;            // MCOM session pointer from creation
    void*               DecodeSession;          // Pointer to the decode session (IAMVideoAccelerator(DXVA1)/IDirectXVideoDecoder(DXVA2))
} MCOM_END_DECODE_TARGET_ACCESS_INPUT, *PMCOM_END_DECODE_TARGET_ACCESS_INPUT;

typedef struct _MCOM_END_DECODE_TARGET_ACCESS_OUTPUT
{
    unsigned int        size;                   // Size of the output structure
} MCOM_END_DECODE_TARGET_ACCESS_OUTPUT, *PMCOM_END_DECODE_TARGET_ACCESS_OUTPUT;

MCOM_STATUS MCOM_API_CALL MCOMEndDecodeTargetAccess (PMCOM_END_DECODE_TARGET_ACCESS_INPUT, PMCOM_END_DECODE_TARGET_ACCESS_OUTPUT);

#ifdef __cplusplus
}
#endif
#endif
