#pragma warning(disable:4995) // warning C4995: 'gets': name was marked as #pragma deprecated

#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include <atlbase.h>

#define LOKI_CLASS_LEVEL_THREADING
#include <loki\threads.h>
#include <loki\singleton.h>

#include "DOGEventProvider.h"

//#include "DOG\DOG.h"

#include "DOG\DogCategory.h"
#include "DOG\SubCategory.h"
#include "DOG\DogUserEvent.h"

#include <string>

#ifndef _delayimp_h
extern "C" IMAGE_DOS_HEADER __ImageBase;
#endif

//-----------------------------------------------------------------------------
inline dog::IEventTracing* CreateDOGFromLocalDirDLL(HINSTANCE* phDLL)
{
	const TCHAR* DOGDLL_FileName = L"DOG.DLL";

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

//-----------------------------------------------------------------------------
class CDogModule
{
public:
	CDogModule() : m_hDOGDLL(NULL)
	{
		m_pDOG = CreateDOGFromLocalDirDLL(&m_hDOGDLL);
		if(!m_pDOG) {
			printf("CreateDOGFromLocalDirDLL fail!\n");
		}
	}

	~CDogModule()
	{
		m_pDOG = NULL;
		::FreeLibrary(m_hDOGDLL);
	}

	bool IsAvailable() const 
	{
		return m_pDOG!=NULL;
	}

	dog::IEventTracing* operator->() const 
	{
		return m_pDOG;
	}

protected:
	HINSTANCE m_hDOGDLL;
	CComPtr<dog::IEventTracing> m_pDOG;
};

//-----------------------------------------------------------------------------
// singleton holder
typedef Loki::SingletonHolder<CDogModule, Loki::CreateUsingNew, Loki::DefaultLifetime, Loki::ClassLevelLockable> DogSingleton;


//-----------------------------------------------------------------------------
namespace dogproxy
{
	void SetupDogConfigByFile(const char* filename)
	{
		if(!DogSingleton::Instance().IsAvailable()) {
			return;
		}
		DogSingleton::Instance()->SetupDogConfigByFile(filename);
	}

	void SetupDogConfigByString(const char* szConfigString)
	{
		if(!DogSingleton::Instance().IsAvailable()) {
			return;
		}
		DogSingleton::Instance()->SetupDogConfigByString(szConfigString);
	}

	bool GetEnableFlag(DWORD dwCategory, DWORD dwSubCategory)
	{
		if(!DogSingleton::Instance().IsAvailable()) {
			return false;
		}
		return DogSingleton::Instance()->GetEnableFlag(dwCategory, dwSubCategory);
	}	

	void WriteEvent(const dog::DogEvent& dogEvent)
	{
		if(!DogSingleton::Instance().IsAvailable()) {
			return;
		}
		DogSingleton::Instance()->WriteEvent(dogEvent);
	}

} // namespace dogproxy
