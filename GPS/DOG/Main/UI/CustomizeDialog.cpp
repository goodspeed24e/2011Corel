#include "stdafx.h"
#include "Perf2.h"
#include "MainFrame.h"
#include "CustomizeDialog.h"
#include "CMergeListBox.h"
#include "MyApp.h"


BEGIN_MESSAGE_MAP(CustomizeDialog, CDialog)
END_MESSAGE_MAP()


CustomizeDialog::CustomizeDialog(CWnd* pParent /*=NULL*/)
	:CDialog(IDD_MERGE, pParent)
{
	m_mergedPerfWnd = NULL;
}

BOOL CustomizeDialog::OnInitDialog()
{
	int listBoxIndex = 0;
	m_MergeListBox.Attach(GetDlgItem(IDC_LIST)->m_hWnd);
	map<DWORD, CChartData*> mapChartData = ((MyApp*)AfxGetApp())->m_mapChartData;
	
 	for(map<DWORD, CChartData*>::iterator it = ((MyApp*)AfxGetApp())->m_mapChartData.begin(); it != ((MyApp*)AfxGetApp())->m_mapChartData.end(); it++)
 	{
		//m_MergeListBox.AddString((it->second)->GetDogEventName());
		m_MergeListBox.InsertString(listBoxIndex, (it->second)->GetDogEventName());
		m_MergeListBox.SetItemData(listBoxIndex, (it->second)->GetDogId());
		++listBoxIndex;
 	}

	if (m_mergedPerfWnd != NULL)
	{
		int mergeListCount = m_MergeListBox.GetCount();

		for (int i=0; i < mergeListCount; i++)
		{
			int dogId = (int)m_MergeListBox.GetItemData(i);
			CChartData* pChartData = ((MyApp*)AfxGetApp())->m_mapChartData[dogId];
			if ((CPerfPainterWnd*)pChartData->GetParentWnd() == m_mergedPerfWnd)
				m_MergeListBox.SetSel(i, TRUE);
		}
	}

	return FALSE;
}

void CustomizeDialog::OnOK()
{
	CArray<int,int> aryListBoxSel;
	int mergeListCount = m_MergeListBox.GetCount();
	aryListBoxSel.SetSize(mergeListCount);

	m_MergeListBox.GetSelItems(mergeListCount, aryListBoxSel.GetData()); 
	
	int selCount = m_MergeListBox.GetSelCount();

	if (selCount > 1)
	{
		int dogId = (int)m_MergeListBox.GetItemData(aryListBoxSel.GetAt(0));
		CChartData* pChartData = ((MyApp*)AfxGetApp())->m_mapChartData[dogId];
		
		//Perf window to exist
		m_mergedPerfWnd = (CPerfPainterWnd*)pChartData->GetParentWnd();

		for (int i=1; i < selCount; i++)
		{
			dogId = (int)m_MergeListBox.GetItemData(aryListBoxSel.GetAt(i));
			pChartData = ((MyApp*)AfxGetApp())->m_mapChartData[dogId];
			CPerfPainterWnd* perfPainterWndWnd = (CPerfPainterWnd*)pChartData->GetParentWnd();
			
			//Destroy other perf window (data is merged)
			if (perfPainterWndWnd != m_mergedPerfWnd)
			{
				perfPainterWndWnd->DestroyWindow();

				//Set parent window = merged window
				pChartData->SetParentWnd(m_mergedPerfWnd);

				//Add pChartData to merged window's data list
				m_mergedPerfWnd->m_ActiveDataList.AddTail(pChartData);
			}
		}
	}	//if (selCount > 1)

	CDialog::OnOK();
	m_MergeListBox.Detach();
}

void CustomizeDialog::OnCancel()
{
	CDialog::OnCancel();
	m_MergeListBox.Detach();
}