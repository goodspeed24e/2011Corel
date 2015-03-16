/************************************************************************/
/* Copyright (C) 2005 ATI Technologies Inc. All Rights Reserved.		*/
/* Filename: atih264.h													*/
/* Content : header for ATI DXVA AVC Extension							*/
/************************************************************************/

#ifndef _ATIH264_H_
#define _ATIH264_H_

#include "global.h"

#if defined(_HW_ACCEL_)

#define H264_VERSION 400

DEFINE_GUID(DXVA_ModeH264_ATI_A, // Host-based iTrans
						0x5cd11ee4, 0xdac9, 0x4449, 0xa0, 0x17, 0x44, 0xb, 0x36, 0xc0, 0xde, 0x3d);

// definitions of comp new buffer types, 0~15 are not changed
#define DXVA_SLICE_PARAMETER_BUFFER_264EXT    16
#define DXVA_MOTION_VECTOR_BUFFER_264EXT      17
#define DXVA_DEBLOCKING_CONTROL_BUFFER_264EXT	18
#define DXVA_NUM_TYPES_COMP_BUFFERS_264EXT		19
#define ATI_BA_DATA_BUFFER                    20
#define ATI_BA_DATA_CTRL_BUFFER               21
#define ATI_BA_DECODE_ERROR_BUFFER            22

////////////////////////////
//capability check bits
//spatial support - Note DXVA driver reports only the maximum spatial capability format
#define ATI_H264_1080					0x00000001 //up to 1920x1080p 30 fps or 1920x1080i 60Hz)
#define ATI_H264_720					0x00000002 // up to 1280x720p 30 fps
#define ATI_H264_480					0x00000003 // up to 720x576 25 fps xor 720px480 30 fps
//bitstream type support - DXVA driver reports all the bitstream types supported
#define ATI_H264_PROGRESSIVE  0x00000100
#define ATI_H264_INTERLACED   0x00000200
#define ATI_H264_MBAFF        0x00000400

//decode compliance - DXVA driver reports what is not supported
//progressive decode compliance
#define ATI_H264_NO_PROG_WP				0x00010000
#define ATI_H264_NO_PROG_INTRA8x8		0x00020000
#define ATI_H264_PROG_INTRA_SLICE_APP	0x00040000
//interlaced decode compliance
#define ATI_H264_NO_INTL_WP				0x00100000
#define ATI_H264_INTL_INTRA_SLICE_APP	0x00200000
//mbaff decode compliance
#define ATI_H264_NO_MBAFF_WP			0x04000000
#define ATI_H264_MBAFF_INTRA_SLICE_APP	0x08000000

typedef struct _ATI_DXVA_ConfigPictureDecode 
{	
	// Operation Indicated
	DXVA_ConfigQueryOrReplyFunc dwFunction;
	// Alignment
	DWORD dwReservedBits[3];
	// Encryption GUIDs
	GUID guidConfigBitstreamEncryption;
	GUID guidConfigMBcontrolEncryption;
	GUID guidConfigResidDiffEncryption;
	// Bitstream Processing Indicator
	BYTE bConfigBitstreamRaw;
	// Macroblock Control Config
	BYTE bConfigMBcontrolRasterOrder;
	// Host Resid Diff Config
	BYTE bConfigResidDiffHost;
	BYTE bConfigSpatialResid8;
	BYTE bConfigResid8Subtraction;
	BYTE bConfigSpatialHost8or9Clipping;
	BYTE bConfigSpatialResidInterleaved;
	BYTE bConfigIntraResidUnsigned;
	// Accelerator Resid Diff Config
	BYTE bConfigResidDiffAccelerator;
	BYTE bConfigHostInverseScan;
	BYTE bConfigSpecificIDCT;
	BYTE bConfig4GroupedCoefs;
} ATI_DXVA_ConfigPictureDecode, *LP_ATI_DXVA_ConfigPictureDecode;

