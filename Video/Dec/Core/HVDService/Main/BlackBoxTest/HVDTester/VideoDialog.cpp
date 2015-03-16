// VideoDialog.cpp : implementation file
//

#include "stdafx.h"
#include "HVDServiceTester.h"
#include "VideoDialog.h"


// CVideoDialog dialog

IMPLEMENT_DYNAMIC(CVideoDialog, CDialog)

CVideoDialog::CVideoDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CVideoDialog::IDD, pParent)
{

}

CVideoDialog::~CVideoDialog()
{
}

void CVideoDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CVideoDialog, CDialog)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CVideoDialog message handlers

void CVideoDialog::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	ShowWindow(SW_HIDE);
}
