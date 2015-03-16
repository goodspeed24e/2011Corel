/*!
*************************************************************************************
* \file loopFilter_mmx.cpp
*
* \brief
*    Use the MMX codes to implement in-loop deblocking.
*************************************************************************************
*/

#include "global.h"
#ifdef H264_ENABLE_INTRINSICS
#include "image.h"
#include "mb_access.h"
#include "loopfilter.h"
#include "clipping.h"
#include "defines.h"
#include <mmintrin.h>
#include <xmmintrin.h>
#include <emmintrin.h>

// Disable "No EMMS at end of function '<function name>'"
#pragma warning ( disable : 4799 )

void Deblock_chroma_h_1_mmx(imgpel* SrcPtrQ, int inc, int Alpha, int Beta, int C0, int pNum)
{
	int inc2 = (inc<<1);
	const __m64 n4 = _mm_set1_pi16((short) 4), nff = _mm_set1_pi16((short) 0xffff);
	const __m64 msk_0 = _mm_set1_pi16((short) 0x00ff);
	const __m64 msk_1 = _mm_set1_pi16((short) 0xff00);
	__m64 mm0, mm1, mm2, mm3, mm4, mm5, mm6, mm7;
	__m64 tmp_pR0, tmp_pL0;

	//L1  = SrcPtrQ[-inc2];
	//L0  = SrcPtrQ[-inc];
	//R0  = SrcPtrQ[0];
	//R1  = SrcPtrQ[inc];
	DWORD* pL1 = (DWORD*) (SrcPtrQ - inc2);
	DWORD* pL0 = (DWORD*) (SrcPtrQ - inc);
	DWORD* pR0 = (DWORD*) (SrcPtrQ);
	DWORD* pR1 = (DWORD*) (SrcPtrQ + inc);

	mm7 = _mm_setzero_si64();
	mm0 = _m_from_int(*pL1);               
	mm1 = _m_from_int(*pL0);                   
	mm2 = _m_from_int(*pR0);                   
	mm3 = _m_from_int(*pR1);

	tmp_pR0 = mm2;
	tmp_pL0 = mm1;

	mm0 = _m_punpcklbw(mm0, mm7);			//mm0 = L1    
	mm1 = _m_punpcklbw(mm1, mm7);			//mm1 = L0
	mm2 = _m_punpcklbw(mm2, mm7);			//mm2 = R0 
	mm3 = _m_punpcklbw(mm3, mm7);			//mm3 = R1

	//xabs(R0 - L0 ) < Alpha
	mm4 = _mm_sub_pi16(mm2, mm1);
	mm7 = _mm_sub_pi16(mm7, mm4);
	mm4 = _mm_max_pi16(mm7, mm4);				//xabs(R0 - L0)
	mm7 = _mm_set1_pi16(Alpha);
	mm4 = _mm_sub_pi16(mm4, mm7);				//mm4 = xabs(R0 - L0) - Alpha

	//xabs( R0 - R1) < Beta
	mm5 = _mm_sub_pi16(mm2, mm3);   
	mm7 = _m_pxor(mm7, mm7);
	mm7 = _mm_sub_pi16(mm7, mm5);
	mm5 = _mm_max_pi16(mm7, mm5);				//xabs(R0 - R1)

	//xabs(L0 - L1) < Beta
	mm6 = _mm_sub_pi16(mm1, mm0);   
	mm7 = _m_pxor(mm7, mm7);
	mm7 = _mm_sub_pi16(mm7, mm6);
	mm6 = _mm_max_pi16(mm7, mm6);				//xabs(L0 - L1)
	mm7 = _mm_set1_pi16(Beta);
	mm5 = _mm_sub_pi16(mm5, mm7);				//mm5 = xabs(R0 - R1) - Beta
	mm6 = _mm_sub_pi16(mm6, mm7);				//mm6 = xabs(L0 - L1) - Beta

	mm7 = _m_pxor(mm7, mm7);
	mm4 = _mm_cmpgt_pi16(mm7, mm4);
	mm5 = _mm_cmpgt_pi16(mm7, mm5);
	mm6 = _mm_cmpgt_pi16(mm7, mm6);	
	mm4 = _m_pand(mm4, mm5);
	mm4 = _m_pand(mm4, mm6);				//mm4 is deblocking mask				

	//c0 = ClipTab[Strng] + 1;
	mm7 = _mm_set1_pi16(C0+1);

	//dif = __fast_iclipX_X(c0, ( ((R0 - L0 ) << 2) + (L1 - R1) + 4) >> 3 );
	mm5 = _mm_sub_pi16(mm2, mm1);
	mm5 = _mm_slli_pi16(mm5, 2);
	mm6 = _mm_sub_pi16(mm0, mm3);
	mm5 = _mm_add_pi16(mm5, mm6);
	mm5 = _mm_add_pi16(mm5, n4);
	mm5 = _mm_srai_pi16(mm5, 3);
	mm5 = _mm_min_pi16(mm5, mm7);
	mm6 = _m_pxor(mm6, mm6);
	mm6 = _mm_sub_pi16(mm6, mm7);
	mm5 = _mm_max_pi16(mm5, mm6);			//mm5 = diff

	//SrcPtrQ[-inc] = CLIP0_255(L0 + dif) ;
	mm6 = _mm_add_pi16(mm1, mm5);
	mm6 = _m_por(_m_pand(mm6, mm4), _m_pand(mm1, _m_pxor(mm4, nff)));	
	mm6 = _m_packuswb(mm6, mm7);

	mm6 = _m_pand(mm6, msk_0);
	mm7 = _m_pand(tmp_pL0, msk_1);
	mm6 = _m_paddb(mm6, mm7);

	*pL0 = _m_to_int(mm6);

	//SrcPtrQ[0] = CLIP0_255(R0 - dif) ;
	mm6 = _mm_sub_pi16(mm2, mm5);
	mm6 = _m_por(_m_pand(mm6, mm4), _m_pand(mm2, _m_pxor(mm4, nff)));
	mm6 = _m_packuswb(mm6, mm7);

	mm6 = _m_pand(mm6, msk_0);
	mm7 = _m_pand(tmp_pR0, msk_1);
	mm6 = _m_paddb(mm6, mm7);

	*pR0 = _m_to_int(mm6);
}
void Deblock_chroma_h_p8_1_mmx(imgpel* SrcPtrQ, int inc, int Alpha, int Beta, int C0, int pNum)
{
	int inc2 = (inc<<1);
	const __m64 n4 = _mm_set1_pi16((short) 4), nff = _mm_set1_pi16((short) 0xffff);
	const __m64 msk_0 = _mm_set1_pi16((short) 0x00ff);
	const __m64 msk_1 = _mm_set1_pi16((short) 0xff00);
	__m64 mm0, mm1, mm2, mm3, mm4, mm5, mm6, mm7, tmp, tmp1;
	__m64 tmp_pR0, tmp_pL0;

	//L1  = SrcPtrQ[-inc2];
	//L0  = SrcPtrQ[-inc];
	//R0  = SrcPtrQ[0];
	//R1  = SrcPtrQ[inc];
	DWORD* pL1 = (DWORD*) (SrcPtrQ - inc2);
	DWORD* pL0 = (DWORD*) (SrcPtrQ - inc);
	DWORD* pR0 = (DWORD*) (SrcPtrQ);
	DWORD* pR1 = (DWORD*) (SrcPtrQ + inc);

	// 1
	mm7 = _mm_setzero_si64();
	mm0 = *((__m64*)pL1);
	mm1 = *((__m64*)pL0);
	mm2 = *((__m64*)pR0);
	mm3 = *((__m64*)pR1);

	tmp_pR0 = mm2;
	tmp_pL0 = mm1; 

	mm0 = _mm_unpacklo_pi8(mm0, mm7);			//mm0 = L1    
	mm1 = _mm_unpacklo_pi8(mm1, mm7);			//mm1 = L0
	mm2 = _mm_unpacklo_pi8(mm2, mm7);			//mm2 = R0 
	mm3 = _mm_unpacklo_pi8(mm3, mm7);			//mm3 = R1

	//xabs(R0 - L0 ) < Alpha
	mm4 = _mm_sub_pi16(mm2, mm1);
	mm7 = _mm_sub_pi16(mm7, mm4);
	mm4 = _mm_max_pi16(mm7, mm4);				//xabs(R0 - L0)
	mm7 = _mm_set1_pi16(Alpha);
	mm4 = _mm_sub_pi16(mm4, mm7);				//mm4 = xabs(R0 - L0) - Alpha

	//xabs( R0 - R1) < Beta
	mm5 = _mm_sub_pi16(mm2, mm3);   
	mm7 = _mm_xor_si64(mm7, mm7);
	mm7 = _mm_sub_pi16(mm7, mm5);
	mm5 = _mm_max_pi16(mm7, mm5);				//xabs(R0 - R1)

	//xabs(L0 - L1) < Beta
	mm6 = _mm_sub_pi16(mm1, mm0);   
	mm7 = _mm_xor_si64(mm7, mm7);
	mm7 = _mm_sub_pi16(mm7, mm6);
	mm6 = _mm_max_pi16(mm7, mm6);				//xabs(L0 - L1)
	mm7 = _mm_set1_pi16(Beta);
	mm5 = _mm_sub_pi16(mm5, mm7);				//mm5 = xabs(R0 - R1) - Beta
	mm6 = _mm_sub_pi16(mm6, mm7);				//mm6 = xabs(L0 - L1) - Beta

	mm7 = _mm_xor_si64(mm7, mm7);
	mm4 = _mm_cmpgt_pi16(mm7, mm4);
	mm5 = _mm_cmpgt_pi16(mm7, mm5);
	mm6 = _mm_cmpgt_pi16(mm7, mm6);	
	mm4 = _mm_and_si64(mm4, mm5);
	mm4 = _mm_and_si64(mm4, mm6);				//mm4 is deblocking mask				

	//c0 = ClipTab[Strng] + 1;
	mm7 = _mm_set1_pi16(C0+1);

	//dif = __fast_iclipX_X(c0, ( ((R0 - L0 ) << 2) + (L1 - R1) + 4) >> 3 );
	mm5 = _mm_sub_pi16(mm2, mm1);
	mm5 = _mm_slli_pi16(mm5, 2);
	mm6 = _mm_sub_pi16(mm0, mm3);
	mm5 = _mm_add_pi16(mm5, mm6);
	mm5 = _mm_add_pi16(mm5, n4);
	mm5 = _mm_srai_pi16(mm5, 3);
	mm5 = _mm_min_pi16(mm5, mm7);
	mm6 = _mm_xor_si64(mm6, mm6);
	mm6 = _mm_sub_pi16(mm6, mm7);
	mm5 = _mm_max_pi16(mm5, mm6);			//mm5 = diff

	//SrcPtrQ[-inc] = CLIP0_255(L0 + dif) ;
	mm6 = _mm_add_pi16(mm1, mm5);
	mm6 = _mm_or_si64(_mm_and_si64(mm6, mm4), _mm_and_si64(mm1, _mm_xor_si64(mm4, nff)));	
	tmp = mm6;
	
	mm6 = _mm_sub_pi16(mm2, mm5);
	mm6 = _mm_or_si64(_mm_and_si64(mm6, mm4), _mm_and_si64(mm2, _mm_xor_si64(mm4, nff)));
	tmp1 = mm6;
	
	//  2
	mm7 = _mm_setzero_si64();
	mm0 = *((__m64*)pL1);
	mm1 = *((__m64*)pL0);
	mm2 = *((__m64*)pR0);
	mm3 = *((__m64*)pR1);

	tmp_pR0 = mm2;
	tmp_pL0 = mm1; 

	mm0 = _mm_unpackhi_pi8(mm0, mm7);			//mm0 = L1    
	mm1 = _mm_unpackhi_pi8(mm1, mm7);			//mm1 = L0
	mm2 = _mm_unpackhi_pi8(mm2, mm7);			//mm2 = R0 
	mm3 = _mm_unpackhi_pi8(mm3, mm7);			//mm3 = R1

	//xabs(R0 - L0 ) < Alpha
	mm4 = _mm_sub_pi16(mm2, mm1);
	mm7 = _mm_sub_pi16(mm7, mm4);
	mm4 = _mm_max_pi16(mm7, mm4);				//xabs(R0 - L0)
	mm7 = _mm_set1_pi16(Alpha);
	mm4 = _mm_sub_pi16(mm4, mm7);				//mm4 = xabs(R0 - L0) - Alpha

	//xabs( R0 - R1) < Beta
	mm5 = _mm_sub_pi16(mm2, mm3);   
	mm7 = _mm_xor_si64(mm7, mm7);
	mm7 = _mm_sub_pi16(mm7, mm5);
	mm5 = _mm_max_pi16(mm7, mm5);				//xabs(R0 - R1)

	//xabs(L0 - L1) < Beta
	mm6 = _mm_sub_pi16(mm1, mm0);   
	mm7 = _mm_xor_si64(mm7, mm7);
	mm7 = _mm_sub_pi16(mm7, mm6);
	mm6 = _mm_max_pi16(mm7, mm6);				//xabs(L0 - L1)
	mm7 = _mm_set1_pi16(Beta);
	mm5 = _mm_sub_pi16(mm5, mm7);				//mm5 = xabs(R0 - R1) - Beta
	mm6 = _mm_sub_pi16(mm6, mm7);				//mm6 = xabs(L0 - L1) - Beta

	mm7 = _mm_xor_si64(mm7, mm7);
	mm4 = _mm_cmpgt_pi16(mm7, mm4);
	mm5 = _mm_cmpgt_pi16(mm7, mm5);
	mm6 = _mm_cmpgt_pi16(mm7, mm6);	
	mm4 = _mm_and_si64(mm4, mm5);
	mm4 = _mm_and_si64(mm4, mm6);				//mm4 is deblocking mask				

	//c0 = ClipTab[Strng] + 1;
	mm7 = _mm_set1_pi16(C0+1);

	//dif = __fast_iclipX_X(c0, ( ((R0 - L0 ) << 2) + (L1 - R1) + 4) >> 3 );
	mm5 = _mm_sub_pi16(mm2, mm1);
	mm5 = _mm_slli_pi16(mm5, 2);
	mm6 = _mm_sub_pi16(mm0, mm3);
	mm5 = _mm_add_pi16(mm5, mm6);
	mm5 = _mm_add_pi16(mm5, n4);
	mm5 = _mm_srai_pi16(mm5, 3);
	mm5 = _mm_min_pi16(mm5, mm7);
	mm6 = _mm_xor_si64(mm6, mm6);
	mm6 = _mm_sub_pi16(mm6, mm7);
	mm5 = _mm_max_pi16(mm5, mm6);			//mm5 = diff

	//SrcPtrQ[-inc] = CLIP0_255(L0 + dif) ;
	mm6 = _mm_add_pi16(mm1, mm5);
	mm6 = _mm_or_si64(_mm_and_si64(mm6, mm4), _mm_and_si64(mm1, _mm_xor_si64(mm4, nff)));	
	mm6 = _mm_packs_pu16(tmp, mm6);

	mm6 = _mm_and_si64(mm6, msk_0);
	mm7 = _mm_and_si64(tmp_pL0, msk_1);
	mm6 = _mm_add_pi8(mm6, mm7);
	*((__m64*)pL0) = mm6;

	//SrcPtrQ[0] = CLIP0_255(R0 - dif) ;
	mm6 = _mm_sub_pi16(mm2, mm5);
	mm6 = _mm_or_si64(_mm_and_si64(mm6, mm4), _mm_and_si64(mm2, _mm_xor_si64(mm4, nff)));
	mm6 = _mm_packs_pu16(tmp1, mm6);

	mm6 = _mm_and_si64(mm6, msk_0);
	mm7 = _mm_and_si64(tmp_pR0, msk_1);
	mm6 = _mm_add_pi8(mm6, mm7);
	*((__m64*)pR0) = mm6;
}

void Deblock_chroma_h_p8_1_sse2(imgpel* SrcPtrQ, int inc, int Alpha, int Beta, int C0, int pNum)
{
	int inc2 = (inc<<1);
	const __m128i n4 = _mm_set1_epi16((short) 4), nff = _mm_set1_epi16((short) 0xffff);
	const __m128i msk_0 = _mm_set1_epi16((short) 0x00ff);
	const __m128i msk_1 = _mm_set1_epi16((short) 0xff00);
	__m128i mm0, mm1, mm2, mm3, mm4, mm5, mm6, mm7;
	__m128i tmp_pR0, tmp_pL0;

	//L1  = SrcPtrQ[-inc2];
	//L0  = SrcPtrQ[-inc];
	//R0  = SrcPtrQ[0];
	//R1  = SrcPtrQ[inc];
	DWORD* pL1 = (DWORD*) (SrcPtrQ - inc2);
	DWORD* pL0 = (DWORD*) (SrcPtrQ - inc);
	DWORD* pR0 = (DWORD*) (SrcPtrQ);
	DWORD* pR1 = (DWORD*) (SrcPtrQ + inc);

	mm7 = _mm_setzero_si128();
	mm0 = _mm_loadl_epi64((__m128i*)pL1);               
	mm1 = _mm_loadl_epi64((__m128i*)pL0);                   
	mm2 = _mm_loadl_epi64((__m128i*)pR0);                   
	mm3 = _mm_loadl_epi64((__m128i*)pR1);

	tmp_pR0 = mm2;
	tmp_pL0 = mm1; 

	mm0 = _mm_unpacklo_epi8(mm0, mm7);			//mm0 = L1    
	mm1 = _mm_unpacklo_epi8(mm1, mm7);			//mm1 = L0
	mm2 = _mm_unpacklo_epi8(mm2, mm7);			//mm2 = R0 
	mm3 = _mm_unpacklo_epi8(mm3, mm7);			//mm3 = R1

	//xabs(R0 - L0 ) < Alpha
	mm4 = _mm_sub_epi16(mm2, mm1);
	mm7 = _mm_sub_epi16(mm7, mm4);
	mm4 = _mm_max_epi16(mm7, mm4);				//xabs(R0 - L0)
	mm7 = _mm_set1_epi16(Alpha);
	mm4 = _mm_sub_epi16(mm4, mm7);				//mm4 = xabs(R0 - L0) - Alpha

	//xabs( R0 - R1) < Beta
	mm5 = _mm_sub_epi16(mm2, mm3);   
	mm7 = _mm_xor_si128(mm7, mm7);
	mm7 = _mm_sub_epi16(mm7, mm5);
	mm5 = _mm_max_epi16(mm7, mm5);				//xabs(R0 - R1)

	//xabs(L0 - L1) < Beta
	mm6 = _mm_sub_epi16(mm1, mm0);   
	mm7 = _mm_xor_si128(mm7, mm7);
	mm7 = _mm_sub_epi16(mm7, mm6);
	mm6 = _mm_max_epi16(mm7, mm6);				//xabs(L0 - L1)
	mm7 = _mm_set1_epi16(Beta);
	mm5 = _mm_sub_epi16(mm5, mm7);				//mm5 = xabs(R0 - R1) - Beta
	mm6 = _mm_sub_epi16(mm6, mm7);				//mm6 = xabs(L0 - L1) - Beta

	mm7 = _mm_xor_si128(mm7, mm7);
	mm4 = _mm_cmpgt_epi16(mm7, mm4);
	mm5 = _mm_cmpgt_epi16(mm7, mm5);
	mm6 = _mm_cmpgt_epi16(mm7, mm6);	
	mm4 = _mm_and_si128(mm4, mm5);
	mm4 = _mm_and_si128(mm4, mm6);				//mm4 is deblocking mask				

	//c0 = ClipTab[Strng] + 1;
	mm7 = _mm_set1_epi16(C0+1);

	//dif = __fast_iclipX_X(c0, ( ((R0 - L0 ) << 2) + (L1 - R1) + 4) >> 3 );
	mm5 = _mm_sub_epi16(mm2, mm1);
	mm5 = _mm_slli_epi16(mm5, 2);
	mm6 = _mm_sub_epi16(mm0, mm3);
	mm5 = _mm_add_epi16(mm5, mm6);
	mm5 = _mm_add_epi16(mm5, n4);
	mm5 = _mm_srai_epi16(mm5, 3);
	mm5 = _mm_min_epi16(mm5, mm7);
	mm6 = _mm_xor_si128(mm6, mm6);
	mm6 = _mm_sub_epi16(mm6, mm7);
	mm5 = _mm_max_epi16(mm5, mm6);			//mm5 = diff

	//SrcPtrQ[-inc] = CLIP0_255(L0 + dif) ;
	mm6 = _mm_add_epi16(mm1, mm5);
	mm6 = _mm_or_si128(_mm_and_si128(mm6, mm4), _mm_and_si128(mm1, _mm_xor_si128(mm4, nff)));	
	mm6 = _mm_packus_epi16(mm6, mm7);

	mm6 = _mm_and_si128(mm6, msk_0);
	mm7 = _mm_and_si128(tmp_pL0, msk_1);
	mm6 = _mm_add_epi8(mm6, mm7);

	_mm_storel_epi64((__m128i*)pL0, mm6);	

	//SrcPtrQ[0] = CLIP0_255(R0 - dif) ;
	mm6 = _mm_sub_epi16(mm2, mm5);
	mm6 = _mm_or_si128(_mm_and_si128(mm6, mm4), _mm_and_si128(mm2, _mm_xor_si128(mm4, nff)));
	mm6 = _mm_packus_epi16(mm6, mm7);

	mm6 = _mm_and_si128(mm6, msk_0);
	mm7 = _mm_and_si128(tmp_pR0, msk_1);
	mm6 = _mm_add_epi8(mm6, mm7);

	_mm_storel_epi64((__m128i*)pR0, mm6);	
}

