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

#ifndef _AMDMCOM_H
#define _AMDMCOM_H
#ifdef __cplusplus
extern "C" {
#endif

#ifdef WIN32
#define MCOM_API_CALL __stdcall
#else
#define MCOM_API_CALL
#endif

// A minor revision change indicates a backward-compatible change; a major revision change indicates a backward-incompatible 
#define MCOM_VERSION_MAJOR         1
#define MCOM_VERSION_MINOR         2
#define MCOM_VERSION ((MCOM_VERSION_MAJOR << 16) | MCOM_VERSION_MINOR)


typedef enum _MCOM_STATUS
{
    // good status
    MCOM_OK                     = 0x00000000,

    // bad status
    MCOM_FAIL                   = 0x80000000,
    MCOM_INVALID_PARAM          = 0x80000001,
    MCOM_NOT_IMPLEMENTED        = 0x80000002,
    MCOM_INVALID_INPUT_SIZE     = 0x80000003,
    MCOM_INVALID_OUTPUT_SIZE    = 0x80000004,

} MCOM_STATUS;

//
// MCOMCreate
//
// Used to create and initialize MCOM session and receive version and MCOMSession pointer
// for future calls to MCOM.
//

// Placeholder for creation flags
typedef unsigned int MCOM_CREATE_FLAGS;

typedef struct _MCOM_CREATE_INPUT
{
    unsigned int        size;               // Size of the input structure
    MCOM_CREATE_FLAGS   flags;              // Creation flags (Placeholder)
    void*               windowHandle;       // Window handle that holds the device context (NULL on Linux)
    void*               GfxDevice;          // 3D Device context (D3D9Device, XvBAContext)
} MCOM_CREATE_INPUT, *PMCOM_CREATE_INPUT;

typedef struct _MCOM_CREATE_OUTPUT
{
    unsigned int        size;               // Size of the output structure
    unsigned int        revision;           // MCOM revision
    void*               MCOMSession;        // MCOM session pointer, used for future calls
} MCOM_CREATE_OUTPUT, *PMCOM_CREATE_OUTPUT;

MCOM_STATUS MCOM_API_CALL MCOMCreate (PMCOM_CREATE_INPUT, PMCOM_CREATE_OUTPUT);

//
// MCOMBluRayDecodeStreamCaps
//
// Used to ascertain supported capabilities from the driver for hardware accelerated decode.
//

// MCOM_DECODE_STREAM_CAPS
// usage: 
// 1HD == 0 means 1 HD stream supported.
//  nonzero means not supported.
#define BD_DECODE_STREAM_CAPS_SUPPORTED   0x0

typedef struct _MCOM_DECODE_STREAM_CAPS
{
    unsigned int CAP_1HD_PROGRESSIVE;
    unsigned int CAP_1HD_OTHER;
    unsigned int CAP_1HD_1SD;
    unsigned int CAP_2HD_PROGRESSIVE;
    unsigned int CAP_2HD_OTHER;
    unsigned int reserved[27];
} MCOM_DECODE_STREAM_CAPS;

typedef struct _MCOM_GET_BLURAY_DECODE_STREAM_INPUT
{
    unsigned int        size;                       // Size of the input structure
    void*               MCOMSession;                // MCOM session pointer from creation
} MCOM_GET_BLURAY_DECODE_STREAM_INPUT, *PMCOM_GET_BLURAY_DECODE_STREAM_INPUT;


typedef struct _MCOM_GET_BLURAY_DECODE_STREAM_OUTPUT
{
    unsigned int                size;               // Size of the output structure
    MCOM_DECODE_STREAM_CAPS     DecodeStreamCaps;   // Supported decode stream caps
} MCOM_GET_BLURAY_DECODE_STREAM_OUTPUT, *PMCOM_GET_BLURAY_DECODE_STREAM_OUTPUT;

MCOM_STATUS MCOM_API_CALL MCOMBluRayDecodeStreamCaps (PMCOM_GET_BLURAY_DECODE_STREAM_INPUT, PMCOM_GET_BLURAY_DECODE_STREAM_OUTPUT);

//
// MCOMDestroy
//
// Destroys the MCOM session.  The MCOMSession pointer provided is then invalid for future
// calls to MCOM.
//
MCOM_STATUS MCOM_API_CALL MCOMDestroy(void* MCOMSession);

#ifdef __cplusplus
}
#endif
#endif
