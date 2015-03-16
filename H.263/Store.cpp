// Store.cpp: implementation of the CStore class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "codedecoder.h"
#include "Store.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#define OBFRSIZE 4096
static unsigned char obfr[OBFRSIZE];
static unsigned char *optr;
static int outfile;
extern unsigned char *clp;
extern int outtype,horizontal_size,coded_picture_width,coded_picture_height;
extern int quiet,vertical_size;
CStore::CStore()
{

}

CStore::~CStore()
{

}
// store a picture as either one frame or two fields
void CStore::storeframe(unsigned char *src[], int frame)
{
    char outname[8]="D:\\测试";
    //outname=
	store_one(outname,src,0,coded_picture_width,vertical_size);
}

void CStore::store_one(char *outname, unsigned char *src[], 
					       int offset, int incr, int height)
{
 switch (outtype)
  {
  case T_BMP:
    store_bmp(outname,src,offset,incr,height);
	break;
  case T_YUV:
   // store_yuv(outname,src,offset,incr,height);
    break;
  case T_YUV_CONC:
   // store_yuv_append(outname,src,offset,incr,height);
    break;
  case T_SIF:
   // store_sif(outname,src,offset,incr,height);
    break;
  case T_TGA:
   // store_ppm_tga(outname,src,offset,incr,height,1);
    break;
  case T_PPM:
  //  store_ppm_tga(outname,src,offset,incr,height,0);
    break;
#ifdef DISPLAY
  case T_X11:
    dither(src);
    break;
#endif
  default:
    break;
  }
}

void CStore::store_bmp(char *outname, unsigned char *src[],  
					               int offset, int incr, int height)
{
  

}

