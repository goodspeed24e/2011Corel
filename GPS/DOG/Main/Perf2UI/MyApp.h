// MyApp.h : main header file for the Perf2 application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "EventListener.h"
#include "resource.h"       // main symbols

class CChildFrame;

// CMyApp:
// See MyApp.cpp for the implementation of this class
//

class CMyApp : public CWinApp
{
public:
	CMyApp();


// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

	void AddInterestedEventId(DWORD eventId)
	{
		m_EventListener.AddInterestedEventId(eventId);
	}

	void RemoveInterestedEventId(DWORD eventId)
	{
		m_EventListener.RemoveInterestedEventId(eventId);
	}

	CEventListener* GetEventListener() 
	{
		return &m_EventListener;
	}


// Implementation
protected:
	CEventListener m_EventListener;

public:
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CMyApp theApp;

