#if !defined(AFX_MOTIONDLG_H__587737E9_CDEB_4308_ACAB_424351A9F2C0__INCLUDED_)
#define AFX_MOTIONDLG_H__587737E9_CDEB_4308_ACAB_424351A9F2C0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MotionDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMotionDlg dialog

class CMotionDlg : public CPropertyPage
{
	DECLARE_DYNCREATE(CMotionDlg)

// Construction
public:
	CMotionDlg();
	~CMotionDlg();

// Dialog Data
	//{{AFX_DATA(CMotionDlg)
	enum { IDD = IDD_Motion };
	int		m_mulResolution;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CMotionDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CMotionDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MOTIONDLG_H__587737E9_CDEB_4308_ACAB_424351A9F2C0__INCLUDED_)
