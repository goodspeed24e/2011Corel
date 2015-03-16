#ifndef _DIAGNOSIS_OF_SUB_CATEGORY_H_
#define _DIAGNOSIS_OF_SUB_CATEGORY_H_

namespace dogsub
{

enum GenericSubCategory
{
	CAT_GENERIC_NONE = 0,
	CAT_GENERIC_MAIN,
	CAT_GENERIC_DISPLAY_FIRST_FRAME,
};

enum DRMSubCategory
{
	CAT_DRM_GENERIC = 0,
};

enum DemuxSubCategory
{
	CAT_DEMUX_GENERIC = 0,
    CAT_DEMUX_GET_DISC_TYPE,
    CAT_DEMUX_INIT_CTRL,
    CAT_DEMUX_CTRL_OPEN,
    CAT_DEMUX_DEMUX_OPEN,
    CAT_DEMUX_SET_ID,
    CAT_DEMUX_BUILD_GRAPH,
    CAT_DEMUX_FILTER_PAUSE_BEGIN,
    CAT_DEMUX_FILTER_PAUSE_END,
    CAT_DEMUX_FILTER_RUN_BEGIN,
    CAT_DEMUX_FILTER_RUN_END,
    CAT_DEMUX_DVDCTRL_PLAY,
    CAT_DEMUX_DEMUX_PLAY,
    CAT_DEMUX_1ST_VDO_GET_PACKET_FROM_QUEUE,
    CAT_DEMUX_1ST_VDOPACK_BEGIN_DELIVER,
    CAT_DEMUX_1ST_VDOPACK_END_DELIVER
};

enum VideoSubCategory
{
	CAT_VIDEO_GENERIC = 000,
	CAT_VIDEO_GENERIC_DXVADRAWSETPARAMETER = 001,
	CAT_VIDEO_PLAYBACK_STATUS,

	CAT_VIDEO_RENDER= 100,
	CAT_VIDEO_RENDER_OPEN,                          //RENDER_OPEN = HVDSERVICE_UNINIT + HVDSERVICE_INIT + IVICP_OPEN + DISPLAY_OPEN
    CAT_VIDEO_RENDER_DISPLAY_OPEN,                  //DISPLAY_OPEN = DISPLAY_CLOSE + SFRAMEMGR_OPEN + RENDER_CREATE
    CAT_VIDEO_RENDER_DISPLAY_CLOSE,
    CAT_VIDEO_RENDER_CREATE,
    CAT_VIDEO_RENDER_1ST_FRAME_GET_BEGIN,
    CAT_VIDEO_RENDER_1ST_FRAME_GET_END,
    CAT_VIDEO_RENDER_1ST_FRAME_PREPARE_BEGIN,
    CAT_VIDEO_RENDER_1ST_FRAME_PREPARE_END,
    CAT_VIDEO_RENDER_1ST_FRAME_DISPLAY_BEGIN,
    CAT_VIDEO_RENDER_1ST_FRAME_DISPLAY_END,

	CAT_VIDEO_MPEG2 = 200,
	CAT_VIDEO_MPEG2_OPEN,
	CAT_VIDEO_MPEG2_INIT_BEGIN,
	CAT_VIDEO_MPEG2_INIT_END,
	CAT_VIDEO_MPEG2_RESOLUTION_CHANGE_BEGIN,
	CAT_VIDEO_MPEG2_RESOLUTION_CHANGE_END,
	CAT_VIDEO_MPEG2_1ST_PLAY_BEGIN,
	CAT_VIDEO_MPEG2_1ST_PLAY_END,

	CAT_VIDEO_VC1 = 300,
	CAT_VIDEO_VC1_OPEN,
	CAT_VIDEO_VC1_INIT_BEGIN,
	CAT_VIDEO_VC1_INIT_END,
	CAT_VIDEO_VC1_RESOLUTION_CHANGE_BEGIN,
	CAT_VIDEO_VC1_RESOLUTION_CHANGE_END,
	CAT_VIDEO_VC1_1ST_PLAY_BEGIN,
	CAT_VIDEO_VC1_1ST_PLAY_END,

	CAT_VIDEO_H264 = 400,
	CAT_VIDEO_H264_OPEN,
	CAT_VIDEO_H264_INIT_BEGIN,
	CAT_VIDEO_H264_INIT_END,
	CAT_VIDEO_H264_RESOLUTION_CHANGE_BEGIN,
	CAT_VIDEO_H264_RESOLUTION_CHANGE_END,
	CAT_VIDEO_H264_1ST_PLAY_BEGIN,
	CAT_VIDEO_H264_1ST_PLAY_END,
	
	CAT_VIDEO_HVDSERVICE = 500,
    CAT_VIDEO_HVDSERVICE_INIT,
    CAT_VIDEO_HVDSERVICE_UNINIT,
    CAT_VIDEO_HVDSERVICE_START,

	CAT_VIDEO_EFFECT= 600,

	CAT_VIDEO_IVICP = 700,
	CAT_VIDEO_IVICP_START_SERVICE = 701,
    CAT_VIDEO_IVICP_OPEN,                           //IVICP_OPEN = HVDSERVICE_START + IVICP_DLL_OPEN
    CAT_VIDEO_IVICP_DLL_OPEN,
    CAT_VIDEO_IVICP_DLL_CLOSE,

