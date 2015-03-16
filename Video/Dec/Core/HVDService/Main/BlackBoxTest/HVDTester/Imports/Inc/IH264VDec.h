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

#ifndef _I_H264_VDEC_H_
#define _I_H264_VDEC_H_

#include <windows.h>

#if (!defined(__linux__))
#include "iviDXVAdef.h"
#include <d3d9.h>
#include <dxva.h>
#include <dxva2api.h>
#endif

class DXVA_ENCRYPTBase;

#if !defined(E_PROP_ID_UNSUPPORTED)
#define E_PROP_ID_UNSUPPORTED     ((HRESULT)0x80070490L)
#endif

#define PERFORM_FRAME_POINTER_SCRAMBLING
#define _USE_SCRAMBLE_DATA_
#define H264_SCRAMBLE_DATA_LEN	256

#ifdef PERFORM_FRAME_POINTER_SCRAMBLING
#define SCRAMBLE_PARAM_1 0x73EE3EAF
#define SCRAMBLE_PARAM_2 0x3F264FCE
#define SCRAMBLE_PARAM_3 0xE25B06C1
#define INLINE_XOR1(x,y) (((x) & ~(y)) | ((y) & ~(x)))
#define INLINE_XOR2(x,y) (((x) | (y)) & (~((x) & (y))))
#define INLINE_SCRAMBLE(x,param1,param2,param3) INLINE_XOR2((INLINE_XOR1(x,param1) + param2), param3)
#define INLINE_DESCRAMBLE(x,param1,param2,param3) INLINE_XOR2((INLINE_XOR1(x,param3) + (0xffffffff-param2+1)), param1)
#else
#define INLINE_SCRAMBLE(x,param1,param2,param3) (x)
#define INLINE_DESCRAMBLE(x,param1,param2,param3) (x)
#endif

#define LINE21BUF_SIZE 256

typedef void H264VDecHP;

namespace H264VDecParam
{
#if !defined(_H264_TS_DEFINED_)
#define _H264_TS_DEFINED_

	typedef struct H264_TS_
	{
		unsigned __int64	ts;
		unsigned int		freq;
		unsigned char		tslength;
		unsigned char		flags;
		unsigned char		unused1;
		unsigned char		unused2;
	}H264_TS;

	enum H264_FREQ
	{
		H264_FREQ_1KHZ	= 1000,
		H264_FREQ_90KHZ	= 90000,
		H264_FREQ_DSHOW	= 10000000
	};

	enum H264_TS_PRECISION
	{
		H264_TS_PRECISION_DVD		= 32,
		H264_TS_PRECISION_PROGRAM	= 33,
		H264_TS_PRECISION_DSHOW		= 64
	};

	enum H264_TS_FLAG
	{
		H264_TS_FLAG_INVALID = (1<<0),
		H264_TS_FLAG_SIGNED	= (1<<1)
	};

#endif //_H264_TS_DEFINED_

#if !defined(_H264_REG_DEFINED_)
#define _H264_REG_DEFINED_

	enum H264_REG
	{
		// Note, smart decoding used the first 3 bits; 1 ~ 7.
		H264_REG_SMART_DEC_1    = 1,		// SMARTDEC: Level 1, least quality degradation
		H264_REG_SMART_DEC_2    = 2,		// SMARTDEC: Level 2, ...
		H264_REG_SMART_DEC_3    = 3,		// SMARTDEC: Level 3, middle quality degradation
		H264_REG_SMART_DEC_4    = 4,		// SMARTDEC: Level 4, ...
		H264_REG_SMART_DEC_5    = 5,		// SMARTDEC: Level 5, worst quality degradation
		H264_REG_SMART_DEC_6    = 6,		// SMARTDEC: Level 6, special usage
		H264_REG_DEBLOCKING     = 1 << 3,	// DEBLOCKING: 0, disable; 1, enable.
		H264_REG_DROP_FRAME     = 1 << 4,	// NOVIDEODROP: 0, enable; 1, disable.
		H264_REG_NVFORMAT       = 1 << 5,
		H264_REG_SET_DPBBUFSIZE = 1 << 6,
		H264_REG_HYBRID_DECODE1 = 1 << 7,   
		H264_REG_HYBRID_DECODE2 = 1<<8,  
		H264_REG_RESIZE_SD      = 1<<9,	//Resize output frame. (half-width and half-height)
		H264_REG_RESIZE_HEIGHT  = 1<<11,	//Resize output frame. (half-height)
		H264_REG_HYBRID_DECODE3 = 1<<12,  // HW_PB = 5
		H264_REG_INTELFORMAT = 1<<13,
		H264_REG_END
	};

