#pragma once

#include "HVDTestDef.h"
// CCtrlPanel
typedef struct
{
	const int nID;
	const DWORD dwValue;
	const int nX;
	const int nY;
	const int nWidth;
	const int nHeight;
	const DWORD dwStyle;
	const TCHAR ObjectName[50];
} ObjectGenInfo;

enum enumDXVA1Type
{
	eDXVA1_OverlayMixer = 0,
	eDXVA1_VMR7,
	eDXVA1_VMR9Windowless,
	eDXVA1_VMR9Custom,
	eDXVA1_External,
};

enum enumDXVA2Type
{
	eDXVA2_Direct3D = 0,
	eDXVA2_DirectShow,
};

enum enumCodecType
{
	eCodec_MPEG2 = 0,
	eCodec_H264,
	eCodec_VC1,
};

enum enumCtrlPanelState
{
	ePanel_Default_State = 0,
	ePanel_InProgress_State,
};

class CHVDTestCenter;
class CCtrlPanel : public CWnd
{
	DECLARE_DYNAMIC(CCtrlPanel)

public:
	CCtrlPanel();
	virtual ~CCtrlPanel();

	BOOL Create(CWnd *pParentWnd, CHVDTestCenter *pTestCenter);
	void SetCtrlPanelItemState(enumCtrlPanelState eCPState);
protected:
	CWnd *m_pParentWnd;
	CFont m_Font;
	
	CButton m_btnDXVA1Group;
	CButton m_btnDXVA2Group;
	CButton m_btnCodecGroup;
	
	CButton m_btnDXVA1TestItem[DXVA1_TEST_ITEM_NUM];
	CButton m_btnDXVA2TestItem[DXVA2_TEST_ITEM_NUM];
	CButton m_btnCodecTestItem[CODEC_TEST_ITEM_NUM];

	CButton m_btnTest;

	CHVDTestCenter *m_pTestCenter;
protected:
	void EnableTestButton(BOOL bEnable);

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnClose();
	afx_msg void OnBtnTestClicked();
};


