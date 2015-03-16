// Store.h: interface for the CStore class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_STORE_H__5DCE47A2_490D_11D5_9DCC_5254AB2B9F00__INCLUDED_)
#define AFX_STORE_H__5DCE47A2_490D_11D5_9DCC_5254AB2B9F00__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CStore  
{
public:
	void storeframe(unsigned char *src[], int frame);
	CStore();
	virtual ~CStore();

private:
	void store_bmp(char *outname, unsigned char *src[], int offset, int incr, int height);
	void conv420to422(unsigned char *src, unsigned char *dst);
	void conv422to444(unsigned char *src, unsigned char *dst);
	void putword(int w);
	void putbyte(int c);
	void store_yuv1(char *name, unsigned char *src,int offset, int incr, int width, int height, int append);
	void store_ppm_tga(char *outname, unsigned char *src[],int offset, int incr, int height, int tgaflag);
	void store_sif(char *outname, unsigned char *src[], int offset, int incr, int height);
	void store_yuv_append(char *outname, unsigned char *src[],int offset, int incr, int height);
	void store_yuv(char *outname, unsigned char *src[],int offset, int incr, int height);
	void store_one(char *outname, unsigned char *src[],int offset, int incr, int height);
};

#endif // !defined(AFX_STORE_H__5DCE47A2_490D_11D5_9DCC_5254AB2B9F00__INCLUDED_)
