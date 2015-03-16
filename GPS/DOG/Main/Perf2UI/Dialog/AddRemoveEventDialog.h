#pragma once

#include "Resource.h"
#include "afxwin.h"
#include <vector>
#include <map>
#include "EventIdNameMap.h"

class CAddRemoveEventDialog : public CDialog
{
public:
	DECLARE_DYNAMIC(CAddRemoveEventDialog)

public:
	enum { IDD = IDD_ADD_REMOVE_EVENT_DLG };
	CAddRemoveEventDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAddRemoveEventDialog();

	void SetSelectedEvent(const std::vector<DWORD>& selectedEventList);
	void GetSelectedEvent(std::vector<DWORD>& selectedEventList);

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	afx_msg void UpdateAvailableEventListView();
	afx_msg void OnButtonAdd();
	afx_msg void OnButtonRemove();
	afx_msg void OnButtonClear();
	afx_msg void OnButtonAddUserEvent();


	DECLARE_MESSAGE_MAP()

private:
	CListCtrl m_List_AvailableEvents;
	CListCtrl m_List_SelectedEvents;

	CButton m_CheckBoxAudio;
	CButton m_CheckBoxVideo;
	CButton m_CheckBoxDemux;
	CButton m_CheckBoxDisplay;

	CEdit m_EditUserEventId;
	CEdit m_EditUserEventName;

	TEventIdNameMap m_Available_Events;
	TEventIdNameMap m_Selected_Events;
};

