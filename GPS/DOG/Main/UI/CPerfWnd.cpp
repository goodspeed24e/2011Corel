#include "stdafx.h"
#include "MyApp.h"
#include "MainFrame.h"
#include "CPerfWnd.h"
#include "CChartData.h"

const POINT CPerfWnd::m_WindowMinSize = {500,250};
const POINT CPerfWnd::m_WindowMaxSize = {10000,800};

CPerfWnd::CPerfWnd()
{
}

BEGIN_MESSAGE_MAP(CPerfWnd, CFrameWnd)
    ON_WM_DESTROY()
	ON_WM_GETMINMAXINFO()
END_MESSAGE_MAP()

void CPerfWnd::OnDestroy()
{	
	//Base handler
	CFrameWnd::OnDestroy();
}

void CPerfWnd::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	lpMMI->ptMinTrackSize = m_WindowMinSize;
	lpMMI->ptMaxTrackSize = m_WindowMaxSize;
}