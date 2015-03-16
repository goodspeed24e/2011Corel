
/*!
***********************************************************************
*  \file
*      mbuffer.c
*
*  \brief
*      Frame buffer functions
*
*  \author
*      Main contributors (see contributors.h for copyright, address and affiliation details)
*      - Karsten SÅEring          <suehring@hhi.de>
*      - Alexis Tourapis                 <alexismt@ieee.org>
***********************************************************************
*/
#include <windows.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include <string.h>
#include <malloc.h>
#include <math.h>

#include "global.h"
#include "mbuffer.h"
#include "memalloc.h"
#include "output.h"
#include "image.h"
#include "header.h"
#include "defines.h"
#include "videoframe.h"
#ifdef H264_ENABLE_INTRINSICS
#include <mmintrin.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#define _XMM_COPY_
#endif

#include "parset.h"

static CREL_RETURN insert_picture_in_dpb PARGS2(FrameStore* fs, StorablePicture* p);
int  is_used_for_reference(FrameStore* fs);
static int  remove_unused_frame_from_dpb PARGS1(unsigned int view_index);
static int  is_short_term_reference(FrameStore* fs);
static int  is_long_term_reference(FrameStore* fs);
void gen_field_ref_ids PARGS1(StorablePicture *p);
CREL_RETURN dpb_split_field_common PARGS1(FrameStore *fs);

/*!
************************************************************************
* \brief
*    Print out list of pictures in DPB. Used for debug purposes.
************************************************************************
*/

#ifdef _COLLECT_PIC_
#define stream_global  (img->stream_global)
#endif

#define DP H264Debug

#if 0
void dump_dpb()
{
	unsigned i;

	// return;

	for (i=0; i<dpb.used_size;i++)
	{
		DP("(");
		DP("fn=%d  ", dpb.fs[i]->frame_num);
		if (dpb.fs[i]->is_used & 1)
		{
			if (dpb.fs[i]->top_field)
				DP("T: poc=%d  ", dpb.fs[i]->top_field->poc);
			else
				DP("T: poc=%d  ", dpb.fs[i]->frame->top_poc);
		}
		if (dpb.fs[i]->is_used & 2)
		{
			if (dpb.fs[i]->bottom_field)
				DP("B: poc=%d  ", dpb.fs[i]->bottom_field->poc);
			else
				DP("B: poc=%d  ", dpb.fs[i]->frame->bottom_poc);
		}
		if (dpb.fs[i]->is_used == 3)
			DP("F: poc=%d  ", dpb.fs[i]->frame->poc);
		DP("G: poc=%d)  ", dpb.fs[i]->poc);
		if (dpb.fs[i]->is_reference) DP ("ref (%d) ", dpb.fs[i]->is_reference);
		if (dpb.fs[i]->is_long_term) DP ("lt_ref (%d) ", dpb.fs[i]->is_reference);
		if (dpb.fs[i]->is_output) DP ("out  ");
		if (dpb.fs[i]->is_used == 3)
		{
			if (dpb.fs[i]->frame->non_existing) DP ("ne  ");
		}
		DP ("\n");
	}
}
#endif

/*!
************************************************************************
* \brief
*    Returns the size of the dpb depending on level and picture size
*
*
************************************************************************
*/
CREL_RETURN getDpbSize PARGS2(unsigned int* dpb_size, unsigned int view_index)
{
	// int pic_size = (active_sps.pic_width_in_mbs_minus1 + 1) * (active_sps.pic_height_in_map_units_minus1 + 1) * (active_sps.frame_mbs_only_flag?1:2) * 384;
	seq_parameter_set_rbsp_t *latest_sps;

#if !defined(_COLLECT_PIC_)
	latest_sps = &active_sps;
#else
	latest_sps = stream_global->m_active_sps_on_view[0];		//All activated SPS on different views shall has same following settings
#endif
	int pic_size = (latest_sps->pic_width_in_mbs_minus1 + 1) * (latest_sps->pic_height_in_map_units_minus1 + 1) * (latest_sps->frame_mbs_only_flag?1:2) * 384;

	int num_views = 1;

	if(stream_global->bMVC == TRUE)
		num_views = stream_global->num_views;

	int size = 0;	//bytes

	switch (latest_sps->level_idc)
	{
	case 10:
		size = 152064;
		break;
	case 11:
		size = 345600;
		break;
	case 12:
		size = 912384;
		break;
	case 13:
		size = 912384;
		break;
	case 20:
		size = 912384;
		break;
	case 21:
		size = 1824768;
		break;
	case 22:
		size = 3110400;
		break;
	case 30:
		size = 3110400;
		break;
	case 31:
		size = 6912000;
		break;
	case 32:
		size = 7864320;
		break;
	case 40:
		size = 12582912;

		if(stream_global->m_bIsBD3D == TRUE && num_views > 1)
		{
			if(latest_sps->frame_mbs_only_flag == 1)
				size = 18874368;
		}
		break;
	case 41:
		size = 12582912;

		if(stream_global->m_bIsBD3D == TRUE && num_views > 1)
		{
			if(latest_sps->frame_mbs_only_flag == 1)
				size = 18874368;
		}
		break;
	case 42:
		size = 12582912;
		break;
	case 50:
		size = 42393600;
		break;
	case 51:
		size = 70778880;
		break;
	default:
		DEBUG_SHOW_ERROR_INFO ("[ERROR]undefined level", 500);
		*dpb_size = 0;
		return CREL_WARNING_H264_STREAMERROR_LEVEL_1|CREL_WARNING_H264_ERROR_SPS;    
	}

	if(num_views == 1 || (stream_global->m_bIsBD3D == TRUE && num_views > 1 && latest_sps->frame_mbs_only_flag == 1))
	{
		size = min( size / pic_size, 16);
	}
	else
	{
		int mvcScaleFactor = 2;

		double log2_num_views = (log((double)num_views)/log(2.0));

		int max_size = ((log2_num_views - (int)log2_num_views) > 0)? (((int)log2_num_views) + 1) * 16: (int)log2_num_views * 16;

		size = min( mvcScaleFactor *  size / pic_size, max( 1, max_size));
	}

	if (latest_sps->vui_parameters_present_flag && latest_sps->vui_seq_parameters.bitstream_restriction_flag)
	{
		if ((int)latest_sps->vui_seq_parameters.max_dec_frame_buffering > size)
		{
			DEBUG_SHOW_ERROR_INFO ("[ERROR]max_dec_frame_buffering larger than MaxDpbSize", 500);
			*dpb_size = 0;
			return CREL_WARNING_H264_STREAMERROR_LEVEL_1|CREL_WARNING_H264_ERROR_SPS;
		}
		size = max (1, latest_sps->vui_seq_parameters.max_dec_frame_buffering * num_views);
	}

	if(num_views > 1)
	{
		if(stream_global->m_bIsBD3D == TRUE)
		{
			//The upper limit size is 6 for frame_mbs_only_flag = 1 and the view number is 2.
			if(latest_sps->frame_mbs_only_flag == 1)
				size = min(size, 3);

			//The upper limit size is 4 in Dependent view.
			if(view_index > 0)
				size = min(size, 4);
		}
		else
			size = size / num_views;
	}
	

#if defined(_HW_ACCEL_)
	if(g_DXVAVer)
	{
		size = max(2, min( size, HW_BUF_SIZE-4));
		if (IMGPAR set_dpb_buffer_size) 
			size = 4;
	}
#endif

	*dpb_size = size;
	return CREL_OK;
}

/*!
************************************************************************
* \brief
*    Check then number of frames marked "used for reference" and break 
*    if maximum is exceeded
*
************************************************************************
*/
CREL_RETURN check_num_ref PARGS0()
{
	if ((int)(dpb.ltref_frames_in_buffer_on_view[IMGPAR currentSlice->viewIndex] +  dpb.ref_frames_in_buffer_on_view[IMGPAR currentSlice->viewIndex] ) > (max(1,dpb.num_ref_frames)))
	{
		DEBUG_SHOW_ERROR_INFO ("[ERROR]Max. number of reference frames exceeded. Invalid stream.", 500);
		return CREL_WARNING_H264_STREAMERROR_LEVEL_1|CREL_WARNING_H264_ERROR_DPB;
	}
	return CREL_OK;
}


/*!
************************************************************************
* \brief
*    Allocate memory for decoded picture buffer and initialize with sane values.
*
************************************************************************
*/
CREL_RETURN init_dpb PARGS1(unsigned int view_index)
{
	unsigned i;
	CREL_RETURN ret;

	//only set storable_picture_count as zero when start playing, otherwise keep the original value of storable_picture_count to avoid the crash
	if(storable_picture_count < 0) 
		storable_picture_count = 0;

	if(stream_global->num_views > stream_global->dpb_pre_alloc_views)
	{
		dpb.init_done = (int*)realloc(dpb.init_done, stream_global->num_views * sizeof(int));
		dpb.size_on_view = (unsigned *) _aligned_realloc(dpb.size_on_view, stream_global->num_views*sizeof (unsigned *), 16);
		dpb.used_size_on_view = (unsigned int *) _aligned_realloc(dpb.used_size_on_view, stream_global->num_views*sizeof (unsigned int *), 16);
		dpb.ref_frames_in_buffer_on_view = (unsigned *) _aligned_realloc(dpb.ref_frames_in_buffer_on_view, stream_global->num_views*sizeof (unsigned *), 16);
		dpb.fs_on_view = (frame_store ***) _aligned_realloc(dpb.fs_on_view, stream_global->num_views*sizeof (FrameStore**), 16);
		dpb.fs_ltref_on_view = (frame_store ***) _aligned_realloc(dpb.fs_ltref_on_view, stream_global->num_views*sizeof (FrameStore**), 16);
		dpb.fs_ref_on_view = (frame_store ***) _aligned_realloc(dpb.fs_ref_on_view, stream_global->num_views*sizeof (FrameStore**), 16);
		dpb.ltref_frames_in_buffer_on_view = (unsigned *) _aligned_realloc(dpb.ltref_frames_in_buffer_on_view, stream_global->num_views*sizeof (unsigned *), 16);
		dpb.max_long_term_pic_idx_on_view = (int*) _aligned_realloc(dpb.max_long_term_pic_idx_on_view, stream_global->num_views*sizeof (int), 16);
		dpb.last_picture = (frame_store **) _aligned_realloc(dpb.last_picture, stream_global->num_views*sizeof(FrameStore*), 16);

		for ( i = stream_global->dpb_pre_alloc_views; i < stream_global->num_views; i++)
		{
			dpb.init_done[i] = 0;
			dpb.size_on_view[i] = 0;
			dpb.used_size_on_view[i] = 0;
			dpb.ref_frames_in_buffer_on_view[i] = 0;
			dpb.fs_on_view[i] = NULL;
			dpb.fs_ltref_on_view[i] = NULL;
			dpb.fs_ref_on_view[i] = NULL;
			dpb.ltref_frames_in_buffer_on_view[i] = 0;
			dpb.max_long_term_pic_idx_on_view[i] = 0;
			dpb.last_picture[i] = NULL;
		}

		stream_global->dpb_pre_alloc_views = stream_global->num_views;
	}

	if (dpb.init_done != NULL && view_index < stream_global->dpb_pre_alloc_views && dpb.init_done[view_index])
	{
		free_dpb ARGS1(view_index);
	}

	ret  = getDpbSize ARGS2(&(dpb.size_on_view[view_index]), view_index);
	if (FAILED(ret)) {
		for ( i = 0; i < stream_global->num_views; i++) {
			stream_global->m_active_sps_on_view[i] = NULL;
		}
		return ret;
	}

#if !defined(_COLLECT_PIC_)
	dpb.num_ref_frames = active_sps.num_ref_frames;
	if (dpb.size < active_sps.num_ref_frames)
#else
	dpb.num_ref_frames = stream_global->m_active_sps_on_view[0]->num_ref_frames;
	//DPB configurations depends on base view

	if (!stream_global->bMVC && stream_global->m_active_sps_on_view[view_index]) {
		if (dpb.size_on_view[view_index] < stream_global->m_active_sps_on_view[view_index]->num_ref_frames)
#endif
		{
			DEBUG_SHOW_ERROR_INFO ("[ERROR]DPB size at specified level is smaller than the specified number of reference frames. This is not allowed.\n", 1000);
			//FIXME: actually we will exit program in above error(), but here we try to re-assign dpb_size and let program continue.
			dpb.size_on_view[view_index] = dpb.num_ref_frames;
			//return CREL_WARNING_H264_STREAMERROR_LEVEL_1|CREL_WARNING_H264_ERROR_DPB;
		}
	}

	dpb.used_size_on_view[view_index] = 0;

	dpb.ref_frames_in_buffer_on_view[view_index] = 0;
	dpb.ltref_frames_in_buffer_on_view[view_index] = 0;

	dpb.fs_on_view[view_index] = (frame_store **) _aligned_malloc(dpb.size_on_view[view_index]*sizeof (FrameStore*), 16);
	if (NULL==dpb.fs_on_view[view_index])  {
		no_mem_exit("init_dpb: dpb->fs");
		return CREL_ERROR_H264_NOMEMORY;
	}

	dpb.fs_ref_on_view[view_index] = (frame_store **) _aligned_malloc(dpb.size_on_view[view_index]*sizeof (FrameStore*), 16);
	if (NULL==dpb.fs_ref_on_view[view_index]) {
		no_mem_exit("init_dpb: dpb->fs_ref");
		return CREL_ERROR_H264_NOMEMORY;
	}

	dpb.fs_ltref_on_view[view_index] = (frame_store **) _aligned_malloc(dpb.size_on_view[view_index]*sizeof (FrameStore*), 16);
	if (NULL==dpb.fs_ltref_on_view[view_index]) {
		no_mem_exit("init_dpb: dpb->fs_ltref");
		return CREL_ERROR_H264_NOMEMORY;
	}

	for (i=0; i<dpb.size_on_view[view_index]; i++)
	{
		dpb.fs_on_view[view_index][i]       = alloc_frame_store();
		if (dpb.fs_on_view[view_index][i]== NULL) {
			return CREL_ERROR_H264_NOMEMORY;
		}
		dpb.fs_ref_on_view[view_index][i]   = NULL;
		dpb.fs_ltref_on_view[view_index][i] = NULL;
	}

	dpb.last_output_poc = INT_MIN;
	dpb.init_done[view_index] = 1;

#if !defined(_COLLECT_PIC_)
	int j;
	for (i=0; i<2; i++)
	{
		listX[i] = (storable_picture **) _aligned_malloc(MAX_LIST_SIZE_FRAME*sizeof (StorablePicture*), 16); // +1 for reordering
		if (NULL==listX[i]) 
			no_mem_exit("init_dpb: listX[i]");
	}

	for (i=2; i<6; i++)
	{
		listX[i] = (storable_picture **) _aligned_malloc(MAX_LIST_SIZE*sizeof (StorablePicture*), 16); // +1 for reordering
		if (NULL==listX[i]) 
			no_mem_exit("init_dpb: listX[i]");
	}

	for (j=0;j<2;j++)
	{
		for (i=0; i<MAX_LIST_SIZE_FRAME; i++)
		{
			listX[j][i] = NULL;
		}
		listXsize[j]=0;
	}

	for (j=2;j<6;j++)
	{
		for (i=0; i<MAX_LIST_SIZE; i++)
		{
			listX[j][i] = NULL;
		}
		listXsize[j]=0;
	}

	IMGPAR last_has_mmco_5 = 0;
#endif  
	return CREL_OK;
}


/*!
************************************************************************
* \brief
*    Free memory for decoded picture buffer.
************************************************************************
*/
void free_dpb PARGS1(unsigned int view_index)
{
	unsigned i;
	if (dpb.fs_on_view && dpb.fs_on_view[view_index])
	{
		for (i=0; i<dpb.size_on_view[view_index]; i++)
		{
			free_frame_store ARGS2(dpb.fs_on_view[view_index][i], -1);
		}
		_aligned_free (dpb.fs_on_view[view_index]);
		dpb.fs_on_view[view_index]=NULL;
		dpb.used_size_on_view[view_index] = 0;
	}
	if (dpb.fs_ref_on_view && dpb.fs_ref_on_view[view_index])
	{
		_aligned_free (dpb.fs_ref_on_view[view_index]);
		dpb.fs_ref_on_view[view_index] = NULL;
	}
	if (dpb.fs_ltref_on_view && dpb.fs_ltref_on_view[view_index])
	{
		_aligned_free (dpb.fs_ltref_on_view[view_index]);
		dpb.fs_ltref_on_view[view_index] = NULL;
	}
	dpb.last_output_poc = INT_MIN;	

#if !defined(_COLLECT_PIC_)
	for (i=0; i<6; i++)
		if (listX[i])
		{
			_aligned_free (listX[i]);
			listX[i] = NULL;
		}
		//#else
		//  for (int j=0; j<IMG_NUM; j++)
		//  {
		//	img = img_array[j];
		//	for (i=0; i<6; i++)
		//		if (listX[i])
		//		{
		//			_aligned_free (listX[i]);
		//			listX[i] = NULL;
		//		}
		//  }
#endif
	if(dpb.init_done)
		dpb.init_done[view_index] = 0;
}


/*!
************************************************************************
* \brief
*    Allocate memory for decoded picture buffer frame stores an initialize with sane values.
*
* \return
*    the allocated FrameStore structure
************************************************************************
*/
FrameStore* alloc_frame_store()
{
	FrameStore *f;

	f = (frame_store *) _aligned_malloc (sizeof(FrameStore), 16);
	if (f) {
		f->is_used      = 0;
		f->is_reference = 0;
		f->is_long_term = 0;
		f->is_orig_reference = 0;

		f->is_non_existent = 0;

		f->frame_num = 0;
		f->frame_num_wrap = 0;
		f->long_term_frame_idx = 0;
		f->is_output = 0;
		f->poc = 0;

		f->frame        = NULL;;
		f->top_field    = NULL;
		f->bottom_field = NULL;
	}  

	return f;
}

/*!
************************************************************************
* \brief
*    Allocate memory for a stored picture. 
*
* \param structure
*    picture structure
* \param size_x
*    horizontal luma size
* \param size_y
*    vertical luma size
* \param size_x_cr
*    horizontal chroma size
* \param size_y_cr
*    vertical chroma size
*
* \return
*    the allocated StorablePicture structure
************************************************************************
*/
StorablePicture *alloc_storable_picture PARGS9(PictureStructure structure, int size_x, int size_y, int size_x_cr, int size_y_cr, StorablePicture *s, int mem_layout, int nMode, int non_exist)
{
	int sz_img_Y;
	int sz_img_UV;
	int sz_img;
	unsigned int view_index = (IMGPAR firstSlice) ? (IMGPAR firstSlice->viewIndex) : 0;

	DEBUG_SHOW_SW_INFO("Allocating (%s) picture (x=%d, y=%d, x_cr=%d, y_cr=%d)\n", (structure == FRAME)?"FRAME":(structure == TOP_FIELD)?"TOP_FIELD":"BOTTOM_FIELD", size_x, size_y, size_x_cr, size_y_cr);

	if(s==0)
		s = (storable_picture *) _aligned_malloc (sizeof(StorablePicture), 16);
	if(s==0) 
		no_mem_exit("alloc_storable_picture: s");

	s->Y_stride = (size_x+2*BOUNDARY_EXTENSION_X+MEMORY_ALIGNMENT-1)&((int) ~(MEMORY_ALIGNMENT-1));
	sz_img_Y = (size_y+2*BOUNDARY_EXTENSION_Y)*s->Y_stride;

	if(active_sps.chroma_format_idc == YUV420)
	{
		s->UV_stride =s->Y_stride;
		sz_img_UV = (size_y_cr+2*(BOUNDARY_EXTENSION_Y/2))*s->UV_stride;
	}
	else
	{
		sz_img_UV = 0;
	}
	sz_img = sz_img_Y+sz_img_UV;


	if(structure!=FRAME)
	{
		size_y    >>= 1;
		size_y_cr >>= 1;
	}
	s->PicSizeInMbs = (size_x*size_y)>>8;

	//_USE_TR_
	//BYTE pByte = new BYTE[randomnumber];
	//delete [] pByte;

	if(mem_layout!=0)
	{
#if defined (_COLLECT_PIC_)
		//Yolk: workaround for downsample
		if (stream_global->m_resize_width_height && (size_x*size_y > 960*540))
			s->FrameBuffer = (imgpel *) _aligned_malloc(sz_img*sizeof(imgpel)*2,MEMORY_ALIGNMENT);
		else
#endif
			s->FrameBuffer = (imgpel *) _aligned_malloc(sz_img*sizeof(imgpel),MEMORY_ALIGNMENT);
	}
	else
	{
		s->FrameBuffer = NULL;
	}

	int nSeekFlag = g_framemgr->GetDisplayCount() + dpb.used_size_on_view[view_index];
	if(mem_layout==0 && ((IMGPAR smart_dec <= SMART_DEC_LEVEL_3 || IMGPAR smart_dec == SMART_DEC_LEVEL_0) && nSeekFlag && IMGPAR dwFillFrameNumGap))
	{
		s->imgY = NULL;
		s->imgUV = NULL;
		if(structure!=FRAME)
		{
			s->Y_stride  <<= 1;
			s->UV_stride <<= 1;
		}
	}
	else
	{
		// Set pointers
		s->imgY  = s->FrameBuffer;
		if (active_sps.chroma_format_idc != YUV400)
		{
			s->imgUV  = s->imgY + sz_img_Y;
			s->imgUV += (BOUNDARY_EXTENSION_Y/2)*s->UV_stride+BOUNDARY_EXTENSION_X;	
		}
		else
		{
			s->imgUV = NULL;
		}
		s->imgY += BOUNDARY_EXTENSION_Y*s->Y_stride+BOUNDARY_EXTENSION_X;
		// Set strides for FIELD structures and move pointers for BOTTOM_FIELD structures
		if(structure!=FRAME)
		{
			if(structure==BOTTOM_FIELD)
			{
				s->imgY     += s->Y_stride;
				s->imgUV	  += s->UV_stride;
			}
			s->Y_stride  <<= 1;
			s->UV_stride <<= 1;
		}
	}

#if defined (_COLLECT_PIC_)
	if ((s->FrameBuffer != NULL) && stream_global->m_resize_width_height && (size_x*size_y > 960*540))
	{
		s->pDownSampleY = s->FrameBuffer + sz_img;
		s->pDownSampleUV = s->pDownSampleY + sz_img_Y;
		s->pDownSampleUV += (BOUNDARY_EXTENSION_Y/2)*s->UV_stride+BOUNDARY_EXTENSION_X;
		s->pDownSampleY += BOUNDARY_EXTENSION_Y*s->Y_stride+BOUNDARY_EXTENSION_X;
	}
	else
#endif
		s->pDownSampleY = s->pDownSampleUV = NULL;

	if (non_exist == 0) {
		if((s->mb_data = (Macroblock_s *) _aligned_malloc(s->PicSizeInMbs*sizeof(Macroblock_s),16)) == NULL)
			no_mem_exit("alloc_storable_picture: s->mb_data");
	} else {
		s->mb_data = NULL;
	}

	s->pic_num             = 0;
	s->frame_num           = 0;
	s->long_term_frame_idx = 0;
	s->long_term_pic_num   = 0;
	s->used_for_reference  = 0;
	s->is_long_term        = 0;
	s->non_existing        = 0;
	s->is_output           = 0;
	s->max_slice_id        = 0;

	s->structure  = structure;
	s->mem_layout = mem_layout;

	s->size_x = size_x;
	s->size_y = size_y;
	s->size_x_cr = size_x_cr;
	s->size_y_cr = size_y_cr;


	s->top_field    = NULL;
	s->bottom_field = NULL;
	s->frame        = NULL;

	s->dec_ref_pic_marking_buffer = NULL;

	s->coded_frame    = 0;
	s->MbaffFrameFlag = 0;

	if ((nMode == 0) && (non_exist == 0)) {
		s->unique_id = storable_picture_count;
		storable_picture_map[storable_picture_count++] = s;
	}

#if defined(_HW_ACCEL_)
	if (g_DXVAVer==IviDxva2 && (g_DXVAMode==E_H264_DXVA_NVIDIA_PROPRIETARY_A || g_DXVAMode==E_H264_DXVA_MODE_A))
	{
		s->m_lpRESD_Intra_Luma = (BYTE*)_aligned_malloc(8160 * 512, 16);
		s->m_lpRESD_Intra_Chroma = (BYTE*)_aligned_malloc(8160 * 256, 16);
		s->m_lpRESD_Inter_Luma = (BYTE*)_aligned_malloc(8160 * 512, 16);
		s->m_lpRESD_Inter_Chroma = (BYTE*)_aligned_malloc(8160 * 256, 16);
	} 
	else
	{
		s->m_lpRESD_Intra_Luma = 0;
		s->m_lpRESD_Intra_Chroma = 0;
		s->m_lpRESD_Inter_Luma = 0;
		s->m_lpRESD_Inter_Chroma = 0;
	}  
#endif


	return s;
}

/*!
************************************************************************
* \brief
*    Free frame store memory.
*
* \param f
*    FrameStore to be freed
*
************************************************************************
*/
void free_frame_store PARGS2(FrameStore* f, int pic_store_index)
{
	
	if (f)
	{
		if (f->frame)
		{
			release_storable_picture ARGS2(f->frame,(f->frame->pic_store_idx == pic_store_index)?0:1);
			f->frame=NULL;
		}
		if (f->top_field)
		{
			release_storable_picture ARGS2(f->top_field,0);
			f->top_field=NULL;
		}
		if (f->bottom_field)
		{
			release_storable_picture ARGS2(f->bottom_field,0);
			f->bottom_field=NULL;
		}
		_aligned_free(f);
	}
}

/*!
************************************************************************
* \brief
*    Free picture memory.
*
* \param p
*    Picture to be freed
*
************************************************************************
*/

void free_storable_picture(StorablePicture* p, bool bFreeStruct)
{
	if(!p)
		return;

	if(p->mb_data)
	{
		_aligned_free(p->mb_data);
		p->mb_data = NULL;
	}

	if (p->FrameBuffer)
	{
		_aligned_free((void *)p->FrameBuffer);
		p->FrameBuffer=NULL;
	}
	p->imgUV = NULL;
	p->imgY = NULL;

	p->pDownSampleY = p->pDownSampleUV = NULL;

#if defined(_HW_ACCEL_)
	if (p->m_lpRESD_Intra_Luma)
	{
		_aligned_free(p->m_lpRESD_Intra_Luma);
		p->m_lpRESD_Intra_Luma=NULL;
	}
	if (p->m_lpRESD_Intra_Chroma)
	{
		_aligned_free(p->m_lpRESD_Intra_Chroma);
		p->m_lpRESD_Intra_Chroma=NULL;
	}
	if (p->m_lpRESD_Inter_Luma)
	{
		_aligned_free(p->m_lpRESD_Inter_Luma);
		p->m_lpRESD_Inter_Luma=NULL;
	}
	if (p->m_lpRESD_Inter_Chroma)
	{
		_aligned_free(p->m_lpRESD_Inter_Chroma);
		p->m_lpRESD_Inter_Chroma=NULL;
	}
#endif

	if(bFreeStruct)
	{
		_aligned_free(p);
		p = NULL;
	}
}

/*!
************************************************************************
* \brief
*    mark FrameStore unused for reference
*
************************************************************************
*/
static void unmark_for_reference(FrameStore* fs)
{

	if (fs->is_used & TOP_FIELD)
	{
		if (fs->top_field)
		{
			fs->top_field->used_for_reference = 0;
		}
	}
	if (fs->is_used & BOTTOM_FIELD)
	{
		if (fs->bottom_field)
		{
			fs->bottom_field->used_for_reference = 0;
		}
	}
	if (fs->is_used == (TOP_FIELD|BOTTOM_FIELD))
	{
		if (fs->top_field && fs->bottom_field)
		{
			fs->top_field->used_for_reference = 0;
			fs->bottom_field->used_for_reference = 0;
		}
		fs->frame->used_for_reference = 0;
	}

	fs->is_reference = 0;
}


/*!
************************************************************************
* \brief
*    mark FrameStore unused for reference and reset long term flags
*
************************************************************************
*/
static void unmark_for_long_term_reference(FrameStore* fs)
{

	if (fs->is_used & TOP_FIELD)
	{
		if (fs->top_field)
		{
			fs->top_field->used_for_reference = 0;
			fs->top_field->is_long_term = 0;
		}
	}
	if (fs->is_used & BOTTOM_FIELD)
	{
		if (fs->bottom_field)
		{
			fs->bottom_field->used_for_reference = 0;
			fs->bottom_field->is_long_term = 0;
		}
	}
	if (fs->is_used == (TOP_FIELD|BOTTOM_FIELD))
	{
		if (fs->top_field && fs->bottom_field)
		{
			fs->top_field->used_for_reference = 0;
			fs->top_field->is_long_term = 0;
			fs->bottom_field->used_for_reference = 0;
			fs->bottom_field->is_long_term = 0;
		}
		fs->frame->used_for_reference = 0;
		fs->frame->is_long_term = 0;
	}

	fs->is_reference = 0;
	fs->is_long_term = 0;
}


/*!
************************************************************************
* \brief
*    compares two stored pictures by picture number for qsort in descending order
*
************************************************************************
*/
static int __cdecl compare_pic_by_pic_num_desc( const void *arg1, const void *arg2 )
{
	if ( (*(StorablePicture**)arg1)->pic_num < (*(StorablePicture**)arg2)->pic_num)
		return 1;
	if ( (*(StorablePicture**)arg1)->pic_num > (*(StorablePicture**)arg2)->pic_num)
		return -1;
	else
		return 0;
}

/*!
************************************************************************
* \brief
*    compares two stored pictures by picture number for qsort in descending order
*
************************************************************************
*/
static int __cdecl compare_pic_by_lt_pic_num_asc( const void *arg1, const void *arg2 )
{
	if ( (*(StorablePicture**)arg1)->long_term_pic_num < (*(StorablePicture**)arg2)->long_term_pic_num)
		return -1;
	if ( (*(StorablePicture**)arg1)->long_term_pic_num > (*(StorablePicture**)arg2)->long_term_pic_num)
		return 1;
	else
		return 0;
}

/*!
************************************************************************
* \brief
*    compares two frame stores by pic_num for qsort in descending order
*
************************************************************************
*/
static int __cdecl compare_fs_by_frame_num_desc( const void *arg1, const void *arg2 )
{
	if ( (*(FrameStore**)arg1)->frame_num_wrap < (*(FrameStore**)arg2)->frame_num_wrap)
		return 1;
	if ( (*(FrameStore**)arg1)->frame_num_wrap > (*(FrameStore**)arg2)->frame_num_wrap)
		return -1;
	else
		return 0;
}


/*!
************************************************************************
* \brief
*    compares two frame stores by lt_pic_num for qsort in descending order
*
************************************************************************
*/
static int __cdecl compare_fs_by_lt_pic_idx_asc( const void *arg1, const void *arg2 )
{
	if ( (*(FrameStore**)arg1)->long_term_frame_idx < (*(FrameStore**)arg2)->long_term_frame_idx)
		return -1;
	if ( (*(FrameStore**)arg1)->long_term_frame_idx > (*(FrameStore**)arg2)->long_term_frame_idx)
		return 1;
	else
		return 0;
}


/*!
************************************************************************
* \brief
*    compares two stored pictures by poc for qsort in ascending order
*
************************************************************************
*/
static int __cdecl compare_pic_by_poc_asc( const void *arg1, const void *arg2 )
{
	if ( (*(StorablePicture**)arg1)->poc < (*(StorablePicture**)arg2)->poc)
		return -1;
	if ( (*(StorablePicture**)arg1)->poc > (*(StorablePicture**)arg2)->poc)
		return 1;
	else
		return 0;
}


