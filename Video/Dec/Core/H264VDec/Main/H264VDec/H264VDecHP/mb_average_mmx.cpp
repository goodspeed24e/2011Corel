/*!
***************************************************************************
* \file
*    mb_average_mmx.cpp
*
* \brief
*    MMX version file for two macrblocks/sub-macroblocks average
*
***************************************************************************
*/
#include "global.h"
#ifdef H264_ENABLE_INTRINSICS
#include <mmintrin.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include "mb_average.h"

// Disable "No EMMS at end of function '<function name>'"
#pragma warning ( disable : 4799 )

#ifdef H264_ENABLE_ASM
void average_16_mmx(imgpel *output, 
										int stride_out,
										imgpel *input1,
										imgpel *input2,
										int stride_in,
										int height)
{
	__asm
	{
		mov eax, input1;
		mov ebx, input2;
		mov ecx, stride_in;
		mov esi, output;
		mov edx, stride_out;
		mov edi, height;

LOOP16:
		movq mm0, [eax];
		movq mm1, [eax+8];
		movq mm2, [eax+ecx];
		movq mm3, [eax+ecx+8];
		movq mm4, [ebx];
		movq mm5, [ebx+8];
		movq mm6, [ebx+ecx];
		movq mm7, [ebx+ecx+8];
		pavgb mm0, mm4;
		pavgb mm1, mm5;
		pavgb mm2, mm6;
		pavgb mm3, mm7;
		movq [esi], mm0;
		movq [esi+8], mm1;
		movq [esi+edx], mm2;
		movq [esi+edx+8], mm3;

		lea eax, [eax+2*ecx];
		lea ebx, [ebx+2*ecx];
		lea esi, [esi+2*edx];
		sub edi, 2;
		jnz LOOP16;

	}
}

void average_8_mmx(imgpel *output, 
									 int stride_out,
									 imgpel *input1,
									 imgpel *input2,
									 int stride_in,
									 int height)
{
	__asm
	{
		mov eax, input1;
		mov ebx, input2;
		mov ecx, stride_in;
		mov esi, output;
		mov edx, stride_out;
		mov edi, height;
LOOP8:
		movq mm0, [eax];
		movq mm2, [eax+ecx];
		movq mm4, [ebx];
		movq mm6, [ebx+ecx];
		pavgb mm0, mm4;
		pavgb mm2, mm6;
		movq [esi], mm0;
		movq [esi+edx], mm2;
		lea eax, [eax+2*ecx];
		lea ebx, [ebx+2*ecx];
		lea esi, [esi+2*edx];
		sub edi, 2;
		jnz LOOP8;

	}
}

void average_4_mmx(imgpel *output, 
									 int stride_out,
									 imgpel *input1,
									 imgpel *input2,
									 int stride_in,
									 int height)
{
	__asm
	{
		mov eax, input1;
		mov ebx, input2;
		mov ecx, stride_in;
		mov esi, output;
		mov edx, stride_out;
		mov edi, height;
LOOP4:
		movd mm0, [eax];
		movd mm2, [eax+ecx];
		movd mm4, [ebx];
		movd mm6, [ebx+ecx];
		pavgb mm0, mm4;
		pavgb mm2, mm6;
		movd [esi], mm0;
		movd [esi+edx], mm2;
		lea eax, [eax+2*ecx];
		lea ebx, [ebx+2*ecx];
		lea esi, [esi+2*edx];
		sub edi, 2;
		jnz LOOP4;

	}
}

void average_2_mmx(imgpel *output, 
									 int stride_out,
									 imgpel *input1,
									 imgpel *input2,
									 int stride_in,
									 int height)
{
	__asm
	{
		mov eax, input1;
		mov ebx, input2;
		mov ecx, stride_in;
		mov esi, output;
		mov edi, stride_out;
LOOP2:
		movd mm0, [eax];
		movd mm2, [eax+ecx];
		movd mm4, [ebx];
		movd mm6, [ebx+ecx];
		pavgb mm0, mm4;
		pavgb mm2, mm6;
		movd edx, mm0;
		mov [esi], dx;
		movd edx, mm2;
		mov [esi+edi], dx;

		lea eax, [eax+2*ecx];
		lea ebx, [ebx+2*ecx];
		lea esi, [esi+2*edi];
		//mov edx, height;
		//sub edx, 2;
		sub height, 2;
		jnz LOOP2;

	}
}
#endif //H264_ENABLE_ASM

void weight_16_sse2(imgpel *input_image, int weight, int rounding_offset, unsigned int down_shift, int final_offset, int stride_in, int height)
{

	int i;
	__m128i xmm0, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7;

	xmm0 = _mm_setzero_si128();
	//xmm1 = _mm_set1_epi16(255);
	xmm2 = _mm_set1_epi16((short) weight);
	xmm3 = _mm_set1_epi16((short) rounding_offset);
	xmm4 = _mm_set1_epi16((short) final_offset);

	for (i=0; i<height; i++)
	{
		xmm5 = _mm_load_si128((__m128i*)input_image);	
		xmm6 = _mm_unpacklo_epi8(xmm5, xmm0);
		xmm7 = _mm_unpackhi_epi8(xmm5, xmm0);	

		xmm6 = _mm_mullo_epi16(xmm6, xmm2);
		xmm7 = _mm_mullo_epi16(xmm7, xmm2);

		xmm6 = _mm_add_epi16(xmm6, xmm3);
		xmm7 = _mm_add_epi16(xmm7, xmm3);

		xmm6 = _mm_srai_epi16(xmm6, down_shift);
		xmm7 = _mm_srai_epi16(xmm7, down_shift);

		xmm6 = _mm_add_epi16(xmm6, xmm4);
		xmm7 = _mm_add_epi16(xmm7, xmm4);

		xmm5 = _mm_packus_epi16(xmm6, xmm7);  //clipping

		_mm_store_si128((__m128i*)input_image, xmm5);

		input_image += stride_in;
	}	
}