    CAT_VIDEO_FRAMEMGR = 800,
    CAT_VIDEO_FRAMEMGR_OPEN,
    CAT_VIDEO_FRAMEMGR_CLOSE,
    CAT_VIDEO_SFRAMEMGR_OPEN,
    CAT_VIDEO_SFRAMEMGR_CLOSE,

    CAT_VIDEO_DISPSVR = 900,
    CAT_VIDEO_DISPSVR_1ST_FRAME_PREPARE_BEGIN,
    CAT_VIDEO_DISPSVR_1ST_FRAME_PREPARE_END,
    CAT_VIDEO_DISPSVR_1ST_FRAME_DISPLAY_BEGIN,
    CAT_VIDEO_DISPSVR_1ST_FRAME_DISPLAY_END,
    CAT_VIDEO_DISPSVR_DSHOW_1ST_PRESENTIMAGE_BEGIN,
    CAT_VIDEO_DISPSVR_DSHOW_1ST_PRESENTIMAGE_END,
};

enum AudioSubCategory
{
	CAT_AUDIO_GENERIC = 0,
	CAT_AUDIO_INITIAL = 1,
	CAT_AUDIO_PLAYBACK_STATUS, 

	CAT_AUDIO_DECODER = 20,
	CAT_AUDIO_LPCM,
	CAT_AUDIO_MPEGA,
	CAT_AUDIO_AC3,
	CAT_AUDIO_DDPLUS,
	CAT_AUDIO_TRUEHD,
	CAT_AUDIO_DTS,
	CAT_AUDIO_DTSHD,
	CAT_AUDIO_AAC,

	CAT_AUDIO_ENCODER = 40,
	CAT_AUDIO_AC3_ENCODER,
	CAT_AUDIO_DTS_ENCODER,

	CAT_AUDIO_BACKEND = 60,
	CAT_AUDIO_MIXER,
	CAT_AUDIO_RENDERER = 70,

	CAT_AUDIO_EFFECT = 80,	

	CAT_AUDIO_DECODE_BEGIN = 100,
	CAT_AUDIO_DECODE_END   = 101,

	CAT_AUDIO_MIXING_BEGIN,
	CAT_AUDIO_MIXING_END,
	CAT_AUDIO_EFFECT_BEGIN,
	CAT_AUDIO_EFFECT_END,
	CAT_AUDIO_ENCODE_BEGIN,
	CAT_AUDIO_ENCODE_END,
	CAT_AUDIO_RENDER_BEGIN,
	CAT_AUDIO_RENDER_END,
	CAT_AUDIO_DS_RENDER_BEGIN,
	
	CAT_AUDIO_RECEIVE_SAMPLE,
	CAT_AUDIO_DELIVER_SAMPLE,	
};


enum VCDNavSubCategory
{
	CAT_VCDNAV_GENERIC = 0,
};

enum DVDNavSubCategory
{
	CAT_DVDNAV_GENERIC = 0,
};

enum BDNavSubCategory
{
	CAT_BDNAV_GENERIC = 0,
	CAT_BDNAV_CREATE_METADATATEMPLATE,
	CAT_BDNAV_DECRYPT_AACS_RECORD,
	CAT_BDNAV_INIT_BDPLUS,
	CAT_BDNAV_LOAD_DUMPINFO,
	CAT_BDNAV_LOCALSTORAGEFILE_UPDATE,
	CAT_BDNAV_CONSTRUCT_BINDING_UNIT,
	CAT_BDNAV_DUMP_BTN_SOUND,
	CAT_BDNAV_CHECK_HAS_BDJ_TITLE,
	CAT_BDNAV_GPICTRL_OPEN,
};

enum BDJSubCategory
{
	CAT_BDJ_GENERIC = 0,
	CAT_BDJ_JARFS,
};

enum UISubCategory
{
	CAT_UI_GENERIC = 0,
	CAT_UI_LOAD_BASEWND,
	CAT_UI_INIT_PANELMGR,
	CAT_UI_LOAD_UIPLUGIN,
	CAT_UI_INIT_WINDVDX,
	CAT_UI_CREATE_LOGICLAYER,
	CAT_UI_LOAD_SKIN,
	CAT_UI_INIT_VIDEO_DEVICE,
};

enum AppSubCategory
{
	CAT_APP_GENERIC = 0,
	CAT_APP_LOGICLAYER_INIT,
	CAT_APP_INIT_IPM,
	CAT_APP_INIT_UPNP,
	CAT_APP_INIT_GPICTRL,
	CAT_APP_INIT_IPM_COMMON,
	CAT_APP_INIT_IPM_CREATE_MAIN_LICENSE,
	CAT_APP_INIT_IPM_CREATE_VARIANT_LICENSE,
	CAT_APP_INIT_IPM_SET_LANGTITLE,
	CAT_APP_INIT_IPM_AGREEDTOEULA,
	CAT_APP_INIT_IPM_AUTHENTICATION,
	CAT_APP_INIT_IPM_INVOKE_PCUUI,
	CAT_APP_INIT_IPM_CHECK_LICENSE,
	CAT_APP_INIT_IPM_ALL_REST,
	CAT_APP_NEW_PASM_LICENSE,
};

}//namespace dogsub

#endif // _DIAGNOSIS_OF_SUB_CATEGORY_H_

