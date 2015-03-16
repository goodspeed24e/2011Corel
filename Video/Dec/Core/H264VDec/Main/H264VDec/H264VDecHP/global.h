/*!
************************************************************************
*  \file
*     global.h
*  \brief
*     global definitions for for H.264 decoder.
*  \author
*     Copyright (C) 1999  Telenor Satellite Services,Norway
*                         Ericsson Radio Systems, Sweden
*
*     Inge Lille-Langoy               <inge.lille-langoy@telenor.com>
*
*     Telenor Satellite Services
*     Keysers gt.13                       tel.:   +47 23 13 86 98
*     N-0130 Oslo,Norway                  fax.:   +47 22 77 79 80
*
*     Rickard Sjoberg                 <rickard.sjoberg@era.ericsson.se>
*
*     Ericsson Radio Systems
*     KI/ERA/T/VV
*     164 80 Stockholm, Sweden
*
************************************************************************
*/
#include "ts.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include <stdio.h>                              //!< for FILE
#include <time.h>
#include <sys/timeb.h>
#include "returntype.h"
#include "IH264VDec.h"
#include "Imports/Inc/CPInterfaces.h"
#include "defines.h"
#include "parsetcommon.h"
#include "Utility\utildbg.hpp"

using namespace H264VDecParam;

//Turn on the macro to dump DXVA side Motion Vector and Residue data. Enabled on DXVA1 ATI already
//#define DUMPDXVA_MV
//#define DUMPDXVA_RESIDUE
//#define DUMPDXVA_PICTURE


//#define CONFIG_H264_C_ONLY                 /* no inline asm or intrinsics */
//#define CONFIG_H264_INTRINSICS_ONLY        /* no inline asm */
#define _GLOBAL_IMG_
#define ONE_COF
#define _ARM_FIX_ 1                        /* temporary: fixes for ARM compilation */
#define REC_NEIGHBOR

//#define IP_RD_MERGE				/*I and P slice do read and decode macroblock in same thread*/

#if defined(USE_FOR_WINDVD) || defined(LINUX_H264_MT)

#undef _GLOBAL_IMG_  
#undef ONE_COF
#define _COLLECT_PIC_										/* enables multithreaded decoding*/

#if !defined(__linux__)
#define _HW_ACCEL_                         /* Enable DXVA acceleration */
#define _USE_QUEUE_FOR_DXVA2_
#define _USE_QUEUE_FOR_DXVA1_
#endif

#define _SLEEP_FOR_AUDIO_

#endif

//#define DEBUG_SHOW_PROCESS_TIME
//#define _SHOW_THREAD_TIME_


//#define ANALYZE_CODE_WRITE_DATA_OUT
//#define ANALYZE_CODE_READ_DATA_IN

#if defined(_HW_ACCEL_)
#if !defined(__linux__)
// For DXVA support - needs to be conditionally included
#include <ddraw.h>
#include <dshow.h>
#include <dxva.h>
#include <videoacc.h>
#else
#undef _HW_ACCEL_   // disable DXVA acceleration
#endif /* __linux__ */
#endif /* _HW_ACCEL_ */

#ifdef WIN32
#define  snprintf _snprintf
#define  open     _open
#define  close    _close
#define  read     _read
#define  write    _write
#define  lseek    _lseeki64
#define  fsync    _commit
#define  OPENFLAGS_WRITE _O_WRONLY|_O_CREAT|_O_BINARY|_O_TRUNC
#define  OPEN_PERMISSIONS _S_IREAD | _S_IWRITE
#define  OPENFLAGS_READ  _O_RDONLY|_O_BINARY
#else
#define  OPENFLAGS_WRITE O_WRONLY|O_CREAT|O_TRUNC
#define  OPENFLAGS_READ  O_RDONLY
#define  OPEN_PERMISSIONS S_IRUSR | S_IWUSR
#endif


typedef unsigned char   byte;                   //!<  8 bit unsigned
typedef int             int32;
typedef unsigned int    u_int32;

#define LUMA_BLOCK_SIZE 16
#define LINE21BUF_SIZE 256   /* must be >= 199 */

	// MB Colocated appears to cause a minor reduction in speed at this time
#define CONFIG_PIXELSIZE_BYTE

#if !defined(CONFIG_PIXELSIZE_BYTE)
#define imgpel unsigned short
#else
#define imgpel unsigned char
#endif /* ~CONFIG_PIXELSIZE_BYTE */

#if defined(WIN32) && !defined(__GNUC__)
typedef __int64   int64;
#if !defined(INT64_MIN)
#define INT64_MIN        (-9223372036854775807i64 - 1i64)
#endif
#else
typedef long long  int64;
#if !defined(INT64_MIN)
#define INT64_MIN        (-9223372036854775807LL - 1LL)
#endif
#endif

#ifdef GLOBAL_INSTANCE
#define GL_EXTERN
#else
#define GL_EXTERN extern
#endif


#if !defined(__GNUC__) || defined(__INTEL_COMPILER)
#pragma warning(default:4035)
#define GCC_ALIGN(bytes)
#else
#define GCC_ALIGN(bytes) __attribute__((aligned(bytes)))
#endif
#define DO_ALIGN(bytes,decl) __declspec(align(bytes)) decl GCC_ALIGN(bytes)

// H264_* settings are derived from CONFIG_*
#if   defined(CONFIG_H264_C_ONLY)
#undef  H264_ENABLE_INTRINSICS             /* no intrinsics */
#undef  H264_ENABLE_ASM                    /* no inline asm */
#undef _PRE_TRANSPOSE_
//#define _BIG_ENDIAN_                       /* may be used with CONFIG_H264_C_ONLY */
#define _NO_INT64_                         /* For certain platforms, where int64 is not available and/or has bugs */
#elif defined(CONFIG_H264_INTRINSICS_ONLY)
#define H264_ENABLE_INTRINSICS             /* intrinsics OK */
#undef  H264_ENABLE_ASM                    /* no inline asm */
#define _PRE_TRANSPOSE_
#else
#define H264_ENABLE_INTRINSICS             /* intrinsics OK */
#define H264_ENABLE_ASM                    /* inline asm OK */
#define _PRE_TRANSPOSE_
#endif

// The following flag makes IMG a static (no passing it as an argument to all biari_decode_symbol functions)
// _GLOBAL_IMG_ is NOT thread-safe (Slice-level MT)
#define CODE_ALIGNMENT_PROPERTY 8
#define CONFIG_BIARI_ENABLE_ASM
//#define CONFIG_BIARI_ENABLE_MMX 1


#ifdef _GLOBAL_IMG_

#define ARGS0()                                        ()
#define ARGS1(x1)                                      (x1)
#define ARGS2(x1, x2)                                  (x1, x2)
#define ARGS3(x1, x2, x3)                              (x1, x2, x3)
#define ARGS4(x1, x2, x3, x4)                          (x1, x2, x3, x4)
#define ARGS5(x1, x2, x3, x4, x5)                      (x1, x2, x3, x4, x5)
#define ARGS6(x1, x2, x3, x4, x5, x6)                  (x1, x2, x3, x4, x5, x6)
#define ARGS7(x1, x2, x3, x4, x5, x6, x7)              (x1, x2, x3, x4, x5, x6, x7)
#define ARGS8(x1, x2, x3, x4, x5, x6, x7, x8)          (x1, x2, x3, x4, x5, x6, x7, x8)
#define ARGS9(x1, x2, x3, x4, x5, x6, x7, x8, x9)      (x1, x2, x3, x4, x5, x6, x7, x8, x9)
#define ARGS10(x1, x2, x3, x4, x5, x6, x7, x8, x9, x10) (x1, x2, x3, x4, x5, x6, x7, x8, x9, x10)

#define PARGS0()                                        ()
#define PARGS1(x1)                                      (x1)
#define PARGS2(x1, x2)                                  (x1, x2)
#define PARGS3(x1, x2, x3)                              (x1, x2, x3)
#define PARGS4(x1, x2, x3, x4)                          (x1, x2, x3, x4)
#define PARGS5(x1, x2, x3, x4, x5)                      (x1, x2, x3, x4, x5)
#define PARGS6(x1, x2, x3, x4, x5, x6)                  (x1, x2, x3, x4, x5, x6)
#define PARGS7(x1, x2, x3, x4, x5, x6, x7)              (x1, x2, x3, x4, x5, x6, x7)
#define PARGS8(x1, x2, x3, x4, x5, x6, x7, x8)          (x1, x2, x3, x4, x5, x6, x7, x8)
#define PARGS9(x1, x2, x3, x4, x5, x6, x7, x8, x9)      (x1, x2, x3, x4, x5, x6, x7, x8, x9)
#define PARGS10(x1, x2, x3, x4, x5, x6, x7, x8, x9, x10) (x1, x2, x3, x4, x5, x6, x7, x8, x9, x10)

#else

#define ARGS0()                                        (img)
#define ARGS1(x1)                                      (x1, img)
#define ARGS2(x1, x2)                                  (x1, x2, img)
#define ARGS3(x1, x2, x3)                              (x1, x2, x3, img)
#define ARGS4(x1, x2, x3, x4)                          (x1, x2, x3, x4, img)
#define ARGS5(x1, x2, x3, x4, x5)                      (x1, x2, x3, x4, x5, img)
#define ARGS6(x1, x2, x3, x4, x5, x6)                  (x1, x2, x3, x4, x5, x6, img)
#define ARGS7(x1, x2, x3, x4, x5, x6, x7)              (x1, x2, x3, x4, x5, x6, x7, img)
#define ARGS8(x1, x2, x3, x4, x5, x6, x7, x8)          (x1, x2, x3, x4, x5, x6, x7, x8, img)
#define ARGS9(x1, x2, x3, x4, x5, x6, x7, x8, x9)      (x1, x2, x3, x4, x5, x6, x7, x8, x9, img)
#define ARGS10(x1, x2, x3, x4, x5, x6, x7, x8, x9, x10) (x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, img)

#define PARGS0()                                        (img_par *img)
#define PARGS1(x1)                                      (x1, img_par *img)
#define PARGS2(x1, x2)                                  (x1, x2, img_par *img)
#define PARGS3(x1, x2, x3)                              (x1, x2, x3, img_par *img)
#define PARGS4(x1, x2, x3, x4)                          (x1, x2, x3, x4, img_par *img)
#define PARGS5(x1, x2, x3, x4, x5)                      (x1, x2, x3, x4, x5, img_par *img)
#define PARGS6(x1, x2, x3, x4, x5, x6)                  (x1, x2, x3, x4, x5, x6, img_par *img)
#define PARGS7(x1, x2, x3, x4, x5, x6, x7)              (x1, x2, x3, x4, x5, x6, x7, img_par *img)
#define PARGS8(x1, x2, x3, x4, x5, x6, x7, x8)          (x1, x2, x3, x4, x5, x6, x7, x8, img_par *img)
#define PARGS9(x1, x2, x3, x4, x5, x6, x7, x8, x9)      (x1, x2, x3, x4, x5, x6, x7, x8, x9, img_par *img)
#define PARGS10(x1, x2, x3, x4, x5, x6, x7, x8, x9, x10) (x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, img_par *img)

#endif /* _GLOBAL_IMG_ */


// Microsoft Visual Studio 6 - Debug build bug:
// naked functions have 1 line of prolog that messes up the stack variables...
// Temp (?) fix: use C-version.
// Note that _MSC_VER>=1300 indicated .NET compiler or higher - shouldn't have problems
#if (defined(_DEBUG) && defined(_MSC_VER) && (_MSC_VER<1300)) || \
	(!defined(H264_ENABLE_ASM))
#undef CONFIG_BIARI_ENABLE_ASM
#undef CONFIG_BIARI_ENABLE_MMX
#endif

#ifdef CONFIG_BIARI_ENABLE_ASM
#define biari_decode_final               biari_decode_final_asm
#define biari_decode_symbol              biari_decode_symbol_asm
#define biari_decode_symbol_eq_prob      biari_decode_symbol_eq_prob_asm
#define BARGS0()                         ARGS0()
#define BARGS1(x1)                       ARGS1(x1)
#define P_BARGS0()                       PARGS0()
#define P_BARGS1(x1)                     PARGS1(x1)
#elif defined(CONFIG_BIARI_ENABLE_MMX)
#define store_dep                        store_dep_mmx
#define load_dep                         load_dep_mmx
#define biari_decode_final               biari_decode_final_mmx
#define biari_decode_symbol              biari_decode_symbol_mmx
#define biari_decode_symbol_eq_prob      biari_decode_symbol_eq_prob_mmx
#define BARGS0()                         ()
#define BARGS1(x1)                       (x1)
#define P_BARGS0()                       ()
#define P_BARGS1(x1)                     (x1)
#else /* C version */
#define biari_decode_final               biari_decode_final_c
#define biari_decode_symbol              biari_decode_symbol_c
#define biari_decode_symbol_eq_prob      biari_decode_symbol_eq_prob_c
#define BARGS0()                         ARGS0()
#define BARGS1(x1)                       ARGS1(x1)
#define P_BARGS0()                       PARGS0()
#define P_BARGS1(x1)                     PARGS1(x1)
#endif /* CONFIG_BIARI_ENABLE_ASM or CONFIG_BIARI_ENABLE_MMX */

