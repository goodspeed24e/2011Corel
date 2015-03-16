#ifndef CUserSetting_h__
#define CUserSetting_h__

#include "Perf2.h"
#include "structData.h"
#include <vector>
using namespace std;

class CUserSetting
{
public:
	typedef vector<struct UI::DogEvent> DataVector;
	CUserSetting();
	~ CUserSetting();
	void LoadIni(LPCTSTR IniPath);
	void SaveIni(LPCTSTR IniPath);
	DataVector& GetSettingVector();
	void ClearSettingVector();
	void AddToSettingVector(const UI::DogEvent& dogEvent);
	void RemoveFromSettingVector(const CString& str);
	
private:
    CUserSetting(const CUserSetting& rhs);
	DataVector m_vecIniSetting;
};

#endif // CUserSetting_h__