/*!
************************************************************************
* \brief
*    compares two stored pictures by poc for qsort in descending order
*
************************************************************************
*/
static int __cdecl compare_pic_by_poc_desc( const void *arg1, const void *arg2 )
{
	if ( (*(StorablePicture**)arg1)->poc < (*(StorablePicture**)arg2)->poc)
		return 1;
	if ( (*(StorablePicture**)arg1)->poc > (*(StorablePicture**)arg2)->poc)
		return -1;
	else
		return 0;
}


/*!
************************************************************************
* \brief
*    compares two frame stores by poc for qsort in ascending order
*
************************************************************************
*/
static int __cdecl compare_fs_by_poc_asc( const void *arg1, const void *arg2 )
{
	if ( (*(FrameStore**)arg1)->poc < (*(FrameStore**)arg2)->poc)
		return -1;
	if ( (*(FrameStore**)arg1)->poc > (*(FrameStore**)arg2)->poc)
		return 1;
	else
		return 0;
}


/*!
************************************************************************
* \brief
*    compares two frame stores by poc for qsort in descending order
*
************************************************************************
*/
static int __cdecl compare_fs_by_poc_desc( const void *arg1, const void *arg2 )
{
	if ( (*(FrameStore**)arg1)->poc < (*(FrameStore**)arg2)->poc)
		return 1;
	if ( (*(FrameStore**)arg1)->poc > (*(FrameStore**)arg2)->poc)
		return -1;
	else
		return 0;
}


/*!
************************************************************************
* \brief
*    returns true, if picture is short term reference picture
*
************************************************************************
*/
int is_short_ref(StorablePicture *s)
{
	return ((s->used_for_reference) && (!(s->is_long_term)));
}


/*!
************************************************************************
* \brief
*    returns true, if picture is long term reference picture
*
************************************************************************
*/
int is_long_ref(StorablePicture *s)
{
	return ((s->used_for_reference) && (s->is_long_term));
}


/*!
************************************************************************
* \brief
*    Generates a alternating field list from a given FrameStore list
*
************************************************************************
*/
static void gen_pic_list_from_frame_list(PictureStructure currStrcture, FrameStore **fs_list, int list_idx, StorablePicture **list, int *list_size, int long_term)
{
	int top_idx = 0;
	int bot_idx = 0;

	int (*is_ref)(StorablePicture *s);

	if (long_term)
		is_ref=is_long_ref;
	else
		is_ref=is_short_ref;

	if (currStrcture == TOP_FIELD)
	{
		while ((top_idx<list_idx)||(bot_idx<list_idx))
		{
			for ( ; top_idx<list_idx; top_idx++)
			{
				if(fs_list[top_idx]->is_used & TOP_FIELD)
				{
					if(is_ref(fs_list[top_idx]->top_field))
					{
						// short term ref pic
						list[*list_size] = fs_list[top_idx]->top_field;
						(*list_size)++;
						top_idx++;
						break;
					}
				}
			}
			for ( ; bot_idx<list_idx; bot_idx++)
			{
				if(fs_list[bot_idx]->is_used & BOTTOM_FIELD)
				{
					if(is_ref(fs_list[bot_idx]->bottom_field))
					{
						// short term ref pic
						list[*list_size] = fs_list[bot_idx]->bottom_field;
						(*list_size)++;
						bot_idx++;
						break;
					}
				}
			}
		}
	}
	if (currStrcture == BOTTOM_FIELD)
	{
		while ((top_idx<list_idx)||(bot_idx<list_idx))
		{
			for ( ; bot_idx<list_idx; bot_idx++)
			{
				if(fs_list[bot_idx]->is_used & BOTTOM_FIELD)
				{
					if(is_ref(fs_list[bot_idx]->bottom_field))
					{
						// short term ref pic
						list[*list_size] = fs_list[bot_idx]->bottom_field;
						(*list_size)++;
						bot_idx++;
						break;
					}
				}
			}
			for ( ; top_idx<list_idx; top_idx++)
			{
				if(fs_list[top_idx]->is_used & TOP_FIELD)
				{
					if(is_ref(fs_list[top_idx]->top_field))
					{
						// short term ref pic
						list[*list_size] = fs_list[top_idx]->top_field;
						(*list_size)++;
						top_idx++;
						break;
					}
				}
			}
		}
	}
}


/*!
************************************************************************
* \brief
*    Initialize listX[0] and list 1 depending on current picture type
*
************************************************************************
*/
CREL_RETURN init_lists PARGS2(int currSliceType, PictureStructure currPicStructure)
{
	int add_top = 0, add_bottom = 0;
	unsigned i;
	int j;
	int MaxFrameNum = 1 << (active_sps.log2_max_frame_num_minus4 + 4);
	int diff;

	int list0idx = 0;
	int list0idx_1 = 0;
	int listltidx = 0;

	FrameStore **fs_list0;
	FrameStore **fs_list1;
	FrameStore **fs_listlt;

	StorablePicture *tmp_s;
	unsigned int view_index = (IMGPAR currentSlice) ? (IMGPAR currentSlice->viewIndex) : 0;

	if ( dpb.ref_frames_in_buffer_on_view[view_index] > dpb.used_size_on_view[view_index] ) {
		return CREL_WARNING_H264_ERROR_DPB | CREL_WARNING_H264_STREAMERROR_LEVEL_1;
	}

	if (currPicStructure == FRAME)  
	{
		for (i=0; i<dpb.ref_frames_in_buffer_on_view[view_index]; i++)
		{
			if (dpb.fs_ref_on_view[view_index][i]->is_used==(TOP_FIELD|BOTTOM_FIELD))
			{
				if ((dpb.fs_ref_on_view[view_index][i]->frame->used_for_reference)&&(!dpb.fs_ref_on_view[view_index][i]->frame->is_long_term))
				{
					if( dpb.fs_ref_on_view[view_index][i]->frame_num > IMGPAR frame_num )
					{
						dpb.fs_ref_on_view[view_index][i]->frame_num_wrap = dpb.fs_ref_on_view[view_index][i]->frame_num - MaxFrameNum;
					}
					else
					{
						dpb.fs_ref_on_view[view_index][i]->frame_num_wrap = dpb.fs_ref_on_view[view_index][i]->frame_num;
					}
					dpb.fs_ref_on_view[view_index][i]->frame->pic_num = dpb.fs_ref_on_view[view_index][i]->frame_num_wrap;
					dpb.fs_ref_on_view[view_index][i]->frame->order_num=list0idx;
				}
			}
		}
	}
	else
	{
		if (currPicStructure == TOP_FIELD)
		{
			add_top    = 1;
			add_bottom = 0;
		}
		else
		{
			add_top    = 0;
			add_bottom = 1;
		}

		for (i=0; i<dpb.ref_frames_in_buffer_on_view[view_index]; i++)
		{
			if (dpb.fs_ref_on_view[view_index][i]->is_reference)
			{
				if( dpb.fs_ref_on_view[view_index][i]->frame_num > IMGPAR frame_num )
				{
					dpb.fs_ref_on_view[view_index][i]->frame_num_wrap = dpb.fs_ref_on_view[view_index][i]->frame_num - MaxFrameNum;
				}
				else
				{
					dpb.fs_ref_on_view[view_index][i]->frame_num_wrap = dpb.fs_ref_on_view[view_index][i]->frame_num;
				}
				if (dpb.fs_ref_on_view[view_index][i]->is_reference & TOP_FIELD)
				{
					dpb.fs_ref_on_view[view_index][i]->top_field->pic_num = (2 * dpb.fs_ref_on_view[view_index][i]->frame_num_wrap) + add_top;
				}
				if (dpb.fs_ref_on_view[view_index][i]->is_reference & BOTTOM_FIELD)
				{
					dpb.fs_ref_on_view[view_index][i]->bottom_field->pic_num = (2 * dpb.fs_ref_on_view[view_index][i]->frame_num_wrap) + add_bottom;
				}
			}
		}
	}

	if (currSliceType == I_SLICE)
	{
		listXsize[0] = 0;
		listXsize[1] = 0;
		return CREL_OK;
	}

	if (currSliceType == P_SLICE)
	{
		// Calculate FrameNumWrap and PicNum
		if (currPicStructure == FRAME)  
		{
			for (i=0; i<dpb.ref_frames_in_buffer_on_view[view_index]; i++)
			{
				if (dpb.fs_ref_on_view[view_index][i]->is_used==(TOP_FIELD|BOTTOM_FIELD))
				{
					if ((dpb.fs_ref_on_view[view_index][i]->frame->used_for_reference)&&(!dpb.fs_ref_on_view[view_index][i]->frame->is_long_term))
					{
						listX[0][list0idx++] = dpb.fs_ref_on_view[view_index][i]->frame;
					}
				}
			}
			// order list 0 by PicNum
			qsort((void *)listX[0], list0idx, sizeof(StorablePicture*), compare_pic_by_pic_num_desc);
			listXsize[0] = list0idx;

			// long term handling
			for (i=0; i<dpb.ltref_frames_in_buffer_on_view[view_index]; i++)
			{
				if (dpb.fs_ltref_on_view[view_index][i]->is_used==(TOP_FIELD|BOTTOM_FIELD))
				{
					if (dpb.fs_ltref_on_view[view_index][i]->frame->is_long_term)
					{
						dpb.fs_ltref_on_view[view_index][i]->frame->long_term_pic_num = dpb.fs_ltref_on_view[view_index][i]->frame->long_term_frame_idx;
						dpb.fs_ltref_on_view[view_index][i]->frame->order_num=list0idx;
						listX[0][list0idx++]=dpb.fs_ltref_on_view[view_index][i]->frame;
					}
				}
			}
			qsort((void *)&listX[0][listXsize[0]], list0idx-listXsize[0], sizeof(StorablePicture*), compare_pic_by_lt_pic_num_asc);
			listXsize[0] = list0idx;

			//MVC Inter-view

			if (view_index > 0) {		//Dependent View: Adds inter-view reference pictures

				if ( img->currentSlice->anchorPicFlag ) 
				{
					int num_ref = active_sps.num_anchor_refs_l0[view_index];
					for ( i = 0; i < num_ref; i++) 
					{
						//Search inter-view reference on SPS pre-defined views
						int anchor_refs_view_index = GetViewIndex ARGS1(active_sps.anchor_refs_l0[view_index][i]);
						int ref_dpb_size = dpb.used_size_on_view[anchor_refs_view_index];

						for ( j = 0; j < ref_dpb_size; j++ ) {						

							if (( dpb.fs_on_view[anchor_refs_view_index][j]->poc == IMGPAR ThisPOC) 
								&& dpb.fs_on_view[anchor_refs_view_index][j]->is_used == 3 && dpb.fs_on_view[anchor_refs_view_index][j]->frame->inter_view_flag){
									listX[0][list0idx++] = dpb.fs_on_view[anchor_refs_view_index][j]->frame;
							}
						}
					}

				} 
				else 
				{
					int num_ref = active_sps.num_non_anchor_refs_l0[view_index];
					for ( i = 0; i < active_sps.num_non_anchor_refs_l0[view_index]; i++) 
					{
						int non_anchor_refs_view_index = GetViewIndex ARGS1(active_sps.non_anchor_refs_l0[view_index][i]);
						int ref_dpb_size = dpb.used_size_on_view[non_anchor_refs_view_index];
						//Search inter-view reference on SPS pre-defined views

						for ( j = 0; j < ref_dpb_size; j++ ) {						

							if (( dpb.fs_on_view[non_anchor_refs_view_index][j]->poc == IMGPAR ThisPOC) 
								&& dpb.fs_on_view[non_anchor_refs_view_index][j]->is_used == 3 && dpb.fs_on_view[non_anchor_refs_view_index][j]->frame->inter_view_flag){
									listX[0][list0idx++] = dpb.fs_on_view[non_anchor_refs_view_index][j]->frame;
							}
						}
					}
				}
			}

			listXsize[0] = list0idx;
		}
		else
		{
			fs_list0 = (struct frame_store **) _aligned_malloc(dpb.size_on_view[view_index]*sizeof (FrameStore*), 16);
			/*
			if (NULL==fs_list0) 
			no_mem_exit("init_lists: fs_list0");
			*/
			fs_listlt = (struct frame_store **) _aligned_malloc(dpb.size_on_view[view_index]*sizeof (FrameStore*), 16);
			/*
			if (NULL==fs_listlt) 
			no_mem_exit("init_lists: fs_listlt");
			*/

			for (i=0; i<dpb.ref_frames_in_buffer_on_view[view_index]; i++)
			{
				if (dpb.fs_ref_on_view[view_index][i]->is_reference)
				{
					fs_list0[list0idx++] = dpb.fs_ref_on_view[view_index][i];
				}
			}

			qsort((void *)fs_list0, list0idx, sizeof(FrameStore*), compare_fs_by_frame_num_desc);


			listXsize[0] = 0;
			gen_pic_list_from_frame_list(currPicStructure, fs_list0, list0idx, listX[0], &listXsize[0], 0);


			// long term handling
			for (i=0; i<dpb.ltref_frames_in_buffer_on_view[view_index]; i++)
			{
				fs_listlt[listltidx++]=dpb.fs_ltref_on_view[view_index][i];
				if (dpb.fs_ltref_on_view[view_index][i]->is_long_term & TOP_FIELD)
				{
					dpb.fs_ltref_on_view[view_index][i]->top_field->long_term_pic_num = 2 * dpb.fs_ltref_on_view[view_index][i]->top_field->long_term_frame_idx + add_top;
				}
				if (dpb.fs_ltref_on_view[view_index][i]->is_long_term & BOTTOM_FIELD)
				{
					dpb.fs_ltref_on_view[view_index][i]->bottom_field->long_term_pic_num = 2 * dpb.fs_ltref_on_view[view_index][i]->bottom_field->long_term_frame_idx + add_bottom;
				}
			}

			qsort((void *)fs_listlt, listltidx, sizeof(FrameStore*), compare_fs_by_lt_pic_idx_asc);

			gen_pic_list_from_frame_list(currPicStructure, fs_listlt, listltidx, listX[0], &listXsize[0], 1);

			_aligned_free(fs_list0);
			_aligned_free(fs_listlt);

			//MVC Inter-view
			if (view_index > 0) {		//Dependent View: Adds inter-view reference pictures

				list0idx = listXsize[0];

				if ( img->currentSlice->anchorPicFlag ) 
				{
					int num_ref = active_sps.num_anchor_refs_l0[view_index];
					for ( i = 0; i < num_ref; i++) 
					{
						//Search inter-view reference on SPS pre-defined views
						int anchor_refs_view_index = GetViewIndex ARGS1(active_sps.anchor_refs_l0[view_index][i]);
						int ref_dpb_size = dpb.used_size_on_view[anchor_refs_view_index];

						for ( j = 0; j < ref_dpb_size; j++ ) 
						{						
							if(currPicStructure == TOP_FIELD)
							{
								if (( dpb.fs_on_view[anchor_refs_view_index][j]->top_field->poc == IMGPAR ThisPOC) 
									&& (dpb.fs_on_view[anchor_refs_view_index][j]->is_used & TOP_FIELD) && dpb.fs_on_view[anchor_refs_view_index][j]->top_field->inter_view_flag)
								{
									listX[0][list0idx++] = dpb.fs_on_view[anchor_refs_view_index][j]->top_field;
								}
							}
							else if(currPicStructure == BOTTOM_FIELD)
							{
								if (( dpb.fs_on_view[anchor_refs_view_index][j]->bottom_field->poc == IMGPAR ThisPOC) 
									&& (dpb.fs_on_view[anchor_refs_view_index][j]->is_used & BOTTOM_FIELD) && dpb.fs_on_view[anchor_refs_view_index][j]->bottom_field->inter_view_flag)
								{
									listX[0][list0idx++] = dpb.fs_on_view[anchor_refs_view_index][j]->bottom_field;
								}
							}
							
						}

					}

				} 
				else 
				{
					int num_ref = active_sps.num_non_anchor_refs_l0[view_index];
					for ( i = 0; i < active_sps.num_non_anchor_refs_l0[view_index]; i++) 
					{
						int non_anchor_refs_view_index = GetViewIndex ARGS1(active_sps.non_anchor_refs_l0[view_index][i]);
						int ref_dpb_size = dpb.used_size_on_view[non_anchor_refs_view_index];
						//Search inter-view reference on SPS pre-defined views

						for ( j = 0; j < ref_dpb_size; j++ ) 
						{						
							if(currPicStructure == TOP_FIELD)
							{
								if (( dpb.fs_on_view[non_anchor_refs_view_index][j]->top_field->poc == IMGPAR ThisPOC) 
									&& (dpb.fs_on_view[non_anchor_refs_view_index][j]->is_used & TOP_FIELD) && dpb.fs_on_view[non_anchor_refs_view_index][j]->top_field->inter_view_flag)
								{
									listX[0][list0idx++] = dpb.fs_on_view[non_anchor_refs_view_index][j]->top_field;
								}
							}
							else if(currPicStructure == BOTTOM_FIELD)
							{
								if (( dpb.fs_on_view[non_anchor_refs_view_index][j]->bottom_field->poc == IMGPAR ThisPOC) 
									&& (dpb.fs_on_view[non_anchor_refs_view_index][j]->is_used & BOTTOM_FIELD) && dpb.fs_on_view[non_anchor_refs_view_index][j]->bottom_field->inter_view_flag)
								{
									listX[0][list0idx++] = dpb.fs_on_view[non_anchor_refs_view_index][j]->bottom_field;
								}
							}

							
						}
					}
				}
				listXsize[0] = list0idx;
			}
			
		}
		listXsize[1] = 0;
	}
	else
	{
		// B-Slice
		if (currPicStructure == FRAME)  
		{
			for (i=0; i<dpb.ref_frames_in_buffer_on_view[view_index]; i++)
			{
				if (dpb.fs_ref_on_view[view_index][i]->is_used==(TOP_FIELD|BOTTOM_FIELD))
				{
					if ((dpb.fs_ref_on_view[view_index][i]->frame->used_for_reference)&&(!dpb.fs_ref_on_view[view_index][i]->frame->is_long_term))
					{
						if (IMGPAR framepoc > dpb.fs_ref_on_view[view_index][i]->frame->poc)
						{
							dpb.fs_ref_on_view[view_index][i]->frame->order_num=list0idx;
							listX[0][list0idx++] = dpb.fs_ref_on_view[view_index][i]->frame;
						}
					}
				}
			}
			qsort((void *)listX[0], list0idx, sizeof(StorablePicture*), compare_pic_by_poc_desc);
			list0idx_1 = list0idx;
			for (i=0; i<dpb.ref_frames_in_buffer_on_view[view_index]; i++)
			{
				if (dpb.fs_ref_on_view[view_index][i]->is_used==(TOP_FIELD|BOTTOM_FIELD))
				{
					if ((dpb.fs_ref_on_view[view_index][i]->frame->used_for_reference)&&(!dpb.fs_ref_on_view[view_index][i]->frame->is_long_term))
					{
						if (IMGPAR framepoc < dpb.fs_ref_on_view[view_index][i]->frame->poc)
						{
							dpb.fs_ref_on_view[view_index][i]->frame->order_num=list0idx;
							listX[0][list0idx++] = dpb.fs_ref_on_view[view_index][i]->frame;
						}
					}
				}
			}
			qsort((void *)&listX[0][list0idx_1], list0idx-list0idx_1, sizeof(StorablePicture*), compare_pic_by_poc_asc);

			for (j=0; j<list0idx_1; j++)
			{
				listX[1][list0idx-list0idx_1+j]=listX[0][j];
			}
			for (j=list0idx_1; j<list0idx; j++)
			{
				listX[1][j-list0idx_1]=listX[0][j];
			}

			listXsize[0] = listXsize[1] = list0idx;


			// long term handling
			for (i=0; i<dpb.ltref_frames_in_buffer_on_view[view_index]; i++)
			{
				if (dpb.fs_ltref_on_view[view_index][i]->is_used==(TOP_FIELD|BOTTOM_FIELD))
				{
					if (dpb.fs_ltref_on_view[view_index][i]->frame->is_long_term)
					{
						dpb.fs_ltref_on_view[view_index][i]->frame->long_term_pic_num = dpb.fs_ltref_on_view[view_index][i]->frame->long_term_frame_idx;
						dpb.fs_ltref_on_view[view_index][i]->frame->order_num=list0idx;

						listX[0][list0idx]  =dpb.fs_ltref_on_view[view_index][i]->frame;
						listX[1][list0idx++]=dpb.fs_ltref_on_view[view_index][i]->frame;
					}
				}
			}
			qsort((void *)&listX[0][listXsize[0]], list0idx-listXsize[0], sizeof(StorablePicture*), compare_pic_by_lt_pic_num_asc);
			qsort((void *)&listX[1][listXsize[0]], list0idx-listXsize[0], sizeof(StorablePicture*), compare_pic_by_lt_pic_num_asc);
			listXsize[0] = listXsize[1] = list0idx;

			//MVC Inter-view

			if (view_index > 0) 
			{
				list0idx = listXsize[0];

				if ( img->currentSlice->anchorPicFlag ) 
				{
					int num_ref = active_sps.num_anchor_refs_l0[view_index];
					for ( i = 0; i < num_ref; i++) 
					{
						int anchor_refs_view_index = GetViewIndex ARGS1(active_sps.anchor_refs_l0[view_index][i]);
						int ref_dpb_size = dpb.used_size_on_view[anchor_refs_view_index];

						for ( j = 0; j < ref_dpb_size; j++ ) 
						{
							if (( dpb.fs_on_view[anchor_refs_view_index][j]->poc == IMGPAR ThisPOC) 
								&& dpb.fs_on_view[anchor_refs_view_index][j]->is_used == 3 && dpb.fs_on_view[anchor_refs_view_index][j]->frame->inter_view_flag)
							{
									listX[0][list0idx++] = dpb.fs_on_view[anchor_refs_view_index][j]->frame;
							}
						}
					}

					listXsize[0] = list0idx;

					list0idx = listXsize[1];

					num_ref = active_sps.num_anchor_refs_l1[view_index];
					for ( i = 0; i < num_ref; i++) 
					{
						int anchor_refs_view_index = GetViewIndex ARGS1(active_sps.anchor_refs_l1[view_index][i]);
						int ref_dpb_size = dpb.used_size_on_view[anchor_refs_view_index];

						for ( j = 0; j < ref_dpb_size; j++ )
						{
							if (( dpb.fs_on_view[anchor_refs_view_index][j]->poc == IMGPAR ThisPOC) 
								&& dpb.fs_on_view[anchor_refs_view_index][j]->is_used == 3 && dpb.fs_on_view[anchor_refs_view_index][j]->frame->inter_view_flag )
							{
									listX[1][list0idx++] = dpb.fs_on_view[anchor_refs_view_index][j]->frame;
							}
						}
					}

					listXsize[1] = list0idx;

				} 
				else 
				{
					int num_ref = active_sps.num_non_anchor_refs_l0[view_index];
					for ( i = 0; i < num_ref; i++) 
					{
						int non_anchor_refs_view_index = GetViewIndex ARGS1(active_sps.non_anchor_refs_l0[view_index][i]);
						int ref_dpb_size = dpb.used_size_on_view[non_anchor_refs_view_index];

						for ( j = 0; j < ref_dpb_size; j++ ) 
						{
							if (( dpb.fs_on_view[non_anchor_refs_view_index][j]->poc == IMGPAR ThisPOC)
								&& dpb.fs_on_view[non_anchor_refs_view_index][j]->is_used == 3 && dpb.fs_on_view[non_anchor_refs_view_index][j]->frame->inter_view_flag )
							{
									listX[0][list0idx++] = dpb.fs_on_view[non_anchor_refs_view_index][j]->frame;
							}
						}
					}

					listXsize[0] = list0idx;

					list0idx = listXsize[1];

					num_ref = active_sps.num_non_anchor_refs_l1[view_index];
					for ( i = 0; i < num_ref; i++)
					{
						int non_anchor_refs_view_index = GetViewIndex ARGS1(active_sps.non_anchor_refs_l1[view_index][i]);
						int ref_dpb_size = dpb.used_size_on_view[non_anchor_refs_view_index];

						for ( j = 0; j < ref_dpb_size; j++ ) 
						{
							if (( dpb.fs_on_view[non_anchor_refs_view_index][j]->poc == IMGPAR ThisPOC)
								&& dpb.fs_on_view[non_anchor_refs_view_index][j]->is_used == 3 && dpb.fs_on_view[non_anchor_refs_view_index][j]->frame->inter_view_flag )
							{
									listX[1][list0idx++] = dpb.fs_on_view[non_anchor_refs_view_index][j]->frame;
							}
						}
					}

					listXsize[1] = list0idx;
				}
			}

			//listXsize[0] = list0idx;
		}
		else
		{
			fs_list0 = (struct frame_store **) _aligned_malloc(dpb.size_on_view[view_index]*sizeof (FrameStore*), 16);
			/*
			if (NULL==fs_list0) 
			no_mem_exit("init_lists: fs_list0");
			*/
			fs_list1 = (struct frame_store **) _aligned_malloc(dpb.size_on_view[view_index]*sizeof (FrameStore*), 16);
			/*
			if (NULL==fs_list1) 
			no_mem_exit("init_lists: fs_list1");
			*/
			fs_listlt = (struct frame_store **) _aligned_malloc(dpb.size_on_view[view_index]*sizeof (FrameStore*), 16);
			/*if (NULL==fs_listlt) 
			no_mem_exit("init_lists: fs_listlt");
			*/
			listXsize[0] = 0;
			listXsize[1] = 1;

			for (i=0; i<dpb.ref_frames_in_buffer_on_view[view_index]; i++)
			{
				if (dpb.fs_ref_on_view[view_index][i]->is_used)
				{
					if (IMGPAR ThisPOC >= dpb.fs_ref_on_view[view_index][i]->poc)
					{
						fs_list0[list0idx++] = dpb.fs_ref_on_view[view_index][i];
					}
				}
			}
			qsort((void *)fs_list0, list0idx, sizeof(FrameStore*), compare_fs_by_poc_desc);
			list0idx_1 = list0idx;
			for (i=0; i<dpb.ref_frames_in_buffer_on_view[view_index]; i++)
			{
				if (dpb.fs_ref_on_view[view_index][i]->is_used)
				{
					if (IMGPAR ThisPOC < dpb.fs_ref_on_view[view_index][i]->poc)
					{
						fs_list0[list0idx++] = dpb.fs_ref_on_view[view_index][i];
					}
				}
			}
			qsort((void *)&fs_list0[list0idx_1], list0idx-list0idx_1, sizeof(FrameStore*), compare_fs_by_poc_asc);

			for (j=0; j<list0idx_1; j++)
			{
				fs_list1[list0idx-list0idx_1+j]=fs_list0[j];
			}
			for (j=list0idx_1; j<list0idx; j++)
			{
				fs_list1[j-list0idx_1]=fs_list0[j];
			}

			listXsize[0] = 0;
			listXsize[1] = 0;
			gen_pic_list_from_frame_list(currPicStructure, fs_list0, list0idx, listX[0], &listXsize[0], 0);
			gen_pic_list_from_frame_list(currPicStructure, fs_list1, list0idx, listX[1], &listXsize[1], 0);

			// long term handling
			for (i=0; i<dpb.ltref_frames_in_buffer_on_view[view_index]; i++)
			{
				fs_listlt[listltidx++]=dpb.fs_ltref_on_view[view_index][i];
				if (dpb.fs_ltref_on_view[view_index][i]->is_long_term & TOP_FIELD)
				{
					dpb.fs_ltref_on_view[view_index][i]->top_field->long_term_pic_num = 2 * dpb.fs_ltref_on_view[view_index][i]->top_field->long_term_frame_idx + add_top;
				}
				if (dpb.fs_ltref_on_view[view_index][i]->is_long_term & BOTTOM_FIELD)
				{
					dpb.fs_ltref_on_view[view_index][i]->bottom_field->long_term_pic_num = 2 * dpb.fs_ltref_on_view[view_index][i]->bottom_field->long_term_frame_idx + add_bottom;
				}
			}

			qsort((void *)fs_listlt, listltidx, sizeof(FrameStore*), compare_fs_by_lt_pic_idx_asc);

			gen_pic_list_from_frame_list(currPicStructure, fs_listlt, listltidx, listX[0], &listXsize[0], 1);
			gen_pic_list_from_frame_list(currPicStructure, fs_listlt, listltidx, listX[1], &listXsize[1], 1);

			_aligned_free(fs_list0);
			_aligned_free(fs_list1);
			_aligned_free(fs_listlt);

			
			//MVC Inter-view

			if (view_index > 0) 
			{
				list0idx = listXsize[0];

				if ( img->currentSlice->anchorPicFlag ) 
				{
					int num_ref = active_sps.num_anchor_refs_l0[view_index];
					for ( i = 0; i < num_ref; i++) 
					{
						int anchor_refs_view_index = GetViewIndex ARGS1(active_sps.anchor_refs_l0[view_index][i]);
						int ref_dpb_size = dpb.used_size_on_view[anchor_refs_view_index];

						for ( j = 0; j < ref_dpb_size; j++ ) 
						{
							if(currPicStructure == TOP_FIELD)
							{
								if (( dpb.fs_on_view[anchor_refs_view_index][j]->top_field->poc == IMGPAR ThisPOC) 
									&& (dpb.fs_on_view[anchor_refs_view_index][j]->is_used & TOP_FIELD) && dpb.fs_on_view[anchor_refs_view_index][j]->top_field->inter_view_flag)
								{
									listX[0][list0idx++] = dpb.fs_on_view[anchor_refs_view_index][j]->top_field;
								}
							}
							else if(currPicStructure == BOTTOM_FIELD)
							{
								if (( dpb.fs_on_view[anchor_refs_view_index][j]->bottom_field->poc == IMGPAR ThisPOC) 
									&& (dpb.fs_on_view[anchor_refs_view_index][j]->is_used & BOTTOM_FIELD) && dpb.fs_on_view[anchor_refs_view_index][j]->bottom_field->inter_view_flag)
								{
									listX[0][list0idx++] = dpb.fs_on_view[anchor_refs_view_index][j]->bottom_field;
								}
							}
						}
					}

					listXsize[0] = list0idx;

					list0idx = listXsize[1];

					num_ref = active_sps.num_anchor_refs_l1[view_index];
					for ( i = 0; i < num_ref; i++) 
					{
						int anchor_refs_view_index = GetViewIndex ARGS1(active_sps.anchor_refs_l1[view_index][i]);
						int ref_dpb_size = dpb.used_size_on_view[anchor_refs_view_index];

						for ( j = 0; j < ref_dpb_size; j++ )
						{
							if(currPicStructure == TOP_FIELD)
							{
								if (( dpb.fs_on_view[anchor_refs_view_index][j]->top_field->poc == IMGPAR ThisPOC) 
									&& (dpb.fs_on_view[anchor_refs_view_index][j]->is_used & TOP_FIELD) && dpb.fs_on_view[anchor_refs_view_index][j]->top_field->inter_view_flag )
								{
									listX[1][list0idx++] = dpb.fs_on_view[anchor_refs_view_index][j]->top_field;
								}
							}
							else if(currPicStructure == BOTTOM_FIELD)
							{
								if (( dpb.fs_on_view[anchor_refs_view_index][j]->bottom_field->poc == IMGPAR ThisPOC) 
									&& (dpb.fs_on_view[anchor_refs_view_index][j]->is_used & BOTTOM_FIELD) && dpb.fs_on_view[anchor_refs_view_index][j]->bottom_field->inter_view_flag )
								{
									listX[1][list0idx++] = dpb.fs_on_view[anchor_refs_view_index][j]->bottom_field;
								}
							}
						}
					}

					listXsize[1] = list0idx;

				} 
				else 
				{
					int num_ref = active_sps.num_non_anchor_refs_l0[view_index];
					for ( i = 0; i < num_ref; i++) 
					{
						int non_anchor_refs_view_index = GetViewIndex ARGS1(active_sps.non_anchor_refs_l0[view_index][i]);
						int ref_dpb_size = dpb.used_size_on_view[non_anchor_refs_view_index];

						for ( j = 0; j < ref_dpb_size; j++ ) 
						{
							if(currPicStructure == TOP_FIELD)
							{
								if (( dpb.fs_on_view[non_anchor_refs_view_index][j]->top_field->poc == IMGPAR ThisPOC)
									&& (dpb.fs_on_view[non_anchor_refs_view_index][j]->is_used & TOP_FIELD) && dpb.fs_on_view[non_anchor_refs_view_index][j]->top_field->inter_view_flag )
								{
									listX[0][list0idx++] = dpb.fs_on_view[non_anchor_refs_view_index][j]->top_field;
								}
							}
							else if(currPicStructure == BOTTOM_FIELD)
							{
								if (( dpb.fs_on_view[non_anchor_refs_view_index][j]->bottom_field->poc == IMGPAR ThisPOC)
									&& (dpb.fs_on_view[non_anchor_refs_view_index][j]->is_used & BOTTOM_FIELD) && dpb.fs_on_view[non_anchor_refs_view_index][j]->bottom_field->inter_view_flag )
								{
									listX[0][list0idx++] = dpb.fs_on_view[non_anchor_refs_view_index][j]->bottom_field;
								}
							}
						}
					}

					listXsize[0] = list0idx;

					list0idx = listXsize[1];

					num_ref = active_sps.num_non_anchor_refs_l1[view_index];
					for ( i = 0; i < num_ref; i++)
					{
						int non_anchor_refs_view_index = GetViewIndex ARGS1(active_sps.non_anchor_refs_l1[view_index][i]);
						int ref_dpb_size = dpb.used_size_on_view[non_anchor_refs_view_index];

						for ( j = 0; j < ref_dpb_size; j++ ) 
						{
							if(currPicStructure == TOP_FIELD)
							{
								if (( dpb.fs_on_view[non_anchor_refs_view_index][j]->top_field->poc == IMGPAR ThisPOC)
									&& (dpb.fs_on_view[non_anchor_refs_view_index][j]->is_used & TOP_FIELD) && dpb.fs_on_view[non_anchor_refs_view_index][j]->top_field->inter_view_flag )
								{
									listX[1][list0idx++] = dpb.fs_on_view[non_anchor_refs_view_index][j]->top_field;
								}
							}
							else if(currPicStructure == BOTTOM_FIELD)
							{
								if (( dpb.fs_on_view[non_anchor_refs_view_index][j]->bottom_field->poc == IMGPAR ThisPOC)
									&& (dpb.fs_on_view[non_anchor_refs_view_index][j]->is_used & BOTTOM_FIELD) && dpb.fs_on_view[non_anchor_refs_view_index][j]->bottom_field->inter_view_flag )
								{
									listX[1][list0idx++] = dpb.fs_on_view[non_anchor_refs_view_index][j]->bottom_field;
								}
							}

						
						}
					}

					listXsize[1] = list0idx;
				}
			}			
		}
	}

	if ((listXsize[0] == listXsize[1]) && (listXsize[0] > 1))
	{
		// check if lists are identical, if yes swap first two elements of listX[1]
		diff=0;
		for (j = 0; j< listXsize[0]; j++)
		{
			if (listX[0][j]!=listX[1][j])
				diff=1;
		}
		if (!diff)
		{
			tmp_s = listX[1][0];
			listX[1][0]=listX[1][1];
			listX[1][1]=tmp_s;
		}
	}
	// set max size
#if !defined(_COLLECT_PIC_)
	listXsize[0] = min (listXsize[0], IMGPAR num_ref_idx_l0_active);
	listXsize[1] = min (listXsize[1], IMGPAR num_ref_idx_l1_active);
#else
	listXsize[0] = min (listXsize[0], IMGPAR currentSlice->num_ref_idx_l0_active);
	listXsize[1] = min (listXsize[1], IMGPAR currentSlice->num_ref_idx_l1_active);
#endif

	// set the unused list entries to NULL
	for (i=listXsize[0]; i< (MAX_LIST_SIZE_FRAME) ; i++)
	{
		listX[0][i] = NULL;
	}
	for (i=listXsize[1]; i< (MAX_LIST_SIZE_FRAME) ; i++)
	{
		listX[1][i] = NULL;
	}

	return CREL_OK;
}

