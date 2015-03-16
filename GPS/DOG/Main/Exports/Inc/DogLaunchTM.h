#ifndef _DIAGNOSIS_OF_LAUNCH_TIME_H_
#define _DIAGNOSIS_OF_LAUNCH_TIME_H_

#ifdef _UNICODE
#define _DOG_DEFINED_POINT( NAME, GUID ) \
	const wchar_t g___dog_##NAME[] = L#GUID;
#else
#define _DOG_DEFINED_POINT( NAME, GUID ) \
	const char g___dog_##NAME[] = #GUID;
#endif

#define _DOG_DEFINED_NAME( NAME ) g___dog_##NAME

#if defined(ENABLE_DOG_PROFILE)
#define _DOG_SET_CHECK_POINT(STR_UNIQ_NAME, NUM) { \
	HANDLE h_Event##NUM = ::OpenEvent(EVENT_MODIFY_STATE, FALSE, STR_UNIQ_NAME); \
	if (h_Event##NUM) \
		::SetEvent(h_Event##NUM); \
		::CloseHandle(h_Event##NUM); \
	} 

#define _DOG_SET_CHECK_POINT_AT_FIRST_TIME(STR_UNIQ_NAME, NUM) { \
	static LONG nDogFlag##NUM = 0; \
	LONG nDogInitFlag##NUM = ::InterlockedCompareExchange(&nDogFlag##NUM, 1, 0); \
	if (nDogFlag##NUM != nDogInitFlag##NUM) { \
		HANDLE h_Event##NUM = ::OpenEvent(EVENT_MODIFY_STATE, FALSE, STR_UNIQ_NAME); \
		if (h_Event##NUM) \
			::SetEvent(h_Event##NUM); \
			::CloseHandle(h_Event##NUM); \				
		} \
	}

#define DOG_SET_CHECK_POINT(STR_UNIQ_GUID_NAME) _DOG_SET_CHECK_POINT(STR_UNIQ_GUID_NAME, __LINE__)
#define DOG_SET_CHECK_POINT_AT_FIRST_TIME(STR_UNIQ_GUID_NAME) _DOG_SET_CHECK_POINT_AT_FIRST_TIME(STR_UNIQ_GUID_NAME, __LINE__)

#define DOG_DEFINED_CHECK_POINT( DEF_NAME, DEF_GUID ) _DOG_DEFINED_POINT(DEF_NAME, DEF_GUID)
#define DOG_DEFINED_POINT_NAME( DEF_NAME ) _DOG_DEFINED_NAME(DEF_NAME)

#else

#define _DOG_SET_CHECK_POINT(STR_UNIQ_NAME, NUM)
#define _DOG_SET_CHECK_POINT_AT_FIRST_TIME(STR_UNIQ_NAME, NUM)  

#define DOG_SET_CHECK_POINT(STR_UNIQ_GUID_NAME)  
#define DOG_SET_CHECK_POINT_AT_FIRST_TIME(STR_UNIQ_GUID_NAME)  

#define DOG_DEFINED_CHECK_POINT( DEF_NAME, DEF_GUID )  
#define DOG_DEFINED_POINT_NAME( DEF_NAME )  

#endif

#include "DogLaunchTM_DEF.h"

#endif // _DIAGNOSIS_OF_LAUNCH_TIME_H_