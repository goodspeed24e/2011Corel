#include "stdafx.h"
#include "AddRemoveEventDialog.h"
#include "DogEventId.h"
#include "EventIdNameMap.h"

IMPLEMENT_DYNAMIC(CAddRemoveEventDialog, CDialog)

using namespace dog;

CAddRemoveEventDialog::CAddRemoveEventDialog(CWnd* pParent)
	: CDialog(IDD, pParent)
{

}

CAddRemoveEventDialog::~CAddRemoveEventDialog()
{
	m_List_AvailableEvents.Detach();
	m_List_SelectedEvents.Detach();

	m_CheckBoxDemux.Detach();
	m_CheckBoxAudio.Detach();
	m_CheckBoxVideo.Detach();	
	m_CheckBoxDisplay.Detach();

	m_EditUserEventId.Detach();
	m_EditUserEventName.Detach();
}

BOOL CAddRemoveEventDialog::OnInitDialog()
{
	m_CheckBoxDemux.Attach(GetDlgItem(IDC_DLG1_CBX_SHOW_DEMUX)->GetSafeHwnd());
	m_CheckBoxAudio.Attach(GetDlgItem(IDC_DLG1_CBX_SHOW_AUDIO)->GetSafeHwnd());
	m_CheckBoxVideo.Attach(GetDlgItem(IDC_DLG1_CBX_SHOW_VIDEO)->GetSafeHwnd());	
	m_CheckBoxDisplay.Attach(GetDlgItem(IDC_DLG1_CBX_SHOW_DISPLAY)->GetSafeHwnd());

	m_EditUserEventId.Attach(GetDlgItem(IDC_DLG1_EDIT_USEREVENTID)->GetSafeHwnd());
	m_EditUserEventName.Attach(GetDlgItem(IDC_DLG1_EDIT_USEREVENT)->GetSafeHwnd());

	m_List_AvailableEvents.Attach(GetDlgItem(IDC_DLG1_LIST_AVAILABLE)->GetSafeHwnd());
	const int nIdFieldWidth = 70;
	RECT rect;
	{
		m_List_AvailableEvents.SetExtendedStyle(LVS_EX_FULLROWSELECT | m_List_AvailableEvents.GetExtendedStyle());
		m_List_AvailableEvents.GetWindowRect(&rect);
		const UINT width = rect.right - rect.left;
		const UINT height = rect.bottom - rect.top;
		m_List_AvailableEvents.InsertColumn(0, _T("Id"), LVCFMT_LEFT, nIdFieldWidth);
		m_List_AvailableEvents.InsertColumn(1, _T("Friendly Name"), LVCFMT_LEFT, width-nIdFieldWidth-25);
	}

	m_List_SelectedEvents.Attach(GetDlgItem(IDC_DLG1_LIST_ACTIVE)->GetSafeHwnd());
	{
		m_List_SelectedEvents.SetExtendedStyle(LVS_EX_FULLROWSELECT | m_List_SelectedEvents.GetExtendedStyle());
		m_List_SelectedEvents.GetWindowRect(&rect);
		const UINT width = rect.right - rect.left;
		const UINT height = rect.bottom - rect.top;
		m_List_SelectedEvents.InsertColumn(0, _T("Id"), LVCFMT_LEFT, nIdFieldWidth);
		m_List_SelectedEvents.InsertColumn(1, _T("Friendly Name"), LVCFMT_LEFT, width-nIdFieldWidth-25);
	}

	UpdateAvailableEventListView();
	return TRUE;
}

void CAddRemoveEventDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


inline void AddEventToListView(TEventIdNameMap& EventMap, CListCtrl& listCtrl)
{
	TEventIdNameMap::iterator iter = EventMap.begin();
	for(; iter!=EventMap.end(); ++iter)
	{
		int iItem = listCtrl.GetItemCount();

		DWORD id = iter->first;
		TCHAR strId[32];
		_stprintf_s(strId, sizeof(strId)/sizeof(TCHAR), _T("%d"), id);

		LVITEM item = {LVIF_TEXT | LVIF_PARAM, iItem, 0, 0, 0, strId, 0, 0, id};
		iItem = listCtrl.InsertItem(&item);
		listCtrl.SetItemText(iItem, 1, iter->second.c_str());
	}
}

void CAddRemoveEventDialog::UpdateAvailableEventListView()
{
	m_Available_Events.clear();

	if(m_CheckBoxDemux.GetCheck())
	{
		m_Available_Events.insert(g_EventIdNameMap.m_DemuxEvents.begin(), g_EventIdNameMap.m_DemuxEvents.end());
	}

	if(m_CheckBoxAudio.GetCheck())
	{
		m_Available_Events.insert(g_EventIdNameMap.m_AudioEvents.begin(), g_EventIdNameMap.m_AudioEvents.end());
	}

	if(m_CheckBoxVideo.GetCheck())
	{
		m_Available_Events.insert(g_EventIdNameMap.m_VideoEvents.begin(), g_EventIdNameMap.m_VideoEvents.end());
	}

	if(m_CheckBoxDisplay.GetCheck())
	{
		m_Available_Events.insert(g_EventIdNameMap.m_DisplayEvents.begin(), g_EventIdNameMap.m_DisplayEvents.end());
	}

	TEventIdNameMap::iterator iter = m_Selected_Events.begin();
	for(; iter!=m_Selected_Events.end(); ++iter)
	{
		m_Available_Events.erase(iter->first);
	}

	m_List_AvailableEvents.DeleteAllItems();
	AddEventToListView(m_Available_Events, m_List_AvailableEvents);

	m_List_SelectedEvents.DeleteAllItems();
	AddEventToListView(m_Selected_Events, m_List_SelectedEvents);
}

