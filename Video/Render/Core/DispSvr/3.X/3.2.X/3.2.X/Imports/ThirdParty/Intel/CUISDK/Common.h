/*
"INTEL CONFIDENTIAL"
Copyright 2009 October - December 
Intel Corporation All Rights Reserved.

The source code contained or described herein and all documents related to the 
source code ("Material") are owned by Intel Corporation or its suppliers or licensors. 
Title to the Material remains with Intel Corporation or its suppliers and licensors. 
The Material may contain trade secrets and proprietary and confidential information of Intel
Corporation and its suppliers and licensors, and is protected by worldwide copyright and 
trade secret laws and treaty provisions. No part of the Material may be used, copied, 
reproduced, modified, published, uploaded, posted, transmitted, distributed, or disclosed 
in any way without Intel’s prior express written permission. 
No license under any patent, copyright, trade secret or other intellectual property right 
is granted to or conferred upon you by disclosure or delivery of the Materials, 
either expressly, by implication, inducement, estoppel or otherwise. 
Any license under such intellectual property rights must be express and approved by Intel
in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or 
any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.	 
Author - CUI Team
*/
// Common includes. This is a file shared between Intel & External customers
#ifndef CUISDK_COMMON_H
#define CUISDK_COMMON_H

// EVENT FLAGS
#define IGFX_NULL_EVENT							0x0000
#define IGFX_ROTATION_EVENT						0x0001
#define IGFX_PANEL_FITTING_EVENT				0x0002
#define IGFX_PANNING_EVENT						0x0004
#define IGFX_ALL_EVENTS							IGFX_ROTATION_EVENT | IGFX_PANEL_FITTING_EVENT | IGFX_PANNING_EVENT

/************************************************************
*			 Intel specific error codes						*
*************************************************************/
// Generic Error Codes
#define IGFX_SUCCESS							0x0000
#define IGFX_REGISTRATION_ERROR					0x0001
#define IGFX_INVALID_EVENTHANDLE				0x0002
#define IGFX_INVALID_EVENTMASK					0x0003
#define IGFX_CORRUPT_BUFFER						0x0004
#define IGFX_UNSUPPORTED_GUID					0x0005
#define IGFX_GETCONFIGURATION_ERROR				0x0006
#define IGFX_DEREGISTRATION_ERROR				0x0007
#define IGFX_INVALIDMONITOR_ID					0x0008
#define IGFX_INVALIDCONFIG_FLAG					0x0009
#define IGFX_SETCONFIGATION_ERROR				0x000A

// Error codes for MCCS
#define IGFX_INVALID_MCCS_HANDLE				0x000C
#define IGFX_INVALID_MCCS_CONTROLCODE			0x000D
#define IGFX_INVALID_MCCS_SIZE					0x000E
#define IGFX_MCCS_DRIVER_ERROR					0x000F
#define IGFX_MCCS_DEVICE_ERROR					0x0010
#define IGFX_MCCS_INVALID_MONITOR				0x0011

// Error codes for PowerAPI.
#define IGFX_POWER_API_NOT_SUPPORTED			0x0013
#define IGFX_POWER_API_LOCKED					0x0014
#define IGFX_POWER_API_INVALID_UNLOCK_REQUEST	0x0015
#define IGFX_INVALID_POWER_HANDLE				0x0016
#define IGFX_INVALID_POWER_POLICY				0x0017
#define IGFX_UNSUPPORTED_POWER_POLICY			0x0018
#define IGFX_POWERDEVICE_ERROR					0x0019
#define IGFX_INVALID_DISPLAYID					0x001A

// Error codes for Aspect Scaling
#define IGFX_WRONG_ASPECT_PREFERENCE			0x001B
#define IGFX_INVAILD_OPERATING_MODE				0x001C

// Error codes for Gamma
#define IGFX_INVALID_GAMMA_RAMP					0x001D

// Error codes for TV Connectors
#define IGFX_INVALID_CONNECTOR_SELECTION		0x001E

// Error Code for Get/Set RR RCR
#define IGFX_INVALID_DEVICE_COMBINATION			0x001F
#define IGFX_SETCLONE_FAILED					0x0020
#define IGFX_INVALID_RESOLUTION					0x0021
#define IGFX_INVALID_CONFIGURATION				0x0022

// Error Codes for PWM Frequency
#define IGFX_UNSUPPORTED_INVERTER				0x0023
#define IGFX_BACKLIGHT_PARAMS_INVALID_FREQ		0x0024

// Error Codes for ICUIExternal8
#define IGFX_FAILURE							0x0025

// Error codes for MCCS
#define	IGFX_INVALID_INDEX						0x0026

// Error Code for Persistence Data
#define IGFX_INVALID_INPUT						0x0027

// Error Code for Set Configuration
#define IGFX_INCORRECT_RESOLUTION_FORMAT		0x0028
#define IGFX_INVALID_ORIENTATION_COMBINATION	0x0029
#define IGFX_INVALID_ORIENTATION				0x002A

//Error codes for Custom Mode
#define IGFX_INVALID_CUSTOM_MODE				0x002B
#define IGFX_EXISTING_MODE						0x002C
#define IGFX_EXISTING_BASIC_MODE 				0x002D
#define IGFX_EXISTING_ADVANCED_MODE				0x002E
#define IGFX_EXCEEDING_BW_LIMITATION			0x002F
#define IGFX_EXISTING_INSUFFICIENT_MEMORY		0x0030

//Error codes for unsupported features
#define IGFX_UNSUPPORTED_FEATURE				0x0031

// Error code for PF2
#define IGFX_PF2_MEDIA_SCALING_NOT_SUPPORTED		0x0032

// Error codes for AVI InfoFrame RCR: 1022131
#define IGFX_INVALID_QUAN_RANGE		0x0033
#define IGFX_INVALID_SCAN_INFO		0x0034



//Error code for I2C/DDC Bus access
#define IGFX_INVALID_BUS_TYPE					0x0035
#define IGFX_INVALID_OPERATION_TYPE				0x0036
#define IGFX_INVALID_BUS_DATA_SIZE				0x0037
#define IGFX_INVALID_BUS_ADDRESS				0x0038
#define IGFX_INVALID_BUS_DEVICE					0x0039
#define IGFX_INVALID_BUS_FLAGS					0x0040

// Error codes for RCR: 1022465
#define IGFX_INVALID_POWER_PLAN					0x0041
#define IGFX_INVALID_POWER_OPERATION			0x0042

//Error codes for AUX API
#define IGFX_INVALID_AUX_DEVICE					0x0043
#define IGFX_INVALID_AUX_ADDRESS				0x0044
#define IGFX_INVALID_AUX_DATA_SIZE				0x0045
#define IGFX_AUX_DEFER							0x0046
#define IGFX_AUX_TIMEOUT						0x0047
#define IGFX_AUX_INCOMPLETE_WRITE		0x0048

// Error codes for PAR: CUI 3.5
#define IGFX_INVALID_PAR_VALUE		0x0049
#define IGFX_NOT_ENOUGH_RESOURCE 		0x004A
#define IGFX_NO_S3D_MODE  		0x004B

// Error codes for PAR: CUI 3.5
#define IGFX_S3D_ALREADY_IN_USE_BY_ANOTHER_PROCESS	0x004C
#define IGFX_S3D_INVALID_MODE_FORMAT 0x004D
#define IGFX_S3D_INVALID_MONITOR_ID	0x004E
#define IGFX_S3D_DEVICE_NOT_PRIMARY	0x004F

//#define IGFX_LAYOUT_ERROR			0x0037. Removing as it is not used.

/************************************************************
*            Gamma Ramp specific flags						*
*************************************************************/
// Number of elements in each array of gamma ramp
#define MAX_GAMMA_ELEMENTS	256

// Max value of any gamma ramp element
#define MAX_GAMMA_ELEMENT_VALUE	0xFF00

// Max number of elements in each array of the gamma ramp from index 0 that can 
// have value = 0; this is to prevent excessive darkness of the screen
#define DARKNESS_INDEX		50

// Max number of elements in each array of the gamma ramp from index 0 
// that can have max value. This is to prevent excessive brightness
#define BRIGHTNESS_INDEX	175	

// the numbers DARKNESS_INDEX, BRIGHTNESS_INDEX and MAX_GAMMA_ELEMENT_VALUE
// are fixed based on the gamma ramp calculation that is done by
// CUI. In the calculation, after a certain limit, the ramp is clamped.
// also the calulation uses only the higer 8 bits of each element
// and neglect the lower 8 bits. 
/************************************************************
*            Intel specific display types					*
*************************************************************/
#define IGFX_NULL_DEVICE						0x0000
#define IGFX_CRT								0x0001
#define IGFX_LocalFP							0x0002
#define IGFX_ExternalFP							0x0003
#define IGFX_TV									0x0004
#define IGFX_HDMI								0x0005
#define IGFX_NIVO								0x0006
#define IGFX_DP									0x0007
#define IGFX_FLAG_HDMI_DEIVCE_SUPPORT			(1<<31)
#define IGFX_FLAG_NIVO_DEIVCE_SUPPORT			(1<<30)
#define IGFX_FLAG_DP_DEVICE_SUPPORT				(1<<29)
#define IGFX_ENABLE_PASSIVE_DETECTION			(1<<16)
/************************************************************
*            TV Connector Specific Flags					*
*************************************************************/
#define IGFX_FORCE_TV					(1<<17) // Same as CUIDEVICE_IGFX_ANY_TV in UserStructs.h

#define IGFX_FLAG_DACMODERGB        (1<<0 )	// SCART
#define IGFX_FLAG_DACMODEYC         (1<<1 )	// SVIDEO
#define IGFX_FLAG_DACMODECOMPOSITE  (1<<2 )	// RCA Composite
#define IGFX_FLAG_DACMODEHDTV       (1<<3 )
#define IGFX_FLAG_DACMODEHDRGB      (1<<4 ) 
#define IGFX_FLAG_DACMODECOMPONENT  (1<<5 )	// Component YPrPb
#define IGFX_FLAG_DACMODEDCONNECTOR IGFX_FLAG_DACMODECOMPONENT // flag for d-connector, same as component
#define IGFX_FLAG_AUTO_CONNECTOR_SELECTION (1<<6)

/************************************************************
*            Get/Set RR Specific Flags/Data Structures		*
*************************************************************/
#define INTERLACED			0x01
#define PROGRESSIVE			0x02
#define MAX_REFRESH_RATE	0x14

typedef struct _RefreshRate
{
	unsigned short usRefRate;
	unsigned short usReserved; // for usScanType Interlaced or progressive
}RefreshRate, *pRefreshRate;

typedef struct _DISPLAY_CONFIG
{
	DWORD uidMonitorPrimary;	// Primary Monitor ID
	DWORD uidMonitorSecondary;	// Secondary Monitor ID
	DWORD XResolution;			// Horizontal Resolution to Set
	DWORD YResolution;			// Vertical Resolution to Set
	DWORD BPP;					// Bits Per Pixel/Color Depth to Set
}Display_Config, *pDisplay_Config;

////////////////////////////////////////////////////////////////////////////////////
//
// Display configuration struct Ver 1.0
//
// GUID Definition - {059D08C1-6475-42aa-BD93-2EAA0ECB241F}
static const GUID IGFX_DISPLAY_CONFIG_GUID_1_0 = 
{ 0x59d08c1, 0x6475, 0x42aa, { 0xbd, 0x93, 0x2e, 0xaa, 0xe, 0xcb, 0x24, 0x1f } };


static const GUID IGFX_DISPLAY_CONFIG_GUID_1_1 = 
{ 0x4ea14ecf, 0x9769, 0x44cc, { 0xbb, 0xa2, 0xdc, 0x5d, 0xa9, 0xfd, 0x50, 0xb6 }};


// Structure definition
typedef struct _IGFX_DISPLAY_CONFIG_1_1
{
	// Filled in by the client/caller
	UINT	nSize;						// Filled in by the caller allocating the structure
	
	// Device identification
	DWORD	uidMonitor;					// Unique ID number for the requested monitor

	// Flags member describing validity of output members
	DWORD	dwFlags;					// Flag bitmask

	// Following methods are returned by the interface
	// Panning related
	BOOL	bHorizontalPanningEnabled;	// Horizontal panning enabled ?
	BOOL	bVerticalPanningEnabled;	// Vertical panning enabled ?
	POINT	ptViewPortPosition;			// Viewport position when in panning

	// Panel fitting related
	LONG	lHorizontalScaling;			// Horizontal scaling: -1, 0, 1
	LONG	lVerticalScaling;			// Vertical scaling: -1, 0, 1
	UINT	ulPhysicalWidth;			// Physical width of display
	UINT	ulPhysicalHeight;			// Physical width of display
	UINT	ulDisplayWidth;				// Resolution X
	UINT	ulDisplayHeight;			// Resolution Y
	
	// Rotation related
	BOOL	bRotationEnabled;			// Rotation Enabled?
	BOOL	bPortraitPolicy;			// Portrait orientation policy
	BOOL	bLandscapePolicy;			// Landscape orientation policy
	DWORD	dwOrientation;				// Orientation of the display

}IGFX_DISPLAY_CONFIG_1_1, *PIGFX_DISPLAY_CONFIG_1_1;

// Definition of values for scaling
#define IGFX_DISPLAY_SCALING_UP		(LONG)1
#define IGFX_DISPLAY_SCALING_NONE	(LONG)0
#define IGFX_DISPLAY_SCALING_DOWN	(LONG)-1

