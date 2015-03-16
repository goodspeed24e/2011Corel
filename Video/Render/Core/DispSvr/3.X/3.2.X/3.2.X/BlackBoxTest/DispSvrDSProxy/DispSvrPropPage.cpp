#include "stdafx.h"
#include "DispSvrPropPage.h"
#include "resource.h"
#include "../../Exports/Inc/DispSvr_i.c"

CDispSvrPropPage::CDispSvrPropPage(IUnknown *pUnk) :
CBasePropertyPage(NAME("DispSvr Property"), pUnk, IDD_DISPSVR_PROPPAGE, IDS_DISPSVR_PROPPAGE)
{
	m_pKsSet = NULL;
	m_DispSvrInitFlags = NULL;
}

CDispSvrPropPage::~CDispSvrPropPage()
{
	if (m_pKsSet)
		m_pKsSet->Release();
}

CUnknown * WINAPI CDispSvrPropPage::CreateInstance(LPUNKNOWN pUnk, HRESULT *pHr) 
{
	CDispSvrPropPage *pNewObject = new CDispSvrPropPage(pUnk);
	if (pNewObject == NULL) 
	{
		*pHr = E_OUTOFMEMORY;
	}
	return pNewObject;
} 

HRESULT CDispSvrPropPage::OnActivate()
{
	if (m_hwnd)
	{
		EnableWindow( m_hwnd, TRUE);
		if (m_pKsSet)
		{
			HWND hAncestor = GetAncestor( m_hwnd, GA_ROOTOWNER);
			m_pKsSet->Set(PROPSET_DISPSVR, PROP_GRAPHEDIT_HWND, NULL, NULL, hAncestor, sizeof(hAncestor));

			HRESULT hr = E_FAIL;
			DWORD dwReturn = NULL;

			hr = m_pKsSet->Get(PROPSET_DISPSVR, PROP_DISPSVR_INIT_FLAG, NULL, NULL,
				(LPVOID)(&m_DispSvrInitFlags), sizeof(m_DispSvrInitFlags), &dwReturn);
			if (SUCCEEDED(hr))
			{
				struct _FlagMap
				{
					DWORD dwFlag;
					DWORD dwUIID;
				};
				_FlagMap FlagMap[] =
				{
					{DISPSVR_USE_D3D9EX, IDC_USE_D3D9EX},
					{DISPSVR_USE_CUSTOMIZED_OUTPUT, IDC_USE_CUSTOMIZED_OUTPUT},
					{DISPSVR_USE_RT_VIRTUALIZATION, IDC_USE_RT_VIRTUALIZATION},
					{DISPSVR_USE_STECIL_BUFFER, IDC_USE_STECIL_BUFFER},
					{DISPSVR_DEVICE_LOST_NOTIFY, IDC_DEVICE_LOST_NOTIFY},
					{DISPSVR_DETECT_D3D_HIJACK, IDC_DETECT_D3D_HIJACK},
					{DISPSVR_WAITING_FOR_VSYNC, IDC_WAITING_FOR_VSYNC},
				};
				DWORD nSize = sizeof(FlagMap)/ sizeof(FlagMap[0]);
				for (UINT i = 0; i < nSize; i++)
				{
					if (m_DispSvrInitFlags & FlagMap[i].dwFlag)
						CheckDlgButton(m_Dlg, FlagMap[i].dwUIID, BST_CHECKED);
					else
						CheckDlgButton(m_Dlg, FlagMap[i].dwUIID, BST_UNCHECKED);
				}
			}
		}
	}
	return S_OK;
}

HRESULT CDispSvrPropPage::OnConnect(IUnknown *pUnknown)
{
	HRESULT hr;

	hr = pUnknown->QueryInterface(IID_IKsPropertySet, (void **) &m_pKsSet);
	if(FAILED(hr))
		return hr;
	return NOERROR;
}

