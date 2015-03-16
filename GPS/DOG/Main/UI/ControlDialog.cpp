// ControlDialog.cpp : implementation file
//

#include "stdafx.h"
#include "ControlDialog.h"
//#include "CUserSetting.h"
#define AUDIOSTARTINDEX
#define AUDIOENDINDEX

/*
	// Demux event
	m_mapEventID[(CString)"DEMUX_EVENT"] = 10000;
	m_mapEventID[(CString)"DEMUX_MAIN_PATH_UPSTREAM_DATA_RATE_FROM_SECTORBUFF_UPDATE_EVENT"] = 10001;
	m_mapEventID[(CString)"DEMUX_SUB_PATH_UPSTREAM_DATA_RATE_FROM_SECTORBUFF_UPDATE_EVENT"] = 10002;
	m_mapEventID[(CString)"DEMUX_MAIN_PATH_UPSTREAM_DATA_RATE_FROM_IO_UPDATE_EVENT"] = 10003;
	m_mapEventID[(CString)"DEMUX_SUB_PATH_UPSTREAM_DATA_RATE_FROM_IO_UPDATE_EVENT"] = 10004;
	m_mapEventID[(CString)"DEMUX_DOWNSTREAM_VIDEO_DATA_RATE_UPDATE_EVENT"] = 10005;
	m_mapEventID[(CString)"DEMUX_DOWNSTREAM_2ND_VIDE_DATA_RATE_UPDATEO_EVENT"] = 10006;
	m_mapEventID[(CString)"DEMUX_DOWNSTREAM_AUDIO_DATA_RATE_UPDATE_EVENT"] = 10007;
	m_mapEventID[(CString)"DEMUX_DOWNSTREAM_2ND_AUDIO_DATA_RATE_UPDATE_EVENT"] = 10008;
	m_mapEventID[(CString)"DEMUX_DOWNSTREAM_PG_DATA_RATE_UPDATE_EVENT"] = 10009;
	m_mapEventID[(CString)"DEMUX_DOWNSTREAM_IG_DATA_RATE_UPDATE_EVENT"] = 10010;
	m_mapEventID[(CString)"DEMUX_COUNTER_DISCONTINUITY_EVENT"] = 10011;
	m_mapEventID[(CString)"DEMUX_MAIN_PATH_MEDIA_ERROR_EVENT"] = 10012;
	m_mapEventID[(CString)"DEMUX_SUB_PATH_MEDIA_ERROR_EVENT"] = 10013;
	m_mapEventID[(CString)"DEMUX_PTS_GAP_EVENT"] = 10014;
	m_mapEventID[(CString)"DEMUX_SECTOR_BUFFER_EVENT"] = 10015;
	m_mapEventID[(CString)"DEMUX_VIDEO_MB_EVENT"] = 10016;
	m_mapEventID[(CString)"DEMUX_2ND_VIDEO_MB_EVENT"] = 10017;
	m_mapEventID[(CString)"DEMUX_VIDEO_EB_EVENT"] = 10018;
	m_mapEventID[(CString)"DEMUX_2ND_VIDEO_EB_EVENT"] = 10019;
	m_mapEventID[(CString)"DEMUX_AUDIO_EB_EVENT"] = 10020;
	m_mapEventID[(CString)"DEMUX_2ND_AUDIO_EB_EVENT"] = 10021;
	m_mapEventID[(CString)"DEMUX_PG_EB_EVENT"] = 10022;
	m_mapEventID[(CString)"DEMUX_2ND_PG_EB_EVENT"] = 10023;
	m_mapEventID[(CString)"DEMUX_IG_EB_EVENT"] = 10024;
	m_mapEventID[(CString)"DEMUX_SECTOR_INDEX_UPDATE_EVENT"] = 10025;
	m_mapEventID[(CString)"DEMUX_DISPATCH_INDEX_UPDATE_EVENT"] = 10026;
	m_mapEventID[(CString)"DEMUX_VIDEO_PTS_DELTA_EVENT"] = 10027;
	m_mapEventID[(CString)"DEMUX_2ND_VIDEO_PTS_DELTA_EVENT"] = 10028;
	m_mapEventID[(CString)"DEMUX_AUDIO_PTS_DELTA_EVENT"] = 10029;
	m_mapEventID[(CString)"DEMUX_2ND_AUDIO_PTS_DELTA_EVENT"] = 10030;
	m_mapEventID[(CString)"DEMUX_VIDEO_SRC_DELTA_EVENT"] = 10031;
	m_mapEventID[(CString)"DEMUX_2ND_VIDEO_SRC_DELTA_EVENT"] = 10032;
	m_mapEventID[(CString)"DEMUX_AUDIO_SRC_DELTA_EVENT"] = 10033;
	m_mapEventID[(CString)"DEMUX_2ND_AUDIO_SRC_DELTA_EVENT"] = 10034;

	// Video event
	m_mapEventID[(CString)"VIDEO_EVENT"] = 20000;
	m_mapEventID[(CString)"VIDEO_FRAME_EVENT"] = 20001;
	m_mapEventID[(CString)"VIDEO_2ND_FRAME_EVENT"] = 20002;
	m_mapEventID[(CString)"VIDEO_BITRATE_UPDATE_EVENT"] = 20003;
	m_mapEventID[(CString)"VIDEO_2ND_BITRATE_UPDATE_EVENT"] = 20004;
	m_mapEventID[(CString)"VIDEO_DROPPED_FRAMES_UPDATE_EVENT"] = 20005;
	m_mapEventID[(CString)"VIDEO_2ND_DROPPED_FRAMES_UPDATE_EVENT"] = 20006;
	m_mapEventID[(CString)"VIDEO_TOTAL_FRAMES_UPDATE_EVENT"] = 20007;
	m_mapEventID[(CString)"VIDEO_2ND_TOTAL_FRAMES_UPDATE_EVENT"] = 20008;
	m_mapEventID[(CString)"VIDEO_I_FRAMES_UPDATE_EVENT"] = 20009;
	m_mapEventID[(CString)"VIDEO_2ND_I_FRAMES_UPDATE_EVENT"] = 20010;
	m_mapEventID[(CString)"VIDEO_P_FRAMES_UPDATE_EVENT"] = 20011;
	m_mapEventID[(CString)"VIDEO_2ND_P_FRAMES_UPDATE_EVENT"] = 20012;
	m_mapEventID[(CString)"VIDEO_B_FRAMES_UPDATE_EVENT"] = 20013;
	m_mapEventID[(CString)"VIDEO_2ND_B_FRAMES_UPDATE_EVENT"] = 20014;
	m_mapEventID[(CString)"VIDEO_INTERLACED_FRAMES_UPDATE_EVENT"] = 20015;
	m_mapEventID[(CString)"VIDEO_2ND_INTERLACED_FRAMES_UPDATE_EVENT"] = 20016;
	m_mapEventID[(CString)"VIDEO_ENCRYPTED_FRAMES_UPDATE_EVENT"] = 20017;
	m_mapEventID[(CString)"VIDEO_2ND_ENCRYPTED_FRAMES_UPDATE_EVENT"] = 20018;
	m_mapEventID[(CString)"VIDEO_FRAME_RATE_UPDATE_EVENT"] = 20019;
	m_mapEventID[(CString)"VIDEO_2ND_FRAME_RATE_UPDATE_EVENT"] = 20020;
	m_mapEventID[(CString)"VIDEO_PRESENTATION_JITTER_UPDATE_EVENT"] = 20021;
	m_mapEventID[(CString)"VIDEO_2ND_PRESENTATION_JITTER_UPDATE_EVENT"] = 20022;
	m_mapEventID[(CString)"VIDEO_DECODE_MSEC_UPDATE_EVENT"] = 20023;
	m_mapEventID[(CString)"VIDEO_2ND_DECODE_MSEC_UPDATE_EVENT"] = 20024;

	// Display event
	m_mapEventID[(CString)"DISPLAY_EVENT"] = 30000;
	m_mapEventID[(CString)"DISPLAY_AVSYNC_UPDATE_EVENT"] = 30001;
	m_mapEventID[(CString)"DISPLAY_FRAME_QUEUE_SIZE_UPDATE_EVENT"] = 30002;
	m_mapEventID[(CString)"DISPLAY_2ND_FRAME_QUEUE_SIZE_UPDATE_EVENT"] = 30003;
	m_mapEventID[(CString)"DISPLAY_FRAME_DROP_EVENT"] = 30004;
	m_mapEventID[(CString)"DISPLAY_2ND_FRAME_DROP_EVENT"] = 30005;
	m_mapEventID[(CString)"DISPLAY_FRAME_PTS_DELTA_UPDATE_EVENT"] = 30006;
	m_mapEventID[(CString)"DISPLAY_2ND_FRAME_PTS_DELTA_UPDATE_EVENT"] = 30007;
	m_mapEventID[(CString)"DISPLAY_PREPARE_FRAME_COST_UPDATE_EVENT"] = 30008;
	m_mapEventID[(CString)"DISPLAY_2ND_PREPARE_FRAME_COST_UPDATE_EVENT"] = 30009;
	m_mapEventID[(CString)"DISPLAY_DISPLAY_FRAME_COST_UPDATE_EVENT"] = 30010;
	m_mapEventID[(CString)"DISPLAY_DELTA_UPDATE_EVENT"] = 30011;
	m_mapEventID[(CString)"DISPLAY_JITTER_EVENT"] = 30012;

	// Audio event
	m_mapEventID[(CString)"AUDIO_EVENT"] = 40000;
	m_mapEventID[(CString)"AUDIO_DECODING_1ST_DATA_EVENT"] = 40001;
	m_mapEventID[(CString)"AUDIO_DECODING_2ND_DATA_EVENT"] = 40002;
	m_mapEventID[(CString)"AUDIO_DECODING_METADATA_EVENT"] = 40003;
	m_mapEventID[(CString)"AUDIO_DECODING_DATA_IN_PTS_DELTA_UPDATE_EVENT"] = 40004;
	m_mapEventID[(CString)"AUDIO_DECODING_DATA_OUT_PTS_DELTA_UPDATE_EVENT"] = 40005;
	m_mapEventID[(CString)"AUDIO_BACKEND_1ST_BUFFER_EVENT"] = 40006;
	m_mapEventID[(CString)"AUDIO_BACKEND_2ND_BUFFER_EVENT"] = 40007;
	m_mapEventID[(CString)"AUDIO_BACKEND_METADATA_BUFFER_EVENT"] = 40008;
	m_mapEventID[(CString)"AUDIO_RENDER_BUFFER_EVENT"] = 40009;
	m_mapEventID[(CString)"AUDIO_AVSYNC_PTS_DELTA_UPDATE_EVENT"] = 40010;
	m_mapEventID[(CString)"AUDIO_1ST_STREAM_INFO_UPDATE_EVENT"] = 40011;
	m_mapEventID[(CString)"AUDIO_2ND_STREAM_INFO_UPDATE_EVENT"] = 40012;
	m_mapEventID[(CString)"AUDIO_OUTPUT_UPDATE_EVENT"] = 40013;
	m_mapEventID[(CString)"AUDIO_DEVICE_CAPABILITY_UPDATE_EVENT"] = 40014;
	*/
