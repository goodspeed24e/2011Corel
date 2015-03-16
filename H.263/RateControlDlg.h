#if !defined(AFX_RATECONTROLDLG_H__62108696_BC70_46B6_9B69_CD5E8BC7649D__INCLUDED_)
#define AFX_RATECONTROLDLG_H__62108696_BC70_46B6_9B69_CD5E8BC7649D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RateControlDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRateControlDlg dialog

class CRateControlDlg : public CPropertyPage
{
	DECLARE_DYNCREATE(CRateControlDlg)

// Construction
public:
	CRateControlDlg();
	~CRateControlDlg();

// Dialog Data
	//{{AFX_DATA(CRateControlDlg)
	enum { IDD = IDD_RateControl };
	int		m_bitrate;
	BOOL	m_ifratecontrol;
	int		m_framerate;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CRateControlDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CRateControlDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RATECONTROLDLG_H__62108696_BC70_46B6_9B69_CD5E8BC7649D__INCLUDED_)