void Deblock_chromaUV_h_1_mmx(imgpel* SrcUV, int inc, int Alpha, int Beta, int C0, int pNum)
{
	int inc2 = (inc<<1);
	const __m64 n4 = _mm_set1_pi16((short) 4), nff = _mm_set1_pi16((short) 0xffff);
	__m64 mm0, mm1, mm2, mm3, mm4, mm5, mm6, mm7;

	//L1  = SrcPtrQ[-inc2];
	//L0  = SrcPtrQ[-inc];
	//R0  = SrcPtrQ[0];
	//R1  = SrcPtrQ[inc];
	DWORD* pUL1 = (DWORD*) (SrcUV - inc2);
	DWORD* pUL0 = (DWORD*) (SrcUV - inc);
	DWORD* pUR0 = (DWORD*) (SrcUV);
	DWORD* pUR1 = (DWORD*) (SrcUV + inc);

	mm0 = _m_from_int(*pUL1);               
	mm1 = _m_from_int(*pUL0);                   
	mm2 = _m_from_int(*pUR0);                   
	mm3 = _m_from_int(*pUR1);

	mm7 = _mm_setzero_si64();
	mm0 = _m_punpcklbw(mm0, mm7);			//mm0 = L1    
	mm1 = _m_punpcklbw(mm1, mm7);			//mm1 = L0
	mm2 = _m_punpcklbw(mm2, mm7);			//mm2 = R0 
	mm3 = _m_punpcklbw(mm3, mm7);			//mm3 = R1

	//xabs(R0 - L0 ) < Alpha
	mm4 = _mm_sub_pi16(mm2, mm1);
	mm7 = _mm_sub_pi16(mm7, mm4);
	mm4 = _mm_max_pi16(mm7, mm4);				//xabs(R0 - L0)
	mm7 = _mm_set1_pi16(Alpha);
	mm4 = _mm_sub_pi16(mm4, mm7);				//mm4 = xabs(R0 - L0) - Alpha

	//xabs( R0 - R1) < Beta
	mm5 = _mm_sub_pi16(mm2, mm3);   
	mm7 = _m_pxor(mm7, mm7);
	mm7 = _mm_sub_pi16(mm7, mm5);
	mm5 = _mm_max_pi16(mm7, mm5);				//xabs(R0 - R1)

	//xabs(L0 - L1) < Beta
	mm6 = _mm_sub_pi16(mm1, mm0);   
	mm7 = _m_pxor(mm7, mm7);
	mm7 = _mm_sub_pi16(mm7, mm6);
	mm6 = _mm_max_pi16(mm7, mm6);				//xabs(L0 - L1)
	mm7 = _mm_set1_pi16(Beta);
	mm5 = _mm_sub_pi16(mm5, mm7);				//mm5 = xabs(R0 - R1) - Beta
	mm6 = _mm_sub_pi16(mm6, mm7);				//mm6 = xabs(L0 - L1) - Beta

	mm7 = _m_pxor(mm7, mm7);
	mm4 = _mm_cmpgt_pi16(mm7, mm4);
	mm5 = _mm_cmpgt_pi16(mm7, mm5);
	mm6 = _mm_cmpgt_pi16(mm7, mm6);	
	mm4 = _m_pand(mm4, mm5);
	mm4 = _m_pand(mm4, mm6);				//mm4 is deblocking mask				

	//c0 = ClipTab[Strng] + 1;
	mm7 = _mm_set1_pi16(C0+1);

	//dif = __fast_iclipX_X(c0, ( ((R0 - L0 ) << 2) + (L1 - R1) + 4) >> 3 );
	mm5 = _mm_sub_pi16(mm2, mm1);
	mm5 = _mm_slli_pi16(mm5, 2);
	mm6 = _mm_sub_pi16(mm0, mm3);
	mm5 = _mm_add_pi16(mm5, mm6);
	mm5 = _mm_add_pi16(mm5, n4);
	mm5 = _mm_srai_pi16(mm5, 3);
	mm5 = _mm_min_pi16(mm5, mm7);
	mm6 = _m_pxor(mm6, mm6);
	mm6 = _mm_sub_pi16(mm6, mm7);
	mm5 = _mm_max_pi16(mm5, mm6);			//mm5 = diff

	//SrcPtrQ[-inc] = CLIP0_255(L0 + dif) ;
	mm6 = _mm_add_pi16(mm1, mm5);
	mm6 = _m_por(_m_pand(mm6, mm4), _m_pand(mm1, _m_pxor(mm4, nff)));	
	mm1 = _m_packuswb(mm6, mm7);
	*pUL0 = _m_to_int(mm1);

	//SrcPtrQ[0] = CLIP0_255(R0 - dif) ;
	mm6 = _mm_sub_pi16(mm2, mm5);
	mm6 = _m_por(_m_pand(mm6, mm4), _m_pand(mm2, _m_pxor(mm4, nff)));
	mm2 = _m_packuswb(mm6, mm7);
	*pUR0 = _m_to_int(mm2);	
}
void Deblock_chromaUV_h_p8_1_mmx(imgpel* SrcUV, int inc, int Alpha, int Beta, int C0, int pNum)
{
	int inc2 = (inc<<1);
	const __m64 n4 = _mm_set1_pi16((short) 4), nff = _mm_set1_pi16((short) 0xffff);
	__m64 mm0, mm1, mm2, mm3, mm4, mm5, mm6, mm7;
	__m64 tmp1, tmp2;
	imgpel* SrcPtrQ = SrcUV;

	//L1  = SrcPtrQ[-inc2];
	//L0  = SrcPtrQ[-inc];
	//R0  = SrcPtrQ[0];
	//R1  = SrcPtrQ[inc];
	DWORD* pL1 = (DWORD*) (SrcPtrQ - inc2);
	DWORD* pL0 = (DWORD*) (SrcPtrQ - inc);
	DWORD* pR0 = (DWORD*) (SrcPtrQ);
	DWORD* pR1 = (DWORD*) (SrcPtrQ + inc);

  // 1
	mm7 = _mm_setzero_si64();
	mm0 = *((__m64*)pL1);
	mm1 = *((__m64*)pL0);
	mm2 = *((__m64*)pR0);
	mm3 = *((__m64*)pR1);
	
	mm0 = _mm_unpacklo_pi8(mm0, mm7);			//mm0 = L1    
	mm1 = _mm_unpacklo_pi8(mm1, mm7);			//mm1 = L0
	mm2 = _mm_unpacklo_pi8(mm2, mm7);			//mm2 = R0 
	mm3 = _mm_unpacklo_pi8(mm3, mm7);			//mm3 = R1

	//xabs(R0 - L0 ) < Alpha
	mm4 = _mm_sub_pi16(mm2, mm1);
	mm7 = _mm_sub_pi16(mm7, mm4);
	mm4 = _mm_max_pi16(mm7, mm4);				//xabs(R0 - L0)
	mm7 = _mm_set1_pi16(Alpha);
	mm4 = _mm_cmpgt_pi16(mm7, mm4);				//mm4 = xabs(R0 - L0) - Alpha

	//xabs( R0 - R1) < Beta
	mm5 = _mm_sub_pi16(mm2, mm3);   
	mm7 = _mm_xor_si64(mm7, mm7);
	mm7 = _mm_sub_pi16(mm7, mm5);
	mm5 = _mm_max_pi16(mm7, mm5);				//xabs(R0 - R1)

	//xabs(L0 - L1) < Beta
	mm6 = _mm_sub_pi16(mm1, mm0);   
	mm7 = _mm_xor_si64(mm7, mm7);
	mm7 = _mm_sub_pi16(mm7, mm6);
	mm6 = _mm_max_pi16(mm7, mm6);				//xabs(L0 - L1)
	mm7 = _mm_set1_pi16(Beta);
	mm5 = _mm_cmpgt_pi16(mm7, mm5);				//mm5 = xabs(R0 - R1) - Beta
	mm6 = _mm_cmpgt_pi16(mm7, mm6);				//mm6 = xabs(L0 - L1) - Beta

	mm7 = _mm_xor_si64(mm7, mm7);

	mm4 = _mm_and_si64(mm4, mm5);
	mm4 = _mm_and_si64(mm4, mm6);				//mm4 is deblocking mask				

	//c0 = ClipTab[Strng] + 1;
	mm7 = _mm_set1_pi16(C0+1);

	//dif = __fast_iclipX_X(c0, ( ((R0 - L0 ) << 2) + (L1 - R1) + 4) >> 3 );
	mm5 = _mm_sub_pi16(mm2, mm1);
	mm5 = _mm_slli_pi16(mm5, 2);
	mm6 = _mm_sub_pi16(mm0, mm3);
	mm5 = _mm_add_pi16(mm5, mm6);
	mm5 = _mm_add_pi16(mm5, n4);
	mm5 = _mm_srai_pi16(mm5, 3);
	mm5 = _mm_min_pi16(mm5, mm7);
	mm6 = _mm_xor_si64(mm6, mm6);
	mm6 = _mm_sub_pi16(mm6, mm7);
	mm5 = _mm_max_pi16(mm5, mm6);			//mm5 = diff

	//SrcPtrQ[-inc] = CLIP0_255(L0 + dif) ;
	mm6 = _mm_add_pi16(mm1, mm5);
	mm6 = _mm_or_si64(_mm_and_si64(mm6, mm4), _mm_and_si64(mm1, _mm_xor_si64(mm4, nff)));	
	tmp1 = mm6;
	mm6 = _mm_sub_pi16(mm2, mm5);
	mm6 = _mm_or_si64(_mm_and_si64(mm6, mm4), _mm_and_si64(mm2, _mm_xor_si64(mm4, nff)));
	tmp2 = mm6;
	
	// 2
	mm7 = _mm_setzero_si64();
	mm0 = *((__m64*)pL1);
	mm1 = *((__m64*)pL0);
	mm2 = *((__m64*)pR0);
	mm3 = *((__m64*)pR1);
	
	mm0 = _mm_unpackhi_pi8(mm0, mm7);			//mm0 = L1    
	mm1 = _mm_unpackhi_pi8(mm1, mm7);			//mm1 = L0
	mm2 = _mm_unpackhi_pi8(mm2, mm7);			//mm2 = R0 
	mm3 = _mm_unpackhi_pi8(mm3, mm7);			//mm3 = R1

	//xabs(R0 - L0 ) < Alpha
	mm4 = _mm_sub_pi16(mm2, mm1);
	mm7 = _mm_sub_pi16(mm7, mm4);
	mm4 = _mm_max_pi16(mm7, mm4);				//xabs(R0 - L0)
	mm7 = _mm_set1_pi16(Alpha);
	mm4 = _mm_cmpgt_pi16(mm7, mm4);				//mm4 = xabs(R0 - L0) - Alpha

	//xabs( R0 - R1) < Beta
	mm5 = _mm_sub_pi16(mm2, mm3);   
	mm7 = _mm_xor_si64(mm7, mm7);
	mm7 = _mm_sub_pi16(mm7, mm5);
	mm5 = _mm_max_pi16(mm7, mm5);				//xabs(R0 - R1)

	//xabs(L0 - L1) < Beta
	mm6 = _mm_sub_pi16(mm1, mm0);   
	mm7 = _mm_xor_si64(mm7, mm7);
	mm7 = _mm_sub_pi16(mm7, mm6);
	mm6 = _mm_max_pi16(mm7, mm6);				//xabs(L0 - L1)
	mm7 = _mm_set1_pi16(Beta);
	mm5 = _mm_cmpgt_pi16(mm7, mm5);				//mm5 = xabs(R0 - R1) - Beta
	mm6 = _mm_cmpgt_pi16(mm7, mm6);				//mm6 = xabs(L0 - L1) - Beta

	mm7 = _mm_xor_si64(mm7, mm7);

	mm4 = _mm_and_si64(mm4, mm5);
	mm4 = _mm_and_si64(mm4, mm6);				//mm4 is deblocking mask				

	//c0 = ClipTab[Strng] + 1;
	mm7 = _mm_set1_pi16(C0+1);

	//dif = __fast_iclipX_X(c0, ( ((R0 - L0 ) << 2) + (L1 - R1) + 4) >> 3 );
	mm5 = _mm_sub_pi16(mm2, mm1);
	mm5 = _mm_slli_pi16(mm5, 2);
	mm6 = _mm_sub_pi16(mm0, mm3);
	mm5 = _mm_add_pi16(mm5, mm6);
	mm5 = _mm_add_pi16(mm5, n4);
	mm5 = _mm_srai_pi16(mm5, 3);
	mm5 = _mm_min_pi16(mm5, mm7);
	mm6 = _mm_xor_si64(mm6, mm6);
	mm6 = _mm_sub_pi16(mm6, mm7);
	mm5 = _mm_max_pi16(mm5, mm6);			//mm5 = diff

	//SrcPtrQ[-inc] = CLIP0_255(L0 + dif) ;
	mm6 = _mm_add_pi16(mm1, mm5);
	mm6 = _mm_or_si64(_mm_and_si64(mm6, mm4), _mm_and_si64(mm1, _mm_xor_si64(mm4, nff)));	
	mm6 = _mm_packs_pu16(tmp1, mm6);
	*((__m64*)pL0) = mm6;

	//SrcPtrQ[0] = CLIP0_255(R0 - dif) ;
	mm6 = _mm_sub_pi16(mm2, mm5);
	mm6 = _mm_or_si64(_mm_and_si64(mm6, mm4), _mm_and_si64(mm2, _mm_xor_si64(mm4, nff)));
	mm6 = _mm_packs_pu16(tmp2, mm6);
	*((__m64*)pR0) = mm6;
}
void Deblock_chromaUV_h_p8_1_sse2(imgpel* SrcUV, int inc, int Alpha, int Beta, int C0, int pNum)
{
	int inc2 = (inc<<1);
	const __m128i n4 = _mm_set1_epi16((short) 4), nff = _mm_set1_epi16((short) 0xffff);
	__m128i mm0, mm1, mm2, mm3, mm4, mm5, mm6, mm7;
	imgpel* SrcPtrQ = SrcUV;

	//L1  = SrcPtrQ[-inc2];
	//L0  = SrcPtrQ[-inc];
	//R0  = SrcPtrQ[0];
	//R1  = SrcPtrQ[inc];
	DWORD* pL1 = (DWORD*) (SrcPtrQ - inc2);
	DWORD* pL0 = (DWORD*) (SrcPtrQ - inc);
	DWORD* pR0 = (DWORD*) (SrcPtrQ);
	DWORD* pR1 = (DWORD*) (SrcPtrQ + inc);

	mm7 = _mm_setzero_si128();
	mm0 = _mm_loadl_epi64((__m128i*)pL1);               
	mm1 = _mm_loadl_epi64((__m128i*)pL0);                   
	mm2 = _mm_loadl_epi64((__m128i*)pR0);                   
	mm3 = _mm_loadl_epi64((__m128i*)pR1);
	mm0 = _mm_unpacklo_epi8(mm0, mm7);			//mm0 = L1    
	mm1 = _mm_unpacklo_epi8(mm1, mm7);			//mm1 = L0
	mm2 = _mm_unpacklo_epi8(mm2, mm7);			//mm2 = R0 
	mm3 = _mm_unpacklo_epi8(mm3, mm7);			//mm3 = R1

	//xabs(R0 - L0 ) < Alpha
	mm4 = _mm_sub_epi16(mm2, mm1);
	mm7 = _mm_sub_epi16(mm7, mm4);
	mm4 = _mm_max_epi16(mm7, mm4);				//xabs(R0 - L0)
	mm7 = _mm_set1_epi16(Alpha);
	mm4 = _mm_cmpgt_epi16(mm7, mm4);				//mm4 = xabs(R0 - L0) - Alpha

	//xabs( R0 - R1) < Beta
	mm5 = _mm_sub_epi16(mm2, mm3);   
	mm7 = _mm_xor_si128(mm7, mm7);
	mm7 = _mm_sub_epi16(mm7, mm5);
	mm5 = _mm_max_epi16(mm7, mm5);				//xabs(R0 - R1)

	//xabs(L0 - L1) < Beta
	mm6 = _mm_sub_epi16(mm1, mm0);   
	mm7 = _mm_xor_si128(mm7, mm7);
	mm7 = _mm_sub_epi16(mm7, mm6);
	mm6 = _mm_max_epi16(mm7, mm6);				//xabs(L0 - L1)
	mm7 = _mm_set1_epi16(Beta);
	mm5 = _mm_cmpgt_epi16(mm7, mm5);				//mm5 = xabs(R0 - R1) - Beta
	mm6 = _mm_cmpgt_epi16(mm7, mm6);				//mm6 = xabs(L0 - L1) - Beta

	mm7 = _mm_xor_si128(mm7, mm7);

	mm4 = _mm_and_si128(mm4, mm5);
	mm4 = _mm_and_si128(mm4, mm6);				//mm4 is deblocking mask				

	//c0 = ClipTab[Strng] + 1;
	mm7 = _mm_set1_epi16(C0+1);

	//dif = __fast_iclipX_X(c0, ( ((R0 - L0 ) << 2) + (L1 - R1) + 4) >> 3 );
	mm5 = _mm_sub_epi16(mm2, mm1);
	mm5 = _mm_slli_epi16(mm5, 2);
	mm6 = _mm_sub_epi16(mm0, mm3);
	mm5 = _mm_add_epi16(mm5, mm6);
	mm5 = _mm_add_epi16(mm5, n4);
	mm5 = _mm_srai_epi16(mm5, 3);
	mm5 = _mm_min_epi16(mm5, mm7);
	mm6 = _mm_xor_si128(mm6, mm6);
	mm6 = _mm_sub_epi16(mm6, mm7);
	mm5 = _mm_max_epi16(mm5, mm6);			//mm5 = diff

	//SrcPtrQ[-inc] = CLIP0_255(L0 + dif) ;
	mm6 = _mm_add_epi16(mm1, mm5);
	mm6 = _mm_or_si128(_mm_and_si128(mm6, mm4), _mm_and_si128(mm1, _mm_xor_si128(mm4, nff)));	
	mm6 = _mm_packus_epi16(mm6, mm7);
	_mm_storel_epi64((__m128i*)pL0, mm6);

	//SrcPtrQ[0] = CLIP0_255(R0 - dif) ;
	mm6 = _mm_sub_epi16(mm2, mm5);
	mm6 = _mm_or_si128(_mm_and_si128(mm6, mm4), _mm_and_si128(mm2, _mm_xor_si128(mm4, nff)));
	mm6 = _mm_packus_epi16(mm6, mm7);
	_mm_storel_epi64((__m128i*)pR0, mm6);
}

