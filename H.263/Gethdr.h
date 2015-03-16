// Gethdr.h: interface for the CGethdr class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GETHDR_H__D3C9E464_48B2_11D5_9DCC_5254AB2B9F00__INCLUDED_)
#define AFX_GETHDR_H__D3C9E464_48B2_11D5_9DCC_5254AB2B9F00__INCLUDED_

#include "Getbits.h"	// Added by ClassView
#include "Sarcode.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CGethdr  
{
public:
	CSarcode m_sacode;
	CGetbits m_getbits;
	void startcode (void);
	int getheader(void);
	CGethdr();
	virtual ~CGethdr();

private:
	void getpicturehdr(void);
};

#endif // !defined(AFX_GETHDR_H__D3C9E464_48B2_11D5_9DCC_5254AB2B9F00__INCLUDED_)