/* Picture Parameters */
//bH264PicType defines
#define H264PIC_INTRA   0x01 //Intra macroblocks only
#define H264PIC_P       0x02 //P with no Intra macroblocks
#define H264PIC_PI      0x03 //P with Intra macroblocks
#define H264PIC_B       0x04 //B with no Intra macroblocks
#define H264PIC_BI      0x05 //B with Intra macroblocks
#define H264PIC_UNKNOWN 0x00

/* Picture Decoding Parameters */
typedef struct _DXVA_PictureParameters_H264 
{
	WORD wDecodedPictureIndex;
	WORD wPicWidthInMBminus1;
	WORD wPicHeightInMBminus1;
	BYTE bBPPminus1;
	BYTE bPicStructure;
	BYTE bSecondField;
	BYTE bChromaFormat;
	BYTE bMV_RPS;
	BYTE bWeightedPrediction; // used only in DXVA_RESTRICTED_MODE_H264_AB profile
	BYTE bReference[16];
	WORD wNumberOfReference;
	BYTE bH264PicType;
	BYTE bDisableILD;
} DXVA_PictureParameters_H264, *LPDXVA_PictureParameters_H264;

/* Slice Parameters */
#define MAX_NUMBER_OF_SLICES 32
#define MAX_NUMBER_OF_REFERENCES 16

typedef struct _H264_Slice_Control_Map1 {
	char mQPUOffset; // QP U offset
	char mQPVOffset ; // QP V offset
	char mAlfaCOffset; // Alpha and CO offset
	char mBetaOffset ; //Beta Offset
	char mWeightDenomLuma ; // (1 << weighted denominator luma) or 0
	char mWeightDenomChroma ; // (1 << weighted denominator chroma ) or 0
	char mReserved0; //not used
	char mReserved1; //not used
} H264_Slice_Control_Map1, *pH264_Slice_Control_Map1;

typedef struct _H264_Slice_Control_WOffset {
	char mWeightOffsetY;
	char mWeightOffsetU;
	char mWeightOffsetV;
	char mReserved0; //not used
} H264_Slice_Control_WOffset, *pH264_Slice_Control_WOffset;

typedef struct _H264_Slice_Control_Map2 {
	H264_Slice_Control_WOffset WOffset_L0[MAX_NUMBER_OF_REFERENCES * 2];
	H264_Slice_Control_WOffset WOffset_L1[MAX_NUMBER_OF_REFERENCES * 2];
} H264_Slice_Control_Map2, *pH264_Slice_Control_Map2;

typedef struct _H264_Explicit_Weight_Packet {
	WORD mWeightY;
	WORD mWeightU;
	WORD mWeightV;
	WORD mReserved0; //not used
} H264_Explicit_Weight_Packet, *pH264_Explicit_Weight_Packet;

typedef struct _H264_Explicit_Weight_Map {
	H264_Explicit_Weight_Packet WPacket_L0[MAX_NUMBER_OF_REFERENCES * 2];
	H264_Explicit_Weight_Packet WPacket_L1[MAX_NUMBER_OF_REFERENCES * 2];
} H264_Explicit_Weight_Map, *pH264_Explicit_Weight_Map;

typedef char H264_Implicit_Weight_Map[64][64];

typedef struct _DXVA_SliceParameter_H264 {
	UINT nNumberOfSlices;
	H264_Slice_Control_Map1 SliceCtrlMap1[MAX_NUMBER_OF_SLICES];
	H264_Slice_Control_Map2 SliceCtrlMap2[MAX_NUMBER_OF_SLICES];
	H264_Explicit_Weight_Map ExpWMap[MAX_NUMBER_OF_SLICES];
	H264_Implicit_Weight_Map ImpWMap[MAX_NUMBER_OF_SLICES];
} DXVA_SliceParameter_H264, *LPDXVA_SliceParameter_H264;

/* H.264 Control map */
typedef DWORD	H264_MC_CTRLMAP;
typedef WORD	H264_SB_CTRLMAP;

