//=============================================================================
//	THIS SOURCE CODE IS PROPRIETARY INFORMATION BELONGING TO INTERVIDEO, INC.
// 	ANY USE INCLUDING BUT NOT LIMITED TO COPYING OF CODE, CONCEPTS, AND/OR
//	ALGORITHMS IS PROHIBITED EXCEPT WITH EXPRESS WRITTEN PERMISSION BY THE 
//	COMPANY.
//
// 	THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
// 	KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// 	IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
// 	PURPOSE.
//
// 	Copyright (c) 2007 InterVideo Corporation.  All Rights Reserved.
//
//-----------------------------------------------------------------------------

#ifndef _IMP2VDEC_H_
#define _IMP2VDEC_H_

#include <windows.h>

#include "iviDXVAdef.h"
#include <d3d9.h>
#include <dxva.h>

class DXVA_ENCRYPTBase;
class Cline21;

#ifndef E_PROP_ID_UNSUPPORTED
#define E_PROP_ID_UNSUPPORTED     ((HRESULT)0x80070490L)
#endif

#ifndef _MP2V_TS_DEFINED_
#define _MP2V_TS_DEFINED_

struct MP2V_TS
{
	unsigned __int64	ts;
	unsigned int		freq;
	unsigned char		tslength;
	unsigned char		flags;
	unsigned char		unused1;
	unsigned char		unused2;
};

extern const MP2V_TS MP2V_TS_INVALID;

enum MP2V_FREQ
{
	MP2V_FREQ_1KHZ	= 1000,
	MP2V_FREQ_90KHZ	= 90000,
	MP2V_FREQ_DSHOW	= 10000000
};

enum MP2V_TS_PRECISION
{
	MP2V_TS_PRECISION_DVD		= 32,
	MP2V_TS_PRECISION_PROGRAM	= 33,
	MP2V_TS_PRECISION_DSHOW		= 64
};

enum MP2V_TS_FLAG
{
	MP2V_TS_FLAG_INVALID = (1<<0),
	MP2V_TS_FLAG_SIGNED	= (1<<1)
};

#endif //_MP2V_TS_DEFINED_

struct SurfaceInfo;

typedef HRESULT
(*PFN_MP2MP_VIDEO_GET_DATA)(
							IN PVOID pvContext,
							OUT const BYTE **ppbOutBuffer,
							OUT DWORD *pcbNumberOfBytesRead,
							OUT BOOL *pbHasPTS,
							OUT MP2V_TS *pTimeStamp
							);

typedef HRESULT
(*PFN_MP2MP_VIDEO_SKIP_FRAME)(
							  IN PVOID pvContext,
							  IN DWORD dwPictureCodingType,
							  IN MP2V_TS *pTimeStamp,
							  OUT BOOL *pbSkip
							  );

typedef HRESULT
(*PFN_MP2MP_ReSetDXVA)(
					   IN	HRESULT error,
					   OUT LPVOID* pVideoAccel
					   );

typedef HRESULT
(*PFN_DXVA_BeginFrame)(
					   IN PVOID disp_instance,
					   IN WORD frame_index
					   );

typedef HRESULT
(*PFN_DXVA_EndFrame)(
					 IN PVOID disp_instance,
					 IN WORD frame_index
					 );


typedef HRESULT
(*PFN_DXVA_LockSurface)(
						IN PVOID disp_instance,
						IN WORD frame_index, 
						OUT SurfaceInfo *pInfo
						);

typedef HRESULT
(*PFN_DXVA_UnLockSurface)(
						  IN PVOID disp_instance,
						  IN WORD frame_index
						  );

typedef struct _DXVA_DrawSetParametersInfo
{
	int width;
	int height;
	int frame_rate_code;
	int MPEG2_Flag;
	int aspect_ratio_information;
	TiviDxvaP			pAccel;
	DXVA_ENCRYPTBase*	pEncrypt;
	BOOL bResetAccel;
} DXVA_DrawSetParametersInfo;
typedef HRESULT
(*PFN_DXVA_DrawSetParameters)(
							  IN PVOID disp_instance,
							  IN PVOID info
							  );

typedef HRESULT 
(*MP2VDec_OnOpenKeyFcn)(
						IN LPVOID pParam,
						IN BYTE reqData[16], 
						IN DWORD dwStrmType, 
						OUT BYTE rspData[16]
						);

typedef HRESULT
(*PFN_DXVA_GET_RESOLUTION)(
						   IN void *pvContext, 
						   IN int width, 
						   IN int height, 
						   IN int aspect_ratio_information, 
						   OUT LPVOID *pEncrypt
						   );

