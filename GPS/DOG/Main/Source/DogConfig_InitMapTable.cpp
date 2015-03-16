#include "DogConfig.h"
#include "DogCategory.h"
#include "SubCategory.h"

// namespace Diagnosis-of-GPS
namespace dog
{
using namespace dogsub;

void CDogConfig::InitConfigStringMapTable()
{
	//---------------------------------------
	// DogFunction
	m_DogFunctionNameMap["log"]      = EVENT_LOG;
	m_DogFunctionNameMap["profiler"] = PROFILER;

	//---------------------------------------
	// output
	m_OutputNameMap["dogview"] = DOG_VIEW;
	m_OutputNameMap["file"] = DOG_LOG_FILE;
	m_OutputNameMap["debugview"] = DEBUG_VIEW;	

	//---------------------------------------
	// category
	m_CategoryNameMap["drm"]   = CAT_DRM;
	m_CategoryNameMap["demux"] = CAT_DEMUX;
	m_CategoryNameMap["video"] = CAT_VIDEO;
	m_CategoryNameMap["audio"] = CAT_AUDIO;

	m_CategoryNameMap["vcdnav"]= CAT_VCDNAV;
	m_CategoryNameMap["dvdnav"]= CAT_DVDNAV;

	m_CategoryNameMap["bdnav"] = CAT_BDNAV;
	m_CategoryNameMap["bdj"]   = CAT_BDJ;

	m_CategoryNameMap["ui"]    = CAT_UI;

	//---------------------------------------
	// sub-category
	
	m_SubCategoryNameMap[CAT_AUDIO]["generic"]  = CAT_AUDIO_GENERIC;
	m_SubCategoryNameMap[CAT_AUDIO]["renderer"] = CAT_AUDIO_RENDERER;
	m_SubCategoryNameMap[CAT_AUDIO]["lpcm"]   = CAT_AUDIO_LPCM;
	m_SubCategoryNameMap[CAT_AUDIO]["ac3"]    = CAT_AUDIO_AC3;
	m_SubCategoryNameMap[CAT_AUDIO]["ddplus"] = CAT_AUDIO_DDPLUS;
	m_SubCategoryNameMap[CAT_AUDIO]["truehd"] = CAT_AUDIO_TRUEHD;
	m_SubCategoryNameMap[CAT_AUDIO]["dts"]    = CAT_AUDIO_DTS;
	m_SubCategoryNameMap[CAT_AUDIO]["dtshd"]  = CAT_AUDIO_DTSHD;
	m_SubCategoryNameMap[CAT_AUDIO]["aac"]    = CAT_AUDIO_AAC;
	m_SubCategoryNameMap[CAT_AUDIO]["mpega"]  = CAT_AUDIO_MPEGA;
	m_SubCategoryNameMap[CAT_AUDIO]["ac3enc"] = CAT_AUDIO_AC3_ENCODER;
	m_SubCategoryNameMap[CAT_AUDIO]["dtsenc"] = CAT_AUDIO_DTS_ENCODER;
	m_SubCategoryNameMap[CAT_AUDIO]["effect"] = CAT_AUDIO_EFFECT;

}



} // namespace dog
