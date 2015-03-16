#pragma warning (disable:4819)
//boost_1_35_0\include\boost\utility\enable_if.hpp : warning C4819: The file contains a character that cannot be represented in the current code page (950). Save the file in Unicode format to prevent data loss

#include <tchar.h>

#include <boost/test/unit_test.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/format.hpp>

#include "EventListener.h"
#include "DataSource.h"

#include "DogCategory.h"
#include "SubCategory.h"
#include "DogEventId.h"

namespace EventListenerBoostTest {


void CheckMemoryLeak()
{
	CEventListener listener;

	listener.AddInterestedEventId(dog::DEBUG_MESSAGE_EVENT);
	listener.AddInterestedEventId(dog::AUDIO_DECODING_1ST_DATA_EVENT);
	listener.AddInterestedEventId(dog::AUDIO_BACKEND_1ST_BUFFER_EVENT);


	listener.NotifyEvent(dog::DebugMessageEvent(0,0, "[AnsiString] HelloDog"));
	listener.NotifyEvent(dog::DebugMessageEvent(0,0, L"[UnicodeString] HelloDog"));

	const int COUNT = 100;
	const double PI = 3.14159265358979323846;
	for(int i=0; i<COUNT; ++i)
	{
		const DWORD bufCapacity = 100;
		const double val = sin(PI * i/45.0f);
		int datSize = (int)(bufCapacity * (val+1)/2);
		listener.NotifyEvent(dog::BufferEvent(dog::CAT_AUDIO, dogsub::CAT_AUDIO_PLAYBACK_STATUS, dog::AUDIO_DECODING_1ST_DATA_EVENT, 0, 0, 0, datSize, datSize, bufCapacity));
		listener.NotifyEvent(dog::BufferEvent(dog::CAT_AUDIO, dogsub::CAT_AUDIO_PLAYBACK_STATUS, dog::AUDIO_BACKEND_1ST_BUFFER_EVENT, 0, 0, 0, datSize, datSize, bufCapacity));
	}
}


} // namespace EventListenerBoostTest

//============================================================================
boost::unit_test::test_suite* make_EventListenerBoostTest_suite()
{
	using namespace EventListenerBoostTest;

	boost::unit_test_framework::test_suite* suite = NULL;
	suite = BOOST_TEST_SUITE("EventListenerBoostTest");
	{
		suite->add( BOOST_TEST_CASE( CheckMemoryLeak ));
	}
	return suite;
}
