 /***************************************************************************\
|*                                                                           *|
|*      Copyright 2005-2009 NVIDIA Corporation.  All rights reserved.        *|
|*                                                                           *|
|*   NOTICE TO USER:                                                         *|
|*                                                                           *|
|*   This source code is subject to NVIDIA ownership rights under U.S.       *|
|*   and international Copyright laws.                                       *|
|*                                                                           *|
|*   This software and the information contained herein is PROPRIETARY       *|
|*   and CONFIDENTIAL to NVIDIA and is being provided under the terms        *|
|*   and conditions of a Non-Disclosure Agreement. Any reproduction or       *|
|*   disclosure to any third party without the express written consent of    *|
|*   NVIDIA is prohibited.                                                   *|
|*                                                                           *|
|*   NVIDIA MAKES NO REPRESENTATION ABOUT THE SUITABILITY OF THIS SOURCE     *|
|*   CODE FOR ANY PURPOSE. IT IS PROVIDED "AS IS" WITHOUT EXPRESS OR         *|
|*   IMPLIED WARRANTY OF ANY KIND.  NVIDIA DISCLAIMS ALL WARRANTIES WITH     *|
|*   REGARD TO THIS SOURCE CODE, INCLUDING ALL IMPLIED WARRANTIES OF         *|
|*   MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR          *|
|*   PURPOSE. IN NO EVENT SHALL NVIDIA BE LIABLE FOR ANY SPECIAL,            *|
|*   INDIRECT, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES          *|
|*   WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN      *|
|*   AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING     *|
|*   OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOURCE      *|
|*   CODE.                                                                   *|
|*                                                                           *|
|*   U.S. Government End Users. This source code is a "commercial item"      *|
|*   as that term is defined at 48 C.F.R. 2.101 (OCT 1995), consisting       *|
|*   of "commercial computer software" and "commercial computer software     *|
|*   documentation" as such terms are used in 48 C.F.R. 12.212 (SEPT 1995)   *|
|*   and is provided to the U.S. Government only as a commercial end item.   *|
|*   Consistent with 48 C.F.R.12.212 and 48 C.F.R. 227.7202-1 through        *|
|*   227.7202-4 (JUNE 1995), all U.S. Government End Users acquire the       *|
|*   source code with only those rights set forth herein.                    *|
|*                                                                           *|
|*                                                                           *|
 \***************************************************************************/
///////////////////////////////////////////////////////////////////////////////
//
// Date: Jul 4, 2009
// File: nvapi.h
//
// NvAPI provides an interface to NVIDIA devices. This file contains the
// interface constants, structure definitions and function prototypes.
//
// Target Profile: NDA-developer
// Target OS-Arch: windows
//
///////////////////////////////////////////////////////////////////////////////
#ifndef _NVAPI_H
#define _NVAPI_H

#pragma pack(push,8) // Make sure we have consistent structure packings

#ifdef __cplusplus
extern "C" {
#endif

// ====================================================
// Universal NvAPI Definitions
// ====================================================
#ifndef _WIN32
#define __cdecl
#endif

#define NVAPI_INTERFACE extern NvAPI_Status __cdecl

/* 64-bit types for compilers that support them, plus some obsolete variants */
#if defined(__GNUC__) || defined(__arm) || defined(__IAR_SYSTEMS_ICC__) || defined(__ghs__) || defined(_WIN64)
typedef unsigned long long NvU64; /* 0 to 18446744073709551615          */
#else
typedef unsigned __int64   NvU64; /* 0 to 18446744073709551615              */
#endif

// mac os 32-bit still needs this
#if (defined(macintosh) || defined(__APPLE__)) && !defined(__LP64__)
typedef signed long        NvS32; /* -2147483648 to 2147483647               */
#else
typedef signed int         NvS32; /* -2147483648 to 2147483647               */
#endif

// mac os 32-bit still needs this
#if ( (defined(macintosh) && defined(__LP64__) && (__NVAPI_RESERVED0__)) || \
      (!defined(macintosh) && defined(__NVAPI_RESERVED0__)) ) 
typedef unsigned int       NvU32; /* 0 to 4294967295                         */
#else
typedef unsigned long      NvU32; /* 0 to 4294967295                         */
#endif

typedef unsigned short   NvU16;
typedef unsigned char    NvU8;

#define NV_DECLARE_HANDLE(name) struct name##__ { int unused; }; typedef struct name##__ *name

// NVAPI Handles - These handles are retrieved from various calls and passed in to others in NvAPI
//                 These are meant to be opaque types.  Do not assume they correspond to indices, HDCs,
//                 display indexes or anything else.
//
//                 Most handles remain valid until a display re-configuration (display mode set) or GPU
//                 reconfiguration (going into or out of SLI modes) occurs.  If NVAPI_HANDLE_INVALIDATED
//                 is received by an app, it should discard all handles, and re-enumerate them.
//
NV_DECLARE_HANDLE(NvDisplayHandle);                // Display Device driven by NVIDIA GPU(s) (an attached display)
NV_DECLARE_HANDLE(NvUnAttachedDisplayHandle);      // Unattached Display Device driven by NVIDIA GPU(s)
NV_DECLARE_HANDLE(NvLogicalGpuHandle);             // One or more physical GPUs acting in concert (SLI)
NV_DECLARE_HANDLE(NvPhysicalGpuHandle);            // A single physical GPU
NV_DECLARE_HANDLE(NvEventHandle);                  // A handle to an event registration instance
NV_DECLARE_HANDLE(NvVisualComputingDeviceHandle);  // A handle to Visual Computing Device
NV_DECLARE_HANDLE(NvGSyncDeviceHandle);            // A handle to a GSync device
NV_DECLARE_HANDLE(NvVioHandle);                    // A handle to a SDI device
NV_DECLARE_HANDLE(NvTransitionHandle);             // A handle to address a single transition request
NV_DECLARE_HANDLE(NvAudioHandle);                  // Nvidia HD Audio Device
typedef void* StereoHandle;

NV_DECLARE_HANDLE(NvSourceHandle);                 // Unique source handle on the system
NV_DECLARE_HANDLE(NvTargetHandle);                 // Unique target handle on the system

#define NVAPI_DEFAULT_HANDLE        0

#define NVAPI_GENERIC_STRING_MAX    4096
#define NVAPI_LONG_STRING_MAX       256
#define NVAPI_SHORT_STRING_MAX      64

typedef struct
{
    NvS32   sX;
    NvS32   sY;
    NvS32   sWidth;
    NvS32   sHeight;
} NvSBox;

#define NVAPI_MAX_PHYSICAL_GPUS             64
#define NVAPI_MAX_LOGICAL_GPUS              64
#define NVAPI_MAX_AVAILABLE_GPU_TOPOLOGIES  256
#define NVAPI_MAX_AVAILABLE_SLI_GROUPS      256
#define NVAPI_MAX_GPU_TOPOLOGIES            NVAPI_MAX_PHYSICAL_GPUS
#define NVAPI_MAX_GPU_PER_TOPOLOGY          8
#define NVAPI_MAX_DISPLAY_HEADS             2
#define NVAPI_MAX_DISPLAYS                  NVAPI_MAX_PHYSICAL_GPUS * NVAPI_MAX_DISPLAY_HEADS
#define NVAPI_MAX_ACPI_IDS                  16
#define NVAPI_MAX_VIEW_MODES                8
#define NV_MAX_HEADS                        4   // Maximum heads, each with NVAPI_DESKTOP_RES resolution
#define NVAPI_MAX_HEADS_PER_GPU             32

#define NV_MAX_HEADS        4   // Maximum heads, each with NVAPI_DESKTOP_RES resolution
#define NV_MAX_VID_STREAMS  4   // Maximum input video streams, each with a NVAPI_VIDEO_SRC_INFO
#define NV_MAX_VID_PROFILES 4   // Maximum output video profiles supported

#define NVAPI_SYSTEM_MAX_DISPLAYS           NVAPI_MAX_PHYSICAL_GPUS * NV_MAX_HEADS

typedef char NvAPI_String[NVAPI_GENERIC_STRING_MAX];
typedef char NvAPI_LongString[NVAPI_LONG_STRING_MAX];
typedef char NvAPI_ShortString[NVAPI_SHORT_STRING_MAX];

// =========================================================================================
// NvAPI Version Definition
// Maintain per structure specific version define using the MAKE_NVAPI_VERSION macro.
// Usage: #define NV_GENLOCK_STATUS_VER  MAKE_NVAPI_VERSION(NV_GENLOCK_STATUS, 1)
// =========================================================================================
#define MAKE_NVAPI_VERSION(typeName,ver) (NvU32)(sizeof(typeName) | ((ver)<<16))
#define GET_NVAPI_VERSION(ver) (NvU32)((ver)>>16)
#define GET_NVAPI_SIZE(ver) (NvU32)((ver) & 0xffff)

// ====================================================
// NvAPI Status Values
//    All NvAPI functions return one of these codes.
// ====================================================


typedef enum
{
    NVAPI_OK                                    =  0,      // Success
    NVAPI_ERROR                                 = -1,      // Generic error
    NVAPI_LIBRARY_NOT_FOUND                     = -2,      // nvapi.dll can not be loaded
    NVAPI_NO_IMPLEMENTATION                     = -3,      // not implemented in current driver installation
    NVAPI_API_NOT_INTIALIZED                    = -4,      // NvAPI_Initialize has not been called (successfully)
    NVAPI_INVALID_ARGUMENT                      = -5,      // invalid argument
    NVAPI_NVIDIA_DEVICE_NOT_FOUND               = -6,      // no NVIDIA display driver was found
    NVAPI_END_ENUMERATION                       = -7,      // no more to enum
    NVAPI_INVALID_HANDLE                        = -8,      // invalid handle
    NVAPI_INCOMPATIBLE_STRUCT_VERSION           = -9,      // an argument's structure version is not supported
    NVAPI_HANDLE_INVALIDATED                    = -10,     // handle is no longer valid (likely due to GPU or display re-configuration)
    NVAPI_OPENGL_CONTEXT_NOT_CURRENT            = -11,     // no NVIDIA OpenGL context is current (but needs to be)
    NVAPI_INVALID_POINTER                       = -14,     // An invalid pointer, usually NULL, was passed as a parameter
    NVAPI_NO_GL_EXPERT                          = -12,     // OpenGL Expert is not supported by the current drivers
    NVAPI_INSTRUMENTATION_DISABLED              = -13,     // OpenGL Expert is supported, but driver instrumentation is currently disabled
    NVAPI_EXPECTED_LOGICAL_GPU_HANDLE           = -100,    // expected a logical GPU handle for one or more parameters
    NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE          = -101,    // expected a physical GPU handle for one or more parameters
    NVAPI_EXPECTED_DISPLAY_HANDLE               = -102,    // expected an NV display handle for one or more parameters
    NVAPI_INVALID_COMBINATION                   = -103,    // used in some commands to indicate that the combination of parameters is not valid
    NVAPI_NOT_SUPPORTED                         = -104,    // Requested feature not supported in the selected GPU
    NVAPI_PORTID_NOT_FOUND                      = -105,    // NO port ID found for I2C transaction
    NVAPI_EXPECTED_UNATTACHED_DISPLAY_HANDLE    = -106,    // expected an unattached display handle as one of the input param
    NVAPI_INVALID_PERF_LEVEL                    = -107,    // invalid perf level
    NVAPI_DEVICE_BUSY                           = -108,    // device is busy, request not fulfilled
    NVAPI_NV_PERSIST_FILE_NOT_FOUND             = -109,    // NV persist file is not found
    NVAPI_PERSIST_DATA_NOT_FOUND                = -110,    // NV persist data is not found
    NVAPI_EXPECTED_TV_DISPLAY                   = -111,    // expected TV output display
    NVAPI_EXPECTED_TV_DISPLAY_ON_DCONNECTOR     = -112,    // expected TV output on D Connector - HDTV_EIAJ4120.
    NVAPI_NO_ACTIVE_SLI_TOPOLOGY                = -113,    // SLI is not active on this device
    NVAPI_SLI_RENDERING_MODE_NOTALLOWED         = -114,    // setup of SLI rendering mode is not possible right now
    NVAPI_EXPECTED_DIGITAL_FLAT_PANEL           = -115,    // expected digital flat panel
    NVAPI_ARGUMENT_EXCEED_MAX_SIZE              = -116,    // argument exceeds expected size
    NVAPI_DEVICE_SWITCHING_NOT_ALLOWED          = -117,    // inhibit ON due to one of the flags in NV_GPU_DISPLAY_CHANGE_INHIBIT or SLI Active
    NVAPI_TESTING_CLOCKS_NOT_SUPPORTED          = -118,    // testing clocks not supported
    NVAPI_UNKNOWN_UNDERSCAN_CONFIG              = -119,    // the specified underscan config is from an unknown source (e.g. INF)
    NVAPI_TIMEOUT_RECONFIGURING_GPU_TOPO        = -120,    // timeout while reconfiguring GPUs
    NVAPI_DATA_NOT_FOUND                        = -121,    // Requested data was not found
    NVAPI_EXPECTED_ANALOG_DISPLAY               = -122,    // expected analog display
    NVAPI_NO_VIDLINK                            = -123,    // No SLI video bridge present
    NVAPI_REQUIRES_REBOOT                       = -124,    // NVAPI requires reboot for its settings to take effect
    NVAPI_INVALID_HYBRID_MODE                   = -125,    // the function is not supported with the current hybrid mode.
    NVAPI_MIXED_TARGET_TYPES                    = -126,    // The target types are not all the same
    NVAPI_SYSWOW64_NOT_SUPPORTED                = -127,    // the function is not supported from 32-bit on a 64-bit system
    NVAPI_IMPLICIT_SET_GPU_TOPOLOGY_CHANGE_NOT_ALLOWED = -128,    //there is any implicit GPU topo active. Use NVAPI_SetHybridMode to change topology.
    NVAPI_REQUEST_USER_TO_CLOSE_NON_MIGRATABLE_APPS = -129,      //Prompt the user to close all non-migratable apps.
    NVAPI_OUT_OF_MEMORY                         = -130,    // Could not allocate sufficient memory to complete the call
    NVAPI_WAS_STILL_DRAWING                     = -131,    // The previous operation that is transferring information to or from this surface is incomplete
    NVAPI_FILE_NOT_FOUND                        = -132,    // The file was not found
    NVAPI_TOO_MANY_UNIQUE_STATE_OBJECTS         = -133,    // There are too many unique instances of a particular type of state object
    NVAPI_INVALID_CALL                          = -134,    // The method call is invalid. For example, a method's parameter may not be a valid pointer
    NVAPI_D3D10_1_LIBRARY_NOT_FOUND             = -135,    // d3d10_1.dll can not be loaded
    NVAPI_FUNCTION_NOT_FOUND                    = -136,    // Couldn't find the function in loaded dll library
    NVAPI_INVALID_USER_PRIVILEDGE               = -137,    // Current User is not Admin 
    NVAPI_EXPECTED_NON_PRIMARY_DISPLAY_HANDLE   = -138,    // The handle corresponds to GDIPrimary
    NVAPI_EXPECTED_COMPUTE_GPU_HANDLE           = -139,    // Setting Physx GPU requires that the GPU is compute capable
    NVAPI_STEREO_NOT_INITIALIZED                = -140,    // Stereo part of NVAPI failed to initialize completely. Check if stereo driver is installed.
    NVAPI_STEREO_REGISTRY_ACCESS_FAILED         = -141,    // Access to stereo related registry keys or values failed.
    NVAPI_STEREO_REGISTRY_PROFILE_TYPE_NOT_SUPPORTED = -142, // Given registry profile type is not supported.
    NVAPI_STEREO_REGISTRY_VALUE_NOT_SUPPORTED   = -143,    // Given registry value is not supported.
    NVAPI_STEREO_NOT_ENABLED                    = -144,    // Stereo is not enabled and function needed it to execute completely.
    NVAPI_STEREO_NOT_TURNED_ON                  = -145,    // Stereo is not turned on and function needed it to execute completely.
    NVAPI_STEREO_INVALID_DEVICE_INTERFACE       = -146,    // Invalid device interface.
    NVAPI_STEREO_PARAMETER_OUT_OF_RANGE         = -147,    // Separation percentage or JPEG image capture quality out of [0-100] range.
    NVAPI_STEREO_FRUSTUM_ADJUST_MODE_NOT_SUPPORTED = -148, // Given frustum adjust mode is not supported.
    NVAPI_TOPO_NOT_POSSIBLE                     = -149,    // The mosaic topo is not possible given the current state of HW
    NVAPI_MODE_CHANGE_FAILED                    = -150,    // An attempt to do a display resolution mode change has failed
    NVAPI_D3D11_LIBRARY_NOT_FOUND               = -151,    // d3d11.dll/d3d11_beta.dll cannot be loaded.
    NVAPI_INVALID_ADDRESS                       = -152,    // Address outside of valid range.
    NVAPI_STRING_TOO_SMALL                      = -153,    // The pre-allocated string is too small to hold the result.
    NVAPI_MATCHING_DEVICE_NOT_FOUND             = -154,    // The input does not match any of the available devices.
    NVAPI_DRIVER_RUNNING                        = -155,    // Driver is running
    NVAPI_DRIVER_NOTRUNNING                     = -156,    // Driver is not running
} NvAPI_Status;

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Initialize
//
//   DESCRIPTION: Initializes NVAPI library. This must be called before any
//                other NvAPI_ function.
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// RETURN STATUS: NVAPI_ERROR            Something is wrong during the initialization process (generic error)
//                NVAPI_LIBRARYNOTFOUND  Can not load nvapi.dll
//                NVAPI_OK                  Initialized
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Initialize();

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GetErrorMessage
//
//   DESCRIPTION: converts an NvAPI error code into a null terminated string
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// RETURN STATUS: null terminated string (always, never NULL)
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GetErrorMessage(NvAPI_Status nr,NvAPI_ShortString szDesc);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GetInterfaceVersionString
//
//   DESCRIPTION: Returns a string describing the version of the NvAPI library.
//                Contents of the string are human readable.  Do not assume a fixed
//                format.
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// RETURN STATUS: User readable string giving info on NvAPI's version
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GetInterfaceVersionString(NvAPI_ShortString szDesc);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GetDisplayDriverVersion
//
//   DESCRIPTION: Returns a struct that describes aspects of the display driver
//                build.
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// RETURN STATUS: NVAPI_ERROR or NVAPI_OK
//
///////////////////////////////////////////////////////////////////////////////
typedef struct
{
    NvU32              version;             // Structure version
    NvU32              drvVersion;
    NvU32              bldChangeListNum;
    NvAPI_ShortString  szBuildBranchString;
    NvAPI_ShortString  szAdapterString;
} NV_DISPLAY_DRIVER_VERSION;
#define NV_DISPLAY_DRIVER_VERSION_VER  MAKE_NVAPI_VERSION(NV_DISPLAY_DRIVER_VERSION,1)
NVAPI_INTERFACE NvAPI_GetDisplayDriverVersion(NvDisplayHandle hNvDisplay, NV_DISPLAY_DRIVER_VERSION *pVersion);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_EnumNvidiaDisplayHandle
//
//   DESCRIPTION: Returns the handle of the NVIDIA display specified by the enum
//                index (thisEnum). The client should keep enumerating until it
//                returns NVAPI_END_ENUMERATION.
//
//                Note: Display handles can get invalidated on a modeset, so the calling applications need to
//                renum the handles after every modeset.
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: either the handle pointer is NULL or enum index too big
//                NVAPI_OK: return a valid NvDisplayHandle based on the enum index
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA device found in the system
//                NVAPI_END_ENUMERATION: no more display device to enumerate.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_EnumNvidiaDisplayHandle(NvU32 thisEnum, NvDisplayHandle *pNvDispHandle);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_EnumNvidiaUnAttachedDisplayHandle
//
//   DESCRIPTION: Returns the handle of the NVIDIA UnAttached display specified by the enum
//                index (thisEnum). The client should keep enumerating till it
//                return error.
//
//                Note: Display handles can get invalidated on a modeset, so the calling applications need to
//                renum the handles after every modeset.
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: either the handle pointer is NULL or enum index too big
//                NVAPI_OK: return a valid NvDisplayHandle based on the enum index
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA device found in the system
//                NVAPI_END_ENUMERATION: no more display device to enumerate.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_EnumNvidiaUnAttachedDisplayHandle(NvU32 thisEnum, NvUnAttachedDisplayHandle *pNvUnAttachedDispHandle);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_EnumPhysicalGPUs
//
//   DESCRIPTION: Returns an array of physical GPU handles.
//
//                Each handle represents a physical GPU present in the system.
//                That GPU may be part of a SLI configuration, or not be visible to the OS directly.
//
//                At least 1 GPU must be present in the system and running an NV display driver.
//
//                The array nvGPUHandle will be filled with physical GPU handle values.  The returned
//                gpuCount determines how many entries in the array are valid.
//
//                Note: In drivers older than 105.00, all physical GPU handles get invalidated on a modeset. So the calling applications
//                      need to renum the handles after every modeset.
//                      With drivers 105.00 and up all physical GPU handles are constant.
//                      Physical GPU handles are constant as long as the GPUs are not physically moved and the SBIOS VGA order is unchanged.
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: nvGPUHandle or pGpuCount is NULL
//                NVAPI_OK: one or more handles were returned
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_EnumPhysicalGPUs(NvPhysicalGpuHandle nvGPUHandle[NVAPI_MAX_PHYSICAL_GPUS], NvU32 *pGpuCount);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_EnumLogicalGPUs
//
//   DESCRIPTION: Returns an array of logical GPU handles.
//
//                Each handle represents one or more GPUs acting in concert as a single graphics device.
//
//                At least 1 GPU must be present in the system and running an NV display driver.
//
//                The array nvGPUHandle will be filled with logical GPU handle values.  The returned
//                gpuCount determines how many entries in the array are valid.
//
//                Note: All logical GPUs handles get invalidated on a GPU topology change, so the calling application is required to
//                renum the logical GPU handles to get latest physical handle mapping after every GPU topology change activated
//                by a call to NvAPI_SetGpuTopologies.
//
//                To detect if SLI rendering is enabled please use NvAPI_D3D_GetCurrentSLIState
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: nvGPUHandle or pGpuCount is NULL
//                NVAPI_OK: one or more handles were returned
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_EnumLogicalGPUs(NvLogicalGpuHandle nvGPUHandle[NVAPI_MAX_LOGICAL_GPUS], NvU32 *pGpuCount);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GetPhysicalGPUsFromDisplay
//
//   DESCRIPTION: Returns an array of physical GPU handles associated with the specified display.
//
//                At least 1 GPU must be present in the system and running an NV display driver.
//
//                The array nvGPUHandle will be filled with physical GPU handle values.  The returned
//                gpuCount determines how many entries in the array are valid.
//
//                If the display corresponds to more than one physical GPU, the first GPU returned
//                is the one with the attached active output.
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: hNvDisp is not valid; nvGPUHandle or pGpuCount is NULL
//                NVAPI_OK: one or more handles were returned
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GetPhysicalGPUsFromDisplay(NvDisplayHandle hNvDisp, NvPhysicalGpuHandle nvGPUHandle[NVAPI_MAX_PHYSICAL_GPUS], NvU32 *pGpuCount);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GetPhysicalGPUFromUnAttachedDisplay
//
//   DESCRIPTION: Returns a physical source GPU handle associated with the specified unattached display.
//                The source GPU is a physical render GPU which renders the frame buffer but may or may not drive the scan out.
//
//                At least 1 GPU must be present in the system and running an NV display driver.
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: hNvUnAttachedDisp is not valid or pPhysicalGpu is NULL.
//                NVAPI_OK: one or more handles were returned
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GetPhysicalGPUFromUnAttachedDisplay(NvUnAttachedDisplayHandle hNvUnAttachedDisp, NvPhysicalGpuHandle *pPhysicalGpu);


///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_CreateDisplayFromUnAttachedDisplay
//
//   DESCRIPTION: The unattached display handle is converted to a active attached display handle.
//
//                At least 1 GPU must be present in the system and running an NV display driver.
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: hNvUnAttachedDisp is not valid or pNvDisplay is NULL.
//                NVAPI_OK: one or more handles were returned
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_CreateDisplayFromUnAttachedDisplay(NvUnAttachedDisplayHandle hNvUnAttachedDisp, NvDisplayHandle *pNvDisplay);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GetLogicalGPUFromDisplay
//
//   DESCRIPTION: Returns the logical GPU handle associated with the specified display.
//
//                At least 1 GPU must be present in the system and running an NV display driver.
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: hNvDisp is not valid; pLogicalGPU is NULL
//                NVAPI_OK: one or more handles were returned
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GetLogicalGPUFromDisplay(NvDisplayHandle hNvDisp, NvLogicalGpuHandle *pLogicalGPU);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GetLogicalGPUFromPhysicalGPU
//
//   DESCRIPTION: Returns the logical GPU handle associated with specified physical GPU handle.
//
//                At least 1 GPU must be present in the system and running an NV display driver.
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: hPhysicalGPU is not valid; pLogicalGPU is NULL
//                NVAPI_OK: one or more handles were returned
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GetLogicalGPUFromPhysicalGPU(NvPhysicalGpuHandle hPhysicalGPU, NvLogicalGpuHandle *pLogicalGPU);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GetPhysicalGPUsFromLogicalGPU
//
//   DESCRIPTION: Returns the physical GPU handles associated with the specified logical GPU handle.
//
//                At least 1 GPU must be present in the system and running an NV display driver.
//
//                The array hPhysicalGPU will be filled with physical GPU handle values.  The returned
//                gpuCount determines how many entries in the array are valid.
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: hLogicalGPU is not valid; hPhysicalGPU is NULL
//                NVAPI_OK: one or more handles were returned
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//                NVAPI_EXPECTED_LOGICAL_GPU_HANDLE: hLogicalGPU was not a logical GPU handle
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GetPhysicalGPUsFromLogicalGPU(NvLogicalGpuHandle hLogicalGPU,NvPhysicalGpuHandle hPhysicalGPU[NVAPI_MAX_PHYSICAL_GPUS], NvU32 *pGpuCount);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GetAssociatedNvidiaDisplayHandle
//
//   DESCRIPTION: Returns the handle of the NVIDIA display that is associated
//                with the display name given.  Eg: "\\DISPLAY1"
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: either argument is NULL
//                NVAPI_OK: *pNvDispHandle is now valid
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA device maps to that display name
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GetAssociatedNvidiaDisplayHandle(const char *szDisplayName, NvDisplayHandle *pNvDispHandle);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_DISP_GetAssociatedUnAttachedNvidiaDisplayHandle
//
//   DESCRIPTION: Returns the handle of an unattached NVIDIA display that is 
//                associated with the display name given.  Eg: "\\DISPLAY1"
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: either argument is NULL
//                NVAPI_OK: *pNvUnAttachedDispHandle is now valid
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA device maps to that display name
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DISP_GetAssociatedUnAttachedNvidiaDisplayHandle(const char *szDisplayName, NvUnAttachedDisplayHandle *pNvUnAttachedDispHandle);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GetAssociatedNvidiaDisplayName
//
//   DESCRIPTION: Returns the display name given.  Eg: "\\DISPLAY1" using the NVIDIA display handle
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: either argument is NULL
//                NVAPI_OK: *pNvDispHandle is now valid
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA device maps to that display name
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GetAssociatedNvidiaDisplayName(NvDisplayHandle NvDispHandle, NvAPI_ShortString szDisplayName);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GetUnAttachedAssociatedDisplayName
//
//   DESCRIPTION: Returns the display name given.  Eg: "\\DISPLAY1" using the NVIDIA unattached display handle
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: either argument is NULL
//                NVAPI_OK: *pNvDispHandle is now valid
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA device maps to that display name
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GetUnAttachedAssociatedDisplayName(NvUnAttachedDisplayHandle hNvUnAttachedDisp, NvAPI_ShortString szDisplayName);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_EnableHWCursor
//
//   DESCRIPTION: Enable hardware cursor support
//
//  SUPPORTED OS: Windows XP
//
// RETURN STATUS: NVAPI_ERROR or NVAPI_OK
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_EnableHWCursor(NvDisplayHandle hNvDisplay);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_DisableHWCursor
//
//   DESCRIPTION: Disable hardware cursor support
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS: NVAPI_ERROR or NVAPI_OK
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DisableHWCursor(NvDisplayHandle hNvDisplay);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GetVBlankCounter
//
//   DESCRIPTION: get vblank counter
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS: NVAPI_ERROR or NVAPI_OK
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GetVBlankCounter(NvDisplayHandle hNvDisplay, NvU32 *pCounter);

///////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME: NvAPI_SetRefreshRateOverride
//   DESCRIPTION: Override the refresh rate on the given  display/outputsMask.
//                The new refresh rate can be applied right away in this API call or deferred to happen with the
//                next OS modeset. The override is only good for one modeset (doesn't matter it's deferred or immediate).
//
//  SUPPORTED OS: Windows XP
//
//
//         INPUT: hNvDisplay - the NVIDIA display handle. It can be NVAPI_DEFAULT_HANDLE or a handle
//                             enumerated from NvAPI_EnumNVidiaDisplayHandle().
//
//                outputsMask - a set of bits that identify all target outputs which are associated with the NVIDIA
//                              display handle to apply the refresh rate override. Note when SLI is enabled,  the
//                              outputsMask only applies to the GPU that is driving the display output.
//
//                refreshRate - the override value. "0.0" means cancel the override.
//
//
//                bSetDeferred - "0": apply the refresh rate override immediately in this API call.
//                               "1":  apply refresh rate at the next OS modeset.
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: hNvDisplay or outputsMask is invalid
//                NVAPI_OK: the refresh rate override is correct set
//                NVAPI_ERROR: the operation failed
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_SetRefreshRateOverride(NvDisplayHandle hNvDisplay, NvU32 outputsMask, float refreshRate, NvU32 bSetDeferred);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GetAssociatedDisplayOutputId
//
//   DESCRIPTION: Gets the active outputId associated with the display handle.
//
//  SUPPORTED OS: Windows XP and higher
//
//    PARAMETERS: hNvDisplay(IN) - NVIDIA Display selection. It can be NVAPI_DEFAULT_HANDLE or a handle enumerated from NvAPI_EnumNVidiaDisplayHandle().
//                outputId(OUT)  - The active display output id associated with the selected display handle hNvDisplay.
//                                 The outputid will have only one bit set. In case of clone or span this  will indicate the display
//                                 outputId of the primary display that the GPU is driving.
// RETURN STATUS: NVAPI_OK: call successful.
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found.
//                NVAPI_EXPECTED_DISPLAY_HANDLE: hNvDisplay is not a valid display handle.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GetAssociatedDisplayOutputId(NvDisplayHandle hNvDisplay, NvU32 *pOutputId);


///////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME:   NvAPI_GetDisplayPortInfo
//
// DESCRIPTION:     This API returns the current DP related into on the specified device(monitor)
//
//  SUPPORTED OS: Windows XP and higher
//
// PARAMETERS:      hNvDisplay(IN) - NVIDIA Display selection. It can be NVAPI_DEFAULT_HANDLE or a handle enumerated from NvAPI_EnumNVidiaDisplayHandle().
//                  outputId(IN)   - The display output id. If it's "0" then the default outputId from NvAPI_GetAssociatedDisplayOutputId() will be used.
//                  pInfo(OUT)     - The display port info
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_ERROR - miscellaneous error occurred
//                  NVAPI_INVALID_ARGUMENT: Invalid input parameter.
//
///////////////////////////////////////////////////////////////////////////////
typedef enum
{
    NV_DP_1_62GBPS            = 6,
    NV_DP_2_70GBPS            = 0xA,
} NV_DP_LINK_RATE;

typedef enum
{
    NV_DP_1_LANE              = 1,
    NV_DP_2_LANE              = 2,
    NV_DP_4_LANE              = 4,
} NV_DP_LANE_COUNT;

typedef enum
{
    NV_DP_COLOR_FORMAT_RGB     = 0,
    NV_DP_COLOR_FORMAT_YCbCr422,
    NV_DP_COLOR_FORMAT_YCbCr444,
} NV_DP_COLOR_FORMAT;

typedef enum
{
    NV_DP_COLORIMETRY_RGB = 0,
    NV_DP_COLORIMETRY_YCbCr_ITU601,
    NV_DP_COLORIMETRY_YCbCr_ITU709,
} NV_DP_COLORIMETRY;

typedef enum
{
    NV_DP_DYNAMIC_RANGE_VESA  = 0,
    NV_DP_DYNAMIC_RANGE_CEA,
} NV_DP_DYNAMIC_RANGE;

typedef enum
{
    NV_DP_BPC_DEFAULT         = 0,
    NV_DP_BPC_6,
    NV_DP_BPC_8,
    NV_DP_BPC_10,
    NV_DP_BPC_12,
    NV_DP_BPC_16,
} NV_DP_BPC;

typedef struct
{
    NvU32               version;                     // structure version
    NvU32               dpcd_ver;                    // the DPCD version of the monitor
    NV_DP_LINK_RATE     maxLinkRate;                 // the max supported link rate
    NV_DP_LANE_COUNT    maxLaneCount;                // the max supported lane count
    NV_DP_LINK_RATE     curLinkRate;                 // the current link rate
    NV_DP_LANE_COUNT    curLaneCount;                // the current lane count
    NV_DP_COLOR_FORMAT  colorFormat;                 // the current color format
    NV_DP_DYNAMIC_RANGE dynamicRange;                // the dynamic range
    NV_DP_COLORIMETRY   colorimetry;                 // ignored in RGB space
    NV_DP_BPC           bpc;                         // the current bit-per-component;
    NvU32               isDp                   : 1;  // if the monitor is driven by display port
    NvU32               isInternalDp           : 1;  // if the monitor is driven by NV Dp transmitter
    NvU32               isColorCtrlSupported   : 1;  // if the color format change is supported
    NvU32               is6BPCSupported        : 1;  // if 6 bpc is supported
    NvU32               is8BPCSupported        : 1;  // if 8 bpc is supported
    NvU32               is10BPCSupported       : 1;  // if 10 bpc is supported
    NvU32               is12BPCSupported       : 1;  // if 12 bpc is supported
    NvU32               is16BPCSupported       : 1;  // if 16 bpc is supported
    NvU32               isYCrCb422Supported    : 1;  // if YCrCb422 is supported                                                  
    NvU32               isYCrCb444Supported    : 1;  // if YCrCb444 is supported
 } NV_DISPLAY_PORT_INFO;

#define NV_DISPLAY_PORT_INFO_VER   MAKE_NVAPI_VERSION(NV_DISPLAY_PORT_INFO,1)

NVAPI_INTERFACE NvAPI_GetDisplayPortInfo(NvDisplayHandle hNvDisplay, NvU32 outputId, NV_DISPLAY_PORT_INFO *pInfo);

///////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME:   NvAPI_SetDisplayPort
//
// DESCRIPTION:     This API is used to setup DP related configurations.
//
//  SUPPORTED OS: Windows XP and higher
//
// PARAMETERS:      hNvDisplay(IN) - NVIDIA display handle. It can be NVAPI_DEFAULT_HANDLE or a handle enumerated from NvAPI_EnumNVidiaDisplayHandle().
//                  outputId(IN)   - This display output ID, when it's "0" it means the default outputId generated from the return of NvAPI_GetAssociatedDisplayOutputId().
//                  pCfg(IN)       - The display port config structure. If pCfg is NULL, it means to use the driver's default value to setup.
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_ERROR - miscellaneous error occurred
//                  NVAPI_INVALID_ARGUMENT: Invalid input parameter.
///////////////////////////////////////////////////////////////////////////////
typedef struct
{
    NvU32               version;                     // structure version - 2 is latest
    NV_DP_LINK_RATE     linkRate;                    // the link rate
    NV_DP_LANE_COUNT    laneCount;                   // the lane count
    NV_DP_COLOR_FORMAT  colorFormat;                 // the color format to set
    NV_DP_DYNAMIC_RANGE dynamicRange;                // the dynamic range
    NV_DP_COLORIMETRY   colorimetry;                 // ignored in RGB space
    NV_DP_BPC           bpc;                         // the current bit-per-component;
    NvU32               isHPD               : 1;     // if CPL is making this call due to HPD
    NvU32               isSetDeferred       : 1;     // requires an OS modeset to finalize the setup if set
    NvU32               isChromaLpfOff      : 1;     // force the chroma low_pass_filter to be off
    NvU32               isDitherOff         : 1;     // force to turn off dither
    NvU32               testLinkTrain       : 1;     // if testing mode, skip validation
    NvU32               testColorChange     : 1;     // if testing mode, skip validation
} NV_DISPLAY_PORT_CONFIG;

#define NV_DISPLAY_PORT_CONFIG_VER   MAKE_NVAPI_VERSION(NV_DISPLAY_PORT_CONFIG,2)
#define NV_DISPLAY_PORT_CONFIG_VER_1 MAKE_NVAPI_VERSION(NV_DISPLAY_PORT_CONFIG,1)
#define NV_DISPLAY_PORT_CONFIG_VER_2 MAKE_NVAPI_VERSION(NV_DISPLAY_PORT_CONFIG,2)

NVAPI_INTERFACE NvAPI_SetDisplayPort(NvDisplayHandle hNvDisplay, NvU32 outputId, NV_DISPLAY_PORT_CONFIG *pCfg);

///////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME:   NvAPI_GetHDMISupportInfo
//
// DESCRIPTION:     This API returns the current infoframe data on the specified device(monitor)
//
//  SUPPORTED OS: Windows Vista and higher
//
// PARAMETERS:      hNvDisplay(IN) - NVIDIA Display selection. It can be NVAPI_DEFAULT_HANDLE or a handle enumerated from NvAPI_EnumNVidiaDisplayHandle().
//                  outputId(IN)   - The display output id. If it's "0" then the default outputId from NvAPI_GetAssociatedDisplayOutputId() will be used.
//                  pInfo(OUT)     - The monitor and GPU's HDMI support info
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_ERROR - miscellaneous error occurred
//                  NVAPI_INVALID_ARGUMENT: Invalid input parameter.
//
///////////////////////////////////////////////////////////////////////////////
typedef struct
{
    NvU32      version;                     // structure version
    NvU32      isGpuHDMICapable       : 1;  // if the GPU can handle HDMI
    NvU32      isMonUnderscanCapable  : 1;  // if the monitor supports underscan
    NvU32      isMonBasicAudioCapable : 1;  // if the monitor supports basic audio
    NvU32      isMonYCbCr444Capable   : 1;  // if YCbCr 4:4:4 is supported
    NvU32      isMonYCbCr422Capable   : 1;  // if YCbCr 4:2:2 is supported
    NvU32      isMonxvYCC601Capable   : 1;  // if xvYCC 601 is supported
    NvU32      isMonxvYCC709Capable   : 1;  // if xvYCC 709 is supported
    NvU32      isMonHDMI              : 1;  // if the monitor is HDMI (with IEEE's HDMI registry ID)
    NvU32      EDID861ExtRev;               // the revision number of the EDID 861 extension
 } NV_HDMI_SUPPORT_INFO;

#define NV_HDMI_SUPPORT_INFO_VER  MAKE_NVAPI_VERSION(NV_HDMI_SUPPORT_INFO,1)

NVAPI_INTERFACE NvAPI_GetHDMISupportInfo(NvDisplayHandle hNvDisplay, NvU32 outputId, NV_HDMI_SUPPORT_INFO *pInfo);

///////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME:   NvAPI_GetInfoFrame
//
// DESCRIPTION:     This API returns the current infoframe data on the specified device(monitor)
//
//  SUPPORTED OS: Windows XP and higher
//
// PARAMETERS:      hNvDisplay(IN) - NVIDIA Display selection. It can be NVAPI_DEFAULT_HANDLE or a handle enumerated from NvAPI_EnumNVidiaDisplayHandle().
//                  outputId(IN)   - The display output id. If it's "0" then the default outputId from NvAPI_GetAssociatedDisplayOutputId() will be used.
//                  type(IN)       - The type of infoframe to set
//                  pInfoFrame(IN) - The infoframe data
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_ERROR - miscellaneous error occurred
//                  NVAPI_INVALID_ARGUMENT: Invalid input parameter.
//
///////////////////////////////////////////////////////////////////////////////
typedef enum _NV_INFOFRAME_TYPE
{
    NV_INFOFRAME_TYPE_AVI   = 2,
    NV_INFOFRAME_TYPE_SPD   = 3,
    NV_INFOFRAME_TYPE_AUDIO = 4,
    NV_INFOFRAME_TYPE_MS    = 5,
} NV_INFOFRAME_TYPE;

typedef struct
{
    NvU8 type;
    NvU8 version;
    NvU8 length;
} NV_INFOFRAME_HEADER;

// since this is for Windows OS so far, we use this bit little endian defination
// to handle the translation
typedef struct
{
    // byte 1
    NvU8 channelCount     : 3;
    NvU8 rsvd_bits_byte1  : 1;
    NvU8 codingType       : 4;

    // byte 2
    NvU8 sampleSize       : 2;
    NvU8 sampleRate       : 3;
    NvU8 rsvd_bits_byte2  : 3;

    // byte 3
    NvU8  byte3;

    // byte 4
    NvU8  speakerPlacement;

    // byte 5
    NvU8 rsvd_bits_byte5  : 3;
    NvU8 levelShift       : 4;
    NvU8 downmixInhibit   : 1;

    // byte 6~10
    NvU8 rsvd_byte6;
    NvU8 rsvd_byte7;
    NvU8 rsvd_byte8;
    NvU8 rsvd_byte9;
    NvU8 rsvd_byte10;

}NV_AUDIO_INFOFRAME;

typedef struct
{
    // byte 1
    NvU8 scanInfo                : 2;
    NvU8 barInfo                 : 2;
    NvU8 activeFormatInfoPresent : 1;
    NvU8 colorSpace              : 2;
    NvU8 rsvd_bits_byte1         : 1;

    // byte 2
    NvU8 activeFormatAspectRatio : 4;
    NvU8 picAspectRatio          : 2;
    NvU8 colorimetry             : 2;

    // byte 3
    NvU8 nonuniformScaling       : 2;
    NvU8 rgbQuantizationRange    : 2;
    NvU8 extendedColorimetry     : 3;
    NvU8 itContent               : 1;

    // byte 4
    NvU8 vic                     : 7;
    NvU8 rsvd_bits_byte4         : 1;

    // byte 5
    NvU8 pixelRepeat             : 4;
    NvU8 rsvd_bits_byte5         : 4;

    // byte 6~13
    NvU8 topBarLow;
    NvU8 topBarHigh;
    NvU8 bottomBarLow;
    NvU8 bottomBarHigh;
    NvU8 leftBarLow;
    NvU8 leftBarHigh;
    NvU8 rightBarLow;
    NvU8 rightBarHigh;

} NV_VIDEO_INFOFRAME;

typedef struct
{
    NV_INFOFRAME_HEADER    header;
    union
    {
        NV_AUDIO_INFOFRAME audio;
        NV_VIDEO_INFOFRAME video;
    }u;
} NV_INFOFRAME;
NVAPI_INTERFACE NvAPI_GetInfoFrame(NvDisplayHandle hNvDisplay, NvU32 outputId, NV_INFOFRAME_TYPE type, NV_INFOFRAME *pInfoFrame);

///////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME:   NvAPI_SetInfoFrame
//
// DESCRIPTION:     This API returns the current infoframe data on the specified device(monitor)
//
//  SUPPORTED OS: Windows XP and higher
//
// PARAMETERS:      hNvDisplay(IN) - NVIDIA Display selection. It can be NVAPI_DEFAULT_HANDLE or a handle enumerated from NvAPI_EnumNVidiaDisplayHandle().
//                  outputId(IN)   - The display output id. If it's "0" then the default outputId from NvAPI_GetAssociatedDisplayOutputId() will be used.
//                  type(IN)       - The type of infoframe to set
//                  pInfoFrame(IN) - The infoframe data, NULL mean reset to the default value.
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_ERROR - miscellaneous error occurred
//                  NVAPI_INVALID_ARGUMENT: Invalid input parameter.
//
///////////////////////////////////////////////////////////////////////////////

NVAPI_INTERFACE NvAPI_SetInfoFrame(NvDisplayHandle hNvDisplay, NvU32 outputId, NV_INFOFRAME_TYPE type, NV_INFOFRAME *pInfoFrame);

///////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME:   NvAPI_SetInfoFrameState
//
// DESCRIPTION:     Disables or enabled the sending of infoframe packets. Currently, this is supported for audio packets only
//
//  SUPPORTED OS: Windows Vista and higher
//
// PARAMETERS:      hNvDisplay(IN) - NVIDIA Display selection. It can be NVAPI_DEFAULT_HANDLE or a handle enumerated from NvAPI_EnumNVidiaDisplayHandle().
//                  outputId(IN)   - The display output id. If it's "0" then the default outputId from NvAPI_GetAssociatedDisplayOutputId() will be used.
//                  type(IN)       - The type of infoframe to set
//                  state(IN)      - state of infoframe to set
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_ERROR - miscellaneous error occurred
//                  NVAPI_INVALID_ARGUMENT: Invalid input parameter.
//
///////////////////////////////////////////////////////////////////////////////

typedef struct _NV_INFOFRAME_STATE
{
    NvU32 bDisabled    :    1;       // enable or disable the infoframe when "bDriverCtrl" is 0
    NvU32 bDriverCtrl  :    1;       // if set, let the driver control the infoframe state and "bDisabled" is ignored.
    NvU32 reserved     :   30;
} NV_INFOFRAME_STATE;

NVAPI_INTERFACE NvAPI_SetInfoFrameState(NvDisplayHandle hNvDisplay, NvU32 outputId, NV_INFOFRAME_TYPE type, NV_INFOFRAME_STATE *pState);

///////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME:   NvAPI_GetInfoFrameState
//
// DESCRIPTION:     Gets the state of the infoframe. Currently, this is supported for audio packets only
//
//  SUPPORTED OS: Windows Vista and higher
//
// PARAMETERS:      hNvDisplay(IN) - NVIDIA Display selection. It can be NVAPI_DEFAULT_HANDLE or a handle enumerated from NvAPI_EnumNVidiaDisplayHandle().
//                  outputId(IN)   - The display output id. If it's "0" then the default outputId from NvAPI_GetAssociatedDisplayOutputId() will be used.
//                  type(IN)       - The type of infoframe to get state of
//                  state(OUT)      - state of infoframe
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_ERROR - miscellaneous error occurred
//                  NVAPI_INVALID_ARGUMENT: Invalid input parameter.
//
///////////////////////////////////////////////////////////////////////////////

NVAPI_INTERFACE NvAPI_GetInfoFrameState(NvDisplayHandle hNvDisplay, NvU32 outputId, NV_INFOFRAME_TYPE type, NV_INFOFRAME_STATE *pState);

typedef struct
{
    NvU32   version;                           // version info
    NvU32   dedicatedVideoMemory;              // size(in kb) of the physical framebuffer.
    NvU32   availableDedicatedVideoMemory;     // size(in kb) of the available physical framebuffer for allocating video memory surfaces.
    NvU32   systemVideoMemory;                 // size(in kb) of system memory the driver allocates at load time.
    NvU32   sharedSystemMemory;                // size(in kb) of shared system memory that driver is allowed to commit for surfaces across all allocations.

} NV_DISPLAY_DRIVER_MEMORY_INFO_V1;

typedef struct
{
    NvU32   version;                           // version info
    NvU32   dedicatedVideoMemory;              // size(in kb) of the physical framebuffer.
    NvU32   availableDedicatedVideoMemory;     // size(in kb) of the available physical framebuffer for allocating video memory surfaces.
    NvU32   systemVideoMemory;                 // size(in kb) of system memory the driver allocates at load time.
    NvU32   sharedSystemMemory;                // size(in kb) of shared system memory that driver is allowed to commit for surfaces across all allocations.
    NvU32   curAvailableDedicatedVideoMemory;  // size(in kb) of the current available physical framebuffer for allocating video memory surfaces.

} NV_DISPLAY_DRIVER_MEMORY_INFO_V2;

typedef NV_DISPLAY_DRIVER_MEMORY_INFO_V2 NV_DISPLAY_DRIVER_MEMORY_INFO;

#define NV_DISPLAY_DRIVER_MEMORY_INFO_VER_1  MAKE_NVAPI_VERSION(NV_DISPLAY_DRIVER_MEMORY_INFO_V1,1)
#define NV_DISPLAY_DRIVER_MEMORY_INFO_VER_2  MAKE_NVAPI_VERSION(NV_DISPLAY_DRIVER_MEMORY_INFO_V2,2)
#define NV_DISPLAY_DRIVER_MEMORY_INFO_VER    NV_DISPLAY_DRIVER_MEMORY_INFO_VER_2

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GetDisplayDriverMemoryInfo
//
//   DESCRIPTION: Retrieves the display driver memory information for the active display handle.
//                In case of multiGPU scenario the physical framebuffer information is obtained for the GPU associated with active display handle.
//                In case of SLI physical framebuffer information is obtained only from the display owner GPU.
//
//  SUPPORTED OS: Windows XP and higher
//
//    PARAMETERS: hNvDisplay(IN) - NVIDIA Display selection. It can be NVAPI_DEFAULT_HANDLE or a handle enumerated from NvAPI_EnumNVidiaDisplayHandle().
//                pMemoryInfo(OUT) - The memory footprint available in the driver.
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: pMemoryInfo is NULL.
//                NVAPI_OK: call successful.
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found.
//                NVAPI_EXPECTED_DISPLAY_HANDLE: hNvDisplay is not a valid display handle.
//                NVAPI_INCOMPATIBLE_STRUCT_VERSION: NV_DISPLAY_DRIVER_MEMORY_INFO structure version mismatch.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GetDisplayDriverMemoryInfo(NvDisplayHandle hNvDisplay, NV_DISPLAY_DRIVER_MEMORY_INFO *pMemoryInfo);


///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GetDriverMemoryInfo
//
//   DESCRIPTION: Retrieves the display driver memory information for the active display handle.
//                In case of multiGPU scenario the physical framebuffer information is obtained for the GPU associated with active display handle.
//                In case of SLI physical framebuffer information is obtained only from the display owner GPU.
//
//  SUPPORTED OS: Windows XP and higher
//
//    PARAMETERS: hNvDisplay(IN) - NVIDIA Display selection. It can be NVAPI_DEFAULT_HANDLE or a handle enumerated from NvAPI_EnumNVidiaDisplayHandle().
//                pMemoryInfo(OUT) - The memory footprint available in the driver.
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: pMemoryInfo is NULL.
//                NVAPI_OK: call successful.
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found.
//                NVAPI_EXPECTED_DISPLAY_HANDLE: hNvDisplay is not a valid display handle.
//                NVAPI_INCOMPATIBLE_STRUCT_VERSION: NV_DISPLAY_DRIVER_MEMORY_INFO structure version mismatch.
//
///////////////////////////////////////////////////////////////////////////////
typedef struct
{
    NvU32   version;                        // version info
    NvU32   dedicatedVideoMemory;           // size(in kb) of the physical framebuffer.
    NvU32   systemVideoMemory;              // size(in kb) of system memory the driver allocates at load time.
    NvU32   sharedSystemMemory;             // size(in kb) of shared system memory that driver is allowed to commit for surfaces across all allocations.

} NV_DRIVER_MEMORY_INFO;

#define NV_DRIVER_MEMORY_INFO_VER   MAKE_NVAPI_VERSION(NV_DRIVER_MEMORY_INFO,1)

NVAPI_INTERFACE NvAPI_GetDriverMemoryInfo(NvDisplayHandle hNvDisplay, NV_DRIVER_MEMORY_INFO *pMemoryInfo);



typedef struct
{
    NvU32   version;            //IN version info  
    NvU32   currentLevel;       //OUT current DVC level
    NvU32   minLevel;           //OUT min range level
    NvU32   maxLevel;           //OUT max range level
} NV_DISPLAY_DVC_INFO;

#define NV_DISPLAY_DVC_INFO_VER  MAKE_NVAPI_VERSION(NV_DISPLAY_DVC_INFO,1)

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GetDVCInfo
//
//   DESCRIPTION: Retrieves the Digital Vibrance Control(DVC) information of the selected display.
//
//  SUPPORTED OS: Windows XP and higher
//
//    PARAMETERS: hNvDisplay(IN) - NVIDIA Display selection. It can be NVAPI_DEFAULT_HANDLE or a handle enumerated from NvAPI_EnumNVidiaDisplayHandle().
//                outputId(IN)   - One of the selected outputId retrieved from NvAPI_GPU_GetActiveOutputs or NvAPI_GetView, to identify the target
//                                 output in case multiple targets are associated with the selected hNvDisplay.
//                                 Can be NULL to pick the display output id associated with hNvDisplay.
//                pDVCInfo(OUT)  - The returned DVC information.
//
// RETURN STATUS: NVAPI_OK: call successful.
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found.
//                NVAPI_EXPECTED_DISPLAY_HANDLE: hNvDisplay is not a valid display handle.
//                NVAPI_INVALID_ARGUMENT: pDVCInfo is NULL.
//                NVAPI_INCOMPATIBLE_STRUCT_VERSION - the version of the NV_DISPLAY_DVC_INFO struct is not supported
//                NVAPI_NOT_SUPPORTED - DVC feature is not supported on the selected GPU
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GetDVCInfo(NvDisplayHandle hNvDisplay, NvU32 outputId, NV_DISPLAY_DVC_INFO *pDVCInfo);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_SetDVCLevel
//
//   DESCRIPTION: Sets the DVC level for the selected display.
//
//  SUPPORTED OS: Windows XP and higher
//
//    PARAMETERS: hNvDisplay(IN) - NVIDIA Display selection. It can be NVAPI_DEFAULT_HANDLE or a handle enumerated from NvAPI_EnumNVidiaDisplayHandle().
//                outputId(IN)   - One of the selected outputId retrieved from NvAPI_GPU_GetActiveOutputs or NvAPI_GetView, to identify the target
//                                 output in case multiple targets are associated with the selected hNvDisplay.
//                                 Can be NULL to pick the display output id associated with hNvDisplay.
//                level(IN)      - The new level to apply. Value should be within the range of min and max.
//
// RETURN STATUS: NVAPI_OK: call successful.
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found.
//                NVAPI_EXPECTED_DISPLAY_HANDLE: hNvDisplay is not a valid display handle.
//                NVAPI_NOT_SUPPORTED - DVC is not supported on the selected GPU
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_SetDVCLevel(NvDisplayHandle hNvDisplay, NvU32 outputId, NvU32 level);


typedef struct
{
    NvU32   version;            //IN version info  
    NvS32   currentLevel;       //OUT current DVC level
    NvS32   minLevel;           //OUT min range level
    NvS32   maxLevel;           //OUT max range level
    NvS32   defaultLevel;       //OUT default DVC level
} NV_DISPLAY_DVC_INFO_EX;

#define NV_DISPLAY_DVC_INFO_EX_VER  MAKE_NVAPI_VERSION(NV_DISPLAY_DVC_INFO_EX,1)

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GetDVCInfoEx
//
//   DESCRIPTION: Retrieves the Digital Vibrance Control(DVC) information of the selected display.
//
//  SUPPORTED OS: Windows XP and higher
//
//    PARAMETERS: hNvDisplay(IN) - NVIDIA Display selection. It can be NVAPI_DEFAULT_HANDLE or a handle enumerated from NvAPI_EnumNVidiaDisplayHandle().
//                outputId(IN)   - One of the selected outputId retrieved from NvAPI_GPU_GetActiveOutputs or NvAPI_GetView, to identify the target
//                                 output in case multiple targets are associated with the selected hNvDisplay.
//                                 Can be NULL to pick the display output id associated with hNvDisplay.
//                pDVCInfo(OUT)  - The returned DVC information.
//
// RETURN STATUS: NVAPI_OK: call successful.
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found.
//                NVAPI_EXPECTED_DISPLAY_HANDLE: hNvDisplay is not a valid display handle.
//                NVAPI_INVALID_ARGUMENT: pDVCInfo is NULL.
//                NVAPI_INCOMPATIBLE_STRUCT_VERSION - the version of the NV_DISPLAY_DVC_INFO struct is not supported
//                NVAPI_NOT_SUPPORTED - DVC feature is not supported on the selected GPU
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GetDVCInfoEx(NvDisplayHandle hNvDisplay, NvU32 outputId, NV_DISPLAY_DVC_INFO_EX *pDVCInfo);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_SetDVCLevelEx
//
//   DESCRIPTION: Sets the DVC level for the selected display.
//
//  SUPPORTED OS: Windows XP and higher
//
//    PARAMETERS: hNvDisplay(IN) - NVIDIA Display selection. It can be NVAPI_DEFAULT_HANDLE or a handle enumerated from NvAPI_EnumNVidiaDisplayHandle().
//                outputId(IN)   - One of the selected outputId retrieved from NvAPI_GPU_GetActiveOutputs or NvAPI_GetView, to identify the target
//                                 output in case multiple targets are associated with the selected hNvDisplay.
//                                 Can be NULL to pick the display output id associated with hNvDisplay.
//                dvcInfo(IN)    - The new DVC to apply. This structure can be accessed by GetDVCInfo API.
//
// RETURN STATUS: NVAPI_OK: call successful.
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found.
//                NVAPI_EXPECTED_DISPLAY_HANDLE: hNvDisplay is not a valid display handle.
//                NVAPI_NOT_SUPPORTED - DVC is not supported on the selected GPU
//
///////////////////////////////////////////////////////////////////////////////

NVAPI_INTERFACE NvAPI_SetDVCLevelEx(NvDisplayHandle hNvDisplay, NvU32 outputId, NV_DISPLAY_DVC_INFO_EX *pDVCInfo);

typedef struct
{
    NvU32   version;            //IN version info
    NvU32   currentHueAngle;    //OUT current HUE Angle. Typically between 0 - 360 degrees
    NvU32   defaultHueAngle;    //OUT default HUE Angle
} NV_DISPLAY_HUE_INFO;

#define NV_DISPLAY_HUE_INFO_VER  MAKE_NVAPI_VERSION(NV_DISPLAY_HUE_INFO,1)

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GetHUEInfo
//
//   DESCRIPTION: Retrieves the HUE information of the selected display.
//
//  SUPPORTED OS: Windows XP and higher
//
//    PARAMETERS: hNvDisplay(IN) - NVIDIA Display selection. It can be NVAPI_DEFAULT_HANDLE or a handle enumerated from NvAPI_EnumNVidiaDisplayHandle().
//                outputId(IN)   - One of the selected outputId retrieved from NvAPI_GPU_GetActiveOutputs or NvAPI_GetView, to identify the target 
//                                 output in case multiple targets are associated with the selected hNvDisplay.
//                                 Can be NULL to pick the display output id associated with hNvDisplay.
//                pHUEInfo(OUT)  - The returned HUE information.
//
// RETURN STATUS: NVAPI_OK: call successful.
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVidia GPU driving a display was found.
//                NVAPI_EXPECTED_DISPLAY_HANDLE: hNvDisplay is not a valid display handle.
//                NVAPI_INVALID_ARGUMENT: pHUEInfo is NULL.
//                NVAPI_INCOMPATIBLE_STRUCT_VERSION - the version of the NV_DISPLAY_HUE_INFO struct is not supported
//                NVAPI_NOT_SUPPORTED - HUE feature is not supported on the selected GPU
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GetHUEInfo(NvDisplayHandle hNvDisplay, NvU32 outputId, NV_DISPLAY_HUE_INFO *pHUEInfo);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_SetHUEAngle
//
//   DESCRIPTION: Sets the HUE level for the selected display.
//
//  SUPPORTED OS: Windows XP and higher
//
//    PARAMETERS: hNvDisplay(IN) - NVIDIA Display selection. It can be NVAPI_DEFAULT_HANDLE or a handle enumerated from NvAPI_EnumNVidiaDisplayHandle().
//                outputId(IN)   - One of the selected outputId retrieved from NvAPI_GPU_GetActiveOutputs or NvAPI_GetView, to identify the target 
//                                 output in case multiple targets are associated with the selected hNvDisplay.
//                                 Can be NULL to pick the display output id associated with hNvDisplay.
//                level(IN)      - The new level to apply. Value should be within the range of min and max.
//
// RETURN STATUS: NVAPI_OK: call successful.
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVidia GPU driving a display was found.
//                NVAPI_EXPECTED_DISPLAY_HANDLE: hNvDisplay is not a valid display handle.
//                NVAPI_NOT_SUPPORTED - HUE feature is not supported on the selected GPU
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_SetHUEAngle(NvDisplayHandle hNvDisplay, NvU32 outputId, NvU32 hueAngle);

typedef struct
{
    NvU32   version;            //IN version info
    NvU32   currentLevel;       //OUT current Image Sharpening level
    NvU32   minLevel;           //OUT min range level
    NvU32   maxLevel;           //OUT max range level
} NV_DISPLAY_IMAGE_SHARPENING_INFO;

#define NV_DISPLAY_IMAGE_SHARPENING_INFO_VER  MAKE_NVAPI_VERSION(NV_DISPLAY_IMAGE_SHARPENING_INFO,1)

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GetImageSharpeningInfo
//
//   DESCRIPTION: Retrieves the Image Sharpening information of the selected display.
//
//  SUPPORTED OS: Windows XP and higher
//
//    PARAMETERS: hNvDisplay(IN) - NVIDIA Display selection. It can be NVAPI_DEFAULT_HANDLE or a handle enumerated from NvAPI_EnumNVidiaDisplayHandle().
//                outputId(IN)   - One of the selected outputId retrieved from NvAPI_GPU_GetActiveOutputs or NvAPI_GetView, to identify the target
//                                 output in case multiple targets are associated with the selected hNvDisplay.
//                                 Can be NULL to pick the display output id associated with hNvDisplay.
//                pImageSharpeningInfo(OUT)  - The returned Image Sharpening information.
//
// RETURN STATUS: NVAPI_OK: call successful.
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found.
//                NVAPI_EXPECTED_DISPLAY_HANDLE: hNvDisplay is not a valid display handle.
//                NVAPI_INVALID_ARGUMENT: pImageSharpeningInfo is NULL.
//                NVAPI_INCOMPATIBLE_STRUCT_VERSION - the version of the NV_DISPLAY_IMAGE_SHARPENING_INFO struct is not supported
//                NVAPI_NOT_SUPPORTED - Image Sharpening is not supported on the selected GPU
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GetImageSharpeningInfo(NvDisplayHandle hNvDisplay, NvU32 outputId, NV_DISPLAY_IMAGE_SHARPENING_INFO *pImageSharpeningInfo);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_SetImageSharpeningLevel
//
//   DESCRIPTION: Sets the Image Sharpening level for the selected display.
//
//  SUPPORTED OS: Windows XP and higher
//
//    PARAMETERS: hNvDisplay(IN) - NVIDIA Display selection. It can be NVAPI_DEFAULT_HANDLE or a handle enumerated from NvAPI_EnumNVidiaDisplayHandle().
//                outputId(IN)   - One of the selected outputId retrieved from NvAPI_GPU_GetActiveOutputs or NvAPI_GetView, to identify the target
//                                 output in case multiple targets are associated with the selected hNvDisplay.
//                                 Can be NULL to pick the display output id associated with hNvDisplay.
//                level(IN)      - The new level to apply. Value should be within the range of min and max.
//
// RETURN STATUS: NVAPI_OK: call successful.
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found.
//                NVAPI_EXPECTED_DISPLAY_HANDLE: hNvDisplay is not a valid display handle.
//                NVAPI_NOT_SUPPORTED - Image Sharpening is not supported on the selected GPU
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_SetImageSharpeningLevel(NvDisplayHandle hNvDisplay, NvU32 outputId, NvU32 level);


//-----------------------------------------------------------------------------
// DirectX APIs
//-----------------------------------------------------------------------------
typedef struct
{
    NvU32 version;                    // Structure version
    NvU32 maxNumAFRGroups;            // [OUT] The maximum possible value of numAFRGroups
    NvU32 numAFRGroups;               // [OUT] The number of AFR groups enabled in the system
    NvU32 currentAFRIndex;            // [OUT] The AFR group index for the frame currently being rendered
    NvU32 nextFrameAFRIndex;          // [OUT] What the AFR group index will be for the next frame (i.e. after calling Present)
    NvU32 previousFrameAFRIndex;      // [OUT] The AFR group index that was used for the previous frame (~0 if more than one frame has not been rendered yet)
    NvU32 bIsCurAFRGroupNew;          // [OUT] boolean: Is this frame the first time running on the current AFR group
} NV_GET_CURRENT_SLI_STATE;
#define NV_GET_CURRENT_SLI_STATE_VER  MAKE_NVAPI_VERSION(NV_GET_CURRENT_SLI_STATE,1)

#if defined(_D3D9_H_) || defined(__d3d10_h__)

NV_DECLARE_HANDLE(NVDX_ObjectHandle);  // DX Objects
static const NVDX_ObjectHandle NVDX_OBJECT_NONE = 0;

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_D3D_GetCurrentSLIState
//
// DESCRIPTION:     Returns the current SLI state for this device.  The struct
//                  contains the number of AFR groups, the current AFR group index
//                  and what the AFR group index will be for the next frame.
//                  pDevice can be either a IDirect3DDevice9 or ID3D10Device ptr.
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_ERROR - error occurred
//
///////////////////////////////////////////////////////////////////////////////

NVAPI_INTERFACE NvAPI_D3D_GetCurrentSLIState(IUnknown *pDevice, NV_GET_CURRENT_SLI_STATE *pSliState);

#endif //if defined(_D3D9_H_) || defined(__d3d10_h__)


#if defined(_D3D9_H_) || defined(__d3d10_h__)

///////////////////////////////////////////////////////////////////////////////
// NVAPI Query Types
///////////////////////////////////////////////////////////////////////////////

typedef enum _NVAPI_D3D_QUERY_TYPE
{
    NVAPI_D3D_QUERY_TYPE_RESERVED0       = 0
}NVAPI_D3D_QUERY_TYPE;

typedef enum _NVAPI_D3D_QUERY_GETDATA_FLAGS
{
    NVAPI_D3D_QUERY_GETDATA_FLUSH      = 0,
    NVAPI_D3D_QUERY_GETDATA_DONOTFLUSH = 1
}NVAPI_D3D_QUERY_GETDATA_FLAGS;

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_D3D_CreateQuery
//
// DESCRIPTION:     Creates NVIDIA-specific D3D Query Objects.
//
// INPUT:           pDevice             The device to create this query object on
//
//                  type                Type of the query to be created (see NVAPI_D3D_QUERY_TYPE)
//
// OUTPUT:          pHandle             The handle to the Query object
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_INVALID_POINTER - pDevice was NULL
//                  NVAPI_INVALID_ARGUMENT - one of the arguments was not valid
//                  NVAPI_OUT_OF_MEMORY - unable to allocate sufficient memory to complete the call
//                  NVAPI_ERROR - error occurred
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_D3D_CreateQuery(IUnknown *pDevice, NVAPI_D3D_QUERY_TYPE type, NVDX_ObjectHandle *pHandle);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_D3D_DestroyQuery
//
// DESCRIPTION:     Destroys NVIDIA-specific D3D Query Objects
//
// INPUT:           queryHandle         The handle to the Query object
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_INVALID_HANDLE - the Query object handle is invalid
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_D3D_DestroyQuery(NVDX_ObjectHandle queryHandle);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_D3D_Query_Begin
//
// DESCRIPTION:     Mark the beginning of a series of commands
//
// INPUT:           queryHandle         The handle to the Query object
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_INVALID_HANDLE - the Query object handle is invalid
//                  NVAPI_INVALID_CALL - the call could not be completed
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_D3D_Query_Begin(NVDX_ObjectHandle queryHandle);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_D3D_Query_End
//
// DESCRIPTION:     Mark the end of a series of commands
//
// INPUT:           queryHandle         The handle to the Query object
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_INVALID_HANDLE - the Query object handle is invalid
//                  NVAPI_INVALID_CALL - the call could not be completed
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_D3D_Query_End(NVDX_ObjectHandle queryHandle);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_D3D_Query_GetData
//
// DESCRIPTION:     Polls a queried resource to get the query state or a query result.
//                  Allows getting data from the GPU or the driver asynchronously.
//
// INPUT:           queryHandle         The handle to the Query object.
//                  dwSize              Size of the data to retrieve or 0.
//                                      This value can be obtained with NvAPI_D3D_Query_GetDataSize.
//                  dwGetDataFlags      Optional flags. Can be 0 or any combination of NVAPI_D3D_QUERY_GETDATA_FLAGS
//
// OUTPUT:          pData               Address of memory that will receive the data. 
//                                      If NULL, GetData will be used only to check status
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_INVALID_HANDLE - the Query object handle is invalid
//                  NVAPI_INVALID_CALL - the call could not be completed
//                  NVAPI_DATA_NOT_FOUND - the Queried data was not yet available
//                  NVAPI_ERROR - error occurred
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_D3D_Query_GetData(NVDX_ObjectHandle queryHandle, void* pData, UINT dwSize, UINT dwGetDataFlags);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_D3D_Query_GetDataSize
//
// DESCRIPTION:     Get the size of the data (in bytes) that is output when calling
//                  NvAPI_D3D_Query_GetData
//
// INPUT:           queryHandle         The handle to the Query object
//
// OUTPUT:          pDwSize             The requested size value
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_INVALID_HANDLE - the Query object handle is invalid
//                  NVAPI_INVALID_POINTER - pDwSize is NULL
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_D3D_Query_GetDataSize(NVDX_ObjectHandle queryHandle, UINT *pDwSize);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_D3D_Query_GetType
//
// DESCRIPTION:     Get the NVAPI_D3D_QUERY_TYPE of the given query object
//
// INPUT:           queryHandle         The handle to the Query object
//
// OUTPUT:          pType               The requested NVAPI_D3D_QUERY_TYPE value
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_INVALID_HANDLE - the Query object handle is invalid
//                  NVAPI_INVALID_POINTER - pType is NULL
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_D3D_Query_GetType(NVDX_ObjectHandle queryHandle, NVAPI_D3D_QUERY_TYPE *pType);


///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_D3D_RegisterApp
//
// DESCRIPTION:     Allow known dx10/dx9 app to register itself to enable specific driver modes
//
//  SUPPORTED OS: Windows Vista and higher
// INPUT: userAppId  - a predetermined number (from nvidia) so that the driver would
//                     turn on app specific features.
//                     If the code is not recognized, it would return NVAPI_NOT_SUPPORTED.
//                     Otherwise, it would return NVAPI_OK.
//
// RETURN STATUS: 
//  NVAPI_ERROR             - Registration failed
//  NVAPI_OK                - the app is supported
//  NVAPI_INVALID_ARGUMENT  - if bad parameters are given
//
// NOTE: 
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_D3D_RegisterApp(IUnknown *pDev, NvU32 userAppId);

#endif // defined(_D3D9_H_) || defined(__d3d10_h__)

//-----------------------------------------------------------------------------
// Direct3D9 APIs
//-----------------------------------------------------------------------------

#if defined(_D3D9_H_) && defined(__cplusplus)

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_D3D9_GetSurfaceHandle
//
//   DESCRIPTION: Get the handle of a given surface. This handle uniquely
//                identify the surface through all NvAPI entries.
//
//  SUPPORTED OS: Windows XP and higher
//
//         INPUT: pSurface - Surface to be identified
//
//        OUTPUT: pHandle - Will be filled by the return handle
//
//          DEMO: PrimTexture
//
// RETURN STATUS: an int which could be an NvAPI status or DX HRESULT code
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_D3D9_GetSurfaceHandle(IDirect3DSurface9 *pSurface,
                                        NVDX_ObjectHandle *pHandle);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_D3D9_GetSurfaceHandle
//
//   DESCRIPTION: Get the surface handles for the YUY2 overlay surfaces created.
//
//  SUPPORTED OS: Windows XP and higher
//
//         INPUT: pDev - Device whose YUY2 surface handles need to be retrieved.
//
//        OUTPUT: NVDX_ObjectHandle - Array of surface handles that will be filled by
//                the driver. The array has to initialzed before it is passed on to
//                the funtion. The handles are populated in the order in which the
//                they are created. e.g The handle of first surface created will be
//                store in pHandle[0], second surface will be stored in pHandle[1] and so on.
//
//          DEMO: PrimTexture
//
// RETURN STATUS: an int which could be an NvAPI status or DX HRESULT code
//
///////////////////////////////////////////////////////////////////////////////
#define MAX_OVLY_SURFS     6

typedef struct _NV_OVLY_SURFS_INFO
{
    NVDX_ObjectHandle  handle[MAX_OVLY_SURFS];
    unsigned int       numSurfs;
}NV_OVLY_SURFS_INFO;

NVAPI_INTERFACE NvAPI_D3D9_GetOverlaySurfaceHandles(IDirect3DDevice9 *pDev, NV_OVLY_SURFS_INFO *pInfo);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_D3D9_GetTextureHandle
//
//   DESCRIPTION: Get the handle of a given DX9 texture.
//
//  SUPPORTED OS: Windows XP and higher
//
//         INPUT: pTexture - Surface to be identified
//
//        OUTPUT: pHandle - Will be filled by the return handle
//
// RETURN STATUS: an int which could be an NvAPI status or DX HRESULT code
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_D3D9_GetTextureHandle(IDirect3DTexture9 *pTexture,
                                        NVDX_ObjectHandle *pHandle);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_D3D9_GpuSyncGetHandleSize
//
//   DESCRIPTION: returns size of the init and copy sync handles. These handles are
//                then allocated and initialized to zero by the application
//
//  SUPPORTED OS: Windows XP and higher
//
//        OUTPUT: pInitHandleSize - size of GpuSync init handle
//                pMapHandleSize - size of GpuSync copy handle
//
// RETURN STATUS: an int which could be an NvAPI status or DX HRESULT code
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_D3D9_GpuSyncGetHandleSize(IDirect3DDevice9 *pDev,
                                            unsigned int *pInitHandleSize,
                                            unsigned int *pMapHandleSize);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_D3D9_GpuSyncInit
//
//   DESCRIPTION: setup sync functionality
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS: an int which could be an NvAPI status or DX HRESULT code
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_D3D9_GpuSyncInit(IDirect3DDevice9 *pDev,
                                            void * syncInitData);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_D3D9_GpuSyncEnd
//
//   DESCRIPTION: tear down sync structures
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS: an int which could be an NvAPI status or DX HRESULT code
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_D3D9_GpuSyncEnd(IDirect3DDevice9 *pDev,
                                            void * syncData);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_D3D9_GpuSyncMapTexBuffer
//
//   DESCRIPTION: map a texture to receive ogl data
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS: an int which could be an NvAPI status or DX HRESULT code
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_D3D9_GpuSyncMapTexBuffer(IDirect3DDevice9 *pDev,
                                            IDirect3DTexture9 *pTexture,
                                            void * syncData);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_D3D9_GpuSyncMapSurfaceBuffer
//
//   DESCRIPTION: map a texture to receive ogl data
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS: an int which could be an NvAPI status or DX HRESULT code
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_D3D9_GpuSyncMapSurfaceBuffer(IDirect3DDevice9 *pDev,
                                                   IDirect3DSurface9 *pSurface,
                                                   void * syncData);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_D3D9_GpuSyncMapVertexBuffer
//
//   DESCRIPTION: map a vertex buffer to recieve ogl data
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS: an int which could be an NvAPI status or DX HRESULT code
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_D3D9_GpuSyncMapVertexBuffer(IDirect3DDevice9 *pDev,
                                            IDirect3DVertexBuffer9 *pVertexBuffer,
                                            void * syncData);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_D3D9_GpuSyncMapIndexBuffer
//
//   DESCRIPTION: map a index buffer to recieve ogl data
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS: an int which could be an NvAPI status or DX HRESULT code
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_D3D9_GpuSyncMapIndexBuffer(IDirect3DDevice9 *pDev,
                                            IDirect3DIndexBuffer9 *pIndexBuffer,
                                            void * syncData);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_D3D9_SetPitchLinearSurfaceCreation
//
//   DESCRIPTION: Force the next CreateTexture/CreateRenderTarget call to use pitch surface
//                Applied only once for the next surface. Set Signature==0 to make sure this disabled.
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS: an int which could be an NvAPI status or DX HRESULT code
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_D3D9_SetPitchSurfaceCreation(IDirect3DDevice9 *pDev,
                                                   NvU32 Signature);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_D3D9_GpuSyncAcquire
//
//   DESCRIPTION: semaphore acquire for synchronization control of mapped buffer
//
//  SUPPORTED OS: Windows XP and higher
//
//         INPUT: accessMode - acquire mapped buffer read/write access
//
// RETURN STATUS: an int which could be an NvAPI status or DX HRESULT code
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_D3D9_GpuSyncAcquire(IDirect3DDevice9 *pDev,
                                            void * syncData);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_D3D9_GpuSyncRelease
//
//   DESCRIPTION: semaphore release for synchronization control of mapped buffer
//
//  SUPPORTED OS: Windows XP and higher
//
//         INPUT: accessMode - release mapped buffer read/write access
//
// RETURN STATUS: an int which could be an NvAPI status or DX HRESULT code
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_D3D9_GpuSyncRelease(IDirect3DDevice9 *pDev,
                                            void * syncData);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_D3D9_GetCurrentRenderTargetHandle
//
//   DESCRIPTION: Get the handle of current RT/ZB.
//
//  SUPPORTED OS: Windows XP and higher
//
//         INPUT: pDev - Device whose current ZB to be identified
//
//        OUTPUT: pHandle - Will be filled by the return handle
//
// RETURN STATUS: an int which could be an NvAPI status or DX HRESULT code
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_D3D9_GetCurrentRenderTargetHandle(IDirect3DDevice9 *pDev,
                                            NVDX_ObjectHandle *pHandle);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_D3D9_GetCurrentZBufferHandle
//
//   DESCRIPTION:
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS: an int which could be an NvAPI status or DX HRESULT code
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_D3D9_GetCurrentZBufferHandle(IDirect3DDevice9 *pDev,
                                               NVDX_ObjectHandle *pHandle);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_D3D9_GetIndexBufferHandle
//
//   DESCRIPTION: Get the handle of a given DX9 index buffer.
//
//  SUPPORTED OS: Windows XP and higher
//
//         INPUT: pIndexBuffer - Index Buffer to be identified
//
//        OUTPUT: pHandle - Will be filled by the return handle
//
// RETURN STATUS: an int which could be an NvAPI status or DX HRESULT code
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_D3D9_GetIndexBufferHandle(IDirect3DIndexBuffer9 *pIndexBuffer,
                                        NVDX_ObjectHandle *pHandle);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_D3D9_GetVertexBufferHandle
//
//   DESCRIPTION: Get the handle of a given DX9 vertex buffer.
//
//  SUPPORTED OS: Windows XP and higher
//
//         INPUT: pVertexBuffer - Vertex Buffer to be identified
//
//        OUTPUT: pHandle - Will be filled by the return handle
//
// RETURN STATUS: an int which could be an NvAPI status or DX HRESULT code
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_D3D9_GetVertexBufferHandle(IDirect3DVertexBuffer9 *pVertexBuffer,
                                        NVDX_ObjectHandle *pHandle);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_D3D9_CreateTexture
//
//   DESCRIPTION: Create a texture with special properties. NOTE: The texture
//                is always created in "POOL_DEFAULT", not managed.
//
//  SUPPORTED OS: Windows XP and higher
//
//         INPUT:  pDev       The device to get primary surface from
//                 Width,Height,Levels,Format
//                            Same as IDirect3D9Device::CreateTexture
//                 Flags      The flags for special texture creation
//
//                 FORCEVIDMEM  This will force the texture into video memory,
//                              if that can not be done, the texture creation will fail.
//                 FORCELINEAR  Ensure the texture is stored in pitched linear layout.
//                 NOTMOVABLE   Indicates the texture should not be moved once allocated.
//                              This is usually used to accomodate 3rd party DMA engine.
//                              NOTE: This is only a hint, OS may still move the texture
//                                    under memory constrained circumstances.
//
//        OUTPUT:  ppTexture  Fill with the texture created
//                 pHandle    If non-NULL, fill with the NVDX handle of the created texture
//
//          DEMO:  TextureLock
//
///////////////////////////////////////////////////////////////////////////////

#define NV_SURFACEFLAG_FORCEVIDMEM      0x00000001
#define NV_SURFACEFLAG_FORCELINEAR      0x00000010
#define NV_SURFACEFLAG_NOTMOVABLE       0x00000100

NVAPI_INTERFACE NvAPI_D3D9_CreateTexture(IDirect3DDevice9 *pDev,
                                         NvU32 Width, NvU32 Height, NvU32 Level,
                                         NvU32 Flags, D3DFORMAT Format,
                                         IDirect3DTexture9 **ppTexture,
                                         NVDX_ObjectHandle *pHandle = 0);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_D3D9_AliasPrimaryAsTexture
//
//   DESCRIPTION: Create an texture that is an alias of current device's primary surface
//
//  SUPPORTED OS: Windows XP and higher
//
//         INPUT:  pDev       The device to get primary surface from
//                 dwIndex    The index to the primary flipchain of device (usually 0)
//
//        OUTPUT:  ppTexture  Fill with the texture created
//                 pHandle    If non-NULL, fill with the NVDX handle of the created texture
//
//          DEMO:  PrimTexture
//
// RETURN STATUS: an int which could be an NvAPI status or DX HRESULT code
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_D3D9_AliasPrimaryAsTexture(IDirect3DDevice9 *pDev,
                                            NvU32 dwIndex,
                                            IDirect3DTexture9 **ppTexture,
                                            NVDX_ObjectHandle *pHandle = 0);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_D3D9_PresentSurfaceToDesktop
//
//   DESCRIPTION: Present a given surface to the desktop. This interface can be
//                used to start a fullscreen flipping mode even within windowed D3D app.
//
//  SUPPORTED OS: Windows XP and higher
//
//         INPUT: pDev              The device (display) to present to surfaceHandle   The surface
//                                  handle obtained from NVD3D9_GetSurfaceHandle
//                                  NOTE: NVDX_OBJECT_NONE means restore
//                dwFlipFlags       Flags to indicate SYNC mode (other bits reserved and must be 0)
//                dwExcludeDevices  This is a bitmask (usually 0) to indicate which device
//                                  will be EXCLUDED from this present. This is only
//                                  effective when used in a clone mode configuration where
//                                  the application wants one monitor to show the specially
//                                  rendered screen and the other normal desktop.
//
//          NOTE: It is applications responsibility to determine which devices are
//                available on the current clone config, through nvcpl interfaces.
//
//          DEMO: PrimTexture
//
// RETURN STATUS: an int which could be an NvAPI status or DX HRESULT code
//
///////////////////////////////////////////////////////////////////////////////

// Parameter used for dwFlipFlags (All other bits reserved)
#define NV_FLIPFLAG_VSYNC               0x00000001  // SYNCMODE         (bit 0:1) - 0:NOSYNC, 1:VSYNC, 2:HSYNC
#define NV_FLIPFLAG_HSYNC               0x00000002
#define NV_FLIPFLAG_TRIPLEBUFFERING     0x00000004  // TRIPLEBUFFERING  (bit 2)   - 0: DoubleBuffer, 1:TripleBuffer or more

NVAPI_INTERFACE NvAPI_D3D9_PresentSurfaceToDesktop(IDirect3DDevice9 *pDev,
                                               NVDX_ObjectHandle surfaceHandle,
                                               NvU32 dwFlipFlags,
                                               NvU32 dwExcludeDevices = 0);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION_NAME: NvAPI_D3D9_CreateVideoBegin
//
//   DESCRIPTION: Signals the driver that the application next will create a set of
//                D3DFMT_X8R8G8B8 render targets for overlay use. The call will fail
//                if the driver/hardware doesn't support this mode, in which case the
//                application should fall back to the traditional overlay (with driver
//                internal overlay buffers). If this call returns successfully, then
//                before the driver sees _CreateVideoEnd, all D3DFMT_A8R8G8B8 render
//                targets will be allocated as overlay surfaces.
//                See _CreateVideo, NV_CVFLAG_EXTERNAL_OVERLAY flag for more details.
//                This interface is only available on Windows Vista.
//
//  SUPPORTED OS: Windows Vista and higher
//
//         INPUT: pDev              The device (display) to present to
//
// RETURN STATUS: an int which could be an NvAPI status or DX HRESULT code
//
///////////////////////////////////////////////////////////////////////////////

NVAPI_INTERFACE NvAPI_D3D9_CreateVideoBegin(IDirect3DDevice9 *pDev);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION_NAME: NvAPI_D3D9_CreateVideoEnd
//
//   DESCRIPTION: Signals the driver that the application has finished creating
//                D3DFMT_X8R8G8B8 render targets for overlay.
//                See _CreateVideo, NV_CVFLAG_EXTERNAL_OVERLAY flag for more details.
//                This interface is only available on Windows Vista.
//
//  SUPPORTED OS: Windows Vista and higher
//
//         INPUT: pDev              The device (display) to present to
//
// RETURN STATUS: an int which could be an NvAPI status or DX HRESULT code
//
///////////////////////////////////////////////////////////////////////////////

NVAPI_INTERFACE NvAPI_D3D9_CreateVideoEnd(IDirect3DDevice9 *pDev);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION_NAME: NvAPI_D3D9_CreateVideo
//
//   DESCRIPTION: Allocates and initializes video resources for playback within a D3D9 context
//
//  SUPPORTED OS: Windows XP and higher
//
//         INPUT: pDev              The device (display) to present to
//                dwVersion         Version of the CreateVideoParams structure
//                dwCVFlags         Initialization flags (see defines for description)
//                dwFlipQueueHint   If overlay is allocated, indication of the desired number of flips
//                                  to be queued (minimum of 1).  A larger number is better for performance,
//                                  but also consumes more frame buffer resources.  This number should be
//                                  set according to the number of fields/frames that can be decoded in
//                                  advance of presentation.  Eg., if the decoder cannot decode ahead and
//                                  must display immediately after decoding a frame, then set this to 1.
//                                  If the decoder is decoding field content one frame at a time, then it
//                                  is decoding one field ahead and should set this to 2.  If the decoder
//                                  can decode n flips ahead, then set this to n+1.  If the GPU has
//                                  constrained frame buffer resources, then set this number lower, and also
//                                  reduce the decode-ahead pipeline resources accordingly.
//                dwMaxSrcWidth     Maximum video source width
//                dwMaxSrcHeight    Maximum video source height
//                dwNumOvlSurfs     (VER2 only) Number of surfaces that the application intends to use for
//                                  overlay (also refer to the NV_CVFLAG_EXTERNAL_OVERLAY flag)
//                hOvlSurfs         (VER2 only) Handles to the surfaces that the application intends to use
//                                  for overlay (also refer to the NV_CVFLAG_EXTERNAL_OVERLAY flag)
//
// RETURN STATUS: an int which could be an NvAPI status or DX HRESULT code
//
///////////////////////////////////////////////////////////////////////////////

#define NV_CVFLAG_OVERLAY                   0x00000001      // Overlay will be used
#define NV_CVFLAG_OVERLAY_REINIT            0x00000002      // Change source size or flip queue hint once overlay
                                                            // resources have already been allocated.  Be wary
                                                            // of upward allocating new resources, if there is
                                                            // a failure, be prepared to deallocate everything
                                                            // and start from scratch.
#define NV_CVFLAG_EXTERNAL_OVERLAY          0x00000004      // Use overlay surfaces allocated by the application. This flag
                                                            // is supported only in NV_DX_CREATE_VIDEO_PARAMS_VER2 and above
                                                            // on Windows Vista.
                                                            //
                                                            // The application sets this flag to indicate that the driver should
                                                            // not allocate any internal overlay buffers, and the first
                                                            // dwNumOvlSurfs surfaces in pCVParams->hOvlSurfs[] will be used for
                                                            // overlay scan-out directly. The driver will validate these surfaces
                                                            // against the following conditions, and fail the call otherwise:
                                                            //
                                                            // 1. No less than NV_CV_MIN_OVERLAY_SURFACE_NUMBER and no more than
                                                            //    NV_CV_MAX_OVERLAY_SURFACE_NUMBER should be passed in
                                                            // 2. Only surface handles of D3DFMT_A8R8G8B8 render targets created
                                                            //    inside a pair of _CreateVideoBegin ()/_CreateVideoEnd() should be
                                                            //    used
                                                            // 3. The size of the surfaces must be identical to the target display
                                                            //
                                                            // If all of the above condictions are met for each of the surfaces,
                                                            // then the driver will not allocate any internal overlay buffers. Upon
                                                            // _PresentVideo(), the source surface will be used for overlay scan-
                                                            // out directly without the driver copying the content to an overlay
                                                            // buffer first.
#define NV_CVFLAG_2ND_FORCE_DISABLE         0x00010000      // Forces full screen video on secondary device to be
                                                            // disabled in multihead modes, regardless of user setting
#define NV_CVFLAG_2ND_FORCE_ENABLE          0x00020000      // Forces full screen video on secondary device to be
                                                            // enabled on the default device in multihead modes,
                                                            // regardless of user setting
#define NV_CVFLAG_2ND_FORCE_ENABLE1         0x00040000      // Forces full screen video on secondary device to be
                                                            // enabled on the primary device in clone mode,
                                                            // regardless of user setting
#define NV_CVFLAG_2ND_COMMANDEER            0x00100000      // If another application owns the secondary full screen
                                                            // device, forcibly take possession of it.
#define NV_CVFLAG_SECONDARY_DISPLAY         0x01000000      // Is the target display secondary display when in Dual-view mode

typedef struct
{
    NvU32 version;
    NvU32 cvFlags;
    NvU32 flipQueueHint;
    NvU32 maxSrcWidth;
    NvU32 maxSrcHeight;
} NV_DX_CREATE_VIDEO_PARAMS1;

#define NV_CV_MIN_OVERLAY_SURFACE_NUMBER    2
#define NV_CV_MAX_OVERLAY_SURFACE_NUMBER    6
typedef struct
{
    NvU32 version;
    NvU32 cvFlags;
    NvU32 flipQueueHint;
    NvU32 maxSrcWidth;
    NvU32 maxSrcHeight;
    NvU32 dwNumOvlSurfs;
    NVDX_ObjectHandle hOvlSurfs[NV_CV_MAX_OVERLAY_SURFACE_NUMBER];
} NV_DX_CREATE_VIDEO_PARAMS2;

typedef NV_DX_CREATE_VIDEO_PARAMS2  NV_DX_CREATE_VIDEO_PARAMS;

#define NV_DX_CREATE_VIDEO_PARAMS_VER1  MAKE_NVAPI_VERSION(NV_DX_CREATE_VIDEO_PARAMS1,1)

#define NV_DX_CREATE_VIDEO_PARAMS_VER2  MAKE_NVAPI_VERSION(NV_DX_CREATE_VIDEO_PARAMS2,2)

#define NV_DX_CREATE_VIDEO_PARAMS_VER   NV_DX_CREATE_VIDEO_PARAMS_VER2

NVAPI_INTERFACE NvAPI_D3D9_CreateVideo(IDirect3DDevice9 *pDev,
                                       NV_DX_CREATE_VIDEO_PARAMS *pCVParams);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION_NAME: NvAPI_D3D9_FreeVideo
//
//   DESCRIPTION: Releases all video resources
//
//  SUPPORTED OS: Windows XP and higher
//
//         INPUT: pDev              The device (display) to present to
//
// RETURN STATUS: an int which could be an NvAPI status or DX HRESULT code
//
///////////////////////////////////////////////////////////////////////////////

NVAPI_INTERFACE NvAPI_D3D9_FreeVideo(IDirect3DDevice9 *pDev);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_D3D9_PresentVideo
//
//   DESCRIPTION: Signals a final and complete frame ready for presentation.
//                Can optionally render to the overlay, but should be called
//                regardless of whether any actual rendering occurs.  If the
//                user has enabled full screen video in a multihead mode,
//                this frame will also be rendered on the secondary device.
//
//  SUPPORTED OS: Windows XP and higher
//
//          NOTE: Use NV_DX_PRESENT_VIDEO_PARAMS_VER in the "version" method for both 32 bit and 64 bit
//                callers.  For older drivers that do not support 64 bit callers, 32 bit callers
//                should use the evaluation of MAKE_NVAPI_VERSION(NV_DX_PRESENT_VIDEO_PARAMS1,1)
//                in that field.
//
//         INPUT: pDev              The device (display) to present to
//                dwVersion         Version of the PresentVideoParams structure
//                surfaceHandle     The surface handle obtained from NvAPI_D3D9_GetSurfaceHandle
//                                  or NvAPI_D3D9_GetCurrentRenderTargetHandle
//                dwPVFlags         Presentation flags (see defines for description)
//                dwColourKey       Colour key to use if NV_PVFLAG_DST_KEY is set
//                qwTimeStamp*      If NV_PVFLAG_USE_STAMP is set, time in ns when the frame is to be presented
//                                  If NV_PVFLAG_SET_STAMP is set, set the current time to this, and present on next vblank
//                dwFlipRate        Set to the current flip rate
//                                  Set to zero if the frame to be presented is a still frame
//                srcUnclipped      Unclipped source rectangle of the entire frame of data
//                srcClipped        Cropped source rectangle.  It is the caller's responsibility to crop
//                                  the source if the desktop crops the destination.
//                dst               Destination rectangle (in desktop coordinates) of the overlay.  It is the
//                                  caller's responsibility to crop the destination against the desktop.
//
// RETURN STATUS: an int which could be an NvAPI status or DX HRESULT code.
//
//                NVAPI_DEVICE_BUSY - This return value indicates the flip request was dropped
//                because the hardware flip queue was too deep, thus no more present requests can be
//                accepted at the moment.
//
//                When external overlay is used (for "external overlay" please refer to NvAPI_D3D9_CreateVideoBegin,
//                NvAPI_D3D9_CreateVideoEnd, and VER2-only descriptions of NvAPI_D3D9_CreateVideo),
//                a failure in _PresentVideo means the current front (on-screen) buffer will remain the
//                unchanged. In this case the application should be careful not to render to the current
//                front buffer, as it will cause video tearing. The application could re-try presenting
//                the same frame during the next vsync, or continue with rendering the next frame to a
//                back buffer.
//
///////////////////////////////////////////////////////////////////////////////

// PresentVideo flags
#define NV_PVFLAG_ODD           0x00000001      // Field is odd
#define NV_PVFLAG_EVEN          0x00000002      // Field is even
#define NV_PVFLAG_PROTECTED     0x00000004      // Indicates that this frame is protected and guarantees full
                                                // screen video will not display this frame on any secondary device.
                                                // Conversely, not setting this indicates an unprotected frame
#define NV_PVFLAG_PROGRESSIVE   0x00000008      // Indicates progressive frame.  If the odd or even flags are set
                                                // in conjunction with this, it indicates the original field that
                                                // generated this deinterlaced frame, and attempts to synchronize
                                                // this presentation to the corresponding display field of an
                                                // interlaced display
#define NV_PVFLAG_SHOW          0x00000010      // Show the overlay
                                                // If the app is minimized or obscured, continue to call NvAPI_D3D9_PresentVideo
                                                // for every complete frame without this flag set.
                                                // If enabled, unprotected video will continue to play full screen
                                                // on the secondary device, using the pixel aspect cached from
                                                // the last time a frame was shown.  To change the pixel aspect while hidden,
                                                // the caller must "show" a frame at least once with a new clipped source and
                                                // destination rectangle.  This shown frame can be rendered invisible with
                                                // appropriate selection of colour key.
#define NV_PVFLAG_FAST_MOVE     0x00000020      // Move overlay position without waiting for vblank.
                                                // The only parameters used are dwDstX, dwDstY, and NV_PVFLAG_SHOW.
#define NV_PVFLAG_WAIT          0x00000040      // If set, blocking flip, wait until flip queue can accept another flip.
                                                // A non-blocking flip will return an error if flip cannot be queued yet.
#define NV_PVFLAG_REPEAT        0x00000080      // Video data is completely unchanged from the previous flip (used for telecine)
#define NV_PVFLAG_DST_KEY       0x00000100      // Use destination colour key.
#define NV_PVFLAG_FULLSCREEN    0x00000200      // Indicates that the overlay is playing fullscreen on the desktop.
                                                // This bit is used to automatically overscan the image on TV's.
#define NV_PVFLAG_SET_STAMP     0x00001000      // Set the current time.
#define NV_PVFLAG_USE_STAMP     0x00002000      // If set, use timestamps.
                                                // If not set, flip on the next vblank.

typedef struct
{
    NvU32 version;
    NVDX_ObjectHandle surfaceHandle;
    NvU32 pvFlags;
    NvU32 colourKey;
    NvU32 timeStampLow;
    NvU32 timeStampHigh;
    NvU32 flipRate;
    NvSBox srcUnclipped;
    NvSBox srcClipped;
    NvSBox dst;
} NV_DX_PRESENT_VIDEO_PARAMS1;

typedef NV_DX_PRESENT_VIDEO_PARAMS1 NV_DX_PRESENT_VIDEO_PARAMS;

#ifdef _WIN32
#define NV_DX_PRESENT_VIDEO_PARAMS_VER1  MAKE_NVAPI_VERSION(NV_DX_PRESENT_VIDEO_PARAMS1,1)
#endif //_WIN32

#define NV_DX_PRESENT_VIDEO_PARAMS_VER  MAKE_NVAPI_VERSION(NV_DX_PRESENT_VIDEO_PARAMS1,2)

NVAPI_INTERFACE NvAPI_D3D9_PresentVideo(IDirect3DDevice9 *pDev,
                                        NV_DX_PRESENT_VIDEO_PARAMS *pPVParams);

///////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME:   NvAPI_D3D9_SetGamutData
//
// DESCRIPTION:     This API sets the Gamut Boundary Description (GBD) data.
//
//  SUPPORTED OS: Windows XP and higher
//
// PARAMETERS:      pDev(IN)       - The device (display) to present to.
//                  outputId(IN)   - The display output id, which can be found using NvAPI_GetAssociatedDisplayOutputId() 
//                                   for an intended display.
//                  format(IN)     - The format of GBD data structure.
//                  pGamutData(IN) - The GBD data.
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_ERROR - miscellaneous error occurred
//                  NVAPI_INVALID_ARGUMENT: Invalid input parameter.
//
///////////////////////////////////////////////////////////////////////////////

typedef enum _NV_GAMUT_FORMAT
{
    NV_GAMUT_FORMAT_VERTICES   = 0,
    NV_GAMUT_FORMAT_RANGE,
} NV_GAMUT_FORMAT;

typedef struct _NV_GAMUT_METADATA_RANGE
{
    // Header
    NvU32    GBD_Color_Space:3;
    NvU32    GBD_Color_Precision:2;
    NvU32    Rsvd:2;                    // Must be set to 0
    NvU32    Format_Flag:1;             // Must be set to 1

    // Packaged data
    NvU32    Min_Red_Data:12;
    NvU32    Max_Red_Data:12;
    NvU32    Min_Green_Data:12;
    NvU32    Max_Green_Data:12;
    NvU32    Min_Blue_Data:12;
    NvU32    Max_Blue_Data:12;
} NV_GAMUT_METADATA_RANGE;

typedef struct _NV_GAMUT_METADATA_VERTICES
{
    // Header
    NvU32    GBD_Color_Space:3;
    NvU32    GBD_Color_Precision:2;
    NvU32    Rsvd:1;
    NvU32    Facet_Mode:1;              // Must be set to 0
    NvU32    Format_Flag:1;             // Must be set to 0
    NvU32    Number_Vertices_H:8;       // Must be set to 0
    NvU32    Number_Vertices_L:8;       // Must be set to 4

    // Packaged data
    NvU32    Black_Y_R:12;
    NvU32    Black_Cb_G:12;
    NvU32    Black_Cr_B:12;
    NvU32    Red_Y_R:12;
    NvU32    Red_Cb_G:12;
    NvU32    Red_Cr_B:12;
    NvU32    Green_Y_R:12;
    NvU32    Green_Cb_G:12;
    NvU32    Green_Cr_B:12;
    NvU32    Blue_Y_R:12;
    NvU32    Blue_Cb_G:12;
    NvU32    Blue_Cr_B:12;
} NV_GAMUT_METADATA_VERTICES;

typedef struct _NV_GAMUT_METADATA
{
   union
   {
        NV_GAMUT_METADATA_RANGE     rangeData;
        NV_GAMUT_METADATA_VERTICES  verticesData;
   }data;
}NV_GAMUT_METADATA;

NVAPI_INTERFACE NvAPI_D3D9_SetGamutData(IDirect3DDevice9 *pDev, NvU32 outputId, NV_GAMUT_FORMAT format, NV_GAMUT_METADATA *pGamutData);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_D3D9_Video_GetVideoCapabilities
//
//   DESCRIPTION: Get the GPU's video processing capabilities. The caller is responsible for furnishing
//                the inputs within the NVAPI_VIDEO_CAPS_PACKET. This packet shall contain the output
//                consisting of the supported video features for the current configuration. The number of
//                NVAPI_VIDEO_CAPS will match the number of input video streams for which the capabilites
//                have been requested. Multiple supported video profiles may be returned that would include
//                a combination of various video features.
//
//  SUPPORTED OS: Windows XP and higher
//
//         INPUT: pDev                          The device (display) to present to
//                version                       version of the NV_DX_VIDEO_CAPS structure
//                videoCapsPacket               NVAPI_VIDEO_CAPS_PACKET containing both the
//                                              input Video Source information for which the capabilities
//                                              are requested, as well as the output available
//                                              supported video features for the current configuration.
//
// RETURN STATUS: an int which could be an NvAPI status or DX HRESULT code
//
///////////////////////////////////////////////////////////////////////////////

typedef enum _NV_CODEC
{
    NV_CODEC_TYPE_NONE,
    NV_CODEC_TYPE_MPEG2,
    NV_CODEC_TYPE_H264,
    NV_CODEC_TYPE_VC1,
} NV_CODEC;

// current video surface to be played back
typedef struct _NVAPI_VIDEO_SRC_INFO
{
    NvU32       srcWidth;   // Input video width
    NvU32       srcHeight;  // Input video height
    NV_CODEC    codecType;  // One of the available NV_CODEC's
    NvU32       avgBitrate; // Average bit rate for each stream in Kbps
    NvU64       reserved1;  // reserved for future expansion
    NvU64       reserved2;
} NVAPI_VIDEO_SRC_INFO;

// current desktop resolution
typedef struct _NVAPI_DESKTOP_RES
{
    NvU32 width;            // current resolution width
    NvU32 height;           // current resolution height
    NvU32 bitsPerPixel;     // current resolution depth
    NvU32 refreshRate;      // current display refresh rate
    NvU64 reserved1;        // reserved for future expansion
    NvU64 reserved2;
} NVAPI_DESKTOP_RES;

#define NV_DEINTERLACE_PIXADAPTIVE_BIT      0   // HW Pixel adaptive deinterlacing available
#define NV_VID_ENHANCE_EDGE_ENHANCE_BIT     0   // Edge enhancement present
#define NV_VID_ENHANCE_NOISE_REDUCTION_BIT  1   // Noise reduction present
#define NV_COLOR_CTRL_PROCAMP_BIT           0   // ProCamp is supported
#define NV_COLOR_CTRL_COLOR_TEMP_BIT        1   // Color temp control supported
#define NV_GAMMA_Y_BIT                      0   // Y-Gamma controls present
#define NV_GAMMA_RGB_BIT                    1   // RGB-Gamma controls present
#define NV_MISC_CAPS_INV_TELECINE_BIT       0   // Inverse telecine is available

// available video post-process features
typedef struct _NVAPI_VIDEO_PROCESSING_CAPS
{
    NvU64               deinterlaceMode;    // Possible deinterlace modes supported, "check NV_DEINTERLACE_xxx bits"
    NvU64               videoEnhance;       // Possible HW postproc enhancements e.g. NR, EE, "check NV_VID_ENHANCE_xxx bits"
    NvU64               colorControl;       // Color control is supported, "check NV_COLOR_xxx bits"
    NvU64               gamma;              // Available gamma conversions, "check NV_GAMMA_xxx bits"
    NvU64               miscCaps;           // Miscellaneous post processing caps supported, "check NV_MISC_CAPS_xxx bits"
    NvU64               reserved1;          // reserved for future use
    NvU64               reserved2;
    NvU64               reserved3;
} NVAPI_VIDEO_PROCESSING_CAPS;

#define NV_VID_FEATURE_NO_SYNC_FLIPS_BIT            0   // when this bit is set, APP needs to Lock the RGB render target
                                                        // before calling RGB overlay to present video.
                                                        // In general this bit is set for G7x, and not set for G8x and later chips
#define NV_VID_FEATURE_HALF_RES_ON_INTERLACED_BIT   1   // When this bit is set, it means that GPU can only show half resolution
                                                        // video on interlaced display, which is true for G7x. Needs special
                                                        // workaround by app to show full-resolution video.
#define NV_VID_FEATURE_DX_PROTECTION_VERSION_2      2   // when this bit it set, it means the driver supports version 2 of
                                                        // DX video protection.
#define NV_PERF_LEVEL_RED_BIT                       0   // when this bit is set, it means the GPU can NOT support HD/BD playback
#define NV_PERF_LEVEL_YELLOW_BIT                    1   // when this bit is set, it means the GPU meets the minimum requirement for HD/BD playback
#define NV_PERF_LEVEL_GREEN_BIT                     2   // when this bit is set, it means the GPU can support HD/BD playback without problem
#define NV_PERF_LEVEL_AERO_BIT                      3   // when this bit is set, it means the GPU can support HD/BD playback with Aero enabled
#define NV_PERF_LEVEL_FRUC_BIT                      4   // when this bit is set, it means the GPU can support FRUC

// available video decode and post-process features
typedef struct _NVAPI_VIDEO_CAPS
{
    NvU32                       maxFlipRate;    // Maximum flipping rate supported
    NV_CODEC                    hwDecode;       // Supported codec for HW decoding
    NvU64                       vidFeature;     // Indicates presence of special video processing features
    NvU32                       perfLevel;      // GPU dependent level; can be red, yellow, or green
    NvU32                       numVidProfiles; // Number of returned video profiles
    NvU32                       maxResPixels;   // Number of pixels in the max screen resolution supported at maxFlipRate for the input codecType
                                                // (refer to codecType field of the NVAPI_VIDEO_SRC_INFO structure)
    NvU32                       reserved1;
    NVAPI_VIDEO_PROCESSING_CAPS vidProcCaps[NV_MAX_VID_PROFILES];
} NVAPI_VIDEO_CAPS;

#define NVAPI_VIDEO_CAPS_PACKET_VER 1
#define RENDER_MODE_DWM_BIT         0
#define RENDER_MODE_OVERLAY_BIT     1

typedef struct _NVAPI_VIDEO_CAPS_PACKET
{
    NvU32                   packetVer;                      // (IN) Packet version that needs to match NVAPI_VIDEO_CAPS_PACKET_VER
    NvU32                   numVidStreams;                  // (IN) Number of streams for which the video caps are requested
    NVAPI_VIDEO_SRC_INFO    vidSrcInfo[NV_MAX_VID_STREAMS]; // (IN) Video source info
    NVAPI_DESKTOP_RES       res[NV_MAX_HEADS];              // (IN) Current desktop resolution(s)
    NvU64                   renderMode;                     // (IN) Requested rendering mode for the video source
    NvU64                   totalFB;                        // (OUT) total FB supported
    NvU16                   NumExtOvlBufNeeded;             // (OUT) recommanded number of external overlay buffers
    NvU16                   reserved1;                      // (IN/OUT) reserved for future expansion
    NvU32                   reserved2;                      // (IN/OUT)

    NVAPI_VIDEO_CAPS        videoCaps[NV_MAX_VID_STREAMS];  // (OUT) Available video features for requested video streams
} NVAPI_VIDEO_CAPS_PACKET;

typedef struct _NV_DX_VIDEO_CAPS
{
    NvU32                   version;                        // (IN) NVAPI version that matched NV_DX_VIDEO_CAPS_VER
    NVAPI_VIDEO_CAPS_PACKET videoCapsPacket;                // (IN/OUT)
} NV_DX_VIDEO_CAPS;

#define NV_DX_VIDEO_CAPS_VER  MAKE_NVAPI_VERSION(NV_DX_VIDEO_CAPS,1)

NVAPI_INTERFACE NvAPI_D3D9_GetVideoCapabilities(IDirect3DDevice9 *pDev,
                                                NV_DX_VIDEO_CAPS *pVideoCaps);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_D3D9_RestoreDesktop
//
//   DESCRIPTION:
//
// RETURN STATUS: NVAPI_ERROR or NVAPI_OK
//
// NOTE: This is not an interface, this is just a short-hand helper
//
///////////////////////////////////////////////////////////////////////////////
inline int NvAPI_D3D9_RestoreDesktop(IDirect3DDevice9 *pDev)
{
    return NvAPI_D3D9_PresentSurfaceToDesktop(pDev,NVDX_OBJECT_NONE,0);
}


///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_D3D9_AliasPrimaryFromDevice
//
//   DESCRIPTION: Create an alias surface from the given pDevFrom's primary swap chain.
//
//  SUPPORTED OS: Windows XP and higher
//
// INPUT:   pDevTo      Where new surfaces created in
//          pDevFrom    Where the surfaces aliased from
//          dwIndex     Index to the primary flipchain of pDevFrom
//
// OUTPUT:  ppSurf      Filled with new surface pointer (to be released by caller)
//          pHandle     (optional) If non-NULL, filled with SurfaceHandle of the surface
//                      Same can be achieved by calling NVD3D9_GetSurfaceHandle afterwards
//
// DEMO:    Multihead
//
// RETURN STATUS: an int which could be an NvAPI status or DX HRESULT code
//
NVAPI_INTERFACE NvAPI_D3D9_AliasPrimaryFromDevice(IDirect3DDevice9 *pDevTo,
                                              IDirect3DDevice9 *pDevFrom,
                                              NvU32 dwIndex,
                                              IDirect3DSurface9 **ppSurf,
                                              NVDX_ObjectHandle *pHandle = 0);
 
///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_D3D9_SetResourceHint
//
//   DESCRIPTION: Deprecated
//
//  SUPPORTED OS: Windows XP and higher
//
//  INPUT: pDev - valid device context
//         obj  - previously obtained HV resource handle
//         dwHintCategory - category of the hints
//         dwHintType     - a hint within this category
//         *pdwHintValue  - pointer to location containing hint value
//
//  OUTPUT:
//         *dwHintValue   - receives previous value of this hint.
//
//
// Avaliable hint categories / hint names:
//     Sli:
typedef enum _NVAPI_SETRESOURCEHINT_CATEGORY
{
    NvApiHints_Sli = 1,

}  NVAPI_SETRESOURCEHINT_CATEGORY;
//
//
// types of Sli hints
  // NvApiHints_Sli_InterframeAwareForTexturing - deprecated
  // default value: zero
//
typedef enum _NVAPI_SETRESOURCEHINT_SLI_HINTS
{
    NvApiHints_Sli_InterframeAwareForTexturing = 1,
}  NVAPI_SETRESOURCEHINT_SLI_HINTS;
//
// end of hint categories
//
// RETURN STATUS: an int which could be an NvAPI status or DX HRESULT code
NVAPI_INTERFACE NvAPI_D3D9_SetResourceHint(IDirect3DDevice9 *pDev, NVDX_ObjectHandle obj,
                            NVAPI_SETRESOURCEHINT_CATEGORY dwHintCategory, NvU32 dwHintName, NvU32 *pdwHintValue);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_D3D9_Lock
//
//   DESCRIPTION: Lock and Unlock a given surface identified by handle. This
//                function can provide CPU access to all object including
//                RTs, ZBs, textures, VBs and IBs.
//
//  SUPPORTED OS: Windows XP and higher
//
// NOTE:
// (a) If an object can be accessed with normal DX9 means, please do not use this
// (b) Lock should be called right before CPU access, and Unlock called right after
//     the access is done. Any 3D rendering or state change may cause the locked
//     surface to be lost. When that happens, trying to access the cached CPU
//     address may causing app to crash.
//
// RETURN STATUS: an int which could be an NvAPI status or DX HRESULT code
//
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
#define NV_ACCESSFLAG_READONLY  0x00000001 // TBD: do these go with dwLockFlags?
#define NV_ACCESSFLAG_DISCARD   0x00000002
NVAPI_INTERFACE NvAPI_D3D9_Lock(IDirect3DDevice9 *pDev, NVDX_ObjectHandle obj, NvU32 dwLockFlags,
                            void **ppAddress, NvU32 *pPitch);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_D3D9_Unlock
//
//   DESCRIPTION:
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS: an int which could be an NvAPI status or DX HRESULT code
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_D3D9_Unlock(IDirect3DDevice9 *pDev, NVDX_ObjectHandle obj);

#ifndef NV_VIDEO_COMPONENTS_DEFINE
#define NV_VIDEO_COMPONENTS_DEFINE

///////////////////////////////////////////////////////////////////////////////
// Structs and enums related to Video state
///////////////////////////////////////////////////////////////////////////////

// Components related to video state
typedef enum _NVAPI_VIDEO_STATE_COMPONENT_ID
{
    NVAPI_VIDEO_STATE_COMPONENT_ID_NONE     = -1,   // Placeholder for invalid component ID
    NVAPI_VIDEO_STATE_COMPONENT_BRIGHTNESS      ,   // Permits control of video's brightness value
    NVAPI_VIDEO_STATE_COMPONENT_CONTRAST        ,   // Allows control of video's contrast value
    NVAPI_VIDEO_STATE_COMPONENT_HUE             ,   // To control the hue value
    NVAPI_VIDEO_STATE_COMPONENT_SATURATION      ,   // Allows control of video's saturation value
    NVAPI_VIDEO_STATE_COMPONENT_COLORTEMP       ,   // Allows control of the color temperature value
    NVAPI_VIDEO_STATE_COMPONENT_Y_GAMMA         ,   // To set the Y-gamma values
    NVAPI_VIDEO_STATE_COMPONENT_RGB_GAMMA_R     ,   // To set the R value of RGB gamma
    NVAPI_VIDEO_STATE_COMPONENT_RGB_GAMMA_G     ,   // To set the G value of RGB gamma
    NVAPI_VIDEO_STATE_COMPONENT_RGB_GAMMA_B     ,   // To set the B value of RGB gamma
    NVAPI_VIDEO_STATE_COMPONENT_COLOR_SPACE     ,   // Permits choice of various color spaces using VIDEO_COMP_ALGO_COLOR_SPACE_xxx
    NVAPI_VIDEO_STATE_COMPONENT_COLOR_RANGE     ,   // Allows setting between a limited/full color range using VIDEO_COMP_ALGO_COLOR_RANGE_xxx
    NVAPI_VIDEO_STATE_COMPONENT_PLAYBACK_PROFILE,   // Permits using special postprocessing for Adobe Flash 9 Content
    NVAPI_VIDEO_STATE_COMPONENT_DEINTERLACE     ,   // To set various types of deinterlacing algorithms
    NVAPI_VIDEO_STATE_COMPONENT_SCALING         ,   // Allows setting video scaling algorithms
    NVAPI_VIDEO_STATE_COMPONENT_CADENCE         ,   // Allows control of the cadence algorithms
    NVAPI_VIDEO_STATE_COMPONENT_NOISE_REDUCE    ,   // Allows setting post-processing noise reduction values
    NVAPI_VIDEO_STATE_COMPONENT_EDGE_ENHANCE    ,   // Permits post-processing edge enhancement value adjustment
    NVAPI_VIDEO_STATE_COMPONENT_OVERDRIVE       ,   // To control the overdrive feature
    NVAPI_VIDEO_STATE_COMPONENT_SPLITSCREEN     ,   // To permit setting a splitscreen using one of VIDEO_COMP_ALGO_SPLITSCREEN_xxx
    NVAPI_VIDEO_STATE_COMPONENT_DEBLOCKING      ,   // Allows out-of-loop deblocking
    NVAPI_VIDEO_STATE_COMPONENT_DYNAMIC_CONTRAST,  // Permits control of video's dynamic contrast value
    NVAPI_VIDEO_STATE_COMPONENT_GREEN_STRETCH   ,   // Permits control of green stretch
    NVAPI_VIDEO_STATE_COMPONENT_BLUE_STRETCH    ,   // Allows control of blue enhancement
    NVAPI_VIDEO_STATE_COMPONENT_SKIN_TONE_CORRECTION, // Allows skin-tone correction for video
    NVAPI_VIDEO_STATE_COMPONENT_GAMUT_REMAPPING ,   // Applies gamut remapping on video
    NVAPI_VIDEO_STATE_COMPONENT_ID_LAST         ,   // All valid components defined before this one
} NVAPI_VIDEO_STATE_COMPONENT_ID;

#define NVAPI_VIDEO_STATE_COMPONENT_DYNAMIC_CONSTRAST  NVAPI_VIDEO_STATE_COMPONENT_DYNAMIC_CONTRAST  // dynamic contrast value. Kept this for backward compatibility

// Algorithms controlling various video components

#define VIDEO_COMP_ALGO_CUSTOM_BASE 64

typedef enum _NVAPI_VIDEO_COMPONENT_ALGORITHM
{
    VIDEO_COMP_ALGO_COLOR_SPACE_601                  = 0,  // Use the ITU-R BT.601 standard in color-space conversion for xxx_COLOR_SPACE component
    VIDEO_COMP_ALGO_COLOR_SPACE_709                  = 1,  // Use the ITU-R BT.709 standard in color-space conversion for xxx_COLOR_SPACE component
    VIDEO_COMP_ALGO_COLOR_RANGE_STD                  = 0,  // Full range of (0-255) for xxx_COLOR_RANGE component, equivalent to Microsoft's DXVADDI_NOMINALRANGE::DXVADDI_NominalRange_0_255
    VIDEO_COMP_ALGO_COLOR_RANGE_EXT                  = 1,  // Limited range of (16-235) for xxx_COLOR_RANGE component, equivalent to Microsoft's DXVADDI_NOMINALRANGE::DXVADDI_NominalRange_16_235
    VIDEO_COMP_ALGO_PLAYBACK_PROFILE_NONE            = 0,  // Use no playback profile
    VIDEO_COMP_ALGO_PLAYBACK_PROFILE_ADOBE_FLASH_9   = 1,  // Use the internet video enhancement postprocessing for Adobe Flash 9 
    VIDEO_COMP_ALGO_DEINTERLACE_NONE                 = 0,  // No deinterlacing is done
    VIDEO_COMP_ALGO_DEINTERLACE_BOB                  = 1,  // Perform Bob deinterlacing
    VIDEO_COMP_ALGO_DEINTERLACE_WEAVE                = 2,  // Use weave deinterlacing
    VIDEO_COMP_ALGO_DEINTERLACE_SIMPLE_ADAPTIVE      = 3,  // Perform a simple motion adaptive deinterlacing
    VIDEO_COMP_ALGO_DEINTERLACE_GRADIENT_SIMPLE      = 4,  // Use a simple gradient deinterlacing
    VIDEO_COMP_ALGO_DEINTERLACE_GRADIENT_FULL        = 5,  // Use advanced gradient deinterlacing
    VIDEO_COMP_ALGO_DEINTERLACE_ADAPTIVE_FOUR_FIELD  = 6,  // Perform four field motion adaptive deinterlacing
    VIDEO_COMP_ALGO_DEINTERLACE_DIR_SPATIAL          = 7,  // User directional spatial deinterlacing
    VIDEO_COMP_ALGO_DEINTERLACE_ADVANCED             = 8,  // Perform proprietary advanced deinterlacing
    VIDEO_COMP_ALGO_DEINTERLACE_GPU_CAPABLE          = 9,  // Best available but GPU dependent. (video driver decides dynamically)
    VIDEO_COMP_ALGO_DEINTERLACE_MEDIAN               = 10,  // Best available but GPU dependent. (video driver decides dynamically)
    VIDEO_COMP_ALGO_DEINTERLACE_DIR_SPATIAL_LIGHT    = 11,  // Best available but GPU dependent. (video driver decides dynamically)
    VIDEO_COMP_ALGO_DEINTERLACE_DIR_SPATIAL_SD       = 12,  // Best available but GPU dependent. (video driver decides dynamically)
    VIDEO_COMP_ALGO_DEINTERLACE_DIR_SPATIAL_HD       = 13,  // Best available but GPU dependent. (video driver decides dynamically)
    VIDEO_COMP_ALGO_DEINTERLACE_ONE_PASS             = 14,  // Best available but GPU dependent. (video driver decides dynamically)
    VIDEO_COMP_ALGO_DEINTERLACE_COMPUTE              = 15,  // Best available but GPU dependent. (video driver decides dynamically)
    VIDEO_COMP_ALGO_DEINTERLACE_CUSTOM_00            = VIDEO_COMP_ALGO_CUSTOM_BASE+0,    // Use custom Deinterlacing algorithm
    VIDEO_COMP_ALGO_DEINTERLACE_CUSTOM_01            = VIDEO_COMP_ALGO_CUSTOM_BASE+1,    // Use custom Deinterlacing algorithm
    VIDEO_COMP_ALGO_SCALING_ALG_SIMPLE               = 0,  // Do scaling using a simple algorithm
    VIDEO_COMP_ALGO_SCALING_ALG_4x4FILTER            = 1,  // Perform scaling using a 4x4 filter
    VIDEO_COMP_ALGO_SCALING_ALG_8x8FILTER            = 2,  // Perform scaling using a 8x8 filter
    VIDEO_COMP_ALGO_SCALING_ALG_CUSTOM_00            = VIDEO_COMP_ALGO_CUSTOM_BASE+0,    // Use custom scaling component
    VIDEO_COMP_ALGO_SCALING_ALG_CUSTOM_01            = VIDEO_COMP_ALGO_CUSTOM_BASE+1,    // Use custom scaling component
    VIDEO_COMP_ALGO_CADENCE_NONE                     = 0,  // Turn cadence OFF
    VIDEO_COMP_ALGO_CADENCE_SIMPLE                   = 1,  // Use simple cadence detection
    VIDEO_COMP_ALGO_CADENCE_VOF                      = 2,  // Use video on film cadence detection
    VIDEO_COMP_ALGO_CADENCE_COMPUTE                  = 3,  // Use compute cadence detection
    VIDEO_COMP_ALGO_CADENCE_GPU_CAPABLE              = 4,  // Best available but GPU dependent. (video driver decides dynamically)
    VIDEO_COMP_ALGO_NOISE_REDUCE_PUREVIDEO           = 0,  // Use PureVideo noise reduction
    VIDEO_COMP_ALGO_NOISE_REDUCE_CUSTOM_00           = VIDEO_COMP_ALGO_CUSTOM_BASE+0,  // Use custom noise reduction
    VIDEO_COMP_ALGO_NOISE_REDUCE_CUSTOM_01           = VIDEO_COMP_ALGO_CUSTOM_BASE+1,  // Use custom noise reduction
    VIDEO_COMP_ALGO_EDGE_ENHANCE_PUREVIDEO           = 0,  // Use PureVideo Sharpening Filter
    VIDEO_COMP_ALGO_EDGE_ENHANCE_CUSTOM_00           = VIDEO_COMP_ALGO_CUSTOM_BASE+0,  // Use custom Sharpening Filter
    VIDEO_COMP_ALGO_EDGE_ENHANCE_CUSTOM_01           = VIDEO_COMP_ALGO_CUSTOM_BASE+1,  // Use custom Sharpening Filter
    VIDEO_COMP_ALGO_OVERDRIVE_SIMPLE                 = 0,  // Use simple overdrive algorithm
    VIDEO_COMP_ALGO_SPLITSCREEN_TYPE_NORMAL          = 0,  // Set the splitscreen in normal mode
    VIDEO_COMP_ALGO_SPLITSCREEN_TYPE_REPEATED        = 1,  // Set the splitscreen to be repeated
    VIDEO_COMP_ALGO_SPLITSCREEN_TYPE_ON_MIRROR       = 2,  // Set the splitscreen as a mirror
    VIDEO_COMP_ALGO_DYNAMIC_CONTRAST_PRESET_NONE     = 0,  // Use the value setting for dynamic contrast instead of a preset
    VIDEO_COMP_ALGO_DYNAMIC_CONTRAST_PRESET_LOW      = 1,  // Turn the dynamic contrast to a low setting
    VIDEO_COMP_ALGO_DYNAMIC_CONTRAST_PRESET_MEDIUM   = 2,  // Turn the dynamic contrast to a medium setting
    VIDEO_COMP_ALGO_DYNAMIC_CONTRAST_PRESET_HIGH     = 3,  // Turn the dynamic contrast to a high setting
    VIDEO_COMP_ALGO_GREEN_STRETCH_PRESET_NONE        = 0,  // Use the value setting for green stretch instead of a preset
    VIDEO_COMP_ALGO_GREEN_STRETCH_PRESET_LOW         = 1,  // Set the green strech to a low setting
    VIDEO_COMP_ALGO_GREEN_STRETCH_PRESET_MEDIUM      = 2,  // Set the green strech to a medium setting
    VIDEO_COMP_ALGO_GREEN_STRETCH_PRESET_HIGH        = 3,  // Set the green strech to a high setting
    VIDEO_COMP_ALGO_BLUE_STRETCH_PRESET_NONE         = 0,  // Use the value setting for blue stretch instead of a preset
    VIDEO_COMP_ALGO_BLUE_STRETCH_PRESET_LOW          = 1,  // Set the blue strech to a low setting
    VIDEO_COMP_ALGO_BLUE_STRETCH_PRESET_MEDIUM       = 2,  // Set the blue strech to a medium setting
    VIDEO_COMP_ALGO_BLUE_STRETCH_PRESET_HIGH         = 3,  // Set the blue strech to a high setting
    VIDEO_COMP_ALGO_SKIN_TONE_CORRECTION_PRESET_NONE = 0,  // Use the value setting for skin tone correction instead of a preset
    VIDEO_COMP_ALGO_SKIN_TONE_CORRECTION_PRESET_SQUEEZE = 1,  // Turn the skin tone correction to a low setting
    VIDEO_COMP_ALGO_SKIN_TONE_CORRECTION_PRESET_RED     = 2,  // Turn the skin tone correction to a medium setting
    VIDEO_COMP_ALGO_SKIN_TONE_CORRECTION_PRESET_YELLOW  = 3,  // Turn the skin tone correction to a high setting
    VIDEO_COMP_ALGO_GAMUT_REMAPPING_CUSTOM_00        = VIDEO_COMP_ALGO_CUSTOM_BASE+0, // Use custom 3x3 gamut remapping matrix
    VIDEO_COMP_ALGO_GAMUT_REMAPPING_CUSTOM_01        = VIDEO_COMP_ALGO_CUSTOM_BASE+1, // Use custom 3x3 gamut remapping matrix
} NVAPI_VIDEO_COMPONENT_ALGORITHM;

// bitmasks for video components' enable at various resolutions
typedef enum _NVAPI_VIDEO_COMPONENT_ENABLE
{
    VIDEO_COMP_ENA_480i     = 0x00000001, // component is enabled at 480i video resolution
    VIDEO_COMP_ENA_480p     = 0x00000002, // component is enabled at 480p video resolution
    VIDEO_COMP_ENA_576i     = 0x00000004, // component is enabled at 576i video resolution
    VIDEO_COMP_ENA_576p     = 0x00000008, // component is enabled at 576p video resolution
    VIDEO_COMP_ENA_720p     = 0x00000010, // component is enabled at 720p video resolution
    VIDEO_COMP_ENA_1080i    = 0x00000020, // component is enabled at 1080i video resolution
    VIDEO_COMP_ENA_1080p    = 0x00000040, // component is enabled at 1080p video resolution
} NVAPI_VIDEO_COMPONENT_ENABLE;

// Packet that facilitates retrieving information about a video component
typedef struct _NVAPI_GET_VIDEO_STATE_COMPONENT
{
    NvU32   version;                    // (IN)  NVAPI version that matches NVAPI_GET_VIDEO_STATE_COMPONENT_VER
    NvU32   componentID;                // (IN)  identify the individual component, one of NVAPI_VIDEO_STATE_COMPONENT_xxx enums
    NvU32   bIsSupported        : 1;    // (OUT) set if this component feature is supported
    NvU32   bIsOverridenByUser  : 1;    // (OUT) set if component is overriden by user's choice
    NvU32   reserved1           : 30;   // (OUT) reserved for future expansion
    NvU32   isEnabled;                  // (OUT) set if component is enabled, one or more of NVAPI_VIDEO_COMPONENT_ENABLE bitmasks
    NvU32   minValue;                   // (OUT) min valid value
    NvU32   maxValue;                   // (OUT) max valid value
    NvU32   totalSteps;                 // (OUT) number of steps between min and max
    NvU32   defaultValue;               // (OUT) pre-defined NVIDIA default
    NvU32   unityValue;                 // (OUT) unity is the disable value for a component
    NvU32   currentValueActive;         // (OUT) value in use
    NvU64   defaultAlgo;                // (OUT) default algo, one or more of NVAPI_VIDEO_COMPONENT_ALGORITHM enums
    NvU64   currentAlgoActive;          // (OUT) algo in use, one or more of NVAPI_VIDEO_COMPONENT_ALGORITHM enums
    union
    {
        NvU64 qwReserved[9];
        struct
        {
            NvU32   dwAppKey;           // (IN) Secret key to authenticate the caller, ONLY used in NvAPI_SetVideoState
            NvU32   bTopPriority   : 1; // (OUT)indicates that these settings have precedence over D3D NvAPI settings, ONLY valid in NvAPI_GetVideoState
            NvU32   bHasCustomAlgo : 1; // (OUT)indicates whether Out-of-process app has custom algorithm data
            NvU32   bReserved      : 30;// (OUT) reserved for expansion
            struct
            {
                NvU64   pData;          // (IN) Buffer to hold the retreived custom algo data
                NvU32   dwSize;         // (IN) Size in Bytes of the above buffer, must be <= NVAPI_VIDEO_STATE_MAX_CUSTOM_ALGO_SIZE.
            } customAlgo;
        } appInfo;
    };
} NVAPI_GET_VIDEO_STATE_COMPONENT;

#define NVAPI_GET_VIDEO_STATE_COMPONENT_VER  MAKE_NVAPI_VERSION(NVAPI_GET_VIDEO_STATE_COMPONENT,1)

// Return status after attempting to set a video component
typedef enum _NVAPI_VIDEO_COMP_RETURN_STATUS
{
    VIDEO_COMP_STATUS_SUCCESS                   = 0x00000000, // Video component is set successfully
    VIDEO_COMP_STATUS_UNSUCCESSFUL              = 0x00000001, // Failed to set video component
    VIDEO_COMP_STATUS_COMPONENT_NOT_SUPPORTED   = 0x00000002, // Video component is not supported
    VIDEO_COMP_STATUS_VALUE_OUT_OF_RANGE        = 0x00000004, // Video component's value is invalid and does not fall into range
    VIDEO_COMP_STATUS_ALGO_NOT_RECOGNIZED       = 0x00000008, // Video component's algorithm is invalid
    VIDEO_COMP_STATUS_OVERRIDDEN_BY_USER        = 0x00000010, // Request not completed because of user-mandated override
    VIDEO_COMP_STATUS_Y_GAMMA_ENABLED           = 0x00000020, // Cannot set RGB-gamma because Y-Gamma is already enabled
    VIDEO_COMP_STATUS_RGB_GAMMA_ENABLED         = 0x00000040, // Cannot set Y-gamma because RGB-Gamma is already enabled
} NVAPI_VIDEO_COMP_RETURN_STATUS;

// Packet containing information to allow setting the video component

#define NVAPI_VIDEO_STATE_MAX_CUSTOM_ALGO_SIZE 128

typedef struct _NVAPI_SET_VIDEO_STATE_COMPONENT
{
    NvU32   version;                        // (IN) NVAPI version that matches NVAPI_SET_VIDEO_STATE_COMPONENT_VER
    NvU32   componentID;                    // (IN) identify the individual component, one of NVAPI_VIDEO_STATE_COMPONENT_xxx enums
    NvU32   enable;                         // (IN) flag to enable setting of component, one or more of NVAPI_VIDEO_COMPONENT_ENABLE bitmasks
    NvU32   setToValue;                     // (IN) value to use
    NvU64   setToAlgo;                      // (IN) algorithm to use
    NvU32   retStatus;                      // (OUT) result of video-component-set operation; a combination of VIDEO_COMP_STATUS_xxx bitmasks
    NvU32   reserved;
    union
    {
        NvU64 qwReserved[4];
        struct
        {
            NvU32   dwAppKey;               // (IN) Secret key to authenticate the caller, ONLY used in NvAPI_SetVideoState
            NvU32   bTopPriority       : 1; // (IN) Force these settings to have priority over D3D NvAPI settings, ONLY valid in NvAPI_GetVideoState
            NvU32   bHasCustomAlgo     : 1; // (IN) Out-of-process app has custom algorithm data
            NvU32   bReserved          : 30;// (IN) reserved for expansion
            struct
            {
                NvU64   pData;              // (IN) (Used only when bHasCustomAlgo == 1) Pointer to the custom algo data.
                NvU32   dwSize;             // (IN) (Used only when bHasCustomAlgo == 1) Size in Bytes of the custom algo data, must be <= NVAPI_VIDEO_STATE_MAX_CUSTOM_ALGO_SIZE.
            } customAlgo;
        } appInfo;
    };
} NVAPI_SET_VIDEO_STATE_COMPONENT;

#define NVAPI_SET_VIDEO_STATE_COMPONENT_VER  MAKE_NVAPI_VERSION(NVAPI_SET_VIDEO_STATE_COMPONENT,1)

#endif // NV_VIDEO_COMPONENTS_DEFINE

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_D3D9_GetVideoState
//
//   DESCRIPTION: Return the video state component for the component ID passed in.
//
//  SUPPORTED OS: Windows Vista and higher
//
//         INPUT: pDev             (IN)         The D3D device for which the component is requested
//                pGetVidStateComp (IN/OUT)     NVAPI_GET_VIDEO_STATE_COMPONENT packet containing
//                                              a valid component ID
//
// RETURN STATUS: an int which could be an NvAPI status (NVAPI_OK, NVAPI_INVALID_ARGUMENT,
//                NVAPI_INCOMPATIBLE_STRUCT_VERSION, NVAPI_ERROR) or DX HRESULT code
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_D3D9_GetVideoState(IDirect3DDevice9 *pDev,
                                         NVAPI_GET_VIDEO_STATE_COMPONENT *pGetVidStateComp);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_D3D9_SetVideoState
//
//   DESCRIPTION: API that allows callers to set a particular video state component
//
//  SUPPORTED OS: Windows Vista and higher
//
//         INPUT: pDev             (IN)         The D3D device for which the component-set is requested
//                pSetVidStateComp (IN/OUT)     NVAPI_SET_VIDEO_STATE_COMPONENT packet containing
//                                              the video component info to be applied
//
// RETURN STATUS: an int which could be an NvAPI status (NVAPI_OK, NVAPI_INVALID_ARGUMENT,
//                NVAPI_INCOMPATIBLE_STRUCT_VERSION, NVAPI_ERROR) or DX HRESULT code
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_D3D9_SetVideoState(IDirect3DDevice9 *pDev,
                                         NVAPI_SET_VIDEO_STATE_COMPONENT *pSetVidStateComp);


///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_D3D9_EnumVideoFeatures
//
//   DESCRIPTION:   Enumerates all video featuew supported on current config 
//                  First, the client should call the function with VF_GET_COUNT flag set. This will return the total 
//                  feature count. 
//                  Then client will call this function again iteratively, with the VF_GET_DETAILS flag until all of 
//                  the features get enumerated.
//  SUPPORTED OS: Windows XP and higher
//
//
// RETURN STATUS: an int which could be an NvAPI status or DX HRESULT code
//
///////////////////////////////////////////////////////////////////////////////
// enums and structs related to Video Features
#define NVAPI_MAX_FEATURES_PER_PACKET   3 // max number of structs that can fit into dwArgs[20] of nvdxExtensionData struct

// Video feature IDs. Update the nvVidFeatureNames[] array in nvd3d.cpp when adding a new Feature ID.
typedef enum _NVAPI_VIDEO_FEATURE
{
    NVAPI_VF_UNKNOWN = 0,
    NVAPI_VF_MPEG2SDDecodeAcceleration,
    NVAPI_VF_MPEG2HDDecodeAcceleration,
    NVAPI_VF_VC1SDDecodeAcceleration,
    NVAPI_VF_VC1HDDecodeAcceleration,
    NVAPI_VF_H264SDDecodeAcceleration,
    NVAPI_VF_H264HDDecodeAcceleration,
    NVAPI_VF_WMV9SDDecodeAcceleration,
    NVAPI_VF_WMV9HDDecodeAcceleration,
    NVAPI_VF_ProcAmp,
    NVAPI_VF_ColorTemperatureCorrection,
    NVAPI_VF_ColorSpaceConversion,
    NVAPI_VF_GammaCorrection,
    NVAPI_VF_OverDrive,
    NVAPI_VF_DynamicContrastEnhancement,
    NVAPI_VF_StretchBlueGreenSkin,
    NVAPI_VF_AdaptiveDeInterlacing,
    NVAPI_VF_NoiseReduction,
    NVAPI_VF_EdgeEnhancement,
    NVAPI_VF_InverseTelecine,
    NVAPI_VF_Scaling,
    NVAPI_VF_VideoMirror,
    NVAPI_VF_Blend,
    NVAPI_VF_DxvaHDTest,
    NVAPI_VF_GamutRemap,
    NVAPI_VF_MftXcode,
    NVAPI_VF_SkinToneCorrection,

    NVAPI_VF_ID_LAST,
} NVAPI_VIDEO_FEATURE;

// Update the nvVidEngineNames[] array in nvd3d.cpp when adding a new Engine name.
typedef enum _NVAPI_VIDEO_ENGINES
{
    NVAPI_VE_UNKNOWN = 0,
    NVAPI_VE_PixelShader,
    NVAPI_VE_MPEG2Decoder,
    NVAPI_VE_VideoProcessor1,
    NVAPI_VE_VideoProcessor2,
    NVAPI_VE_MSDEC,
} NVAPI_VIDEO_ENGINES;

// Update the nvVidEntryPointNames[] array in nvd3d.cpp when adding a new Entry point in this enum.
typedef enum _NVAPI_VIDEO_DRIVER_ENTRY_POINT
{
    NVAPI_EP_UNKNOWN = 0,
    NVAPI_EP_DecodeEndFrame,
    NVAPI_EP_OverlayTransfer,
    NVAPI_EP_DIBlitEx,
    NVAPI_EP_VideoProcessBlt,
    NVAPI_EP_D3DBlt,
    NVAPI_EP_Blit32,
    NVAPI_EP_VPBltHD,
} NVAPI_VIDEO_DRIVER_ENTRY_POINT;

typedef enum _NVAPI_VIDEO_FEATURE_GET_FLAG
{
    NVAPI_VF_GET_COUNT = 0,
    NVAPI_VF_GET_DETAILS,
} NVAPI_VIDEO_FEATURE_GET_FLAG;

typedef struct _NVAPI_VIDEO_FEATURE_DETAILS
{
    NVAPI_VIDEO_FEATURE            eFeature;
    NVAPI_VIDEO_ENGINES            eEngine;
    NVAPI_VIDEO_DRIVER_ENTRY_POINT eEntryPoint;
    NvU32                          dwReserved;
} NVAPI_VIDEO_FEATURE_DETAILS;

// returns the strings for each feature to the client 
typedef struct _NVAPI_VID_FEATURE_STRINGS
{
    NVAPI_VIDEO_FEATURE eFeature;
    NvAPI_ShortString   szFeature; // feature name
    NvAPI_ShortString   szEngine; // video engine name
    NvAPI_ShortString   szDrvEntryPoint; // drv Entry Point name
    NvAPI_ShortString   szReserved1; // reserved for future expansion
} NVAPI_VID_FEATURE_STRINGS;

#define NVAPI_VIDEO_FEATURE_DESCRIPTOR_VER  1

typedef struct _NVAPI_VIDEO_FEATURE_DESCRIPTOR
{
    NvU32                          version;                     // (IN) version that matches NVAPI_VIDEO_FEATURE_DETAILS_PACKET_VER
    NVAPI_VIDEO_FEATURE_GET_FLAG   eVideoFeatureGetID;          // (IN) get total feature count or feature details
    NvU32                          dwVFCount;                   // (OUT) feature count returned
    NvU32                          dwStartVFCount;              // (IN) starting surface count when reading the data
    NVAPI_VIDEO_FEATURE_DETAILS    astFeatureDetails[NVAPI_MAX_FEATURES_PER_PACKET]; // (OUT) array of video feature details
    NvU32                          adwReserved[4];              // (IN/OUT) reserved for future expansion
} NVAPI_VIDEO_FEATURE_DESCRIPTOR;

typedef struct _NVAPI_DX_VIDEO_FEATURE_DETAILS
{
    NvU32                           version;              // (IN) NVAPI version that matched NVAPI_DX_VIDEO_FEATURE_DETAILS_VER
    NVAPI_VIDEO_FEATURE_DESCRIPTOR  videoFeaturesPacket;  // (IN/OUT)
    NVAPI_VID_FEATURE_STRINGS       astFeatureDescNames[NVAPI_MAX_FEATURES_PER_PACKET]; // (OUT) descriptor strings 
} NVAPI_DX_VIDEO_FEATURE_DETAILS;


#define NVAPI_DX_VIDEO_FEATURE_DETAILS_VER MAKE_NVAPI_VERSION(NVAPI_DX_VIDEO_FEATURE_DETAILS,1)


NVAPI_INTERFACE NvAPI_D3D9_EnumVideoFeatures(IDirect3DDevice9 *pDev, 
                                                NVAPI_DX_VIDEO_FEATURE_DETAILS *pVideoFeatureDetails);

                                         
///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_D3D9_GetSLIInfo
//
// DESCRIPTION:     This API is obsolete, please use NvAPI_D3D_GetCurrentSLIState
//
//  SUPPORTED OS: Windows XP
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_NO_ACTIVE_SLI_TOPOLOGY - SLI is not active on this device
//                  NVAPI_ERROR - error occurred
//
///////////////////////////////////////////////////////////////////////////////
typedef enum _NV_SLI_MODE
{
    NV_SLI_RENDERING_MODE_AUTOSELECT  = 0,     // AutoSelect
    NV_SLI_RENDERING_MODE_AFR         = 1,     // Alternate Frames
    NV_SLI_RENDERING_MODE_SFR         = 2,     // Split Frame
    NV_SLI_RENDERING_MODE_SINGLE      = 3      // Single GPU
} NV_SLI_MODE;

typedef struct
{
    NvU32 version;                       // Structure version

    NV_SLI_MODE         mode;            // [OUT] Current SLI mode
    NvU32               gpus;            // [OUT] Number of GPUs
} NV_SLI_INFO;
#define NV_SLI_INFO_VER  MAKE_NVAPI_VERSION(NV_SLI_INFO,1)

NVAPI_INTERFACE NvAPI_D3D9_GetSLIInfo(IDirect3DDevice9 *pDev, NV_SLI_INFO *pSliInfo);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_D3D9_SetSLIMode
//
// DESCRIPTION:     Sets the SLI rendering mode for D3D device. The NVAPI_SLI_RENDERING_MODE_NOTALLOWED
//                  return status occurs when SLI mode is set in the middle of a rendering. An app may try
//                  to recreate/reset device and perform the call again. The normal way to setup the SLI mode
//                  is immediately after CreateDevice() or Reset(). The NVAPI_NO_ACTIVE_SLI_TOPOLOGY value is
//                  returned when SLI is not active on this device.
//
//  SUPPORTED OS: Windows XP
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_NO_ACTIVE_SLI_TOPOLOGY - SLI is not active on this device
//                  NVAPI_SLI_RENDERING_MODE_NOTALLOWED - setup of SLI mode is not possible right now
//                  NVAPI_INVALID_ARGUMENT - invalid rendering mode
//                  NVAPI_ERROR - error occurred
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_D3D9_SetSLIMode(IDirect3DDevice9 *pDev, NV_SLI_MODE SliMode);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_D3D9_QueryAAOverrideMode
//
// DESCRIPTION:     Returns the AA mode selected through NVCPL,
//                  the corresponding buffer requirement and the equivalent multisample quality
//
//  SUPPORTED OS: Windows XP and higher
//
// INPUT:           pDev                  (IN)     The D3D device for which the component is requested
//                  pRenderingSampleCount (OUT)    The number of equivalent sample counts for this AA method, 0 indicates no CPLAA
//                  pBufferSampleCount    (OUT)    The number of buffer samples required for every pixel, 0 indicates no CPLAA
//                  pAAMode               (OUT)    The AA mode that is currently selected
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_ERROR - error occurred
//
///////////////////////////////////////////////////////////////////////////////

enum
{
    NVAPI_AAMODE_VCAA             = 0x00000001,    // The current AA method is one of the VCAA methods
    NVAPI_AAMODE_SLIAA            = 0x00000002,    // The current AA method is one of the SLIAA methods
    NVAPI_AAMODE_VCAA_HIGHQUALITY = 0x00000004     // The current AA method is a VCAA high quality method
};

NVAPI_INTERFACE NvAPI_D3D9_QueryAAOverrideMode(IDirect3DDevice9 *pDev, NvU32* pRenderingSampleCount, NvU32* pBufferSampleCount, NvU32* pAAMode);

///////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME:   NvAPI_D3D9_VideoSurfaceEncryptionControl
//
// DESCRIPTION:     This API is deprecated.
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS:   NVAPI_NOT_SUPPORTED - deprecated
//
///////////////////////////////////////////////////////////////////////////////

// Video Surface Encryption Control Commands
typedef enum
{
    NVAPI_VIDSURF_ENCRYPT_CMD_NULL              = 0x00, // Null command
    NVAPI_VIDSURF_ENCRYPT_CMD_GET_GUID_COUNT    = 0x01, // Get GUID count
    NVAPI_VIDSURF_ENCRYPT_CMD_GET_GUIDS         = 0x02, // Get GUIDs
    NVAPI_VIDSURF_ENCRYPT_CMD_SET_GUID          = 0x03, // Set GUID
    NVAPI_VIDSURF_ENCRYPT_CMD_ENABLE            = 0x04, // Enable encryption
    NVAPI_VIDSURF_ENCRYPT_CMD_SET_KEY           = 0x05, // Set key
    NVAPI_VIDSURF_ENCRYPT_CMD_SET_IV            = 0x06  // Set IV
} NVAPI_VIDSURF_ENCRYPT_COMMANDS;

#define NVAPI_VIDSURF_ENCRYPT_MAX_GUIDS        (4)

typedef struct
{
    NvU32   dwGuidCount;                                // (OUT)
} NVAPI_VIDSURF_ENCRYPT_CTRL_GET_GUID_COUNT;

typedef struct
{
    GUID    guids[NVAPI_VIDSURF_ENCRYPT_MAX_GUIDS];     // (OUT)
} NVAPI_VIDSURF_ENCRYPT_CTRL_GET_GUIDS;

typedef struct
{
    GUID    guid;                                       // (IN)
} NVAPI_VIDSURF_ENCRYPT_CTRL_SET_GUID;

typedef struct
{
    IDirect3DSurface9  *pSurface;                       // D3D9 surface ptr (IN)
    NvU32               bEnable;                        // (IN)
} NVAPI_VIDSURF_ENCRYPT_CTRL_ENABLE;

typedef struct
{
    NvU32   dwEncryptKeyProtectionMode;
    NvU32   dwEncryptKeyProtectionIdentifier;
    NvU32   dwKey[4];                                   // (IN)
} NVAPI_VIDSURF_ENCRYPT_CTRL_SET_KEY;

typedef struct
{
    NvU32   dwEncryptKeyProtectionMode;
    NvU32   dwEncryptKeyProtectionIdentifier;
    NvU32   dwIV[4];                                    // (IN)
} NVAPI_VIDSURF_ENCRYPT_CTRL_SET_IV;

typedef struct
{
    NvU32               version;                        // structure version (IN)
    NvU32               dwCommand;                      // command (IN)
    union
    {
        NVAPI_VIDSURF_ENCRYPT_CTRL_GET_GUID_COUNT       GetGuidCount;
        NVAPI_VIDSURF_ENCRYPT_CTRL_GET_GUIDS            GetGuids;
        NVAPI_VIDSURF_ENCRYPT_CTRL_SET_GUID             SetGuid;
        NVAPI_VIDSURF_ENCRYPT_CTRL_ENABLE               Enable;
        NVAPI_VIDSURF_ENCRYPT_CTRL_SET_KEY              SetKey;
        NVAPI_VIDSURF_ENCRYPT_CTRL_SET_IV               SetIV;
    };
} NVAPI_VIDSURF_ENCRYPT_CTRL_PARAMS;

#define NVAPI_VIDSURF_ENCRYPT_CTRL_PARAMS_VER \
    MAKE_NVAPI_VERSION(NVAPI_VIDSURF_ENCRYPT_CTRL_PARAMS,1)

NVAPI_INTERFACE NvAPI_D3D9_VideoSurfaceEncryptionControl(IDirect3DDevice9 *pDev,
    NVAPI_VIDSURF_ENCRYPT_CTRL_PARAMS *pVidSurfEncrCtrlParams);


///////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME:   NvAPI_D3D9_DMA
//
// DESCRIPTION:     This API lets caller to perform a number of functions related to
//                  the efficient DMA transfer of data between a D3D9 surface and
//                  user-allocated system memory.
//
//  SUPPORTED OS: Windows Vista and higher
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_ERROR - miscellaneous error occurred
//                  NVAPI_INVALID_ARGUMENT: Invalid input parameter.
//                  NVAPI_INCOMPATIBLE_STRUCT_VERSION - the version of
//                      NVAPI_D3D9_DMA_PARAMS struct not supported
//
///////////////////////////////////////////////////////////////////////////////

// D3D9 DMA Commands
typedef enum _NVAPI_D3D9_DMA_COMMANDS
{
    NVAPI_D3D9_DMA_CMD_NULL         = 0x00,             // Null command
    NVAPI_D3D9_DMA_CMD_DESCRIBE     = 0x01,             // Describe allocation requirements
    NVAPI_D3D9_DMA_CMD_MAP          = 0x02,             // Map user-allocated system memory
    NVAPI_D3D9_DMA_CMD_UNMAP        = 0x03,             // Unmap user-allocated system memory
    NVAPI_D3D9_DMA_CMD_REG_EVENT    = 0x04,             // Register user-mode event handle
    NVAPI_D3D9_DMA_CMD_UNREG_EVENT  = 0x05,             // Unregister user-mode event handle
    NVAPI_D3D9_DMA_CMD_TRANSFER     = 0x06,             // Transfer between user system memory and D3D9 surface
} NVAPI_D3D9_DMA_COMMANDS;

// specifies the direction of the DMA transfer
// see NVAPI_D3D9_DMA_TRANSFER_PARAMS for more details
typedef enum _NVAPI_D3D9_DMA_TRANSFER_DIR
{
    NVAPI_D3D9_DMA_TRANSFER_DIR_UPLOAD     = 0x00,      // transfer from user system memory to D3D9 surface
    NVAPI_D3D9_DMA_TRANSFER_DIR_DOWNLOAD   = 0x01,      // transfer from D3D9 surface to user system memory
} NVAPI_D3D9_DMA_TRANSFER_DIR;

// specifies the type of the DMA transfer
// see NVAPI_D3D9_DMA_TRANSFER_PARAMS for more details
typedef enum _NVAPI_D3D9_DMA_TRANSFER_TYPE
{
    NVAPI_D3D9_DMA_TRANSFER_TYPE_PROGRESSIVE    = 0x00, // transfer progressive frame
    NVAPI_D3D9_DMA_TRANSFER_TYPE_INTERLACED     = 0x01, // transfer both fields and keep the result interleaved
    NVAPI_D3D9_DMA_TRANSFER_TYPE_BOB_TOP_FIELD  = 0x02, // perform bob deinterlacing on the top field
    NVAPI_D3D9_DMA_TRANSFER_TYPE_BOB_BTM_FIELD  = 0x03, // perform bob deinterlacing on the bottom field
} NVAPI_D3D9_DMA_TRANSFER_TYPE;

// parameter data structure for the NVAPI_D3D9_DMA_CMD_DESCRIBE command
// given the surface dimension and format as input, this command
// describes the driver requirements (such as alignment) for memory allocation
typedef struct _NVAPI_D3D9_DMA_DESCRIBE_PARAMS
{
    NvU32                       dwWidth;                // width (IN)
    NvU32                       dwHeight;               // height (IN)
    NvU32                       dwFormat;               // FOURCC format (IN)
    NvU32                       dwPitch;                // pitch (OUT)
    NvU32                       dwSize;                 // size (OUT)
    NvU32                       dwAlignment;            // alignment (OUT)
} NVAPI_D3D9_DMA_DESCRIBE_PARAMS;

// parameter data structure for the NVAPI_D3D9_DMA_CMD_MAP command
// maps user-allocated system memory buffer to NV DX object handle
// the pMemory buffer must satisfy the driver requirements
// supported FOURCC formats are: NV12, YV12, YUY2
typedef struct _NVAPI_D3D9_DMA_MAP_PARAMS
{
    NvU32                       dwWidth;                // width (IN)
    NvU32                       dwHeight;               // height (IN)
    NvU32                       dwFormat;               // FOURCC format (IN)
    void                       *pMemory;                // memory pointer (IN)
    NVDX_ObjectHandle           hSysmemSurface;         // sysmem surface handle (OUT)
} NVAPI_D3D9_DMA_MAP_PARAMS;

// parameter data structure for the NVAPI_D3D9_DMA_CMD_UNMAP command
// unmaps user-allocated system memory buffer
typedef struct _NVAPI_D3D9_DMA_UNMAP_PARAMS
{
    NVDX_ObjectHandle           hSysmemSurface;         // sysmem surface handle (IN)
} NVAPI_D3D9_DMA_UNMAP_PARAMS;

// parameter data structure for the NVAPI_D3D9_DMA_CMD_REG_EVENT
// command and the NVAPI_D3D9_DMA_CMD_UNREG_EVENT command
// registers or unregisters a user-mode event handle with the D3D9 device
typedef struct _NVAPI_D3D9_DMA_EVENT_PARAMS
{
    HANDLE                      hCompletionEvent;       // user-mode event handle (IN)
} NVAPI_D3D9_DMA_EVENT_PARAMS;

// parameter data structure for the NVAPI_D3D9_DMA_CMD_TRANSFER command
// performs DMA transfer between system memory buffer and D3D9 surface
// supports both upload and download directions
// supports scaling, color space and pixel format conversion
// a NULL rectangle indicates that the entire surface is used
typedef struct _NVAPI_D3D9_DMA_TRANSFER_PARAMS
{
    NVAPI_D3D9_DMA_TRANSFER_DIR     direction;          // direction of the transfer (IN)
    NVDX_ObjectHandle               hSysmemSurface;     // sysmem surface handle (IN)
    RECT*                           pSysmemSurfaceRect; // sysmem surface rectangle (IN)
    NVDX_ObjectHandle               hD3D9Surface;       // D3D9 surface handle (IN)
    RECT*                           pD3D9SurfaceRect;   // D3D9 surface rectangle (IN)
    HANDLE                          hCompletionEvent;   // completion event handle (IN)
    NVAPI_D3D9_DMA_TRANSFER_TYPE    transferType;       // type of the transfer (IN)
    NvU32                           reserved;           // reserved for future expansion (IN / OUT)
} NVAPI_D3D9_DMA_TRANSFER_PARAMS;

typedef struct _NVAPI_D3D9_DMA_PARAMS
{
    NvU32               version;                        // structure version (IN)
    NvU32               dwCommand;                      // command (IN)
    union
    {
        NVAPI_D3D9_DMA_DESCRIBE_PARAMS     DescribeParams;
        NVAPI_D3D9_DMA_MAP_PARAMS          MapParams;
        NVAPI_D3D9_DMA_UNMAP_PARAMS        UnmapParams;
        NVAPI_D3D9_DMA_EVENT_PARAMS        EventParams;
        NVAPI_D3D9_DMA_TRANSFER_PARAMS     TransferParams;
    };
} NVAPI_D3D9_DMA_PARAMS;

#define NVAPI_D3D9_DMA_PARAMS_VER \
    MAKE_NVAPI_VERSION(NVAPI_D3D9_DMA_PARAMS, 1)

NVAPI_INTERFACE NvAPI_D3D9_DMA(IDirect3DDevice9 *pDev,
    NVAPI_D3D9_DMA_PARAMS *pVideoDMAParams);
        
///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_D3D9_StretchRect
//
// DESCRIPTION:     Copy the contents of the source rectangle to the
//                  destination rectangle.  This function can convert
//                  between a wider range of surfaces than
//                  IDirect3DDevice9::StretchRect.  For example, it can copy
//                  from a depth/stencil surface to a texture.
//
//  SUPPORTED OS: Windows XP and higher
//
// INPUT:           pDev          (IN)     The D3D device that owns the objects.
//                  hSrcObj       (IN)     Handle to the source object.
//                  pSrcRect      (IN)     Defines the rectangle on the source to copy from.  If null, copy from the entire object.
//                  hDstObj       (IN)     Handle to the destination object.
//                  pDstRect      (IN)     Defines the rectangle on the destination to copy to.  If null, copy to the entire object.
//                  Filter        (IN)     Choose a filtering method: D3DTEXF_NONE, D3DTEXF_POINT, D3DTEXF_LINEAR.
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_ERROR - error occurred
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_D3D9_StretchRect(IDirect3DDevice9 *pDev,
                                       NVDX_ObjectHandle hSrcObj,
                                       CONST RECT * pSourceRect,
                                       NVDX_ObjectHandle hDstObj,
                                       CONST RECT * pDestRect,
                                       D3DTEXTUREFILTERTYPE Filter);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_D3D9_CreateRenderTarget
//
// DESCRIPTION:     This API lets the user createrendertargets with multisampling
//                  and also it lets FOURCC formats have multisampling 
//                  feature which is not present in IDirect3DDevice9::CreateRenderTarget.
//  SUPPORTED OS: Windows Vista and higher
//
// INPUT:           pDev              (IN)     The D3D device that owns the objects.
//                  Width             (IN)     Width in Pixels.
//                  Height            (IN)     Height in Pixels.
//                  Format            (IN)     Format of the Rendertarget.
//                  MultiSample       (IN)     Defines the antialiasing type of this RenderTarget
//                  MultisampleQuality(IN)     Quality level.
//                  Lockable          (IN)     Is this RenderTarget User lockable
//                  ppSurface         (OUT)    Address of the Pointer to surface
//                  pHandle           (IN/OUT) Set this to NULL,If non-NULL, fill with the NVDX handle of the created RenderTarget
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_ERROR - error occurred
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_D3D9_CreateRenderTarget(IDirect3DDevice9 *pDev,
                                              UINT Width,
                                              UINT Height,
                                              D3DFORMAT Format,
                                              D3DMULTISAMPLE_TYPE MultiSample,
                                              DWORD MultisampleQuality,
                                              BOOL Lockable,
                                              IDirect3DSurface9** ppSurface,
                                              NVDX_ObjectHandle *pHandle = NULL);

                                       
///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_D3D9_RegisterResource
//
// DESCRIPTION:     To bind a resource (surface/texture) so that it can be retrieved
//                  internally by nvapi.
//
//  SUPPORTED OS: Windows XP and higher
// INPUT:           pUnknown    (IN)    surface/texture
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_ERROR - error occurred
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_D3D9_RegisterResource(IUnknown* pUnknown);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_D3D9_UnregisterResource
//
// DESCRIPTION:     To unbind a resource (surface/texture) after usage.
//
//  SUPPORTED OS: Windows XP and higher
// INPUT:           pUnknown    (IN)    surface/texture
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_ERROR - error occurred
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_D3D9_UnregisterResource(IUnknown* pUnknown);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_D3D9_SetTexture
//
// DESCRIPTION:     The format/usage is similar to IDirect3DDevice9::SetTexture, 
//                  but it would allow you to bind a msaa depthbuffer and 
//                  use as a texture by the pshaders
//
//  SUPPORTED OS: Windows XP and higher
// INPUT:           pDev          (IN)     The D3D device that owns the objects.
//                  uStage        (IN)     The stage/sampler number
//                  pSurface      (IN/OUT) Pointer to a depth stencil surface to bind (cannot be MSAA RenderTarget)
//                  ppTex         (IN/OUT) Pointer to a texture pointer, so that it can return a linked texture
//                  pState        (IN/OUT) pState to describe how to deal with the linked texture and the depth stencil
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_ERROR - error occurred
//
///////////////////////////////////////////////////////////////////////////////
typedef enum {
    NVAPI_SET_DEPTH_TEX_DEFAULT    = 0,
    NVAPI_SET_DEPTH_TEX_DISCONNECT = 1,
    NVAPI_SET_DEPTH_TEX_RECONNECT  = 2
} NVAPI_SET_DEPTH_TEX_STATE;

NVAPI_INTERFACE NvAPI_D3D9_SetTexture(IDirect3DDevice9 *pDev, NvU32 uStage, IDirect3DSurface9 *pSurface, IDirect3DTexture9 **ppTex, NVAPI_SET_DEPTH_TEX_STATE *pState);

#endif //defined(_D3D9_H_) && defined(__cplusplus)

//-----------------------------------------------------------------------------
// DirectX + BDVMA private API
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Direct3D10 APIs
//-----------------------------------------------------------------------------

#if defined(__cplusplus) && defined(__d3d10_h__)

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_D3D10_AliasPrimaryAsTexture
//
//   DESCRIPTION: Create an texture that is an alias of current device's
//                primary surface
//
//  SUPPORTED OS: Windows XP and higher
//
//         INPUT:  pDev        The device to get primary surface on
//                 dwHeadIndex The index to the head to alias.
//
//        OUTPUT:  ppTexture  Fill with the texture created
//
// RETURN STATUS: NVAPI_OK if the texture was successfully created
//                NVAPI_ERROR if the texture could not be created
//
// NOTE: The texture returned is created without any CPU access flags. Locking
//       to read from the texture should be done by creating an second
//       application. Otherwise, this texture can be used as any normal
//       Direct3D texture (it can be blitted from, used as a texture in a
//       pixel shader, etc.)
//
//       This texture is not a render target and cannot be rendered to.
//
//       Before this texture is used, the application must call
//       NvAPI_D3D10_ProcessFlipChainCallbacks to properly update any internal
//       driver state.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_D3D10_AliasPrimaryAsTexture(ID3D10Device *pDev,
                                             NvU32 headIndex,
                                             ID3D10Texture2D **ppTexture);

typedef void (*NVAPI_PRIMARY_FLIP_CHAIN_CALLBACK)(NvU32);

typedef struct
{
    NvU32                     version;              // structure version (IN)

    NVAPI_PRIMARY_FLIP_CHAIN_CALLBACK pPrimaryFlipped; // a callback function to be notified
                                                       // when the primary flip occured.

    NVAPI_PRIMARY_FLIP_CHAIN_CALLBACK pPrimaryFlipChainModified; // a callback function to be notified
                                                                 // when the primary flip chain has
                                                                 // been modified, either due to a new
                                                                 // surface being added, an existing
                                                                 // surface being removed, or the resolution
                                                                 // was changed. In response to this
                                                                 // an application MUST recreate any
                                                                 // aliased primary surfaces with
                                                                 // NvAPI_D3D10_AliasPrimaryAsTexture
                                                                 // as the existing texture will be invalid

    NVAPI_PRIMARY_FLIP_CHAIN_CALLBACK pPrimaryAliasInvalid; // an error occured while using a texture
                                                            // created with NvAPI_D3D10_AliasPrimaryAsTexture
                                                            // The texture needs to be re-created

    NVAPI_PRIMARY_FLIP_CHAIN_CALLBACK pPrimaryAliasOperationDropped; // an operation using the primary alias
                                                                     // was not completed because a flip
                                                                     // happened while the the operation
                                                                     // was in progress. The last operation
                                                                     // may have operated on an incomplete
                                                                     // primary, and should be retried.

    NVAPI_PRIMARY_FLIP_CHAIN_CALLBACK pCursorVisibleUpdated;  // a callback function to be notified
                                                              // when the cursor visibility has been toggled

    NVAPI_PRIMARY_FLIP_CHAIN_CALLBACK pCursorShapeUpdated; // a callback function to be notified
                                                           // when the cursor shape has been updated

} NVAPI_FLIP_CHAIN_CALLBACK_PARAMS;

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_D3D10_SetPrimaryFlipChainCallbacks
//
//   DESCRIPTION: Defines callback functions to receive notification about
//                certain events relating to the primary flip chain.
//
//  SUPPORTED OS: Windows XP and higher
//
//         INPUT:  pDev            The device to receive events on
//                 pCallbackParams A structure containing function
//                                 pointers that will receive the events.
//                                 A pointer may be set to NULL if
//                                 an application does not wish to receive
//                                 the notification.
//
// RETURN STATUS: NVAPI_OK if the events were registered successfully
//
// NOTE: The events will only be registered at this time. To receive the events
//       an application must call NvAPI_D3D10_ProcessFlipChainCallbacks.
//
//       An application may change it's callback functions at any time, and may
//       unregister from any function by passing NULL in as a function pointer.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_D3D10_SetPrimaryFlipChainCallbacks(ID3D10Device *pDev,
                                             const NVAPI_FLIP_CHAIN_CALLBACK_PARAMS* pCallbackParams);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_D3D10_ProcessCallbacks
//
//   DESCRIPTION: Processes pending events
//
//  SUPPORTED OS: Windows XP and higher
//
//         INPUT:  pDev            The device to process events on
//                 dwMilliseconds  The number of milliseconds to sleep while
//                                 waiting for events before returning. This
//                                 may be zero to only process any outstanding
//                                 events, or INFINITE if the application
//                                 wishes to wait forever.
//
// RETURN STATUS: NVAPI_OK if events were properly processed
//
// NOTE: All callback functions are passed the head index for the event.
//
//       This will call the registered callbacks for any events pending. With
//       the exeception of the primary flipped callback, an application cannot
//       "miss" an event if the event were to be notified while the
//       application was not calling ProcessCallbacks. The application
//       will receive this event the first time it calls ProcessCallbacks
//       after the event happened.
//
//       The flipped event is an exception to this rule. This callback will
//       only be fired for flips that occur after the application calls
//       ProcessCallbacks.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_D3D10_ProcessCallbacks(ID3D10Device *pDev,
                                             NvU32 dwMilliseconds);


typedef enum
{
    NV_HW_CURSOR_COLOR_FORMAT_MONOCHROME     = 0,
    NV_HW_CURSOR_COLOR_FORMAT_COLOR          = 1,
    NV_HW_CURSOR_COLOR_FORMAT_MASKED_COLOR   = 2
} NV_HW_CURSOR_COLOR_FORMAT_FLAGS;

typedef struct _NVAPI_RENDERED_CURSOR_BITMAP_DATA
{
    NvU32 version;     // (in) version info
    NvU32 headIndex;   // (in) head index to get cursor on
    NvU32 bufferSize;  // (in) size of raw bitmap data buffer
    PBITMAP pBitmap;   // (in/out) bitmap data for rendered cursor
    NvU32 xHot;        // (out) x value for Hotspot
    NvU32 yHot;        // (out) y value for Hotspot
    NV_HW_CURSOR_COLOR_FORMAT_FLAGS formatFlag; // (out) cursor color format
    NvU32 bVisible;    // (out) cursor visibility (0 = not visible)
} NVAPI_RENDERED_CURSOR_BITMAP_DATA;

#define NVAPI_RENDERED_CURSOR_BITMAP_DATA_VER  MAKE_NVAPI_VERSION(NVAPI_RENDERED_CURSOR_BITMAP_DATA,1)


///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_D3D10_GetRenderedCursorAsBitmap
//
//   DESCRIPTION:  Provide a bitmap of the hardware cursor and the visible
//                 state of the cursor at the time the bitmap is captured.
//
//  SUPPORTED OS: Windows XP and higher
//
//  INPUT:         pDev                       The device to get rendered cursor
//                                            on
//  INPUT/OUTPUT:  pRenderedCursorBitmapData  Structure for input/output with
//                                            fields as follows:
//                 INPUT:
//                 headIndex  The head to get the cursor on
//                 bufferSize The size of the buffer for storing
//                              raw bitmap data
//                 OUTPUT:
//                 pBitmap      Bitmap data for the rendered cursor.  Note
//                              that this result data is undefined in the
//                              case where NVAPI_OK is not the return status
//                 xHot         Location of the X hot spot in the bitmap
//                 yHot         Location of the Y hot spot in the bitmap
//                 formatFlag   Format of cursor (monochrome, color, or
//                              masked color).  For monochrome or masked
//                              color formats, the corresponding bitmap
//                              format is ROP1R5G5B5.  For color format,
//                              the bitmap format is A8R8G8B8.
//                 bVisible     Boolean:  zero=not visible, one=visible
//
//
// RETURN STATUS: NVAPI_OK if the cursor bitmap is successfully returned
//
///////////////////////////////////////////////////////////////////////////////

NVAPI_INTERFACE NvAPI_D3D10_GetRenderedCursorAsBitmap(ID3D10Device *pDev,
                                                      NVAPI_RENDERED_CURSOR_BITMAP_DATA *pRenderedCursorBitmapData);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_D3D10_SetDepthBoundsTest
//
//   DESCRIPTION: This function enables/disables the depth bounds test
//
//  SUPPORTED OS: Windows XP and higher
//
//         INPUT:  pDev         The device to set depth bounds test
//                 bEnable      Enable(non-zero)/disable(zero) the depth bounds test
//                 fMinDepth    The minimum depth for depth bounds test
//                 fMaxDepth    The maximum depth for depth bounds test
//                              The valid values for fMinDepth and fMaxDepth
//                              are such that 0 <= fMinDepth <= fMaxDepth <= 1
//
// RETURN STATUS: NVAPI_OK if the depth bounds test was correcly enabled or
//                disabled
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_D3D10_SetDepthBoundsTest(ID3D10Device *pDev,
                                               NvU32 bEnable,
                                               float fMinDepth,
                                               float fMaxDepth);

#ifdef __d3d10_1_h__

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_D3D10_CreateDevice
//
//   DESCRIPTION: This function creates a d3d10 device. The function call is the
//                same as D3D10CreateDevice1, but with an extra argument
//                (D3D10_FEATURE_LEVEL supported by the device) that the function fills in.
//
//  SUPPORTED OS: Windows XP and higher
//
//         INPUT:  IDXGIAdapter* pAdapter,
//                 D3D10_DRIVER_TYPE DriverType,
//                 HMODULE Software,
//                 UINT32 Flags,
//                 D3D10_FEATURE_LEVEL1 HardwareLevel,
//                 UINT SDKVersion,
//                 ID3D10Device1** ppDevice,
//                 NVAPI_DEVICE_FEATURE_LEVEL *pLevel  //D3D10_FEATURE_LEVEL supported
//
// RETURN STATUS: NVAPI_OK if the createDevice call succeeded.
//
///////////////////////////////////////////////////////////////////////////////
typedef enum
{
    NVAPI_DEVICE_FEATURE_LEVEL_NULL       = -1,
    NVAPI_DEVICE_FEATURE_LEVEL_10_0       = 0,
    NVAPI_DEVICE_FEATURE_LEVEL_10_0_PLUS  = 1,
    NVAPI_DEVICE_FEATURE_LEVEL_10_1       = 2,
    NVAPI_DEVICE_FEATURE_LEVEL_11_0       = 3,
} NVAPI_DEVICE_FEATURE_LEVEL;

NVAPI_INTERFACE NvAPI_D3D10_CreateDevice(IDXGIAdapter* pAdapter,
                                         D3D10_DRIVER_TYPE DriverType,
                                         HMODULE Software,
                                         UINT32 Flags,
                                         D3D10_FEATURE_LEVEL1 HardwareLevel,
                                         UINT SDKVersion,
                                         ID3D10Device1** ppDevice,
                                         NVAPI_DEVICE_FEATURE_LEVEL *pLevel);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_D3D10_CreateDeviceAndSwapChain
//
//   DESCRIPTION: This function creates a d3d10 device and swap chain. The function call is the
//                same as D3D10CreateDeviceAndSwapChain1, but with an extra argument
//                (D3D10_FEATURE_LEVEL supported by the device) that the function fills in .
//
//  SUPPORTED OS: Windows XP and higher
//
//         INPUT:  IDXGIAdapter* pAdapter,
//                 D3D10_DRIVER_TYPE DriverType,
//                 HMODULE Software,
//                 UINT32 Flags,
//                 D3D10_FEATURE_LEVEL1 HardwareLevel,
//                 UINT SDKVersion,
//                 DXGI_SWAP_CHAIN_DESC* pSwapChainDesc,
//                 IDXGISwapChain** ppSwapChain,
//                 ID3D10Device1** ppDevice,
//                 NVAPI_DEVICE_FEATURE_LEVEL *pLevel  //D3D10_FEATURE_LEVEL supported
//
// RETURN STATUS: NVAPI_OK if the createDevice with swap chain call succeeded.
//
///////////////////////////////////////////////////////////////////////////////

NVAPI_INTERFACE NvAPI_D3D10_CreateDeviceAndSwapChain(IDXGIAdapter* pAdapter,
                                                     D3D10_DRIVER_TYPE DriverType,
                                                     HMODULE Software,
                                                     UINT32 Flags,
                                                     D3D10_FEATURE_LEVEL1 HardwareLevel,
                                                     UINT SDKVersion,
                                                     DXGI_SWAP_CHAIN_DESC* pSwapChainDesc,
                                                     IDXGISwapChain** ppSwapChain,
                                                     ID3D10Device1** ppDevice,
                                                     NVAPI_DEVICE_FEATURE_LEVEL *pLevel);

#endif //defined(__d3d10_1_h__)

//-----------------------------------------------------------------------------
// Private Direct3D10 APIs
//-----------------------------------------------------------------------------
#endif // defined(__cplusplus) && defined(__d3d10_h__)

//-----------------------------------------------------------------------------
// Direct3D11 APIs
//-----------------------------------------------------------------------------


#if defined(__cplusplus) && defined(__d3d11_h__)

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_D3D11_CreateDevice
//
//   DESCRIPTION: This function tries to create a d3d11 device. If the call fails (if we are running
//                on pre DX11 HW), depending on the type of HW, it will try to create a DX10.1 OR DX10.0+
//                OR DX10.0 device. The function call is the same as D3D11CreateDevice, but with an extra 
//                argument (D3D_FEATURE_LEVEL supported by the device) that the function fills in. This argument
//                can contain -1 (NVAPI_DEVICE_FEATURE_LEVEL_NULL), if the requested featureLevel is less than dx10.0
//
//  SUPPORTED OS: Windows XP and higher
//
//         INPUT:  IDXGIAdapter* pAdapter,
//                 D3D_DRIVER_TYPE DriverType,
//                 HMODULE Software,
//                 UINT Flags,
//                 CONST D3D_FEATURE_LEVEL *pFeatureLevels,
//                 UINT FeatureLevels,
//                 UINT SDKVersion,
//                 ID3D11Device** ppDevice,
//                 D3D_FEATURE_LEVEL *pFeatureLevel,
//                 ID3D11DeviceContext **ppImmediateContext,
//                 NVAPI_DEVICE_FEATURE_LEVEL *pSupportedLevel  //D3D_FEATURE_LEVEL supported
//
// RETURN STATUS: NVAPI_OK if the createDevice call succeeded.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_D3D11_CreateDevice(IDXGIAdapter* pAdapter,
                                         D3D_DRIVER_TYPE DriverType,
                                         HMODULE Software,
                                         UINT Flags,
                                         CONST D3D_FEATURE_LEVEL *pFeatureLevels,
                                         UINT FeatureLevels,
                                         UINT SDKVersion,
                                         ID3D11Device **ppDevice,
                                         D3D_FEATURE_LEVEL *pFeatureLevel,
                                         ID3D11DeviceContext **ppImmediateContext,
                                         NVAPI_DEVICE_FEATURE_LEVEL *pSupportedLevel);
///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_D3D11_CreateDeviceAndSwapChain
//
//   DESCRIPTION: This function tries to create a d3d11 device and swap chain. If the call fails (if we are 
//                running on pre DX11 HW), depending on the type of HW, it will try to create a DX10.1 OR 
//                DX10.0+ OR DX10.0 device. The function call is the same as D3D11CreateDeviceAndSwapChain,  
//                but with an extra argument (D3D_FEATURE_LEVEL supported by the device) that the function fills
//                in.  This argument can contain -1 (NVAPI_DEVICE_FEATURE_LEVEL_NULL), if the requested featureLevel
//                is less than dx10.0
//
//  SUPPORTED OS: Windows XP and higher
//
//         INPUT:  IDXGIAdapter* pAdapter,
//                 D3D_DRIVER_TYPE DriverType,
//                 HMODULE Software,
//                 UINT Flags,
//                 CONST D3D_FEATURE_LEVEL *pFeatureLevels,
//                 UINT FeatureLevels,
//                 UINT SDKVersion,
//                 CONST DXGI_SWAP_CHAIN_DESC *pSwapChainDesc,
//                 IDXGISwapChain **ppSwapChain,
//                 ID3D11Device** ppDevice,
//                 D3D_FEATURE_LEVEL *pFeatureLevel,
//                 ID3D11DeviceContext **ppImmediateContext,
//                 NVAPI_DEVICE_FEATURE_LEVEL *pSupportedLevel  //D3D_FEATURE_LEVEL supported
//
// RETURN STATUS: NVAPI_OK if the createDevice with swap chain call succeeded.
//
///////////////////////////////////////////////////////////////////////////////

NVAPI_INTERFACE NvAPI_D3D11_CreateDeviceAndSwapChain(IDXGIAdapter* pAdapter,
                                         D3D_DRIVER_TYPE DriverType,
                                         HMODULE Software,
                                         UINT Flags,
                                         CONST D3D_FEATURE_LEVEL *pFeatureLevels,
                                         UINT FeatureLevels,
                                         UINT SDKVersion,
                                         CONST DXGI_SWAP_CHAIN_DESC *pSwapChainDesc,
                                         IDXGISwapChain **ppSwapChain,
                                         ID3D11Device **ppDevice,
                                         D3D_FEATURE_LEVEL *pFeatureLevel,
                                         ID3D11DeviceContext **ppImmediateContext,
                                         NVAPI_DEVICE_FEATURE_LEVEL *pSupportedLevel);

#endif //defined(__cplusplus) && defined(__d3d11_h__)

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GetShaderPipeCount
//
//   DESCRIPTION: Retrieves the number of Shader Pipes on the GPU
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: pCount is NULL
//                NVAPI_OK: *pCount is set
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//                NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE: hPhysicalGpu was not a physical GPU handle
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetShaderPipeCount(NvPhysicalGpuHandle hPhysicalGpu,NvU32 *pCount);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GetShaderSubPipeCount
//
//   DESCRIPTION: Retrieves the number of Shader SubPipes on the GPU
//                On newer architectures, this corresponds to the number of SM units
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: pCount is NULL
//                NVAPI_OK: *pCount is set
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//                NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE: hPhysicalGpu was not a physical GPU handle
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetShaderSubPipeCount(NvPhysicalGpuHandle hPhysicalGpu,NvU32 *pCount);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GetPartitionCount
//
//   DESCRIPTION: Retrieves the number of frame buffer partitions on the GPU
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: pCount is NULL
//                NVAPI_OK: *pCount is set
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//                NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE: hPhysicalGpu was not a physical GPU handle
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetPartitionCount(NvPhysicalGpuHandle hPhysicalGpu,NvU32 *pCount);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GetMemPartitionMask
//
//   DESCRIPTION: Retrieves a 32-bit mask showing which memory partitions are enabled.
//                NvAPI_GPU_GetPartitionCount() returns the count of enabled partitions.
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: pMask is NULL
//                NVAPI_OK: *pMask is set
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//                NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE: hPhysicalGpu was not a physical GPU handle
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetMemPartitionMask(NvPhysicalGpuHandle hPhysicalGpu,NvU32 *pMask);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GetTPCMask
//
//   DESCRIPTION: Retrieves a 32-bit mask showing which TPCs (Texture Processor Cluster)
//                are enabled.  Returns 0 on architectures that don't have TPCs
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: pMask is NULL
//                NVAPI_OK: *pMask is set
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//                NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE: hPhysicalGpu was not a physical GPU handle
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetTPCMask(NvPhysicalGpuHandle hPhysicalGpu,NvU32 *pMask);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GetSMMask
//
//   DESCRIPTION: Retrieves a 32-bit mask showing which SMs
//                (Streaming Multiprocessor) are enabled on the TPC idenified by tpcNdx.
//
//                tpcNdx values start at 0 and correspond to the position of bits
//                returned by NvAPI_GPU_GetTPCMask().  [mask bit = 2^tpcNdx]
//
//                Returns 0 on architectures that don't have SMs
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: pMask is NULL, or tpcId does not match a TPC
//                NVAPI_OK: *pMask is set
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//                NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE: hPhysicalGpu was not a physical GPU handle
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetSMMask(NvPhysicalGpuHandle hPhysicalGpu,NvU32 tpcId,NvU32 *pMask);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GetTotalTPCCount
//
//   DESCRIPTION: Retrieves the total number of enabled TPCs (Texture Processor Cluster)
//                Returns 0 on architectures that don't have TPCs
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: pCount is NULL
//                NVAPI_OK: *pCount is set
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//                NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE: hPhysicalGpu was not a physical GPU handle
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetTotalTPCCount(NvPhysicalGpuHandle hPhysicalGpu,NvU32 *pCount);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GetTotalSMCount
//
//   DESCRIPTION: Retrieves the total number of enabled SMs across all TPCs
//                Returns 0 on architectures that don't have SMs
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: pCount is NULL
//                NVAPI_OK: *pCount is set
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//                NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE: hPhysicalGpu was not a physical GPU handle
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetTotalSMCount(NvPhysicalGpuHandle hPhysicalGpu,NvU32 *pCount);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GetTotalSPCount
//
//   DESCRIPTION: Retrieves the total number of enabled SPs across all SMs on all TPCs
//                Returns 0 on architectures that don't have SPs
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: pCount is NULL
//                NVAPI_OK: *pCount is set
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//                NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE: hPhysicalGpu was not a physical GPU handle
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetTotalSPCount(NvPhysicalGpuHandle hPhysicalGpu,NvU32 *pCount);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GetAllOutputs
//
//   DESCRIPTION: Returns set of all GPU-output identifiers as a bitmask.
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: hPhysicalGpu or pOutputsMask is NULL
//                NVAPI_OK: *pOutputsMask contains a set of GPU-output identifiers
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//                NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE: hPhysicalGpu was not a physical GPU handle
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetAllOutputs(NvPhysicalGpuHandle hPhysicalGpu,NvU32 *pOutputsMask);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GetConnectedOutputs
//
//   DESCRIPTION: Same as NvAPI_GPU_GetAllOutputs but returns only the set of GPU-output
//                identifiers that are connected to display devices.
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: hPhysicalGpu or pOutputsMask is NULL
//                NVAPI_OK: *pOutputsMask contains a set of GPU-output identifiers
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//                NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE: hPhysicalGpu was not a physical GPU handle
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetConnectedOutputs(NvPhysicalGpuHandle hPhysicalGpu, NvU32 *pOutputsMask);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GetConnectedSLIOutputs
//
//   DESCRIPTION: Same as NvAPI_GPU_GetConnectedOutputs but returns only the set of GPU-output
//                identifiers that can be selected in an SLI configuration. 
//                NOTE: This function matches NvAPI_GPU_GetConnectedOutputs
//                 - On systems which are not SLI capable.
//                 - If the queried GPU is not part of a valid SLI group.
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: hPhysicalGpu or pOutputsMask is NULL
//                NVAPI_OK: *pOutputsMask contains a set of GPU-output identifiers
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//                NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE: hPhysicalGpu was not a physical GPU handle
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetConnectedSLIOutputs(NvPhysicalGpuHandle hPhysicalGpu, NvU32 *pOutputsMask);


///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GetConnectedOutputsWithLidState
//
//   DESCRIPTION: Similar to NvAPI_GPU_GetConnectedOutputs this API returns the connected display identifiers that are connected
//                as a output mask but unlike NvAPI_GPU_GetConnectedOutputs this API "always" reflects the Lid State in the output mask.
//                Thus if you expect the LID close state to be available in the connection mask use this API.
//                If LID is closed then this API will remove the LID panel from the connected display identifiers.
//                If LID is open then this API will reflect the LID panel in the connected display identifiers.
//                Note:This API should be used on laptop systems and on systems where LID state is required in the connection output mask.
//                     On desktop systems the returned identifiers will match NvAPI_GPU_GetConnectedOutputs.
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: hPhysicalGpu or pOutputsMask is NULL
//                NVAPI_OK: *pOutputsMask contains a set of GPU-output identifiers
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//                NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE: hPhysicalGpu was not a physical GPU handle
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetConnectedOutputsWithLidState(NvPhysicalGpuHandle hPhysicalGpu, NvU32 *pOutputsMask);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GetConnectedSLIOutputsWithLidState
//
//   DESCRIPTION: Same as NvAPI_GPU_GetConnectedOutputsWithLidState but returns only the set of GPU-output
//                identifiers that can be selected in an SLI configuration. With SLI disabled
//                this function matches NvAPI_GPU_GetConnectedOutputsWithLidState
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: hPhysicalGpu or pOutputsMask is NULL
//                NVAPI_OK: *pOutputsMask contains a set of GPU-output identifiers
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//                NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE: hPhysicalGpu was not a physical GPU handle
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetConnectedSLIOutputsWithLidState(NvPhysicalGpuHandle hPhysicalGpu, NvU32 *pOutputsMask);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GetSystemType
//
//   DESCRIPTION: Returns information to identify if the GPU type is for a laptop system or a desktop system.
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: hPhysicalGpu or pOutputsMask is NULL
//                NVAPI_OK: *pSystemType contains the GPU system type
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//                NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE: hPhysicalGpu was not a physical GPU handle
//
///////////////////////////////////////////////////////////////////////////////
typedef enum
{
    NV_SYSTEM_TYPE_UNKNOWN = 0,
    NV_SYSTEM_TYPE_LAPTOP  = 1,
    NV_SYSTEM_TYPE_DESKTOP = 2,

} NV_SYSTEM_TYPE;

NVAPI_INTERFACE NvAPI_GPU_GetSystemType(NvPhysicalGpuHandle hPhysicalGpu, NV_SYSTEM_TYPE *pSystemType);


///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GetActiveOutputs
//
//   DESCRIPTION: Same as NvAPI_GPU_GetAllOutputs but returns only the set of GPU-output
//                identifiers that are actively driving display devices.
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: hPhysicalGpu or pOutputsMask is NULL
//                NVAPI_OK: *pOutputsMask contains a set of GPU-output identifiers
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//                NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE: hPhysicalGpu was not a physical GPU handle
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetActiveOutputs(NvPhysicalGpuHandle hPhysicalGpu, NvU32 *pOutputsMask);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GetEDID
//
//   DESCRIPTION: Returns the EDID data for the specified GPU handle and connection bit mask.
//                displayOutputId should have exactly 1 bit set to indicate a single display.
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: pEDID is NULL; displayOutputId has 0 or > 1 bits set.
//                NVAPI_OK: *pEDID contains valid data.
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found.
//                NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE: hPhysicalGpu was not a physical GPU handle.
//                NVAPI_DATA_NOT_FOUND: requested display does not contain an EDID
//
///////////////////////////////////////////////////////////////////////////////
#define NV_EDID_V1_DATA_SIZE   256
#define NV_EDID_DATA_SIZE      NV_EDID_V1_DATA_SIZE

typedef struct
{
    NvU32   version;        //structure version
    NvU8    EDID_Data[NV_EDID_DATA_SIZE];
    NvU32   sizeofEDID;
} NV_EDID;

#define NV_EDID_VER         MAKE_NVAPI_VERSION(NV_EDID,2)

NVAPI_INTERFACE NvAPI_GPU_GetEDID(NvPhysicalGpuHandle hPhysicalGpu, NvU32 displayOutputId, NV_EDID *pEDID);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_SetEDID
//
//   DESCRIPTION: Sets the EDID data for the specified GPU handle and connection bit mask.
//                displayOutputId should have exactly 1 bit set to indicate a single display.
//                Note:The EDID will be cached for the current boot session and will be enumerated to the OS in this call.
//                     To remove the EDID set the sizeofEDID to zero.
//                     OS and NVAPI connection status APIs will reflect the newly set or removed EDID dynamically.
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: pEDID is NULL; displayOutputId has 0 or > 1 bits set
//                NVAPI_OK: *pEDID data was applied to the requested displayOutputId.
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found.
//                NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE: hPhysicalGpu was not a physical GPU handle.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_SetEDID(NvPhysicalGpuHandle hPhysicalGpu, NvU32 displayOutputId, NV_EDID *pEDID);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GetOutputType
//
//   DESCRIPTION: Give a physical GPU handle and a single outputId (exactly 1 bit set), this API
//                returns the output type.
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: hPhysicalGpu, outputId or pOutputsMask is NULL; or outputId has > 1 bit set
//                NVAPI_OK: *pOutputType contains a NvGpuOutputType value
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//                NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE: hPhysicalGpu was not a physical GPU handle
//
///////////////////////////////////////////////////////////////////////////////
typedef enum _NV_GPU_OUTPUT_TYPE
{
    NVAPI_GPU_OUTPUT_UNKNOWN  = 0,
    NVAPI_GPU_OUTPUT_CRT      = 1,     // CRT display device
    NVAPI_GPU_OUTPUT_DFP      = 2,     // Digital Flat Panel display device
    NVAPI_GPU_OUTPUT_TV       = 3,     // TV display device
} NV_GPU_OUTPUT_TYPE;

NVAPI_INTERFACE NvAPI_GPU_GetOutputType(NvPhysicalGpuHandle hPhysicalGpu, NvU32 outputId, NV_GPU_OUTPUT_TYPE *pOutputType);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GetDeviceDisplayMode
//
//   DESCRIPTION: Give a physical GPU handle and a single active displayId (exactly 1 bit set), this API
//                returns the device display mode according to hardware (including raster extension). This
//                includes the backend timing info.
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: hPhysicalGpu, displayId or pOutputsMask is NULL; displayId has > 1 bit set;
//                                        displayId is not an active display.
//                NVAPI_OK: *pDeviceDisplayMode contains the returned display mode information.
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//                NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE: hPhysicalGpu was not a physical GPU handle
//
///////////////////////////////////////////////////////////////////////////////
typedef struct
{
    NvU32 version;          // structure version
    NvU32 activeWidth;      // Number of active horizontal pixels
    NvU32 activeHeight;     // Number of active vertical pixels
    NvU32 totalWidth;        // Total size of raster width (including blanking)
    NvU32 totalHeight;      // Total size of raster height (including blanking)
    NvU32 depth;            // Color depth
    NvU32 frequency;        // Calculated refresh rate based upon current raster and pixel clock
} NV_GPU_DISPLAY_MODE;

#define NV_GPU_DISPLAY_MODE_VER  MAKE_NVAPI_VERSION(NV_GPU_DISPLAY_MODE,1)

NVAPI_INTERFACE NvAPI_GPU_GetDeviceDisplayMode(NvPhysicalGpuHandle hPhysicalGpu, NvU32 displayId, NV_GPU_DISPLAY_MODE *pDeviceDisplayMode);

//DISPLAYPORT is all private for now. Do not change category until that info is public.
///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GetFlatPanelInfo
//
//   DESCRIPTION: Given a physical GPU handle and a single display outputId of the flat panel, this API
//                returns the flat panel attributes.
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: hPhysicalGpu, outputId or pFlatPanelInfo is NULL; or outputId has > 1 bit set.
//                NVAPI_OK: *pFlatPanelInfo contains the returned flat panel information.
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found.
//                NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE: hPhysicalGpu was not a physical GPU handle.
//                NVAPI_EXPECTED_DIGITAL_FLAT_PANEL: outputId is not associated with digital flat panel.
//
///////////////////////////////////////////////////////////////////////////////

typedef enum _NV_GPU_FLATPANEL_SIGNAL_TYPE
{
  NV_GPU_FLATPANEL_SIGNAL_TYPE_TMDS         = 0,
  NV_GPU_FLATPANEL_SIGNAL_TYPE_LVDS         = 1,
  NV_GPU_FLATPANEL_SIGNAL_TYPE_SDI          = 2,
  NV_GPU_FLATPANEL_SIGNAL_TYPE_DISPLAYPORT  = 3,

} NV_GPU_FLATPANEL_SIGNAL_TYPE;

typedef enum _NV_GPU_FLATPANEL_SIGNAL_LINK
{
  NV_GPU_FLATPANEL_SIGNAL_SINGLE_LINK       = 1,
  NV_GPU_FLATPANEL_SIGNAL_DUAL_LINK         = 2,

} NV_GPU_FLATPANEL_SIGNAL_LINK;

typedef enum _NV_GPU_FLATPANEL_SIGNAL_DISPLAYPORT_LANES
{
  NV_GPU_FLATPANEL_SIGNAL_DISPLAYPORT_NONE              = 0,
  NV_GPU_FLATPANEL_SIGNAL_DISPLAYPORT_SINGLE_LANE       = 1,
  NV_GPU_FLATPANEL_SIGNAL_DISPLAYPORT_DUAL_LANE         = 2,
  NV_GPU_FLATPANEL_SIGNAL_DISPLAYPORT_QUAD_LANE         = 3,

} NV_GPU_FLATPANEL_SIGNAL_DISPLAYPORT_LANES;

typedef enum _NV_GPU_FLATPANEL_SIGNAL_COLOR_FLAGS
{
  NV_GPU_FLATPANEL_SIGNAL_COLOR_LIMITED_RANGE           = 0x00000001,   //flatpanel supports limited color range.
  NV_GPU_FLATPANEL_SIGNAL_COLOR_AUTO_CONFIGURE          = 0x00000002,   //flatpanel supports auto-configuring the color range.
  NV_GPU_FLATPANEL_SIGNAL_COLOR_FORMAT_YCBCR422_CAPABLE = 0x00000004,   //flatpanel is YCBCR422 color format capable.
  NV_GPU_FLATPANEL_SIGNAL_COLOR_FORMAT_YCBCR444_CAPABLE = 0x00000008,   //flatpanel is YCBCR444 color format capable.

} NV_GPU_FLATPANEL_SIGNAL_COLOR_FLAGS;

typedef struct
{
    NvU32                                       version;                    //structure version
    NV_GPU_FLATPANEL_SIGNAL_TYPE                signalType;                 //flat panel signal type
    NV_GPU_FLATPANEL_SIGNAL_LINK                linkType;                   //link type
    NV_GPU_FLATPANEL_SIGNAL_DISPLAYPORT_LANES   displayPortLanes;           //1, 2 or 4 lanes in case of displayport connected panel.
    NvU32                                       colorFlags;                 //one or more bits from NV_GPU_FLATPANEL_SIGNAL_COLOR_FLAGS
    NvU32                                       hdmiCapable:1;              //hdmi status
    NvU32                                       scalerDisabled:1;           //indicates if GPU scaling is disabled(possible with SLI active on certain GPUs)
    NvU32                                       refreshRateLocked:1;        //flat panel supports 60hz only
}
NV_GPU_FLAT_PANEL_INFO;

#define NV_GPU_FLAT_PANEL_INFO_VER  MAKE_NVAPI_VERSION(NV_GPU_FLAT_PANEL_INFO,1)

NVAPI_INTERFACE NvAPI_GPU_GetFlatPanelInfo(NvPhysicalGpuHandle hPhysicalGpu, NvU32 outputId, NV_GPU_FLAT_PANEL_INFO *pFlatPanelInfo);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_ValidateOutputCombination
//
//   DESCRIPTION: This call is used to determine if a set of GPU outputs can be active
//                simultaneously.  While a GPU may have <n> outputs, they can not typically
//                all be active at the same time due to internal resource sharing.
//
//                Given a physical GPU handle and a mask of candidate outputs, this call
//                will return NVAPI_OK if all of the specified outputs can be driven
//                simultaneously.  It will return NVAPI_INVALID_COMBINATION if they cannot.
//
//                Use NvAPI_GPU_GetAllOutputs() to determine which outputs are candidates.
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS: NVAPI_OK: combination of outputs in outputsMask are valid (can be active simultaneously)
//                NVAPI_INVALID_COMBINATION: combination of outputs in outputsMask are NOT valid
//                NVAPI_INVALID_ARGUMENT: hPhysicalGpu or outputsMask does not have at least 2 bits set
//                NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE: hPhysicalGpu was not a physical GPU handle
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_ValidateOutputCombination(NvPhysicalGpuHandle hPhysicalGpu, NvU32 outputsMask);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GetConnectorInfo
//
//   DESCRIPTION: Given a physical GPU handle and a single outputId (exactly 1 bit set),
//                this API fills the NV_GPU_CONNECTOR_INFO with connector specific data.
//                Note:If outputId is a connected or active then the current attached connector information is returned.
//                If there is no connector attached for the outputId then all possible connections on the board are returned.
//                Some TV outputs may have multiple connectors attached or it could have ambiguous connector layout on the board.
//                In that case the connector[] array will list all connectors but doesn't
//                indicate which one is 'active'. To get the active TV connector use NvAPI_GetTVOutputInfo.
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: hPhysicalGpu, outputId or pOutputsMask is NULL; or outputId has > 1 bit set
//                NVAPI_OK: *pConnectorInfo contains valid NV_GPU_CONNECTOR_INFO data
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//                NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE: hPhysicalGpu was not a physical GPU handle
//                NVAPI_INCOMPATIBLE_STRUCT_VERSION - NV_GPU_CONNECTOR_INFO version not compatible with driver
//
///////////////////////////////////////////////////////////////////////////////

typedef enum _NV_GPU_CONNECTOR_TYPE
{
    NVAPI_GPU_CONNECTOR_VGA_15_PIN                      = 0x00000000,
    NVAPI_GPU_CONNECTOR_TV_COMPOSITE                    = 0x00000010,
    NVAPI_GPU_CONNECTOR_TV_SVIDEO                       = 0x00000011,
    NVAPI_GPU_CONNECTOR_TV_HDTV_COMPONENT               = 0x00000013,
    NVAPI_GPU_CONNECTOR_TV_SCART                        = 0x00000014,
    NVAPI_GPU_CONNECTOR_TV_COMPOSITE_SCART_ON_EIAJ4120  = 0x00000016,
    NVAPI_GPU_CONNECTOR_TV_HDTV_EIAJ4120                = 0x00000017,
    NVAPI_GPU_CONNECTOR_PC_POD_HDTV_YPRPB               = 0x00000018,
    NVAPI_GPU_CONNECTOR_PC_POD_SVIDEO                   = 0x00000019,
    NVAPI_GPU_CONNECTOR_PC_POD_COMPOSITE                = 0x0000001A,
    NVAPI_GPU_CONNECTOR_DVI_I_TV_SVIDEO                 = 0x00000020,
    NVAPI_GPU_CONNECTOR_DVI_I_TV_COMPOSITE              = 0x00000021,
    NVAPI_GPU_CONNECTOR_DVI_I                           = 0x00000030,
    NVAPI_GPU_CONNECTOR_DVI_D                           = 0x00000031,
    NVAPI_GPU_CONNECTOR_ADC                             = 0x00000032,
    NVAPI_GPU_CONNECTOR_LFH_DVI_I_1                     = 0x00000038,
    NVAPI_GPU_CONNECTOR_LFH_DVI_I_2                     = 0x00000039,
    NVAPI_GPU_CONNECTOR_SPWG                            = 0x00000040,
    NVAPI_GPU_CONNECTOR_OEM                             = 0x00000041,
    NVAPI_GPU_CONNECTOR_DISPLAYPORT_EXTERNAL            = 0x00000046,
    NVAPI_GPU_CONNECTOR_DISPLAYPORT_INTERNAL            = 0x00000047,
    NVAPI_GPU_CONNECTOR_HDMI_A                          = 0x00000061,
    NVAPI_GPU_CONNECTOR_UNKNOWN                         = 0xFFFFFFFF,
} NV_GPU_CONNECTOR_TYPE;

typedef enum _NV_GPU_CONNECTOR_PLATFORM
{
    NVAPI_GPU_CONNECTOR_PLATFORM_DEFAULT_ADD_IN_CARD   = 0x00000000,
    NVAPI_GPU_CONNECTOR_PLATFORM_TWO_PLATE_ADD_IN_CARD = 0x00000001,
    NVAPI_GPU_CONNECTOR_PLATFORM_MOBILE_ADD_IN_CARD    = 0x00000008,
    NVAPI_GPU_CONNECTOR_PLATFORM_MOBILE_BACK           = 0x00000010,
    NVAPI_GPU_CONNECTOR_PLATFORM_MOBILE_BACK_LEFT      = 0x00000011,
    NVAPI_GPU_CONNECTOR_PLATFORM_MOBILE_BACK_DOCK      = 0x00000018,
    NVAPI_GPU_CONNECTOR_PLATFORM_MAINBOARD_DEFAULT     = 0x00000020,
    NVAPI_GPU_CONNECTOR_PLATFORM_UNKNOWN               = 0xFFFFFFFF,
} NV_GPU_CONNECTOR_PLATFORM;

typedef struct
{
    NV_GPU_CONNECTOR_TYPE   type;           // connector type
    NvU32                   locationIndex;  // connector location
} NV_GPU_CONNECTOR_DATA;

#define NV_API_MAX_CONNECTOR_PER_OUTPUT     4

typedef struct
{
    NvU32                       version;            // structure version
    NV_GPU_CONNECTOR_PLATFORM   connectorPlatform;  // connector platform
    NvU32                       connectorCount;     // num valid entries in connector[]
    NV_GPU_CONNECTOR_DATA       connector[NV_API_MAX_CONNECTOR_PER_OUTPUT];
} NV_GPU_CONNECTOR_INFO;

#define NV_GPU_CONNECTOR_INFO_VER  MAKE_NVAPI_VERSION(NV_GPU_CONNECTOR_INFO,1)

NVAPI_INTERFACE NvAPI_GPU_GetConnectorInfo(NvPhysicalGpuHandle hPhysicalGpu, NvU32 outputId, NV_GPU_CONNECTOR_INFO *pConnectorInfo);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GetFullName
//
//   DESCRIPTION: Retrieves the full GPU name as an ascii string.  Eg: "Quadro FX 1400"
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// RETURN STATUS: NVAPI_ERROR or NVAPI_OK
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetFullName(NvPhysicalGpuHandle hPhysicalGpu, NvAPI_ShortString szName);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GetPCIIdentifiers
//
//   DESCRIPTION: Returns the PCI identifiers associated with this GPU.
//                  DeviceId - the internal PCI device identifier for the GPU.
//                  SubSystemId - the internal PCI subsystem identifier for the GPU.
//                  RevisionId - the internal PCI device-specific revision identifier for the GPU.
//                  ExtDeviceId - the external PCI device identifier for the GPU.
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: hPhysicalGpu or an argument is NULL
//                NVAPI_OK: arguments are populated with PCI identifiers
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//                NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE: hPhysicalGpu was not a physical GPU handle
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetPCIIdentifiers(NvPhysicalGpuHandle hPhysicalGpu,NvU32 *pDeviceId,NvU32 *pSubSystemId,NvU32 *pRevisionId,NvU32 *pExtDeviceId);

typedef enum _NV_GPU_TYPE
{
    NV_SYSTEM_TYPE_GPU_UNKNOWN     = 0,
    NV_SYSTEM_TYPE_IGPU            = 1, //integrated
    NV_SYSTEM_TYPE_DGPU            = 2, //discrete
} NV_GPU_TYPE;

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GetGPUType
//
// DESCRIPTION: Returns information to identify the GPU type
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: hPhysicalGpu
// NVAPI_OK: *pGpuType contains the GPU type
// NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
// NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE: hPhysicalGpu was not a physical GPU handle
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetGPUType(NvPhysicalGpuHandle hPhysicalGpu, NV_GPU_TYPE *pGpuType);

typedef enum _NV_GPU_BUS_TYPE
{
    NVAPI_GPU_BUS_TYPE_UNDEFINED    = 0,
    NVAPI_GPU_BUS_TYPE_PCI          = 1,
    NVAPI_GPU_BUS_TYPE_AGP          = 2,
    NVAPI_GPU_BUS_TYPE_PCI_EXPRESS  = 3,
    NVAPI_GPU_BUS_TYPE_FPCI         = 4,
} NV_GPU_BUS_TYPE;

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GetBusType
//
//   DESCRIPTION: Returns the type of bus associated with this GPU.
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: hPhysicalGpu or pBusType is NULL
//                NVAPI_OK: *pBusType contains bus identifier
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//                NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE: hPhysicalGpu was not a physical GPU handle
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetBusType(NvPhysicalGpuHandle hPhysicalGpu,NV_GPU_BUS_TYPE *pBusType);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GetBusId
//
//   DESCRIPTION: Returns the ID of bus associated with this GPU.
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: hPhysicalGpu or pBusId is NULL
//                NVAPI_OK: *pBusId contains bus id
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//                NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE: hPhysicalGpu was not a physical GPU handle
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetBusId(NvPhysicalGpuHandle hPhysicalGpu, NvU32 *pBusId);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GetBusSlotId
//
//   DESCRIPTION: Returns the ID of bus-slot associated with this GPU.
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: hPhysicalGpu or pBusSlotId is NULL
//                NVAPI_OK: *pBusSlotId contains bus-slot id
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//                NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE: hPhysicalGpu was not a physical GPU handle
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetBusSlotId(NvPhysicalGpuHandle hPhysicalGpu, NvU32 *pBusSlotId);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GetIRQ
//
//   DESCRIPTION: Returns the interrupt number associated with this GPU.
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: hPhysicalGpu or pIRQ is NULL
//                NVAPI_OK: *pIRQ contains interrupt number
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//                NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE: hPhysicalGpu was not a physical GPU handle
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetIRQ(NvPhysicalGpuHandle hPhysicalGpu,NvU32 *pIRQ);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GetVbiosRevision
//
//   DESCRIPTION: Returns the revision of the video bios associated with this GPU.
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: hPhysicalGpu or pBiosRevision is NULL
//                NVAPI_OK: *pBiosRevision contains revision number
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//                NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE: hPhysicalGpu was not a physical GPU handle
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetVbiosRevision(NvPhysicalGpuHandle hPhysicalGpu,NvU32 *pBiosRevision);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GetVbiosOEMRevision
//
//   DESCRIPTION: Returns the OEM revision of the video bios associated with this GPU.
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: hPhysicalGpu or pBiosRevision is NULL
//                NVAPI_OK: *pBiosRevision contains revision number
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//                NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE: hPhysicalGpu was not a physical GPU handle
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetVbiosOEMRevision(NvPhysicalGpuHandle hPhysicalGpu,NvU32 *pBiosRevision);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GetVbiosVersionString
//
//   DESCRIPTION: Returns the full bios version string in the form of xx.xx.xx.xx.yy where
//                the xx numbers come from NvAPI_GPU_GetVbiosRevision and yy comes from
//                NvAPI_GPU_GetVbiosOEMRevision.
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: hPhysicalGpu is NULL
//                NVAPI_OK: szBiosRevision contains version string
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//                NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE: hPhysicalGpu was not a physical GPU handle
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetVbiosVersionString(NvPhysicalGpuHandle hPhysicalGpu,NvAPI_ShortString szBiosRevision);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GetAGPAperture
//
//   DESCRIPTION: Returns AGP aperture in megabytes
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: pSize is NULL
//                NVAPI_OK: call successful
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//                NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE: hPhysicalGpu was not a physical GPU handle
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetAGPAperture(NvPhysicalGpuHandle hPhysicalGpu,NvU32 *pSize);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GetCurrentAGPRate
//
//   DESCRIPTION: Returns the current AGP Rate (1 = 1x, 2=2x etc, 0 = AGP not present)
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: pRate is NULL
//                NVAPI_OK: call successful
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//                NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE: hPhysicalGpu was not a physical GPU handle
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetCurrentAGPRate(NvPhysicalGpuHandle hPhysicalGpu,NvU32 *pRate);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GetCurrentPCIEDownstreamWidth
//
//   DESCRIPTION: Returns the number of PCIE lanes being used for the PCIE interface
//                downstream from the GPU.
//
//                On systems that do not support PCIE, the maxspeed for the root link
//                will be zero.
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: pWidth is NULL
//                NVAPI_OK: call successful
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//                NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE: hPhysicalGpu was not a physical GPU handle
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetCurrentPCIEDownstreamWidth(NvPhysicalGpuHandle hPhysicalGpu,NvU32 *pWidth);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GetPhysicalFrameBufferSize
//
//   DESCRIPTION: Returns the physical size of framebuffer in Kb.  This does NOT include any
//                system RAM that may be dedicated for use by the GPU.
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: pSize is NULL
//                NVAPI_OK: call successful
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//                NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE: hPhysicalGpu was not a physical GPU handle
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetPhysicalFrameBufferSize(NvPhysicalGpuHandle hPhysicalGpu,NvU32 *pSize);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GetVirtualFrameBufferSize
//
//   DESCRIPTION: Returns the virtual size of framebuffer in Kb.  This includes the physical RAM plus any
//                system RAM that has been dedicated for use by the GPU.
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: pSize is NULL
//                NVAPI_OK: call successful
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//                NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE: hPhysicalGpu was not a physical GPU handle
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetVirtualFrameBufferSize(NvPhysicalGpuHandle hPhysicalGpu,NvU32 *pSize);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GetQuadroStatus
//
//   DESCRIPTION: Retrieves the Quadro status for the GPU (1 if Quadro, 0 if GeForce)
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// RETURN STATUS: NVAPI_ERROR or NVAPI_OK
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetQuadroStatus(NvPhysicalGpuHandle hPhysicalGpu, NvU32 *pStatus);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GetFBWidthAndLocation
//
//   DESCRIPTION: Returns the width and location of the GPU's RAM memory bus.
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS: NVAPI_ERROR or NVAPI_OK
//
///////////////////////////////////////////////////////////////////////////////
typedef enum _NV_GPU_FB_LOCATION
{
    NV_GPU_FB_RAM_LOCATION_UNKNOWN       = 0,
    NV_GPU_FB_RAM_LOCATION_GPU_DEDICATED = 1,
    NV_GPU_FB_RAM_LOCATION_SYS_SHARED    = 2,
    NV_GPU_FB_RAM_LOCATION_SYS_DEDICATED = 3,
} NV_GPU_FB_LOCATION;
NVAPI_INTERFACE NvAPI_GPU_GetFBWidthAndLocation(NvPhysicalGpuHandle hPhysicalGpu, NvU32 *pBusWidth, NV_GPU_FB_LOCATION *pFBLocation);

///////////////////////////////////////////////////////////////////////////////
//
//  GPU Clock Control
//
//  These APIs allow the user to get and set individual clock domains
//  on a per-GPU basis.
//
///////////////////////////////////////////////////////////////////////////////
#define NVAPI_MAX_GPU_CLOCKS 32

typedef enum _NV_GPU_CLOCK_INFO_DOMAIN_ID
{
    NVAPI_GPU_CLOCK_INFO_DOMAIN_NV      = 0,
    NVAPI_GPU_CLOCK_INFO_DOMAIN_M       = 4,
    NVAPI_GPU_CLOCK_INFO_DOMAIN_UNDEFINED = NVAPI_MAX_GPU_CLOCKS,
} NV_GPU_CLOCK_INFO_DOMAIN_ID;


//Performance table overclocking

typedef enum _NV_GPU_PERF_CLOCK_DOMAIN_ID
{
    NVAPI_GPU_PERF_CLOCK_DOMAIN_NV      = 0,
    NVAPI_GPU_PERF_CLOCK_DOMAIN_M       = 4,
    NVAPI_GPU_PERF_CLOCK_DOMAIN_HOTCLK  = 7,

} NV_GPU_PERF_CLOCK_DOMAIN_ID;

#define NVAPI_MAX_GPU_PERF_CLOCKS       32

#define NVAPI_MAX_PERF_CLOCK_LEVELS     12
#define NVAPI_TARGET_ALL_PERF_LEVELS    0xffffffff

#define NV_PERF_CLOCK_LEVEL_STATE_DEFAULT                   0x00000000 //level is in its default state
#define NV_PERF_CLOCK_LEVEL_STATE_OVERCLOCKED               0x00000001 //level is overclocked
#define NV_PERF_CLOCK_LEVEL_STATE_DESKTOP                   0x00000002 //2d desktop perf level
#define NV_PERF_CLOCK_LEVEL_STATE_PERFORMANCE               0x00000004 //3d applications perf level
#define NV_PERF_CLOCK_LEVEL_STATE_TEST                      0x00000008 //test the new clocks for this level. Does not apply.
#define NV_PERF_CLOCK_LEVEL_STATE_TEST_SUCCESS              0x00000010 //test result

#define NV_PERF_CLOCK_GPU_STATE_DEFAULT                     0x00000000 //default state
#define NV_PERF_CLOCK_GPU_STATE_DYNAMIC_SUPPORTED           0x00000001 //gpu supports dynamic performance level transitions
#define NV_PERF_CLOCK_GPU_STATE_DESKTOP                     0x00000002 //gpu in desktop level
#define NV_PERF_CLOCK_GPU_STATE_PERFORMANCE                 0x00000004 //gpu in performance level
#define NV_PERF_CLOCK_GPU_STATE_ACTIVE_CLOCKING_SUPPORTED   0x00000008 //active clocking supported
#define NV_PERF_CLOCK_GPU_STATE_ACTIVE_CLOCKING_ENABLE      0x00000010 //enable active clocking
#define NV_PERF_CLOCK_GPU_STATE_ACTIVE_CLOCKING_DISABLE     0x00000020 //disable active clocking
#define NV_PERF_CLOCK_GPU_STATE_MEMCLK_CONTROL_DISABLED     0x00000040 //memory clock control disabled
#define NV_PERF_CLOCK_GPU_STATE_GFXCLK_CONTROL_DISABLED     0x00000080 //core clock control disabled
#define NV_PERF_CLOCK_GPU_STATE_SET_DEFERRED                0x00000100 //No immediate perf transitions. Deferred until perf triggers kick in.
#define NV_PERF_CLOCK_GPU_STATE_TESTING_CLOCKS_SUPPORTED    0x00000200 //testing clocks supported

typedef struct
{
    NvU32   version;                                //IN perf clock table version
    NvU32   levelCount;                             //number of the performance levels. count increases everytime a level is overclocked
    NvU32   gpuPerflevel;                           //OUT the current perf level. This is a dynamic level which can possibly change on every call
    NvU32   domainCount;                            //IN/OUT number of domains
    NvU32   gpuPerfFlags;                           //IN/OUT gpu flags - one of the flags defined in NV_PERF_CLOCK_GPU_STATE
    struct
    {
        NvU32   level;                              //IN/OUT performance level indicator, range 0 to levelCount - 1.
        NvU32   flags;                              //IN/OUT per level flags - one of more flags defined in NV_PERF_CLOCK_LEVEL_STATE
        struct
        {
            NV_GPU_PERF_CLOCK_DOMAIN_ID domainId;       //IN/OUT current domain indicator - one of the ids from NV_GPU_PERF_CLOCK_DOMAIN_ID
            NvU32                       domainFlags;    //reserved unused domain flags
            NvU32                       currentFreq;    //IN/OUT current clock KHz
            NvU32                       defaultFreq;    //default clock KHz
            NvU32                       minFreq;        //min KHz
            NvU32                       maxFreq;        //max KHz
            NvU32                       bSetClock:1;    //IN if set during NvAPI_GPU_SetPerfClocks call, this domain currentFreq will be applied
        } domain[NVAPI_MAX_GPU_PERF_CLOCKS];
    } perfLevel[NVAPI_MAX_PERF_CLOCK_LEVELS];

} NV_GPU_PERF_CLOCK_TABLE;

#define NV_GPU_PERF_CLOCK_TABLE_VER  MAKE_NVAPI_VERSION(NV_GPU_PERF_CLOCK_TABLE,1)

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_GPU_GetPerfClocks
//
// DESCRIPTION:     Retrieves the performance clock table information for one or all the supported levels.
//                  NOTE: This call will return 0 for system memory
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// PARAMETERS:      hPhysicalGPU(IN) - GPU selection.
//                  level(IN) - Specific level selection. Zero for all levels. Number of levels increases with overclocking of the levels.
//                  pPerfClkTable(OUT) - Table of performance levels retrieved.
//
// RETURN STATUS:
//    NVAPI_OK - completed request
//    NVAPI_ERROR - miscellaneous error occurred
//    NVAPI_HANDLE_INVALIDATED - handle passed has been invalidated (see user guide)
//    NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE - handle passed is not a physical GPU handle
//    NVAPI_INCOMPATIBLE_STRUCT_VERSION - the version of the PERF_CLOCK_TABLE struct is not supported
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetPerfClocks(NvPhysicalGpuHandle hPhysicalGpu, NvU32 level, NV_GPU_PERF_CLOCK_TABLE *pPerfClkTable);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_GPU_SetPerfClocks
//
// DESCRIPTION:     Overclock a specific level in the performance table or overclock all levels with bSetClock set.
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// PARAMETERS:      hPhysicalGPU(IN) - GPU selection.
//                  level(IN) - Specific level selection. Zero for all levels. Number of levels increases with overclocking of the levels.
//                  pPerfClkTable(IN) - Table of performance levels to set.
//                  Any other than DEFAULT for GPU and Level flags - gpuPerfFlags and level flags gets applied.
//                  If bSetClock is set, currentFreq gets applied.
//                  Overclocking DOMAIN_NV requires simulteneous overclocking of DOMAIN_M, else overclocking will fail.
//
// RETURN STATUS:
//    NVAPI_OK - completed request
//    NVAPI_ERROR - miscellaneous error occurred
//    NVAPI_HANDLE_INVALIDATED - handle passed has been invalidated (see user guide)
//    NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE - handle passed is not a physical GPU handle
//    NVAPI_INCOMPATIBLE_STRUCT_VERSION - the version of the PERF_CLOCK_TABLE struct is not supported
//    NVAPI_TESTING_CLOCKS_NOT_SUPPORTED - testing clocks not supported
//
///////////////////////////////////////////////////////////////////////////////

NVAPI_INTERFACE NvAPI_GPU_SetPerfClocks(NvPhysicalGpuHandle hPhysicalGpu, NvU32 level, NV_GPU_PERF_CLOCK_TABLE *pPerfClkTable);

///////////////////////////////////////////////////////////////////////////////////
//  GPU Cooler Controls
//  Provides ability to Get and Set the fan level or equivalent cooler levels for various target devices associated with the GPU

#define NVAPI_MAX_COOLERS_PER_GPU_VER1  3
#define NVAPI_MAX_COOLERS_PER_GPU_VER2  20

#define NVAPI_MAX_COOLERS_PER_GPU       NVAPI_MAX_COOLERS_PER_GPU_VER2
#define NVAPI_MIN_COOLER_LEVEL          0
#define NVAPI_MAX_COOLER_LEVEL          100
#define NVAPI_MAX_COOLER_LEVELS         24

typedef enum
{
    NVAPI_COOLER_TYPE_NONE = 0,
    NVAPI_COOLER_TYPE_FAN,
    NVAPI_COOLER_TYPE_WATER,
    NVAPI_COOLER_TYPE_LIQUID_NO2,
} NV_COOLER_TYPE;

typedef enum
{
    NVAPI_COOLER_CONTROLLER_NONE = 0,
    NVAPI_COOLER_CONTROLLER_ADI,
    NVAPI_COOLER_CONTROLLER_INTERNAL,
} NV_COOLER_CONTROLLER;

typedef enum
{
    NVAPI_COOLER_POLICY_NONE = 0,
    NVAPI_COOLER_POLICY_MANUAL,                     //Manual adjustment of cooler level. Gets applied right away independent of temperature or performance level.
    NVAPI_COOLER_POLICY_PERF,                       //GPU performance controls the cooler level.
    NVAPI_COOLER_POLICY_TEMPERATURE_DISCRETE = 4,   //Discrete thermal levels control the cooler level.
    NVAPI_COOLER_POLICY_TEMPERATURE_CONTINUOUS = 8, //Cooler level adjusted at continuous thermal levels.
    NVAPI_COOLER_POLICY_HYBRID,                     //Hybrid of performance and temperature levels.
} NV_COOLER_POLICY;

typedef enum
{
    NVAPI_COOLER_TARGET_NONE = 0,   
    NVAPI_COOLER_TARGET_GPU,                //GPU cooler requires NvPhysicalGpuHandle
    NVAPI_COOLER_TARGET_MEMORY,             //GPU memory cooler requires NvPhysicalGpuHandle
    NVAPI_COOLER_TARGET_POWER_SUPPLY = 4,   //GPU power supply cooler requires NvPhysicalGpuHandle
    NVAPI_COOLER_TARGET_ALL = 7,            //This cooler cools all of the components related to its target gpu.
    NVAPI_COOLER_TARGET_VCD_COOLER1 = 8,    //Visual Computing Device coolers 1 to 20
    NVAPI_COOLER_TARGET_VCD_COOLER2,        //Requires NvVisualComputingDeviceHandle     
    NVAPI_COOLER_TARGET_VCD_COOLER3,   
    NVAPI_COOLER_TARGET_VCD_COOLER4,   
    NVAPI_COOLER_TARGET_VCD_COOLER5,   
    NVAPI_COOLER_TARGET_VCD_COOLER6,   
    NVAPI_COOLER_TARGET_VCD_COOLER7,   
    NVAPI_COOLER_TARGET_VCD_COOLER8,   
    NVAPI_COOLER_TARGET_VCD_COOLER9,   
    NVAPI_COOLER_TARGET_VCD_COOLER10,  
    NVAPI_COOLER_TARGET_VCD_COOLER11,  
    NVAPI_COOLER_TARGET_VCD_COOLER12,  
    NVAPI_COOLER_TARGET_VCD_COOLER13,  
    NVAPI_COOLER_TARGET_VCD_COOLER14,  
    NVAPI_COOLER_TARGET_VCD_COOLER15,  
    NVAPI_COOLER_TARGET_VCD_COOLER16,  
    NVAPI_COOLER_TARGET_VCD_COOLER17,  
    NVAPI_COOLER_TARGET_VCD_COOLER18,  
    NVAPI_COOLER_TARGET_VCD_COOLER19,  
    NVAPI_COOLER_TARGET_VCD_COOLER20,  
} NV_COOLER_TARGET;

typedef enum
{
    NVAPI_COOLER_CONTROL_NONE = 0,
    NVAPI_COOLER_CONTROL_TOGGLE,                   //ON/OFF
    NVAPI_COOLER_CONTROL_VARIABLE,                 //Suppports variable control.
} NV_COOLER_CONTROL;

typedef enum
{
    NVAPI_INACTIVE = 0,             //inactive or unsupported
    NVAPI_ACTIVE = 1,               //active and spinning in case of fan
} NV_COOLER_ACTIVITY_LEVEL;

typedef struct
{
    NvU32   version;                //structure version
    NvU32   count;                  //number of associated coolers with the selected GPU
    struct
    {
        NV_COOLER_TYPE              type;               //type of cooler - FAN, WATER, LIQUID_NO2...
        NV_COOLER_CONTROLLER        controller;         //internal, ADI...
        NvU32                       defaultMinLevel;    //the min default value % of the cooler
        NvU32                       defaultMaxLevel;    //the max default value % of the cooler
        NvU32                       currentMinLevel;    //the current allowed min value % of the cooler
        NvU32                       currentMaxLevel;    //the current allowed max value % of the cooler
        NvU32                       currentLevel;       //the current value % of the cooler
        NV_COOLER_POLICY            defaultPolicy;      //cooler control policy - auto-perf, auto-thermal, manual, hybrid...
        NV_COOLER_POLICY            currentPolicy;      //cooler control policy - auto-perf, auto-thermal, manual, hybrid...
        NV_COOLER_TARGET            target;             //cooling target - GPU, memory, chipset, powersupply, Visual Computing Device...
        NV_COOLER_CONTROL           controlType;        //toggle or variable
        NV_COOLER_ACTIVITY_LEVEL    active;             //is the cooler active - fan spinning...
    } cooler[NVAPI_MAX_COOLERS_PER_GPU_VER1];
} NV_GPU_GETCOOLER_SETTINGS_V1;

typedef struct
{
    NvU32   version;                //structure version
    NvU32   count;                  //number of associated coolers with the selected GPU
    struct
    {
        NV_COOLER_TYPE              type;               //type of cooler - FAN, WATER, LIQUID_NO2...
        NV_COOLER_CONTROLLER        controller;         //internal, ADI...
        NvU32                       defaultMinLevel;    //the min default value % of the cooler
        NvU32                       defaultMaxLevel;    //the max default value % of the cooler
        NvU32                       currentMinLevel;    //the current allowed min value % of the cooler
        NvU32                       currentMaxLevel;    //the current allowed max value % of the cooler
        NvU32                       currentLevel;       //the current value % of the cooler
        NV_COOLER_POLICY            defaultPolicy;      //cooler control policy - auto-perf, auto-thermal, manual, hybrid...
        NV_COOLER_POLICY            currentPolicy;      //cooler control policy - auto-perf, auto-thermal, manual, hybrid...
        NV_COOLER_TARGET            target;             //cooling target - GPU, memory, chipset, powersupply, Visual Computing Device...
        NV_COOLER_CONTROL           controlType;        //toggle or variable
        NV_COOLER_ACTIVITY_LEVEL    active;             //is the cooler active - fan spinning...
    } cooler[NVAPI_MAX_COOLERS_PER_GPU_VER2];
} NV_GPU_GETCOOLER_SETTINGS_V2;


#define NV_GPU_GETCOOLER_SETTINGS           NV_GPU_GETCOOLER_SETTINGS_V2

#define NV_GPU_GETCOOLER_SETTINGS_VER1      MAKE_NVAPI_VERSION(NV_GPU_GETCOOLER_SETTINGS_V1,1)
#define NV_GPU_GETCOOLER_SETTINGS_VER2A     MAKE_NVAPI_VERSION(NV_GPU_GETCOOLER_SETTINGS_V2,1)  // For shipped R180 compatibility
#define NV_GPU_GETCOOLER_SETTINGS_VER2      MAKE_NVAPI_VERSION(NV_GPU_GETCOOLER_SETTINGS_V2,2)

#define NV_GPU_GETCOOLER_SETTINGS_VER       NV_GPU_GETCOOLER_SETTINGS_VER2

typedef struct
{
    NvU32   version;        //structure version
    struct
    {
        NvU32               currentLevel;           //the new value % of the cooler
        NV_COOLER_POLICY    currentPolicy;          //the new cooler control policy - auto-perf, auto-thermal, manual, hybrid...
    } cooler[NVAPI_MAX_COOLERS_PER_GPU_VER1];
} NV_GPU_SETCOOLER_LEVEL_V1;

typedef struct
{
    NvU32   version;        //structure version
    struct
    {
        NvU32               currentLevel;           //the new value % of the cooler
        NV_COOLER_POLICY    currentPolicy;          //the new cooler control policy - auto-perf, auto-thermal, manual, hybrid...
    } cooler[NVAPI_MAX_COOLERS_PER_GPU_VER2];
} NV_GPU_SETCOOLER_LEVEL_V2;

#define NV_GPU_SETCOOLER_LEVEL          NV_GPU_SETCOOLER_LEVEL_V2

#define NV_GPU_SETCOOLER_LEVEL_VER1     MAKE_NVAPI_VERSION(NV_GPU_SETCOOLER_LEVEL_V1,1)
#define NV_GPU_SETCOOLER_LEVEL_VER2     MAKE_NVAPI_VERSION(NV_GPU_SETCOOLER_LEVEL_V2,1)

#define NV_GPU_SETCOOLER_LEVEL_VER      NV_GPU_SETCOOLER_LEVEL_VER2

typedef struct
{
    NvU32               version;        //structure version
    NV_COOLER_POLICY    policy;         //selected policy to update the cooler levels for, example NVAPI_COOLER_POLICY_PERF
    struct
    {
        NvU32 levelId;      // level indicator for a policy
        NvU32 currentLevel; // new cooler level for the selected policy level indicator.
        NvU32 defaultLevel; // default cooler level for the selected policy level indicator.
    } policyCoolerLevel[NVAPI_MAX_COOLER_LEVELS];
} NV_GPU_COOLER_POLICY_TABLE;

#define NV_GPU_COOLER_POLICY_TABLE_VER MAKE_NVAPI_VERSION(NV_GPU_COOLER_POLICY_TABLE,1)

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_GPU_GetCoolerSettings
//
// DESCRIPTION:     Retrieves the cooler information of all coolers or a specific cooler associated with the selected GPU.
//                  Coolers are indexed 0 to NVAPI_MAX_COOLERS_PER_GPU-1.
//                  To retrieve specific cooler info set the coolerIndex to the appropriate cooler index.
//                  To retrieve info for all cooler set coolerIndex to NVAPI_COOLER_TARGET_ALL.
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// PARAMETERS :     hPhysicalGPU(IN) - GPU selection.
//                  coolerIndex(IN)  - Explicit cooler index selection.
//                  pCoolerInfo(OUT) - Array of cooler settings.
//
// RETURN STATUS:
//    NVAPI_OK - completed request
//    NVAPI_ERROR - miscellaneous error occurred
//    NVAPI_INVALID_ARGUMENT - pCoolerInfo is NULL
//    NVAPI_HANDLE_INVALIDATED - handle passed has been invalidated (see user guide)
//    NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE - handle passed is not a physical GPU handle
//    NVAPI_INCOMPATIBLE_STRUCT_VERSION - the version of the INFO struct is not supported
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetCoolerSettings(NvPhysicalGpuHandle hPhysicalGpu, NvU32 coolerIndex, NV_GPU_GETCOOLER_SETTINGS *pCoolerInfo);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_GPU_SetCoolerLevels
//
// DESCRIPTION:     Set the cooler levels for all coolers or a specific cooler associated with the selected GPU.
//                  Coolers are indexed 0 to NVAPI_MAX_COOLERS_PER_GPU-1. Every cooler level with non-zero currentpolicy gets applied.
//                  The new level should be in the range of minlevel and maxlevel retrieved from GetCoolerSettings API or between
//                  and NVAPI_MIN_COOLER_LEVEL to MAX_COOLER_LEVEL.
//                  To set level for a specific cooler set the coolerIndex to the appropriate cooler index.
//                  To set level for all coolers set coolerIndex to NVAPI_COOLER_TARGET_ALL.
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// NOTE:            To lock the fan speed independent of the temperature or performance changes set the cooler currentPolicy to
//                  NVAPI_COOLER_POLICY_MANUAL else set it to the current policy retrieved from the GetCoolerSettings API.
// PARAMETERS:      hPhysicalGPU(IN) - GPU selection.
//                  coolerIndex(IN)  - Explicit cooler index selection.
//                  pCoolerLevels(IN) - Updated cooler level and cooler policy.
//
// RETURN STATUS:
//    NVAPI_OK - completed request
//    NVAPI_ERROR - miscellaneous error occurred
//    NVAPI_INVALID_ARGUMENT - pCoolerLevels is NULL
//    NVAPI_HANDLE_INVALIDATED - handle passed has been invalidated (see user guide)
//    NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE - handle passed is not a physical GPU handle
//    NVAPI_INCOMPATIBLE_STRUCT_VERSION - the version of the INFO struct is not supported
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_SetCoolerLevels(NvPhysicalGpuHandle hPhysicalGpu, NvU32 coolerIndex, NV_GPU_SETCOOLER_LEVEL *pCoolerLevels);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_GPU_RestoreCoolerSettings
//
// DESCRIPTION:     Restore the modified cooler settings to NVIDIA defaults.
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// PARAMETERS:      hPhysicalGPU(IN) - GPU selection.
//                  pCoolerIndex(IN) - Array containing absolute cooler indexes to restore. Pass NULL restore all coolers.
//                  CoolerCount - Number of coolers to restore.
//
// RETURN STATUS:
//    NVAPI_OK - completed request
//    NVAPI_ERROR - miscellaneous error occurred
//    NVAPI_HANDLE_INVALIDATED - handle passed has been invalidated (see user guide)
//    NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE - handle passed is not a physical GPU handle
//    NVAPI_INCOMPATIBLE_STRUCT_VERSION - the version of the INFO struct is not supported
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_RestoreCoolerSettings(NvPhysicalGpuHandle hPhysicalGpu, NvU32 *pCoolerIndex, NvU32 coolerCount);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_GPU_GetCoolerPolicyTable
//
// DESCRIPTION:     Retrieves the table of cooler and policy levels for the selected policy. Supported only for NVAPI_COOLER_POLICY_PERF.
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// PARAMETERS:      hPhysicalGPU(IN) - GPU selection.
//                  coolerIndex(IN) - cooler index selection.
//                  pCoolerTable(OUT) - Table of policy levels and associated cooler levels.
//                  count(OUT) - Count of the number of valid levels for the selected policy.
//
// RETURN STATUS:
//    NVAPI_OK - completed request
//    NVAPI_ERROR - miscellaneous error occurred
//    NVAPI_HANDLE_INVALIDATED - handle passed has been invalidated (see user guide)
//    NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE - handle passed is not a physical GPU handle
//    NVAPI_INCOMPATIBLE_STRUCT_VERSION - the version of the INFO struct is not supported
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetCoolerPolicyTable(NvPhysicalGpuHandle hPhysicalGpu, NvU32 coolerIndex, NV_GPU_COOLER_POLICY_TABLE *pCoolerTable, NvU32 *count);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_GPU_SetCoolerPolicyTable
//
// DESCRIPTION:     Restore the modified cooler settings to NVIDIA defaults. Supported only for NVAPI_COOLER_POLICY_PERF.
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// PARAMETERS:      hPhysicalGPU(IN) - GPU selection.
//                  coolerIndex(IN) - cooler index selection.
//                  pCoolerTable(IN) - Updated table of policy levels and associated cooler levels. Every non-zero policy level gets updated.
//                  count(IN) - Number of valid levels in the policy table.
//
// RETURN STATUS:
//    NVAPI_OK - completed request
//    NVAPI_ERROR - miscellaneous error occurred
//    NVAPI_HANDLE_INVALIDATED - handle passed has been invalidated (see user guide)
//    NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE - handle passed is not a physical GPU handle
//    NVAPI_INCOMPATIBLE_STRUCT_VERSION - the version of the INFO struct is not supported
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_SetCoolerPolicyTable(NvPhysicalGpuHandle hPhysicalGpu, NvU32 coolerIndex, NV_GPU_COOLER_POLICY_TABLE *pCoolerTable, NvU32 count);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_GPU_RestoreCoolerPolicyTable
//
// DESCRIPTION:     Restores the perf table policy levels to the defaults.
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// PARAMETERS:      hPhysicalGPU(IN) - GPU selection.
//                  coolerIndex(IN) - cooler index selection.
//                  pCoolerIndex(IN) - Array containing absolute cooler indexes to restore. Pass NULL restore all coolers.
//                  coolerCount - Number of coolers to restore.
//                  policy - restore for the selected policy
//
// RETURN STATUS:
//    NVAPI_OK - completed request
//    NVAPI_ERROR - miscellaneous error occurred
//    NVAPI_HANDLE_INVALIDATED - handle passed has been invalidated (see user guide)
//    NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE - handle passed is not a physical GPU handle
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_RestoreCoolerPolicyTable(NvPhysicalGpuHandle hPhysicalGpu, NvU32 *pCoolerIndex, NvU32 coolerCount, NV_COOLER_POLICY policy);


#define NV_GPU_PERF_PSTATES_FLAGS_PERFMON_ENABLED        0x00000001
#define NV_GPU_PERF_PSTATES_FLAGS_DYN_PSTATES_CAPABLE    0x00000002
#define NV_GPU_PERF_PSTATES_FLAGS_DYNAMIC_PSTATE_ENABLED 0x00000004

#define NV_GPU_PERF_SET_FORCE_PSTATE_FLAGS_ASYNC       0x00000001

#define NVAPI_MAX_GPU_PERF_VOLTAGES         16
#define NVAPI_MAX_GPU_PERF_PSTATES          16

#define NV_GPU_PERF_PSTATE_FLAGS_PCIELIMIT_GEN1       0x00000001
#define NV_GPU_PERF_PSTATE_FLAGS_OVERCLOCKED_TRUE     0x00000002

typedef enum _NV_GPU_PERF_PSTATE_ID
{
    NVAPI_GPU_PERF_PSTATE_P0                = 0,
    NVAPI_GPU_PERF_PSTATE_P1                = 1,
    NVAPI_GPU_PERF_PSTATE_P2                = 2,
    NVAPI_GPU_PERF_PSTATE_P3                = 3,
    NVAPI_GPU_PERF_PSTATE_P8                = 8,
    NVAPI_GPU_PERF_PSTATE_P10               = 10,
    NVAPI_GPU_PERF_PSTATE_P12               = 12,
    NVAPI_GPU_PERF_PSTATE_P15               = 15,
    NVAPI_GPU_PERF_PSTATE_UNDEFINED         = NVAPI_MAX_GPU_PERF_PSTATES,

} NV_GPU_PERF_PSTATE_ID;


typedef struct
{
    NvU32   version;
    NvU32   flags;
    NvU32   numPstates;
    NvU32   numClocks;
    struct
    {
        NV_GPU_PERF_PSTATE_ID   pstateId;
        NvU32                   flags;
        struct
        {
            NV_GPU_CLOCK_INFO_DOMAIN_ID domainId;
            NvU32                                flags;
            NvU32                                freq;

        } clocks[NVAPI_MAX_GPU_PERF_CLOCKS];
    } pstates[NVAPI_MAX_GPU_PERF_PSTATES];

} NV_GPU_PERF_PSTATES_INFO;

#define NV_GPU_PERF_PSTATES_INFO_VER  MAKE_NVAPI_VERSION(NV_GPU_PERF_PSTATES_INFO,1)

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_GPU_GetPstatesInfo
//
// DESCRIPTION:     Retrieves all performance states (P-States) information.
//
//                  P-States are GPU active/executing performance capability and power consumption states.
//                  P-States ranges from P0 to P15, with P0 being the highest performance/power state, and
//                  P15 being the lowest performance/power state. Each P-State, if available, maps to a
//                  performance level. Not all P-States are available on a given system. The definition
//                  of each P-States are currently as follow:
//                    P0/P1 - Maximum 3D performance
//                    P2/P3 - Balanced 3D performance-power
//                    P8 - Basic HD video playback
//                    P10 - DVD playback
//                    P12 - Minimum idle power consumption
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// PARAMETERS:      hPhysicalGPU(IN)  - GPU selection.
//                  pPerfPstates(OUT) - P-States information retrieved, as detailed below:
//                  - flags is reserved for future use.
//                  - numPstates is the number of available P-States
//                  - numClocks is the number of clock domains supported by each P-State
//                  - pstates has valid index range from 0 to numPstates - 1
//                  - pstates[i].pstateId is the ID of the P-State,
//                      containing the following info:
//                    - pstates[i].flags containing the following info:
//                        bit 0 indicates if the PCIE limit is GEN1 or GEN2
//                        bit 1 indicates if the Pstate is overclocked or not
//                    - pstates[i].clocks has valid index range from 0 to numClocks -1
//                    - pstates[i].clocks[j].domainId is the ID of the clock domain,
//                        containing the following info:
//                      - pstates[i].clocks[j].flags is reserved for future use.
//                      - pstates[i].clocks[j].freq is the clock frequency in kHz
//
// RETURN STATUS:
//    NVAPI_OK - completed request
//    NVAPI_ERROR - miscellaneous error occurred
//    NVAPI_HANDLE_INVALIDATED - handle passed has been invalidated (see user guide)
//    NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE - handle passed is not a physical GPU handle
//    NVAPI_INCOMPATIBLE_STRUCT_VERSION - the version of the NV_GPU_PERF_PSTATES struct is not supported
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetPstatesInfo(NvPhysicalGpuHandle hPhysicalGpu, NV_GPU_PERF_PSTATES_INFO *pPerfPstatesInfo);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_GPU_GetCurrentPstate
//
// DESCRIPTION:     Retrieves the current performance state (P-State).
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// PARAMETERS:      hPhysicalGPU(IN)    - GPU selection.
//                  pCurrentPstate(OUT) - The ID of the P-State the GPU is currently in.
//
// RETURN STATUS:
//    NVAPI_OK - completed request
//    NVAPI_ERROR - miscellaneous error occurred
//    NVAPI_HANDLE_INVALIDATED - handle passed has been invalidated (see user guide)
//    NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE - handle passed is not a physical GPU handle
//    NVAPI_NOT_SUPPORTED - P-States is not supported on this setup
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetCurrentPstate(NvPhysicalGpuHandle hPhysicalGpu, NV_GPU_PERF_PSTATE_ID *pCurrentPstate);

typedef enum _NV_PERF_PSTATE_CLIENT_LIMIT_ID
{
    NVAPI_PERF_PSTATE_CLIENT_LIMIT_HARD                 = 0x00000001,       // P-States Hard limit 
    NVAPI_PERF_PSTATE_CLIENT_LIMIT_SOFT                 = 0x00000002,       // P-States Soft limit 
    NVAPI_PERF_PSTATE_CLIENT_LIMIT_BOTH                 = 0x00000003        // For both hard and soft P-States limits
} NV_GPU_PERF_PSTATE_CLIENT_LIMIT_ID;

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_GPU_GetPstateClientLimits
//
// DESCRIPTION:     Get soft or hard limit of performance state (P-State)
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// PARAMETERS:      hPhysicalGPU(IN)  - GPU selection.
//                  limitId(IN)       - To indicate hard limit or soft limit 
//                  PstateLimit(OUT)  - The ID of the P-State that is the current limit
//
// RETURN STATUS:
//    NVAPI_OK - completed request
//    NVAPI_ERROR - miscellaneous error occurred
//    NVAPI_HANDLE_INVALIDATED - handle passed has been invalidated (see user guide)
//    NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE - handle passed is not a physical GPU handle
//    NVAPI_INVALID_ARGUMENT - Invalid input parameter
//    NVAPI_NOT_SUPPORTED - P-States is not supported on this setup
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetPstateClientLimits(NvPhysicalGpuHandle hPhysicalGpu,
                                                NV_GPU_PERF_PSTATE_CLIENT_LIMIT_ID limitId,
                                                NV_GPU_PERF_PSTATE_ID* PstateLimit);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_GPU_SetPstateClientLimits
//
// DESCRIPTION:     Set soft or hard limit of performance state (P-State)
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// PARAMETERS:      hPhysicalGPU(IN)  - GPU selection.
//                  limitId(IN)       - To indicate hard limit, soft limit or both
//                  PstateLimit(IN)   - The ID of the P-State to be the new limit
//
// RETURN STATUS:
//    NVAPI_OK - completed request
//    NVAPI_ERROR - miscellaneous error occurred
//    NVAPI_HANDLE_INVALIDATED - handle passed has been invalidated (see user guide)
//    NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE - handle passed is not a physical GPU handle
//    NVAPI_INVALID_ARGUMENT - Invalid input parameter
//    NVAPI_NOT_SUPPORTED - P-States is not supported on this setup
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_SetPstateClientLimits(NvPhysicalGpuHandle hPhysicalGpu,
                                                NV_GPU_PERF_PSTATE_CLIENT_LIMIT_ID limitId,
                                                NV_GPU_PERF_PSTATE_ID PstateLimit);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_GPU_EnableOverclockedPstates
//
// DESCRIPTION:     To allow overclocked P-states 
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// PARAMETERS:      hPhysicalGPU(IN)  - GPU selection.
//                  bEnable(IN)       - (boolean) To enable or disable overclocked P-states 
//
// RETURN STATUS:
//    NVAPI_OK - completed request
//    NVAPI_ERROR - miscellaneous error occurred
//    NVAPI_HANDLE_INVALIDATED - handle passed has been invalidated (see user guide)
//    NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE - handle passed is not a physical GPU handle
//    NVAPI_INVALID_ARGUMENT - Invalid input parameter
//    NVAPI_NOT_SUPPORTED - P-States or overclocked P-states is not supported on this setup
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_EnableOverclockedPstates(NvPhysicalGpuHandle hPhysicalGpu,
                                                   NvU8 bEnable);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_GPU_EnableDynamicPstates
//
// DESCRIPTION:     Enables or Disables Dynamic P-states 
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// PARAMETERS:      hPhysicalGPU(IN)  - GPU selection.
//                  bEnable(IN)       - (boolean) To enable or disable Dynamic P-states 
//
// RETURN STATUS:
//    NVAPI_OK - completed request
//    NVAPI_ERROR - miscellaneous error occurred
//    NVAPI_HANDLE_INVALIDATED - handle passed has been invalidated (see user guide)
//    NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE - handle passed is not a physical GPU handle
//    NVAPI_INVALID_ARGUMENT - Invalid input parameter
//    NVAPI_NOT_SUPPORTED - P-States is not supported on this setup
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_EnableDynamicPstates(NvPhysicalGpuHandle hPhysicalGpu,
                                               NvU8 bEnable);

#define NVAPI_MAX_GPU_UTILIZATIONS 8

typedef enum _NV_GPU_UTILIZATION_DOMAIN_ID
{
    NVAPI_GPU_UTILIZATION_DOMAIN_GPU    = 0,
    NVAPI_GPU_UTILIZATION_DOMAIN_FB     = 1,
    NVAPI_GPU_UTILIZATION_DOMAIN_VID    = 2,
} NV_GPU_UTILIZATION_DOMAIN_ID;

typedef struct
{
    NvU32       version;        // Structure version
    NvU32       flags;          // Reserved for future use
    struct
    {
        NvU32   bIsPresent:1;   // Set if this utilization domain is present on this GPU
        NvU32   percentage;     // Percentage of time where the domain is considered busy in the last 1 second interval
    } utilization[NVAPI_MAX_GPU_UTILIZATIONS];
} NV_GPU_DYNAMIC_PSTATES_INFO_EX;
#define NV_GPU_DYNAMIC_PSTATES_INFO_EX_VER MAKE_NVAPI_VERSION(NV_GPU_DYNAMIC_PSTATES_INFO_EX,1)

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GetDynamicPstatesInfoEx
//
// DESCRIPTION:   This retrieves the NV_GPU_DYNAMIC_PSTATES_INFO_EX structure for the specified physical GPU.
//                    flags contains following information:
//                      bit 0 indicates if the dynamic Pstate is enabled or not
//                    For each utilization domain:
//                        bIsPresent is set for each domain that is present on the GPU
//                        percentage is the percentage of time where the domain is considered busy in the last 1 second interval
//
//                Each domain's info is indexed in the array.  For example:
//                    pDynamicPstatesInfo->utilization[NVAPI_GPU_UTILIZATION_DOMAIN_GPU] holds the info for the GPU domain
//                There are currently 3 domains, for which GPU utilization and dynamic P-State thresholds can be retrieved
//                    graphic engine (GPU), frame buffer (FB), and video engine (VID).
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// RETURN STATUS:
//    NVAPI_OK - completed request
//    NVAPI_ERROR - miscellaneous error occurred
//    NVAPI_INVALID_ARGUMENT - pDynamicPstatesInfo is NULL
//    NVAPI_HANDLE_INVALIDATED - handle passed has been invalidated (see user guide)
//    NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE - handle passed is not a physical GPU handle
//    NVAPI_INCOMPATIBLE_STRUCT_VERSION - the version of the INFO struct is not supported
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetDynamicPstatesInfoEx(NvPhysicalGpuHandle hPhysicalGpu, NV_GPU_DYNAMIC_PSTATES_INFO_EX *pDynamicPstatesInfoEx);

///////////////////////////////////////////////////////////////////////////////////
//  Thermal API
//  Provides ability to get temperature levels from the various thermal sensors associated with the GPU

#define NVAPI_MAX_THERMAL_SENSORS_PER_GPU 3

typedef enum
{
    NVAPI_THERMAL_TARGET_NONE          = 0,
    NVAPI_THERMAL_TARGET_GPU           = 1,     //GPU core temperature requires NvPhysicalGpuHandle
    NVAPI_THERMAL_TARGET_MEMORY        = 2,     //GPU memory temperature requires NvPhysicalGpuHandle
    NVAPI_THERMAL_TARGET_POWER_SUPPLY  = 4,     //GPU power supply temperature requires NvPhysicalGpuHandle
    NVAPI_THERMAL_TARGET_BOARD         = 8,     //GPU board ambient temperature requires NvPhysicalGpuHandle
    NVAPI_THERMAL_TARGET_VCD_BOARD     = 9,     //Visual Computing Device Board temperature requires NvVisualComputingDeviceHandle
    NVAPI_THERMAL_TARGET_VCD_INLET     = 10,    //Visual Computing Device Inlet temperature requires NvVisualComputingDeviceHandle
    NVAPI_THERMAL_TARGET_VCD_OUTLET    = 11,    //Visual Computing Device Outlet temperature requires NvVisualComputingDeviceHandle
    NVAPI_THERMAL_TARGET_ALL           = 15,
    NVAPI_THERMAL_TARGET_UNKNOWN       = -1,
} NV_THERMAL_TARGET;

typedef enum
{
    NVAPI_THERMAL_CONTROLLER_NONE = 0,
    NVAPI_THERMAL_CONTROLLER_GPU_INTERNAL,
    NVAPI_THERMAL_CONTROLLER_ADM1032,
    NVAPI_THERMAL_CONTROLLER_MAX6649,
    NVAPI_THERMAL_CONTROLLER_MAX1617,
    NVAPI_THERMAL_CONTROLLER_LM99,
    NVAPI_THERMAL_CONTROLLER_LM89,
    NVAPI_THERMAL_CONTROLLER_LM64,
    NVAPI_THERMAL_CONTROLLER_ADT7473,
    NVAPI_THERMAL_CONTROLLER_SBMAX6649,
    NVAPI_THERMAL_CONTROLLER_VBIOSEVT,
    NVAPI_THERMAL_CONTROLLER_OS,
    NVAPI_THERMAL_CONTROLLER_UNKNOWN = -1,
} NV_THERMAL_CONTROLLER;

typedef struct
{
    NvU32   version;                //structure version
    NvU32   count;                  //number of associated thermal sensors
    struct
    {
        NV_THERMAL_CONTROLLER       controller;         //internal, ADM1032, MAX6649...
        NvU32                       defaultMinTemp;     //the min default temperature value of the thermal sensor in degrees centigrade
        NvU32                       defaultMaxTemp;    //the max default temperature value of the thermal sensor in degrees centigrade
        NvU32                       currentTemp;       //the current temperature value of the thermal sensor in degrees centigrade
        NV_THERMAL_TARGET           target;             //thermal senor targeted @ GPU, memory, chipset, powersupply, Visual Computing Device...
    } sensor[NVAPI_MAX_THERMAL_SENSORS_PER_GPU];

} NV_GPU_THERMAL_SETTINGS;

#define NV_GPU_THERMAL_SETTINGS_VER  MAKE_NVAPI_VERSION(NV_GPU_THERMAL_SETTINGS,1)

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_GPU_GetThermalSettings
//
// DESCRIPTION:     Retrieves the thermal information of all thermal sensors or specific thermal sensor associated with the selected GPU.
//                  Thermal sensors are indexed 0 to NVAPI_MAX_THERMAL_SENSORS_PER_GPU-1.
//                  To retrieve specific thermal sensor info set the sensorIndex to the required thermal sensor index.
//                  To retrieve info for all sensors set sensorIndex to NVAPI_THERMAL_TARGET_ALL.
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// PARAMETERS :     hPhysicalGPU(IN) - GPU selection.
//                  sensorIndex(IN)  - Explicit thermal sensor index selection.
//                  pThermalSettings(OUT) - Array of thermal settings.
//
// RETURN STATUS:
//    NVAPI_OK - completed request
//    NVAPI_ERROR - miscellaneous error occurred
//    NVAPI_INVALID_ARGUMENT - pThermalInfo is NULL
//    NVAPI_HANDLE_INVALIDATED - handle passed has been invalidated (see user guide)
//    NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE - handle passed is not a physical GPU handle
//    NVAPI_INCOMPATIBLE_STRUCT_VERSION - the version of the INFO struct is not supported
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetThermalSettings(NvPhysicalGpuHandle hPhysicalGpu, NvU32 sensorIndex, NV_GPU_THERMAL_SETTINGS *pThermalSettings);

typedef enum _NVAPI_DITHER_TYPE
{
    NVAPI_DITHER_TYPE_DEFAULT = 0,  // enable/disable based on the default behavior
    NVAPI_DITHER_TYPE_ENABLE  = 1,  // enable dithering
    NVAPI_DITHER_TYPE_DISABLE = 2,  // disable dithering
} NVAPI_DITHER_TYPE;

typedef enum _NVAPI_DITHER_BITS
{
    NVAPI_DITHER_BITS_6BITS = 0,
    NVAPI_DITHER_BITS_8BITS = 1,
} NVAPI_DITHER_BITS;

typedef enum _NVAPI_DITHER_MODE
{
    NVAPI_DITHER_MODE_DYNAMIC_ERR_ACC = 0,
    NVAPI_DITHER_MODE_STATIC_ERR_ACC  = 1,
    NVAPI_DITHER_MODE_DYNAMIC_2X2     = 2,
    NVAPI_DITHER_MODE_STATIC_2X2      = 3,
} NVAPI_DITHER_MODE;

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_SetDitherControl
//
// DESCRIPTION: Sets display related HW dither controls (dithering a higher bpp framebuffer to a lower bpp display)
//
//  SUPPORTED OS: Windows XP and higher
//
// PARAMETERS:    outputId(IN)   - One of the selected outputId retrieved from NvAPI_GPU_GetActiveOutputs or
//                                 NvAPI_GetView, to identify the targeted TV.
//                type(IN)       - The dithering mode to apply (default, enable, or disable)
//                bits(IN)       - The bits to dither to
//                mode(IN)       - The dither mode
//
// RETURN STATUS:
//    NVAPI_OK - Dither controls successfully set.
//    NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE - handle passed is not a physical GPU handle.
//    NVAPI_INVALID_ARGUMENT - Parameters passed do not match the hardware implementation
//    NVAPI_NOT_SUPPORTED - Dither control feature is not supported on the selected GPU
//    NVAPI_NO_IMPLEMENTATION - No implementation for dither controls implemented for this GPU
//    NVAPI_ERROR - Dither controls were not successfully propogated
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_SetDitherControl(NvPhysicalGpuHandle hPhysicalGpu,
                                           NvU32 outputId,
                                           NVAPI_DITHER_TYPE type,
                                           NVAPI_DITHER_BITS bits,
                                           NVAPI_DITHER_MODE mode);

////////////////////////////////////////////////////////////////////////////////
//NvAPI_TVOutput Information

typedef enum _NV_DISPLAY_TV_FORMAT
{
    NV_DISPLAY_TV_FORMAT_NONE         = 0,
    NV_DISPLAY_TV_FORMAT_SD_NTSCM     = 0x00000001,
    NV_DISPLAY_TV_FORMAT_SD_NTSCJ     = 0x00000002,
    NV_DISPLAY_TV_FORMAT_SD_PALM      = 0x00000004,
    NV_DISPLAY_TV_FORMAT_SD_PALBDGH   = 0x00000008,
    NV_DISPLAY_TV_FORMAT_SD_PALN      = 0x00000010,
    NV_DISPLAY_TV_FORMAT_SD_PALNC     = 0x00000020,
    NV_DISPLAY_TV_FORMAT_SD_576i      = 0x00000100,
    NV_DISPLAY_TV_FORMAT_SD_480i      = 0x00000200,
    NV_DISPLAY_TV_FORMAT_ED_480p      = 0x00000400,
    NV_DISPLAY_TV_FORMAT_ED_576p      = 0x00000800,
    NV_DISPLAY_TV_FORMAT_HD_720p      = 0x00001000,
    NV_DISPLAY_TV_FORMAT_HD_1080i     = 0x00002000,
    NV_DISPLAY_TV_FORMAT_HD_1080p     = 0x00004000,
    NV_DISPLAY_TV_FORMAT_HD_720p50    = 0x00008000,
    NV_DISPLAY_TV_FORMAT_HD_1080p24   = 0x00010000,
    NV_DISPLAY_TV_FORMAT_HD_1080i50   = 0x00020000,
    NV_DISPLAY_TV_FORMAT_HD_1080p50   = 0x00040000,

    NV_DISPLAY_TV_FORMAT_SD_OTHER     = 0x01000000,
    NV_DISPLAY_TV_FORMAT_ED_OTHER     = 0x02000000,
    NV_DISPLAY_TV_FORMAT_HD_OTHER     = 0x04000000,

} NV_DISPLAY_TV_FORMAT;


typedef struct
{
    NvU32                   version;                            //[IN]       Structure version.
    NvU32                   supportedFormats;                   //[OUT only] One or more TV formats defined in NV_DISPLAY_TV_FORMAT matching, encoder supported formats for analog TVs or EDID exposed modes for digital TVs.
    NV_DISPLAY_TV_FORMAT    currentFormat;                      //[IN/OUT]   One of the selected TV output format from supportedFormats defined in NV_DISPLAY_TV_FORMAT.
    NV_GPU_CONNECTOR_TYPE   currentConnector;                   //[IN/OUT]   For Analog TV, valid TV output connector is one of the NVAPI_GPU_CONNECTOR_TV types.
                                                                //           For Digital TV, valid TV output connector is one of the NVAPI_GPU_CONNECTOR_DVI types.
} NV_DISPLAY_TV_OUTPUT_INFO;

#define NV_DISPLAY_TV_OUTPUT_INFO_VER MAKE_NVAPI_VERSION(NV_DISPLAY_TV_OUTPUT_INFO, 1)

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GetTVOutputInfo
//
//   DESCRIPTION: Retrieves the TV display output information of the selected display.
//
//  SUPPORTED OS: Windows Vista and higher
//
//    PARAMETERS: hNvDisplay(IN)    - NVIDIA Display selection. It can be NVAPI_DEFAULT_HANDLE or a handle enumerated from NvAPI_EnumNVidiaDisplayHandle().
//                outputId(IN)      - One of the selected outputId retrieved from NvAPI_GPU_GetActiveOutputs or NvAPI_GetView, to identify the targeted TV.
//                                    Can be NULL to auto pick the TV output associated with hNvDisplay.
//                                    The outputId has to be of type NVAPI_GPU_OUTPUT_TV or NVAPI_GPU_OUTPUT_DFP in case of digital HDTV.
//                pTVOutInfo(OUT)  -  The returned TV output information.
//                                    With digital HDTV, the supportedFormats are limited to the available EIA-861B modes in the EDID
//                                    or the custom 861B modes if added by the user.
//
// RETURN STATUS: NVAPI_OK: call successful.
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found.
//                NVAPI_EXPECTED_DISPLAY_HANDLE: hNvDisplay is not a valid display handle.
//                NVAPI_INVALID_ARGUMENT: pTVOutInfo is NULL.
//                NVAPI_INCOMPATIBLE_STRUCT_VERSION: the version of the NV_DISPLAY_TV_OUTPUT_INFO_VER struct is not supported.
//                NVAPI_EXPECTED_TV_DISPLAY: expected TV output display in outputId.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GetTVOutputInfo(NvDisplayHandle hNvDisplay, NvU32 outputId, NV_DISPLAY_TV_OUTPUT_INFO *pTVOutInfo);


typedef struct
{
    NvU32   version;                //IN version info

    struct
    {
        NvU32   defaultLevel;       //OUT default level
        NvU32   currentLevel;       //IN/OUT current level
        NvU32   minLevel;           //OUT min range level
        NvU32   maxLevel;           //OUT max range level
    }   flicker, saturation;

} NV_TV_ENCODER_CONTROLS;

#define NV_TV_ENCODER_CONTROLS_VER  MAKE_NVAPI_VERSION(NV_TV_ENCODER_CONTROLS,1)

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GetTVEncoderControls
//
//   DESCRIPTION: Retrieves the flicker and saturation levels for the selected TV.
//
//  SUPPORTED OS: Windows Vista and higher
//
//    PARAMETERS: hNvDisplay(IN) - NVIDIA Display selection. It can be NVAPI_DEFAULT_HANDLE or a handle enumerated from NvAPI_EnumNVidiaDisplayHandle().
//                outputId(IN)   - One of the selected outputId retrieved from NvAPI_GPU_GetActiveOutputs or NvAPI_GetView, to identify the target
//                                 output in case multiple targets are associated with the selected hNvDisplay.
//                                 Can be NULL to pick the display output id associated with hNvDisplay.
//                pTvEncoderInfo(OUT)  - The returned TV encoder controls and its levels.
//                Note: If maxLevel and minLevel are both zeros then that control is not supported on this GPU and should not be used.
//
// RETURN STATUS: NVAPI_OK: call successful.
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found.
//                NVAPI_EXPECTED_DISPLAY_HANDLE: hNvDisplay is not a valid display handle.
//                NVAPI_INVALID_ARGUMENT: pTvEncoderInfo is NULL.
//                NVAPI_INCOMPATIBLE_STRUCT_VERSION - the version of the NV_TV_ENCODER_CONTROLS struct is not supported
//                NVAPI_NOT_SUPPORTED - TV or this API is not supported on the selected GPU
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GetTVEncoderControls(NvDisplayHandle hNvDisplay, NvU32 outputId, NV_TV_ENCODER_CONTROLS *pTvEncoderInfo);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_SetTVEncoderControls
//
//   DESCRIPTION: Sets the flicker and saturation levels for the selected TV.
//
//  SUPPORTED OS: Windows Vista and higher
//
//    PARAMETERS: hNvDisplay(IN) - NVIDIA Display selection. It can be NVAPI_DEFAULT_HANDLE or a handle enumerated from NvAPI_EnumNVidiaDisplayHandle().
//                outputId(IN)   - One of the selected outputId retrieved from NvAPI_GPU_GetActiveOutputs or NvAPI_GetView, to identify the target
//                                 output in case multiple targets are associated with the selected hNvDisplay.
//                                 Can be NULL to pick the display output id associated with hNvDisplay.
//                pTvEncoderInfo(IN)  - The selected TV encoder levels gets applied.
//
// RETURN STATUS: NVAPI_OK: call successful.
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found.
//                NVAPI_EXPECTED_DISPLAY_HANDLE: hNvDisplay is not a valid display handle.
//                NVAPI_INVALID_ARGUMENT: pTvEncoderInfo is NULL.
//                NVAPI_INCOMPATIBLE_STRUCT_VERSION - the version of the NV_TV_ENCODER_CONTROLS struct is not supported
//                NVAPI_NOT_SUPPORTED - TV or this API is not supported on the selected GPU
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_SetTVEncoderControls(NvDisplayHandle hNvDisplay, NvU32 outputId, NV_TV_ENCODER_CONTROLS *pTvEncoderInfo);

typedef enum
{
    NV_TV_BORDER_COLOR_BLACK    = 0,
    NV_TV_BORDER_COLOR_GREY     = 1,
} NV_TV_BORDER_COLOR;

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GetTVOutputBorderColor
//
//   DESCRIPTION: Retrieves the color of the TV border.
//
//  SUPPORTED OS: Windows Vista and higher
//
//    PARAMETERS: hNvDisplay(IN) - NVIDIA Display selection. It can be NVAPI_DEFAULT_HANDLE or a handle enumerated from NvAPI_EnumNVidiaDisplayHandle().
//                outputId(IN)   - One of the selected outputId retrieved from NvAPI_GPU_GetActiveOutputs or NvAPI_GetView, to identify the target
//                                 output in case multiple targets are associated with the selected hNvDisplay.
//                                 Can be NULL to pick the display output id associated with hNvDisplay.
//                pBorderColor(OUT)  - The returned TV border color.
//
// RETURN STATUS: NVAPI_OK: call successful.
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found.
//                NVAPI_EXPECTED_DISPLAY_HANDLE: hNvDisplay is not a valid display handle.
//                NVAPI_INVALID_ARGUMENT: pBorderColor is NULL.
//                NVAPI_NOT_SUPPORTED - TV or this API is not supported on the selected GPU
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GetTVOutputBorderColor(NvDisplayHandle hNvDisplay, NvU32 outputId, NV_TV_BORDER_COLOR *pBorderColor);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_SetTVOutputBorderColor
//
//   DESCRIPTION: Sets the color of the TV border.
//
//  SUPPORTED OS: Windows Vista and higher
//
//    PARAMETERS: hNvDisplay(IN) - NVIDIA Display selection. It can be NVAPI_DEFAULT_HANDLE or a handle enumerated from NvAPI_EnumNVidiaDisplayHandle().
//                outputId(IN)   - One of the selected outputId retrieved from NvAPI_GPU_GetActiveOutputs or NvAPI_GetView, to identify the target
//                                 output in case multiple targets are associated with the selected hNvDisplay.
//                                 Can be NULL to pick the display output id associated with hNvDisplay.
//                borderColor(IN)  - The TV border color to set.
//
// RETURN STATUS: NVAPI_OK: call successful.
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found.
//                NVAPI_EXPECTED_DISPLAY_HANDLE: hNvDisplay is not a valid display handle.
//                NVAPI_NOT_SUPPORTED - TV or this API is not supported on the selected GPU
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_SetTVOutputBorderColor(NvDisplayHandle hNvDisplay, NvU32 outputId, NV_TV_BORDER_COLOR borderColor);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GetDisplayPosition
//
//   DESCRIPTION: Retrieves the display position of the associated analog display.
//
//  SUPPORTED OS: Windows Vista and higher
//
//    PARAMETERS: hNvDisplay(IN) - NVIDIA Display selection. It can be NVAPI_DEFAULT_HANDLE or a handle enumerated from NvAPI_EnumNVidiaDisplayHandle().
//                outputId(IN)   - One of the selected outputId retrieved from NvAPI_GPU_GetActiveOutputs or NvAPI_GetView, to identify the target
//                                 output in case multiple targets are associated with the selected hNvDisplay.
//                                 Can be NULL to pick the display output id associated with hNvDisplay.
//                pXOffset and pYOffset(OUT)  - The returned display position Offsets.
//
// RETURN STATUS: NVAPI_OK: call successful.
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found.
//                NVAPI_EXPECTED_DISPLAY_HANDLE: hNvDisplay is not a valid display handle.
//                NVAPI_INVALID_ARGUMENT: pBorderColor is NULL.
//                NVAPI_NOT_SUPPORTED - This API is not supported.
//                NVAPI_EXPECTED_ANALOG_DISPLAY - Selected display is not an analog display.
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GetDisplayPosition(NvDisplayHandle hNvDisplay, NvU32 outputId, NvU32 *pXOffset, NvU32 *pYOffset);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_SetDisplayPosition
//
//   DESCRIPTION: Sets the display position of the associated analog display.
//
//  SUPPORTED OS: Windows Vista and higher
//
//    PARAMETERS: hNvDisplay(IN) - NVIDIA Display selection. It can be NVAPI_DEFAULT_HANDLE or a handle enumerated from NvAPI_EnumNVidiaDisplayHandle().
//                outputId(IN)   - One of the selected outputId retrieved from NvAPI_GPU_GetActiveOutputs or NvAPI_GetView, to identify the target
//                                 output in case multiple targets are associated with the selected hNvDisplay.
//                                 Can be NULL to pick the display output id associated with hNvDisplay.
//                xOffset and yOffset(IN)  - The set display position Offsets.
//
// RETURN STATUS: NVAPI_OK: call successful.
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found.
//                NVAPI_EXPECTED_DISPLAY_HANDLE: hNvDisplay is not a valid display handle.
//                NVAPI_NOT_SUPPORTED - This API is not supported.
//                NVAPI_EXPECTED_ANALOG_DISPLAY - Selected display is not an analog display.
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_SetDisplayPosition(NvDisplayHandle hNvDisplay, NvU32 outputId, NvU32 xOffset, NvU32 yOffset);

///////////////////////////////////////////////////////////////////////////////////
//  GPU topology reconfiguration APIs.
//  Provides ability to define one or more SLI devices and standalone GPU topology.
//
//  NV_GPU_TOPOLOGY - this structure defines a set of all GPUs present in a system.  All GPUs with
//  the same parentNdx value describe a single logical GPU.  GPUs that have a unique parentNdx
//  represent standalone GPUs.
//
//  The values returned in parentNdx are arbitrary.  They are only used to determine which
//  physical GPUs will belong to the same logical-GPU.
//

typedef enum
{
    NV_GPU_TOPOLOGY_STATUS_OK                       = 0x00000000,//SLI is capable, topology "status" field indicates this state.
    NV_GPU_TOPOLOGY_STATUS_INVALID_GPU_COUNT        = 0x00000001,//SLI is NOT capable, "pStatus" param in NvAPI_GetValidGpuTopologies indicates these states.
    NV_GPU_TOPOLOGY_STATUS_OS_NOT_SUPPORTED         = 0x00000002,
    NV_GPU_TOPOLOGY_STATUS_OS_ERROR                 = 0x00000004,
    NV_GPU_TOPOLOGY_STATUS_NO_VIDLINK               = 0x00000008,
    NV_GPU_TOPOLOGY_STATUS_INSUFFICIENT_LINK_WIDTH  = 0x00000010,
    NV_GPU_TOPOLOGY_STATUS_CPU_NOT_SUPPORTED        = 0x00000020,
    NV_GPU_TOPOLOGY_STATUS_GPU_NOT_SUPPORTED        = 0x00000040,
    NV_GPU_TOPOLOGY_STATUS_BUS_NOT_SUPPORTED        = 0x00000080,
    NV_GPU_TOPOLOGY_STATUS_NON_APPROVED_CHIPSET     = 0x00000100,
    NV_GPU_TOPOLOGY_STATUS_VBIOS_NOT_SUPPORTED      = 0x00000200,
    NV_GPU_TOPOLOGY_STATUS_GPU_MISMATCH             = 0x00000400,
    NV_GPU_TOPOLOGY_STATUS_ARCH_MISMATCH            = 0x00000800,
    NV_GPU_TOPOLOGY_STATUS_IMPL_MISMATCH            = 0x00001000,
    NV_GPU_TOPOLOGY_STATUS_REV_MISMATCH             = 0x00002000,
    NV_GPU_TOPOLOGY_STATUS_NON_PCIE_BUS             = 0x00004000,
    NV_GPU_TOPOLOGY_STATUS_FB_MISMATCH              = 0x00008000,
    NV_GPU_TOPOLOGY_STATUS_VBIOS_MISMATCH           = 0x00010000,
    NV_GPU_TOPOLOGY_STATUS_QUADRO_MISMATCH          = 0x00020000,
    NV_GPU_TOPOLOGY_STATUS_BUS_TOPOLOGY_ERROR       = 0x00040000,
    NV_GPU_TOPOLOGY_STATUS_PCI_ID_MISMATCH          = 0x00080000,
    NV_GPU_TOPOLOGY_STATUS_CONFIGSPACE_ACCESS_ERROR = 0x00100000,
    NV_GPU_TOPOLOGY_STATUS_INCONSISTENT_CONFIG_SPACE= 0x00200000,
    NV_GPU_TOPOLOGY_STATUS_CONFIG_NOT_SUPPORTED     = 0x00400000,
    NV_GPU_TOPOLOGY_STATUS_RM_NOT_SUPPORTED         = 0x00800000,
    NV_GPU_TOPOLOGY_STATUS_TOPOLOGY_NOT_ALLOWED     = 0x01000000,
    NV_GPU_TOPOLOGY_STATUS_MOBILE_MISMATCH          = 0x02000000,// The system has a mix of notebook and desktop GPUs
    NV_GPU_TOPOLOGY_STATUS_NO_TOPOLOGIES_IN_HYBRID_POWER_MODE = 0x04000000,
} NV_GPU_TOPOLOGY_STATUS_FLAGS;


typedef enum
{
    NV_SET_GPU_TOPOLOGY_DEFER_APPLY                 = 0x00000001,//calling application controls the reload of the display driver
    NV_SET_GPU_TOPOLOGY_DEFER_3D_APP_SHUTDOWN       = 0x00000002,//calling application will control the shutdown of non-migratable applications holding hw resources
    NV_SET_GPU_TOPOLOGY_DEFER_DISPLAY_RECONFIG      = 0x00000004,//calling application will control the display configuration required for the settopology to work
    NV_SET_GPU_TOPOLOGY_RELOAD_DRIVER               = 0x80000000,//calling application requesting force reload given correct topology.
} NV_SET_GPU_TOPOLOGY_FLAGS;

// All of these flags are Readonly unless otherwise noted in NvAPI_SetGpuTopologies()
typedef enum
{
    NV_GPU_TOPOLOGY_ACTIVE                          = 0x00000001,// This topology is currently active.
    NV_GPU_TOPOLOGY_VIDLINK_PRESENT                 = 0x00000002,// Video link between all GPUs is present. (physically bridged)
    NV_GPU_TOPOLOGY_MULTIGPU                        = 0x00000004,// This is a "Multi-GPU"-labeled topology.
    NV_GPU_TOPOLOGY_GX2_BOARD                       = 0x00000008,// GPUs comprising this topology are Dagwoods.
    NV_GPU_TOPOLOGY_DYNAMIC_NOT_ALLOWED             = 0x00000010,// Dynamically switching to SLI is not allowed (it requires a reboot)
    NV_GPU_TOPOLOGY_ACTIVE_IMPLICIT                 = 0x00000020,//Implicit Read only SLI is ACTIVE on this topology of gpus. NvAPI_SetHybridMode can be used to disable this topology.
    NV_GPU_TOPOLOGY_ENABLE_SLI_BY_DEFAULT           = 0x00000040,// SLI must be enabled by default, otherwise SLI is optional
    NV_GPU_TOPOLOGY_ENABLE_CORELOGIC_BROADCAST      = 0x00000080,// Broadcast mode is enabled in the corelogic chipset.
    NV_GPU_TOPOLOGY_BROADCAST                       = 0x00000100,// Broadcast mode is enabled
    NV_GPU_TOPOLOGY_UNICAST                         = 0x00000200,// Unicast mode enabled
    NV_GPU_TOPOLOGY_COMPUTE                         = 0x00010000,// Gpus in this topology are for SLI compute.
                                                                 // All these GPUs of this SLI compute group can be enum'd using NvAPI_GPU_CudaEnumComputeCapableGpus.
    NV_GPU_TOPOLOGY_SLIMULTIMON                     = 0x00020000,// This topology allows multi-display SLI output
    NV_GPU_TOPOLOGY_VIDLINK_CONNECTOR_PRESENT       = 0x00040000,// Vidlink connectors on all GPUs are present (not not necessarily connected)
    NV_GPU_TOPOLOGY_VIEW_CAN_SPAN_GPUS              = 0x00080000,// This topology allows multi-display SLI output across GPUs
} NV_GPU_TOPOLOGY_FLAGS;

typedef enum
{
    NV_SLI_GROUP_ACTIVE                          = 0x00000001,// This SLI group is currently active.
} NV_SLI_GROUP_FLAGS;

typedef struct
{
    NvU32                   version;                                    //structure version
    NvU32                   gpuCount;                                   //count of GPUs in this topology
    NvPhysicalGpuHandle     hPhysicalGpu[NVAPI_MAX_GPU_PER_TOPOLOGY];   //array of GPU handles
    NvU32                   displayGpuIndex;                            //index of the display GPU owner in the gpu array
    NvU32                   displayOutputTargetMask;                    //target device mask
    NvU32                   flags;                                      //one or more topology flags from NV_GPU_TOPOLOGY_FLAGS
    NvU32                   status;                                     //indicates one of the flags in NV_GPU_TOPOLOGY_STATUS_FLAGS
} NV_GPU_TOPOLOGY;

#define NV_GPU_TOPOLOGY_VER  MAKE_NVAPI_VERSION(NV_GPU_TOPOLOGY,1)

typedef struct
{
    NvU32                   version;                                    //structure version
    NvU64                   topologyMask;                               //mask of indexes in gpuTopo
    NvU32                   flags;                                      //one or more SLI group flags from NV_GPU_SLI_GROUP_FLAGS
} NV_SLI_GROUP;

#define NV_GPU_SLI_GROUP_VER  MAKE_NVAPI_VERSION(NV_SLI_GROUP,1)

typedef struct
{
    NvU32                   version;                                    //structure version
    NvU32                   gpuTopoCount;                               //count of valid topologies
    NV_GPU_TOPOLOGY         gpuTopo[NVAPI_MAX_AVAILABLE_GPU_TOPOLOGIES];//max gputopologies
} NV_GPU_TOPOLOGIES_v1;
#define NV_GPU_TOPOLOGIES_VER_1  MAKE_NVAPI_VERSION(NV_GPU_TOPOLOGIES_v1,1)

typedef struct
{
    NvU32                   version;                                    //structure version
    NvU32                   gpuTopoCount;                               //count of valid topologies
    NV_GPU_TOPOLOGY         gpuTopo[NVAPI_MAX_AVAILABLE_GPU_TOPOLOGIES];//max gputopologies
    NvU32                   sliGroupCount;                              //count of valid SLI groups
    NV_SLI_GROUP            sliGroup[NVAPI_MAX_AVAILABLE_SLI_GROUPS];   //max SLI groups
} NV_GPU_TOPOLOGIES;
#define NV_GPU_TOPOLOGIES_VER  MAKE_NVAPI_VERSION(NV_GPU_TOPOLOGIES,2)
#define NV_GPU_VALID_GPU_TOPOLOGIES NV_GPU_TOPOLOGIES
#define NV_GPU_INVALID_GPU_TOPOLOGIES NV_GPU_TOPOLOGIES
#define NV_GPU_VALID_GPU_TOPOLOGIES_VER  NV_GPU_TOPOLOGIES_VER
#define NV_GPU_INVALID_GPU_TOPOLOGIES_VER  NV_GPU_TOPOLOGIES_VER
#define NV_GPU_VALID_GPU_TOPOLOGIES_v1 NV_GPU_TOPOLOGIES_v1
#define NV_GPU_INVALID_GPU_TOPOLOGIES_v1 NV_GPU_TOPOLOGIES_v1
#define NV_GPU_VALID_GPU_TOPOLOGIES_VER_1  NV_GPU_TOPOLOGIES_VER_1
#define NV_GPU_INVALID_GPU_TOPOLOGIES_VER_1  NV_GPU_TOPOLOGIES_VER_1

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_GetValidGpuTopologies
//
// DESCRIPTION:     This API returns all valid GPU topologies that can be used to configure the physical GPUs
//                  using the NvAPI_SetGpuTopologies API. Also returns the current active topologies.
//
//                  This call returns an array of NV_GPU_TOPOLOGY structs; one for each valid configuration
//                  of GPUs present in the system.  Note that this list is constant while GPUs remain in the
//                  same slots in the system.  It is not affected by which GPUs are presently in use.
//                  NV_GPU_TOPOLOGY.displayGpuIndex returned will match the boot GPU if it exists as an active topology.
//                  If it not an active topology, it points to the "first" GPU that has a display monitor connected.
//                  This call also returns an array of NV_SLI_GROUP, describing the list of topologies combinations 
//                  that can be enabled at the same time.
//
//  SUPPORTED OS: Windows XP and higher
// PARAMETERS:      ptopology(OUT): An array of *pCount (OUT) topology structures. Use NvAPI_SetGpuTopology() to set 
//                  up one or several of these GPU topologies.
//                  sliGroup(OUT): An array of *sliGroupCount (OUT) sli goup structures, describing which topologies
//                  can be set up concurrently.
//                  pStatus(OUT): Any system status returned in case zero topology is retrieved. 
//                  System status is one or more flags in NV_GPU_TOPOLOGY_STATUS_FLAGS when SLI is NOT capable.
//
// RETURN STATUS    NVAPI_OK: Call succeeded; 1 or more GPU topologies were returned
//                  NVAPI_INVALID_ARGUMENT: one or more args are invalid
//                  NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVidia GPU driving a display was found
//                  NVAPI_ERROR: Miscellaneous Error.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GetValidGpuTopologies(NV_GPU_VALID_GPU_TOPOLOGIES *pTopology, NvU32 *pStatus);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_GetInvalidGpuTopologies
//
// DESCRIPTION:     This API returns all invalid GPU topologies and the current active topologies.
//
//                  This call returns an array of NV_GPU_TOPOLOGY structs; one for each invalid configuration
//                  of GPUs present in the system. Note that this list is constant while GPUs remain in the
//                  same slots in the system.  It is not affected by which GPUs that are presently in use.
//
//                  The topologies with the status NV_GPU_TOPOLOGY_STATUS_TOPOLOGY_NOT_ALLOWED are valid for
//                  the system but not for productization.
//
//  SUPPORTED OS: Windows XP and higher
// PARAMETERS:      ptopology(OUT): An array of *pCount (OUT) topology structures.
//                  sliGroup, *sliGroupCount (OUT): No SLI groups returned here. *sliGroupCount = 0.
//
// RETURN STATUS    NVAPI_OK: Call succeeded; 1 or more GPU topologies were returned
//                  NVAPI_INVALID_ARGUMENT: one or more args are invalid
//                  NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//                  NVAPI_ERROR: Miscellaneous Error.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GetInvalidGpuTopologies(NV_GPU_INVALID_GPU_TOPOLOGIES *pTopology);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_SetGpuTopologies
//
// DESCRIPTION:     This API lets the caller configure the physical GPUs in the system into one or more logical devices
//                  defined by the NV_GPU_TOPOLOGY structure.
//                  Recommended that the caller application:
//                  - save the current GPU topology retrieved from NvAPI_EnumLogicalGPUs and NvAPI_GetPhysicalGPUsFromLogicalGPU APIs.
//                  - save the current view state for associated displays on these GPUs using the GetView and GetDisplayTargets APIs.
//                  - set NV_GPU_TOPOLOGY.displayGpuIndex to the GPU index in the topology with an active display connection.
//                  - if DEFER_3D_APP_SHUTDOWN is not set notify the user that all 3D application will be forced to close.
//                  - itself does not create 3D handles or objects that can block the topology transition.
//                  - On Vista the calling app must run in elevated mode for the transition to succeed.
//                  - On Vista this API can be called from a system service to derive the elevated context of the System service.
//                  - Non migratable apps running can prevent a successful transition if DEFER_3D_APP_SHUTDOWN is set.
//                  - To query non migratable apps use the NvAPI_QueryNonMigratableApps API.
//                  - More than 1 multi-GPU topology being enabled need to have the NV_GPU_TOPOLOGY_SLI_GROUPS flag set
//
//  SUPPORTED OS: Windows XP and higher
//
// PARAMETERS:      pTopology(IN) - a pointer to a array of structure definining the desired GPU topology retrieved
//                  from NvAPI_GetValidGpuTopologies
//                  flags(IN) - See NV_SET_GPU_TOPOLOGY_FLAGS
//
// RETURN STATUS:   NVAPI_OK: Call succeeded. pTopology.gputTopo[].flags indicates the new status. Reenum all GPU handles after this call.
//                  NVAPI_API_NOT_INTIALIZED: NVAPI not initialized
//                  NVAPI_INVALID_ARGUMENT: one or more args are invalid.
//                  NVAPI_INCOMPATIBLE_STRUCT_VERSION - NV_CPU_INFO version not compatible with driver
//                  NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found.
//                  NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE - handle passed is not a physical GPU handle
//                  NVAPI_INVALID_HANDLE: physical handle is invalid
//                  NVAPI_ERROR: check the status returned in pTopology->status.
//                  NVAPI_TIMEOUT_RECONFIGURING_GPU_TOPO: timeout while reconfiguring GPUs
//                  NVAPI_IMPLICIT_SET_GPU_TOPOLOGY_CHANGE_NOT_ALLOWED: there is any implicit GPU topo active. Use NVAPI_SetHybridMode to change topology.
//                  NVAPI_REQUEST_USER_TO_CLOSE_NON_MIGRATABLE_APPS: Prompt the user to close all non-migratable apps.
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_SetGpuTopologies(NV_GPU_VALID_GPU_TOPOLOGIES *pTopology, NvU32 flags);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:  NvAPI_GPU_GetPerGpuTopologyStatus
//
// DESCRIPTION:    Returns per GPU topology state flags from NV_GPU_TOPOLOGY_STATUS_FLAGS for the queried GPU handle.
//
//  SUPPORTED OS: Windows XP and higher
//
// PARAMETERS:     hPhysicalGPU(IN) - GPU selection.
//                 pStatus(OUT)     - Indicates one or more flags from NV_GPU_TOPOLOGY_STATUS_FLAGS which are the subset of the
//                                    same flags retrieved from NV_GPU_TOPOLOGY.status or pStatus in NvAPI_GetValidGpuTopologies API.
//                 Note: The per GPU topology status can be queried independent of the whether the queried GPU is part of a topology or not.
//
// RETURN STATUS:
//    NVAPI_OK - completed request
//    NVAPI_ERROR - miscellaneous error occurred
//    NVAPI_HANDLE_INVALIDATED - handle passed has been invalidated (see user guide)
//    NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE - handle passed is not a physical GPU handle
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetPerGpuTopologyStatus(NvPhysicalGpuHandle hPhysicalGpu, NvU32 *pStatus);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_SYS_GetChipSetTopologyStatus
//
// DESCRIPTION:   Returns topology state flags from NV_GPU_TOPOLOGY_STATUS_FLAGS possible with the System ChipSet.
//
//  SUPPORTED OS: Windows XP and higher
//
// PARAMETERS:    pStatus(OUT)     - Indicates one or more flags from NV_GPU_TOPOLOGY_STATUS_FLAGS which are the subset of the
//                                   same flags retrieved from NV_GPU_TOPOLOGY.status or pStatus in NvAPI_GetValidGpuTopologies API.
// RETURN STATUS:
//    NVAPI_OK    - completed request
//    NVAPI_ERROR - miscellaneous error occurred
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_SYS_GetChipSetTopologyStatus(NvU32 *pStatus);

///////////////////////////////////////////////////////////////////////////////////
//  I2C API
//  Provides ability to read or write data using I2C protocol.
//  These APIs allow I2C access only to DDC monitors

#define NVAPI_MAX_SIZEOF_I2C_DATA_BUFFER 4096
#define NVAPI_DISPLAY_DEVICE_MASK_MAX 24

typedef struct
{
    NvU32                   version;        //structure version
    NvU32                   displayMask;    //the Display Mask of the concerned display
    NvU8                    bIsDDCPort;     //Flag indicating DDC port or a communication port
    NvU8                    i2cDevAddress;  //the I2C target device address
    NvU8*                   pbI2cRegAddress;//the I2C target register address
    NvU32                   regAddrSize;    //the size in bytes of target register address
    NvU8*                   pbData;         //The buffer of data which is to be read/written
    NvU32                   cbSize;         //The size of Data buffer to be read.
    NvU32                   i2cSpeed;       //The speed at which the transaction is be made(between 28kbps to 40kbps)
} NV_I2C_INFO;

#define NV_I2C_INFO_VER  MAKE_NVAPI_VERSION(NV_I2C_INFO,1)
/***********************************************************************************/

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:  NvAPI_I2CRead
//
// DESCRIPTION:    Read data buffer from I2C port
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// PARAMETERS:     hPhysicalGPU(IN) - GPU selection.
//                 NV_I2C_INFO *pI2cInfo -The I2c data input structure
//
// RETURN STATUS:
//    NVAPI_OK - completed request
//    NVAPI_ERROR - miscellaneous error occurred
//    NVAPI_HANDLE_INVALIDATED - handle passed has been invalidated (see user guide)
//    NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE - handle passed is not a physical GPU handle
//    NVAPI_INCOMPATIBLE_STRUCT_VERSION - structure version is not supported
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_I2CRead(NvPhysicalGpuHandle hPhysicalGpu, NV_I2C_INFO *pI2cInfo);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:  NvAPI_I2CWrite
//
// DESCRIPTION:    Writes data buffer to I2C port
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// PARAMETERS:     hPhysicalGPU(IN) - GPU selection.
//                 NV_I2C_INFO *pI2cInfo -The I2c data input structure
//
// RETURN STATUS:
//    NVAPI_OK - completed request
//    NVAPI_ERROR - miscellaneous error occurred
//    NVAPI_HANDLE_INVALIDATED - handle passed has been invalidated (see user guide)
//    NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE - handle passed is not a physical GPU handle
//    NVAPI_INCOMPATIBLE_STRUCT_VERSION - structure version is not supported
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_I2CWrite(NvPhysicalGpuHandle hPhysicalGpu, NV_I2C_INFO *pI2cInfo);

#define NVAPI_I2C_FLAGS_PRIVILEGE           0x1
#define NVAPI_I2C_FLAGS_DATA_ENCRYPTED      0x2 // Currently Encrypted I2C is not supported
#define NVAPI_I2C_FLAGS_NONSTD_SI1930UC     0x4
#define NVAPI_I2C_FLAGS_PX3540              0x10

typedef struct
{
    NvU32                   flags;        // I2C flags for Priviledged, Encrypted and non-std Si1930uC i2c access
    NvU32                   encrClientID; // Client ID for Encrypted I2C
} NV_I2C_INFO_EX;

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:  NvAPI_I2CWriteEx
//
// DESCRIPTION:    Writes data buffer to I2C port
//                 Please use DisplayMask as Zero, If I2C access required for non display devices
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// PARAMETERS:     hPhysicalGPU(IN) - GPU selection.
//                 NV_I2C_INFO *pI2cInfo -The I2c data input structure
//                 NV_I2C_INFO_EX *pI2cInfoEx - The I2c extended data input structure
//
// RETURN STATUS:
//    NVAPI_OK - completed request
//    NVAPI_ERROR - miscellaneous error occurred
//    NVAPI_HANDLE_INVALIDATED - handle passed has been invalidated (see user guide)
//    NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE - handle passed is not a physical GPU handle
//    NVAPI_INCOMPATIBLE_STRUCT_VERSION - structure version is not supported
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_I2CWriteEx(NvPhysicalGpuHandle hPhysicalGpu, NV_I2C_INFO *pI2cInfo, NV_I2C_INFO_EX *pI2cInfoEx);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:  NvAPI_I2CReadEx
//
// DESCRIPTION:    Read data buffer from I2C port
//                 Please use DisplayMask as Zero, If I2C access required for non display devices
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// PARAMETERS:     hPhysicalGPU(IN) - GPU selection.
//                 NV_I2C_INFO *pI2cInfo -The I2c data input structure
//                 NV_I2C_INFO_EX *pI2cInfoEx - The I2c extended data input structure
//
// RETURN STATUS:
//    NVAPI_OK - completed request
//    NVAPI_ERROR - miscellaneous error occurred
//    NVAPI_HANDLE_INVALIDATED - handle passed has been invalidated (see user guide)
//    NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE - handle passed is not a physical GPU handle
//    NVAPI_INCOMPATIBLE_STRUCT_VERSION - structure version is not supported
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_I2CReadEx(NvPhysicalGpuHandle hPhysicalGpu, NV_I2C_INFO *pI2cInfo, NV_I2C_INFO_EX *pI2cInfoEx);

// END OF I2C API


///////////////////////////////////////////////////////////////////////////////
// POWERMIZER APIs
//
// Provides the ability to Limit PowerMizer's Maximum Performance.
// Grants access on Adaptive Clocking turn on and off.
// PowerMizer can be either Soft Limited or Hard Limited.
// Soft Limit can be exceeded by the adaptive systems in the GPU if there is a need.
// Hard Limit cannot be exceeded even if there is a need to exceed this limit.
//
//////////////////////////////////////////////////////////////////////////////
typedef enum _NV_LEVEL_INFO
{
    NVAPI_PWR_MZR_HARD_LIMIT_MAX                        = 0x00000001,       // Power Mizer Maximum Performance for Hard limit
    NVAPI_PWR_MZR_HARD_LIMIT_BAL                        = 0x00000002,       // Power Mizer Balanced Performance for Hard limit
    NVAPI_PWR_MZR_MAX_BATT                              = 0x00000003,       // Power Mizer Maximum Battery Performance
    NVAPI_PWR_MZR_SOFT_LIMIT_MAX                        = 0x00000004,       // Power mizer Maximum performance for Soft limit
    NVAPI_PWR_MZR_SOFT_LIMIT_BAL                        = 0x00000005,       // Power mizer Balanced performance for Soft limit
    NVAPI_ADC_OFF                                       = 0x00000006,       // Adaptive Clocking Disable
    NVAPI_ADC_ON                                        = 0x00000007,       // Adaptive Clocking Enable
} NV_LEVEL_INFO;


typedef enum _NV_PWR_SOURCE_INFO
{
    NVAPI_PWR_SOURCE_AC                                 = 0x00000001,       // Power source AC
    NVAPI_PWR_SOURCE_BATT                               = 0x00000002,       // Power source Battery
} NV_PWR_SOURCE_INFO;

typedef enum _NV_SELECT_INFO
{
    NVAPI_INDEX_PWR_MZR_HARD                            = 0x00000001,       // To set/get PowerMizer Hard limits. Hard limits modifies the hardware limits.
    NVAPI_INDEX_PWR_MZR_SOFT                            = 0x00000002,       // To set/get PowerMizer Soft limits. Soft limits sets the application preference and could be exceeded upto Hard limits if required by the system.
    NVAPI_INDEX_ADC                                     = 0x00000003,       // To set/get Adaptive Clocking parameters where the driver automatically selects the limits.
} NV_SELECT_INFO;


///////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME:    NvAPI_GPU_GetPowerMizerInfo
//
// DESCRIPTION:      Gets the PowerMizer Maximum Limit for Battery or AC
//                   Gets the Adaptive Clocking status for Battery or AC
//
//  SUPPORTED OS: Windows XP and higher
//
// PARAMETERS:       hPhysicalGPU(IN)         GPU selection.
//                   powerSource              Power source selection with one of the values from NV_PWR_SOURCE_INFO.
//                   select                   PowerMizer type selection with one of the values from NV_SELECT_INFO.
//                   pLevel                   Pointer to return value
//
// RETURN STATUS:
//                   NVAPI_OK                 completed request
//                   NVAPI_NOT_SUPPORTED      Power Mizer Not supported
//                   NVAPI_ERROR              Invalid return to API
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetPowerMizerInfo(NvPhysicalGpuHandle hPhysicalGpu, NV_PWR_SOURCE_INFO powerSourceInfo,
                                          NV_SELECT_INFO select, NV_LEVEL_INFO *pLevelInfo);

///////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME:    NvAPI_GPU_SetPowerMizerInfo
//
// DESCRIPTION:      Sets the PowerMizer Maximum Limit for both Battery and/or AC
//
//  SUPPORTED OS: Windows XP and higher
//
// PARAMETERS:       hPhysicalGPU(IN)          GPU selection
//                   powerSource               Power source selection with one of the values from NV_PWR_SOURCE_INFO.
//                   select                    PowerMizer type selection with one of the values from NV_SELECT_INFO.
//                   level                     Level that has to be set on PwrMzr /Adaptive clocking
//
// RETURN STATUS:    NVAPI_OK                  completed request
//                   NVAPI_NOT_SUPPORTED       PowerMizer Not supported
//                   NVAPI_INVALID_ARGUMENT    Invalid arguments
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_SetPowerMizerInfo(NvPhysicalGpuHandle hPhysicalGpu, NV_PWR_SOURCE_INFO powerSourceInfo,
                                         NV_SELECT_INFO select, NV_LEVEL_INFO levelInfo);


typedef struct
{
    NvU32               version;            //structure version
    NvU32               vendorId;           //vendorId
    NvU32               deviceId;           //deviceId
    NvAPI_ShortString   szVendorName;       //vendor Name
    NvAPI_ShortString   szChipsetName;      //device Name
    NvU32               flags;              //Chipset info flags - obsolete
    NvU32               subSysVendorId;     //subsystem vendorId
    NvU32               subSysDeviceId;     //subsystem deviceId
    NvAPI_ShortString   szSubSysVendorName; //subsystem vendor Name
} NV_CHIPSET_INFO;

#define NV_CHIPSET_INFO_VER     MAKE_NVAPI_VERSION(NV_CHIPSET_INFO,3)

typedef enum
{
    NV_CHIPSET_INFO_HYBRID          = 0x00000001,
} NV_CHIPSET_INFO_FLAGS;

typedef struct
{
    NvU32               version;        //structure version
    NvU32               vendorId;       //vendorId
    NvU32               deviceId;       //deviceId
    NvAPI_ShortString   szVendorName;   //vendor Name
    NvAPI_ShortString   szChipsetName;  //device Name
    NvU32               flags;          //Chipset info flags
} NV_CHIPSET_INFO_v2;

#define NV_CHIPSET_INFO_VER_2   MAKE_NVAPI_VERSION(NV_CHIPSET_INFO_v2,2)

typedef struct
{
    NvU32               version;        //structure version
    NvU32               vendorId;       //vendorId
    NvU32               deviceId;       //deviceId
    NvAPI_ShortString   szVendorName;   //vendor Name
    NvAPI_ShortString   szChipsetName;  //device Name
} NV_CHIPSET_INFO_v1;

#define NV_CHIPSET_INFO_VER_1  MAKE_NVAPI_VERSION(NV_CHIPSET_INFO_v1,1)

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_SYS_GetChipSetInfo
//
//   DESCRIPTION: Returns information about the System's ChipSet
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: pChipSetInfo is NULL
//                NVAPI_OK: *pChipSetInfo is now set
//                NVAPI_INCOMPATIBLE_STRUCT_VERSION - NV_CHIPSET_INFO version not compatible with driver
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_SYS_GetChipSetInfo(NV_CHIPSET_INFO *pChipSetInfo);

typedef struct
{
    NvU32 version;    // Structure version, constructed from macro below
    NvU32 currentLidState;
    NvU32 currentDockState;
    NvU32 currentLidPolicy;
    NvU32 currentDockPolicy;
    NvU32 forcedLidMechanismPresent;
    NvU32 forcedDockMechanismPresent;
}NV_LID_DOCK_PARAMS;

#define NV_LID_DOCK_PARAMS_VER  MAKE_NVAPI_VERSION(NV_LID_DOCK_PARAMS,1)

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GetLidDockInfo
//
// DESCRIPTION: Returns current lid and dock information.
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// RETURN STATUS: NVAPI_OK: now *pLidAndDock contains the returned lid and dock information.
//                NVAPI_ERROR:If any way call is not success.
//                NVAPI_NOT_SUPPORTED:If any way call is not success.
//                NVAPI_HANDLE_INVALIDATED:If nvapi escape result handle is invalid.
//                NVAPI_API_NOT_INTIALIZED:If NVAPI is not initialized.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_SYS_GetLidAndDockInfo(NV_LID_DOCK_PARAMS *pLidAndDock);


///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_OGL_ExpertModeSet[Get]
//
//   DESCRIPTION: Configure OpenGL Expert Mode, an API usage feedback and
//                advice reporting mechanism. The effects of this call are
//                applied only to the current context, and are reset to the
//                defaults when the context is destroyed.
//
//                Note: This feature is valid at runtime only when GLExpert
//                      functionality has been built into the OpenGL driver
//                      installed on the system. All Windows Vista OpenGL
//                      drivers provided by NVIDIA have this instrumentation
//                      included by default. Windows XP, however, requires a
//                      special display driver available with the NVIDIA
//                      PerfSDK found at developer.nvidia.com.
//
//                Note: These functions are valid only for the current OpenGL
//                      context. Calling these functions prior to creating a
//                      context and calling MakeCurrent with it will result
//                      in errors and undefined behavior.
//
//    PARAMETERS: expertDetailMask  Mask made up of NVAPI_OGLEXPERT_DETAIL bits,
//                                  this parameter specifies the detail level in
//                                  the feedback stream.
//
//                expertReportMask  Mask made up of NVAPI_OGLEXPERT_REPORT bits,
//                                  this parameter specifies the areas of
//                                  functional interest.
//
//                expertOutputMask  Mask made up of NVAPI_OGLEXPERT_OUTPUT bits,
//                                  this parameter specifies the feedback output
//                                  location.
//
//                expertCallback    Used in conjunction with OUTPUT_TO_CALLBACK,
//                                  this is a simple callback function the user
//                                  may use to obtain the feedback stream. The
//                                  function will be called once per fully
//                                  qualified feedback stream entry.
//
// RETURN STATUS: NVAPI_API_NOT_INTIALIZED         : NVAPI not initialized
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND    : no NVIDIA GPU found
//                NVAPI_OPENGL_CONTEXT_NOT_CURRENT : no NVIDIA OpenGL context
//                                                   which supports GLExpert
//                                                   has been made current
//                NVAPI_ERROR : OpenGL driver failed to load properly
//                NVAPI_OK    : success
//
///////////////////////////////////////////////////////////////////////////////
#define NVAPI_OGLEXPERT_DETAIL_NONE                 0x00000000
#define NVAPI_OGLEXPERT_DETAIL_ERROR                0x00000001
#define NVAPI_OGLEXPERT_DETAIL_SWFALLBACK           0x00000002
#define NVAPI_OGLEXPERT_DETAIL_BASIC_INFO           0x00000004
#define NVAPI_OGLEXPERT_DETAIL_DETAILED_INFO        0x00000008
#define NVAPI_OGLEXPERT_DETAIL_PERFORMANCE_WARNING  0x00000010
#define NVAPI_OGLEXPERT_DETAIL_QUALITY_WARNING      0x00000020
#define NVAPI_OGLEXPERT_DETAIL_USAGE_WARNING        0x00000040
#define NVAPI_OGLEXPERT_DETAIL_ALL                  0xFFFFFFFF

#define NVAPI_OGLEXPERT_REPORT_NONE                 0x00000000
#define NVAPI_OGLEXPERT_REPORT_ERROR                0x00000001
#define NVAPI_OGLEXPERT_REPORT_SWFALLBACK           0x00000002
#define NVAPI_OGLEXPERT_REPORT_PIPELINE_VERTEX      0x00000004
#define NVAPI_OGLEXPERT_REPORT_PIPELINE_GEOMETRY    0x00000008
#define NVAPI_OGLEXPERT_REPORT_PIPELINE_XFB         0x00000010
#define NVAPI_OGLEXPERT_REPORT_PIPELINE_RASTER      0x00000020
#define NVAPI_OGLEXPERT_REPORT_PIPELINE_FRAGMENT    0x00000040
#define NVAPI_OGLEXPERT_REPORT_PIPELINE_ROP         0x00000080
#define NVAPI_OGLEXPERT_REPORT_PIPELINE_FRAMEBUFFER 0x00000100
#define NVAPI_OGLEXPERT_REPORT_PIPELINE_PIXEL       0x00000200
#define NVAPI_OGLEXPERT_REPORT_PIPELINE_TEXTURE     0x00000400
#define NVAPI_OGLEXPERT_REPORT_OBJECT_BUFFEROBJECT  0x00000800
#define NVAPI_OGLEXPERT_REPORT_OBJECT_TEXTURE       0x00001000
#define NVAPI_OGLEXPERT_REPORT_OBJECT_PROGRAM       0x00002000
#define NVAPI_OGLEXPERT_REPORT_OBJECT_FBO           0x00004000
#define NVAPI_OGLEXPERT_REPORT_FEATURE_SLI          0x00008000
#define NVAPI_OGLEXPERT_REPORT_ALL                  0xFFFFFFFF

#define NVAPI_OGLEXPERT_OUTPUT_TO_NONE              0x00000000
#define NVAPI_OGLEXPERT_OUTPUT_TO_CONSOLE           0x00000001
#define NVAPI_OGLEXPERT_OUTPUT_TO_DEBUGGER          0x00000004
#define NVAPI_OGLEXPERT_OUTPUT_TO_CALLBACK          0x00000008
#define NVAPI_OGLEXPERT_OUTPUT_TO_ALL               0xFFFFFFFF

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION TYPE: NVAPI_OGLEXPERT_CALLBACK
//
//   DESCRIPTION: Used in conjunction with OUTPUT_TO_CALLBACK, this is a simple
//                callback function the user may use to obtain the feedback
//                stream. The function will be called once per fully qualified
//                feedback stream entry.
//
//    PARAMETERS: categoryId   Contains the bit from the NVAPI_OGLEXPERT_REPORT
//                             mask that corresponds to the current message
//                messageId    Unique Id for the current message
//                detailLevel  Contains the bit from the NVAPI_OGLEXPERT_DETAIL
//                             mask that corresponds to the current message
//                objectId     Unique Id of the object that corresponds to the
//                             current message
//                messageStr   Text string from the current message
//
///////////////////////////////////////////////////////////////////////////////
typedef void (* NVAPI_OGLEXPERT_CALLBACK) (unsigned int categoryId, unsigned int messageId, unsigned int detailLevel, int objectId, const char *messageStr);

//  SUPPORTED OS: Windows XP and higher
NVAPI_INTERFACE NvAPI_OGL_ExpertModeSet(NvU32 expertDetailLevel,
                                        NvU32 expertReportMask,
                                        NvU32 expertOutputMask,
                     NVAPI_OGLEXPERT_CALLBACK expertCallback);


//  SUPPORTED OS: Windows XP and higher
NVAPI_INTERFACE NvAPI_OGL_ExpertModeGet(NvU32 *pExpertDetailLevel,
                                        NvU32 *pExpertReportMask,
                                        NvU32 *pExpertOutputMask,
                     NVAPI_OGLEXPERT_CALLBACK *pExpertCallback);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_OGL_ExpertModeDefaultsSet[Get]
//
//   DESCRIPTION: Configure OpenGL Expert Mode global defaults. These settings
//                apply to any OpenGL application which starts up after these
//                values are applied (i.e. these settings *do not* apply to
//                currently running applications).
//
//    PARAMETERS: expertDetailLevel Value which specifies the detail level in
//                                  the feedback stream. This is a mask made up
//                                  of NVAPI_OGLEXPERT_LEVEL bits.
//
//                expertReportMask  Mask made up of NVAPI_OGLEXPERT_REPORT bits,
//                                  this parameter specifies the areas of
//                                  functional interest.
//
//                expertOutputMask  Mask made up of NVAPI_OGLEXPERT_OUTPUT bits,
//                                  this parameter specifies the feedback output
//                                  location. Note that using OUTPUT_TO_CALLBACK
//                                  here is meaningless and has no effect, but
//                                  using it will not cause an error.
//
// RETURN STATUS: NVAPI_ERROR or NVAPI_OK
//
///////////////////////////////////////////////////////////////////////////////

//  SUPPORTED OS: Windows XP and higher
NVAPI_INTERFACE NvAPI_OGL_ExpertModeDefaultsSet(NvU32 expertDetailLevel,
                                                NvU32 expertReportMask,
                                                NvU32 expertOutputMask);

//  SUPPORTED OS: Windows XP and higher
NVAPI_INTERFACE NvAPI_OGL_ExpertModeDefaultsGet(NvU32 *pExpertDetailLevel,
                                                NvU32 *pExpertReportMask,
                                                NvU32 *pExpertOutputMask);

#define NVAPI_MAX_VIEW_TARGET  2

typedef enum _NV_TARGET_VIEW_MODE
{
    NV_VIEW_MODE_STANDARD  = 0,
    NV_VIEW_MODE_CLONE     = 1,
    NV_VIEW_MODE_HSPAN     = 2,
    NV_VIEW_MODE_VSPAN     = 3,
    NV_VIEW_MODE_DUALVIEW  = 4,
    NV_VIEW_MODE_MULTIVIEW = 5,
} NV_TARGET_VIEW_MODE;


// Following definitions are used in NvAPI_SetViewEx.
// Scaling modes
typedef enum _NV_SCALING
{
    NV_SCALING_DEFAULT          = 0,        // No change
    NV_SCALING_MONITOR_SCALING  = 1,
    NV_SCALING_ADAPTER_SCALING  = 2,
    NV_SCALING_CENTERED         = 3,
    NV_SCALING_ASPECT_SCALING   = 5,
    NV_SCALING_CUSTOMIZED       = 255       // For future use
} NV_SCALING;

// Rotate modes
typedef enum _NV_ROTATE
{
    NV_ROTATE_0           = 0,
    NV_ROTATE_90          = 1,
    NV_ROTATE_180         = 2,
    NV_ROTATE_270         = 3
} NV_ROTATE;

// Color formats
typedef enum _NV_FORMAT
{
    NV_FORMAT_UNKNOWN       =  0,       // unknown. Driver will choose one as following value.
    NV_FORMAT_P8            = 41,       // for 8bpp mode
    NV_FORMAT_R5G6B5        = 23,       // for 16bpp mode
    NV_FORMAT_A8R8G8B8      = 21,       // for 32bpp mode
    NV_FORMAT_A16B16G16R16F = 113       // for 64bpp(floating point) mode.
} NV_FORMAT;

// TV standard


///////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME:   NvAPI_SetDisplaySettings
//
// DESCRIPTION:     This API caller set the display settings for a selected display source.
//
//  SUPPORTED OS: Windows Vista and higher
//
// PARAMETERS:      hNvDisplay(IN) - NVIDIA Display selection. It can be NVAPI_DEFAULT_HANDLE or a handle enumerated from NvAPI_EnumNVidiaDisplayHandle().
//                  pPaths(IN) - Detailed target display arrangement for clone, span and edge blending display modes.
//                  pathCount(IN) - Count of targets for the selected display source.
//                  Note: Display PATH with this API is limited to single GPU. DUALVIEW across GPUs cannot be enabled with this API. 
//                        Please use NvAPI_SYS_SetDisplayTopologies for enabling views across GPUs.
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_ERROR - miscellaneous error occurred
//                  NVAPI_INVALID_ARGUMENT: Invalid input parameter.
//
///////////////////////////////////////////////////////////////////////////////

// the connector is not support yet, must be set to _AUTO
typedef enum _NV_CONNECTOR
{
    NV_CONN_AUTO = 0,
}NV_CONNECTOR;

// the timing override is not support yet, must be set to _AUTO
typedef enum _NV_TIMING_OVERRIDE
{
    NV_TIMING_OVERRIDE_CURRENT = 0,          // get the current timing
    NV_TIMING_OVERRIDE_AUTO,                 // the timing the driver will use based the current policy
    NV_TIMING_OVERRIDE_EDID,                 // EDID timing
    NV_TIMING_OVERRIDE_DMT,                  // VESA DMT timing
    NV_TIMING_OVERRIDE_DMT_RB,               // VESA DMT timing with reduced blanking
    NV_TIMING_OVERRIDE_CVT,                  // VESA CVT timing
    NV_TIMING_OVERRIDE_CVT_RB,               // VESA CVT timing with reduced blanking
    NV_TIMING_OVERRIDE_GTF,                  // VESA GTF timing
    NV_TIMING_OVERRIDE_EIA861,               // EIA 861x pre-defined timing
    NV_TIMING_OVERRIDE_ANALOG_TV,            // analog SD/HDTV timing
    NV_TIMING_OVERRIDE_CUST,                 // NV custom timings
    NV_TIMING_OVERRIDE_NV_PREDEFINED,        // NV pre-defined timing (basically the PsF timings)

    NV_TIMING_OVRRIDE_MAX,
}NV_TIMING_OVERRIDE;

typedef struct
{
    float x;    //the x-coordinate of the viewport top-left point
    float y;    //the y-coordinate of the viewport top-left point
    float w;    //the width of the viewport
    float h;    //the height of the viewport
} NV_VIEWPORTF;

//***********************
// The Timing Structure
//***********************
//
// NVIDIA specific timing extras
typedef struct tagNV_TIMINGEXT
{
    NvU32   flag;          // reserve for NV h/w based enhancement like double-scan.
    NvU16   rr;            // the logical refresh rate to present
    NvU32   rrx1k;         // the physical vertical refresh rate in 0.001Hz
    NvU32   aspect;        // the display aspect ratio Hi(aspect):horizontal-aspect, Low(aspect):vertical-aspect
    NvU16   rep;           // bit wised pixel repetition factor: 0x1:no pixel repetition; 0x2:each pixel repeats twice horizontally,..
    NvU32   status;        // the timing standard being used
    NvU8    name[40];      // the name of the timing
}NV_TIMINGEXT;

//
//
//The very basic timing structure based on the VESA standard:
//
//           |<----------------------------htotal--------------------------->|
//            ---------"active" video-------->|<-------blanking------>|<-----
//           |<-------hvisible-------->|<-hb->|<-hfp->|<-hsw->|<-hbp->|<-hb->|
// ----------+-------------------------+      |       |       |       |      |
//   A     A |                         |      |       |       |       |      |
//   :     : |                         |      |       |       |       |      |
//   :     : |                         |      |       |       |       |      |
//   :verical|    addressable video    |      |       |       |       |      |
//   :visible|                         |      |       |       |       |      |
//   :     : |                         |      |       |       |       |      |
//   :     : |                         |      |       |       |       |      |
// verical V |                         |      |       |       |       |      |
//  total  --+-------------------------+      |       |       |       |      |
//   :     vb         border                  |       |       |       |      |
//   :     -----------------------------------+       |       |       |      |
//   :     vfp        front porch                     |       |       |      |
//   :     -------------------------------------------+       |       |      |
//   :     vsw        sync width                              |       |      |
//   :     ---------------------------------------------------+       |      |
//   :     vbp        back porch                                      |      |
//   :     -----------------------------------------------------------+      |
//   V     vb         border                                                 |
// --------------------------------------------------------------------------+
//
typedef struct tagNV_TIMING
{
    // VESA scan out timing parameters:
    NvU16 HVisible;         //horizontal visible
    NvU16 HBorder;          //horizontal border
    NvU16 HFrontPorch;      //horizontal front porch
    NvU16 HSyncWidth;       //horizontal sync width
    NvU16 HTotal;           //horizontal totel
    NvU8  HSyncPol;         //horizontal sync polarity: 1-negative, 0-positive

    NvU16 VVisible;         //vertical visible
    NvU16 VBorder;          //vertical border
    NvU16 VFrontPorch;      //vertical front porch
    NvU16 VSyncWidth;       //vertical sync width
    NvU16 VTotal;           //vertical total
    NvU8  VSyncPol;         //vertical sync polarity: 1-negative, 0-positive

    NvU16 interlaced;       //1-interlaced, 0-progressive
    NvU32 pclk;             //pixel clock in 10KHz

    //other timing related extras
    NV_TIMINGEXT etc;
}NV_TIMING;

// timing related constants:
#define NV_TIMING_H_SYNC_POSITIVE                             0
#define NV_TIMING_H_SYNC_NEGATIVE                             1
#define NV_TIMING_H_SYNC_DEFAULT                              NV_TIMING_H_SYNC_NEGATIVE
//
#define NV_TIMING_V_SYNC_POSITIVE                             0
#define NV_TIMING_V_SYNC_NEGATIVE                             1
#define NV_TIMING_V_SYNC_DEFAULT                              NV_TIMING_V_SYNC_POSITIVE
//
#define NV_TIMING_PROGRESSIVE                                 0
#define NV_TIMING_INTERLACED                                  1
#define NV_TIMING_INTERLACED_EXTRA_VBLANK_ON_FIELD2           1
#define NV_TIMING_INTERLACED_NO_EXTRA_VBLANK_ON_FIELD2        2

typedef enum _NVAPI_TIMING_TYPE
{
    NV_TIMING_TYPE_DMT = 1,                                 // DMT
    NV_TIMING_TYPE_GTF,                                     // GTF
    NV_TIMING_TYPE_ASPR,                                    // wide aspect ratio timing, for legacy support only
    NV_TIMING_TYPE_NTSC_TV,                                 // NTSC TV timing. for legacy support only
    NV_TIMING_TYPE_PAL_TV,                                  // PAL TV timing, legacy support only
    NV_TIMING_TYPE_CVT,                                     // CVT timing
    NV_TIMING_TYPE_CVT_RB,                                  // CVT timing with reduced blanking
    NV_TIMING_TYPE_CUST,                                    // Customized timing
    NV_TIMING_TYPE_EDID_DTD,                                // EDID detailed timing
    NV_TIMING_TYPE_EDID_STD,                                // EDID standard timing
    NV_TIMING_TYPE_EDID_EST,                                // EDID established timing
    NV_TIMING_TYPE_EDID_CVT,                                // EDID defined CVT timing (EDID 1.4)
    NV_TIMING_TYPE_EDID_861ST,                              // EDID defined CEA/EIA 861 timing (in the EDID 861 extension)
    NV_TIMING_TYPE_NV_PREDEFINED,                           // NV pre-defined timings (PsF timings)
    NV_TIMING_TYPE_DMT_RB,                                  // DMT timing with reduced blanking
    NV_TIMING_TYPE_EDID_EXT_DTD,                            // EDID detailed timing in the extension
    NV_TIMING_TYPE_SDTV,                                    // SDTV timing (including NTSC, PAL etc)
    NV_TIMING_TYPE_HDTV,                                    // HDTV timing (480p,480i,720p, 1080i etc)
}NVAPI_TIMING_TYPE;


// config on specified display is not supported yet
#define NV_DISP_INDEX_AUTO 0

// the generic display target config info which is independent of any specific mode
typedef struct
{
    NvU32                   version;    // Structure version

    NvU32                   device;     // target display ids or target device mask
    NV_GPU_CONNECTOR_TYPE   connector;

    // the source display index
    NvU32                   srcID;

    // the source importance
    NvU32                   srcImportance; //(OUT) Indicates if this is the GPU's primary view target. This is not the desktop GDI primary.
                                           //NvAPI_SetDisplaySettings automatically selects the first target in NV_DISP_PATH index 0 as the GPU's primary view.
    // the source mode information
    NvU32                   width;
    NvU32                   height;
    NvU32                   depth;
    NV_ROTATE               rotation;
    NV_FORMAT               colorFormat;       // color format. Not used now.

    // the section of the source surface for scan out, defined in normalized desktop coordinates
    NV_VIEWPORTF            srcPartition;      // not used now

    // the scan out viewport in (at the front end, i.e. the compositor/CRTC).
    // defined in the normalized desktop coordinates
    NV_VIEWPORTF            viewportIn ;       // not used now

    // the scaling mode
    NV_SCALING              scaling;

    // the scan out viewport out (at the front end, i.e. the compositor/CRTC).
    // this is for the future arbitrary scaling support (not supported by any the current GPUs)
    // defined in the normalized raster/backend timing coordinates
    // viewportOut is not supported yet, must be set to {0.0, 0.0, 0.0, 0.0}
    NV_VIEWPORTF            viewportOut;       // not used now

    // the backend (raster) timing standard
    NV_TIMING_OVERRIDE      timingOverride;

    NvU32                   refreshRate;        // only used for backward compatible when NV_DISP_PATH_VER1 is specified
    NvU32                   interlaced    :1;   // only used for backward compatible when NV_DISP_PATH_VER1 is specified
    NvU32                   hwModeSetOnly :1;   // if this flag is set, the modeset is a pure h/w modeset without OS update. Only used with NV_DISP_PATH_VER;
    NvU32                   SelectCustomTiming:1;    // For HD modes over DVI to select custom timings
    NvU32                   needNullModeset   :1;    // for read only - indicating a NULL modeset is needed on this monitor (for internal DP link training)
    NvU32                   need6x4Modeset    :1;    // for read only - indicating a 640x480x32bppx60Hz modeset is needed (for DP bad EDID fallback)
    NvU32                   forceModeSet      :1;    // Used only on Win7 and higher during a call to NvAPI_SetDisplaySettings. Turns off optimization & forces OS to set supplied mode.
    NvU32                   gpuId             :24;   // the display/target physical Gpu id which is the owner of the scan out (for SLI multimon, display from the slave Gpu)
    NvU32                   isSliFocusDisplay :1;    // this display path is the sli focus (so far it's read only)

    NV_DISPLAY_TV_FORMAT    tvFormat;           // Valid only on TV device. set to 0 for Other devices.

    NV_TIMING               timing;             // the scan out timing, NV_DISP_PATH_VER2 only, ignored it's on analog TV.

} NV_DISP_PATH;

#define NV_DISP_PATH_VER  NV_DISP_PATH_VER3
#define NV_DISP_PATH_VER3 MAKE_NVAPI_VERSION(NV_DISP_PATH,3)
#define NV_DISP_PATH_VER2 MAKE_NVAPI_VERSION(NV_DISP_PATH,2)
#define NV_DISP_PATH_VER1 MAKE_NVAPI_VERSION(NV_DISP_PATH,1)

///////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME:   NvAPI_SetDisplaySettings
//
// DESCRIPTION:     This API caller set the display settings for selected display sources.
//
//  SUPPORTED OS: Windows Vista and higher
//
// PARAMETERS:      hNvDisplay(IN) - NVIDIA Display selection. NVAPI_DEFAULT_HANDLE not allowed, it has to be a handle enumerated with NvAPI_EnumNVidiaDisplayHandle().
//                  pPaths(IN) - Detailed target display arrangement for clone, span and edge blending display modes.
//                  pathCount(IN) - Count of targets for the selected display source.
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_ERROR - miscellaneous error occurred
//                  NVAPI_INVALID_ARGUMENT: Invalid input parameter.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_SetDisplaySettings(NvDisplayHandle hNvDisplay, NV_DISP_PATH *paths, NvU32 pathCount);

///////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME:   NvAPI_GetDisplaySettings
//
// DESCRIPTION:     This API lets caller retrieve the target display arrangement for a selected display source.
//                  Note: Display PATH with this API is limited to single GPU. DUALVIEW across GPUs will be returned as STANDARD VIEW. 
//                        Please use NvAPI_SYS_GetDisplayTopologies to query views across GPUs.
//
//  SUPPORTED OS: Windows Vista and higher
//
// PARAMETERS:      hNvDisplay(IN) - NVIDIA Display selection. NVAPI_DEFAULT_HANDLE not allowed, it has to be a handle enumerated with NvAPI_EnumNVidiaDisplayHandle().
//                  pPaths(OUT) - Target display informatiom.
//                  pPathCount(OUT) - Count of targets on the selected display source.
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_ERROR - miscellaneous error occurred
//                  NVAPI_INVALID_ARGUMENT: Invalid input parameter.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GetDisplaySettings(NvDisplayHandle hNvDisplay, NV_DISP_PATH *pPaths, NvU32 *pPathCount);

///////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME:   NvAPI_GetTiming
//
// DESCRIPTION:     Calculate the timing from the visible width/height/refresh-rate and timing type info
//
//  SUPPORTED OS: Windows XP and higher
//
// PARAMETERS:      hNvDisplay(IN) - NVIDIA Display selection. NVAPI_DEFAULT_HANDLE not allowed, it has to be a handle enumerated with NvAPI_EnumNVidiaDisplayHandle().
//                  width    - the horizontal visible size
//                  heigth   - the veritical visible size
//                  rr       - the refresh rate of the timing
//                  flag     - the flag to pass in addtional info for timing calculation.
//                  outputId - the monitor Id(mask), only used to get EDID timing or custom timing or NV policy related timing(NV_TIMING_OVERRIDE_AUTO).
//                  type     - the timing type(formula) to use to calculate the timing
//                  pT(OUT)  - the pointer to the NV_TIMING structure
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_ERROR - miscellaneous error occurred
//                  NVAPI_INVALID_ARGUMENT: Invalid input parameter.
//
///////////////////////////////////////////////////////////////////////////////
typedef struct
{
    NvU32                   isInterlaced   : 4;  // to retrieve interlaced/progressive timing
    NvU32                   reserved0      : 12;
    union
    {
        NvU32               tvFormat       : 8;  // the actual analog HD/SDTV format. Used when the timing type is NV_TIMING_OVERRIDE_ANALOG_TV and
                                                 // and width==height==rr==0;
        NvU32               ceaId          : 8;  // the EIA/CEA 861B/D predefined short timing descriptor ID. Used when the timing type is  NV_TIMING_OVERRIDE_EIA861;
                                                 // and width==height==rr==0;
        NvU32               nvPsfId        : 8;  // the NV predefined PsF format Id. Used when the timing type is NV_TIMING_OVERRIDE_NV_PREDEFINED.
    };
    NvU32                   scaling        : 8;  // define prefered scaling
}NV_TIMING_FLAG;

#define NV_GET_CEA_FORMAT(n) (((n)&0x3F800000)>>23)     // Get CEA format (digital TV format) from NV_TIMING(::etc.status)
#define NV_GET_ANALOG_TV_FORMAT(n) ((((n)&0x0FF00)==0x1100||((n)&0x0FF00)==0x1200)?((n)&0x0FF):0)   // Get the analog TV format from NV_TIMING(::etc.status)
#define NV_IS_ANALOG_TV_FORMAT(n)  ((((n)&0x0FF00)==0x1100||((n)&0x0FF00)==0x1200)?1:0)             // Check if the timing(NV_TIMING::etc.status) is an analog TV format

NVAPI_INTERFACE NvAPI_GetTiming(NvDisplayHandle hNvDisplay, NvU32 width, NvU32 height, float rr, NV_TIMING_FLAG flag, NvU32 outputId, NV_TIMING_OVERRIDE type, NV_TIMING *pT);

///////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME:   NvAPI_EnumCustomDisplay
//
// DESCRIPTION:     Calculate the timing from the visible width/height/refresh-rate and timing type info
//
//  SUPPORTED OS: Windows XP and higher
//
// PARAMETERS:      hNvDisplay(IN) - NVIDIA Display selection. NVAPI_DEFAULT_HANDLE not allowed, it has to be a handle enumerated with NvAPI_EnumNVidiaDisplayHandle().
//                  index - the enum index
//                  outputId - the monitor Id only which the custom display config should be applied. "-1" is to enum all custom display config
//                  pC(OUT)  - the pointer to the NV_CUSTOM_DISPLAY structure
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_ERROR - miscellaneous error occurred
//                  NVAPI_INVALID_ARGUMENT: Invalid input parameter.
//
///////////////////////////////////////////////////////////////////////////////

typedef struct
{
    NvU32                   version;

    // the source mode information
    NvU32                   width;                   // source surface(source mode) width
    NvU32                   height;                  // source surface(source mode) height
    NvU32                   depth;                   // source surface color depth."0" means all 8/16/32bpp
    NV_FORMAT               colorFormat;             // color format. (optional)

    NV_VIEWPORTF            srcPartition;            // for multimon support, should be set to (0,0,1.0,1.0) for now.

    float                   xRatio;                  // the horizontal scaling ratio
    float                   yRatio;                  // the vertical scaling ratio

    NV_TIMING               timing;                  // the timing used to program TMDS/DAC/LVDS/HDMI/TVEncoder etc
    NvU32                   hwModeSetOnly : 1;         // if set, it means a h/w modeset without OS update

}NV_CUSTOM_DISPLAY;

#define NV_CUSTOM_DISPLAY_VER  MAKE_NVAPI_VERSION(NV_CUSTOM_DISPLAY,1)
NVAPI_INTERFACE NvAPI_EnumCustomDisplay(NvDisplayHandle hNvDisplay, NvU32 index, NvU32 outputId, NV_CUSTOM_DISPLAY *pC);

///////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME:   NvAPI_TryCustomDisplay
//
// DESCRIPTION:     This API is a wrapper function of NvAPI_SetDisplaySettings to setup a custom display without saving the config.
//
//  SUPPORTED OS: Windows XP and higher
//
// PARAMETERS:      hNvDisplay(IN) - NVIDIA Display selection. NVAPI_DEFAULT_HANDLE not allowed, it has to be a handle enumerated with NvAPI_EnumNVidiaDisplayHandle().
//                  pDispOutId(IN) - the array of the target monitor output Ids
//                  pCustDisp(IN) - a pointer to the NV_CUSTOM_DISPLAY structure.
//                  count(IN) - the total number of the incoming NV_CUSTOM_DISPLAY structure. This is for the multi-head support
//                  hwModeSetOnly(IN) - the option to let the user flush the timing without OS update
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_ERROR - miscellaneous error occurred
//                  NVAPI_INVALID_ARGUMENT: Invalid input parameter.
//                  NVAPI_NO_IMPLEMENTATION:Not implemented.
//                  NVAPI_INCOMPATIBLE_STRUCT_VERSION:the version of NV_CUSTOM_DISPLAY is not supported
//
// NOTES :          In clone mode the timings can applied to both the target monitors but only one target at a time.
//                  For the secondary target the applied timings works under the following conditions:
//                  1. if the secondary monitor EDID supports the selected timing OR
//                  2. if the selected custom timings can be scaled by the secondary monitor for the selected source resolution on the primary OR
//                  3. if the selected custom timings matches the existing source resolution on the primary.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_TryCustomDisplay(NvDisplayHandle hNvDisplay, NvU32 *pDispOutputId, NV_CUSTOM_DISPLAY *pCustDisp, NvU32 count, NvU32 hwModeSetOnly);

///////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME:   NvAPI_RevertCustomDisplayTrial
//
// DESCRIPTION:     This API is used to restore the old display config before NvAPI_TryCustomDisplay is called. This function
//                  can only be called after custom display config is tried on the h/w (i.e. NvAPI_TryCustomDisplay() is called).
//                  Otherwise this function won't take any action.
//                  In XP, it is not supported so it will return NVAPI_NO_IMPLEMENTATION.
//
//  SUPPORTED OS: Windows Vista and higher
//
// PARAMETERS:      hNvDisplay(IN) - NVIDIA Display selection. NVAPI_DEFAULT_HANDLE not allowed, it has to be a handle enumerated with NvAPI_EnumNVidiaDisplayHandle().
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_ERROR - miscellaneous error occurred
//                  NVAPI_INVALID_ARGUMENT: Invalid input parameter.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_RevertCustomDisplayTrial(NvDisplayHandle hNvDisplay);

///////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME:   NvAPI_DeleteCustomDisplay
//
// DESCRIPTION:     Delete the index specified custom display configuration from registry.
//
//  SUPPORTED OS: Windows XP and higher
//
// PARAMETERS:      hNvDisplay(IN) - NVIDIA Display selection. NVAPI_DEFAULT_HANDLE not allowed, it has to be a handle enumerated with NvAPI_EnumNVidiaDisplayHandle().
//                  displayOutputId(IN) - 1. the display Id on which the underscan config is to apply. It has to be a legal display Id (one bit set)
//                                        2. "0" is allowed meaning the default display used by the hNvDisplay handle.
//                                        3. "-1" is also allowed meaning "index" is the absolute index and not displayOutputId related
//                  index - the index of the custom display
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_ERROR - miscellaneous error occurred
//                  NVAPI_INVALID_ARGUMENT: Invalid input parameter.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DeleteCustomDisplay(NvDisplayHandle hNvDisplay, NvU32 displayOutputId, NvU32 index);

///////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME:   NvAPI_SaveCustomDisplay
//
// DESCRIPTION:     Save the current h/w display config on the specified output ID as a custom display configuration
//                  This function should be called right after NvAPI_TryCustomDisplay() to save custom display from the current
//                  h/w context. This function will not do anything if the custom display config is not tried on the h/w.
//
//  SUPPORTED OS: Windows XP and higher
//
// PARAMETERS:      hNvDisplay(IN)      - NVIDIA Display selection. NVAPI_DEFAULT_HANDLE not allowed, it has to be a handle enumerated with NvAPI_EnumNVidiaDisplayHandle().
//                  isThisOutputIdOnly  - if set, the saved custom display will only be applied on the monitor with the same outputId
//                  isThisMonitorIdOnly - if set, the saved custom display will only be applied on the monitor with the same EDID ID or
//                                        the same TV connector in case of analog TV.
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_ERROR - miscellaneous error occurred
//                  NVAPI_INVALID_ARGUMENT: Invalid input parameter.
//
///////////////////////////////////////////////////////////////////////////////

NVAPI_INTERFACE NvAPI_SaveCustomDisplay(NvDisplayHandle hNvDisplay, NvU32 isThisOutputIdOnly, NvU32 isThisMonitorIdOnly);

///////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME:   NvAPI_QueryUnderscanCap
//
// DESCRIPTION:     Get the max underscan/overscan ratio and the support type(s) of underscan.
//
//  SUPPORTED OS: Windows Vista and higher
//
// PARAMETERS:      hNvDisplay(IN)      - NVIDIA Display selection. NVAPI_DEFAULT_HANDLE not allowed, it has to be a handle enumerated with NvAPI_EnumNVidiaDisplayHandle().
//
//                  displayOutputId(IN) - 1. the display Id on which the underscan config is to apply. It has to be a legal display Id (one bit set)
//                                        2. "0" is allowed meaning the default display used by the hNvDisplay handle.
//
//                  cap(OUT)            - the output pointer of the NV_UNDERSCAN_CAP structure
//
// RETURN STATUS:
//                  NVAPI_OK               - completed request
//                  NVAPI_ERROR            - miscellaneous error occurred
//                  NVAPI_INVALID_ARGUMENT - invalid input parameter.
//
///////////////////////////////////////////////////////////////////////////////
typedef struct
{
    NvU32 version;                           // the structure version

    float xRatioMin;                         // the supported min horizontal underscan ratio (e.g. 0.75 ~ 1.25 etc)
    float xRatioMax;                         // the supoorted max horizontal underscan ratio
    float yRatioMin;                         // the supported min vertical underscan ratio (e.g. 0.75 ~ 1.25)
    float yRatioMax;                         // the supported min veritcal underscan ratio

    NvU32 supportSourceUnderscan       : 1;  // indicate if source underscan is supported
    NvU32 supportScalerUnderscan       : 1;  // indicate if the scaler based underscan is supported
    NvU32 supportMonitorIdMatch        : 1;  // indicate if the underscan configuration can be set for the monitors with the same EDID
    NvU32 supportDisplayOutputIdMatch  : 1;  // indicate if the underscan configuration can be set for monitors connected to the same display output Id
    NvU32 supportXYIndependentCtrl     : 1;  // indicate if we can independently control on X- and Y- direction

                                             // Note 1:
                                             //------------------------+--------------------+-------------------------------------------------------------------------------------------
                                             //          GPU           |    DisplayOutput   |  Available Underscan option
                                             //------------------------+--------------------+-------------------------------------------------------------------------------------------
                                             // GeForce8/post-GeForce8 |   CRT/TV/DFP/HDMI  |  source and scaler underscan
                                             // GeForce7/pre-GeForce7  |      TV/DFP/HDMI   |  source under only, the API will automatically fall back if scaler underscan is specified
                                             // GeForce7/pre-GeForce7  |         CRT        |  no underscan option yet, the API will return failure
                                             //------------------------+--------------------+-------------------------------------------------------------------------------------------

                                             // Note 2:
                                             //-------------------+-----------------------------+--------------------------------------------------------------------
                                             // supportEdidMatch  | supportDisplayOutputIdMatch |                        behavior
                                             //-------------------+-----------------------------+--------------------------------------------------------------------
                                             //       0           |        0                    |  no Edid check, apply to all devices (not recommended)
                                             //       0           |        1                    |  no Edid check, apply to a specified device only (the old XP style)
                                             //       1           |        0                    |  apply to the specified Edid regardless of the device mask (new)
                                             //       1           |        1                    |  apply to the specified Edid on a specified device (new)
                                             //-------------------+-----------------------------+--------------------------------------------------------------------

} NV_UNDERSCAN_CAP;
#define NV_UNDERSCAN_CAP_VER  MAKE_NVAPI_VERSION(NV_UNDERSCAN_CAP,1)
NVAPI_INTERFACE NvAPI_QueryUnderscanCap(NvDisplayHandle hNvDisplay, NvU32 displayOutputId, NV_UNDERSCAN_CAP* cap);

///////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME:   NvAPI_EnumUnderscanConfig
//
// DESCRIPTION:     Enumerates the user customized underscan configurations on the given device
//
//  SUPPORTED OS: Windows Vista and higher
//
// PARAMETERS:      hNvDisplay(IN)      - NVIDIA Display selection. NVAPI_DEFAULT_HANDLE not allowed, it has to be a handle enumerated with NvAPI_EnumNVidiaDisplayHandle().
//
//                  displayOutputId(IN) - 1. the display Id on which the underscan config is to apply. It has to be a legal display Id (one bit set)
//                                        2. "0" is allowed meaning the default display used by the hNvDisplay handle.
//                                        3. "-1" is allowed to enum all saved underscan configurations regardless of the target display
//
//                  index(IN)           - 1. the regular the enum index.
//                                        2. "-1"(0xFFFFFFFF) is allowed to enum the current active underscan config on the display uniquely indicated by displayOutId
//
//                  config(OUT)         - the output pointer for the API to populate the underscan config info.
//
// RETURN STATUS:
//                  NVAPI_OK               - completed request
//                  NVAPI_ERROR            - miscellaneous error occurred
//                  NVAPI_INVALID_ARGUMENT - invalid input parameter.
//
///////////////////////////////////////////////////////////////////////////////
typedef struct
{
    NvU32 version;                           // the structure version

    float xRatio;                            // the horizontal underscan ratio
    float yRatio;                            // the vertical undercan ratio

                                             // NOTE:
                                             // the API may modify the client passed in config due to the h/w restrictions. For example the xRatio/yRatio may be
                                             // fine tuned so that they're aligned on a certain number because of the tiled memory restriction. Also "isSrcUnderscan"
                                             // may also be changed if the h/w can't do backend underscan.

    NvU32 nativeWidth;                       // the width of the native mode
    NvU32 nativeHeight;                      // the height of the native mode
    NvU32 nativeRR;                          // the refresh rate of the native mode
    NvU32 isInterlaced                 : 1;  // the native timing is interlaced or not

    NvU32 isScalerUnderscan            : 1;  // scaler based underscan or source based underscan
    NvU32 isOnThisMonitorOnly          : 1;  // whether this config is only applied to this specific monitor (EDID) only
    NvU32 isOnThisDisplayOutputIdOnly  : 1;  // whether this underscan config is only applied to this display output Id only

} NV_UNDERSCAN_CONFIG;
#define NV_UNDERSCAN_CONFIG_VER  MAKE_NVAPI_VERSION(NV_UNDERSCAN_CONFIG,1)
NVAPI_INTERFACE NvAPI_EnumUnderscanConfig(NvDisplayHandle hNvDisplay, NvU32 displayOutputId, NvU32 index, NV_UNDERSCAN_CONFIG* config);

///////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME:   NvAPI_DeleteUnderscanConfig
//
// DESCRIPTION:     Delete the specified underscan config entry
//
//  SUPPORTED OS: Windows Vista and higher
//
// PARAMETERS:      hNvDisplay(IN)      - NVIDIA Display selection. NVAPI_DEFAULT_HANDLE not allowed, it has to be a handle enumerated with NvAPI_EnumNVidiaDisplayHandle().
//
//                  displayOutputId(IN) - 1. the display Id on which the underscan config is to apply. It has to be a legal display Id (one bit set)
//                                        2. "0" is allowed meaning the default display used by the hNvDisplay handle.
//                                        3. "-1" is also allowed meaning "index" is the absolute index and not displayOutputId related
////
//                  index(IN)           - 1. the config index enumerated from NvAPI_EnumUnderScanConfig
//                                        2. "-1" is allowed for the current active underscan config being applied on the specified display output Id
//
// RETURN STATUS:
//                  NVAPI_OK                       - completed request
//                  NVAPI_ERROR                    - miscellaneous error occurred
//                  NVAPI_INVALID_ARGUMENT         - invalid input parameter.
//                  NVAPI_UNKNOWN_UNDERSCAN_CONFIG - the current active underscan config is from an unknown source and can't be deleted.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DeleteUnderscanConfig(NvDisplayHandle hNvDisplay, NvU32 displayOutputId, NvU32 index);

///////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME:   NvAPI_SetUnderscanConfig
//
// DESCRIPTION:     Add and set a custom underscan config on the give device
//
//  SUPPORTED OS: Windows Vista and higher
//
// PARAMETERS:      hNvDisplay(IN)      - NVIDIA Display selection. NVAPI_DEFAULT_HANDLE not allowed, it has to be a handle enumerated with NvAPI_EnumNVidiaDisplayHandle().
//
//                  displayOutputId(IN) - 1. the display Id on which the underscan config is to apply. It has to be a legal display Id (one bit set)
//                                        2. "0" is allowed meaning the default display used by the hNvDisplay handle.
//                                        NOTE: 0xFFFFFFFF(-1) is not allowed in NvAPI_SetUnderScanConfig()
//
//                  config(IN/OUT)      - the underscan config to set
//
//                  setDeferred         - "0": apply the setup immediately.
//                                        "1":  apply refresh at next OS modeset.

// RETURN STATUS:
//                  NVAPI_OK               - completed request
//                  NVAPI_ERROR            - miscellaneous error occurred
//                  NVAPI_INVALID_ARGUMENT - Invalid input parameter.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_SetUnderscanConfig(NvDisplayHandle hNvDisplay, NvU32 displayOutputId, NV_UNDERSCAN_CONFIG* config, NvU32 setDeferred);


///////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME:   NvAPI_GetDisplayFeatureConfig
//
// DESCRIPTION:     Return the current display feature configuration.
//
//  SUPPORTED OS: Windows Vista and higher
//
// PARAMETERS:      hNvDisplay(IN)      - NVIDIA Display selection. NVAPI_DEFAULT_HANDLE not allowed, it has to be a handle enumerated with NvAPI_EnumNVidiaDisplayHandle().
//
//                  feature (IN/OUT)    - a pointer to NV_DISPLAY_FEATURE structure to retrieve the current feature configuration.
// RETURN STATUS:
//                  NVAPI_OK               - completed request
//                  NVAPI_ERROR            - miscellaneous error occurred
//                  NVAPI_INVALID_ARGUMENT - Invalid input parameter.
//
///////////////////////////////////////////////////////////////////////////////
typedef struct _NV_DISPLAY_FEATURE
{
    NvU32      enableScalerUnderscan  : 1;          // enable/disable HDTV scaler based resizing
    NvU32      enableCEoverHDMI       : 1;          // enable/disable the Feature To Force CE timings over HDMI.
                                                    //If this feature is enabled then, CE timings will only be used for HDMI displays
    NvU32      reservedOEM            : 1;          // Reserved bit for OEM configuration
    NvU32      reserved               : 29;
    
} NV_DISPLAY_FEATURE;
NVAPI_INTERFACE NvAPI_GetDisplayFeatureConfig(NvDisplayHandle hNvDisplay, NV_DISPLAY_FEATURE* pFeature);

///////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME:   NvAPI_SetDisplayFeatureConfig
//
// DESCRIPTION:     Set display feature configuration.
//
//  SUPPORTED OS: Windows Vista and higher
//
// PARAMETERS:      hNvDisplay(IN)      - NVIDIA Display selection. NVAPI_DEFAULT_HANDLE not allowed, it has to be a handle enumerated with NvAPI_EnumNVidiaDisplayHandle().
//
//                  feature (IN/OUT)    - a copy NV_DISPLAY_FEATURE structure to setup the new feature configuration
// RETURN STATUS:
//                  NVAPI_OK               - completed request
//                  NVAPI_ERROR            - miscellaneous error occurred
//                  NVAPI_INVALID_ARGUMENT - Invalid input parameter.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_SetDisplayFeatureConfig(NvDisplayHandle hNvDisplay, NV_DISPLAY_FEATURE feature);

///////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME:   NvAPI_GetDisplayFeatureConfigDefaults
//
// DESCRIPTION:     Return the Default display feature configuration.
//
//  SUPPORTED OS: Windows Vista and higher
//
// PARAMETERS:      hNvDisplay(IN)      - NVIDIA Display selection. NVAPI_DEFAULT_HANDLE not allowed, it has to be a handle enumerated with NvAPI_EnumNVidiaDisplayHandle().
//
//                  feature (IN/OUT)    - a copy NV_DISPLAY_FEATURE structure to setup the Default feature configuration
// RETURN STATUS:
//                  NVAPI_OK               - completed request
//                  NVAPI_ERROR            - miscellaneous error occurred
//                  NVAPI_INVALID_ARGUMENT - Invalid input parameter.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GetDisplayFeatureConfigDefaults(NvDisplayHandle hNvDisplay, NV_DISPLAY_FEATURE* pFeature);


///////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME:   NvAPI_SetView
//
// DESCRIPTION:     This API lets caller to modify target display arrangement for selected source display handle in any of the nview modes.
//                  It also allows to modify or extend the source display in dualview mode.
//                  Note: Maps the selected source to the associated target Ids.
//                  Note: Display PATH with this API is limited to single GPU. DUALVIEW across GPUs cannot be enabled with this API. 
//                        Please use NvAPI_SYS_SetDisplayTopologies for enabling views across GPUs.
//  SUPPORTED OS: Windows Vista and higher
//
// PARAMETERS:      hNvDisplay(IN) - NVIDIA Display selection. NVAPI_DEFAULT_HANDLE not allowed, it has to be a handle enumerated with NvAPI_EnumNVidiaDisplayHandle().
//                  pTargetInfo(IN) - Pointer to array of NV_VIEW_TARGET_INFO, specifying device properties in this view.
//                                    The first device entry in the array is the physical primary.
//                                    The device entry with the lowest source id is the desktop primary.
//                  targetCount(IN) - Count of target devices specified in pTargetInfo.
//                  targetView(IN) - Target view selected from NV_TARGET_VIEW_MODE.
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_ERROR - miscellaneous error occurred
//                  NVAPI_INVALID_ARGUMENT: Invalid input parameter.
//
///////////////////////////////////////////////////////////////////////////////

typedef struct
{
    NvU32 version;     // (IN) structure version
    NvU32 count;       // (IN) target count
    struct
    {
        NvU32 deviceMask;   // (IN/OUT) device mask
        NvU32 sourceId;     // (IN/OUT) Values will be based on the number of heads exposed per GPU(0, 1?).
        NvU32 bPrimary:1;   // (OUT) Indicates if this is the GPU's primary view target. This is not the desktop GDI primary.
                            // NvAPI_SetView automatically selects the first target in NV_VIEW_TARGET_INFO index 0 as the GPU's primary view.
        NvU32 bInterlaced:1;// (IN/OUT) Indicates if the timing being used on this monitor is interlaced
        NvU32 bGDIPrimary:1;// (IN/OUT) Indicates if this is the desktop GDI primary.
        NvU32 bForceModeSet:1;// (IN) Used only on Win7 and higher during a call to NvAPI_SetView. Turns off optimization & forces OS to set supplied mode.
    } target[NVAPI_MAX_VIEW_TARGET];
} NV_VIEW_TARGET_INFO;

#define NV_VIEW_TARGET_INFO_VER  MAKE_NVAPI_VERSION(NV_VIEW_TARGET_INFO,2)

NVAPI_INTERFACE NvAPI_SetView(NvDisplayHandle hNvDisplay, NV_VIEW_TARGET_INFO *pTargetInfo, NV_TARGET_VIEW_MODE targetView);

///////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME:   NvAPI_GetView
//
// DESCRIPTION:     This API lets caller retrieve the target display arrangement for selected source display handle.
//                  Note: Display PATH with this API is limited to single GPU. DUALVIEW across GPUs will be returned as STANDARD VIEW. 
//                        Please use NvAPI_SYS_GetDisplayTopologies to query views across GPUs.
//  SUPPORTED OS: Windows Vista and higher
//
// PARAMETERS:      hNvDisplay(IN) - NVIDIA Display selection. NVAPI_DEFAULT_HANDLE not allowed, it has to be a handle enumerated with NvAPI_EnumNVidiaDisplayHandle().
//                  pTargetInfo(OUT) - User allocated storage to retrieve an array of  NV_VIEW_TARGET_INFO. Can be NULL to retrieve the targetCount.
//                  targetMaskCount(IN/OUT) - Count of target device mask specified in pTargetMask.
//                  targetView(OUT) - Target view selected from NV_TARGET_VIEW_MODE.
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_ERROR - miscellaneous error occurred
//                  NVAPI_INVALID_ARGUMENT: Invalid input parameter.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GetView(NvDisplayHandle hNvDisplay, NV_VIEW_TARGET_INFO *pTargets, NvU32 *pTargetMaskCount, NV_TARGET_VIEW_MODE *pTargetView);

///////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME:   NvAPI_SetViewEx
//
// DESCRIPTION:     This API lets caller to modify the display arrangement for selected source display handle in any of the nview modes.
//                  It also allows to modify or extend the source display in dualview mode.
//                  Note: Maps the selected source to the associated target Ids.
//                  Note: Display PATH with this API is limited to single GPU. DUALVIEW across GPUs cannot be enabled with this API. 
//                        Please use NvAPI_SYS_SetDisplayTopologies for enabling views across GPUs.
//  SUPPORTED OS: Windows Vista and higher
//
// PARAMETERS:      hNvDisplay(IN) - NVIDIA Display selection. NVAPI_DEFAULT_HANDLE not allowed, it has to be a handle enumerated with NvAPI_EnumNVidiaDisplayHandle().
//                  pPathInfo(IN)  - Pointer to array of NV_VIEW_PATH_INFO, specifying device properties in this view.
//                                    The first device entry in the array is the physical primary.
//                                    The device entry with the lowest source id is the desktop primary.
//                  pathCount(IN)  - Count of paths specified in pPathInfo.
//                  displayView(IN)- Display view selected from NV_TARGET_VIEW_MODE.
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_ERROR - miscellaneous error occurred
//                  NVAPI_INVALID_ARGUMENT: Invalid input parameter.
//
///////////////////////////////////////////////////////////////////////////////

#define NVAPI_MAX_DISPLAY_PATH  NVAPI_MAX_VIEW_TARGET

typedef struct
{
    NvU32                   deviceMask;     // (IN) device mask
    NvU32                   sourceId;       // (IN) Values will be based on the number of heads exposed per GPU(0, 1?)
    NvU32                   bPrimary:1;     // (IN/OUT) Indicates if this is the GPU's primary view target. This is not the desktop GDI primary.
                                            // NvAPI_SetViewEx automatically selects the first target in NV_DISPLAY_PATH_INFO index 0 as the GPU's primary view.
    NV_GPU_CONNECTOR_TYPE   connector;      // (IN) Specify connector type. For TV only.

    // source mode information
    NvU32                   width;          // (IN) width of the mode
    NvU32                   height;         // (IN) height of the mode
    NvU32                   depth;          // (IN) depth of the mode
    NV_FORMAT               colorFormat;    //      color format if needs to specify. Not used now.

    //rotation setting of the mode
    NV_ROTATE               rotation;       // (IN) rotation setting.

    // the scaling mode
    NV_SCALING              scaling;        // (IN) scaling setting

    // Timing info
    NvU32                   refreshRate;    // (IN) refresh rate of the mode
    NvU32                   interlaced:1;   // (IN) interlaced mode flag

    NV_DISPLAY_TV_FORMAT    tvFormat;       // (IN) to choose the last TV format set this value to NV_DISPLAY_TV_FORMAT_NONE

    // Windows desktop position
    NvU32                   posx;           // (IN/OUT) x offset of this display on the Windows desktop
    NvU32                   posy;           // (IN/OUT) y offset of this display on the Windows desktop
    NvU32                   bGDIPrimary:1;  // (IN/OUT) Indicates if this is the desktop GDI primary.

    NvU32                   bForceModeSet:1;// (IN) Used only on Win7 and higher during a call to NvAPI_SetViewEx. Turns off optimization & forces OS to set supplied mode.
    NvU32                   bFocusDisplay:1;// (IN) If set, this display path should have the focus after the GPU topology change
    NvU32                   gpuId:24;       // (IN) the physical display/target Gpu id which is the owner of the scan out (for SLI multimon, display from the slave Gpu)

} NV_DISPLAY_PATH;

typedef struct
{
    NvU32 version;     // (IN) structure version
    NvU32 count;       // (IN) path count
    NV_DISPLAY_PATH path[NVAPI_MAX_DISPLAY_PATH];
} NV_DISPLAY_PATH_INFO;

#define NV_DISPLAY_PATH_INFO_VER  NV_DISPLAY_PATH_INFO_VER3
#define NV_DISPLAY_PATH_INFO_VER3 MAKE_NVAPI_VERSION(NV_DISPLAY_PATH_INFO,3)
#define NV_DISPLAY_PATH_INFO_VER2 MAKE_NVAPI_VERSION(NV_DISPLAY_PATH_INFO,2)
#define NV_DISPLAY_PATH_INFO_VER1 MAKE_NVAPI_VERSION(NV_DISPLAY_PATH_INFO,1)

NVAPI_INTERFACE NvAPI_SetViewEx(NvDisplayHandle hNvDisplay, NV_DISPLAY_PATH_INFO *pPathInfo, NV_TARGET_VIEW_MODE displayView);

///////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME:   NvAPI_GetViewEx
//
// DESCRIPTION:     This API lets caller retrieve the target display arrangement for selected source display handle.
//                  Note: Display PATH with this API is limited to single GPU. DUALVIEW across GPUs will be returned as STANDARD VIEW. 
//                        Please use NvAPI_SYS_GetDisplayTopologies to query views across GPUs.
//
//  SUPPORTED OS: Windows Vista and higher
//
// PARAMETERS:      hNvDisplay(IN) - NVIDIA Display selection. NVAPI_DEFAULT_HANDLE not allowed, it has to be a handle enumerated with NvAPI_EnumNVidiaDisplayHandle().
//                  pPathInfo(IN/OUT) - count field should be set to NVAPI_MAX_DISPLAY_PATH. Can be NULL to retrieve just the pathCount.
//                  pPathCount(IN/OUT) - Number of elements in array pPathInfo->path.
//                  pTargetViewMode(OUT)- Display view selected from NV_TARGET_VIEW_MODE.
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_API_NOT_INTIALIZED - NVAPI not initialized
//                  NVAPI_ERROR - miscellaneous error occurred
//                  NVAPI_INVALID_ARGUMENT - Invalid input parameter.
//                  NVAPI_EXPECTED_DISPLAY_HANDLE - hNvDisplay is not a valid display handle.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GetViewEx(NvDisplayHandle hNvDisplay, NV_DISPLAY_PATH_INFO *pPathInfo, NvU32 *pPathCount, NV_TARGET_VIEW_MODE *pTargetViewMode);

///////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME:   NvAPI_GetSupportedViews
//
// DESCRIPTION:     This API lets caller enumerate all the supported NVIDIA display views - nview and dualview modes.
//
//  SUPPORTED OS: Windows XP and higher
//
// PARAMETERS:      hNvDisplay(IN) - NVIDIA Display selection. It can be NVAPI_DEFAULT_HANDLE or a handle enumerated from NvAPI_EnumNVidiaDisplayHandle().
//                  pTargetViews(OUT) - Array of supported views. Can be NULL to retrieve the pViewCount first.
//                  pViewCount(IN/OUT) - Count of supported views.
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_ERROR - miscellaneous error occurred
//                  NVAPI_INVALID_ARGUMENT: Invalid input parameter.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GetSupportedViews(NvDisplayHandle hNvDisplay, NV_TARGET_VIEW_MODE *pTargetViews, NvU32 *pViewCount);



///////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME:   NvAPI_GetHDCPLinkParameters
//
// DESCRIPTION:     This API lets caller enumerate fields within the NV_HDCP_INFO.
//
//                  The API provides support for the HDCP Upstream Protocol. The caller is responsible to fill the input fields
//                  within NV_HDCP_PACKET to ensure that the specified HDCP_COMMANDS can be fulfilled. Different HDCP_COMMANDS will
//                  enumerate different fields inside NV_HDCP_INFO. The following are the HDCP_COMMANDS supported
//
//                  (A) HDCP_CMD_QUERY_HEAD_CONFIG  : Enumerates ports attached to a head.
//                  (B) HDCP_CMD_READ_LINK_STATUS   : Reads the Status of the cipher returning a signed status and connection state.
//                  (C) HDCP_CMD_VALIDATE_LINK      : Returns the parameters necessary to validate the links for the specified attach-point.
//                  (D) HDCP_CMD_RENEGOTIATE        : Forced renegotiation of the link.
//
//  SUPPORTED OS: Windows XP and higher
//
//
// PARAMETERS:      hNvDisplay(IN) - NVIDIA Display selection. It should be a handle enumerated from NvAPI_EnumNVidiaDisplayHandle().
//                  pNvHdcpInfo(IN) - Pointer to NV_HDCP_INFO that contains the requested command and input parameters.
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_ERROR - miscellaneous error occurred
//                  NVAPI_INVALID_ARGUMENT - pNvHdcpPacket is NULL
//                  NVAPI_INCOMPATIBLE_STRUCT_VERSION - the version of NV_HDCP_INFO struct is not supported
//
///////////////////////////////////////////////////////////////////////////////

#define NVAPI_MAX_NUM_AP        16      // Max number of ports / attach points supported in HDCP Connection State
#define NVAPI_MAX_DEVICES       127     // Max number of Receiver & Repeater devices

typedef struct
{
    union
    {
        NvU32  data1;
        struct
        {
            NvU8    revMin;
            NvU8    revMaj;
            NvU8    verMin;
            NvU8    verMaj;
        } ver;
    };
    NvU16 data2;
    NvU16 data3;
    NvU8  data4[8];
} NV_UID;

typedef struct
{
#pragma pack(1)
    NvU64 uSessionID    : 37;   // [36:0] Random number
    NvU64 uDisplay      : 3;    // [39:37] O/S level display device
    NvU64 uReserved     : 24;   // [63:40] Reserved
#pragma pack()
} NV_HDCP_CN;

typedef struct
{
    union
    {
        NvU64   quadWord;
        struct
        {
            NvU64 hugePart      : 40; // lower 40 in little endian
            NvU64 unusedPart    : 24;
        } parts;
    };
} NV_U40;

typedef struct
{
    union
    {
        NvU64   quadWord;
        struct
        {
            NvU64 hugePart      : 56; // lower 56 in little endian
            NvU64 unusedPart    : 8;
        } parts;
    };
} NV_U56;


// Structure of data returned from monitor - as defined in HDCP Spec
typedef struct
{
#pragma pack(1)
    NvU32 ucDeviceCount         : 7;    // [6:0] Total Receivers (except rep)
    NvU32 bMaxDevicesExceeded   : 1;    // [7] Topology Error > 127 devices
    NvU32 bRepeaterDepth        : 3;    // [10:8] Repeater depth
    NvU32 bMaxCascadeExceeded   : 1;    // [11] Topology Error > 7 levels repeater
    NvU32 bHDMImode             : 1;    // [12] HDCP Receiver in HDMI mode
    NvU32 bReserved             : 19;   // [31:13] Reserved for future expansion
#pragma pack()
} NV_HDCP_BSTATUS;

// The Connection State
typedef struct
{
#pragma pack(1)
    NvU64 uAttachPoints         : 16;       // [15:0] Transmitting Attach Point
    NvU64 bNonHDCP              : 1;        // [16] Transmitting Attach Point
    NvU64 uHeadIndex            : 4;        // [20:17] Index of Head
    NvU64 uRFUPlanes            : 8;        // [28:21] *NOT* yet supported
    NvU64 uNumberOfActiveHeads  : 2;        // [30:29] Number of Heads - 1
    NvU64 uReserved2            : 9;        // [39:31] Reserved for future use
    NvU64 uAttachedPlanes       : 8;        // [47:40] Will be moved to 28:21 in future chips
    NvU64 bCloneMode            : 1;        // [48] Dual-Display Clone Mode *NOT* yet supported
    NvU64 bSpanMode             : 1;        // [49] Dual-Display Span Mode *NOT* yet supported
    NvU64 uPadding              : 14;       // [63:50] To fill up qword
#pragma pack()
} NV_HDCP_CS;

// The Status of the Attach-Point (HDCP-capable or other)
typedef struct
{
#pragma pack(1)
    NvU64 bEncrypting           : 1;        // [0] This Attach Point is Transmitting and has Output Encryption Enabled
    NvU64 bRepeater             : 1;        // [1] This Attach Point is Transmitting to a Repeater
    NvU64 bUserAccessible       : 1;        // [2] This Attach Point is Transmitting on a user-accessible external digital port
    NvU64 bExtUnprotected       : 1;        // [3] This Attach-point is Transmitting externally and is unprotected
    NvU64 uPortIndex            : 4;        // [7:4] Port/Attach-point index
    NvU64 uNumPorts             : 4;        // [11:8] Number of Connectable
    NvU64 bInternalPanel        : 1;        // [12] Compliant Internal / Non-User accessible Port Panel without HDCP Encryption
    NvU64 bWideScope            : 1;        // [13] Cs:16 is not enough to determine presence of non-compliant outputs+ (always ??
    NvU64 bHasCs                : 1;        // [14] Supports Connection State (always ??
    NvU64 bReadZ                : 1;        // [15] Supports ReadZ (always ??
    NvU64 uReserved             : 24;       // [39:16] Reserved for Future Expansion
    NvU64 bDualLinkEven         : 1;        // [40] The Even half of a Dual-Link (0x74)
    NvU64 bDualLinkOdd          : 1;        // [41] The Odd half of a Dual-Link (0x76)
    NvU64 bDualLinkCapable      : 1;        // [42] This Attach Point has Dual-Link capability
    NvU64 uPadding              : 21;       // [63:43] To fill up qword
    NvU32 DisplayId;                        // ID of the Display on this attach point
#pragma pack()
} NV_HDCP_STATUS;

// Flags used for indicating active member elements
typedef enum
{
    NV_HDCP_FLAGS_NULL          = 0x00000000, // Get AP Status
    NV_HDCP_FLAGS_APINDEX       = 0x00000001, // Index of Attach Point
    NV_HDCP_FLAGS_AN            = 0x00000010, // Downstream Session ID
    NV_HDCP_FLAGS_AKSV          = 0x00000020, // Downstream/Xmtr KSV
    NV_HDCP_FLAGS_BKSV          = 0x00000040, // Downstream/Rcvr KSV
    NV_HDCP_FLAGS_BSTATUS       = 0x00000080, // Link/Repeater Status
    NV_HDCP_FLAGS_CN            = 0x00000100, // Upstream Session ID
    NV_HDCP_FLAGS_CKSV          = 0x00000200, // Upstream ClientApp KSV
    NV_HDCP_FLAGS_DKSV          = 0x00000400, // Upstream/Xmtr KSV
    NV_HDCP_FLAGS_KP            = 0x00001000, // Signature
    NV_HDCP_FLAGS_S             = 0x00002000, // Status
    NV_HDCP_FLAGS_CS            = 0x00004000, // Connection State
    NV_HDCP_FLAGS_V             = 0x00010000, // V of the KSVList
    NV_HDCP_FLAGS_MP            = 0x00020000, // Encrypted initializer for KSV List
    NV_HDCP_FLAGS_BKSVLIST      = 0x00040000, // NumKsvs & BksvList[NumKsvs]
    NV_HDCP_FLAGS_DUAL_LINK     = 0x00100000, // Two sets of An, Aksv, Kp, Bksv, Dksv
    NV_HDCP_FLAGS_ALWAYS_AUTH   = 0x00200000, // Always authenticate
    NV_HDCP_FLAGS_ON_BY_DEMAND  = 0x00000000, // Authenticate on demand
    NV_HDCP_FLAGS_ABORT_UNTRUST = 0x00400000, // Abort, Kp didnt match
    NV_HDCP_FLAGS_ABORT_UNRELBL = 0x00800000, // Abort, Repeated Link Failures
    NV_HDCP_FLAGS_ABORT_KSV_LEN = 0x01000000, // Abort, incorrect KSV Length
    NV_HDCP_FLAGS_ABORT_KSV_SIG = 0x02000000, // Abort, bad KSV Signature
    NV_HDCP_FLAGS_ABORT_SRM_SIG = 0x04000000, // Abort, bad SRM Signature
    NV_HDCP_FLAGS_ABORT_SRM_REV = 0x08000000, // Abort due to SRM Revocation
    NV_HDCP_FLAGS_ABORT_NORDY   = 0x10000000, // Abort, Repeater Not Ready
    NV_HDCP_FLAGS_ABORT_KSVTOP  = 0x20000000, // Abort, KSV Topology Error
    NV_HDCP_FLAGS_ABORT_BADBKSV = 0x40000000, // Abort due to invalid Bksv
} NV_HDCP_FLAGS;

// HDCP Commands
typedef enum
{
    NV_HDCP_CMD_NULL               = 0x00, // Null command
    NV_HDCP_CMD_QUERY_HEAD_CONFIG  = 0x01, // Status of the head attach-points
    NV_HDCP_CMD_READ_LINK_STATUS   = 0x02, // Get the Status
    NV_HDCP_CMD_VALIDATE_LINK      = 0x03, // Gets M & V
    NV_HDCP_CMD_RENEGOTIATE        = 0x04, // Forced renegotiation of the link
    NV_HDCP_CMD_ABORTAUTHENTICATION= 0x05, // Abort authentication protocol
    NV_HDCP_CMD_SETLINKPOLICY      = 0x06, // Set the link policy
} NV_HDCP_COMMANDS;

// HDCP Return Status
typedef enum
{
    NV_HDCP_STATUS_SUCCESS                 = (0x00000000L), // Function completed successfully
    NV_HDCP_STATUS_UNSUCCESSFUL            = (0xC0000001L), // Function failed
    NV_HDCP_STATUS_PENDING                 = (0x00000103L), // Renegotiation is not complete, check status later
    NV_HDCP_STATUS_LINK_FAILED             = (0xC000013EL), // Renegotiation could not complete
    NV_HDCP_STATUS_INVALID_PARAMETER       = (0xC000000DL), // One or more of the calling parameters was invalid
    NV_HDCP_STATUS_INVALID_PARAMETER_MIX   = (0xC0000030L), // The combination of flFlags was invalid
    NV_HDCP_STATUS_NO_MEMORY               = (0xC0000017L), // Insufficient buffer space was allocated. Re-allocate using the size returned in the dwSize member
    NV_HDCP_STATUS_BAD_TOKEN_TYPE          = (0xC00000A8L), // The Session ID &/or KSV supplied were rejected
} NV_HDCP_RET_STATUS;

// HDCP Packet
typedef struct
{
#pragma pack(4)
    NV_UID              uidHDCP;                        // (IN)
    NvU32               packetSize;                     // (IN/OUT)
    NvU32               hDisplayContext;                // (IN/OUT)
    NV_HDCP_COMMANDS    cmdCommand;                     // (IN)
    NV_HDCP_FLAGS       flFlags;                        // (IN/OUT)
    NV_HDCP_RET_STATUS  hdcpPacketStatus;               // (OUT)

    NvU32               apIndex;                        // (IN) Attach point index
    NV_HDCP_CN          cN;                             // (IN) Client Session ID
    NV_U40              cKsv;                           // (IN)

    NV_HDCP_BSTATUS     bStatus[NVAPI_MAX_NUM_AP];      // (OUT)
    NV_HDCP_STATUS      hdcpStatus[NVAPI_MAX_NUM_AP];   // (OUT)
    NV_HDCP_CS          cS;                             // (OUT) Connection State

    NV_U56              kP[2];                          // (OUT) KPRIME value
    NV_U40              aN[2];                          // (OUT)
    NV_U40              aKsv[2];                        // (OUT)
    NV_U40              dKsv[2];                        // (OUT)
    NvU8                vP[20];                         // (OUT) VPRIME value
    NvU64               mP;                             // (OUT) MPRIME value
    NvU32               numBKSVs;                       // (OUT) Valid KSVs in the bKsvList. Maximum is 127 devices
    NV_U40              bKsvList[NVAPI_MAX_DEVICES];    // (OUT) Up to 127 receivers & repeaters
#pragma pack()
} NV_HDCP_PACKET;

typedef struct
{
    NvU32 version;
    NV_HDCP_PACKET nvHdcpPacket;
} NV_HDCP_INFO;

#define NV_HDCP_INFO_VER  MAKE_NVAPI_VERSION(NV_HDCP_INFO,1)

NVAPI_INTERFACE NvAPI_GetHDCPLinkParameters(NvDisplayHandle hNvDisplay, NV_HDCP_INFO *pNvHdcpInfo);

///////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME:   NvAPI_SetHybridMode
//
// DESCRIPTION: This API sets Hybrid mode. Supported on Vista and higher.
//              App calling this API is required to run in elevated mode.
//              This API can be called from a system service to derive the elevated context of the System service.
//              Non migratable apps running can prevent a successful transition. To query non migratable
//              apps use the NvAPI_QueryNonMigratableApps.
//
//  SUPPORTED OS: Windows Vista and higher
//
// PARAMETERS:      mode(IN)     - hybrid mode
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_ERROR - miscellaneous error occurred
//                  NVAPI_INVALID_ARGUMENT - Invalid input parameter
//
///////////////////////////////////////////////////////////////////////////////

typedef enum _NV_HYBRID_MODE
{
    NV_HYBRID_MODE_IGPU                 = 0x00,
    NV_HYBRID_MODE_DGPU                 = 0x01,
    NV_HYBRID_MODE_MULTI_ADAPTER        = 0x02,
    NV_HYBRID_MODE_INVALID              = 0x03,
    NV_HYBRID_MODE_DGPU_WARMUP          = 0x04,
    NV_HYBRID_MODE_DGPU_DEEP_IDLE       = 0x05,
    NV_HYBRID_MODE_INSTALL              = 0x06,
} NV_HYBRID_MODE;

NVAPI_INTERFACE NvAPI_SetHybridMode(NV_HYBRID_MODE mode);

///////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME:   NvAPI_GetHybridMode
//
// DESCRIPTION:     This API gets current Hybrid mode. Supported on Vista and higher.
//
//  SUPPORTED OS: Windows Vista and higher
//
// PARAMETERS:      pMode(OUT)     - hybrid mode
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_ERROR - miscellaneous error occurred
//                  NVAPI_INVALID_ARGUMENT - Invalid input parameter
//
///////////////////////////////////////////////////////////////////////////////

NVAPI_INTERFACE NvAPI_GetHybridMode(NV_HYBRID_MODE *pMode);

///////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME:   NvAPI_GetVideoState
//
// DESCRIPTION:     Return the video state component for the component ID passed in. It is the responsibility
//                  of the caller to fill secret application key that enables authentication of the caller.
//
//  SUPPORTED OS: Windows XP and higher
//
// PARAMETERS:      hNvDisplay(IN)          - NVIDIA Display selection. It can be NVAPI_DEFAULT_HANDLE or a handle enumerated
//                                            from NvAPI_EnumNVidiaDisplayHandle().
//                  pGetVidStateComp(IN)    - NVAPI_GET_VIDEO_STATE_COMPONENT packet containing
//                                            a valid component ID and a valid application secret key.
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_ERROR - miscellaneous error occurred
//                  NVAPI_INVALID_ARGUMENT - Invalid input parameter
//
///////////////////////////////////////////////////////////////////////////////

#ifndef NV_VIDEO_COMPONENTS_DEFINE
#define NV_VIDEO_COMPONENTS_DEFINE

///////////////////////////////////////////////////////////////////////////////
// Structs and enums related to Video state
///////////////////////////////////////////////////////////////////////////////

// Components related to video state
typedef enum _NVAPI_VIDEO_STATE_COMPONENT_ID
{
    NVAPI_VIDEO_STATE_COMPONENT_ID_NONE     = -1,   // Placeholder for invalid component ID
    NVAPI_VIDEO_STATE_COMPONENT_BRIGHTNESS      ,   // Permits control of video's brightness value
    NVAPI_VIDEO_STATE_COMPONENT_CONTRAST        ,   // Allows control of video's contrast value
    NVAPI_VIDEO_STATE_COMPONENT_HUE             ,   // To control the hue value
    NVAPI_VIDEO_STATE_COMPONENT_SATURATION      ,   // Allows control of video's saturation value
    NVAPI_VIDEO_STATE_COMPONENT_COLORTEMP       ,   // Allows control of the color temperature value
    NVAPI_VIDEO_STATE_COMPONENT_Y_GAMMA         ,   // To set the Y-gamma values
    NVAPI_VIDEO_STATE_COMPONENT_RGB_GAMMA_R     ,   // To set the R value of RGB gamma
    NVAPI_VIDEO_STATE_COMPONENT_RGB_GAMMA_G     ,   // To set the G value of RGB gamma
    NVAPI_VIDEO_STATE_COMPONENT_RGB_GAMMA_B     ,   // To set the B value of RGB gamma
    NVAPI_VIDEO_STATE_COMPONENT_COLOR_SPACE     ,   // Permits choice of various color spaces using VIDEO_COMP_ALGO_COLOR_SPACE_xxx
    NVAPI_VIDEO_STATE_COMPONENT_COLOR_RANGE     ,   // Allows setting between a limited/full color range using VIDEO_COMP_ALGO_COLOR_RANGE_xxx
    NVAPI_VIDEO_STATE_COMPONENT_PLAYBACK_PROFILE,   // Permits using special postprocessing for Adobe Flash 9 Content
    NVAPI_VIDEO_STATE_COMPONENT_DEINTERLACE     ,   // To set various types of deinterlacing algorithms
    NVAPI_VIDEO_STATE_COMPONENT_SCALING         ,   // Allows setting video scaling algorithms
    NVAPI_VIDEO_STATE_COMPONENT_CADENCE         ,   // Allows control of the cadence algorithms
    NVAPI_VIDEO_STATE_COMPONENT_NOISE_REDUCE    ,   // Allows setting post-processing noise reduction values
    NVAPI_VIDEO_STATE_COMPONENT_EDGE_ENHANCE    ,   // Permits post-processing edge enhancement value adjustment
    NVAPI_VIDEO_STATE_COMPONENT_OVERDRIVE       ,   // To control the overdrive feature
    NVAPI_VIDEO_STATE_COMPONENT_SPLITSCREEN     ,   // To permit setting a splitscreen using one of VIDEO_COMP_ALGO_SPLITSCREEN_xxx
    NVAPI_VIDEO_STATE_COMPONENT_DEBLOCKING      ,   // Allows out-of-loop deblocking
    NVAPI_VIDEO_STATE_COMPONENT_DYNAMIC_CONTRAST,   // Permits control of video's dynamic contrast value
    NVAPI_VIDEO_STATE_COMPONENT_GREEN_STRETCH   ,   // Permits control of green stretch
    NVAPI_VIDEO_STATE_COMPONENT_BLUE_STRETCH    ,   // Allows control of blue enhancement
    NVAPI_VIDEO_STATE_COMPONENT_SKIN_TONE_CORRECTION, // Allows skin-tone correction for video
    NVAPI_VIDEO_STATE_COMPONENT_GAMUT_REMAPPING ,   // Applies gamut remapping on video
    NVAPI_VIDEO_STATE_COMPONENT_ID_LAST         ,   // All valid components defined before this one
} NVAPI_VIDEO_STATE_COMPONENT_ID;

#define NVAPI_VIDEO_STATE_COMPONENT_DYNAMIC_CONSTRAST  NVAPI_VIDEO_STATE_COMPONENT_DYNAMIC_CONTRAST  // dynamic contrast value. Kept this for backward compatibility

// Algorithms controlling various video components

#define VIDEO_COMP_ALGO_CUSTOM_BASE 64

typedef enum _NVAPI_VIDEO_COMPONENT_ALGORITHM
{
    VIDEO_COMP_ALGO_COLOR_SPACE_601                  = 0,  // Use the ITU-R BT.601 standard in color-space conversion for xxx_COLOR_SPACE component
    VIDEO_COMP_ALGO_COLOR_SPACE_709                  = 1,  // Use the ITU-R BT.709 standard in color-space conversion for xxx_COLOR_SPACE component
    VIDEO_COMP_ALGO_COLOR_RANGE_STD                  = 0,  // Full range of (0-255) for xxx_COLOR_RANGE component, equivalent to Microsoft's DXVADDI_NOMINALRANGE::DXVADDI_NominalRange_0_255
    VIDEO_COMP_ALGO_COLOR_RANGE_EXT                  = 1,  // Limited range of (16-235) for xxx_COLOR_RANGE component, equivalent to Microsoft's DXVADDI_NOMINALRANGE::DXVADDI_NominalRange_16_235
    VIDEO_COMP_ALGO_PLAYBACK_PROFILE_NONE            = 0,  // Use no playback profile
    VIDEO_COMP_ALGO_PLAYBACK_PROFILE_ADOBE_FLASH_9   = 1,  // Use the internet video enhancement postprocessing for Adobe Flash 9 
    VIDEO_COMP_ALGO_DEINTERLACE_BOB                  = 0,  // Perform Bob deinterlacing
    VIDEO_COMP_ALGO_DEINTERLACE_WEAVE                = 1,  // Use weave deinterlacing
    VIDEO_COMP_ALGO_DEINTERLACE_SIMPLE_ADAPTIVE      = 2,  // Perform a simple motion adaptive deinterlacing
    VIDEO_COMP_ALGO_DEINTERLACE_GRADIENT_SIMPLE      = 3,  // Use a simple gradient deinterlacing
    VIDEO_COMP_ALGO_DEINTERLACE_GRADIENT_FULL        = 4,  // Use advanced gradient deinterlacing
    VIDEO_COMP_ALGO_DEINTERLACE_ADAPTIVE_FOUR_FIELD  = 5,  // Perform four field motion adaptive deinterlacing
    VIDEO_COMP_ALGO_DEINTERLACE_DIR_SPATIAL          = 6,  // User directional spatial deinterlacing
    VIDEO_COMP_ALGO_DEINTERLACE_ADVANCED             = 7,  // Perform proprietary advanced deinterlacing
    VIDEO_COMP_ALGO_DEINTERLACE_GPU_CAPABLE          = 8,  // Best available but GPU dependent. (video driver decides dynamically)
    VIDEO_COMP_ALGO_DEINTERLACE_CUSTOM_00            = VIDEO_COMP_ALGO_CUSTOM_BASE+0,    // Use custom Deinterlacing algorithm
    VIDEO_COMP_ALGO_DEINTERLACE_CUSTOM_01            = VIDEO_COMP_ALGO_CUSTOM_BASE+1,    // Use custom Deinterlacing algorithm
    VIDEO_COMP_ALGO_SCALING_ALG_SIMPLE               = 0,  // Do scaling using a simple algorithm
    VIDEO_COMP_ALGO_SCALING_ALG_4x4FILTER            = 1,  // Perform scaling using a 4x4 filter
    VIDEO_COMP_ALGO_SCALING_ALG_8x8FILTER            = 2,  // Perform scaling using a 8x8 filter
    VIDEO_COMP_ALGO_SCALING_ALG_CUSTOM_00            = VIDEO_COMP_ALGO_CUSTOM_BASE+0,    // Use custom scaling component
    VIDEO_COMP_ALGO_SCALING_ALG_CUSTOM_01            = VIDEO_COMP_ALGO_CUSTOM_BASE+1,    // Use custom scaling component
    VIDEO_COMP_ALGO_CADENCE_NONE                     = 0,  // Turn cadence OFF
    VIDEO_COMP_ALGO_CADENCE_SIMPLE                   = 1,  // Use simple cadence detection
    VIDEO_COMP_ALGO_CADENCE_VOF                      = 2,  // Use video on film cadence detection
    VIDEO_COMP_ALGO_CADENCE_COMPUTE                  = 3,  // Use compute cadence detection
    VIDEO_COMP_ALGO_CADENCE_GPU_CAPABLE              = 4,  // Best available but GPU dependent. (video driver decides dynamically)
    VIDEO_COMP_ALGO_NOISE_REDUCE_PUREVIDEO           = 0,  // Use PureVideo noise reduction
    VIDEO_COMP_ALGO_NOISE_REDUCE_CUSTOM_00           = VIDEO_COMP_ALGO_CUSTOM_BASE+0,  // Use custom noise reduction
    VIDEO_COMP_ALGO_NOISE_REDUCE_CUSTOM_01           = VIDEO_COMP_ALGO_CUSTOM_BASE+1,  // Use custom noise reduction
    VIDEO_COMP_ALGO_EDGE_ENHANCE_PUREVIDEO           = 0,  // Use PureVideo Sharpening Filter
    VIDEO_COMP_ALGO_EDGE_ENHANCE_CUSTOM_00           = VIDEO_COMP_ALGO_CUSTOM_BASE+0,  // Use custom Sharpening Filter
    VIDEO_COMP_ALGO_EDGE_ENHANCE_CUSTOM_01           = VIDEO_COMP_ALGO_CUSTOM_BASE+1,  // Use custom Sharpening Filter
    VIDEO_COMP_ALGO_OVERDRIVE_SIMPLE                 = 0,  // Use simple overdrive algorithm
    VIDEO_COMP_ALGO_SPLITSCREEN_TYPE_NORMAL          = 0,  // Set the splitscreen in normal mode
    VIDEO_COMP_ALGO_SPLITSCREEN_TYPE_REPEATED        = 1,  // Set the splitscreen to be repeated
    VIDEO_COMP_ALGO_SPLITSCREEN_TYPE_ON_MIRROR       = 2,  // Set the splitscreen as a mirror
    VIDEO_COMP_ALGO_DYNAMIC_CONTRAST_PRESET_NONE     = 0,  // Use the value setting for dynamic contrast instead of a preset
    VIDEO_COMP_ALGO_DYNAMIC_CONTRAST_PRESET_LOW      = 1,  // Turn the dynamic contrast to a low setting
    VIDEO_COMP_ALGO_DYNAMIC_CONTRAST_PRESET_MEDIUM   = 2,  // Turn the dynamic contrast to a medium setting
    VIDEO_COMP_ALGO_DYNAMIC_CONTRAST_PRESET_HIGH     = 3,  // Turn the dynamic contrast to a high setting
    VIDEO_COMP_ALGO_GREEN_STRETCH_PRESET_NONE        = 0,  // Use the value setting for green stretch instead of a preset
    VIDEO_COMP_ALGO_GREEN_STRETCH_PRESET_LOW         = 1,  // Set the green strech to a low setting
    VIDEO_COMP_ALGO_GREEN_STRETCH_PRESET_MEDIUM      = 2,  // Set the green strech to a medium setting
    VIDEO_COMP_ALGO_GREEN_STRETCH_PRESET_HIGH        = 3,  // Set the green strech to a high setting
    VIDEO_COMP_ALGO_BLUE_STRETCH_PRESET_NONE         = 0,  // Use the value setting for blue stretch instead of a preset
    VIDEO_COMP_ALGO_BLUE_STRETCH_PRESET_LOW          = 1,  // Set the blue strech to a low setting
    VIDEO_COMP_ALGO_BLUE_STRETCH_PRESET_MEDIUM       = 2,  // Set the blue strech to a medium setting
    VIDEO_COMP_ALGO_BLUE_STRETCH_PRESET_HIGH         = 3,  // Set the blue strech to a high setting
    VIDEO_COMP_ALGO_SKIN_TONE_CORRECTION_PRESET_NONE = 0,  // Use the value setting for skin tone correction instead of a preset
    VIDEO_COMP_ALGO_SKIN_TONE_CORRECTION_PRESET_SQUEEZE = 1,  // Turn the skin tone correction to a low setting
    VIDEO_COMP_ALGO_SKIN_TONE_CORRECTION_PRESET_RED     = 2,  // Turn the skin tone correction to a medium setting
    VIDEO_COMP_ALGO_SKIN_TONE_CORRECTION_PRESET_YELLOW  = 3,  // Turn the skin tone correction to a high setting
    VIDEO_COMP_ALGO_GAMUT_REMAPPING_CUSTOM_00        = VIDEO_COMP_ALGO_CUSTOM_BASE+0, // Use custom 3x3 gamut remapping matrix
    VIDEO_COMP_ALGO_GAMUT_REMAPPING_CUSTOM_01        = VIDEO_COMP_ALGO_CUSTOM_BASE+1, // Use custom 3x3 gamut remapping matrix
} NVAPI_VIDEO_COMPONENT_ALGORITHM;

// bitmasks for video components' enable at various resolutions
typedef enum _NVAPI_VIDEO_COMPONENT_ENABLE
{
    VIDEO_COMP_ENA_480i     = 0x00000001, // component is enabled at 480i video resolution
    VIDEO_COMP_ENA_480p     = 0x00000002, // component is enabled at 480p video resolution
    VIDEO_COMP_ENA_576i     = 0x00000004, // component is enabled at 576i video resolution
    VIDEO_COMP_ENA_576p     = 0x00000008, // component is enabled at 576p video resolution
    VIDEO_COMP_ENA_720p     = 0x00000010, // component is enabled at 720p video resolution
    VIDEO_COMP_ENA_1080i    = 0x00000020, // component is enabled at 1080i video resolution
    VIDEO_COMP_ENA_1080p    = 0x00000040, // component is enabled at 1080p video resolution
} NVAPI_VIDEO_COMPONENT_ENABLE;

// Packet that facilitates retrieving information about a video component
typedef struct _NVAPI_GET_VIDEO_STATE_COMPONENT
{
    NvU32   version;                    // (IN)  NVAPI version that matches NVAPI_GET_VIDEO_STATE_COMPONENT_VER
    NvU32   componentID;                // (IN)  identify the individual component, one of NVAPI_VIDEO_STATE_COMPONENT_xxx enums
    NvU32   bIsSupported        : 1;    // (OUT) set if this component feature is supported
    NvU32   bIsOverridenByUser  : 1;    // (OUT) set if component is overriden by user's choice
    NvU32   reserved1           : 30;   // (OUT) reserved for future expansion
    NvU32   isEnabled;                  // (OUT) set if component is enabled, one or more of NVAPI_VIDEO_COMPONENT_ENABLE bitmasks
    NvU32   minValue;                   // (OUT) min valid value
    NvU32   maxValue;                   // (OUT) max valid value
    NvU32   totalSteps;                 // (OUT) number of steps between min and max
    NvU32   defaultValue;               // (OUT) pre-defined NVIDIA default
    NvU32   unityValue;                 // (OUT) unity is the disable value for a component
    NvU32   currentValueActive;         // (OUT) value in use
    NvU64   defaultAlgo;                // (OUT) default algo, one or more of NVAPI_VIDEO_COMPONENT_ALGORITHM enums
    NvU64   currentAlgoActive;          // (OUT) algo in use, one or more of NVAPI_VIDEO_COMPONENT_ALGORITHM enums
    union
    {
        NvU64 qwReserved[9];
        struct
        {
            NvU32   dwAppKey;           // (IN) Secret key to authenticate the caller, ONLY used in NvAPI_SetVideoState
            NvU32   bTopPriority   : 1; // (OUT)indicates that these settings have precedence over D3D NvAPI settings, ONLY valid in NvAPI_GetVideoState
            NvU32   bHasCustomAlgo : 1; // (OUT)indicates whether Out-of-process app has custom algorithm data
            NvU32   bReserved      : 30;// (OUT) reserved for expansion
            struct
            {
                NvU64   pData;          // (IN) Buffer to hold the retreived custom algo data
                NvU32   dwSize;         // (IN) Size in Bytes of the above buffer, must be <= NVAPI_VIDEO_STATE_MAX_CUSTOM_ALGO_SIZE.
            } customAlgo;
        } appInfo;
    };
} NVAPI_GET_VIDEO_STATE_COMPONENT;

#define NVAPI_GET_VIDEO_STATE_COMPONENT_VER  MAKE_NVAPI_VERSION(NVAPI_GET_VIDEO_STATE_COMPONENT,1)

// Return status after attempting to set a video component
typedef enum _NVAPI_VIDEO_COMP_RETURN_STATUS
{
    VIDEO_COMP_STATUS_SUCCESS                   = 0x00000000, // Video component is set successfully
    VIDEO_COMP_STATUS_UNSUCCESSFUL              = 0x00000001, // Failed to set video component
    VIDEO_COMP_STATUS_COMPONENT_NOT_SUPPORTED   = 0x00000002, // Video component is not supported
    VIDEO_COMP_STATUS_VALUE_OUT_OF_RANGE        = 0x00000004, // Video component's value is invalid and does not fall into range
    VIDEO_COMP_STATUS_ALGO_NOT_RECOGNIZED       = 0x00000008, // Video component's algorithm is invalid
    VIDEO_COMP_STATUS_OVERRIDDEN_BY_USER        = 0x00000010, // Request not completed because of user-mandated override
    VIDEO_COMP_STATUS_Y_GAMMA_ENABLED           = 0x00000020, // Cannot set RGB-gamma because Y-Gamma is already enabled
    VIDEO_COMP_STATUS_RGB_GAMMA_ENABLED         = 0x00000040, // Cannot set Y-gamma because RGB-Gamma is already enabled
} NVAPI_VIDEO_COMP_RETURN_STATUS;

// Packet containing information to allow setting the video component

#define NVAPI_VIDEO_STATE_MAX_CUSTOM_ALGO_SIZE 128

typedef struct _NVAPI_SET_VIDEO_STATE_COMPONENT
{
    NvU32   version;                        // (IN) NVAPI version that matches NVAPI_SET_VIDEO_STATE_COMPONENT_VER
    NvU32   componentID;                    // (IN) identify the individual component, one of NVAPI_VIDEO_STATE_COMPONENT_xxx enums
    NvU32   enable;                         // (IN) flag to enable setting of component, one or more of NVAPI_VIDEO_COMPONENT_ENABLE bitmasks
    NvU32   setToValue;                     // (IN) value to use
    NvU64   setToAlgo;                      // (IN) algorithm to use
    NvU32   retStatus;                      // (OUT) result of video-component-set operation; a combination of VIDEO_COMP_STATUS_xxx bitmasks
    NvU32   reserved;
    union
    {
        NvU64 qwReserved[4];
        struct
        {
            NvU32   dwAppKey;               // (IN) Secret key to authenticate the caller, ONLY used in NvAPI_SetVideoState
            NvU32   bTopPriority       : 1; // (IN) Force these settings to have priority over D3D NvAPI settings, ONLY valid in NvAPI_GetVideoState
            NvU32   bHasCustomAlgo     : 1; // (IN) Out-of-process app has custom algorithm data
            NvU32   bReserved          : 30;// (IN) reserved for expansion
            struct
            {
                NvU64   pData;              // (IN) (Used only when bHasCustomAlgo == 1) Pointer to the custom algo data.
                NvU32   dwSize;             // (IN) (Used only when bHasCustomAlgo == 1) Size in Bytes of the custom algo data, must be <= NVAPI_VIDEO_STATE_MAX_CUSTOM_ALGO_SIZE.
            } customAlgo;
        } appInfo;
    };
} NVAPI_SET_VIDEO_STATE_COMPONENT;

#define NVAPI_SET_VIDEO_STATE_COMPONENT_VER  MAKE_NVAPI_VERSION(NVAPI_SET_VIDEO_STATE_COMPONENT,1)

#endif // ifndef NV_VIDEO_COMPONENTS_DEFINE

NVAPI_INTERFACE NvAPI_GetVideoState(NvDisplayHandle hNvDisplay, NVAPI_GET_VIDEO_STATE_COMPONENT *pGetVidStateComp);

///////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME:   NvAPI_SetVideoState
//
// DESCRIPTION:     API that allows callers to set a particular video state component. It is the responsibility
//                  of the caller to fill the secret application key that enables authentication of the caller.
//
//  SUPPORTED OS: Windows XP and higher
//
// PARAMETERS:      hNvDisplay(IN)          - NVIDIA Display selection. It can be NVAPI_DEFAULT_HANDLE or a handle enumerated
//                                            from NvAPI_EnumNVidiaDisplayHandle().
//                  pSetVidStateComp(IN)    - NVAPI_SET_VIDEO_STATE_COMPONENT packet containing
//                                            a valid component ID and a valid application secret key.
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_ERROR - miscellaneous error occurred
//                  NVAPI_INVALID_ARGUMENT - Invalid input parameter
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_SetVideoState(NvDisplayHandle hNvDisplay, NVAPI_SET_VIDEO_STATE_COMPONENT *pSetVidStateComp);

//  SUPPORTED OS: Windows XP and higher

///////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME:   NvAPI_SetFrameRateNotify
//
// DESCRIPTION:     This API signals the driver to turn on/off the notification to
//                  the service app of the video frame rate.
//
//
// PARAMETERS:      hNvDisplay(IN)  - NVIDIA Display selection. It can be NVAPI_DEFAULT_HANDLE or a handle enumerated
//                                    from NvAPI_EnumNVidiaDisplayHandle().
//                  pFRNotifyInfo(IN) - Pointer to the supplied NVAPI_FRAME_RATE_NOTIFY_INFO struct.
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_ERROR - miscellaneous error occurred
//                  NVAPI_INVALID_ARGUMENT - Invalid input parameter
//
///////////////////////////////////////////////////////////////////////////////
typedef struct _NVAPI_FRAME_RATE_NOTIFY_INFO
{
    NvU32   version;                // (IN) version for this NVAPI struct
    NvU64   hWnd;                   // (IN) the window handle of the service application handling the RR adjust
    NvU32   bEnable         : 1;    // (IN) To turn feature ON/OFF
    NvU32   bReserved       : 31;   // (IN/OUT) reserved for later use
} NVAPI_FRAME_RATE_NOTIFY_INFO;

#define NVAPI_FRAME_RATE_NOTIFY_INFO_VER  MAKE_NVAPI_VERSION(NVAPI_FRAME_RATE_NOTIFY_INFO,1)

NVAPI_INTERFACE NvAPI_SetFrameRateNotify(NvDisplayHandle hNvDisplay, NVAPI_FRAME_RATE_NOTIFY_INFO* pFRNotifyInfo);

//  SUPPORTED OS: Windows XP and higher

///////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME:   NvAPI_SetPVExtName
//
// DESCRIPTION:     This API specifies the name of the PureVideo extension dll
//                  to be used when custom algorithms are enabled via 
//                  NvAPI_SetVideoState calls
//
// PARAMETERS:      hNvDisplay(IN)  - NVIDIA Display selection. It can be NVAPI_DEFAULT_HANDLE or a handle enumerated
//                                    from NvAPI_EnumNVidiaDisplayHandle().
//                  szDllName - name of the dll to be used for custom algorithms
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_ERROR - miscellaneous error occurred
//                  NVAPI_INVALID_ARGUMENT - Invalid input parameter
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_SetPVExtName(NvDisplayHandle hNvDisplay, const char *szDllName);

//  SUPPORTED OS: Windows XP and higher

///////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME:   NvAPI_GetPVExtName
//
// DESCRIPTION:     This API retrieves the name of the PureVideo extension dll
//                  to be used when custom algorithms are enabled via 
//                  NvAPI_SetVideoState calls
//
// PARAMETERS:      hNvDisplay(IN)  - NVIDIA Display selection. It can be NVAPI_DEFAULT_HANDLE or a handle enumerated
//                                    from NvAPI_EnumNVidiaDisplayHandle().
//                  szDllName - name of the dll that's currently registered with the driver
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_ERROR - miscellaneous error occurred
//                  NVAPI_INVALID_ARGUMENT - Invalid input parameter
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GetPVExtName(NvDisplayHandle hNvDisplay, NvAPI_String szDllName);


////////////////////////////////////////////////////////////////////////////////////////
//
// MOSAIC allows a multi display target output scanout on a single source. 
//
// SAMPLE of MOSAIC 1x4 topo with 8 pixel horizontal overlap
//
//+-------------------------++-------------------------++-------------------------++-------------------------+
//|                         ||                         ||                         ||                         |
//|                         ||                         ||                         ||                         |
//|                         ||                         ||                         ||                         |
//|        DVI1             ||           DVI2          ||         DVI3            ||          DVI4           |
//|                         ||                         ||                         ||                         |
//|                         ||                         ||                         ||                         |
//|                         ||                         ||                         ||                         |
//|                         ||                         ||                         ||                         |
//+-------------------------++-------------------------++-------------------------++-------------------------+

#define NVAPI_MAX_MOSAIC_DISPLAY_ROWS       8
#define NVAPI_MAX_MOSAIC_DISPLAY_COLUMNS    8

//
// These bits are used to describe the validity of a topo.
//
#define NV_MOSAIC_TOPO_VALIDITY_VALID               0x00000000  // The topo is valid
#define NV_MOSAIC_TOPO_VALIDITY_MISSING_GPU         0x00000001  // Not enough SLI GPUs were found to fill the entire
                                                                // topo.  hPhysicalGPU will be 0 for these.
#define NV_MOSAIC_TOPO_VALIDITY_MISSING_DISPLAY     0x00000002  // Not enough displays were found to fill the entire
                                                                // topo.  displayOutputId will be 0 for these.
#define NV_MOSAIC_TOPO_VALIDITY_MIXED_DISPLAY_TYPES 0x00000004  // Topo is only possible with displays of the same
                                                                // NV_GPU_OUTPUT_TYPE.  Check displayOutputIds to make
                                                                // sure they are all CRT, or all DFP.


//
// This structure defines the details of a topo.
//
typedef struct 
{
    NvU32                version;              // version of this structure
    NvLogicalGpuHandle   hLogicalGPU;          // logical gpu this topo is for
    NvU32                validityMask;         // 0 means topo is valid with current hardware.
                                               // If not 0, inspect bits against NV_MOSAIC_TOPO_VALIDITY_*.
    NvU32                rowCount;             // number of displays in a row
    NvU32                colCount;             // number of displays in a column

    struct 
    {
        NvPhysicalGpuHandle hPhysicalGPU;      // physical gpu to be used in the topo (0 if GPU missing)
        NvU32               displayOutputId;   // connected display target (0 if no display connected)
        NvS32               overlapX;          // pixels of overlap on left of target: (+overlap, -gap)
        NvS32               overlapY;          // pixels of overlap on top of target: (+overlap, -gap)

    } gpuLayout[NVAPI_MAX_MOSAIC_DISPLAY_ROWS][NVAPI_MAX_MOSAIC_DISPLAY_COLUMNS];

} NV_MOSAIC_TOPO_DETAILS;

#define NVAPI_MOSAIC_TOPO_DETAILS_VER         MAKE_NVAPI_VERSION(NV_MOSAIC_TOPO_DETAILS,1)


//
// These values refer to the different types of Mosaic topos that are possible.  When
// getting the supported Mosaic topos, you can specify one of these types to narrow down
// the returned list to only those that match the given type.
//
typedef enum
{
    NV_MOSAIC_TOPO_TYPE_ALL,                          // All mosaic topos
    NV_MOSAIC_TOPO_TYPE_BASIC,                        // Basic Mosaic topos
    NV_MOSAIC_TOPO_TYPE_PASSIVE_STEREO,               // Passive Stereo topos
    NV_MOSAIC_TOPO_TYPE_SCALED_CLONE,                 // Not supported at this time
    NV_MOSAIC_TOPO_TYPE_PASSIVE_STEREO_SCALED_CLONE,  // Not supported at this time
    NV_MOSAIC_TOPO_TYPE_MAX,                          // Always leave this at end of enum
} NV_MOSAIC_TOPO_TYPE;


//
// The complete list of supported Mosaic topos.
//
// NOTE: common\inc\nvEscDef.h shadows a couple PASSIVE_STEREO enums.  If this
//       enum list changes and effects the value of NV_MOSAIC_TOPO_BEGIN_PASSIVE_STEREO
//       please update the corresponding value in nvEscDef.h
//
typedef enum
{
    NV_MOSAIC_TOPO_NONE,

    // 'BASIC' topos start here
    //
    // The result of using one of these Mosaic topos is that multiple monitors
    // will combine to create a single desktop.
    //
    NV_MOSAIC_TOPO_BEGIN_BASIC,
    NV_MOSAIC_TOPO_1x2_BASIC = NV_MOSAIC_TOPO_BEGIN_BASIC,
    NV_MOSAIC_TOPO_2x1_BASIC,
    NV_MOSAIC_TOPO_1x3_BASIC,
    NV_MOSAIC_TOPO_3x1_BASIC,
    NV_MOSAIC_TOPO_1x4_BASIC,
    NV_MOSAIC_TOPO_4x1_BASIC,
    NV_MOSAIC_TOPO_2x2_BASIC,
    NV_MOSAIC_TOPO_2x3_BASIC,
    NV_MOSAIC_TOPO_2x4_BASIC,
    NV_MOSAIC_TOPO_END_BASIC = NV_MOSAIC_TOPO_2x4_BASIC,

    // 'PASSIVE_STEREO' topos start here
    //
    // The result of using one of these Mosaic topos is that multiple monitors
    // will combine to create a single PASSIVE STEREO desktop.  What this means is
    // that there will be two topos that combine to create the overall desktop.
    // One topo will be used for the left eye, and the other topo (of the
    // same rows x cols), will be used for the right eye.  The difference between
    // the two topos is that different GPUs and displays will be used.
    //
    NV_MOSAIC_TOPO_BEGIN_PASSIVE_STEREO,    // value shadowed in nvEscDef.h
    NV_MOSAIC_TOPO_1x2_PASSIVE_STEREO = NV_MOSAIC_TOPO_BEGIN_PASSIVE_STEREO,
    NV_MOSAIC_TOPO_2x1_PASSIVE_STEREO,
    NV_MOSAIC_TOPO_1x3_PASSIVE_STEREO,
    NV_MOSAIC_TOPO_3x1_PASSIVE_STEREO,
    NV_MOSAIC_TOPO_1x4_PASSIVE_STEREO,
    NV_MOSAIC_TOPO_4x1_PASSIVE_STEREO,
    NV_MOSAIC_TOPO_2x2_PASSIVE_STEREO,
    NV_MOSAIC_TOPO_END_PASSIVE_STEREO = NV_MOSAIC_TOPO_2x2_PASSIVE_STEREO,

    //
    // Total number of topos.  Always leave this at the end of the enumeration.
    //
    NV_MOSAIC_TOPO_MAX

} NV_MOSAIC_TOPO;


//
// This is a topo brief structure.  It tells you what you need to know about
// a topo at a high level.  A list of these is returned when you query for the
// supported Mosaic information.
//
// If you need more detailed information about the topo, call
// NvAPI_Mosaic_GetTopoGroup() with the topo value from this structure.
//
typedef struct 
{
    NvU32                        version;            // version of this structure
    NV_MOSAIC_TOPO               topo;               // the topo
    NvU32                        enabled;            // 1 if topo is enabled, else 0
    NvU32                        isPossible;         // 1 if topo *can* be enabled, else 0

} NV_MOSAIC_TOPO_BRIEF;

#define NVAPI_MOSAIC_TOPO_BRIEF_VER         MAKE_NVAPI_VERSION(NV_MOSAIC_TOPO_BRIEF,1)


//
// Basic per display settings that are used in setting/getting the Mosaic mode
//
typedef struct 
{
    NvU32                        version;            // version of this structure
    NvU32                        width;              // per display width
    NvU32                        height;             // per display height
    NvU32                        bpp;                // bits per pixel
    NvU32                        freq;               // display frequency
} NV_MOSAIC_DISPLAY_SETTING;

#define NVAPI_MOSAIC_DISPLAY_SETTING_VER         MAKE_NVAPI_VERSION(NV_MOSAIC_DISPLAY_SETTING,1)


//
// Set a reasonable max number of display settings to support
// so arrays are bound.
//
#define NV_MOSAIC_DISPLAY_SETTINGS_MAX 40


//
// This structure is used to contain a list of supported Mosaic topos
// along with the display settings that can be used.
//
typedef struct 
{
    NvU32                       version;                                         // version of this structure
    NvU32                       topoBriefsCount;                                 // number of topos in below array
    NV_MOSAIC_TOPO_BRIEF        topoBriefs[NV_MOSAIC_TOPO_MAX];                  // list of supported topos with only brief details
    NvU32                       displaySettingsCount;                            // number of display settings in below array
    NV_MOSAIC_DISPLAY_SETTING   displaySettings[NV_MOSAIC_DISPLAY_SETTINGS_MAX]; // list of per display settings possible

} NV_MOSAIC_SUPPORTED_TOPO_INFO;

#define NVAPI_MOSAIC_SUPPORTED_TOPO_INFO_VER         MAKE_NVAPI_VERSION(NV_MOSAIC_SUPPORTED_TOPO_INFO,1)


//
// Indexes to use to access the topos array within the mosaic topo
//
#define NV_MOSAIC_TOPO_IDX_DEFAULT       0

#define NV_MOSAIC_TOPO_IDX_LEFT_EYE      0
#define NV_MOSAIC_TOPO_IDX_RIGHT_EYE     1
#define NV_MOSAIC_TOPO_NUM_EYES          2


//
// This defines the maximum number of topos that can be in a topo group.
// At this time, it is set to 2 because our largest topo group (passive
// stereo) only needs 2 topos (left eye and right eye).
//
// If a new topo group with more than 2 topos is added above, then this
// number will also have to be incremented.
//
#define NV_MOSAIC_MAX_TOPO_PER_TOPO_GROUP 2


//
// This structure defines a group of topos that work together to create one
// overall layout.  All of the supported topos are represented with this
// structure.
//
// For example, a 'Passive Stereo' topo would be represented with this
// structure, and would have separate topo details for the left and right eyes.
// The count would be 2.  A 'Basic' topo is also represented by this structure,
// with a count of 1.
//
// The structure is primarily used internally, but is exposed to applications in a
// read only fashion because there are some details in it that might be useful
// (like the number of rows/cols, or connected display information).  A user can
// get the filled in structure by calling NvAPI_Mosaic_GetTopoGroup().
//
// You can then look at the detailed values within the structure.  There are no
// entrypoints which take this structure as input (effectively making it read only).
//
typedef struct 
{
    NvU32                      version;              // version of this structure
    NV_MOSAIC_TOPO_BRIEF       brief;                // the brief details of this topo
    NvU32                      count;                // number of topos in array below
    NV_MOSAIC_TOPO_DETAILS     topos[NV_MOSAIC_MAX_TOPO_PER_TOPO_GROUP];

} NV_MOSAIC_TOPO_GROUP;

#define NVAPI_MOSAIC_TOPO_GROUP_VER         MAKE_NVAPI_VERSION(NV_MOSAIC_TOPO_GROUP,1)

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_Mosaic_GetSupportedTopoInfo
//
// DESCRIPTION:     This API returns information on the topos and display resolutions
//                  supported by Mosaic.
//
//                  NOTE: Not all topos returned can be immediately set.
//                        See 'OUT' Notes below.
//
//                  Once you get the list of supported topos, you can call
//                  NvAPI_Mosaic_GetTopoGroup() with a Mosaic topo if you need
//                  more information about that topo.
//
// PARAMETERS:      pSupportedTopoInfo(IN/OUT):  Information about what topos and display resolutions
//                                               are supported for Mosaic.
//                  type(IN):                    The type of topos the caller is interested in
//                                               getting.  See NV_MOSAIC_TOPO_TYPE for possible
//                                               values.
//  SUPPORTED OS: Windows XP
//
//     'IN' Notes:  pSupportedTopoInfo->version must be set before calling this function.
//                  If the specified version is not supported by this implementation,
//                  an error will be returned (NVAPI_INCOMPATIBLE_STRUCT_VERSION).
//
//     'OUT' Notes: Some of the topos returned might not be valid for one reason or
//                  another.  It could be due to mismatched or missing displays.  It
//                  could also be because the required number of GPUs is not found.
//                  At a high level, you can see if the topo is valid and can be enabled
//                  by looking at the pSupportedTopoInfo->topoBriefs[xxx].isPossible flag.
//                  If this is true, the topo can be enabled.  Otherwise, if it
//                  is false, you can find out why it cannot be enabled by getting the
//                  details of the topo via NvAPI_Mosaic_GetTopoGroup().  From there,
//                  look at the validityMask of the individual topos.  The bits can
//                  be tested against the NV_MOSAIC_TOPO_VALIDITY_* bits.
//
//                  It is possible for this function to return NVAPI_OK with no topos
//                  listed in the return structure.  If this is the case, it means that
//                  the current hardware DOES support Mosaic, but with the given configuration
//                  no valid topos were found.  This most likely means that SLI was not
//                  enabled for the hardware.  Once enabled, you should see valid topos
//                  returned from this function.
//
// RETURN STATUS    NVAPI_OK:                          No errors in returning supported topos
//                  NVAPI_NOT_SUPPORTED:               Mosaic is not supported with the existing hardware
//                  NVAPI_INVALID_ARGUMENT:            One or more args passed in are invalid
//                  NVAPI_API_NOT_INTIALIZED:          The NvAPI API needs to be initialized first
//                  NVAPI_NO_IMPLEMENTATION:           This entrypoint not available
//                  NVAPI_INCOMPATIBLE_STRUCT_VERSION: The version of the structure passed in is not
//                                                     compatible with this entrypoint
//                  NVAPI_ERROR:                       Miscellaneous error occurred
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Mosaic_GetSupportedTopoInfo(NV_MOSAIC_SUPPORTED_TOPO_INFO *pSupportedTopoInfo, NV_MOSAIC_TOPO_TYPE type);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_Mosaic_GetTopoGroup
//
// DESCRIPTION:     This API returns a structure filled with the topo details
//                  for the given Mosaic topo.
//
//                  If the pTopoBrief passed in matches the topo which is
//                  current, then information in the brief and group structures
//                  will reflect what is current.  Thus the brief would have
//                  the current 'enable' status, and the group would have the
//                  current overlap values.  If there is no match, then the
//                  returned brief has an 'enable' status of FALSE (since it
//                  is obviously not enabled), and the overlap values will be 0.
//
// PARAMETERS:      pTopoBrief(IN):         The topo to get details for.
//                                          This must be one of the topo briefs
//                                          returned from NvAPI_Mosaic_GetSupportedTopoInfo().
//                  pTopoGroup(IN/OUT):     The topo details matching the brief.
//
//  SUPPORTED OS: Windows XP
//     'IN' Notes:  pTopoGroup->version must be set before calling this function.
//                  If the specified version is not supported by this implementation,
//                  an error will be returned (NVAPI_INCOMPATIBLE_STRUCT_VERSION).
//
// RETURN STATUS    NVAPI_OK:                          Details were retrieved successfully
//                  NVAPI_NOT_SUPPORTED:               Mosaic is not supported with the existing hardware
//                  NVAPI_INVALID_ARGUMENT:            One or more args passed in are invalid.
//                  NVAPI_API_NOT_INTIALIZED:          The NvAPI API needs to be initialized first
//                  NVAPI_NO_IMPLEMENTATION:           This entrypoint not available
//                  NVAPI_INCOMPATIBLE_STRUCT_VERSION: The version of the structure passed in is not
//                                                     compatible with this entrypoint
//                  NVAPI_ERROR:                       Miscellaneous error occurred
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Mosaic_GetTopoGroup(NV_MOSAIC_TOPO_BRIEF *pTopoBrief, NV_MOSAIC_TOPO_GROUP *pTopoGroup);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_Mosaic_GetOverlapLimits
//
// DESCRIPTION:     This API returns the X and Y overlap limits required if
//                  the given Mosaic topo and display settings are to be used.
//
// PARAMETERS:      pTopoBrief(IN):         The topo to get limits for.
//                                          This must be one of the topo briefs
//                                          returned from NvAPI_Mosaic_GetSupportedTopoInfo().
//                  pDisplaySetting(IN):    The display settings to get limits for.
//                                          This must be one of the settings
//                                          returned from NvAPI_Mosaic_GetSupportedTopoInfo().
//                  pMinOverlapX(OUT):      X overlap minimum
//                  pMaxOverlapX(OUT):      X overlap maximum
//                  pMinOverlapY(OUT):      Y overlap minimum
//                  pMaxOverlapY(OUT):      Y overlap maximum
//
//  SUPPORTED OS: Windows XP
//
//
// RETURN STATUS    NVAPI_OK:                          Details were retrieved successfully
//                  NVAPI_NOT_SUPPORTED:               Mosaic is not supported with the existing hardware
//                  NVAPI_INVALID_ARGUMENT:            One or more args passed in are invalid.
//                  NVAPI_API_NOT_INTIALIZED:          The NvAPI API needs to be initialized first
//                  NVAPI_NO_IMPLEMENTATION:           This entrypoint not available
//                  NVAPI_INCOMPATIBLE_STRUCT_VERSION: The version of the structure passed in is not
//                                                     compatible with this entrypoint
//                  NVAPI_ERROR:                       Miscellaneous error occurred
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Mosaic_GetOverlapLimits(NV_MOSAIC_TOPO_BRIEF *pTopoBrief, NV_MOSAIC_DISPLAY_SETTING *pDisplaySetting, NvS32 *pMinOverlapX, NvS32 *pMaxOverlapX, NvS32 *pMinOverlapY, NvS32 *pMaxOverlapY);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_Mosaic_SetCurrentTopo
//
// DESCRIPTION:     This API sets the Mosaic topo and does a mode change
//                  using the given display settings.
//
//                  If NVAPI_OK is returned, the current Mosaic topo was set
//                  correctly.  Any other status returned means the
//                  topo was not set, and remains what it was before this
//                  function was called.
//
//
//  SUPPORTED OS: Windows XP
//
// PARAMETERS:      pTopoBrief(IN):       The topo to set.
//                                        This must be one of the topos
//                                        returned from NvAPI_Mosaic_GetSupportedTopoInfo(),
//                                        and it must have an isPossible value of 1.
//                  pDisplaySetting(IN):  The per display settings to be used in the
//                                        setting of Mosaic mode.
//                                        This must be one of the settings
//                                        returned from NvAPI_Mosaic_GetSupportedTopoInfo().
//                  overlapX(IN):         The pixel overlap to use between horizontal
//                                        displays (use positive a number for overlap,
//                                        or a negative number to create a gap.)
//                                        If the overlap is out of bounds for what is
//                                        possible given the topo and display setting,
//                                        the overlap will be clamped.
//                  overlapY(IN):         The pixel overlap to use between vertical
//                                        displays (use positive a number for overlap,
//                                        or a negative number to create a gap.)
//                                        If the overlap is out of bounds for what is
//                                        possible given the topo and display setting,
//                                        the overlap will be clamped.
//                  enable(IN):           If 1, the topo being set will also be enabled,
//                                        meaning that the mode set will occur.  
//                                        Passing a 0 means you don't want to be in
//                                        Mosaic mode right now, but want to set the current
//                                        Mosaic topo so you can enable it later with
//                                        NvAPI_Mosaic_EnableCurrentTopo().
//
// RETURN STATUS    NVAPI_OK:                          Mosaic topo was set
//                  NVAPI_NOT_SUPPORTED:               Mosaic is not supported with the existing hardware
//                  NVAPI_INVALID_ARGUMENT:            One or more args passed in are invalid.
//                  NVAPI_TOPO_NOT_POSSIBLE:           The topo passed in is not currently possible
//                  NVAPI_API_NOT_INTIALIZED:          The NvAPI API needs to be initialized first
//                  NVAPI_NO_IMPLEMENTATION:           This entrypoint not available
//                  NVAPI_INCOMPATIBLE_STRUCT_VERSION: The version of the structure passed in is not
//                                                     compatible with this entrypoint
//                  NVAPI_MODE_CHANGE_FAILED:          There was an error changing the display mode
//                  NVAPI_ERROR:                       Miscellaneous error occurred
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Mosaic_SetCurrentTopo(NV_MOSAIC_TOPO_BRIEF *pTopoBrief, NV_MOSAIC_DISPLAY_SETTING *pDisplaySetting, NvS32 overlapX, NvS32 overlapY, NvU32 enable);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_Mosaic_GetCurrentTopo
//
// DESCRIPTION:     This API returns information for the current Mosaic topo.
//                  This includes topo, display settings, and overlap values.
//
//                  You can call NvAPI_Mosaic_GetTopoGroup() with the topo
//                  if you require more information on the topo.
//
//                  If there isn't a current topo, then pTopoBrief->topo will
//                  be NV_MOSAIC_TOPO_NONE.
//
// PARAMETERS:      pTopoBrief(OUT):      The current Mosaic topo
//                  pDisplaySetting(OUT): The current per display settings
//                  pOverlapX(OUT):       The pixel overlap between horizontal displays
//                  pOverlapY(OUT):       The pixel overlap between vertical displays
//
//  SUPPORTED OS: Windows XP
//
// RETURN STATUS    NVAPI_OK:                          Success getting current info
//                  NVAPI_NOT_SUPPORTED:               Mosaic is not supported with the existing hardware
//                  NVAPI_INVALID_ARGUMENT:            One or more args passed in are invalid
//                  NVAPI_API_NOT_INTIALIZED:          The NvAPI API needs to be initialized first
//                  NVAPI_NO_IMPLEMENTATION:           This entrypoint not available
//                  NVAPI_ERROR:                       Miscellaneous error occurred
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Mosaic_GetCurrentTopo(NV_MOSAIC_TOPO_BRIEF *pTopoBrief, NV_MOSAIC_DISPLAY_SETTING *pDisplaySetting, NvS32 *pOverlapX, NvS32 *pOverlapY);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_Mosaic_EnableCurrentTopo
//
// DESCRIPTION:     This API enables or disables the current Mosaic topo
//                  based on the setting of the incoming 'enable' parameter.
//
//                  When enabling, this will enable the current Mosaic topo
//                  that was previously set.  Note that when the current Mosaic
//                  topo is retrieved, it must have an isPossible value of 1 or
//                  an error will occur.
//
//                  When disabling, the current Mosaic topo is disabled.
//                  The topo information will persist, even across reboots.
//                  To re-enable the Mosaic topo, simply call this function
//                  again with the enable parameter set to 1.
//
// PARAMETERS:      enable(IN):               1 to enable the current Mosaic topo, 0 to disable it.
//
//  SUPPORTED OS: Windows XP
//
// RETURN STATUS    NVAPI_OK:                 The Mosaic topo was enabled/disabled
//                  NVAPI_NOT_SUPPORTED:      Mosaic is not supported with the existing hardware
//                  NVAPI_INVALID_ARGUMENT:   One or more args passed in are invalid
//                  NVAPI_TOPO_NOT_POSSIBLE:  The current topo is not currently possible
//                  NVAPI_MODE_CHANGE_FAILED: There was an error changing the display mode
//                  NVAPI_ERROR:              Miscellaneous error occurred
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Mosaic_EnableCurrentTopo(NvU32 enable);





////////////////////////////////////////////////////////////////////////////////////////
//
// ###########################################################################
// DELME_RUSS - DELME_RUSS - DELME_RUSS - DELME_RUSS - DELME_RUSS - DELME_RUSS
//
//   Below is the Phase 1 Mosaic stuff, the Phase 2 stuff above is what will remain
//   once Phase 2 is complete.  For a small amount of time, the two will co-exist.  As
//   soon as apps (nvapichk, NvAPITestMosaic, and CPL) are updated to use the Phase 2
//   entrypoints, the code below will be deleted.
//
// DELME_RUSS - DELME_RUSS - DELME_RUSS - DELME_RUSS - DELME_RUSS - DELME_RUSS
// ###########################################################################
//
//
// Supported topos 1x4, 4x1 and 2x2 to start with.
//
// Selected scanout targets can be one per GPU or more than one on the same GPU.
//
// SAMPLE of MOSAIC 1x4 SCAN OUT TOPO with 8 pixel horizontal overlap
//
//+-------------------------++-------------------------++-------------------------++-------------------------+
//|                         ||                         ||                         ||                         |
//|                         ||                         ||                         ||                         |
//|                         ||                         ||                         ||                         |
//|        DVI1             ||           DVI2          ||         DVI3            ||          DVI4           |
//|                         ||                         ||                         ||                         |
//|                         ||                         ||                         ||                         |
//|                         ||                         ||                         ||                         |
//|                         ||                         ||                         ||                         |
//+-------------------------++-------------------------++-------------------------++-------------------------+

#define NVAPI_MAX_MOSAIC_DISPLAY_ROWS       8
#define NVAPI_MAX_MOSAIC_DISPLAY_COLUMNS    8
#define NVAPI_MAX_MOSAIC_TOPOS              16

typedef struct
{
    NvU32 version;                             // version number of mosaic topology
    NvU32 rowCount;                            // horizontal display count
    NvU32 colCount;                            // vertical display count

    struct
    {
        NvPhysicalGpuHandle hPhysicalGPU;      // physical gpu to be used in the topology
        NvU32               displayOutputId;   // connected display target
        NvS32               overlapX;          // pixels of overlap on left of target: (+overlap, -gap)
        NvS32               overlapY;          // pixels of overlap on top of target: (+overlap, -gap)

    } gpuLayout[NVAPI_MAX_MOSAIC_DISPLAY_ROWS][NVAPI_MAX_MOSAIC_DISPLAY_COLUMNS];

} NV_MOSAIC_TOPOLOGY;

#define NVAPI_MOSAIC_TOPOLOGY_VER         MAKE_NVAPI_VERSION(NV_MOSAIC_TOPOLOGY,1)

typedef struct
{
    NvU32                   version;
    NvU32                   totalCount;                     //count of valid topologies
    NV_MOSAIC_TOPOLOGY      topos[NVAPI_MAX_MOSAIC_TOPOS];  //max topologies

} NV_MOSAIC_SUPPORTED_TOPOLOGIES;

#define NVAPI_MOSAIC_SUPPORTED_TOPOLOGIES_VER         MAKE_NVAPI_VERSION(NV_MOSAIC_SUPPORTED_TOPOLOGIES,1)


///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_GetSupportedMosaicTopologies
//
// DESCRIPTION:     This API returns all valid Mosaic topologies
//
//  SUPPORTED OS: Windows XP
//
// PARAMETERS:      pMosaicTopos(OUT):  An array of valid Mosaic topologies.
//
// RETURN STATUS    NVAPI_OK:                      Call succeeded; 1 or more topologies were returned
//                  NVAPI_INVALID_ARGUMENT:        one or more args are invalid
//                  NVAPI_MIXED_TARGET_TYPES:      Mosaic topology is only possible with all targets of the same NV_GPU_OUTPUT_TYPE.
//                  NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//                  NVAPI_NOT_SUPPORTED:           Mosaic is not supported with GPUs on this system.
//                  NVAPI_NO_ACTIVE_SLI_TOPOLOGY:  SLI is not enabled, yet needs to be, in order for this function to succeed.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GetSupportedMosaicTopologies(NV_MOSAIC_SUPPORTED_TOPOLOGIES *pMosaicTopos);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_GetCurrentMosaicTopology
//
// DESCRIPTION:     This API gets the current Mosaic topology
//
//  SUPPORTED OS: Windows XP
//
// PARAMETERS:      pMosaicTopo(OUT):  The current Mosaic topology
//                  pEnabled(OUT):     TRUE if returned topology is currently enabled, else FALSE
//
// RETURN STATUS    NVAPI_OK:                       Call succeeded.
//                  NVAPI_INVALID_ARGUMENT:         one or more args are invalid
//                  NVAPI_NVIDIA_DEVICE_NOT_FOUND:  no NVIDIA GPU driving a display was found
//                  NVAPI_NOT_SUPPORTED:            Mosaic is not supported with GPUs on this system.
//                  NVAPI_NO_ACTIVE_SLI_TOPOLOGY:   SLI is not enabled, yet needs to be, in order for this function to succeed.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GetCurrentMosaicTopology(NV_MOSAIC_TOPOLOGY *pMosaicTopo, NvU32 *pEnabled);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_SetCurrentMosaicTopology
//
// DESCRIPTION:     This API sets the Mosaic topology, and will enable it so the
//                  Mosaic display settings will be enumerated upon request.
//
//  SUPPORTED OS: Windows XP
//
// PARAMETERS:      pMosaicTopo(IN):  A valid Mosaic topology
//
// RETURN STATUS    NVAPI_OK:                      Call succeeded
//                  NVAPI_INVALID_ARGUMENT:        One or more args are invalid
//                  NVAPI_NVIDIA_DEVICE_NOT_FOUND: No NVIDIA GPU driving a display was found
//                  NVAPI_NOT_SUPPORTED:           Mosaic mode could not be set
//                  NVAPI_NO_ACTIVE_SLI_TOPOLOGY:  SLI is not enabled, yet needs to be, in order for this function to succeed.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_SetCurrentMosaicTopology(NV_MOSAIC_TOPOLOGY *pMosaicTopo);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_EnableCurrentMosaicTopology
//
// DESCRIPTION:     This API enables or disables the current Mosaic topology.
//                  When enabling, this will use the last Mosaic topology that was set.
//                  If enabled, enumeration of display settings will include valid
//                  Mosaic resolutions.  If disabled, enumeration of display settings
//                  will not include Mosaic resolutions.
//
//  SUPPORTED OS: Windows XP
//
// PARAMETERS:      enable(IN):  TRUE to enable the Mosaic Topology, FALSE to disable it.
//
// RETURN STATUS    NVAPI_OK:                      Call succeeded
//                  NVAPI_INVALID_ARGUMENT:        One or more args are invalid
//                  NVAPI_NVIDIA_DEVICE_NOT_FOUND: No NVIDIA GPU driving a display was found
//                  NVAPI_NOT_SUPPORTED:           Mosaic mode could not be enabled/disabled
//                  NVAPI_NO_ACTIVE_SLI_TOPOLOGY:  SLI is not enabled, yet needs to be, in order for this function to succeed.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_EnableCurrentMosaicTopology(NvU32 enable);



#define NVAPI_MAX_3D_Apps 128
// Structure to get both processIds and processNames
typedef struct
{
  NvU32 version;    // Structure version
  NvU32 processId;
  NvAPI_ShortString   processName;
}NV_3D_APP_INFO;

#define NV_3D_APP_INFO_VER  MAKE_NVAPI_VERSION(NV_3D_APP_INFO,1)

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_QueryNonMigratableApps
//
// PARAMETERS:      apps(IN/OUT)       -Empty structure passed as an input and upon successfull exit, it contains list of nonmigratable apps and processIDs.
//                  total(IN/OUT)      -Total number of nonmigratable apps currently running in the system.
// DESCRIPTION:     Query all non-migratable apps which can block successful SLI or Hybrid transition.
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_ERROR - miscellaneous error occurred
//                  NVAPI_INVALID_ARGUMENT - Invalid input parameter
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_QueryNonMigratableApps(NV_3D_APP_INFO apps[NVAPI_MAX_3D_Apps] , NvU32 *total);


#define NVAPI_GPIO_SIZE_QUERY_ARRAY 0x00000020

typedef enum
{
    NVAPI_GPIO_DIRECTION_INPUT,
    NVAPI_GPIO_DIRECTION_OUTPUT
} NVAPI_GPIO_DIRECTION;

typedef struct
{
    NvU32         version;
    NvU32         gpioPinCount;
    NvU32         gpioLegalPins[NVAPI_GPIO_SIZE_QUERY_ARRAY];
} NV_GPU_GPIO_LEGAL_PINS;
#define NV_GPU_GPIO_LEGAL_PINS_VER MAKE_NVAPI_VERSION(NV_GPU_GPIO_LEGAL_PINS,1)

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GPIOQueryLegalPins
//
// PARAMETERS:    hPhysicalGpu(IN)        - GPU selection.
//                pgpioLegalPins          - pointer to a versioned structure to get number of the pin(s) associated with the
//                                          CUSTOMER_ASYNCRW functions and an array of size NVAPI_GPIO_SIZE_QUERY_ARRAY in
//                                          which the legal pin numbers are retrieved. These legal pin numbers are to be used
//                                          while performing read and write operations on the GPIO port.
//
// DESCRIPTION:   This function will return the number of available gpio customer asyncrw instance.
//
//  SUPPORTED OS: Windows XP and higher
//
// HOW TO USE:    NV_GPU_GPIO_LEGAL_PINS gpioLegalPins = {0};
//                gpioLegalPins.version = NV_GPU_GPIO_LEGAL_PINS_VER;
//                ret = NvAPI_GPU_GPIOQueryLegalPins(hPhysicalGpu, &gpioLegalPins);
//                On call success:
//                The gpioLegalPins.gpioPinCount would contain some integer (On my G96 it contains 3)
//                The gpioLegalPins.gpioLegalPins array would contain some values (on my G96 it contains 0xd, 0xe & 0xf, others 0) to be used
//                in NvAPI_GPU_GPIOReadFromPin & NvAPI_GPU_GPIOWriteToPin calls respectively.
//
// RETURN STATUS:
//                NVAPI_OK - completed request
//                NVAPI_ERROR - miscellaneous error occurred
//                NVAPI_NOT_SUPPORTED - this feature is not supported on this GPU
//                NVAPI_INVALID_HANDLE - physical GPU not found
//                NVAPI_INVALID_ARGUMENT - invalid arugument passed
//                NVAPI_HANDLE_INVALIDATED - handle passed has been invalidated (see user guide)
//                NVAPI_API_NOT_INITIALIZED - nvapi not initialized
//                NVAPI_INCOMPATIBLE_STRUCT_VERSION - structure(s) passed not initialized with proper version data
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GPIOQueryLegalPins(NvPhysicalGpuHandle hPhysicalGpu, NV_GPU_GPIO_LEGAL_PINS *pgpioLegalPins);

typedef struct
{
    NvU32         version;
    NvU32         gpioPinNumber;
    NvU32         gpioDataReadWrite;
    NVAPI_GPIO_DIRECTION         gpioDirection;
} NV_GPU_GPIO_PIN_DATA;
#define NV_GPU_GPIO_PIN_DATA_VER MAKE_NVAPI_VERSION(NV_GPU_GPIO_PIN_DATA,1)

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GPIOReadFromPin
//
// PARAMETERS:    hPhysicalGpu(IN)        - GPU selection.
//                pgpioReadPin            - pointer to a versioned structure to pass gpio pin number from which the data is
//                                          to be read, the GPIO direction and the variable in which the data is collected.
//                                          The gpioPinNumber parameter is selected from pgpioLegalPins array by calling
//                                          NvAPI_GPU_GPIOQueryLegalPins.
//
// DESCRIPTION:   This function reads the data from the specified gpio pin.
//
//  SUPPORTED OS: Windows XP and higher
//
// HOW TO USE:    First make NvAPI_GPU_GPIOQueryLegalPins call as shown above
//                NV_GPU_GPIO_PIN_DATA gpioReadPin = {0};
//                gpioReadPin.version = NV_GPU_GPIO_PIN_DATA_VER;
//                gpioReadPin.gpioDirection = NVAPI_GPIO_DIRECTION_INPUT;
//                gpioReadPin.gpioPinNumber = any legal pin data from gpioLegalPins array after successful call to NvAPI_GPU_GPIOQueryLegalPins
//                ret = NvAPI_GPU_GPIOReadFromPin(hPhysicalGpu, &gpioReadPin);
//                On call success:
//                The gpioReadPin.gpioDataReadWrite would contain the data value read (0 or 1) from the specified pin.
//
// RETURN STATUS:
//                NVAPI_OK - completed request
//                NVAPI_ERROR - miscellaneous error occurred
//                NVAPI_INVALID_HANDLE - physical GPU not found
//                NVAPI_INVALID_ARGUMENT - invalid arugument passed
//                NVAPI_HANDLE_INVALIDATED - handle passed has been invalidated (see user guide)
//                NVAPI_API_NOT_INITIALIZED - nvapi not initialized
//                NVAPI_INCOMPATIBLE_STRUCT_VERSION - structure(s) passed not initialized with proper version data
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GPIOReadFromPin(NvPhysicalGpuHandle hPhysicalGpu, NV_GPU_GPIO_PIN_DATA *pgpioReadPin);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GPIOWriteToPin
//
// PARAMETERS:    hPhysicalGpu(IN)        - GPU selection.
//                pgpioWritePin           - pointer to a versioned structure to pass gpio pin number to which the data is
//                                          to be written, the GPIO direction and the data value. The gpioPinNumber parameter
//                                          is selected from pgpioLegalPins array by calling NvAPI_GPU_GPIOQueryLegalPins.
//
// DESCRIPTION:   This function writes the data to the specified gpio pin.
//
//  SUPPORTED OS: Windows XP and higher
//
// HOW TO USE:    First make NvAPI_GPU_GPIOQueryLegalPins call as shown above
//                NV_GPU_GPIO_PIN_DATA gpioWritePin = {0};
//                gpioWritePin.version = NV_GPU_GPIO_PIN_DATA_VER;
//                gpioWritePin.gpioDirection = NVAPI_GPIO_DIRECTION_OUTPUT;
//                gpioWritePin.gpioPinNumber = any legal pin data from gpioLegalPins array after successful call to NvAPI_GPU_GPIOQueryLegalPins
//                gpioWritePin.gpioDataReadWrite = any bit value (0 or 1)
//                ret = NvAPI_GPU_GPIOReadFromPin(hPhysicalGpu, &gpioReadPin);
//                On call success:
//                The gpioWritePin.gpioDataReadWrite value (0 or 1) is written to the specified pin.
//
// RETURN STATUS:
//                NVAPI_OK - completed request
//                NVAPI_ERROR - miscellaneous error occurred
//                NVAPI_INVALID_HANDLE - physical GPU not found
//                NVAPI_INVALID_ARGUMENT - invalid arugument passed
//                NVAPI_HANDLE_INVALIDATED - handle passed has been invalidated (see user guide)
//                NVAPI_API_NOT_INITIALIZED - nvapi not initialized
//                NVAPI_INCOMPATIBLE_STRUCT_VERSION - structure(s) passed not initialized with proper version data
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GPIOWriteToPin(NvPhysicalGpuHandle hPhysicalGpu, NV_GPU_GPIO_PIN_DATA *pgpioWritePin);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GetHDCPSupportStatus
//
// DESCRIPTION: Returns information on a GPU's HDCP support status
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// RETURN STATUS:
//    NVAPI_OK - completed request
//    NVAPI_ERROR - miscellaneous error occurred
//    NVAPI_INVALID_ARGUMENT - pGetGpuHdcpSupportStatus is NULL
//    NVAPI_HANDLE_INVALIDATED - handle passed has been invalidated (see user guide)
//    NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE - handle passed is not a physical GPU handle
//    NVAPI_INCOMPATIBLE_STRUCT_VERSION - the version of the INFO struct is not supported
//
///////////////////////////////////////////////////////////////////////////////

typedef enum _NV_GPU_HDCP_FUSE_STATE
{
    NV_GPU_HDCP_FUSE_STATE_UNKNOWN  = 0,
    NV_GPU_HDCP_FUSE_STATE_DISABLED = 1,
    NV_GPU_HDCP_FUSE_STATE_ENABLED  = 2,
} NV_GPU_HDCP_FUSE_STATE;

typedef enum _NV_GPU_HDCP_KEY_SOURCE
{
    NV_GPU_HDCP_KEY_SOURCE_UNKNOWN    = 0,
    NV_GPU_HDCP_KEY_SOURCE_NONE       = 1,
    NV_GPU_HDCP_KEY_SOURCE_CRYPTO_ROM = 2,
    NV_GPU_HDCP_KEY_SOURCE_SBIOS      = 3,
    NV_GPU_HDCP_KEY_SOURCE_I2C_ROM    = 4,
    NV_GPU_HDCP_KEY_SOURCE_FUSES      = 5,
} NV_GPU_HDCP_KEY_SOURCE;

typedef enum _NV_GPU_HDCP_KEY_SOURCE_STATE
{
    NV_GPU_HDCP_KEY_SOURCE_STATE_UNKNOWN = 0,
    NV_GPU_HDCP_KEY_SOURCE_STATE_ABSENT  = 1,
    NV_GPU_HDCP_KEY_SOURCE_STATE_PRESENT = 2,
} NV_GPU_HDCP_KEY_SOURCE_STATE;

typedef struct
{
    NvU32                        version;               // Structure version
    NV_GPU_HDCP_FUSE_STATE       hdcpFuseState;         // GPU's HDCP fuse state
    NV_GPU_HDCP_KEY_SOURCE       hdcpKeySource;         // GPU's HDCP key source
    NV_GPU_HDCP_KEY_SOURCE_STATE hdcpKeySourceState;    // GPU's HDCP key source state
} NV_GPU_GET_HDCP_SUPPORT_STATUS;

#define NV_GPU_GET_HDCP_SUPPORT_STATUS_VER MAKE_NVAPI_VERSION(NV_GPU_GET_HDCP_SUPPORT_STATUS,1)

NVAPI_INTERFACE NvAPI_GPU_GetHDCPSupportStatus(NvPhysicalGpuHandle hPhysicalGpu, NV_GPU_GET_HDCP_SUPPORT_STATUS *pGetHDCPSupportStatus);

 
///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_SetTopologyFocusDisplayAndView
// DESCRIPTION:     This API works on the active display GPU and will let the calling app switch 
//                    focus display, and accepts a path info to specify single, clone or dualview.
//                   Note : If SLI is not active then this API returns NVAPI_NO_ACTIVE_SLI_TOPOLOGY.
//
//  SUPPORTED OS: Windows Vista and higher
//                      
// PARAMETERS:      hLogicalGPU(IN) - Active logical gpu topology containing more than one physical gpus.
//                  focusDisplayOutputId(IN) - Connected display output Id on the target GPU which should be focused.
//                  pPathInfo(IN) - Pointer to NV_VIEW_PATH_INFO, specifying device properties in this view.
//                                    The first device entry in the array is the physical primary.
//                                    The device entry with the lowest source id is the desktop primary.
//                  displayView(IN)- Display view selected from NV_TARGET_VIEW_MODE.
//                  
// RETURN STATUS: 
//                  NVAPI_OK - completed request
//                  NVAPI_ERROR - miscellaneous error occurred
//                  NVAPI_INVALID_ARGUMENT - Invalid input parameter.
//                  NVAPI_NO_ACTIVE_SLI_TOPOLOGY - SLI is not active on this device.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_SetTopologyFocusDisplayAndView(NvLogicalGpuHandle hLogicalGPU, NvU32 focusDisplayOutputId, NV_DISPLAY_PATH_INFO *pPathInfo, NV_TARGET_VIEW_MODE displayView);


///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_CreateConfigurationProfileRegistryKey
//
//  SUPPORTED OS: Windows Vista and higher
//
// DESCRIPTION:   Creates new configuration registry key for current application.
//
//                If there was no configuration profile prior to the function call, 
//                tries to create brand new configuration profile registry key 
//                for a given application and fill it with default values. 
//                If an application already had a configuration profile registry key, does nothing. 
//                Name of the key is automatically determined as the name of the executable that calls this function.
//                Because of this, application executable should have distinct and unique name.
//                If the application is using only one version of DirectX, than the default profile type will be appropriate.
//                If the application is using more than one version of DirectX from same executable,
//                it should use appropriate profile type for each configuration profile.
//
// PARAMETERS:    registryProfileType(IN) - Type of profile that application wants to create.
//                                          Should be one of the symbolic constants defined in NV_STEREO_REGISTRY_PROFILE_TYPE.
//                                          Any other value will cause function to do nothing and return NV_STEREO_REGISTRY_PROFILE_TYPE_NOT_SUPPORTED.
//
// HOW TO USE:    When there is a need for an application to have default stereo parameter values,
//                use this function to create a key where they will be stored.
//
// RETURN STATUS:
//                NVAPI_OK - Key exists in the registry.
//                NVAPI_STEREO_REGISTRY_PROFILE_TYPE_NOT_SUPPORTED - This profile type is not supported.
//                NVAPI_STEREO_REGISTRY_ACCESS_FAILED - Access to registry failed.
//                NVAPI_API_NOT_INTIALIZED - NVAPI not initialized.
//                NVAPI_STEREO_NOT_INITIALIZED - Stereo part of NVAPI not initialized.
//                NVAPI_ERROR - Something is wrong (generic error).
//
///////////////////////////////////////////////////////////////////////////////

typedef enum _NV_StereoRegistryProfileType
{
    NVAPI_STEREO_DEFAULT_REGISTRY_PROFILE, // Default registry configuration profile.
    NVAPI_STEREO_DX9_REGISTRY_PROFILE,     // Separate registry configuration profile for DX9 executable.
    NVAPI_STEREO_DX10_REGISTRY_PROFILE     // Separate registry configuration profile for DX10 executable.
} NV_STEREO_REGISTRY_PROFILE_TYPE;

NVAPI_INTERFACE NvAPI_Stereo_CreateConfigurationProfileRegistryKey(NV_STEREO_REGISTRY_PROFILE_TYPE registryProfileType);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_DeleteConfigurationProfileRegistryKey
//
//  SUPPORTED OS: Windows Vista and higher
//
// DESCRIPTION:   Removes configuration registry key for current application.
//
//                If an application already had a configuration profile prior to the function call, 
//                this function will try to remove application's configuration profile registry key from the registry. 
//                If there was no configuration profile registry key prior to the function call, 
//                the function will do nothing and will not report an error. 
//  
// PARAMETERS:    registryProfileType(IN) - Type of profile that application wants to delete.
//                                          Should be one of the symbolic constants defined in NV_STEREO_REGISTRY_PROFILE_TYPE.
//                                          Any other value will cause function to do nothing and return NV_STEREO_REGISTRY_PROFILE_TYPE_NOT_SUPPORTED.
//
// RETURN STATUS:
//                NVAPI_OK - Key does not exist in the registry any more.
//                NVAPI_STEREO_REGISTRY_PROFILE_TYPE_NOT_SUPPORTED - This profile type is not supported.
//                NVAPI_STEREO_REGISTRY_ACCESS_FAILED - Access to registry failed.
//                NVAPI_API_NOT_INTIALIZED - NVAPI not initialized.
//                NVAPI_STEREO_NOT_INITIALIZED - Stereo part of NVAPI not initialized.
//                NVAPI_ERROR - Something is wrong (generic error).
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Stereo_DeleteConfigurationProfileRegistryKey(NV_STEREO_REGISTRY_PROFILE_TYPE registryProfileType);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_SetConfigurationProfileValue
//
// PARAMETERS:    registryProfileType(IN) - Type of profile that application wants to access.
//                                          Should be one of the symbolic constants defined in NV_STEREO_REGISTRY_PROFILE_TYPE.
//                                          Any other value will cause function to do nothing and return NV_STEREO_REGISTRY_PROFILE_TYPE_NOT_SUPPORTED.
//                valueRegistryID(IN)     - ID of the value that is being set.
//                                          Should be one of the symbolic constants defined in NV_STEREO_REGISTRY_PROFILE_TYPE.
//                                          Any other value will cause function to do nothing and return NVAPI_STEREO_REGISTRY_VALUE_NOT_SUPPORTED.
//                pValue(IN)              - Address of the value that is being set. 
//                                          Should be either address of a DWORD or of a float, 
//                                          dependent on the type of the stereo parameter whose value is being set. 
//                                          The API will then cast that address to DWORD* 
//                                          and write whatever is in those 4 bytes as a DWORD to the registry. 
//
//  SUPPORTED OS: Windows Vista and higher
//
// DESCRIPTION:   Sets given parameter value under the application's registry key. 
//
//                If the value does not exist under the application's registry key, 
//                the value will be created under the key. 
//
// RETURN STATUS:
//                NVAPI_OK - Value is written to registry.
//                NVAPI_STEREO_REGISTRY_PROFILE_TYPE_NOT_SUPPORTED - This profile type is not supported.
//                NVAPI_STEREO_REGISTRY_VALUE_NOT_SUPPORTED - This value is not supported.
//                NVAPI_STEREO_REGISTRY_ACCESS_FAILED - Access to registry failed.
//                NVAPI_API_NOT_INTIALIZED - NVAPI not initialized.
//                NVAPI_STEREO_NOT_INITIALIZED - Stereo part of NVAPI not initialized.
//                NVAPI_ERROR - Something is wrong (generic error).
//
///////////////////////////////////////////////////////////////////////////////

typedef enum _NV_StereoRegistryID
{
    NVAPI_CONVERGENCE_ID,         // Symbolic constant for convergence registry ID.
    NVAPI_FRUSTUM_ADJUST_MODE_ID, // Symbolic constant for frustum adjust mode registry ID.
} NV_STEREO_REGISTRY_ID;

NVAPI_INTERFACE NvAPI_Stereo_SetConfigurationProfileValue(NV_STEREO_REGISTRY_PROFILE_TYPE registryProfileType, NV_STEREO_REGISTRY_ID valueRegistryID, void *pValue);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_DeleteConfigurationProfileValue
//
// PARAMETERS:    registryProfileType(IN) - Type of profile that application wants to access.
//                                          Should be one of the symbolic constants defined in NV_STEREO_REGISTRY_PROFILE_TYPE.
//                                          Any other value will cause function to do nothing and return NV_STEREO_REGISTRY_PROFILE_TYPE_NOT_SUPPORTED.
//                valueRegistryID(IN)     - ID of the value that is being deleted.
//                                          Should be one of the symbolic constants defined in NV_STEREO_REGISTRY_PROFILE_TYPE.
//                                          Any other value will cause function to do nothing and return NVAPI_STEREO_REGISTRY_VALUE_NOT_SUPPORTED.
//
//  SUPPORTED OS: Windows Vista and higher
//
// DESCRIPTION:   Removes given value from application's configuration profile registry key. 
//
//                If there is no such value, the function will do nothing and will not report an error. 
//
// RETURN STATUS:
//                NVAPI_OK - Value does not exist in registry any more.
//                NVAPI_STEREO_REGISTRY_PROFILE_TYPE_NOT_SUPPORTED - This profile type is not supported.
//                NVAPI_STEREO_REGISTRY_VALUE_NOT_SUPPORTED - This value is not supported.
//                NVAPI_STEREO_REGISTRY_ACCESS_FAILED - Access to registry failed.
//                NVAPI_API_NOT_INTIALIZED - NVAPI not initialized.
//                NVAPI_STEREO_NOT_INITIALIZED - Stereo part of NVAPI not initialized.
//                NVAPI_ERROR - Something is wrong (generic error).
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Stereo_DeleteConfigurationProfileValue(NV_STEREO_REGISTRY_PROFILE_TYPE registryProfileType, NV_STEREO_REGISTRY_ID valueRegistryID);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_Enable
//
//  SUPPORTED OS: Windows Vista and higher
//
// DESCRIPTION:   Enables stereo mode in the registry. 
//                Call to this function affects entire system.
//                Calls to functions that require stereo enabled with stereo disabled will have no effect,
//                and will return apropriate error code.
//
// RETURN STATUS:
//                NVAPI_OK - Stereo is now enabled.
//                NVAPI_API_NOT_INTIALIZED - NVAPI not initialized.
//                NVAPI_STEREO_NOT_INITIALIZED - Stereo part of NVAPI not initialized.
//                NVAPI_ERROR - Something is wrong (generic error).
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Stereo_Enable(void);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_Disable
//
//  SUPPORTED OS: Windows Vista and higher
//
// DESCRIPTION:   Disables stereo mode in the registry. 
//                Call to this function affects entire system.
//                Calls to functions that require stereo enabled with stereo disabled will have no effect,
//                and will return apropriate error code.
//
// RETURN STATUS:
//                NVAPI_OK - Stereo is now disabled.
//                NVAPI_API_NOT_INTIALIZED - NVAPI not initialized.
//                NVAPI_STEREO_NOT_INITIALIZED - Stereo part of NVAPI not initialized.
//                NVAPI_ERROR - Something is wrong (generic error).
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Stereo_Disable(void);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_IsEnabled
//
// PARAMETERS:    pIsStereoEnabled(OUT)  - Address where result of the inquiry will be placed.
//
//  SUPPORTED OS: Windows Vista and higher
//
// DESCRIPTION:   Checks if stereo mode is enabled in the registry. 
//
// RETURN STATUS:
//                NVAPI_OK - Check was sucessfully completed and result reflects current state of stereo availability.
//                NVAPI_API_NOT_INTIALIZED - NVAPI not initialized.
//                NVAPI_STEREO_NOT_INITIALIZED - Stereo part of NVAPI not initialized.
//                NVAPI_ERROR - Something is wrong (generic error).
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Stereo_IsEnabled(NvU8 *pIsStereoEnabled);



#if defined(_D3D9_H_) || defined(__d3d10_h__)
///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_CreateHandleFromIUnknown
//
// PARAMETERS:    pDevice(IN) - Pointer to IUnknown interface that IDirect3DDevice9* in DX9, ID3D10Device*.
//                pStereoHandle(OUT) - Pointer to newly created stereo handle.
//
//  SUPPORTED OS: Windows Vista and higher
//
// DESCRIPTION:   Creates stereo handle, that is used in subsequent calls related to given device interface. 
//                This must be called before any other NvAPI_Stereo_ function for that handle.
//                Multiple devices can be used at one time using multiple calls to this function (one per each device).
//
// HOW TO USE:    After the Direct3D device is created, create stereo handle.
//                On call success:
//                Use all other NvAPI_Stereo_ functions that have stereo handle as first parameter.
//                After the device interface correspondent to the stereo handle is destroyed, 
//                application should call NvAPI_DestroyStereoHandle for that stereo handle.
//
// RETURN STATUS:
//                NVAPI_OK - Stereo handle is created for given device interface.
//                NVAPI_INVALID_ARGUMENT - Provided device interface is invalid.
//                NVAPI_API_NOT_INTIALIZED - NVAPI not initialized.
//                NVAPI_STEREO_NOT_INITIALIZED - Stereo part of NVAPI not initialized.
//                NVAPI_ERROR - Something is wrong (generic error).
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Stereo_CreateHandleFromIUnknown(IUnknown *pDevice, StereoHandle *pStereoHandle);
#endif // defined(_D3D9_H_) || defined(__d3d10_h__)

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_DestroyHandle
//
// PARAMETERS:    stereoHandle(IN) - Stereo handle that is to be destroyed.
//
//  SUPPORTED OS: Windows Vista and higher
//
// DESCRIPTION:   Destroys stereo handle created with one of NvAPI_Stereo_CreateHandleFrom functions. 
//                This should be called after device corresponding to the handle has been destroyed.
//
// RETURN STATUS:
//                NVAPI_OK - Stereo handle is destroyed.
//                NVAPI_API_NOT_INTIALIZED - NVAPI not initialized.
//                NVAPI_STEREO_NOT_INITIALIZED - Stereo part of NVAPI not initialized.
//                NVAPI_ERROR - Something is wrong (generic error).
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Stereo_DestroyHandle(StereoHandle stereoHandle);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_Activate
//
// PARAMETERS:    stereoHandle(IN) - Stereo handle correspondent to device interface.
//
//  SUPPORTED OS: Windows Vista and higher
//
// DESCRIPTION:   Activates stereo for device interface correspondent to given stereo handle.
//                Activating stereo will be possible only if stereo was enabled previously in the registry.
//                Calls to all functions that require stereo activated  
//                with stereo deactivated will have no effect and will return appropriate error code.
//
// HOW TO USE:    After the stereo handle for device interface is created via successfull call to appropriate NvAPI_Stereo_CreateHandleFrom function.
//
// RETURN STATUS:
//                NVAPI_OK - Stereo is turned on.
//                NVAPI_STEREO_INVALID_DEVICE_INTERFACE - Device interface is not valid. Create again, then attach again.
//                NVAPI_API_NOT_INTIALIZED - NVAPI not initialized.
//                NVAPI_STEREO_NOT_INITIALIZED - Stereo part of NVAPI not initialized.
//                NVAPI_ERROR - Something is wrong (generic error).
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Stereo_Activate(StereoHandle stereoHandle);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_Deactivate
//
// PARAMETERS:    stereoHandle(IN) - Stereo handle correspondent to device interface.
//
//  SUPPORTED OS: Windows Vista and higher
//
// DESCRIPTION:   Deactivates stereo for given device interface.
//                Calls to all functions that require stereo activated  
//                with stereo deactivated will have no effect and will return appropriate error code.
//
// HOW TO USE:    After the stereo handle for device interface is created via successfull call to appropriate NvAPI_Stereo_CreateHandleFrom function.
//
// RETURN STATUS:
//                NVAPI_OK - Stereo is turned off.
//                NVAPI_STEREO_INVALID_DEVICE_INTERFACE - Device interface is not valid. Create again, then attach again.
//                NVAPI_API_NOT_INTIALIZED - NVAPI not initialized.
//                NVAPI_STEREO_NOT_INITIALIZED - Stereo part of NVAPI not initialized.
//                NVAPI_ERROR - Something is wrong (generic error).
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Stereo_Deactivate(StereoHandle stereoHandle);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_IsActivated
//
// PARAMETERS:    stereoHandle(IN) - Stereo handle correspondent to device interface.
//                pIsStereoOn(IN)  - Address where result of the inquiry will be placed.
//
//  SUPPORTED OS: Windows Vista and higher
//
// DESCRIPTION:   Checks if stereo is activated for given device interface. 
//
// HOW TO USE:    After the stereo handle for device interface is created via successfull call to appropriate NvAPI_Stereo_CreateHandleFrom function.
//
// RETURN STATUS:
//                NVAPI_OK - Check was sucessfully completed and result reflects current state of stereo (on/off).
//                NVAPI_STEREO_INVALID_DEVICE_INTERFACE - Device interface is not valid. Create again, then attach again.
//                NVAPI_API_NOT_INTIALIZED - NVAPI not initialized.
//                NVAPI_STEREO_NOT_INITIALIZED - Stereo part of NVAPI not initialized.
//                NVAPI_ERROR - Something is wrong (generic error).
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Stereo_IsActivated(StereoHandle stereoHandle, NvU8 *pIsStereoOn);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_GetSeparation
//
// PARAMETERS:    stereoHandle(IN)           - Stereo handle correspondent to device interface.
//                pSeparationPercentage(OUT) - Address of @c float type variable to store current separation percentage in.
//
//  SUPPORTED OS: Windows Vista and higher
//
// DESCRIPTION:   Gets current separation value (in percents). 
//
// HOW TO USE:    After the stereo handle for device interface is created via successfull call to appropriate NvAPI_Stereo_CreateHandleFrom function.
//
// RETURN STATUS:
//                NVAPI_OK - Retrieval of separation percentage was successfull.
//                NVAPI_STEREO_INVALID_DEVICE_INTERFACE - Device interface is not valid. Create again, then attach again.
//                NVAPI_API_NOT_INTIALIZED - NVAPI not initialized.
//                NVAPI_STEREO_NOT_INITIALIZED - Stereo part of NVAPI not initialized.
//                NVAPI_ERROR - Something is wrong (generic error).
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Stereo_GetSeparation(StereoHandle stereoHandle, float *pSeparationPercentage);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_SetSeparation
//
// PARAMETERS:    stereoHandle(IN)            - Stereo handle correspondent to device interface.
//                newSeparationPercentage(IN) - New value for separation percentage.
//
//  SUPPORTED OS: Windows Vista and higher
//
// DESCRIPTION:   Sets separation to given percentage. 
//
// HOW TO USE:    After the stereo handle for device interface is created via successfull call to appropriate NvAPI_Stereo_CreateHandleFrom function.
//
// RETURN STATUS:
//                NVAPI_OK - Setting of separation percentage was successfull.
//                NVAPI_STEREO_INVALID_DEVICE_INTERFACE - Device interface is not valid. Create again, then attach again.
//                NVAPI_API_NOT_INTIALIZED - NVAPI not initialized.
//                NVAPI_STEREO_NOT_INITIALIZED - Stereo part of NVAPI not initialized.
//                NVAPI_STEREO_PARAMETER_OUT_OF_RANGE - Given separation percentage is out of [0..100] range.
//                NVAPI_ERROR - Something is wrong (generic error).
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Stereo_SetSeparation(StereoHandle stereoHandle, float newSeparationPercentage);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_DecreaseSeparation
//
// PARAMETERS:    stereoHandle(IN) - Stereo handle correspondent to device interface.
//
//  SUPPORTED OS: Windows Vista and higher
//
// DESCRIPTION:   Decreases separation for given device interface (same like Ctrl+F3 hotkey).
//
// HOW TO USE:    After the stereo handle for device interface is created via successfull call to appropriate NvAPI_Stereo_CreateHandleFrom function.
//
// RETURN STATUS:
//                NVAPI_OK - Decrease of separation percentage was successfull.
//                NVAPI_STEREO_INVALID_DEVICE_INTERFACE - Device interface is not valid. Create again, then attach again.
//                NVAPI_API_NOT_INTIALIZED - NVAPI not initialized.
//                NVAPI_STEREO_NOT_INITIALIZED - Stereo part of NVAPI not initialized.
//                NVAPI_ERROR - Something is wrong (generic error).
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Stereo_DecreaseSeparation(StereoHandle stereoHandle);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_IncreaseSeparation
//
// PARAMETERS:    stereoHandle(IN) - Stereo handle correspondent to device interface.
//
//  SUPPORTED OS: Windows Vista and higher
//
// DESCRIPTION:   Increases separation for given device interface (same like Ctrl+F4 hotkey).
//
// HOW TO USE:    After the stereo handle for device interface is created via successfull call to appropriate NvAPI_Stereo_CreateHandleFrom function.
//
// RETURN STATUS:
//                NVAPI_OK - Increase of separation percentage was successfull.
//                NVAPI_STEREO_INVALID_DEVICE_INTERFACE - Device interface is not valid. Create again, then attach again.
//                NVAPI_API_NOT_INTIALIZED - NVAPI not initialized.
//                NVAPI_STEREO_NOT_INITIALIZED - Stereo part of NVAPI not initialized.
//                NVAPI_ERROR - Something is wrong (generic error).
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Stereo_IncreaseSeparation(StereoHandle stereoHandle);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_GetConvergence
//
// PARAMETERS:    stereoHandle(IN)  - Stereo handle correspondent to device interface.
//                pConvergence(OUT) - Address of @c float type variable to store current convergence value in.
//
//  SUPPORTED OS: Windows Vista and higher
//
// DESCRIPTION:   Gets current convergence value. 
//
// HOW TO USE:    After the stereo handle for device interface is created via successfull call to appropriate NvAPI_Stereo_CreateHandleFrom function.
//
// RETURN STATUS:
//                NVAPI_OK - Retrieval of convergence value was successfull.
//                NVAPI_STEREO_INVALID_DEVICE_INTERFACE - Device interface is not valid. Create again, then attach again.
//                NVAPI_API_NOT_INTIALIZED - NVAPI not initialized.
//                NVAPI_STEREO_NOT_INITIALIZED - Stereo part of NVAPI not initialized.
//                NVAPI_ERROR - Something is wrong (generic error).
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Stereo_GetConvergence(StereoHandle stereoHandle, float *pConvergence);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_SetConvergence
//
// PARAMETERS:    stereoHandle(IN)             - Stereo handle correspondent to device interface.
//                newConvergencePercentage(IN) - New value for convergence.
//
//  SUPPORTED OS: Windows Vista and higher
//
// DESCRIPTION:   Sets convergence to given value. 
//
// HOW TO USE:    After the stereo handle for device interface is created via successfull call to appropriate NvAPI_Stereo_CreateHandleFrom function.
//
// RETURN STATUS:
//                NVAPI_OK - Setting of convergence value was successfull.
//                NVAPI_STEREO_INVALID_DEVICE_INTERFACE - Device interface is not valid. Create again, then attach again.
//                NVAPI_API_NOT_INTIALIZED - NVAPI not initialized.
//                NVAPI_STEREO_NOT_INITIALIZED - Stereo part of NVAPI not initialized.
//                NVAPI_ERROR - Something is wrong (generic error).
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Stereo_SetConvergence(StereoHandle stereoHandle, float newConvergencePercentage);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_DecreaseConvergence
//
// PARAMETERS:    stereoHandle(IN) - Stereo handle correspondent to device interface.
//
//  SUPPORTED OS: Windows Vista and higher
//
// DESCRIPTION:   Decreases convergence for given device interface (same like Ctrl+F5 hotkey).
//
// HOW TO USE:    After the stereo handle for device interface is created via successfull call to appropriate NvAPI_Stereo_CreateHandleFrom function.
//
// RETURN STATUS:
//                NVAPI_OK - Decrease of convergence was successfull.
//                NVAPI_STEREO_INVALID_DEVICE_INTERFACE - Device interface is not valid. Create again, then attach again.
//                NVAPI_API_NOT_INTIALIZED - NVAPI not initialized.
//                NVAPI_STEREO_NOT_INITIALIZED - Stereo part of NVAPI not initialized.
//                NVAPI_ERROR - Something is wrong (generic error).
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Stereo_DecreaseConvergence(StereoHandle stereoHandle);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_IncreaseConvergence
//
// PARAMETERS:    stereoHandle(IN) - Stereo handle correspondent to device interface.
//
//  SUPPORTED OS: Windows Vista and higher
//
// DESCRIPTION:   Increases convergence for given device interface (same like Ctrl+F5 hotkey).
//
// HOW TO USE:    After the stereo handle for device interface is created via successfull call to appropriate NvAPI_Stereo_CreateHandleFrom function.
//
// RETURN STATUS:
//                NVAPI_OK - Increase of convergence was successfull.
//                NVAPI_STEREO_INVALID_DEVICE_INTERFACE - Device interface is not valid. Create again, then attach again.
//                NVAPI_API_NOT_INTIALIZED - NVAPI not initialized.
//                NVAPI_STEREO_NOT_INITIALIZED - Stereo part of NVAPI not initialized.
//                NVAPI_ERROR - Something is wrong (generic error).
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Stereo_IncreaseConvergence(StereoHandle stereoHandle);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_GetFrustumAdjustMode
//
// PARAMETERS:    stereoHandle(IN)        - Stereo handle correspondent to device interface.
//                pFrustumAdjustMode(OUT) - Address of the NV_FRUSTUM_ADJUST_MODE type variable to store current frustum value in.
//
//  SUPPORTED OS: Windows Vista and higher
//
// DESCRIPTION:   Gets current frustum adjust mode value. 
//
// HOW TO USE:    After the stereo handle for device interface is created via successfull call to appropriate NvAPI_Stereo_CreateHandleFrom function.
//
// RETURN STATUS:
//                NVAPI_OK - Retrieval of frustum adjust mode was successfull.
//                NVAPI_STEREO_INVALID_DEVICE_INTERFACE - Device interface is not valid. Create again, then attach again.
//                NVAPI_API_NOT_INTIALIZED - NVAPI not initialized.
//                NVAPI_STEREO_NOT_INITIALIZED - Stereo part of NVAPI not initialized.
//                NVAPI_ERROR - Something is wrong (generic error).
//
///////////////////////////////////////////////////////////////////////////////

typedef enum _NV_FrustumAdjustMode
{
    NVAPI_NO_FRUSTUM_ADJUST,    // Do not adjust frustum.
    NVAPI_FRUSTUM_STRETCH,      // Stretch images in X.
    NVAPI_FRUSTUM_CLEAR_EDGES   // Clear corresponding edges for each eye.
} NV_FRUSTUM_ADJUST_MODE;

NVAPI_INTERFACE NvAPI_Stereo_GetFrustumAdjustMode(StereoHandle stereoHandle, NV_FRUSTUM_ADJUST_MODE *pFrustumAdjustMode);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_SetFrustumAdjustMode
//
// PARAMETERS:    stereoHandle(IN)               - Stereo handle correspondent to device interface.
//                newFrustumAdjustModeValue (IN) - New value for frustum adjust mode.
//                                                 Should be one of the symbolic constants defined in NV_FRUSTUM_ADJUST_MODE.
//                                                 Any other value will cause function to do nothing and return NVAPI_STEREO_FRUSTUM_ADJUST_MODE_NOT_SUPPORTED.
//
//  SUPPORTED OS: Windows Vista and higher
//
// DESCRIPTION:   Sets current frustum adjust mode value. 
//
// HOW TO USE:    After the stereo handle for device interface is created via successfull call to appropriate NvAPI_Stereo_CreateHandleFrom function.
//
// RETURN STATUS:
//                NVAPI_OK - Retrieval of frustum adjust mode was successfull.
//                NVAPI_STEREO_INVALID_DEVICE_INTERFACE - Device interface is not valid. Create again, then attach again.
//                NVAPI_API_NOT_INTIALIZED - NVAPI not initialized.
//                NVAPI_STEREO_NOT_INITIALIZED - Stereo part of NVAPI not initialized.
//                NVAPI_STEREO_FRUSTUM_ADJUST_MODE_NOT_SUPPORTED - Given frustum adjust mode is not supported.
//                NVAPI_ERROR - Something is wrong (generic error).
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Stereo_SetFrustumAdjustMode(StereoHandle stereoHandle, NV_FRUSTUM_ADJUST_MODE newFrustumAdjustModeValue);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_CaptureJpegImage
//
// PARAMETERS:    stereoHandle(IN) - Stereo handle correspondent to device interface.
//                quality(IN)      - Quality of the JPEG image to be captured. Integer value betweeen 0 and 100.
//
//  SUPPORTED OS: Windows Vista and higher
//
// DESCRIPTION:   Captures current stereo image in JPEG stereo format with given quality. 
//                Only the last capture call per flip will be effective.
//
// HOW TO USE:    After the stereo handle for device interface is created via successfull call to appropriate NvAPI_Stereo_CreateHandleFrom function.
//
// RETURN STATUS:
//                NVAPI_OK - Image captured.
//                NVAPI_STEREO_INVALID_DEVICE_INTERFACE - Device interface is not valid. Create again, then attach again.
//                NVAPI_API_NOT_INTIALIZED - NVAPI not initialized.
//                NVAPI_STEREO_NOT_INITIALIZED - Stereo part of NVAPI not initialized.
//                NVAPI_STEREO_PARAMETER_OUT_OF_RANGE - Given quality is out of [0..100] range.
//                NVAPI_ERROR - Something is wrong (generic error).
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Stereo_CaptureJpegImage(StereoHandle stereoHandle, NvU32 quality);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_CapturePngImage
//
// PARAMETERS:    stereoHandle(IN) - Stereo handle correspondent to device interface.
//
//  SUPPORTED OS: Windows Vista and higher
//
// DESCRIPTION:   Captures current stereo image in PNG stereo format. 
//                Only the last capture call per flip will be effective.
//
// HOW TO USE:    After the stereo handle for device interface is created via successfull call to appropriate NvAPI_Stereo_CreateHandleFrom function.
//
// RETURN STATUS:
//                NVAPI_OK - Image captured.
//                NVAPI_STEREO_INVALID_DEVICE_INTERFACE - Device interface is not valid. Create again, then attach again.
//                NVAPI_API_NOT_INTIALIZED - NVAPI not initialized.
//                NVAPI_STEREO_NOT_INITIALIZED - Stereo part of NVAPI not initialized.
//                NVAPI_ERROR - Something is wrong (generic error).
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Stereo_CapturePngImage(StereoHandle stereoHandle);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_ReverseStereoBlitControl
//
// PARAMETERS:    stereoHandle(IN) - Stereo handle correspondent to device interface.
//                TurnOn(IN)       != 0  - turns on, 
//                                 == 0  - turns off 
//
//  SUPPORTED OS: Windows Vista and higher
//
// DESCRIPTION:   Turns on/off reverse stereo blit
//
// HOW TO USE:    After the stereo handle for device interface is created via successfull call to appropriate 
//                NvAPI_Stereo_CreateHandleFrom function.
//                After reversed stereo blit control turned on blit from stereo surface will
//                produce right eye image in the left side of the destination surface and left
//                eye image in the right side of the destination surface
//                In DX9 Dst surface has to be created as render target and StretchRect has to be used.
//                Conditions:
//                1. DstWidth == 2*SrcWidth
//                2. DstHeight == SrcHeight
//                3. Src surface is actually stereo surface.
//                4. SrcRect must be {0,0,SrcWidth,SrcHeight}
//                5. DstRect must be {0,0,DstWidth,DstHeight}
//
//                In DX10 ResourceCopyRegion has to be used 
//                Conditions:
//                1. DstWidth == 2*SrcWidth
//                2. DstHeight == SrcHeight
//                3. dstX == 0,
//                4. dstY == 0,
//                5. dstZ == 0,
//                6  SrcBox: left=top=front==0;
//                           right==SrcWidth; bottom==SrcHeight; back==1;
//
// RETURN STATUS:
//                NVAPI_OK - Retrieval of frustum adjust mode was successfull.
//                NVAPI_STEREO_INVALID_DEVICE_INTERFACE - Device interface is not valid. Create again, then attach again.
//                NVAPI_API_NOT_INTIALIZED - NVAPI not initialized.
//                NVAPI_STEREO_NOT_INITIALIZED - Stereo part of NVAPI not initialized.
//                NVAPI_ERROR - Something is wrong (generic error).
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Stereo_ReverseStereoBlitControl(StereoHandle hStereoHandle, NvU8 TurnOn);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_SetNotificationMessage
//
// PARAMETERS:    stereoHandle(IN) - Stereo handle correspondent to device interface.
//                hWnd(IN)         - Window HWND that will be notified when user changed stereo driver state.
//                                   Actual HWND must be cast to an NvU64.
//                messageID(IN)    - MessageID of the message that will be posted to hWnd
//
//  SUPPORTED OS: Windows Vista and higher
//
// DESCRIPTION:   Setup notification message that stereo driver will use to notify application
//                when user changes stereo driver state. 
//                Call this API with NULL hWnd to prohibit notification.
//
//
// HOW TO USE:    After the stereo handle for device interface is created via successfull call to appropriate
//                NvAPI_Stereo_CreateHandleFrom function.
//                
//                When user changes stereo state Activated or Deactivated, separation or conversion
//                stereo driver will post defined message with the folloing parameters
//          
//                wParam == MAKEWPARAM(l, h) where l == 0 if stereo is deactivated
//                                                      1 if stereo is deactivated
//                                                 h  - is current separation. 
//                                                      Actual separation is float(h*100.f/0xFFFF);
//                lParam                           is current conversion. 
//                                                      Actual conversion is *(float*)&lParam
//
// RETURN STATUS:
//                NVAPI_OK - Notification set.
//                NVAPI_STEREO_INVALID_DEVICE_INTERFACE - Device interface is not valid. Create again, then attach again.
//                NVAPI_API_NOT_INTIALIZED - NVAPI not initialized.
//                NVAPI_STEREO_NOT_INITIALIZED - Stereo part of NVAPI not initialized.
//                NVAPI_ERROR - Something is wrong (generic error).
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Stereo_SetNotificationMessage(StereoHandle hStereoHandle, NvU64 hWnd,NvU64 messageID);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_IsWindowedModeSupported
//
// PARAMETERS:    bSupported(OUT)   != 0  - supported, 
//                                  == 0  - is not supported 
//
//  SUPPORTED OS: Windows Vista and higher
//
// DESCRIPTION:   Return availibilty of windowed mode stereo
//
// HOW TO USE:    
//
// RETURN STATUS:
//                NVAPI_OK - Retrieval of frustum adjust mode was successfull.
//                NVAPI_API_NOT_INTIALIZED - NVAPI not initialized.
//                NVAPI_STEREO_NOT_INITIALIZED - Stereo part of NVAPI not initialized.
//                NVAPI_ERROR - Something is wrong (generic error).
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Stereo_IsWindowedModeSupported(NvU8* bSupported);



/////////////////////////////////////////////////////////////////////////
// Video Input Output (VIO) API
/////////////////////////////////////////////////////////////////////////

//  SUPPORTED OS: Windows XP and higher
///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GetTachReading
//
//   DESCRIPTION: This retrieves the tachometer reading for fan speed for the specified physical GPU.
//
//   PARAMETERS:   hPhysicalGpu(IN) - GPU selection.
//                 pValue(OUT)      - Pointer to a variable to get the tachometer reading
//   HOW TO USE:   NvU32 Value = 0;
//                 ret = NvAPI_GPU_GetTachReading(hPhysicalGpu, &Value);  
//                 On call success:
//                 Value would contain the tachometer reading   
//
// RETURN STATUS: 
//    NVAPI_OK - completed request
//    NVAPI_ERROR - miscellaneous error occurred
//    NVAPI_NOT_SUPPORTED - functionality not supported 
//    NVAPI_API_NOT_INTIALIZED - nvapi not initialized
//    NVAPI_INVALID_ARGUMENT - invalid argument passed
//    NVAPI_HANDLE_INVALIDATED - handle passed has been invalidated (see user guide)
//    NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE - handle passed is not a physical GPU handle
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetTachReading(NvPhysicalGpuHandle hPhysicalGPU, NvU32 *pValue);


// 3D profile for one key cinema feature

#define NV_3D_MAX_RANGE         25

typedef enum
{
    NV_3D_PROPERTY_AS         = 1,  // Anti-Aliasing selector
    NV_3D_PROPERTY_AA         = 2,  // Anti-Aliasing setting
    NV_3D_PROPERTY_AF         = 3,  // Anisotropic filtering
    NV_3D_PROPERTY_MAX_FRAMES = 4,  // Maximum pre-rendered frames
    NV_3D_PROPERTY_TEX_FILTER = 5,  // Texture filtering
} NV_3D_PROPERTY;

typedef struct
{
    NvU32           version;                // [in] version
    NV_3D_PROPERTY  settingName;            // [in] 3d setting
    NvU32           flags;                  // [out] indicates whether the settings are read only
    NvU32           defaultValue;           // [out] default base value at driver install
    NvU32           currentValue;           // [in/out] current value or new value to update
} NV_3D_SETTING;

typedef struct
{
    NvU32           version;                // [in] version
    NV_3D_PROPERTY  settingName;            // [in] 3d setting
    NvU32           value[NV_3D_MAX_RANGE]; // [out] range of values as unique array of numbers
    NvU32           validRangeCount;        // [out] number of valid items in the property
} NV_3D_SETTING_RANGE;

#define NV_3D_PROFILE_VERSION           MAKE_NVAPI_VERSION(NV_3D_SETTING,1)
#define NV_3D_PROFILE_RANGE_VERSION     MAKE_NVAPI_VERSION(NV_3D_SETTING_RANGE,1)

/////////////////////////////////////////////////////////////////////////////// 
// 
// FUNCTION NAME:  NvAPI_3D_GetProperty 
// 
// PARAMETERS:     profileName - Which profile to access
//                 p3dSettings - Setting of 3D profile property 
// 
// DESCRIPTION:    This API gets the specific 3D profile property
//
//  SUPPORTED OS: Windows XP and higher
// HOW TO USE:     Set profileName to "Base Profile"
//                 Set 3D property name to settingName of NV_3D_SETTING
//                 On call success:
//                 It returns the specific profile property setting
// 
// RETURN STATUS:  NVAPI_OK - completed request 
//                 NVAPI_ERROR - miscellaneous error occurred
//                 NVAPI_INVALID_ARGUMENT - invalid argument passed
// 
/////////////////////////////////////////////////////////////////////////////// 
NVAPI_INTERFACE NvAPI_3D_GetProperty(NvAPI_ShortString szProfileName, NV_3D_SETTING* p3dSetting);

/////////////////////////////////////////////////////////////////////////////// 
// 
// FUNCTION NAME:  NvAPI_3D_SetProperty 
// 
// PARAMETERS:     profileName - Which profile to access
//                 p3dSettings - Setting of 3D profile property 
// 
// DESCRIPTION:    This API sets the specific 3D profile property
//
//  SUPPORTED OS: Windows XP and higher
// HOW TO USE:     Set profileName to "Base Profile"
//                 Set 3D property name to settingName of NV_3D_SETTING
//                 On call success:
//                 It return the specific profile property setting                  
// 
// RETURN STATUS:  NVAPI_OK - completed request 
//                 NVAPI_ERROR - miscellaneous error occurred
//                 NVAPI_INVALID_ARGUMENT - invalid argument passed
// 
/////////////////////////////////////////////////////////////////////////////// 
NVAPI_INTERFACE NvAPI_3D_SetProperty(NvAPI_ShortString szProfileName, NV_3D_SETTING* p3dSetting);

/////////////////////////////////////////////////////////////////////////////// 
// 
// FUNCTION NAME:  NvAPI_3D_GetPropertyRange 
// 
// PARAMETERS:     profileName - Which profile to access
//                 p3dsettingRange - Setting range of 3D profile property 
// 
// DESCRIPTION:    This API gets property setting range of the specific 3D profile 
//
//  SUPPORTED OS: Windows XP and higher
// HOW TO USE:     Set profileName to "Base Profile"
//                 Set 3D property name to settingName of NV_3D_SETTING_RANGE
//                 On call success:
//                 It returns the specific profile property setting                  
// 
// RETURN STATUS:  NVAPI_OK - completed request 
//                 NVAPI_ERROR - miscellaneous error occurred
//                 NVAPI_INVALID_ARGUMENT - invalid argument passed
// 
/////////////////////////////////////////////////////////////////////////////// 
NVAPI_INTERFACE NvAPI_3D_GetPropertyRange(NvAPI_ShortString szProfileName, NV_3D_SETTING_RANGE* p3dsettingRange);

#ifdef __cplusplus
}; //extern "C" {

#endif

#pragma pack(pop)

#endif // _NVAPI_H

