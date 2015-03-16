#pragma warning ( disable : 4799 )

#include <stdlib.h>
#include "global.h"
#ifdef H264_ENABLE_INTRINSICS
#include <mmintrin.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include "transform8x8.h"

// IoK-02-NOV-2005: There are 5 versions
//                  v.1 Original (intrinsics)
//                  v.2 Mix MMX/SSE2 intrinsics
//                  v.3 Mix MMX/SSE2 inline assembly
//                  v.4 New SSE2 intrinsics - BEST
//                  v.5 New SSE2 inline assembly
//Eric-06-NOV-2006: Add a new version
//                  v.6 New SSE2 intrinsics - best than v.4 in single-threaded mode
//                  v.7 New SSE2 intrinsics - best than v.4 in multi-threaded mode
#if 0
// Original code - SSE2 intrinsics
void inverse_transform8x8_sse2 PARGS4(byte *dest, byte *pred, short *src, int stride)
{
	__m128i mm0,mm1,mm2,mm3,mm4,mm5,mm6,mm7;
	__m128i  a0, a1, a2, a3, a4, a5, a6, a7;
	__m128i  b0, b1, b2, b3, b4, b5, b6, b7;

	// Transpose 8x8 matrix
	//start	
	mm0 = _mm_load_si128((__m128i*)&src[0 ]); //0 1 2 3 4 5 6 7
	mm1 = _mm_load_si128((__m128i*)&src[8 ]); //0 1 2 3 4 5 6 7
	mm2 = _mm_load_si128((__m128i*)&src[16]); //0 1 2 3 4 5 6 7
	mm3 = _mm_load_si128((__m128i*)&src[24]); //0 1 2 3 4 5 6 7
	mm4 = _mm_load_si128((__m128i*)&src[32]); //0 1 2 3 4 5 6 7
	mm5 = _mm_load_si128((__m128i*)&src[40]); //0 1 2 3 4 5 6 7
	mm6 = _mm_load_si128((__m128i*)&src[48]); //0 1 2 3 4 5 6 7
	mm7 = _mm_load_si128((__m128i*)&src[56]); //0 1 2 3 4 5 6 7

	a0 = _mm_unpacklo_epi16(mm0,mm1);// 0 0 1 1 2 2 3 3
	a1 = _mm_unpacklo_epi16(mm2,mm3);// 0 0 1 1 2 2 3 3
	a2 = _mm_unpacklo_epi16(mm4,mm5);// 0 0 1 1 2 2 3 3
	a3 = _mm_unpacklo_epi16(mm6,mm7);// 0 0 1 1 2 2 3 3

	a4 = _mm_unpackhi_epi16(mm0,mm1);// 4 4 5 5 6 6 7 7
	a5 = _mm_unpackhi_epi16(mm2,mm3);// 4 4 5 5 6 6 7 7
	a6 = _mm_unpackhi_epi16(mm4,mm5);// 4 4 5 5 6 6 7 7
	a7 = _mm_unpackhi_epi16(mm6,mm7);// 4 4 5 5 6 6 7 7

	b0  = _mm_unpacklo_epi32(a0,a1);// 0 0 0 0 1 1 1 1
	b1  = _mm_unpacklo_epi32(a2,a3);// 0 0 0 0 1 1 1 1
	mm0 = _mm_unpacklo_epi64(b0,b1);// 0 0 0 0 0 0 0 0
	mm1 = _mm_unpackhi_epi64(b0,b1);// 1 1 1 1 1 1 1 1

	b0  = _mm_unpackhi_epi32(a0,a1);// 2 2 2 2 3 3 3 3
	b1  = _mm_unpackhi_epi32(a2,a3);// 2 2 2 2 3 3 3 3
	mm2 = _mm_unpacklo_epi64(b0,b1);// 2 2 2 2 2 2 2 2
	mm3 = _mm_unpackhi_epi64(b0,b1);// 3 3 3 3 3 3 3 3

	b0  = _mm_unpacklo_epi32(a4,a5);// 4 4 4 4 5 5 5 5
	b1  = _mm_unpacklo_epi32(a6,a7);// 4 4 4 4 5 5 5 5
	mm4 = _mm_unpacklo_epi64(b0,b1);// 4 4 4 4 4 4 4 4
	mm5 = _mm_unpackhi_epi64(b0,b1);// 5 5 5 5 5 5 5 5 

	b0  = _mm_unpackhi_epi32(a4,a5);// 6 6 6 6 7 7 7 7
	b1  = _mm_unpackhi_epi32(a6,a7);// 6 6 6 6 7 7 7 7
	mm6 = _mm_unpacklo_epi64(b0,b1);// 6 6 6 6 6 6 6 6
	mm7 = _mm_unpackhi_epi64(b0,b1);// 7 7 7 7 7 7 7 7
	//end

	static __m128i  ncoeff_0 = _mm_setzero_si128();	
	static __m128i  ncoeff_32 = _mm_set1_epi16(32);

	memset(src, 0, 8*8*sizeof(short));

	a0 = _mm_adds_epi16(mm0,mm4);
	a4 = _mm_subs_epi16(mm0,mm4);
	a2 = _mm_subs_epi16(_mm_srai_epi16(mm2,1),mm6);
	a6 = _mm_adds_epi16(mm2,_mm_srai_epi16(mm6,1));
	a1 = _mm_subs_epi16(_mm_subs_epi16(mm5,mm3),_mm_adds_epi16(mm7,_mm_srai_epi16(mm7,1)));
	a3 = _mm_subs_epi16(_mm_adds_epi16(mm1,mm7),_mm_adds_epi16(mm3,_mm_srai_epi16(mm3,1)));
	a5 = _mm_adds_epi16(_mm_subs_epi16(mm7,mm1),_mm_adds_epi16(mm5,_mm_srai_epi16(mm5,1)));
	a7 = _mm_adds_epi16(_mm_adds_epi16(mm3,mm5),_mm_adds_epi16(mm1,_mm_srai_epi16(mm1,1)));

	b0 = _mm_adds_epi16(a0,a6);
	b6 = _mm_subs_epi16(a0,a6);
	b2 = _mm_adds_epi16(a4,a2);
	b4 = _mm_subs_epi16(a4,a2);
	b1 = _mm_adds_epi16(a1,_mm_srai_epi16(a7,2));
	b7 = _mm_subs_epi16(a7,_mm_srai_epi16(a1,2));
	b3 = _mm_adds_epi16(a3,_mm_srai_epi16(a5,2));
	b5 = _mm_subs_epi16(_mm_srai_epi16(a3,2),a5);

	// Transpose 8x8 matrix
	// start
	mm0 = _mm_adds_epi16(b0,b7);// 0 1 2 3 4 5 6 7
	mm1 = _mm_adds_epi16(b2,b5);// 0 1 2 3 4 5 6 7
	mm2 = _mm_adds_epi16(b4,b3);// 0 1 2 3 4 5 6 7
	mm3 = _mm_adds_epi16(b6,b1);// 0 1 2 3 4 5 6 7
	mm4 = _mm_subs_epi16(b6,b1);// 0 1 2 3 4 5 6 7
	mm5 = _mm_subs_epi16(b4,b3);// 0 1 2 3 4 5 6 7
	mm6 = _mm_subs_epi16(b2,b5);// 0 1 2 3 4 5 6 7
	mm7 = _mm_subs_epi16(b0,b7);// 0 1 2 3 4 5 6 7

	a0 = _mm_unpacklo_epi16(mm0,mm1);
	a1 = _mm_unpacklo_epi16(mm2,mm3);
	a2 = _mm_unpacklo_epi16(mm4,mm5);
	a3 = _mm_unpacklo_epi16(mm6,mm7);

	a4 = _mm_unpackhi_epi16(mm0,mm1);
	a5 = _mm_unpackhi_epi16(mm2,mm3);
	a6 = _mm_unpackhi_epi16(mm4,mm5);
	a7 = _mm_unpackhi_epi16(mm6,mm7);

	b0  = _mm_unpacklo_epi32(a0,a1);
	b1  = _mm_unpacklo_epi32(a2,a3);
	mm0 = _mm_unpacklo_epi64(b0,b1);
	mm1 = _mm_unpackhi_epi64(b0,b1);

	b0  = _mm_unpackhi_epi32(a0,a1);
	b1  = _mm_unpackhi_epi32(a2,a3);
	mm2 = _mm_unpacklo_epi64(b0,b1);
	mm3 = _mm_unpackhi_epi64(b0,b1);

	b0  = _mm_unpacklo_epi32(a4,a5);
	b1  = _mm_unpacklo_epi32(a6,a7);
	mm4 = _mm_unpacklo_epi64(b0,b1);
	mm5 = _mm_unpackhi_epi64(b0,b1);

	b0  = _mm_unpackhi_epi32(a4,a5);
	b1  = _mm_unpackhi_epi32(a6,a7);
	mm6 = _mm_unpacklo_epi64(b0,b1);
	mm7 = _mm_unpackhi_epi64(b0,b1);
	//end

	// vertical
	a0 = _mm_adds_epi16(mm0,mm4);
	a4 = _mm_subs_epi16(mm0,mm4);
	a2 = _mm_subs_epi16(_mm_srai_epi16(mm2,1),mm6);
	a6 = _mm_adds_epi16(mm2,_mm_srai_epi16(mm6,1));
	a1 = _mm_subs_epi16(_mm_subs_epi16(mm5,mm3),_mm_adds_epi16(mm7,_mm_srai_epi16(mm7,1)));
	a3 = _mm_subs_epi16(_mm_adds_epi16(mm1,mm7),_mm_adds_epi16(mm3,_mm_srai_epi16(mm3,1)));
	a5 = _mm_adds_epi16(_mm_subs_epi16(mm7,mm1),_mm_adds_epi16(mm5,_mm_srai_epi16(mm5,1)));
	a7 = _mm_adds_epi16(_mm_adds_epi16(mm3,mm5),_mm_adds_epi16(mm1,_mm_srai_epi16(mm1,1)));

	b0 = _mm_adds_epi16(a0,a6);
	b6 = _mm_subs_epi16(a0,a6);
	b2 = _mm_adds_epi16(a4,a2);
	b4 = _mm_subs_epi16(a4,a2);
	b1 = _mm_adds_epi16(a1,_mm_srai_epi16(a7,2));
	b7 = _mm_subs_epi16(a7,_mm_srai_epi16(a1,2));
	b3 = _mm_adds_epi16(a3,_mm_srai_epi16(a5,2));
	b5 = _mm_subs_epi16(_mm_srai_epi16(a3,2),a5);

	mm0 = _mm_adds_epi16(b0,b7);
	mm1 = _mm_adds_epi16(b2,b5);
	mm2 = _mm_adds_epi16(b4,b3);
	mm3 = _mm_adds_epi16(b6,b1);
	mm4 = _mm_subs_epi16(b6,b1);
	mm5 = _mm_subs_epi16(b4,b3);
	mm6 = _mm_subs_epi16(b2,b5);
	mm7 = _mm_subs_epi16(b0,b7);

	a0 = _mm_loadl_epi64 ((__m128i *) (pred+0  ));
	a1 = _mm_loadl_epi64 ((__m128i *) (pred+16 ));
	a2 = _mm_loadl_epi64 ((__m128i *) (pred+32 ));
	a3 = _mm_loadl_epi64 ((__m128i *) (pred+48 ));
	a4 = _mm_loadl_epi64 ((__m128i *) (pred+64 ));
	a5 = _mm_loadl_epi64 ((__m128i *) (pred+80 ));
	a6 = _mm_loadl_epi64 ((__m128i *) (pred+96 ));
	a7 = _mm_loadl_epi64 ((__m128i *) (pred+112));

	a0 = _mm_unpacklo_epi8(a0,ncoeff_0);
	a1 = _mm_unpacklo_epi8(a1,ncoeff_0);
	a2 = _mm_unpacklo_epi8(a2,ncoeff_0);
	a3 = _mm_unpacklo_epi8(a3,ncoeff_0);
	a4 = _mm_unpacklo_epi8(a4,ncoeff_0);
	a5 = _mm_unpacklo_epi8(a5,ncoeff_0);
	a6 = _mm_unpacklo_epi8(a6,ncoeff_0);
	a7 = _mm_unpacklo_epi8(a7,ncoeff_0);

	mm0 = _mm_srai_epi16(_mm_adds_epi16(mm0,_mm_adds_epi16(_mm_slli_epi16(a0,6),ncoeff_32)),6);
	mm1 = _mm_srai_epi16(_mm_adds_epi16(mm1,_mm_adds_epi16(_mm_slli_epi16(a1,6),ncoeff_32)),6);
	mm2 = _mm_srai_epi16(_mm_adds_epi16(mm2,_mm_adds_epi16(_mm_slli_epi16(a2,6),ncoeff_32)),6);
	mm3 = _mm_srai_epi16(_mm_adds_epi16(mm3,_mm_adds_epi16(_mm_slli_epi16(a3,6),ncoeff_32)),6);
	mm4 = _mm_srai_epi16(_mm_adds_epi16(mm4,_mm_adds_epi16(_mm_slli_epi16(a4,6),ncoeff_32)),6);
	mm5 = _mm_srai_epi16(_mm_adds_epi16(mm5,_mm_adds_epi16(_mm_slli_epi16(a5,6),ncoeff_32)),6);
	mm6 = _mm_srai_epi16(_mm_adds_epi16(mm6,_mm_adds_epi16(_mm_slli_epi16(a6,6),ncoeff_32)),6);
	mm7 = _mm_srai_epi16(_mm_adds_epi16(mm7,_mm_adds_epi16(_mm_slli_epi16(a7,6),ncoeff_32)),6);

	mm0 = _mm_packus_epi16(mm0,mm0);
	mm1 = _mm_packus_epi16(mm1,mm1);
	mm2 = _mm_packus_epi16(mm2,mm2);
	mm3 = _mm_packus_epi16(mm3,mm3);
	mm4 = _mm_packus_epi16(mm4,mm4);
	mm5 = _mm_packus_epi16(mm5,mm5);
	mm6 = _mm_packus_epi16(mm6,mm6);
	mm7 = _mm_packus_epi16(mm7,mm7);


	_mm_storel_epi64((__m128i*)&dest[0*stride], mm0);
	_mm_storel_epi64((__m128i*)&dest[1*stride], mm1);
	_mm_storel_epi64((__m128i*)&dest[2*stride], mm2);
	_mm_storel_epi64((__m128i*)&dest[3*stride], mm3);
	_mm_storel_epi64((__m128i*)&dest[4*stride], mm4);
	_mm_storel_epi64((__m128i*)&dest[5*stride], mm5);
	_mm_storel_epi64((__m128i*)&dest[6*stride], mm6);
	_mm_storel_epi64((__m128i*)&dest[7*stride], mm7);

}
#elif 0
// New code, Mix MMX-SSE2 intrinsics
void inverse_transform8x8_sse2 PARGS4(byte *dest, byte *pred, short *src, int stride)
{
	__m128i xmm0,xmm1,xmm2,xmm3,xmm4,xmm5,xmm6,xmm7;
	__m64   mm0, mm1, mm2, mm3, mm4, mm5, mm6, mm7;
	static unsigned short __declspec(align(16)) const_32[8] = { 32, 32, 32, 32, 32, 32, 32, 32 };

	// Transpose 4x8 matrix
	//start
	xmm0 = _mm_load_si128((__m128i*)&src[0 ]); //a 0 1 2 3 4 5 6 7
	xmm1 = _mm_load_si128((__m128i*)&src[8 ]); //b 0 1 2 3 4 5 6 7
	xmm2 = _mm_load_si128((__m128i*)&src[16]); //c 0 1 2 3 4 5 6 7
	xmm3 = _mm_load_si128((__m128i*)&src[24]); //d 0 1 2 3 4 5 6 7

	xmm6 = xmm0; //a 0 1 2 3 4 5 6 7
	xmm7 = xmm2; //c 0 1 2 3 4 5 6 7

	xmm0 = _mm_unpacklo_epi16(xmm0,xmm1); // a0 b0 a1 b1 a2 b2 a3 b3
	xmm2 = _mm_unpacklo_epi16(xmm2,xmm3); // c0 d0 c1 d1 c2 d2 c3 d3
	xmm6 = _mm_unpackhi_epi16(xmm6,xmm1); // a4 b4 a5 b5 a6 b6 a7 b7
	xmm7 = _mm_unpackhi_epi16(xmm7,xmm3); // c4 d4 c5 d5 c6 d6 c7 d7

	xmm1 = xmm0; // a0 b0 a1 b1 a2 b2 a3 b3
	xmm3 = xmm6; // a4 b4 a5 b5 a6 b6 a7 b7

	xmm0 = _mm_unpacklo_epi32(xmm0,xmm2); // a0 b0 c0 d0 a1 b1 c1 d1
	xmm6 = _mm_unpacklo_epi32(xmm6,xmm7); // a4 b4 c4 d4 a5 b5 c5 d5
	xmm1 = _mm_unpackhi_epi32(xmm1,xmm2); // a2 b2 c2 d2 a3 b3 c3 d3
	xmm3 = _mm_unpackhi_epi32(xmm3,xmm7); // a6 b6 c6 d6 a7 b7 c7 d7
	// 16
	mm3 = _mm_movepi64_pi64(xmm0); // a0 b0 c0 d0
	mm6 = _mm_movepi64_pi64(xmm1); // a2 b2 c2 d2
	mm0 = _mm_movepi64_pi64(xmm6); // a4 b4 c4 d4
	mm1 = _mm_movepi64_pi64(xmm3); // a6 b6 c6 d6

	// mm0= a4, mm1= a6, mm3= a0, mm6= a2
	mm4 = mm3; // a0
	mm7 = mm6; // a2

	mm3 = _mm_adds_pi16(mm3, mm0); // a0+a4 = a[0]
	mm4 = _mm_subs_pi16(mm4, mm0); // a0-a4 = a[4]
	mm7 = _mm_srai_pi16(mm7, 1);   // a2>>1
	mm7 = _mm_subs_pi16(mm7, mm1); // (a2>>1) - a6 = a[2]
	mm1 = _mm_srai_pi16(mm1, 1);   // a6>>1
	mm6 = _mm_adds_pi16(mm6, mm1); // a2 + (a6>>1) = a[6]

	mm0 = mm3; // a[0]
	mm1 = mm4; // a[4]

	mm0 = _mm_adds_pi16(mm0, mm6); // a[0] + a[6] = b[0]
	mm1 = _mm_adds_pi16(mm1, mm7); // a[4] + a[2] = b[2]
	mm3 = _mm_subs_pi16(mm3, mm6); // a[0] - a[6] = b[6]
	mm4 = _mm_subs_pi16(mm4, mm7); // a[4] - a[2] = b[4]
	// 34
	// xmm0= b[0], xmm1= b[2], xmm2= a7, xmm3= b[6], xmm4 = b[4], xmm5= a5, src[16] = a1, src[24] = a3
	xmm2 = xmm0; // a1
	xmm4 = xmm1; // a3
	xmm5 = xmm6; // a5
	xmm7 = xmm3; // a7

	xmm2 = _mm_srai_epi16(xmm2, 1); // a1>>1
	xmm4 = _mm_srai_epi16(xmm4, 1); // a3>>1
	xmm5 = _mm_srai_epi16(xmm5, 1); // a5>>1
	xmm7 = _mm_srai_epi16(xmm7, 1); // a7>>1

	xmm2 = _mm_adds_epi16(xmm2, xmm0); // a1 + (a1>>1)
	xmm4 = _mm_adds_epi16(xmm4, xmm1); // a3 + (a3>>1)
	xmm5 = _mm_adds_epi16(xmm5, xmm6); // a5 + (a5>>1)
	xmm7 = _mm_adds_epi16(xmm7, xmm3); // a7 + (a7>>1)

	xmm2 = _mm_adds_epi16(xmm2, xmm6); //  a5 + a1 + (a1>>1)
	xmm4 = _mm_subs_epi16(xmm4, xmm3); // -a7 + a3 + (a3>>1)
	xmm5 = _mm_subs_epi16(xmm5, xmm0); // -a1 + a5 + (a5>>1)
	xmm7 = _mm_adds_epi16(xmm7, xmm1); //  a3 + a7 + (a7>>1)

	xmm1 = _mm_adds_epi16(xmm1, xmm2); //  a3 + a5 + a1 + (a1>>1) = a[7]
	xmm0 = _mm_subs_epi16(xmm0, xmm4); //  a1 + a7 - a3 - (a3>>1) = a[3]
	xmm3 = _mm_adds_epi16(xmm3, xmm5); //  a7 - a1 + a5 + (a5>>1) = a[5]
	xmm6 = _mm_subs_epi16(xmm6, xmm7); //  a5 - a3 - a7 - (a7>>1) = a[1]
	// 54
	// xmm2= a[5], xmm5= a[1], xmm6 = a[3], xmm7= a[7], src[32] = b[0], src[40] = b[2], src[48] = b[4], src[56] = b[6]
	xmm2 = xmm1; // a[7]
	xmm4 = xmm3; // a[5]

	xmm2 = _mm_srai_epi16(xmm2, 2); // a[7]>>2
	xmm4 = _mm_srai_epi16(xmm4, 2); // a[5]>>2
	xmm2 = _mm_adds_epi16(xmm2, xmm6); //  a[1] + (a[7]>>2) = b[1]
	xmm4 = _mm_adds_epi16(xmm4, xmm0); //  a[3] + (a[5]>>2) = b[3]

	xmm0 = _mm_srai_epi16(xmm0, 2); // a[3]>>2
	xmm6 = _mm_srai_epi16(xmm6, 2); // a[1]>>2
	xmm0 = _mm_subs_epi16(xmm0, xmm3); //  (a[3]>>2) - a[5] = b[5]
	xmm1 = _mm_subs_epi16(xmm1, xmm6); //  a[7] - (a[1]>>2) = b[7]

	xmm2 = _mm_srli_si128(xmm2,8); // b[1]
	xmm4 = _mm_srli_si128(xmm4,8); // b[3]
	xmm0 = _mm_srli_si128(xmm0,8); // b[5]
	xmm1 = _mm_srli_si128(xmm1,8); // b[7]

	xmm3 = _mm_movpi64_epi64(mm0); // b[0]
	xmm5 = _mm_movpi64_epi64(mm1); // b[2]

	xmm6 = xmm3; // b[0]
	xmm7 = xmm5; // b[2]

	xmm6 = _mm_adds_epi16(xmm6, xmm1); // b[0] + b[7] = A(0) B(0) C(0) D(0)
	xmm3 = _mm_subs_epi16(xmm3, xmm1); // b[0] - b[7] = A(7) B(7) C(7) D(7)
	xmm7 = _mm_adds_epi16(xmm7, xmm0); // b[2] + b[5] = A(1) B(1) C(1) D(1)
	xmm5 = _mm_subs_epi16(xmm5, xmm0); // b[2] - b[5] = A(6) B(6) C(6) D(6)

	xmm6 = _mm_unpacklo_epi16(xmm6, xmm7); // A(0) A(1) B(0) B(1) C(0) C(1) D(0) D(1)
	xmm5 = _mm_unpacklo_epi16(xmm5, xmm3); // A(6) A(7) B(6) B(7) C(6) C(7) D(6) D(7)

	xmm0 = _mm_movpi64_epi64(mm4); // b[4]
	xmm1 = _mm_movpi64_epi64(mm3); // b[6]
	xmm7 = xmm0; // b[4]
	xmm3 = xmm1; // b[6]

	xmm7 = _mm_adds_epi16(xmm7, xmm4); // b[4] + b[3] = A(2) B(2) C(2) D(2)
	xmm0 = _mm_subs_epi16(xmm0, xmm4); // b[4] - b[3] = A(5) B(5) C(5) D(5)
	xmm3 = _mm_adds_epi16(xmm3, xmm2); // b[6] + b[1] = A(3) B(3) C(3) D(3)
	xmm1 = _mm_subs_epi16(xmm1, xmm2); // b[6] - b[1] = A(4) B(4) C(4) D(4)

	xmm7 = _mm_unpacklo_epi16(xmm7, xmm3); // A(2) A(3) B(2) B(3) C(2) C(3) D(2) D(3)
	xmm1 = _mm_unpacklo_epi16(xmm1, xmm0); // A(4) A(5) B(4) B(5) C(4) C(5) D(4) D(5)

	xmm4 = xmm6; // A(0) A(1) B(0) B(1) C(0) C(1) D(0) D(1)
	xmm2 = xmm1; // A(4) A(5) B(4) B(5) C(4) C(5) D(4) D(5)

	xmm4 = _mm_unpacklo_epi32(xmm4, xmm7); // A(0) A(1) A(2) A(3) B(0) B(1) B(2) B(3)
	xmm2 = _mm_unpacklo_epi32(xmm2, xmm5); // A(4) A(5) A(6) A(7) B(4) B(5) B(6) B(7)
	xmm6 = _mm_unpackhi_epi32(xmm6, xmm7); // C(0) C(1) C(2) C(3) D(0) D(1) D(2) D(3)
	xmm1 = _mm_unpackhi_epi32(xmm1, xmm5); // C(4) C(5) C(6) C(7) D(4) D(5) D(6) D(7)

	xmm3 = xmm4; // A(0) A(1) A(2) A(3) B(0) B(1) B(2) B(3)
	xmm0 = xmm6; // C(0) C(1) C(2) C(3) D(0) D(1) D(2) D(3)

	xmm3 = _mm_unpacklo_epi64(xmm3, xmm2); // A(0) A(1) A(2) A(3) A(4) A(5) A(6) A(7)
	xmm0 = _mm_unpacklo_epi64(xmm0, xmm1); // C(0) C(1) C(2) C(3) C(4) C(5) C(6) C(7)
	xmm4 = _mm_unpackhi_epi64(xmm4, xmm2); // B(0) B(1) B(2) B(3) B(4) B(5) B(6) B(7)
	xmm6 = _mm_unpackhi_epi64(xmm6, xmm1); // D(0) D(1) D(2) D(3) D(4) D(5) D(6) D(7)
	// 100
	/*********************/
	_mm_store_si128((__m128i*)&src[0 ], xmm3); //a 0 1 2 3 4 5 6 7
	_mm_store_si128((__m128i*)&src[8 ], xmm4); //b 0 1 2 3 4 5 6 7
	_mm_store_si128((__m128i*)&src[16], xmm0); //c 0 1 2 3 4 5 6 7
	_mm_store_si128((__m128i*)&src[24], xmm6); //d 0 1 2 3 4 5 6 7

	// In the following, a, b, c, d are substituted by e, f, g, h
	xmm0 = _mm_load_si128((__m128i*)&src[32]); //a 0 1 2 3 4 5 6 7
	xmm1 = _mm_load_si128((__m128i*)&src[40]); //b 0 1 2 3 4 5 6 7
	xmm2 = _mm_load_si128((__m128i*)&src[48]); //c 0 1 2 3 4 5 6 7
	xmm3 = _mm_load_si128((__m128i*)&src[56]); //d 0 1 2 3 4 5 6 7

	xmm6 = xmm0; //a 0 1 2 3 4 5 6 7
	xmm7 = xmm2; //c 0 1 2 3 4 5 6 7

	xmm0 = _mm_unpacklo_epi16(xmm0,xmm1); // a0 b0 a1 b1 a2 b2 a3 b3
	xmm2 = _mm_unpacklo_epi16(xmm2,xmm3); // c0 d0 c1 d1 c2 d2 c3 d3
	xmm6 = _mm_unpackhi_epi16(xmm6,xmm1); // a4 b4 a5 b5 a6 b6 a7 b7
	xmm7 = _mm_unpackhi_epi16(xmm7,xmm3); // c4 d4 c5 d5 c6 d6 c7 d7

	xmm1 = xmm0; // a0 b0 a1 b1 a2 b2 a3 b3
	xmm3 = xmm6; // a4 b4 a5 b5 a6 b6 a7 b7

	xmm0 = _mm_unpacklo_epi32(xmm0,xmm2); // a0 b0 c0 d0 a1 b1 c1 d1
	xmm6 = _mm_unpacklo_epi32(xmm6,xmm7); // a4 b4 c4 d4 a5 b5 c5 d5
	xmm1 = _mm_unpackhi_epi32(xmm1,xmm2); // a2 b2 c2 d2 a3 b3 c3 d3
	xmm3 = _mm_unpackhi_epi32(xmm3,xmm7); // a6 b6 c6 d6 a7 b7 c7 d7
	// 
	mm3 = _mm_movepi64_pi64(xmm0); // a0 b0 c0 d0
	mm6 = _mm_movepi64_pi64(xmm1); // a2 b2 c2 d2
	mm0 = _mm_movepi64_pi64(xmm6); // a4 b4 c4 d4
	mm1 = _mm_movepi64_pi64(xmm3); // a6 b6 c6 d6

	// mm0= a4, mm1= a6, mm3= a0, mm6= a2
	mm4 = mm3; // a0
	mm7 = mm6; // a2

	mm3 = _mm_adds_pi16(mm3, mm0); // a0+a4 = a[0]
	mm4 = _mm_subs_pi16(mm4, mm0); // a0-a4 = a[4]
	mm7 = _mm_srai_pi16(mm7, 1);   // a2>>1
	mm7 = _mm_subs_pi16(mm7, mm1); // (a2>>1) - a6 = a[2]
	mm1 = _mm_srai_pi16(mm1, 1);   // a6>>1
	mm6 = _mm_adds_pi16(mm6, mm1); // a2 + (a6>>1) = a[6]

	mm0 = mm3; // a[0]
	mm1 = mm4; // a[4]

	mm0 = _mm_adds_pi16(mm0, mm6); // a[0] + a[6] = b[0]
	mm1 = _mm_adds_pi16(mm1, mm7); // a[4] + a[2] = b[2]
	mm3 = _mm_subs_pi16(mm3, mm6); // a[0] - a[6] = b[6]
	mm4 = _mm_subs_pi16(mm4, mm7); // a[4] - a[2] = b[4]
	// 
	// xmm0= b[0], xmm1= b[2], xmm2= a7, xmm3= b[6], xmm4 = b[4], xmm5= a5, src[16] = a1, src[24] = a3
	xmm2 = xmm0; // a1
	xmm4 = xmm1; // a3
	xmm5 = xmm6; // a5
	xmm7 = xmm3; // a7

	xmm2 = _mm_srai_epi16(xmm2, 1); // a1>>1
	xmm4 = _mm_srai_epi16(xmm4, 1); // a3>>1
	xmm5 = _mm_srai_epi16(xmm5, 1); // a5>>1
	xmm7 = _mm_srai_epi16(xmm7, 1); // a7>>1

	xmm2 = _mm_adds_epi16(xmm2, xmm0); // a1 + (a1>>1)
	xmm4 = _mm_adds_epi16(xmm4, xmm1); // a3 + (a3>>1)
	xmm5 = _mm_adds_epi16(xmm5, xmm6); // a5 + (a5>>1)
	xmm7 = _mm_adds_epi16(xmm7, xmm3); // a7 + (a7>>1)

	xmm2 = _mm_adds_epi16(xmm2, xmm6); //  a5 + a1 + (a1>>1)
	xmm4 = _mm_subs_epi16(xmm4, xmm3); // -a7 + a3 + (a3>>1)
	xmm5 = _mm_subs_epi16(xmm5, xmm0); // -a1 + a5 + (a5>>1)
	xmm7 = _mm_adds_epi16(xmm7, xmm1); //  a3 + a7 + (a7>>1)

	xmm1 = _mm_adds_epi16(xmm1, xmm2); //  a3 + a5 + a1 + (a1>>1) = a[7]
	xmm0 = _mm_subs_epi16(xmm0, xmm4); //  a1 + a7 - a3 - (a3>>1) = a[3]
	xmm3 = _mm_adds_epi16(xmm3, xmm5); //  a7 - a1 + a5 + (a5>>1) = a[5]
	xmm6 = _mm_subs_epi16(xmm6, xmm7); //  a5 - a3 - a7 - (a7>>1) = a[1]
	// 
	// xmm2= a[5], xmm5= a[1], xmm6 = a[3], xmm7= a[7], src[32] = b[0], src[40] = b[2], src[48] = b[4], src[56] = b[6]
	xmm2 = xmm1; // a[7]
	xmm4 = xmm3; // a[5]

	xmm2 = _mm_srai_epi16(xmm2, 2); // a[7]>>2
	xmm4 = _mm_srai_epi16(xmm4, 2); // a[5]>>2
	xmm2 = _mm_adds_epi16(xmm2, xmm6); //  a[1] + (a[7]>>2) = b[1]
	xmm4 = _mm_adds_epi16(xmm4, xmm0); //  a[3] + (a[5]>>2) = b[3]

	xmm0 = _mm_srai_epi16(xmm0, 2); // a[3]>>2
	xmm6 = _mm_srai_epi16(xmm6, 2); // a[1]>>2
	xmm0 = _mm_subs_epi16(xmm0, xmm3); //  (a[3]>>2) - a[5] = b[5]
	xmm1 = _mm_subs_epi16(xmm1, xmm6); //  a[7] - (a[1]>>2) = b[7]

	xmm2 = _mm_srli_si128(xmm2,8); // b[1]
	xmm4 = _mm_srli_si128(xmm4,8); // b[3]
	xmm0 = _mm_srli_si128(xmm0,8); // b[5]
	xmm1 = _mm_srli_si128(xmm1,8); // b[7]

	xmm3 = _mm_movpi64_epi64(mm0); // b[0]
	xmm5 = _mm_movpi64_epi64(mm1); // b[2]

	xmm6 = xmm3; // b[0]
	xmm7 = xmm5; // b[2]

	xmm6 = _mm_adds_epi16(xmm6, xmm1); // b[0] + b[7] = A(0) B(0) C(0) D(0)
	xmm3 = _mm_subs_epi16(xmm3, xmm1); // b[0] - b[7] = A(7) B(7) C(7) D(7)
	xmm7 = _mm_adds_epi16(xmm7, xmm0); // b[2] + b[5] = A(1) B(1) C(1) D(1)
	xmm5 = _mm_subs_epi16(xmm5, xmm0); // b[2] - b[5] = A(6) B(6) C(6) D(6)

	xmm6 = _mm_unpacklo_epi16(xmm6, xmm7); // A(0) A(1) B(0) B(1) C(0) C(1) D(0) D(1)
	xmm5 = _mm_unpacklo_epi16(xmm5, xmm3); // A(6) A(7) B(6) B(7) C(6) C(7) D(6) D(7)

	xmm0 = _mm_movpi64_epi64(mm4); // b[4]
	xmm1 = _mm_movpi64_epi64(mm3); // b[6]
	xmm7 = xmm0; // b[4]
	xmm3 = xmm1; // b[6]

	xmm7 = _mm_adds_epi16(xmm7, xmm4); // b[4] + b[3] = A(2) B(2) C(2) D(2)
	xmm0 = _mm_subs_epi16(xmm0, xmm4); // b[4] - b[3] = A(5) B(5) C(5) D(5)
	xmm3 = _mm_adds_epi16(xmm3, xmm2); // b[6] + b[1] = A(3) B(3) C(3) D(3)
	xmm1 = _mm_subs_epi16(xmm1, xmm2); // b[6] - b[1] = A(4) B(4) C(4) D(4)

	xmm7 = _mm_unpacklo_epi16(xmm7, xmm3); // A(2) A(3) B(2) B(3) C(2) C(3) D(2) D(3)
	xmm1 = _mm_unpacklo_epi16(xmm1, xmm0); // A(4) A(5) B(4) B(5) C(4) C(5) D(4) D(5)

	xmm4 = xmm6; // A(0) A(1) B(0) B(1) C(0) C(1) D(0) D(1)
	xmm2 = xmm1; // A(4) A(5) B(4) B(5) C(4) C(5) D(4) D(5)

	xmm4 = _mm_unpacklo_epi32(xmm4, xmm7); // A(0) A(1) A(2) A(3) B(0) B(1) B(2) B(3)
	xmm2 = _mm_unpacklo_epi32(xmm2, xmm5); // A(4) A(5) A(6) A(7) B(4) B(5) B(6) B(7)
	xmm6 = _mm_unpackhi_epi32(xmm6, xmm7); // C(0) C(1) C(2) C(3) D(0) D(1) D(2) D(3)
	xmm1 = _mm_unpackhi_epi32(xmm1, xmm5); // C(4) C(5) C(6) C(7) D(4) D(5) D(6) D(7)

	xmm3 = xmm4; // A(0) A(1) A(2) A(3) B(0) B(1) B(2) B(3)
	xmm0 = xmm6; // C(0) C(1) C(2) C(3) D(0) D(1) D(2) D(3)

	xmm3 = _mm_unpacklo_epi64(xmm3, xmm2); // A(0) A(1) A(2) A(3) A(4) A(5) A(6) A(7)
	xmm0 = _mm_unpacklo_epi64(xmm0, xmm1); // C(0) C(1) C(2) C(3) C(4) C(5) C(6) C(7)
	xmm4 = _mm_unpackhi_epi64(xmm4, xmm2); // B(0) B(1) B(2) B(3) B(4) B(5) B(6) B(7)
	xmm6 = _mm_unpackhi_epi64(xmm6, xmm1); // D(0) D(1) D(2) D(3) D(4) D(5) D(6) D(7)
	// 100 + 4 + 100 = 204
	// ********************* //
	// xmm0 = mm[6], xmm3 = mm[4], xmm4 = mm[5], xmm6 = mm[7], src[0] = mm[0], src[1] = mm[1], src[2] = mm[2], src[3] = mm[3]
	xmm1 = _mm_load_si128((__m128i*)&src[0 ]); //mm[0]
	xmm2 = _mm_load_si128((__m128i*)&src[16]); //mm[2]

	xmm5 = xmm1; //mm[0]
	xmm7 = xmm2; //mm[2]

	xmm5 = _mm_adds_epi16(xmm5, xmm3); // mm[0] + mm[4] = a[0]
	xmm1 = _mm_subs_epi16(xmm1, xmm3); // mm[0] - mm[4] = a[4]
	xmm7 = _mm_srai_epi16(xmm7, 1);    // mm[2]>>1
	xmm7 = _mm_subs_epi16(xmm7, xmm0); // (mm[2]>>1) - mm[6] = a[2]
	xmm0 = _mm_srai_epi16(xmm0, 1);    // mm[6]>>1
	xmm2 = _mm_adds_epi16(xmm2, xmm0); // mm[2] + (mm[6]>>1) = a[6]
	// 214
	// xmm1 = a[4], xmm2 = a[6], xmm5 = a[0], xmm7 = a[2], src[8] = mm[1], src[24] = mm[3]
	xmm3 = xmm5; // a[0]
	xmm0 = xmm1; // a[4]

	xmm3 = _mm_adds_epi16(xmm3, xmm2); // a[0] + a[6] = b[0]
	xmm0 = _mm_adds_epi16(xmm0, xmm7); // a[4] + a[2] = b[2]
	xmm5 = _mm_subs_epi16(xmm5, xmm2); // a[0] - a[6] = b[6]
	xmm1 = _mm_subs_epi16(xmm1, xmm7); // a[4] - a[2] = b[4]

	_mm_store_si128((__m128i*)&src[32], xmm3); // b[0]
	_mm_store_si128((__m128i*)&src[40], xmm0); // b[2]
	_mm_store_si128((__m128i*)&src[48], xmm1); // b[4]
	_mm_store_si128((__m128i*)&src[56], xmm5); // b[6]

	xmm2 = _mm_load_si128((__m128i*)&src[8 ]); // mm[1]
	xmm7 = _mm_load_si128((__m128i*)&src[24]); // mm[3]
	// 226
	// xmm4 = mm[5] xmm6 = mm[7]
	xmm0 = xmm2; // a1
	xmm1 = xmm7; // a3
	xmm3 = xmm4; // a5
	xmm5 = xmm6; // a7

	xmm0 = _mm_srai_epi16(xmm0, 1); // a1>>1
	xmm1 = _mm_srai_epi16(xmm1, 1); // a3>>1
	xmm3 = _mm_srai_epi16(xmm3, 1); // a5>>1
	xmm5 = _mm_srai_epi16(xmm5, 1); // a7>>1

	xmm0 = _mm_adds_epi16(xmm0, xmm2); // a1 + (a1>>1)
	xmm1 = _mm_adds_epi16(xmm1, xmm7); // a3 + (a3>>1)
	xmm3 = _mm_adds_epi16(xmm3, xmm4); // a5 + (a5>>1)
	xmm5 = _mm_adds_epi16(xmm5, xmm6); // a7 + (a7>>1)

	xmm0 = _mm_adds_epi16(xmm0, xmm4); //  a5 + a1 + (a1>>1)
	xmm1 = _mm_subs_epi16(xmm1, xmm6); // -a7 + a3 + (a3>>1)
	xmm3 = _mm_subs_epi16(xmm3, xmm2); // -a1 + a5 + (a5>>1)
	xmm5 = _mm_adds_epi16(xmm5, xmm7); //  a3 + a7 + (a7>>1)

	xmm7 = _mm_adds_epi16(xmm7, xmm0); //  a3 + a5 + a1 + (a1>>1) = a[7]
	xmm2 = _mm_subs_epi16(xmm2, xmm1); //  a1 + a7 - a3 - (a3>>1) = a[3]
	xmm6 = _mm_adds_epi16(xmm6, xmm3); //  a7 - a1 + a5 + (a5>>1) = a[5]
	xmm4 = _mm_subs_epi16(xmm4, xmm5); //  a5 - a3 - a7 - (a7>>1) = a[1]
	// 246
	xmm0 = xmm7; // a[7]
	xmm1 = xmm6; // a[5]

	xmm0 = _mm_srai_epi16(xmm0, 2); // a[7]>>2
	xmm1 = _mm_srai_epi16(xmm1, 2); // a[5]>>2
	xmm0 = _mm_adds_epi16(xmm0, xmm4); //  a[1] + (a[7]>>2) = b[1]
	xmm1 = _mm_adds_epi16(xmm1, xmm2); //  a[3] + (a[5]>>2) = b[3]

	xmm4 = _mm_srai_epi16(xmm4, 2); // a[1]>>2
	xmm2 = _mm_srai_epi16(xmm2, 2); // a[3]>>2
	xmm7 = _mm_subs_epi16(xmm7, xmm4); //  a[7] - (a[1]>>2) = b[7]
	xmm2 = _mm_subs_epi16(xmm2, xmm6); //  (a[3]>>2) - a[5] = b[5]
	// 256
	// xmm0 = b[1], xmm1 = b[3], xmm2 = b[5], xmm7 = b[7]
	xmm3 = _mm_load_si128((__m128i*)&src[32]); // b[0]
	xmm4 = _mm_load_si128((__m128i*)&src[40]); // b[2]
	xmm5 = xmm3; // b[0]
	xmm6 = xmm4; // b[2]

	xmm5 = _mm_adds_epi16(xmm5, xmm7); //  b[0] + b[7]
	xmm3 = _mm_subs_epi16(xmm3, xmm7); //  b[0] - b[7]
	xmm6 = _mm_adds_epi16(xmm6, xmm2); //  b[2] + b[5]
	xmm4 = _mm_subs_epi16(xmm4, xmm2); //  b[2] - b[5]
	// 264
	// xmm0 = b[1], xmm1 = b[3], xmm3 = ROW[7], xmm4 = ROW[6], xmm5 = ROW[0], xmm6 = ROW[1]
	xmm7 = _mm_load_si128((__m128i*)&const_32); // all 32s
	xmm2 = _mm_loadl_epi64 ((__m128i *) (pred+0  )); // pred[0]
	xmm5 = _mm_adds_epi16(xmm5, xmm7); //  b[0] + b[7] + 32
	xmm5 = _mm_srai_epi16(xmm5, 6); // (b[0] + b[7] + 32)>>6
	xmm3 = _mm_adds_epi16(xmm3, xmm7); //  b[0] - b[7] + 32
	xmm3 = _mm_srai_epi16(xmm3, 6); // (b[0] - b[7] + 32)>>6
	xmm6 = _mm_adds_epi16(xmm6, xmm7); //  b[2] + b[5] + 32
	xmm6 = _mm_srai_epi16(xmm6, 6); // (b[2] + b[5] + 32)>>6
	xmm4 = _mm_adds_epi16(xmm4, xmm7); //  b[2] - b[5] + 32
	xmm4 = _mm_srai_epi16(xmm4, 6); // (b[2] - b[5] + 32)>>6
	// 274
	// xmm0 = b[1], xmm1 = b[3], xmm2 = pred[0], xmm3 = ROW[7], xmm4 = ROW[6], xmm5 = ROW[0], xmm6 = ROW[1], xmm7 = "32"
	// src[48] = b[4], src[56] = b[6]
	xmm7 = _mm_setzero_si128(); // all 0s
	xmm2 = _mm_unpacklo_epi8(xmm2,xmm7);
	xmm2 = _mm_adds_epi16(xmm2, xmm5); //  (b[0] + b[7] + 32)>>6 + pred[0]
	xmm2 = _mm_packus_epi16(xmm2,xmm2);
	_mm_storel_epi64((__m128i*)&dest[0*stride], xmm2);

	xmm5 = _mm_loadl_epi64 ((__m128i *) (pred+16 )); // pred[1]
	xmm5 = _mm_unpacklo_epi8(xmm5,xmm7);
	xmm5 = _mm_adds_epi16(xmm5, xmm6); //  (b[2] + b[5] + 32)>>6 + pred[1]
	xmm5 = _mm_packus_epi16(xmm5,xmm5);
	_mm_storel_epi64((__m128i*)&dest[1*stride], xmm5);
	// 284
	// xmm0 = b[1], xmm1 = b[3], xmm3 = ROW[7], xmm4 = ROW[6], xmm7 = 0
	// src[48] = b[4], src[56] = b[6]
	xmm2 = _mm_load_si128((__m128i*)&src[48]); // b[4]
	xmm5 = xmm2; // b[4]
	xmm2 = _mm_adds_epi16(xmm2, xmm1); //  b[4] + b[3]
	xmm5 = _mm_subs_epi16(xmm5, xmm1); //  b[4] - b[3]
	xmm6 = _mm_load_si128((__m128i*)&const_32); // all 32s
	xmm2 = _mm_adds_epi16(xmm2, xmm6); //  b[4] + b[3] + 32
	xmm2 = _mm_srai_epi16(xmm2, 6); // (b[4] + b[3] + 32)>>6
	xmm5 = _mm_adds_epi16(xmm5, xmm6); //  b[4] - b[3] + 32
	xmm5 = _mm_srai_epi16(xmm5, 6); // (b[4] - b[3] + 32)>>6
	xmm1 = _mm_loadl_epi64 ((__m128i *) (pred+32  )); // pred[2]
	xmm1 = _mm_unpacklo_epi8(xmm1,xmm7);
	xmm1 = _mm_adds_epi16(xmm1, xmm2); //  (b[4] + b[3] + 32)>>6 + pred[2]
	xmm1 = _mm_packus_epi16(xmm1,xmm1);
	_mm_storel_epi64((__m128i*)&dest[2*stride], xmm1);
	// 298
	// xmm0 = b[1], xmm3 = ROW[7], xmm4 = ROW[6], xmm5 = ROW[5], xmm6 = "32", xmm7 = 0
	// src[56] = b[6]
	xmm2 = _mm_load_si128((__m128i*)&src[56]); // b[6]
	xmm1 = xmm2; // b[6]
	xmm2 = _mm_adds_epi16(xmm2, xmm0); //  b[6] + b[1]
	xmm1 = _mm_subs_epi16(xmm1, xmm0); //  b[6] - b[1]
	xmm2 = _mm_adds_epi16(xmm2, xmm6); //  b[6] + b[1] + 32
	xmm2 = _mm_srai_epi16(xmm2, 6); // (b[6] + b[1] + 32)>>6
	xmm1 = _mm_adds_epi16(xmm1, xmm6); //  b[6] - b[1] + 32
	xmm1 = _mm_srai_epi16(xmm1, 6); // (b[6] - b[1] + 32)>>6
	xmm0 = _mm_loadl_epi64 ((__m128i *) (pred+48  )); // pred[3]
	xmm0 = _mm_unpacklo_epi8(xmm0,xmm7);
	xmm0 = _mm_adds_epi16(xmm0, xmm2); //  (b[6] + b[1] + 32)>>6 + pred[3]
	xmm0 = _mm_packus_epi16(xmm0,xmm0);
	_mm_storel_epi64((__m128i*)&dest[3*stride], xmm0);
	// 311
	// xmm1 = ROW[4], xmm3 = ROW[7], xmm4 = ROW[6], xmm5 = ROW[5], xmm6 = "32", xmm7 = 0
	xmm2 = _mm_loadl_epi64 ((__m128i *) (pred+64  )); // pred[4]
	xmm2 = _mm_unpacklo_epi8(xmm2,xmm7);
	xmm2 = _mm_adds_epi16(xmm2, xmm1); //  (b[6] - b[1] + 32)>>6 + pred[4]
	xmm2 = _mm_packus_epi16(xmm2,xmm2);
	_mm_storel_epi64((__m128i*)&dest[4*stride], xmm2);

	xmm0 = _mm_loadl_epi64 ((__m128i *) (pred+80  )); // pred[5]
	xmm0 = _mm_unpacklo_epi8(xmm0,xmm7);
	xmm0 = _mm_adds_epi16(xmm0, xmm5); //  (b[4] - b[3] + 32)>>6 + pred[5]
	xmm0 = _mm_packus_epi16(xmm0,xmm0);
	_mm_storel_epi64((__m128i*)&dest[5*stride], xmm0);

	xmm2 = _mm_loadl_epi64 ((__m128i *) (pred+96  )); // pred[6]
	xmm2 = _mm_unpacklo_epi8(xmm2,xmm7);
	xmm2 = _mm_adds_epi16(xmm2, xmm4); //  (b[2] - b[5] + 32)>>6 + pred[6]
	xmm2 = _mm_packus_epi16(xmm2,xmm2);
	_mm_storel_epi64((__m128i*)&dest[6*stride], xmm2);

	xmm0 = _mm_loadl_epi64 ((__m128i *) (pred+112 )); // pred[7]
	xmm0 = _mm_unpacklo_epi8(xmm0,xmm7);
	xmm0 = _mm_adds_epi16(xmm0, xmm3); //  (b[0] - b[7] + 32)>>6 + pred[7]
	xmm0 = _mm_packus_epi16(xmm0,xmm0);
	_mm_storel_epi64((__m128i*)&dest[7*stride], xmm0);
	// 331
	_mm_store_si128((__m128i*)&src[0 ], xmm7); //0
	_mm_store_si128((__m128i*)&src[8 ], xmm7); //0
	_mm_store_si128((__m128i*)&src[16], xmm7); //0
	_mm_store_si128((__m128i*)&src[24], xmm7); //0
	_mm_store_si128((__m128i*)&src[32], xmm7); //0
	_mm_store_si128((__m128i*)&src[40], xmm7); //0
	_mm_store_si128((__m128i*)&src[48], xmm7); //0
	_mm_store_si128((__m128i*)&src[56], xmm7); //0
	// 339
}
#elif 0
// New code, Mix MMX-SSE2 inline assembly
void inverse_transform8x8_sse2 PARGS4(byte *dest, byte *pred, short *src, int stride)
{
	__m128i xmm0,xmm1,xmm2,xmm3,xmm4,xmm5,xmm6,xmm7;
	__m64   mm0, mm1, mm2, mm3, mm4, mm5, mm6, mm7;
	static unsigned short __declspec(align(16)) const_32[8] = { 32, 32, 32, 32, 32, 32, 32, 32 };

	// Transpose 4x8 matrix
	//start
	_asm 
	{
		mov        eax,       src;       // eax = src
		movdqa     xmm0,      [eax+0  ]; //a 0 1 2 3 4 5 6 7
		movdqa     xmm1,      [eax+16 ]; //b 0 1 2 3 4 5 6 7
		movdqa     xmm2,      [eax+32 ]; //c 0 1 2 3 4 5 6 7
		movdqa     xmm3,      [eax+48 ]; //d 0 1 2 3 4 5 6 7

		movdqa     xmm6,      xmm0;      //a 0 1 2 3 4 5 6 7
		movdqa     xmm7,      xmm2;      //c 0 1 2 3 4 5 6 7

		punpcklwd  xmm0,      xmm1;      // a0 b0 a1 b1 a2 b2 a3 b3
		punpcklwd  xmm2,      xmm3;      // c0 d0 c1 d1 c2 d2 c3 d3
		punpckhwd  xmm6,      xmm1;      // a4 b4 a5 b5 a6 b6 a7 b7
		punpckhwd  xmm7,      xmm3;      // c4 d4 c5 d5 c6 d6 c7 d7

		movdqa     xmm1,      xmm0;      // a0 b0 a1 b1 a2 b2 a3 b3
		movdqa     xmm3,      xmm6;      // a4 b4 a5 b5 a6 b6 a7 b7

		punpckldq  xmm0,      xmm2;      // a0 b0 c0 d0 a1 b1 c1 d1
		punpckldq  xmm6,      xmm7;      // a4 b4 c4 d4 a5 b5 c5 d5
		punpckhdq  xmm1,      xmm2;      // a2 b2 c2 d2 a3 b3 c3 d3
		punpckhdq  xmm3,      xmm7;      // a6 b6 c6 d6 a7 b7 c7 d7
		// 16
		movdq2q    mm3,       xmm0;      // a0 b0 c0 d0
		movdq2q    mm6,       xmm1;      // a2 b2 c2 d2
		movdq2q    mm0,       xmm6;      // a4 b4 c4 d4
		movdq2q    mm1,       xmm3;      // a6 b6 c6 d6

		// mm0= a4, mm1= a6, mm3= a0, mm6= a2
		movq       mm4,       mm3;       // a0
		movq       mm7,       mm6;       // a2

		paddsw     mm3,       mm0;       // a0+a4 = a[0]
		psubsw     mm4,       mm0;       // a0-a4 = a[4]
		psraw      mm7,       1;         // a2>>1
		psubsw     mm7,       mm1;       // (a2>>1) - a6 = a[2]
		psraw      mm1,       1;         // a6>>1
		paddsw     mm6,       mm1;       // a2 + (a6>>1) = a[6]

		movq       mm0,       mm3;       // a[0]
		movq       mm1,       mm4;       // a[4]

		paddsw     mm0,       mm6;       // a[0] + a[6] = b[0]
		paddsw     mm1,       mm7;       // a[4] + a[2] = b[2]
		psubsw     mm3,       mm6;       // a[0] - a[6] = b[6]
		psubsw     mm4,       mm7;       // a[4] - a[2] = b[4]
		// 34
		// xmm0= b[0], xmm1= b[2], xmm2= a7, xmm3= b[6], xmm4 = b[4], xmm5= a5, src[16] = a1, src[24] = a3
		movdqa     xmm2,      xmm0;      // a1
		movdqa     xmm4,      xmm1;      // a3
		movdqa     xmm5,      xmm6;      // a5
		movdqa     xmm7,      xmm3;      // a7

		psraw      xmm2,      1;         // a1>>1
		psraw      xmm4,      1;         // a3>>1
		psraw      xmm5,      1;         // a5>>1
		psraw      xmm7,      1;         // a7>>1

		paddsw     xmm2,      xmm0;      // a1 + (a1>>1)
		paddsw     xmm4,      xmm1;      // a3 + (a3>>1)
		paddsw     xmm5,      xmm6;      // a5 + (a5>>1)
		paddsw     xmm7,      xmm3;      // a7 + (a7>>1)

		paddsw     xmm2,      xmm6;      //  a5 + a1 + (a1>>1)
		psubsw     xmm4,      xmm3;      // -a7 + a3 + (a3>>1)
		psubsw     xmm5,      xmm0;      // -a1 + a5 + (a5>>1)
		paddsw     xmm7,      xmm1;      //  a3 + a7 + (a7>>1)

		paddsw     xmm1,      xmm2;      //  a3 + a5 + a1 + (a1>>1) = a[7]
		psubsw     xmm0,      xmm4;      //  a1 + a7 - a3 - (a3>>1) = a[3]
		paddsw     xmm3,      xmm5;      //  a7 - a1 + a5 + (a5>>1) = a[5]
		psubsw     xmm6,      xmm7;      //  a5 - a3 - a7 - (a7>>1) = a[1]
		// 54
		// xmm2= a[5], xmm5= a[1], xmm6 = a[3], xmm7= a[7], src[32] = b[0], src[40] = b[2], src[48] = b[4], src[56] = b[6]
		movdqa     xmm2,      xmm1;      // a[7]
		movdqa     xmm4,      xmm3;      // a[5]

		psraw      xmm2,      2;         // a[7]>>2
		psraw      xmm4,      2;         // a[5]>>2
		paddsw     xmm2,      xmm6;      // a[1] + (a[7]>>2) = b[1]
		paddsw     xmm4,      xmm0;      // a[3] + (a[5]>>2) = b[3]

		psraw      xmm0,      2;         // a[3]>>2
		psraw      xmm6,      2;         // a[1]>>2
		psubsw     xmm0,      xmm3;      // (a[3]>>2) - a[5] = b[5]
		psubsw     xmm1,      xmm6;      // a[7] - (a[1]>>2) = b[7]

		psrldq     xmm2,      8;         // b[1]
		psrldq     xmm4,      8;         // b[3]
		psrldq     xmm0,      8;         // b[5]
		psrldq     xmm1,      8;         // b[7]

		movq2dq    xmm3,      mm0;       // b[0]
		movq2dq    xmm5,      mm1;       // b[2]

		movdqa     xmm6,      xmm3;      // b[0]
		movdqa     xmm7,      xmm5;      // b[2]

		paddsw     xmm6,      xmm1;      // b[0] + b[7] = A(0) B(0) C(0) D(0)
		psubsw     xmm3,      xmm1;      // b[0] - b[7] = A(7) B(7) C(7) D(7)
		paddsw     xmm7,      xmm0;      // b[2] + b[5] = A(1) B(1) C(1) D(1)
		psubsw     xmm5,      xmm0;      // b[2] - b[5] = A(6) B(6) C(6) D(6)

		punpcklwd  xmm6,      xmm7;      // A(0) A(1) B(0) B(1) C(0) C(1) D(0) D(1)
		punpcklwd  xmm5,      xmm3;      // A(6) A(7) B(6) B(7) C(6) C(7) D(6) D(7)

		movq2dq    xmm0,      mm4;       // b[4]
		movq2dq    xmm1,      mm3;       // b[6]
		movdqa     xmm7,      xmm0;      // b[4]
		movdqa     xmm3,      xmm1;      // b[6]

		paddsw     xmm7,      xmm4;      // b[4] + b[3] = A(2) B(2) C(2) D(2)
		psubsw     xmm0,      xmm4;      // b[4] - b[3] = A(5) B(5) C(5) D(5)
		paddsw     xmm3,      xmm2;      // b[6] + b[1] = A(3) B(3) C(3) D(3)
		psubsw     xmm1,      xmm2;      // b[6] - b[1] = A(4) B(4) C(4) D(4)

		punpcklwd  xmm7,      xmm3;      // A(2) A(3) B(2) B(3) C(2) C(3) D(2) D(3)
		punpcklwd  xmm1,      xmm0;      // A(4) A(5) B(4) B(5) C(4) C(5) D(4) D(5)

		movdqa     xmm4,      xmm6;      // A(0) A(1) B(0) B(1) C(0) C(1) D(0) D(1)
		movdqa     xmm2,      xmm1;      // A(4) A(5) B(4) B(5) C(4) C(5) D(4) D(5)

		punpckldq  xmm4,      xmm7;      // A(0) A(1) A(2) A(3) B(0) B(1) B(2) B(3)
		punpckldq  xmm2,      xmm5;      // A(4) A(5) A(6) A(7) B(4) B(5) B(6) B(7)
		punpckhdq  xmm6,      xmm7;      // C(0) C(1) C(2) C(3) D(0) D(1) D(2) D(3)
		punpckhdq  xmm1,      xmm5;      // C(4) C(5) C(6) C(7) D(4) D(5) D(6) D(7)

		movdqa     xmm3,      xmm4;      // A(0) A(1) A(2) A(3) B(0) B(1) B(2) B(3)
		movdqa     xmm0,      xmm6;      // C(0) C(1) C(2) C(3) D(0) D(1) D(2) D(3)

		punpcklqdq xmm3,      xmm2;      // A(0) A(1) A(2) A(3) A(4) A(5) A(6) A(7)
		punpcklqdq xmm0,      xmm1;      // C(0) C(1) C(2) C(3) C(4) C(5) C(6) C(7)
		punpckhqdq xmm4,      xmm2;      // B(0) B(1) B(2) B(3) B(4) B(5) B(6) B(7)
		punpckhqdq xmm6,      xmm1;      // D(0) D(1) D(2) D(3) D(4) D(5) D(6) D(7)
		// 100
		/*********************/
		movdqa     [eax+0  ], xmm3;      //a 0 1 2 3 4 5 6 7
		movdqa     [eax+16 ], xmm4;      //b 0 1 2 3 4 5 6 7
		movdqa     [eax+32 ], xmm0;      //c 0 1 2 3 4 5 6 7
		movdqa     [eax+48 ], xmm6;      //d 0 1 2 3 4 5 6 7

		// In the following, a, b, c, d are substituted by e, f, g, h
		movdqa     xmm0,      [eax+64 ]; //a 0 1 2 3 4 5 6 7
		movdqa     xmm1,      [eax+80 ]; //b 0 1 2 3 4 5 6 7
		movdqa     xmm2,      [eax+96 ]; //c 0 1 2 3 4 5 6 7
		movdqa     xmm3,      [eax+112]; //d 0 1 2 3 4 5 6 7

		movdqa     xmm6,      xmm0;      //a 0 1 2 3 4 5 6 7
		movdqa     xmm7,      xmm2;      //c 0 1 2 3 4 5 6 7

		punpcklwd  xmm0,      xmm1;      // a0 b0 a1 b1 a2 b2 a3 b3
		punpcklwd  xmm2,      xmm3;      // c0 d0 c1 d1 c2 d2 c3 d3
		punpckhwd  xmm6,      xmm1;      // a4 b4 a5 b5 a6 b6 a7 b7
		punpckhwd  xmm7,      xmm3;      // c4 d4 c5 d5 c6 d6 c7 d7

		movdqa     xmm1,      xmm0;      // a0 b0 a1 b1 a2 b2 a3 b3
		movdqa     xmm3,      xmm6;      // a4 b4 a5 b5 a6 b6 a7 b7

		punpckldq  xmm0,      xmm2;      // a0 b0 c0 d0 a1 b1 c1 d1
		punpckldq  xmm6,      xmm7;      // a4 b4 c4 d4 a5 b5 c5 d5
		punpckhdq  xmm1,      xmm2;      // a2 b2 c2 d2 a3 b3 c3 d3
		punpckhdq  xmm3,      xmm7;      // a6 b6 c6 d6 a7 b7 c7 d7
		// 16
		movdq2q    mm3,       xmm0;      // a0 b0 c0 d0
		movdq2q    mm6,       xmm1;      // a2 b2 c2 d2
		movdq2q    mm0,       xmm6;      // a4 b4 c4 d4
		movdq2q    mm1,       xmm3;      // a6 b6 c6 d6

		// mm0= a4, mm1= a6, mm3= a0, mm6= a2
		movq       mm4,       mm3;       // a0
		movq       mm7,       mm6;       // a2

		paddsw     mm3,       mm0;       // a0+a4 = a[0]
		psubsw     mm4,       mm0;       // a0-a4 = a[4]
		psraw      mm7,       1;         // a2>>1
		psubsw     mm7,       mm1;       // (a2>>1) - a6 = a[2]
		psraw      mm1,       1;         // a6>>1
		paddsw     mm6,       mm1;       // a2 + (a6>>1) = a[6]

		movq       mm0,       mm3;       // a[0]
		movq       mm1,       mm4;       // a[4]

		paddsw     mm0,       mm6;       // a[0] + a[6] = b[0]
		paddsw     mm1,       mm7;       // a[4] + a[2] = b[2]
		psubsw     mm3,       mm6;       // a[0] - a[6] = b[6]
		psubsw     mm4,       mm7;       // a[4] - a[2] = b[4]
		// 34
		// xmm0= b[0], xmm1= b[2], xmm2= a7, xmm3= b[6], xmm4 = b[4], xmm5= a5, src[16] = a1, src[24] = a3
		movdqa     xmm2,      xmm0;      // a1
		movdqa     xmm4,      xmm1;      // a3
		movdqa     xmm5,      xmm6;      // a5
		movdqa     xmm7,      xmm3;      // a7

		psraw      xmm2,      1;         // a1>>1
		psraw      xmm4,      1;         // a3>>1
		psraw      xmm5,      1;         // a5>>1
		psraw      xmm7,      1;         // a7>>1

		paddsw     xmm2,      xmm0;      // a1 + (a1>>1)
		paddsw     xmm4,      xmm1;      // a3 + (a3>>1)
		paddsw     xmm5,      xmm6;      // a5 + (a5>>1)
		paddsw     xmm7,      xmm3;      // a7 + (a7>>1)

		paddsw     xmm2,      xmm6;      //  a5 + a1 + (a1>>1)
		psubsw     xmm4,      xmm3;      // -a7 + a3 + (a3>>1)
		psubsw     xmm5,      xmm0;      // -a1 + a5 + (a5>>1)
		paddsw     xmm7,      xmm1;      //  a3 + a7 + (a7>>1)

		paddsw     xmm1,      xmm2;      //  a3 + a5 + a1 + (a1>>1) = a[7]
		psubsw     xmm0,      xmm4;      //  a1 + a7 - a3 - (a3>>1) = a[3]
		paddsw     xmm3,      xmm5;      //  a7 - a1 + a5 + (a5>>1) = a[5]
		psubsw     xmm6,      xmm7;      //  a5 - a3 - a7 - (a7>>1) = a[1]
		// 54
		// xmm2= a[5], xmm5= a[1], xmm6 = a[3], xmm7= a[7], src[32] = b[0], src[40] = b[2], src[48] = b[4], src[56] = b[6]
		movdqa     xmm2,      xmm1;      // a[7]
		movdqa     xmm4,      xmm3;      // a[5]

		psraw      xmm2,      2;         // a[7]>>2
		psraw      xmm4,      2;         // a[5]>>2
		paddsw     xmm2,      xmm6;      // a[1] + (a[7]>>2) = b[1]
		paddsw     xmm4,      xmm0;      // a[3] + (a[5]>>2) = b[3]

		psraw      xmm0,      2;         // a[3]>>2
		psraw      xmm6,      2;         // a[1]>>2
		psubsw     xmm0,      xmm3;      // (a[3]>>2) - a[5] = b[5]
		psubsw     xmm1,      xmm6;      // a[7] - (a[1]>>2) = b[7]

		psrldq     xmm2,      8;         // b[1]
		psrldq     xmm4,      8;         // b[3]
		psrldq     xmm0,      8;         // b[5]
		psrldq     xmm1,      8;         // b[7]

		movq2dq    xmm3,      mm0;       // b[0]
		movq2dq    xmm5,      mm1;       // b[2]

		movdqa     xmm6,      xmm3;      // b[0]
		movdqa     xmm7,      xmm5;      // b[2]

		paddsw     xmm6,      xmm1;      // b[0] + b[7] = A(0) B(0) C(0) D(0)
		psubsw     xmm3,      xmm1;      // b[0] - b[7] = A(7) B(7) C(7) D(7)
		paddsw     xmm7,      xmm0;      // b[2] + b[5] = A(1) B(1) C(1) D(1)
		psubsw     xmm5,      xmm0;      // b[2] - b[5] = A(6) B(6) C(6) D(6)

		punpcklwd  xmm6,      xmm7;      // A(0) A(1) B(0) B(1) C(0) C(1) D(0) D(1)
		punpcklwd  xmm5,      xmm3;      // A(6) A(7) B(6) B(7) C(6) C(7) D(6) D(7)

		movq2dq    xmm0,      mm4;       // b[4]
		movq2dq    xmm1,      mm3;       // b[6]
		movdqa     xmm7,      xmm0;      // b[4]
		movdqa     xmm3,      xmm1;      // b[6]

		paddsw     xmm7,      xmm4;      // b[4] + b[3] = A(2) B(2) C(2) D(2)
		psubsw     xmm0,      xmm4;      // b[4] - b[3] = A(5) B(5) C(5) D(5)
		paddsw     xmm3,      xmm2;      // b[6] + b[1] = A(3) B(3) C(3) D(3)
		psubsw     xmm1,      xmm2;      // b[6] - b[1] = A(4) B(4) C(4) D(4)

		punpcklwd  xmm7,      xmm3;      // A(2) A(3) B(2) B(3) C(2) C(3) D(2) D(3)
		punpcklwd  xmm1,      xmm0;      // A(4) A(5) B(4) B(5) C(4) C(5) D(4) D(5)

		movdqa     xmm4,      xmm6;      // A(0) A(1) B(0) B(1) C(0) C(1) D(0) D(1)
		movdqa     xmm2,      xmm1;      // A(4) A(5) B(4) B(5) C(4) C(5) D(4) D(5)

		punpckldq  xmm4,      xmm7;      // A(0) A(1) A(2) A(3) B(0) B(1) B(2) B(3)
		punpckldq  xmm2,      xmm5;      // A(4) A(5) A(6) A(7) B(4) B(5) B(6) B(7)
		punpckhdq  xmm6,      xmm7;      // C(0) C(1) C(2) C(3) D(0) D(1) D(2) D(3)
		punpckhdq  xmm1,      xmm5;      // C(4) C(5) C(6) C(7) D(4) D(5) D(6) D(7)

		movdqa     xmm3,      xmm4;      // A(0) A(1) A(2) A(3) B(0) B(1) B(2) B(3)
		movdqa     xmm0,      xmm6;      // C(0) C(1) C(2) C(3) D(0) D(1) D(2) D(3)

		punpcklqdq xmm3,      xmm2;      // A(0) A(1) A(2) A(3) A(4) A(5) A(6) A(7)
		punpcklqdq xmm0,      xmm1;      // C(0) C(1) C(2) C(3) C(4) C(5) C(6) C(7)
		punpckhqdq xmm4,      xmm2;      // B(0) B(1) B(2) B(3) B(4) B(5) B(6) B(7)
		punpckhqdq xmm6,      xmm1;      // D(0) D(1) D(2) D(3) D(4) D(5) D(6) D(7)
		// 100 + 4 + 100 = 204
		// ********************* //
		// xmm0 = mm[6], xmm3 = mm[4], xmm4 = mm[5], xmm6 = mm[7], src[0] = mm[0], src[1] = mm[1], src[2] = mm[2], src[3] = mm[3]
		movdqa     xmm1,      [eax+0  ]; // mm[0]
		movdqa     xmm2,      [eax+32 ]; // mm[2]

		movdqa     xmm5,      xmm1;      // mm[0]
		movdqa     xmm7,      xmm2;      // mm[2]

		paddsw     xmm5,      xmm3;      // mm[0] + mm[4] = a[0]
		psubsw     xmm1,      xmm3;      // mm[0] - mm[4] = a[4]
		psraw      xmm7,      1;         // mm[2]>>1
		psubsw     xmm7,      xmm0;      // (mm[2]>>1) - mm[6] = a[2]
		psraw      xmm0,      1;         // mm[6]>>1
		paddsw     xmm2,      xmm0;      // mm[2] + (mm[6]>>1) = a[6]
		// 214
		// xmm1 = a[4], xmm2 = a[6], xmm5 = a[0], xmm7 = a[2], src[8] = mm[1], src[24] = mm[3]
		movdqa     xmm3,      xmm5;      // a[0]
		movdqa     xmm0,      xmm1;      // a[4]

		paddsw     xmm3,      xmm2;      // a[0] + a[6] = b[0]
		paddsw     xmm0,      xmm7;      // a[4] + a[2] = b[2]
		psubsw     xmm5,      xmm2;      // a[0] - a[6] = b[6]
		psubsw     xmm1,      xmm7;      // a[4] - a[2] = b[4]

		movdqa     [eax+64 ], xmm3;      // b[0]
		movdqa     [eax+80 ], xmm0;      // b[2]
		movdqa     [eax+96 ], xmm1;      // b[4]
		movdqa     [eax+112], xmm5;      // b[6]

		movdqa     xmm2,      [eax+16 ]; // mm[1]
		movdqa     xmm7,      [eax+48 ]; // mm[3]
		// 226
		// xmm4 = mm[5] xmm6 = mm[7]
		movdqa     xmm0,      xmm2;      // a1
		movdqa     xmm1,      xmm7;      // a3
		movdqa     xmm3,      xmm4;      // a5
		movdqa     xmm5,      xmm6;      // a7

		psraw      xmm0,      1;         // a1>>1
		psraw      xmm1,      1;         // a3>>1
		psraw      xmm3,      1;         // a5>>1
		psraw      xmm5,      1;         // a7>>1

		paddsw     xmm0,      xmm2;      // a1 + (a1>>1)
		paddsw     xmm1,      xmm7;      // a3 + (a3>>1)
		paddsw     xmm3,      xmm4;      // a5 + (a5>>1)
		paddsw     xmm5,      xmm6;      // a7 + (a7>>1)

		paddsw     xmm0,      xmm4;      //  a5 + a1 + (a1>>1)
		psubsw     xmm1,      xmm6;      // -a7 + a3 + (a3>>1)
		psubsw     xmm3,      xmm2;      // -a1 + a5 + (a5>>1)
		paddsw     xmm5,      xmm7;      //  a3 + a7 + (a7>>1)

		paddsw     xmm7,      xmm0;      //  a3 + a5 + a1 + (a1>>1) = a[7]
		psubsw     xmm2,      xmm1;      //  a1 + a7 - a3 - (a3>>1) = a[3]
		paddsw     xmm6,      xmm3;      //  a7 - a1 + a5 + (a5>>1) = a[5]
		psubsw     xmm4,      xmm5;      //  a5 - a3 - a7 - (a7>>1) = a[1]
		// 246
		movdqa     xmm0,      xmm7;      // a[7]
		movdqa     xmm1,      xmm6;      // a[5]

		psraw      xmm0,      2;         // a[7]>>2
		psraw      xmm1,      2;         // a[5]>>2
		paddsw     xmm0,      xmm4;      // a[1] + (a[7]>>2) = b[1]
		paddsw     xmm1,      xmm2;      // a[3] + (a[5]>>2) = b[3]

		psraw      xmm4,      2;         // a[1]>>2
		psraw      xmm2,      2;         // a[3]>>2
		psubsw     xmm7,      xmm4;      // a[7] - (a[1]>>2) = b[7]
		psubsw     xmm2,      xmm6;      // (a[3]>>2) - a[5] = b[5]
		// 256
		// xmm0 = b[1], xmm1 = b[3], xmm2 = b[5], xmm7 = b[7]
		movdqa     xmm3,      [eax+64 ]; // b[0]
		movdqa     xmm4,      [eax+80 ]; // b[2]
		movdqa     xmm5,      xmm3;      // b[0]
		movdqa     xmm6,      xmm4;      // b[2]

		paddsw     xmm5,      xmm7;      // b[0] + b[7]
		psubsw     xmm3,      xmm7;      // b[0] - b[7]
		paddsw     xmm6,      xmm2;      // b[2] + b[5]
		psubsw     xmm4,      xmm2;      // b[2] - b[5]
		// 264
		// xmm0 = b[1], xmm1 = b[3], xmm3 = ROW[7], xmm4 = ROW[6], xmm5 = ROW[0], xmm6 = ROW[1]
		mov        ecx,       pred;      // ecx = pred
		mov        edx,       dest;      // edx = dest
		movdqa     xmm7,      const_32[0]; // all 32s
		movq       xmm2,      [ecx];     // pred[0]
		paddsw     xmm5,      xmm7;      //  b[0] + b[7] + 32
		psraw      xmm5,      6;         // (b[0] + b[7] + 32)>>6
		paddsw     xmm3,      xmm7;      //  b[0] - b[7] + 32
		psraw      xmm3,      6;         // (b[0] - b[7] + 32)>>6
		paddsw     xmm6,      xmm7;      //  b[2] + b[5] + 32
		psraw      xmm6,      6;         // (b[2] + b[5] + 32)>>6
		paddsw     xmm4,      xmm7;      //  b[2] - b[5] + 32
		psraw      xmm4,      6;         // (b[2] - b[5] + 32)>>6
		// 274
		// xmm0 = b[1], xmm1 = b[3], xmm2 = pred[0], xmm3 = ROW[7], xmm4 = ROW[6], xmm5 = ROW[0], xmm6 = ROW[1], xmm7 = "32"
		// src[48] = b[4], src[56] = b[6]
		pxor       xmm7,      xmm7;      // all 0s
		punpcklbw  xmm2,      xmm7;
		paddsw     xmm2,      xmm5;      //  (b[0] + b[7] + 32)>>6 + pred[0]
		packuswb   xmm2,      xmm2;
		movq       [edx],     xmm2;

		mov        esi,       stride;
		lea        edi,       [esi+2*esi];// 3*stride
		movq       xmm5,      [ecx+16 ]; // pred[1]
		punpcklbw  xmm5,      xmm7;
		paddsw     xmm5,      xmm6;      //  (b[2] + b[5] + 32)>>6 + pred[1]
		packuswb   xmm5,      xmm5;
		movq       [edx+esi], xmm5;
		// 284
		// xmm0 = b[1], xmm1 = b[3], xmm3 = ROW[7], xmm4 = ROW[6], xmm7 = 0
		// src[48] = b[4], src[56] = b[6]
		movdqa     xmm2,      [eax+96 ]; // b[4]
		movdqa     xmm5,      xmm2;      // b[4]
		paddsw     xmm2,      xmm1;      //  b[4] + b[3]
		psubsw     xmm5,      xmm1;      //  b[4] - b[3]
		movdqa     xmm6,      const_32[0];// all 32s
		paddsw     xmm2,      xmm6;      //  b[4] + b[3] + 32
		psraw      xmm2,      6;         // (b[4] + b[3] + 32)>>6
		paddsw     xmm5,      xmm6;      //  b[4] - b[3] + 32
		psraw      xmm5,      6;         // (b[4] - b[3] + 32)>>6
		movq       xmm1,      [ecx+32 ]; // pred[2]
		punpcklbw  xmm1,      xmm7;
		paddsw     xmm1,      xmm2;      //  (b[4] + b[3] + 32)>>6 + pred[2]
		packuswb   xmm1,      xmm1;
		movq       [edx+2*esi],xmm1;
		// 298
		// xmm0 = b[1], xmm3 = ROW[7], xmm4 = ROW[6], xmm5 = ROW[5], xmm6 = "32", xmm7 = 0
		// src[56] = b[6]
		movdqa     xmm2,      [eax+112]; // b[6]
		movdqa     xmm1,      xmm2;      // b[6]
		paddsw     xmm2,      xmm0;      //  b[6] + b[1]
		psubsw     xmm1,      xmm0;      //  b[6] - b[1]
		paddsw     xmm2,      xmm6;      //  b[6] + b[1] + 32
		psraw      xmm2,      6;         // (b[6] + b[1] + 32)>>6
		paddsw     xmm1,      xmm6;      //  b[6] - b[1] + 32
		psraw      xmm1,      6;         // (b[6] - b[1] + 32)>>6
		movq       xmm0,      [ecx+48 ]; // pred[3]
		punpcklbw  xmm0,      xmm7;
		paddsw     xmm0,      xmm2;      //  (b[6] + b[1] + 32)>>6 + pred[3]
		packuswb   xmm0,      xmm0;
		movq       [edx+edi], xmm0;
		// 311
		// xmm1 = ROW[4], xmm3 = ROW[7], xmm4 = ROW[6], xmm5 = ROW[5], xmm6 = "32", xmm7 = 0
		lea        edx,       [edx+4*esi];// edx = &dest[4*stride]
		movq       xmm2,      [ecx+64 ]; // pred[4]
		punpcklbw  xmm2,      xmm7;
		paddsw     xmm2,      xmm1;      //  (b[6] - b[1] + 32)>>6 + pred[4]
		packuswb   xmm2,      xmm2;
		movq       [edx],     xmm2;

		movq       xmm0,      [ecx+80 ]; // pred[5]
		punpcklbw  xmm0,      xmm7;
		paddsw     xmm0,      xmm5;      //  (b[4] - b[3] + 32)>>6 + pred[5]
		packuswb   xmm0,      xmm0;
		movq       [edx+esi], xmm0;

		movq       xmm2,      [ecx+96 ]; // pred[6]
		punpcklbw  xmm2,      xmm7;
		paddsw     xmm2,      xmm4;      //  (b[2] - b[5] + 32)>>6 + pred[6]
		packuswb   xmm2,      xmm2;
		movq       [edx+2*esi],xmm2;

		movq       xmm0,      [ecx+112]; // pred[7]
		punpcklbw  xmm0,      xmm7;
		paddsw     xmm0,      xmm3;      //  (b[0] - b[7] + 32)>>6 + pred[7]
		packuswb   xmm0,      xmm0;
		movq       [edx+edi], xmm0;
		// 331
		movdqa     [eax+0  ], xmm7;      //0
		movdqa     [eax+16 ], xmm7;      //0
		movdqa     [eax+32 ], xmm7;      //0
		movdqa     [eax+48 ], xmm7;      //0
		movdqa     [eax+64 ], xmm7;      //0
		movdqa     [eax+80 ], xmm7;      //0
		movdqa     [eax+96 ], xmm7;      //0
		movdqa     [eax+112], xmm7;      //0
		// 339
	}
}
#elif 0
// New code, SSE2 intrinsics
void inverse_transform8x8_sse2 PARGS4(byte *dest, byte *pred, short *src, int stride)
{
	__m128i xmm0,xmm1,xmm2,xmm3,xmm4,xmm5,xmm6,xmm7;
	static unsigned short __declspec(align(16)) const_32[8] = { 32, 32, 32, 32, 32, 32, 32, 32 };
	static unsigned char *ptr_32 = (unsigned char *) &const_32[0];

#ifdef _PRE_TRANSPOSE_
	xmm3 = _mm_load_si128((__m128i*)&src[0 ]); // a0 b0 c0 d0 e0 f0 g0 h0
	xmm6 = _mm_load_si128((__m128i*)&src[16]); // a2 b2 c2 d2 e2 f2 g2 h2
	xmm0 = _mm_load_si128((__m128i*)&src[32]); // a4 b4 c4 d4 e4 f4 g4 h4
	xmm5 = _mm_load_si128((__m128i*)&src[40]); // a5 b5 c5 d5 e5 f5 g5 h5
	xmm1 = _mm_load_si128((__m128i*)&src[48]); // a6 b6 c6 d6 e6 f6 g6 h6
	xmm2 = _mm_load_si128((__m128i*)&src[56]); // a7 b7 c7 d7 e7 f7 g7 h7
	// 6
#else
	// Transpose 8x8 matrix
	//start	
	//__asm int 3;
	xmm0 = _mm_load_si128((__m128i*)&src[0 ]); //a 0 1 2 3 4 5 6 7
	xmm1 = _mm_load_si128((__m128i*)&src[8 ]); //b 0 1 2 3 4 5 6 7
	xmm2 = _mm_load_si128((__m128i*)&src[16]); //c 0 1 2 3 4 5 6 7
	xmm3 = _mm_load_si128((__m128i*)&src[24]); //d 0 1 2 3 4 5 6 7

	xmm6 = xmm0; //a 0 1 2 3 4 5 6 7
	xmm7 = xmm2; //c 0 1 2 3 4 5 6 7

	xmm0 = _mm_unpacklo_epi16(xmm0,xmm1); // a0 b0 a1 b1 a2 b2 a3 b3
	xmm2 = _mm_unpacklo_epi16(xmm2,xmm3); // c0 d0 c1 d1 c2 d2 c3 d3
	xmm6 = _mm_unpackhi_epi16(xmm6,xmm1); // a4 b4 a5 b5 a6 b6 a7 b7
	xmm7 = _mm_unpackhi_epi16(xmm7,xmm3); // c4 d4 c5 d5 c6 d6 c7 d7

	xmm1 = xmm0; // a0 b0 a1 b1 a2 b2 a3 b3
	xmm3 = xmm6; // a4 b4 a5 b5 a6 b6 a7 b7

	xmm0 = _mm_unpacklo_epi32(xmm0,xmm2); // a0 b0 c0 d0 a1 b1 c1 d1
	xmm6 = _mm_unpacklo_epi32(xmm6,xmm7); // a4 b4 c4 d4 a5 b5 c5 d5
	xmm1 = _mm_unpackhi_epi32(xmm1,xmm2); // a2 b2 c2 d2 a3 b3 c3 d3
	xmm3 = _mm_unpackhi_epi32(xmm3,xmm7); // a6 b6 c6 d6 a7 b7 c7 d7
	// 16
	// xmm0 = abcd-0/1, xmm1 = abcd-2/3, xmm3 = abcd-6/7, xmm6 = abcd-4/5
	_mm_store_si128((__m128i*)&src[0 ], xmm6); // a4 b4 c4 d4 a5 b5 c5 d5
	_mm_store_si128((__m128i*)&src[16], xmm3); // a6 b6 c6 d6 a7 b7 c7 d7

	xmm4 = _mm_load_si128((__m128i*)&src[32]); //e 0 1 2 3 4 5 6 7
	xmm5 = _mm_load_si128((__m128i*)&src[40]); //f 0 1 2 3 4 5 6 7
	xmm6 = _mm_load_si128((__m128i*)&src[48]); //g 0 1 2 3 4 5 6 7
	xmm7 = _mm_load_si128((__m128i*)&src[56]); //h 0 1 2 3 4 5 6 7

	xmm2 = xmm4; //e 0 1 2 3 4 5 6 7
	xmm3 = xmm6; //g 0 1 2 3 4 5 6 7

	xmm2 = _mm_unpacklo_epi16(xmm2,xmm5); // e0 f0 e1 f1 e2 f2 e3 f3
	xmm3 = _mm_unpacklo_epi16(xmm3,xmm7); // g0 h0 g1 h1 g2 h2 g3 h3
	xmm4 = _mm_unpackhi_epi16(xmm4,xmm5); // e4 f4 e5 f5 e6 f6 e7 f7
	xmm6 = _mm_unpackhi_epi16(xmm6,xmm7); // g4 h4 g5 h5 g6 h6 g7 h7

	xmm5 = xmm2; // e0 f0 e1 f1 e2 f2 e3 f3
	xmm7 = xmm4; // e4 f4 e5 f5 e6 f6 e7 f7

	xmm5 = _mm_unpacklo_epi32(xmm5,xmm3); // e0 f0 g0 h0 e1 f1 g1 h1
	xmm7 = _mm_unpacklo_epi32(xmm7,xmm6); // e4 f4 g4 h4 e5 f5 g5 h5
	xmm2 = _mm_unpackhi_epi32(xmm2,xmm3); // e2 f2 g2 h2 e3 f3 g3 h3
	xmm4 = _mm_unpackhi_epi32(xmm4,xmm6); // e6 f6 g6 h6 e7 f7 g7 h7

	xmm3 = xmm0; // a0 b0 c0 d0 a1 b1 c1 d1
	xmm6 = xmm1; // a2 b2 c2 d2 a3 b3 c3 d3
	// 36
	// xmm0= abcd-0/1, xmm1= abcd-2/3, xmm2= efgh-2/3, xmm3= abcd-0/1, xmm4= efgh-6/7, xmm5= efgh-0/1, xmm6= abcd-2/3, xmm7= efgh-4/5
	// src[0]= abcd-4/5, src[8]= abcd-6/7
	xmm3 = _mm_unpacklo_epi64(xmm3,xmm5); // a0 b0 c0 d0 e0 f0 g0 h0
	xmm6 = _mm_unpacklo_epi64(xmm6,xmm2); // a2 b2 c2 d2 e2 f2 g2 h2
	xmm0 = _mm_unpackhi_epi64(xmm0,xmm5); // a1 b1 c1 d1 e1 f1 g1 h1
	xmm1 = _mm_unpackhi_epi64(xmm1,xmm2); // a3 b3 c3 d3 e3 f3 g3 h3

	_mm_store_si128((__m128i*)&src[8 ], xmm0); // a1 b1 c1 d1 e1 f1 g1 h1
	_mm_store_si128((__m128i*)&src[24], xmm1); // a3 b3 c3 d3 e3 f3 g3 h3

	xmm5 = _mm_load_si128((__m128i*)&src[0 ]); // a4 b4 c4 d4 a5 b5 c5 d5
	xmm2 = _mm_load_si128((__m128i*)&src[16]); // a6 b6 c6 d6 a7 b7 c7 d7

	xmm0 = xmm5; // a4 b4 c4 d4 a5 b5 c5 d5
	xmm1 = xmm2; // a6 b6 c6 d6 a7 b7 c7 d7

	xmm0 = _mm_unpacklo_epi64(xmm0,xmm7); // a4 b4 c4 d4 e4 f4 g4 h4
	xmm1 = _mm_unpacklo_epi64(xmm1,xmm4); // a6 b6 c6 d6 e6 f6 g6 h6
	xmm5 = _mm_unpackhi_epi64(xmm5,xmm7); // a5 b5 c5 d5 e5 f5 g5 h5
	xmm2 = _mm_unpackhi_epi64(xmm2,xmm4); // a7 b7 c7 d7 e7 f7 g7 h7
	// 50
#endif
	// xmm0= a4, xmm1= a6, xmm2= a7, xmm3= a0, xmm5= a5, xmm6= a2, src[16] = a1, src[24] = a3
	xmm4 = xmm3; // a0
	xmm7 = xmm6; // a2

	xmm3 = _mm_adds_epi16(xmm3, xmm0); // a0+a4 = a[0]
	xmm4 = _mm_subs_epi16(xmm4, xmm0); // a0-a4 = a[4]
	xmm7 = _mm_srai_epi16(xmm7, 1); // a2>>1
	xmm7 = _mm_subs_epi16(xmm7, xmm1); // (a2>>1) - a6 = a[2]
	xmm1 = _mm_srai_epi16(xmm1, 1); // a6>>1
	xmm6 = _mm_adds_epi16(xmm6, xmm1); // a2 + (a6>>1) = a[6]

	xmm0 = xmm3; // a[0]
	xmm1 = xmm4; // a[4]

	xmm0 = _mm_adds_epi16(xmm0, xmm6); // a[0] + a[6] = b[0]
	xmm1 = _mm_adds_epi16(xmm1, xmm7); // a[4] + a[2] = b[2]
	xmm3 = _mm_subs_epi16(xmm3, xmm6); // a[0] - a[6] = b[6]
	xmm4 = _mm_subs_epi16(xmm4, xmm7); // a[4] - a[2] = b[4]
	// 64
	// xmm0= b[0], xmm1= b[2], xmm2= a7, xmm3= b[6], xmm4 = b[4], xmm5= a5, src[16] = a1, src[24] = a3
	_mm_store_si128((__m128i*)&src[32], xmm0); // b[0]
	_mm_store_si128((__m128i*)&src[40], xmm1); // b[2]
	_mm_store_si128((__m128i*)&src[48], xmm4); // b[4]
	_mm_store_si128((__m128i*)&src[56], xmm3); // b[6]

	xmm6 = _mm_load_si128((__m128i*)&src[8 ]); // a1
	xmm7 = _mm_load_si128((__m128i*)&src[24]); // a3

	xmm0 = xmm6; // a1
	xmm1 = xmm7; // a3
	xmm3 = xmm5; // a5
	xmm4 = xmm2; // a7

	xmm0 = _mm_srai_epi16(xmm0, 1); // a1>>1
	xmm1 = _mm_srai_epi16(xmm1, 1); // a3>>1
	xmm3 = _mm_srai_epi16(xmm3, 1); // a5>>1
	xmm4 = _mm_srai_epi16(xmm4, 1); // a7>>1

	xmm0 = _mm_adds_epi16(xmm0, xmm6); // a1 + (a1>>1)
	xmm1 = _mm_adds_epi16(xmm1, xmm7); // a3 + (a3>>1)
	xmm3 = _mm_adds_epi16(xmm3, xmm5); // a5 + (a5>>1)
	xmm4 = _mm_adds_epi16(xmm4, xmm2); // a7 + (a7>>1)

	xmm0 = _mm_adds_epi16(xmm0, xmm5); //  a5 + a1 + (a1>>1)
	xmm1 = _mm_subs_epi16(xmm1, xmm2); // -a7 + a3 + (a3>>1)
	xmm3 = _mm_subs_epi16(xmm3, xmm6); // -a1 + a5 + (a5>>1)
	xmm4 = _mm_adds_epi16(xmm4, xmm7); //  a3 + a7 + (a7>>1)

	xmm7 = _mm_adds_epi16(xmm7, xmm0); //  a3 + a5 + a1 + (a1>>1) = a[7]
	xmm6 = _mm_subs_epi16(xmm6, xmm1); //  a1 + a7 - a3 - (a3>>1) = a[3]
	xmm2 = _mm_adds_epi16(xmm2, xmm3); //  a7 - a1 + a5 + (a5>>1) = a[5]
	xmm5 = _mm_subs_epi16(xmm5, xmm4); //  a5 - a3 - a7 - (a7>>1) = a[1]
	// 90
	// xmm2= a[5], xmm5= a[1], xmm6 = a[3], xmm7= a[7], src[32] = b[0], src[40] = b[2], src[48] = b[4], src[56] = b[6]
	xmm0 = xmm7; // a[7]
	xmm1 = xmm2; // a[5]

	xmm0 = _mm_srai_epi16(xmm0, 2); // a[7]>>2
	xmm1 = _mm_srai_epi16(xmm1, 2); // a[5]>>2
	xmm0 = _mm_adds_epi16(xmm0, xmm5); //  a[1] + (a[7]>>2) = b[1]
	xmm1 = _mm_adds_epi16(xmm1, xmm6); //  a[3] + (a[5]>>2) = b[3]

	xmm6 = _mm_srai_epi16(xmm6, 2); // a[3]>>2
	xmm5 = _mm_srai_epi16(xmm5, 2); // a[1]>>2
	xmm6 = _mm_subs_epi16(xmm6, xmm2); //  (a[3]>>2) - a[5] = b[5]
	xmm7 = _mm_subs_epi16(xmm7, xmm5); //  a[7] - (a[1]>>2) = b[7]
	// 100
	// xmm0 = b[1], xmm1 = b[3], xmm6 = b[5], xmm7= b[7], src[32] = b[0], src[40] = b[2], src[48] = b[4], src[56] = b[6]
	xmm2 = _mm_load_si128((__m128i*)&src[32]); // b[0]
	xmm3 = _mm_load_si128((__m128i*)&src[40]); // b[2]
	xmm4 = xmm2; // b[0]
	xmm5 = xmm3; // b[2]

	xmm2 = _mm_adds_epi16(xmm2, xmm7); //  b[0] + b[7]
	xmm4 = _mm_subs_epi16(xmm4, xmm7); //  b[0] - b[7]
	xmm3 = _mm_adds_epi16(xmm3, xmm6); //  b[2] + b[5]
	xmm5 = _mm_subs_epi16(xmm5, xmm6); //  b[2] - b[5]

	_mm_store_si128((__m128i*)&src[0 ], xmm2); // MM0
	_mm_store_si128((__m128i*)&src[8 ], xmm3); // MM1

	xmm6 = _mm_load_si128((__m128i*)&src[48]); // b[4]
	xmm7 = _mm_load_si128((__m128i*)&src[56]); // b[6]
	xmm2 = xmm6; // b[4]
	xmm3 = xmm7; // b[6]

	xmm2 = _mm_adds_epi16(xmm2, xmm1); //  b[4] + b[3]
	xmm6 = _mm_subs_epi16(xmm6, xmm1); //  b[4] - b[3]
	xmm3 = _mm_adds_epi16(xmm3, xmm0); //  b[6] + b[1]
	xmm7 = _mm_subs_epi16(xmm7, xmm0); //  b[6] - b[1]
	// 118
	// xmm2 = MM2, xmm3 = MM3, xmm4 = MM7, xmm5 = MM6, xmm6 = MM5, xmm7 = MM4, src[0] = MM0, src[8] = MM1
	xmm0 = xmm7; //e 0 1 2 3 4 5 6 7
	xmm1 = xmm5; //g 0 1 2 3 4 5 6 7

	xmm0 = _mm_unpacklo_epi16(xmm0,xmm6); // e0 f0 e1 f1 e2 f2 e3 f3
	xmm1 = _mm_unpacklo_epi16(xmm1,xmm4); // g0 h0 g1 h1 g2 h2 g3 h3
	xmm7 = _mm_unpackhi_epi16(xmm7,xmm6); // e4 f4 e5 f5 e6 f6 e7 f7
	xmm5 = _mm_unpackhi_epi16(xmm5,xmm4); // g4 h4 g5 h5 g6 h6 g7 h7

	xmm6 = xmm0; // e0 f0 e1 f1 e2 f2 e3 f3
	xmm4 = xmm7; // e4 f4 e5 f5 e6 f6 e7 f7

	xmm6 = _mm_unpacklo_epi32(xmm6,xmm1); // e0 f0 g0 h0 e1 f1 g1 h1
	xmm4 = _mm_unpacklo_epi32(xmm4,xmm5); // e4 f4 g4 h4 e5 f5 g5 h5
	xmm0 = _mm_unpackhi_epi32(xmm0,xmm1); // e2 f2 g2 h2 e3 f3 g3 h3
	xmm7 = _mm_unpackhi_epi32(xmm7,xmm5); // e6 f6 g6 h6 e7 f7 g7 h7
	// 130
	// xmm0 = efgh-2/3, xmm2 = MM2, xmm3 = MM3, xmm4 = efgh-4/5, xmm6 = efgh-0/1, xmm7 = efgh-6/7, src[0] = MM0, src[8] = MM1
	_mm_store_si128((__m128i*)&src[16], xmm4); // e4 f4 g4 h4 e5 f5 g5 h5
	_mm_store_si128((__m128i*)&src[24], xmm7); // e6 f6 g6 h6 e7 f7 g7 h7

	xmm1 = _mm_load_si128((__m128i*)&src[0 ]); // a 0 1 2 3 4 5 6 7
	xmm5 = _mm_load_si128((__m128i*)&src[8 ]); // b 0 1 2 3 4 5 6 7
	xmm4 = xmm1; // a 0 1 2 3 4 5 6 7
	xmm7 = xmm2; // c 0 1 2 3 4 5 6 7

	xmm1 = _mm_unpacklo_epi16(xmm1,xmm5); // a0 b0 a1 b1 a2 b2 a3 b3
	xmm2 = _mm_unpacklo_epi16(xmm2,xmm3); // c0 d0 c1 d1 c2 d2 c3 d3
	xmm4 = _mm_unpackhi_epi16(xmm4,xmm5); // a4 b4 a5 b5 a6 b6 a7 b7
	xmm7 = _mm_unpackhi_epi16(xmm7,xmm3); // c4 d4 c5 d5 c6 d6 c7 d7

	xmm5 = xmm1; // a0 b0 a1 b1 a2 b2 a3 b3
	xmm3 = xmm4; // a4 b4 a5 b5 a6 b6 a7 b7

	xmm1 = _mm_unpacklo_epi32(xmm1,xmm2); // a0 b0 c0 d0 a1 b1 c1 d1
	xmm4 = _mm_unpacklo_epi32(xmm4,xmm7); // a4 b4 c4 d4 a5 b5 c5 d5
	xmm5 = _mm_unpackhi_epi32(xmm5,xmm2); // a2 b2 c2 d2 a3 b3 c3 d3
	xmm3 = _mm_unpackhi_epi32(xmm3,xmm7); // a6 b6 c6 d6 a7 b7 c7 d7
	// 146
	// xmm0 = efgh-2/3, xmm1 = abcd-0/1, xmm3 = abcd-6/7, xmm4 = abcd-4/5, xmm5 = abcd-2/3, xmm6 = efgh-0/1, src[16] = efgh-4/5, src[24] = efgh-6/7
	xmm2 = xmm1; // a0 b0 c0 d0 a1 b1 c1 d1
	xmm7 = xmm5; // a2 b2 c2 d2 a3 b3 c3 d3

	xmm1 = _mm_unpacklo_epi64(xmm1,xmm6); // a0 b0 c0 d0 e0 f0 g0 h0
	xmm5 = _mm_unpacklo_epi64(xmm5,xmm0); // a2 b2 c2 d2 e2 f2 g2 h2
	xmm2 = _mm_unpackhi_epi64(xmm2,xmm6); // a1 b1 c1 d1 e1 f1 g1 h1
	xmm7 = _mm_unpackhi_epi64(xmm7,xmm0); // a3 b3 c3 d3 e3 f3 g3 h3

	_mm_store_si128((__m128i*)&src[32], xmm2); // a1 b1 c1 d1 e1 f1 g1 h1
	_mm_store_si128((__m128i*)&src[40], xmm7); // a3 b3 c3 d3 e3 f3 g3 h3

	xmm6 = _mm_load_si128((__m128i*)&src[16]); // e4 f4 g4 h4 e5 f5 g5 h5
	xmm0 = _mm_load_si128((__m128i*)&src[24]); // e6 f6 g6 h6 e7 f7 g7 h7
	xmm2 = xmm4; // a4 b4 c4 d4 a5 b5 c5 d5
	xmm7 = xmm3; // a6 b6 c6 d6 a7 b7 c7 d7

	xmm4 = _mm_unpacklo_epi64(xmm4,xmm6); // a4 b4 c4 d4 e4 f4 g4 h4
	xmm3 = _mm_unpacklo_epi64(xmm3,xmm0); // a6 b6 c6 d6 e6 f6 g6 h6
	xmm2 = _mm_unpackhi_epi64(xmm2,xmm6); // a5 b5 c5 d5 e5 f5 g5 h5
	xmm7 = _mm_unpackhi_epi64(xmm7,xmm0); // a7 b7 c7 d7 e7 f7 g7 h7
	// 162
	// xmm1 = mm[0], xmm2 = mm[5], xmm3 = mm[6], xmm4 = mm[4], xmm5 = mm[2], xmm7 = mm[7], src[32] = mm[1], src[40] = mm[3]
	// ********************* //
	xmm6 = xmm1; //mm[0]
	xmm0 = xmm5; //mm[2]

	xmm1 = _mm_adds_epi16(xmm1, xmm4); // mm[0] + mm[4] = a[0]
	xmm6 = _mm_subs_epi16(xmm6, xmm4); // mm[0] - mm[4] = a[4]
	xmm5 = _mm_srai_epi16(xmm5, 1); // mm[2]>>1
	xmm5 = _mm_subs_epi16(xmm5, xmm3); // (mm[2]>>1) - mm[6] = a[2]
	xmm3 = _mm_srai_epi16(xmm3, 1); // mm[6]>>1
	xmm0 = _mm_adds_epi16(xmm0, xmm3); // mm[2] + (mm[6]>>1) = a[6]
	// 170
	// xmm0 = a[6], xmm1 = a[0], xmm2 = mm[5], xmm5 = a[2], xmm6 = a[4], xmm7 = mm[7], src[32] = mm[1], src[40] = mm[3]
	xmm4 = xmm1; // a[0]
	xmm3 = xmm6; // a[4]

	xmm4 = _mm_adds_epi16(xmm4, xmm0); // a[0] + a[6] = b[0]
	xmm3 = _mm_adds_epi16(xmm3, xmm5); // a[4] + a[2] = b[2]
	xmm1 = _mm_subs_epi16(xmm1, xmm0); // a[0] - a[6] = b[6]
	xmm6 = _mm_subs_epi16(xmm6, xmm5); // a[4] - a[2] = b[4]

	_mm_store_si128((__m128i*)&src[0 ], xmm4); // b[0]
	_mm_store_si128((__m128i*)&src[8 ], xmm3); // b[2]
	_mm_store_si128((__m128i*)&src[16], xmm6); // b[4]
	_mm_store_si128((__m128i*)&src[24], xmm1); // b[6]

	xmm0 = _mm_load_si128((__m128i*)&src[32]); // mm[1]
	xmm5 = _mm_load_si128((__m128i*)&src[40]); // mm[3]
	// 182
	// xmm0 = mm[1], xmm2 = mm[5], xmm5 = mm[3], xmm7 = mm[7], src[0] = b[0], src[8] = b[2], src[16] = b[4], src[24] = b[6]
	xmm1 = xmm0; // a1
	xmm3 = xmm5; // a3
	xmm4 = xmm2; // a5
	xmm6 = xmm7; // a7

	xmm1 = _mm_srai_epi16(xmm1, 1); // a1>>1
	xmm3 = _mm_srai_epi16(xmm3, 1); // a3>>1
	xmm4 = _mm_srai_epi16(xmm4, 1); // a5>>1
	xmm6 = _mm_srai_epi16(xmm6, 1); // a7>>1

	xmm1 = _mm_adds_epi16(xmm1, xmm0); // a1 + (a1>>1)
	xmm3 = _mm_adds_epi16(xmm3, xmm5); // a3 + (a3>>1)
	xmm4 = _mm_adds_epi16(xmm4, xmm2); // a5 + (a5>>1)
	xmm6 = _mm_adds_epi16(xmm6, xmm7); // a7 + (a7>>1)

	xmm1 = _mm_adds_epi16(xmm1, xmm2); //  a5 + a1 + (a1>>1)
	xmm3 = _mm_subs_epi16(xmm3, xmm7); // -a7 + a3 + (a3>>1)
	xmm4 = _mm_subs_epi16(xmm4, xmm0); // -a1 + a5 + (a5>>1)
	xmm6 = _mm_adds_epi16(xmm6, xmm5); //  a3 + a7 + (a7>>1)

	xmm5 = _mm_adds_epi16(xmm5, xmm1); //  a3 + a5 + a1 + (a1>>1) = a[7]
	xmm0 = _mm_subs_epi16(xmm0, xmm3); //  a1 + a7 - a3 - (a3>>1) = a[3]
	xmm7 = _mm_adds_epi16(xmm7, xmm4); //  a7 - a1 + a5 + (a5>>1) = a[5]
	xmm2 = _mm_subs_epi16(xmm2, xmm6); //  a5 - a3 - a7 - (a7>>1) = a[1]
	// 202
	// xmm0 = a[3], xmm2 = a[1], xmm5 = a[7], xmm7 = a[5], src[0] = b[0], src[8] = b[2], src[16] = b[4], src[24] = b[6]
	xmm1 = xmm5; // a[7]
	xmm3 = xmm7; // a[5]

	xmm1 = _mm_srai_epi16(xmm1, 2); // a[7]>>2
	xmm3 = _mm_srai_epi16(xmm3, 2); // a[5]>>2
	xmm1 = _mm_adds_epi16(xmm1, xmm2); //  a[1] + (a[7]>>2) = b[1]
	xmm3 = _mm_adds_epi16(xmm3, xmm0); //  a[3] + (a[5]>>2) = b[3]

	xmm2 = _mm_srai_epi16(xmm2, 2); // a[1]>>2
	xmm0 = _mm_srai_epi16(xmm0, 2); // a[3]>>2
	xmm5 = _mm_subs_epi16(xmm5, xmm2); //  a[7] - (a[1]>>2) = b[7]
	xmm0 = _mm_subs_epi16(xmm0, xmm7); //  (a[3]>>2) - a[5] = b[5]
	// 212
	// xmm0 = b[5], xmm1 = b[1], xmm3 = b[3], xmm5 = b[7], src[0] = b[0], src[8] = b[2], src[16] = b[4], src[24] = b[6]
	xmm2 = _mm_load_si128((__m128i*)&src[0 ]); // b[0]
	xmm4 = _mm_load_si128((__m128i*)&src[8 ]); // b[2]
	xmm6 = xmm2; // b[0]
	xmm7 = xmm4; // b[2]

	xmm2 = _mm_adds_epi16(xmm2, xmm5); //  b[0] + b[7]
	xmm6 = _mm_subs_epi16(xmm6, xmm5); //  b[0] - b[7]
	xmm4 = _mm_adds_epi16(xmm4, xmm0); //  b[2] + b[5]
	xmm7 = _mm_subs_epi16(xmm7, xmm0); //  b[2] - b[5]
	// 220
	// xmm1 = b[1], xmm2 = ROW[0], xmm3 = b[3], xmm4 = ROW[1], xmm6 = ROW[7], xmm7 = ROW[6], src[16] = b[4], src[24] = b[6]
	xmm5 = _mm_load_si128((__m128i*)&const_32); // all 32s
	xmm0 = _mm_loadl_epi64 ((__m128i *) (pred+0  )); // pred[0]
	xmm2 = _mm_adds_epi16(xmm2, xmm5); //  b[0] + b[7] + 32
	xmm2 = _mm_srai_epi16(xmm2, 6); // (b[0] + b[7] + 32)>>6
	xmm6 = _mm_adds_epi16(xmm6, xmm5); //  b[0] - b[7] + 32
	xmm6 = _mm_srai_epi16(xmm6, 6); // (b[0] - b[7] + 32)>>6
	xmm4 = _mm_adds_epi16(xmm4, xmm5); //  b[2] + b[5] + 32
	xmm4 = _mm_srai_epi16(xmm4, 6); // (b[2] + b[5] + 32)>>6
	xmm7 = _mm_adds_epi16(xmm7, xmm5); //  b[2] - b[5] + 32
	xmm7 = _mm_srai_epi16(xmm7, 6); // (b[2] - b[5] + 32)>>6
	// 230
	// xmm0 = pred[0], xmm1 = b[1], xmm2 = ROW[0], xmm3 = b[3], xmm4 = ROW[1], xmm5 = "32", xmm6 = ROW[7], xmm7 = ROW[6],
	// src[16] = b[4], src[24] = b[6]
	xmm5 = _mm_setzero_si128(); // all 0s
	xmm0 = _mm_unpacklo_epi8(xmm0,xmm5);
	xmm2 = _mm_adds_epi16(xmm2, xmm0); //  (b[0] + b[7] + 32)>>6 + pred[0]
	xmm2 = _mm_packus_epi16(xmm2,xmm2);
	_mm_storel_epi64((__m128i*)&dest[0*stride], xmm2);

	xmm0 = _mm_loadl_epi64 ((__m128i *) (pred+16 )); // pred[1]
	xmm0 = _mm_unpacklo_epi8(xmm0,xmm5);
	xmm4 = _mm_adds_epi16(xmm4, xmm0); //  (b[2] + b[5] + 32)>>6 + pred[1]
	xmm4 = _mm_packus_epi16(xmm4,xmm4);
	_mm_storel_epi64((__m128i*)&dest[1*stride], xmm4);
	// 240
	// xmm1 = b[1], xmm3 = b[3], xmm5 = 0, xmm6 = ROW[7], xmm7 = ROW[6], src[16] = b[4], src[24] = b[6]
	xmm2 = _mm_load_si128((__m128i*)&src[16]); // b[4]
	xmm4 = xmm2; // b[4]
	xmm2 = _mm_adds_epi16(xmm2, xmm3); //  b[4] + b[3]
	xmm4 = _mm_subs_epi16(xmm4, xmm3); //  b[4] - b[3]
	xmm0 = _mm_load_si128((__m128i*)&const_32); // all 32s
	xmm2 = _mm_adds_epi16(xmm2, xmm0); //  b[4] + b[3] + 32
	xmm2 = _mm_srai_epi16(xmm2, 6); // (b[4] + b[3] + 32)>>6
	xmm4 = _mm_adds_epi16(xmm4, xmm0); //  b[4] - b[3] + 32
	xmm4 = _mm_srai_epi16(xmm4, 6); // (b[4] - b[3] + 32)>>6
	xmm3 = _mm_loadl_epi64 ((__m128i *) (pred+32  )); // pred[2]
	xmm3 = _mm_unpacklo_epi8(xmm3,xmm5);
	xmm2 = _mm_adds_epi16(xmm2, xmm3); //  (b[4] + b[3] + 32)>>6 + pred[2]
	xmm2 = _mm_packus_epi16(xmm2,xmm2);
	_mm_storel_epi64((__m128i*)&dest[2*stride], xmm2);
	// 254
	// xmm0 = "32", xmm1 = b[1], xmm4 = ROW[5], xmm5 = 0, xmm6 = ROW[7], xmm7 = ROW[6], src[24] = b[6]
	xmm3 = _mm_load_si128((__m128i*)&src[24]); // b[6]
	xmm2 = xmm3; // b[6]
	xmm2 = _mm_adds_epi16(xmm2, xmm1); //  b[6] + b[1]
	xmm3 = _mm_subs_epi16(xmm3, xmm1); //  b[6] - b[1]
	xmm2 = _mm_adds_epi16(xmm2, xmm0); //  b[6] + b[1] + 32
	xmm2 = _mm_srai_epi16(xmm2, 6); // (b[6] + b[1] + 32)>>6
	xmm3 = _mm_adds_epi16(xmm3, xmm0); //  b[6] - b[1] + 32
	xmm3 = _mm_srai_epi16(xmm3, 6); // (b[6] - b[1] + 32)>>6
	xmm1 = _mm_loadl_epi64 ((__m128i *) (pred+48  )); // pred[3]
	xmm1 = _mm_unpacklo_epi8(xmm1,xmm5);
	xmm2 = _mm_adds_epi16(xmm2, xmm1); //  (b[6] + b[1] + 32)>>6 + pred[3]
	xmm2 = _mm_packus_epi16(xmm2,xmm2);
	_mm_storel_epi64((__m128i*)&dest[3*stride], xmm2);
	// 267
	// xmm0 = "32", xmm3 = ROW[4], xmm4 = ROW[5], xmm5 = 0, xmm6 = ROW[7], xmm7 = ROW[6], src[24] = b[6]
	xmm1 = _mm_loadl_epi64 ((__m128i *) (pred+64  )); // pred[4]
	xmm1 = _mm_unpacklo_epi8(xmm1,xmm5);
	xmm3 = _mm_adds_epi16(xmm3, xmm1); //  (b[6] - b[1] + 32)>>6 + pred[4]
	xmm3 = _mm_packus_epi16(xmm3,xmm3);
	_mm_storel_epi64((__m128i*)&dest[4*stride], xmm3);

	xmm1 = _mm_loadl_epi64 ((__m128i *) (pred+80  )); // pred[5]
	xmm1 = _mm_unpacklo_epi8(xmm1,xmm5);
	xmm4 = _mm_adds_epi16(xmm4, xmm1); //  (b[4] - b[3] + 32)>>6 + pred[5]
	xmm4 = _mm_packus_epi16(xmm4,xmm4);
	_mm_storel_epi64((__m128i*)&dest[5*stride], xmm4);

	xmm1 = _mm_loadl_epi64 ((__m128i *) (pred+96  )); // pred[6]
	xmm1 = _mm_unpacklo_epi8(xmm1,xmm5);
	xmm7 = _mm_adds_epi16(xmm7, xmm1); //  (b[2] - b[5] + 32)>>6 + pred[6]
	xmm7 = _mm_packus_epi16(xmm7,xmm7);
	_mm_storel_epi64((__m128i*)&dest[6*stride], xmm7);

	xmm1 = _mm_loadl_epi64 ((__m128i *) (pred+112 )); // pred[7]
	xmm1 = _mm_unpacklo_epi8(xmm1,xmm5);
	xmm6 = _mm_adds_epi16(xmm6, xmm1); //  (b[0] - b[7] + 32)>>6 + pred[7]
	xmm6 = _mm_packus_epi16(xmm6,xmm6);
	_mm_storel_epi64((__m128i*)&dest[7*stride], xmm6);
	// 287
	_mm_store_si128((__m128i*)&src[0 ], xmm5); //0
	_mm_store_si128((__m128i*)&src[8 ], xmm5); //0
	_mm_store_si128((__m128i*)&src[16], xmm5); //0
	_mm_store_si128((__m128i*)&src[24], xmm5); //0
	_mm_store_si128((__m128i*)&src[32], xmm5); //0
	_mm_store_si128((__m128i*)&src[40], xmm5); //0
	_mm_store_si128((__m128i*)&src[48], xmm5); //0
	_mm_store_si128((__m128i*)&src[56], xmm5); //0
	// 295 - 44 if _PRE_TRANSPOSE_
}
#elif 0
// New code, SSE2 inline assembly
void inverse_transform8x8_sse2 PARGS4(byte *dest, byte *pred, short *src, int stride)
{
	__m128i xmm0,xmm1,xmm2,xmm3,xmm4,xmm5,xmm6,xmm7;
	static unsigned short __declspec(align(16)) const_32[8] = { 32, 32, 32, 32, 32, 32, 32, 32 };
	static unsigned char *ptr_32 = (unsigned char *) &const_32[0];

	// Transpose 8x8 matrix
	//start	
	//__asm int 3;
	__asm
	{
		mov        eax,       src;       // eax = src
		movdqa     xmm0,      [eax+0  ]; //a 0 1 2 3 4 5 6 7
		movdqa     xmm1,      [eax+16 ]; //b 0 1 2 3 4 5 6 7
		movdqa     xmm2,      [eax+32 ]; //c 0 1 2 3 4 5 6 7
		movdqa     xmm3,      [eax+48 ]; //d 0 1 2 3 4 5 6 7

		movdqa     xmm6,      xmm0;      //a 0 1 2 3 4 5 6 7
		movdqa     xmm7,      xmm2;      //c 0 1 2 3 4 5 6 7

		punpcklwd  xmm0,      xmm1;      // a0 b0 a1 b1 a2 b2 a3 b3
		punpcklwd  xmm2,      xmm3;      // c0 d0 c1 d1 c2 d2 c3 d3
		punpckhwd  xmm6,      xmm1;      // a4 b4 a5 b5 a6 b6 a7 b7
		punpckhwd  xmm7,      xmm3;      // c4 d4 c5 d5 c6 d6 c7 d7

		movdqa     xmm1,      xmm0;      // a0 b0 a1 b1 a2 b2 a3 b3
		movdqa     xmm3,      xmm6;      // a4 b4 a5 b5 a6 b6 a7 b7

		punpckldq  xmm0,      xmm2;      // a0 b0 c0 d0 a1 b1 c1 d1
		punpckldq  xmm6,      xmm7;      // a4 b4 c4 d4 a5 b5 c5 d5
		punpckhdq  xmm1,      xmm2;      // a2 b2 c2 d2 a3 b3 c3 d3
		punpckhdq  xmm3,      xmm7;      // a6 b6 c6 d6 a7 b7 c7 d7
		// 16
		// xmm0 = abcd-0/1, xmm1 = abcd-2/3, xmm6 = abcd-4/5, xmm3 = abcd-6/7
		movdqa     [eax+0  ], xmm6;      // a4 b4 c4 d4 a5 b5 c5 d5
		movdqa     [eax+16 ], xmm3;      // a6 b6 c6 d6 a7 b7 c7 d7

		movdqa     xmm4,      [eax+64 ]; //e 0 1 2 3 4 5 6 7
		movdqa     xmm5,      [eax+80 ]; //f 0 1 2 3 4 5 6 7
		movdqa     xmm6,      [eax+96 ]; //g 0 1 2 3 4 5 6 7
		movdqa     xmm7,      [eax+112]; //h 0 1 2 3 4 5 6 7

		movdqa     xmm2,      xmm4;      //e 0 1 2 3 4 5 6 7
		movdqa     xmm3,      xmm6;      //g 0 1 2 3 4 5 6 7

		punpcklwd  xmm2,      xmm5;      // e0 f0 e1 f1 e2 f2 e3 f3
		punpcklwd  xmm3,      xmm7;      // g0 h0 g1 h1 g2 h2 g3 h3
		punpckhwd  xmm4,      xmm5;      // e4 f4 e5 f5 e6 f6 e7 f7
		punpckhwd  xmm6,      xmm7;      // g4 h4 g5 h5 g6 h6 g7 h7

		movdqa     xmm5,      xmm2;      // e0 f0 e1 f1 e2 f2 e3 f3
		movdqa     xmm7,      xmm4;      // e4 f4 e5 f5 e6 f6 e7 f7

		punpckldq  xmm5,      xmm3;      // e0 f0 g0 h0 e1 f1 g1 h1
		punpckldq  xmm7,      xmm6;      // e4 f4 g4 h4 e5 f5 g5 h5
		punpckhdq  xmm2,      xmm3;      // e2 f2 g2 h2 e3 f3 g3 h3
		punpckhdq  xmm4,      xmm6;      // e6 f6 g6 h6 e7 f7 g7 h7

		movdqa     xmm3,      xmm0;      // a0 b0 c0 d0 a1 b1 c1 d1
		movdqa     xmm6,      xmm1;      // a2 b2 c2 d2 a3 b3 c3 d3
		// 36
		// xmm0= abcd-0/1, xmm1= abcd-2/3, xmm2= efgh-2/3, xmm3= abcd-0/1, xmm4= efgh-6/7, xmm5= efgh-0/1, xmm6= abcd-2/3, xmm7= efgh-4/5
		punpcklqdq xmm3,      xmm5;      // a0 b0 c0 d0 e0 f0 g0 h0
		punpcklqdq xmm6,      xmm2;      // a2 b2 c2 d2 e2 f2 g2 h2
		punpckhqdq xmm0,      xmm5;      // a1 b1 c1 d1 e1 f1 g1 h1
		punpckhqdq xmm1,      xmm2;      // a3 b3 c3 d3 e3 f3 g3 h3

		movdqa     [eax+32 ], xmm0;      // a1 b1 c1 d1 e1 f1 g1 h1
		movdqa     [eax+48 ], xmm1;      // a3 b3 c3 d3 e3 f3 g3 h3

		movdqa     xmm5,      [eax+0  ]; // a4 b4 c4 d4 a5 b5 c5 d5
		movdqa     xmm2,      [eax+16 ]; // a6 b6 c6 d6 a7 b7 c7 d7

		movdqa     xmm0,      xmm5;      // a4 b4 c4 d4 a5 b5 c5 d5
		movdqa     xmm1,      xmm2;      // a6 b6 c6 d6 a7 b7 c7 d7

		punpcklqdq xmm0,      xmm7;      // a4 b4 c4 d4 e4 f4 g4 h4
		punpcklqdq xmm1,      xmm4;      // a6 b6 c6 d6 e6 f6 g6 h6
		punpckhqdq xmm5,      xmm7;      // a5 b5 c5 d5 e5 f5 g5 h5
		punpckhqdq xmm2,      xmm4;      // a7 b7 c7 d7 e7 f7 g7 h7
		// 50
		// xmm0= a4, xmm1= a6, xmm2= a7, xmm3= a0, xmm5= a5, xmm6= a2, src[16] = a1, src[24] = a3
		movdqa     xmm4,      xmm3;      // a0
		movdqa     xmm7,      xmm6;      // a2

		paddsw     xmm3,      xmm0;      // a0+a4 = a[0]
		psubsw     xmm4,      xmm0;      // a0-a4 = a[4]
		psraw      xmm7,      1;         // a2>>1
		psubsw     xmm7,      xmm1;      // (a2>>1) - a6 = a[2]
		psraw      xmm1,      1;         // a6>>1
		paddsw     xmm6,      xmm1;      // a2 + (a6>>1) = a[6]

		movdqa     xmm0,      xmm3;      // a[0]
		movdqa     xmm1,      xmm4;      // a[4]

		paddsw     xmm0,      xmm6;      // a[0] + a[6] = b[0]
		paddsw     xmm1,      xmm7;      // a[4] + a[2] = b[2]
		psubsw     xmm3,      xmm6;      // a[0] - a[6] = b[6]
		psubsw     xmm4,      xmm7;      // a[4] - a[2] = b[4]
		// 64
		// xmm0= b[0], xmm1= b[2], xmm2= a7, xmm3= b[6], xmm4 = b[4], xmm5= a5, src[16] = a1, src[24] = a3
		movdqa     [eax+64 ], xmm0;      // b[0]
		movdqa     [eax+80 ], xmm1;      // b[2]
		movdqa     [eax+96 ], xmm4;      // b[4]
		movdqa     [eax+112], xmm3;      // b[6]

		movdqa     xmm6,      [eax+32 ]; // a1
		movdqa     xmm7,      [eax+48 ]; // a3

		movdqa     xmm0,      xmm6;      // a1
		movdqa     xmm1,      xmm7;      // a3
		movdqa     xmm3,      xmm5;      // a5
		movdqa     xmm4,      xmm2;      // a7

		psraw      xmm0,      1;         // a1>>1
		psraw      xmm1,      1;         // a3>>1
		psraw      xmm3,      1;         // a5>>1
		psraw      xmm4,      1;         // a7>>1

		paddsw     xmm0,      xmm6;      // a1 + (a1>>1)
		paddsw     xmm1,      xmm7;      // a3 + (a3>>1)
		paddsw     xmm3,      xmm5;      // a5 + (a5>>1)
		paddsw     xmm4,      xmm2;      // a7 + (a7>>1)

		paddsw     xmm0,      xmm5;      //  a5 + a1 + (a1>>1)
		psubsw     xmm1,      xmm2;      // -a7 + a3 + (a3>>1)
		psubsw     xmm3,      xmm6;      // -a1 + a5 + (a5>>1)
		paddsw     xmm4,      xmm7;      //  a3 + a7 + (a7>>1)

		paddsw     xmm7,      xmm0;      //  a3 + a5 + a1 + (a1>>1) = a[7]
		psubsw     xmm6,      xmm1;      //  a1 + a7 - a3 - (a3>>1) = a[3]
		paddsw     xmm2,      xmm3;      //  a7 - a1 + a5 + (a5>>1) = a[5]
		psubsw     xmm5,      xmm4;      //  a5 - a3 - a7 - (a7>>1) = a[1]
		// 90
		// xmm2= a[5], xmm5= a[1], xmm6 = a[3], xmm7= a[7], src[32] = b[0], src[40] = b[2], src[48] = b[4], src[56] = b[6]
		movdqa     xmm0,      xmm7;      // a[7]
		movdqa     xmm1,      xmm2;      // a[5]

		psraw      xmm0,      2;         // a[7]>>2
		psraw      xmm1,      2;         // a[5]>>2
		paddsw     xmm0,      xmm5;      // a[1] + (a[7]>>2) = b[1]
		paddsw     xmm1,      xmm6;      // a[3] + (a[5]>>2) = b[3]

		psraw      xmm6,      2;         // a[3]>>2
		psraw      xmm5,      2;         // a[1]>>2
		psubsw     xmm6,      xmm2;      // (a[3]>>2) - a[5] = b[5]
		psubsw     xmm7,      xmm5;      // a[7] - (a[1]>>2) = b[7]
		// 100
		// xmm0 = b[1], xmm1 = b[3], xmm6 = b[5], xmm7= b[7], src[32] = b[0], src[40] = b[2], src[48] = b[4], src[56] = b[6]
		movdqa     xmm2,      [eax+64 ]; // b[0]
		movdqa     xmm3,      [eax+80 ]; // b[2]
		movdqa     xmm4,      xmm2;      // b[0]
		movdqa     xmm5,      xmm3;      // b[2]

		paddsw     xmm2,      xmm7;      // b[0] + b[7]
		psubsw     xmm4,      xmm7;      // b[0] - b[7]
		paddsw     xmm3,      xmm6;      // b[2] + b[5]
		psubsw     xmm5,      xmm6;      // b[2] - b[5]

		movdqa     [eax+0  ], xmm2;      // MM0
		movdqa     [eax+16 ], xmm3;      // MM1

		movdqa     xmm6,      [eax+96 ]; // b[4]
		movdqa     xmm7,      [eax+112]; // b[6]
		movdqa     xmm2,      xmm6;      // b[4]
		movdqa     xmm3,      xmm7;      // b[6]

		paddsw     xmm2,      xmm1;      // b[4] + b[3]
		psubsw     xmm6,      xmm1;      // b[4] - b[3]
		paddsw     xmm3,      xmm0;      // b[6] + b[1]
		psubsw     xmm7,      xmm0;      // b[6] - b[1]
		// 118
		// xmm2 = MM2, xmm3 = MM3, xmm4 = MM7, xmm5 = MM6, xmm6 = MM5, xmm7 = MM4, src[0] = MM0, src[8] = MM1
		movdqa     xmm0,      xmm7;      //e 0 1 2 3 4 5 6 7
		movdqa     xmm1,      xmm5;      //g 0 1 2 3 4 5 6 7

		punpcklwd  xmm0,      xmm6;      // e0 f0 e1 f1 e2 f2 e3 f3
		punpcklwd  xmm1,      xmm4;      // g0 h0 g1 h1 g2 h2 g3 h3
		punpckhwd  xmm7,      xmm6;      // e4 f4 e5 f5 e6 f6 e7 f7
		punpckhwd  xmm5,      xmm4;      // g4 h4 g5 h5 g6 h6 g7 h7

		movdqa     xmm6,      xmm0;      // e0 f0 e1 f1 e2 f2 e3 f3
		movdqa     xmm4,      xmm7;      // e4 f4 e5 f5 e6 f6 e7 f7

		punpckldq  xmm6,      xmm1;      // e0 f0 g0 h0 e1 f1 g1 h1
		punpckldq  xmm4,      xmm5;      // e4 f4 g4 h4 e5 f5 g5 h5
		punpckhdq  xmm0,      xmm1;      // e2 f2 g2 h2 e3 f3 g3 h3
		punpckhdq  xmm7,      xmm5;      // e6 f6 g6 h6 e7 f7 g7 h7
		// 130
		// xmm0 = efgh-2/3, xmm2 = MM2, xmm3 = MM3, xmm4 = efgh-4/5, xmm6 = efgh-0/1, xmm7 = efgh-6/7, src[0] = MM0, src[8] = MM1
		movdqa     [eax+32 ], xmm4;      // e4 f4 g4 h4 e5 f5 g5 h5
		movdqa     [eax+48 ], xmm7;      // e6 f6 g6 h6 e7 f7 g7 h7

		movdqa     xmm1,      [eax+0  ]; // a 0 1 2 3 4 5 6 7
		movdqa     xmm5,      [eax+16 ]; // b 0 1 2 3 4 5 6 7
		movdqa     xmm4,      xmm1;      // a 0 1 2 3 4 5 6 7
		movdqa     xmm7,      xmm2;      // c 0 1 2 3 4 5 6 7

		punpcklwd  xmm1,      xmm5;      // a0 b0 a1 b1 a2 b2 a3 b3
		punpcklwd  xmm2,      xmm3;      // c0 d0 c1 d1 c2 d2 c3 d3
		punpckhwd  xmm4,      xmm5;      // a4 b4 a5 b5 a6 b6 a7 b7
		punpckhwd  xmm7,      xmm3;      // c4 d4 c5 d5 c6 d6 c7 d7

		movdqa     xmm5,      xmm1;      // a0 b0 a1 b1 a2 b2 a3 b3
		movdqa     xmm3,      xmm4;      // a4 b4 a5 b5 a6 b6 a7 b7

		punpckldq  xmm1,      xmm2;      // a0 b0 c0 d0 a1 b1 c1 d1
		punpckldq  xmm4,      xmm7;      // a4 b4 c4 d4 a5 b5 c5 d5
		punpckhdq  xmm5,      xmm2;      // a2 b2 c2 d2 a3 b3 c3 d3
		punpckhdq  xmm3,      xmm7;      // a6 b6 c6 d6 a7 b7 c7 d7
		// 146
		// xmm0 = efgh-2/3, xmm1 = abcd-0/1, xmm3 = abcd-6/7, xmm4 = abcd-4/5, xmm5 = abcd-2/3, xmm6 = efgh-0/1, src[16] = efgh-4/5, src[24] = efgh-6/7
		movdqa     xmm2,      xmm1;      // a0 b0 c0 d0 a1 b1 c1 d1
		movdqa     xmm7,      xmm5;      // a2 b2 c2 d2 a3 b3 c3 d3

		punpcklqdq xmm1,      xmm6;      // a0 b0 c0 d0 e0 f0 g0 h0
		punpcklqdq xmm5,      xmm0;      // a2 b2 c2 d2 e2 f2 g2 h2
		punpckhqdq xmm2,      xmm6;      // a1 b1 c1 d1 e1 f1 g1 h1
		punpckhqdq xmm7,      xmm0;      // a3 b3 c3 d3 e3 f3 g3 h3

		movdqa     [eax+64 ], xmm2;      // a1 b1 c1 d1 e1 f1 g1 h1
		movdqa     [eax+80 ], xmm7;      // a3 b3 c3 d3 e3 f3 g3 h3

		movdqa     xmm6,      [eax+32 ]; // e4 f4 g4 h4 e5 f5 g5 h5
		movdqa     xmm0,      [eax+48 ]; // e6 f6 g6 h6 e7 f7 g7 h7
		movdqa     xmm2,      xmm4;      // a4 b4 c4 d4 a5 b5 c5 d5
		movdqa     xmm7,      xmm3;      // a6 b6 c6 d6 a7 b7 c7 d7

		punpcklqdq xmm4,      xmm6;      // a4 b4 c4 d4 e4 f4 g4 h4
		punpcklqdq xmm3,      xmm0;      // a6 b6 c6 d6 e6 f6 g6 h6
		punpckhqdq xmm2,      xmm6;      // a5 b5 c5 d5 e5 f5 g5 h5
		punpckhqdq xmm7,      xmm0;      // a7 b7 c7 d7 e7 f7 g7 h7
		// 162
		// xmm1 = mm[0], xmm2 = mm[5], xmm3 = mm[6], xmm4 = mm[4], xmm5 = mm[2], xmm7 = mm[7], src[32] = mm[1], src[40] = mm[3]
		// ********************* //
		movdqa     xmm6,      xmm1;      //mm[0]
		movdqa     xmm0,      xmm5;      //mm[2]

		paddsw     xmm1,      xmm4;      // mm[0] + mm[4] = a[0]
		psubsw     xmm6,      xmm4;      // mm[0] - mm[4] = a[4]
		psraw      xmm5,      1;         // mm[2]>>1
		psubsw     xmm5,      xmm3;      // (mm[2]>>1) - mm[6] = a[2]
		psraw      xmm3,      1;         // mm[6]>>1
		paddsw     xmm0,      xmm3;      // mm[2] + (mm[6]>>1) = a[6]
		// 170
		// xmm0 = a[6], xmm1 = a[0], xmm2 = mm[5], xmm5 = a[2], xmm6 = a[4], xmm7 = mm[7], src[32] = mm[1], src[40] = mm[3]
		movdqa     xmm4,      xmm1;      // a[0]
		movdqa     xmm3,      xmm6;      // a[4]

		paddsw     xmm4,      xmm0;      // a[0] + a[6] = b[0]
		paddsw     xmm3,      xmm5;      // a[4] + a[2] = b[2]
		psubsw     xmm1,      xmm0;      // a[0] - a[6] = b[6]
		psubsw     xmm6,      xmm5;      // a[4] - a[2] = b[4]

		movdqa     [eax+0  ], xmm4;      // b[0]
		movdqa     [eax+16 ], xmm3;      // b[2]
		movdqa     [eax+32 ], xmm6;      // b[4]
		movdqa     [eax+48 ], xmm1;      // b[6]

		movdqa     xmm0,      [eax+64 ]; // mm[1]
		movdqa     xmm5,      [eax+80 ]; // mm[3]
		// 182
		// xmm0 = mm[1], xmm2 = mm[5], xmm5 = mm[3], xmm7 = mm[7], src[0] = b[0], src[8] = b[2], src[16] = b[4], src[24] = b[6]
		movdqa     xmm1,      xmm0;      // a1
		movdqa     xmm3,      xmm5;      // a3
		movdqa     xmm4,      xmm2;      // a5
		movdqa     xmm6,      xmm7;      // a7

		psraw      xmm1,      1;         // a1>>1
		psraw      xmm3,      1;         // a3>>1
		psraw      xmm4,      1;         // a5>>1
		psraw      xmm6,      1;         // a7>>1

		paddsw     xmm1,      xmm0;      // a1 + (a1>>1)
		paddsw     xmm3,      xmm5;      // a3 + (a3>>1)
		paddsw     xmm4,      xmm2;      // a5 + (a5>>1)
		paddsw     xmm6,      xmm7;      // a7 + (a7>>1)

		paddsw     xmm1,      xmm2;      //  a5 + a1 + (a1>>1)
		psubsw     xmm3,      xmm7;      // -a7 + a3 + (a3>>1)
		psubsw     xmm4,      xmm0;      // -a1 + a5 + (a5>>1)
		paddsw     xmm6,      xmm5;      //  a3 + a7 + (a7>>1)

		paddsw     xmm5,      xmm1;      //  a3 + a5 + a1 + (a1>>1) = a[7]
		psubsw     xmm0,      xmm3;      //  a1 + a7 - a3 - (a3>>1) = a[3]
		paddsw     xmm7,      xmm4;      //  a7 - a1 + a5 + (a5>>1) = a[5]
		psubsw     xmm2,      xmm6;      //  a5 - a3 - a7 - (a7>>1) = a[1]
		// 202
		// xmm0 = a[3], xmm2 = a[1], xmm5 = a[7], xmm7 = a[5], src[0] = b[0], src[8] = b[2], src[16] = b[4], src[24] = b[6]
		movdqa     xmm1,      xmm5;      // a[7]
		movdqa     xmm3,      xmm7;      // a[5]

		psraw      xmm1,      2;         // a[7]>>2
		psraw      xmm3,      2;         // a[5]>>2
		paddsw     xmm1,      xmm2;      // a[1] + (a[7]>>2) = b[1]
		paddsw     xmm3,      xmm0;      // a[3] + (a[5]>>2) = b[3]

		psraw      xmm2,      2;         // a[1]>>2
		psraw      xmm0,      2;         // a[3]>>2
		psubsw     xmm5,      xmm2;      // a[7] - (a[1]>>2) = b[7]
		psubsw     xmm0,      xmm7;      // (a[3]>>2) - a[5] = b[5]
		// 212
		// xmm0 = b[5], xmm1 = b[1], xmm3 = b[3], xmm5 = b[7], src[0] = b[0], src[8] = b[2], src[16] = b[4], src[24] = b[6]
		movdqa     xmm2,      [eax+0  ]; // b[0]
		movdqa     xmm4,      [eax+16 ]; // b[2]
		movdqa     xmm6,      xmm2;      // b[0]
		movdqa     xmm7,      xmm4;      // b[2]

		paddsw     xmm2,      xmm5;      //  b[0] + b[7]
		psubsw     xmm6,      xmm5;      //  b[0] - b[7]
		paddsw     xmm4,      xmm0;      //  b[2] + b[5]
		psubsw     xmm7,      xmm0;      //  b[2] - b[5]
		// 220
		// xmm1 = b[1], xmm2 = ROW[0], xmm3 = b[3], xmm4 = ROW[1], xmm6 = ROW[7], xmm7 = ROW[6], src[16] = b[4], src[24] = b[6]
		//mov        ebx,       ptr_32;    // ebx = &const_32[0]
		mov        ecx,       pred;      // ecx = pred
		mov        edx,       dest;      // edx = dest
		//movdqa     xmm5,      [ebx];     // all 32s
		movdqa     xmm5,      const_32[0];// all 32s
		movq       xmm0,      [ecx];     // pred[0]
		paddsw     xmm2,      xmm5;      //  b[0] + b[7] + 32
		psraw      xmm2,      6;         // (b[0] + b[7] + 32)>>6
		paddsw     xmm6,      xmm5;      //  b[0] - b[7] + 32
		psraw      xmm6,      6;         // (b[0] - b[7] + 32)>>6
		paddsw     xmm4,      xmm5;      //  b[2] + b[5] + 32
		psraw      xmm4,      6;         // (b[2] + b[5] + 32)>>6
		paddsw     xmm7,      xmm5;      //  b[2] - b[5] + 32
		psraw      xmm7,      6;         // (b[2] - b[5] + 32)>>6
		// 230
		// xmm0 = pred[0], xmm1 = b[1], xmm2 = ROW[0], xmm3 = b[3], xmm4 = ROW[1], xmm5 = "32", xmm6 = ROW[7], xmm7 = ROW[6],
		// src[16] = b[4], src[24] = b[6]
		pxor       xmm5,      xmm5;      // all 0s
		punpcklbw  xmm0,      xmm5;
		paddsw     xmm2,      xmm0;      // (b[0] + b[7] + 32)>>6 + pred[0]
		packuswb   xmm2,      xmm2;
		movq       [edx],     xmm2;

		mov        esi,       stride;
		lea        edi,       [esi+2*esi];// 3*stride
		movq       xmm0,      [ecx+16 ]; // pred[1]
		punpcklbw  xmm0,      xmm5;
		paddsw     xmm4,      xmm0;      // (b[2] + b[5] + 32)>>6 + pred[1]
		packuswb   xmm4,      xmm4;
		movq       [edx+esi], xmm4;
		// 240
		// xmm1 = b[1], xmm3 = b[3], xmm5 = 0, xmm6 = ROW[7], xmm7 = ROW[6], src[16] = b[4], src[24] = b[6]
		movdqa     xmm2,      [eax+32 ]; // b[4]
		movdqa     xmm4,      xmm2;      // b[4]
		paddsw     xmm2,      xmm3;      // b[4] + b[3]
		psubsw     xmm4,      xmm3;      // b[4] - b[3]
		//movdqa     xmm0,      [ebx];     // all 32s
		movdqa     xmm0,      const_32[0];// all 32s
		paddsw     xmm2,      xmm0;      // b[4] + b[3] + 32
		psraw      xmm2,      6;         // (b[4] + b[3] + 32)>>6
		paddsw     xmm4,      xmm0;      // b[4] - b[3] + 32
		psraw      xmm4,      6;         // (b[4] - b[3] + 32)>>6
		movq       xmm3,      [ecx+32 ]; // pred[2]
		punpcklbw  xmm3,      xmm5;
		paddsw     xmm2,      xmm3;      // (b[4] + b[3] + 32)>>6 + pred[2]
		packuswb   xmm2,      xmm2;
		movq       [edx+2*esi], xmm2;
		// 254
		// xmm0 = "32", xmm1 = b[1], xmm4 = ROW[5], xmm5 = 0, xmm6 = ROW[7], xmm7 = ROW[6], src[24] = b[6]
		movdqa     xmm3,      [eax+48 ]; // b[6]
		movdqa     xmm2,      xmm3;      // b[6]
		paddsw     xmm2,      xmm1;      // b[6] + b[1]
		psubsw     xmm3,      xmm1;      // b[6] - b[1]
		paddsw     xmm2,      xmm0;      // b[6] + b[1] + 32
		psraw      xmm2,      6;         // (b[6] + b[1] + 32)>>6
		paddsw     xmm3,      xmm0;      // b[6] - b[1] + 32
		psraw      xmm3,      6;         // (b[6] - b[1] + 32)>>6
		movq       xmm1,      [ecx+48 ]; // pred[3]
		punpcklbw  xmm1,      xmm5;
		paddsw     xmm2,      xmm1;      // (b[6] + b[1] + 32)>>6 + pred[3]
		packuswb   xmm2,      xmm2;
		movq       [edx+edi], xmm2;
		// 267
		// xmm0 = "32", xmm3 = ROW[4], xmm4 = ROW[5], xmm5 = 0, xmm6 = ROW[7], xmm7 = ROW[6], src[24] = b[6]
		lea        edx,       [edx+4*esi];// edx = &dest[4*stride]
		movq       xmm1,      [ecx+64 ]; // pred[4]
		punpcklbw  xmm1,      xmm5;
		paddsw     xmm3,      xmm1;      // (b[6] - b[1] + 32)>>6 + pred[4]
		packuswb   xmm3,      xmm3;
		movq       [edx],     xmm3;

		movq       xmm1,      [ecx+80 ]; // pred[5]
		punpcklbw  xmm1,      xmm5;
		paddsw     xmm4,      xmm1;      // (b[4] - b[3] + 32)>>6 + pred[5]
		packuswb   xmm4,      xmm4;
		movq       [edx+esi], xmm4;

		movq       xmm1,      [ecx+96 ]; // pred[6]
		punpcklbw  xmm1,      xmm5;
		paddsw     xmm7,      xmm1;      // (b[2] - b[5] + 32)>>6 + pred[6]
		packuswb   xmm7,      xmm7;
		movq       [edx+2*esi], xmm7;

		movq       xmm1,      [ecx+112]; // pred[7]
		punpcklbw  xmm1,      xmm5;
		paddsw     xmm6,      xmm1;      // (b[0] - b[7] + 32)>>6 + pred[7]
		packuswb   xmm6,      xmm6;
		movq       [edx+edi], xmm6;
		// 287
		movdqa     [eax+0  ], xmm5;      //0
		movdqa     [eax+16 ], xmm5;      //0
		movdqa     [eax+32 ], xmm5;      //0
		movdqa     [eax+48 ], xmm5;      //0
		movdqa     [eax+64 ], xmm5;      //0
		movdqa     [eax+80 ], xmm5;      //0
		movdqa     [eax+96 ], xmm5;      //0
		movdqa     [eax+112], xmm5;      //0
	}
	// 295 - 302 with register overhead
}
#elif !defined(USE_FOR_WINDVD) || defined(LINUX_H264_MT)
void inverse_transform8x8_sse2 PARGS4(byte *dest, byte *pred, short *src, int stride)
{
	__m128i xmm0,xmm1,xmm2,xmm3,xmm4,xmm5,xmm6,xmm7;
	__m128i xmm8,xmm9,xmm10,xmm11,xmm12,xmm13;
	static unsigned short __declspec(align(16)) const_32[8] = { 32, 32, 32, 32, 32, 32, 32, 32 };

#ifdef _PRE_TRANSPOSE_
	xmm3 = _mm_load_si128((__m128i*)&src[0 ]); // a0 b0 c0 d0 e0 f0 g0 h0
	xmm6 = _mm_load_si128((__m128i*)&src[16]); // a2 b2 c2 d2 e2 f2 g2 h2
	xmm0 = _mm_load_si128((__m128i*)&src[32]); // a4 b4 c4 d4 e4 f4 g4 h4
	xmm5 = _mm_load_si128((__m128i*)&src[40]); // a5 b5 c5 d5 e5 f5 g5 h5
	xmm1 = _mm_load_si128((__m128i*)&src[48]); // a6 b6 c6 d6 e6 f6 g6 h6
	xmm2 = _mm_load_si128((__m128i*)&src[56]); // a7 b7 c7 d7 e7 f7 g7 h7
#else
	// Transpose 8x8 matrix
	//start	
	//__asm int 3;
	xmm0 = _mm_load_si128((__m128i*)&src[0 ]); //a 0 1 2 3 4 5 6 7
	xmm1 = _mm_load_si128((__m128i*)&src[8 ]); //b 0 1 2 3 4 5 6 7
	xmm2 = _mm_load_si128((__m128i*)&src[16]); //c 0 1 2 3 4 5 6 7
	xmm3 = _mm_load_si128((__m128i*)&src[24]); //d 0 1 2 3 4 5 6 7

	xmm6 = xmm0; //a 0 1 2 3 4 5 6 7
	xmm7 = xmm2; //c 0 1 2 3 4 5 6 7

	xmm0 = _mm_unpacklo_epi16(xmm0,xmm1); // a0 b0 a1 b1 a2 b2 a3 b3
	xmm2 = _mm_unpacklo_epi16(xmm2,xmm3); // c0 d0 c1 d1 c2 d2 c3 d3
	xmm6 = _mm_unpackhi_epi16(xmm6,xmm1); // a4 b4 a5 b5 a6 b6 a7 b7
	xmm7 = _mm_unpackhi_epi16(xmm7,xmm3); // c4 d4 c5 d5 c6 d6 c7 d7

	xmm1 = xmm0; // a0 b0 a1 b1 a2 b2 a3 b3
	xmm3 = xmm6; // a4 b4 a5 b5 a6 b6 a7 b7

	xmm0 = _mm_unpacklo_epi32(xmm0,xmm2); // a0 b0 c0 d0 a1 b1 c1 d1
	xmm6 = _mm_unpacklo_epi32(xmm6,xmm7); // a4 b4 c4 d4 a5 b5 c5 d5
	xmm1 = _mm_unpackhi_epi32(xmm1,xmm2); // a2 b2 c2 d2 a3 b3 c3 d3
	xmm3 = _mm_unpackhi_epi32(xmm3,xmm7); // a6 b6 c6 d6 a7 b7 c7 d7
	// 16
	// xmm0 = abcd-0/1, xmm1 = abcd-2/3, xmm3 = abcd-6/7, xmm6 = abcd-4/5
	_mm_store_si128((__m128i*)&src[0 ], xmm6); // a4 b4 c4 d4 a5 b5 c5 d5
	_mm_store_si128((__m128i*)&src[16], xmm3); // a6 b6 c6 d6 a7 b7 c7 d7

	xmm4 = _mm_load_si128((__m128i*)&src[32]); //e 0 1 2 3 4 5 6 7
	xmm5 = _mm_load_si128((__m128i*)&src[40]); //f 0 1 2 3 4 5 6 7
	xmm6 = _mm_load_si128((__m128i*)&src[48]); //g 0 1 2 3 4 5 6 7
	xmm7 = _mm_load_si128((__m128i*)&src[56]); //h 0 1 2 3 4 5 6 7

	xmm2 = xmm4; //e 0 1 2 3 4 5 6 7
	xmm3 = xmm6; //g 0 1 2 3 4 5 6 7

	xmm2 = _mm_unpacklo_epi16(xmm2,xmm5); // e0 f0 e1 f1 e2 f2 e3 f3
	xmm3 = _mm_unpacklo_epi16(xmm3,xmm7); // g0 h0 g1 h1 g2 h2 g3 h3
	xmm4 = _mm_unpackhi_epi16(xmm4,xmm5); // e4 f4 e5 f5 e6 f6 e7 f7
	xmm6 = _mm_unpackhi_epi16(xmm6,xmm7); // g4 h4 g5 h5 g6 h6 g7 h7

	xmm5 = xmm2; // e0 f0 e1 f1 e2 f2 e3 f3
	xmm7 = xmm4; // e4 f4 e5 f5 e6 f6 e7 f7

	xmm5 = _mm_unpacklo_epi32(xmm5,xmm3); // e0 f0 g0 h0 e1 f1 g1 h1
	xmm7 = _mm_unpacklo_epi32(xmm7,xmm6); // e4 f4 g4 h4 e5 f5 g5 h5
	xmm2 = _mm_unpackhi_epi32(xmm2,xmm3); // e2 f2 g2 h2 e3 f3 g3 h3
	xmm4 = _mm_unpackhi_epi32(xmm4,xmm6); // e6 f6 g6 h6 e7 f7 g7 h7

	xmm3 = xmm0; // a0 b0 c0 d0 a1 b1 c1 d1
	xmm6 = xmm1; // a2 b2 c2 d2 a3 b3 c3 d3
	// 36
	// xmm0= abcd-0/1, xmm1= abcd-2/3, xmm2= efgh-2/3, xmm3= abcd-0/1, xmm4= efgh-6/7, xmm5= efgh-0/1, xmm6= abcd-2/3, xmm7= efgh-4/5
	// src[0]= abcd-4/5, src[8]= abcd-6/7
	xmm3 = _mm_unpacklo_epi64(xmm3,xmm5); // a0 b0 c0 d0 e0 f0 g0 h0
	xmm6 = _mm_unpacklo_epi64(xmm6,xmm2); // a2 b2 c2 d2 e2 f2 g2 h2
	xmm0 = _mm_unpackhi_epi64(xmm0,xmm5); // a1 b1 c1 d1 e1 f1 g1 h1
	xmm1 = _mm_unpackhi_epi64(xmm1,xmm2); // a3 b3 c3 d3 e3 f3 g3 h3

	_mm_store_si128((__m128i*)&src[8 ], xmm0); // a1 b1 c1 d1 e1 f1 g1 h1
	_mm_store_si128((__m128i*)&src[24], xmm1); // a3 b3 c3 d3 e3 f3 g3 h3

	xmm5 = _mm_load_si128((__m128i*)&src[0 ]); // a4 b4 c4 d4 a5 b5 c5 d5
	xmm2 = _mm_load_si128((__m128i*)&src[16]); // a6 b6 c6 d6 a7 b7 c7 d7

	xmm0 = xmm5; // a4 b4 c4 d4 a5 b5 c5 d5
	xmm1 = xmm2; // a6 b6 c6 d6 a7 b7 c7 d7

	xmm0 = _mm_unpacklo_epi64(xmm0,xmm7); // a4 b4 c4 d4 e4 f4 g4 h4
	xmm1 = _mm_unpacklo_epi64(xmm1,xmm4); // a6 b6 c6 d6 e6 f6 g6 h6
	xmm5 = _mm_unpackhi_epi64(xmm5,xmm7); // a5 b5 c5 d5 e5 f5 g5 h5
	xmm2 = _mm_unpackhi_epi64(xmm2,xmm4); // a7 b7 c7 d7 e7 f7 g7 h7
	// 50
#endif
	// xmm0= a4, xmm1= a6, xmm2= a7, xmm3= a0, xmm5= a5, xmm6= a2, src[16] = a1, src[24] = a3
	xmm4 = xmm3; // a0
	xmm7 = xmm6; // a2

	xmm3 = _mm_adds_epi16(xmm3, xmm0); // a0+a4 = a[0]
	xmm4 = _mm_subs_epi16(xmm4, xmm0); // a0-a4 = a[4]
	xmm7 = _mm_srai_epi16(xmm7, 1); // a2>>1
	xmm7 = _mm_subs_epi16(xmm7, xmm1); // (a2>>1) - a6 = a[2]
	xmm1 = _mm_srai_epi16(xmm1, 1); // a6>>1
	xmm6 = _mm_adds_epi16(xmm6, xmm1); // a2 + (a6>>1) = a[6]

	xmm0 = xmm3; // a[0]
	xmm1 = xmm4; // a[4]
	xmm3 = _mm_subs_epi16(xmm3, xmm6); // a[0] - a[6] = b[6]
	xmm4 = _mm_subs_epi16(xmm4, xmm7); // a[4] - a[2] = b[4]
	xmm0 = _mm_adds_epi16(xmm0, xmm6); // a[0] + a[6] = b[0]
	xmm1 = _mm_adds_epi16(xmm1, xmm7); // a[4] + a[2] = b[2]

	// 64
	// xmm0= b[0], xmm1= b[2], xmm2= a7, xmm3= b[6], xmm4 = b[4], xmm5= a5, src[16] = a1, src[24] = a3
	xmm8 = xmm0;//_mm_store_si128((__m128i*)&src[32], xmm0); // b[0]
	xmm9 = xmm1;//_mm_store_si128((__m128i*)&src[40], xmm1); // b[2]
	xmm10 = xmm4;//_mm_store_si128((__m128i*)&src[48], xmm4); // b[4]
	xmm11 = xmm3;//_mm_store_si128((__m128i*)&src[56], xmm3); // b[6]

	xmm6 = _mm_load_si128((__m128i*)&src[8 ]); // a1
	xmm7 = _mm_load_si128((__m128i*)&src[24]); // a3

	xmm0 = xmm6; // a1
	xmm1 = xmm7; // a3
	xmm3 = xmm5; // a5
	xmm4 = xmm2; // a7

	xmm0 = _mm_srai_epi16(xmm0, 1); // a1>>1
	xmm1 = _mm_srai_epi16(xmm1, 1); // a3>>1
	xmm3 = _mm_srai_epi16(xmm3, 1); // a5>>1
	xmm4 = _mm_srai_epi16(xmm4, 1); // a7>>1

	xmm0 = _mm_adds_epi16(xmm0, xmm6); // a1 + (a1>>1)
	xmm1 = _mm_adds_epi16(xmm1, xmm7); // a3 + (a3>>1)
	xmm3 = _mm_adds_epi16(xmm3, xmm5); // a5 + (a5>>1)
	xmm4 = _mm_adds_epi16(xmm4, xmm2); // a7 + (a7>>1)

	xmm0 = _mm_adds_epi16(xmm0, xmm5); //  a5 + a1 + (a1>>1)
	xmm1 = _mm_subs_epi16(xmm1, xmm2); // -a7 + a3 + (a3>>1)
	xmm3 = _mm_subs_epi16(xmm3, xmm6); // -a1 + a5 + (a5>>1)
	xmm4 = _mm_adds_epi16(xmm4, xmm7); //  a3 + a7 + (a7>>1)

	xmm7 = _mm_adds_epi16(xmm7, xmm0); //  a3 + a5 + a1 + (a1>>1) = a[7]
	xmm6 = _mm_subs_epi16(xmm6, xmm1); //  a1 + a7 - a3 - (a3>>1) = a[3]
	xmm2 = _mm_adds_epi16(xmm2, xmm3); //  a7 - a1 + a5 + (a5>>1) = a[5]
	xmm5 = _mm_subs_epi16(xmm5, xmm4); //  a5 - a3 - a7 - (a7>>1) = a[1]
	// 90
	// xmm2= a[5], xmm5= a[1], xmm6 = a[3], xmm7= a[7], src[32] = b[0], src[40] = b[2], src[48] = b[4], src[56] = b[6]
	xmm0 = xmm7; // a[7]
	xmm1 = xmm2; // a[5]

	xmm0 = _mm_srai_epi16(xmm0, 2); // a[7]>>2
	xmm1 = _mm_srai_epi16(xmm1, 2); // a[5]>>2
	xmm0 = _mm_adds_epi16(xmm0, xmm5); //  a[1] + (a[7]>>2) = b[1]
	xmm1 = _mm_adds_epi16(xmm1, xmm6); //  a[3] + (a[5]>>2) = b[3]

	xmm6 = _mm_srai_epi16(xmm6, 2); // a[3]>>2
	xmm5 = _mm_srai_epi16(xmm5, 2); // a[1]>>2
	xmm6 = _mm_subs_epi16(xmm6, xmm2); //  (a[3]>>2) - a[5] = b[5]
	xmm7 = _mm_subs_epi16(xmm7, xmm5); //  a[7] - (a[1]>>2) = b[7]
	// 100
	// xmm0 = b[1], xmm1 = b[3], xmm6 = b[5], xmm7= b[7], src[32] = b[0], src[40] = b[2], src[48] = b[4], src[56] = b[6]
	xmm2 = xmm8;//xmm2 = _mm_load_si128((__m128i*)&src[32]); // b[0]
	xmm3 = xmm9;//xmm3 = _mm_load_si128((__m128i*)&src[40]); // b[2]
	xmm4 = xmm2; // b[0]
	xmm5 = xmm3; // b[2]

	xmm2 = _mm_adds_epi16(xmm2, xmm7); //  b[0] + b[7]
	xmm4 = _mm_subs_epi16(xmm4, xmm7); //  b[0] - b[7]
	xmm3 = _mm_adds_epi16(xmm3, xmm6); //  b[2] + b[5]
	xmm5 = _mm_subs_epi16(xmm5, xmm6); //  b[2] - b[5]

	xmm8 = xmm2;//_mm_store_si128((__m128i*)&src[0 ], xmm2); // MM0
	xmm9 = xmm3;//_mm_store_si128((__m128i*)&src[8 ], xmm3); // MM1

	xmm6 = xmm10;//xmm6 = _mm_load_si128((__m128i*)&src[48]); // b[4]
	xmm7 = xmm11;//xmm7 = _mm_load_si128((__m128i*)&src[56]); // b[6]
	xmm2 = xmm6; // b[4]
	xmm3 = xmm7; // b[6]

	xmm2 = _mm_adds_epi16(xmm2, xmm1); //  b[4] + b[3]
	xmm6 = _mm_subs_epi16(xmm6, xmm1); //  b[4] - b[3]
	xmm3 = _mm_adds_epi16(xmm3, xmm0); //  b[6] + b[1]
	xmm7 = _mm_subs_epi16(xmm7, xmm0); //  b[6] - b[1]
	// 118
	// xmm2 = MM2, xmm3 = MM3, xmm4 = MM7, xmm5 = MM6, xmm6 = MM5, xmm7 = MM4, src[0] = MM0, src[8] = MM1
	xmm0 = xmm7; //e 0 1 2 3 4 5 6 7
	xmm1 = xmm5; //g 0 1 2 3 4 5 6 7

	xmm0 = _mm_unpacklo_epi16(xmm0,xmm6); // e0 f0 e1 f1 e2 f2 e3 f3
	xmm1 = _mm_unpacklo_epi16(xmm1,xmm4); // g0 h0 g1 h1 g2 h2 g3 h3
	xmm7 = _mm_unpackhi_epi16(xmm7,xmm6); // e4 f4 e5 f5 e6 f6 e7 f7
	xmm5 = _mm_unpackhi_epi16(xmm5,xmm4); // g4 h4 g5 h5 g6 h6 g7 h7

	xmm6 = xmm0; // e0 f0 e1 f1 e2 f2 e3 f3
	xmm4 = xmm7; // e4 f4 e5 f5 e6 f6 e7 f7

	xmm6 = _mm_unpacklo_epi32(xmm6,xmm1); // e0 f0 g0 h0 e1 f1 g1 h1
	xmm4 = _mm_unpacklo_epi32(xmm4,xmm5); // e4 f4 g4 h4 e5 f5 g5 h5
	xmm0 = _mm_unpackhi_epi32(xmm0,xmm1); // e2 f2 g2 h2 e3 f3 g3 h3
	xmm7 = _mm_unpackhi_epi32(xmm7,xmm5); // e6 f6 g6 h6 e7 f7 g7 h7
	// 130
	// xmm0 = efgh-2/3, xmm2 = MM2, xmm3 = MM3, xmm4 = efgh-4/5, xmm6 = efgh-0/1, xmm7 = efgh-6/7, src[0] = MM0, src[8] = MM1
	xmm10 = xmm4;//_mm_store_si128((__m128i*)&src[16], xmm4); // e4 f4 g4 h4 e5 f5 g5 h5
	xmm11 = xmm7;//_mm_store_si128((__m128i*)&src[24], xmm7); // e6 f6 g6 h6 e7 f7 g7 h7

	xmm1 = xmm8;//xmm1 = _mm_load_si128((__m128i*)&src[0 ]); // a 0 1 2 3 4 5 6 7
	xmm5 = xmm9;//xmm5 = _mm_load_si128((__m128i*)&src[8 ]); // b 0 1 2 3 4 5 6 7
	xmm4 = xmm1; // a 0 1 2 3 4 5 6 7
	xmm7 = xmm2; // c 0 1 2 3 4 5 6 7

	xmm1 = _mm_unpacklo_epi16(xmm1,xmm5); // a0 b0 a1 b1 a2 b2 a3 b3
	xmm2 = _mm_unpacklo_epi16(xmm2,xmm3); // c0 d0 c1 d1 c2 d2 c3 d3
	xmm4 = _mm_unpackhi_epi16(xmm4,xmm5); // a4 b4 a5 b5 a6 b6 a7 b7
	xmm7 = _mm_unpackhi_epi16(xmm7,xmm3); // c4 d4 c5 d5 c6 d6 c7 d7

	xmm5 = xmm1; // a0 b0 a1 b1 a2 b2 a3 b3
	xmm3 = xmm4; // a4 b4 a5 b5 a6 b6 a7 b7

	xmm1 = _mm_unpacklo_epi32(xmm1,xmm2); // a0 b0 c0 d0 a1 b1 c1 d1
	xmm4 = _mm_unpacklo_epi32(xmm4,xmm7); // a4 b4 c4 d4 a5 b5 c5 d5
	xmm5 = _mm_unpackhi_epi32(xmm5,xmm2); // a2 b2 c2 d2 a3 b3 c3 d3
	xmm3 = _mm_unpackhi_epi32(xmm3,xmm7); // a6 b6 c6 d6 a7 b7 c7 d7
	// 146
	// xmm0 = efgh-2/3, xmm1 = abcd-0/1, xmm3 = abcd-6/7, xmm4 = abcd-4/5, xmm5 = abcd-2/3, xmm6 = efgh-0/1, src[16] = efgh-4/5, src[24] = efgh-6/7
	xmm2 = xmm1; // a0 b0 c0 d0 a1 b1 c1 d1
	xmm7 = xmm5; // a2 b2 c2 d2 a3 b3 c3 d3

	xmm1 = _mm_unpacklo_epi64(xmm1,xmm6); // a0 b0 c0 d0 e0 f0 g0 h0
	xmm5 = _mm_unpacklo_epi64(xmm5,xmm0); // a2 b2 c2 d2 e2 f2 g2 h2
	xmm2 = _mm_unpackhi_epi64(xmm2,xmm6); // a1 b1 c1 d1 e1 f1 g1 h1
	xmm7 = _mm_unpackhi_epi64(xmm7,xmm0); // a3 b3 c3 d3 e3 f3 g3 h3

	xmm8 = xmm2;//_mm_store_si128((__m128i*)&src[32], xmm2); // a1 b1 c1 d1 e1 f1 g1 h1
	xmm9 = xmm7;//_mm_store_si128((__m128i*)&src[40], xmm7); // a3 b3 c3 d3 e3 f3 g3 h3

	xmm6 = xmm10;//xmm6 = _mm_load_si128((__m128i*)&src[16]); // e4 f4 g4 h4 e5 f5 g5 h5
	xmm0 = xmm11;//xmm0 = _mm_load_si128((__m128i*)&src[24]); // e6 f6 g6 h6 e7 f7 g7 h7
	xmm2 = xmm4; // a4 b4 c4 d4 a5 b5 c5 d5
	xmm7 = xmm3; // a6 b6 c6 d6 a7 b7 c7 d7

	xmm4 = _mm_unpacklo_epi64(xmm4,xmm6); // a4 b4 c4 d4 e4 f4 g4 h4
	xmm3 = _mm_unpacklo_epi64(xmm3,xmm0); // a6 b6 c6 d6 e6 f6 g6 h6
	xmm2 = _mm_unpackhi_epi64(xmm2,xmm6); // a5 b5 c5 d5 e5 f5 g5 h5
	xmm7 = _mm_unpackhi_epi64(xmm7,xmm0); // a7 b7 c7 d7 e7 f7 g7 h7
	// 162
	// xmm1 = mm[0], xmm2 = mm[5], xmm3 = mm[6], xmm4 = mm[4], xmm5 = mm[2], xmm7 = mm[7], src[32] = mm[1], src[40] = mm[3]
	// ********************* //
	xmm6 = xmm1; //mm[0]
	xmm0 = xmm5; //mm[2]

	xmm1 = _mm_adds_epi16(xmm1, xmm4); // mm[0] + mm[4] = a[0]
	xmm6 = _mm_subs_epi16(xmm6, xmm4); // mm[0] - mm[4] = a[4]
	xmm5 = _mm_srai_epi16(xmm5, 1); // mm[2]>>1
	xmm5 = _mm_subs_epi16(xmm5, xmm3); // (mm[2]>>1) - mm[6] = a[2]
	xmm3 = _mm_srai_epi16(xmm3, 1); // mm[6]>>1
	xmm0 = _mm_adds_epi16(xmm0, xmm3); // mm[2] + (mm[6]>>1) = a[6]
	// 170
	// xmm0 = a[6], xmm1 = a[0], xmm2 = mm[5], xmm5 = a[2], xmm6 = a[4], xmm7 = mm[7], src[32] = mm[1], src[40] = mm[3]
	xmm4 = xmm1; // a[0]
	xmm3 = xmm6; // a[4]

	xmm4 = _mm_adds_epi16(xmm4, xmm0); // a[0] + a[6] = b[0]
	xmm3 = _mm_adds_epi16(xmm3, xmm5); // a[4] + a[2] = b[2]
	xmm1 = _mm_subs_epi16(xmm1, xmm0); // a[0] - a[6] = b[6]
	xmm6 = _mm_subs_epi16(xmm6, xmm5); // a[4] - a[2] = b[4]

	xmm10 = xmm4;//_mm_store_si128((__m128i*)&src[0 ], xmm4); // b[0]
	xmm11 = xmm3;//_mm_store_si128((__m128i*)&src[8 ], xmm3); // b[2]
	xmm12 = xmm6;//_mm_store_si128((__m128i*)&src[16], xmm6); // b[4]
	xmm13 = xmm1;//_mm_store_si128((__m128i*)&src[24], xmm1); // b[6]

	xmm0 = xmm8;//xmm0 = _mm_load_si128((__m128i*)&src[32]); // mm[1]
	xmm5 = xmm9;//xmm5 = _mm_load_si128((__m128i*)&src[40]); // mm[3]
	// 182
	// xmm0 = mm[1], xmm2 = mm[5], xmm5 = mm[3], xmm7 = mm[7], src[0] = b[0], src[8] = b[2], src[16] = b[4], src[24] = b[6]
	xmm1 = xmm0; // a1
	xmm3 = xmm5; // a3
	xmm4 = xmm2; // a5
	xmm6 = xmm7; // a7

	xmm1 = _mm_srai_epi16(xmm1, 1); // a1>>1
	xmm3 = _mm_srai_epi16(xmm3, 1); // a3>>1
	xmm4 = _mm_srai_epi16(xmm4, 1); // a5>>1
	xmm6 = _mm_srai_epi16(xmm6, 1); // a7>>1

	xmm1 = _mm_adds_epi16(xmm1, xmm0); // a1 + (a1>>1)
	xmm3 = _mm_adds_epi16(xmm3, xmm5); // a3 + (a3>>1)
	xmm4 = _mm_adds_epi16(xmm4, xmm2); // a5 + (a5>>1)
	xmm6 = _mm_adds_epi16(xmm6, xmm7); // a7 + (a7>>1)

	xmm1 = _mm_adds_epi16(xmm1, xmm2); //  a5 + a1 + (a1>>1)
	xmm3 = _mm_subs_epi16(xmm3, xmm7); // -a7 + a3 + (a3>>1)
	xmm4 = _mm_subs_epi16(xmm4, xmm0); // -a1 + a5 + (a5>>1)
	xmm6 = _mm_adds_epi16(xmm6, xmm5); //  a3 + a7 + (a7>>1)

	xmm5 = _mm_adds_epi16(xmm5, xmm1); //  a3 + a5 + a1 + (a1>>1) = a[7]
	xmm0 = _mm_subs_epi16(xmm0, xmm3); //  a1 + a7 - a3 - (a3>>1) = a[3]
	xmm7 = _mm_adds_epi16(xmm7, xmm4); //  a7 - a1 + a5 + (a5>>1) = a[5]
	xmm2 = _mm_subs_epi16(xmm2, xmm6); //  a5 - a3 - a7 - (a7>>1) = a[1]
	// 202
	// xmm0 = a[3], xmm2 = a[1], xmm5 = a[7], xmm7 = a[5], src[0] = b[0], src[8] = b[2], src[16] = b[4], src[24] = b[6]
	xmm1 = xmm5; // a[7]
	xmm3 = xmm7; // a[5]

	xmm1 = _mm_srai_epi16(xmm1, 2); // a[7]>>2
	xmm3 = _mm_srai_epi16(xmm3, 2); // a[5]>>2
	xmm1 = _mm_adds_epi16(xmm1, xmm2); //  a[1] + (a[7]>>2) = b[1]
	xmm3 = _mm_adds_epi16(xmm3, xmm0); //  a[3] + (a[5]>>2) = b[3]

	xmm2 = _mm_srai_epi16(xmm2, 2); // a[1]>>2
	xmm0 = _mm_srai_epi16(xmm0, 2); // a[3]>>2
	xmm5 = _mm_subs_epi16(xmm5, xmm2); //  a[7] - (a[1]>>2) = b[7]
	xmm0 = _mm_subs_epi16(xmm0, xmm7); //  (a[3]>>2) - a[5] = b[5]
	// 212
	// xmm0 = b[5], xmm1 = b[1], xmm3 = b[3], xmm5 = b[7], src[0] = b[0], src[8] = b[2], src[16] = b[4], src[24] = b[6]
	xmm2 = xmm10;//xmm2 = _mm_load_si128((__m128i*)&src[0 ]); // b[0]
	xmm4 = xmm11;//xmm4 = _mm_load_si128((__m128i*)&src[8 ]); // b[2]
	xmm6 = xmm2; // b[0]
	xmm7 = xmm4; // b[2]

	xmm2 = _mm_adds_epi16(xmm2, xmm5); //  b[0] + b[7]
	xmm6 = _mm_subs_epi16(xmm6, xmm5); //  b[0] - b[7]
	xmm4 = _mm_adds_epi16(xmm4, xmm0); //  b[2] + b[5]
	xmm7 = _mm_subs_epi16(xmm7, xmm0); //  b[2] - b[5]
	// 220
	// xmm1 = b[1], xmm2 = ROW[0], xmm3 = b[3], xmm4 = ROW[1], xmm6 = ROW[7], xmm7 = ROW[6], src[16] = b[4], src[24] = b[6]
	xmm5 = _mm_load_si128((__m128i*)&const_32); // all 32s
	xmm0 = _mm_loadl_epi64 ((__m128i *) (pred  )); // pred[0]
	xmm2 = _mm_adds_epi16(xmm2, xmm5); //  b[0] + b[7] + 32
	xmm2 = _mm_srai_epi16(xmm2, 6); // (b[0] + b[7] + 32)>>6
	xmm6 = _mm_adds_epi16(xmm6, xmm5); //  b[0] - b[7] + 32
	xmm6 = _mm_srai_epi16(xmm6, 6); // (b[0] - b[7] + 32)>>6
	xmm4 = _mm_adds_epi16(xmm4, xmm5); //  b[2] + b[5] + 32
	xmm4 = _mm_srai_epi16(xmm4, 6); // (b[2] + b[5] + 32)>>6
	xmm7 = _mm_adds_epi16(xmm7, xmm5); //  b[2] - b[5] + 32
	xmm7 = _mm_srai_epi16(xmm7, 6); // (b[2] - b[5] + 32)>>6
	// 230
	// xmm0 = pred[0], xmm1 = b[1], xmm2 = ROW[0], xmm3 = b[3], xmm4 = ROW[1], xmm5 = "32", xmm6 = ROW[7], xmm7 = ROW[6],
	// src[16] = b[4], src[24] = b[6]
	xmm5 = _mm_setzero_si128(); // all 0s
	xmm0 = _mm_unpacklo_epi8(xmm0,xmm5);
	xmm2 = _mm_adds_epi16(xmm2, xmm0); //  (b[0] + b[7] + 32)>>6 + pred[0]
	xmm2 = _mm_packus_epi16(xmm2,xmm2);
	_mm_storel_epi64((__m128i*)&dest[0], xmm2);
	xmm0 = _mm_loadl_epi64 ((__m128i *) (pred+16 )); // pred[1]
	xmm0 = _mm_unpacklo_epi8(xmm0,xmm5);
	xmm4 = _mm_adds_epi16(xmm4, xmm0); //  (b[2] + b[5] + 32)>>6 + pred[1]
	xmm4 = _mm_packus_epi16(xmm4,xmm5);
	_mm_storel_epi64((__m128i*)&dest[1*stride], xmm4);
	// 240
	// xmm1 = b[1], xmm3 = b[3], xmm5 = 0, xmm6 = ROW[7], xmm7 = ROW[6], src[16] = b[4], src[24] = b[6]
	xmm2 = xmm12;//xmm2 = _mm_load_si128((__m128i*)&src[16]); // b[4]
	xmm4 = xmm2; // b[4]
	xmm2 = _mm_adds_epi16(xmm2, xmm3); //  b[4] + b[3]
	xmm4 = _mm_subs_epi16(xmm4, xmm3); //  b[4] - b[3]
	xmm0 = _mm_load_si128((__m128i*)&const_32); // all 32s
	xmm2 = _mm_adds_epi16(xmm2, xmm0); //  b[4] + b[3] + 32
	xmm2 = _mm_srai_epi16(xmm2, 6); // (b[4] + b[3] + 32)>>6
	xmm4 = _mm_adds_epi16(xmm4, xmm0); //  b[4] - b[3] + 32
	xmm4 = _mm_srai_epi16(xmm4, 6); // (b[4] - b[3] + 32)>>6
	xmm3 = _mm_loadl_epi64 ((__m128i *) (pred+32  )); // pred[2]
	xmm3 = _mm_unpacklo_epi8(xmm3,xmm5);
	xmm2 = _mm_adds_epi16(xmm2, xmm3); //  (b[4] + b[3] + 32)>>6 + pred[2]
	xmm2 = _mm_packus_epi16(xmm2,xmm5);
	_mm_storel_epi64((__m128i*)&dest[2*stride], xmm2);
	// 254
	// xmm0 = "32", xmm1 = b[1], xmm4 = ROW[5], xmm5 = 0, xmm6 = ROW[7], xmm7 = ROW[6], src[24] = b[6]
	xmm3 = xmm13;//xmm3 = _mm_load_si128((__m128i*)&src[24]); // b[6]
	xmm2 = xmm3; // b[6]
	xmm2 = _mm_adds_epi16(xmm2, xmm1); //  b[6] + b[1]
	xmm3 = _mm_subs_epi16(xmm3, xmm1); //  b[6] - b[1]
	xmm2 = _mm_adds_epi16(xmm2, xmm0); //  b[6] + b[1] + 32
	xmm2 = _mm_srai_epi16(xmm2, 6); // (b[6] + b[1] + 32)>>6
	xmm3 = _mm_adds_epi16(xmm3, xmm0); //  b[6] - b[1] + 32
	xmm3 = _mm_srai_epi16(xmm3, 6); // (b[6] - b[1] + 32)>>6
	xmm1 = _mm_loadl_epi64 ((__m128i *) (pred+48  )); // pred[3]
	xmm1 = _mm_unpacklo_epi8(xmm1,xmm5);
	xmm2 = _mm_adds_epi16(xmm2, xmm1); //  (b[6] + b[1] + 32)>>6 + pred[3]
	xmm2 = _mm_packus_epi16(xmm2,xmm5);
	_mm_storel_epi64((__m128i*)&dest[3*stride], xmm2);
	// 267
	// xmm0 = "32", xmm3 = ROW[4], xmm4 = ROW[5], xmm5 = 0, xmm6 = ROW[7], xmm7 = ROW[6], src[24] = b[6]
	xmm1 = _mm_loadl_epi64 ((__m128i *) (pred+64  )); // pred[4]
	xmm1 = _mm_unpacklo_epi8(xmm1,xmm5);
	xmm3 = _mm_adds_epi16(xmm3, xmm1); //  (b[6] - b[1] + 32)>>6 + pred[4]
	xmm3 = _mm_packus_epi16(xmm3,xmm5);
	_mm_storel_epi64((__m128i*)&dest[4*stride], xmm3);
	xmm1 = _mm_loadl_epi64 ((__m128i *) (pred+80  )); // pred[5]
	xmm1 = _mm_unpacklo_epi8(xmm1,xmm5);
	xmm4 = _mm_adds_epi16(xmm4, xmm1); //  (b[4] - b[3] + 32)>>6 + pred[5]
	xmm4 = _mm_packus_epi16(xmm4,xmm5);
	_mm_storel_epi64((__m128i*)&dest[5*stride], xmm4);
	xmm1 = _mm_loadl_epi64 ((__m128i *) (pred+96  )); // pred[6]
	xmm1 = _mm_unpacklo_epi8(xmm1,xmm5);
	xmm7 = _mm_adds_epi16(xmm7, xmm1); //  (b[2] - b[5] + 32)>>6 + pred[6]
	xmm7 = _mm_packus_epi16(xmm7,xmm5);
	_mm_storel_epi64((__m128i*)&dest[6*stride], xmm7);
	xmm1 = _mm_loadl_epi64 ((__m128i *) (pred+112 )); // pred[7]
	xmm1 = _mm_unpacklo_epi8(xmm1,xmm5);
	xmm6 = _mm_adds_epi16(xmm6, xmm1); //  (b[0] - b[7] + 32)>>6 + pred[7]
	xmm6 = _mm_packus_epi16(xmm6,xmm5);
	_mm_storel_epi64((__m128i*)&dest[7*stride], xmm6);
	// 287
	__m64 mm7 = _mm_setzero_si64();
	*(__m64*) (&src[0]) = mm7;
	*(__m64*) (&src[4]) = mm7;
	*(__m64*) (&src[8]) = mm7;
	*(__m64*) (&src[12]) = mm7;
	*(__m64*) (&src[16]) = mm7;
	*(__m64*) (&src[20]) = mm7;
	*(__m64*) (&src[24]) = mm7;
	*(__m64*) (&src[28]) = mm7;
	*(__m64*) (&src[32]) = mm7;
	*(__m64*) (&src[36]) = mm7;
	*(__m64*) (&src[40]) = mm7;
	*(__m64*) (&src[44]) = mm7;
	*(__m64*) (&src[48]) = mm7;
	*(__m64*) (&src[52]) = mm7;
	*(__m64*) (&src[56]) = mm7;
	*(__m64*) (&src[60]) = mm7;
	// 295 - 44 if _PRE_TRANSPOSE_
}
#elif defined(USE_FOR_WINDVD) || defined(LINUX_H264_MT)
// SSE intrinsics
void inverse_transform8x8_sse PARGS4(byte *dest, byte *pred, short *src, int stride)
{
	__m64 mm0,  mm1,  mm2,  mm3,  mm4,  mm5,  mm6,  mm7;
	static const __declspec(align(8)) unsigned short ncoeff_32[4] = { 32, 32, 32, 32};

	// 8x8 matrix left-hand part
	mm3 = *((__m64*)&src[0]); // a0 b0 c0 d0
	mm6 = *((__m64*)&src[16]); // a2 b2 c2 d2
	mm0 = *((__m64*)&src[32]); // a4 b4 c4 d4
	mm5 = *((__m64*)&src[40]); // a5 b5 c5 d5
	mm1 = *((__m64*)&src[48]); // a6 b6 c6 d6
	mm2 = *((__m64*)&src[56]); // a7 b7 c7 d7
	
	mm4 = mm3;
	mm7 = mm6;
	
	mm3 = _mm_add_pi16(mm3, mm0); // a0+a4 = a[0]
	mm4 = _mm_sub_pi16(mm4, mm0); // a0-a4 = a[4]
	mm7 = _mm_srai_pi16(mm7, 1); // a2>>1
	mm7 = _mm_sub_pi16(mm7, mm1); // (a2>>1) - a6 = a[2]
	mm1 = _mm_srai_pi16(mm1, 1); // a6>>1
	mm6 = _mm_add_pi16(mm6, mm1); // a2 + (a6>>1) = a[6]
	
	mm0 = mm3;
	mm1 = mm4;
	
	mm0 = _mm_add_pi16(mm0, mm6); // a[0] + a[6] = b[0]
	mm1 = _mm_add_pi16(mm1, mm7); // a[4] + a[2] = b[2]
	mm3 = _mm_sub_pi16(mm3, mm6); // a[0] + a[6] = b[6]
	mm4 = _mm_sub_pi16(mm4, mm7); // a[4] - a[2] = b[4]
	
	*(__m64*) (&src[0]) = mm0;	// b[0]
	*(__m64*) (&src[16]) = mm1; // b[2]
	*(__m64*) (&src[32]) = mm4; // b[4]
	*(__m64*) (&src[48]) = mm3; // b[6]

	// 8x8 matrix right-hand part
	mm3 = *((__m64*)&src[4]); // e0 f0 g0 h0
	mm6 = *((__m64*)&src[20]); // e2 f2 g2 h2
	mm0 = *((__m64*)&src[36]); // e4 f4 g4 h4
	mm1 = *((__m64*)&src[52]); // e6 f6 g6 h6
	
	mm4 = mm3;
	mm7 = mm6;
	
	mm3 = _mm_add_pi16(mm3, mm0); // e0+e4 = a[0]
	mm4 = _mm_sub_pi16(mm4, mm0); // e0-e4 = a[4]
	mm7 = _mm_srai_pi16(mm7, 1); // e2>>1
	mm7 = _mm_sub_pi16(mm7, mm1); // (e2>>1) - e6 = a[2]
	mm1 = _mm_srai_pi16(mm1, 1); // e6>>1
	mm6 = _mm_add_pi16(mm6, mm1); // e2 + (e6>>1) = a[6]
	
	mm0 = mm3;
	mm1 = mm4;
	
	mm0 = _mm_add_pi16(mm0, mm6); // a[0] + a[6] = b[0]
	mm1 = _mm_add_pi16(mm1, mm7); // a[4] + a[2] = b[2]
	mm3 = _mm_sub_pi16(mm3, mm6); // a[0] + a[6] = b[6]
	mm4 = _mm_sub_pi16(mm4, mm7); // a[4] - a[2] = b[4]
	
	*(__m64*) (&src[4]) = mm0;	// b[0]'
	*(__m64*) (&src[20]) = mm1; // b[2]'
	*(__m64*) (&src[36]) = mm4; // b[4]'
	*(__m64*) (&src[52]) = mm3; // b[6]'

	
	// 1
	mm6 = *((__m64*)&src[8]); // a1 b1 c1 d1
	mm7 = *((__m64*)&src[24]); // a3 b3 c3 d3
	
	mm0 = mm6; // a1
	mm1 = mm7; // a3
	mm3 = mm5; // a5
	mm4 = mm2; // a7
	
	mm0 = _mm_srai_pi16(mm0, 1); // a1>>1
	mm1 = _mm_srai_pi16(mm1, 1); // a3>>1
	mm3 = _mm_srai_pi16(mm3, 1); // a5>>1
	mm4 = _mm_srai_pi16(mm4, 1); // a7>>1
	
	mm0 = _mm_add_pi16(mm0, mm6); // a1 + (a1>>1)
	mm1 = _mm_add_pi16(mm1, mm7); // a3 + (a3>>1)
	mm3 = _mm_add_pi16(mm3, mm5); // a5 + (a5>>1)
	mm4 = _mm_add_pi16(mm4, mm2); // a7 + (a7>>1)

	mm0 = _mm_add_pi16(mm0, mm5); //  a5 + a1 + (a1>>1)
	mm1 = _mm_sub_pi16(mm1, mm2); // -a7 + a3 + (a3>>1)
	mm3 = _mm_sub_pi16(mm3, mm6); // -a1 + a5 + (a5>>1)
	mm4 = _mm_add_pi16(mm4, mm7); //  a3 + a7 + (a7>>1)

	mm7 = _mm_add_pi16(mm7, mm0); //  a3 + a5 + a1 + (a1>>1) = a[7]
	mm6 = _mm_sub_pi16(mm6, mm1); //  a1 + a7 - a3 - (a3>>1) = a[3]
	mm2 = _mm_add_pi16(mm2, mm3); //  a7 - a1 + a5 + (a5>>1) = a[5]
	mm5 = _mm_sub_pi16(mm5, mm4); //  a5 - a3 - a7 - (a7>>1) = a[1]
	// 90
	// xmm2= a[5], xmm5= a[1], xmm6 = a[3], xmm7= a[7], src[32] = b[0], src[40] = b[2], src[48] = b[4], src[56] = b[6]
	mm0 = mm7; // a[7]
	mm1 = mm2; // a[5]

	mm0 = _mm_srai_pi16(mm0, 2); // a[7]>>2
	mm1 = _mm_srai_pi16(mm1, 2); // a[5]>>2
	mm0 = _mm_add_pi16(mm0, mm5); //  a[1] + (a[7]>>2) = b[1]
	mm1 = _mm_add_pi16(mm1, mm6); //  a[3] + (a[5]>>2) = b[3]
	*(__m64*) (&src[40]) = mm0;
	*(__m64*) (&src[56]) = mm1;

	mm6 = _mm_srai_pi16(mm6, 2); // a[3]>>2
	mm5 = _mm_srai_pi16(mm5, 2); // a[1]>>2
	mm6 = _mm_sub_pi16(mm6, mm2); //  (a[3]>>2) - a[5] = b[5]
	mm7 = _mm_sub_pi16(mm7, mm5); //  a[7] - (a[1]>>2) = b[7]
	
	mm2 = *((__m64*)&src[0]); // b[0]
	mm3 = *((__m64*)&src[16]); // b[2]
	mm4 = mm2; // b[0]
	mm5 = mm3; // b[2]

	mm2 = _mm_add_pi16(mm2, mm7); //  b[0] + b[7]
	mm4 = _mm_sub_pi16(mm4, mm7); //  b[0] - b[7]
	mm3 = _mm_add_pi16(mm3, mm6); //  b[2] + b[5]
	mm5 = _mm_sub_pi16(mm5, mm6); //  b[2] - b[5]
	
	*(__m64*) (&src[0]) = mm2;	// MM0 a 0 1 2 3
	*(__m64*) (&src[8]) = mm3; // MM1 b 0 1 2 3
	
	*(__m64*) (&src[16]) = mm4;	// xmm4
	*(__m64*) (&src[24]) = mm5; // xmm5
	
	// 2
	mm6 = *((__m64*)&src[12]); // a1 b1 c1 d1
	mm7 = *((__m64*)&src[28]); // a3 b3 c3 d3
	mm5 = *((__m64*)&src[44]); 
	mm2 = *((__m64*)&src[60]); 
	
	mm0 = mm6; // a1
	mm1 = mm7; // a3
	mm3 = mm5; // a5
	mm4 = mm2; // a7
	
	mm0 = _mm_srai_pi16(mm0, 1); // a1>>1
	mm1 = _mm_srai_pi16(mm1, 1); // a3>>1
	mm3 = _mm_srai_pi16(mm3, 1); // a5>>1
	mm4 = _mm_srai_pi16(mm4, 1); // a7>>1
	
	mm0 = _mm_add_pi16(mm0, mm6); // a1 + (a1>>1)
	mm1 = _mm_add_pi16(mm1, mm7); // a3 + (a3>>1)
	mm3 = _mm_add_pi16(mm3, mm5); // a5 + (a5>>1)
	mm4 = _mm_add_pi16(mm4, mm2); // a7 + (a7>>1)

	mm0 = _mm_add_pi16(mm0, mm5); //  a5 + a1 + (a1>>1)
	mm1 = _mm_sub_pi16(mm1, mm2); // -a7 + a3 + (a3>>1)
	mm3 = _mm_sub_pi16(mm3, mm6); // -a1 + a5 + (a5>>1)
	mm4 = _mm_add_pi16(mm4, mm7); //  a3 + a7 + (a7>>1)

	mm7 = _mm_add_pi16(mm7, mm0); //  a3 + a5 + a1 + (a1>>1) = a[7]
	mm6 = _mm_sub_pi16(mm6, mm1); //  a1 + a7 - a3 - (a3>>1) = a[3]
	mm2 = _mm_add_pi16(mm2, mm3); //  a7 - a1 + a5 + (a5>>1) = a[5]
	mm5 = _mm_sub_pi16(mm5, mm4); //  a5 - a3 - a7 - (a7>>1) = a[1]
	// 90
	// xmm2= a[5], xmm5= a[1], xmm6 = a[3], xmm7= a[7], src[32] = b[0], src[40] = b[2], src[48] = b[4], src[56] = b[6]
	mm0 = mm7; // a[7]
	mm1 = mm2; // a[5]

	mm0 = _mm_srai_pi16(mm0, 2); // a[7]>>2
	mm1 = _mm_srai_pi16(mm1, 2); // a[5]>>2
	mm0 = _mm_add_pi16(mm0, mm5); //  a[1] + (a[7]>>2) = b[1]'
	mm1 = _mm_add_pi16(mm1, mm6); //  a[3] + (a[5]>>2) = b[3]'

	mm6 = _mm_srai_pi16(mm6, 2); // a[3]>>2
	mm5 = _mm_srai_pi16(mm5, 2); // a[1]>>2
	mm6 = _mm_sub_pi16(mm6, mm2); //  (a[3]>>2) - a[5] = b[5]'
	mm7 = _mm_sub_pi16(mm7, mm5); //  a[7] - (a[1]>>2) = b[7]'
	
	// 100
	// xmm0 = b[1], xmm1 = b[3], xmm6 = b[5], xmm7= b[7], src[32] = b[0], src[40] = b[2], src[48] = b[4], src[56] = b[6]
	mm2 = *((__m64*)&src[4]); // b[0]'
	mm3 = *((__m64*)&src[20]); // b[2]'
	mm4 = mm2; // b[0]
	mm5 = mm3; // b[2]

	mm2 = _mm_add_pi16(mm2, mm7); //  b[0] + b[7]
	mm4 = _mm_sub_pi16(mm4, mm7); //  b[0] - b[7] // h 4 5 6 7
	mm3 = _mm_add_pi16(mm3, mm6); //  b[2] + b[5]
	mm5 = _mm_sub_pi16(mm5, mm6); //  b[2] - b[5]
	
	*(__m64*) (&src[4]) = mm2;	// MM0' a 4 5 6 7
	*(__m64*) (&src[12]) = mm3; // MM1' b 4 5 6 7
	
	*(__m64*) (&src[20]) = mm4;	// xmm4'
	*(__m64*) (&src[28]) = mm5; // xmm5'

	mm6 = *((__m64*)&src[36]); // b[4]'
	mm7 = *((__m64*)&src[52]); // b[6]'
	
	mm2 = mm6; // b[4]'
	mm3 = mm7; // b[6]'


	mm2 = _mm_add_pi16(mm2, mm1); //  b[4] + b[3] // c 4 5 6 7
	*(__m64*) (&src[20]) = mm2;
	mm6 = _mm_sub_pi16(mm6, mm1); //  b[4] - b[3] // f 4 5 6 7 
	mm3 = _mm_add_pi16(mm3, mm0); //  b[6] + b[1] // d 4 5 6 7
	mm7 = _mm_sub_pi16(mm7, mm0); //  b[6] - b[1]
	
	mm0 = mm7; //e 4 5 6 7
	mm1 = mm5; //g 4 5 6 7 
	
	mm5 = _mm_unpacklo_pi16(mm0, mm6); // e4 f4 e5 f5
	mm7 = _mm_unpacklo_pi16(mm1, mm4); // g4 h4 g5 h5
	mm2 = _mm_unpacklo_pi32(mm5, mm7); // e4 f4 g4 h4
	*(__m64*) (&src[36]) = mm2;			
	mm2 = _mm_unpackhi_pi32(mm5, mm7); // e5 f5 g5 h5
	*(__m64*) (&src[52]) = mm2;	 
	mm5 = _mm_unpackhi_pi16(mm0, mm6); // e6 f6 e7 f7
	mm7 = _mm_unpackhi_pi16(mm1, mm4); // g6 h6 g7 h7
	mm2 = _mm_unpacklo_pi32(mm5, mm7); // e6 f6 g6 h6
	*(__m64*) (&src[28]) = mm2;	 
	mm2 = _mm_unpackhi_pi32(mm5, mm7); // e7 f7 g7 h7
	mm5 = *((__m64*)&src[20]);   // c 4 5 6 7 
	*(__m64*) (&src[20]) = mm2;	 
	
	mm0 = *((__m64*)&src[4]);  // a 4 5 6 7
	mm6 = *((__m64*)&src[12]); // b 4 5 6 7
	mm1 = mm5; 				   // c 4 5 6 7
	mm4 = mm3; 				   // d 4 5 6 7
	mm5 = _mm_unpacklo_pi16(mm0, mm6); // a4 b4 a5 b5
	mm7 = _mm_unpacklo_pi16(mm1, mm4); // c4 d4 c5 d5
	mm2 = _mm_unpacklo_pi32(mm5, mm7); // a4 b4 c4 d4
	*(__m64*) (&src[4]) = mm2;				
	mm2 = _mm_unpackhi_pi32(mm5, mm7); // a5 b5 c5 d5 
	*(__m64*) (&src[12]) = mm2;	          
	mm5 = _mm_unpackhi_pi16(mm0, mm6); // a6 b6 a7 b7
	mm7 = _mm_unpackhi_pi16(mm1, mm4); //  c6 d6 c7 d7
	mm2 = _mm_unpacklo_pi32(mm5, mm7); //  a6 b6 c6 d6
	*(__m64*) (&src[44]) = mm2;	  
	mm2 = _mm_unpackhi_pi32(mm5, mm7); // a7 b7 c7 d7
	*(__m64*) (&src[60]) = mm2;	 

	mm6 = *((__m64*)&src[32]); // b[4]
	mm7 = *((__m64*)&src[48]); // b[6]
	mm2 = mm6; // b[4]
	mm3 = mm7; // b[6]
	mm0 = *((__m64*)&src[40]); 
	mm1 = *((__m64*)&src[56]); 
	mm4 = *((__m64*)&src[16]); // h 0 1 2 3
	mm5 = *((__m64*)&src[24]); //g 0 1 2 3
	
	mm2 = _mm_add_pi16(mm2, mm1); //  b[4] + b[3] // c 0 1 2 3
	*(__m64*) (&src[16]) = mm2;
	mm6 = _mm_sub_pi16(mm6, mm1); //  b[4] - b[3] // f 0 1 2 3 
	mm3 = _mm_add_pi16(mm3, mm0); //  b[6] + b[1] // d 0 1 2 3
	mm7 = _mm_sub_pi16(mm7, mm0); //  b[6] - b[1]
	
	mm0 = mm7; //e 0 1 2 3
	mm1 = mm5; //g 0 1 2 3 
	
	mm5 = _mm_unpacklo_pi16(mm0, mm6); // e0 f0 e1 f1
	mm7 = _mm_unpacklo_pi16(mm1, mm4); // g0 h0 g1 h1
	mm2 = _mm_unpacklo_pi32(mm5, mm7); // e0 f0 g0 h0
	*(__m64*) (&src[32]) = mm2;			
	mm2 = _mm_unpackhi_pi32(mm5, mm7); // e1 f1 g1 h1
	*(__m64*) (&src[40]) = mm2;	 
	mm5 = _mm_unpackhi_pi16(mm0, mm6); // e2 f2 e3 f3
	mm7 = _mm_unpackhi_pi16(mm1, mm4); // g2 h2 g3 h3
	mm2 = _mm_unpacklo_pi32(mm5, mm7); // e2 f2 g2 h2
	*(__m64*) (&src[48]) = mm2;	 
	mm2 = _mm_unpackhi_pi32(mm5, mm7); // e3 f3 g3 h3
	*(__m64*) (&src[56]) = mm2;
	
	mm0 = *((__m64*)&src[0]);  // a 0 1 2 3
	mm6 = *((__m64*)&src[8]); // b 0 1 2 3
	mm5 = *((__m64*)&src[16]);   // c 0 1 2 3
	mm1 = mm5; 				   
	mm4 = mm3; 				   // d 0 1 2 3
	mm5 = _mm_unpacklo_pi16(mm0, mm6); // a0 b0 a1 b1
	mm7 = _mm_unpacklo_pi16(mm1, mm4); // c0 d0 c1 d1
	mm2 = _mm_unpacklo_pi32(mm5, mm7); // a0 b0 c0 d0
	*(__m64*) (&src[0]) = mm2;				
	mm2 = _mm_unpackhi_pi32(mm5, mm7); // a1 b1 c1 d1 
	*(__m64*) (&src[8]) = mm2;	      
	mm5 = _mm_unpackhi_pi16(mm0, mm6); // a2 b2 a3 b3
	mm7 = _mm_unpackhi_pi16(mm1, mm4); //  c2 d2 c3 d3
	mm2 = _mm_unpacklo_pi32(mm5, mm7); //  a2 b2 c2 d2
	*(__m64*) (&src[16]) = mm2;	  
	mm2 = _mm_unpackhi_pi32(mm5, mm7); // a3 b3 c3 d3
	*(__m64*) (&src[24]) = mm2;	 

	//abcd
	mm1 = *((__m64*)&src[0]);  // mm[0]
	mm5 = *((__m64*)&src[16]); // mm[2]
	mm6 = mm1;
	mm0 = mm5;
	mm4 = *((__m64*)&src[4]); // mm[4]
	mm3 = *((__m64*)&src[44]); // mm[6]
	
	mm1 = _mm_add_pi16(mm1, mm4); // mm[0] + mm[4] = a[0]
	mm6 = _mm_sub_pi16(mm6, mm4); // mm[0] - mm[4] = a[4]
	mm5 = _mm_srai_pi16(mm5, 1); // mm[2]>>1
	mm5 = _mm_sub_pi16(mm5, mm3); // (mm[2]>>1) - mm[6] = a[2]
	mm3 = _mm_srai_pi16(mm3, 1); // mm[6]>>1
	mm0 = _mm_add_pi16(mm0, mm3); // mm[2] + (mm[6]>>1) = a[6]
	// 170
	// mm0 = a[6], mm1 = a[0], mm2 = mm[5], mm5 = a[2], mm6 = a[4], mm7 = mm[7]
	mm4 = mm1; // a[0]
	mm3 = mm6; // a[4]

	mm4 = _mm_add_pi16(mm4, mm0); // a[0] + a[6] = b[0]
	mm3 = _mm_add_pi16(mm3, mm5); // a[4] + a[2] = b[2]
	mm1 = _mm_sub_pi16(mm1, mm0); // a[0] - a[6] = b[6]
	mm6 = _mm_sub_pi16(mm6, mm5); // a[4] - a[2] = b[4]
	
	*(__m64*) (&src[0]) = mm4;	
	*(__m64*) (&src[16]) = mm3;	
	*(__m64*) (&src[4]) = mm1;	
	*(__m64*) (&src[44]) = mm6;	
	
	//efgh
	mm1 = *((__m64*)&src[32]);  // mm[0]
	mm5 = *((__m64*)&src[48]); // mm[2]
	mm6 = mm1;
	mm0 = mm5;
	mm4 = *((__m64*)&src[36]); // mm[4]
	mm3 = *((__m64*)&src[28]); // mm[6]
	
	mm1 = _mm_add_pi16(mm1, mm4); // mm[0] + mm[4] = a[0]
	mm6 = _mm_sub_pi16(mm6, mm4); // mm[0] - mm[4] = a[4]
	mm5 = _mm_srai_pi16(mm5, 1); // mm[2]>>1
	mm5 = _mm_sub_pi16(mm5, mm3); // (mm[2]>>1) - mm[6] = a[2]
	mm3 = _mm_srai_pi16(mm3, 1); // mm[6]>>1
	mm0 = _mm_add_pi16(mm0, mm3); // mm[2] + (mm[6]>>1) = a[6]
	// 170
	// mm0 = a[6], mm1 = a[0], mm2 = mm[5], mm5 = a[2], mm6 = a[4], mm7 = mm[7]
	mm4 = mm1; // a[0]
	mm3 = mm6; // a[4]

	mm4 = _mm_add_pi16(mm4, mm0); // a[0] + a[6] = b[0]'
	mm3 = _mm_add_pi16(mm3, mm5); // a[4] + a[2] = b[2]'
	mm1 = _mm_sub_pi16(mm1, mm0); // a[0] - a[6] = b[6]'
	mm6 = _mm_sub_pi16(mm6, mm5); // a[4] - a[2] = b[4]'
	
	*(__m64*) (&src[32]) = mm4;	
	*(__m64*) (&src[48]) = mm3;	
	*(__m64*) (&src[36]) = mm1;	
	*(__m64*) (&src[28]) = mm6;	

	//abcd
	mm0 = *((__m64*)&src[8]);  // mm[1]
	mm5 = *((__m64*)&src[24]); // mm[3]
	mm2 = *((__m64*)&src[12]);  // mm[5]
	mm7 = *((__m64*)&src[60]); // mm[7]
	mm1 = mm0; // a1
	mm3 = mm5; // a3
	mm4 = mm2;
	mm6 = mm7;
	
	mm1 = _mm_srai_pi16(mm1, 1); // a1>>1
	mm3 = _mm_srai_pi16(mm3, 1); // a3>>1
	mm4 = _mm_srai_pi16(mm4, 1); // a5>>1
	mm6 = _mm_srai_pi16(mm6, 1); // a7>>1

	mm1 = _mm_add_pi16(mm1, mm0); // a1 + (a1>>1)
	mm3 = _mm_add_pi16(mm3, mm5); // a3 + (a3>>1)
	mm4 = _mm_add_pi16(mm4, mm2); // a5 + (a5>>1)
	mm6 = _mm_add_pi16(mm6, mm7); // a7 + (a7>>1)

	mm1 = _mm_add_pi16(mm1, mm2); //  a5 + a1 + (a1>>1)
	mm3 = _mm_sub_pi16(mm3, mm7); // -a7 + a3 + (a3>>1)
	mm4 = _mm_sub_pi16(mm4, mm0); // -a1 + a5 + (a5>>1)
	mm6 = _mm_add_pi16(mm6, mm5); //  a3 + a7 + (a7>>1)

	mm5 = _mm_add_pi16(mm5, mm1); //  a3 + a5 + a1 + (a1>>1) = a[7]
	mm0 = _mm_sub_pi16(mm0, mm3); //  a1 + a7 - a3 - (a3>>1) = a[3]
	mm7 = _mm_add_pi16(mm7, mm4); //  a7 - a1 + a5 + (a5>>1) = a[5]
	mm2 = _mm_sub_pi16(mm2, mm6); //  a5 - a3 - a7 - (a7>>1) = a[1]
	// 202
	// mm0 = a[3], mm2 = a[1], mm5 = a[7], mm7 = a[5]
	mm1 = mm5; // a[7]
	mm3 = mm7; // a[5]

	mm1 = _mm_srai_pi16(mm1, 2); // a[7]>>2
	mm3 = _mm_srai_pi16(mm3, 2); // a[5]>>2
	mm1 = _mm_add_pi16(mm1, mm2); //  a[1] + (a[7]>>2) = b[1]
	mm3 = _mm_add_pi16(mm3, mm0); //  a[3] + (a[5]>>2) = b[3]

	mm2 = _mm_srai_pi16(mm2, 2); // a[1]>>2
	mm0 = _mm_srai_pi16(mm0, 2); // a[3]>>2
	mm5 = _mm_sub_pi16(mm5, mm2); //  a[7] - (a[1]>>2) = b[7]
	mm0 = _mm_sub_pi16(mm0, mm7); //  (a[3]>>2) - a[5] = b[5]
	
	*(__m64*) (&src[8]) = mm1;	  
	*(__m64*) (&src[24]) = mm3;	
	*(__m64*) (&src[12]) = mm5;	
	*(__m64*) (&src[60]) = mm0;	
	
	//efgh
	mm0 = *((__m64*)&src[40]);  // mm[1]'
	mm5 = *((__m64*)&src[56]); // mm[3]'
	mm2 = *((__m64*)&src[52]);  // mm[5]'
	mm7 = *((__m64*)&src[20]); // mm[7]'
	mm1 = mm0; // a1
	mm3 = mm5; // a3
	mm4 = mm2;
	mm6 = mm7;
	
	mm1 = _mm_srai_pi16(mm1, 1); // a1>>1
	mm3 = _mm_srai_pi16(mm3, 1); // a3>>1
	mm4 = _mm_srai_pi16(mm4, 1); // a5>>1
	mm6 = _mm_srai_pi16(mm6, 1); // a7>>1

	mm1 = _mm_add_pi16(mm1, mm0); // a1 + (a1>>1)
	mm3 = _mm_add_pi16(mm3, mm5); // a3 + (a3>>1)
	mm4 = _mm_add_pi16(mm4, mm2); // a5 + (a5>>1)
	mm6 = _mm_add_pi16(mm6, mm7); // a7 + (a7>>1)

	mm1 = _mm_add_pi16(mm1, mm2); //  a5 + a1 + (a1>>1)
	mm3 = _mm_sub_pi16(mm3, mm7); // -a7 + a3 + (a3>>1)
	mm4 = _mm_sub_pi16(mm4, mm0); // -a1 + a5 + (a5>>1)
	mm6 = _mm_add_pi16(mm6, mm5); //  a3 + a7 + (a7>>1)

	mm5 = _mm_add_pi16(mm5, mm1); //  a3 + a5 + a1 + (a1>>1) = a[7]
	mm0 = _mm_sub_pi16(mm0, mm3); //  a1 + a7 - a3 - (a3>>1) = a[3]
	mm7 = _mm_add_pi16(mm7, mm4); //  a7 - a1 + a5 + (a5>>1) = a[5]
	mm2 = _mm_sub_pi16(mm2, mm6); //  a5 - a3 - a7 - (a7>>1) = a[1]
	// 202
	// mm0 = a[3], mm2 = a[1], mm5 = a[7], mm7 = a[5]
	mm1 = mm5; // a[7]
	mm3 = mm7; // a[5]

	mm1 = _mm_srai_pi16(mm1, 2); // a[7]>>2
	mm3 = _mm_srai_pi16(mm3, 2); // a[5]>>2
	mm1 = _mm_add_pi16(mm1, mm2); //  a[1] + (a[7]>>2) = b[1]'
	mm3 = _mm_add_pi16(mm3, mm0); //  a[3] + (a[5]>>2) = b[3]'

	mm2 = _mm_srai_pi16(mm2, 2); // a[1]>>2
	mm0 = _mm_srai_pi16(mm0, 2); // a[3]>>2
	mm5 = _mm_sub_pi16(mm5, mm2); //  a[7] - (a[1]>>2) = b[7]'
	mm0 = _mm_sub_pi16(mm0, mm7); //  (a[3]>>2) - a[5] = b[5]'
	
	*(__m64*) (&src[40]) = mm1;	
	*(__m64*) (&src[56]) = mm3;	
	*(__m64*) (&src[52]) = mm5;	
	*(__m64*) (&src[20]) = mm0;

	// 212
	// mm0 = b[5], mm1 = b[1], mm3 = b[3], mm5 = b[7]
	//abcd
	 mm2 = *((__m64*)&src[0]);  // b[0]
	mm4 = *((__m64*)&src[16]); // b[2]
	mm6 = mm2;

	mm7 = mm4;
	mm0 = *((__m64*)&src[60]);  // b[5]
	mm5 = *((__m64*)&src[12]); // b[7]
	
	mm2 = _mm_add_pi16(mm2, mm5); //  b[0] + b[7]
	mm6 = _mm_sub_pi16(mm6, mm5); //  b[0] - b[7]
	mm4 = _mm_add_pi16(mm4, mm0); //  b[2] + b[5]
	mm7 = _mm_sub_pi16(mm7, mm0); //  b[2] - b[5]
	// 220
	// mm1 = b[1], mm2 = ROW[0], mm3 = b[3], mm4 = ROW[1], mm6 = ROW[7], mm7 = ROW[6]
	mm5 = *((__m64*)&ncoeff_32[0]); // all 32s
	mm0 = *((__m64*) (pred+0  )); // pred[0]
	mm2 = _mm_add_pi16(mm2, mm5); //  b[0] + b[7] + 32
	mm2 = _mm_srai_pi16(mm2, 6); // (b[0] + b[7] + 32)>>6
	mm6 = _mm_add_pi16(mm6, mm5); //  b[0] - b[7] + 32
	mm6 = _mm_srai_pi16(mm6, 6); // (b[0] - b[7] + 32)>>6
	mm4 = _mm_add_pi16(mm4, mm5); //  b[2] + b[5] + 32
	mm4 = _mm_srai_pi16(mm4, 6); // (b[2] + b[5] + 32)>>6
	mm7 = _mm_add_pi16(mm7, mm5); //  b[2] - b[5] + 32
	mm7 = _mm_srai_pi16(mm7, 6); // (b[2] - b[5] + 32)>>6
	// 230
	// xmm0 = pred[0], xmm1 = b[1], xmm2 = ROW[0], xmm3 = b[3], xmm4 = ROW[1], xmm5 = "32", xmm6 = ROW[7], xmm7 = ROW[6],
	mm5 = _mm_setzero_si64(); // all 0s
	mm0 = _mm_unpacklo_pi8(mm0,mm5);
	mm2 = _mm_add_pi16(mm2, mm0); //  (b[0] + b[7] + 32)>>6 + pred[0]
	*(__m64*) (&src[0]) = mm2;


	mm0 = *((__m64*) (pred+16  )); // pred[1]
	mm0 = _mm_unpacklo_pi8(mm0,mm5);
	mm4 = _mm_add_pi16(mm4, mm0); //  (b[2] + b[5] + 32)>>6 + pred[1]
	*(__m64*) (&src[16]) = mm4;
	
	mm2 = *((__m64*)&src[44]);  // b[4]
	mm4 = mm2; // b[4]
	mm3 = *((__m64*)&src[24]); // b[3]

	mm2 = _mm_add_pi16(mm2, mm3); //  b[4] + b[3]
	mm4 = _mm_sub_pi16(mm4, mm3); //  b[4] - b[3]
	mm0 = *((__m64*)&ncoeff_32[0]); // all 32s
	mm2 = _mm_add_pi16(mm2, mm0); //  b[4] + b[3] + 32
	mm2 = _mm_srai_pi16(mm2, 6); // (b[4] + b[3] + 32)>>6
	mm4 = _mm_add_pi16(mm4, mm0); //  b[4] - b[3] + 32
	mm4 = _mm_srai_pi16(mm4, 6); // (b[4] - b[3] + 32)>>6
	mm3 = *((__m64*) (pred+32  )); // pred[2]
	mm3 = _mm_unpacklo_pi8(mm3,mm5);
	mm2 = _mm_add_pi16(mm2, mm3); //  (b[4] + b[3] + 32)>>6 + pred[2]
	*(__m64*) (&src[60]) = mm2;
	
	mm3 = *((__m64*)&src[4]);  // b[6]
	mm2 = mm3; // b[6]
	mm1 = *((__m64*)&src[8]); // b[1]

	mm2 = _mm_add_pi16(mm2, mm1); //  b[6] + b[1]
	mm3 = _mm_sub_pi16(mm3, mm1); //  b[6] - b[1]
	mm2 = _mm_add_pi16(mm2, mm0); //  b[6] + b[1] + 32
	mm2 = _mm_srai_pi16(mm2, 6); // (b[6] + b[1] + 32)>>6
	mm3 = _mm_add_pi16(mm3, mm0); //  b[6] - b[1] + 32
	mm3 = _mm_srai_pi16(mm3, 6); // (b[6] - b[1] + 32)>>6
	mm1 = *((__m64*) (pred+48  )); // pred[3]
	mm1 = _mm_unpacklo_pi8(mm1,mm5);
	mm2 = _mm_add_pi16(mm2, mm1); //  (b[6] + b[1] + 32)>>6 + pred[3]
	*(__m64*) (&src[12]) = mm2;
	
	mm1 = *((__m64*) (pred+64  )); // pred[4]
	mm1 = _mm_unpacklo_pi8(mm1,mm5);
	mm3 = _mm_add_pi16(mm3, mm1); //  (b[6] - b[1] + 32)>>6 + pred[4]
	*(__m64*) (&src[44]) = mm3;
	
	mm1 = *((__m64*) (pred+80  )); // pred[5]
	mm1 = _mm_unpacklo_pi8(mm1,mm5);
	mm4 = _mm_add_pi16(mm4, mm1); //  (b[4] - b[3] + 32)>>6 + pred[5]
	*(__m64*) (&src[24]) = mm4;
	
	mm1 = *((__m64*) (pred+96  )); // pred[6]
	mm1 = _mm_unpacklo_pi8(mm1,mm5);
	mm7 = _mm_add_pi16(mm7, mm1); //  (b[2] - b[5] + 32)>>6 + pred[6]
	*(__m64*) (&src[4]) = mm7;
	
	mm1 = *((__m64*) (pred+112  )); // pred[7]
	mm1 = _mm_unpacklo_pi8(mm1,mm5);
	mm6 = _mm_add_pi16(mm6, mm1); //  (b[0] - b[7] + 32)>>6 + pred[7]
	*(__m64*) (&src[8]) = mm6;
	


	//efgh
	mm2 = *((__m64*)&src[32]);  // b[0]'
	mm4 = *((__m64*)&src[48]); // b[2]'
	mm6 = mm2;
	mm7 = mm4;
	mm0 = *((__m64*)&src[20]);  // b[5]'
	mm5 = *((__m64*)&src[52]); // b[7]'
	
	mm2 = _mm_add_pi16(mm2, mm5); //  b[0] + b[7]
	mm6 = _mm_sub_pi16(mm6, mm5); //  b[0] - b[7]
	mm4 = _mm_add_pi16(mm4, mm0); //  b[2] + b[5]
	mm7 = _mm_sub_pi16(mm7, mm0); //  b[2] - b[5]
	// 220
	// mm1 = b[1], mm2 = ROW[0], mm3 = b[3], mm4 = ROW[1], mm6 = ROW[7], mm7 = ROW[6]
	mm5 = *((__m64*)&ncoeff_32[0]); // all 32s
	mm0 = *((__m64*) (pred+4  )); // pred[0]
	mm2 = _mm_add_pi16(mm2, mm5); //  b[0] + b[7] + 32
	mm2 = _mm_srai_pi16(mm2, 6); // (b[0] + b[7] + 32)>>6
	mm6 = _mm_add_pi16(mm6, mm5); //  b[0] - b[7] + 32
	mm6 = _mm_srai_pi16(mm6, 6); // (b[0] - b[7] + 32)>>6
	mm4 = _mm_add_pi16(mm4, mm5); //  b[2] + b[5] + 32
	mm4 = _mm_srai_pi16(mm4, 6); // (b[2] + b[5] + 32)>>6
	mm7 = _mm_add_pi16(mm7, mm5); //  b[2] - b[5] + 32
	mm7 = _mm_srai_pi16(mm7, 6); // (b[2] - b[5] + 32)>>6
	// 230						
	// xmm0 = pred[0], xmm1 = b[1], xmm2 = ROW[0], xmm3 = b[3], xmm4 = ROW[1], xmm5 = "32", xmm6 = ROW[7], xmm7 = ROW[6],
	
	mm5 = _mm_setzero_si64(); // all 0s
	mm0 = _mm_unpacklo_pi8(mm0,mm5);
	mm2 = _mm_add_pi16(mm2, mm0); //  (b[0] + b[7] + 32)>>6 + pred[0]
	*(__m64*) (&src[32]) = mm2;

	
	mm0 = *((__m64*) (pred+20  )); // pred[1]
	mm0 = _mm_unpacklo_pi8(mm0,mm5);
	mm4 = _mm_add_pi16(mm4, mm0); //  (b[2] + b[5] + 32)>>6 + pred[1]
	*(__m64*) (&src[48]) = mm4;
	
	mm2 = *((__m64*)&src[28]);  // b[4]'
	mm4 = mm2; // b[4]
	mm3 = *((__m64*)&src[56]); // b[3]'

	mm2 = _mm_add_pi16(mm2, mm3); //  b[4] + b[3]
	mm4 = _mm_sub_pi16(mm4, mm3); //  b[4] - b[3]
	mm0 = *((__m64*)&ncoeff_32[0]); // all 32s
	mm2 = _mm_add_pi16(mm2, mm0); //  b[4] + b[3] + 32
	mm2 = _mm_srai_pi16(mm2, 6); // (b[4] + b[3] + 32)>>6
	mm4 = _mm_add_pi16(mm4, mm0); //  b[4] - b[3] + 32
	mm4 = _mm_srai_pi16(mm4, 6); // (b[4] - b[3] + 32)>>6
	mm3 = *((__m64*) (pred+36  )); // pred[2]
	mm3 = _mm_unpacklo_pi8(mm3,mm5);
	mm2 = _mm_add_pi16(mm2, mm3); //  (b[4] + b[3] + 32)>>6 + pred[2]
	*(__m64*) (&src[20]) = mm2;
	
	mm3 = *((__m64*)&src[36]);  // b[6]'
	mm2 = mm3; // b[6]
	mm1 = *((__m64*)&src[40]); // b[1]'

	mm2 = _mm_add_pi16(mm2, mm1); //  b[6] + b[1]
	mm3 = _mm_sub_pi16(mm3, mm1); //  b[6] - b[1]
	mm2 = _mm_add_pi16(mm2, mm0); //  b[6] + b[1] + 32
	mm2 = _mm_srai_pi16(mm2, 6); // (b[6] + b[1] + 32)>>6
	mm3 = _mm_add_pi16(mm3, mm0); //  b[6] - b[1] + 32
	mm3 = _mm_srai_pi16(mm3, 6); // (b[6] - b[1] + 32)>>6
	mm1 = *((__m64*) (pred+52  )); // pred[3]
	mm1 = _mm_unpacklo_pi8(mm1,mm5);
	mm2 = _mm_add_pi16(mm2, mm1); //  (b[6] + b[1] + 32)>>6 + pred[3]
	*(__m64*) (&src[52]) = mm2;
	
	mm1 = *((__m64*) (pred+68  )); // pred[4]
	mm1 = _mm_unpacklo_pi8(mm1,mm5);
	mm3 = _mm_add_pi16(mm3, mm1); //  (b[6] - b[1] + 32)>>6 + pred[4]
	*(__m64*) (&src[28]) = mm3;
	
	mm1 = *((__m64*) (pred+84  )); // pred[5]
	mm1 = _mm_unpacklo_pi8(mm1,mm5);
	mm4 = _mm_add_pi16(mm4, mm1); //  (b[4] - b[3] + 32)>>6 + pred[5]
	*(__m64*) (&src[56]) = mm4;
	
	mm1 = *((__m64*) (pred+100  )); // pred[6]
	mm1 = _mm_unpacklo_pi8(mm1,mm5);
	mm7 = _mm_add_pi16(mm7, mm1); //  (b[2] - b[5] + 32)>>6 + pred[6]
	*(__m64*) (&src[36]) = mm7;
	
	mm1 = *((__m64*) (pred+116  )); // pred[7]
	mm1 = _mm_unpacklo_pi8(mm1,mm5);
	mm6 = _mm_add_pi16(mm6, mm1); //  (b[0] - b[7] + 32)>>6 + pred[7]
	*(__m64*) (&src[40]) = mm6;

	
	mm0 = *((__m64*)&src[0]);
	mm1 = *((__m64*)&src[32]);
	mm2 = _mm_packs_pu16(mm0,mm1);
	*((__m64*)&dest[0]) = mm2;
	
	mm0 = *((__m64*)&src[16]);
	mm1 = *((__m64*)&src[48]);
	mm4 = _mm_packs_pu16(mm0, mm1);
	*((__m64*)&dest[1*stride]) = mm4;

	mm0 = *((__m64*)&src[60]);
	mm1 = *((__m64*)&src[20]);
	mm2 = _mm_packs_pu16(mm0,mm1);
	*((__m64*)&dest[2*stride]) = mm2;

	mm0 = *((__m64*)&src[12]);
	mm1 = *((__m64*)&src[52]);
	mm2 = _mm_packs_pu16(mm0,mm1);
	*((__m64*)&dest[3*stride]) = mm2;

	mm0 = *((__m64*)&src[44]);
	mm1 = *((__m64*)&src[28]);
	mm3 = _mm_packs_pu16(mm0,mm1);
	*((__m64*)&dest[4*stride]) = mm3;
		
	mm0 = *((__m64*)&src[24]);
	mm1 = *((__m64*)&src[56]);
	mm4 = _mm_packs_pu16(mm0,mm1);
	*((__m64*)&dest[5*stride]) = mm4;

	mm0 = *((__m64*)&src[4]);
	mm1 = *((__m64*)&src[36]);
	mm7 = _mm_packs_pu16(mm0,mm1);
	*((__m64*)&dest[6*stride]) = mm7;

	mm0 = *((__m64*)&src[8]);
	mm1 = *((__m64*)&src[40]);
	mm6 = _mm_packs_pu16(mm0,mm1);
	*((__m64*)&dest[7*stride]) = mm6;
	

	mm7 = _mm_setzero_si64();
	*(__m64*) (&src[0]) = mm7;
	*(__m64*) (&src[4]) = mm7;
	*(__m64*) (&src[8]) = mm7;
	*(__m64*) (&src[12]) = mm7;
	*(__m64*) (&src[16]) = mm7;
	*(__m64*) (&src[20]) = mm7;
	*(__m64*) (&src[24]) = mm7;
	*(__m64*) (&src[28]) = mm7;
	*(__m64*) (&src[32]) = mm7;
	*(__m64*) (&src[36]) = mm7;
	*(__m64*) (&src[40]) = mm7;
	*(__m64*) (&src[44]) = mm7;
	*(__m64*) (&src[48]) = mm7;
	*(__m64*) (&src[52]) = mm7;
	*(__m64*) (&src[56]) = mm7;
	*(__m64*) (&src[60]) = mm7;
}