void weight_16_b_sse2(imgpel *output, 
											int stride_out,
											imgpel *input1, imgpel *input2, 
											int weight1, int weight2, 
											int rounding_offset, unsigned int down_shift, int final_offset, 
											int stride_in, int height)
{

	int i;
	__m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9;

	xmm0 = _mm_setzero_si128();
	xmm1 = _mm_set1_epi16((short) weight1);
	xmm2 = _mm_set1_epi16((short) weight2);
	xmm3 = _mm_set1_epi16((short) rounding_offset);
	xmm4 = _mm_set1_epi16((short) final_offset);

	for (i=0; i<height; i++)
	{
		xmm5 = _mm_load_si128((__m128i*)input1);	
		xmm6 = _mm_unpacklo_epi8(xmm5, xmm0);
		xmm7 = _mm_unpackhi_epi8(xmm5, xmm0);
		xmm5 = _mm_load_si128((__m128i*)input2);
		xmm8 = _mm_unpacklo_epi8(xmm5, xmm0);
		xmm9 = _mm_unpackhi_epi8(xmm5, xmm0);


		xmm6 = _mm_mullo_epi16(xmm6, xmm1);
		xmm8 = _mm_mullo_epi16(xmm8, xmm2);
		xmm7 = _mm_mullo_epi16(xmm7, xmm1);		
		xmm9 = _mm_mullo_epi16(xmm9, xmm2);

		xmm6 = _mm_add_epi16(xmm6, xmm8);
		xmm7 = _mm_add_epi16(xmm7, xmm9);

		xmm6 = _mm_add_epi16(xmm6, xmm3);
		xmm7 = _mm_add_epi16(xmm7, xmm3);

		xmm6 = _mm_srai_epi16(xmm6, down_shift);
		xmm7 = _mm_srai_epi16(xmm7, down_shift);

		xmm6 = _mm_add_epi16(xmm6, xmm4);
		xmm7 = _mm_add_epi16(xmm7, xmm4);

		xmm5 = _mm_packus_epi16(xmm6, xmm7);  //clipping

		_mm_store_si128((__m128i*)output, xmm5);

		output += stride_out;
		input1 += stride_in;
		input2 += stride_in;
	}
}

void weight_8_sse2(imgpel *input_image, int weight, int rounding_offset, unsigned int down_shift, int final_offset, int stride_in, int height)
{

	int i;
	__m128i xmm0, xmm2, xmm3, xmm4, xmm5, xmm6;

	xmm0 = _mm_setzero_si128();
	//xmm1 = _mm_set1_epi16(255);
	xmm2 = _mm_set1_epi16((short) weight);
	xmm3 = _mm_set1_epi16((short) rounding_offset);
	xmm4 = _mm_set1_epi16((short) final_offset);

	for (i=0; i<height; i++)
	{
		xmm5 = _mm_loadl_epi64((__m128i*)input_image);	
		xmm6 = _mm_unpacklo_epi8(xmm5, xmm0);
		xmm6 = _mm_mullo_epi16(xmm6, xmm2);
		xmm6 = _mm_add_epi16(xmm6, xmm3);
		xmm6 = _mm_srai_epi16(xmm6, down_shift);
		xmm6 = _mm_add_epi16(xmm6, xmm4);
		xmm5 = _mm_packus_epi16(xmm6, xmm0);  //clipping
		_mm_storel_epi64((__m128i*)input_image, xmm5);
		input_image += stride_in;
	}	
}

void weight_8_b_sse2(imgpel *output, 
										 int stride_out,
										 imgpel *input1, imgpel *input2, 
										 int weight1, int weight2, 
										 int rounding_offset, unsigned int down_shift, int final_offset, 
										 int stride_in, int height)
{

	int i;
	__m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7;

	xmm0 = _mm_setzero_si128();
	xmm1 = _mm_set1_epi16((short) weight1);
	xmm2 = _mm_set1_epi16((short) weight2);
	xmm3 = _mm_set1_epi16((short) rounding_offset);
	xmm4 = _mm_set1_epi16((short) final_offset);

	for (i=0; i<height; i++)
	{
		xmm5 = _mm_loadl_epi64((__m128i*)input1);	
		xmm6 = _mm_unpacklo_epi8(xmm5, xmm0);
		xmm5 = _mm_loadl_epi64((__m128i*)input2);
		xmm7 = _mm_unpacklo_epi8(xmm5, xmm0);

		xmm6 = _mm_mullo_epi16(xmm6, xmm1);
		xmm7 = _mm_mullo_epi16(xmm7, xmm2);

		xmm6 = _mm_add_epi16(xmm6, xmm7);
		xmm6 = _mm_add_epi16(xmm6, xmm3);
		xmm6 = _mm_srai_epi16(xmm6, down_shift);
		xmm6 = _mm_add_epi16(xmm6, xmm4);
		xmm5 = _mm_packus_epi16(xmm6, xmm0);  //clipping
		_mm_storel_epi64((__m128i*)output, xmm5);

		input1 += stride_in;
		input2 += stride_in;
		output+=stride_out;
	}	
}


void weight_16_mmx(imgpel *input_image, int weight, int rounding_offset, unsigned int down_shift, int final_offset, int stride_in, int height)
{
	int i;
	__m64 mm0, mm1, mm2, mm3, mm4, mm5, mm6;

	mm0 = _mm_setzero_si64();  //se mm0 to zero
	mm2 = _mm_set1_pi16 ((short) weight);
	mm3 = _mm_set1_pi16 ((short) rounding_offset);
	mm4 = _mm_set1_pi16 ((short) final_offset);

	for (i=0; i<height; i++)
	{
		mm1 = _m_from_int( *((DWORD*) input_image));
		mm6 = _mm_unpacklo_pi8(mm1, mm0);
		mm6 = _mm_mullo_pi16 (mm6, mm2);		
		mm6 = _mm_add_pi16 (mm6, mm3);
		mm6 = _mm_srai_pi16(mm6, down_shift);
		mm6 = _mm_add_pi16(mm6, mm4);

		mm1 = _m_from_int( *((DWORD*) (input_image+4)));

		mm5 = _m_packuswb(mm6, mm0);

		mm6 = _mm_unpacklo_pi8(mm1, mm0);
		mm6 = _mm_mullo_pi16 (mm6, mm2);	
		*((DWORD*) input_image) = _m_to_int(mm5);
		mm6 = _mm_add_pi16 (mm6, mm3);
		mm6 = _mm_srai_pi16(mm6, down_shift);
		mm6 = _mm_add_pi16(mm6, mm4);
		mm1 = _m_from_int( *((DWORD*) (input_image+8)));

		mm5 = _m_packuswb(mm6, mm0);
		*((DWORD*) (input_image+4)) = _m_to_int(mm5);

		mm6 = _mm_unpacklo_pi8(mm1, mm0);
		mm6 = _mm_mullo_pi16 (mm6, mm2);		
		mm6 = _mm_add_pi16 (mm6, mm3);
		mm6 = _mm_srai_pi16(mm6, down_shift);
		mm1 = _m_from_int( *((DWORD*) (input_image+12)));
		mm6 = _mm_add_pi16(mm6, mm4);
		mm5 = _m_packuswb(mm6, mm0);
		*((DWORD*) (input_image+8)) = _m_to_int(mm5);


		mm6 = _mm_unpacklo_pi8(mm1, mm0);
		mm6 = _mm_mullo_pi16 (mm6, mm2);		
		mm6 = _mm_add_pi16 (mm6, mm3);
		mm6 = _mm_srai_pi16(mm6, down_shift);
		mm6 = _mm_add_pi16(mm6, mm4);
		mm5 = _m_packuswb(mm6, mm0);
		*((DWORD*) (input_image+12)) = _m_to_int(mm5);

		input_image += stride_in;
	}
}

