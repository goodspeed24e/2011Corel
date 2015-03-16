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
	CAT_APP	  = 50,
};

} // namespace dog

#endif // _DIAGNOSIS_OF_GPS_CATEGORY_H_

