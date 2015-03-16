#include "stdafx.h"
#include "CUserSetting.h"

CUserSetting::CUserSetting()
{
}

CUserSetting::~CUserSetting()
{
	ClearSettingVector();
}

//vector<struct DogEvent> CUserSetting::LoadIni(LPCTSTR IniPath)
void CUserSetting::LoadIni(LPCTSTR IniPath)
{
	CStdioFile file;

	if (file.Open(IniPath, CFile::modeRead) > 0)
	{
		CString strRead;

		while (file.ReadString(strRead) > 0)
		{
			int nEventId = _ttoi(strRead);

			if (nEventId > 0 && file.ReadString(strRead) > 0)
			{
				UI::DogEvent tmpDogEvent;

				tmpDogEvent.dogId = nEventId;

#ifdef  UNICODE
				int len = strRead.GetLength() * 2;
#else
				int len = strRead.GetLength();
#endif
				tmpDogEvent.pEventName = new TCHAR[len + 1];

				_tcscpy_s(tmpDogEvent.pEventName, len, strRead);
				m_vecIniSetting.push_back(tmpDogEvent);
			}
		}

		file.Close();
	}
}

void CUserSetting::SaveIni(LPCTSTR IniPath)
{
	CStdioFile file;

	if (m_vecIniSetting.size() >= 0 && file.Open(IniPath, CFile::modeWrite | CFile::modeCreate) > 0)
	{
		vector<struct UI::DogEvent>::iterator it = m_vecIniSetting.begin();
		CString strRead;

		for (it; it != m_vecIniSetting.end(); ++it)
		{
			strRead.Format(_T("%d\n"), it->dogId);
			file.WriteString(strRead);
			file.WriteString(it->pEventName);
			file.WriteString(_T("\n"));
		}

		file.Close();
	}
}

vector<struct UI::DogEvent>& CUserSetting::GetSettingVector()
{
	return m_vecIniSetting;
}

void CUserSetting::ClearSettingVector()
{
	if (m_vecIniSetting.size() > 0)
	{
		DataVector::iterator it = m_vecIniSetting.begin();

		for (it; it != m_vecIniSetting.end(); ++it)
		{
			delete((UI::DogEvent(*it)).pEventName);
		}

		m_vecIniSetting.clear();
	}
}

void CUserSetting::AddToSettingVector(const UI::DogEvent& dogEvent)
{
	m_vecIniSetting.push_back(dogEvent);
}

void CUserSetting::RemoveFromSettingVector(const CString& str)
{
	if (m_vecIniSetting.size() <= 0)
		return;

	DataVector::iterator it = m_vecIniSetting.begin();

	for (it; it != m_vecIniSetting.end(); ++it)
	{
		if (0 == str.Compare(static_cast<CString> ((*it).pEventName)))
		{
			delete (*it).pEventName;
			m_vecIniSetting.erase(it);
			break;
		}
	}
}