// Recon.h: interface for the CRecon class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RECON_H__D3C9E468_48B2_11D5_9DCC_5254AB2B9F00__INCLUDED_)
#define AFX_RECON_H__D3C9E468_48B2_11D5_9DCC_5254AB2B9F00__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CRecon  
{
public:
	void reconstruct(int bx, int by, int P, int bdx, int bdy);
	CRecon();
	virtual ~CRecon();

private:
	void recon_comp(unsigned char *src, unsigned char *dst,int lx, int lx2, int w, int h, int x, int y, int dx, int dy, int flag);
    void rec4o(unsigned char *s, int *d, int lx, int lx2, int addflag,int c, int xa, int xb, int ya, int yb);
	void rec4c(unsigned char *s, unsigned char *d, int lx, int lx2, int h);
	void rec4(unsigned char *s, unsigned char *d, int lx, int lx2, int h);
	void recvo(unsigned char *s, int *d, int lx, int lx2, int addflag,int c, int xa, int xb, int ya, int yb);
	void recvc(unsigned char *s, unsigned char *d, int lx, int lx2, int h);
	void recv(unsigned char *s, unsigned char *d, int lx, int lx2, int h);
	void recho(unsigned char *s, int *d, int lx, int lx2, int addflag,int c, int xa, int xb, int ya, int yb);
	void rechc(unsigned char *s, unsigned char *d, int lx, int lx2, int h);
	void rech(unsigned char *s, unsigned char *d, int lx, int lx2, int h);
	void reco(unsigned char *s, int *d, int lx, int lx2, int addflag,int c, int xa, int xb, int ya, int yb);
	void recc(unsigned char *s, unsigned char *d, int lx, int lx2, int h);
	void rec(unsigned char *s, unsigned char *d, int lx, int lx2, int h);
	void recon_comp_obmc(unsigned char *src, unsigned char *dst,int lx,int lx2,int comp,int w,int h,int x,int y);
};

#endif // !defined(AFX_RECON_H__D3C9E468_48B2_11D5_9DCC_5254AB2B9F00__INCLUDED_)