#define __fast_min(a,b)								((b) + (((int)(a)-(int)(b)) & ((int)(a)-(int)(b))>>31))
#define __fast_max(a,b)								((a) - (((int)(a)-(int)(b)) & ((int)(a)-(int)(b))>>31))
#define __fast_iclip(min,max,val)					((val) + (((int)(max)-(int)(val)) & ((int)(max)-(int)(val))>>31) - (((int)(val)-(int)(min)) & ((int)(val)-(int)(min))>>31))
#define __fast_iclip0_255(val)                      (((val)&(-256)) ? (255& (~(int)(val) >> 31)) : (val))
#define __fast_iclip0_X(x,val)                      ((unsigned)(val) > (x) ? ((x)&(~(int)(val)>>31)) : (val))
#define __fast_iclipX_X(x,val)                      ((unsigned)((val) + (x)) > (unsigned)((x) + (x)) ? ((((x)+(x))&(~(int)((val) + (x))>>31))-(x)) : (val)) 

#define __MB_POS_TABLE_LOOKUP__
//#define __SUPPORT_YUV400__  //define it to support YUV400 format

//for detecting  the type of CPU (supports MMX, SSE, SSE2 and SSE3)
typedef enum
{
	CPU_LEVEL_NONE,				//supports nothing
	CPU_LEVEL_MMX,
	CPU_LEVEL_SSE,
	CPU_LEVEL_SSE2,
	CPU_LEVEL_SSE3
} CPU_LEVEL;
GL_EXTERN CPU_LEVEL cpu_type;


#ifdef _HW_ACCEL_

class DXVA_ENCRYPTBase;

typedef struct _DrawSetParametersInfo
{
	int width;
	int height;
	int frame_rate_code;
	int MPEG2_Flag;
	int aspect_ratio_information;
	TiviDxvaP pAccel;
	DXVA_ENCRYPTBase*	 pEncrypt;
} DrawSetParametersInfo;

#endif

#if defined(_HW_ACCEL_)
	GL_EXTERN int      HW_BUF_SIZE;
#endif

	// For clipping (x,y) coordinates for M.C.
	//GL_EXTERN const int clip_min_x    = (-18);	// 6-tap filter, needs 2 left and 3 to the right
#define clip_min_x (-18)
	//GL_EXTERN const int clip_min_y    = (-18);
#define clip_min_y (-18)
	//GL_EXTERN const int clip_min_x_cr = (-8);		// Bilinear filter, needs 1 to the right
#define clip_min_x_cr (-8)
	//GL_EXTERN const int clip_min_y_cr = (-8);
#define clip_min_y_cr (-8)

#define LUMA_BLOCK_SIZE	16
#define MAX_LIST_SIZE 65
#define MAX_LIST_SIZE_FRAME 33

//! IoK: Introduced to track HD-profile violations
typedef enum
{
	HD_ProfileFault_FALSE = 0,                                //!< no fault
	HD_ProfileFault_SPS_chroma_format_idc,                    //!< should be [0..1]
	HD_ProfileFault_SPS_bit_depth_luma_minus8,                //!< should be 0
	HD_ProfileFault_SPS_bit_depth_chroma_minus8,              //!< should be 0
	HD_ProfileFault_SPS_qpprime_y_zero_transform_bypass_flag, //!< should be 0
	HD_ProfileFault_PPS_num_slice_groups_minus1,              //!< should be 0
	HD_ProfileFault_PPS_redundant_pic_cnt_present_flag,       //!< should be 0
	HD_ProfileFault_NALU_nal_unit_type,                       //!< should NOT be [2..4]
	HD_ProfileFault_SLICE_first_mb_in_slice,                  //!< should be strictly increasing in each picture (no ASO)
	HD_ProfileFault_SLICE_slice_type,                         //!< should be [0..2] (I/P/B)
	HD_ProfileFault_SPS_direct_8x8_stream_violation,          //!< general (not only HD) violation of direct_8x8_inference_flag
} HDProfileFault_STATUS;

#if defined(__linux__) && defined(__INTEL_COMPILER)
#define _asm asm
#endif

#if defined(H264_ENABLE_ASM)
#if !defined(__linux__) || defined(__INTEL_COMPILER)
#pragma warning(disable:4035)
__forceinline unsigned int __cdecl _bswap(unsigned int x)
{
	_asm
	{
		mov eax, x;
		bswap eax;
	}
}

__forceinline unsigned int __cdecl _bsr(int x)
{
	_asm
	{
		bsr eax, x;
	}
}

__forceinline unsigned int __cdecl _shld(int x, int y, int bits)
{
	_asm
	{
		mov ecx, bits;
		mov eax, x;
		mov edx, y;
		shld eax, edx, cl;
	}
}
#else
	/* x86 gcc asm version */
#define _bswap(x) ({ register unsigned int __v, __x = (x); \
	__asm__ ("bswap %0" : "=r" (__v) : "0" (__x)); \
	__v; })

#define _bsr(x) ({ register unsigned int __v, __x = (x);\
	__asm__ ("bsr %1, %0" : "=r" (__v) : "r" (__x)); \
	__v; } ) 

#define _shld(x,y,bits) ({ register int __x =(x); \
	__asm__ ("shld %%cl, %%edx, %%eax" : "=a" (__x)  : \
	"c" (bits), "a" (__x), "d" (y)); \
	__x; })
#endif
#else
	/* c only version */
#define _bswap(x) ((((x) & 0xff000000) >> 24) | \
	(((x) & 0x00ff0000) >>  8) | \
	(((x) & 0x0000ff00) <<  8) | \
	(((x) & 0x000000ff) << 24))

#define _shld(x,y,bits)  (((x)<<(bits)) + ((y)>>(32-(bits))))

#endif

/***********************************************************************
* T y p e    d e f i n i t i o n s    f o r    T M L
***********************************************************************
*/

//! Data Partitioning Modes
typedef enum
{
	PAR_DP_1,    //!< no data partitioning is supported
	PAR_DP_3,    //!< data partitioning with 3 partitions
} PAR_DP_TYPE;


//! Output File Types
typedef enum
{
	PAR_OF_ANNEXB,   //!< Current TML description
#ifdef _SUPPORT_RTP_
	PAR_OF_RTP,   //!< RTP Packet Output format
#endif /* _SUPPORT_RTP_ */
	//  PAR_OF_IFF    //!< Interim File Format
} PAR_OF_TYPE;

//! definition of H.264 syntax elements
typedef enum {
	SE_HEADER,
	SE_PTYPE,
	SE_MBTYPE,
	SE_REFFRAME,
	SE_INTRAPREDMODE,
	SE_MVD,
	SE_CBP_INTRA,
	SE_LUM_DC_INTRA,
	SE_CHR_DC_INTRA,
	SE_LUM_AC_INTRA,
	SE_CHR_AC_INTRA,
	SE_CBP_INTER,
	SE_LUM_DC_INTER,
	SE_CHR_DC_INTER,
	SE_LUM_AC_INTER,
	SE_CHR_AC_INTER,
	SE_DELTA_QUANT_INTER,
	SE_DELTA_QUANT_INTRA,
	SE_BFRAME,
	SE_EOS,
	SE_TRANSFORM_SIZE_FLAG,
	SE_MAX_ELEMENTS //!< number of maximum syntax elements, this MUST be the last one!
} SE_type;        // substituting the definitions in element.h


typedef enum {
	INTER_MB,
	INTRA_MB_4x4,
	INTRA_MB_16x16
} IntraInterDecision;

typedef enum {
	BITS_TOTAL_MB,
	BITS_HEADER_MB,
	BITS_INTER_MB,
	BITS_CBP_MB,
	BITS_COEFF_Y_MB,
	BITS_COEFF_UV_MB,
	MAX_BITCOUNTER_MB
} BitCountType;

typedef enum {
	SM_NO_SLICES,
	SM_FIXED_MB,
	SM_FIXED_RATE,
	SM_CALLBACK,
	SM_FMO
} SliceMode;


typedef enum {
	UVLC,
	CABAC
} SymbolMode;

typedef enum {
	LIST_0=0,
	LIST_1=1
} Lists;


typedef enum {
	FRAME,
	TOP_FIELD,
	BOTTOM_FIELD
} PictureStructure;           //!< New enum for field processing


typedef enum {
	P_SLICE = 0,
	B_SLICE,
	I_SLICE,
	SP_SLICE,
	SI_SLICE,	
	IDR_SLICE,		// IDR
	I_GOP,			// GOP access unit
	APP_SLICE,		// additional frame type for randam access, IDR and AP P
	I_APR,			// reference I of AP P
	P_APR,			// reference P of AP P
	RB_SLICE,			// reference B
} SliceType;


enum SMART_DEC
{
	SMART_DEC_DISABLE = 0,
	SMART_DEC_NODEBLK_BND = 1,		// no deblocking each MB boundary
	SMART_DEC_NODEBLK_B   = 1 << 1, // no deblocking B-frame
	SMART_DEC_NODEBLK_ALL = 1 << 2, // no deblocking all frames
	SMART_DEC_SKIP_FIL_B  = 1 << 3, // skip the 2nd B-field 
	SMART_DEC_SKIP_HW_CR  = 1 << 4,	// skip the 2nd B-field and chroma part for hardware only
	SMART_DEC_SKIP_1B     = 1 << 5, // skip one B-frame between I- or P-frames
	SMART_DEC_SKIP_PB     = 1 << 6, // skip all P- and B-frame
	SMART_DEC_ONLY_IDR    = 1 << 7, // only decode IDR frame
	SMART_DEC_INT_PEL_Y   = 1 << 8, // Skip quarter pixel MC only for Luma
	SMART_DEC_INT_PEL_UV  = 1 << 9, // Skip quarter pixel MC only for Chroma
	SMART_DEC_SKIP_2B     = 1 << 10,// skip two B-frames between I- or P-frames
	SMART_DEC_BITBYBIT    = 1 << 11, //To omit multi-thread data race in single slice B-frame picture. 
	//Might impact the motion compensated value at bottom rows of B-picture. But the difference is minor
	SMART_DEC_END
};

#define SMART_DEC_LEVEL_0	SMART_DEC_BITBYBIT
#define SMART_DEC_LEVEL_1	(SMART_DEC_NODEBLK_B | SMART_DEC_NODEBLK_ALL)
#define SMART_DEC_LEVEL_2	(SMART_DEC_LEVEL_1 | SMART_DEC_SKIP_FIL_B | SMART_DEC_INT_PEL_UV)
#define SMART_DEC_LEVEL_3	(SMART_DEC_LEVEL_2 | SMART_DEC_INT_PEL_Y)
#define SMART_DEC_LEVEL_4	(SMART_DEC_LEVEL_3 | SMART_DEC_SKIP_1B)
#define SMART_DEC_LEVEL_5	(SMART_DEC_LEVEL_4 | SMART_DEC_SKIP_2B)
#define SMART_DEC_LEVEL_6	(SMART_DEC_LEVEL_1 | SMART_DEC_ONLY_IDR)	// Terry: This level is only for sub picture  (PIP).



//Terry: The first four frames should not be skipped for computing the correct pull-down rate.
//           No skipping the 6th frame, it is used to reduce the gap of 2B pull-down case.
#define NO_SKIP_FRAME_FOR_PULLDOWN	6

/***********************************************************************
* D a t a    t y p e s   f o r  C A B A C
***********************************************************************
*/

//! DecodingEnvironment
//! struct to characterize the state of the decoding engine
typedef __declspec(align(32)) struct decodingenvironment
{
	int Drange;
	int Ddelta;
	int Dbits_to_go;
	unsigned int Dbuffer;
	unsigned char *Dcodestrm;
	unsigned char *Dbasestrm;
	int Dstrmlength;
	int Dei_flag;
} DecodingEnvironment GCC_ALIGN(32);

//! struct for context management
#ifdef _ARM_FIX_
typedef unsigned char BiContextType; // index into state-table CP
#else
typedef struct
{
	unsigned char state;         // index into state-table CP  
} BiContextType;
#endif

typedef BiContextType *BiContextTypePtr;


/**********************************************************************
* C O N T E X T S   F O R   T M L   S Y N T A X   E L E M E N T S
**********************************************************************
*/

#define NUM_MB_TYPE_CTX  11
#define NUM_B8_TYPE_CTX  9
#define NUM_MV_RES_CTX   10
#define NUM_REF_NO_CTX   6
#define NUM_DELTA_QP_CTX 4
#define NUM_MB_AFF_CTX 4
#define NUM_TRANSFORM_SIZE_CTX 3


typedef __declspec(align(16)) struct
{
	BiContextType mb_type_contexts [4][NUM_MB_TYPE_CTX];
	BiContextType b8_type_contexts [2][NUM_B8_TYPE_CTX];
	BiContextType mv_res_contexts  [2][NUM_MV_RES_CTX];
	BiContextType ref_no_contexts  [2][NUM_REF_NO_CTX];
	BiContextType delta_qp_contexts[NUM_DELTA_QP_CTX];
	BiContextType mb_aff_contexts  [NUM_MB_AFF_CTX];
	BiContextType transform_size_contexts [NUM_TRANSFORM_SIZE_CTX];

} GCC_ALIGN(16) MotionInfoContexts;