// CControlDialog dialog

IMPLEMENT_DYNAMIC(CControlDialog, CDialog)

CControlDialog::CControlDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CControlDialog::IDD, pParent)
{
}

CControlDialog::~CControlDialog()
{
}

void CControlDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_AVAILABLELIST, m_lstbxAvailable);
	DDX_Control(pDX, IDC_ACTIVELIST, m_lstbxActive);
}


BEGIN_MESSAGE_MAP(CControlDialog, CDialog)
	ON_BN_CLICKED(IDC_ADDBUTTON, &CControlDialog::OnBnClickedAddbutton)
	ON_BN_CLICKED(IDC_REMOVEBUTTON, &CControlDialog::OnBnClickedRemovebutton)
	ON_BN_CLICKED(IDC_RESETBUTTON, &CControlDialog::OnBnClickedResetbutton)
	ON_BN_CLICKED(IDOK, &CControlDialog::OnBnClickedOk)
	ON_BN_CLICKED(IDC_CHECKAUDIO, &CControlDialog::OnBnClickedCheckaudio)
	ON_BN_CLICKED(IDC_CHECKVIDEO, &CControlDialog::OnBnClickedCheckvideo)
	ON_BN_CLICKED(IDC_CHECKDEMUX, &CControlDialog::OnBnClickedCheckdemux)
	ON_BN_CLICKED(IDC_CHECKDISPLAY, &CControlDialog::OnBnClickedCheckdisplay)
	ON_BN_CLICKED(IDCANCEL, &CControlDialog::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTONADDEVENT, &CControlDialog::OnBnClickedButtonAddEvent)
	ON_BN_CLICKED(IDC_CLEARBUTTON, &CControlDialog::OnBnClickedClearbutton)
