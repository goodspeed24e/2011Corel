#pragma once

#include "DataSource.h"
#include <vector>
#include <string>
#include <map>

class CEventView
{
public:
	virtual void SetDataSource(CEventCollection* pEventCollection) = 0;
	virtual void AddEvent(DWORD id, std::basic_string<TCHAR> friendlyName) = 0;
	virtual void RemoveEvent(DWORD id) = 0;
	virtual std::vector<DWORD> GetEventList() = 0;
	virtual void Clear() = 0;
	virtual void Update() = 0;
};
