
/*!
************************************************************************
*  \file
*     parset.c
*  \brief
*     Parameter Sets
*  \author
*     Main contributors (see contributors.h for copyright, address and affiliation details)
*     - Stephan Wenger          <stewe@cs.tu-berlin.de>
*
***********************************************************************
*/

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <malloc.h>

#include "global.h"
#include "parsetcommon.h"
#include "parset.h"
#include "nalu.h"
#include "memalloc.h"
#include "fmo.h"
#include "cabac.h"
#include "vlc.h"
#include "mbuffer.h"
#if defined(_HW_ACCEL_)
#include "h264dxvabase.h"
#endif

const byte ZZ_SCAN[16]  =
{  0,  1,  4,  8,  5,  2,  3,  6,  9, 12, 13, 10,  7, 11, 14, 15
};

const byte ZZ_SCAN8[64] =
{  0,  1,  8, 16,  9,  2,  3, 10, 17, 24, 32, 25, 18, 11,  4,  5,
12, 19, 26, 33, 40, 48, 41, 34, 27, 20, 13,  6,  7, 14, 21, 28,
35, 42, 49, 56, 57, 50, 43, 36, 29, 22, 15, 23, 30, 37, 44, 51,
58, 59, 52, 45, 38, 31, 39, 46, 53, 60, 61, 54, 47, 55, 62, 63
};



extern void init_frext PARGS0();

#ifdef _COLLECT_PIC_
#define		stream_global		IMGPAR stream_global
#endif

// syntax for scaling list matrix values
void Scaling_List PARGS3(UCHAR *scalingList, int sizeOfScalingList, BOOL *UseDefaultScalingMatrix)
{
	int j, scanj;
	int delta_scale, lastScale, nextScale;

	lastScale      = 8;
	nextScale      = 8;

	for(j=0; j<sizeOfScalingList; j++)
	{
		scanj = (sizeOfScalingList==16) ? ZZ_SCAN[j]:ZZ_SCAN8[j];

		if(nextScale!=0)
		{
			delta_scale = se_v (   "   : delta_sl   "   );
			nextScale = (lastScale + delta_scale + 256) % 256;
			*UseDefaultScalingMatrix = (scanj==0 && nextScale==0);
		}

		scalingList[scanj] = (nextScale==0) ? lastScale:nextScale;
		lastScale = scalingList[scanj];
	}
}
// fill sps with content of p

CREL_RETURN InterpretSPS PARGS1(seq_parameter_set_rbsp_t *sps)
{
	unsigned i;
	int reserved_zero, curr_sps_width, curr_sps_height;
	CREL_RETURN ret = CREL_OK;
	/*
	assert (&(IMGPAR g_dep) != NULL);
	assert (IMGPAR g_dep.Dcodestrm != 0);
	assert (sps != NULL);
	*/
	if ((&(IMGPAR g_dep) == NULL)||(IMGPAR g_dep.Dcodestrm == NULL)||(sps == NULL)) {
		return CREL_ERROR_H264_NOMEMORY;
	}

	sps->profile_idc                            = u_v (8, "SPS: profile_idc");

	sps->constrained_set0_flag                  = u_1 (   "SPS: constrained_set0_flag");
	sps->constrained_set1_flag                  = u_1 (   "SPS: constrained_set1_flag");
	sps->constrained_set2_flag                  = u_1 (   "SPS: constrained_set2_flag");
	sps->constrained_set3_flag                  = u_1 (   "SPS: constrained_set3_flag");
	sps->constrained_set4_flag                  = u_1 (   "SPS: constrained_set4_flag");
	reserved_zero                               = u_v (3, "SPS: reserved_zero_4bits");
	//assert (reserved_zero==0);
	if (reserved_zero) {
		//return CREL_WARNING_H264_STREAMERROR_LEVEL_1|CREL_WARNING_H264_ERROR_SPS;
	}

	sps->level_idc                              = u_v (8, "SPS: level_idc");

	sps->seq_parameter_set_id                   = ue_v ("SPS: seq_parameter_set_id");

	if (sps->seq_parameter_set_id > MAXSPS - 1) {
		return CREL_WARNING_H264_STREAMERROR_LEVEL_1|CREL_WARNING_H264_ERROR_SPS;
	}

	// Fidelity Range Extensions stuff
	sps->chroma_format_idc = 1;
	sps->bit_depth_luma_minus8   = 0;
	sps->bit_depth_chroma_minus8 = 0;
	IMGPAR lossless_qpprime_flag   = 0;
	sps->seq_scaling_matrix_present_flag = 0;

	// Residue Color Transform
	IMGPAR residue_transform_flag = 0;

	if((sps->profile_idc==FREXT_HP   ) ||
		(sps->profile_idc==FREXT_Hi10P) ||
		(sps->profile_idc==FREXT_Hi422) ||
		(sps->profile_idc==FREXT_Hi444) || (sps->profile_idc==118) || (sps->profile_idc==128))
	{
		sps->chroma_format_idc                      = ue_v ("SPS: chroma_format_idc");
		// IoK: This is constrained to be either 0 or 1 for HD-DVD streams
		if(sps->chroma_format_idc !=YUV400 && sps->chroma_format_idc!=YUV420)
		{
			//g_HDProfileFault = HD_ProfileFault_SPS_chroma_format_idc;
			return CREL_WARNING_H264_STREAMERROR_LEVEL_3|CREL_WARNING_H264_HD_ProfileFault;
		}

		// Residue Color Transform
		if(sps->chroma_format_idc == 3)
			IMGPAR residue_transform_flag = u_1 ("SPS: residue_transform_flag");

		sps->bit_depth_luma_minus8                  = ue_v ("SPS: bit_depth_luma_minus8");
		// IoK: This is constrained to be 0 for HD-DVD streams

		if(sps->bit_depth_luma_minus8)
		{
			//g_HDProfileFault = HD_ProfileFault_SPS_bit_depth_luma_minus8;
			return CREL_WARNING_H264_STREAMERROR_LEVEL_3|CREL_WARNING_H264_HD_ProfileFault;
		}
		sps->bit_depth_chroma_minus8                = ue_v ("SPS: bit_depth_chroma_minus8");
		// IoK: This is constrained to be 0 for HD-DVD streams
		if(sps->bit_depth_chroma_minus8)
		{
			//g_HDProfileFault = HD_ProfileFault_SPS_bit_depth_chroma_minus8;
			return CREL_WARNING_H264_STREAMERROR_LEVEL_3|CREL_WARNING_H264_HD_ProfileFault;
		}
		IMGPAR lossless_qpprime_flag                  = u_1 ("SPS: lossless_qpprime_y_zero_flag");
		// IoK: This is constrained to be 0 for HD-DVD streams
		if(IMGPAR lossless_qpprime_flag)
		{
			//g_HDProfileFault = HD_ProfileFault_SPS_qpprime_y_zero_transform_bypass_flag;
			return CREL_WARNING_H264_STREAMERROR_LEVEL_3|CREL_WARNING_H264_HD_ProfileFault;
		}

		sps->seq_scaling_matrix_present_flag        = u_1 (   "SPS: seq_scaling_matrix_present_flag");

		if(sps->seq_scaling_matrix_present_flag)
		{
			for(i=0; i<8; i++)
			{
				sps->seq_scaling_list_present_flag[i]   = u_1 (   "SPS: seq_scaling_list_present_flag");
				if(sps->seq_scaling_list_present_flag[i])
				{
					if(i<6)
						Scaling_List ARGS3(sps->ScalingList4x4[i], 16, &sps->UseDefaultScalingMatrix4x4Flag[i]);
					else
						Scaling_List ARGS3(sps->ScalingList8x8[i-6], 64, &sps->UseDefaultScalingMatrix8x8Flag[i-6]);
				}
			}
		}
	}

	sps->log2_max_frame_num_minus4              = ue_v ("SPS: log2_max_frame_num_minus4");
	sps->pic_order_cnt_type                     = ue_v ("SPS: pic_order_cnt_type");

	//init.
	sps->log2_max_pic_order_cnt_lsb_minus4 = 0;
	sps->delta_pic_order_always_zero_flag  = 0;

	if (sps->pic_order_cnt_type == 0)
		sps->log2_max_pic_order_cnt_lsb_minus4 = ue_v ("SPS: log2_max_pic_order_cnt_lsb_minus4");
	else if (sps->pic_order_cnt_type == 1)
	{
		sps->delta_pic_order_always_zero_flag      = u_1 ("SPS: delta_pic_order_always_zero_flag");
		sps->offset_for_non_ref_pic                = se_v ("SPS: offset_for_non_ref_pic");
		sps->offset_for_top_to_bottom_field        = se_v ("SPS: offset_for_top_to_bottom_field");
		sps->num_ref_frames_in_pic_order_cnt_cycle = ue_v ("SPS: num_ref_frames_in_pic_order_cnt_cycle");
		for(i=0; i<sps->num_ref_frames_in_pic_order_cnt_cycle; i++)
			sps->offset_for_ref_frame[i]               = se_v ("SPS: offset_for_ref_frame[i]");
	}
	sps->num_ref_frames                        = ue_v ("SPS: num_ref_frames");
	sps->gaps_in_frame_num_value_allowed_flag  = u_1 ("SPS: gaps_in_frame_num_value_allowed_flag");
	sps->pic_width_in_mbs_minus1               = ue_v ("SPS: pic_width_in_mbs_minus1");
	sps->pic_height_in_map_units_minus1        = ue_v ("SPS: pic_height_in_map_units_minus1");
	sps->frame_mbs_only_flag                   = u_1 ("SPS: frame_mbs_only_flag");
	curr_sps_height = sps->frame_mbs_only_flag ? sps->pic_height_in_map_units_minus1 : ((sps->pic_height_in_map_units_minus1<<1) +1);
	curr_sps_width = (int)sps->pic_width_in_mbs_minus1;

	if (g_pH264DXVA && ((curr_sps_width <= 3) || (curr_sps_height <= 2))) {
		//In DXVA mode, Send_Resolution will crash when resolution is too small.
		return CREL_WARNING_H264_STREAMERROR_LEVEL_1|CREL_WARNING_H264_ERROR_SPS;
	}

	if (g_pH264DXVA && ((curr_sps_width > 119) || (curr_sps_height > 67))) {
		//In DXVA mode, Send_Resolution will crash when resolution is too large. Set maximum limit as HD
		return CREL_WARNING_H264_STREAMERROR_LEVEL_1|CREL_WARNING_H264_ERROR_SPS;
	}	

	if(stream_global->pre_sps_height != curr_sps_height || stream_global->pre_sps_width != curr_sps_width)
	{
#if defined(_HW_ACCEL_)
		if (g_pH264DXVA)
			((CH264DXVABase*)g_pH264DXVA)->m_bResolutionChange = TRUE;
#endif
		stream_global->pre_sps_height = curr_sps_height;
		stream_global->pre_sps_width = curr_sps_width;
	}
	if (!sps->frame_mbs_only_flag)
	{
		sps->mb_adaptive_frame_field_flag          = u_1 ("SPS: mb_adaptive_frame_field_flag");
	}
	else
	{
		sps->mb_adaptive_frame_field_flag          = 0; // Later code assumes this value to be 0
	}
	sps->direct_8x8_inference_flag             = u_1 ("SPS: direct_8x8_inference_flag");
	// IoK: This is constrained to be 1 when sps->frame_mbs_only_flag == 0
	if(sps->frame_mbs_only_flag == 0 && sps->direct_8x8_inference_flag==0)
	{
		//g_HDProfileFault = HD_ProfileFault_SPS_direct_8x8_stream_violation;
		return CREL_WARNING_H264_STREAMERROR_LEVEL_3|CREL_WARNING_H264_HD_ProfileFault;
	}
	sps->frame_cropping_flag                   = u_1 ("SPS: frame_cropping_flag");

	if (sps->frame_cropping_flag)
	{
		sps->frame_cropping_rect_left_offset      = ue_v ("SPS: frame_cropping_rect_left_offset");
		sps->frame_cropping_rect_right_offset     = ue_v ("SPS: frame_cropping_rect_right_offset");
		sps->frame_cropping_rect_top_offset       = ue_v ("SPS: frame_cropping_rect_top_offset");
		sps->frame_cropping_rect_bottom_offset    = ue_v ("SPS: frame_cropping_rect_bottom_offset");

		//FIXME: do we need to enable or disable this? Now we have bugs: (a)if enable frame_cropping_flag, DS filter will crash. (b) if disable frame_cropping_flag, DXVA will have green line in bottom of output frame.
		//#if defined(_HW_ACCEL_)
		//	if(g_DXVAVer)
		//		sps->frame_cropping_flag = 0;
		//#endif

	}
	sps->vui_parameters_present_flag           = u_1 ("SPS: vui_parameters_present_flag");

	InitVUI(sps);
	ret = ReadVUI ARGS1(sps);

	if (SUCCEEDED(ret)) {
		sps->Valid = TRUE;
	}  

	return ret;
}