void Deblock_chroma_v_1_mmx(imgpel* SrcPtrQ, int dpel, int Alpha, int Beta, int C0, int pNum)
{
	const __m64 n4 = _mm_set1_pi16((short) 4), nff = _mm_set1_pi16((short) 0xffff);
	__m64 mm0, mm1, mm2, mm3, mm4, mm5, mm6, mm7, tmp1, tmp2, tmp3, tmp4;
	__m64 tmp_L1, tmp_L2;
	const __m64 msk_0 = _mm_set1_pi16((short) 0x00ff);
	const __m64 msk_1 = _mm_set1_pi16((short) 0xff00);

	//L1  = SrcPtrQ[-2] ;
	//L0  = SrcPtrQ[-1] ;
	//R0  = SrcPtrQ[ 0] ;	  
	//R1  = SrcPtrQ[ 1] ;
	DWORD* Line1 = (DWORD*) (SrcPtrQ - 4);
	DWORD* Line2 = (DWORD*) (SrcPtrQ - 4 + dpel);

	mm7 = _mm_setzero_si64();	
	
	//  1
	mm0 = *((__m64*)Line1); // 12344321
	mm1 = *((__m64*)Line2); // 56788765

	tmp_L1 = mm0;
	tmp_L2 = mm1;

	mm0 = _mm_unpacklo_pi8(mm0, mm7); // 10203040
	mm1 = _mm_unpacklo_pi8(mm1, mm7);	// 50607080
	mm6 = _mm_xor_si64(mm7, mm7);
	
	//  2
	mm2 = *((__m64*)Line1);  //12344321
	mm3 = *((__m64*)Line2);  //56788765

	mm2 = _mm_unpackhi_pi8(mm2, mm7); // 40302010
	mm3 = _mm_unpackhi_pi8(mm3, mm7);	// 80706050		
	

	//transpose
	mm4 = _mm_unpacklo_pi32(mm2,mm3); // 40308070
	mm5 = _mm_unpacklo_pi32(mm0,mm1); // 10205060
	mm6 = _mm_unpackhi_pi32(mm2,mm3);  // 20106050
	mm7 = _mm_unpackhi_pi32(mm0,mm1);	 // 30407080	
	mm0 = mm5;		//mm0 = L1	
	mm1 = mm7;		//mm1 = L0	
	mm2 = mm4;		//mm2 = R0
	mm3 = mm6;		//mm3 = R1	

  // 1
	mm7 = _mm_setzero_si64();
	mm6 = _mm_xor_si64(mm7, mm7);

	//xabs(R0 - L0 ) < Alpha
	mm4 = _mm_sub_pi16(mm2, mm1);
	mm7 = _mm_xor_si64(mm7, mm7);
	mm7 = _mm_sub_pi16(mm7, mm4);
	mm4 = _mm_max_pi16(mm7, mm4);				//xabs(R0 - L0)
	mm7 = _mm_set1_pi16(Alpha);
	mm4 = _mm_sub_pi16(mm4, mm7);				//mm4 = xabs(R0 - L0) - Alpha

	//xabs( R0 - R1) < Beta
	mm5 = _mm_sub_pi16(mm2, mm3);   
	mm7 = _mm_xor_si64(mm7, mm7);
	mm7 = _mm_sub_pi16(mm7, mm5);
	mm5 = _mm_max_pi16(mm7, mm5);				//xabs(R0 - R1)

	//xabs(L0 - L1) < Beta
	mm6 = _mm_sub_pi16(mm1, mm0);   
	mm7 = _mm_xor_si64(mm7, mm7);
	mm7 = _mm_sub_pi16(mm7, mm6);
	mm6 = _mm_max_pi16(mm7, mm6);				//xabs(L0 - L1)
	mm7 = _mm_set1_pi16(Beta);
	mm5 = _mm_sub_pi16(mm5, mm7);				//mm5 = xabs(R0 - R1) - Beta
	mm6 = _mm_sub_pi16(mm6, mm7);				//mm6 = xabs(L0 - L1) - Beta

	mm7 = _mm_xor_si64(mm7, mm7);
	mm4 = _mm_cmpgt_pi16(mm7, mm4);
	mm5 = _mm_cmpgt_pi16(mm7, mm5);
	mm6 = _mm_cmpgt_pi16(mm7, mm6);
	mm4 = _mm_and_si64(mm4, mm5);
	mm4 = _mm_and_si64(mm4, mm6);				//mm4 is deblocking mask

	//c0 = C0 + 1;
	mm7 = _mm_set1_pi16(C0+1);

	//dif = __fast_iclipX_X(c0, ( ((R0 - L0 ) << 2) + (L1 - R1) + 4) >> 3 );
	mm5 = _mm_sub_pi16(mm2, mm1);
	mm5 = _mm_slli_pi16(mm5, 2);
	mm6 = _mm_sub_pi16(mm0, mm3);
	mm5 = _mm_add_pi16(mm5, mm6);
	mm5 = _mm_add_pi16(mm5, n4);
	mm5 = _mm_srai_pi16(mm5, 3);
	mm5 = _mm_min_pi16(mm5, mm7);
	mm6 = _mm_xor_si64(mm6, mm6);
	mm6 = _mm_sub_pi16(mm6, mm7);
	mm5 = _mm_max_pi16(mm5, mm6);				//mm5 = diff

	//SrcPtrQ[-1] = CLIP0_255(L0 + dif) ;
	mm6 = _mm_add_pi16(mm1, mm5);
	mm1 = _mm_or_si64(_mm_and_si64(mm6, mm4), _mm_and_si64(mm1, _mm_xor_si64(mm4, nff)));	

	//SrcPtrQ[0] = CLIP0_255(R0 - dif) ;
	mm6 = _mm_sub_pi16(mm2, mm5);
	mm2 = _mm_or_si64(_mm_and_si64(mm6, mm4), _mm_and_si64(mm2, _mm_xor_si64(mm4, nff)));

  tmp1 = mm0;
  tmp2 = mm1;
  tmp3 = mm2; 
  tmp4 = mm3;
	// mm0: 0a 0b 0c 0d
	// mm1: 1a 1b 1c 1d
  // mm2: 2a 2b 2c 2d
  // mm3: 3a 3b 3c 3d

	mm0 = _mm_setzero_si64();		//mm0 = L1	
	mm1 = _mm_setzero_si64();		//mm1 = L0	
	mm2 = _mm_setzero_si64();		//mm2 = R0
	mm3 = _mm_setzero_si64();		//mm3 = R1	

  // 2
	mm7 = _mm_setzero_si64();
	mm6 = _mm_xor_si64(mm7, mm7);

	//xabs(R0 - L0 ) < Alpha
	mm4 = _mm_sub_pi16(mm2, mm1);
	mm7 = _mm_xor_si64(mm7, mm7);
	mm7 = _mm_sub_pi16(mm7, mm4);
	mm4 = _mm_max_pi16(mm7, mm4);				//xabs(R0 - L0)
	mm7 = _mm_set1_pi16(Alpha);
	mm4 = _mm_sub_pi16(mm4, mm7);				//mm4 = xabs(R0 - L0) - Alpha

	//xabs( R0 - R1) < Beta
	mm5 = _mm_sub_pi16(mm2, mm3);   
	mm7 = _mm_xor_si64(mm7, mm7);
	mm7 = _mm_sub_pi16(mm7, mm5);
	mm5 = _mm_max_pi16(mm7, mm5);				//xabs(R0 - R1)

	//xabs(L0 - L1) < Beta
	mm6 = _mm_sub_pi16(mm1, mm0);   
	mm7 = _mm_xor_si64(mm7, mm7);
	mm7 = _mm_sub_pi16(mm7, mm6);
	mm6 = _mm_max_pi16(mm7, mm6);				//xabs(L0 - L1)
	mm7 = _mm_set1_pi16(Beta);
	mm5 = _mm_sub_pi16(mm5, mm7);				//mm5 = xabs(R0 - R1) - Beta
	mm6 = _mm_sub_pi16(mm6, mm7);				//mm6 = xabs(L0 - L1) - Beta

	mm7 = _mm_xor_si64(mm7, mm7);
	mm4 = _mm_cmpgt_pi16(mm7, mm4);
	mm5 = _mm_cmpgt_pi16(mm7, mm5);
	mm6 = _mm_cmpgt_pi16(mm7, mm6);
	mm4 = _mm_and_si64(mm4, mm5);
	mm4 = _mm_and_si64(mm4, mm6);				//mm4 is deblocking mask

	//c0 = C0 + 1;
	mm7 = _mm_set1_pi16(C0+1);

	//dif = __fast_iclipX_X(c0, ( ((R0 - L0 ) << 2) + (L1 - R1) + 4) >> 3 );
	mm5 = _mm_sub_pi16(mm2, mm1);
	mm5 = _mm_slli_pi16(mm5, 2);
	mm6 = _mm_sub_pi16(mm0, mm3);
	mm5 = _mm_add_pi16(mm5, mm6);
	mm5 = _mm_add_pi16(mm5, n4);
	mm5 = _mm_srai_pi16(mm5, 3);
	mm5 = _mm_min_pi16(mm5, mm7);
	mm6 = _mm_xor_si64(mm6, mm6);
	mm6 = _mm_sub_pi16(mm6, mm7);
	mm5 = _mm_max_pi16(mm5, mm6);				//mm5 = diff

	//SrcPtrQ[-1] = CLIP0_255(L0 + dif) ;
	mm6 = _mm_add_pi16(mm1, mm5);
	mm1 = _mm_or_si64(_mm_and_si64(mm6, mm4), _mm_and_si64(mm1, _mm_xor_si64(mm4, nff)));	

	//SrcPtrQ[0] = CLIP0_255(R0 - dif) ;
	mm6 = _mm_sub_pi16(mm2, mm5);
	mm2 = _mm_or_si64(_mm_and_si64(mm6, mm4), _mm_and_si64(mm2, _mm_xor_si64(mm4, nff)));

	// mm0: 0e 0f 0g 0h
	// mm1: 1e 1f 1g 1h
  // mm2: 2e 2f 2g 2h
  // mm3: 3e 3f 3g 3h
	
	// tmp1: 0a 0b 0c 0d
	// tmp2: 1a 1b 1c 1d
  // tmp3: 2a 2b 2c 2d
  // tmp4: 3a 3b 3c 3d

	//transpose
	mm4 = _mm_unpacklo_pi32(tmp1, tmp2); // 0a 0b 1a 1b :mm0
	mm5 = _mm_unpackhi_pi32(tmp1, tmp2); // 0c 0d 1c 1d :mm1
	mm6 = _mm_unpacklo_pi32(tmp3, tmp4); // 2a 2b 3a 3b :mm0' 
	mm7 = _mm_unpackhi_pi32(tmp3, tmp4); // 2c 2d 3c 3d :mm1'

	//save to L0 and R0
	mm0 = _mm_packs_pu16(mm4, mm6);
	mm1 = _mm_packs_pu16(mm5, mm7);

	//restore u or v
	mm0 = _mm_and_si64(mm0, msk_0);
	mm7 = _mm_and_si64(tmp_L1, msk_1);
	mm0 = _mm_add_pi8(mm0, mm7);

	mm1 = _mm_and_si64(mm1, msk_0);
	mm7 = _mm_and_si64(tmp_L2, msk_1);
	mm1 = _mm_add_pi8(mm1, mm7);

	*((__m64*)Line1) = mm0;
	*((__m64*)Line2) = mm1;
}
void Deblock_chroma_v_1_sse2(imgpel* SrcPtrQ, int dpel, int Alpha, int Beta, int C0, int pNum)
{
	const __m128i n4 = _mm_set1_epi16((short) 4), nff = _mm_set1_epi16((short) 0xffff);
	__m128i mm0, mm1, mm2, mm3, mm4, mm5, mm6, mm7;
	__m128i tmp_L1, tmp_L2;
	const __m128i msk_0 = _mm_set1_epi16((short) 0x00ff);
	const __m128i msk_1 = _mm_set1_epi16((short) 0xff00);

	//L1  = SrcPtrQ[-2] ;
	//L0  = SrcPtrQ[-1] ;
	//R0  = SrcPtrQ[ 0] ;	  
	//R1  = SrcPtrQ[ 1] ;
	DWORD* Line1 = (DWORD*) (SrcPtrQ - 4);
	DWORD* Line2 = (DWORD*) (SrcPtrQ - 4 + dpel);

	mm7 = _mm_setzero_si128();	
	mm0 = _mm_loadl_epi64((__m128i*)Line1);               
	mm1 = _mm_loadl_epi64((__m128i*)Line2);

	tmp_L1 = mm0;
	tmp_L2 = mm1;

	mm0 = _mm_unpacklo_epi8(mm0, mm7);
	mm1 = _mm_unpacklo_epi8(mm1, mm7);			
	//mm2 = _m_from_int(*Line1);                
	//mm2 = _m_punpcklbw(mm2, mm7);			    
	//mm3 = _m_from_int(*Line2);                
	//mm3 = _m_punpcklbw(mm3, mm7);
	mm6 = _mm_xor_si128(mm7, mm7);

	//transpose
	mm4 = _mm_unpackhi_epi32(mm0,mm1);
	mm5 = _mm_unpacklo_epi32(mm0,mm1);
	//mm6 = _mm_unpackhi_pi16(mm2,mm3);
	//mm7 = _mm_unpacklo_pi16(mm2,mm3);		
	mm0 = _mm_unpacklo_epi64(mm5,mm7);		//mm0 = L1	
	mm1 = _mm_unpackhi_epi64(mm5,mm7);		//mm1 = L0	
	mm2 = _mm_unpacklo_epi64(mm4,mm6);		//mm2 = R0
	mm3 = _mm_unpackhi_epi64(mm4,mm6);		//mm3 = R1	

	//xabs(R0 - L0 ) < Alpha
	mm4 = _mm_sub_epi16(mm2, mm1);
	mm7 = _mm_xor_si128(mm7, mm7);
	mm7 = _mm_sub_epi16(mm7, mm4);
	mm4 = _mm_max_epi16(mm7, mm4);				//xabs(R0 - L0)
	mm7 = _mm_set1_epi16(Alpha);
	mm4 = _mm_sub_epi16(mm4, mm7);				//mm4 = xabs(R0 - L0) - Alpha

	//xabs( R0 - R1) < Beta
	mm5 = _mm_sub_epi16(mm2, mm3);   
	mm7 = _mm_xor_si128(mm7, mm7);
	mm7 = _mm_sub_epi16(mm7, mm5);
	mm5 = _mm_max_epi16(mm7, mm5);				//xabs(R0 - R1)

	//xabs(L0 - L1) < Beta
	mm6 = _mm_sub_epi16(mm1, mm0);   
	mm7 = _mm_xor_si128(mm7, mm7);
	mm7 = _mm_sub_epi16(mm7, mm6);
	mm6 = _mm_max_epi16(mm7, mm6);				//xabs(L0 - L1)
	mm7 = _mm_set1_epi16(Beta);
	mm5 = _mm_sub_epi16(mm5, mm7);				//mm5 = xabs(R0 - R1) - Beta
	mm6 = _mm_sub_epi16(mm6, mm7);				//mm6 = xabs(L0 - L1) - Beta

	mm7 = _mm_xor_si128(mm7, mm7);
	mm4 = _mm_cmpgt_epi16(mm7, mm4);
	mm5 = _mm_cmpgt_epi16(mm7, mm5);
	mm6 = _mm_cmpgt_epi16(mm7, mm6);
	mm4 = _mm_and_si128(mm4, mm5);
	mm4 = _mm_and_si128(mm4, mm6);				//mm4 is deblocking mask

	//c0 = C0 + 1;
	mm7 = _mm_set1_epi16(C0+1);

	//dif = __fast_iclipX_X(c0, ( ((R0 - L0 ) << 2) + (L1 - R1) + 4) >> 3 );
	mm5 = _mm_sub_epi16(mm2, mm1);
	mm5 = _mm_slli_epi16(mm5, 2);
	mm6 = _mm_sub_epi16(mm0, mm3);
	mm5 = _mm_add_epi16(mm5, mm6);
	mm5 = _mm_add_epi16(mm5, n4);
	mm5 = _mm_srai_epi16(mm5, 3);
	mm5 = _mm_min_epi16(mm5, mm7);
	mm6 = _mm_xor_si128(mm6, mm6);
	mm6 = _mm_sub_epi16(mm6, mm7);
	mm5 = _mm_max_epi16(mm5, mm6);				//mm5 = diff

	//SrcPtrQ[-1] = CLIP0_255(L0 + dif) ;
	mm6 = _mm_add_epi16(mm1, mm5);
	mm1 = _mm_or_si128(_mm_and_si128(mm6, mm4), _mm_and_si128(mm1, _mm_xor_si128(mm4, nff)));	

	//SrcPtrQ[0] = CLIP0_255(R0 - dif) ;
	mm6 = _mm_sub_epi16(mm2, mm5);
	mm2 = _mm_or_si128(_mm_and_si128(mm6, mm4), _mm_and_si128(mm2, _mm_xor_si128(mm4, nff)));

	//transpose
	mm4 = _mm_unpackhi_epi32(mm0,mm1);
	mm5 = _mm_unpacklo_epi32(mm0,mm1);
	mm6 = _mm_unpackhi_epi32(mm2,mm3);
	mm7 = _mm_unpacklo_epi32(mm2,mm3);		
	mm0 = _mm_unpacklo_epi64(mm5,mm7);	
	mm1 = _mm_unpackhi_epi64(mm5,mm7);	
	mm2 = _mm_unpacklo_epi64(mm4,mm6);
	mm3 = _mm_unpackhi_epi64(mm4,mm6);

	//save to L0 and R0
	mm7 = _mm_xor_si128(mm7, mm7);
	mm0 = _mm_packus_epi16(mm0, mm7);
	mm1 = _mm_packus_epi16(mm1, mm7);

	//restore u or v
	mm0 = _mm_and_si128(mm0, msk_0);
	mm7 = _mm_and_si128(tmp_L1, msk_1);
	mm0 = _mm_add_epi8(mm0, mm7);

	mm1 = _mm_and_si128(mm1, msk_0);
	mm7 = _mm_and_si128(tmp_L2, msk_1);
	mm1 = _mm_add_epi8(mm1, mm7);

	_mm_storel_epi64((__m128i*)Line1, mm0);
	_mm_storel_epi64((__m128i*)Line2, mm1);
}