#define NUM_IPR_CTX    2
#define NUM_CIPR_CTX   4
#define NUM_CBP_CTX    4
#define NUM_BCBP_CTX   4
#define NUM_MAP_CTX   15
#define NUM_LAST_CTX  15
#define NUM_ONE_CTX    5
#define NUM_ABS_CTX    5

#define MAP_LAST_CTX_FRAME_FIELD_DIFF (NUM_BLOCK_TYPES*(NUM_MAP_CTX+NUM_LAST_CTX+NUM_ONE_CTX+NUM_ABS_CTX))

typedef __declspec(align(16)) struct
{
	BiContextType ipr_contexts [NUM_IPR_CTX];
	BiContextType cipr_contexts[NUM_CIPR_CTX]; 
	BiContextType cbp_contexts [3][NUM_CBP_CTX];
	BiContextType bcbp_contexts[NUM_BLOCK_TYPES][NUM_BCBP_CTX];
	BiContextType map_contexts [NUM_BLOCK_TYPES][NUM_MAP_CTX];
	BiContextType last_contexts[NUM_BLOCK_TYPES][NUM_LAST_CTX];
	BiContextType one_contexts [NUM_BLOCK_TYPES][NUM_ONE_CTX];
	BiContextType abs_contexts [NUM_BLOCK_TYPES][NUM_ABS_CTX];
	BiContextType fld_map_contexts [NUM_BLOCK_TYPES][NUM_MAP_CTX];
	BiContextType fld_last_contexts[NUM_BLOCK_TYPES][NUM_LAST_CTX];
} GCC_ALIGN(16) TextureInfoContexts;


//*********************** end of data type definition for CABAC *******************

/***********************************************************************
* N e w   D a t a    t y p e s   f o r    T M L
***********************************************************************
*/

struct img_par;
struct nalu_type;
struct decoded_picture_buffer;

//KevinChien
#ifdef _COLLECT_PIC_
struct stream_par;
#endif
//~KevinChien

typedef struct old_slice_par
{
	unsigned field_pic_flag;
	unsigned bottom_field_flag;
	unsigned frame_num;
	int nal_ref_idc;
	unsigned pic_oder_cnt_lsb;
	int delta_pic_oder_cnt_bottom;
	int delta_pic_order_cnt[2];
	int idr_flag;
	int idr_pic_id;
	int pps_id;
} OldSliceParams;


/*! Buffer structure for decoded referenc picture marking commands */
typedef struct DecRefPicMarking_s
{
	int memory_management_control_operation;
	int difference_of_pic_nums_minus1;
	int long_term_pic_num;
	int long_term_frame_idx;
	int max_long_term_frame_idx_plus1;
	struct DecRefPicMarking_s *Next;
} DecRefPicMarking_t;

typedef union
{
	struct {
		short x;
		short y;
	};
	unsigned int mv_comb;
} MotionVector;

typedef CREL_RETURN (read0_tt) PARGS0();
typedef int  (read1_tt) PARGS0();
typedef int  (read2_tt) PARGS2(int list, MotionVector *mvd);

typedef read0_tt (*read0_t);
typedef read1_tt (*read1_t);
typedef read2_tt (*read2_t);

//structure to hold all symbol reading functions
typedef struct read_functions_t
{
	read0_t read_ipred_modes;

	read1_t raw_ipred4x4_mode_luma;
	read1_t raw_ipred4x4_mode_chroma;

	read2_t readMVD;

} read_functions_t;

//! Macroblock
typedef __declspec(align(32)) struct macroblock
{
	/*
	#if !defined(_GLOBAL_IMG_)
	// CABAC stuff
	unsigned char m_coeff_pos[64];
	short         m_coeff_level[64];
	#endif
	*/
	// some storage of macroblock syntax elements for global access  
	union 
	{
		byte          ipredmode[4*4];
		MotionVector    upmvd[2][BLOCK_MULTIPLE];      //!< indices correspond to [forw,backw][block_y][block_x].[x,y]
	};

	char   mbStatusA, mbStatusB, mbStatusC, mbStatusD;

	unsigned int  cbp_bits;
	int           cbp;
	byte          b8mode[4];
	char          b8pdir[4];  

	short         mb_type;  
	char          i16mode;
	char          c_ipred_mode;       //!< chroma intra prediction mode
	char          mb_field;
	char          ei_flag;
	char          skip_flag;

	char          luma_transform_size_8x8_flag;
	char          NoMbPartLessThan8x8Flag;


	byte			intra_block;

	byte			mbIntraA;	//this is the flag for neighbor A intra information when constrained_intra_pred_flag==1, using bit 0 and 1

} GCC_ALIGN(32) Macroblock;

typedef __declspec(align(16)) struct pix_pos
{
	Macroblock* pMB;
	int mb_addr;
	int x;
	int y;
} GCC_ALIGN(16) PixelPos;


typedef struct pred_storage_info
{
	MotionVector  mv[2][4*4];
	char			ref_idx[2][4];
	char	 		ref_pic_id[2][4];
} Pred_s_info;

typedef __declspec(align(16)) struct macroblock_storage
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

	Pred_s_info   pred_info;

} GCC_ALIGN(16) Macroblock_s;

typedef struct colocated_params_MB
{	
	Pred_s_info   pred_info;
	byte		  moving_block[4][4];
	//int           size_x, size_y;
	byte          is_long_term;
} ColocatedParamsMB;

static const int b8_idx[4] = {0,2,8,10};

static const int RSD[4] = {0,0,3,3};

static const int l_16_4[] = {
	0, 0, 1, 1,
	0, 0, 1, 1,
	2, 2, 3, 3,
	2, 2, 3, 3
};

/*
const int b44_idx[4][4] = { {0,1,4,5} , {2,3,6,7} , {8,9,12,13} , {10,11,14,15} };
const int b44_offset[4][4] = { {0,16,64,80} , {32,48,96,112} , {128,144,192,208} , {160,176,224,240} };
const int b84_idx[4][2] = { {0,4} , {2,6} , {8,12} , {10,14} };
const int b48_idx[4][2] = { {0,1} , {2,3} , {8,9} , {10,11} };
*/


//! Slice
typedef struct
{  
	int                 qp;
	int                 slice_qp_delta;
	int                 picture_type;  //!< picture type
	PictureStructure    structure;     //!< Identify picture structure type
	int                 start_mb_nr;   //!< MUST be set by NAL even in case of ei_flag == 1
	int                 max_part_nr;
	int                 dp_mode;       //!< data partioning mode
	int                 next_header;
	//  int                 last_mb_nr;    //!< only valid when entropy coding == CABAC
	DecodingEnvironment *partArr;      //!< array of decoding environments
	MotionInfoContexts  *mot_ctx;      //!< pointer to struct of context models for use in CABAC
	TextureInfoContexts *tex_ctx;      //!< pointer to struct of context models for use in CABAC

	int                 ref_pic_list_reordering_flag_l0;
	int                 *remapping_of_pic_nums_idc_l0;
	int                 *abs_diff_pic_num_minus1_l0;
	int                 *long_term_pic_idx_l0;
	int					*abs_diff_view_idx_minus1_l0;
	int                 ref_pic_list_reordering_flag_l1;
	int                 *remapping_of_pic_nums_idc_l1;
	int                 *abs_diff_pic_num_minus1_l1;
	int                 *long_term_pic_idx_l1;
	int					*abs_diff_view_idx_minus1_l1;

	int                (*readSlice)();

	char                 ei_flag;       //!< 0 if the partArr[0] contains valid information	
	char                 LFDisableIdc;     //!< Disable loop filter on slice
	char                 LFAlphaC0Offset;  //!< Alpha and C0 offset for filtering slice
	char                 LFBetaOffset;     //!< Beta offset for filtering slice

	int                 pic_parameter_set_id;   //!<the ID of the picture parameter set the slice is reffering to
	read_functions_t    readSyntax;

#ifdef _COLLECT_PIC_
	void					*nextSlice;
	int					idr_flag;
	int					nal_reference_idc;
	int					disposable_flag;
	DecodingEnvironment	                *g_dep;  
	int					header;
	int					Transform8x8Mode;
	int					frame_num;
	int					pre_frame_num;  
	unsigned int			        field_pic_flag;  
	unsigned int			        bottom_field_flag;
	int					m_pic_combine_status;  
	int					pic_oder_cnt_lsb;
	byte					MbaffFrameFlag;
	int					idr_pic_id;
	unsigned				pic_order_cnt_lsb;
	int					delta_pic_order_cnt_bottom;
	int					delta_pic_order_cnt[3];
	byte					direct_spatial_mv_pred_flag;
	int					num_ref_idx_l0_active;
	int					num_ref_idx_l1_active;
	unsigned	int			apply_weights;
	int					exit_flag;
	//pic_parameter_set_rbsp_t *m_active_pps;
	H264_TS				pts;
	H264_TS				dts;
	unsigned long				framerate1000;
	int					m_mb_nr_offset; 
	int					PreviousPOC, ThisPOC;
	int					read_mb_nr;
	int					has_pts;
	int					NumClockTs;
#endif

	//weighted prediction
	unsigned int luma_log2_weight_denom;
	unsigned int chroma_log2_weight_denom;
	int *wp_weight;  // weight in [list][index][component] order
	int *wp_offset;  // offset in [list][index][component] order
	int *wbp_weight; //weight in [list][fw_index][bw_index][component] order
	int wp_round_luma;
	int wp_round_chroma;

	int AU_type;
	int m_nDispPicStructure;

	//MVC Extension
	unsigned int viewId;
	unsigned int viewIndex;
	unsigned int temporalId;
	unsigned int anchorPicFlag;
	unsigned int interViewFlag;
	unsigned int priorityId;
	BOOL bIsBaseView;

} Slice;

typedef struct 
{
	char nz_coeff_num[24]; //4*(4+IMGPAR num_blk8x8_uv)
} NZCoeff;

//****************************** ~DM ***********************************
typedef void (MB_itrans_Luma_t) PARGS2(imgpel * imgY, int stride);

typedef struct storable_picture StorablePicture;

//! Frame Stores for Decoded Picture Buffer
typedef struct frame_store
{
	int       is_used;                //!< 0=empty; 1=top; 2=bottom; 3=both fields (or frame)
	int       is_reference;           //!< 0=not used for ref; 1=top used; 2=bottom used; 3=both fields (or frame) used
	int       is_long_term;           //!< 0=not used for ref; 1=top used; 2=bottom used; 3=both fields (or frame) used
	int       is_orig_reference;      //!< original marking by nal_ref_idc: 0=not used for ref; 1=top used; 2=bottom used; 3=both fields (or frame) used

	int       is_non_existent;

	unsigned  frame_num;
	int       frame_num_wrap;
	int       long_term_frame_idx;
	int       is_output;
	int       poc;

	StorablePicture *frame;
	StorablePicture *top_field;
	StorablePicture *bottom_field;

} FrameStore;


//! Decoded Picture Buffer
typedef struct decoded_picture_buffer
{
	FrameStore  ***fs_on_view;
	FrameStore  ***fs_ref_on_view;
	FrameStore  ***fs_ltref_on_view;
	unsigned      *size_on_view;
	unsigned int		*used_size_on_view;
	unsigned      *ref_frames_in_buffer_on_view;
	unsigned      *ltref_frames_in_buffer_on_view;
	int           last_output_poc;
	int           *max_long_term_pic_idx_on_view;

	int           *init_done;
	int           num_ref_frames;

	FrameStore   **last_picture;
} DecodedPictureBuffer;

typedef struct AnnexbNALUDecodingState_
{
	BOOL bStartCodeFound;
	char *buf;				// should be free on EndAnnexbNALUDecoding().
	unsigned int buf_size;	// current buffer size.
	int pos;				// next buffer position.
	int begin;			// start index of acutal NALU data
}AnnexbNALUDecodingState;

typedef void mb_chroma_WxH_t PARGS8( StorablePicture *p,
																		imgpel *pDstUV,
																		int stride_dst,
																		int base_x_pos,
																		int base_y_pos,
																		int mv_x,
																		int mv_y,
																		int H );

typedef void get_block_WxH_t PARGS7(StorablePicture *p, int base_x_pos, int base_y_pos, int mv_x, int mv_y, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int height);
typedef void Arrange_Residual_qp_t PARGS5(int qp_per, const byte *scan_ptr, const short *Inv_table, int i, int j);
typedef int decode_picture_t PARGS0();

class CH264VideoFrameMgr;
class CATSCDec;

#ifdef __MB_POS_TABLE_LOOKUP__
typedef struct __MB_POS_TABLE__
{
	unsigned char x;
	unsigned char y;
}MB_POS_TABLE;
#endif

#define ASMCALL __cdecl

#include "parsetcommon.h"

typedef struct h264_cc 
{
	unsigned char ccbuf[LINE21BUF_SIZE];
	unsigned int cclen;
	unsigned int ccnum;
}H264_CC;

#if defined(_COLLECT_PIC_)
typedef  struct stream_par *p_stream_par;
#endif

