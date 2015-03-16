#include "DOG\DOG.h"

namespace dogproxy
{
	void SetupDogConfigByFile(const char* filename);
	void SetupDogConfigByString(const char* szConfigString);
	bool GetEnableFlag(DWORD dwCategory, DWORD dwSubCategory);
	void WriteEvent(const dog::DogEvent& dogEvent);
}

#define DOG_WATCH_LOCAL_TIME_BEGIN \
	LARGE_INTEGER liPerfFreq, liPerfBegin, liPerfEnd; \
	::QueryPerformanceFrequency(&liPerfFreq); \
	::QueryPerformanceCounter(&liPerfBegin); 

#define DOG_WATCH_LOCAL_TIME_END(cat, subcat, id) \
	::QueryPerformanceCounter(&liPerfEnd); \
	int iTime = (int)(((liPerfEnd.QuadPart - liPerfBegin.QuadPart) * 1000.0) / liPerfFreq.QuadPart); \
	dogproxy::WriteEvent(dog::VariantDataEvent(cat, subcat, id, iTime));

