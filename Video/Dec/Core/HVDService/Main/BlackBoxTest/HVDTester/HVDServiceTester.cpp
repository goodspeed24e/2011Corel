// HVDServiceTester.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "HVDServiceTester.h"
#include "MainFrm.h"

#include "ChildFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CHVDServiceTesterApp

BEGIN_MESSAGE_MAP(CHVDServiceTesterApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, &CHVDServiceTesterApp::OnAppAbout)
	ON_COMMAND(ID_FILE_NEW, &CHVDServiceTesterApp::OnFileNew)
	ON_COMMAND(ID_FILE_LOAD, &CHVDServiceTesterApp::OnFileLoad)
END_MESSAGE_MAP()


// CHVDServiceTesterApp construction

CHVDServiceTesterApp::CHVDServiceTesterApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CHVDServiceTesterApp object

CHVDServiceTesterApp theApp;


// CHVDServiceTesterApp initialization

BOOL CHVDServiceTesterApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}
	AfxEnableControlContainer();
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));
	// To create the main window, this code creates a new frame window
	// object and then sets it as the application's main window object
	CMDIFrameWnd* pFrame = new CMainFrame;
	if (!pFrame)
		return FALSE;
	m_pMainWnd = pFrame;
	// create main MDI frame window
	if (!pFrame->LoadFrame(IDR_MAINFRAME))
		return FALSE;
	// try to load shared MDI menus and accelerator table
	//TODO: add additional member variables and load calls for
	//	additional menu types your application may need
	HINSTANCE hInst = AfxGetResourceHandle();
	m_hMDIMenu  = ::LoadMenu(hInst, MAKEINTRESOURCE(IDR_HVDServiceTesteTYPE));
	m_hMDIAccel = ::LoadAccelerators(hInst, MAKEINTRESOURCE(IDR_HVDServiceTesteTYPE));

	// The main window has been initialized, so show and update it
	pFrame->ShowWindow(m_nCmdShow);
	pFrame->UpdateWindow();
	CMainFrame* pMainFrame = STATIC_DOWNCAST(CMainFrame, m_pMainWnd);
	pMainFrame->SetupHVDTesterItem();
	return TRUE;
}


// CHVDServiceTesterApp message handlers

int CHVDServiceTesterApp::ExitInstance() 
{
	//TODO: handle additional resources you may have added
	if (m_hMDIMenu != NULL)
		FreeResource(m_hMDIMenu);
	if (m_hMDIAccel != NULL)
		FreeResource(m_hMDIAccel);

	return CWinApp::ExitInstance();
}

void CHVDServiceTesterApp::OnFileLoad()
{
	// TODO: Add your command handler code here
	TCHAR szFilters[]= _T("HVDTest Config Files(*.ini)|*.ini||");
	CFileDialog OpenFile(TRUE, NULL, NULL, OFN_FILEMUSTEXIST|OFN_HIDEREADONLY, szFilters);
	if (OpenFile.DoModal() == IDOK)
	{
		CMainFrame* pFrame = STATIC_DOWNCAST(CMainFrame, m_pMainWnd);
		CString filename = OpenFile.GetPathName();
		TCHAR tcConfigFile[256];
		_tcscpy_s( tcConfigFile, 256, filename);
		pFrame->OpenHVDTestConfigFile(tcConfigFile);
	}
}

void CHVDServiceTesterApp::OnFileNew() 
{
	CMainFrame* pFrame = STATIC_DOWNCAST(CMainFrame, m_pMainWnd);
	// create a new MDI child window
	pFrame->CreateNewChild(
		RUNTIME_CLASS(CChildFrame), IDR_HVDServiceTesteTYPE, m_hMDIMenu, m_hMDIAccel);
}


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

// App command to run the dialog
void CHVDServiceTesterApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}