#include "stdafx.h"
#include "Perf2.h"
#include "MainFrame.h"
#include "CPerfWnd.h"
#include "CPerfPainterWnd.h"
#include "MyFunction.h"
#include "ControlDialog.h"
#include <atlbase.h>	//CRegKey
#include "structData.h"

//const int timeBuffer = 100000000;	//10s
const int timeBuffer = 1000000000;	//100s

MainFrame::MainFrame(CEventListener* pEventListener) : m_pEventListener(pEventListener)
{
 	m_DestroyMainFrame = false;	
}

int MainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	SetTimer(TimerID, RefreshTimer, NULL);

	int dynMenuId = ID_DYNMENU_BASE;
	int DefaultMenuId = 0;
	CString curSectionName = _T("");
	m_Menu.LoadMenu(IDR_MAINMENU);
	SetMenu(&m_Menu);
	CMenu* subMenu = m_Menu.GetSubMenu(1);

	subMenu->DeleteMenu(ID_VIEWS_LOAD, MF_BYCOMMAND);
 	subMenu->DeleteMenu(ID_VIEWS_SAVE, MF_BYCOMMAND);

	//Show default window
	ShowPerfWnd(((MyApp*)AfxGetApp())->GetSettingVector());
	return 0;
}

void MainFrame::OnDestroy()
{	
	m_DestroyMainFrame = true;
	CloseAllPerfWnd();
	CFrameWnd::OnDestroy();
}

void MainFrame::RemoveWindowFromList(CPerfPainterWnd* pWnd)
{
	POSITION p = m_lstChild.Find(pWnd);

	if (p)
	{
		((MyApp*)AfxGetApp())->RemoveFromSettingVector(pWnd->GetTitle());
		m_lstChild.RemoveAt(p);
	}

	std::vector<TViewSettingList::iterator> removeList;
	TViewSettingList::iterator iter = m_ViewList.begin();	
	for(; iter!=m_ViewList.end(); ++iter)
	{
		if(pWnd == iter->pView)
		{
			iter->pView = NULL;
			removeList.push_back(iter);
		}
	}

	for(size_t i=0; i<removeList.size(); ++i)
	{
		m_ViewList.erase(removeList[i]);
	}
}

void MainFrame::OnTimer(UINT_PTR nIDEvent)
{
// 	if (IsProcessRunning(_T("WinDVD.exe")) == FALSE)
// 	{
// 		POSITION p = m_lstChild.GetHeadPosition();
// 		while (p) 
// 		{
// 			CPerfPainterWnd* pFrame = (CPerfPainterWnd*)m_lstChild.GetNext(p);
// 			pFrame->SetIsWinDVDClosed(true);
// 		}
// 	}

	TViewSettingList::const_iterator iter = m_ViewList.begin();
	for(; iter!=m_ViewList.end(); ++iter)
	{
		const ViewSetting& setting = *iter;
		if(setting.pView) {
			setting.pView->AddDataSource(setting.id, setting.name, m_pEventListener->GetDataSource(setting.id));
		}
	}
}

void MainFrame::CloseAllPerfWnd()
{
	POSITION p = m_lstChild.GetHeadPosition();
	while (p) {
		CPerfWnd* pFrame = m_lstChild.GetNext(p);
		pFrame->DestroyWindow();
	}
	m_lstChild.RemoveAll();
}

