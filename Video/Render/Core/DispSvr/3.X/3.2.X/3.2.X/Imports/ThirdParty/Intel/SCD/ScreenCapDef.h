/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
**
** Copyright (c) Intel Corporation (2007).
**
** INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS 
** LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT, 
** ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT 
** PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY 
** DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
** PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability, 
** including liability for infringement of any proprietary rights, relating to
** use of the code. No license, express or implied, by estoppel or otherwise, 
** to any intellectual property rights is granted herein.
**
**
** File Name  : screencapdef.h
**
** Abstract   : DXVA Screen Capture Defense Auxiliary Device Service
**
** Environment: Windows Vista
**
** Notes      : 
**
**+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
#ifndef DXVA_SCD_INCLUDE
#define DXVA_SCD_INCLUDE

// Screen Capture Defense Service GUID - {31D1F97E-DF6C-4e03-81AB-5BE2F2E7E5D4}
static const GUID DXVA2_Intel_SCD = 
{ 0x31d1f97e, 0xdf6c, 0x4e03, { 0x81, 0xab, 0x5b, 0xe2, 0xf2, 0xe7, 0xe5, 0xd4 } };

// Auxilary Service Function Codes
typedef enum tagSCD_FUNCTION_ID
{
    SCD_BEGIN                  = 0x301,
    SCD_STOP                   = 0x302,
    SCD_SET_WINDOW_POSITION    = 0x303,
	SCD_INIT_AUTHENTICATION	   = 0x304,
	SCD_UNSCRAMBLE_SURFACE	   = 0x305
} SCD_FUNCTION_ID;

// Screen Capture Defense Modes
enum
{
    SCD_OVL_BASED = 1,
    SCD_DWM_BASED = 2
};

typedef struct tagSCD_MODES
{
    // Device data
    DWORD   ScdMode;            // Vista User Mode Device context
    DWORD   dwRsvdModeData;     // Pointer to the extension device data

} SCD_MODE;

typedef struct tagSCD_CREATE_DEVICE
{
    DWORD	dwRsvdSCDServiceVer;  // Reserved
} SCD_CREATE_DEVICE;


//===========================================================================
// typedef:
//        SCD_SET_WINDOW_POSITION
//
// Description:
//     The application sends down the target window position.
//     This resolves problems displaying when transparent windows
//     are being used.
//---------------------------------------------------------------------------
typedef struct tagSCD_SET_WINDOW_POSITION_PARAMS
{
    RECT WindowPosition;
    RECT VisibleContent;
    HDC hdcMonitorId;
} SCD_SET_WINDOW_POSITION_PARAMS;


//===========================================================================
// dwAppId			This is an input value and is provided to the application vendor pre-build time.
// hRegistration	This is an output value and the application uses it for registering surfaces.
//---------------------------------------------------------------------------

typedef struct tagSCD_INIT_AUTHENTICATION_PARAMS
{
	DWORD dwAppId;
	HANDLE hRegistration;
} SCD_INIT_AUTHENTICATION_PARAMS;


//===========================================================================
// The SCD unscramble surface parameters are passed through the following structure:
// 
// pScrambledSurface	This is a pointer to the content that has been scrambled.
//   This pointer must be registered as described in section 3.2 prior to calling the unscramble function.
// dwSizeScrambled	This is the size of the content that has been scrambled.
//   It is not required that this size is the same as the entire surface, but it is required that
//   scrambling begin at the origin of the surface. 
// dwScrambleSeed	This is an input value to function F().
//---------------------------------------------------------------------------

typedef struct tagSCD_UNSCRAMBLE_SURFACE_PARAMS
{
	void* pScrambledSurface;
	DWORD dwSizeScrambled; 
	DWORD dwScrambleSeed;
} SCD_UNSCRAMBLE_SURFACE_PARAMS;


#endif // DXVA_SCD_INCLUDE