/*!
************************************************************************
* \brief
*    Initilaize listX[2..5] from lists 0 and 1
*    listX[2]: list0 for current_field==top
*    listX[3]: list1 for current_field==top
*    listX[4]: list0 for current_field==bottom
*    listX[5]: list1 for current_field==bottom
*
************************************************************************
*/
void init_mbaff_lists PARGS0()
{
	unsigned j;
	int i;

	for (i=2;i<6;i++)
	{
		for (j=0; j<MAX_LIST_SIZE; j++)
		{
			listX[i][j] = NULL;
		}
		listXsize[i]=0;
	}

	for (i=0; i<listXsize[0]; i++)
	{
		listX[2][2*i]  =listX[0][i]->top_field;
		listX[2][2*i+1]=listX[0][i]->bottom_field;
		listX[4][2*i]  =listX[0][i]->bottom_field;
		listX[4][2*i+1]=listX[0][i]->top_field;
	}
	listXsize[2]=listXsize[4]=listXsize[0] * 2;

	for (i=0; i<listXsize[1]; i++)
	{
		listX[3][2*i]  =listX[1][i]->top_field;
		listX[3][2*i+1]=listX[1][i]->bottom_field;
		listX[5][2*i]  =listX[1][i]->bottom_field;
		listX[5][2*i+1]=listX[1][i]->top_field;
	}
	listXsize[3]=listXsize[5]=listXsize[1] * 2;
}

/*!
************************************************************************
* \brief
*    Returns short term pic with given picNum
*
************************************************************************
*/
static StorablePicture*  get_short_term_pic PARGS1(int picNum)
{
	unsigned i;
	unsigned int view_index = (IMGPAR firstSlice) ? (IMGPAR firstSlice->viewIndex) : 0;

	for (i=0; i<dpb.ref_frames_in_buffer_on_view[view_index]; i++)
	{
		if (IMGPAR structure==FRAME)
		{
			if (dpb.fs_ref_on_view[view_index][i]->is_reference == (TOP_FIELD|BOTTOM_FIELD))
				if ((!dpb.fs_ref_on_view[view_index][i]->frame->is_long_term)&&(dpb.fs_ref_on_view[view_index][i]->frame->pic_num == picNum))
					return dpb.fs_ref_on_view[view_index][i]->frame;
		}
		else
		{
			if (dpb.fs_ref_on_view[view_index][i]->is_reference & TOP_FIELD)
				if ((!dpb.fs_ref_on_view[view_index][i]->top_field->is_long_term)&&(dpb.fs_ref_on_view[view_index][i]->top_field->pic_num == picNum))
					return dpb.fs_ref_on_view[view_index][i]->top_field;
			if (dpb.fs_ref_on_view[view_index][i]->is_reference & BOTTOM_FIELD)
				if ((!dpb.fs_ref_on_view[view_index][i]->bottom_field->is_long_term)&&(dpb.fs_ref_on_view[view_index][i]->bottom_field->pic_num == picNum))
					return dpb.fs_ref_on_view[view_index][i]->bottom_field;
		}
	}
	return NULL;
}

/*!
************************************************************************
* \brief
*    Returns short term pic with given LongtermPicNum
*
************************************************************************
*/
static StorablePicture*  get_long_term_pic PARGS1(int LongtermPicNum)
{
	unsigned i;
	unsigned int view_index = (IMGPAR firstSlice) ? (IMGPAR firstSlice->viewIndex) : 0;

	for (i=0; i<dpb.ltref_frames_in_buffer_on_view[view_index]; i++)
	{
		if (IMGPAR structure==FRAME)
		{
			if (dpb.fs_ltref_on_view[view_index][i]->is_reference == (TOP_FIELD|BOTTOM_FIELD))
				if ((dpb.fs_ltref_on_view[view_index][i]->frame->is_long_term)&&(dpb.fs_ltref_on_view[view_index][i]->frame->long_term_pic_num == LongtermPicNum))
					return dpb.fs_ltref_on_view[view_index][i]->frame;
		}
		else
		{
			if (dpb.fs_ltref_on_view[view_index][i]->is_reference & TOP_FIELD)
				if ((dpb.fs_ltref_on_view[view_index][i]->top_field->is_long_term)&&(dpb.fs_ltref_on_view[view_index][i]->top_field->long_term_pic_num == LongtermPicNum))
					return dpb.fs_ltref_on_view[view_index][i]->top_field;
			if (dpb.fs_ltref_on_view[view_index][i]->is_reference & BOTTOM_FIELD)
				if ((dpb.fs_ltref_on_view[view_index][i]->bottom_field->is_long_term)&&(dpb.fs_ltref_on_view[view_index][i]->bottom_field->long_term_pic_num == LongtermPicNum))
					return dpb.fs_ltref_on_view[view_index][i]->bottom_field;
		}
	}
	return NULL;
}

/*!
************************************************************************
* \brief
*    Reordering process for short-term reference pictures
*
************************************************************************
*/
static void reorder_short_term PARGS4(StorablePicture **RefPicListX, int num_ref_idx_lX_active_minus1, int picNumLX, int *refIdxLX)
{
	int cIdx, nIdx;

	StorablePicture *picLX;

	picLX = get_short_term_pic ARGS1(picNumLX);

	if (picLX == NULL) {
		DEBUG_SHOW_ERROR_INFO("[ERROR] reorder_short_term: Can't get the matched pic_num pic on DPB!!");
		return;		//Error found in DPB, can't find reference frame index by picNumLX, skip reordering as a work-around to avoid Seek IDR
	}

	for( cIdx = num_ref_idx_lX_active_minus1+1; cIdx > *refIdxLX; cIdx-- )
		RefPicListX[ cIdx ] = RefPicListX[ cIdx - 1];

	RefPicListX[ (*refIdxLX)++ ] = picLX;

	nIdx = *refIdxLX;

	for( cIdx = *refIdxLX; cIdx <= num_ref_idx_lX_active_minus1+1; cIdx++ )
		if (RefPicListX[ cIdx ])
			if( (RefPicListX[ cIdx ]->is_long_term ) ||  (RefPicListX[ cIdx ]->pic_num != picNumLX ))
				RefPicListX[ nIdx++ ] = RefPicListX[ cIdx ];

}


/*!
************************************************************************
* \brief
*    Reordering process for short-term reference pictures
*
************************************************************************
*/
static void reorder_long_term PARGS4(StorablePicture **RefPicListX, int num_ref_idx_lX_active_minus1, int LongTermPicNum, int *refIdxLX)
{
	int cIdx, nIdx;

	StorablePicture *picLX;

	picLX = get_long_term_pic ARGS1(LongTermPicNum);

	for( cIdx = num_ref_idx_lX_active_minus1+1; cIdx > *refIdxLX; cIdx-- )
		RefPicListX[ cIdx ] = RefPicListX[ cIdx - 1];

	RefPicListX[ (*refIdxLX)++ ] = picLX;

	nIdx = *refIdxLX;

	for( cIdx = *refIdxLX; cIdx <= num_ref_idx_lX_active_minus1+1; cIdx++ )
		if (RefPicListX[ cIdx ])
			if( (!RefPicListX[ cIdx ]->is_long_term ) ||  (RefPicListX[ cIdx ]->long_term_pic_num != LongTermPicNum ))
				RefPicListX[ nIdx++ ] = RefPicListX[ cIdx ];
}


/*!
************************************************************************
* \brief
*    Reordering process for reference picture lists
*
************************************************************************
*/
void reorder_ref_pic_list PARGS8(StorablePicture **list, int *list_size, int num_ref_idx_lX_active_minus1, 
								 int *remapping_of_pic_nums_idc, int *abs_diff_pic_num_minus1, 
								 int *long_term_pic_idx, int *abs_diff_view_idx_minus1, int listIdx
								 )
{
	int i, j;

	int maxPicNum, currPicNum, picNumLXNoWrap, picNumLXPred, picNumLX;
	int refIdxLX = 0;
	int maxViewIdx, absDiffViewIdx, picViewIdxLXPred, picViewIdxLX, targetViewId;
	int view_index = (IMGPAR currentSlice) ? (IMGPAR currentSlice->viewIndex) : 0;
	int cIdx, nIdx;

	if (active_sps.num_views > 1 && view_index > 0) {

		picViewIdxLXPred = -1;

		if (listIdx == 0) {
			if (img->currentSlice->anchorPicFlag) {
				maxViewIdx = active_sps.num_anchor_refs_l0[view_index];
			} else {
				maxViewIdx = active_sps.num_non_anchor_refs_l0[view_index];
			}

		} else {
			if (img->currentSlice->anchorPicFlag) {
				maxViewIdx = active_sps.num_anchor_refs_l1[view_index];
			} else {
				maxViewIdx = active_sps.num_non_anchor_refs_l1[view_index];
			}
		}
	}

	if (IMGPAR structure==FRAME)
	{
		maxPicNum  = IMGPAR MaxFrameNum;
		currPicNum = IMGPAR frame_num;
	}
	else
	{
		maxPicNum  = 2 * IMGPAR MaxFrameNum;
		currPicNum = 2 * IMGPAR frame_num + 1;
	}

	picNumLXPred = currPicNum;

	for (i=0; remapping_of_pic_nums_idc[i]!=3; i++)
	{
		if (remapping_of_pic_nums_idc[i]>5 || i > num_ref_idx_lX_active_minus1) {
			DEBUG_SHOW_ERROR_INFO ("[ERROR]Invalid remapping_of_pic_nums_idc command", 500);
			return;		//Error found, not return warning as it can be concealed in the picture
		}
      

		if (remapping_of_pic_nums_idc[i] < 2)
		{
			if (remapping_of_pic_nums_idc[i] == 0)
			{
				if( picNumLXPred - ( abs_diff_pic_num_minus1[i] + 1 ) < 0 )
					picNumLXNoWrap = picNumLXPred - ( abs_diff_pic_num_minus1[i] + 1 ) + maxPicNum;
				else
					picNumLXNoWrap = picNumLXPred - ( abs_diff_pic_num_minus1[i] + 1 );
			}
			else // (remapping_of_pic_nums_idc[i] == 1)
			{
				if( picNumLXPred + ( abs_diff_pic_num_minus1[i] + 1 )  >=  maxPicNum )
					picNumLXNoWrap = picNumLXPred + ( abs_diff_pic_num_minus1[i] + 1 ) - maxPicNum;
				else
					picNumLXNoWrap = picNumLXPred + ( abs_diff_pic_num_minus1[i] + 1 );
			}
			picNumLXPred = picNumLXNoWrap;

			if( picNumLXNoWrap > currPicNum )
				picNumLX = picNumLXNoWrap - maxPicNum;
			else
				picNumLX = picNumLXNoWrap;

			reorder_short_term ARGS4(list, num_ref_idx_lX_active_minus1, picNumLX, &refIdxLX);
		}
		else if (remapping_of_pic_nums_idc[i] == 2)//(remapping_of_pic_nums_idc[i] == 2)
		{
			reorder_long_term ARGS4(list, num_ref_idx_lX_active_minus1, long_term_pic_idx[i], &refIdxLX);
		} else if ((remapping_of_pic_nums_idc[i] == 4) || (remapping_of_pic_nums_idc[i] == 5)) {

			absDiffViewIdx = abs_diff_view_idx_minus1[i] + 1;

			if (remapping_of_pic_nums_idc[i] == 4) {

				if ( picViewIdxLXPred - absDiffViewIdx < 0 ) {
					picViewIdxLX = picViewIdxLXPred - absDiffViewIdx + maxViewIdx;
				} else {
					picViewIdxLX = picViewIdxLXPred - absDiffViewIdx;
				}

			} else {	//remapping_of_pic_nums_idc[i] == 5

				if ( picViewIdxLXPred + absDiffViewIdx >= maxViewIdx ) {
					picViewIdxLX = picViewIdxLXPred + absDiffViewIdx - maxViewIdx;
				} else {
					picViewIdxLX = picViewIdxLXPred + absDiffViewIdx;
				}

			}

			if (listIdx == 0) {
				if (img->currentSlice->anchorPicFlag) {
					targetViewId = active_sps.anchor_refs_l0[view_index][picViewIdxLX];
				} else {
					targetViewId = active_sps.non_anchor_refs_l0[view_index][picViewIdxLX];
				}
			} else {
				if (img->currentSlice->anchorPicFlag) {
					targetViewId = active_sps.anchor_refs_l1[view_index][picViewIdxLX];
				} else {
					targetViewId = active_sps.non_anchor_refs_l1[view_index][picViewIdxLX];
				}
			}

			FrameStore  * fs_inter_view;

			int target_view_index = GetViewIndex ARGS1 (targetViewId);
			for ( j = 0; j < stream_global->m_dpb.used_size_on_view[target_view_index]; j++) {

				fs_inter_view = stream_global->m_dpb.fs_on_view[target_view_index][j];				

				if(IMGPAR structure==FRAME)
				{
					if (!fs_inter_view->is_non_existent && fs_inter_view->is_used == 3 
						&& (fs_inter_view->poc == img->framepoc) && fs_inter_view->frame->inter_view_flag) {
							break;
					}
				}
				else if(IMGPAR structure == TOP_FIELD)
				{
					if (!fs_inter_view->is_non_existent && (fs_inter_view->is_used & TOP_FIELD) 
						&& (fs_inter_view->top_field->poc == img->framepoc) && fs_inter_view->top_field->inter_view_flag) {
							break;
					}
				}
				else if(IMGPAR structure == BOTTOM_FIELD)
				{
					if (!fs_inter_view->is_non_existent && (fs_inter_view->is_used & BOTTOM_FIELD) 
						&& (fs_inter_view->bottom_field->poc == img->framepoc) && fs_inter_view->bottom_field->inter_view_flag) {
							break;
					}
				}
				

			}

			if(j < stream_global->m_dpb.used_size_on_view[target_view_index])
			{
				for( cIdx = num_ref_idx_lX_active_minus1+1; cIdx > refIdxLX; cIdx-- ) {
					list[ cIdx ] = list[ cIdx - 1];
				}

				if(IMGPAR structure==FRAME)
					list[ refIdxLX++ ] = fs_inter_view->frame;
				else if(IMGPAR structure == TOP_FIELD)
					list[ refIdxLX++ ] = fs_inter_view->top_field;
				else if(IMGPAR structure == BOTTOM_FIELD)
					list[ refIdxLX++ ] = fs_inter_view->bottom_field;
				
				nIdx = refIdxLX;

				for( cIdx = refIdxLX; cIdx <= num_ref_idx_lX_active_minus1+1; cIdx++ ) {
					if (list[ cIdx ]) {			
						if( (list[ cIdx ]->is_long_term ) ||  (list[ cIdx ]->poc != img->ThisPOC ) || (list[cIdx]->view_id != targetViewId) ) {
							list[ nIdx++ ] = list[ cIdx ];
						}
					}
				}
			}

			picViewIdxLXPred = picViewIdxLX;


		}

	}
	// that's a definition
	*list_size = num_ref_idx_lX_active_minus1 + 1;
}



/*!
************************************************************************
* \brief
*    Update the list of frame stores that contain reference frames/fields
*
************************************************************************
*/
void update_ref_list PARGS1(unsigned int view_index)
{
	unsigned i, j;	

	for (i=0, j=0; i<dpb.used_size_on_view[view_index]; i++)
	{
		if (is_short_term_reference(dpb.fs_on_view[view_index][i]))
		{
			dpb.fs_ref_on_view[view_index][j++]=dpb.fs_on_view[view_index][i];
		}
	}

	dpb.ref_frames_in_buffer_on_view[view_index] = j;

	while (j<dpb.size_on_view[view_index])
	{
		dpb.fs_ref_on_view[view_index][j++]=NULL;
	}
}


/*!
************************************************************************
* \brief
*    Update the list of frame stores that contain long-term reference 
*    frames/fields
*
************************************************************************
*/
void update_ltref_list PARGS1(unsigned int view_index)
{
	unsigned i, j;	

	for (i=0, j=0; i<dpb.used_size_on_view[view_index]; i++)
	{
		if (is_long_term_reference(dpb.fs_on_view[view_index][i]))
		{
			dpb.fs_ltref_on_view[view_index][j++]=dpb.fs_on_view[view_index][i];
		}
	}

	dpb.ltref_frames_in_buffer_on_view[view_index]=j;

	while (j<dpb.size_on_view[view_index])
	{
		dpb.fs_ltref_on_view[view_index][j++]=NULL;
	}
}

/*!
************************************************************************
* \brief
*    Perform Memory management for idr pictures
*
************************************************************************
*/
static CREL_RETURN idr_memory_management PARGS3(StorablePicture* p, int view_index, BOOL bFlush)
{
	int ret;
	//assert (p->idr_flag);	

	//Terry: For display smoothly, we skip this flag and force to output the decoded frames.
	/*if (p->no_output_of_prior_pics_flag)
	{
	unsigned i;

	// free all stored pictures
	for (i=0; i<dpb.used_size; i++)
	{
	// reset all reference settings
	free_frame_store ARGS2(dpb.fs[i], -1);
	dpb.fs[i] = alloc_frame_store();
	}
	for (i=0; i<dpb.ref_frames_in_buffer; i++)
	{
	dpb.fs_ref[i]=NULL;
	}
	for (i=0; i<dpb.ltref_frames_in_buffer; i++)
	{
	dpb.fs_ltref[i]=NULL;
	}
	dpb.used_size=0;
	}
	else
	*/

	if(bFlush)
	{
		ret = flush_dpb ARGS1(view_index);
		if (FAILED(ret)) {
			return ret;
		}

		ret = init_dpb ARGS1(view_index);
		if (FAILED(ret)) {
			return ret;
		}

		update_ref_list ARGS1(view_index);
		update_ltref_list ARGS1(view_index);
				
		dpb.last_picture[view_index] = NULL;
		

		dpb.last_output_poc = INT_MIN;
	}

	if (p->long_term_reference_flag)
	{
		dpb.max_long_term_pic_idx_on_view[view_index] = 0;
		p->is_long_term           = 1;
		p->long_term_frame_idx    = 0;
	}
	else
	{
		dpb.max_long_term_pic_idx_on_view[view_index] = -1;
		p->is_long_term           = 0;
	}

	return CREL_OK;
}

/*!
************************************************************************
* \brief
*    Perform Sliding window decoded reference picture marking process
*
************************************************************************
*/
static void sliding_window_memory_management PARGS1(StorablePicture* p)
{
	unsigned i;
	int view_index = (IMGPAR firstSlice) ? (IMGPAR firstSlice->viewIndex) : 0;

	//assert (!p->idr_flag);
	// if this is a reference pic with sliding sliding window, unmark first ref frame
	if (dpb.ref_frames_in_buffer_on_view[view_index]==dpb.num_ref_frames - dpb.ltref_frames_in_buffer_on_view[view_index])
	{
		for (i=0; i<dpb.used_size_on_view[view_index];i++)
		{
			if (dpb.fs_on_view[view_index][i]->is_reference  && (!(dpb.fs_on_view[view_index][i]->is_long_term)))
			{
				unmark_for_reference(dpb.fs_on_view[view_index][i]);
				update_ref_list ARGS1(view_index);
				break;
			}
		}
	}

	p->is_long_term = 0;
}

/*!
************************************************************************
* \brief
*    Calculate picNumX
************************************************************************
*/
static int get_pic_num_x (StorablePicture *p, int difference_of_pic_nums_minus1)
{
	int currPicNum;

	if (p->structure == FRAME)
		currPicNum = p->frame_num;
	else 
		currPicNum = 2 * p->frame_num + 1;

	return currPicNum - (difference_of_pic_nums_minus1 + 1);
}


/*!
************************************************************************
* \brief
*    Adaptive Memory Management: Mark short term picture unused
************************************************************************
*/
static void mm_unmark_short_term_for_reference PARGS2(StorablePicture *p, int difference_of_pic_nums_minus1)
{
	int picNumX;

	unsigned i;
	unsigned int view_index = (IMGPAR firstSlice) ? (IMGPAR firstSlice->viewIndex) : 0;

	picNumX = get_pic_num_x(p, difference_of_pic_nums_minus1);

	for (i=0; i<dpb.ref_frames_in_buffer_on_view[view_index]; i++)
	{
		if (p->structure == FRAME)
		{
			if ((dpb.fs_ref_on_view[view_index][i]->is_reference==(TOP_FIELD|BOTTOM_FIELD)) && (dpb.fs_ref_on_view[view_index][i]->is_long_term==0))
			{
				if (dpb.fs_ref_on_view[view_index][i]->frame->pic_num == picNumX)
				{
					unmark_for_reference(dpb.fs_ref_on_view[view_index][i]);
					return;
				}
			}
		}
		else
		{
			if ((dpb.fs_ref_on_view[view_index][i]->is_reference & TOP_FIELD) && (!(dpb.fs_ref_on_view[view_index][i]->is_long_term & TOP_FIELD)))
			{
				if (dpb.fs_ref_on_view[view_index][i]->top_field->pic_num == picNumX)
				{
					dpb.fs_ref_on_view[view_index][i]->top_field->used_for_reference = 0;
					dpb.fs_ref_on_view[view_index][i]->is_reference &= BOTTOM_FIELD;
					if (dpb.fs_ref_on_view[view_index][i]->is_used == (TOP_FIELD|BOTTOM_FIELD))
					{
						dpb.fs_ref_on_view[view_index][i]->frame->used_for_reference = 0;
					}
					return;
				}
			}
			if ((dpb.fs_ref_on_view[view_index][i]->is_reference & BOTTOM_FIELD) && (!(dpb.fs_ref_on_view[view_index][i]->is_long_term & BOTTOM_FIELD)))
			{
				if (dpb.fs_ref_on_view[view_index][i]->bottom_field->pic_num == picNumX)
				{
					dpb.fs_ref_on_view[view_index][i]->bottom_field->used_for_reference = 0;
					dpb.fs_ref_on_view[view_index][i]->is_reference &= TOP_FIELD;
					if (dpb.fs_ref_on_view[view_index][i]->is_used == (TOP_FIELD|BOTTOM_FIELD))
					{
						dpb.fs_ref_on_view[view_index][i]->frame->used_for_reference = 0;
					}
					return;
				}
			}
		}
	}
}


/*!
************************************************************************
* \brief
*    Adaptive Memory Management: Mark long term picture unused
************************************************************************
*/
static void mm_unmark_long_term_for_reference PARGS2(StorablePicture *p, int long_term_pic_num)
{
	unsigned i;
	unsigned int view_index = (IMGPAR firstSlice) ? (IMGPAR firstSlice->viewIndex) : 0;

	for (i=0; i<dpb.ltref_frames_in_buffer_on_view[view_index]; i++)
	{
		if (p->structure == FRAME)
		{
			if ((dpb.fs_ltref_on_view[view_index][i]->is_reference==(TOP_FIELD|BOTTOM_FIELD)) && (dpb.fs_ltref_on_view[view_index][i]->is_long_term==(TOP_FIELD|BOTTOM_FIELD)))
			{
				if (dpb.fs_ltref_on_view[view_index][i]->frame->long_term_pic_num == long_term_pic_num)
				{
					unmark_for_long_term_reference(dpb.fs_ltref_on_view[view_index][i]);
				}
			}
		}
		else
		{
			if ((dpb.fs_ltref_on_view[view_index][i]->is_reference & TOP_FIELD) && ((dpb.fs_ltref_on_view[view_index][i]->is_long_term & TOP_FIELD)))
			{
				if (dpb.fs_ltref_on_view[view_index][i]->top_field->long_term_pic_num == long_term_pic_num)
				{
					dpb.fs_ltref_on_view[view_index][i]->top_field->used_for_reference = 0;
					dpb.fs_ltref_on_view[view_index][i]->top_field->is_long_term = 0;
					dpb.fs_ltref_on_view[view_index][i]->is_reference &= BOTTOM_FIELD;
					dpb.fs_ltref_on_view[view_index][i]->is_long_term &= BOTTOM_FIELD;
					if (dpb.fs_ltref_on_view[view_index][i]->is_used == (TOP_FIELD|BOTTOM_FIELD))
					{
						dpb.fs_ltref_on_view[view_index][i]->frame->used_for_reference = 0;
						dpb.fs_ltref_on_view[view_index][i]->frame->is_long_term = 0;
					}
					return;
				}
			}
			if ((dpb.fs_ltref_on_view[view_index][i]->is_reference & BOTTOM_FIELD) && ((dpb.fs_ltref_on_view[view_index][i]->is_long_term & BOTTOM_FIELD)))
			{
				if (dpb.fs_ltref_on_view[view_index][i]->bottom_field->long_term_pic_num == long_term_pic_num)
				{
					dpb.fs_ltref_on_view[view_index][i]->bottom_field->used_for_reference = 0;
					dpb.fs_ltref_on_view[view_index][i]->bottom_field->is_long_term = 0;
					dpb.fs_ltref_on_view[view_index][i]->is_reference &= TOP_FIELD;
					dpb.fs_ltref_on_view[view_index][i]->is_long_term &= TOP_FIELD;
					if (dpb.fs_ltref_on_view[view_index][i]->is_used == (TOP_FIELD|BOTTOM_FIELD))
					{
						dpb.fs_ltref_on_view[view_index][i]->frame->used_for_reference = 0;
						dpb.fs_ltref_on_view[view_index][i]->frame->is_long_term = 0;
					}
					return;
				}
			}
		}
	}
}


/*!
************************************************************************
* \brief
*    Mark a long-term reference frame or complementary field pair unused for referemce
************************************************************************
*/
static void unmark_long_term_frame_for_reference_by_frame_idx PARGS1(int long_term_frame_idx)
{
	unsigned i;
	unsigned int view_index = (IMGPAR firstSlice) ? (IMGPAR firstSlice->viewIndex) : 0;

	for(i=0; i<dpb.ltref_frames_in_buffer_on_view[view_index]; i++)
	{
		if (dpb.fs_ltref_on_view[view_index][i]->long_term_frame_idx == long_term_frame_idx)
			unmark_for_long_term_reference(dpb.fs_ltref_on_view[view_index][i]);
	}
}