typedef struct _SECOP_SliceInfo
{
	DWORD                      dwSliceByteCount;
	DWORD                      dwSliceHandle;
} SECOP_SliceInfo;

typedef struct _h264_offset_metadata_control
{
	DWORD dwCookie;
	H264VDecHP_Offset_Metadata meta_data;
}H264_Offset_Metadata_Control;


// image parameters
typedef __declspec(align(32)) struct img_par
{
	DecodingEnvironment g_dep;
	Macroblock mbpair[4];

	Macroblock *mb_decdata;
	Macroblock *pMBpair_left;
	Macroblock *pMBpair_current;
	Macroblock *pLeftMB_r;

	Macroblock *pUpMB_r;
	Macroblock *pUpLeftMB_r;
	Macroblock *pUpRightMB_r;
	Macroblock *pLastMB_r;

	Macroblock *pLeftMB_temp;
	Macroblock *pUpMB_temp;

	short  mbAddrA, mbAddrB, mbAddrC, mbAddrD;

	int array_index;

	int number;                                 //!< frame number
	short current_mb_nr_r;                        // bitstream order
	short current_mb_nr_d;                        // bitstream order
	short num_dec_mb;
	short current_slice_nr;  
	short qp;                                     //!< quant for the current frame
	short type;                                   //!< image type INTER/INTRA
	short error_mb_nr;							//The first macroblock which error detected

	short width;
	short height;
	short width_cr;                               //!< width chroma
	short height_cr;                              //!< height chroma
	short mb_y_r;
	short mb_x_r;
	short mb_y_d;
	short mb_x_d;

	short mb_left_x_r_FMO;

	short block_y_d;
	short block_x_d;
	short pix_y_d;
	short pix_x_d;
	short pix_c_y_d;
	short pix_c_x_d;
	int	start_mb_x;    //For ATI DXVA data transfer, in first 2 rows of slice, start_mb_x = start_mb_nr % (PicWidthinMB *2)

	byte  allrefzero;
	byte  MbaffFrameFlag;
	byte	direct_spatial_mv_pred_flag;            //!< 1 for Spatial Direct, 0 for Temporal
	byte  mb_up_factor;

	byte  mvd_pairs_mask;

	__declspec(align(16)) imgpel mpr[16][16] GCC_ALIGN(16);                         //!< predicted block
	imgpel mprUV[16][16];  

	MotionVector	curr_mvd[2][BLOCK_MULTIPLE*BLOCK_MULTIPLE];
	MotionVector  left_mvd[4][2][BLOCK_MULTIPLE];

#if defined(ONE_COF)
	short cof[6][4][4][4];                       //!< correction coefficients from predicted   // HP restriction
#else
	unsigned char m_coeff_pos[64];
	short         m_coeff_level[64];

	short  *cof_array;  
	short  *cof_r;
	short  *cof_d;
#endif

	short pix_y_rows;
	short pix_c_y_rows;

	int mvscale[6][MAX_REFERENCE_PICTURES];
	PixelPos left[4];

	imgpel *m_imgY;
	imgpel *m_imgUV;

	int cod_counter;                            //!< Current count of number of skipped macroblocks in a row
	int structure;                               //!< Identify picture structure type

	NZCoeff *nz_coeff;
	Slice       *currentSlice;                   //!< pointer to current Slice data struct
	PAR_OF_TYPE FileFormat;
	DecRefPicMarking_t *dec_ref_pic_marking_buffer;                    //!< stores the memory management control operations

	int subblock_x;
	int subblock_y;
	int is_intra_block;
	int is_v_block;

	int disposable_flag;                          //!< flag for disposable frame, 1:disposable
	int num_ref_idx_l0_active;             //!< number of forward reference
	int num_ref_idx_l1_active;             //!< number of backward reference
	int slice_group_change_cycle;
	// JVT-D101

	unsigned int pre_frame_num;           //!< store the frame_num in the last decoded slice. For detecting gap in frame_num.

	// End JVT-D101
	// POC200301: from unsigned int to int
	int toppoc;      //poc for this top field // POC200301
	int bottompoc;   //poc of bottom field of frame
	int framepoc;    //poc of this frame // POC200301
	unsigned int frame_num;   //frame_num for this frame
	unsigned int field_pic_flag;
	unsigned int bottom_field_flag;

	//the following is for slice header syntax elements of poc
	// for poc mode 0.
	unsigned int pic_order_cnt_lsb;
	int delta_pic_order_cnt_bottom;
	// for poc mode 1.
	int delta_pic_order_cnt[3];

	// ////////////////////////
	// for POC mode 0:
	signed int PrevPicOrderCntMsb;
	unsigned int PrevPicOrderCntLsb;
	signed int PicOrderCntMsb;

	// for POC mode 1:
	unsigned int AbsFrameNum;
	signed int ExpectedPicOrderCnt, PicOrderCntCycleCnt, FrameNumInPicOrderCntCycle;
	unsigned int PreviousFrameNum, FrameNumOffset;
	int ExpectedDeltaPerPicOrderCntCycle;
	int PreviousPOC, ThisPOC;
	int PreviousFrameNumOffset;
	// /////////////////////////

	//weighted prediction
	unsigned int luma_log2_weight_denom;
	unsigned int chroma_log2_weight_denom;
	int *wp_weight;  // weight in [list][index][component] order
	int *wp_offset;  // offset in [list][index][component] order
	int *wbp_weight; //weight in [list][fw_index][bw_index][component] order
	int wp_round_luma;
	int wp_round_chroma;
	unsigned int apply_weights;

	int idr_flag;
	int nal_reference_idc;                       //!< nal_reference_idc from NAL unit

	int idr_pic_id;

	int MaxFrameNum;

	short PicWidthInMbs;
	short PicHeightInMapUnits;
	short FrameHeightInMbs;
	short PicHeightInMbs;
	short PicSizeInMbs;
	short FrameSizeInMbs;

	int no_output_of_prior_pics_flag;
	int long_term_reference_flag;
	int adaptive_ref_pic_buffering_flag;

	int last_has_mmco_5;
	int last_pic_bottom_field;

	int model_number;

	// Fidelity Range Extensions Stuff
	int pic_unit_bitsize_on_disk;	// HP: 1
	int bitdepth_luma;			// HP: 8
	int bitdepth_chroma;			// HP: (YUV400=0, YUV420=8)
	int bitdepth_luma_qp_scale;	// HP: 0
	int bitdepth_chroma_qp_scale;	// HP: 0
	unsigned int dc_pred_value;   // HP: 128    //!< value for DC prediction (depends on pel bit depth)
	int max_imgpel_value;         // HP: 255    //!< max value that one luma picture element (pixel) can take (depends on pic_unit_bitdepth)
	int max_imgpel_value_uv;      // HP: (YUV400=0, YUV420=255)  //!< max value that one chroma picture element (pixel) can take (depends on pic_unit_bitdepth)
	int Transform8x8Mode;        
	int profile_idc;              // HP: 77 or 100
	int yuv_format;				// HP: (YUV400=0, YUV420=1)
	int lossless_qpprime_flag;	// HP: 0
	int num_blk8x8_uv;			// HP: (YUV400=0, YUV420=2)
	int num_cdc_coeff;			// HP: (YUV400=0, YUV420=4)
	int mb_cr_size_x;				// HP: (YUV400=0, YUV420=8)
	int mb_cr_size_y;				// HP: (YUV400=0, YUV420=8)

	// Residue Color Transform
	int residue_transform_flag;

	CREL_RETURN (*FP_decode_one_macroblock_I) PARGS0();
	CREL_RETURN (*FP_decode_one_macroblock_P) PARGS0();
	CREL_RETURN (*FP_decode_one_macroblock_B) PARGS0();
	CREL_RETURN (*FP_decode_one_macroblock_P_Intra) PARGS0();
	CREL_RETURN (*FP_decode_one_macroblock_B_Intra) PARGS0();

	int (*FP_StoreImgRowToImgPic) PARGS2(int start_x, int end_x);
	int (*FP_TransferData_at_SliceEnd) PARGS0();

	void (*FP_ReadMV_16x16) PARGS2(Pred_s_info *info, int list);
	void (*FP_ReadMV_16x8) PARGS2(Pred_s_info *info, int list);
	void (*FP_ReadMV_8x16) PARGS2(Pred_s_info *info, int list);
	void (*FP_ReadMV_8x8) PARGS2(Pred_s_info *info, int list);
	CREL_RETURN (*FP_ReadMV_Direct_Spatial) PARGS3(int current_mb_nr, Macroblock *currMB, Macroblock_s *currMB_s);
	CREL_RETURN (*FP_ReadMV_Direct_Temproal) PARGS3(int current_mb_nr, Macroblock *currMB, Macroblock_s *currMB_s);  

#if (!defined(__linux__)) && defined(_HW_ACCEL_)
	void* (*FP_Open) PARGS4(int nframe, int iVGAType, int iDXVAMode, ICPService *m_pIviCP);
#else
	void* (*FP_Open) PARGS3(int nframe, int iVGAType, int iDXVAMode);
#endif

	CREL_RETURN (*FP_BeginDecodeFrame) PARGS0();
	CREL_RETURN (*FP_EndDecodeFrame) PARGS0();
	CREL_RETURN (*FP_ReleaseDecodeFrame) PARGS1(int frame_index);
	CREL_RETURN (*FP_Close) PARGS0();
	void (*FP_DeblockSlice) PARGS3(StorablePicture *p, int start_mb, int num_mb);

	int do_co_located;

	MB_itrans_Luma_t *FP_MB_itrans_Luma;

	int smart_dec;			//0: normal, 1: I/P only (no B), 2: I only (no P/B), 3: IRD only, 4: Skip bottom field including MBaff
	byte de_blocking_flag;
	int Hybrid_Decoding;
	int stop_indicator;
	bool SkipControlB;
	byte bResidualDataFormat;
	bool set_dpb_buffer_size;
	BOOL SkipThisFrame;
	unsigned long framerate1000;
	int has_pts;
	int  NumClockTs;
	unsigned int SkipedBFrames[MAX_NUM_VIEWS][2];
	// IoK: Introduced to keep track of (possibly) paired fields into 1 Frame
	int m_pic_combine_status;

#if !defined(_GLOBAL_IMG_)
	StorablePicture *m_dec_picture;

#ifdef _COLLECT_PIC_
	StorablePicture *m_dec_picture_top;
	StorablePicture *m_dec_picture_bottom;

	Macroblock *mb_decdata_ori;
	Macroblock *mb_decdata_top;
	Macroblock *mb_decdata_bottom;

	short  *cof_array_ori;  
	short  *cof_array_top;
	short  *cof_array_bottom;
#endif

	int m_global_init_done;

	pic_parameter_set_rbsp_t m_active_pps[2];
	seq_parameter_set_rbsp_t m_active_sps[2];

	StorablePicture **m_listX[6];
	int             m_listXsize[6];

	Macroblock      *m_currMB_r;
	Macroblock_s    *m_currMB_s_r;
	Macroblock      *m_currMB_d;
	Macroblock_s    *m_currMB_s_d;

	// global picture format dependend buffers, mem allocation in decod.c ******************
	int  **m_refFrArr;                                //!< Array for reference frames of each block

	// For MB level frame/field coding
	short  m_InvLevelScale4x4Luma_Intra[6][16];
	short  m_InvLevelScale4x4Chroma_Intra[2][6][16];

	short  m_InvLevelScale4x4Luma_Inter[6][16];
	short  m_InvLevelScale4x4Chroma_Inter[2][6][16];

	short  m_InvLevelScale8x8Luma_Intra[6][8][8];
	short  m_InvLevelScale8x8Luma_Inter[6][8][8];

	UCHAR  *m_qmatrix[8];

	int m_clip_max_x;
	int m_clip_max_y;
	int m_clip_max_x_cr;
	int m_clip_max_y_cr;

	// FMO stuff
	int                 *m_MbToSliceGroupMap;
	int                 *m_MapUnitToSliceGroupMap;
	int                  m_NumberOfSliceGroups;    // the number of slice groups -1 (0 == scan order, 7 == maximum)

	// Decoding stuff
	char                *m_v2b8pd;

	// CABAC stuff
	int                  m_last_dquant;
	unsigned char       *m_map;
	unsigned char       *m_maplast;
	BiContextTypePtr     m_map_ctx;
	BiContextTypePtr     m_last_ctx;
	BiContextTypePtr     m_ctxone;
	BiContextTypePtr     m_ctxlvl;
	unsigned char       *m_c1isdc_maxpos_ptr;
	unsigned char       *m_new_c2_ptr;

	// VLC stuff
	unsigned char       *m_NCBP;

	// Colocated stuff
	ColocatedParamsMB *m_Co_located_MB;
	void (*m_compute_colocated_SUBMB) PARGS6(ColocatedParamsMB* p,
		StorablePicture **listX[6],
		int start_y,
		int start_x,
		int loop_y,
		int loop_x);

	// Deblocking Function pointers
	int (*m_fp_GetStrength_v)(byte *Strength,
		Macroblock_s *MbQ,
		int MbQAddr,
		int mbx,
		int mby,
		Macroblock_s *MbP,
		int edge,
		int mvlimit,
		bool bMixedEdge);
	int (*m_fp_GetStrength_h) PARGS9 (byte *Strength,
		Macroblock_s *MbQ,
		int MbQAddr,
		int mbx,
		int mby,
		Macroblock_s *MbP,
		int edge,
		int mvlimit,
		StorablePicture *p);
	int (*m_fp_GetStrengthInternal_v)(byte *Strength,
		Macroblock_s *MbQ,
		int edge,
		int mvlimit);
	int (*m_fp_GetStrengthInternal_h)(byte *Strength,
		Macroblock_s *MbQ,
		int edge,
		int mvlimit);

#if   defined(H264_ENABLE_INTRINSICS)
	int (*m_fp_GetStrengthInternal_v_3)(
		int  *StrengthSumV,
		byte *Strength,
		Macroblock_s *MbQ,
		int mvlimit);
	int (*m_fp_GetStrengthInternal_h_3)(
		int  *StrengthSumH,
		byte *Strength,
		Macroblock_s *MbQ,
		int mvlimit);
#endif

	// Various
	StorablePicture       *m_prev_dec_picture;
	OldSliceParams         m_old_slice;

	StorablePicture       *m_pending_output;
	int                    m_pending_output_state;

	// From Macroblock.cpp
	read_functions_t      *m_read_functions;
	int (*m_read_one_macroblock) PARGS0();
	CREL_RETURN (*m_readRefInfo) PARGS3(int flag_mode, int list, Pred_s_info *info);
	void (*m_CheckAvailabilityOfNeighbors) PARGS0();

	// mb_motcomp.cpp
	__declspec(align(16)) 
		imgpel m_fw_block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE];
	imgpel m_bw_block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE];

	get_block_WxH_t *m_get_block_4xh;
	get_block_WxH_t *m_get_block_8xh;
	get_block_WxH_t *m_get_block_16xh;
	get_block_WxH_t *m_get_block_4xh_p[8];

	mb_chroma_WxH_t *m_mb_chroma_2xH;
	mb_chroma_WxH_t *m_mb_chroma_4xH;
	mb_chroma_WxH_t *m_mb_chroma_8xH;
	mb_chroma_WxH_t *m_mb_chroma_2xH_p[8];

	// IntraPred (4x4 & 8x8)
	byte             m_LoopArray[32];
	__declspec(align(16)) byte m_PredPel[32] GCC_ALIGN(16);  

	// SLICE function pointers
	int  (*m_nal_startcode_follows) PARGS1(int dummy);

	// NALU-related
	nalu_type *m_nalu;
	PFN_H264HP_GET_DATA  m_get_data_fcn;
	PVOID                m_H264_pvDataContext;

	PFN_H264HP_GET_PARAM m_get_param_fcn;
	DWORD                tReserved[3];

	unsigned long        m_streamptscounter;	/* count of last PTS on stream */
	unsigned long        m_lastptscounter;		/* used to detect new stream pts */
	CH264Ts              m_streampts;			/* passed in via get_data_fcn*/
	CH264Ts              m_PTS;				/* extrapolated from m_streampts */
	int                  m_LastAccessUnitExists;
	int                  m_NALUCount;
	AnnexbNALUDecodingState m_stNAL;

	// IoK: Introduced flag to track HD-profile violations
	HDProfileFault_STATUS   m_HDProfileFault;
	// IoK: Introduced flag to track ASO (arbitraty slice order)
	int                     m_prev_start_mb_nr;

