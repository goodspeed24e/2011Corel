// 视频编解码器View.h : interface of the CMyView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_VIEW_H__1247F38D_CA2A_4348_AB3D_72E1DC1137BF__INCLUDED_)
#define AFX_VIEW_H__1247F38D_CA2A_4348_AB3D_72E1DC1137BF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



class CMyView : public CView
{
protected: // create from serialization only
	CMyView();
	DECLARE_DYNCREATE(CMyView)

// Attributes
public:
	CMyDoc* GetDocument();

// Operations
public:
	//率控制
	int ratecontrol;
	int  targetrate;
	float ref_frame_rate;
    //encoding parameters
	int bFlag;
	CString csTimeElapse;
	CString conclusion;

	CString	m_szFilePathName;
	CString m_szFileName;
	int MaxFrame;
	int m_orgWidth;
	int m_orgHeight;
	unsigned char * m_pImageData;
	int m_Type;
	int QP,QPI;
	int Pbetween;
	int ifPsnr;
	double psnrs[3];
    //decoding parameters
    CString DecfileName;
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMyView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

// Implementation
public:
	BOOL PeekAndPump();
	void CodeBmps();
	void CodeYUV();
	virtual ~CMyView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	CFont m_font;
// Generated message map functions
protected:
	//{{AFX_MSG(CMyView)
	afx_msg void OnEncode();
	afx_msg void OnDecode();
	afx_msg void OnFileOpen();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in 视频编解码器View.cpp
inline CMyDoc* CMyView::GetDocument()
   { return (CMyDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VIEW_H__1247F38D_CA2A_4348_AB3D_72E1DC1137BF__INCLUDED_)
