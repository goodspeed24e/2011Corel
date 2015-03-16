#ifndef _ATI_SORT_DEFINITION_H_
#define _ATI_SORT_DEFINITION_H_

#define SORT_VERSION_MAJOR         0
#define SORT_VERSION_MINOR         4
#define SORT_VERSION ( ( SORT_VERSION_MAJOR << 16 ) | SORT_VERSION_MINOR )

// flags
#define SORT_FLAG_USERTFLIP             1	// for flip
#define SORT_FLAG_CHECK_PREFLIP_STATUS  2 // for flip

// commands
#define SORT_CMD_UPDATEOVERLAY			0x00000001
#define SORT_CMD_FLIPOVERLAY            0x00000002	// Flags = 0 - use old flip, 1 - use new run time flip
#define SORT_CMD_SHOWOVERLAY            0x00000003
#define SORT_CMD_HIDEOVERLAY            0x00000004
#define SORT_CMD_PREFLIPOVERLAY         0x00000005	// blits SORT to flip Q or do this via colorFill ??? 
// in the case Lock/Unlock ISVs don't have to flush before the flip call
#define SORT_CMD_RESET_QUEUE            0x00000020

// good status
#define SORT_STATUS_OK                  0x00000000
#define SORT_STATUS_QUEUE_IS_EMPTY      0x00000001	// flip (UMD queue under Vista)
#define SORT_STATUS_QUEUE_IS_FULL       0x00000002	// preflip
#define SORT_STATUS_PREFLIP_NOT_READY   0x00000003	// flip failed

// extended status for some commands (for for API versions 0.4 and up)
#define SORT_STATUSEX_UNDEFINED         0x00000000	// undefined
// extended status for SORT_STATUS_PREFLIP_NOT_READY
#define SORT_STATUSEX_PREFLIP_NOT_READY 0x00000001	// flip failed because preflip wasn't finished by HW
#define SORT_STATUSEX_FLIP_PENDING      0x00000002	// flip failed because previous flip wasn't complete yet

// bad status
#define SORT_STATUS_FAIL                0x80000000	// generic failure
#define SORT_STATUS_INVLCMD             0x80000001	// invalid command
#define SORT_STATUS_INVLPARAMS          0x80000002	// bad parameters for the given command
#define SORT_STATUS_OUTOFMEM            0x80000003	// mem allocation failed if command needed mem allocation

// Caps
#define SORT_CAPS_RTFLIP                0x00000001

// ATI private
#define SORT_CMD_NOP                    0x00000100	// this will cancel all of the above


typedef struct _SORT_CAPS 
{
	DWORD dwRevision;
	DWORD dwCaps;
	DWORD dwRecomHDHeight;
	DWORD dwRecomFps;
} SORT_CAPS;

// we can have GET_CPAS command or on Lock we return our caps all the time
typedef struct _SORT_CMD_BUF {
	// in, out
	DWORD dwSize;
	// in
	DWORD dwFlags; // custom for each command
	DWORD dwCommand;
	RECT  rcSrc;    // has to be filled for 1 - 5 commands
	RECT  rcDst;
	union 
	{
		DWORD dwDstColorKey;
		DWORD dwFunctionID; // for Colorfill
	};

	// out
	SORT_CAPS sCaps;
	DWORD dwStatus;     // return error code
	// fields below added for API versions 0.4 and up 
	DWORD dwStatusEx;   // extended status for some specific operations
	DWORD dwNumOfQueuedUpPreflips;
	DWORD dwReserved[18];
} SORT_CMD_BUF;

#define Control_Config_CWDDE32          0x07306
#define CWDDEVA_OK                      0x00A00001
#define CWDDEVA_RTFLIP                  0x00A00007

typedef struct tagCWDDECMD
{
	// CWDDECMD structure.
	ULONG ulSize;// size of CWDDECMD
	ULONG ulEscape32;// 32 bit escape code
	ULONG ulIndex;// index
	ULONG ulDriverReserved;
} CWDDECMD;


// A minor revision change indicates a backward-compatible change; a major revision change indicates a backward-incompatible
#define EAPI_VERSION_MAJOR         1
#define EAPI_VERSION_MINOR         2
#define EAPI_VERSION ((EAPI_VERSION_MAJOR << 16) | EAPI_VERSION_MINOR)

#define EAPI_CMD_GETCAPS                0x00000001
#define EAPI_CMD_COLFILTAGSURF          0x00000002 // app has to specify specify dwColorFillID and dwFlag
#define EAPI_CMD_VIRTUALIZESORT         0x00000003 // in this mode app allocates render target of desktop size,
// and it must render video playback window into window position
// - not top-left corner, SORT preflip does NOT do the blit
#define EAPI_CMD_ENABLEFULLSCREN        0x00000004

#define EAPI_CAPS_TAGASPROTECTED        0x00000001
// added in 1.2
#define EAPI_CAPS_SORTVIRTUALIZATION        0x00000002
#define EAPI_CAPS_FULLSCREENOPTIMIZATION    0x00000004

#define EAPI_STATUS_OK                      0x00000000
#define EAPI_STATUS_FAIL                    0x80000000     // generic failure
#define EAPI_STATUS_INVLCMD                 0x80000001     // invalid command
#define EAPI_STATUS_INVLPARAMS              0x80000002     // bad parameters for the given command
#define EAPI_STATUS_OUTOFMEM                0x80000003     // mem allocation failed if command needed mem allocation