// Definition of values for orientation
#define IGFX_DISPLAY_ORIENTATION_0		0x00
#define IGFX_DISPLAY_ORIENTATION_90		0x01
#define IGFX_DISPLAY_ORIENTATION_180	0x02
#define IGFX_DISPLAY_ORIENTATION_270	0x03
#define IGFX_DISPLAY_ORIENTATION_MASK	0x0F

// Definition of dwFlags member
#define IGFX_DISPLAY_CONFIG_FLAG_HORIZONTAL_PANNING		0x0001		// Horizontal panning
#define IGFX_DISPLAY_CONFIG_FLAG_VERTICAL_PANNING		0x0002		// Vertical panning
#define IGFX_DISPLAY_CONFIG_FLAG_HORIZONTAL_SCALING		0x0004		// Horizontal scaling
#define IGFX_DISPLAY_CONFIG_FLAG_VERTICAL_SCALING		0x0008		// Vertical scaling
#define IGFX_DISPLAY_CONFIG_FLAG_PHYSICAL_WIDTH			0x0010		// Physical width
#define IGFX_DISPLAY_CONFIG_FLAG_PHYSICAL_HEIGHT		0x0020		// Physical height
#define IGFX_DISPLAY_CONFIG_FLAG_DISPLAY_WIDTH			0x0040		// Display width
#define IGFX_DISPLAY_CONFIG_FLAG_DISPLAY_HEIGHT			0x0080		// Display height
#define IGFX_DISPLAY_CONFIG_FLAG_ORIENTATION			0x0100		// Orientation
#define IGFX_DISPLAY_CONFIG_FLAG_VIEWPORT				0x0200		// Viewport while panning
#define IGFX_DISPLAY_CONFIG_FLAG_ROTATION				0x0400		// Rotation 
#define IGFX_DISPLAY_CONFIG_FLAG_ROTATION_PORTRAIT		0x0800		// Portrait policy
#define IGFX_DISPLAY_CONFIG_FLAG_ROTATION_LANDSCAPE		0x1000		// Portrait policy


////////////////////////////////////////////////////////////////////////////////////

// Definition of pdwStatus DWORD flag used in EnumAttachableDevices method
#define IGFX_DISPLAY_DEVICE_NOTATTACHED	(0x001)	// No display device currently attached
#define IGFX_DISPLAY_DEVICE_ATTACHED	(0x002)	// Attached
#define IGFX_DISPLAY_DEVICE_OVERRIDE	(0x004)	// Policy indicates Unattached device 
												// will be treated as Attached
#define IGFX_DISPLAY_DEVICE_ACTIVE		(0x010)	// Active device 
#define IGFX_DISPLAY_DEVICE_PRIMARY		(0x100)	// Primary Display Device
#define IGFX_DISPLAY_DEVICE_SECONDARY	(0x200)	// Secondary Display Device


////////////////////////////////////////////////////////////////////////////////////
//
// Device Displays structure Ver 1.0
//

// Interface GUID Definition - {381ECED3-EFF1-4df2-B560-6648E352B18E}, 
//
static const GUID IGFX_DEVICE_DISPLAYS_GUID_1_0 = { 0x381eced3, 0xeff1, 0x4df2, { 0xb5, 0x60, 0x66, 0x48, 0xe3, 0x52, 0xb1, 0x8e } };


#define MAX_MONITORS_PER_ADAPTER			6	// The maximum number of monitors that 
												// can be attached to one Graphics Device

#ifndef ALREADY_DEFINED_IN_IDL

#define ALREADY_DEFINED_IN_IDL

typedef struct DEVICE_DISPLAYS 
{
	UINT	nSize;
	WCHAR	strDeviceName[40];			// Win32 Graphics Device Name 
	DWORD	dwFlags;					// Configuration of the Monitors
	DWORD	uidPrimaryMonitor;			// ID of the Primary Monitor
	UINT 	nMonitors;				// number of elements in the array of ID’s
	DWORD	puidMonitor[MAX_MONITORS_PER_ADAPTER];	// Array of Monitor ID’s attached 
}DEVICE_DISPLAYS, * PDEVICE_DISPLAYS;

#endif // ALREADY_DEFINED_IN_IDL


// Definition of dwFlags in DEVICE_DISPLAYS struct
#define IGFX_DISPLAY_DEVICE_CONFIG_FLAG_SINGLE		(0x1)// Single-Display. Default 
#define IGFX_DISPLAY_DEVICE_CONFIG_FLAG_DDTWIN		(0x2)// Dual-Display Twin
#define IGFX_DISPLAY_DEVICE_CONFIG_FLAG_DDCLONE		(0x3)// Dual-Display Clone
#define IGFX_DISPLAY_DEVICE_CONFIG_FLAG_DDZOOM		(0x4)// Dual-Display Zoom LEGACY
#define IGFX_DISPLAY_DEVICE_CONFIG_FLAG_DDEXTD		(0x5)// Dual-Display Extended Desktop

//Change Begin - Power API.
//Power Conservation Capabilities flags.
#define IGFX_DFGT_SUPPORTED				0x01
#define IGFX_DPST_SUPPORTED				0x02
#define IGFX_ADB_SUPPORTED_AC			0x04
#define IGFX_ADB_SUPPORTED_DC			0x08
#define IGFX_DFGT_AC_SUPPORTED			0x10


// DFGT in DC Mode
//DFGT Structure GUID - {8A2A926D-FF9D-4737-935B-96BF28805B73}
//The following are the various formats of the GUID.

//Registry Format : {8A2A926D-FF9D-4737-935B-96BF28805B73}
//static const GUID IGFX_DFGT_POLICY_GUID_1_0 = { 0x8a2a926d, 0xff9d, 0x4737, { 0x93, 0x5b, 0x96, 0xbf, 0x28, 0x80, 0x5b, 0x73 } };
//DEFINE_GUID(IGFX_DFGT_POLICY_GUID_1_0, 0x8a2a926d, 0xff9d, 0x4737, 0x93, 0x5b, 0x96, 0xbf, 0x28, 0x80, 0x5b, 0x73);
static const GUID IGFX_DFGT_POLICY_GUID_1_0 = { 0x8a2a926d, 0xff9d, 0x4737, { 0x93, 0x5b, 0x96, 0xbf, 0x28, 0x80, 0x5b, 0x73 } };

//Structure Definition for DFGT
typedef struct _IGFX_DFGT_POLICY_1_0
{
	BOOL bEnabled;
}IGFX_DFGT_POLICY_1_0, *PIGFX_DFGT_POLICY_1_0;

// RCR 929546 -- DFGT in AC Mode
// {A119659E-7078-4f5f-B769-AD33668B160D}
static const GUID IGFX_DFGT_POLICY_GUID_1_1 = { 0xa119659e, 0x7078, 0x4f5f, { 0xb7, 0x69, 0xad, 0x33, 0x66, 0x8b, 0x16, 0xd } };

//DPST Structure GUID - {6D24D96F-019C-4baa-ACD5-D48B78050325}
//The following are the various formats of the GUID.

// {6D24D96F-019C-4baa-ACD5-D48B78050325}
//static const GUID IGFX_DPST_POLICY_GUID_1_0 = { 0x6d24d96f, 0x19c, 0x4baa, { 0xac, 0xd5, 0xd4, 0x8b, 0x78, 0x5, 0x3, 0x25 } };
//DEFINE_GUID(IGFX_DPST_POLICY_GUID_1_0, 0x6d24d96f, 0x19c, 0x4baa, 0xac, 0xd5, 0xd4, 0x8b, 0x78, 0x5, 0x3, 0x25);
static const GUID IGFX_DPST_POLICY_GUID_1_0 = { 0x6d24d96f, 0x19c, 0x4baa, { 0xac, 0xd5, 0xd4, 0x8b, 0x78, 0x5, 0x3, 0x25 } };

//Structure Definition for DPST
typedef struct _IGFX_DPST_POLICY_1_0
{
	BOOL  bEnabledDC;
	BOOL  bReserved1; //Reserved for now.
	ULONG ulMaxLevels;
	ULONG ulCurrentAggressivenessDC;
	ULONG ulReserved2; //Reserved for now.
}IGFX_DPST_POLICY_1_0, *PIGFX_DPST_POLICY_1_0;

//ADB Structure GUID - {2B3C5F6C-97B7-4c85-B4F2-335719D24F3D}
//The following are the various formats of the GUID.

//{2B3C5F6C-97B7-4c85-B4F2-335719D24F3D}
//static const GUID IGFX_ADB_POLICY_GUID_1_0 = { 0x2b3c5f6c, 0x97b7, 0x4c85, { 0xb4, 0xf2, 0x33, 0x57, 0x19, 0xd2, 0x4f, 0x3d } };
//DEFINE_GUID(IGFX_ADB_POLICY_GUID_1_0, 0x2b3c5f6c, 0x97b7, 0x4c85, 0xb4, 0xf2, 0x33, 0x57, 0x19, 0xd2, 0x4f, 0x3d);
static const GUID IGFX_ADB_POLICY_GUID_1_0 = { 0x2b3c5f6c, 0x97b7, 0x4c85, { 0xb4, 0xf2, 0x33, 0x57, 0x19, 0xd2, 0x4f, 0x3d } };

//Structure Definition for ADB
typedef struct _IGFX_ADB_POLICY_1_0
{
	BOOL  bEnabledDC;
	BOOL  bEnabledAC; //Reserved for now.
	ULONG ulReserved1;
	ULONG ulReserved2;
	ULONG ulReserved3; //Reserved for now.
}IGFX_ADB_POLICY_1_0, *PIGFX_ADB_POLICY_1_0;

//Policy ID Definitions
#define IGFX_POWER_POLICY_DFGT				0x1
#define IGFX_POWER_POLICY_DPST				0x2
#define IGFX_POWER_POLICY_ADB				0x3
#define IGFX_POWER_POLICY_FEATURECONTROL	0x4 // Enable/Disable FBC/C3SR

// Feature Control
// {81D82F3B-E0D6-4138-9ED9-4CC4FE508906}
static const GUID IGFX_FEATURE_CONTROL_GUID_1_0 = 
{ 0x81d82f3b, 0xe0d6, 0x4138, { 0x9e, 0xd9, 0x4c, 0xc4, 0xfe, 0x50, 0x89, 0x6 } };

#define IGFX_ENABLE_ALL_PC_FEATURES	0x00
#define IGFX_DISABLE_C3SR_RMPM		0x01
#define IGFX_DISABLE_FBC_S2DDT		0x02
#define IGFX_DISABLE_GSV_DFGT		0x04
#define IGFX_DISABLE_BLC			0x08
#define	IGFX_DISABLE_BIA_DPST		0x10
#define IGFX_DISABLE_ALS			0x20

//Change End - Power API.

// Get/Set Gamma Ramp

typedef struct _GAMMARAMP
{
	unsigned short Red[MAX_GAMMA_ELEMENTS];
	unsigned short Green[MAX_GAMMA_ELEMENTS];
	unsigned short Blue[MAX_GAMMA_ELEMENTS];
} GAMMARAMP, *pGAMMARAMP;

// Aspect Scaling Refer igfxcfg\Publics.h and Source\inc\common\dispcomp.h
#define IGFX_CENTERING		0x01
#define IGFX_PANEL_FITTING	0x02
#define IGFX_ASPECT_SCALING	0x04
#define IGFX_SCALING_CUSTOM	0x08
// for PF2 Media Scalar
#define IGFX_MEDIASCALING_FS	0x10
#define IGFX_MEDIASCALING_MAS	0x20
#define IGFX_MAINTAIN_DISPLAY_SCALING	0x40

/////////////////// HDTV API STARTS ////////////////
typedef struct
{
    DWORD           dwValue;    //Current Value
    DWORD           dwDefault;  //Default Value 
    DWORD           dwMin;      //Minimium Value 
    DWORD           dwMax;      //Maximium Value 
    DWORD           dwStep;     //Adjustment value for each step
}IGFX_EXTV_DATA, *pIGFX_EXTV_DATA;

typedef  struct { 
	DWORD           dwFlags;
	/* Standard TV parameters */
	DWORD           dwMode;
	DWORD           dwTVStandard;
	DWORD			dwAvailableTVStandard;
	IGFX_EXTV_DATA   PositionX;  
	IGFX_EXTV_DATA   PositionY; 
	IGFX_EXTV_DATA   SizeX; 
	IGFX_EXTV_DATA   SizeY; 
	IGFX_EXTV_DATA   Brightness; 
	IGFX_EXTV_DATA   Contrast; 
	IGFX_EXTV_DATA   Flicker; 
	/* Extended TV parameters */
	IGFX_EXTV_DATA   Sharpness; 
	IGFX_EXTV_DATA   AdaptiveFlicker; 
	IGFX_EXTV_DATA   TwoDFlicker; 
	IGFX_EXTV_DATA   Saturation; 
	IGFX_EXTV_DATA   Hue; 
	IGFX_EXTV_DATA   DotCrawl; 
	IGFX_EXTV_DATA   LumaFilter; 
	IGFX_EXTV_DATA   ChromaFilter;
	DWORD			 dwLetterBox;
}IGFX_TV_PARAMETER_DATA, *pIGFX_TV_PARAMETER_DATA;

//Std TV Data
#define IGFX_FLAG_TV_POSITIONX			(0x01<<0)
#define IGFX_FLAG_TV_POSITIONY			(0x01<<1)
#define IGFX_FLAG_TV_SIZEX				(0x01<<2)
#define IGFX_FLAG_TV_SIZEY				(0x01<<3)
#define IGFX_FLAG_TV_BRIGHTNESS			(0x01<<4)
#define IGFX_FLAG_TV_CONTRAST			(0x01<<5)
#define IGFX_FLAG_TV_FLICKER			(0x01<<6)