#define H264_MC_TOP_FIELD_REFERENCE    0x8000
#define H264_VALID_REFERENCE_1         0x0400
#define H264_MC_BOTTOM_FIELD_REFERENCE 0x0080
#define H264_VALID_REFERENCE_0         0x0004
#define H264_FIELD_MB                  0x0002 //if bit 1 equals 1 means interlaced and mbaff MB
#define H264_I8x8_UP_RIGHT_AVAILABLE   0x0200
#define H264_I8x8_UP_LEFT_AVAILABLE    0x0100
#define H264_INTRA_PREDICTION          0x0001 //if bit 0 equals 0 means Inter prediction

//Intra Luma 4x4
typedef enum 
{
	LUMA_VERT_PRED_4 = 0,
	LUMA_HOR_PRED_4,
	LUMA_DC_PRED_4_NONE,
	LUMA_DIAG_DOWN_LEFT_PRED_4,
	LUMA_DIAG_DOWN_RIGHT_PRED_4,
	LUMA_VERT_RIGHT_PRED_4,
	LUMA_HOR_DOWN_PRED_4,
	LUMA_VERT_LEFT_PRED_4,
	LUMA_HOR_UP_PRED_4,
	LUMA_DC_PRED_4_UP_LF,
	LUMA_DC_PRED_4_UP,
	LUMA_DC_PRED_4_LF
} ENUM_INTRA4x4_MODE; // Intra4x4 modes.

//Intra 8x8
// Intra8x8Mode = ENUM_INTRA8x8_MODE + LUMA_MODE_8_OFFSET
typedef enum 
{
	LUMA_VERT_PRED_8 = 16,
	LUMA_HOR_PRED_8,
	LUMA_DC_PRED_8_NONE,
	LUMA_DIAG_DOWN_LEFT_PRED_8,
	LUMA_DIAG_DOWN_RIGHT_PRED_8,
	LUMA_VERT_RIGHT_PRED_8,
	LUMA_HOR_DOWN_PRED_8,
	LUMA_VERT_LEFT_PRED_8,
	LUMA_HOR_UP_PRED_8,
	LUMA_DC_PRED_8_UP_LF,
	LUMA_DC_PRED_8_UP,
	LUMA_DC_PRED_8_LF,
} ENUM_INTRA8x8_MODE; // Intra8x8 modes

//Intra 16x16
// Intra16x16Mode = ENUM_INTRA16x16_MODE + LUMA_MODE_16_OFFSET
typedef enum 
{
	LUMA_VERT_PRED_16 = 32,
	LUMA_HOR_PRED_16,
	LUMA_DC_PRED_16_NONE,
	LUMA_PLANE_16,
	LUMA_DC_PRED_16_UP_LF,
	LUMA_DC_PRED_16_UP,
	LUMA_DC_PRED_16_LF,
	LUMA_DC_PRED_16_IPCM
} ENUM_INTRA16x16_MODE; // Intra16x16 modes

// Chroma Intra Prediction
typedef enum 
{
	CHROMA_DC_PRED_8_NONE = 0,
	CHROMA_HOR_PRED_8,
	CHROMA_VERT_PRED_8,
	CHROMA_PLANE_8,
	CHROMA_DC_PRED_8_UP_LF,
	CHROMA_DC_PRED_8_UP,
	CHROMA_DC_PRED_8_LF,
	CHROMA_8_IPCM
} ENUM_CHROMA_MODE; // Chroma (UV) modes

/* H.264 Motion Vector */
typedef DWORD H264_MVMAP;

/* Deblocking Control */
typedef BYTE H264_DBLK_STRENGHT; //both vertical and horizontal map strength values are bytes.

/* Residual data */
typedef WORD H264_RD;

//Error types
typedef enum {
	ATI_BA_NO_ERROR = 0,
	//tbd: define other error types
} ENUM_BA_DECODE_ERROR_TYPE;

#endif /* defined(_HW_ACCEL_) */

#endif