struct MP2VDecMP_OpenOptions
{
	DWORD dwThreads;							/// number of threads to use (0 or 1) = 1; 2 = 2, etc.
	DWORD dwThreadAffinity;						/// the preferred processor affinity for the threads above, currently ignored by decoder.
	DWORD dwBuffers;							/// decode buffers, (MINIMUM 3)
	PFN_MP2MP_VIDEO_GET_DATA pfnDataCallback;	/// Callback function provided by the client.  The decoder should call 
	/// this function when it needs more input data to perform decode.
	PVOID pvDataContext;						/// Context passed as a first argument to the data callback function.
	PFN_MP2MP_VIDEO_SKIP_FRAME pfnSkipFrameCallback; /// Callback function provided by the client.  The decoder should call 
	// this function when it needs more input data to perform decode.
	PVOID pvSkipFrameContext;						/// Context passed as a first argument to the data callback function.
	DWORD dwOpenFlags;

	PVOID	pdisp_instance;
	PFN_DXVA_BeginFrame pfnDXVA_BeginFrame;
	PFN_DXVA_EndFrame	pfnDXVA_EndFrame;
	PFN_DXVA_LockSurface		pfnDXVA_LockSurface;
	PFN_DXVA_UnLockSurface		pfnDXVA_UnLockSurface;
	PFN_DXVA_DrawSetParameters pfnDXVA_DrawSetParameters;
	TPiviDXVAConfigPictureDecode	pdxvaConfigPictureDecode;
	LPDXVA_ConfigAlphaLoad		pdxvaAlphaLoadConfig;
	LPDXVA_ConfigAlphaCombine	pdxvaAlphaCombConfig;
	LPDXVA_GeneralInfo			pdxvaGeneralInfo;
	DXVA_ENCRYPTBase*			pdxvaConfigEncryption;
	TiviDxva					pAccel;
	PFN_MP2MP_ReSetDXVA			pfnReSetDXVACallback;
	PFN_DXVA_GET_RESOLUTION pfnDXVA_DrawGetResolutionCallback;

	BOOL *pbVobuBegin;
	BOOL *pbSurfaceLost;
	Cline21 **ppLine21;
	CRITICAL_SECTION *pcsLine21;
	BOOL *pbLine21;
	BOOL *pbIsValidSpeedLine21;
	DWORD dwDXVAERR;
	DWORD dwEncryptForced;
	BOOL bSingleSliceDecode;
	BYTE *pSeqHdr;	
	int  iSeqHdrSize;
	DWORD	m_dwVendorID;
	DWORD	m_dwDeviceID;
};

enum MP2V_OPENFLAG
{
	MP2V_OPENFLAG_PLANARUV = (1<<0),				// chose planar UV (default merged UV)
};

enum MP2V_DECODEFLAG
{
	MP2V_DECODEFLAG_SKIPFIRSTB = (1<<0),			/// if at the start of the sequence, skip B frames
	MP2V_DECODEFLAG_WAITFORFRAME = (1<<1),			/// block for available frame
	MP2V_DECODEFLAG_REVERSEPLAY = (1<<2),			/// reverse play preloading enabled
	MP2V_DECODEFLAG_NOSKIPCURFRAME = (1<<3),			/// Frame drop decoding.
	MP2V_DECODEFLAG_SKIPCURFRAME = (1<<4),			/// Frame drop decoding.
	MP2V_DECODEFLAG_SKIPPBFRAME = (1<<5),			/// Frame drop decoding.
	MP2V_DECODEFLAG_GMODECODEMODE = (1<<6),			/// Use GMO decoder to decode frame.
};

enum MP2V_PLAY_STATE
{
	MP2V_PLAY_STATE_NORMAL,
	MP2V_PLAY_STATE_REVERSE_STORE_FIRST,
	MP2V_PLAY_STATE_REVERSE_STORE_PRELOAD,
	MP2V_PLAY_STATE_REVERSE_NORMAL,
	MP2V_PLAY_STATE_REVERSE_READ_PRELOAD,
};

struct MP2VDecMP_DecodeOptions
{
	DWORD dwDecodeFlags;
};

enum MP2V_GETFRAMEFLAG
{
	MP2V_GETFRAMEFLAG_PEEK = (1<<0),
	MP2V_GETFRAMEFLAG_REFERENCE = (1<<1),
};

struct MP2VDecMP_GetFrameOptions
{
	DWORD dwGetFrameFlags;				/// options for getting frame
};