#ifdef __MB_POS_TABLE_LOOKUP__
	MB_POS_TABLE            m_mb_pos_table[16384];
	int                     m_mb_pos_table_size;
	int                     m_pic_width_in_mbs;
#endif


#if !defined(_COLLECT_PIC_)
	unsigned int                    m_storable_picture_count;
	StorablePicture           *m_storable_picture_map[128];
	seq_parameter_set_rbsp_t  m_SeqParSet[MAXSPS];
	pic_parameter_set_rbsp_t  m_PicParSet[MAXPPS];

	FrameStore*            m_out_buffer;

	// DPB and FrameStore-related
	DecodedPictureBuffer   m_dpb;

	const unsigned char *m_buf_begin;
	const unsigned char *m_buf_end;

	int                      m_tot_time;

	// files
	int                      m_p_out;                    //!< file descriptor to output YUV file
	//FILE *p_out2;                    //!< pointer to debug output YUV file
	int                      m_p_ref;                    //!< pointer to input original reference YUV file file

	FILE                    *m_p_log;                    //!< SNR file

	CH264VideoFrameMgr      *m_framemgr;
	unsigned long            m_dwSkipFrameCounter;
	unsigned int             m_uSkipBFrameCounter[2];
	unsigned int             m_uSkipSmartDecBFrameCounter;

	H264_TS                  m_pts;
	H264_TS                  m_ref_pts;
	int                      m_has_pts;
	int                      m_has_pts_poc;
	unsigned long            m_framerate1000;
	unsigned __int64         m_llDispCount;
	unsigned __int64         m_llDispPulldownCount;
	bool                     m_pulldown;
	unsigned int             m_PulldownRate;
	unsigned __int64         m_llDtsCount;
	unsigned __int64         m_llDecCount;

#endif//End of _COLLECT_PIC_

	unsigned char				m_line21buf[LINE21BUF_SIZE];
	unsigned int				m_line21len;
	char dummt_tmp[7];
#endif /* NOT _GLOBAL_IMG_ */

#ifdef _COLLECT_PIC_
	p_stream_par stream_global;
	Slice *firstSlice;
	Slice *prevSlice;
	byte  *ori_nalu_buf;
	int   slice_number;
	int   m_start_mb_nr;
#endif

	//for DXVA
#if defined(_HW_ACCEL_)
	long    m_DXVAVer;  

	BYTE*	  m_lpRESD_DXVA;

	BYTE*   m_pnv1PictureDecode;
	BYTE*   m_lpQMatrix;
	BYTE*   m_lpMBLK_Intra_Luma;	// MB/Resd -> Intra/Inter -> Luma/Chroma
	BYTE*   m_lpMBLK_Intra_Chroma;
	BYTE*   m_lpMBLK_Inter_Luma;
	BYTE*   m_lpMBLK_Inter_Chroma;
	BYTE*   m_lpRESD_Intra_Luma;
	BYTE*   m_lpRESD_Intra_Chroma;
	BYTE*   m_lpRESD_Inter_Luma;
	BYTE*   m_lpRESD_Inter_Chroma;
	BYTE*   m_lpDEBLK_Luma;
	BYTE*   m_lpDEBLK_Chroma;
	BYTE*	  m_lpMV;
	BYTE*	  m_lpSLICE;

	int     m_lFirstMBAddress_Inter;
	int	  m_lFirstMBAddress_Intra;
	int	  m_lmbCount_Inter;
	int	  m_lmbCount_Intra;
	int	  m_iFirstCompBufIndex;
	int	  m_Intra_lCompBufIndex;
	int     pic_dec_buf_idx;
	int     m_iIntraMCBufUsage;
	int     m_iInterMCBufUsage;
	int     m_iIntraRESBufUsage;
	int     m_iInterRESBufUsage_L;
	int     m_iInterRESBufUsage_C;
	int     m_IntraL_lCompBufIndex;
	int     m_IntraC_lCompBufIndex;
	int     m_InterL_lCompBufIndex;
	int     m_InterC_lCompBufIndex;
	int     m_lFrame_Counter;
	int     UnCompress_Idx;

	BOOL	  m_bLastIntraMB;
	BOOL	  m_bLastPairIntraMBs;
	BOOL	  m_lPrev_Field_pair;
	BOOL    m_bFirstMB;
	int	  m_L0L1Map[128];
	int	  m_TopL0Map[128];
	int	  m_TopL1Map[128];
	int	  m_BotL0Map[128];
	int	  m_BotL1Map[128];
	int	  m_UpLeft;
	int     m_UpRight;
	int	  m_PicHeight;
	int	  m_MVBufSwitch;
	int	  m_SBBufOffset;
	int	  m_iFp_Idx;
	int	  m_slice_number_in_field;

	DXVA_BufferDescription m_pDxvaBufferDescription[6];
	AMVABUFFERINFO         m_pBufferInfo[6];

	long	  m_UnCompressedBufferStride;
	BYTE*	  m_pUnCompressedBuffer;

	int     m_data_count_for_Mode_C;		//For Intel Mode C
	USHORT  m_BitOffsetToSliceData; //For Intel Mode E


#endif  

	char	cr_vector_adjustment[6][MAX_LIST_SIZE];

	DWORD dwFillFrameNumGap;
	CREL_RETURN (*m_start_macroblock) PARGS0();

	BOOL bReadCompleted;

	H264_CC m_CCCode;

	BYTE *decoded_flag;  //record if the MB is decoded or not

	//for NV_SECOP
	DWORD dwSliceCountForSECOP;
	SECOP_SliceInfo sSliceInfoArray[4096];

#ifdef DEBUG_SHOW_PROCESS_TIME
	time_t ltime_start;               // for time measurement
	time_t ltime_end;                 // for time measurement

#ifdef WIN32
	struct _timeb tstruct_start;
	struct _timeb tstruct_end;
#else
	struct timeb tstruct_start;
	struct timeb tstruct_end;
#endif
#endif 


} ImageParameters GCC_ALIGN(32);