CREL_RETURN InterpretSPSSubsetExt PARGS1(seq_parameter_set_rbsp_t *sps)
{
	unsigned i, j, k;
	int temp;
	CREL_RETURN ret = CREL_OK;
	
	if ((&(IMGPAR g_dep) == NULL)||(IMGPAR g_dep.Dcodestrm == NULL)||(sps == NULL)) {
		return CREL_ERROR_H264_NOMEMORY;
	}

	if ((sps->profile_idc == 118) || (sps->profile_idc == 128)){		//MVC HIGH PROFILE

		temp   = u_1 (   "SPS MVC EXT: constrained_set0_flag");

		if (temp == 0) {
			return CREL_WARNING_H264_STREAMERROR_LEVEL_1|CREL_WARNING_H264_ERROR_SPS;
		}

		

		stream_global->num_views = sps->num_views = ue_v ("SPS MVC EXT: num_views_minus1") + 1;		

		if (sps->num_views < 2 || sps->num_views > MAX_NUM_VIEWS) {
			return CREL_WARNING_H264_STREAMERROR_LEVEL_1|CREL_WARNING_H264_ERROR_SPS;
		}

		sps->view_id = (unsigned *) _aligned_malloc(sps->num_views * sizeof(unsigned), 16);
		sps->num_anchor_refs_l0 = (unsigned *) _aligned_malloc(sps->num_views * sizeof(unsigned), 16);
		sps->num_anchor_refs_l1 = (unsigned *) _aligned_malloc(sps->num_views * sizeof(unsigned), 16);

		sps->num_non_anchor_refs_l0 = (unsigned *) _aligned_malloc(sps->num_views * sizeof(unsigned), 16);
		sps->num_non_anchor_refs_l1 = (unsigned *) _aligned_malloc(sps->num_views * sizeof(unsigned), 16);

		sps->anchor_refs_l0 = (unsigned **) _aligned_malloc((sps->num_views)*sizeof(unsigned *), 16);
		sps->anchor_refs_l1 = (unsigned **) _aligned_malloc((sps->num_views)*sizeof(unsigned *), 16);

		sps->non_anchor_refs_l0 = (unsigned **) _aligned_malloc((sps->num_views)*sizeof(unsigned *), 16);
		sps->non_anchor_refs_l1 = (unsigned **) _aligned_malloc((sps->num_views)*sizeof(unsigned *), 16);


		for (i = 0; i < sps->num_views; i++) {

			sps->anchor_refs_l0[i] = (unsigned *)_aligned_malloc((sps->num_views) * sizeof(unsigned), 16);
			sps->anchor_refs_l1[i] = (unsigned *)_aligned_malloc((sps->num_views) * sizeof(unsigned), 16);

			sps->non_anchor_refs_l0[i] = (unsigned *)_aligned_malloc((sps->num_views) * sizeof(unsigned), 16);
			sps->non_anchor_refs_l1[i] = (unsigned *)_aligned_malloc((sps->num_views) * sizeof(unsigned), 16);			

		}

		//Parse view_id for each available view
		for( i=0; i<sps->num_views; i++) {
			sps->view_id[i] = ue_v("SPS MVC EXT: view_id[]");			
		}

		for( i=1; i<sps->num_views; i++) {

			//Parse the default anchor L0 inter-view reference for each view
			sps->num_anchor_refs_l0[i] = ue_v("SPS MVC EXT: num_anchor_refs_l0[]");

			if (sps->num_anchor_refs_l0[i] > (MAX_NUM_VIEWS-1)) {
				return CREL_WARNING_H264_STREAMERROR_LEVEL_1|CREL_WARNING_H264_ERROR_SPS;
			}

			for( j=0; j<sps->num_anchor_refs_l0[i]; j++) {

				sps->anchor_refs_l0[i][j] = ue_v("SPS MVC EXT: anchor_refs_l0[][]");

				if (sps->anchor_refs_l0[i][j] > (MAX_NUM_VIEWS-1)) {
					return CREL_WARNING_H264_STREAMERROR_LEVEL_1|CREL_WARNING_H264_ERROR_SPS;
				}
				

			}

			//Parse the default anchor L1 inter-view reference for each view
			sps->num_anchor_refs_l1[i] = ue_v("SPS MVC EXT: num_anchor_refs_l1[]");
			
			if (sps->num_anchor_refs_l1[i] > (MAX_NUM_VIEWS-1)) {
				return CREL_WARNING_H264_STREAMERROR_LEVEL_1|CREL_WARNING_H264_ERROR_SPS;
			}


			for( j=0; j<sps->num_anchor_refs_l1[i]; j++) {

				sps->anchor_refs_l1[i][j] = ue_v("SPS MVC EXT: anchor_refs_l1[][]");	

				if (sps->anchor_refs_l1[i][j] > (MAX_NUM_VIEWS-1)) {
					return CREL_WARNING_H264_STREAMERROR_LEVEL_1|CREL_WARNING_H264_ERROR_SPS;
				}

			}
			
		}


		for( i=1; i<sps->num_views; i++) {

			//Parse the default non-anchor L0 inter-view reference for each view
			sps->num_non_anchor_refs_l0[i] = ue_v("SPS MVC EXT: num_non_anchor_refs_l0[]");

			if (sps->num_non_anchor_refs_l0[i] > (MAX_NUM_VIEWS-1)) {
				return CREL_WARNING_H264_STREAMERROR_LEVEL_1|CREL_WARNING_H264_ERROR_SPS;
			}

			for( j=0; j<sps->num_non_anchor_refs_l0[i]; j++) {
				
				sps->non_anchor_refs_l0[i][j] = ue_v("SPS MVC EXT: non_anchor_refs_l0[][]");

				if (sps->non_anchor_refs_l0[i][j] > (MAX_NUM_VIEWS-1)) {
					return CREL_WARNING_H264_STREAMERROR_LEVEL_1|CREL_WARNING_H264_ERROR_SPS;
				}
			}

			//Parse the default non-anchor L1 inter-view reference for each view
			sps->num_non_anchor_refs_l1[i] = ue_v("SPS MVC EXT: num_non_anchor_refs_l1[]");

			if (sps->num_non_anchor_refs_l1[i] > (MAX_NUM_VIEWS-1)) {
				return CREL_WARNING_H264_STREAMERROR_LEVEL_1|CREL_WARNING_H264_ERROR_SPS;
			}

			for( j=0; j<sps->num_non_anchor_refs_l1[i]; j++) {
				
				sps->non_anchor_refs_l1[i][j] = ue_v("SPS MVC EXT: non_anchor_refs_l1[][]");

				if (sps->non_anchor_refs_l1[i][j] > (MAX_NUM_VIEWS-1)) {
					return CREL_WARNING_H264_STREAMERROR_LEVEL_1|CREL_WARNING_H264_ERROR_SPS;
				}
			}

		}



		sps->num_level_values_signalled = ue_v ("SPS MVC EXT: num_level_values_signalled_minus1") + 1;		
		
		if (sps->num_level_values_signalled > MAX_NUM_VIEWS) {
			return CREL_WARNING_H264_STREAMERROR_LEVEL_1|CREL_WARNING_H264_ERROR_SPS;
		}
		

		/* Allocate memory */
		sps->num_applicable_ops = (unsigned *) _aligned_malloc(sps->num_level_values_signalled * sizeof(unsigned), 16);
		sps->level_idc_mvc = (unsigned *) _aligned_malloc(sps->num_level_values_signalled * sizeof(unsigned), 16);
		sps->applicable_op_temporal_id = (unsigned **) _aligned_malloc(sps->num_level_values_signalled* sizeof(unsigned *), 16);
		sps->applicable_op_num_target_views_minus1 = (unsigned **) _aligned_malloc(sps->num_level_values_signalled* sizeof(unsigned *), 16);
		sps->applicable_op_target_view_id = (unsigned ***) _aligned_malloc(sps->num_level_values_signalled* sizeof(unsigned **), 16);
		sps->applicable_op_num_views_minus1 = (unsigned **) _aligned_malloc(sps->num_level_values_signalled* sizeof(unsigned *), 16);


		for( i=0; i<sps->num_level_values_signalled; i++) {
			

			sps->level_idc_mvc[i] = u_v(8, "SPS MVC EXT:  level_idc_mvc");

			sps->num_applicable_ops[i] = ue_v("SPS MVC EXT:  num_applicable_ops_minus1") + 1;
			if (sps->num_applicable_ops[i] > MAX_NUM_APPLICABLE_OPS) {
				return CREL_WARNING_H264_STREAMERROR_LEVEL_1|CREL_WARNING_H264_ERROR_SPS;
			}			
			

			/* Allocate memory */
			sps->applicable_op_temporal_id[i] = (unsigned *) _aligned_malloc(sps->num_applicable_ops[i] * sizeof(unsigned), 16);
			sps->applicable_op_num_target_views_minus1[i] = (unsigned *) _aligned_malloc((sps->num_applicable_ops[i]) * sizeof(unsigned), 16);
			sps->applicable_op_target_view_id[i] = (unsigned **) _aligned_malloc(sps->num_applicable_ops[i] * sizeof(unsigned *), 16);
			sps->applicable_op_num_views_minus1[i] = (unsigned *) _aligned_malloc(sps->num_applicable_ops[i] * sizeof(unsigned), 16);

			for( j=0; j<sps->num_applicable_ops[i]; j++) {
				
				sps->applicable_op_temporal_id[i][j] = u_v(3, "SPS MVC EXT:  applicable_op_temporal_id[][]");

				sps->applicable_op_num_target_views_minus1[i][j] = ue_v("SPS MVC EXT:  applicable_op_num_target_views_minus1[][]");
				if (sps->applicable_op_num_target_views_minus1[i][j] > (MAX_NUM_VIEWS-1)) {
					return CREL_WARNING_H264_STREAMERROR_LEVEL_1|CREL_WARNING_H264_ERROR_SPS;
				}

				
				/* Allocate memory */
				sps->applicable_op_target_view_id[i][j] = (unsigned *) _aligned_malloc((sps->applicable_op_num_target_views_minus1[i][j] + 1) * sizeof(unsigned), 16);

				for( k=0; k<=sps->applicable_op_num_target_views_minus1[i][j]; k++) {

					sps->applicable_op_target_view_id[i][j][k] = ue_v("SPS MVC EXT:  applicable_op_target_view_id[][][]");				

				}

				sps->applicable_op_num_views_minus1[i][j] = ue_v("applicable_op_num_views_minus1[i][j]");				
			}
		}
		

		sps->mvc_vui_parameters_present_flag = u_1 (   "SPS MVC EXT: mvc_vui_parameters_present_flag");

		if (sps->mvc_vui_parameters_present_flag) {
			ReadVUIExtension ARGS1(sps);
		}

		stream_global->bMVC = TRUE;

	}


	/* Additional extension flag */
	temp = u_1("additional_extension2_flag");

	if (temp) {
		//Will be added laterly
		return CREL_WARNING_H264_STREAMERROR_LEVEL_1|CREL_ERROR_H264_NOTSUPPORTED;
	}

	
	if (SUCCEEDED(ret)) {
		sps->Valid = TRUE;
	}  

	return ret;
}


