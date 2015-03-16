// ChartFrm.cpp : implementation of the CChildFrame class
//
#include "stdafx.h"
#include "MyApp.h"
#include "EventIdNameMap.h"

#include "ChartFrm.h"
#include "MainFrm.h"
#include "AddRemoveEventDialog.h"
#include "InputBox.h"
#include <set>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CChildFrame


BEGIN_MESSAGE_MAP(CChildFrame, CMDIChildWnd)
	ON_COMMAND(ID_FILE_CLOSE, &CChildFrame::OnFileClose)
	ON_COMMAND(ID_ADD_REMOVE_EVENT, &CChildFrame::OnAddRemoveEvent)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CChildFrame construction/destruction

CChildFrame::CChildFrame()
{
	// TODO: add member initialization code here
}

CChildFrame::~CChildFrame()
{

}

void CChildFrame::OnFileClose() 
{
	// To close the frame, just send a WM_CLOSE, which is the equivalent
	// choosing close from the system menu.
	SendMessage(WM_CLOSE);
}

int CChildFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIChildWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}

void CChildFrame::OnDestroy()
{
	std::vector<DWORD> curEventList = GetEventView()->GetEventList();

	std::vector<DWORD>::iterator iter;
	for(iter = curEventList.begin(); iter!=curEventList.end(); ++iter)
	{
		DWORD removedId = *iter;	
		theApp.RemoveInterestedEventId(removedId);
	}

	CMainFrame* pMainFrm = dynamic_cast<CMainFrame*>(GetOwner());
	if(pMainFrm) 
	{
		pMainFrm->OnChildDestory(this);
	}
}

void CChildFrame::OnSetTitle()
{
	CInputBox inputbox(GetSafeHwnd());
	if (inputbox.DoModal(_T("Set Title"), _T("Input window title"), GetTitle().GetBuffer()) )
	{
		SetTitle(inputbox.Text);
		SetWindowText(GetTitle());
	}
}

void CChildFrame::OnClearEventData()
{
	theApp.GetEventListener()->ClearData();
}

void CChildFrame::OnAddRemoveEvent()
{
	std::vector<DWORD> curEventList = GetEventView()->GetEventList();

	CAddRemoveEventDialog dlg;
	dlg.SetSelectedEvent(curEventList);
	if( IDOK == dlg.DoModal())
	{
		std::vector<DWORD> newEventList;
		dlg.GetSelectedEvent(newEventList);


		// get removedEvents
		// curEventList - newEventList = removedEvents
		std::set<DWORD> removedEvents(curEventList.begin(), curEventList.end());
		for(int i=0; i<newEventList.size(); ++i)
		{
			DWORD id = newEventList[i];
			removedEvents.erase(id);
		}

		// get addedEvents
		// newEventList - curEventList = addedEvents
		std::set<DWORD> addedEvents(newEventList.begin(), newEventList.end());
		for(int i=0; i<curEventList.size(); ++i)
		{
			DWORD id = curEventList[i];
			addedEvents.erase(id);
		}

		std::set<DWORD>::iterator iter;

		// update addedEvents
		for(iter = addedEvents.begin(); iter!=addedEvents.end(); ++iter)
		{
			DWORD addedId = *iter;
			GetEventView()->AddEvent(addedId, g_EventIdNameMap.GetFriendlyEventName(addedId));
			theApp.AddInterestedEventId(addedId);
		}

		// update removedEvents
		for(iter = removedEvents.begin(); iter!=removedEvents.end(); ++iter)
		{
			DWORD removedId = *iter;
			GetEventView()->RemoveEvent(removedId);
			theApp.RemoveInterestedEventId(removedId);
		}
	}
}

