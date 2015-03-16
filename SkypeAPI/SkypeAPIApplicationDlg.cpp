// SkypeAPIApplicationDlg.cpp : implementation file

#include "stdafx.h"
#include "SkypeAPIApplication.h"
#include "SkypeAPIApplicationDlg.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About
class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
void ShowMessages ( LPCTSTR lpszText, ... )
{
	CSkypeAPIApplicationDlg *pDlg = reinterpret_cast<CSkypeAPIApplicationDlg*>(::AfxGetMainWnd());    //将主程序窗体句柄强制转换成对话框句柄，待解释
	if ( pDlg )   //如果对话框句柄不为空
	{
		char buf[1024] = {0};
		
		va_list  va;
		va_start ( va, lpszText );
		_vsnprintf  ( buf, sizeof(buf) - 1, (char*)lpszText, va);
		va_end(va);
		pDlg->AddMessages ( buf );
	}
}
/////////////////////////////////////////////////////////////////////////////
// CSkypeAPIApplicationDlg dialog

CSkypeAPIApplicationDlg::CSkypeAPIApplicationDlg(CWnd* pParent /*=NULL*/): CDialog(CSkypeAPIApplicationDlg::IDD, pParent), m_SkypeAPI ( ShowMessages )
{
	//{{AFX_DATA_INIT(CSkypeAPIApplicationDlg)
	m_DailNum = _T("");
	m_ComToSkype = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSkypeAPIApplicationDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSkypeAPIApplicationDlg)
	DDX_Control(pDX, IDC_EDITMSGTOSP, m_ComSendToSKype);
	DDX_Control(pDX, IDC_COM_LIST, m_ListCommands);
	DDX_Text(pDX, IDC_EDIT_DAILNUM, m_DailNum);
	DDX_Text(pDX, IDC_EDITMSGTOSP, m_ComToSkype);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSkypeAPIApplicationDlg, CDialog)
	//{{AFX_MSG_MAP(CSkypeAPIApplicationDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, OnButton1)
	ON_BN_CLICKED(IDC_BUTTON2, OnButton2)
	ON_BN_CLICKED(IDC_BUTTON3, OnButton3)
	ON_BN_CLICKED(IDC_BUTTON4, OnButton4)
	ON_BN_CLICKED(IDC_BUTTON5, OnButton5)
	ON_BN_CLICKED(IDC_BUTTON6, OnButton6)
	ON_BN_CLICKED(IDC_BUTTON7, OnButton7)
	ON_BN_CLICKED(IDC_BUTTON8, OnButton8)
	ON_BN_CLICKED(IDC_BUTTON0, OnButton0)
	ON_BN_CLICKED(IDC_BUTTON_UP, OnButtonUp)
	ON_BN_CLICKED(IDC_BUTTON_DOWN, OnButtonDown)
	ON_BN_CLICKED(IDC_BUTTON_XING, OnButtonXing)
	ON_BN_CLICKED(IDC_BUTTON_JING, OnButtonJing)
	ON_BN_CLICKED(IDC_BUTTON_SMSG, OnButtonSendMSG)
	ON_BN_CLICKED(IDC_BUTTON_SKYPESEND, OnButtonSendMSGBySkype)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON9, OnButton9)
	ON_BN_CLICKED(IDC_BUTTON_DIAL, OnButtonDial)
	ON_BN_CLICKED(IDC_BUTTON_CALL, OnButtonCall)
	ON_BN_CLICKED(IDC_BUTTON_SENDCOMTOSKYPE, OnButtonSendComToSkype)
	ON_BN_CLICKED(IDC_BUTTON_HOOKON, OnButtonHookon)
	ON_LBN_DBLCLK(IDC_COM_LIST, SetListComToEdit)
	ON_BN_CLICKED(IDC_BUTTON_CE, OnButtonHON)
	ON_MESSAGE(WM_SKYPEMSG_USERS,GetUsers)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

// CSkypeAPIApplicationDlg message handlers

BOOL CSkypeAPIApplicationDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	ShowMessages("%s\r\n",GREETINGS );
	if (m_SkypeAPI.InitialConnection (GetSafeHwnd())) SetTimer(1,100,NULL );
	
	EnableItemsOnApplication ( FALSE );

	m_ListCommands.AddString("001>>ALTER APPLICATION <appname> CONNECT <username> ");
	m_ListCommands.AddString("002>>ALTER APPLICATION <appname> DATAGRAM <username>:<id> <text>");
	m_ListCommands.AddString("003>>ALTER APPLICATION <appname> DISCONNECT <username>:<id>");
	m_ListCommands.AddString("004>>ALTER APPLICATION <appname> READ <username>:<id> ");
	m_ListCommands.AddString("005>>ALTER APPLICATION <appname> WRITE <username>:<id> <text>");
	m_ListCommands.AddString("006>>ALTER CALL <id> <status>");
	m_ListCommands.AddString("007>>ALTER CHAT <chat_id> ADDMEMBERS <target>");
	m_ListCommands.AddString("008>>ALTER CHAT <chat_id> LEAVE ");
	m_ListCommands.AddString("009>>ALTER CHAT <chat_id> SETTOPIC <topic>");
	m_ListCommands.AddString("010>>ALTER GROUP <id> ADDUSER <userhandle|PSTN>");
	m_ListCommands.AddString("011>>ALTER GROUP <id> REMOVEUSER <userhandle|PSTN>");
	m_ListCommands.AddString("012>>ALTER VOICEMAIL <id> <action>");
	m_ListCommands.AddString("013>>APPLICATION <appname> <property> <value>");
	m_ListCommands.AddString("014>>BTN_PRESSED <btn>");
	m_ListCommands.AddString("015>>BTN_RELEASED <btn>");
	m_ListCommands.AddString("016>>CALL <id>");
	m_ListCommands.AddString("017>>CHAT CREATE <target>");
	m_ListCommands.AddString("018>>CHATMESSAGE <chat_id> <message> ");
	m_ListCommands.AddString("019>>CLEAR CALLHISTORY ");
	m_ListCommands.AddString("020>>CLEAR CHATHISTORY ");
	m_ListCommands.AddString("021>>CLEAR VOICEMAILHISTORY");
	m_ListCommands.AddString("022>>CONTACTS FOCUSED");
	m_ListCommands.AddString("023>>CONTACTS FOCUSED <username>");
	m_ListCommands.AddString("024>>CREATE APPLICATION <appname>");
	m_ListCommands.AddString("025>>CREATE GROUP <GroupIId>");
	m_ListCommands.AddString("026>>DELETE APPLICATION <appname> ");
	m_ListCommands.AddString("027>>DELETE GROUP <Id>");
	m_ListCommands.AddString("028>>GET AGC");
	m_ListCommands.AddString("029>>GET AUDIO_IN");
	m_ListCommands.AddString("030>>GET AUDIO_OUT");
	m_ListCommands.AddString("031>>GET CALL <id> <property>");
	m_ListCommands.AddString("032>>GET CHAT <chat_id> <property>");
	m_ListCommands.AddString("033>>GET CHAT <chat_id> CHATMESSAGES");
	m_ListCommands.AddString("034>>GET CHAT <chat_id> RECENTCHATMESAGES");
	m_ListCommands.AddString("035>>GET CONNSTATUS");
	m_ListCommands.AddString("036>>GET CURRENTUSERHANDLE");
	m_ListCommands.AddString("037>>GET GROUP <id> property");
	m_ListCommands.AddString("038>>GET MESSAGE <id> <property>");
	m_ListCommands.AddString("039>>GET MUTE");
	m_ListCommands.AddString("040>>GET PRIVILEGE <user_privilege>");
	m_ListCommands.AddString("041>>GET PROFILE <profile_property>");
	m_ListCommands.AddString("042>>GET PROFILE <profile>");
	m_ListCommands.AddString("043>>GET RINGER");
	m_ListCommands.AddString("044>>GET SKYPEVERSION");
	m_ListCommands.AddString("045>>GET USER <username> <property>");
	m_ListCommands.AddString("046>>GET USERSTATUS");
	m_ListCommands.AddString("047>>GET VIDEO_IN");
	m_ListCommands.AddString("048>>HOOK <ON|OFF>");
	m_ListCommands.AddString("049>>MESSAGE <id> <property> <value>");
	m_ListCommands.AddString("050>>MINIMIZE");
	m_ListCommands.AddString("051>>MUTE <ON|OFF>");
	m_ListCommands.AddString("052>>OPEN ADDAFRIEND <username>");
	m_ListCommands.AddString("053>>OPEN AUTHORIZATION <username>");
	m_ListCommands.AddString("054>>OPEN BLOCKEDUSERS");
	m_ListCommands.AddString("055>>OPEN CALLHISTORY");
	m_ListCommands.AddString("056>>OPEN CHAT <chat_id>");
	m_ListCommands.AddString("057>>OPEN CONTACTS");
	m_ListCommands.AddString("058>>OPEN DIALPAD");
	m_ListCommands.AddString("059>>OPEN FILETRANSFER <username> IN <folder>");
	m_ListCommands.AddString("060>>OPEN GETTINGSTARTED");
	m_ListCommands.AddString("061>>OPEN IM <username> <message> ");
	m_ListCommands.AddString("062>>OPEN IMPORTCONTACTS");
	m_ListCommands.AddString("063>>OPEN OPTIONS <page>");
	m_ListCommands.AddString("064>>OPEN PROFILE ");
	m_ListCommands.AddString("065>>OPEN SEARCH");
	m_ListCommands.AddString("066>>OPEN SENDCONTACTS");
	m_ListCommands.AddString("067>>OPEN USRERINFO <username> ");
	m_ListCommands.AddString("068>>OPEN VIDEOTEST");
	m_ListCommands.AddString("069>>OPEN VOICEMAIL <id> ");
	m_ListCommands.AddString("070>>PING");
	m_ListCommands.AddString("071>>PROTOCOL <version>");
	m_ListCommands.AddString("072>>PROTOCOL 9999");
	m_ListCommands.AddString("073>>SEARCH ACTIVECALLS");
	m_ListCommands.AddString("074>>SEARCH ACTIVECHATS ");
	m_ListCommands.AddString("075>>SEARCH BOOKMARKEDCHATS ");
	m_ListCommands.AddString("076>>SEARCH CALLS <target> ");
	m_ListCommands.AddString("077>>SEARCH CHATMESSAGES <username>");
	m_ListCommands.AddString("078>>SEARCH CHATS");
	m_ListCommands.AddString("079>>SEARCH FRIENDS");
	m_ListCommands.AddString("080>>SEARCH GROUPS");
	m_ListCommands.AddString("081>>SEARCH MESSAGES <target>");
	m_ListCommands.AddString("082>>SEARCH MISSEDCALLS");
	m_ListCommands.AddString("083>>SEARCH MISSEDCHATMESSAGES");
	m_ListCommands.AddString("084>>SEARCH MISSEDCHATS");
	m_ListCommands.AddString("085>>SEARCH MISSEDMESSAGES");
	m_ListCommands.AddString("086>>SEARCH RECENTCHATS");
	m_ListCommands.AddString("087>>SEARCH USERS <target>");
	m_ListCommands.AddString("088>>SEARCH USERSWAITINGMYAUTHORIZATION");
	m_ListCommands.AddString("089>>SEARCH VOICEMAILS");
	m_ListCommands.AddString("090>>SET AUDIO_IN <device_name>");
	m_ListCommands.AddString("091>>SET AUDIO_OUT <device_name>");
	m_ListCommands.AddString("092>>SET CALL <id> property");
	m_ListCommands.AddString("093>>SET CHATMESSAGE <id> SEEN ");
	m_ListCommands.AddString("094>>SET MESSAGE <id> SEEN ");
	m_ListCommands.AddString("095>>SET MUTE <ON|OFF>");
	m_ListCommands.AddString("096>>SET PROFILE <profile_property> <value>");
	m_ListCommands.AddString("097>>SET PROFILE <profile>");
	m_ListCommands.AddString("098>>SET PROFILE MOOD_TEXT <text>");
	m_ListCommands.AddString("099>>SET RINGER <device_name>");
	m_ListCommands.AddString("100>>SET USER <username> <property> <value>");
	m_ListCommands.AddString("101>>SET USERSTATUS <value>");
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CSkypeAPIApplicationDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CSkypeAPIApplicationDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CSkypeAPIApplicationDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CSkypeAPIApplicationDlg::OnButton1() 
{
	m_SkypeAPI.KeyPressd ( "1" );
    m_DailNum=m_DailNum+"1";
	UpdateData(FALSE);
}

