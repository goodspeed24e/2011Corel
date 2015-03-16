#pragma once


// CVideoWnd

class CVideoWnd : public CWnd
{
	DECLARE_DYNAMIC(CVideoWnd)

public:
	CVideoWnd();
	virtual ~CVideoWnd();

	BOOL Create(CWnd *pParentWnd, CPoint ptTopLeft);

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnClose();
};