//Video Parameter Data
#define IGFX_FLAG_TV_STANDARD			(0x01<<7)
#define IGFX_FLAG_TV_MODE				(0x01<<8)

//Ext TV Data
#define IGFX_FLAG_TV_SHARPNESS         (0x01<<9)
#define IGFX_FLAG_TV_ADAPTIVEFLICKER   (0x01<<10)
#define IGFX_FLAG_TV_TWODFLICKER       (0x01<<11)
#define IGFX_FLAG_TV_SATURATION        (0x01<<12)
#define IGFX_FLAG_TV_HUE               (0x01<<13)
#define IGFX_FLAG_TV_DOTCRAWL          (0x01<<14)
#define IGFX_FLAG_TV_LUMAFILTER        (0x01<<15) 
#define IGFX_FLAG_TV_CHROMAFILTER      (0x01<<16)

//TV Letter Box Status
#define IGFX_FLAG_TV_LETTERBOX    (0x01<<24)
//support for TV Aspect Scaling RCR
#define IGFX_FLAGS_TV_ASPECT_SCALING  (0x01<<25)
//TV Letter Box Status FLAGS
#define IGFX_EXTV_LETTERBOX_ON   (0x01)
#define IGFX_EXTV_LETTERBOX_OFF  (0x01<<1)
//support for TV Aspect Scaling FLAGS
#define IGFX_EXTV_ASPECT_SCALING_4_3       (0x01<<2)
#define IGFX_EXTV_ASPECT_SCALING_16_9      (0x01<<3)



// Standard TV
#define IGFX_TV_STANDARD_NTSC_M   0x0001  //        75 IRE Setup
#define IGFX_TV_STANDARD_NTSC_M_J 0x0002  // Japan,  0 IRE Setup
#define IGFX_TV_STANDARD_PAL_B    0x0004
#define IGFX_TV_STANDARD_PAL_D    0x0008
#define IGFX_TV_STANDARD_PAL_H    0x0010
#define IGFX_TV_STANDARD_PAL_I    0x0020
#define IGFX_TV_STANDARD_PAL_M    0x0040
#define IGFX_TV_STANDARD_PAL_N    0x0080
#define IGFX_TV_STANDARD_SECAM_B  0x0100  
#define IGFX_TV_STANDARD_SECAM_D  0x0200
#define IGFX_TV_STANDARD_SECAM_G  0x0400
#define IGFX_TV_STANDARD_SECAM_H  0x0800
#define IGFX_TV_STANDARD_SECAM_K  0x1000
#define IGFX_TV_STANDARD_SECAM_K1 0x2000
#define IGFX_TV_STANDARD_SECAM_L  0x4000
#define IGFX_TV_STANDARD_WIN_VGA  0x8000	//  VGA graphics
#define IGFX_TV_STANDARD_NTSC_433 0x00010000
#define IGFX_TV_STANDARD_PAL_G    0x00020000
#define IGFX_TV_STANDARD_PAL_60   0x00040000
#define IGFX_TV_STANDARD_SECAM_L1 0x00080000
#define IGFX_TV_STANDARD_PAL_NC   0x00100000	// New Policy

// HDTV standard definition added using the unused upper 12 bits of dwTVStandard
#define IGFX_HDTV_SMPTE_170M_480i59   0x00100000
#define IGFX_HDTV_SMPTE_293M_480p60   0x00200000
#define IGFX_HDTV_SMPTE_293M_480p59   0x00400000
#define IGFX_HDTV_ITURBT601_576i50    0x00800000
#define IGFX_HDTV_ITURBT601_576p50    0x01000000
#define IGFX_HDTV_SMPTE_296M_720p50   0x02000000
#define IGFX_HDTV_SMPTE_296M_720p59   0x04000000
#define IGFX_HDTV_SMPTE_296M_720p60   0x08000000
#define IGFX_HDTV_SMPTE_274M_1080i50  0x10000000
#define IGFX_HDTV_SMPTE_274M_1080i59  0x20000000
#define IGFX_HDTV_SMPTE_274M_1080i60  0x40000000
#define IGFX_HDTV_SMPTE_274M_1080p60  0x80000000

/////////////////// HDTV API ENDS ////////////////

//////////// INVERTER POWER POLICIES END /////////////

// PWM FREQUENCY SET
// {814472D8-36EE-4afd-BAB0-D680EEBB0034}
static const GUID IGFX_POWER_PARAMS_1_0 = { 0x814472d8, 0x36ee, 0x4afd, { 0xba, 0xb0, 0xd6, 0x80, 0xee, 0xbb, 0x0, 0x34 } };

// Policy ID:
#define	IGFX_BACKLIGHT_POLICY	0x1

typedef struct _IGFX_POWER_PARAMS_0
{
    UINT	uiInverterType;
	ULONG	ulInverterFrequency;	//units of Hz
}IGFX_POWER_PARAMS_0, *pIGFX_POWER_PARAMS_0;


// Inverter Types Flags. Keep them same as UserStructs.h
#define	IGFX_INVERTER_I2C     0x01 //INVERTERI2C
#define	IGFX_INVERTER_PWM     0x02 //INVERTER_PWM
#define IGFX_INVERTER_UNKNOWN 0x04 //INVERTER_UNKNOWN

/*
inverterType parameter can be any one of the flags above. Today driver supports get/set 
for only IGFX_INVERTER_PWM

pwmInverterFrequency can be the current Inverter frequency (for Get call) or the frequency 
to set (Set Call)
*/

//////////// INVERTER POWER POLICIES END /////////////

// {CEB7E9C6-00BF-4985-BBF0-917D8FD9BB59}
static const GUID IGFX_CURRENT_CONFIG_1_0 = { 0xceb7e9c6, 0xbf, 0x4985, { 0xbb, 0xf0, 0x91, 0x7d, 0x8f, 0xd9, 0xbb, 0x59 } };

/////////// ICUIEXTERNAL8 START /////////////////////

/****************************************************************************/
//////// DVMT ////////

// {2C70E8A2-70FA-402a-99E6-5D663C629EB8}
static const GUID IGFX_DVMT_GUID_1_0 = { 0x2c70e8a2, 0x70fa, 0x402a, { 0x99, 0xe6, 0x5d, 0x66, 0x3c, 0x62, 0x9e, 0xb8 } };

// Note: This is supported by ICUIExternal8::GetDeviceData only

typedef struct _IGFX_DVMT_1_0
{
	/*OUT*/ DWORD dwMinDVMTMemory;
	/*OUT*/ DWORD dwMaxDVMTMemory;
	/*OUT*/ DWORD dwCurrentUsedDVMTMemory;
	/*OUT*/	DWORD dwTotalSystemMemory; // Reserved for Vista
}IGFX_DVMT_1_0, *PIGFX_DVMT_1_0;
//Note: Vista - Does not support Current DVMT; Something called total memory


/****************************************************************************/
//////// OVERLAY ////////

// {EE7D1AA9-872D-40d2-9742-1258F5187B73}
static const GUID IGFX_OVERLAY_GUID_1_0 = { 0xee7d1aa9, 0x872d, 0x40d2, { 0x97, 0x42, 0x12, 0x58, 0xf5, 0x18, 0x7b, 0x73 } };

// Note: This is supported by ICUIExternal8::GetDeviceData only

typedef struct _IGFX_OVERLAY_1_0
{
	/*OUT*/ BOOL bIsOverlayActive;
}IGFX_OVERLAY_1_0, *PIGFX_OVERLAY_1_0;
//Note: Any WM_DISPLAYCHANGE or DEVICECHANGE, Video Full Screen (ALT+Enter),  causes overlay ON-OFF issue, other than the app start.


/****************************************************************************/
//////// EDID ////////

// {897843AE-998A-4eec-A21E-EA642B4490EC}
static const GUID IGFX_EDID_GUID_1_0 = { 0x897843ae, 0x998a, 0x4eec, { 0xa2, 0x1e, 0xea, 0x64, 0x2b, 0x44, 0x90, 0xec } };

// Note: This is supported by ICUIExternal8::GetDeviceData only

typedef struct _IGFX_EDID_1_0
{
	/*IN*/  DWORD dwDisplayDevice;
	/*IN*/  DWORD dwEDIDBlock;
	/*OUT*/ DWORD dwEDIDVersion;
	/*OUT*/ unsigned char EDID_Data[256];
}IGFX_EDID_1_0, *PIGFX_EDID_1_0;


/****************************************************************************/
//////// SCALING ////////

// {69A26315-B0DF-4d66-8FCA-0F164843E79E}
static const GUID IGFX_SCALING_GUID_1_0 = { 0x69a26315, 0xb0df, 0x4d66, { 0x8f, 0xca, 0xf, 0x16, 0x48, 0x43, 0xe7, 0x9e } };

// Note: This is supported by both ICUIExternal8::GetDeviceData and ICUIExternal8::SetDeviceData

// This should be same as CUI_CUSTOM_SCALING
typedef struct _IGFX_CUSTOM_SCALING
{
	DWORD	dwCustomScalingMax;
	DWORD	dwCustomScalingMin;
	DWORD	dwCustomScalingCurrent;
	DWORD	dwCustomScalingStep;
	DWORD	dwCustomScalingDefault;
}IGFX_CUSTOM_SCALING, *PIGFX_CUSTOM_SCALING;

typedef struct _IGFX_DISPLAY_RESOLUTION
{
	DWORD dwHzRes;
	DWORD dwVtRes;
	DWORD dwRR;
	DWORD dwBPP;
}IGFX_DISPLAY_RESOLUTION, *PIGFX_DISPLAY_RESOLUTION;

typedef struct _IGFX_SCALING_1_0
{
	/*IN*/  DWORD					dwPrimaryDevice;
	/*IN*/  DWORD					dwSecondaryDevice;
	/*IN*/  DWORD					dwOperatingMode;
	/*IN*/  BOOL					bIsSecondaryDevice; //If secondary device information is required!
	/*IN*/  IGFX_DISPLAY_RESOLUTION	PrimaryResolution;
	/*IN*/  IGFX_DISPLAY_RESOLUTION	SecondaryResolution;
	/*OUT*/ DWORD					dwCurrentAspectOption;
	/*OUT*/ DWORD					dwSupportedAspectOption;
	/*OUT*/ IGFX_CUSTOM_SCALING		customScalingX;
	/*OUT*/ IGFX_CUSTOM_SCALING		customScalingY;
}IGFX_SCALING_1_0, *PIGFX_SCALING_1_0;

/****************************************************************************/

/****************** MCCS START ************************************/
// {6AB2DFDE-DC65-449c-9714-AA43E3C66158}
static const GUID IGFX_MCCS_GUID_1_0 = { 0x6ab2dfde, 0xdc65, 0x449c, { 0x97, 0x14, 0xaa, 0x43, 0xe3, 0xc6, 0x61, 0x58 } };

#define	MAX_NUM_MCCS_CTLS	0xFF // Same as CUI_MAX_NUM_MCCS_PARAMS UserStructs.h
#define	MAX_NUM_VALUES		0x0F // same as CUI_MAX_NUM_VALUES UserStructs.h
#define	MCCS_OPEN			0x01
#define	MCCS_CLOSE			0x02
#define	MCCS_GET_MAX		0x03
#define	MCCS_GET_MIN		0x04
#define	MCCS_GET_CURRENT	0x05
#define	MCCS_SET_CURRENT	0x06
#define	MCCS_RESET_CONTROL	0x07
#define	MCCS_GET_SUPPORTED	0x08
#define	MCCS_GET_POSSIBLE	0x09

typedef struct _IGFX_MCCS_DATA
{
	/*IN*/		DWORD	Cmd;
	/*IN OUT*/		UINT	uiHandle;
	/*IN*/		DWORD	dwDevice;
	/*IN*/		UINT	uiControlCode;
	/*IN*/		UINT	uiSize;
	/*IN*/		DWORD	dwNCValueIndex;
	/*IN OUT*/	DWORD	dwValue;
	/*OUT*/		UINT	iNumSupportedControls;
	/*OUT*/		UINT	iSupportedControls[MAX_NUM_MCCS_CTLS];
	/*OUT*/		UINT	iNCValue[MAX_NUM_VALUES];
}IGFX_MCCS_DATA, *PIGFX_MCCS_DATA;


/****************** MCCS END *********************************************/

/****************** TV Format - New Policy START ************************************/
// {C50130D9-DC02-4f55-A0CD-E824DB8A4772}
static const GUID IGFX_TV_FORMAT_GUID_1_0 = { 0xc50130d9, 0xdc02, 0x4f55, { 0xa0, 0xcd, 0xe8, 0x24, 0xdb, 0x8a, 0x47, 0x72 } };

#define	IGFX_TV_TYPE_STANDARD	0x00
#define	IGFX_TV_TYPE_HDTV		0x01

typedef struct _IGFX_TV_FORMAT_EX
{
	/*IN*/		DWORD	dwReserved;
	/*IN*/		DWORD	dwDevice;
	/*IN OUT*/	DWORD	dwTVStandard;
	/*IN OUT*/	DWORD	dwTVType;
	/*OUT*/		DWORD	dwAvailableTVStd;
}IGFX_TV_FORMAT_EX, *PIGFX_TV_FORMAT_EX;

/****************** TV Format - New Policy END *********************************************/

