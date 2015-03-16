
/**
* @file vssh_types.h
* Types definitions used in all other code.
*
* - Copyright (c) 2002-2004 Videosoft, Inc.
* - Project:	Videosoft H.264 Codec
* - Module:	Decoder and Encoder
*/


#ifndef __VSSH_TYPES_H__
#define __VSSH_TYPES_H__

#ifdef __cplusplus
//extern "C" {
#endif

#ifdef WIN32
#define VSSHAPI __cdecl
#else
#define VSSHAPI
#endif

// API Version info
/** 
* Obtain library version info (major.minor.rev.build, i.e. 2.2.3.2);
* @param magic pointer to a variable to receive magic API number;
* @param major pointer to a variable to receive major version number;
* @param minor pointer to a variable to receive minor version number;
* @param rev   pointer to a variable to receive revision number;
* @param build pointer to a variable to receive build number;
* @return VSSH_OK
*/
int vssh_get_version(int *magic, int *major, int *minor, int *rev, int *build);

/** @defgroup defs_err_codes Definitions and Error Codes
* @{
*/

/* function return codes */
/** @name Error codes
*/

// @{
/// Everything is OK
#define VSSH_OK 0

/// Warning: data is not ready yet
#define VSSH_WARN_NOT_READY 1

/// Error: general fault
#define VSSH_ERR_GENERAL -1

/// Error: not enough memory
#define VSSH_ERR_MEMORY -2

/// Error: wrong function argument value
#define VSSH_ERR_ARG -3

/// Warning: evaluation expired
#define VSSH_ERR_EXPIRED -4

/// Warning: frame skipped
#define VSSH_ERR_SKIP_FRAME -5

/// Error: invalid input frame dimensions specified
#define VSSH_ERR_FRAME_DIMENSIONS -6

/// Error: invalid encoder settings specified
#define VSSH_ERR_SETTINGS -7

/// Error: more input data needed to perform operation
#define VSSH_ERR_MORE_DATA -8

/// Error: file operation failed
#define VSSH_ERR_FILE -9

/// Error in stream prediction type
#define VSSH_ERR_STREAM_PRED_TYPE -10

/// Error in stream: direct params
#define VSSH_ERR_STREAM_DIRECT -11

/// Error in stream: try to read after end of stream
#define VSSH_ERR_STREAM_EOS -12

/// Error in stream: error in CAVLC 
#define VSSH_ERR_STREAM_CAVLC -13

/// Error in stream: error in REF_FRAME_NO 
#define VSSH_ERR_STREAM_REF_FRAME_NO -14

/// Error in stream: error in CBP 
#define VSSH_ERR_STREAM_CBP -15

/// Error in stream: error in subdiv type 
#define VSSH_ERR_STREAM_SUBDIV_TYPE -16

/// Error in stream: error in sps type 
#define VSSH_ERR_STREAM_SPS -17

/// Error in stream: error in mb type 
#define VSSH_ERR_STREAM_MB_TYPE -18

/// Error in stream: error in subdiv 8x8 type 
#define VSSH_ERR_STREAM_SUBDIV_8X8_TYPE -19

/// Error in stream: slice before sps or pps 
#define VSSH_ERR_STREAM_SLICE_BEFORE_SPS_OR_PPS -20

//This is a number of first fatal for whole stream

#define VSSH_ERR_FIRST_FATAL -100

#define VSSH_ERR_NOT_SUPPORTED_ADAPTIVE_REF_PIC_BUFFERING_FLAG  -100
#define VSSH_ERR_NOT_SUPPORTED_SLICE_GROUPS -101 
#define VSSH_ERR_NOT_SUPPORTED_ADAPTIVE_FRAME_FIELD -102
#define VSSH_ERR_NOT_SUPPORTED_FRAMES_AND_FIELDS_MUX -103
#define VSSH_ERR_NOT_SUPPORTED_SLICE_TYPE  -104
#define VSSH_ERR_NOT_SUPPORTED_REF_PICS_REORDERING  -105
#define VSSH_ERR_NOT_SUPPORTED_PIC_TIMING_PIC_STRUCT -106
// @}
/** @} */

/* general data type representation */
#ifdef byte
#undef byte
#endif
typedef unsigned char byte;					  //!<  8 bit unsigned

