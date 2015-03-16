#ifndef MyApp_h__
#define MyApp_h__

#include <string>
#include <atlbase.h>       // base ATL classes
//#include <atlapp.h>        // base WTL classes
//#include <atlwin.h>        // ATL GUI classes
//#include <atlframe.h>      // WTL frame window classes
//#include <atlmisc.h>       // WTL utility classes like CString
//#include <atlcrack.h>      // WTL enhanced msg map macros
#include <map>
#include "CChartData.h"
#include "CUserSetting.h"

#include "DOG.h"
#include "DogCategory.h"
#include "SubCategory.h"
#include "DogUserEvent.h"
#include "EventListener.h"
#include "structData.h"
//#include "DogEventId.h"

using namespace std;

class MyApp : public CWinApp
{

public:
	MyApp();
	BOOL InitInstance();
	
	//map DogEventId to CChartData*
	map<DWORD, CChartData*> m_mapChartData;
	vector<struct UI::DogEvent>& GetSettingVector();
	void UpdateSettingVector(vector<struct UI::DogEvent>& vectorSource);
	void ClearSettingVector();
	void AddToSettingVector(const UI::DogEvent& dogEvent);
	void RemoveFromSettingVector(const CString& str);

private:
	CComPtr<dog::IEventTracing> m_pDOG;
	CUserSetting m_UserSetting;
	HINSTANCE m_hDOGDLL;
	CEventListener m_EventListener;
	CChartData* m_pChartData;
	virtual int ExitInstance();
};

inline dog::IEventTracing* CreateDOGFromLocalDirDLL(HINSTANCE* phDLL)
{
	const TCHAR* DOGDLL_FileName = _T("DOG.DLL");

	std::basic_string<TCHAR> ModulePath(MAX_PATH, NULL);
	::GetModuleFileName(reinterpret_cast<HMODULE>(&__ImageBase), &ModulePath[0], MAX_PATH);
	ModulePath.replace(ModulePath.begin() + ModulePath.find_last_of(_T('\\')) + 1, ModulePath.end(), DOGDLL_FileName);
	HINSTANCE hDLL = LoadLibrary(ModulePath.c_str());

	dog::IEventTracing* pDOG = NULL;

	if(hDLL)
	{
		typedef void* (*LPCreateComponentT)(void);
		LPCreateComponentT pCreateComponent = reinterpret_cast<LPCreateComponentT>(GetProcAddress(hDLL, "CreateComponent"));
		if(pCreateComponent)
		{
			pDOG = reinterpret_cast<dog::IEventTracing*>(pCreateComponent());
		}
		else
		{
			::FreeLibrary(hDLL);
			hDLL = NULL;
		}
	}

	if(phDLL)
	{
		*phDLL = hDLL;
	}

	return pDOG;
}

#endif // MyApp_h__