void InitVUI(seq_parameter_set_rbsp_t *sps)
{
	sps->vui_seq_parameters.matrix_coefficients = 2;
	sps->vui_seq_parameters.NumClockTs = 0;
}


CREL_RETURN ReadVUI PARGS1(seq_parameter_set_rbsp_t *sps)
{
	CREL_RETURN ret;

	if (sps->vui_parameters_present_flag)
	{
		sps->vui_seq_parameters.aspect_ratio_info_present_flag = u_1 ("VUI: aspect_ratio_info_present_flag");
		if (sps->vui_seq_parameters.aspect_ratio_info_present_flag)
		{
			sps->vui_seq_parameters.aspect_ratio_idc             = u_v ( 8, "VUI: aspect_ratio_idc");
			if (255==sps->vui_seq_parameters.aspect_ratio_idc)
			{
				sps->vui_seq_parameters.sar_width                  = u_v (16, "VUI: sar_width");
				sps->vui_seq_parameters.sar_height                 = u_v (16, "VUI: sar_height");
			}
		}

		sps->vui_seq_parameters.overscan_info_present_flag     = u_1 ("VUI: overscan_info_present_flag");
		if (sps->vui_seq_parameters.overscan_info_present_flag)
		{
			sps->vui_seq_parameters.overscan_appropriate_flag    = u_1 ("VUI: overscan_appropriate_flag");
		}

		sps->vui_seq_parameters.video_signal_type_present_flag = u_1 ("VUI: video_signal_type_present_flag");
		if (sps->vui_seq_parameters.video_signal_type_present_flag)
		{
			sps->vui_seq_parameters.video_format                    = u_v ( 3,"VUI: video_format");
			sps->vui_seq_parameters.video_full_range_flag           = u_1 (   "VUI: video_full_range_flag");
			sps->vui_seq_parameters.colour_description_present_flag = u_1 (   "VUI: color_description_present_flag");
			if(sps->vui_seq_parameters.colour_description_present_flag)
			{
				sps->vui_seq_parameters.colour_primaries              = u_v ( 8,"VUI: colour_primaries");
				sps->vui_seq_parameters.transfer_characteristics      = u_v ( 8,"VUI: transfer_characteristics");
				sps->vui_seq_parameters.matrix_coefficients           = u_v ( 8,"VUI: matrix_coefficients");
			}
		}
		sps->vui_seq_parameters.chroma_location_info_present_flag = u_1 (   "VUI: chroma_loc_info_present_flag");
		if(sps->vui_seq_parameters.chroma_location_info_present_flag)
		{
			sps->vui_seq_parameters.chroma_sample_loc_type_top_field     = ue_v ( "VUI: chroma_sample_loc_type_top_field");
			sps->vui_seq_parameters.chroma_sample_loc_type_bottom_field  = ue_v ( "VUI: chroma_sample_loc_type_bottom_field");
		}
		sps->vui_seq_parameters.timing_info_present_flag          = u_1 ("VUI: timing_info_present_flag");
		if (sps->vui_seq_parameters.timing_info_present_flag)
		{
			sps->vui_seq_parameters.num_units_in_tick               = u_v (32,"VUI: num_units_in_tick");
			sps->vui_seq_parameters.time_scale                      = u_v (32,"VUI: time_scale");
			sps->vui_seq_parameters.fixed_frame_rate_flag           = u_1 (   "VUI: fixed_frame_rate_flag");
			//Terry: get the initial frame rate before refering field_pic_flag.
			g_framerate1000 = (unsigned long)((1000*(__int64)sps->vui_seq_parameters.time_scale)/(__int64)sps->vui_seq_parameters.num_units_in_tick);

			//YK: temporal solution for OEM case, 2005/11/21
			if (g_framerate1000 > 30000)
				g_framerate1000 /= 2;
			if (g_framerate1000 < 23000)
				g_framerate1000 = 23970;
		}
		else//Terry: we set the default frame rate is 29.97 fps, if the timing_info_present_flag is disable.
			g_framerate1000 = 29970;

		sps->vui_seq_parameters.nal_hrd_parameters_present_flag   = u_1 ("VUI: nal_hrd_parameters_present_flag");
		if (sps->vui_seq_parameters.nal_hrd_parameters_present_flag)
		{
			ret = ReadHRDParameters ARGS1(&(sps->vui_seq_parameters.nal_hrd_parameters));
			if (FAILED(ret)) {
				sps->vui_seq_parameters.nal_hrd_parameters_present_flag = 0;
				return ret;

			}
		}
		sps->vui_seq_parameters.vcl_hrd_parameters_present_flag   = u_1 ("VUI: vcl_hrd_parameters_present_flag");
		if (sps->vui_seq_parameters.vcl_hrd_parameters_present_flag)
		{
			ret =ReadHRDParameters ARGS1(&(sps->vui_seq_parameters.vcl_hrd_parameters));
			if (FAILED(ret)) {
				sps->vui_seq_parameters.vcl_hrd_parameters_present_flag = 0;
				return ret;

			}
		}
		if (sps->vui_seq_parameters.nal_hrd_parameters_present_flag || sps->vui_seq_parameters.vcl_hrd_parameters_present_flag)
		{
			sps->vui_seq_parameters.low_delay_hrd_flag             =  u_1 ("VUI: low_delay_hrd_flag");
		}
		sps->vui_seq_parameters.picture_structure = 0;
		sps->vui_seq_parameters.pic_struct_present_flag          =  u_1 ("VUI: pic_struct_present_flag   ");
		sps->vui_seq_parameters.bitstream_restriction_flag       =  u_1 ("VUI: bitstream_restriction_flag");
		if (sps->vui_seq_parameters.bitstream_restriction_flag)
		{
			sps->vui_seq_parameters.motion_vectors_over_pic_boundaries_flag =  u_1 ("VUI: motion_vectors_over_pic_boundaries_flag");
			sps->vui_seq_parameters.max_bytes_per_pic_denom                 =  ue_v ("VUI: max_bytes_per_pic_denom");
			sps->vui_seq_parameters.max_bits_per_mb_denom                   =  ue_v ("VUI: max_bits_per_mb_denom");
			sps->vui_seq_parameters.log2_max_mv_length_horizontal           =  ue_v ("VUI: log2_max_mv_length_horizontal");
			sps->vui_seq_parameters.log2_max_mv_length_vertical             =  ue_v ("VUI: log2_max_mv_length_vertical");
			sps->vui_seq_parameters.num_reorder_frames                      =  ue_v ("VUI: num_reorder_frames");
			sps->vui_seq_parameters.max_dec_frame_buffering                 =  ue_v ("VUI: max_dec_frame_buffering");
		}
	}

	return CREL_OK;
}

