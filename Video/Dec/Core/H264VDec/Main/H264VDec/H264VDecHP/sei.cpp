/*!
************************************************************************
* \file  sei.c
*
* \brief
*    Functions to implement SEI messages
*
* \author
*    Main contributors (see contributors.h for copyright, address and affiliation details)
*    - Dong Tian        <tian@cs.tut.fi>
*    - Karsten Suehring <suehring@hhi.de>
************************************************************************
*/

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <malloc.h>

#include "global.h"
#include "memalloc.h"
#include "sei.h"
#include "vlc.h"
#include "header.h"
#include "mbuffer.h"
#include "parset.h"
#include "ATSCDec.h"
#include <queue>
#include <list>

#ifdef _COLLECT_PIC_
#define		stream_global		IMGPAR stream_global
#endif

// #define PRINT_BUFFERING_PERIOD_INFO    // uncomment to print buffering period SEI info
// #define PRINT_PCITURE_TIMING_INFO      // uncomment to print picture timing SEI info
// #define WRITE_MAP_IMAGE                // uncomment to write spare picture sei_map
// #define PRINT_SUBSEQUENCE_INFO         // uncomment to print sub-sequence SEI info
// #define PRINT_SUBSEQUENCE_LAYER_CHAR   // uncomment to print sub-sequence layer characteristics SEI info
// #define PRINT_SUBSEQUENCE_CHAR         // uncomment to print sub-sequence characteristics SEI info
// #define PRINT_SCENE_INFORMATION        // uncomment to print scene information SEI info
// #define PRINT_PAN_SCAN_RECT            // uncomment to print pan-scan rectangle SEI info
// #define PRINT_RECOVERY_POINT            // uncomment to print random access point SEI info
// #define PRINT_FILLER_PAYLOAD_INFO      // uncomment to print filler payload SEI info
// #define PRINT_DEC_REF_PIC_MARKING      // uncomment to print decoded picture buffer management repetition SEI info
// #define PRINT_RESERVED_INFO            // uncomment to print reserved SEI info
// #define PRINT_USER_DATA_UNREGISTERED_INFO          // uncomment to print unregistered user data SEI info
// #define PRINT_USER_DATA_REGISTERED_ITU_T_T35_INFO  // uncomment to print ITU-T T.35 user data SEI info
// #define PRINT_FULL_FRAME_FREEZE_INFO               // uncomment to print full-frame freeze SEI info
// #define PRINT_FULL_FRAME_FREEZE_RELEASE_INFO       // uncomment to print full-frame freeze release SEI info
// #define PRINT_FULL_FRAME_SNAPSHOT_INFO             // uncomment to print full-frame snapshot SEI info
// #define PRINT_PROGRESSIVE_REFINEMENT_END_INFO      // uncomment to print Progressive refinement segment start SEI info
// #define PRINT_PROGRESSIVE_REFINEMENT_END_INFO      // uncomment to print Progressive refinement segment end SEI info
// #define PRINT_MOTION_CONST_SLICE_GROUP_SET_INFO    // uncomment to print Motion-constrained slice group set SEI info

/*!
************************************************************************
*  \brief
*     Interpret the SEI rbsp
*  \param msg
*     a pointer that point to the sei message.
*  \param size
*     the size of the sei message
*  \param img
*     the image pointer
*    
************************************************************************
*/
CREL_RETURN InterpretSEIMessage PARGS2(byte* msg, int size)
{
	int payload_type = 0;
	int payload_size = 0;
	int offset = 1;
	byte tmp_byte;
	CREL_RETURN ret;

	int num_views = 1;
	int view_indexs[64];
	memset(view_indexs, 0, 64 * sizeof(int));

	do
	{
		// sei_message();
		payload_type = 0;
		tmp_byte = msg[offset++];
		while (tmp_byte == 0xFF)
		{
			payload_type += 255;
			tmp_byte = msg[offset++];
		}
		payload_type += tmp_byte;   // this is the last byte

		payload_size = 0;
		tmp_byte = msg[offset++];
		while (tmp_byte == 0xFF)
		{
			payload_size += 255;
			tmp_byte = msg[offset++];
		}
		payload_size += tmp_byte;   // this is the last byte

		//Terry: For PIP's sub bitstream, it may has no SPS before pasing SEI nalu during enable fast forward (FF) function.
#ifdef _COLLECT_PIC_
		if ((stream_global->m_active_sps_on_view[view_indexs[0]] == NULL) && (payload_type == SEI_PIC_TIMING || payload_type == SEI_DEC_REF_PIC_MARKING_REPETITION))
#else
		if ((active_sps.Valid == NULL) && (payload_type == SEI_PIC_TIMING || payload_type == SEI_DEC_REF_PIC_MARKING_REPETITION))
#endif
		{
			DEBUG_SHOW_SW_INFO("InterpretSEIMessage: stream_global->m_active_sps == NULL.");
			int i;
			seq_parameter_set_rbsp_t *sps = NULL;
			pic_parameter_set_rbsp_t *pps = NULL;
			for (i=0; i<256; i++)
			{
				if (PicParSet[i].Valid)
				{
					pps = &PicParSet[i];
					break;
				}
			}

			if (pps)
			{
				sps = &SeqParSet[pps->seq_parameter_set_id];

				if (sps->Valid)
				{
					ret = activate_global_sps ARGS2(sps, view_indexs[0]);
					if (FAILED(ret)) {
						return ret;
					}
					activate_global_pps ARGS2(pps, view_indexs[0]);	// Always CREL_OK
				}
				else
				{
					return CREL_ERROR_H264_UNDEFINED;
				}
			}
			else
			{
				return CREL_ERROR_H264_UNDEFINED;
			}
		}

		switch ( payload_type )     // sei_payload( type, size );
		{
		case  SEI_BUFFERING_PERIOD:
			for(int i = 0; i < num_views; i++)
				ret = interpret_buffering_period_info ARGS3( msg+offset, payload_size, view_indexs[i] );
			if (FAILED(ret)) {
				return ret;
			}
			break;
		case  SEI_PIC_TIMING:
			for(int i = 0; i < num_views; i++)
				interpret_picture_timing_info ARGS3( msg+offset, payload_size, view_indexs[i] );
			break;
		case  SEI_PAN_SCAN_RECT:
			interpret_pan_scan_rect_info ARGS2( msg+offset, payload_size );
			break;
		case  SEI_FILLER_PAYLOAD:
			interpret_filler_payload_info( msg+offset, payload_size );
			break;
		case  SEI_USER_DATA_REGISTERED_ITU_T_T35:
			interpret_user_data_registered_itu_t_t35_info( msg+offset, payload_size );
			break;
		case  SEI_USER_DATA_UNREGISTERED:
			interpret_user_data_unregistered_info ARGS2( msg+offset, payload_size );
			break;
		case  SEI_RECOVERY_POINT:
			interpret_recovery_point_info ARGS2( msg+offset, payload_size );
			break;
		case  SEI_DEC_REF_PIC_MARKING_REPETITION:
			for(int i = 0; i < num_views; i++)
				interpret_dec_ref_pic_marking_repetition_info ARGS3( msg+offset, payload_size, view_indexs[i]);
			break;
		case  SEI_SPARE_PIC:
			interpret_spare_pic ARGS2( msg+offset, payload_size );
			break;
		case  SEI_SCENE_INFO:
			interpret_scene_information ARGS2( msg+offset, payload_size );
			break;
		case  SEI_SUB_SEQ_INFO:
			interpret_subsequence_info ARGS2( msg+offset, payload_size );
			break;
		case  SEI_SUB_SEQ_LAYER_CHARACTERISTICS:
			interpret_subsequence_layer_characteristics_info ARGS2( msg+offset, payload_size );
			break;
		case  SEI_SUB_SEQ_CHARACTERISTICS:
			interpret_subsequence_characteristics_info ARGS2( msg+offset, payload_size );
			break;
		case  SEI_FULL_FRAME_FREEZE:
			interpret_full_frame_freeze_info( msg+offset, payload_size );
			break;
		case  SEI_FULL_FRAME_FREEZE_RELEASE:
			interpret_full_frame_freeze_release_info( msg+offset, payload_size );
			break;
		case  SEI_FULL_FRAME_SNAPSHOT:
			interpret_full_frame_snapshot_info ARGS2( msg+offset, payload_size );
			break;
		case  SEI_PROGRESSIVE_REFINEMENT_SEGMENT_START:
			interpret_progressive_refinement_end_info ARGS2( msg+offset, payload_size );
			break;
		case  SEI_PROGRESSIVE_REFINEMENT_SEGMENT_END:
			interpret_progressive_refinement_end_info ARGS2( msg+offset, payload_size );
			break;
		case  SEI_MOTION_CONSTRAINED_SLICE_GROUP_SET:
			interpret_motion_constrained_slice_group_set_info ARGS2( msg+offset, payload_size );
			break;
		case  SEI_MVC_SCALABLE_NESTING:
			interpret_mvc_scalable_nesting ARGS4(msg+offset, &payload_size, &num_views, view_indexs);
			break;
		default:
			interpret_reserved_info( msg+offset, payload_size );
			break;
		}
		offset += payload_size;

	} while(( msg[offset] != 0x80 ) && ((offset+1) < size));    // more_rbsp_data()  msg[offset] != 0x80
	// ignore the trailing bits rbsp_trailing_bits();

	//assert(msg[offset] == 0x80);      // this is the trailing bits
	//assert( offset+1 == size );
	if ((msg[offset] != 0x80)||( offset+1 != size )) {
		return CREL_WARNING_H264_STREAMERROR_LEVEL_3;
	}

	return CREL_OK;
}


