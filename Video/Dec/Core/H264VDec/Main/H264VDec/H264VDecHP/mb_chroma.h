
/*!
************************************************************************************
* \file mb_chroma.h
*
* \brief
*     Bilinear interpolation function prototypes used for chroma Motion Compensation
*
* \author
*    Main contributors
*    - Ioannis Katsavounidis
*    - Terry Chen
************************************************************************************
*/

#ifndef _MB_CHROMA_H_
#define _MB_CHROMA_H_

#if defined(_MSC_VER)
// Disable "No EMMS at end of function '<function name>'"
#pragma warning ( disable : 4799 )
#endif


typedef void mb_chroma_pred_t PARGS8( StorablePicture *p,
																		 imgpel *pDstUV,							         
																		 int stride_dst,
																		 int base_x_pos,
																		 int base_y_pos,
																		 int mv_x,
																		 int mv_y,
																		 int H );

mb_chroma_pred_t mb_chroma_2xH_pred_c;
mb_chroma_pred_t mb_chroma_4xH_pred_c;
mb_chroma_pred_t mb_chroma_8xH_pred_c;

mb_chroma_pred_t mb_chroma_2xH_pred_mmx; // for mmx optimization
mb_chroma_pred_t mb_chroma_4xH_pred_mmx; // for mmx optimization
mb_chroma_pred_t mb_chroma_4xH_pred_sse2; // for mmx optimization
mb_chroma_pred_t mb_chroma_8xH_pred_mmx; // for mmx optimization

mb_chroma_pred_t mb_chroma_8xH_pred_sse2;// for sse2 optimization
mb_chroma_pred_t mb_chroma_8xH_pred_mmx;

mb_chroma_pred_t mb_chroma_2xH_full;
mb_chroma_pred_t mb_chroma_4xH_full;
mb_chroma_pred_t mb_chroma_8xH_full;

mb_chroma_pred_t mb_chroma_2xH_int;
mb_chroma_pred_t mb_chroma_4xH_int;
mb_chroma_pred_t mb_chroma_8xH_int;

extern mb_chroma_pred_t *mb_chroma_2xH_pred;
extern mb_chroma_pred_t *mb_chroma_4xH_pred;
extern mb_chroma_pred_t *mb_chroma_8xH_pred;

typedef void mb_chroma_pred_mv00_t PARGS6( StorablePicture *p,
																					imgpel *pDstUV,										  
																					int stride_dst,
																					int ii0,
																					int jj0,
																					int H );

mb_chroma_pred_mv00_t mb_chroma_2xH_pred_mv00;
mb_chroma_pred_mv00_t mb_chroma_4xH_pred_mv00;
mb_chroma_pred_mv00_t mb_chroma_8xH_pred_mv00;

// C version - works for all pad values (2,4,8) by H
inline int clip_vertical_c PARGS5( int jj0, imgpel *pSrcUV, int stride_src, int pad, int H )
{
	int jj, jjj;
	if(jj0<0)
	{
		// We need to worry about Frame/Field difference in extended area
		if(jj0<(-H))
			jj0 = (-H);
		pad++;	// +3 for 2xH blocks, +5 for 4xH blocks, +9 for 8xH blocks
		jjj = jj0*stride_src;
		for(jj=jj0;jj<0;jj++)
		{
			memcpy(pSrcUV+jjj, pSrcUV, (pad<<1));		
			jjj += stride_src;
		}
	}
	else if (jj0+H>clip_max_y_cr)
	{
		if(jj0>clip_max_y_cr)
			jj0 = clip_max_y_cr;
		pad++;	// +3 for 2xH blocks, +5 for 4xH blocks, +9 for 8xH blocks
		jjj = stride_src;
		pSrcUV += clip_max_y_cr*stride_src;
		for(jj=1;jj<=jj0+H-clip_max_y_cr;jj++)
		{
			memcpy(pSrcUV+jjj, pSrcUV, (pad<<1));		
			jjj += stride_src;
		}
	}
	return(jj0);
}

#ifdef H264_ENABLE_ASM
inline void mb_chroma_4xH_copy( imgpel *pSrcUV, imgpel *pDstUV, int stride_src, int stride_dst, int H )
{
	__asm
	{
		mov  esi, pSrcUV;
		mov  edi, pDstUV;
		mov  eax, stride_src;
		mov  ebx, stride_dst;
		mov  edx, H;

Loop4x4:
		movq mm0, [esi];
		movq mm1, [esi+eax];
		movq [edi], mm0;
		movq [edi+ebx], mm1;
		lea  esi, [esi+2*eax];
		movq mm2, [esi];
		movq mm3, [esi+eax];
		lea  edi, [edi+2*ebx];
		movq [edi], mm2;
		movq [edi+ebx], mm3;
		lea  esi, [esi+2*eax];
		lea  edi, [edi+2*ebx];
		sub  edx, 4;
		jg   Loop4x4;		
	}
};


inline void mb_chroma_8xH_copy( imgpel *pSrcUV, imgpel *pDstUV, int stride_src, int stride_dst, int H )
{
	__asm
	{
		mov  esi, pSrcUV;
		mov  edi, pDstUV;
		mov  eax, stride_src;
		mov  ebx, stride_dst;		
		mov  edx, H;

Loop8x4:
		movdqu xmm0, [esi];
		movdqu xmm1, [esi+eax];
		movdqa [edi], xmm0;
		movdqa [edi+ebx], xmm1;
		lea  esi, [esi+2*eax];
		movdqu xmm2, [esi];
		movdqu xmm3, [esi+eax];
		lea  edi, [edi+2*ebx];
		movdqa [edi], xmm2;
		movdqa [edi+ebx], xmm3;
		lea  esi, [esi+2*eax];
		lea  edi, [edi+2*ebx];
		sub  edx, 4;
		jg   Loop8x4;		
	}
};
#else
inline void mb_chroma_4xH_copy( imgpel *pSrcUV, imgpel *pDstUV, int stride_src, int stride_dst, int H )
{
	imgpel *src = pSrcUV;
	imgpel *dst = pDstUV;	

	for (int j = 0; j<H; j+=2)
	{
		*((long*)(dst))             = *((long*)(src));
		*((long*)(dst+stride_dst))  = *((long*)(src+stride_src));
		*((long*)(dst+4))           = *((long*)(src+4));
		*((long*)(dst+stride_dst+4))= *((long*)(src+stride_src+4));
		src += stride_src<<1;
		dst += stride_dst<<1;
	}
}

inline void mb_chroma_8xH_copy( imgpel *pSrcUV, imgpel *pDstUV, int stride_src, int stride_dst, int H )
{
	imgpel *src = pSrcUV;
	imgpel *dst = pDstUV;	

	for (int j = 0; j<H; j+=2)
	{
		*((long*)(dst))             = *((long*)(src));
		*((long*)(dst+stride_dst))  = *((long*)(src+stride_src));
		*((long*)(dst+4))           = *((long*)(src+4));
		*((long*)(dst+4+stride_dst))= *((long*)(src+stride_src+4));
		*((long*)(dst+8))             = *((long*)(src+8));
		*((long*)(dst+8+stride_dst))  = *((long*)(src+stride_src+8));
		*((long*)(dst+12))           = *((long*)(src+12));
		*((long*)(dst+12+stride_dst))= *((long*)(src+stride_src+12));
		src += stride_src<<1;
		dst += stride_dst<<1;
	}		
}
#endif

#endif//_MB_CHROMA_H_