#ifdef _COLLECT_PIC_
typedef __declspec(align(32)) struct stream_par 
{
	// DPB and FrameStore-related
	DecodedPictureBuffer    m_dpb;
	CH264VideoFrameMgr      *m_framemgr;
	unsigned long           m_dwSkipFrameCounter;
	unsigned int            m_uSkipBFrameCounter[MAX_NUM_VIEWS][2];
	unsigned int            m_uSkipSmartDecBFrameCounter;
	H264_TS                 m_pts;
	H264_TS                 m_ref_pts;
	int                     m_has_pts;
	int                     m_has_pts_poc;
	unsigned long           m_framerate1000;
	unsigned __int64        m_llDispCount[16];
	unsigned __int64        m_llDispPulldownCount[16];
	bool                    m_pulldown;
	unsigned int            m_PulldownRate;
	unsigned __int64        m_llDtsCount[MAX_NUM_VIEWS];
	unsigned __int64        m_llDecCount[MAX_NUM_VIEWS];

	nalu_type               *m_nalu_global;
	int	                    m_nalu_global_available;
	ImageParameters         **m_img;
	seq_parameter_set_rbsp_t *m_active_sps_on_view[MAX_NUM_VIEWS];
	pic_parameter_set_rbsp_t *m_active_pps_on_view[MAX_NUM_VIEWS];
	unsigned int							m_storable_picture_count;
	StorablePicture           *m_storable_picture_map[512];
	seq_parameter_set_rbsp_t  m_SeqParSet[MAXSPS];
	seq_parameter_set_rbsp_t  m_SeqParSubset[MAXSPS];
	pic_parameter_set_rbsp_t  m_PicParSet[MAXPPS];
	int						no_output_of_prior_pics_flag;

	//Collect Data
	int							number;
	int						pre_frame_num;
	int						PreviousFrameNum[MAX_NUM_VIEWS];
	int						PreviousFrameNumOffset[MAX_NUM_VIEWS];
	int							ThisPOC[MAX_NUM_VIEWS];
	int						PreviousPOC[MAX_NUM_VIEWS];
	int							PrevPicOrderCntLsb[MAX_NUM_VIEWS];
	int							PrevPicOrderCntMsb[MAX_NUM_VIEWS];
	int							 m_listXsize[6];

	const unsigned char     *m_buf_begin;
	const unsigned char     *m_buf_end;

	FrameStore*             m_out_buffer;
	AnnexbNALUDecodingState		m_stNAL;

	//picture decoding status
	int pic_ip_num;
	int pic_b_num;
	int is_first;
	int bHasB;
	int b_Count;
	int nSwitchB;
	int two_B_Flag;
	int nNeedToDecodeIP;
	int nCollectB;
	int last_has_mmco_5;

	img_par *m_gImgIP_r;
	img_par *m_gImgIP_d;
	img_par *m_gImgB_r0;
	img_par *m_gImgB_r1;
	img_par *m_gImgB_d0;
	img_par *m_gImgB_d1;

	//Decoding thread related
	HANDLE						m_read_handle_ip;
	HANDLE						m_read_handle_b0;
	HANDLE						m_read_handle_b1;

	HANDLE						m_decode_handle_ip;
	HANDLE						m_decode_handle_b0;
	HANDLE						m_decode_handle_b1;

	HANDLE						m_event_read_start_ip;
	HANDLE						m_event_read_finish_ip;
	HANDLE						m_event_read_start_b[2];
	HANDLE						m_event_read_finish_b[2];

	HANDLE						m_event_decode_start_ip;
	HANDLE						m_event_decode_finish_ip;
	HANDLE						m_event_decode_start_b[2];
	HANDLE						m_event_decode_finish_b[2];

	HANDLE						m_event_for_field_ip;

	HANDLE						m_event_RB_1stfield_decode_complete;
	HANDLE						m_event_RB_2ndfield_read_complete;
	HANDLE						m_event_RB_wait_clear;

	HANDLE						m_handle_decode_thread;
	HANDLE						m_event_decode_thread;
	HANDLE						m_event_begin_decode_thread;
	HANDLE						m_event_finish_decode_thread;

	HANDLE						m_handle_decode_picture_RB;
	HANDLE						m_event_start_pic_RB;
	HANDLE						m_event_finish_pic_RB;

	int						m_event_exit_flag;
	int						m_next_image_type;
	//~Decoding thread

	int                      m_tot_time;
	int                      m_p_out;                    //!< file descriptor to output YUV file	
	int                      m_p_ref;                    //!< pointer to input original reference YUV file file
	FILE                    *m_p_log;                    //!< SNR file

	bool                     bSeekToOpenGOP;             //Terry: this flag is only for seeking.

	//MTMS
	HANDLE						m_decode_slice_handle_0;
	HANDLE						m_decode_slice_handle_1;
	HANDLE						m_decode_slice_handle_2;
	HANDLE						m_decode_slice_handle_3;
	HANDLE						m_decode_slice_handle_4;
	HANDLE						m_decode_slice_handle_5;
	HANDLE						m_event_start_slice[6];
	HANDLE						m_event_end_slice[6];
	int								m_is_MTMS;
	//~MTMS

	LPVOID	m_pVideoAccel;  // ==0 for disable _HW_ACCEL_ case

	unsigned int uiH264DXVAMode;

	int m_iStop_Decode;

	int	m_resize_width_height;
	int	m_resize_width_height_reg; //Registry value.
	int profile_idc;

#if defined(_USE_QUEUE_FOR_DXVA2_) || defined(_USE_QUEUE_FOR_DXVA1_)
	int m_dxva_queue_reset;
	HANDLE m_queue_semaphore;
	HANDLE h_dxva_queue_reset_done; //for reset
	HANDLE hReadyForRender[20];
	CRITICAL_SECTION crit_ACCESS_EXESTA;
#endif

	CRITICAL_SECTION crit_dyna_expand;
	CRITICAL_SECTION m_csExit_MB;

	HANDLE hFinishDisplaySemaphore;
	HANDLE hGetDisplaySemaphore;
	int m_iEstimateNextFrame;
	LARGE_INTEGER m_liRecodeTime0, m_liRecodeTime1;

	BOOL m_Initial_Flag;

	bool m_bTrickEOS; //set EOS by upper layer.

	BOOL m_bReceivedFirst;
	BOOL m_bNormalSpeed;
	BOOL m_bRewindDecoder;

	//From class CH264VDecHP, playback control related
	BOOL m_bEOS;					// true when end of stream occurs
	DWORD m_dwSmartDecLevel;		// smart decoding level
	DWORD	m_dwSavedSmartDecLevel;
	BOOL m_bDoSkipPB;
	long lDXVAVer;

	BOOL m_bSeekIDR;
	BOOL m_bRefBWaitB;
	BOOL m_bRefBWaitP;
	BOOL m_bRefBWaitBRead;

	int m_bMultiThreadModeSwitch;	
	// 0: No Switch   
	// 1: Switch after complete remaining pictures. ( Single Slice mode to Multi Slice mode at middle of stream, some pictures remained to decode )
	// 2: Switch directly ( Multi Slice mode to Single Slice mode, Single Slice mode to Multi Slice at very beginning, no pictures remained to decode )
	decode_picture_t * m_decode_picture;

	LPVOID m_pH264DxvaBase; //for CH264DxvaBase
	LPVOID m_pH264DXVA;

	BOOL m_bNextImgForRef;
	Slice *m_pNewSlice;

	CATSCDec *m_pATSCDec;
	H264_CC *m_pArrangedPCCCode;

	BOOL m_bDisplayed;

	BOOL m_bSkipFirstB;
	BOOL m_bNotHDDVD;

	BYTE  m_pbYCCBuffer[12];
	DWORD m_dwYCCBufferLen;

	int pre_sps_width;
	int pre_sps_height;
	int frame_execute_count;
	BOOL m_iExecute_Of_I_Frame;
	BOOL m_SPSchecked;
	bool m_CheckExecute;

	nalu_header_mvc_extension_t	nalu_mvc_extension;
	unsigned int num_views;
	unsigned int dpb_pre_alloc_views;
	BOOL bMVC;

	BOOL m_pbValidViews[16];
	int m_CurrOutputViewIndex;

#if defined(_SHOW_THREAD_TIME_)
	LARGE_INTEGER t_freq, t_ip[4], t_b[8];
#endif

	int m_iPOCofDroppedFrame;
	unsigned int m_iNextViewIndex;
	int m_iNextPOC;
	int m_iFrameNeedtoSkip;

	BOOL m_bIsSingleThreadMode;
	void *m_pListOffsetMetadata;

	BOOL m_bIsBD3D;

	H264_TS                 m_pts_baseview;
	H264_TS                 m_dts_baseview;
	int                     m_has_pts_baseview;

	CRITICAL_SECTION m_csOffsetMetadata;

	//For MVC filed coding Active Image index
	int m_iMVCFieldPicActiveIndex;
}StreamParameters GCC_ALIGN(32);;
#endif

// IoK: INITIALIZATION VALUES - collected from different files.
// Someone needs to run a sanity check on them...
//PFN_H264HP_GET_DATA  get_data_fcn = NULL;
//PVOID                H264_pvDataContext=NULL;
//FILE *               bits = NULL;
//int                  IsFirstByteStreamNALU=1;
//int                  LastAccessUnitExists=0;
//int                  NALUCount=0;
//int                  last_dquant = 0;
//int *                MbToSliceGroupMap = NULL;
//int *                MapUnitToSliceGroupMap = NULL;
//CH264VideoFrameMgr * g_framemgr = 0; <- every time
//int                  global_init_done = 0;
//int                  mb_pos_table_size = -1;
//int                  pic_width_in_mbs=-1;
//ColocatedParamsMB *  Co_located_MB = NULL;
//StorablePicture *    pending_output = NULL;
//int                  pending_output_state = FRAME;

#ifdef _GLOBAL_IMG_

GL_EXTERN ImageParameters img;
#define IMGPAR img.
GL_EXTERN StorablePicture *dec_picture;
GL_EXTERN Macroblock      *currMB_r;
GL_EXTERN Macroblock_s    *currMB_s_r;
GL_EXTERN Macroblock      *currMB_d;
GL_EXTERN Macroblock_s    *currMB_s_d;

//GL_EXTERN pic_parameter_set_rbsp_t *active_pps;
//GL_EXTERN seq_parameter_set_rbsp_t *active_sps;
GL_EXTERN pic_parameter_set_rbsp_t m_active_pps[2];
GL_EXTERN seq_parameter_set_rbsp_t m_active_sps[2];
//GL_EXTERN seq_parameter_set_rbsp_t SeqParSet[MAXSPS];
//GL_EXTERN pic_parameter_set_rbsp_t PicParSet[MAXPPS];
GL_EXTERN int                      global_init_done;

// global picture format dependend buffers, mem allocation in decod.c ******************
GL_EXTERN int  **refFrArr;                                //!< Array for reference frames of each block

// For MB level frame/field coding
GL_EXTERN short  InvLevelScale4x4Luma_Intra[6][16];
GL_EXTERN short  InvLevelScale4x4Chroma_Intra[2][6][16];

GL_EXTERN short  InvLevelScale4x4Luma_Inter[6][16];
GL_EXTERN short  InvLevelScale4x4Chroma_Inter[2][6][16];

GL_EXTERN short  InvLevelScale8x8Luma_Intra[6][8][8];
GL_EXTERN short  InvLevelScale8x8Luma_Inter[6][8][8];

GL_EXTERN UCHAR  *qmatrix[8];

GL_EXTERN int clip_max_x;
GL_EXTERN int clip_max_y;
GL_EXTERN int clip_max_x_cr;
GL_EXTERN int clip_max_y_cr;

// IoK: Introduced to keep track of (possibly) paired fields into 1 Frame
GL_EXTERN int pic_combine_status;

// FMO stuff
GL_EXTERN int *MbToSliceGroupMap;
GL_EXTERN int *MapUnitToSliceGroupMap;

GL_EXTERN int  NumberOfSliceGroups;    // the number of slice groups -1 (0 == scan order, 7 == maximum)

// Decoding stuff
GL_EXTERN char *v2b8pd;

// CABAC stuff
GL_EXTERN unsigned char coeff_pos[64];
GL_EXTERN short         coeff_level[64];
GL_EXTERN int           last_dquant;
GL_EXTERN unsigned char *map;
GL_EXTERN unsigned char *maplast;
GL_EXTERN BiContextTypePtr map_ctx;
GL_EXTERN BiContextTypePtr last_ctx;
GL_EXTERN BiContextTypePtr ctxone;
GL_EXTERN BiContextTypePtr ctxlvl;
GL_EXTERN unsigned char *c1isdc_maxpos_ptr;
GL_EXTERN unsigned char *new_c2_ptr;

GL_EXTERN StorablePicture         **listX[6];
GL_EXTERN int                       listXsize[6];

// Colocated stuff
GL_EXTERN ColocatedParamsMB *Co_located_MB;
GL_EXTERN void (*compute_colocated_SUBMB) PARGS6(ColocatedParamsMB* p,
																								 StorablePicture **list[6],
																								 int start_y,
																								 int start_x,
																								 int loop_y,
																								 int loop_x);

// Deblocking function pointers
GL_EXTERN int (*fp_GetStrength_v)(byte *Strength,
																	Macroblock_s *MbQ,
																	int MbQAddr,
																	int mbx,
																	int mby,
																	Macroblock_s *MbP,
																	int edge,
																	int mvlimit,
																	bool bMixedEdge);
GL_EXTERN int (*fp_GetStrength_h) PARGS9(byte *Strength,
																				 Macroblock_s *MbQ,
																				 int MbQAddr,
																				 int mbx,
																				 int mby,
																				 Macroblock_s *MbP,
																				 int edge,
																				 int mvlimit,
																				 StorablePicture *p);
GL_EXTERN int (*fp_GetStrengthInternal_v)(byte *Strength,
																					Macroblock_s *MbQ,
																					int edge,
																					int mvlimit);
GL_EXTERN int (*fp_GetStrengthInternal_h)(byte *Strength,
																					Macroblock_s *MbQ,
																					int edge,
																					int mvlimit);

#if   defined(H264_ENABLE_INTRINSICS)
GL_EXTERN int (*fp_GetStrengthInternal_v_3)(
	int  *StrengthSumV,
	byte *Strength,
	Macroblock_s *MbQ,
	int mvlimit);
GL_EXTERN int (*fp_GetStrengthInternal_h_3)(
	int  *StrengthSumH,
	byte *Strength,
	Macroblock_s *MbQ,
	int mvlimit);
#endif

// Various
GL_EXTERN StorablePicture         *prev_dec_picture;
// DPB and FrameStore-related
//GL_EXTERN DecodedPictureBuffer     dpb;
//GL_EXTERN FrameStore*              out_buffer;
GL_EXTERN StorablePicture         *pending_output;
GL_EXTERN int                      pending_output_state;
//GL_EXTERN int                      storable_picture_count;
//GL_EXTERN StorablePicture         *storable_picture_map[128];

// VLC stuff
GL_EXTERN unsigned char         *NCBP;

// From Macroblock.cpp
GL_EXTERN read_functions_t      *g_read_functions;
GL_EXTERN int (*read_one_macroblock) PARGS0();
GL_EXTERN void (*CheckAvailabilityOfNeighbors) PARGS0();

GL_EXTERN CREL_RETURN (*readRefInfo) PARGS3(int flag_mode,
																						int list,
																						Pred_s_info *info);
// SLICE function pointers
GL_EXTERN int  (*nal_startcode_follows) PARGS1(int dummy);


GL_EXTERN OldSliceParams         old_slice;

// mb_motcomp.cpp
GL_EXTERN __declspec(align(16)) imgpel fw_block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE];
GL_EXTERN __declspec(align(16)) imgpel bw_block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE];

// IntraPred (4x4 & 8x8)
GL_EXTERN  byte             LoopArray[32];
GL_EXTERN  __declspec(align(16)) byte PredPel[32];  // array of predictor pels

// NALU-related
GL_EXTERN nalu_type *nalu;
GL_EXTERN PFN_H264HP_GET_DATA  get_data_fcn;
GL_EXTERN PVOID                H264_pvDataContext;
GL_EXTERN PFN_H264HP_GET_PARAM get_param_fcn;
//GL_EXTERN const unsigned char *buf_begin;
//GL_EXTERN const unsigned char *buf_end;
GL_EXTERN unsigned long        streamptscounter;	/* count of last PTS on stream */
GL_EXTERN unsigned long        lastptscounter;		/* used to detect new stream pts */
GL_EXTERN CH264Ts              streampts;			/* passed in via get_data_fcn*/
GL_EXTERN CH264Ts              PTS;				/* extrapolated from m_streampts */
GL_EXTERN int                  LastAccessUnitExists;
GL_EXTERN int                  NALUCount;
//GL_EXTERN AnnexbNALUDecodingState stNAL;