/*!
************************************************************************
*  \brief
*     Interpret the spare picture SEI message
*  \param payload
*     a pointer that point to the sei payload
*  \param size
*     the size of the sei message
*  \param img
*     the image pointer
*    
************************************************************************
*/
void interpret_spare_pic PARGS2( byte* payload, int size)
{
	int i,x,y;
	DecodingEnvironment *dep;
	int bit0, bit1, bitc, no_bit0;
	int target_frame_num;
	int num_spare_pics;
	int delta_spare_frame_num, CandidateSpareFrameNum, SpareFrameNum = 0;
	int ref_area_indicator;

	int m, n, left, right, top, bottom,directx, directy;
	byte *sei_map;

#ifdef WRITE_MAP_IMAGE
	int symbol_size_in_bytes = 1; // HP restriction
	int  j, k, i0, j0, tmp, kk;
	char filename[20] = "map_dec.yuv";
	FILE *fp;
	imgpel** Y;
	static int old_pn=-1;
	static int first = 1;

	DEBUG_SHOW_SW_INFO("Spare picture SEI message\n");
#endif

	//assert( payload!=NULL);
	//assert( img!=NULL);
	if ((payload==NULL)||(img == NULL)) {
		return;
	}

	dep = AllocPartition ARGS1(1);
	dep->Dcodestrm   = payload;
	dep->Dbuffer     = 0;
	dep->Dbits_to_go = 0;

	target_frame_num = ue_v ("SEI: target_frame_num");

#ifdef WRITE_MAP_IMAGE
	DEBUG_SHOW_SW_INFO( "target_frame_num is %d\n", target_frame_num );
#endif

	num_spare_pics = 1 + ue_v ("SEI: num_spare_pics_minus1");

#ifdef WRITE_MAP_IMAGE
	DEBUG_SHOW_SW_INFO( "num_spare_pics is %d\n", num_spare_pics );
#endif

	sei_map = (byte *) _aligned_malloc(num_spare_pics*(IMGPAR height/16)*(IMGPAR width/16)*sizeof(byte), 16);

	for (i=0; i<num_spare_pics; i++)
	{
		if (i==0) 
		{
			CandidateSpareFrameNum = target_frame_num - 1;
			if ( CandidateSpareFrameNum < 0 ) CandidateSpareFrameNum = MAX_FN - 1;
		}
		else
			CandidateSpareFrameNum = SpareFrameNum;

		delta_spare_frame_num = ue_v ("SEI: delta_spare_frame_num");

		SpareFrameNum = CandidateSpareFrameNum - delta_spare_frame_num;
		if( SpareFrameNum < 0 )
			SpareFrameNum = MAX_FN + SpareFrameNum;

		ref_area_indicator = ue_v ("SEI: ref_area_indicator");

		switch ( ref_area_indicator )
		{
		case 0:   // The whole frame can serve as spare picture
			for (y=0; y<IMGPAR height/16; y++)
				for (x=0; x<IMGPAR width/16; x++)
					sei_map[(i*(IMGPAR height/16)+y)*(IMGPAR width/16)+x] = 0;
			break;
		case 1:   // The sei_map is not compressed
			for (y=0; y<IMGPAR height/16; y++)
				for (x=0; x<IMGPAR width/16; x++)
				{
					sei_map[(i*(IMGPAR height/16)+y)*(IMGPAR width/16)+x] = u_1 ("SEI: ref_mb_indicator");
				}
				break;
		case 2:   // The sei_map is compressed
			//!KS: could not check this function, description is unclear (as stated in Ed. Note)
			bit0 = 0;
			bit1 = 1;
			bitc = bit0;
			no_bit0 = -1;

			x = ( IMGPAR width/16 - 1 ) / 2;
			y = ( IMGPAR height/16 - 1 ) / 2;
			left = right = x;
			top = bottom = y;
			directx = 0;
			directy = 1;

			for (m=0; m<IMGPAR height/16; m++)
				for (n=0; n<IMGPAR width/16; n++)
				{

					if (no_bit0<0)
					{
						no_bit0 = ue_v ("SEI: zero_run_length");
					}
					if (no_bit0>0) sei_map[(i*(IMGPAR height/16)+y)*(IMGPAR width/16)+x] = bit0;
					else sei_map[(i*(IMGPAR height/16)+y)*(IMGPAR width/16)+x] = bit1;
					no_bit0--;

					// go to the next mb:
					if ( directx == -1 && directy == 0 )
					{
						if (x > left) x--;
						else if (x == 0)
						{
							y = bottom + 1;
							bottom++;
							directx = 1;
							directy = 0;
						}
						else if (x == left)
						{
							x--;
							left--;
							directx = 0;
							directy = 1;
						}
					}
					else if ( directx == 1 && directy == 0 )
					{
						if (x < right) x++;
						else if (x == IMGPAR width/16 - 1)
						{
							y = top - 1;
							top--;
							directx = -1;
							directy = 0;
						}
						else if (x == right)
						{
							x++;
							right++;
							directx = 0;
							directy = -1;
						}
					}
					else if ( directx == 0 && directy == -1 )
					{
						if ( y > top) y--;
						else if (y == 0)
						{
							x = left - 1;
							left--;
							directx = 0;
							directy = 1;
						}
						else if (y == top)
						{
							y--;
							top--;
							directx = -1;
							directy = 0;
						}
					}
					else if ( directx == 0 && directy == 1 )
					{
						if (y < bottom) y++;
						else if (y == IMGPAR height/16 - 1)
						{
							x = right+1;
							right++;
							directx = 0;
							directy = -1;
						}
						else if (y == bottom)
						{
							y++;
							bottom++;
							directx = 1;
							directy = 0;
						}
					}


				}
				break;
		default:
			DEBUG_SHOW_ERROR_INFO( "Wrong ref_area_indicator %d!\n", ref_area_indicator );
			break;
		}

	} // end of num_spare_pics

#ifdef WRITE_MAP_IMAGE
	// begin to write sei_map seq
	if ( old_pn != IMGPAR number )
	{
		old_pn = IMGPAR number;
		get_mem2Dpel(&Y, IMGPAR height, IMGPAR width);
		if (first)
		{
			fp = fopen( filename, "wb" );
			first = 0;
		}
		else
			fp = fopen( filename, "ab" );
		assert( fp != NULL );
		for (kk=0; kk<num_spare_pics; kk++)
		{
			for (i=0; i < IMGPAR height/16; i++)
				for (j=0; j < IMGPAR width/16; j++)
				{
					tmp=sei_map[(kk*(IMGPAR height/16)+i)*(IMGPAR width/16)+j]==0? 255 : 0; // HP restriction
					for (i0=0; i0<16; i0++)
						for (j0=0; j0<16; j0++)
							Y[i*16+i0][j*16+j0]=tmp;
				}

				// write the sei_map image
				for (i=0; i < IMGPAR height; i++)
					for (j=0; j < IMGPAR width; j++)
						fwrite(&(Y[i][j]), symbol_size_in_bytes, 1, p_out);

				for (k=0; k < 2; k++)
					for (i=0; i < IMGPAR height/2; i++)
						for (j=0; j < IMGPAR width/2; j++)
						{
							const unsigned char ct=128;
							fwrite(&(ct), symbol_size_in_bytes, 1, p_out); // HP restriction
						}
		}
		fclose( fp );
		free_mem2Dpel( Y );
	}
	// end of writing sei_map image
#undef WRITE_MAP_IMAGE
#endif

	_aligned_free ((void *) sei_map);

	FreePartition (dep);
}


