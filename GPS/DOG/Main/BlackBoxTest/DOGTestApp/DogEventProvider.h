#include "DOG.h"
#include "DogCategory.h"
#include "SubCategory.h"
#include "DogUserEvent.h"

namespace dogproxy
{
	void SetupDogConfigByFile(const char* filename);
	void SetupDogConfigByString(const char* szConfigString);
	bool GetEnableFlag(DWORD dwCategory, DWORD dwSubCategory);
	void WriteEvent(const dog::DogEvent& dogEvent);
}
