
/*!
*************************************************************************************
* \file sei.h
*
* \brief
*    Prototypes for sei.c
*************************************************************************************
*/

#ifndef SEI_H
#define SEI_H

typedef enum {
	SEI_BUFFERING_PERIOD = 0,
	SEI_PIC_TIMING,
	SEI_PAN_SCAN_RECT,
	SEI_FILLER_PAYLOAD,
	SEI_USER_DATA_REGISTERED_ITU_T_T35,
	SEI_USER_DATA_UNREGISTERED,
	SEI_RECOVERY_POINT,
	SEI_DEC_REF_PIC_MARKING_REPETITION,
	SEI_SPARE_PIC,
	SEI_SCENE_INFO,
	SEI_SUB_SEQ_INFO,
	SEI_SUB_SEQ_LAYER_CHARACTERISTICS,
	SEI_SUB_SEQ_CHARACTERISTICS,
	SEI_FULL_FRAME_FREEZE,
	SEI_FULL_FRAME_FREEZE_RELEASE,
	SEI_FULL_FRAME_SNAPSHOT,
	SEI_PROGRESSIVE_REFINEMENT_SEGMENT_START,
	SEI_PROGRESSIVE_REFINEMENT_SEGMENT_END,
	SEI_MOTION_CONSTRAINED_SLICE_GROUP_SET,
	SEI_FILM_GRAIN_CHARACTERISTICS,
	SEI_DEBLOCKING_FILTER_DISPLAY_PREFERENCE,
	SEI_STEREO_VIDEO_INFO,
	SEI_POST_FILTER_HINT,
	SEI_TONE_MAPPING_INFO,
	SEI_SCALABILITY_INFO,
	SEI_SUB_PIC_SCALABLE_LAYER,
	SEI_NON_REQUIRED_LAYER_REP,
	SEI_PRIORITY_LAYER_INFO,
	SEI_LAYERS_NO_PRESENT,
	SEI_LAYER_DEPENDENCY_CHANGE,
	SEI_SCALABLE_NESTING,
	SEI_BASE_LAYER_TEMPORAL_HRD,
	SEI_QUALITY_LAYER_INTEGRITY_CHECK,
	SEI_REDUNDANT_PIC_PROPERTY,
	SEI_TL0_DEP_REP_INDEX,
	SEI_TL_SWITCHING_POINT,
	SEI_PARALLEL_DECODING_INFO,
	SEI_MVC_SCALABLE_NESTING,
	SEI_VIEW_SCALABILITY_INFO,
	SEI_MULTIVIEW_SCENE_INFO,
	SEI_MULTIVIEW_ACQUISITION_INFO,
	SEI_NON_REQUIRED_VIEW_COMPONENT,
	SEI_VIEW_DEPENDENCY_CHANGE,
	SEI_OPERATION_POINTS_NOT_PRESENT,
	SEI_BASE_VIEW_TEMPORAL_HRD,

	SEI_MAX_ELEMENTS  //!< number of maximum syntax elements
} SEI_type;

#define MAX_FN 256

CREL_RETURN  InterpretSEIMessage PARGS2(byte* msg, int size);
void interpret_spare_pic PARGS2( byte* payload, int size);
void interpret_subsequence_info PARGS2( byte* payload, int size);
void interpret_subsequence_layer_characteristics_info PARGS2( byte* payload, int size);
void interpret_subsequence_characteristics_info PARGS2( byte* payload, int size);
void interpret_scene_information PARGS2( byte* payload, int size); // JVT-D099
void interpret_user_data_registered_itu_t_t35_info( byte* payload, int size);
void interpret_user_data_unregistered_info PARGS2( byte* payload, int size);
void interpret_pan_scan_rect_info PARGS2( byte* payload, int size);
void interpret_recovery_point_info PARGS2( byte* payload, int size);
void interpret_filler_payload_info( byte* payload, int size);
void interpret_dec_ref_pic_marking_repetition_info PARGS3( byte* payload, int size, unsigned int view_index);
void interpret_full_frame_freeze_info( byte* payload, int size);
void interpret_full_frame_freeze_release_info( byte* payload, int size);
void interpret_full_frame_snapshot_info PARGS2( byte* payload, int size);
void interpret_progressive_refinement_start_info( byte* payload, int size);
void interpret_progressive_refinement_end_info PARGS2( byte* payload, int size);
void interpret_motion_constrained_slice_group_set_info PARGS2( byte* payload, int size);
void interpret_reserved_info( byte* payload, int size);
CREL_RETURN interpret_buffering_period_info PARGS3( byte* payload, int size, unsigned int view_index);
void interpret_picture_timing_info PARGS3( byte* payload, int size, unsigned int view_index);
void interpret_mvc_scalable_nesting PARGS4( byte* payload, int *size, int *num_views, int* view_indexs);
void interpret_offset_metadata PARGS2(byte* payload, int size);

#endif