/*!
************************************************************************
*  \brief
*     Interpret the Sub-sequence information SEI message
*  \param payload
*     a pointer that point to the sei payload
*  \param size
*     the size of the sei message
*  \param img
*     the image pointer
*    
************************************************************************
*/
void interpret_subsequence_info PARGS2( byte* payload, int size)
{
	DecodingEnvironment *dep;
	int sub_seq_layer_num, sub_seq_id, first_ref_pic_flag, leading_non_ref_pic_flag, last_pic_flag, 
		sub_seq_frame_num_flag, sub_seq_frame_num;

	dep = AllocPartition ARGS1(1);
	dep->Dcodestrm   = payload;
	dep->Dbuffer     = 0;
	dep->Dbits_to_go = 0;

	sub_seq_layer_num        = ue_v ("SEI: sub_seq_layer_num");
	sub_seq_id               = ue_v ("SEI: sub_seq_id");
	first_ref_pic_flag       = u_1 ("SEI: first_ref_pic_flag");
	leading_non_ref_pic_flag = u_1 ("SEI: leading_non_ref_pic_flag");
	last_pic_flag            = u_1 ("SEI: last_pic_flag");
	sub_seq_frame_num_flag   = u_1 ("SEI: sub_seq_frame_num_flag");
	if (sub_seq_frame_num_flag)
	{
		sub_seq_frame_num        = ue_v ("SEI: sub_seq_frame_num");
	}

#ifdef PRINT_SUBSEQUENCE_INFO
	DEBUG_INFO("Sub-sequence information SEI message\n");
	DEBUG_INFO("sub_seq_layer_num        = %d\n", sub_seq_layer_num );
	DEBUG_INFO("sub_seq_id               = %d\n", sub_seq_id);
	DEBUG_INFO("first_ref_pic_flag       = %d\n", first_ref_pic_flag);
	DEBUG_INFO("leading_non_ref_pic_flag = %d\n", leading_non_ref_pic_flag);
	DEBUG_INFO("last_pic_flag            = %d\n", last_pic_flag);
	DEBUG_INFO("sub_seq_frame_num_flag   = %d\n", sub_seq_frame_num_flag);
	if (sub_seq_frame_num_flag)
	{
		DEBUG_INFO("sub_seq_frame_num        = %d\n", sub_seq_frame_num);
	}
#endif

	FreePartition (dep);
#ifdef PRINT_SUBSEQUENCE_INFO
#undef PRINT_SUBSEQUENCE_INFO
#endif
}

/*!
************************************************************************
*  \brief
*     Interpret the Sub-sequence layer characteristics SEI message
*  \param payload
*     a pointer that point to the sei payload
*  \param size
*     the size of the sei message
*  \param img
*     the image pointer
*    
************************************************************************
*/
void interpret_subsequence_layer_characteristics_info PARGS2( byte* payload, int size)
{
	DecodingEnvironment *dep;
	long num_sub_layers, accurate_statistics_flag, average_bit_rate, average_frame_rate;
	int i;

	dep = AllocPartition ARGS1(1);
	dep->Dcodestrm   = payload;
	dep->Dbuffer     = 0;
	dep->Dbits_to_go = 0;

	num_sub_layers = 1 + ue_v ("SEI: num_sub_layers_minus1");

#ifdef PRINT_SUBSEQUENCE_LAYER_CHAR
	DEBUG_INFO("Sub-sequence layer characteristics SEI message\n");
	DEBUG_INFO("num_sub_layers_minus1 = %d\n", num_sub_layers - 1);
#endif

	for (i=0; i<num_sub_layers; i++)
	{
		accurate_statistics_flag = u_1 (   "SEI: accurate_statistics_flag");
		average_bit_rate         = u_v (16,"SEI: average_bit_rate");
		average_frame_rate       = u_v (16,"SEI: average_frame_rate");

#ifdef PRINT_SUBSEQUENCE_LAYER_CHAR
		DEBUG_INFO("layer %d: accurate_statistics_flag = %ld \n", i, accurate_statistics_flag);
		DEBUG_INFO("layer %d: average_bit_rate         = %ld \n", i, average_bit_rate);
		DEBUG_INFO("layer %d: average_frame_rate       = %ld \n", i, average_frame_rate);
#endif
	}
	FreePartition (dep);
}


/*!
************************************************************************
*  \brief
*     Interpret the Sub-sequence characteristics SEI message
*  \param payload
*     a pointer that point to the sei payload
*  \param size
*     the size of the sei message
*  \param img
*     the image pointer
*    
************************************************************************
*/
void interpret_subsequence_characteristics_info PARGS2( byte* payload, int size)
{
	DecodingEnvironment *dep;
	int i;
	int sub_seq_layer_num, sub_seq_id, duration_flag, average_rate_flag, accurate_statistics_flag;
	unsigned long sub_seq_duration, average_bit_rate, average_frame_rate;
	int num_referenced_subseqs, ref_sub_seq_layer_num, ref_sub_seq_id, ref_sub_seq_direction;

	dep = AllocPartition ARGS1(1);
	dep->Dcodestrm   = payload;
	dep->Dbuffer     = 0;
	dep->Dbits_to_go = 0;

	sub_seq_layer_num = ue_v ("SEI: sub_seq_layer_num");
	sub_seq_id        = ue_v ("SEI: sub_seq_id");
	duration_flag     = u_1 ("SEI: duration_flag");

#ifdef PRINT_SUBSEQUENCE_CHAR
	DEBUG_INFO("Sub-sequence characteristics SEI message\n");
	DEBUG_INFO("sub_seq_layer_num = %d\n", sub_seq_layer_num );
	DEBUG_INFO("sub_seq_id        = %d\n", sub_seq_id);
	DEBUG_INFO("duration_flag     = %d\n", duration_flag);
#endif

	if ( duration_flag )
	{
		sub_seq_duration = u_v (32, "SEI: duration_flag");
#ifdef PRINT_SUBSEQUENCE_CHAR
		DEBUG_INFO("sub_seq_duration = %ld\n", sub_seq_duration);
#endif
	}

	average_rate_flag = u_1 ("SEI: average_rate_flag");

#ifdef PRINT_SUBSEQUENCE_CHAR
	DEBUG_INFO("average_rate_flag = %d\n", average_rate_flag);
#endif

	if ( average_rate_flag )
	{
		accurate_statistics_flag = u_1 (    "SEI: accurate_statistics_flag");
		average_bit_rate         = u_v (16, "SEI: average_bit_rate");
		average_frame_rate       = u_v (16, "SEI: average_frame_rate");

#ifdef PRINT_SUBSEQUENCE_CHAR
		DEBUG_INFO("accurate_statistics_flag = %d\n", accurate_statistics_flag);
		DEBUG_INFO("average_bit_rate         = %ld\n", average_bit_rate);
		DEBUG_INFO("average_frame_rate       = %ld\n", average_frame_rate);
#endif
	}

	num_referenced_subseqs  = ue_v ("SEI: num_referenced_subseqs");

#ifdef PRINT_SUBSEQUENCE_CHAR
	DEBUG_INFO("num_referenced_subseqs = %d\n", num_referenced_subseqs);
#endif

	for (i=0; i<num_referenced_subseqs; i++)
	{
		ref_sub_seq_layer_num  = ue_v ("SEI: ref_sub_seq_layer_num");
		ref_sub_seq_id         = ue_v ("SEI: ref_sub_seq_id");
		ref_sub_seq_direction  = u_1 ("SEI: ref_sub_seq_direction");

#ifdef PRINT_SUBSEQUENCE_CHAR
		DEBUG_INFO("ref_sub_seq_layer_num = %d\n", ref_sub_seq_layer_num);
		DEBUG_INFO("ref_sub_seq_id        = %d\n", ref_sub_seq_id);
		DEBUG_INFO("ref_sub_seq_direction = %d\n", ref_sub_seq_direction);
#endif
	}

	FreePartition (dep);
#ifdef PRINT_SUBSEQUENCE_CHAR
#undef PRINT_SUBSEQUENCE_CHAR
#endif
}


/*!
************************************************************************
*  \brief
*     Interpret the Scene information SEI message
*  \param payload
*     a pointer that point to the sei payload
*  \param size
*     the size of the sei message
*  \param img
*     the image pointer
*    
************************************************************************
*/
void interpret_scene_information PARGS2( byte* payload, int size)
{
	DecodingEnvironment *dep;
	int scene_id, scene_transition_type, second_scene_id;

	dep = AllocPartition ARGS1(1);
	dep->Dcodestrm   = payload;
	dep->Dbuffer     = 0;
	dep->Dbits_to_go = 0;

	scene_id              = ue_v ("SEI: scene_id");
	scene_transition_type = ue_v ("SEI: scene_transition_type");
	if ( scene_transition_type > 3 )
	{
		second_scene_id     = ue_v ("SEI: scene_transition_type");
	}

#ifdef PRINT_SCENE_INFORMATION
	DEBUG_INFO("Scene information SEI message\n");
	DEBUG_INFO("scene_transition_type = %d\n", scene_transition_type);
	DEBUG_INFO("scene_id              = %d\n", scene_id);
	if ( scene_transition_type > 3 )
	{
		DEBUG_INFO("second_scene_id       = %d\n", second_scene_id);
	}
#endif
	FreePartition (dep);
#ifdef PRINT_SCENE_INFORMATION
#undef PRINT_SCENE_INFORMATION
#endif
}


