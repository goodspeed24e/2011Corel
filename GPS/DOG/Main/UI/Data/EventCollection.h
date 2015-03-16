#pragma once

#include "DOG.h"
#include "DogUserEvent.h"
#include "AsyncCircularBuffer.h"
#include "DataSource.h"
#include <map>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

#include <assert.h>

/** 
 * CEventCollection contains circular buffers.
 */
class CEventCollection : boost::noncopyable
{
public:
	CEventCollection()
	{
		InitializeCriticalSection(&m_cs);

		m_BasicEventMap.SetOwner(this);

		m_MessageEventMap.SetOwner(this);
		m_VariantDataEventMap.SetOwner(this);
		m_BufferEventMap.SetOwner(this);

		m_VideoFrameEventMap.SetOwner(this);

		m_AudioStreamInfoUpdateEventMap.SetOwner(this);
		m_AudioOutputUpdateEventMap.SetOwner(this);
		m_AudioDevCapUpdateEventMap.SetOwner(this);
	}

	~CEventCollection()
	{
		Clear();
		DeleteCriticalSection(&m_cs);
	}

	/// Push events to its associated buffer
	void Push(const dog::DogEvent& event)
	{
		using namespace dog;
		CAutoLock loc(&m_cs);

		switch(event.DataType)
		{
		default:
		case BASIC_EVENT_TYPE:
			m_BasicEventMap.Push(event);
			break;

		case STRING_MESSAGE_EVENT_TYPE:
			m_MessageEventMap.Push(event);
			break;
		case VARIANT_DATA_EVENT_TYPE:
			m_VariantDataEventMap.Push(event);
			break;
		case DATA_PROCESS_EVENT_TYPE:
		case BUFFER_EVENT_TYPE:
			m_BufferEventMap.Push(event);
			break;

		case VIDEO_FRAME_EVENT_TYPE:
			m_VideoFrameEventMap.Push(event);
			break;

		case AUDIO_STREAM_INFO_UPDATE_EVENT_TYPE:
			m_AudioStreamInfoUpdateEventMap.Push(event);
			break;
		case AUDIO_OUTPUT_UPDATE_EVENT_TYPE:
			m_AudioOutputUpdateEventMap.Push(event);
			break;
		case AUDIO_DEVICE_CAPABILITY_UPDATE_EVENT_TYPE:
			m_AudioDevCapUpdateEventMap.Push(event);
			break;
		}
	}

	/// Remove data in EventCollection with specified eventId
	void Erase(DWORD eventId)
	{
		CAutoLock loc(&m_cs);
		EventDataSourceMapErase(eventId);

		m_BasicEventMap.erase(eventId);

		m_MessageEventMap.erase(eventId);
		m_VariantDataEventMap.erase(eventId);
		m_BufferEventMap.erase(eventId);

		m_VideoFrameEventMap.erase(eventId);

		m_AudioStreamInfoUpdateEventMap.erase(eventId);
		m_AudioOutputUpdateEventMap.erase(eventId);
		m_AudioDevCapUpdateEventMap.erase(eventId);
	}

	/// Remove all EventCollection
	void Clear()
	{
		CAutoLock loc(&m_cs);
		EventDataSourceMapClear();

		m_BasicEventMap.clear();

		m_MessageEventMap.clear();
		m_VariantDataEventMap.clear();
		m_BufferEventMap.clear();

		m_VideoFrameEventMap.clear();

		m_AudioStreamInfoUpdateEventMap.clear();
		m_AudioOutputUpdateEventMap.clear();
		m_AudioDevCapUpdateEventMap.clear();
	}

	/// Returns the CDataSource with the specified event Id.
	/// If no CDataSource associated with the specified event Id, this method will return NULL.
	CDataSource* GetDataSource(DWORD eventId)
	{
		CAutoLock loc(&m_cs);
		const std::map<DWORD, CDataSource*>::const_iterator iter = m_EventDataSourceMap.find(eventId);
		if(iter != m_EventDataSourceMap.end())
		{
			return iter->second;
		}
		return NULL;
	}

protected:
	mutable CRITICAL_SECTION m_cs;

	class CAutoLock
	{
		CRITICAL_SECTION* m_pcs;
	public:
		CAutoLock(CRITICAL_SECTION* cs) : m_pcs(cs) { EnterCriticalSection(m_pcs); }
		~CAutoLock() { LeaveCriticalSection(m_pcs); }
	};


protected:
	std::map<DWORD, CDataSource* > m_EventDataSourceMap;

	void RegisterEventDataSourceMap(DWORD eventId, CDataSource* pDataSource)
	{
		m_EventDataSourceMap[eventId] = pDataSource;
	}

	void EventDataSourceMapErase(DWORD eventId)
	{
		std::map<DWORD, CDataSource* >::iterator iter = m_EventDataSourceMap.find(eventId);
		if(iter != m_EventDataSourceMap.end())
		{
			delete iter->second;
			iter->second = NULL;
		}
		m_EventDataSourceMap.erase(eventId);
	}

	void EventDataSourceMapClear()
	{
		std::map<DWORD, CDataSource* >::iterator iter = m_EventDataSourceMap.begin();
		for(; iter!=m_EventDataSourceMap.end(); ++iter)
		{
			delete iter->second;
			iter->second = NULL;
		}
		m_EventDataSourceMap.clear();
	}


protected:
	/// EventBufferMap< EventId, TAsyncCircularBuffer >
	template<typename EventT, size_t QUEUE_SIZE>
	class EventBufferMap : public std::map<DWORD, TAsyncCircularBuffer<EventT>* >, boost::noncopyable
	{
		CEventCollection* m_pOwner;
	public:
		EventBufferMap() : m_pOwner(NULL)
		{}

		void SetOwner(CEventCollection* pOwner)
		{
			m_pOwner = pOwner;
		}

		inline void Push(const dog::DogEvent& event)
		{
			iterator iter = find(event.Id);
			if(iter == end())
			{
				//create a new TAsyncCircularBuffer for new event id
				CDataSourceImpl<EventT>* pDataSource = new CDataSourceImpl<EventT>(QUEUE_SIZE);
				insert( std::pair<DWORD, TAsyncCircularBuffer<EventT>*>(event.Id, pDataSource));
				
				assert(m_pOwner);
				m_pOwner->RegisterEventDataSourceMap(event.Id, pDataSource);

				pDataSource->Push((EventT&)event);
			}
			else
			{
				iter->second->Push((EventT&)event);
			}
		}
	};


	EventBufferMap<dog::DogEvent, 500> m_BasicEventMap;

 	EventBufferMap<dog::StringEvent, 500>      m_MessageEventMap;
 	EventBufferMap<dog::VariantDataEvent, 500> m_VariantDataEventMap;
 	EventBufferMap<dog::BufferEvent, 500>      m_BufferEventMap;

	EventBufferMap<dog::VideoFrameEvent, 500>  m_VideoFrameEventMap;

	EventBufferMap<dog::AudioStreamInfoUpdateEvent, 50> m_AudioStreamInfoUpdateEventMap;
	EventBufferMap<dog::AudioOutputUpdateEvent, 50>     m_AudioOutputUpdateEventMap;
	EventBufferMap<dog::AudioDevCapUpdateEvent, 50>     m_AudioDevCapUpdateEventMap;
};

