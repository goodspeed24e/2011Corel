/*!
**************************************************************************
* \file defines.h
*
* \brief
*    Headerfile containing some useful global definitions
*
* \author
*    Detlev Marpe  
*    Copyright (C) 2000 HEINRICH HERTZ INSTITUTE All Rights Reserved.
*
* \date
*    21. March 2001
**************************************************************************
*/

#ifndef _DEFINES_H_
#define _DEFINES_H_

#if defined(__linux__) && !defined(__INTEL_COMPILER)
#include <windows.h>  // needed for __forceinline definition
#endif

//#define PAIR_FIELDS_IN_OUTPUT

//#define MAX_NUM_SLICES 150
#define MAX_NUM_SLICES 50

//FREXT Profile IDC definitions
#define FREXT_HP        100      //!< YUV 4:2:0/8 "High"
#define FREXT_Hi10P     110      //!< YUV 4:2:0/10 "High 10"
#define FREXT_Hi422     122      //!< YUV 4:2:2/10 "High 4:2:2"
#define FREXT_Hi444     144      //!< YUV 4:4:4/12 "High 4:4:4"

#define MVC_HIGH		118

#define YUV400 0
#define YUV420 1
#define YUV422 2
#define YUV444 3


// CAVLC
#define LUMA              0
#define LUMA_INTRA16x16DC 1
#define LUMA_INTRA16x16AC 2
#define CHROMA_DC_CAVLC		3
#define CHROMA_AC_CAVLC		4

#define TOTRUN_NUM    15
#define RUNBEFORE_NUM  7


//--- block types for CABAC ----
#define LUMA_16DC       0
#define LUMA_16AC       1
#define LUMA_8x8        2
#define LUMA_8x4        3
#define LUMA_4x8        4
#define LUMA_4x4        5
#define CHROMA_DC       6
#define CHROMA_AC       7
// CHROMA_DC_2x4 and CHROMA_DC_2x4 are only used in YUV422 and YUV444 profiles ?
//#define CHROMA_DC_2x4   8
//#define CHROMA_DC_4x4   9
#define NUM_BLOCK_TYPES 8


#define MAX_CODED_FRAME_SIZE 3133440         //!< bytes for one frame - 1920x1088x1.5 (YUV420)/2(min. compression ratio)*2(collec next picture)

//#define absm(A) ((A)<(0) ? (-(A)):(A))      //!< abs macro, faster than procedure
//#define fast_abs_int(x) x*(1+(((x)>>31)<<1)) //for int only
__forceinline int fast_abs_short(int x)
{
	int sign = (int)(x) >> 31;
	return (x ^ sign) - sign;
}


#define Clip1(a)            ((a)>255?255:((a)<0?0:(a))) // HP restriction
#define Clip1_Chr(a)        ((a)>IMGPAR max_imgpel_value_uv?IMGPAR max_imgpel_value_uv:((a)<0?0:(a)))
#define Clip3(min,max,val) (((val)<(min))?(min):(((val)>(max))?(max):(val)))


//Grant - needs debugging, there are hard-coded mb-types in code
#define P_SKIP		0
#define B_DIRECT	0
#define PB_16x16	1
#define PB_16x8		2
#define PB_8x16		3
#define PB_8x8		4
#define PB_8x4		5
#define PB_4x8		6
#define PB_4x4		7

#define I4MB    0x0008
#define I16MB   0x0010
#define IBLOCK  I4MB
//#define SI4MB   0x0040
#define I8MB    0x0040
#define IPCM	0x0080
#define MAXMODE	0x0100


#define INTRA_MODE				(I4MB|I16MB|IPCM|I8MB)
#define INTRA_NOSI4MB_MODE		(I4MB|I16MB|IPCM|I8MB)
#define NEWINTRA_MODE			(I16MB|IPCM)

#define IS_INTRA(MB)    		((MB)->mb_type&INTRA_MODE)
#define IS_INTRANOSI4MB(MB)    	((MB)->mb_type&INTRA_NOSI4MB_MODE)
#define IS_NEWINTRA(MB) 		((MB)->mb_type&NEWINTRA_MODE)

#define IS_OLDINTRA(MB) 		((MB)->mb_type&I4MB)
#define IS_INTER(MB)    		(!IS_INTRANOSI4MB(MB))
#define IS_INTERMV(MB)  		(IS_INTER(MB) && (MB)->mb_type!=0)


#define IS_DIRECT(MB)   		((MB)->mb_type==0     && (IMGPAR type==B_SLICE ))
#define IS_COPY(MB)     		((MB)->mb_type==0     && (IMGPAR type==P_SLICE ))
#define IS_P8x8(MB)     		((MB)->mb_type==PB_8x8)


// Quantization parameter range

#define MIN_QP          0
#define MAX_QP          51

#define BLOCK_SIZE      4
#define MB_BLOCK_SIZE   16


#define NO_INTRA_PMODE  9        //!< #intra prediction modes
/* 4x4 intra prediction modes */
#define VERT_PRED             0
#define HOR_PRED              1
#define DC_PRED               2
#define DIAG_DOWN_LEFT_PRED   3
#define DIAG_DOWN_RIGHT_PRED  4
#define VERT_RIGHT_PRED       5
#define HOR_DOWN_PRED         6
#define VERT_LEFT_PRED        7
#define HOR_UP_PRED           8

// 16x16 intra prediction modes
#define VERT_PRED_16    0
#define HOR_PRED_16     1
#define DC_PRED_16      2
#define PLANE_16        3

// 8x8 chroma intra prediction modes
#define DC_PRED_8       0
#define HOR_PRED_8      1
#define VERT_PRED_8     2
#define PLANE_8         3

#define EOS             1                       //!< End Of Sequence
#define SOP             2                       //!< Start Of Picture
#define SOS             3                       //!< Start Of Slice

#define DECODING_OK     0
#define SEARCH_SYNC     1

#define MAX_REFERENCE_PICTURES 32               //!< H264 allows 32 fields

#define INVALIDINDEX  (-135792468)

#if !defined(WIN32) || defined(__GNUC__)
#if !defined(max)
#define max(a, b)      ((a) > (b) ? (a) : (b))  //!< Macro returning max value
#endif
#if !defined(min)
#define min(a, b)      ((a) < (b) ? (a) : (b))  //!< Macro returning min value
#endif
#endif


#define MVPRED_MEDIAN   0
#define MVPRED_L        1
#define MVPRED_U        2
#define MVPRED_UR       3

#define DECODE_COPY_MB  0
#define DECODE_MB       1
//#define DECODE_MB_BFRAME 2

#define BLOCK_MULTIPLE      (MB_BLOCK_SIZE/BLOCK_SIZE)

//Start code and Emulation Prevention need this to be defined in identical manner at encoder and decoder
#define ZEROBYTES_SHORTSTARTCODE 2 //indicates the number of zero bytes in the short start-code prefix

#endif