enum MP2V_FRAMETYPE
{
	MP2V_FRAMETYPE_PLANAR420 = 1,
	MP2V_FRAMETYPE_MERGED420
};

enum MP2V_ERRORSTATUS
{
	MP2V_ERRORSTATUS_NONE,
	MP2V_ERRORSTATUS_LOW,
	MP2V_ERRORSTATUS_MEDIUM,
	MP2V_ERRORSTATUS_HIGH
};

struct MP2VDecMP_Frame
{
	DWORD		dwCookie;				/// unique signature used to release frame
	DWORD		dwFrameCounter;			/// used to indicate presence of dropped frames.
	DWORD		dwFrameType;			/// frame storage type
	DWORD		dwErrorStatus;			/// error level
	DWORD		adwWidth[3];			/// width
	DWORD		adwHeight[3];			/// height
	DWORD		adwStride[3];			/// stride of each buffer
	PBYTE		apbFrame[3];			/// pointer to each buffer
	PBYTE		apbDSFrame[3];			/// pointer to downsample buffer
	PBYTE       pbLine21Buf;            /// line21 information
	DWORD       dwLine21Len;            /// length of line21 information
	MP2V_TS     pts;					/// presentation timestamp
	// normative variables (or thusly derived)
	int			frame_index;
	DWORD		frame_rate_code;
	DWORD		frame_rate_extension_n;
	DWORD		frame_rate_extension_d;
	DOUBLE		bit_rate;
	DWORD		horizontal_size;
	DWORD		vertical_size;
	DWORD		coded_picture_width;	// derivable from horizontal_size
	DWORD		coded_picture_height;	// derivable from vertical_size
	DWORD		chroma_format;
	DWORD		aspect_ratio_information;
	DWORD		progressive_sequence;
	DWORD		picture_coding_type;
	DWORD		picture_structure;
	DWORD		temporal_reference;
	DWORD		repeat_first_field;
	DWORD		top_field_first;
	DWORD		progressive_frame;
	LONG		frame_center_horizontal_offset[3];
	LONG		frame_center_vertical_offset[3];
	LONG		display_horizontal_size;
	LONG		display_vertical_size;
	BOOL		is_gop_firstframe;
	LONG		gop_header;
	int			MPEG2_Flag;
};

// {C090F2EF-1668-4aff-BAE1-7110ABBAE575}
EXTERN_GUID(IID_IMP2VDecDllAPI, 0xc090f2ef, 0x1668, 0x4aff, 0xba, 0xe1, 0x71, 0x10, 0xab, 0xba, 0xe5, 0x75);
class IMP2VDecDllAPI : public IUnknown
{
public:
	virtual HRESULT MP2VDecMP_Create(
		OUT LPVOID *ppDecoder,
		IN const bool IsEnableDXVA
		) = 0;

	virtual HRESULT MP2VDecMP_Release(
		IN LPVOID pDecoder
		) = 0;
	virtual HRESULT MP2VDecMP_AddRef(
		IN LPVOID pDecoder
		) = 0;

	virtual HRESULT MP2VDecMP_Open(
		IN LPVOID pDecoder,
		IN const MP2VDecMP_OpenOptions *pOptions,
		IN const DWORD dwSize
		) = 0;

	virtual HRESULT MP2VDecMP_Close(
		IN LPVOID pDecoder
		) = 0;

	virtual HRESULT MP2VDecMP_Stop(
		IN LPVOID pDecoder,
		IN const DWORD dwStopFlags
		) = 0;

	virtual HRESULT MP2VDecMP_DecodeFrame(
		IN LPVOID pDecoder,
		IN const MP2VDecMP_DecodeOptions *pOptions,
		IN const DWORD dwSize,
		OUT DWORD *pdwNumberOfDecodedFrames
		) = 0;

	virtual HRESULT MP2VDecMP_GetFrame(
		IN LPVOID pDecoder,
		IN const MP2VDecMP_GetFrameOptions *pOptions,
		IN const DWORD dwSize,
		OUT MP2VDecMP_Frame *pFrame,
		IN const DWORD dwFrameSize
		) = 0;
	virtual HRESULT MP2VDecMP_AddRefFrame(
		IN LPVOID pDecoder,
		IN DWORD dwCookie
		) = 0;

	virtual HRESULT MP2VDecMP_ReleaseFrame(
		IN LPVOID pDecoder,
		IN DWORD dwCookie
		) = 0;

