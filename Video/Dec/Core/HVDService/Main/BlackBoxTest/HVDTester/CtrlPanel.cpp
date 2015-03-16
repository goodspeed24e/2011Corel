// CtrlPanel.cpp : implementation file
//

#include "stdafx.h"
#include "HVDServiceTester.h"
#include "CtrlPanel.h"
#include "HVDTestCenter.h"


// CCtrlPanel

ObjectGenInfo CtrlPanelInfo = { ID_CTRLPANEL, NULL, 0, 0, 500, 550, WS_POPUPWINDOW|WS_CAPTION, _T("Control Panel")};
ObjectGenInfo DXVA1GroupInfo = { ID_DXVA1_GROUP, NULL,  20,   5, 220, 280, WS_CHILD|WS_VISIBLE|BS_GROUPBOX, _T("DXVA1")};
ObjectGenInfo DXVA2GroupInfo = { ID_DXVA2_GROUP, NULL, 260,   5, 220, 280, WS_CHILD|WS_VISIBLE|BS_GROUPBOX, _T("DXVA2")};
ObjectGenInfo CodecGroupInfo = { ID_CODEC_GROUP, NULL,  20, 300, 460,  80, WS_CHILD|WS_VISIBLE|BS_GROUPBOX, _T("Codec")};

ObjectGenInfo DXVA1ItemTable[] = 
{
	{ ID_DXVA1_OVERLAY,			TEST_ITEM_DXVA1_OVERLAY, 30,  30, 200, 25, WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX, _T("Overlay Mixer")},
	{ ID_DXVA1_VMR7,			TEST_ITEM_DXVA1_VMR7, 30,  60, 200, 25, WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX, _T("VMR-7")},
	{ ID_DXVA1_VMR9WINDOWLESS,	TEST_ITEM_DXVA1_VMR9WINDOWLESS, 30,  90, 200, 25, WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX, _T("VMR-9 Windowless")},
	{ ID_DXVA1_VMR9CUSTOM,		TEST_ITEM_DXVA1_VMR9CUSTOM, 30, 120, 200, 25, WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX, _T("VMR-9 Custom")},
	{ ID_DXVA1_EXTERNAL,		TEST_ITEM_DXVA1_EXTERNAL, 30, 150, 200, 25, WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX, _T("External Filter")},
};

ObjectGenInfo DXVA2ItemTable[] = 
{
	{ ID_DXVA2_D3D,	TEST_ITEM_DXVA2_D3D, 270, 30, 200, 25, WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX, _T("Direct3D")},
	{ ID_DXVA2_DSHOW, TEST_ITEM_DXVA2_DSHOW, 270, 60, 200, 25, WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX, _T("DirectShow")},
};

ObjectGenInfo CodecItemTable[] = 
{
	{ ID_CODEC_MPEG2, TEST_CODEC_MPEG2,	30, 325, 120, 25, WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX, _T("MPEG-2")},
	{ ID_CODEC_H264, TEST_CODEC_H264, 170, 325, 120, 25, WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX, _T("H.264")},
	{ ID_CODEC_VC1,	TEST_CODEC_VC1, 310, 325, 120, 25, WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX, _T("VC-1")},
	{ ID_CODEC_MPEG4, TEST_CODEC_MPEG4, 30, 355, 120, 25, WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX, _T("MPEG-4")},
};

ObjectGenInfo BtnTestInfo = { ID_BTN_TEST, NULL, 190, 460, 120, 30, WS_CHILD|WS_VISIBLE, _T("Launch Test")};

IMPLEMENT_DYNAMIC(CCtrlPanel, CWnd)

BEGIN_MESSAGE_MAP(CCtrlPanel, CWnd)
	ON_WM_CLOSE()
	ON_BN_CLICKED(ID_BTN_TEST, &CCtrlPanel::OnBtnTestClicked)
END_MESSAGE_MAP()



