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
// 	Copyright (c) 1998 - 2005  InterVideo Corporation.  All Rights Reserved.
//
//-----------------------------------------------------------------------------
//

#ifndef _VC1VDEC_DEF_H_
#define _VC1VDEC_DEF_H_

#ifdef __cplusplus
//extern "C" {
#endif

#include "VC1CallBackFCN.h"

#ifdef USE_FOR_WINDVD
#include <dxva.h>
#include "Ividxva2.h"
#else
#include <d3d9.h>
#include <dxva.h>
#include "iviDXVAdef.h"
#endif

#define VC1_SCRAMBLE_DATA_LEN 256
#define VC1_LINE21BUF_SIZE 256

struct IAMVideoAccelerator;
struct IDirectXVideoDecoder;
interface ICPService;

namespace VC1VDecParam
{
	typedef struct VC1VDec_OpenOptions_
	{
		DWORD  dwThreads;                   /// number of decoder threads to use (0 or 1) = 1; 2 = 2, etc.
		DWORD  dwThreadAffinity;            /// the preferred processor affinity for the threads above, currently ignored by decoder.
		DWORD  dwBuffers;                   /// display Q size. (MINIMUM 3)
		DWORD  dwStreamType;                /// streaming mode

		PFN_VC1_GET_DATA pfnDataCallback;   /// Callback function provided by the client.  The decoder should call 
                                        /// this function when it needs more input data to perform decode.
		PVOID  pvDataContext;               /// Context passed as a first argument to the data callback function.
		long   lBaseOption;
		DWORD  dwVC1RegKey; 

		TiviDxvaVer dxvaVer;
		DWORD  dwVendorID;
		DWORD  dwVideoProfileID;            /// indicate DXVA guid. (differ from Profile, define in dxva.h)

		PFN_VC1_GET_PARAM  pfnGetParamCallback; /// General callback function
		ICPService *pIviCP;                      /// data scrambling

		DWORD  dwReserved[32];

	}VC1VDec_OpenOptions;

	typedef struct VC1VDec_DecodeOptions_
	{
		DWORD	dwNumberOfProgressiveFrames;	/// number of decoded progressive frames.
		DWORD	dwNumberOfPFrames;		/// number of decoded P frames
		DWORD	dwNumberOfBFrames;		/// number of decoded B frames
		DWORD	dwNumberOfIFrames;		/// number of decoded I frames
		DWORD	dwNumberOfBIFrames;		/// number of decoded BI frames
		DWORD	dwNumberOfSFrames;		/// number of skipped frames (NotDecode, not display)
		DWORD	dwFramePeriod;			/// PTS interval of 2 frames.
		DWORD	dwFrames;			/// number of feed frames.
		long	lFrameDropPolicy;		/// vc1 frame dropping policy
		long	lFrameDequalityPolicy;		/// vc1 frame de-quality policy
		long	lVideoStretch40;		/// vc1 frame pts stretched value for bit-rate (40)
		long	lVideoStretch35;		/// vc1 frame pts stretched value for bit-rate (35)
		long	lVideoStretch;			/// vc1 frame pts stretched value for usual case

		DWORD dwReserved[32];

	}VC1VDec_DecodeOptions;

	typedef struct VC1VDec_GetFrameOptions_
	{
		DWORD dwReserved[32];
	}VC1VDec_GetFrameOptions;

	typedef enum {
		VC1_STREAM_INVALID,
		VC1_STREAM_VC1,
		VC1_STREAM_RCV
	} VC1StreamType;

	typedef enum {
		VC1_P_SLICE = 0,
		VC1_B_SLICE,
		VC1_I_SLICE,
		VC1_SP_SLICE,
		VC1_SI_SLICE
	} VC1SliceType;

	enum VC1_SKIPFRMAE_POLICY
	{
		NoSkipFrame	=-1,	//	Decode All Frames.
		CheckSkipEndOfBFrame =0,	// Normal Checking for All stage.
		CheckSkipAllBFrames =1,	// Check All B Frames.
		CheckSkipAllPBFrames =2,	//Check All PB Frames.
		SkipEndOfBFrame =3,	// Always Skip EOB.
		SkipAllBFrames =4,	// Always Skip B Frames.
		SkipAllPBFrames =5,	// Always Skip PB Frames.
		SkipAllIPBFrames =6,// Always Skip IPB Frames.
	};
enum VC1_REG
{
	// Note, smart decoding used the first 3 bits; 1 ~ 7.
	VC1_REG_SMART_DEC_1    = 1,		// SMARTDEC: Level 1, least quality degradation disable in-loop deblocking
	VC1_REG_SMART_DEC_2    = 2,		// SMARTDEC: Level 2, bi-linear only
	VC1_REG_SMART_DEC_3    = 3,		// SMARTDEC: Level 3, half-pixel MC
	VC1_REG_END
};

enum VC1_PROPID
{
	VC1_PROPID_SMART_DECODE = 1, // Set the smart decoding level between 1 to 5 in above definition.
	VC1_PROPID_NVCTRL_SPEED = 2,	// Set interpolate skip one- or two-B or not.
	VC1_PROPID_DOWNSAMPLE   = 3,	// Set Downsample: width, height, or none.
	VC1_PROPID_EOS          = 4,	// Set EOS by upper layer.
	VC1_PROPID_END          = 5,
	VC1_PROPID_FLUSH_EMPTY_STREAM_DATA = 6,
};

enum VC1_PROPID_CB
{
	VC1_PROPID_CB_CHANGE_RESOLUTION = 1,
};

