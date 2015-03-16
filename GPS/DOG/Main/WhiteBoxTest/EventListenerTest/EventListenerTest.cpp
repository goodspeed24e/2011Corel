#include "EventListener.h"
#include "DataSource.h"

#include "DogCategory.h"
#include "SubCategory.h"
#include "DogEventId.h"

#include <gtest/gtest.h>

#pragma warning(disable: 4267) // warning C4267: 'argument' : conversion from 'size_t' to 'int', possible loss of data

namespace
{

class CEventListenerTest : public CEventListener
{
public:
};

class EventListenerTestFixture : public ::testing::Test
{
protected:
	EventListenerTestFixture()
	{}

	virtual ~EventListenerTestFixture()
	{}

	static void SetUpTestCase()
	{}

	static void TearDownTestCase()
	{}

	virtual void SetUp()
	{}

	virtual void TearDown()
	{}
};

TEST_F(EventListenerTestFixture, Basic)
{
	CEventListenerTest listener;

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

	{
		CDataSource* pDataSource = listener.GetDataSource(dog::AUDIO_DECODING_1ST_DATA_EVENT);
		ASSERT_TRUE( pDataSource != NULL);
		EXPECT_EQ( pDataSource->GetEventType(), dog::BUFFER_EVENT_TYPE);
		EXPECT_EQ( pDataSource->GetCount(), 100);

		for(int i=0; i<COUNT; ++i)
		{
			const DWORD bufCapacity = 100;
			const double val = sin(PI * i/45.0f);
			int datSize = (int)(bufCapacity * (val+1)/2);

			const dog::BufferEvent& bufEvent = (dog::BufferEvent&)pDataSource->GetEvent(i);
			EXPECT_EQ( bufEvent.DataSize, datSize);
			EXPECT_EQ( bufEvent.Capacity, bufCapacity );
		}
	}
	{
		CDataSource* pDataSource = listener.GetDataSource(dog::AUDIO_BACKEND_1ST_BUFFER_EVENT);
		ASSERT_TRUE( pDataSource != NULL);
		EXPECT_EQ( pDataSource->GetEventType(), dog::BUFFER_EVENT_TYPE);
		EXPECT_EQ( pDataSource->GetCount(), 100);

		for(int i=0; i<COUNT; ++i)
		{
			const DWORD bufCapacity = 100;
			const double val = sin(PI * i/45.0f);
			int datSize = (int)(bufCapacity * (val+1)/2);

			const dog::BufferEvent& bufEvent = (dog::BufferEvent&)pDataSource->GetEvent(i);
			EXPECT_EQ( bufEvent.DataSize, datSize);
			EXPECT_EQ( bufEvent.Capacity, bufCapacity );
		}
	}
}


TEST_F(EventListenerTestFixture, TestRemove)
{
	CEventListenerTest listener;

	listener.RemoveInterestedEventId(0);
	listener.ClearInterestedTable();
}


} // namespace