// IoK: Introduced flag to track HD-profile violations
//GL_EXTERN HDProfileFault_STATUS   g_HDProfileFault;
// IoK: Introduced flag to track ASO (arbitraty slice order)
GL_EXTERN int                     g_prev_start_mb_nr;

#ifdef __MB_POS_TABLE_LOOKUP__
GL_EXTERN MB_POS_TABLE            mb_pos_table[16384];
GL_EXTERN int                     mb_pos_table_size;
GL_EXTERN int                     pic_width_in_mbs;
#endif

#if !defined(_COLLECT_PIC_)
GL_EXTERN int                      storable_picture_count;
GL_EXTERN StorablePicture         *storable_picture_map[128];
GL_EXTERN seq_parameter_set_rbsp_t SeqParSet[MAXSPS];
GL_EXTERN pic_parameter_set_rbsp_t PicParSet[MAXPPS];

// DPB and FrameStore-related
GL_EXTERN DecodedPictureBuffer     dpb;
GL_EXTERN FrameStore*              out_buffer;

GL_EXTERN const unsigned char *buf_begin;
GL_EXTERN const unsigned char *buf_end;
GL_EXTERN AnnexbNALUDecodingState stNAL;

GL_EXTERN int                     tot_time;

// files
GL_EXTERN int                     p_out;                    //!< file descriptor to output YUV file
//FILE *p_out2;                    //!< pointer to debug output YUV file
GL_EXTERN int                     p_ref;                    //!< pointer to input original reference YUV file file

GL_EXTERN FILE                   *p_log;                    //!< SNR file

GL_EXTERN CH264VideoFrameMgr     *g_framemgr;
GL_EXTERN unsigned long           g_dwSkipFrameCounter;
GL_EXTERN unsigned int            g_uSkipBFrameCounter[2];

GL_EXTERN H264_TS                 g_pts;
GL_EXTERN H264_TS                 g_ref_pts;
GL_EXTERN int                     g_has_pts;
GL_EXTERN unsigned long           g_framerate1000;
GL_EXTERN unsigned __int64        g_llDispCount;
GL_EXTERN unsigned __int64        g_llDispPulldownCount;
GL_EXTERN bool                    g_pulldown;
GL_EXTERN unsigned int            g_PulldownRate;
GL_EXTERN unsigned __int64        g_llDtsCount;
GL_EXTERN unsigned __int64        g_llDecCount;

GL_EXTERN int                     g_uiH264DXVAMode;

GL_EXTERN  get_block_WxH_t        *get_block_4xh;
GL_EXTERN  get_block_WxH_t        *get_block_8xh;
GL_EXTERN  get_block_WxH_t        *get_block_16xh;
GL_EXTERN  get_block_WxH_t        *get_block_4xh_p[8];

GL_EXTERN  mb_chroma_WxH_t        *mb_chroma_2xH;
GL_EXTERN  mb_chroma_WxH_t        *mb_chroma_4xH;
GL_EXTERN  mb_chroma_WxH_t        *mb_chroma_8xH;
GL_EXTERN  mb_chroma_WxH_t        *mb_chroma_2xH_p[8];

GL_EXTERN  BOOL g_bReceivedFirst;
GL_EXTERN  BOOL g_bNormalSpeed;
GL_EXTERN  CIsmpInputObject g_IsmpData;
GL_EXTERN  BOOL g_bIsmpEnabled;

GL_EXTERN  DWORD g_dwSmartDecLevel;
GL_EXTERN  BOOL  g_bEOS;
GL_EXTERN  decode_picture_t *g_pfnDecodePicture;

GL_EXTERN  Slice *g_pNewSlice;

GL_EXTERN CATSCDec *g_pCCDec;
GL_EXTERN H264_CC *g_pArrangedPCCCode;

#define active_pps m_active_pps[IMGPAR structure == BOTTOM_FIELD]
#define active_sps m_active_sps[IMGPAR structure == BOTTOM_FIELD]

GL_EXTERN  LPVOID g_pH264DXVA;

GL_EXTERN  BOOL g_bSkipFirstB;

GL_EXTERN  BOOL g_bDisplayed;

GL_EXTERN  BYTE g_pbYCCBuffer[12];
GL_EXTERN  DWORD g_dwYCCBufferLen;

#endif //End of _COLLECT_PIC_


#else /* NOT _GLOBAL_IMG_ */

//GL_EXTERN ImageParameters *img;
#define IMGPAR                        img->
#define dec_picture                   img->m_dec_picture
#define currMB_r                        img->m_currMB_r
#define currMB_s_r                      img->m_currMB_s_r
#define currMB_d                        img->m_currMB_d
#define currMB_s_d                      img->m_currMB_s_d

#define active_pps                    img->m_active_pps[IMGPAR structure == BOTTOM_FIELD]
#define active_sps                    img->m_active_sps[IMGPAR structure == BOTTOM_FIELD]
//#define SeqParSet                     img->m_SeqParSet
//#define PicParSet                     img->m_PicParSet
#define global_init_done              img->m_global_init_done

// global picture format dependend buffers, mem allocation in decod.c ******************
#define refFrArr                      img->m_refFrArr

// For MB level frame/field coding
#define InvLevelScale4x4Luma_Intra    img->m_InvLevelScale4x4Luma_Intra
#define InvLevelScale4x4Chroma_Intra  img->m_InvLevelScale4x4Chroma_Intra
#define InvLevelScale4x4Luma_Inter    img->m_InvLevelScale4x4Luma_Inter
#define InvLevelScale4x4Chroma_Inter  img->m_InvLevelScale4x4Chroma_Inter
#define InvLevelScale8x8Luma_Intra    img->m_InvLevelScale8x8Luma_Intra
#define InvLevelScale8x8Luma_Inter    img->m_InvLevelScale8x8Luma_Inter
#define qmatrix                       img->m_qmatrix

#define clip_max_x                    img->m_clip_max_x
#define clip_max_y                    img->m_clip_max_y
#define clip_max_x_cr                 img->m_clip_max_x_cr
#define clip_max_y_cr                 img->m_clip_max_y_cr

// IoK: Introduced to keep track of (possibly) paired fields into 1 Frame
#define pic_combine_status            img->m_pic_combine_status

// FMO stuff
#define MbToSliceGroupMap             img->m_MbToSliceGroupMap
#define MapUnitToSliceGroupMap        img->m_MapUnitToSliceGroupMap

#define NumberOfSliceGroups           img->m_NumberOfSliceGroups

// Decoding stuff
#define v2b8pd                        img->m_v2b8pd

// CABAC stuff
//#define coeff_pos                     currMB_r->m_coeff_pos
//#define coeff_level                   currMB_r->m_coeff_level
#define coeff_pos                     img->m_coeff_pos
#define coeff_level                   img->m_coeff_level
#define last_dquant                   img->m_last_dquant
#define map                           img->m_map
#define maplast                       img->m_maplast
#define map_ctx                       img->m_map_ctx
#define last_ctx                      img->m_last_ctx
#define ctxone                        img->m_ctxone
#define ctxlvl                        img->m_ctxlvl
#define c1isdc_maxpos_ptr             img->m_c1isdc_maxpos_ptr
#define new_c2_ptr                    img->m_new_c2_ptr

#define listX                         img->m_listX
#define listXsize                     img->m_listXsize

// Colocated stuff
#define Co_located_MB                 img->m_Co_located_MB
#define compute_colocated_SUBMB       img->m_compute_colocated_SUBMB

// Deblocking function pointers
#define fp_GetStrength_v              img->m_fp_GetStrength_v
#define fp_GetStrength_h              img->m_fp_GetStrength_h
#define fp_GetStrengthInternal_v      img->m_fp_GetStrengthInternal_v
#define fp_GetStrengthInternal_h      img->m_fp_GetStrengthInternal_h
#if   defined(H264_ENABLE_INTRINSICS)
#define fp_GetStrengthInternal_v_3      img->m_fp_GetStrengthInternal_v_3
#define fp_GetStrengthInternal_h_3      img->m_fp_GetStrengthInternal_h_3
#endif

// Various
#define prev_dec_picture              img->m_prev_dec_picture
// DPB and FrameStore-related
//#define dpb                           img->m_dpb
//#define out_buffer                    img->m_out_buffer
#define pending_output                img->m_pending_output
#define pending_output_state          img->m_pending_output_state
//#define storable_picture_count        img->m_storable_picture_count
//#define storable_picture_map          img->m_storable_picture_map

// VLC stuff
#define NCBP                          img->m_NCBP

// From Macroblock.cpp
#define g_read_functions              img->m_read_functions
#define read_one_macroblock           img->m_read_one_macroblock
#define CheckAvailabilityOfNeighbors  img->m_CheckAvailabilityOfNeighbors
#define start_macroblock	      img->m_start_macroblock

#define readRefInfo                   img->m_readRefInfo
#define nal_startcode_follows         img->m_nal_startcode_follows

#define old_slice                     img->m_old_slice

// mb_motcomp.cpp
#define fw_block                      img->m_fw_block
#define bw_block                      img->m_bw_block

#define get_block_4xh                 img->m_get_block_4xh
#define get_block_8xh                 img->m_get_block_8xh
#define get_block_16xh                img->m_get_block_16xh
#define get_block_4xh_p               img->m_get_block_4xh_p

#define mb_chroma_2xH                 img->m_mb_chroma_2xH
#define mb_chroma_4xH                 img->m_mb_chroma_4xH
#define mb_chroma_8xH                 img->m_mb_chroma_8xH
#define mb_chroma_2xH_p               img->m_mb_chroma_2xH_p

// IntraPred (4x4 & 8x8)
#define LoopArray                     img->m_LoopArray
#define PredPel                       img->m_PredPel

// NALU-related
#define nalu                          img->m_nalu
#define get_data_fcn                  img->m_get_data_fcn
#define H264_pvDataContext            img->m_H264_pvDataContext
#define get_param_fcn                 img->m_get_param_fcn
//#define buf_begin                     img->m_buf_begin
//#define buf_end                       img->m_buf_end
#define streamptscounter              img->m_streamptscounter
#define lastptscounter                img->m_lastptscounter
#define streampts                     img->m_streampts
#define PTS                           img->m_PTS
#define LastAccessUnitExists          img->m_LastAccessUnitExists
#define NALUCount                     img->m_NALUCount
//#define stNAL                         img->m_stNAL

// IoK: Introduced flag to track HD-profile violations
//#define g_HDProfileFault              img->m_HDProfileFault
// IoK: Introduced flag to track ASO (arbitraty slice order)
#define g_prev_start_mb_nr            img->m_prev_start_mb_nr

#ifdef __MB_POS_TABLE_LOOKUP__
#define mb_pos_table                  img->m_mb_pos_table
#define mb_pos_table_size             img->m_mb_pos_table_size
#define pic_width_in_mbs              img->m_pic_width_in_mbs
#endif
#ifdef _HW_ACCEL_
#define imgDXVAVer				  img->m_DXVAVer
#endif
#if !defined (_COLLECT_PIC_) //KevinChien
#define storable_picture_count        img->m_storable_picture_count
#define storable_picture_map          img->m_storable_picture_map
#define SeqParSet                     img->m_SeqParSet
#define PicParSet                     img->m_PicParSet

#define dpb                           img->m_dpb
#define out_buffer                    img->m_out_buffer

#define buf_begin                     img->m_buf_begin
#define buf_end                       img->m_buf_end

#define stNAL                         img->m_stNAL

#define tot_time                      img->m_tot_time

// files
#define p_out                         img->m_p_out
#define p_ref                         img->m_p_ref
#define p_log                         img->m_p_log

#define g_framemgr                    img->m_framemgr
#define g_dwSkipFrameCounter          img->m_dwSkipFrameCounter
#define g_uSkipBFrameCounter          img->m_uSkipBFrameCounter

#define g_pts                         img->m_pts
#define g_ref_pts                     img->m_ref_pts
#define g_has_pts                     img->m_has_pts
#define g_has_pts_poc                 img->m_has_pts_poc
#define g_framerate1000               img->m_framerate1000
#define g_llDispCount                 img->m_llDispCount
#define g_llDispPulldownCount         img->m_llDispPulldownCount
#define g_pulldown                    img->m_pulldown
#define g_PulldownRate                img->m_PulldownRate
#define g_llDtsCount                  img->m_llDtsCount
#define g_llDecCount                  img->m_llDecCount

#endif
//End of _COLLECT_PIC_

#endif /* _GLOBAL_IMG_ or NOT */

//KevinChien
#ifdef _COLLECT_PIC_
#define   IMG_NUM                 8
#define   nalu_global				stream_global->m_nalu_global
#define   nalu_global_available		stream_global->m_nalu_global_available

#define	  storable_picture_count    stream_global->m_storable_picture_count
#define   storable_picture_map      stream_global->m_storable_picture_map
#define   SeqParSet                 stream_global->m_SeqParSet
#define   SeqParSubset              stream_global->m_SeqParSubset
#define   PicParSet                 stream_global->m_PicParSet
#define   dpb						stream_global->m_dpb
#define	  out_buffer				stream_global->m_out_buffer
#define   buf_begin     			stream_global->m_buf_begin
#define   buf_end       			stream_global->m_buf_end

