// Getbits.h: interface for the CGetbits class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GETBITS_H__D3C9E462_48B2_11D5_9DCC_5254AB2B9F00__INCLUDED_)
#define AFX_GETBITS_H__D3C9E462_48B2_11D5_9DCC_5254AB2B9F00__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CGetbits  
{
public:
	unsigned int getbits(int n);
	void flushbits (int n);
	unsigned int getbits1(void);
	unsigned int showbits (int n);
	void fillbfr();
	void initbits(void);
	CGetbits();
	virtual ~CGetbits();

};

#endif // !defined(AFX_GETBITS_H__D3C9E462_48B2_11D5_9DCC_5254AB2B9F00__INCLUDED_)
