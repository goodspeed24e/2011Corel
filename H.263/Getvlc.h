// Getvlc.h: interface for the CGetvlc class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GETVLC_H__D3C9E466_48B2_11D5_9DCC_5254AB2B9F00__INCLUDED_)
#define AFX_GETVLC_H__D3C9E466_48B2_11D5_9DCC_5254AB2B9F00__INCLUDED_

#include "Getbits.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CGetvlc  
{
public:
	CGetbits m_getbits;
	int getCBPY(void);
	int getMCBPCintra(void);
	int getMODB(void);
	int getMCBPC (void);
	int getTMNMV (void);
	CGetvlc();
	virtual ~CGetvlc();

};

#endif // !defined(AFX_GETVLC_H__D3C9E466_48B2_11D5_9DCC_5254AB2B9F00__INCLUDED_)