/****************** Connector Details START ************************************/
// {2F317ACE-3968-4a57-9667-C6A47B6C4B8C}
static const GUID IGFX_CONNECTOR_STATUS_GUID_1_0 = { 0x2f317ace, 0x3968, 0x4a57, { 0x96, 0x67, 0xc6, 0xa4, 0x7b, 0x6c, 0x4b, 0x8c } };

typedef struct _IGFX_CONNECTOR_STATUS
{
	DWORD dwConnectorSupported;
	DWORD dwConnectorDispAttached;
	DWORD dwConnectorDispActive;
}IGFX_CONNECTOR_STATUS, *PIGFX_CONNECTOR_STATUS;

// make them same as that of Source\inc\common\itvout.h
#define IGFX_CONNECTOR_TYPE_UNKNOWN              0xFFFFFFFF
#define IGFX_CONNECTOR_TYPE_NONE                 0x00000000
#define IGFX_CONNECTOR_TYPE_VGA                  0x00000001 //For CRT 
#define IGFX_CONNECTOR_TYPE_LVDS                 0x00000002	// LFP
#define IGFX_CONNECTOR_TYPE_DVI                  0x00000004	// DFP
#define IGFX_CONNECTOR_TYPE_HDMI                 0x00000008	// HDMI
#define IGFX_CONNECTOR_TYPE_SVIDEO               0x00000010	// TV - SVDIEO
#define IGFX_CONNECTOR_TYPE_COMPOSITE_VIDEO      0x00000020	// TV - COMPOSITE
#define IGFX_CONNECTOR_TYPE_COMPONENT_VIDEO      0x00000040	// TV - COMPONENT
#define IGFX_CONNECTOR_TYPE_DCONNECTOR_VIDEO     0x00000040	// TV - D-CONNECTOR
#define IGFX_CONNECTOR_TYPE_SCART_VIDEO          0x00000080	// TV - SCART
#define IGFX_CONNECTOR_TYPE_DISPLAY_PORT         0x00000100 //Display Port
#define IGFX_CONNECTOR_TYPE_EMBEDDED_DISPLAY_PORT         0x00000200

/****************** Connector Details END *********************************************/

/****************************** TV WIZARD NEEDS ***************************************/
//			ONLY GET IS IMPLEMENTED

static const GUID IGFX_SYSTEM_INFO_GUID_1_0 = 
{ 0x2d23d807, 0x6617, 0x4f92, { 0xb0, 0x1e, 0x51, 0x36, 0x78, 0xa6, 0x23, 0x58 } };


static const GUID IGFX_DISPLAY_DATA_GUID_1_0 = 
{ 0xcc136f73, 0xb8a0, 0x49ee, { 0x97, 0x57, 0xf6, 0xd8, 0xe9, 0x6f, 0x66, 0xb5 } };

/****************************** TV WIZARD NEEDS ***************************************/

/****************** Disable Driver Persistence Algorithm START *********************************************/
// {10690626-F3B2-48d7-A50A-59E91017E273}
static const GUID IGFX_DRIVER_PERSISTENCE_ALGO_DISABLE = 
{ 0x10690626, 0xf3b2, 0x48d7, { 0xa5, 0x0a, 0x59, 0xe9, 0x10, 0x17, 0xe2, 0x73 } };

#define IGFX_DATA_NOT_AVAILABLE			0x00000000
#define IGFX_DISABLE_DRIVER_PERSISTENCE	0x00000001
#define IGFX_ENABLE_DRIVER_PERSISTENCE	0x00000002

/****************** Disable Driver Persistence Algorithm END *********************************************/

/****************** Overlay Gamma START *********************************************/
// {2F96AB2A-3F5F-4f15-A347-0FB005A88EBC}
static const GUID IGFX_OVERLAY_COLOR_GUID_1_0  =
{ 0x2f96ab2a, 0x3f5f, 0x4f15, { 0xa3, 0x47, 0xf, 0xb0, 0x5, 0xa8, 0x8e, 0xbc } };

typedef struct _IGFX_OVERLAY_COLOR_DATA
{
    long lValue;    // Current Value
    long lDefault;  // Default Value 
    long lMin;      // Minimium Value 
    long lMax;      // Maximium Value 
    long lStep;     // Adjustment value for each step
}IGFX_OVERLAY_COLOR_DATA, *PIGFX_OVERLAY_COLOR_DATA;

typedef struct _IGFX_OVERLAY_COLOR_SETTINGS
{
	DWORD dwFlags;
	IGFX_OVERLAY_COLOR_DATA GammaSettings;
	IGFX_OVERLAY_COLOR_DATA BrightnessSettings;
	IGFX_OVERLAY_COLOR_DATA SaturationSettings;
	IGFX_OVERLAY_COLOR_DATA ContrastSettings;
	IGFX_OVERLAY_COLOR_DATA HueSettings;
} IGFX_OVERLAY_COLOR_SETTINGS, *PIGFX_OVERLAY_COLOR_SETTINGS;

#define IGFX_OVERLAY_COLOR_ALL			0x01
#define IGFX_OVERLAY_COLOR_DEFAULT		0x02
#define IGFX_OVERLAY_COLOR_GAMMA		0x04
#define IGFX_OVERLAY_COLOR_BRIGHTNESS	0x08
#define IGFX_OVERLAY_COLOR_SATURATION	0x10
#define IGFX_OVERLAY_COLOR_CONTRAST		0x20
#define IGFX_OVERLAY_COLOR_HUE			0x40

/****************** Overlay Gamma END *********************************************/
/**
	HDMI INFO FRAME STRUCTURES
**/

// {A619D1A8-863F-4a30-AD0E-BB4444067448}
static const GUID IGFX_HDMI_INFOFRAME_1_0 = 
{ 0xa619d1a8, 0x863f, 0x4a30, { 0xad, 0xe, 0xbb, 0x44, 0x44, 0x6, 0x74, 0x48 } };

typedef struct
{
   BOOLEAN ErrorOccured;         // TRUE or FALSE
   DWORD LastSystemErrorVal;     // return from GetLastError() api call.
   //enum CUIErrorCode ExtendedError;   //
   BSTR  ExtendedErrorBstr;      // Optional Use, use for Test purposes only!
}IGFX_ERROR;

typedef enum
{
    IGFX_HDMI_COMMAND_GET,
    IGFX_HDMI_COMMAND_SET
} IGFX_HDMI_COMMAND;

typedef struct _IGFX_IF_HEADER
{
    UCHAR ucType;       // InfoFrame Type
    UCHAR ucVersion;    // InfoFrame Version
    UCHAR ucLength;     // InfoFrame Length
    UCHAR ucChksum;     // Checksum of the InfoFrame
}IGFX_IF_HEADER, *PIGFX_IF_HEADER;

typedef struct _IGFX_VS_IF
{
    IGFX_IF_HEADER IfHeader; // VS header data
    UCHAR ucIEEERegID[3];   // 3-byte IEEE registration ID
    UCHAR ucPayload[24];        // Payload bytes
 
}IGFX_VS_IF, *PIGFX_VS_IF;

typedef struct _IGFX_HDMI_INFOFRAME
{
	IGFX_ERROR ErrorInfo;
	DWORD	dwDeviceID;
    GUID            Guid;
    IGFX_HDMI_COMMAND    dwCommand;
    UCHAR           ucType;
    UCHAR           ucSize;
    IGFX_VS_IF	VS_INFOFRAME;
    
}IGFX_HDMI_INFOFRAME, *PIGFX_HDMI_INFOFRAME;
/**
	HDMI INFO FRAME STRUCTURES END
**/
/*	SDVO Vendor Opcode Start	*/
// {2C1C2738-B9E8-4cc8-B3ED-969ABBB63BFE}
static const GUID IGFX_SDVO_VENDOR_OPCODE_EXECUTION = 
{ 0x2c1c2738, 0xb9e8, 0x4cc8, { 0xb3, 0xed, 0x96, 0x9a, 0xbb, 0xb6, 0x3b, 0xfe } };

// This enum should be same as of SDVO_CMD_STATUS in AIM
// and CUI_SDVO_CMD_STATUS as in UserStructs.h
typedef enum _IGFX_SDVO_CMD_STATUS 
{
	IGFX_SDVO_POWER_ON_STATE	        		= 0,
	IGFX_SDVO_SUCCESS               			= 1,
	IGFX_SDVO_COMMAND_NOT_SUPPORTED         	= 2,
	IGFX_SDVO_INVALID_ARGUEMENT         		= 3,
	IGFX_SDVO_PENDING               			= 4,
	IGFX_SDVO_TARGET_NOT_SPECIFIED      		= 5,
	IGFX_SDVO_DEVICE_SCALING_NOT_SUPPORTED	= 6
}IGFX_SDVO_CMD_STATUS;

typedef struct _IGFX_VENDOR_OPCODE_ARGS
{
			IGFX_ERROR				ErrorInfo;	// Reserved Parameter
	/*IN*/	GUID					guid;
	/*IN*/	DWORD 					dwDeviceAddress;
	/*IN*/	DWORD 					dwOpcode;
	/*IN*/	BYTE   					ParamIn[8];
	/*IN*/	DWORD 					dwParamInCount;
	/*OUT*/ BYTE   					Return[8];
	/*IN*/	DWORD	 				dwReturnCount;
	/*OUT*/ IGFX_SDVO_CMD_STATUS 	CmdStatus;
			DWORD 					Reserved1;
			DWORD 					Reserved2;
}IGFX_VENDOR_OPCODE_ARGS, *PIGFX_VENDOR_OPCODE_ARGS;


/*	SDVO Vendor Opcode End	*/

/* Feature Support Begin */

// {51EC9997-98DE-4172-B4D0-BF8F59D38676}
static const GUID IGFX_FEATURE_SUPPORT = 
{ 0x51ec9997, 0x98de, 0x4172, { 0xb4, 0xd0, 0xbf, 0x8f, 0x59, 0xd3, 0x86, 0x76 } };

typedef struct _IGFX_FEATURE_SUPPORT_ARGS
{
	/*OUT*/ DWORD		dwFeatureSupport;
			DWORD		Reserved1;
}IGFX_FEATURE_SUPPORT_ARGS, *PIGFX_FEATURE_SUPPORT_ARGS;

// Supported Fetaure List
#define	 CLEAR_VIDEO_TECHNOLOGY	0x0001
#define IGFX_TVWIZARD_SUPPORTED 0x02


/* Feature Support End */

/*** STC SDK START ***/
// Supported configurations in the system
// {6D4C394C-D7F8-4123-8483-6B723C537414}
static const GUID IGFX_SUPPORTED_CONFIGURATIONS = 
{ 0x6d4c394c, 0xd7f8, 0x4123, { 0x84, 0x83, 0x6b, 0x72, 0x3c, 0x53, 0x74, 0x14 } };

#define MAX_VALID_CONFIG 140
typedef struct 
{ 
    DWORD       dwOperatingMode;
    DWORD       dwPriDevUID; 		//Device on Primary Display( For Single Pipe Simultaneous mode, both devices are here )
    DWORD       dwSecDevUID; 		//Device on Secondary Display
}IGFX_CONFIG_DATA; 

typedef struct 
{ 
    DWORD 				dwNumTotalCfg;		//Total of validation configuration in the following array 
	DWORD				dwReserved1;		// Reserved
	DWORD				dwReserved2;		// Reserved
    IGFX_CONFIG_DATA	ConfigList[MAX_VALID_CONFIG];	//Valid device combinations, upto 7 devices
}IGFX_TEST_CONFIG, *PIGFX_TEST_CONFIG;

// Supported graphics modes in the system
// {C6978584-3908-457b-8CA6-02CF95857279}
static const GUID IGFX_SUPPORTED_GRAPHICS_MODES = 
{ 0xc6978584, 0x3908, 0x457b, { 0x8c, 0xa6, 0x2, 0xcf, 0x95, 0x85, 0x72, 0x79 } };

#define MAX_GRAPHICS_MODE 1200
typedef struct
{
	DWORD					dwOperatingMode;
	DWORD					dwPriDevUID;
	DWORD					dwSecDevUID;
	BOOL					bIsPrimary;
    WORD					vmlNumModes;        // Number of video modes in list
	DWORD					dwReserved1;		// Reserved
	DWORD					dwReserved2;		// Reserved
    IGFX_DISPLAY_RESOLUTION	vmlModes[MAX_GRAPHICS_MODE];		// List of video modes
}IGFX_VIDEO_MODE_LIST, *PIGFX_VIDEO_MODE_LIST;

// {F54A8104-814F-442f-8A69-15F8F03390E3}
static const GUID IGFX_VBIOS_VERSION_GUID = 
{ 0xf54a8104, 0x814f, 0x442f, { 0x8a, 0x69, 0x15, 0xf8, 0xf0, 0x33, 0x90, 0xe3 } };
// For VBIOS version, input should be DWORD

/*** STC SDK END ***/

/*** RCR 995613 Gamma with CUI Algo START ***/
// {0155F7AC-F8A1-4e64-B2E0-5B58F7821DC3}
static const GUID IGFX_DESKTOP_GAMMA = 
{ 0x155f7ac, 0xf8a1, 0x4e64, { 0xb2, 0xe0, 0x5b, 0x58, 0xf7, 0x82, 0x1d, 0xc3 } };

#define IGFX_REDGAMMA			0x00
#define IGFX_GREENGAMMA			0x01
#define	IGFX_BLUEGAMMA			0x02
#define IGFX_REDBRIGHTNESS		0x03
#define IGFX_GREENBRIGHTNESS	0x04
#define	IGFX_BLUEBRIGHTNESS		0x05
#define IGFX_REDCONTRAST		0x06
#define IGFX_GREENCONTRAST		0x07
#define	IGFX_BLUECONTRAST		0x08