void MainFrame::ShowPerfWnd(vector<struct UI::DogEvent>& DogEventId)
{
	m_pEventListener->ClearInterestedTable();
	m_ViewList.clear();

	CloseAllPerfWnd();
	((MyApp*)AfxGetApp())->m_mapChartData.clear();


	CPerfPainterWnd* pFrame = NULL;
	DWORD perfWndStyle = WS_CLIPSIBLINGS|WS_CHILD|WS_TILEDWINDOW|CS_DBLCLKS;

	int wndCount = 0;
	int xPos = 0;
	int yPos = 0;
	int xSize = CPerfWnd::m_WindowMinSize.x;
	int ySize = CPerfWnd::m_WindowMinSize.y;
	CString chartType = _T("");
	int dogId = 0;

	for(vector<struct UI::DogEvent>::iterator it = DogEventId.begin(); it != DogEventId.end(); it++)
	{
		dogId = (*it).dogId;

		m_pEventListener->AddInterestedEventId(dogId);

		pFrame = new CPerfPainterWnd();
		pFrame->Create(NULL, it->pEventName, perfWndStyle, CRect(xPos, yPos, xPos+xSize, yPos+ySize), this, 0);
		pFrame->ShowWindow(SW_SHOW);
		m_lstChild.AddTail(pFrame);

		ViewSetting setting = {pFrame, dogId, it->pEventName};
		m_ViewList.push_back(setting);

// 		CChartData* pChartData = new CChartData();
// 		pChartData->SetParentWnd(pFrame);
// 		pChartData->SetChartType(CChartData::ChartType::eCurveType);
// 		((MyApp*)AfxGetApp())->m_mapChartData[dogId] = pChartData;		
// 		pFrame->m_ActiveDataList.AddTail(pChartData);


// 	 	//Title + Font
// 	 	pFrame->m_ChartCtrl.GetTitle()->AddString((TChartString)(it->eventName));
// 	 	CChartFont titleFont;
// 	 	titleFont.SetFont(_T("Arial Black"),80,false,false,false);
// 	 	pFrame->m_ChartCtrl.GetTitle()->SetFont(titleFont);
// 	 	pFrame->m_ChartCtrl.GetTitle()->SetColor(RGB(0,0,128));

		if (wndCount%2 == 0)
			yPos = yPos + ySize;

		else if (wndCount%2 == 1)
		{
			xPos = xPos + xSize;
			yPos = 0;
		}	//if (wndCount%2 == 0)

		++wndCount;
	}	//for(vector<int>::iterator it = ShowWnd.begin(); it != ShowWnd.end(); it++)
}

void MainFrame::LoadSetting(LPCTSTR FilePath)
{
	/*
	TCHAR sectionNameBuffer[1024];
	memset(sectionNameBuffer, 0, sizeof(sectionNameBuffer));
	TCHAR keyBuffer[5144];

	DWORD readNameNum = GetPrivateProfileSectionNames (sectionNameBuffer, sizeof(sectionNameBuffer), FilePath);
	vector<int> DogEventId;
	CString sectionName = _T("");
	CString key = _T("");
	CMenu* subMenu = m_Menu.GetSubMenu(1);;
	int dynMenuId = ID_DYNMENU_BASE;

	for (int i = 0; i < readNameNum; i++)
	{
		sectionName += sectionNameBuffer[i];

		if (sectionNameBuffer[i] == NULL)
		{
			if (sectionName != _T("Audio") && sectionName != _T("Demux") && sectionName != _T("Display"))
			{				
				subMenu->InsertMenu(0, 0, ++dynMenuId, sectionName);

				DWORD readKeyNum = GetPrivateProfileSection(sectionName, keyBuffer, sizeof(keyBuffer), FilePath);
				for (int j = 0; j < readKeyNum; j++)
				{
					key += keyBuffer[j];

					if (keyBuffer[j] == NULL)
					{
						DogEventId.push_back(m_mapDogId[key]);
						
						key = _T("");
						int cc  = m_mapm_vecIniSetting[dynMenuId].size();
						int a =2;
					}				
				}	//end of handle a group of section keys
				//m_mapm_vecIniSetting[dynMenuId] = DogEventId;
				
				//m_mapTitle
				DogEventId.clear();

			}	//if (sectionName != _T("Audio") && sectionName != _T("Demux") && sectionName != _T("Display"))
			sectionName = _T("");
		}	//for (int i = 0; i < readNameNum; i++)
	}	//for (int i = 0; i < readNameNum; i++)
	//subMenu->DeleteMenu(1, MF_BYPOSITION);
	subMenu->DeleteMenu(ID_VIEWS_LOAD, MF_BYCOMMAND);
	subMenu->DeleteMenu(ID_VIEWS_SAVE, MF_BYCOMMAND);
	//subMenu->DeleteMenu(subMenu->GetMenuItemCount(), MF_BYPOSITION);
	*/
}	//void MainFrame::LoadSetting(LPCTSTR FilePath)