void CSkypeAPIApplicationDlg::OnButton2() 
{
	m_SkypeAPI.KeyPressd ( "2" );
	m_DailNum=m_DailNum+"2";
	UpdateData(FALSE);
}

void CSkypeAPIApplicationDlg::OnButton3() 
{
	m_SkypeAPI.KeyPressd ( "3" );
	m_DailNum=m_DailNum+"3";
	UpdateData(FALSE);
}

void CSkypeAPIApplicationDlg::OnButton4() 
{
	m_SkypeAPI.KeyPressd ( "4" );
	m_DailNum=m_DailNum+"4";
	UpdateData(FALSE);
}

void CSkypeAPIApplicationDlg::OnButton5() 
{
	m_SkypeAPI.KeyPressd ( "5" );
	m_DailNum=m_DailNum+"5";
	UpdateData(FALSE);
}

void CSkypeAPIApplicationDlg::OnButton6() 
{
	m_SkypeAPI.KeyPressd ( "6" );
	m_DailNum=m_DailNum+"6";
	UpdateData(FALSE);
}

void CSkypeAPIApplicationDlg::OnButton7() 
{
	m_SkypeAPI.KeyPressd ( "7" );
	m_DailNum=m_DailNum+"7";
	UpdateData(FALSE);
}

void CSkypeAPIApplicationDlg::OnButton8() 
{
	m_SkypeAPI.KeyPressd ( "8" );
	m_DailNum=m_DailNum+"8";
	UpdateData(FALSE);
}
void CSkypeAPIApplicationDlg::OnButton0() 
{
	m_SkypeAPI.KeyPressd ( "0" );
	m_DailNum=m_DailNum+"0";
	UpdateData(FALSE);
}

