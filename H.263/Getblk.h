// Getblk.h: interface for the CGetblk class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GETBLK_H__D3C9E463_48B2_11D5_9DCC_5254AB2B9F00__INCLUDED_)
#define AFX_GETBLK_H__D3C9E463_48B2_11D5_9DCC_5254AB2B9F00__INCLUDED_

#include "Getbits.h"	// Added by ClassView
#include "Sarcode.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CGetblk  
{
public:
	CSarcode m_sacode;
	CGetbits m_getbits;
	void get_sac_block (int comp, int ptype);
	void getblock (int comp, int mode);
	CGetblk();
	virtual ~CGetblk();

private:
	int DecodeTCoef (int position, int intra);
	RunCoef vlc_word_decode (int symbol_word, int *last);
	RunCoef Decode_Escape_Char (int intra, int *last);
};

#endif // !defined(AFX_GETBLK_H__D3C9E463_48B2_11D5_9DCC_5254AB2B9F00__INCLUDED_)
