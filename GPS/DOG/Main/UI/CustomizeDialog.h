#ifndef CustomizeDialog_h__
#define CustomizeDialog_h__

//#include "Perf2.h"

class CustomizeDialog : public CDialog
{
public:
	//CustomizeDialog();
	CustomizeDialog(CWnd* pParent = NULL);	// standard constructor
	CListBox m_MergeListBox;
	BOOL OnInitDialog();
	void OnOK();
	void OnCancel();

private:
	CPerfPainterWnd* m_mergedPerfWnd;

	DECLARE_MESSAGE_MAP()
};


#endif // CustomizeDialog_h__