void CSkypeAPIApplicationDlg::OnButtonUp() 
{
	m_SkypeAPI.KeyPressd ( "UP" );
}

void CSkypeAPIApplicationDlg::OnButtonDown() 
{
	m_SkypeAPI.KeyPressd ( "DOWN" );
}

void CSkypeAPIApplicationDlg::AddMessages(LPCTSTR lpszText)
{
	ASSERT ( lpszText );
	if ( !::IsWindow ( m_hWnd ) ) return;
	CEdit *pEditLog = (CEdit*) GetDlgItem ( IDC_EDIT_Message );
	if ( !pEditLog || !::IsWindow ( pEditLog->m_hWnd ) )
		return;
	
	CString csLog, csBuf;
	csBuf.Format ( "%s\r\n", lpszText );
	pEditLog->GetWindowText ( csLog );
	int nTextLen = csLog.GetLength ();
	if ( nTextLen > 256*1024 )
		nTextLen = 0;
	pEditLog->SetSel ( nTextLen, -1, TRUE );
	pEditLog->ReplaceSel( csBuf );
}

void CSkypeAPIApplicationDlg::ConnectSkypeSuccess()
{
	EnableItemsOnApplication ( TRUE );
	
	m_SkypeAPI.SendComToSkype ( "PROTOCOL 99999" );     // 查询协议版本
	m_SkypeAPI.SendComToSkype ( "GET SKYPEVERSION" );	// 查询Skype版本
	m_SkypeAPI.SendComToSkype ( "SEARCH FRIENDS" );     // 索取SkypeOut联络人名单
	m_SkypeAPI.SendComToSkype ( "GET USERSTATUS" );     // 查询当前用户状态
	m_SkypeAPI.SendComToSkype ( "GET CONNSTATUS" );     // 查询当前用户连接状态	
	m_SkypeAPI.SendComToSkype ( "FOCUS" );              // 激活skype窗口
}

LRESULT CSkypeAPIApplicationDlg::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	if ( m_SkypeAPI.TranslateMessage ( message, wParam, lParam ) ) //拦截窗口类的消息
		return TRUE;
	return CDialog::DefWindowProc(message, wParam, lParam);
}