/*
//没有文件头的Y U V格式文件
void CStore::store_yuv(char *outname, unsigned char *src[], 
					       int offset, int incr, int height)
{
  int hsize;
  char tmpname[32];

  hsize = horizontal_size;

  sprintf(tmpname,"%s.Y",outname);
  store_yuv1(tmpname,src[0],offset,incr,hsize,height,0);

  offset>>=1; incr>>=1; hsize>>=1;
  height>>=1;

  sprintf(tmpname,"%s.U",outname);
  store_yuv1(tmpname,src[1],offset,incr,hsize,height,0);

  sprintf(tmpname,"%s.V",outname);
  store_yuv1(tmpname,src[2],offset,incr,hsize,height,0);

}
//有文件头的YUV格式的文件 
void CStore::store_yuv_append(char *outname, unsigned char *src[], int offset, int incr, int height)
{
  int hsize;
  hsize = horizontal_size;

  store_yuv1(outname,src[0],offset,incr,hsize,height,1);

  offset>>=1; incr>>=1; hsize>>=1;
  height>>=1;

  store_yuv1(outname,src[1],offset,incr,hsize,height,1);
  store_yuv1(outname,src[2],offset,incr,hsize,height,1);

}

void CStore::store_sif(char *outname, unsigned char *src[], int offset, int incr, int height)
{
  int i,j;
  unsigned char *py, *pu, *pv;
  static unsigned char *u422, *v422;

  if (!u422) 
  {
    if (!(u422 = (unsigned char *)malloc((coded_picture_width>>1)
					 *coded_picture_height)))
	{
		AfxMessageBox("malloc failed!");
		return;
	}
    if (!(v422 = (unsigned char *)malloc((coded_picture_width>>1)
					 *coded_picture_height)))
	{
		AfxMessageBox("malloc failed!");
		return;
	}
  }
  
  conv420to422(src[1],u422);
  conv420to422(src[2],v422);

  strcat(outname,".SIF");

  if (!quiet)
    fprintf(stderr,"saving %s\n",outname);

  if ((outfile = open(outname,O_CREAT|O_TRUNC|O_WRONLY|O_BINARY,0666))==-1)
  {
      AfxMessageBox("Couldn't create outname!");
      
  }

  optr = obfr;

  for (i=0; i<height; i++)
  {
    py = src[0] + offset + incr*i;
    pu = u422 + (offset>>1) + (incr>>1)*i;
    pv = v422 + (offset>>1) + (incr>>1)*i;

    for (j=0; j<horizontal_size; j+=2)
    {
      putbyte(*pu++);
      putbyte(*py++);
      putbyte(*pv++);
      putbyte(*py++);
    }
  }

  if (optr!=obfr)
    write(outfile,obfr,optr-obfr);

  close(outfile);

}

void CStore::store_ppm_tga(char *outname, unsigned char *src[], int offset, int incr, int height, int tgaflag)
{
  int i, j;
  int y, u, v, r, g, b;
  int crv, cbu, cgu, cgv;
  unsigned char *py, *pu, *pv;
  static unsigned char tga24[14] = {0,0,2,0,0,0,0, 0,0,0,0,0,24,32};
  char header[32];
  static unsigned char *u422, *v422, *u444, *v444;

  if (!u444) {
    if (!(u422 = (unsigned char *)malloc((coded_picture_width>>1)
					 *coded_picture_height)))
      error("malloc failed");
    if (!(v422 = (unsigned char *)malloc((coded_picture_width>>1)
					 *coded_picture_height)))
      error("malloc failed");

    if (!(u444 = (unsigned char *)malloc(coded_picture_width
					 *coded_picture_height)))
      error("malloc failed");

    if (!(v444 = (unsigned char *)malloc(coded_picture_width
					 *coded_picture_height)))
      error("malloc failed");
  }

  conv420to422(src[1],u422);
  conv420to422(src[2],v422);
  conv422to444(u422,u444);
  conv422to444(v422,v444);

  strcat(outname,tgaflag ? ".tga" : ".ppm");

  if (!quiet)
    fprintf(stderr,"saving %s\n",outname);

  if ((outfile = open(outname,O_CREAT|O_TRUNC|O_WRONLY|O_BINARY,0666))==-1)
    {
      sprintf(errortext,"Couldn't create %s\n",outname);
      error(errortext);
    }

  optr = obfr;

  if (tgaflag) {
    //* TGA header 
    for (i=0; i<12; i++)
      putbyte(tga24[i]);

    putword(horizontal_size); putword(height);
    putbyte(tga24[12]); putbyte(tga24[13]);
  }
  else {
    // PPM header 
    sprintf(header,"P6\n%d %d\n255\n",horizontal_size,height);

    for (i=0; header[i]!=0; i++)
      putbyte(header[i]);
  }

  //* matrix coefficients 
  crv = convmat[matrix_coefficients][0];
  cbu = convmat[matrix_coefficients][1];
  cgu = convmat[matrix_coefficients][2];
  cgv = convmat[matrix_coefficients][3];
  
  for (i=0; i<height; i++)
  {
    py = src[0] + offset + incr*i;
    pu = u444 + offset + incr*i;
    pv = v444 + offset + incr*i;

    for (j=0; j<horizontal_size; j++)
	{
      u = *pu++ - 128;
      v = *pv++ - 128;
      y = 76309 * (*py++ - 16); //* (255/219)*65536 
      r = clp[(y + crv*v + 32768)>>16];
      g = clp[(y - cgu*u - cgv*v + 32768)>>16];
      b = clp[(y + cbu*u + 32786)>>16];

      if (tgaflag) 
	  {
    	putbyte(b);
		putbyte(g); 
		putbyte(r);
      }
      else 
	  {
	    putbyte(r);
		putbyte(g);
		putbyte(b);
      }
    }
  }

  if (optr!=obfr)
    write(outfile,obfr,optr-obfr);

  close(outfile);

}

void CStore::store_yuv1(char *name, unsigned char *src, int offset, int incr, int width, int height, int append)
{
 int i, j;
  unsigned char *p;

  if (append)
  {
    if ((outfile = open(name,O_APPEND|O_WRONLY|O_BINARY,0666))==-1) 
	{
      sprintf(errortext,"Couldn't append to %s\n",name);
      error(errortext);
    }
  }
  else 
  {
    if ((outfile = open(name,O_CREAT|O_TRUNC|O_WRONLY|O_BINARY,0666))==-1)
	{
      AfxMessageBox("Couldn't create file name!");
      error(errortext);
    }
  }
  if (!quiet)
    AfxMessageBox("saving file!");

  optr=obfr;

  for (i=0; i<height; i++) 
  {
    p = src + offset + incr*i;
    for (j=0; j<width; j++)
      putbyte(*p++);
  }

  if (optr!=obfr)
    write(outfile,obfr,optr-obfr);

  close(outfile);

}
*/
void CStore::putbyte(int c)
{
  *optr++ = c;

  if (optr == obfr+OBFRSIZE)
  {
//    write(outfile,obfr,OBFRSIZE);
    optr = obfr;
  }

}

