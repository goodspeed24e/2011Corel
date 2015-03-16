// Sarcode.h: interface for the CSarcode class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SARCODE_H__5CAA58E2_4940_11D5_9DCC_5254AB2B9F00__INCLUDED_)
#define AFX_SARCODE_H__5CAA58E2_4940_11D5_9DCC_5254AB2B9F00__INCLUDED_

#include "Getbits.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CSarcode  
{
public:
   long low, high, code_value, bit, length, sacindex, cum, zerorun;
	CGetbits m_getbits;
	void decoder_reset( );
	int decode_a_symbol(int cumul_freq[ ]);
	CSarcode();
	virtual ~CSarcode();

private:
	void bit_out_psc_layer();
};

#endif // !defined(AFX_SARCODE_H__5CAA58E2_4940_11D5_9DCC_5254AB2B9F00__INCLUDED_)
