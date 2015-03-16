// HolderView.cpp : implementation file
//

#include "stdafx.h"
#include "HolderView.h"

IMPLEMENT_DYNCREATE(CHolderView, CView)


CHolderView::CHolderView()
{
	m_pAttachedWnd=NULL;
}

CHolderView::~CHolderView()
{
}

BEGIN_MESSAGE_MAP(CHolderView, CView)
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CHolderView drawing


// CHolderView diagnostics

#ifdef _DEBUG
void CHolderView::AssertValid() const
{
	CView::AssertValid();
}

void CHolderView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG


// CHolderView message handlers

void CHolderView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	if(m_pAttachedWnd==NULL) {
		return;
	}
	if(!(::IsWindow(m_pAttachedWnd->m_hWnd))) { 
		return;
	}
	
	m_pAttachedWnd->MoveWindow(0,0,cx,cy);
}


BOOL CHolderView::PreCreateWindow(CREATESTRUCT& cs)
{
	BOOL bRes = CView::PreCreateWindow(cs);

	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;

	return bRes;
}

void CHolderView::AttachWnd(CWnd* pWnd, CWnd* pOwner)
{
	ASSERT(pOwner);
	m_pOwner = pOwner;

	ASSERT(pWnd);
	m_pAttachedWnd = pWnd;
	if(::IsWindow(m_hWnd) && ::IsWindow(pWnd->m_hWnd)) 
	{
		CRect rect;
		GetClientRect(rect);
		m_pAttachedWnd->MoveWindow(rect);
	}
}

BOOL CHolderView::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if(m_pOwner)
	{
		return (BOOL) m_pOwner->SendMessage(WM_COMMAND, wParam, lParam);
	}
	else
	{
		return FALSE;
	}
}
