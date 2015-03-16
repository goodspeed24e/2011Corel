#ifndef _DIAGNOSIS_OF_GPS_USER_EVENT_IDENTIFICATION_H_
#define _DIAGNOSIS_OF_GPS_USER_EVENT_IDENTIFICATION_H_

// namespace Diagnosis-of-GPS
namespace dog
{

enum DogEventIdEnum
{
	DEBUG_MESSAGE_EVENT = 0,
	ERROR_MESSAGE_EVENT,
	FATAL_MESSAGE_EVENT,

	//-------------------------------------------
	// demux events

	DEMUX_EVENT = 10000, // reserved
	DEMUX_MAIN_PATH_UPSTREAM_DATA_RATE_FROM_SECTORBUFF_UPDATE_EVENT,
    DEMUX_SUB_PATH_UPSTREAM_DATA_RATE_FROM_SECTORBUFF_UPDATE_EVENT,
    DEMUX_MAIN_PATH_UPSTREAM_DATA_RATE_FROM_IO_UPDATE_EVENT,
    DEMUX_SUB_PATH_UPSTREAM_DATA_RATE_FROM_IO_UPDATE_EVENT,
	DEMUX_DOWNSTREAM_VIDEO_DATA_RATE_UPDATE_EVENT,
	DEMUX_DOWNSTREAM_2ND_VIDE_DATA_RATE_UPDATEO_EVENT,
	DEMUX_DOWNSTREAM_AUDIO_DATA_RATE_UPDATE_EVENT,
	DEMUX_DOWNSTREAM_2ND_AUDIO_DATA_RATE_UPDATE_EVENT,
	DEMUX_DOWNSTREAM_PG_DATA_RATE_UPDATE_EVENT,
	DEMUX_DOWNSTREAM_IG_DATA_RATE_UPDATE_EVENT,

	DEMUX_COUNTER_DISCONTINUITY_EVENT,
	DEMUX_MAIN_PATH_MEDIA_ERROR_EVENT,
    DEMUX_SUB_PATH_MEDIA_ERROR_EVENT,
	DEMUX_PTS_GAP_EVENT,

	DEMUX_SECTOR_BUFFER_EVENT,
	DEMUX_VIDEO_MB_EVENT,
	DEMUX_2ND_VIDEO_MB_EVENT,
	DEMUX_VIDEO_EB_EVENT,
	DEMUX_2ND_VIDEO_EB_EVENT,
	DEMUX_AUDIO_EB_EVENT,
	DEMUX_2ND_AUDIO_EB_EVENT,
	DEMUX_PG_EB_EVENT,
    DEMUX_2ND_PG_EB_EVENT,
	DEMUX_IG_EB_EVENT,

	DEMUX_SECTOR_INDEX_UPDATE_EVENT,
	DEMUX_DISPATCH_INDEX_UPDATE_EVENT,

	DEMUX_VIDEO_PTS_DELTA_EVENT,
	DEMUX_2ND_VIDEO_PTS_DELTA_EVENT,
	DEMUX_AUDIO_PTS_DELTA_EVENT,
	DEMUX_2ND_AUDIO_PTS_DELTA_EVENT,

	DEMUX_VIDEO_SRC_DELTA_EVENT,
	DEMUX_2ND_VIDEO_SRC_DELTA_EVENT,
	DEMUX_AUDIO_SRC_DELTA_EVENT,
	DEMUX_2ND_AUDIO_SRC_DELTA_EVENT,


	//-------------------------------------------
	// video events

	VIDEO_EVENT = 20000, // reserved
	VIDEO_FRAME_EVENT,
	VIDEO_2ND_FRAME_EVENT,