void CSkypeAPIApplicationDlg::EnableItemsOnApplication(BOOL bEnable)
{
	GetDlgItem ( IDC_EDIT_Message )->EnableWindow ( bEnable );
	GetDlgItem ( IDC_COMBO_CONTACTS )->EnableWindow ( bEnable );
	GetDlgItem ( IDC_BUTTON_CALL )->EnableWindow ( bEnable );
	GetDlgItem ( IDC_BUTTON_HOOKON )->EnableWindow ( bEnable );
	GetDlgItem ( IDC_BUTTON_SMSG )->EnableWindow ( bEnable );
	GetDlgItem ( IDC_BUTTON_SKYPESEND )->EnableWindow ( bEnable );
	GetDlgItem ( IDC_BUTTON0 )->EnableWindow ( bEnable );
	GetDlgItem ( IDC_BUTTON1 )->EnableWindow ( bEnable );
	GetDlgItem ( IDC_BUTTON2 )->EnableWindow ( bEnable );
	GetDlgItem ( IDC_BUTTON3 )->EnableWindow ( bEnable );
	GetDlgItem ( IDC_BUTTON4 )->EnableWindow ( bEnable );
	GetDlgItem ( IDC_BUTTON5 )->EnableWindow ( bEnable );
	GetDlgItem ( IDC_BUTTON6 )->EnableWindow ( bEnable );
	GetDlgItem ( IDC_BUTTON7 )->EnableWindow ( bEnable );
	GetDlgItem ( IDC_BUTTON8 )->EnableWindow ( bEnable );
	GetDlgItem ( IDC_BUTTON9 )->EnableWindow ( bEnable );
	GetDlgItem ( IDC_BUTTON_XING )->EnableWindow ( bEnable );
	GetDlgItem ( IDC_BUTTON_JING)->EnableWindow ( bEnable );
	GetDlgItem ( IDC_BUTTON_UP )->EnableWindow ( bEnable );
	GetDlgItem ( IDC_BUTTON_DOWN )->EnableWindow ( bEnable );
	GetDlgItem ( IDC_BUTTON_DIAL )->EnableWindow ( bEnable );
	GetDlgItem ( IDC_EDITMSGTOSP )->EnableWindow ( bEnable );
	GetDlgItem ( IDC_BUTTON_SENDCOMTOSKYPE )->EnableWindow ( bEnable );
	GetDlgItem ( IDC_COM_LIST )->EnableWindow ( bEnable );	 
	GetDlgItem ( IDC_BUTTON_CE )->EnableWindow ( bEnable );	
	GetDlgItem ( IDC_EDIT_MSGTOFRIEND )->EnableWindow ( bEnable );	
}

CString CSkypeAPIApplicationDlg::GetCurrentSellectedUserName()
{
	CComboBox *pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_CONTACTS);
	ASSERT ( pComboBox );
	CString csUserName;
	pComboBox->GetWindowText ( csUserName );
	return csUserName;
}

void CSkypeAPIApplicationDlg::OnButtonCall() 
{
	m_SkypeAPI.SendComToSkype ( "CALL %s", GetCurrentSellectedUserName() );
}

void CSkypeAPIApplicationDlg::OnButtonHookon() 
{
	m_SkypeAPI.EndCurrentCall ();
	m_DailNum="";
	UpdateData(FALSE);
}

void CSkypeAPIApplicationDlg::OnButtonXing() 
{
	m_SkypeAPI.KeyPressd ( "*" );
	m_DailNum=m_DailNum+"*";
	UpdateData(FALSE);
}

void CSkypeAPIApplicationDlg::OnButtonJing() 
{
	m_SkypeAPI.KeyPressd ( "#" );
	m_DailNum=m_DailNum+"#";
	UpdateData(FALSE);
}

void CSkypeAPIApplicationDlg::OnButtonSendMSG() 
{
	CEdit *pMsgEdit = (CEdit*)GetDlgItem ( IDC_EDIT_MSGTOFRIEND );
	ASSERT ( pMsgEdit );
	CString csMessage;
	pMsgEdit->GetWindowText ( csMessage );
	if ( m_SkypeAPI.SendComToSkype ( "MESSAGE %s %s", GetCurrentSellectedUserName(), csMessage ) )
		pMsgEdit->SetWindowText ( "" );
}