	enum H264_PROPID
	{
		H264_PROPID_SMART_DECODE = 1, // Set the smart decoding level between 1 to 5 in above definition.
		H264_PROPID_NVCTRL_SPEED = 2,	// Set interpolate skip one- or two-B or not.
		H264_PROPID_DOWNSAMPLE   = 3,	// Set Downsample: width, height, or none.
		H264_PROPID_EOS          = 4,	// Set EOS by upper layer.
		H264_PROPID_END          = 5,
		H264_PROPID_FLUSH_EMPTY_STREAM_DATA = 6,
	};

#endif//_H264_REG_DEFINED_

#if !defined(_H264_VGACARD_DEFINED_)
#define _H264_VGACARD_DEFINED_

	enum E_H264_VGACARD
	{
		E_H264_VGACARD_ATI     = 1,
		E_H264_VGACARD_NVIDIA  = 1 << 1,
		E_H264_VGACARD_INTEL   = 1 << 2,
		E_H264_VGACARD_S3      = 1 << 3,
		E_H264_VGACARD_GENERAL = 1 << 4
	};

	enum E_H264_DXVA_MODE
	{
		E_H264_DXVA_ATI_PROPRIETARY_A    = 1,		// ATI DXVA PROPRIETARY HWMC level
		E_H264_DXVA_ATI_PROPRIETARY_E    = 1 << 1,	// ATI DXVA PROPRIETARY BA level
		E_H264_DXVA_NVIDIA_PROPRIETARY_A = 1 << 2,	// NVIDIA DXVA PROPRIETARY HWMC level
		E_H264_DXVA_MODE_A			= 1 << 3,	// MS DXVA HWMC level
		E_H264_DXVA_MODE_B			= 1 << 4,	// MS DXVA HWMC-FGT level
		E_H264_DXVA_MODE_C			= 1 << 5,	// MS DXVA IDCT level
		E_H264_DXVA_MODE_D			= 1 << 6,	// MS DXVA IDCT-FGT level
		E_H264_DXVA_MODE_E			= 1 << 7,	// MS DXVA BA level
		E_H264_DXVA_MODE_F			= 1 << 8,	// MS DXVA BA-FGT level
	};

#endif//_H264_VGACARD_DEFINED_

	enum E_H264_DATA_STATUS
	{
		E_H264_DATA_STATUS_SUCCESS	= 0,
		E_H264_DATA_STATUS_SUCCESS_NO_SCRAMBLE = 1,
		E_H264_DATA_STATUS_NO_DATA	= _HRESULT_TYPEDEF_(0x80004004L),
		E_H264_DATA_STATUS_DATA_DISCONTINUITY	= _HRESULT_TYPEDEF_(0x80004005L),
		E_H264_DATA_STATUS_READ_ERROR	= _HRESULT_TYPEDEF_(0x80004006L)
	};
	
	enum APP_CONSTRAINTS
	{
		H264_MVC_NO_CONSTRAINTS,
		H264_BD,
		MVC_3DBD
	};

	typedef union
	{
		struct {
			short x;
			short y;
		};
		unsigned int mv_comb;
	} H264DecHP_MotionVector;

	typedef struct H264DecHP_storage_info_
	{
		H264DecHP_MotionVector    mv[2][4*4];
		char			ref_idx[2][4];
		char	 		ref_pic_id[2][4];
	} H264DecHP_storage_info;

	typedef struct H264DecHP_Macroblock_
	{
		// some storage of macroblock syntax elements for global access     
		unsigned long	cbp_blk ;
		short         qp;
		short         mb_type;

		short         slice_nr;
		char          luma_transform_size_8x8_flag;
		char			filterLeftMbEdgeFlag;
		char			filterTopMbEdgeFlag;
		char          mb_field;

		short         do_record;  

		H264DecHP_storage_info   pred_info;

	} H264DecHP_Macroblock;

