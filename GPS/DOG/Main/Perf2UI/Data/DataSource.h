#pragma once

#include "DOG.h"
#include "DogUserEvent.h"
#include "AsyncCircularBuffer.h"
#include <windows.h>

class CDataSource
{
public:
	CDataSource() {}
	virtual ~CDataSource() {}

	virtual DWORD  GetEventType() const = 0;
 	virtual size_t GetCount() const = 0;
	virtual const dog::DogEvent& GetEvent(int idx) const = 0;
	virtual void Pop() = 0;
};

template<typename EventT>
class CDataSourceImpl : public CDataSource, public TAsyncCircularBuffer<EventT>
{
	typedef TAsyncCircularBuffer<EventT> TBufferClass;
public:
	CDataSourceImpl(size_t nQsize) : TBufferClass(nQsize) 
	{}

	DWORD GetEventType() const
	{
		return dog::BASIC_EVENT_TYPE;
	}

	size_t GetCount() const
	{
		return TBufferClass::GetSize();
	}

	const dog::DogEvent& GetEvent(int idx) const
	{
		return TBufferClass::operator[](idx);
	}

	void Pop()
	{
		TBufferClass::Pop();
	}
};


#define DATASOURCE_TEMPLATE_SPECIALIZATION(EventType, EventTypeEnum) \
template<> \
class CDataSourceImpl< EventType > : public CDataSource, public TAsyncCircularBuffer<EventType> \
{                                                                                          \
	typedef TAsyncCircularBuffer::value_type EventT;                                       \
	typedef TAsyncCircularBuffer<EventT> TBufferClass;                                     \
public:                                                                                    \
	CDataSourceImpl(size_t nQsize) : TBufferClass(nQsize) {}                               \
	DWORD GetEventType() const { return EventTypeEnum; }                                   \
	size_t GetCount() const { return TBufferClass::GetSize(); }                            \
	const dog::DogEvent& GetEvent(int idx) const { return TBufferClass::operator[](idx); } \
	void Pop() { TBufferClass::Pop(); }                                                    \
};

DATASOURCE_TEMPLATE_SPECIALIZATION(dog::StringEvent,      dog::STRING_MESSAGE_EVENT_TYPE);
DATASOURCE_TEMPLATE_SPECIALIZATION(dog::VariantDataEvent, dog::VARIANT_DATA_EVENT_TYPE);
DATASOURCE_TEMPLATE_SPECIALIZATION(dog::BufferEvent,      dog::BUFFER_EVENT_TYPE);

DATASOURCE_TEMPLATE_SPECIALIZATION(dog::VideoFrameEvent,             dog::VIDEO_FRAME_EVENT_TYPE);
DATASOURCE_TEMPLATE_SPECIALIZATION(dog::AudioStreamInfoUpdateEvent,  dog::AUDIO_STREAM_INFO_UPDATE_EVENT_TYPE);
DATASOURCE_TEMPLATE_SPECIALIZATION(dog::AudioOutputUpdateEvent,      dog::AUDIO_OUTPUT_UPDATE_EVENT_TYPE);
DATASOURCE_TEMPLATE_SPECIALIZATION(dog::AudioDevCapUpdateEvent,      dog::AUDIO_DEVICE_CAPABILITY_UPDATE_EVENT_TYPE);