void weight_8_mmx(imgpel *input_image, int weight, int rounding_offset, unsigned int down_shift, int final_offset, int stride_in, int height)
{
	/*
	__asm
	{		
	pxor mm0, mm0;
	mov esi, input_image;
	movsx eax, weight;
	movd mm2, eax;
	mov ebx, rounding_offset;
	movd mm3, ebx;
	mov ecx, down_shift;
	movd mm4, ecx;
	mov ecx, final_offset;
	movd mm5, ecx;
	mov edx, stride_in;
	mov edi, height;
	punpcklwd mm2, mm2;  
	punpcklwd mm2, mm2;  //0w 0w 0w 0w 
	punpcklwd mm3, mm3;  
	punpcklwd mm3, mm3;  //0r 0r 0r 0r 
	punpcklwd mm5, mm5;
	punpcklwd mm5, mm5;  //0f 0f 0f 0f

	LOOP8:		
	movq mm1, [esi];
	movq mm6, mm1;		
	punpcklbw mm1, mm0;				
	pmullw mm1, mm2;
	punpckhbw mm6, mm0;
	paddw mm1, mm3;
	pmullw mm6, mm2;		
	paddw mm6, mm3;
	psraw mm1, mm4;
	psraw mm6, mm4;
	paddw mm1, mm5;
	packuswb mm1, mm0;
	paddw mm6, mm5;
	packuswb mm6, mm0;
	punpckldq mm1, mm6;
	movq [esi], mm1;
	lea esi, [esi+edx];
	sub edi, 1;
	jnz LOOP8;
	}
	*/
	int i;
	__m64 mm0, mm2, mm3, mm4, mm5, mm6;

	mm0 = _mm_setzero_si64();  //se mm0 to zero
	mm2 = _mm_set1_pi16 ((short) weight);
	mm3 = _mm_set1_pi16 ((short) rounding_offset);
	mm4 = _mm_set1_pi16 ((short) final_offset);

	for (i=0; i<height; i++)
	{
		mm5 = _m_from_int( *((DWORD*) input_image));
		mm6 = _mm_unpacklo_pi8(mm5, mm0);
		mm6 = _mm_mullo_pi16 (mm6, mm2);		
		mm6 = _mm_add_pi16 (mm6, mm3);
		mm6 = _mm_srai_pi16(mm6, down_shift);
		mm6 = _mm_add_pi16(mm6, mm4);
		mm5 = _m_packuswb(mm6, mm0);
		*((DWORD*) input_image) = _m_to_int(mm5);

		mm5 = _m_from_int( *((DWORD*) (input_image+4)));
		mm6 = _mm_unpacklo_pi8(mm5, mm0);
		mm6 = _mm_mullo_pi16 (mm6, mm2);		
		mm6 = _mm_add_pi16 (mm6, mm3);
		mm6 = _mm_srai_pi16(mm6, down_shift);
		mm6 = _mm_add_pi16(mm6, mm4);
		mm5 = _m_packuswb(mm6, mm0);
		*((DWORD*) (input_image+4)) = _m_to_int(mm5);

		input_image += stride_in;
	}
}

void weight_8_sse(imgpel *input_image, int weight, int rounding_offset, unsigned int down_shift, int final_offset, int stride_in, int height)
{
	int i;
	__m64 mm0, mm2, mm3, mm4, mm5, mm6;

	mm0 = _mm_setzero_si64();  //se mm0 to zero
	mm2 = _mm_set1_pi16 ((short) weight);
	mm3 = _mm_set1_pi16 ((short) rounding_offset);
	mm4 = _mm_set1_pi16 ((short) final_offset);

	for (i=0; i<height; i++)
	{
		mm5 = _m_from_int( *((DWORD*) input_image));
		mm6 = _mm_unpacklo_pi8(mm5, mm0);
		mm6 = _mm_mullo_pi16 (mm6, mm2);		
		mm6 = _mm_add_pi16 (mm6, mm3);
		mm6 = _mm_srai_pi16(mm6, down_shift);
		mm6 = _mm_add_pi16(mm6, mm4);
		mm5 = _m_packuswb(mm6, mm0);
		*((DWORD*) input_image) = _m_to_int(mm5);

		mm5 = _m_from_int( *((DWORD*) (input_image+4)));
		mm6 = _mm_unpacklo_pi8(mm5, mm0);
		mm6 = _mm_mullo_pi16 (mm6, mm2);		
		mm6 = _mm_add_pi16 (mm6, mm3);
		mm6 = _mm_srai_pi16(mm6, down_shift);
		mm6 = _mm_add_pi16(mm6, mm4);
		mm5 = _m_packuswb(mm6, mm0);
		*((DWORD*) (input_image+4)) = _m_to_int(mm5);

		input_image += stride_in;
	}
}

void weight_4_mmx(imgpel *input_image, int weight, int rounding_offset, unsigned int down_shift, int final_offset, int stride_in, int height)
{
#ifdef H264_ENABLE_ASM
	__asm
	{		
		pxor mm0, mm0;
		mov esi, input_image;
		movsx eax, weight;
		movd mm2, eax;
		mov ebx, rounding_offset;
		movd mm3, ebx;
		mov ecx, down_shift;
		movd mm4, ecx;
		mov ecx, final_offset;
		movd mm5, ecx;
		mov edx, stride_in;
		mov edi, height;
		punpcklwd mm2, mm2;  
		punpcklwd mm2, mm2;  //0w 0w 0w 0w 
		punpcklwd mm3, mm3;  
		punpcklwd mm3, mm3;  //0r 0r 0r 0r 
		punpcklwd mm5, mm5;
		punpcklwd mm5, mm5;  //0f 0f 0f 0f 

LOOP4:		
		movq mm1, [esi];
		punpcklbw mm1, mm0;
		pmullw mm1, mm2;
		paddw mm1, mm3;
		psraw mm1, mm4;
		paddw mm1, mm5;
		packuswb mm1, mm0;
		movd [esi], mm1;
		lea esi, [esi+edx];
		sub edi, 1;
		jnz LOOP4;
	}
#else
	int i;
	__m64 mm0, mm2, mm3, mm4, mm5, mm6;

	mm0 = _mm_setzero_si64();  //se mm0 to zero
	mm2 = _mm_set1_pi16 ((short) weight);
	mm3 = _mm_set1_pi16 ((short) rounding_offset);
	mm4 = _mm_set1_pi16 ((short) final_offset);

	for (i=0; i<height; i++)
	{
		mm5 = _m_from_int( *((DWORD*) input_image));
		mm6 = _mm_unpacklo_pi8(mm5, mm0);
		mm6 = _mm_mullo_pi16 (mm6, mm2);		
		mm6 = _mm_add_pi16 (mm6, mm3);
		mm6 = _mm_srai_pi16(mm6, down_shift);
		mm6 = _mm_add_pi16(mm6, mm4);
		mm5 = _m_packuswb(mm6, mm0);
		*((DWORD*) input_image) = _m_to_int(mm5);
		input_image += stride_in;
	}
#endif
}