void Deblock_chroma_v_p8_1_mmx(imgpel* SrcPtrQ, int dpel, int Alpha, int Beta, int C0, int pNum)
{
	int dpel2 = (dpel<<1), dpel3 = (dpel<<1) + dpel;
	const __m64 n4 = _mm_set1_pi16((short) 4), nff = _mm_set1_pi16((short) 0xffff);
	__m64 mm0, mm1, mm2, mm3, mm4, mm5, mm6, mm7;
	__m64 tmp_L1, tmp_L2, tmp_L3, tmp_L4, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp, tmpp;
	const __m64 msk_0 = _mm_set1_pi16((short) 0x00ff);
	const __m64 msk_1 = _mm_set1_pi16((short) 0xff00);


	//L1  = SrcPtrQ[-2] ;
	//L0  = SrcPtrQ[-1] ;
	//R0  = SrcPtrQ[ 0] ;	  
	//R1  = SrcPtrQ[ 1] ;
	DWORD* Line1 = (DWORD*) (SrcPtrQ - 4);
	DWORD* Line2 = (DWORD*) (SrcPtrQ - 4 + dpel);
	DWORD* Line3 = (DWORD*) (SrcPtrQ - 4 + dpel2);
	DWORD* Line4 = (DWORD*) (SrcPtrQ - 4 + dpel3);

	mm7 = _mm_setzero_si64();
	mm0 = *((__m64*)Line1); 
	mm1 = *((__m64*)Line2);
	mm2 = *((__m64*)Line3);
	mm3 = *((__m64*)Line4);

	tmp_L1 = mm0; // 0a 0b 0c 0d 0e 0f 0g 0h
	tmp_L2 = mm1; // 1a 1b 1c 1d 1e 1f 1g 1h
	tmp_L3 = mm2; // 2a 2b 2c 2d 2e 2f 2g 2h
	tmp_L4 = mm3; // 3a 3b 3c 3d 3e 3f 3g 3h

  // 1
	mm0 = _mm_unpacklo_pi8(mm0, mm7); // 0a 0 0b 0 0c 0 0d 0
	mm1 = _mm_unpacklo_pi8(mm1, mm7);	// 1a 0 1b 0 1c 0 1d 0
	mm2 = _mm_unpacklo_pi8(mm2, mm7);	// 2a 0 2b 0 2c 0 2d 0		                    
	mm3 = _mm_unpacklo_pi8(mm3, mm7); // 3a 0 3b 0 3c 0 3d 0 

	tmp1 = mm0;
	tmp2 = mm1;
	tmp3 = mm2;
	tmp4 = mm3;
	tmpp =  _mm_unpacklo_pi32(mm2,mm3);
	
	// 2
	mm0 = *((__m64*)Line1); 
	mm1 = *((__m64*)Line2);
	mm2 = *((__m64*)Line3);
	mm3 = *((__m64*)Line4);

	mm0 = _mm_unpackhi_pi8(mm0, mm7); // 0e 0 0f 0 0g 0 0h 0
	mm1 = _mm_unpackhi_pi8(mm1, mm7);	// 1e 0 1f 0 1g 0 1h 0
	mm2 = _mm_unpackhi_pi8(mm2, mm7);	// 2e 0 2f 0 2g 0 2h 0		                    
	mm3 = _mm_unpackhi_pi8(mm3, mm7); // 3e 0 3f 0 3g 0 3h 0 
	tmp =  _mm_unpacklo_pi32(mm2,mm3);

	//transpose
	tmp5 = _mm_unpacklo_pi32(tmp1,tmp2); // 0a 0 0b 0 1a 0 1b 0 : mm0
	tmp6 = _mm_unpacklo_pi32(tmp3,tmp4); // mm0'
	tmp7 = _mm_unpackhi_pi32(tmp1,tmp2); // mm1
	tmp8 = _mm_unpackhi_pi32(tmp3,tmp4); // mm1'
	tmp1 = _mm_unpacklo_pi32(mm0,mm1); // mm2
	tmp2 = _mm_unpacklo_pi32(mm2,mm3); // mm2'
	tmp3 = _mm_unpackhi_pi32(mm0,mm1); // mm3
	tmp4 = _mm_unpackhi_pi32(mm2,mm3); // mm3'

	// 1 
	mm0 = tmp5;
	mm1 = tmp7;
	mm2 = tmp1;
	mm3 = tmp3;
	mm7 = tmpp;
	
	
	//xabs(R0 - L0 ) < Alpha
	mm4 = _mm_sub_pi16(mm2, mm1);
	mm7 = _mm_xor_si64(mm7, mm7);
	mm7 = _mm_sub_pi16(mm7, mm4);
	mm4 = _mm_max_pi16(mm7, mm4);				//xabs(R0 - L0)
	mm7 = _mm_set1_pi16(Alpha);
	mm4 = _mm_sub_pi16(mm4, mm7);				//mm4 = xabs(R0 - L0) - Alpha

	//xabs( R0 - R1) < Beta
	mm5 = _mm_sub_pi16(mm2, mm3);   
	mm7 = _mm_xor_si64(mm7, mm7);
	mm7 = _mm_sub_pi16(mm7, mm5);
	mm5 = _mm_max_pi16(mm7, mm5);				//xabs(R0 - R1)

	//xabs(L0 - L1) < Beta
	mm6 = _mm_sub_pi16(mm1, mm0);   
	mm7 = _mm_xor_si64(mm7, mm7);
	mm7 = _mm_sub_pi16(mm7, mm6);
	mm6 = _mm_max_pi16(mm7, mm6);				//xabs(L0 - L1)
	mm7 = _mm_set1_pi16(Beta);
	mm5 = _mm_sub_pi16(mm5, mm7);				//mm5 = xabs(R0 - R1) - Beta
	mm6 = _mm_sub_pi16(mm6, mm7);				//mm6 = xabs(L0 - L1) - Beta

	mm7 = _mm_xor_si64(mm7, mm7);
	mm4 = _mm_cmpgt_pi16(mm7, mm4);
	mm5 = _mm_cmpgt_pi16(mm7, mm5);
	mm6 = _mm_cmpgt_pi16(mm7, mm6);
	mm4 = _mm_and_si64(mm4, mm5);
	mm4 = _mm_and_si64(mm4, mm6);				//mm4 is deblocking mask

	//c0 = C0 + 1;
	mm7 = _mm_set1_pi16(C0+1);

	//dif = __fast_iclipX_X(c0, ( ((R0 - L0 ) << 2) + (L1 - R1) + 4) >> 3 );
	mm5 = _mm_sub_pi16(mm2, mm1);
	mm5 = _mm_slli_pi16(mm5, 2);
	mm6 = _mm_sub_pi16(mm0, mm3);
	mm5 = _mm_add_pi16(mm5, mm6);
	mm5 = _mm_add_pi16(mm5, n4);
	mm5 = _mm_srai_pi16(mm5, 3);
	mm5 = _mm_min_pi16(mm5, mm7);
	mm6 = _mm_xor_si64(mm6, mm6);
	mm6 = _mm_sub_pi16(mm6, mm7);
	mm5 = _mm_max_pi16(mm5, mm6);				//mm5 = diff

	//SrcPtrQ[-1] = CLIP0_255(L0 + dif) ;
	mm6 = _mm_add_pi16(mm1, mm5);
	mm1 = _mm_or_si64(_mm_and_si64(mm6, mm4), _mm_and_si64(mm1, _mm_xor_si64(mm4, nff)));	

	//SrcPtrQ[0] = CLIP0_255(R0 - dif) ;
	mm6 = _mm_sub_pi16(mm2, mm5);
	mm2 = _mm_or_si64(_mm_and_si64(mm6, mm4), _mm_and_si64(mm2, _mm_xor_si64(mm4, nff)));

	tmp5 = mm0;
	tmp7 = mm1;
	tmp1 = mm2;
	tmp3 = mm3;

	// 2
	mm0 = tmp6;
	mm1 = tmp8;
	mm2 = tmp2;
	mm3 = tmp4;
	mm7 = tmp;
	
	//xabs(R0 - L0 ) < Alpha
	mm4 = _mm_sub_pi16(mm2, mm1);
	mm7 = _mm_xor_si64(mm7, mm7);
	mm7 = _mm_sub_pi16(mm7, mm4);
	mm4 = _mm_max_pi16(mm7, mm4);				//xabs(R0 - L0)
	mm7 = _mm_set1_pi16(Alpha);
	mm4 = _mm_sub_pi16(mm4, mm7);				//mm4 = xabs(R0 - L0) - Alpha

	//xabs( R0 - R1) < Beta
	mm5 = _mm_sub_pi16(mm2, mm3);   
	mm7 = _mm_xor_si64(mm7, mm7);
	mm7 = _mm_sub_pi16(mm7, mm5);
	mm5 = _mm_max_pi16(mm7, mm5);				//xabs(R0 - R1)

	//xabs(L0 - L1) < Beta
	mm6 = _mm_sub_pi16(mm1, mm0);   
	mm7 = _mm_xor_si64(mm7, mm7);
	mm7 = _mm_sub_pi16(mm7, mm6);
	mm6 = _mm_max_pi16(mm7, mm6);				//xabs(L0 - L1)
	mm7 = _mm_set1_pi16(Beta);
	mm5 = _mm_sub_pi16(mm5, mm7);				//mm5 = xabs(R0 - R1) - Beta
	mm6 = _mm_sub_pi16(mm6, mm7);				//mm6 = xabs(L0 - L1) - Beta

	mm7 = _mm_xor_si64(mm7, mm7);
	mm4 = _mm_cmpgt_pi16(mm7, mm4);
	mm5 = _mm_cmpgt_pi16(mm7, mm5);
	mm6 = _mm_cmpgt_pi16(mm7, mm6);
	mm4 = _mm_and_si64(mm4, mm5);
	mm4 = _mm_and_si64(mm4, mm6);				//mm4 is deblocking mask

	//c0 = C0 + 1;
	mm7 = _mm_set1_pi16(C0+1);

	//dif = __fast_iclipX_X(c0, ( ((R0 - L0 ) << 2) + (L1 - R1) + 4) >> 3 );
	mm5 = _mm_sub_pi16(mm2, mm1);
	mm5 = _mm_slli_pi16(mm5, 2);
	mm6 = _mm_sub_pi16(mm0, mm3);
	mm5 = _mm_add_pi16(mm5, mm6);
	mm5 = _mm_add_pi16(mm5, n4);
	mm5 = _mm_srai_pi16(mm5, 3);
	mm5 = _mm_min_pi16(mm5, mm7);
	mm6 = _mm_xor_si64(mm6, mm6);
	mm6 = _mm_sub_pi16(mm6, mm7);
	mm5 = _mm_max_pi16(mm5, mm6);				//mm5 = diff

	//SrcPtrQ[-1] = CLIP0_255(L0 + dif) ;
	mm6 = _mm_add_pi16(mm1, mm5);
	mm1 = _mm_or_si64(_mm_and_si64(mm6, mm4), _mm_and_si64(mm1, _mm_xor_si64(mm4, nff)));	

	//SrcPtrQ[0] = CLIP0_255(R0 - dif) ;
	mm6 = _mm_sub_pi16(mm2, mm5);
	mm2 = _mm_or_si64(_mm_and_si64(mm6, mm4), _mm_and_si64(mm2, _mm_xor_si64(mm4, nff)));

	// mm0: 0a 0b 0c 0d 0e 0f 0g 0h tmp5 mm0
	// mm1: 1a 1b 1c 1d 1e 1f 1g 1h tmp7 mm1
  // mm2: 2a 2b 2c 2d 2e 2f 2g 2h tmp1 mm2
  // mm3: 3a 3b 3c 3d 3e 3f 3g 3h tmp3 mm3
  
	//transpose
	mm4 = _mm_unpacklo_pi32(tmp5,tmp7); // 0a 0b 1a 1b :mm0
	mm5 = _mm_unpacklo_pi32(tmp1,tmp3); // mm0'
	mm6 = _mm_unpackhi_pi32(tmp5,tmp7); // mm1
	mm7 = _mm_unpackhi_pi32(tmp1,tmp3); // mm1'
	tmp1 = _mm_unpacklo_pi32(mm0,mm1); // mm2
	tmp2 = _mm_unpacklo_pi32(mm2,mm3); // mm2'
	tmp3 = _mm_unpackhi_pi32(mm0,mm1); // mm3
	tmp4 = _mm_unpackhi_pi32(mm2,mm3); // mm3'

	//save to L0 and R0
	mm0 = _mm_packs_pu16(mm4, mm5);
	mm1 = _mm_packs_pu16(mm6, mm7);
	mm2 = _mm_packs_pu16(tmp1, tmp2);
	mm3 = _mm_packs_pu16(tmp3, tmp4);

	//restore u or v
	mm0 = _mm_and_si64(mm0, msk_0);
	mm7 = _mm_and_si64(tmp_L1, msk_1);
	mm0 = _mm_add_pi8(mm0, mm7);

	mm1 = _mm_and_si64(mm1, msk_0);
	mm7 = _mm_and_si64(tmp_L2, msk_1);
	mm1 = _mm_add_pi8(mm1, mm7);

	mm2 = _mm_and_si64(mm2, msk_0);
	mm7 = _mm_and_si64(tmp_L3, msk_1);
	mm2 = _mm_add_pi8(mm2, mm7);

	mm3 = _mm_and_si64(mm3, msk_0);
	mm7 = _mm_and_si64(tmp_L4, msk_1);
	mm3 = _mm_add_pi8(mm3, mm7);

	*((__m64*)Line1) = mm0;
	*((__m64*)Line2) = mm1;
	*((__m64*)Line3) = mm2;
	*((__m64*)Line4) = mm3;
}
void Deblock_chroma_v_p8_1_sse2(imgpel* SrcPtrQ, int dpel, int Alpha, int Beta, int C0, int pNum)
{
	int dpel2 = (dpel<<1), dpel3 = (dpel<<1) + dpel;
	const __m128i n4 = _mm_set1_epi16((short) 4), nff = _mm_set1_epi16((short) 0xffff);
	__m128i mm0, mm1, mm2, mm3, mm4, mm5, mm6, mm7;
	__m128i tmp_L1, tmp_L2, tmp_L3, tmp_L4;
	const __m128i msk_0 = _mm_set1_epi16((short) 0x00ff);
	const __m128i msk_1 = _mm_set1_epi16((short) 0xff00);


	//L1  = SrcPtrQ[-2] ;
	//L0  = SrcPtrQ[-1] ;
	//R0  = SrcPtrQ[ 0] ;	  
	//R1  = SrcPtrQ[ 1] ;
	DWORD* Line1 = (DWORD*) (SrcPtrQ - 4);
	DWORD* Line2 = (DWORD*) (SrcPtrQ - 4 + dpel);
	DWORD* Line3 = (DWORD*) (SrcPtrQ - 4 + dpel2);
	DWORD* Line4 = (DWORD*) (SrcPtrQ - 4 + dpel3);

	mm7 = _mm_setzero_si128();
	mm0 = _mm_loadl_epi64((__m128i*)Line1);               
	mm1 = _mm_loadl_epi64((__m128i*)Line2);
	mm2 = _mm_loadl_epi64((__m128i*)Line3);
	mm3 = _mm_loadl_epi64((__m128i*)Line4);

	tmp_L1 = mm0;
	tmp_L2 = mm1;
	tmp_L3 = mm2;
	tmp_L4 = mm3;

	mm0 = _mm_unpacklo_epi8(mm0, mm7);
	mm1 = _mm_unpacklo_epi8(mm1, mm7);			
	mm2 = _mm_unpacklo_epi8(mm2, mm7);			                    
	mm3 = _mm_unpacklo_epi8(mm3, mm7);

	//transpose
	mm4 = _mm_unpackhi_epi32(mm0,mm1);
	mm5 = _mm_unpacklo_epi32(mm0,mm1);
	mm6 = _mm_unpackhi_epi32(mm2,mm3);
	mm7 = _mm_unpacklo_epi32(mm2,mm3);		
	mm0 = _mm_unpacklo_epi64(mm5,mm7);		//mm0 = L1	
	mm1 = _mm_unpackhi_epi64(mm5,mm7);		//mm1 = L0	
	mm2 = _mm_unpacklo_epi64(mm4,mm6);		//mm2 = R0
	mm3 = _mm_unpackhi_epi64(mm4,mm6);		//mm3 = R1	

	//xabs(R0 - L0 ) < Alpha
	mm4 = _mm_sub_epi16(mm2, mm1);
	mm7 = _mm_xor_si128(mm7, mm7);
	mm7 = _mm_sub_epi16(mm7, mm4);
	mm4 = _mm_max_epi16(mm7, mm4);				//xabs(R0 - L0)
	mm7 = _mm_set1_epi16(Alpha);
	mm4 = _mm_sub_epi16(mm4, mm7);				//mm4 = xabs(R0 - L0) - Alpha

	//xabs( R0 - R1) < Beta
	mm5 = _mm_sub_epi16(mm2, mm3);   
	mm7 = _mm_xor_si128(mm7, mm7);
	mm7 = _mm_sub_epi16(mm7, mm5);
	mm5 = _mm_max_epi16(mm7, mm5);				//xabs(R0 - R1)

	//xabs(L0 - L1) < Beta
	mm6 = _mm_sub_epi16(mm1, mm0);   
	mm7 = _mm_xor_si128(mm7, mm7);
	mm7 = _mm_sub_epi16(mm7, mm6);
	mm6 = _mm_max_epi16(mm7, mm6);				//xabs(L0 - L1)
	mm7 = _mm_set1_epi16(Beta);
	mm5 = _mm_sub_epi16(mm5, mm7);				//mm5 = xabs(R0 - R1) - Beta
	mm6 = _mm_sub_epi16(mm6, mm7);				//mm6 = xabs(L0 - L1) - Beta

	mm7 = _mm_xor_si128(mm7, mm7);
	mm4 = _mm_cmpgt_epi16(mm7, mm4);
	mm5 = _mm_cmpgt_epi16(mm7, mm5);
	mm6 = _mm_cmpgt_epi16(mm7, mm6);
	mm4 = _mm_and_si128(mm4, mm5);
	mm4 = _mm_and_si128(mm4, mm6);				//mm4 is deblocking mask

	//c0 = C0 + 1;
	mm7 = _mm_set1_epi16(C0+1);

	//dif = __fast_iclipX_X(c0, ( ((R0 - L0 ) << 2) + (L1 - R1) + 4) >> 3 );
	mm5 = _mm_sub_epi16(mm2, mm1);
	mm5 = _mm_slli_epi16(mm5, 2);
	mm6 = _mm_sub_epi16(mm0, mm3);
	mm5 = _mm_add_epi16(mm5, mm6);
	mm5 = _mm_add_epi16(mm5, n4);
	mm5 = _mm_srai_epi16(mm5, 3);
	mm5 = _mm_min_epi16(mm5, mm7);
	mm6 = _mm_xor_si128(mm6, mm6);
	mm6 = _mm_sub_epi16(mm6, mm7);
	mm5 = _mm_max_epi16(mm5, mm6);				//mm5 = diff

	//SrcPtrQ[-1] = CLIP0_255(L0 + dif) ;
	mm6 = _mm_add_epi16(mm1, mm5);
	mm1 = _mm_or_si128(_mm_and_si128(mm6, mm4), _mm_and_si128(mm1, _mm_xor_si128(mm4, nff)));	

	//SrcPtrQ[0] = CLIP0_255(R0 - dif) ;
	mm6 = _mm_sub_epi16(mm2, mm5);
	mm2 = _mm_or_si128(_mm_and_si128(mm6, mm4), _mm_and_si128(mm2, _mm_xor_si128(mm4, nff)));

	//transpose
	mm4 = _mm_unpackhi_epi32(mm0,mm1);
	mm5 = _mm_unpacklo_epi32(mm0,mm1);
	mm6 = _mm_unpackhi_epi32(mm2,mm3);
	mm7 = _mm_unpacklo_epi32(mm2,mm3);		
	mm0 = _mm_unpacklo_epi64(mm5,mm7);	
	mm1 = _mm_unpackhi_epi64(mm5,mm7);	
	mm2 = _mm_unpacklo_epi64(mm4,mm6);
	mm3 = _mm_unpackhi_epi64(mm4,mm6);

	//save to L0 and R0
	mm7 = _mm_xor_si128(mm7, mm7);
	mm0 = _mm_packus_epi16(mm0, mm7);
	mm1 = _mm_packus_epi16(mm1, mm7);
	mm2 = _mm_packus_epi16(mm2, mm7);
	mm3 = _mm_packus_epi16(mm3, mm7);

	//restore u or v
	mm0 = _mm_and_si128(mm0, msk_0);
	mm7 = _mm_and_si128(tmp_L1, msk_1);
	mm0 = _mm_add_epi8(mm0, mm7);

	mm1 = _mm_and_si128(mm1, msk_0);
	mm7 = _mm_and_si128(tmp_L2, msk_1);
	mm1 = _mm_add_epi8(mm1, mm7);

	mm2 = _mm_and_si128(mm2, msk_0);
	mm7 = _mm_and_si128(tmp_L3, msk_1);
	mm2 = _mm_add_epi8(mm2, mm7);

	mm3 = _mm_and_si128(mm3, msk_0);
	mm7 = _mm_and_si128(tmp_L4, msk_1);
	mm3 = _mm_add_epi8(mm3, mm7);

	_mm_storel_epi64((__m128i*)Line1, mm0);
	_mm_storel_epi64((__m128i*)Line2, mm1);
	_mm_storel_epi64((__m128i*)Line3, mm2);
	_mm_storel_epi64((__m128i*)Line4, mm3);
}

void Deblock_chromaUV_v_1_mmx(imgpel* SrcUV, int dpel, int Alpha, int Beta, int C0, int pNum)
{
	const __m64 n4 = _mm_set1_pi16((short) 4), nff = _mm_set1_pi16((short) 0xffff);
	__m64 mm0, mm1, mm2, mm3, mm4, mm5, mm6, mm7;
	__m64 tmp1, tmp2, tmp3, tmp4;

	//L1  = SrcPtrQ[-2] ;
	//L0  = SrcPtrQ[-1] ;
	//R0  = SrcPtrQ[ 0] ;	  
	//R1  = SrcPtrQ[ 1] ;
	DWORD* ULine1 = (DWORD*) (SrcUV - 4);
	DWORD* ULine2 = (DWORD*) (SrcUV - 4 + dpel);	

	mm6 = _mm_setzero_si64();
	mm7 = _mm_setzero_si64();
	mm0 = *((__m64*)ULine1); // 0a 0b 0c 0d 0e 0f 0g 0h 
	mm1 = *((__m64*)ULine2); // 1a 1b 1c 1d 1e 1f 1g 1h
	tmp1 = _mm_unpacklo_pi8(mm0, mm7); // 0a 0 0b 0 0c 0 0d 0
	tmp2 = _mm_unpacklo_pi8(mm1, mm7);	// 1a 0 1b 0 1c 0 1d 0
	mm2 = _mm_unpackhi_pi8(mm0, mm7); // 0e 0 0f 0 0g 0 0h 0
	mm3 = _mm_unpackhi_pi8(mm1, mm7); // 1e 0 1f 0 1g 0 1h 0
	
	mm0 = tmp1;
	mm1 = tmp2;

	//transpose
	mm4 = _mm_unpacklo_pi32(mm0,mm1);
	mm5 = _mm_unpackhi_pi32(mm0,mm1);		
	mm6 = _mm_unpacklo_pi32(mm2,mm3);	
	mm7 = _mm_unpackhi_pi32(mm2,mm3);		
	
  mm0 = mm4;    //mm0 = L1
  mm1 = mm5;	 	//mm1 = L0
	mm2 = mm6;		//mm2 = R0
	mm3 = mm7;		//mm3 = R1	

	//xabs(R0 - L0 ) < Alpha
	mm4 = _mm_sub_pi16(mm2, mm1);
	mm7 = _mm_xor_si64(mm7, mm7);
	mm7 = _mm_sub_pi16(mm7, mm4);
	mm4 = _mm_max_pi16(mm7, mm4);				//xabs(R0 - L0)
	mm7 = _mm_set1_pi16(Alpha);
	mm4 = _mm_sub_pi16(mm4, mm7);				//mm4 = xabs(R0 - L0) - Alpha

	//xabs( R0 - R1) < Beta
	mm5 = _mm_sub_pi16(mm2, mm3);   
	mm7 = _mm_xor_si64(mm7, mm7);
	mm7 = _mm_sub_pi16(mm7, mm5);
	mm5 = _mm_max_pi16(mm7, mm5);				//xabs(R0 - R1)

	//xabs(L0 - L1) < Beta
	mm6 = _mm_sub_pi16(mm1, mm0);   
	mm7 = _mm_xor_si64(mm7, mm7);
	mm7 = _mm_sub_pi16(mm7, mm6);
	mm6 = _mm_max_pi16(mm7, mm6);				//xabs(L0 - L1)
	mm7 = _mm_set1_pi16(Beta);
	mm5 = _mm_sub_pi16(mm5, mm7);				//mm5 = xabs(R0 - R1) - Beta
	mm6 = _mm_sub_pi16(mm6, mm7);				//mm6 = xabs(L0 - L1) - Beta

	mm7 = _mm_xor_si64(mm7, mm7);
	mm4 = _mm_cmpgt_pi16(mm7, mm4);
	mm5 = _mm_cmpgt_pi16(mm7, mm5);
	mm6 = _mm_cmpgt_pi16(mm7, mm6);
	mm4 = _mm_and_si64(mm4, mm5);
	mm4 = _mm_and_si64(mm4, mm6);				//mm4 is deblocking mask

	//c0 = C0 + 1;
	mm7 = _mm_set1_pi16(C0+1);

	//dif = __fast_iclipX_X(c0, ( ((R0 - L0 ) << 2) + (L1 - R1) + 4) >> 3 );
	mm5 = _mm_sub_pi16(mm2, mm1);
	mm5 = _mm_slli_pi16(mm5, 2);
	mm6 = _mm_sub_pi16(mm0, mm3);
	mm5 = _mm_add_pi16(mm5, mm6);
	mm5 = _mm_add_pi16(mm5, n4);
	mm5 = _mm_srai_pi16(mm5, 3);
	mm5 = _mm_min_pi16(mm5, mm7);
	mm6 = _mm_xor_si64(mm6, mm6);
	mm6 = _mm_sub_pi16(mm6, mm7);
	mm5 = _mm_max_pi16(mm5, mm6);				//mm5 = diff

	//SrcPtrQ[-1] = CLIP0_255(L0 + dif) ;
	mm6 = _mm_add_pi16(mm1, mm5);
	mm1 = _mm_or_si64(_mm_and_si64(mm6, mm4), _mm_and_si64(mm1, _mm_xor_si64(mm4, nff)));	

	//SrcPtrQ[0] = CLIP0_255(R0 - dif) ;
	mm6 = _mm_sub_pi16(mm2, mm5);
	mm2 = _mm_or_si64(_mm_and_si64(mm6, mm4), _mm_and_si64(mm2, _mm_xor_si64(mm4, nff)));

	tmp1 = mm0;
	tmp2 = mm1;
	tmp3 = mm2;
	tmp4 = mm3;

	//transpose
	mm5 = _mm_unpacklo_pi32(tmp1,tmp2); //1133
	mm6 = _mm_unpackhi_pi32(tmp1,tmp2); //1133
	mm7 = _mm_unpacklo_pi32(tmp3,tmp4); //5577
	mm4 = _mm_unpackhi_pi32(tmp3,tmp4); //5577	

	//save to L0 and R0
	mm0 = _mm_packs_pu16(mm5, mm7);
	mm1 = _mm_packs_pu16(mm6, mm4);

	*((__m64*)ULine1) = mm0;
	*((__m64*)ULine2) = mm1;
}

void Deblock_chromaUV_v_1_sse2(imgpel* SrcUV, int dpel, int Alpha, int Beta, int C0, int pNum)
{
	const __m128i n4 = _mm_set1_epi16((short) 4), nff = _mm_set1_epi16((short) 0xffff);
	__m128i mm0, mm1, mm2, mm3, mm4, mm5, mm6, mm7;

	//L1  = SrcPtrQ[-2] ;
	//L0  = SrcPtrQ[-1] ;
	//R0  = SrcPtrQ[ 0] ;	  
	//R1  = SrcPtrQ[ 1] ;
	DWORD* ULine1 = (DWORD*) (SrcUV - 4);
	DWORD* ULine2 = (DWORD*) (SrcUV - 4 + dpel);	

	mm6 = _mm_setzero_si128();
	mm7 = _mm_setzero_si128();
	mm0 = _mm_loadl_epi64((__m128i*)ULine1);               
	mm1 = _mm_loadl_epi64((__m128i*)ULine2);
	//mm2 = _mm_setzero_si128();
	//mm3 = _mm_setzero_si128();
	mm0 = _mm_unpacklo_epi8(mm0, mm7);
	mm1 = _mm_unpacklo_epi8(mm1, mm7);			               
	//mm2 = _mm_unpacklo_epi8(mm2, mm7);			    	               
	//mm3 = _mm_unpacklo_epi8(mm3, mm7);

	//transpose
	mm4 = _mm_unpackhi_epi32(mm0,mm1);
	mm5 = _mm_unpacklo_epi32(mm0,mm1);
	//mm6 = _mm_unpackhi_epi32(mm2,mm3);
	//mm7 = _mm_unpacklo_epi32(mm2,mm3);		
	mm0 = _mm_unpacklo_epi64(mm5,mm7);		//mm0 = L1	
	mm1 = _mm_unpackhi_epi64(mm5,mm7);		//mm1 = L0	
	mm2 = _mm_unpacklo_epi64(mm4,mm6);		//mm2 = R0
	mm3 = _mm_unpackhi_epi64(mm4,mm6);		//mm3 = R1	

	//xabs(R0 - L0 ) < Alpha
	mm4 = _mm_sub_epi16(mm2, mm1);
	mm7 = _mm_xor_si128(mm7, mm7);
	mm7 = _mm_sub_epi16(mm7, mm4);
	mm4 = _mm_max_epi16(mm7, mm4);				//xabs(R0 - L0)
	mm7 = _mm_set1_epi16(Alpha);
	mm4 = _mm_sub_epi16(mm4, mm7);				//mm4 = xabs(R0 - L0) - Alpha

	//xabs( R0 - R1) < Beta
	mm5 = _mm_sub_epi16(mm2, mm3);   
	mm7 = _mm_xor_si128(mm7, mm7);
	mm7 = _mm_sub_epi16(mm7, mm5);
	mm5 = _mm_max_epi16(mm7, mm5);				//xabs(R0 - R1)

	//xabs(L0 - L1) < Beta
	mm6 = _mm_sub_epi16(mm1, mm0);   
	mm7 = _mm_xor_si128(mm7, mm7);
	mm7 = _mm_sub_epi16(mm7, mm6);
	mm6 = _mm_max_epi16(mm7, mm6);				//xabs(L0 - L1)
	mm7 = _mm_set1_epi16(Beta);
	mm5 = _mm_sub_epi16(mm5, mm7);				//mm5 = xabs(R0 - R1) - Beta
	mm6 = _mm_sub_epi16(mm6, mm7);				//mm6 = xabs(L0 - L1) - Beta

	mm7 = _mm_xor_si128(mm7, mm7);
	mm4 = _mm_cmpgt_epi16(mm7, mm4);
	mm5 = _mm_cmpgt_epi16(mm7, mm5);
	mm6 = _mm_cmpgt_epi16(mm7, mm6);
	mm4 = _mm_and_si128(mm4, mm5);
	mm4 = _mm_and_si128(mm4, mm6);				//mm4 is deblocking mask

	//c0 = C0 + 1;
	mm7 = _mm_set1_epi16(C0+1);

	//dif = __fast_iclipX_X(c0, ( ((R0 - L0 ) << 2) + (L1 - R1) + 4) >> 3 );
	mm5 = _mm_sub_epi16(mm2, mm1);
	mm5 = _mm_slli_epi16(mm5, 2);
	mm6 = _mm_sub_epi16(mm0, mm3);
	mm5 = _mm_add_epi16(mm5, mm6);
	mm5 = _mm_add_epi16(mm5, n4);
	mm5 = _mm_srai_epi16(mm5, 3);
	mm5 = _mm_min_epi16(mm5, mm7);
	mm6 = _mm_xor_si128(mm6, mm6);
	mm6 = _mm_sub_epi16(mm6, mm7);
	mm5 = _mm_max_epi16(mm5, mm6);				//mm5 = diff

	//SrcPtrQ[-1] = CLIP0_255(L0 + dif) ;
	mm6 = _mm_add_epi16(mm1, mm5);
	mm1 = _mm_or_si128(_mm_and_si128(mm6, mm4), _mm_and_si128(mm1, _mm_xor_si128(mm4, nff)));	

	//SrcPtrQ[0] = CLIP0_255(R0 - dif) ;
	mm6 = _mm_sub_epi16(mm2, mm5);
	mm2 = _mm_or_si128(_mm_and_si128(mm6, mm4), _mm_and_si128(mm2, _mm_xor_si128(mm4, nff)));

	//transpose
	mm4 = _mm_unpackhi_epi32(mm0,mm1);
	mm5 = _mm_unpacklo_epi32(mm0,mm1);
	mm6 = _mm_unpackhi_epi32(mm2,mm3);
	mm7 = _mm_unpacklo_epi32(mm2,mm3);		
	mm0 = _mm_unpacklo_epi64(mm5,mm7);	
	mm1 = _mm_unpackhi_epi64(mm5,mm7);	
	//mm2 = _mm_unpacklo_epi64(mm4,mm6);
	//mm3 = _mm_unpackhi_epi64(mm4,mm6);

	//save to L0 and R0
	mm7 = _mm_xor_si128(mm7, mm7);
	mm0 = _mm_packus_epi16(mm0, mm7);
	mm1 = _mm_packus_epi16(mm1, mm7);
	//mm2 = _mm_packus_epi16(mm2, mm7);
	//mm3 = _mm_packus_epi16(mm3, mm7);

	_mm_storel_epi64((__m128i*)ULine1, mm0);
	_mm_storel_epi64((__m128i*)ULine2, mm1);
}