#define   img_array                 stream_global->m_img
#define   stNAL                     stream_global->m_stNAL

#define   read_handle_ip 					stream_global->m_read_handle_ip
#define		read_handle_b0					stream_global->m_read_handle_b0
#define		read_handle_b1					stream_global->m_read_handle_b1

#define   decode_handle_ip				stream_global->m_decode_handle_ip
#define   decode_handle_b0				stream_global->m_decode_handle_b0
#define   decode_handle_b1				stream_global->m_decode_handle_b1

#define   event_RB_1stfield_decode_complete		stream_global->m_event_RB_1stfield_decode_complete
#define   event_RB_2ndfield_read_complete   stream_global->m_event_RB_2ndfield_read_complete
#define   event_RB_wait_clear						stream_global->m_event_RB_wait_clear

#define   g_event_exit_flag       stream_global->m_event_exit_flag

#define   next_image_type		stream_global->m_next_image_type
#define   g_bNextImgForRef  stream_global->m_bNextImgForRef

#define   tot_time      			stream_global->m_tot_time
#define   p_out         			stream_global->m_p_out
#define   p_ref         			stream_global->m_p_ref
#define   p_log         			stream_global->m_p_log

#define   g_framemgr    			stream_global->m_framemgr
#define   g_dwSkipFrameCounter			stream_global->m_dwSkipFrameCounter
#define   g_uSkipBFrameCounter      stream_global->m_uSkipBFrameCounter

#define   g_pts                     stream_global->m_pts
#define   g_ref_pts                 stream_global->m_ref_pts
#define   g_has_pts                 stream_global->m_has_pts
#define   g_has_pts_poc             stream_global->m_has_pts_poc

#define   g_pts_baseview            stream_global->m_pts_baseview
#define   g_dts_baseview            stream_global->m_dts_baseview
#define   g_has_pts_baseview        stream_global->m_has_pts_baseview

#define   g_framerate1000           stream_global->m_framerate1000
#define	  g_llDispCount             stream_global->m_llDispCount
#define   g_llDispPulldownCount     stream_global->m_llDispPulldownCount
#define	  g_pulldown                stream_global->m_pulldown
#define   g_PulldownRate            stream_global->m_PulldownRate
#define   g_llDtsCount              stream_global->m_llDtsCount
#define   g_llDecCount              stream_global->m_llDecCount

#define		decode_slice_handle_0		stream_global->m_decode_slice_handle_0
#define		decode_slice_handle_1		stream_global->m_decode_slice_handle_1
#define		decode_slice_handle_2		stream_global->m_decode_slice_handle_2
#define		decode_slice_handle_3		stream_global->m_decode_slice_handle_3
#define		decode_slice_handle_4		stream_global->m_decode_slice_handle_4
#define		decode_slice_handle_5		stream_global->m_decode_slice_handle_5

#define		gImgIP_r							stream_global->m_gImgIP_r
#define		gImgIP_d							stream_global->m_gImgIP_d
#define		gImgB_r0							stream_global->m_gImgB_r0
#define		gImgB_r1							stream_global->m_gImgB_r1
#define		gImgB_d0							stream_global->m_gImgB_d0
#define		gImgB_d1							stream_global->m_gImgB_d1

// The following is used in H264VDecHP.cpp, (H264VDecHP_Stop, H264VDecHP_DecodeFrame) where stream is obtained from imgp
#define   g_Initial_Flag        stream_global->m_Initial_Flag

#define   g_DXVAVer             stream_global->lDXVAVer
#define   g_DXVAMode            stream_global->uiH264DXVAMode

#define   g_bReceivedFirst      stream_global->m_bReceivedFirst
#define	  g_bNormalSpeed        stream_global->m_bNormalSpeed
#define   g_bRewindDecoder      stream_global->m_bRewindDecoder

#define   g_dwSmartDecLevel     stream_global->m_dwSmartDecLevel
#define   g_bEOS                stream_global->m_bEOS
#define   g_pfnDecodePicture    stream_global->m_decode_picture

#define   g_pNewSlice           stream_global->m_pNewSlice

#define   g_pCCDec				stream_global->m_pATSCDec
#define   g_pArrangedPCCCode		stream_global->m_pArrangedPCCCode

#define   g_pH264DXVA           stream_global->m_pH264DXVA

#define   g_bSkipFirstB         stream_global->m_bSkipFirstB

#define   g_bDisplayed          stream_global->m_bDisplayed

#define   g_pbYCCBuffer         stream_global->m_pbYCCBuffer
#define   g_dwYCCBufferLen      stream_global->m_dwYCCBufferLen

#endif
//~KevinChien

#ifdef _COLLECT_PIC_
CREL_RETURN malloc_new_slice(Slice**  ppnewSlice);
void     free_new_slice(Slice *currSlice);
void Check_MultiThreadModel PARGS0 ();
CREL_RETURN	 read_new_picture PARGS1(int *header);
//int      decode_picture PARGS0();
unsigned __stdcall decode_picture_read_ip(void *parameters);
unsigned __stdcall decode_picture_read_b0(void *parameters);
unsigned __stdcall decode_picture_read_b1(void *parameters);

unsigned __stdcall decode_picture_RB(void *parameters);
#endif


void malloc_slice PARGS0();
//void free_slice PARGS0();

//int  decode_one_frame();
CREL_RETURN  init_picture PARGS0();
CREL_RETURN exit_picture PARGS0();

#if !defined (_COLLECT_PIC_)
int  read_new_slice PARGS0();
#endif
CREL_RETURN read_one_slice PARGS0();

#ifdef _COLLECT_PIC_
CREL_RETURN start_macroblock_MBAff PARGS0();
CREL_RETURN start_macroblock_NonMBAff PARGS0();
#else
CREL_RETURN start_macroblock PARGS0();
#endif

#ifdef _COLLECT_PIC_
CREL_RETURN  read_one_macroblock_UVLC_I_odd PARGS0();
CREL_RETURN  read_one_macroblock_UVLC_P_odd PARGS0();
CREL_RETURN  read_one_macroblock_UVLC_B_odd PARGS0();
CREL_RETURN  read_one_macroblock_UVLC_I_even PARGS0();
CREL_RETURN  read_one_macroblock_UVLC_P_even PARGS0();
CREL_RETURN  read_one_macroblock_UVLC_B_even PARGS0();

CREL_RETURN  read_one_macroblock_CABAC_I_odd PARGS0();
CREL_RETURN  read_one_macroblock_CABAC_P_odd PARGS0();
CREL_RETURN  read_one_macroblock_CABAC_B_odd PARGS0();
CREL_RETURN  read_one_macroblock_CABAC_I_even PARGS0();
CREL_RETURN  read_one_macroblock_CABAC_P_even PARGS0();
CREL_RETURN  read_one_macroblock_CABAC_B_even PARGS0();
#else
//int  read_one_macroblock_UVLC PARGS0();
CREL_RETURN  read_one_macroblock_UVLC_I PARGS0();
CREL_RETURN  read_one_macroblock_UVLC_P PARGS0();
CREL_RETURN  read_one_macroblock_UVLC_B PARGS0();

//int  read_one_macroblock_CABAC PARGS0();
CREL_RETURN  read_one_macroblock_CABAC_I PARGS0();
CREL_RETURN  read_one_macroblock_CABAC_P PARGS0();
CREL_RETURN  read_one_macroblock_CABAC_B PARGS0();
#endif

//extern int (*read_one_macroblock) PARGS0();

int  decode_one_macroblock_P PARGS0();
CREL_RETURN  decode_one_macroblock_I PARGS0();
CREL_RETURN  decode_one_macroblock_P_0 PARGS0();
CREL_RETURN  decode_one_macroblock_P_1 PARGS0();
CREL_RETURN  decode_one_macroblock_B_0 PARGS0();
CREL_RETURN  decode_one_macroblock_B_1 PARGS0();

CREL_RETURN  decode_one_macroblock_I_FMO PARGS0();
CREL_RETURN  decode_one_macroblock_P_0_FMO PARGS0();
CREL_RETURN  decode_one_macroblock_P_1_FMO PARGS0();

CREL_RETURN  exit_macroblock PARGS2(int eos_bit, BOOL* slice_end);
CREL_RETURN  exit_macroblock_FMO PARGS2(int eos_bit, BOOL* slice_end);
void decode_ipcm_mb PARGS4(imgpel * imgY, int stride, imgpel * imgUV, int stride_UV);

#if (!defined(__linux__)) && defined(_HW_ACCEL_)
void*  SW_Open PARGS4(int nframe, int iVGAType, int iDXVAMode, ICPService *m_pIviCP);
#else
void*  SW_Open PARGS3(int nframe, int iVGAType, int iDXVAMode);
#endif

int  SW_BeginDecodeFrame PARGS0();
int  SW_EndDecodeFrame PARGS0();
int  SW_ReleaseDecodeFrame PARGS1(int frame_index);
int  SW_Close PARGS0();

void CheckAvailabilityOfNeighbors_NonMBAff_Row PARGS0();
void CheckAvailabilityOfNeighbors_MBAff_Row PARGS0();
void CheckAvailabilityOfNeighbors_NonMBAff_Plane PARGS0();
void CheckAvailabilityOfNeighbors_NonMBAff_Plane_FMO PARGS0();
void CheckAvailabilityOfNeighbors_MBAff_Plane PARGS0();
void CheckAvailabilityOfNeighbors_NonMBAff_Row_FMO PARGS0();

CREL_RETURN readMotionInfoFromNAL PARGS0();
void readCBPandCoeffsFromNAL_UVLC PARGS0();
void readCBPandCoeffsFromNAL_CABAC PARGS0();
void readIPCMcoeffsFromNAL_UVLC PARGS0();
CREL_RETURN readIPCMcoeffsFromNAL_CABAC PARGS0();
CREL_RETURN readRefInfo_UVLC PARGS3(int flag_mode, int list, Pred_s_info *info);
CREL_RETURN readRefInfo_CABAC PARGS3(int flag_mode, int list, Pred_s_info *info);
CREL_RETURN read_ipred_modes_MbAFF PARGS0();
CREL_RETURN read_ipred_modes_NonMbAFF PARGS0();

void ReadMotionInfo16x16 PARGS2(Pred_s_info *info, int list);
void ReadMotionInfo16x8 PARGS2(Pred_s_info *info, int list);
void ReadMotionInfo8x16 PARGS2(Pred_s_info *info, int list);
void ReadMotionInfo8x8 PARGS2(Pred_s_info *info, int list);
CREL_RETURN mb_direct_spatial_mv_pred PARGS3(int current_mb_nr, Macroblock *currMB, Macroblock_s *currMB_s);
CREL_RETURN mb_direct_temporal_mv_pred PARGS3(int current_mb_nr, Macroblock *currMB, Macroblock_s *currMB_s);

void readLumaCoeff8x8_CABAC ();

CREL_RETURN  intrapred PARGS2(int ioff,int joff);
CREL_RETURN  intrapred_luma_16x16 PARGS1(int predmode);
CREL_RETURN intrapred_chroma PARGS0();

// NAL functions TML/CABAC bitstream
int  uvlc_startcode_follows PARGS1(int dummy);
// void free_Partition PARGS0();


int  is_new_picture PARGS0();
void init_old_slice PARGS0();

// dynamic mem allocation
CREL_RETURN init_global_buffers PARGS0();
void free_global_buffers PARGS0();

CREL_RETURN read_slice PARGS1(int header);

CREL_RETURN RBSPtoSODB(byte *streamBuffer, int *last_byte_pos);
int EBSPtoRBSP(byte *streamBuffer, int end_bytepos, int begin_bytepos);

void fill_wp_params PARGS0();
void reset_wp_params PARGS0();

void FreePartition (DecodingEnvironment *dep);
DecodingEnvironment *AllocPartition PARGS1(int n);

void tracebits2(const char *trace_str, int len, int info);

CREL_RETURN init_decoding_engine_IPCM();

void set_mb_comp_function_pointers();

void set_4xH_mc_function_ptr PARGS0();
void set_2xH_mc_function_ptr PARGS0();

#endif

// For Q-matrix
void AssignQuantParam PARGS2(pic_parameter_set_rbsp_t* pps, seq_parameter_set_rbsp_t* sps);
void CalculateQuantParam PARGS0();
void CalculateQuant8Param PARGS0();

#if !defined(_COLLECT_PIC_)
int DecodePicture_SingleThread PARGS0();
#else
void DecodeProcessBB PARGS0();
void DecodeProcessRB PARGS0();
BOOL CheckSkipAndPTSProcess PARGS1(BOOL bIsForReference);
void RearrangePBCCBuf PARGS0();
void SetH264CCCode PARGS0();

int DecodePicture_MultiThread_SingleSlice_IPRD_Merge PARGS0();
int DecodePicture_MultiThread_SingleSlice_IPRD_Seperate PARGS0();

int DecodePicture_MultiThread_MultiSlice PARGS0();
int DecodePicture_HW_BA PARGS0();
#endif

CREL_RETURN output_one_frame_from_dpb PARGS1(unsigned int view_index);

void __cdecl H264Debug(const char *szFormat, ...);

#ifdef __cplusplus
}
#endif