/*!
************************************************************************
* \brief
*    Mark a long-term reference field unused for reference only if it's not
*    the complementary field of the picture indicated by picNumX
************************************************************************
*/
static void unmark_long_term_field_for_reference_by_frame_idx PARGS5(PictureStructure structure, int long_term_frame_idx, int mark_current, unsigned curr_frame_num, int curr_pic_num)
{
	unsigned i;
	unsigned int view_index = (IMGPAR firstSlice) ? (IMGPAR firstSlice->viewIndex) : 0;

	//assert(structure!=FRAME); //Guaranteed by caller
	if (curr_pic_num<0)
		curr_pic_num+=(2*IMGPAR MaxFrameNum);

	for(i=0; i<dpb.ltref_frames_in_buffer_on_view[view_index]; i++)
	{
		if (dpb.fs_ltref_on_view[view_index][i]->long_term_frame_idx == long_term_frame_idx)
		{
			if (structure == TOP_FIELD)
			{
				if ((dpb.fs_ltref_on_view[view_index][i]->is_long_term == (TOP_FIELD|BOTTOM_FIELD)))
				{
					unmark_for_long_term_reference(dpb.fs_ltref_on_view[view_index][i]);
				}
				else
				{
					if ((dpb.fs_ltref_on_view[view_index][i]->is_long_term == TOP_FIELD))
					{
						unmark_for_long_term_reference(dpb.fs_ltref_on_view[view_index][i]);
					}
					else
					{
						if (mark_current)
						{
							if (dpb.last_picture && dpb.last_picture[view_index])
							{
								if ( ( dpb.last_picture[view_index] != dpb.fs_ltref_on_view[view_index][i] )|| dpb.last_picture[view_index]->frame_num != curr_frame_num)
									unmark_for_long_term_reference(dpb.fs_ltref_on_view[view_index][i]);
							}
							else
							{
								unmark_for_long_term_reference(dpb.fs_ltref_on_view[view_index][i]);
							}
						}
						else
						{
							if ((dpb.fs_ltref_on_view[view_index][i]->frame_num) != (unsigned)(curr_pic_num/2))
							{
								unmark_for_long_term_reference(dpb.fs_ltref_on_view[view_index][i]);
							}
						}
					}
				}
			}
			if (structure == BOTTOM_FIELD)
			{
				if ((dpb.fs_ltref_on_view[view_index][i]->is_long_term == (TOP_FIELD|BOTTOM_FIELD)))
				{
					unmark_for_long_term_reference(dpb.fs_ltref_on_view[view_index][i]);
				}
				else
				{
					if ((dpb.fs_ltref_on_view[view_index][i]->is_long_term == BOTTOM_FIELD))
					{
						unmark_for_long_term_reference(dpb.fs_ltref_on_view[view_index][i]);
					}
					else
					{
						if (mark_current)
						{
							if (dpb.last_picture && dpb.last_picture[view_index])
							{
								if ( ( dpb.last_picture[view_index] != dpb.fs_ltref_on_view[view_index][i] )|| dpb.last_picture[view_index]->frame_num != curr_frame_num)
									unmark_for_long_term_reference(dpb.fs_ltref_on_view[view_index][i]);
							}
							else
							{
								unmark_for_long_term_reference(dpb.fs_ltref_on_view[view_index][i]);
							}
						}
						else
						{
							if ((dpb.fs_ltref_on_view[view_index][i]->frame_num) != (unsigned)(curr_pic_num/2))
							{
								unmark_for_long_term_reference(dpb.fs_ltref_on_view[view_index][i]);
							}
						}
					}
				}
			}
		}
	}
}


/*!
************************************************************************
* \brief
*    mark a picture as long-term reference
************************************************************************
*/
static CREL_RETURN mark_pic_long_term PARGS3(StorablePicture* p, int long_term_frame_idx, int picNumX)
{
	unsigned i;
	int add_top, add_bottom;
	unsigned int view_index = (IMGPAR firstSlice) ? (IMGPAR firstSlice->viewIndex) : 0;

	if (p->structure == FRAME)
	{
		for (i=0; i<dpb.ref_frames_in_buffer_on_view[view_index]; i++)
		{
			if (dpb.fs_ref_on_view[view_index][i]->is_reference == (TOP_FIELD|BOTTOM_FIELD))
			{
				if ((!dpb.fs_ref_on_view[view_index][i]->frame->is_long_term)&&(dpb.fs_ref_on_view[view_index][i]->frame->pic_num == picNumX))
				{
					dpb.fs_ref_on_view[view_index][i]->long_term_frame_idx = dpb.fs_ref_on_view[view_index][i]->frame->long_term_frame_idx
						= long_term_frame_idx;
					dpb.fs_ref_on_view[view_index][i]->frame->long_term_pic_num = long_term_frame_idx;
					dpb.fs_ref_on_view[view_index][i]->frame->is_long_term = 1;

					if (dpb.fs_ref_on_view[view_index][i]->top_field && dpb.fs_ref_on_view[view_index][i]->bottom_field)
					{
						dpb.fs_ref_on_view[view_index][i]->top_field->long_term_frame_idx = dpb.fs_ref_on_view[view_index][i]->bottom_field->long_term_frame_idx
							= long_term_frame_idx;
						dpb.fs_ref_on_view[view_index][i]->top_field->long_term_pic_num = long_term_frame_idx;
						dpb.fs_ref_on_view[view_index][i]->bottom_field->long_term_pic_num = long_term_frame_idx;

						dpb.fs_ref_on_view[view_index][i]->top_field->is_long_term = dpb.fs_ref_on_view[view_index][i]->bottom_field->is_long_term
							= 1;

					}
					dpb.fs_ref_on_view[view_index][i]->is_long_term = (TOP_FIELD|BOTTOM_FIELD);
					return CREL_OK;
				}
			}
		}
		DEBUG_SHOW_ERROR_INFO ("[ERROR]Warning: reference frame for long term marking not found\n");
		return CREL_WARNING_H264_STREAMERROR_LEVEL_1|CREL_WARNING_H264_ERROR_DPB;
	}
	else
	{
		if (p->structure == TOP_FIELD)
		{
			add_top    = 1;
			add_bottom = 0;
		}
		else
		{
			add_top    = 0;
			add_bottom = 1;
		}
		for (i=0; i<dpb.ref_frames_in_buffer_on_view[view_index]; i++)
		{
			if (dpb.fs_ref_on_view[view_index][i]->is_reference & TOP_FIELD)
			{
				if ((!dpb.fs_ref_on_view[view_index][i]->top_field->is_long_term)&&(dpb.fs_ref_on_view[view_index][i]->top_field->pic_num == picNumX))
				{
					if ((dpb.fs_ref_on_view[view_index][i]->is_long_term) && (dpb.fs_ref_on_view[view_index][i]->long_term_frame_idx != long_term_frame_idx))
					{
						DEBUG_SHOW_ERROR_INFO ("[ERROR]Warning: assigning long_term_frame_idx different from other field\n");
						return CREL_WARNING_H264_STREAMERROR_LEVEL_1|CREL_WARNING_H264_ERROR_DPB;
					}

					dpb.fs_ref_on_view[view_index][i]->long_term_frame_idx = dpb.fs_ref_on_view[view_index][i]->top_field->long_term_frame_idx 
						= long_term_frame_idx;
					dpb.fs_ref_on_view[view_index][i]->top_field->long_term_pic_num = 2 * long_term_frame_idx + add_top;
					dpb.fs_ref_on_view[view_index][i]->top_field->is_long_term = 1;
					dpb.fs_ref_on_view[view_index][i]->is_long_term |= TOP_FIELD;
					if (dpb.fs_ref_on_view[view_index][i]->is_long_term == (TOP_FIELD|BOTTOM_FIELD))
					{
						dpb.fs_ref_on_view[view_index][i]->frame->is_long_term = 1;
						dpb.fs_ref_on_view[view_index][i]->frame->long_term_frame_idx = dpb.fs_ref_on_view[view_index][i]->frame->long_term_pic_num = long_term_frame_idx;
					}
					return CREL_OK;
				}
			}
			if (dpb.fs_ref_on_view[view_index][i]->is_reference & BOTTOM_FIELD)
			{
				if ((!dpb.fs_ref_on_view[view_index][i]->bottom_field->is_long_term)&&(dpb.fs_ref_on_view[view_index][i]->bottom_field->pic_num == picNumX))
				{
					if ((dpb.fs_ref_on_view[view_index][i]->is_long_term) && (dpb.fs_ref_on_view[view_index][i]->long_term_frame_idx != long_term_frame_idx))
					{
						DEBUG_SHOW_ERROR_INFO ("[ERROR]Warning: assigning long_term_frame_idx different from other field\n");
						return CREL_WARNING_H264_STREAMERROR_LEVEL_1|CREL_WARNING_H264_ERROR_DPB;
					}

					dpb.fs_ref_on_view[view_index][i]->long_term_frame_idx = dpb.fs_ref_on_view[view_index][i]->bottom_field->long_term_frame_idx 
						= long_term_frame_idx;
					dpb.fs_ref_on_view[view_index][i]->bottom_field->long_term_pic_num = 2 * long_term_frame_idx + add_top;
					dpb.fs_ref_on_view[view_index][i]->bottom_field->is_long_term = 1;
					dpb.fs_ref_on_view[view_index][i]->is_long_term |= BOTTOM_FIELD;
					if (dpb.fs_ref_on_view[view_index][i]->is_long_term == (TOP_FIELD|BOTTOM_FIELD))
					{
						dpb.fs_ref_on_view[view_index][i]->frame->is_long_term = 1;
						dpb.fs_ref_on_view[view_index][i]->frame->long_term_frame_idx = dpb.fs_ref_on_view[view_index][i]->frame->long_term_pic_num = long_term_frame_idx;
					}
					return CREL_OK;
				}
			}
		}
		DEBUG_SHOW_ERROR_INFO ("[ERROR]Warning: reference field for long term marking not found\n");
		return CREL_WARNING_H264_STREAMERROR_LEVEL_1|CREL_WARNING_H264_ERROR_DPB;
	}  
}


/*!
************************************************************************
* \brief
*    Assign a long term frame index to a short term picture
************************************************************************
*/
static CREL_RETURN mm_assign_long_term_frame_idx PARGS3(StorablePicture* p, int difference_of_pic_nums_minus1, int long_term_frame_idx)
{
	int picNumX;
	CREL_RETURN ret;
	unsigned int view_index = (IMGPAR firstSlice) ? (IMGPAR firstSlice->viewIndex) : 0;

	picNumX = get_pic_num_x(p, difference_of_pic_nums_minus1);

	// remove frames/fields with same long_term_frame_idx
	if (p->structure == FRAME)
	{
		unmark_long_term_frame_for_reference_by_frame_idx ARGS1(long_term_frame_idx);
	}
	else
	{
		unsigned i;
		PictureStructure structure = FRAME;

		for (i=0; i<dpb.ref_frames_in_buffer_on_view[view_index]; i++)
		{
			if (dpb.fs_ref_on_view[view_index][i]->is_reference & TOP_FIELD)
			{
				if (dpb.fs_ref_on_view[view_index][i]->top_field->pic_num == picNumX)
				{
					structure = TOP_FIELD;
					break;
				}
			}
			if (dpb.fs_ref_on_view[view_index][i]->is_reference & BOTTOM_FIELD)
			{
				if (dpb.fs_ref_on_view[view_index][i]->bottom_field->pic_num == picNumX)
				{
					structure = BOTTOM_FIELD;
					break;
				}
			}
		}
		if (structure==FRAME)
		{
			//DEBUG_SHOW_ERROR_INFO ("[ERROR]field for long term marking not found",200);
			return CREL_WARNING_H264_STREAMERROR_LEVEL_1|CREL_WARNING_H264_ERROR_DPB;
		}

		unmark_long_term_field_for_reference_by_frame_idx ARGS5(structure, long_term_frame_idx, 0, 0, picNumX);
	}

	ret = mark_pic_long_term ARGS3(p, long_term_frame_idx, picNumX);
	if (FAILED(ret)) {
		return ret;
	}

	return CREL_OK;
}

/*!
************************************************************************
* \brief
*    Set new max long_term_frame_idx
************************************************************************
*/
void mm_update_max_long_term_frame_idx PARGS2(int max_long_term_frame_idx_plus1, unsigned int view_index)
{
	unsigned i;	

	dpb.max_long_term_pic_idx_on_view[view_index] = max_long_term_frame_idx_plus1 - 1;

	// check for invalid frames
	for (i=0; i<dpb.ltref_frames_in_buffer_on_view[view_index]; i++)
	{
		if (dpb.fs_ltref_on_view[view_index][i]->long_term_frame_idx > dpb.max_long_term_pic_idx_on_view[view_index])
		{
			unmark_for_long_term_reference(dpb.fs_ltref_on_view[view_index][i]);
		}
	}
}


/*!
************************************************************************
* \brief
*    Mark all long term reference pictures unused for reference
************************************************************************
*/
static void mm_unmark_all_long_term_for_reference PARGS1(unsigned int view_index)
{
	mm_update_max_long_term_frame_idx ARGS2(0, view_index);
}

/*!
************************************************************************
* \brief
*    Mark all short term reference pictures unused for reference
************************************************************************
*/
static void mm_unmark_all_short_term_for_reference PARGS1(unsigned int view_index)
{
	unsigned int i;	

	for (i=0; i<dpb.ref_frames_in_buffer_on_view[view_index]; i++)
	{
		unmark_for_reference(dpb.fs_ref_on_view[view_index][i]);
	}
	update_ref_list ARGS1(view_index);
}


/*!
************************************************************************
* \brief
*    Mark the current picture used for long term reference
************************************************************************
*/
static void mm_mark_current_picture_long_term PARGS2(StorablePicture *p, int long_term_frame_idx)
{
	// remove long term pictures with same long_term_frame_idx
	if (p->structure == FRAME)
	{
		unmark_long_term_frame_for_reference_by_frame_idx ARGS1(long_term_frame_idx);
	}
	else
	{
		unmark_long_term_field_for_reference_by_frame_idx ARGS5(p->structure, long_term_frame_idx, 1, p->pic_num, 0);
	}

	p->is_long_term = 1;
	p->long_term_frame_idx = long_term_frame_idx;
}


/*!
************************************************************************
* \brief
*    Perform Adaptive memory control decoded reference picture marking process
************************************************************************
*/
static CREL_RETURN adaptive_memory_management PARGS2(StorablePicture* p, unsigned int view_index)
{
	DecRefPicMarking_t *tmp_drpm;
	int ret;

	IMGPAR last_has_mmco_5 = 0;

	//assert (!p->idr_flag);	// No necessary as guarantted by caller
	//assert (p->adaptive_ref_pic_buffering_flag);

	while (p->dec_ref_pic_marking_buffer)
	{
		tmp_drpm = p->dec_ref_pic_marking_buffer;
		switch (tmp_drpm->memory_management_control_operation)
		{
		case 0:
			if (tmp_drpm->Next != NULL)
			{
				DEBUG_SHOW_ERROR_INFO ("[ERROR]memory_management_control_operation = 0 not last operation in buffer", 500);
				return CREL_WARNING_H264_STREAMERROR_LEVEL_1|CREL_WARNING_H264_ERROR_DPB;
			}
			break;
		case 1:
			mm_unmark_short_term_for_reference ARGS2(p, tmp_drpm->difference_of_pic_nums_minus1);
			update_ref_list ARGS1(view_index);
			break;
		case 2:
			mm_unmark_long_term_for_reference ARGS2(p, tmp_drpm->long_term_pic_num);
			update_ltref_list ARGS1(view_index);
			break;
		case 3:
			ret = mm_assign_long_term_frame_idx ARGS3(p, tmp_drpm->difference_of_pic_nums_minus1, tmp_drpm->long_term_frame_idx);
			if (FAILED(ret)) {
				return ret;
			}
			update_ref_list ARGS1(view_index);
			update_ltref_list ARGS1(view_index);
			break;
		case 4:
			mm_update_max_long_term_frame_idx ARGS2(tmp_drpm->max_long_term_frame_idx_plus1, view_index);
			update_ltref_list ARGS1(view_index);
			break;
		case 5:
			mm_unmark_all_short_term_for_reference ARGS1(view_index);
			mm_unmark_all_long_term_for_reference ARGS1(view_index);
			IMGPAR last_has_mmco_5 = 1;
			break;
		case 6:
			mm_mark_current_picture_long_term ARGS2(p, tmp_drpm->long_term_frame_idx);
			ret = check_num_ref ARGS0();
			if (FAILED(ret)) {
				return ret;
			}
			break;
		default:
			DEBUG_SHOW_ERROR_INFO ("[ERROR]invalid memory_management_control_operation in buffer", 500);
			return CREL_WARNING_H264_STREAMERROR_LEVEL_1|CREL_WARNING_H264_ERROR_DPB;
		}
		p->dec_ref_pic_marking_buffer = tmp_drpm->Next;
		_aligned_free (tmp_drpm);
	}
	if ( IMGPAR last_has_mmco_5 )
	{
		p->pic_num = p->frame_num = 0;

		switch (p->structure)
		{
		case TOP_FIELD:
			{
				p->poc = p->top_poc = IMGPAR toppoc =0;
				break;
			}
		case BOTTOM_FIELD:
			{
				p->poc = p->bottom_poc = IMGPAR bottompoc = 0;
				break;
			}
		case FRAME:
			{
				p->top_poc    -= p->poc;
				p->bottom_poc -= p->poc;

				IMGPAR toppoc = p->top_poc;
				IMGPAR bottompoc = p->bottom_poc;

				p->poc = min (p->top_poc, p->bottom_poc);
				IMGPAR framepoc = p->poc;
				break;
			}
		}
		IMGPAR PreviousPOC = IMGPAR ThisPOC;
#ifdef _COLLECT_PIC_
		if(stream_global->bMVC == TRUE && IMGPAR firstSlice->field_pic_flag == TRUE)
			stream_global->PreviousPOC[view_index] = IMGPAR ThisPOC;
		else
			stream_global->PreviousPOC[0] = IMGPAR ThisPOC;
#endif
		IMGPAR ThisPOC = p->poc;
		ret = flush_dpb ARGS1(view_index);
		if (FAILED(ret)) {
			return ret;
		}
		ret = init_dpb ARGS1(view_index);
		if (FAILED(ret)) {
			return ret;
		}
	}
	return CREL_OK;
}


/*!
************************************************************************
* \brief
*    Store a picture in DPB. This includes cheking for space in DPB and 
*    flushing frames.
*    If we received a frame, we need to check for a new store, if we
*    got a field, check if it's the second field of an already allocated
*    store.
*
* \param p
*    Picture to be stored
*
************************************************************************
*/
CREL_RETURN store_picture_in_dpb PARGS1(StorablePicture* p)
{
	unsigned i, ret;
	int poc, pos;

	int view_index = (IMGPAR firstSlice) ? (IMGPAR firstSlice->viewIndex) : 0;
	p->view_id = IMGPAR firstSlice->viewId;
	p->view_index = IMGPAR firstSlice->viewIndex;
	p->inter_view_flag = IMGPAR firstSlice->interViewFlag;
	// diagnostics
	DEBUG_SHOW_SW_INFO ("Storing (%s) non-ref pic with frame_num #%d\n", (p->structure == FRAME)?"FRAME":(p->structure == TOP_FIELD)?"TOP_FIELD":"BOTTOM_FIELD", p->pic_num);
	// if frame, check for new store, 
	assert (p!=NULL);
	IMGPAR last_has_mmco_5=0;
	IMGPAR last_pic_bottom_field = (p->structure == BOTTOM_FIELD);

	if (p->idr_flag) {
		if(p->view_index == 0)
		{
			for(int i = 0; i < stream_global->num_views; i++)
				ret = idr_memory_management ARGS3(p, i, true);
		}
		else
			ret = idr_memory_management ARGS3(p, p->view_index, false);
		if (FAILED(ret)) {
			return ret;
		}
	} else {
		// adaptive memory management
		if (p->used_for_reference && (p->adaptive_ref_pic_buffering_flag)) {
			ret = adaptive_memory_management ARGS2(p, view_index);
			if (FAILED(ret)) {
				return ret;
			}
		}
	}
	DecRefPicMarking_t *tmp_drpm;
	while(p->dec_ref_pic_marking_buffer)
	{
		tmp_drpm=p->dec_ref_pic_marking_buffer;

		p->dec_ref_pic_marking_buffer=tmp_drpm->Next;
		_aligned_free (tmp_drpm);
	}

	if ((p->structure==TOP_FIELD)||(p->structure==BOTTOM_FIELD))
	{
		// check for frame store with same pic_number
		if (dpb.last_picture && dpb.last_picture[view_index])
		{
			if ((int)dpb.last_picture[view_index]->frame_num == p->pic_num)
			{
				if (((p->structure==TOP_FIELD)&&(dpb.last_picture[view_index]->is_used==BOTTOM_FIELD))||((p->structure==BOTTOM_FIELD)&&(dpb.last_picture[view_index]->is_used==TOP_FIELD)))
				{
					//if ((p->used_for_reference && (dpb.last_picture->is_orig_reference!=0))||
					//	(!p->used_for_reference && (dpb.last_picture->is_orig_reference==0)))
					{
						ret = insert_picture_in_dpb ARGS2(dpb.last_picture[view_index], p);
						if (FAILED(ret)) {
							return ret;
						}
						update_ref_list ARGS1(view_index);
						update_ltref_list ARGS1(view_index);
						//         dump_dpb();
#ifdef _COLLECT_PIC_		  			
						for (unsigned int j=0; j<storable_picture_count; j++)
						{				
							if (storable_picture_map[j]->used_for_first_field_reference == dpb.last_picture[view_index]->poc)
								storable_picture_map[j]->used_for_first_field_reference = 0;			
						}
#endif

						dpb.last_picture[view_index] = NULL;
						return CREL_OK;
					}
				}
			}
		}

		// Check out_buffer frame store with same pic_number, add here to stop error propogation
		if ( p->pic_num == out_buffer->frame_num ) {
			if (((p->structure==TOP_FIELD)&&(out_buffer->is_used==BOTTOM_FIELD))
				||((p->structure==BOTTOM_FIELD)&&(out_buffer->is_used==TOP_FIELD))){
					ret = direct_output ARGS2(p, p_out);
					if (FAILED(ret)) {
						return ret;
					}

#ifdef _COLLECT_PIC_
					for (unsigned int j=0; j<storable_picture_count; j++)
					{				
						if (storable_picture_map[j]->used_for_first_field_reference == p->poc)
							storable_picture_map[j]->used_for_first_field_reference = 0;			
					}
#endif
					return CREL_OK;

			}
		}


	}

	// this is a frame or a field which has no stored complementary field

	// sliding window, if necessary
	if ((!p->idr_flag)&&(p->used_for_reference && (!p->adaptive_ref_pic_buffering_flag)))
	{
		sliding_window_memory_management ARGS1(p);
	} 

#ifdef _COLLECT_PIC_
#if defined(_HW_ACCEL_)
	if(g_DXVAVer==IviNotDxva 
		|| g_DXVAMode==E_H264_DXVA_ATI_PROPRIETARY_A 
		|| g_DXVAMode==E_H264_DXVA_NVIDIA_PROPRIETARY_A
		|| g_DXVAMode==E_H264_DXVA_MODE_A
		|| g_DXVAMode==E_H264_DXVA_MODE_C
		|| IMGPAR Hybrid_Decoding == 5) //FIXME: add nvidiaba, do we need this or not??
#endif
	{
		if (IMGPAR currentSlice->structure != FRAME && IMGPAR currentSlice->m_pic_combine_status != 0 && p->used_for_reference && (p->non_existing == 0))  
			gen_field_ref_ids ARGS1(p);	  
	}
#endif

	// first try to remove unused frames
	if (dpb.used_size_on_view[view_index]==dpb.size_on_view[view_index])
	{
		remove_unused_frame_from_dpb ARGS1(view_index);
	}

	// then output frames until one can be removed
	while (dpb.used_size_on_view[view_index]==dpb.size_on_view[view_index])
	{
		
		// non-reference frames may be output directly
		if (!p->used_for_reference)
		{
			ret = get_smallest_poc ARGS3(&poc, &pos, view_index);
			if (FAILED(ret)) {
				return ret;
			}

			if ((-1==pos) || (p->poc < poc))
			{
				ret = direct_output ARGS2(p, p_out);
				if (FAILED(ret)) {
					return ret;
				}

#ifdef _COLLECT_PIC_
				for (unsigned int j=0; j<storable_picture_count; j++)
				{				
					if (storable_picture_map[j]->used_for_first_field_reference == p->poc)
						storable_picture_map[j]->used_for_first_field_reference = 0;			
				}
#endif
				return CREL_OK;
			}
		}
		// flush a frame
		
		ret = output_one_frame_from_dpb ARGS1(view_index);
		if (FAILED(ret)) {
			return ret;
		}
	}

	// check for duplicate frame number in short term reference buffer
	if ((p->used_for_reference)&&(!p->is_long_term))
	{
		for (i=0; i<dpb.ref_frames_in_buffer_on_view[IMGPAR firstSlice->viewIndex]; i++)
		{
			if (dpb.fs_ref_on_view[IMGPAR firstSlice->viewIndex][i]->frame_num == p->frame_num)
			{
				DEBUG_SHOW_ERROR_INFO("[ERROR]duplicate frame_num im short-term reference picture buffer", 500);
				//release_storable_picture ARGS2(p,1);
				return CREL_WARNING_H264_STREAMERROR_LEVEL_1|CREL_WARNING_H264_ERROR_DPB;
			}
		}

	}
	// store at end of buffer
	//  printf ("store frame/field at pos %d\n",dpb.used_size);
	ret = insert_picture_in_dpb ARGS2(dpb.fs_on_view[view_index][dpb.used_size_on_view[view_index]],p);
	if (FAILED(ret)) {
		return ret;
	}

	if (p->structure != FRAME)
	{
		dpb.last_picture[view_index] = dpb.fs_on_view[view_index][dpb.used_size_on_view[view_index]];
	}
	else
	{
		dpb.last_picture[view_index] = NULL;
	}

	dpb.used_size_on_view[view_index]++;

	update_ref_list ARGS1(view_index);
	update_ltref_list ARGS1(view_index);

	ret = check_num_ref ARGS0();
	if (FAILED(ret)) {
		return ret;
	}

	// dump_dpb();
	return CREL_OK;
}

/*!
************************************************************************
* \brief
*    Insert the picture into the DPB. A free DPB position is necessary
*    for frames, .
*
* \param fs
*    FrameStore into which the picture will be inserted
* \param p
*    StorablePicture to be inserted
*
************************************************************************
*/
static CREL_RETURN insert_picture_in_dpb PARGS2(FrameStore* fs, StorablePicture* p)
{
	CREL_RETURN ret;
	DEBUG_SHOW_SW_INFO ("insert (%s) pic with frame_num #%d, poc %d\n", (p->structure == FRAME)?"FRAME":(p->structure == TOP_FIELD)?"TOP_FIELD":"BOTTOM_FIELD", p->pic_num, p->poc);
	//assert (p!=NULL);
	//assert (fs!=NULL);	//Guaranteed by Caller
	switch (p->structure)
	{
	case FRAME: 
		fs->frame = p;
		fs->frame->repeat_first_field = (IMGPAR firstSlice->m_nDispPicStructure==5 || IMGPAR firstSlice->m_nDispPicStructure==6);
		fs->frame->top_field_first = (IMGPAR firstSlice->m_nDispPicStructure>2 && IMGPAR firstSlice->m_nDispPicStructure<7)?(IMGPAR firstSlice->m_nDispPicStructure&1):1;
		if (g_dwYCCBufferLen && IMGPAR firstSlice->picture_type==I_SLICE)
		{
			memcpy((void*)fs->frame->pbYCCBuffer, (void*)g_pbYCCBuffer, 12);
			fs->frame->dwYCCBufferLen = g_dwYCCBufferLen;
			g_dwYCCBufferLen = 0;
		}
		fs->is_used = (TOP_FIELD|BOTTOM_FIELD);
		if (p->used_for_reference || p->inter_view_flag)
		{
			if(p->used_for_reference)
			{
				fs->is_reference = (TOP_FIELD|BOTTOM_FIELD);
				fs->is_orig_reference = (TOP_FIELD|BOTTOM_FIELD);
				if (p->is_long_term)
				{
					fs->is_long_term = (TOP_FIELD|BOTTOM_FIELD);
					fs->long_term_frame_idx = p->long_term_frame_idx;
				}
			}
			// generate field views
#ifdef _HW_ACCEL_
			if(g_DXVAVer && (g_DXVAMode==E_H264_DXVA_ATI_PROPRIETARY_E || g_DXVAMode==E_H264_DXVA_MODE_E || g_DXVAMode==E_H264_DXVA_INTEL_MODE_E) && (IMGPAR Hybrid_Decoding!=5 || g_bRewindDecoder==FALSE))
				ret = dpb_split_field_common ARGS1(fs);
			else
#endif
			{
				if(IMGPAR direct_spatial_mv_pred_flag)
					ret = dpb_split_field_spatial ARGS1(fs);
				else
					ret = dpb_split_field_temporal ARGS1(fs);
			}

			if (FAILED(ret)) {
				return ret;
			}

		} else {	  
			fs->poc = fs->frame->poc;		
			fs->long_term_frame_idx = fs->frame->long_term_frame_idx;
		}    
		break;
	case TOP_FIELD:
		fs->top_field = p;
		fs->is_used |= TOP_FIELD;
		if (p->used_for_reference)
		{
			fs->is_reference |= TOP_FIELD;
			fs->is_orig_reference |= TOP_FIELD;
			if (p->is_long_term)
			{
				fs->is_long_term |= TOP_FIELD;
				fs->long_term_frame_idx = p->long_term_frame_idx;
			}
		}
		if (fs->is_used == (TOP_FIELD|BOTTOM_FIELD))
		{
			// generate frame view
#ifdef _HW_ACCEL_
			if(g_DXVAVer && (g_DXVAMode==E_H264_DXVA_ATI_PROPRIETARY_E || g_DXVAMode==E_H264_DXVA_MODE_E || g_DXVAMode==E_H264_DXVA_INTEL_MODE_E) && (IMGPAR Hybrid_Decoding!=5 || g_bRewindDecoder==FALSE))
				ret = dpb_combine_field_yuv ARGS1(fs);
			else
#endif
				ret = dpb_combine_field ARGS1(fs);

			if (FAILED(ret)) {
				return ret;
			}
		

			fs->frame->repeat_first_field = 0;
			fs->frame->top_field_first = 0;
			if (g_dwYCCBufferLen && IMGPAR firstSlice->picture_type==I_SLICE)
			{
				memcpy((void*)fs->frame->pbYCCBuffer, (void*)g_pbYCCBuffer, 12);
				fs->frame->dwYCCBufferLen = g_dwYCCBufferLen;
				g_dwYCCBufferLen = 0;
			}

			if(stream_global->m_is_MTMS || stream_global->m_bIsSingleThreadMode == TRUE)
				memcpy(&(fs->frame->m_CCCode), &(fs->bottom_field->m_CCCode), sizeof(H264_CC));
			else
				memcpy(&(fs->frame->m_CCCode), &(fs->top_field->m_CCCode), sizeof(H264_CC));			
		} 
		else
		{
			fs->poc = p->poc;
#if !defined(_COLLECT_PIC_)
			if (p->used_for_reference)
				gen_field_ref_ids ARGS1(p);
#endif
		}
		break;
	case BOTTOM_FIELD:
		fs->bottom_field = p;
		fs->is_used |= BOTTOM_FIELD;
		if (p->used_for_reference)
		{
			fs->is_reference |= BOTTOM_FIELD;
			fs->is_orig_reference |= BOTTOM_FIELD;
			if (p->is_long_term)
			{
				fs->is_long_term |= BOTTOM_FIELD;
				fs->long_term_frame_idx = p->long_term_frame_idx;
			}
		}
		if (fs->is_used == (TOP_FIELD|BOTTOM_FIELD))
		{
			// generate frame view
#ifdef _HW_ACCEL_
			if(g_DXVAVer && (g_DXVAMode==E_H264_DXVA_ATI_PROPRIETARY_E || g_DXVAMode==E_H264_DXVA_MODE_E || g_DXVAMode==E_H264_DXVA_INTEL_MODE_E) && (IMGPAR Hybrid_Decoding!=5 || g_bRewindDecoder==FALSE))
				ret = dpb_combine_field_yuv ARGS1(fs);
			else
#endif
				ret = dpb_combine_field ARGS1(fs);

			if (FAILED(ret)) {
				return ret;
			}

			fs->frame->repeat_first_field = 0;
			fs->frame->top_field_first = 1;
			if (g_dwYCCBufferLen && IMGPAR firstSlice->picture_type==I_SLICE)
			{
				memcpy((void*)fs->frame->pbYCCBuffer, (void*)g_pbYCCBuffer, 12);
				fs->frame->dwYCCBufferLen = g_dwYCCBufferLen;
				g_dwYCCBufferLen = 0;
			}

			if(stream_global->m_is_MTMS || stream_global->m_bIsSingleThreadMode == TRUE)
				memcpy(&(fs->frame->m_CCCode), &(fs->top_field->m_CCCode), sizeof(H264_CC));
			else
				memcpy(&(fs->frame->m_CCCode), &(fs->bottom_field->m_CCCode), sizeof(H264_CC));
		} 
		else
		{
			fs->poc = p->poc;
#if !defined(_COLLECT_PIC_)
			if (p->used_for_reference)
				gen_field_ref_ids ARGS1(p);
#endif
		}
		break;
	}
	fs->frame_num = p->pic_num;
	fs->is_output = p->is_output;

	return CREL_OK;

}

