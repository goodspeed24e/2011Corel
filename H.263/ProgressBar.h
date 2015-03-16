#if !defined(AFX_PROGRESSBAR_H__08333FA6_BB2A_4D7B_9D83_E5431C049383__INCLUDED_)
#define AFX_PROGRESSBAR_H__08333FA6_BB2A_4D7B_9D83_E5431C049383__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ProgressBar.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CProgressBar window

class CProgressBar : public CProgressCtrl
{
// Construction
public:
	CProgressBar();
	CProgressBar(LPCTSTR strMessage, int nSize=100, int MaxValue=100, BOOL bSmooth=FALSE);
    BOOL Create(LPCTSTR strMessage, int nSize=100, int MaxValue=100, BOOL bSmooth=FALSE);


// Attributes
public:

// Operations
public:
	BOOL Success() {return m_bSuccess;}// Was the creation successful?

    void SetRange(int nLower, int nUpper, int nStep = 1);
    void SetText(LPCTSTR strMessage);
    void SetSize(int nSize);
    int  SetStep(int nStep);
    int  StepIt();
    void Clear();


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProgressBar)
	public:
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CProgressBar();

	// Generated message map functions
protected:
	//{{AFX_MSG(CProgressBar)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG

	BOOL		m_bSuccess;		// Successfully created?
    int			m_nSize;		// Percentage size of control
    CString		m_strMessage;	// Message to display to left of control
	CRect		m_Rect;			// Dimensions of the whole thing

    CStatusBar *GetStatusBar();
    void Resize();


	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROGRESSBAR_H__08333FA6_BB2A_4D7B_9D83_E5431C049383__INCLUDED_)
