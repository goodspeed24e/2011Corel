// PropertyDlg.cpp : implementation file
//

#include "stdafx.h"
#include "codedecoder.h"
#include "PropertyDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPropertyDlg

IMPLEMENT_DYNAMIC(CPropertyDlg, CPropertySheet)

CPropertyDlg::CPropertyDlg(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
   AddPage(&encodeDlg);
   AddPage(&m_page1);
   AddPage(&m_page2);
}

CPropertyDlg::CPropertyDlg(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
   AddPage(&encodeDlg);
   AddPage(&m_page1);
   AddPage(&m_page2);
}

CPropertyDlg::~CPropertyDlg()
{
}


BEGIN_MESSAGE_MAP(CPropertyDlg, CPropertySheet)
	//{{AFX_MSG_MAP(CPropertyDlg)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPropertyDlg message handlers