#define IGFX_DOUBLE_PRESISION_GAMMA 0x1000

typedef struct
{
	DWORD dwDeviceUID;
	DWORD dwFlags;	// Reserved
	long  lGammaValues[9];
}IGFX_DESKTOP_GAMMA_ARGS, *PIGFX_DESKTOP_GAMMA_ARGS;
/*** RCR 995613 Gamma with CUI Algo END ***/

/*** RCR 988835 DDC Get/Set RR START ***/
// {BC51E8BC-3A25-402e-9D20-4C3FC4E68BF2}
static const GUID IGFX_GET_SET_CONFIGURATION_GUID = 
{ 0xbc51e8bc, 0x3a25, 0x402e, { 0x9d, 0x20, 0x4c, 0x3f, 0xc4, 0xe6, 0x8b, 0xf2 } };

typedef struct _IGFX_DISPLAY_RESOLUTION_EX
{
	DWORD dwHzRes;				// Horizontal Resolution
	DWORD dwVtRes;				// Vertical Resolution
	DWORD dwRR;					// Refresh Rate
	DWORD dwBPP;				// Color Depth
	DWORD dwSupportedStandard;	// Reserved
	DWORD dwPreferredStandard;	// Reserved
	WORD  InterlaceFlag;		// Resreved
}IGFX_DISPLAY_RESOLUTION_EX, *PIGFX_DISPLAY_RESOLUTION_EX;

typedef struct _IGFX_DISPLAY_POSITION
{
	int	iLeft;			// Position - Left		*********************************************************** 
	int	iRight;			// Position - Right		** Position Fields are optional. They are valid only for **
	int	iTop;			// Position - Top		** Secondary Display device in Extended Desktop.		 **
	int	iBottom;		// Position - Bottom	***********************************************************

}IGFX_DISPLAY_POSITION, *PIGFX_DISPLAY_POSITION;

typedef struct _IGFX_DISPLAY_CONFIG_DATA
{     
    DWORD						dwDisplayUID;   // Display Device UID for this display 
	IGFX_DISPLAY_RESOLUTION_EX	Resolution;     // Display Mode for this display
	IGFX_DISPLAY_POSITION		Position;		// Display Position
	DWORD						dwTvStandard;	// Reserved
	BOOL						bIsHDTV;		// Reserved
	DWORD						dwOrientation;	// Display Orientation
	DWORD						dwScaling;		// Reserved	
}IGFX_DISPLAY_CONFIG_DATA, *PIGFX_DISPLAY_CONFIG_DATA;

typedef struct _IGFX_SYSTEM_CONFIG_DATA
{
	DWORD						dwOpMode;	// Operating Mode
	IGFX_DISPLAY_CONFIG_DATA	PriDispCfg;	// Primary Display Device Config Data
	IGFX_DISPLAY_CONFIG_DATA	SecDispCfg;	// Secondary Display Device Config Data
}IGFX_SYSTEM_CONFIG_DATA, *PIGFX_SYSTEM_CONFIG_DATA;

/*** RCR 988835 DDC Get/Set RR END ***/
/*n View Get Set Config*/
#define IGFX_N_VIEW_CONFIG_SIZE_ONLY			(1<<0)

// {21ADA76B-A70E-4d4e-94E2-4B8EE0CC3284}
static const GUID IGFX_GET_SET_N_VIEW_CONFIG_GUID = 
{ 0x21ada76b, 0xa70e, 0x4d4e, { 0x94, 0xe2, 0x4b, 0x8e, 0xe0, 0xcc, 0x32, 0x84 } };

typedef struct _IGFX_DISPLAY_CONFIG_DATA_EX{     
    DWORD	dwDisplayUID;   // Display Device UID for this display 
	IGFX_DISPLAY_RESOLUTION_EX	Resolution;      // Display Mode
	IGFX_DISPLAY_POSITION	Position;	// Display Position
	DWORD			dwTvStandard;		// Reserved
	BOOL			bIsHDTV;			// Reserved
	DWORD			dwOrientation;		// Orientation
	DWORD			dwScaling;			// Reserved
	DWORD			dwFlags;			// Flags
}IGFX_DISPLAY_CONFIG_DATA_EX, *PIGFX_DISPLAY_CONFIG_DATA_EX;
typedef struct _IGFX_SYSTEM_CONFIG_DATA_N_VIEW {
	DWORD			dwOpMode;		// Operating Mode
	DWORD			dwFlags;		// Flags
	UINT			uiSize;			// Reserved
	UINT			uiNDisplays;	// Reserved
	IGFX_DISPLAY_CONFIG_DATA_EX	DispCfg[1];	// Array of Display Data
}IGFX_SYSTEM_CONFIG_DATA_N_VIEW, *PIGFX_SYSTEM_CONFIG_DATA_N_VIEW;

//CUI 2.5 Features starts here
//Custom Modes
typedef struct _IGFX_CUSTOM_MODE
{
	DWORD dwHzRes;
	DWORD dwVtRes;
	DWORD dwRR;
	DWORD dwBPP;
	BOOL	bInterlacedMode;		
}IGFX_CUSTOM_MODE, *PIGFX_CUSTOM_MODE;

typedef struct _IGFX_ADVANCED_MODE
{
	DWORD dwBPP;		//Color quality
	DWORD dwHFPorch; 	//Hrozontal front porch
	DWORD dwHBPorch; 	//Hrozontal back porch
	DWORD dwHSWidth; 	//Hrozontal Sync width
	DWORD dwHActive; 	//Hrozontal Active pixels
	BOOL  bHSPolarity;	//Hrozontal Sync polarity
	FLOAT fHSRate; 		//Hrozontal Scan rate
	DWORD dwVFPorch; 	//Vertical front porch
	DWORD dwVBPorch; 	//Vertical back porch
	DWORD dwVSWidth; 	//Vertical Sync width
	DWORD dwVActive; 	//Vertical Active pixels
	BOOL  bVSPolarity;	//Vertical Sync polarity
	FLOAT fVSRate; 		//Vertical Scan rate
	FLOAT fPixelClock;	//Pixel Clock
	BOOL	bInterlacedMode;			
}IGFX_ADVANCED_MODE, *PIGFX_ADVANCED_MODE;

#define IGFX_COLOR_QUALITY_8 	8 // 8 BPP 
#define IGFX_COLOR_QUALITY_16 	16// 16 BPP 
#define IGFX_COLOR_QUALITY_32 	32// 32 BPP 
#define IGFX_COLOR_QUALITY_ALL 	(IGFX_COLOR_QUALITY_8 | IGFX_COLOR_QUALITY_16 | IGFX_COLOR_QUALITY_32)// 8, 16 and 32 BPP s at a shot

#define IGFX_TIMING_STANDARD_GTF 	0x0001	// GTF 
#define IGFX_TIMING_STANDARD_CVT 	0x0002	// CVT 
#define IGFX_TIMING_STANDARD_CVT_RB 0x0003	// CVT-RB 
#define IGFX_TIMING_STANDARD_CEA_861_B 	0x0004	// CE 861

#define IGFX_BASIC_CUSTOM_MODES 	0x0001	// Modes added using basic mode details
#define IGFX_ADVANCED_CUSTOM_MODES 	0x0002	// Modes added using timing details 

//Get the custom mode list
// {2CCC237D-4438-4159-BE45-794E2FAF074D}
static const GUID IGFX_GET_CUSTOM_MODELIST_GUID = 
{ 0x2ccc237d, 0x4438, 0x4159, { 0xbe, 0x45, 0x79, 0x4e, 0x2f, 0xaf, 0x7, 0x4d } };

typedef struct _IGFX_CUSTOM_MODELIST
{
	DWORD	dwDisplayUID;   	// Display Device UID for this display
	DWORD dwFlags;				// Flags
	DWORD	dwTotalModes;   	// Number of custom modes for this display 
	IGFX_CUSTOM_MODE	ModeList[MAX_GRAPHICS_MODE]; // List of custom modes	
}IGFX_CUSTOM_MODELIST, *PIGFX_CUSTOM_MODELIST;

//Get the custom mode timing
// {1BE9CBD4-2611-4817-9F7A-DE4B84185C16}
static const GUID IGFX_GET_CUSTOM_MODE_TIMING_GUID = 
{ 0x1be9cbd4, 0x2611, 0x4817, { 0x9f, 0x7a, 0xde, 0x4b, 0x84, 0x18, 0x5c, 0x16 } };

typedef struct _IGFX_CUSTOM_MODE_TIMING_DATA
{
	DWORD	dwDisplayUID;   	// Display Device UID for this display
	DWORD dwFlags;		// Cuurently a reserved parameter
	IGFX_CUSTOM_MODE	AddedMode;	// Added custom mode
	IGFX_ADVANCED_MODE TimingInfo; //Timing details of the added mode	
}IGFX_CUSTOM_MODE_TIMING_DATA, *PIGFX_CUSTOM_MODE_TIMING_DATA;

//Add a mode by specifying the basic mode details
// {0CC2FDB2-8A29-491f-ACCB-63913EEDD6F1}
static const GUID IGFX_ADD_BASIC_CUSTOM_MODE_GUID = 
{ 0xcc2fdb2, 0x8a29, 0x491f, { 0xac, 0xcb, 0x63, 0x91, 0x3e, 0xed, 0xd6, 0xf1 } };

typedef struct _IGFX_ADD_BASIC_CUSTOM_MODE_DATA
{
	DWORD	dwDisplayUID;   	// Display Device UID for this display
	DWORD dwFlags;		// Cuurently a reserved parameter
	BOOL	bForcedModeAddition; // Specifies whether overwrite an existing mode
	FLOAT fUnderscan;			//Underscan Percentage
	DWORD dwTimingStandard;		//Desired timing standard
	IGFX_CUSTOM_MODE BasicMode;	// Basic mode details
}IGFX_ADD_BASIC_CUSTOM_MODE_DATA, *PIGFX_ADD_BASIC_CUSTOM_MODE_DATA;

//Add a mode by specifying the entire timing details
// {FC43F7AB-E9F2-4388-BC2C-FE476C7F4E67}
static const GUID IGFX_ADD_ADVANCED_CUSTOM_MODE_GUID = 
{ 0xfc43f7ab, 0xe9f2, 0x4388, { 0xbc, 0x2c, 0xfe, 0x47, 0x6c, 0x7f, 0x4e, 0x67 } };

typedef struct _IGFX_ADD_ADVANCED_CUSTOM_MODE_DATA
{
	DWORD	dwDisplayUID;   	// Display Device UID for this display
	DWORD dwFlags;		// Cuurently a reserved parameter
	BOOL	bForcedModeAddition; // Specifies whether overwrite an existing mode
	IGFX_ADVANCED_MODE AdvancedMode; // Advanced Mode Details	
}IGFX_ADD_ADVANCED_CUSTOM_MODE_DATA, *PIGFX_ADD_ADVANCED_CUSTOM_MODE_DATA;

//Remove the specified custom modes
// {FE4362FA-0FF8-4d5b-8CCB-BF6A23278A44}
static const GUID IGFX_REMOVE_CUSTOM_MODELIST_GUID = 
{ 0xfe4362fa, 0xff8, 0x4d5b, { 0x8c, 0xcb, 0xbf, 0x6a, 0x23, 0x27, 0x8a, 0x44 } };
//Structure used for this GUID is  IGFX_CUSTOM_MODELIST

//End of Custom modes

//Media Settings features
#define IGFX_MEDIA_FEATURE_CVT 					0x0001 // CVT Feature
#define IGFX_MEDIA_FEATURE_FMD 					0x0002 // Film mode detection 
#define IGFX_MEDIA_FEATURE_NOISE_REDUCTION 		0x0004 // Noise Reduction
#define IGFX_MEDIA_FEATURE_SHARPNESS 			0x0008 // Sharpness 
#define IGFX_MEDIA_FEATURE_COLOR 				0x0010 // Procamp 
#define IGFX_MEDIA_FEATURE_SCALING				0x0020 // NLAS
#define IGFX_MEDIA_FEATURE_TCC					0x0040 // TCC
#define IGFX_MEDIA_FEATURE_STE					0x0080 // STE
#define IGFX_MEDIA_FEATURE_ACE					0x0100 // ACE


typedef struct _IGFX_MEDIA_SETTINGS_DATA
{
    float fValue;    // Current Value
    float fDefault;  // Default Value 
    float fMin;      // Minimium Value 
    float fMax;      // Maximium Value 
    float fStep;     // Adjustment value for each step
}IGFX_MEDIA_SETTINGS_DATA, *PIGFX_MEDIA_SETTINGS_DATA;

typedef struct _IGFX_MEDIA_ENABLE_DATA
{
    BOOL bEnable;		// Current Value
    BOOL bDefault;	// Default Value 
}IGFX_MEDIA_ENABLE_DATA, *PIGFX_MEDIA_ENABLE_DATA;

//Video quality
// {B30262A8-3EEE-482d-A6A0-1DC71A44860F}
static const GUID IGFX_GET_SET_VIDEO_QUALITY_GUID = 
{ 0xb30262a8, 0x3eee, 0x482d, { 0xa6, 0xa0, 0x1d, 0xc7, 0x1a, 0x44, 0x86, 0xf } };