INT_PTR CDispSvrPropPage::OnReceiveMessage(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{

	switch (uMsg)
	{
	case WM_COMMAND:
		{
			HRESULT hr = E_FAIL;
			UINT nID = NULL;
			BOOL bUICommand = TRUE;
			switch(wParam)
			{
				case IDC_LOAD_DISPSVR:
					nID = PROP_LOAD_DISPSVR;
				break;
				case IDC_LOAD_MS_CUSTOM_EVR:
					nID = PROP_LOAD_MS_CUSTOM_EVR;
					break;
				case IDC_PROCESS_DEVICE_LOST:
					nID = PROP_PROCESS_DEVICE_LOST;
					break;
				case IDC_DETACH_DISPSVR:
					nID = PROP_DETACH_DISPSVR;
					break;
				case IDC_USE_D3D9EX:
					{
						if (IsDlgButtonChecked(m_Dlg, IDC_USE_D3D9EX) == BST_CHECKED)
							m_DispSvrInitFlags |= DISPSVR_USE_D3D9EX;
						else
							m_DispSvrInitFlags &= ~DISPSVR_USE_D3D9EX;

						if (m_pKsSet)
							hr = m_pKsSet->Set(PROPSET_DISPSVR, PROP_DISPSVR_INIT_FLAG, NULL, NULL, ((LPVOID)(&m_DispSvrInitFlags)), sizeof(m_DispSvrInitFlags));
						return (INT_PTR)TRUE;
					}
				case IDC_USE_CUSTOMIZED_OUTPUT:
					{
						if (IsDlgButtonChecked(m_Dlg, IDC_USE_CUSTOMIZED_OUTPUT) == BST_CHECKED)
							m_DispSvrInitFlags |= DISPSVR_USE_CUSTOMIZED_OUTPUT;
						else
							m_DispSvrInitFlags &= ~DISPSVR_USE_CUSTOMIZED_OUTPUT;

						if (m_pKsSet)
							hr = m_pKsSet->Set(PROPSET_DISPSVR, PROP_DISPSVR_INIT_FLAG, NULL, NULL, ((LPVOID)(&m_DispSvrInitFlags)), sizeof(m_DispSvrInitFlags));
						return (INT_PTR)TRUE;
					}
				case IDC_USE_RT_VIRTUALIZATION:
					{
						if (IsDlgButtonChecked(m_Dlg, IDC_USE_RT_VIRTUALIZATION) == BST_CHECKED)
							m_DispSvrInitFlags |= DISPSVR_USE_RT_VIRTUALIZATION;
						else
							m_DispSvrInitFlags &= ~DISPSVR_USE_RT_VIRTUALIZATION;

						if (m_pKsSet)
							hr = m_pKsSet->Set(PROPSET_DISPSVR, PROP_DISPSVR_INIT_FLAG, NULL, NULL, ((LPVOID)(&m_DispSvrInitFlags)), sizeof(m_DispSvrInitFlags));
						return (INT_PTR)TRUE;
					}
				case IDC_USE_STECIL_BUFFER:
					{
						if (IsDlgButtonChecked(m_Dlg, IDC_USE_STECIL_BUFFER) == BST_CHECKED)
							m_DispSvrInitFlags |= DISPSVR_USE_STECIL_BUFFER;
						else
							m_DispSvrInitFlags &= ~DISPSVR_USE_STECIL_BUFFER;

						if (m_pKsSet)
							hr = m_pKsSet->Set(PROPSET_DISPSVR, PROP_DISPSVR_INIT_FLAG, NULL, NULL, ((LPVOID)(&m_DispSvrInitFlags)), sizeof(m_DispSvrInitFlags));
						return (INT_PTR)TRUE;
					}
				case IDC_DEVICE_LOST_NOTIFY:
					{
						if (IsDlgButtonChecked(m_Dlg, IDC_DEVICE_LOST_NOTIFY) == BST_CHECKED)
							m_DispSvrInitFlags |= DISPSVR_DEVICE_LOST_NOTIFY;
						else
							m_DispSvrInitFlags &= ~DISPSVR_DEVICE_LOST_NOTIFY;

						if (m_pKsSet)
							hr = m_pKsSet->Set(PROPSET_DISPSVR, PROP_DISPSVR_INIT_FLAG, NULL, NULL, ((LPVOID)(&m_DispSvrInitFlags)), sizeof(m_DispSvrInitFlags));
						return (INT_PTR)TRUE;
					}
				case IDC_DETECT_D3D_HIJACK:
					{
						if (IsDlgButtonChecked(m_Dlg, IDC_DETECT_D3D_HIJACK) == BST_CHECKED)
							m_DispSvrInitFlags |= DISPSVR_DETECT_D3D_HIJACK;
						else
							m_DispSvrInitFlags &= ~DISPSVR_DETECT_D3D_HIJACK;

						if (m_pKsSet)
							hr = m_pKsSet->Set(PROPSET_DISPSVR, PROP_DISPSVR_INIT_FLAG, NULL, NULL, ((LPVOID)(&m_DispSvrInitFlags)), sizeof(m_DispSvrInitFlags));
						return (INT_PTR)TRUE;
					}
				case IDC_WAITING_FOR_VSYNC:
					{
						if (IsDlgButtonChecked(m_Dlg, IDC_WAITING_FOR_VSYNC) == BST_CHECKED)
							m_DispSvrInitFlags |= DISPSVR_WAITING_FOR_VSYNC;
						else
							m_DispSvrInitFlags &= ~DISPSVR_WAITING_FOR_VSYNC;

						if (m_pKsSet)
							hr = m_pKsSet->Set(PROPSET_DISPSVR, PROP_DISPSVR_INIT_FLAG, NULL, NULL, ((LPVOID)(&m_DispSvrInitFlags)), sizeof(m_DispSvrInitFlags));
						return (INT_PTR)TRUE;
					}
				default:
					bUICommand = FALSE;
					break;
			}
			if (bUICommand)
			{
				if (m_pKsSet)
					hr = m_pKsSet->Set(PROPSET_DISPSVR, nID, NULL, NULL, NULL, NULL);
			}
			return (INT_PTR)TRUE;
		}
		break;
	} // switch


	return CBasePropertyPage::OnReceiveMessage(hwnd, uMsg, wParam, lParam);
}