void MainFrame::OnViewsSave()
{
	TCHAR exeFullName[BUFSIZE];
	GetModuleFileName(NULL, exeFullName, BUFSIZE);
	PathRemoveFileSpec(exeFullName);

	CFile cfile;
	CFileDialog saveDialog(FALSE, _T("ini"), NULL, OFN_OVERWRITEPROMPT, _T("INI Files (*.ini)|*.ini|All Files (*.*)|*.*||"), this);
	saveDialog.m_pOFN->lpstrInitialDir = exeFullName;	

	if (saveDialog.DoModal() == IDOK)
	{
		if (cfile.Open(saveDialog.GetPathName(), CFile::modeCreate | CFile::modeWrite) == TRUE)
		{
		}
	}
}

void MainFrame::OnViewsLoad()
{
	CFileDialog cFileDialog(TRUE, _T("ini"), NULL, OFN_OVERWRITEPROMPT, _T("*.ini||"), this);
	if (cFileDialog.DoModal() == IDOK)
	{
		CString pathName = cFileDialog.GetPathName();

		// Implement opening and reading file in here.

		//Change the window's title to the opened file's title.
		CString fileName = cFileDialog.GetFileTitle();
		SetWindowText(fileName);

		//AfxMessageBox(cFileDialog.GetFileName() , MB_OK | MB_ICONQUESTION);
	}
}

void MainFrame::OnExit()
{
	DestroyWindow();
}

BOOL MainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* HandlerInfo)
{
	CFrameWnd::OnCmdMsg(nID, nCode, pExtra, HandlerInfo);
	return TRUE;
}

BOOL MainFrame::OnCommand(WPARAM wParam, LPARAM lParam)
{
	int MenuCmdId=LOWORD(wParam);
	//if (MenuCmdId == ID_VIEWS_MERGE || MenuCmdId == ID_EXIT || MenuCmdId == ID_WINDOWS_SELECT)
	CFrameWnd::OnCommand(wParam, lParam);
	//else
	//	ShowPerfWnd(m_mapm_vecIniSetting[MenuCmdId]);
	
	return TRUE;
}

void MainFrame::OnWindowsSelect()
{
	// TODO: Add your command handler code here
	CControlDialog dlgControl;

	if (dlgControl.DoModal() == IDOK)
		ShowPerfWnd(((MyApp*)AfxGetApp())->GetSettingVector());
}

void MainFrame::OnWindowsReset()
{
	// TODO: Add your command handler code here
	int wndCount = 0;
	int xPos = 0;
	int yPos = 0;
	int xSize = CPerfWnd::m_WindowMinSize.x;
	int ySize = CPerfWnd::m_WindowMinSize.y;

	POSITION p = m_lstChild.GetHeadPosition();

	while (p) 
	{
		CPerfPainterWnd* pFrame = (CPerfPainterWnd*)m_lstChild.GetNext(p);
		pFrame->MoveWindow(xPos, yPos, xSize, ySize);

		if (wndCount%2 == 0)
			yPos = yPos + ySize;

		else if (wndCount%2 == 1)
		{
			xPos = xPos + xSize;
			yPos = 0;
		}	//if (wndCount%2 == 0)

		++wndCount;
	}
}

BEGIN_MESSAGE_MAP(MainFrame, CFrameWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_COMMAND(ID_VIEWS_SAVE, &MainFrame::OnViewsSave)
	ON_COMMAND(ID_VIEWS_LOAD, &MainFrame::OnViewsLoad)
	ON_COMMAND(ID_WINDOWS_SELECT, &MainFrame::OnWindowsSelect)
	ON_COMMAND(ID_WINDOWS_RESET, &MainFrame::OnWindowsReset)
	ON_COMMAND(ID_EXIT, &MainFrame::OnExit)
END_MESSAGE_MAP()