/*!
************************************************************************
* \brief
*    Check if one of the frames/fields in frame store is used for reference
************************************************************************
*/
int is_used_for_reference(FrameStore* fs)
{
#if !defined(_COLLECT_PIC_)
	if (fs->is_reference)
	{
		return 1;
	}

	if (fs->is_used == (TOP_FIELD|BOTTOM_FIELD)) // frame
	{
		if (fs->frame->used_for_reference)
		{
			return 1;
		}
	}

	if (fs->is_used & TOP_FIELD) // top field
	{
		if (fs->top_field)
		{
			if (fs->top_field->used_for_reference)
			{
				return 1;
			}
		}
	}

	if (fs->is_used & BOTTOM_FIELD) // bottom field
	{
		if (fs->bottom_field)
		{
			if (fs->bottom_field->used_for_reference)
			{
				return 1;
			}
		}
	}

#else

	if (fs->is_reference )
	{
		return 1;
	}

	if (fs->is_used == (TOP_FIELD|BOTTOM_FIELD)) // frame
	{
		if (fs->frame->used_for_reference)// || fs->frame->used_for_first_field_reference)
		{
			return 1;
		}
	}

	if (fs->is_used & TOP_FIELD) // top field
	{
		if (fs->top_field)
		{
			if (fs->top_field->used_for_reference)// || fs->top_field->used_for_first_field_reference)
			{
				return 1;
			}
		}
	}

	if (fs->is_used & BOTTOM_FIELD) // bottom field
	{
		if (fs->bottom_field)
		{
			if (fs->bottom_field->used_for_reference)// || fs->bottom_field->used_for_first_field_reference)
			{
				return 1;
			}
		}
	}
#endif

	return 0;
}


/*!
************************************************************************
* \brief
*    Check if one of the frames/fields in frame store is used for short-term reference
************************************************************************
*/
static int is_short_term_reference(FrameStore* fs)
{

	if (fs->is_used==(TOP_FIELD|BOTTOM_FIELD)) // frame
	{
		if ((fs->frame->used_for_reference)&&(!fs->frame->is_long_term))
		{
			return 1;
		}
	}

	if (fs->is_used & TOP_FIELD) // top field
	{
		if (fs->top_field)
		{
			if ((fs->top_field->used_for_reference)&&(!fs->top_field->is_long_term))
			{
				return 1;
			}
		}
	}

	if (fs->is_used & BOTTOM_FIELD) // bottom field
	{
		if (fs->bottom_field)
		{
			if ((fs->bottom_field->used_for_reference)&&(!fs->bottom_field->is_long_term))
			{
				return 1;
			}
		}
	}
	return 0;
}


/*!
************************************************************************
* \brief
*    Check if one of the frames/fields in frame store is used for short-term reference
************************************************************************
*/
static int is_long_term_reference(FrameStore* fs)
{

	if (fs->is_used==(TOP_FIELD|BOTTOM_FIELD)) // frame
	{
		if ((fs->frame->used_for_reference)&&(fs->frame->is_long_term))
		{
			return 1;
		}
	}

	if (fs->is_used & TOP_FIELD) // top field
	{
		if (fs->top_field)
		{
			if ((fs->top_field->used_for_reference)&&(fs->top_field->is_long_term))
			{
				return 1;
			}
		}
	}

	if (fs->is_used & BOTTOM_FIELD) // bottom field
	{
		if (fs->bottom_field)
		{
			if ((fs->bottom_field->used_for_reference)&&(fs->bottom_field->is_long_term))
			{
				return 1;
			}
		}
	}
	return 0;
}


/*!
************************************************************************
* \brief
*    remove one frame from DPB
************************************************************************
*/
void remove_frame_from_dpb PARGS2(int pos, unsigned int view_index)
{	
	FrameStore* fs = dpb.fs_on_view[view_index][pos];
	FrameStore* tmp;
	unsigned i;	

	DEBUG_SHOW_SW_INFO ("remove frame with frame_num #%d\n", fs->frame_num);
	switch (fs->is_used)
	{
	case (TOP_FIELD|BOTTOM_FIELD):
		//release_storable_picture ARGS2(fs->frame,fs->is_output?0:1);
		release_storable_picture ARGS2(fs->frame,1);
		release_storable_picture ARGS2(fs->top_field,0);
		release_storable_picture ARGS2(fs->bottom_field,0);
		fs->frame=NULL;
		fs->top_field=NULL;
		fs->bottom_field=NULL;
		break;
	case BOTTOM_FIELD:
		release_storable_picture ARGS2(fs->bottom_field,0);
		fs->bottom_field=NULL;
		break;
	case TOP_FIELD:
		release_storable_picture ARGS2(fs->top_field,0);
		fs->top_field=NULL;
		break;
	case 0:
		break;
	default:
		DEBUG_SHOW_ERROR_INFO("[ERROR]invalid frame store type",500);
	}
	fs->is_used = 0;
	fs->is_long_term = 0;
	fs->is_reference = 0;
	fs->is_orig_reference = 0;

	// move empty framestore to end of buffer
	tmp = dpb.fs_on_view[view_index][pos];

	for (i=pos; i<dpb.used_size_on_view[view_index]-1;i++)
	{
		dpb.fs_on_view[view_index][i] = dpb.fs_on_view[view_index][i+1];
	}
	dpb.fs_on_view[view_index][dpb.used_size_on_view[view_index]-1] = tmp;
	dpb.used_size_on_view[view_index]--;
}

/*!
************************************************************************
* \brief
*    find smallest POC in the DPB.
************************************************************************
*/
CREL_RETURN get_smallest_poc PARGS3(int *poc,int * pos, unsigned int view_index)
{
	unsigned i;	

	if (dpb.used_size_on_view[view_index]<1)
	{
		DEBUG_SHOW_ERROR_INFO("[ERROR]Cannot determine smallest POC, DPB empty.",150);	
		return CREL_WARNING_H264_STREAMERROR_LEVEL_1|CREL_WARNING_H264_ERROR_DPB;

	}

	*pos=-1;
	*poc = INT_MAX;
	for (i=0; i<dpb.used_size_on_view[view_index]; i++)
	{
		if ((*poc>dpb.fs_on_view[view_index][i]->poc)&&(!dpb.fs_on_view[view_index][i]->is_output))
		{
			*poc = dpb.fs_on_view[view_index][i]->poc;
			*pos=i;
		}
	}

	return CREL_OK;
}

/*!
************************************************************************
* \brief
*    Remove a picture from DPB which is no longer needed.
************************************************************************
*/
static int remove_unused_frame_from_dpb PARGS1(unsigned int view_index)
{
	unsigned i;

	// check for frames that were already output and no longer used for reference
	for (i=0; i<dpb.used_size_on_view[view_index]; i++)
	{
		if (dpb.fs_on_view[view_index][i]->is_output && (!is_used_for_reference(dpb.fs_on_view[view_index][i])))
		{
			remove_frame_from_dpb ARGS2(i, view_index);
			return 1;
		}
	}
	return 0;
}

/*!
************************************************************************
* \brief
*    Output one picture stored in the DPB.
************************************************************************
*/
CREL_RETURN output_one_frame_from_dpb PARGS1(unsigned int view_index)
{
	int poc, pos; 
	CREL_RETURN ret;

	// find smallest POC
	ret = get_smallest_poc ARGS3(&poc, &pos, view_index);
	if (FAILED(ret)) {
		return ret;
	}

	if(pos==-1)
	{
		//DEBUG_SHOW_ERROR_INFO("[ERROR]no frames for output available", 150);
		return CREL_WARNING_H264_STREAMERROR_LEVEL_1|CREL_WARNING_H264_ERROR_DPB;
	}

	// call the output function
	DEBUG_SHOW_SW_INFO("output frame with frame_num #%d, poc %d (dpb. dpb.size=%d, dpb.used_size=%d)\n", dpb.fs_on_view[view_index][pos]->frame_num, dpb.fs_on_view[view_index][pos]->is_used==3 ? dpb.fs_on_view[view_index][pos]->frame->poc:(-777), dpb.size_on_view[view_index], dpb.used_size_on_view[view_index]);

	write_stored_frame ARGS2(dpb.fs_on_view[view_index][pos], p_out);

	if ((dpb.last_output_poc >= poc) && (stream_global->num_views == 1))
	{
		//DEBUG_SHOW_ERROR_INFO ("[ERROR]output POC must be in ascending order", 150);
		return CREL_WARNING_H264_STREAMERROR_LEVEL_1|CREL_WARNING_H264_ERROR_DPB;
	} 
	dpb.last_output_poc = poc;
	// free frame store and move empty store to end of buffer
	if (!is_used_for_reference(dpb.fs_on_view[view_index][pos]))
	{
		remove_frame_from_dpb ARGS2(pos, view_index);
	}

	return CREL_OK;
}



/*!
************************************************************************
* \brief
*    All stored picture are output. Should be called to empty the buffer
************************************************************************
*/
CREL_RETURN flush_dpb PARGS1(unsigned int view_index)
{
	unsigned i;
	CREL_RETURN ret;	

	if (dpb.init_done == NULL || view_index >= stream_global->dpb_pre_alloc_views || dpb.init_done[view_index] == 0) {
		return CREL_OK;
	}

	//diagnostics
	DEBUG_SHOW_SW_INFO("Flush remaining frames from dpb. dpb.size=%d, dpb.used_size=%d\n",dpb.size_on_view[view_index],dpb.used_size_on_view[view_index]);

	// mark all frames unused
	for (i=0; i<dpb.used_size_on_view[view_index]; i++)
	{
		unmark_for_reference (dpb.fs_on_view[view_index][i]);
	}

	while (remove_unused_frame_from_dpb ARGS1(view_index)) ;

	// output frames in POC order
	while (dpb.used_size_on_view[view_index])
	{
		ret = output_one_frame_from_dpb ARGS1(view_index);
		if (FAILED(ret)) {
			return ret;
		}
		if (g_bNormalSpeed || g_bDisplayed==FALSE)
			g_framemgr->m_nFlushDPBsize++;
	}

	dpb.last_output_poc = INT_MIN;

	//Reset these values here to avoid crash at CheckSkipAndPTSProcess()!
	g_uSkipBFrameCounter[view_index][0] = 0;
	g_uSkipBFrameCounter[view_index][1] = 0;

	return CREL_OK;
}

//#define RSD(x) ((x&2)?(x|1):(x&(~1)))
//#define RSD(x) ((((((int)(x&2))-1)>>31) & (x&(~1)) ) | ((((((int)(x&2))-1)>>31)^-1L) & (x|1)))


void gen_field_ref_ids PARGS1(StorablePicture *p)
{
	int i,j, dummylist0;
	int mb_loop;
	Macroblock_s *mb = p->mb_data;  
	//! Generate Frame parameters from field information.
	for(mb_loop=0;mb_loop<p->PicSizeInMbs;mb_loop++)
	{
		mb->mb_field = 1;
		for (j=LIST_0 ; j!=LIST_1 ; j=LIST_1)
		{
			for (i=0 ; i<4 ; i++)
			{
				dummylist0= mb->pred_info.ref_idx[j][i];
				//! association with id already known for fields.
				mb->pred_info.ref_pic_id[j][i]= (dummylist0>=0)? listX[j][dummylist0]->unique_id : -1;

#ifdef _COLLECT_PIC_
				if (dummylist0>=0 && storable_picture_map[mb->pred_info.ref_pic_id[j][i]]->used_for_first_field_reference == 0)			    									
					storable_picture_map[mb->pred_info.ref_pic_id[j][i]]->used_for_first_field_reference = p->poc;		 
#endif
			}
		}
		mb++;
	}
}

// YKChen-Ulead, 5/10/05: minor optimization
/*!
************************************************************************
* \brief
*    Extract top field from a frame
************************************************************************
*/

CREL_RETURN dpb_split_field_common PARGS1(FrameStore *fs)
{
#ifdef DO_REF_PIC_NUM
	int i, j;
#endif

	fs->poc = fs->frame->poc;

	if (!fs->frame->frame_mbs_only_flag) 	// ER protection for frame gap 
	{   // direct_8x8_inference_flag=1
		fs->top_field    = get_storable_picture ARGS7(TOP_FIELD,    fs->frame->size_x, fs->frame->size_y, fs->frame->size_x_cr, fs->frame->size_y_cr, 0, fs->frame->non_existing);
		if (fs->top_field == NULL) {
			return CREL_WARNING_H264_STREAMERROR_LEVEL_1|CREL_WARNING_H264_ERROR_DPB;			
		}
		fs->bottom_field = get_storable_picture ARGS7(BOTTOM_FIELD, fs->frame->size_x, fs->frame->size_y, fs->frame->size_x_cr, fs->frame->size_y_cr, 0, fs->frame->non_existing);
		if (fs->bottom_field == NULL) {
			return CREL_WARNING_H264_STREAMERROR_LEVEL_1|CREL_WARNING_H264_ERROR_DPB;			
		}
#ifdef _HW_ACCEL_
		if(g_DXVAVer==IviNotDxva || IMGPAR Hybrid_Decoding)
#else
		if(1)
#endif
		{
			if (/*fs->frame->imgY*/fs->frame->FrameBuffer && fs->frame->non_existing == 0) {
				fs->top_field->imgY        = fs->frame->imgY;
				fs->bottom_field->imgY     = fs->frame->imgY + fs->frame->Y_stride;
			} else if (fs->frame->non_existing == 0){
				fs->top_field->imgY        =  NULL;
				fs->bottom_field->imgY     =  NULL;
			}

			if (/*fs->frame->imgUV*/fs->frame->FrameBuffer && fs->frame->non_existing == 0) {
				fs->top_field->imgUV	   = fs->frame->imgUV;			
				fs->bottom_field->imgUV    = fs->frame->imgUV + fs->frame->UV_stride;
			} else if (fs->frame->non_existing == 0){
				fs->top_field->imgUV	   = NULL;			
				fs->bottom_field->imgUV    = NULL;
			}
		}
		fs->top_field->poc = fs->frame->top_poc;
		fs->bottom_field->poc =  fs->frame->bottom_poc;

		fs->top_field->frame_poc =  fs->frame->frame_poc;

		fs->top_field->bottom_poc =fs->bottom_field->bottom_poc =  fs->frame->bottom_poc;
		fs->top_field->top_poc =fs->bottom_field->top_poc =  fs->frame->top_poc;
		fs->bottom_field->frame_poc =  fs->frame->frame_poc;

		fs->top_field->used_for_reference = fs->bottom_field->used_for_reference 
			= fs->frame->used_for_reference;
		fs->top_field->is_long_term = fs->bottom_field->is_long_term 
			= fs->frame->is_long_term;
		fs->long_term_frame_idx = fs->top_field->long_term_frame_idx 
			= fs->bottom_field->long_term_frame_idx 
			= fs->frame->long_term_frame_idx;

		fs->top_field->coded_frame = fs->bottom_field->coded_frame = 1;
		fs->top_field->MbaffFrameFlag = fs->bottom_field->MbaffFrameFlag
			= fs->frame->MbaffFrameFlag;

		fs->frame->top_field    = fs->top_field;
		fs->frame->bottom_field = fs->bottom_field;

		fs->top_field->bottom_field = fs->bottom_field;
		fs->top_field->frame        = fs->frame;
		fs->bottom_field->top_field = fs->top_field;
		fs->bottom_field->frame     = fs->frame;

		fs->top_field->chroma_format_idc = fs->bottom_field->chroma_format_idc = fs->frame->chroma_format_idc;

		fs->top_field->view_id = fs->bottom_field->view_id = fs->frame->view_id;
		fs->top_field->view_index = fs->bottom_field->view_index = fs->frame->view_index;

#ifdef DO_REF_PIC_NUM
		//store reference picture index
		for (j=0; j<=fs->frame->max_slice_id; j++)
		{
			for (i=0;i<listXsize[LIST_1];i++)
			{
				tmp0=i<<1;
				tmp1=tmp0+1;

				fs->top_field->ref_pic_num[j][LIST_1][tmp0]     =fs->frame->ref_pic_num[j][3][tmp0];
				fs->top_field->ref_pic_num[j][LIST_1][tmp1] =fs->frame->ref_pic_num[j][3][tmp1];
				fs->bottom_field->ref_pic_num[j][LIST_1][tmp0]  =fs->frame->ref_pic_num[j][5][tmp0];
				fs->bottom_field->ref_pic_num[j][LIST_1][tmp1]=fs->frame->ref_pic_num[j][5][tmp1] ;
			}

			for (i=0;i<listXsize[LIST_0];i++)
			{
				tmp0=i<<1;
				tmp1=tmp0+1;

				fs->top_field->ref_pic_num[j][LIST_0][tmp0]     =fs->frame->ref_pic_num[j][2][tmp0];
				fs->top_field->ref_pic_num[j][LIST_0][tmp1] =fs->frame->ref_pic_num[j][2][tmp1];
				fs->bottom_field->ref_pic_num[j][LIST_0][tmp0]  =fs->frame->ref_pic_num[j][4][tmp0];
				fs->bottom_field->ref_pic_num[j][LIST_0][tmp1]=fs->frame->ref_pic_num[j][4][tmp1] ;
			}
		}
#endif
	}
	else
	{   // direct_8x8_inference_flag can be 0 or 1
		fs->top_field=NULL;
		fs->bottom_field=NULL;
		fs->frame->top_field=NULL;
		fs->frame->bottom_field=NULL;
	}

	return CREL_OK;
}