void weight_2_sse(imgpel *input_image, int weight, int rounding_offset, unsigned int down_shift, int final_offset, int stride_in, int height)
{	
 	int i;
	__m64 mm0, mm2, mm3, mm4, mm5, mm6;

/*		input_image[0] = Clip1(((weight * input_image[0] + rounding_offset) >> down_shift) + final_offset);
		#define Clip1(a)      ((a)>255?255:((a)<0?0:(a))) // HP restriction

		input_image[1] = Clip1(((weight * input_image[1] + rounding_offset) >> down_shift) + final_offset);
		input_image+=stride_in; */
		
		mm0 = _mm_setzero_si64();  //se mm0 to zero
		mm2 = _mm_set1_pi16 ((short) weight);	
		mm3 = _mm_set1_pi16 ((short) rounding_offset);
		mm4 = _mm_set1_pi16 ((short) final_offset);

		for (i=0; i<height; i++)
		{
			mm5 = _m_from_int( *((DWORD*) input_image));
			mm6 = _mm_unpacklo_pi8(mm5, mm0);
			mm6 = _mm_mullo_pi16 (mm6, mm2);		
			mm6 = _mm_add_pi16 (mm6, mm3);
			mm6 = _mm_srai_pi16(mm6, down_shift);
			mm6 = _mm_add_pi16(mm6, mm4);
			mm6 = _mm_unpacklo_pi32(mm6,mm0);
			mm5 = _m_packuswb(mm6, mm0);
			*((DWORD*) input_image) = _m_to_int(mm5);
			input_image-=2;
			input_image += stride_in;
		}
	

}


void weight_16_b_mmx(imgpel *output, 
										 int stride_out,
										 imgpel *input1, imgpel *input2, 
										 int weight1, int weight2, 
										 int rounding_offset, unsigned int down_shift, int final_offset, 
										 int stride_in, int height)
{
	int i;
	__m64 mm0, mm1, mm2, mm3, mm4, mm5, mm6;

	mm0 = _mm_setzero_si64();  //se mm0 to zero
	mm1 = _mm_set1_pi16 ((short) weight1);
	mm2 = _mm_set1_pi16 ((short) weight2);
	mm3 = _mm_set1_pi16 ((short) rounding_offset);	
	mm4 = _mm_set1_pi16 ((short) final_offset);	

	for (i=0; i<height; i++)
	{
		mm5 = _m_from_int( *((DWORD*) input1));
		mm6 = _m_from_int( *((DWORD*) input2));
		mm5 = _mm_unpacklo_pi8(mm5, mm0);
		mm6 = _mm_unpacklo_pi8(mm6, mm0);
		mm5 = _mm_mullo_pi16 (mm5, mm1);
		mm6 = _mm_mullo_pi16 (mm6, mm2);
		mm5 = _mm_add_pi16 (mm5, mm6);
		mm5 = _mm_add_pi16 (mm5, mm3);
		mm5 = _mm_srai_pi16(mm5, down_shift);
		mm5 = _mm_add_pi16(mm5, mm4);
		mm5 = _m_packuswb(mm5, mm0);
		*((DWORD*) output) = _m_to_int(mm5);		

		mm5 = _m_from_int( *((DWORD*) (input1+4)));
		mm6 = _m_from_int( *((DWORD*) (input2+4)));
		mm5 = _mm_unpacklo_pi8(mm5, mm0);
		mm6 = _mm_unpacklo_pi8(mm6, mm0);
		mm5 = _mm_mullo_pi16 (mm5, mm1);
		mm6 = _mm_mullo_pi16 (mm6, mm2);
		mm5 = _mm_add_pi16 (mm5, mm6);
		mm5 = _mm_add_pi16 (mm5, mm3);
		mm5 = _mm_srai_pi16(mm5, down_shift);
		mm5 = _mm_add_pi16(mm5, mm4);
		mm5 = _m_packuswb(mm5, mm0);
		*((DWORD*) (output+4)) = _m_to_int(mm5);

		mm5 = _m_from_int( *((DWORD*) (input1+8)));
		mm6 = _m_from_int( *((DWORD*) (input2+8)));
		mm5 = _mm_unpacklo_pi8(mm5, mm0);
		mm6 = _mm_unpacklo_pi8(mm6, mm0);
		mm5 = _mm_mullo_pi16 (mm5, mm1);
		mm6 = _mm_mullo_pi16 (mm6, mm2);
		mm5 = _mm_add_pi16 (mm5, mm6);
		mm5 = _mm_add_pi16 (mm5, mm3);
		mm5 = _mm_srai_pi16(mm5, down_shift);
		mm5 = _mm_add_pi16(mm5, mm4);
		mm5 = _m_packuswb(mm5, mm0);
		*((DWORD*) (output+8)) = _m_to_int(mm5);

		mm5 = _m_from_int( *((DWORD*) (input1+12)));
		mm6 = _m_from_int( *((DWORD*) (input2+12)));
		mm5 = _mm_unpacklo_pi8(mm5, mm0);
		mm6 = _mm_unpacklo_pi8(mm6, mm0);
		mm5 = _mm_mullo_pi16 (mm5, mm1);
		mm6 = _mm_mullo_pi16 (mm6, mm2);
		mm5 = _mm_add_pi16 (mm5, mm6);
		mm5 = _mm_add_pi16 (mm5, mm3);
		mm5 = _mm_srai_pi16(mm5, down_shift);
		mm5 = _mm_add_pi16(mm5, mm4);
		mm5 = _m_packuswb(mm5, mm0);
		*((DWORD*) (output+12)) = _m_to_int(mm5);

		input1 += stride_in;
		input2+= stride_in;
		output+=stride_out;
	}	
}

