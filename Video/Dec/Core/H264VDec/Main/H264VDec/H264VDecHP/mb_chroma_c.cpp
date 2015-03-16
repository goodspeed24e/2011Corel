/*!
*************************************************************************
* \file mb_chroma_c.cpp
*
* \brief
*    Bilinear interpolation function used for chroma Motion Compensation
*
* \coding
*    C-only
*
* \author
*    Main contributors
*    - Ioannis Katsavounidis
*    - Terry Chen
*************************************************************************
*/

#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "global.h"
#include "mbuffer.h"
#include "mb_chroma.h"

mb_chroma_pred_t *mb_chroma_2xH_pred;
mb_chroma_pred_t *mb_chroma_4xH_pred;
mb_chroma_pred_t *mb_chroma_8xH_pred;

#define bil_final(frac,p,q)	(((frac)*((q)-(p)) + ((p)<<3) + 32)>>(6))

/*
static int MUL_table[9][9]={
{ 0, 0, 0, 0, 0, 0, 0, 0, 0},
{ 0, 1, 2, 3, 4, 5, 6, 7, 8},
{ 0, 2, 4, 6, 8,10,12,14,16},
{ 0, 3, 6, 9,12,15,18,21,24},
{ 0, 4, 8,12,16,20,24,28,32},
{ 0, 5,10,15,20,25,30,35,40},
{ 0, 6,12,18,24,30,36,42,48},
{ 0, 7,14,21,28,35,42,49,56},
{ 0, 8,16,24,32,40,48,56,64}
};
*/

//static int weight_table[9]={8,7,6,5,4,3,2,1,0};

/*
#define Chroma_Interp(value,temp_uv,temp_size)\
{\
if0=weight_table[if1];\
jf0=weight_table[jf1];\
if(if1==0)\
{\
if(jf1==0)\
{\
value = ((*(temp_uv+(jj0*temp_size)+ii0)));\
}\
else\
{\
value = (jf0*(*(temp_uv+(jj0*temp_size)+ii0))+\
jf1*(*(temp_uv+(jj1*temp_size)+ii0))+4)>>3;\
}\
}\
else\
{\
if(jf1==0)\
{\
value = (if0*(*(temp_uv+(jj0*temp_size)+ii0))+\
if1*(*(temp_uv+(jj0*temp_size)+ii1))+4)>>3;\
}\
else\
{\
value = (MUL_table[if0][jf0]*(*(temp_uv+(jj0*temp_size)+ii0))+\
MUL_table[if1][jf0]*(*(temp_uv+(jj0*temp_size)+ii1))+\
MUL_table[if0][jf1]*(*(temp_uv+(jj1*temp_size)+ii0))+\
MUL_table[if1][jf1]*(*(temp_uv+(jj1*temp_size)+ii1))+32) >> (6);\
}\
}\
}
*/

