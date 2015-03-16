
/*!
**************************************************************************************
* \file
*    parsetcommon.h
* \brief
*    Picture and Sequence Parameter Sets, structures common to encoder and decoder
*    This code reflects JVT version xxx
*  \date 25 November 2002
* \author
*    Main contributors (see contributors.h for copyright, address and affiliation details) 
*      - Stephan Wenger        <stewe@cs.tu-berlin.de>
***************************************************************************************
*/



// In the JVT syntax, frequently flags are used that indicate the presence of
// certain pieces of information in the NALU.  Here, these flags are also
// present.  In the encoder, those bits indicate that the values signalled to
// be present are meaningful and that this part of the syntax should be
// written to the NALU.  In the decoder, the flag indicates that information
// was received from the decoded NALU and should be used henceforth.
// The structure names were chosen as indicated in the JVT syntax

#ifndef _PARSETCOMMON_H_
#define _PARSETCOMMON_H_

#include <windows.h>
#include "defines.h"

#define MAXIMUMPARSETRBSPSIZE   1500
#define MAXIMUMPARSETNALUSIZE   1500
#define SIZEslice_group_id      (sizeof (int) * 60000)    // should be sufficient for HUGE pictures, need one int per MB in a picture

#define MAXSPS  32
#define MAXPPS  256

#define MAXIMUMVALUEOFcpb_cnt   32
typedef struct
{
	unsigned  cpb_cnt_minus1;                                   // ue(v)
	unsigned  bit_rate_scale;                                   // u(4)
	unsigned  cpb_size_scale;                                   // u(4)
	unsigned  bit_rate_value_minus1 [MAXIMUMVALUEOFcpb_cnt];  // ue(v)
	unsigned  cpb_size_value_minus1 [MAXIMUMVALUEOFcpb_cnt];  // ue(v)
	unsigned  cbr_flag              [MAXIMUMVALUEOFcpb_cnt];  // u(1)
	unsigned  initial_cpb_removal_delay_length_minus1;          // u(5)
	unsigned  cpb_removal_delay_length_minus1;                  // u(5)
	unsigned  dpb_output_delay_length_minus1;                   // u(5)
	unsigned  time_offset_length;                               // u(5)
} hrd_parameters_t;


typedef struct
{
	BOOL      aspect_ratio_info_present_flag;                   // u(1)
	unsigned  aspect_ratio_idc;                               // u(8)
	unsigned  sar_width;                                    // u(16)
	unsigned  sar_height;                                   // u(16)
	BOOL      overscan_info_present_flag;                       // u(1)
	BOOL      overscan_appropriate_flag;                      // u(1)
	BOOL      video_signal_type_present_flag;                   // u(1)
	unsigned  video_format;                                   // u(3)
	BOOL      video_full_range_flag;                          // u(1)
	BOOL      colour_description_present_flag;                // u(1)
	unsigned  colour_primaries;                             // u(8)
	unsigned  transfer_characteristics;                     // u(8)
	unsigned  matrix_coefficients;                          // u(8)
	BOOL      chroma_location_info_present_flag;                // u(1)
	unsigned   chroma_sample_loc_type_top_field;               // ue(v)
	unsigned   chroma_sample_loc_type_bottom_field;            // ue(v)
	BOOL      timing_info_present_flag;                         // u(1)
	unsigned  num_units_in_tick;                              // u(32)
	unsigned  time_scale;                                     // u(32)
	BOOL      fixed_frame_rate_flag;                          // u(1)
	BOOL      nal_hrd_parameters_present_flag;                  // u(1)
	hrd_parameters_t nal_hrd_parameters;                      // hrd_paramters_t
	BOOL      vcl_hrd_parameters_present_flag;                  // u(1)
	hrd_parameters_t vcl_hrd_parameters;                      // hrd_paramters_t
	// if ((nal_hrd_parameters_present_flag || (vcl_hrd_parameters_present_flag))
	BOOL      low_delay_hrd_flag;                             // u(1)
	BOOL      pic_struct_present_flag;                        // u(1)
	BOOL      bitstream_restriction_flag;                       // u(1)
	BOOL      motion_vectors_over_pic_boundaries_flag;        // u(1)
	unsigned  max_bytes_per_pic_denom;                        // ue(v)
	unsigned  max_bits_per_mb_denom;                          // ue(v)
	unsigned  log2_max_mv_length_vertical;                    // ue(v)
	unsigned  log2_max_mv_length_horizontal;                  // ue(v)
	unsigned  num_reorder_frames;                             // ue(v)
	unsigned  max_dec_frame_buffering;                        // ue(v)
	int picture_structure;  // if pic_struct_present_flag=0, then picture_structure is zero
	int NumClockTs;	// it is based on picture_structre and it's default is 1
} vui_seq_parameters_t;