CREL_RETURN ReadVUIExtension PARGS1(seq_parameter_set_rbsp_t *sps)
{
	if(sps->mvc_vui_parameters_present_flag)
	{
		sps->vui_parameters_ext.vui_mvc_num_ops = ue_v("VUI: vui_mvc_num_ops_minus1") + 1;

		if(sps->vui_parameters_ext.vui_mvc_num_ops > 0)
		{
			int num = sps->vui_parameters_ext.vui_mvc_num_ops;

			if(sps->vui_parameters_ext.vui_mvc_temporal_id)
				free(sps->vui_parameters_ext.vui_mvc_temporal_id);
			sps->vui_parameters_ext.vui_mvc_temporal_id = (unsigned*)malloc(num * sizeof(unsigned));
			memset(sps->vui_parameters_ext.vui_mvc_temporal_id, 0, num * sizeof(unsigned));

			if(sps->vui_parameters_ext.vui_mvc_num_target_output_views)
				free(sps->vui_parameters_ext.vui_mvc_num_target_output_views);
			sps->vui_parameters_ext.vui_mvc_num_target_output_views = (unsigned*)malloc(num * sizeof(unsigned));
			memset(sps->vui_parameters_ext.vui_mvc_num_target_output_views, 0, num * sizeof(unsigned));

			if(sps->vui_parameters_ext.vui_mvc_view_id)
				free(sps->vui_parameters_ext.vui_mvc_view_id);
			sps->vui_parameters_ext.vui_mvc_view_id = (unsigned**)malloc(num * sizeof(unsigned*));
			memset(sps->vui_parameters_ext.vui_mvc_view_id, 0, num * sizeof(unsigned*));

			if(sps->vui_parameters_ext.vui_mvc_timing_info_present_flag)
				free(sps->vui_parameters_ext.vui_mvc_timing_info_present_flag);
			sps->vui_parameters_ext.vui_mvc_timing_info_present_flag = (BOOL*)malloc(num * sizeof(BOOL));
			memset(sps->vui_parameters_ext.vui_mvc_timing_info_present_flag, 0, num * sizeof(BOOL));

			if(sps->vui_parameters_ext.vui_mvc_num_units_in_tick)
				free(sps->vui_parameters_ext.vui_mvc_num_units_in_tick);
			sps->vui_parameters_ext.vui_mvc_num_units_in_tick = (unsigned*)malloc(num * sizeof(unsigned));
			memset(sps->vui_parameters_ext.vui_mvc_num_units_in_tick, 0, num * sizeof(unsigned));

			if(sps->vui_parameters_ext.vui_mvc_time_scale)
				free(sps->vui_parameters_ext.vui_mvc_time_scale);
			sps->vui_parameters_ext.vui_mvc_time_scale = (unsigned*)malloc(num * sizeof(unsigned));
			memset(sps->vui_parameters_ext.vui_mvc_time_scale, 0, num * sizeof(unsigned));

			if(sps->vui_parameters_ext.vui_mvc_fixed_frame_rate_flag)
				free(sps->vui_parameters_ext.vui_mvc_fixed_frame_rate_flag);
			sps->vui_parameters_ext.vui_mvc_fixed_frame_rate_flag = (BOOL*)malloc(num * sizeof(BOOL));
			memset(sps->vui_parameters_ext.vui_mvc_fixed_frame_rate_flag, 0, num * sizeof(BOOL));

			if(sps->vui_parameters_ext.vui_mvc_nal_hrd_parameters_present_flag)
				free(sps->vui_parameters_ext.vui_mvc_nal_hrd_parameters_present_flag);
			sps->vui_parameters_ext.vui_mvc_nal_hrd_parameters_present_flag = (BOOL*)malloc(num * sizeof(BOOL));
			memset(sps->vui_parameters_ext.vui_mvc_nal_hrd_parameters_present_flag, 0, num * sizeof(BOOL));

			if(sps->vui_parameters_ext.nal_hrd_parameters)
				free(sps->vui_parameters_ext.nal_hrd_parameters);
			sps->vui_parameters_ext.nal_hrd_parameters = (hrd_parameters_t*)malloc(num * sizeof(hrd_parameters_t));
			memset(sps->vui_parameters_ext.nal_hrd_parameters, 0, num * sizeof(hrd_parameters_t));

			if(sps->vui_parameters_ext.vui_mvc_vcl_hrd_parameters_present_flag)
				free(sps->vui_parameters_ext.vui_mvc_vcl_hrd_parameters_present_flag);
			sps->vui_parameters_ext.vui_mvc_vcl_hrd_parameters_present_flag = (BOOL*)malloc(num * sizeof(BOOL));
			memset(sps->vui_parameters_ext.vui_mvc_vcl_hrd_parameters_present_flag, 0, num * sizeof(BOOL));

			if(sps->vui_parameters_ext.vcl_hrd_parameters)
				free(sps->vui_parameters_ext.vcl_hrd_parameters);
			sps->vui_parameters_ext.vcl_hrd_parameters = (hrd_parameters_t*)malloc(num * sizeof(hrd_parameters_t));
			memset(sps->vui_parameters_ext.vcl_hrd_parameters, 0, num * sizeof(hrd_parameters_t));

			if(sps->vui_parameters_ext.vui_mvc_low_delay_hrd_flag)
				free(sps->vui_parameters_ext.vui_mvc_low_delay_hrd_flag);
			sps->vui_parameters_ext.vui_mvc_low_delay_hrd_flag = (BOOL*)malloc(num * sizeof(BOOL));
			memset(sps->vui_parameters_ext.vui_mvc_low_delay_hrd_flag, 0, num * sizeof(BOOL));

			if(sps->vui_parameters_ext.vui_mvc_pic_struct_present_flag)
				free(sps->vui_parameters_ext.vui_mvc_pic_struct_present_flag);
			sps->vui_parameters_ext.vui_mvc_pic_struct_present_flag = (BOOL*)malloc(num * sizeof(BOOL));
			memset(sps->vui_parameters_ext.vui_mvc_pic_struct_present_flag, 0, num * sizeof(BOOL));

			for(int i = 0; i < num; i++)
			{
				sps->vui_parameters_ext.vui_mvc_temporal_id[i] = u_v(3, "VUI: vui_mvc_temporal_id");
				sps->vui_parameters_ext.vui_mvc_num_target_output_views[i] = ue_v("VUI; vui_mvc_num_target_output_views_minus1") + 1;

				if(sps->vui_parameters_ext.vui_mvc_num_target_output_views[i] > 0)
				{
					sps->vui_parameters_ext.vui_mvc_view_id[i] = (unsigned*)malloc(sps->vui_parameters_ext.vui_mvc_num_target_output_views[i] * sizeof(unsigned));
					for(int j = 0; j < sps->vui_parameters_ext.vui_mvc_num_target_output_views[i]; j++)
					{
						sps->vui_parameters_ext.vui_mvc_view_id[i][j] = ue_v("VUI; vui_mvc_view_id");
					}
				}

				sps->vui_parameters_ext.vui_mvc_timing_info_present_flag[i] = u_1("VUI: vui_mvc_timing_info_present_flag");
				if(sps->vui_parameters_ext.vui_mvc_timing_info_present_flag[i] > 0)
				{
					sps->vui_parameters_ext.vui_mvc_num_units_in_tick[i] = u_v(32, "VUI: vui_mvc_num_units_in_tick");
					sps->vui_parameters_ext.vui_mvc_time_scale[i] = u_v(32, "VUI: vui_mvc_time_scale");
					sps->vui_parameters_ext.vui_mvc_fixed_frame_rate_flag[i] = u_1("VUI: vui_mvc_fixed_frame_rate_flag");
				}

				sps->vui_parameters_ext.vui_mvc_nal_hrd_parameters_present_flag[i] = u_1("VUI: vui_mvc_nal_hrd_parameters_present_flag");
				if(sps->vui_parameters_ext.vui_mvc_nal_hrd_parameters_present_flag[i])
				{
					ReadHRDParameters ARGS1(&sps->vui_parameters_ext.nal_hrd_parameters[i]);
				}

				sps->vui_parameters_ext.vui_mvc_vcl_hrd_parameters_present_flag[i] = u_1("VUI: vui_mvc_vcl_hrd_parameters_present_flag");
				if(sps->vui_parameters_ext.vui_mvc_vcl_hrd_parameters_present_flag[i])
				{
					ReadHRDParameters ARGS1(&sps->vui_parameters_ext.vcl_hrd_parameters[i]);
				}

				if(sps->vui_parameters_ext.vui_mvc_nal_hrd_parameters_present_flag[i] || sps->vui_parameters_ext.vui_mvc_vcl_hrd_parameters_present_flag[i])
				{
					sps->vui_parameters_ext.vui_mvc_low_delay_hrd_flag[i] = u_1("VUI: vui_mvc_low_delay_hrd_flag");
				}

				sps->vui_parameters_ext.vui_mvc_pic_struct_present_flag[i] = u_1("VUI: vui_mvc_pic_struct_present_flag");
			}
		}
	}
	return CREL_OK;
}