void Deblock_chromaUV_v_p8_1_mmx(imgpel* SrcUV, int dpel, int Alpha, int Beta, int C0, int pNum)
{
	int dpel2 = (dpel<<1), dpel3 = (dpel<<1) + dpel;
	const __m64 n4 = _mm_set1_pi16((short) 4), nff = _mm_set1_pi16((short) 0xffff);
	__m64 mm0, mm1, mm2, mm3, mm4, mm5, mm6, mm7;
	__m64 tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8;
	imgpel* SrcPtrQ = SrcUV;

	//L1  = SrcPtrQ[-2] ;
	//L0  = SrcPtrQ[-1] ;
	//R0  = SrcPtrQ[ 0] ;	  
	//R1  = SrcPtrQ[ 1] ;
	DWORD* Line1 = (DWORD*) (SrcPtrQ - 4);
	DWORD* Line2 = (DWORD*) (SrcPtrQ - 4 + dpel);
	DWORD* Line3 = (DWORD*) (SrcPtrQ - 4 + dpel2);
	DWORD* Line4 = (DWORD*) (SrcPtrQ - 4 + dpel3);

	mm7 = _mm_setzero_si64();
	mm0 = *((__m64*)Line1);
	mm1 = *((__m64*)Line2);
	mm2 = *((__m64*)Line3);
	mm3 = *((__m64*)Line4);

	tmp1 = _mm_unpackhi_pi8(mm0, mm7);	//0e 0 0f 0 0g 0 0h 0 
	tmp2 = _mm_unpackhi_pi8(mm1, mm7);	//1e 0 1f 0 1g 0 1h 0 
	tmp3 = _mm_unpackhi_pi8(mm2, mm7);	//2e 0 2f 0 2g 0 2h 0		                    
	tmp4 = _mm_unpackhi_pi8(mm3, mm7);  //3e 0 3f 0 3g 0 3h 0 

	mm0 = _mm_unpacklo_pi8(mm0, mm7);	//0a 0 0b 0 0c 0 0d 0
	mm1 = _mm_unpacklo_pi8(mm1, mm7);	//1a 0 1b 0 1c 0 1d 0		
	mm2 = _mm_unpacklo_pi8(mm2, mm7);	//2a 0 2b 0 2c 0 2d 0 		                    
	mm3 = _mm_unpacklo_pi8(mm3, mm7); //3a 0 3b 0 3c 0 3d 0

	//transpose
	mm4 = _mm_unpacklo_pi32(tmp1,tmp2);	//mm0 = L1	         mm2
	mm5 = _mm_unpacklo_pi32(tmp3,tmp4);	//mm1 = L0	         mm2'
	mm6 = _mm_unpackhi_pi32(tmp1,tmp2);	//mm2 = R0           mm3
	mm7 = _mm_unpackhi_pi32(tmp3,tmp4);	//mm3 = R1	         mm3'
	tmp5 = _mm_unpacklo_pi32(mm0,mm1); //0a 0 0b 0 1a 0 1b 0  mm0
	tmp6 = _mm_unpacklo_pi32(mm2,mm3); //2a 0 2b 0 3a 0 3b 0  mm0'
	tmp7 = _mm_unpackhi_pi32(mm0,mm1); //0c 0 0d 0 1c 0 1d 0  mm1 
	tmp8 = _mm_unpackhi_pi32(mm2,mm3);	//2c 0 2d 0 3c 0 3d 0  mm1'

	tmp1 = tmp6; 
	tmp2 = tmp8; 
  tmp3 = mm5; 
  tmp4 = mm7;
  
  //  1
  mm0 = tmp5;
  mm1 = tmp7;
  mm2 = mm4;
	mm3 = mm6;
	mm7 = tmp1;

	//xabs(R0 - L0 ) < Alpha
	mm4 = _mm_sub_pi16(mm2, mm1);
	mm7 = _mm_xor_si64(mm7, mm7);
	mm5 = _mm_sub_pi16(mm7, mm4);
	mm4 = _mm_max_pi16(mm5, mm4);				//xabs(R0 - L0)
	mm5 = _mm_set1_pi16(Alpha);
	mm4 = _mm_cmpgt_pi16(mm5, mm4);				//mm4 = xabs(R0 - L0) - Alpha

	//xabs( R0 - R1) < Beta
	mm5 = _mm_sub_pi16(mm2, mm3);   
	mm6 = _mm_sub_pi16(mm7, mm5);
	mm5 = _mm_max_pi16(mm6, mm5);				//xabs(R0 - R1)

	//xabs(L0 - L1) < Beta
	mm6 = _mm_sub_pi16(mm1, mm0);   
	mm7 = _mm_sub_pi16(mm7, mm6);
	mm6 = _mm_max_pi16(mm7, mm6);				//xabs(L0 - L1)

	mm7 = _mm_set1_pi16(Beta);
	mm5 = _mm_cmpgt_pi16(mm7, mm5);				//mm5 = xabs(R0 - R1) - Beta
	mm6 = _mm_cmpgt_pi16(mm7, mm6);				//mm6 = xabs(L0 - L1) - Beta		

	mm4 = _mm_and_si64(mm4, mm5);
	mm4 = _mm_and_si64(mm4, mm6);				//mm4 is deblocking mask

	//c0 = C0 + 1;
	mm7 = _mm_set1_pi16(C0+1);

	//dif = __fast_iclipX_X(c0, ( ((R0 - L0 ) << 2) + (L1 - R1) + 4) >> 3 );
	mm5 = _mm_sub_pi16(mm2, mm1);
	mm5 = _mm_slli_pi16(mm5, 2);
	mm6 = _mm_sub_pi16(mm0, mm3);
	mm5 = _mm_add_pi16(mm5, mm6);
	mm5 = _mm_add_pi16(mm5, n4);
	mm5 = _mm_srai_pi16(mm5, 3);
	mm5 = _mm_min_pi16(mm5, mm7);
	mm6 = _mm_xor_si64(mm6, mm6);
	mm6 = _mm_sub_pi16(mm6, mm7);
	mm5 = _mm_max_pi16(mm5, mm6);				//mm5 = diff

	//SrcPtrQ[-1] = CLIP0_255(L0 + dif) ;
	mm6 = _mm_add_pi16(mm1, mm5);
	mm1 = _mm_or_si64(_mm_and_si64(mm6, mm4), _mm_and_si64(mm1, _mm_xor_si64(mm4, nff)));	

	//SrcPtrQ[0] = CLIP0_255(R0 - dif) ;
	mm6 = _mm_sub_pi16(mm2, mm5);
	mm2 = _mm_or_si64(_mm_and_si64(mm6, mm4), _mm_and_si64(mm2, _mm_xor_si64(mm4, nff)));

	tmp5 = mm0;
	tmp6 = mm1;
	tmp7 = mm2;
	tmp8 = mm3;

	//  2
  mm0 = tmp1;
  mm1 = tmp2;
  mm2 = tmp3;
	mm3 = tmp4;
	mm7 = tmp2;

	//xabs(R0 - L0 ) < Alpha
	mm4 = _mm_sub_pi16(mm2, mm1);
	mm7 = _mm_xor_si64(mm7, mm7);
	mm5 = _mm_sub_pi16(mm7, mm4);
	mm4 = _mm_max_pi16(mm5, mm4);				//xabs(R0 - L0)
	mm5 = _mm_set1_pi16(Alpha);
	mm4 = _mm_cmpgt_pi16(mm5, mm4);				//mm4 = xabs(R0 - L0) - Alpha

	//xabs( R0 - R1) < Beta
	mm5 = _mm_sub_pi16(mm2, mm3);   
	mm6 = _mm_sub_pi16(mm7, mm5);
	mm5 = _mm_max_pi16(mm6, mm5);				//xabs(R0 - R1)

	//xabs(L0 - L1) < Beta
	mm6 = _mm_sub_pi16(mm1, mm0);   
	mm7 = _mm_sub_pi16(mm7, mm6);
	mm6 = _mm_max_pi16(mm7, mm6);				//xabs(L0 - L1)

	mm7 = _mm_set1_pi16(Beta);
	mm5 = _mm_cmpgt_pi16(mm7, mm5);				//mm5 = xabs(R0 - R1) - Beta
	mm6 = _mm_cmpgt_pi16(mm7, mm6);				//mm6 = xabs(L0 - L1) - Beta		

	mm4 = _mm_and_si64(mm4, mm5);
	mm4 = _mm_and_si64(mm4, mm6);				//mm4 is deblocking mask

	//c0 = C0 + 1;
	mm7 = _mm_set1_pi16(C0+1);

	//dif = __fast_iclipX_X(c0, ( ((R0 - L0 ) << 2) + (L1 - R1) + 4) >> 3 );
	mm5 = _mm_sub_pi16(mm2, mm1);
	mm5 = _mm_slli_pi16(mm5, 2);
	mm6 = _mm_sub_pi16(mm0, mm3);
	mm5 = _mm_add_pi16(mm5, mm6);
	mm5 = _mm_add_pi16(mm5, n4);
	mm5 = _mm_srai_pi16(mm5, 3);
	mm5 = _mm_min_pi16(mm5, mm7);
	mm6 = _mm_xor_si64(mm6, mm6);
	mm6 = _mm_sub_pi16(mm6, mm7);
	mm5 = _mm_max_pi16(mm5, mm6);				//mm5 = diff

	//SrcPtrQ[-1] = CLIP0_255(L0 + dif) ;
	mm6 = _mm_add_pi16(mm1, mm5);
	mm1 = _mm_or_si64(_mm_and_si64(mm6, mm4), _mm_and_si64(mm1, _mm_xor_si64(mm4, nff)));	

	//SrcPtrQ[0] = CLIP0_255(R0 - dif) ;
	mm6 = _mm_sub_pi16(mm2, mm5);
	mm2 = _mm_or_si64(_mm_and_si64(mm6, mm4), _mm_and_si64(mm2, _mm_xor_si64(mm4, nff)));

	//transpose
	mm4 = _mm_unpacklo_pi32(tmp5,tmp6); //mm0
	mm5 = _mm_unpacklo_pi32(tmp7,tmp8); //mm0'
	mm6 = _mm_unpackhi_pi32(tmp5,tmp6); //mm1
	mm7 = _mm_unpackhi_pi32(tmp7,tmp8); //mm1'		
	tmp5 = _mm_unpacklo_pi32(mm0,mm1);   //mm2	
	tmp6 = _mm_unpacklo_pi32(mm2,mm3);	 //mm2'
	tmp7 = _mm_unpackhi_pi32(mm0,mm1);	 //mm3	
	tmp8 = _mm_unpackhi_pi32(mm2,mm3);   //mm3'

	//save to L0 and R0
	mm0 = _mm_packs_pu16(mm4, mm5);
	mm1 = _mm_packs_pu16(mm6, mm7);
	mm2 = _mm_packs_pu16(tmp5, tmp6);
	mm3 = _mm_packs_pu16(tmp7, tmp8);

	*((__m64*)Line1) = mm0;
	*((__m64*)Line2) = mm1;
	*((__m64*)Line3) = mm2;
	*((__m64*)Line4) = mm3;
}

void Deblock_chromaUV_v_p8_1_sse2(imgpel* SrcUV, int dpel, int Alpha, int Beta, int C0, int pNum)
{
	int dpel2 = (dpel<<1), dpel3 = (dpel<<1) + dpel;
	const __m128i n4 = _mm_set1_epi16((short) 4), nff = _mm_set1_epi16((short) 0xffff);
	__m128i mm0, mm1, mm2, mm3, mm4, mm5, mm6, mm7;
	imgpel* SrcPtrQ = SrcUV;

	//L1  = SrcPtrQ[-2] ;
	//L0  = SrcPtrQ[-1] ;
	//R0  = SrcPtrQ[ 0] ;	  
	//R1  = SrcPtrQ[ 1] ;
	DWORD* Line1 = (DWORD*) (SrcPtrQ - 4);
	DWORD* Line2 = (DWORD*) (SrcPtrQ - 4 + dpel);
	DWORD* Line3 = (DWORD*) (SrcPtrQ - 4 + dpel2);
	DWORD* Line4 = (DWORD*) (SrcPtrQ - 4 + dpel3);

	mm7 = _mm_setzero_si128();
	mm0 = _mm_loadl_epi64((__m128i*)Line1);               
	mm1 = _mm_loadl_epi64((__m128i*)Line2);
	mm2 = _mm_loadl_epi64((__m128i*)Line3);
	mm3 = _mm_loadl_epi64((__m128i*)Line4);
	mm0 = _mm_unpacklo_epi8(mm0, mm7);	//12345678
	mm1 = _mm_unpacklo_epi8(mm1, mm7);	//12345678		
	mm2 = _mm_unpacklo_epi8(mm2, mm7);	//12345678		                    
	mm3 = _mm_unpacklo_epi8(mm3, mm7);  //12345678

	//transpose
	mm4 = _mm_unpackhi_epi32(mm0,mm1);
	mm5 = _mm_unpacklo_epi32(mm0,mm1);
	mm6 = _mm_unpackhi_epi32(mm2,mm3);
	mm7 = _mm_unpacklo_epi32(mm2,mm3);		
	mm0 = _mm_unpacklo_epi64(mm5,mm7);		//mm0 = L1	
	mm1 = _mm_unpackhi_epi64(mm5,mm7);		//mm1 = L0	
	mm2 = _mm_unpacklo_epi64(mm4,mm6);		//mm2 = R0
	mm3 = _mm_unpackhi_epi64(mm4,mm6);		//mm3 = R1	

	//xabs(R0 - L0 ) < Alpha
	mm4 = _mm_sub_epi16(mm2, mm1);
	mm7 = _mm_xor_si128(mm7, mm7);
	mm5 = _mm_sub_epi16(mm7, mm4);
	mm4 = _mm_max_epi16(mm5, mm4);				//xabs(R0 - L0)
	mm5 = _mm_set1_epi16(Alpha);
	mm4 = _mm_cmpgt_epi16(mm5, mm4);				//mm4 = xabs(R0 - L0) - Alpha

	//xabs( R0 - R1) < Beta
	mm5 = _mm_sub_epi16(mm2, mm3);   
	mm6 = _mm_sub_epi16(mm7, mm5);
	mm5 = _mm_max_epi16(mm6, mm5);				//xabs(R0 - R1)

	//xabs(L0 - L1) < Beta
	mm6 = _mm_sub_epi16(mm1, mm0);   
	mm7 = _mm_sub_epi16(mm7, mm6);
	mm6 = _mm_max_epi16(mm7, mm6);				//xabs(L0 - L1)

	mm7 = _mm_set1_epi16(Beta);
	mm5 = _mm_cmpgt_epi16(mm7, mm5);				//mm5 = xabs(R0 - R1) - Beta
	mm6 = _mm_cmpgt_epi16(mm7, mm6);				//mm6 = xabs(L0 - L1) - Beta		

	mm4 = _mm_and_si128(mm4, mm5);
	mm4 = _mm_and_si128(mm4, mm6);				//mm4 is deblocking mask

	//c0 = C0 + 1;
	mm7 = _mm_set1_epi16(C0+1);

	//dif = __fast_iclipX_X(c0, ( ((R0 - L0 ) << 2) + (L1 - R1) + 4) >> 3 );
	mm5 = _mm_sub_epi16(mm2, mm1);
	mm5 = _mm_slli_epi16(mm5, 2);
	mm6 = _mm_sub_epi16(mm0, mm3);
	mm5 = _mm_add_epi16(mm5, mm6);
	mm5 = _mm_add_epi16(mm5, n4);
	mm5 = _mm_srai_epi16(mm5, 3);
	mm5 = _mm_min_epi16(mm5, mm7);
	mm6 = _mm_xor_si128(mm6, mm6);
	mm6 = _mm_sub_epi16(mm6, mm7);
	mm5 = _mm_max_epi16(mm5, mm6);				//mm5 = diff

	//SrcPtrQ[-1] = CLIP0_255(L0 + dif) ;
	mm6 = _mm_add_epi16(mm1, mm5);
	mm1 = _mm_or_si128(_mm_and_si128(mm6, mm4), _mm_and_si128(mm1, _mm_xor_si128(mm4, nff)));	

	//SrcPtrQ[0] = CLIP0_255(R0 - dif) ;
	mm6 = _mm_sub_epi16(mm2, mm5);
	mm2 = _mm_or_si128(_mm_and_si128(mm6, mm4), _mm_and_si128(mm2, _mm_xor_si128(mm4, nff)));

	//transpose
	mm4 = _mm_unpackhi_epi32(mm0,mm1);
	mm5 = _mm_unpacklo_epi32(mm0,mm1);
	mm6 = _mm_unpackhi_epi32(mm2,mm3);
	mm7 = _mm_unpacklo_epi32(mm2,mm3);		
	mm0 = _mm_unpacklo_epi64(mm5,mm7);	
	mm1 = _mm_unpackhi_epi64(mm5,mm7);	
	mm2 = _mm_unpacklo_epi64(mm4,mm6);
	mm3 = _mm_unpackhi_epi64(mm4,mm6);

	//save to L0 and R0
	mm7 = _mm_xor_si128(mm7, mm7);
	mm0 = _mm_packus_epi16(mm0, mm7);
	mm1 = _mm_packus_epi16(mm1, mm7);
	mm2 = _mm_packus_epi16(mm2, mm7);
	mm3 = _mm_packus_epi16(mm3, mm7);

	_mm_storel_epi64((__m128i*)Line1, mm0);
	_mm_storel_epi64((__m128i*)Line2, mm1);
	_mm_storel_epi64((__m128i*)Line3, mm2);
	_mm_storel_epi64((__m128i*)Line4, mm3);
}

