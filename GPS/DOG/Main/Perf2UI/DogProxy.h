#include "DOG.h"
#include "DogCategory.h"
#include "SubCategory.h"
#include "DogUserEvent.h"

namespace dogproxy
{
	void SetupDogConfigByFile(const char* filename);
	void SetupDogConfigByString(const char* szConfigString);

	void RegisterListener(dog::IEventListener* listener);
	void UnregisterListener(dog::IEventListener* listener);
}

