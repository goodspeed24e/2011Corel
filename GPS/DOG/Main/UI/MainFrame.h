#ifndef MainFrame_h__
#define MainFrame_h__

#include "DOG.h"
#include "MyApp.h"
#include "CPerfWnd.h"
#include "CPerfPainterWnd.h"
#include "EventListener.h"
//#include "CChartData.h"

using namespace std;

class MainFrame : public CFrameWnd
{
public:
    MainFrame(CEventListener* pEventListener);

	CList<CPerfPainterWnd*> m_MergingWnd;
	CList<CPerfWnd*> m_lstChild;
	bool m_DestroyMainFrame;
	map<int, CString> m_mapDogString;

	CEventListener* m_pEventListener;

	void ShowPerfWnd(vector<struct UI::DogEvent>& ShowWnd);
    int OnCreate(LPCREATESTRUCT lpCreateStruct);
    void OnDestroy();
	void OnTimer(UINT_PTR nIDEvent);
	void OnExit();
	void OnViewsSave();
	void OnViewsLoad();
	void RemoveWindowFromList(CPerfPainterWnd* pWnd);
	BOOL IsWinDVDClosed();
	BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* HandlerInfo);
	BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	afx_msg void OnWindowsSelect();
	afx_msg void OnWindowsReset();
	DECLARE_MESSAGE_MAP()

private:
	enum {
		TimerID = 1,
		RefreshTimer = 500
	};

	void CloseAllPerfWnd();
	void LoadSetting(LPCTSTR FilePath);

	map<CString, int> m_mapDogId;
	//map<int, vector<struct CUserSetting::DogEvent>> m_mapm_vecIniSetting;
	CMenu m_Menu;

	struct ViewSetting
	{
		CPerfPainterWnd* pView;
		DWORD id;
		std::basic_string<TCHAR> name;
	};
	typedef std::vector<ViewSetting> TViewSettingList;
	TViewSettingList m_ViewList;
};

#endif // MainFrame_h__
