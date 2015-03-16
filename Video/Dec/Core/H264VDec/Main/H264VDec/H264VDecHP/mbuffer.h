#ifndef __H264_MBUFFER_H__
#define __H264_MBUFFER_H__
/*!
***********************************************************************
*  \file
*      mbuffer.h
*
*  \brief
*      Frame buffer functions
*
*  \author
*      Main contributors (see contributors.h for copyright, address and affiliation details)
*      - Karsten Sühring          <suehring@hhi.de>
***********************************************************************
*/

#ifdef __cplusplus
//extern "C" {
#endif

#ifndef _MBUFFER_H_
#define _MBUFFER_H_

#include "global.h"


CREL_RETURN get_smallest_poc PARGS3(int *poc,int * pos, unsigned int view_index);

//! definition a picture (field or frame)
// Frame is extended by 32 pixels (Y component) and 16 pixels (U/V components) on left/right boundary
// and by 40 pixels (Y component) and 20 pixels (U/V components) on top/bottom boundary
// meaning that we can reference from (-32,-40) to (size_x+31,size_y+39) for luminance values
// and (-16,-20) to (size_x_cr+15,size_y_cr+19) for chrominance values
#define BOUNDARY_EXTENSION_X 64
#define BOUNDARY_EXTENSION_Y 40
// Force aligned memory allocation for frame
#define MEMORY_ALIGNMENT 64
typedef struct storable_picture
{
	Macroblock_s  *mb_data;                //!< array containing all MBs of a whole frame

	PictureStructure structure;
	int         unique_id;
	int			mem_layout;

	int         poc;
	int         top_poc;
	int         bottom_poc;
	int         frame_poc;
	int         order_num;
#ifdef DO_REF_PIC_NUM
	int64       ref_pic_num        [MAX_NUM_SLICES][6][MAX_LIST_SIZE];
	int64       frm_ref_pic_num    [MAX_NUM_SLICES][6][MAX_LIST_SIZE];
	int64       top_ref_pic_num    [MAX_NUM_SLICES][6][MAX_LIST_SIZE];
	int64       bottom_ref_pic_num [MAX_NUM_SLICES][6][MAX_LIST_SIZE];
#endif
	unsigned    frame_num;
	int         pic_num;
	int         long_term_pic_num;
	int         long_term_frame_idx;

	byte        is_long_term;
	byte        used_for_reference;
	byte        is_output;
	byte        non_existing;

	short       max_slice_id;

	short       size_x, size_y, size_x_cr, size_y_cr;
	//char        chroma_vector_adjustment;
	byte        coded_frame;
	byte        MbaffFrameFlag;
	short       PicWidthInMbs;
	short       PicSizeInMbs;

	short        Y_stride;
	short        UV_stride;


	imgpel *     FrameBuffer;   //!< Allocated buffer
	imgpel *     imgY;          //!< Y picture component
	imgpel *     imgUV;      //!< U and V picture components 


	imgpel *pDownSampleY;
	imgpel *pDownSampleUV;

	struct storable_picture *top_field;     // for mb aff, if frame for referencing the top field
	struct storable_picture *bottom_field;  // for mb aff, if frame for referencing the bottom field
	struct storable_picture *frame;         // for mb aff, if field for referencing the combined frame

	int         slice_type;
	int         idr_flag;
	int         no_output_of_prior_pics_flag;
	int         long_term_reference_flag;
	int         adaptive_ref_pic_buffering_flag;

	int         chroma_format_idc;
	int         frame_mbs_only_flag;
	int         frame_cropping_flag;
	int         frame_cropping_rect_left_offset;
	int         frame_cropping_rect_right_offset;
	int         frame_cropping_rect_top_offset;
	int         frame_cropping_rect_bottom_offset;
	int         qp;
	int         chroma_qp_offset[2];
	int         slice_qp_delta;
	DecRefPicMarking_t *dec_ref_pic_marking_buffer;              //!< stores the memory management control operations

	int		  pic_store_idx;
	unsigned	  long framerate1000;
	int         nAspectRatio;
	DWORD       dwXAspect;
	DWORD       dwYAspect;
	DWORD       dwBitRate;
	unsigned int progressive_frame;
	H264_TS     pts;
	int         has_pts;
	int         NumClockTs;
	unsigned int SkipedBFrames[MAX_NUM_VIEWS][2];

#ifdef _COLLECT_PIC_
	int used_for_first_field_reference;
#endif

	H264_CC m_CCCode;

	BYTE  pbYCCBuffer[12];
	DWORD dwYCCBufferLen;

	int  pull_down_flag;
	int  repeat_first_field;
	int  top_field_first;
	int  decoded_mb_number;	//For FMO only.
	int  has_PB_slice;
#if defined(_HW_ACCEL_)
	long    m_DXVAVer;
	BYTE*   m_lpRESD_Intra_Luma;
	BYTE*   m_lpRESD_Intra_Chroma;
	BYTE*   m_lpRESD_Inter_Luma;
	BYTE*   m_lpRESD_Inter_Chroma;
#endif
	int		view_id;
	int		view_index;
	BOOL	inter_view_flag;
} StorablePicture;




