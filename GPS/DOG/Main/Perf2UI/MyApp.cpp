// MyApp.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "MyApp.h"
#include "DogProxy.h"

#include "MainFrm.h"
#include "EventIdNameMap.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//-----------------------------------------------------------------------------
inline BOOL GetValidSys(char* DomainName);

// CMyApp
BEGIN_MESSAGE_MAP(CMyApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, &CMyApp::OnAppAbout)
END_MESSAGE_MAP()

// CMyApp construction
CMyApp::CMyApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CMyApp object

CMyApp theApp;


// CMyApp initialization

BOOL CMyApp::InitInstance()
{
	BOOL bValidSys = GetValidSys("tw.corelcorp.corel.ics") || GetValidSys("cn.corelcorp.corel.ics");
	if(!bValidSys)
	{
		MessageBox(NULL, _T("Not a valid system!\nPerf2 are going to close."), _T("Perf2 Error"), 0);
		return FALSE;
	}

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
	SetRegistryKey(_T("Perf2"));

	// To create the main window, this code creates a new frame window
	// object and then sets it as the application's main window object
	CMDIFrameWnd* pFrame = new CMainFrame;
	if (!pFrame)
		return FALSE;

	m_pMainWnd = pFrame;

	// create main MDI frame window
	if (!pFrame->LoadFrame(IDR_MAINFRAME))
		return FALSE;

	// The main window has been initialized, so show and update it
	pFrame->ShowWindow(m_nCmdShow);
	pFrame->UpdateWindow();

	dogproxy::RegisterListener(&m_EventListener);
	dogproxy::SetupDogConfigByString("log = all");

	g_EventIdNameMap.Init();

	return TRUE;
}


// CMyApp message handlers

int CMyApp::ExitInstance()
{
	dogproxy::UnregisterListener(&m_EventListener);

	return CWinApp::ExitInstance();
}

//-----------------------------------------------------------------------------
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
void CMyApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

//-----------------------------------------------------------------------------
// Check if system domain name is same with the specified DomainName
inline BOOL GetValidSys(char* DomainName)
{
	typedef enum _COMPUTER_NAME_FORMAT {
		ComputerNameNetBIOS,
		ComputerNameDnsHostname,
		ComputerNameDnsDomain,
		ComputerNameDnsFullyQualified,
		ComputerNamePhysicalNetBIOS,
		ComputerNamePhysicalDnsHostname,
		ComputerNamePhysicalDnsDomain,
		ComputerNamePhysicalDnsFullyQualified,
		ComputerNameMax
	} COMPUTER_NAME_FORMAT;

	HINSTANCE	hDll;
	typedef BOOL (WINAPI *PFGetComputerName)(COMPUTER_NAME_FORMAT NameType,
		LPSTR lpBuffer,
		LPDWORD nSize);

	hDll = LoadLibrary(_T("Kernel32.dll"));

	char  buffer[256] = "";
	char  szDescription[8][32] = {	"NetBIOS",
		"DNS hostname",
		"DNS domain",
		"DNS fully-qualified",
		"Physical NetBIOS",
		"Physical DNS hostname",
		"Physical DNS domain",
		"Physical DNS fully-qualified"};
	BOOL bRet = FALSE;
	DWORD dwSize = sizeof(buffer);
	if (hDll)
	{
		PFGetComputerName pGetComputerName = (PFGetComputerName) GetProcAddress(hDll,"GetComputerNameExA");
		if (pGetComputerName)
		{
			int nRet = pGetComputerName(ComputerNameDnsDomain, buffer, &dwSize);
			if (strstr(buffer, DomainName) != NULL)
				bRet = TRUE;
		}

		FreeLibrary(hDll);
	}
	return bRet;
}

// CMyApp message handlers
