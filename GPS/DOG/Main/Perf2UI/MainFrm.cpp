// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "MyApp.h"
#include "EventIdNameMap.h"

#include "MainFrm.h"
#include "ChildFrm.h"
#include "ChartFrm.h"
#include "MessageFrm.h"
#include "WinUnicodeUtil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_COMMAND(ID_FILE_SAVESETTINGAS, OnSaveSetting)
	ON_COMMAND(ID_FILE_LOADSETTING, OnLoadSetting)
	ON_COMMAND(ID_CHART_NEW, OnChartNew)
	ON_COMMAND(ID_MESSAGE_VIEW_NEW, OnMessageViewNew)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};


// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
}

CMainFrame::~CMainFrame()
{
}


int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	// TODO: Delete these three lines if you don't want the toolbar to be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);


	// try to load shared MDI menus and accelerator table
	HINSTANCE hInst = AfxGetResourceHandle();
	m_hMDIAccel = ::LoadAccelerators(hInst, MAKEINTRESOURCE(IDR_MAINFRAME));
	m_hMDIMenu  = ::LoadMenu(hInst, MAKEINTRESOURCE(IDR_Perf2TYPE));
	m_hChartViewMenu = ::LoadMenu(hInst, MAKEINTRESOURCE(IDR_ChartView));
	m_hMessageViewMenu = ::LoadMenu(hInst, MAKEINTRESOURCE(IDR_MessageView));

	return 0;
}

void CMainFrame::OnDestroy()
{
	if (m_hMDIMenu != NULL)
		FreeResource(m_hMDIMenu);
	if (m_hMDIAccel != NULL)
		FreeResource(m_hMDIAccel);
	if (m_hChartViewMenu != NULL)
		FreeResource(m_hChartViewMenu);
	if (m_hMessageViewMenu != NULL)
		FreeResource(m_hMessageViewMenu);	
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CMDIFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}

void CMainFrame::CloseAllWindows()
{
	std::list<CChildFrame*> childFrmList = m_ChildFrameList;

	std::list<CChildFrame*>::iterator iter = childFrmList.begin();
	for(; iter!=childFrmList.end(); ++iter)
	{
		(*iter)->DestroyWindow();
	}
}

template<typename T>
CChildFrame* CMainFrame::CreateChild(CString strTitle, UINT nResource, HMENU hMenu, HACCEL hAccel)
{
	CMDIChildWnd* mdiChild = CreateNewChild( RUNTIME_CLASS(T), nResource, hMenu, hAccel);
	mdiChild->SetOwner(this);
	mdiChild->SetTitle(strTitle);
	mdiChild->SetWindowText(mdiChild->GetTitle());

	T* childFrm = dynamic_cast<T*>(mdiChild);
	if (NULL != childFrm)
	{
		m_ChildFrameList.push_back(childFrm);
		CEventView* pView = childFrm->GetEventView();
		if(pView)
		{
			pView->SetDataSource(theApp.GetEventListener());
		}
	}

	return childFrm;
}

CChildFrame* CMainFrame::CreateChartViewFrame()
{
	return CreateChild<CChartFrame>(_T("ChartView"), IDR_Perf2TYPE, m_hChartViewMenu, m_hMDIAccel);
}

CChildFrame* CMainFrame::CreateMessageViewFrame()
{
	return CreateChild<CMessageFrame>( _T("MessageView"), IDR_Perf2TYPE, m_hMessageViewMenu, m_hMDIAccel);
}

// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWnd::Dump(dc);
}

#endif //_DEBUG

//-----------------------------------------------------------------------------
void CMainFrame::OnSaveSetting()
{
	TCHAR BASED_CODE szFilter[] = _T("Perf2 setting (*.xml)|*.xml|*.*||");

	CFileDialog dlgFile(FALSE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter, NULL, 0);
	INT_PTR nResult = dlgFile.DoModal();
	if(IDOK == nResult)
	{
		SaveConfig( (LPCTSTR)dlgFile.GetPathName());
	}
}

void CMainFrame::OnLoadSetting()
{
	TCHAR BASED_CODE szFilter[] = _T("Perf2 setting (*.xml)|*.xml|*.*||");

	CFileDialog dlgFile(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter, NULL, 0);
	INT_PTR nResult = dlgFile.DoModal();
	if(IDOK == nResult)
	{
		LoadConfig((LPCTSTR)dlgFile.GetPathName());
	}
}