/*!
************************************************************************
*  \brief
*     Interpret the Filler payload SEI message
*  \param payload
*     a pointer that point to the sei payload
*  \param size
*     the size of the sei message
*  \param img
*     the image pointer
*    
************************************************************************
*/
void interpret_filler_payload_info( byte* payload, int size)
{
	int payload_cnt = 0;

	while (payload_cnt<size)
	{
		if (payload[payload_cnt] == 0xFF)
		{
			payload_cnt++;
		} else {
			break;
		}
	}


#ifdef PRINT_FILLER_PAYLOAD_INFO
	DEBUG_INFO("Filler payload SEI message\n");
	if (payload_cnt==size)
	{
		DEBUG_INFO("read %d bytes of filler payload\n", payload_cnt);
	}
	else
	{
		DEBUG_INFO("error reading filler payload: not all bytes are 0xFF (%d of %d)\n", payload_cnt, size);
	}
#endif

#ifdef PRINT_FILLER_PAYLOAD_INFO
#undef PRINT_FILLER_PAYLOAD_INFO
#endif
}

void interpret_offset_metadata PARGS2(byte* payload, int size)
{
	std::list<H264_Offset_Metadata_Control> *pListOffsetMetadata = (std::list<H264_Offset_Metadata_Control> *)stream_global->m_pListOffsetMetadata;
	BYTE marker_bit = (BYTE)((payload[0] & 0x80)>>7);

	int frame_rate = payload[0] & 0x0F;

	int frame_rate1000;
	switch(frame_rate)
	{
	case 1:
		frame_rate1000 = 23976;
		break;
	case 2:
		frame_rate1000 = 24000;
		break;
	case 3:
		frame_rate1000 = 25000;
		break;
	case 4:
		frame_rate1000 = 29970;
		break;
	case 6:
		frame_rate1000 = 50000;
		break;
	case 7:
		frame_rate1000 = 59940;
		break;
	default:
		frame_rate1000 = 29970;
		break;
	}

	unsigned __int64 pts = (unsigned __int64)((payload[1] & 0x07)<<30) + (unsigned __int64)((payload[2] & 0x7F)<<23) + 
		(unsigned __int64)((payload[3] & 0xFF)<<15) + (unsigned __int64)((payload[4] & 0x7F)<<8) + 
		(unsigned __int64)((payload[5] & 0xFF));

	marker_bit &= (BYTE)((payload[2] & 0x80)>>7);
	marker_bit &= (BYTE)((payload[4] & 0x80)>>7);
	marker_bit &= (BYTE)((payload[6] & 0x80)>>7);

	int number_of_offset_sequences = payload[6] & 0x3F;
	int number_of_displayed_frames_in_GOP = payload[7] & 0xFF;

	marker_bit &= (BYTE)((payload[8] & 0x80)>>7);

	if(marker_bit != 1)
	{
		DEBUG_INFO("[ERROR][Offset_MetaData] The offset meta_data marker_bit isn't 1");
		return;
	}

	BYTE **Plane_offset_direction = (BYTE **) malloc(number_of_offset_sequences * sizeof(BYTE*));
	BYTE **Plane_offset_value = (BYTE **) malloc(number_of_offset_sequences * sizeof(BYTE*));

	for(int i=0; i<number_of_offset_sequences; i++)
	{
		Plane_offset_direction[i] = (BYTE *) malloc(number_of_displayed_frames_in_GOP * sizeof(BYTE));
		Plane_offset_value[i] = (BYTE *) malloc(number_of_displayed_frames_in_GOP * sizeof(BYTE));
	}

	int iOffset = 10; // The offset direction and value starts at 11th bytes.

	DEBUG_INFO("[H264INFO][Offset_MetaData] frame_rate mode: %d, offset sequences: %d, displayed frames in GOP: %d", frame_rate, number_of_offset_sequences, number_of_displayed_frames_in_GOP);
	for(int offset_sequence_id = 0; offset_sequence_id < number_of_offset_sequences; offset_sequence_id++)
	{
		for(int i = 0; i < number_of_displayed_frames_in_GOP; i++)
		{
			if(iOffset>size)
			{
				DEBUG_INFO("[ERROR][Offset_MetaData] The offset metadata size is overflow");
				return;
			}

			Plane_offset_direction[offset_sequence_id][i] = (BYTE)((payload[iOffset] & 0x80)>>7);
			Plane_offset_value[offset_sequence_id][i] = payload[iOffset] & 0x7F;
			iOffset++;
		}
	}

	EnterCriticalSection( &stream_global->m_csOffsetMetadata);
	for(int i = 0; i < number_of_displayed_frames_in_GOP; i++)
	{
		H264_Offset_Metadata_Control offset_metadata_control;
		ZeroMemory(&offset_metadata_control, sizeof(H264_Offset_Metadata_Control));

		offset_metadata_control.meta_data.Plane_offset_direction = (BYTE *)malloc(number_of_offset_sequences * sizeof(BYTE));
		offset_metadata_control.meta_data.Plane_offset_value = (BYTE *)malloc(number_of_offset_sequences * sizeof(BYTE));

		DEBUG_INFO("[H264INFO][Offset_MetaData] Index: %d", i);
		for(int offset_sequence_id = 0; offset_sequence_id < number_of_offset_sequences; offset_sequence_id++)
		{
			offset_metadata_control.meta_data.Plane_offset_direction[offset_sequence_id] = Plane_offset_direction[offset_sequence_id][i];
			offset_metadata_control.meta_data.Plane_offset_value[offset_sequence_id] = Plane_offset_value[offset_sequence_id][i];
			DEBUG_INFO("[H264INFO][Offset_MetaData] Sequence id: %d, offset direction: %d, offset value: %d", offset_sequence_id, offset_metadata_control.meta_data.Plane_offset_direction[offset_sequence_id], offset_metadata_control.meta_data.Plane_offset_value[offset_sequence_id]);
		}

		int pts_offset = i * (double)((double)(H264_FREQ_90KHZ * 1000)/(double)(frame_rate1000));
		offset_metadata_control.meta_data.pts = pts + (unsigned __int64)pts_offset;

		offset_metadata_control.meta_data.number_of_offset_sequences = number_of_offset_sequences;

		DEBUG_INFO("[H264INFO][Offset_MetaData] Push pts: %I64d", offset_metadata_control.meta_data.pts);

		pListOffsetMetadata->push_back(offset_metadata_control);
	}
	LeaveCriticalSection( &stream_global->m_csOffsetMetadata);

	for(int i=0; i<number_of_offset_sequences; i++)
	{
		free(Plane_offset_direction[i]);
		free(Plane_offset_value[i]);
	}

	free(Plane_offset_direction);
	free(Plane_offset_value);
}


/*!
************************************************************************
*  \brief
*     Interpret the User data unregistered SEI message
*  \param payload
*     a pointer that point to the sei payload
*  \param size
*     the size of the sei message
*  \param img
*     the image pointer
*    
************************************************************************
*/
void interpret_user_data_unregistered_info PARGS2( byte* payload, int size)
{
	int offset = 0;

#ifdef PRINT_USER_DATA_UNREGISTERED_INFO
	DEBUG_INFO("User data unregistered SEI message\n");
	DEBUG_INFO("uuid_iso_11578 = 0x");
#endif
	//assert (size>=16);
	if (size<16) {
		return;
	}

	for (offset = 0; offset < 16; offset++)
	{
#ifdef PRINT_USER_DATA_UNREGISTERED_INFO
		DEBUG_INFO("%02x",payload[offset]);
#endif
	}

#ifdef PRINT_USER_DATA_UNREGISTERED_INFO
	DEBUG_INFO("\n");
#endif

	if( offset<size )
	{
		if( *( (DWORD*) (payload+offset)) == 0x444D464F )
		{
			interpret_offset_metadata ARGS2(payload+(offset+4), size-(offset+4));
			return;
		}
		

		if ( *( (DWORD*) (payload+offset))!=0x34394147 )
		{
			DEBUG_INFO("[H264INFO] This unregistered user data TypeIndicator is %d!\n", *( (DWORD*) (payload+offset)));
			if ( *( (DWORD*) (payload+offset))==0x44494c43 )
			{
				DEBUG_INFO("[H264INFO] This unregistered user data is Extended-Gamut YCC Colour Space!\n");
				memset((void*)g_pbYCCBuffer, 0, sizeof(g_pbYCCBuffer));
				memcpy((void*)g_pbYCCBuffer, (void*)(payload+offset+4), 12);
				g_dwYCCBufferLen = size-(offset+4);
			}
			return; //not cc block.
		}
		if (size>LINE21BUF_SIZE)
			return; //content issue, unbelievabel cc length.

		g_pCCDec->DecodeLine21Data( payload, size );
	}

#ifdef PRINT_USER_DATA_UNREGISTERED_INFO
#undef PRINT_USER_DATA_UNREGISTERED_INFO
#endif
}

