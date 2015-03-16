/*!
*************************************************************************
* \file mb_chroma_mmx.cpp
*
* \brief
*    Bilinear interpolation function used for chroma Motion Compensation
*
* \coding
*    MMX/SSE/SSE2/SSE3
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
#ifdef H264_ENABLE_INTRINSICS
#include <xmmintrin.h>
#include <emmintrin.h>

#include "mbuffer.h"
#include "mb_chroma.h"
#endif

#ifdef H264_ENABLE_ASM
// Disable "No EMMS at end of function '<function name>'"
#pragma warning ( disable : 4799 )
#pragma warning ( disable : 997 )

// Used for mmx routines
static __int64 mul_tab_base[10] = {	0x0000000800000008,
0x0001000700010007,
0x0002000600020006,
0x0003000500030005,
0x0004000400040004,
0x0005000300050003,
0x0006000200060002,
0x0007000100070001,
0x0008000000080000,
0x0020002000200020};

void mb_chroma_2xH_pred_mmx PARGS8( StorablePicture *p,
																	 imgpel *pDstUV,							      
																	 int stride_dst,
																	 int base_x_pos,
																	 int base_y_pos,
																	 int mv_x,
																	 int mv_y,
																	 int H)
{
	int if1 = mv_x&7;
	int jf1 = mv_y&7;
	int ii0 = base_x_pos + (mv_x>>3);
	int jj0 = base_y_pos + (mv_y>>3);
	int stride_src = p->UV_stride;
	int count;

	imgpel *pSrcUV;

	static unsigned short const_val[9][4] = { { 0, 0, 0, 0 },
	{ 1, 1, 1, 1 },
	{ 2, 2, 2, 2 },
	{ 3, 3, 3, 3 },
	{ 4, 4, 4, 4 },
	{ 5, 5, 5, 5 },
	{ 6, 6, 6, 6 },
	{ 7, 7, 7, 7 },
	{32,32,32,32 }
	};

	static int *t_ptr = (int *) const_val;

	ii0 = __fast_iclip(clip_min_x_cr,clip_max_x_cr,ii0);

	pSrcUV = p->imgUV +(ii0<<1);

	jj0 = clip_vertical_c ARGS5(jj0, pSrcUV, stride_src, 2, H);

	pSrcUV += jj0*stride_src;

	// detect the case of motion vector(0,0)
	if(if1==0)
	{
		if(jf1==0)
		{
			for(count=0;count<H;count++)
			{
				*((long*)(pDstUV)) = *((long*)(pSrcUV));
				pSrcUV += stride_src;
				pDstUV += stride_dst;				
			}
		}
		else
		{
			__asm{
				mov       esi, pSrcUV;		// chroma U source pointer
				mov       edi, pDstUV;		// chroma U destination pointer
				mov       eax, t_ptr;		// weight table
				mov       ebx, stride_src;	// the line offset of source points
				mov       ecx, stride_dst;	// the line offset of destination points
				movq      mm4, [eax+32];	// Load const_val[4] value, mm4 = (4, 4, 4, 4)
				mov       edx, jf1;			// jf1 = mv_y = y
				movq      mm5, [eax+edx*8];	// Load vertical weight values, mm5 = (mv_y, mv_y, mv_y, mv_y)
				mov		  edx, H;			// edx = H
				dec		  edx;				// edx = H--, only for U.
				pxor      mm7, mm7;			// Zero mm7

				movq      mm0, [esi];		// Load 1st line, (A, B, C, D, E, ?, ?, ?), mm0 = (? ? ? E D C B A)
				movq      mm2, mm0;			// mm2 = (? ? ? E D C B A)
				punpcklbw mm0, mm7;			// mm0 = (0 D 0 C 0 B 0 A) = line1 result, (D', C', B', A')

				lea	      esi, [esi+ebx];	// Load effective source address for next line

X0_4x2:
				movq      mm1, [esi];		// Load 2nd line, (F, G, H, I, J, ?, ?, ?), mm1 = (? ? ? J I H G F)
				movq      mm3, mm1;			// mm3 = (? ? ? J I H G F)
				punpcklbw mm1, mm7;			// mm1 = (0 I 0 H 0 G 0 F) = line2 result, (I', H', G', F')

				movq      mm2, mm1;			// Backup line2 result, mm2 = mm1
				psubsw	  mm1, mm0;			// Vertical: line2 - line1, mm1 = (I'-D', H'-C', G'-B', F'-A')
				psllw	  mm0, 3;			// Vertical: 8 * line1, mm0 = 8 * (D' C' B' A')
				pmullw    mm1, mm5;			// Vertical: (line2 - line1) * mv_y, mm1 = (I'-D', H'-C', G'-B', F'-A') * mv_y
				paddsw	  mm0, mm1;			// mm0 = final 1st line result without +4/8 for each point
				paddsw    mm0, mm4;			// +4 for final 1st line result
				psraw     mm0, 3;			// /8 for final 1st line result
				packuswb  mm0, mm7;			// signed WORD to unsigned BYTE
				movd      [edi], mm0;		// write the 1st horizontal 4 points

				movq      mm0, [esi+ebx];	// Load 3rd line, (K, L, M, N, O, ?, ?, ?), mm0 = (? ? ? O N M L K)
				movq      mm1, mm0;			// mm1 = (? ? ? O N M L K)
				punpcklbw mm0, mm7;			// mm0 = (0 N 0 M 0 L 0 K) = line3 result, (N', M', L', K')

				movq      mm3, mm0;			// Backup line3 result, mm3 = mm0
				psubsw	  mm0, mm2;			// Vertical: line3 - line2, mm0 = (N'-I', M'-H', L'-G', K'-F')
				psllw	  mm2, 3;			// Vertical: 8 * line2, mm2 = 8 * (I', H', G', F')
				pmullw    mm0, mm5;			// Vertical: (line3 - line2) * mv_y, mm0 = (N'-I', M'-H', L'-G', K'-F') * mv_y
				paddsw	  mm2, mm0;			// mm2 = final 2nd line result without +4/8 for each point
				paddsw    mm2, mm4;			// +4 for final 2nd line result
				psraw     mm2, 3;			// /8 for final 2nd line result
				packuswb  mm2, mm7;			// signed WORD to unsigned BYTE
				movd      [edi+ecx], mm2;	// write the 2nd horizontal 4 points

				sub		  edx, 2;			// Loop 4x2 part
				jle		  X0_Out;				// H - 2 <= 0, U (<0) or V (=0) finish.

				movq	  mm0, mm3;			// write back the backup line
				lea	      esi, [esi+2*ebx];	// Load effective source address for next line
				lea	      edi, [edi+2*ecx];	// Load effective destination address for next line
				jmp	      X0_4x2;			// If not zero then goto Loop4x2
X0_Out:
			}
		}
	}
	else
	{
		if(jf1==0)
		{
			__asm{
				mov       esi, pSrcUV;		// chroma U source pointer
				mov       edi, pDstUV;		// chroma U destination pointer
				mov       eax, t_ptr;		// weight table
				mov       ebx, stride_src;	// the line offset of source points
				mov       ecx, stride_dst;	// the line offset of destination points
				movq      mm4, [eax+32];	// Load const_val[4] value, mm4 = (32, 32, 32, 32)
				mov       edx, if1;			// if1 = mv_x = x
				movq      mm6, [eax+edx*8];	// Load horizontal weight values, mm6 = (mv_x, mv_x, mv_x, mv_x)
				mov		  edx, H;			// edx = H
				dec		  edx;				// edx = H--, only for U.
				pxor      mm7, mm7;			// Zero mm7

Y0_4x2:
				movq      mm0, [esi];		// Load 1st line, (A, B, C, D, E, ?, ?, ?), mm0 = (? ? ? E D C B A)
				movq      mm2, mm0;			// mm2 = (? ? ? E D C B A)
				punpcklbw mm0, mm7;			// mm0 = (0 D 0 C 0 B 0 A), line1 part 1
				psrlq     mm2, 16;			// mm2 = (0 ? ? ? E D C B)
				punpcklbw mm2, mm7;			// mm2 = (0 E 0 D 0 C 0 B), line1 part 2
				psubsw	  mm2, mm0;			// Line1: part2 - part1, mm2 = (E-D, D-C, C-B, B-A)
				psllw	  mm0, 3;			// Line1: 8 * part1, mm0 = 8 * (D, C, B, A)
				pmullw	  mm2, mm6;			// Line1: (part2 - part1) * mv_x, mm2 = (E-D, D-C, C-B, B-A) * mv_x
				paddsw	  mm0, mm2;			// Line1: the result, 8 * part1 + (part2 - part1) * mv_x, mm0 = (D', C', B', A')

				paddsw    mm0, mm4;			// +4 for final 1st line result
				psraw     mm0, 3;			// /8 for final 1st line result
				packuswb  mm0, mm7;			// signed WORD to unsigned BYTE
				movd      [edi], mm0;		// write the 1st horizontal 4 points


				movq      mm1, [esi+ebx];	// Load 2nd line, (F, G, H, I, J, ?, ?, ?), mm1 = (? ? ? J I H G F)
				movq      mm3, mm1;			// mm3 = (? ? ? J I H G F)
				punpcklbw mm1, mm7;			// mm1 = (0 I 0 H 0 G 0 F), line2 part 1
				psrlq     mm3, 16;			// mm3 = (0 ? ? ? J I H G)
				punpcklbw mm3, mm7;			// mm3 = (0 J 0 I 0 H 0 G), line2 part 2
				psubsw	  mm3, mm1;			// Line2: part2 - part1, mm3 = (J-I, I-H, H-G, G-F)
				psllw	  mm1, 3;			// Line2: 8 * part1, mm1 = 8 * (I, H, G, F)
				pmullw	  mm3, mm6;			// Line2: (part2 - part1) * mv_x, mm3 = (J-I, I-H, H-G, G-F) * mv_x
				paddsw	  mm1, mm3;			// Line2: the result, 8 * part1 + (part2 - part1) * mv_x, mm1 = (I', H', G', F')

				paddsw    mm1, mm4;			// +4 for final 1st line result
				psraw     mm1, 3;			// /8 for final 1st line result
				packuswb  mm1, mm7;			// signed WORD to unsigned BYTE
				movd      [edi+ecx], mm1;	// write the 1st horizontal 4 points

				sub		  edx, 2;			// Loop 4x2 part
				jle		  Y0_Out;				// H - 2 <= 0, U (<0) or V (=0) finish.

				lea	      esi, [esi+2*ebx];	// Load effective source address for next line
				lea	      edi, [edi+2*ecx];	// Load effective destination address for next line
				jmp	      Y0_4x2;			// If not zero then goto Loop4x2
Y0_Out:
			}

		}
		else
		{
			__asm{
				mov       esi, pSrcUV;		// chroma U source pointer
				mov       edi, pDstUV;		// chroma U destination pointer
				mov       eax, t_ptr;		// weight table
				mov       ebx, stride_src;	// the line offset of source points
				mov       ecx, stride_dst;	// the line offset of destination points
				movq      mm4, [eax+64];	// Load const_val[4] value, mm4 = (32, 32, 32, 32)
				mov       edx, if1;			// if1 = mv_x = x
				movq      mm6, [eax+edx*8];	// Load horizontal weight values, mm6 = (mv_x, mv_x, mv_x, mv_x)
				mov       edx, jf1;			// jf1 = mv_y = y
				movq      mm5, [eax+edx*8];	// Load vertical weight values, mm5 = (mv_y, mv_y, mv_y, mv_y)
				mov		  edx, H;			// edx = H
				dec		  edx;				// edx = H--, only for U.
				pxor      mm7, mm7;			// Zero mm7

				movq      mm0, [esi];		// Load 1st line, (A, B, C, D, E, ?, ?, ?), mm0 = (? ? ? E D C B A)
				movq      mm2, mm0;			// mm2 = (? ? ? E D C B A)
				punpcklbw mm0, mm7;			// mm0 = (0 D 0 C 0 B 0 A), line1 part 1
				psrlq     mm2, 16;			// mm2 = (0 ? ? ? E D C B)
				punpcklbw mm2, mm7;			// mm2 = (0 E 0 D 0 C 0 B), line1 part 2
				psubsw	  mm2, mm0;			// Line1: part2 - part1, mm2 = (E-D, D-C, C-B, B-A)
				psllw	  mm0, 3;			// Line1: 8 * part1, mm0 = 8 * (D, C, B, A)
				pmullw	  mm2, mm6;			// Line1: (part2 - part1) * mv_x, mm2 = (E-D, D-C, C-B, B-A) * mv_x
				paddsw	  mm0, mm2;			// Line1: the result, 8 * part1 + (part2 - part1) * mv_x, mm0 = (D', C', B', A')

				lea	      esi, [esi+ebx];	// Load effective source address for next line

Loop4x2:
				movq      mm1, [esi];		// Load 2nd line, (F, G, H, I, J, ?, ?, ?), mm1 = (? ? ? J I H G F)
				movq      mm3, mm1;			// mm3 = (? ? ? J I H G F)
				punpcklbw mm1, mm7;			// mm1 = (0 I 0 H 0 G 0 F), line2 part 1
				psrlq     mm3, 16;			// mm3 = (0 ? ? ? J I H G)
				punpcklbw mm3, mm7;			// mm3 = (0 J 0 I 0 H 0 G), line2 part 2
				psubsw	  mm3, mm1;			// Line2: part2 - part1, mm3 = (J-I, I-H, H-G, G-F)
				psllw	  mm1, 3;			// Line2: 8 * part1, mm1 = 8 * (I, H, G, F)
				pmullw	  mm3, mm6;			// Line2: (part2 - part1) * mv_x, mm3 = (J-I, I-H, H-G, G-F) * mv_x
				paddsw	  mm1, mm3;			// Line2: the result, 8 * part1 + (part2 - part1) * mv_x, mm1 = (I', H', G', F')

				movq      mm2, mm1;			// Backup line2 result, mm2 = mm1
				psubsw	  mm1, mm0;			// Vertical: line2 - line1, mm1 = (I'-D', H'-C', G'-B', F'-A')
				psllw	  mm0, 3;			// Vertical: 8 * line1, mm0 = 8 * (D' C' B' A')
				pmullw    mm1, mm5;			// Vertical: (line2 - line1) * mv_y, mm1 = (I'-D', H'-C', G'-B', F'-A') * mv_y
				paddsw	  mm0, mm1;			// mm0 = final 1st line result without +32/64 for each point
				paddsw    mm0, mm4;			// +32 for final 1st line result
				psraw     mm0, 6;			// /64 for final 1st line result
				packuswb  mm0, mm7;			// signed WORD to unsigned BYTE
				movd      [edi], mm0;		// write the 1st horizontal 4 points

				movq      mm0, [esi+ebx];	// Load 3rd line, (K, L, M, N, O, ?, ?, ?), mm0 = (? ? ? O N M L K)
				movq      mm1, mm0;			// mm1 = (? ? ? O N M L K)
				punpcklbw mm0, mm7;			// mm0 = (0 N 0 M 0 L 0 K), line3 part1
				psrlq     mm1, 16;			// mm1 = (0 ? ? ? O N M L)
				punpcklbw mm1, mm7;			// mm1 = (0 O 0 N 0 M 0 L), line3 part2
				psubsw	  mm1, mm0;			// Line3: part2 - part1, mm1 = (O-N, N-M, M-L, L-K)
				psllw	  mm0, 3;			// Line3: 8 * part1, mm0 = 8 * (N, M, L, K)
				pmullw	  mm1, mm6;			// Line3: (part2 - part1) * mv_x, mm1 = (O-N, N-M, M-L, L-K) * mv_x
				paddsw	  mm0, mm1;			// Line3: the result, 8 * part1 + (part2 - part1) * mv_x, mm0 = (N', M', L', K')

				movq      mm3, mm0;			// Backup line3 result, mm3 = mm0
				psubsw	  mm0, mm2;			// Vertical: line3 - line2, mm0 = (N'-I', M'-H', L'-G', K'-F')
				psllw	  mm2, 3;			// Vertical: 8 * line2, mm2 = 8 * (I', H', G', F')
				pmullw    mm0, mm5;			// Vertical: (line3 - line2) * mv_y, mm0 = (N'-I', M'-H', L'-G', K'-F') * mv_y
				paddsw	  mm2, mm0;			// mm2 = final 2nd line result without +32/64 for each point
				paddsw    mm2, mm4;			// +32 for final 2nd line result
				psraw     mm2, 6;			// /64 for final 2nd line result
				packuswb  mm2, mm7;			// signed WORD to unsigned BYTE
				movd      [edi+ecx], mm2;	// write the 2nd horizontal 4 points

				sub		  edx, 2;			// Loop 4x2 part
				jle		  Exit;			// H - 2 <= 0, U (<0) or V (=0) finish.

				movq	  mm0, mm3;			// write back the backup line
				lea	      esi, [esi+2*ebx];	// Load effective source address for next line
				lea	      edi, [edi+2*ecx];	// Load effective destination address for next line
				jmp	      Loop4x2;			// If not zero then goto Loop4x2
Exit:
			}

		}
	}
}

void mb_chroma_4xH_pred_mmx PARGS8( StorablePicture *p,
																	 imgpel *pDstUV,
																	 int stride_dst,
																	 int base_x_pos,
																	 int base_y_pos,
																	 int mv_x,
																	 int mv_y,
																	 int H)
{
	int if1 = mv_x&7;
	int jf1 = mv_y&7;
	int ii0 = base_x_pos + (mv_x>>3);
	int jj0 = base_y_pos + (mv_y>>3);
	int stride_src = p->UV_stride;
	imgpel *pSrcUV;

	int count;
	int number=0;
	__m64   mm0, mm1, mm2, mm3, mm4, mm5, mm6, mm7;
	static unsigned short __declspec(align(16)) const_val[8][8] = { {32,32,32,32,32,32,32,32 },
	{ 1, 1, 1, 1, 1, 1, 1, 1 },
	{ 2, 2, 2, 2, 2, 2, 2, 2 },
	{ 3, 3, 3, 3, 3, 3, 3, 3 },
	{ 4, 4, 4, 4, 4, 4, 4, 4 },
	{ 5, 5, 5, 5, 5, 5, 5, 5 },
	{ 6, 6, 6, 6, 6, 6, 6, 6 },
	{ 7, 7, 7, 7, 7, 7, 7, 7 }
	};

	ii0 = __fast_iclip(clip_min_x_cr,clip_max_x_cr,ii0);

	pSrcUV = p->imgUV+(ii0<<1);

	jj0 = clip_vertical_c ARGS5(jj0, pSrcUV, stride_src, 8, H);

	pSrcUV += jj0*stride_src;

	// detect the case of motion vector(0,0)  
	if(if1==0)
	{
		if(jf1==0)
		{
			for(count=H;count>0;count-=4)
			{
				mm0 = *(__m64*) &pSrcUV[0*stride_src];
				mm1 = *(__m64*) &pSrcUV[1*stride_src];
				mm2 = *(__m64*) &pSrcUV[2*stride_src];
				mm3 = *(__m64*) &pSrcUV[3*stride_src];
				*(__m64*) &pDstUV[0*stride_dst] = mm0;
				*(__m64*) &pDstUV[1*stride_dst] = mm1;
				*(__m64*) &pDstUV[2*stride_dst] = mm2;
				*(__m64*) &pDstUV[3*stride_dst] = mm3;	    
				pSrcUV += 4*stride_src;
				pDstUV += 4*stride_dst;
				// Process 4 lines at a time
			}
		}
		else
		{
			mm4 = _mm_setzero_si64();
			mm5 = *((__m64*)&const_val[4][0]);
			mm6 = *((__m64*)&const_val[jf1][0]);
		
			//  1
			mm1  = *((__m64*) &pSrcUV[0]);
			mm0 = _mm_unpacklo_pi8(mm1, mm4); // aU

			for(count=H;count>0;count-=2)
			{
				number++;
				mm2  = *((__m64*) &pSrcUV[stride_src]);
				mm2 = _mm_unpacklo_pi8(mm2, mm4); // cU
				mm1 = mm2;                          // cU - need to save
				mm2 = _mm_sub_pi16(mm2, mm0);    // cU-aU
				mm0 = _mm_slli_pi16(mm0, 3);       // 8*aU
				mm2 = _mm_mullo_pi16(mm2, mm6);   // mv_y*(cU-aU)
				mm0 = _mm_add_pi16(mm0, mm5);    // 8*aU + 4
				mm0 = _mm_add_pi16(mm0, mm2);    // 8*aU + mv_y*(cU-aU) + 4
				mm0 = _mm_srai_pi16(mm0, 3);       // (8*aU + mv_y*(cU-aU) + 4)>>3
				mm0 = _mm_packs_pu16(mm0, mm0);  // Words -> bytes
				*((DWORD*)&pDstUV[0]) = _m_to_int(mm0);		

				mm2  = *((__m64*) &pSrcUV[2*stride_src]);
				mm2 = _mm_unpacklo_pi8(mm2, mm4); // cU
				mm0 = mm2;                          // cU - need to save
				mm2 = _mm_sub_pi16(mm2, mm1);    // cU-aU
				mm1 = _mm_slli_pi16(mm1, 3);       // 8*aU
				mm2 = _mm_mullo_pi16(mm2, mm6);   // mv_y*(cU-aU)
				mm1 = _mm_add_pi16(mm1, mm5);    // 8*aU + 4
				mm1 = _mm_add_pi16(mm1, mm2);    // 8*aU + mv_y*(cU-aU) + 4
				mm1 = _mm_srai_pi16(mm1, 3);       // (8*aU + mv_y*(cU-aU) + 4)>>3
				mm1 = _mm_packs_pu16(mm1, mm1);  // Words -> bytes
				*((DWORD*)&pDstUV[stride_dst]) = _m_to_int(mm1);		

				pSrcUV += 2*stride_src;
				pDstUV += 2*stride_dst;
			}
			pSrcUV -= 2*stride_src*number;
			pDstUV -= 2*stride_dst*number;
			number=0;
			
			//  2
			mm1  = *((__m64*) &pSrcUV[0]);
			mm0 = _mm_unpackhi_pi8(mm1, mm4); // aU
			
			for(count=H;count>0;count-=2)
			{
				mm2  = *((__m64*) &pSrcUV[stride_src]);
				mm2 = _mm_unpackhi_pi8(mm2, mm4); // cU
				mm1 = mm2;                          // cU - need to save
				mm2 = _mm_sub_pi16(mm2, mm0);    // cU-aU
				mm0 = _mm_slli_pi16(mm0, 3);       // 8*aU
				mm2 = _mm_mullo_pi16(mm2, mm6);   // mv_y*(cU-aU)
				mm0 = _mm_add_pi16(mm0, mm5);    // 8*aU + 4
				mm0 = _mm_add_pi16(mm0, mm2);    // 8*aU + mv_y*(cU-aU) + 4
				mm0 = _mm_srai_pi16(mm0, 3);       // (8*aU + mv_y*(cU-aU) + 4)>>3
				mm0 = _mm_packs_pu16(mm0, mm0);  // Words -> bytes
				*((DWORD*)&pDstUV[4]) = _m_to_int(mm0);
				
				mm2  = *((__m64*) &pSrcUV[2*stride_src]);
				mm2 = _mm_unpackhi_pi8(mm2, mm4); // cU
				mm0 = mm2;                          // cU - need to save
				mm2 = _mm_sub_pi16(mm2, mm1);    // cU-aU
				mm1 = _mm_slli_pi16(mm1, 3);       // 8*aU
				mm2 = _mm_mullo_pi16(mm2, mm6);   // mv_y*(cU-aU)
				mm1 = _mm_add_pi16(mm1, mm5);    // 8*aU + 4
				mm1 = _mm_add_pi16(mm1, mm2);    // 8*aU + mv_y*(cU-aU) + 4
				mm1 = _mm_srai_pi16(mm1, 3);       // (8*aU + mv_y*(cU-aU) + 4)>>3
				mm1 = _mm_packs_pu16(mm1, mm1);  // Words -> bytes
				*((DWORD*)&pDstUV[stride_dst+4]) = _m_to_int(mm1);
			
				pSrcUV += 2*stride_src;
				pDstUV += 2*stride_dst;
			}
			
		}
	}
	else
	{
		if(jf1==0)
		{
			mm4 = _mm_setzero_si64();
			mm5 = *((__m64*)&const_val[4][0]);
			mm7 = *((__m64*)&const_val[if1][0]);

			for(count=H;count>0;count-=2)
			{
				// 1
				mm2 = *((__m64*)&pSrcUV[0]);
				mm3 = *((__m64*)&pSrcUV[2]);
				mm2 = _mm_unpacklo_pi8(mm2, mm4); // cU
				mm3 = _mm_unpacklo_pi8(mm3, mm4); // dU
				mm3 = _mm_sub_pi16(mm3, mm2);    // dU-cU
				mm2 = _mm_slli_pi16(mm2, 3);       // 8*cU
				mm3 = _mm_mullo_pi16(mm3, mm7);   // mv_x*(dU-cU)
				mm2 = _mm_add_pi16(mm2, mm5);    // 8*cU + 4
				mm2 = _mm_add_pi16(mm2, mm3);    // 8*cU + mv_x*(dU-cU) + 4
				mm2 = _mm_srai_pi16(mm2, 3);       // (8*cU + mv_x*(dU-cU) + 4)>>3
				mm2 = _mm_packs_pu16(mm2, mm2);  // Words -> bytes
				*((DWORD*)&pDstUV[0]) = _m_to_int(mm2);

				mm2 = *((__m64*)&pSrcUV[stride_src]);
				mm3 = *((__m64*)&pSrcUV[stride_src+2]);
				mm2 = _mm_unpacklo_pi8(mm2, mm4); // cU
				mm3 = _mm_unpacklo_pi8(mm3, mm4); // dU
				mm3 = _mm_sub_pi16(mm3, mm2);    // dU-cU
				mm2 = _mm_slli_pi16(mm2, 3);       // 8*cU
				mm3 = _mm_mullo_pi16(mm3, mm7);   // mv_x*(dU-cU)
				mm2 = _mm_add_pi16(mm2, mm5);    // 8*cU + 4
				mm2 = _mm_add_pi16(mm2, mm3);    // 8*cU + mv_x*(dU-cU) + 4
				mm2 = _mm_srai_pi16(mm2, 3);       // (8*cU + mv_x*(dU-cU) + 4)>>3
				mm2 = _mm_packs_pu16(mm2, mm2);  // Words -> bytes
				*((DWORD*)&pDstUV[stride_dst]) = _m_to_int(mm2);   
				
				//  2
				mm2 = *((__m64*)&pSrcUV[0]);
				mm3 = *((__m64*)&pSrcUV[2]);
				mm2 = _mm_unpackhi_pi8(mm2, mm4); // cU
				mm3 = _mm_unpackhi_pi8(mm3, mm4); // dU
				mm3 = _mm_sub_pi16(mm3, mm2);    // dU-cU
				mm2 = _mm_slli_pi16(mm2, 3);       // 8*cU
				mm3 = _mm_mullo_pi16(mm3, mm7);   // mv_x*(dU-cU)
				mm2 = _mm_add_pi16(mm2, mm5);    // 8*cU + 4
				mm2 = _mm_add_pi16(mm2, mm3);    // 8*cU + mv_x*(dU-cU) + 4
				mm2 = _mm_srai_pi16(mm2, 3);       // (8*cU + mv_x*(dU-cU) + 4)>>3
				mm2 = _mm_packs_pu16(mm2, mm2);  // Words -> bytes
				*((DWORD*)&pDstUV[4]) = _m_to_int(mm2);
				
				mm2 = *((__m64*)&pSrcUV[stride_src]);
				mm3 = *((__m64*)&pSrcUV[stride_src+2]);
				mm2 = _mm_unpackhi_pi8(mm2, mm4); // cU
				mm3 = _mm_unpackhi_pi8(mm3, mm4); // dU
				mm3 = _mm_sub_pi16(mm3, mm2);    // dU-cU
				mm2 = _mm_slli_pi16(mm2, 3);       // 8*cU
				mm3 = _mm_mullo_pi16(mm3, mm7);   // mv_x*(dU-cU)
				mm2 = _mm_add_pi16(mm2, mm5);    // 8*cU + 4
				mm2 = _mm_add_pi16(mm2, mm3);    // 8*cU + mv_x*(dU-cU) + 4
				mm2 = _mm_srai_pi16(mm2, 3);       // (8*cU + mv_x*(dU-cU) + 4)>>3
				mm2 = _mm_packs_pu16(mm2, mm2);  // Words -> bytes
				*((DWORD*)&pDstUV[stride_dst+4]) = _m_to_int(mm2);

				pSrcUV  += 2*stride_src;
				pDstUV  += 2*stride_dst;
			}
		}
		else
		{
			mm4 = _mm_setzero_si64();
			mm5 = *((__m64*)&const_val[0][0]);
			mm6 = *((__m64*)&const_val[jf1][0]);
			mm7 = *((__m64*)&const_val[if1][0]);
			
			
			mm0 = *((__m64*)&pSrcUV[0]);
			mm1 = *((__m64*)&pSrcUV[2]);
			mm0 = _mm_unpacklo_pi8(mm0, mm4); // aU
			mm1 = _mm_unpacklo_pi8(mm1, mm4); // bU
			mm1 = _mm_sub_pi16(mm1, mm0);    // bU-aU
			mm0 = _mm_slli_pi16(mm0, 3);       // 8*aU
			mm1 = _mm_mullo_pi16(mm1, mm7);   // mv_x*(bU-aU)
			mm0 = _mm_add_pi16(mm0, mm1);    // 8*aU+mv_x*(bU-aU) = ABU

			for(count=H;count>0;count-=2)
			{
				number++;
				mm1 = *((__m64*)&pSrcUV[stride_src]);
				mm3 = *((__m64*)&pSrcUV[stride_src+2]);
				mm1 = _mm_unpacklo_pi8(mm1, mm4); // cU
				mm3 = _mm_unpacklo_pi8(mm3, mm4); // dU
				mm3 = _mm_sub_pi16(mm3, mm1);    // dU-cU
				mm1 = _mm_slli_pi16(mm1, 3);       // 8*cU
				mm3 = _mm_mullo_pi16(mm3, mm7);   // mv_x*(dU-cU)
				mm1 = _mm_add_pi16(mm1, mm3);    // 8*cU+mv_x*(dU-cU) = CDU
				mm3 = mm1;                          // 8*cU+mv_x*(dU-cU) = CDU - need to save
				mm1 = _mm_sub_pi16(mm1, mm0);    // CDU-ABU
				mm0 = _mm_slli_pi16(mm0, 3);       // 8*ABU
				mm1 = _mm_mullo_pi16(mm1, mm6);   // mv_y*(CDU-ABU)
				mm0 = _mm_add_pi16(mm0, mm5);    // 8*ABU + 32
				mm0 = _mm_add_pi16(mm0, mm1);    // 8*ABU + mv_y*(CDU-ABU) + 32
				mm0 = _mm_srai_pi16(mm0, 6);       // (8*ABU + mv_y*(CDU-ABU) + 32)>>6
				mm0 = _mm_packs_pu16(mm0, mm0);  // Words -> bytes
				*((DWORD*)&pDstUV[0]) = _m_to_int(mm0);

				mm2 = *((__m64*)&pSrcUV[2*stride_src]);
				mm0 = *((__m64*)&pSrcUV[2*stride_src+2]);
				mm2 = _mm_unpacklo_pi8(mm2, mm4); // cU
				mm0 = _mm_unpacklo_pi8(mm0, mm4); // dU
				mm0 = _mm_sub_pi16(mm0, mm2);    // dU-cU
				mm2 = _mm_slli_pi16(mm2, 3);       // 8*cU
				mm0 = _mm_mullo_pi16(mm0, mm7);   // mv_x*(dU-cU)
				mm2 = _mm_add_pi16(mm2, mm0);    // 8*cU+mv_x*(dU-cU) = CDU
				mm0 = mm2;                          // 8*cU+mv_x*(dU-cU) = CDU - need to save
				mm2 = _mm_sub_pi16(mm2, mm3);    // CDU-ABU
				mm3 = _mm_slli_pi16(mm3, 3);       // 8*ABU
				mm2 = _mm_mullo_pi16(mm2, mm6);   // mv_y*(CDU-ABU)
				mm3 = _mm_add_pi16(mm3, mm5);    // 8*ABU + 32
				mm3 = _mm_add_pi16(mm3, mm2);    // 8*ABU + mv_y*(CDU-ABU) + 32
				mm3 = _mm_srai_pi16(mm3, 6);       // (8*ABU + mv_y*(CDU-ABU) + 32)>>6
				mm3 = _mm_packs_pu16(mm3, mm3);  // Words -> bytes
				*((DWORD*)&pDstUV[stride_dst]) = _m_to_int(mm3);		

				pSrcUV  += 2*stride_src;
				pDstUV  += 2*stride_dst;
			}
			pSrcUV -= 2*stride_src*number;
			pDstUV -= 2*stride_dst*number;
			number=0;			
			// 2
			mm0 = *((__m64*)&pSrcUV[0]);
			mm1 = *((__m64*)&pSrcUV[2]);
			mm0 = _mm_unpackhi_pi8(mm0, mm4); // aU
			mm1 = _mm_unpackhi_pi8(mm1, mm4); // bU
			mm1 = _mm_sub_pi16(mm1, mm0);    // bU-aU
			mm0 = _mm_slli_pi16(mm0, 3);       // 8*aU
			mm1 = _mm_mullo_pi16(mm1, mm7);   // mv_x*(bU-aU)
			mm0 = _mm_add_pi16(mm0, mm1);    // 8*aU+mv_x*(bU-aU) = ABU
			
			for(count=H;count>0;count-=2)
			{
				mm1 = *((__m64*)&pSrcUV[stride_src]);
				mm3 = *((__m64*)&pSrcUV[stride_src+2]);
				mm1 = _mm_unpackhi_pi8(mm1, mm4); // cU
				mm3 = _mm_unpackhi_pi8(mm3, mm4); // dU
				mm3 = _mm_sub_pi16(mm3, mm1);    // dU-cU
				mm1 = _mm_slli_pi16(mm1, 3);       // 8*cU
				mm3 = _mm_mullo_pi16(mm3, mm7);   // mv_x*(dU-cU)
				mm1 = _mm_add_pi16(mm1, mm3);    // 8*cU+mv_x*(dU-cU) = CDU
				mm3 = mm1;                          // 8*cU+mv_x*(dU-cU) = CDU - need to save
				mm1 = _mm_sub_pi16(mm1, mm0);    // CDU-ABU
				mm0 = _mm_slli_pi16(mm0, 3);       // 8*ABU
				mm1 = _mm_mullo_pi16(mm1, mm6);   // mv_y*(CDU-ABU)
				mm0 = _mm_add_pi16(mm0, mm5);    // 8*ABU + 32
				mm0 = _mm_add_pi16(mm0, mm1);    // 8*ABU + mv_y*(CDU-ABU) + 32
				mm0 = _mm_srai_pi16(mm0, 6);       // (8*ABU + mv_y*(CDU-ABU) + 32)>>6
				mm0 = _mm_packs_pu16(mm0, mm0);  // Words -> bytes
				*((DWORD*)&pDstUV[4]) = _m_to_int(mm0);
				
					mm2 = *((__m64*)&pSrcUV[2*stride_src]);
				mm0 = *((__m64*)&pSrcUV[2*stride_src+2]);
				mm2 = _mm_unpackhi_pi8(mm2, mm4); // cU
				mm0 = _mm_unpackhi_pi8(mm0, mm4); // dU
				mm0 = _mm_sub_pi16(mm0, mm2);    // dU-cU
				mm2 = _mm_slli_pi16(mm2, 3);       // 8*cU
				mm0 = _mm_mullo_pi16(mm0, mm7);   // mv_x*(dU-cU)
				mm2 = _mm_add_pi16(mm2, mm0);    // 8*cU+mv_x*(dU-cU) = CDU
				mm0 = mm2;                          // 8*cU+mv_x*(dU-cU) = CDU - need to save
				mm2 = _mm_sub_pi16(mm2, mm3);    // CDU-ABU
				mm3 = _mm_slli_pi16(mm3, 3);       // 8*ABU
				mm2 = _mm_mullo_pi16(mm2, mm6);   // mv_y*(CDU-ABU)
				mm3 = _mm_add_pi16(mm3, mm5);    // 8*ABU + 32
				mm3 = _mm_add_pi16(mm3, mm2);    // 8*ABU + mv_y*(CDU-ABU) + 32
				mm3 = _mm_srai_pi16(mm3, 6);       // (8*ABU + mv_y*(CDU-ABU) + 32)>>6
				mm3 = _mm_packs_pu16(mm3, mm3);  // Words -> bytes
				*((DWORD*)&pDstUV[stride_dst+4]) = _m_to_int(mm3);
			
				pSrcUV  += 2*stride_src;
				pDstUV  += 2*stride_dst;
			}
			
		}
	}
}

void mb_chroma_4xH_pred_sse2 PARGS8( StorablePicture *p,
																	 imgpel *pDstUV,
																	 int stride_dst,
																	 int base_x_pos,
																	 int base_y_pos,
																	 int mv_x,
																	 int mv_y,
																	 int H)
{
	int if1 = mv_x&7;
	int jf1 = mv_y&7;
	int ii0 = base_x_pos + (mv_x>>3);
	int jj0 = base_y_pos + (mv_y>>3);
	int stride_src = p->UV_stride;
	imgpel *pSrcUV;

	int count;
	__m128i xmm0,xmm1,xmm2,xmm3,xmm4,xmm5,xmm6,xmm7;
	__m64   mm0, mm1, mm2, mm3;
	static unsigned short __declspec(align(16)) const_val[8][8] = { {32,32,32,32,32,32,32,32 },
	{ 1, 1, 1, 1, 1, 1, 1, 1 },
	{ 2, 2, 2, 2, 2, 2, 2, 2 },
	{ 3, 3, 3, 3, 3, 3, 3, 3 },
	{ 4, 4, 4, 4, 4, 4, 4, 4 },
	{ 5, 5, 5, 5, 5, 5, 5, 5 },
	{ 6, 6, 6, 6, 6, 6, 6, 6 },
	{ 7, 7, 7, 7, 7, 7, 7, 7 }
	};

	ii0 = __fast_iclip(clip_min_x_cr,clip_max_x_cr,ii0);

	pSrcUV = p->imgUV+(ii0<<1);

	jj0 = clip_vertical_c ARGS5(jj0, pSrcUV, stride_src, 8, H);

	pSrcUV += jj0*stride_src;

	// detect the case of motion vector(0,0)  
	if(if1==0)
	{
		if(jf1==0)
		{
			for(count=H;count>0;count-=4)
			{
				mm0 = *(__m64*) &pSrcUV[0*stride_src];
				mm1 = *(__m64*) &pSrcUV[1*stride_src];
				mm2 = *(__m64*) &pSrcUV[2*stride_src];
				mm3 = *(__m64*) &pSrcUV[3*stride_src];
				*(__m64*) &pDstUV[0*stride_dst] = mm0;
				*(__m64*) &pDstUV[1*stride_dst] = mm1;
				*(__m64*) &pDstUV[2*stride_dst] = mm2;
				*(__m64*) &pDstUV[3*stride_dst] = mm3;	    
				pSrcUV += 4*stride_src;
				pDstUV += 4*stride_dst;
				// Process 4 lines at a time
			}
		}
		else
		{
			xmm4 = _mm_setzero_si128();
			xmm5 = _mm_load_si128((__m128i*) &const_val[4][0]);
			xmm6 = _mm_load_si128((__m128i*) &const_val[jf1][0]);

			mm0  = *(__m64*) &pSrcUV[0];
			xmm0 = _mm_movpi64_epi64(mm0);
			xmm0 = _mm_unpacklo_epi8(xmm0, xmm4); // aU

			for(count=H;count>0;count-=2)
			{
				mm2  = *(__m64*) &pSrcUV[stride_src];
				xmm2 = _mm_movpi64_epi64(mm2);
				xmm2 = _mm_unpacklo_epi8(xmm2, xmm4); // cU
				xmm1 = xmm2;                          // cU - need to save
				xmm2 = _mm_subs_epi16(xmm2, xmm0);    // cU-aU
				xmm0 = _mm_slli_epi16(xmm0, 3);       // 8*aU
				xmm2 = _mm_mullo_epi16(xmm2, xmm6);   // mv_y*(cU-aU)
				xmm0 = _mm_adds_epi16(xmm0, xmm5);    // 8*aU + 4
				xmm0 = _mm_adds_epi16(xmm0, xmm2);    // 8*aU + mv_y*(cU-aU) + 4
				xmm0 = _mm_srai_epi16(xmm0, 3);       // (8*aU + mv_y*(cU-aU) + 4)>>3
				xmm0 = _mm_packus_epi16(xmm0, xmm0);  // Words -> bytes
				_mm_storel_epi64((__m128i*) &pDstUV[0], xmm0);		

				mm2  = *(__m64*) &pSrcUV[2*stride_src];
				xmm2 = _mm_movpi64_epi64(mm2);
				xmm2 = _mm_unpacklo_epi8(xmm2, xmm4); // cU
				xmm0 = xmm2;                          // cU - need to save
				xmm2 = _mm_subs_epi16(xmm2, xmm1);    // cU-aU
				xmm1 = _mm_slli_epi16(xmm1, 3);       // 8*aU
				xmm2 = _mm_mullo_epi16(xmm2, xmm6);   // mv_y*(cU-aU)
				xmm1 = _mm_adds_epi16(xmm1, xmm5);    // 8*aU + 4
				xmm1 = _mm_adds_epi16(xmm1, xmm2);    // 8*aU + mv_y*(cU-aU) + 4
				xmm1 = _mm_srai_epi16(xmm1, 3);       // (8*aU + mv_y*(cU-aU) + 4)>>3
				xmm1 = _mm_packus_epi16(xmm1, xmm1);  // Words -> bytes
				_mm_storel_epi64((__m128i*) &pDstUV[stride_dst], xmm1);		

				pSrcUV += 2*stride_src;
				pDstUV += 2*stride_dst;
				// Process 2 lines at a time
			}
		}
	}
	else
	{
		if(jf1==0)
		{
			xmm4 = _mm_setzero_si128();
			xmm5 = _mm_load_si128((__m128i*) &const_val[4][0]);
			xmm7 = _mm_load_si128((__m128i*) &const_val[if1][0]);

			for(count=H;count>0;count-=2)
			{
				xmm2 = _mm_loadu_si128((__m128i*) &pSrcUV[0]);
				xmm3 = xmm2;
				xmm3 = _mm_srli_si128(xmm3, 2);
				xmm2 = _mm_unpacklo_epi8(xmm2, xmm4); // cU
				xmm3 = _mm_unpacklo_epi8(xmm3, xmm4); // dU
				xmm3 = _mm_subs_epi16(xmm3, xmm2);    // dU-cU
				xmm2 = _mm_slli_epi16(xmm2, 3);       // 8*cU
				xmm3 = _mm_mullo_epi16(xmm3, xmm7);   // mv_x*(dU-cU)
				xmm2 = _mm_adds_epi16(xmm2, xmm5);    // 8*cU + 4
				xmm2 = _mm_adds_epi16(xmm2, xmm3);    // 8*cU + mv_x*(dU-cU) + 4
				xmm2 = _mm_srai_epi16(xmm2, 3);       // (8*cU + mv_x*(dU-cU) + 4)>>3
				xmm2 = _mm_packus_epi16(xmm2, xmm2);  // Words -> bytes
				_mm_storel_epi64((__m128i*) &pDstUV[0], xmm2);	    

				xmm2 = _mm_loadu_si128((__m128i*) &pSrcUV[stride_src]);
				xmm3 = xmm2;
				xmm3 = _mm_srli_si128(xmm3, 2);
				xmm2 = _mm_unpacklo_epi8(xmm2, xmm4); // cU
				xmm3 = _mm_unpacklo_epi8(xmm3, xmm4); // dU
				xmm3 = _mm_subs_epi16(xmm3, xmm2);    // dU-cU
				xmm2 = _mm_slli_epi16(xmm2, 3);       // 8*cU
				xmm3 = _mm_mullo_epi16(xmm3, xmm7);   // mv_x*(dU-cU)
				xmm2 = _mm_adds_epi16(xmm2, xmm5);    // 8*cU + 4
				xmm2 = _mm_adds_epi16(xmm2, xmm3);    // 8*cU + mv_x*(dU-cU) + 4
				xmm2 = _mm_srai_epi16(xmm2, 3);       // (8*cU + mv_x*(dU-cU) + 4)>>3
				xmm2 = _mm_packus_epi16(xmm2, xmm2);  // Words -> bytes
				_mm_storel_epi64((__m128i*) &pDstUV[stride_dst], xmm2);	    

				pSrcUV  += 2*stride_src;
				pDstUV  += 2*stride_dst;
				// Process 2 lines at a time
			}
		}
		else
		{
			xmm4 = _mm_setzero_si128();
			xmm5 = _mm_load_si128((__m128i*) &const_val[0][0]);
			xmm6 = _mm_load_si128((__m128i*) &const_val[jf1][0]);
			xmm7 = _mm_load_si128((__m128i*) &const_val[if1][0]);

			xmm0 = _mm_loadu_si128((__m128i*) &pSrcUV[0]);
			xmm1 = xmm0;
			xmm1 = _mm_srli_si128(xmm1, 2);
			xmm0 = _mm_unpacklo_epi8(xmm0, xmm4); // aU
			xmm1 = _mm_unpacklo_epi8(xmm1, xmm4); // bU
			xmm1 = _mm_subs_epi16(xmm1, xmm0);    // bU-aU
			xmm0 = _mm_slli_epi16(xmm0, 3);       // 8*aU
			xmm1 = _mm_mullo_epi16(xmm1, xmm7);   // mv_x*(bU-aU)
			xmm0 = _mm_adds_epi16(xmm0, xmm1);    // 8*aU+mv_x*(bU-aU) = ABU

			for(count=H;count>0;count-=2)
			{
				// xmm0 = ABU, xmm2 = ABV, xmm1, xmm3 = available
				xmm1 = _mm_loadu_si128((__m128i*) &pSrcUV[stride_src]);
				xmm3 = xmm1;
				xmm3 = _mm_srli_si128(xmm3, 2);
				xmm1 = _mm_unpacklo_epi8(xmm1, xmm4); // cU
				xmm3 = _mm_unpacklo_epi8(xmm3, xmm4); // dU
				xmm3 = _mm_subs_epi16(xmm3, xmm1);    // dU-cU
				xmm1 = _mm_slli_epi16(xmm1, 3);       // 8*cU
				xmm3 = _mm_mullo_epi16(xmm3, xmm7);   // mv_x*(dU-cU)
				xmm1 = _mm_adds_epi16(xmm1, xmm3);    // 8*cU+mv_x*(dU-cU) = CDU
				xmm3 = xmm1;                          // 8*cU+mv_x*(dU-cU) = CDU - need to save
				xmm1 = _mm_subs_epi16(xmm1, xmm0);    // CDU-ABU
				xmm0 = _mm_slli_epi16(xmm0, 3);       // 8*ABU
				xmm1 = _mm_mullo_epi16(xmm1, xmm6);   // mv_y*(CDU-ABU)
				xmm0 = _mm_adds_epi16(xmm0, xmm5);    // 8*ABU + 32
				xmm0 = _mm_adds_epi16(xmm0, xmm1);    // 8*ABU + mv_y*(CDU-ABU) + 32
				xmm0 = _mm_srai_epi16(xmm0, 6);       // (8*ABU + mv_y*(CDU-ABU) + 32)>>6
				xmm0 = _mm_packus_epi16(xmm0, xmm0);  // Words -> bytes
				_mm_storel_epi64((__m128i*) &pDstUV[0], xmm0);		

				// xmm3 = ABU, xmm1 = ABV, xmm0, xmm2 = available
				xmm2 = _mm_loadu_si128((__m128i*) &pSrcUV[2*stride_src]);
				xmm0 = xmm2;
				xmm0 = _mm_srli_si128(xmm0, 2);
				xmm2 = _mm_unpacklo_epi8(xmm2, xmm4); // cU
				xmm0 = _mm_unpacklo_epi8(xmm0, xmm4); // dU
				xmm0 = _mm_subs_epi16(xmm0, xmm2);    // dU-cU
				xmm2 = _mm_slli_epi16(xmm2, 3);       // 8*cU
				xmm0 = _mm_mullo_epi16(xmm0, xmm7);   // mv_x*(dU-cU)
				xmm2 = _mm_adds_epi16(xmm2, xmm0);    // 8*cU+mv_x*(dU-cU) = CDU
				xmm0 = xmm2;                          // 8*cU+mv_x*(dU-cU) = CDU - need to save
				xmm2 = _mm_subs_epi16(xmm2, xmm3);    // CDU-ABU
				xmm3 = _mm_slli_epi16(xmm3, 3);       // 8*ABU
				xmm2 = _mm_mullo_epi16(xmm2, xmm6);   // mv_y*(CDU-ABU)
				xmm3 = _mm_adds_epi16(xmm3, xmm5);    // 8*ABU + 32
				xmm3 = _mm_adds_epi16(xmm3, xmm2);    // 8*ABU + mv_y*(CDU-ABU) + 32
				xmm3 = _mm_srai_epi16(xmm3, 6);       // (8*ABU + mv_y*(CDU-ABU) + 32)>>6
				xmm3 = _mm_packus_epi16(xmm3, xmm3);  // Words -> bytes
				_mm_storel_epi64((__m128i*) &pDstUV[stride_dst], xmm3);		

				pSrcUV  += 2*stride_src;
				pDstUV  += 2*stride_dst;
				// Process 2 lines at a time
			}
		}
	}
}
#endif//H264_ENABLE_ASM

#ifdef H264_ENABLE_INTRINSICS

void mb_chroma_8xH_pred_sse2 PARGS8( StorablePicture *p,
																		imgpel *pDstUV,
																		int stride_dst,
																		int base_x_pos,
																		int base_y_pos,
																		int mv_x,
																		int mv_y,
																		int H)
{
	int if1 = mv_x&7;
	int jf1 = mv_y&7;
	int ii0 = base_x_pos + (mv_x>>3);
	int jj0 = base_y_pos + (mv_y>>3);
	int stride_src = p->UV_stride;
	imgpel *pSrcUV;

	int count;
	__m128i xmm0,xmm1,xmm2,xmm3,xmm4,xmm5,xmm6,xmm7;
	__m64   mm0, mm2, mm3;
	static unsigned short __declspec(align(16)) const_val[8][8] = { {32,32,32,32,32,32,32,32 },
	{ 1, 1, 1, 1, 1, 1, 1, 1 },
	{ 2, 2, 2, 2, 2, 2, 2, 2 },
	{ 3, 3, 3, 3, 3, 3, 3, 3 },
	{ 4, 4, 4, 4, 4, 4, 4, 4 },
	{ 5, 5, 5, 5, 5, 5, 5, 5 },
	{ 6, 6, 6, 6, 6, 6, 6, 6 },
	{ 7, 7, 7, 7, 7, 7, 7, 7 }
	};

	ii0 = __fast_iclip(clip_min_x_cr,clip_max_x_cr,ii0);

	pSrcUV = p->imgUV+(ii0<<1);

	jj0 = clip_vertical_c ARGS5(jj0, pSrcUV, stride_src, 8, H);

	pSrcUV += jj0*stride_src;

	// detect the case of motion vector(0,0)
	if(if1==0)
	{
		if(jf1==0)
		{
			for(count=H;count>0;count-=4)
			{
				xmm0 = _mm_loadu_si128 ((__m128i*) &pSrcUV[0*stride_src]);
				xmm1 = _mm_loadu_si128 ((__m128i*) &pSrcUV[1*stride_src]);
				xmm2 = _mm_loadu_si128 ((__m128i*) &pSrcUV[2*stride_src]);
				xmm3 = _mm_loadu_si128 ((__m128i*) &pSrcUV[3*stride_src]);
				*(__m128i*) &pDstUV[0*stride_dst] = xmm0;
				*(__m128i*) &pDstUV[1*stride_dst] = xmm1;
				*(__m128i*) &pDstUV[2*stride_dst] = xmm2;
				*(__m128i*) &pDstUV[3*stride_dst] = xmm3;

				pSrcUV += 4*stride_src;
				pDstUV += 4*stride_dst;
				// Process 4 lines at a time
			}
		}
		else
		{
			xmm4 = _mm_setzero_si128();
			xmm5 = _mm_load_si128((__m128i*) &const_val[4][0]);
			xmm6 = _mm_load_si128((__m128i*) &const_val[jf1][0]);

			mm0  = *(__m64*) &pSrcUV[0];
			mm3  = *(__m64*) &pSrcUV[8];
			xmm0 = _mm_movpi64_epi64(mm0);
			xmm3 = _mm_movpi64_epi64(mm3);
			xmm0 = _mm_unpacklo_epi8(xmm0, xmm4); // aU
			xmm3 = _mm_unpacklo_epi8(xmm3, xmm4); // aV
			for(count=H;count>0;count-=2)
			{
				mm2  = *(__m64*) &pSrcUV[stride_src];
				xmm2 = _mm_movpi64_epi64(mm2);
				xmm2 = _mm_unpacklo_epi8(xmm2, xmm4); // cU
				xmm1 = xmm2;                          // cU - need to save
				xmm2 = _mm_subs_epi16(xmm2, xmm0);    // cU-aU
				xmm0 = _mm_slli_epi16(xmm0, 3);       // 8*aU
				xmm2 = _mm_mullo_epi16(xmm2, xmm6);   // mv_y*(cU-aU)
				xmm0 = _mm_adds_epi16(xmm0, xmm5);    // 8*aU + 4
				xmm0 = _mm_adds_epi16(xmm0, xmm2);    // 8*aU + mv_y*(cU-aU) + 4
				xmm0 = _mm_srai_epi16(xmm0, 3);       // (8*aU + mv_y*(cU-aU) + 4)>>3
				xmm0 = _mm_packus_epi16(xmm0, xmm0);  // Words -> bytes
				_mm_storel_epi64((__m128i*) &pDstUV[0], xmm0);

				mm2  = *(__m64*) &pSrcUV[stride_src+8];
				xmm2 = _mm_movpi64_epi64(mm2);
				xmm2 = _mm_unpacklo_epi8(xmm2, xmm4); // cV
				xmm7 = xmm2;                          // cV - need to save
				xmm2 = _mm_subs_epi16(xmm2, xmm3);    // cV-aV
				xmm3 = _mm_slli_epi16(xmm3, 3);       // 8*aV
				xmm2 = _mm_mullo_epi16(xmm2, xmm6);   // mv_y*(cV-aV)
				xmm3 = _mm_adds_epi16(xmm3, xmm5);    // 8*aV + 4
				xmm3 = _mm_adds_epi16(xmm3, xmm2);    // 8*aV + mv_y*(cV-aV) + 4
				xmm3 = _mm_srai_epi16(xmm3, 3);       // (8*aV + mv_y*(cV-aV) + 4)>>3
				xmm3 = _mm_packus_epi16(xmm3, xmm3);  // Words -> bytes
				_mm_storel_epi64((__m128i*) &pDstUV[8], xmm3);

				mm2  = *(__m64*) &pSrcUV[2*stride_src];
				xmm2 = _mm_movpi64_epi64(mm2);
				xmm2 = _mm_unpacklo_epi8(xmm2, xmm4); // cU
				xmm0 = xmm2;                          // cU - need to save
				xmm2 = _mm_subs_epi16(xmm2, xmm1);    // cU-aU
				xmm1 = _mm_slli_epi16(xmm1, 3);       // 8*aU
				xmm2 = _mm_mullo_epi16(xmm2, xmm6);   // mv_y*(cU-aU)
				xmm1 = _mm_adds_epi16(xmm1, xmm5);    // 8*aU + 4
				xmm1 = _mm_adds_epi16(xmm1, xmm2);    // 8*aU + mv_y*(cU-aU) + 4
				xmm1 = _mm_srai_epi16(xmm1, 3);       // (8*aU + mv_y*(cU-aU) + 4)>>3
				xmm1 = _mm_packus_epi16(xmm1, xmm1);  // Words -> bytes
				_mm_storel_epi64((__m128i*) &pDstUV[stride_dst], xmm1);

				mm2  = *(__m64*) &pSrcUV[2*stride_src+8];
				xmm2 = _mm_movpi64_epi64(mm2);
				xmm2 = _mm_unpacklo_epi8(xmm2, xmm4); // cV
				xmm3 = xmm2;                          // cV - need to save
				xmm2 = _mm_subs_epi16(xmm2, xmm7);    // cV-aV
				xmm7 = _mm_slli_epi16(xmm7, 3);       // 8*aV
				xmm2 = _mm_mullo_epi16(xmm2, xmm6);   // mv_y*(cV-aV)
				xmm7 = _mm_adds_epi16(xmm7, xmm5);    // 8*aV + 4
				xmm7 = _mm_adds_epi16(xmm7, xmm2);    // 8*aV + mv_y*(cV-aV) + 4
				xmm7 = _mm_srai_epi16(xmm7, 3);       // (8*aV + mv_y*(cV-aV) + 4)>>3
				xmm7 = _mm_packus_epi16(xmm7, xmm7);  // Words -> bytes
				_mm_storel_epi64((__m128i*) &pDstUV[stride_dst+8], xmm7);

				pSrcUV += 2*stride_src;
				pDstUV += 2*stride_dst;
				// Process 2 lines at a time
			}
		}
	}
	else
	{
		if(jf1==0)
		{
			xmm4 = _mm_setzero_si128();
			xmm5 = _mm_load_si128((__m128i*) &const_val[4][0]);
			xmm7 = _mm_load_si128((__m128i*) &const_val[if1][0]);

			for(count=H;count>0;count-=2)
			{
				xmm2 = _mm_loadu_si128((__m128i*) &pSrcUV[0]);
				xmm3 = xmm2;
				xmm3 = _mm_srli_si128(xmm3, 2);
				xmm2 = _mm_unpacklo_epi8(xmm2, xmm4); // cU
				xmm3 = _mm_unpacklo_epi8(xmm3, xmm4); // dU
				xmm3 = _mm_subs_epi16(xmm3, xmm2);    // dU-cU
				xmm2 = _mm_slli_epi16(xmm2, 3);       // 8*cU
				xmm3 = _mm_mullo_epi16(xmm3, xmm7);   // mv_x*(dU-cU)
				xmm2 = _mm_adds_epi16(xmm2, xmm5);    // 8*cU + 4
				xmm2 = _mm_adds_epi16(xmm2, xmm3);    // 8*cU + mv_x*(dU-cU) + 4
				xmm2 = _mm_srai_epi16(xmm2, 3);       // (8*cU + mv_x*(dU-cU) + 4)>>3
				xmm2 = _mm_packus_epi16(xmm2, xmm2);  // Words -> bytes
				_mm_storel_epi64((__m128i*) &pDstUV[0], xmm2);

				xmm0 = _mm_loadu_si128((__m128i*) &pSrcUV[8]);
				xmm1 = xmm0;
				xmm1 = _mm_srli_si128(xmm1, 2);
				xmm0 = _mm_unpacklo_epi8(xmm0, xmm4); // cV
				xmm1 = _mm_unpacklo_epi8(xmm1, xmm4); // dV
				xmm1 = _mm_subs_epi16(xmm1, xmm0);    // dV-cV
				xmm0 = _mm_slli_epi16(xmm0, 3);       // 8*cV
				xmm1 = _mm_mullo_epi16(xmm1, xmm7);   // mv_x*(dV-cV)
				xmm0 = _mm_adds_epi16(xmm0, xmm5);    // 8*cV + 4
				xmm0 = _mm_adds_epi16(xmm0, xmm1);    // 8*cV + mv_x*(dV-cV) + 4
				xmm0 = _mm_srai_epi16(xmm0, 3);       // (8*cV + mv_x*(dV-cV) + 4)>>3
				xmm0 = _mm_packus_epi16(xmm0, xmm0);  // Words -> bytes
				_mm_storel_epi64((__m128i*) &pDstUV[8], xmm0);

				xmm2 = _mm_loadu_si128((__m128i*) &pSrcUV[stride_src]);
				xmm3 = xmm2;
				xmm3 = _mm_srli_si128(xmm3, 2);
				xmm2 = _mm_unpacklo_epi8(xmm2, xmm4); // cU
				xmm3 = _mm_unpacklo_epi8(xmm3, xmm4); // dU
				xmm3 = _mm_subs_epi16(xmm3, xmm2);    // dU-cU
				xmm2 = _mm_slli_epi16(xmm2, 3);       // 8*cU
				xmm3 = _mm_mullo_epi16(xmm3, xmm7);   // mv_x*(dU-cU)
				xmm2 = _mm_adds_epi16(xmm2, xmm5);    // 8*cU + 4
				xmm2 = _mm_adds_epi16(xmm2, xmm3);    // 8*cU + mv_x*(dU-cU) + 4
				xmm2 = _mm_srai_epi16(xmm2, 3);       // (8*cU + mv_x*(dU-cU) + 4)>>3
				xmm2 = _mm_packus_epi16(xmm2, xmm2);  // Words -> bytes
				_mm_storel_epi64((__m128i*) &pDstUV[stride_dst], xmm2);

				xmm0 = _mm_loadu_si128((__m128i*) &pSrcUV[stride_src+8]);
				xmm1 = xmm0;
				xmm1 = _mm_srli_si128(xmm1, 2);
				xmm0 = _mm_unpacklo_epi8(xmm0, xmm4); // cV
				xmm1 = _mm_unpacklo_epi8(xmm1, xmm4); // dV
				xmm1 = _mm_subs_epi16(xmm1, xmm0);    // dV-cV
				xmm0 = _mm_slli_epi16(xmm0, 3);       // 8*cV
				xmm1 = _mm_mullo_epi16(xmm1, xmm7);   // mv_x*(dV-cV)
				xmm0 = _mm_adds_epi16(xmm0, xmm5);    // 8*cV + 4
				xmm0 = _mm_adds_epi16(xmm0, xmm1);    // 8*cV + mv_x*(dV-cV) + 4
				xmm0 = _mm_srai_epi16(xmm0, 3);       // (8*cV + mv_x*(dV-cV) + 4)>>3
				xmm0 = _mm_packus_epi16(xmm0, xmm0);  // Words -> bytes
				_mm_storel_epi64((__m128i*) &pDstUV[stride_dst+8], xmm0);

				pSrcUV  += 2*stride_src;
				pDstUV  += 2*stride_dst;
				// Process 2 lines at a time
			}
		}
		else
		{
			xmm4 = _mm_setzero_si128();
			xmm5 = _mm_load_si128((__m128i*) &const_val[0][0]);
			xmm6 = _mm_load_si128((__m128i*) &const_val[jf1][0]);
			xmm7 = _mm_load_si128((__m128i*) &const_val[if1][0]);

			xmm0 = _mm_loadu_si128((__m128i*) &pSrcUV[0]);
			xmm1 = xmm0;
			xmm1 = _mm_srli_si128(xmm1, 2);
			xmm0 = _mm_unpacklo_epi8(xmm0, xmm4); // aU
			xmm1 = _mm_unpacklo_epi8(xmm1, xmm4); // bU
			xmm1 = _mm_subs_epi16(xmm1, xmm0);    // bU-aU
			xmm0 = _mm_slli_epi16(xmm0, 3);       // 8*aU
			xmm1 = _mm_mullo_epi16(xmm1, xmm7);   // mv_x*(bU-aU)
			xmm0 = _mm_adds_epi16(xmm0, xmm1);    // 8*aU+mv_x*(bU-aU) = ABU

			xmm2 = _mm_loadu_si128((__m128i*) &pSrcUV[8]);
			xmm3 = xmm2;
			xmm3 = _mm_srli_si128(xmm3, 2);
			xmm2 = _mm_unpacklo_epi8(xmm2, xmm4); // aV
			xmm3 = _mm_unpacklo_epi8(xmm3, xmm4); // bV
			xmm3 = _mm_subs_epi16(xmm3, xmm2);    // bV-aV
			xmm2 = _mm_slli_epi16(xmm2, 3);       // 8*aV
			xmm3 = _mm_mullo_epi16(xmm3, xmm7);   // mv_x*(bV-aV)
			xmm2 = _mm_adds_epi16(xmm2, xmm3);    // 8*aU+mv_x*(bV-aV) = ABV
			for(count=H;count>0;count-=2)
			{
				// xmm0 = ABU, xmm2 = ABV, xmm1, xmm3 = available
				xmm1 = _mm_loadu_si128((__m128i*) &pSrcUV[stride_src]);
				xmm3 = xmm1;
				xmm3 = _mm_srli_si128(xmm3, 2);
				xmm1 = _mm_unpacklo_epi8(xmm1, xmm4); // cU
				xmm3 = _mm_unpacklo_epi8(xmm3, xmm4); // dU
				xmm3 = _mm_subs_epi16(xmm3, xmm1);    // dU-cU
				xmm1 = _mm_slli_epi16(xmm1, 3);       // 8*cU
				xmm3 = _mm_mullo_epi16(xmm3, xmm7);   // mv_x*(dU-cU)
				xmm1 = _mm_adds_epi16(xmm1, xmm3);    // 8*cU+mv_x*(dU-cU) = CDU
				xmm3 = xmm1;                          // 8*cU+mv_x*(dU-cU) = CDU - need to save
				xmm1 = _mm_subs_epi16(xmm1, xmm0);    // CDU-ABU
				xmm0 = _mm_slli_epi16(xmm0, 3);       // 8*ABU
				xmm1 = _mm_mullo_epi16(xmm1, xmm6);   // mv_y*(CDU-ABU)
				xmm0 = _mm_adds_epi16(xmm0, xmm5);    // 8*ABU + 32
				xmm0 = _mm_adds_epi16(xmm0, xmm1);    // 8*ABU + mv_y*(CDU-ABU) + 32
				xmm0 = _mm_srai_epi16(xmm0, 6);       // (8*ABU + mv_y*(CDU-ABU) + 32)>>6
				xmm0 = _mm_packus_epi16(xmm0, xmm0);  // Words -> bytes
				_mm_storel_epi64((__m128i*) &pDstUV[0], xmm0);

				// xmm3 = CDU, xmm2 = ABV, xmm0, xmm1 = available
				xmm0 = _mm_loadu_si128((__m128i*) &pSrcUV[stride_src+8]);
				xmm1 = xmm0;
				xmm1 = _mm_srli_si128(xmm1, 2);
				xmm0 = _mm_unpacklo_epi8(xmm0, xmm4); // cV
				xmm1 = _mm_unpacklo_epi8(xmm1, xmm4); // dV
				xmm1 = _mm_subs_epi16(xmm1, xmm0);    // dV-cV
				xmm0 = _mm_slli_epi16(xmm0, 3);       // 8*cV
				xmm1 = _mm_mullo_epi16(xmm1, xmm7);   // mv_x*(dV-cV)
				xmm0 = _mm_adds_epi16(xmm0, xmm1);    // 8*cV+mv_x*(dV-cV) = CDV
				xmm1 = xmm0;                          // 8*cV+mv_x*(dV-cV) = CDV - need to save
				xmm0 = _mm_subs_epi16(xmm0, xmm2);    // CDV-ABV
				xmm2 = _mm_slli_epi16(xmm2, 3);       // 8*ABV
				xmm0 = _mm_mullo_epi16(xmm0, xmm6);   // mv_y*(CDV-ABV)
				xmm2 = _mm_adds_epi16(xmm2, xmm5);    // 8*ABV + 32
				xmm2 = _mm_adds_epi16(xmm2, xmm0);    // 8*ABV + mv_y*(CDV-ABV) + 32
				xmm2 = _mm_srai_epi16(xmm2, 6);       // (8*ABV + mv_y*(CDV-ABV) + 32)>>6
				xmm2 = _mm_packus_epi16(xmm2, xmm2);  // Words -> bytes
				_mm_storel_epi64((__m128i*) &pDstUV[8], xmm2);

				// xmm3 = ABU, xmm1 = ABV, xmm0, xmm2 = available
				xmm2 = _mm_loadu_si128((__m128i*) &pSrcUV[2*stride_src]);
				xmm0 = xmm2;
				xmm0 = _mm_srli_si128(xmm0, 2);
				xmm2 = _mm_unpacklo_epi8(xmm2, xmm4); // cU
				xmm0 = _mm_unpacklo_epi8(xmm0, xmm4); // dU
				xmm0 = _mm_subs_epi16(xmm0, xmm2);    // dU-cU
				xmm2 = _mm_slli_epi16(xmm2, 3);       // 8*cU
				xmm0 = _mm_mullo_epi16(xmm0, xmm7);   // mv_x*(dU-cU)
				xmm2 = _mm_adds_epi16(xmm2, xmm0);    // 8*cU+mv_x*(dU-cU) = CDU
				xmm0 = xmm2;                          // 8*cU+mv_x*(dU-cU) = CDU - need to save
				xmm2 = _mm_subs_epi16(xmm2, xmm3);    // CDU-ABU
				xmm3 = _mm_slli_epi16(xmm3, 3);       // 8*ABU
				xmm2 = _mm_mullo_epi16(xmm2, xmm6);   // mv_y*(CDU-ABU)
				xmm3 = _mm_adds_epi16(xmm3, xmm5);    // 8*ABU + 32
				xmm3 = _mm_adds_epi16(xmm3, xmm2);    // 8*ABU + mv_y*(CDU-ABU) + 32
				xmm3 = _mm_srai_epi16(xmm3, 6);       // (8*ABU + mv_y*(CDU-ABU) + 32)>>6
				xmm3 = _mm_packus_epi16(xmm3, xmm3);  // Words -> bytes
				_mm_storel_epi64((__m128i*) &pDstUV[stride_dst], xmm3);

				// xmm0 = CDU, xmm1 = ABV, xmm2, xmm3 = available
				xmm3 = _mm_loadu_si128((__m128i*) &pSrcUV[2*stride_src+8]);
				xmm2 = xmm3;
				xmm2 = _mm_srli_si128(xmm2, 2);
				xmm3 = _mm_unpacklo_epi8(xmm3, xmm4); // cV
				xmm2 = _mm_unpacklo_epi8(xmm2, xmm4); // dV
				xmm2 = _mm_subs_epi16(xmm2, xmm3);    // dV-cV
				xmm3 = _mm_slli_epi16(xmm3, 3);       // 8*cV
				xmm2 = _mm_mullo_epi16(xmm2, xmm7);   // mv_x*(dV-cV)
				xmm3 = _mm_adds_epi16(xmm3, xmm2);    // 8*cV+mv_x*(dV-cV) = CDV
				xmm2 = xmm3;                          // 8*cV+mv_x*(dV-cV) = CDV - need to save
				xmm3 = _mm_subs_epi16(xmm3, xmm1);    // CDV-ABV
				xmm1 = _mm_slli_epi16(xmm1, 3);       // 8*ABV
				xmm3 = _mm_mullo_epi16(xmm3, xmm6);   // mv_y*(CDV-ABV)
				xmm1 = _mm_adds_epi16(xmm1, xmm5);    // 8*ABV + 32
				xmm1 = _mm_adds_epi16(xmm1, xmm3);    // 8*ABV + mv_y*(CDV-ABV) + 32
				xmm1 = _mm_srai_epi16(xmm1, 6);       // (8*ABV + mv_y*(CDV-ABV) + 32)>>6
				xmm1 = _mm_packus_epi16(xmm1, xmm1);  // Words -> bytes
				_mm_storel_epi64((__m128i*) &pDstUV[stride_dst+8], xmm1);

				pSrcUV  += 2*stride_src;
				pDstUV  += 2*stride_dst;
				// Process 2 lines at a time
			}
		}
	}
}

void mb_chroma_8xH_pred_mmx PARGS8( StorablePicture *p,
																		imgpel *pDstUV,
																		int stride_dst,
																		int base_x_pos,
																		int base_y_pos,
																		int mv_x,
																		int mv_y,
																		int H)
{
	int if1 = mv_x&7;
	int jf1 = mv_y&7;
	int ii0 = base_x_pos + (mv_x>>3);
	int jj0 = base_y_pos + (mv_y>>3);
	int stride_src = p->UV_stride;
	imgpel *pSrcUV;

	int count;
	int number=0;
	__m64   mm0, mm1, mm2, mm3, mm4, mm5, mm6, mm7;
	static unsigned short __declspec(align(16)) const_val[8][8] = { {32,32,32,32,32,32,32,32 },
	{ 1, 1, 1, 1, 1, 1, 1, 1 },
	{ 2, 2, 2, 2, 2, 2, 2, 2 },
	{ 3, 3, 3, 3, 3, 3, 3, 3 },
	{ 4, 4, 4, 4, 4, 4, 4, 4 },
	{ 5, 5, 5, 5, 5, 5, 5, 5 },
	{ 6, 6, 6, 6, 6, 6, 6, 6 },
	{ 7, 7, 7, 7, 7, 7, 7, 7 }
	};

	ii0 = __fast_iclip(clip_min_x_cr,clip_max_x_cr,ii0);

	pSrcUV = p->imgUV+(ii0<<1);

	jj0 = clip_vertical_c ARGS5(jj0, pSrcUV, stride_src, 8, H);

	pSrcUV += jj0*stride_src;

	// detect the case of motion vector(0,0)
	if(if1==0)
	{
		if(jf1==0)
		{
			for(count=H;count>0;count-=4)
			{
				mm0 = *((__m64*)&pSrcUV[0*stride_src]);
				mm1 = *((__m64*)&pSrcUV[1*stride_src]);
				mm2 = *((__m64*)&pSrcUV[2*stride_src]);
				mm3 = *((__m64*)&pSrcUV[3*stride_src]);
				*((__m64*)&pDstUV[0*stride_dst]) = mm0;
				*((__m64*)&pDstUV[1*stride_dst]) = mm1;
				*((__m64*)&pDstUV[2*stride_dst]) = mm2;
				*((__m64*)&pDstUV[3*stride_dst]) = mm3;

				mm0 = *((__m64*)&pSrcUV[0*stride_src+8]);
				mm1 = *((__m64*)&pSrcUV[1*stride_src+8]);
				mm2 = *((__m64*)&pSrcUV[2*stride_src+8]);
				mm3 = *((__m64*)&pSrcUV[3*stride_src+8]);
			
				*((__m64*)&pDstUV[0*stride_dst+8]) = mm0;
				*((__m64*)&pDstUV[1*stride_dst+8]) = mm1;
				*((__m64*)&pDstUV[2*stride_dst+8]) = mm2;
				*((__m64*)&pDstUV[3*stride_dst+8]) = mm3;
				
				pSrcUV += 4*stride_src;
				pDstUV += 4*stride_dst;
				// Process 4 lines at a time
			}
		}
		else
		{
			//  1
			mm4 = _mm_setzero_si64();
			mm5 = *((__m64*)&const_val[4][0]);
			mm6 = *((__m64*)&const_val[jf1][0]);

			mm1  = *(__m64*) &pSrcUV[0];
			mm2  = *(__m64*) &pSrcUV[8];

			mm0 = _mm_unpacklo_pi8(mm1, mm4); // aU
			mm3 = _mm_unpacklo_pi8(mm2, mm4); // aV


			for(count=H;count>0;count-=2)
			{
				number++;
				mm2  = *(__m64*) &pSrcUV[stride_src];
				mm2 = _mm_unpacklo_pi8(mm2, mm4); // cU
				mm1 = mm2;                          // cU - need to save
				mm2 = _mm_sub_pi16(mm2, mm0);    // cU-aU
				mm0 = _mm_slli_pi16(mm0, 3);       // 8*aU
				mm2 = _mm_mullo_pi16(mm2, mm6);   // mv_y*(cU-aU)
				mm0 = _mm_add_pi16(mm0, mm5);    // 8*aU + 4
				mm0 = _mm_add_pi16(mm0, mm2);    // 8*aU + mv_y*(cU-aU) + 4
				mm0 = _mm_srai_pi16(mm0, 3);       // (8*aU + mv_y*(cU-aU) + 4)>>3
				mm0 = _mm_packs_pu16(mm0, mm0);  // Words -> bytes
				*((DWORD*)&pDstUV[0]) = _m_to_int(mm0);

				mm2  = *(__m64*) &pSrcUV[stride_src+8];
				mm2 = _mm_unpacklo_pi8(mm2, mm4); // cV
				mm7 = mm2;                          // cV - need to save
				mm2 = _mm_sub_pi16(mm2, mm3);    // cV-aV
				mm3 = _mm_slli_pi16(mm3, 3);       // 8*aV
				mm2 = _mm_mullo_pi16(mm2, mm6);   // mv_y*(cV-aV)
				mm3 = _mm_add_pi16(mm3, mm5);    // 8*aV + 4
				mm3 = _mm_add_pi16(mm3, mm2);    // 8*aV + mv_y*(cV-aV) + 4
				mm3 = _mm_srai_pi16(mm3, 3);       // (8*aV + mv_y*(cV-aV) + 4)>>3
				mm3 = _mm_packs_pu16(mm3, mm3);  // Words -> bytes
				*((DWORD*)&pDstUV[8]) = _m_to_int(mm3);


				mm2  = *(__m64*) &pSrcUV[2*stride_src];
				mm2 = _mm_unpacklo_pi8(mm2, mm4); // cU
				mm0 = mm2;                          // cU - need to save
				mm2 = _mm_sub_pi16(mm2, mm1);    // cU-aU
				mm1 = _mm_slli_pi16(mm1, 3);       // 8*aU
				mm2 = _mm_mullo_pi16(mm2, mm6);   // mv_y*(cU-aU)
				mm1 = _mm_add_pi16(mm1, mm5);    // 8*aU + 4
				mm1 = _mm_add_pi16(mm1, mm2);    // 8*aU + mv_y*(cU-aU) + 4
				mm1 = _mm_srai_pi16(mm1, 3);       // (8*aU + mv_y*(cU-aU) + 4)>>3
				mm1 = _mm_packs_pu16(mm1, mm1);  // Words -> bytes
				*((DWORD*)&pDstUV[stride_dst]) = _m_to_int(mm1);

				mm2  = *(__m64*) &pSrcUV[2*stride_src+8];
				mm2 = _mm_unpacklo_pi8(mm2, mm4); // cV
				mm3 = mm2;                          // cV - need to save
				mm2 = _mm_sub_pi16(mm2, mm7);    // cV-aV
				mm7 = _mm_slli_pi16(mm7, 3);       // 8*aV
				mm2 = _mm_mullo_pi16(mm2, mm6);   // mv_y*(cV-aV)
				mm7 = _mm_add_pi16(mm7, mm5);    // 8*aV + 4
				mm7 = _mm_add_pi16(mm7, mm2);    // 8*aV + mv_y*(cV-aV) + 4
				mm7 = _mm_srai_pi16(mm7, 3);       // (8*aV + mv_y*(cV-aV) + 4)>>3
				mm7 = _mm_packs_pu16(mm7, mm7);  // Words -> bytes
				*((DWORD*)&pDstUV[stride_dst+8]) = _m_to_int(mm7);

				pSrcUV += 2*stride_src;
				pDstUV += 2*stride_dst;
			}
			pSrcUV -= 2*stride_src*number;
			pDstUV -= 2*stride_dst*number;
			number=0;
			
			// 2
			mm5 = *((__m64*)&const_val[4][4]);
			mm6 = *((__m64*)&const_val[jf1][4]);

			mm1  = *(__m64*) &pSrcUV[0];
			mm2  = *(__m64*) &pSrcUV[8];

			mm0 = _mm_unpackhi_pi8(mm1, mm4); // aU
			mm3 = _mm_unpackhi_pi8(mm2, mm4); // aV


			for(count=H;count>0;count-=2)
			{
				mm2  = *(__m64*) &pSrcUV[stride_src];
				mm2 = _mm_unpackhi_pi8(mm2, mm4); // cU
				mm1 = mm2;                          // cU - need to save
				mm2 = _mm_sub_pi16(mm2, mm0);    // cU-aU
				mm0 = _mm_slli_pi16(mm0, 3);       // 8*aU
				mm2 = _mm_mullo_pi16(mm2, mm6);   // mv_y*(cU-aU)
				mm0 = _mm_add_pi16(mm0, mm5);    // 8*aU + 4
				mm0 = _mm_add_pi16(mm0, mm2);    // 8*aU + mv_y*(cU-aU) + 4
				mm0 = _mm_srai_pi16(mm0, 3);       // (8*aU + mv_y*(cU-aU) + 4)>>3
				mm0 = _mm_packs_pu16(mm0, mm0);  // Words -> bytes
				*((DWORD*)&pDstUV[4]) = _m_to_int(mm0);

				mm2  = *(__m64*) &pSrcUV[stride_src+8];
				mm2 = _mm_unpackhi_pi8(mm2, mm4); // cV
				mm7 = mm2;                          // cV - need to save
				mm2 = _mm_sub_pi16(mm2, mm3);    // cV-aV
				mm3 = _mm_slli_pi16(mm3, 3);       // 8*aV
				mm2 = _mm_mullo_pi16(mm2, mm6);   // mv_y*(cV-aV)
				mm3 = _mm_add_pi16(mm3, mm5);    // 8*aV + 4
				mm3 = _mm_add_pi16(mm3, mm2);    // 8*aV + mv_y*(cV-aV) + 4
				mm3 = _mm_srai_pi16(mm3, 3);       // (8*aV + mv_y*(cV-aV) + 4)>>3
				mm3 = _mm_packs_pu16(mm3, mm3);  // Words -> bytes
				*((DWORD*)&pDstUV[12]) = _m_to_int(mm3);


				mm2  = *(__m64*) &pSrcUV[2*stride_src];
				mm2 = _mm_unpackhi_pi8(mm2, mm4); // cU
				mm0 = mm2;                          // cU - need to save
				mm2 = _mm_sub_pi16(mm2, mm1);    // cU-aU
				mm1 = _mm_slli_pi16(mm1, 3);       // 8*aU
				mm2 = _mm_mullo_pi16(mm2, mm6);   // mv_y*(cU-aU)
				mm1 = _mm_add_pi16(mm1, mm5);    // 8*aU + 4
				mm1 = _mm_add_pi16(mm1, mm2);    // 8*aU + mv_y*(cU-aU) + 4
				mm1 = _mm_srai_pi16(mm1, 3);       // (8*aU + mv_y*(cU-aU) + 4)>>3
				mm1 = _mm_packs_pu16(mm1, mm1);  // Words -> bytes
				*((DWORD*)&pDstUV[stride_dst+4]) = _m_to_int(mm1);

				mm2  = *(__m64*) &pSrcUV[2*stride_src+8];
				mm2 = _mm_unpackhi_pi8(mm2, mm4); // cV
				mm3 = mm2;                          // cV - need to save
				mm2 = _mm_sub_pi16(mm2, mm7);    // cV-aV
				mm7 = _mm_slli_pi16(mm7, 3);       // 8*aV
				mm2 = _mm_mullo_pi16(mm2, mm6);   // mv_y*(cV-aV)
				mm7 = _mm_add_pi16(mm7, mm5);    // 8*aV + 4
				mm7 = _mm_add_pi16(mm7, mm2);    // 8*aV + mv_y*(cV-aV) + 4
				mm7 = _mm_srai_pi16(mm7, 3);       // (8*aV + mv_y*(cV-aV) + 4)>>3
				mm7 = _mm_packs_pu16(mm7, mm7);  // Words -> bytes
				*((DWORD*)&pDstUV[stride_dst+12]) = _m_to_int(mm7);

				pSrcUV += 2*stride_src;
				pDstUV += 2*stride_dst;
			}
			
		}
	}
	else
	{
		if(jf1==0)
		{
			mm4 = _mm_setzero_si64();
			mm5 = *((__m64*)&const_val[4][0]);
			mm7 = *((__m64*)&const_val[if1][0]);

			for(count=H;count>0;count-=2)
			{
				// 1
				mm2 = *((__m64*)&pSrcUV[0]);
				mm3 = *((__m64*)&pSrcUV[2]);
				mm2 = _mm_unpacklo_pi8(mm2, mm4); // cU
				mm3 = _mm_unpacklo_pi8(mm3, mm4); // dU
				mm3 = _mm_sub_pi16(mm3, mm2);    // dU-cU
				mm2 = _mm_slli_pi16(mm2, 3);       // 8*cU
				mm3 = _mm_mullo_pi16(mm3, mm7);   // mv_x*(dU-cU)
				mm2 = _mm_add_pi16(mm2, mm5);    // 8*cU + 4
				mm2 = _mm_add_pi16(mm2, mm3);    // 8*cU + mv_x*(dU-cU) + 4
				mm2 = _mm_srai_pi16(mm2, 3);       // (8*cU + mv_x*(dU-cU) + 4)>>3
				mm2 = _mm_packs_pu16(mm2, mm2);  // Words -> bytes
				*((DWORD*)&pDstUV[0]) = _m_to_int(mm2);

				mm0 = *((__m64*)&pSrcUV[8]);
				mm1 = *((__m64*)&pSrcUV[10]);
				mm0 = _mm_unpacklo_pi8(mm0, mm4); // cV
				mm1 = _mm_unpacklo_pi8(mm1, mm4); // dV
				mm1 = _mm_sub_pi16(mm1, mm0);    // dV-cV
				mm0 = _mm_slli_pi16(mm0, 3);       // 8*cV
				mm1 = _mm_mullo_pi16(mm1, mm7);   // mv_x*(dV-cV)
				mm0 = _mm_add_pi16(mm0, mm5);    // 8*cV + 4
				mm0 = _mm_add_pi16(mm0, mm1);    // 8*cV + mv_x*(dV-cV) + 4
				mm0 = _mm_srai_pi16(mm0, 3);       // (8*cV + mv_x*(dV-cV) + 4)>>3
				mm0 = _mm_packs_pu16(mm0, mm0);  // Words -> bytes
				*((DWORD*)&pDstUV[8]) = _m_to_int(mm0);

				mm2 = *((__m64*)&pSrcUV[stride_src]);
				mm3 = *((__m64*)&pSrcUV[stride_src+2]);
				mm2 = _mm_unpacklo_pi8(mm2, mm4); // cU
				mm3 = _mm_unpacklo_pi8(mm3, mm4); // dU
				mm3 = _mm_sub_pi16(mm3, mm2);    // dU-cU
				mm2 = _mm_slli_pi16(mm2, 3);       // 8*cU
				mm3 = _mm_mullo_pi16(mm3, mm7);   // mv_x*(dU-cU)
				mm2 = _mm_add_pi16(mm2, mm5);    // 8*cU + 4
				mm2 = _mm_add_pi16(mm2, mm3);    // 8*cU + mv_x*(dU-cU) + 4
				mm2 = _mm_srai_pi16(mm2, 3);       // (8*cU + mv_x*(dU-cU) + 4)>>3
				mm2 = _mm_packs_pu16(mm2, mm2);  // Words -> bytes
				*((DWORD*)&pDstUV[stride_dst]) = _m_to_int(mm2);

				mm0 = *((__m64*)&pSrcUV[stride_src+8]);
				mm1 = *((__m64*)&pSrcUV[stride_src+10]);
				mm0 = _mm_unpacklo_pi8(mm0, mm4); // cV
				mm1 = _mm_unpacklo_pi8(mm1, mm4); // dV
				mm1 = _mm_sub_pi16(mm1, mm0);    // dV-cV
				mm0 = _mm_slli_pi16(mm0, 3);       // 8*cV
				mm1 = _mm_mullo_pi16(mm1, mm7);   // mv_x*(dV-cV)
				mm0 = _mm_add_pi16(mm0, mm5);    // 8*cV + 4
				mm0 = _mm_add_pi16(mm0, mm1);    // 8*cV + mv_x*(dV-cV) + 4
				mm0 = _mm_srai_pi16(mm0, 3);       // (8*cV + mv_x*(dV-cV) + 4)>>3
				mm0 = _mm_packs_pu16(mm0, mm0);  // Words -> bytes
				*((DWORD*)&pDstUV[stride_dst+8]) = _m_to_int(mm0);

				//  2
				mm2 = *((__m64*)&pSrcUV[0]);
				mm3 = *((__m64*)&pSrcUV[2]);
				mm2 = _mm_unpackhi_pi8(mm2, mm4); // cU
				mm3 = _mm_unpackhi_pi8(mm3, mm4); // dU
				mm3 = _mm_sub_pi16(mm3, mm2);    // dU-cU
				mm2 = _mm_slli_pi16(mm2, 3);       // 8*cU
				mm3 = _mm_mullo_pi16(mm3, mm7);   // mv_x*(dU-cU)
				mm2 = _mm_add_pi16(mm2, mm5);    // 8*cU + 4
				mm2 = _mm_add_pi16(mm2, mm3);    // 8*cU + mv_x*(dU-cU) + 4
				mm2 = _mm_srai_pi16(mm2, 3);       // (8*cU + mv_x*(dU-cU) + 4)>>3
				mm2 = _mm_packs_pu16(mm2, mm2);  // Words -> bytes
				*((DWORD*)&pDstUV[4]) = _m_to_int(mm2);

				mm0 = *((__m64*)&pSrcUV[8]);
				mm1 = *((__m64*)&pSrcUV[10]);
				mm0 = _mm_unpackhi_pi8(mm0, mm4); // cV
				mm1 = _mm_unpackhi_pi8(mm1, mm4); // dV
				mm1 = _mm_sub_pi16(mm1, mm0);    // dV-cV
				mm0 = _mm_slli_pi16(mm0, 3);       // 8*cV
				mm1 = _mm_mullo_pi16(mm1, mm7);   // mv_x*(dV-cV)
				mm0 = _mm_add_pi16(mm0, mm5);    // 8*cV + 4
				mm0 = _mm_add_pi16(mm0, mm1);    // 8*cV + mv_x*(dV-cV) + 4
				mm0 = _mm_srai_pi16(mm0, 3);       // (8*cV + mv_x*(dV-cV) + 4)>>3
				mm0 = _mm_packs_pu16(mm0, mm0);  // Words -> bytes
				*((DWORD*)&pDstUV[12]) = _m_to_int(mm0);

				mm2 = *((__m64*)&pSrcUV[stride_src]);
				mm3 = *((__m64*)&pSrcUV[stride_src+2]);
				mm2 = _mm_unpackhi_pi8(mm2, mm4); // cU
				mm3 = _mm_unpackhi_pi8(mm3, mm4); // dU
				mm3 = _mm_sub_pi16(mm3, mm2);    // dU-cU
				mm2 = _mm_slli_pi16(mm2, 3);       // 8*cU
				mm3 = _mm_mullo_pi16(mm3, mm7);   // mv_x*(dU-cU)
				mm2 = _mm_add_pi16(mm2, mm5);    // 8*cU + 4
				mm2 = _mm_add_pi16(mm2, mm3);    // 8*cU + mv_x*(dU-cU) + 4
				mm2 = _mm_srai_pi16(mm2, 3);       // (8*cU + mv_x*(dU-cU) + 4)>>3
				mm2 = _mm_packs_pu16(mm2, mm2);  // Words -> bytes
				*((DWORD*)&pDstUV[stride_dst+4]) = _m_to_int(mm2);

				mm0 = *((__m64*)&pSrcUV[stride_src+8]);
				mm1 = *((__m64*)&pSrcUV[stride_src+10]);
				mm0 = _mm_unpackhi_pi8(mm0, mm4); // cV
				mm1 = _mm_unpackhi_pi8(mm1, mm4); // dV
				mm1 = _mm_sub_pi16(mm1, mm0);    // dV-cV
				mm0 = _mm_slli_pi16(mm0, 3);       // 8*cV
				mm1 = _mm_mullo_pi16(mm1, mm7);   // mv_x*(dV-cV)
				mm0 = _mm_add_pi16(mm0, mm5);    // 8*cV + 4
				mm0 = _mm_add_pi16(mm0, mm1);    // 8*cV + mv_x*(dV-cV) + 4
				mm0 = _mm_srai_pi16(mm0, 3);       // (8*cV + mv_x*(dV-cV) + 4)>>3
				mm0 = _mm_packs_pu16(mm0, mm0);  // Words -> bytes
				*((DWORD*)&pDstUV[stride_dst+12]) = _m_to_int(mm0);


				pSrcUV  += 2*stride_src;
				pDstUV  += 2*stride_dst;
			}
		}
		else
		{
			mm4 = _mm_setzero_si64();
			mm5 = *((__m64*)&const_val[0][0]);
			mm6 = *((__m64*)&const_val[jf1][0]);
			mm7 = *((__m64*)&const_val[if1][0]);
			
			
			mm0 = *((__m64*)&pSrcUV[0]);
			mm1 = *((__m64*)&pSrcUV[2]);
			mm0 = _mm_unpacklo_pi8(mm0, mm4); // aU
			mm1 = _mm_unpacklo_pi8(mm1, mm4); // bU
			mm1 = _mm_sub_pi16(mm1, mm0);    // bU-aU
			mm0 = _mm_slli_pi16(mm0, 3);       // 8*aU
			mm1 = _mm_mullo_pi16(mm1, mm7);   // mv_x*(bU-aU)
			mm0 = _mm_add_pi16(mm0, mm1);    // 8*aU+mv_x*(bU-aU) = ABU

			mm2 = *((__m64*)&pSrcUV[8]);
			mm3 = *((__m64*)&pSrcUV[10]);
			mm2 = _mm_unpacklo_pi8(mm2, mm4); // aV
			mm3 = _mm_unpacklo_pi8(mm3, mm4); // bV
			mm3 = _mm_sub_pi16(mm3, mm2);    // bV-aV
			mm2 = _mm_slli_pi16(mm2, 3);       // 8*aV
			mm3 = _mm_mullo_pi16(mm3, mm7);   // mv_x*(bV-aV)
			mm2 = _mm_add_pi16(mm2, mm3);    // 8*aU+mv_x*(bV-aV) = ABV
			for(count=H;count>0;count-=2)
			{
				number++;
				mm1 = *((__m64*)&pSrcUV[stride_src]);
				mm3 = *((__m64*)&pSrcUV[stride_src+2]);
				mm1 = _mm_unpacklo_pi8(mm1, mm4); // cU
				mm3 = _mm_unpacklo_pi8(mm3, mm4); // dU
				mm3 = _mm_sub_pi16(mm3, mm1);    // dU-cU
				mm1 = _mm_slli_pi16(mm1, 3);       // 8*cU
				mm3 = _mm_mullo_pi16(mm3, mm7);   // mv_x*(dU-cU)
				mm1 = _mm_add_pi16(mm1, mm3);    // 8*cU+mv_x*(dU-cU) = CDU
				mm3 = mm1;                          // 8*cU+mv_x*(dU-cU) = CDU - need to save
				mm1 = _mm_sub_pi16(mm1, mm0);    // CDU-ABU
				mm0 = _mm_slli_pi16(mm0, 3);       // 8*ABU
				mm1 = _mm_mullo_pi16(mm1, mm6);   // mv_y*(CDU-ABU)
				mm0 = _mm_add_pi16(mm0, mm5);    // 8*ABU + 32
				mm0 = _mm_add_pi16(mm0, mm1);    // 8*ABU + mv_y*(CDU-ABU) + 32
				mm0 = _mm_srai_pi16(mm0, 6);       // (8*ABU + mv_y*(CDU-ABU) + 32)>>6
				mm0 = _mm_packs_pu16(mm0, mm0);  // Words -> bytes
				*((DWORD*)&pDstUV[0]) = _m_to_int(mm0);

				mm0 = *((__m64*)&pSrcUV[stride_src+8]);
				mm1 = *((__m64*)&pSrcUV[stride_src+10]);
				mm0 = _mm_unpacklo_pi8(mm0, mm4); // cV
				mm1 = _mm_unpacklo_pi8(mm1, mm4); // dV
				mm1 = _mm_sub_pi16(mm1, mm0);    // dV-cV
				mm0 = _mm_slli_pi16(mm0, 3);       // 8*cV
				mm1 = _mm_mullo_pi16(mm1, mm7);   // mv_x*(dV-cV)
				mm0 = _mm_add_pi16(mm0, mm1);    // 8*cV+mv_x*(dV-cV) = CDV
				mm1 = mm0;                          // 8*cV+mv_x*(dV-cV) = CDV - need to save
				mm0 = _mm_sub_pi16(mm0, mm2);    // CDV-ABV
				mm2 = _mm_slli_pi16(mm2, 3);       // 8*ABV
				mm0 = _mm_mullo_pi16(mm0, mm6);   // mv_y*(CDV-ABV)
				mm2 = _mm_add_pi16(mm2, mm5);    // 8*ABV + 32
				mm2 = _mm_add_pi16(mm2, mm0);    // 8*ABV + mv_y*(CDV-ABV) + 32
				mm2 = _mm_srai_pi16(mm2, 6);       // (8*ABV + mv_y*(CDV-ABV) + 32)>>6
				mm2 = _mm_packs_pu16(mm2, mm2);  // Words -> bytes
				*((DWORD*)&pDstUV[8]) = _m_to_int(mm2);

				mm2 = *((__m64*)&pSrcUV[2*stride_src]);
				mm0 = *((__m64*)&pSrcUV[2*stride_src+2]);
				mm2 = _mm_unpacklo_pi8(mm2, mm4); // cU
				mm0 = _mm_unpacklo_pi8(mm0, mm4); // dU
				mm0 = _mm_sub_pi16(mm0, mm2);    // dU-cU
				mm2 = _mm_slli_pi16(mm2, 3);       // 8*cU
				mm0 = _mm_mullo_pi16(mm0, mm7);   // mv_x*(dU-cU)
				mm2 = _mm_add_pi16(mm2, mm0);    // 8*cU+mv_x*(dU-cU) = CDU
				mm0 = mm2;                          // 8*cU+mv_x*(dU-cU) = CDU - need to save
				mm2 = _mm_sub_pi16(mm2, mm3);    // CDU-ABU
				mm3 = _mm_slli_pi16(mm3, 3);       // 8*ABU
				mm2 = _mm_mullo_pi16(mm2, mm6);   // mv_y*(CDU-ABU)
				mm3 = _mm_add_pi16(mm3, mm5);    // 8*ABU + 32
				mm3 = _mm_add_pi16(mm3, mm2);    // 8*ABU + mv_y*(CDU-ABU) + 32
				mm3 = _mm_srai_pi16(mm3, 6);       // (8*ABU + mv_y*(CDU-ABU) + 32)>>6
				mm3 = _mm_packs_pu16(mm3, mm3);  // Words -> bytes
				*((DWORD*)&pDstUV[stride_dst]) = _m_to_int(mm3);

				mm3 = *((__m64*)&pSrcUV[2*stride_src+8]);
				mm2 = *((__m64*)&pSrcUV[2*stride_src+10]);
				mm3 = _mm_unpacklo_pi8(mm3, mm4); // cV
				mm2 = _mm_unpacklo_pi8(mm2, mm4); // dV
				mm2 = _mm_sub_pi16(mm2, mm3);    // dV-cV
				mm3 = _mm_slli_pi16(mm3, 3);       // 8*cV
				mm2 = _mm_mullo_pi16(mm2, mm7);   // mv_x*(dV-cV)
				mm3 = _mm_add_pi16(mm3, mm2);    // 8*cV+mv_x*(dV-cV) = CDV
				mm2 = mm3;                          // 8*cV+mv_x*(dV-cV) = CDV - need to save
				mm3 = _mm_sub_pi16(mm3, mm1);    // CDV-ABV
				mm1 = _mm_slli_pi16(mm1, 3);       // 8*ABV
				mm3 = _mm_mullo_pi16(mm3, mm6);   // mv_y*(CDV-ABV)
				mm1 = _mm_add_pi16(mm1, mm5);    // 8*ABV + 32
				mm1 = _mm_add_pi16(mm1, mm3);    // 8*ABV + mv_y*(CDV-ABV) + 32
				mm1 = _mm_srai_pi16(mm1, 6);       // (8*ABV + mv_y*(CDV-ABV) + 32)>>6
				mm1 = _mm_packs_pu16(mm1, mm1);  // Words -> bytes
				*((DWORD*)&pDstUV[stride_dst+8]) = _m_to_int(mm1);

				pSrcUV  += 2*stride_src;
				pDstUV  += 2*stride_dst;
			}

			pSrcUV -= 2*stride_src*number;
			pDstUV -= 2*stride_dst*number;
			number=0;			
			// 2
			mm0 = *((__m64*)&pSrcUV[0]);
			mm1 = *((__m64*)&pSrcUV[2]);
			mm0 = _mm_unpackhi_pi8(mm0, mm4); // aU
			mm1 = _mm_unpackhi_pi8(mm1, mm4); // bU
			mm1 = _mm_sub_pi16(mm1, mm0);    // bU-aU
			mm0 = _mm_slli_pi16(mm0, 3);       // 8*aU
			mm1 = _mm_mullo_pi16(mm1, mm7);   // mv_x*(bU-aU)
			mm0 = _mm_add_pi16(mm0, mm1);    // 8*aU+mv_x*(bU-aU) = ABU

			mm2 = *((__m64*)&pSrcUV[8]);
			mm3 = *((__m64*)&pSrcUV[10]);
			mm2 = _mm_unpackhi_pi8(mm2, mm4); // aV
			mm3 = _mm_unpackhi_pi8(mm3, mm4); // bV
			mm3 = _mm_sub_pi16(mm3, mm2);    // bV-aV
			mm2 = _mm_slli_pi16(mm2, 3);       // 8*aV
			mm3 = _mm_mullo_pi16(mm3, mm7);   // mv_x*(bV-aV)
			mm2 = _mm_add_pi16(mm2, mm3);    // 8*aU+mv_x*(bV-aV) = ABV
			for(count=H;count>0;count-=2)
			{
				mm1 = *((__m64*)&pSrcUV[stride_src]);
				mm3 = *((__m64*)&pSrcUV[stride_src+2]);
				mm1 = _mm_unpackhi_pi8(mm1, mm4); // cU
				mm3 = _mm_unpackhi_pi8(mm3, mm4); // dU
				mm3 = _mm_sub_pi16(mm3, mm1);    // dU-cU
				mm1 = _mm_slli_pi16(mm1, 3);       // 8*cU
				mm3 = _mm_mullo_pi16(mm3, mm7);   // mv_x*(dU-cU)
				mm1 = _mm_add_pi16(mm1, mm3);    // 8*cU+mv_x*(dU-cU) = CDU
				mm3 = mm1;                          // 8*cU+mv_x*(dU-cU) = CDU - need to save
				mm1 = _mm_sub_pi16(mm1, mm0);    // CDU-ABU
				mm0 = _mm_slli_pi16(mm0, 3);       // 8*ABU
				mm1 = _mm_mullo_pi16(mm1, mm6);   // mv_y*(CDU-ABU)
				mm0 = _mm_add_pi16(mm0, mm5);    // 8*ABU + 32
				mm0 = _mm_add_pi16(mm0, mm1);    // 8*ABU + mv_y*(CDU-ABU) + 32
				mm0 = _mm_srai_pi16(mm0, 6);       // (8*ABU + mv_y*(CDU-ABU) + 32)>>6
				mm0 = _mm_packs_pu16(mm0, mm0);  // Words -> bytes
				*((DWORD*)&pDstUV[4]) = _m_to_int(mm0);

				mm0 = *((__m64*)&pSrcUV[stride_src+8]);
				mm1 = *((__m64*)&pSrcUV[stride_src+10]);
				mm0 = _mm_unpackhi_pi8(mm0, mm4); // cV
				mm1 = _mm_unpackhi_pi8(mm1, mm4); // dV
				mm1 = _mm_sub_pi16(mm1, mm0);    // dV-cV
				mm0 = _mm_slli_pi16(mm0, 3);       // 8*cV
				mm1 = _mm_mullo_pi16(mm1, mm7);   // mv_x*(dV-cV)
				mm0 = _mm_add_pi16(mm0, mm1);    // 8*cV+mv_x*(dV-cV) = CDV
				mm1 = mm0;                          // 8*cV+mv_x*(dV-cV) = CDV - need to save
				mm0 = _mm_sub_pi16(mm0, mm2);    // CDV-ABV
				mm2 = _mm_slli_pi16(mm2, 3);       // 8*ABV
				mm0 = _mm_mullo_pi16(mm0, mm6);   // mv_y*(CDV-ABV)
				mm2 = _mm_add_pi16(mm2, mm5);    // 8*ABV + 32
				mm2 = _mm_add_pi16(mm2, mm0);    // 8*ABV + mv_y*(CDV-ABV) + 32
				mm2 = _mm_srai_pi16(mm2, 6);       // (8*ABV + mv_y*(CDV-ABV) + 32)>>6
				mm2 = _mm_packs_pu16(mm2, mm2);  // Words -> bytes
				*((DWORD*)&pDstUV[12]) = _m_to_int(mm2);

				mm2 = *((__m64*)&pSrcUV[2*stride_src]);
				mm0 = *((__m64*)&pSrcUV[2*stride_src+2]);
				mm2 = _mm_unpackhi_pi8(mm2, mm4); // cU
				mm0 = _mm_unpackhi_pi8(mm0, mm4); // dU
				mm0 = _mm_sub_pi16(mm0, mm2);    // dU-cU
				mm2 = _mm_slli_pi16(mm2, 3);       // 8*cU
				mm0 = _mm_mullo_pi16(mm0, mm7);   // mv_x*(dU-cU)
				mm2 = _mm_add_pi16(mm2, mm0);    // 8*cU+mv_x*(dU-cU) = CDU
				mm0 = mm2;                          // 8*cU+mv_x*(dU-cU) = CDU - need to save
				mm2 = _mm_sub_pi16(mm2, mm3);    // CDU-ABU
				mm3 = _mm_slli_pi16(mm3, 3);       // 8*ABU
				mm2 = _mm_mullo_pi16(mm2, mm6);   // mv_y*(CDU-ABU)
				mm3 = _mm_add_pi16(mm3, mm5);    // 8*ABU + 32
				mm3 = _mm_add_pi16(mm3, mm2);    // 8*ABU + mv_y*(CDU-ABU) + 32
				mm3 = _mm_srai_pi16(mm3, 6);       // (8*ABU + mv_y*(CDU-ABU) + 32)>>6
				mm3 = _mm_packs_pu16(mm3, mm3);  // Words -> bytes
				*((DWORD*)&pDstUV[stride_dst+4]) = _m_to_int(mm3);

				mm3 = *((__m64*)&pSrcUV[2*stride_src+8]);
				mm2 = *((__m64*)&pSrcUV[2*stride_src+10]);
				mm3 = _mm_unpackhi_pi8(mm3, mm4); // cV
				mm2 = _mm_unpackhi_pi8(mm2, mm4); // dV
				mm2 = _mm_sub_pi16(mm2, mm3);    // dV-cV
				mm3 = _mm_slli_pi16(mm3, 3);       // 8*cV
				mm2 = _mm_mullo_pi16(mm2, mm7);   // mv_x*(dV-cV)
				mm3 = _mm_add_pi16(mm3, mm2);    // 8*cV+mv_x*(dV-cV) = CDV
				mm2 = mm3;                          // 8*cV+mv_x*(dV-cV) = CDV - need to save
				mm3 = _mm_sub_pi16(mm3, mm1);    // CDV-ABV
				mm1 = _mm_slli_pi16(mm1, 3);       // 8*ABV
				mm3 = _mm_mullo_pi16(mm3, mm6);   // mv_y*(CDV-ABV)
				mm1 = _mm_add_pi16(mm1, mm5);    // 8*ABV + 32
				mm1 = _mm_add_pi16(mm1, mm3);    // 8*ABV + mv_y*(CDV-ABV) + 32
				mm1 = _mm_srai_pi16(mm1, 6);       // (8*ABV + mv_y*(CDV-ABV) + 32)>>6
				mm1 = _mm_packs_pu16(mm1, mm1);  // Words -> bytes
				*((DWORD*)&pDstUV[stride_dst+12]) = _m_to_int(mm1);

				pSrcUV  += 2*stride_src;
				pDstUV  += 2*stride_dst;
			}
		}
	}
}





#endif //H264_ENABLE_INTRINSICS