static inline void mb_chroma_WxH_pred_c PARGS9( StorablePicture *p,
																							 imgpel *pDstUV,
																							 int stride_dst,
																							 int base_x_pos,
																							 int base_y_pos,
																							 int mv_x,
																							 int mv_y,
																							 int W,
																							 int H)
{
	int if1 = mv_x&7;
	int jf1 = mv_y&7;
	int ii0 = base_x_pos + (mv_x>>3);
	int jj0 = base_y_pos + (mv_y>>3);
	int stride_src = p->UV_stride;
	imgpel *pSrcUV;

	ii0 = __fast_iclip(clip_min_x_cr,clip_max_x_cr,ii0);

	pSrcUV = p->imgUV+(ii0<<1);  

	jj0 = clip_vertical_c ARGS5(jj0, pSrcUV, stride_src, W, H);

	pSrcUV += jj0*stride_src; 

	int if0, jf0;
	int i, j;
	if0 = 8 - if1;
	jf0 = 8 - jf1;

	if(if1==0)
	{
		if(jf1==0)
		{
			for (j=0; j<H; j++)
			{
				for (i=0; i<W*2; i++)
				{
					pDstUV[i] = pSrcUV[i];			
				}
				pSrcUV += stride_src;
				pDstUV += stride_dst;
			}
		}
		else
		{
			for (j=0; j<H; j++)
			{
				for (i=0; i<W*2; i++)
				{
					pDstUV[i] = (jf0*pSrcUV[i]+jf1*pSrcUV[stride_src+i]+4)>>3;			
				}
				pSrcUV += stride_src;
				pDstUV += stride_dst;
			}
		}
	}
	else
	{
		if (jf1==0)
		{
			for (j=0; j<H; j++)
			{
				for (i=0; i<W*2; i++)
				{
					pDstUV[i] = (if0*pSrcUV[i]+if1*pSrcUV[i+2]+4)>>3;			
				}
				pSrcUV += stride_src;
				pDstUV += stride_dst;
			}
		}
		else
		{
			for (j=0; j<H; j++)
			{
				for (i=0; i<W*2; i++)
				{ /*
					pDst[i] = (MUL_table[if0][jf0]*pSrc[i]+ MUL_table[if1][jf0]*pSrc[i+1]+
					MUL_table[if0][jf1]*pSrc[stride_src+i]+ 
					MUL_table[if1][jf1]*pSrc[stride_src+i+1]+32)>> (6); 		
					*/
					//bil_final(y,E,F):

					// Method 1:
					// E = A * (8 - x) + B * x
					// F = C * (8 - x) + D * x
					// P = (E * 8 + (F - E) * y + 32) >> 6
					// pDst[i] = bil_final(jf1, pSrc[i]*if0+pSrc[i+1]*if1, pSrc[stride_src+i]*if0+pSrc[stride_src+i+1]*if1);

					// Method 2:
					// E = A * 8 + (B - A) * x
					// F = C * 8 + (D - C) * x
					// P = (E * 8 + (F - E) * y + 32) >> 6
					pDstUV[i] = bil_final(jf1, (pSrcUV[i]<<3)+(pSrcUV[i+2]-pSrcUV[i])*if1, (pSrcUV[stride_src+i]<<3)+(pSrcUV[stride_src+i+2]-pSrcUV[stride_src+i])*if1);
				}
				pSrcUV += stride_src;
				pDstUV += stride_dst;
			}
		}
	}
}

void mb_chroma_2xH_pred_c PARGS8( StorablePicture *p,
																 imgpel *pDstUV,
																 int stride_dst,
																 int base_x_pos,
																 int base_y_pos,
																 int mv_x,
																 int mv_y,
																 int H)
{
	mb_chroma_WxH_pred_c ARGS9( p, pDstUV, stride_dst, base_x_pos, base_y_pos, mv_x, mv_y, 2, H);
}

void mb_chroma_4xH_pred_c PARGS8( StorablePicture *p,
																 imgpel *pDstUV,
																 int stride_dst,
																 int base_x_pos,
																 int base_y_pos,
																 int mv_x,
																 int mv_y,
																 int H)
{
	mb_chroma_WxH_pred_c ARGS9( p, pDstUV, stride_dst, base_x_pos, base_y_pos, mv_x, mv_y, 4, H);
}

void mb_chroma_8xH_pred_c PARGS8( StorablePicture *p,
																 imgpel *pDstUV,
																 int stride_dst,
																 int base_x_pos,
																 int base_y_pos,
																 int mv_x,
																 int mv_y,
																 int H)
{
	mb_chroma_WxH_pred_c ARGS9( p, pDstUV, stride_dst, base_x_pos, base_y_pos, mv_x, mv_y, 8, H);
}

void mb_chroma_2xH_pred_mv00 PARGS6( StorablePicture *p, imgpel *pDstUV, int stride_dst, int ii0, int jj0, int H )
{
	imgpel *pSrcUV;

	ii0 = __fast_iclip(clip_min_x_cr, clip_max_x_cr, ii0);

	pSrcUV = p->imgUV + (ii0<<1) + (jj0 * p->UV_stride);

	for (int j=0; j<H; j++)
	{
		*((long*)(pDstUV)) = *((long*)(pSrcUV));

		pSrcUV += p->UV_stride;
		pDstUV += stride_dst;
	}
}