END_MESSAGE_MAP()

// CControlDialog message handlers
void CControlDialog::LoadDefaultItems()
{
	m_lstbxAvailable.ResetContent();
	m_lstbxActive.ResetContent();

	if (m_mapEventID.size() > 0)
	{
		std::map<CString, int>::iterator iter = m_mapEventID.begin();

		while (iter != m_mapEventID.end())
		{
			m_lstbxAvailable.AddString((*iter).first);
			iter++;
		}
	}

	// copy exist windows to active list
	std::map<CString, int>::iterator it = m_mapSelectedEventID.begin();

	for (it; it != m_mapSelectedEventID.end(); ++it)
	{
		m_lstbxActive.AddString(it->first);
	}

	UpdateListBox();
}

// remove exist item from available listbox, and add to active listbox
void CControlDialog::UpdateListBox()
{
	if (m_mapSelectedEventID.size() < 0)
		return;

	std::map<CString, int>::iterator it = m_mapSelectedEventID.begin();

	for (it; it != m_mapSelectedEventID.end(); ++it)
	{
		int nIndex = m_lstbxAvailable.FindString(0, it->first);

		if (nIndex >= 0)
			m_lstbxAvailable.DeleteString(nIndex);
	}
}

void CControlDialog::InitData()
{
	// not support text dump in 1st stage
	/*
	m_mapEventID[(CString)"DEBUG_MESSAGE_EVENT"] = 0;
	m_mapEventID[(CString)"ERROR_MESSAGE_EVENT"] = 1;
	m_mapEventID[(CString)"FATAL_MESSAGE_EVENT"] = 2;
	*/

	std::vector<struct UI::DogEvent> vecDogEvent = ((MyApp*)AfxGetApp())->GetSettingVector();
	std::vector<struct UI::DogEvent>::iterator it = vecDogEvent.begin();

	for (it; it != vecDogEvent.end(); ++it)
	{
		m_mapSelectedEventID[it->pEventName] = it->dogId;
	}
}

BOOL CControlDialog::OnInitDialog()
{
	m_lstbxAvailable.Attach(GetDlgItem(IDC_AVAILABLELIST)->m_hWnd);
	m_lstbxActive.Attach(GetDlgItem(IDC_ACTIVELIST)->m_hWnd);

	InitData();

	LoadDefaultItems();

	return FALSE;
}

