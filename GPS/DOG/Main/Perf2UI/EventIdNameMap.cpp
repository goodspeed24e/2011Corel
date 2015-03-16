#include "stdafx.h"
#include "EventIdNameMap.h"
#include "DogEventId.h"

CEventIdNameMap g_EventIdNameMap;

//-----------------------------------------------------------------------------
std::basic_string<TCHAR> CEventIdNameMap::GetFriendlyEventName(DWORD eventId)
{
	const TEventIdNameMap::const_iterator iter = m_TotalEvents.find(eventId);
	if(iter != m_TotalEvents.end())
	{
		return iter->second;
	}
	else
	{
		const TEventIdNameMap::const_iterator usrIter = m_UserDefineEvents.find(eventId);
		if(usrIter != m_UserDefineEvents.end())
		{
			return usrIter->second;
		}
		else
		{
			TCHAR strId[32];
			_stprintf_s(strId, sizeof(strId)/sizeof(TCHAR), _T("%d"), eventId);
			return strId;
		}
	}
}

//-----------------------------------------------------------------------------
void CEventIdNameMap::Init()
{
	using namespace dog;

	// Demux Event

	m_DemuxEvents[DEMUX_MAIN_PATH_UPSTREAM_DATA_RATE_FROM_SECTORBUFF_UPDATE_EVENT]= _T("DEMUX_MAIN_PATH_UPSTREAM_DATA_RATE_FROM_SECTORBUFF_UPDATE_EVENT");
	m_DemuxEvents[DEMUX_SUB_PATH_UPSTREAM_DATA_RATE_FROM_SECTORBUFF_UPDATE_EVENT]=  _T("DEMUX_SUB_PATH_UPSTREAM_DATA_RATE_FROM_SECTORBUFF_UPDATE_EVENT");
	m_DemuxEvents[DEMUX_MAIN_PATH_UPSTREAM_DATA_RATE_FROM_IO_UPDATE_EVENT]=         _T("DEMUX_MAIN_PATH_UPSTREAM_DATA_RATE_FROM_IO_UPDATE_EVENT");
	m_DemuxEvents[DEMUX_SUB_PATH_UPSTREAM_DATA_RATE_FROM_IO_UPDATE_EVENT]=          _T("DEMUX_SUB_PATH_UPSTREAM_DATA_RATE_FROM_IO_UPDATE_EVENT");
	m_DemuxEvents[DEMUX_DOWNSTREAM_VIDEO_DATA_RATE_UPDATE_EVENT]=                   _T("DEMUX_DOWNSTREAM_VIDEO_DATA_RATE_UPDATE_EVENT");
	m_DemuxEvents[DEMUX_DOWNSTREAM_2ND_VIDE_DATA_RATE_UPDATEO_EVENT]=               _T("DEMUX_DOWNSTREAM_2ND_VIDE_DATA_RATE_UPDATEO_EVENT");
	m_DemuxEvents[DEMUX_DOWNSTREAM_AUDIO_DATA_RATE_UPDATE_EVENT]=                   _T("DEMUX_DOWNSTREAM_AUDIO_DATA_RATE_UPDATE_EVENT");
	m_DemuxEvents[DEMUX_DOWNSTREAM_2ND_AUDIO_DATA_RATE_UPDATE_EVENT]=               _T("DEMUX_DOWNSTREAM_2ND_AUDIO_DATA_RATE_UPDATE_EVENT");
	m_DemuxEvents[DEMUX_DOWNSTREAM_PG_DATA_RATE_UPDATE_EVENT]=                      _T("DEMUX_DOWNSTREAM_PG_DATA_RATE_UPDATE_EVENT");
	m_DemuxEvents[DEMUX_DOWNSTREAM_IG_DATA_RATE_UPDATE_EVENT]=                      _T("DEMUX_DOWNSTREAM_IG_DATA_RATE_UPDATE_EVENT");
	m_DemuxEvents[DEMUX_COUNTER_DISCONTINUITY_EVENT]=                               _T("DEMUX_COUNTER_DISCONTINUITY_EVENT");
	m_DemuxEvents[DEMUX_MAIN_PATH_MEDIA_ERROR_EVENT]=                               _T("DEMUX_MAIN_PATH_MEDIA_ERROR_EVENT");
	m_DemuxEvents[DEMUX_SUB_PATH_MEDIA_ERROR_EVENT]=                                _T("DEMUX_SUB_PATH_MEDIA_ERROR_EVENT");
	m_DemuxEvents[DEMUX_PTS_GAP_EVENT]=                                             _T("DEMUX_PTS_GAP_EVENT");
	m_DemuxEvents[DEMUX_SECTOR_BUFFER_EVENT]=                                       _T("DEMUX_SECTOR_BUFFER_EVENT");
	m_DemuxEvents[DEMUX_VIDEO_MB_EVENT]=                                            _T("DEMUX_VIDEO_MB_EVENT");
	m_DemuxEvents[DEMUX_2ND_VIDEO_MB_EVENT]=                                        _T("DEMUX_2ND_VIDEO_MB_EVENT");
	m_DemuxEvents[DEMUX_VIDEO_EB_EVENT]=                                            _T("DEMUX_VIDEO_EB_EVENT");
	m_DemuxEvents[DEMUX_2ND_VIDEO_EB_EVENT]=                                        _T("DEMUX_2ND_VIDEO_EB_EVENT");
	m_DemuxEvents[DEMUX_AUDIO_EB_EVENT]=                                            _T("DEMUX_AUDIO_EB_EVENT");
	m_DemuxEvents[DEMUX_2ND_AUDIO_EB_EVENT]=                                        _T("DEMUX_2ND_AUDIO_EB_EVENT");
	m_DemuxEvents[DEMUX_PG_EB_EVENT]=                                               _T("DEMUX_PG_EB_EVENT");
	m_DemuxEvents[DEMUX_2ND_PG_EB_EVENT]=                                           _T("DEMUX_2ND_PG_EB_EVENT");
	m_DemuxEvents[DEMUX_IG_EB_EVENT]=                                               _T("DEMUX_IG_EB_EVENT");
	m_DemuxEvents[DEMUX_SECTOR_INDEX_UPDATE_EVENT]=                                 _T("DEMUX_SECTOR_INDEX_UPDATE_EVENT");
	m_DemuxEvents[DEMUX_DISPATCH_INDEX_UPDATE_EVENT]=                               _T("DEMUX_DISPATCH_INDEX_UPDATE_EVENT");
	m_DemuxEvents[DEMUX_VIDEO_PTS_DELTA_EVENT]=                                     _T("DEMUX_VIDEO_PTS_DELTA_EVENT");
	m_DemuxEvents[DEMUX_2ND_VIDEO_PTS_DELTA_EVENT]=                                 _T("DEMUX_2ND_VIDEO_PTS_DELTA_EVENT");
	m_DemuxEvents[DEMUX_AUDIO_PTS_DELTA_EVENT]=                                     _T("DEMUX_AUDIO_PTS_DELTA_EVENT");
	m_DemuxEvents[DEMUX_2ND_AUDIO_PTS_DELTA_EVENT]=                                 _T("DEMUX_2ND_AUDIO_PTS_DELTA_EVENT");
	m_DemuxEvents[DEMUX_VIDEO_SRC_DELTA_EVENT]=                                     _T("DEMUX_VIDEO_SRC_DELTA_EVENT");
	m_DemuxEvents[DEMUX_2ND_VIDEO_SRC_DELTA_EVENT]=                                 _T("DEMUX_2ND_VIDEO_SRC_DELTA_EVENT");
	m_DemuxEvents[DEMUX_AUDIO_SRC_DELTA_EVENT]=                                     _T("DEMUX_AUDIO_SRC_DELTA_EVENT");
	m_DemuxEvents[DEMUX_2ND_AUDIO_SRC_DELTA_EVENT]=                                 _T("DEMUX_2ND_AUDIO_SRC_DELTA_EVENT");

	m_TotalEvents.insert(m_DemuxEvents.begin(), m_DemuxEvents.end());


	// Audio Events

	m_AudioEvents[AUDIO_DECODING_1ST_DATA_EVENT]=                  _T("AUDIO_DECODING_1ST_DATA_EVENT");
	m_AudioEvents[AUDIO_DECODING_2ND_DATA_EVENT]=                  _T("AUDIO_DECODING_2ND_DATA_EVENT");
	m_AudioEvents[AUDIO_DECODING_METADATA_EVENT]=                  _T("AUDIO_DECODING_METADATA_EVENT");
	m_AudioEvents[AUDIO_DECODING_DATA_IN_PTS_DELTA_UPDATE_EVENT]=  _T("AUDIO_DECODING_DATA_IN_PTS_DELTA_UPDATE_EVENT");
	m_AudioEvents[AUDIO_DECODING_DATA_OUT_PTS_DELTA_UPDATE_EVENT]= _T("AUDIO_DECODING_DATA_OUT_PTS_DELTA_UPDATE_EVENT");
	m_AudioEvents[AUDIO_BACKEND_1ST_BUFFER_EVENT]=                 _T("AUDIO_BACKEND_1ST_BUFFER_EVENT");
	m_AudioEvents[AUDIO_BACKEND_2ND_BUFFER_EVENT]=                 _T("AUDIO_BACKEND_2ND_BUFFER_EVENT");
	m_AudioEvents[AUDIO_BACKEND_METADATA_BUFFER_EVENT]=            _T("AUDIO_BACKEND_METADATA_BUFFER_EVENT");
	m_AudioEvents[AUDIO_RENDER_BUFFER_EVENT]=                      _T("AUDIO_RENDER_BUFFER_EVENT");
	m_AudioEvents[AUDIO_AVSYNC_PTS_DELTA_UPDATE_EVENT]=            _T("AUDIO_AVSYNC_PTS_DELTA_UPDATE_EVENT");
	m_AudioEvents[AUDIO_1ST_STREAM_INFO_UPDATE_EVENT]=             _T("AUDIO_1ST_STREAM_INFO_UPDATE_EVENT");
	m_AudioEvents[AUDIO_2ND_STREAM_INFO_UPDATE_EVENT]=             _T("AUDIO_2ND_STREAM_INFO_UPDATE_EVENT");
	m_AudioEvents[AUDIO_OUTPUT_UPDATE_EVENT]=                      _T("AUDIO_OUTPUT_UPDATE_EVENT");
	m_AudioEvents[AUDIO_DEVICE_CAPABILITY_UPDATE_EVENT]=           _T("AUDIO_DEVICE_CAPABILITY_UPDATE_EVENT");

	m_TotalEvents.insert(m_AudioEvents.begin(), m_AudioEvents.end());


	// Video Events

	m_VideoEvents[VIDEO_FRAME_EVENT]=                           _T("VIDEO_FRAME_EVENT");
	m_VideoEvents[VIDEO_2ND_FRAME_EVENT]=                       _T("VIDEO_2ND_FRAME_EVENT");
	m_VideoEvents[VIDEO_BITRATE_UPDATE_EVENT]=                  _T("VIDEO_BITRATE_UPDATE_EVENT");
	m_VideoEvents[VIDEO_2ND_BITRATE_UPDATE_EVENT]=              _T("VIDEO_2ND_BITRATE_UPDATE_EVENT");
	m_VideoEvents[VIDEO_DROPPED_FRAMES_UPDATE_EVENT]=           _T("VIDEO_DROPPED_FRAMES_UPDATE_EVENT");
	m_VideoEvents[VIDEO_2ND_DROPPED_FRAMES_UPDATE_EVENT]=       _T("VIDEO_2ND_DROPPED_FRAMES_UPDATE_EVENT");
	m_VideoEvents[VIDEO_TOTAL_FRAMES_UPDATE_EVENT]=             _T("VIDEO_TOTAL_FRAMES_UPDATE_EVENT");
	m_VideoEvents[VIDEO_2ND_TOTAL_FRAMES_UPDATE_EVENT]=         _T("VIDEO_2ND_TOTAL_FRAMES_UPDATE_EVENT");
	m_VideoEvents[VIDEO_I_FRAMES_UPDATE_EVENT]=                 _T("VIDEO_I_FRAMES_UPDATE_EVENT");
	m_VideoEvents[VIDEO_2ND_I_FRAMES_UPDATE_EVENT]=             _T("VIDEO_2ND_I_FRAMES_UPDATE_EVENT");
	m_VideoEvents[VIDEO_P_FRAMES_UPDATE_EVENT]=                 _T("VIDEO_P_FRAMES_UPDATE_EVENT");
	m_VideoEvents[VIDEO_2ND_P_FRAMES_UPDATE_EVENT]=             _T("VIDEO_2ND_P_FRAMES_UPDATE_EVENT");
	m_VideoEvents[VIDEO_B_FRAMES_UPDATE_EVENT]=                 _T("VIDEO_B_FRAMES_UPDATE_EVENT");
	m_VideoEvents[VIDEO_2ND_B_FRAMES_UPDATE_EVENT]=             _T("VIDEO_2ND_B_FRAMES_UPDATE_EVENT");
	m_VideoEvents[VIDEO_INTERLACED_FRAMES_UPDATE_EVENT]=        _T("VIDEO_INTERLACED_FRAMES_UPDATE_EVENT");
	m_VideoEvents[VIDEO_2ND_INTERLACED_FRAMES_UPDATE_EVENT]=    _T("VIDEO_2ND_INTERLACED_FRAMES_UPDATE_EVENT");
	m_VideoEvents[VIDEO_ENCRYPTED_FRAMES_UPDATE_EVENT]=         _T("VIDEO_ENCRYPTED_FRAMES_UPDATE_EVENT");
	m_VideoEvents[VIDEO_2ND_ENCRYPTED_FRAMES_UPDATE_EVENT]=     _T("VIDEO_2ND_ENCRYPTED_FRAMES_UPDATE_EVENT");
	m_VideoEvents[VIDEO_FRAME_RATE_UPDATE_EVENT]=               _T("VIDEO_FRAME_RATE_UPDATE_EVENT");
	m_VideoEvents[VIDEO_2ND_FRAME_RATE_UPDATE_EVENT]=           _T("VIDEO_2ND_FRAME_RATE_UPDATE_EVENT");
	m_VideoEvents[VIDEO_PRESENTATION_JITTER_UPDATE_EVENT]=      _T("VIDEO_PRESENTATION_JITTER_UPDATE_EVENT");
	m_VideoEvents[VIDEO_2ND_PRESENTATION_JITTER_UPDATE_EVENT]=  _T("VIDEO_2ND_PRESENTATION_JITTER_UPDATE_EVENT");
	m_VideoEvents[VIDEO_DECODE_MSEC_UPDATE_EVENT]=              _T("VIDEO_DECODE_MSEC_UPDATE_EVENT");
	m_VideoEvents[VIDEO_2ND_DECODE_MSEC_UPDATE_EVENT]=          _T("VIDEO_2ND_DECODE_MSEC_UPDATE_EVENT");

	m_TotalEvents.insert(m_VideoEvents.begin(), m_VideoEvents.end());


	// Display Events

	m_DisplayEvents[DISPLAY_AVSYNC_UPDATE_EVENT]=                 _T("DISPLAY_AVSYNC_UPDATE_EVENT");
	m_DisplayEvents[DISPLAY_FRAME_QUEUE_SIZE_UPDATE_EVENT]=       _T("DISPLAY_FRAME_QUEUE_SIZE_UPDATE_EVENT");
	m_DisplayEvents[DISPLAY_2ND_FRAME_QUEUE_SIZE_UPDATE_EVENT]=   _T("DISPLAY_2ND_FRAME_QUEUE_SIZE_UPDATE_EVENT");
	m_DisplayEvents[DISPLAY_FRAME_DROP_EVENT]=                    _T("DISPLAY_FRAME_DROP_EVENT");
	m_DisplayEvents[DISPLAY_2ND_FRAME_DROP_EVENT]=                _T("DISPLAY_2ND_FRAME_DROP_EVENT");
	m_DisplayEvents[DISPLAY_FRAME_PTS_DELTA_UPDATE_EVENT]=        _T("DISPLAY_FRAME_PTS_DELTA_UPDATE_EVENT");
	m_DisplayEvents[DISPLAY_2ND_FRAME_PTS_DELTA_UPDATE_EVENT]=    _T("DISPLAY_2ND_FRAME_PTS_DELTA_UPDATE_EVENT");
	m_DisplayEvents[DISPLAY_PREPARE_FRAME_COST_UPDATE_EVENT]=     _T("DISPLAY_PREPARE_FRAME_COST_UPDATE_EVENT");
	m_DisplayEvents[DISPLAY_2ND_PREPARE_FRAME_COST_UPDATE_EVENT]= _T("DISPLAY_2ND_PREPARE_FRAME_COST_UPDATE_EVENT");
	m_DisplayEvents[DISPLAY_DISPLAY_FRAME_COST_UPDATE_EVENT]=     _T("DISPLAY_DISPLAY_FRAME_COST_UPDATE_EVENT");
	m_DisplayEvents[DISPLAY_DELTA_UPDATE_EVENT]=                  _T("DISPLAY_DELTA_UPDATE_EVENT");
	m_DisplayEvents[DISPLAY_JITTER_EVENT]=                        _T("DISPLAY_JITTER_EVENT");

	m_DisplayEvents[DISPLAY_EVR_AVSYNC_UPDATE_EVENT]                = _T("DISPLAY_EVR_AVSYNC_UPDATE_EVENT");
	m_DisplayEvents[DISPLAY_EVR_SAMPLE_IN_TIME_DELTA_UPDATE_EVENT]  = _T("DISPLAY_EVR_SAMPLE_IN_TIME_DELTA_UPDATE_EVENT");
	m_DisplayEvents[DISPLAY_EVR_SAMPLE_POOL_UPDATE_EVENT]           = _T("DISPLAY_EVR_SAMPLE_POOL_UPDATE_EVENT");
	m_DisplayEvents[DISPLAY_EVR_PRESENT_QUEUE_UPDATE_EVENT]         = _T("DISPLAY_EVR_PRESENT_QUEUE_UPDATE_EVENT");
	m_DisplayEvents[DISPLAY_EVR_RENDER_COST_UPDATE_EVENT]           = _T("DISPLAY_EVR_RENDER_COST_UPDATE_EVENT");
	m_DisplayEvents[DISPLAY_EVR_PRESENT_COST_UPDATE_EVENT]          = _T("DISPLAY_EVR_PRESENT_COST_UPDATE_EVENT");
	m_DisplayEvents[DISPLAY_EVR_RENDER_PRESENT_COST_UPDATE_EVENT]   = _T("DISPLAY_EVR_RENDER_PRESENT_COST_UPDATE_EVENT");

	m_TotalEvents.insert(m_DisplayEvents.begin(), m_DisplayEvents.end());
}