CCtrlPanel::CCtrlPanel()
{
	m_pParentWnd = NULL;
	LOGFONT logfont;
	ZeroMemory(&logfont, sizeof(logfont));
	logfont.lfCharSet = 0;
	logfont.lfHeight = -13;
	logfont.lfWeight = 400;
	_tcscpy_s(logfont.lfFaceName, 32, _T("MS Shell Dlg"));

	m_Font.CreateFontIndirect(&logfont);
}

CCtrlPanel::~CCtrlPanel()
{
	m_Font.DeleteObject();
}

BOOL CCtrlPanel::Create(CWnd *pParentWnd, CHVDTestCenter *pTestCenter)
{
	if (!pTestCenter)
		return FALSE;

	m_pParentWnd = pParentWnd;
	m_pTestCenter = pTestCenter;

	UINT nTableSize = NULL;

	COLORREF BrushColor = GetSysColor(COLOR_MENUBAR);

	CRect rect(CPoint(CtrlPanelInfo.nX, CtrlPanelInfo.nY), CSize( CtrlPanelInfo.nWidth, CtrlPanelInfo.nHeight));

	if (pParentWnd)
	{
		CRect ParentRect;
		pParentWnd->GetClientRect(&ParentRect);
		pParentWnd->ClientToScreen(&ParentRect);
		rect.MoveToXY((ParentRect.right - CtrlPanelInfo.nWidth), ParentRect.top);
	}
	CString DispSvrCtrlDialogClass;

	DispSvrCtrlDialogClass = AfxRegisterWndClass(
		CS_VREDRAW | CS_HREDRAW,
		::LoadCursor(NULL, IDC_ARROW),
		(HBRUSH) CreateSolidBrush(BrushColor),
		::LoadIcon(NULL, IDI_INFORMATION));

	BOOL bReturnValue = FALSE;
///////////////////////////////////////////////////
// Contrl Panel Window
	bReturnValue = CWnd::CreateEx(NULL, DispSvrCtrlDialogClass, CtrlPanelInfo.ObjectName,
		CtrlPanelInfo.dwStyle, rect, pParentWnd, NULL);
	SetFont(&m_Font, TRUE);

///////////////////////////////////////////////////
// DXVA1
	m_btnDXVA1Group.Create(DXVA1GroupInfo.ObjectName, DXVA1GroupInfo.dwStyle,
		CRect(CPoint(DXVA1GroupInfo.nX,DXVA1GroupInfo.nY),
		CSize(DXVA1GroupInfo.nWidth,DXVA1GroupInfo.nHeight)),
		this, DXVA1GroupInfo.nID);
	m_btnDXVA1Group.SetFont(&m_Font, TRUE);

	nTableSize = sizeof(DXVA1ItemTable) / sizeof(DXVA1ItemTable[0]);
	nTableSize = nTableSize < DXVA1_TEST_ITEM_NUM ? nTableSize : DXVA1_TEST_ITEM_NUM;
	for ( UINT i = 0; i < nTableSize; i++)
	{
		m_btnDXVA1TestItem[i].Create(DXVA1ItemTable[i].ObjectName, DXVA1ItemTable[i].dwStyle,
			CRect(CPoint(DXVA1ItemTable[i].nX,DXVA1ItemTable[i].nY),
			CSize(DXVA1ItemTable[i].nWidth,DXVA1ItemTable[i].nHeight)),
			this, DXVA1ItemTable[i].nID);
		m_btnDXVA1TestItem[i].SetFont(&m_Font, TRUE);
	}
///////////////////////////////////////////////////
//	DXVA2
	m_btnDXVA2Group.Create(DXVA2GroupInfo.ObjectName, DXVA2GroupInfo.dwStyle,
		CRect(CPoint(DXVA2GroupInfo.nX,DXVA2GroupInfo.nY),
		CSize(DXVA2GroupInfo.nWidth,DXVA2GroupInfo.nHeight)),
		this, DXVA2GroupInfo.nID);
	m_btnDXVA2Group.SetFont(&m_Font, TRUE);

	nTableSize = sizeof(DXVA2ItemTable) / sizeof(DXVA2ItemTable[0]);
	nTableSize = nTableSize < DXVA2_TEST_ITEM_NUM ? nTableSize : DXVA2_TEST_ITEM_NUM;
	for ( UINT i = 0; i < nTableSize; i++)
	{
		m_btnDXVA2TestItem[i].Create(DXVA2ItemTable[i].ObjectName, DXVA2ItemTable[i].dwStyle,
			CRect(CPoint(DXVA2ItemTable[i].nX,DXVA2ItemTable[i].nY),
			CSize(DXVA2ItemTable[i].nWidth,DXVA2ItemTable[i].nHeight)),
			this, DXVA2ItemTable[i].nID);
		m_btnDXVA2TestItem[i].SetFont(&m_Font, TRUE);
	}

///////////////////////////////////////////////////
// Codec
	m_btnCodecGroup.Create(CodecGroupInfo.ObjectName, CodecGroupInfo.dwStyle,
		CRect(CPoint(CodecGroupInfo.nX,CodecGroupInfo.nY),
		CSize(CodecGroupInfo.nWidth,CodecGroupInfo.nHeight)),
		this, CodecGroupInfo.nID);
	m_btnCodecGroup.SetFont(&m_Font, TRUE);

	nTableSize = sizeof(CodecItemTable) / sizeof(CodecItemTable[0]);
	nTableSize = nTableSize < CODEC_TEST_ITEM_NUM ? nTableSize : CODEC_TEST_ITEM_NUM;
	for ( UINT i = 0; i < nTableSize; i++)
	{
		m_btnCodecTestItem[i].Create(CodecItemTable[i].ObjectName, CodecItemTable[i].dwStyle,
			CRect(CPoint(CodecItemTable[i].nX,CodecItemTable[i].nY),
			CSize(CodecItemTable[i].nWidth,CodecItemTable[i].nHeight)),
			this, CodecItemTable[i].nID);
		m_btnCodecTestItem[i].SetFont(&m_Font, TRUE);
	}

///////////////////////////////////////////////////
// Button
	m_btnTest.Create(BtnTestInfo.ObjectName, BtnTestInfo.dwStyle,
		CRect(CPoint(BtnTestInfo.nX,BtnTestInfo.nY),
		CSize(BtnTestInfo.nWidth,BtnTestInfo.nHeight)),
		this, BtnTestInfo.nID);
	m_btnTest.SetFont(&m_Font, TRUE);

	SetCtrlPanelItemState(ePanel_Default_State);

	ShowWindow(SW_SHOW);

	return bReturnValue;
}