void CControlDialog::OnBnClickedAddbutton()
{
	// TODO: Add your control notification handler code here

	// single select
	/*int nSelectIndex = m_lstbxAvailable.GetCurSel();

	if (nSelectIndex >=0)
	{
		CString str;
		m_lstbxAvailable.GetText(nSelectIndex, str);
		m_mapSelectedEventID[str] = m_mapEventID[str];
		m_lstbxActive.AddString(str);
		m_lstbxAvailable.DeleteString(nSelectIndex);
	}*/

	// multiple select
	CArray<int,int> aryListBoxSel;
	int nSelectCount = m_lstbxAvailable.GetSelCount();
	aryListBoxSel.SetSize(nSelectCount);
	m_lstbxAvailable.GetSelItems(nSelectCount, aryListBoxSel.GetData()); 

	if (nSelectCount > 0)
	{
		int nSelectIndex = nSelectCount - 1;
		CString str;

		for (int i = nSelectIndex; i >= 0; i--)
		{
			m_lstbxAvailable.GetText(aryListBoxSel.GetAt(i), str);
			m_mapSelectedEventID[str] = m_mapEventID[str];
			m_lstbxActive.AddString(str);
			m_lstbxAvailable.DeleteString(aryListBoxSel.GetAt(i));
		}
	}
}

void CControlDialog::OnBnClickedRemovebutton()
{
	// TODO: Add your control notification handler code here

	// single select
	/*int nSelectIndex = m_lstbxActive.GetCurSel();

	if (nSelectIndex >=0)
	{
		CString str;
		m_lstbxActive.GetText(nSelectIndex, str);

		// only add to available listbox when the group is checked
		if (m_mapEventID.count(str) > 0)
			m_lstbxAvailable.AddString(str);

		m_lstbxActive.DeleteString(nSelectIndex);
		m_mapSelectedEventID.erase(str);
	}*/

	// multiple select
	CArray<int,int> aryListBoxSel;
	int nSelectCount = m_lstbxActive.GetSelCount();
	aryListBoxSel.SetSize(nSelectCount);
	m_lstbxActive.GetSelItems(nSelectCount, aryListBoxSel.GetData()); 

	if (nSelectCount > 0)
	{
		int nSelectIndex = nSelectCount - 1;
		CString str;

		for (int i = nSelectIndex; i >= 0; i--)
		{
			m_lstbxActive.GetText(aryListBoxSel.GetAt(i), str);

			if (m_mapEventID.count(str) > 0)
				m_lstbxAvailable.AddString(str);

			m_lstbxActive.DeleteString(aryListBoxSel.GetAt(i));
			m_mapSelectedEventID.erase(str);
		}
	}
}

void CControlDialog::OnBnClickedResetbutton()
{
	// TODO: Add your control notification handler code here
	m_lstbxAvailable.ResetContent();
	m_lstbxActive.ResetContent();
	m_mapSelectedEventID.clear();
	InitData();
	LoadDefaultItems();
}

void CControlDialog::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	//Perf window to exist
	int nSelectNum = m_lstbxActive.GetCount();

	for (int i = 0; i < nSelectNum; i++)
	{
		CString strEventName;
		m_lstbxActive.GetText(i, strEventName);

		int nEventId = m_mapSelectedEventID[strEventName];
		UI::DogEvent dogEvent;
		dogEvent.dogId = nEventId;
#ifdef  UNICODE
		int len = strEventName.GetLength() * 2;
#else
		int len = strEventName.GetLength();
#endif
		dogEvent.pEventName = new TCHAR[len + 1];
		_tcscpy_s(dogEvent.pEventName, len, strEventName);
		((MyApp*)AfxGetApp())->AddToSettingVector(dogEvent);
	}
	
	CDialog::OnOK();
	m_lstbxAvailable.Detach();
	m_lstbxActive.Detach();
}

void CControlDialog::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	CDialog::OnCancel();
	m_lstbxAvailable.Detach();
	m_lstbxActive.Detach();
}

void RemoveStringFromListbox(CListBox* plistBox, CString &strArray)
{
}

void CControlDialog::OnBnClickedCheckaudio()
{
	// TODO: Add your control notification handler code here
	CButton *pCheckBox = (CButton*)GetDlgItem(IDC_CHECKAUDIO);

	if (!pCheckBox)
		return;
	
	if (pCheckBox->GetCheck())
	{
		m_mapEventID[(CString)"AUDIO_EVENT"] = 40000;
		m_mapEventID[(CString)"AUDIO_DECODING_1ST_DATA_EVENT"] = 40001;
		m_mapEventID[(CString)"AUDIO_DECODING_2ND_DATA_EVENT"] = 40002;
		m_mapEventID[(CString)"AUDIO_DECODING_METADATA_EVENT"] = 40003;
		m_mapEventID[(CString)"AUDIO_DECODING_DATA_IN_PTS_DELTA_UPDATE_EVENT"] = 40004;
		m_mapEventID[(CString)"AUDIO_DECODING_DATA_OUT_PTS_DELTA_UPDATE_EVENT"] = 40005;
		m_mapEventID[(CString)"AUDIO_BACKEND_1ST_BUFFER_EVENT"] = 40006;
		m_mapEventID[(CString)"AUDIO_BACKEND_2ND_BUFFER_EVENT"] = 40007;
		m_mapEventID[(CString)"AUDIO_BACKEND_METADATA_BUFFER_EVENT"] = 40008;
		m_mapEventID[(CString)"AUDIO_RENDER_BUFFER_EVENT"] = 40009;
		m_mapEventID[(CString)"AUDIO_AVSYNC_PTS_DELTA_UPDATE_EVENT"] = 40010;
		m_mapEventID[(CString)"AUDIO_1ST_STREAM_INFO_UPDATE_EVENT"] = 40011;
		m_mapEventID[(CString)"AUDIO_2ND_STREAM_INFO_UPDATE_EVENT"] = 40012;
		m_mapEventID[(CString)"AUDIO_OUTPUT_UPDATE_EVENT"] = 40013;
		m_mapEventID[(CString)"AUDIO_DEVICE_CAPABILITY_UPDATE_EVENT"] = 40014;
	}
	else
	{
		m_mapEventID.erase((CString)"AUDIO_EVENT");
		m_mapEventID.erase((CString)"AUDIO_DECODING_1ST_DATA_EVENT");
		m_mapEventID.erase((CString)"AUDIO_DECODING_2ND_DATA_EVENT");
		m_mapEventID.erase((CString)"AUDIO_DECODING_METADATA_EVENT");
		m_mapEventID.erase((CString)"AUDIO_DECODING_DATA_IN_PTS_DELTA_UPDATE_EVENT");
		m_mapEventID.erase((CString)"AUDIO_DECODING_DATA_OUT_PTS_DELTA_UPDATE_EVENT");
		m_mapEventID.erase((CString)"AUDIO_BACKEND_1ST_BUFFER_EVENT");
		m_mapEventID.erase((CString)"AUDIO_BACKEND_2ND_BUFFER_EVENT");
		m_mapEventID.erase((CString)"AUDIO_BACKEND_METADATA_BUFFER_EVENT");
		m_mapEventID.erase((CString)"AUDIO_RENDER_BUFFER_EVENT");
		m_mapEventID.erase((CString)"AUDIO_AVSYNC_PTS_DELTA_UPDATE_EVENT");
		m_mapEventID.erase((CString)"AUDIO_1ST_STREAM_INFO_UPDATE_EVENT");
		m_mapEventID.erase((CString)"AUDIO_2ND_STREAM_INFO_UPDATE_EVENT");
		m_mapEventID.erase((CString)"AUDIO_OUTPUT_UPDATE_EVENT");
		m_mapEventID.erase((CString)"AUDIO_DEVICE_CAPABILITY_UPDATE_EVENT");
	}

	LoadDefaultItems();
}

