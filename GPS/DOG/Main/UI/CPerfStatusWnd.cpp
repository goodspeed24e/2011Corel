#include "stdafx.h"
#include "CPerfStatusWnd.h"
#include "structData.h"
#include <math.h>

static int s_OriginX = 20;
static int s_OriginY = 20;
static int s_tickNum = 20;
static int s_dataNum  = 200;

CPerfStatusWnd::CPerfStatusWnd()
{
}

CPerfStatusWnd::CPerfStatusWnd(int DataSource)
{
}

afx_msg int CPerfStatusWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	CPerfWnd::OnCreate(lpCreateStruct);

	return 0;
}

afx_msg void CPerfStatusWnd::OnPaint()
{
	CPaintDC dc (this);
	CRect rect;
	GetClientRect (&rect);
}

afx_msg void CPerfStatusWnd::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{

	this->Invalidate();
}


BEGIN_MESSAGE_MAP(CPerfStatusWnd, CPerfWnd)
	//ON_MESSAGE(WM_HSCROLL, OnHScroll)
	ON_WM_HSCROLL()
	ON_WM_CREATE()
	ON_WM_PAINT()
END_MESSAGE_MAP()
