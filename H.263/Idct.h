// Idct.h: interface for the CIdct class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IDCT_H__D3C9E467_48B2_11D5_9DCC_5254AB2B9F00__INCLUDED_)
#define AFX_IDCT_H__D3C9E467_48B2_11D5_9DCC_5254AB2B9F00__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CIdct  
{
public:
	void idctref(short *block);
	void init_idctref(void);
	void init_idct(void);
	void idct(short *block);
	CIdct();
	virtual ~CIdct();

private:
	void idctcol(short *blk);
	void idctrow(short *blk);
};

#endif // !defined(AFX_IDCT_H__D3C9E467_48B2_11D5_9DCC_5254AB2B9F00__INCLUDED_)