void weight_8_b_mmx(imgpel *output, 
										int stride_out,
										imgpel *input1, imgpel *input2, 
										int weight1, int weight2, 
										int rounding_offset, unsigned int down_shift, int final_offset, 
										int stride_in, int height)
{
	int i;
	__m64 mm0, mm1, mm2, mm3, mm4, mm5, mm6;

	mm0 = _mm_setzero_si64();  //se mm0 to zero
	mm1 = _mm_set1_pi16 ((short) weight1);
	mm2 = _mm_set1_pi16 ((short) weight2);
	mm3 = _mm_set1_pi16 ((short) rounding_offset);	
	mm4 = _mm_set1_pi16 ((short) final_offset);	

	for (i=0; i<height; i++)
	{
		mm5 = _m_from_int( *((DWORD*) input1));
		mm6 = _m_from_int( *((DWORD*) input2));
		mm5 = _mm_unpacklo_pi8(mm5, mm0);
		mm6 = _mm_unpacklo_pi8(mm6, mm0);
		mm5 = _mm_mullo_pi16 (mm5, mm1);
		mm6 = _mm_mullo_pi16 (mm6, mm2);
		mm5 = _mm_add_pi16 (mm5, mm6);
		mm5 = _mm_add_pi16 (mm5, mm3);
		mm5 = _mm_srai_pi16(mm5, down_shift);
		mm5 = _mm_add_pi16(mm5, mm4);
		mm5 = _m_packuswb(mm5, mm0);
		*((DWORD*) output) = _m_to_int(mm5);

		mm5 = _m_from_int( *((DWORD*) (input1+4)));
		mm6 = _m_from_int( *((DWORD*) (input2+4)));
		mm5 = _mm_unpacklo_pi8(mm5, mm0);
		mm6 = _mm_unpacklo_pi8(mm6, mm0);
		mm5 = _mm_mullo_pi16 (mm5, mm1);
		mm6 = _mm_mullo_pi16 (mm6, mm2);
		mm5 = _mm_add_pi16 (mm5, mm6);
		mm5 = _mm_add_pi16 (mm5, mm3);
		mm5 = _mm_srai_pi16(mm5, down_shift);
		mm5 = _mm_add_pi16(mm5, mm4);
		mm5 = _m_packuswb(mm5, mm0);
		*((DWORD*) (output+4)) = _m_to_int(mm5);

		input1 += stride_in;
		input2+= stride_in;
		output+=stride_out;
	}	
}

void weight_4_b_mmx(imgpel *output, 
										int stride_out,
										imgpel *input1, imgpel *input2, 
										int weight1, int weight2, 
										int rounding_offset, unsigned int down_shift, int final_offset, 
										int stride_in, int height)
{
	/*__asm
	{		
	pxor mm0, mm0;
	mov esi, output;
	mov edi, input1;
	mov eax, input2;
	movsx ebx, weight1;
	movd mm1, ebx;
	movsx ebx, weight2;
	movd  mm2, ebx;
	mov ebx, rounding_offset;
	movd mm3, ebx;
	mov ebx, down_shift;
	movd mm4, ebx;
	mov ebx, final_offset;
	movd mm5, ebx;
	mov ebx, stride_out;
	mov ecx, stride_in;
	mov edx, height;
	punpcklwd mm1, mm1;  
	punpcklwd mm1, mm1;  //0w 0w 0w 0w 
	punpcklwd mm2, mm2;  
	punpcklwd mm2, mm2;  //0w 0w 0w 0w 
	punpcklwd mm3, mm3;
	punpcklwd mm3, mm3;  //0r 0r 0r 0r 
	punpcklwd mm5, mm5;
	punpcklwd mm5, mm5;  //0f 0f 0f 0f 

	LOOP4:		
	movq mm6, [edi];
	movq mm7, [eax];
	punpcklbw mm6, mm0;
	punpcklbw mm7, mm0;
	pmullw mm6, mm1;
	pmullw mm7, mm2;
	paddw mm6, mm3;
	paddw mm6, mm7;
	psraw mm6, mm4;		
	paddw mm6, mm5;		
	packuswb mm6, mm0;
	movd [esi], mm6;
	lea esi, [esi+ebx];
	lea edi, [edi+ecx];
	lea eax, [eax+ecx];
	sub edx, 1;
	jnz LOOP4;
	}*/

	int i;
	__m64 mm0, mm1, mm2, mm3, mm4, mm5, mm6;

	mm0 = _mm_setzero_si64();  //se mm0 to zero
	mm1 = _mm_set1_pi16 ((short) weight1);
	mm2 = _mm_set1_pi16 ((short) weight2);
	mm3 = _mm_set1_pi16 ((short) rounding_offset);	
	mm4 = _mm_set1_pi16 ((short) final_offset);	

	for (i=0; i<height; i++)
	{
		mm5 = _m_from_int( *((DWORD*) input1));
		mm6 = _m_from_int( *((DWORD*) input2));
		mm5 = _mm_unpacklo_pi8(mm5, mm0);
		mm6 = _mm_unpacklo_pi8(mm6, mm0);
		mm5 = _mm_mullo_pi16 (mm5, mm1);
		mm6 = _mm_mullo_pi16 (mm6, mm2);
		mm5 = _mm_add_pi16 (mm5, mm6);
		mm5 = _mm_add_pi16 (mm5, mm3);
		mm5 = _mm_srai_pi16(mm5, down_shift);
		mm5 = _mm_add_pi16(mm5, mm4);
		mm5 = _m_packuswb(mm5, mm0);
		*((DWORD*) output) = _m_to_int(mm5);
		input1 += stride_in;
		input2+= stride_in;
		output+=stride_out;
	}
}

void   weight_2_b_sse(imgpel *output, 
										int stride_out,
										imgpel *input1, imgpel *input2, 
										int weight1, int weight2, 
										int rounding_offset, unsigned int down_shift, int final_offset, 
										int stride_in, int height)
{
	//int jj;
	/*for(jj=0;jj<height;jj++)
	{
		output[0] = Clip1(((weight1 * input1[0] + weight2 * input2[0] + rounding_offset) >> down_shift) + final_offset);
		output[1] = Clip1(((weight1 * input1[1] + weight2 * input2[1] + rounding_offset) >> down_shift) + final_offset);
		output+=stride_out;
		input1+=stride_in;
		input2+=stride_in;
	}*/
	int i;
	__m64 mm0, mm1, mm2, mm3, mm4, mm5, mm6;

	mm0 = _mm_setzero_si64();  //se mm0 to zero
	mm1 = _mm_set1_pi16 ((short) weight1);
	mm2 = _mm_set1_pi16 ((short) weight2);
	mm3 = _mm_set1_pi16 ((short) rounding_offset);	
	mm4 = _mm_set1_pi16 ((short) final_offset);	

	for (i=0; i<height; i++)
	{
		mm5 = _m_from_int( *((DWORD*) input1));
		mm6 = _m_from_int( *((DWORD*) input2));
		mm5 = _mm_unpacklo_pi8(mm5, mm0);
		mm6 = _mm_unpacklo_pi8(mm6, mm0);
		mm5 = _mm_mullo_pi16 (mm5, mm1);
		mm6 = _mm_mullo_pi16 (mm6, mm2);
		mm5 = _mm_add_pi16 (mm5, mm6);
		mm5 = _mm_add_pi16 (mm5, mm3);
		mm5 = _mm_srai_pi16(mm5, down_shift);
		mm5 = _mm_add_pi16(mm5, mm4);
		mm5 = _mm_unpacklo_pi32(mm5,mm0);
		mm5 = _m_packuswb(mm5, mm0);
		*((DWORD*) output) = _m_to_int(mm5);
		input1-=2;
		input2-=2;
		output-=2;
		input1 += stride_in;
		input2+= stride_in;
		output+=stride_out;
	}
}
/*****************************************************
//merge UV code
******************************************************/

