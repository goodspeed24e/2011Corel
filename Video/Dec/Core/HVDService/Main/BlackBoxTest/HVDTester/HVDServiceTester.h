// HVDServiceTester.h : main header file for the HVDServiceTester application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CHVDServiceTesterApp:
// See HVDServiceTester.cpp for the implementation of this class
//

class CHVDServiceTesterApp : public CWinApp
{
public:
	CHVDServiceTesterApp();


// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation
protected:
	HMENU m_hMDIMenu;
	HACCEL m_hMDIAccel;

public:
	afx_msg void OnAppAbout();
	afx_msg void OnFileNew();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnFileLoad();
};

extern CHVDServiceTesterApp theApp;