void CControlDialog::OnBnClickedCheckvideo()
{
	// TODO: Add your control notification handler code here
	CButton *pCheckBox = (CButton*)GetDlgItem(IDC_CHECKVIDEO);

	if (!pCheckBox)
		return;

	if (pCheckBox->GetCheck())
	{
		m_mapEventID[(CString)"VIDEO_EVENT"] = 20000;
		m_mapEventID[(CString)"VIDEO_FRAME_EVENT"] = 20001;
		m_mapEventID[(CString)"VIDEO_2ND_FRAME_EVENT"] = 20002;
		m_mapEventID[(CString)"VIDEO_BITRATE_UPDATE_EVENT"] = 20003;
		m_mapEventID[(CString)"VIDEO_2ND_BITRATE_UPDATE_EVENT"] = 20004;
		m_mapEventID[(CString)"VIDEO_DROPPED_FRAMES_UPDATE_EVENT"] = 20005;
		m_mapEventID[(CString)"VIDEO_2ND_DROPPED_FRAMES_UPDATE_EVENT"] = 20006;
		m_mapEventID[(CString)"VIDEO_TOTAL_FRAMES_UPDATE_EVENT"] = 20007;
		m_mapEventID[(CString)"VIDEO_2ND_TOTAL_FRAMES_UPDATE_EVENT"] = 20008;
		m_mapEventID[(CString)"VIDEO_I_FRAMES_UPDATE_EVENT"] = 20009;
		m_mapEventID[(CString)"VIDEO_2ND_I_FRAMES_UPDATE_EVENT"] = 20010;
		m_mapEventID[(CString)"VIDEO_P_FRAMES_UPDATE_EVENT"] = 20011;
		m_mapEventID[(CString)"VIDEO_2ND_P_FRAMES_UPDATE_EVENT"] = 20012;
		m_mapEventID[(CString)"VIDEO_B_FRAMES_UPDATE_EVENT"] = 20013;
		m_mapEventID[(CString)"VIDEO_2ND_B_FRAMES_UPDATE_EVENT"] = 20014;
		m_mapEventID[(CString)"VIDEO_INTERLACED_FRAMES_UPDATE_EVENT"] = 20015;
		m_mapEventID[(CString)"VIDEO_2ND_INTERLACED_FRAMES_UPDATE_EVENT"] = 20016;
		m_mapEventID[(CString)"VIDEO_ENCRYPTED_FRAMES_UPDATE_EVENT"] = 20017;
		m_mapEventID[(CString)"VIDEO_2ND_ENCRYPTED_FRAMES_UPDATE_EVENT"] = 20018;
		m_mapEventID[(CString)"VIDEO_FRAME_RATE_UPDATE_EVENT"] = 20019;
		m_mapEventID[(CString)"VIDEO_2ND_FRAME_RATE_UPDATE_EVENT"] = 20020;
		m_mapEventID[(CString)"VIDEO_PRESENTATION_JITTER_UPDATE_EVENT"] = 20021;
		m_mapEventID[(CString)"VIDEO_2ND_PRESENTATION_JITTER_UPDATE_EVENT"] = 20022;
		m_mapEventID[(CString)"VIDEO_DECODE_MSEC_UPDATE_EVENT"] = 20023;
		m_mapEventID[(CString)"VIDEO_2ND_DECODE_MSEC_UPDATE_EVENT"] = 20024;
	}
	else
	{
		m_mapEventID.erase((CString)"VIDEO_EVENT");
		m_mapEventID.erase((CString)"VIDEO_FRAME_EVENT");
		m_mapEventID.erase((CString)"VIDEO_2ND_FRAME_EVENT");
		m_mapEventID.erase((CString)"VIDEO_BITRATE_UPDATE_EVENT");
		m_mapEventID.erase((CString)"VIDEO_2ND_BITRATE_UPDATE_EVENT");
		m_mapEventID.erase((CString)"VIDEO_DROPPED_FRAMES_UPDATE_EVENT");
		m_mapEventID.erase((CString)"VIDEO_2ND_DROPPED_FRAMES_UPDATE_EVENT");
		m_mapEventID.erase((CString)"VIDEO_TOTAL_FRAMES_UPDATE_EVENT");
		m_mapEventID.erase((CString)"VIDEO_2ND_TOTAL_FRAMES_UPDATE_EVENT");
		m_mapEventID.erase((CString)"VIDEO_I_FRAMES_UPDATE_EVENT");
		m_mapEventID.erase((CString)"VIDEO_2ND_I_FRAMES_UPDATE_EVENT");
		m_mapEventID.erase((CString)"VIDEO_P_FRAMES_UPDATE_EVENT");
		m_mapEventID.erase((CString)"VIDEO_2ND_P_FRAMES_UPDATE_EVENT");
		m_mapEventID.erase((CString)"VIDEO_B_FRAMES_UPDATE_EVENT");
		m_mapEventID.erase((CString)"VIDEO_2ND_B_FRAMES_UPDATE_EVENT");
		m_mapEventID.erase((CString)"VIDEO_INTERLACED_FRAMES_UPDATE_EVENT");
		m_mapEventID.erase((CString)"VIDEO_2ND_INTERLACED_FRAMES_UPDATE_EVENT");
		m_mapEventID.erase((CString)"VIDEO_ENCRYPTED_FRAMES_UPDATE_EVENT");
		m_mapEventID.erase((CString)"VIDEO_2ND_ENCRYPTED_FRAMES_UPDATE_EVENT");
		m_mapEventID.erase((CString)"VIDEO_FRAME_RATE_UPDATE_EVENT");
		m_mapEventID.erase((CString)"VIDEO_2ND_FRAME_RATE_UPDATE_EVENT");
		m_mapEventID.erase((CString)"VIDEO_PRESENTATION_JITTER_UPDATE_EVENT");
		m_mapEventID.erase((CString)"VIDEO_2ND_PRESENTATION_JITTER_UPDATE_EVENT");
		m_mapEventID.erase((CString)"VIDEO_DECODE_MSEC_UPDATE_EVENT");
		m_mapEventID.erase((CString)"VIDEO_2ND_DECODE_MSEC_UPDATE_EVENT");
	}

	LoadDefaultItems();
}