void mb_chroma_4xH_pred_mv00 PARGS6( StorablePicture *p, imgpel *pDstUV, int stride_dst, int ii0, int jj0, int H )
{
	imgpel *pSrcUV;

	ii0 = __fast_iclip(clip_min_x_cr, clip_max_x_cr, ii0);

	pSrcUV = p->imgUV + (ii0<<1) + (jj0 * p->UV_stride);

	mb_chroma_4xH_copy( pSrcUV, pDstUV, p->UV_stride, stride_dst, H );
}

void mb_chroma_8xH_pred_mv00 PARGS6( StorablePicture *p, imgpel *pDstUV, int stride_dst, int ii0, int jj0, int H )
{
	imgpel *pSrcUV;

	ii0 = __fast_iclip(clip_min_x_cr, clip_max_x_cr, ii0);

	pSrcUV = p->imgUV + (ii0<<1) + (jj0 * p->UV_stride);	

	mb_chroma_8xH_copy( pSrcUV, pDstUV, p->UV_stride, stride_dst, H );
}

void mb_chroma_2xH_full PARGS8( StorablePicture *p,
															 imgpel *pDstUV,						          
															 int stride_dst,
															 int base_x_pos,
															 int base_y_pos,
															 int mv_x,
															 int mv_y,
															 int H)
{
	if ((mv_x&7) || mv_y)
		mb_chroma_2xH_pred ARGS8(p, pDstUV, stride_dst, base_x_pos, base_y_pos, mv_x, mv_y, H);
	else
		mb_chroma_2xH_pred_mv00 ARGS6(p, pDstUV, stride_dst, base_x_pos + (mv_x>>3), base_y_pos, H);
}

void mb_chroma_4xH_full PARGS8( StorablePicture *p,
															 imgpel *pDstUV,						          
															 int stride_dst,
															 int base_x_pos,
															 int base_y_pos,
															 int mv_x,
															 int mv_y,
															 int H)
{
	if ((mv_x&7) || mv_y)
		mb_chroma_4xH_pred ARGS8(p, pDstUV, stride_dst, base_x_pos, base_y_pos, mv_x, mv_y, H);			
	else
		mb_chroma_4xH_pred_mv00 ARGS6(p, pDstUV, stride_dst, base_x_pos+(mv_x>>3), base_y_pos, H);
}

void mb_chroma_8xH_full PARGS8( StorablePicture *p,
															 imgpel *pDstUV,
															 int stride_dst,
															 int base_x_pos,
															 int base_y_pos,
															 int mv_x,
															 int mv_y,
															 int H)
{
	if ((mv_x&7) || mv_y)
		mb_chroma_8xH_pred ARGS8(p, pDstUV, stride_dst, base_x_pos, base_y_pos, mv_x, mv_y, H);
	else
		mb_chroma_8xH_pred_mv00 ARGS6(p, pDstUV, stride_dst, base_x_pos+(mv_x>>3), base_y_pos, H);
}

void mb_chroma_2xH_int PARGS8( StorablePicture *p,
															imgpel *pDstUV,
															int stride_dst,
															int base_x_pos,
															int base_y_pos,
															int mv_x,
															int mv_y,
															int H)
{
	mb_chroma_2xH_pred ARGS8(p, pDstUV, stride_dst, base_x_pos, base_y_pos, (mv_x&(~7)), (mv_y&(~7)), H);
}

void mb_chroma_4xH_int PARGS8( StorablePicture *p,
															imgpel *pDstUV,
															int stride_dst,
															int base_x_pos,
															int base_y_pos,
															int mv_x,
															int mv_y,
															int H)
{
	mb_chroma_4xH_pred ARGS8(p, pDstUV, stride_dst, base_x_pos, base_y_pos, (mv_x&(~7)), (mv_y&(~7)), H);			
}

void mb_chroma_8xH_int PARGS8( StorablePicture *p,
															imgpel *pDstUV,
															int stride_dst,
															int base_x_pos,
															int base_y_pos,
															int mv_x,
															int mv_y,
															int H)
{
	mb_chroma_8xH_pred ARGS8(p, pDstUV, stride_dst, base_x_pos, base_y_pos, (mv_x&(~7)), (mv_y&(~7)), H);
}