	VIDEO_BITRATE_UPDATE_EVENT,
	VIDEO_2ND_BITRATE_UPDATE_EVENT,
	VIDEO_DROPPED_FRAMES_UPDATE_EVENT,
	VIDEO_2ND_DROPPED_FRAMES_UPDATE_EVENT,
	VIDEO_TOTAL_FRAMES_UPDATE_EVENT,
	VIDEO_2ND_TOTAL_FRAMES_UPDATE_EVENT,
	VIDEO_I_FRAMES_UPDATE_EVENT,
	VIDEO_2ND_I_FRAMES_UPDATE_EVENT,
	VIDEO_P_FRAMES_UPDATE_EVENT,
	VIDEO_2ND_P_FRAMES_UPDATE_EVENT,
	VIDEO_B_FRAMES_UPDATE_EVENT,
	VIDEO_2ND_B_FRAMES_UPDATE_EVENT,
	VIDEO_INTERLACED_FRAMES_UPDATE_EVENT,
	VIDEO_2ND_INTERLACED_FRAMES_UPDATE_EVENT,
	VIDEO_ENCRYPTED_FRAMES_UPDATE_EVENT,
	VIDEO_2ND_ENCRYPTED_FRAMES_UPDATE_EVENT,
	VIDEO_FRAME_RATE_UPDATE_EVENT,
	VIDEO_2ND_FRAME_RATE_UPDATE_EVENT,
	VIDEO_PRESENTATION_JITTER_UPDATE_EVENT,
	VIDEO_2ND_PRESENTATION_JITTER_UPDATE_EVENT,
	VIDEO_DECODE_MSEC_UPDATE_EVENT,
	VIDEO_2ND_DECODE_MSEC_UPDATE_EVENT,

	//-------------------------------------------
	// display events

	DISPLAY_EVENT = 30000, // reserved
	DISPLAY_AVSYNC_UPDATE_EVENT,
	DISPLAY_FRAME_QUEUE_SIZE_UPDATE_EVENT,
	DISPLAY_2ND_FRAME_QUEUE_SIZE_UPDATE_EVENT,
	DISPLAY_FRAME_DROP_EVENT,
	DISPLAY_2ND_FRAME_DROP_EVENT,
	DISPLAY_FRAME_PTS_DELTA_UPDATE_EVENT,
	DISPLAY_2ND_FRAME_PTS_DELTA_UPDATE_EVENT,
	DISPLAY_PREPARE_FRAME_COST_UPDATE_EVENT,
	DISPLAY_2ND_PREPARE_FRAME_COST_UPDATE_EVENT,
	DISPLAY_DISPLAY_FRAME_COST_UPDATE_EVENT,
	DISPLAY_DELTA_UPDATE_EVENT,
	DISPLAY_JITTER_EVENT,

	DISPLAY_EVR_EVENT = 31000, // reserved
	DISPLAY_EVR_AVSYNC_UPDATE_EVENT,
	DISPLAY_EVR_SAMPLE_IN_TIME_DELTA_UPDATE_EVENT,
	DISPLAY_EVR_SAMPLE_POOL_UPDATE_EVENT,
	DISPLAY_EVR_PRESENT_QUEUE_UPDATE_EVENT,
	DISPLAY_EVR_RENDER_COST_UPDATE_EVENT,  // render cost
	DISPLAY_EVR_PRESENT_COST_UPDATE_EVENT, // present cost
	DISPLAY_EVR_RENDER_PRESENT_COST_UPDATE_EVENT, // render + present cost


	//-------------------------------------------
	// audio events

	AUDIO_EVENT = 40000, // reserved

	AUDIO_DECODING_1ST_DATA_EVENT,
	AUDIO_DECODING_2ND_DATA_EVENT,
	AUDIO_DECODING_METADATA_EVENT,
	AUDIO_DECODING_DATA_IN_PTS_DELTA_UPDATE_EVENT,
	AUDIO_DECODING_DATA_OUT_PTS_DELTA_UPDATE_EVENT,

	AUDIO_BACKEND_1ST_BUFFER_EVENT,
	AUDIO_BACKEND_2ND_BUFFER_EVENT,
	AUDIO_BACKEND_METADATA_BUFFER_EVENT,

	AUDIO_RENDER_BUFFER_EVENT,

	AUDIO_AVSYNC_PTS_DELTA_UPDATE_EVENT,

	AUDIO_1ST_STREAM_INFO_UPDATE_EVENT,
	AUDIO_2ND_STREAM_INFO_UPDATE_EVENT,
	AUDIO_OUTPUT_UPDATE_EVENT,
	AUDIO_DEVICE_CAPABILITY_UPDATE_EVENT,

	//-------------------------------------------
	// user define event

	USER_DEFINE_EVENT = 0x80000000,
};

} // namespace dog

#endif //_DIAGNOSIS_OF_GPS_USER_EVENT_IDENTIFICATION_H_