void CControlDialog::OnBnClickedCheckdemux()
{
	// TODO: Add your control notification handler code here
	CButton *pCheckBox = (CButton*)GetDlgItem(IDC_CHECKDEMUX);

	if (!pCheckBox)
		return;

	if (pCheckBox->GetCheck())
	{
		m_mapEventID[(CString)"DEMUX_EVENT"] = 10000;
		m_mapEventID[(CString)"DEMUX_MAIN_PATH_UPSTREAM_DATA_RATE_FROM_SECTORBUFF_UPDATE_EVENT"] = 10001;
		m_mapEventID[(CString)"DEMUX_SUB_PATH_UPSTREAM_DATA_RATE_FROM_SECTORBUFF_UPDATE_EVENT"] = 10002;
		m_mapEventID[(CString)"DEMUX_MAIN_PATH_UPSTREAM_DATA_RATE_FROM_IO_UPDATE_EVENT"] = 10003;
		m_mapEventID[(CString)"DEMUX_SUB_PATH_UPSTREAM_DATA_RATE_FROM_IO_UPDATE_EVENT"] = 10004;
		m_mapEventID[(CString)"DEMUX_DOWNSTREAM_VIDEO_DATA_RATE_UPDATE_EVENT"] = 10005;
		m_mapEventID[(CString)"DEMUX_DOWNSTREAM_2ND_VIDE_DATA_RATE_UPDATEO_EVENT"] = 10006;
		m_mapEventID[(CString)"DEMUX_DOWNSTREAM_AUDIO_DATA_RATE_UPDATE_EVENT"] = 10007;
		m_mapEventID[(CString)"DEMUX_DOWNSTREAM_2ND_AUDIO_DATA_RATE_UPDATE_EVENT"] = 10008;
		m_mapEventID[(CString)"DEMUX_DOWNSTREAM_PG_DATA_RATE_UPDATE_EVENT"] = 10009;
		m_mapEventID[(CString)"DEMUX_DOWNSTREAM_IG_DATA_RATE_UPDATE_EVENT"] = 10010;
		m_mapEventID[(CString)"DEMUX_COUNTER_DISCONTINUITY_EVENT"] = 10011;
		m_mapEventID[(CString)"DEMUX_MAIN_PATH_MEDIA_ERROR_EVENT"] = 10012;
		m_mapEventID[(CString)"DEMUX_SUB_PATH_MEDIA_ERROR_EVENT"] = 10013;
		m_mapEventID[(CString)"DEMUX_PTS_GAP_EVENT"] = 10014;
		m_mapEventID[(CString)"DEMUX_SECTOR_BUFFER_EVENT"] = 10015;
		m_mapEventID[(CString)"DEMUX_VIDEO_MB_EVENT"] = 10016;
		m_mapEventID[(CString)"DEMUX_2ND_VIDEO_MB_EVENT"] = 10017;
		m_mapEventID[(CString)"DEMUX_VIDEO_EB_EVENT"] = 10018;
		m_mapEventID[(CString)"DEMUX_2ND_VIDEO_EB_EVENT"] = 10019;
		m_mapEventID[(CString)"DEMUX_AUDIO_EB_EVENT"] = 10020;
		m_mapEventID[(CString)"DEMUX_2ND_AUDIO_EB_EVENT"] = 10021;
		m_mapEventID[(CString)"DEMUX_PG_EB_EVENT"] = 10022;
		m_mapEventID[(CString)"DEMUX_2ND_PG_EB_EVENT"] = 10023;
		m_mapEventID[(CString)"DEMUX_IG_EB_EVENT"] = 10024;
		m_mapEventID[(CString)"DEMUX_SECTOR_INDEX_UPDATE_EVENT"] = 10025;
		m_mapEventID[(CString)"DEMUX_DISPATCH_INDEX_UPDATE_EVENT"] = 10026;
		m_mapEventID[(CString)"DEMUX_VIDEO_PTS_DELTA_EVENT"] = 10027;
		m_mapEventID[(CString)"DEMUX_2ND_VIDEO_PTS_DELTA_EVENT"] = 10028;
		m_mapEventID[(CString)"DEMUX_AUDIO_PTS_DELTA_EVENT"] = 10029;
		m_mapEventID[(CString)"DEMUX_2ND_AUDIO_PTS_DELTA_EVENT"] = 10030;
		m_mapEventID[(CString)"DEMUX_VIDEO_SRC_DELTA_EVENT"] = 10031;
		m_mapEventID[(CString)"DEMUX_2ND_VIDEO_SRC_DELTA_EVENT"] = 10032;
		m_mapEventID[(CString)"DEMUX_AUDIO_SRC_DELTA_EVENT"] = 10033;
		m_mapEventID[(CString)"DEMUX_2ND_AUDIO_SRC_DELTA_EVENT"] = 10034;
	}
	else
	{
		m_mapEventID.erase((CString)"DEMUX_EVENT");
		m_mapEventID.erase((CString)"DEMUX_MAIN_PATH_UPSTREAM_DATA_RATE_FROM_SECTORBUFF_UPDATE_EVENT");
		m_mapEventID.erase((CString)"DEMUX_SUB_PATH_UPSTREAM_DATA_RATE_FROM_SECTORBUFF_UPDATE_EVENT");
		m_mapEventID.erase((CString)"DEMUX_MAIN_PATH_UPSTREAM_DATA_RATE_FROM_IO_UPDATE_EVENT");
		m_mapEventID.erase((CString)"DEMUX_SUB_PATH_UPSTREAM_DATA_RATE_FROM_IO_UPDATE_EVENT");
		m_mapEventID.erase((CString)"DEMUX_DOWNSTREAM_VIDEO_DATA_RATE_UPDATE_EVENT");
		m_mapEventID.erase((CString)"DEMUX_DOWNSTREAM_2ND_VIDE_DATA_RATE_UPDATEO_EVENT");
		m_mapEventID.erase((CString)"DEMUX_DOWNSTREAM_AUDIO_DATA_RATE_UPDATE_EVENT");
		m_mapEventID.erase((CString)"DEMUX_DOWNSTREAM_2ND_AUDIO_DATA_RATE_UPDATE_EVENT");
		m_mapEventID.erase((CString)"DEMUX_DOWNSTREAM_PG_DATA_RATE_UPDATE_EVENT");
		m_mapEventID.erase((CString)"DEMUX_DOWNSTREAM_IG_DATA_RATE_UPDATE_EVENT");
		m_mapEventID.erase((CString)"DEMUX_COUNTER_DISCONTINUITY_EVENT");
		m_mapEventID.erase((CString)"DEMUX_MAIN_PATH_MEDIA_ERROR_EVENT");
		m_mapEventID.erase((CString)"DEMUX_SUB_PATH_MEDIA_ERROR_EVENT");
		m_mapEventID.erase((CString)"DEMUX_PTS_GAP_EVENT");
		m_mapEventID.erase((CString)"DEMUX_SECTOR_BUFFER_EVENT");
		m_mapEventID.erase((CString)"DEMUX_VIDEO_MB_EVENT");
		m_mapEventID.erase((CString)"DEMUX_2ND_VIDEO_MB_EVENT");
		m_mapEventID.erase((CString)"DEMUX_VIDEO_EB_EVENT");
		m_mapEventID.erase((CString)"DEMUX_2ND_VIDEO_EB_EVENT");
		m_mapEventID.erase((CString)"DEMUX_AUDIO_EB_EVENT");
		m_mapEventID.erase((CString)"DEMUX_2ND_AUDIO_EB_EVENT");
		m_mapEventID.erase((CString)"DEMUX_PG_EB_EVENT");
		m_mapEventID.erase((CString)"DEMUX_2ND_PG_EB_EVENT");
		m_mapEventID.erase((CString)"DEMUX_IG_EB_EVENT");
		m_mapEventID.erase((CString)"DEMUX_SECTOR_INDEX_UPDATE_EVENT");
		m_mapEventID.erase((CString)"DEMUX_DISPATCH_INDEX_UPDATE_EVENT");
		m_mapEventID.erase((CString)"DEMUX_VIDEO_PTS_DELTA_EVENT");
		m_mapEventID.erase((CString)"DEMUX_2ND_VIDEO_PTS_DELTA_EVENT");
		m_mapEventID.erase((CString)"DEMUX_AUDIO_PTS_DELTA_EVENT");
		m_mapEventID.erase((CString)"DEMUX_2ND_AUDIO_PTS_DELTA_EVENT");
		m_mapEventID.erase((CString)"DEMUX_VIDEO_SRC_DELTA_EVENT");
		m_mapEventID.erase((CString)"DEMUX_2ND_VIDEO_SRC_DELTA_EVENT");
		m_mapEventID.erase((CString)"DEMUX_AUDIO_SRC_DELTA_EVENT");
		m_mapEventID.erase((CString)"DEMUX_2ND_AUDIO_SRC_DELTA_EVENT");
	}

	LoadDefaultItems();
}