void CSkypeAPIApplicationDlg::OnButtonSendMSGBySkype() 
{
	CEdit *pMsgEdit = (CEdit*)GetDlgItem ( IDC_EDIT_MSGTOFRIEND );
	ASSERT ( pMsgEdit );
	CString csMessage;
	pMsgEdit->GetWindowText ( csMessage );
	if ( m_SkypeAPI.SendComToSkype ( "OPEN IM %s %s", GetCurrentSellectedUserName(), csMessage ) )
		pMsgEdit->SetWindowText ( "" );
}

void CSkypeAPIApplicationDlg::OnTimer(UINT nIDEvent) 
{
	if ( m_SkypeAPI.IsSkypeWindowOK () )
	{
		KillTimer ( nIDEvent );
		ConnectSkypeSuccess ();
	}
	CDialog::OnTimer(nIDEvent);
}

void CSkypeAPIApplicationDlg::OnButton9() 
{
	m_SkypeAPI.KeyPressd ( "9" );
	m_DailNum=m_DailNum+"9";
	UpdateData(FALSE);
}

void CSkypeAPIApplicationDlg::OnButtonDial() 
{
	m_SkypeAPI.KeyPressd ( "YES" );
}
LRESULT CSkypeAPIApplicationDlg::GetUsers(WPARAM wParam, LPARAM lParam)
{
	CStringArray *pStrAry = reinterpret_cast<CStringArray*>(wParam);
	ASSERT ( pStrAry );
	int nCmdPos = (int)lParam;
	ASSERT ( nCmdPos > 0 );
	CComboBox *pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_CONTACTS);
	ASSERT ( pComboBox );
	pComboBox->ResetContent ();
	for ( int i=nCmdPos; i<pStrAry->GetSize(); i++ )
	{
		pComboBox->AddString ( pStrAry->GetAt(i) );
	}

	if ( pComboBox->GetCount() > 0 )
		pComboBox->SetCurSel ( 0 );

	return TRUE;
}

void CSkypeAPIApplicationDlg::OnButtonSendComToSkype() 
{
	UpdateData(TRUE);
	m_SkypeAPI.SendComToSkype(m_ComToSkype);
}

void CSkypeAPIApplicationDlg::SetListComToEdit() 
{
	CRect Rect,Rectm;                   
	CPoint pointm;     
	
	GetCursorPos(&pointm);          //取得鼠标当前的位置
	
	CString ItemAllText;
	m_ListCommands.ScreenToClient(&pointm);    //将ListBox窗口的坐标转化为屏幕坐标形式，使坐标统一

	for(int ItemIndex=0; ItemIndex<120;ItemIndex++)
	{
		m_ListCommands.GetItemRect(ItemIndex,&Rectm);  //通过Rectm取得ListBox一个条目的区域
		if(Rectm.PtInRect(pointm))                    //判断鼠标是否在Rectm区域内
		{
			CurrentListLine=ItemIndex;               //For循环中此句获得当前鼠标所在的条目位置
			break;
		}else{
			CurrentListLine=-1;                       //如果鼠标不在任何条目上时赋值为-1
		}
	}
	
	m_ListCommands.SetCurSel(CurrentListLine);               //将当前鼠标所在位置的条目高亮显示

	if(m_ListCommands.GetItemRect(CurrentListLine,&Rect))      
	{
		m_ListCommands.GetText(CurrentListLine,ItemAllText);
		ItemAllText.Delete(0,5);
		m_ComToSkype=ItemAllText;
		UpdateData(FALSE);            
		int x;
		int y;
		x=m_ComToSkype.Find("<",0);
		y=m_ComToSkype.Find(">",0)+1;
		m_ComSendToSKype.SetSel(x,y);
	}
}

void CSkypeAPIApplicationDlg::OnButtonHON() 
{
	m_SkypeAPI.SendComToSkype("OPEN DIALPAD");
	m_DailNum="";
	UpdateData(FALSE);
}
