#pragma once

#include <string>

inline std::basic_string<TCHAR> GuidToString(const GUID& guid)
{
	TCHAR buf[64];	
	_stprintf_s(buf, sizeof(buf)/sizeof(TCHAR), _T("{%X-%X-%X-%X%X-%X%X%X%X%X%X}"),
		guid.Data1,
		guid.Data2,
		guid.Data3,
		guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);

	buf[sizeof(buf)-1]=0;
	return std::basic_string<TCHAR>(buf);
}

inline std::basic_string<TCHAR> GuidToStringEx(const GUID& guid, char format)
{
	TCHAR buf[128];

	switch(format)
	{
	default:
	case 'r':
	case 'R':
		// registry format
		// 32 digits separated by hyphens, enclosed in brackets:
		_stprintf_s(buf, sizeof(buf)/sizeof(TCHAR), _T("{%X-%X-%X-%X%X-%X%X%X%X%X%X}"),
			guid.Data1,
			guid.Data2,
			guid.Data3,
			guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
		break;
	case 's':
	case 'S':
		// structure
		// static const GUID = {...}
		_stprintf_s(buf, sizeof(buf)/sizeof(TCHAR), _T("{ %#x, %#x, %#x, { %#x, %#x, %#x, %#x, %#x, %#x, %#x, %#x } }"),
			guid.Data1,
			guid.Data2,
			guid.Data3,
			guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
		break;
	}

	buf[sizeof(buf)-1]=0;
	return std::basic_string<TCHAR>(buf);
}