	enum VC1_DEQUALITY_POLICY
	{
		NotUse_Dequality =0,
		Dequality_DeBolck =1,
		Dequality_BottomField =2,
		Dequality_PreciseErrorInterpolation_SkipCbCrWithB =4,  //Won't be used in the future.
		Dequality_PreciseErrorInterpolation_SkipYCbCrWithB =8, //Won't be used in the future.
		Dequality_PreciseErrorInterpolation_SkipCbCrWithP =16, //Won't be used in the future.
		Dequality_PreciseErrorInterpolation_SkipYCbCrWithP =32,//Won't be used in the future.
		Dequality_Half_Resolution = 64,
		Dequality_Quater_Resolution = 128,
	};

	enum E_VC1_ASPECT_RATIO_INFORMATION
	{
		E_VC1_ASPECT_RATIO_RESOLUTION	= 1,
		E_VC1_ASPECT_RATIO_4_3		= 2,
		E_VC1_ASPECT_RATIO_16_9		= 3,
		E_VC1_ASPECT_RATIO_221_110	= 4,
	};

	enum E_VC1_DATA_STATUS
	{
		E_VC1_DATA_STATUS_SUCCESS =1,
		E_VC1_DATA_STATUS_SCRAMBLED_SCRMBLING =2,
		E_VC1_DATA_STATUS_SCRAMBLED_ISMP =3,
	};

	typedef struct VC1VDec_Frame_
	{
		DWORD		dwCookie;				/// unique signature used to release frame
		DWORD		dwErrorStatus;			/// future
		DWORD		dwPictureOrderCounter;	/// temporary
		DWORD		adwWidth[3];			/// width
		DWORD		adwHeight[3];			/// height
		DWORD		adwStride[3];			/// stride of each buffer
		DWORD		adwLeft[3];				/// left
		DWORD		adwTop[3];				/// top
		DWORD       dwXAspect;
		DWORD       dwYAspect;
		DWORD       dwBitRate;
		PBYTE		apbFrame[3];			/// pointer to each buffer
		CVC1TS		cVC1Pts;					/// presentation timestamp
		DWORD		decode_errors;
		BOOL		bRgbOutput;
		int			chroma_format_idc;
		unsigned char symbol_size_in_bytes;
		unsigned int frame_num;
		unsigned int pic_num;
		int		picture_coding_type;

		int		slice_type;
		int		frame_index;
		int		progressive_frame;
		int		picture_structure;
		int		progressive_sequence;
		int		m_iTop_field_first;	// TFF flag
		int		m_iRepeat_first_field;	// RFF flag
		unsigned long 	framerate1000;
		DWORD		frame_rate_code;
		int		m_downSampleFrame; 
		PBYTE       cc1buf;     /// cc info contained in field1 or frame       
		DWORD       cc1len;            
		PBYTE       cc2buf;     /// cc info contained in field2.       
		DWORD       cc2len;   
		int	display_horizontal_size;	// For Display Horizontal Size
		int	display_vertical_size;		// For Display Vertical Size
	}VC1VDec_Frame;

	typedef struct VC1VDecGMO_Frame_
	{
		DWORD dwWidth[3];	// width
		DWORD dwHeight[3];	// height
		DWORD dwStride[3];	// stride of each buffer
		DWORD dwXAspect;
		DWORD dwYAspect;
		DWORD dwBitRate;

		DWORD frame_rate_code;
		CVC1TS cVC1Pts;					// presentation timestamp
		int	frame_index;
		int	progressive_frame;
		int	picture_structure;
		int	picture_coding_type;
		int	progressive_sequence;
		int	m_iTop_field_first;			// TFF flag
		int	m_iRepeat_first_field;		// RFF flag
		int	display_vertical_size;		// For Display Vertical Size
		int	display_horizontal_size;	// For Display Horizontal Size
		unsigned long framerate1000;
		BYTE   ccbuf[VC1_LINE21BUF_SIZE];     /// cc info       
		DWORD  cclen; 
		int m_downSampleFrame;

		DWORD dwReserved[32];
	}VC1VDecGMO_Frame;
};

#ifdef __cplusplus
//}
#endif

#endif /* _VC1VDec_H_ */