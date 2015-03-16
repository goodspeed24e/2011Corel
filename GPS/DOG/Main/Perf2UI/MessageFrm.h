// MessageFrm.h : interface of the CChildFrame class
//


#pragma once

#include "ChildFrm.h"
#include "MessageView.h"
#include "EventView.h"

class CMessageFrame : public CChildFrame
{
	DECLARE_DYNCREATE(CMessageFrame)
public:
	CMessageFrame();
	virtual ~CMessageFrame();

public:
	virtual int GetWndType() 
	{
		return CChildFrame::MESSAGE_WND;
	}

	virtual CEventView* GetEventView()
	{
		return &m_messageView;
	}

	// Overrides
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSetRefreshTime();
	DECLARE_MESSAGE_MAP()

	// view for the client area of the frame.
	CMessageView m_messageView;
};