void Deblock_luma_h_1_mmx(imgpel* SrcPtrQ, int inc, int Alpha, int Beta, int C0, int pNum)
{
	int inc2 = (inc<<1), inc3 = (inc<<1) + inc;
	const __m64 n4 = _mm_set1_pi16((short) 4), nff = _mm_set1_pi16((short) 0xffff);
	__m64 mm0, mm1, mm2, mm3, mm4, mm5, mm6, mm7;
	__m64 deblock_mask, ap_mask, aq_mask;

	//L2  = SrcPtrQ[-inc3];
	//L1  = SrcPtrQ[-inc2];
	//L0  = SrcPtrQ[-inc];
	//R0  = SrcPtrQ[0];
	//R1  = SrcPtrQ[ inc];
	//R2  = SrcPtrQ[ inc2];
	DWORD* pL2 = (DWORD*) (SrcPtrQ - inc3);
	DWORD* pL1 = (DWORD*) (SrcPtrQ - inc2);
	DWORD* pL0 = (DWORD*) (SrcPtrQ - inc);
	DWORD* pR0 = (DWORD*) (SrcPtrQ);
	DWORD* pR1 = (DWORD*) (SrcPtrQ + inc);
	DWORD* pR2 = (DWORD*) (SrcPtrQ + inc2);

	mm7 = _mm_setzero_si64();
	mm0 = _m_from_int(*pL1);                  
	mm1 = _m_from_int(*pL0);                   
	mm2 = _m_from_int(*pR0);                	    
	mm3 = _m_from_int(*pR1);
	mm0 = _m_punpcklbw(mm0, mm7);			//mm0 = L1
	mm1 = _m_punpcklbw(mm1, mm7);			//mm1 = L0 
	mm2 = _m_punpcklbw(mm2, mm7);			//mm2 = R0
	mm3 = _m_punpcklbw(mm3, mm7);			//mm3 = R1

	//xabs(R0 - L0 ) < Alpha
	mm4 = _m_psubw(mm2, mm1);
	mm7 = _m_psubw(mm7, mm4);
	mm4 = _m_pmaxsw(mm7, mm4);				//xabs(R0 - L0)
	mm7 = _mm_set1_pi16(Alpha);
	mm4 = _m_psubw(mm4, mm7);				//mm4 = xabs(R0 - L0) - Alpha

	//xabs( R0 - R1) < Beta
	mm5 = _m_psubw(mm2, mm3);   
	mm7 = _m_pxor(mm7, mm7);
	mm7 = _m_psubw(mm7, mm5);
	mm5 = _m_pmaxsw(mm7, mm5);				//xabs(R0 - R1)

	//xabs(L0 - L1) < Beta
	mm6 = _m_psubw(mm1, mm0);   
	mm7 = _m_pxor(mm7, mm7);
	mm7 = _m_psubw(mm7, mm6);
	mm6 = _m_pmaxsw(mm7, mm6);				//xabs(L0 - L1)
	mm7 = _mm_set1_pi16(Beta);
	mm5 = _m_psubw(mm5, mm7);				//mm5 = xabs(R0 - R1) - Beta
	mm6 = _m_psubw(mm6, mm7);				//mm6 = xabs(L0 - L1) - Beta

	mm7 = _m_pxor(mm7, mm7);
	mm4 = _m_pcmpgtw(mm7, mm4);
	mm5 = _m_pcmpgtw(mm7, mm5);
	mm6 = _m_pcmpgtw(mm7, mm6);	
	mm4 = _m_pand(mm4, mm5);
	mm4 = _m_pand(mm4, mm6);				//mm4 is deblocking mask				
	deblock_mask = mm4;

	//aq  =	(xabs( R0 - R2)	- Beta ) < 0 ;
	mm5 = _m_from_int(*pR2);
	mm5 = _m_punpcklbw(mm5, mm7);			//mm5 = R2
	mm5 = _m_psubw(mm2, mm5);
	mm7 = _m_psubw(mm7, mm5);
	mm5 = _m_pmaxsw(mm7, mm5);				//xabs(R0 - R2)

	//ap  =	(xabs( L0 - L2)	- Beta ) < 0 ;
	mm7 = _m_pxor(mm7, mm7);
	mm6 = _m_from_int(*pL2);
	mm6 = _m_punpcklbw(mm6, mm7);			//mm6 = L2
	mm6 = _m_psubw(mm1, mm6);
	mm7 = _m_psubw(mm7, mm6);
	mm6 = _m_pmaxsw(mm7, mm6);				//xabs(L0 - L2)
	mm7 = _mm_set1_pi16(Beta);
	mm5 = _m_psubw(mm5, mm7);				//mm5 = xabs(R0 - R2) - Beta
	mm6 = _m_psubw(mm6, mm7);				//mm6 = xabs(L0 - L2) - Beta
	mm7 = _m_pxor(mm7, mm7);
	mm5 = _m_pcmpgtw(mm7, mm5);				//mm5 = aq
	mm6 = _m_pcmpgtw(mm7, mm6);				//mm6 = ap
	aq_mask = mm5;
	ap_mask = mm6;

	//C0  =	ClipTab[ Strng ];
	mm4 = _mm_set1_pi16(C0);
	mm5 = _m_psrlwi(mm5, 15);
	mm6 = _m_psrlwi(mm6, 15);
	mm7 = _m_paddw(mm5, mm6);
	mm4 = _m_paddw(mm4, mm7);				//mm4 = C0 + ap	+ aq

	//dif =	__fast_iclipX_X(c0, ( ((R0 - L0 ) << 2) + (L1 - R1) + 4) >> 3 );
	mm5 = _m_psubw(mm2, mm1);
	mm5 = _m_psllwi(mm5, 2);
	mm6 = _m_psubw(mm0, mm3);
	mm5 = _m_paddw(mm5, mm6);
	mm5 = _m_paddw(mm5, n4);
	mm5 = _m_psrawi(mm5, 3);
	mm5 = _m_pminsw(mm5, mm4);
	mm6 = _m_pxor(mm6, mm6);
	mm6 = _m_psubw(mm6, mm4);
	mm5 = _m_pmaxsw(mm5, mm6);				//mm5 = diff
	mm4 = _m_psubw(mm4, mm7);				//mm4 = C0

	//SrcPtrQ[-inc] = CLIP0_255(L0 + dif) ;
	mm6 = _m_paddw(mm1, mm5);
	mm6 = _m_por(_m_pand(mm6, deblock_mask), _m_pand(mm1, _m_pxor(deblock_mask, nff)));	
	mm6 = _m_packuswb(mm6, mm7);
	*pL0 = _m_to_int(mm6);

	//SrcPtrQ[0] = CLIP0_255(R0 - dif) ;
	mm6 = _m_psubw(mm2, mm5);
	mm6 = _m_por(_m_pand(mm6, deblock_mask), _m_pand(mm2, _m_pxor(deblock_mask, nff)));
	mm6 = _m_packuswb(mm6, mm7);
	*pR0 = _m_to_int(mm6);

	//if(ap) SrcPtrQ[-inc2] += __fast_iclipX_X(C0, ( L2 + ((L0 + R0 + 1) >> 1) - (L1<<1)) >> 1);
	mm7 = _m_pxor(mm7, mm7);
	mm5 = _m_from_int(*pL2);
	mm5 = _m_punpcklbw(mm5, mm7);			//mm5 = L2
	mm6 = _m_pavgw(mm1, mm2);
	mm6 = _m_paddw(mm6, mm5);
	mm5 = _m_psllwi(mm0, 1);
	mm6 = _m_psubw(mm6, mm5);
	mm6 = _m_psrawi(mm6, 1);
	mm6 = _m_pminsw(mm6, mm4);	
	mm7 = _m_psubw(mm7, mm4);
	mm6 = _m_pmaxsw(mm6, mm7);
	mm6 = _m_pand(mm6, deblock_mask);
	mm6 = _m_pand(mm6, ap_mask);
	mm6 = _m_paddw(mm0, mm6);
	mm6 = _m_packuswb(mm6, mm7);
	*pL1 = _m_to_int(mm6);

	//if(aq) SrcPtrQ[ inc] += __fast_iclipX_X(C0, ( R2 + ((L0 + R0 + 1) >> 1) - (R1<<1)) >> 1 );
	mm7 = _m_pxor(mm7, mm7);
	mm5 = _m_from_int(*pR2);
	mm5 = _m_punpcklbw(mm5, mm7);			//mm5 = R2
	mm6 = _m_pavgw(mm1, mm2);
	mm6 = _m_paddw(mm6, mm5);
	mm5 = _m_psllwi(mm3, 1);
	mm6 = _m_psubw(mm6, mm5);
	mm6 = _m_psrawi(mm6, 1);
	mm6 = _m_pminsw(mm6, mm4);	
	mm7 = _m_psubw(mm7, mm4);
	mm6 = _m_pmaxsw(mm6, mm7);
	mm6 = _m_pand(mm6, deblock_mask);
	mm6 = _m_pand(mm6, aq_mask);
	mm6 = _m_paddw(mm3, mm6);
	mm6 = _m_packuswb(mm6, mm7);
	*pR1 = _m_to_int(mm6);	
}

void Deblock_luma_h_p8_1_sse(imgpel* SrcPtrQ, int inc, int Alpha, int Beta, int C0, int pNum)
{
	int inc2 = (inc<<1), inc3 = (inc<<1) + inc;
	const __m64 n4 = _mm_set1_pi16((short) 4), nff = _mm_set1_pi16((short) 0xffff);
	__m64 mm0, mm1, mm2, mm3, mm4, mm5, mm6, mm7;
	__m64 deblock_mask, ap_mask, aq_mask, tmp, tmp1,number1, number2;
	int temp;
	int temp1;

	//L2  = SrcPtrQ[-inc3];
	//L1  = SrcPtrQ[-inc2];
	//L0  = SrcPtrQ[-inc];
	//R0  = SrcPtrQ[0];
	//R1  = SrcPtrQ[ inc];
	//R2  = SrcPtrQ[ inc2];
	__m64* pL2 = (__m64*) (SrcPtrQ - inc3);
	__m64* pL1 = (__m64*) (SrcPtrQ - inc2);
	__m64* pL0 = (__m64*) (SrcPtrQ - inc);
	__m64* pR0 = (__m64*) (SrcPtrQ);
	__m64* pR1 = (__m64*) (SrcPtrQ + inc);
	__m64* pR2 = (__m64*) (SrcPtrQ + inc2);

	number1 = *((__m64*)pL0);
	number2 = *((__m64*)pR0);

	//  1
	mm7 = _mm_setzero_si64();
	mm0 = *((__m64*)pL1);
	mm1 = *((__m64*)pL0);
	mm2 = *((__m64*)pR0);
	mm3 = *((__m64*)pR1);
	mm0 = _mm_unpacklo_pi8(mm0, mm7);		//mm0 = L1
	mm1 = _mm_unpacklo_pi8(mm1, mm7);		//mm1 = L0
	mm2 = _mm_unpacklo_pi8(mm2, mm7);		//mm2 = R0
	mm3 = _mm_unpacklo_pi8(mm3, mm7);		//mm3 = R1

	//xabs(R0 - L0 ) < Alpha
	mm4 = _mm_sub_pi16(mm2, mm1);
	mm7 = _mm_sub_pi16(mm7, mm4);
	mm4 = _mm_max_pi16(mm7, mm4);				//xabs(R0 - L0)
	mm7 = _mm_set1_pi16(Alpha);
	mm4 = _mm_cmpgt_pi16(mm7, mm4);				//mm4 = xabs(R0 - L0) - Alpha

	//xabs( R0 - R1) < Beta
	mm5 = _mm_sub_pi16(mm2, mm3);   
	mm7 = _mm_xor_si64(mm7, mm7);
	mm7 = _mm_sub_pi16(mm7, mm5);
	mm5 = _mm_max_pi16(mm7, mm5);				//xabs(R0 - R1)

	//xabs(L0 - L1) < Beta
	mm6 = _mm_sub_pi16(mm1, mm0);   
	mm7 = _mm_xor_si64(mm7, mm7);
	mm7 = _mm_sub_pi16(mm7, mm6);
	mm6 = _mm_max_pi16(mm7, mm6);				//xabs(L0 - L1)
	mm7 = _mm_set1_pi16(Beta);
	mm5 = _mm_cmpgt_pi16(mm7, mm5);				//mm5 = xabs(R0 - R1) - Beta
	mm6 = _mm_cmpgt_pi16(mm7, mm6);				//mm6 = xabs(L0 - L1) - Beta

	mm7 = _mm_xor_si64(mm7, mm7);		
	mm4 = _mm_and_si64(mm4, mm5);
	mm4 = _mm_and_si64(mm4, mm6);				//mm4 is deblocking mask				


	temp1 = _mm_movemask_pi8(mm4);
	deblock_mask = mm4;

	//aq  =	(xabs( R0 - R2)	- Beta ) < 0 ;
	mm5 = *((__m64*)pR2);
	mm5 = _mm_unpacklo_pi8(mm5, mm7);			//mm5 = R2
	mm5 = _mm_sub_pi16(mm2, mm5);
	mm7 = _mm_sub_pi16(mm7, mm5);
	mm5 = _mm_max_pi16(mm7, mm5);				//xabs(R0 - R2)

	//ap  =	(xabs( L0 - L2)	- Beta ) < 0 ;
	mm7 = _mm_xor_si64(mm7, mm7);
	mm6 = *((__m64*)pL2);
	mm6 = _mm_unpacklo_pi8(mm6, mm7);			//mm6 = L2
	mm6 = _mm_sub_pi16(mm1, mm6);
	mm7 = _mm_sub_pi16(mm7, mm6);
	mm6 = _mm_max_pi16(mm7, mm6);				//xabs(L0 - L2)
	mm7 = _mm_set1_pi16(Beta);	
	mm5 = _mm_cmpgt_pi16(mm7, mm5);				//mm5 = xabs(R0 - R2) - Beta
	mm6 = _mm_cmpgt_pi16(mm7, mm6);				//mm6 = xabs(L0 - L2) - Beta
	mm7 = _mm_xor_si64(mm7, mm7);
	aq_mask = mm5;
	ap_mask = mm6;

	//C0  =	ClipTab[ Strng ];
	mm4 = _mm_set1_pi16(C0);
	mm5 = _mm_srli_pi16(mm5, 15);
	mm6 = _mm_srli_pi16(mm6, 15);
	mm7 = _mm_add_pi16(mm5, mm6);
	mm4 = _mm_add_pi16(mm4, mm7);				//mm4 = C0 + ap	+ aq

	//dif =	__fast_iclipX_X(c0, ( ((R0 - L0 ) << 2) + (L1 - R1) + 4) >> 3 );
	mm5 = _mm_sub_pi16(mm2, mm1);
	mm5 = _mm_slli_pi16(mm5, 2);
	mm6 = _mm_sub_pi16(mm0, mm3);
	mm5 = _mm_add_pi16(mm5, mm6);
	mm5 = _mm_add_pi16(mm5, n4);
	mm5 = _mm_srai_pi16(mm5, 3);
	mm5 = _mm_min_pi16(mm5, mm4);
	mm6 = _mm_xor_si64(mm6, mm6);
	mm6 = _mm_sub_pi16(mm6, mm4);
	mm5 = _mm_max_pi16(mm5, mm6);				//mm5 = diff
	mm4 = _mm_sub_pi16(mm4, mm7);				//mm4 = C0

	//SrcPtrQ[-inc] = CLIP0_255(L0 + dif) ;
	mm6 = _mm_add_pi16(mm1, mm5);
	mm6 = _mm_or_si64(_mm_and_si64(mm6, deblock_mask), _mm_and_si64(mm1, _mm_xor_si64(deblock_mask, nff)));	
	tmp = mm6;
	mm6 = _mm_sub_pi16(mm2, mm5);
	mm6 = _mm_or_si64(_mm_and_si64(mm6, deblock_mask), _mm_and_si64(mm2, _mm_xor_si64(deblock_mask, nff)));
	tmp1 = mm6;

	//  2
	mm7 = _mm_setzero_si64();
	
	mm0 = *((__m64*)pL1);
	mm1 = *((__m64*)pL0);
	mm2 = *((__m64*)pR0);
	mm3 = *((__m64*)pR1);
	mm0 = _mm_unpackhi_pi8(mm0, mm7);		//mm0 = L1
	mm1 = _mm_unpackhi_pi8(mm1, mm7);		//mm1 = L0
	mm2 = _mm_unpackhi_pi8(mm2, mm7);		//mm2 = R0
	mm3 = _mm_unpackhi_pi8(mm3, mm7);		//mm3 = R1

	//xabs(R0 - L0 ) < Alpha
	mm4 = _mm_sub_pi16(mm2, mm1);
	mm7 = _mm_sub_pi16(mm7, mm4);
	mm4 = _mm_max_pi16(mm7, mm4);				//xabs(R0 - L0)
	mm7 = _mm_set1_pi16(Alpha);
	mm4 = _mm_cmpgt_pi16(mm7, mm4);				//mm4 = xabs(R0 - L0) - Alpha

	//xabs( R0 - R1) < Beta
	mm5 = _mm_sub_pi16(mm2, mm3);   
	mm7 = _mm_xor_si64(mm7, mm7);
	mm7 = _mm_sub_pi16(mm7, mm5);
	mm5 = _mm_max_pi16(mm7, mm5);				//xabs(R0 - R1)

	//xabs(L0 - L1) < Beta
	mm6 = _mm_sub_pi16(mm1, mm0);   
	mm7 = _mm_xor_si64(mm7, mm7);
	mm7 = _mm_sub_pi16(mm7, mm6);
	mm6 = _mm_max_pi16(mm7, mm6);				//xabs(L0 - L1)
	mm7 = _mm_set1_pi16(Beta);
	mm5 = _mm_cmpgt_pi16(mm7, mm5);				//mm5 = xabs(R0 - R1) - Beta
	mm6 = _mm_cmpgt_pi16(mm7, mm6);				//mm6 = xabs(L0 - L1) - Beta

	mm7 = _mm_xor_si64(mm7, mm7);		
	mm4 = _mm_and_si64(mm4, mm5);
	mm4 = _mm_and_si64(mm4, mm6);				//mm4 is deblocking mask				


	temp = _mm_movemask_pi8(mm4);
	if(!(temp || temp1))
		return;		
	deblock_mask = mm4;

	//aq  =	(xabs( R0 - R2)	- Beta ) < 0 ;
	mm5 = *((__m64*)pR2);
	mm5 = _mm_unpackhi_pi8(mm5, mm7);			//mm5 = R2
	mm5 = _mm_sub_pi16(mm2, mm5);
	mm7 = _mm_sub_pi16(mm7, mm5);
	mm5 = _mm_max_pi16(mm7, mm5);				//xabs(R0 - R2)

	//ap  =	(xabs( L0 - L2)	- Beta ) < 0 ;
	mm7 = _mm_xor_si64(mm7, mm7);
	mm6 = *((__m64*)pL2);
	mm6 = _mm_unpackhi_pi8(mm6, mm7);			//mm6 = L2
	mm6 = _mm_sub_pi16(mm1, mm6);
	mm7 = _mm_sub_pi16(mm7, mm6);
	mm6 = _mm_max_pi16(mm7, mm6);				//xabs(L0 - L2)
	mm7 = _mm_set1_pi16(Beta);	
	mm5 = _mm_cmpgt_pi16(mm7, mm5);				//mm5 = xabs(R0 - R2) - Beta
	mm6 = _mm_cmpgt_pi16(mm7, mm6);				//mm6 = xabs(L0 - L2) - Beta
	mm7 = _mm_xor_si64(mm7, mm7);
	aq_mask = mm5;
	ap_mask = mm6;

	//C0  =	ClipTab[ Strng ];
	mm4 = _mm_set1_pi16(C0);
	mm5 = _mm_srli_pi16(mm5, 15);
	mm6 = _mm_srli_pi16(mm6, 15);
	mm7 = _mm_add_pi16(mm5, mm6);
	mm4 = _mm_add_pi16(mm4, mm7);				//mm4 = C0 + ap	+ aq

	//dif =	__fast_iclipX_X(c0, ( ((R0 - L0 ) << 2) + (L1 - R1) + 4) >> 3 );
	mm5 = _mm_sub_pi16(mm2, mm1);
	mm5 = _mm_slli_pi16(mm5, 2);
	mm6 = _mm_sub_pi16(mm0, mm3);
	mm5 = _mm_add_pi16(mm5, mm6);
	mm5 = _mm_add_pi16(mm5, n4);
	mm5 = _mm_srai_pi16(mm5, 3);
	mm5 = _mm_min_pi16(mm5, mm4);
	mm6 = _mm_xor_si64(mm6, mm6);
	mm6 = _mm_sub_pi16(mm6, mm4);
	mm5 = _mm_max_pi16(mm5, mm6);				//mm5 = diff
	mm4 = _mm_sub_pi16(mm4, mm7);				//mm4 = C0

	//SrcPtrQ[-inc] = CLIP0_255(L0 + dif) ;
	mm6 = _mm_add_pi16(mm1, mm5);
	mm6 = _mm_or_si64(_mm_and_si64(mm6, deblock_mask), _mm_and_si64(mm1, _mm_xor_si64(deblock_mask, nff)));	
	mm6 = _mm_packs_pu16(tmp, mm6);
	*((__m64*)pL0) = mm6;

	mm6 = _mm_sub_pi16(mm2, mm5);
	mm6 = _mm_or_si64(_mm_and_si64(mm6, deblock_mask), _mm_and_si64(mm2, _mm_xor_si64(deblock_mask, nff)));
	mm6 = _mm_packs_pu16(tmp1, mm6);
	*((__m64*)pR0) = mm6;
//************************************************************
	//SrcPtrQ[0] = CLIP0_255(R0 - dif) ;
	
	//  1
	mm7 = _mm_setzero_si64();
	mm0 = *((__m64*)pL1);
	mm1 = number1;
	mm2 = number2;
	mm3 = *((__m64*)pR1);
	mm0 = _mm_unpacklo_pi8(mm0, mm7);		//mm0 = L1
	mm1 = _mm_unpacklo_pi8(mm1, mm7);		//mm1 = L0
	mm2 = _mm_unpacklo_pi8(mm2, mm7);		//mm2 = R0
	mm3 = _mm_unpacklo_pi8(mm3, mm7);		//mm3 = R1

	mm4 = _mm_sub_pi16(mm2, mm1);
	mm7 = _mm_sub_pi16(mm7, mm4);
	mm4 = _mm_max_pi16(mm7, mm4);				//xabs(R0 - L0)
	mm7 = _mm_set1_pi16(Alpha);
	mm4 = _mm_cmpgt_pi16(mm7, mm4);				//mm4 = xabs(R0 - L0) - Alpha

	//xabs( R0 - R1) < Beta
	mm5 = _mm_sub_pi16(mm2, mm3);   
	mm7 = _mm_xor_si64(mm7, mm7);
	mm7 = _mm_sub_pi16(mm7, mm5);
	mm5 = _mm_max_pi16(mm7, mm5);				//xabs(R0 - R1)

	//xabs(L0 - L1) < Beta
	mm6 = _mm_sub_pi16(mm1, mm0);   
	mm7 = _mm_xor_si64(mm7, mm7);
	mm7 = _mm_sub_pi16(mm7, mm6);
	mm6 = _mm_max_pi16(mm7, mm6);				//xabs(L0 - L1)
	mm7 = _mm_set1_pi16(Beta);
	mm5 = _mm_cmpgt_pi16(mm7, mm5);				//mm5 = xabs(R0 - R1) - Beta
	mm6 = _mm_cmpgt_pi16(mm7, mm6);				//mm6 = xabs(L0 - L1) - Beta

	mm7 = _mm_xor_si64(mm7, mm7);		
	mm4 = _mm_and_si64(mm4, mm5);
	mm4 = _mm_and_si64(mm4, mm6);				//mm4 is deblocking mask				


	deblock_mask = mm4;

	//aq  =	(xabs( R0 - R2)	- Beta ) < 0 ;
	mm5 = *((__m64*)pR2);
	mm5 = _mm_unpacklo_pi8(mm5, mm7);			//mm5 = R2
	mm5 = _mm_sub_pi16(mm2, mm5);
	mm7 = _mm_sub_pi16(mm7, mm5);
	mm5 = _mm_max_pi16(mm7, mm5);				//xabs(R0 - R2)

	//ap  =	(xabs( L0 - L2)	- Beta ) < 0 ;
	mm7 = _mm_xor_si64(mm7, mm7);
	mm6 = *((__m64*)pL2);
	mm6 = _mm_unpacklo_pi8(mm6, mm7);			//mm6 = L2
	mm6 = _mm_sub_pi16(mm1, mm6);
	mm7 = _mm_sub_pi16(mm7, mm6);
	mm6 = _mm_max_pi16(mm7, mm6);				//xabs(L0 - L2)
	mm7 = _mm_set1_pi16(Beta);	
	mm5 = _mm_cmpgt_pi16(mm7, mm5);				//mm5 = xabs(R0 - R2) - Beta
	mm6 = _mm_cmpgt_pi16(mm7, mm6);				//mm6 = xabs(L0 - L2) - Beta
	mm7 = _mm_xor_si64(mm7, mm7);
	aq_mask = mm5;
	ap_mask = mm6;

	//C0  =	ClipTab[ Strng ];
	mm4 = _mm_set1_pi16(C0);
	mm5 = _mm_srli_pi16(mm5, 15);
	mm6 = _mm_srli_pi16(mm6, 15);
	mm7 = _mm_add_pi16(mm5, mm6);
	mm4 = _mm_add_pi16(mm4, mm7);				//mm4 = C0 + ap	+ aq

	//dif =	__fast_iclipX_X(c0, ( ((R0 - L0 ) << 2) + (L1 - R1) + 4) >> 3 );
	mm5 = _mm_sub_pi16(mm2, mm1);
	mm5 = _mm_slli_pi16(mm5, 2);
	mm6 = _mm_sub_pi16(mm0, mm3);
	mm5 = _mm_add_pi16(mm5, mm6);
	mm5 = _mm_add_pi16(mm5, n4);
	mm5 = _mm_srai_pi16(mm5, 3);
	mm5 = _mm_min_pi16(mm5, mm4);
	mm6 = _mm_xor_si64(mm6, mm6);
	mm6 = _mm_sub_pi16(mm6, mm4);
	mm5 = _mm_max_pi16(mm5, mm6);				//mm5 = diff
	mm4 = _mm_sub_pi16(mm4, mm7);				//mm4 = C0

	
	//if(ap) SrcPtrQ[-inc2] += __fast_iclipX_X(C0, ( L2 + ((L0 + R0 + 1) >> 1) - (L1<<1)) >> 1);
	mm7 = _mm_xor_si64(mm7, mm7);
	mm5 = *((__m64*)pL2);
	mm5 = _mm_unpacklo_pi8(mm5, mm7);			//mm5 = L2
	mm6 = _mm_avg_pu8(mm1, mm2);
	mm6 = _mm_add_pi16(mm6, mm5);
	mm5 = _mm_slli_pi16(mm0, 1);
	mm6 = _mm_sub_pi16(mm6, mm5);
	mm6 = _mm_srai_pi16(mm6, 1);
	mm6 = _mm_min_pi16(mm6, mm4);	
	mm7 = _mm_sub_pi16(mm7, mm4);
	mm6 = _mm_max_pi16(mm6, mm7);
	mm6 = _mm_and_si64(mm6, deblock_mask);
	mm6 = _mm_and_si64(mm6, ap_mask);
	mm6 = _mm_add_pi16(mm0, mm6);
	
	tmp = mm6;
	
	//if(aq) SrcPtrQ[ inc] += __fast_iclipX_X(C0, ( R2 + ((L0 + R0 + 1) >> 1) - (R1<<1)) >> 1 );
	mm7 = _mm_xor_si64(mm7, mm7);
	mm5 = *((__m64*)pR2);
	mm5 = _mm_unpacklo_pi8(mm5, mm7);			//mm5 = R2
	mm6 = _mm_avg_pu8(mm1, mm2);
	mm6 = _mm_add_pi16(mm6, mm5);
	mm5 = _mm_slli_pi16(mm3, 1);
	mm6 = _mm_sub_pi16(mm6, mm5);
	mm6 = _mm_srai_pi16(mm6, 1);
	mm6 = _mm_min_pi16(mm6, mm4);	
	mm7 = _mm_sub_pi16(mm7, mm4);
	mm6 = _mm_max_pi16(mm6, mm7);
	mm6 = _mm_and_si64(mm6, deblock_mask);
	mm6 = _mm_and_si64(mm6, aq_mask);
	mm6 = _mm_add_pi16(mm3, mm6);

	tmp1 = mm6;
	
	//  2
	mm7 = _mm_setzero_si64();
	mm0 = *((__m64*)pL1);
	mm1 = number1;
	mm2 = number2;
	mm3 = *((__m64*)pR1);
	mm0 = _mm_unpackhi_pi8(mm0, mm7);		//mm0 = L1
	mm1 = _mm_unpackhi_pi8(mm1, mm7);		//mm1 = L0
	mm2 = _mm_unpackhi_pi8(mm2, mm7);		//mm2 = R0
	mm3 = _mm_unpackhi_pi8(mm3, mm7);		//mm3 = R1
	
	mm4 = _mm_sub_pi16(mm2, mm1);
	mm7 = _mm_sub_pi16(mm7, mm4);
	mm4 = _mm_max_pi16(mm7, mm4);				//xabs(R0 - L0)
	mm7 = _mm_set1_pi16(Alpha);
	mm4 = _mm_cmpgt_pi16(mm7, mm4);				//mm4 = xabs(R0 - L0) - Alpha

	//xabs( R0 - R1) < Beta
	mm5 = _mm_sub_pi16(mm2, mm3);   
	mm7 = _mm_xor_si64(mm7, mm7);
	mm7 = _mm_sub_pi16(mm7, mm5);
	mm5 = _mm_max_pi16(mm7, mm5);				//xabs(R0 - R1)

	//xabs(L0 - L1) < Beta
	mm6 = _mm_sub_pi16(mm1, mm0);   
	mm7 = _mm_xor_si64(mm7, mm7);
	mm7 = _mm_sub_pi16(mm7, mm6);
	mm6 = _mm_max_pi16(mm7, mm6);				//xabs(L0 - L1)
	mm7 = _mm_set1_pi16(Beta);
	mm5 = _mm_cmpgt_pi16(mm7, mm5);				//mm5 = xabs(R0 - R1) - Beta
	mm6 = _mm_cmpgt_pi16(mm7, mm6);				//mm6 = xabs(L0 - L1) - Beta

	mm7 = _mm_xor_si64(mm7, mm7);		
	mm4 = _mm_and_si64(mm4, mm5);
	mm4 = _mm_and_si64(mm4, mm6);				//mm4 is deblocking mask				


	deblock_mask = mm4;

	//aq  =	(xabs( R0 - R2)	- Beta ) < 0 ;
	mm5 = *((__m64*)pR2);
	mm5 = _mm_unpackhi_pi8(mm5, mm7);			//mm5 = R2
	mm5 = _mm_sub_pi16(mm2, mm5);
	mm7 = _mm_sub_pi16(mm7, mm5);
	mm5 = _mm_max_pi16(mm7, mm5);				//xabs(R0 - R2)

	//ap  =	(xabs( L0 - L2)	- Beta ) < 0 ;
	mm7 = _mm_xor_si64(mm7, mm7);
	mm6 = *((__m64*)pL2);
	mm6 = _mm_unpackhi_pi8(mm6, mm7);			//mm6 = L2
	mm6 = _mm_sub_pi16(mm1, mm6);
	mm7 = _mm_sub_pi16(mm7, mm6);
	mm6 = _mm_max_pi16(mm7, mm6);				//xabs(L0 - L2)
	mm7 = _mm_set1_pi16(Beta);	
	mm5 = _mm_cmpgt_pi16(mm7, mm5);				//mm5 = xabs(R0 - R2) - Beta
	mm6 = _mm_cmpgt_pi16(mm7, mm6);				//mm6 = xabs(L0 - L2) - Beta
	mm7 = _mm_xor_si64(mm7, mm7);
	aq_mask = mm5;
	ap_mask = mm6;

	//C0  =	ClipTab[ Strng ];
	mm4 = _mm_set1_pi16(C0);
	mm5 = _mm_srli_pi16(mm5, 15);
	mm6 = _mm_srli_pi16(mm6, 15);
	mm7 = _mm_add_pi16(mm5, mm6);
	mm4 = _mm_add_pi16(mm4, mm7);				//mm4 = C0 + ap	+ aq

	//dif =	__fast_iclipX_X(c0, ( ((R0 - L0 ) << 2) + (L1 - R1) + 4) >> 3 );
	mm5 = _mm_sub_pi16(mm2, mm1);
	mm5 = _mm_slli_pi16(mm5, 2);
	mm6 = _mm_sub_pi16(mm0, mm3);
	mm5 = _mm_add_pi16(mm5, mm6);
	mm5 = _mm_add_pi16(mm5, n4);
	mm5 = _mm_srai_pi16(mm5, 3);
	mm5 = _mm_min_pi16(mm5, mm4);
	mm6 = _mm_xor_si64(mm6, mm6);
	mm6 = _mm_sub_pi16(mm6, mm4);
	mm5 = _mm_max_pi16(mm5, mm6);				//mm5 = diff
	mm4 = _mm_sub_pi16(mm4, mm7);				//mm4 = C0
	
	//if(ap) SrcPtrQ[-inc2] += __fast_iclipX_X(C0, ( L2 + ((L0 + R0 + 1) >> 1) - (L1<<1)) >> 1);
	mm7 = _mm_xor_si64(mm7, mm7);
	mm5 = *((__m64*)pL2);
	mm5 = _mm_unpackhi_pi8(mm5, mm7);			//mm5 = L2
	mm6 = _mm_avg_pu8(mm1, mm2);
	mm6 = _mm_add_pi16(mm6, mm5);
	mm5 = _mm_slli_pi16(mm0, 1);
	mm6 = _mm_sub_pi16(mm6, mm5);
	mm6 = _mm_srai_pi16(mm6, 1);
	mm6 = _mm_min_pi16(mm6, mm4);	
	mm7 = _mm_sub_pi16(mm7, mm4);
	mm6 = _mm_max_pi16(mm6, mm7);
	mm6 = _mm_and_si64(mm6, deblock_mask);
	mm6 = _mm_and_si64(mm6, ap_mask);
	mm6 = _mm_add_pi16(mm0, mm6);
	
	mm6 = _mm_packs_pu16(tmp, mm6);
	*((__m64*)pL1) = mm6;
	
	//if(aq) SrcPtrQ[ inc] += __fast_iclipX_X(C0, ( R2 + ((L0 + R0 + 1) >> 1) - (R1<<1)) >> 1 );
	mm7 = _mm_xor_si64(mm7, mm7);
	mm5 = *((__m64*)pR2);
	mm5 = _mm_unpackhi_pi8(mm5, mm7);			//mm5 = R2
	mm6 = _mm_avg_pu8(mm1, mm2);
	mm6 = _mm_add_pi16(mm6, mm5);
	mm5 = _mm_slli_pi16(mm3, 1);
	mm6 = _mm_sub_pi16(mm6, mm5);
	mm6 = _mm_srai_pi16(mm6, 1);
	mm6 = _mm_min_pi16(mm6, mm4);	
	mm7 = _mm_sub_pi16(mm7, mm4);
	mm6 = _mm_max_pi16(mm6, mm7);
	mm6 = _mm_and_si64(mm6, deblock_mask);
	mm6 = _mm_and_si64(mm6, aq_mask);
	mm6 = _mm_add_pi16(mm3, mm6);
	
	mm6 = _mm_packs_pu16(tmp1, mm6);
	*((__m64*)pR1) = mm6;
}