// New code, SSE2 intrinsics
void inverse_transform8x8_sse2 PARGS4(byte *dest, byte *pred, short *src, int stride)
{
	__m128i xmm0,xmm1,xmm2,xmm3,xmm4,xmm5,xmm6,xmm7;
	static unsigned short __declspec(align(16)) const_32[8] = { 32, 32, 32, 32, 32, 32, 32, 32 };
	static unsigned char *ptr_32 = (unsigned char *) &const_32[0];

#ifdef _PRE_TRANSPOSE_
	xmm3 = _mm_load_si128((__m128i*)&src[0 ]); // a0 b0 c0 d0 e0 f0 g0 h0
	xmm6 = _mm_load_si128((__m128i*)&src[16]); // a2 b2 c2 d2 e2 f2 g2 h2
	xmm0 = _mm_load_si128((__m128i*)&src[32]); // a4 b4 c4 d4 e4 f4 g4 h4
	xmm5 = _mm_load_si128((__m128i*)&src[40]); // a5 b5 c5 d5 e5 f5 g5 h5
	xmm1 = _mm_load_si128((__m128i*)&src[48]); // a6 b6 c6 d6 e6 f6 g6 h6
	xmm2 = _mm_load_si128((__m128i*)&src[56]); // a7 b7 c7 d7 e7 f7 g7 h7
	// 6
#else
	// Transpose 8x8 matrix
	//start	
	//__asm int 3;
	xmm0 = _mm_load_si128((__m128i*)&src[0 ]); //a 0 1 2 3 4 5 6 7
	xmm1 = _mm_load_si128((__m128i*)&src[8 ]); //b 0 1 2 3 4 5 6 7
	xmm2 = _mm_load_si128((__m128i*)&src[16]); //c 0 1 2 3 4 5 6 7
	xmm3 = _mm_load_si128((__m128i*)&src[24]); //d 0 1 2 3 4 5 6 7

	xmm6 = xmm0; //a 0 1 2 3 4 5 6 7
	xmm7 = xmm2; //c 0 1 2 3 4 5 6 7

	xmm0 = _mm_unpacklo_epi16(xmm0,xmm1); // a0 b0 a1 b1 a2 b2 a3 b3
	xmm2 = _mm_unpacklo_epi16(xmm2,xmm3); // c0 d0 c1 d1 c2 d2 c3 d3
	xmm6 = _mm_unpackhi_epi16(xmm6,xmm1); // a4 b4 a5 b5 a6 b6 a7 b7
	xmm7 = _mm_unpackhi_epi16(xmm7,xmm3); // c4 d4 c5 d5 c6 d6 c7 d7

	xmm1 = xmm0; // a0 b0 a1 b1 a2 b2 a3 b3
	xmm3 = xmm6; // a4 b4 a5 b5 a6 b6 a7 b7

	xmm0 = _mm_unpacklo_epi32(xmm0,xmm2); // a0 b0 c0 d0 a1 b1 c1 d1
	xmm6 = _mm_unpacklo_epi32(xmm6,xmm7); // a4 b4 c4 d4 a5 b5 c5 d5
	xmm1 = _mm_unpackhi_epi32(xmm1,xmm2); // a2 b2 c2 d2 a3 b3 c3 d3
	xmm3 = _mm_unpackhi_epi32(xmm3,xmm7); // a6 b6 c6 d6 a7 b7 c7 d7
	// 16
	// xmm0 = abcd-0/1, xmm1 = abcd-2/3, xmm3 = abcd-6/7, xmm6 = abcd-4/5
	_mm_store_si128((__m128i*)&src[0 ], xmm6); // a4 b4 c4 d4 a5 b5 c5 d5
	_mm_store_si128((__m128i*)&src[16], xmm3); // a6 b6 c6 d6 a7 b7 c7 d7

	xmm4 = _mm_load_si128((__m128i*)&src[32]); //e 0 1 2 3 4 5 6 7
	xmm5 = _mm_load_si128((__m128i*)&src[40]); //f 0 1 2 3 4 5 6 7
	xmm6 = _mm_load_si128((__m128i*)&src[48]); //g 0 1 2 3 4 5 6 7
	xmm7 = _mm_load_si128((__m128i*)&src[56]); //h 0 1 2 3 4 5 6 7

	xmm2 = xmm4; //e 0 1 2 3 4 5 6 7
	xmm3 = xmm6; //g 0 1 2 3 4 5 6 7

	xmm2 = _mm_unpacklo_epi16(xmm2,xmm5); // e0 f0 e1 f1 e2 f2 e3 f3
	xmm3 = _mm_unpacklo_epi16(xmm3,xmm7); // g0 h0 g1 h1 g2 h2 g3 h3
	xmm4 = _mm_unpackhi_epi16(xmm4,xmm5); // e4 f4 e5 f5 e6 f6 e7 f7
	xmm6 = _mm_unpackhi_epi16(xmm6,xmm7); // g4 h4 g5 h5 g6 h6 g7 h7

	xmm5 = xmm2; // e0 f0 e1 f1 e2 f2 e3 f3
	xmm7 = xmm4; // e4 f4 e5 f5 e6 f6 e7 f7

	xmm5 = _mm_unpacklo_epi32(xmm5,xmm3); // e0 f0 g0 h0 e1 f1 g1 h1
	xmm7 = _mm_unpacklo_epi32(xmm7,xmm6); // e4 f4 g4 h4 e5 f5 g5 h5
	xmm2 = _mm_unpackhi_epi32(xmm2,xmm3); // e2 f2 g2 h2 e3 f3 g3 h3
	xmm4 = _mm_unpackhi_epi32(xmm4,xmm6); // e6 f6 g6 h6 e7 f7 g7 h7

	xmm3 = xmm0; // a0 b0 c0 d0 a1 b1 c1 d1
	xmm6 = xmm1; // a2 b2 c2 d2 a3 b3 c3 d3
	// 36
	// xmm0= abcd-0/1, xmm1= abcd-2/3, xmm2= efgh-2/3, xmm3= abcd-0/1, xmm4= efgh-6/7, xmm5= efgh-0/1, xmm6= abcd-2/3, xmm7= efgh-4/5
	// src[0]= abcd-4/5, src[8]= abcd-6/7
	xmm3 = _mm_unpacklo_epi64(xmm3,xmm5); // a0 b0 c0 d0 e0 f0 g0 h0
	xmm6 = _mm_unpacklo_epi64(xmm6,xmm2); // a2 b2 c2 d2 e2 f2 g2 h2
	xmm0 = _mm_unpackhi_epi64(xmm0,xmm5); // a1 b1 c1 d1 e1 f1 g1 h1
	xmm1 = _mm_unpackhi_epi64(xmm1,xmm2); // a3 b3 c3 d3 e3 f3 g3 h3

	_mm_store_si128((__m128i*)&src[8 ], xmm0); // a1 b1 c1 d1 e1 f1 g1 h1
	_mm_store_si128((__m128i*)&src[24], xmm1); // a3 b3 c3 d3 e3 f3 g3 h3

	xmm5 = _mm_load_si128((__m128i*)&src[0 ]); // a4 b4 c4 d4 a5 b5 c5 d5
	xmm2 = _mm_load_si128((__m128i*)&src[16]); // a6 b6 c6 d6 a7 b7 c7 d7

	xmm0 = xmm5; // a4 b4 c4 d4 a5 b5 c5 d5
	xmm1 = xmm2; // a6 b6 c6 d6 a7 b7 c7 d7

	xmm0 = _mm_unpacklo_epi64(xmm0,xmm7); // a4 b4 c4 d4 e4 f4 g4 h4
	xmm1 = _mm_unpacklo_epi64(xmm1,xmm4); // a6 b6 c6 d6 e6 f6 g6 h6
	xmm5 = _mm_unpackhi_epi64(xmm5,xmm7); // a5 b5 c5 d5 e5 f5 g5 h5
	xmm2 = _mm_unpackhi_epi64(xmm2,xmm4); // a7 b7 c7 d7 e7 f7 g7 h7
	// 50
#endif
	// xmm0= a4, xmm1= a6, xmm2= a7, xmm3= a0, xmm5= a5, xmm6= a2, src[16] = a1, src[24] = a3
	xmm4 = xmm3; // a0
	xmm7 = xmm6; // a2

	xmm3 = _mm_adds_epi16(xmm3, xmm0); // a0+a4 = a[0]
	xmm4 = _mm_subs_epi16(xmm4, xmm0); // a0-a4 = a[4]
	xmm7 = _mm_srai_epi16(xmm7, 1); // a2>>1
	xmm7 = _mm_subs_epi16(xmm7, xmm1); // (a2>>1) - a6 = a[2]
	xmm1 = _mm_srai_epi16(xmm1, 1); // a6>>1
	xmm6 = _mm_adds_epi16(xmm6, xmm1); // a2 + (a6>>1) = a[6]

	xmm0 = xmm3; // a[0]
	xmm1 = xmm4; // a[4]

	xmm0 = _mm_adds_epi16(xmm0, xmm6); // a[0] + a[6] = b[0]
	xmm1 = _mm_adds_epi16(xmm1, xmm7); // a[4] + a[2] = b[2]
	xmm3 = _mm_subs_epi16(xmm3, xmm6); // a[0] - a[6] = b[6]
	xmm4 = _mm_subs_epi16(xmm4, xmm7); // a[4] - a[2] = b[4]
	// 64
	// xmm0= b[0], xmm1= b[2], xmm2= a7, xmm3= b[6], xmm4 = b[4], xmm5= a5, src[16] = a1, src[24] = a3
	_mm_store_si128((__m128i*)&src[32], xmm0); // b[0]
	_mm_store_si128((__m128i*)&src[40], xmm1); // b[2]
	_mm_store_si128((__m128i*)&src[48], xmm4); // b[4]
	_mm_store_si128((__m128i*)&src[56], xmm3); // b[6]

	xmm6 = _mm_load_si128((__m128i*)&src[8 ]); // a1
	xmm7 = _mm_load_si128((__m128i*)&src[24]); // a3

	xmm0 = xmm6; // a1
	xmm1 = xmm7; // a3
	xmm3 = xmm5; // a5
	xmm4 = xmm2; // a7

	xmm0 = _mm_srai_epi16(xmm0, 1); // a1>>1
	xmm1 = _mm_srai_epi16(xmm1, 1); // a3>>1
	xmm3 = _mm_srai_epi16(xmm3, 1); // a5>>1
	xmm4 = _mm_srai_epi16(xmm4, 1); // a7>>1

	xmm0 = _mm_adds_epi16(xmm0, xmm6); // a1 + (a1>>1)
	xmm1 = _mm_adds_epi16(xmm1, xmm7); // a3 + (a3>>1)
	xmm3 = _mm_adds_epi16(xmm3, xmm5); // a5 + (a5>>1)
	xmm4 = _mm_adds_epi16(xmm4, xmm2); // a7 + (a7>>1)

	xmm0 = _mm_adds_epi16(xmm0, xmm5); //  a5 + a1 + (a1>>1)
	xmm1 = _mm_subs_epi16(xmm1, xmm2); // -a7 + a3 + (a3>>1)
	xmm3 = _mm_subs_epi16(xmm3, xmm6); // -a1 + a5 + (a5>>1)
	xmm4 = _mm_adds_epi16(xmm4, xmm7); //  a3 + a7 + (a7>>1)

	xmm7 = _mm_adds_epi16(xmm7, xmm0); //  a3 + a5 + a1 + (a1>>1) = a[7]
	xmm6 = _mm_subs_epi16(xmm6, xmm1); //  a1 + a7 - a3 - (a3>>1) = a[3]
	xmm2 = _mm_adds_epi16(xmm2, xmm3); //  a7 - a1 + a5 + (a5>>1) = a[5]
	xmm5 = _mm_subs_epi16(xmm5, xmm4); //  a5 - a3 - a7 - (a7>>1) = a[1]
	// 90
	// xmm2= a[5], xmm5= a[1], xmm6 = a[3], xmm7= a[7], src[32] = b[0], src[40] = b[2], src[48] = b[4], src[56] = b[6]
	xmm0 = xmm7; // a[7]
	xmm1 = xmm2; // a[5]

	xmm0 = _mm_srai_epi16(xmm0, 2); // a[7]>>2
	xmm1 = _mm_srai_epi16(xmm1, 2); // a[5]>>2
	xmm0 = _mm_adds_epi16(xmm0, xmm5); //  a[1] + (a[7]>>2) = b[1]
	xmm1 = _mm_adds_epi16(xmm1, xmm6); //  a[3] + (a[5]>>2) = b[3]

	xmm6 = _mm_srai_epi16(xmm6, 2); // a[3]>>2
	xmm5 = _mm_srai_epi16(xmm5, 2); // a[1]>>2
	xmm6 = _mm_subs_epi16(xmm6, xmm2); //  (a[3]>>2) - a[5] = b[5]
	xmm7 = _mm_subs_epi16(xmm7, xmm5); //  a[7] - (a[1]>>2) = b[7]
	// 100
	// xmm0 = b[1], xmm1 = b[3], xmm6 = b[5], xmm7= b[7], src[32] = b[0], src[40] = b[2], src[48] = b[4], src[56] = b[6]
	xmm2 = _mm_load_si128((__m128i*)&src[32]); // b[0]
	xmm3 = _mm_load_si128((__m128i*)&src[40]); // b[2]
	xmm4 = xmm2; // b[0]
	xmm5 = xmm3; // b[2]

	xmm2 = _mm_adds_epi16(xmm2, xmm7); //  b[0] + b[7]
	xmm4 = _mm_subs_epi16(xmm4, xmm7); //  b[0] - b[7] // h 0 1 2 3 4 5 6 7
	xmm3 = _mm_adds_epi16(xmm3, xmm6); //  b[2] + b[5]
	xmm5 = _mm_subs_epi16(xmm5, xmm6); //  b[2] - b[5]

	_mm_store_si128((__m128i*)&src[0 ], xmm2); // MM0
	_mm_store_si128((__m128i*)&src[8 ], xmm3); // MM1

	xmm6 = _mm_load_si128((__m128i*)&src[48]); // b[4]
	xmm7 = _mm_load_si128((__m128i*)&src[56]); // b[6]
	xmm2 = xmm6; // b[4]
	xmm3 = xmm7; // b[6]

	xmm2 = _mm_adds_epi16(xmm2, xmm1); //  b[4] + b[3]
	xmm6 = _mm_subs_epi16(xmm6, xmm1); //  b[4] - b[3] // f 0 1 2 3 4 5 6 7
	xmm3 = _mm_adds_epi16(xmm3, xmm0); //  b[6] + b[1]
	xmm7 = _mm_subs_epi16(xmm7, xmm0); //  b[6] - b[1]
	// 118
	// xmm2 = MM2, xmm3 = MM3, xmm4 = MM7, xmm5 = MM6, xmm6 = MM5, xmm7 = MM4, src[0] = MM0, src[8] = MM1
	xmm0 = xmm7; //e 0 1 2 3 4 5 6 7
	xmm1 = xmm5; //g 0 1 2 3 4 5 6 7

	xmm0 = _mm_unpacklo_epi16(xmm0,xmm6); // e0 f0 e1 f1 e2 f2 e3 f3
	xmm1 = _mm_unpacklo_epi16(xmm1,xmm4); // g0 h0 g1 h1 g2 h2 g3 h3
	xmm7 = _mm_unpackhi_epi16(xmm7,xmm6); // e4 f4 e5 f5 e6 f6 e7 f7
	xmm5 = _mm_unpackhi_epi16(xmm5,xmm4); // g4 h4 g5 h5 g6 h6 g7 h7

	xmm6 = xmm0; // e0 f0 e1 f1 e2 f2 e3 f3
	xmm4 = xmm7; // e4 f4 e5 f5 e6 f6 e7 f7

	xmm6 = _mm_unpacklo_epi32(xmm6,xmm1); // e0 f0 g0 h0 e1 f1 g1 h1
	xmm4 = _mm_unpacklo_epi32(xmm4,xmm5); // e4 f4 g4 h4 e5 f5 g5 h5
	xmm0 = _mm_unpackhi_epi32(xmm0,xmm1); // e2 f2 g2 h2 e3 f3 g3 h3
	xmm7 = _mm_unpackhi_epi32(xmm7,xmm5); // e6 f6 g6 h6 e7 f7 g7 h7
	// 130
	// xmm0 = efgh-2/3, xmm2 = MM2, xmm3 = MM3, xmm4 = efgh-4/5, xmm6 = efgh-0/1, xmm7 = efgh-6/7, src[0] = MM0, src[8] = MM1
	_mm_store_si128((__m128i*)&src[16], xmm4); // e4 f4 g4 h4 e5 f5 g5 h5
	_mm_store_si128((__m128i*)&src[24], xmm7); // e6 f6 g6 h6 e7 f7 g7 h7

	xmm1 = _mm_load_si128((__m128i*)&src[0 ]); // a 0 1 2 3 4 5 6 7
	xmm5 = _mm_load_si128((__m128i*)&src[8 ]); // b 0 1 2 3 4 5 6 7
	xmm4 = xmm1; // a 0 1 2 3 4 5 6 7
	xmm7 = xmm2; // c 0 1 2 3 4 5 6 7

	xmm1 = _mm_unpacklo_epi16(xmm1,xmm5); // a0 b0 a1 b1 a2 b2 a3 b3
	xmm2 = _mm_unpacklo_epi16(xmm2,xmm3); // c0 d0 c1 d1 c2 d2 c3 d3
	xmm4 = _mm_unpackhi_epi16(xmm4,xmm5); // a4 b4 a5 b5 a6 b6 a7 b7
	xmm7 = _mm_unpackhi_epi16(xmm7,xmm3); // c4 d4 c5 d5 c6 d6 c7 d7

	xmm5 = xmm1; // a0 b0 a1 b1 a2 b2 a3 b3
	xmm3 = xmm4; // a4 b4 a5 b5 a6 b6 a7 b7

	xmm1 = _mm_unpacklo_epi32(xmm1,xmm2); // a0 b0 c0 d0 a1 b1 c1 d1
	xmm4 = _mm_unpacklo_epi32(xmm4,xmm7); // a4 b4 c4 d4 a5 b5 c5 d5
	xmm5 = _mm_unpackhi_epi32(xmm5,xmm2); // a2 b2 c2 d2 a3 b3 c3 d3
	xmm3 = _mm_unpackhi_epi32(xmm3,xmm7); // a6 b6 c6 d6 a7 b7 c7 d7
	// 146
	// xmm0 = efgh-2/3, xmm1 = abcd-0/1, xmm3 = abcd-6/7, xmm4 = abcd-4/5, xmm5 = abcd-2/3, xmm6 = efgh-0/1, src[16] = efgh-4/5, src[24] = efgh-6/7
	xmm2 = xmm1; // a0 b0 c0 d0 a1 b1 c1 d1
	xmm7 = xmm5; // a2 b2 c2 d2 a3 b3 c3 d3

	xmm1 = _mm_unpacklo_epi64(xmm1,xmm6); // a0 b0 c0 d0 e0 f0 g0 h0 //mm[0] 
	xmm5 = _mm_unpacklo_epi64(xmm5,xmm0); // a2 b2 c2 d2 e2 f2 g2 h2 //mm[2]
	xmm2 = _mm_unpackhi_epi64(xmm2,xmm6); // a1 b1 c1 d1 e1 f1 g1 h1 
	xmm7 = _mm_unpackhi_epi64(xmm7,xmm0); // a3 b3 c3 d3 e3 f3 g3 h3

	_mm_store_si128((__m128i*)&src[32], xmm2); // a1 b1 c1 d1 e1 f1 g1 h1
	_mm_store_si128((__m128i*)&src[40], xmm7); // a3 b3 c3 d3 e3 f3 g3 h3

	xmm6 = _mm_load_si128((__m128i*)&src[16]); // e4 f4 g4 h4 e5 f5 g5 h5
	xmm0 = _mm_load_si128((__m128i*)&src[24]); // e6 f6 g6 h6 e7 f7 g7 h7
	xmm2 = xmm4; // a4 b4 c4 d4 a5 b5 c5 d5
	xmm7 = xmm3; // a6 b6 c6 d6 a7 b7 c7 d7

	xmm4 = _mm_unpacklo_epi64(xmm4,xmm6); // a4 b4 c4 d4 e4 f4 g4 h4
	xmm3 = _mm_unpacklo_epi64(xmm3,xmm0); // a6 b6 c6 d6 e6 f6 g6 h6
	xmm2 = _mm_unpackhi_epi64(xmm2,xmm6); // a5 b5 c5 d5 e5 f5 g5 h5
	xmm7 = _mm_unpackhi_epi64(xmm7,xmm0); // a7 b7 c7 d7 e7 f7 g7 h7
	// 162
	// xmm1 = mm[0], xmm2 = mm[5], xmm3 = mm[6], xmm4 = mm[4], xmm5 = mm[2], xmm7 = mm[7], src[32] = mm[1], src[40] = mm[3]
	// ********************* //
	xmm6 = xmm1; //mm[0]
	xmm0 = xmm5; //mm[2]

	xmm1 = _mm_adds_epi16(xmm1, xmm4); // mm[0] + mm[4] = a[0]
	xmm6 = _mm_subs_epi16(xmm6, xmm4); // mm[0] - mm[4] = a[4]
	xmm5 = _mm_srai_epi16(xmm5, 1); // mm[2]>>1
	xmm5 = _mm_subs_epi16(xmm5, xmm3); // (mm[2]>>1) - mm[6] = a[2]
	xmm3 = _mm_srai_epi16(xmm3, 1); // mm[6]>>1
	xmm0 = _mm_adds_epi16(xmm0, xmm3); // mm[2] + (mm[6]>>1) = a[6]
	// 170
	// xmm0 = a[6], xmm1 = a[0], xmm2 = mm[5], xmm5 = a[2], xmm6 = a[4], xmm7 = mm[7], src[32] = mm[1], src[40] = mm[3]
	xmm4 = xmm1; // a[0]
	xmm3 = xmm6; // a[4]

	xmm4 = _mm_adds_epi16(xmm4, xmm0); // a[0] + a[6] = b[0]
	xmm3 = _mm_adds_epi16(xmm3, xmm5); // a[4] + a[2] = b[2]
	xmm1 = _mm_subs_epi16(xmm1, xmm0); // a[0] - a[6] = b[6]
	xmm6 = _mm_subs_epi16(xmm6, xmm5); // a[4] - a[2] = b[4]

	_mm_store_si128((__m128i*)&src[0 ], xmm4); // b[0]
	_mm_store_si128((__m128i*)&src[8 ], xmm3); // b[2]
	_mm_store_si128((__m128i*)&src[16], xmm6); // b[4]
	_mm_store_si128((__m128i*)&src[24], xmm1); // b[6]

	xmm0 = _mm_load_si128((__m128i*)&src[32]); // mm[1]
	xmm5 = _mm_load_si128((__m128i*)&src[40]); // mm[3]
	// 182
	// xmm0 = mm[1], xmm2 = mm[5], xmm5 = mm[3], xmm7 = mm[7], src[0] = b[0], src[8] = b[2], src[16] = b[4], src[24] = b[6]
	xmm1 = xmm0; // a1
	xmm3 = xmm5; // a3
	xmm4 = xmm2; // a5
	xmm6 = xmm7; // a7

	xmm1 = _mm_srai_epi16(xmm1, 1); // a1>>1
	xmm3 = _mm_srai_epi16(xmm3, 1); // a3>>1
	xmm4 = _mm_srai_epi16(xmm4, 1); // a5>>1
	xmm6 = _mm_srai_epi16(xmm6, 1); // a7>>1

	xmm1 = _mm_adds_epi16(xmm1, xmm0); // a1 + (a1>>1)
	xmm3 = _mm_adds_epi16(xmm3, xmm5); // a3 + (a3>>1)
	xmm4 = _mm_adds_epi16(xmm4, xmm2); // a5 + (a5>>1)
	xmm6 = _mm_adds_epi16(xmm6, xmm7); // a7 + (a7>>1)

	xmm1 = _mm_adds_epi16(xmm1, xmm2); //  a5 + a1 + (a1>>1)
	xmm3 = _mm_subs_epi16(xmm3, xmm7); // -a7 + a3 + (a3>>1)
	xmm4 = _mm_subs_epi16(xmm4, xmm0); // -a1 + a5 + (a5>>1)
	xmm6 = _mm_adds_epi16(xmm6, xmm5); //  a3 + a7 + (a7>>1)

	xmm5 = _mm_adds_epi16(xmm5, xmm1); //  a3 + a5 + a1 + (a1>>1) = a[7]
	xmm0 = _mm_subs_epi16(xmm0, xmm3); //  a1 + a7 - a3 - (a3>>1) = a[3]
	xmm7 = _mm_adds_epi16(xmm7, xmm4); //  a7 - a1 + a5 + (a5>>1) = a[5]
	xmm2 = _mm_subs_epi16(xmm2, xmm6); //  a5 - a3 - a7 - (a7>>1) = a[1]
	// 202
	// xmm0 = a[3], xmm2 = a[1], xmm5 = a[7], xmm7 = a[5], src[0] = b[0], src[8] = b[2], src[16] = b[4], src[24] = b[6]
	xmm1 = xmm5; // a[7]
	xmm3 = xmm7; // a[5]

	xmm1 = _mm_srai_epi16(xmm1, 2); // a[7]>>2
	xmm3 = _mm_srai_epi16(xmm3, 2); // a[5]>>2
	xmm1 = _mm_adds_epi16(xmm1, xmm2); //  a[1] + (a[7]>>2) = b[1]
	xmm3 = _mm_adds_epi16(xmm3, xmm0); //  a[3] + (a[5]>>2) = b[3]

	xmm2 = _mm_srai_epi16(xmm2, 2); // a[1]>>2
	xmm0 = _mm_srai_epi16(xmm0, 2); // a[3]>>2
	xmm5 = _mm_subs_epi16(xmm5, xmm2); //  a[7] - (a[1]>>2) = b[7]
	xmm0 = _mm_subs_epi16(xmm0, xmm7); //  (a[3]>>2) - a[5] = b[5]
	// 212
	// xmm0 = b[5], xmm1 = b[1], xmm3 = b[3], xmm5 = b[7], src[0] = b[0], src[8] = b[2], src[16] = b[4], src[24] = b[6]
	xmm2 = _mm_load_si128((__m128i*)&src[0 ]); // b[0]
	xmm4 = _mm_load_si128((__m128i*)&src[8 ]); // b[2]
	xmm6 = xmm2; // b[0]
	xmm7 = xmm4; // b[2]

	xmm2 = _mm_adds_epi16(xmm2, xmm5); //  b[0] + b[7]
	xmm6 = _mm_subs_epi16(xmm6, xmm5); //  b[0] - b[7]
	xmm4 = _mm_adds_epi16(xmm4, xmm0); //  b[2] + b[5]
	xmm7 = _mm_subs_epi16(xmm7, xmm0); //  b[2] - b[5]
	// 220
	// xmm1 = b[1], xmm2 = ROW[0], xmm3 = b[3], xmm4 = ROW[1], xmm6 = ROW[7], xmm7 = ROW[6], src[16] = b[4], src[24] = b[6]
	xmm5 = _mm_load_si128((__m128i*)&const_32); // all 32s
	xmm0 = _mm_loadl_epi64 ((__m128i *) (pred+0  )); // pred[0]
	xmm2 = _mm_adds_epi16(xmm2, xmm5); //  b[0] + b[7] + 32
	xmm2 = _mm_srai_epi16(xmm2, 6); // (b[0] + b[7] + 32)>>6
	xmm6 = _mm_adds_epi16(xmm6, xmm5); //  b[0] - b[7] + 32
	xmm6 = _mm_srai_epi16(xmm6, 6); // (b[0] - b[7] + 32)>>6
	xmm4 = _mm_adds_epi16(xmm4, xmm5); //  b[2] + b[5] + 32
	xmm4 = _mm_srai_epi16(xmm4, 6); // (b[2] + b[5] + 32)>>6
	xmm7 = _mm_adds_epi16(xmm7, xmm5); //  b[2] - b[5] + 32
	xmm7 = _mm_srai_epi16(xmm7, 6); // (b[2] - b[5] + 32)>>6
	// 230
	// xmm0 = pred[0], xmm1 = b[1], xmm2 = ROW[0], xmm3 = b[3], xmm4 = ROW[1], xmm5 = "32", xmm6 = ROW[7], xmm7 = ROW[6],
	// src[16] = b[4], src[24] = b[6]
	xmm5 = _mm_setzero_si128(); // all 0s
	xmm0 = _mm_unpacklo_epi8(xmm0,xmm5);
	xmm2 = _mm_adds_epi16(xmm2, xmm0); //  (b[0] + b[7] + 32)>>6 + pred[0]
	xmm2 = _mm_packus_epi16(xmm2,xmm2);
	_mm_storel_epi64((__m128i*)&dest[0*stride], xmm2);

	xmm0 = _mm_loadl_epi64 ((__m128i *) (pred+16 )); // pred[1]
	xmm0 = _mm_unpacklo_epi8(xmm0,xmm5);
	xmm4 = _mm_adds_epi16(xmm4, xmm0); //  (b[2] + b[5] + 32)>>6 + pred[1]
	xmm4 = _mm_packus_epi16(xmm4,xmm4);
	_mm_storel_epi64((__m128i*)&dest[1*stride], xmm4);
	// 240
	// xmm1 = b[1], xmm3 = b[3], xmm5 = 0, xmm6 = ROW[7], xmm7 = ROW[6], src[16] = b[4], src[24] = b[6]
	xmm2 = _mm_load_si128((__m128i*)&src[16]); // b[4]
	xmm4 = xmm2; // b[4]
	xmm2 = _mm_adds_epi16(xmm2, xmm3); //  b[4] + b[3]
	xmm4 = _mm_subs_epi16(xmm4, xmm3); //  b[4] - b[3]
	xmm0 = _mm_load_si128((__m128i*)&const_32); // all 32s
	xmm2 = _mm_adds_epi16(xmm2, xmm0); //  b[4] + b[3] + 32
	xmm2 = _mm_srai_epi16(xmm2, 6); // (b[4] + b[3] + 32)>>6
	xmm4 = _mm_adds_epi16(xmm4, xmm0); //  b[4] - b[3] + 32
	xmm4 = _mm_srai_epi16(xmm4, 6); // (b[4] - b[3] + 32)>>6
	xmm3 = _mm_loadl_epi64 ((__m128i *) (pred+32  )); // pred[2]
	xmm3 = _mm_unpacklo_epi8(xmm3,xmm5);
	xmm2 = _mm_adds_epi16(xmm2, xmm3); //  (b[4] + b[3] + 32)>>6 + pred[2]
	xmm2 = _mm_packus_epi16(xmm2,xmm2);
	_mm_storel_epi64((__m128i*)&dest[2*stride], xmm2);
	// 254
	// xmm0 = "32", xmm1 = b[1], xmm4 = ROW[5], xmm5 = 0, xmm6 = ROW[7], xmm7 = ROW[6], src[24] = b[6]
	xmm3 = _mm_load_si128((__m128i*)&src[24]); // b[6]
	xmm2 = xmm3; // b[6]
	xmm2 = _mm_adds_epi16(xmm2, xmm1); //  b[6] + b[1]
	xmm3 = _mm_subs_epi16(xmm3, xmm1); //  b[6] - b[1]
	xmm2 = _mm_adds_epi16(xmm2, xmm0); //  b[6] + b[1] + 32
	xmm2 = _mm_srai_epi16(xmm2, 6); // (b[6] + b[1] + 32)>>6
	xmm3 = _mm_adds_epi16(xmm3, xmm0); //  b[6] - b[1] + 32
	xmm3 = _mm_srai_epi16(xmm3, 6); // (b[6] - b[1] + 32)>>6
	xmm1 = _mm_loadl_epi64 ((__m128i *) (pred+48  )); // pred[3]
	xmm1 = _mm_unpacklo_epi8(xmm1,xmm5);
	xmm2 = _mm_adds_epi16(xmm2, xmm1); //  (b[6] + b[1] + 32)>>6 + pred[3]
	xmm2 = _mm_packus_epi16(xmm2,xmm2);
	_mm_storel_epi64((__m128i*)&dest[3*stride], xmm2);
	// 267
	// xmm0 = "32", xmm3 = ROW[4], xmm4 = ROW[5], xmm5 = 0, xmm6 = ROW[7], xmm7 = ROW[6], src[24] = b[6]
	xmm1 = _mm_loadl_epi64 ((__m128i *) (pred+64  )); // pred[4]
	xmm1 = _mm_unpacklo_epi8(xmm1,xmm5);
	xmm3 = _mm_adds_epi16(xmm3, xmm1); //  (b[6] - b[1] + 32)>>6 + pred[4]
	xmm3 = _mm_packus_epi16(xmm3,xmm3);
	_mm_storel_epi64((__m128i*)&dest[4*stride], xmm3);

	xmm1 = _mm_loadl_epi64 ((__m128i *) (pred+80  )); // pred[5]
	xmm1 = _mm_unpacklo_epi8(xmm1,xmm5);
	xmm4 = _mm_adds_epi16(xmm4, xmm1); //  (b[4] - b[3] + 32)>>6 + pred[5]
	xmm4 = _mm_packus_epi16(xmm4,xmm4);
	_mm_storel_epi64((__m128i*)&dest[5*stride], xmm4);

	xmm1 = _mm_loadl_epi64 ((__m128i *) (pred+96  )); // pred[6]
	xmm1 = _mm_unpacklo_epi8(xmm1,xmm5);
	xmm7 = _mm_adds_epi16(xmm7, xmm1); //  (b[2] - b[5] + 32)>>6 + pred[6]
	xmm7 = _mm_packus_epi16(xmm7,xmm7);
	_mm_storel_epi64((__m128i*)&dest[6*stride], xmm7);

	xmm1 = _mm_loadl_epi64 ((__m128i *) (pred+112 )); // pred[7]
	xmm1 = _mm_unpacklo_epi8(xmm1,xmm5);
	xmm6 = _mm_adds_epi16(xmm6, xmm1); //  (b[0] - b[7] + 32)>>6 + pred[7]
	xmm6 = _mm_packus_epi16(xmm6,xmm6);
	_mm_storel_epi64((__m128i*)&dest[7*stride], xmm6);

	__m64 mm7 = _mm_setzero_si64();
	*(__m64*) (&src[0]) = mm7;
	*(__m64*) (&src[4]) = mm7;
	*(__m64*) (&src[8]) = mm7;
	*(__m64*) (&src[12]) = mm7;
	*(__m64*) (&src[16]) = mm7;
	*(__m64*) (&src[20]) = mm7;
	*(__m64*) (&src[24]) = mm7;
	*(__m64*) (&src[28]) = mm7;
	*(__m64*) (&src[32]) = mm7;
	*(__m64*) (&src[36]) = mm7;
	*(__m64*) (&src[40]) = mm7;
	*(__m64*) (&src[44]) = mm7;
	*(__m64*) (&src[48]) = mm7;
	*(__m64*) (&src[52]) = mm7;
	*(__m64*) (&src[56]) = mm7;
	*(__m64*) (&src[60]) = mm7;
}
#endif 
#endif //H264_ENABLE_INTRINSICS