//faster C version 5/13
CREL_RETURN dpb_split_field_temporal PARGS1(FrameStore *fs)
{
	int i, j;
	int dummylist0,dummylist1;
#ifdef _XMM_COPY_
	__m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7;
#else
	int ii;
#endif

	Macroblock_s *mb, *mb_top, *mb_bot;
	int mb_loop;
	int mb_loop_i,mb_loop_j;
	CREL_RETURN ret;

	ret = dpb_split_field_common ARGS1(fs);
	if ( FAILED(ret)  || fs->frame->non_existing ) {
		return ret;
	}

	if (fs->frame->MbaffFrameFlag){//Mbaff==1
		mb = fs->frame->mb_data;
		for(mb_loop=0;mb_loop<fs->frame->PicSizeInMbs;mb_loop++)
		{
			if (mb->mb_field)
			{
				for (i=0; i<4 ; i++)
				{
					dummylist0 = mb->pred_info.ref_idx[LIST_0][i];
					dummylist1 = mb->pred_info.ref_idx[LIST_1][i];
					//! association with id already known for fields.

					if((dummylist0>=0)&&(storable_picture_map[mb->pred_info.ref_pic_id[LIST_0][i]]->frame)){
						mb->pred_info.ref_pic_id[LIST_0][i]= 
							storable_picture_map[mb->pred_info.ref_pic_id[LIST_0][i]]->frame->unique_id;
					}else{
						mb->pred_info.ref_pic_id[LIST_0][i]=-1;
					}

					if((dummylist1>=0)&&(storable_picture_map[mb->pred_info.ref_pic_id[LIST_1][i]]->frame)){
						mb->pred_info.ref_pic_id[LIST_1][i]= 
							storable_picture_map[mb->pred_info.ref_pic_id[LIST_1][i]]->frame->unique_id;
					}else{
						mb->pred_info.ref_pic_id[LIST_1][i]=-1;
					}//! need to make association with frames		  
				}
			}
			mb++;
		}


		//! Generate field MVs from Frame MVs
		if (!fs->frame->frame_mbs_only_flag)	// ER protection for frame gap 
		{   // direct_8x8_inference_flag=1
			mb	 = &fs->frame->mb_data[0];
			mb_top = &fs->top_field->mb_data[0];
			mb_bot = &fs->bottom_field->mb_data[0];

			for(mb_loop=0;mb_loop<fs->frame->PicSizeInMbs/2;mb_loop++)
			{
				if(mb->mb_field)
				{
					mb_bot->mb_field = mb_top->mb_field = 1;
#ifdef _XMM_COPY_
					xmm0 = _mm_load_si128((__m128i *) &mb->pred_info.mv[LIST_0][ 0]);
					xmm1 = _mm_load_si128((__m128i *) &mb->pred_info.mv[LIST_0][ 4]);
					xmm2 = _mm_load_si128((__m128i *) &mb->pred_info.mv[LIST_0][ 8]);
					xmm3 = _mm_load_si128((__m128i *) &mb->pred_info.mv[LIST_0][12]);
					xmm4 = _mm_load_si128((__m128i *) &mb->pred_info.mv[LIST_1][ 0]);
					xmm5 = _mm_load_si128((__m128i *) &mb->pred_info.mv[LIST_1][ 4]);
					xmm6 = _mm_load_si128((__m128i *) &mb->pred_info.mv[LIST_1][ 8]);
					xmm7 = _mm_load_si128((__m128i *) &mb->pred_info.mv[LIST_1][12]);
					_mm_store_si128((__m128i *) &mb_top->pred_info.mv[LIST_0][ 0], xmm0);
					_mm_store_si128((__m128i *) &mb_top->pred_info.mv[LIST_0][ 4], xmm1);
					_mm_store_si128((__m128i *) &mb_top->pred_info.mv[LIST_0][ 8], xmm2);
					_mm_store_si128((__m128i *) &mb_top->pred_info.mv[LIST_0][12], xmm3);
					_mm_store_si128((__m128i *) &mb_top->pred_info.mv[LIST_1][ 0], xmm4);
					_mm_store_si128((__m128i *) &mb_top->pred_info.mv[LIST_1][ 4], xmm5);
					_mm_store_si128((__m128i *) &mb_top->pred_info.mv[LIST_1][ 8], xmm6);
					_mm_store_si128((__m128i *) &mb_top->pred_info.mv[LIST_1][12], xmm7);

					xmm0 = _mm_load_si128((__m128i *) &(mb+1)->pred_info.mv[LIST_0][ 0]);
					xmm1 = _mm_load_si128((__m128i *) &(mb+1)->pred_info.mv[LIST_0][ 4]);
					xmm2 = _mm_load_si128((__m128i *) &(mb+1)->pred_info.mv[LIST_0][ 8]);
					xmm3 = _mm_load_si128((__m128i *) &(mb+1)->pred_info.mv[LIST_0][12]);
					xmm4 = _mm_load_si128((__m128i *) &(mb+1)->pred_info.mv[LIST_1][ 0]);
					xmm5 = _mm_load_si128((__m128i *) &(mb+1)->pred_info.mv[LIST_1][ 4]);
					xmm6 = _mm_load_si128((__m128i *) &(mb+1)->pred_info.mv[LIST_1][ 8]);
					xmm7 = _mm_load_si128((__m128i *) &(mb+1)->pred_info.mv[LIST_1][12]);
					_mm_store_si128((__m128i *) &mb_bot->pred_info.mv[LIST_0][ 0], xmm0);
					_mm_store_si128((__m128i *) &mb_bot->pred_info.mv[LIST_0][ 4], xmm1);
					_mm_store_si128((__m128i *) &mb_bot->pred_info.mv[LIST_0][ 8], xmm2);
					_mm_store_si128((__m128i *) &mb_bot->pred_info.mv[LIST_0][12], xmm3);
					_mm_store_si128((__m128i *) &mb_bot->pred_info.mv[LIST_1][ 0], xmm4);
					_mm_store_si128((__m128i *) &mb_bot->pred_info.mv[LIST_1][ 4], xmm5);
					_mm_store_si128((__m128i *) &mb_bot->pred_info.mv[LIST_1][ 8], xmm6);
					_mm_store_si128((__m128i *) &mb_bot->pred_info.mv[LIST_1][12], xmm7);
#else
					memcpy(mb_top->pred_info.mv,mb->pred_info.mv,sizeof(mb->pred_info.mv));
					memcpy(mb_bot->pred_info.mv,(mb+1)->pred_info.mv,sizeof((mb+1)->pred_info.mv));
#endif
					for(j=0;j<4;j++)
					{
						dummylist0 = mb_top->pred_info.ref_idx[LIST_0][j] = mb->pred_info.ref_idx[LIST_0][j];
						dummylist1 = mb_top->pred_info.ref_idx[LIST_1][j] = mb->pred_info.ref_idx[LIST_1][j];
						mb_top->pred_info.ref_pic_id[LIST_0][j] = dummylist0>=0 ? listX[2][dummylist0]->unique_id : -1;
						mb_top->pred_info.ref_pic_id[LIST_1][j] = dummylist1>=0 ? listX[3][dummylist1]->unique_id : -1;							  

						dummylist0 = mb_bot->pred_info.ref_idx[LIST_0][j] = (mb+1)->pred_info.ref_idx[LIST_0][j];
						dummylist1 = mb_bot->pred_info.ref_idx[LIST_1][j] = (mb+1)->pred_info.ref_idx[LIST_1][j];
						mb_bot->pred_info.ref_pic_id[LIST_0][j] = dummylist0>=0 ? listX[4][dummylist0]->unique_id : -1;
						mb_bot->pred_info.ref_pic_id[LIST_1][j] = dummylist1>=0 ? listX[5][dummylist1]->unique_id : -1;
					}
				}
				else
				{
					mb_bot->mb_field = mb_top->mb_field = 0;
#ifdef _XMM_COPY_
					xmm0 = _mm_load_si128((__m128i *) &mb->pred_info.mv[LIST_0][0]);
					xmm0 = _mm_shuffle_epi32(xmm0, 0xF0); // RSD
					xmm2 = _mm_load_si128((__m128i *) &mb->pred_info.mv[LIST_1][0]);
					xmm2 = _mm_shuffle_epi32(xmm2, 0xF0); // RSD
					xmm1 = _mm_load_si128((__m128i *) &(mb+1)->pred_info.mv[LIST_0][8]);
					xmm1 = _mm_shuffle_epi32(xmm1, 0xF0); // RSD
					xmm3 = _mm_load_si128((__m128i *) &(mb+1)->pred_info.mv[LIST_1][8]);
					xmm3 = _mm_shuffle_epi32(xmm3, 0xF0); // RSD

					_mm_store_si128((__m128i *) &mb_top->pred_info.mv[LIST_0][ 0], xmm0);
					_mm_store_si128((__m128i *) &mb_top->pred_info.mv[LIST_0][ 4], xmm0);
					_mm_store_si128((__m128i *) &mb_top->pred_info.mv[LIST_0][ 8], xmm1);
					_mm_store_si128((__m128i *) &mb_top->pred_info.mv[LIST_0][12], xmm1);
					_mm_store_si128((__m128i *) &mb_top->pred_info.mv[LIST_1][ 0], xmm2);
					_mm_store_si128((__m128i *) &mb_top->pred_info.mv[LIST_1][ 4], xmm2);
					_mm_store_si128((__m128i *) &mb_top->pred_info.mv[LIST_1][ 8], xmm3);
					_mm_store_si128((__m128i *) &mb_top->pred_info.mv[LIST_1][12], xmm3);

					_mm_store_si128((__m128i *) &mb_bot->pred_info.mv[LIST_0][ 0], xmm0);
					_mm_store_si128((__m128i *) &mb_bot->pred_info.mv[LIST_0][ 4], xmm0);
					_mm_store_si128((__m128i *) &mb_bot->pred_info.mv[LIST_0][ 8], xmm1);
					_mm_store_si128((__m128i *) &mb_bot->pred_info.mv[LIST_0][12], xmm1);
					_mm_store_si128((__m128i *) &mb_bot->pred_info.mv[LIST_1][ 0], xmm2);
					_mm_store_si128((__m128i *) &mb_bot->pred_info.mv[LIST_1][ 4], xmm2);
					_mm_store_si128((__m128i *) &mb_bot->pred_info.mv[LIST_1][ 8], xmm3);
					_mm_store_si128((__m128i *) &mb_bot->pred_info.mv[LIST_1][12], xmm3);
#else
					for(j=0;j<2;j++)
					{
						for(i=0;i<4;i++)
						{
							ii = RSD[i];
							int a = (j<<2)+i;

							mb_top->pred_info.mv[LIST_0][a].mv_comb =
								mb_bot->pred_info.mv[LIST_0][a].mv_comb = mb->pred_info.mv[LIST_0][ii].mv_comb;
							mb_top->pred_info.mv[LIST_1][a].mv_comb =
								mb_bot->pred_info.mv[LIST_1][a].mv_comb = mb->pred_info.mv[LIST_1][ii].mv_comb;
						}
					}
					for(j=2;j<4;j++)
					{
						for(i=0;i<4;i++)
						{
							ii = RSD[i];
							int a = (j<<2)+i;

							mb_top->pred_info.mv[LIST_0][a].mv_comb =
								mb_bot->pred_info.mv[LIST_0][a].mv_comb = (mb+1)->pred_info.mv[LIST_0][8+ii].mv_comb; // maybe 12+ ?
							mb_top->pred_info.mv[LIST_1][a].mv_comb =
								mb_bot->pred_info.mv[LIST_1][a].mv_comb = (mb+1)->pred_info.mv[LIST_1][8+ii].mv_comb; // maybe 12+ ?
						}
					}
#endif
					for(j=0;j<2;j++)
					{
						dummylist0 = 
							mb_bot->pred_info.ref_idx[LIST_0][j] = 
							mb_top->pred_info.ref_idx[LIST_0][j] = mb->pred_info.ref_idx[LIST_0][j];
						if(dummylist0==-1)
						{
							mb_top->pred_info.ref_pic_id[LIST_0][j] = -1;
							mb_bot->pred_info.ref_pic_id[LIST_0][j] = -1;
						}
						else
						{
							if (storable_picture_map[mb->pred_info.ref_pic_id[LIST_0][j]]->top_field) {
								mb_top->pred_info.ref_pic_id[LIST_0][j] = 
									storable_picture_map[mb->pred_info.ref_pic_id[LIST_0][j]]->top_field->unique_id;
							} else {
								mb_top->pred_info.ref_pic_id[LIST_0][j] = -1;
							}

							if (storable_picture_map[mb->pred_info.ref_pic_id[LIST_0][j]]->bottom_field) {
								mb_bot->pred_info.ref_pic_id[LIST_0][j] = 
									storable_picture_map[mb->pred_info.ref_pic_id[LIST_0][j]]->bottom_field->unique_id;
							} else {
								mb_bot->pred_info.ref_pic_id[LIST_0][j] = -1;
							}
						}

						dummylist0 = 
							mb_bot->pred_info.ref_idx[LIST_1][j] = 
							mb_top->pred_info.ref_idx[LIST_1][j] = mb->pred_info.ref_idx[LIST_1][j];
						if(dummylist0==-1)
						{
							mb_top->pred_info.ref_pic_id[LIST_1][j] = -1;
							mb_bot->pred_info.ref_pic_id[LIST_1][j] = -1;
						}
						else
						{
							if (storable_picture_map[mb->pred_info.ref_pic_id[LIST_1][j]]->top_field) {
								mb_top->pred_info.ref_pic_id[LIST_1][j] = 
									storable_picture_map[mb->pred_info.ref_pic_id[LIST_1][j]]->top_field->unique_id;
							} else {
								mb_top->pred_info.ref_pic_id[LIST_1][j] = -1;
							}

							if (storable_picture_map[mb->pred_info.ref_pic_id[LIST_1][j]]->bottom_field) {
								mb_bot->pred_info.ref_pic_id[LIST_1][j] = 
									storable_picture_map[mb->pred_info.ref_pic_id[LIST_1][j]]->bottom_field->unique_id;
							} else {
								mb_bot->pred_info.ref_pic_id[LIST_1][j] = -1;
							}
						}
					}

					for(j=2;j<4;j++)
					{
						dummylist0 = 
							mb_bot->pred_info.ref_idx[LIST_0][j] = 
							mb_top->pred_info.ref_idx[LIST_0][j] = (mb+1)->pred_info.ref_idx[LIST_0][j];
						if(dummylist0==-1)
						{
							mb_top->pred_info.ref_pic_id[LIST_0][j] = -1;
							mb_bot->pred_info.ref_pic_id[LIST_0][j] = -1;
						}
						else
						{
							if (storable_picture_map[(mb+1)->pred_info.ref_pic_id[LIST_0][j]]->top_field) {
								mb_top->pred_info.ref_pic_id[LIST_0][j] = 
									storable_picture_map[(mb+1)->pred_info.ref_pic_id[LIST_0][j]]->top_field->unique_id;
							} else {
								mb_top->pred_info.ref_pic_id[LIST_0][j] = -1;
							}

							if (storable_picture_map[(mb+1)->pred_info.ref_pic_id[LIST_0][j]]->bottom_field) {
								mb_bot->pred_info.ref_pic_id[LIST_0][j] = 
									storable_picture_map[(mb+1)->pred_info.ref_pic_id[LIST_0][j]]->bottom_field->unique_id;
							} else {
								mb_bot->pred_info.ref_pic_id[LIST_0][j] = -1;
							}
						}

						dummylist0 = 
							mb_bot->pred_info.ref_idx[LIST_1][j] = 
							mb_top->pred_info.ref_idx[LIST_1][j] = (mb+1)->pred_info.ref_idx[LIST_1][j];
						if(dummylist0==-1)
						{
							mb_top->pred_info.ref_pic_id[LIST_1][j] = -1;
							mb_bot->pred_info.ref_pic_id[LIST_1][j] = -1;
						}
						else
						{
							if (storable_picture_map[(mb+1)->pred_info.ref_pic_id[LIST_1][j]]->top_field) {
								mb_top->pred_info.ref_pic_id[LIST_1][j] = 
									storable_picture_map[(mb+1)->pred_info.ref_pic_id[LIST_1][j]]->top_field->unique_id;
							} else {
								mb_top->pred_info.ref_pic_id[LIST_1][j] = -1;
							}

							if (storable_picture_map[(mb+1)->pred_info.ref_pic_id[LIST_1][j]]->bottom_field) {
								mb_bot->pred_info.ref_pic_id[LIST_1][j] = 
									storable_picture_map[(mb+1)->pred_info.ref_pic_id[LIST_1][j]]->bottom_field->unique_id;
							} else {
								mb_bot->pred_info.ref_pic_id[LIST_1][j] = -1;
							}
						}
					}
				}
				mb+=2;
				mb_top++;
				mb_bot++;
			}
		}
		else
		{   // direct_8x8_inference_flag can be 0 or 1
			mb = &fs->frame->mb_data[0];
			for(mb_loop=0;mb_loop<fs->frame->PicSizeInMbs;mb_loop++)
			{
				mb->mb_field = 0;
				mb++;
			}
		}
	}else{//Mbaff==0
		mb = &fs->frame->mb_data[0];

		//! Generate field MVs from Frame MVs
		if (!fs->frame->frame_mbs_only_flag)	// ER protection for frame gap 
		{   // direct_8x8_inference_flag=1
			int mb_width = fs->frame->PicWidthInMbs;

			mb	 = &fs->frame->mb_data[0];
			mb_top = &fs->top_field->mb_data[0];
			mb_bot = &fs->bottom_field->mb_data[0];

			for(mb_loop_i=0;mb_loop_i<fs->frame->PicSizeInMbs/fs->frame->PicWidthInMbs;mb_loop_i+=2)
			{
				for(mb_loop_j=0;mb_loop_j<fs->frame->PicWidthInMbs;mb_loop_j++)
				{
					mb->mb_field = (mb+mb_width)->mb_field = mb_bot->mb_field = mb_top->mb_field = 0;
#ifdef _XMM_COPY_
					xmm0 = _mm_load_si128((__m128i *) &mb->pred_info.mv[LIST_0][0]);
					xmm0 = _mm_shuffle_epi32(xmm0, 0xF0); // RSD
					xmm2 = _mm_load_si128((__m128i *) &mb->pred_info.mv[LIST_1][0]);
					xmm2 = _mm_shuffle_epi32(xmm2, 0xF0); // RSD
					xmm1 = _mm_load_si128((__m128i *) &(mb+mb_width)->pred_info.mv[LIST_0][8]);
					xmm1 = _mm_shuffle_epi32(xmm1, 0xF0); // RSD
					xmm3 = _mm_load_si128((__m128i *) &(mb+mb_width)->pred_info.mv[LIST_1][8]);
					xmm3 = _mm_shuffle_epi32(xmm3, 0xF0); // RSD

					_mm_store_si128((__m128i *) &mb_top->pred_info.mv[LIST_0][ 0], xmm0);
					_mm_store_si128((__m128i *) &mb_top->pred_info.mv[LIST_0][ 4], xmm0);
					_mm_store_si128((__m128i *) &mb_top->pred_info.mv[LIST_0][ 8], xmm1);
					_mm_store_si128((__m128i *) &mb_top->pred_info.mv[LIST_0][12], xmm1);
					_mm_store_si128((__m128i *) &mb_top->pred_info.mv[LIST_1][ 0], xmm2);
					_mm_store_si128((__m128i *) &mb_top->pred_info.mv[LIST_1][ 4], xmm2);
					_mm_store_si128((__m128i *) &mb_top->pred_info.mv[LIST_1][ 8], xmm3);
					_mm_store_si128((__m128i *) &mb_top->pred_info.mv[LIST_1][12], xmm3);

					_mm_store_si128((__m128i *) &mb_bot->pred_info.mv[LIST_0][ 0], xmm0);
					_mm_store_si128((__m128i *) &mb_bot->pred_info.mv[LIST_0][ 4], xmm0);
					_mm_store_si128((__m128i *) &mb_bot->pred_info.mv[LIST_0][ 8], xmm1);
					_mm_store_si128((__m128i *) &mb_bot->pred_info.mv[LIST_0][12], xmm1);
					_mm_store_si128((__m128i *) &mb_bot->pred_info.mv[LIST_1][ 0], xmm2);
					_mm_store_si128((__m128i *) &mb_bot->pred_info.mv[LIST_1][ 4], xmm2);
					_mm_store_si128((__m128i *) &mb_bot->pred_info.mv[LIST_1][ 8], xmm3);
					_mm_store_si128((__m128i *) &mb_bot->pred_info.mv[LIST_1][12], xmm3);
#else
					for(j=0;j<2;j++)
					{
						for(i=0;i<4;i++)
						{
							ii = RSD[i];
							int a = (j<<2)+i;

							mb_top->pred_info.mv[LIST_0][a].mv_comb =
								mb_bot->pred_info.mv[LIST_0][a].mv_comb = mb->pred_info.mv[LIST_0][ii].mv_comb;
							mb_top->pred_info.mv[LIST_1][a].mv_comb =
								mb_bot->pred_info.mv[LIST_1][a].mv_comb = mb->pred_info.mv[LIST_1][ii].mv_comb;

						}
					}
					for(j=2;j<4;j++)
					{
						for(i=0;i<4;i++)
						{
							ii = RSD[i];
							int a = (j<<2)+i;

							mb_top->pred_info.mv[LIST_0][a].mv_comb =
								mb_bot->pred_info.mv[LIST_0][a].mv_comb = (mb+mb_width)->pred_info.mv[LIST_0][8+ii].mv_comb;
							mb_top->pred_info.mv[LIST_1][a].mv_comb =
								mb_bot->pred_info.mv[LIST_1][a].mv_comb = (mb+mb_width)->pred_info.mv[LIST_1][8+ii].mv_comb;

						}
					}
#endif
					for(j=0;j<2;j++)
					{
						dummylist0 = 
							mb_bot->pred_info.ref_idx[LIST_0][j] = 
							mb_top->pred_info.ref_idx[LIST_0][j] = mb->pred_info.ref_idx[LIST_0][j];
						if(dummylist0==-1)
						{
							mb_top->pred_info.ref_pic_id[LIST_0][j] = -1;
							mb_bot->pred_info.ref_pic_id[LIST_0][j] = -1;
						}
						else
						{
							if (storable_picture_map[mb->pred_info.ref_pic_id[LIST_0][j]]->top_field) {
								mb_top->pred_info.ref_pic_id[LIST_0][j] = 
									storable_picture_map[mb->pred_info.ref_pic_id[LIST_0][j]]->top_field->unique_id;
							} else {
								mb_top->pred_info.ref_pic_id[LIST_0][j] = -1;
							}

							if (storable_picture_map[mb->pred_info.ref_pic_id[LIST_0][j]]->bottom_field) {
								mb_bot->pred_info.ref_pic_id[LIST_0][j] = 
									storable_picture_map[mb->pred_info.ref_pic_id[LIST_0][j]]->bottom_field->unique_id;
							} else {
								mb_bot->pred_info.ref_pic_id[LIST_0][j] = -1;
							}
						}

						dummylist0 = 
							mb_bot->pred_info.ref_idx[LIST_1][j] = 
							mb_top->pred_info.ref_idx[LIST_1][j] = mb->pred_info.ref_idx[LIST_1][j];
						if(dummylist0==-1)
						{
							mb_top->pred_info.ref_pic_id[LIST_1][j] = -1;
							mb_bot->pred_info.ref_pic_id[LIST_1][j] = -1;
						}
						else
						{
							if (storable_picture_map[mb->pred_info.ref_pic_id[LIST_1][j]]->top_field) {
								mb_top->pred_info.ref_pic_id[LIST_1][j] = 
									storable_picture_map[mb->pred_info.ref_pic_id[LIST_1][j]]->top_field->unique_id;
							} else {
								mb_top->pred_info.ref_pic_id[LIST_1][j] = -1;
							}

							if (storable_picture_map[mb->pred_info.ref_pic_id[LIST_1][j]]->bottom_field) {
								mb_bot->pred_info.ref_pic_id[LIST_1][j] = 
									storable_picture_map[mb->pred_info.ref_pic_id[LIST_1][j]]->bottom_field->unique_id;
							} else {
								mb_bot->pred_info.ref_pic_id[LIST_1][j] = -1;
							}
						}
					}

					for(j=2;j<4;j++)
					{
						dummylist0 = 
							mb_bot->pred_info.ref_idx[LIST_0][j] = 
							mb_top->pred_info.ref_idx[LIST_0][j] = (mb+mb_width)->pred_info.ref_idx[LIST_0][j];
						if(dummylist0==-1)
						{
							mb_top->pred_info.ref_pic_id[LIST_0][j] = -1;
							mb_bot->pred_info.ref_pic_id[LIST_0][j] = -1;
						}
						else
						{
							if (storable_picture_map[(mb+mb_width)->pred_info.ref_pic_id[LIST_0][j]]->top_field) {
								mb_top->pred_info.ref_pic_id[LIST_0][j] = 
									storable_picture_map[(mb+mb_width)->pred_info.ref_pic_id[LIST_0][j]]->top_field->unique_id;
							} else {
								mb_top->pred_info.ref_pic_id[LIST_0][j] = -1;

							}

							if (storable_picture_map[(mb+mb_width)->pred_info.ref_pic_id[LIST_0][j]]->bottom_field) {
								mb_bot->pred_info.ref_pic_id[LIST_0][j] = 
									storable_picture_map[(mb+mb_width)->pred_info.ref_pic_id[LIST_0][j]]->bottom_field->unique_id;
							} else {
								mb_bot->pred_info.ref_pic_id[LIST_0][j] = -1;
							}
						}

						dummylist0 = 
							mb_bot->pred_info.ref_idx[LIST_1][j] = 
							mb_top->pred_info.ref_idx[LIST_1][j] = (mb+mb_width)->pred_info.ref_idx[LIST_1][j];
						if(dummylist0==-1)
						{
							mb_top->pred_info.ref_pic_id[LIST_1][j] = -1;
							mb_bot->pred_info.ref_pic_id[LIST_1][j] = -1;
						}
						else
						{
							if (storable_picture_map[(mb+mb_width)->pred_info.ref_pic_id[LIST_1][j]]->top_field) {
								mb_top->pred_info.ref_pic_id[LIST_1][j] = 
									storable_picture_map[(mb+mb_width)->pred_info.ref_pic_id[LIST_1][j]]->top_field->unique_id;
							} else {
								mb_top->pred_info.ref_pic_id[LIST_1][j] = -1;
							}

							if (storable_picture_map[(mb+mb_width)->pred_info.ref_pic_id[LIST_1][j]]->bottom_field) {
								mb_bot->pred_info.ref_pic_id[LIST_1][j] = 
									storable_picture_map[(mb+mb_width)->pred_info.ref_pic_id[LIST_1][j]]->bottom_field->unique_id;
							} else {
								mb_bot->pred_info.ref_pic_id[LIST_1][j] = -1;
							}						
						}
					}
					mb++;
					mb_top++;
					mb_bot++;
				}
				mb += mb_width;
			}
		}
		else
		{   // direct_8x8_inference_flag can be 0 or 1
			mb = &fs->frame->mb_data[0];
			for(mb_loop=0;mb_loop<fs->frame->PicSizeInMbs;mb_loop++)
			{
				mb->mb_field = 0;
				mb++;
			}
		}
	}

	return CREL_OK;
}

CREL_RETURN dpb_split_field_spatial PARGS1(FrameStore *fs)
{
	int j;

	Macroblock_s *mb, *mb_top, *mb_bot;
	int mb_loop;
	int mb_loop_i,mb_loop_j;
#ifdef _XMM_COPY_
	__m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7;
	__m64 mm0;
#else
	int i, ii;
#endif
	CREL_RETURN ret;

	ret = dpb_split_field_common ARGS1(fs);
	if (FAILED(ret) ||fs->frame->non_existing) {
		return ret;
	}

	if (fs->frame->MbaffFrameFlag){//Mbaff==1

		//! Generate field MVs from Frame MVs
		if (!fs->frame->frame_mbs_only_flag)	// ER protection for frame gap 
		{
			mb	 = &fs->frame->mb_data[0];
			mb_top = &fs->top_field->mb_data[0];
			mb_bot = &fs->bottom_field->mb_data[0];

			for(mb_loop=0;mb_loop<fs->frame->PicSizeInMbs/2;mb_loop++)
			{
				if(mb->mb_field)
				{
					mb_bot->mb_field= mb_top->mb_field = 1;
#ifdef _XMM_COPY_
					xmm0 = _mm_load_si128((__m128i *) &mb->pred_info.mv[LIST_0][ 0]);
					xmm1 = _mm_load_si128((__m128i *) &mb->pred_info.mv[LIST_0][ 4]);
					xmm2 = _mm_load_si128((__m128i *) &mb->pred_info.mv[LIST_0][ 8]);
					xmm3 = _mm_load_si128((__m128i *) &mb->pred_info.mv[LIST_0][12]);
					xmm4 = _mm_load_si128((__m128i *) &mb->pred_info.mv[LIST_1][ 0]);
					xmm5 = _mm_load_si128((__m128i *) &mb->pred_info.mv[LIST_1][ 4]);
					xmm6 = _mm_load_si128((__m128i *) &mb->pred_info.mv[LIST_1][ 8]);
					xmm7 = _mm_load_si128((__m128i *) &mb->pred_info.mv[LIST_1][12]);
					mm0 = *(__m64 *) &mb->pred_info.ref_idx[0][0];

					_mm_store_si128((__m128i *) &mb_top->pred_info.mv[LIST_0][ 0], xmm0);
					_mm_store_si128((__m128i *) &mb_top->pred_info.mv[LIST_0][ 4], xmm1);
					_mm_store_si128((__m128i *) &mb_top->pred_info.mv[LIST_0][ 8], xmm2);
					_mm_store_si128((__m128i *) &mb_top->pred_info.mv[LIST_0][12], xmm3);
					_mm_store_si128((__m128i *) &mb_top->pred_info.mv[LIST_1][ 0], xmm4);
					_mm_store_si128((__m128i *) &mb_top->pred_info.mv[LIST_1][ 4], xmm5);
					_mm_store_si128((__m128i *) &mb_top->pred_info.mv[LIST_1][ 8], xmm6);
					_mm_store_si128((__m128i *) &mb_top->pred_info.mv[LIST_1][12], xmm7);
					*(__m64 *) &mb_top->pred_info.ref_idx[0][0] = mm0;

					xmm0 = _mm_load_si128((__m128i *) &(mb+1)->pred_info.mv[LIST_0][ 0]);
					xmm1 = _mm_load_si128((__m128i *) &(mb+1)->pred_info.mv[LIST_0][ 4]);
					xmm2 = _mm_load_si128((__m128i *) &(mb+1)->pred_info.mv[LIST_0][ 8]);
					xmm3 = _mm_load_si128((__m128i *) &(mb+1)->pred_info.mv[LIST_0][12]);
					xmm4 = _mm_load_si128((__m128i *) &(mb+1)->pred_info.mv[LIST_1][ 0]);
					xmm5 = _mm_load_si128((__m128i *) &(mb+1)->pred_info.mv[LIST_1][ 4]);
					xmm6 = _mm_load_si128((__m128i *) &(mb+1)->pred_info.mv[LIST_1][ 8]);
					xmm7 = _mm_load_si128((__m128i *) &(mb+1)->pred_info.mv[LIST_1][12]);
					mm0 = *(__m64 *) &(mb+1)->pred_info.ref_idx[0][0];

					_mm_store_si128((__m128i *) &mb_bot->pred_info.mv[LIST_0][ 0], xmm0);
					_mm_store_si128((__m128i *) &mb_bot->pred_info.mv[LIST_0][ 4], xmm1);
					_mm_store_si128((__m128i *) &mb_bot->pred_info.mv[LIST_0][ 8], xmm2);
					_mm_store_si128((__m128i *) &mb_bot->pred_info.mv[LIST_0][12], xmm3);
					_mm_store_si128((__m128i *) &mb_bot->pred_info.mv[LIST_1][ 0], xmm4);
					_mm_store_si128((__m128i *) &mb_bot->pred_info.mv[LIST_1][ 4], xmm5);
					_mm_store_si128((__m128i *) &mb_bot->pred_info.mv[LIST_1][ 8], xmm6);
					_mm_store_si128((__m128i *) &mb_bot->pred_info.mv[LIST_1][12], xmm7);
					*(__m64 *) &mb_bot->pred_info.ref_idx[0][0] = mm0;
#else
					memcpy(&mb_top->pred_info.mv[0][0],&mb->pred_info.mv[0][0],
						sizeof(mb->pred_info.mv)+sizeof(mb->pred_info.ref_idx));
					memcpy(&mb_bot->pred_info.mv[0][0],&(mb+1)->pred_info.mv[0][0],
						sizeof((mb+1)->pred_info.mv)+sizeof((mb+1)->pred_info.ref_idx));
#endif
				}
				else
				{
					mb_bot->mb_field = mb_top->mb_field = 0;
#ifdef _XMM_COPY_
					xmm0 = _mm_load_si128((__m128i *) &mb->pred_info.mv[LIST_0][0]);
					xmm0 = _mm_shuffle_epi32(xmm0, 0xF0); // RSD
					xmm2 = _mm_load_si128((__m128i *) &mb->pred_info.mv[LIST_1][0]);
					xmm2 = _mm_shuffle_epi32(xmm2, 0xF0); // RSD
					xmm1 = _mm_load_si128((__m128i *) &(mb+1)->pred_info.mv[LIST_0][8]);
					xmm1 = _mm_shuffle_epi32(xmm1, 0xF0); // RSD
					xmm3 = _mm_load_si128((__m128i *) &(mb+1)->pred_info.mv[LIST_1][8]);
					xmm3 = _mm_shuffle_epi32(xmm3, 0xF0); // RSD

					_mm_store_si128((__m128i *) &mb_top->pred_info.mv[LIST_0][ 0], xmm0);
					_mm_store_si128((__m128i *) &mb_top->pred_info.mv[LIST_0][ 4], xmm0);
					_mm_store_si128((__m128i *) &mb_top->pred_info.mv[LIST_0][ 8], xmm1);
					_mm_store_si128((__m128i *) &mb_top->pred_info.mv[LIST_0][12], xmm1);
					_mm_store_si128((__m128i *) &mb_top->pred_info.mv[LIST_1][ 0], xmm2);
					_mm_store_si128((__m128i *) &mb_top->pred_info.mv[LIST_1][ 4], xmm2);
					_mm_store_si128((__m128i *) &mb_top->pred_info.mv[LIST_1][ 8], xmm3);
					_mm_store_si128((__m128i *) &mb_top->pred_info.mv[LIST_1][12], xmm3);

					_mm_store_si128((__m128i *) &mb_bot->pred_info.mv[LIST_0][ 0], xmm0);
					_mm_store_si128((__m128i *) &mb_bot->pred_info.mv[LIST_0][ 4], xmm0);
					_mm_store_si128((__m128i *) &mb_bot->pred_info.mv[LIST_0][ 8], xmm1);
					_mm_store_si128((__m128i *) &mb_bot->pred_info.mv[LIST_0][12], xmm1);
					_mm_store_si128((__m128i *) &mb_bot->pred_info.mv[LIST_1][ 0], xmm2);
					_mm_store_si128((__m128i *) &mb_bot->pred_info.mv[LIST_1][ 4], xmm2);
					_mm_store_si128((__m128i *) &mb_bot->pred_info.mv[LIST_1][ 8], xmm3);
					_mm_store_si128((__m128i *) &mb_bot->pred_info.mv[LIST_1][12], xmm3);
#else
					for(j=0;j<2;j++)
					{
						for(i=0;i<4;i++)
						{
							ii = RSD[i];
							int a = (j<<2)+i;

							mb_top->pred_info.mv[LIST_0][a].mv_comb =
								mb_bot->pred_info.mv[LIST_0][a].mv_comb = mb->pred_info.mv[LIST_0][ii].mv_comb;
							mb_top->pred_info.mv[LIST_1][a].mv_comb =
								mb_bot->pred_info.mv[LIST_1][a].mv_comb = mb->pred_info.mv[LIST_1][ii].mv_comb;

						}
					}
					for(j=2;j<4;j++)
					{
						for(i=0;i<4;i++)
						{
							ii = RSD[i];
							int a = (j<<2)+i;

							mb_top->pred_info.mv[LIST_0][a].mv_comb =
								mb_bot->pred_info.mv[LIST_0][a].mv_comb = (mb+1)->pred_info.mv[LIST_0][8+ii].mv_comb;
							mb_top->pred_info.mv[LIST_1][a].mv_comb =
								mb_bot->pred_info.mv[LIST_1][a].mv_comb = (mb+1)->pred_info.mv[LIST_1][8+ii].mv_comb;

						}
					}
#endif
					for(j=0;j<2;j++)
					{
						mb_bot->pred_info.ref_idx[LIST_0][j] = 
							mb_top->pred_info.ref_idx[LIST_0][j] = mb->pred_info.ref_idx[LIST_0][j];

						mb_bot->pred_info.ref_idx[LIST_1][j] = 
							mb_top->pred_info.ref_idx[LIST_1][j] = mb->pred_info.ref_idx[LIST_1][j];
					}

					for(j=2;j<4;j++)
					{
						mb_bot->pred_info.ref_idx[LIST_0][j] = 
							mb_top->pred_info.ref_idx[LIST_0][j] = (mb+1)->pred_info.ref_idx[LIST_0][j];

						mb_bot->pred_info.ref_idx[LIST_1][j] = 
							mb_top->pred_info.ref_idx[LIST_1][j] = (mb+1)->pred_info.ref_idx[LIST_1][j];
					}
				}
				mb+=2;
				mb_top++;
				mb_bot++;
			}
		}
		else
		{
			mb = &fs->frame->mb_data[0];
			for(mb_loop=0;mb_loop<fs->frame->PicSizeInMbs;mb_loop++)
			{
				mb->mb_field = 0;
				mb++;
			}
		}
	}else{//Mbaff==0

		int mb_width = fs->frame->PicWidthInMbs;

		//! Generate field MVs from Frame MVs
		if (!fs->frame->frame_mbs_only_flag) 	// ER protection for frame gap 
		{   // direct_8x8_inference_flag=1
			mb	 = &fs->frame->mb_data[0];
			mb_top = &fs->top_field->mb_data[0];
			mb_bot = &fs->bottom_field->mb_data[0];

			for(mb_loop_i=0;mb_loop_i<fs->frame->PicSizeInMbs/fs->frame->PicWidthInMbs;mb_loop_i+=2)
			{
				for(mb_loop_j=0;mb_loop_j<fs->frame->PicWidthInMbs;mb_loop_j++)
				{
					mb->mb_field = (mb+mb_width)->mb_field = mb_bot->mb_field = mb_top->mb_field = 0;
#ifdef _XMM_COPY_
					xmm0 = _mm_load_si128((__m128i *) &mb->pred_info.mv[LIST_0][0]);
					xmm0 = _mm_shuffle_epi32(xmm0, 0xF0); // RSD
					xmm2 = _mm_load_si128((__m128i *) &mb->pred_info.mv[LIST_1][0]);
					xmm2 = _mm_shuffle_epi32(xmm2, 0xF0); // RSD
					xmm1 = _mm_load_si128((__m128i *) &(mb+mb_width)->pred_info.mv[LIST_0][8]);
					xmm1 = _mm_shuffle_epi32(xmm1, 0xF0); // RSD
					xmm3 = _mm_load_si128((__m128i *) &(mb+mb_width)->pred_info.mv[LIST_1][8]);
					xmm3 = _mm_shuffle_epi32(xmm3, 0xF0); // RSD

					_mm_store_si128((__m128i *) &mb_top->pred_info.mv[LIST_0][ 0], xmm0);
					_mm_store_si128((__m128i *) &mb_top->pred_info.mv[LIST_0][ 4], xmm0);
					_mm_store_si128((__m128i *) &mb_top->pred_info.mv[LIST_0][ 8], xmm1);
					_mm_store_si128((__m128i *) &mb_top->pred_info.mv[LIST_0][12], xmm1);
					_mm_store_si128((__m128i *) &mb_top->pred_info.mv[LIST_1][ 0], xmm2);
					_mm_store_si128((__m128i *) &mb_top->pred_info.mv[LIST_1][ 4], xmm2);
					_mm_store_si128((__m128i *) &mb_top->pred_info.mv[LIST_1][ 8], xmm3);
					_mm_store_si128((__m128i *) &mb_top->pred_info.mv[LIST_1][12], xmm3);

					_mm_store_si128((__m128i *) &mb_bot->pred_info.mv[LIST_0][ 0], xmm0);
					_mm_store_si128((__m128i *) &mb_bot->pred_info.mv[LIST_0][ 4], xmm0);
					_mm_store_si128((__m128i *) &mb_bot->pred_info.mv[LIST_0][ 8], xmm1);
					_mm_store_si128((__m128i *) &mb_bot->pred_info.mv[LIST_0][12], xmm1);
					_mm_store_si128((__m128i *) &mb_bot->pred_info.mv[LIST_1][ 0], xmm2);
					_mm_store_si128((__m128i *) &mb_bot->pred_info.mv[LIST_1][ 4], xmm2);
					_mm_store_si128((__m128i *) &mb_bot->pred_info.mv[LIST_1][ 8], xmm3);
					_mm_store_si128((__m128i *) &mb_bot->pred_info.mv[LIST_1][12], xmm3);
#else
					for(j=0;j<2;j++)
					{
						for(i=0;i<4;i++)
						{
							ii = RSD[i];
							int a = (j<<2)+i;

							mb_top->pred_info.mv[LIST_0][a].mv_comb =
								mb_bot->pred_info.mv[LIST_0][a].mv_comb = mb->pred_info.mv[LIST_0][ii].mv_comb;
							mb_top->pred_info.mv[LIST_1][a].mv_comb =
								mb_bot->pred_info.mv[LIST_1][a].mv_comb = mb->pred_info.mv[LIST_1][ii].mv_comb;
						}
					}
					for(j=2;j<4;j++)
					{
						for(i=0;i<4;i++)
						{
							ii = RSD[i];
							int a = (j<<2)+i;

							mb_top->pred_info.mv[LIST_0][a].mv_comb =
								mb_bot->pred_info.mv[LIST_0][a].mv_comb = (mb+mb_width)->pred_info.mv[LIST_0][8+ii].mv_comb;
							mb_top->pred_info.mv[LIST_1][a].mv_comb =
								mb_bot->pred_info.mv[LIST_1][a].mv_comb = (mb+mb_width)->pred_info.mv[LIST_1][8+ii].mv_comb;

						}
					}
#endif
					for(j=0;j<2;j++)
					{
						mb_bot->pred_info.ref_idx[LIST_0][j] = 
							mb_top->pred_info.ref_idx[LIST_0][j] = mb->pred_info.ref_idx[LIST_0][j];

						mb_bot->pred_info.ref_idx[LIST_1][j] = 
							mb_top->pred_info.ref_idx[LIST_1][j] = mb->pred_info.ref_idx[LIST_1][j];
					}

					for(j=2;j<4;j++)
					{
						mb_bot->pred_info.ref_idx[LIST_0][j] = 
							mb_top->pred_info.ref_idx[LIST_0][j] = (mb+mb_width)->pred_info.ref_idx[LIST_0][j];

						mb_bot->pred_info.ref_idx[LIST_1][j] = 
							mb_top->pred_info.ref_idx[LIST_1][j] = (mb+mb_width)->pred_info.ref_idx[LIST_1][j];
					}
					mb++;
					mb_top++;
					mb_bot++;
				}
				mb += mb_width;
			}
		}
		else
		{   // direct_8x8_inference_flag can be 0 or 1
			mb = &fs->frame->mb_data[0];
			for(mb_loop=0;mb_loop<fs->frame->PicSizeInMbs;mb_loop++)
			{
				mb->mb_field = 0;
				mb++;
			}
		}
	}

	return CREL_OK;
}