/*!
************************************************************************
*  \brief
*     Interpret the User data registered by ITU-T T.35 SEI message
*  \param payload
*     a pointer that point to the sei payload
*  \param size
*     the size of the sei message
*  \param img
*     the image pointer
*    
************************************************************************
*/
void interpret_user_data_registered_itu_t_t35_info( byte* payload, int size)
{
	int offset = 0;
	byte itu_t_t35_country_code, itu_t_t35_country_code_extension_byte, payload_byte;

	itu_t_t35_country_code = payload[offset];
	offset++;
#ifdef PRINT_USER_DATA_REGISTERED_ITU_T_T35_INFO
	DEBUG_INFO("User data registered by ITU-T T.35 SEI message\n");
	DEBUG_INFO(" itu_t_t35_country_code = %d \n", itu_t_t35_country_code);
#endif
	if(itu_t_t35_country_code == 0xFF) 
	{
		itu_t_t35_country_code_extension_byte = payload[offset];
		offset++;
#ifdef PRINT_USER_DATA_REGISTERED_ITU_T_T35_INFO
		DEBUG_INFO(" ITU_T_T35_COUNTRTY_CODE_EXTENSION_BYTE %d \n", itu_t_t35_country_code_extension_byte);
#endif
	}
	while (offset < size)
	{
		payload_byte = payload[offset];
		offset ++;
#ifdef PRINT_USER_DATA_REGISTERED_ITU_T_T35_INFO
		DEBUG_INFO("itu_t_t35 payload_byte = %d\n", payload_byte);
#endif
	}
#ifdef PRINT_USER_DATA_REGISTERED_ITU_T_T35_INFO
#undef PRINT_USER_DATA_REGISTERED_ITU_T_T35_INFO
#endif
}


/*!
************************************************************************
*  \brief
*     Interpret the Pan scan rectangle SEI message
*  \param payload
*     a pointer that point to the sei payload
*  \param size
*     the size of the sei message
*  \param img
*     the image pointer
*    
************************************************************************
*/
void interpret_pan_scan_rect_info PARGS2( byte* payload, int size)
{
	int pan_scan_rect_cancel_flag;
	int pan_scan_cnt_minus1, i;
	int pan_scan_rect_repetition_period;
	int pan_scan_rect_id, pan_scan_rect_left_offset, pan_scan_rect_right_offset;
	int pan_scan_rect_top_offset, pan_scan_rect_bottom_offset;

	DecodingEnvironment *dep;

	dep = AllocPartition ARGS1(1);
	dep->Dcodestrm   = payload;
	dep->Dbuffer     = 0;
	dep->Dbits_to_go = 0;

	pan_scan_rect_id = ue_v ("SEI: pan_scan_rect_id");
	pan_scan_rect_cancel_flag = u_1("SEI: pan_scan_rect_cancel_flag");

	if (!pan_scan_rect_cancel_flag) {
		pan_scan_cnt_minus1 = ue_v("SEI: pan_scan_cnt_minus1");
		if(pan_scan_cnt_minus1>=0 && pan_scan_cnt_minus1<=2)
		{
			for (i = 0; i <= pan_scan_cnt_minus1; i++) {

				pan_scan_rect_left_offset   = se_v ("SEI: pan_scan_rect_left_offset");
				pan_scan_rect_right_offset  = se_v ("SEI: pan_scan_rect_right_offset");
				pan_scan_rect_top_offset    = se_v ("SEI: pan_scan_rect_top_offset");
				pan_scan_rect_bottom_offset = se_v ("SEI: pan_scan_rect_bottom_offset");

#ifdef PRINT_PAN_SCAN_RECT
				DEBUG_SHOW_SW_INFO("Pan scan rectangle SEI message %d/%d\n", i, pan_scan_cnt_minus1);
				DEBUG_INFO("pan_scan_rect_id            = %d\n", pan_scan_rect_id);
				DEBUG_INFO("pan_scan_rect_left_offset   = %d\n", pan_scan_rect_left_offset);
				DEBUG_INFO("pan_scan_rect_right_offset  = %d\n", pan_scan_rect_right_offset);
				DEBUG_INFO("pan_scan_rect_top_offset    = %d\n", pan_scan_rect_top_offset);
				DEBUG_INFO("pan_scan_rect_bottom_offset = %d\n", pan_scan_rect_bottom_offset);
#endif
			}
		}
		pan_scan_rect_repetition_period = ue_v("SEI: pan_scan_rect_repetition_period");
	}
	FreePartition (dep);
#ifdef PRINT_PAN_SCAN_RECT
#undef PRINT_PAN_SCAN_RECT
#endif
}


/*!
************************************************************************
*  \brief
*     Interpret the Random access point SEI message
*  \param payload
*     a pointer that point to the sei payload
*  \param size
*     the size of the sei message
*  \param img
*     the image pointer
*    
************************************************************************
*/
void interpret_recovery_point_info PARGS2( byte* payload, int size)
{
	int recovery_frame_cnt, exact_match_flag, broken_link_flag, changing_slice_group_idc;

	DecodingEnvironment *dep;

	dep = AllocPartition ARGS1(1);
	dep->Dcodestrm   = payload;
	dep->Dbuffer     = 0;
	dep->Dbits_to_go = 0;

	recovery_frame_cnt       = ue_v (    "SEI: recovery_frame_cnt");
	exact_match_flag         = u_1 (    "SEI: exact_match_flag");
	broken_link_flag         = u_1 (    "SEI: broken_link_flag");
	changing_slice_group_idc = u_v ( 2, "SEI: changing_slice_group_idc");

#ifdef PRINT_RECOVERY_POINT
	DEBUG_INFO("Recovery point SEI message\n");
	DEBUG_INFO("recovery_frame_cnt       = %d\n", recovery_frame_cnt);
	DEBUG_INFO("exact_match_flag         = %d\n", exact_match_flag);
	DEBUG_INFO("broken_link_flag         = %d\n", broken_link_flag);
	DEBUG_INFO("changing_slice_group_idc = %d\n", changing_slice_group_idc);
#endif
	FreePartition (dep);
#ifdef PRINT_RECOVERY_POINT
#undef PRINT_RECOVERY_POINT
#endif
}