#ifdef sword
#undef sword
#endif

#ifdef uword
#undef uword
#endif

typedef	short sword;
typedef unsigned short uword;


#if defined(WIN32) || defined(_WIN32_WCE)
typedef __int64	  timestamp_t;
#else
typedef long long timestamp_t;
#endif

/** @addtogroup defs_err_codes */
// @{

/// Types of slices
typedef enum
{
	P_SLICE = 0,
	B_SLICE,
	I_SLICE,
	SP_SLICE,
	SI_SLICE,
	NUMBER_SLICETYPES
} slice_type_e;

// @}

/// Generic frame information
typedef struct
{
	int	slice_type;	///< type of the slice (see slice_type_e)
	int	frame_num;	///< logical number of the frame
	int	slice_num;	///< logical number of the slice
	int	idr_flag;	///< 1=IDR (key) frame, 0=regular frame;
	int	qp_used;	///< qp used for the frame
	int	num_intra_mb;	///< number of intra mbs in encoded frame
	int	num_bits;	///< number of encoded bits in slice
	int	profile_idc;	///< profile IDC (see profile_idc_e);
	int	error_no;	//<<!=0 indicates that some error was detetecting in procedding
	timestamp_t timestamp;	///< timestamp based on 10kHz frequency
	int	is_sei;		///< 1=sei message NAL unit;
} frame_info_t;

/// YUV pixel domain image arrays for uncompressed video frame
typedef struct
{
	int	width;		///< frame (allocated) buffer width in bytes, always dividible by 16;
	int	height;		///< frame (allocated) buffer height in bytes;
	int	image_width;	///< actual image width in pixels, less or equal to buffer width;
	int	image_height;	///< actual image height in pixels, less or equal to buffer height;
	byte	*y;		///< pointer to Y component data
	byte	*u;		///< pointer to U component data
	byte	*v;		///< pointer to V component data
	frame_info_t info; 	///< frame info
} yuv_frame_t;

/** @addtogroup defs_err_codes */
// @{

/// Typical values for profile IDC
typedef enum
{
	PROFILE_IDC_BASELINE = 66,
	PROFILE_IDC_MAIN     = 77,
	PROFILE_IDC_EXTENDED = 88
} profile_idc_e;

/// Typical values for level IDC
typedef enum
{
	LEVEL_IDC_12 = 12,
	LEVEL_IDC_32 = 32
} level_idc_e;

/// Interlace mode
typedef enum
{
	INTERLACE_MODE_NONE = 0,
	INTERLACE_MODE_ALL_FIELD_TOP_FIRST = 1,
	INTERLACE_MODE_ALL_FIELD_BOTTOM_FIRST = 2
} interlace_mode_e;

// @}

/// Video usability information (see JVT-g050r1)
typedef struct vui_info_t
{
	unsigned short sar_width;	///< aspect ratio width;
	unsigned short sar_height;	///< aspect ration height;
	unsigned int num_units_in_tick; ///< frame rate divider;
	unsigned int time_scale;	///< fps = time_scale/num_units_in_tick;
	unsigned char fixed_frame_rate_flag;	///< 0/1;
	//< To be expanded...
} vui_info_t;


/// Cropping information of the allocated yuv frames
typedef struct cropping_info_t
{
	int luma_offset;	///< offset in pixels of actual luma data;
	int chroma_offset;	///< offset in pixels of actual chroma data;
	int frame_width;	///< actual frame width;
	int frame_height;	///< actual frame height;
} cropping_info_t;

/// Sequence Parameter Set (SPS) information
typedef struct
{
	int	profile_idc;	///< profile idc
	int	level_idc;	///< level
	int	is_interlace;	///< 0 - progresive; 1 - interlace
	int	allocated_frame_width;	///< frame width in pixels
	int	allocated_frame_height;	///< frame height in pixels
	cropping_info_t cropping_info;	///< cropping info from sps header
	vui_info_t vui_info;	///< extra info (fps and resize can obtained from here);
	int	error_no;  //<<!=0 indicates that some error was detetecting in SPS
} sps_info_t;

#ifdef __cplusplus
//}
#endif

#endif //__VSSH_TYPES_H__