void weight_8_uv_sse2(imgpel *input_image, int weight_u, int weight_v, int rounding_offset, unsigned int down_shift, int final_offset_u, int final_offset_v, int stride_in, int height)
{
	int i;
	__m128i xmm0, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7;

	xmm0 = _mm_setzero_si128();
	//xmm1 = _mm_set1_epi16(255);
	xmm2 = _mm_set1_epi32((weight_v<<16)|(weight_u&0xFFFF));
	xmm3 = _mm_set1_epi16((short) rounding_offset);
	xmm4 = _mm_set1_epi32((final_offset_v<<16)|(final_offset_u&0xFFFF));

	for (i=0; i<height; i++)
	{
		xmm5 = _mm_load_si128((__m128i*)input_image);	
		xmm6 = _mm_unpacklo_epi8(xmm5, xmm0);
		xmm7 = _mm_unpackhi_epi8(xmm5, xmm0);	

		xmm6 = _mm_mullo_epi16(xmm6, xmm2);
		xmm7 = _mm_mullo_epi16(xmm7, xmm2);

		xmm6 = _mm_add_epi16(xmm6, xmm3);
		xmm7 = _mm_add_epi16(xmm7, xmm3);

		xmm6 = _mm_srai_epi16(xmm6, down_shift);
		xmm7 = _mm_srai_epi16(xmm7, down_shift);

		xmm6 = _mm_add_epi16(xmm6, xmm4);
		xmm7 = _mm_add_epi16(xmm7, xmm4);

		xmm5 = _mm_packus_epi16(xmm6, xmm7);  //clipping

		_mm_store_si128((__m128i*)input_image, xmm5);

		input_image += stride_in;
	}
}
void weight_8_uv_sse(imgpel *input_image, int weight_u, int weight_v, int rounding_offset, unsigned int down_shift, int final_offset_u, int final_offset_v, int stride_in, int height)
{
	int i;
	__m64 mm0, mm2, mm3, mm4, mm5, mm6, mm7;
	
	mm0 = _mm_setzero_si64();
	mm2 = _mm_set1_pi32((weight_v<<16)|(weight_u&0xFFFF));
	mm3 = _mm_set1_pi16((short) rounding_offset);
	mm4 = _mm_set1_pi32((final_offset_v<<16)|(final_offset_u&0xFFFF));

	for (i=0; i<height; i++)
	{
		mm5 = *((__m64*)input_image);
		mm6 = _mm_unpacklo_pi8(mm5, mm0);
		mm7 = _mm_unpackhi_pi8(mm5, mm0);	

		mm6 = _mm_mullo_pi16(mm6, mm2);
		mm7 = _mm_mullo_pi16(mm7, mm2);

		mm6 = _mm_add_pi16(mm6, mm3);
		mm7 = _mm_add_pi16(mm7, mm3);

		mm6 = _mm_srai_pi16(mm6, down_shift);
		mm7 = _mm_srai_pi16(mm7, down_shift);

		mm6 = _mm_add_pi16(mm6, mm4);
		mm7 = _mm_add_pi16(mm7, mm4);
		mm5 = _mm_packs_pu16(mm6, mm7);  //clipping
		*(__m64*)input_image = mm5;

		mm5 = *((__m64*)(input_image+8));
		mm6 = _mm_unpacklo_pi8(mm5, mm0);
		mm7 = _mm_unpackhi_pi8(mm5, mm0);	

		mm6 = _mm_mullo_pi16(mm6, mm2);
		mm7 = _mm_mullo_pi16(mm7, mm2);

		mm6 = _mm_add_pi16(mm6, mm3);
		mm7 = _mm_add_pi16(mm7, mm3);

		mm6 = _mm_srai_pi16(mm6, down_shift);
		mm7 = _mm_srai_pi16(mm7, down_shift);

		mm6 = _mm_add_pi16(mm6, mm4);
		mm7 = _mm_add_pi16(mm7, mm4);

		mm5 = _mm_packs_pu16(mm6, mm7);  //clipping
		*(__m64*)(input_image+8) = mm5;

		input_image += stride_in;
	}
	
}


void weight_4_uv_mmx(imgpel *input_image, int weight_u, int weight_v, int rounding_offset, unsigned int down_shift, int final_offset_u, int final_offset_v, int stride_in, int height)
{
	int i;
	__m64 mm0, mm2, mm3, mm4, mm5, mm6;

	mm0 = _mm_setzero_si64();  //se mm0 to zero
	mm2 = _mm_set1_pi32 ((weight_v<<16)|(weight_u&0xFFFF));
	mm3 = _mm_set1_pi16 ((short) rounding_offset);
	mm4 = _mm_set1_pi32 ((final_offset_v<<16)|(final_offset_u&0xFFFF));

	for (i=0; i<height; i++)
	{
		mm5 = _m_from_int( *((DWORD*) input_image));
		mm6 = _mm_unpacklo_pi8(mm5, mm0);
		mm6 = _mm_mullo_pi16 (mm6, mm2);		
		mm6 = _mm_add_pi16 (mm6, mm3);
		mm6 = _mm_srai_pi16(mm6, down_shift);
		mm6 = _mm_add_pi16(mm6, mm4);
		mm5 = _m_packuswb(mm6, mm0);
		*((DWORD*) input_image) = _m_to_int(mm5);

		mm5 = _m_from_int( *((DWORD*) (input_image+4)));
		mm6 = _mm_unpacklo_pi8(mm5, mm0);
		mm6 = _mm_mullo_pi16 (mm6, mm2);		
		mm6 = _mm_add_pi16 (mm6, mm3);
		mm6 = _mm_srai_pi16(mm6, down_shift);
		mm6 = _mm_add_pi16(mm6, mm4);
		mm5 = _m_packuswb(mm6, mm0);
		*((DWORD*) (input_image+4)) = _m_to_int(mm5);

		input_image += stride_in;
	}
}

