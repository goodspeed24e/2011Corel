#if !defined(AFX_PROPERTYDLG_H__9CACB5A2_876F_4AEE_B1D2_E6A1A1004A75__INCLUDED_)
#define AFX_PROPERTYDLG_H__9CACB5A2_876F_4AEE_B1D2_E6A1A1004A75__INCLUDED_

#include "EncodeDlg.h"
#include "RateControlDlg.h"
#include "MotionDlg.h"
#include "EncodeDlg.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PropertyDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPropertyDlg

class CPropertyDlg : public CPropertySheet
{
	DECLARE_DYNAMIC(CPropertyDlg)

// Construction
public:
	CPropertyDlg(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	CPropertyDlg(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

// Attributes
public:
	CRateControlDlg m_page1;
	CMotionDlg m_page2;
	CEncodeDlg encodeDlg;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPropertyDlg)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CPropertyDlg();

	// Generated message map functions
protected:
	//{{AFX_MSG(CPropertyDlg)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROPERTYDLG_H__9CACB5A2_876F_4AEE_B1D2_E6A1A1004A75__INCLUDED_)
