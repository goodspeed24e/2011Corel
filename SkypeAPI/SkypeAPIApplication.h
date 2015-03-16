// SkypeAPIApplication.h : main header file for the SKYPEAPIAPPLICATION application
//

#if !defined(AFX_SKYPEAPIAPPLICATION_H__9BCEA085_1E27_473C_A14B_1CEA10D05DC3__INCLUDED_)
#define AFX_SKYPEAPIAPPLICATION_H__9BCEA085_1E27_473C_A14B_1CEA10D05DC3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CSkypeAPIApplicationApp:
// See SkypeAPIApplication.cpp for the implementation of this class
//

class CSkypeAPIApplicationApp : public CWinApp
{
public:
	CSkypeAPIApplicationApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSkypeAPIApplicationApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CSkypeAPIApplicationApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SKYPEAPIAPPLICATION_H__9BCEA085_1E27_473C_A14B_1CEA10D05DC3__INCLUDED_)