/*!
************************************************************************
*  \brief
*     Interpret the Decoded Picture Buffer Management Repetition SEI message
*  \param payload
*     a pointer that point to the sei payload
*  \param size
*     the size of the sei message
*  \param img
*     the image pointer
*    
************************************************************************
*/
void interpret_dec_ref_pic_marking_repetition_info PARGS3( byte* payload, int size, unsigned int view_index)
{
	int original_idr_flag, original_frame_num;
	int original_field_pic_flag;

	DecRefPicMarking_t *tmp_drpm;

	DecRefPicMarking_t *old_drpm;
	int old_idr_flag , old_no_output_of_prior_pics_flag, old_long_term_reference_flag , old_adaptive_ref_pic_buffering_flag;

	DecodingEnvironment *dep;

#if !defined(_COLLECT_PIC_)
	seq_parameter_set_rbsp_t *sps = &active_sps;
#else
	seq_parameter_set_rbsp_t *sps = stream_global->m_active_sps_on_view[view_index];
#endif

	dep = AllocPartition ARGS1(1);
	dep->Dcodestrm   = payload;
	dep->Dbuffer     = 0;
	dep->Dbits_to_go = 0;

	original_idr_flag     = u_1 (    "SEI: original_idr_flag");
	original_frame_num    = ue_v (    "SEI: original_frame_num");
	if(!sps->frame_mbs_only_flag)
	{
		if(original_field_pic_flag = u_1(    "SEI: original_field_pic_flag"))
			original_field_pic_flag = u_1(    "SEI: original_field_pic_flag");
	}

#ifdef PRINT_DEC_REF_PIC_MARKING
	DEBUG_INFO("Decoded Picture Buffer Management Repetition SEI message\n");
	DEBUG_INFO("original_idr_flag       = %d\n", original_idr_flag);
	DEBUG_INFO("original_frame_num      = %d\n", original_frame_num);
#endif

	// we need to save everything that is probably overwritten in dec_ref_pic_marking()
	old_drpm = IMGPAR dec_ref_pic_marking_buffer;
	old_idr_flag = IMGPAR idr_flag;

	old_no_output_of_prior_pics_flag = IMGPAR no_output_of_prior_pics_flag;
	old_long_term_reference_flag = IMGPAR long_term_reference_flag;
	old_adaptive_ref_pic_buffering_flag = IMGPAR adaptive_ref_pic_buffering_flag;

	// set new initial values
	IMGPAR idr_flag = original_idr_flag;
	IMGPAR dec_ref_pic_marking_buffer = NULL;

	dec_ref_pic_marking ARGS0();

	// print out decoded values
#ifdef PRINT_DEC_REF_PIC_MARKING
	if (IMGPAR idr_flag)
	{
		DEBUG_INFO("no_output_of_prior_pics_flag = %d\n", IMGPAR no_output_of_prior_pics_flag);
		DEBUG_INFO("long_term_reference_flag     = %d\n", IMGPAR long_term_reference_flag);
	}
	else
	{
		DEBUG_INFO("adaptive_ref_pic_buffering_flag  = %d\n", IMGPAR adaptive_ref_pic_buffering_flag);
		if (IMGPAR adaptive_ref_pic_buffering_flag)
		{
			tmp_drpm=IMGPAR dec_ref_pic_marking_buffer;
			while (tmp_drpm != NULL)
			{
				DEBUG_INFO("memory_management_control_operation  = %d\n", tmp_drpm->memory_management_control_operation);

				if ((tmp_drpm->memory_management_control_operation==1)||(tmp_drpm->memory_management_control_operation==3)) 
				{
					DEBUG_INFO("difference_of_pic_nums_minus1        = %d\n", tmp_drpm->difference_of_pic_nums_minus1);
				}
				if (tmp_drpm->memory_management_control_operation==2)
				{
					DEBUG_INFO("long_term_pic_num                    = %d\n", tmp_drpm->long_term_pic_num);
				}
				if ((tmp_drpm->memory_management_control_operation==3)||(tmp_drpm->memory_management_control_operation==6))
				{
					DEBUG_INFO("long_term_frame_idx                  = %d\n", tmp_drpm->long_term_frame_idx);
				}
				if (tmp_drpm->memory_management_control_operation==4)
				{
					DEBUG_INFO("max_long_term_pic_idx_plus1          = %d\n", tmp_drpm->max_long_term_frame_idx_plus1);
				}
				tmp_drpm = tmp_drpm->Next;
			}

		}
	}
#endif

	while (IMGPAR dec_ref_pic_marking_buffer)
	{
		tmp_drpm = IMGPAR dec_ref_pic_marking_buffer->Next;
		_aligned_free (IMGPAR dec_ref_pic_marking_buffer);
		IMGPAR dec_ref_pic_marking_buffer = tmp_drpm;
	}

	// restore old values in img
	IMGPAR dec_ref_pic_marking_buffer = old_drpm;
	IMGPAR idr_flag = old_idr_flag;
	IMGPAR no_output_of_prior_pics_flag = old_no_output_of_prior_pics_flag;
	IMGPAR long_term_reference_flag = old_long_term_reference_flag;
	IMGPAR adaptive_ref_pic_buffering_flag = old_adaptive_ref_pic_buffering_flag;

	FreePartition (dep);
#ifdef PRINT_DEC_REF_PIC_MARKING
#undef PRINT_DEC_REF_PIC_MARKING
#endif
}

/*!
************************************************************************
*  \brief
*     Interpret the Full-frame freeze SEI message
*  \param payload
*     a pointer that point to the sei payload
*  \param size
*     the size of the sei message
*  \param img
*     the image pointer
*    
************************************************************************
*/
void interpret_full_frame_freeze_info( byte* payload, int size)
{
#ifdef PRINT_FULL_FRAME_FREEZE_INFO
	DEBUG_INFO("Full-frame freeze SEI message\n");
	if (size)
	{
		DEBUG_INFO("payload size of this message should be zero, but is %d bytes.\n", size);
	}
#endif

#ifdef PRINT_FULL_FRAME_FREEZE_INFO
#undef PRINT_FULL_FRAME_FREEZE_INFO
#endif
}


/*!
************************************************************************
*  \brief
*     Interpret the Full-frame freeze release SEI message
*  \param payload
*     a pointer that point to the sei payload
*  \param size
*     the size of the sei message
*  \param img
*     the image pointer
*    
************************************************************************
*/
void interpret_full_frame_freeze_release_info( byte* payload, int size)
{
#ifdef PRINT_FULL_FRAME_FREEZE_RELEASE_INFO
	DEBUG_INFO("Full-frame freeze release SEI message\n");
	if (size)
	{
		DEBUG_INFO("payload size of this message should be zero, but is %d bytes.\n", size);
	}
#endif

#ifdef PRINT_FULL_FRAME_FREEZE_RELEASE_INFO
#undef PRINT_FULL_FRAME_FREEZE_RELEASE_INFO
#endif
}

/*!
************************************************************************
*  \brief
*     Interpret the Full-frame snapshot SEI message
*  \param payload
*     a pointer that point to the sei payload
*  \param size
*     the size of the sei message
*  \param img
*     the image pointer
*    
************************************************************************
*/
void interpret_full_frame_snapshot_info PARGS2( byte* payload, int size)
{
	int snapshot_id;

	DecodingEnvironment *dep;

	dep = AllocPartition ARGS1(1);
	dep->Dcodestrm   = payload;
	dep->Dbuffer     = 0;
	dep->Dbits_to_go = 0;

	snapshot_id = ue_v ("SEI: snapshot_id");

#ifdef PRINT_FULL_FRAME_SNAPSHOT_INFO
	DEBUG_INFO("Full-frame snapshot SEI message\n");
	DEBUG_INFO("snapshot_id = %d\n", snapshot_id);
#endif
	FreePartition (dep);
#ifdef PRINT_FULL_FRAME_SNAPSHOT_INFO
#undef PRINT_FULL_FRAME_SNAPSHOT_INFO
#endif
}

/*!
************************************************************************
*  \brief
*     Interpret the Progressive refinement segment start SEI message
*  \param payload
*     a pointer that point to the sei payload
*  \param size
*     the size of the sei message
*  \param img
*     the image pointer
*    
************************************************************************
*/
void interpret_progressive_refinement_start_info PARGS2( byte* payload, int size)
{
	int progressive_refinement_id, num_refinement_steps_minus1;

	DecodingEnvironment *dep;

	dep = AllocPartition ARGS1(1);
	dep->Dcodestrm   = payload;
	dep->Dbuffer     = 0;
	dep->Dbits_to_go = 0;

	progressive_refinement_id   = ue_v ("SEI: progressive_refinement_id");
	num_refinement_steps_minus1 = ue_v ("SEI: num_refinement_steps_minus1");

#ifdef PRINT_PROGRESSIVE_REFINEMENT_START_INFO
	DEBUG_INFO("Progressive refinement segment start SEI message\n");
	DEBUG_INFO("progressive_refinement_id   = %d\n", progressive_refinement_id);
	DEBUG_INFO("num_refinement_steps_minus1 = %d\n", num_refinement_steps_minus1);
#endif
	FreePartition (dep);
#ifdef PRINT_PROGRESSIVE_REFINEMENT_START_INFO
#undef PRINT_PROGRESSIVE_REFINEMENT_START_INFO
#endif
}


/*!
************************************************************************
*  \brief
*     Interpret the Progressive refinement segment end SEI message
*  \param payload
*     a pointer that point to the sei payload
*  \param size
*     the size of the sei message
*  \param img
*     the image pointer
*    
************************************************************************
*/
void interpret_progressive_refinement_end_info PARGS2( byte* payload, int size)
{
	int progressive_refinement_id;

	DecodingEnvironment *dep;

	dep = AllocPartition ARGS1(1);
	dep->Dcodestrm   = payload;
	dep->Dbuffer     = 0;
	dep->Dbits_to_go = 0;

	progressive_refinement_id   = ue_v ("SEI: progressive_refinement_id");

#ifdef PRINT_PROGRESSIVE_REFINEMENT_END_INFO
	DEBUG_INFO("Progressive refinement segment end SEI message\n");
	DEBUG_INFO("progressive_refinement_id   = %d\n", progressive_refinement_id);
#endif
	FreePartition (dep);
#ifdef PRINT_PROGRESSIVE_REFINEMENT_END_INFO
#undef PRINT_PROGRESSIVE_REFINEMENT_END_INFO
#endif
}