/*!
************************************************************************
* \brief
*    Generate a frame from top and bottom fields, 
*    YUV components and display information only
************************************************************************
*/
CREL_RETURN dpb_combine_field_yuv PARGS1(FrameStore *fs)
{
	
	fs->frame = get_storable_picture ARGS7(FRAME, fs->top_field->size_x, fs->top_field->size_y*2, fs->top_field->size_x_cr, fs->top_field->size_y_cr*2, 0, fs->top_field->non_existing);
	if (fs->frame == NULL) {
		return CREL_WARNING_H264_STREAMERROR_LEVEL_1|CREL_WARNING_H264_ERROR_DPB;			
	}
#ifdef _HW_ACCEL_
	if(g_DXVAVer==IviNotDxva || IMGPAR Hybrid_Decoding)
#else
	if(1)
#endif
	{
		fs->frame->FrameBuffer = NULL;
		fs->frame->imgY     = fs->top_field->imgY;
		fs->frame->imgUV    = fs->top_field->imgUV;
		fs->frame->pDownSampleY	= fs->top_field->pDownSampleY;
		fs->frame->pDownSampleUV	= fs->top_field->pDownSampleUV;
	}

	fs->poc=fs->frame->poc =fs->frame->frame_poc = min (fs->top_field->poc, fs->bottom_field->poc);
	fs->frame_num = fs->frame->frame_num = min (fs->top_field->frame_num, fs->bottom_field->frame_num);

	fs->bottom_field->frame_poc=fs->top_field->frame_poc=
		fs->bottom_field->top_poc=fs->frame->frame_poc=fs->frame->top_poc=fs->top_field->poc;
	fs->top_field->bottom_poc=fs->bottom_field->poc;

	fs->frame->bottom_poc=fs->bottom_field->poc;

	fs->frame->used_for_reference = (fs->top_field->used_for_reference && fs->bottom_field->used_for_reference );
	fs->frame->is_long_term = (fs->top_field->is_long_term && fs->bottom_field->is_long_term );

	if (fs->frame->is_long_term) 
		fs->frame->long_term_frame_idx = fs->long_term_frame_idx;

	fs->frame->top_field    = fs->top_field;
	fs->frame->bottom_field = fs->bottom_field;

	// We should get the same pic_store_idx in top & bottom fields in HWA case.
	if(fs->frame->top_field->pic_store_idx != -1)
		fs->frame->pic_store_idx = fs->frame->top_field->pic_store_idx;
	else
		fs->frame->pic_store_idx = fs->frame->bottom_field->pic_store_idx;

	fs->frame->coded_frame = 0;

	fs->frame->chroma_format_idc = fs->top_field->chroma_format_idc;
	fs->frame->frame_cropping_flag = fs->top_field->frame_cropping_flag;
	if (fs->frame->frame_cropping_flag)
	{
		fs->frame->frame_cropping_rect_top_offset = fs->top_field->frame_cropping_rect_top_offset;
		fs->frame->frame_cropping_rect_bottom_offset = fs->top_field->frame_cropping_rect_bottom_offset;
		fs->frame->frame_cropping_rect_left_offset = fs->top_field->frame_cropping_rect_left_offset;
		fs->frame->frame_cropping_rect_right_offset = fs->top_field->frame_cropping_rect_right_offset;
	}

	fs->frame->framerate1000 = fs->top_field->framerate1000;
	fs->frame->dwXAspect = fs->top_field->dwXAspect;
	fs->frame->dwYAspect = fs->top_field->dwYAspect;
	fs->frame->dwBitRate = fs->top_field->dwBitRate;

	//Using slice_type of first field
	if (fs->top_field->poc < fs->bottom_field->poc)
		fs->frame->slice_type = fs->top_field->slice_type;
	else    
		fs->frame->slice_type = fs->bottom_field->slice_type;	    

	if (fs->top_field->has_pts && !fs->bottom_field->has_pts)
	{
		fs->frame->pts = fs->top_field->pts;
		fs->frame->has_pts = fs->top_field->has_pts;
	}
	else if(!fs->top_field->has_pts && fs->bottom_field->has_pts)
	{
		fs->frame->pts = fs->bottom_field->pts;
		fs->frame->has_pts = fs->bottom_field->has_pts;
	}
	else
	{
		if (fs->top_field->poc < fs->bottom_field->poc)
		{		  
			fs->frame->pts = fs->top_field->pts;
			fs->frame->has_pts = fs->top_field->has_pts;
		}
		else  
		{		  
			fs->frame->pts = fs->bottom_field->pts;
			fs->frame->has_pts = fs->bottom_field->has_pts;
		}
	}

	int view_index = fs->top_field->view_index;
	fs->frame->NumClockTs = fs->top_field->NumClockTs;
	fs->frame->SkipedBFrames[view_index][0] = fs->top_field->SkipedBFrames[view_index][0];
	fs->frame->SkipedBFrames[view_index][1] = fs->top_field->SkipedBFrames[view_index][1];

	fs->frame->frame_mbs_only_flag = (fs->top_field->frame_mbs_only_flag & fs->bottom_field->frame_mbs_only_flag);
	fs->top_field->frame = fs->bottom_field->frame = fs->frame;
	fs->top_field->progressive_frame = fs->bottom_field->progressive_frame = fs->frame->progressive_frame = 0;

	fs->frame->view_id = fs->top_field->view_id;
	fs->frame->view_index = fs->top_field->view_index;

	return CREL_OK;
}


/*!
************************************************************************
* \brief
*    Generate a frame from top and bottom fields
************************************************************************
*/
CREL_RETURN dpb_combine_field PARGS1(FrameStore *fs)
{
#ifdef DO_REF_PIC_NUM
	int i;
#endif
	int j;
	int dummylist0, dummylist1;
	Macroblock_s *mb, *mb_top, *mb_bot;
	int mb_loop, mb_loop_i, mb_loop_j;
	CREL_RETURN ret;
#ifdef _XMM_COPY_
	__m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7;
#endif

	ret = dpb_combine_field_yuv ARGS1(fs);
	if (FAILED(ret) || fs->frame->non_existing) {
		return ret;
	}

	
	//combine field for frame
#ifdef DO_REF_PIC_NUM
	for (j=0; j<=(max(fs->top_field->max_slice_id, fs->bottom_field->max_slice_id)); j++)
	{
		for (i=0;i<(listXsize[LIST_1]+1)/2;i++)
		{
			fs->frame->ref_pic_num[j][LIST_1][i]=   min ((fs->top_field->ref_pic_num[j][LIST_1][2*i]/2)*2, (fs->bottom_field->ref_pic_num[j][LIST_1][2*i]/2)*2);
		}

		for (i=0;i<(listXsize[LIST_0]+1)/2;i++)
		{
			fs->frame->ref_pic_num[j][LIST_0][i] =   min ((fs->top_field->ref_pic_num[j][LIST_0][2*i]/2)*2, (fs->bottom_field->ref_pic_num[j][LIST_0][2*i]/2)*2);  
		}
	}
#endif

	//! Use inference flag to remap mvs/references

	//! Generate Frame parameters from field information.
	mb	   = &fs->frame->mb_data[0];
	mb_top = &fs->top_field->mb_data[0];
	mb_bot = &fs->bottom_field->mb_data[0];

	if(active_sps.mb_adaptive_frame_field_flag)
	{
		for(mb_loop=0;mb_loop<fs->frame->PicSizeInMbs/2;mb_loop++)
		{
			mb->mb_field = (mb+1)->mb_field = mb_top->mb_field = mb_bot->mb_field = 1;

#ifdef _XMM_COPY_
			xmm0 = _mm_load_si128((__m128i *) &mb_top->pred_info.mv[LIST_0][ 0]);
			xmm1 = _mm_load_si128((__m128i *) &mb_top->pred_info.mv[LIST_0][ 4]);
			xmm2 = _mm_load_si128((__m128i *) &mb_top->pred_info.mv[LIST_0][ 8]);
			xmm3 = _mm_load_si128((__m128i *) &mb_top->pred_info.mv[LIST_0][12]);
			xmm4 = _mm_load_si128((__m128i *) &mb_top->pred_info.mv[LIST_1][ 0]);
			xmm5 = _mm_load_si128((__m128i *) &mb_top->pred_info.mv[LIST_1][ 4]);
			xmm6 = _mm_load_si128((__m128i *) &mb_top->pred_info.mv[LIST_1][ 8]);
			xmm7 = _mm_load_si128((__m128i *) &mb_top->pred_info.mv[LIST_1][12]);
			_mm_store_si128((__m128i *) &mb->pred_info.mv[LIST_0][ 0], xmm0);
			_mm_store_si128((__m128i *) &mb->pred_info.mv[LIST_0][ 4], xmm1);
			_mm_store_si128((__m128i *) &mb->pred_info.mv[LIST_0][ 8], xmm2);
			_mm_store_si128((__m128i *) &mb->pred_info.mv[LIST_0][12], xmm3);
			_mm_store_si128((__m128i *) &mb->pred_info.mv[LIST_1][ 0], xmm4);
			_mm_store_si128((__m128i *) &mb->pred_info.mv[LIST_1][ 4], xmm5);
			_mm_store_si128((__m128i *) &mb->pred_info.mv[LIST_1][ 8], xmm6);
			_mm_store_si128((__m128i *) &mb->pred_info.mv[LIST_1][12], xmm7);
#else
			memcpy(mb->pred_info.mv, mb_top->pred_info.mv, sizeof(mb_top->pred_info.mv));
#endif
			for(j=0;j<4;j++)
			{
				dummylist0= 
					mb->pred_info.ref_idx[LIST_0][j] = mb_top->pred_info.ref_idx[LIST_0][j];
				dummylist1= 
					mb->pred_info.ref_idx[LIST_1][j] = mb_top->pred_info.ref_idx[LIST_1][j];

				//! association with id already known for fields.
				//! need to make association with frames

				//Check if frame exists to resist stream error in ref_pic_id, no Level 1 Warning necessary, just set -1
				//mb->pred_info.ref_pic_id[LIST_0][j] =((dummylist0>=0)&&(storable_picture_map[mb_top->pred_info.ref_pic_id[LIST_0][j]]->frame))?
				//	storable_picture_map[mb_top->pred_info.ref_pic_id[LIST_0][j]]->frame->unique_id : -1;
				//mb->pred_info.ref_pic_id[LIST_1][j] =((dummylist1>=0)&&(storable_picture_map[mb_top->pred_info.ref_pic_id[LIST_1][j]]->frame))? 
				//	storable_picture_map[mb_top->pred_info.ref_pic_id[LIST_1][j]]->frame->unique_id : -1;
				if (dummylist0>=0)
				{
					if (storable_picture_map[mb_top->pred_info.ref_pic_id[LIST_0][j]]->frame == NULL)
					{
						//DEBUG_SHOW_SW_INFO("WARNING TOP[%d %d POC: %d MBType: %d]this unique id: %d POC: %d", mb_loop_i, mb_loop_j, IMGPAR ThisPOC, mb_top->mb_type, mb_top->pred_info.ref_pic_id[LIST_0][j], storable_picture_map[mb_top->pred_info.ref_pic_id[LIST_0][j]]->top_poc);
						mb->pred_info.ref_pic_id[LIST_0][j] = fs->frame->unique_id;
						//DEBUG_SHOW_SW_INFO("WARNING Concealment, unique id: %d", fs->frame->unique_id);
					}
					else
						mb->pred_info.ref_pic_id[LIST_0][j] = storable_picture_map[mb_top->pred_info.ref_pic_id[LIST_0][j]]->frame->unique_id;
				}
				else
					mb->pred_info.ref_pic_id[LIST_0][j] = -1;

				if (dummylist1>=0)
				{
					if (storable_picture_map[mb_top->pred_info.ref_pic_id[LIST_1][j]]->frame == NULL)
					{
						//DEBUG_SHOW_SW_INFO("WARNING TOP[%d %d POC: %d MBType: %d]this unique id: %d POC: %d", mb_loop_i, mb_loop_j, IMGPAR ThisPOC, mb_top->mb_type, mb_top->pred_info.ref_pic_id[LIST_1][j], storable_picture_map[mb_top->pred_info.ref_pic_id[LIST_1][j]]->top_poc);
						mb->pred_info.ref_pic_id[LIST_1][j] = fs->frame->unique_id;
						//DEBUG_SHOW_SW_INFO("WARNING Concealment, unique id: %d", fs->frame->unique_id);
					}
					else
						mb->pred_info.ref_pic_id[LIST_1][j] = storable_picture_map[mb_top->pred_info.ref_pic_id[LIST_1][j]]->frame->unique_id;
				}
				else
					mb->pred_info.ref_pic_id[LIST_1][j] = -1;
			}

			mb++;

#ifdef _XMM_COPY_
			xmm0 = _mm_load_si128((__m128i *) &mb_bot->pred_info.mv[LIST_0][ 0]);
			xmm1 = _mm_load_si128((__m128i *) &mb_bot->pred_info.mv[LIST_0][ 4]);
			xmm2 = _mm_load_si128((__m128i *) &mb_bot->pred_info.mv[LIST_0][ 8]);
			xmm3 = _mm_load_si128((__m128i *) &mb_bot->pred_info.mv[LIST_0][12]);
			xmm4 = _mm_load_si128((__m128i *) &mb_bot->pred_info.mv[LIST_1][ 0]);
			xmm5 = _mm_load_si128((__m128i *) &mb_bot->pred_info.mv[LIST_1][ 4]);
			xmm6 = _mm_load_si128((__m128i *) &mb_bot->pred_info.mv[LIST_1][ 8]);
			xmm7 = _mm_load_si128((__m128i *) &mb_bot->pred_info.mv[LIST_1][12]);
			_mm_store_si128((__m128i *) &mb->pred_info.mv[LIST_0][ 0], xmm0);
			_mm_store_si128((__m128i *) &mb->pred_info.mv[LIST_0][ 4], xmm1);
			_mm_store_si128((__m128i *) &mb->pred_info.mv[LIST_0][ 8], xmm2);
			_mm_store_si128((__m128i *) &mb->pred_info.mv[LIST_0][12], xmm3);
			_mm_store_si128((__m128i *) &mb->pred_info.mv[LIST_1][ 0], xmm4);
			_mm_store_si128((__m128i *) &mb->pred_info.mv[LIST_1][ 4], xmm5);
			_mm_store_si128((__m128i *) &mb->pred_info.mv[LIST_1][ 8], xmm6);
			_mm_store_si128((__m128i *) &mb->pred_info.mv[LIST_1][12], xmm7);
#else
			memcpy(mb->pred_info.mv, mb_bot->pred_info.mv, sizeof(mb_bot->pred_info.mv));
#endif
			for(j=0;j<4;j++)
			{
				dummylist0= mb->pred_info.ref_idx[LIST_0][j] = mb_bot->pred_info.ref_idx[LIST_0][j];
				dummylist1= mb->pred_info.ref_idx[LIST_1][j] = mb_bot->pred_info.ref_idx[LIST_1][j];

				//! association with id already known for fields.
				//! need to make association with frames

				//Check if frame exists to resist stream error in ref_pic_id, no Level 1 Warning necessary, just set -1
				//mb->pred_info.ref_pic_id[LIST_0][j] =((dummylist0>=0)&&(storable_picture_map[mb_bot->pred_info.ref_pic_id[LIST_0][j]]->frame))?
				//	storable_picture_map[mb_bot->pred_info.ref_pic_id[LIST_0][j]]->frame->unique_id : -1;
				//mb->pred_info.ref_pic_id[LIST_1][j] =((dummylist1>=0)&&(storable_picture_map[mb_bot->pred_info.ref_pic_id[LIST_1][j]]->frame))? 
				//	storable_picture_map[mb_bot->pred_info.ref_pic_id[LIST_1][j]]->frame->unique_id : -1;
				if (dummylist0>=0)
				{
					if (storable_picture_map[mb_bot->pred_info.ref_pic_id[LIST_0][j]]->frame == NULL)
					{
						//DEBUG_SHOW_SW_INFO("WARNING BOT[%d %d POC: %d MBType: %d]this unique id: %d POC: %d", mb_loop_i, mb_loop_j, IMGPAR ThisPOC, mb_bot->mb_type, mb_bot->pred_info.ref_pic_id[LIST_0][j], storable_picture_map[mb_bot->pred_info.ref_pic_id[LIST_0][j]]->bottom_poc);
						mb->pred_info.ref_pic_id[LIST_0][j] = fs->frame->unique_id;
						//DEBUG_SHOW_SW_INFO("WARNING Concealment unique id: %d", fs->frame->unique_id);
					}
					else
						mb->pred_info.ref_pic_id[LIST_0][j] = storable_picture_map[mb_bot->pred_info.ref_pic_id[LIST_0][j]]->frame->unique_id;
				}
				else
					mb->pred_info.ref_pic_id[LIST_0][j] = -1;

				if (dummylist1>=0)
				{
					if (storable_picture_map[mb_bot->pred_info.ref_pic_id[LIST_1][j]]->frame == NULL)
					{
						//DEBUG_SHOW_SW_INFO("WARNING BOT[%d %d POC: %d MBType: %d]this unique id: %d POC: %d", mb_loop_i, mb_loop_j, IMGPAR ThisPOC, mb_bot->mb_type, mb_bot->pred_info.ref_pic_id[LIST_1][j], storable_picture_map[mb_bot->pred_info.ref_pic_id[LIST_1][j]]->bottom_poc);
						mb->pred_info.ref_pic_id[LIST_1][j] = fs->frame->unique_id;
						//DEBUG_SHOW_SW_INFO("WARNING Concealment unique id: %d", fs->frame->unique_id);
					}
					else
						mb->pred_info.ref_pic_id[LIST_1][j] = storable_picture_map[mb_bot->pred_info.ref_pic_id[LIST_1][j]]->frame->unique_id;
				}
				else
					mb->pred_info.ref_pic_id[LIST_1][j] = -1;
			}

			mb++;
			mb_top++;
			mb_bot++;
		}
	}
	else
	{
		int mb_width = fs->top_field->PicWidthInMbs;

		for(mb_loop_i=0;mb_loop_i<fs->frame->PicSizeInMbs/mb_width/*fs->frame->PicWidthInMbs*/;mb_loop_i+=2)
		{
			for(mb_loop_j=0;mb_loop_j<mb_width/*fs->frame->PicWidthInMbs*/;mb_loop_j++)
			{
				mb->mb_field = (mb+mb_width)->mb_field = mb_top->mb_field = mb_bot->mb_field = 1;

#ifdef _XMM_COPY_
				xmm0 = _mm_load_si128((__m128i *) &mb_top->pred_info.mv[LIST_0][ 0]);
				xmm1 = _mm_load_si128((__m128i *) &mb_top->pred_info.mv[LIST_0][ 4]);
				xmm2 = _mm_load_si128((__m128i *) &mb_top->pred_info.mv[LIST_0][ 8]);
				xmm3 = _mm_load_si128((__m128i *) &mb_top->pred_info.mv[LIST_0][12]);
				xmm4 = _mm_load_si128((__m128i *) &mb_top->pred_info.mv[LIST_1][ 0]);
				xmm5 = _mm_load_si128((__m128i *) &mb_top->pred_info.mv[LIST_1][ 4]);
				xmm6 = _mm_load_si128((__m128i *) &mb_top->pred_info.mv[LIST_1][ 8]);
				xmm7 = _mm_load_si128((__m128i *) &mb_top->pred_info.mv[LIST_1][12]);
				_mm_store_si128((__m128i *) &mb->pred_info.mv[LIST_0][ 0], xmm0);
				_mm_store_si128((__m128i *) &mb->pred_info.mv[LIST_0][ 4], xmm1);
				_mm_store_si128((__m128i *) &mb->pred_info.mv[LIST_0][ 8], xmm2);
				_mm_store_si128((__m128i *) &mb->pred_info.mv[LIST_0][12], xmm3);
				_mm_store_si128((__m128i *) &mb->pred_info.mv[LIST_1][ 0], xmm4);
				_mm_store_si128((__m128i *) &mb->pred_info.mv[LIST_1][ 4], xmm5);
				_mm_store_si128((__m128i *) &mb->pred_info.mv[LIST_1][ 8], xmm6);
				_mm_store_si128((__m128i *) &mb->pred_info.mv[LIST_1][12], xmm7);
#else
				memcpy(mb->pred_info.mv, mb_top->pred_info.mv, sizeof(mb_top->pred_info.mv));
#endif

				for(j=0;j<4;j++)
				{
					dummylist0= 
						mb->pred_info.ref_idx[LIST_0][j] = mb_top->pred_info.ref_idx[LIST_0][j];
					dummylist1= 
						mb->pred_info.ref_idx[LIST_1][j] = mb_top->pred_info.ref_idx[LIST_1][j];

					//! association with id already known for fields.
					//! need to make association with frames
					//Check if frame exists to resist stream error in ref_pic_id, no Level 1 Warning necessary, just set -1
					//mb->pred_info.ref_pic_id[LIST_0][j] =((dummylist0>=0)&&(storable_picture_map[mb_top->pred_info.ref_pic_id[LIST_0][j]]->frame))?
					//	storable_picture_map[mb_top->pred_info.ref_pic_id[LIST_0][j]]->frame->unique_id : -1;
					//mb->pred_info.ref_pic_id[LIST_1][j] =((dummylist1>=0)&&(storable_picture_map[mb_top->pred_info.ref_pic_id[LIST_1][j]]->frame))? 
					//	storable_picture_map[mb_top->pred_info.ref_pic_id[LIST_1][j]]->frame->unique_id : -1;
					if (dummylist0>=0)
					{
						if (storable_picture_map[mb_top->pred_info.ref_pic_id[LIST_0][j]]->frame == NULL)
						{
							//DEBUG_SHOW_SW_INFO("WARNING TOP[%d %d POC: %d MBType: %d]this unique id: %d POC: %d", mb_loop_i, mb_loop_j, IMGPAR ThisPOC, mb_top->mb_type, mb_top->pred_info.ref_pic_id[LIST_0][j], storable_picture_map[mb_top->pred_info.ref_pic_id[LIST_0][j]]->top_poc);
							mb->pred_info.ref_pic_id[LIST_0][j] = fs->frame->unique_id;
							//DEBUG_SHOW_SW_INFO("WARNING Concealment unique id: %d", fs->frame->unique_id);
						}
						else
							mb->pred_info.ref_pic_id[LIST_0][j] = storable_picture_map[mb_top->pred_info.ref_pic_id[LIST_0][j]]->frame->unique_id;
					}
					else
						mb->pred_info.ref_pic_id[LIST_0][j] = -1;

					if (dummylist1>=0)
					{
						if (storable_picture_map[mb_top->pred_info.ref_pic_id[LIST_1][j]]->frame == NULL)
						{
							//DEBUG_SHOW_SW_INFO("WARNING TOP[%d %d POC: %d MBType: %d]this unique id: %d POC: %d", mb_loop_i, mb_loop_j, IMGPAR ThisPOC, mb_top->mb_type, mb_top->pred_info.ref_pic_id[LIST_1][j], storable_picture_map[mb_top->pred_info.ref_pic_id[LIST_1][j]]->top_poc);
							mb->pred_info.ref_pic_id[LIST_1][j] = fs->frame->unique_id;
							//DEBUG_SHOW_SW_INFO("WARNING Concealment unique id: %d", fs->frame->unique_id);
						}
						else
							mb->pred_info.ref_pic_id[LIST_1][j] = storable_picture_map[mb_top->pred_info.ref_pic_id[LIST_1][j]]->frame->unique_id;
					}
					else
						mb->pred_info.ref_pic_id[LIST_1][j] = -1;
				}

				mb->mb_type = mb_top->mb_type;
				mb->qp = mb_top->qp;
				mb+=mb_width;

#ifdef _XMM_COPY_
				xmm0 = _mm_load_si128((__m128i *) &mb_bot->pred_info.mv[LIST_0][ 0]);
				xmm1 = _mm_load_si128((__m128i *) &mb_bot->pred_info.mv[LIST_0][ 4]);
				xmm2 = _mm_load_si128((__m128i *) &mb_bot->pred_info.mv[LIST_0][ 8]);
				xmm3 = _mm_load_si128((__m128i *) &mb_bot->pred_info.mv[LIST_0][12]);
				xmm4 = _mm_load_si128((__m128i *) &mb_bot->pred_info.mv[LIST_1][ 0]);
				xmm5 = _mm_load_si128((__m128i *) &mb_bot->pred_info.mv[LIST_1][ 4]);
				xmm6 = _mm_load_si128((__m128i *) &mb_bot->pred_info.mv[LIST_1][ 8]);
				xmm7 = _mm_load_si128((__m128i *) &mb_bot->pred_info.mv[LIST_1][12]);
				_mm_store_si128((__m128i *) &mb->pred_info.mv[LIST_0][ 0], xmm0);
				_mm_store_si128((__m128i *) &mb->pred_info.mv[LIST_0][ 4], xmm1);
				_mm_store_si128((__m128i *) &mb->pred_info.mv[LIST_0][ 8], xmm2);
				_mm_store_si128((__m128i *) &mb->pred_info.mv[LIST_0][12], xmm3);
				_mm_store_si128((__m128i *) &mb->pred_info.mv[LIST_1][ 0], xmm4);
				_mm_store_si128((__m128i *) &mb->pred_info.mv[LIST_1][ 4], xmm5);
				_mm_store_si128((__m128i *) &mb->pred_info.mv[LIST_1][ 8], xmm6);
				_mm_store_si128((__m128i *) &mb->pred_info.mv[LIST_1][12], xmm7);
#else
				memcpy(mb->pred_info.mv, mb_bot->pred_info.mv, sizeof(mb_bot->pred_info.mv));
#endif
				for(j=0;j<4;j++)
				{
					dummylist0= mb->pred_info.ref_idx[LIST_0][j] = mb_bot->pred_info.ref_idx[LIST_0][j];
					dummylist1= mb->pred_info.ref_idx[LIST_1][j] = mb_bot->pred_info.ref_idx[LIST_1][j];

					//! association with id already known for fields.
					//! need to make association with frames
					//Check if frame exists to resist stream error in ref_pic_id, no Level 1 Warning necessary, just set -1
					//mb->pred_info.ref_pic_id[LIST_0][j] =((dummylist0>=0)&&(storable_picture_map[mb_bot->pred_info.ref_pic_id[LIST_0][j]]->frame))?
					//	storable_picture_map[mb_bot->pred_info.ref_pic_id[LIST_0][j]]->frame->unique_id : -1;
					//mb->pred_info.ref_pic_id[LIST_1][j] =((dummylist1>=0)&&(storable_picture_map[mb_bot->pred_info.ref_pic_id[LIST_1][j]]->frame))? 
					//	storable_picture_map[mb_bot->pred_info.ref_pic_id[LIST_1][j]]->frame->unique_id : -1;
					if (dummylist0>=0)
					{
						if (storable_picture_map[mb_bot->pred_info.ref_pic_id[LIST_0][j]]->frame == NULL)
						{
							//DEBUG_SHOW_SW_INFO("WARNING BOT[%d %d POC: %d MBType: %d]this unique id: %d POC: %d", mb_loop_i, mb_loop_j, IMGPAR ThisPOC, mb_bot->mb_type, mb_bot->pred_info.ref_pic_id[LIST_0][j], storable_picture_map[mb_bot->pred_info.ref_pic_id[LIST_0][j]]->bottom_poc);
							mb->pred_info.ref_pic_id[LIST_0][j] = fs->frame->unique_id;
							//DEBUG_SHOW_SW_INFO("WARNING Concealment unique id: %d", fs->frame->unique_id);
						}
						else
							mb->pred_info.ref_pic_id[LIST_0][j] = storable_picture_map[mb_bot->pred_info.ref_pic_id[LIST_0][j]]->frame->unique_id;
					}
					else
						mb->pred_info.ref_pic_id[LIST_0][j] = -1;

					if (dummylist1>=0)
					{
						if (storable_picture_map[mb_bot->pred_info.ref_pic_id[LIST_1][j]]->frame == NULL)
						{
							//DEBUG_SHOW_SW_INFO("WARNING BOT[%d %d POC: %d MBType: %d]this unique id: %d POC: %d", mb_loop_i, mb_loop_j, IMGPAR ThisPOC, mb_bot->mb_type, mb_bot->pred_info.ref_pic_id[LIST_1][j], storable_picture_map[mb_bot->pred_info.ref_pic_id[LIST_1][j]]->bottom_poc);
							mb->pred_info.ref_pic_id[LIST_1][j] = fs->frame->unique_id;
							//DEBUG_SHOW_SW_INFO("WARNING Concealment unique id: %d", fs->frame->unique_id);
						}
						else
							mb->pred_info.ref_pic_id[LIST_1][j] = storable_picture_map[mb_bot->pred_info.ref_pic_id[LIST_1][j]]->frame->unique_id;
					}
					else
						mb->pred_info.ref_pic_id[LIST_1][j] = -1;
				}

				mb->mb_type = mb_bot->mb_type;
				mb->qp = mb_bot->qp;

				mb-=mb_width-1;
				mb_top++;
				mb_bot++;
			}
			mb += mb_width;
		}
	}

	return CREL_OK;
}


/*!
************************************************************************
* \brief
*    Allocate memory for buffering of reference picture reordering commands
************************************************************************
*/
void alloc_ref_pic_list_reordering_buffer PARGS1(Slice *currSlice)
{
	int size = IMGPAR num_ref_idx_l0_active+1;

	if ((currSlice->remapping_of_pic_nums_idc_l0 = (int *) _aligned_malloc(size*sizeof(int), 16))==NULL) {
		no_mem_exit("alloc_ref_pic_list_reordering_buffer: remapping_of_pic_nums_idc_l0");
		//return CREL_ERROR_H264_NOMEMORY;
	}
	if ((currSlice->abs_diff_pic_num_minus1_l0 = (int *) _aligned_malloc(size*sizeof(int), 16))==NULL) {
		no_mem_exit("alloc_ref_pic_list_reordering_buffer: abs_diff_pic_num_minus1_l0");
		//return CREL_ERROR_H264_NOMEMORY;
	}
	if ((currSlice->long_term_pic_idx_l0 = (int *) _aligned_malloc(size*sizeof(int), 16))==NULL) {
		no_mem_exit("alloc_ref_pic_list_reordering_buffer: long_term_pic_idx_l0");
		//return CREL_ERROR_H264_NOMEMORY;
	}

	if ((currSlice->abs_diff_view_idx_minus1_l0 = (int *) _aligned_malloc(size*sizeof(int), 16))==NULL) {
		no_mem_exit("alloc_ref_pic_list_reordering_buffer: abs_diff_view_idx_minus1_l0");
		//return CREL_ERROR_H264_NOMEMORY;
	}

	size = IMGPAR num_ref_idx_l1_active+1;

	if (IMGPAR type==B_SLICE)
	{
		if ((currSlice->remapping_of_pic_nums_idc_l1 = (int *) _aligned_malloc(size*sizeof(int), 16))==NULL) {
			no_mem_exit("alloc_ref_pic_list_reordering_buffer: remapping_of_pic_nums_idc_l1");
			//return CREL_ERROR_H264_NOMEMORY;
		}
		if ((currSlice->abs_diff_pic_num_minus1_l1 = (int *) _aligned_malloc(size*sizeof(int), 16))==NULL) {
			no_mem_exit("alloc_ref_pic_list_reordering_buffer: abs_diff_pic_num_minus1_l1");
			//return CREL_ERROR_H264_NOMEMORY;
		}
		if ((currSlice->long_term_pic_idx_l1 = (int *) _aligned_malloc(size*sizeof(int), 16))==NULL) {
			no_mem_exit("alloc_ref_pic_list_reordering_buffer: long_term_pic_idx_l1");
			//return CREL_ERROR_H264_NOMEMORY;
		}

		if ((currSlice->abs_diff_view_idx_minus1_l1 = (int *) _aligned_malloc(size*sizeof(int), 16))==NULL) {
			no_mem_exit("alloc_ref_pic_list_reordering_buffer: abs_diff_view_idx_minus1_l1");
			//return CREL_ERROR_H264_NOMEMORY;
		}
	}
	else
	{
		currSlice->remapping_of_pic_nums_idc_l1 = NULL;
		currSlice->abs_diff_pic_num_minus1_l1 = NULL;
		currSlice->long_term_pic_idx_l1 = NULL;
		currSlice->abs_diff_view_idx_minus1_l1 = NULL;
	}

}