void CCtrlPanel::EnableTestButton(BOOL bEnable)
{
	if (::IsWindow(m_btnTest.GetSafeHwnd()))
		m_btnTest.EnableWindow(bEnable);
}

void CCtrlPanel::SetCtrlPanelItemState(enumCtrlPanelState eCPState)
{
	BOOL bEnable = TRUE;
	DWORD dwSupportedTestItem = NULL;
	DWORD dwSupportedCodec = NULL;
	UINT nTableSize = NULL;

	if (m_pTestCenter)
	{
		dwSupportedCodec = m_pTestCenter->GetSupportedCodec();
		dwSupportedTestItem = m_pTestCenter->GetSupportedTestItem();
		if ((eCPState == ePanel_InProgress_State) || dwSupportedCodec == NULL || dwSupportedTestItem == NULL)// Disable all items
		{
			bEnable = FALSE;
		}
		else if (eCPState == ePanel_Default_State)
		{
			bEnable = TRUE;
		}
	}
	else
	{
		bEnable = FALSE;
	}
//DXVA1
	nTableSize = sizeof(DXVA1ItemTable) / sizeof(DXVA1ItemTable[0]);
	nTableSize = nTableSize < DXVA1_TEST_ITEM_NUM ? nTableSize : DXVA1_TEST_ITEM_NUM;
	for ( UINT i = 0; i < nTableSize; i++)
	{
		m_btnDXVA1TestItem[i].EnableWindow((DXVA1ItemTable[i].dwValue & dwSupportedTestItem) ? bEnable : FALSE);
	}

//DXVA2
	nTableSize = sizeof(DXVA2ItemTable) / sizeof(DXVA2ItemTable[0]);
	nTableSize = nTableSize < DXVA2_TEST_ITEM_NUM ? nTableSize : DXVA2_TEST_ITEM_NUM;
	for ( UINT i = 0; i < nTableSize; i++)
	{
		m_btnDXVA2TestItem[i].EnableWindow((DXVA2ItemTable[i].dwValue & dwSupportedTestItem) ? bEnable : FALSE);
	}

//Codec
	nTableSize = sizeof(CodecItemTable) / sizeof(CodecItemTable[0]);
	nTableSize = nTableSize < CODEC_TEST_ITEM_NUM ? nTableSize : CODEC_TEST_ITEM_NUM;
	for ( UINT i = 0; i < nTableSize; i++)
	{
		m_btnCodecTestItem[i].EnableWindow((dwSupportedCodec & CodecItemTable[i].dwValue) ? bEnable : FALSE);
	}
//Button
	m_btnTest.EnableWindow(bEnable);
}

