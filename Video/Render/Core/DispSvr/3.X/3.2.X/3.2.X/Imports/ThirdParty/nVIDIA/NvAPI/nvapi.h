 /***************************************************************************\
|*                                                                           *|
|*      Copyright 2005-2010 NVIDIA Corporation.  All rights reserved.        *|
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
// Date: Jun 11, 2010
// File: nvapi.h
//
// NvAPI provides an interface to NVIDIA devices. This file contains the
// interface constants, structure definitions and function prototypes.
//
//   Target Profile: NDA-developer
//  Target Platform: windows
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

typedef struct _NV_RECT
{
    NvU32    left;
    NvU32    top;
    NvU32    right;
    NvU32    bottom;
} NV_RECT;

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
NV_DECLARE_HANDLE(NvHICHandle);                    // A handle to a Host Interface Card
NV_DECLARE_HANDLE(NvGSyncDeviceHandle);            // A handle to a GSync device
NV_DECLARE_HANDLE(NvVioHandle);                    // A handle to a SDI device
NV_DECLARE_HANDLE(NvTransitionHandle);             // A handle to address a single transition request
NV_DECLARE_HANDLE(NvAudioHandle);                  // Nvidia HD Audio Device
NV_DECLARE_HANDLE(NvIDMHandle);                    // A handle to a Win7 IDM-enabled display

typedef void* StereoHandle;

NV_DECLARE_HANDLE(NvSourceHandle);                 // Unique source handle on the system
NV_DECLARE_HANDLE(NvTargetHandle);                 // Unique target handle on the system


#define NVAPI_DEFAULT_HANDLE        0
#define NV_BIT(x)    (1 << (x))


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

typedef struct
{
    NvU32 data1;
    NvU16 data2;
    NvU16 data3;
    NvU8  data4[8];
} NvGUID, NvLUID;

#define NVAPI_MAX_PHYSICAL_GPUS             64
#define NVAPI_PHYSICAL_GPUS                 32
#define NVAPI_MAX_LOGICAL_GPUS              64
#define NVAPI_MAX_AVAILABLE_GPU_TOPOLOGIES  256
#define NVAPI_MAX_AVAILABLE_SLI_GROUPS      256
#define NVAPI_MAX_GPU_TOPOLOGIES            NVAPI_MAX_PHYSICAL_GPUS
#define NVAPI_MAX_GPU_PER_TOPOLOGY          8
#define NVAPI_MAX_DISPLAY_HEADS             2
#define NVAPI_ADVANCED_DISPLAY_HEADS        4
#define NVAPI_MAX_DISPLAYS                  NVAPI_PHYSICAL_GPUS * NVAPI_ADVANCED_DISPLAY_HEADS
#define NVAPI_MAX_ACPI_IDS                  16
#define NVAPI_MAX_VIEW_MODES                8
#define NV_MAX_HEADS                        4   // Maximum heads, each with NVAPI_DESKTOP_RES resolution
#define NVAPI_MAX_HEADS_PER_GPU             32

#define NV_MAX_HEADS        4   // Maximum heads, each with NVAPI_DESKTOP_RES resolution
#define NV_MAX_VID_STREAMS  4   // Maximum input video streams, each with a NVAPI_VIDEO_SRC_INFO
#define NV_MAX_VID_PROFILES 4   // Maximum output video profiles supported

#define NVAPI_SYSTEM_MAX_DISPLAYS           NVAPI_MAX_PHYSICAL_GPUS * NV_MAX_HEADS

#define NVAPI_SYSTEM_MAX_HWBCS              128
#define NVAPI_SYSTEM_HWBC_INVALID_ID        0xffffffff
#define NVAPI_MAX_AUDIO_DEVICES             16

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
    NVAPI_API_NOT_INITIALIZED                   = -4,      // NvAPI_Initialize has not been called (successfully)
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
    NVAPI_INVALID_USER_PRIVILEGE                = -137,    // Current User is not Admin
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
    NVAPI_ERROR_DRIVER_RELOAD_REQUIRED          = -157,    // A driver reload is required to apply these settings
    NVAPI_SET_NOT_ALLOWED                       = -158,    // Intended Setting is not allowed
    NVAPI_ADVANCED_DISPLAY_TOPOLOGY_REQUIRED    = -159,    // Information can't be returned due to "advanced display topology"
    NVAPI_SETTING_NOT_FOUND                     = -160,    // Setting is not found
    NVAPI_SETTING_SIZE_TOO_LARGE                = -161,    // Setting size is too large
    NVAPI_TOO_MANY_SETTINGS_IN_PROFILE          = -162,    // There are too many settings for a profile
    NVAPI_PROFILE_NOT_FOUND                     = -163,    // Profile is not found
    NVAPI_PROFILE_NAME_IN_USE                   = -164,    // Profile name is duplicated
    NVAPI_PROFILE_NAME_EMPTY                    = -165,    // Profile name is empty
    NVAPI_EXECUTABLE_NOT_FOUND                  = -166,    // Application not found in Profile
    NVAPI_EXECUTABLE_ALREADY_IN_USE             = -167,    // Application already exists in the other profile
    NVAPI_DATATYPE_MISMATCH                     = -168,    // Data Type mismatch
    NVAPI_PROFILE_REMOVED                       = -169,    /// The profile passed as parameter has been removed and is no longer valid.
    NVAPI_UNREGISTERED_RESOURCE                 = -170,    // An unregistered resource was passed as a parameter
    NVAPI_ID_OUT_OF_RANGE                       = -171,    // The DisplayId corresponds to a display which is not within the normal outputId range
    NVAPI_DISPLAYCONFIG_VALIDATION_FAILED       = -172,    // Display topology is not valid so we can't do a mode set on this config
    NVAPI_MOSAIC_NOT_ACTIVE                     = -176,    // The requested action cannot be performed without Mosaic being enabled
    NVAPI_SHARE_RESOURCE_RELOCATED              = -177,    // The surface is relocated away from vidmem
} NvAPI_Status;

#define NVAPI_API_NOT_INTIALIZED        NVAPI_API_NOT_INITIALIZED       // Fix typo in error code
#define NVAPI_INVALID_USER_PRIVILEDGE   NVAPI_INVALID_USER_PRIVILEGE    // Fix typo in error code


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
// FUNCTION NAME: NvAPI_Unload
//
//   DESCRIPTION: Unloads NVAPI library. This must be the last function called.
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
//  !! This is not thread safe. In a multithreaded environment, calling NvAPI_Unload       !!
//  !! while another thread is executing another NvAPI_XXX function, results in           !!
//  !! undefined behaviour and might even cause the application to crash. Developers       !!
//  !! must make sure that they are not in any other function before calling NvAPI_Unload. !!
//
//
//  Unloading NvAPI library is not supported when the library is in a resource locked state.
//  Some functions in the NvAPI library initiates an operation or allocates certain resources
//  and there are corresponding functions available, to complete the operation or free the
//  allocated resources. All such function pairs are designed to prevent unloading NvAPI library.
//
//  For example, if NvAPI_Unload is called after NvAPI_XXX which locks a resource, it fails with
//  NVAPI_ERROR. Developers need to call the corresponding NvAPI_YYY to unlock the resources,
//  before calling NvAPI_Unload again.
//
//
// RETURN STATUS: NVAPI_ERROR            One or more resources are locked and hence cannot unload NVAPI library
//                NVAPI_OK               NVAPI library unloaded
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Unload();

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

//  SUPPORTED OS: Windows XP and higher
///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_SYS_GetDriverAndBranchVersion
//
//   DESCRIPTION: Returns display driver version and driver-branch string.
//
//    PARAMETERS: pDriverVersion(OUT)  - contains driver-version after successful return
//                szBuildBranchString(OUT)  - contains driver-branch string after successful return
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: either pDriverVersion is NULL or enum index too big
//                NVAPI_OK - completed request
//                NVAPI_API_NOT_INTIALIZED - NVAPI not initialized
//                NVAPI_ERROR - miscellaneous error occurred
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_SYS_GetDriverAndBranchVersion(NvU32* pDriverVersion, NvAPI_ShortString szBuildBranchString);


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


#define NVAPI_HDMI_STEREO_MAX_MODES 256

typedef enum
{
    NV_HDMI_STEREO_3D_NONE                = 0x00,
    NV_STEREO_NVISION,
    NV_HDMI_STEREO_3D_FRAME_PACKING,
    NV_HDMI_STEREO_3D_FRAME_PACKING_INT,
    NV_HDMI_STEREO_3D_LINE_ALT,
    NV_HDMI_STEREO_3D_SIDE_BY_SIDE_FULL,
    NV_HDMI_STEREO_3D_SIDE_BY_SIDE_HALF_HORIZONTAIL_ODD_LEFT_ODD_RIGHT,
    NV_HDMI_STEREO_3D_SIDE_BY_SIDE_HALF_HORIZONTAIL_ODD_LEFT_EVEN_RIGHT,
    NV_HDMI_STEREO_3D_SIDE_BY_SIDE_HALF_HORIZONTAIL_EVEN_LEFT_ODD_RIGHT,
    NV_HDMI_STEREO_3D_SIDE_BY_SIDE_HALF_HORIZONTAIL_EVEN_LEFT_EVEN_RIGHT,
    NV_HDMI_STEREO_3D_SIDE_BY_SIDE_HALF_QUINCUX_ODD_LEFT_ODD_RIGHT,
    NV_HDMI_STEREO_3D_SIDE_BY_SIDE_HALF_QUINCUX_ODD_LEFT_EVEN_RIGHT,
    NV_HDMI_STEREO_3D_SIDE_BY_SIDE_HALF_QUINCUX_EVEN_LEFT_ODD_RIGHT,
    NV_HDMI_STEREO_3D_SIDE_BY_SIDE_HALF_QUINCUX_EVEN_LEFT_EVEN_RIGHT,
    NV_HDMI_STEREO_3D_FIELD_ALT,
    NV_HDMI_STEREO_3D_L_DEPTH,
    NV_HDMI_STEREO_3D_L_DEPTH_GFX,
    NV_HDMI_STEREO_3D_ANY                 = 0xff,

} NV_HDMI_STEREO_TYPE;

typedef struct
{
    NvU16 HVisible;                     // horizontal visible (size of single visable surface)
    NvU16 VVisible;                     // vertical visible   (size of single visable surface)
    NvU16 HActive;                      // horizontal active  (active frame size I.E. both right & left surfaces, plus any padding)
    NvU16 VActive ;                     // vertical active    (active frame size I.E. both right & left surfaces, plus any padding)
    NvU16 VActiveSpace[2];              // vertical active space

    NvU16   rr;                         // the refresh rate

    NV_HDMI_STEREO_TYPE stereoType;     // HDMI Stereo type

} NV_HDMI_STEREO_MODE;

typedef struct
{
    // IN
    NvU32    version;             // structure version
    NvU32    displayId;           // (IN)Monitor Identifier. Retrieved from NvAPI_SYS_GetDisplayIdFromGpuAndOutputId
    NvU16    enumIndex;           // (IN)mode index 0 = 1st HDMI stereo mode
    NvU16    count;               // (IN)the max number of modes to return
    NvU32    width;               // (IN)visible desktop width, only required when bMatchDimension is true to do optional resolution filtering
    NvU32    height;              // (IN)visible desktop height, only required when bMatchDimension is true to do optional resolution filtering
    NvU32    refreshRate;         // (IN)desktop refresh rate, only required when bMatchRR is true to do optional refreshrate filtering
    NvU32    bMatchDimension : 1; // (IN)if true, return modes that match specified dimensions (height/width)
    NvU32    bMatchRR : 1;        // (IN)if true, return modes that match specified refresh rates
    NvU32    reserved : 30;       // reserved
    // OUT
    NvU16   numberOfModes;        // (OUT)number of modes returned
    NV_HDMI_STEREO_MODE modeList[NVAPI_HDMI_STEREO_MAX_MODES];    //(OUT) mode list

} NV_HDMI_STEREO_MODES_LIST_V1;

typedef NV_HDMI_STEREO_MODES_LIST_V1 NV_HDMI_STEREO_MODES_LIST;
#define NV_HDMI_STEREO_MODES_LIST_VER1  MAKE_NVAPI_VERSION(NV_HDMI_STEREO_MODES_LIST_V1,1)
#define NV_HDMI_STEREO_MODES_LIST_VER    NV_HDMI_STEREO_MODES_LIST_VER1

///////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME:   NvAPI_DISP_EnumHDMIStereoModes
//
// DESCRIPTION:     This API returns HDMI stereo modes with indices between [enumIndex, enumIndex+count] supported by the specified monitor
//                  numberOfModes returned would be  min(count, no of stereo modes available formonitor).
//  SUPPORTED OS: Windows Vista and higher
//
// PARAMETERS:      pHDMIStereoModes(IN\OUT) - contains displayID, enumIndex as Input and after successful return, it contains HDMI stereo mode list
//
// RETURN STATUS:
//                  NVAPI_OK: completed request
//                  NVAPI_END_ENUMERATION: no more entry
//                  NVAPI_ERROR: miscellaneous error occurred
//                  NVAPI_INVALID_ARGUMENT: Invalid input parameter.
//                  NVAPI_INCOMPATIBLE_STRUCT_VERSION: NV_HDMI_STEREO_MODES_LIST structure version mismatch. see pHDMIStereoModes->version.
//                  NVAPI_ID_OUT_OF_RANGE: Incorrect displayId.
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DISP_EnumHDMIStereoModes(NV_HDMI_STEREO_MODES_LIST *pHDMIStereoModes);


///////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME:   NvAPI_DISP_GetMonitorCapabilities
//
// DESCRIPTION:     This API returns the Monitor capabilities
//
//  SUPPORTED OS: Windows Vista and higher
//
// PARAMETERS:      displayId(IN)                - Monitor Identifier
//                  pMonitorCapabilities(OUT)     - The monitor support info
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_ERROR - miscellaneous error occurred
//                  NVAPI_INVALID_ARGUMENT: Invalid input parameter.
//
///////////////////////////////////////////////////////////////////////////////

typedef enum
{
    // hdmi related caps
    NV_MONITOR_CAPS_TYPE_HDMI_VSDB = 0x1000,
    NV_MONITOR_CAPS_TYPE_HDMI_VCDB = 0x1001
} NV_MONITOR_CAPS_TYPE;

typedef enum
{
    NV_MONITOR_CONN_TYPE_UNINITIALIZED = 0,
    NV_MONITOR_CONN_TYPE_VGA,
    NV_MONITOR_CONN_TYPE_COMPONENT,
    NV_MONITOR_CONN_TYPE_SVIDEO,
    NV_MONITOR_CONN_TYPE_HDMI,
    NV_MONITOR_CONN_TYPE_DVI,
    NV_MONITOR_CONN_TYPE_LVDS,
    NV_MONITOR_CONN_TYPE_DP,
    NV_MONITOR_CONN_TYPE_COMPOSITE,
    NV_MONITOR_CONN_TYPE_UNKNOWN =  -1
} NV_MONITOR_CONN_TYPE;

typedef struct _NV_MONITOR_CAPS_VCDB
{
    NvU8    quantizationRangeYcc         : 1;
    NvU8    quantizationRangeRgb         : 1;
    NvU8    scanInfoPreferredVideoFormat : 2;
    NvU8    scanInfoITVideoFormats       : 2;
    NvU8    scanInfoCEVideoFormats       : 2;
} NV_MONITOR_CAPS_VCDB;

typedef struct _NV_MONITOR_CAPS_VSDB
{
    // byte 1
    NvU8    sourcePhysicalAddressB         : 4;
    NvU8    sourcePhysicalAddressA         : 4;
    // byte 2
    NvU8    sourcePhysicalAddressD         : 4;
    NvU8    sourcePhysicalAddressC         : 4;
    // byte 3
    NvU8    supportDualDviOperation        : 1;
    NvU8    reserved6                      : 2;
    NvU8    supportDeepColorYCbCr444       : 1;
    NvU8    supportDeepColor30bits         : 1;
    NvU8    supportDeepColor36bits         : 1;
    NvU8    supportDeepColor48bits         : 1;
    NvU8    supportAI                      : 1;
    // byte 4
    NvU8    maxTmdsClock;
    // byte 5
    NvU8    cnc0SupportGraphicsTextContent : 1;
    NvU8    cnc1SupportPhotoContent        : 1;
    NvU8    cnc2SupportCinemaContent       : 1;
    NvU8    cnc3SupportGameContent         : 1;
    NvU8    reserved8                      : 1;
    NvU8    hasVicEntries                  : 1;
    NvU8    hasInterlacedLatencyField      : 1;
    NvU8    hasLatencyField                : 1;
    // byte 6
    NvU8    videoLatency;
    // byte 7
    NvU8    audioLatency;
    // byte 8
    NvU8    interlacedVideoLatency;
    // byte 9
    NvU8    interlacedAudioLatency;
    // byte 10
    NvU8    reserved13                     : 7;
    NvU8    has3dEntries                   : 1;
    // byte 11
    NvU8    hdmi3dLength                   : 5;
    NvU8    hdmiVicLength                  : 3;
    // Remaining bytes
    NvU8    hdmi_vic[7];  // Keeping maximum length for 3 bits
    NvU8    hdmi_3d[31];  // Keeping maximum length for 5 bits
} NV_MONITOR_CAPS_VSDB;

typedef struct _NV_MONITOR_CAPABILITIES
{
    NvU32    version;
    NvU16    size;
    NvU32    infoType;
    NvU32    connectorType;        // out: vga, tv, dvi, hdmi, dp
    NvU8     bIsValidInfo : 1;     // boolean : Returns invalid if requested info is not present such as VCDB not present
    union {
        NV_MONITOR_CAPS_VSDB  vsdb;
        NV_MONITOR_CAPS_VCDB  vcdb;
    } data;
} NV_MONITOR_CAPABILITIES;

#define NV_MONITOR_CAPABILITIES_VER   MAKE_NVAPI_VERSION(NV_MONITOR_CAPABILITIES,1)

NVAPI_INTERFACE NvAPI_DISP_GetMonitorCapabilities(NvU32 displayId, NV_MONITOR_CAPABILITIES *pMonitorCapabilities);



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
    NvU8 channelCount         : 3;
    NvU8 rsvd_bits_byte1      : 1;
    NvU8 codingType           : 4;

    // byte 2
    NvU8 sampleSize           : 2;
    NvU8 sampleRate           : 3;
    NvU8 rsvd_bits_byte2      : 3;

    // byte 3
    NvU8 codingExtensionType  : 5;
    NvU8 rsvd_bits_byte3      : 3;

    // byte 4
    NvU8  speakerPlacement;

    // byte 5
    NvU8 lfePlaybackLevel     : 2;
    NvU8 rsvd_bits_byte5      : 1;
    NvU8 levelShift           : 4;
    NvU8 downmixInhibit       : 1;

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
    NvU8 contentTypes            : 2;
    NvU8 yccQuantizationRange    : 2;

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


///////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME:   NvAPI_Disp_InfoFrameControl
//
// DESCRIPTION:     Controls the InfoFrame values
//
//  SUPPORTED OS: Windows Vista and higher
//
// PARAMETERS:      displayId(IN)           - Monitor Identifier
//                  pInfoframeData(IN/OUT)  - Contains data corresponding to InfoFrame
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_ERROR - miscellaneous error occurred
//                  NVAPI_INVALID_ARGUMENT: Invalid input parameter.
//
///////////////////////////////////////////////////////////////////////////////

typedef enum
{
    NV_INFOFRAME_CMD_GET_DEFAULT = 0,     // returns the fields in the infoframe with values set by manufacturer:nvidia/oem
    NV_INFOFRAME_CMD_RESET,               // sets the fields in the infoframe to auto, and infoframe to default infoframe for use in a set
    NV_INFOFRAME_CMD_GET,                 // get the current infoframe state
    NV_INFOFRAME_CMD_SET,                 // set the current infoframe state (flushed to the monitor), the values are one time and does not persist
    NV_INFOFRAME_CMD_GET_OVERRIDE,        // get the override infoframe state, non-override fields will be set to value = AUTO, overridden fields will have the current override values
    NV_INFOFRAME_CMD_SET_OVERRIDE,        // set the override infoframe state, non-override fields will be set to value = AUTO, other values indicate override; persist across modeset/reboot
    NV_INFOFRAME_CMD_GET_PROPERTY,        // get properties associated with infoframe (each of the infoframe type will have properties)
    NV_INFOFRAME_CMD_SET_PROPERTY,        // set properties associated with infoframe
} NV_INFOFRAME_CMD;


typedef enum
{
    NV_INFOFRAME_PROPERTY_MODE_AUTO           = 0, // driver determines whether to send infoframes
    NV_INFOFRAME_PROPERTY_MODE_ENABLE,             // driver always sends infoframe
    NV_INFOFRAME_PROPERTY_MODE_DISABLE,            // driver always don't send infoframe
    NV_INFOFRAME_PROPERTY_MODE_ALLOW_OVERRIDE,     // driver only sends infoframe when client request thru infoframe escape call
} NV_INFOFRAME_PROPERTY_MODE;

// returns whether the current monitor is in blacklist or force this monitor to be in blacklist
typedef enum
{
    NV_INFOFRAME_PROPERTY_BLACKLIST_FALSE = 0,
    NV_INFOFRAME_PROPERTY_BLACKLIST_TRUE,
} NV_INFOFRAME_PROPERTY_BLACKLIST;

typedef struct
{
    NvU32 mode      :  4;
    NvU32 blackList :  2;
    NvU32 reserved  : 10;
    NvU32 version   :  8;
    NvU32 length    :  8;
} NV_INFOFRAME_PROPERTY;

// byte1 related
typedef enum
{
    NV_INFOFRAME_FIELD_VALUE_AVI_SCANINFO_NODATA    = 0,
    NV_INFOFRAME_FIELD_VALUE_AVI_SCANINFO_OVERSCAN,
    NV_INFOFRAME_FIELD_VALUE_AVI_SCANINFO_UNDERSCAN,
    NV_INFOFRAME_FIELD_VALUE_AVI_SCANINFO_FUTURE,
    NV_INFOFRAME_FIELD_VALUE_AVI_SCANINFO_AUTO      = 7
} NV_INFOFRAME_FIELD_VALUE_AVI_SCANINFO;


typedef enum
{
    NV_INFOFRAME_FIELD_VALUE_AVI_BARDATA_NOT_PRESENT         = 0,
    NV_INFOFRAME_FIELD_VALUE_AVI_BARDATA_VERTICAL_PRESENT,
    NV_INFOFRAME_FIELD_VALUE_AVI_BARDATA_HORIZONTAL_PRESENT,
    NV_INFOFRAME_FIELD_VALUE_AVI_BARDATA_BOTH_PRESENT,
    NV_INFOFRAME_FIELD_VALUE_AVI_BARDATA_AUTO                = 7
} NV_INFOFRAME_FIELD_VALUE_AVI_BARDATA;

typedef enum
{
    NV_INFOFRAME_FIELD_VALUE_AVI_AFI_ABSENT   = 0,
    NV_INFOFRAME_FIELD_VALUE_AVI_AFI_PRESENT,
    NV_INFOFRAME_FIELD_VALUE_AVI_AFI_AUTO     = 3
} NV_INFOFRAME_FIELD_VALUE_AVI_ACTIVEFORMATINFO;

typedef enum
{
    NV_INFOFRAME_FIELD_VALUE_AVI_COLORFORMAT_RGB      = 0,
    NV_INFOFRAME_FIELD_VALUE_AVI_COLORFORMAT_YCbCr422,
    NV_INFOFRAME_FIELD_VALUE_AVI_COLORFORMAT_YCbCr444,
    NV_INFOFRAME_FIELD_VALUE_AVI_COLORFORMAT_FUTURE,
    NV_INFOFRAME_FIELD_VALUE_AVI_COLORFORMAT_AUTO     = 7
} NV_INFOFRAME_FIELD_VALUE_AVI_COLORFORMAT;

typedef enum
{
    NV_INFOFRAME_FIELD_VALUE_AVI_F17_FALSE = 0,
    NV_INFOFRAME_FIELD_VALUE_AVI_F17_TRUE,
    NV_INFOFRAME_FIELD_VALUE_AVI_F17_AUTO = 3
} NV_INFOFRAME_FIELD_VALUE_AVI_F17;

// byte2 related
typedef enum
{
    NV_INFOFRAME_FIELD_VALUE_AVI_ASPECTRATIOACTIVEPORTION_NO_AFD           = 0,
    NV_INFOFRAME_FIELD_VALUE_AVI_ASPECTRATIOACTIVEPORTION_RESERVE01,
    NV_INFOFRAME_FIELD_VALUE_AVI_ASPECTRATIOACTIVEPORTION_RESERVE02,
    NV_INFOFRAME_FIELD_VALUE_AVI_ASPECTRATIOACTIVEPORTION_RESERVE03,
    NV_INFOFRAME_FIELD_VALUE_AVI_ASPECTRATIOACTIVEPORTION_LETTERBOX_GT16x9,
    NV_INFOFRAME_FIELD_VALUE_AVI_ASPECTRATIOACTIVEPORTION_RESERVE05,
    NV_INFOFRAME_FIELD_VALUE_AVI_ASPECTRATIOACTIVEPORTION_RESERVE06,
    NV_INFOFRAME_FIELD_VALUE_AVI_ASPECTRATIOACTIVEPORTION_RESERVE07,
    NV_INFOFRAME_FIELD_VALUE_AVI_ASPECTRATIOACTIVEPORTION_EQUAL_CODEDFRAME = 8,
    NV_INFOFRAME_FIELD_VALUE_AVI_ASPECTRATIOACTIVEPORTION_CENTER_4x3,
    NV_INFOFRAME_FIELD_VALUE_AVI_ASPECTRATIOACTIVEPORTION_CENTER_16x9,
    NV_INFOFRAME_FIELD_VALUE_AVI_ASPECTRATIOACTIVEPORTION_CENTER_14x9,
    NV_INFOFRAME_FIELD_VALUE_AVI_ASPECTRATIOACTIVEPORTION_RESERVE12,
    NV_INFOFRAME_FIELD_VALUE_AVI_ASPECTRATIOACTIVEPORTION_4x3_ON_14x9,
    NV_INFOFRAME_FIELD_VALUE_AVI_ASPECTRATIOACTIVEPORTION_16x9_ON_14x9,
    NV_INFOFRAME_FIELD_VALUE_AVI_ASPECTRATIOACTIVEPORTION_16x9_ON_4x3,
    NV_INFOFRAME_FIELD_VALUE_AVI_ASPECTRATIOACTIVEPORTION_AUTO             = 31,
} NV_INFOFRAME_FIELD_VALUE_AVI_ASPECTRATIOACTIVEPORTION;

typedef enum
{
    NV_INFOFRAME_FIELD_VALUE_AVI_ASPECTRATIOCODEDFRAME_NO_DATA = 0,
    NV_INFOFRAME_FIELD_VALUE_AVI_ASPECTRATIOCODEDFRAME_4x3,
    NV_INFOFRAME_FIELD_VALUE_AVI_ASPECTRATIOCODEDFRAME_16x9,
    NV_INFOFRAME_FIELD_VALUE_AVI_ASPECTRATIOCODEDFRAME_FUTURE,
    NV_INFOFRAME_FIELD_VALUE_AVI_ASPECTRATIOCODEDFRAME_AUTO    = 7
} NV_INFOFRAME_FIELD_VALUE_AVI_ASPECTRATIOCODEDFRAME;

typedef enum
{
    NV_INFOFRAME_FIELD_VALUE_AVI_COLORIMETRY_NO_DATA                   = 0,
    NV_INFOFRAME_FIELD_VALUE_AVI_COLORIMETRY_SMPTE_170M,
    NV_INFOFRAME_FIELD_VALUE_AVI_COLORIMETRY_ITUR_BT709,
    NV_INFOFRAME_FIELD_VALUE_AVI_COLORIMETRY_USE_EXTENDED_COLORIMETRY,
    NV_INFOFRAME_FIELD_VALUE_AVI_COLORIMETRY_AUTO                      = 7
} NV_INFOFRAME_FIELD_VALUE_AVI_COLORIMETRY;

// byte 3 related
typedef enum
{
    NV_INFOFRAME_FIELD_VALUE_AVI_NONUNIFORMPICTURESCALING_NO_DATA    = 0,
    NV_INFOFRAME_FIELD_VALUE_AVI_NONUNIFORMPICTURESCALING_HORIZONTAL,
    NV_INFOFRAME_FIELD_VALUE_AVI_NONUNIFORMPICTURESCALING_VERTICAL,
    NV_INFOFRAME_FIELD_VALUE_AVI_NONUNIFORMPICTURESCALING_BOTH,
    NV_INFOFRAME_FIELD_VALUE_AVI_NONUNIFORMPICTURESCALING_AUTO       = 7
} NV_INFOFRAME_FIELD_VALUE_AVI_NONUNIFORMPICTURESCALING;

typedef enum
{
    NV_INFOFRAME_FIELD_VALUE_AVI_RGBQUANTIZATION_DEFAULT       = 0,
    NV_INFOFRAME_FIELD_VALUE_AVI_RGBQUANTIZATION_LIMITED_RANGE,
    NV_INFOFRAME_FIELD_VALUE_AVI_RGBQUANTIZATION_FULL_RANGE,
    NV_INFOFRAME_FIELD_VALUE_AVI_RGBQUANTIZATION_RESERVED,
    NV_INFOFRAME_FIELD_VALUE_AVI_RGBQUANTIZATION_AUTO          = 7
} NV_INFOFRAME_FIELD_VALUE_AVI_RGBQUANTIZATION;

typedef enum
{
    NV_INFOFRAME_FIELD_VALUE_AVI_EXTENDEDCOLORIMETRY_XVYCC601     = 0,
    NV_INFOFRAME_FIELD_VALUE_AVI_EXTENDEDCOLORIMETRY_XVYCC709,
    NV_INFOFRAME_FIELD_VALUE_AVI_EXTENDEDCOLORIMETRY_SYCC601,
    NV_INFOFRAME_FIELD_VALUE_AVI_EXTENDEDCOLORIMETRY_ADOBEYCC601,
    NV_INFOFRAME_FIELD_VALUE_AVI_EXTENDEDCOLORIMETRY_ADOBERGB,
    NV_INFOFRAME_FIELD_VALUE_AVI_EXTENDEDCOLORIMETRY_RESERVED05,
    NV_INFOFRAME_FIELD_VALUE_AVI_EXTENDEDCOLORIMETRY_RESERVED06,
    NV_INFOFRAME_FIELD_VALUE_AVI_EXTENDEDCOLORIMETRY_RESERVED07,
    NV_INFOFRAME_FIELD_VALUE_AVI_EXTENDEDCOLORIMETRY_AUTO         = 15
} NV_INFOFRAME_FIELD_VALUE_AVI_EXTENDEDCOLORIMETRY;

typedef enum
{
    NV_INFOFRAME_FIELD_VALUE_AVI_ITC_VIDEO_CONTENT = 0,
    NV_INFOFRAME_FIELD_VALUE_AVI_ITC_ITCONTENT,
    NV_INFOFRAME_FIELD_VALUE_AVI_ITC_AUTO          = 3
} NV_INFOFRAME_FIELD_VALUE_AVI_ITC;

// byte 4 related
typedef enum
{
    NV_INFOFRAME_FIELD_VALUE_AVI_PIXELREPETITION_NONE = 0,
    NV_INFOFRAME_FIELD_VALUE_AVI_PIXELREPETITION_X02,
    NV_INFOFRAME_FIELD_VALUE_AVI_PIXELREPETITION_X03,
    NV_INFOFRAME_FIELD_VALUE_AVI_PIXELREPETITION_X04,
    NV_INFOFRAME_FIELD_VALUE_AVI_PIXELREPETITION_X05,
    NV_INFOFRAME_FIELD_VALUE_AVI_PIXELREPETITION_X06,
    NV_INFOFRAME_FIELD_VALUE_AVI_PIXELREPETITION_X07,
    NV_INFOFRAME_FIELD_VALUE_AVI_PIXELREPETITION_X08,
    NV_INFOFRAME_FIELD_VALUE_AVI_PIXELREPETITION_X09,
    NV_INFOFRAME_FIELD_VALUE_AVI_PIXELREPETITION_X10,
    NV_INFOFRAME_FIELD_VALUE_AVI_PIXELREPETITION_RESERVED10,
    NV_INFOFRAME_FIELD_VALUE_AVI_PIXELREPETITION_RESERVED11,
    NV_INFOFRAME_FIELD_VALUE_AVI_PIXELREPETITION_RESERVED12,
    NV_INFOFRAME_FIELD_VALUE_AVI_PIXELREPETITION_RESERVED13,
    NV_INFOFRAME_FIELD_VALUE_AVI_PIXELREPETITION_RESERVED14,
    NV_INFOFRAME_FIELD_VALUE_AVI_PIXELREPETITION_RESERVED15,
    NV_INFOFRAME_FIELD_VALUE_AVI_PIXELREPETITION_AUTO         = 31
} NV_INFOFRAME_FIELD_VALUE_AVI_PIXELREPETITION;

typedef enum
{
    NV_INFOFRAME_FIELD_VALUE_AVI_CONTENTTYPE_GRAPHICS = 0,
    NV_INFOFRAME_FIELD_VALUE_AVI_CONTENTTYPE_PHOTO,
    NV_INFOFRAME_FIELD_VALUE_AVI_CONTENTTYPE_CINEMA,
    NV_INFOFRAME_FIELD_VALUE_AVI_CONTENTTYPE_GAME,
    NV_INFOFRAME_FIELD_VALUE_AVI_CONTENTTYPE_AUTO     = 7
} NV_INFOFRAME_FIELD_VALUE_AVI_CONTENTTYPE;

typedef enum
{
    NV_INFOFRAME_FIELD_VALUE_AVI_YCCQUANTIZATION_LIMITED_RANGE = 0,
    NV_INFOFRAME_FIELD_VALUE_AVI_YCCQUANTIZATION_FULL_RANGE,
    NV_INFOFRAME_FIELD_VALUE_AVI_YCCQUANTIZATION_RESERVED02,
    NV_INFOFRAME_FIELD_VALUE_AVI_YCCQUANTIZATION_RESERVED03,
    NV_INFOFRAME_FIELD_VALUE_AVI_YCCQUANTIZATION_AUTO          = 7
} NV_INFOFRAME_FIELD_VALUE_AVI_YCCQUANTIZATION;


// Adding an Auto bit to each field
typedef struct
{
    NvU32 vic                     : 8;
    NvU32 pixelRepeat             : 5;
    NvU32 colorSpace              : 3;
    NvU32 colorimetry             : 3;
    NvU32 extendedColorimetry     : 4;
    NvU32 rgbQuantizationRange    : 3;
    NvU32 yccQuantizationRange    : 3;
    NvU32 itContent               : 2;
    NvU32 contentTypes            : 3;
    NvU32 scanInfo                : 3;
    NvU32 activeFormatInfoPresent : 2;
    NvU32 activeFormatAspectRatio : 5;
    NvU32 picAspectRatio          : 3;
    NvU32 nonuniformScaling       : 3;
    NvU32 barInfo                 : 3;
    NvU32 top_bar                 : 17;
    NvU32 bottom_bar              : 17;
    NvU32 left_bar                : 17;
    NvU32 right_bar               : 17;
    NvU32 Future17                : 2;
    NvU32 Future47                : 2;
} NV_INFOFRAME_VIDEO;

// byte 1 related
typedef enum
{
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELCOUNT_IN_HEADER = 0,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELCOUNT_2,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELCOUNT_3,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELCOUNT_4,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELCOUNT_5,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELCOUNT_6,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELCOUNT_7,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELCOUNT_8,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELCOUNT_AUTO      = 15
} NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELCOUNT;

typedef enum
{
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGTYPE_IN_HEADER                  = 0,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGTYPE_PCM,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGTYPE_AC3,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGTYPE_MPEG1,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGTYPE_MP3,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGTYPE_MPEG2,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGTYPE_AACLC,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGTYPE_DTS,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGTYPE_ATRAC,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGTYPE_DSD,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGTYPE_EAC3,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGTYPE_DTSHD,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGTYPE_MLP,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGTYPE_DST,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGTYPE_WMAPRO,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGTYPE_USE_CODING_EXTENSION_TYPE,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGTYPE_AUTO                      = 31
} NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGTYPE;

// byte 2 related
typedef enum
{
    NV_INFOFRAME_FIELD_VALUE_AUDIO_SAMPLESIZE_IN_HEADER = 0,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_SAMPLESIZE_16BITS,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_SAMPLESIZE_20BITS,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_SAMPLESIZE_24BITS,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_SAMPLESIZE_AUTO      = 7
} NV_INFOFRAME_FIELD_VALUE_AUDIO_SAMPLESIZE;

typedef enum
{
    NV_INFOFRAME_FIELD_VALUE_AUDIO_SAMPLEFREQUENCY_IN_HEADER = 0,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_SAMPLEFREQUENCY_32000HZ,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_SAMPLEFREQUENCY_44100HZ,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_SAMPLEFREQUENCY_48000HZ,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_SAMPLEFREQUENCY_88200KHZ,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_SAMPLEFREQUENCY_96000KHZ,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_SAMPLEFREQUENCY_176400KHZ,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_SAMPLEFREQUENCY_192000KHZ,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_SAMPLEFREQUENCY_AUTO      = 15
} NV_INFOFRAME_FIELD_VALUE_AUDIO_SAMPLEFREQUENCY;

// byte 3 related
typedef enum
{
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGEXTENSIONTYPE_USE_CODING_TYPE = 0,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGEXTENSIONTYPE_HEAAC,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGEXTENSIONTYPE_HEAACV2,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGEXTENSIONTYPE_MPEGSURROUND,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGEXTENSIONTYPE_RESERVE04,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGEXTENSIONTYPE_RESERVE05,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGEXTENSIONTYPE_RESERVE06,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGEXTENSIONTYPE_RESERVE07,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGEXTENSIONTYPE_RESERVE08,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGEXTENSIONTYPE_RESERVE09,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGEXTENSIONTYPE_RESERVE10,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGEXTENSIONTYPE_RESERVE11,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGEXTENSIONTYPE_RESERVE12,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGEXTENSIONTYPE_RESERVE13,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGEXTENSIONTYPE_RESERVE14,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGEXTENSIONTYPE_RESERVE15,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGEXTENSIONTYPE_RESERVE16,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGEXTENSIONTYPE_RESERVE17,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGEXTENSIONTYPE_RESERVE18,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGEXTENSIONTYPE_RESERVE19,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGEXTENSIONTYPE_RESERVE20,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGEXTENSIONTYPE_RESERVE21,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGEXTENSIONTYPE_RESERVE22,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGEXTENSIONTYPE_RESERVE23,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGEXTENSIONTYPE_RESERVE24,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGEXTENSIONTYPE_RESERVE25,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGEXTENSIONTYPE_RESERVE26,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGEXTENSIONTYPE_RESERVE27,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGEXTENSIONTYPE_RESERVE28,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGEXTENSIONTYPE_RESERVE29,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGEXTENSIONTYPE_RESERVE30,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGEXTENSIONTYPE_RESERVE31,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGEXTENSIONTYPE_AUTO           = 63
} NV_INFOFRAME_FIELD_VALUE_AUDIO_CODINGEXTENSIONTYPE;

// byte 4 related
typedef enum
{
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_X_X_X_X_X_X_FR_FL           =0,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_X_X_X_X_X_LFE_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_X_X_X_X_FC_X_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_X_X_X_X_FC_LFE_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_X_X_X_RC_X_X_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_X_X_X_RC_X_LFE_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_X_X_X_RC_FC_X_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_X_X_X_RC_FC_LFE_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_X_X_RR_RL_X_X_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_X_X_RR_RL_X_LFE_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_X_X_RR_RL_FC_X_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_X_X_RR_RL_FC_LFE_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_X_RC_RR_RL_X_X_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_X_RC_RR_RL_X_LFE_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_X_RC_RR_RL_FC_X_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_X_RC_RR_RL_FC_LFE_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_RRC_RLC_RR_RL_X_X_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_RRC_RLC_RR_RL_X_LFE_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_RRC_RLC_RR_RL_FC_X_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_RRC_RLC_RR_RL_FC_LFE_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_FRC_FLC_X_X_X_X_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_FRC_FLC_X_X_X_LFE_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_FRC_FLC_X_X_FC_X_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_FRC_FLC_X_X_FC_LFE_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_FRC_FLC_X_RC_X_X_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_FRC_FLC_X_RC_X_LFE_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_FRC_FLC_X_RC_FC_X_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_FRC_FLC_X_RC_FC_LFE_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_FRC_FLC_RR_RL_X_X_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_FRC_FLC_RR_RL_X_LFE_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_FRC_FLC_RR_RL_FC_X_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_FRC_FLC_RR_RL_FC_LFE_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_X_FCH_RR_RL_FC_X_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_X_FCH_RR_RL_FC_LFE_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_TC_X_RR_RL_FC_X_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_TC_X_RR_RL_FC_LFE_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_FRH_FLH_RR_RL_X_X_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_FRH_FLH_RR_RL_X_LFE_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_FRW_FLW_RR_RL_X_X_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_FRW_FLW_RR_RL_X_LFE_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_TC_RC_RR_RL_FC_X_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_TC_RC_RR_RL_FC_LFE_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_FCH_RC_RR_RL_FC_X_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_FCH_RC_RR_RL_FC_LFE_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_TC_FCH_RR_RL_FC_X_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_TC_FCH_RR_RL_FC_LFE_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_FRH_FLH_RR_RL_FC_X_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_FRH_FLH_RR_RL_FC_LFE_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_FRW_FLW_RR_RL_FC_X_FR_FL,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_FRW_FLW_RR_RL_FC_LFE_FR_FL  = 0X31,
    // all other values should default to auto
    NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION_AUTO                        = 0x1FF
} NV_INFOFRAME_FIELD_VALUE_AUDIO_CHANNELALLOCATION;

// byte 5 related
typedef enum
{
    NV_INFOFRAME_FIELD_VALUE_AUDIO_LFEPLAYBACKLEVEL_NO_DATA    = 0,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_LFEPLAYBACKLEVEL_0DB,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_LFEPLAYBACKLEVEL_PLUS10DB,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_LFEPLAYBACKLEVEL_RESERVED03,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_LFEPLAYBACKLEVEL_AUTO       = 7
} NV_INFOFRAME_FIELD_VALUE_AUDIO_LFEPLAYBACKLEVEL;

typedef enum
{
    NV_INFOFRAME_FIELD_VALUE_AUDIO_LEVELSHIFTVALUES_0DB  = 0,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_LEVELSHIFTVALUES_1DB,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_LEVELSHIFTVALUES_2DB,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_LEVELSHIFTVALUES_3DB,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_LEVELSHIFTVALUES_4DB,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_LEVELSHIFTVALUES_5DB,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_LEVELSHIFTVALUES_6DB,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_LEVELSHIFTVALUES_7DB,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_LEVELSHIFTVALUES_8DB,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_LEVELSHIFTVALUES_9DB,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_LEVELSHIFTVALUES_10DB,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_LEVELSHIFTVALUES_11DB,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_LEVELSHIFTVALUES_12DB,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_LEVELSHIFTVALUES_13DB,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_LEVELSHIFTVALUES_14DB,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_LEVELSHIFTVALUES_15DB,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_LEVELSHIFTVALUES_AUTO = 31
} NV_INFOFRAME_FIELD_VALUE_AUDIO_LEVELSHIFTVALUES;

typedef enum
{
    NV_INFOFRAME_FIELD_VALUE_AUDIO_DOWNMIX_PERMITTED  = 0,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_DOWNMIX_PROHIBITED,
    NV_INFOFRAME_FIELD_VALUE_AUDIO_DOWNMIX_AUTO       = 3
} NV_INFOFRAME_FIELD_VALUE_AUDIO_DOWNMIX;

typedef struct
{
    NvU32 codingType          : 5;
    NvU32 codingExtensionType : 6;
    NvU32 sampleSize          : 3;
    NvU32 sampleRate          : 4;
    NvU32 channelCount        : 4;
    NvU32 speakerPlacement    : 9;
    NvU32 downmixInhibit      : 2;
    NvU32 lfePlaybackLevel    : 3;
    NvU32 levelShift          : 5;
    NvU32 Future12            : 2;
    NvU32 Future2x            : 4;
    NvU32 Future3x            : 4;
    NvU32 Future52            : 2;
    NvU32 Future6             : 9;
    NvU32 Future7             : 9;
    NvU32 Future8             : 9;
    NvU32 Future9             : 9;
    NvU32 Future10            : 9;
} NV_INFOFRAME_AUDIO;

typedef struct
{
    NvU32 version; // version of this structure
    NvU16 size;    // size of this structure
    NvU8  cmd;     // The actions to perform from NV_INFOFRAME_CMD
    NvU8  type;    // type of infoframe

    union
    {
        NV_INFOFRAME_PROPERTY     property;  // This is nvidia specific and corresponds to the property cmds and associated infoframe
        NV_INFOFRAME_AUDIO        audio;
        NV_INFOFRAME_VIDEO        video;
    } infoframe;
} NV_INFOFRAME_DATA;

#define NV_INFOFRAME_DATA_VER   MAKE_NVAPI_VERSION(NV_INFOFRAME_DATA,1)

NVAPI_INTERFACE NvAPI_Disp_InfoFrameControl(NvU32 displayId, NV_INFOFRAME_DATA *pInfoframeData);


///////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME:   NvAPI_Disp_ColorControl
//
// DESCRIPTION:     Controls the Color values
//
//  SUPPORTED OS: Windows Vista and higher
//
// PARAMETERS:      displayId(IN)           - Monitor Identifier
//                  pColorData(IN/OUT)      - Contains data corresponding to color information
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_ERROR - miscellaneous error occurred
//                  NVAPI_INVALID_ARGUMENT: Invalid input parameter.
//
///////////////////////////////////////////////////////////////////////////////

typedef enum
{
    NV_COLOR_CMD_GET                 = 1,
    NV_COLOR_CMD_SET,
    NV_COLOR_CMD_IS_SUPPORTED_COLOR,
    NV_COLOR_CMD_GET_DEFAULT
} NV_COLOR_CMD;

// See Table 14 of CEA-861E.  Not all this is supported by GPU.
typedef enum
{
    NV_COLOR_FORMAT_RGB             = 0,
    NV_COLOR_FORMAT_YUV422,
    NV_COLOR_FORMAT_YUV444,
    NV_COLOR_FORMAT_DEFAULT         = 0xFE,
    NV_COLOR_FORMAT_AUTO            = 0xFF
} NV_COLOR_FORMAT;

typedef enum
{
    NV_COLOR_COLORIMETRY_RGB             = 0,
    NV_COLOR_COLORIMETRY_YCC601,
    NV_COLOR_COLORIMETRY_YCC709,
    NV_COLOR_COLORIMETRY_XVYCC601,
    NV_COLOR_COLORIMETRY_XVYCC709,
    NV_COLOR_COLORIMETRY_SYCC601,
    NV_COLOR_COLORIMETRY_ADOBEYCC601,
    NV_COLOR_COLORIMETRY_ADOBERGB,
    NV_COLOR_COLORIMETRY_DEFAULT         = 0xFE,
    NV_COLOR_COLORIMETRY_AUTO            = 0xFF
} NV_COLOR_COLORIMETRY;

typedef struct
{
    NvU32 version; // version of this structure
    NvU16 size;    // size of this structure
    NvU8  cmd;
    struct
    {
        NvU8  colorFormat;
        NvU8  colorimetry;
    } data;
} NV_COLOR_DATA;

NVAPI_INTERFACE NvAPI_Disp_ColorControl(NvU32 displayId, NV_COLOR_DATA *pColorData);
#define NV_COLOR_DATA_VER   MAKE_NVAPI_VERSION(NV_COLOR_DATA,1)

///////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME:   NvAPI_DISP_GetVirtualModeData
//
// DESCRIPTION:     This API lets the caller get state information related to
//                  virtual mode.  See NvAPI_DISP_OverrideDisplayModeList
//
//  SUPPORTED OS: Windows Vista and higher
//
// PARAMETERS:      displayId(IN)           - Display ID of display to override the
//                                            mode list. Retrieved from
//                                            NvAPI_SYS_GetDisplayIdFromGpuAndOutputId
//                  virtualModeData(IN/OUT) - type of information to retrieve
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_API_NOT_INTIALIZED - NVAPI not initialized
//                  NVAPI_ERROR - miscellaneous error occurred
//                  NVAPI_INVALID_ARGUMENT - Invalid input parameter.
//
///////////////////////////////////////////////////////////////////////////////
typedef enum
{
    NV_VIRTUALMODE_CMD_GET_INFO = 1,
} NV_VIRTUALMODE_CMD;

typedef struct
{
    NvU32 isCapableOfVirtualMode : 1;  // set to 1, if system is capable of supporting virtual mode
    NvU32 isInVirtualMode        : 1;  // set to 1, if system is currently in virtual mode; 0, otherwise
    NvU32 reserved               : 30;
} NV_VIRTUALMODE_INFO;

typedef struct
{
    NvU32               version;
    NV_VIRTUALMODE_CMD  cmd;
    union
    {
        NV_VIRTUALMODE_INFO virtualModeInfo;
    } data;
} NV_VIRTUALMODE_DATA;

#define NV_VIRTUALMODE_DATA_VER   MAKE_NVAPI_VERSION(NV_VIRTUALMODE_DATA,1)

NVAPI_INTERFACE NvAPI_DISP_GetVirtualModeData(NvU32 displayId, NV_VIRTUALMODE_DATA *virtualModeData);

///////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME:   NvAPI_DISP_OverrideDisplayModeList
//
// DESCRIPTION:     This API lets the caller override the Display Mode List.
//
//  SUPPORTED OS: Windows Vista and higher
//
// PARAMETERS:      displayId(IN)    - Display ID of display to override the
//                                     mode list. Retrieved from
//                                     NvAPI_SYS_GetDisplayIdFromGpuAndOutputId
//                  modeCount(IN)    - Number of supplied elements in modeList
//                                     Passing in 0 will disable mode list override
//                  modeList(IN)     - Array of NV_DISPLAY_MODE_INFO elements. Pass
//                                     in NULL with modeCount is 0.
//                  enableOutput(IN) - when set allows monitor output; must be 0
//                                     if modeCount is 0.//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_API_NOT_INTIALIZED - NVAPI not initialized
//                  NVAPI_ERROR - miscellaneous error occurred
//                  NVAPI_INVALID_ARGUMENT - Invalid input parameter.
//
///////////////////////////////////////////////////////////////////////////////

typedef struct _NV_DISPLAY_MODE_INFO
{
    NvU32    version;
    NvU32    width;
    NvU32    height;
    NvU32    depth;
    NvU32    refreshRate1K;         // Refresh rate * 1000
    NvU32    preferred      :  1;   // Set to true if a preferred mode, false for non-preferred
    NvU32    reserved       : 31;
} NV_DISPLAY_MODE_INFO;

#define NV_DISPLAY_MODE_INFO_VER   MAKE_NVAPI_VERSION(NV_DISPLAY_MODE_INFO,1)

NVAPI_INTERFACE NvAPI_DISP_OverrideDisplayModeList(NvU32 displayId, NvU32 modeCount, NV_DISPLAY_MODE_INFO* modeList, NvU32 enableOutput);

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


#if defined(_D3D9_H_) || defined(__d3d10_h__) || defined(__d3d11_h__)

NV_DECLARE_HANDLE(NVDX_ObjectHandle);  // DX Objects
static const NVDX_ObjectHandle NVDX_OBJECT_NONE = 0;

#endif //if defined(_D3D9_H_) || defined(__d3d10_h__) || defined(__d3d11_h__)

#if defined(_D3D9_H_) || defined(__d3d10_h__) || defined(__d3d11_h__)
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

#endif //if defined(_D3D9_H_) || defined(__d3d10_h__) || defined(__d3d11_h__)

#if defined(_D3D9_H_)
///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_D3D9_RegisterResource
//
// DESCRIPTION:     To bind a resource (surface/texture) so that it can be retrieved
//                  internally by nvapi.
//
//  SUPPORTED OS: Windows XP and higher
// INPUT:           pResource   (IN)    surface/texture
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_ERROR - error occurred
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_D3D9_RegisterResource(IDirect3DResource9* pResource);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_D3D9_UnregisterResource
//
// DESCRIPTION:     To unbind a resource (surface/texture) after usage.
//
//  SUPPORTED OS: Windows XP and higher
// INPUT:           pResource   (IN)    surface/texture
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_ERROR - error occurred
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_D3D9_UnregisterResource(IDirect3DResource9* pResource);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_D3D9_AliasSurfaceAsTexture
//
//   DESCRIPTION: Create a texture that is an alias of a surface registered with NvAPI.  The
//                new texture can be bound with IDirect3DDevice9::SetTexture().  Note that the texture must
//                be unbound before drawing to the surface again.
//                Unless the USE_SUPER flag is passed, MSAA surfaces will be resolved before
//                being used as a texture.  MSAA depth buffers are resolved with a point filter,
//                and non-depth MSAA surfaces are resolved with a linear filter.
//
//  SUPPORTED OS: Windows XP and higher
//
//         INPUT:  pDev      (IN)   The D3D device that owns the objects
//                 pSurface  (IN)   Pointer to a surface that has been registered with NvAPI
//                                  to provide a texture alias to
//                 ppTexture (OUT)  Fill with the texture created
//                 dwFlag    (IN)   NVAPI_ALIAS_SURFACE_FLAG to describe how to handle the texture
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_INVALID_POINTER - A null pointer was passed as an argument
//                  NVAPI_INVALID_ARGUMENT - One of the arguments was invalid, probably dwFlag.
//                  NVAPI_UNREGISTERED_RESOURCE - pSurface has not been registered with NvAPI
//                  NVAPI_ERROR - error occurred
//
///////////////////////////////////////////////////////////////////////////////
typedef enum {
    NVAPI_ALIAS_SURFACE_FLAG_NONE                     = 0x00000000,
    NVAPI_ALIAS_SURFACE_FLAG_USE_SUPER                = 0x00000001,  // Use the surface's msaa buffer directly as a texture, rather than resolving. (This is much slower, but potentially has higher quality.)
    NVAPI_ALIAS_SURFACE_FLAG_MASK                     = 0x00000001
} NVAPI_ALIAS_SURFACE_FLAG;

NVAPI_INTERFACE NvAPI_D3D9_AliasSurfaceAsTexture(IDirect3DDevice9* pDev,
                                                 IDirect3DSurface9* pSurface,
                                                 IDirect3DTexture9 **ppTexture,
                                                 DWORD dwFlag);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_D3D9_StretchRectEx
//
// DESCRIPTION:     Copy the contents of the source resource to the destination
//                  resource.  This function can convert
//                  between a wider range of surfaces than
//                  IDirect3DDevice9::StretchRect.  For example, it can copy
//                  from a depth/stencil surface to a texture.
//                  Note that the source and destination resources *must* be registered
//                  with NvAPI before being used with NvAPI_D3D9_StretchRectEx.
//
//  SUPPORTED OS: Windows XP and higher
//
// INPUT:           pDevice        (IN)     The D3D device that owns the objects.
//                  pSourceResource(IN)     Pointer to the source resource.
//                  pSrcRect       (IN)     Defines the rectangle on the source to copy from.  If null, copy from the entire resource.
//                  pDestResource  (IN)     Pointer to the destination resource.
//                  pDstRect       (IN)     Defines the rectangle on the destination to copy to.  If null, copy to the entire resource.
//                  Filter         (IN)     Choose a filtering method: D3DTEXF_NONE, D3DTEXF_POINT, D3DTEXF_LINEAR.
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_INVALID_POINTER - An invalid pointer was passed as an argument (probably NULL)
//                  NVAPI_INVALID_ARGUMENT - One of the arguments was invalid
//                  NVAPI_UNREGISTERED_RESOURCE - a resource was passed in without being registered
//                  NVAPI_ERROR - error occurred
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_D3D9_StretchRectEx(IDirect3DDevice9 * pDevice,
                                         IDirect3DResource9 * pSourceResource,
                                         CONST RECT * pSourceRect,
                                         IDirect3DResource9 * pDestResource,
                                         CONST RECT * pDestRect,
                                         D3DTEXTUREFILTERTYPE Filter);


#endif //if defined(_D3D9_H_)


#if defined(_D3D9_H_) || defined(__d3d10_h__)

///////////////////////////////////////////////////////////////////////////////
// NVAPI Query Types
///////////////////////////////////////////////////////////////////////////////

typedef enum _NVAPI_D3D_QUERY_TYPE
{
    NVAPI_D3D_QUERY_TYPE_RESERVED0       = 0,
    NVAPI_D3D_QUERY_TYPE_RESERVED1       = 1,
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
//  SUPPORTED OS: Windows XP and higher
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
// FUNCTION NAME:   NvAPI_D3D9_CreatePathContextNV
//
//   DESCRIPTION:   This API call is part of the DX implementation of the
//                  NV Path Library.
//                  Creates a new NVPL context.
//
//  SUPPORTED OS: Windows XP and higher
//
//         INPUT:   pDevice - Direct3D 9 device
//
//        OUTPUT:   context - NVPL context
//
// RETURN STATUS: a NvAPI status code or DX HRESULT code
//
///////////////////////////////////////////////////////////////////////////////
//--------------------------------------------------------------------------------------
// NV Path Library data types
//--------------------------------------------------------------------------------------

typedef enum _NV_PATH_DATATYPE
{
    NV_BYTE                     = 0,
    NV_UNSIGNED_BYTE,
    NV_SHORT,
    NV_UNSIGNED_SHORT,
    NV_INT,
    NV_UNSIGNED_INT,
    NV_FLOAT,
    NV_FIXED
} NvPathDataType;

typedef enum _NV_PATH_PARAMTYPE
{
    PATH_QUALITY_NV             = 0x8ED8,
    FILL_RULE_NV                = 0x8ED9,
    STROKE_CAP0_STYLE_NV        = 0x8EE0,
    STROKE_CAP1_STYLE_NV        = 0x8EE1,
    STROKE_CAP2_STYLE_NV        = 0x8EE2,
    STROKE_CAP3_STYLE_NV        = 0x8EE3,
    STROKE_JOIN_STYLE_NV        = 0x8EE8,
    STROKE_MITER_LIMIT_NV       = 0x8EE9
} NVPathParamType;

typedef enum _NV_PATH_FILLRULE
{
    EVEN_ODD_NV                 = 0x8EF0,
    NON_ZERO_NV                 = 0x8EF1
} NVPathFillRule;

typedef enum _NV_PATH_CAPSTYLE
{
    CAP_BUTT_NV                 = 0x8EF4,
    CAP_ROUND_NV                = 0x8EF5,
    CAP_SQUARE_NV               = 0x8EF6,
    CAP_TRIANGLE_NV             = 0x8EF7
} NVPathCapStyle;

typedef enum _NV_PATH_JOINSTYLE
{
    JOIN_MITER_NV               = 0x8EFC,
    JOIN_ROUND_NV               = 0x8EFD,
    JOIN_BEVEL_NV               = 0x8EFE,
    JOIN_CLIPPED_MITER_NV       = 0x8EFF
} NVPathJoinStyle;

typedef enum _NV_PATH_TARGETTYPE
{
    MATRIX_PATH_TO_CLIP_NV      = 0x8F04,
    MATRIX_STROKE_TO_PATH_NV    = 0x8F05,
    MATRIX_PATH_COORD0_NV       = 0x8F08,
    MATRIX_PATH_COORD1_NV       = 0x8F09,
    MATRIX_PATH_COORD2_NV       = 0x8F0A,
    MATRIX_PATH_COORD3_NV       = 0x8F0B
} NVPathTargetType;

typedef enum _NV_PATH_MODE
{
    FILL_PATH_NV                = 0x8F18,
    STROKE_PATH_NV              = 0x8F19
} NVPathMode;

typedef enum _NV_PATH_CMD
{
    MOVE_TO_NV                  = 0x00,
    LINE_TO_NV                  = 0x01,
    QUADRATIC_BEZIER_TO_NV      = 0x02,
    CUBIC_BEZIER_TO_NV          = 0x03,
    START_MARKER_NV             = 0x20,
    CLOSE_NV                    = 0x21,
    STROKE_CAP0_NV              = 0x40,
    STROKE_CAP1_NV              = 0x41,
    STROKE_CAP2_NV              = 0x42,
    STROKE_CAP3_NV              = 0x43,
} NVPathCmd;


NVAPI_INTERFACE NvAPI_D3D9_CreatePathContextNV(IDirect3DDevice9* pDevice, NvU32* context);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_D3D9_DestroyPathContextNV
//
//   DESCRIPTION:   This API call is part of the DX implementation of the
//                  NV Path Library.
//                  Destroys a NVPL context. This function *MUST* be called
//                  on all created path contexts or else memory leaks will
//                  occur.
//
//  SUPPORTED OS: Windows XP and higher
//
//         INPUT:   context - NVPL context
//
// RETURN STATUS: NVAPI_OK or NVAPI_INVALID_ARGUMENT
//
///////////////////////////////////////////////////////////////////////////////

NVAPI_INTERFACE NvAPI_D3D9_DestroyPathContextNV(NvU32 context);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_D3D9_CreatePathNV
//
//   DESCRIPTION:   This API call is part of the DX implementation of the
//                  NV Path Library.
//                  Creates a NVPL path.
//
//  SUPPORTED OS: Windows XP and higher
//
//         INPUT:   context     - NVPL context
//         INPUT:   datatype    - Type of path vertex data
//         INPUT:   numCommands - Number of commands in path
//         INPUT:   commands    - Path command buffer
//
//        OUTPUT:   path        - Path handle
//
// RETURN STATUS: a NvAPI status code or DX HRESULT code
//
///////////////////////////////////////////////////////////////////////////////

NVAPI_INTERFACE NvAPI_D3D9_CreatePathNV(NvU32 context, NvPathDataType datatype, NvU32 numCommands, const NvU8* commands, NvU32* path);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_D3D9_DeletePathNV
//
//   DESCRIPTION:   This API call is part of the DX implementation of the
//                  NV Path Library.
//                  Destroys a NVPL path. This function *MUST* be called
//                  on all created paths or else memory leaks will occur.
//
//  SUPPORTED OS: Windows XP and higher
//
//         INPUT:   path - NVPL path handle
//
// RETURN STATUS: NVAPI_OK or NVAPI_INVALID_ARGUMENT
//
///////////////////////////////////////////////////////////////////////////////

NVAPI_INTERFACE NvAPI_D3D9_DeletePathNV(NvU32 path);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_D3D9_PathVerticesNV
//
//   DESCRIPTION:   This API call is part of the DX implementation of the
//                  NV Path Library.
//                  Sets vertex data for a path.
//
//  SUPPORTED OS: Windows XP and higher
//
//         INPUT:   path - NVPL path handle
//         INPUT:   vertices - vertex data
//
// RETURN STATUS: a NvAPI status code or DX HRESULT code
//
///////////////////////////////////////////////////////////////////////////////

NVAPI_INTERFACE NvAPI_D3D9_PathVerticesNV(NvU32 path, const void* vertices);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_D3D9_PathParameterfNV
//
//   DESCRIPTION:   This API call is part of the DX implementation of the
//                  NV Path Library.
//                  Sets a path rendering parameter of type float
//
//  SUPPORTED OS: Windows XP and higher
//
//         INPUT:   path - NVPL path handle
//         INPUT:   paramType - parameter name
//         INPUT:   param - parameter value
//
// RETURN STATUS: a NvAPI status code or DX HRESULT code
//
///////////////////////////////////////////////////////////////////////////////

NVAPI_INTERFACE NvAPI_D3D9_PathParameterfNV(NvU32 path, NVPathParamType paramType, float param);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_D3D9_PathParameteriNV
//
//   DESCRIPTION:   This API call is part of the DX implementation of the
//                  NV Path Library.
//                  Sets a path rendering parameter of type int
//
//  SUPPORTED OS: Windows XP and higher
//
//         INPUT:   path - NVPL path handle
//         INPUT:   paramType - parameter name
//         INPUT:   param - parameter value
//
// RETURN STATUS: a NvAPI status code or DX HRESULT code
//
///////////////////////////////////////////////////////////////////////////////

NVAPI_INTERFACE NvAPI_D3D9_PathParameteriNV(NvU32 path, NVPathParamType paramType, int param);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_D3D9_PathMatrixNV
//
//   DESCRIPTION:   This API call is part of the DX implementation of the
//                  NV Path Library.
//                  Sets a transformation matrix
//
//  SUPPORTED OS: Windows XP and higher
//
//         INPUT:   context - NVPL context handle
//         INPUT:   target - type of transformation
//         INPUT:   value - matrix values
//
// RETURN STATUS: a NvAPI status code or DX HRESULT code
//
///////////////////////////////////////////////////////////////////////////////

NVAPI_INTERFACE NvAPI_D3D9_PathMatrixNV(NvU32 context, NVPathTargetType paramType, const float* value);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_D3D9_PathDepthNV
//
//   DESCRIPTION:   This API call is part of the DX implementation of the
//                  NV Path Library.
//                  Sets the current path depth
//
//  SUPPORTED OS: Windows XP and higher
//
//         INPUT:   context - NVPL context handle
//         INPUT:   value - depth value
//
// RETURN STATUS: a NvAPI status code or DX HRESULT code
//
///////////////////////////////////////////////////////////////////////////////

NVAPI_INTERFACE NvAPI_D3D9_PathDepthNV(NvU32 context, float value);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_D3D9_PathClearDepthNV
//
//   DESCRIPTION:   This API call is part of the DX implementation of the
//                  NV Path Library.
//                  Clears the depth buffer
//
//  SUPPORTED OS: Windows XP and higher
//
//         INPUT:   context - NVPL context handle
//         INPUT:   value - depth value
//
// RETURN STATUS: a NvAPI status code or DX HRESULT code
//
///////////////////////////////////////////////////////////////////////////////

NVAPI_INTERFACE NvAPI_D3D9_PathClearDepthNV(NvU32 context, float value);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_D3D9_PathEnableDepthTestNV
//
//   DESCRIPTION:   This API call is part of the DX implementation of the
//                  NV Path Library.
//                  Enables / Disables depth testing
//
//  SUPPORTED OS: Windows XP and higher
//
//         INPUT:   context - NVPL context handle
//         INPUT:   enable - enable depth test if true, else disable
//
// RETURN STATUS: a NvAPI status code or DX HRESULT code
//
///////////////////////////////////////////////////////////////////////////////

NVAPI_INTERFACE NvAPI_D3D9_PathEnableDepthTestNV(NvU32 context, bool enable);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_D3D9_PathEnableColorWriteNV
//
//   DESCRIPTION:   This API call is part of the DX implementation of the
//                  NV Path Library.
//                  Enables / Disables color write
//
//  SUPPORTED OS: Windows XP and higher
//
//         INPUT:   context - NVPL context handle
//         INPUT:   enable - enable color write if true, else disable
//
// RETURN STATUS: a NvAPI status code or DX HRESULT code
//
///////////////////////////////////////////////////////////////////////////////

NVAPI_INTERFACE NvAPI_D3D9_PathEnableColorWriteNV(NvU32 context, bool enable);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_D3D9_DrawPathNV
//
//   DESCRIPTION:   This API call is part of the DX implementation of the
//                  NV Path Library.
//                  Draws the path.
//
//  SUPPORTED OS: Windows XP and higher
//
//         INPUT:   path - NVPL path handle
//         INPUT:   mode - path rendering mode
//
// RETURN STATUS: a NvAPI status code or DX HRESULT code
//
///////////////////////////////////////////////////////////////////////////////

NVAPI_INTERFACE NvAPI_D3D9_DrawPathNV(NvU32 path, NVPathMode mode);

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


//  SUPPORTED OS: Windows XP and higher
///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION_NAME: NvAPI_D3D9_VideoSetStereoInfo
//
//   DESCRIPTION: This api specifies the stereo format of a surface, so that the
//                surface could be used for stereo video processing or compositing.
//                In particular, this api could be used to link the left and right
//                views of a decoded picture.
//
//         INPUT: pDev        - The device on which the stereo surface will be used
//                pStereoInfo - The stereo format of the surface
//
// RETURN STATUS: an int which could be an NvAPI status or DX HRESULT code
//
///////////////////////////////////////////////////////////////////////////////

#ifndef NV_STEREO_VIDEO_FORMAT_DEFINE
#define NV_STEREO_VIDEO_FORMAT_DEFINE

typedef enum _NV_STEREO_VIDEO_FORMAT
{
    NV_STEREO_VIDEO_FORMAT_NOT_STEREO         = 0,

    NV_STEREO_VIDEO_FORMAT_SIDE_BY_SIDE_LR    = 1,
    NV_STEREO_VIDEO_FORMAT_SIDE_BY_SIDE_RL    = 2,
    NV_STEREO_VIDEO_FORMAT_TOP_BOTTOM_LR      = 3,
    NV_STEREO_VIDEO_FORMAT_TOP_BOTTOM_RL      = 4,
    NV_STEREO_VIDEO_FORMAT_ROW_INTERLEAVE_LR  = 5,
    NV_STEREO_VIDEO_FORMAT_ROW_INTERLEAVE_RL  = 6,
    NV_STEREO_VIDEO_FORMAT_TWO_FRAMES_LR      = 7,
    NV_STEREO_VIDEO_FORMAT_MONO_PLUS_OFFSET   = 8,

    NV_STEREO_VIDEO_FORMAT_LAST               = 9,
} NV_STEREO_VIDEO_FORMAT;

#endif // NV_STEREO_VIDEO_FORMAT_DEFINE

typedef struct _NV_DX_VIDEO_STEREO_INFO {
    NvU32                     dwVersion;         // Must be NV_DX_VIDEO_STEREO_INFO_VER
    NVDX_ObjectHandle         hSurface;          // The surface whose stereo format is to be set
    NVDX_ObjectHandle         hLinkedSurface;    // The linked surface (must be valid when eFormat==NV_STEREO_VIDEO_FORMAT_TWO_FRAMES_LR)
    NV_STEREO_VIDEO_FORMAT    eFormat;           // Stereo format of the surface
    NvS32                     sViewOffset;       // Signed offset of each view (positive offset indicating left view is shifted left)
    BOOL                      bStereoEnable;     // Whether stereo rendering should be enabled (if FALSE, only left view will be used)
} NV_DX_VIDEO_STEREO_INFO;

#define NV_DX_VIDEO_STEREO_INFO_VER  MAKE_NVAPI_VERSION(NV_DX_VIDEO_STEREO_INFO,1)

NVAPI_INTERFACE NvAPI_D3D9_VideoSetStereoInfo(IDirect3DDevice9 *pDev,
                                              NV_DX_VIDEO_STEREO_INFO *pStereoInfo);


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
// FUNCTION NAME: NvAPI_D3D9_SetSurfaceCreationLayout
//
//   DESCRIPTION: This call will be used primarily for testing VIC for MCP89.
//                This will set up the layout of surfaces that are created through CreateSurface calls
//
//  SUPPORTED OS: Windows Vista and higher
//
//         INPUT: pDev                          The device (display) to present to
//                pSurfaceLayout                The pointer to the surface layout
//                Surface layouts supported right now include Block-Linear (BL),
//                Pitch-Linear (PL) and Tiled (16x16)
// RETURN STATUS: an int which could be an NvAPI status or DX HRESULT code
//
///////////////////////////////////////////////////////////////////////////////
typedef enum _NVAPI_SURFACE_LAYOUT
{
    NVAPI_SURFACE_LAYOUT_BL    = 0,
    NVAPI_SURFACE_LAYOUT_PL    = 1,
    NVAPI_SURFACE_LAYOUT_TILED = 2,
} NVAPI_SURFACE_LAYOUT;

typedef struct _NVAPI_SURFACE_LAYOUT_STRUCT
{
    BOOL                 bEnableLayoutOverride;
    NVAPI_SURFACE_LAYOUT slLayout;
    DWORD                reserved[4];

} NVAPI_SURFACE_LAYOUT_STRUCT;


NVAPI_INTERFACE NvAPI_D3D9_SetSurfaceCreationLayout(IDirect3DDevice9            *pDev,
                                                    NVAPI_SURFACE_LAYOUT_STRUCT *pSurfaceLayout);

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
    NV_CODEC_TYPE_MVC,
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
#define NV_COLOR_CTRL_COLORSPACE_601_BIT    2   // Color space format ITU-R BT.601 supported
#define NV_COLOR_CTRL_COLORSPACE_709_BIT    3   // Color space format ITU-R BT.709 supported
#define NV_COLOR_CTRL_COLORSPACE_RGB_BIT    4   // Color space format RGB supported

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
#define NV_VID_FEATURE_STEREO_VIDEO_BIT             3   // when this bit is set, it means the driver supports stereo VPBlit/VBBlitHD
                                                        // using NvAPI_D3D9_VideoSetStereoInfo
#define NV_PERF_LEVEL_RED_BIT                       0   // when this bit is set, it means the GPU can NOT support HD/BD playback
#define NV_PERF_LEVEL_YELLOW_BIT                    1   // when this bit is set, it means the GPU meets the minimum requirement for HD/BD playback
#define NV_PERF_LEVEL_GREEN_BIT                     2   // when this bit is set, it means the GPU can support HD/BD playback without problem
#define NV_PERF_LEVEL_AERO_BIT                      3   // when this bit is set, it means the GPU can support HD/BD playback with Aero enabled
#define NV_PERF_LEVEL_FRUC_BIT                      4   // when this bit is set, it means the GPU can support FRUC
#define NV_PERF_LEVEL_3DBD_COMPOSITE_BIT            6   // when this bit is set, it means the GPU can support compositing for 3DBD

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

#define NVAPI_VIDEO_CAPS_PACKET_VER       1
#define RENDER_MODE_DWM_BIT               0
#define RENDER_MODE_OVERLAY_BIT           1
#define RENDER_MODE_STEREO_FULLSCREEN_BIT 2

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

//  SUPPORTED OS: Windows XP and higher
///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_D3D9_QueryVideoInfo
//
//   DESCRIPTION: Extensible NvAPI command for mechanism to retrieve various video related information
//                The information may not be specific to the GPU, but will appear like it is from the
//                application's POV. The calling application can various query commands, and fill in the
//                appropriate structure packet (if necessary), for the specified command. The list of
//                queries available to the application and exposed through this command is intended to be
//                customizable so we only need to expose as much as needed.
//
//
//         INPUT: pDev                          The D3D9 device
//                pQueryInfo                    NVAPI_D3D9_QUERY_VIDEO_INFO containing the type of query to
//                                              perform and returned information.
//
// RETURN STATUS: an int which could be an NvAPI status or DX HRESULT code
//
///////////////////////////////////////////////////////////////////////////////

// Query video info commands
typedef enum _NVAPI_D3D9_QUERY_COMMAND
{
    NVAPI_D3D9_QUERY_NULL_CMD                       = 0,    // Null command
    NVAPI_D3D9_QUERY_STEREO_INFO_CMD                = 1,    // Stereo video information
    NVAPI_D3D9_QUERY_COLOR_INFO_CMD                 = 2,    // Color space and range information
} NVAPI_D3D9_QUERY_COMMAND;

// Query stereo input formats
typedef enum _NV_QUERY_STEREO_INFO_FORMAT
{
    NVAPI_QUERY_STEREO_INFO_FORMAT_NOT_STEREO           = 0x00000000,
    NVAPI_QUERY_STEREO_INFO_FORMAT_SIDE_BY_SIDE_LR      = 0x00000001,
    NVAPI_QUERY_STEREO_INFO_FORMAT_SIDE_BY_SIDE_RL      = 0x00000002,
    NVAPI_QUERY_STEREO_INFO_FORMAT_TOP_BOTTOM_LR        = 0x00000004,
    NVAPI_QUERY_STEREO_INFO_FORMAT_TOP_BOTTOM_RL        = 0x00000008,
    NVAPI_QUERY_STEREO_INFO_FORMAT_ROW_INTERLEAVE_LR    = 0x00000010,
    NVAPI_QUERY_STEREO_INFO_FORMAT_ROW_INTERLEAVE_RL    = 0x00000020,
    NVAPI_QUERY_STEREO_INFO_FORMAT_TWO_FRAMES_LR        = 0x00000040,
    NVAPI_QUERY_STEREO_INFO_FORMAT_MONO_PLUS_OFFSET     = 0x00000080,
} NV_QUERY_STEREO_INFO_FORMAT;

#define NVAPI_QUERY_STEREO_INFO_FORMAT_ALL              NVAPI_QUERY_STEREO_INFO_FORMAT_SIDE_BY_SIDE_LR      | \
                                                        NVAPI_QUERY_STEREO_INFO_FORMAT_SIDE_BY_SIDE_RL      | \
                                                        NVAPI_QUERY_STEREO_INFO_FORMAT_TOP_BOTTOM_LR        | \
                                                        NVAPI_QUERY_STEREO_INFO_FORMAT_TOP_BOTTOM_RL        | \
                                                        NVAPI_QUERY_STEREO_INFO_FORMAT_ROW_INTERLEAVE_LR    | \
                                                        NVAPI_QUERY_STEREO_INFO_FORMAT_ROW_INTERLEAVE_RL    | \
                                                        NVAPI_QUERY_STEREO_INFO_FORMAT_TWO_FRAMES_LR        | \
                                                        NVAPI_QUERY_STEREO_INFO_FORMAT_MONO_PLUS_OFFSET

// Query stereo info packet
typedef struct _NVAPI_QUERY_STEREO_INFO
{
    NvU32   dwFormats;                              // (OUT) Mask of supported stereo formats
    NvU32   bIsSupported        : 1;                // (OUT) Whether stereo is supported
    NvU32   bIsEnabled          : 1;                // (OUT) Whether stereo is current enabled
    NvU32   dwReserved1         :30;                // (IN/OUT) Future expansion
    NvU32   dwReserved2[4];                         // (IN/OUT) Future expansion
} NVAPI_QUERY_STEREO_INFO;

// Query color colorspace formats
typedef enum _NVAPI_QUERY_COLOR_INFO_COLORSPACE
{
    NVAPI_QUERY_COLOR_INFO_COLORSPACE_NONE          = 0x00000000,
    NVAPI_QUERY_COLOR_INFO_COLORSPACE_601           = 0x00000001,
    NVAPI_QUERY_COLOR_INFO_COLORSPACE_709           = 0x00000002,
    NVAPI_QUERY_COLOR_INFO_COLORSPACE_RGB           = 0x00000004,
} NVAPI_QUERY_COLOR_INFO_COLORSPACE;

#define NVAPI_QUERY_COLOR_INFO_COLORSPACE_ALL       NVAPI_QUERY_COLOR_INFO_COLORSPACE_601   | \
                                                    NVAPI_QUERY_COLOR_INFO_COLORSPACE_709   | \
                                                    NVAPI_QUERY_COLOR_INFO_COLORSPACE_RGB

// Query color info packet
typedef struct _NVAPI_QUERY_COLOR_INFO
{
    NvU32   dwColorSpaces;                          // (OUT) Mask of supported color spaces
    NvU32   dwReserved[4];                          // (IN/OUT) Future expansion
} NVAPI_QUERY_COLOR_INFO;

// Query video info packet
typedef struct _NVAPI_DX_QUERY_VIDEO_INFO
{
    NvU32                               dwVersion;          // (IN) NVAPI version that matched NV_DX_QUERY_INFO_VER
    NVAPI_D3D9_QUERY_COMMAND            eQueryCommand;      // (IN) Type of query to perform or information requested
    union
    {
        NVAPI_QUERY_STEREO_INFO         stStereoInfo;       // (OUT)
        NVAPI_QUERY_COLOR_INFO          stColorInfo;        // (OUT)
    };
} NVAPI_D3D9_QUERY_VIDEO_INFO;

#define NVAPI_D3D9_QUERY_VIDEO_INFO_VER  MAKE_NVAPI_VERSION(NVAPI_D3D9_QUERY_VIDEO_INFO,1)

NVAPI_INTERFACE NvAPI_D3D9_QueryVideoInfo(IDirect3DDevice9 *pDev,
                                          NVAPI_D3D9_QUERY_VIDEO_INFO *pQueryInfo);

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
    NVAPI_VIDEO_STATE_COMPONENT_2DTO3D          ,   // Converts 2D video to 3D stereo video
    NVAPI_VIDEO_STATE_COMPONENT_3D_ANALYSIS     ,   // Analyzing 3D stereo video
    NVAPI_VIDEO_STATE_COMPONENT_FRC             ,   // Frame Rate Converter
    NVAPI_VIDEO_STATE_COMPONENT_ID_LAST         ,   // All valid components defined before this one
} NVAPI_VIDEO_STATE_COMPONENT_ID;

#define NVAPI_VIDEO_STATE_COMPONENT_DYNAMIC_CONSTRAST  NVAPI_VIDEO_STATE_COMPONENT_DYNAMIC_CONTRAST  // dynamic contrast value. Kept this for backward compatibility

// Algorithms controlling various video components

#define VIDEO_COMP_ALGO_CUSTOM_BASE 64

typedef enum _NVAPI_VIDEO_COMPONENT_ALGORITHM
{
    VIDEO_COMP_ALGO_COLOR_SPACE_601                  = 0,  // Use the ITU-R BT.601 standard in color-space conversion for xxx_COLOR_SPACE component
    VIDEO_COMP_ALGO_COLOR_SPACE_709                  = 1,  // Use the ITU-R BT.709 standard in color-space conversion for xxx_COLOR_SPACE component
    VIDEO_COMP_ALGO_COLOR_SPACE_CUSTOM_04            = VIDEO_COMP_ALGO_CUSTOM_BASE+4, // Use custom color matrix
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
    VIDEO_COMP_ALGO_2DTO3D_CUSTOM_00                 = VIDEO_COMP_ALGO_CUSTOM_BASE+0, // Use custom 2Dto3D conversion
    VIDEO_COMP_ALGO_2DTO3D_CUSTOM_01                 = VIDEO_COMP_ALGO_CUSTOM_BASE+1, // Use custom 2Dto3D conversion
    VIDEO_COMP_ALGO_3D_ANALYSIS_CUSTOM_00            = VIDEO_COMP_ALGO_CUSTOM_BASE+0, // Use custom 3DAnalysis algorithm
    VIDEO_COMP_ALGO_3D_ANALYSIS_CUSTOM_01            = VIDEO_COMP_ALGO_CUSTOM_BASE+1, // Use custom 3DAnalysis algorithm
    VIDEO_COMP_ALGO_FRC_CUSTOM_00                    = VIDEO_COMP_ALGO_CUSTOM_BASE+0, // Use custom frame rate converter
    VIDEO_COMP_ALGO_FRC_CUSTOM_01                    = VIDEO_COMP_ALGO_CUSTOM_BASE+1, // Use custom frame rate converter
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
    NVAPI_VF_MftYUY2Xcode,
    NVAPI_VF_MftDivxXcode,
    NVAPI_VF_MftWmv9Encode,
    NVAPI_VF_DynamicPState,

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
//  SUPPORTED OS: Windows XP and higher
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
    NVAPI_D3D9_DMA_CMD_COLOR_MATRIX = 0x07,             // Setup the color matrix for the transfer
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

#define NVAPI_D3D9_DMA_COLOR_MATRIX_FLAGS_STD_MASK (NVAPI_D3D9_DMA_COLOR_MATRIX_FLAGS_STD_601|NVAPI_D3D9_DMA_COLOR_MATRIX_FLAGS_STD_709|NVAPI_D3D9_DMA_COLOR_MATRIX_FLAGS_STD_240|NVAPI_D3D9_DMA_COLOR_MATRIX_FLAGS_STD_CUSTOM)

#define NVAPI_D3D9_DMA_COLOR_MATRIX_FLAGS_RANGE_MASK (NVAPI_D3D9_DMA_COLOR_MATRIX_FLAGS_RANGE_FULL| NVAPI_D3D9_DMA_COLOR_MATRIX_FLAGS_RANGE_LIM)

// specifies the type of the color space and range
// see NVAPI_D3D9_DMA_CMD_COLOR_MATRIX for more details
typedef enum _NVAPI_D3D9_DMA_COLOR_MATRIX_FLAGS
{
    NVAPI_D3D9_DMA_COLOR_MATRIX_FLAGS_STD_601         = 0x01, // Color Standard: 601
    NVAPI_D3D9_DMA_COLOR_MATRIX_FLAGS_STD_709         = 0x02, // Color Standard: 709
    NVAPI_D3D9_DMA_COLOR_MATRIX_FLAGS_STD_240         = 0x04, // Color Standard: 240
    NVAPI_D3D9_DMA_COLOR_MATRIX_FLAGS_STD_CUSTOM      = 0x08, // Color Standard: Custom
    NVAPI_D3D9_DMA_COLOR_MATRIX_FLAGS_RANGE_FULL      = 0x10, // Color Range: Full [0 .. 255]
    NVAPI_D3D9_DMA_COLOR_MATRIX_FLAGS_RANGE_LIM       = 0x20, // Color Range: Limited [16 .. 235]
} NVAPI_D3D9_DMA_COLOR_MATRIX_FLAGS;

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
// The last reserved word is used to specify the scaling interpolation method
// and whether the source transfer rect has floating pt co-ords.
// Floating point rects and smoothing levels are applied to NV12/IYUV/YV12->ARGB transfers only.
typedef struct _NVAPI_D3D9_DMA_TRANSFER_PARAMS
{
    NVAPI_D3D9_DMA_TRANSFER_DIR     direction;           // direction of the transfer (IN)
    NVDX_ObjectHandle               hSysmemSurface;      // sysmem surface handle (IN)
    RECT*                           pSysmemSurfaceRect;  // sysmem surface rectangle (IN)
    NVDX_ObjectHandle               hD3D9Surface;        // D3D9 surface handle (IN)
    RECT*                           pD3D9SurfaceRect;    // floatSrcRectFlag=0 => (RECT *) D3D9 Surface rectangle with integral co-ords (IN)
                                                         // floatSrcRectFlag=1 => (RECTF *)D3D9 Surface rectangle with float co-ords appended to integral co-ords (IN)
    HANDLE                          hCompletionEvent;    // completion event handle (IN)
    NVAPI_D3D9_DMA_TRANSFER_TYPE    transferType;        // type of the transfer (IN)
    NvU32                           floatSrcRectFlag:1;  // 1=>pD3D9SurfaceRectF has float co-ords appended (IN)
    NvU32                           smoothingLevel:2;    // Type of interpolation for scaling.
                                                         // 0=>Bilinear, 1=>Nearest Neighbor, 2,3=>Reserved for future.
    NvU32                           reserved:17;         // reserved for future expansion (IN / OUT)
    NvU32                           reservedFlagValid:12;// Secret keyword that implies floatSrcRectFlag/smoothingLevel flags are valid (IN)
} NVAPI_D3D9_DMA_TRANSFER_PARAMS;

// parameter data structure for the NVAPI_D3D9_DMA_CMD_COLOR_MATRIX command
// Specifies the flags for the color spaces and color range.
// Currently color Spaces 601 and 709 are available inside the driver. For others the input
// color matrix [cm0-cm11] will be used. It has the following format for a YUV to RGB color conversion
//
//   | cm0  cm1  cm2 |    | y |   | cm9  |    | r |
//   | cm3  cm4  cm5 | * (| u | + | cm10 |) = | g |
//   | cm6  cm7  cm8 |    | v |   | cm11 |    | b |
//  i.e.  MATRIX * (YUV + const) = RGB
//
//  The offsets cm9..cm11 are divided by 255.
//

#define NVAPI_D3D9_DMA_COLOR_MATRIX_SIZE            12
typedef struct _NVAPI_D3D9_DMA_COLOR_MATRIX_PARAMS
{
   NvU32                             colorMatrixFlags;  // NVAPI_D3D9_DMA_COLOR_MATRIX_FLAGS_XX flags to define the color space and range.
   float*                            colorMatrix;       // 4x3 float array containing the matrix (NVAPI_D3D9_DMA_COLOR_MATRIX_SIZE)
} NVAPI_D3D9_DMA_COLOR_MATRIX_PARAMS;


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
        NVAPI_D3D9_DMA_COLOR_MATRIX_PARAMS ColorMatrixParams;
    };
} NVAPI_D3D9_DMA_PARAMS;

#define NVAPI_D3D9_DMA_PARAMS_VER \
    MAKE_NVAPI_VERSION(NVAPI_D3D9_DMA_PARAMS, 1)

NVAPI_INTERFACE NvAPI_D3D9_DMA(IDirect3DDevice9 *pDev,
    NVAPI_D3D9_DMA_PARAMS *pVideoDMAParams);

//  SUPPORTED OS: Windows XP and higher

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION_NAME: NvAPI_D3D9_EnableStereo
//
//   DESCRIPTION:   This API allows an approved application to enable stereo viewing
//                  on an HDMI 1.4 TV.
//
//   RETURN STATUS: NVAPI_OK - completed request
//                  NVAPI_INVALID_ARGUMENT - the vendor ID and/or response is invalid
//                  NVAPI_INCOMPATIBLE_STRUCT_VERSION - the version of
//                      NVAPI_D3D9_ENABLE_STEREO_PARAMS struct not supported
//
///////////////////////////////////////////////////////////////////////////////

// D3D9 Enable Stereo Commands
typedef enum _NVAPI_D3D9_ENABLE_STEREO_COMMANDS
{
    NVAPI_D3D9_ENABLE_STEREO_CMD_CHALLENGE = 0x01,
    NVAPI_D3D9_ENABLE_STEREO_CMD_RESPONSE  = 0x02,
} NVAPI_D3D9_ENABLE_STEREO_COMMANDS;

// D3D9 Enable Stereo Constants
#define NVAPI_D3D9_ENABLE_STEREO_CHALLENGE_SIZE     16
#define NVAPI_D3D9_ENABLE_STEREO_RESPONSE_SIZE      20

//
// parameter data structure for the NVAPI_D3D9_ENABLE_STEREO_CMD_CHALLENGE command
// this command requests a random challenge from driver
// must be invoked before the NVAPI_D3D9_ENABLE_STEREO_CMD_RESPONSE command
//
typedef struct _NVAPI_D3D9_ENABLE_STEREO_CHALLENGE_PARAMS
{
    NvU8 challenge[NVAPI_D3D9_ENABLE_STEREO_CHALLENGE_SIZE];    // random challenge from driver (OUT)
} NVAPI_D3D9_ENABLE_STEREO_CHALLENGE_PARAMS;

//
// parameter data structure for the NVAPI_D3D9_ENABLE_STEREO_CMD_RESPONSE command
//
// the application must possess the following information in order to calculate a valid response:
// - vendorGUID: a unique ID assigned to the application vendor by NVIDIA;
// - vendorKEY:  a secret key issued to the application vendor by NVIDIA;
//
// the response is calculated as HMAC(vendorKEY, (vendorGUID || challenge))
// where HMAC is the Keyed-Hash Message Authentication Code using SHA-1 as the
// underlying hash function, see FIPS Publication 198 for details of the algorithm
//
typedef struct _NVAPI_D3D9_ENABLE_STEREO_RESPONSE_PARAMS
{
    NvGUID vendorGUID;                                          // vendor GUID from app (IN)
    NvU8   response[NVAPI_D3D9_ENABLE_STEREO_RESPONSE_SIZE];    // response from app (IN)
} NVAPI_D3D9_ENABLE_STEREO_RESPONSE_PARAMS;

typedef struct _NVAPI_D3D9_ENABLE_STEREO_PARAMS
{
    NvU32 version;                      // structure version (IN)
    NvU32 dwCommand;                    // command (IN)
    union
    {
        NVAPI_D3D9_ENABLE_STEREO_CHALLENGE_PARAMS   ChallengeParams;
        NVAPI_D3D9_ENABLE_STEREO_RESPONSE_PARAMS    ResponseParams;
    };
} NVAPI_D3D9_ENABLE_STEREO_PARAMS_V1;

typedef NVAPI_D3D9_ENABLE_STEREO_PARAMS_V1      NVAPI_D3D9_ENABLE_STEREO_PARAMS;
#define NVAPI_D3D9_ENABLE_STEREO_PARAMS_VER1    MAKE_NVAPI_VERSION(NVAPI_D3D9_ENABLE_STEREO_PARAMS_V1, 1)
#define NVAPI_D3D9_ENABLE_STEREO_PARAMS_VER     NVAPI_D3D9_ENABLE_STEREO_PARAMS_VER1

NVAPI_INTERFACE NvAPI_D3D9_EnableStereo(IDirect3DDevice9 *pDev,
    NVAPI_D3D9_ENABLE_STEREO_PARAMS *pEnableStereoParams);

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
//                  NVAPI_INVALID_POINTER - An invalid pointer was passed as an argument (probably pDev is NULL)
//                  NVAPI_INVALID_ARGUMENT - One of the arguments was invalid
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
// DESCRIPTION:     This API is deprecated.
//
//  SUPPORTED OS: Windows Vista and higher
//
// RETURN STATUS:   NVAPI_NOT_SUPPORTED - deprecated
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
// FUNCTION NAME:   NvAPI_D3D9_SetTexture
//
// DESCRIPTION:     *WARNING* NvAPI_SetTexture is obsolete and will be phased out.  Please
//                  use NvAPI_D3D9_AliasSurfaceAsTexture(), which will give you a texture
//                  you can use with IDirect3DDevice9::SetTexture().
//
//                  The format/usage is similar to IDirect3DDevice9::SetTexture,
//                  but it would allow you to bind a msaa depthbuffer or render target and
//                  use as a texture by the pshaders.  The surface must be registered with
//                  NvAPI before it can be set as a texture.
//                  When *ppTex == NULL, NvAPI_D3D9_SetTexture will create a new texture
//                  that aliases the underlying, registered surface and bind the texture sampler uStage
//                  to it.  If it is not NULL, it will attempt to set the texture state uStage to the
//                  the texture *ppTex.
//
//  SUPPORTED OS: Windows XP and higher
// INPUT:           pDev          (IN)     The D3D device that owns the objects.
//                  uStage        (IN)     The stage/sampler number
//                  pSurface      (IN)     Pointer to a surface registered with NvAPI to bind (either a depth buffer or render target)
//                  ppTex         (IN/OUT) Pointer to a texture pointer, so that it can return a pointer to a texture that aliases pSurface
//                  dwFlag        (IN)     NVAPI_ALIAS_SURFACE_FLAG to describe how to handle the texture
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_INVALID_POINTER - A null pointer was passed as an argument
//                  NVAPI_INVALID_ARGUMENT - One of the arguments was invalid, probably dwFlag.
//                  NVAPI_UNREGISTERED_RESOURCE - pSurface has not been registered with NvAPI
//                  NVAPI_ERROR - error occurred
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_D3D9_SetTexture(IDirect3DDevice9 *pDev, NvU32 uStage, IDirect3DSurface9 *pSurface, IDirect3DTexture9 **ppTex, DWORD dwFlag);

#endif //defined(_D3D9_H_) && defined(__cplusplus)



typedef enum
{

    // include in NDA for IFR
    NVFBC_FORMAT_ARGB               =      0,      // ARGB
    NVFBC_FORMAT_RGB                =      1,      // RGB packed
    NVFBC_FORMAT_YUV_420            =      2,      // YYYYUV


}NVFBC_BUFFER_FORMAT;




#if defined(_D3D9_H_) && defined(__cplusplus)

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_D3D9_IFR_SetUpTargetBufferToSys
//
//   DESCRIPTION: Ask to the driver to allocate n buffers (up to a maximum of 3).
//                Additional calls to NvAPI_D3D9_IFR_SetUpTargetBufferToSys will free the buffers that were allocated previously with the same call on the d3d device.
//
//         INPUT: pDev       The device to get primary surface on
//                eFormat    The format of the blit
//                dwNBuffers The number of buffers in ppBuffer (max 3).
//                ppBuffer   A pointer to an array of dwNBuffers (max 3) pointers. Virtual memory buffers will be allocated by the driver.
//                ppDiffmap  Reserved
//
// RETURN STATUS: NVAPI_OK if the call succeeded
//
///////////////////////////////////////////////////////////////////////////////

//  SUPPORTED OS: Windows Vista and higher
NVAPI_INTERFACE NvAPI_D3D9_IFR_SetUpTargetBufferToSys(IDirect3DDevice9 *pDev, NVFBC_BUFFER_FORMAT eFormat, DWORD dwNBuffers, unsigned char ** ppBuffer, unsigned char ** ppDiffMap);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_D3D9_IFR_TransferRenderTarget
//
//   DESCRIPTION: Copies of the current rendertarget into the provided system memory buffer.
//
//         INPUT: pDev           The device to get primary surface on
//                pEvent         When not NULL, will receive a handle to an even that the driver will signal upon completion of NvAPI_D3D9_IFR_TransferRenderTarget
//                dwBufferIndex  The index of the buffer that will receive a copy of the rendertarget. This ordinal is between 0 and the number of buffer created by NvAPI_D3D9_IFR_SetUpTargetBufferToSys.
//                dwTargetWidth  When dwTargetWidth and dwTargetHeight are not 0, a bilinear filtered scaling will be done before the blit to system memory.
//                dwTargetHeight When dwTargetWidth and dwTargetHeight are not 0, a bilinear filtered scaling will be done before the blit to system memory.
//
// RETURN STATUS: NVAPI_OK if the call succeeded
//
///////////////////////////////////////////////////////////////////////////////

//  SUPPORTED OS: Windows Vista and higher
NVAPI_INTERFACE NvAPI_D3D9_IFR_TransferRenderTarget(IDirect3DDevice9 *pDev, HANDLE * pEvent, DWORD dwBufferIndex, DWORD dwTargetWidth, DWORD dwTargetHeight);

///////////////////////////////////////////////////////////////////////////////
#endif // defined(_D3D9_H_) && defined(__cplusplus)


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

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_D3D11_SetDepthBoundsTest
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
NVAPI_INTERFACE NvAPI_D3D11_SetDepthBoundsTest(ID3D11Device *pDev,
                                               NvU32 bEnable,
                                               float fMinDepth,
                                               float fMaxDepth);

#endif //defined(__cplusplus) && defined(__d3d11_h__)

//-----------------------------------------------------------------------------
// Private Direct3D11 APIs
//-----------------------------------------------------------------------------
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
//                NVAPI_NOT_SUPPORTED: API call is not supported on current architecture
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
//                NVAPI_NOT_SUPPORTED: API call is not supported on current architecture
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
//                NVAPI_NOT_SUPPORTED: API call is not supported on current architecture
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
//                NVAPI_NOT_SUPPORTED: API call is not supported on current architecture
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetTotalSPCount(NvPhysicalGpuHandle hPhysicalGpu,NvU32 *pCount);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GetGpuCoreCount
//
//   DESCRIPTION: Retrieves the total number of Cores defined for a GPU
//                Returns 0 on architectures that don't define GPU Core's
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: pCount is NULL
//                NVAPI_OK: *pCount is set
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found
//                NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE: hPhysicalGpu was not a physical GPU handle
//                NVAPI_NOT_SUPPORTED: API call is not supported on current architecture
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetGpuCoreCount(NvPhysicalGpuHandle hPhysicalGpu,NvU32 *pCount);

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
} NV_EDID_V1;

typedef struct
{
    NvU32   version;        //structure version
    NvU8    EDID_Data[NV_EDID_DATA_SIZE];
    NvU32   sizeofEDID;
} NV_EDID_V2;

typedef struct
{
    NvU32   version;        //structure version
    NvU8    EDID_Data[NV_EDID_DATA_SIZE];
    NvU32   sizeofEDID;
    NvU32   edidId;     // edidId is an ID which always returned in a monotonically increasing counter.
                       // Across a split-edid read we need to verify that all calls returned the same edidId.
                       // This counter is incremented if we get the updated EDID.
    NvU32   offset;    // which 256byte page of the EDID we want to read. Start at 0.
                       // If the read succeeds with edidSize > NV_EDID_DATA_SIZE
                       // call back again with offset+256 until we have read the entire buffer
} NV_EDID_V3;

typedef NV_EDID_V3    NV_EDID;

#define NV_EDID_VER1    MAKE_NVAPI_VERSION(NV_EDID_V1,1)
#define NV_EDID_VER2    MAKE_NVAPI_VERSION(NV_EDID_V2,2)
#define NV_EDID_VER3    MAKE_NVAPI_VERSION(NV_EDID_V3,3)
#define NV_EDID_VER   NV_EDID_VER3

NVAPI_INTERFACE NvAPI_GPU_GetEDID(NvPhysicalGpuHandle hPhysicalGpu, NvU32 displayOutputId, NV_EDID *pEDID);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_SetEDID
//
//   DESCRIPTION: Sets the EDID data for the specified GPU handle and connection bit mask.
//                displayOutputId should have exactly 1 bit set to indicate a single display.
//                Note:The EDID will be cached across boot session and will be enumerated to the OS in this call.
//                     To remove the EDID set the sizeofEDID to zero.
//                     OS and NVAPI connection status APIs will reflect the newly set or removed EDID dynamically.
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS: NVAPI_INVALID_ARGUMENT: pEDID is NULL; displayOutputId has 0 or > 1 bits set
//                NVAPI_OK: *pEDID data was applied to the requested displayOutputId.
//                NVAPI_NVIDIA_DEVICE_NOT_FOUND: no NVIDIA GPU driving a display was found.
//                NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE: hPhysicalGpu was not a physical GPU handle.
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


typedef enum
{
    NV_GPU_RAM_TYPE_UNKNOWN = 0,
    NV_GPU_RAM_TYPE_SDRAM   = 1,
    NV_GPU_RAM_TYPE_DDR1    = 2,
    NV_GPU_RAM_TYPE_DDR2    = 3,
    NV_GPU_RAM_TYPE_GDDR2   = 4,
    NV_GPU_RAM_TYPE_GDDR3   = 5,
    NV_GPU_RAM_TYPE_GDDR4   = 6,
    NV_GPU_RAM_TYPE_DDR3    = 7,
    NV_GPU_RAM_TYPE_GDDR5   = 8
} NV_GPU_RAM_TYPE;

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GetRamType
//
//   DESCRIPTION: Retrieves the type of VRAM associated with this CPU
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// RETURN STATUS: NVAPI_ERROR or NVAPI_OK
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetRamType(NvPhysicalGpuHandle hPhysicalGpu, NV_GPU_RAM_TYPE *pRamType);

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
#define NVAPI_MAX_GPU_CLOCKS            32
#define NVAPI_MAX_GPU_PUBLIC_CLOCKS     32
#define NVAPI_MAX_GPU_PERF_CLOCKS       32
#define NVAPI_MAX_GPU_PERF_VOLTAGES     16
#define NVAPI_MAX_GPU_PERF_PSTATES      16

//
//  NV_GPU_CLOCK_INFO_DOMAIN_ID is obsolete.  Use NV_GPU_PUBLIC_CLOCK_ID below.
//  Remove after NV_GPU_PUBLIC_CLOCK_ID propogates and all apps are updated.
//
typedef enum _NV_GPU_CLOCK_INFO_DOMAIN_ID
{
    NVAPI_GPU_CLOCK_INFO_DOMAIN_NV      = 0,
    NVAPI_GPU_CLOCK_INFO_DOMAIN_M       = 4,
    NVAPI_GPU_CLOCK_INFO_DOMAIN_HOTCLK  = 7,
    NVAPI_GPU_CLOCK_INFO_DOMAIN_UNDEFINED = NVAPI_MAX_GPU_CLOCKS,
} NV_GPU_CLOCK_INFO_DOMAIN_ID;

//
//  NV_GPU_PERF_CLOCK_DOMAIN_ID is obsolete.  Use NV_GPU_PUBLIC_CLOCK_ID below.
//  Remove after NV_GPU_PUBLIC_CLOCK_ID propogates and all apps are updated.
//
typedef enum _NV_GPU_PERF_CLOCK_DOMAIN_ID
{
    NVAPI_GPU_PERF_CLOCK_DOMAIN_GRAPHICS_CLK  = 0,
    NVAPI_GPU_PERF_CLOCK_DOMAIN_MEMORY_CLK    = 4,
    NVAPI_GPU_PERF_CLOCK_DOMAIN_PROCESSOR_CLK = 7,
} NV_GPU_PERF_CLOCK_DOMAIN_ID;

typedef enum _NV_GPU_PUBLIC_CLOCK_ID
{
    NVAPI_GPU_PUBLIC_CLOCK_GRAPHICS  = 0,
    NVAPI_GPU_PUBLIC_CLOCK_MEMORY    = 4,
    NVAPI_GPU_PUBLIC_CLOCK_PROCESSOR = 7,
    NVAPI_GPU_PUBLIC_CLOCK_UNDEFINED = NVAPI_MAX_GPU_PUBLIC_CLOCKS,
} NV_GPU_PUBLIC_CLOCK_ID;

typedef enum _NV_GPU_PERF_VOLTAGE_INFO_DOMAIN_ID
{
    NVAPI_GPU_PERF_VOLTAGE_INFO_DOMAIN_CORE      = 0,
    NVAPI_GPU_PERF_VOLTAGE_INFO_DOMAIN_UNDEFINED = NVAPI_MAX_GPU_PERF_VOLTAGES,
} NV_GPU_PERF_VOLTAGE_INFO_DOMAIN_ID;


//Performance table overclocking

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
            NV_GPU_PUBLIC_CLOCK_ID      domainId;       //IN/OUT current domain indicator - one of the ids from NV_GPU_PUBLIC_CLOCK_ID
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
// NOTE:            The clocks represented by NV_GPU_PUBLIC_CLOCK_ID are interlocked on some GPU families.
//                  API will fail to set the clocks if the interlocking ratios are violated.
//                  Fermi family : PROCESSOR_CLK = 2 x GRAPHICS_CLK
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
    NVAPI_COOLER_POLICY_NONE                      = 0x00000000,
    NVAPI_COOLER_POLICY_MANUAL                    = 0x00000001, //Manual adjustment of cooler level. Gets applied right away independent of temperature or performance level.
    NVAPI_COOLER_POLICY_PERF                      = 0x00000002, //GPU performance controls the cooler level.
    NVAPI_COOLER_POLICY_TEMPERATURE_DISCRETE      = 0x00000004, //Discrete thermal levels control the cooler level.
    NVAPI_COOLER_POLICY_TEMPERATURE_CONTINUOUS    = 0x00000008, //Cooler level adjusted at continuous thermal levels by HW.
    NVAPI_COOLER_POLICY_DEFAULT                   = 0x00000020, //Default is used to allowe system to chose system's default policy.
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


#define NV_GPU_PERF_INPUT_FLAGS_DEFAULT_SETTINGS         0x00000001

#define NV_GPU_PERF_PSTATES_FLAGS_PERFMON_ENABLED        0x00000001
#define NV_GPU_PERF_PSTATES_FLAGS_DYN_PSTATES_CAPABLE    0x00000002
#define NV_GPU_PERF_PSTATES_FLAGS_DYNAMIC_PSTATE_ENABLED 0x00000004
#define NV_GPU_PERF_PSTATES_FLAGS_MODE_INTERNAL_TEST     0x00000008

#define NV_GPU_PERF_SET_FORCE_PSTATE_FLAGS_ASYNC       0x00000001

#define NV_GPU_PERF_PSTATE_FLAGS_PCIELIMIT_GEN1       0x00000001
#define NV_GPU_PERF_PSTATE_FLAGS_OVERCLOCKED_TRUE     0x00000002
#define NV_GPU_PERF_PSTATE_FLAGS_OVERCLOCKABLE        0x00000004

#define NV_GPU_PERF_PSTATE_CLOCK_FLAGS_OVERCLOCKABLE  0x00000001

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
            NV_GPU_PUBLIC_CLOCK_ID domainId;
            NvU32                  flags;
            NvU32                  freq;

        } clocks[NVAPI_MAX_GPU_PERF_CLOCKS];

    } pstates[NVAPI_MAX_GPU_PERF_PSTATES];

} NV_GPU_PERF_PSTATES_INFO_V1;

typedef struct
{
    NvU32   version;
    NvU32   flags;
    NvU32   numPstates;
    NvU32   numClocks;
    NvU32   numVoltages;
    struct
    {
        NV_GPU_PERF_PSTATE_ID   pstateId;
        NvU32                   flags;
        struct
        {
            NV_GPU_PUBLIC_CLOCK_ID domainId;
            NvU32                  flags;
            NvU32                  freq;

        } clocks[NVAPI_MAX_GPU_PERF_CLOCKS];
        struct
        {
            NV_GPU_PERF_VOLTAGE_INFO_DOMAIN_ID domainId;
            NvU32                       flags;
            NvU32                       mvolt;

        } voltages[NVAPI_MAX_GPU_PERF_VOLTAGES];

    } pstates[NVAPI_MAX_GPU_PERF_PSTATES];

} NV_GPU_PERF_PSTATES_INFO_V2;

typedef  NV_GPU_PERF_PSTATES_INFO_V2 NV_GPU_PERF_PSTATES_INFO;

#define NV_GPU_PERF_PSTATES_INFO_VER1  MAKE_NVAPI_VERSION(NV_GPU_PERF_PSTATES_INFO_V1,1)
#define NV_GPU_PERF_PSTATES_INFO_VER2  MAKE_NVAPI_VERSION(NV_GPU_PERF_PSTATES_INFO_V2,2)

#define NV_GPU_PERF_PSTATES_INFO_VER   NV_GPU_PERF_PSTATES_INFO_VER2

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
//                        bit 2 indicates if the Pstate is overclockable or not
//                    - pstates[i].clocks has valid index range from 0 to numClocks -1
//                    - pstates[i].clocks[j].domainId is the public ID of the clock domain,
//                        containing the following info:
//                      - pstates[i].clocks[j].flags containing the following info:
//                          bit 0 indicates if the clock domain is overclockable or not
//                      - pstates[i].clocks[j].freq is the clock frequency in kHz
//                    - pstates[i].voltages has a valid index range from 0 to numVoltages - 1
//                    - pstates[i].voltages[j].domainId is the ID of the voltage domain,
//                        containing the following info:
//                      - pstates[i].voltages[j].flags is reserved for future use.
//                      - pstates[i].voltages[j].mvolt is the voltage in mV
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
// FUNCTION NAME:   NvAPI_GPU_GetPstatesInfoEx
//
// DESCRIPTION:     Retrieves all performance states (P-States) information. The same as
//                  NvAPI_GPU_GetPstatesInfo(), but supports an input flag for various options.
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
//                  pPerfPstatesInfo(OUT) - P-States information retrieved, as detailed below:
//                  - flags is reserved for future use.
//                  - numPstates is the number of available P-States
//                  - numClocks is the number of clock domains supported by each P-State
//                  - pstates has valid index range from 0 to numPstates - 1
//                  - pstates[i].pstateId is the ID of the P-State,
//                      containing the following info:
//                    - pstates[i].flags containing the following info:
//                        bit 0 indicates if the PCIE limit is GEN1 or GEN2
//                        bit 1 indicates if the Pstate is overclocked or not
//                        bit 2 indicates if the Pstate is overclockable or not
//                    - pstates[i].clocks has valid index range from 0 to numClocks -1
//                    - pstates[i].clocks[j].domainId is the public ID of the clock domain,
//                        containing the following info:
//                      - pstates[i].clocks[j].flags containing the following info:
//                          bit 0 indicates if the clock domain is overclockable or not
//                      - pstates[i].clocks[j].freq is the clock frequency in kHz
//                    - pstates[i].voltages has a valid index range from 0 to numVoltages - 1
//                    - pstates[i].voltages[j].domainId is the ID of the voltage domain,
//                        containing the following info:
//                      - pstates[i].voltages[j].flags is reserved for future use.
//                      - pstates[i].voltages[j].mvolt is the voltage in mV
//                  inputFlags(IN)   - This can be used to select various options:
//                    - if bit 0 is set, pPerfPstatesInfo would contain the default settings
//                        instead of the current, possibily overclocked settings.
//
// RETURN STATUS:
//    NVAPI_OK - completed request
//    NVAPI_ERROR - miscellaneous error occurred
//    NVAPI_HANDLE_INVALIDATED - handle passed has been invalidated (see user guide)
//    NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE - handle passed is not a physical GPU handle
//    NVAPI_INCOMPATIBLE_STRUCT_VERSION - the version of the NV_GPU_PERF_PSTATES struct is not supported
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetPstatesInfoEx(NvPhysicalGpuHandle hPhysicalGpu, NV_GPU_PERF_PSTATES_INFO *pPerfPstatesInfo, NvU32 inputFlags);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_GPU_SetPstatesInfo
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
//                  The changes this API makes are *not* persistent across a driver unload or reboot.
//                  Client applications must handle enforcing persistence.  However, client apps should
//                  also be careful not to persist bad pstate configurations across reboots (i.e. if the
//                  bad pstate configuration hangs the chip and the client application always applies that
//                  configuration on boot, the user will never be able to boot and revert those changes
//                  without booting to safe mode).
//
//                  NOTE: This API was introduced starting with NV_GPU_PERF_PSTATES_INFO structure version
//                  NV_GPU_PERF_PSTATES_INFO_VER2.  This API must be called with structure version
//                  >= NV_GPU_PERF_PSTATES_INFO_VER2.
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// PARAMETERS:      hPhysicalGPU(IN)  - GPU selection.
//                  pPerfPstates(IN) - P-States information retrieved, as detailed below:
//                  - flags is reserved for future use.
//                  - numPstates the number of p-states specified
//                  - numClocks is the number of clock domains specified in each p-state.
//                  - numVoltages is the number of voltage domains specified in each p-State
//                  - pstates has valid index range from 0 to numPstates - 1
//                  - pstates[i].pstateId is the ID of the P-State,
//                      containing the following info:
//                    - pstates[i].flags is reserved for future use
//                    - pstates[i].clocks has valid index range from 0 to numClocks -1
//                    - pstates[i].clocks[j].domainId is the public ID of the clock domain,
//                        containing the following info:
//                      - pstates[i].clocks[j].flags is reserved for future use.
//                      - pstates[i].clocks[j].freq is the clock frequency in kHz
//                    - pstates[i].voltages has a valid index range from 0 to numVoltages - 1
//                    - pstates[i].voltages[j].domainId is the ID of the voltage domain,
//                        containing the following info:
//                      - pstates[i].voltages[j].flags is reserved for future use.
//                      - pstates[i].voltages[j].mvolt is the voltage in mV
//
// RETURN STATUS:
//    NVAPI_OK - completed request
//    NVAPI_ERROR - miscellaneous error occurred
//    NVAPI_HANDLE_INVALIDATED - handle passed has been invalidated (see user guide)
//    NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE - handle passed is not a physical GPU handle
//    NVAPI_INCOMPATIBLE_STRUCT_VERSION - the version of the NV_GPU_PERF_PSTATES struct is not supported
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_SetPstatesInfo(NvPhysicalGpuHandle hPhysicalGpu, NV_GPU_PERF_PSTATES_INFO *pPerfPstatesInfo);

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


#define NVAPI_MAX_GPU_PERF_DOMAIN_VOLTAGES   128

typedef struct
{
    NvU32   version;
    NvU32   flags;
    NvU32   numDomains;

    struct
    {
        NV_GPU_PERF_VOLTAGE_INFO_DOMAIN_ID domainId;
        NvU32                              flags;
        NvU32                              numVoltages;
        struct
        {
            NvU32                       flags;
            NvU32                       mvolt;
        } voltages[NVAPI_MAX_GPU_PERF_DOMAIN_VOLTAGES];
    } domains[NVAPI_MAX_GPU_PERF_VOLTAGES];

} NV_GPU_PERF_VOLTAGES;

#define NV_GPU_PERF_VOLTAGES_VER  MAKE_NVAPI_VERSION(NV_GPU_PERF_VOLTAGES,1)

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_GPU_GetVoltages
//
// DESCRIPTION:     Returns the supported voltage levels for each supported voltage domain on the board.
//                  These are the valid voltage levels which can be used for NvAPI_GPU_SetPstates() and
//                  NvAPI_GPU_SetPstatesInfo().
//
//  SUPPORTED OS: Mac OS X, Windows XP and higher
//
// PARAMETERS:      hPhysicalGPU(IN)  - GPU selection.
//                  pPerfVoltages(OUT) - Voltage information retrieved, as detailed below:
//                  - flags - currently unsupported
//                  - numDomains - the number of voltage domains supported on the board
//                  - domains - valid index range from 0 to numDomains - 1
//                    - domains[i].domainId - the ID of the voltage domain
//                    - domains[i].flags - currently unsupported
//                    - domains[i].numVoltages - number of voltage levels defined for this domain
//                    - domains[i].voltages -  valid index range from 0 to domains[i].numVoltages - 1
//                      - domains[i].voltages[j].flags - currently unsupported
//                      - domains[i].voltages[j].mvolt - voltage in mV
//
// RETURN STATUS:
//    NVAPI_OK - completed request
//    NVAPI_ERROR - miscellaneous error occurred
//    NVAPI_HANDLE_INVALIDATED - handle passed has been invalidated (see user guide)
//    NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE - handle passed is not a physical GPU handle
//    NVAPI_INCOMPATIBLE_STRUCT_VERSION - the version of the NV_GPU_PERF_PSTATES struct is not supported
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetVoltages(NvPhysicalGpuHandle hPhysicalGpu, NV_GPU_PERF_VOLTAGES *pPerfVoltages);


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

} NV_GPU_THERMAL_SETTINGS_V1;

typedef struct
{
    NvU32   version;                //structure version
    NvU32   count;                  //number of associated thermal sensors
    struct
    {
        NV_THERMAL_CONTROLLER       controller;         //internal, ADM1032, MAX6649...
        NvS32                       defaultMinTemp;     //the min default temperature value of the thermal sensor in degrees centigrade
        NvS32                       defaultMaxTemp;     //the max default temperature value of the thermal sensor in degrees centigrade
        NvS32                       currentTemp;        //the current temperature value of the thermal sensor in degrees centigrade
        NV_THERMAL_TARGET           target;             //thermal senor targeted @ GPU, memory, chipset, powersupply, Visual Computing Device...
    } sensor[NVAPI_MAX_THERMAL_SENSORS_PER_GPU];

} NV_GPU_THERMAL_SETTINGS_V2;

typedef NV_GPU_THERMAL_SETTINGS_V2  NV_GPU_THERMAL_SETTINGS;

#define NV_GPU_THERMAL_SETTINGS_VER_1   MAKE_NVAPI_VERSION(NV_GPU_THERMAL_SETTINGS_V1,1)
#define NV_GPU_THERMAL_SETTINGS_VER_2   MAKE_NVAPI_VERSION(NV_GPU_THERMAL_SETTINGS_V2,2)
#define NV_GPU_THERMAL_SETTINGS_VER     NV_GPU_THERMAL_SETTINGS_VER_2

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

    NV_DISPLAY_TV_FORMAT_ANY          = 0x80000000,

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
    NV_SET_GPU_TOPOLOGY_DEFER_DISPLAY_REAPPLY       = 0x00000008,//calling application will ocntrol the display configuration after the settopology returns
} NV_SET_GPU_TOPOLOGY_FLAGS;

// All of these flags are Readonly unless otherwise noted in NvAPI_SetGpuTopologies()
typedef enum
{
    NV_GPU_TOPOLOGY_ACTIVE                          = 0x00000001,// This topology is currently active.
    NV_GPU_TOPOLOGY_VIDLINK_PRESENT                 = 0x00000002,// Video link between all GPUs is present. (physically bridged)
    NV_GPU_TOPOLOGY_MULTIGPU                        = 0x00000004,// This is a "Multi-GPU"-labeled topology.
    NV_GPU_TOPOLOGY_GX2_BOARD                       = 0x00000008,// GPUs comprising this topology are Dagwoods.
    NV_GPU_TOPOLOGY_DYNAMIC_NOT_ALLOWED             = 0x00000010,// Dynamically switching to SLI is not allowed (it requires a reboot)
    NV_GPU_TOPOLOGY_ACTIVE_IMPLICIT                 = 0x00000020,// Implicit Read only SLI is ACTIVE on this topology of gpus. NvAPI_SetHybridMode can be used to disable this topology.
    NV_GPU_TOPOLOGY_ENABLE_SLI_BY_DEFAULT           = 0x00000040,// SLI must be enabled by default, otherwise SLI is optional
    NV_GPU_TOPOLOGY_ENABLE_CORELOGIC_BROADCAST      = 0x00000080,// Broadcast mode is enabled in the corelogic chipset.
    NV_GPU_TOPOLOGY_BROADCAST                       = 0x00000100,// Broadcast mode is enabled
    NV_GPU_TOPOLOGY_UNICAST                         = 0x00000200,// Unicast mode enabled
    NV_GPU_TOPOLOGY_4_WAY_SLI                       = 0x00000400,// This is a "4-Way-SLI"-labeled topology.
    NV_GPU_TOPOLOGY_COMPUTE                         = 0x00010000,// Gpus in this topology are for SLI compute.
                                                                 // All these GPUs of this SLI compute group can be enum'd using NvAPI_GPU_CudaEnumComputeCapableGpus.
    NV_GPU_TOPOLOGY_SLIMULTIMON                     = 0x00020000,// This topology allows multi-display SLI output
    NV_GPU_TOPOLOGY_VIDLINK_CONNECTOR_PRESENT       = 0x00040000,// Vidlink connectors on all GPUs are present (not not necessarily connected)
    NV_GPU_TOPOLOGY_VIEW_CAN_SPAN_GPUS              = 0x00080000,// This topology allows multi-display SLI output across GPUs
    NV_GPU_TOPOLOGY_DRIVER_RELOADING                = 0x00100000,// The display driver for this topology is currently being reloaded
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
} NV_GPU_TOPOLOGY_V1;

#define NV_GPU_TOPOLOGY_VER_1  MAKE_NVAPI_VERSION(NV_GPU_TOPOLOGY_V1,1)

typedef struct
{
    NvU32                   version;                                    //structure version
    NvU32                   gpuCount;                                   //count of GPUs in this topology
    NvPhysicalGpuHandle     hPhysicalGpu[NVAPI_MAX_GPU_PER_TOPOLOGY];   //array of GPU handles
    NvU32                   displayGpuIndex;                            //index of the display GPU owner in the gpu array
    NvU32                   displayOutputTargetMask;                    //target device mask
    NvU32                   noDisplayGpuMask;                           //index mask in the hPhysicalGpu[] array pointing to GPUs that cannot display when SLI is enabled
    NvU32                   flags;                                      //one or more topology flags from NV_GPU_TOPOLOGY_FLAGS
    NvU32                   status;                                     //indicates one of the flags in NV_GPU_TOPOLOGY_STATUS_FLAGS
} NV_GPU_TOPOLOGY;

#define NV_GPU_TOPOLOGY_VER  MAKE_NVAPI_VERSION(NV_GPU_TOPOLOGY,2)

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
    NV_GPU_TOPOLOGY_V1       gpuTopo[NVAPI_MAX_AVAILABLE_GPU_TOPOLOGIES];//max gputopologies
} NV_GPU_TOPOLOGIES_V1;
#define NV_GPU_TOPOLOGIES_VER_1  MAKE_NVAPI_VERSION(NV_GPU_TOPOLOGIES_V1,1)

typedef struct
{
    NvU32                   version;                                    //structure version
    NvU32                   gpuTopoCount;                               //count of valid topologies
    NV_GPU_TOPOLOGY_V1      gpuTopo[NVAPI_MAX_AVAILABLE_GPU_TOPOLOGIES];//max gputopologies
    NvU32                   sliGroupCount;                              //count of valid SLI groups
    NV_SLI_GROUP            sliGroup[NVAPI_MAX_AVAILABLE_SLI_GROUPS];   //max SLI groups
} NV_GPU_TOPOLOGIES_V2;
#define NV_GPU_TOPOLOGIES_VER_2  MAKE_NVAPI_VERSION(NV_GPU_TOPOLOGIES_V2,2)

typedef struct
{
    NvU32                   version;                                    //structure version
    NvU32                   gpuTopoCount;                               //count of valid topologies
    NV_GPU_TOPOLOGY         gpuTopo[NVAPI_MAX_AVAILABLE_GPU_TOPOLOGIES];//max gputopologies
    NvU32                   sliGroupCount;                              //count of valid SLI groups
    NV_SLI_GROUP            sliGroup[NVAPI_MAX_AVAILABLE_SLI_GROUPS];   //max SLI groups
} NV_GPU_TOPOLOGIES;
#define NV_GPU_TOPOLOGIES_VER   MAKE_NVAPI_VERSION(NV_GPU_TOPOLOGIES,3)

#define NV_GPU_VALID_GPU_TOPOLOGIES NV_GPU_TOPOLOGIES
#define NV_GPU_INVALID_GPU_TOPOLOGIES NV_GPU_TOPOLOGIES
#define NV_GPU_VALID_GPU_TOPOLOGIES_VER  NV_GPU_TOPOLOGIES_VER
#define NV_GPU_INVALID_GPU_TOPOLOGIES_VER  NV_GPU_TOPOLOGIES_VER
#define NV_GPU_VALID_GPU_TOPOLOGIES_V1 NV_GPU_TOPOLOGIES_V1
#define NV_GPU_INVALID_GPU_TOPOLOGIES_V1 NV_GPU_TOPOLOGIES_V1
#define NV_GPU_VALID_GPU_TOPOLOGIES_VER_1  NV_GPU_TOPOLOGIES_VER_1
#define NV_GPU_INVALID_GPU_TOPOLOGIES_VER_1  NV_GPU_TOPOLOGIES_VER_1
#define NV_GPU_VALID_GPU_TOPOLOGIES_V2 NV_GPU_TOPOLOGIES_V2
#define NV_GPU_INVALID_GPU_TOPOLOGIES_V2 NV_GPU_TOPOLOGIES_V2
#define NV_GPU_VALID_GPU_TOPOLOGIES_VER_2  NV_GPU_TOPOLOGIES_VER_2
#define NV_GPU_INVALID_GPU_TOPOLOGIES_VER_2  NV_GPU_TOPOLOGIES_VER_2

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
//                  NVAPI_ERROR: check the status returned in pTopology->gpuTopo[].status.
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

typedef struct
{
    NvU32      version;        //structure version
    struct{
        NvU32   displayMask;  // This field name should be outputid and will have only one bit set
    } input;
    struct{
        NvU32   isDP2DVI:1;
        NvU32   isDP2HDMI:1;
        NvU32   reserved : 30;
    } output;

} NV_NVAPI_GET_DP_DONGLE_INFO;
#define NV_NVAPI_GET_DP_DONGLE_INFO_VER  MAKE_NVAPI_VERSION(NV_NVAPI_GET_DP_DONGLE_INFO,1)
///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_Get_DisplayPort_DongleInfo
//
// DESCRIPTION: Gets the DisplayPort Dongle info like DP2DVI/DP2HDMI.
//
//
//  SUPPORTED OS: Windows XP and higher
//
// PARAMETERS:     hPhysicalGPU(IN) - GPU selection.
//                 NV_NVAPI_GET_DP_DONGLE_INFO - data input/output structure
//

// RETURN STATUS:
//    NVAPI_OK - completed request
//    NVAPI_ERROR - miscellaneous error occurred
//    NVAPI_INVALID_ARGUMENT - one or more args are invalid
//    NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE - handle passed is not a physical GPU handle
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_Get_DisplayPort_DongleInfo(NvPhysicalGpuHandle hPhysicalGpu, NV_NVAPI_GET_DP_DONGLE_INFO *pDongleInfo);

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
    NvU32                   i2cSpeed;       //The target speed of the transaction (between 28kbps to 40kbps; not guaranteed)
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
#define NVAPI_ADVANCED_MAX_VIEW_TARGET 4

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
    NV_ROTATE_270         = 3,
    NV_ROTATE_IGNORED     = 4,
} NV_ROTATE;

// Color formats
#define NVFORMAT_MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
                         ((NvU32)(NvU8)(ch0) | ((NvU32)(NvU8)(ch1) << 8) |   \
                     ((NvU32)(NvU8)(ch2) << 16) | ((NvU32)(NvU8)(ch3) << 24 ))

typedef enum _NV_FORMAT
{
    NV_FORMAT_UNKNOWN           =  0,       // unknown. Driver will choose one as following value.
    NV_FORMAT_P8                = 41,       // for 8bpp mode
    NV_FORMAT_R5G6B5            = 23,       // for 16bpp mode
    NV_FORMAT_A8R8G8B8          = 21,       // for 32bpp mode
    NV_FORMAT_A16B16G16R16F     = 113,      // for 64bpp(floating point) mode.

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
//
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
// Make sure to keep this in sync with NVL_TIMING_OVERRIDE from drivers/common/inc/nvlEscDef.h
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
    NV_TIMING_OVERRIDE_NV_PSF                = NV_TIMING_OVERRIDE_NV_PREDEFINED,

    NV_TIMING_OVERRIDE_NV_ASPR,
    NV_TIMING_OVERRIDE_SDI,                  // Override for SDI timing

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
    NvU32                   forceModeEnum     :1;    // Used only on Win7 and higher during a call to NvAPI_SetDisplaySettings. Requests a mdoeset after forced mode enumaration

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

    NvU32 isSourceUnderscanRecommended : 1;  // indicate if source underscan is recommended under the current display context
    NvU32 isScalerUnderscanRecommended : 1;  // indicate if the scaler based underscan is recommended under the current display context
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
                                                    // If this feature is enabled then, CE timings will only be used for HDMI displays
    NvU32      reservedOEM            : 1;          // Reserved bit for OEM configuration
    NvU32      isDriverCtrlCEoverHDMI : 1;          // if == 1, the feature "Force CE timing over HDMI" is dynamically controlled by the driver so
                                                    // it's selectively enabled/disabled based on internal driver policy.
                                                    // if == 0, this feature is statically controlled by app
    NvU32      reserved               : 28;

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
//
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
//
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

#define NVAPI_ADVANCED_MAX_DISPLAY_PATH  NVAPI_ADVANCED_MAX_VIEW_TARGET

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
    NvU64 reserved              : 14;       // [63:50] To fill up qword
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
    NvU64 reserved              : 21;       // [63:43] To fill up qword
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


#define NVAPI_DPCD_MAX_DATA_SIZE 16

typedef enum _NV_DPCD_CMD
{
    NV_DP_AUXCH_CMD_REQ_TYPE_WRITE = 0,             // DP Aux Channel write
    NV_DP_AUXCH_CMD_REQ_TYPE_READ,                  // DP Aux Channel read
    NV_DP_AUXCH_CMD_REQ_TYPE_I2C_WRITE,             // I2C write request
    NV_DP_AUXCH_CMD_REQ_TYPE_I2C_READ,              // I2C read request
    NV_DP_AUXCH_CMD_REQ_TYPE_I2C_WRITE_STATUS,      // I2C write status request
    NV_DP_AUXCH_CMD_REQ_TYPE_MOT_I2C_WRITE,         // I2C write request in middle of transaction
    NV_DP_AUXCH_CMD_REQ_TYPE_MOT_I2C_READ           // I2C read request in middle of transaction

} NV_DPCD_CMD;

typedef enum _NV_DPCD_REPLY_TYPE
{
    NV_DP_AUXCH_REPLYTYPE_ACK = 0,                  // For write: transaction completed and all data bytes written.
                                                    // For read: ACK indicates it is ready for another read request.
    NV_DP_AUXCH_REPLYTYPE_NACK,                     // For write: first return size bytes have been written.
                                                    // For read: implies does not have requested data for the read request transaction.
    NV_DP_AUXCH_REPLYTYPE_DEFER,                    // Not ready for the write/read request and client should retry later.

    NV_DP_AUXCH_REPLYTYPE_TIMEOUT = 0xFF            // The receiver did not respond within the timeout period defined in the DisplayPort 1.1a specification.
} NV_DPCD_REPLY_TYPE;

typedef struct _NV_DPCD_PARAMS
{
    NvU32                version;                        // Structure version
    NvU32                displayId;                      // displayId of DP
    NV_DPCD_CMD          cmd;                            // NV_DPCD_CMD
    NvU32                addr;                           // addr for read/write
    NvU8                 data[NVAPI_DPCD_MAX_DATA_SIZE]; // data for the read/write cmd
    NvU32                size;                           // number of bytes to read/write. The input size value should be indexed from 0. On return
                                                         // this parameter returns total number of data bytes successfully read/written, indexed from 1
    NV_DPCD_REPLY_TYPE   replyType;                      // NV_DPCD_REPLY_TYPE
} NV_DPCD_PARAMS;

#define NV_DPCD_PARAMS_VER  MAKE_NVAPI_VERSION(NV_DPCD_PARAMS,1)
///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Disp_DpAuxChannelControl
//
// DESCRIPTION:   Read/write to DP Aux Channel
//                The DP display needs to be active to make this NvAPI call.
//
// PARAMETERS:    hNvDisplay (IN)      - Display handle associated with the
//                                       attached DP monitor
//                pDpcdParams (IN/OUT) - pointer to NV_DPCD_PARAMS structure
//                                       for input/output parameters
//                size (IN)            - length of the pDpcdParams struct
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS: NVAPI_ERROR or NVAPI_OK or NVAPI_NOT_SUPPORTED or
//                NVAPI_INVALID_ARGUMENT or NVAPI_API_NOT_INTIALIZED
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Disp_DpAuxChannelControl(NvDisplayHandle hNvDisplay, NV_DPCD_PARAMS *pDpcdParams, NvU32 size);


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
    NV_HYBRID_MODE_ADAPTIVE             = 0x07, // This mode is only applicable for GetSwitchSettings.
                                                // SetHybridMode and GetHybridMode have no support of this mode.
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

typedef enum _NV_HYBRID_SWITCH_METHOD
{
    // This is from hybrid spec 3.0 DSM function HYBRIDCAPS bits 28:27
    NV_HYBRID_SWITCH_METHOD_NONE                    = 0x00, // None of physical selector available
                                                            // (no hotkey, no physical button/switch, etc)
    NV_HYBRID_SWITCH_METHOD_UNKNOWN                 = 0x01, // Uses unspecified means to pass required state via
                                                            // POLICYSELECT, or other API
    NV_HYBRID_SWITCH_METHOD_HOTKEY                  = 0x02,
    NV_HYBRID_SWITCH_METHOD_PHYSICAL_SWITCH         = 0x03,
} NV_HYBRID_SWITCH_METHOD;

typedef struct
{
    NV_HYBRID_SWITCH_METHOD method;
    NvU32                   numberOfWays;
} NV_HYBRID_TRANSITION_METHOD;

///////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME:   NvAPI_Coproc_GetCoprocStatus
//
// DESCRIPTION:     This API queries the the copoc status and state.
//                  Supported on Vista and higher.
//
//  SUPPORTED OS: Windows Vista and higher
//
// PARAMETERS:      pCoprocStatus (IN/OUT)   - the coproc status
//                  pCoprocState (IN/OUT)    - the coproc state
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_NOT_SUPPORTED - This feature is supported on Windows XP
//                  NVAPI_API_NOT_INTIALIZED - You must call NvAPI_Initialize first
//                  NVAPI_ERROR - miscellaneous error occurred
//                  NVAPI_INVALID_ARGUMENT - Invalid input parameter
//
///////////////////////////////////////////////////////////////////////////////
typedef enum
{
    NV_COPROC_STATUS_OK                        = 0x00000000,
    NV_COPROC_STATUS_CHIPSET_NOT_SUPPORTED     = 0x00000001,
    NV_COPROC_STATUS_MGPU_NOT_SUPPORTED        = 0x00000002,
    NV_COPROC_STATUS_DGPU_NOT_SUPPORTED        = 0x00000004,
    NV_COPROC_STATUS_INVALID_GPU_COUNT         = 0x00000008,
    NV_COPROC_STATUS_DISABLED_BY_REG_KEY       = 0x00000010,
    NV_COPROC_STATUS_DGPU_POSTING_DEVICE       = 0x00000020,
    NV_COPROC_STATUS_DISABLED_BY_HYBRID        = 0x00000040,
    NV_COPROC_STATUS_OS_NOT_SUPPORTED          = 0x00000080,
    NV_COPROC_STATUS_SBIOS_NOT_CONFIGURED      = 0x00000100,
} NV_COPROC_STATUS;

typedef enum
{
    NV_COPROC_STATE_DGPU_GOLD              = 0x00000000,
    NV_COPROC_STATE_DGPU_ON                = 0x00000001,
} NV_COPROC_STATE;

NVAPI_INTERFACE NvAPI_Coproc_GetCoprocStatus(NvU32 *pCoprocStatus, NV_COPROC_STATE* pCoprocState);

///////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME:   NvAPI_Coproc_GetApplicationCoprocInfo
//
// DESCRIPTION:     This API queries the the coproc status for the application
//                  running in the current process.
//
//                  Supported on Vista and higher.
//
//  SUPPORTED OS: Windows Vista and higher
//
// PARAMETERS:      pCoprocAppInfo (IN)     - the coproc app info structure
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_ERROR - miscellaneous error occurred
//                  NVAPI_INVALID_ARGUMENT - Invalid input parameter
//                  NVAPI_NOT_SUPPORTED - function is not supported in this platform
//                  NVAPI_API_NOT_INTIALIZED
//                  NVAPI_INCOMPATIBLE_STRUCT_VERSION
//
///////////////////////////////////////////////////////////////////////////////

typedef struct
{
    NvU32 version;              // the structure version
    NvU32 appStatus;            // the application status
} NV_COPROC_APP_INFO;


#define NV_COPROC_APP_INFO_VER          MAKE_NVAPI_VERSION(NV_COPROC_APP_INFO, 1)

typedef enum
{
    NV_COPROC_APP_STATUS_ENABLED           = 0x00000000, // coproc mode has been enabled
    NV_COPROC_APP_STATUS_DISABLED          = 0x00000001, // copro mode has been disabled.
} NV_COPROC_APP_STATUS;

NVAPI_INTERFACE NvAPI_Coproc_GetApplicationCoprocInfo(NV_COPROC_APP_INFO *pCoprocAppInfo);

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
    NVAPI_VIDEO_STATE_COMPONENT_2DTO3D          ,   // Converts 2D video to 3D stereo video
    NVAPI_VIDEO_STATE_COMPONENT_3D_ANALYSIS     ,   // Analyzing 3D stereo video
    NVAPI_VIDEO_STATE_COMPONENT_FRC             ,   // Frame Rate Converter
    NVAPI_VIDEO_STATE_COMPONENT_ID_LAST         ,   // All valid components defined before this one
} NVAPI_VIDEO_STATE_COMPONENT_ID;

#define NVAPI_VIDEO_STATE_COMPONENT_DYNAMIC_CONSTRAST  NVAPI_VIDEO_STATE_COMPONENT_DYNAMIC_CONTRAST  // dynamic contrast value. Kept this for backward compatibility

// Algorithms controlling various video components

#define VIDEO_COMP_ALGO_CUSTOM_BASE 64

typedef enum _NVAPI_VIDEO_COMPONENT_ALGORITHM
{
    VIDEO_COMP_ALGO_COLOR_SPACE_601                  = 0,  // Use the ITU-R BT.601 standard in color-space conversion for xxx_COLOR_SPACE component
    VIDEO_COMP_ALGO_COLOR_SPACE_709                  = 1,  // Use the ITU-R BT.709 standard in color-space conversion for xxx_COLOR_SPACE component
    VIDEO_COMP_ALGO_COLOR_SPACE_CUSTOM_04            = VIDEO_COMP_ALGO_CUSTOM_BASE+4,   // Use custom color matrix
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
    VIDEO_COMP_ALGO_2DTO3D_CUSTOM_00                 = VIDEO_COMP_ALGO_CUSTOM_BASE+0, // Use custom 2Dto3D conversion
    VIDEO_COMP_ALGO_2DTO3D_CUSTOM_01                 = VIDEO_COMP_ALGO_CUSTOM_BASE+1, // Use custom 2Dto3D conversion
    VIDEO_COMP_ALGO_3D_ANALYSIS_CUSTOM_00            = VIDEO_COMP_ALGO_CUSTOM_BASE+0, // Use custom 3DAnalysis algorithm
    VIDEO_COMP_ALGO_3D_ANALYSIS_CUSTOM_01            = VIDEO_COMP_ALGO_CUSTOM_BASE+1, // Use custom 3DAnalysis algorithm
    VIDEO_COMP_ALGO_FRC_CUSTOM_00                    = VIDEO_COMP_ALGO_CUSTOM_BASE+0, // Use custom frame rate converter
    VIDEO_COMP_ALGO_FRC_CUSTOM_01                    = VIDEO_COMP_ALGO_CUSTOM_BASE+1, // Use custom frame rate converter
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

//  SUPPORTED OS: Windows XP and higher

///////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME:   NvAPI_SetPVExtProfile
//
// DESCRIPTION:     This API specifies the profile number that the PureVideo
//                  extension dll should use
//
// PARAMETERS:      hNvDisplay(IN)  - NVIDIA Display selection. It can be NVAPI_DEFAULT_HANDLE
//                                    or a handle enumerated from NvAPI_EnumNVidiaDisplayHandle()
//                  dwProfile(IN)   - the profile number
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_ERROR - miscellaneous error occurred
//                  NVAPI_INVALID_ARGUMENT - Invalid input parameter
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_SetPVExtProfile(NvDisplayHandle hNvDisplay, NvU32 dwProfile);

//  SUPPORTED OS: Windows XP and higher

///////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME:   NvAPI_GetPVExtProfile
//
// DESCRIPTION:     This API retrieves the profile number that the PureVideo
//                  extension dll is set to use
//
// PARAMETERS:      hNvDisplay(IN)  - NVIDIA Display selection. It can be NVAPI_DEFAULT_HANDLE
//                                    or a handle enumerated from NvAPI_EnumNVidiaDisplayHandle()
//                  pProfile(OUT)   - the returned profile number
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_ERROR - miscellaneous error occurred
//                  NVAPI_INVALID_ARGUMENT - Invalid input parameter
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GetPVExtProfile(NvDisplayHandle hNvDisplay, NvU32* pProfile);

//  SUPPORTED OS: Windows XP and higher

#ifndef NV_STEREO_VIDEO_FORMAT_DEFINE
#define NV_STEREO_VIDEO_FORMAT_DEFINE

typedef enum _NV_STEREO_VIDEO_FORMAT
{
    NV_STEREO_VIDEO_FORMAT_NOT_STEREO         = 0,

    NV_STEREO_VIDEO_FORMAT_SIDE_BY_SIDE_LR    = 1,
    NV_STEREO_VIDEO_FORMAT_SIDE_BY_SIDE_RL    = 2,
    NV_STEREO_VIDEO_FORMAT_TOP_BOTTOM_LR      = 3,
    NV_STEREO_VIDEO_FORMAT_TOP_BOTTOM_RL      = 4,
    NV_STEREO_VIDEO_FORMAT_ROW_INTERLEAVE_LR  = 5,
    NV_STEREO_VIDEO_FORMAT_ROW_INTERLEAVE_RL  = 6,
    NV_STEREO_VIDEO_FORMAT_TWO_FRAMES_LR      = 7,
    NV_STEREO_VIDEO_FORMAT_MONO_PLUS_OFFSET   = 8,

    NV_STEREO_VIDEO_FORMAT_LAST               = 9,
} NV_STEREO_VIDEO_FORMAT;

#endif // NV_STEREO_VIDEO_FORMAT_DEFINE

typedef struct _NV_VIDEO_STEREO_INFO {
    NvU32                     dwVersion;         // Must be NV_VIDEO_STEREO_INFO_VER
    NV_STEREO_VIDEO_FORMAT    eFormat;           // Stereo format of the surface (please note that format NV_STEREO_VIDEO_FORMAT_TWO_FRAMES_LR is invalid for this NvAPI)
    NvS32                     sViewOffset;       // Signed offset of each view (positive offset indicating left view is shifted left)
    NvU32                     bStereoEnable : 1; // Whether stereo rendering should be enabled (if FALSE, only left view will be used)
} NV_VIDEO_STEREO_INFO;

#define NV_VIDEO_STEREO_INFO_VER  MAKE_NVAPI_VERSION(NV_VIDEO_STEREO_INFO, 1)

//  SUPPORTED OS: Windows XP and higher
///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION_NAME:   NvAPI_VideoSetStereoInfo
//
//   DESCRIPTION:   This api specifies the stereo format of the video source. 
//
//    PARAMETERS:   hNvDisplay(IN) - NVIDIA Display selection. It can be NVAPI_DEFAULT_HANDLE
//                                 or a handle enumerated from NvAPI_EnumNVidiaDisplayHandle()
//                  pStereoInfo(IN) - The stereo format.
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_ERROR - miscellaneous error occurred
//                  NVAPI_INVALID_ARGUMENT - Invalid input parameter
//
///////////////////////////////////////////////////////////////////////////////

NVAPI_INTERFACE NvAPI_VideoSetStereoInfo(NvDisplayHandle hNvDisplay, NV_VIDEO_STEREO_INFO* pStereoInfo);

//  SUPPORTED OS: Windows XP and higher
///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION_NAME:   NvAPI_VideoGetStereoInfo
//
//   DESCRIPTION:   This api retrieves the stereo format of the video source (as set by 
//                  NvAPI_VideoSetStereoInfo()). 
//
//    PARAMETERS:   hNvDisplay(IN) - NVIDIA Display selection. It can be NVAPI_DEFAULT_HANDLE
//                                 or a handle enumerated from NvAPI_EnumNVidiaDisplayHandle()
//                  pStereoInfo(IN/OUT) - The stereo format returned
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_ERROR - miscellaneous error occurred
//                  NVAPI_INVALID_ARGUMENT - Invalid input parameter
//
///////////////////////////////////////////////////////////////////////////////

NVAPI_INTERFACE NvAPI_VideoGetStereoInfo(NvDisplayHandle hNvDisplay, NV_VIDEO_STEREO_INFO* pStereoInfo);


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

#define NV_MOSAIC_MAX_DISPLAYS      (64)

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
    NV_MOSAIC_TOPO_3x2_BASIC,
    NV_MOSAIC_TOPO_4x2_BASIC,
    NV_MOSAIC_TOPO_1x5_BASIC,
    NV_MOSAIC_TOPO_1x6_BASIC,
    NV_MOSAIC_TOPO_7x1_BASIC,

    // Add padding for 10 more entries. 6 will be enough room to specify every
    // possible topology with 8 or fewer displays, so this gives us a little
    // extra should we need it.
    NV_MOSAIC_TOPO_END_BASIC = NV_MOSAIC_TOPO_7x1_BASIC + 9,

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
    NV_MOSAIC_TOPO_END_PASSIVE_STEREO = NV_MOSAIC_TOPO_2x2_PASSIVE_STEREO + 4,

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

//  SUPPORTED OS: Windows XP and higher
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

//  SUPPORTED OS: Windows XP and higher
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

//  SUPPORTED OS: Windows XP and higher
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

//  SUPPORTED OS: Windows XP and higher
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

//  SUPPORTED OS: Windows XP and higher
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

//  SUPPORTED OS: Windows XP and higher
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


//  SUPPORTED OS: Windows XP and higher
///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_Mosaic_SetGridTopology
//
// DESCRIPTION:     This API sets a grid-based Mosaic topology and does a
//                  mode change using the given display settings.
//
//                  If NVAPI_OK is returned, the requested Mosaic topo was set
//                  correctly.  Any other status returned means the
//                  topo was not set, and remains what it was before this
//                  function was called. It is possible for this API to return
//                  success, but the displays enabled have an issue, leading
//                  to a black screen.
//
//                  Note: Not all exposed features are available on all
//                  platforms.
//
//
// PARAMETERS:      pGridTopology(IN):    The topology details to set.
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
//                  NVAPI_NO_ACTIVE_SLI_TOPOLOGY:      GPUs are not currently in SLI
//                  NVAPI_INVALID_COMBINATION:         The current SLI set does not support Mosaic SLI or Immersive Gaming
//                  NVAPI_TOPO_NOT_POSSIBLE:           The topo passed in is not currently possible
//                  NVAPI_API_NOT_INTIALIZED:          The NvAPI API needs to be initialized first
//                  NVAPI_NO_IMPLEMENTATION:           This entrypoint not available
//                  NVAPI_INCOMPATIBLE_STRUCT_VERSION: The version of the structure passed in is not
//                                                     compatible with this entrypoint
//                  NVAPI_MODE_CHANGE_FAILED:          There was an error changing the display mode
//                  NVAPI_ERROR:                       Miscellaneous error occurred
//
///////////////////////////////////////////////////////////////////////////////

//
// This structure is used for applying topologies using NvAPI_Mosaic_SetCurrentTopoEx
// and retrieving them with NvAPI_Mosaic_GetCurrentTopology
//
typedef struct
{
    NvU32                       displayId;              // DisplayID of the display
    NvS32                       overlapX;               // (+overlap, -gap)
    NvS32                       overlapY;               // (+overlap, -gap)
    NV_ROTATE                   rotation;               // Rotation of display
    NvU32                       cloneGroup;             // Reserved, must be 0
} NV_MOSAIC_GRID_TOPO_DISPLAY;

typedef struct
{
    NvU32                       version;                // version of this structure
    NvU32                       rows;                   // Number of rows
    NvU32                       columns;                // Number of columns
    NvU32                       displayCount;           // Number of display details
    NvU32                       applyWithBezelCorrect : 1;  // When enabling and doing the modeset, do we switch to the bezel-corrected resolution
    NvU32                       immersiveGaming : 1;    // Enable as immersive gaming instead of Mosaic SLI (for Quadro-boards only)
    NvU32                       reserved : 30;          // Reserved, must be 0
    NV_MOSAIC_GRID_TOPO_DISPLAY displays[NV_MOSAIC_MAX_DISPLAYS];   // Displays are done as [(row * columns) + column]
    NV_MOSAIC_DISPLAY_SETTING   displaySettings;        // Display settings
} NV_MOSAIC_GRID_TOPO;

#define NV_MOSAIC_GRID_TOPO_VER         MAKE_NVAPI_VERSION(NV_MOSAIC_GRID_TOPO,1)

NVAPI_INTERFACE NvAPI_Mosaic_SetGridTopology(NV_MOSAIC_GRID_TOPO *pGridTopology, NvU32 enable);


//  SUPPORTED OS: Windows XP and higher
///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_Mosaic_GetMosaicCapabilities
//
// DESCRIPTION:     This API returns a set of capabilities based on a provided
//                  SLI topology. The pSliTopology is the same parameter as
//                  used for NvAPI_SetGpuTopologies. If more than one SLI
//                  topology is marked to be active, only the first active
//                  topology will be validated.
//                  At least one topology in pSliTopology must have the
//                  NV_GPU_TOPOLOGY_ACTIVE flag set. This does not require the
//                  topology to be currently active, but marks which topology
//                  will have the capabilities retrieved for.
//
// PARAMETERS:      pSliTopology(IN):   SLI topology for capabilties query
//                  pCaps(OUT):         Capabilities for Mosaic SLI
//
// RETURN STATUS    NVAPI_OK:                          Capabilties have been returned.
//                  NVAPI_INVALID_ARGUMENT:            One or more args passed in are invalid.
//                  NVAPI_API_NOT_INTIALIZED:          The NvAPI API needs to be initialized first
//                  NVAPI_NO_ACTIVE_SLI_TOPOLOGY:      No topology had the NV_GPU_TOPOLOGY_ACTIVE flag marked
//                  NVAPI_NO_IMPLEMENTATION:           This entrypoint not available
//                  NVAPI_ERROR:                       Miscellaneous error occurred
//
///////////////////////////////////////////////////////////////////////////////
typedef struct
{
    NvU32  version;                          // Must be set to NV_MOSAIC_CAPS_VER
    NvU32  bSupportsImmersiveGaming : 1;     // These GPUs could enter immersive gaming if proper display are attached
    NvU32  bSupportsMosaicSli : 1;           // Supports QuadroPlex Mosaic SLI
    NvU32  bSupportsGlobalRotation : 1;      // All displays must share rotation
    NvU32  bSupportsPerDisplayRotation : 1;  // Each display may have a different rotation
    NvU32  bSupportsPerDisplaySettings : 1;  // Each display may have a different resolution
    NvU32  bSupportsOverlap : 1;             // Overlap is supported (+overlap)
    NvU32  bSupportsGaps : 1;                // Gap is supported (-overlap)
    NvU32  bSupportsUnevenGapping : 1;       // Allows for each display to have different overlap values
    NvU32  reserved : 24;                    // Reserved, must be 0
} NV_MOSAIC_CAPS;

#define NV_MOSAIC_CAPS_VER                  MAKE_NVAPI_VERSION(NV_MOSAIC_CAPS,1)

NVAPI_INTERFACE NvAPI_Mosaic_GetMosaicCapabilities(NV_GPU_VALID_GPU_TOPOLOGIES *pSliTopology, NV_MOSAIC_CAPS *pCaps);


//  SUPPORTED OS: Windows XP and higher
///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_Mosaic_GetDisplayCapabilities
//
// DESCRIPTION:     This API returns a set of capabilities based on a provided
//                  displays. The caller uses this function to determine if a
//                  given set of displays, regardless of SLI topology, would
//                  support Mosaic SLI / Immersive Gaming. This API can also
//                  assist in identifying if a display would be compatible,
//                  but is currently plugged into the wrong connector of a GPU.
//
//                  Problem flags are SLI-agnostic. This API will not verify
//                  that all displays are on the same SLI topology.
//
// PARAMETERS:      pDisplayCaps(IN/OUT):  List of display capabilities and
//                                         problem codes, as well as list of
//                                         valid display settings shared by the
//                                         selected monitors.
//
// RETURN STATUS    NVAPI_OK:                          The display capabilities have been returned.
//                  NVAPI_INVALID_ARGUMENT:            One or more args passed in are invalid.
//                  NVAPI_API_NOT_INTIALIZED:          The NvAPI API needs to be initialized first
//                  NVAPI_NO_IMPLEMENTATION:           This entrypoint not available
//                  NVAPI_ERROR:                       Miscellaneous error occurred
//
///////////////////////////////////////////////////////////////////////////////

#define NV_MOSAIC_DISPLAYCAPS_PROBLEM_DISPLAY_ON_INVALID_GPU        NV_BIT(0)
#define NV_MOSAIC_DISPLAYCAPS_PROBLEM_DISPLAY_ON_WRONG_CONNECTOR    NV_BIT(1)
#define NV_MOSAIC_DISPLAYCAPS_PROBLEM_NO_COMMON_TIMINGS             NV_BIT(2)
#define NV_MOSAIC_DISPLAYCAPS_PROBLEM_NO_EDID_AVAILABLE             NV_BIT(3)
#define NV_MOSAIC_DISPLAYCAPS_PROBLEM_MISMATCHED_OUTPUT_TYPE        NV_BIT(4)
#define NV_MOSAIC_DISPLAYCAPS_PROBLEM_NO_DISPLAY_CONNECTED          NV_BIT(5)

typedef struct
{
    NvU32  displayId;                       // (IN) DisplayID of display
    NvU32  problemFlags;                    // (OUT) Any problem flags found (NV_MOSAIC_DISPLAYCAPS_PROBLEM_*)
    NvU32  supportsRotation : 1;            // (OUT) This display can be rotated
    NvU32  reserved : 31;                   // (OUT) reserved
} NV_MOSAIC_DISPLAY_DETAILS;

typedef struct
{
    NvU32  version;                         // (IN) Must be NV_MOSAIC_DISPLAY_CAPS_VER
    NvU32  displayCount;                    // (IN) Number of valid displays in displays array
    NV_MOSAIC_DISPLAY_DETAILS   displays[NVAPI_MAX_DISPLAYS];   // (IN/OUT) Array of individual displays, the first
                                                                // display will be used to validate the remaining displays
                                                                // for EDID matching. There is other order dependency.
    NvU32  displaySettingsCount;            // (OUT) Number of valid display settings found
    NV_MOSAIC_DISPLAY_SETTING   displaySettings[NV_MOSAIC_DISPLAY_SETTINGS_MAX];    // (OUT) List of common timings
    NvU32  problemFlags;                    // (OUT) Any problem flags found (NV_MOSAIC_DISPLAYCAPS_PROBLEM_*)
} NV_MOSAIC_DISPLAY_CAPS_V1;

#define NV_MOSAIC_DISPLAY_CAPS_MAX      1600

typedef struct
{
    NvU32  version;                         // (IN) Must be NV_MOSAIC_DISPLAY_CAPS_VER
    NvU32  displayCount;                    // (IN) Number of valid displays in displays array
    NV_MOSAIC_DISPLAY_DETAILS   displays[NVAPI_MAX_DISPLAYS];   // (IN/OUT) Array of individual displays, the first
                                                                // display will be used to validate the remaining displays
                                                                // for EDID matching. There is other order dependency.
    NvU32  displaySettingsCount;            // (OUT) Number of valid display settings found
    NV_MOSAIC_DISPLAY_SETTING   displaySettings[NV_MOSAIC_DISPLAY_CAPS_MAX];    // (OUT) List of common timings
    NvU32  problemFlags;                    // (OUT) Any problem flags found (NV_MOSAIC_DISPLAYCAPS_PROBLEM_*)
} NV_MOSAIC_DISPLAY_CAPS_V2;

typedef NV_MOSAIC_DISPLAY_CAPS_V2           NV_MOSAIC_DISPLAY_CAPS;

#define NV_MOSAIC_DISPLAY_CAPS_VER1         MAKE_NVAPI_VERSION(NV_MOSAIC_DISPLAY_CAPS_V1,1)
#define NV_MOSAIC_DISPLAY_CAPS_VER2         MAKE_NVAPI_VERSION(NV_MOSAIC_DISPLAY_CAPS_V2,2)
#define NV_MOSAIC_DISPLAY_CAPS_VER          NV_MOSAIC_DISPLAY_CAPS_VER2

NVAPI_INTERFACE NvAPI_Mosaic_GetDisplayCapabilities(NV_MOSAIC_DISPLAY_CAPS *pDisplayCaps);


//  SUPPORTED OS: Windows XP and higher
///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_Mosaic_EnumGridTopologies
//
// DESCRIPTION:     This API retrieves the current grid topology information
//                  for Mosaic SLI.
//
//                  If there has been no existing call to set a topology
//                  (either enabled or disable), this call will return
//                  NVAPI_DATA_NOT_FOUND. If the existing data is no longer
//                  valid due to display connectors being moved, or GPUs
//                  being moved, NVAPI_DATA_NOT_FOUND will also be returned.
//                  This API does not validate whether SLI is currently
//                  enabled.
//
//                  When enumerating all current grid topologies, NVAPI_DATA_NOT_FOUND will
//                  be returned when there are no more topologies.
//
// PARAMETERS:      index(IN):              A zero-based index to enumerate
//                                          multiple applied grid topologies.
//                  flags(IN):              This field controls which data is returned.
//                                          By default, all known topologies are returned.
//                                          A known topology is either an active topology
//                                          or a topology which has been disabled and not
//                                          included in another topology. Unless specified,
//                                          topology validity will be applied during enum,
//                                          and topologies which are no longer valid will
//                                          not be returned (ex: a monitor was disconnected).
//                                          Only topologies which have been enabled in the 
//                                          past will be returned, this API will not 
//                                          enumerate all possible topologies. To determine 
//                                          all possible topologies, manually create each 
//                                          possible topology and validate each using 
//                                          NvAPI_Mosaic_GetDisplayCapabilities. 
//                  pGridTopology(IN/OUT):  The topology details of the current
//                                          topology. On input, version must be
//                                          properly set, all other fields are ignored.
//                  bEnabled(OUT):          (May pass in NULL to ignore)
//                                          1 if Mosaic is enabled,
//                                          0 if Mosaic is currently disabled.
//
// RETURN STATUS    NVAPI_OK:                          Mosaic topo was set
//                  NVAPI_NOT_SUPPORTED:               Mosaic is not supported with the existing hardware
//                  NVAPI_INVALID_ARGUMENT:            One or more args passed in are invalid.
//                  NVAPI_API_NOT_INTIALIZED:          The NvAPI API needs to be initialized first
//                  NVAPI_NO_IMPLEMENTATION:           This entrypoint not available
//                  NVAPI_INCOMPATIBLE_STRUCT_VERSION: The version of the structure passed in is not
//                                                     compatible with this entrypoint
//                  NVAPI_DATA_NOT_FOUND:              There was no existing valid topology information.
//                  NVAPI_ERROR:                       Miscellaneous error occurred
//
///////////////////////////////////////////////////////////////////////////////

#define NVAPI_MOSAIC_ENUMGRIDTOPOS_FLAG_ACTIVE_ONLY       NV_BIT(0)
#define NVAPI_MOSAIC_ENUMGRIDTOPOS_FLAG_NO_VALIDATION     NV_BIT(1)

NVAPI_INTERFACE NvAPI_Mosaic_EnumGridTopologies(NvU32 index, NvU32 flags, NV_MOSAIC_GRID_TOPO *pGridTopology, NvU8* bEnabled);


//  SUPPORTED OS: Windows Vista and higher
///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_Mosaic_GetDisplayViewportsByResolution
//
// DESCRIPTION:     This API returns the viewports which would be applied on
//                  the requested display.
//
// PARAMETERS:      displayId(IN):    Display ID of a single display in the active
//                                    mosaic topology to query.
//                  srcWidth(IN):     Width of full display topology. If both
//                                    width and height are 0, the current
//                                    resolution is used.
//                  srcHeight(IN):    Height of full display topology. If both
//                                    width and height are 0, the current
//                                    resolution is used.
//                  viewports (OUT):  Array of NV_RECT viewports which represent
//                                    the displays as identified in
//                                    NvAPI_Mosaic_EnumGridTopologies. If the
//                                    requested resolution is a single-wide
//                                    resolution, only viewports[0] will
//                                    contain the viewport details, regardless
//                                    of which display is driving the display.
//                  bezelCorrected(OUT):  Returns 1 if the requested resolution is
//                                    bezel corrected. May be NULL.
//
// RETURN STATUS    NVAPI_OK:                          Capabilties have been returned.
//                  NVAPI_INVALID_ARGUMENT:            One or more args passed in are invalid.
//                  NVAPI_API_NOT_INTIALIZED:          The NvAPI API needs to be initialized first
//                  NVAPI_MOSAIC_NOT_ACTIVE:           The display does not belong to an active Mosaic Topology
//                  NVAPI_NO_IMPLEMENTATION:           This entrypoint not available
//                  NVAPI_ERROR:                       Miscellaneous error occurred
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Mosaic_GetDisplayViewportsByResolution(NvU32 displayId, NvU32 srcWidth, NvU32 srcHeight, NV_RECT viewports[NV_MOSAIC_MAX_DISPLAYS], NvU8* bezelCorrected);

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
}NV_3D_APP_INFO_V1;

typedef struct
{
  NvU32 version;    // Structure version
  NvU32 processId;
  NvAPI_LongString   processName;
} NV_3D_APP_INFO_V2;

typedef NV_3D_APP_INFO_V2     NV_3D_APP_INFO;

#define NV_3D_APP_INFO_VER_1  MAKE_NVAPI_VERSION(NV_3D_APP_INFO_V1,1)
#define NV_3D_APP_INFO_VER_2  MAKE_NVAPI_VERSION(NV_3D_APP_INFO_V2,2)
#define NV_3D_APP_INFO_VER    NV_3D_APP_INFO_VER_2

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_QueryNonMigratableApps
//
// PARAMETERS:      apps(IN/OUT)       -Empty structure passed as an input and upon successfull exit, it contains list of nonmigratable apps and processIDs.
//                  total(IN/OUT)      -Total number of nonmigratable apps currently running in the system.
// DESCRIPTION:     Query all non-migratable apps which can block successful driver reload like SLI or Hybrid transition.
//
//  SUPPORTED OS: Windows XP and higher
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_ERROR - miscellaneous error occurred
//                  NVAPI_INVALID_ARGUMENT - Invalid input parameter
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_QueryNonMigratableApps(NV_3D_APP_INFO apps[NVAPI_MAX_3D_Apps] , NvU32 *total);

typedef struct
{
  NvU32 version;    // Structure version
  NvAPI_LongString   appName; //an application executable name, a full path, or a partial path
  NvAPI_LongString   friendlyName;
}NV_HYBRID_APP_INFO;

#define NV_HYBRID_APP_INFO_VER  MAKE_NVAPI_VERSION(NV_HYBRID_APP_INFO,1)

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_Hybrid_QueryUnblockedNonMigratableApps
//
// PARAMETERS:      apps(IN/OUT)     -Empty structure passed as an input and upon successfull exit, it contains list of Unblocked(user white-list) applications..
//                  total(IN/OUT)    -Total number of applications returned from the white list.
// DESCRIPTION:     Query the user-white list applications for Hybrid transition
//
//  SUPPORTED OS: Windows Vista and higher
//
// RETURN STATUS:
//                  NVAPI_OK - completed request.
//                  NVAPI_ERROR - miscellaneous error occurred.
//                  NVAPI_INVALID_ARGUMENT - invalid input parameter.
//                  NVAPI_NO_IMPLEMENTATION - not implemented.
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Hybrid_QueryUnblockedNonMigratableApps(NV_HYBRID_APP_INFO apps[NVAPI_MAX_3D_Apps] , NvU32 *total);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_Hybrid_QueryBlockedMigratableApps
//
// PARAMETERS:      apps(IN/OUT)     -Empty structure passed as an input and upon successfull exit, it contains list of blocked(user black-list) applications..
//                  total(IN/OUT)    -Total number of applications returned from the black list.
// DESCRIPTION:     Query the user-black list applications for Hybrid transition
//
//  SUPPORTED OS: Windows Vista and higher
//
// RETURN STATUS:
//                  NVAPI_OK - completed request.
//                  NVAPI_ERROR - miscellaneous error occurred.
//                  NVAPI_INVALID_ARGUMENT - invalid input parameter.
//                  NVAPI_NO_IMPLEMENTATION - not implemented.
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Hybrid_QueryBlockedMigratableApps(NV_HYBRID_APP_INFO apps[NVAPI_MAX_3D_Apps] , NvU32 *total);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_Hybrid_SetAppMigrationState
//
// PARAMETERS:      app(IN) - application executable name that is to be blocked, unblocked, reset or removed
//                  flag(IN) -decides on the block, unblock, reset or remove operation.
//                            block =0, unblock =1, remove =2, reset =3
//                  remove - it will remove the application from both blocked and unblocked list
//                  reset  - it will remove all application blocked and unblocked list.(no need to specify any application)
// DESCRIPTION:     Override the pre-determined application migration state.
//
//  SUPPORTED OS: Windows Vista and higher
//
// RETURN STATUS:
//                  NVAPI_OK - completed request.
//                  NVAPI_ERROR - miscellaneous error occurred.
//                  NVAPI_INVALID_ARGUMENT - invalid input parameter.
//                  NVAPI_SET_NOT_ALLOWED - the application list override is not allowed.
//                  NVAPI_NO_IMPLEMENTATION - not implemented.
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Hybrid_SetAppMigrationState(NV_HYBRID_APP_INFO app , NvU32 flag);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_Hybrid_IsAppMigrationStateChangeable
//
// PARAMETERS:      app(IN) - application executable name which user is querying about
//                  allowed(OUT) -Change in migration state of app is allowed or not.
//                                not_allowed =0, allowed =1
// DESCRIPTION:     Query if an application's migration state can be changed by the user.
//
//  SUPPORTED OS: Windows Vista and higher
//
// RETURN STATUS:
//                  NVAPI_OK - completed request.
//                  NVAPI_ERROR - miscellaneous error occurred.
//                  NVAPI_INVALID_ARGUMENT - invalid input parameter.
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Hybrid_IsAppMigrationStateChangeable(NV_HYBRID_APP_INFO app , NvU32 *allowed);


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



#if defined(_D3D9_H_) || defined(__d3d10_h__) || defined(__d3d11_h__)
///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_CreateHandleFromIUnknown
//
// PARAMETERS:    pDevice(IN) - Pointer to IUnknown interface that is IDirect3DDevice9*, ID3D10Device* or ID3D11Device.
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
// FUNCTION NAME: NvAPI_Stereo_SetActiveEye
//
// PARAMETERS:    stereoHandle(IN) - Stereo handle correspondent to device interface.
//                StereoEye(IN)    - Defines active eye in Direct stereo mode
//
//  SUPPORTED OS: Windows Vista and higher
//
// DESCRIPTION:   Set BackBuffer to left or right in Direct stereo mode.
//
// HOW TO USE:    After the stereo handle for device interface is created via successfull call to appropriate
//                NvAPI_Stereo_CreateHandleFrom function.
//
// RETURN STATUS:
//                NVAPI_OK - Active eye is set.
//                NVAPI_STEREO_INVALID_DEVICE_INTERFACE - Device interface is not valid. Create again, then attach again.
//                NVAPI_API_NOT_INTIALIZED - NVAPI not initialized.
//                NVAPI_STEREO_NOT_INITIALIZED - Stereo part of NVAPI not initialized.
//                NVAPI_INVALID_ARGUMENT - StereoEye parameter has not allowed value.
//                NVAPI_SET_NOT_ALLOWED  - Current stereo mode is not Direct
//                NVAPI_ERROR - Something is wrong (generic error).
//
///////////////////////////////////////////////////////////////////////////////
typedef enum _NV_StereoActiveEye
{
    NVAPI_STEREO_EYE_RIGHT = 1,
    NVAPI_STEREO_EYE_LEFT = 2,
} NV_STEREO_ACTIVE_EYE;

NVAPI_INTERFACE NvAPI_Stereo_SetActiveEye(StereoHandle hStereoHandle, NV_STEREO_ACTIVE_EYE StereoEye);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_SetDriverMode
//
// PARAMETERS:    mode(IN)         - Defines stereo driver mode: Direct or Automatic
//
//
//  SUPPORTED OS: Windows Vista and higher
//
// DESCRIPTION:   Set stereo driver mode: Direct or Automatic
//
// HOW TO USE:    This API has to be called before device is created
//                Applyed for DX9 and up.
//
// RETURN STATUS:
//                NVAPI_OK - Active eye is set.
//                NVAPI_API_NOT_INTIALIZED - NVAPI not initialized.
//                NVAPI_STEREO_NOT_INITIALIZED - Stereo part of NVAPI not initialized.
//                NVAPI_INVALID_ARGUMENT - mode parameter has not allowed value.
//                NVAPI_ERROR - Something is wrong (generic error).
//
///////////////////////////////////////////////////////////////////////////////
typedef enum _NV_StereoDriverMode
{
    NVAPI_STEREO_DRIVER_MODE_AUTOMATIC = 0,
    NVAPI_STEREO_DRIVER_MODE_DIRECT    = 2,
} NV_STEREO_DRIVER_MODE;

NVAPI_INTERFACE NvAPI_Stereo_SetDriverMode( NV_STEREO_DRIVER_MODE mode );

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_GetEyeSeparation
//
// PARAMETERS:    stereoHandle(IN) - Stereo handle correspondent to device interface.
//                pSeparation(OUT) - Eye separation.
//
//  SUPPORTED OS: Windows Vista and higher
//
// DESCRIPTION:   Return eye separation as <between eye distance>/<phisical screen width> ratio
//
// HOW TO USE:    After the stereo handle for device interface is created via successfull call to appropriate
//                Applyed only for DX9 and up.
//
// RETURN STATUS:
//                NVAPI_OK - Active eye is set.
//                NVAPI_STEREO_INVALID_DEVICE_INTERFACE - Device interface is not valid. Create again, then attach again.
//                NVAPI_API_NOT_INTIALIZED - NVAPI not initialized.
//                NVAPI_STEREO_NOT_INITIALIZED - Stereo part of NVAPI not initialized.
//                NVAPI_ERROR - Something is wrong (generic error).
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Stereo_GetEyeSeparation(StereoHandle hStereoHandle,  float *pSeparation );

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


typedef NvU32   NVVIOOWNERID;                               // Unique identifier for VIO owner (process identifier or NVVIOOWNERID_NONE)
#define NVVIOOWNERID_NONE                   0               // Unregistered ownerId

typedef enum _NVVIOOWNERTYPE                                // Owner type for device
{
    NVVIOOWNERTYPE_NONE                             ,       //  No owner for device
    NVVIOOWNERTYPE_APPLICATION                      ,       //  Application owns device
    NVVIOOWNERTYPE_DESKTOP                          ,       //  Desktop transparent mode owns device (not applicable for video input)
}NVVIOOWNERTYPE;

// Access rights for NvAPI_VIO_Open()
#define NVVIO_O_READ                        0x00000000      // Read access             (not applicable for video output)
#define NVVIO_O_WRITE_EXCLUSIVE             0x00010001      // Write exclusive access  (not applicable for video input)

#define NVVIO_VALID_ACCESSRIGHTS            (NVVIO_O_READ              | \
                                             NVVIO_O_WRITE_EXCLUSIVE   )


// VIO_DATA.ulOwnerID high-bit is set only if device has been initialized by VIOAPI
// examined at NvAPI_GetCapabilities|NvAPI_VIO_Open to determine if settings need to be applied from registry or POR state read
#define NVVIO_OWNERID_INITIALIZED  0x80000000

// VIO_DATA.ulOwnerID next-bit is set only if device is currently in exclusive write access mode from NvAPI_VIO_Open()
#define NVVIO_OWNERID_EXCLUSIVE    0x40000000

// VIO_DATA.ulOwnerID lower bits are:
//  NVGVOOWNERTYPE_xxx enumerations indicating use context
#define NVVIO_OWNERID_TYPEMASK     0x0FFFFFFF // mask for NVVIOOWNERTYPE_xxx
//---------------------------------------------------------------------
// Enumerations
//---------------------------------------------------------------------

// Video signal format and resolution
typedef enum _NVVIOSIGNALFORMAT
{
    NVVIOSIGNALFORMAT_NONE,                         // Invalid signal format
    NVVIOSIGNALFORMAT_487I_59_94_SMPTE259_NTSC,     // 01  487i    59.94Hz  (SMPTE259) NTSC
    NVVIOSIGNALFORMAT_576I_50_00_SMPTE259_PAL,      // 02  576i    50.00Hz  (SMPTE259) PAL
    NVVIOSIGNALFORMAT_1035I_60_00_SMPTE260,         // 03  1035i   60.00Hz  (SMPTE260)
    NVVIOSIGNALFORMAT_1035I_59_94_SMPTE260,         // 04  1035i   59.94Hz  (SMPTE260)
    NVVIOSIGNALFORMAT_1080I_50_00_SMPTE295,         // 05  1080i   50.00Hz  (SMPTE295)
    NVVIOSIGNALFORMAT_1080I_60_00_SMPTE274,         // 06  1080i   60.00Hz  (SMPTE274)
    NVVIOSIGNALFORMAT_1080I_59_94_SMPTE274,         // 07  1080i   59.94Hz  (SMPTE274)
    NVVIOSIGNALFORMAT_1080I_50_00_SMPTE274,         // 08  1080i   50.00Hz  (SMPTE274)
    NVVIOSIGNALFORMAT_1080P_30_00_SMPTE274,         // 09  1080p   30.00Hz  (SMPTE274)
    NVVIOSIGNALFORMAT_1080P_29_97_SMPTE274,         // 10  1080p   29.97Hz  (SMPTE274)
    NVVIOSIGNALFORMAT_1080P_25_00_SMPTE274,         // 11  1080p   25.00Hz  (SMPTE274)
    NVVIOSIGNALFORMAT_1080P_24_00_SMPTE274,         // 12  1080p   24.00Hz  (SMPTE274)
    NVVIOSIGNALFORMAT_1080P_23_976_SMPTE274,        // 13  1080p   23.976Hz (SMPTE274)
    NVVIOSIGNALFORMAT_720P_60_00_SMPTE296,          // 14  720p    60.00Hz  (SMPTE296)
    NVVIOSIGNALFORMAT_720P_59_94_SMPTE296,          // 15  720p    59.94Hz  (SMPTE296)
    NVVIOSIGNALFORMAT_720P_50_00_SMPTE296,          // 16  720p    50.00Hz  (SMPTE296)
    NVVIOSIGNALFORMAT_1080I_48_00_SMPTE274,         // 17  1080I   48.00Hz  (SMPTE274)
    NVVIOSIGNALFORMAT_1080I_47_96_SMPTE274,         // 18  1080I   47.96Hz  (SMPTE274)
    NVVIOSIGNALFORMAT_720P_30_00_SMPTE296,          // 19  720p    30.00Hz  (SMPTE296)
    NVVIOSIGNALFORMAT_720P_29_97_SMPTE296,          // 20  720p    29.97Hz  (SMPTE296)
    NVVIOSIGNALFORMAT_720P_25_00_SMPTE296,          // 21  720p    25.00Hz  (SMPTE296)
    NVVIOSIGNALFORMAT_720P_24_00_SMPTE296,          // 22  720p    24.00Hz  (SMPTE296)
    NVVIOSIGNALFORMAT_720P_23_98_SMPTE296,          // 23  720p    23.98Hz  (SMPTE296)
    NVVIOSIGNALFORMAT_2048P_30_00_SMPTE372,         // 24  2048p   30.00Hz  (SMPTE372)
    NVVIOSIGNALFORMAT_2048P_29_97_SMPTE372,         // 25  2048p   29.97Hz  (SMPTE372)
    NVVIOSIGNALFORMAT_2048I_60_00_SMPTE372,         // 26  2048i   60.00Hz  (SMPTE372)
    NVVIOSIGNALFORMAT_2048I_59_94_SMPTE372,         // 27  2048i   59.94Hz  (SMPTE372)
    NVVIOSIGNALFORMAT_2048P_25_00_SMPTE372,         // 28  2048p   25.00Hz  (SMPTE372)
    NVVIOSIGNALFORMAT_2048I_50_00_SMPTE372,         // 29  2048i   50.00Hz  (SMPTE372)
    NVVIOSIGNALFORMAT_2048P_24_00_SMPTE372,         // 30  2048p   24.00Hz  (SMPTE372)
    NVVIOSIGNALFORMAT_2048P_23_98_SMPTE372,         // 31  2048p   23.98Hz  (SMPTE372)
    NVVIOSIGNALFORMAT_2048I_48_00_SMPTE372,         // 32  2048i   48.00Hz  (SMPTE372)
    NVVIOSIGNALFORMAT_2048I_47_96_SMPTE372,         // 33  2048i   47.96Hz  (SMPTE372)

    NVVIOSIGNALFORMAT_1080PSF_25_00_SMPTE274,       // 34  1080PsF 25.00Hz  (SMPTE274)
    NVVIOSIGNALFORMAT_1080PSF_29_97_SMPTE274,       // 35  1080PsF 29.97Hz  (SMPTE274)
    NVVIOSIGNALFORMAT_1080PSF_30_00_SMPTE274,       // 36  1080PsF 30.00Hz  (SMPTE274)
    NVVIOSIGNALFORMAT_1080PSF_24_00_SMPTE274,       // 37  1080PsF 24.00Hz  (SMPTE274)
    NVVIOSIGNALFORMAT_1080PSF_23_98_SMPTE274,       // 38  1080PsF 23.98Hz  (SMPTE274)

    NVVIOSIGNALFORMAT_1080P_50_00_SMPTE274_3G_LEVEL_A, // 39  1080P   50.00Hz  (SMPTE274) 3G Level A
    NVVIOSIGNALFORMAT_1080P_59_94_SMPTE274_3G_LEVEL_A, // 40  1080P   59.94Hz  (SMPTE274) 3G Level A
    NVVIOSIGNALFORMAT_1080P_60_00_SMPTE274_3G_LEVEL_A, // 41  1080P   60.00Hz  (SMPTE274) 3G Level A

    NVVIOSIGNALFORMAT_1080P_60_00_SMPTE274_3G_LEVEL_B, // 42  1080p   60.00Hz  (SMPTE274) 3G Level B
    NVVIOSIGNALFORMAT_1080I_60_00_SMPTE274_3G_LEVEL_B, // 43  1080i   60.00Hz  (SMPTE274) 3G Level B
    NVVIOSIGNALFORMAT_2048I_60_00_SMPTE372_3G_LEVEL_B, // 44  2048i   60.00Hz  (SMPTE372) 3G Level B
    NVVIOSIGNALFORMAT_1080P_50_00_SMPTE274_3G_LEVEL_B, // 45  1080p   50.00Hz  (SMPTE274) 3G Level B
    NVVIOSIGNALFORMAT_1080I_50_00_SMPTE274_3G_LEVEL_B, // 46  1080i   50.00Hz  (SMPTE274) 3G Level B
    NVVIOSIGNALFORMAT_2048I_50_00_SMPTE372_3G_LEVEL_B, // 47  2048i   50.00Hz  (SMPTE372) 3G Level B
    NVVIOSIGNALFORMAT_1080P_30_00_SMPTE274_3G_LEVEL_B, // 48  1080p   30.00Hz  (SMPTE274) 3G Level B
    NVVIOSIGNALFORMAT_2048P_30_00_SMPTE372_3G_LEVEL_B, // 49  2048p   30.00Hz  (SMPTE372) 3G Level B
    NVVIOSIGNALFORMAT_1080P_25_00_SMPTE274_3G_LEVEL_B, // 50  1080p   25.00Hz  (SMPTE274) 3G Level B
    NVVIOSIGNALFORMAT_2048P_25_00_SMPTE372_3G_LEVEL_B, // 51  2048p   25.00Hz  (SMPTE372) 3G Level B
    NVVIOSIGNALFORMAT_1080P_24_00_SMPTE274_3G_LEVEL_B, // 52  1080p   24.00Hz  (SMPTE274) 3G Level B
    NVVIOSIGNALFORMAT_2048P_24_00_SMPTE372_3G_LEVEL_B, // 53  2048p   24.00Hz  (SMPTE372) 3G Level B
    NVVIOSIGNALFORMAT_1080I_48_00_SMPTE274_3G_LEVEL_B, // 54  1080i   48.00Hz  (SMPTE274) 3G Level B
    NVVIOSIGNALFORMAT_2048I_48_00_SMPTE372_3G_LEVEL_B, // 55  2048i   48.00Hz  (SMPTE372) 3G Level B
    NVVIOSIGNALFORMAT_1080P_59_94_SMPTE274_3G_LEVEL_B, // 56  1080p   59.94Hz  (SMPTE274) 3G Level B
    NVVIOSIGNALFORMAT_1080I_59_94_SMPTE274_3G_LEVEL_B, // 57  1080i   59.94Hz  (SMPTE274) 3G Level B
    NVVIOSIGNALFORMAT_2048I_59_94_SMPTE372_3G_LEVEL_B, // 58  2048i   59.94Hz  (SMPTE372) 3G Level B
    NVVIOSIGNALFORMAT_1080P_29_97_SMPTE274_3G_LEVEL_B, // 59  1080p   29.97Hz  (SMPTE274) 3G Level B
    NVVIOSIGNALFORMAT_2048P_29_97_SMPTE372_3G_LEVEL_B, // 60  2048p   29.97Hz  (SMPTE372) 3G Level B
    NVVIOSIGNALFORMAT_1080P_23_98_SMPTE274_3G_LEVEL_B, // 61  1080p   29.98Hz  (SMPTE274) 3G Level B
    NVVIOSIGNALFORMAT_2048P_23_98_SMPTE372_3G_LEVEL_B, // 62  2048p   29.98Hz  (SMPTE372) 3G Level B
    NVVIOSIGNALFORMAT_1080I_47_96_SMPTE274_3G_LEVEL_B, // 63  1080i   47.96Hz  (SMPTE274) 3G Level B
    NVVIOSIGNALFORMAT_2048I_47_96_SMPTE372_3G_LEVEL_B, // 64  2048i   47.96Hz  (SMPTE372) 3G Level B

    NVVIOSIGNALFORMAT_END                              // 65  To indicate end of signal format list
}NVVIOSIGNALFORMAT;

// SMPTE standards format
typedef enum _NVVIOVIDEOSTANDARD
{
    NVVIOVIDEOSTANDARD_SMPTE259                        ,       // SMPTE259
    NVVIOVIDEOSTANDARD_SMPTE260                        ,       // SMPTE260
    NVVIOVIDEOSTANDARD_SMPTE274                        ,       // SMPTE274
    NVVIOVIDEOSTANDARD_SMPTE295                        ,       // SMPTE295
    NVVIOVIDEOSTANDARD_SMPTE296                        ,       // SMPTE296
    NVVIOVIDEOSTANDARD_SMPTE372                        ,       // SMPTE372
}NVVIOVIDEOSTANDARD;

// HD or SD video type
typedef enum _NVVIOVIDEOTYPE
{
    NVVIOVIDEOTYPE_SD                                  ,       // Standard-definition (SD)
    NVVIOVIDEOTYPE_HD                                  ,       // High-definition     (HD)
}NVVIOVIDEOTYPE;

// Interlace mode
typedef enum _NVVIOINTERLACEMODE
{
    NVVIOINTERLACEMODE_PROGRESSIVE                     ,       // Progressive               (p)
    NVVIOINTERLACEMODE_INTERLACE                       ,       // Interlace                 (i)
    NVVIOINTERLACEMODE_PSF                             ,       // Progressive Segment Frame (psf)
}NVVIOINTERLACEMODE;

// Video data format
typedef enum _NVVIODATAFORMAT
{
    NVVIODATAFORMAT_UNKNOWN   = -1                     ,       // Invalid DataFormat
    NVVIODATAFORMAT_R8G8B8_TO_YCRCB444                 ,       // R8:G8:B8                => YCrCb  (4:4:4)
    NVVIODATAFORMAT_R8G8B8A8_TO_YCRCBA4444             ,       // R8:G8:B8:A8             => YCrCbA (4:4:4:4)
    NVVIODATAFORMAT_R8G8B8Z10_TO_YCRCBZ4444            ,       // R8:G8:B8:Z10            => YCrCbZ (4:4:4:4)
    NVVIODATAFORMAT_R8G8B8_TO_YCRCB422                 ,       // R8:G8:B8                => YCrCb  (4:2:2)
    NVVIODATAFORMAT_R8G8B8A8_TO_YCRCBA4224             ,       // R8:G8:B8:A8             => YCrCbA (4:2:2:4)
    NVVIODATAFORMAT_R8G8B8Z10_TO_YCRCBZ4224            ,       // R8:G8:B8:Z10            => YCrCbZ (4:2:2:4)
    NVVIODATAFORMAT_X8X8X8_444_PASSTHRU                ,       // R8:G8:B8                => RGB    (4:4:4)
    NVVIODATAFORMAT_X8X8X8A8_4444_PASSTHRU             ,       // R8:G8:B8:A8             => RGBA   (4:4:4:4)
    NVVIODATAFORMAT_X8X8X8Z10_4444_PASSTHRU            ,       // R8:G8:B8:Z10            => RGBZ   (4:4:4:4)
    NVVIODATAFORMAT_X10X10X10_444_PASSTHRU             ,       // Y10:CR10:CB10           => YCrCb  (4:4:4)
    NVVIODATAFORMAT_X10X8X8_444_PASSTHRU               ,       // Y10:CR8:CB8             => YCrCb  (4:4:4)
    NVVIODATAFORMAT_X10X8X8A10_4444_PASSTHRU           ,       // Y10:CR8:CB8:A10         => YCrCbA (4:4:4:4)
    NVVIODATAFORMAT_X10X8X8Z10_4444_PASSTHRU           ,       // Y10:CR8:CB8:Z10         => YCrCbZ (4:4:4:4)
    NVVIODATAFORMAT_DUAL_R8G8B8_TO_DUAL_YCRCB422       ,       // R8:G8:B8 + R8:G8:B8     => YCrCb  (4:2:2 + 4:2:2)
    NVVIODATAFORMAT_DUAL_X8X8X8_TO_DUAL_422_PASSTHRU   ,       // Y8:CR8:CB8 + Y8:CR8:CB8 => YCrCb  (4:2:2 + 4:2:2)
    NVVIODATAFORMAT_R10G10B10_TO_YCRCB422              ,       // R10:G10:B10             => YCrCb  (4:2:2)
    NVVIODATAFORMAT_R10G10B10_TO_YCRCB444              ,       // R10:G10:B10             => YCrCb  (4:4:4)
    NVVIODATAFORMAT_X12X12X12_444_PASSTHRU             ,       // X12:X12:X12             => XXX    (4:4:4)
    NVVIODATAFORMAT_X12X12X12_422_PASSTHRU             ,       // X12:X12:X12             => XXX    (4:2:2)
    NVVIODATAFORMAT_Y10CR10CB10_TO_YCRCB422            ,       // Y10:CR10:CB10           => YCrCb  (4:2:2)
    NVVIODATAFORMAT_Y8CR8CB8_TO_YCRCB422               ,       // Y8:CR8:CB8              => YCrCb  (4:2:2)
    NVVIODATAFORMAT_Y10CR8CB8A10_TO_YCRCBA4224         ,       // Y10:CR8:CB8:A10         => YCrCbA (4:2:2:4)
    NVVIODATAFORMAT_R10G10B10_TO_RGB444                ,       // R10:G10:B10             => RGB    (4:4:4)
    NVVIODATAFORMAT_R12G12B12_TO_YCRCB444              ,       // R12:G12:B12             => YCrCb  (4:4:4)
    NVVIODATAFORMAT_R12G12B12_TO_YCRCB422              ,       // R12:G12:B12             => YCrCb  (4:2:2)
}NVVIODATAFORMAT;

// Video output area
typedef enum _NVVIOOUTPUTAREA
{
    NVVIOOUTPUTAREA_FULLSIZE                           ,       // Output to entire video resolution (full size)
    NVVIOOUTPUTAREA_SAFEACTION                         ,       // Output to centered 90% of video resolution (safe action)
    NVVIOOUTPUTAREA_SAFETITLE                          ,       // Output to centered 80% of video resolution (safe title)
}NVVIOOUTPUTAREA;

// Synchronization source
typedef enum _NVVIOSYNCSOURCE
{
    NVVIOSYNCSOURCE_SDISYNC                            ,       // SDI Sync  (Digital input)
    NVVIOSYNCSOURCE_COMPSYNC                           ,       // COMP Sync (Composite input)
}NVVIOSYNCSOURCE;

// Composite synchronization type
typedef enum _NVVIOCOMPSYNCTYPE
{
    NVVIOCOMPSYNCTYPE_AUTO                             ,       // Auto-detect
    NVVIOCOMPSYNCTYPE_BILEVEL                          ,       // Bi-level signal
    NVVIOCOMPSYNCTYPE_TRILEVEL                         ,       // Tri-level signal
}NVVIOCOMPSYNCTYPE;

// Video input output status
typedef enum _NVVIOINPUTOUTPUTSTATUS
{
    NVINPUTOUTPUTSTATUS_OFF                            ,       // Not in use
    NVINPUTOUTPUTSTATUS_ERROR                          ,       // Error detected
    NVINPUTOUTPUTSTATUS_SDI_SD                         ,       // SDI (standard-definition)
    NVINPUTOUTPUTSTATUS_SDI_HD                         ,       // SDI (high-definition)
}NVVIOINPUTOUTPUTSTATUS;

// Synchronization input status
typedef enum _NVVIOSYNCSTATUS
{
    NVVIOSYNCSTATUS_OFF                                ,       // Sync not detected
    NVVIOSYNCSTATUS_ERROR                              ,       // Error detected
    NVVIOSYNCSTATUS_SYNCLOSS                           ,       // Genlock in use, format mismatch with output
    NVVIOSYNCSTATUS_COMPOSITE                          ,       // Composite sync
    NVVIOSYNCSTATUS_SDI_SD                             ,       // SDI sync (standard-definition)
    NVVIOSYNCSTATUS_SDI_HD                             ,       // SDI sync (high-definition)
}NVVIOSYNCSTATUS;

//Video Capture Status
typedef enum _NVVIOCAPTURESTATUS
{
    NVVIOSTATUS_STOPPED                                ,       // Sync not detected
    NVVIOSTATUS_RUNNING                                ,       // Error detected
    NVVIOSTATUS_ERROR                                  ,       // Genlock in use, format mismatch with output
}NVVIOCAPTURESTATUS;

//Video Capture Status
typedef enum _NVVIOSTATUSTYPE
{
    NVVIOSTATUSTYPE_IN                                 ,       // Input Status
    NVVIOSTATUSTYPE_OUT                                ,       // Output Status
}NVVIOSTATUSTYPE;

#define NVAPI_MAX_VIO_DEVICES                 8   // Assumption, maximum 4 SDI input and 4 SDI output cards supported on a system
#define NVAPI_MAX_VIO_JACKS                   4   // 4 physical jacks supported on each SDI input card.
#define NVAPI_MAX_VIO_CHANNELS_PER_JACK       2   // Each physical jack an on SDI input card can have
                                                  // two "channels" in the case of "3G" VideoFormats, as specified
                                                  // by SMPTE 425; for non-3G VideoFormats, only the first channel within
                                                  // a physical jack is valid
#define NVAPI_MAX_VIO_STREAMS                 4   // 4 Streams, 1 per physical jack
#define NVAPI_MIN_VIO_STREAMS                 1
#define NVAPI_MAX_VIO_LINKS_PER_STREAM        2   // SDI input supports a max of 2 links per stream
#define NVAPI_MAX_FRAMELOCK_MAPPING_MODES     20
#define NVAPI_GVI_MIN_RAW_CAPTURE_IMAGES      1   // Min number of capture images
#define NVAPI_GVI_MAX_RAW_CAPTURE_IMAGES      32  // Max number of capture images
#define NVAPI_GVI_DEFAULT_RAW_CAPTURE_IMAGES  5   // Default number of capture images

// Data Signal notification events. These need a event handler in RM.
// Register/Unregister and PopEvent NVAPI's are already available.

// Device configuration
typedef enum _NVVIOCONFIGTYPE
{
    NVVIOCONFIGTYPE_IN                                 ,       // Input Status
    NVVIOCONFIGTYPE_OUT                                ,       // Output Status
}NVVIOCONFIGTYPE;

typedef enum _NVVIOCOLORSPACE
{
    NVVIOCOLORSPACE_UNKNOWN,
    NVVIOCOLORSPACE_YCBCR,
    NVVIOCOLORSPACE_YCBCRA,
    NVVIOCOLORSPACE_YCBCRD,
    NVVIOCOLORSPACE_GBR,
    NVVIOCOLORSPACE_GBRA,
    NVVIOCOLORSPACE_GBRD,
} NVVIOCOLORSPACE;

// Component sampling
typedef enum _NVVIOCOMPONENTSAMPLING
{
    NVVIOCOMPONENTSAMPLING_UNKNOWN,
    NVVIOCOMPONENTSAMPLING_4444,
    NVVIOCOMPONENTSAMPLING_4224,
    NVVIOCOMPONENTSAMPLING_444,
    NVVIOCOMPONENTSAMPLING_422
} NVVIOCOMPONENTSAMPLING;

typedef enum _NVVIOBITSPERCOMPONENT
{
    NVVIOBITSPERCOMPONENT_UNKNOWN,
    NVVIOBITSPERCOMPONENT_8,
    NVVIOBITSPERCOMPONENT_10,
    NVVIOBITSPERCOMPONENT_12,
} NVVIOBITSPERCOMPONENT;

typedef enum _NVVIOLINKID
{
    NVVIOLINKID_UNKNOWN,
    NVVIOLINKID_A,
    NVVIOLINKID_B,
    NVVIOLINKID_C,
    NVVIOLINKID_D
} NVVIOLINKID;

//---------------------------------------------------------------------
// Structures
//---------------------------------------------------------------------

#define NVVIOCAPS_VIDOUT_SDI                0x00000001      // Supports Serial Digital Interface (SDI) output
#define NVVIOCAPS_SYNC_INTERNAL             0x00000100      // Supports Internal timing source
#define NVVIOCAPS_SYNC_GENLOCK              0x00000200      // Supports Genlock timing source
#define NVVIOCAPS_SYNCSRC_SDI               0x00001000      // Supports Serial Digital Interface (SDI) synchronization input
#define NVVIOCAPS_SYNCSRC_COMP              0x00002000      // Supports Composite synchronization input
#define NVVIOCAPS_OUTPUTMODE_DESKTOP        0x00010000      // Supports Desktop transparent mode
#define NVVIOCAPS_OUTPUTMODE_OPENGL         0x00020000      // Supports OpenGL application mode
#define NVVIOCAPS_VIDIN_SDI                 0x00100000      // Supports Serial Digital Interface (SDI) input

#define NVVIOCLASS_SDI                      0x00000001      // SDI-class interface: SDI output with two genlock inputs

// Device capabilities
typedef struct _NVVIOCAPS
{
    NvU32             version;                              // Structure version
    NvAPI_String      adapterName;                          // Graphics adapter name
    NvU32             adapterClass;                         // Graphics adapter classes (NVVIOCLASS_SDI mask)
    NvU32             adapterCaps;                          // Graphics adapter capabilities (NVVIOCAPS_* mask)
    NvU32             dipSwitch;                            // On-board DIP switch settings bits
    NvU32             dipSwitchReserved;                    // On-board DIP switch settings reserved bits
    NvU32             boardID;                              // Board ID
    struct                                                  //
    {                                                       // Driver version
        NvU32          majorVersion;                        // Major version
        NvU32          minorVersion;                        // Minor version
    } driver;                                               //
    struct                                                  //
    {                                                       // Firmware version
        NvU32          majorVersion;                        // Major version
        NvU32          minorVersion;                        // Minor version
    } firmWare;                                             //
    NVVIOOWNERID      ownerId;                              // Unique identifier for owner of video output (NVVIOOWNERID_INVALID if free running)
    NVVIOOWNERTYPE    ownerType;                            // Owner type (OpenGL application or Desktop mode)
} NVVIOCAPS;

#define NVVIOCAPS_VER   MAKE_NVAPI_VERSION(NVVIOCAPS,1)

// Input channel status
typedef struct _NVVIOCHANNELSTATUS
{
    NvU32                  smpte352;                         // 4-byte SMPTE 352 video payload identifier
    NVVIOSIGNALFORMAT      signalFormat;                     // Signal format
    NVVIOBITSPERCOMPONENT  bitsPerComponent;                 // Bits per component
    NVVIOCOMPONENTSAMPLING samplingFormat;                   // Sampling format
    NVVIOCOLORSPACE        colorSpace;                       // Color space
    NVVIOLINKID            linkID;                           // Link ID
} NVVIOCHANNELSTATUS;

// Input device status
typedef struct _NVVIOINPUTSTATUS
{
    NVVIOCHANNELSTATUS     vidIn[NVAPI_MAX_VIO_JACKS][NVAPI_MAX_VIO_CHANNELS_PER_JACK];     // Video input status per channel within a jack
    NVVIOCAPTURESTATUS     captureStatus;                  // status of video capture
} NVVIOINPUTSTATUS;

// Output device status
typedef struct _NVVIOOUTPUTSTATUS
{
    NVVIOINPUTOUTPUTSTATUS  vid1Out;                        // Video 1 output status
    NVVIOINPUTOUTPUTSTATUS  vid2Out;                        // Video 2 output status
    NVVIOSYNCSTATUS         sdiSyncIn;                      // SDI sync input status
    NVVIOSYNCSTATUS         compSyncIn;                     // Composite sync input status
    NvU32                   syncEnable;                     // Sync enable (TRUE if using syncSource)
    NVVIOSYNCSOURCE         syncSource;                     // Sync source
    NVVIOSIGNALFORMAT       syncFormat;                     // Sync format
    NvU32                   frameLockEnable;                // Framelock enable flag
    NvU32                   outputVideoLocked;              // Output locked status
    NvU32                   dataIntegrityCheckErrorCount;   // Data integrity check error count
    NvU32                   dataIntegrityCheckEnabled;      // Data integrity check status enabled
    NvU32                   dataIntegrityCheckFailed;       // Data integrity check status failed
    NvU32                   uSyncSourceLocked;              // genlocked to framelocked to ref signal
    NvU32                   uPowerOn;                       // TRUE: indicates there is sufficient power
} NVVIOOUTPUTSTATUS;

// Video device status.
typedef struct _NVVIOSTATUS
{
    NvU32                 version;                        // Structure version
    NVVIOSTATUSTYPE       nvvioStatusType;                // Input or Output status
    union
    {
        NVVIOINPUTSTATUS  inStatus;                       //  Input device status
        NVVIOOUTPUTSTATUS outStatus;                      //  Output device status
    }vioStatus;
} NVVIOSTATUS;

#define NVVIOSTATUS_VER   MAKE_NVAPI_VERSION(NVVIOSTATUS,1)

// Output region
typedef struct _NVVIOOUTPUTREGION
{
    NvU32              x;                                    // Horizontal origin in pixels
    NvU32              y;                                    // Vertical origin in pixels
    NvU32              width;                                // Width of region in pixels
    NvU32              height;                               // Height of region in pixels
} NVVIOOUTPUTREGION;

// Gamma ramp (8-bit index)
typedef struct _NVVIOGAMMARAMP8
{
    NvU16              uRed[256];                            // Red channel gamma ramp (8-bit index, 16-bit values)
    NvU16              uGreen[256];                          // Green channel gamma ramp (8-bit index, 16-bit values)
    NvU16              uBlue[256];                           // Blue channel gamma ramp (8-bit index, 16-bit values)
} NVVIOGAMMARAMP8;

// Gamma ramp (10-bit index)
typedef struct _NVVIOGAMMARAMP10
{
    NvU16              uRed[1024];                           // Red channel gamma ramp (10-bit index, 16-bit values)
    NvU16              uGreen[1024];                         // Green channel gamma ramp (10-bit index, 16-bit values)
    NvU16              uBlue[1024];                          // Blue channel gamma ramp (10-bit index, 16-bit values)
} NVVIOGAMMARAMP10;

// Sync delay
typedef struct _NVVIOSYNCDELAY
{
    NvU32              version;                              // Structure version
    NvU32              horizontalDelay;                      // Horizontal delay in pixels
    NvU32              verticalDelay;                        // Vertical delay in lines
} NVVIOSYNCDELAY;

#define NVVIOSYNCDELAY_VER   MAKE_NVAPI_VERSION(NVVIOSYNCDELAY,1)


// Video mode information
typedef struct _NVVIOVIDEOMODE
{
    NvU32                horizontalPixels;                   // Horizontal resolution (in pixels)
    NvU32                verticalLines;                      // Vertical resolution for frame (in lines)
    float                fFrameRate;                         // Frame rate
    NVVIOINTERLACEMODE   interlaceMode;                      // Interlace mode
    NVVIOVIDEOSTANDARD   videoStandard;                      // SMPTE standards format
    NVVIOVIDEOTYPE       videoType;                          // HD or SD signal classification
} NVVIOVIDEOMODE;

// Signal format details
typedef struct _NVVIOSIGNALFORMATDETAIL
{
    NVVIOSIGNALFORMAT    signalFormat;                       // Signal format enumerated value
    NVVIOVIDEOMODE       videoMode;                          // Video mode for signal format
}NVVIOSIGNALFORMATDETAIL;

// Buffer formats
#define NVVIOBUFFERFORMAT_R8G8B8                  0x00000001   // R8:G8:B8
#define NVVIOBUFFERFORMAT_R8G8B8Z24               0x00000002   // R8:G8:B8:Z24
#define NVVIOBUFFERFORMAT_R8G8B8A8                0x00000004   // R8:G8:B8:A8
#define NVVIOBUFFERFORMAT_R8G8B8A8Z24             0x00000008   // R8:G8:B8:A8:Z24
#define NVVIOBUFFERFORMAT_R16FPG16FPB16FP         0x00000010   // R16FP:G16FP:B16FP
#define NVVIOBUFFERFORMAT_R16FPG16FPB16FPZ24      0x00000020   // R16FP:G16FP:B16FP:Z24
#define NVVIOBUFFERFORMAT_R16FPG16FPB16FPA16FP    0x00000040   // R16FP:G16FP:B16FP:A16FP
#define NVVIOBUFFERFORMAT_R16FPG16FPB16FPA16FPZ24 0x00000080   // R16FP:G16FP:B16FP:A16FP:Z24

// Data format details
typedef struct _NVVIODATAFORMATDETAIL
{
    NVVIODATAFORMAT   dataFormat;                              // Data format enumerated value
    NvU32             vioCaps;                                 // Data format capabilities (NVVIOCAPS_* mask)
}NVVIODATAFORMATDETAIL;

// Colorspace conversion
typedef struct _NVVIOCOLORCONVERSION
{
    NvU32       version;                                    //  Structure version
    float       colorMatrix[3][3];                          //  Output[n] =
    float       colorOffset[3];                             //  Input[0] * colorMatrix[n][0] +
    float       colorScale[3];                              //  Input[1] * colorMatrix[n][1] +
                                                            //  Input[2] * colorMatrix[n][2] +
                                                            //  OutputRange * colorOffset[n]
                                                            //  where OutputRange is the standard magnitude of
                                                            //  Output[n][n] and colorMatrix and colorOffset
                                                            //  values are within the range -1.0 to +1.0
    NvU32      compositeSafe;                               //  compositeSafe constrains luminance range when using composite output
} NVVIOCOLORCONVERSION;

#define NVVIOCOLORCONVERSION_VER   MAKE_NVAPI_VERSION(NVVIOCOLORCONVERSION,1)

// Gamma correction
typedef struct _NVVIOGAMMACORRECTION
{
    NvU32            version;                               // Structure version
    NvU32            vioGammaCorrectionType;                // Gamma correction type (8-bit or 10-bit)
    union                                                   // Gamma correction:
    {
        NVVIOGAMMARAMP8  gammaRamp8;                        // Gamma ramp (8-bit index, 16-bit values)
        NVVIOGAMMARAMP10 gammaRamp10;                       // Gamma ramp (10-bit index, 16-bit values)
    }gammaRamp;
    float            fGammaValueR;                          // Red Gamma value within gamma ranges. 0.5 - 6.0
    float            fGammaValueG;                          // Green Gamma value within gamma ranges. 0.5 - 6.0
    float            fGammaValueB;                          // Blue Gamma value within gamma ranges. 0.5 - 6.0
} NVVIOGAMMACORRECTION;

#define NVVIOGAMMACORRECTION_VER   MAKE_NVAPI_VERSION(NVVIOGAMMACORRECTION,1)

#define MAX_NUM_COMPOSITE_RANGE      2                      // maximum number of ranges per channel

typedef struct _NVVIOCOMPOSITERANGE
{
    NvU32   uRange;
    NvU32   uEnabled;
    NvU32   uMin;
    NvU32   uMax;
} NVVIOCOMPOSITERANGE;


// Device configuration (fields masks indicating NVVIOCONFIG fields to use for NvAPI_VIO_GetConfig/NvAPI_VIO_SetConfig() )
#define NVVIOCONFIG_SIGNALFORMAT            0x00000001      // fields: signalFormat
#define NVVIOCONFIG_DATAFORMAT              0x00000002      // fields: dataFormat
#define NVVIOCONFIG_OUTPUTREGION            0x00000004      // fields: outputRegion
#define NVVIOCONFIG_OUTPUTAREA              0x00000008      // fields: outputArea
#define NVVIOCONFIG_COLORCONVERSION         0x00000010      // fields: colorConversion
#define NVVIOCONFIG_GAMMACORRECTION         0x00000020      // fields: gammaCorrection
#define NVVIOCONFIG_SYNCSOURCEENABLE        0x00000040      // fields: syncSource and syncEnable
#define NVVIOCONFIG_SYNCDELAY               0x00000080      // fields: syncDelay
#define NVVIOCONFIG_COMPOSITESYNCTYPE       0x00000100      // fields: compositeSyncType
#define NVVIOCONFIG_FRAMELOCKENABLE         0x00000200      // fields: EnableFramelock
#define NVVIOCONFIG_422FILTER               0x00000400      // fields: bEnable422Filter
#define NVVIOCONFIG_COMPOSITETERMINATE      0x00000800      // fields: bCompositeTerminate        //Not Supported on Quadro FX 4000 SDI
#define NVVIOCONFIG_DATAINTEGRITYCHECK      0x00001000      // fields: bEnableDataIntegrityCheck  //Not Supported on Quadro FX 4000 SDI
#define NVVIOCONFIG_CSCOVERRIDE             0x00002000      // fields: colorConversion override
#define NVVIOCONFIG_FLIPQUEUELENGTH         0x00004000      // fields: flipqueuelength control
#define NVVIOCONFIG_ANCTIMECODEGENERATION   0x00008000      // fields: bEnableANCTimeCodeGeneration
#define NVVIOCONFIG_COMPOSITE               0x00010000      // fields: bEnableComposite
#define NVVIOCONFIG_ALPHAKEYCOMPOSITE       0x00020000      // fields: bEnableAlphaKeyComposite
#define NVVIOCONFIG_COMPOSITE_Y             0x00040000      // fields: compRange
#define NVVIOCONFIG_COMPOSITE_CR            0x00080000      // fields: compRange
#define NVVIOCONFIG_COMPOSITE_CB            0x00100000      // fields: compRange
#define NVVIOCONFIG_FULL_COLOR_RANGE        0x00200000      // fields: bEnableFullColorRange
#define NVVIOCONFIG_RGB_DATA                0x00400000      // fields: bEnableRGBData
#define NVVIOCONFIG_RESERVED_SDIOUTPUTENABLE         0x00800000      // fields: bEnableSDIOutput
#define NVVIOCONFIG_STREAMS                 0x01000000      // fields: streams

// Don't forget to update NVVIOCONFIG_VALIDFIELDS in nvapi.spec when NVVIOCONFIG_ALLFIELDS changes.
#define NVVIOCONFIG_ALLFIELDS   ( NVVIOCONFIG_SIGNALFORMAT          | \
                                  NVVIOCONFIG_DATAFORMAT            | \
                                  NVVIOCONFIG_OUTPUTREGION          | \
                                  NVVIOCONFIG_OUTPUTAREA            | \
                                  NVVIOCONFIG_COLORCONVERSION       | \
                                  NVVIOCONFIG_GAMMACORRECTION       | \
                                  NVVIOCONFIG_SYNCSOURCEENABLE      | \
                                  NVVIOCONFIG_SYNCDELAY             | \
                                  NVVIOCONFIG_COMPOSITESYNCTYPE     | \
                                  NVVIOCONFIG_FRAMELOCKENABLE       | \
                                  NVVIOCONFIG_422FILTER             | \
                                  NVVIOCONFIG_COMPOSITETERMINATE    | \
                                  NVVIOCONFIG_DATAINTEGRITYCHECK    | \
                                  NVVIOCONFIG_CSCOVERRIDE           | \
                                  NVVIOCONFIG_FLIPQUEUELENGTH       | \
                                  NVVIOCONFIG_ANCTIMECODEGENERATION | \
                                  NVVIOCONFIG_COMPOSITE             | \
                                  NVVIOCONFIG_ALPHAKEYCOMPOSITE     | \
                                  NVVIOCONFIG_COMPOSITE_Y           | \
                                  NVVIOCONFIG_COMPOSITE_CR          | \
                                  NVVIOCONFIG_COMPOSITE_CB          | \
                                  NVVIOCONFIG_FULL_COLOR_RANGE      | \
                                  NVVIOCONFIG_RGB_DATA              | \
                                  NVVIOCONFIG_RESERVED_SDIOUTPUTENABLE | \
                                  NVVIOCONFIG_STREAMS)

#define NVVIOCONFIG_VALIDFIELDS  ( NVVIOCONFIG_SIGNALFORMAT          | \
                                   NVVIOCONFIG_DATAFORMAT            | \
                                   NVVIOCONFIG_OUTPUTREGION          | \
                                   NVVIOCONFIG_OUTPUTAREA            | \
                                   NVVIOCONFIG_COLORCONVERSION       | \
                                   NVVIOCONFIG_GAMMACORRECTION       | \
                                   NVVIOCONFIG_SYNCSOURCEENABLE      | \
                                   NVVIOCONFIG_SYNCDELAY             | \
                                   NVVIOCONFIG_COMPOSITESYNCTYPE     | \
                                   NVVIOCONFIG_FRAMELOCKENABLE       | \
                                   NVVIOCONFIG_RESERVED_SDIOUTPUTENABLE | \
                                   NVVIOCONFIG_422FILTER             | \
                                   NVVIOCONFIG_COMPOSITETERMINATE    | \
                                   NVVIOCONFIG_DATAINTEGRITYCHECK    | \
                                   NVVIOCONFIG_CSCOVERRIDE           | \
                                   NVVIOCONFIG_FLIPQUEUELENGTH       | \
                                   NVVIOCONFIG_ANCTIMECODEGENERATION | \
                                   NVVIOCONFIG_COMPOSITE             | \
                                   NVVIOCONFIG_ALPHAKEYCOMPOSITE     | \
                                   NVVIOCONFIG_COMPOSITE_Y           | \
                                   NVVIOCONFIG_COMPOSITE_CR          | \
                                   NVVIOCONFIG_COMPOSITE_CB          | \
                                   NVVIOCONFIG_FULL_COLOR_RANGE      | \
                                   NVVIOCONFIG_RGB_DATA              | \
                                   NVVIOCONFIG_RESERVED_SDIOUTPUTENABLE | \
                                   NVVIOCONFIG_STREAMS)

#define NVVIOCONFIG_DRIVERFIELDS ( NVVIOCONFIG_OUTPUTREGION          | \
                                   NVVIOCONFIG_OUTPUTAREA            | \
                                   NVVIOCONFIG_COLORCONVERSION       | \
                                   NVVIOCONFIG_FLIPQUEUELENGTH)

#define NVVIOCONFIG_GAMMAFIELDS  ( NVVIOCONFIG_GAMMACORRECTION       )

#define NVVIOCONFIG_RMCTRLFIELDS ( NVVIOCONFIG_SIGNALFORMAT          | \
                                   NVVIOCONFIG_DATAFORMAT            | \
                                   NVVIOCONFIG_SYNCSOURCEENABLE      | \
                                   NVVIOCONFIG_COMPOSITESYNCTYPE     | \
                                   NVVIOCONFIG_FRAMELOCKENABLE       | \
                                   NVVIOCONFIG_422FILTER             | \
                                   NVVIOCONFIG_COMPOSITETERMINATE    | \
                                   NVVIOCONFIG_DATAINTEGRITYCHECK    | \
                                   NVVIOCONFIG_COMPOSITE             | \
                                   NVVIOCONFIG_ALPHAKEYCOMPOSITE     | \
                                   NVVIOCONFIG_COMPOSITE_Y           | \
                                   NVVIOCONFIG_COMPOSITE_CR          | \
                                   NVVIOCONFIG_COMPOSITE_CB)

#define NVVIOCONFIG_RMSKEWFIELDS ( NVVIOCONFIG_SYNCDELAY             )

#define NVVIOCONFIG_ALLOWSDIRUNNING_FIELDS ( NVVIOCONFIG_DATAINTEGRITYCHECK     | \
                                             NVVIOCONFIG_SYNCDELAY              | \
                                             NVVIOCONFIG_CSCOVERRIDE            | \
                                             NVVIOCONFIG_ANCTIMECODEGENERATION  | \
                                             NVVIOCONFIG_COMPOSITE              | \
                                             NVVIOCONFIG_ALPHAKEYCOMPOSITE      | \
                                             NVVIOCONFIG_COMPOSITE_Y            | \
                                             NVVIOCONFIG_COMPOSITE_CR           | \
                                             NVVIOCONFIG_COMPOSITE_CB)

 #define NVVIOCONFIG_RMMODESET_FIELDS ( NVVIOCONFIG_SIGNALFORMAT         | \
                                        NVVIOCONFIG_DATAFORMAT           | \
                                        NVVIOCONFIG_SYNCSOURCEENABLE     | \
                                        NVVIOCONFIG_FRAMELOCKENABLE      | \
                                        NVVIOCONFIG_COMPOSITESYNCTYPE )


// Output device configuration
// No members can be deleted from below structure. Only add new members at the
// end of the structure
typedef struct _NVVIOOUTPUTCONFIG
{
    NVVIOSIGNALFORMAT    signalFormat;                         // Signal format for video output
    NVVIODATAFORMAT      dataFormat;                           // Data format for video output
    NVVIOOUTPUTREGION    outputRegion;                         // Region for video output (Desktop mode)
    NVVIOOUTPUTAREA      outputArea;                           // Usable resolution for video output (safe area)
    NVVIOCOLORCONVERSION colorConversion;                      // Color conversion.
    NVVIOGAMMACORRECTION gammaCorrection;
    NvU32                syncEnable;                           // Sync enable (TRUE to use syncSource)
    NVVIOSYNCSOURCE      syncSource;                           // Sync source
    NVVIOSYNCDELAY       syncDelay;                            // Sync delay
    NVVIOCOMPSYNCTYPE    compositeSyncType;                    // Composite sync type
    NvU32                frameLockEnable;                      // Flag indicating whether framelock was on/off
    NvU32                psfSignalFormat;                      // Inidcates whether contained format is PSF Signal format
    NvU32                enable422Filter;                      // Enables/Disables 4:2:2 filter
    NvU32                compositeTerminate;                   // Composite termination
    NvU32                enableDataIntegrityCheck;             // Enable data integrity check: true - enable, false - disable
    NvU32                cscOverride;                          // Use provided CSC color matrix to overwrite
    NvU32                flipQueueLength;                      // Number of buffers used for the internal flipqueue
    NvU32                enableANCTimeCodeGeneration;          // Enable SDI ANC time code generation
    NvU32                enableComposite;                      // Enable composite
    NvU32                enableAlphaKeyComposite;              // Enable Alpha key composite
    NVVIOCOMPOSITERANGE  compRange;                            // Composite ranges
    NvU8                 reservedData[256];                    // Inicates last stored SDI output state TRUE-ON / FALSE-OFF
    NvU32                enableFullColorRange;                 // Flag indicating Full Color Range
    NvU32                enableRGBData;                        // Indicates data is in RGB format
} NVVIOOUTPUTCONFIG;

// Stream configuration
typedef struct _NVVIOSTREAM
{
    NvU32                   bitsPerComponent;                     // Bits per component
    NVVIOCOMPONENTSAMPLING  sampling;                             // Sampling
    NvU32                   expansionEnable;                      // Enable/disable 4:2:2->4:4:4 expansion
    NvU32                   numLinks;                             // Number of active links
    struct
    {
        NvU32               jack;                                 // This stream's link[i] will use the specified (0-based) channel within the
        NvU32               channel;                              // specified (0-based) jack
    } links[NVAPI_MAX_VIO_LINKS_PER_STREAM];
} NVVIOSTREAM;

// Input device configuration
typedef struct _NVVIOINPUTCONFIG
{
    NvU32                numRawCaptureImages;                  // numRawCaptureImages is the number of frames to keep in the capture queue.
                                                               // must be between NVAPI_GVI_MIN_RAW_CAPTURE_IMAGES and NVAPI_GVI_MAX_RAW_CAPTURE_IMAGES,
    NVVIOSIGNALFORMAT    signalFormat;                         // Signal format.
                                                               // Please note that both numRawCaptureImages and signalFormat should be set together.
    NvU32                numStreams;                           // Number of active streams.
    NVVIOSTREAM          streams[NVAPI_MAX_VIO_STREAMS];       // Stream configurations
    NvU32                bTestMode;                            // This attribute controls the GVI test mode. Possible values 0/1.
                                                               // When testmode enabled, the GVI device will generate fake data as quickly as possible.
} NVVIOINPUTCONFIG;

typedef struct _NVVIOCONFIG
{
    NvU32                version;                              // Structure version
    NvU32                fields;                               // Caller sets to NVVIOCONFIG_* mask for fields to use
    NVVIOCONFIGTYPE      nvvioConfigType;                      // Input or Output configuration
    union
    {
        NVVIOINPUTCONFIG  inConfig;                            //  Input device configuration
        NVVIOOUTPUTCONFIG outConfig;                           //  Output device configuration
    }vioConfig;
} NVVIOCONFIG;

#define NVVIOCONFIG_VER   MAKE_NVAPI_VERSION(NVVIOCONFIG,1)

typedef struct
{
    NvPhysicalGpuHandle                 hPhysicalGpu;                   //handle to Physical GPU (This could be NULL for GVI device if its not binded)
    NvVioHandle                         hVioHandle;                     //handle to SDI Input/Output device
    NvU32                               vioId;                          //device Id of SDI Input/Output device
    NvU32                               outputId;                       //deviceMask of the SDI display connected to GVO device.
                                                                        //outputId will be 0 for GVI device.
} NVVIOTOPOLOGYTARGET;

typedef struct _NV_VIO_TOPOLOGY
{
    NvU32                       version;
    NvU32                       vioTotalDeviceCount;                    //How many vio targets are valid
    NVVIOTOPOLOGYTARGET         vioTarget[NVAPI_MAX_VIO_DEVICES];       //Array of vio targets
}NV_VIO_TOPOLOGY, NVVIOTOPOLOGY;

#define NV_VIO_TOPOLOGY_VER  MAKE_NVAPI_VERSION(NV_VIO_TOPOLOGY,1)
#define NVVIOTOPOLOGY_VER    MAKE_NVAPI_VERSION(NVVIOTOPOLOGY,1)

//---------------------------------------------------------------------
// Function:    NvAPI_VIO_GetCapabilities
//
// Description: Determine graphics adapter video I/O capabilities.
//
//  SUPPORTED OS: Windows XP and higher
//
// Parameters:  NvVioHandle[IN] - The caller provides the SDI device handle as input.
//              pAdapterCaps[OUT] - Pointer to receive capabilities
//
// Returns:     NVAPI_OK                          - Success
//              NVAPI_API_NOT_INTIALIZED          - NVAPI Not Initialized
//              NVAPI_INVALID_ARGUMENT            - Arguments passed to API are not valid
//              NVAPI_INCOMPATIBLE_STRUCT_VERSION - NVVIOCAPS struct version used by the app is not compatible
//              NVAPI_NOT_SUPPORTED                - Video I/O not supported
//              NVAPI_ERROR                       - NVAPI Random errors
//---------------------------------------------------------------------
NVAPI_INTERFACE NvAPI_VIO_GetCapabilities(NvVioHandle     hVioHandle,
                                          NVVIOCAPS       *pAdapterCaps);

//---------------------------------------------------------------------
// Function:    NvAPI_VIO_Open
//
// Description: Open graphics adapter for video I/O operations
//              using the OpenGL application interface.  Read operations
//              are permitted in this mode by multiple clients, but Write
//              operations are application exclusive.
//
//  SUPPORTED OS: Windows XP and higher
//
// Parameters:  NvVioHandle[IN]     - The caller provides the SDI output device handle as input.
//              vioClass[IN]        - Class interface (NVVIOCLASS_* value)
//              ownerType[IN]       - user should specify the ownerType ( NVVIOOWNERTYPE_APPLICATION or NVVIOOWNERTYPE_DESKTOP)
//
// Returns:     NVAPI_OK                          - Success
//              NVAPI_API_NOT_INTIALIZED          - NVAPI Not Initialized
//              NVAPI_INVALID_ARGUMENT            - Arguments passed to API are not valid
//              NVAPI_NOT_SUPPORTED                - Video I/O not supported
//              NVAPI_ERROR                       - NVAPI Random errors
//              NVAPI_DEVICE_BUSY                 - Access denied for requested access
//---------------------------------------------------------------------
NVAPI_INTERFACE NvAPI_VIO_Open(NvVioHandle       hVioHandle,
                               NvU32             vioClass,
                               NVVIOOWNERTYPE    ownerType);

//---------------------------------------------------------------------
// Function:    NvAPI_VIO_Close
//
// Description: Closes graphics adapter for Graphics to Video operations
//              using the OpenGL application interface.  Closing an
//              OpenGL handle releases the device.
//
//  SUPPORTED OS: Windows XP and higher
//
// Parameters:  NvVioHandle[IN]  - The caller provides the SDI output device handle as input.
//              bRelease         - boolean value to decide on keeping or releasing ownership
//
// Returns:     NVAPI_OK                          - Success
//              NVAPI_API_NOT_INTIALIZED          - NVAPI Not Initialized
//              NVAPI_INVALID_ARGUMENT            - Arguments passed to API are not valid
//              NVAPI_NOT_SUPPORTED                - Video I/O not supported
//              NVAPI_ERROR                       - NVAPI Random errors
//              NVAPI_DEVICE_BUSY                 - Access denied for requested access
//---------------------------------------------------------------------
NVAPI_INTERFACE NvAPI_VIO_Close(NvVioHandle       hVioHandle,
                                NvU32             bRelease);

//---------------------------------------------------------------------
// Function:    NvAPI_VIO_Status
//
// Description: Get Video I/O LED status.
//
//  SUPPORTED OS: Windows XP and higher
//
// Parameters:  NvVioHandle[IN] - The caller provides the SDI device handle as input.
//              pStatus(OUT)   - returns pointer to the NVVIOSTATUS
//
// Returns:     NVAPI_OK                          - Success
//              NVAPI_API_NOT_INTIALIZED          - NVAPI Not Initialized
//              NVAPI_INVALID_ARGUMENT            - Arguments passed to API are not valid
//              NVAPI_INCOMPATIBLE_STRUCT_VERSION - Invalid structure version
//              NVAPI_NOT_SUPPORTED               - Video I/O not supported
//              NVAPI_ERROR                       - NVAPI Random errors
//---------------------------------------------------------------------
NVAPI_INTERFACE NvAPI_VIO_Status(NvVioHandle     hVioHandle,
                                 NVVIOSTATUS     *pStatus);

//---------------------------------------------------------------------
// Function:    NvAPI_VIO_SyncFormatDetect
//
// Description: Detects Video I/O incoming sync video format.
//
//  SUPPORTED OS: Windows XP and higher
//
// Parameters:  NvVioHandle[IN] - The caller provides the SDI device handle as input.
//              pWait(OUT)     - Pointer to receive milliseconds to wait
//                               before VIOStatus will return detected
//                               syncFormat.
//
// Returns:     NVAPI_OK                          - Success
//              NVAPI_API_NOT_INTIALIZED          - NVAPI Not Initialized
//              NVAPI_INVALID_ARGUMENT            - Arguments passed to API are not valid
//              NVAPI_NOT_SUPPORTED               - Video I/O not supported
//              NVAPI_ERROR                       - NVAPI Random errors
//---------------------------------------------------------------------
NVAPI_INTERFACE NvAPI_VIO_SyncFormatDetect(NvVioHandle hVioHandle,
                                           NvU32       *pWait);

//---------------------------------------------------------------------
// Function:    NvAPI_VIO_GetConfig
//
// Description: Get Graphics to Video configuration.
//
//  SUPPORTED OS: Windows XP and higher
//
// Parameters:  NvVioHandle[IN] - The caller provides the SDI device handle as input.
//              pConfig(OUT)    - Pointer to Graphics to Video configuration
//
// Returns:     NVAPI_OK                          - Success
//              NVAPI_API_NOT_INTIALIZED          - NVAPI Not Initialized
//              NVAPI_INVALID_ARGUMENT            - Arguments passed to API are not valid
//              NVAPI_INCOMPATIBLE_STRUCT_VERSION - Invalid structure version
//              NVAPI_NOT_SUPPORTED               - Video I/O not supported
//              NVAPI_ERROR                       - NVAPI Random errors
//---------------------------------------------------------------------
NVAPI_INTERFACE NvAPI_VIO_GetConfig(NvVioHandle        hVioHandle,
                                    NVVIOCONFIG        *pConfig);

//---------------------------------------------------------------------
// Function:    NvAPI_VIO_SetConfig
//
// Description: Set Graphics to Video configuration.
//
//  SUPPORTED OS: Windows XP and higher
//
// Parameters:  NvVioHandle[IN]     - The caller provides the SDI device handle as input.
//              pConfig(IN)         - Pointer to Graphics to Video config
//
// Returns:     NVAPI_OK                          - Success
//              NVAPI_API_NOT_INTIALIZED          - NVAPI Not Initialized
//              NVAPI_INVALID_ARGUMENT            - Arguments passed to API are not valid
//              NVAPI_INCOMPATIBLE_STRUCT_VERSION - Stucture version invalid
//              NVAPI_NOT_SUPPORTED               - Video I/O not supported
//              NVAPI_ERROR                       - NVAPI Random errors
//              NVAPI_DEVICE_BUSY                 - Access denied for requested access
//---------------------------------------------------------------------
NVAPI_INTERFACE NvAPI_VIO_SetConfig(NvVioHandle            hVioHandle,
                                    const NVVIOCONFIG      *pConfig);

//---------------------------------------------------------------------
// Function:    NvAPI_VIO_SetCSC
//
// Description: Set colorspace conversion parameters.
//
//  SUPPORTED OS: Windows XP and higher
//
// Parameters:  NvVioHandle[IN]     - The caller provides the SDI device handle as input.
//              pCSC(IN)            - Pointer to CSC parameters
//
// Returns:     NVAPI_OK                          - Success
//              NVAPI_API_NOT_INTIALIZED          - NVAPI Not Initialized
//              NVAPI_INVALID_ARGUMENT            - Arguments passed to API are not valid
//              NVAPI_INCOMPATIBLE_STRUCT_VERSION - Stucture version invalid
//              NVAPI_NOT_SUPPORTED               - Video I/O not supported
//              NVAPI_ERROR                       - NVAPI Random errors
//              NVAPI_DEVICE_BUSY                 - Access denied for requested access
//---------------------------------------------------------------------
NVAPI_INTERFACE NvAPI_VIO_SetCSC(NvVioHandle           hVioHandle,
                                 NVVIOCOLORCONVERSION  *pCSC);

//---------------------------------------------------------------------
// Function:    NvAPI_VIO_GetCSC
//
// Description: Get colorspace conversion parameters.
//
//  SUPPORTED OS: Windows XP and higher
//
// Parameters:  NvVioHandle[IN]     - The caller provides the SDI device handle as input.
//              pCSC(OUT)           - Pointer to CSC parameters
//
// Returns:     NVAPI_OK                          - Success
//              NVAPI_API_NOT_INTIALIZED          - NVAPI Not Initialized
//              NVAPI_INVALID_ARGUMENT            - Arguments passed to API are not valid
//              NVAPI_INCOMPATIBLE_STRUCT_VERSION - Stucture version invalid
//              NVAPI_NOT_SUPPORTED               - Video I/O not supported
//              NVAPI_ERROR                       - NVAPI Random errors
//---------------------------------------------------------------------
NVAPI_INTERFACE NvAPI_VIO_GetCSC(NvVioHandle           hVioHandle,
                                 NVVIOCOLORCONVERSION  *pCSC);

//---------------------------------------------------------------------
// Function:    NvAPI_VIO_SetGamma
//
// Description: Set gamma conversion parameters.
//
//  SUPPORTED OS: Windows XP and higher
//
// Parameters:  NvVioHandle[IN]     - The caller provides the SDI device handle as input.
//              pGamma(IN)          - Pointer to gamma parameters
//
// Returns:     NVAPI_OK                          - Success
//              NVAPI_API_NOT_INTIALIZED          - NVAPI Not Initialized
//              NVAPI_INVALID_ARGUMENT            - Arguments passed to API are not valid
//              NVAPI_INCOMPATIBLE_STRUCT_VERSION - Stucture version invalid
//              NVAPI_NOT_SUPPORTED               - Video I/O not supported
//              NVAPI_ERROR                       - NVAPI Random errors
//              NVAPI_DEVICE_BUSY                 - Access denied for requested access
//---------------------------------------------------------------------
NVAPI_INTERFACE NvAPI_VIO_SetGamma(NvVioHandle           hVioHandle,
                                   NVVIOGAMMACORRECTION  *pGamma);

//---------------------------------------------------------------------
// Function:    NvAPI_VIO_GetGamma
//
// Description: Get gamma conversion parameters.
//
//  SUPPORTED OS: Windows XP and higher
//
// Parameters:  NvVioHandle[IN]     - The caller provides the SDI device handle as input.
//              pGamma(OUT)         - Pointer to gamma parameters
//
// Returns:     NVAPI_OK                          - Success
//              NVAPI_API_NOT_INTIALIZED          - NVAPI Not Initialized
//              NVAPI_INVALID_ARGUMENT            - Arguments passed to API are not valid
//              NVAPI_INCOMPATIBLE_STRUCT_VERSION - Stucture version invalid
//              NVAPI_NOT_SUPPORTED               - Video I/O not supported
//              NVAPI_ERROR                       - NVAPI Random errors
//---------------------------------------------------------------------
NVAPI_INTERFACE NvAPI_VIO_GetGamma(NvVioHandle           hVioHandle,
                                   NVVIOGAMMACORRECTION* pGamma);

//---------------------------------------------------------------------
// Function:    NvAPI_VIO_SetSyncDelay
//
// Description: Set sync delay parameters.
//
//  SUPPORTED OS: Windows XP and higher
//
// Parameters:  NvVioHandle[IN] - The caller provides the SDI device handle as input.
//              pSyncDelay(IN)  - const Pointer to sync delay parameters
//
// Returns:     NVAPI_OK                          - Success
//              NVAPI_API_NOT_INTIALIZED          - NVAPI Not Initialized
//              NVAPI_INVALID_ARGUMENT            - Arguments passed to API are not valid
//              NVAPI_INCOMPATIBLE_STRUCT_VERSION - Stucture version invalid
//              NVAPI_ERROR                       - NVAPI Random errors
//              NVAPI_DEVICE_BUSY                 - Access denied for requested access
//---------------------------------------------------------------------
NVAPI_INTERFACE NvAPI_VIO_SetSyncDelay(NvVioHandle            hVioHandle,
                                       const NVVIOSYNCDELAY   *pSyncDelay);

//---------------------------------------------------------------------
// Function:    NvAPI_VIO_GetSyncDelay
//
// Description: Get sync delay parameters.
//
//  SUPPORTED OS: Windows XP and higher
//
// Parameters:  NvVioHandle[IN]     - The caller provides the SDI device handle as input.
//              pSyncDelay(OUT)     - Pointer to sync delay parameters
//
// Returns:     NVAPI_OK                          - Success
//              NVAPI_API_NOT_INTIALIZED          - NVAPI Not Initialized
//              NVAPI_INVALID_ARGUMENT            - Arguments passed to API are not valid
//              NVAPI_INCOMPATIBLE_STRUCT_VERSION - Stucture version invalid
//              NVAPI_ERROR                       - NVAPI Random errors
//---------------------------------------------------------------------
NVAPI_INTERFACE NvAPI_VIO_GetSyncDelay(NvVioHandle      hVioHandle,
                                       NVVIOSYNCDELAY   *pSyncDelay);

//---------------------------------------------------------------------
// Function:    NvAPI_VIO_IsRunning
//
// Description: Determine if Video I/O is running.
//
//  SUPPORTED OS: Windows XP and higher
//
// Parameters:  NvVioHandle[IN]            - The caller provides the SDI device handle as input.
//
// Returns:     NVAPI_DRIVER_RUNNING       - Video I/O running
//              NVAPI_DRIVER_NOTRUNNING    - Video I/O not running
//---------------------------------------------------------------------
NVAPI_INTERFACE NvAPI_VIO_IsRunning(NvVioHandle   hVioHandle);

//---------------------------------------------------------------------
// Function:    NvAPI_VIO_Start
//
// Description: Start Video I/O.
//              This API should be called for NVVIOOWNERTYPE_DESKTOP only and will not work for OGL applications.
//
//  SUPPORTED OS: Windows XP and higher
//
// Parameters:  NvVioHandle[IN]    - The caller provides the SDI device handle as input.
//
// Returns:     NVAPI_OK                          - Success
//              NVAPI_API_NOT_INTIALIZED          - NVAPI Not Initialized
//              NVAPI_INVALID_ARGUMENT            - Arguments passed to API are not valid
//              NVAPI_NOT_SUPPORTED               - Video I/O not supported
//              NVAPI_ERROR                       - NVAPI Random errors
//              NVAPI_DEVICE_BUSY                 - Access denied for requested access
//---------------------------------------------------------------------
NVAPI_INTERFACE NvAPI_VIO_Start(NvVioHandle     hVioHandle);

//---------------------------------------------------------------------
// Function:    NvAPI_VIO_Stop
//
// Description: Stop Video I/O.
//              This API should be called for NVVIOOWNERTYPE_DESKTOP only and will not work for OGL applications.
//
//  SUPPORTED OS: Windows XP and higher
//
// Parameters:  NvVioHandle[IN]    - The caller provides the SDI device handle as input.
//
// Returns:     NVAPI_OK                          - Success
//              NVAPI_API_NOT_INTIALIZED          - NVAPI Not Initialized
//              NVAPI_INVALID_ARGUMENT            - Arguments passed to API are not valid
//              NVAPI_NOT_SUPPORTED               - Video I/O not supported
//              NVAPI_ERROR                       - NVAPI Random errors
//              NVAPI_DEVICE_BUSY                 - Access denied for requested access
//---------------------------------------------------------------------
NVAPI_INTERFACE NvAPI_VIO_Stop(NvVioHandle     hVioHandle);


//---------------------------------------------------------------------
// Function:    NvAPI_VIO_IsFrameLockModeCompatible
//
// Description: Checkes whether modes are compatible in framelock mode
//
//  SUPPORTED OS: Windows XP and higher
//
// Parameters:  NvVioHandle[IN]         - The caller provides the SDI device handle as input.
//              srcEnumIndex(IN)        - Source Enumeration index
//              destEnumIndex(IN)       - Destination Enumeration index
//              pbCompatible(OUT)       - Pointer to receive compatability
//
// Returns:     NVAPI_OK                          - Success
//              NVAPI_API_NOT_INTIALIZED          - NVAPI Not Initialized
//              NVAPI_INVALID_ARGUMENT            - Arguments passed to API are not valid
//              NVAPI_NOT_SUPPORTED               - Video I/O not supported
//              NVAPI_ERROR                       - NVAPI Random errors
//---------------------------------------------------------------------
NVAPI_INTERFACE NvAPI_VIO_IsFrameLockModeCompatible(NvVioHandle              hVioHandle,
                                                    NvU32                    srcEnumIndex,
                                                    NvU32                    destEnumIndex,
                                                    NvU32*                   pbCompatible);

//---------------------------------------------------------------------
// Function:    NvAPI_VIO_EnumDevices
//
// Description: Enumerate all VIO devices connected to the system.
//
//  SUPPORTED OS: Windows XP and higher
//
// Parameters:  NvVioHandle[OUT]                  - User passes the pointer of NvVioHandle[] array to get handles to all the connected vio devices.
//              vioDeviceCount[OUT]               - User gets total number of VIO devices connected to the system.
//
// Returns:     NVAPI_OK                          - Success
//              NVAPI_API_NOT_INTIALIZED          - NVAPI Not Initialized
//              NVAPI_INVALID_ARGUMENT            - Arguments passed to API are not valid
//              NVAPI_ERROR                       - NVAPI Random errors
//              NVAPI_NVIDIA_DEVICE_NOT_FOUND     - No SDI Device found
//---------------------------------------------------------------------
NVAPI_INTERFACE NvAPI_VIO_EnumDevices(NvVioHandle       hVioHandle[NVAPI_MAX_VIO_DEVICES],
                                      NvU32             *vioDeviceCount);


//---------------------------------------------------------------------
// Function:    NvAPI_VIO_QueryTopology
//
// Description: Enumerate all valid SDI topologies
//
//  SUPPORTED OS: Windows XP and higher
//
// Parameters:  pNvVIOTopology[OUT]       - User passes the pointer to NVVIOTOPOLOGY to fetch all valid SDI Topologies.
//
// Returns:     NVAPI_OK                          - Success
//              NVAPI_API_NOT_INTIALIZED          - NVAPI Not Initialized
//              NVAPI_INVALID_ARGUMENT            - Arguments passed to API are not valid
//              NVAPI_INCOMPATIBLE_STRUCT_VERSION - Invalid structure version
//              NVAPI_ERROR                       - NVAPI Random errors
//---------------------------------------------------------------------
NVAPI_INTERFACE NvAPI_VIO_QueryTopology(NV_VIO_TOPOLOGY   *pNvVIOTopology);


//---------------------------------------------------------------------
// Function:    NvAPI_VIO_EnumSignalFormats
//
// Description: Enumerate signal formats supported by Video I/O.
//
//  SUPPORTED OS: Windows XP and higher
//
// Parameters:  NvVioHandle[IN]          - The caller provides the SDI device handle as input.
//              enumIndex(IN)            - Enumeration index
//              pSignalFormatDetail(OUT) - Pointer to receive detail or NULL
//
// Returns:     NVAPI_OK                 - Success
//              NVAPI_API_NOT_INTIALIZED - NVAPI not initialized
//              NVAPI_INVALID_ARGUMENT   - Invalid argument passed
//              NVAPI_END_ENUMERATION    - No more signal formats to enumerate
//---------------------------------------------------------------------
NVAPI_INTERFACE NvAPI_VIO_EnumSignalFormats(NvVioHandle              hVioHandle,
                                            NvU32                    enumIndex,
                                            NVVIOSIGNALFORMATDETAIL  *pSignalFormatDetail);

//---------------------------------------------------------------------
// Function:    NvAPI_VIO_EnumDataFormats
//
// Description: Enumerate data formats supported by Video I/O.
//
//  SUPPORTED OS: Windows XP and higher
//
// Parameters:  NvVioHandle[IN]        - The caller provides the SDI device handle as input.
//              enumIndex(IN)          - Enumeration index
//              pDataFormatDetail(OUT) - Pointer to receive detail or NULL
//
// Returns:     NVAPI_OK               - Success
//              NVAPI_END_ENUMERATION  - No more data formats to enumerate
//              NVAPI_NOT_SUPPORTED    - Unsupported NVVIODATAFORMAT_ enumeration
//---------------------------------------------------------------------
NVAPI_INTERFACE NvAPI_VIO_EnumDataFormats(NvVioHandle            hVioHandle,
                                          NvU32                  enumIndex,
                                          NVVIODATAFORMATDETAIL  *pDataFormatDetail);


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


//
// NV_GET_SCALING_CAPS
//
// Interface structure used in NvAPI_GetScalingCaps call.
//
// This NvAPI_GetScalingCaps returns scaling capability info for the specified display device
//


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


///////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME:   NvAPI_SYS_GetDisplayIdFromGpuAndOutputId
//
// DESCRIPTION:     Converts a Physical GPU handle and an output Id to a
//                  displayId
//
//  SUPPORTED OS: Windows XP and higher
//
// PARAMETERS:      hPhysicalGpu(IN)  - Handle to the physical GPU
//                  outputId(IN)      - Connected display output Id on the
//                                      target GPU, must only have one bit
//                                      set.
//                  displayId(OUT)    - Pointer to an NvU32 which will contain
//                                      the display ID
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_API_NOT_INTIALIZED - NVAPI not initialized
//                  NVAPI_ERROR - miscellaneous error occurred
//                  NVAPI_INVALID_ARGUMENT - Invalid input parameter.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_SYS_GetDisplayIdFromGpuAndOutputId(NvPhysicalGpuHandle hPhysicalGpu, NvU32 outputId, NvU32* displayId);

///////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME:   NvAPI_SYS_GetGpuAndOutputIdFromDisplayId
//
// DESCRIPTION:     Converts a displayId to a Physical GPU handle and an outputId
//
//  SUPPORTED OS: Windows XP and higher
//
// PARAMETERS:      displayId(IN)     - Display ID of display to retrieve
//                                      GPU and outputId for
//                  hPhysicalGpu(OUT) - Handle to the physical GPU
//                  outputId(OUT)     - Connected display output Id on the
//                                      target GPU, will only have one bit
//                                      set.
//
// RETURN STATUS:
//                  NVAPI_OK - completed request
//                  NVAPI_API_NOT_INTIALIZED - NVAPI not initialized
//                  NVAPI_ID_OUT_OF_RANGE - The DisplayId corresponds to a
//                                          display which is not within the
//                                          normal outputId range.
//                  NVAPI_ERROR - miscellaneous error occurred
//                  NVAPI_INVALID_ARGUMENT - Invalid input parameter.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_SYS_GetGpuAndOutputIdFromDisplayId(NvU32 displayId, NvPhysicalGpuHandle *hPhysicalGpu, NvU32 *outputId);

//  SUPPORTED OS: Windows XP and higher
///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_DISP_GetDisplayIdByDisplayName
//
// DESCRIPTION:     This API retrieves the Display Id of a given display by
//                  display name. The display must be active to retrieve the
//                  displayId. In the case of clone mode or Surround gaming,
//                  the primary or top-left display will be returned.
//
// PARAMETERS:      displayName(IN):  Name of display (Eg: "\\DISPLAY1" to
//                                    retrieve the displayId for.
//                  displayId(OUT):   Display ID of the requested display.
//
// RETURN STATUS    NVAPI_OK:                          Capabilties have been returned.
//                  NVAPI_INVALID_ARGUMENT:            One or more args passed in are invalid.
//                  NVAPI_API_NOT_INTIALIZED:          The NvAPI API needs to be initialized first
//                  NVAPI_NO_IMPLEMENTATION:           This entrypoint not available
//                  NVAPI_ERROR:                       Miscellaneous error occurred
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DISP_GetDisplayIdByDisplayName(const char *displayName, NvU32* displayId);

//  SUPPORTED OS: Windows XP and higher
///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_DISP_GetGDIPrimaryDisplayId
//
// DESCRIPTION:     This API returns the Display ID of the GDI Primary.
//
// PARAMETERS:      displayId(OUT):   Display ID of the GDI Primary display.
//
// RETURN STATUS    NVAPI_OK:                          Capabilties have been returned.
//                  NVAPI_NVIDIA_DEVICE_NOT_FOUND:     GDI Primary not on an NVIDIA GPU.
//                  NVAPI_INVALID_ARGUMENT:            One or more args passed in are invalid.
//                  NVAPI_API_NOT_INTIALIZED:          The NvAPI API needs to be initialized first
//                  NVAPI_NO_IMPLEMENTATION:           This entrypoint not available
//                  NVAPI_ERROR:                       Miscellaneous error occurred
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DISP_GetGDIPrimaryDisplayId(NvU32* displayId);



#define MAX_PCLK_RANGES 10

typedef struct
{
    NvU32           uMaxPixClk;                         // Maximum Upper Frequency
    NvU32           uMinPixClk;                         // Minimum Upper Frequency
} NV_PCLK_RANGE;

typedef struct
{
    NvU32           uOrgPixelClock;                     // Original Frequency in Hz
    NvU32           uCurrPixelClock;                    // Frequency in Hz
    NvU32           uCurrSpreadSpectrum;                // Spread Spectrum Range in Hz
    NV_PCLK_RANGE   uFineAdjustBounds;                  // Optional: Upper/Lower bounds
                                                        // If not present then only
                                                        // course adjustment is supported
    NV_PCLK_RANGE   uCourseAdjustBounds;                // Timing Shift Upper/Lower
} NV_GET_PCLK;

typedef struct
{
    NV_PCLK_RANGE   uAllowedRanges[MAX_PCLK_RANGES];    // Allowed List
    NV_PCLK_RANGE   uDisallowedRanges[MAX_PCLK_RANGES]; // Disallowed List
} NV_SET_PCLK;

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GetPixelClockRange
//
// PARAMETERS:    hPhysicalGpu (IN)  - GPU selection
//                outputId     (IN)  - Display output id
//                pTimings     (OUT) - current pixel clock settings
//
//                  NOTE: use hPhysicalGpu=NULL and outputId=0 to specify the
//                        first internal panel
//
//  SUPPORTED OS: Windows Vista and higher
// DESCRIPTION:   Retrieve the current pixel clock settings from the driver.
//
// RETURN STATUS:
//                NVAPI_OK                  - completed request
//                NVAPI_API_NOT_INITIALIZED - nvapi not initialized
//                NVAPI_ERROR               - miscellaneous error occurred
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetPixelClockRange(NvPhysicalGpuHandle hPhysicalGpu, NvU32 outputId, NV_GET_PCLK *pTimings);


///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_SetPixelClockRange
//
// PARAMETERS:    hPhysicalGpu (IN)  - GPU selection
//                outputId     (IN)  - Display output id
//                pTimings     (IN)  - allowed and disallowed pixel clock ranges
//
//                  NOTE: use hPhysicalGpu=NULL and outputId=0 to specify the
//                        first internal panel
//
//  SUPPORTED OS: Windows Vista and higher
// DESCRIPTION:   Pass allowed and disallowed pixel clock values to the
//                driver.  If a possible timing found, the driver
//                performs modeset to this timing.
//
// RETURN STATUS:
//                NVAPI_OK                  - completed request
//                NVAPI_API_NOT_INITIALIZED - nvapi not initialized
//                NVAPI_ERROR               - miscellaneous error occurred
//                NVAPI_INVALID_ARGUMENT    - no suitable pixel clock was found
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_SetPixelClockRange(NvPhysicalGpuHandle hPhysicalGpu, NvU32 outputId, NV_SET_PCLK *pTimings);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_GPU_GetECCStatusInfo
//
// PARAMETERS:      hPhysicalGpu (IN) - A handle identifying the physical GPU for
//                                      which ECC status information is to be
//                                      retrieved.
//                  pECCStatusInfo (OUT) - A pointer to an ECC status structure.
//
//  SUPPORTED OS: Windows XP and higher
//
// DESCRIPTION:     This function returns ECC memory status information.
//
// RETURN STATUS:
//    NVAPI_OK - The request was completed successfully.
//    NVAPI_ERROR - An unknown error occurred.
//    NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE - The provided GPU handle is not a physical GPU handle.
//    NVAPI_INVALID_HANDLE - The provided GPU handle is invalid.
//    NVAPI_HANDLE_INVALIDATED - The provided GPU handle is no longer valid.
//    NVAPI_INVALID_POINTER - An invalid argument pointer was provided.
//    NVAPI_NOT_SUPPORTED - The request is not supported.
//    NVAPI_API_NOT_INTIALIZED - NvAPI was not yet initialized.
//
///////////////////////////////////////////////////////////////////////////////

typedef enum _NV_ECC_CONFIGURATION
{
    NV_ECC_CONFIGURATION_NOT_SUPPORTED = 0,
    NV_ECC_CONFIGURATION_DEFERRED,           // Changes require a POST to take effect
    NV_ECC_CONFIGURATION_IMMEDIATE,          // Changes can optionally be made to take effect immediately
} NV_ECC_CONFIGURATION;

typedef struct
{
    NvU32                 version;               // Structure version
    NvU32                 isSupported : 1;       // ECC memory feature support
    NV_ECC_CONFIGURATION  configurationOptions;  // Supported ECC memory feature configuration options
    NvU32                 isEnabled : 1;         // Active ECC memory setting
} NV_GPU_ECC_STATUS_INFO;

#define NV_GPU_ECC_STATUS_INFO_VER MAKE_NVAPI_VERSION(NV_GPU_ECC_STATUS_INFO,1)

NVAPI_INTERFACE NvAPI_GPU_GetECCStatusInfo(NvPhysicalGpuHandle hPhysicalGpu,
                                           NV_GPU_ECC_STATUS_INFO *pECCStatusInfo);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_GPU_GetECCErrorInfo
//
// PARAMETERS:      hPhysicalGpu (IN) - A handle identifying the physical GPU for
//                                      which ECC error information is to be
//                                      retrieved.
//                  pECCErrorInfo (OUT) - A pointer to an ECC error structure.
//
//  SUPPORTED OS: Windows XP and higher
//
// DESCRIPTION:     This function returns ECC memory error information.
//
// RETURN STATUS:
//    NVAPI_OK - The request was completed successfully.
//    NVAPI_ERROR - An unknown error occurred.
//    NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE - The provided GPU handle is not a physical GPU handle.
//    NVAPI_INVALID_ARGUMENT - incorrect param value
//    NVAPI_INVALID_POINTER - An invalid argument pointer was provided.
//    NVAPI_INCOMPATIBLE_STRUCT_VERSION - structure version is not supported, initialize to NV_GPU_ECC_ERROR_INFO_VER.
//    NVAPI_HANDLE_INVALIDATED - The provided GPU handle is no longer valid.
//    NVAPI_NOT_SUPPORTED - The request is not supported.
//    NVAPI_API_NOT_INTIALIZED - NvAPI was not yet initialized.
//
///////////////////////////////////////////////////////////////////////////////

typedef struct
{
    NvU32   version;             // Structure version
    struct {
        NvU64  singleBitErrors;  // Number of single-bit ECC errors detected since last boot
        NvU64  doubleBitErrors;  // Number of double-bit ECC errors detected since last boot
    } current;
    struct {
        NvU64  singleBitErrors;  // Number of single-bit ECC errors detected since last counter reset
        NvU64  doubleBitErrors;  // Number of double-bit ECC errors detected since last counter reset
    } aggregate;
} NV_GPU_ECC_ERROR_INFO;

#define NV_GPU_ECC_ERROR_INFO_VER MAKE_NVAPI_VERSION(NV_GPU_ECC_ERROR_INFO,1)

NVAPI_INTERFACE NvAPI_GPU_GetECCErrorInfo(NvPhysicalGpuHandle hPhysicalGpu,
                                          NV_GPU_ECC_ERROR_INFO *pECCErrorInfo);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_GPU_ResetECCErrorInfo
//
// PARAMETERS:      hPhysicalGpu (IN) - A handle identifying the physical GPU for
//                                      which ECC error information is to be
//                                      cleared.
//                  bResetCurrent (IN) - Reset the current ECC error counters.
//                  bResetAggregate (IN) - Reset the aggregate ECC error counters.
//
//  SUPPORTED OS: Windows XP and higher
//
// DESCRIPTION:     This function resets ECC memory error counters.
//
// RETURN STATUS:
//    NVAPI_OK - The request was completed successfully.
//    NVAPI_ERROR - An unknown error occurred.
//    NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE - The provided GPU handle is not a physical GPU handle.
//    NVAPI_INVALID_HANDLE - The provided GPU handle is invalid.
//    NVAPI_HANDLE_INVALIDATED - The provided GPU handle is no longer valid.
//    NVAPI_NOT_SUPPORTED - The request is not supported.
//    NVAPI_API_NOT_INTIALIZED - NvAPI was not yet initialized.
//
///////////////////////////////////////////////////////////////////////////////

NVAPI_INTERFACE NvAPI_GPU_ResetECCErrorInfo(NvPhysicalGpuHandle hPhysicalGpu, NvU8 bResetCurrent,
                                            NvU8 bResetAggregate);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_GPU_GetECCConfigurationInfo
//
// PARAMETERS:      hPhysicalGpu (IN) - A handle identifying the physical GPU for
//                                      which ECC configuration information
//                                      is to be retrieved.
//                  pECCConfigurationInfo (OUT) - A pointer to an ECC
//                                                configuration structure.
//
//  SUPPORTED OS: Windows XP and higher
//
// DESCRIPTION:     This function returns ECC memory configuration information.
//
// RETURN STATUS:
//    NVAPI_OK - The request was completed successfully.
//    NVAPI_ERROR - An unknown error occurred.
//    NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE - The provided GPU handle is not a physical GPU handle.
//    NVAPI_INVALID_HANDLE - The provided GPU handle is invalid.
//    NVAPI_HANDLE_INVALIDATED - The provided GPU handle is no longer valid.
//    NVAPI_INVALID_POINTER - An invalid argument pointer was provided.
//    NVAPI_NOT_SUPPORTED - The request is not supported.
//    NVAPI_API_NOT_INTIALIZED - NvAPI was not yet initialized.
//
///////////////////////////////////////////////////////////////////////////////

typedef struct
{
    NvU32  version;                 // Structure version
    NvU32  isEnabled : 1;           // Current ECC configuration stored in non-volatile memory
    NvU32  isEnabledByDefault : 1;  // Factory default ECC configuration (static)
} NV_GPU_ECC_CONFIGURATION_INFO;

#define NV_GPU_ECC_CONFIGURATION_INFO_VER MAKE_NVAPI_VERSION(NV_GPU_ECC_CONFIGURATION_INFO,1)

NVAPI_INTERFACE NvAPI_GPU_GetECCConfigurationInfo(NvPhysicalGpuHandle hPhysicalGpu,
                                                  NV_GPU_ECC_CONFIGURATION_INFO *pECCConfigurationInfo);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_GPU_SetECCConfiguration
//
// PARAMETERS:      hPhysicalGpu (IN) - A handle identifying the physical GPU for
//                                      which to update the ECC configuration
//                                      setting.
//                  bEnable (IN) - The new ECC configuration setting.
//                  bEnableImmediately (IN) - Request that the new setting take effect immediately.
//
//  SUPPORTED OS: Windows XP and higher
//
// DESCRIPTION:     This function updates the ECC memory configuration setting.
//
// RETURN STATUS:
//    NVAPI_OK - The request was completed successfully.
//    NVAPI_ERROR - An unknown error occurred.
//    NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE - The provided GPU handle is not a physical GPU handle.
//    NVAPI_INVALID_HANDLE - The provided GPU handle is invalid.
//    NVAPI_HANDLE_INVALIDATED - The provided GPU handle is no longer valid.
//    NVAPI_NOT_SUPPORTED - The request is not supported.
//    NVAPI_API_NOT_INTIALIZED - NvAPI was not yet initialized.
//
///////////////////////////////////////////////////////////////////////////////

NVAPI_INTERFACE NvAPI_GPU_SetECCConfiguration(NvPhysicalGpuHandle hPhysicalGpu, NvU8 bEnable,
                                              NvU8 bEnableImmediately);

#if defined(_D3D9_H_) || defined(__d3d10_h__) || defined(__d3d10_1_h__) || defined(__d3d11_h__)
///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_D3D_SetFPSIndicatorState
//
//   DESCRIPTION: Display an overlay that tracks the number of times the app presents per second, or,
//      the number of frames-per-second (FPS)
//
//  SUPPORTED OS: Windows XP and higher
//
//   PARAMETERS: bool(IN)  - Whether or not to enable the fps indicator.
//
//   RETURN STATUS: NVAPI_OK: SUCCESS if the profile is found
//                  NVAPI_ERROR: For miscellaneous errors.
//
///////////////////////////////////////////////////////////////////////////////

NVAPI_INTERFACE NvAPI_D3D_SetFPSIndicatorState(IUnknown *pDev, NvU8 doEnable);

#endif //if defined(_D3D9_H_) || defined(__d3d10_h__) || defined(__d3d10_1_h__) || defined(__d3d11_h__)

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_SYS_VenturaGetState
//
// PARAMETERS:      state (OUT)         - current Ventura state
//
//  SUPPORTED OS: Windows Vista and higher
//
// DESCRIPTION:     This API call is used to query current Ventura state.
//                  If call succeeds, variable state hold one of the following
//                  values:
//                  - NVAPI_VENTURA_STATE_UNSUPPORTED - when executed on
//                  Ventura non-capable system
//                  - NVAPI_VENTURA_STATE_DISABLED - Ventura control is not
//                  active (but system is Ventura capable)
//                  - NVAPI_VENTURA_STATE_ENABLED - Ventura is actively
//                  managing power consumption to stay within defined budget
//
// RETURN STATUS:
//    NVAPI_OK - completed request
//    NVAPI_ERROR - miscellaneous error occurred
//    NVAPI_INVALID_POINTER - NULL argument passed
//    NVAPI_INVALID_ARGUMENT - incorrect param value
//    NVAPI_NOT_SUPPORTED - call is not supported
//    NVAPI_API_NOT_INTIALIZED - NvAPI not initialized
//
///////////////////////////////////////////////////////////////////////////////
typedef enum _NVAPI_VENTURA_STATE
{
    NVAPI_VENTURA_STATE_UNSUPPORTED,
    NVAPI_VENTURA_STATE_DISABLED,
    NVAPI_VENTURA_STATE_ENABLED

} NVAPI_VENTURA_STATE;

NVAPI_INTERFACE NvAPI_SYS_VenturaGetState(NVAPI_VENTURA_STATE *state);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_SYS_VenturaSetState
//
// PARAMETERS:      state (IN)          - new Ventura state
//
//  SUPPORTED OS: Windows Vista and higher
//
// DESCRIPTION:     This API call is used to control Ventura state.
//                  Valid values for state are:
//                  - NVAPI_VENTURA_STATE_DISABLED - deactivate Ventura
//                  - NVAPI_VENTURA_STATE_ENABLED - activate Ventura
//
// RETURN STATUS:
//    NVAPI_OK - completed request
//    NVAPI_ERROR - miscellaneous error occurred
//    NVAPI_INVALID_ARGUMENT - incorrect param value
//    NVAPI_NOT_SUPPORTED - call is not supported
//    NVAPI_API_NOT_INTIALIZED - NvAPI not initialized
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_SYS_VenturaSetState(NVAPI_VENTURA_STATE state);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_SYS_VenturaGetCoolingBudget
//
// PARAMETERS:      budget (OUT)        - current cooling budget [mW]
//
//  SUPPORTED OS: Windows Vista and higher
//
// DESCRIPTION:     This API call is used to retrieve cooling budget that is
//                  currently being used by Ventura.
//
// RETURN STATUS:
//    NVAPI_OK - completed request
//    NVAPI_ERROR - miscellaneous error occurred
//    NVAPI_INVALID_POINTER - NULL argument passed
//    NVAPI_INVALID_ARGUMENT - incorrect param value
//    NVAPI_NOT_SUPPORTED - call is not supported
//    NVAPI_API_NOT_INTIALIZED - NvAPI not initialized
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_SYS_VenturaGetCoolingBudget(NvU32 *budget);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_SYS_VenturaSetCoolingBudget
//
// PARAMETERS:      budget (IN)         - new cooling budget [mW]
//
//  SUPPORTED OS: Windows Vista and higher
//
// DESCRIPTION:     This API call is used to set new cooling budget
//                  that will be used by Ventura.
//
// RETURN STATUS:
//    NVAPI_OK - completed request
//    NVAPI_ERROR - miscellaneous error occurred
//    NVAPI_INVALID_ARGUMENT - incorrect param value
//    NVAPI_NOT_SUPPORTED - call is not supported
//    NVAPI_API_NOT_INTIALIZED - NvAPI not initialized
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_SYS_VenturaSetCoolingBudget(NvU32 budget);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_SYS_VenturaGetPowerReading
//
// PARAMETERS:      device (IN)         - targeted device
// PARAMETERS:      power (OUT)         - latest power reading [mW]
//
//  SUPPORTED OS: Windows Vista and higher
//
// DESCRIPTION:     This API call is used to query for latest power reading.
//                  'Device' describes desired target device .
//                  If call succeeds, variable 'power' holds device's latest
//                  power reading in [mW] (1/1000 of Watt).
//
//                  Power measurement for device 'system' can be greater than
//                  sum of power measurements for devices 'cpu_0' and 'gpu_0'
//                  since additional devices can be introduced in the future.
//
// RETURN STATUS:
//    NVAPI_OK - completed request
//    NVAPI_ERROR - miscellaneous error occurred
//    NVAPI_INVALID_POINTER - NULL argument passed
//    NVAPI_INVALID_ARGUMENT - incorrect param value
//    NVAPI_NOT_SUPPORTED - call is not supported
//    NVAPI_API_NOT_INTIALIZED - NvAPI not initialized
//
///////////////////////////////////////////////////////////////////////////////
typedef enum _NVAPI_VENTURA_DEVICE
{
    NVAPI_VENTURA_DEVICE_SYSTEM,
    NVAPI_VENTURA_DEVICE_CPU_0,
    NVAPI_VENTURA_DEVICE_GPU_0

} NVAPI_VENTURA_DEVICE;

NVAPI_INTERFACE NvAPI_SYS_VenturaGetPowerReading(NVAPI_VENTURA_DEVICE device, NvU32 *power);


// GPU Profile APIs

#define NVAPI_UNICODE_STRING_MAX                             2048
#define NVAPI_BINARY_DATA_MAX                                4096
#define NVAPI_SETTING_MAX_VALUES                             100

NV_DECLARE_HANDLE(NvDRSSessionHandle);
NV_DECLARE_HANDLE(NvDRSProfileHandle);

#define NVAPI_DRS_GLOBAL_PROFILE                             ((NvDRSProfileHandle) -1)

typedef NvU16 NvAPI_UnicodeString[NVAPI_UNICODE_STRING_MAX];

typedef enum _NVDRS_SETTING_TYPE
{
     NVDRS_DWORD_TYPE,
     NVDRS_BINARY_TYPE,
     NVDRS_STRING_TYPE,
     NVDRS_WSTRING_TYPE
} NVDRS_SETTING_TYPE;

typedef enum _NVDRS_SETTING_LOCATION
{
     NVDRS_CURRENT_PROFILE_LOCATION,
     NVDRS_GLOBAL_PROFILE_LOCATION,
     NVDRS_BASE_PROFILE_LOCATION
} NVDRS_SETTING_LOCATION;

typedef struct _NVDRS_GPU_SUPPORT
{
    NvU32 geforce    :  1;
    NvU32 quadro     :  1;
    NvU32 nvs        :  1;
    NvU32 reserved4  :  1;
    NvU32 reserved5  :  1;
    NvU32 reserved6  :  1;
    NvU32 reserved7  :  1;
    NvU32 reserved8  :  1;
    NvU32 reserved9  :  1;
    NvU32 reserved10 :  1;
    NvU32 reserved11 :  1;
    NvU32 reserved12 :  1;
    NvU32 reserved13 :  1;
    NvU32 reserved14 :  1;
    NvU32 reserved15 :  1;
    NvU32 reserved16 :  1;
    NvU32 reserved17 :  1;
    NvU32 reserved18 :  1;
    NvU32 reserved19 :  1;
    NvU32 reserved20 :  1;
    NvU32 reserved21 :  1;
    NvU32 reserved22 :  1;
    NvU32 reserved23 :  1;
    NvU32 reserved24 :  1;
    NvU32 reserved25 :  1;
    NvU32 reserved26 :  1;
    NvU32 reserved27 :  1;
    NvU32 reserved28 :  1;
    NvU32 reserved29 :  1;
    NvU32 reserved30 :  1;
    NvU32 reserved31 :  1;
    NvU32 reserved32 :  1;
} NVDRS_GPU_SUPPORT;

typedef struct _NVDRS_BINARY_SETTING // Enum to decide on the datatype of setting value.
{
     NvU32                valueLength;               // valueLength should always be in number of bytes.
     NvU8                 valueData[NVAPI_BINARY_DATA_MAX];
} NVDRS_BINARY_SETTING;

typedef struct _NVDRS_SETTING_VALUES
{
     NvU32                      version;                // Structure Version
     NvU32                      numSettingValues;       // Total number of values available in a setting.
     NVDRS_SETTING_TYPE         settingType;            // Type of setting value.
     union                                              // Setting can hold either DWORD or Binary value or string. Not mixed types.
     {
         NvU32                      u32DefaultValue;    // Accessing default DWORD value of this setting.
         NVDRS_BINARY_SETTING       binaryDefaultValue; // Accessing default Binary value of this setting.
                                                        // Must be allocated by caller with valueLength specifying buffer size, or only valueLength will be filled in.
         NvAPI_UnicodeString        wszDefaultValue;    // Accessing default unicode string value of this setting.
     };
     union                                              // Setting values can be of either DWORD, Binary values or String type,
     {                                                  // NOT mixed types.
         NvU32                      u32Value;           // All possible DWORD values for a setting
         NVDRS_BINARY_SETTING       binaryValue;        // All possible Binary values for a setting
         NvAPI_UnicodeString        wszValue;           // Accessing current unicode string value of this setting.
     }settingValues[NVAPI_SETTING_MAX_VALUES];
} NVDRS_SETTING_VALUES;

#define NVDRS_SETTING_VALUES_VER    MAKE_NVAPI_VERSION(NVDRS_SETTING_VALUES,1)

typedef struct _NVDRS_SETTING
{
     NvU32                      version;                // Structure Version
     NvAPI_UnicodeString        settingName;            // String name of setting
     NvU32                      settingId;              // 32 bit setting Id
     NVDRS_SETTING_TYPE         settingType;            // Type of setting value.
     NVDRS_SETTING_LOCATION     settingLocation;        // Describes where the value in CurrentValue comes from.
     NvU32                      isCurrentPredefined;    // It is different than 0 if the currentValue is a predefined Value,
                                                        // 0 if the currentValue is a user value.
     NvU32                      isPredefinedValid;      // It is different than 0 if the PredefinedValue union contains a valid value.
     union                                              // Setting can hold either DWORD or Binary value or string. Not mixed types.
     {
         NvU32                      u32PredefinedValue;    // Accessing default DWORD value of this setting.
         NVDRS_BINARY_SETTING       binaryPredefinedValue; // Accessing default Binary value of this setting.
                                                           // Must be allocated by caller with valueLength specifying buffer size,
                                                           // or only valueLength will be filled in.
         NvAPI_UnicodeString        wszPredefinedValue;    // Accessing default unicode string value of this setting.
     };
     union                                              // Setting can hold either DWORD or Binary value or string. Not mixed types.
     {
         NvU32                      u32CurrentValue;    // Accessing current DWORD value of this setting.
         NVDRS_BINARY_SETTING       binaryCurrentValue; // Accessing current Binary value of this setting.
                                                        // Must be allocated by caller with valueLength specifying buffer size,
                                                        // or only valueLength will be filled in.
         NvAPI_UnicodeString        wszCurrentValue;    // Accessing current unicode string value of this setting.
     };
} NVDRS_SETTING;

#define NVDRS_SETTING_VER        MAKE_NVAPI_VERSION(NVDRS_SETTING,1)

typedef struct _NVDRS_APPLICATION
{
     NvU32                      version;            // Structure Version
     NvU32                      isPredefined;       // Is the application userdefined/predefined
     NvAPI_UnicodeString        appName;            // String name of the Application
     NvAPI_UnicodeString        userFriendlyName;   // UserFriendly name of the Application
     NvAPI_UnicodeString        launcher;
} NVDRS_APPLICATION;

#define NVDRS_APPLICATION_VER        MAKE_NVAPI_VERSION(NVDRS_APPLICATION,1)

typedef struct _NVDRS_PROFILE
{
     NvU32                      version;            // Structure Version
     NvAPI_UnicodeString        profileName;        // String name of the Profile
     NVDRS_GPU_SUPPORT          gpuSupport;         // This flag indicates the profile support on either,
                                                    // Quadro or Geforce or Both. Read Only.
     NvU32                      isPredefined;       // Is the Profile userdefined/predefined
     NvU32                      numOfApps;          // Total number of apps that belong to this profile. Read-only
     NvU32                      numOfSettings;      // Total number of settings applied for this Profile. Read-only
} NVDRS_PROFILE;

#define NVDRS_PROFILE_VER        MAKE_NVAPI_VERSION(NVDRS_PROFILE,1)

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_DRS_CreateSession
//
//   DESCRIPTION: Allocate memory and initialization
//
//  SUPPORTED OS: Windows XP and higher
//
//   PARAMETERS: NvDRSSessionHandle(OUT) - Returns pointer to session handle.
//
//   RETURN STATUS: NVAPI_OK: SUCCESS
//                  NVAPI_ERROR: For miscellaneous errors.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DRS_CreateSession(NvDRSSessionHandle *phSession);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_DRS_DestroySession
//
//   DESCRIPTION: Free allocation and cleanup of NvDrsSession
//
//  SUPPORTED OS: Windows XP and higher
//
//   PARAMETERS: NvDRSSessionHandle(IN) - Input to the session handle.
//
//   RETURN STATUS: NVAPI_OK: SUCCESS
//                  NVAPI_ERROR: For miscellaneous errors.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DRS_DestroySession(NvDRSSessionHandle hSession);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_DRS_LoadSettings
//
//   DESCRIPTION: Load and parse settings data
//
//  SUPPORTED OS: Windows XP and higher
//
//   PARAMETERS: NvDRSSessionHandle(IN) - Input to the session handle.
//
//   RETURN STATUS: NVAPI_OK: SUCCESS
//                  NVAPI_ERROR: For miscellaneous errors.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DRS_LoadSettings(NvDRSSessionHandle hSession);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_DRS_SaveSettings
//
//   DESCRIPTION: Save settings data to the system
//
//  SUPPORTED OS: Windows XP and higher
//
//   PARAMETERS: NvDRSSessionHandle(IN) - Input to the session handle.
//
//   RETURN STATUS: NVAPI_OK: SUCCESS
//                  NVAPI_ERROR: For miscellaneous errors.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DRS_SaveSettings(NvDRSSessionHandle hSession);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_DRS_LoadSettingsFromFile
//
//   DESCRIPTION: Load settings from the given file path
//
//  SUPPORTED OS: Windows XP and higher
//
//   PARAMETERS: NvDRSSessionHandle(IN) - Input to the session handle.
//               NvAPI_UnicodeString(IN) - Binary File Name/Path
//
//   RETURN STATUS: NVAPI_OK: SUCCESS
//                  NVAPI_ERROR: For miscellaneous errors.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DRS_LoadSettingsFromFile(NvDRSSessionHandle hSession, NvAPI_UnicodeString fileName);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_DRS_SaveSettingsToFile
//
//   DESCRIPTION: Save settings to the given file path
//
//  SUPPORTED OS: Windows XP and higher
//
//   PARAMETERS: NvDRSSessionHandle(IN) - Input to the session handle.
//               NvAPI_UnicodeString(IN) - Binary File Name/Path
//
//   RETURN STATUS: NVAPI_OK: SUCCESS
//                  NVAPI_ERROR: For miscellaneous errors.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DRS_SaveSettingsToFile(NvDRSSessionHandle hSession, NvAPI_UnicodeString fileName);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_DRS_CreateProfile
//
//   DESCRIPTION: Create an empty profile.
//
//  SUPPORTED OS: Windows XP and higher
//
//   PARAMETERS: NvDRSSessionHandle(IN)   - Input to the session handle.
//               NVDRS_PROFILE(IN)        - Input pointer to NVDRS_PROFILE.
//               NvDRSProfileHandle(OUT)  - Returns pointer to profile handle.
//
//   RETURN STATUS: NVAPI_OK: SUCCESS
//                  NVAPI_ERROR: For miscellaneous errors.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DRS_CreateProfile(NvDRSSessionHandle hSession, NVDRS_PROFILE *pProfileInfo, NvDRSProfileHandle *phProfile);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_DRS_DeleteProfile
//
//   DESCRIPTION: Delete a profile or sets it back to predefined value.
//
//  SUPPORTED OS: Windows XP and higher
//
//   PARAMETERS: NvDRSSessionHandle(IN)  - Input to the session handle.
//               NvDRSProfileHandle (IN) - Input profile handle.
//
//   RETURN STATUS: NVAPI_OK: SUCCESS if the profile is found
//                  NVAPI_ERROR: For miscellaneous errors.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DRS_DeleteProfile(NvDRSSessionHandle hSession, NvDRSProfileHandle hProfile);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_DRS_SetCurrentGlobalProfile
//
//   DESCRIPTION: Sets the current global profile in the driver.
//
//  SUPPORTED OS: Windows XP and higher
//
//   PARAMETERS: NvDRSSessionHandle(IN)  - Input to the session handle.
//               NvAPI_UnicodeString(IN) - Input current Global profile name.
//
//   RETURN STATUS: NVAPI_OK: SUCCESS
//                  NVAPI_ERROR: For miscellaneous errors.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DRS_SetCurrentGlobalProfile(NvDRSSessionHandle hSession, NvAPI_UnicodeString wszGlobalProfileName);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_DRS_GetCurrentGlobalProfile
//
//   DESCRIPTION: Returns the handle to the current global profile.
//
//  SUPPORTED OS: Windows XP and higher
//
//   PARAMETERS: NvDRSSessionHandle(IN)  - Input to the session handle.
//               NvDRSProfileHandle(OUT) - Returns current Global profile handle.
//
//   RETURN STATUS: NVAPI_OK: SUCCESS
//                  NVAPI_ERROR: For miscellaneous errors.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DRS_GetCurrentGlobalProfile(NvDRSSessionHandle hSession, NvDRSProfileHandle *phProfile);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_DRS_GetProfileInfo
//
//   DESCRIPTION: Get the information of the given profile. User needs to specify the name of the Profile.
//
//  SUPPORTED OS: Windows XP and higher
//
//   PARAMETERS: NvDRSSessionHandle(IN)  - Input to the session handle.
//               NvDRSProfileHandle(IN)  - Input profile handle.
//               NVDRS_PROFILE(OUT)      - Return the profile info.
//
//   RETURN STATUS: NVAPI_OK: SUCCESS
//                  NVAPI_ERROR: For miscellaneous errors.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DRS_GetProfileInfo(NvDRSSessionHandle hSession, NvDRSProfileHandle hProfile, NVDRS_PROFILE *pProfileInfo);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_DRS_SetProfileInfo
//
//   DESCRIPTION: Specifies flags for a given profile. Currently only the NVDRS_GPU_SUPPORT is
//                used to update the profile. Neither the name, number of settings or applications
//                or other profile information can be changed with this function.
//
//  SUPPORTED OS: Windows XP and higher
//
//   PARAMETERS: NvDRSSessionHandle(IN)  - Input to the session handle.
//               NvDRSProfileHandle(IN)  - Input profile handle.
//               NVDRS_PROFILE(IN)       - Input the new profile info.
//
//   RETURN STATUS: NVAPI_OK: SUCCESS
//                  NVAPI_ERROR: For miscellaneous errors.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DRS_SetProfileInfo(NvDRSSessionHandle hSession, NvDRSProfileHandle hProfile, NVDRS_PROFILE *pProfileInfo);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_DRS_FindProfileByName
//
//   DESCRIPTION: Find a profile in the current session.
//
//  SUPPORTED OS: Windows XP and higher
//
//   PARAMETERS: NvDRSSessionHandle(IN)  - Input to the session handle.
//               NvAPI_UnicodeString(IN) - Input profileName.
//               NvDRSProfileHandle(OUT) - Input profile handle.
//
//   RETURN STATUS: NVAPI_OK: SUCCESS if the profile is found
//                  NVAPI_PROFILE_NOT_FOUND: if profile is not found
//                  NVAPI_ERROR: For miscellaneous errors.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DRS_FindProfileByName(NvDRSSessionHandle hSession, NvAPI_UnicodeString profileName, NvDRSProfileHandle* phProfile);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_DRS_EnumProfiles
//
//   DESCRIPTION: Enumerate through all the profiles in the session.
//
//  SUPPORTED OS: Windows XP and higher
//
//   PARAMETERS: NvDRSSessionHandle(IN)   - Input to the session handle.
//               index(IN)                - Input the index for enumeration.
//               NvDRSProfileHandle(OUT)  - Returns profile handle.
//
//   RETURN STATUS: NVAPI_OK: SUCCESS if the profile is found
//                  NVAPI_ERROR: For miscellaneous errors.
//                  NVAPI_END_ENUMERATION: index exceeds the total number of available Profiles in DB.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DRS_EnumProfiles(NvDRSSessionHandle hSession, NvU32 index, NvDRSProfileHandle *phProfile);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_DRS_GetNumProfiles
//
//   DESCRIPTION: Obtain number of profiles in the current session object
//
//  SUPPORTED OS: Windows XP and higher
//
//   PARAMETERS: NvDRSSessionHandle(IN)  - Input to the session handle.
//               numProfiles(OUT) -  returns count of profiles in the current hSession.
//
//   RETURN STATUS: NVAPI_OK: SUCCESS
//                  NVAPI_API_NOT_INTIALIZED: Failed to initialize.
//                  NVAPI_INVALID_ARGUMENT: Invalid Arguments.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DRS_GetNumProfiles(NvDRSSessionHandle hSession, NvU32 *numProfiles);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_DRS_CreateApplication
//
//   DESCRIPTION: Add an executable name to a profile.
//
//  SUPPORTED OS: Windows XP and higher
//
//   PARAMETERS: NvDRSSessionHandle(IN)  - Input to the session handle.
//               NvDRSProfileHandle(IN)  - Input profile handle.
//               NVDRS_APPLICATION(IN)   - Input NVDRS_APPLICATION struct with the executable name to be added.
//
//   RETURN STATUS: NVAPI_OK: SUCCESS
//                  NVAPI_ERROR: For miscellaneous errors.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DRS_CreateApplication(NvDRSSessionHandle hSession, NvDRSProfileHandle  hProfile, NVDRS_APPLICATION *pApplication);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_DRS_DeleteApplication
//
//   DESCRIPTION: Removes an executable from a profile.
//
//  SUPPORTED OS: Windows XP and higher
//
//   PARAMETERS: NvDRSSessionHandle(IN)  - Input to the session handle.
//               NvDRSProfileHandle(IN)  - Input profile handle.
//               NvAPI_UnicodeString(IN) - Input the executable name to be removed.
//
//   RETURN STATUS: NVAPI_OK: SUCCESS
//                  NVAPI_ERROR: For miscellaneous errors.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DRS_DeleteApplication(NvDRSSessionHandle hSession, NvDRSProfileHandle hProfile, NvAPI_UnicodeString appName);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_DRS_GetApplicationInfo
//
//   DESCRIPTION: Get the information of the given application.
//
//  SUPPORTED OS: Windows XP and higher
//
//   PARAMETERS: NvDRSSessionHandle(IN)  - Input to the session handle.
//               NvDRSProfileHandle(IN)  - Input profile handle.
//               NvAPI_UnicodeString(IN) - Input application name.
//               NVDRS_APPLICATION(OUT)  - Returns NVDRS_APPLICATION struct with all the attributes.
//
//   RETURN STATUS: NVAPI_OK: SUCCESS
//                  NVAPI_ERROR: For miscellaneous errors.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DRS_GetApplicationInfo(NvDRSSessionHandle hSession, NvDRSProfileHandle hProfile, NvAPI_UnicodeString appName, NVDRS_APPLICATION *pApplication);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_DRS_EnumApplications
//
//   DESCRIPTION: Enumerate all the applications in a given profile from the starting index to the max length.
//
//  SUPPORTED OS: Windows XP and higher
//
//   PARAMETERS: NvDRSSessionHandle(IN)  - Input to the session handle.
//               NvDRSProfileHandle(IN)  - Input profile handle.
//               startIndex(IN)          - Indicates starting index for enumeration.
//               appCount(IN OUT)        - Input max length of the passed in arrays, Returns the actual length.
//               NVDRS_APPLICATION(OUT)  - Returns NVDRS_APPLICATION struct with all the attributes.
//
//   RETURN STATUS: NVAPI_OK: SUCCESS
//                  NVAPI_ERROR: For miscellaneous errors.
//                  NVAPI_END_ENUMERATION: startIndex exceeds the total appCount.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DRS_EnumApplications(NvDRSSessionHandle hSession, NvDRSProfileHandle hProfile, NvU32 startIndex, NvU32 *appCount, NVDRS_APPLICATION *pApplication);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_DRS_FindApplicationByName
//
//   DESCRIPTION: Search the Application and the associated Profile for given appName.
//
//  SUPPORTED OS: Windows XP and higher
//
//   PARAMETERS: NvDRSSessionHandle(IN)  - Input to the hSession handle.
//               NvAPI_UnicodeString(IN) - Input appName.
//               NvDRSProfileHandle(OUT) - Returns profile handle.
//               NVDRS_APPLICATION(OUT)  - Returns NVDRS_APPLICATION struct pointer.
//
//   RETURN STATUS: NVAPI_OK: SUCCESS if the profile is found
//                  NVAPI_APPLICATION_NOT_FOUND: if App not found
//                  NVAPI_ERROR: For miscellaneous errors.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DRS_FindApplicationByName(NvDRSSessionHandle hSession, NvAPI_UnicodeString appName, NvDRSProfileHandle *phProfile, NVDRS_APPLICATION *pApplication);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_DRS_SetSetting
//
//   DESCRIPTION: Add/Modify a setting to a Profile.
//
//  SUPPORTED OS: Windows XP and higher
//
//   PARAMETERS: NvDRSSessionHandle(IN)  - Input to the session handle.
//               NvDRSProfileHandle(IN)  - Input profile handle.
//               NVDRS_SETTING(IN)       - Input NVDRS_SETTING struct pointer.
//
//   RETURN STATUS: NVAPI_OK: SUCCESS
//                  NVAPI_ERROR: For miscellaneous errors.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DRS_SetSetting(NvDRSSessionHandle hSession, NvDRSProfileHandle hProfile, NVDRS_SETTING *pSetting);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_DRS_GetSetting
//
//   DESCRIPTION: Get the information of the given setting.
//
//  SUPPORTED OS: Windows XP and higher
//
//   PARAMETERS: NvDRSSessionHandle(IN)  - Input to the session handle.
//               NvDRSProfileHandle(IN)  - Input profile handle.
//               NvU32(IN)               - Input settingId.
//               NVDRS_SETTING(OUT)      - Returns all the setting info
//
//   RETURN STATUS: NVAPI_OK: SUCCESS
//                  NVAPI_ERROR: For miscellaneous errors.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DRS_GetSetting(NvDRSSessionHandle hSession, NvDRSProfileHandle hProfile, NvU32 settingId, NVDRS_SETTING *pSetting);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_DRS_EnumSettings
//
//   DESCRIPTION: Enumerate all the settings of a given profile from startIndex to the max length.
//
//  SUPPORTED OS: Windows XP and higher
//
//   PARAMETERS: NvDRSSessionHandle(IN)  - Input to the session handle.
//               NvDRSProfileHandle(IN)  - Input profile handle.
//               startIndex(IN)          - Indicates starting index for enumeration.
//               settingsCount(IN OUT)   - Input max length of the passed in arrays, Returns the actual length.
//               NVDRS_SETTING(OUT)      - Returns all the settings info.
//
//   RETURN STATUS: NVAPI_OK: SUCCESS
//                  NVAPI_ERROR: For miscellaneous errors.
//                  NVAPI_END_ENUMERATION: startIndex exceeds the total appCount.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DRS_EnumSettings(NvDRSSessionHandle hSession, NvDRSProfileHandle hProfile, NvU32 startIndex, NvU32 *settingsCount, NVDRS_SETTING *pSetting);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_DRS_EnumAvaliableSettingIds
//
//   DESCRIPTION: Enumerate all the Ids of all the settings recognized by NVAPI.
//
//  SUPPORTED OS: Windows XP and higher
//
//   PARAMETERS: pSettingIds(OUT)    - User-provided array of length *pMaxCount that NVAPI will fill with IDs.
//               pMaxCount(IN OUT)   - Input max length of the passed in array, Returns the actual length.
//
//   RETURN STATUS: NVAPI_OK: SUCCESS
//                  NVAPI_ERROR: For miscellaneous errors.
//                  NVAPI_END_ENUMERATION: the provided pMaxCount is not enough to hold all settingIds.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DRS_EnumAvailableSettingIds(NvU32 *pSettingIds, NvU32 *pMaxCount);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_DRS_EnumAvailableSettingValues
//
//   DESCRIPTION: Enumerate all available setting values for a given setting.
//
//  SUPPORTED OS: Windows XP and higher
//
//   PARAMETERS: settingId(IN)               - Input settingId.
//               maxNumCount(IN/OUT)         - Input max length of the passed in arrays, Returns the actual length.
//               NVDRS_SETTING_VALUES(OUT)   - Returns all available setting values and its count.
//
//   RETURN STATUS: NVAPI_OK: SUCCESS
//                  NVAPI_ERROR: For miscellaneous errors.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DRS_EnumAvailableSettingValues(NvU32 settingId, NvU32 *pMaxNumValues, NVDRS_SETTING_VALUES *pSettingValues);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_DRS_GetSettingIdFromName
//
//   DESCRIPTION: Get the binary ID of a setting given the setting name.
//
//  SUPPORTED OS: Windows XP and higher
//
//   PARAMETERS: NvAPI_UnicodeString(IN)  - Input Unicode settingName.
//               NvU32(OUT)               - Returns corresponding settingId.
//
//   RETURN STATUS: NVAPI_OK: SUCCESS if the profile is found
//                  NVAPI_PROFILE_NOT_FOUND: if profile is not found
//                  NVAPI_SETTING_NOT_FOUND: if setting is not found
//                  NVAPI_ERROR: For miscellaneous errors.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DRS_GetSettingIdFromName(NvAPI_UnicodeString settingName, NvU32 *pSettingId);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_DRS_GetSettingNameFromId
//
//   DESCRIPTION: Get the setting name given the binary ID.
//
//  SUPPORTED OS: Windows XP and higher
//
//   PARAMETERS: NvU32(IN)                - Input settingId.
//               NvAPI_UnicodeString(OUT) - Returns corresponding Unicode settingName.
//
//   RETURN STATUS: NVAPI_OK: SUCCESS if the profile is found
//                  NVAPI_PROFILE_NOT_FOUND: if profile is not found
//                  NVAPI_SETTING_NOT_FOUND: if setting is not found
//                  NVAPI_ERROR: For miscellaneous errors.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DRS_GetSettingNameFromId(NvU32 settingId, NvAPI_UnicodeString *pSettingName);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_DRS_DeleteProfileSetting
//
//   DESCRIPTION: Delete a setting or set it back to predefined value.
//
//  SUPPORTED OS: Windows XP and higher
//
//   PARAMETERS: NvDRSSessionHandle(IN)  - Input to the session handle.
//               NvDRSProfileHandle(IN)  - Input profile handle.
//               settingId(IN)           - Input settingId to be deleted.
//
//   RETURN STATUS: NVAPI_OK: SUCCESS if the profile is found
//                  NVAPI_ERROR: For miscellaneous errors.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DRS_DeleteProfileSetting(NvDRSSessionHandle hSession, NvDRSProfileHandle hProfile, NvU32 settingId);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_DRS_RestoreAllDefaults
//
//   DESCRIPTION: Restore the whole system to predefined(default) values.
//
//  SUPPORTED OS: Windows XP and higher
//
//   PARAMETERS: NvDRSSessionHandle(IN)  - Input to the session handle.
//
//   RETURN STATUS: NVAPI_OK: SUCCESS if the profile is found
//                  NVAPI_ERROR: For miscellaneous errors.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DRS_RestoreAllDefaults(NvDRSSessionHandle hSession);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_DRS_RestoreProfileDefaults
//
//   DESCRIPTION: Restore the given profile to predefined(default) values.
//                Any and all user specified modifications will be removed.
//                If the whole profile was set by the user, the profile will be removed.
//
//  SUPPORTED OS: Windows XP and higher
//
//   PARAMETERS: NvDRSSessionHandle(IN)  - Input to the session handle.
//               NvDRSProfileHandle(IN)  - Input profile handle.
//
//   RETURN STATUS: NVAPI_OK: SUCCESS if the profile is found
//                  NVAPI_PROFILE_REMOVED: SUCCESS, and the hProfile is no longer valid.
//                  NVAPI_ERROR: For miscellaneous errors.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DRS_RestoreProfileDefault(NvDRSSessionHandle hSession, NvDRSProfileHandle hProfile);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_DRS_RestoreProfileDefaultSetting
//
//   DESCRIPTION: Restore the given profile setting to predefined(default) values.
//
//  SUPPORTED OS: Windows XP and higher
//
//   PARAMETERS: NvDRSSessionHandle(IN)  - Input to the session handle.
//               NvDRSProfileHandle(IN)  - Input profile handle.
//               NvU32(IN)               - Input settingId.
//
//   RETURN STATUS: NVAPI_OK: SUCCESS if the profile is found
//                  NVAPI_ERROR: For miscellaneous errors.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DRS_RestoreProfileDefaultSetting(NvDRSSessionHandle hSession, NvDRSProfileHandle hProfile, NvU32 settingId);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_DRS_GetBaseProfile
//
//   DESCRIPTION: Returns the handle to the current global profile.
//
//  SUPPORTED OS: Windows XP and higher
//
//   PARAMETERS: NvDRSSessionHandle(IN)  - Input to the session handle.
//               NvDRSProfileHandle(OUT) - Returns Base profile handle.
//
//   RETURN STATUS: NVAPI_OK: SUCCESS
//                  NVAPI_ERROR: For miscellaneous errors.
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_DRS_GetBaseProfile(NvDRSSessionHandle hSession, NvDRSProfileHandle *phProfile);


// Enum for Event IDs
typedef enum
{
    NV_EVENT_TYPE_NONE = 0,
    NV_EVENT_TYPE_FAN_SPEED_CHANGE = 1,
    NV_EVENT_TYPE_THERMAL_CHANGE = 2,
} NV_EVENT_TYPE;

typedef enum
{
    UNKNOWN_LEVEL  = 0,
    NORMAL_LEVEL   = 1,
    WARNING_LEVEL  = 2, 
    CRITICAL_LEVEL = 3,
} NV_EVENT_LEVEL;

// Callback for thermal
typedef void (__cdecl *NVAPI_CALLBACK_THERMALEVENT)(NvPhysicalGpuHandle gpuHandle, NV_EVENT_LEVEL thermalLevel, void *callbackParam);

// Callback for fan speed
typedef void (__cdecl *NVAPI_CALLBACK_FANSPEEDEVENT)(NvPhysicalGpuHandle gpuHandle, NV_EVENT_LEVEL fanSpeedLevel, void *callbackParam);

// Core NV_EVENT_REGISTER_CALLBACK structure declaration
typedef struct
{
    NvU32                 version;          // version field to ensure minimum version compatibility
    NV_EVENT_TYPE         eventId;          // ID of the event being sent
    void                  *callbackParam;   // This value will be passed back to the callback function when an event occurs
    union
    {
        NVAPI_CALLBACK_THERMALEVENT  nvThermalCallback;   // Callback function pointer for thermal                               
        NVAPI_CALLBACK_FANSPEEDEVENT nvFanSpeedCallback;  // Callback function pointer for fanSpeed
        
    }nvCallBackFunc;
    
} NV_EVENT_REGISTER_CALLBACK, *PNV_EVENT_REGISTER_CALLBACK;

#define NV_EVENT_REGISTER_CALLBACK_VERSION        MAKE_NVAPI_VERSION(NV_EVENT_REGISTER_CALLBACK,1);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Event_RegisterCallback
//
// DESCRIPTION:   Register the process for events. This API should be called for each eventcallback.
//                Handle returned to the client will be common across all eventCallbacks.
//
//  SUPPORTED OS: Windows XP and higher
//
// PARAMETERS:    IN PNV_EVENT_REGISTER_CALLBACK - Pointer to NV_EVENT_REGISTER_CALLBACK structure to call
//                                                 on new events
//                OUT NvEventHandle* phClient    - Handle to client for use with
//                                                 unregister function
//
// RETURN STATUS:
//                NVAPI_OK - completed request
//                NVAPI_API_NOT_INTIALIZED - NvAPI not initialized
//                NVAPI_INVALID_ARGUMENT - Invalid argument
//                NVAPI_ERROR - miscellaneous error occurred
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Event_RegisterCallback(PNV_EVENT_REGISTER_CALLBACK eventCallback,
                                             NvEventHandle* phClient);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Event_UnregisterCallback
//
// DESCRIPTION:   Unregister an event handle. This API should be called only once per process(irrespective of the number of callbacks registered).
//
//  SUPPORTED OS: Windows XP and higher
//
// PARAMETERS:    hClient(IN) - Handle associated with this listeners
//                              event queue. Same as returned from
//                              NvAPI_Event_RegisterCallback.
//
// RETURN STATUS:
//                NVAPI_OK - completed request
//                NVAPI_API_NOT_INTIALIZED - NvAPI not initialized
//                NVAPI_INVALID_ARGUMENT - Invalid argument
//                NVAPI_ERROR - miscellaneous error occurred
//
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Event_UnregisterCallback(NvEventHandle hClient);



/////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GetCurrentThermalLevel
//
//
// DESCRIPTION:   This API returns the current Level (Normal, Medium or 
//                Critical) of the thermal sensor.
//
//  SUPPORTED OS: Windows XP and higher
//
// PARAMETERS:    nvGPUHandle    (IN)  - Physical GPU handle.
//                pThermalLevel  (OUT) - Returns Thermal Level. 
//
// RETURN STATUS:
//                NVAPI_OK                                 - Success
//                NVAPI_API_NOT_INTIALIZED                 - NvAPI not initialized
//                NVAPI_INVALID_ARGUMENT                   - Invalid argument
//                NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE       - Invalid GPU handle
//                NVAPI_NOT_SUPPORTED                      - API not supported
//                NVAPI_ERROR                              - miscellaneous error occurred
//
/////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetCurrentThermalLevel(NvPhysicalGpuHandle nvGPUHandle, NV_EVENT_LEVEL *pThermalLevel);

///////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME: NvAPI_GPU_GetCurrentFanSpeedLevel
//
// DESCRIPTION:   This API returns the current Level (Normal, Medium or 
//                Critical) of the fan speed.
//
//  SUPPORTED OS: Windows XP and higher
//
// PARAMETERS:    nvGPUHandle    (IN)  - Physical GPU handle.
//                pFanSpeedLevel (OUT) - Returns fan speed Level. 
//
// RETURN STATUS:
//                NVAPI_OK                                 - Success
//                NVAPI_API_NOT_INTIALIZED                 - NvAPI not initialized
//                NVAPI_INVALID_ARGUMENT                   - Invalid argument
//                NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE       - Invalid GPU handle
//                NVAPI_NOT_SUPPORTED                      - API not supported
//                NVAPI_ERROR                              - miscellaneous error occurred
//
/////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetCurrentFanSpeedLevel(NvPhysicalGpuHandle nvGPUHandle, NV_EVENT_LEVEL *pFanSpeedLevel);



#ifdef __cplusplus
}; //extern "C" {

#endif

#pragma pack(pop)

#endif // _NVAPI_H