/*!
************************************************************************
*  \brief
*     Interpret the Motion-constrained slice group set SEI message
*  \param payload
*     a pointer that point to the sei payload
*  \param size
*     the size of the sei message
*  \param img
*     the image pointer
*    
************************************************************************
*/
void interpret_motion_constrained_slice_group_set_info PARGS2( byte* payload, int size)
{
	int num_slice_groups_minus1, exact_match_flag, pan_scan_rect_flag, pan_scan_rect_id;

	DecodingEnvironment *dep;

	dep = AllocPartition ARGS1(1);
	dep->Dcodestrm   = payload;
	dep->Dbuffer     = 0;
	dep->Dbits_to_go = 0;

	num_slice_groups_minus1   = ue_v ("SEI: num_slice_groups_minus1");

#ifdef PRINT_MOTION_CONST_SLICE_GROUP_SET_INFO
	DEBUG_INFO("Motion-constrained slice group set SEI message\n");
	DEBUG_INFO("num_slice_groups_minus1   = %d\n", num_slice_groups_minus1);
#endif


	exact_match_flag   = u_1 ("SEI: exact_match_flag");
	pan_scan_rect_flag = u_1 ("SEI: pan_scan_rect_flag");

#ifdef PRINT_MOTION_CONST_SLICE_GROUP_SET_INFO
	DEBUG_INFO("exact_match_flag         = %d\n", exact_match_flag);
	DEBUG_INFO("pan_scan_rect_flag       = %d\n", pan_scan_rect_flag);
#endif

	if (pan_scan_rect_flag)
	{
		pan_scan_rect_id = ue_v ("SEI: pan_scan_rect_id");
#ifdef PRINT_MOTION_CONST_SLICE_GROUP_SET_INFO
		DEBUG_INFO("pan_scan_rect_id         = %d\n", pan_scan_rect_id);
#endif
	}

	FreePartition (dep);
#ifdef PRINT_MOTION_CONST_SLICE_GROUP_SET_INFO
#undef PRINT_MOTION_CONST_SLICE_GROUP_SET_INFO
#endif
}


/*!
************************************************************************
*  \brief
*     Interpret the Reserved SEI message
*  \param payload
*     a pointer that point to the sei payload
*  \param size
*     the size of the sei message
*  \param img
*     the image pointer
*    
************************************************************************
*/
void interpret_reserved_info( byte* payload, int size)
{
	int offset = 0;
	byte payload_byte;

#ifdef PRINT_RESERVED_INFO
	DEBUG_INFO("Reserved SEI message\n");
#endif
	//assert (size<16);
	if  (size>=16) {
		return;
	}

	while (offset < size)
	{
		payload_byte = payload[offset];
		offset ++;
#ifdef PRINT_RESERVED_INFO
		DEBUG_INFO("reserved_sei_message_payload_byte = %d\n", payload_byte);
#endif
	}
#ifdef PRINT_RESERVED_INFO
#undef PRINT_RESERVED_INFO
#endif
}


/*!
************************************************************************
*  \brief
*     Interpret the Buffering period SEI message
*  \param payload
*     a pointer that point to the sei payload
*  \param size
*     the size of the sei message
*  \param img
*     the image pointer
*    
************************************************************************
*/
CREL_RETURN interpret_buffering_period_info PARGS3( byte* payload, int size, unsigned int view_index)
{
	int seq_parameter_set_id, initial_cpb_removal_delay, initial_cpb_removal_delay_offset;
	unsigned int k;
	int ret;

	DecodingEnvironment *dep;
	seq_parameter_set_rbsp_t *sps;

	dep = AllocPartition ARGS1(1);
	dep->Dcodestrm   = payload;
	dep->Dbuffer     = 0;
	dep->Dbits_to_go = 0;

	seq_parameter_set_id   = ue_v ("SEI: seq_parameter_set_id");

	if ( seq_parameter_set_id > MAXSPS - 1 ) {
		return CREL_WARNING_H264_STREAMERROR_LEVEL_3;
	}

	sps = (view_index == 0) ? &SeqParSet[seq_parameter_set_id] : &SeqParSubset[seq_parameter_set_id];

#if !defined(_COLLECT_PIC_)
	if ( (active_sps.Valid == 0) || (sps_is_equal(sps, &active_sps) == SPS_CRITICAL_CHANGE) ) 
	{
		memcpy(&active_sps, sps, sizeof(seq_parameter_set_rbsp_t));

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
	}
#else
	if ( (stream_global->m_active_sps_on_view[view_index] == NULL) || (sps_is_equal(sps, stream_global->m_active_sps_on_view[view_index]) == SPS_CRITICAL_CHANGE) ) 
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

	//activate_sps ARGS1(sps);

#ifdef PRINT_BUFFERING_PERIOD_INFO
	DEBUG_INFO("Buffering period SEI message\n");
	DEBUG_INFO("seq_parameter_set_id   = %d\n", seq_parameter_set_id);
#endif

	// Note: NalHrdBpPresentFlag and CpbDpbDelaysPresentFlag can also be set "by some means not specified in this Recommendation | International Standard"
	if (sps->vui_parameters_present_flag)
	{

		if (sps->vui_seq_parameters.nal_hrd_parameters_present_flag)
		{
			for (k=0; k<sps->vui_seq_parameters.nal_hrd_parameters.cpb_cnt_minus1+1; k++)
			{
				initial_cpb_removal_delay        = u_v (sps->vui_seq_parameters.nal_hrd_parameters.initial_cpb_removal_delay_length_minus1+1, "SEI: initial_cpb_removal_delay");
				initial_cpb_removal_delay_offset = u_v (sps->vui_seq_parameters.nal_hrd_parameters.initial_cpb_removal_delay_length_minus1+1, "SEI: initial_cpb_removal_delay_offset");

#ifdef PRINT_BUFFERING_PERIOD_INFO
				DEBUG_INFO("nal initial_cpb_removal_delay[%d]        = %d\n", k, initial_cpb_removal_delay);
				DEBUG_INFO("nal initial_cpb_removal_delay_offset[%d] = %d\n", k, initial_cpb_removal_delay_offset);
#endif
			}
		}

		if (sps->vui_seq_parameters.vcl_hrd_parameters_present_flag)
		{
			for (k=0; k<sps->vui_seq_parameters.vcl_hrd_parameters.cpb_cnt_minus1+1; k++)
			{
				initial_cpb_removal_delay        = u_v (sps->vui_seq_parameters.vcl_hrd_parameters.initial_cpb_removal_delay_length_minus1+1, "SEI: initial_cpb_removal_delay");
				initial_cpb_removal_delay_offset = u_v (sps->vui_seq_parameters.vcl_hrd_parameters.initial_cpb_removal_delay_length_minus1+1, "SEI: initial_cpb_removal_delay_offset");

#ifdef PRINT_BUFFERING_PERIOD_INFO
				DEBUG_INFO("vcl initial_cpb_removal_delay[%d]        = %d\n", k, initial_cpb_removal_delay);
				DEBUG_INFO("vcl initial_cpb_removal_delay_offset[%d] = %d\n", k, initial_cpb_removal_delay_offset);
#endif
			}
		}
	}

	FreePartition (dep);

	return CREL_OK;

#ifdef PRINT_BUFFERING_PERIOD_INFO
#undef PRINT_BUFFERING_PERIOD_INFO
#endif

}

void interpret_mvc_scalable_nesting PARGS4( byte* payload, int *size, int *num_views, int* view_indexs)
{
	DecodingEnvironment *dep;

	dep = AllocPartition ARGS1(1);
	dep->Dcodestrm   = payload;
	dep->Dbuffer     = 0;
	dep->Dbits_to_go = 0;

	BOOL operation_point_flag;
	BOOL all_view_components_in_au_flag;
	unsigned num_view_components;
	unsigned *sei_view_id = NULL;
	unsigned num_view_components_op;
	unsigned *sei_op_view_id = NULL;
	unsigned sei_op_temporal_id;

	operation_point_flag = u_1("SEI: operation_point_flag");

	if(!operation_point_flag)
	{
		all_view_components_in_au_flag = u_1("SEI: all_view_components_in_au_flag");
		if(!all_view_components_in_au_flag)
		{
			num_view_components = ue_v("SEI: num_view_components") + 1;
			sei_view_id = (unsigned*)malloc(num_view_components * sizeof(unsigned));
			*num_views = num_view_components;

			for(int i = 0; i < num_view_components; i++)
			{
				sei_view_id[i] = u_v(10, "SEI: sei_view_id");
				view_indexs[i] = GetViewIndex ARGS1(sei_view_id[i]);
			}
		}
		else
		{
			*num_views = stream_global->num_views;
			for(int i = 0; i < stream_global->num_views; i++)
				view_indexs[i] = i;
		}
	}
	else
	{
		num_view_components_op = ue_v("SEI: num_view_components_op") + 1;
		sei_op_view_id = (unsigned*)malloc(num_view_components_op * sizeof(unsigned));

		int number_view_index = 0;
		for(int i = 0; i < num_view_components_op; i++)
		{
			sei_op_view_id[i] = u_v(10, "SEI: sei_op_view_id");
			int index = GetViewIndex ARGS1(sei_op_view_id[i]);
			if(index != 0)
				view_indexs[number_view_index++] = index;
		}

		sei_op_temporal_id = u_v(3, "SEI: sei_op_temporal_id");
		*num_views = number_view_index;
	}

	int byte_used = dep->Dcodestrm - payload - (dep->Dbits_to_go >> 3);

	if(byte_used < *size)
		*size = byte_used;

	FreePartition (dep);
	if(sei_view_id)
		free(sei_view_id);
	if(sei_op_view_id)
		free(sei_op_view_id);
}

