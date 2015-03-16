// DispSvrPropDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DispSvrDSProxy.h"
#include "DispSvrPropDlg.h"


// CDispSvrPropDlg dialog

IMPLEMENT_DYNAMIC(CDispSvrPropDlg, CDialog)

CDispSvrPropDlg::CDispSvrPropDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDispSvrPropDlg::IDD, pParent)
{

}

CDispSvrPropDlg::~CDispSvrPropDlg()
{
}

void CDispSvrPropDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDispSvrPropDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON1, &CDispSvrPropDlg::OnBnClickedButton1)
END_MESSAGE_MAP()


// CDispSvrPropDlg message handlers

void CDispSvrPropDlg::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
}
