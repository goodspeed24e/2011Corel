#include "global.h"
#ifdef H264_ENABLE_INTRINSICS
#include <mmintrin.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include "get_block.h"

// Disable "No EMMS at end of function '<function name>'"
#ifdef _MSC_VER
#pragma warning ( disable : 4799 )
#endif

DO_ALIGN(16,const static short ncoeff_16[8])  = {16,16,16,16,16,16,16,16};
DO_ALIGN(16,const static int   ncoeff_512[4]) = {512,512,512,512};

void  get_block_4xh_p00_mmx(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	imgpel *pblock = &block[0][0];

	for(int i = height; i > 0; i-=4)
	{
		*(unsigned long*)(pblock+0*LUMA_BLOCK_SIZE) = *(unsigned long*)(pSrc+0*stride);
		*(unsigned long*)(pblock+1*LUMA_BLOCK_SIZE) = *(unsigned long*)(pSrc+1*stride);
		*(unsigned long*)(pblock+2*LUMA_BLOCK_SIZE) = *(unsigned long*)(pSrc+2*stride);
		*(unsigned long*)(pblock+3*LUMA_BLOCK_SIZE) = *(unsigned long*)(pSrc+3*stride);
		pSrc   += 4*stride;
		pblock += 4*LUMA_BLOCK_SIZE;
	}
}

void  get_block_4xh_p01_mmx(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int j;
	byte *src;
	const int w1 = stride;
	const int w2 = stride + stride;
	const int w3 = w2 + w1;
	src = pSrc;

	__m64 mm0,mm1,mm2,mm3,mm4,mm5,tmp;
	const __m64 nNull    = _mm_setzero_si64();
	const __m64 coeff_16 = *(__m64 *) ncoeff_16;

	for (j = 0; j < height; j++)
	{
		DWORD* pIn0 = (DWORD*) (src-w2);
		DWORD* pIn1 = (DWORD*) (src-w1);
		DWORD* pIn2 = (DWORD*) (src);
		DWORD* pIn3 = (DWORD*) (src+w1);
		DWORD* pIn4 = (DWORD*) (src+w2);
		DWORD* pIn5 = (DWORD*) (src+w3);

		DWORD* pOut = (DWORD*) &block[j][0];

		mm0 = _m_from_int(*pIn0);
		mm1 = _m_from_int(*pIn1);
		mm2 = _m_from_int(*pIn2);
		mm3 = _m_from_int(*pIn3);
		mm4 = _m_from_int(*pIn4);
		mm5 = _m_from_int(*pIn5);

		mm0 = _mm_unpacklo_pi8(mm0, nNull);
		mm1 = _mm_unpacklo_pi8(mm1, nNull);
		mm2 = _mm_unpacklo_pi8(mm2, nNull);
		mm3 = _mm_unpacklo_pi8(mm3, nNull);
		mm4 = _mm_unpacklo_pi8(mm4, nNull);
		mm5 = _mm_unpacklo_pi8(mm5, nNull);

		tmp = _m_psubw(_m_psllwi(_m_paddw(mm2,mm3),2),_m_paddw(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _m_paddw(_m_psllwi(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _m_paddw(_m_paddw(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _m_psrawi(_m_paddw(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _m_packuswb(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		mm2 = _m_packuswb(mm2,mm2);
		tmp = _m_pavgb(tmp,mm2);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		*pOut = _m_to_int(tmp);

		src += w1;
	}
}

void  get_block_4xh_p02_mmx(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int j;
	byte *src;
	const int w1 = stride;
	const int w2 = stride + stride;
	const int w3 = w2 + w1;
	src = pSrc;

	__m64 mm0,mm1,mm2,mm3,mm4,mm5,tmp;
	const __m64 nNull    = _mm_setzero_si64();
	const __m64 coeff_16 = *(__m64 *) ncoeff_16;

	for (j = 0; j < height; j++)
	{
		DWORD* pIn0 = (DWORD*) (src-w2);
		DWORD* pIn1 = (DWORD*) (src-w1);
		DWORD* pIn2 = (DWORD*) (src);
		DWORD* pIn3 = (DWORD*) (src+w1);
		DWORD* pIn4 = (DWORD*) (src+w2);
		DWORD* pIn5 = (DWORD*) (src+w3);

		DWORD* pOut = (DWORD*) &block[j][0];

		mm0 = _m_from_int(*pIn0);
		mm1 = _m_from_int(*pIn1);
		mm2 = _m_from_int(*pIn2);
		mm3 = _m_from_int(*pIn3);
		mm4 = _m_from_int(*pIn4);
		mm5 = _m_from_int(*pIn5);

		mm0 = _mm_unpacklo_pi8(mm0, nNull);
		mm1 = _mm_unpacklo_pi8(mm1, nNull);
		mm2 = _mm_unpacklo_pi8(mm2, nNull);
		mm3 = _mm_unpacklo_pi8(mm3, nNull);
		mm4 = _mm_unpacklo_pi8(mm4, nNull);
		mm5 = _mm_unpacklo_pi8(mm5, nNull);

		tmp = _m_psubw(_m_psllwi(_m_paddw(mm2,mm3),2),_m_paddw(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _m_paddw(_m_psllwi(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _m_paddw(_m_paddw(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _m_psrawi(_m_paddw(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _m_packuswb(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		*pOut = _m_to_int(tmp);

		src += w1;
	}
}

void  get_block_4xh_p03_mmx(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int j;
	byte *src;
	const int w1 = stride;
	const int w2 = stride + stride;
	const int w3 = w2 + w1;
	src = pSrc;

	__m64 mm0,mm1,mm2,mm3,mm4,mm5,tmp;
	const __m64 nNull    = _mm_setzero_si64();
	const __m64 coeff_16 = *(__m64 *) ncoeff_16;

	for (j = 0; j < height; j++)
	{
		DWORD* pIn0 = (DWORD*) (src-w2);
		DWORD* pIn1 = (DWORD*) (src-w1);
		DWORD* pIn2 = (DWORD*) (src);
		DWORD* pIn3 = (DWORD*) (src+w1);
		DWORD* pIn4 = (DWORD*) (src+w2);
		DWORD* pIn5 = (DWORD*) (src+w3);

		DWORD* pOut = (DWORD*) &block[j][0];

		mm0 = _m_from_int(*pIn0);
		mm1 = _m_from_int(*pIn1);
		mm2 = _m_from_int(*pIn2);
		mm3 = _m_from_int(*pIn3);
		mm4 = _m_from_int(*pIn4);
		mm5 = _m_from_int(*pIn5);

		mm0 = _mm_unpacklo_pi8(mm0, nNull);
		mm1 = _mm_unpacklo_pi8(mm1, nNull);
		mm2 = _mm_unpacklo_pi8(mm2, nNull);
		mm3 = _mm_unpacklo_pi8(mm3, nNull);
		mm4 = _mm_unpacklo_pi8(mm4, nNull);
		mm5 = _mm_unpacklo_pi8(mm5, nNull);

		tmp = _m_psubw(_m_psllwi(_m_paddw(mm2,mm3),2),_m_paddw(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _m_paddw(_m_psllwi(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _m_paddw(_m_paddw(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _m_psrawi(_m_paddw(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _m_packuswb(tmp,tmp);
		mm3 = _m_packuswb(mm3,mm3);
		tmp = _m_pavgb(tmp,mm3);
		*pOut = _m_to_int(tmp);

		src += w1;
	}
}

void  get_block_4xh_p10_mmx(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int j;
	byte *src;
	const int w1 = stride;
	src = pSrc;

	__m64 mm0,mm1,mm2,mm3,mm4,mm5,tmp;
	const __m64 nNull    = _mm_setzero_si64();
	const __m64 coeff_16 = *(__m64 *) ncoeff_16;

	for (j = 0; j < height; j++)
	{
		DWORD* pIn0 = (DWORD*) (src-2);
		DWORD* pIn1 = (DWORD*) (src-1);
		DWORD* pIn2 = (DWORD*) (src);
		DWORD* pIn3 = (DWORD*) (src+1);
		DWORD* pIn4 = (DWORD*) (src+2);
		DWORD* pIn5 = (DWORD*) (src+3);

		DWORD* pOut = (DWORD*) &block[j][0];

		mm0 = _m_from_int(*pIn0);
		mm1 = _m_from_int(*pIn1);
		mm2 = _m_from_int(*pIn2);
		mm3 = _m_from_int(*pIn3);
		mm4 = _m_from_int(*pIn4);
		mm5 = _m_from_int(*pIn5);

		mm0 = _mm_unpacklo_pi8(mm0, nNull);
		mm1 = _mm_unpacklo_pi8(mm1, nNull);
		mm2 = _mm_unpacklo_pi8(mm2, nNull);
		mm3 = _mm_unpacklo_pi8(mm3, nNull);
		mm4 = _mm_unpacklo_pi8(mm4, nNull);
		mm5 = _mm_unpacklo_pi8(mm5, nNull);

		tmp = _m_psubw(_m_psllwi(_m_paddw(mm2,mm3),2),_m_paddw(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _m_paddw(_m_psllwi(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _m_paddw(_m_paddw(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _m_psrawi(_m_paddw(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _m_packuswb(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		mm2 = _m_packuswb(mm2,mm2);
		tmp = _m_pavgb(tmp,mm2);
		*pOut = _m_to_int(tmp);

		src += w1;
	}
}

void  get_block_4xh_p11_mmx(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int j;
	byte *src;
	const int w1 = stride;
	const int w2 = stride + stride;
	const int w3 = w2 + w1;

	src = pSrc;

	__m64 mm0,mm1,mm2,mm3,mm4,mm5,mm6,tmp;
	const __m64 nNull    = _mm_setzero_si64();
	const __m64 coeff_16 = *(__m64 *) ncoeff_16;

	for (j = 0; j < height; j++)
	{
		DWORD* pIn0 = (DWORD*) (src-2);
		DWORD* pIn1 = (DWORD*) (src-1);
		DWORD* pIn2 = (DWORD*) (src);
		DWORD* pIn3 = (DWORD*) (src+1);
		DWORD* pIn4 = (DWORD*) (src+2);
		DWORD* pIn5 = (DWORD*) (src+3);

		DWORD* pOut = (DWORD*) &block[j][0];

		mm0 = _m_from_int(*pIn0);
		mm1 = _m_from_int(*pIn1);
		mm2 = _m_from_int(*pIn2);
		mm3 = _m_from_int(*pIn3);
		mm4 = _m_from_int(*pIn4);
		mm5 = _m_from_int(*pIn5);

		mm0 = _mm_unpacklo_pi8(mm0, nNull);
		mm1 = _mm_unpacklo_pi8(mm1, nNull);
		mm2 = _mm_unpacklo_pi8(mm2, nNull);
		mm3 = _mm_unpacklo_pi8(mm3, nNull);
		mm4 = _mm_unpacklo_pi8(mm4, nNull);
		mm5 = _mm_unpacklo_pi8(mm5, nNull);

		tmp = _m_psubw(_m_psllwi(_m_paddw(mm2,mm3),2),_m_paddw(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _m_paddw(_m_psllwi(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _m_paddw(_m_paddw(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _m_psrawi(_m_paddw(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _m_packuswb(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		*pOut = _m_to_int(tmp);

		src += w1;
	}

	src = pSrc;	

	for (j = 0; j < height; j++)
	{
		DWORD* pIn0 = (DWORD*) (src-w2);
		DWORD* pIn1 = (DWORD*) (src-w1);
		DWORD* pIn2 = (DWORD*) (src);
		DWORD* pIn3 = (DWORD*) (src+w1);
		DWORD* pIn4 = (DWORD*) (src+w2);
		DWORD* pIn5 = (DWORD*) (src+w3);

		DWORD* pOut = (DWORD*) &block[j][0];

		mm0 = _m_from_int(*pIn0);
		mm1 = _m_from_int(*pIn1);
		mm2 = _m_from_int(*pIn2);
		mm3 = _m_from_int(*pIn3);
		mm4 = _m_from_int(*pIn4);
		mm5 = _m_from_int(*pIn5);
		mm6 = _m_from_int(*pOut);

		mm0 = _mm_unpacklo_pi8(mm0, nNull);
		mm1 = _mm_unpacklo_pi8(mm1, nNull);
		mm2 = _mm_unpacklo_pi8(mm2, nNull);
		mm3 = _mm_unpacklo_pi8(mm3, nNull);
		mm4 = _mm_unpacklo_pi8(mm4, nNull);
		mm5 = _mm_unpacklo_pi8(mm5, nNull);
		mm6 = _mm_unpacklo_pi8(mm6, nNull);

		tmp = _m_psubw(_m_psllwi(_m_paddw(mm2,mm3),2),_m_paddw(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _m_paddw(_m_psllwi(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _m_paddw(_m_paddw(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _m_psrawi(_m_paddw(tmp,coeff_16),5);//(result_block+16)>>5
		mm6 = _m_packuswb(mm6,mm6);
		tmp = _m_packuswb(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		tmp = _m_pavgb(mm6,tmp);
		*pOut = _m_to_int(tmp);

		src += w1;
	}
}

void  get_block_4xh_p12_mmx(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	byte *src;
	short luma_res[8][9];
	int i, j;
	int tmp_res[8][9];
	const int w1 = stride;
	const int w2 = stride + stride;
	const int w3 = w2 + w1;
	src = pSrc;

	__m64 mm0,mm1,mm2,mm3,mm4,mm5,tmp,tmp1;
	const __m64 nNull     = _mm_setzero_si64();
	const __m64 coeff_16  = *(__m64 *) ncoeff_16;
	const __m64 coeff_512 = *(__m64 *) ncoeff_512;

	for (j = 0; j < height; j++)
	{
		for(i = 0; i < 5; i += 4)
		{
			DWORD* pIn0 = (DWORD*) (src-w2-2+i);
			DWORD* pIn1 = (DWORD*) (src-w1-2+i);
			DWORD* pIn2 = (DWORD*) (src-2+i);
			DWORD* pIn3 = (DWORD*) (src+w1-2+i);
			DWORD* pIn4 = (DWORD*) (src+w2-2+i);
			DWORD* pIn5 = (DWORD*) (src+w3-2+i);

			__m64* pOut = (__m64*) &luma_res[j][i];

			mm0 = _m_from_int(*pIn0);
			mm1 = _m_from_int(*pIn1);
			mm2 = _m_from_int(*pIn2);
			mm3 = _m_from_int(*pIn3);
			mm4 = _m_from_int(*pIn4);
			mm5 = _m_from_int(*pIn5);

			mm0 = _mm_unpacklo_pi8(mm0, nNull);
			mm1 = _mm_unpacklo_pi8(mm1, nNull);
			mm2 = _mm_unpacklo_pi8(mm2, nNull);
			mm3 = _mm_unpacklo_pi8(mm3, nNull);
			mm4 = _mm_unpacklo_pi8(mm4, nNull);
			mm5 = _mm_unpacklo_pi8(mm5, nNull);

			tmp = _m_psubw(_m_psllwi(_m_paddw(mm2,mm3),2),_m_paddw(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
			tmp = _m_paddw(_m_psllwi(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
			tmp = _m_paddw(_m_paddw(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5

			mm0 = _mm_unpacklo_pi16(nNull,tmp);
			mm1 = _mm_unpackhi_pi16(nNull,tmp);

			mm0 = _m_psradi(mm0,16);
			mm1 = _m_psradi(mm1,16);

			tmp = _m_psrawi(_m_paddw(tmp,coeff_16),5);//(result_block+16)>>5

			*((__m64*)&tmp_res[j][i+0]) = mm0;
			*((__m64*)&tmp_res[j][i+2]) = mm1;

			*pOut = tmp;
		}
		tmp_res[j][8] = q_interpol(src[6-w2],src[6-w1], src[6], src[6+w1], src[6+w2], src[6+w3]);
		src += w1;
	}

	for (j = 0; j < height; j++)
	{
		mm0 = *((__m64*)&tmp_res[j][0]);
		mm1 = *((__m64*)&tmp_res[j][1]);
		mm2 = *((__m64*)&tmp_res[j][2]);
		mm3 = *((__m64*)&tmp_res[j][3]);
		mm4 = *((__m64*)&tmp_res[j][4]);
		mm5 = *((__m64*)&tmp_res[j][5]);

		DWORD* pOut = (DWORD*) &block[j][0];

		tmp = _m_psubd(_m_pslldi(_m_paddd(mm2,mm3),2),_m_paddd(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _m_paddd(_m_pslldi(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _m_paddd(_m_paddd(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _m_psradi(_m_paddd(tmp,coeff_512),10);//(result_block+512)>>10

		mm0 = *((__m64*)&tmp_res[j][2]);
		mm1 = *((__m64*)&tmp_res[j][3]);
		mm2 = *((__m64*)&tmp_res[j][4]);
		mm3 = *((__m64*)&tmp_res[j][5]);
		mm4 = *((__m64*)&tmp_res[j][6]);
		mm5 = *((__m64*)&tmp_res[j][7]);

		tmp1 = _m_psubd(_m_pslldi(_m_paddd(mm2,mm3),2),_m_paddd(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _m_paddd(_m_pslldi(tmp1,2),tmp1);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _m_paddd(_m_paddd(mm0,mm5),tmp1); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp1 = _m_psradi(_m_paddd(tmp1,coeff_512),10);//(result_block+512)>>10

		tmp = _m_packssdw(tmp,tmp1);
		mm0 = *((__m64*)&luma_res[j][2]);	//__fast_iclip0_255(((tmp_res[j][i+2]+16)>>5))
		tmp = _m_packuswb(tmp,tmp);
		mm0 = _m_packuswb(mm0,mm0);
		tmp = _m_pavgb(tmp,mm0);

		*pOut = _m_to_int(tmp);
	}
}

void  get_block_4xh_p13_mmx(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int j;
	byte *src;
	const int w1 = stride;
	const int w2 = stride + stride;
	const int w3 = w2 + w1;

	src = pSrc+w1;

	__m64 mm0,mm1,mm2,mm3,mm4,mm5,mm6,tmp;
	const __m64 nNull    = _mm_setzero_si64();
	const __m64 coeff_16 = *(__m64 *) ncoeff_16;

	for (j = 0; j < height; j++)
	{
		DWORD* pIn0 = (DWORD*) (src-2);
		DWORD* pIn1 = (DWORD*) (src-1);
		DWORD* pIn2 = (DWORD*) (src);
		DWORD* pIn3 = (DWORD*) (src+1);
		DWORD* pIn4 = (DWORD*) (src+2);
		DWORD* pIn5 = (DWORD*) (src+3);

		DWORD* pOut = (DWORD*) &block[j][0];

		mm0 = _m_from_int(*pIn0);
		mm1 = _m_from_int(*pIn1);
		mm2 = _m_from_int(*pIn2);
		mm3 = _m_from_int(*pIn3);
		mm4 = _m_from_int(*pIn4);
		mm5 = _m_from_int(*pIn5);

		mm0 = _mm_unpacklo_pi8(mm0, nNull);
		mm1 = _mm_unpacklo_pi8(mm1, nNull);
		mm2 = _mm_unpacklo_pi8(mm2, nNull);
		mm3 = _mm_unpacklo_pi8(mm3, nNull);
		mm4 = _mm_unpacklo_pi8(mm4, nNull);
		mm5 = _mm_unpacklo_pi8(mm5, nNull);

		tmp = _m_psubw(_m_psllwi(_m_paddw(mm2,mm3),2),_m_paddw(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _m_paddw(_m_psllwi(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _m_paddw(_m_paddw(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _m_psrawi(_m_paddw(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _m_packuswb(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		*pOut = _m_to_int(tmp);

		src += w1;
	}

	src = pSrc;

	for (j = 0; j < height; j++)
	{
		DWORD* pIn0 = (DWORD*) (src-w2);
		DWORD* pIn1 = (DWORD*) (src-w1);
		DWORD* pIn2 = (DWORD*) (src);
		DWORD* pIn3 = (DWORD*) (src+w1);
		DWORD* pIn4 = (DWORD*) (src+w2);
		DWORD* pIn5 = (DWORD*) (src+w3);

		DWORD* pOut = (DWORD*) &block[j][0];

		mm0 = _m_from_int(*pIn0);
		mm1 = _m_from_int(*pIn1);
		mm2 = _m_from_int(*pIn2);
		mm3 = _m_from_int(*pIn3);
		mm4 = _m_from_int(*pIn4);
		mm5 = _m_from_int(*pIn5);
		mm6 = _m_from_int(*pOut);

		mm0 = _mm_unpacklo_pi8(mm0, nNull);
		mm1 = _mm_unpacklo_pi8(mm1, nNull);
		mm2 = _mm_unpacklo_pi8(mm2, nNull);
		mm3 = _mm_unpacklo_pi8(mm3, nNull);
		mm4 = _mm_unpacklo_pi8(mm4, nNull);
		mm5 = _mm_unpacklo_pi8(mm5, nNull);
		mm6 = _mm_unpacklo_pi8(mm6, nNull);

		tmp = _m_psubw(_m_psllwi(_m_paddw(mm2,mm3),2),_m_paddw(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _m_paddw(_m_psllwi(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _m_paddw(_m_paddw(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _m_psrawi(_m_paddw(tmp,coeff_16),5);//(result_block+16)>>5
		mm6 = _m_packuswb(mm6,mm6);
		tmp = _m_packuswb(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		tmp = _m_pavgb(mm6,tmp);
		*pOut = _m_to_int(tmp);

		src += w1;
	}
}

void  get_block_4xh_p20_mmx(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int j;
	byte *src;
	const int w1 = stride;

	src = pSrc;

	__m64 mm0,mm1,mm2,mm3,mm4,mm5,tmp;
	const __m64 nNull    = _mm_setzero_si64();
	const __m64 coeff_16 = *(__m64 *) ncoeff_16;

	for (j = 0; j < height; j++)
	{
		DWORD* pIn0 = (DWORD*) (src-2);
		DWORD* pIn1 = (DWORD*) (src-1);
		DWORD* pIn2 = (DWORD*) (src);
		DWORD* pIn3 = (DWORD*) (src+1);
		DWORD* pIn4 = (DWORD*) (src+2);
		DWORD* pIn5 = (DWORD*) (src+3);

		DWORD* pOut = (DWORD*) &block[j][0];

		mm0 = _m_from_int(*pIn0);
		mm1 = _m_from_int(*pIn1);
		mm2 = _m_from_int(*pIn2);
		mm3 = _m_from_int(*pIn3);
		mm4 = _m_from_int(*pIn4);
		mm5 = _m_from_int(*pIn5);

		mm0 = _mm_unpacklo_pi8(mm0, nNull);
		mm1 = _mm_unpacklo_pi8(mm1, nNull);
		mm2 = _mm_unpacklo_pi8(mm2, nNull);
		mm3 = _mm_unpacklo_pi8(mm3, nNull);
		mm4 = _mm_unpacklo_pi8(mm4, nNull);
		mm5 = _mm_unpacklo_pi8(mm5, nNull);

		tmp = _m_psubw(_m_psllwi(_m_paddw(mm2,mm3),2),_m_paddw(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _m_paddw(_m_psllwi(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _m_paddw(_m_paddw(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _m_psrawi(_m_paddw(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _m_packuswb(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		*pOut = _m_to_int(tmp);

		src += w1;
	}
}

void  get_block_4xh_p21_mmx(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	byte *src;
	short luma_res[13][4];
	int j;
	int tmp_res[13][4];
	const int w1 = stride;

	src = pSrc-(2*w1);

	__m64 mm0,mm1,mm2,mm3,mm4,mm5,tmp,tmp1;
	const __m64 nNull     = _mm_setzero_si64();
	const __m64 coeff_16  = *(__m64 *) ncoeff_16;
	const __m64 coeff_512 = *(__m64 *) ncoeff_512;

	for (j = 0; j < height+5; j++)
	{
		DWORD* pIn0 = (DWORD*) (src-2);
		DWORD* pIn1 = (DWORD*) (src-1);
		DWORD* pIn2 = (DWORD*) (src);
		DWORD* pIn3 = (DWORD*) (src+1);
		DWORD* pIn4 = (DWORD*) (src+2);
		DWORD* pIn5 = (DWORD*) (src+3);

		__m64* pOut = (__m64*) &luma_res[j][0];

		mm0 = _m_from_int(*pIn0);
		mm1 = _m_from_int(*pIn1);
		mm2 = _m_from_int(*pIn2);
		mm3 = _m_from_int(*pIn3);
		mm4 = _m_from_int(*pIn4);
		mm5 = _m_from_int(*pIn5);

		mm0 = _mm_unpacklo_pi8(mm0, nNull);
		mm1 = _mm_unpacklo_pi8(mm1, nNull);
		mm2 = _mm_unpacklo_pi8(mm2, nNull);
		mm3 = _mm_unpacklo_pi8(mm3, nNull);
		mm4 = _mm_unpacklo_pi8(mm4, nNull);
		mm5 = _mm_unpacklo_pi8(mm5, nNull);

		tmp = _m_psubw(_m_psllwi(_m_paddw(mm2,mm3),2),_m_paddw(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _m_paddw(_m_psllwi(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _m_paddw(_m_paddw(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5

		mm0 = _mm_unpacklo_pi16(nNull,tmp);
		mm1 = _mm_unpackhi_pi16(nNull,tmp);

		mm0 = _m_psradi(mm0,16);
		mm1 = _m_psradi(mm1,16);

		tmp = _m_psrawi(_m_paddw(tmp,coeff_16),5);//(result_block+16)>>5

		*((__m64*)&tmp_res[j][0]) = mm0;
		*((__m64*)&tmp_res[j][2]) = mm1;

		*pOut = tmp;

		src += w1;
	}

	for (j = 0; j < height; j++)
	{
		mm0 = *((__m64*)&tmp_res[j][0]);
		mm1 = *((__m64*)&tmp_res[j+1][0]);
		mm2 = *((__m64*)&tmp_res[j+2][0]);
		mm3 = *((__m64*)&tmp_res[j+3][0]);
		mm4 = *((__m64*)&tmp_res[j+4][0]);
		mm5 = *((__m64*)&tmp_res[j+5][0]);

		DWORD* pOut = (DWORD*) &block[j][0];

		tmp = _m_psubd(_m_pslldi(_m_paddd(mm2,mm3),2),_m_paddd(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _m_paddd(_m_pslldi(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _m_paddd(_m_paddd(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _m_psradi(_m_paddd(tmp,coeff_512),10);//(result_block+512)>>10

		mm0 = *((__m64*)&tmp_res[j][2]);
		mm1 = *((__m64*)&tmp_res[j+1][2]);
		mm2 = *((__m64*)&tmp_res[j+2][2]);
		mm3 = *((__m64*)&tmp_res[j+3][2]);
		mm4 = *((__m64*)&tmp_res[j+4][2]);
		mm5 = *((__m64*)&tmp_res[j+5][2]);

		tmp1 = _m_psubd(_m_pslldi(_m_paddd(mm2,mm3),2),_m_paddd(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _m_paddd(_m_pslldi(tmp1,2),tmp1);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _m_paddd(_m_paddd(mm0,mm5),tmp1); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp1 = _m_psradi(_m_paddd(tmp1,coeff_512),10);//(result_block+512)>>10

		tmp = _m_packssdw(tmp,tmp1);//check
		mm0 = *((__m64*)&luma_res[j+2][0]);	//__fast_iclip0_255(((tmp_res[j+2][i]+16)>>5))
		tmp = _m_packuswb(tmp,tmp);
		mm0 = _m_packuswb(mm0,mm0);
		tmp = _m_pavgb(tmp,mm0);
		*pOut = _m_to_int(tmp);
	}
}

void  get_block_4xh_p22_mmx(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	byte *src;
	int j;
	int tmp_res[13][4];
	const int w1 = stride;

	src = pSrc - (2*w1);

	__m64 mm0,mm1,mm2,mm3,mm4,mm5,tmp,tmp1;
	const __m64 nNull     = _mm_setzero_si64();
	const __m64 coeff_512 = *(__m64 *) ncoeff_512;

	for (j = 0; j < height+5; j++)
	{
		DWORD* pIn0 = (DWORD*) (src-2);
		DWORD* pIn1 = (DWORD*) (src-1);
		DWORD* pIn2 = (DWORD*) (src);
		DWORD* pIn3 = (DWORD*) (src+1);
		DWORD* pIn4 = (DWORD*) (src+2);
		DWORD* pIn5 = (DWORD*) (src+3);

		mm0 = _m_from_int(*pIn0);
		mm1 = _m_from_int(*pIn1);
		mm2 = _m_from_int(*pIn2);
		mm3 = _m_from_int(*pIn3);
		mm4 = _m_from_int(*pIn4);
		mm5 = _m_from_int(*pIn5);

		mm0 = _mm_unpacklo_pi8(mm0, nNull);
		mm1 = _mm_unpacklo_pi8(mm1, nNull);
		mm2 = _mm_unpacklo_pi8(mm2, nNull);
		mm3 = _mm_unpacklo_pi8(mm3, nNull);
		mm4 = _mm_unpacklo_pi8(mm4, nNull);
		mm5 = _mm_unpacklo_pi8(mm5, nNull);

		tmp = _m_psubw(_m_psllwi(_m_paddw(mm2,mm3),2),_m_paddw(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _m_paddw(_m_psllwi(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _m_paddw(_m_paddw(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5

		mm0 = _mm_unpacklo_pi16(nNull,tmp);
		mm1 = _mm_unpackhi_pi16(nNull,tmp);

		mm0 = _m_psradi(mm0,16);
		mm1 = _m_psradi(mm1,16);

		*((__m64*)&tmp_res[j][0]) = mm0;
		*((__m64*)&tmp_res[j][2]) = mm1;

		src += w1;
	}

	for (j = 0; j < height; j++)
	{
		mm0 = *((__m64*)&tmp_res[j][0]);
		mm1 = *((__m64*)&tmp_res[j+1][0]);
		mm2 = *((__m64*)&tmp_res[j+2][0]);
		mm3 = *((__m64*)&tmp_res[j+3][0]);
		mm4 = *((__m64*)&tmp_res[j+4][0]);
		mm5 = *((__m64*)&tmp_res[j+5][0]);

		DWORD* pOut = (DWORD*) &block[j][0];

		tmp = _m_psubd(_m_pslldi(_m_paddd(mm2,mm3),2),_m_paddd(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _m_paddd(_m_pslldi(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _m_paddd(_m_paddd(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _m_psradi(_m_paddd(tmp,coeff_512),10);//(result_block+512)>>10

		mm0 = *((__m64*)&tmp_res[j][2]);
		mm1 = *((__m64*)&tmp_res[j+1][2]);
		mm2 = *((__m64*)&tmp_res[j+2][2]);
		mm3 = *((__m64*)&tmp_res[j+3][2]);
		mm4 = *((__m64*)&tmp_res[j+4][2]);
		mm5 = *((__m64*)&tmp_res[j+5][2]);

		tmp1 = _m_psubd(_m_pslldi(_m_paddd(mm2,mm3),2),_m_paddd(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _m_paddd(_m_pslldi(tmp1,2),tmp1);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _m_paddd(_m_paddd(mm0,mm5),tmp1); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp1 = _m_psradi(_m_paddd(tmp1,coeff_512),10);//(result_block+512)>>10

		tmp = _m_packssdw(tmp,tmp1);//check
		tmp = _m_packuswb(tmp,tmp);//__fast_iclip0_255((result_block+512)>>10)
		*pOut = _m_to_int(tmp);
	}
}

void  get_block_4xh_p23_mmx(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	short luma_res[13][4];
	int j;
	int tmp_res[13][4];
	byte *src;
	const int w1 = stride;

	src = pSrc -(2*w1);

	__m64 mm0,mm1,mm2,mm3,mm4,mm5,tmp,tmp1;
	const __m64 nNull     = _mm_setzero_si64();
	const __m64 coeff_16  = *(__m64 *) ncoeff_16;
	const __m64 coeff_512 = *(__m64 *) ncoeff_512;

	for (j = 0; j < height+5; j++)
	{
		DWORD* pIn0 = (DWORD*) (src-2);
		DWORD* pIn1 = (DWORD*) (src-1);
		DWORD* pIn2 = (DWORD*) (src);
		DWORD* pIn3 = (DWORD*) (src+1);
		DWORD* pIn4 = (DWORD*) (src+2);
		DWORD* pIn5 = (DWORD*) (src+3);

		__m64* pOut = (__m64*) &luma_res[j][0];

		mm0 = _m_from_int(*pIn0);
		mm1 = _m_from_int(*pIn1);
		mm2 = _m_from_int(*pIn2);
		mm3 = _m_from_int(*pIn3);
		mm4 = _m_from_int(*pIn4);
		mm5 = _m_from_int(*pIn5);

		mm0 = _mm_unpacklo_pi8(mm0, nNull);
		mm1 = _mm_unpacklo_pi8(mm1, nNull);
		mm2 = _mm_unpacklo_pi8(mm2, nNull);
		mm3 = _mm_unpacklo_pi8(mm3, nNull);
		mm4 = _mm_unpacklo_pi8(mm4, nNull);
		mm5 = _mm_unpacklo_pi8(mm5, nNull);

		tmp = _m_psubw(_m_psllwi(_m_paddw(mm2,mm3),2),_m_paddw(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _m_paddw(_m_psllwi(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _m_paddw(_m_paddw(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5

		mm0 = _mm_unpacklo_pi16(nNull,tmp);
		mm1 = _mm_unpackhi_pi16(nNull,tmp);

		mm0 = _m_psradi(mm0,16);
		mm1 = _m_psradi(mm1,16);

		tmp = _m_psrawi(_m_paddw(tmp,coeff_16),5);//(result_block+16)>>5

		*((__m64*)&tmp_res[j][0]) = mm0;
		*((__m64*)&tmp_res[j][2]) = mm1;

		*pOut = tmp;

		src += w1;
	}


	for (j = 0; j < height; j++)
	{
		mm0 = *((__m64*)&tmp_res[j][0]);
		mm1 = *((__m64*)&tmp_res[j+1][0]);
		mm2 = *((__m64*)&tmp_res[j+2][0]);
		mm3 = *((__m64*)&tmp_res[j+3][0]);
		mm4 = *((__m64*)&tmp_res[j+4][0]);
		mm5 = *((__m64*)&tmp_res[j+5][0]);

		DWORD* pOut = (DWORD*) &block[j][0];

		tmp = _m_psubd(_m_pslldi(_m_paddd(mm2,mm3),2),_m_paddd(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _m_paddd(_m_pslldi(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _m_paddd(_m_paddd(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _m_psradi(_m_paddd(tmp,coeff_512),10);//(result_block+512)>>10

		mm0 = *((__m64*)&tmp_res[j][2]);
		mm1 = *((__m64*)&tmp_res[j+1][2]);
		mm2 = *((__m64*)&tmp_res[j+2][2]);
		mm3 = *((__m64*)&tmp_res[j+3][2]);
		mm4 = *((__m64*)&tmp_res[j+4][2]);
		mm5 = *((__m64*)&tmp_res[j+5][2]);

		tmp1 = _m_psubd(_m_pslldi(_m_paddd(mm2,mm3),2),_m_paddd(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _m_paddd(_m_pslldi(tmp1,2),tmp1);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _m_paddd(_m_paddd(mm0,mm5),tmp1); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp1 = _m_psradi(_m_paddd(tmp1,coeff_512),10);//(result_block+512)>>10

		tmp = _m_packssdw(tmp,tmp1);//check
		mm0 = *((__m64*)&luma_res[j+3][0]);	//__fast_iclip0_255(((tmp_res[j+2][i]+16)>>5))
		tmp = _m_packuswb(tmp,tmp);
		mm0 = _m_packuswb(mm0,mm0);
		tmp = _m_pavgb(tmp,mm0);

		*pOut = _m_to_int(tmp);
	}
}

void  get_block_4xh_p30_mmx(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int j;
	byte *src;
	const int w1 = stride;

	src = pSrc;

	__m64 mm0,mm1,mm2,mm3,mm4,mm5,tmp;
	const __m64 nNull    = _mm_setzero_si64();
	const __m64 coeff_16 = *(__m64 *) ncoeff_16;

	for (j = 0; j < height; j++)
	{
		DWORD* pIn0 = (DWORD*) (src-2);
		DWORD* pIn1 = (DWORD*) (src-1);
		DWORD* pIn2 = (DWORD*) (src);
		DWORD* pIn3 = (DWORD*) (src+1);
		DWORD* pIn4 = (DWORD*) (src+2);
		DWORD* pIn5 = (DWORD*) (src+3);

		DWORD* pOut = (DWORD*) &block[j][0];

		mm0 = _m_from_int(*pIn0);
		mm1 = _m_from_int(*pIn1);
		mm2 = _m_from_int(*pIn2);
		mm3 = _m_from_int(*pIn3);
		mm4 = _m_from_int(*pIn4);
		mm5 = _m_from_int(*pIn5);

		mm0 = _mm_unpacklo_pi8(mm0, nNull);
		mm1 = _mm_unpacklo_pi8(mm1, nNull);
		mm2 = _mm_unpacklo_pi8(mm2, nNull);
		mm3 = _mm_unpacklo_pi8(mm3, nNull);
		mm4 = _mm_unpacklo_pi8(mm4, nNull);
		mm5 = _mm_unpacklo_pi8(mm5, nNull);

		tmp = _m_psubw(_m_psllwi(_m_paddw(mm2,mm3),2),_m_paddw(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _m_paddw(_m_psllwi(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _m_paddw(_m_paddw(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _m_psrawi(_m_paddw(tmp,coeff_16),5);//(result_block+16)>>5
		mm3 = _m_packuswb(mm3,mm3);
		tmp = _m_packuswb(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		tmp = _m_pavgb(mm3,tmp);
		*pOut = _m_to_int(tmp);

		src += w1;
	}
}

void  get_block_4xh_p31_mmx(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int j;
	byte *src;
	const int w1 = stride;
	const int w2 = stride + stride;
	const int w3 = w2 + w1;

	src = pSrc;

	__m64 mm0,mm1,mm2,mm3,mm4,mm5,mm6,tmp;
	const __m64 nNull    = _mm_setzero_si64();
	const __m64 coeff_16 = *(__m64 *) ncoeff_16;

	for (j = 0; j < height; j++)
	{
		DWORD* pIn0 = (DWORD*) (src-2);
		DWORD* pIn1 = (DWORD*) (src-1);
		DWORD* pIn2 = (DWORD*) (src);
		DWORD* pIn3 = (DWORD*) (src+1);
		DWORD* pIn4 = (DWORD*) (src+2);
		DWORD* pIn5 = (DWORD*) (src+3);

		DWORD* pOut = (DWORD*) &block[j][0];

		mm0 = _m_from_int(*pIn0);
		mm1 = _m_from_int(*pIn1);
		mm2 = _m_from_int(*pIn2);
		mm3 = _m_from_int(*pIn3);
		mm4 = _m_from_int(*pIn4);
		mm5 = _m_from_int(*pIn5);

		mm0 = _mm_unpacklo_pi8(mm0, nNull);
		mm1 = _mm_unpacklo_pi8(mm1, nNull);
		mm2 = _mm_unpacklo_pi8(mm2, nNull);
		mm3 = _mm_unpacklo_pi8(mm3, nNull);
		mm4 = _mm_unpacklo_pi8(mm4, nNull);
		mm5 = _mm_unpacklo_pi8(mm5, nNull);

		tmp = _m_psubw(_m_psllwi(_m_paddw(mm2,mm3),2),_m_paddw(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _m_paddw(_m_psllwi(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _m_paddw(_m_paddw(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _m_psrawi(_m_paddw(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _m_packuswb(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		*pOut = _m_to_int(tmp);

		src += w1;
	}

	src = pSrc + 1;

	for (j = 0; j < height; j++)
	{
		DWORD* pIn0 = (DWORD*) (src-w2);
		DWORD* pIn1 = (DWORD*) (src-w1);
		DWORD* pIn2 = (DWORD*) (src);
		DWORD* pIn3 = (DWORD*) (src+w1);
		DWORD* pIn4 = (DWORD*) (src+w2);
		DWORD* pIn5 = (DWORD*) (src+w3);

		DWORD* pOut = (DWORD*) &block[j][0];

		mm0 = _m_from_int(*pIn0);
		mm1 = _m_from_int(*pIn1);
		mm2 = _m_from_int(*pIn2);
		mm3 = _m_from_int(*pIn3);
		mm4 = _m_from_int(*pIn4);
		mm5 = _m_from_int(*pIn5);
		mm6 = _m_from_int(*pOut);

		mm0 = _mm_unpacklo_pi8(mm0, nNull);
		mm1 = _mm_unpacklo_pi8(mm1, nNull);
		mm2 = _mm_unpacklo_pi8(mm2, nNull);
		mm3 = _mm_unpacklo_pi8(mm3, nNull);
		mm4 = _mm_unpacklo_pi8(mm4, nNull);
		mm5 = _mm_unpacklo_pi8(mm5, nNull);
		mm6 = _mm_unpacklo_pi8(mm6, nNull);

		tmp = _m_psubw(_m_psllwi(_m_paddw(mm2,mm3),2),_m_paddw(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _m_paddw(_m_psllwi(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _m_paddw(_m_paddw(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _m_psrawi(_m_paddw(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _m_packuswb(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		mm6 = _m_packuswb(mm6,mm6);
		tmp = _m_pavgb(mm6,tmp);
		*pOut = _m_to_int(tmp);

		src += w1;
	}
}

void  get_block_4xh_p32_mmx(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	byte *src;
	short luma_res[8][9];
	int i, j;
	int tmp_res[8][9];
	const int w1 = stride;
	const int w2 = stride + stride;
	const int w3 = w2 + w1;

	src = pSrc;

	__m64 mm0,mm1,mm2,mm3,mm4,mm5,tmp,tmp1;
	const __m64 nNull     = _mm_setzero_si64();
	const __m64 coeff_16  = *(__m64 *) ncoeff_16;
	const __m64 coeff_512 = *(__m64 *) ncoeff_512;

	for (j = 0; j < height; j++)
	{
		for(i = 0; i < 5; i += 4)
		{
			DWORD* pIn0 = (DWORD*) (src-w2-2+i);
			DWORD* pIn1 = (DWORD*) (src-w1-2+i);
			DWORD* pIn2 = (DWORD*) (src-2+i);
			DWORD* pIn3 = (DWORD*) (src+w1-2+i);
			DWORD* pIn4 = (DWORD*) (src+w2-2+i);
			DWORD* pIn5 = (DWORD*) (src+w3-2+i);

			__m64* pOut = (__m64*) &luma_res[j][i];

			mm0 = _m_from_int(*pIn0);
			mm1 = _m_from_int(*pIn1);
			mm2 = _m_from_int(*pIn2);
			mm3 = _m_from_int(*pIn3);
			mm4 = _m_from_int(*pIn4);
			mm5 = _m_from_int(*pIn5);

			mm0 = _mm_unpacklo_pi8(mm0, nNull);
			mm1 = _mm_unpacklo_pi8(mm1, nNull);
			mm2 = _mm_unpacklo_pi8(mm2, nNull);
			mm3 = _mm_unpacklo_pi8(mm3, nNull);
			mm4 = _mm_unpacklo_pi8(mm4, nNull);
			mm5 = _mm_unpacklo_pi8(mm5, nNull);

			tmp = _m_psubw(_m_psllwi(_m_paddw(mm2,mm3),2),_m_paddw(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
			tmp = _m_paddw(_m_psllwi(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
			tmp = _m_paddw(_m_paddw(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5

			mm0 = _mm_unpacklo_pi16(nNull,tmp);
			mm1 = _mm_unpackhi_pi16(nNull,tmp);

			mm0 = _m_psradi(mm0,16);
			mm1 = _m_psradi(mm1,16);

			tmp = _m_psrawi(_m_paddw(tmp,coeff_16),5);//(result_block+16)>>5

			*((__m64*)&tmp_res[j][i+0]) = mm0;
			*((__m64*)&tmp_res[j][i+2]) = mm1;

			*pOut = tmp;
		}
		tmp_res[j][8] = q_interpol(src[6-w2],src[6-w1], src[6], src[6+w1], src[6+w2], src[6+w3]);
		src += w1;
	}

	for (j = 0; j < height; j++)
	{
		mm0 = *((__m64*)&tmp_res[j][0]);
		mm1 = *((__m64*)&tmp_res[j][1]);
		mm2 = *((__m64*)&tmp_res[j][2]);
		mm3 = *((__m64*)&tmp_res[j][3]);
		mm4 = *((__m64*)&tmp_res[j][4]);
		mm5 = *((__m64*)&tmp_res[j][5]);

		DWORD* pOut = (DWORD*) &block[j][0];

		tmp = _m_psubd(_m_pslldi(_m_paddd(mm2,mm3),2),_m_paddd(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _m_paddd(_m_pslldi(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _m_paddd(_m_paddd(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _m_psradi(_m_paddd(tmp,coeff_512),10);//(result_block+512)>>10

		mm0 = *((__m64*)&tmp_res[j][2]);
		mm1 = *((__m64*)&tmp_res[j][3]);
		mm2 = *((__m64*)&tmp_res[j][4]);
		mm3 = *((__m64*)&tmp_res[j][5]);
		mm4 = *((__m64*)&tmp_res[j][6]);
		mm5 = *((__m64*)&tmp_res[j][7]);

		tmp1 = _m_psubd(_m_pslldi(_m_paddd(mm2,mm3),2),_m_paddd(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _m_paddd(_m_pslldi(tmp1,2),tmp1);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _m_paddd(_m_paddd(mm0,mm5),tmp1); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp1 = _m_psradi(_m_paddd(tmp1,coeff_512),10);//(result_block+512)>>10

		tmp = _m_packssdw(tmp,tmp1);
		mm0 = *((__m64*)&luma_res[j][3]);	//__fast_iclip0_255(((tmp_res[j][i+3]+16)>>5))
		tmp = _m_packuswb(tmp,tmp);
		mm0 = _m_packuswb(mm0,mm0);
		tmp = _m_pavgb(tmp,mm0);
		*pOut = _m_to_int(tmp);
	}
}

void  get_block_4xh_p33_mmx(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	byte *src;
	int j;
	const int w1 = stride;
	const int w2 = stride + stride;
	const int w3 = w2 + w1;

	src = pSrc + w1;

	__m64 mm0,mm1,mm2,mm3,mm4,mm5,mm6,tmp;
	const __m64 nNull    = _mm_setzero_si64();
	const __m64 coeff_16 = *(__m64 *) ncoeff_16;

	for (j = 0; j < height; j++)
	{
		DWORD* pIn0 = (DWORD*) (src-2);
		DWORD* pIn1 = (DWORD*) (src-1);
		DWORD* pIn2 = (DWORD*) (src);
		DWORD* pIn3 = (DWORD*) (src+1);
		DWORD* pIn4 = (DWORD*) (src+2);
		DWORD* pIn5 = (DWORD*) (src+3);

		DWORD* pOut = (DWORD*) &block[j][0];

		mm0 = _m_from_int(*pIn0);
		mm1 = _m_from_int(*pIn1);
		mm2 = _m_from_int(*pIn2);
		mm3 = _m_from_int(*pIn3);
		mm4 = _m_from_int(*pIn4);
		mm5 = _m_from_int(*pIn5);

		mm0 = _mm_unpacklo_pi8(mm0, nNull);
		mm1 = _mm_unpacklo_pi8(mm1, nNull);
		mm2 = _mm_unpacklo_pi8(mm2, nNull);
		mm3 = _mm_unpacklo_pi8(mm3, nNull);
		mm4 = _mm_unpacklo_pi8(mm4, nNull);
		mm5 = _mm_unpacklo_pi8(mm5, nNull);

		tmp = _m_psubw(_m_psllwi(_m_paddw(mm2,mm3),2),_m_paddw(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _m_paddw(_m_psllwi(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _m_paddw(_m_paddw(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _m_psrawi(_m_paddw(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _m_packuswb(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		*pOut = _m_to_int(tmp);

		src += w1;
	}

	src = pSrc + 1;

	for (j = 0; j < height; j++)
	{
		DWORD* pIn0 = (DWORD*) (src-w2);
		DWORD* pIn1 = (DWORD*) (src-w1);
		DWORD* pIn2 = (DWORD*) (src);
		DWORD* pIn3 = (DWORD*) (src+w1);
		DWORD* pIn4 = (DWORD*) (src+w2);
		DWORD* pIn5 = (DWORD*) (src+w3);

		DWORD* pOut = (DWORD*) &block[j][0];

		mm0 = _m_from_int(*pIn0);
		mm1 = _m_from_int(*pIn1);
		mm2 = _m_from_int(*pIn2);
		mm3 = _m_from_int(*pIn3);
		mm4 = _m_from_int(*pIn4);
		mm5 = _m_from_int(*pIn5);
		mm6 = _m_from_int(*pOut);

		mm0 = _mm_unpacklo_pi8(mm0, nNull);
		mm1 = _mm_unpacklo_pi8(mm1, nNull);
		mm2 = _mm_unpacklo_pi8(mm2, nNull);
		mm3 = _mm_unpacklo_pi8(mm3, nNull);
		mm4 = _mm_unpacklo_pi8(mm4, nNull);
		mm5 = _mm_unpacklo_pi8(mm5, nNull);
		mm6 = _mm_unpacklo_pi8(mm6, nNull);

		tmp = _m_psubw(_m_psllwi(_m_paddw(mm2,mm3),2),_m_paddw(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _m_paddw(_m_psllwi(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _m_paddw(_m_paddw(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _m_psrawi(_m_paddw(tmp,coeff_16),5);//(result_block+16)>>5
		mm6 = _m_packuswb(mm6,mm6);
		tmp = _m_packuswb(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		tmp = _m_pavgb(mm6,tmp);
		*pOut = _m_to_int(tmp);

		src += w1;
	}
}

// The SSE2 code of get_block_8xh_pxx
void  get_block_8xh_p00_sse(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	imgpel *pblock = &block[0][0];
	for(int i = height; i > 0; i-=4)
	{
		//*((unsigned long long*)&block[i][0]) = *((unsigned long long*)src);
		*(__m64*)(pblock+0*LUMA_BLOCK_SIZE) = *(__m64*)&pSrc[0*stride];
		*(__m64*)(pblock+1*LUMA_BLOCK_SIZE) = *(__m64*)&pSrc[1*stride];
		*(__m64*)(pblock+2*LUMA_BLOCK_SIZE) = *(__m64*)&pSrc[2*stride];
		*(__m64*)(pblock+3*LUMA_BLOCK_SIZE) = *(__m64*)&pSrc[3*stride];
		pSrc   += 4*stride;
		pblock += 4*LUMA_BLOCK_SIZE;
	}
}

void  get_block_8xh_p00_sse2(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	imgpel *pblock = &block[0][0];
	for(int i = height; i > 0; i-=4)
	{
		//*((unsigned long long*)&block[i][0]) = *((unsigned long long*)src);
		*(__m64*)(pblock+0*LUMA_BLOCK_SIZE) = *(__m64*)&pSrc[0*stride];
		*(__m64*)(pblock+1*LUMA_BLOCK_SIZE) = *(__m64*)&pSrc[1*stride];
		*(__m64*)(pblock+2*LUMA_BLOCK_SIZE) = *(__m64*)&pSrc[2*stride];
		*(__m64*)(pblock+3*LUMA_BLOCK_SIZE) = *(__m64*)&pSrc[3*stride];
		pSrc   += 4*stride;
		pblock += 4*LUMA_BLOCK_SIZE;
	}
}
void  get_block_8xh_p01_sse(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int j;
	byte *src;
	const int w1 = stride;
	const int w2 = stride + stride;
	const int w3 = w2 + w1;
	src = pSrc;

	__m64 mm0, mm1, mm2, mm3, mm4, mm5, tmp;
	const __m64 Null     = _mm_setzero_si64();
	const __m64 coeff_16 = *((__m64*)ncoeff_16);

	for (j = 0; j < height; j++)
	{
		mm0 = *((__m64*)(src-w2));
		mm1 = *((__m64*)(src-w1));
		mm2 = *((__m64*)(src));
		mm3 = *((__m64*)(src+w1));
		mm4 = *((__m64*)(src+w2));
		mm5 = *((__m64*)(src+w3));

		mm0 = _mm_unpacklo_pi8(mm0, Null);
		mm1 = _mm_unpacklo_pi8(mm1, Null);
		mm2 = _mm_unpacklo_pi8(mm2, Null);
		mm3 = _mm_unpacklo_pi8(mm3, Null);
		mm4 = _mm_unpacklo_pi8(mm4, Null);
		mm5 = _mm_unpacklo_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		mm2 = _mm_packs_pu16(mm2,mm2);
		tmp = _mm_avg_pu8(tmp,mm2);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		*((DWORD*)&block[j][0]) = _m_to_int(tmp);

		mm0 = *((__m64*)(src-w2));
		mm1 = *((__m64*)(src-w1));
		mm2 = *((__m64*)(src));
		mm3 = *((__m64*)(src+w1));
		mm4 = *((__m64*)(src+w2));
		mm5 = *((__m64*)(src+w3));

		mm0 = _mm_unpackhi_pi8(mm0, Null);
		mm1 = _mm_unpackhi_pi8(mm1, Null);
		mm2 = _mm_unpackhi_pi8(mm2, Null);
		mm3 = _mm_unpackhi_pi8(mm3, Null);
		mm4 = _mm_unpackhi_pi8(mm4, Null);
		mm5 = _mm_unpackhi_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		mm2 = _mm_packs_pu16(mm2,mm2);
		tmp = _mm_avg_pu8(tmp,mm2);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		*((DWORD*)&block[j][4]) = _m_to_int(tmp);

		src += w1;
	}
}

void  get_block_8xh_p01_sse2(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int j;
	byte *src;
	const int w1 = stride;
	const int w2 = stride + stride;
	const int w3 = w2 + w1;
	src = pSrc;

	__m128i mm0,mm1,mm2,mm3,mm4,mm5,tmp,tmp1;
	const __m128i Null     = _mm_setzero_si128();
	const __m128i coeff_16 = _mm_load_si128((__m128i*)ncoeff_16);

	mm0 = _mm_loadl_epi64(((__m128i*) (src-w2)));
	mm1 = _mm_loadl_epi64(((__m128i*) (src-w1)));
	mm2 = _mm_loadl_epi64(((__m128i*) (src)));
	mm3 = _mm_loadl_epi64(((__m128i*) (src+w1)));
	mm4 = _mm_loadl_epi64(((__m128i*) (src+w2)));
	mm5 = _mm_loadl_epi64(((__m128i*) (src+w3)));

	mm0 = _mm_unpacklo_epi8(mm0, Null);
	mm1 = _mm_unpacklo_epi8(mm1, Null);
	mm2 = _mm_unpacklo_epi8(mm2, Null);
	mm3 = _mm_unpacklo_epi8(mm3, Null);
	mm4 = _mm_unpacklo_epi8(mm4, Null);
	mm5 = _mm_unpacklo_epi8(mm5, Null);

	tmp = _mm_sub_epi16(_mm_slli_epi16(_mm_add_epi16(mm2,mm3),2),_mm_add_epi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
	tmp = _mm_add_epi16(_mm_slli_epi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
	tmp = _mm_add_epi16(_mm_add_epi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
	tmp = _mm_srai_epi16(_mm_add_epi16(tmp,coeff_16),5);//(result_block+16)>>5
	tmp = _mm_packus_epi16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
	tmp1 = _mm_packus_epi16(mm2,mm2);
	tmp = _mm_avg_epu8(tmp,tmp1);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
	_mm_storel_epi64((__m128i*)&block[0][0], tmp);

	src += w1;

	for (j = 1; j < height; j++)
	{
		mm0 = mm1;
		mm1 = mm2;
		mm2 = mm3;
		mm3 = mm4;
		mm4 = mm5;

		mm5 = _mm_loadl_epi64(((__m128i*) (src+w3)));
		mm5 = _mm_unpacklo_epi8(mm5, Null);

		tmp = _mm_sub_epi16(_mm_slli_epi16(_mm_add_epi16(mm2,mm3),2),_mm_add_epi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_slli_epi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_add_epi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_epi16(_mm_add_epi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packus_epi16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		tmp1 = _mm_packus_epi16(mm2,mm2);
		tmp = _mm_avg_epu8(tmp,tmp1);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		_mm_storel_epi64((__m128i*)&block[j][0], tmp);

		src += w1;
	}
}
void  get_block_8xh_p02_sse(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int j;
	byte *src;
	const int w1 = stride;
	const int w2 = stride + stride;
	const int w3 = w2 + w1;
	src = pSrc;

	__m64 mm0, mm1, mm2, mm3, mm4, mm5, tmp;
	const __m64 Null     = _mm_setzero_si64();
	const __m64 coeff_16 = *((__m64*)ncoeff_16);

	for (j = 0; j < height; j++)
	{
		mm0 = *((__m64*)(src-w2));
		mm1 = *((__m64*)(src-w1));
		mm2 = *((__m64*)(src));
		mm3 = *((__m64*)(src+w1));
		mm4 = *((__m64*)(src+w2));
		mm5 = *((__m64*)(src+w3));

		mm0 = _mm_unpacklo_pi8(mm0, Null);
		mm1 = _mm_unpacklo_pi8(mm1, Null);
		mm2 = _mm_unpacklo_pi8(mm2, Null);
		mm3 = _mm_unpacklo_pi8(mm3, Null);
		mm4 = _mm_unpacklo_pi8(mm4, Null);
		mm5 = _mm_unpacklo_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		*((DWORD*)&block[j][0]) = _m_to_int(tmp);

		mm0 = *((__m64*)(src-w2));
		mm1 = *((__m64*)(src-w1));
		mm2 = *((__m64*)(src));
		mm3 = *((__m64*)(src+w1));
		mm4 = *((__m64*)(src+w2));
		mm5 = *((__m64*)(src+w3));

		mm0 = _mm_unpackhi_pi8(mm0, Null);
		mm1 = _mm_unpackhi_pi8(mm1, Null);
		mm2 = _mm_unpackhi_pi8(mm2, Null);
		mm3 = _mm_unpackhi_pi8(mm3, Null);
		mm4 = _mm_unpackhi_pi8(mm4, Null);
		mm5 = _mm_unpackhi_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		*((DWORD*)&block[j][4]) = _m_to_int(tmp);

		src += w1;
	}
}

void  get_block_8xh_p02_sse2(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int j;
	byte *src;
	const int w1 = stride;
	const int w2 = stride + stride;
	const int w3 = w2 + w1;
	src = pSrc;

	__m128i mm0,mm1,mm2,mm3,mm4,mm5,tmp;
	const __m128i Null     = _mm_setzero_si128();
	const __m128i coeff_16 = _mm_load_si128((__m128i*)ncoeff_16);

	mm0 = _mm_loadl_epi64(((__m128i*) (src-w2)));
	mm1 = _mm_loadl_epi64(((__m128i*) (src-w1)));
	mm2 = _mm_loadl_epi64(((__m128i*) (src)));
	mm3 = _mm_loadl_epi64(((__m128i*) (src+w1)));
	mm4 = _mm_loadl_epi64(((__m128i*) (src+w2)));
	mm5 = _mm_loadl_epi64(((__m128i*) (src+w3)));

	mm0 = _mm_unpacklo_epi8(mm0, Null);
	mm1 = _mm_unpacklo_epi8(mm1, Null);
	mm2 = _mm_unpacklo_epi8(mm2, Null);
	mm3 = _mm_unpacklo_epi8(mm3, Null);
	mm4 = _mm_unpacklo_epi8(mm4, Null);
	mm5 = _mm_unpacklo_epi8(mm5, Null);

	tmp = _mm_sub_epi16(_mm_slli_epi16(_mm_add_epi16(mm2,mm3),2),_mm_add_epi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
	tmp = _mm_add_epi16(_mm_slli_epi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
	tmp = _mm_add_epi16(_mm_add_epi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
	tmp = _mm_srai_epi16(_mm_add_epi16(tmp,coeff_16),5);//(result_block+16)>>5
	tmp = _mm_packus_epi16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
	_mm_storel_epi64((__m128i*)&block[0][0], tmp);

	src += w1;

	for (j = 1; j < height; j++)
	{
		mm0 = mm1;
		mm1 = mm2;
		mm2 = mm3;
		mm3 = mm4;
		mm4 = mm5;

		mm5 = _mm_loadl_epi64(((__m128i*) (src+w3)));
		mm5 = _mm_unpacklo_epi8(mm5, Null);

		tmp = _mm_sub_epi16(_mm_slli_epi16(_mm_add_epi16(mm2,mm3),2),_mm_add_epi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_slli_epi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_add_epi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_epi16(_mm_add_epi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packus_epi16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		_mm_storel_epi64((__m128i*)&block[j][0], tmp);

		src += w1;
	}
}
void  get_block_8xh_p03_sse(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int j;
	byte *src;
	const int w1 = stride;
	const int w2 = stride + stride;
	const int w3 = w2 + w1;
	src = pSrc;

	__m64 mm0, mm1, mm2, mm3, mm4, mm5, tmp;
	const __m64 Null     = _mm_setzero_si64();
	const __m64 coeff_16 = *((__m64*)ncoeff_16);

	for (j = 0; j < height; j++)
	{
		mm0 = *((__m64*)(src-w2));
		mm1 = *((__m64*)(src-w1));
		mm2 = *((__m64*)(src));
		mm3 = *((__m64*)(src+w1));
		mm4 = *((__m64*)(src+w2));
		mm5 = *((__m64*)(src+w3));

		mm0 = _mm_unpacklo_pi8(mm0, Null);
		mm1 = _mm_unpacklo_pi8(mm1, Null);
		mm2 = _mm_unpacklo_pi8(mm2, Null);
		mm3 = _mm_unpacklo_pi8(mm3, Null);
		mm4 = _mm_unpacklo_pi8(mm4, Null);
		mm5 = _mm_unpacklo_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		mm3 = _mm_packs_pu16(mm3,mm3);
		tmp = _mm_avg_pu8(tmp,mm3);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		*((DWORD*)&block[j][0]) = _m_to_int(tmp);

		mm0 = *((__m64*)(src-w2));
		mm1 = *((__m64*)(src-w1));
		mm2 = *((__m64*)(src));
		mm3 = *((__m64*)(src+w1));
		mm4 = *((__m64*)(src+w2));
		mm5 = *((__m64*)(src+w3));

		mm0 = _mm_unpackhi_pi8(mm0, Null);
		mm1 = _mm_unpackhi_pi8(mm1, Null);
		mm2 = _mm_unpackhi_pi8(mm2, Null);
		mm3 = _mm_unpackhi_pi8(mm3, Null);
		mm4 = _mm_unpackhi_pi8(mm4, Null);
		mm5 = _mm_unpackhi_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		mm3 = _mm_packs_pu16(mm3,mm3);
		tmp = _mm_avg_pu8(tmp,mm3);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		*((DWORD*)&block[j][4]) = _m_to_int(tmp);

		src += w1;
	}
}

void  get_block_8xh_p03_sse2(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int j;
	byte *src;
	const int w1 = stride;
	const int w2 = stride + stride;
	const int w3 = w2 + w1;
	src = pSrc;

	__m128i mm0,mm1,mm2,mm3,mm4,mm5,tmp,tmp1;
	const __m128i Null     = _mm_setzero_si128();
	const __m128i coeff_16 = _mm_load_si128((__m128i*)ncoeff_16);

	mm0 = _mm_loadl_epi64(((__m128i*) (src-w2)));
	mm1 = _mm_loadl_epi64(((__m128i*) (src-w1)));
	mm2 = _mm_loadl_epi64(((__m128i*) (src)));
	mm3 = _mm_loadl_epi64(((__m128i*) (src+w1)));
	mm4 = _mm_loadl_epi64(((__m128i*) (src+w2)));
	mm5 = _mm_loadl_epi64(((__m128i*) (src+w3)));

	mm0 = _mm_unpacklo_epi8(mm0, Null);
	mm1 = _mm_unpacklo_epi8(mm1, Null);
	mm2 = _mm_unpacklo_epi8(mm2, Null);
	mm3 = _mm_unpacklo_epi8(mm3, Null);
	mm4 = _mm_unpacklo_epi8(mm4, Null);
	mm5 = _mm_unpacklo_epi8(mm5, Null);

	tmp = _mm_sub_epi16(_mm_slli_epi16(_mm_add_epi16(mm2,mm3),2),_mm_add_epi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
	tmp = _mm_add_epi16(_mm_slli_epi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
	tmp = _mm_add_epi16(_mm_add_epi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
	tmp = _mm_srai_epi16(_mm_add_epi16(tmp,coeff_16),5);//(result_block+16)>>5
	tmp = _mm_packus_epi16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
	tmp1 = _mm_packus_epi16(mm3,mm3);
	tmp = _mm_avg_epu8(tmp,tmp1);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
	_mm_storel_epi64((__m128i*)&block[0][0], tmp);

	src += w1;

	for (j = 1; j < height; j++)
	{
		mm0 = mm1;
		mm1 = mm2;
		mm2 = mm3;
		mm3 = mm4;
		mm4 = mm5;

		mm5 = _mm_loadl_epi64(((__m128i*) (src+w3)));
		mm5 = _mm_unpacklo_epi8(mm5, Null);

		tmp = _mm_sub_epi16(_mm_slli_epi16(_mm_add_epi16(mm2,mm3),2),_mm_add_epi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_slli_epi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_add_epi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_epi16(_mm_add_epi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packus_epi16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		tmp1 = _mm_packus_epi16(mm3,mm3);
		tmp = _mm_avg_epu8(tmp,tmp1);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		_mm_storel_epi64((__m128i*)&block[j][0], tmp);

		src += w1;
	}
}

void  get_block_8xh_p10_sse(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int j;
	byte *src;
	const int w1 = stride;
	src = pSrc;

	__m64 mm0, mm1, mm2, mm3, mm4, mm5, tmp, tmp1;
	const __m64 Null     = _mm_setzero_si64();
	const __m64 coeff_16 = *((__m64*)ncoeff_16);

	for (j = 0; j < height; j++)
	{
		mm0 = *((__m64*)(src-2));
		mm1 = *((__m64*)(src-1));
		mm2 = *((__m64*)(src));
		mm3 = *((__m64*)(src+1));
		mm4 = *((__m64*)(src+2));
		mm5 = *((__m64*)(src+3));

		mm0 = _mm_unpacklo_pi8(mm0, Null);
		mm1 = _mm_unpacklo_pi8(mm1, Null);
		mm2 = _mm_unpacklo_pi8(mm2, Null);
		mm3 = _mm_unpacklo_pi8(mm3, Null);
		mm4 = _mm_unpacklo_pi8(mm4, Null);
		mm5 = _mm_unpacklo_pi8(mm5, Null);
		*((__m64*)&block[j][0]) = mm2;

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		
		mm0 = *((__m64*)(src-2));
		mm1 = *((__m64*)(src-1));
		mm2 = *((__m64*)(src));
		mm3 = *((__m64*)(src+1));
		mm4 = *((__m64*)(src+2));
		mm5 = *((__m64*)(src+3));

		mm0 = _mm_unpackhi_pi8(mm0, Null);
		mm1 = _mm_unpackhi_pi8(mm1, Null);
		mm2 = _mm_unpackhi_pi8(mm2, Null);
		mm3 = _mm_unpackhi_pi8(mm3, Null);
		mm4 = _mm_unpackhi_pi8(mm4, Null);
		mm5 = _mm_unpackhi_pi8(mm5, Null);

		tmp1 = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_pi16(_mm_slli_pi16(tmp1,2),tmp1);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp1); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp1 = _mm_srai_pi16(_mm_add_pi16(tmp1,coeff_16),5);//(result_block+16)>>5
		
		tmp = _mm_packs_pu16(tmp,tmp1);//__fast_iclip0_255(((result+16)>>5))
		mm4 = *((__m64*)&block[j][0]);
		mm3 = _mm_packs_pu16(mm4,mm2);
		tmp = _mm_avg_pu8(tmp,mm3);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		*((__m64*)&block[j][0]) = tmp;
		
		src += w1;
	}
}
void  get_block_8xh_p10_sse2(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int j;
	byte *src;
	const int w1 = stride;
	src = pSrc;

	__m128i mm0,mm1,mm2,mm3,mm4,mm5,tmp;
	const __m128i Null     = _mm_setzero_si128();
	const __m128i coeff_16 = _mm_load_si128((__m128i*)ncoeff_16);

	for (j = 0; j < height; j++)
	{
		mm0 = _mm_loadl_epi64(((__m128i*) (src-2)));
		mm1 = _mm_loadl_epi64(((__m128i*) (src-1)));
		mm2 = _mm_loadl_epi64(((__m128i*) (src)));
		mm3 = _mm_loadl_epi64(((__m128i*) (src+1)));
		mm4 = _mm_loadl_epi64(((__m128i*) (src+2)));
		mm5 = _mm_loadl_epi64(((__m128i*) (src+3)));

		mm0 = _mm_unpacklo_epi8(mm0, Null);
		mm1 = _mm_unpacklo_epi8(mm1, Null);
		mm2 = _mm_unpacklo_epi8(mm2, Null);
		mm3 = _mm_unpacklo_epi8(mm3, Null);
		mm4 = _mm_unpacklo_epi8(mm4, Null);
		mm5 = _mm_unpacklo_epi8(mm5, Null);

		tmp = _mm_sub_epi16(_mm_slli_epi16(_mm_add_epi16(mm2,mm3),2),_mm_add_epi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_slli_epi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_add_epi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_epi16(_mm_add_epi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packus_epi16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		mm2 = _mm_packus_epi16(mm2,mm2);
		tmp = _mm_avg_epu8(tmp,mm2);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		_mm_storel_epi64((__m128i*)&block[j][0], tmp);

		src += w1;
	}
}

void  get_block_8xh_p11_sse(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int j;
	byte *src;
	const int w1 = stride;
	const int w2 = stride + stride;
	const int w3 = w2 + w1;

	src = pSrc;

	__m64 mm0, mm1, mm2, mm3, mm4, mm5, mm6, tmp;
	const __m64 Null     = _mm_setzero_si64();
	const __m64 coeff_16 = *((__m64*)ncoeff_16);

	for (j = 0; j < height; j++)
	{
		mm0 = *((__m64*)(src-2));
		mm1 = *((__m64*)(src-1));
		mm2 = *((__m64*)(src));
		mm3 = *((__m64*)(src+1));
		mm4 = *((__m64*)(src+2));
		mm5 = *((__m64*)(src+3));

		mm0 = _mm_unpacklo_pi8(mm0, Null);
		mm1 = _mm_unpacklo_pi8(mm1, Null);
		mm2 = _mm_unpacklo_pi8(mm2, Null);
		mm3 = _mm_unpacklo_pi8(mm3, Null);
		mm4 = _mm_unpacklo_pi8(mm4, Null);
		mm5 = _mm_unpacklo_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		*((DWORD*)&block[j][0]) = _m_to_int(tmp);

		mm0 = *((__m64*)(src-2));
		mm1 = *((__m64*)(src-1));
		mm2 = *((__m64*)(src));
		mm3 = *((__m64*)(src+1));
		mm4 = *((__m64*)(src+2));
		mm5 = *((__m64*)(src+3));

		mm0 = _mm_unpackhi_pi8(mm0, Null);
		mm1 = _mm_unpackhi_pi8(mm1, Null);
		mm2 = _mm_unpackhi_pi8(mm2, Null);
		mm3 = _mm_unpackhi_pi8(mm3, Null);
		mm4 = _mm_unpackhi_pi8(mm4, Null);
		mm5 = _mm_unpackhi_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		*((DWORD*)&block[j][4]) = _m_to_int(tmp);


		src += w1;
	}

	src = pSrc;

	for (j = 0; j < height; j++)
	{
		mm0 = *((__m64*)(src-w2));
		mm1 = *((__m64*)(src-w1));
		mm2 = *((__m64*)(src));
		mm3 = *((__m64*)(src+w1));
		mm4 = *((__m64*)(src+w2));
		mm5 = *((__m64*)(src+w3));
		mm6 = *((__m64*)&block[j][0]);

		mm0 = _mm_unpacklo_pi8(mm0, Null);
		mm1 = _mm_unpacklo_pi8(mm1, Null);
		mm2 = _mm_unpacklo_pi8(mm2, Null);
		mm3 = _mm_unpacklo_pi8(mm3, Null);
		mm4 = _mm_unpacklo_pi8(mm4, Null);
		mm5 = _mm_unpacklo_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		tmp = _mm_avg_pu8(tmp,mm6);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		*((DWORD*)&block[j][0]) = _m_to_int(tmp);

		mm0 = *((__m64*)(src-w2));
		mm1 = *((__m64*)(src-w1));
		mm2 = *((__m64*)(src));
		mm3 = *((__m64*)(src+w1));
		mm4 = *((__m64*)(src+w2));
		mm5 = *((__m64*)(src+w3));
		mm6 = *((__m64*)&block[j][4]);

		mm0 = _mm_unpackhi_pi8(mm0, Null);
		mm1 = _mm_unpackhi_pi8(mm1, Null);
		mm2 = _mm_unpackhi_pi8(mm2, Null);
		mm3 = _mm_unpackhi_pi8(mm3, Null);
		mm4 = _mm_unpackhi_pi8(mm4, Null);
		mm5 = _mm_unpackhi_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		tmp = _mm_avg_pu8(tmp,mm6);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		*((DWORD*)&block[j][4]) = _m_to_int(tmp);

		src += w1;
	}
}

void  get_block_8xh_p11_sse2(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int j;
	byte *src;
	const int w1 = stride;
	const int w2 = stride + stride;
	const int w3 = w2 + w1;

	src = pSrc;

	__m128i mm0,mm1,mm2,mm3,mm4,mm5,mm6,tmp;
	const __m128i Null     = _mm_setzero_si128();
	const __m128i coeff_16 = _mm_load_si128((__m128i*)ncoeff_16);

	for (j = 0; j < height; j++)
	{
		mm0 = _mm_loadl_epi64(((__m128i*) (src-2)));
		mm1 = _mm_loadl_epi64(((__m128i*) (src-1)));
		mm2 = _mm_loadl_epi64(((__m128i*) (src)));
		mm3 = _mm_loadl_epi64(((__m128i*) (src+1)));
		mm4 = _mm_loadl_epi64(((__m128i*) (src+2)));
		mm5 = _mm_loadl_epi64(((__m128i*) (src+3)));

		mm0 = _mm_unpacklo_epi8(mm0, Null);
		mm1 = _mm_unpacklo_epi8(mm1, Null);
		mm2 = _mm_unpacklo_epi8(mm2, Null);
		mm3 = _mm_unpacklo_epi8(mm3, Null);
		mm4 = _mm_unpacklo_epi8(mm4, Null);
		mm5 = _mm_unpacklo_epi8(mm5, Null);

		tmp = _mm_sub_epi16(_mm_slli_epi16(_mm_add_epi16(mm2,mm3),2),_mm_add_epi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_slli_epi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_add_epi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_epi16(_mm_add_epi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packus_epi16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		_mm_storel_epi64((__m128i*)&block[j][0], tmp);

		src += w1;
	}

	src = pSrc;

	mm0 = _mm_loadl_epi64(((__m128i*) (src-w2)));
	mm1 = _mm_loadl_epi64(((__m128i*) (src-w1)));
	mm2 = _mm_loadl_epi64(((__m128i*) (src)));
	mm3 = _mm_loadl_epi64(((__m128i*) (src+w1)));
	mm4 = _mm_loadl_epi64(((__m128i*) (src+w2)));
	mm5 = _mm_loadl_epi64(((__m128i*) (src+w3)));
	mm6 = _mm_loadl_epi64(((__m128i*)&block[0][0]));

	mm0 = _mm_unpacklo_epi8(mm0, Null);
	mm1 = _mm_unpacklo_epi8(mm1, Null);
	mm2 = _mm_unpacklo_epi8(mm2, Null);
	mm3 = _mm_unpacklo_epi8(mm3, Null);
	mm4 = _mm_unpacklo_epi8(mm4, Null);
	mm5 = _mm_unpacklo_epi8(mm5, Null);

	tmp = _mm_sub_epi16(_mm_slli_epi16(_mm_add_epi16(mm2,mm3),2),_mm_add_epi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
	tmp = _mm_add_epi16(_mm_slli_epi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
	tmp = _mm_add_epi16(_mm_add_epi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
	tmp = _mm_srai_epi16(_mm_add_epi16(tmp,coeff_16),5);//(result_block+16)>>5
	tmp = _mm_packus_epi16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
	tmp = _mm_avg_epu8(tmp,mm6);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
	_mm_storel_epi64((__m128i*)&block[0][0], tmp);

	src += w1;

	for (j = 1; j < height; j++)
	{

		mm0 = mm1;
		mm1 = mm2;
		mm2 = mm3;
		mm3 = mm4;
		mm4 = mm5;

		mm5 = _mm_loadl_epi64(((__m128i*) (src+w3)));
		mm5 = _mm_unpacklo_epi8(mm5, Null);

		mm6 = _mm_loadl_epi64(((__m128i*)&block[j][0]));

		tmp = _mm_sub_epi16(_mm_slli_epi16(_mm_add_epi16(mm2,mm3),2),_mm_add_epi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_slli_epi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_add_epi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_epi16(_mm_add_epi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packus_epi16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		tmp = _mm_avg_epu8(tmp,mm6);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		_mm_storel_epi64((__m128i*)&block[j][0], tmp);

		src += w1;
	}
}
void  get_block_8xh_p12_sse(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	byte *src;
	short *tmp_src;
	short *luma_src;
	int i,j;
	DO_ALIGN(16,short luma_res[16][16]);
	DO_ALIGN(16,short tmp_res[16][16]);
	const int w1 = stride;
	const int w2 = stride + stride;
	const int w3 = w2 + w1;
	src = pSrc;

	__m64 pIn0, pIn1;
	__m64 mm0, mm1, mm2, mm3, mm4, mm5, tmp, tmp1;
	const __m64 Null     = _mm_setzero_si64();
	const __m64 coeff_16 = *((__m64*)ncoeff_16);
	const __m64 coeff_512 = *((__m64*)ncoeff_512);

	for (j = 0; j < height; j++)
	{
		for(i = 0; i < 10; i += 8)
		{
			mm0 = *((__m64*)(src-w2-2+i));
			mm1 = *((__m64*)(src-w1-2+i));
			mm2 = *((__m64*)(src-2+i));
			mm3 = *((__m64*)(src+w1-2+i));
			mm4 = *((__m64*)(src+w2-2+i));
			mm5 = *((__m64*)(src+w3-2+i));

			mm0 = _mm_unpacklo_pi8(mm0, Null);
			mm1 = _mm_unpacklo_pi8(mm1, Null);
			mm2 = _mm_unpacklo_pi8(mm2, Null);
			mm3 = _mm_unpacklo_pi8(mm3, Null);
			mm4 = _mm_unpacklo_pi8(mm4, Null);
			mm5 = _mm_unpacklo_pi8(mm5, Null);

			tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_adds_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
			tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
			tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5

			tmp1 = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
			*((__m64*)&tmp_res[j][i]) = tmp;
			*((__m64*)&luma_res[j][i]) = tmp1;
			
			mm0 = *((__m64*)(src-w2-2+i));
			mm1 = *((__m64*)(src-w1-2+i));
			mm2 = *((__m64*)(src-2+i));
			mm3 = *((__m64*)(src+w1-2+i));
			mm4 = *((__m64*)(src+w2-2+i));
			mm5 = *((__m64*)(src+w3-2+i));

			mm0 = _mm_unpackhi_pi8(mm0, Null);
			mm1 = _mm_unpackhi_pi8(mm1, Null);
			mm2 = _mm_unpackhi_pi8(mm2, Null);
			mm3 = _mm_unpackhi_pi8(mm3, Null);
			mm4 = _mm_unpackhi_pi8(mm4, Null);
			mm5 = _mm_unpackhi_pi8(mm5, Null);

			tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_adds_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
			tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
			tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5

			tmp1 = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
			*((__m64*)&tmp_res[j][i+4]) = tmp;
			*((__m64*)&luma_res[j][i+4]) = tmp1;


		}
		src += w1;
	}

	tmp_src = &tmp_res[0][0];
	luma_src = &luma_res[0][2];
	for (j = 0; j < height; j++)
	{
		mm0 = *((__m64*)(tmp_src));
		mm1 = *((__m64*)(tmp_src+1));
		mm2 = *((__m64*)(tmp_src+2));
		mm3 = *((__m64*)(tmp_src+3));
		mm4 = *((__m64*)(tmp_src+4));
		mm5 = *((__m64*)(tmp_src+5));

		mm0 = _mm_unpacklo_pi16(Null, mm0);
		mm1 = _mm_unpacklo_pi16(Null, mm1);
		mm2 = _mm_unpacklo_pi16(Null, mm2);
		mm3 = _mm_unpacklo_pi16(Null, mm3);
		mm4 = _mm_unpacklo_pi16(Null, mm4);
		mm5 = _mm_unpacklo_pi16(Null, mm5);

		mm0 = _mm_srai_pi32(mm0,16);
		mm1 = _mm_srai_pi32(mm1,16);
		mm2 = _mm_srai_pi32(mm2,16);
		mm3 = _mm_srai_pi32(mm3,16);
		mm4 = _mm_srai_pi32(mm4,16);
		mm5 = _mm_srai_pi32(mm5,16);

		tmp = _mm_sub_pi32(_mm_slli_pi32(_mm_add_pi32(mm2,mm3),2),_mm_add_pi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi32(_mm_slli_pi32(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi32(_mm_add_pi32(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi32(_mm_add_pi32(tmp,coeff_512),10);//(result_block+512)>>10

		mm0 = mm4;
		mm1 = mm5;
		mm2 = *((__m64*)(tmp_src+6));
		mm3 = *((__m64*)(tmp_src+7));
		mm4 = *((__m64*)(tmp_src+8));
		mm5 = *((__m64*)(tmp_src+9));

		mm2 = _mm_unpacklo_pi16(Null, mm2);
		mm3 = _mm_unpacklo_pi16(Null, mm3);
		mm4 = _mm_unpacklo_pi16(Null, mm4);
		mm5 = _mm_unpacklo_pi16(Null, mm5);

		mm2 = _mm_srai_pi32(mm2,16);
		mm3 = _mm_srai_pi32(mm3,16);
		mm4 = _mm_srai_pi32(mm4,16);
		mm5 = _mm_srai_pi32(mm5,16);

		tmp1 = _mm_sub_pi32(_mm_slli_pi32(_mm_add_pi32(mm2,mm3),2),_mm_add_pi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_pi32(_mm_slli_pi32(tmp1,2),tmp1);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_pi32(_mm_add_pi32(mm0,mm5),tmp1); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp1 = _mm_srai_pi32(_mm_add_pi32(tmp1,coeff_512),10);//(result_block+512)>>10

		tmp = _mm_packs_pi32(tmp,tmp1);
		*((__m64*)&block[j][0]) = tmp;

		mm0 = *((__m64*)(tmp_src));
		mm1 = *((__m64*)(tmp_src+1));
		mm2 = *((__m64*)(tmp_src+2));
		mm3 = *((__m64*)(tmp_src+3));
		mm4 = *((__m64*)(tmp_src+4));
		mm5 = *((__m64*)(tmp_src+5));

		mm0 = _mm_unpackhi_pi16(Null, mm0);
		mm1 = _mm_unpackhi_pi16(Null, mm1);
		mm2 = _mm_unpackhi_pi16(Null, mm2);
		mm3 = _mm_unpackhi_pi16(Null, mm3);
		mm4 = _mm_unpackhi_pi16(Null, mm4);
		mm5 = _mm_unpackhi_pi16(Null, mm5);

		mm0 = _mm_srai_pi32(mm0,16);
		mm1 = _mm_srai_pi32(mm1,16);
		mm2 = _mm_srai_pi32(mm2,16);
		mm3 = _mm_srai_pi32(mm3,16);
		mm4 = _mm_srai_pi32(mm4,16);
		mm5 = _mm_srai_pi32(mm5,16);

		tmp = _mm_sub_pi32(_mm_slli_pi32(_mm_add_pi32(mm2,mm3),2),_mm_add_pi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi32(_mm_slli_pi32(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi32(_mm_add_pi32(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi32(_mm_add_pi32(tmp,coeff_512),10);//(result_block+512)>>10

		mm0 = mm4;
		mm1 = mm5;
		mm2 = *((__m64*)(tmp_src+6));
		mm3 = *((__m64*)(tmp_src+7));
		mm4 = *((__m64*)(tmp_src+8));
		mm5 = *((__m64*)(tmp_src+9));

		mm2 = _mm_unpackhi_pi16(Null, mm2);
		mm3 = _mm_unpackhi_pi16(Null, mm3);
		mm4 = _mm_unpackhi_pi16(Null, mm4);
		mm5 = _mm_unpackhi_pi16(Null, mm5);

		mm2 = _mm_srai_pi32(mm2,16);
		mm3 = _mm_srai_pi32(mm3,16);
		mm4 = _mm_srai_pi32(mm4,16);
		mm5 = _mm_srai_pi32(mm5,16);

		tmp1 = _mm_sub_pi32(_mm_slli_pi32(_mm_add_pi32(mm2,mm3),2),_mm_add_pi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_pi32(_mm_slli_pi32(tmp1,2),tmp1);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_pi32(_mm_add_pi32(mm0,mm5),tmp1); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp1 = _mm_srai_pi32(_mm_add_pi32(tmp1,coeff_512),10);//(result_block+512)>>10

		tmp = _mm_packs_pi32(tmp,tmp1);
		tmp1 = *((__m64*)&block[j][0]);
		mm4 = _mm_unpacklo_pi32(tmp1, tmp);
		mm5 = _mm_unpackhi_pi32(tmp1, tmp);

		pIn0 = *((__m64*) (luma_src));
		pIn1 = *((__m64*) (luma_src+4));
		tmp = _mm_packs_pu16(mm4,mm5);//__fast_iclip0_255((result_block+512)>>10)
		mm0 = _mm_packs_pu16(pIn0,pIn1);
		tmp = _mm_avg_pu8(tmp,mm0);
		*((__m64*)&block[j][0]) = tmp;

		tmp_src += 16;
		luma_src += 16;
	}
}
void  get_block_8xh_p12_sse2(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	byte *src;
	short *tmp_src;
	short *luma_src;
	int i,j;
	DO_ALIGN(16,short luma_res[16][16]);
	DO_ALIGN(16,short tmp_res[16][16]);
	const int w1 = stride;
	const int w2 = stride + stride;
	const int w3 = w2 + w1;
	src = pSrc;

	__m64 pIn0, pIn1;
	__m128i mm0,mm1,mm2,mm3,mm4,mm5,tmp,tmp1;
	const __m128i Null      = _mm_setzero_si128();
	const __m128i coeff_16  = _mm_load_si128((__m128i*)ncoeff_16);
	const __m128i coeff_512 = _mm_load_si128((__m128i*)ncoeff_512);

	for (j = 0; j < height; j++)
	{
		for(i = 0; i < 10; i += 8)
		{
			mm0 = _mm_loadl_epi64(((__m128i*) (src-w2-2+i)));
			mm1 = _mm_loadl_epi64(((__m128i*) (src-w1-2+i)));
			mm2 = _mm_loadl_epi64(((__m128i*) (src-2+i)));
			mm3 = _mm_loadl_epi64(((__m128i*) (src+w1-2+i)));
			mm4 = _mm_loadl_epi64(((__m128i*) (src+w2-2+i)));
			mm5 = _mm_loadl_epi64(((__m128i*) (src+w3-2+i)));

			mm0 = _mm_unpacklo_epi8(mm0, Null);
			mm1 = _mm_unpacklo_epi8(mm1, Null);
			mm2 = _mm_unpacklo_epi8(mm2, Null);
			mm3 = _mm_unpacklo_epi8(mm3, Null);
			mm4 = _mm_unpacklo_epi8(mm4, Null);
			mm5 = _mm_unpacklo_epi8(mm5, Null);

			tmp = _mm_sub_epi16(_mm_slli_epi16(_mm_add_epi16(mm2,mm3),2),_mm_adds_epi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
			tmp = _mm_add_epi16(_mm_slli_epi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
			tmp = _mm_add_epi16(_mm_add_epi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5

			_mm_store_si128(((__m128i*)&tmp_res[j][i]),tmp);

			tmp = _mm_srai_epi16(_mm_add_epi16(tmp,coeff_16),5);//(result_block+16)>>5

			_mm_store_si128(((__m128i*)&luma_res[j][i]),tmp);

		}
		src += w1;
	}

	tmp_src = &tmp_res[0][0];
	luma_src = &luma_res[0][2];
	for (j = 0; j < height; j++)
	{
		mm0 = _mm_loadl_epi64(((__m128i*) (tmp_src)));
		mm1 = _mm_loadl_epi64(((__m128i*) (tmp_src+1)));
		mm2 = _mm_loadl_epi64(((__m128i*) (tmp_src+2)));
		mm3 = _mm_loadl_epi64(((__m128i*) (tmp_src+3)));
		mm4 = _mm_loadl_epi64(((__m128i*) (tmp_src+4)));
		mm5 = _mm_loadl_epi64(((__m128i*) (tmp_src+5)));

		mm0 = _mm_unpacklo_epi16(Null, mm0);
		mm1 = _mm_unpacklo_epi16(Null, mm1);
		mm2 = _mm_unpacklo_epi16(Null, mm2);
		mm3 = _mm_unpacklo_epi16(Null, mm3);
		mm4 = _mm_unpacklo_epi16(Null, mm4);
		mm5 = _mm_unpacklo_epi16(Null, mm5);

		mm0 = _mm_srai_epi32(mm0,16);
		mm1 = _mm_srai_epi32(mm1,16);
		mm2 = _mm_srai_epi32(mm2,16);
		mm3 = _mm_srai_epi32(mm3,16);
		mm4 = _mm_srai_epi32(mm4,16);
		mm5 = _mm_srai_epi32(mm5,16);

		tmp = _mm_sub_epi32(_mm_slli_epi32(_mm_add_epi32(mm2,mm3),2),_mm_add_epi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi32(_mm_slli_epi32(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi32(_mm_add_epi32(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_epi32(_mm_add_epi32(tmp,coeff_512),10);//(result_block+512)>>10

		mm0 = mm4;
		mm1 = mm5;
		mm2 = _mm_loadl_epi64(((__m128i*) (tmp_src+6)));
		mm3 = _mm_loadl_epi64(((__m128i*) (tmp_src+7)));
		mm4 = _mm_loadl_epi64(((__m128i*) (tmp_src+8)));
		mm5 = _mm_loadl_epi64(((__m128i*) (tmp_src+9)));

		mm2 = _mm_unpacklo_epi16(Null, mm2);
		mm3 = _mm_unpacklo_epi16(Null, mm3);
		mm4 = _mm_unpacklo_epi16(Null, mm4);
		mm5 = _mm_unpacklo_epi16(Null, mm5);

		mm2 = _mm_srai_epi32(mm2,16);
		mm3 = _mm_srai_epi32(mm3,16);
		mm4 = _mm_srai_epi32(mm4,16);
		mm5 = _mm_srai_epi32(mm5,16);

		tmp1 = _mm_sub_epi32(_mm_slli_epi32(_mm_add_epi32(mm2,mm3),2),_mm_add_epi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_epi32(_mm_slli_epi32(tmp1,2),tmp1);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_epi32(_mm_add_epi32(mm0,mm5),tmp1); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp1 = _mm_srai_epi32(_mm_add_epi32(tmp1,coeff_512),10);//(result_block+512)>>10

		tmp = _mm_packs_epi32(tmp,tmp1);

		pIn0 = *((__m64*) (luma_src));
		pIn1 = *((__m64*) (luma_src+4));
		mm0 = _mm_setr_epi64(pIn0,pIn1);
		tmp = _mm_packus_epi16(tmp,tmp);//__fast_iclip0_255((result_block+512)>>10)
		mm0 = _mm_packus_epi16(mm0,mm0);
		tmp = _mm_avg_epu8(tmp,mm0);

		_mm_storel_epi64((__m128i*)&block[j][0], tmp);

		tmp_src += 16;
		luma_src += 16;
	}
}
void  get_block_8xh_p13_sse(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int j;
	byte *src;
	const int w1 = stride;
	const int w2 = stride + stride;
	const int w3 = w2 + w1;

	src = pSrc+w1;

	__m64 mm0, mm1, mm2, mm3, mm4, mm5, mm6, tmp;
	const __m64 Null     = _mm_setzero_si64();
	const __m64 coeff_16 = *((__m64*)ncoeff_16);

	for (j = 0; j < height; j++)
	{
		mm0 = *((__m64*)(src-2));
		mm1 = *((__m64*)(src-1));
		mm2 = *((__m64*)(src));
		mm3 = *((__m64*)(src+1));
		mm4 = *((__m64*)(src+2));
		mm5 = *((__m64*)(src+3));

		mm0 = _mm_unpacklo_pi8(mm0, Null);
		mm1 = _mm_unpacklo_pi8(mm1, Null);
		mm2 = _mm_unpacklo_pi8(mm2, Null);
		mm3 = _mm_unpacklo_pi8(mm3, Null);
		mm4 = _mm_unpacklo_pi8(mm4, Null);
		mm5 = _mm_unpacklo_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		*((DWORD*)&block[j][0]) = _m_to_int(tmp);
		
		mm0 = *((__m64*)(src-2));
		mm1 = *((__m64*)(src-1));
		mm2 = *((__m64*)(src));
		mm3 = *((__m64*)(src+1));
		mm4 = *((__m64*)(src+2));
		mm5 = *((__m64*)(src+3));

		mm0 = _mm_unpackhi_pi8(mm0, Null);
		mm1 = _mm_unpackhi_pi8(mm1, Null);
		mm2 = _mm_unpackhi_pi8(mm2, Null);
		mm3 = _mm_unpackhi_pi8(mm3, Null);
		mm4 = _mm_unpackhi_pi8(mm4, Null);
		mm5 = _mm_unpackhi_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		*((DWORD*)&block[j][4]) = _m_to_int(tmp);

		src += w1;
	}

	src = pSrc;

	for (j = 0; j < height; j++)
	{
		mm0 = *((__m64*)(src-w2));
		mm1 = *((__m64*)(src-w1));
		mm2 = *((__m64*)(src));
		mm3 = *((__m64*)(src+w1));
		mm4 = *((__m64*)(src+w2));
		mm5 = *((__m64*)(src+w3));
		mm6 = *((__m64*)&block[j][0]);

		mm0 = _mm_unpacklo_pi8(mm0, Null);
		mm1 = _mm_unpacklo_pi8(mm1, Null);
		mm2 = _mm_unpacklo_pi8(mm2, Null);
		mm3 = _mm_unpacklo_pi8(mm3, Null);
		mm4 = _mm_unpacklo_pi8(mm4, Null);
		mm5 = _mm_unpacklo_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		tmp = _mm_avg_pu8(tmp,mm6);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		*((DWORD*)&block[j][0]) = _m_to_int(tmp);

		mm0 = *((__m64*)(src-w2));
		mm1 = *((__m64*)(src-w1));
		mm2 = *((__m64*)(src));
		mm3 = *((__m64*)(src+w1));
		mm4 = *((__m64*)(src+w2));
		mm5 = *((__m64*)(src+w3));
		mm6 = *((__m64*)&block[j][4]);

		mm0 = _mm_unpackhi_pi8(mm0, Null);
		mm1 = _mm_unpackhi_pi8(mm1, Null);
		mm2 = _mm_unpackhi_pi8(mm2, Null);
		mm3 = _mm_unpackhi_pi8(mm3, Null);
		mm4 = _mm_unpackhi_pi8(mm4, Null);
		mm5 = _mm_unpackhi_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		tmp = _mm_avg_pu8(tmp,mm6);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		*((DWORD*)&block[j][4]) = _m_to_int(tmp);

		src += w1;
	}
}

void  get_block_8xh_p13_sse2(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int j;
	byte *src;
	const int w1 = stride;
	const int w2 = stride + stride;
	const int w3 = w2 + w1;

	src = pSrc+w1;

	__m128i mm0,mm1,mm2,mm3,mm4,mm5,mm6,tmp;
	const __m128i Null     = _mm_setzero_si128();
	const __m128i coeff_16 = _mm_load_si128((__m128i*)ncoeff_16);

	for (j = 0; j < height; j++)
	{
		mm0 = _mm_loadl_epi64(((__m128i*) (src-2)));
		mm1 = _mm_loadl_epi64(((__m128i*) (src-1)));
		mm2 = _mm_loadl_epi64(((__m128i*) (src)));
		mm3 = _mm_loadl_epi64(((__m128i*) (src+1)));
		mm4 = _mm_loadl_epi64(((__m128i*) (src+2)));
		mm5 = _mm_loadl_epi64(((__m128i*) (src+3)));

		mm0 = _mm_unpacklo_epi8(mm0, Null);
		mm1 = _mm_unpacklo_epi8(mm1, Null);
		mm2 = _mm_unpacklo_epi8(mm2, Null);
		mm3 = _mm_unpacklo_epi8(mm3, Null);
		mm4 = _mm_unpacklo_epi8(mm4, Null);
		mm5 = _mm_unpacklo_epi8(mm5, Null);

		tmp = _mm_sub_epi16(_mm_slli_epi16(_mm_add_epi16(mm2,mm3),2),_mm_add_epi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_slli_epi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_add_epi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_epi16(_mm_add_epi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packus_epi16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		_mm_storel_epi64((__m128i*)&block[j][0], tmp);

		src += w1;
	}

	src = pSrc;

	mm0 = _mm_loadl_epi64(((__m128i*) (src-w2)));
	mm1 = _mm_loadl_epi64(((__m128i*) (src-w1)));
	mm2 = _mm_loadl_epi64(((__m128i*) (src)));
	mm3 = _mm_loadl_epi64(((__m128i*) (src+w1)));
	mm4 = _mm_loadl_epi64(((__m128i*) (src+w2)));
	mm5 = _mm_loadl_epi64(((__m128i*) (src+w3)));
	mm6 = _mm_loadl_epi64(((__m128i*)&block[0][0]));

	mm0 = _mm_unpacklo_epi8(mm0, Null);
	mm1 = _mm_unpacklo_epi8(mm1, Null);
	mm2 = _mm_unpacklo_epi8(mm2, Null);
	mm3 = _mm_unpacklo_epi8(mm3, Null);
	mm4 = _mm_unpacklo_epi8(mm4, Null);
	mm5 = _mm_unpacklo_epi8(mm5, Null);

	tmp = _mm_sub_epi16(_mm_slli_epi16(_mm_add_epi16(mm2,mm3),2),_mm_add_epi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
	tmp = _mm_add_epi16(_mm_slli_epi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
	tmp = _mm_add_epi16(_mm_add_epi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
	tmp = _mm_srai_epi16(_mm_add_epi16(tmp,coeff_16),5);//(result_block+16)>>5
	tmp = _mm_packus_epi16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
	tmp = _mm_avg_epu8(tmp,mm6);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
	_mm_storel_epi64((__m128i*)&block[0][0], tmp);

	src += w1;

	for (j = 1; j < height; j++)
	{
		mm0 = mm1;
		mm1 = mm2;
		mm2 = mm3;
		mm3 = mm4;
		mm4 = mm5;

		mm5 = _mm_loadl_epi64(((__m128i*) (src+w3)));
		mm5 = _mm_unpacklo_epi8(mm5, Null);

		mm6 = _mm_loadl_epi64(((__m128i*)&block[j][0]));

		tmp = _mm_sub_epi16(_mm_slli_epi16(_mm_add_epi16(mm2,mm3),2),_mm_add_epi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_slli_epi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_add_epi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_epi16(_mm_add_epi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packus_epi16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		tmp = _mm_avg_epu8(tmp,mm6);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		_mm_storel_epi64((__m128i*)&block[j][0], tmp);

		src += w1;
	}
}
void  get_block_8xh_p20_sse(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int j;
	byte *src;
	const int w1 = stride;

	src = pSrc;

	__m64 mm0, mm1, mm2, mm3, mm4, mm5, tmp;
	const __m64 Null     = _mm_setzero_si64();
	const __m64 coeff_16 = *((__m64*)ncoeff_16);

	for (j = 0; j < height; j++)
	{
		mm0 = *((__m64*)(src-2));
		mm1 = *((__m64*)(src-1));
		mm2 = *((__m64*)(src));
		mm3 = *((__m64*)(src+1));
		mm4 = *((__m64*)(src+2));
		mm5 = *((__m64*)(src+3));

		mm0 = _mm_unpacklo_pi8(mm0, Null);
		mm1 = _mm_unpacklo_pi8(mm1, Null);
		mm2 = _mm_unpacklo_pi8(mm2, Null);
		mm3 = _mm_unpacklo_pi8(mm3, Null);
		mm4 = _mm_unpacklo_pi8(mm4, Null);
		mm5 = _mm_unpacklo_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		*((DWORD*)&block[j][0]) = _m_to_int(tmp);
		
		mm0 = *((__m64*)(src-2));
		mm1 = *((__m64*)(src-1));
		mm2 = *((__m64*)(src));
		mm3 = *((__m64*)(src+1));
		mm4 = *((__m64*)(src+2));
		mm5 = *((__m64*)(src+3));

		mm0 = _mm_unpackhi_pi8(mm0, Null);
		mm1 = _mm_unpackhi_pi8(mm1, Null);
		mm2 = _mm_unpackhi_pi8(mm2, Null);
		mm3 = _mm_unpackhi_pi8(mm3, Null);
		mm4 = _mm_unpackhi_pi8(mm4, Null);
		mm5 = _mm_unpackhi_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		*((DWORD*)&block[j][4]) = _m_to_int(tmp);

		src += w1;
	}
}
void  get_block_8xh_p20_sse2(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int j;
	byte *src;
	const int w1 = stride;

	src = pSrc;

	__m128i mm0,mm1,mm2,mm3,mm4,mm5,tmp;
	const __m128i Null     = _mm_setzero_si128();
	const __m128i coeff_16 = _mm_load_si128((__m128i*)ncoeff_16);

	for (j = 0; j < height; j++)
	{
		mm0 = _mm_loadl_epi64(((__m128i*) (src-2)));
		mm1 = _mm_loadl_epi64(((__m128i*) (src-1)));
		mm2 = _mm_loadl_epi64(((__m128i*) (src)));
		mm3 = _mm_loadl_epi64(((__m128i*) (src+1)));
		mm4 = _mm_loadl_epi64(((__m128i*) (src+2)));
		mm5 = _mm_loadl_epi64(((__m128i*) (src+3)));

		mm0 = _mm_unpacklo_epi8(mm0, Null);
		mm1 = _mm_unpacklo_epi8(mm1, Null);
		mm2 = _mm_unpacklo_epi8(mm2, Null);
		mm3 = _mm_unpacklo_epi8(mm3, Null);
		mm4 = _mm_unpacklo_epi8(mm4, Null);
		mm5 = _mm_unpacklo_epi8(mm5, Null);

		tmp = _mm_sub_epi16(_mm_slli_epi16(_mm_add_epi16(mm2,mm3),2),_mm_add_epi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_slli_epi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_add_epi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_epi16(_mm_add_epi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packus_epi16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		_mm_storel_epi64((__m128i*)&block[j][0], tmp);

		src += w1;
	}
}

void  get_block_8xh_p21_sse(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	byte *src;
	int j;
	DO_ALIGN(16, short luma_res[21][8]);
	DO_ALIGN(16, int tmp_res[21][8]);
	const int w1 = stride;

	src = pSrc-(2*w1);

	__m64 mm0, mm1, mm2, mm3, mm4, mm5, tmp, tmp1;
	const __m64 Null     = _mm_setzero_si64();
	const __m64 coeff_16 = *((__m64*)ncoeff_16);
	const __m64 coeff_512 = *((__m64*)ncoeff_512);

	for (j = 0; j < height+5; j++)
	{
		mm0 = *((__m64*)(src-2));
		mm1 = *((__m64*)(src-1));
		mm2 = *((__m64*)(src));
		mm3 = *((__m64*)(src+1));
		mm4 = *((__m64*)(src+2));
		mm5 = *((__m64*)(src+3));

		mm0 = _mm_unpacklo_pi8(mm0, Null);
		mm1 = _mm_unpacklo_pi8(mm1, Null);
		mm2 = _mm_unpacklo_pi8(mm2, Null);
		mm3 = _mm_unpacklo_pi8(mm3, Null);
		mm4 = _mm_unpacklo_pi8(mm4, Null);
		mm5 = _mm_unpacklo_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		
		mm0 = _mm_unpacklo_pi16(Null,tmp);
		mm0 = _mm_srai_pi32(mm0,16);		
		*((__m64*)&tmp_res[j][0]) = mm0;
		mm0 = _mm_unpackhi_pi16(Null,tmp);
		mm0 = _mm_srai_pi32(mm0,16);		
		*((__m64*)&tmp_res[j][2]) = mm0;
		
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		*((__m64*)&luma_res[j][0]) = tmp;
		
		
		mm0 = *((__m64*)(src-2));
		mm1 = *((__m64*)(src-1));
		mm2 = *((__m64*)(src));
		mm3 = *((__m64*)(src+1));
		mm4 = *((__m64*)(src+2));
		mm5 = *((__m64*)(src+3));

		mm0 = _mm_unpackhi_pi8(mm0, Null);
		mm1 = _mm_unpackhi_pi8(mm1, Null);
		mm2 = _mm_unpackhi_pi8(mm2, Null);
		mm3 = _mm_unpackhi_pi8(mm3, Null);
		mm4 = _mm_unpackhi_pi8(mm4, Null);
		mm5 = _mm_unpackhi_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		
		mm1 = _mm_unpacklo_pi16(Null,tmp);
		mm1 = _mm_srai_pi32(mm1,16);
		*((__m64*)&tmp_res[j][4]) = mm1;
		mm1 = _mm_unpackhi_pi16(Null,tmp);
		mm1 = _mm_srai_pi32(mm1,16);
		*((__m64*)&tmp_res[j][6]) = mm1;
		
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		*((__m64*)&luma_res[j][4]) = tmp;

		src += w1;
	}


	for (j = 0; j < height; j++)
	{
	mm0 = *((__m64*)&tmp_res[j][0]);
		mm1 = *((__m64*)&tmp_res[j+1][0]);
		mm2 = *((__m64*)&tmp_res[j+2][0]);
		mm3 = *((__m64*)&tmp_res[j+3][0]);
		mm4 = *((__m64*)&tmp_res[j+4][0]);
		mm5 = *((__m64*)&tmp_res[j+5][0]);

		tmp = _mm_sub_pi32(_mm_slli_pi32(_mm_add_pi32(mm2,mm3),2),_mm_add_pi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi32(_mm_slli_pi32(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi32(_mm_add_pi32(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi32(_mm_add_pi32(tmp,coeff_512),10);//(result_block+512)>>10

	
		mm0 = *((__m64*)&tmp_res[j][2]);
		mm1 = *((__m64*)&tmp_res[j+1][2]);
		mm2 = *((__m64*)&tmp_res[j+2][2]);
		mm3 = *((__m64*)&tmp_res[j+3][2]);
		mm4 = *((__m64*)&tmp_res[j+4][2]);
		mm5 = *((__m64*)&tmp_res[j+5][2]);

		tmp1 = _mm_sub_pi32(_mm_slli_pi32(_mm_add_pi32(mm2,mm3),2),_mm_add_pi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_pi32(_mm_slli_pi32(tmp1,2),tmp1);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp1= _mm_add_pi32(_mm_add_pi32(mm0,mm5),tmp1); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp1 = _mm_srai_pi32(_mm_add_pi32(tmp1,coeff_512),10);//(result_block+512)>>10

		mm1 = _mm_packs_pi32(tmp,tmp1);//check
		*((__m64*)&block[j][0]) = mm1;

		mm0 = *((__m64*)&tmp_res[j][4]);
		mm1 = *((__m64*)&tmp_res[j+1][4]);
		mm2 = *((__m64*)&tmp_res[j+2][4]);
		mm3 = *((__m64*)&tmp_res[j+3][4]);
		mm4 = *((__m64*)&tmp_res[j+4][4]);
		mm5 = *((__m64*)&tmp_res[j+5][4]);

		tmp = _mm_sub_pi32(_mm_slli_pi32(_mm_add_pi32(mm2,mm3),2),_mm_add_pi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi32(_mm_slli_pi32(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi32(_mm_add_pi32(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi32(_mm_add_pi32(tmp,coeff_512),10);//(result_block+512)>>10

		mm0 = *((__m64*)&tmp_res[j][6]);
		mm1 = *((__m64*)&tmp_res[j+1][6]);
		mm2 = *((__m64*)&tmp_res[j+2][6]);
		mm3 = *((__m64*)&tmp_res[j+3][6]);
		mm4 = *((__m64*)&tmp_res[j+4][6]);
		mm5 = *((__m64*)&tmp_res[j+5][6]);

		tmp1 = _mm_sub_pi32(_mm_slli_pi32(_mm_add_pi32(mm2,mm3),2),_mm_add_pi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_pi32(_mm_slli_pi32(tmp1,2),tmp1);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp1= _mm_add_pi32(_mm_add_pi32(mm0,mm5),tmp1); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp1 = _mm_srai_pi32(_mm_add_pi32(tmp1,coeff_512),10);//(result_block+512)>>10

		mm2 = _mm_packs_pi32(tmp,tmp1);//check
		mm1 = *((__m64*)&block[j][0]);

		mm0 = *((__m64*)&luma_res[j+2][0]);
		mm3 = *((__m64*)&luma_res[j+2][4]);
		tmp = _mm_packs_pu16(mm1,mm2);//__fast_iclip0_255((result_block+512)>>10)
		mm0 = _mm_packs_pu16(mm0,mm3);
		tmp = _mm_avg_pu8(tmp,mm0);
		*((__m64*)&block[j][0]) = tmp;
	}
}


void  get_block_8xh_p21_sse2(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	byte *src;
	int j;
	DO_ALIGN(16, short luma_res[21][8]);
	DO_ALIGN(16, int tmp_res[21][8]);
	const int w1 = stride;

	src = pSrc-(2*w1);

	__m128i mm0,mm1,mm2,mm3,mm4,mm5,tmp,tmp1;
	const __m128i Null      = _mm_setzero_si128();
	const __m128i coeff_16  = _mm_load_si128((__m128i*)ncoeff_16);
	const __m128i coeff_512 = _mm_load_si128((__m128i*)ncoeff_512);

	for (j = 0; j < height+5; j++)
	{
		mm0 = _mm_loadl_epi64(((__m128i*) (src-2)));
		mm1 = _mm_loadl_epi64(((__m128i*) (src-1)));
		mm2 = _mm_loadl_epi64(((__m128i*) (src)));
		mm3 = _mm_loadl_epi64(((__m128i*) (src+1)));
		mm4 = _mm_loadl_epi64(((__m128i*) (src+2)));
		mm5 = _mm_loadl_epi64(((__m128i*) (src+3)));

		mm0 = _mm_unpacklo_epi8(mm0, Null);
		mm1 = _mm_unpacklo_epi8(mm1, Null);
		mm2 = _mm_unpacklo_epi8(mm2, Null);
		mm3 = _mm_unpacklo_epi8(mm3, Null);
		mm4 = _mm_unpacklo_epi8(mm4, Null);
		mm5 = _mm_unpacklo_epi8(mm5, Null);

		tmp = _mm_sub_epi16(_mm_slli_epi16(_mm_add_epi16(mm2,mm3),2),_mm_adds_epi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_slli_epi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_add_epi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5

		mm0 = _mm_unpacklo_epi16(Null,tmp);
		mm1 = _mm_unpackhi_epi16(Null,tmp);

		mm0 = _mm_srai_epi32(mm0,16);
		mm1 = _mm_srai_epi32(mm1,16);

		_mm_store_si128(((__m128i*)&tmp_res[j][0]),mm0);
		_mm_store_si128(((__m128i*)&tmp_res[j][4]),mm1);

		tmp = _mm_srai_epi16(_mm_add_epi16(tmp,coeff_16),5);//(result_block+16)>>5

		_mm_store_si128(((__m128i*)&luma_res[j][0]),tmp);

		src += w1;
	}


	for (j = 0; j < height; j++)
	{
		mm0 = _mm_load_si128((__m128i*)&tmp_res[j][0]);
		mm1 = _mm_load_si128((__m128i*)&tmp_res[j+1][0]);
		mm2 = _mm_load_si128((__m128i*)&tmp_res[j+2][0]);
		mm3 = _mm_load_si128((__m128i*)&tmp_res[j+3][0]);
		mm4 = _mm_load_si128((__m128i*)&tmp_res[j+4][0]);
		mm5 = _mm_load_si128((__m128i*)&tmp_res[j+5][0]);

		tmp = _mm_sub_epi32(_mm_slli_epi32(_mm_add_epi32(mm2,mm3),2),_mm_add_epi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi32(_mm_slli_epi32(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi32(_mm_add_epi32(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_epi32(_mm_add_epi32(tmp,coeff_512),10);//(result_block+512)>>10	

		mm0 = _mm_load_si128((__m128i*)&tmp_res[j][4]);
		mm1 = _mm_load_si128((__m128i*)&tmp_res[j+1][4]);
		mm2 = _mm_load_si128((__m128i*)&tmp_res[j+2][4]);
		mm3 = _mm_load_si128((__m128i*)&tmp_res[j+3][4]);
		mm4 = _mm_load_si128((__m128i*)&tmp_res[j+4][4]);
		mm5 = _mm_load_si128((__m128i*)&tmp_res[j+5][4]);

		tmp1 = _mm_sub_epi32(_mm_slli_epi32(_mm_add_epi32(mm2,mm3),2),_mm_add_epi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_epi32(_mm_slli_epi32(tmp1,2),tmp1);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp1= _mm_add_epi32(_mm_add_epi32(mm0,mm5),tmp1); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp1 = _mm_srai_epi32(_mm_add_epi32(tmp1,coeff_512),10);//(result_block+512)>>10

		tmp = _mm_packs_epi32(tmp,tmp1);//check

		mm0 = _mm_load_si128((__m128i*)&luma_res[j+2][0]);	//__fast_iclip0_255(((tmp_res[j+2][i]+16)>>5))
		tmp = _mm_packus_epi16(tmp,tmp);//__fast_iclip0_255((result_block+512)>>10)
		mm0 = _mm_packus_epi16(mm0,mm0);
		tmp = _mm_avg_epu8(tmp,mm0);

		_mm_storel_epi64((__m128i*)&block[j][0], tmp);
	}
}
void  get_block_8xh_p22_sse(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	byte *src;
	int j;
	DO_ALIGN(16, int tmp_res[21][8]);
	const int w1 = stride;

	src = pSrc - (2*w1);

	__m64 mm0, mm1, mm2, mm3, mm4, mm5, tmp, tmp1;
	const __m64 Null     = _mm_setzero_si64();
	const __m64 coeff_512 = *((__m64*)ncoeff_512);

	for (j = 0; j < height+5; j++)
	{
		mm0 = *((__m64*)(src-2));
		mm1 = *((__m64*)(src-1));
		mm2 = *((__m64*)(src));
		mm3 = *((__m64*)(src+1));
		mm4 = *((__m64*)(src+2));
		mm5 = *((__m64*)(src+3));


		mm0 = _mm_unpacklo_pi8(mm0, Null);
		mm1 = _mm_unpacklo_pi8(mm1, Null);
		mm2 = _mm_unpacklo_pi8(mm2, Null);
		mm3 = _mm_unpacklo_pi8(mm3, Null);
		mm4 = _mm_unpacklo_pi8(mm4, Null);
		mm5 = _mm_unpacklo_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5

		mm0 = _mm_unpacklo_pi16(Null,tmp);
		mm0 = _mm_srai_pi32(mm0,16);
		*((__m64*)&tmp_res[j][0]) = mm0;
		mm0 = _mm_unpackhi_pi16(Null,tmp);
		mm0 = _mm_srai_pi32(mm0,16);
		*((__m64*)&tmp_res[j][2]) = mm0;
		
		mm0 = *((__m64*)(src-2));
		mm1 = *((__m64*)(src-1));
		mm2 = *((__m64*)(src));
		mm3 = *((__m64*)(src+1));
		mm4 = *((__m64*)(src+2));
		mm5 = *((__m64*)(src+3));

		mm0 = _mm_unpackhi_pi8(mm0, Null);
		mm1 = _mm_unpackhi_pi8(mm1, Null);
		mm2 = _mm_unpackhi_pi8(mm2, Null);
		mm3 = _mm_unpackhi_pi8(mm3, Null);
		mm4 = _mm_unpackhi_pi8(mm4, Null);
		mm5 = _mm_unpackhi_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5

		mm0 = _mm_unpacklo_pi16(Null,tmp);
		mm0 = _mm_srai_pi32(mm0,16);
		*((__m64*)&tmp_res[j][4]) = mm0;
		mm0 = _mm_unpackhi_pi16(Null,tmp);
		mm0 = _mm_srai_pi32(mm0,16);
		*((__m64*)&tmp_res[j][6]) = mm0;

		src += w1;
	}

	for (j = 0; j < height; j++)
	{
		mm0 = *((__m64*)&tmp_res[j][0]);
		mm1 = *((__m64*)&tmp_res[j+1][0]);
		mm2 = *((__m64*)&tmp_res[j+2][0]);
		mm3 = *((__m64*)&tmp_res[j+3][0]);
		mm4 = *((__m64*)&tmp_res[j+4][0]);
		mm5 = *((__m64*)&tmp_res[j+5][0]);

		tmp = _mm_sub_pi32(_mm_slli_pi32(_mm_add_pi32(mm2,mm3),2),_mm_add_pi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi32(_mm_slli_pi32(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi32(_mm_add_pi32(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi32(_mm_add_pi32(tmp,coeff_512),10);//(result_block+512)>>10


		mm0 = *((__m64*)&tmp_res[j][2]);
		mm1 = *((__m64*)&tmp_res[j+1][2]);
		mm2 = *((__m64*)&tmp_res[j+2][2]);
		mm3 = *((__m64*)&tmp_res[j+3][2]);
		mm4 = *((__m64*)&tmp_res[j+4][2]);
		mm5 = *((__m64*)&tmp_res[j+5][2]);

		tmp1 = _mm_sub_pi32(_mm_slli_pi32(_mm_add_pi32(mm2,mm3),2),_mm_add_pi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_pi32(_mm_slli_pi32(tmp1,2),tmp1);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_pi32(_mm_add_pi32(mm0,mm5),tmp1); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp1 = _mm_srai_pi32(_mm_add_pi32(tmp1,coeff_512),10);//(result_block+512)>>10

		tmp = _mm_packs_pi32(tmp,tmp1);
		*((__m64*)&block[j][0]) = tmp;
		
		mm0 = *((__m64*)&tmp_res[j][4]);
		mm1 = *((__m64*)&tmp_res[j+1][4]);
		mm2 = *((__m64*)&tmp_res[j+2][4]);
		mm3 = *((__m64*)&tmp_res[j+3][4]);
		mm4 = *((__m64*)&tmp_res[j+4][4]);
		mm5 = *((__m64*)&tmp_res[j+5][4]);

		tmp = _mm_sub_pi32(_mm_slli_pi32(_mm_add_pi32(mm2,mm3),2),_mm_add_pi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi32(_mm_slli_pi32(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi32(_mm_add_pi32(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi32(_mm_add_pi32(tmp,coeff_512),10);//(result_block+512)>>10

		mm0 = *((__m64*)&tmp_res[j][6]);
		mm1 = *((__m64*)&tmp_res[j+1][6]);
		mm2 = *((__m64*)&tmp_res[j+2][6]);
		mm3 = *((__m64*)&tmp_res[j+3][6]);
		mm4 = *((__m64*)&tmp_res[j+4][6]);
		mm5 = *((__m64*)&tmp_res[j+5][6]);

		tmp1 = _mm_sub_pi32(_mm_slli_pi32(_mm_add_pi32(mm2,mm3),2),_mm_add_pi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_pi32(_mm_slli_pi32(tmp1,2),tmp1);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_pi32(_mm_add_pi32(mm0,mm5),tmp1); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp1 = _mm_srai_pi32(_mm_add_pi32(tmp1,coeff_512),10);//(result_block+512)>>10

		tmp1 = _mm_packs_pi32(tmp,tmp1);
		tmp = *((__m64*)&block[j][0]);
		tmp = _mm_packs_pu16(tmp,tmp1);//__fast_iclip0_255((result_block+512)>>10)
		*((__m64*)&block[j][0]) = tmp;
		
	}
}

void  get_block_8xh_p22_sse2(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	byte *src;
	int j;
	DO_ALIGN(16, int tmp_res[21][8]);
	const int w1 = stride;

	src = pSrc - (2*w1);

	__m128i mm0,mm1,mm2,mm3,mm4,mm5,tmp,tmp1;
	const __m128i Null      = _mm_setzero_si128();
	const __m128i coeff_512 = _mm_load_si128((__m128i*)ncoeff_512);

	for (j = 0; j < height+5; j++)
	{
		mm0 = _mm_loadl_epi64(((__m128i*) (src-2)));
		mm1 = _mm_loadl_epi64(((__m128i*) (src-1)));
		mm2 = _mm_loadl_epi64(((__m128i*) (src)));
		mm3 = _mm_loadl_epi64(((__m128i*) (src+1)));
		mm4 = _mm_loadl_epi64(((__m128i*) (src+2)));
		mm5 = _mm_loadl_epi64(((__m128i*) (src+3)));

		mm0 = _mm_unpacklo_epi8(mm0, Null);
		mm1 = _mm_unpacklo_epi8(mm1, Null);
		mm2 = _mm_unpacklo_epi8(mm2, Null);
		mm3 = _mm_unpacklo_epi8(mm3, Null);
		mm4 = _mm_unpacklo_epi8(mm4, Null);
		mm5 = _mm_unpacklo_epi8(mm5, Null);

		tmp = _mm_sub_epi16(_mm_slli_epi16(_mm_add_epi16(mm2,mm3),2),_mm_add_epi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_slli_epi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_add_epi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5

		mm0 = _mm_unpacklo_epi16(Null,tmp);
		mm1 = _mm_unpackhi_epi16(Null,tmp);

		mm0 = _mm_srai_epi32(mm0,16);
		mm1 = _mm_srai_epi32(mm1,16);

		_mm_store_si128(((__m128i*)&tmp_res[j][0]),mm0);
		_mm_store_si128(((__m128i*)&tmp_res[j][4]),mm1);

		src += w1;
	}

	for (j = 0; j < height; j++)
	{
		mm0 = _mm_load_si128((__m128i*)&tmp_res[j][0]);
		mm1 = _mm_load_si128((__m128i*)&tmp_res[j+1][0]);
		mm2 = _mm_load_si128((__m128i*)&tmp_res[j+2][0]);
		mm3 = _mm_load_si128((__m128i*)&tmp_res[j+3][0]);
		mm4 = _mm_load_si128((__m128i*)&tmp_res[j+4][0]);
		mm5 = _mm_load_si128((__m128i*)&tmp_res[j+5][0]);

		tmp = _mm_sub_epi32(_mm_slli_epi32(_mm_add_epi32(mm2,mm3),2),_mm_add_epi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi32(_mm_slli_epi32(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi32(_mm_add_epi32(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_epi32(_mm_add_epi32(tmp,coeff_512),10);//(result_block+512)>>10

		mm0 = _mm_load_si128((__m128i*)&tmp_res[j][4]);
		mm1 = _mm_load_si128((__m128i*)&tmp_res[j+1][4]);
		mm2 = _mm_load_si128((__m128i*)&tmp_res[j+2][4]);
		mm3 = _mm_load_si128((__m128i*)&tmp_res[j+3][4]);
		mm4 = _mm_load_si128((__m128i*)&tmp_res[j+4][4]);
		mm5 = _mm_load_si128((__m128i*)&tmp_res[j+5][4]);

		tmp1 = _mm_sub_epi32(_mm_slli_epi32(_mm_add_epi32(mm2,mm3),2),_mm_add_epi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_epi32(_mm_slli_epi32(tmp1,2),tmp1);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_epi32(_mm_add_epi32(mm0,mm5),tmp1); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp1 = _mm_srai_epi32(_mm_add_epi32(tmp1,coeff_512),10);//(result_block+512)>>10

		tmp = _mm_packs_epi32(tmp,tmp1);//check
		tmp = _mm_packus_epi16(tmp,tmp);//__fast_iclip0_255((result_block+512)>>10)
		_mm_storel_epi64((__m128i*)&block[j][0], tmp);
	}
}
void  get_block_8xh_p23_sse(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int j;
	DO_ALIGN(16, short luma_res[21][8]);
	DO_ALIGN(16, int tmp_res[21][8]);
	byte *src;
	const int w1 = stride;

	src = pSrc -(2*w1);

	__m64 mm0, mm1, mm2, mm3, mm4, mm5, tmp, tmp1;
	const __m64 Null     = _mm_setzero_si64();
	const __m64 coeff_16 = *((__m64*)ncoeff_16);
	const __m64 coeff_512 = *((__m64*)ncoeff_512);

	for (j = 0; j < height+5; j++)
	{
		mm0 = *((__m64*)(src-2));
		mm1 = *((__m64*)(src-1));
		mm2 = *((__m64*)(src));
		mm3 = *((__m64*)(src+1));
		mm4 = *((__m64*)(src+2));
		mm5 = *((__m64*)(src+3));

		mm0 = _mm_unpacklo_pi8(mm0, Null);
		mm1 = _mm_unpacklo_pi8(mm1, Null);
		mm2 = _mm_unpacklo_pi8(mm2, Null);
		mm3 = _mm_unpacklo_pi8(mm3, Null);
		mm4 = _mm_unpacklo_pi8(mm4, Null);
		mm5 = _mm_unpacklo_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		
		mm0 = _mm_unpacklo_pi16(Null,tmp);
		mm0 = _mm_srai_pi32(mm0,16);		
		*((__m64*)&tmp_res[j][0]) = mm0;
		mm0 = _mm_unpackhi_pi16(Null,tmp);
		mm0 = _mm_srai_pi32(mm0,16);		
		*((__m64*)&tmp_res[j][2]) = mm0;
		
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		*((__m64*)&luma_res[j][0]) = tmp;
		
		
		mm0 = *((__m64*)(src-2));
		mm1 = *((__m64*)(src-1));
		mm2 = *((__m64*)(src));
		mm3 = *((__m64*)(src+1));
		mm4 = *((__m64*)(src+2));
		mm5 = *((__m64*)(src+3));

		mm0 = _mm_unpackhi_pi8(mm0, Null);
		mm1 = _mm_unpackhi_pi8(mm1, Null);
		mm2 = _mm_unpackhi_pi8(mm2, Null);
		mm3 = _mm_unpackhi_pi8(mm3, Null);
		mm4 = _mm_unpackhi_pi8(mm4, Null);
		mm5 = _mm_unpackhi_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		
		mm1 = _mm_unpacklo_pi16(Null,tmp);
		mm1 = _mm_srai_pi32(mm1,16);
		*((__m64*)&tmp_res[j][4]) = mm1;
		mm1 = _mm_unpackhi_pi16(Null,tmp);
		mm1 = _mm_srai_pi32(mm1,16);
		*((__m64*)&tmp_res[j][6]) = mm1;
		
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		*((__m64*)&luma_res[j][4]) = tmp;

		src += w1;
	}


	for (j = 0; j < height; j++)
	{
		mm0 = *((__m64*)&tmp_res[j][0]);
		mm1 = *((__m64*)&tmp_res[j+1][0]);
		mm2 = *((__m64*)&tmp_res[j+2][0]);
		mm3 = *((__m64*)&tmp_res[j+3][0]);
		mm4 = *((__m64*)&tmp_res[j+4][0]);
		mm5 = *((__m64*)&tmp_res[j+5][0]);

		tmp = _mm_sub_pi32(_mm_slli_pi32(_mm_add_pi32(mm2,mm3),2),_mm_add_pi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi32(_mm_slli_pi32(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi32(_mm_add_pi32(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi32(_mm_add_pi32(tmp,coeff_512),10);//(result_block+512)>>10

	
		mm0 = *((__m64*)&tmp_res[j][2]);
		mm1 = *((__m64*)&tmp_res[j+1][2]);
		mm2 = *((__m64*)&tmp_res[j+2][2]);
		mm3 = *((__m64*)&tmp_res[j+3][2]);
		mm4 = *((__m64*)&tmp_res[j+4][2]);
		mm5 = *((__m64*)&tmp_res[j+5][2]);

		tmp1 = _mm_sub_pi32(_mm_slli_pi32(_mm_add_pi32(mm2,mm3),2),_mm_add_pi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_pi32(_mm_slli_pi32(tmp1,2),tmp1);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp1= _mm_add_pi32(_mm_add_pi32(mm0,mm5),tmp1); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp1 = _mm_srai_pi32(_mm_add_pi32(tmp1,coeff_512),10);//(result_block+512)>>10

		mm1 = _mm_packs_pi32(tmp,tmp1);//check
		*((__m64*)&block[j][0]) = mm1;

		mm0 = *((__m64*)&tmp_res[j][4]);
		mm1 = *((__m64*)&tmp_res[j+1][4]);
		mm2 = *((__m64*)&tmp_res[j+2][4]);
		mm3 = *((__m64*)&tmp_res[j+3][4]);
		mm4 = *((__m64*)&tmp_res[j+4][4]);
		mm5 = *((__m64*)&tmp_res[j+5][4]);

		tmp = _mm_sub_pi32(_mm_slli_pi32(_mm_add_pi32(mm2,mm3),2),_mm_add_pi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi32(_mm_slli_pi32(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi32(_mm_add_pi32(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi32(_mm_add_pi32(tmp,coeff_512),10);//(result_block+512)>>10

		mm0 = *((__m64*)&tmp_res[j][6]);
		mm1 = *((__m64*)&tmp_res[j+1][6]);
		mm2 = *((__m64*)&tmp_res[j+2][6]);
		mm3 = *((__m64*)&tmp_res[j+3][6]);
		mm4 = *((__m64*)&tmp_res[j+4][6]);
		mm5 = *((__m64*)&tmp_res[j+5][6]);

		tmp1 = _mm_sub_pi32(_mm_slli_pi32(_mm_add_pi32(mm2,mm3),2),_mm_add_pi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_pi32(_mm_slli_pi32(tmp1,2),tmp1);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp1= _mm_add_pi32(_mm_add_pi32(mm0,mm5),tmp1); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp1 = _mm_srai_pi32(_mm_add_pi32(tmp1,coeff_512),10);//(result_block+512)>>10

		mm2 = _mm_packs_pi32(tmp,tmp1);//check
		mm1 = *((__m64*)&block[j][0]);

		mm0 = *((__m64*)&luma_res[j+3][0]);
		mm3 = *((__m64*)&luma_res[j+3][4]);
		tmp = _mm_packs_pu16(mm1,mm2);//__fast_iclip0_255((result_block+512)>>10)
		mm0 = _mm_packs_pu16(mm0,mm3);
		tmp = _mm_avg_pu8(tmp,mm0);
		*((__m64*)&block[j][0]) = tmp;
	}
}

void  get_block_8xh_p23_sse2(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int j;
	DO_ALIGN(16, short luma_res[21][8]);
	DO_ALIGN(16, int tmp_res[21][8]);
	byte *src;
	const int w1 = stride;

	src = pSrc -(2*w1);

	__m128i mm0,mm1,mm2,mm3,mm4,mm5,tmp,tmp1;
	const __m128i Null      = _mm_setzero_si128();
	const __m128i coeff_16  = _mm_load_si128((__m128i*)ncoeff_16);
	const __m128i coeff_512 = _mm_load_si128((__m128i*)ncoeff_512);

	for (j = 0; j < height+5; j++)
	{
		mm0 = _mm_loadl_epi64(((__m128i*) (src-2)));
		mm1 = _mm_loadl_epi64(((__m128i*) (src-1)));
		mm2 = _mm_loadl_epi64(((__m128i*) (src)));
		mm3 = _mm_loadl_epi64(((__m128i*) (src+1)));
		mm4 = _mm_loadl_epi64(((__m128i*) (src+2)));
		mm5 = _mm_loadl_epi64(((__m128i*) (src+3)));

		mm0 = _mm_unpacklo_epi8(mm0, Null);
		mm1 = _mm_unpacklo_epi8(mm1, Null);
		mm2 = _mm_unpacklo_epi8(mm2, Null);
		mm3 = _mm_unpacklo_epi8(mm3, Null);
		mm4 = _mm_unpacklo_epi8(mm4, Null);
		mm5 = _mm_unpacklo_epi8(mm5, Null);

		tmp = _mm_sub_epi16(_mm_slli_epi16(_mm_add_epi16(mm2,mm3),2),_mm_adds_epi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_slli_epi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_add_epi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5

		mm0 = _mm_unpacklo_epi16(Null,tmp);
		mm1 = _mm_unpackhi_epi16(Null,tmp);

		mm0 = _mm_srai_epi32(mm0,16);
		mm1 = _mm_srai_epi32(mm1,16);

		_mm_store_si128(((__m128i*)&tmp_res[j][0]),mm0);
		_mm_store_si128(((__m128i*)&tmp_res[j][4]),mm1);

		tmp = _mm_srai_epi16(_mm_add_epi16(tmp,coeff_16),5);//(result_block+16)>>5

		_mm_store_si128(((__m128i*)&luma_res[j][0]),tmp);

		src += w1;
	}


	for (j = 0; j < height; j++)
	{
		mm0 = _mm_load_si128((__m128i*)&tmp_res[j][0]);
		mm1 = _mm_load_si128((__m128i*)&tmp_res[j+1][0]);
		mm2 = _mm_load_si128((__m128i*)&tmp_res[j+2][0]);
		mm3 = _mm_load_si128((__m128i*)&tmp_res[j+3][0]);
		mm4 = _mm_load_si128((__m128i*)&tmp_res[j+4][0]);
		mm5 = _mm_load_si128((__m128i*)&tmp_res[j+5][0]);

		tmp = _mm_sub_epi32(_mm_slli_epi32(_mm_add_epi32(mm2,mm3),2),_mm_add_epi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi32(_mm_slli_epi32(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi32(_mm_add_epi32(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_epi32(_mm_add_epi32(tmp,coeff_512),10);//(result_block+512)>>10

		mm0 = _mm_load_si128((__m128i*)&tmp_res[j][4]);
		mm1 = _mm_load_si128((__m128i*)&tmp_res[j+1][4]);
		mm2 = _mm_load_si128((__m128i*)&tmp_res[j+2][4]);
		mm3 = _mm_load_si128((__m128i*)&tmp_res[j+3][4]);
		mm4 = _mm_load_si128((__m128i*)&tmp_res[j+4][4]);
		mm5 = _mm_load_si128((__m128i*)&tmp_res[j+5][4]);

		tmp1 = _mm_sub_epi32(_mm_slli_epi32(_mm_add_epi32(mm2,mm3),2),_mm_add_epi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp1= _mm_add_epi32(_mm_slli_epi32(tmp1,2),tmp1);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_epi32(_mm_add_epi32(mm0,mm5),tmp1); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp1 = _mm_srai_epi32(_mm_add_epi32(tmp1,coeff_512),10);//(result_block+512)>>10

		tmp = _mm_packs_epi32(tmp,tmp1);//check

		mm0 = _mm_load_si128((__m128i*)&luma_res[j+3][0]);	//__fast_iclip0_255(((tmp_res[j+2][i]+16)>>5))
		tmp = _mm_packus_epi16(tmp,tmp);//__fast_iclip0_255((result_block+512)>>10)
		mm0 = _mm_packus_epi16(mm0,mm0);
		tmp = _mm_avg_epu8(tmp,mm0);

		_mm_storel_epi64((__m128i*)&block[j][0], tmp);
	}
}
void  get_block_8xh_p30_sse(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int j;
	byte *src;
	const int w1 = stride;

	src = pSrc;

	__m64 mm0, mm1, mm2, mm3, mm4, mm5, tmp;
	const __m64 Null     = _mm_setzero_si64();
	const __m64 coeff_16 = *((__m64*)ncoeff_16);

	for (j = 0; j < height; j++)
	{
		mm0 = *((__m64*)(src-2));
		mm1 = *((__m64*)(src-1));
		mm2 = *((__m64*)(src));
		mm3 = *((__m64*)(src+1));
		mm4 = *((__m64*)(src+2));
		mm5 = *((__m64*)(src+3));

		mm0 = _mm_unpacklo_pi8(mm0, Null);
		mm1 = _mm_unpacklo_pi8(mm1, Null);
		mm2 = _mm_unpacklo_pi8(mm2, Null);
		mm3 = _mm_unpacklo_pi8(mm3, Null);
		mm4 = _mm_unpacklo_pi8(mm4, Null);
		mm5 = _mm_unpacklo_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		mm3 = _mm_packs_pu16(mm3,mm3);
		tmp = _mm_avg_pu8(tmp,mm3);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		*((DWORD*)&block[j][0]) = _m_to_int(tmp);

		mm0 = *((__m64*)(src-2));
		mm1 = *((__m64*)(src-1));
		mm2 = *((__m64*)(src));
		mm3 = *((__m64*)(src+1));
		mm4 = *((__m64*)(src+2));
		mm5 = *((__m64*)(src+3));

		mm0 = _mm_unpackhi_pi8(mm0, Null);
		mm1 = _mm_unpackhi_pi8(mm1, Null);
		mm2 = _mm_unpackhi_pi8(mm2, Null);
		mm3 = _mm_unpackhi_pi8(mm3, Null);
		mm4 = _mm_unpackhi_pi8(mm4, Null);
		mm5 = _mm_unpackhi_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		mm3 = _mm_packs_pu16(mm3,mm3);
		tmp = _mm_avg_pu8(tmp,mm3);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		*((DWORD*)&block[j][4]) = _m_to_int(tmp);

		src += w1;
	}
}

void  get_block_8xh_p30_sse2(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int j;
	byte *src;
	const int w1 = stride;

	src = pSrc;

	__m128i mm0,mm1,mm2,mm3,mm4,mm5,tmp;
	const __m128i Null     = _mm_setzero_si128();
	const __m128i coeff_16 = _mm_load_si128((__m128i*)ncoeff_16);

	for (j = 0; j < height; j++)
	{
		mm0 = _mm_loadl_epi64(((__m128i*) (src-2)));
		mm1 = _mm_loadl_epi64(((__m128i*) (src-1)));
		mm2 = _mm_loadl_epi64(((__m128i*) (src)));
		mm3 = _mm_loadl_epi64(((__m128i*) (src+1)));
		mm4 = _mm_loadl_epi64(((__m128i*) (src+2)));
		mm5 = _mm_loadl_epi64(((__m128i*) (src+3)));

		mm0 = _mm_unpacklo_epi8(mm0, Null);
		mm1 = _mm_unpacklo_epi8(mm1, Null);
		mm2 = _mm_unpacklo_epi8(mm2, Null);
		mm3 = _mm_unpacklo_epi8(mm3, Null);
		mm4 = _mm_unpacklo_epi8(mm4, Null);
		mm5 = _mm_unpacklo_epi8(mm5, Null);

		tmp = _mm_sub_epi16(_mm_slli_epi16(_mm_add_epi16(mm2,mm3),2),_mm_add_epi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_slli_epi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_add_epi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_epi16(_mm_add_epi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packus_epi16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		mm3 = _mm_packus_epi16(mm3,mm3);
		tmp = _mm_avg_epu8(tmp,mm3);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		_mm_storel_epi64((__m128i*)&block[j][0], tmp);

		src += w1;
	}
}

void  get_block_8xh_p31_sse(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int j;
	byte *src;
	const int w1 = stride;
	const int w2 = stride + stride;
	const int w3 = w2 + w1;

	src = pSrc;

	__m64 mm0, mm1, mm2, mm3, mm4, mm5, mm6, tmp;
	const __m64 Null     = _mm_setzero_si64();
	const __m64 coeff_16 = *((__m64*)ncoeff_16);

	for (j = 0; j < height; j++)
	{
		mm0 = *((__m64*)(src-2));
		mm1 = *((__m64*)(src-1));
		mm2 = *((__m64*)(src));
		mm3 = *((__m64*)(src+1));
		mm4 = *((__m64*)(src+2));
		mm5 = *((__m64*)(src+3));

		mm0 = _mm_unpacklo_pi8(mm0, Null);
		mm1 = _mm_unpacklo_pi8(mm1, Null);
		mm2 = _mm_unpacklo_pi8(mm2, Null);
		mm3 = _mm_unpacklo_pi8(mm3, Null);
		mm4 = _mm_unpacklo_pi8(mm4, Null);
		mm5 = _mm_unpacklo_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		*((DWORD*)&block[j][0]) = _m_to_int(tmp);
		
		mm0 = *((__m64*)(src-2));
		mm1 = *((__m64*)(src-1));
		mm2 = *((__m64*)(src));
		mm3 = *((__m64*)(src+1));
		mm4 = *((__m64*)(src+2));
		mm5 = *((__m64*)(src+3));

		mm0 = _mm_unpackhi_pi8(mm0, Null);
		mm1 = _mm_unpackhi_pi8(mm1, Null);
		mm2 = _mm_unpackhi_pi8(mm2, Null);
		mm3 = _mm_unpackhi_pi8(mm3, Null);
		mm4 = _mm_unpackhi_pi8(mm4, Null);
		mm5 = _mm_unpackhi_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		*((DWORD*)&block[j][4]) = _m_to_int(tmp);

		src += w1;
	}

	src = pSrc + 1;

	for (j = 0; j < height; j++)
	{
		mm0 = *((__m64*)(src-w2));
		mm1 = *((__m64*)(src-w1));
		mm2 = *((__m64*)(src));
		mm3 = *((__m64*)(src+w1));
		mm4 = *((__m64*)(src+w2));
		mm5 = *((__m64*)(src+w3));
		mm6 = *((__m64*)&block[j][0]);

		mm0 = _mm_unpacklo_pi8(mm0, Null);
		mm1 = _mm_unpacklo_pi8(mm1, Null);
		mm2 = _mm_unpacklo_pi8(mm2, Null);
		mm3 = _mm_unpacklo_pi8(mm3, Null);
		mm4 = _mm_unpacklo_pi8(mm4, Null);
		mm5 = _mm_unpacklo_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		tmp = _mm_avg_pu8(tmp,mm6);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		*((DWORD*)&block[j][0]) = _m_to_int(tmp);

		mm0 = *((__m64*)(src-w2));
		mm1 = *((__m64*)(src-w1));
		mm2 = *((__m64*)(src));
		mm3 = *((__m64*)(src+w1));
		mm4 = *((__m64*)(src+w2));
		mm5 = *((__m64*)(src+w3));
		mm6 = *((__m64*)&block[j][4]);

		mm0 = _mm_unpackhi_pi8(mm0, Null);
		mm1 = _mm_unpackhi_pi8(mm1, Null);
		mm2 = _mm_unpackhi_pi8(mm2, Null);
		mm3 = _mm_unpackhi_pi8(mm3, Null);
		mm4 = _mm_unpackhi_pi8(mm4, Null);
		mm5 = _mm_unpackhi_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		tmp = _mm_avg_pu8(tmp,mm6);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		*((DWORD*)&block[j][4]) = _m_to_int(tmp);

		src += w1;
	}
}

void  get_block_8xh_p31_sse2(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int j;
	byte *src;
	const int w1 = stride;
	const int w2 = stride + stride;
	const int w3 = w2 + w1;

	src = pSrc;

	__m128i mm0,mm1,mm2,mm3,mm4,mm5,mm6,tmp;
	const __m128i Null     = _mm_setzero_si128();
	const __m128i coeff_16 = _mm_load_si128((__m128i*)ncoeff_16);

	for (j = 0; j < height; j++)
	{
		mm0 = _mm_loadl_epi64(((__m128i*) (src-2)));
		mm1 = _mm_loadl_epi64(((__m128i*) (src-1)));
		mm2 = _mm_loadl_epi64(((__m128i*) (src)));
		mm3 = _mm_loadl_epi64(((__m128i*) (src+1)));
		mm4 = _mm_loadl_epi64(((__m128i*) (src+2)));
		mm5 = _mm_loadl_epi64(((__m128i*) (src+3)));

		mm0 = _mm_unpacklo_epi8(mm0, Null);
		mm1 = _mm_unpacklo_epi8(mm1, Null);
		mm2 = _mm_unpacklo_epi8(mm2, Null);
		mm3 = _mm_unpacklo_epi8(mm3, Null);
		mm4 = _mm_unpacklo_epi8(mm4, Null);
		mm5 = _mm_unpacklo_epi8(mm5, Null);

		tmp = _mm_sub_epi16(_mm_slli_epi16(_mm_add_epi16(mm2,mm3),2),_mm_add_epi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_slli_epi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_add_epi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_epi16(_mm_add_epi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packus_epi16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		_mm_storel_epi64((__m128i*)&block[j][0], tmp);

		src += w1;
	}

	src = pSrc + 1;

	mm0 = _mm_loadl_epi64(((__m128i*) (src-w2)));
	mm1 = _mm_loadl_epi64(((__m128i*) (src-w1)));
	mm2 = _mm_loadl_epi64(((__m128i*) (src)));
	mm3 = _mm_loadl_epi64(((__m128i*) (src+w1)));
	mm4 = _mm_loadl_epi64(((__m128i*) (src+w2)));
	mm5 = _mm_loadl_epi64(((__m128i*) (src+w3)));
	mm6 = _mm_loadl_epi64(((__m128i*)&block[0][0]));

	mm0 = _mm_unpacklo_epi8(mm0, Null);
	mm1 = _mm_unpacklo_epi8(mm1, Null);
	mm2 = _mm_unpacklo_epi8(mm2, Null);
	mm3 = _mm_unpacklo_epi8(mm3, Null);
	mm4 = _mm_unpacklo_epi8(mm4, Null);
	mm5 = _mm_unpacklo_epi8(mm5, Null);

	tmp = _mm_sub_epi16(_mm_slli_epi16(_mm_add_epi16(mm2,mm3),2),_mm_add_epi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
	tmp = _mm_add_epi16(_mm_slli_epi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
	tmp = _mm_add_epi16(_mm_add_epi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
	tmp = _mm_srai_epi16(_mm_add_epi16(tmp,coeff_16),5);//(result_block+16)>>5
	tmp = _mm_packus_epi16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
	tmp = _mm_avg_epu8(tmp,mm6);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
	_mm_storel_epi64((__m128i*)&block[0][0], tmp);

	src += w1;

	for (j = 1; j < height; j++)
	{
		mm0 = mm1;
		mm1 = mm2;
		mm2 = mm3;
		mm3 = mm4;
		mm4 = mm5;

		mm5 = _mm_loadl_epi64(((__m128i*) (src+w3)));
		mm5 = _mm_unpacklo_epi8(mm5, Null);

		mm6 = _mm_loadl_epi64(((__m128i*)&block[j][0]));

		tmp = _mm_sub_epi16(_mm_slli_epi16(_mm_add_epi16(mm2,mm3),2),_mm_add_epi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_slli_epi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_add_epi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_epi16(_mm_add_epi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packus_epi16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		tmp = _mm_avg_epu8(tmp,mm6);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		_mm_storel_epi64((__m128i*)&block[j][0], tmp);

		src += w1;
	}
}
void  get_block_8xh_p32_sse(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	byte *src;
	short *tmp_src;
	short *luma_src;
	int i,j;
	DO_ALIGN(16, short luma_res[16][16]);
	DO_ALIGN(16, short tmp_res[16][16]);
	const int w1 = stride;
	const int w2 = stride + stride;
	const int w3 = w2 + w1;
	src = pSrc;

	__m64 pIn0, pIn1;
	__m64 mm0, mm1, mm2, mm3, mm4, mm5, tmp, tmp1;
	const __m64 Null     = _mm_setzero_si64();
	const __m64 coeff_16 = *((__m64*)ncoeff_16);
	const __m64 coeff_512 = *((__m64*)ncoeff_512);

	for (j = 0; j < height; j++)
	{
		for(i = 0; i < 10; i += 8)
		{
			mm0 = *((__m64*)(src-w2-2+i));
			mm1 = *((__m64*)(src-w1-2+i));
			mm2 = *((__m64*)(src-2+i));
			mm3 = *((__m64*)(src+w1-2+i));
			mm4 = *((__m64*)(src+w2-2+i));
			mm5 = *((__m64*)(src+w3-2+i));

			mm0 = _mm_unpacklo_pi8(mm0, Null);
			mm1 = _mm_unpacklo_pi8(mm1, Null);
			mm2 = _mm_unpacklo_pi8(mm2, Null);
			mm3 = _mm_unpacklo_pi8(mm3, Null);
			mm4 = _mm_unpacklo_pi8(mm4, Null);
			mm5 = _mm_unpacklo_pi8(mm5, Null);

			tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_adds_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
			tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
			tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5

			tmp1 = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
			*((__m64*)&tmp_res[j][i]) = tmp;
			*((__m64*)&luma_res[j][i]) = tmp1;
			
			mm0 = *((__m64*)(src-w2-2+i));
			mm1 = *((__m64*)(src-w1-2+i));
			mm2 = *((__m64*)(src-2+i));
			mm3 = *((__m64*)(src+w1-2+i));
			mm4 = *((__m64*)(src+w2-2+i));
			mm5 = *((__m64*)(src+w3-2+i));

			mm0 = _mm_unpackhi_pi8(mm0, Null);
			mm1 = _mm_unpackhi_pi8(mm1, Null);
			mm2 = _mm_unpackhi_pi8(mm2, Null);
			mm3 = _mm_unpackhi_pi8(mm3, Null);
			mm4 = _mm_unpackhi_pi8(mm4, Null);
			mm5 = _mm_unpackhi_pi8(mm5, Null);

			tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_adds_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
			tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
			tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5

			tmp1 = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
			*((__m64*)&tmp_res[j][i+4]) = tmp;
			*((__m64*)&luma_res[j][i+4]) = tmp1;

		}
		src += w1;
	}

	tmp_src = &tmp_res[0][0];
	luma_src = &luma_res[0][3];
	for (j = 0; j < height; j++)
	{
		mm0 = *((__m64*)(tmp_src));
		mm1 = *((__m64*)(tmp_src+1));
		mm2 = *((__m64*)(tmp_src+2));
		mm3 = *((__m64*)(tmp_src+3));
		mm4 = *((__m64*)(tmp_src+4));
		mm5 = *((__m64*)(tmp_src+5));

		mm0 = _mm_unpacklo_pi16(Null, mm0);
		mm1 = _mm_unpacklo_pi16(Null, mm1);
		mm2 = _mm_unpacklo_pi16(Null, mm2);
		mm3 = _mm_unpacklo_pi16(Null, mm3);
		mm4 = _mm_unpacklo_pi16(Null, mm4);
		mm5 = _mm_unpacklo_pi16(Null, mm5);

		mm0 = _mm_srai_pi32(mm0,16);
		mm1 = _mm_srai_pi32(mm1,16);
		mm2 = _mm_srai_pi32(mm2,16);
		mm3 = _mm_srai_pi32(mm3,16);
		mm4 = _mm_srai_pi32(mm4,16);
		mm5 = _mm_srai_pi32(mm5,16);

		tmp = _mm_sub_pi32(_mm_slli_pi32(_mm_add_pi32(mm2,mm3),2),_mm_add_pi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi32(_mm_slli_pi32(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi32(_mm_add_pi32(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi32(_mm_add_pi32(tmp,coeff_512),10);//(result_block+512)>>10

		mm0 = mm4;
		mm1 = mm5;
		mm2 = *((__m64*)(tmp_src+6));
		mm3 = *((__m64*)(tmp_src+7));
		mm4 = *((__m64*)(tmp_src+8));
		mm5 = *((__m64*)(tmp_src+9));

		mm2 = _mm_unpacklo_pi16(Null, mm2);
		mm3 = _mm_unpacklo_pi16(Null, mm3);
		mm4 = _mm_unpacklo_pi16(Null, mm4);
		mm5 = _mm_unpacklo_pi16(Null, mm5);

		mm2 = _mm_srai_pi32(mm2,16);
		mm3 = _mm_srai_pi32(mm3,16);
		mm4 = _mm_srai_pi32(mm4,16);
		mm5 = _mm_srai_pi32(mm5,16);

		tmp1 = _mm_sub_pi32(_mm_slli_pi32(_mm_add_pi32(mm2,mm3),2),_mm_add_pi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_pi32(_mm_slli_pi32(tmp1,2),tmp1);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_pi32(_mm_add_pi32(mm0,mm5),tmp1); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp1 = _mm_srai_pi32(_mm_add_pi32(tmp1,coeff_512),10);//(result_block+512)>>10

		tmp = _mm_packs_pi32(tmp,tmp1);
		*((__m64*)&block[j][0]) = tmp;

		mm0 = *((__m64*)(tmp_src));
		mm1 = *((__m64*)(tmp_src+1));
		mm2 = *((__m64*)(tmp_src+2));
		mm3 = *((__m64*)(tmp_src+3));
		mm4 = *((__m64*)(tmp_src+4));
		mm5 = *((__m64*)(tmp_src+5));

		mm0 = _mm_unpackhi_pi16(Null, mm0);
		mm1 = _mm_unpackhi_pi16(Null, mm1);
		mm2 = _mm_unpackhi_pi16(Null, mm2);
		mm3 = _mm_unpackhi_pi16(Null, mm3);
		mm4 = _mm_unpackhi_pi16(Null, mm4);
		mm5 = _mm_unpackhi_pi16(Null, mm5);

		mm0 = _mm_srai_pi32(mm0,16);
		mm1 = _mm_srai_pi32(mm1,16);
		mm2 = _mm_srai_pi32(mm2,16);
		mm3 = _mm_srai_pi32(mm3,16);
		mm4 = _mm_srai_pi32(mm4,16);
		mm5 = _mm_srai_pi32(mm5,16);

		tmp = _mm_sub_pi32(_mm_slli_pi32(_mm_add_pi32(mm2,mm3),2),_mm_add_pi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi32(_mm_slli_pi32(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi32(_mm_add_pi32(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi32(_mm_add_pi32(tmp,coeff_512),10);//(result_block+512)>>10

		mm0 = mm4;
		mm1 = mm5;
		mm2 = *((__m64*)(tmp_src+6));
		mm3 = *((__m64*)(tmp_src+7));
		mm4 = *((__m64*)(tmp_src+8));
		mm5 = *((__m64*)(tmp_src+9));

		mm2 = _mm_unpackhi_pi16(Null, mm2);
		mm3 = _mm_unpackhi_pi16(Null, mm3);
		mm4 = _mm_unpackhi_pi16(Null, mm4);
		mm5 = _mm_unpackhi_pi16(Null, mm5);

		mm2 = _mm_srai_pi32(mm2,16);
		mm3 = _mm_srai_pi32(mm3,16);
		mm4 = _mm_srai_pi32(mm4,16);
		mm5 = _mm_srai_pi32(mm5,16);

		tmp1 = _mm_sub_pi32(_mm_slli_pi32(_mm_add_pi32(mm2,mm3),2),_mm_add_pi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_pi32(_mm_slli_pi32(tmp1,2),tmp1);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_pi32(_mm_add_pi32(mm0,mm5),tmp1); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp1 = _mm_srai_pi32(_mm_add_pi32(tmp1,coeff_512),10);//(result_block+512)>>10

		tmp = _mm_packs_pi32(tmp,tmp1);
		tmp1 = *((__m64*)&block[j][0]);
		mm4 = _mm_unpacklo_pi32(tmp1, tmp);
		mm5 = _mm_unpackhi_pi32(tmp1, tmp);

		pIn0 = *((__m64*) (luma_src));
		pIn1 = *((__m64*) (luma_src+4));
		tmp = _mm_packs_pu16(mm4,mm5);//__fast_iclip0_255((result_block+512)>>10)
		mm0 = _mm_packs_pu16(pIn0,pIn1);
		tmp = _mm_avg_pu8(tmp,mm0);
		*((__m64*)&block[j][0]) = tmp;

		tmp_src += 16;
		luma_src += 16;

	}
}
void  get_block_8xh_p32_sse2(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	byte *src;
	short *tmp_src;
	short *luma_src;
	int i,j;
	DO_ALIGN(16, short luma_res[16][16]);
	DO_ALIGN(16, short tmp_res[16][16]);
	const int w1 = stride;
	const int w2 = stride + stride;
	const int w3 = w2 + w1;
	src = pSrc;

	__m64 pIn0, pIn1;
	__m128i mm0,mm1,mm2,mm3,mm4,mm5,tmp,tmp1;
	const __m128i Null      = _mm_setzero_si128();
	const __m128i coeff_16  = _mm_load_si128((__m128i*)ncoeff_16);
	const __m128i coeff_512 = _mm_load_si128((__m128i*)ncoeff_512);

	for (j = 0; j < height; j++)
	{
		for(i = 0; i < 10; i += 8)
		{
			mm0 = _mm_loadl_epi64(((__m128i*) (src-w2-2+i)));
			mm1 = _mm_loadl_epi64(((__m128i*) (src-w1-2+i)));
			mm2 = _mm_loadl_epi64(((__m128i*) (src-2+i)));
			mm3 = _mm_loadl_epi64(((__m128i*) (src+w1-2+i)));
			mm4 = _mm_loadl_epi64(((__m128i*) (src+w2-2+i)));
			mm5 = _mm_loadl_epi64(((__m128i*) (src+w3-2+i)));

			mm0 = _mm_unpacklo_epi8(mm0, Null);
			mm1 = _mm_unpacklo_epi8(mm1, Null);
			mm2 = _mm_unpacklo_epi8(mm2, Null);
			mm3 = _mm_unpacklo_epi8(mm3, Null);
			mm4 = _mm_unpacklo_epi8(mm4, Null);
			mm5 = _mm_unpacklo_epi8(mm5, Null);

			tmp = _mm_sub_epi16(_mm_slli_epi16(_mm_add_epi16(mm2,mm3),2),_mm_adds_epi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
			tmp = _mm_add_epi16(_mm_slli_epi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
			tmp = _mm_add_epi16(_mm_add_epi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5

			_mm_store_si128(((__m128i*)&tmp_res[j][i]),tmp);

			tmp = _mm_srai_epi16(_mm_add_epi16(tmp,coeff_16),5);//(result_block+16)>>5

			_mm_store_si128(((__m128i*)&luma_res[j][i]),tmp);

		}
		src += w1;
	}

	tmp_src = &tmp_res[0][0];
	luma_src = &luma_res[0][3];
	for (j = 0; j < height; j++)
	{
		mm0 = _mm_loadl_epi64(((__m128i*) (tmp_src)));
		mm1 = _mm_loadl_epi64(((__m128i*) (tmp_src+1)));
		mm2 = _mm_loadl_epi64(((__m128i*) (tmp_src+2)));
		mm3 = _mm_loadl_epi64(((__m128i*) (tmp_src+3)));
		mm4 = _mm_loadl_epi64(((__m128i*) (tmp_src+4)));
		mm5 = _mm_loadl_epi64(((__m128i*) (tmp_src+5)));

		mm0 = _mm_unpacklo_epi16(Null, mm0);
		mm1 = _mm_unpacklo_epi16(Null, mm1);
		mm2 = _mm_unpacklo_epi16(Null, mm2);
		mm3 = _mm_unpacklo_epi16(Null, mm3);
		mm4 = _mm_unpacklo_epi16(Null, mm4);
		mm5 = _mm_unpacklo_epi16(Null, mm5);

		mm0 = _mm_srai_epi32(mm0,16);
		mm1 = _mm_srai_epi32(mm1,16);
		mm2 = _mm_srai_epi32(mm2,16);
		mm3 = _mm_srai_epi32(mm3,16);
		mm4 = _mm_srai_epi32(mm4,16);
		mm5 = _mm_srai_epi32(mm5,16);

		tmp = _mm_sub_epi32(_mm_slli_epi32(_mm_add_epi32(mm2,mm3),2),_mm_add_epi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi32(_mm_slli_epi32(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi32(_mm_add_epi32(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_epi32(_mm_add_epi32(tmp,coeff_512),10);//(result_block+512)>>10

		mm0 = mm4;
		mm1 = mm5;
		mm2 = _mm_loadl_epi64(((__m128i*) (tmp_src+6)));
		mm3 = _mm_loadl_epi64(((__m128i*) (tmp_src+7)));
		mm4 = _mm_loadl_epi64(((__m128i*) (tmp_src+8)));
		mm5 = _mm_loadl_epi64(((__m128i*) (tmp_src+9)));

		mm2 = _mm_unpacklo_epi16(Null, mm2);
		mm3 = _mm_unpacklo_epi16(Null, mm3);
		mm4 = _mm_unpacklo_epi16(Null, mm4);
		mm5 = _mm_unpacklo_epi16(Null, mm5);

		mm2 = _mm_srai_epi32(mm2,16);
		mm3 = _mm_srai_epi32(mm3,16);
		mm4 = _mm_srai_epi32(mm4,16);
		mm5 = _mm_srai_epi32(mm5,16);

		tmp1 = _mm_sub_epi32(_mm_slli_epi32(_mm_add_epi32(mm2,mm3),2),_mm_add_epi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_epi32(_mm_slli_epi32(tmp1,2),tmp1);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_epi32(_mm_add_epi32(mm0,mm5),tmp1); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp1 = _mm_srai_epi32(_mm_add_epi32(tmp1,coeff_512),10);//(result_block+512)>>10

		tmp = _mm_packs_epi32(tmp,tmp1);

		pIn0 = *((__m64*) (luma_src));
		pIn1 = *((__m64*) (luma_src+4));
		mm0 = _mm_setr_epi64(pIn0,pIn1);
		tmp = _mm_packus_epi16(tmp,tmp);//__fast_iclip0_255((result_block+512)>>10)
		mm0 = _mm_packus_epi16(mm0,mm0);
		tmp = _mm_avg_epu8(tmp,mm0);

		_mm_storel_epi64((__m128i*)&block[j][0], tmp);

		tmp_src += 16;
		luma_src += 16;

	}
}
void  get_block_8xh_p33_sse(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	byte *src;
	int j;
	const int w1 = stride;
	const int w2 = stride + stride;
	const int w3 = w2 + w1;

	src = pSrc + w1;

	__m64 mm0, mm1, mm2, mm3, mm4, mm5, mm6, tmp;
	const __m64 Null     = _mm_setzero_si64();
	const __m64 coeff_16 = *((__m64*)ncoeff_16);

	for (j = 0; j < height; j++)
	{
		mm0 = *((__m64*)(src-2));
		mm1 = *((__m64*)(src-1));
		mm2 = *((__m64*)(src));
		mm3 = *((__m64*)(src+1));
		mm4 = *((__m64*)(src+2));
		mm5 = *((__m64*)(src+3));

		mm0 = _mm_unpacklo_pi8(mm0, Null);
		mm1 = _mm_unpacklo_pi8(mm1, Null);
		mm2 = _mm_unpacklo_pi8(mm2, Null);
		mm3 = _mm_unpacklo_pi8(mm3, Null);
		mm4 = _mm_unpacklo_pi8(mm4, Null);
		mm5 = _mm_unpacklo_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		*((DWORD*)&block[j][0]) = _m_to_int(tmp);
		
		mm0 = *((__m64*)(src-2));
		mm1 = *((__m64*)(src-1));
		mm2 = *((__m64*)(src));
		mm3 = *((__m64*)(src+1));
		mm4 = *((__m64*)(src+2));
		mm5 = *((__m64*)(src+3));

		mm0 = _mm_unpackhi_pi8(mm0, Null);
		mm1 = _mm_unpackhi_pi8(mm1, Null);
		mm2 = _mm_unpackhi_pi8(mm2, Null);
		mm3 = _mm_unpackhi_pi8(mm3, Null);
		mm4 = _mm_unpackhi_pi8(mm4, Null);
		mm5 = _mm_unpackhi_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		*((DWORD*)&block[j][4]) = _m_to_int(tmp);

		src += w1;
	}

	src = pSrc + 1;

	for (j = 0; j < height; j++)
	{
		mm0 = *((__m64*)(src-w2));
		mm1 = *((__m64*)(src-w1));
		mm2 = *((__m64*)(src));
		mm3 = *((__m64*)(src+w1));
		mm4 = *((__m64*)(src+w2));
		mm5 = *((__m64*)(src+w3));
		mm6 = *((__m64*)&block[j][0]);

		mm0 = _mm_unpacklo_pi8(mm0, Null);
		mm1 = _mm_unpacklo_pi8(mm1, Null);
		mm2 = _mm_unpacklo_pi8(mm2, Null);
		mm3 = _mm_unpacklo_pi8(mm3, Null);
		mm4 = _mm_unpacklo_pi8(mm4, Null);
		mm5 = _mm_unpacklo_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		tmp = _mm_avg_pu8(tmp,mm6);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		*((DWORD*)&block[j][0]) = _m_to_int(tmp);

		mm0 = *((__m64*)(src-w2));
		mm1 = *((__m64*)(src-w1));
		mm2 = *((__m64*)(src));
		mm3 = *((__m64*)(src+w1));
		mm4 = *((__m64*)(src+w2));
		mm5 = *((__m64*)(src+w3));
		mm6 = *((__m64*)&block[j][4]);

		mm0 = _mm_unpackhi_pi8(mm0, Null);
		mm1 = _mm_unpackhi_pi8(mm1, Null);
		mm2 = _mm_unpackhi_pi8(mm2, Null);
		mm3 = _mm_unpackhi_pi8(mm3, Null);
		mm4 = _mm_unpackhi_pi8(mm4, Null);
		mm5 = _mm_unpackhi_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		tmp = _mm_avg_pu8(tmp,mm6);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		*((DWORD*)&block[j][4]) = _m_to_int(tmp);

		src += w1;
	}
}

void  get_block_8xh_p33_sse2(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	byte *src;
	int j;
	const int w1 = stride;
	const int w2 = stride + stride;
	const int w3 = w2 + w1;

	src = pSrc + w1;

	__m128i mm0,mm1,mm2,mm3,mm4,mm5,mm6,tmp;
	const __m128i Null     = _mm_setzero_si128();
	const __m128i coeff_16 = _mm_load_si128((__m128i*)ncoeff_16);

	for (j = 0; j < height; j++)
	{
		mm0 = _mm_loadl_epi64(((__m128i*) (src-2)));
		mm1 = _mm_loadl_epi64(((__m128i*) (src-1)));
		mm2 = _mm_loadl_epi64(((__m128i*) (src)));
		mm3 = _mm_loadl_epi64(((__m128i*) (src+1)));
		mm4 = _mm_loadl_epi64(((__m128i*) (src+2)));
		mm5 = _mm_loadl_epi64(((__m128i*) (src+3)));

		mm0 = _mm_unpacklo_epi8(mm0, Null);
		mm1 = _mm_unpacklo_epi8(mm1, Null);
		mm2 = _mm_unpacklo_epi8(mm2, Null);
		mm3 = _mm_unpacklo_epi8(mm3, Null);
		mm4 = _mm_unpacklo_epi8(mm4, Null);
		mm5 = _mm_unpacklo_epi8(mm5, Null);

		tmp = _mm_sub_epi16(_mm_slli_epi16(_mm_add_epi16(mm2,mm3),2),_mm_add_epi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_slli_epi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_add_epi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_epi16(_mm_add_epi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packus_epi16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		_mm_storel_epi64((__m128i*)&block[j][0], tmp);

		src += w1;
	}

	src = pSrc + 1;

	mm0 = _mm_loadl_epi64(((__m128i*) (src-w2)));
	mm1 = _mm_loadl_epi64(((__m128i*) (src-w1)));
	mm2 = _mm_loadl_epi64(((__m128i*) (src)));
	mm3 = _mm_loadl_epi64(((__m128i*) (src+w1)));
	mm4 = _mm_loadl_epi64(((__m128i*) (src+w2)));
	mm5 = _mm_loadl_epi64(((__m128i*) (src+w3)));
	mm6 = _mm_loadl_epi64(((__m128i*)&block[0][0]));

	mm0 = _mm_unpacklo_epi8(mm0, Null);
	mm1 = _mm_unpacklo_epi8(mm1, Null);
	mm2 = _mm_unpacklo_epi8(mm2, Null);
	mm3 = _mm_unpacklo_epi8(mm3, Null);
	mm4 = _mm_unpacklo_epi8(mm4, Null);
	mm5 = _mm_unpacklo_epi8(mm5, Null);

	tmp = _mm_sub_epi16(_mm_slli_epi16(_mm_add_epi16(mm2,mm3),2),_mm_add_epi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
	tmp = _mm_add_epi16(_mm_slli_epi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
	tmp = _mm_add_epi16(_mm_add_epi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
	tmp = _mm_srai_epi16(_mm_add_epi16(tmp,coeff_16),5);//(result_block+16)>>5
	tmp = _mm_packus_epi16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
	tmp = _mm_avg_epu8(tmp,mm6);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
	_mm_storel_epi64((__m128i*)&block[0][0], tmp);

	src += w1;

	for (j = 1; j < height; j++)
	{
		mm0 = mm1;
		mm1 = mm2;
		mm2 = mm3;
		mm3 = mm4;
		mm4 = mm5;

		mm5 = _mm_loadl_epi64(((__m128i*) (src+w3)));
		mm5 = _mm_unpacklo_epi8(mm5, Null);

		mm6 = _mm_loadl_epi64(((__m128i*)&block[j][0]));

		tmp = _mm_sub_epi16(_mm_slli_epi16(_mm_add_epi16(mm2,mm3),2),_mm_add_epi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_slli_epi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_add_epi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_epi16(_mm_add_epi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packus_epi16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		tmp = _mm_avg_epu8(tmp,mm6);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		_mm_storel_epi64((__m128i*)&block[j][0], tmp);

		src += w1;
	}
}

// The SSE2 code of get_block_16xh_pxx
void  get_block_16xh_p00_sse(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	imgpel *pblock = (imgpel *) &block[0][0];
	__m64 mm0, mm1, mm2, mm3, mm4, mm5, mm6, mm7;
	for(int i = height; i > 0; i-=8)
	{
		mm0 = *((__m64*)(pSrc+0*stride));
		mm1 = *((__m64*)(pSrc+1*stride));
		mm2 = *((__m64*)(pSrc+2*stride));
		mm3 = *((__m64*)(pSrc+3*stride));
		mm4 = *((__m64*)(pSrc+4*stride));
		mm5 = *((__m64*)(pSrc+5*stride));
		mm6 = *((__m64*)(pSrc+6*stride));
		mm7 = *((__m64*)(pSrc+7*stride));

		*((__m64*)(pblock+0*LUMA_BLOCK_SIZE)) = mm0;
		*((__m64*)(pblock+1*LUMA_BLOCK_SIZE)) = mm1;
		*((__m64*)(pblock+2*LUMA_BLOCK_SIZE)) = mm2;
		*((__m64*)(pblock+3*LUMA_BLOCK_SIZE)) = mm3;
		*((__m64*)(pblock+4*LUMA_BLOCK_SIZE)) = mm4;
		*((__m64*)(pblock+5*LUMA_BLOCK_SIZE)) = mm5;
		*((__m64*)(pblock+6*LUMA_BLOCK_SIZE)) = mm6;
		*((__m64*)(pblock+7*LUMA_BLOCK_SIZE)) = mm7;

		mm0 = *((__m64*)(pSrc+0*stride+8));
		mm1 = *((__m64*)(pSrc+1*stride+8));
		mm2 = *((__m64*)(pSrc+2*stride+8));
		mm3 = *((__m64*)(pSrc+3*stride+8));
		mm4 = *((__m64*)(pSrc+4*stride+8));
		mm5 = *((__m64*)(pSrc+5*stride+8));
		mm6 = *((__m64*)(pSrc+6*stride+8));
		mm7 = *((__m64*)(pSrc+7*stride+8));

		*((__m64*)(pblock+0*LUMA_BLOCK_SIZE+8)) = mm0;
		*((__m64*)(pblock+1*LUMA_BLOCK_SIZE+8)) = mm1;
		*((__m64*)(pblock+2*LUMA_BLOCK_SIZE+8)) = mm2;
		*((__m64*)(pblock+3*LUMA_BLOCK_SIZE+8)) = mm3;
		*((__m64*)(pblock+4*LUMA_BLOCK_SIZE+8)) = mm4;
		*((__m64*)(pblock+5*LUMA_BLOCK_SIZE+8)) = mm5;
		*((__m64*)(pblock+6*LUMA_BLOCK_SIZE+8)) = mm6;
		*((__m64*)(pblock+7*LUMA_BLOCK_SIZE+8)) = mm7;

		pSrc += 8*stride;
	}
}

void  get_block_16xh_p00_sse2(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	imgpel *pblock = (imgpel *) &block[0][0];
	__m128i xmm0,xmm1,xmm2,xmm3,xmm4,xmm5,xmm6,xmm7;
	for(int i = height; i > 0; i-=8)
	{
		xmm0 = _mm_loadu_si128((__m128i*)(pSrc+0*stride));
		xmm1 = _mm_loadu_si128((__m128i*)(pSrc+1*stride));
		xmm2 = _mm_loadu_si128((__m128i*)(pSrc+2*stride));
		xmm3 = _mm_loadu_si128((__m128i*)(pSrc+3*stride));
		xmm4 = _mm_loadu_si128((__m128i*)(pSrc+4*stride));
		xmm5 = _mm_loadu_si128((__m128i*)(pSrc+5*stride));
		xmm6 = _mm_loadu_si128((__m128i*)(pSrc+6*stride));
		xmm7 = _mm_loadu_si128((__m128i*)(pSrc+7*stride));
		_mm_store_si128((__m128i*)(pblock+0*LUMA_BLOCK_SIZE), xmm0);
		_mm_store_si128((__m128i*)(pblock+1*LUMA_BLOCK_SIZE), xmm1);
		_mm_store_si128((__m128i*)(pblock+2*LUMA_BLOCK_SIZE), xmm2);
		_mm_store_si128((__m128i*)(pblock+3*LUMA_BLOCK_SIZE), xmm3);
		_mm_store_si128((__m128i*)(pblock+4*LUMA_BLOCK_SIZE), xmm4);
		_mm_store_si128((__m128i*)(pblock+5*LUMA_BLOCK_SIZE), xmm5);
		_mm_store_si128((__m128i*)(pblock+6*LUMA_BLOCK_SIZE), xmm6);
		_mm_store_si128((__m128i*)(pblock+7*LUMA_BLOCK_SIZE), xmm7);
		pSrc += 8*stride;
	}
}

void  get_block_16xh_p01_sse(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int j;
	byte *src;
	const int w1 = stride;
	const int w2 = stride + stride;
	const int w3 = w2 + w1;
	src = pSrc;

	__m64 mm0, mm1, mm2, mm3, mm4, mm5, tmp;
	const __m64 Null     = _mm_setzero_si64();
	const __m64 coeff_16 = *((__m64*)ncoeff_16);

	for (j = 0; j < height; j++)
	{
		mm0 = *((__m64*)(src-w2));
		mm1 = *((__m64*)(src-w1));
		mm2 = *((__m64*)(src));
		mm3 = *((__m64*)(src+w1));
		mm4 = *((__m64*)(src+w2));
		mm5 = *((__m64*)(src+w3));

		mm0 = _mm_unpacklo_pi8(mm0, Null);
		mm1 = _mm_unpacklo_pi8(mm1, Null);
		mm2 = _mm_unpacklo_pi8(mm2, Null);
		mm3 = _mm_unpacklo_pi8(mm3, Null);
		mm4 = _mm_unpacklo_pi8(mm4, Null);
		mm5 = _mm_unpacklo_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		mm2 = _mm_packs_pu16(mm2,mm2);
		tmp = _mm_avg_pu8(tmp,mm2);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		*((DWORD*)&block[j][0]) = _m_to_int(tmp);

		mm0 = *((__m64*)(src-w2));
		mm1 = *((__m64*)(src-w1));
		mm2 = *((__m64*)(src));
		mm3 = *((__m64*)(src+w1));
		mm4 = *((__m64*)(src+w2));
		mm5 = *((__m64*)(src+w3));

		mm0 = _mm_unpackhi_pi8(mm0, Null);
		mm1 = _mm_unpackhi_pi8(mm1, Null);
		mm2 = _mm_unpackhi_pi8(mm2, Null);
		mm3 = _mm_unpackhi_pi8(mm3, Null);
		mm4 = _mm_unpackhi_pi8(mm4, Null);
		mm5 = _mm_unpackhi_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		mm2 = _mm_packs_pu16(mm2,mm2);
		tmp = _mm_avg_pu8(tmp,mm2);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		*((DWORD*)&block[j][4]) = _m_to_int(tmp);


		mm0 = *((__m64*)(src-w2+8));
		mm1 = *((__m64*)(src-w1+8));
		mm2 = *((__m64*)(src+8));
		mm3 = *((__m64*)(src+w1+8));
		mm4 = *((__m64*)(src+w2+8));
		mm5 = *((__m64*)(src+w3+8));

		mm0 = _mm_unpacklo_pi8(mm0, Null);
		mm1 = _mm_unpacklo_pi8(mm1, Null);
		mm2 = _mm_unpacklo_pi8(mm2, Null);
		mm3 = _mm_unpacklo_pi8(mm3, Null);
		mm4 = _mm_unpacklo_pi8(mm4, Null);
		mm5 = _mm_unpacklo_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		mm2 = _mm_packs_pu16(mm2,mm2);
		tmp = _mm_avg_pu8(tmp,mm2);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		*((DWORD*)&block[j][8]) = _m_to_int(tmp);

		mm0 = *((__m64*)(src-w2+8));
		mm1 = *((__m64*)(src-w1+8));
		mm2 = *((__m64*)(src+8));
		mm3 = *((__m64*)(src+w1+8));
		mm4 = *((__m64*)(src+w2+8));
		mm5 = *((__m64*)(src+w3+8));

		mm0 = _mm_unpackhi_pi8(mm0, Null);
		mm1 = _mm_unpackhi_pi8(mm1, Null);
		mm2 = _mm_unpackhi_pi8(mm2, Null);
		mm3 = _mm_unpackhi_pi8(mm3, Null);
		mm4 = _mm_unpackhi_pi8(mm4, Null);
		mm5 = _mm_unpackhi_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		mm2 = _mm_packs_pu16(mm2,mm2);
		tmp = _mm_avg_pu8(tmp,mm2);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		*((DWORD*)&block[j][12]) = _m_to_int(tmp);

		src += w1;
	}
}

void  get_block_16xh_p01_sse2(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int j;
	byte *src;
	const int w1 = stride;
	const int w2 = stride + stride;
	const int w3 = w2 + w1;
	src = pSrc;

	__m128i mm0,mm1,mm2,mm3,mm4,mm5,tmp;
	const __m128i Null     = _mm_setzero_si128();
	const __m128i coeff_16 = _mm_load_si128((__m128i*)ncoeff_16);

	for (j = 0; j < height; j++)
	{
		mm0 = _mm_loadl_epi64(((__m128i*) (src-w2)));
		mm1 = _mm_loadl_epi64(((__m128i*) (src-w1)));
		mm2 = _mm_loadl_epi64(((__m128i*) (src)));
		mm3 = _mm_loadl_epi64(((__m128i*) (src+w1)));
		mm4 = _mm_loadl_epi64(((__m128i*) (src+w2)));
		mm5 = _mm_loadl_epi64(((__m128i*) (src+w3)));

		mm0 = _mm_unpacklo_epi8(mm0, Null);
		mm1 = _mm_unpacklo_epi8(mm1, Null);
		mm2 = _mm_unpacklo_epi8(mm2, Null);
		mm3 = _mm_unpacklo_epi8(mm3, Null);
		mm4 = _mm_unpacklo_epi8(mm4, Null);
		mm5 = _mm_unpacklo_epi8(mm5, Null);

		tmp = _mm_sub_epi16(_mm_slli_epi16(_mm_add_epi16(mm2,mm3),2),_mm_add_epi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_slli_epi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_add_epi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_epi16(_mm_add_epi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packus_epi16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		mm2 = _mm_packus_epi16(mm2,mm2);
		tmp = _mm_avg_epu8(tmp,mm2);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		_mm_storel_epi64((__m128i*)&block[j][0], tmp);

		mm0 = _mm_loadl_epi64(((__m128i*) (src-w2+8)));
		mm1 = _mm_loadl_epi64(((__m128i*) (src-w1+8)));
		mm2 = _mm_loadl_epi64(((__m128i*) (src+8)));
		mm3 = _mm_loadl_epi64(((__m128i*) (src+w1+8)));
		mm4 = _mm_loadl_epi64(((__m128i*) (src+w2+8)));
		mm5 = _mm_loadl_epi64(((__m128i*) (src+w3+8)));

		mm0 = _mm_unpacklo_epi8(mm0, Null);
		mm1 = _mm_unpacklo_epi8(mm1, Null);
		mm2 = _mm_unpacklo_epi8(mm2, Null);
		mm3 = _mm_unpacklo_epi8(mm3, Null);
		mm4 = _mm_unpacklo_epi8(mm4, Null);
		mm5 = _mm_unpacklo_epi8(mm5, Null);

		tmp = _mm_sub_epi16(_mm_slli_epi16(_mm_add_epi16(mm2,mm3),2),_mm_add_epi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_slli_epi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_add_epi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_epi16(_mm_add_epi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packus_epi16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		mm2 = _mm_packus_epi16(mm2,mm2);
		tmp = _mm_avg_epu8(tmp,mm2);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		_mm_storel_epi64((__m128i*)&block[j][8], tmp);

		src += w1;
	}
}

void  get_block_16xh_p02_sse(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int j;
	byte *src;
	const int w1 = stride;
	const int w2 = stride + stride;
	const int w3 = w2 + w1;
	src = pSrc;

	__m64 mm0, mm1, mm2, mm3, mm4, mm5, tmp;
	const __m64 Null     = _mm_setzero_si64();
	const __m64 coeff_16 = *((__m64*)ncoeff_16);

	for (j = 0; j < height; j++)
	{
		mm0 = *((__m64*)(src-w2));
		mm1 = *((__m64*)(src-w1));
		mm2 = *((__m64*)(src));
		mm3 = *((__m64*)(src+w1));
		mm4 = *((__m64*)(src+w2));
		mm5 = *((__m64*)(src+w3));

		mm0 = _mm_unpacklo_pi8(mm0, Null);
		mm1 = _mm_unpacklo_pi8(mm1, Null);
		mm2 = _mm_unpacklo_pi8(mm2, Null);
		mm3 = _mm_unpacklo_pi8(mm3, Null);
		mm4 = _mm_unpacklo_pi8(mm4, Null);
		mm5 = _mm_unpacklo_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		*((DWORD*)&block[j][0]) = _m_to_int(tmp);

		mm0 = *((__m64*)(src-w2));
		mm1 = *((__m64*)(src-w1));
		mm2 = *((__m64*)(src));
		mm3 = *((__m64*)(src+w1));
		mm4 = *((__m64*)(src+w2));
		mm5 = *((__m64*)(src+w3));

		mm0 = _mm_unpackhi_pi8(mm0, Null);
		mm1 = _mm_unpackhi_pi8(mm1, Null);
		mm2 = _mm_unpackhi_pi8(mm2, Null);
		mm3 = _mm_unpackhi_pi8(mm3, Null);
		mm4 = _mm_unpackhi_pi8(mm4, Null);
		mm5 = _mm_unpackhi_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		*((DWORD*)&block[j][4]) = _m_to_int(tmp);


		mm0 = *((__m64*)(src-w2+8));
		mm1 = *((__m64*)(src-w1+8));
		mm2 = *((__m64*)(src+8));
		mm3 = *((__m64*)(src+w1+8));
		mm4 = *((__m64*)(src+w2+8));
		mm5 = *((__m64*)(src+w3+8));

		mm0 = _mm_unpacklo_pi8(mm0, Null);
		mm1 = _mm_unpacklo_pi8(mm1, Null);
		mm2 = _mm_unpacklo_pi8(mm2, Null);
		mm3 = _mm_unpacklo_pi8(mm3, Null);
		mm4 = _mm_unpacklo_pi8(mm4, Null);
		mm5 = _mm_unpacklo_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		*((DWORD*)&block[j][8]) = _m_to_int(tmp);
		
		mm0 = *((__m64*)(src-w2+8));
		mm1 = *((__m64*)(src-w1+8));
		mm2 = *((__m64*)(src+8));
		mm3 = *((__m64*)(src+w1+8));
		mm4 = *((__m64*)(src+w2+8));
		mm5 = *((__m64*)(src+w3+8));

		mm0 = _mm_unpackhi_pi8(mm0, Null);
		mm1 = _mm_unpackhi_pi8(mm1, Null);
		mm2 = _mm_unpackhi_pi8(mm2, Null);
		mm3 = _mm_unpackhi_pi8(mm3, Null);
		mm4 = _mm_unpackhi_pi8(mm4, Null);
		mm5 = _mm_unpackhi_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		*((DWORD*)&block[j][12]) = _m_to_int(tmp);

		src += w1;
	}
}
void  get_block_16xh_p02_sse2(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int j;
	byte *src;
	const int w1 = stride;
	const int w2 = stride + stride;
	const int w3 = w2 + w1;
	src = pSrc;

	__m128i mm0,mm1,mm2,mm3,mm4,mm5,tmp;
	const __m128i Null     = _mm_setzero_si128();
	const __m128i coeff_16 = _mm_load_si128((__m128i*)ncoeff_16);

	for (j = 0; j < height; j++)
	{
		mm0 = _mm_loadl_epi64(((__m128i*) (src-w2)));
		mm1 = _mm_loadl_epi64(((__m128i*) (src-w1)));
		mm2 = _mm_loadl_epi64(((__m128i*) (src)));
		mm3 = _mm_loadl_epi64(((__m128i*) (src+w1)));
		mm4 = _mm_loadl_epi64(((__m128i*) (src+w2)));
		mm5 = _mm_loadl_epi64(((__m128i*) (src+w3)));

		mm0 = _mm_unpacklo_epi8(mm0, Null);
		mm1 = _mm_unpacklo_epi8(mm1, Null);
		mm2 = _mm_unpacklo_epi8(mm2, Null);
		mm3 = _mm_unpacklo_epi8(mm3, Null);
		mm4 = _mm_unpacklo_epi8(mm4, Null);
		mm5 = _mm_unpacklo_epi8(mm5, Null);

		tmp = _mm_sub_epi16(_mm_slli_epi16(_mm_add_epi16(mm2,mm3),2),_mm_add_epi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_slli_epi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_add_epi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_epi16(_mm_add_epi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packus_epi16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		_mm_storel_epi64((__m128i*)&block[j][0], tmp);

		mm0 = _mm_loadl_epi64(((__m128i*) (src-w2+8)));
		mm1 = _mm_loadl_epi64(((__m128i*) (src-w1+8)));
		mm2 = _mm_loadl_epi64(((__m128i*) (src+8)));
		mm3 = _mm_loadl_epi64(((__m128i*) (src+w1+8)));
		mm4 = _mm_loadl_epi64(((__m128i*) (src+w2+8)));
		mm5 = _mm_loadl_epi64(((__m128i*) (src+w3+8)));

		mm0 = _mm_unpacklo_epi8(mm0, Null);
		mm1 = _mm_unpacklo_epi8(mm1, Null);
		mm2 = _mm_unpacklo_epi8(mm2, Null);
		mm3 = _mm_unpacklo_epi8(mm3, Null);
		mm4 = _mm_unpacklo_epi8(mm4, Null);
		mm5 = _mm_unpacklo_epi8(mm5, Null);

		tmp = _mm_sub_epi16(_mm_slli_epi16(_mm_add_epi16(mm2,mm3),2),_mm_add_epi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_slli_epi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_add_epi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_epi16(_mm_add_epi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packus_epi16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		_mm_storel_epi64((__m128i*)&block[j][8], tmp);

		src += w1;
	}
}


void  get_block_16xh_p03_sse(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int j;
	byte *src;
	const int w1 = stride;
	const int w2 = stride + stride;
	const int w3 = w2 + w1;
	src = pSrc;

	__m64 mm0, mm1, mm2, mm3, mm4, mm5, tmp;
	const __m64 Null     = _mm_setzero_si64();
	const __m64 coeff_16 = *((__m64*)ncoeff_16);

	for (j = 0; j < height; j++)
	{
		mm0 = *((__m64*)(src-w2));
		mm1 = *((__m64*)(src-w1));
		mm2 = *((__m64*)(src));
		mm3 = *((__m64*)(src+w1));
		mm4 = *((__m64*)(src+w2));
		mm5 = *((__m64*)(src+w3));

		mm0 = _mm_unpacklo_pi8(mm0, Null);
		mm1 = _mm_unpacklo_pi8(mm1, Null);
		mm2 = _mm_unpacklo_pi8(mm2, Null);
		mm3 = _mm_unpacklo_pi8(mm3, Null);
		mm4 = _mm_unpacklo_pi8(mm4, Null);
		mm5 = _mm_unpacklo_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		mm3 = _mm_packs_pu16(mm3,mm3);
		tmp = _mm_avg_pu8(tmp,mm3);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		*((DWORD*)&block[j][0]) = _m_to_int(tmp);

		mm0 = *((__m64*)(src-w2));
		mm1 = *((__m64*)(src-w1));
		mm2 = *((__m64*)(src));
		mm3 = *((__m64*)(src+w1));
		mm4 = *((__m64*)(src+w2));
		mm5 = *((__m64*)(src+w3));

		mm0 = _mm_unpackhi_pi8(mm0, Null);
		mm1 = _mm_unpackhi_pi8(mm1, Null);
		mm2 = _mm_unpackhi_pi8(mm2, Null);
		mm3 = _mm_unpackhi_pi8(mm3, Null);
		mm4 = _mm_unpackhi_pi8(mm4, Null);
		mm5 = _mm_unpackhi_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		mm3 = _mm_packs_pu16(mm3,mm3);
		tmp = _mm_avg_pu8(tmp,mm3);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		*((DWORD*)&block[j][4]) = _m_to_int(tmp);


		mm0 = *((__m64*)(src-w2+8));
		mm1 = *((__m64*)(src-w1+8));
		mm2 = *((__m64*)(src+8));
		mm3 = *((__m64*)(src+w1+8));
		mm4 = *((__m64*)(src+w2+8));
		mm5 = *((__m64*)(src+w3+8));

		mm0 = _mm_unpacklo_pi8(mm0, Null);
		mm1 = _mm_unpacklo_pi8(mm1, Null);
		mm2 = _mm_unpacklo_pi8(mm2, Null);
		mm3 = _mm_unpacklo_pi8(mm3, Null);
		mm4 = _mm_unpacklo_pi8(mm4, Null);
		mm5 = _mm_unpacklo_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		mm3 = _mm_packs_pu16(mm3,mm3);
		tmp = _mm_avg_pu8(tmp,mm3);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		*((DWORD*)&block[j][8]) = _m_to_int(tmp);
		
		mm0 = *((__m64*)(src-w2+8));
		mm1 = *((__m64*)(src-w1+8));
		mm2 = *((__m64*)(src+8));
		mm3 = *((__m64*)(src+w1+8));
		mm4 = *((__m64*)(src+w2+8));
		mm5 = *((__m64*)(src+w3+8));

		mm0 = _mm_unpackhi_pi8(mm0, Null);
		mm1 = _mm_unpackhi_pi8(mm1, Null);
		mm2 = _mm_unpackhi_pi8(mm2, Null);
		mm3 = _mm_unpackhi_pi8(mm3, Null);
		mm4 = _mm_unpackhi_pi8(mm4, Null);
		mm5 = _mm_unpackhi_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		mm3 = _mm_packs_pu16(mm3,mm3);
		tmp = _mm_avg_pu8(tmp,mm3);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		*((DWORD*)&block[j][12]) = _m_to_int(tmp);

		src += w1;
	}
}
void  get_block_16xh_p03_sse2(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int j;
	byte *src;
	const int w1 = stride;
	const int w2 = stride + stride;
	const int w3 = w2 + w1;
	src = pSrc;

	__m128i mm0,mm1,mm2,mm3,mm4,mm5,tmp;
	const __m128i Null     = _mm_setzero_si128();
	const __m128i coeff_16 = _mm_load_si128((__m128i*)ncoeff_16);

	for (j = 0; j < height; j++)
	{
		mm0 = _mm_loadl_epi64(((__m128i*) (src-w2)));
		mm1 = _mm_loadl_epi64(((__m128i*) (src-w1)));
		mm2 = _mm_loadl_epi64(((__m128i*) (src)));
		mm3 = _mm_loadl_epi64(((__m128i*) (src+w1)));
		mm4 = _mm_loadl_epi64(((__m128i*) (src+w2)));
		mm5 = _mm_loadl_epi64(((__m128i*) (src+w3)));

		mm0 = _mm_unpacklo_epi8(mm0, Null);
		mm1 = _mm_unpacklo_epi8(mm1, Null);
		mm2 = _mm_unpacklo_epi8(mm2, Null);
		mm3 = _mm_unpacklo_epi8(mm3, Null);
		mm4 = _mm_unpacklo_epi8(mm4, Null);
		mm5 = _mm_unpacklo_epi8(mm5, Null);

		tmp = _mm_sub_epi16(_mm_slli_epi16(_mm_add_epi16(mm2,mm3),2),_mm_add_epi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_slli_epi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_add_epi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_epi16(_mm_add_epi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packus_epi16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		mm3 = _mm_packus_epi16(mm3,mm3);
		tmp = _mm_avg_epu8(tmp,mm3);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		_mm_storel_epi64((__m128i*)&block[j][0], tmp);

		mm0 = _mm_loadl_epi64(((__m128i*) (src-w2+8)));
		mm1 = _mm_loadl_epi64(((__m128i*) (src-w1+8)));
		mm2 = _mm_loadl_epi64(((__m128i*) (src+8)));
		mm3 = _mm_loadl_epi64(((__m128i*) (src+w1+8)));
		mm4 = _mm_loadl_epi64(((__m128i*) (src+w2+8)));
		mm5 = _mm_loadl_epi64(((__m128i*) (src+w3+8)));

		mm0 = _mm_unpacklo_epi8(mm0, Null);
		mm1 = _mm_unpacklo_epi8(mm1, Null);
		mm2 = _mm_unpacklo_epi8(mm2, Null);
		mm3 = _mm_unpacklo_epi8(mm3, Null);
		mm4 = _mm_unpacklo_epi8(mm4, Null);
		mm5 = _mm_unpacklo_epi8(mm5, Null);

		tmp = _mm_sub_epi16(_mm_slli_epi16(_mm_add_epi16(mm2,mm3),2),_mm_add_epi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_slli_epi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_add_epi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_epi16(_mm_add_epi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packus_epi16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		mm3 = _mm_packus_epi16(mm3,mm3);
		tmp = _mm_avg_epu8(tmp,mm3);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		_mm_storel_epi64((__m128i*)&block[j][8], tmp);

		src += w1;
	}
}


void  get_block_16xh_p10_sse(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int j;
	byte *src;
	const int w1 = stride;
	src = pSrc;

	__m64 mm0, mm1, mm2, mm3, mm4, mm5, tmp;
	const __m64 Null     = _mm_setzero_si64();
	const __m64 coeff_16 = *((__m64*)ncoeff_16);

	for (j = 0; j < height; j++)
	{
		
		mm0 = *((__m64*)(src-2));
		mm1 = *((__m64*)(src-1));
		mm2 = *((__m64*)(src));
		mm3 = *((__m64*)(src+1));
		mm4 = *((__m64*)(src+2));
		mm5 = *((__m64*)(src+3));

		mm0 = _mm_unpacklo_pi8(mm0, Null);
		mm1 = _mm_unpacklo_pi8(mm1, Null);
		mm2 = _mm_unpacklo_pi8(mm2, Null);
		mm3 = _mm_unpacklo_pi8(mm3, Null);
		mm4 = _mm_unpacklo_pi8(mm4, Null);
		mm5 = _mm_unpacklo_pi8(mm5, Null);


		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		mm2 = _mm_packs_pu16(mm2,mm2);
		tmp = _mm_avg_pu8(tmp,mm2);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		*((DWORD*)&block[j][0]) = _m_to_int(tmp);
		
		mm0 = *((__m64*)(src-2));
		mm1 = *((__m64*)(src-1));
		mm2 = *((__m64*)(src));
		mm3 = *((__m64*)(src+1));
		mm4 = *((__m64*)(src+2));
		mm5 = *((__m64*)(src+3));

		mm0 = _mm_unpackhi_pi8(mm0, Null);
		mm1 = _mm_unpackhi_pi8(mm1, Null);
		mm2 = _mm_unpackhi_pi8(mm2, Null);
		mm3 = _mm_unpackhi_pi8(mm3, Null);
		mm4 = _mm_unpackhi_pi8(mm4, Null);
		mm5 = _mm_unpackhi_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		mm2 = _mm_packs_pu16(mm2,mm2);
		tmp = _mm_avg_pu8(tmp,mm2);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		*((DWORD*)&block[j][4]) = _m_to_int(tmp);

		
		mm0 = *((__m64*)(src+6));
		mm1 = *((__m64*)(src+7));
		mm2 = *((__m64*)(src+8));
		mm3 = *((__m64*)(src+9));
		mm4 = *((__m64*)(src+10));
		mm5 = *((__m64*)(src+11));

		mm0 = _mm_unpacklo_pi8(mm0, Null);
		mm1 = _mm_unpacklo_pi8(mm1, Null);
		mm2 = _mm_unpacklo_pi8(mm2, Null);
		mm3 = _mm_unpacklo_pi8(mm3, Null);
		mm4 = _mm_unpacklo_pi8(mm4, Null);
		mm5 = _mm_unpacklo_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		mm2 = _mm_packs_pu16(mm2,mm2);
		tmp = _mm_avg_pu8(tmp,mm2);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		*((DWORD*)&block[j][8]) = _m_to_int(tmp);

		mm0 = *((__m64*)(src+6));
		mm1 = *((__m64*)(src+7));
		mm2 = *((__m64*)(src+8));
		mm3 = *((__m64*)(src+9));
		mm4 = *((__m64*)(src+10));
		mm5 = *((__m64*)(src+11));

		mm0 = _mm_unpackhi_pi8(mm0, Null);
		mm1 = _mm_unpackhi_pi8(mm1, Null);
		mm2 = _mm_unpackhi_pi8(mm2, Null);
		mm3 = _mm_unpackhi_pi8(mm3, Null);
		mm4 = _mm_unpackhi_pi8(mm4, Null);
		mm5 = _mm_unpackhi_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		mm2 = _mm_packs_pu16(mm2,mm2);
		tmp = _mm_avg_pu8(tmp,mm2);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		*((DWORD*)&block[j][12]) = _m_to_int(tmp);
		
		src += w1;
	}
}
void  get_block_16xh_p10_sse2(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int j;
	byte *src;
	const int w1 = stride;
	src = pSrc;

	__m128i mm0,mm1,mm2,mm3,mm4,mm5,tmp;
	const __m128i Null     = _mm_setzero_si128();
	const __m128i coeff_16 = _mm_load_si128((__m128i*)ncoeff_16);

	for (j = 0; j < height; j++)
	{
		mm0 = _mm_loadl_epi64(((__m128i*) (src-2)));
		mm1 = _mm_loadl_epi64(((__m128i*) (src-1)));
		mm2 = _mm_loadl_epi64(((__m128i*) (src)));
		mm3 = _mm_loadl_epi64(((__m128i*) (src+1)));
		mm4 = _mm_loadl_epi64(((__m128i*) (src+2)));
		mm5 = _mm_loadl_epi64(((__m128i*) (src+3)));

		mm0 = _mm_unpacklo_epi8(mm0, Null);
		mm1 = _mm_unpacklo_epi8(mm1, Null);
		mm2 = _mm_unpacklo_epi8(mm2, Null);
		mm3 = _mm_unpacklo_epi8(mm3, Null);
		mm4 = _mm_unpacklo_epi8(mm4, Null);
		mm5 = _mm_unpacklo_epi8(mm5, Null);

		tmp = _mm_sub_epi16(_mm_slli_epi16(_mm_add_epi16(mm2,mm3),2),_mm_add_epi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_slli_epi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_add_epi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_epi16(_mm_add_epi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packus_epi16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		mm2 = _mm_packus_epi16(mm2,mm2);
		tmp = _mm_avg_epu8(tmp,mm2);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		_mm_storel_epi64((__m128i*)&block[j][0], tmp);

		mm0 = _mm_loadl_epi64(((__m128i*) (src+6)));
		mm1 = _mm_loadl_epi64(((__m128i*) (src+7)));
		mm2 = _mm_loadl_epi64(((__m128i*) (src+8)));
		mm3 = _mm_loadl_epi64(((__m128i*) (src+9)));
		mm4 = _mm_loadl_epi64(((__m128i*) (src+10)));
		mm5 = _mm_loadl_epi64(((__m128i*) (src+11)));

		mm0 = _mm_unpacklo_epi8(mm0, Null);
		mm1 = _mm_unpacklo_epi8(mm1, Null);
		mm2 = _mm_unpacklo_epi8(mm2, Null);
		mm3 = _mm_unpacklo_epi8(mm3, Null);
		mm4 = _mm_unpacklo_epi8(mm4, Null);
		mm5 = _mm_unpacklo_epi8(mm5, Null);

		tmp = _mm_sub_epi16(_mm_slli_epi16(_mm_add_epi16(mm2,mm3),2),_mm_add_epi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_slli_epi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_add_epi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_epi16(_mm_add_epi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packus_epi16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		mm2 = _mm_packus_epi16(mm2,mm2);
		tmp = _mm_avg_epu8(tmp,mm2);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		_mm_storel_epi64((__m128i*)&block[j][8], tmp);

		src += w1;
	}
}
void  get_block_16xh_p11_sse(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int j;
	byte *src;
	const int w1 = stride;
	const int w2 = stride + stride;
	const int w3 = w2 + w1;

	src = pSrc;

	__m64 mm0, mm1, mm2, mm3, mm4, mm5, mm6, tmp;
	const __m64 Null     = _mm_setzero_si64();
	const __m64 coeff_16 = *((__m64*)ncoeff_16);

	for (j = 0; j < height; j++)
	{
		mm0 = *((__m64*)(src-2));
		mm1 = *((__m64*)(src-1));
		mm2 = *((__m64*)(src));
		mm3 = *((__m64*)(src+1));
		mm4 = *((__m64*)(src+2));
		mm5 = *((__m64*)(src+3));

		mm0 = _mm_unpacklo_pi8(mm0, Null);
		mm1 = _mm_unpacklo_pi8(mm1, Null);
		mm2 = _mm_unpacklo_pi8(mm2, Null);
		mm3 = _mm_unpacklo_pi8(mm3, Null);
		mm4 = _mm_unpacklo_pi8(mm4, Null);
		mm5 = _mm_unpacklo_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		*((DWORD*)&block[j][0]) = _m_to_int(tmp);

		mm0 = *((__m64*)(src-2));
		mm1 = *((__m64*)(src-1));
		mm2 = *((__m64*)(src));
		mm3 = *((__m64*)(src+1));
		mm4 = *((__m64*)(src+2));
		mm5 = *((__m64*)(src+3));

		mm0 = _mm_unpackhi_pi8(mm0, Null);
		mm1 = _mm_unpackhi_pi8(mm1, Null);
		mm2 = _mm_unpackhi_pi8(mm2, Null);
		mm3 = _mm_unpackhi_pi8(mm3, Null);
		mm4 = _mm_unpackhi_pi8(mm4, Null);
		mm5 = _mm_unpackhi_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		*((DWORD*)&block[j][4]) = _m_to_int(tmp);


		mm0 = *((__m64*)(src+6));
		mm1 = *((__m64*)(src+7));
		mm2 = *((__m64*)(src+8));
		mm3 = *((__m64*)(src+9));
		mm4 = *((__m64*)(src+10));
		mm5 = *((__m64*)(src+11));

		mm0 = _mm_unpacklo_pi8(mm0, Null);
		mm1 = _mm_unpacklo_pi8(mm1, Null);
		mm2 = _mm_unpacklo_pi8(mm2, Null);
		mm3 = _mm_unpacklo_pi8(mm3, Null);
		mm4 = _mm_unpacklo_pi8(mm4, Null);
		mm5 = _mm_unpacklo_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		*((DWORD*)&block[j][8]) = _m_to_int(tmp);
			
		mm0 = *((__m64*)(src+6));
		mm1 = *((__m64*)(src+7));
		mm2 = *((__m64*)(src+8));
		mm3 = *((__m64*)(src+9));
		mm4 = *((__m64*)(src+10));
		mm5 = *((__m64*)(src+11));

		mm0 = _mm_unpackhi_pi8(mm0, Null);
		mm1 = _mm_unpackhi_pi8(mm1, Null);
		mm2 = _mm_unpackhi_pi8(mm2, Null);
		mm3 = _mm_unpackhi_pi8(mm3, Null);
		mm4 = _mm_unpackhi_pi8(mm4, Null);
		mm5 = _mm_unpackhi_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		*((DWORD*)&block[j][12]) = _m_to_int(tmp);
		
		src += w1;
	}

	src = pSrc;

	for (j = 0; j < height; j++)
	{
		mm0 = *((__m64*)(src-w2));
		mm1 = *((__m64*)(src-w1));
		mm2 = *((__m64*)(src));
		mm3 = *((__m64*)(src+w1));
		mm4 = *((__m64*)(src+w2));
		mm5 = *((__m64*)(src+w3));
		mm6 = *((__m64*)&block[j][0]);

		mm0 = _mm_unpacklo_pi8(mm0, Null);
		mm1 = _mm_unpacklo_pi8(mm1, Null);
		mm2 = _mm_unpacklo_pi8(mm2, Null);
		mm3 = _mm_unpacklo_pi8(mm3, Null);
		mm4 = _mm_unpacklo_pi8(mm4, Null);
		mm5 = _mm_unpacklo_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		tmp = _mm_avg_pu8(tmp,mm6);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		*((DWORD*)&block[j][0]) = _m_to_int(tmp);

		mm0 = *((__m64*)(src-w2));
		mm1 = *((__m64*)(src-w1));
		mm2 = *((__m64*)(src));
		mm3 = *((__m64*)(src+w1));
		mm4 = *((__m64*)(src+w2));
		mm5 = *((__m64*)(src+w3));
		mm6 = *((__m64*)&block[j][4]);

		mm0 = _mm_unpackhi_pi8(mm0, Null);
		mm1 = _mm_unpackhi_pi8(mm1, Null);
		mm2 = _mm_unpackhi_pi8(mm2, Null);
		mm3 = _mm_unpackhi_pi8(mm3, Null);
		mm4 = _mm_unpackhi_pi8(mm4, Null);
		mm5 = _mm_unpackhi_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		tmp = _mm_avg_pu8(tmp,mm6);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		*((DWORD*)&block[j][4]) = _m_to_int(tmp);

		
	  mm0 = *((__m64*)(src-w2+8));
		mm1 = *((__m64*)(src-w1+8));
		mm2 = *((__m64*)(src+8));
		mm3 = *((__m64*)(src+w1+8));
		mm4 = *((__m64*)(src+w2+8));
		mm5 = *((__m64*)(src+w3+8));
		mm6 = *((__m64*)&block[j][8]);

		mm0 = _mm_unpacklo_pi8(mm0, Null);
		mm1 = _mm_unpacklo_pi8(mm1, Null);
		mm2 = _mm_unpacklo_pi8(mm2, Null);
		mm3 = _mm_unpacklo_pi8(mm3, Null);
		mm4 = _mm_unpacklo_pi8(mm4, Null);
		mm5 = _mm_unpacklo_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		tmp = _mm_avg_pu8(tmp,mm6);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		*((DWORD*)&block[j][8]) = _m_to_int(tmp);

		mm0 = *((__m64*)(src-w2+8));
		mm1 = *((__m64*)(src-w1+8));
		mm2 = *((__m64*)(src+8));
		mm3 = *((__m64*)(src+w1+8));
		mm4 = *((__m64*)(src+w2+8));
		mm5 = *((__m64*)(src+w3+8));
		mm6 = *((__m64*)&block[j][12]);

		mm0 = _mm_unpackhi_pi8(mm0, Null);
		mm1 = _mm_unpackhi_pi8(mm1, Null);
		mm2 = _mm_unpackhi_pi8(mm2, Null);
		mm3 = _mm_unpackhi_pi8(mm3, Null);
		mm4 = _mm_unpackhi_pi8(mm4, Null);
		mm5 = _mm_unpackhi_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		tmp = _mm_avg_pu8(tmp,mm6);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		*((DWORD*)&block[j][12]) = _m_to_int(tmp);


		src += w1;
	}
}
void  get_block_16xh_p11_sse2(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int j;
	byte *src;
	const int w1 = stride;
	const int w2 = stride + stride;
	const int w3 = w2 + w1;

	src = pSrc;

	__m128i mm0,mm1,mm2,mm3,mm4,mm5,mm6,tmp;
	const __m128i Null     = _mm_setzero_si128();
	const __m128i coeff_16 = _mm_load_si128((__m128i*)ncoeff_16);

	for (j = 0; j < height; j++)
	{
		mm0 = _mm_loadl_epi64(((__m128i*) (src-2)));
		mm1 = _mm_loadl_epi64(((__m128i*) (src-1)));
		mm2 = _mm_loadl_epi64(((__m128i*) (src)));
		mm3 = _mm_loadl_epi64(((__m128i*) (src+1)));
		mm4 = _mm_loadl_epi64(((__m128i*) (src+2)));
		mm5 = _mm_loadl_epi64(((__m128i*) (src+3)));

		mm0 = _mm_unpacklo_epi8(mm0, Null);
		mm1 = _mm_unpacklo_epi8(mm1, Null);
		mm2 = _mm_unpacklo_epi8(mm2, Null);
		mm3 = _mm_unpacklo_epi8(mm3, Null);
		mm4 = _mm_unpacklo_epi8(mm4, Null);
		mm5 = _mm_unpacklo_epi8(mm5, Null);

		tmp = _mm_sub_epi16(_mm_slli_epi16(_mm_add_epi16(mm2,mm3),2),_mm_add_epi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_slli_epi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_add_epi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_epi16(_mm_add_epi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packus_epi16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		_mm_storel_epi64((__m128i*)&block[j][0], tmp);

		mm0 = _mm_loadl_epi64(((__m128i*) (src+6)));
		mm1 = _mm_loadl_epi64(((__m128i*) (src+7)));
		mm2 = _mm_loadl_epi64(((__m128i*) (src+8)));
		mm3 = _mm_loadl_epi64(((__m128i*) (src+9)));
		mm4 = _mm_loadl_epi64(((__m128i*) (src+10)));
		mm5 = _mm_loadl_epi64(((__m128i*) (src+11)));

		mm0 = _mm_unpacklo_epi8(mm0, Null);
		mm1 = _mm_unpacklo_epi8(mm1, Null);
		mm2 = _mm_unpacklo_epi8(mm2, Null);
		mm3 = _mm_unpacklo_epi8(mm3, Null);
		mm4 = _mm_unpacklo_epi8(mm4, Null);
		mm5 = _mm_unpacklo_epi8(mm5, Null);

		tmp = _mm_sub_epi16(_mm_slli_epi16(_mm_add_epi16(mm2,mm3),2),_mm_add_epi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_slli_epi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_add_epi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_epi16(_mm_add_epi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packus_epi16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		_mm_storel_epi64((__m128i*)&block[j][8], tmp);

		src += w1;
	}

	src = pSrc;

	for (j = 0; j < height; j++)
	{
		mm0 = _mm_loadl_epi64(((__m128i*) (src-w2)));
		mm1 = _mm_loadl_epi64(((__m128i*) (src-w1)));
		mm2 = _mm_loadl_epi64(((__m128i*) (src)));
		mm3 = _mm_loadl_epi64(((__m128i*) (src+w1)));
		mm4 = _mm_loadl_epi64(((__m128i*) (src+w2)));
		mm5 = _mm_loadl_epi64(((__m128i*) (src+w3)));
		mm6 = _mm_loadl_epi64(((__m128i*)&block[j][0]));

		mm0 = _mm_unpacklo_epi8(mm0, Null);
		mm1 = _mm_unpacklo_epi8(mm1, Null);
		mm2 = _mm_unpacklo_epi8(mm2, Null);
		mm3 = _mm_unpacklo_epi8(mm3, Null);
		mm4 = _mm_unpacklo_epi8(mm4, Null);
		mm5 = _mm_unpacklo_epi8(mm5, Null);

		tmp = _mm_sub_epi16(_mm_slli_epi16(_mm_add_epi16(mm2,mm3),2),_mm_add_epi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_slli_epi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_add_epi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_epi16(_mm_add_epi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packus_epi16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		tmp = _mm_avg_epu8(tmp,mm6);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		_mm_storel_epi64((__m128i*)&block[j][0], tmp);

		mm0 = _mm_loadl_epi64(((__m128i*) (src-w2+8)));
		mm1 = _mm_loadl_epi64(((__m128i*) (src-w1+8)));
		mm2 = _mm_loadl_epi64(((__m128i*) (src+8)));
		mm3 = _mm_loadl_epi64(((__m128i*) (src+w1+8)));
		mm4 = _mm_loadl_epi64(((__m128i*) (src+w2+8)));
		mm5 = _mm_loadl_epi64(((__m128i*) (src+w3+8)));
		mm6 = _mm_loadl_epi64(((__m128i*)&block[j][8]));

		mm0 = _mm_unpacklo_epi8(mm0, Null);
		mm1 = _mm_unpacklo_epi8(mm1, Null);
		mm2 = _mm_unpacklo_epi8(mm2, Null);
		mm3 = _mm_unpacklo_epi8(mm3, Null);
		mm4 = _mm_unpacklo_epi8(mm4, Null);
		mm5 = _mm_unpacklo_epi8(mm5, Null);

		tmp = _mm_sub_epi16(_mm_slli_epi16(_mm_add_epi16(mm2,mm3),2),_mm_add_epi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_slli_epi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_add_epi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_epi16(_mm_add_epi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packus_epi16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		tmp = _mm_avg_epu8(tmp,mm6);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		_mm_storel_epi64((__m128i*)&block[j][8], tmp);

		src += w1;
	}
}
void  get_block_16xh_p12_sse(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	byte *src;
	short *tmp_src;
	short *luma_src;
	int i,j;
	DO_ALIGN(16, short luma_res[16][24]);
	DO_ALIGN(16, short tmp_res[16][24]);
	const int w1 = stride;
	const int w2 = stride + stride;
	const int w3 = w2 + w1;
	src = pSrc;

	__m64 pIn0, pIn1;
	__m64 mm0, mm1, mm2, mm3, mm4, mm5, tmp, tmp1;
	const __m64 Null     = _mm_setzero_si64();
	const __m64 coeff_16 = *((__m64*)ncoeff_16);
	const __m64 coeff_512 = *((__m64*)ncoeff_512);

	for (j = 0; j < height; j++)
	{
		for(i = 0; i < 20; i += 8)
		{
			mm0 = *((__m64*)(src-w2-2+i));
			mm1 = *((__m64*)(src-w1-2+i));
			mm2 = *((__m64*)(src-2+i));
			mm3 = *((__m64*)(src+w1-2+i));
			mm4 = *((__m64*)(src+w2-2+i));
			mm5 = *((__m64*)(src+w3-2+i));

			mm0 = _mm_unpacklo_pi8(mm0, Null);
			mm1 = _mm_unpacklo_pi8(mm1, Null);
			mm2 = _mm_unpacklo_pi8(mm2, Null);
			mm3 = _mm_unpacklo_pi8(mm3, Null);
			mm4 = _mm_unpacklo_pi8(mm4, Null);
			mm5 = _mm_unpacklo_pi8(mm5, Null);

			tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_adds_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
			tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
			tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5

			tmp1 = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
			*((__m64*)&tmp_res[j][i]) = tmp;
			*((__m64*)&luma_res[j][i]) = tmp1;
			
			mm0 = *((__m64*)(src-w2-2+i));
			mm1 = *((__m64*)(src-w1-2+i));
			mm2 = *((__m64*)(src-2+i));
			mm3 = *((__m64*)(src+w1-2+i));
			mm4 = *((__m64*)(src+w2-2+i));
			mm5 = *((__m64*)(src+w3-2+i));

			mm0 = _mm_unpackhi_pi8(mm0, Null);
			mm1 = _mm_unpackhi_pi8(mm1, Null);
			mm2 = _mm_unpackhi_pi8(mm2, Null);
			mm3 = _mm_unpackhi_pi8(mm3, Null);
			mm4 = _mm_unpackhi_pi8(mm4, Null);
			mm5 = _mm_unpackhi_pi8(mm5, Null);

			tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_adds_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
			tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
			tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5

			tmp1 = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
			*((__m64*)&tmp_res[j][i+4]) = tmp;
			*((__m64*)&luma_res[j][i+4]) = tmp1;

		}
		src += w1;
	}

	tmp_src = &tmp_res[0][0];
	luma_src = &luma_res[0][2];	
	for (j = 0; j < height; j++)
	{
	mm0 = *((__m64*)(tmp_src));
		mm1 = *((__m64*)(tmp_src+1));
		mm2 = *((__m64*)(tmp_src+2));
		mm3 = *((__m64*)(tmp_src+3));
		mm4 = *((__m64*)(tmp_src+4));
		mm5 = *((__m64*)(tmp_src+5));

		mm0 = _mm_unpacklo_pi16(Null, mm0);
		mm1 = _mm_unpacklo_pi16(Null, mm1);
		mm2 = _mm_unpacklo_pi16(Null, mm2);
		mm3 = _mm_unpacklo_pi16(Null, mm3);
		mm4 = _mm_unpacklo_pi16(Null, mm4);
		mm5 = _mm_unpacklo_pi16(Null, mm5);

		mm0 = _mm_srai_pi32(mm0,16);
		mm1 = _mm_srai_pi32(mm1,16);
		mm2 = _mm_srai_pi32(mm2,16);
		mm3 = _mm_srai_pi32(mm3,16);
		mm4 = _mm_srai_pi32(mm4,16);
		mm5 = _mm_srai_pi32(mm5,16);

		tmp = _mm_sub_pi32(_mm_slli_pi32(_mm_add_pi32(mm2,mm3),2),_mm_add_pi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi32(_mm_slli_pi32(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi32(_mm_add_pi32(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi32(_mm_add_pi32(tmp,coeff_512),10);//(result_block+512)>>10

		mm0 = mm4;
		mm1 = mm5;
		mm2 = *((__m64*)(tmp_src+6));
		mm3 = *((__m64*)(tmp_src+7));
		mm4 = *((__m64*)(tmp_src+8));
		mm5 = *((__m64*)(tmp_src+9));

		mm2 = _mm_unpacklo_pi16(Null, mm2);
		mm3 = _mm_unpacklo_pi16(Null, mm3);
		mm4 = _mm_unpacklo_pi16(Null, mm4);
		mm5 = _mm_unpacklo_pi16(Null, mm5);

		mm2 = _mm_srai_pi32(mm2,16);
		mm3 = _mm_srai_pi32(mm3,16);
		mm4 = _mm_srai_pi32(mm4,16);
		mm5 = _mm_srai_pi32(mm5,16);

		tmp1 = _mm_sub_pi32(_mm_slli_pi32(_mm_add_pi32(mm2,mm3),2),_mm_add_pi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_pi32(_mm_slli_pi32(tmp1,2),tmp1);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_pi32(_mm_add_pi32(mm0,mm5),tmp1); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp1 = _mm_srai_pi32(_mm_add_pi32(tmp1,coeff_512),10);//(result_block+512)>>10

		tmp = _mm_packs_pi32(tmp,tmp1);
		*((__m64*)&block[j][0]) = tmp;

		mm0 = *((__m64*)(tmp_src));
		mm1 = *((__m64*)(tmp_src+1));
		mm2 = *((__m64*)(tmp_src+2));
		mm3 = *((__m64*)(tmp_src+3));
		mm4 = *((__m64*)(tmp_src+4));
		mm5 = *((__m64*)(tmp_src+5));

		mm0 = _mm_unpackhi_pi16(Null, mm0);
		mm1 = _mm_unpackhi_pi16(Null, mm1);
		mm2 = _mm_unpackhi_pi16(Null, mm2);
		mm3 = _mm_unpackhi_pi16(Null, mm3);
		mm4 = _mm_unpackhi_pi16(Null, mm4);
		mm5 = _mm_unpackhi_pi16(Null, mm5);

		mm0 = _mm_srai_pi32(mm0,16);
		mm1 = _mm_srai_pi32(mm1,16);
		mm2 = _mm_srai_pi32(mm2,16);
		mm3 = _mm_srai_pi32(mm3,16);
		mm4 = _mm_srai_pi32(mm4,16);
		mm5 = _mm_srai_pi32(mm5,16);

		tmp = _mm_sub_pi32(_mm_slli_pi32(_mm_add_pi32(mm2,mm3),2),_mm_add_pi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi32(_mm_slli_pi32(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi32(_mm_add_pi32(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi32(_mm_add_pi32(tmp,coeff_512),10);//(result_block+512)>>10

		mm0 = mm4;
		mm1 = mm5;
		mm2 = *((__m64*)(tmp_src+6));
		mm3 = *((__m64*)(tmp_src+7));
		mm4 = *((__m64*)(tmp_src+8));
		mm5 = *((__m64*)(tmp_src+9));

		mm2 = _mm_unpackhi_pi16(Null, mm2);
		mm3 = _mm_unpackhi_pi16(Null, mm3);
		mm4 = _mm_unpackhi_pi16(Null, mm4);
		mm5 = _mm_unpackhi_pi16(Null, mm5);

		mm2 = _mm_srai_pi32(mm2,16);
		mm3 = _mm_srai_pi32(mm3,16);
		mm4 = _mm_srai_pi32(mm4,16);
		mm5 = _mm_srai_pi32(mm5,16);

		tmp1 = _mm_sub_pi32(_mm_slli_pi32(_mm_add_pi32(mm2,mm3),2),_mm_add_pi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_pi32(_mm_slli_pi32(tmp1,2),tmp1);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_pi32(_mm_add_pi32(mm0,mm5),tmp1); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp1 = _mm_srai_pi32(_mm_add_pi32(tmp1,coeff_512),10);//(result_block+512)>>10

		tmp = _mm_packs_pi32(tmp,tmp1);
		tmp1 = *((__m64*)&block[j][0]);
		mm4 = _mm_unpacklo_pi32(tmp1, tmp);
		mm5 = _mm_unpackhi_pi32(tmp1, tmp);

		pIn0 = *((__m64*) (luma_src));
		pIn1 = *((__m64*) (luma_src+4));
		tmp = _mm_packs_pu16(mm4,mm5);//__fast_iclip0_255((result_block+512)>>10)
		mm0 = _mm_packs_pu16(pIn0,pIn1);
		tmp = _mm_avg_pu8(tmp,mm0);
		*((__m64*)&block[j][0]) = tmp;

		mm0 = *((__m64*)(tmp_src+8));
		mm1 = *((__m64*)(tmp_src+9));
		mm2 = *((__m64*)(tmp_src+10));
		mm3 = *((__m64*)(tmp_src+11));
		mm4 = *((__m64*)(tmp_src+12));
		mm5 = *((__m64*)(tmp_src+13));

		mm0 = _mm_unpacklo_pi16(Null, mm0);
		mm1 = _mm_unpacklo_pi16(Null, mm1);
		mm2 = _mm_unpacklo_pi16(Null, mm2);
		mm3 = _mm_unpacklo_pi16(Null, mm3);
		mm4 = _mm_unpacklo_pi16(Null, mm4);
		mm5 = _mm_unpacklo_pi16(Null, mm5);

		mm0 = _mm_srai_pi32(mm0,16);
		mm1 = _mm_srai_pi32(mm1,16);
		mm2 = _mm_srai_pi32(mm2,16);
		mm3 = _mm_srai_pi32(mm3,16);
		mm4 = _mm_srai_pi32(mm4,16);
		mm5 = _mm_srai_pi32(mm5,16);

		tmp = _mm_sub_pi32(_mm_slli_pi32(_mm_add_pi32(mm2,mm3),2),_mm_add_pi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi32(_mm_slli_pi32(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi32(_mm_add_pi32(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi32(_mm_add_pi32(tmp,coeff_512),10);//(result_block+512)>>10

		mm0 = mm4;
		mm1 = mm5;
		mm2 = *((__m64*)(tmp_src+14));
		mm3 = *((__m64*)(tmp_src+15));
		mm4 = *((__m64*)(tmp_src+16));
		mm5 = *((__m64*)(tmp_src+17));

		mm2 = _mm_unpacklo_pi16(Null, mm2);
		mm3 = _mm_unpacklo_pi16(Null, mm3);
		mm4 = _mm_unpacklo_pi16(Null, mm4);
		mm5 = _mm_unpacklo_pi16(Null, mm5);

		mm2 = _mm_srai_pi32(mm2,16);
		mm3 = _mm_srai_pi32(mm3,16);
		mm4 = _mm_srai_pi32(mm4,16);
		mm5 = _mm_srai_pi32(mm5,16);

		tmp1 = _mm_sub_pi32(_mm_slli_pi32(_mm_add_pi32(mm2,mm3),2),_mm_add_pi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_pi32(_mm_slli_pi32(tmp1,2),tmp1);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_pi32(_mm_add_pi32(mm0,mm5),tmp1); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp1 = _mm_srai_pi32(_mm_add_pi32(tmp1,coeff_512),10);//(result_block+512)>>10

		tmp = _mm_packs_pi32(tmp,tmp1);
		*((__m64*)&block[j][8]) = tmp;

		mm0 = *((__m64*)(tmp_src+8));
		mm1 = *((__m64*)(tmp_src+9));
		mm2 = *((__m64*)(tmp_src+10));
		mm3 = *((__m64*)(tmp_src+11));
		mm4 = *((__m64*)(tmp_src+12));
		mm5 = *((__m64*)(tmp_src+13));

		mm0 = _mm_unpackhi_pi16(Null, mm0);
		mm1 = _mm_unpackhi_pi16(Null, mm1);
		mm2 = _mm_unpackhi_pi16(Null, mm2);
		mm3 = _mm_unpackhi_pi16(Null, mm3);
		mm4 = _mm_unpackhi_pi16(Null, mm4);
		mm5 = _mm_unpackhi_pi16(Null, mm5);

		mm0 = _mm_srai_pi32(mm0,16);
		mm1 = _mm_srai_pi32(mm1,16);
		mm2 = _mm_srai_pi32(mm2,16);
		mm3 = _mm_srai_pi32(mm3,16);
		mm4 = _mm_srai_pi32(mm4,16);
		mm5 = _mm_srai_pi32(mm5,16);

		tmp = _mm_sub_pi32(_mm_slli_pi32(_mm_add_pi32(mm2,mm3),2),_mm_add_pi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi32(_mm_slli_pi32(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi32(_mm_add_pi32(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi32(_mm_add_pi32(tmp,coeff_512),10);//(result_block+512)>>10

		mm0 = mm4;
		mm1 = mm5;
		mm2 = *((__m64*)(tmp_src+14));
		mm3 = *((__m64*)(tmp_src+15));
		mm4 = *((__m64*)(tmp_src+16));
		mm5 = *((__m64*)(tmp_src+17));

		mm2 = _mm_unpackhi_pi16(Null, mm2);
		mm3 = _mm_unpackhi_pi16(Null, mm3);
		mm4 = _mm_unpackhi_pi16(Null, mm4);
		mm5 = _mm_unpackhi_pi16(Null, mm5);

		mm2 = _mm_srai_pi32(mm2,16);
		mm3 = _mm_srai_pi32(mm3,16);
		mm4 = _mm_srai_pi32(mm4,16);
		mm5 = _mm_srai_pi32(mm5,16);

		tmp1 = _mm_sub_pi32(_mm_slli_pi32(_mm_add_pi32(mm2,mm3),2),_mm_add_pi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_pi32(_mm_slli_pi32(tmp1,2),tmp1);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_pi32(_mm_add_pi32(mm0,mm5),tmp1); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp1 = _mm_srai_pi32(_mm_add_pi32(tmp1,coeff_512),10);//(result_block+512)>>10

		tmp = _mm_packs_pi32(tmp,tmp1);
		tmp1 = *((__m64*)&block[j][8]);
		mm4 = _mm_unpacklo_pi32(tmp1, tmp);
		mm5 = _mm_unpackhi_pi32(tmp1, tmp);


		pIn0 = *((__m64*) (luma_src+8));
		pIn1 = *((__m64*) (luma_src+12));
		tmp = _mm_packs_pu16(mm4,mm5);//__fast_iclip0_255((result_block+512)>>10)
		mm0 = _mm_packs_pu16(pIn0,pIn1);
		tmp = _mm_avg_pu8(tmp,mm0);
		*((__m64*)&block[j][8]) = tmp;

		tmp_src += 24;
		luma_src += 24;
	}
}

void  get_block_16xh_p12_sse2(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	byte *src;
	short *tmp_src;
	short *luma_src;
	int i,j;
	DO_ALIGN(16, short luma_res[16][24]);
	DO_ALIGN(16, short tmp_res[16][24]);
	const int w1 = stride;
	const int w2 = stride + stride;
	const int w3 = w2 + w1;
	src = pSrc;

	__m64 pIn0, pIn1;
	__m128i mm0,mm1,mm2,mm3,mm4,mm5,tmp,tmp1;
	const __m128i Null      = _mm_setzero_si128();
	const __m128i coeff_16  = _mm_load_si128((__m128i*)ncoeff_16);
	const __m128i coeff_512 = _mm_load_si128((__m128i*)ncoeff_512);

	for (j = 0; j < height; j++)
	{
		for(i = 0; i < 20; i += 8)
		{
			mm0 = _mm_loadl_epi64(((__m128i*) (src-w2-2+i)));
			mm1 = _mm_loadl_epi64(((__m128i*) (src-w1-2+i)));
			mm2 = _mm_loadl_epi64(((__m128i*) (src-2+i)));
			mm3 = _mm_loadl_epi64(((__m128i*) (src+w1-2+i)));
			mm4 = _mm_loadl_epi64(((__m128i*) (src+w2-2+i)));
			mm5 = _mm_loadl_epi64(((__m128i*) (src+w3-2+i)));

			mm0 = _mm_unpacklo_epi8(mm0, Null);
			mm1 = _mm_unpacklo_epi8(mm1, Null);
			mm2 = _mm_unpacklo_epi8(mm2, Null);
			mm3 = _mm_unpacklo_epi8(mm3, Null);
			mm4 = _mm_unpacklo_epi8(mm4, Null);
			mm5 = _mm_unpacklo_epi8(mm5, Null);

			tmp = _mm_sub_epi16(_mm_slli_epi16(_mm_add_epi16(mm2,mm3),2),_mm_adds_epi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
			tmp = _mm_add_epi16(_mm_slli_epi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
			tmp = _mm_add_epi16(_mm_add_epi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5

			_mm_store_si128(((__m128i*)&tmp_res[j][i]),tmp);

			tmp = _mm_srai_epi16(_mm_add_epi16(tmp,coeff_16),5);//(result_block+16)>>5

			_mm_store_si128(((__m128i*)&luma_res[j][i]),tmp);

		}
		src += w1;
	}

	tmp_src = &tmp_res[0][0];
	luma_src = &luma_res[0][2];	
	for (j = 0; j < height; j++)
	{
		mm0 = _mm_loadl_epi64(((__m128i*) (tmp_src)));
		mm1 = _mm_loadl_epi64(((__m128i*) (tmp_src+1)));
		mm2 = _mm_loadl_epi64(((__m128i*) (tmp_src+2)));
		mm3 = _mm_loadl_epi64(((__m128i*) (tmp_src+3)));
		mm4 = _mm_loadl_epi64(((__m128i*) (tmp_src+4)));
		mm5 = _mm_loadl_epi64(((__m128i*) (tmp_src+5)));

		mm0 = _mm_unpacklo_epi16(Null, mm0);
		mm1 = _mm_unpacklo_epi16(Null, mm1);
		mm2 = _mm_unpacklo_epi16(Null, mm2);
		mm3 = _mm_unpacklo_epi16(Null, mm3);
		mm4 = _mm_unpacklo_epi16(Null, mm4);
		mm5 = _mm_unpacklo_epi16(Null, mm5);

		mm0 = _mm_srai_epi32(mm0,16);
		mm1 = _mm_srai_epi32(mm1,16);
		mm2 = _mm_srai_epi32(mm2,16);
		mm3 = _mm_srai_epi32(mm3,16);
		mm4 = _mm_srai_epi32(mm4,16);
		mm5 = _mm_srai_epi32(mm5,16);

		tmp = _mm_sub_epi32(_mm_slli_epi32(_mm_add_epi32(mm2,mm3),2),_mm_add_epi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi32(_mm_slli_epi32(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi32(_mm_add_epi32(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_epi32(_mm_add_epi32(tmp,coeff_512),10);//(result_block+512)>>10

		mm0 = mm4;
		mm1 = mm5;
		mm2 = _mm_loadl_epi64(((__m128i*) (tmp_src+6)));
		mm3 = _mm_loadl_epi64(((__m128i*) (tmp_src+7)));
		mm4 = _mm_loadl_epi64(((__m128i*) (tmp_src+8)));
		mm5 = _mm_loadl_epi64(((__m128i*) (tmp_src+9)));

		mm2 = _mm_unpacklo_epi16(Null, mm2);
		mm3 = _mm_unpacklo_epi16(Null, mm3);
		mm4 = _mm_unpacklo_epi16(Null, mm4);
		mm5 = _mm_unpacklo_epi16(Null, mm5);

		mm2 = _mm_srai_epi32(mm2,16);
		mm3 = _mm_srai_epi32(mm3,16);
		mm4 = _mm_srai_epi32(mm4,16);
		mm5 = _mm_srai_epi32(mm5,16);

		tmp1 = _mm_sub_epi32(_mm_slli_epi32(_mm_add_epi32(mm2,mm3),2),_mm_add_epi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_epi32(_mm_slli_epi32(tmp1,2),tmp1);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_epi32(_mm_add_epi32(mm0,mm5),tmp1); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp1 = _mm_srai_epi32(_mm_add_epi32(tmp1,coeff_512),10);//(result_block+512)>>10

		tmp = _mm_packs_epi32(tmp,tmp1);

		pIn0 = *((__m64*) (luma_src));
		pIn1 = *((__m64*) (luma_src+4));
		mm0 = _mm_setr_epi64(pIn0,pIn1);
		tmp = _mm_packus_epi16(tmp,tmp);//__fast_iclip0_255((result_block+512)>>10)
		mm0 = _mm_packus_epi16(mm0,mm0);
		tmp = _mm_avg_epu8(tmp,mm0);
		_mm_storel_epi64((__m128i*)&block[j][0], tmp);


		mm0 = _mm_loadl_epi64(((__m128i*) (tmp_src+8)));
		mm1 = _mm_loadl_epi64(((__m128i*) (tmp_src+9)));
		mm2 = _mm_loadl_epi64(((__m128i*) (tmp_src+10)));
		mm3 = _mm_loadl_epi64(((__m128i*) (tmp_src+11)));
		mm4 = _mm_loadl_epi64(((__m128i*) (tmp_src+12)));
		mm5 = _mm_loadl_epi64(((__m128i*) (tmp_src+13)));

		mm0 = _mm_unpacklo_epi16(Null, mm0);
		mm1 = _mm_unpacklo_epi16(Null, mm1);
		mm2 = _mm_unpacklo_epi16(Null, mm2);
		mm3 = _mm_unpacklo_epi16(Null, mm3);
		mm4 = _mm_unpacklo_epi16(Null, mm4);
		mm5 = _mm_unpacklo_epi16(Null, mm5);

		mm0 = _mm_srai_epi32(mm0,16);
		mm1 = _mm_srai_epi32(mm1,16);
		mm2 = _mm_srai_epi32(mm2,16);
		mm3 = _mm_srai_epi32(mm3,16);
		mm4 = _mm_srai_epi32(mm4,16);
		mm5 = _mm_srai_epi32(mm5,16);

		tmp = _mm_sub_epi32(_mm_slli_epi32(_mm_add_epi32(mm2,mm3),2),_mm_add_epi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi32(_mm_slli_epi32(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi32(_mm_add_epi32(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_epi32(_mm_add_epi32(tmp,coeff_512),10);//(result_block+512)>>10

		mm0 = mm4;
		mm1 = mm5;
		mm2 = _mm_loadl_epi64(((__m128i*) (tmp_src+14)));
		mm3 = _mm_loadl_epi64(((__m128i*) (tmp_src+15)));
		mm4 = _mm_loadl_epi64(((__m128i*) (tmp_src+16)));
		mm5 = _mm_loadl_epi64(((__m128i*) (tmp_src+17)));

		mm2 = _mm_unpacklo_epi16(Null, mm2);
		mm3 = _mm_unpacklo_epi16(Null, mm3);
		mm4 = _mm_unpacklo_epi16(Null, mm4);
		mm5 = _mm_unpacklo_epi16(Null, mm5);

		mm2 = _mm_srai_epi32(mm2,16);
		mm3 = _mm_srai_epi32(mm3,16);
		mm4 = _mm_srai_epi32(mm4,16);
		mm5 = _mm_srai_epi32(mm5,16);

		tmp1 = _mm_sub_epi32(_mm_slli_epi32(_mm_add_epi32(mm2,mm3),2),_mm_add_epi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_epi32(_mm_slli_epi32(tmp1,2),tmp1);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_epi32(_mm_add_epi32(mm0,mm5),tmp1); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp1 = _mm_srai_epi32(_mm_add_epi32(tmp1,coeff_512),10);//(result_block+512)>>10

		tmp = _mm_packs_epi32(tmp,tmp1);

		pIn0 = *((__m64*) (luma_src+8));
		pIn1 = *((__m64*) (luma_src+12));
		mm0 = _mm_setr_epi64(pIn0,pIn1);
		tmp = _mm_packus_epi16(tmp,tmp);//__fast_iclip0_255((result_block+512)>>10)
		mm0 = _mm_packus_epi16(mm0,mm0);
		tmp = _mm_avg_epu8(tmp,mm0);
		_mm_storel_epi64((__m128i*)&block[j][8], tmp);

		tmp_src += 24;
		luma_src += 24;
	}
}
void  get_block_16xh_p13_sse(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int j;
	byte *src;
	const int w1 = stride;
	const int w2 = stride + stride;
	const int w3 = w2 + w1;

	src = pSrc+w1;

	__m64 mm0, mm1, mm2, mm3, mm4, mm5, mm6, tmp;
	const __m64 Null     = _mm_setzero_si64();
	const __m64 coeff_16 = *((__m64*)ncoeff_16);

	for (j = 0; j < height; j++)
	{
		mm0 = *((__m64*)(src-2));
		mm1 = *((__m64*)(src-1));
		mm2 = *((__m64*)(src));
		mm3 = *((__m64*)(src+1));
		mm4 = *((__m64*)(src+2));
		mm5 = *((__m64*)(src+3));

		mm0 = _mm_unpacklo_pi8(mm0, Null);
		mm1 = _mm_unpacklo_pi8(mm1, Null);
		mm2 = _mm_unpacklo_pi8(mm2, Null);
		mm3 = _mm_unpacklo_pi8(mm3, Null);
		mm4 = _mm_unpacklo_pi8(mm4, Null);
		mm5 = _mm_unpacklo_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		*((DWORD*)&block[j][0]) = _m_to_int(tmp);
		
		mm0 = *((__m64*)(src-2));
		mm1 = *((__m64*)(src-1));
		mm2 = *((__m64*)(src));
		mm3 = *((__m64*)(src+1));
		mm4 = *((__m64*)(src+2));
		mm5 = *((__m64*)(src+3));

		mm0 = _mm_unpackhi_pi8(mm0, Null);
		mm1 = _mm_unpackhi_pi8(mm1, Null);
		mm2 = _mm_unpackhi_pi8(mm2, Null);
		mm3 = _mm_unpackhi_pi8(mm3, Null);
		mm4 = _mm_unpackhi_pi8(mm4, Null);
		mm5 = _mm_unpackhi_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		*((DWORD*)&block[j][4]) = _m_to_int(tmp);

		mm0 = *((__m64*)(src+6));
		mm1 = *((__m64*)(src+7));
		mm2 = *((__m64*)(src+8));
		mm3 = *((__m64*)(src+9));
		mm4 = *((__m64*)(src+10));
		mm5 = *((__m64*)(src+11));

		mm0 = _mm_unpacklo_pi8(mm0, Null);
		mm1 = _mm_unpacklo_pi8(mm1, Null);
		mm2 = _mm_unpacklo_pi8(mm2, Null);
		mm3 = _mm_unpacklo_pi8(mm3, Null);
		mm4 = _mm_unpacklo_pi8(mm4, Null);
		mm5 = _mm_unpacklo_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		*((DWORD*)&block[j][8]) = _m_to_int(tmp);

		mm0 = *((__m64*)(src+6));
		mm1 = *((__m64*)(src+7));
		mm2 = *((__m64*)(src+8));
		mm3 = *((__m64*)(src+9));
		mm4 = *((__m64*)(src+10));
		mm5 = *((__m64*)(src+11));

		mm0 = _mm_unpackhi_pi8(mm0, Null);
		mm1 = _mm_unpackhi_pi8(mm1, Null);
		mm2 = _mm_unpackhi_pi8(mm2, Null);
		mm3 = _mm_unpackhi_pi8(mm3, Null);
		mm4 = _mm_unpackhi_pi8(mm4, Null);
		mm5 = _mm_unpackhi_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		*((DWORD*)&block[j][12]) = _m_to_int(tmp);

		src += w1;
	}

	src = pSrc;

	for (j = 0; j < height; j++)
	{
		mm0 = *((__m64*)(src-w2));
		mm1 = *((__m64*)(src-w1));
		mm2 = *((__m64*)(src));
		mm3 = *((__m64*)(src+w1));
		mm4 = *((__m64*)(src+w2));
		mm5 = *((__m64*)(src+w3));
		mm6 = *((__m64*)&block[j][0]);

		mm0 = _mm_unpacklo_pi8(mm0, Null);
		mm1 = _mm_unpacklo_pi8(mm1, Null);
		mm2 = _mm_unpacklo_pi8(mm2, Null);
		mm3 = _mm_unpacklo_pi8(mm3, Null);
		mm4 = _mm_unpacklo_pi8(mm4, Null);
		mm5 = _mm_unpacklo_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		tmp = _mm_avg_pu8(tmp,mm6);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		*((DWORD*)&block[j][0]) = _m_to_int(tmp);

		mm0 = *((__m64*)(src-w2));
		mm1 = *((__m64*)(src-w1));
		mm2 = *((__m64*)(src));
		mm3 = *((__m64*)(src+w1));
		mm4 = *((__m64*)(src+w2));
		mm5 = *((__m64*)(src+w3));
		mm6 = *((__m64*)&block[j][4]);

		mm0 = _mm_unpackhi_pi8(mm0, Null);
		mm1 = _mm_unpackhi_pi8(mm1, Null);
		mm2 = _mm_unpackhi_pi8(mm2, Null);
		mm3 = _mm_unpackhi_pi8(mm3, Null);
		mm4 = _mm_unpackhi_pi8(mm4, Null);
		mm5 = _mm_unpackhi_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		tmp = _mm_avg_pu8(tmp,mm6);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		*((DWORD*)&block[j][4]) = _m_to_int(tmp);

		mm0 = *((__m64*)(src-w2+8));
		mm1 = *((__m64*)(src-w1+8));
		mm2 = *((__m64*)(src+8));
		mm3 = *((__m64*)(src+w1+8));
		mm4 = *((__m64*)(src+w2+8));
		mm5 = *((__m64*)(src+w3+8));
		mm6 = *((__m64*)&block[j][8]);

		mm0 = _mm_unpacklo_pi8(mm0, Null);
		mm1 = _mm_unpacklo_pi8(mm1, Null);
		mm2 = _mm_unpacklo_pi8(mm2, Null);
		mm3 = _mm_unpacklo_pi8(mm3, Null);
		mm4 = _mm_unpacklo_pi8(mm4, Null);
		mm5 = _mm_unpacklo_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		tmp = _mm_avg_pu8(tmp,mm6);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		*((DWORD*)&block[j][8]) = _m_to_int(tmp);

		mm0 = *((__m64*)(src-w2+8));
		mm1 = *((__m64*)(src-w1+8));
		mm2 = *((__m64*)(src+8));
		mm3 = *((__m64*)(src+w1+8));
		mm4 = *((__m64*)(src+w2+8));
		mm5 = *((__m64*)(src+w3+8));
		mm6 = *((__m64*)&block[j][12]);

		mm0 = _mm_unpackhi_pi8(mm0, Null);
		mm1 = _mm_unpackhi_pi8(mm1, Null);
		mm2 = _mm_unpackhi_pi8(mm2, Null);
		mm3 = _mm_unpackhi_pi8(mm3, Null);
		mm4 = _mm_unpackhi_pi8(mm4, Null);
		mm5 = _mm_unpackhi_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		tmp = _mm_avg_pu8(tmp,mm6);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		*((DWORD*)&block[j][12]) = _m_to_int(tmp);

		src += w1;
	}
}
void  get_block_16xh_p13_sse2(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int j;
	byte *src;
	const int w1 = stride;
	const int w2 = stride + stride;
	const int w3 = w2 + w1;

	src = pSrc+w1;

	__m128i mm0,mm1,mm2,mm3,mm4,mm5,mm6,tmp;
	const __m128i Null     = _mm_setzero_si128();
	const __m128i coeff_16 = _mm_load_si128((__m128i*)ncoeff_16);

	for (j = 0; j < height; j++)
	{
		mm0 = _mm_loadl_epi64(((__m128i*) (src-2)));
		mm1 = _mm_loadl_epi64(((__m128i*) (src-1)));
		mm2 = _mm_loadl_epi64(((__m128i*) (src)));
		mm3 = _mm_loadl_epi64(((__m128i*) (src+1)));
		mm4 = _mm_loadl_epi64(((__m128i*) (src+2)));
		mm5 = _mm_loadl_epi64(((__m128i*) (src+3)));

		mm0 = _mm_unpacklo_epi8(mm0, Null);
		mm1 = _mm_unpacklo_epi8(mm1, Null);
		mm2 = _mm_unpacklo_epi8(mm2, Null);
		mm3 = _mm_unpacklo_epi8(mm3, Null);
		mm4 = _mm_unpacklo_epi8(mm4, Null);
		mm5 = _mm_unpacklo_epi8(mm5, Null);

		tmp = _mm_sub_epi16(_mm_slli_epi16(_mm_add_epi16(mm2,mm3),2),_mm_add_epi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_slli_epi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_add_epi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_epi16(_mm_add_epi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packus_epi16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		_mm_storel_epi64((__m128i*)&block[j][0], tmp);


		mm0 = _mm_loadl_epi64(((__m128i*) (src+6)));
		mm1 = _mm_loadl_epi64(((__m128i*) (src+7)));
		mm2 = _mm_loadl_epi64(((__m128i*) (src+8)));
		mm3 = _mm_loadl_epi64(((__m128i*) (src+9)));
		mm4 = _mm_loadl_epi64(((__m128i*) (src+10)));
		mm5 = _mm_loadl_epi64(((__m128i*) (src+11)));

		mm0 = _mm_unpacklo_epi8(mm0, Null);
		mm1 = _mm_unpacklo_epi8(mm1, Null);
		mm2 = _mm_unpacklo_epi8(mm2, Null);
		mm3 = _mm_unpacklo_epi8(mm3, Null);
		mm4 = _mm_unpacklo_epi8(mm4, Null);
		mm5 = _mm_unpacklo_epi8(mm5, Null);

		tmp = _mm_sub_epi16(_mm_slli_epi16(_mm_add_epi16(mm2,mm3),2),_mm_add_epi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_slli_epi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_add_epi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_epi16(_mm_add_epi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packus_epi16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		_mm_storel_epi64((__m128i*)&block[j][8], tmp);

		src += w1;
	}

	src = pSrc;

	for (j = 0; j < height; j++)
	{
		mm0 = _mm_loadl_epi64(((__m128i*) (src-w2)));
		mm1 = _mm_loadl_epi64(((__m128i*) (src-w1)));
		mm2 = _mm_loadl_epi64(((__m128i*) (src)));
		mm3 = _mm_loadl_epi64(((__m128i*) (src+w1)));
		mm4 = _mm_loadl_epi64(((__m128i*) (src+w2)));
		mm5 = _mm_loadl_epi64(((__m128i*) (src+w3)));
		mm6 = _mm_loadl_epi64(((__m128i*)&block[j][0]));

		mm0 = _mm_unpacklo_epi8(mm0, Null);
		mm1 = _mm_unpacklo_epi8(mm1, Null);
		mm2 = _mm_unpacklo_epi8(mm2, Null);
		mm3 = _mm_unpacklo_epi8(mm3, Null);
		mm4 = _mm_unpacklo_epi8(mm4, Null);
		mm5 = _mm_unpacklo_epi8(mm5, Null);

		tmp = _mm_sub_epi16(_mm_slli_epi16(_mm_add_epi16(mm2,mm3),2),_mm_add_epi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_slli_epi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_add_epi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_epi16(_mm_add_epi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packus_epi16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		tmp = _mm_avg_epu8(tmp,mm6);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		_mm_storel_epi64((__m128i*)&block[j][0], tmp);

		mm0 = _mm_loadl_epi64(((__m128i*) (src-w2+8)));
		mm1 = _mm_loadl_epi64(((__m128i*) (src-w1+8)));
		mm2 = _mm_loadl_epi64(((__m128i*) (src+8)));
		mm3 = _mm_loadl_epi64(((__m128i*) (src+w1+8)));
		mm4 = _mm_loadl_epi64(((__m128i*) (src+w2+8)));
		mm5 = _mm_loadl_epi64(((__m128i*) (src+w3+8)));
		mm6 = _mm_loadl_epi64(((__m128i*)&block[j][8]));

		mm0 = _mm_unpacklo_epi8(mm0, Null);
		mm1 = _mm_unpacklo_epi8(mm1, Null);
		mm2 = _mm_unpacklo_epi8(mm2, Null);
		mm3 = _mm_unpacklo_epi8(mm3, Null);
		mm4 = _mm_unpacklo_epi8(mm4, Null);
		mm5 = _mm_unpacklo_epi8(mm5, Null);

		tmp = _mm_sub_epi16(_mm_slli_epi16(_mm_add_epi16(mm2,mm3),2),_mm_add_epi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_slli_epi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_add_epi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_epi16(_mm_add_epi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packus_epi16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		tmp = _mm_avg_epu8(tmp,mm6);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		_mm_storel_epi64((__m128i*)&block[j][8], tmp);

		src += w1;
	}
}

void  get_block_16xh_p20_sse(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int j;
	byte *src;
	const int w1 = stride;

	src = pSrc;

	__m64 mm0, mm1, mm2, mm3, mm4, mm5, tmp;
	const __m64 Null     = _mm_setzero_si64();
	const __m64 coeff_16 = *((__m64*)ncoeff_16);

	for (j = 0; j < height; j++)
	{
		mm0 = *((__m64*)(src-2));
		mm1 = *((__m64*)(src-1));
		mm2 = *((__m64*)(src));
		mm3 = *((__m64*)(src+1));
		mm4 = *((__m64*)(src+2));
		mm5 = *((__m64*)(src+3));

		mm0 = _mm_unpacklo_pi8(mm0, Null);
		mm1 = _mm_unpacklo_pi8(mm1, Null);
		mm2 = _mm_unpacklo_pi8(mm2, Null);
		mm3 = _mm_unpacklo_pi8(mm3, Null);
		mm4 = _mm_unpacklo_pi8(mm4, Null);
		mm5 = _mm_unpacklo_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		*((DWORD*)&block[j][0]) = _m_to_int(tmp);
		
		mm0 = *((__m64*)(src-2));
		mm1 = *((__m64*)(src-1));
		mm2 = *((__m64*)(src));
		mm3 = *((__m64*)(src+1));
		mm4 = *((__m64*)(src+2));
		mm5 = *((__m64*)(src+3));

		mm0 = _mm_unpackhi_pi8(mm0, Null);
		mm1 = _mm_unpackhi_pi8(mm1, Null);
		mm2 = _mm_unpackhi_pi8(mm2, Null);
		mm3 = _mm_unpackhi_pi8(mm3, Null);
		mm4 = _mm_unpackhi_pi8(mm4, Null);
		mm5 = _mm_unpackhi_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		*((DWORD*)&block[j][4]) = _m_to_int(tmp);

		
		mm0 = *((__m64*)(src+6));
		mm1 = *((__m64*)(src+7));
		mm2 = *((__m64*)(src+8));
		mm3 = *((__m64*)(src+9));
		mm4 = *((__m64*)(src+10));
		mm5 = *((__m64*)(src+11));

		mm0 = _mm_unpacklo_pi8(mm0, Null);
		mm1 = _mm_unpacklo_pi8(mm1, Null);
		mm2 = _mm_unpacklo_pi8(mm2, Null);
		mm3 = _mm_unpacklo_pi8(mm3, Null);
		mm4 = _mm_unpacklo_pi8(mm4, Null);
		mm5 = _mm_unpacklo_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		*((DWORD*)&block[j][8]) = _m_to_int(tmp);

		mm0 = *((__m64*)(src+6));
		mm1 = *((__m64*)(src+7));
		mm2 = *((__m64*)(src+8));
		mm3 = *((__m64*)(src+9));
		mm4 = *((__m64*)(src+10));
		mm5 = *((__m64*)(src+11));

		mm0 = _mm_unpackhi_pi8(mm0, Null);
		mm1 = _mm_unpackhi_pi8(mm1, Null);
		mm2 = _mm_unpackhi_pi8(mm2, Null);
		mm3 = _mm_unpackhi_pi8(mm3, Null);
		mm4 = _mm_unpackhi_pi8(mm4, Null);
		mm5 = _mm_unpackhi_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		*((DWORD*)&block[j][12]) = _m_to_int(tmp);

		src += w1;
	}
}
void  get_block_16xh_p20_sse2(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int j;
	byte *src;
	const int w1 = stride;

	src = pSrc;

	__m128i mm0,mm1,mm2,mm3,mm4,mm5,tmp;
	const __m128i Null     = _mm_setzero_si128();
	const __m128i coeff_16 = _mm_load_si128((__m128i*)ncoeff_16);

	for (j = 0; j < height; j++)
	{
		mm0 = _mm_loadl_epi64(((__m128i*) (src-2)));
		mm1 = _mm_loadl_epi64(((__m128i*) (src-1)));
		mm2 = _mm_loadl_epi64(((__m128i*) (src)));
		mm3 = _mm_loadl_epi64(((__m128i*) (src+1)));
		mm4 = _mm_loadl_epi64(((__m128i*) (src+2)));
		mm5 = _mm_loadl_epi64(((__m128i*) (src+3)));

		mm0 = _mm_unpacklo_epi8(mm0, Null);
		mm1 = _mm_unpacklo_epi8(mm1, Null);
		mm2 = _mm_unpacklo_epi8(mm2, Null);
		mm3 = _mm_unpacklo_epi8(mm3, Null);
		mm4 = _mm_unpacklo_epi8(mm4, Null);
		mm5 = _mm_unpacklo_epi8(mm5, Null);

		tmp = _mm_sub_epi16(_mm_slli_epi16(_mm_add_epi16(mm2,mm3),2),_mm_add_epi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_slli_epi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_add_epi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_epi16(_mm_add_epi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packus_epi16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		_mm_storel_epi64((__m128i*)&block[j][0], tmp);

		mm0 = _mm_loadl_epi64(((__m128i*) (src+6)));
		mm1 = _mm_loadl_epi64(((__m128i*) (src+7)));
		mm2 = _mm_loadl_epi64(((__m128i*) (src+8)));
		mm3 = _mm_loadl_epi64(((__m128i*) (src+9)));
		mm4 = _mm_loadl_epi64(((__m128i*) (src+10)));
		mm5 = _mm_loadl_epi64(((__m128i*) (src+11)));

		mm0 = _mm_unpacklo_epi8(mm0, Null);
		mm1 = _mm_unpacklo_epi8(mm1, Null);
		mm2 = _mm_unpacklo_epi8(mm2, Null);
		mm3 = _mm_unpacklo_epi8(mm3, Null);
		mm4 = _mm_unpacklo_epi8(mm4, Null);
		mm5 = _mm_unpacklo_epi8(mm5, Null);

		tmp = _mm_sub_epi16(_mm_slli_epi16(_mm_add_epi16(mm2,mm3),2),_mm_add_epi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_slli_epi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_add_epi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_epi16(_mm_add_epi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packus_epi16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		_mm_storel_epi64((__m128i*)&block[j][8], tmp);

		src += w1;
	}
}
void  get_block_16xh_p21_sse(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	byte *src;
	int j;
	DO_ALIGN(16, short luma_res[21][16]);
	DO_ALIGN(16, int tmp_res[21][16]);
	const int w1 = stride;

	src = pSrc-(2*w1);

	__m64 mm0, mm1, mm2, mm3, mm4, mm5, tmp, tmp1;
	const __m64 Null     = _mm_setzero_si64();
	const __m64 coeff_16 = *((__m64*)ncoeff_16);
	const __m64 coeff_512 = *((__m64*)ncoeff_512);

	for (j = 0; j < height+5; j++)
	{
		mm0 = *((__m64*)(src-2));
		mm1 = *((__m64*)(src-1));
		mm2 = *((__m64*)(src));
		mm3 = *((__m64*)(src+1));
		mm4 = *((__m64*)(src+2));
		mm5 = *((__m64*)(src+3));

		mm0 = _mm_unpacklo_pi8(mm0, Null);
		mm1 = _mm_unpacklo_pi8(mm1, Null);
		mm2 = _mm_unpacklo_pi8(mm2, Null);
		mm3 = _mm_unpacklo_pi8(mm3, Null);
		mm4 = _mm_unpacklo_pi8(mm4, Null);
		mm5 = _mm_unpacklo_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		
		mm0 = _mm_unpacklo_pi16(Null,tmp);
		mm0 = _mm_srai_pi32(mm0,16);		
		*((__m64*)&tmp_res[j][0]) = mm0;
		mm0 = _mm_unpackhi_pi16(Null,tmp);
		mm0 = _mm_srai_pi32(mm0,16);		
		*((__m64*)&tmp_res[j][2]) = mm0;
		
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		*((__m64*)&luma_res[j][0]) = tmp;
		
		
		mm0 = *((__m64*)(src-2));
		mm1 = *((__m64*)(src-1));
		mm2 = *((__m64*)(src));
		mm3 = *((__m64*)(src+1));
		mm4 = *((__m64*)(src+2));
		mm5 = *((__m64*)(src+3));

		mm0 = _mm_unpackhi_pi8(mm0, Null);
		mm1 = _mm_unpackhi_pi8(mm1, Null);
		mm2 = _mm_unpackhi_pi8(mm2, Null);
		mm3 = _mm_unpackhi_pi8(mm3, Null);
		mm4 = _mm_unpackhi_pi8(mm4, Null);
		mm5 = _mm_unpackhi_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		
		mm1 = _mm_unpacklo_pi16(Null,tmp);
		mm1 = _mm_srai_pi32(mm1,16);
		*((__m64*)&tmp_res[j][4]) = mm1;
		mm1 = _mm_unpackhi_pi16(Null,tmp);
		mm1 = _mm_srai_pi32(mm1,16);
		*((__m64*)&tmp_res[j][6]) = mm1;
		
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		*((__m64*)&luma_res[j][4]) = tmp;


		mm0 = *((__m64*)(src+6));
		mm1 = *((__m64*)(src+7));
		mm2 = *((__m64*)(src+8));
		mm3 = *((__m64*)(src+9));
		mm4 = *((__m64*)(src+10));
		mm5 = *((__m64*)(src+11));
		
		mm0 = _mm_unpacklo_pi8(mm0, Null);
		mm1 = _mm_unpacklo_pi8(mm1, Null);
		mm2 = _mm_unpacklo_pi8(mm2, Null);
		mm3 = _mm_unpacklo_pi8(mm3, Null);
		mm4 = _mm_unpacklo_pi8(mm4, Null);
		mm5 = _mm_unpacklo_pi8(mm5, Null);
		
		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_adds_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5

		mm0 = _mm_unpacklo_pi16(Null,tmp);
		mm0 = _mm_srai_pi32(mm0,16);		
		*((__m64*)&tmp_res[j][8]) = mm0;
		mm0 = _mm_unpackhi_pi16(Null,tmp);
		mm0 = _mm_srai_pi32(mm0,16);		
		*((__m64*)&tmp_res[j][10]) = mm0;
		
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		*((__m64*)&luma_res[j][8]) = tmp;

			mm0 = *((__m64*)(src+6));
		mm1 = *((__m64*)(src+7));
		mm2 = *((__m64*)(src+8));
		mm3 = *((__m64*)(src+9));
		mm4 = *((__m64*)(src+10));
		mm5 = *((__m64*)(src+11));
		
		mm0 = _mm_unpackhi_pi8(mm0, Null);
		mm1 = _mm_unpackhi_pi8(mm1, Null);
		mm2 = _mm_unpackhi_pi8(mm2, Null);
		mm3 = _mm_unpackhi_pi8(mm3, Null);
		mm4 = _mm_unpackhi_pi8(mm4, Null);
		mm5 = _mm_unpackhi_pi8(mm5, Null);
		
		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_adds_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5

		mm1 = _mm_unpacklo_pi16(Null,tmp);
		mm1 = _mm_srai_pi32(mm1,16);
		*((__m64*)&tmp_res[j][12]) = mm1;
		mm1 = _mm_unpackhi_pi16(Null,tmp);
		mm1 = _mm_srai_pi32(mm1,16);
		*((__m64*)&tmp_res[j][14]) = mm1;
		
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		*((__m64*)&luma_res[j][12]) = tmp;		
		
		src += w1;
	}


	for (j = 0; j < height; j++)
	{
		//  1

		mm0 = *((__m64*)&tmp_res[j][0]);
		mm1 = *((__m64*)&tmp_res[j+1][0]);
		mm2 = *((__m64*)&tmp_res[j+2][0]);
		mm3 = *((__m64*)&tmp_res[j+3][0]);
		mm4 = *((__m64*)&tmp_res[j+4][0]);
		mm5 = *((__m64*)&tmp_res[j+5][0]);

		tmp = _mm_sub_pi32(_mm_slli_pi32(_mm_add_pi32(mm2,mm3),2),_mm_add_pi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi32(_mm_slli_pi32(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi32(_mm_add_pi32(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi32(_mm_add_pi32(tmp,coeff_512),10);//(result_block+512)>>10

	
		mm0 = *((__m64*)&tmp_res[j][2]);
		mm1 = *((__m64*)&tmp_res[j+1][2]);
		mm2 = *((__m64*)&tmp_res[j+2][2]);
		mm3 = *((__m64*)&tmp_res[j+3][2]);
		mm4 = *((__m64*)&tmp_res[j+4][2]);
		mm5 = *((__m64*)&tmp_res[j+5][2]);

		tmp1 = _mm_sub_pi32(_mm_slli_pi32(_mm_add_pi32(mm2,mm3),2),_mm_add_pi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_pi32(_mm_slli_pi32(tmp1,2),tmp1);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp1= _mm_add_pi32(_mm_add_pi32(mm0,mm5),tmp1); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp1 = _mm_srai_pi32(_mm_add_pi32(tmp1,coeff_512),10);//(result_block+512)>>10

		mm1 = _mm_packs_pi32(tmp,tmp1);//check
		*((__m64*)&block[j][0]) = mm1;

		mm0 = *((__m64*)&tmp_res[j][4]);
		mm1 = *((__m64*)&tmp_res[j+1][4]);
		mm2 = *((__m64*)&tmp_res[j+2][4]);
		mm3 = *((__m64*)&tmp_res[j+3][4]);
		mm4 = *((__m64*)&tmp_res[j+4][4]);
		mm5 = *((__m64*)&tmp_res[j+5][4]);

		tmp = _mm_sub_pi32(_mm_slli_pi32(_mm_add_pi32(mm2,mm3),2),_mm_add_pi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi32(_mm_slli_pi32(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi32(_mm_add_pi32(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi32(_mm_add_pi32(tmp,coeff_512),10);//(result_block+512)>>10

		mm0 = *((__m64*)&tmp_res[j][6]);
		mm1 = *((__m64*)&tmp_res[j+1][6]);
		mm2 = *((__m64*)&tmp_res[j+2][6]);
		mm3 = *((__m64*)&tmp_res[j+3][6]);
		mm4 = *((__m64*)&tmp_res[j+4][6]);
		mm5 = *((__m64*)&tmp_res[j+5][6]);

		tmp1 = _mm_sub_pi32(_mm_slli_pi32(_mm_add_pi32(mm2,mm3),2),_mm_add_pi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_pi32(_mm_slli_pi32(tmp1,2),tmp1);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp1= _mm_add_pi32(_mm_add_pi32(mm0,mm5),tmp1); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp1 = _mm_srai_pi32(_mm_add_pi32(tmp1,coeff_512),10);//(result_block+512)>>10

		mm2 = _mm_packs_pi32(tmp,tmp1);//check
		mm1 = *((__m64*)&block[j][0]);

		mm0 = *((__m64*)&luma_res[j+2][0]);
		mm3 = *((__m64*)&luma_res[j+2][4]);
		tmp = _mm_packs_pu16(mm1,mm2);//__fast_iclip0_255((result_block+512)>>10)
		mm0 = _mm_packs_pu16(mm0,mm3);
		tmp = _mm_avg_pu8(tmp,mm0);
		*((__m64*)&block[j][0]) = tmp;
		

		
		mm0 = *((__m64*)&tmp_res[j][8]);
		mm1 = *((__m64*)&tmp_res[j+1][8]);
		mm2 = *((__m64*)&tmp_res[j+2][8]);
		mm3 = *((__m64*)&tmp_res[j+3][8]);
		mm4 = *((__m64*)&tmp_res[j+4][8]);
		mm5 = *((__m64*)&tmp_res[j+5][8]);

		tmp = _mm_sub_pi32(_mm_slli_pi32(_mm_add_pi32(mm2,mm3),2),_mm_add_pi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi32(_mm_slli_pi32(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi32(_mm_add_pi32(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi32(_mm_add_pi32(tmp,coeff_512),10);//(result_block+512)>>10

	
		mm0 = *((__m64*)&tmp_res[j][10]);
		mm1 = *((__m64*)&tmp_res[j+1][10]);
		mm2 = *((__m64*)&tmp_res[j+2][10]);
		mm3 = *((__m64*)&tmp_res[j+3][10]);
		mm4 = *((__m64*)&tmp_res[j+4][10]);
		mm5 = *((__m64*)&tmp_res[j+5][10]);

		tmp1 = _mm_sub_pi32(_mm_slli_pi32(_mm_add_pi32(mm2,mm3),2),_mm_add_pi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_pi32(_mm_slli_pi32(tmp1,2),tmp1);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp1= _mm_add_pi32(_mm_add_pi32(mm0,mm5),tmp1); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp1 = _mm_srai_pi32(_mm_add_pi32(tmp1,coeff_512),10);//(result_block+512)>>10

		mm1 = _mm_packs_pi32(tmp,tmp1);//check
		*((__m64*)&block[j][8]) = mm1;

		mm0 = *((__m64*)&tmp_res[j][12]);
		mm1 = *((__m64*)&tmp_res[j+1][12]);
		mm2 = *((__m64*)&tmp_res[j+2][12]);
		mm3 = *((__m64*)&tmp_res[j+3][12]);
		mm4 = *((__m64*)&tmp_res[j+4][12]);
		mm5 = *((__m64*)&tmp_res[j+5][12]);

		tmp = _mm_sub_pi32(_mm_slli_pi32(_mm_add_pi32(mm2,mm3),2),_mm_add_pi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi32(_mm_slli_pi32(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi32(_mm_add_pi32(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi32(_mm_add_pi32(tmp,coeff_512),10);//(result_block+512)>>10

		mm0 = *((__m64*)&tmp_res[j][14]);
		mm1 = *((__m64*)&tmp_res[j+1][14]);
		mm2 = *((__m64*)&tmp_res[j+2][14]);
		mm3 = *((__m64*)&tmp_res[j+3][14]);
		mm4 = *((__m64*)&tmp_res[j+4][14]);
		mm5 = *((__m64*)&tmp_res[j+5][14]);

		tmp1 = _mm_sub_pi32(_mm_slli_pi32(_mm_add_pi32(mm2,mm3),2),_mm_add_pi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_pi32(_mm_slli_pi32(tmp1,2),tmp1);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp1= _mm_add_pi32(_mm_add_pi32(mm0,mm5),tmp1); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp1 = _mm_srai_pi32(_mm_add_pi32(tmp1,coeff_512),10);//(result_block+512)>>10

		mm2 = _mm_packs_pi32(tmp,tmp1);//check
		mm1 = *((__m64*)&block[j][8]);

		mm0 = *((__m64*)&luma_res[j+2][8]);
		mm3 = *((__m64*)&luma_res[j+2][12]);
		tmp = _mm_packs_pu16(mm1,mm2);//__fast_iclip0_255((result_block+512)>>10)
		mm0 = _mm_packs_pu16(mm0,mm3);
		tmp = _mm_avg_pu8(tmp,mm0);
		*((__m64*)&block[j][8]) = tmp;
		
	}
}
void  get_block_16xh_p21_sse2(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	byte *src;
	int j;
	DO_ALIGN(16, short luma_res[21][16]);
	DO_ALIGN(16, int tmp_res[21][16]);
	const int w1 = stride;

	src = pSrc-(2*w1);

	__m128i mm0,mm1,mm2,mm3,mm4,mm5,tmp,tmp1;
	const __m128i Null      = _mm_setzero_si128();
	const __m128i coeff_16  = _mm_load_si128((__m128i*)ncoeff_16);
	const __m128i coeff_512 = _mm_load_si128((__m128i*)ncoeff_512);

	for (j = 0; j < height+5; j++)
	{
		mm0 = _mm_loadl_epi64(((__m128i*) (src-2)));
		mm1 = _mm_loadl_epi64(((__m128i*) (src-1)));
		mm2 = _mm_loadl_epi64(((__m128i*) (src)));
		mm3 = _mm_loadl_epi64(((__m128i*) (src+1)));
		mm4 = _mm_loadl_epi64(((__m128i*) (src+2)));
		mm5 = _mm_loadl_epi64(((__m128i*) (src+3)));

		mm0 = _mm_unpacklo_epi8(mm0, Null);
		mm1 = _mm_unpacklo_epi8(mm1, Null);
		mm2 = _mm_unpacklo_epi8(mm2, Null);
		mm3 = _mm_unpacklo_epi8(mm3, Null);
		mm4 = _mm_unpacklo_epi8(mm4, Null);
		mm5 = _mm_unpacklo_epi8(mm5, Null);

		tmp = _mm_sub_epi16(_mm_slli_epi16(_mm_add_epi16(mm2,mm3),2),_mm_adds_epi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_slli_epi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_add_epi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5

		mm0 = _mm_unpacklo_epi16(Null,tmp);
		mm1 = _mm_unpackhi_epi16(Null,tmp);

		mm0 = _mm_srai_epi32(mm0,16);
		mm1 = _mm_srai_epi32(mm1,16);

		_mm_store_si128(((__m128i*)&tmp_res[j][0]),mm0);
		_mm_store_si128(((__m128i*)&tmp_res[j][4]),mm1);

		tmp = _mm_srai_epi16(_mm_add_epi16(tmp,coeff_16),5);//(result_block+16)>>5
		_mm_store_si128(((__m128i*)&luma_res[j][0]),tmp);

		mm0 = _mm_loadl_epi64(((__m128i*) (src+6)));
		mm1 = _mm_loadl_epi64(((__m128i*) (src+7)));
		mm2 = _mm_loadl_epi64(((__m128i*) (src+8)));
		mm3 = _mm_loadl_epi64(((__m128i*) (src+9)));
		mm4 = _mm_loadl_epi64(((__m128i*) (src+10)));
		mm5 = _mm_loadl_epi64(((__m128i*) (src+11)));

		mm0 = _mm_unpacklo_epi8(mm0, Null);
		mm1 = _mm_unpacklo_epi8(mm1, Null);
		mm2 = _mm_unpacklo_epi8(mm2, Null);
		mm3 = _mm_unpacklo_epi8(mm3, Null);
		mm4 = _mm_unpacklo_epi8(mm4, Null);
		mm5 = _mm_unpacklo_epi8(mm5, Null);

		tmp = _mm_sub_epi16(_mm_slli_epi16(_mm_add_epi16(mm2,mm3),2),_mm_adds_epi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_slli_epi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_add_epi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5

		mm0 = _mm_unpacklo_epi16(Null,tmp);
		mm1 = _mm_unpackhi_epi16(Null,tmp);

		mm0 = _mm_srai_epi32(mm0,16);
		mm1 = _mm_srai_epi32(mm1,16);

		_mm_store_si128(((__m128i*)&tmp_res[j][8]),mm0);
		_mm_store_si128(((__m128i*)&tmp_res[j][12]),mm1);

		tmp = _mm_srai_epi16(_mm_add_epi16(tmp,coeff_16),5);//(result_block+16)>>5
		_mm_store_si128(((__m128i*)&luma_res[j][8]),tmp);

		src += w1;
	}


	for (j = 0; j < height; j++)
	{
		mm0 = _mm_load_si128((__m128i*)&tmp_res[j][0]);
		mm1 = _mm_load_si128((__m128i*)&tmp_res[j+1][0]);
		mm2 = _mm_load_si128((__m128i*)&tmp_res[j+2][0]);
		mm3 = _mm_load_si128((__m128i*)&tmp_res[j+3][0]);
		mm4 = _mm_load_si128((__m128i*)&tmp_res[j+4][0]);
		mm5 = _mm_load_si128((__m128i*)&tmp_res[j+5][0]);

		tmp = _mm_sub_epi32(_mm_slli_epi32(_mm_add_epi32(mm2,mm3),2),_mm_add_epi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi32(_mm_slli_epi32(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi32(_mm_add_epi32(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_epi32(_mm_add_epi32(tmp,coeff_512),10);//(result_block+512)>>10

		mm0 = _mm_load_si128((__m128i*)&tmp_res[j][4]);
		mm1 = _mm_load_si128((__m128i*)&tmp_res[j+1][4]);
		mm2 = _mm_load_si128((__m128i*)&tmp_res[j+2][4]);
		mm3 = _mm_load_si128((__m128i*)&tmp_res[j+3][4]);
		mm4 = _mm_load_si128((__m128i*)&tmp_res[j+4][4]);
		mm5 = _mm_load_si128((__m128i*)&tmp_res[j+5][4]);

		tmp1 = _mm_sub_epi32(_mm_slli_epi32(_mm_add_epi32(mm2,mm3),2),_mm_add_epi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_epi32(_mm_slli_epi32(tmp1,2),tmp1);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp1= _mm_add_epi32(_mm_add_epi32(mm0,mm5),tmp1); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp1 = _mm_srai_epi32(_mm_add_epi32(tmp1,coeff_512),10);//(result_block+512)>>10

		tmp = _mm_packs_epi32(tmp,tmp1);//check

		mm0 = _mm_load_si128((__m128i*)&luma_res[j+2][0]);	//__fast_iclip0_255(((tmp_res[j+2][i]+16)>>5))
		tmp = _mm_packus_epi16(tmp,tmp);//__fast_iclip0_255((result_block+512)>>10)
		mm0 = _mm_packus_epi16(mm0,mm0);
		tmp = _mm_avg_epu8(tmp,mm0);
		_mm_storel_epi64((__m128i*)&block[j][0], tmp);

		mm0 = _mm_load_si128((__m128i*)&tmp_res[j][8]);
		mm1 = _mm_load_si128((__m128i*)&tmp_res[j+1][8]);
		mm2 = _mm_load_si128((__m128i*)&tmp_res[j+2][8]);
		mm3 = _mm_load_si128((__m128i*)&tmp_res[j+3][8]);
		mm4 = _mm_load_si128((__m128i*)&tmp_res[j+4][8]);
		mm5 = _mm_load_si128((__m128i*)&tmp_res[j+5][8]);

		tmp = _mm_sub_epi32(_mm_slli_epi32(_mm_add_epi32(mm2,mm3),2),_mm_add_epi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi32(_mm_slli_epi32(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi32(_mm_add_epi32(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_epi32(_mm_add_epi32(tmp,coeff_512),10);//(result_block+512)>>10

		mm0 = _mm_load_si128((__m128i*)&tmp_res[j][12]);
		mm1 = _mm_load_si128((__m128i*)&tmp_res[j+1][12]);
		mm2 = _mm_load_si128((__m128i*)&tmp_res[j+2][12]);
		mm3 = _mm_load_si128((__m128i*)&tmp_res[j+3][12]);
		mm4 = _mm_load_si128((__m128i*)&tmp_res[j+4][12]);
		mm5 = _mm_load_si128((__m128i*)&tmp_res[j+5][12]);

		tmp1 = _mm_sub_epi32(_mm_slli_epi32(_mm_add_epi32(mm2,mm3),2),_mm_add_epi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_epi32(_mm_slli_epi32(tmp1,2),tmp1);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp1= _mm_add_epi32(_mm_add_epi32(mm0,mm5),tmp1); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp1 = _mm_srai_epi32(_mm_add_epi32(tmp1,coeff_512),10);//(result_block+512)>>10

		tmp = _mm_packs_epi32(tmp,tmp1);//check

		mm0 = _mm_load_si128((__m128i*)&luma_res[j+2][8]);	//__fast_iclip0_255(((tmp_res[j+2][i]+16)>>5))
		tmp = _mm_packus_epi16(tmp,tmp);//__fast_iclip0_255((result_block+512)>>10)
		mm0 = _mm_packus_epi16(mm0,mm0);
		tmp = _mm_avg_epu8(tmp,mm0);

		_mm_storel_epi64((__m128i*)&block[j][8], tmp);
	}
}

void  get_block_16xh_p22_sse(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	byte *src;
	int j;
	DO_ALIGN(16, int tmp_res[21][16]);
	const int w1 = stride;

	src = pSrc - (2*w1);

	__m64 mm0, mm1, mm2, mm3, mm4, mm5, tmp, tmp1;
	const __m64 Null     = _mm_setzero_si64();
	const __m64 coeff_512 = *((__m64*)ncoeff_512);

	for (j = 0; j < height+5; j++)
	{
		mm0 = *((__m64*)(src-2));
		mm1 = *((__m64*)(src-1));
		mm2 = *((__m64*)(src));
		mm3 = *((__m64*)(src+1));
		mm4 = *((__m64*)(src+2));
		mm5 = *((__m64*)(src+3));


		mm0 = _mm_unpacklo_pi8(mm0, Null);
		mm1 = _mm_unpacklo_pi8(mm1, Null);
		mm2 = _mm_unpacklo_pi8(mm2, Null);
		mm3 = _mm_unpacklo_pi8(mm3, Null);
		mm4 = _mm_unpacklo_pi8(mm4, Null);
		mm5 = _mm_unpacklo_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5

		mm0 = _mm_unpacklo_pi16(Null,tmp);
		mm0 = _mm_srai_pi32(mm0,16);
		*((__m64*)&tmp_res[j][0]) = mm0;
		mm0 = _mm_unpackhi_pi16(Null,tmp);
		mm0 = _mm_srai_pi32(mm0,16);
		*((__m64*)&tmp_res[j][2]) = mm0;
		
		mm0 = *((__m64*)(src-2));
		mm1 = *((__m64*)(src-1));
		mm2 = *((__m64*)(src));
		mm3 = *((__m64*)(src+1));
		mm4 = *((__m64*)(src+2));
		mm5 = *((__m64*)(src+3));

		mm0 = _mm_unpackhi_pi8(mm0, Null);
		mm1 = _mm_unpackhi_pi8(mm1, Null);
		mm2 = _mm_unpackhi_pi8(mm2, Null);
		mm3 = _mm_unpackhi_pi8(mm3, Null);
		mm4 = _mm_unpackhi_pi8(mm4, Null);
		mm5 = _mm_unpackhi_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5

		mm0 = _mm_unpacklo_pi16(Null,tmp);
		mm0 = _mm_srai_pi32(mm0,16);
		*((__m64*)&tmp_res[j][4]) = mm0;
		mm0 = _mm_unpackhi_pi16(Null,tmp);
		mm0 = _mm_srai_pi32(mm0,16);
		*((__m64*)&tmp_res[j][6]) = mm0;
		
		
		mm0 = *((__m64*)(src+6));
		mm1 = *((__m64*)(src+7));
		mm2 = *((__m64*)(src+8));
		mm3 = *((__m64*)(src+9));
		mm4 = *((__m64*)(src+10));
		mm5 = *((__m64*)(src+11));

		mm0 = _mm_unpacklo_pi8(mm0, Null);
		mm1 = _mm_unpacklo_pi8(mm1, Null);
		mm2 = _mm_unpacklo_pi8(mm2, Null);
		mm3 = _mm_unpacklo_pi8(mm3, Null);
		mm4 = _mm_unpacklo_pi8(mm4, Null);
		mm5 = _mm_unpacklo_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5

		mm0 = _mm_unpacklo_pi16(Null,tmp);
		mm0 = _mm_srai_pi32(mm0,16);
		*((__m64*)&tmp_res[j][8]) = mm0;
		mm0 = _mm_unpackhi_pi16(Null,tmp);
		mm0 = _mm_srai_pi32(mm0,16);
		*((__m64*)&tmp_res[j][10]) = mm0;

		mm0 = *((__m64*)(src+6));
		mm1 = *((__m64*)(src+7));
		mm2 = *((__m64*)(src+8));
		mm3 = *((__m64*)(src+9));
		mm4 = *((__m64*)(src+10));
		mm5 = *((__m64*)(src+11));

		mm0 = _mm_unpackhi_pi8(mm0, Null);
		mm1 = _mm_unpackhi_pi8(mm1, Null);
		mm2 = _mm_unpackhi_pi8(mm2, Null);
		mm3 = _mm_unpackhi_pi8(mm3, Null);
		mm4 = _mm_unpackhi_pi8(mm4, Null);
		mm5 = _mm_unpackhi_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5

		mm0 = _mm_unpacklo_pi16(Null,tmp);
		mm0 = _mm_srai_pi32(mm0,16);
		*((__m64*)&tmp_res[j][12]) = mm0;
		mm0 = _mm_unpackhi_pi16(Null,tmp);
		mm0 = _mm_srai_pi32(mm0,16);
		*((__m64*)&tmp_res[j][14]) = mm0;

		src += w1;
	}

	for (j = 0; j < height; j++)
	{
		mm0 = *((__m64*)&tmp_res[j][0]);
		mm1 = *((__m64*)&tmp_res[j+1][0]);
		mm2 = *((__m64*)&tmp_res[j+2][0]);
		mm3 = *((__m64*)&tmp_res[j+3][0]);
		mm4 = *((__m64*)&tmp_res[j+4][0]);
		mm5 = *((__m64*)&tmp_res[j+5][0]);

		tmp = _mm_sub_pi32(_mm_slli_pi32(_mm_add_pi32(mm2,mm3),2),_mm_add_pi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi32(_mm_slli_pi32(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi32(_mm_add_pi32(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi32(_mm_add_pi32(tmp,coeff_512),10);//(result_block+512)>>10

		
		mm0 = *((__m64*)&tmp_res[j][2]);
		mm1 = *((__m64*)&tmp_res[j+1][2]);
		mm2 = *((__m64*)&tmp_res[j+2][2]);
		mm3 = *((__m64*)&tmp_res[j+3][2]);
		mm4 = *((__m64*)&tmp_res[j+4][2]);
		mm5 = *((__m64*)&tmp_res[j+5][2]);

		tmp1 = _mm_sub_pi32(_mm_slli_pi32(_mm_add_pi32(mm2,mm3),2),_mm_add_pi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_pi32(_mm_slli_pi32(tmp1,2),tmp1);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_pi32(_mm_add_pi32(mm0,mm5),tmp1); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp1 = _mm_srai_pi32(_mm_add_pi32(tmp1,coeff_512),10);//(result_block+512)>>10

		tmp = _mm_packs_pi32(tmp,tmp1);
		*((__m64*)&block[j][0]) = tmp;
		
		
		mm0 = *((__m64*)&tmp_res[j][4]);
		mm1 = *((__m64*)&tmp_res[j+1][4]);
		mm2 = *((__m64*)&tmp_res[j+2][4]);
		mm3 = *((__m64*)&tmp_res[j+3][4]);
		mm4 = *((__m64*)&tmp_res[j+4][4]);
		mm5 = *((__m64*)&tmp_res[j+5][4]);

		tmp = _mm_sub_pi32(_mm_slli_pi32(_mm_add_pi32(mm2,mm3),2),_mm_add_pi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi32(_mm_slli_pi32(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi32(_mm_add_pi32(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi32(_mm_add_pi32(tmp,coeff_512),10);//(result_block+512)>>10

		
		mm0 = *((__m64*)&tmp_res[j][6]);
		mm1 = *((__m64*)&tmp_res[j+1][6]);
		mm2 = *((__m64*)&tmp_res[j+2][6]);
		mm3 = *((__m64*)&tmp_res[j+3][6]);
		mm4 = *((__m64*)&tmp_res[j+4][6]);
		mm5 = *((__m64*)&tmp_res[j+5][6]);

		tmp1 = _mm_sub_pi32(_mm_slli_pi32(_mm_add_pi32(mm2,mm3),2),_mm_add_pi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_pi32(_mm_slli_pi32(tmp1,2),tmp1);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_pi32(_mm_add_pi32(mm0,mm5),tmp1); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp1 = _mm_srai_pi32(_mm_add_pi32(tmp1,coeff_512),10);//(result_block+512)>>10

		tmp1 = _mm_packs_pi32(tmp,tmp1);
		tmp = *((__m64*)&block[j][0]);
		tmp = _mm_packs_pu16(tmp,tmp1);//__fast_iclip0_255((result_block+512)>>10)
		*((__m64*)&block[j][0]) = tmp;
		
		
		mm0 = *((__m64*)&tmp_res[j][8]);
		mm1 = *((__m64*)&tmp_res[j+1][8]);
		mm2 = *((__m64*)&tmp_res[j+2][8]);
		mm3 = *((__m64*)&tmp_res[j+3][8]);
		mm4 = *((__m64*)&tmp_res[j+4][8]);
		mm5 = *((__m64*)&tmp_res[j+5][8]);

		tmp = _mm_sub_pi32(_mm_slli_pi32(_mm_add_pi32(mm2,mm3),2),_mm_add_pi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi32(_mm_slli_pi32(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi32(_mm_add_pi32(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi32(_mm_add_pi32(tmp,coeff_512),10);//(result_block+512)>>10

		
		mm0 = *((__m64*)&tmp_res[j][10]);
		mm1 = *((__m64*)&tmp_res[j+1][10]);
		mm2 = *((__m64*)&tmp_res[j+2][10]);
		mm3 = *((__m64*)&tmp_res[j+3][10]);
		mm4 = *((__m64*)&tmp_res[j+4][10]);
		mm5 = *((__m64*)&tmp_res[j+5][10]);

		tmp1 = _mm_sub_pi32(_mm_slli_pi32(_mm_add_pi32(mm2,mm3),2),_mm_add_pi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_pi32(_mm_slli_pi32(tmp1,2),tmp1);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_pi32(_mm_add_pi32(mm0,mm5),tmp1); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp1 = _mm_srai_pi32(_mm_add_pi32(tmp1,coeff_512),10);//(result_block+512)>>10

		tmp = _mm_packs_pi32(tmp,tmp1);
		*((__m64*)&block[j][8]) = tmp;
		
		
		mm0 = *((__m64*)&tmp_res[j][12]);
		mm1 = *((__m64*)&tmp_res[j+1][12]);
		mm2 = *((__m64*)&tmp_res[j+2][12]);
		mm3 = *((__m64*)&tmp_res[j+3][12]);
		mm4 = *((__m64*)&tmp_res[j+4][12]);
		mm5 = *((__m64*)&tmp_res[j+5][12]);

		tmp = _mm_sub_pi32(_mm_slli_pi32(_mm_add_pi32(mm2,mm3),2),_mm_add_pi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi32(_mm_slli_pi32(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi32(_mm_add_pi32(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi32(_mm_add_pi32(tmp,coeff_512),10);//(result_block+512)>>10

	
		mm0 = *((__m64*)&tmp_res[j][14]);
		mm1 = *((__m64*)&tmp_res[j+1][14]);
		mm2 = *((__m64*)&tmp_res[j+2][14]);
		mm3 = *((__m64*)&tmp_res[j+3][14]);
		mm4 = *((__m64*)&tmp_res[j+4][14]);
		mm5 = *((__m64*)&tmp_res[j+5][14]);

		tmp1 = _mm_sub_pi32(_mm_slli_pi32(_mm_add_pi32(mm2,mm3),2),_mm_add_pi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_pi32(_mm_slli_pi32(tmp1,2),tmp1);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_pi32(_mm_add_pi32(mm0,mm5),tmp1); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp1 = _mm_srai_pi32(_mm_add_pi32(tmp1,coeff_512),10);//(result_block+512)>>10

		tmp1 = _mm_packs_pi32(tmp,tmp1);
		tmp = *((__m64*)&block[j][8]);
		tmp = _mm_packs_pu16(tmp,tmp1);//__fast_iclip0_255((result_block+512)>>10)
		*((__m64*)&block[j][8]) = tmp;
		
	}
}
void  get_block_16xh_p22_sse2(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	byte *src;
	int j;
	DO_ALIGN(16, int tmp_res[21][16]);
	const int w1 = stride;

	src = pSrc - (2*w1);

	__m128i mm0,mm1,mm2,mm3,mm4,mm5,tmp,tmp1;
	const __m128i Null      = _mm_setzero_si128();
	const __m128i coeff_512 = _mm_load_si128((__m128i*)ncoeff_512);

	for (j = 0; j < height+5; j++)
	{
		mm0 = _mm_loadl_epi64(((__m128i*) (src-2)));
		mm1 = _mm_loadl_epi64(((__m128i*) (src-1)));
		mm2 = _mm_loadl_epi64(((__m128i*) (src)));
		mm3 = _mm_loadl_epi64(((__m128i*) (src+1)));
		mm4 = _mm_loadl_epi64(((__m128i*) (src+2)));
		mm5 = _mm_loadl_epi64(((__m128i*) (src+3)));

		mm0 = _mm_unpacklo_epi8(mm0, Null);
		mm1 = _mm_unpacklo_epi8(mm1, Null);
		mm2 = _mm_unpacklo_epi8(mm2, Null);
		mm3 = _mm_unpacklo_epi8(mm3, Null);
		mm4 = _mm_unpacklo_epi8(mm4, Null);
		mm5 = _mm_unpacklo_epi8(mm5, Null);

		tmp = _mm_sub_epi16(_mm_slli_epi16(_mm_add_epi16(mm2,mm3),2),_mm_add_epi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_slli_epi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_add_epi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5

		mm0 = _mm_unpacklo_epi16(Null,tmp);
		mm1 = _mm_unpackhi_epi16(Null,tmp);

		mm0 = _mm_srai_epi32(mm0,16);
		mm1 = _mm_srai_epi32(mm1,16);

		_mm_store_si128(((__m128i*)&tmp_res[j][0]),mm0);
		_mm_store_si128(((__m128i*)&tmp_res[j][4]),mm1);

		mm0 = _mm_loadl_epi64(((__m128i*) (src+6)));
		mm1 = _mm_loadl_epi64(((__m128i*) (src+7)));
		mm2 = _mm_loadl_epi64(((__m128i*) (src+8)));
		mm3 = _mm_loadl_epi64(((__m128i*) (src+9)));
		mm4 = _mm_loadl_epi64(((__m128i*) (src+10)));
		mm5 = _mm_loadl_epi64(((__m128i*) (src+11)));

		mm0 = _mm_unpacklo_epi8(mm0, Null);
		mm1 = _mm_unpacklo_epi8(mm1, Null);
		mm2 = _mm_unpacklo_epi8(mm2, Null);
		mm3 = _mm_unpacklo_epi8(mm3, Null);
		mm4 = _mm_unpacklo_epi8(mm4, Null);
		mm5 = _mm_unpacklo_epi8(mm5, Null);

		tmp = _mm_sub_epi16(_mm_slli_epi16(_mm_add_epi16(mm2,mm3),2),_mm_add_epi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_slli_epi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_add_epi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5

		mm0 = _mm_unpacklo_epi16(Null,tmp);
		mm1 = _mm_unpackhi_epi16(Null,tmp);

		mm0 = _mm_srai_epi32(mm0,16);
		mm1 = _mm_srai_epi32(mm1,16);

		_mm_store_si128(((__m128i*)&tmp_res[j][8]),mm0);
		_mm_store_si128(((__m128i*)&tmp_res[j][12]),mm1);

		src += w1;
	}

	for (j = 0; j < height; j++)
	{
		mm0 = _mm_load_si128((__m128i*)&tmp_res[j][0]);
		mm1 = _mm_load_si128((__m128i*)&tmp_res[j+1][0]);
		mm2 = _mm_load_si128((__m128i*)&tmp_res[j+2][0]);
		mm3 = _mm_load_si128((__m128i*)&tmp_res[j+3][0]);
		mm4 = _mm_load_si128((__m128i*)&tmp_res[j+4][0]);
		mm5 = _mm_load_si128((__m128i*)&tmp_res[j+5][0]);

		tmp = _mm_sub_epi32(_mm_slli_epi32(_mm_add_epi32(mm2,mm3),2),_mm_add_epi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi32(_mm_slli_epi32(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi32(_mm_add_epi32(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_epi32(_mm_add_epi32(tmp,coeff_512),10);//(result_block+512)>>10

		mm0 = _mm_load_si128((__m128i*)&tmp_res[j][4]);
		mm1 = _mm_load_si128((__m128i*)&tmp_res[j+1][4]);
		mm2 = _mm_load_si128((__m128i*)&tmp_res[j+2][4]);
		mm3 = _mm_load_si128((__m128i*)&tmp_res[j+3][4]);
		mm4 = _mm_load_si128((__m128i*)&tmp_res[j+4][4]);
		mm5 = _mm_load_si128((__m128i*)&tmp_res[j+5][4]);

		tmp1 = _mm_sub_epi32(_mm_slli_epi32(_mm_add_epi32(mm2,mm3),2),_mm_add_epi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_epi32(_mm_slli_epi32(tmp1,2),tmp1);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_epi32(_mm_add_epi32(mm0,mm5),tmp1); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp1 = _mm_srai_epi32(_mm_add_epi32(tmp1,coeff_512),10);//(result_block+512)>>10

		tmp = _mm_packs_epi32(tmp,tmp1);
		tmp = _mm_packus_epi16(tmp,tmp);//__fast_iclip0_255((result_block+512)>>10)
		_mm_storel_epi64((__m128i*)&block[j][0], tmp);

		mm0 = _mm_load_si128((__m128i*)&tmp_res[j][8]);
		mm1 = _mm_load_si128((__m128i*)&tmp_res[j+1][8]);
		mm2 = _mm_load_si128((__m128i*)&tmp_res[j+2][8]);
		mm3 = _mm_load_si128((__m128i*)&tmp_res[j+3][8]);
		mm4 = _mm_load_si128((__m128i*)&tmp_res[j+4][8]);
		mm5 = _mm_load_si128((__m128i*)&tmp_res[j+5][8]);

		tmp = _mm_sub_epi32(_mm_slli_epi32(_mm_add_epi32(mm2,mm3),2),_mm_add_epi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi32(_mm_slli_epi32(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi32(_mm_add_epi32(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_epi32(_mm_add_epi32(tmp,coeff_512),10);//(result_block+512)>>10

		mm0 = _mm_load_si128((__m128i*)&tmp_res[j][12]);
		mm1 = _mm_load_si128((__m128i*)&tmp_res[j+1][12]);
		mm2 = _mm_load_si128((__m128i*)&tmp_res[j+2][12]);
		mm3 = _mm_load_si128((__m128i*)&tmp_res[j+3][12]);
		mm4 = _mm_load_si128((__m128i*)&tmp_res[j+4][12]);
		mm5 = _mm_load_si128((__m128i*)&tmp_res[j+5][12]);

		tmp1 = _mm_sub_epi32(_mm_slli_epi32(_mm_add_epi32(mm2,mm3),2),_mm_add_epi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_epi32(_mm_slli_epi32(tmp1,2),tmp1);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_epi32(_mm_add_epi32(mm0,mm5),tmp1); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp1 = _mm_srai_epi32(_mm_add_epi32(tmp1,coeff_512),10);//(result_block+512)>>10

		tmp = _mm_packs_epi32(tmp,tmp1);//check
		tmp = _mm_packus_epi16(tmp,tmp);//__fast_iclip0_255((result_block+512)>>10)
		_mm_storel_epi64((__m128i*)&block[j][8], tmp);
	}
}
void  get_block_16xh_p23_sse(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int j;
	DO_ALIGN(16, short luma_res[21][16]);
	DO_ALIGN(16, int tmp_res[21][16]);
	byte *src;
	const int w1 = stride;

	src = pSrc -(2*w1);

	__m64 mm0, mm1, mm2, mm3, mm4, mm5, tmp, tmp1;
	const __m64 Null     = _mm_setzero_si64();
	const __m64 coeff_16 = *((__m64*)ncoeff_16);
	const __m64 coeff_512 = *((__m64*)ncoeff_512);

	for (j = 0; j < height+5; j++)
	{
		mm0 = *((__m64*)(src-2));
		mm1 = *((__m64*)(src-1));
		mm2 = *((__m64*)(src));
		mm3 = *((__m64*)(src+1));
		mm4 = *((__m64*)(src+2));
		mm5 = *((__m64*)(src+3));

		mm0 = _mm_unpacklo_pi8(mm0, Null);
		mm1 = _mm_unpacklo_pi8(mm1, Null);
		mm2 = _mm_unpacklo_pi8(mm2, Null);
		mm3 = _mm_unpacklo_pi8(mm3, Null);
		mm4 = _mm_unpacklo_pi8(mm4, Null);
		mm5 = _mm_unpacklo_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		
		mm0 = _mm_unpacklo_pi16(Null,tmp);
		mm0 = _mm_srai_pi32(mm0,16);		
		*((__m64*)&tmp_res[j][0]) = mm0;
		mm0 = _mm_unpackhi_pi16(Null,tmp);
		mm0 = _mm_srai_pi32(mm0,16);		
		*((__m64*)&tmp_res[j][2]) = mm0;
		
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		*((__m64*)&luma_res[j][0]) = tmp;
		
		
		mm0 = *((__m64*)(src-2));
		mm1 = *((__m64*)(src-1));
		mm2 = *((__m64*)(src));
		mm3 = *((__m64*)(src+1));
		mm4 = *((__m64*)(src+2));
		mm5 = *((__m64*)(src+3));

		mm0 = _mm_unpackhi_pi8(mm0, Null);
		mm1 = _mm_unpackhi_pi8(mm1, Null);
		mm2 = _mm_unpackhi_pi8(mm2, Null);
		mm3 = _mm_unpackhi_pi8(mm3, Null);
		mm4 = _mm_unpackhi_pi8(mm4, Null);
		mm5 = _mm_unpackhi_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		
		mm1 = _mm_unpacklo_pi16(Null,tmp);
		mm1 = _mm_srai_pi32(mm1,16);
		*((__m64*)&tmp_res[j][4]) = mm1;
		mm1 = _mm_unpackhi_pi16(Null,tmp);
		mm1 = _mm_srai_pi32(mm1,16);
		*((__m64*)&tmp_res[j][6]) = mm1;
		
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		*((__m64*)&luma_res[j][4]) = tmp;


		mm0 = *((__m64*)(src+6));
		mm1 = *((__m64*)(src+7));
		mm2 = *((__m64*)(src+8));
		mm3 = *((__m64*)(src+9));
		mm4 = *((__m64*)(src+10));
		mm5 = *((__m64*)(src+11));
		
		mm0 = _mm_unpacklo_pi8(mm0, Null);
		mm1 = _mm_unpacklo_pi8(mm1, Null);
		mm2 = _mm_unpacklo_pi8(mm2, Null);
		mm3 = _mm_unpacklo_pi8(mm3, Null);
		mm4 = _mm_unpacklo_pi8(mm4, Null);
		mm5 = _mm_unpacklo_pi8(mm5, Null);
		
		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_adds_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5

		mm0 = _mm_unpacklo_pi16(Null,tmp);
		mm0 = _mm_srai_pi32(mm0,16);		
		*((__m64*)&tmp_res[j][8]) = mm0;
		mm0 = _mm_unpackhi_pi16(Null,tmp);
		mm0 = _mm_srai_pi32(mm0,16);		
		*((__m64*)&tmp_res[j][10]) = mm0;
		
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		*((__m64*)&luma_res[j][8]) = tmp;

			mm0 = *((__m64*)(src+6));
		mm1 = *((__m64*)(src+7));
		mm2 = *((__m64*)(src+8));
		mm3 = *((__m64*)(src+9));
		mm4 = *((__m64*)(src+10));
		mm5 = *((__m64*)(src+11));
		
		mm0 = _mm_unpackhi_pi8(mm0, Null);
		mm1 = _mm_unpackhi_pi8(mm1, Null);
		mm2 = _mm_unpackhi_pi8(mm2, Null);
		mm3 = _mm_unpackhi_pi8(mm3, Null);
		mm4 = _mm_unpackhi_pi8(mm4, Null);
		mm5 = _mm_unpackhi_pi8(mm5, Null);
		
		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_adds_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5

		mm1 = _mm_unpacklo_pi16(Null,tmp);
		mm1 = _mm_srai_pi32(mm1,16);
		*((__m64*)&tmp_res[j][12]) = mm1;
		mm1 = _mm_unpackhi_pi16(Null,tmp);
		mm1 = _mm_srai_pi32(mm1,16);
		*((__m64*)&tmp_res[j][14]) = mm1;
		
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		*((__m64*)&luma_res[j][12]) = tmp;		
		
		src += w1;
	}


	for (j = 0; j < height; j++)
	{
		mm0 = *((__m64*)&tmp_res[j][0]);
		mm1 = *((__m64*)&tmp_res[j+1][0]);
		mm2 = *((__m64*)&tmp_res[j+2][0]);
		mm3 = *((__m64*)&tmp_res[j+3][0]);
		mm4 = *((__m64*)&tmp_res[j+4][0]);
		mm5 = *((__m64*)&tmp_res[j+5][0]);

		tmp = _mm_sub_pi32(_mm_slli_pi32(_mm_add_pi32(mm2,mm3),2),_mm_add_pi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi32(_mm_slli_pi32(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi32(_mm_add_pi32(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi32(_mm_add_pi32(tmp,coeff_512),10);//(result_block+512)>>10

	
		mm0 = *((__m64*)&tmp_res[j][2]);
		mm1 = *((__m64*)&tmp_res[j+1][2]);
		mm2 = *((__m64*)&tmp_res[j+2][2]);
		mm3 = *((__m64*)&tmp_res[j+3][2]);
		mm4 = *((__m64*)&tmp_res[j+4][2]);
		mm5 = *((__m64*)&tmp_res[j+5][2]);

		tmp1 = _mm_sub_pi32(_mm_slli_pi32(_mm_add_pi32(mm2,mm3),2),_mm_add_pi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_pi32(_mm_slli_pi32(tmp1,2),tmp1);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp1= _mm_add_pi32(_mm_add_pi32(mm0,mm5),tmp1); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp1 = _mm_srai_pi32(_mm_add_pi32(tmp1,coeff_512),10);//(result_block+512)>>10

		mm1 = _mm_packs_pi32(tmp,tmp1);//check
		*((__m64*)&block[j][0]) = mm1;

		mm0 = *((__m64*)&tmp_res[j][4]);
		mm1 = *((__m64*)&tmp_res[j+1][4]);
		mm2 = *((__m64*)&tmp_res[j+2][4]);
		mm3 = *((__m64*)&tmp_res[j+3][4]);
		mm4 = *((__m64*)&tmp_res[j+4][4]);
		mm5 = *((__m64*)&tmp_res[j+5][4]);

		tmp = _mm_sub_pi32(_mm_slli_pi32(_mm_add_pi32(mm2,mm3),2),_mm_add_pi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi32(_mm_slli_pi32(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi32(_mm_add_pi32(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi32(_mm_add_pi32(tmp,coeff_512),10);//(result_block+512)>>10

		mm0 = *((__m64*)&tmp_res[j][6]);
		mm1 = *((__m64*)&tmp_res[j+1][6]);
		mm2 = *((__m64*)&tmp_res[j+2][6]);
		mm3 = *((__m64*)&tmp_res[j+3][6]);
		mm4 = *((__m64*)&tmp_res[j+4][6]);
		mm5 = *((__m64*)&tmp_res[j+5][6]);

		tmp1 = _mm_sub_pi32(_mm_slli_pi32(_mm_add_pi32(mm2,mm3),2),_mm_add_pi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_pi32(_mm_slli_pi32(tmp1,2),tmp1);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp1= _mm_add_pi32(_mm_add_pi32(mm0,mm5),tmp1); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp1 = _mm_srai_pi32(_mm_add_pi32(tmp1,coeff_512),10);//(result_block+512)>>10

		mm2 = _mm_packs_pi32(tmp,tmp1);//check
		mm1 = *((__m64*)&block[j][0]);

		mm0 = *((__m64*)&luma_res[j+3][0]);
		mm3 = *((__m64*)&luma_res[j+3][4]);
		tmp = _mm_packs_pu16(mm1,mm2);//__fast_iclip0_255((result_block+512)>>10)
		mm0 = _mm_packs_pu16(mm0,mm3);
		tmp = _mm_avg_pu8(tmp,mm0);
		*((__m64*)&block[j][0]) = tmp;
		

		
		mm0 = *((__m64*)&tmp_res[j][8]);
		mm1 = *((__m64*)&tmp_res[j+1][8]);
		mm2 = *((__m64*)&tmp_res[j+2][8]);
		mm3 = *((__m64*)&tmp_res[j+3][8]);
		mm4 = *((__m64*)&tmp_res[j+4][8]);
		mm5 = *((__m64*)&tmp_res[j+5][8]);

		tmp = _mm_sub_pi32(_mm_slli_pi32(_mm_add_pi32(mm2,mm3),2),_mm_add_pi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi32(_mm_slli_pi32(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi32(_mm_add_pi32(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi32(_mm_add_pi32(tmp,coeff_512),10);//(result_block+512)>>10

	
		mm0 = *((__m64*)&tmp_res[j][10]);
		mm1 = *((__m64*)&tmp_res[j+1][10]);
		mm2 = *((__m64*)&tmp_res[j+2][10]);
		mm3 = *((__m64*)&tmp_res[j+3][10]);
		mm4 = *((__m64*)&tmp_res[j+4][10]);
		mm5 = *((__m64*)&tmp_res[j+5][10]);

		tmp1 = _mm_sub_pi32(_mm_slli_pi32(_mm_add_pi32(mm2,mm3),2),_mm_add_pi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_pi32(_mm_slli_pi32(tmp1,2),tmp1);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp1= _mm_add_pi32(_mm_add_pi32(mm0,mm5),tmp1); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp1 = _mm_srai_pi32(_mm_add_pi32(tmp1,coeff_512),10);//(result_block+512)>>10

		mm1 = _mm_packs_pi32(tmp,tmp1);//check
		*((__m64*)&block[j][8]) = mm1;

		mm0 = *((__m64*)&tmp_res[j][12]);
		mm1 = *((__m64*)&tmp_res[j+1][12]);
		mm2 = *((__m64*)&tmp_res[j+2][12]);
		mm3 = *((__m64*)&tmp_res[j+3][12]);
		mm4 = *((__m64*)&tmp_res[j+4][12]);
		mm5 = *((__m64*)&tmp_res[j+5][12]);

		tmp = _mm_sub_pi32(_mm_slli_pi32(_mm_add_pi32(mm2,mm3),2),_mm_add_pi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi32(_mm_slli_pi32(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi32(_mm_add_pi32(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi32(_mm_add_pi32(tmp,coeff_512),10);//(result_block+512)>>10

		mm0 = *((__m64*)&tmp_res[j][14]);
		mm1 = *((__m64*)&tmp_res[j+1][14]);
		mm2 = *((__m64*)&tmp_res[j+2][14]);
		mm3 = *((__m64*)&tmp_res[j+3][14]);
		mm4 = *((__m64*)&tmp_res[j+4][14]);
		mm5 = *((__m64*)&tmp_res[j+5][14]);

		tmp1 = _mm_sub_pi32(_mm_slli_pi32(_mm_add_pi32(mm2,mm3),2),_mm_add_pi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_pi32(_mm_slli_pi32(tmp1,2),tmp1);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp1= _mm_add_pi32(_mm_add_pi32(mm0,mm5),tmp1); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp1 = _mm_srai_pi32(_mm_add_pi32(tmp1,coeff_512),10);//(result_block+512)>>10

		mm2 = _mm_packs_pi32(tmp,tmp1);//check
		mm1 = *((__m64*)&block[j][8]);

		mm0 = *((__m64*)&luma_res[j+3][8]);
		mm3 = *((__m64*)&luma_res[j+3][12]);
		tmp = _mm_packs_pu16(mm1,mm2);//__fast_iclip0_255((result_block+512)>>10)
		mm0 = _mm_packs_pu16(mm0,mm3);
		tmp = _mm_avg_pu8(tmp,mm0);
		*((__m64*)&block[j][8]) = tmp;
	}
}
void  get_block_16xh_p23_sse2(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int j;
	DO_ALIGN(16, short luma_res[21][16]);
	DO_ALIGN(16, int tmp_res[21][16]);
	byte *src;
	const int w1 = stride;

	src = pSrc -(2*w1);

	__m128i mm0,mm1,mm2,mm3,mm4,mm5,tmp,tmp1;
	const __m128i Null      = _mm_setzero_si128();
	const __m128i coeff_16  = _mm_load_si128((__m128i*)ncoeff_16);
	const __m128i coeff_512 = _mm_load_si128((__m128i*)ncoeff_512);

	for (j = 0; j < height+5; j++)
	{
		mm0 = _mm_loadl_epi64(((__m128i*) (src-2)));
		mm1 = _mm_loadl_epi64(((__m128i*) (src-1)));
		mm2 = _mm_loadl_epi64(((__m128i*) (src)));
		mm3 = _mm_loadl_epi64(((__m128i*) (src+1)));
		mm4 = _mm_loadl_epi64(((__m128i*) (src+2)));
		mm5 = _mm_loadl_epi64(((__m128i*) (src+3)));

		mm0 = _mm_unpacklo_epi8(mm0, Null);
		mm1 = _mm_unpacklo_epi8(mm1, Null);
		mm2 = _mm_unpacklo_epi8(mm2, Null);
		mm3 = _mm_unpacklo_epi8(mm3, Null);
		mm4 = _mm_unpacklo_epi8(mm4, Null);
		mm5 = _mm_unpacklo_epi8(mm5, Null);

		tmp = _mm_sub_epi16(_mm_slli_epi16(_mm_add_epi16(mm2,mm3),2),_mm_adds_epi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_slli_epi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_add_epi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5

		mm0 = _mm_unpacklo_epi16(Null,tmp);
		mm1 = _mm_unpackhi_epi16(Null,tmp);

		mm0 = _mm_srai_epi32(mm0,16);
		mm1 = _mm_srai_epi32(mm1,16);

		_mm_store_si128(((__m128i*)&tmp_res[j][0]),mm0);
		_mm_store_si128(((__m128i*)&tmp_res[j][4]),mm1);

		tmp = _mm_srai_epi16(_mm_add_epi16(tmp,coeff_16),5);//(result_block+16)>>5
		_mm_store_si128(((__m128i*)&luma_res[j][0]),tmp);

		mm0 = _mm_loadl_epi64(((__m128i*) (src+6)));
		mm1 = _mm_loadl_epi64(((__m128i*) (src+7)));
		mm2 = _mm_loadl_epi64(((__m128i*) (src+8)));
		mm3 = _mm_loadl_epi64(((__m128i*) (src+9)));
		mm4 = _mm_loadl_epi64(((__m128i*) (src+10)));
		mm5 = _mm_loadl_epi64(((__m128i*) (src+11)));

		mm0 = _mm_unpacklo_epi8(mm0, Null);
		mm1 = _mm_unpacklo_epi8(mm1, Null);
		mm2 = _mm_unpacklo_epi8(mm2, Null);
		mm3 = _mm_unpacklo_epi8(mm3, Null);
		mm4 = _mm_unpacklo_epi8(mm4, Null);
		mm5 = _mm_unpacklo_epi8(mm5, Null);

		tmp = _mm_sub_epi16(_mm_slli_epi16(_mm_add_epi16(mm2,mm3),2),_mm_adds_epi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_slli_epi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_add_epi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5

		mm0 = _mm_unpacklo_epi16(Null,tmp);
		mm1 = _mm_unpackhi_epi16(Null,tmp);

		mm0 = _mm_srai_epi32(mm0,16);
		mm1 = _mm_srai_epi32(mm1,16);

		_mm_store_si128(((__m128i*)&tmp_res[j][8]),mm0);
		_mm_store_si128(((__m128i*)&tmp_res[j][12]),mm1);

		tmp = _mm_srai_epi16(_mm_add_epi16(tmp,coeff_16),5);//(result_block+16)>>5
		_mm_store_si128(((__m128i*)&luma_res[j][8]),tmp);

		src += w1;
	}


	for (j = 0; j < height; j++)
	{
		mm0 = _mm_load_si128((__m128i*)&tmp_res[j][0]);
		mm1 = _mm_load_si128((__m128i*)&tmp_res[j+1][0]);
		mm2 = _mm_load_si128((__m128i*)&tmp_res[j+2][0]);
		mm3 = _mm_load_si128((__m128i*)&tmp_res[j+3][0]);
		mm4 = _mm_load_si128((__m128i*)&tmp_res[j+4][0]);
		mm5 = _mm_load_si128((__m128i*)&tmp_res[j+5][0]);

		tmp = _mm_sub_epi32(_mm_slli_epi32(_mm_add_epi32(mm2,mm3),2),_mm_add_epi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi32(_mm_slli_epi32(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi32(_mm_add_epi32(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_epi32(_mm_add_epi32(tmp,coeff_512),10);//(result_block+512)>>10

		mm0 = _mm_load_si128((__m128i*)&tmp_res[j][4]);
		mm1 = _mm_load_si128((__m128i*)&tmp_res[j+1][4]);
		mm2 = _mm_load_si128((__m128i*)&tmp_res[j+2][4]);
		mm3 = _mm_load_si128((__m128i*)&tmp_res[j+3][4]);
		mm4 = _mm_load_si128((__m128i*)&tmp_res[j+4][4]);
		mm5 = _mm_load_si128((__m128i*)&tmp_res[j+5][4]);

		tmp1 = _mm_sub_epi32(_mm_slli_epi32(_mm_add_epi32(mm2,mm3),2),_mm_add_epi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp1= _mm_add_epi32(_mm_slli_epi32(tmp1,2),tmp1);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_epi32(_mm_add_epi32(mm0,mm5),tmp1); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp1 = _mm_srai_epi32(_mm_add_epi32(tmp1,coeff_512),10);//(result_block+512)>>10

		tmp = _mm_packs_epi32(tmp,tmp1);//check

		mm0 = _mm_load_si128((__m128i*)&luma_res[j+3][0]);	//__fast_iclip0_255(((tmp_res[j+2][i]+16)>>5))
		tmp = _mm_packus_epi16(tmp,tmp);//__fast_iclip0_255((result_block+512)>>10)
		mm0 = _mm_packus_epi16(mm0,mm0);
		tmp = _mm_avg_epu8(tmp,mm0);
		_mm_storel_epi64((__m128i*)&block[j][0], tmp);

		mm0 = _mm_load_si128((__m128i*)&tmp_res[j][8]);
		mm1 = _mm_load_si128((__m128i*)&tmp_res[j+1][8]);
		mm2 = _mm_load_si128((__m128i*)&tmp_res[j+2][8]);
		mm3 = _mm_load_si128((__m128i*)&tmp_res[j+3][8]);
		mm4 = _mm_load_si128((__m128i*)&tmp_res[j+4][8]);
		mm5 = _mm_load_si128((__m128i*)&tmp_res[j+5][8]);

		tmp = _mm_sub_epi32(_mm_slli_epi32(_mm_add_epi32(mm2,mm3),2),_mm_add_epi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi32(_mm_slli_epi32(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi32(_mm_add_epi32(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_epi32(_mm_add_epi32(tmp,coeff_512),10);//(result_block+512)>>10

		mm0 = _mm_load_si128((__m128i*)&tmp_res[j][12]);
		mm1 = _mm_load_si128((__m128i*)&tmp_res[j+1][12]);
		mm2 = _mm_load_si128((__m128i*)&tmp_res[j+2][12]);
		mm3 = _mm_load_si128((__m128i*)&tmp_res[j+3][12]);
		mm4 = _mm_load_si128((__m128i*)&tmp_res[j+4][12]);
		mm5 = _mm_load_si128((__m128i*)&tmp_res[j+5][12]);

		tmp1 = _mm_sub_epi32(_mm_slli_epi32(_mm_add_epi32(mm2,mm3),2),_mm_add_epi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp1= _mm_add_epi32(_mm_slli_epi32(tmp1,2),tmp1);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_epi32(_mm_add_epi32(mm0,mm5),tmp1); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp1 = _mm_srai_epi32(_mm_add_epi32(tmp1,coeff_512),10);//(result_block+512)>>10

		tmp = _mm_packs_epi32(tmp,tmp1);//check

		mm0 = _mm_load_si128((__m128i*)&luma_res[j+3][8]);	//__fast_iclip0_255(((tmp_res[j+2][i]+16)>>5))
		tmp = _mm_packus_epi16(tmp,tmp);//__fast_iclip0_255((result_block+512)>>10)
		mm0 = _mm_packus_epi16(mm0,mm0);
		tmp = _mm_avg_epu8(tmp,mm0);

		_mm_storel_epi64((__m128i*)&block[j][8], tmp);
	}
}
void  get_block_16xh_p30_sse(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int j;
	byte *src;
	const int w1 = stride;

	src = pSrc;

	__m64 mm0, mm1, mm2, mm3, mm4, mm5, tmp;
	const __m64 Null     = _mm_setzero_si64();
	const __m64 coeff_16 = *((__m64*)ncoeff_16);

	for (j = 0; j < height; j++)
	{
		mm0 = *((__m64*)(src-2));
		mm1 = *((__m64*)(src-1));
		mm2 = *((__m64*)(src));
		mm3 = *((__m64*)(src+1));
		mm4 = *((__m64*)(src+2));
		mm5 = *((__m64*)(src+3));

		mm0 = _mm_unpacklo_pi8(mm0, Null);
		mm1 = _mm_unpacklo_pi8(mm1, Null);
		mm2 = _mm_unpacklo_pi8(mm2, Null);
		mm3 = _mm_unpacklo_pi8(mm3, Null);
		mm4 = _mm_unpacklo_pi8(mm4, Null);
		mm5 = _mm_unpacklo_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		mm3 = _mm_packs_pu16(mm3,mm3);
		tmp = _mm_avg_pu8(tmp,mm3);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		*((DWORD*)&block[j][0]) = _m_to_int(tmp);

		mm0 = *((__m64*)(src-2));
		mm1 = *((__m64*)(src-1));
		mm2 = *((__m64*)(src));
		mm3 = *((__m64*)(src+1));
		mm4 = *((__m64*)(src+2));
		mm5 = *((__m64*)(src+3));

		mm0 = _mm_unpackhi_pi8(mm0, Null);
		mm1 = _mm_unpackhi_pi8(mm1, Null);
		mm2 = _mm_unpackhi_pi8(mm2, Null);
		mm3 = _mm_unpackhi_pi8(mm3, Null);
		mm4 = _mm_unpackhi_pi8(mm4, Null);
		mm5 = _mm_unpackhi_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		mm3 = _mm_packs_pu16(mm3,mm3);
		tmp = _mm_avg_pu8(tmp,mm3);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		*((DWORD*)&block[j][4]) = _m_to_int(tmp);

		mm0 = *((__m64*)(src+6));
		mm1 = *((__m64*)(src+7));
		mm2 = *((__m64*)(src+8));
		mm3 = *((__m64*)(src+9));
		mm4 = *((__m64*)(src+10));
		mm5 = *((__m64*)(src+11));

		mm0 = _mm_unpacklo_pi8(mm0, Null);
		mm1 = _mm_unpacklo_pi8(mm1, Null);
		mm2 = _mm_unpacklo_pi8(mm2, Null);
		mm3 = _mm_unpacklo_pi8(mm3, Null);
		mm4 = _mm_unpacklo_pi8(mm4, Null);
		mm5 = _mm_unpacklo_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		mm3 = _mm_packs_pu16(mm3,mm3);
		tmp = _mm_avg_pu8(tmp,mm3);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		*((DWORD*)&block[j][8]) = _m_to_int(tmp);

		mm0 = *((__m64*)(src+6));
		mm1 = *((__m64*)(src+7));
		mm2 = *((__m64*)(src+8));
		mm3 = *((__m64*)(src+9));
		mm4 = *((__m64*)(src+10));
		mm5 = *((__m64*)(src+11));

		mm0 = _mm_unpackhi_pi8(mm0, Null);
		mm1 = _mm_unpackhi_pi8(mm1, Null);
		mm2 = _mm_unpackhi_pi8(mm2, Null);
		mm3 = _mm_unpackhi_pi8(mm3, Null);
		mm4 = _mm_unpackhi_pi8(mm4, Null);
		mm5 = _mm_unpackhi_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		mm3 = _mm_packs_pu16(mm3,mm3);
		tmp = _mm_avg_pu8(tmp,mm3);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		*((DWORD*)&block[j][12]) = _m_to_int(tmp);

		src += w1;
	}
}
void  get_block_16xh_p30_sse2(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int j;
	byte *src;
	const int w1 = stride;

	src = pSrc;

	__m128i mm0,mm1,mm2,mm3,mm4,mm5,tmp;
	const __m128i Null     = _mm_setzero_si128();
	const __m128i coeff_16 = _mm_load_si128((__m128i*)ncoeff_16);

	for (j = 0; j < height; j++)
	{
		mm0 = _mm_loadl_epi64(((__m128i*) (src-2)));
		mm1 = _mm_loadl_epi64(((__m128i*) (src-1)));
		mm2 = _mm_loadl_epi64(((__m128i*) (src)));
		mm3 = _mm_loadl_epi64(((__m128i*) (src+1)));
		mm4 = _mm_loadl_epi64(((__m128i*) (src+2)));
		mm5 = _mm_loadl_epi64(((__m128i*) (src+3)));

		mm0 = _mm_unpacklo_epi8(mm0, Null);
		mm1 = _mm_unpacklo_epi8(mm1, Null);
		mm2 = _mm_unpacklo_epi8(mm2, Null);
		mm3 = _mm_unpacklo_epi8(mm3, Null);
		mm4 = _mm_unpacklo_epi8(mm4, Null);
		mm5 = _mm_unpacklo_epi8(mm5, Null);

		tmp = _mm_sub_epi16(_mm_slli_epi16(_mm_add_epi16(mm2,mm3),2),_mm_add_epi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_slli_epi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_add_epi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_epi16(_mm_add_epi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packus_epi16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		mm3 = _mm_packus_epi16(mm3,mm3);
		tmp = _mm_avg_epu8(tmp,mm3);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		_mm_storel_epi64((__m128i*)&block[j][0], tmp);

		mm0 = _mm_loadl_epi64(((__m128i*) (src+6)));
		mm1 = _mm_loadl_epi64(((__m128i*) (src+7)));
		mm2 = _mm_loadl_epi64(((__m128i*) (src+8)));
		mm3 = _mm_loadl_epi64(((__m128i*) (src+9)));
		mm4 = _mm_loadl_epi64(((__m128i*) (src+10)));
		mm5 = _mm_loadl_epi64(((__m128i*) (src+11)));

		mm0 = _mm_unpacklo_epi8(mm0, Null);
		mm1 = _mm_unpacklo_epi8(mm1, Null);
		mm2 = _mm_unpacklo_epi8(mm2, Null);
		mm3 = _mm_unpacklo_epi8(mm3, Null);
		mm4 = _mm_unpacklo_epi8(mm4, Null);
		mm5 = _mm_unpacklo_epi8(mm5, Null);

		tmp = _mm_sub_epi16(_mm_slli_epi16(_mm_add_epi16(mm2,mm3),2),_mm_add_epi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_slli_epi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_add_epi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_epi16(_mm_add_epi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packus_epi16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		mm3 = _mm_packus_epi16(mm3,mm3);
		tmp = _mm_avg_epu8(tmp,mm3);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		_mm_storel_epi64((__m128i*)&block[j][8], tmp);

		src += w1;
	}
}

void  get_block_16xh_p31_sse(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int j;
	byte *src;
	const int w1 = stride;
	const int w2 = stride + stride;
	const int w3 = w2 + w1;

	src = pSrc;

	__m64 mm0, mm1, mm2, mm3, mm4, mm5, mm6, tmp;
	const __m64 Null     = _mm_setzero_si64();
	const __m64 coeff_16 = *((__m64*)ncoeff_16);

	for (j = 0; j < height; j++)
	{
		mm0 = *((__m64*)(src-2));
		mm1 = *((__m64*)(src-1));
		mm2 = *((__m64*)(src));
		mm3 = *((__m64*)(src+1));
		mm4 = *((__m64*)(src+2));
		mm5 = *((__m64*)(src+3));

		mm0 = _mm_unpacklo_pi8(mm0, Null);
		mm1 = _mm_unpacklo_pi8(mm1, Null);
		mm2 = _mm_unpacklo_pi8(mm2, Null);
		mm3 = _mm_unpacklo_pi8(mm3, Null);
		mm4 = _mm_unpacklo_pi8(mm4, Null);
		mm5 = _mm_unpacklo_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		*((DWORD*)&block[j][0]) = _m_to_int(tmp);
		
		mm0 = *((__m64*)(src-2));
		mm1 = *((__m64*)(src-1));
		mm2 = *((__m64*)(src));
		mm3 = *((__m64*)(src+1));
		mm4 = *((__m64*)(src+2));
		mm5 = *((__m64*)(src+3));

		mm0 = _mm_unpackhi_pi8(mm0, Null);
		mm1 = _mm_unpackhi_pi8(mm1, Null);
		mm2 = _mm_unpackhi_pi8(mm2, Null);
		mm3 = _mm_unpackhi_pi8(mm3, Null);
		mm4 = _mm_unpackhi_pi8(mm4, Null);
		mm5 = _mm_unpackhi_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		*((DWORD*)&block[j][4]) = _m_to_int(tmp);

		
		mm0 = *((__m64*)(src+6));
		mm1 = *((__m64*)(src+7));
		mm2 = *((__m64*)(src+8));
		mm3 = *((__m64*)(src+9));
		mm4 = *((__m64*)(src+10));
		mm5 = *((__m64*)(src+11));

		mm0 = _mm_unpacklo_pi8(mm0, Null);
		mm1 = _mm_unpacklo_pi8(mm1, Null);
		mm2 = _mm_unpacklo_pi8(mm2, Null);
		mm3 = _mm_unpacklo_pi8(mm3, Null);
		mm4 = _mm_unpacklo_pi8(mm4, Null);
		mm5 = _mm_unpacklo_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		*((DWORD*)&block[j][8]) = _m_to_int(tmp);

		mm0 = *((__m64*)(src+6));
		mm1 = *((__m64*)(src+7));
		mm2 = *((__m64*)(src+8));
		mm3 = *((__m64*)(src+9));
		mm4 = *((__m64*)(src+10));
		mm5 = *((__m64*)(src+11));

		mm0 = _mm_unpackhi_pi8(mm0, Null);
		mm1 = _mm_unpackhi_pi8(mm1, Null);
		mm2 = _mm_unpackhi_pi8(mm2, Null);
		mm3 = _mm_unpackhi_pi8(mm3, Null);
		mm4 = _mm_unpackhi_pi8(mm4, Null);
		mm5 = _mm_unpackhi_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		*((DWORD*)&block[j][12]) = _m_to_int(tmp);


		src += w1;
	}

	src = pSrc + 1;

	for (j = 0; j < height; j++)
	{
		mm0 = *((__m64*)(src-w2));
		mm1 = *((__m64*)(src-w1));
		mm2 = *((__m64*)(src));
		mm3 = *((__m64*)(src+w1));
		mm4 = *((__m64*)(src+w2));
		mm5 = *((__m64*)(src+w3));
		mm6 = *((__m64*)&block[j][0]);

		mm0 = _mm_unpacklo_pi8(mm0, Null);
		mm1 = _mm_unpacklo_pi8(mm1, Null);
		mm2 = _mm_unpacklo_pi8(mm2, Null);
		mm3 = _mm_unpacklo_pi8(mm3, Null);
		mm4 = _mm_unpacklo_pi8(mm4, Null);
		mm5 = _mm_unpacklo_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		tmp = _mm_avg_pu8(tmp,mm6);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		*((DWORD*)&block[j][0]) = _m_to_int(tmp);

		mm0 = *((__m64*)(src-w2));
		mm1 = *((__m64*)(src-w1));
		mm2 = *((__m64*)(src));
		mm3 = *((__m64*)(src+w1));
		mm4 = *((__m64*)(src+w2));
		mm5 = *((__m64*)(src+w3));
		mm6 = *((__m64*)&block[j][4]);

		mm0 = _mm_unpackhi_pi8(mm0, Null);
		mm1 = _mm_unpackhi_pi8(mm1, Null);
		mm2 = _mm_unpackhi_pi8(mm2, Null);
		mm3 = _mm_unpackhi_pi8(mm3, Null);
		mm4 = _mm_unpackhi_pi8(mm4, Null);
		mm5 = _mm_unpackhi_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		tmp = _mm_avg_pu8(tmp,mm6);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		*((DWORD*)&block[j][4]) = _m_to_int(tmp);

		
	  mm0 = *((__m64*)(src-w2+8));
		mm1 = *((__m64*)(src-w1+8));
		mm2 = *((__m64*)(src+8));
		mm3 = *((__m64*)(src+w1+8));
		mm4 = *((__m64*)(src+w2+8));
		mm5 = *((__m64*)(src+w3+8));
		mm6 = *((__m64*)&block[j][8]);

		mm0 = _mm_unpacklo_pi8(mm0, Null);
		mm1 = _mm_unpacklo_pi8(mm1, Null);
		mm2 = _mm_unpacklo_pi8(mm2, Null);
		mm3 = _mm_unpacklo_pi8(mm3, Null);
		mm4 = _mm_unpacklo_pi8(mm4, Null);
		mm5 = _mm_unpacklo_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		tmp = _mm_avg_pu8(tmp,mm6);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		*((DWORD*)&block[j][8]) = _m_to_int(tmp);

		mm0 = *((__m64*)(src-w2+8));
		mm1 = *((__m64*)(src-w1+8));
		mm2 = *((__m64*)(src+8));
		mm3 = *((__m64*)(src+w1+8));
		mm4 = *((__m64*)(src+w2+8));
		mm5 = *((__m64*)(src+w3+8));
		mm6 = *((__m64*)&block[j][12]);

		mm0 = _mm_unpackhi_pi8(mm0, Null);
		mm1 = _mm_unpackhi_pi8(mm1, Null);
		mm2 = _mm_unpackhi_pi8(mm2, Null);
		mm3 = _mm_unpackhi_pi8(mm3, Null);
		mm4 = _mm_unpackhi_pi8(mm4, Null);
		mm5 = _mm_unpackhi_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		tmp = _mm_avg_pu8(tmp,mm6);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		*((DWORD*)&block[j][12]) = _m_to_int(tmp);

		src += w1;
	}
}
void  get_block_16xh_p31_sse2(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int j;
	byte *src;
	const int w1 = stride;
	const int w2 = stride + stride;
	const int w3 = w2 + w1;

	src = pSrc;

	__m128i mm0,mm1,mm2,mm3,mm4,mm5,mm6,tmp;
	const __m128i Null     = _mm_setzero_si128();
	const __m128i coeff_16 = _mm_load_si128((__m128i*)ncoeff_16);

	for (j = 0; j < height; j++)
	{
		mm0 = _mm_loadl_epi64(((__m128i*) (src-2)));
		mm1 = _mm_loadl_epi64(((__m128i*) (src-1)));
		mm2 = _mm_loadl_epi64(((__m128i*) (src)));
		mm3 = _mm_loadl_epi64(((__m128i*) (src+1)));
		mm4 = _mm_loadl_epi64(((__m128i*) (src+2)));
		mm5 = _mm_loadl_epi64(((__m128i*) (src+3)));

		mm0 = _mm_unpacklo_epi8(mm0, Null);
		mm1 = _mm_unpacklo_epi8(mm1, Null);
		mm2 = _mm_unpacklo_epi8(mm2, Null);
		mm3 = _mm_unpacklo_epi8(mm3, Null);
		mm4 = _mm_unpacklo_epi8(mm4, Null);
		mm5 = _mm_unpacklo_epi8(mm5, Null);

		tmp = _mm_sub_epi16(_mm_slli_epi16(_mm_add_epi16(mm2,mm3),2),_mm_add_epi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_slli_epi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_add_epi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_epi16(_mm_add_epi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packus_epi16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		_mm_storel_epi64((__m128i*)&block[j][0], tmp);

		mm0 = _mm_loadl_epi64(((__m128i*) (src+6)));
		mm1 = _mm_loadl_epi64(((__m128i*) (src+7)));
		mm2 = _mm_loadl_epi64(((__m128i*) (src+8)));
		mm3 = _mm_loadl_epi64(((__m128i*) (src+9)));
		mm4 = _mm_loadl_epi64(((__m128i*) (src+10)));
		mm5 = _mm_loadl_epi64(((__m128i*) (src+11)));

		mm0 = _mm_unpacklo_epi8(mm0, Null);
		mm1 = _mm_unpacklo_epi8(mm1, Null);
		mm2 = _mm_unpacklo_epi8(mm2, Null);
		mm3 = _mm_unpacklo_epi8(mm3, Null);
		mm4 = _mm_unpacklo_epi8(mm4, Null);
		mm5 = _mm_unpacklo_epi8(mm5, Null);

		tmp = _mm_sub_epi16(_mm_slli_epi16(_mm_add_epi16(mm2,mm3),2),_mm_add_epi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_slli_epi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_add_epi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_epi16(_mm_add_epi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packus_epi16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		_mm_storel_epi64((__m128i*)&block[j][8], tmp);

		src += w1;
	}

	src = pSrc + 1;

	for (j = 0; j < height; j++)
	{
		mm0 = _mm_loadl_epi64(((__m128i*) (src-w2)));
		mm1 = _mm_loadl_epi64(((__m128i*) (src-w1)));
		mm2 = _mm_loadl_epi64(((__m128i*) (src)));
		mm3 = _mm_loadl_epi64(((__m128i*) (src+w1)));
		mm4 = _mm_loadl_epi64(((__m128i*) (src+w2)));
		mm5 = _mm_loadl_epi64(((__m128i*) (src+w3)));
		mm6 = _mm_loadl_epi64(((__m128i*)&block[j][0]));

		mm0 = _mm_unpacklo_epi8(mm0, Null);
		mm1 = _mm_unpacklo_epi8(mm1, Null);
		mm2 = _mm_unpacklo_epi8(mm2, Null);
		mm3 = _mm_unpacklo_epi8(mm3, Null);
		mm4 = _mm_unpacklo_epi8(mm4, Null);
		mm5 = _mm_unpacklo_epi8(mm5, Null);

		tmp = _mm_sub_epi16(_mm_slli_epi16(_mm_add_epi16(mm2,mm3),2),_mm_add_epi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_slli_epi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_add_epi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_epi16(_mm_add_epi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packus_epi16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		tmp = _mm_avg_epu8(tmp,mm6);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		_mm_storel_epi64((__m128i*)&block[j][0], tmp);

		mm0 = _mm_loadl_epi64(((__m128i*) (src-w2+8)));
		mm1 = _mm_loadl_epi64(((__m128i*) (src-w1+8)));
		mm2 = _mm_loadl_epi64(((__m128i*) (src+8)));
		mm3 = _mm_loadl_epi64(((__m128i*) (src+w1+8)));
		mm4 = _mm_loadl_epi64(((__m128i*) (src+w2+8)));
		mm5 = _mm_loadl_epi64(((__m128i*) (src+w3+8)));
		mm6 = _mm_loadl_epi64(((__m128i*)&block[j][8]));

		mm0 = _mm_unpacklo_epi8(mm0, Null);
		mm1 = _mm_unpacklo_epi8(mm1, Null);
		mm2 = _mm_unpacklo_epi8(mm2, Null);
		mm3 = _mm_unpacklo_epi8(mm3, Null);
		mm4 = _mm_unpacklo_epi8(mm4, Null);
		mm5 = _mm_unpacklo_epi8(mm5, Null);

		tmp = _mm_sub_epi16(_mm_slli_epi16(_mm_add_epi16(mm2,mm3),2),_mm_add_epi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_slli_epi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_add_epi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_epi16(_mm_add_epi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packus_epi16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		tmp = _mm_avg_epu8(tmp,mm6);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		_mm_storel_epi64((__m128i*)&block[j][8], tmp);

		src += w1;
	}
}

void  get_block_16xh_p32_sse(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	byte *src;
	short *tmp_src;
	short *luma_src;
	int i,j;
	DO_ALIGN(16, short luma_res[16][24]);
	DO_ALIGN(16, short tmp_res[16][24]);
	const int w1 = stride;
	const int w2 = stride + stride;
	const int w3 = w2 + w1;
	src = pSrc;

	__m64 pIn0, pIn1;
	__m64 mm0, mm1, mm2, mm3, mm4, mm5, tmp, tmp1;
	const __m64 Null     = _mm_setzero_si64();
	const __m64 coeff_16 = *((__m64*)ncoeff_16);
	const __m64 coeff_512 = *((__m64*)ncoeff_512);

	for (j = 0; j < height; j++)
	{
		for(i = 0; i < 20; i += 8)
		{
			mm0 = *((__m64*)(src-w2-2+i));
			mm1 = *((__m64*)(src-w1-2+i));
			mm2 = *((__m64*)(src-2+i));
			mm3 = *((__m64*)(src+w1-2+i));
			mm4 = *((__m64*)(src+w2-2+i));
			mm5 = *((__m64*)(src+w3-2+i));

			mm0 = _mm_unpacklo_pi8(mm0, Null);
			mm1 = _mm_unpacklo_pi8(mm1, Null);
			mm2 = _mm_unpacklo_pi8(mm2, Null);
			mm3 = _mm_unpacklo_pi8(mm3, Null);
			mm4 = _mm_unpacklo_pi8(mm4, Null);
			mm5 = _mm_unpacklo_pi8(mm5, Null);

			tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_adds_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
			tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
			tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5

			tmp1 = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
			*((__m64*)&tmp_res[j][i]) = tmp;
			*((__m64*)&luma_res[j][i]) = tmp1;
			
			mm0 = *((__m64*)(src-w2-2+i));
			mm1 = *((__m64*)(src-w1-2+i));
			mm2 = *((__m64*)(src-2+i));
			mm3 = *((__m64*)(src+w1-2+i));
			mm4 = *((__m64*)(src+w2-2+i));
			mm5 = *((__m64*)(src+w3-2+i));

			mm0 = _mm_unpackhi_pi8(mm0, Null);
			mm1 = _mm_unpackhi_pi8(mm1, Null);
			mm2 = _mm_unpackhi_pi8(mm2, Null);
			mm3 = _mm_unpackhi_pi8(mm3, Null);
			mm4 = _mm_unpackhi_pi8(mm4, Null);
			mm5 = _mm_unpackhi_pi8(mm5, Null);

			tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_adds_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
			tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
			tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5

			tmp1 = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
			*((__m64*)&tmp_res[j][i+4]) = tmp;
			*((__m64*)&luma_res[j][i+4]) = tmp1;


		}
		src += w1;
	}

	tmp_src = &tmp_res[0][0];
	luma_src = &luma_res[0][3];
	for (j = 0; j < height; j++)
	{
		mm0 = *((__m64*)(tmp_src));
		mm1 = *((__m64*)(tmp_src+1));
		mm2 = *((__m64*)(tmp_src+2));
		mm3 = *((__m64*)(tmp_src+3));
		mm4 = *((__m64*)(tmp_src+4));
		mm5 = *((__m64*)(tmp_src+5));

		mm0 = _mm_unpacklo_pi16(Null, mm0);
		mm1 = _mm_unpacklo_pi16(Null, mm1);
		mm2 = _mm_unpacklo_pi16(Null, mm2);
		mm3 = _mm_unpacklo_pi16(Null, mm3);
		mm4 = _mm_unpacklo_pi16(Null, mm4);
		mm5 = _mm_unpacklo_pi16(Null, mm5);

		mm0 = _mm_srai_pi32(mm0,16);
		mm1 = _mm_srai_pi32(mm1,16);
		mm2 = _mm_srai_pi32(mm2,16);
		mm3 = _mm_srai_pi32(mm3,16);
		mm4 = _mm_srai_pi32(mm4,16);
		mm5 = _mm_srai_pi32(mm5,16);

		tmp = _mm_sub_pi32(_mm_slli_pi32(_mm_add_pi32(mm2,mm3),2),_mm_add_pi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi32(_mm_slli_pi32(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi32(_mm_add_pi32(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi32(_mm_add_pi32(tmp,coeff_512),10);//(result_block+512)>>10

		mm0 = mm4;
		mm1 = mm5;
		mm2 = *((__m64*)(tmp_src+6));
		mm3 = *((__m64*)(tmp_src+7));
		mm4 = *((__m64*)(tmp_src+8));
		mm5 = *((__m64*)(tmp_src+9));

		mm2 = _mm_unpacklo_pi16(Null, mm2);
		mm3 = _mm_unpacklo_pi16(Null, mm3);
		mm4 = _mm_unpacklo_pi16(Null, mm4);
		mm5 = _mm_unpacklo_pi16(Null, mm5);

		mm2 = _mm_srai_pi32(mm2,16);
		mm3 = _mm_srai_pi32(mm3,16);
		mm4 = _mm_srai_pi32(mm4,16);
		mm5 = _mm_srai_pi32(mm5,16);

		tmp1 = _mm_sub_pi32(_mm_slli_pi32(_mm_add_pi32(mm2,mm3),2),_mm_add_pi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_pi32(_mm_slli_pi32(tmp1,2),tmp1);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_pi32(_mm_add_pi32(mm0,mm5),tmp1); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp1 = _mm_srai_pi32(_mm_add_pi32(tmp1,coeff_512),10);//(result_block+512)>>10

		tmp = _mm_packs_pi32(tmp,tmp1);
		*((__m64*)&block[j][0]) = tmp;

		mm0 = *((__m64*)(tmp_src));
		mm1 = *((__m64*)(tmp_src+1));
		mm2 = *((__m64*)(tmp_src+2));
		mm3 = *((__m64*)(tmp_src+3));
		mm4 = *((__m64*)(tmp_src+4));
		mm5 = *((__m64*)(tmp_src+5));

		mm0 = _mm_unpackhi_pi16(Null, mm0);
		mm1 = _mm_unpackhi_pi16(Null, mm1);
		mm2 = _mm_unpackhi_pi16(Null, mm2);
		mm3 = _mm_unpackhi_pi16(Null, mm3);
		mm4 = _mm_unpackhi_pi16(Null, mm4);
		mm5 = _mm_unpackhi_pi16(Null, mm5);

		mm0 = _mm_srai_pi32(mm0,16);
		mm1 = _mm_srai_pi32(mm1,16);
		mm2 = _mm_srai_pi32(mm2,16);
		mm3 = _mm_srai_pi32(mm3,16);
		mm4 = _mm_srai_pi32(mm4,16);
		mm5 = _mm_srai_pi32(mm5,16);

		tmp = _mm_sub_pi32(_mm_slli_pi32(_mm_add_pi32(mm2,mm3),2),_mm_add_pi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi32(_mm_slli_pi32(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi32(_mm_add_pi32(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi32(_mm_add_pi32(tmp,coeff_512),10);//(result_block+512)>>10

		mm0 = mm4;
		mm1 = mm5;
		mm2 = *((__m64*)(tmp_src+6));
		mm3 = *((__m64*)(tmp_src+7));
		mm4 = *((__m64*)(tmp_src+8));
		mm5 = *((__m64*)(tmp_src+9));

		mm2 = _mm_unpackhi_pi16(Null, mm2);
		mm3 = _mm_unpackhi_pi16(Null, mm3);
		mm4 = _mm_unpackhi_pi16(Null, mm4);
		mm5 = _mm_unpackhi_pi16(Null, mm5);

		mm2 = _mm_srai_pi32(mm2,16);
		mm3 = _mm_srai_pi32(mm3,16);
		mm4 = _mm_srai_pi32(mm4,16);
		mm5 = _mm_srai_pi32(mm5,16);

		tmp1 = _mm_sub_pi32(_mm_slli_pi32(_mm_add_pi32(mm2,mm3),2),_mm_add_pi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_pi32(_mm_slli_pi32(tmp1,2),tmp1);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_pi32(_mm_add_pi32(mm0,mm5),tmp1); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp1 = _mm_srai_pi32(_mm_add_pi32(tmp1,coeff_512),10);//(result_block+512)>>10

		tmp = _mm_packs_pi32(tmp,tmp1);
		tmp1 = *((__m64*)&block[j][0]);
		mm4 = _mm_unpacklo_pi32(tmp1, tmp);
		mm5 = _mm_unpackhi_pi32(tmp1, tmp);

		pIn0 = *((__m64*) (luma_src));
		pIn1 = *((__m64*) (luma_src+4));
		tmp = _mm_packs_pu16(mm4,mm5);//__fast_iclip0_255((result_block+512)>>10)
		mm0 = _mm_packs_pu16(pIn0,pIn1);
		tmp = _mm_avg_pu8(tmp,mm0);
		*((__m64*)&block[j][0]) = tmp;

		mm0 = *((__m64*)(tmp_src+8));
		mm1 = *((__m64*)(tmp_src+9));
		mm2 = *((__m64*)(tmp_src+10));
		mm3 = *((__m64*)(tmp_src+11));
		mm4 = *((__m64*)(tmp_src+12));
		mm5 = *((__m64*)(tmp_src+13));

		mm0 = _mm_unpacklo_pi16(Null, mm0);
		mm1 = _mm_unpacklo_pi16(Null, mm1);
		mm2 = _mm_unpacklo_pi16(Null, mm2);
		mm3 = _mm_unpacklo_pi16(Null, mm3);
		mm4 = _mm_unpacklo_pi16(Null, mm4);
		mm5 = _mm_unpacklo_pi16(Null, mm5);

		mm0 = _mm_srai_pi32(mm0,16);
		mm1 = _mm_srai_pi32(mm1,16);
		mm2 = _mm_srai_pi32(mm2,16);
		mm3 = _mm_srai_pi32(mm3,16);
		mm4 = _mm_srai_pi32(mm4,16);
		mm5 = _mm_srai_pi32(mm5,16);

		tmp = _mm_sub_pi32(_mm_slli_pi32(_mm_add_pi32(mm2,mm3),2),_mm_add_pi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi32(_mm_slli_pi32(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi32(_mm_add_pi32(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi32(_mm_add_pi32(tmp,coeff_512),10);//(result_block+512)>>10

		mm0 = mm4;
		mm1 = mm5;
		mm2 = *((__m64*)(tmp_src+14));
		mm3 = *((__m64*)(tmp_src+15));
		mm4 = *((__m64*)(tmp_src+16));
		mm5 = *((__m64*)(tmp_src+17));

		mm2 = _mm_unpacklo_pi16(Null, mm2);
		mm3 = _mm_unpacklo_pi16(Null, mm3);
		mm4 = _mm_unpacklo_pi16(Null, mm4);
		mm5 = _mm_unpacklo_pi16(Null, mm5);

		mm2 = _mm_srai_pi32(mm2,16);
		mm3 = _mm_srai_pi32(mm3,16);
		mm4 = _mm_srai_pi32(mm4,16);
		mm5 = _mm_srai_pi32(mm5,16);

		tmp1 = _mm_sub_pi32(_mm_slli_pi32(_mm_add_pi32(mm2,mm3),2),_mm_add_pi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_pi32(_mm_slli_pi32(tmp1,2),tmp1);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_pi32(_mm_add_pi32(mm0,mm5),tmp1); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp1 = _mm_srai_pi32(_mm_add_pi32(tmp1,coeff_512),10);//(result_block+512)>>10

		tmp = _mm_packs_pi32(tmp,tmp1);
		*((__m64*)&block[j][8]) = tmp;

		mm0 = *((__m64*)(tmp_src+8));
		mm1 = *((__m64*)(tmp_src+9));
		mm2 = *((__m64*)(tmp_src+10));
		mm3 = *((__m64*)(tmp_src+11));
		mm4 = *((__m64*)(tmp_src+12));
		mm5 = *((__m64*)(tmp_src+13));

		mm0 = _mm_unpackhi_pi16(Null, mm0);
		mm1 = _mm_unpackhi_pi16(Null, mm1);
		mm2 = _mm_unpackhi_pi16(Null, mm2);
		mm3 = _mm_unpackhi_pi16(Null, mm3);
		mm4 = _mm_unpackhi_pi16(Null, mm4);
		mm5 = _mm_unpackhi_pi16(Null, mm5);

		mm0 = _mm_srai_pi32(mm0,16);
		mm1 = _mm_srai_pi32(mm1,16);
		mm2 = _mm_srai_pi32(mm2,16);
		mm3 = _mm_srai_pi32(mm3,16);
		mm4 = _mm_srai_pi32(mm4,16);
		mm5 = _mm_srai_pi32(mm5,16);

		tmp = _mm_sub_pi32(_mm_slli_pi32(_mm_add_pi32(mm2,mm3),2),_mm_add_pi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi32(_mm_slli_pi32(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi32(_mm_add_pi32(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi32(_mm_add_pi32(tmp,coeff_512),10);//(result_block+512)>>10

		mm0 = mm4;
		mm1 = mm5;
		mm2 = *((__m64*)(tmp_src+14));
		mm3 = *((__m64*)(tmp_src+15));
		mm4 = *((__m64*)(tmp_src+16));
		mm5 = *((__m64*)(tmp_src+17));

		mm2 = _mm_unpackhi_pi16(Null, mm2);
		mm3 = _mm_unpackhi_pi16(Null, mm3);
		mm4 = _mm_unpackhi_pi16(Null, mm4);
		mm5 = _mm_unpackhi_pi16(Null, mm5);

		mm2 = _mm_srai_pi32(mm2,16);
		mm3 = _mm_srai_pi32(mm3,16);
		mm4 = _mm_srai_pi32(mm4,16);
		mm5 = _mm_srai_pi32(mm5,16);

		tmp1 = _mm_sub_pi32(_mm_slli_pi32(_mm_add_pi32(mm2,mm3),2),_mm_add_pi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_pi32(_mm_slli_pi32(tmp1,2),tmp1);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_pi32(_mm_add_pi32(mm0,mm5),tmp1); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp1 = _mm_srai_pi32(_mm_add_pi32(tmp1,coeff_512),10);//(result_block+512)>>10

		tmp = _mm_packs_pi32(tmp,tmp1);
		tmp1 = *((__m64*)&block[j][8]);
		mm4 = _mm_unpacklo_pi32(tmp1, tmp);
		mm5 = _mm_unpackhi_pi32(tmp1, tmp);


		pIn0 = *((__m64*) (luma_src+8));
		pIn1 = *((__m64*) (luma_src+12));
		tmp = _mm_packs_pu16(mm4,mm5);//__fast_iclip0_255((result_block+512)>>10)
		mm0 = _mm_packs_pu16(pIn0,pIn1);
		tmp = _mm_avg_pu8(tmp,mm0);
		*((__m64*)&block[j][8]) = tmp;


		tmp_src += 24;
		luma_src += 24;
	}
}
void  get_block_16xh_p32_sse2(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	byte *src;
	short *tmp_src;
	short *luma_src;
	int i,j;
	DO_ALIGN(16, short luma_res[16][24]);
	DO_ALIGN(16, short tmp_res[16][24]);
	const int w1 = stride;
	const int w2 = stride + stride;
	const int w3 = w2 + w1;
	src = pSrc;

	__m64 pIn0, pIn1;
	__m128i mm0,mm1,mm2,mm3,mm4,mm5,tmp,tmp1;
	const __m128i Null      = _mm_setzero_si128();
	const __m128i coeff_16  = _mm_load_si128((__m128i*)ncoeff_16);
	const __m128i coeff_512 = _mm_load_si128((__m128i*)ncoeff_512);

	for (j = 0; j < height; j++)
	{
		for(i = 0; i < 20; i += 8)
		{
			mm0 = _mm_loadl_epi64(((__m128i*) (src-w2-2+i)));
			mm1 = _mm_loadl_epi64(((__m128i*) (src-w1-2+i)));
			mm2 = _mm_loadl_epi64(((__m128i*) (src-2+i)));
			mm3 = _mm_loadl_epi64(((__m128i*) (src+w1-2+i)));
			mm4 = _mm_loadl_epi64(((__m128i*) (src+w2-2+i)));
			mm5 = _mm_loadl_epi64(((__m128i*) (src+w3-2+i)));

			mm0 = _mm_unpacklo_epi8(mm0, Null);
			mm1 = _mm_unpacklo_epi8(mm1, Null);
			mm2 = _mm_unpacklo_epi8(mm2, Null);
			mm3 = _mm_unpacklo_epi8(mm3, Null);
			mm4 = _mm_unpacklo_epi8(mm4, Null);
			mm5 = _mm_unpacklo_epi8(mm5, Null);

			tmp = _mm_sub_epi16(_mm_slli_epi16(_mm_add_epi16(mm2,mm3),2),_mm_adds_epi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
			tmp = _mm_add_epi16(_mm_slli_epi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
			tmp = _mm_add_epi16(_mm_add_epi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5

			_mm_store_si128(((__m128i*)&tmp_res[j][i]),tmp);

			tmp = _mm_srai_epi16(_mm_add_epi16(tmp,coeff_16),5);//(result_block+16)>>5

			_mm_store_si128(((__m128i*)&luma_res[j][i]),tmp);

		}
		src += w1;
	}

	tmp_src = &tmp_res[0][0];
	luma_src = &luma_res[0][3];
	for (j = 0; j < height; j++)
	{
		mm0 = _mm_loadl_epi64(((__m128i*) (tmp_src)));
		mm1 = _mm_loadl_epi64(((__m128i*) (tmp_src+1)));
		mm2 = _mm_loadl_epi64(((__m128i*) (tmp_src+2)));
		mm3 = _mm_loadl_epi64(((__m128i*) (tmp_src+3)));
		mm4 = _mm_loadl_epi64(((__m128i*) (tmp_src+4)));
		mm5 = _mm_loadl_epi64(((__m128i*) (tmp_src+5)));

		mm0 = _mm_unpacklo_epi16(Null, mm0);
		mm1 = _mm_unpacklo_epi16(Null, mm1);
		mm2 = _mm_unpacklo_epi16(Null, mm2);
		mm3 = _mm_unpacklo_epi16(Null, mm3);
		mm4 = _mm_unpacklo_epi16(Null, mm4);
		mm5 = _mm_unpacklo_epi16(Null, mm5);

		mm0 = _mm_srai_epi32(mm0,16);
		mm1 = _mm_srai_epi32(mm1,16);
		mm2 = _mm_srai_epi32(mm2,16);
		mm3 = _mm_srai_epi32(mm3,16);
		mm4 = _mm_srai_epi32(mm4,16);
		mm5 = _mm_srai_epi32(mm5,16);

		tmp = _mm_sub_epi32(_mm_slli_epi32(_mm_add_epi32(mm2,mm3),2),_mm_add_epi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi32(_mm_slli_epi32(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi32(_mm_add_epi32(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_epi32(_mm_add_epi32(tmp,coeff_512),10);//(result_block+512)>>10

		mm0 = mm4;
		mm1 = mm5;
		mm2 = _mm_loadl_epi64(((__m128i*) (tmp_src+6)));
		mm3 = _mm_loadl_epi64(((__m128i*) (tmp_src+7)));
		mm4 = _mm_loadl_epi64(((__m128i*) (tmp_src+8)));
		mm5 = _mm_loadl_epi64(((__m128i*) (tmp_src+9)));

		mm2 = _mm_unpacklo_epi16(Null, mm2);
		mm3 = _mm_unpacklo_epi16(Null, mm3);
		mm4 = _mm_unpacklo_epi16(Null, mm4);
		mm5 = _mm_unpacklo_epi16(Null, mm5);

		mm2 = _mm_srai_epi32(mm2,16);
		mm3 = _mm_srai_epi32(mm3,16);
		mm4 = _mm_srai_epi32(mm4,16);
		mm5 = _mm_srai_epi32(mm5,16);

		tmp1 = _mm_sub_epi32(_mm_slli_epi32(_mm_add_epi32(mm2,mm3),2),_mm_add_epi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_epi32(_mm_slli_epi32(tmp1,2),tmp1);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_epi32(_mm_add_epi32(mm0,mm5),tmp1); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp1 = _mm_srai_epi32(_mm_add_epi32(tmp1,coeff_512),10);//(result_block+512)>>10

		tmp = _mm_packs_epi32(tmp,tmp1);

		pIn0 = *((__m64*) (luma_src));
		pIn1 = *((__m64*) (luma_src+4));
		mm0 = _mm_setr_epi64(pIn0,pIn1);
		tmp = _mm_packus_epi16(tmp,tmp);//__fast_iclip0_255((result_block+512)>>10)
		mm0 = _mm_packus_epi16(mm0,mm0);
		tmp = _mm_avg_epu8(tmp,mm0);
		_mm_storel_epi64((__m128i*)&block[j][0], tmp);

		mm0 = _mm_loadl_epi64(((__m128i*) (tmp_src+8)));
		mm1 = _mm_loadl_epi64(((__m128i*) (tmp_src+9)));
		mm2 = _mm_loadl_epi64(((__m128i*) (tmp_src+10)));
		mm3 = _mm_loadl_epi64(((__m128i*) (tmp_src+11)));
		mm4 = _mm_loadl_epi64(((__m128i*) (tmp_src+12)));
		mm5 = _mm_loadl_epi64(((__m128i*) (tmp_src+13)));

		mm0 = _mm_unpacklo_epi16(Null, mm0);
		mm1 = _mm_unpacklo_epi16(Null, mm1);
		mm2 = _mm_unpacklo_epi16(Null, mm2);
		mm3 = _mm_unpacklo_epi16(Null, mm3);
		mm4 = _mm_unpacklo_epi16(Null, mm4);
		mm5 = _mm_unpacklo_epi16(Null, mm5);

		mm0 = _mm_srai_epi32(mm0,16);
		mm1 = _mm_srai_epi32(mm1,16);
		mm2 = _mm_srai_epi32(mm2,16);
		mm3 = _mm_srai_epi32(mm3,16);
		mm4 = _mm_srai_epi32(mm4,16);
		mm5 = _mm_srai_epi32(mm5,16);

		tmp = _mm_sub_epi32(_mm_slli_epi32(_mm_add_epi32(mm2,mm3),2),_mm_add_epi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi32(_mm_slli_epi32(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi32(_mm_add_epi32(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_epi32(_mm_add_epi32(tmp,coeff_512),10);//(result_block+512)>>10

		mm0 = mm4;
		mm1 = mm5;
		mm2 = _mm_loadl_epi64(((__m128i*) (tmp_src+14)));
		mm3 = _mm_loadl_epi64(((__m128i*) (tmp_src+15)));
		mm4 = _mm_loadl_epi64(((__m128i*) (tmp_src+16)));
		mm5 = _mm_loadl_epi64(((__m128i*) (tmp_src+17)));

		mm2 = _mm_unpacklo_epi16(Null, mm2);
		mm3 = _mm_unpacklo_epi16(Null, mm3);
		mm4 = _mm_unpacklo_epi16(Null, mm4);
		mm5 = _mm_unpacklo_epi16(Null, mm5);

		mm2 = _mm_srai_epi32(mm2,16);
		mm3 = _mm_srai_epi32(mm3,16);
		mm4 = _mm_srai_epi32(mm4,16);
		mm5 = _mm_srai_epi32(mm5,16);

		tmp1 = _mm_sub_epi32(_mm_slli_epi32(_mm_add_epi32(mm2,mm3),2),_mm_add_epi32(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_epi32(_mm_slli_epi32(tmp1,2),tmp1);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp1 = _mm_add_epi32(_mm_add_epi32(mm0,mm5),tmp1); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp1 = _mm_srai_epi32(_mm_add_epi32(tmp1,coeff_512),10);//(result_block+512)>>10

		tmp = _mm_packs_epi32(tmp,tmp1);

		pIn0 = *((__m64*) (luma_src+8));
		pIn1 = *((__m64*) (luma_src+12));
		mm0 = _mm_setr_epi64(pIn0,pIn1);
		tmp = _mm_packus_epi16(tmp,tmp);//__fast_iclip0_255((result_block+512)>>10)
		mm0 = _mm_packus_epi16(mm0,mm0);
		tmp = _mm_avg_epu8(tmp,mm0);
		_mm_storel_epi64((__m128i*)&block[j][8], tmp);

		tmp_src += 24;
		luma_src += 24;
	}
}

void  get_block_16xh_p33_sse(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	byte *src;
	int j;
	const int w1 = stride;
	const int w2 = stride + stride;
	const int w3 = w2 + w1;

	src = pSrc + w1;

	__m64 mm0, mm1, mm2, mm3, mm4, mm5, mm6, tmp;
	const __m64 Null     = _mm_setzero_si64();
	const __m64 coeff_16 = *((__m64*)ncoeff_16);

	for (j = 0; j < height; j++)
	{
		mm0 = *((__m64*)(src-2));
		mm1 = *((__m64*)(src-1));
		mm2 = *((__m64*)(src));
		mm3 = *((__m64*)(src+1));
		mm4 = *((__m64*)(src+2));
		mm5 = *((__m64*)(src+3));

		mm0 = _mm_unpacklo_pi8(mm0, Null);
		mm1 = _mm_unpacklo_pi8(mm1, Null);
		mm2 = _mm_unpacklo_pi8(mm2, Null);
		mm3 = _mm_unpacklo_pi8(mm3, Null);
		mm4 = _mm_unpacklo_pi8(mm4, Null);
		mm5 = _mm_unpacklo_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		*((DWORD*)&block[j][0]) = _m_to_int(tmp);
		
		mm0 = *((__m64*)(src-2));
		mm1 = *((__m64*)(src-1));
		mm2 = *((__m64*)(src));
		mm3 = *((__m64*)(src+1));
		mm4 = *((__m64*)(src+2));
		mm5 = *((__m64*)(src+3));

		mm0 = _mm_unpackhi_pi8(mm0, Null);
		mm1 = _mm_unpackhi_pi8(mm1, Null);
		mm2 = _mm_unpackhi_pi8(mm2, Null);
		mm3 = _mm_unpackhi_pi8(mm3, Null);
		mm4 = _mm_unpackhi_pi8(mm4, Null);
		mm5 = _mm_unpackhi_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		*((DWORD*)&block[j][4]) = _m_to_int(tmp);

		
		mm0 = *((__m64*)(src+6));
		mm1 = *((__m64*)(src+7));
		mm2 = *((__m64*)(src+8));
		mm3 = *((__m64*)(src+9));
		mm4 = *((__m64*)(src+10));
		mm5 = *((__m64*)(src+11));

		mm0 = _mm_unpacklo_pi8(mm0, Null);
		mm1 = _mm_unpacklo_pi8(mm1, Null);
		mm2 = _mm_unpacklo_pi8(mm2, Null);
		mm3 = _mm_unpacklo_pi8(mm3, Null);
		mm4 = _mm_unpacklo_pi8(mm4, Null);
		mm5 = _mm_unpacklo_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		*((DWORD*)&block[j][8]) = _m_to_int(tmp);

		mm0 = *((__m64*)(src+6));
		mm1 = *((__m64*)(src+7));
		mm2 = *((__m64*)(src+8));
		mm3 = *((__m64*)(src+9));
		mm4 = *((__m64*)(src+10));
		mm5 = *((__m64*)(src+11));

		mm0 = _mm_unpackhi_pi8(mm0, Null);
		mm1 = _mm_unpackhi_pi8(mm1, Null);
		mm2 = _mm_unpackhi_pi8(mm2, Null);
		mm3 = _mm_unpackhi_pi8(mm3, Null);
		mm4 = _mm_unpackhi_pi8(mm4, Null);
		mm5 = _mm_unpackhi_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		*((DWORD*)&block[j][12]) = _m_to_int(tmp);

		src += w1;
	}

	src = pSrc + 1;

	for (j = 0; j < height; j++)
	{
		mm0 = *((__m64*)(src-w2));
		mm1 = *((__m64*)(src-w1));
		mm2 = *((__m64*)(src));
		mm3 = *((__m64*)(src+w1));
		mm4 = *((__m64*)(src+w2));
		mm5 = *((__m64*)(src+w3));
		mm6 = *((__m64*)&block[j][0]);

		mm0 = _mm_unpacklo_pi8(mm0, Null);
		mm1 = _mm_unpacklo_pi8(mm1, Null);
		mm2 = _mm_unpacklo_pi8(mm2, Null);
		mm3 = _mm_unpacklo_pi8(mm3, Null);
		mm4 = _mm_unpacklo_pi8(mm4, Null);
		mm5 = _mm_unpacklo_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		tmp = _mm_avg_pu8(tmp,mm6);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		*((DWORD*)&block[j][0]) = _m_to_int(tmp);

		mm0 = *((__m64*)(src-w2));
		mm1 = *((__m64*)(src-w1));
		mm2 = *((__m64*)(src));
		mm3 = *((__m64*)(src+w1));
		mm4 = *((__m64*)(src+w2));
		mm5 = *((__m64*)(src+w3));
		mm6 = *((__m64*)&block[j][4]);

		mm0 = _mm_unpackhi_pi8(mm0, Null);
		mm1 = _mm_unpackhi_pi8(mm1, Null);
		mm2 = _mm_unpackhi_pi8(mm2, Null);
		mm3 = _mm_unpackhi_pi8(mm3, Null);
		mm4 = _mm_unpackhi_pi8(mm4, Null);
		mm5 = _mm_unpackhi_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		tmp = _mm_avg_pu8(tmp,mm6);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		*((DWORD*)&block[j][4]) = _m_to_int(tmp);

		
	  mm0 = *((__m64*)(src-w2+8));
		mm1 = *((__m64*)(src-w1+8));
		mm2 = *((__m64*)(src+8));
		mm3 = *((__m64*)(src+w1+8));
		mm4 = *((__m64*)(src+w2+8));
		mm5 = *((__m64*)(src+w3+8));
		mm6 = *((__m64*)&block[j][8]);

		mm0 = _mm_unpacklo_pi8(mm0, Null);
		mm1 = _mm_unpacklo_pi8(mm1, Null);
		mm2 = _mm_unpacklo_pi8(mm2, Null);
		mm3 = _mm_unpacklo_pi8(mm3, Null);
		mm4 = _mm_unpacklo_pi8(mm4, Null);
		mm5 = _mm_unpacklo_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		tmp = _mm_avg_pu8(tmp,mm6);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		*((DWORD*)&block[j][8]) = _m_to_int(tmp);

		mm0 = *((__m64*)(src-w2+8));
		mm1 = *((__m64*)(src-w1+8));
		mm2 = *((__m64*)(src+8));
		mm3 = *((__m64*)(src+w1+8));
		mm4 = *((__m64*)(src+w2+8));
		mm5 = *((__m64*)(src+w3+8));
		mm6 = *((__m64*)&block[j][12]);

		mm0 = _mm_unpackhi_pi8(mm0, Null);
		mm1 = _mm_unpackhi_pi8(mm1, Null);
		mm2 = _mm_unpackhi_pi8(mm2, Null);
		mm3 = _mm_unpackhi_pi8(mm3, Null);
		mm4 = _mm_unpackhi_pi8(mm4, Null);
		mm5 = _mm_unpackhi_pi8(mm5, Null);

		tmp = _mm_sub_pi16(_mm_slli_pi16(_mm_add_pi16(mm2,mm3),2),_mm_add_pi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_slli_pi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_pi16(_mm_add_pi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_pi16(_mm_add_pi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packs_pu16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		tmp = _mm_avg_pu8(tmp,mm6);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		*((DWORD*)&block[j][12]) = _m_to_int(tmp);

		src += w1;
	}
}
void  get_block_16xh_p33_sse2(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	byte *src;
	int j;
	const int w1 = stride;
	const int w2 = stride + stride;
	const int w3 = w2 + w1;

	src = pSrc + w1;

	__m128i mm0,mm1,mm2,mm3,mm4,mm5,mm6,tmp;
	const __m128i Null     = _mm_setzero_si128();
	const __m128i coeff_16 = _mm_load_si128((__m128i*)ncoeff_16);

	for (j = 0; j < height; j++)
	{
		mm0 = _mm_loadl_epi64(((__m128i*) (src-2)));
		mm1 = _mm_loadl_epi64(((__m128i*) (src-1)));
		mm2 = _mm_loadl_epi64(((__m128i*) (src)));
		mm3 = _mm_loadl_epi64(((__m128i*) (src+1)));
		mm4 = _mm_loadl_epi64(((__m128i*) (src+2)));
		mm5 = _mm_loadl_epi64(((__m128i*) (src+3)));

		mm0 = _mm_unpacklo_epi8(mm0, Null);
		mm1 = _mm_unpacklo_epi8(mm1, Null);
		mm2 = _mm_unpacklo_epi8(mm2, Null);
		mm3 = _mm_unpacklo_epi8(mm3, Null);
		mm4 = _mm_unpacklo_epi8(mm4, Null);
		mm5 = _mm_unpacklo_epi8(mm5, Null);

		tmp = _mm_sub_epi16(_mm_slli_epi16(_mm_add_epi16(mm2,mm3),2),_mm_add_epi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_slli_epi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_add_epi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_epi16(_mm_add_epi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packus_epi16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		_mm_storel_epi64((__m128i*)&block[j][0], tmp);

		mm0 = _mm_loadl_epi64(((__m128i*) (src+6)));
		mm1 = _mm_loadl_epi64(((__m128i*) (src+7)));
		mm2 = _mm_loadl_epi64(((__m128i*) (src+8)));
		mm3 = _mm_loadl_epi64(((__m128i*) (src+9)));
		mm4 = _mm_loadl_epi64(((__m128i*) (src+10)));
		mm5 = _mm_loadl_epi64(((__m128i*) (src+11)));

		mm0 = _mm_unpacklo_epi8(mm0, Null);
		mm1 = _mm_unpacklo_epi8(mm1, Null);
		mm2 = _mm_unpacklo_epi8(mm2, Null);
		mm3 = _mm_unpacklo_epi8(mm3, Null);
		mm4 = _mm_unpacklo_epi8(mm4, Null);
		mm5 = _mm_unpacklo_epi8(mm5, Null);

		tmp = _mm_sub_epi16(_mm_slli_epi16(_mm_add_epi16(mm2,mm3),2),_mm_add_epi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_slli_epi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_add_epi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_epi16(_mm_add_epi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packus_epi16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		_mm_storel_epi64((__m128i*)&block[j][8], tmp);

		src += w1;
	}

	src = pSrc + 1;

	for (j = 0; j < height; j++)
	{
		mm0 = _mm_loadl_epi64(((__m128i*) (src-w2)));
		mm1 = _mm_loadl_epi64(((__m128i*) (src-w1)));
		mm2 = _mm_loadl_epi64(((__m128i*) (src)));
		mm3 = _mm_loadl_epi64(((__m128i*) (src+w1)));
		mm4 = _mm_loadl_epi64(((__m128i*) (src+w2)));
		mm5 = _mm_loadl_epi64(((__m128i*) (src+w3)));
		mm6 = _mm_loadl_epi64(((__m128i*)&block[j][0]));

		mm0 = _mm_unpacklo_epi8(mm0, Null);
		mm1 = _mm_unpacklo_epi8(mm1, Null);
		mm2 = _mm_unpacklo_epi8(mm2, Null);
		mm3 = _mm_unpacklo_epi8(mm3, Null);
		mm4 = _mm_unpacklo_epi8(mm4, Null);
		mm5 = _mm_unpacklo_epi8(mm5, Null);

		tmp = _mm_sub_epi16(_mm_slli_epi16(_mm_add_epi16(mm2,mm3),2),_mm_add_epi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_slli_epi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_add_epi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_epi16(_mm_add_epi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packus_epi16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		tmp = _mm_avg_epu8(tmp,mm6);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		_mm_storel_epi64((__m128i*)&block[j][0], tmp);

		mm0 = _mm_loadl_epi64(((__m128i*) (src-w2+8)));
		mm1 = _mm_loadl_epi64(((__m128i*) (src-w1+8)));
		mm2 = _mm_loadl_epi64(((__m128i*) (src+8)));
		mm3 = _mm_loadl_epi64(((__m128i*) (src+w1+8)));
		mm4 = _mm_loadl_epi64(((__m128i*) (src+w2+8)));
		mm5 = _mm_loadl_epi64(((__m128i*) (src+w3+8)));
		mm6 = _mm_loadl_epi64(((__m128i*)&block[j][8]));

		mm0 = _mm_unpacklo_epi8(mm0, Null);
		mm1 = _mm_unpacklo_epi8(mm1, Null);
		mm2 = _mm_unpacklo_epi8(mm2, Null);
		mm3 = _mm_unpacklo_epi8(mm3, Null);
		mm4 = _mm_unpacklo_epi8(mm4, Null);
		mm5 = _mm_unpacklo_epi8(mm5, Null);

		tmp = _mm_sub_epi16(_mm_slli_epi16(_mm_add_epi16(mm2,mm3),2),_mm_add_epi16(mm1,mm4));//(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_slli_epi16(tmp,2),tmp);// 5*(-p1 + (p2 + p3)*4 - p4)
		tmp = _mm_add_epi16(_mm_add_epi16(mm0,mm5),tmp); //p0 + 5*(-p1 + (p2 + p3)*4 - p4) + p5
		tmp = _mm_srai_epi16(_mm_add_epi16(tmp,coeff_16),5);//(result_block+16)>>5
		tmp = _mm_packus_epi16(tmp,tmp);//__fast_iclip0_255(((result+16)>>5))
		tmp = _mm_avg_epu8(tmp,mm6);   //block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		_mm_storel_epi64((__m128i*)&block[j][8], tmp);

		src += w1;
	}
}
#endif //H264_ENABLE_INTRINSICS