/*!
************************************************************************
*  \brief
*     Interpret the Picture timing SEI message
*  \param payload
*     a pointer that point to the sei payload
*  \param size
*     the size of the sei message
*  \param img
*     the image pointer
*    
************************************************************************
*/
void interpret_picture_timing_info PARGS3( byte* payload, int size, unsigned int view_index)
{
	int cpb_removal_delay, dpb_output_delay, picture_structure_present_flag, picture_structure;
	int clock_time_stamp_flag;
	int ct_type, nuit_field_based_flag, counting_type, full_timestamp_flag, discontinuity_flag, cnt_dropped_flag, nframes;
	int seconds_value, minutes_value, hours_value, seconds_flag, minutes_flag, hours_flag, time_offset;
	int NumClockTs = 0;
	int i;

	int cpb_removal_len = 24;
	int dpb_output_len  = 24;

	BOOL CpbDpbDelaysPresentFlag;

	DecodingEnvironment *dep;

#if !defined(_COLLECT_PIC_)
	seq_parameter_set_rbsp_t *sps = &active_sps;
#else
	seq_parameter_set_rbsp_t *sps = stream_global->m_active_sps_on_view[view_index];
#endif

	dep = AllocPartition ARGS1(1);
	dep->Dcodestrm   = payload;
	dep->Dbuffer     = 0;
	dep->Dbits_to_go = 0;

	if (NULL==sps)
	{
		fprintf (stderr, "Warning: no active SPS, timing SEI cannot be parsed\n");
		return;
	}

#ifdef PRINT_PCITURE_TIMING_INFO
	DEBUG_INFO("Picture timing SEI message\n");
#endif

	// CpbDpbDelaysPresentFlag can also be set "by some means not specified in this Recommendation | International Standard"
	CpbDpbDelaysPresentFlag =  (sps->vui_parameters_present_flag 
		&& (   (sps->vui_seq_parameters.nal_hrd_parameters_present_flag != 0)
		||(sps->vui_seq_parameters.vcl_hrd_parameters_present_flag != 0)));

	if (CpbDpbDelaysPresentFlag )
	{
		if (sps->vui_parameters_present_flag)
		{
			if (sps->vui_seq_parameters.nal_hrd_parameters_present_flag)
			{
				cpb_removal_len = sps->vui_seq_parameters.nal_hrd_parameters.cpb_removal_delay_length_minus1 + 1;
				dpb_output_len  = sps->vui_seq_parameters.nal_hrd_parameters.dpb_output_delay_length_minus1  + 1;
			}
			else if (sps->vui_seq_parameters.vcl_hrd_parameters_present_flag)
			{
				cpb_removal_len = sps->vui_seq_parameters.vcl_hrd_parameters.cpb_removal_delay_length_minus1 + 1;
				dpb_output_len  = sps->vui_seq_parameters.vcl_hrd_parameters.dpb_output_delay_length_minus1  + 1;
			}
		}

		if ((sps->vui_seq_parameters.nal_hrd_parameters_present_flag)||
			(sps->vui_seq_parameters.vcl_hrd_parameters_present_flag))
		{
			cpb_removal_delay = u_v (cpb_removal_len, "SEI: cpb_removal_delay");
			dpb_output_delay  = u_v (dpb_output_len,  "SEI: dpb_output_delay");
#ifdef PRINT_PCITURE_TIMING_INFO
			DEBUG_INFO("cpb_removal_delay = %d\n",cpb_removal_delay);
			DEBUG_INFO("dpb_output_delay  = %d\n",dpb_output_delay);
#endif
		}
	}

	if (!sps->vui_parameters_present_flag)
	{
		picture_structure_present_flag = 0;
	}
	else
	{
		picture_structure_present_flag  =  sps->vui_seq_parameters.pic_struct_present_flag;
	}

	if (picture_structure_present_flag)
	{
		picture_structure = u_v (4, "SEI: picture_structure");
		sps->vui_seq_parameters.picture_structure = picture_structure;
#ifdef PRINT_PCITURE_TIMING_INFO
		DEBUG_INFO("picture_structure = %d\n",picture_structure);
#endif    
		switch (picture_structure)
		{
		case 0:
		case 1:
		case 2:
			NumClockTs = 1;
			sps->vui_seq_parameters.NumClockTs = 1;
			break;
		case 3:
		case 4:
		case 7:
			NumClockTs = 2;
			sps->vui_seq_parameters.NumClockTs = 2;
			break;
		case 5:
		case 6:
		case 8:
			NumClockTs = 3;
			sps->vui_seq_parameters.NumClockTs = 3;
			break;
		default:
			DEBUG_SHOW_ERROR_INFO("[ERROR]reserved picture_structure used (can't determine NumClockTs)", 500);
		}
		for (i=0; i<NumClockTs; i++)
		{
			clock_time_stamp_flag = u_1 ("SEI: clock_time_stamp_flag");
#ifdef PRINT_PCITURE_TIMING_INFO
			DEBUG_INFO("clock_time_stamp_flag = %d\n",clock_time_stamp_flag);
#endif
			if (clock_time_stamp_flag)
			{
				ct_type               = u_v (2, "SEI: ct_type");
				nuit_field_based_flag = u_1 (   "SEI: nuit_field_based_flag");
				counting_type         = u_v (5, "SEI: counting_type");
				full_timestamp_flag   = u_1 (   "SEI: full_timestamp_flag");
				discontinuity_flag    = u_1 (   "SEI: discontinuity_flag");
				cnt_dropped_flag      = u_1 (   "SEI: cnt_dropped_flag");
				nframes               = u_v (8, "SEI: nframes");

#ifdef PRINT_PCITURE_TIMING_INFO
				DEBUG_INFO("ct_type               = %d\n",ct_type);
				DEBUG_INFO("nuit_field_based_flag = %d\n",nuit_field_based_flag);
				DEBUG_INFO("full_timestamp_flag   = %d\n",full_timestamp_flag);
				DEBUG_INFO("discontinuity_flag    = %d\n",discontinuity_flag);
				DEBUG_INFO("cnt_dropped_flag      = %d\n",cnt_dropped_flag);
				DEBUG_INFO("nframes               = %d\n",nframes);
#endif    
				if (full_timestamp_flag)
				{
					seconds_value         = u_v (6, "SEI: seconds_value");
					minutes_value         = u_v (6, "SEI: minutes_value");
					hours_value           = u_v (5, "SEI: hours_value");
#ifdef PRINT_PCITURE_TIMING_INFO
					DEBUG_INFO("seconds_value = %d\n",seconds_value);
					DEBUG_INFO("minutes_value = %d\n",minutes_value);
					DEBUG_INFO("hours_value   = %d\n",hours_value);
#endif    
				}
				else
				{
					seconds_flag          = u_1 (   "SEI: seconds_flag");
#ifdef PRINT_PCITURE_TIMING_INFO
					DEBUG_INFO("seconds_flag = %d\n",seconds_flag);
#endif    
					if (seconds_flag)
					{
						seconds_value         = u_v (6, "SEI: seconds_value");
						minutes_flag          = u_1 (   "SEI: minutes_flag");
#ifdef PRINT_PCITURE_TIMING_INFO
						DEBUG_INFO("seconds_value = %d\n",seconds_value);
						DEBUG_INFO("minutes_flag  = %d\n",minutes_flag);
#endif    
						if(minutes_flag)
						{
							minutes_value         = u_v (6, "SEI: minutes_value");
							hours_flag            = u_1 (   "SEI: hours_flag");
#ifdef PRINT_PCITURE_TIMING_INFO
							DEBUG_INFO("minutes_value = %d\n",minutes_value);
							DEBUG_INFO("hours_flag    = %d\n",hours_flag);
#endif    
							if(hours_flag)
							{
								hours_value           = u_v (5, "SEI: hours_value");
#ifdef PRINT_PCITURE_TIMING_INFO
								DEBUG_INFO("hours_value   = %d\n",hours_value);
#endif    
							}

						}
					}
				}

				{
					int time_offset_length;
					if (sps->vui_seq_parameters.vcl_hrd_parameters_present_flag)
						time_offset_length = sps->vui_seq_parameters.vcl_hrd_parameters.time_offset_length;
					else if (sps->vui_seq_parameters.nal_hrd_parameters_present_flag)
						time_offset_length = sps->vui_seq_parameters.nal_hrd_parameters.time_offset_length;
					else
						time_offset_length = 24;
					if (time_offset_length)
						time_offset = u_v(time_offset_length, "SEI: time_offset"); // TODO interpretation is unsigned, need signed interpretation (i_v)
					else
						time_offset=0;
#ifdef PRINT_PCITURE_TIMING_INFO
					DEBUG_INFO("time_offset   = %d\n",time_offset);
#endif    
				}
			}
		}
	}

	FreePartition (dep);
#ifdef PRINT_PCITURE_TIMING_INFO
#undef PRINT_PCITURE_TIMING_INFO
#endif
}