// CCtrlPanel message handlers

void CCtrlPanel::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	ShowWindow(SW_HIDE);
}

void CCtrlPanel::OnBtnTestClicked()
{
	if (!m_pTestCenter)
		return;

	UINT nTableSize = NULL;
	DWORD dwTestItem = NULL;
	DWORD dwTestCodecType = NULL;
//DXVA1
	nTableSize = sizeof(DXVA1ItemTable) / sizeof(DXVA1ItemTable[0]);
	nTableSize = nTableSize < DXVA1_TEST_ITEM_NUM ? nTableSize : DXVA1_TEST_ITEM_NUM;
	for ( UINT i = 0; i < nTableSize; i++)
	{
		if (m_btnDXVA1TestItem[i].GetCheck() == BST_CHECKED)
			dwTestItem |= DXVA1ItemTable[i].dwValue;
	}

//DXVA2
	nTableSize = sizeof(DXVA2ItemTable) / sizeof(DXVA2ItemTable[0]);
	nTableSize = nTableSize < DXVA2_TEST_ITEM_NUM ? nTableSize : DXVA2_TEST_ITEM_NUM;
	for ( UINT i = 0; i < nTableSize; i++)
	{
		if (m_btnDXVA2TestItem[i].GetCheck() == BST_CHECKED)
			dwTestItem |= DXVA2ItemTable[i].dwValue;
	}

//Codec
	nTableSize = sizeof(CodecItemTable) / sizeof(CodecItemTable[0]);
	nTableSize = nTableSize < CODEC_TEST_ITEM_NUM ? nTableSize : CODEC_TEST_ITEM_NUM;
	for ( UINT i = 0; i < nTableSize; i++)
	{
		if (m_btnCodecTestItem[i].GetCheck() == BST_CHECKED)
			dwTestCodecType |= CodecItemTable[i].dwValue;
	}

	HRESULT hr = m_pTestCenter->LaunchTest( dwTestItem, dwTestCodecType, FALSE);
	if (hr == S_OK)
	{   //set item state when main frame receives message.
		//SetCtrlPanelItemState(ePanel_InProgress_State);
	}
	else if (hr == E_INVALIDARG)
	{
		CWnd *pWnd = m_pParentWnd ? m_pParentWnd : this;
		pWnd->MessageBox(_T("Invalid selection!"), _T("Error"), MB_OK|MB_ICONWARNING);
	}
	else if (hr == E_ACCESSDENIED)
	{
		CWnd *pWnd = m_pParentWnd ? m_pParentWnd : this;
		pWnd->MessageBox(_T("HVDService Test is already in progress!"), _T("Error"), MB_OK|MB_ICONWARNING);
	}
}