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

#endif