void Deblock_luma_h_p8_1_sse2(imgpel* SrcPtrQ, int inc, int Alpha, int Beta, int C0, int pNum)
{
	int inc2 = (inc<<1), inc3 = (inc<<1) + inc;
	const __m128i n4 = _mm_set1_epi16((short) 4), nff = _mm_set1_epi16((short) 0xffff);
	__m128i mm0, mm1, mm2, mm3, mm4, mm5, mm6, mm7;
	__m128i deblock_mask, ap_mask, aq_mask;
	int temp;

	//L2  = SrcPtrQ[-inc3];
	//L1  = SrcPtrQ[-inc2];
	//L0  = SrcPtrQ[-inc];
	//R0  = SrcPtrQ[0];
	//R1  = SrcPtrQ[ inc];
	//R2  = SrcPtrQ[ inc2];
	__m64* pL2 = (__m64*) (SrcPtrQ - inc3);
	__m64* pL1 = (__m64*) (SrcPtrQ - inc2);
	__m64* pL0 = (__m64*) (SrcPtrQ - inc);
	__m64* pR0 = (__m64*) (SrcPtrQ);
	__m64* pR1 = (__m64*) (SrcPtrQ + inc);
	__m64* pR2 = (__m64*) (SrcPtrQ + inc2);

	mm7 = _mm_setzero_si128();
	mm0 = _mm_loadl_epi64((__m128i*)pL1);             
	mm1 = _mm_loadl_epi64((__m128i*)pL0);             
	mm2 = _mm_loadl_epi64((__m128i*)pR0);             
	mm3 = _mm_loadl_epi64((__m128i*)pR1);
	mm0 = _mm_unpacklo_epi8(mm0, mm7);		//mm0 = L1
	mm1 = _mm_unpacklo_epi8(mm1, mm7);		//mm1 = L0
	mm2 = _mm_unpacklo_epi8(mm2, mm7);		//mm2 = R0
	mm3 = _mm_unpacklo_epi8(mm3, mm7);		//mm3 = R1

	//xabs(R0 - L0 ) < Alpha
	mm4 = _mm_sub_epi16(mm2, mm1);
	mm7 = _mm_sub_epi16(mm7, mm4);
	mm4 = _mm_max_epi16(mm7, mm4);				//xabs(R0 - L0)
	mm7 = _mm_set1_epi16(Alpha);
	mm4 = _mm_cmpgt_epi16(mm7, mm4);				//mm4 = xabs(R0 - L0) - Alpha

	//xabs( R0 - R1) < Beta
	mm5 = _mm_sub_epi16(mm2, mm3);   
	mm7 = _mm_xor_si128(mm7, mm7);
	mm7 = _mm_sub_epi16(mm7, mm5);
	mm5 = _mm_max_epi16(mm7, mm5);				//xabs(R0 - R1)

	//xabs(L0 - L1) < Beta
	mm6 = _mm_sub_epi16(mm1, mm0);   
	mm7 = _mm_xor_si128(mm7, mm7);
	mm7 = _mm_sub_epi16(mm7, mm6);
	mm6 = _mm_max_epi16(mm7, mm6);				//xabs(L0 - L1)
	mm7 = _mm_set1_epi16(Beta);
	mm5 = _mm_cmpgt_epi16(mm7, mm5);				//mm5 = xabs(R0 - R1) - Beta
	mm6 = _mm_cmpgt_epi16(mm7, mm6);				//mm6 = xabs(L0 - L1) - Beta

	mm7 = _mm_xor_si128(mm7, mm7);		
	mm4 = _mm_and_si128(mm4, mm5);
	mm4 = _mm_and_si128(mm4, mm6);				//mm4 is deblocking mask				


	temp = _mm_movemask_epi8(mm4);
	if(!temp)
		return;		
	deblock_mask = mm4;

	//aq  =	(xabs( R0 - R2)	- Beta ) < 0 ;
	mm5 = _mm_loadl_epi64((__m128i*)pR2);
	mm5 = _mm_unpacklo_epi8(mm5, mm7);			//mm5 = R2
	mm5 = _mm_sub_epi16(mm2, mm5);
	mm7 = _mm_sub_epi16(mm7, mm5);
	mm5 = _mm_max_epi16(mm7, mm5);				//xabs(R0 - R2)

	//ap  =	(xabs( L0 - L2)	- Beta ) < 0 ;
	mm7 = _mm_xor_si128(mm7, mm7);
	mm6 = _mm_loadl_epi64((__m128i*)pL2);
	mm6 = _mm_unpacklo_epi8(mm6, mm7);			//mm6 = L2
	mm6 = _mm_sub_epi16(mm1, mm6);
	mm7 = _mm_sub_epi16(mm7, mm6);
	mm6 = _mm_max_epi16(mm7, mm6);				//xabs(L0 - L2)
	mm7 = _mm_set1_epi16(Beta);	
	mm5 = _mm_cmpgt_epi16(mm7, mm5);				//mm5 = xabs(R0 - R2) - Beta
	mm6 = _mm_cmpgt_epi16(mm7, mm6);				//mm6 = xabs(L0 - L2) - Beta
	mm7 = _mm_xor_si128(mm7, mm7);
	aq_mask = mm5;
	ap_mask = mm6;

	//C0  =	ClipTab[ Strng ];
	mm4 = _mm_set1_epi16(C0);
	mm5 = _mm_srli_epi16(mm5, 15);
	mm6 = _mm_srli_epi16(mm6, 15);
	mm7 = _mm_add_epi16(mm5, mm6);
	mm4 = _mm_add_epi16(mm4, mm7);				//mm4 = C0 + ap	+ aq

	//dif =	__fast_iclipX_X(c0, ( ((R0 - L0 ) << 2) + (L1 - R1) + 4) >> 3 );
	mm5 = _mm_sub_epi16(mm2, mm1);
	mm5 = _mm_slli_epi16(mm5, 2);
	mm6 = _mm_sub_epi16(mm0, mm3);
	mm5 = _mm_add_epi16(mm5, mm6);
	mm5 = _mm_add_epi16(mm5, n4);
	mm5 = _mm_srai_epi16(mm5, 3);
	mm5 = _mm_min_epi16(mm5, mm4);
	mm6 = _mm_xor_si128(mm6, mm6);
	mm6 = _mm_sub_epi16(mm6, mm4);
	mm5 = _mm_max_epi16(mm5, mm6);				//mm5 = diff
	mm4 = _mm_sub_epi16(mm4, mm7);				//mm4 = C0

	//SrcPtrQ[-inc] = CLIP0_255(L0 + dif) ;
	mm6 = _mm_add_epi16(mm1, mm5);
	mm6 = _mm_or_si128(_mm_and_si128(mm6, deblock_mask), _mm_and_si128(mm1, _mm_xor_si128(deblock_mask, nff)));	
	mm6 = _mm_packus_epi16(mm6, mm7);
	_mm_storel_epi64((__m128i*)pL0, mm6);

	//SrcPtrQ[0] = CLIP0_255(R0 - dif) ;
	mm6 = _mm_sub_epi16(mm2, mm5);
	mm6 = _mm_or_si128(_mm_and_si128(mm6, deblock_mask), _mm_and_si128(mm2, _mm_xor_si128(deblock_mask, nff)));
	mm6 = _mm_packus_epi16(mm6, mm7);
	_mm_storel_epi64((__m128i*)pR0, mm6);

	//if(ap) SrcPtrQ[-inc2] += __fast_iclipX_X(C0, ( L2 + ((L0 + R0 + 1) >> 1) - (L1<<1)) >> 1);
	mm7 = _mm_xor_si128(mm7, mm7);
	mm5 = _mm_loadl_epi64((__m128i*)pL2);
	mm5 = _mm_unpacklo_epi8(mm5, mm7);			//mm5 = L2
	mm6 = _mm_avg_epu8(mm1, mm2);
	mm6 = _mm_add_epi16(mm6, mm5);
	mm5 = _mm_slli_epi16(mm0, 1);
	mm6 = _mm_sub_epi16(mm6, mm5);
	mm6 = _mm_srai_epi16(mm6, 1);
	mm6 = _mm_min_epi16(mm6, mm4);	
	mm7 = _mm_sub_epi16(mm7, mm4);
	mm6 = _mm_max_epi16(mm6, mm7);
	mm6 = _mm_and_si128(mm6, deblock_mask);
	mm6 = _mm_and_si128(mm6, ap_mask);
	mm6 = _mm_add_epi16(mm0, mm6);
	mm6 = _mm_packus_epi16(mm6, mm7);
	_mm_storel_epi64((__m128i*)pL1, mm6);

	//if(aq) SrcPtrQ[ inc] += __fast_iclipX_X(C0, ( R2 + ((L0 + R0 + 1) >> 1) - (R1<<1)) >> 1 );
	mm7 = _mm_xor_si128(mm7, mm7);
	mm5 = _mm_loadl_epi64((__m128i*)pR2);
	mm5 = _mm_unpacklo_epi8(mm5, mm7);			//mm5 = R2
	mm6 = _mm_avg_epu8(mm1, mm2);
	mm6 = _mm_add_epi16(mm6, mm5);
	mm5 = _mm_slli_epi16(mm3, 1);
	mm6 = _mm_sub_epi16(mm6, mm5);
	mm6 = _mm_srai_epi16(mm6, 1);
	mm6 = _mm_min_epi16(mm6, mm4);	
	mm7 = _mm_sub_epi16(mm7, mm4);
	mm6 = _mm_max_epi16(mm6, mm7);
	mm6 = _mm_and_si128(mm6, deblock_mask);
	mm6 = _mm_and_si128(mm6, aq_mask);
	mm6 = _mm_add_epi16(mm3, mm6);
	mm6 = _mm_packus_epi16(mm6, mm7);
	_mm_storel_epi64((__m128i*)pR1, mm6);
}

