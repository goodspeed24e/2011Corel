#ifndef CPerfScrollTextWnd_h__
#define CPerfScrollTextWnd_h__

#include "CPerfWnd.h"

class CPerfScrollTextWnd : public CPerfWnd
{
public:
	CPerfScrollTextWnd();
	CPerfScrollTextWnd(int DataSource);

protected:
	afx_msg void OnPaint();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	DECLARE_MESSAGE_MAP()
};

#endif // CPerfScrollTextWnd_h__