typedef struct _IGFX_VIDEO_QUALITY_DATA
{
	DWORD	dwSupportedFeatures;
	IGFX_MEDIA_ENABLE_DATA	EnableFMD; //Film mode detection
	IGFX_MEDIA_ENABLE_DATA	AlwaysEnableNR; // Noise Reduction 
	IGFX_MEDIA_ENABLE_DATA	AlwaysEnableSharpness; //Sharpness
	DWORD	dwFlags;		//Reserved
	IGFX_MEDIA_SETTINGS_DATA SharpnessSettings;
	
}IGFX_VIDEO_QUALITY_DATA, *PIGFX_VIDEO_QUALITY_DATA;

// {41E9B95E-6A77-45ad-A014-F2B4CCE5356A}
static const GUID IGFX_GET_SET_MEDIA_COLOR_GUID = 
{ 0x41e9b95e, 0x6a77, 0x45ad, { 0xa0, 0x14, 0xf2, 0xb4, 0xcc, 0xe5, 0x35, 0x6a } };

typedef struct _IGFX_MEDIA_COLOR_DATA
{
	DWORD	dwSupportedFeatures;
	IGFX_MEDIA_ENABLE_DATA	EnableAlways; //Enable Always
	DWORD	dwFlags;		//Reserved
	IGFX_MEDIA_SETTINGS_DATA HueSettings;
	IGFX_MEDIA_SETTINGS_DATA SaturationSettings;
	IGFX_MEDIA_SETTINGS_DATA ContrastSettings;
	IGFX_MEDIA_SETTINGS_DATA BrightnessSettings;	
}IGFX_MEDIA_COLOR_DATA, *PIGFX_MEDIA_COLOR_DATA;

// {4936CB65-CAAF-424a-9D50-81E47537B6AE}
static const GUID IGFX_GET_SET_MEDIA_SCALING_GUID = 
{ 0x4936cb65, 0xcaaf, 0x424a, { 0x9d, 0x50, 0x81, 0xe4, 0x75, 0x37, 0xb6, 0xae } };

typedef struct _IGFX_MEDIA_SCALING_DATA
{
	DWORD	dwSupportedFeatures;
	IGFX_MEDIA_ENABLE_DATA	EnableNLAS; //Enable NLAS
	DWORD	dwFlags;		//Reserved
	IGFX_MEDIA_SETTINGS_DATA VerticalCropSettings;
	IGFX_MEDIA_SETTINGS_DATA HLinearRegionSettings;
	IGFX_MEDIA_SETTINGS_DATA NonLinearCropSettings;
}IGFX_MEDIA_SCALING_DATA, *PIGFX_MEDIA_SCALING_DATA;

//end of media seetings apis

/*** RCR 960264 PF2 Media Scalar START ***/
// {1BF943C9-507D-400b-AD0C-F47D4B74871A}
static const GUID IGFX_GET_SET_PF2_MEDIASCALAR_GUID = 
{ 0x1bf943c9, 0x507d, 0x400b, { 0xad, 0xc, 0xf4, 0x7d, 0x4b, 0x74, 0x87, 0x1a } };

typedef struct _IGFX_PF2_MEDIA_SCALAR
{
	BOOL bSupported;		// Is Media Scalar Supported (Get)
	BOOL bEnable;			// Is Media Scalar Enabled (Get), Enable Media Scaling (SET)
	DWORD MediaScalingOption; 		// Media FS or Media MAS (Get/Set)
	IGFX_DISPLAY_RESOLUTION	SourceMode; // Mode to set (Set)
	IGFX_DISPLAY_RESOLUTION	LastMode;	// Mode that was set before enabling Media Scalar (Set)
}IGFX_PF2_MEDIA_SCALAR, *PIGFX_PF2_MEDIA_SCALAR;

/*** RCR 960264 PF2 Media Scalar END ***/

/////////// ICUIEXTERNAL8 END /////////////////////

// ICUIDownScale Interface Definitions Start
typedef struct _IGFX_DOWNSCALING_DATA
{
	BOOL 					bIsSupported;
	IGFX_DISPLAY_RESOLUTION	MaxSupportedMode;
	IGFX_DISPLAY_RESOLUTION	SourceMode;
	IGFX_DISPLAY_RESOLUTION	LastMode;	 
}IGFX_DOWNSCALING_DATA, *PIGFX_DOWNSCALING_DATA;

// ICUIDownScale Interface Definitions End

////////////////////////////////////////////////////////////////////////
//
// IGFXDL_INFORM_DLCAPS 
//
////////////////////////////////////////////////////////////////////////
// {4E0275F0-51DA-49a0-84FC-E56481DE446A}
static const GUID IGFX_DLCAPS_GUID_1_0 =
{ 0x4e0275f0, 0x51da, 0x49a0, { 0x84, 0xfc, 0xe5, 0x64, 0x81, 0xde, 0x44, 0x6a } };

#define IGFX_NIVO_DVI 	1  // Specifies that the device type is DVI

typedef struct _IGFX_DLCAPS
{
	DWORD dwFlags; 

	DWORD dwDLVersion; //(LOWORD ?Minor version, HIWORD ?Major version)

	DWORD dwNumDevices;

	DWORD dwReserved1; //dwIGDSpecVer;
	
	DWORD dwReserved2;
}IGFX_DLCAPS, *PIGFX_DLCAPS;

////////////////////////////////////////////////////////////////////////
//
// RCR 1022131: AVI INFO FRAME
//
////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------
// RCR 1022131: AVI INFOFRAME SDK Interface Flags 
//---------------------------------------------------------------

// Values for specific features

// Scan Info: Only 0, 1 and 2 values are valid. 
//	0 0 (0) 	-	NO_SCAN
//	0 1 (1)		- 	OVERSCAN
//	1 0 (2)		- 	UNDERSCAN
#define IGFX_AVIFRAME_SCANINFO_NO_SCAN		0x0000
#define IGFX_AVIFRAME_SCANINFO_OVERSCAN		0x0001 
#define IGFX_AVIFRAME_SCANINFO_UNDERSCAN	0x0002

// Quantization Range: Only 0, 1 and 2 values are valid. 
//	0 0 (0) 	-	Default. Defined by Video format 
//	0 1 (1)		- 	Limited Range
//  1 0 (2)		- 	Full Range
#define IGFX_AVIFRAME_QRANGE_DEFAULT		0x0000
#define IGFX_AVIFRAME_QRANGE_LIMITED		0x0001 
#define IGFX_AVIFRAME_QRANGE_FULL			0x0002

// Picture Aspect Ratio: Only 1 and 2 values are valid. 
//	0 1 (1)		- 	4:3
//  1 0 (2)		- 	16:9 	
#define IGFX_AVIFRAME_PAR_4_3	0x0001
#define IGFX_AVIFRAME_PAR_16_9	0x0002

// Flags to determine if the features are valid or not.
// dwFlags will be set with these values when they are supported.
#define ITCONTENTVALID	0x00800000 
#define SCANINFOVALID	0x00000001
#define DATAINVALID		0x00000000
#define QRANGEVALID		0x00040000
#define BARINFOVALID	0x00000010
#define PARVALID		0x00001000

// GUID
// {BC51275B-1F51-40be-8DC5-BA08C06F70B8}
static const GUID IGFX_GET_SET_AVI_INFOFRAME_GUID = 
{ 0xbc51275b, 0x1f51, 0x40be, { 0x8d, 0xc5, 0xba, 0x8, 0xc0, 0x6f, 0x70, 0xb8 } };

#define BAR_INFO_SIZE	8

// Data Structure
typedef struct _IGFX_AVI_INFOFRAME
{
	UINT  uiTypeCode;					// Reserved
	UINT  uiVersion;					// Reserved
	UINT  uiLength;						// Reserved

	BOOL  bR3R0Valid;			  		// Reserved
	BOOL  bITContent;   		  		// IT Content

	BYTE  barInfo [BAR_INFO_SIZE];		// BAR INFO Fields - RCR: 102228

	DWORD dwDeviceID;					// Device ID

	DWORD dwActiveFormatAspectRatio;	// Reserved	
	DWORD dwNonUniformScaling;	  		// Reserved	
	DWORD dwRGBYCCIndicator;			// Reserved	
	DWORD dwExtColorimetry;				// Reserved	

	DWORD dwPixelFactor;				// Reserved	
	DWORD bBarInfoValid;				// Reserved	
	DWORD dwColorimetry;				// Reserved	
	DWORD dwAspectRatio;				// Picture Aspect Ratio - CUI 3.5	
	DWORD dwQuantRange;					// Quantization Range
	DWORD dwVideoCode;					// Reserved	
	DWORD dwScanInfo;  					// Scan Information

	DWORD dwFlags;  					// Flags

}IGFX_AVI_INFOFRAME, *PIGFX_AVI_INFOFRAME;


//*****************************************************************************
//     CUI 2.75 Structure Definitions Start
//*****************************************************************************

//-----------------------------------------------------------------------------
//           D3D 
//-----------------------------------------------------------------------------

// {6028CC2E-13D5-40dc-8B98-8F13E7EAE053}
static const GUID IGFX_GET_SET_D3D_INFO_GUID = 
{ 0x6028cc2e, 0x13d5, 0x40dc, { 0x8b, 0x98, 0x8f, 0x13, 0xe7, 0xea, 0xe0, 0x53 } };

//Key, value pair definitions
#define IGFX_D3D_BASIC_PERFORMANCE						0
#define IGFX_D3D_BASIC_BALANCE							1 //default value 
#define IGFX_D3D_BASIC_QUALITY							2
#define IGFX_D3D_BASIC_CUSTOM							3


#define IGFX_D3D_CUSTOM_VP_APPLICATION_CHOICE			0 
#define IGFX_D3D_CUSTOM_VP_DRIVER_CHOICE				1 //default value for VP
#define IGFX_D3D_CUSTOM_VP_FORCE_SOFTWARE				2


#define IGFX_D3D_CUSTOM_TEXTURE_QUALITY_PERFORMANCE		0
#define IGFX_D3D_CUSTOM_TEXTURE_QUALITY_BALANCE			1 //default value 
#define IGFX_D3D_CUSTOM_TEXTURE_QUALITY_QUALITY			2


#define IGFX_D3D_CUSTOM_ANISOTROPIC_APPLICATION_CHOICE	0 //default value 
#define IGFX_D3D_CUSTOM_ANISOTROPIC_2X					2
#define IGFX_D3D_CUSTOM_ANISOTROPIC_4X					4
#define IGFX_D3D_CUSTOM_ANISOTROPIC_8X					8
#define IGFX_D3D_CUSTOM_ANISOTROPIC_16X					16


#define IGFX_D3D_CUSTOM_VSYNC_APPLICATION_CHOICE		0 //default value for VSYNC
#define IGFX_D3D_CUSTOM_VSYNC_ALWAYS_ON					1
#define IGFX_D3D_CUSTOM_VSYNC_ALWAYS_OFF				2 

// Flags for D3D

#define IGFX_D3D_TEXTURE_QUALITY_VALID	0x02
#define IGFX_D3D_ANISOTROPIC_VALID		0x04
#define IGFX_D3D_VSYNC_VALID			0x10
#define IGFX_D3D_BASIC_VALID			0x01
#define IGFX_D3D_VP_VALID				0x08

typedef struct _IGFX_D3D_INFO
{
	long lBasicVal;						// Basic value
	long lBasicValDef;					// Default value - basic
				
	long lVertexProcessingVal;			// Vertex Processing
	long lVertexProcessingValDef;		// Default value - vp

	long lTextureQualityVal;			// Texture quality
	long lTextureQualityValDef;			// Default value - tq

	long lAnisotropicVal;				// Anisotropic Filtering
	long lAnisotropicValDef;			// Default value - aniso		

	long lFlipVSyncVal;					// Wait on Flip Vsync
	long lFlipVSyncValDef;				// Default value - async

	DWORD dwFlags;  					// Reserved

} IGFX_D3D_INFO, *PIGFX_D3D_INFO;

//-----------------------------------------------------------------------------
//           XvYCC 
//-----------------------------------------------------------------------------

// {4BB0D5B2-D81C-4c2f-B5F0-33836A036C81}
static const GUID IGFX_GET_SET_XVYCC_INFO_GUID = 
{ 0x4bb0d5b2, 0xd81c, 0x4c2f, { 0xb5, 0xf0, 0x33, 0x83, 0x6a, 0x3, 0x6c, 0x81 } };

typedef struct _IGFX_XVYCC_INFO
{
	BOOL bEnableXvYCC;					// Enabled or Disabled
	BOOL bIsXvYCCSupported;				// Supported(HDMI)/Unsupported(Non HDMI)

	DWORD dwDeviceID;					// Device ID [HDMI Device]
	DWORD dwFlags;  					// Reserved

} IGFX_XVYCC_INFO, *PIGFX_XVYCC_INFO;

#define MAX_VERTICES_DATA 25
#define MAX_FACET_DATA 25
#define GBD_DATA_SIZE 28

static const GUID IGFX_SOURCE_HDMI_GBD_GUID = 
{ 0x78a055be, 0x7667, 0x4bce, { 0xa8, 0x3a, 0xf6, 0xbd, 0x6a, 0x10, 0x53, 0xd0 } };
static const GUID IGFX_SOURCE_LFP_CSC_GUID = 
{ 0xf64a0177, 0x1cbc, 0x46d8, { 0x99, 0x82, 0x85, 0xfd, 0x1d, 0x59, 0xb9, 0xb1 } };

