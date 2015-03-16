#pragma once
#include "perf2.h"
#include "afxwin.h"
#include "MyApp.h"
#include <map>

// CControlDialog dialog

class CControlDialog : public CDialog
{
	DECLARE_DYNAMIC(CControlDialog)

public:
	CControlDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CControlDialog();
	virtual BOOL OnInitDialog();

// Dialog Data
	enum { IDD = IDD_CONTROLDIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	CListBox m_lstbxAvailable;
	CListBox m_lstbxActive;
	//CStringArray m_strArrayEventId;
	afx_msg void OnBnClickedAddbutton();
	afx_msg void OnBnClickedRemovebutton();
	afx_msg void OnBnClickedResetbutton();
	afx_msg void OnBnClickedOk();
	void LoadDefaultItems();
	void UpdateListBox();
	void InitData();
	std::map<CString, int> m_mapEventID, m_mapSelectedEventID;
	afx_msg void OnBnClickedCheckaudio();
	afx_msg void OnBnClickedCheckvideo();
	afx_msg void OnBnClickedCheckdemux();
	afx_msg void OnBnClickedCheckdisplay();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedButtonAddEvent();
	afx_msg void OnBnClickedClearbutton();
};