CREL_RETURN ReadHRDParameters PARGS1(hrd_parameters_t *hrd)
{
	unsigned int SchedSelIdx;

	hrd->cpb_cnt_minus1                                      = ue_v (   "VUI: cpb_cnt_minus1");
	if (hrd->cpb_cnt_minus1 >= MAXIMUMVALUEOFcpb_cnt) {
		return CREL_WARNING_H264_STREAMERROR_LEVEL_1|CREL_WARNING_H264_ERROR_SPS;	//Error found
	}
	hrd->bit_rate_scale                                      = u_v ( 4,"VUI: bit_rate_scale");
	hrd->cpb_size_scale                                      = u_v ( 4,"VUI: cpb_size_scale");

	for( SchedSelIdx = 0; SchedSelIdx <= hrd->cpb_cnt_minus1; SchedSelIdx++ ) 
	{
		hrd->bit_rate_value_minus1[ SchedSelIdx ]             = ue_v ( "VUI: bit_rate_value_minus1");
		hrd->cpb_size_value_minus1[ SchedSelIdx ]             = ue_v ( "VUI: cpb_size_value_minus1");
		hrd->cbr_flag[ SchedSelIdx ]                          = u_1 ( "VUI: cbr_flag");
	}

	hrd->initial_cpb_removal_delay_length_minus1            = u_v ( 5,"VUI: initial_cpb_removal_delay_length_minus1");
	hrd->cpb_removal_delay_length_minus1                    = u_v ( 5,"VUI: cpb_removal_delay_length_minus1");
	hrd->dpb_output_delay_length_minus1                     = u_v ( 5,"VUI: dpb_output_delay_length_minus1");
	hrd->time_offset_length                                 = u_v ( 5,"VUI: time_offset_length");

	return CREL_OK;
}


CREL_RETURN InterpretPPS PARGS1(pic_parameter_set_rbsp_t *pps)
{
	unsigned i;
	int NumberBitsPerSliceGroupId;

	/*
	assert (&(IMGPAR g_dep) != NULL);
	assert (IMGPAR g_dep.Dcodestrm != 0);
	assert (pps != NULL);
	*/
	if ((&(IMGPAR g_dep) == NULL)||(IMGPAR g_dep.Dcodestrm == NULL)||(pps == NULL)) {
		return CREL_ERROR_H264_NOMEMORY;
	}

	pps->pic_parameter_set_id                  = ue_v ("PPS: pic_parameter_set_id");
	if (pps->pic_parameter_set_id  > MAXPPS - 1) {
		return CREL_WARNING_H264_STREAMERROR_LEVEL_1|CREL_WARNING_H264_ERROR_PPS;
	}
	pps->seq_parameter_set_id                  = ue_v ("PPS: seq_parameter_set_id");

	if (pps->seq_parameter_set_id > MAXSPS - 1) {
		return CREL_WARNING_H264_STREAMERROR_LEVEL_1|CREL_WARNING_H264_ERROR_PPS;
	}

	pps->entropy_coding_mode_flag              = u_1 ("PPS: entropy_coding_mode_flag");

	//! Note: as per JVT-F078 the following bit is unconditional.  If F078 is not accepted, then
	//! one has to fetch the correct SPS to check whether the bit is present (hopefully there is
	//! no consistency problem :-(
	//! The current encoder code handles this in the same way.  When you change this, don't forget
	//! the encoder!  StW, 12/8/02
	pps->pic_order_present_flag                = u_1 ("PPS: pic_order_present_flag");

	pps->num_slice_groups_minus1               = ue_v ("PPS: num_slice_groups_minus1");
	// IoK: This is constrained to be 0 for HD-DVD streams
	if(pps->num_slice_groups_minus1)
	{
		//g_HDProfileFault = HD_ProfileFault_PPS_num_slice_groups_minus1;
		return CREL_WARNING_H264_HD_ProfileFault|CREL_WARNING_H264_STREAMERROR_LEVEL_1;
	}

	//init.
	pps->slice_group_map_type = 0;
	pps->slice_group_change_rate_minus1 = 0;
	// FMO stuff begins here
	pps->num_slice_group_map_units_minus1 = 0;
	if (pps->num_slice_groups_minus1 > 0)
	{
		pps->slice_group_map_type               = ue_v ("PPS: slice_group_map_type");
		if (pps->slice_group_map_type == 0)
		{
			for (i=0; i<=pps->num_slice_groups_minus1; i++)
				pps->run_length_minus1 [i]                  = ue_v ("PPS: run_length_minus1 [i]");
		}
		else if (pps->slice_group_map_type == 2)
		{
			for (i=0; i<pps->num_slice_groups_minus1; i++)
			{
				//! JVT-F078: avoid reference of SPS by using ue(v) instead of u(v)
				pps->top_left [i]                          = ue_v ("PPS: top_left [i]");
				pps->bottom_right [i]                      = ue_v ("PPS: bottom_right [i]");
			}
		}
		else if (pps->slice_group_map_type == 3 ||
			pps->slice_group_map_type == 4 ||
			pps->slice_group_map_type == 5)
		{
			pps->slice_group_change_direction_flag     = u_1 ("PPS: slice_group_change_direction_flag");
			pps->slice_group_change_rate_minus1        = ue_v ("PPS: slice_group_change_rate_minus1");
		}
		else if (pps->slice_group_map_type == 6)
		{
			if (pps->num_slice_groups_minus1+1 >4)
				NumberBitsPerSliceGroupId = 3;
			else if (pps->num_slice_groups_minus1+1 > 2)
				NumberBitsPerSliceGroupId = 2;
			else
				NumberBitsPerSliceGroupId = 1;
			//! JVT-F078, exlicitly signal number of MBs in the map
			pps->num_slice_group_map_units_minus1      = ue_v ("PPS: num_slice_group_map_units_minus1");
			for (i=0; i<=pps->num_slice_group_map_units_minus1; i++)
				pps->slice_group_id[i] = u_v (NumberBitsPerSliceGroupId, "slice_group_id[i]");
		}
	}
	else
	{ // Later code assumes num_slice_group_map_units_minus1 == 0
		pps->num_slice_group_map_units_minus1 = 0;
	}

	// End of FMO stuff

	pps->num_ref_idx_l0_active_minus1          = ue_v ("PPS: num_ref_idx_l0_active_minus1");
	pps->num_ref_idx_l1_active_minus1          = ue_v ("PPS: num_ref_idx_l1_active_minus1");	// Add Check here is required
	if ( (pps->num_ref_idx_l0_active_minus1 > MAX_LIST_SIZE_FRAME - 2) || (pps->num_ref_idx_l1_active_minus1 > MAX_LIST_SIZE_FRAME - 2) ) {
		return CREL_WARNING_H264_STREAMERROR_LEVEL_3|CREL_WARNING_H264_ERROR_PPS;
	}

	pps->weighted_pred_flag                    = u_1 ("PPS: weighted prediction flag");
	pps->weighted_bipred_idc                   = u_v ( 2, "PPS: weighted_bipred_idc");
	pps->pic_init_qp_minus26                   = se_v ("PPS: pic_init_qp_minus26");
	pps->pic_init_qs_minus26                   = se_v ("PPS: pic_init_qs_minus26");

	pps->chroma_qp_index_offset                = se_v ("PPS: chroma_qp_index_offset");

	pps->deblocking_filter_control_present_flag = u_1 ("PPS: deblocking_filter_control_present_flag");
	pps->constrained_intra_pred_flag           = u_1 ("PPS: constrained_intra_pred_flag");
	pps->redundant_pic_cnt_present_flag        = u_1 ("PPS: redundant_pic_cnt_present_flag");
	// IoK: This is constrained to be 0 for HD-DVD streams
	/*
	if(pps->redundant_pic_cnt_present_flag)
	{
	g_HDProfileFault = HD_ProfileFault_PPS_redundant_pic_cnt_present_flag;
	g_HDProfileFault = g_HDProfileFault;	//Add this line for debugging
	// Haihua: It would block BD playback. But we still need check if it impact HD-DVD error resillience
	}
	*/
	if(more_rbsp_data ARGS0()) // more_data_in_rbsp()
	{
		//Fidelity Range Extensions Stuff
		pps->transform_8x8_mode_flag           = u_1 ("PPS: transform_8x8_mode_flag");
		pps->pic_scaling_matrix_present_flag     = u_1 ("PPS: pic_scaling_matrix_present_flag");

		if(pps->pic_scaling_matrix_present_flag)
		{
			for(i=0; i<(6+((unsigned)pps->transform_8x8_mode_flag<<1)); i++)
			{
				pps->pic_scaling_list_present_flag[i]= u_1 ("PPS: pic_scaling_list_present_flag");

				if(pps->pic_scaling_list_present_flag[i])
				{
					if(i<6)
						Scaling_List ARGS3(pps->ScalingList4x4[i], 16, &pps->UseDefaultScalingMatrix4x4Flag[i]);
					else
						Scaling_List ARGS3(pps->ScalingList8x8[i-6], 64, &pps->UseDefaultScalingMatrix8x8Flag[i-6]);
				}
			}
		}
		pps->second_chroma_qp_index_offset      = se_v ("PPS: second_chroma_qp_index_offset");
	}
	else
	{
		pps->second_chroma_qp_index_offset      = pps->chroma_qp_index_offset;
		pps->transform_8x8_mode_flag            = 0; // Later code assumes this value
		pps->pic_scaling_matrix_present_flag    = 0; // Later code assumes this value
	}

	pps->Valid = TRUE;
	return CREL_OK;
}


#if 0
void PPSConsistencyCheck (pic_parameter_set_rbsp_t *pps)
{
	DEBUG_SHOW_SW_INFO ("Consistency checking a picture parset, to be implemented\n");
	//  if (pps->seq_parameter_set_id invalid then do something)
}
void SPSConsistencyCheck (seq_parameter_set_rbsp_t *sps)
{
	DEBUG_SHOW_SW_INFO ("Consistency checking a sequence parset, to be implemented\n");
}
#endif

