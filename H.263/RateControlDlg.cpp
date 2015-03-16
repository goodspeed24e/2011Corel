// RateControlDlg.cpp : implementation file
//

#include "stdafx.h"
#include "codedecoder.h"
#include "RateControlDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRateControlDlg property page

IMPLEMENT_DYNCREATE(CRateControlDlg, CPropertyPage)

CRateControlDlg::CRateControlDlg() : CPropertyPage(CRateControlDlg::IDD)
{
	//{{AFX_DATA_INIT(CRateControlDlg)
	m_bitrate = 150000;
	m_ifratecontrol = FALSE;
	m_framerate = 0;
	//}}AFX_DATA_INIT
}

CRateControlDlg::~CRateControlDlg()
{
}

void CRateControlDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRateControlDlg)
	DDX_Text(pDX, IDC_Bitrate, m_bitrate);
	DDV_MinMaxInt(pDX, m_bitrate, 100, 10000000);
	DDX_Check(pDX, IDC_IfRateControl, m_ifratecontrol);
	DDX_Radio(pDX, IDC_frame25, m_framerate);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRateControlDlg, CPropertyPage)
	//{{AFX_MSG_MAP(CRateControlDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRateControlDlg message handlers
