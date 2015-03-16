// Display.h: interface for the CDisplay class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DISPLAY_H__5DCE47A3_490D_11D5_9DCC_5254AB2B9F00__INCLUDED_)
#define AFX_DISPLAY_H__5DCE47A3_490D_11D5_9DCC_5254AB2B9F00__INCLUDED_

#include "Idct.h"	// Added by ClassView
#include "Getbits.h"	// Added by ClassView
#include "Gethdr.h"	// Added by ClassView
#include "Getpic.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CDisplay  
{
public:
	CString conclusion;
	CString szPictureFormat;
	CIdct m_idct;
	int nDitherType;
	PBITMAPINFO pbmi;
	HPALETTE hpal,hPalPrev;
	HDC hDC;
    char  *image;
	CGetpic m_getpic;
	CGethdr m_gethdr;
	CGetbits m_getbits;
	void play_movie(CDC *pDC,CString szFileName);
	void exit_display(void);
	void dither(unsigned char *src[]);
	void init_dither(int bpp);
	void init_display();
	CDisplay();
	virtual ~CDisplay();

private:
	void toDeleteNewSpace();
	void initdecoder(void);
};

#endif // !defined(AFX_DISPLAY_H__5DCE47A3_490D_11D5_9DCC_5254AB2B9F00__INCLUDED_)