typedef struct mvc_vui_parameters_extension 
{
	unsigned vui_mvc_num_ops;									// ue(v)
	unsigned *vui_mvc_temporal_id;								// u(3)
	unsigned *vui_mvc_num_target_output_views;					// ue(v)
	unsigned **vui_mvc_view_id;									// ue(v)
	BOOL *vui_mvc_timing_info_present_flag;						// u(1)
	unsigned *vui_mvc_num_units_in_tick;						// u(32)
	unsigned *vui_mvc_time_scale;								// u(32)
	BOOL *vui_mvc_fixed_frame_rate_flag;						// u(1)
	BOOL *vui_mvc_nal_hrd_parameters_present_flag;				// u(1)
	hrd_parameters_t *nal_hrd_parameters;						// hrd_paramters_t
	BOOL *vui_mvc_vcl_hrd_parameters_present_flag;				// u(1)
	hrd_parameters_t *vcl_hrd_parameters;						// hrd_paramters_t
	BOOL *vui_mvc_low_delay_hrd_flag;							// u(1)
	BOOL *vui_mvc_pic_struct_present_flag;						// u(1)
}mvc_vui_parameters_extension;


#define MAXnum_slice_groups_minus1  8
typedef struct
{
	BOOL   Valid;                  // indicates the parameter set is valid
	unsigned  pic_parameter_set_id;                             // ue(v)
	unsigned  seq_parameter_set_id;                             // ue(v)
	BOOL   entropy_coding_mode_flag;                         // u(1)

	BOOL   transform_8x8_mode_flag;                          // u(1)

	BOOL   pic_scaling_matrix_present_flag;                  // u(1)
	int       pic_scaling_list_present_flag[8];                 // u(1)
	UCHAR       ScalingList4x4[6][16];                            // se(v)
	UCHAR       ScalingList8x8[2][64];                            // se(v)
	BOOL   UseDefaultScalingMatrix4x4Flag[6];
	BOOL   UseDefaultScalingMatrix8x8Flag[2];

	// if( pic_order_cnt_type < 2 )  in the sequence parameter set
	BOOL      pic_order_present_flag;                           // u(1)
	unsigned  num_slice_groups_minus1;                          // ue(v)
	unsigned  slice_group_map_type;                        // ue(v)
	// if( slice_group_map_type = = 0 )
	unsigned  run_length_minus1[MAXnum_slice_groups_minus1]; // ue(v)
	// else if( slice_group_map_type = = 2 )
	unsigned  top_left[MAXnum_slice_groups_minus1];         // ue(v)
	unsigned  bottom_right[MAXnum_slice_groups_minus1];     // ue(v)
	// else if( slice_group_map_type = = 3 || 4 || 5
	BOOL   slice_group_change_direction_flag;            // u(1)
	unsigned  slice_group_change_rate_minus1;               // ue(v)
	// else if( slice_group_map_type = = 6 )
	unsigned  num_slice_group_map_units_minus1;             // ue(v)
	unsigned  *slice_group_id;                              // complete MBAmap u(v)
	unsigned  num_ref_idx_l0_active_minus1;                     // ue(v)
	unsigned  num_ref_idx_l1_active_minus1;                     // ue(v)
	BOOL   weighted_pred_flag;                               // u(1)
	unsigned  weighted_bipred_idc;                              // u(2)
	int       pic_init_qp_minus26;                              // se(v)
	int       pic_init_qs_minus26;                              // se(v)
	int       chroma_qp_index_offset;                           // se(v)

	int       second_chroma_qp_index_offset;                    // se(v)

	BOOL   deblocking_filter_control_present_flag;           // u(1)
	BOOL   constrained_intra_pred_flag;                      // u(1)
	BOOL   redundant_pic_cnt_present_flag;                   // u(1)
} pic_parameter_set_rbsp_t;

typedef struct
{
	unsigned int viewId;
	unsigned int idrFlag;
	unsigned int temporalId;
	unsigned int anchorPicFlag;
	unsigned int interViewFlag;
	unsigned int priorityId;
	BOOL		 valid;
	BOOL		 bIsPrefixNALU;

} nalu_header_mvc_extension_t;


/* MVC related defines */
#define MAX_NUM_VIEWS	2
#define MAX_NUM_LEVELS	63
#define MAX_NUM_APPLICABLE_OPS	1023