void weight_2_uv_mmx(imgpel *input_image, int weight_u, int weight_v, int rounding_offset, unsigned int down_shift, int final_offset_u, int final_offset_v, int stride_in, int height)
{	
	int i;
	__m64 mm0, mm2, mm3, mm4, mm5, mm6;

	mm0 = _mm_setzero_si64();  //se mm0 to zero
	mm2 = _mm_set1_pi32 ((weight_v<<16)|(weight_u&0xFFFF));
	mm3 = _mm_set1_pi16 ((short) rounding_offset);
	mm4 = _mm_set1_pi32 ((final_offset_v<<16)|(final_offset_u&0xFFFF));

	for (i=0; i<height; i++)
	{
		mm5 = _m_from_int( *((DWORD*) input_image));
		mm6 = _mm_unpacklo_pi8(mm5, mm0);
		mm6 = _mm_mullo_pi16 (mm6, mm2);		
		mm6 = _mm_add_pi16 (mm6, mm3);
		mm6 = _mm_srai_pi16(mm6, down_shift);
		mm6 = _mm_add_pi16(mm6, mm4);
		mm5 = _m_packuswb(mm6, mm0);
		*((DWORD*) input_image) = _m_to_int(mm5);
		input_image += stride_in;
	}
}


void weight_8_b_uv_sse2(imgpel *output, 
												int stride_out,
												imgpel *input1, imgpel *input2, 
												int weight1_u, int weight2_u, 
												int weight1_v, int weight2_v, 
												int rounding_offset, unsigned int down_shift, int final_offset_u, int final_offset_v,
												int stride_in, int height)
{
	int i;
	__m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9;

	xmm0 = _mm_setzero_si128();
	xmm1 = _mm_set1_epi32((weight1_v<<16)|(weight1_u&0xFFFF));
	xmm2 = _mm_set1_epi32((weight2_v<<16)|(weight2_u&0xFFFF));
	xmm3 = _mm_set1_epi16((short) rounding_offset);
	xmm4 = _mm_set1_epi32((final_offset_v<<16)|(final_offset_u&0xFFFF));

	for (i=0; i<height; i++)
	{
		xmm5 = _mm_load_si128((__m128i*)input1);	
		xmm6 = _mm_unpacklo_epi8(xmm5, xmm0);
		xmm7 = _mm_unpackhi_epi8(xmm5, xmm0);
		xmm5 = _mm_load_si128((__m128i*)input2);
		xmm8 = _mm_unpacklo_epi8(xmm5, xmm0);
		xmm9 = _mm_unpackhi_epi8(xmm5, xmm0);


		xmm6 = _mm_mullo_epi16(xmm6, xmm1);
		xmm8 = _mm_mullo_epi16(xmm8, xmm2);
		xmm7 = _mm_mullo_epi16(xmm7, xmm1);		
		xmm9 = _mm_mullo_epi16(xmm9, xmm2);

		xmm6 = _mm_add_epi16(xmm6, xmm8);
		xmm7 = _mm_add_epi16(xmm7, xmm9);

		xmm6 = _mm_add_epi16(xmm6, xmm3);
		xmm7 = _mm_add_epi16(xmm7, xmm3);

		xmm6 = _mm_srai_epi16(xmm6, down_shift);
		xmm7 = _mm_srai_epi16(xmm7, down_shift);

		xmm6 = _mm_add_epi16(xmm6, xmm4);
		xmm7 = _mm_add_epi16(xmm7, xmm4);

		xmm5 = _mm_packus_epi16(xmm6, xmm7);  //clipping

		_mm_store_si128((__m128i*)output, xmm5);


		output += stride_out;
		input1 += stride_in;
		input2 += stride_in;
	}
}
void weight_8_b_uv_sse(imgpel *output, 
												int stride_out,
												imgpel *input1, imgpel *input2, 
												int weight1_u, int weight2_u, 
												int weight1_v, int weight2_v, 
												int rounding_offset, unsigned int down_shift, int final_offset_u, int final_offset_v,
												int stride_in, int height)
{
	int i;
	__m64 mm0, mm1, mm2, mm3, mm4, mm5, mm6, mm7;
	mm0 = _mm_setzero_si64();
	mm1 = _mm_set1_pi32((weight1_v<<16)|(weight1_u&0xFFFF));
	mm2 = _mm_set1_pi32((weight2_v<<16)|(weight2_u&0xFFFF));
	mm3 = _mm_set1_pi16((short) rounding_offset);
	mm4 = _mm_set1_pi32((final_offset_v<<16)|(final_offset_u&0xFFFF));

	for (i=0; i<height; i++)
	{
		mm0 = _mm_setzero_si64();
		mm1 = _mm_set1_pi32((weight1_v<<16)|(weight1_u&0xFFFF));
		
		mm5 = *((__m64*)input1);
		mm6 = _mm_unpacklo_pi8(mm5, mm0);
		mm7 = _mm_unpackhi_pi8(mm5, mm0);
		mm6 = _mm_mullo_pi16(mm6, mm1);
		mm7 = _mm_mullo_pi16(mm7, mm1);

		mm5 = *((__m64*)input2);
		mm1 = _mm_unpacklo_pi8(mm5, mm0);
		mm0 = _mm_unpackhi_pi8(mm5, mm0);
		mm1 = _mm_mullo_pi16(mm1, mm2);
		mm0 = _mm_mullo_pi16(mm0, mm2);

		mm6 = _mm_add_pi16(mm6, mm1);
		mm7 = _mm_add_pi16(mm7, mm0);

		mm6 = _mm_add_pi16(mm6, mm3);
		mm7 = _mm_add_pi16(mm7, mm3);

		mm6 = _mm_srai_pi16(mm6, down_shift);
		mm7 = _mm_srai_pi16(mm7, down_shift);

		mm6 = _mm_add_pi16(mm6, mm4);
		mm7 = _mm_add_pi16(mm7, mm4);

		mm5 = _mm_packs_pu16(mm6, mm7);  //clipping
		*((__m64*)output) = mm5;

		mm0 = _mm_setzero_si64();
		mm1 = _mm_set1_pi32((weight1_v<<16)|(weight1_u&0xFFFF));
		mm5 = *((__m64*)(input1+8));
		mm6 = _mm_unpacklo_pi8(mm5, mm0);
		mm7 = _mm_unpackhi_pi8(mm5, mm0);
		mm6 = _mm_mullo_pi16(mm6, mm1);
		mm7 = _mm_mullo_pi16(mm7, mm1);

		mm5 = *((__m64*)(input2+8));
		mm1 = _mm_unpacklo_pi8(mm5, mm0);
		mm0 = _mm_unpackhi_pi8(mm5, mm0);
		mm1 = _mm_mullo_pi16(mm1, mm2);
		mm0 = _mm_mullo_pi16(mm0, mm2);

		mm6 = _mm_add_pi16(mm6, mm1);
		mm7 = _mm_add_pi16(mm7, mm0);

		mm6 = _mm_add_pi16(mm6, mm3);
		mm7 = _mm_add_pi16(mm7, mm3);

		mm6 = _mm_srai_pi16(mm6, down_shift);
		mm7 = _mm_srai_pi16(mm7, down_shift);

		mm6 = _mm_add_pi16(mm6, mm4);
		mm7 = _mm_add_pi16(mm7, mm4);

		mm5 = _mm_packs_pu16(mm6, mm7);  //clipping
		*((__m64*)(output+8)) = mm5;

		output += stride_out;
		input1 += stride_in;
		input2 += stride_in;
	}
}


