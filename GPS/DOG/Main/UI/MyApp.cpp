#include "stdafx.h"
#include "MyApp.h"
#include "MainFrame.h"

// The only one application object
MyApp a_app;
#define DOGPROFILENAME "\\DOG_UI.ini"

MyApp::MyApp()
{
}

void GetIniPath(CString &str)
{
	//Load DOG_UI.ini setting to "m_vecIniSetting"
	TCHAR buffer[1024];
	memset(buffer, 0, sizeof(buffer));
	GetModuleFileName(NULL, buffer, _countof(buffer));

	CString strAppPath = buffer;
	int nBackslash = strAppPath.ReverseFind(_T('\\'));

	if (nBackslash != -1) {
		strAppPath = strAppPath.Left(nBackslash+1);
	}
	
	str = strAppPath + _T(DOGPROFILENAME);
}

BOOL MyApp :: InitInstance()
{
	//Load DOG_UI.ini setting to "m_vecIniSetting"
	CString strPath;
	GetIniPath(strPath);
	m_UserSetting.LoadIni(strPath);

	//create main window, perf window..
	CFrameWnd* Frame = new MainFrame(&m_EventListener);
	//RECT rect = {0,0,1024,768};
	RECT rect = {0,0,1280,768};
    Frame->Create(NULL, _T("DOG v0.5"), WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, rect);
    Frame->ShowWindow(SW_SHOW);
	m_pMainWnd = Frame;

	//start receiving data
	m_pDOG = CreateDOGFromLocalDirDLL(&m_hDOGDLL);
	if(!m_pDOG)
	{
		_tprintf(_T("CreateDOGFromLocalDirDLL fail!"));
		return -1;
	}
	m_pDOG->SetupDogConfigByString("log = all");
	m_pDOG->RegisterListener(&m_EventListener);

	return true;
};

int MyApp::ExitInstance()
{
	CString strPath;
	GetIniPath(strPath);
	m_UserSetting.SaveIni(strPath);

	if(m_pDOG)
	{
		// now Unregister would cause exception
		m_pDOG->UnregisterListener(&m_EventListener);
		m_pDOG = NULL;
	}
	if(m_hDOGDLL)
	{
		FreeLibrary(m_hDOGDLL);
		m_hDOGDLL = NULL;
	}

	for(map<DWORD, CChartData*>::iterator it = m_mapChartData.begin(); it != m_mapChartData.end(); it++)
	{
		if ((*it).second != NULL)
 		{
 			delete (*it).second;
			(*it).second = NULL;
 		}
	}
		
	return __super::ExitInstance();
}


vector<struct UI::DogEvent>& MyApp::GetSettingVector()
{
	return m_UserSetting.GetSettingVector();
}

void MyApp::ClearSettingVector()
{
	m_UserSetting.ClearSettingVector();
}

void MyApp::AddToSettingVector(const UI::DogEvent& dogEvent)
{
	m_UserSetting.AddToSettingVector(dogEvent);
}

void MyApp::RemoveFromSettingVector(const CString& str)
{
	m_UserSetting.RemoveFromSettingVector(str);
}