CREL_RETURN      init_dpb PARGS1(unsigned int view_index);
void             free_dpb PARGS1(unsigned int view_index);
FrameStore*      alloc_frame_store();
void             free_frame_store PARGS2(FrameStore* f, int pic_store_index);
StorablePicture* alloc_storable_picture PARGS9(PictureStructure type, 
										int size_x, 
										int size_y, 
										int size_x_cr, 
										int size_y_cr, 
										StorablePicture *p, 
										int memory_layout,
										int nMode,
										int non_exist);

void             free_storable_picture(StorablePicture* p, bool bFreeStruct = true);
CREL_RETURN      store_picture_in_dpb PARGS1(StorablePicture* p);
CREL_RETURN      flush_dpb PARGS1(unsigned int view_index);
void			 update_ref_list PARGS1(unsigned int view_index);
void			 update_ltref_list PARGS1(unsigned int view_index);

CREL_RETURN		 dpb_split_field_temporal PARGS1(FrameStore *fs);
CREL_RETURN		 dpb_split_field_spatial PARGS1(FrameStore *fs);

CREL_RETURN      dpb_combine_field PARGS1(FrameStore *fs);
CREL_RETURN      dpb_combine_field_yuv PARGS1(FrameStore *fs);

CREL_RETURN      init_lists PARGS2(int currSliceType, PictureStructure currPicStructure);
void             reorder_ref_pic_list PARGS8(StorablePicture **list, int *list_size, 
											int num_ref_idx_lX_active_minus1, int *remapping_of_pic_nums_idc, 
											int *abs_diff_pic_num_minus1, int *long_term_pic_idx, int *abs_diff_view_idx_minus1, int listidx);

void             init_mbaff_lists PARGS0();
void		     alloc_ref_pic_list_reordering_buffer PARGS1(Slice *currSlice);
void             free_ref_pic_list_reordering_buffer(Slice *currSlice);

CREL_RETURN      fill_frame_num_gap PARGS0();

ColocatedParamsMB* alloc_colocatedMB(int size_x, int size_y,int mb_adaptive_frame_field_flag);
void compute_colocated_SUBMB_frame_spatial PARGS6(ColocatedParamsMB* p,
																									StorablePicture **list[6],
																									int start_y,
																									int start_x,
																									int loop_y,
																									int loop_x);
void compute_colocated_SUBMB_frame_temporal PARGS6(ColocatedParamsMB* p,
																									 StorablePicture **list[6],
																									 int start_y,
																									 int start_x,
																									 int loop_y,
																									 int loop_x);
void compute_colocated_SUBMB_field_spatial PARGS6(ColocatedParamsMB* p,
																									StorablePicture **list[6],
																									int start_y,
																									int start_x,
																									int loop_y,
																									int loop_x);
void compute_colocated_SUBMB_field_temporal PARGS6(ColocatedParamsMB* p,
																									 StorablePicture **list[6],
																									 int start_y,
																									 int start_x,
																									 int loop_y,
																									 int loop_x);
void free_colocated_MB(ColocatedParamsMB* p);
void calc_mvscale PARGS0();

#endif

void free_picture_store();
void release_storable_picture PARGS2(StorablePicture* p, BYTE flag);
StorablePicture* get_storable_picture PARGS7(PictureStructure structure,
									  int size_x,
									  int size_y,
									  int size_x_cr,
									  int size_y_cr,
									  int mem_layout,
									  int non_exist);

#ifdef __cplusplus
//}

#endif

#endif