void weight_4_b_uv_sse2(imgpel *output, 
												int stride_out,
												imgpel *input1, imgpel *input2, 
												int weight1_u, int weight2_u, 
												int weight1_v, int weight2_v, 
												int rounding_offset, unsigned int down_shift, int final_offset_u, int final_offset_v,
												int stride_in, int height)
{
	int i;
	__m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7;

	xmm0 = _mm_setzero_si128();
	xmm1 = _mm_set1_epi32((weight1_v<<16)|(weight1_u&0xFFFF));
	xmm2 = _mm_set1_epi32((weight2_v<<16)|(weight2_u&0xFFFF));
	xmm3 = _mm_set1_epi16((short) rounding_offset);
	xmm4 = _mm_set1_epi32((final_offset_v<<16)|(final_offset_u&0xFFFF));

	for (i=0; i<height; i++)
	{
		xmm5 = _mm_loadl_epi64((__m128i*)input1);	
		xmm6 = _mm_unpacklo_epi8(xmm5, xmm0);
		xmm5 = _mm_loadl_epi64((__m128i*)input2);
		xmm7 = _mm_unpacklo_epi8(xmm5, xmm0);

		xmm6 = _mm_mullo_epi16(xmm6, xmm1);
		xmm7 = _mm_mullo_epi16(xmm7, xmm2);

		xmm6 = _mm_add_epi16(xmm6, xmm7);
		xmm6 = _mm_add_epi16(xmm6, xmm3);
		xmm6 = _mm_srai_epi16(xmm6, down_shift);
		xmm6 = _mm_add_epi16(xmm6, xmm4);
		xmm5 = _mm_packus_epi16(xmm6, xmm0);  //clipping
		_mm_storel_epi64((__m128i*)output, xmm5);

		input1 += stride_in;
		input2 += stride_in;
		output+=stride_out;
	}
}

void weight_4_b_uv_sse(imgpel *output, 
												int stride_out,
												imgpel *input1, imgpel *input2, 
												int weight1_u, int weight2_u, 
												int weight1_v, int weight2_v, 
												int rounding_offset, unsigned int down_shift, int final_offset_u, int final_offset_v,
												int stride_in, int height)
{
	int i;
	__m64 mm0, mm1, mm2, mm3, mm4, mm5, mm6, mm7;

	mm0 = _mm_setzero_si64();
	mm1 = _mm_set1_pi32((weight1_v<<16)|(weight1_u&0xFFFF));
	mm2 = _mm_set1_pi32((weight2_v<<16)|(weight2_u&0xFFFF));
	mm3 = _mm_set1_pi16((short) rounding_offset);
	mm4 = _mm_set1_pi32((final_offset_v<<16)|(final_offset_u&0xFFFF));

	for (i=0; i<height; i++)
	{
		mm5 = *((__m64*)input1);
		mm6 = _mm_unpacklo_pi8(mm5, mm0);
		mm5 = *((__m64*)input2);
		mm7 = _mm_unpacklo_pi8(mm5, mm0);

		mm6 = _mm_mullo_pi16(mm6, mm1);
		mm7 = _mm_mullo_pi16(mm7, mm2);

		mm6 = _mm_add_pi16(mm6, mm7);
		mm6 = _mm_add_pi16(mm6, mm3);
		mm6 = _mm_srai_pi16(mm6, down_shift);
		mm6 = _mm_add_pi16(mm6, mm4);
		mm5 = _mm_packs_pu16(mm6, mm0);  //clipping
		*((DWORD*)output) =_m_to_int(mm5); 

		mm5 = *((__m64*)(input1+4));
		mm6 = _mm_unpacklo_pi8(mm5, mm0);
		mm5 = *((__m64*)(input2+4));
		mm7 = _mm_unpacklo_pi8(mm5, mm0);

		mm6 = _mm_mullo_pi16(mm6, mm1);
		mm7 = _mm_mullo_pi16(mm7, mm2);

		mm6 = _mm_add_pi16(mm6, mm7);
		mm6 = _mm_add_pi16(mm6, mm3);
		mm6 = _mm_srai_pi16(mm6, down_shift);
		mm6 = _mm_add_pi16(mm6, mm4);
		mm5 = _mm_packs_pu16(mm6, mm0);  //clipping
		*((DWORD*)(output+4)) = _m_to_int(mm5);

		input1 += stride_in;
		input2 += stride_in;
		output+=stride_out;
	}
}


void weight_2_b_uv_mmx(imgpel *output, 
											 int stride_out,
											 imgpel *input1, imgpel *input2, 
											 int weight1_u, int weight2_u, 
											 int weight1_v, int weight2_v, 
											 int rounding_offset, unsigned int down_shift, int final_offset_u, int final_offset_v,
											 int stride_in, int height)
{
	int i;
	__m64 mm0, mm1, mm2, mm3, mm4, mm5, mm6;

	mm0 = _mm_setzero_si64();  //se mm0 to zero
	mm1 = _mm_set1_pi32 ((weight1_v<<16)|(weight1_u&0xFFFF));
	mm2 = _mm_set1_pi32 ((weight2_v<<16)|(weight2_u&0xFFFF));
	mm3 = _mm_set1_pi16 ((short) rounding_offset);	
	mm4 = _mm_set1_pi32 ((final_offset_v<<16)|(final_offset_u&0xFFFF));	

	for (i=0; i<height; i++)
	{
		mm5 = _m_from_int( *((DWORD*) input1));
		mm6 = _m_from_int( *((DWORD*) input2));
		mm5 = _mm_unpacklo_pi8(mm5, mm0);
		mm6 = _mm_unpacklo_pi8(mm6, mm0);
		mm5 = _mm_mullo_pi16 (mm5, mm1);
		mm6 = _mm_mullo_pi16 (mm6, mm2);
		mm5 = _mm_add_pi16 (mm5, mm6);
		mm5 = _mm_add_pi16 (mm5, mm3);
		mm5 = _mm_srai_pi16(mm5, down_shift);
		mm5 = _mm_add_pi16(mm5, mm4);
		mm5 = _m_packuswb(mm5, mm0);
		*((DWORD*) output) = _m_to_int(mm5);
		input1 += stride_in;
		input2+= stride_in;
		output+=stride_out;
	}
}
#endif //H264_ENABLE_INTRINSICS