typedef struct _IGFX_MEDIA_SOURCE_HDMI_GBD
{ 
WORD Version; //field (=1.3 only supported) [indicates HDMI 1.3 GBD profile]
			  //High byte = 1, Low byte = 3
DWORD Size;   //(HDMI P0 GBD payload size)
//GBD_P0_HDMI_1_3 HdmiGBD; //Data/Payload (send directly to HDMI 1.3 sink)
BYTE GBDPayLoad[GBD_DATA_SIZE];
}IGFX_MEDIA_SOURCE_HDMI_GBD, *PIGFX_MEDIA_SOURCE_HDMI_GBD;

typedef struct _IGFX_SOURCE_HDMI_GBD_DATA
{
/*IN*/ DWORD dwSourceID;
//In case of extended desktop id=0 means primary, 1-secondary and so on
//In case of clone, there is only one id=0
/*OUT*/ BOOL IsXVYCCSupported;
//Returns TRUE if there is an HDMI display associated with ulSourceID which 
//can support xvYCC
/*OUT*/ BOOL IsXVYCCEnabled;
//Returns CUI+display status of xvYCC
//If 2 HDMI displays are in clone, return TRUE if one of the display has xvYCC enabled & is capable
/*IN*/ DWORD dwFlags;
//To indicate type
/*IN*/ IGFX_MEDIA_SOURCE_HDMI_GBD MediaSourceHDMIGBD;
//Applied to the HDMI display associated with source ID
//In case of clone with single source ID, driver applies this to xvYCC enabled displays internally
}IGFX_SOURCE_HDMI_GBD_DATA, *PIGFX_SOURCE_HDMI_GBD_DATA;

typedef struct _IGFX_DISPLAY_CSC_MATRIX
{
float fLFPCSCMatrix_601[3][3]; // for 601 source
float fLFPCSCMatrix_709[3][3]; // for 709 source

/*IN*/ UCHAR flag; //to indicate desktop or video (both not possible)
//LFP_CSC_VIDEO (bit0)
//Use matrix based on source of 601/709 along with YUV?RGB conversion
//LFP_CSC_GRAPHICS (bit1)
}IGFX_DISPLAY_CSC_MATRIX, *PIGFX_DISPLAY_CSC_MATRIX;

typedef struct _IGFX_SOURCE_DISPLAY_CSC_DATA
{
/*IN*/ BOOL bEnable;
//If matrix is to be used set bEnable to TRUE
/*IN*/ ULONG ulReserved;

/*IN*/ IGFX_DISPLAY_CSC_MATRIX CSCmatrix;
//Must have for LFP
/*OUT*/ DWORD dwIsSupported;
//Indicates if this feature is supported
/*IN*/ DWORD dwFlag;
//To indicate CSC for desktop or video
}IGFX_SOURCE_DISPLAY_CSC_DATA, *PIGFX_SOURCE_DISPLAY_CSC_DATA;

////////////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//           POWER - IPS and Power Plan Settings
//-----------------------------------------------------------------------------

// {9EE756C6-4D97-4b5d-BB93-B854E727A674}
static const GUID IGFX_GET_SET_POWER_INFO_GUID = 
{ 0x9ee756c6, 0x4d97, 0x4b5d, { 0xbb, 0x93, 0xb8, 0x54, 0xe7, 0x27, 0xa6, 0x74 } };

#define IGFX_POWER_FBC					0x01
#define IGFX_POWER_GSV					0x02
#define IGFX_POWER_CXSR					0x04
#define IGFX_POWER_DPST					0x08
#define IGFX_POWER_ALS					0x10
#define IGFX_POWER_SRD					0x20 
#define IGFX_POWER_DPS					0x40
#define IGFX_POWER_GRS					0x80
#define IGFX_POWER_FEATURE_SETTINGS		0x01 // Feature Setting operation
#define IGFX_POWER_PLAN_SETTINGS		0x03 // Power Plan Setting operation
#define IGFX_POWER_IPS_SETTINGS			0x04 //Turbo Settings

typedef enum IGFX_POWER_PLAN_ENUM
{
	IGFX_PWR_PLAN_GET_CURRENT,
    IGFX_PWR_PLAN_BEST_POWER_SAVINGS,    
    IGFX_PWR_PLAN_BETTER_POWER_SAVINGS,  
    IGFX_PWR_PLAN_GOOD_POWER_SAVINGS,    
    IGFX_PWR_PLAN_DISABLE_POWER_SAVINGS, 
    IGFX_PWR_PLAN_CUSTOM,                
	IGFX_NUM_OF_PWR_USER_PLANS

} IGFX_POWER_PLAN;

typedef enum IGFX_IPS_DEVICE_ENUM
{
	IGFX_PWRCONS_IPS_DEVICE_UNKNOWN,
	IGFX_PWRCONS_IPS_DEVICE_CPU,
	IGFX_PWRCONS_IPS_DEVICE_GFX

}IGFX_IPS_DEVICE;

typedef struct _IGFX_POWER_CONSERVATION_DATA
{
	// Power State
    DWORD			dwPowerState;       

	// To mention operation: Plan, Feature or backlight
	DWORD			dwOperation;		

	//The predefined power plan
	IGFX_POWER_PLAN	PowerPlan;		    

	//Display Device 
    DWORD           dwDisplayDevice;        

	//For Get, Capability -- features we are supporting in driver
	DWORD       	dwCapability;       

	//For Set, Features CUI wants to change, values in 	//dwEnabledFeature
	DWORD       	dwChangedFeatures;  

	//Current Status -- enabled features 
    DWORD           dwEnabledFeatures;  

	//DPST Level, Current Level and Supported MAX Level 
	DWORD           dwDPSTCurLevel;      
    DWORD           dwDPSTTotalLevel; 
    
	//Graphics P State Level, Current Level and Supported MAX Level 
	DWORD           dwGSVCurLevel;      
    DWORD           dwGSVTotalLevel;

	//Whether MFD is supported or not
    BOOL			bIsMFD;			

	// Static DRRS RCR enabled for Vista 1022287
	BOOL			IsSupportForStaticDRRS; 

	//Refresh rate array
	DWORD			dwRR[20];  

	//Current low RR
	DWORD			dwLowRR;	

	//Number of refresh rates
	DWORD			dwNumRR;

	// Reserved
	IGFX_IPS_DEVICE	IPSDevice; 			

	//Whether Intelligent Power Sharing is supported or not
	BOOL			bIsIPSSupported;		

	//TRUE if Intelligent Power Sharing is enabled
	BOOL			bEnableIPS;		

	// Reserved
	DWORD 			dwIPSRenderFrequency;	

	// Reserved
 	DWORD 			dwFlags;  			

}IGFX_POWER_CONSERVATION_DATA, *PIGFX_POWER_CONSERVATION_DATA;
 
   //-----------------------------------------------------------------------
// RCR 1022266: I2C/DDC BUS ACCESS SDK Interfaces
//-----------------------------------------------------------------------
// {AD295110-93E4-4776-8710-93A72C92DA5A}
static const GUID IGFX_GET_SET_BUS_INFO_GUID = 
{ 0xad295110, 0x93e4, 0x4776, { 0x87, 0x10, 0x93, 0xa7, 0x2c, 0x92, 0xda, 0x5a } };

#define IGFX_BUS_I2C 0x0001
//default bus is DDC
#define IGFX_BUS_DDC 0x0002
#define IGFX_BUS_GET 0x0001
#define IGFX_BUS_SET 0x0002
#define IGFX_MAX_DATA_SIZE 0x0080
#define IGFX_MAX_BUS_BUFSIZE 0x0080
#define IGFX_BUS_NO_INDEXING	0x0001
#define IGFX_BUS_USE_INDEXING	0x0000

static DWORD dwBlockDevice[] =
{ 0xA0, 0xA2, 0x50, 0x51, 0x6E, 0x6F, 0x70, 0x72, 0x74, 0x80, 0x81, 0xFF};

static DWORD dwBlockAddress[] =
{ 0x70, 0x72, 0x74, 0xFF};

typedef struct _IGFX_BUS_INFO
{
	DWORD	dwDeviceUID;
	DWORD	dwOpType;
	DWORD	dwSize;
	DWORD	dwAddress;
	DWORD	dwSubAddress;
	DWORD	dwFlags;
	BYTE	byBusType;
	BYTE	Data[IGFX_MAX_BUS_BUFSIZE];
} IGFX_BUS_INFO, *PIGFX_BUS_INFO;


//-----------------------------------------------------------------------------
//           HUE & SAT 
//-----------------------------------------------------------------------------

// {1CC2BB24-5D6F-4557-8BFB-830C4F4DA701}
static const GUID IGFX_GET_SET_HUESAT_INFO_GUID = 
{ 0x1cc2bb24, 0x5d6f, 0x4557, { 0x8b, 0xfb, 0x83, 0xc, 0x4f, 0x4d, 0xa7, 0x1 } };

typedef struct _IGFX_COLOR_DATA
{
    float fValue;						// Current Value
    float fDefault;						// Default Value 
    float fMin;							// Minimium Value 
    float fMax;							// Maximium Value 
    float fStep;						// Adjustment value for each step
}IGFX_COLOR_DATA, *PIGFX_COLOR_DATA;

#define IGFX_HUESAT_COEFF	0x01

typedef struct _IGFX_HLS_COEFF_INFO
{
	FLOAT CoEff[9];						// Reserved

	DWORD dwFlags;						// Reserved
}IGFX_HLS_COEFF_INFO, *PIGFX_HLS_COEFF_INFO;

typedef struct _IGFX_HUESAT_INFO
{
	BOOL bIsFeatureSupported; 			// Supported? TRUE: FALSE
	BOOL bIsRGB;						// RGB (TRUE) or YUV (FALSE)

	DWORD dwDeviceID;					// Device ID

	IGFX_COLOR_DATA HueSettings; 		// Hue Settings
	IGFX_COLOR_DATA SaturationSettings; // Saturation Settings

	DWORD dwFlags;  					// Reserved

} IGFX_HUESAT_INFO, *PIGFX_HUESAT_INFO;

//-----------------------------------------------------------------------------
//           VIDEO QUALITY EXTENDED SETTINGS 
//-----------------------------------------------------------------------------

// {92884CDE-934B-4516-BB18-4ED8A939D595}
static const GUID IGFX_GET_SET_VIDEO_QUALITY_EX_GUID = 
{ 0x92884cde, 0x934b, 0x4516, { 0xbb, 0x18, 0x4e, 0xd8, 0xa9, 0x39, 0xd5, 0x95 } };

typedef struct _IGFX_VIDEO_QUALITY_DATA_EX
{
	IGFX_VIDEO_QUALITY_DATA		videoQualityData;

	// Denoise Autodetect
	IGFX_MEDIA_ENABLE_DATA		EnableDriverPreference;	

	// Optimal Sharpness
	IGFX_MEDIA_ENABLE_DATA		EnableOptimalSharpness;	

	// DeNoise settings
	IGFX_MEDIA_SETTINGS_DATA	NoiseReductionSettings;

	//Reserved
	DWORD						dwFlags;									

}IGFX_VIDEO_QUALITY_DATA_EX, *PIGFX_VIDEO_QUALITY_DATA_EX;

//-----------------------------------------------------------------------------
//           MBM 
//-----------------------------------------------------------------------------

// {A6E5A3EB-204B-4b30-9144-603E42795477}
static const GUID IGFX_GET_SET_MBM_INFO_GUID = 
{ 0xa6e5a3eb, 0x204b, 0x4b30, { 0x91, 0x44, 0x60, 0x3e, 0x42, 0x79, 0x54, 0x77 } };

typedef struct _IGFX_MBM_INFO
{
	BOOL bIsFeatureSupported;			//Is the feature supported?
	BOOL bEnableMBM;					//Is MBM Enabled?

	DWORD dwDisplayUID;					//Reserved
	DWORD dwFlags;						//Reserved

}IGFX_MBM_INFO, *PIGFX_MBM_INFO;
//*****************************************************************************
//         CUI 2.75 Structure Definitions End
//*****************************************************************************


#define IGFX_MAX_AUX_BUFSIZE 16
// Native AUX
#define IGFX_AUX_WRITE					8
#define IGFX_AUX_READ					9
 // I2C on AUX
#define IGFX_I2C_AUX_WRITE				0
#define IGFX_I2C_AUX_READ				1
#define IGFX_I2C_AUX_WRITE_STATUS_REQ	2
// I2C with MOT set
#define IGFX_I2C_AUX_WRITE_MOT			4
#define IGFX_I2C_AUX_READ_MOT			5
#define IGFX_I2C_AUX_WRITE_STATUS_REQ_MOT 6


// {BFB9816C-AEB0-434b-99F3-0F94E6BEBF0D}
static const GUID IGFX_GET_SET_AUX_INFO_GUID = 
{0xbfb9816c, 0xaeb0, 0x434b, { 0x99, 0xf3, 0xf, 0x94, 0xe6, 0xbe, 0xbf, 0xd} };

typedef struct _IGFX_AUX_INFO
{
	/*IN*/ DWORD	dwDeviceUID;
	/*IN*/  DWORD	dwOpType;
	/*IN*/  DWORD	dwSize;
	/*IN*/  DWORD	dwAddress;	
	/*INOUT*/	BYTE Data[IGFX_MAX_AUX_BUFSIZE];
}IGFX_AUX_INFO, *PIGFX_AUX_INFO;

// Disclaimer
// Intel will no way responsible for loss/damage if client applications
// change this values without prior written permission from CUI team
// Client applications are not supposed to make any changes to the below mentioned macros 
// and structure definitions
// Improper changes to these can result in malfunctiontioning
// of the program viz. wrong results, application crash or even system crash

