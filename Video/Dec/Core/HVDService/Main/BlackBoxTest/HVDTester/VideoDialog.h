#pragma once


// CVideoDialog dialog

class CVideoDialog : public CWnd
{
	DECLARE_DYNAMIC(CVideoDialog)

public:
	CVideoDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CVideoDialog();

// Dialog Data
	enum { IDD = IDD_VIDEO_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnClose();
};