	typedef struct H264VDecHP_Frame_
	{
		DWORD		dwCookie;				/// unique signature used to release frame
		DWORD		dwErrorStatus;			/// future
		DWORD		dwPictureOrderCounter;	/// temporary
		DWORD		adwWidth[3];			/// width
		DWORD		adwHeight[3];			/// height
		DWORD		adwStride[3];			/// stride of each buffer
		DWORD		adwLeft[3];				/// left
		DWORD		adwTop[3];				/// top
		DWORD   dwXAspect;
		DWORD   dwYAspect;
		DWORD   dwBitRate;
		PBYTE		apbFrame[3];			/// pointer to each buffer
		H264_TS pts;					/// presentation timestamp
		DWORD		decode_errors;
		BOOL		bRgbOutput;
		int chroma_format_idc;
		unsigned char symbol_size_in_bytes;
		unsigned int frame_num;
		unsigned int pic_num;
		unsigned int progressive_frame;
		int slice_type;
		int frame_index;
		unsigned long framerate1000;
		int dpb_buffer_size;
		BOOL bRepeatFirstField;
		int top_field_first;
		BYTE  m_cc1buf[LINE21BUF_SIZE]; // cc data of 1st field or frame
		int   m_cc1len;
		BYTE   pbYCCBuffer[12];
		DWORD  dwYCCBufferLen;

		//for transcoder
		H264DecHP_Macroblock *mb_info;
	}H264VDecHP_Frame;

	struct SurfaceInfo
	{
		void *pBits;			//< pointer to the start of the uncompressed buffer.
		unsigned int uPitch;
		DWORD dwFourcc;			//< fourcc format of the uncompressed buffer.
	};

	typedef
		HRESULT
		(__cdecl *PFN_H264HP_GET_DATA)(
		IN PVOID pvContext,
		OUT const BYTE **ppbOutBuffer,
		OUT DWORD *pcbNumberOfBytesRead,
		OUT BOOL *pbHasPTS,
		OUT H264_TS *pTimeStamp
		);

	typedef
		HRESULT
		(__cdecl *PFN_H264HP_VIDEO_SKIP_FRAME)(
		IN PVOID pvContext,
		IN DWORD dwPictureType,
		IN H264_TS *pTimeStamp,
		OUT BOOL *pbSkip
		);

	typedef
		HRESULT
		(__cdecl *PFN_H264HP_DXVA_GET_RESOLUTION)(
		IN PVOID pvContext,
		IN int width,
		IN int height,
		IN int aspect_ratio_information,
		OUT LPVOID *pIviCP
		);

	typedef 
		HRESULT
		(__cdecl *PFN_H264HP_SET_LINE21)(
		IN PVOID pvContext, 
		IN BOOL bframe,
		IN PVOID line21buf, 
		IN int line21length,
		IN int ccnum,
		IN H264_TS pts
		);

	typedef 
		HRESULT
		(__cdecl *PFN_MP2MP_ReSetDXVA)(
		IN	HRESULT error,
		OUT LPVOID* pVideoAccel
		);

	typedef 
		HRESULT
		(__cdecl *PFN_DXVA_BeginFrame)(
		IN PVOID disp_instance,
		IN WORD frame_index
		);

	typedef 
		HRESULT
		(__cdecl *PFN_DXVA_EndFrame)(
		IN PVOID disp_instance,
		IN WORD frame_index
		);

	typedef 
		HRESULT
		(__cdecl *PFN_DXVA_DrawSetParameters)(
		IN PVOID disp_instance,
		IN PVOID info
		);

	typedef 
		HRESULT
		(__cdecl *PFN_DXVA_LockSurface)(
		IN PVOID disp_instance,
		IN WORD frame_index, 
		IN SurfaceInfo *pInfo
		);

	typedef 
		HRESULT
		(__cdecl *PFN_DXVA_UnLockSurface)(
		IN PVOID disp_instance,
		IN WORD frame_index
		);

	typedef
		HRESULT
		(__cdecl *H264VDec_OnOpenKeyFcn)(
		IN LPVOID pParam,
		IN BYTE reqData[16], 
		IN DWORD dwStrmType, 
		OUT BYTE rspData[16]
	);

