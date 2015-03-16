// Getpic.h: interface for the CGetpic class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GETPIC_H__D3C9E465_48B2_11D5_9DCC_5254AB2B9F00__INCLUDED_)
#define AFX_GETPIC_H__D3C9E465_48B2_11D5_9DCC_5254AB2B9F00__INCLUDED_

#include "Getvlc.h"	// Added by ClassView
#include "Gethdr.h"	// Added by ClassView
#include "Getbits.h"// Added by ClassView
#include "Getblk.h"	// Added by ClassView
#include "Idct.h"	// Added by ClassView
#include "Recon.h"	// Added by ClassView
#include "Store.h"	// Added by ClassView
#include "Sarcode.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CGetpic  
{
public:
	CSarcode m_sacode;
	CStore m_store;
	CRecon m_recon;
	CIdct m_idct;
	CGetblk m_getblk;
	CGetvlc m_getvlc;
	CGetbits m_getbits;
	CGethdr m_gethdr;
	void interpolate_image(unsigned char *in, unsigned char *out, int width, int height);
	CGetpic();
	virtual ~CGetpic();
	void getpicture(int *framenum);

private:
	void make_edge_image(unsigned char *src, unsigned char *dst, int width, int height, int edge);
	void find_bidir_chroma_limits(int vec, int *start, int*stop);
	void find_bidir_limits(int vec, int *start, int*stop, int nhv);
	void reconblock_b(int comp,int bx,int by,int mode,int bdx, int bdy);
	void addblock (int comp, int bx, int by,int addflag);
	int find_pmv(int x, int y, int block, int comp);
	int motion_decode(int vec,int pmv);
	void clearblock(int comp);
	void getMBs(int framenum);
};

#endif // !defined(AFX_GETPIC_H__D3C9E465_48B2_11D5_9DCC_5254AB2B9F00__INCLUDED_)