void CStore::putword(int w)
{
  putbyte(w); 
  putbyte(w>>8);

}
//  将420格式的文件转化为444的文件
//  1.垂直插值   422
//  2.水平插值
// 水平方向的插值 1:2 interpolation filter 
void CStore::conv422to444(unsigned char *src, unsigned char *dst)
{
 int i, i2, w, j, im3, im2, im1, ip1, ip2, ip3;

  w = coded_picture_width>>1;

  for (j=0; j<coded_picture_height; j++)
  {
    for (i=0; i<w; i++)
	{
      i2 = i<<1;
      im3 = (i<3) ? 0 : i-3;  //左右边界
      im2 = (i<2) ? 0 : i-2;
      im1 = (i<1) ? 0 : i-1;
      ip1 = (i<w-1) ? i+1 : w-1;
      ip2 = (i<w-2) ? i+2 : w-1;
      ip3 = (i<w-3) ? i+3 : w-1;

      // FIR filter coefficients (*256): 5 -21 70 228 -37 11 
      dst[i2] =   clp[(int)(  5*src[im3]
			      -21*src[im2]
			      +70*src[im1]
			      +228*src[i]
			      -37*src[ip1]
			      +11*src[ip2]+128)>>8];

      dst[i2+1] = clp[(int)(  5*src[ip3]
			      -21*src[ip2]
			      +70*src[ip1]
			      +228*src[i]
			      -37*src[im1]
			      +11*src[im2]+128)>>8];
    }
    src+= w;
    dst+= coded_picture_width;
  }
}
// 垂直方向的插值 1:2 interpolation filter 
void CStore::conv420to422(unsigned char *src, unsigned char *dst)
{
  int w, h, i, j, j2;
  int jm3, jm2, jm1, jp1, jp2, jp3;

  w = coded_picture_width>>1;
  h = coded_picture_height>>1;

  /* intra frame */
  for (i=0; i<w; i++) 
  {
    for (j=0; j<h; j++) 
	{
      j2 = j<<1;//
      jm3 = (j<3) ? 0 : j-3;//边界点
      jm2 = (j<2) ? 0 : j-2;
      jm1 = (j<1) ? 0 : j-1;
      jp1 = (j<h-1) ? j+1 : h-1;//边界点
      jp2 = (j<h-2) ? j+2 : h-1;
      jp3 = (j<h-3) ? j+3 : h-1;
       
      // 进行平滑滤波
	  //FIR filter coefficients (*256): 5 -21 70 228 -37 11 
      // New FIR filter coefficients (*256): 3 -16 67 227 -32 7 
      dst[w*j2] =clp[(int)(  3*src[w*jm3]
				  -16*src[w*jm2]
				  +67*src[w*jm1]
				  +227*src[w*j]
				  -32*src[w*jp1]
				  +7*src[w*jp2]+128)>>8];

      dst[w*(j2+1)] = clp[(int)(  3*src[w*jp3]
				  -16*src[w*jp2]
				  +67*src[w*jp1]
				  +227*src[w*j]
				  -32*src[w*jm1]
				  +7*src[w*jm2]+128)>>8];
    }
    src++;
    dst++;
  }
}