void CMainFrame::OnChartNew()
{
	CreateChartViewFrame();
}

void CMainFrame::OnMessageViewNew()
{
	CreateMessageViewFrame();
}

//-----------------------------------------------------------------------------

void CMainFrame::SaveConfig(const TCHAR* szConfigFileName)
{
	m_ConfigXML.Clear();

	TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "", "" );
	m_ConfigXML.LinkEndChild(decl);
	TiXmlElement* root = new TiXmlElement("Application");
	m_ConfigXML.LinkEndChild(root);

	std::list<CChildFrame*>::iterator iter = m_ChildFrameList.begin();
	for(; iter!=m_ChildFrameList.end(); ++iter)
	{
		TiXmlElement* elemWnd = new TiXmlElement("WndFrame");

		CChildFrame* pWnd = (*iter);
		elemWnd->SetAttribute("type",  pWnd->GetWndType());
		elemWnd->SetAttribute("title", ToAnsiString(pWnd->GetTitle().GetBuffer()));

		RECT rect;
		pWnd->GetWindowRect(&rect);
		ScreenToClient(&rect);

		elemWnd->SetAttribute("left",   rect.left);
		elemWnd->SetAttribute("top",    rect.top);
		elemWnd->SetAttribute("right",  rect.right);
		elemWnd->SetAttribute("bottom", rect.bottom);

		CEventView* pView = pWnd->GetEventView();
		if(pView)
		{
			std::vector<DWORD> eventList = pView->GetEventList();
			for(size_t i=0; i<eventList.size(); ++i)
			{
				TiXmlElement* elemEvent = new TiXmlElement("Event");
				elemEvent->SetAttribute("id", eventList[i]);
				elemWnd->LinkEndChild(elemEvent);
			}
		}

		root->LinkEndChild(elemWnd);
	}


	m_ConfigXML.SaveFile(ToAnsiString(szConfigFileName).c_str());
}

void CMainFrame::LoadConfig(const TCHAR* szConfigFileName)
{
	CloseAllWindows();

	m_ConfigXML.LoadFile(ToAnsiString(szConfigFileName).c_str());
	TiXmlElement* root = m_ConfigXML.RootElement();

	TiXmlElement* elemWnd = root->FirstChildElement("WndFrame");
	while(elemWnd)
	{
		// retrieve attributes/setting
		int wndType = 0;
		std::string strAnsiTitle;
		RECT rect = { 0,0,0,0 };
		elemWnd->QueryIntAttribute("type", &wndType);
		strAnsiTitle = elemWnd->Attribute("title");
		elemWnd->QueryIntAttribute("left",   (int*)&rect.left);
		elemWnd->QueryIntAttribute("top",    (int*)&rect.top);
		elemWnd->QueryIntAttribute("right",  (int*)&rect.right);
		elemWnd->QueryIntAttribute("bottom", (int*)&rect.bottom);

		std::vector<DWORD> eventList;

		TiXmlElement* elemEvent = elemWnd->FirstChildElement("Event");
		while(elemEvent)
		{
			int id = 0;
			elemEvent->QueryIntAttribute("id", &id);
			eventList.push_back(id);

			elemEvent = elemEvent->NextSiblingElement("Event");
		}


		// create child frame
		CChildFrame* childFrm = NULL;
		switch(wndType)
		{
		default:
			break;
		case CChildFrame::CHART_WND:   childFrm = CreateChartViewFrame();   break;
		case CChildFrame::MESSAGE_WND: childFrm = CreateMessageViewFrame(); break;
		}

		// restore layout/setting
		if(childFrm)
		{
			if(!strAnsiTitle.empty())
			{
				CString strTitle = ToTcharString(strAnsiTitle.c_str()).c_str();
				childFrm->SetTitle(strTitle);
				childFrm->SetWindowText(childFrm->GetTitle());
			}
			childFrm->MoveWindow(&rect);

			//restore listening events
			CEventView* pView = childFrm->GetEventView();
			if(pView)
			{
				for(size_t i=0; i<eventList.size(); ++i)
				{
					DWORD addedId = eventList[i];
					pView->AddEvent(addedId, g_EventIdNameMap.GetFriendlyEventName(addedId));
					theApp.AddInterestedEventId(addedId);
				}
			}
		}

		elemWnd = elemWnd->NextSiblingElement("WndFrame");
	}
}



// CMainFrame message handlers