#pragma pack(push, BeforeEAPIpacking, 1)

// common header for all commands
typedef struct _EAPI_IN_HEADER
{
	DWORD dwCommandSize;    // = sizeof(specific command structure), for example: dwCommandSize = szieof(EAPI_GETCAPS_IN_BUF)
	DWORD dwCommand;
} EAPI_IN_HEADER;

// common header for output status structures
typedef struct _EAPI_OUT_HEADER
{
	DWORD dwOutSize;        // = sizeof(specific output structure), for example: dwOutSize = szieof(EAPI_GETCAPS_OUT_BUF)
	DWORD dwStatus;
} EAPI_OUT_HEADER;

// in for EAPI_CMD_GETCAPS command
typedef struct _EAPI_GETCAPS_IN_BUF
{
	EAPI_IN_HEADER inHeader;
	DWORD dwValidFields;
	DWORD dwReserved[7];
} EAPI_GETCAPS_IN_BUF, *PEAPI_GETCAPS_IN_BUF;

// out structure for EAPI_GETCAPS_CMD_BUF
#define EAPI_GETCAPS_OUT_FIELDS_REV		    0x00000001       // set if dwRevision field is valid
#define EAPI_GETCAPS_OUT_FIELDS_CAP         0x00000002       // set if dwCaps field is valid

typedef struct _EAPI_GETCAPS_OUT_BUF
{
	EAPI_OUT_HEADER outHeader;
	DWORD dwValidFields;
	DWORD dwRevision;
	DWORD dwCaps;
	// from 1.2
	DWORD dwRecomFps;
	DWORD dwRecomVideoPixelArea;			// w = SQRT(aspectRatio * dwRecomVideoPixelArea), h = dwRecomVideoPixelArea/w
	DWORD dwReserved[6];
} EAPI_GETCAPS_OUT_BUF, *PEAPI_GETCAPS_OUT_BUF;

//---------------
// tag a surface as protected
#define EAPI_FLAG_TAGASPROTECTED            0x00000001		// a flag for EAPI_CMD_COLFILTAGSURF command

// in for EAPI_CMD_COLFILTAGSURF
#define EAPI_COLFILTAGSURF_IN_FIELDS_CFD    0x00000001       // set if dwColorFillID field is valid
#define EAPI_COLFILTAGSURF_IN_FIELDS_FLG    0x00000002       // set if dwFlags field is valid
// added from 1.1
#define EAPI_COLFILTAGSURF_IN_FIELDS_HWND   0x00000004       // set if hWindow field is valid
typedef struct _EAPI_COLFILTAGSURF_IN_BUF 
{
	EAPI_IN_HEADER inHeader;
	DWORD dwValidFields;
	DWORD dwColorFillID;
	DWORD dwFlags; // custom for each command
	// added from 1.1
	unsigned __int64 hWindow;
	DWORD dwReserved[9];
} EAPI_COLFILTAGSURF_CMD_BUF, *PEAPI_COLFILTAGSURF_CMD_BUF;

// output structure for EAPI_CMD_COLFILTAGSURF
typedef struct _EAPI_COLFILTAGSURF_OUT_BUF 
{
	EAPI_OUT_HEADER outHeader;
	DWORD dwValidFields;
	DWORD dwReserved[11];
} EAPI_COLFILTAGSURF_OUT_BUF;

//---------------

// added from 1.2
// Enable/disable SORT virtualization
#define EAPI_FLAG_VIRTUALIZESORT_ENABLE         0x00000001		// 1 - enable, 0 - disable

// in for EAPI_CMD_VIRTUALIZESORT
#define EAPI_CMD_VIRTUALIZESORT_IN_FIELDS_FLG   0x00000001		// set if dwFlags field is valid
typedef struct _EAPI_VIRTUALIZESORT_IN_BUF 
{
	EAPI_IN_HEADER inHeader;
	DWORD dwValidFields;
	DWORD dwFlags; 
	DWORD dwReserved[9];
} EAPI_VIRTUALIZESORT_CMD_BUF, *PEAPI_VIRTUALIZESORT_CMD_BUF;

typedef struct _EAPI_VIRTUALIZESORT_OUT_BUF 
{
	EAPI_OUT_HEADER outHeader;
	DWORD dwValidFields;
	DWORD dwReserved[5];
} EAPI_VIRTUALIZESORT_OUT_BUF;

//---------------

#define EAPI_FLAG_FULLSCREEN_ENABLE					0x00000001		// 1 - enable, 0 - disable

// in for EAPI_CMD_FULLSCREEN
#define EAPI_CMD_FULLSCREEN_IN_FIELDS_FLG			0x00000001		// set if dwFlags field is valid
typedef struct _EAPI_FULLSCREEN_IN_BUF 
{
	EAPI_IN_HEADER inHeader;
	DWORD dwValidFields;
	DWORD dwFlags; 
	DWORD dwReserved[9];
} EAPI_FULLSCREEN_CMD_BUF, *PEAPI_FULLSCREEN_CMD_BUF;

typedef struct _EAPI_FULLSCREEN_OUT_BUF 
{
	EAPI_OUT_HEADER outHeader;
	DWORD dwValidFields;
	DWORD dwReserved[5];
} EAPI_FULLSCREEN_OUT_BUF;

//---------------

#pragma pack(pop, BeforeEAPIpacking)

#endif