	typedef struct H264VDecHP_OpenOptions_
	{
		DWORD dwThreads;              /// number of threads to use (0 or 1) = 1; 2 = 2, etc.
		DWORD dwThreadAffinity;       /// the preferred processor affinity for the threads above, currently ignored by decoder.
		DWORD dwBuffers;              /// display Q size. (MINIMUM 3)
		PFN_H264HP_GET_DATA pfnDataCallback; /// Callback function provided by the client.  The decoder should call 
		/// this function when it needs more input data to perform decode.
		PVOID pvDataContext;         /// Context passed as a first argument to the data callback function.
		PFN_H264HP_VIDEO_SKIP_FRAME pfnSkipFrameCallback; /// Callback function provided by the client.  The decoder should call 
		// this function when it needs more input data to perform decode.
		PVOID pvSkipFrameContext;    /// Context passed as a first argument to the data callback function.
		LPVOID lpvVideoAccel;
		DWORD dwFillFrameNumGap;
		PFN_H264HP_SET_LINE21 pfnSetLine21Callback;
		PVOID pvSetLine21Context;
		PFN_H264HP_DXVA_GET_RESOLUTION pfnDXVA_DrawGetResolutionCallback;

		DWORD dwH264RegKey;          /// Pack the functional register key "H264_REG" to the decoder.
		UINT uiH264VGACard;          /// Specify the vendor of VGA card.
		UINT uiH264DXVAMode;         /// Specify the DXVA profile mode.

		HANDLE hGetDisplaySemaphore;  //display event
		TiviDxva     pAccel;
		LPVOID       pIviCP; // data scrambling
		TPiviDXVAConfigPictureDecode  pdxvaConfigPictureDecode;
		LPDXVA_ConfigAlphaLoad        pdxvaAlphaLoadConfig;  // <DXVA> Alpha Picture configuration
		LPDXVA_ConfigAlphaCombine     pdxvaAlphaCombConfig;  // <DXVA> Alpha Picture combination configuration
		LPDXVA_GeneralInfo            pdxvaGeneralInfo;      // <DXVA> DXVA_GeneralInfo
		DXVA_ENCRYPTBase*             pdxvaConfigEncryption; // <DXVA> Encryption configuration

		PVOID                         pdisp_instance; // <DXVA 2.0> for VRN display BeginFrame,EndFrame,DrawSetPrarmeters
		PFN_DXVA_BeginFrame           pfnDXVA_BeginFrame;
		PFN_DXVA_EndFrame             pfnDXVA_EndFrame;
		PFN_DXVA_DrawSetParameters    pfnDXVA_DrawSetParameters;
		PFN_DXVA_LockSurface          pfnDXVA_LockSurface;
		PFN_DXVA_UnLockSurface        pfnDXVA_UnLockSurface;

		DWORD dwIsNotHDDVD;

		DWORD dwReserved[20];
	}H264VDecHP_OpenOptions;

	typedef struct H264VDecHP_DecodeOptions_
	{
	}H264VDecHP_DecodeOptions;

	typedef struct H264VDecHP_GetFrameOptions_
	{
	}H264VDecHP_GetFrameOptions;
};

// IH264VDecDllAPI
// {D53C9CC0-AA1B-410f-9A75-01A96AE18229}
EXTERN_GUID(IID_IH264VDecDllAPI, 0xd53c9cc0, 0xaa1b, 0x410f, 0x9a, 0x75, 0x1, 0xa9, 0x6a, 0xe1, 0x82, 0x29);
class IH264VDecDllAPI : public IUnknown
{
public:
	virtual HRESULT __stdcall H264VDec_Create(
		OUT H264VDecHP **ppDecoder
		) = 0;

	virtual HRESULT __stdcall H264VDec_Release(
		IN H264VDecHP *pDecoder
		) = 0;

	virtual HRESULT __stdcall H264VDec_Open(
		IN H264VDecHP *pDecoder,
		IN const H264VDecParam::H264VDecHP_OpenOptions *pOptions,
		IN const DWORD dwSize,
		OUT void **imgp
		) = 0;

	virtual HRESULT __stdcall H264VDec_Close(
		IN H264VDecHP *pDecoder,
		IN void *imgp
		) = 0;

