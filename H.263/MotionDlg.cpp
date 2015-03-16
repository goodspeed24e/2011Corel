// MotionDlg.cpp : implementation file
//

#include "stdafx.h"
#include "codedecoder.h"
#include "MotionDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMotionDlg property page

IMPLEMENT_DYNCREATE(CMotionDlg, CPropertyPage)

CMotionDlg::CMotionDlg() : CPropertyPage(CMotionDlg::IDD)
{
	//{{AFX_DATA_INIT(CMotionDlg)
	m_mulResolution = 0;
	//}}AFX_DATA_INIT
}

CMotionDlg::~CMotionDlg()
{
}

void CMotionDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMotionDlg)
	DDX_Radio(pDX, IDC_RADIO1, m_mulResolution);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMotionDlg, CPropertyPage)
	//{{AFX_MSG_MAP(CMotionDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMotionDlg message handlers
