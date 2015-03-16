#ifndef CPerfStatusWnd_h__
#define CPerfStatusWnd_h__

#include "CPerfWnd.h"

class CPerfStatusWnd : public CPerfWnd
{
public:
	CPerfStatusWnd();
	CPerfStatusWnd(int DataSource);

protected:
	afx_msg void OnPaint();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	DECLARE_MESSAGE_MAP()
};

#endif // CPerfStatusWnd_h__