void MakePPSavailable PARGS2(int id, pic_parameter_set_rbsp_t *pps)
{
	assert (pps->Valid == TRUE);

	if (PicParSet[id].Valid == TRUE && PicParSet[id].slice_group_id != NULL)
	{
		_aligned_free (PicParSet[id].slice_group_id);
		PicParSet[id].slice_group_id = NULL;
	}

	memcpy (&PicParSet[id], pps, sizeof (pic_parameter_set_rbsp_t));

	if ((PicParSet[id].slice_group_id = (unsigned int *) _aligned_malloc ((PicParSet[id].num_slice_group_map_units_minus1+1)*sizeof(int), 16)) == NULL)
		no_mem_exit ("MakePPSavailable: Cannot alloc slice_group_id");

	memcpy (PicParSet[id].slice_group_id, pps->slice_group_id, (pps->num_slice_group_map_units_minus1+1)*sizeof(int));
}

void MakeSPSavailable PARGS2(int id, seq_parameter_set_rbsp_t *sps)
{
	//assert (sps->Valid == TRUE);	//No necessary in current implementation.
	memcpy (&SeqParSet[id], sps, sizeof (seq_parameter_set_rbsp_t));
}

void FreeSPS_MVC_Related(seq_parameter_set_rbsp_t *sps)
{
	if(sps->Valid)
	{
		if(sps->num_views > 0)
		{
			for(int i = 0; i<sps->num_views; i++)
			{
				if(sps->anchor_refs_l0[i])
					_aligned_free(sps->anchor_refs_l0[i]);
				if(sps->anchor_refs_l1[i])
					_aligned_free(sps->anchor_refs_l1[i]);
				if(sps->non_anchor_refs_l0[i])
					_aligned_free(sps->non_anchor_refs_l0[i]);
				if(sps->non_anchor_refs_l1[i])
					_aligned_free(sps->non_anchor_refs_l1[i]);
			}

			if(sps->view_id)
				_aligned_free(sps->view_id);

			if(sps->num_anchor_refs_l0)
				_aligned_free(sps->num_anchor_refs_l0);

			if(sps->anchor_refs_l0)
				_aligned_free(sps->anchor_refs_l0);

			if(sps->num_anchor_refs_l1)
				_aligned_free(sps->num_anchor_refs_l1);

			if(sps->anchor_refs_l1)
				_aligned_free(sps->anchor_refs_l1);

			if(sps->num_non_anchor_refs_l0)
				_aligned_free(sps->num_non_anchor_refs_l0);

			if(sps->non_anchor_refs_l0)
				_aligned_free(sps->non_anchor_refs_l0);

			if(sps->num_non_anchor_refs_l1)
				_aligned_free(sps->num_non_anchor_refs_l1);

			if(sps->non_anchor_refs_l1)
				_aligned_free(sps->non_anchor_refs_l1);

			for( int i=0; i<sps->num_level_values_signalled; i++)
			{
				for( int j=0; j<sps->num_applicable_ops[i]; j++)
				{
					if(sps->applicable_op_target_view_id[i][j])
						_aligned_free(sps->applicable_op_target_view_id[i][j]);
				}

				if(sps->applicable_op_temporal_id[i])
					_aligned_free(sps->applicable_op_temporal_id[i]);
				if(sps->applicable_op_num_target_views_minus1[i])
					_aligned_free(sps->applicable_op_num_target_views_minus1[i]);
				if(sps->applicable_op_target_view_id[i])
					_aligned_free(sps->applicable_op_target_view_id[i]);
				if(sps->applicable_op_num_views_minus1[i])
					_aligned_free(sps->applicable_op_num_views_minus1[i]);
			}

			if(sps->level_idc_mvc)
				_aligned_free(sps->level_idc_mvc);

			if(sps->num_applicable_ops)
				_aligned_free(sps->num_applicable_ops);

			if(sps->applicable_op_temporal_id)
				_aligned_free(sps->applicable_op_temporal_id);

			if(sps->applicable_op_num_target_views_minus1)
				_aligned_free(sps->applicable_op_num_target_views_minus1);

			if(sps->applicable_op_target_view_id)
				_aligned_free(sps->applicable_op_target_view_id);

			if(sps->applicable_op_num_views_minus1)
				_aligned_free(sps->applicable_op_num_views_minus1);
		}

		sps->Valid = 0;
	}
}

void MakeSubsetSPSavailable PARGS2(int id, seq_parameter_set_rbsp_t *sps)
{
	//assert (sps->Valid == TRUE);	//No necessary in current implementation.
	FreeSPS_MVC_Related(&SeqParSubset[id]);
	memcpy (&SeqParSubset[id], sps, sizeof (seq_parameter_set_rbsp_t));
}

BOOL CheckSPS PARGS2(NALU_t *one_nalu, unsigned int view_id)
{
	if(stream_global->m_SPSchecked)
		return FALSE;
	
	stream_global->m_SPSchecked = TRUE;
	DecodingEnvironment *dep = &(IMGPAR g_dep);
	CREL_RETURN ret;
	seq_parameter_set_rbsp_t *sps = AllocSPS();
	int view_index = GetViewIndex ARGS1 (view_id);

#if !defined(_COLLECT_PIC_)
	seq_parameter_set_rbsp_t *latest_sps = &active_sps;
#else
	seq_parameter_set_rbsp_t *latest_sps = stream_global->m_active_sps_on_view[view_index];
#endif

	//memcpy (dep->streamBuffer, &one_nalu->buf[1], one_nalu->len-1);
	dep->Dbasestrm   = &one_nalu->buf[1];
	dep->Dstrmlength = one_nalu->len-1;
	ret = RBSPtoSODB (dep->Dbasestrm, &(dep->Dstrmlength));
	if (FAILED(ret)) {
		FreePartition (dep);
		FreeSPS (sps);
		return FALSE;
	}
	dep->Dcodestrm   = dep->Dbasestrm;
	dep->Dei_flag    = 0;
	dep->Dbits_to_go = 0;
	dep->Dbuffer     = 0;
	ret = InterpretSPS ARGS1(sps);
	if (FAILED(ret)) {
		FreePartition (dep);
		FreeSPS (sps);
		return FALSE;
	}    

	if ((latest_sps) && (latest_sps->Valid))
	{
		if (sps->seq_parameter_set_id == latest_sps->seq_parameter_set_id)
		{
			if (sps_is_equal(sps, latest_sps) == SPS_CRITICAL_CHANGE)
			{
#if !defined(_COLLECT_PIC_)
				if (dec_picture)
				{
					// this may only happen on slice loss
					ret = exit_picture ARGS0();
					if (FAILED(ret)) {
						return ret;
					}
				}
#endif
				FreePartition (dep);
				FreeSPS (sps);
				return TRUE;
			}
		}
	}

	FreePartition (dep);
	FreeSPS (sps);
	return FALSE;
}

CREL_RETURN ProcessSPS PARGS1(NALU_t *one_nalu)
{
	DecodingEnvironment *dep = &(IMGPAR g_dep);
	CREL_RETURN ret;
	seq_parameter_set_rbsp_t *sps = AllocSPS();
#if !defined(_COLLECT_PIC_)
	seq_parameter_set_rbsp_t *latest_sps = &active_sps;
#else
	seq_parameter_set_rbsp_t *latest_sps = stream_global->m_active_sps_on_view[0]; //ProcessSPS only handles base view
#endif
	if(stream_global->m_SPSchecked)
		stream_global->m_SPSchecked = FALSE;

	//memcpy (dep->streamBuffer, &one_nalu->buf[1], one_nalu->len-1);
	dep->Dbasestrm   = &one_nalu->buf[1];
	dep->Dstrmlength = one_nalu->len-1;
	ret = RBSPtoSODB (dep->Dbasestrm, &(dep->Dstrmlength));
	if (FAILED(ret)) {
		return ret;
	}
	dep->Dcodestrm   = dep->Dbasestrm;
	dep->Dei_flag    = 0;
	dep->Dbits_to_go = 0;
	dep->Dbuffer     = 0;
	ret = InterpretSPS ARGS1(sps);
	if (FAILED(ret)) {
		FreePartition (dep);
		FreeSPS (sps);

		return ret;
	}    

	if ((latest_sps) && (latest_sps->Valid))
	{
		if (sps->seq_parameter_set_id == latest_sps->seq_parameter_set_id)
		{
			if (sps_is_equal(sps, latest_sps) == SPS_CRITICAL_CHANGE)
			{
#if !defined(_COLLECT_PIC_)
				if (dec_picture)
				{
					// this may only happen on slice loss
					ret = exit_picture ARGS0();
					if (FAILED(ret)) {
						return ret;
					}
				}
#endif
				latest_sps->Valid=NULL;
				stream_global->m_active_sps_on_view[0] = NULL;
			}
		}
	}
	// SPSConsistencyCheck (pps);
	MakeSPSavailable ARGS2(sps->seq_parameter_set_id, sps);
#if defined(_COLLECT_PIC_)
	stream_global->profile_idc = IMGPAR profile_idc = sps->profile_idc; //ADD-VG
#else
	IMGPAR profile_idc = sps->profile_idc; //ADD-VG
#endif

	FreePartition (dep);
	FreeSPS (sps);

	old_slice.idr_pic_id = -1;
	return CREL_OK;
}

CREL_RETURN ProcessNaluExt PARGS1(NALU_t *one_nalu) {

	DecodingEnvironment *dep = &(IMGPAR g_dep);
	CREL_RETURN ret;
	unsigned int temp;
	nalu_header_mvc_extension_t * nalu_mvc_ext = &(stream_global->nalu_mvc_extension);

	dep->Dbasestrm   = &one_nalu->buf[1];
	dep->Dstrmlength = one_nalu->len-1;
	ret = RBSPtoSODB (dep->Dbasestrm, &(dep->Dstrmlength));
	if (FAILED(ret)) {
		return ret;
	}


	dep->Dcodestrm   = dep->Dbasestrm;
	dep->Dei_flag    = 0;
	dep->Dbits_to_go = 0;
	dep->Dbuffer     = 0;

	temp    = u_1 ("NAL UNIT EXTENSION: svc_extension_flag");

	if (temp) {
		//SVC extension not supported yet
		return CREL_WARNING_H264_STREAMERROR_LEVEL_1|CREL_ERROR_H264_NOTSUPPORTED;

	} else {
		//MVC extension
		nalu_mvc_ext->idrFlag = !(u_1("NAL UNIT MVC EXTENSION: non_idr_flag"));

		nalu_mvc_ext->priorityId = u_v(6, "NAL UNIT MVC EXTENSION: priority_id");

		nalu_mvc_ext->viewId	 = u_v(10, "NAL UNIT MVC EXTENSION: view_id");

		nalu_mvc_ext->temporalId = u_v(3, "NAL UNIT MVC EXTENSION: temporal_id");

		nalu_mvc_ext->anchorPicFlag = u_1("NAL UNIT MVC EXTENSION: anchor_pic_flag");

		nalu_mvc_ext->interViewFlag = u_1("NAL UNIT MVC EXTENSION: inter_view_flag");

		temp = u_1("NAL UNIT MVC EXTENSION: reserved_one_bit");

		nalu_mvc_ext->valid = TRUE;

	}

	return CREL_OK;



}


