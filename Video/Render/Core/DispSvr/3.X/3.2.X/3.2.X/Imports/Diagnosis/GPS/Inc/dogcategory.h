#ifndef _DIAGNOSIS_OF_GPS_CATEGORY_H_
#define _DIAGNOSIS_OF_GPS_CATEGORY_H_

// namespace Diagnosis-of-GPS
namespace dog
{

enum Category
{
	CAT_GENERIC = 0,
	CAT_DRM   = 1,
	CAT_DEMUX = 2,
	CAT_VIDEO = 3,
	CAT_AUDIO = 4,

	CAT_VCDNAV= 10,
	CAT_DVDNAV= 11,

	CAT_BDNAV = 20,
	CAT_BDJ   = 21,

	CAT_UI    = 40,
	CAT_APP   = 50,
};

}// namespace dog

#endif // _DIAGNOSIS_OF_GPS_CATEGORY_H_



/*
enum GenericSubCategory
{
	CAT_GENERIC_NONE = 0,
};

enum DRMSubCategory
{
	CAT_DRM_GENERIC = 0,
};

enum DemuxSubCategory
{
	CAT_DEMUX_GENERIC = 0,
};

enum VideoSubCategory
{
	CAT_VIDEO_GENERIC = 0,
	CAT_VIDEO_RENDERER= 1,

	CAT_VIDEO_MPEG2 = 20,
	CAT_VIDEO_VC1   = 21,
	CAT_VIDEO_H264  = 22,

	CAT_VIDEO_EFFECT= 40,
};

enum AudioSubCategory
{
	CAT_AUDIO_GENERIC = 0,
	CAT_AUDIO_RENDERER= 1,

	CAT_AUDIO_LPCM   = 20,
	CAT_AUDIO_AC3    = 21,
	CAT_AUDIO_DDPLUS = 22,
	CAT_AUDIO_TRUEHD = 23,
	CAT_AUDIO_DTS    = 24,
	CAT_AUDIO_DTSHD  = 25,
	CAT_AUDIO_AAC    = 26,
	CAT_AUDIO_MPEGA  = 27,

	CAT_AUDIO_AC3_ENCODER = 40,
	CAT_AUDIO_DTS_ENCODER = 41,

	CAT_AUDIO_EFFECT = 60,
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
};

enum BDJSubCategory
{
	CAT_BDJ_GENERIC = 0,
};

enum UISubCategory
{
	CAT_UI_GENERIC = 0,
};

} // namespace dog
*/


