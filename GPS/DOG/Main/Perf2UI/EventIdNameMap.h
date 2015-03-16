#pragma once

#include <Windows.h>
#include <tchar.h>
#include <map>

typedef std::map<DWORD, std::basic_string<TCHAR> > TEventIdNameMap;

//-----------------------------------------------------------------------------
class CEventIdNameMap
{
public:
	void Init();
	std::basic_string<TCHAR> GetFriendlyEventName(DWORD eventId);

	TEventIdNameMap m_DemuxEvents;
	TEventIdNameMap m_AudioEvents;
	TEventIdNameMap m_VideoEvents;
	TEventIdNameMap m_DisplayEvents;

	TEventIdNameMap m_TotalEvents;

	TEventIdNameMap m_UserDefineEvents; // user customized events
};


//-----------------------------------------------------------------------------
extern CEventIdNameMap g_EventIdNameMap;