CREL_RETURN ProcessSPSSubset PARGS1(NALU_t *one_nalu)
{

	DecodingEnvironment *dep = &(IMGPAR g_dep);
	CREL_RETURN ret;
	seq_parameter_set_rbsp_t *sps = AllocSPS();
#if !defined(_COLLECT_PIC_)
	seq_parameter_set_rbsp_t *latest_sps = &active_sps;
#else
	seq_parameter_set_rbsp_t *latest_sps;
#endif
	if(stream_global->m_SPSchecked)
		stream_global->m_SPSchecked = FALSE;

	//memcpy (dep->streamBuffer, &one_nalu->buf[1], one_nalu->len-1);
	dep->Dbasestrm   = &one_nalu->buf[1];
	dep->Dstrmlength = one_nalu->len-1;
	ret = RBSPtoSODB (dep->Dbasestrm, &(dep->Dstrmlength));
	if (FAILED(ret)) {
		return ret;
	}
	dep->Dcodestrm   = dep->Dbasestrm;
	dep->Dei_flag    = 0;
	dep->Dbits_to_go = 0;
	dep->Dbuffer     = 0;
	ret = InterpretSPS ARGS1(sps);
	if (FAILED(ret)) {
		FreePartition (dep);
		FreeSPS (sps);

		return ret;
	}    

	ret = InterpretSPSSubsetExt ARGS1(sps);
	if (FAILED(ret)) {
		FreePartition (dep);
		FreeSPS (sps);

		return ret;
	}    
	// don't need to flush if subset sps. referred from reference code.
/*
	for ( unsigned int i = 0; i < stream_global->num_views; i++ ) {

		latest_sps = stream_global->m_active_sps_on_view[i];

		if ((latest_sps) && (latest_sps->Valid))
		{
			if (sps->seq_parameter_set_id == latest_sps->seq_parameter_set_id)
			{
				if (sps_is_equal(sps, latest_sps) == SPS_CRITICAL_CHANGE)
				{
#if !defined(_COLLECT_PIC_)
					if (dec_picture)
					{
						// this may only happen on slice loss
						ret = exit_picture ARGS0();
						if (FAILED(ret)) {
							return ret;
						}
					}
#endif
					latest_sps->Valid=NULL;
					stream_global->m_active_sps_on_view[i] = NULL;
				}
			}
		}

	}
*/
	// SPSConsistencyCheck (pps);
	MakeSubsetSPSavailable ARGS2(sps->seq_parameter_set_id, sps);
#if defined(_COLLECT_PIC_)
	stream_global->profile_idc = IMGPAR profile_idc = sps->profile_idc; //ADD-VG
#else
	IMGPAR profile_idc = sps->profile_idc; //ADD-VG
#endif

	FreePartition (dep);
	FreeSPS (sps);

	old_slice.idr_pic_id = -1;
	return CREL_OK;
}


CREL_RETURN ProcessPPS PARGS1(NALU_t *one_nalu)
{
	DecodingEnvironment *dep;
	pic_parameter_set_rbsp_t *pps;
	CREL_RETURN ret;

	dep = AllocPartition ARGS1(1);
	pps = AllocPPS();
	//memcpy (dep->streamBuffer, &one_nalu->buf[1], one_nalu->len-1);
	dep->Dbasestrm   = &one_nalu->buf[1];
	dep->Dstrmlength = one_nalu->len-1;
	ret = RBSPtoSODB (dep->Dbasestrm, &(dep->Dstrmlength));
	if (FAILED(ret)) {
		return ret;
	}
	dep->Dcodestrm   = dep->Dbasestrm;
	dep->Dei_flag    = 0;
	dep->Dbits_to_go = 0;
	dep->Dbuffer     = 0;
	ret = InterpretPPS ARGS1(pps);
	// IoK: check for HD-profile violation due to incorrect num_slice_groups_minus1 or redundant_pic_cnt_present_flag
	if(FAILED(ret))
	{
		FreePartition (dep);
		FreePPS (pps);
		return ret;
	}
	// PPSConsistencyCheck (pps);
	for (unsigned int i = 0; i < stream_global->num_views; i++) {
#if !defined(_COLLECT_PIC_)
		if (active_pps.Valid && active_pps.Valid)
		{
			if (pps->pic_parameter_set_id == active_pps.pic_parameter_set_id)
			{
				if (!pps_is_equal(pps, &active_pps))
				{
					if (dec_picture)
					{
						// this may only happen on slice loss
						ret = exit_picture ARGS0();
						if (FAILED(ret)) {
							return ret;
						}
					}
					active_pps.Valid = NULL;
				}
			}
		}
#else
		if (stream_global->m_active_pps_on_view[i] && stream_global->m_active_pps_on_view[i]->Valid)
		{
			if (pps->pic_parameter_set_id == stream_global->m_active_pps_on_view[i]->pic_parameter_set_id)
			{
				if (!pps_is_equal(pps, stream_global->m_active_pps_on_view[i]))
				{
					stream_global->m_active_pps_on_view[i] = NULL;
				}
			}
		}
#endif
	}

	MakePPSavailable ARGS2(pps->pic_parameter_set_id, pps);
	FreePartition (dep);
	FreePPS (pps);
	return CREL_OK;
}

CREL_RETURN ProcessAUD PARGS2(NALU_t *one_nalu, int* primary_pic_type)
{
	CREL_RETURN ret;

	DecodingEnvironment *dep;

	dep = AllocPartition ARGS1(1);

	//memcpy (dep->streamBuffer, &one_nalu->buf[1], one_nalu->len-1);
	dep->Dbasestrm   = &one_nalu->buf[1];
	dep->Dstrmlength = one_nalu->len-1;
	ret = RBSPtoSODB (dep->Dbasestrm, &(dep->Dstrmlength));
	if (FAILED(ret)) {
		return ret;
	}
	dep->Dcodestrm   = dep->Dbasestrm;
	dep->Dei_flag    = 0;
	dep->Dbits_to_go = 0;
	dep->Dbuffer     = 0;

	*primary_pic_type = u_v(3, "SH: Primary Picture Type");

	FreePartition (dep);

	return CREL_OK;
}

CREL_RETURN activate_global_sps PARGS2(seq_parameter_set_rbsp_t *sps, unsigned int view_index)
{
	CREL_RETURN ret;
#if !defined(_COLLECT_PIC_)
	if (!IMGPAR no_output_of_prior_pics_flag)
	{
		ret = flush_dpb ARGS1(view_index);
		if (FAILED(ret)) {
			return ret;
		}
	}
	ret = init_dpb ARGS1(view_index);
	if (FAILED(ret)) {
		return ret;
	}
#else
	if ( (stream_global->m_active_sps_on_view[view_index] == NULL) || (stream_global->m_active_sps_on_view[view_index]->Valid == 0) || (sps_is_equal(stream_global->m_active_sps_on_view[view_index], sps) == SPS_CRITICAL_CHANGE) )	
	{
		stream_global->m_active_sps_on_view[view_index] = sps;
		if (!stream_global->no_output_of_prior_pics_flag)
		{
			ret = flush_dpb ARGS1(view_index);
			if (FAILED(ret)) {
				return ret;
			}
		}
		ret = init_dpb ARGS1(view_index);
		if (FAILED(ret)) {
			return ret;
		}
	}
#endif


	if (NULL!=Co_located_MB)
	{
		free_colocated_MB(Co_located_MB);
	}
	Co_located_MB = alloc_colocatedMB (IMGPAR width, IMGPAR height,sps->mb_adaptive_frame_field_flag);
	if (Co_located_MB == NULL) {
		return CREL_ERROR_H264_NOMEMORY;
	}

	return CREL_OK;

}



