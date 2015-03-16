// SkypeAPIApplicationDlg.h : header file
//

#if !defined(AFX_SKYPEAPIAPPLICATIONDLG_H__F9218F0E_1D5D_4973_98C3_9F925FE7B573__INCLUDED_)
#define AFX_SKYPEAPIAPPLICATIONDLG_H__F9218F0E_1D5D_4973_98C3_9F925FE7B573__INCLUDED_

//#include "SkypeAPI.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "SkypeAPI.h"
/////////////////////////////////////////////////////////////////////////////
// CSkypeAPIApplicationDlg dialog

class CSkypeAPIApplicationDlg : public CDialog
{
// Construction
public:
	CString GetCurrentSellectedUserName();
	void EnableItemsOnApplication(BOOL bEnable);
	void ConnectSkypeSuccess();
	void AddMessages(LPCTSTR lpszText);
	CSkypeAPIApplicationDlg(CWnd* pParent = NULL);	// standard constructor
// Dialog Data
	//{{AFX_DATA(CSkypeAPIApplicationDlg)
	enum { IDD = IDD_SKYPEAPIAPPLICATION_DIALOG };
	CEdit	m_ComSendToSKype;
	CListBox	m_ListCommands;
	CString	m_DailNum;
	int CurrentListLine;
	CString	m_ComToSkype;
	//}}AFX_DATA
	
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSkypeAPIApplicationDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CSkypeAPIApplicationDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnButton1();
	afx_msg void OnButton2();
	afx_msg void OnButton3();
	afx_msg void OnButton4();
	afx_msg void OnButton5();
	afx_msg void OnButton6();
	afx_msg void OnButton7();
	afx_msg void OnButton8();
	afx_msg void OnButton0();
	afx_msg void OnButtonUp();
	afx_msg void OnButtonDown();
	afx_msg void OnButtonXing();
	afx_msg void OnButtonJing();
	afx_msg void OnButtonSendMSG();
	afx_msg void OnButtonSendMSGBySkype();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnButton9();
	afx_msg void OnButtonDial();
	afx_msg void OnButtonCall();
	afx_msg void OnButtonSendComToSkype();
	afx_msg void OnButtonHookon();
	afx_msg void SetListComToEdit();
	afx_msg void OnButtonHON();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	CSkypeAPI m_SkypeAPI;
	LRESULT GetUsers(WPARAM wParam, LPARAM lParam);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SKYPEAPIAPPLICATIONDLG_H__F9218F0E_1D5D_4973_98C3_9F925FE7B573__INCLUDED_)