#define VERSIONS_SUPPORTED                      1
#define IS_MEDIA_VERSION_SUPPORTED(x)     (x <= VERSIONS_SUPPORTED)

typedef struct _IGFX_VIDEO_HEADER
{
	DWORD dwVersion;
	DWORD dwReserved;
}IGFX_VIDEO_HEADER;

// {C8A17673-DD2D-4e98-9617-7764AF5009F7}
static const GUID IGFX_GET_SET_VIDEO_QUALITY_EX2_GUID = 
{ 0xc8a17673, 0xdd2d, 0x4e98, { 0x96, 0x17, 0x77, 0x64, 0xaf, 0x50, 0x9, 0xf7 } };

typedef struct _IGFX_VIDEO_QUALITY_DATA_EX2
{
	IGFX_VIDEO_HEADER	header;
#if IS_MEDIA_VERSION_SUPPORTED(1)
	// IGFX_VIDEO_QUALITY_DATA
	DWORD	dwSupportedFeatures;
	BOOL	bCurrentFMD;					// Current Value
	BOOL	bDefaultFMD;					// Default Value 
	BOOL	bCurrentNR;						// Current Value
	BOOL	bDefaultNR;						// Default Value 
	BOOL	bCurrentSharpness;				// Current Value
	BOOL	bDefaultSharpness;				// Default Value 
	DWORD	dwFlags;						// Reserved
	float	fSharpnessCurrent;				// Current Value
	float	fSharpnessDefault;				// Default Value 
	float	fSharpnessMin;					// Minimium Value 
	float	fSharpnessMax;					// Maximium Value 
	float	fSharpnessStep;					// Adjustment value for each step
	// Denoise Autodetect
	BOOL	bCurrentDriverPreference;		// Current Value
	BOOL	bDefaultDriverPreference;		// Default Value 
	// Optimal Sharpness
	BOOL	bCurrentOptimalSharpness;		// Current Value
	BOOL	bDefaultOptimalSharpness;		// Default Value 
	// Noise Reduction
	float	fNoiseReductionCurrent;				// Current Value
	float	fNoiseReductionDefault;				// Default Value 
	float	fNoiseReductionMin;					// Minimium Value 
	float	fNoiseReductionMax;					// Maximium Value 
	float	fNoiseReductionStep;					// Adjustment value for each step
	DWORD	dwFlags2;						// Reserved
	BOOL bEnableDenoiseAutoDetect;			//enable - 1, disable - 0
	BOOL bEnableDenoiseAutoDetectDef;	
	
	BOOL bSkinToneEnhancement;
	BOOL bSkinToneEnhancementDef;

	BOOL bAutoContrastEnhancement;
	BOOL bAutoContrastEnhancementDef;
#endif
#if IS_MEDIA_VERSION_SUPPORTED(2)
	// add new code here for future releases
#endif
}IGFX_VIDEO_QUALITY_DATA_EX2, *PIGFX_VIDEO_QUALITY_DATA_EX2;


// {63F9A187-544A-4db3-B04E-A95CE2809D08}
static const GUID IGFX_GET_SET_MEDIA_SCALING_EX2_GUID = 
{ 0x63f9a187, 0x544a, 0x4db3, { 0xb0, 0x4e, 0xa9, 0x5c, 0xe2, 0x80, 0x9d, 0x8 } };

typedef struct _IGFX_MEDIA_SCALING_DATA_EX2
{
	IGFX_VIDEO_HEADER	header;
#if IS_MEDIA_VERSION_SUPPORTED(1)
	DWORD	dwSupportedFeatures;
	BOOL	bEnableNLAS;		// Current Value
    BOOL	bDefaultNLAS;	// Default Value 
	DWORD	dwFlags;		//Reserved
	float	fVerticalCropCurrent;    // Current Value
    float	fVerticalCropDefault;  // Default Value 
    float	fVerticalCropMin;      // Minimium Value 
    float	fVerticalCropMax;      // Maximium Value 
    float	fVerticalCropStep;     // Adjustment value for each step

	float	fHLinearRegionCurrent;    // Current Value
    float	fHLinearRegionDefault;  // Default Value 
    float	fHLinearRegionMin;      // Minimium Value 
    float	fHLinearRegionMax;      // Maximium Value 
    float	fHLinearRegionStep;     // Adjustment value for each step

	float	fNonLinearCropCurrent;    // Current Value
    float	fNonLinearCropDefault;  // Default Value 
    float	fNonLinearCropMin;      // Minimium Value 
    float	fNonLinearCropMax;      // Maximium Value 
    float	fNonLinearCropStep;     // Adjustment value for each step
#endif
#if IS_MEDIA_VERSION_SUPPORTED(2)
	// add new code here for future releases
#endif
}IGFX_MEDIA_SCALING_DATA_EX2, *PIGFX_MEDIA_SCALING_DATA_EX2;

// {CCE852BF-E425-4728-A6FC-ACB2091FB236}
static const GUID IGFX_GET_SET_MEDIA_COLOR_EX2_GUID = 
{ 0xcce852bf, 0xe425, 0x4728, { 0xa6, 0xfc, 0xac, 0xb2, 0x9, 0x1f, 0xb2, 0x36 } };

typedef struct _IGFX_MEDIA_COLOR_DATA_EX2
{
	IGFX_VIDEO_HEADER	header;
#if IS_MEDIA_VERSION_SUPPORTED(1)
	DWORD	dwSupportedFeatures;
	BOOL	bEnableColor;		// Current Value
    BOOL	bDefaultColor;	// Default Value 
	DWORD	dwFlags;		//Reserved

	float	fHueCurrent;    // Current Value
    float	fHueDefault;  // Default Value 
    float	fHueMin;      // Minimium Value 
    float	fHueMax;      // Maximium Value 
    float	fHueStep;     // Adjustment value for each step

	float	fSaturationCurrent;    // Current Value
    float	fSaturationDefault;  // Default Value 
    float	fSaturationMin;      // Minimium Value 
    float	fSaturationMax;      // Maximium Value 
    float	fSaturationStep;     // Adjustment value for each step

	float	fContrastCurrent;    // Current Value
    float	fContrastDefault;  // Default Value 
    float	fContrastMin;      // Minimium Value 
    float	fContrastMax;      // Maximium Value 
    float	fContrastStep;     // Adjustment value for each step

	float	fBrightnessCurrent;    // Current Value
    float	fBrightnessDefault;  // Default Value 
    float	fBrightnessMin;      // Minimium Value 
    float	fBrightnessMax;      // Maximium Value 
    float	fBrightnessStep;     // Adjustment value for each step

	BOOL bTotalColorControl; //enable - 1, disable - 0
	BOOL bTotalColorControlDef;
	float	fRedCurrent;    // Current Value
    float	fRedDefault;  // Default Value 
    float	fRedMin;      // Minimium Value 
    float	fRedMax;      // Maximium Value 
    float	fRedStep;     // Adjustment value for each step

	float	fGreenCurrent;    // Current Value
    float	fGreenDefault;  // Default Value 
    float	fGreenMin;      // Minimium Value 
    float	fGreenMax;      // Maximium Value 
    float	fGreenStep;     // Adjustment value for each step

	float	fBlueCurrent;    // Current Value
    float	fBlueDefault;  // Default Value 
    float	fBlueMin;      // Minimium Value 
    float	fBlueMax;      // Maximium Value 
    float	fBlueStep;     // Adjustment value for each step

	float	fYellowCurrent;    // Current Value
    float	fYellowDefault;  // Default Value 
    float	fYellowMin;      // Minimium Value 
    float	fYellowMax;      // Maximium Value 
    float	fYellowStep;     // Adjustment value for each step

	float	fCyanCurrent;    // Current Value
    float	fCyanDefault;  // Default Value 
    float	fCyanMin;      // Minimium Value 
    float	fCyanMax;      // Maximium Value 
    float	fCyanStep;     // Adjustment value for each step
	
	float	fMagentaCurrent;    // Current Value
    float	fMagentaDefault;  // Default Value 
    float	fMagentaMin;      // Minimium Value 
    float	fMagentaMax;      // Maximium Value 
    float	fMagentaStep;     // Adjustment value for each step
#endif
#if IS_MEDIA_VERSION_SUPPORTED(2)
	// add new code here for future releases
#endif
}IGFX_MEDIA_COLOR_DATA_EX2, *PIGFX_MEDIA_COLOR_DATA_EX2;

//Supported Device Types
#define IGFX_DEVICETYPE_UNKNOWN			        0x00000000
#define IGFX_DEVICETYPE_CRT                  	0x00000001 	//For CRT 
#define IGFX_DEVICETYPE_LFP                 	0x00000002	// LFP
#define IGFX_DEVICETYPE_DVI                  	0x00000004	// DFP
#define IGFX_DEVICETYPE_HDMI            		0x00000008	// HDMI
#define IGFX_DEVICETYPE_TV	           			0x00000010	//TV
#define IGFX_DEVICETYPE_DISPLAY_PORT  			0x00000020	//DISPLAY PORT

// {46C34D74-4042-4241-9595-B134F01324DF}
static const GUID IGFX_GET_DEVICETYPE_GUID = 
{0x46c34d74, 0x4042, 0x4241, 0x95, 0x95, 0xb1, 0x34, 0xf0, 0x13, 0x24, 0xdf};

typedef struct _IGFX_DEVICETYPE_INFO
{
	/*IN*/  	DWORD	dwDeviceUID;
	/*OUT*/ 	DWORD	dwDeviceType;
				DWORD	dwReserved; //reserved for future use
}IGFX_DEVICETYPE_INFO, *PIGFX_DEVICETYPE_INFO;

//S3D defenitions.
// {BCA5DA13-973B-4986-99F7-CD8E9E12737D}
static const GUID IGFX_S3D_CAPS_GUID = 
{ 0xbca5da13, 0x973b, 0x4986, { 0x99, 0xf7, 0xcd, 0x8e, 0x9e, 0x12, 0x73, 0x7d } };

#define MAX_S3D_MODES	20
#define IGFX_S3D_FORMAT_MASK(eFormat)	((IGFX_S3D_FORMAT)(1<<eFormat))
typedef enum _IGFX_S3D_FORMAT
{
	IGFX_eS3DFramePacking = 0,
	IGFX_eS3DFieldAlternative,
	IGFX_eS3DLineAlternative,
	IGFX_eS3DSideBySideFull,
	IGFX_eS3DLDepth,
	IGFX_eS3DLDepthGraphicsGraphicsDeptch,
	IGFX_eS3DTopBottom,
	IGFX_eS3DSideBySideHalfHorizSubSampling,
	IGFX_eS3DSideBySideHalfQuincunxSubSampling,

	IGFX_eNonS3D = 63 // should be the last entry
} IGFX_S3D_FORMAT;

typedef struct _IGFX_S3D_MODE_CAPS_STRUCT
{
	// Note: All modes with 32BPP color only
	ULONG ulResWidth; // 0xFFFF if s3d possible with all modes
	ULONG ulResHeight; // 0xFFFF if s3d possible with all modes
	ULONG ulRefreshRate; // 0xFFFF if s3d possible with all modes
	ULONG ulReserved; // Don’t care for now

	DWORD dwMonitorS3DFormats; // mask of IGFX_S3D_FORMAT
	DWORD dwGFXS3DFormats; // mask of IGFX_S3D_FORMAT
} IGFX_S3D_MODE_CAPS_STRUCT, *PIGFX_S3D_MODE_CAPS_STRUCT;

// S3D capability info
typedef struct _IGFX_S3D_CAPS_STRUCT
{
	// Set this to a specific display UID if display specific information is required. 
	//Just to know whether GFX supports S3D or not set this to 0
	DWORD dwDisplayUID;

	// Indicates support for s3d in the platform with any of the ports
	// Caller shall use the display specific interface to get 
	// format/mode/display specific s3d support details by display
	// and GFX driver
	BOOL bSupportsS3DLRFrames; 

	// Display specific S3D details
	// Valid iff dwDisplayUID is a valid display identifier
	ULONG ulNumEntries; // number of entries in S3DCapsPerMode[]
	IGFX_S3D_MODE_CAPS_STRUCT S3DCapsPerMode[MAX_S3D_MODES];

	// Current S3D format for dwDisplayUID with current OS mode
	// Valid iff dwDisplayUID is a valid display identifier
	IGFX_S3D_FORMAT eCurrentS3DFormat;
} IGFX_S3D_CAPS_STRUCT, *PIGFX_S3D_CAPS_STRUCT;

// {945849BD-99D7-4be8-ACE1-C49A9A7A0EB1}
static const GUID IGFX_S3D_CONTROL_GUID = 
{ 0x945849bd, 0x99d7, 0x4be8, { 0xac, 0xe1, 0xc4, 0x9a, 0x9a, 0x7a, 0xe, 0xb1 } };

typedef struct _IGFX_S3D_CONTROL_STRUCT
{
	// Should be a valid display UID
	DWORD dwDisplayUID;
	// process ID of requesting application, caller should set it
	DWORD dwProcessID; 
	// Enable/disable flag
	BOOL bEnable;
	// Mode for s3d: Set to 0 for current mode
	// Note: All modes with 32BPP color only
	ULONG ulResWidth;
	ULONG ulResHeight;
	ULONG ulRefreshRate;
	ULONG ulReserved; // Should be 0
	// s3d format to set to
	IGFX_S3D_FORMAT eS3DFormat;
} IGFX_S3D_CONTROL_STRUCT, *PIGFX_S3D_CONTROL_STRUCT;

#endif //CUISDK_COMMON_H