CREL_RETURN activate_sps PARGS2(seq_parameter_set_rbsp_t *sps, unsigned int view_id)
{
	CREL_RETURN ret;
	unsigned int view_index = (IMGPAR currentSlice) ? (IMGPAR currentSlice->viewIndex) : 0;

	if (sps_is_equal(sps, &active_sps) != SPS_NO_CHANGE)
	{
#if !defined(_COLLECT_PIC_)
		if (dec_picture)
		{
			// this may only happen on slice loss
			ret = exit_picture ARGS0();
			if (FAILED(ret)) {
				return ret;
			}
		}
#endif
		//active_sps = sps;
		memcpy (&active_sps, sps, sizeof (seq_parameter_set_rbsp_t));

		if(view_id > 0 && view_index == 0)
		{
			if(sps->view_id != NULL)
			{
				for(int i = 0; i < stream_global->num_views; i++)
				{
					if(view_id == sps->view_id[i])
					{
						view_index = i;
						break;
					}
				}

				IMGPAR currentSlice->viewIndex = view_index;
				stream_global->m_pbValidViews[view_index] = 1;
			}
		}

		IMGPAR bitdepth_chroma = 0;
		IMGPAR width_cr        = 0;
		IMGPAR height_cr       = 0;

		// Fidelity Range Extensions stuff (part 1)
		IMGPAR bitdepth_luma   = 8; // HP restriction
#ifdef __SUPPORT_YUV400__
		if (sps->chroma_format_idc != YUV400)
#endif
			IMGPAR bitdepth_chroma = 8;  // HP restriction

		IMGPAR MaxFrameNum = 1<<(sps->log2_max_frame_num_minus4+4);
		IMGPAR PicWidthInMbs = (sps->pic_width_in_mbs_minus1 +1);
		IMGPAR PicHeightInMapUnits = (sps->pic_height_in_map_units_minus1 +1);
		IMGPAR FrameHeightInMbs = ( 2 - sps->frame_mbs_only_flag ) * IMGPAR PicHeightInMapUnits;
		IMGPAR FrameSizeInMbs = IMGPAR PicWidthInMbs * IMGPAR FrameHeightInMbs;

		IMGPAR yuv_format=sps->chroma_format_idc;

		IMGPAR width = IMGPAR PicWidthInMbs * MB_BLOCK_SIZE;
		IMGPAR height = IMGPAR FrameHeightInMbs * MB_BLOCK_SIZE;

		if (sps->chroma_format_idc == YUV420)
		{
			IMGPAR width_cr = IMGPAR width /2;
			IMGPAR height_cr = IMGPAR height / 2;
		}
		else if (sps->chroma_format_idc == YUV422)
		{
			IMGPAR width_cr = IMGPAR width /2;
			IMGPAR height_cr = IMGPAR height;
		}
		else if (sps->chroma_format_idc == YUV444)
		{
			//YUV444
			IMGPAR width_cr = IMGPAR width;
			IMGPAR height_cr = IMGPAR height;
		}

		init_frext ARGS0();                                               
		ret = init_global_buffers ARGS0();
		if (FAILED(ret)) {
			return CREL_OK;
		}

#if !defined(_COLLECT_PIC_)
		if (!IMGPAR no_output_of_prior_pics_flag)
		{
			ret = flush_dpb ARGS1(view_index);
			if (FAILED(ret)) {
				return ret;
			}
		}
		ret = init_dpb ARGS1(view_index);
		if (FAILED(ret)) {
			return ret;
		}
#else
		if ( ( !( ( stream_global->m_active_sps_on_view[view_index] ) 
			&& (stream_global->m_active_sps_on_view[view_index]->Valid) 
			&& (sps_is_equal(stream_global->m_active_sps_on_view[view_index], sps) != SPS_CRITICAL_CHANGE ) ) ) 
			&& (IMGPAR currentSlice->start_mb_nr == 0) )
		{
			stream_global->m_active_sps_on_view[view_index] = sps;
			if (!stream_global->no_output_of_prior_pics_flag)
			{
				if (dpb.init_done != NULL && view_index < stream_global->dpb_pre_alloc_views && dpb.init_done[view_index]) {
					ret = flush_dpb ARGS1(view_index);
					if (FAILED(ret)) {
						return ret;
					}
				}
			}
			ret = init_dpb ARGS1(view_index);
			if (FAILED(ret)) {
				return ret;
			}
		}
#endif


		if (NULL!=Co_located_MB)
		{
			free_colocated_MB(Co_located_MB);
		}
		Co_located_MB = alloc_colocatedMB (IMGPAR width, IMGPAR height,sps->mb_adaptive_frame_field_flag);

		if (Co_located_MB == NULL) {
			return CREL_ERROR_H264_NOMEMORY;
		}
	}

	//If global active_sps still NULL, set sps as default
	if (stream_global->m_active_sps_on_view[view_index] == NULL)
		stream_global->m_active_sps_on_view[view_index] = sps;

	return CREL_OK;
}


CREL_RETURN activate_global_pps PARGS2(pic_parameter_set_rbsp_t *pps, unsigned int view_index)
{
#if !defined(_COLLECT_PIC_)
	if ( !( ( active_pps.Valid ) && pps_is_equal(&active_pps, pps) ) )	
	{
		memcpy(&active_pps, pps, sizeof(pic_parameter_set_rbsp_t));
	}
#else
	if ( !( ( stream_global->m_active_pps_on_view[view_index] ) && (stream_global->m_active_pps_on_view[view_index]->Valid) && pps_is_equal(stream_global->m_active_pps_on_view[view_index], pps) ) )	
	{
		stream_global->m_active_pps_on_view[view_index] = pps;
	}
#endif

	return CREL_OK;
}


CREL_RETURN activate_pps PARGS2(pic_parameter_set_rbsp_t *pps, unsigned int view_index)
{
	if (!pps_is_equal(pps, &active_pps))
	{
#if !defined(_COLLECT_PIC_)
		if (dec_picture)
		{
			// this may only happen on slice loss
			int ret = exit_picture ARGS0();
			if (FAILED(ret)) {
				return ret;
			}
		}
#endif
		memcpy (&active_pps, pps, sizeof (pic_parameter_set_rbsp_t));

#if !defined(_COLLECT_PIC_)
		if ( !( (active_pps.Valid) && pps_is_equal(&active_pps, pps) ) )	
		{
			memcpy(&active_pps, pps, sizeof(pic_parameter_set_rbsp_t));
		}
#else
		if ( !( ( stream_global->m_active_pps_on_view[view_index] ) && (stream_global->m_active_pps_on_view[view_index]->Valid) && pps_is_equal(stream_global->m_active_pps_on_view[view_index], pps) ) )	
		{
			stream_global->m_active_pps_on_view[view_index] = pps;
		}
#endif

		// Fidelity Range Extensions stuff (part 2)
		IMGPAR Transform8x8Mode = pps->transform_8x8_mode_flag;

	}

	return CREL_OK;
}  

CREL_RETURN UseParameterSet PARGS1(int PicParsetId)
{
	seq_parameter_set_rbsp_t *sps = &SeqParSet[PicParSet[PicParsetId].seq_parameter_set_id];
	pic_parameter_set_rbsp_t *pps = &PicParSet[PicParsetId];
	CREL_RETURN ret;

	if (PicParSet[PicParsetId].Valid != TRUE) {
		DEBUG_SHOW_ERROR_INFO ("Trying to use an invalid (uninitialized) Picture Parameter Set with ID %d, expect the unexpected...\n", PicParsetId);
		return CREL_WARNING_H264_STREAMERROR_LEVEL_1;
	}
	if (SeqParSet[PicParSet[PicParsetId].seq_parameter_set_id].Valid != TRUE) {
		DEBUG_SHOW_ERROR_INFO ("PicParset %d references an invalid (uninitialized) Sequence Parameter Set with ID %d, expect the unexpected...\n", PicParsetId, PicParSet[PicParsetId].seq_parameter_set_id);
		return CREL_WARNING_H264_STREAMERROR_LEVEL_1;
	}

	sps =  &SeqParSet[PicParSet[PicParsetId].seq_parameter_set_id];


	// In theory, and with a well-designed software, the lines above
	// are everything necessary.  In practice, we need to patch many values
	// in IMGPAR  (but no more in inp-> -- these have been taken care of)

	// Sequence Parameter Set Stuff first

	//  DEBUG_SHOW_SW_INFO ("Using Picture Parameter set %d and associated Sequence Parameter Set %d\n", PicParsetId, PicParSet[PicParsetId].seq_parameter_set_id);

	if ((int) sps->pic_order_cnt_type < 0 || sps->pic_order_cnt_type > 2)  // != 1
	{
		DEBUG_SHOW_ERROR_INFO ("invalid sps->pic_order_cnt_type = %d\n", sps->pic_order_cnt_type);
		DEBUG_SHOW_ERROR_INFO ("pic_order_cnt_type != 1", -1000);
		return CREL_WARNING_H264_STREAMERROR_LEVEL_1;
	}

	if (sps->pic_order_cnt_type == 1)
	{
		if(sps->num_ref_frames_in_pic_order_cnt_cycle >= MAXnum_ref_frames_in_pic_order_cnt_cycle)
		{
			DEBUG_SHOW_ERROR_INFO("num_ref_frames_in_pic_order_cnt_cycle too large",-1011);
			return CREL_WARNING_H264_STREAMERROR_LEVEL_1;
		}
	}

	ret = activate_sps ARGS2(sps, 0);		//Base View only
	if (FAILED(ret)) {
		return ret;
	}
	activate_pps ARGS2(pps, 0);  //Base view only


	// currSlice->dp_mode is set by read_new_slice (NALU first byte available there)
	if (pps->entropy_coding_mode_flag == UVLC)
	{
		nal_startcode_follows = uvlc_startcode_follows;
	}
	else
	{
		nal_startcode_follows = cabac_startcode_follows;
	}

	return CREL_OK;
}

int GetBaseViewId PARGS0()
{
	seq_parameter_set_rbsp_t *curr_subset_sps;
	int iBaseViewId = -1;
	curr_subset_sps = &SeqParSubset[0];

	int i;
	for(i = 0; i < MAXSPS; i++)
	{
		if(curr_subset_sps->Valid && curr_subset_sps->seq_parameter_set_id < MAXSPS && curr_subset_sps->view_id != NULL)
		{
			iBaseViewId = curr_subset_sps->view_id[0];
			break;
		}
		curr_subset_sps++;
	}

	if(iBaseViewId < 0)
		iBaseViewId = 0;

	/*if(i<MAXSPS)
		stream_global->m_active_sps_on_view[1] = curr_subset_sps;*/

	return iBaseViewId;
}

int GetViewIndex PARGS1(unsigned int view_id)
{
	seq_parameter_set_rbsp_t *curr_subset_sps = stream_global->m_active_sps_on_view[1];

	if(curr_subset_sps == NULL)
	{
		for(int i = 0; i < MAXSPS; i++)
		{
			if(SeqParSubset[i].Valid)
				curr_subset_sps = &SeqParSubset[i];
		}

		if(curr_subset_sps == NULL)
			return 0;
	}

	int view_index = -1;
	int num_views = curr_subset_sps->num_views;

	int i;
	for(i = 0; i < num_views; i++)
	{
		if(view_id == curr_subset_sps->view_id[i])
		{
			view_index = i;
			break;
		}
	}

	if(view_index < 0)
		view_index = 0;

	return view_index;
}