void Deblock_luma_v_1_mmx(imgpel* SrcPtrQ, int dpel, int Alpha, int Beta, int C0, int pNum)
{
	int dpel2 = (dpel<<1), dpel3 = dpel + (dpel<<1);
	const __m64 n4 = _mm_set1_pi16((short) 4), nff = _mm_set1_pi16((short) 0xffff);
	__m64 mm0, mm1, mm2, mm3, mm4, mm5, mm6, mm7;
	__m64 deblock_mask, ap_mask, aq_mask, sL2, sR2;

	//L2  = SrcPtrQ[-3] ;
	//L1  = SrcPtrQ[-2] ;
	//L0  = SrcPtrQ[-1] ;
	//R0  = SrcPtrQ[0] ;
	//R1  = SrcPtrQ[1] ;
	//R2  = SrcPtrQ[2] ;
	DWORD* Load1_1 = (DWORD*) (SrcPtrQ - 4);
	DWORD* Load2_1 = (DWORD*) (SrcPtrQ);
	DWORD* Store_1 = (DWORD*) (SrcPtrQ - 2);
	DWORD* Load1_2 = (DWORD*) (SrcPtrQ - 4 + dpel);
	DWORD* Load2_2 = (DWORD*) (SrcPtrQ + dpel);
	DWORD* Store_2 = (DWORD*) (SrcPtrQ - 2 + dpel);
	DWORD* Load1_3 = (DWORD*) (SrcPtrQ - 4 + dpel2);
	DWORD* Load2_3 = (DWORD*) (SrcPtrQ + dpel2);
	DWORD* Store_3 = (DWORD*) (SrcPtrQ - 2 + dpel2);
	DWORD* Load1_4 = (DWORD*) (SrcPtrQ - 4 + dpel3);
	DWORD* Load2_4 = (DWORD*) (SrcPtrQ + dpel3);
	DWORD* Store_4 = (DWORD*) (SrcPtrQ - 2 + dpel3);

	mm7 = _mm_setzero_si64();
	mm0 = _m_from_int(*Load1_1);
	mm1 = _m_from_int(*Load1_2); 
	mm2 = _m_from_int(*Load1_3);
	mm3 = _m_from_int(*Load1_4);
	mm0 = _m_punpcklbw(mm0, mm7);    	          
	mm1 = _m_punpcklbw(mm1, mm7);    	                
	mm2 = _m_punpcklbw(mm2, mm7);   	                
	mm3 = _m_punpcklbw(mm3, mm7);

	//transpose
	mm4 = _mm_unpackhi_pi16(mm0,mm1);
	mm5 = _mm_unpacklo_pi16(mm0,mm1);
	mm6 = _mm_unpackhi_pi16(mm2,mm3);
	mm7 = _mm_unpacklo_pi16(mm2,mm3);		
	mm0 = _mm_unpacklo_pi32(mm5,mm7);		
	mm1 = _mm_unpackhi_pi32(mm5,mm7);	
	mm2 = _mm_unpacklo_pi32(mm4,mm6);
	mm3 = _mm_unpackhi_pi32(mm4,mm6);

	sL2 = mm1;

	mm7 = _m_pxor(mm7, mm7);
	mm0 = _m_from_int(*Load2_1);               
	mm1 = _m_from_int(*Load2_2);
	mm4 = _m_from_int(*Load2_3);
	mm5 = _m_from_int(*Load2_4);
	mm0 = _m_punpcklbw(mm0, mm7);    	                
	mm1 = _m_punpcklbw(mm1, mm7);    	
	mm4 = _m_punpcklbw(mm4, mm7);	
	mm5 = _m_punpcklbw(mm5, mm7);

	//transpose
	mm6 = _mm_unpackhi_pi16(mm4,mm5);
	mm7 = _mm_unpacklo_pi16(mm4,mm5);
	mm4 = _mm_unpackhi_pi16(mm0,mm1);
	mm5 = _mm_unpacklo_pi16(mm0,mm1);	
	mm0 = _mm_unpacklo_pi32(mm5,mm7);		
	mm1 = _mm_unpackhi_pi32(mm5,mm7);	
	mm5 = _mm_unpacklo_pi32(mm4,mm6);
	mm7 = _mm_unpackhi_pi32(mm4,mm6);

	sR2 = mm5;
	mm4 = mm0;								//mm2 = R0
	mm5 = mm1;								//mm3 = R1
	mm0 = mm2;								//mm0 = L1
	mm1 = mm3;								//mm1 = L0
	mm2 = mm4;
	mm3 = mm5;

	//xabs(R0 - L0 ) < Alpha
	mm7 = _m_pxor(mm7, mm7);
	mm4 = _m_psubw(mm2, mm1);
	mm7 = _m_psubw(mm7, mm4);
	mm4 = _m_pmaxsw(mm7, mm4);				//xabs(R0 - L0)
	mm7 = _mm_set1_pi16(Alpha);
	mm4 = _m_psubw(mm4, mm7);				//mm4 = xabs(R0 - L0) - Alpha

	//xabs( R0 - R1) < Beta
	mm5 = _m_psubw(mm2, mm3);   
	mm7 = _m_pxor(mm7, mm7);
	mm7 = _m_psubw(mm7, mm5);
	mm5 = _m_pmaxsw(mm7, mm5);				//xabs(R0 - R1)

	//xabs(L0 - L1) < Beta
	mm6 = _m_psubw(mm1, mm0);   
	mm7 = _m_pxor(mm7, mm7);
	mm7 = _m_psubw(mm7, mm6);
	mm6 = _m_pmaxsw(mm7, mm6);				//xabs(L0 - L1)
	mm7 = _mm_set1_pi16(Beta);
	mm5 = _m_psubw(mm5, mm7);				//mm5 = xabs(R0 - R1) - Beta
	mm6 = _m_psubw(mm6, mm7);				//mm6 = xabs(L0 - L1) - Beta

	mm7 = _m_pxor(mm7, mm7);
	mm4 = _m_pcmpgtw(mm7, mm4);
	mm5 = _m_pcmpgtw(mm7, mm5);
	mm6 = _m_pcmpgtw(mm7, mm6);	
	mm4 = _m_pand(mm4, mm5);
	mm4 = _m_pand(mm4, mm6);				//mm4 is deblocking mask				
	deblock_mask = mm4;

	//aq  =	(xabs( R0 - R2)	- Beta ) < 0 ;
	mm5 = sR2;								//mm5 = R2
	mm5 = _m_psubw(mm2, mm5);
	mm7 = _m_psubw(mm7, mm5);
	mm5 = _m_pmaxsw(mm7, mm5);				//xabs(R0 - R2)

	//ap  =	(xabs( L0 - L2)	- Beta ) < 0 ;
	mm7 = _m_pxor(mm7, mm7);
	mm6 = sL2;								//mm6 = L2
	mm6 = _m_psubw(mm1, mm6);
	mm7 = _m_psubw(mm7, mm6);
	mm6 = _m_pmaxsw(mm7, mm6);				//xabs(L0 - L2)
	mm7 = _mm_set1_pi16(Beta);
	mm5 = _m_psubw(mm5, mm7);				//mm5 = xabs(R0 - R2) - Beta
	mm6 = _m_psubw(mm6, mm7);				//mm6 = xabs(L0 - L2) - Beta
	mm7 = _m_pxor(mm7, mm7);
	mm5 = _m_pcmpgtw(mm7, mm5);				//mm5 = aq
	mm6 = _m_pcmpgtw(mm7, mm6);				//mm6 = ap
	aq_mask = mm5;
	ap_mask = mm6;

	//c0 = (C0 + ap + aq) ;
	mm4 = _mm_set1_pi16(C0);
	mm5 = _m_psrlwi(mm5, 15);
	mm6 = _m_psrlwi(mm6, 15);
	mm7 = _m_paddw(mm5, mm6);
	mm4 = _m_paddw(mm4, mm7);				//mm4 = C0 + ap	+ aq

	//dif =	__fast_iclipX_X(c0, ( ((R0 - L0 ) << 2) + (L1 - R1) + 4) >> 3 );
	mm5 = _m_psubw(mm2, mm1);
	mm5 = _m_psllwi(mm5, 2);
	mm6 = _m_psubw(mm0, mm3);
	mm5 = _m_paddw(mm5, mm6);
	mm5 = _m_paddw(mm5, n4);
	mm5 = _m_psrawi(mm5, 3);
	mm5 = _m_pminsw(mm5, mm4);
	mm6 = _m_pxor(mm6, mm6);
	mm6 = _m_psubw(mm6, mm4);
	mm5 = _m_pmaxsw(mm5, mm6);				//mm5 = diff
	mm4 = _m_psubw(mm4, mm7);				//mm4 = C0

	mm7 =  _m_pavgw(mm1, mm2);

	//SrcPtrQ[-1] = CLIP0_255(L0 + dif) ;
	mm6 = _m_paddw(mm1, mm5);
	mm1 = _m_por(_m_pand(mm6, deblock_mask), _m_pand(mm1, _m_pxor(deblock_mask, nff)));

	//SrcPtrQ[0] = CLIP0_255(R0 - dif) ;
	mm6 = _m_psubw(mm2, mm5);
	mm2 = _m_por(_m_pand(mm6, deblock_mask), _m_pand(mm2, _m_pxor(deblock_mask, nff)));

	//if(ap) SrcPtrQ[-2] += __fast_iclipX_X(C0, ( L2 + ((L0 + R0 + 1) >> 1) - (L1<<1)) >> 1);
	mm5 = sL2;								//mm5 = L2
	mm6 = _m_paddw(mm7, mm5);
	mm5 = _m_psllwi(mm0, 1);
	mm6 = _m_psubw(mm6, mm5);
	mm5 = _m_pxor(mm5, mm5);
	mm6 = _m_psrawi(mm6, 1);
	mm6 = _m_pminsw(mm6, mm4);	
	mm5 = _m_psubw(mm5, mm4);
	mm6 = _m_pmaxsw(mm6, mm5);
	mm6 = _m_pand(mm6, deblock_mask);
	mm6 = _m_pand(mm6, ap_mask);
	mm0 = _m_paddw(mm0, mm6);

	//if(aq) SrcPtrQ[1] += __fast_iclipX_X(C0, ( R2 + ((L0 + R0 + 1) >> 1) - (R1<<1)) >> 1 );
	mm5 = sR2;								//mm5 = R2
	mm6 = _m_paddw(mm7, mm5);
	mm5 = _m_psllwi(mm3, 1);
	mm6 = _m_psubw(mm6, mm5);
	mm5 = _m_pxor(mm5, mm5);
	mm6 = _m_psrawi(mm6, 1);
	mm6 = _m_pminsw(mm6, mm4);	
	mm5 = _m_psubw(mm5, mm4);
	mm6 = _m_pmaxsw(mm6, mm5);
	mm6 = _m_pand(mm6, deblock_mask);
	mm6 = _m_pand(mm6, aq_mask);
	mm3 = _m_paddw(mm3, mm6);

	//transpose
	mm4 = _mm_unpackhi_pi16(mm0,mm1);
	mm5 = _mm_unpacklo_pi16(mm0,mm1);
	mm6 = _mm_unpackhi_pi16(mm2,mm3);
	mm7 = _mm_unpacklo_pi16(mm2,mm3);		
	mm0 = _mm_unpacklo_pi32(mm5,mm7);		
	mm1 = _mm_unpackhi_pi32(mm5,mm7);	
	mm2 = _mm_unpacklo_pi32(mm4,mm6);
	mm3 = _mm_unpackhi_pi32(mm4,mm6);

	mm0 = _m_packuswb(mm0, mm7);
	mm1 = _m_packuswb(mm1, mm7);
	mm2 = _m_packuswb(mm2, mm7);
	mm3 = _m_packuswb(mm3, mm7);

	*Store_1 = _m_to_int(mm0);
	*Store_2 = _m_to_int(mm1);
	*Store_3 = _m_to_int(mm2);
	*Store_4 = _m_to_int(mm3);
}

void Deblock_luma_v_p8_1_mmx(imgpel* SrcPtrQ, int dpel, int Alpha, int Beta, int C0, int pNum)
{
	int dpel2 = (dpel<<1), dpel3 = dpel + (dpel<<1);
	const __m64 n4 = _mm_set1_pi16((short) 4), nff = _mm_set1_pi16((short) 0xffff);
	__m64 mm0, mm1, mm2, mm3, mm4, mm5, mm6, mm7;
	__m64 deblock_mask, ap_mask, aq_mask, sL2, sR2;

	int temp;

	for(int i = 0; i < 2; i++)
	{
		//L2  = SrcPtrQ[-3] ;
		//L1  = SrcPtrQ[-2] ;
		//L0  = SrcPtrQ[-1] ;
		//R0  = SrcPtrQ[0] ;
		//R1  = SrcPtrQ[1] ;
		//R2  = SrcPtrQ[2] ;
		DWORD* Load1_1 = (DWORD*) (SrcPtrQ - 4);
		DWORD* Load2_1 = (DWORD*) (SrcPtrQ);
		DWORD* Store_1 = (DWORD*) (SrcPtrQ - 2);
		DWORD* Load1_2 = (DWORD*) (SrcPtrQ - 4 + dpel);
		DWORD* Load2_2 = (DWORD*) (SrcPtrQ + dpel);
		DWORD* Store_2 = (DWORD*) (SrcPtrQ - 2 + dpel);
		DWORD* Load1_3 = (DWORD*) (SrcPtrQ - 4 + dpel2);
		DWORD* Load2_3 = (DWORD*) (SrcPtrQ + dpel2);
		DWORD* Store_3 = (DWORD*) (SrcPtrQ - 2 + dpel2);
		DWORD* Load1_4 = (DWORD*) (SrcPtrQ - 4 + dpel3);
		DWORD* Load2_4 = (DWORD*) (SrcPtrQ + dpel3);
		DWORD* Store_4 = (DWORD*) (SrcPtrQ - 2 + dpel3);

		mm7 = _mm_setzero_si64();
		mm0 = _m_from_int(*Load1_1);
		mm1 = _m_from_int(*Load1_2); 
		mm2 = _m_from_int(*Load1_3);
		mm3 = _m_from_int(*Load1_4);
		mm0 = _m_punpcklbw(mm0, mm7);    	          
		mm1 = _m_punpcklbw(mm1, mm7);    	                
		mm2 = _m_punpcklbw(mm2, mm7);   	                
		mm3 = _m_punpcklbw(mm3, mm7);

		//transpose
		mm4 = _mm_unpackhi_pi16(mm0,mm1);
		mm5 = _mm_unpacklo_pi16(mm0,mm1);
		mm6 = _mm_unpackhi_pi16(mm2,mm3);
		mm7 = _mm_unpacklo_pi16(mm2,mm3);		
		mm0 = _mm_unpacklo_pi32(mm5,mm7);		
		mm1 = _mm_unpackhi_pi32(mm5,mm7);	
		mm2 = _mm_unpacklo_pi32(mm4,mm6);
		mm3 = _mm_unpackhi_pi32(mm4,mm6);

		sL2 = mm1;

		mm7 = _m_pxor(mm7, mm7);
		mm0 = _m_from_int(*Load2_1);               
		mm1 = _m_from_int(*Load2_2);
		mm4 = _m_from_int(*Load2_3);
		mm5 = _m_from_int(*Load2_4);
		mm0 = _m_punpcklbw(mm0, mm7);    	                
		mm1 = _m_punpcklbw(mm1, mm7);    	
		mm4 = _m_punpcklbw(mm4, mm7);	
		mm5 = _m_punpcklbw(mm5, mm7);

		//transpose
		mm6 = _mm_unpackhi_pi16(mm4,mm5);
		mm7 = _mm_unpacklo_pi16(mm4,mm5);
		mm4 = _mm_unpackhi_pi16(mm0,mm1);
		mm5 = _mm_unpacklo_pi16(mm0,mm1);	
		mm0 = _mm_unpacklo_pi32(mm5,mm7);		
		mm1 = _mm_unpackhi_pi32(mm5,mm7);	
		mm5 = _mm_unpacklo_pi32(mm4,mm6);
		mm7 = _mm_unpackhi_pi32(mm4,mm6);

		sR2 = mm5;
		mm4 = mm0;								//mm2 = R0
		mm5 = mm1;								//mm3 = R1
		mm0 = mm2;								//mm0 = L1
		mm1 = mm3;								//mm1 = L0
		mm2 = mm4;
		mm3 = mm5;

		//xabs(R0 - L0 ) < Alpha
		mm7 = _m_pxor(mm7, mm7);
		mm4 = _m_psubw(mm2, mm1);
		mm7 = _m_psubw(mm7, mm4);
		mm4 = _m_pmaxsw(mm7, mm4);				//xabs(R0 - L0)
		mm7 = _mm_set1_pi16(Alpha);
		mm4 = _m_pcmpgtw(mm7, mm4);				//mm4 = xabs(R0 - L0) - Alpha

		//xabs( R0 - R1) < Beta
		mm5 = _m_psubw(mm2, mm3);   
		mm7 = _m_pxor(mm7, mm7);
		mm7 = _m_psubw(mm7, mm5);
		mm5 = _m_pmaxsw(mm7, mm5);				//xabs(R0 - R1)

		//xabs(L0 - L1) < Beta
		mm6 = _m_psubw(mm1, mm0);   
		mm7 = _m_pxor(mm7, mm7);
		mm7 = _m_psubw(mm7, mm6);
		mm6 = _m_pmaxsw(mm7, mm6);				//xabs(L0 - L1)
		mm7 = _mm_set1_pi16(Beta);
		mm5 = _m_pcmpgtw(mm7, mm5);				//mm5 = xabs(R0 - R1) - Beta
		mm6 = _m_pcmpgtw(mm7, mm6);				//mm6 = xabs(L0 - L1) - Beta

		mm7 = _m_pxor(mm7, mm7);
		mm4 = _m_pand(mm4, mm5);
		mm4 = _m_pand(mm4, mm6);				//mm4 is deblocking mask				
		deblock_mask = mm4;

		mm5 = mm4;
		mm5 = _mm_slli_si64(mm5, 16);
		mm4 = _mm_adds_pi16(mm4, mm5);
		mm5 = mm4;
		mm5 = _mm_slli_si64(mm5, 32);
		mm4 = _mm_adds_pi16(mm4, mm5);
		mm4 = _mm_srli_si64(mm4, 48);

		temp = _m_to_int(mm4);

		if(!temp)
			goto end0;	

		//aq  =	(xabs( R0 - R2)	- Beta ) < 0 ;
		mm5 = sR2;								//mm5 = R2
		mm5 = _m_psubw(mm2, mm5);
		mm7 = _m_psubw(mm7, mm5);
		mm5 = _m_pmaxsw(mm7, mm5);				//xabs(R0 - R2)

		//ap  =	(xabs( L0 - L2)	- Beta ) < 0 ;
		mm7 = _m_pxor(mm7, mm7);
		mm6 = sL2;								//mm6 = L2
		mm6 = _m_psubw(mm1, mm6);
		mm7 = _m_psubw(mm7, mm6);
		mm6 = _m_pmaxsw(mm7, mm6);				//xabs(L0 - L2)
		mm7 = _mm_set1_pi16(Beta);

		mm5 = _m_pcmpgtw(mm7, mm5);				//mm5 = xabs(R0 - R2) - Beta
		mm6 = _m_pcmpgtw(mm7, mm6);				//mm6 = xabs(L0 - L2) - Beta
		mm7 = _m_pxor(mm7, mm7);
		aq_mask = mm5;
		ap_mask = mm6;

		//c0 = (C0 + ap + aq) ;
		mm4 = _mm_set1_pi16(C0);
		mm5 = _m_psrlwi(mm5, 15);
		mm6 = _m_psrlwi(mm6, 15);
		mm7 = _m_paddw(mm5, mm6);
		mm4 = _m_paddw(mm4, mm7);				//mm4 = C0 + ap	+ aq

		//dif =	__fast_iclipX_X(c0, ( ((R0 - L0 ) << 2) + (L1 - R1) + 4) >> 3 );
		mm5 = _m_psubw(mm2, mm1);
		mm5 = _m_psllwi(mm5, 2);
		mm6 = _m_psubw(mm0, mm3);
		mm5 = _m_paddw(mm5, mm6);
		mm5 = _m_paddw(mm5, n4);
		mm5 = _m_psrawi(mm5, 3);
		mm5 = _m_pminsw(mm5, mm4);
		mm6 = _m_pxor(mm6, mm6);
		mm6 = _m_psubw(mm6, mm4);
		mm5 = _m_pmaxsw(mm5, mm6);				//mm5 = diff
		mm4 = _m_psubw(mm4, mm7);				//mm4 = C0

		mm7 =  _m_pavgw(mm1, mm2);

		//SrcPtrQ[-1] = CLIP0_255(L0 + dif) ;
		mm6 = _m_paddw(mm1, mm5);
		mm1 = _m_por(_m_pand(mm6, deblock_mask), _m_pand(mm1, _m_pxor(deblock_mask, nff)));

		//SrcPtrQ[0] = CLIP0_255(R0 - dif) ;
		mm6 = _m_psubw(mm2, mm5);
		mm2 = _m_por(_m_pand(mm6, deblock_mask), _m_pand(mm2, _m_pxor(deblock_mask, nff)));

		//if(ap) SrcPtrQ[-2] += __fast_iclipX_X(C0, ( L2 + ((L0 + R0 + 1) >> 1) - (L1<<1)) >> 1);
		mm5 = sL2;								//mm5 = L2
		mm6 = _m_paddw(mm7, mm5);
		mm5 = _m_psllwi(mm0, 1);
		mm6 = _m_psubw(mm6, mm5);
		mm5 = _m_pxor(mm5, mm5);
		mm6 = _m_psrawi(mm6, 1);
		mm6 = _m_pminsw(mm6, mm4);	
		mm5 = _m_psubw(mm5, mm4);
		mm6 = _m_pmaxsw(mm6, mm5);
		mm6 = _m_pand(mm6, deblock_mask);
		mm6 = _m_pand(mm6, ap_mask);
		mm0 = _m_paddw(mm0, mm6);

		//if(aq) SrcPtrQ[1] += __fast_iclipX_X(C0, ( R2 + ((L0 + R0 + 1) >> 1) - (R1<<1)) >> 1 );
		mm5 = sR2;								//mm5 = R2
		mm6 = _m_paddw(mm7, mm5);
		mm5 = _m_psllwi(mm3, 1);
		mm6 = _m_psubw(mm6, mm5);
		mm5 = _m_pxor(mm5, mm5);
		mm6 = _m_psrawi(mm6, 1);
		mm6 = _m_pminsw(mm6, mm4);	
		mm5 = _m_psubw(mm5, mm4);
		mm6 = _m_pmaxsw(mm6, mm5);
		mm6 = _m_pand(mm6, deblock_mask);
		mm6 = _m_pand(mm6, aq_mask);
		mm3 = _m_paddw(mm3, mm6);

		//transpose
		mm4 = _mm_unpackhi_pi16(mm0,mm1);
		mm5 = _mm_unpacklo_pi16(mm0,mm1);
		mm6 = _mm_unpackhi_pi16(mm2,mm3);
		mm7 = _mm_unpacklo_pi16(mm2,mm3);		
		mm0 = _mm_unpacklo_pi32(mm5,mm7);		
		mm1 = _mm_unpackhi_pi32(mm5,mm7);	
		mm2 = _mm_unpacklo_pi32(mm4,mm6);
		mm3 = _mm_unpackhi_pi32(mm4,mm6);

		mm0 = _m_packuswb(mm0, mm7);
		mm1 = _m_packuswb(mm1, mm7);
		mm2 = _m_packuswb(mm2, mm7);
		mm3 = _m_packuswb(mm3, mm7);

		*Store_1 = _m_to_int(mm0);
		*Store_2 = _m_to_int(mm1);
		*Store_3 = _m_to_int(mm2);
		*Store_4 = _m_to_int(mm3);

end0:
		SrcPtrQ += (dpel<<2);
	}
}
#endif //H264_ENABLE_INTRINSICS