#define MAXnum_ref_frames_in_pic_order_cnt_cycle  256
typedef struct
{
	BOOL   Valid;                  // indicates the parameter set is valid

	unsigned  profile_idc;                                      // u(8)
	BOOL   constrained_set0_flag;                            // u(1)
	BOOL   constrained_set1_flag;                            // u(1)
	BOOL   constrained_set2_flag;                            // u(1)
	BOOL   constrained_set3_flag;                            // u(1)
	BOOL   constrained_set4_flag;                            // u(1)
	unsigned  level_idc;                                        // u(8)
	unsigned  seq_parameter_set_id;                             // ue(v)
	unsigned  chroma_format_idc;                                // ue(v)

	BOOL  seq_scaling_matrix_present_flag;                   // u(1)
	int      seq_scaling_list_present_flag[8];                  // u(1)
	UCHAR      ScalingList4x4[6][16];                             // se(v)
	UCHAR      ScalingList8x8[2][64];                             // se(v)
	BOOL  UseDefaultScalingMatrix4x4Flag[6];
	BOOL  UseDefaultScalingMatrix8x8Flag[2];

	unsigned  bit_depth_luma_minus8;                            // ue(v)
	unsigned  bit_depth_chroma_minus8;                          // ue(v)

	unsigned  log2_max_frame_num_minus4;                        // ue(v)
	unsigned pic_order_cnt_type;
	// if( pic_order_cnt_type == 0 ) 
	unsigned log2_max_pic_order_cnt_lsb_minus4;                 // ue(v)
	// else if( pic_order_cnt_type == 1 )
	BOOL delta_pic_order_always_zero_flag;               // u(1)
	int     offset_for_non_ref_pic;                         // se(v)
	int     offset_for_top_to_bottom_field;                 // se(v)
	unsigned  num_ref_frames_in_pic_order_cnt_cycle;          // ue(v)
	// for( i = 0; i < num_ref_frames_in_pic_order_cnt_cycle; i++ )
	int   offset_for_ref_frame[MAXnum_ref_frames_in_pic_order_cnt_cycle];   // se(v)
	unsigned  num_ref_frames;                                   // ue(v)
	BOOL   gaps_in_frame_num_value_allowed_flag;             // u(1)
	unsigned  pic_width_in_mbs_minus1;                          // ue(v)
	unsigned  pic_height_in_map_units_minus1;                   // ue(v)
	BOOL   frame_mbs_only_flag;                              // u(1)
	// if( !frame_mbs_only_flag ) 
	BOOL   mb_adaptive_frame_field_flag;                   // u(1)
	BOOL   direct_8x8_inference_flag;                        // u(1)
	BOOL   frame_cropping_flag;                              // u(1)
	unsigned  frame_cropping_rect_left_offset;                // ue(v)
	unsigned  frame_cropping_rect_right_offset;               // ue(v)
	unsigned  frame_cropping_rect_top_offset;                 // ue(v)
	unsigned  frame_cropping_rect_bottom_offset;              // ue(v)
	BOOL   vui_parameters_present_flag;                      // u(1)
	vui_seq_parameters_t vui_seq_parameters;                  // vui_seq_parameters_t

	//MVC Extension
	BOOL mvc_vui_parameters_present_flag;		//u(1)
	mvc_vui_parameters_extension vui_parameters_ext;
	BOOL additional_extension2_flag;			//u(1)

	unsigned num_views;
	unsigned *view_id;
    unsigned *num_anchor_refs_l0;
	unsigned **anchor_refs_l0;
	unsigned *num_anchor_refs_l1;
	unsigned **anchor_refs_l1;
	unsigned *num_non_anchor_refs_l0;
	unsigned **non_anchor_refs_l0;
	unsigned *num_non_anchor_refs_l1;
	unsigned **non_anchor_refs_l1;

	unsigned num_level_values_signalled;	
	unsigned *level_idc_mvc;
	unsigned *num_applicable_ops;
	unsigned **applicable_op_temporal_id;
	unsigned **applicable_op_num_target_views_minus1;
	unsigned ***applicable_op_target_view_id;
	unsigned **applicable_op_num_views_minus1;

} seq_parameter_set_rbsp_t;

typedef enum
{
	SPS_CRITICAL_CHANGE     = 0,
	SPS_NO_CHANGE		   = 1,	
	SPS_SLIGHT_CHANGE      = 2
};

pic_parameter_set_rbsp_t *AllocPPS ();
seq_parameter_set_rbsp_t *AllocSPS ();

void FreePPS (pic_parameter_set_rbsp_t *pps);
void FreeSPS (seq_parameter_set_rbsp_t *sps);

int sps_is_equal(seq_parameter_set_rbsp_t *sps1, seq_parameter_set_rbsp_t *sps2);
int pps_is_equal(pic_parameter_set_rbsp_t *pps1, pic_parameter_set_rbsp_t *pps2);

#endif