void CAddRemoveEventDialog::SetSelectedEvent(const std::vector<DWORD>& selectedEventList)
{
	int n= selectedEventList.size();
	for(int i=0; i<n; ++i)
	{
		DWORD id = selectedEventList[i];
		m_Selected_Events[id] = g_EventIdNameMap.m_TotalEvents[id];
	}
}

void CAddRemoveEventDialog::GetSelectedEvent(std::vector<DWORD>& selectedEventList)
{
	size_t n = m_Selected_Events.size();
	selectedEventList.reserve(n);

	TEventIdNameMap::iterator iter = m_Selected_Events.begin();
	for(; iter!=m_Selected_Events.end(); ++iter)
	{
		selectedEventList.push_back(iter->first);
	}
}

BEGIN_MESSAGE_MAP(CAddRemoveEventDialog, CDialog)
	ON_BN_CLICKED(IDC_DLG1_CBX_SHOW_AUDIO, UpdateAvailableEventListView)
	ON_BN_CLICKED(IDC_DLG1_CBX_SHOW_VIDEO, UpdateAvailableEventListView)
	ON_BN_CLICKED(IDC_DLG1_CBX_SHOW_DEMUX, UpdateAvailableEventListView)
	ON_BN_CLICKED(IDC_DLG1_CBX_SHOW_DISPLAY, UpdateAvailableEventListView)
	ON_BN_CLICKED(IDC_DLG1_BUTTON_ADD_USEREVENT, OnButtonAddUserEvent)
	ON_BN_CLICKED(IDC_DLG1_LIST_AVAILABLE, OnButtonAdd)
	ON_BN_CLICKED(IDC_DLG1_LIST_ACTIVE, OnButtonAdd)
	ON_BN_CLICKED(IDC_DLG1_BUTTON_ADD, OnButtonAdd)
	ON_BN_CLICKED(IDC_DLG1_BUTTON_REMOVE, OnButtonRemove)
	ON_BN_CLICKED(IDC_DLG1_BUTTON_CLEAR, OnButtonClear)
END_MESSAGE_MAP()


void CAddRemoveEventDialog::OnButtonAdd()
{
	POSITION p = m_List_AvailableEvents.GetFirstSelectedItemPosition();
	while (p)
	{
		int nSelected = m_List_AvailableEvents.GetNextSelectedItem(p);
		DWORD id = m_List_AvailableEvents.GetItemData(nSelected);
 		//CString strId = m_List_AvailableEvents.GetItemText(nSelected, 0);
 		//CString strName = m_List_AvailableEvents.GetItemText(nSelected, 1);

		m_Selected_Events[id] = m_Available_Events[id];
		m_Available_Events.erase(id);
	}

	m_List_SelectedEvents.DeleteAllItems();
	AddEventToListView(m_Selected_Events, m_List_SelectedEvents);

	m_List_AvailableEvents.DeleteAllItems();
	AddEventToListView(m_Available_Events, m_List_AvailableEvents);
}

void CAddRemoveEventDialog::OnButtonRemove()
{
	POSITION p = m_List_SelectedEvents.GetFirstSelectedItemPosition();
	while (p)
	{
		int nSelected = m_List_SelectedEvents.GetNextSelectedItem(p);
		DWORD id = m_List_SelectedEvents.GetItemData(nSelected);

		m_Available_Events[id] = m_Selected_Events[id];
		m_Selected_Events.erase(id);
	}

	m_List_SelectedEvents.DeleteAllItems();
	AddEventToListView(m_Selected_Events, m_List_SelectedEvents);

	m_List_AvailableEvents.DeleteAllItems();
	AddEventToListView(m_Available_Events, m_List_AvailableEvents);
}

void CAddRemoveEventDialog::OnButtonClear()
{
	m_Selected_Events.clear();
	m_List_SelectedEvents.DeleteAllItems();
	UpdateAvailableEventListView();
}

void CAddRemoveEventDialog::OnButtonAddUserEvent()
{
	CString strId;
	CString strEventName;
	m_EditUserEventId.GetWindowText(strId);
	m_EditUserEventName.GetWindowText(strEventName);
	strEventName.Trim();

	DWORD id = _ttoi(strId.GetBuffer());
	m_Selected_Events[id] = strEventName;

	if(!strEventName.IsEmpty())
	{
		// add user defined event to event id-name map
		g_EventIdNameMap.m_UserDefineEvents[id] = strEventName;
	}

	m_List_SelectedEvents.DeleteAllItems();
	AddEventToListView(m_Selected_Events, m_List_SelectedEvents);
}
