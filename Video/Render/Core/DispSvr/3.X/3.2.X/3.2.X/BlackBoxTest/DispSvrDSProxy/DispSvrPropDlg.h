#pragma once


// CDispSvrPropDlg dialog

class CDispSvrPropDlg : public CDialog
{
	DECLARE_DYNAMIC(CDispSvrPropDlg)

public:
	CDispSvrPropDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDispSvrPropDlg();

// Dialog Data
	enum { IDD = IDD_DISPSVR_PROPPAGE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton1();
};