	virtual HRESULT __stdcall H264VDec_Stop(
		IN H264VDecHP *pDecoder,
		IN const DWORD dwStopFlags,
		IN void *imgp
		) = 0;

	virtual HRESULT __stdcall H264VDec_DecodeFrame(
		IN H264VDecHP *pDecoder,
		IN const H264VDecParam::H264VDecHP_DecodeOptions *pOptions,
		IN const DWORD dwSize,
		OUT DWORD *pdwNumberOfDecodedFrames,
		OUT DWORD *pdwNumberOfSkippedFrames,
		IN void *imgp
		) = 0;

	virtual HRESULT __stdcall H264VDec_GetFrame(
		IN H264VDecHP *pDecoder,
		IN const H264VDecParam::H264VDecHP_GetFrameOptions *pOptions,
		IN const DWORD dwSize,
		OUT H264VDecParam::H264VDecHP_Frame *pFrame,
		IN const DWORD dwFrameSize,
		IN void *imgp
		) = 0;

	virtual HRESULT __stdcall H264VDec_ReleaseFrame(
		IN PVOID pDecoder,
		IN DWORD dwCookie,
		IN void *imgp
		) = 0;

	virtual HRESULT __stdcall H264VDec_ReleaseBuffer(
		IN PVOID pDecoder,
		IN const DWORD dwReleaseFlags,
		OUT LPBYTE *ppBufferPtr
		) = 0;

	virtual HRESULT __stdcall H264VDec_Get(
		IN H264VDecHP *pDecoder,
		IN DWORD dwPropID,
		IN LPVOID pPropData,
		IN DWORD cbPropData,
		OUT DWORD *pcbReturned
		) = 0;

	virtual HRESULT __stdcall H264VDec_Set(
		IN H264VDecHP *pDecoder,
		IN DWORD dwPropID,
		IN LPVOID pPropData,
		IN DWORD cbPropData
		) = 0;

	virtual HRESULT __stdcall H264VDec_AddRef(
		IN H264VDecHP *pDecoder
		) = 0;

	virtual HRESULT __stdcall H264VDec_AddRefFrame(
		IN PVOID pDecoder,
		IN DWORD dwCookie
		) = 0;
};

// IH264VDecDllDXVA
// {90DE1BA9-D7F0-420c-B36E-CCC00C4ADCD0}
EXTERN_GUID(IID_IH264VDecDllDXVA, 0x90de1ba9, 0xd7f0, 0x420c, 0xb3, 0x6e, 0xcc, 0xc0, 0x0c, 0x4a, 0xdc, 0xd0);
class IH264VDecDllDXVA : public IUnknown
{
public:
	virtual HRESULT __stdcall H264VDec_Reset_DXVA(
		IN LPVOID pVideoAccel,
		IN DWORD dwCookie,
		IN PVOID pDecoder,
		IN void *imgp
		) = 0;

	virtual HRESULT __stdcall H264VDec_Release_DXVA(
		IN PVOID pDecoder,
		void *imgp
		) = 0;
};


// IH264VDecDllAPI2
// {9E356284-B46A-4304-9B9C-355D5EEA1E9A}
EXTERN_GUID(IID_IH264VDecDllAPI2, 0x9e356284, 0xb46a, 0x4304, 0x9b, 0x9c, 0x35, 0x5d, 0x5e, 0xea, 0x1e, 0x9a);
class IH264VDecDllAPI2 : public IH264VDecDllAPI
{
public:
	virtual HRESULT __stdcall H264VDec_GetDisplayStatus(
		IN H264VDecHP *pDecoder,
		IN const DWORD dwIdx,
		IN void *imgp
		) = 0;

	virtual HRESULT __stdcall H264VDec_FinishDisplay(
		IN H264VDecHP *pDecoder,
		IN DWORD dwEstimateNextFrame,
		IN void *imgp
		) = 0;

	virtual HRESULT __stdcall H264VDec_OpenKey(
		IN H264VDecHP *pDecoder,
		IN H264VDecParam::H264VDec_OnOpenKeyFcn pCallBack,
		IN LPVOID pParam
		) = 0;
};

#endif //_I_H264_VDEC_H_