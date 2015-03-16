#ifndef _CONTENT_PROTECTION_DEFINES_
#define _CONTENT_PROTECTION_DEFINES_

//////////////////////////////////////////////////////////
//	General definitions
#ifdef SAFE_RELEASE
#undef SAFE_RELEASE
#endif
#define SAFE_RELEASE(p) if((p)!=NULL) {(p)->Release(); (p)=NULL;}

#ifdef SAFE_DELETE
#undef SAFE_DELETE
#endif
#define SAFE_DELETE(p) if((p)!=NULL) {delete (p); (p)=NULL;}

#ifdef SAFE_DELETE_ARRAY
#undef SAFE_DELETE_ARRAY
#endif
#define SAFE_DELETE_ARRAY(p) if((p)!=NULL) {delete[] (p); (p)=NULL;}

enum EGPUCPOption
{
	E_CP_OPTION_OFF = 0,
	E_CP_OPTION_ON = 1,
	E_CP_OPTION_AUTO = 1<<1,
	E_CP_OPTION_DBG_MSG = 1<<2,
	E_CP_OPTION_I_ONLY = 1<<3,
	E_CP_OPTION_AUDIO = 1<<8
};

enum GPUVenderID
{
	VENDER_ID_DEFAULT = 0,
	VENDER_ID_NVIDIA = 1,
	VENDER_ID_ATI = 2,
	VENDER_ID_INTEL = 3,
	VENDER_ID_S3 = 4
};

#endif