	virtual HRESULT MP2VDecMP_ReleaseBuffer(
		IN LPVOID pDecoder,
		IN DWORD dwReleaseFlags,
		OUT LPBYTE *ppBufferPtr
		) = 0;

	virtual HRESULT MP2VDecMP_Get(
		IN LPVOID pDecoder,
		IN DWORD dwPropID,
		OUT LPVOID pPropData,
		IN DWORD cbPropData,
		OUT DWORD *pcbReturned
		) = 0;

	virtual HRESULT MP2VDecMP_Set(
		IN LPVOID pDecoder,
		IN DWORD dwPropID,
		IN LPVOID pPropData,
		IN DWORD cbPropData
		) = 0;

	virtual HRESULT MP2VDecMP_OpenKey(
		IN LPVOID pDecoder,
		IN MP2VDec_OnOpenKeyFcn pCallBack,
		IN LPVOID pParam
		) = 0;
};

/*
//	Temporal COM decoder object.
EXTERN_GUID(CLSID_CMP2VDecDXVADllModule,0x62B722E1,0xCCCA,0x425b,0x80,0xF1,0x5C,0xA1,0x4A,0xF0,0x2E,0xDA);
/EXTERN_GUID(CLSID_CMP2VDecDllModule,0x7F054F09,0xA88E,0x486f,0x93,0xC9,0x6C,0x48,0xC9,0xB7,0x77,0x88);

// IID_IMP2VDecDllBase
// {34B94094-B5AF-4a11-846E-904824164CD0}
// DEFINE_GUID(IID_IMP2VDecDllBase, 0x34b94094, 0xb5af, 0x4a11, 0x84, 0x6e, 0x90, 0x48, 0x24, 0x16, 0x4c, 0xd0);
interface __declspec(uuid("{34B94094-B5AF-4a11-846E-904824164CD0}"))
IMP2VDecDllBase : IUnknown
{
	virtual HRESULT MP2VDecMP_Create(
		OUT LPVOID *ppDecoder
		) = 0;

	virtual HRESULT MP2VDecMP_Release(
		IN LPVOID pDecoder
		) = 0;
	virtual HRESULT MP2VDecMP_AddRef(
		IN LPVOID pDecoder
		) = 0;

	virtual HRESULT MP2VDecMP_Open(
		IN LPVOID pDecoder,
		IN const MP2VDecMP_OpenOptions *pOptions,
		IN const DWORD dwSize
		) = 0;

	virtual HRESULT MP2VDecMP_Close(
		IN LPVOID pDecoder
		) = 0;

	virtual HRESULT MP2VDecMP_Stop(
		IN LPVOID pDecoder,
		IN const DWORD dwStopFlags
		) = 0;

	virtual HRESULT MP2VDecMP_DecodeFrame(
		IN LPVOID pDecoder,
		IN const MP2VDecMP_DecodeOptions *pOptions,
		IN const DWORD dwSize,
		OUT DWORD *pdwNumberOfDecodedFrames
		) = 0;

	virtual HRESULT MP2VDecMP_GetFrame(
		IN LPVOID pDecoder,
		IN const MP2VDecMP_GetFrameOptions *pOptions,
		IN const DWORD dwSize,
		OUT MP2VDecMP_Frame *pFrame,
		IN const DWORD dwFrameSize
		) = 0;
	virtual HRESULT MP2VDecMP_AddRefFrame(
		IN LPVOID pDecoder,
		IN DWORD dwCookie
		) = 0;

	virtual HRESULT MP2VDecMP_ReleaseFrame(
		IN LPVOID pDecoder,
		IN DWORD dwCookie
		) = 0;

	virtual HRESULT MP2VDecMP_ReleaseBuffer(
		IN LPVOID pDecoder,
		IN DWORD dwReleaseFlags,
		OUT LPBYTE *ppBufferPtr
		) = 0;

	virtual HRESULT MP2VDecMP_Get(
		IN LPVOID pDecoder,
		IN DWORD dwPropID,
		OUT LPVOID pPropData,
		IN DWORD cbPropData,
		OUT DWORD *pcbReturned
		) = 0;

	virtual HRESULT MP2VDecMP_Set(
		IN LPVOID pDecoder,
		IN DWORD dwPropID,
		IN LPVOID pPropData,
		IN DWORD cbPropData
		) = 0;

	virtual HRESULT MP2VDecMP_OpenKey(
		IN LPVOID pDecoder,
		IN MP2VDec_OnOpenKeyFcn pCallBack,
		IN LPVOID pParam
		) = 0;
};
*/
#endif

