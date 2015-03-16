// VideoWnd.cpp : implementation file
//

#include "stdafx.h"
#include "HVDServiceTester.h"
#include "VideoWnd.h"


// CVideoWnd

IMPLEMENT_DYNAMIC(CVideoWnd, CWnd)

CVideoWnd::CVideoWnd()
{

}

CVideoWnd::~CVideoWnd()
{
}

BOOL CVideoWnd::Create(CWnd *pParentWnd, CPoint ptTopLeft)
{

	COLORREF BrushColor = GetSysColor(COLOR_MENUBAR);

	CRect rect(CPoint(0, 0), CSize(640, 480));

	if (pParentWnd)
	{
		CRect ParentRect;
		pParentWnd->GetClientRect(&ParentRect);
		pParentWnd->ClientToScreen(&ParentRect);
		rect.MoveToXY(ParentRect.TopLeft());
	}
	else
	{
		rect.MoveToXY(ptTopLeft);
	}

	CString DispSvrCtrlDialogClass;

	DispSvrCtrlDialogClass = AfxRegisterWndClass(
		CS_VREDRAW | CS_HREDRAW,
		::LoadCursor(NULL, IDC_ARROW),
		(HBRUSH) CreateSolidBrush(BrushColor),
		::LoadIcon(NULL, IDI_WINLOGO));

	BOOL bReturnValue = FALSE;

	bReturnValue = CWnd::CreateEx(NULL, DispSvrCtrlDialogClass, _T("Video Window"),
		WS_POPUPWINDOW|WS_CAPTION|WS_VISIBLE|WS_MAXIMIZEBOX|WS_MINIMIZEBOX|WS_THICKFRAME,
		rect, pParentWnd, NULL);

	return bReturnValue;
}

BEGIN_MESSAGE_MAP(CVideoWnd, CWnd)
	ON_WM_CLOSE()
END_MESSAGE_MAP()



// CVideoWnd message handlers



void CVideoWnd::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	ShowWindow(SW_HIDE);
//	CWnd::OnClose();
}