/*!
************************************************************************
* \brief
*    Free memory for buffering of reference picture reordering commands
************************************************************************
*/
void free_ref_pic_list_reordering_buffer(Slice *currSlice)
{

	if (currSlice->remapping_of_pic_nums_idc_l0) 
		_aligned_free(currSlice->remapping_of_pic_nums_idc_l0);
	if (currSlice->abs_diff_pic_num_minus1_l0)
		_aligned_free(currSlice->abs_diff_pic_num_minus1_l0);
	if (currSlice->long_term_pic_idx_l0)
		_aligned_free(currSlice->long_term_pic_idx_l0);
	if (currSlice->abs_diff_view_idx_minus1_l0)
		_aligned_free(currSlice->abs_diff_view_idx_minus1_l0);

	currSlice->remapping_of_pic_nums_idc_l0 = NULL;
	currSlice->abs_diff_pic_num_minus1_l0 = NULL;
	currSlice->long_term_pic_idx_l0 = NULL;
	currSlice->abs_diff_view_idx_minus1_l0 = NULL;

	if (currSlice->remapping_of_pic_nums_idc_l1)
		_aligned_free(currSlice->remapping_of_pic_nums_idc_l1);
	if (currSlice->abs_diff_pic_num_minus1_l1)
		_aligned_free(currSlice->abs_diff_pic_num_minus1_l1);
	if (currSlice->long_term_pic_idx_l1)
		_aligned_free(currSlice->long_term_pic_idx_l1);
	if (currSlice->abs_diff_view_idx_minus1_l1)
		_aligned_free(currSlice->abs_diff_view_idx_minus1_l1);

	currSlice->remapping_of_pic_nums_idc_l1 = NULL;
	currSlice->abs_diff_pic_num_minus1_l1 = NULL;
	currSlice->long_term_pic_idx_l1 = NULL;
	currSlice->abs_diff_view_idx_minus1_l1 = NULL;
}

/*!
************************************************************************
* \brief
*      Tian Dong
*          June 13, 2002, Modifed on July 30, 2003
*
*      If a gap in frame_num is found, try to fill the gap
* \param img
*      
************************************************************************
*/
CREL_RETURN fill_frame_num_gap PARGS0()
{
	int CurrFrameNum;
	int UnusedShortTermFrameNum;
	int ret;
	StorablePicture *picture = NULL;
	int tmp1 = IMGPAR delta_pic_order_cnt[0];
	int tmp2 = IMGPAR delta_pic_order_cnt[1];
	IMGPAR delta_pic_order_cnt[0] = IMGPAR delta_pic_order_cnt[1] = 0;

	DEBUG_SHOW_SW_INFO("A gap in frame number is found, try to fill it.\n");


	UnusedShortTermFrameNum = (IMGPAR pre_frame_num + 1) % IMGPAR MaxFrameNum;
	CurrFrameNum = IMGPAR frame_num;

	while (CurrFrameNum != UnusedShortTermFrameNum)
	{

		picture = get_storable_picture ARGS7(FRAME, IMGPAR width, IMGPAR height, IMGPAR width_cr, IMGPAR height_cr, 0, 1);
		if (picture == NULL) {
			return CREL_WARNING_H264_STREAMERROR_LEVEL_1|CREL_WARNING_H264_ERROR_DPB;
		}
		picture->coded_frame = 1;
		picture->pic_num = UnusedShortTermFrameNum;
		picture->frame_num = UnusedShortTermFrameNum;
		picture->non_existing = 1;
		picture->is_output = 1;
		picture->used_for_reference = 1;
		picture->frame_mbs_only_flag = active_sps.frame_mbs_only_flag;
		picture->adaptive_ref_pic_buffering_flag = 0;
		picture->PicWidthInMbs = IMGPAR PicWidthInMbs;

		IMGPAR frame_num = UnusedShortTermFrameNum;
		if (active_sps.pic_order_cnt_type!=0)
		{
			ret = decode_poc ARGS0();
			if (FAILED(ret)) {
				return ret;
			}
		}
		picture->top_poc=IMGPAR toppoc;
		picture->bottom_poc=IMGPAR bottompoc;
		picture->frame_poc=IMGPAR framepoc;
		picture->poc=IMGPAR framepoc;

		ret = store_picture_in_dpb ARGS1(picture);
		if (FAILED(ret)) {
			return ret;
		}

		picture=NULL;
		IMGPAR pre_frame_num = UnusedShortTermFrameNum;
		UnusedShortTermFrameNum = (UnusedShortTermFrameNum + 1) % IMGPAR MaxFrameNum;
	}
	IMGPAR delta_pic_order_cnt[0] = tmp1;
	IMGPAR delta_pic_order_cnt[1] = tmp2;
	IMGPAR frame_num = CurrFrameNum;

	return ret;

}

ColocatedParamsMB* alloc_colocatedMB(int size_x, int size_y, int mb_adaptive_frame_field_flag)
{
	ColocatedParamsMB *s;

	s = (struct colocated_params_MB *) _aligned_malloc(sizeof(ColocatedParamsMB), 16); 
	if (NULL != s) {
		s->is_long_term = 0;
		//s->size_x = size_x;
		//s->size_y = size_y;
	}

	return s;
}


void free_colocated_MB(ColocatedParamsMB* p)
{
	if(!p)
		return;
	_aligned_free(p);
	p=NULL;
}


#if 0
/*!
************************************************************************
* \brief
*    Compute co-located motion info
*
************************************************************************
*/
void ColocatedGenFieldMVs PARGS8(ColocatedParamsMB* p, int start_y, int start_x, int loop_y, int loop_x,
																 StorablePicture *fs_field, StorablePicture *fs, int step)
{	
	int i, j, ii, jj;
	int end_y = start_y+loop_y;	
	int end_x = start_x+loop_x;	

	Pred_s_info *info = &p->pred_info;

	Macroblock_s *mb;

	mb = &fs_field->mb_data[(IMGPAR current_mb_nr_r)>>1];

	for (j=start_y;j<end_y;j+=step)
	{
		jj = RSD[j];

		for (i=start_x;i<end_x;i+=step)
		{
			ii = RSD[i];

			if (IMGPAR direct_spatial_mv_pred_flag == 1)
			{
				p->moving_block[j][i] = 
					!((!fs_field->is_long_term 
					&& (((mb->pred_info.ref_idx[LIST_0][l_16_4[jj*4+ii]]) == 0) 
					&&  (fast_abs_short((mb->pred_info.mv[LIST_0][jj*4+ii].x))>>1 == 0) 
					&&  (fast_abs_short((mb->pred_info.mv[LIST_0][jj*4+ii].y))>>1 == 0))) 
					|| (((mb->pred_info.ref_idx[LIST_0][l_16_4[jj*4+ii]]) == -1) 
					&&  ((mb->pred_info.ref_idx[LIST_1][l_16_4[jj*4+ii]]) == 0) 
					&&  (fast_abs_short((mb->pred_info.mv[LIST_1][jj*4+ii].x))>>1 == 0) 
					&&  (fast_abs_short((mb->pred_info.mv[LIST_1][jj*4+ii].y))>>1 == 0)));
			}
			else
			{
				info->mv[0][j*4+i].mv_comb = mb->pred_info.mv[LIST_0][jj*4+ii].mv_comb;
				info->mv[1][j*4+i].mv_comb = mb->pred_info.mv[LIST_1][jj*4+ii].mv_comb;

				info->ref_idx[0][l_16_4[j*4+i]] = mb->pred_info.ref_idx[LIST_0][l_16_4[jj*4+ii]];
				info->ref_idx[1][l_16_4[j*4+i]] = mb->pred_info.ref_idx[LIST_1][l_16_4[jj*4+ii]];

				info->ref_pic_id[0][l_16_4[j*4+i]] = mb->pred_info.ref_pic_id[LIST_0][l_16_4[jj*4+ii]];
				info->ref_pic_id[1][l_16_4[j*4+i]] = mb->pred_info.ref_pic_id[LIST_1][l_16_4[jj*4+ii]];

				if (!mb->mb_field)
				{
					info->mv[0][j*4+i].y /= 2;
					info->mv[1][j*4+i].y /= 2;
				}
			}
		}
	}
}
#endif



void compute_colocated_SUBMB_frame_spatial PARGS6(ColocatedParamsMB* p, StorablePicture **list[6],int start_y, int start_x,
																									int loop_y, int loop_x)
{
	StorablePicture *fs, *fs_top, *fs_bottom;
	int i,j;	
	int ii,jj;
	int RSDi,RSDj;
	int end_y = start_y+loop_y;	
	int end_x = start_x+loop_x;
	Macroblock_s *mb;	
	Macroblock_s *mb_field;

	fs            = list[LIST_1][0];
	if (IMGPAR MbaffFrameFlag)
	{
		fs_top    = list[LIST_1 + 2][0];
		fs_bottom = list[LIST_1 + 4][0];
	}
	else
	{
		fs_top    = 
			fs_bottom = list[LIST_1][0];
	}

	if (!currMB_r->mb_field)
	{
		if (active_sps.direct_8x8_inference_flag)
		{   // frame_mbs_only_flag can be 0 or 1
			mb = &fs->mb_data[IMGPAR current_mb_nr_r];

			if(mb->mb_field)
			{
				int joff;

				Macroblock_s *mb_temp = &fs->mb_data[(IMGPAR current_mb_nr_r>>1)<<1];
				joff = (IMGPAR mb_y_r&1)<<1;

				//! Assign frame buffers for field MBs   
				//! Check whether we should use top or bottom field mvs.
				//! Depending on the assigned poc values.

				if (abs(dec_picture->poc - fs->bottom_field->poc)> abs(dec_picture->poc - fs->top_field->poc) )
				{
					if(IMGPAR MbaffFrameFlag)
					{
						mb_field = &fs->top_field->mb_data[IMGPAR current_mb_nr_r>>1];
					}
					else
					{
						mb_field = &fs->top_field->mb_data[(IMGPAR mb_y_r>>1)*IMGPAR PicWidthInMbs+IMGPAR mb_x_r];
						mb_temp  = &fs->mb_data[(IMGPAR mb_y_r>>1)*IMGPAR PicWidthInMbs*2+IMGPAR mb_x_r];
					}
					p->is_long_term = fs->top_field->is_long_term;
				}
				else
				{
					if(IMGPAR MbaffFrameFlag)
					{
						mb_field = &fs->bottom_field->mb_data[IMGPAR current_mb_nr_r>>1];
						mb_temp++;
					}
					else
					{
						mb_field = &fs->bottom_field->mb_data[(IMGPAR mb_y_r>>1)*IMGPAR PicWidthInMbs+IMGPAR mb_x_r];
						mb_temp  = &fs->mb_data[(IMGPAR mb_y_r>>1)*IMGPAR PicWidthInMbs*2+IMGPAR mb_x_r];
						mb_temp+=IMGPAR PicWidthInMbs;
					}
					p->is_long_term = fs->bottom_field->is_long_term;
				}

				for(j=start_y;j<end_y;j+=2) 
				{
					RSDj = (RSD[j]>>1)+joff;
					for (i=start_x;i<end_x;i+=2)
					{
						RSDi = RSD[i];

						p->moving_block[j][i]= 
							!((!p->is_long_term 
							&& (((mb_field->pred_info.ref_idx[LIST_0][l_16_4[RSDj*4+RSDi]]) == 0) 
							&&  (fast_abs_short((mb_field->pred_info.mv[LIST_0][RSDj*4+RSDi].x))>>1 == 0) 
							&&  (fast_abs_short((mb_field->pred_info.mv[LIST_0][RSDj*4+RSDi].y))>>1 == 0))) 
							|| (((mb_field->pred_info.ref_idx[LIST_0][l_16_4[RSDj*4+RSDi]]) == -1) 
							&&  ((mb_field->pred_info.ref_idx[LIST_1][l_16_4[RSDj*4+RSDi]]) == 0) 
							&&  (fast_abs_short((mb_field->pred_info.mv[LIST_1][RSDj*4+RSDi].x))>>1 == 0) 
							&&  (fast_abs_short((mb_field->pred_info.mv[LIST_1][RSDj*4+RSDi].y))>>1 == 0)));
					}
				}
			}
			else
			{
				for(j=start_y;j<end_y;j+=2)
				{
					RSDj = RSD[j];
					for(i=start_x;i<end_x;i+=2)
					{
						RSDi = RSD[i];

						p->moving_block[j][i]= 
							!((!p->is_long_term 
							&& (((mb->pred_info.ref_idx[LIST_0][l_16_4[RSDj*4+RSDi]]) == 0) 
							&&  (fast_abs_short((mb->pred_info.mv[LIST_0][RSDj*4+RSDi].x))>>1 == 0) 
							&&  (fast_abs_short((mb->pred_info.mv[LIST_0][RSDj*4+RSDi].y))>>1 == 0))) 
							|| (((mb->pred_info.ref_idx[LIST_0][l_16_4[RSDj*4+RSDi]]) == -1) 
							&&  ((mb->pred_info.ref_idx[LIST_1][l_16_4[RSDj*4+RSDi]]) == 0) 
							&&  (fast_abs_short((mb->pred_info.mv[LIST_1][RSDj*4+RSDi].x))>>1 == 0) 
							&&  (fast_abs_short((mb->pred_info.mv[LIST_1][RSDj*4+RSDi].y))>>1 == 0)));
					}
				}
			}
		}
		else   //if (active_sps.direct_8x8_inference_flag)
		{   // frame_mbs_only_flag=1
			mb = &fs->mb_data[IMGPAR current_mb_nr_r];

			for (j=start_y;j<end_y;j++)
			{
				for (i=start_x;i<end_x;i++)
				{
					p->moving_block[j][i]= 
						!((!p->is_long_term 
						&& (((mb->pred_info.ref_idx[LIST_0][l_16_4[j*4+i]]) == 0) 
						&&  (fast_abs_short((mb->pred_info.mv[LIST_0][j*4+i].x))>>1 == 0) 
						&&  (fast_abs_short((mb->pred_info.mv[LIST_0][j*4+i].y))>>1 == 0))) 
						|| (((mb->pred_info.ref_idx[LIST_0][l_16_4[j*4+i]]) == -1) 
						&&  ((mb->pred_info.ref_idx[LIST_1][l_16_4[j*4+i]]) == 0) 
						&&  (fast_abs_short((mb->pred_info.mv[LIST_1][j*4+i].x))>>1 == 0) 
						&&  (fast_abs_short((mb->pred_info.mv[LIST_1][j*4+i].y))>>1 == 0)));
				}
			}
		}
	}
	else
	{   // currMB_r->mb_field==1 => frame_mbs_only_flag=0 => active_sps.direct_8x8_inference_flag=1
		StorablePicture *fs_field;

		if(IMGPAR current_mb_nr_r&1)
		{
			fs_field = fs_bottom;

			mb = &fs_field->mb_data[(IMGPAR current_mb_nr_r)>>1];

			for (j=start_y;j<end_y;j+=2)
			{
				jj = RSD[j];

				for (i=start_x;i<end_x;i+=2)
				{
					ii = RSD[i];

					p->moving_block[j][i] = 
						!((!fs_field->is_long_term 
						&& (((mb->pred_info.ref_idx[LIST_0][l_16_4[jj*4+ii]]) == 0) 
						&&  (fast_abs_short((mb->pred_info.mv[LIST_0][jj*4+ii].x))>>1 == 0) 
						&&  (fast_abs_short((mb->pred_info.mv[LIST_0][jj*4+ii].y))>>1 == 0))) 
						|| (((mb->pred_info.ref_idx[LIST_0][l_16_4[jj*4+ii]]) == -1) 
						&&  ((mb->pred_info.ref_idx[LIST_1][l_16_4[jj*4+ii]]) == 0) 
						&&  (fast_abs_short((mb->pred_info.mv[LIST_1][jj*4+ii].x))>>1 == 0) 
						&&  (fast_abs_short((mb->pred_info.mv[LIST_1][jj*4+ii].y))>>1 == 0)));
				}
			}
		}
		else
		{
			fs_field = fs_top;

			mb = &fs_field->mb_data[(IMGPAR current_mb_nr_r)>>1];

			for (j=start_y;j<end_y;j+=2)
			{
				jj = RSD[j];

				for (i=start_x;i<end_x;i+=2)
				{
					ii = RSD[i];

					p->moving_block[j][i] = 
						!((!fs_field->is_long_term 
						&& (((mb->pred_info.ref_idx[LIST_0][l_16_4[jj*4+ii]]) == 0) 
						&&  (fast_abs_short((mb->pred_info.mv[LIST_0][jj*4+ii].x))>>1 == 0) 
						&&  (fast_abs_short((mb->pred_info.mv[LIST_0][jj*4+ii].y))>>1 == 0))) 
						|| (((mb->pred_info.ref_idx[LIST_0][l_16_4[jj*4+ii]]) == -1) 
						&&  ((mb->pred_info.ref_idx[LIST_1][l_16_4[jj*4+ii]]) == 0) 
						&&  (fast_abs_short((mb->pred_info.mv[LIST_1][jj*4+ii].x))>>1 == 0) 
						&&  (fast_abs_short((mb->pred_info.mv[LIST_1][jj*4+ii].y))>>1 == 0)));
				}
			}
		}
	}
	p->is_long_term = fs->is_long_term;
}


void compute_colocated_SUBMB_frame_temporal PARGS6(ColocatedParamsMB* p, StorablePicture **list[6],int start_y, int start_x,
																									 int loop_y, int loop_x)
{
	StorablePicture *fs, *fs_top, *fs_bottom;
	int i,j;	
	int RSDi,RSDj;
	int end_y = start_y+loop_y;	
	int end_x = start_x+loop_x;
	Macroblock_s *mb;	
	Macroblock_s *mb_field;

	Pred_s_info *info = &p->pred_info;

	fs            = list[LIST_1][0];
	if (IMGPAR MbaffFrameFlag)
	{
		fs_top    = list[LIST_1 + 2][0];
		fs_bottom = list[LIST_1 + 4][0];
	}
	else
	{
		fs_top    = 
			fs_bottom = list[LIST_1][0];
	}

	if (!currMB_r->mb_field)
	{
		if (active_sps.direct_8x8_inference_flag)
		{   // frame_mbs_only_flag can be 0 or 1
			mb = &fs->mb_data[IMGPAR current_mb_nr_r];

			if(mb->mb_field)
			{
				int joff;

				Macroblock_s *mb_temp = &fs->mb_data[(IMGPAR current_mb_nr_r>>1)<<1];
				joff = (IMGPAR mb_y_r&1)<<1;

				//! Assign frame buffers for field MBs   
				//! Check whether we should use top or bottom field mvs.
				//! Depending on the assigned poc values.

				if (abs(dec_picture->poc - fs->bottom_field->poc)> abs(dec_picture->poc - fs->top_field->poc) )
				{
					if(IMGPAR MbaffFrameFlag)
					{
						mb_field = &fs->top_field->mb_data[IMGPAR current_mb_nr_r>>1];
					}
					else
					{
						mb_field = &fs->top_field->mb_data[(IMGPAR mb_y_r>>1)*IMGPAR PicWidthInMbs+IMGPAR mb_x_r];
						mb_temp  = &fs->mb_data[(IMGPAR mb_y_r>>1)*IMGPAR PicWidthInMbs*2+IMGPAR mb_x_r];
					}
					p->is_long_term = fs->top_field->is_long_term;
				}
				else
				{
					if(IMGPAR MbaffFrameFlag)
					{
						mb_field = &fs->bottom_field->mb_data[IMGPAR current_mb_nr_r>>1];
						mb_temp++;
					}
					else
					{
						mb_field = &fs->bottom_field->mb_data[(IMGPAR mb_y_r>>1)*IMGPAR PicWidthInMbs+IMGPAR mb_x_r];
						mb_temp  = &fs->mb_data[(IMGPAR mb_y_r>>1)*IMGPAR PicWidthInMbs*2+IMGPAR mb_x_r];
						mb_temp+=IMGPAR PicWidthInMbs;
					}
					p->is_long_term = fs->bottom_field->is_long_term;
				}

				for(j=start_y;j<end_y;j+=2) 
				{
					RSDj = (RSD[j]>>1)+joff;
					for (i=start_x;i<end_x;i+=2)
					{
						RSDi = RSD[i];

						info->mv[0][j*4+i].mv_comb = mb_field->pred_info.mv[LIST_0][RSDj*4+RSDi].mv_comb;
						info->mv[1][j*4+i].mv_comb = mb_field->pred_info.mv[LIST_1][RSDj*4+RSDi].mv_comb;

						info->ref_idx[0][l_16_4[j*4+i]] = mb_field->pred_info.ref_idx[LIST_0][l_16_4[RSDj*4+RSDi]];
						info->ref_idx[1][l_16_4[j*4+i]] = mb_field->pred_info.ref_idx[LIST_1][l_16_4[RSDj*4+RSDi]];

						info->ref_pic_id[0][l_16_4[j*4+i]] = mb_temp->pred_info.ref_pic_id[LIST_0][l_16_4[RSDj*4+RSDi]];
						info->ref_pic_id[1][l_16_4[j*4+i]] = mb_temp->pred_info.ref_pic_id[LIST_1][l_16_4[RSDj*4+RSDi]];

						info->mv[0][j*4+i].y <<= 1;
						info->mv[1][j*4+i].y <<= 1;
					}
				}
			}
			else
			{
				for(j=start_y;j<end_y;j+=2)
				{
					RSDj = RSD[j];
					for(i=start_x;i<end_x;i+=2)
					{
						RSDi = RSD[i];

						info->mv[0][j*4+i].mv_comb = mb->pred_info.mv[LIST_0][RSDj*4+RSDi].mv_comb;
						info->mv[1][j*4+i].mv_comb = mb->pred_info.mv[LIST_1][RSDj*4+RSDi].mv_comb;

						info->ref_idx[0][l_16_4[j*4+i]] = mb->pred_info.ref_idx[LIST_0][l_16_4[RSDj*4+RSDi]];
						info->ref_idx[1][l_16_4[j*4+i]] = mb->pred_info.ref_idx[LIST_1][l_16_4[RSDj*4+RSDi]];

						info->ref_pic_id[0][l_16_4[j*4+i]] = mb->pred_info.ref_pic_id[LIST_0][l_16_4[RSDj*4+RSDi]];
						info->ref_pic_id[1][l_16_4[j*4+i]] = mb->pred_info.ref_pic_id[LIST_1][l_16_4[RSDj*4+RSDi]];
					}
				}
			}
		}
		else   //if (active_sps.direct_8x8_inference_flag)
		{   // frame_mbs_only_flag=1
			mb = &fs->mb_data[IMGPAR current_mb_nr_r];

			for (j=start_y;j<end_y;j++)
			{
				for (i=start_x;i<end_x;i++)
				{
					info->mv[0][j*4+i].mv_comb = mb->pred_info.mv[LIST_0][j*4+i].mv_comb;
					info->mv[1][j*4+i].mv_comb = mb->pred_info.mv[LIST_1][j*4+i].mv_comb;

					info->ref_idx[0][l_16_4[j*4+i]] = mb->pred_info.ref_idx[LIST_0][l_16_4[j*4+i]];
					info->ref_idx[1][l_16_4[j*4+i]] = mb->pred_info.ref_idx[LIST_1][l_16_4[j*4+i]];

					info->ref_pic_id[0][l_16_4[j*4+i]] = mb->pred_info.ref_pic_id[LIST_0][l_16_4[j*4+i]];
					info->ref_pic_id[1][l_16_4[j*4+i]] = mb->pred_info.ref_pic_id[LIST_1][l_16_4[j*4+i]]; // ???? ATTENTION - CHANGED
				}
			}
		}
	}
	else
	{   // currMB_r->mb_field==1 => frame_mbs_only_flag=0 => active_sps.direct_8x8_inference_flag=1
		StorablePicture *fs_field;

		if(IMGPAR current_mb_nr_r&1)
		{
			fs_field = fs_bottom;

			mb = &fs_field->mb_data[(IMGPAR current_mb_nr_r)>>1];

			for (j=start_y;j<end_y;j+=2)
			{
				RSDj = RSD[j];

				for (i=start_x;i<end_x;i+=2)
				{
					RSDi = RSD[i];

					info->mv[0][j*4+i].mv_comb = mb->pred_info.mv[LIST_0][RSDj*4+RSDi].mv_comb;
					info->mv[1][j*4+i].mv_comb = mb->pred_info.mv[LIST_1][RSDj*4+RSDi].mv_comb;

					info->ref_idx[0][l_16_4[j*4+i]] = mb->pred_info.ref_idx[LIST_0][l_16_4[RSDj*4+RSDi]];
					info->ref_idx[1][l_16_4[j*4+i]] = mb->pred_info.ref_idx[LIST_1][l_16_4[RSDj*4+RSDi]];

					info->ref_pic_id[0][l_16_4[j*4+i]] = mb->pred_info.ref_pic_id[LIST_0][l_16_4[RSDj*4+RSDi]];
					info->ref_pic_id[1][l_16_4[j*4+i]] = mb->pred_info.ref_pic_id[LIST_1][l_16_4[RSDj*4+RSDi]];

					if (!mb->mb_field)
					{
						info->mv[0][j*4+i].y /= 2;
						info->mv[1][j*4+i].y /= 2;
					}
				}
			}
		}
		else
		{
			fs_field = fs_top;

			mb = &fs_field->mb_data[(IMGPAR current_mb_nr_r)>>1];

			for (j=start_y;j<end_y;j+=2)
			{
				RSDj = RSD[j];

				for (i=start_x;i<end_x;i+=2)
				{
					RSDi = RSD[i];

					info->mv[0][j*4+i].mv_comb = mb->pred_info.mv[LIST_0][RSDj*4+RSDi].mv_comb;
					info->mv[1][j*4+i].mv_comb = mb->pred_info.mv[LIST_1][RSDj*4+RSDi].mv_comb;

					info->ref_idx[0][l_16_4[j*4+i]] = mb->pred_info.ref_idx[LIST_0][l_16_4[RSDj*4+RSDi]];
					info->ref_idx[1][l_16_4[j*4+i]] = mb->pred_info.ref_idx[LIST_1][l_16_4[RSDj*4+RSDi]];

					info->ref_pic_id[0][l_16_4[j*4+i]] = mb->pred_info.ref_pic_id[LIST_0][l_16_4[RSDj*4+RSDi]];
					info->ref_pic_id[1][l_16_4[j*4+i]] = mb->pred_info.ref_pic_id[LIST_1][l_16_4[RSDj*4+RSDi]];

					if (!mb->mb_field)
					{
						info->mv[0][j*4+i].y /= 2;
						info->mv[1][j*4+i].y /= 2;
					}
				}
			}
		}
	}
	p->is_long_term = fs->is_long_term;
}


// Note: This function can only be called when (IMGPAR structure!=FRAME)
//       So, active_sps.frame_mbs_only_flag = 0 and (according to the spec), 
//       active_sps.direct_8x8_inference_flag = 1
void compute_colocated_SUBMB_field_spatial PARGS6(ColocatedParamsMB* p, StorablePicture **list[6], int start_y, int start_x, 
																									int loop_y, int loop_x)
{
	StorablePicture *fs;
	int i,j, ii, jj;
	int end_y = start_y+loop_y;	
	int end_x = start_x+loop_x;
	Macroblock_s *mb;

	fs = list[LIST_1 ][0];	

	if ((IMGPAR structure != fs->structure) && (fs->coded_frame))
	{
		if (IMGPAR structure==TOP_FIELD)
			fs = list[LIST_1 ][0]->top_field;
		else
			fs = list[LIST_1 ][0]->bottom_field;
	}

	if (fs->non_existing) {
		return;
	}
	mb = &fs->mb_data[IMGPAR current_mb_nr_r];
	for (j=start_y;j<end_y;j+=2)
	{
		jj = RSD[j];

		for (i=start_x;i<end_x;i+=2)
		{
			ii = RSD[i];

			p->moving_block[j][i]= 
				!((!p->is_long_term 
				&& (((mb->pred_info.ref_idx[LIST_0][l_16_4[jj*4+ii]]) == 0) 
				&&  (fast_abs_short((mb->pred_info.mv[LIST_0][jj*4+ii].x))>>1 == 0) 
				&&  (fast_abs_short((mb->pred_info.mv[LIST_0][jj*4+ii].y))>>1 == 0))) 
				|| (((mb->pred_info.ref_idx[LIST_0][l_16_4[jj*4+ii]]) == -1) 
				&&  ((mb->pred_info.ref_idx[LIST_1][l_16_4[jj*4+ii]]) == 0) 
				&&  (fast_abs_short((mb->pred_info.mv[LIST_1][jj*4+ii].x))>>1 == 0) 
				&&  (fast_abs_short((mb->pred_info.mv[LIST_1][jj*4+ii].y))>>1 == 0)));
		}
	}
	p->is_long_term = fs->is_long_term;
}


// Note: This function can only be called when (IMGPAR structure!=FRAME)
//       So, active_sps.frame_mbs_only_flag = 0 and (according to the spec), 
//       active_sps.direct_8x8_inference_flag = 1
void compute_colocated_SUBMB_field_temporal PARGS6(ColocatedParamsMB* p, StorablePicture **list[6], int start_y, int start_x, 
																									 int loop_y, int loop_x)
{
	StorablePicture *fs;
	int i,j, ii, jj;
	int end_y = start_y+loop_y;	
	int end_x = start_x+loop_x;
	Macroblock_s *mb;
	Pred_s_info *info = &p->pred_info;


	fs = list[LIST_1 ][0];	

	if ((IMGPAR structure != fs->structure) && (fs->coded_frame))
	{
		if (IMGPAR structure==TOP_FIELD)
			fs = list[LIST_1 ][0]->top_field;
		else
			fs = list[LIST_1 ][0]->bottom_field;
	}

	if (fs->non_existing) {
		return;
	}

	mb = &fs->mb_data[IMGPAR current_mb_nr_r];
	for (j=start_y;j<end_y;j+=2)
	{
		jj = RSD[j];

		for (i=start_x;i<end_x;i+=2)
		{
			ii = RSD[i];

			info->mv[0][j*4+i].mv_comb = mb->pred_info.mv[LIST_0][jj*4+ii].mv_comb;
			info->mv[1][j*4+i].mv_comb = mb->pred_info.mv[LIST_1][jj*4+ii].mv_comb;

			// Scaling of references is done here since it will not affect spatial direct (2*0 =0)			
			info->ref_idx[0][l_16_4[j*4+i]] = mb->pred_info.ref_idx[LIST_0][l_16_4[jj*4+ii]];
			info->ref_pic_id[0][l_16_4[j*4+i]] = mb->pred_info.ref_pic_id[LIST_0][l_16_4[jj*4+ii]];		

			info->ref_idx[1][l_16_4[j*4+i]] = mb->pred_info.ref_idx[LIST_1][l_16_4[jj*4+ii]];
			info->ref_pic_id[1][l_16_4[j*4+i]] = mb->pred_info.ref_pic_id[LIST_1][l_16_4[jj*4+ii]];

			if (!mb->mb_field)
			{
				info->mv[0][j*4+i].y /= 2;
				info->mv[1][j*4+i].y /= 2;
			}
		}
	}
	p->is_long_term = fs->is_long_term;
}


void calc_mvscale PARGS0()
{
	int i, j;
	for (j=0; j<2 + (IMGPAR MbaffFrameFlag<<2);j+=2)
	{
		for (i=0; i<listXsize[j];i++)
		{
			int prescale, iTRb, iTRp;

			if (j==0)
				iTRb = Clip3( -128, 127, dec_picture->poc - listX[LIST_0 + j][i]->poc );

			else if (j == 2)
				iTRb = Clip3( -128, 127, dec_picture->top_poc - listX[LIST_0 + j][i]->poc );

			else
				iTRb = Clip3( -128, 127, dec_picture->bottom_poc - listX[LIST_0 + j][i]->poc );


			iTRp = Clip3( -128, 127,  listX[LIST_1 + j][0]->poc - listX[LIST_0 + j][i]->poc);

			if (iTRp!=0)
			{
				prescale = ( 16384 + abs( iTRp >>1 ) ) / iTRp;
				IMGPAR mvscale[j][i] = Clip3( -1024, 1023, ( iTRb * prescale + 32 ) >> 6 ) ;
			}
			else
			{
				IMGPAR mvscale[j][i] = 9999;
			}
		}
	}
}
