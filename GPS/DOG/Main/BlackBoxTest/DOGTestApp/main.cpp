#pragma warning(disable:4995) // warning C4995: 'gets': name was marked as #pragma deprecated

#include <math.h>

#include "DOGEventProvider.h"
#include "DogCategory.h"
#include "SubCategory.h"
#include "DogEventId.h"
#include "DogUserEvent.h"

#include <tchar.h>

int main( int argc, char* argv[] )
{
	dogproxy::SetupDogConfigByString("log = all");

	dogproxy::WriteEvent(dog::DebugMessageEvent(0,0, "[AnsiString] HelloDog"));
	dogproxy::WriteEvent(dog::DebugMessageEvent(0,0, L"[UnicodeString] HelloDog"));

	const double PI = 3.14159265358979323846;
	for(int i=0; ; ++i)
	{
		int datSize_1 = 100 * (sin(PI * i/45.0f) +1)/2;
		int datSize_2 = 100 * ( cos(PI * i/45.0f) +1)/2;

		dogproxy::WriteEvent(dog::BufferEvent(dog::CAT_AUDIO, dogsub::CAT_AUDIO_PLAYBACK_STATUS, dog::AUDIO_DECODING_1ST_DATA_EVENT, 0, 0, 0, datSize_1, datSize_1, 100));
		dogproxy::WriteEvent(dog::BufferEvent(dog::CAT_AUDIO, dogsub::CAT_AUDIO_PLAYBACK_STATUS, dog::AUDIO_BACKEND_1ST_BUFFER_EVENT, 0, 0, 0, datSize_2, datSize_2, 100));

		TCHAR strMsg[512];
		_stprintf_s(strMsg, sizeof(strMsg)/sizeof(TCHAR), _T("Test message: dat=%d"), datSize_1);
		dogproxy::WriteEvent(dog::DebugMessageEvent(0,0, strMsg));

		Sleep(1);
	}

	return 0;
}