void CControlDialog::OnBnClickedCheckdisplay()
{
	// TODO: Add your control notification handler code here
	CButton *pCheckBox = (CButton*)GetDlgItem(IDC_CHECKDISPLAY);

	if (!pCheckBox)
		return;

	if (pCheckBox->GetCheck())
	{
		m_mapEventID[(CString)"DISPLAY_EVENT"] = 30000;
		m_mapEventID[(CString)"DISPLAY_AVSYNC_UPDATE_EVENT"] = 30001;
		m_mapEventID[(CString)"DISPLAY_FRAME_QUEUE_SIZE_UPDATE_EVENT"] = 30002;
		m_mapEventID[(CString)"DISPLAY_2ND_FRAME_QUEUE_SIZE_UPDATE_EVENT"] = 30003;
		m_mapEventID[(CString)"DISPLAY_FRAME_DROP_EVENT"] = 30004;
		m_mapEventID[(CString)"DISPLAY_2ND_FRAME_DROP_EVENT"] = 30005;
		m_mapEventID[(CString)"DISPLAY_FRAME_PTS_DELTA_UPDATE_EVENT"] = 30006;
		m_mapEventID[(CString)"DISPLAY_2ND_FRAME_PTS_DELTA_UPDATE_EVENT"] = 30007;
		m_mapEventID[(CString)"DISPLAY_PREPARE_FRAME_COST_UPDATE_EVENT"] = 30008;
		m_mapEventID[(CString)"DISPLAY_2ND_PREPARE_FRAME_COST_UPDATE_EVENT"] = 30009;
		m_mapEventID[(CString)"DISPLAY_DISPLAY_FRAME_COST_UPDATE_EVENT"] = 30010;
		m_mapEventID[(CString)"DISPLAY_DELTA_UPDATE_EVENT"] = 30011;
		m_mapEventID[(CString)"DISPLAY_JITTER_EVENT"] = 30012;
	}
	else
	{
		m_mapEventID.erase((CString)"DISPLAY_EVENT");
		m_mapEventID.erase((CString)"DISPLAY_AVSYNC_UPDATE_EVENT");
		m_mapEventID.erase((CString)"DISPLAY_FRAME_QUEUE_SIZE_UPDATE_EVENT");
		m_mapEventID.erase((CString)"DISPLAY_2ND_FRAME_QUEUE_SIZE_UPDATE_EVENT");
		m_mapEventID.erase((CString)"DISPLAY_FRAME_DROP_EVENT");
		m_mapEventID.erase((CString)"DISPLAY_2ND_FRAME_DROP_EVENT");
		m_mapEventID.erase((CString)"DISPLAY_FRAME_PTS_DELTA_UPDATE_EVENT");
		m_mapEventID.erase((CString)"DISPLAY_2ND_FRAME_PTS_DELTA_UPDATE_EVENT");
		m_mapEventID.erase((CString)"DISPLAY_PREPARE_FRAME_COST_UPDATE_EVENT");
		m_mapEventID.erase((CString)"DISPLAY_2ND_PREPARE_FRAME_COST_UPDATE_EVENT");
		m_mapEventID.erase((CString)"DISPLAY_DISPLAY_FRAME_COST_UPDATE_EVENT");
		m_mapEventID.erase((CString)"DISPLAY_DELTA_UPDATE_EVENT");
		m_mapEventID.erase((CString)"DISPLAY_JITTER_EVENT");
	}

	LoadDefaultItems();
}


void CControlDialog::OnBnClickedButtonAddEvent()
{
	// TODO: Add your control notification handler code here
	CString strEventName, strEventId;
	
	GetDlgItem(IDC_EDITCUSTOMIZEEVENT)->GetWindowText(strEventName);
	GetDlgItem(IDC_EDITCUSTOMIZEEVENTID)->GetWindowText(strEventId);

	int nEventId = _ttoi(strEventId);

	if (strEventName.GetLength() > 0 && nEventId > 0)
	{
		if (m_mapEventID.count(strEventName) == 0)
		{
			m_mapEventID[strEventName] = nEventId;
			LoadDefaultItems();
		}
	}
}

void CControlDialog::OnBnClickedClearbutton()
{
	// TODO: Add your control notification handler code here
	m_mapSelectedEventID.clear();
	LoadDefaultItems();
}
