/**
* @file mb_block4x4_mmx.cpp
* Implementation of the various 4x4 block level texture transforms 
* and inverse transforms.
*
*/

#include "global.h"
#ifdef H264_ENABLE_INTRINSICS
#include <mmintrin.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include "mb_block4x4.h"

// Disable "No EMMS at end of function '<function name>'"
#pragma warning ( disable : 4799 )

// IoK: 7-DEC-2005
// Added a new implementation that is ~10% faster - kept the old one

// Old code
#if 0
#define Transpose(mm0,mm1,mm2,mm3)  {	\
	\
	tmp0	= _mm_unpackhi_pi16(mm0,mm1);	\
	tmp2	= _mm_unpacklo_pi16(mm0,mm1);	\
	tmp1	= _mm_unpackhi_pi16(mm2,mm3);	\
	tmp3	= _mm_unpacklo_pi16(mm2,mm3);	\
	\
	(mm0)	= _mm_unpacklo_pi32(tmp2,tmp3);	\
	(mm1)	= _mm_unpackhi_pi32(tmp2,tmp3);	\
	(mm2)	= _mm_unpacklo_pi32(tmp0,tmp1);	\
	(mm3)	= _mm_unpackhi_pi32(tmp0,tmp1);	\
}


void inverse_transform4x4_sse(unsigned char *dest, byte *pred,
															//	PIX_COEFF_TYPE *src, int stride)
															short *src, int stride)
{	
	/*
	//shortcut: all 16 coefficients are zero 
	if (!((src[0]) | (src[1]) | (src[2]) | (src[3]) | (src[4])| (src[5]) |(src[6])|(src[7])
	|(src[8]) | (src[9])| (src[10]) | (src[11])|(src[12])|(src[13])|(src[14])|(src[15])))
	{
	*((unsigned long*)&dest[0])=*((unsigned long*)&pred[0]);
	*((unsigned long*)&dest[stride])=*((unsigned long*)&pred[16]);
	*((unsigned long*)&dest[stride<<1])=*((unsigned long*)&pred[32]);
	*((unsigned long*)&dest[(stride<<1)+stride])=*((unsigned long*)&pred[48]);			
	return;
	}
	*/
	__m64 mm0,  mm1,  mm2,  mm3;
	__m64 tmp0, tmp1, tmp2, tmp3;
	__m64 pred_0, pred_1, pred_2, pred_3;
	static __m64  ncoeff_0   = _mm_setzero_si64();	
	static __m64  ncoeff_32  = _mm_set1_pi16(32);


	DWORD*  pIn0 = (DWORD*) (pred);
	DWORD*  pIn1 = (DWORD*) (pred+16);	
	DWORD*  pIn2 = (DWORD*) (pred+32);	
	DWORD*  pIn3 = (DWORD*) (pred+48);	

	pred_0 = _m_from_int(*pIn0);
	pred_1 = _m_from_int(*pIn1);                
	pred_2 = _m_from_int(*pIn2);                
	pred_3 = _m_from_int(*pIn3);                	

	pred_0 = _mm_unpacklo_pi8(pred_0,ncoeff_0);
	pred_1 = _mm_unpacklo_pi8(pred_1,ncoeff_0);
	pred_2 = _mm_unpacklo_pi8(pred_2,ncoeff_0);
	pred_3 = _mm_unpacklo_pi8(pred_3,ncoeff_0);

	mm0 = *((__m64*)&src[0]);
	mm1 = *((__m64*)&src[4]);
	mm2 = *((__m64*)&src[8]);
	mm3 = *((__m64*)&src[12]);
	memset(src, 0, 16*sizeof(short));

#if !defined(_PRE_TRANSPOSE_)
	Transpose(mm0,mm1,mm2,mm3);
#endif
	// horizontal
	tmp0 = _m_paddw(mm0,mm2);
	tmp1 = _m_psubw(mm0,mm2);
	tmp2 = _m_psubw(_m_psrawi(mm1,1),mm3);
	tmp3 = _m_paddw(mm1,_m_psrawi(mm3,1));

	mm0 = _m_paddw(tmp0,tmp3);
	mm1 = _m_paddw(tmp1,tmp2);
	mm2 = _m_psubw(tmp1,tmp2);
	mm3 = _m_psubw(tmp0,tmp3);

	Transpose(mm0,mm1,mm2,mm3);

	//vertical
	tmp0 = _m_paddw(mm0,mm2);
	tmp1 = _m_psubw(mm0,mm2);
	tmp2 = _m_psubw(_m_psrawi(mm1,1),mm3);
	tmp3 = _m_paddw(mm1,_m_psrawi(mm3,1));

	mm0 = _m_paddw(_m_psrawi(_m_paddw(_m_paddw(tmp0,tmp3),ncoeff_32),6),pred_0);
	mm1 = _m_paddw(_m_psrawi(_m_paddw(_m_paddw(tmp1,tmp2),ncoeff_32),6),pred_1);
	mm2 = _m_paddw(_m_psrawi(_m_paddw(_m_psubw(tmp1,tmp2),ncoeff_32),6),pred_2);
	mm3 = _m_paddw(_m_psrawi(_m_paddw(_m_psubw(tmp0,tmp3),ncoeff_32),6),pred_3);

	mm0 = _m_packuswb(mm0,mm0);
	mm1 = _m_packuswb(mm1,mm1);
	mm2 = _m_packuswb(mm2,mm2);
	mm3 = _m_packuswb(mm3,mm3);

	*((DWORD*)&dest[0]) = _m_to_int(mm0);
	*((DWORD*)&dest[stride]) = _m_to_int(mm1);
	*((DWORD*)&dest[stride<<1]) = _m_to_int(mm2);
	*((DWORD*)&dest[(stride<<1)+stride])= _m_to_int(mm3);
}
#else
// New code, 78 assembly instructions
void inverse_transform4x4_2_sse(unsigned char *dest, byte *pred,
															short *src, int stride)
{
	__m64 mm0,  mm1,  mm2,  mm3,  mm4,  mm5,  mm6,  mm7;
	static const __declspec(align(8)) short ncoeff_32[] = { 32, 32, 32, 32};

#if !defined(_PRE_TRANSPOSE_)
	mm0 = *((__m64*)&src[0]);
	mm1 = *((__m64*)&src[4]);
	mm4 = *((__m64*)&src[8]);
	mm3 = *((__m64*)&src[12]);

	mm2 = mm0;
	mm6 = mm4;
	mm0 = _mm_unpacklo_pi16(mm0, mm1);
	mm4 = _mm_unpacklo_pi16(mm4, mm3);
	mm2 = _mm_unpackhi_pi16(mm2, mm1);
	mm6 = _mm_unpackhi_pi16(mm6, mm3);

	mm1 = mm0;
	mm3 = mm2;
	mm0 = _mm_unpacklo_pi32(mm0, mm4);
	mm2 = _mm_unpacklo_pi32(mm2, mm6);
	mm1 = _mm_unpackhi_pi32(mm1, mm4);
	mm3 = _mm_unpackhi_pi32(mm3, mm6);
#else
	mm0 = *((__m64*)&src[0]); //m0
	mm1 = *((__m64*)&src[4]); //m1
	mm2 = *((__m64*)&src[8]); //m2
	mm3 = *((__m64*)&src[12]); //m3
#endif
	// horizontal
	mm4 = mm0;
	mm0 = _mm_add_pi16(mm0, mm2);	// m0+m2   
	mm4 = _mm_sub_pi16(mm4, mm2);	// m0-m2  

	mm5 = mm1;
	mm1 = _mm_srai_pi16(mm1, 1);    //        
	mm1 = _mm_sub_pi16(mm1, mm3);	// 1/2m1-m3   
	mm3 = _mm_srai_pi16(mm3, 1);    //		  
	mm5 = _mm_add_pi16(mm5, mm3);	// m1+1/2m3   

	mm6 = mm0;                      //        
	mm2 = mm4;                      //        

	mm0 = _mm_add_pi16(mm0, mm5);	// m0+m1+m2+1/2m3  1111 
	mm4 = _mm_add_pi16(mm4, mm1);	// m0+1/2m1-m2-m3  2222
	mm2 = _mm_sub_pi16(mm2, mm1);	// m0-1/2m1-m2+m3  3333	
	mm6 = _mm_sub_pi16(mm6, mm5);	// m0-m1+m2-1/2m3  4444

	mm1 = mm0;     
	mm3 = mm2;      
	mm0 = _mm_unpacklo_pi16(mm0, mm4);  // 1212
	mm2 = _mm_unpacklo_pi16(mm2, mm6);  // 3434
	mm1 = _mm_unpackhi_pi16(mm1, mm4);  // 1212
	mm3 = _mm_unpackhi_pi16(mm3, mm6);  // 3434

	mm4 = mm0;    
	mm5 = mm1;    
	mm0 = _mm_unpacklo_pi32(mm0, mm2); // 1234 n0
	mm1 = _mm_unpacklo_pi32(mm1, mm3); // 1234 n1
	mm4 = _mm_unpackhi_pi32(mm4, mm2); // 1234 n2
	mm5 = _mm_unpackhi_pi32(mm5, mm3); // 1234 n3

	// mm0, mm4, mm1, mm5
	//vertical

	mm2 = mm0;   
	mm0 = _mm_add_pi16(mm0, mm1); // n0+n1
	mm2 = _mm_sub_pi16(mm2, mm1); // n0-n1

	mm6 = mm4; 
	mm4 = _mm_srai_pi16(mm4, 1); 
	mm4 = _mm_sub_pi16(mm4, mm5); // 1/2n2 - n3						  
													
	
	mm5 = _mm_srai_pi16(mm5, 1);										  
	mm6 = _mm_add_pi16(mm6, mm5); // n2 + 1/2n3
	mm1 = mm2; 	
	mm3 = mm0; 	   
			   
	mm0 = _mm_add_pi16(mm0,mm6); // n0+n1+n2+1/2n3 p0
								
							    
	mm1 = _mm_add_pi16(mm1,mm4); // n0-n1+1/2n2-n3 p1
	mm2 = _mm_sub_pi16(mm2,mm4); // n0-n1-1/2n2+n3 p2
	mm3 = _mm_sub_pi16(mm3,mm6); // n0+n1-n2-1/2n3 p3

	mm7 = *(__m64*) ncoeff_32;

	mm0 = _mm_add_pi16(mm0,mm7); //p0+32
	mm1 = _mm_add_pi16(mm1,mm7); //p1+32
	mm2 = _mm_add_pi16(mm2,mm7); //p2+32
	mm3 = _mm_add_pi16(mm3,mm7); //p3+32

	mm0 = _mm_srai_pi16(mm0,6); // (p0+32)>>6
	mm1 = _mm_srai_pi16(mm1,6); // (p1+32)>>6
	mm2 = _mm_srai_pi16(mm2,6); // (p2+32)>>6
	mm3 = _mm_srai_pi16(mm3,6); // (p3+32)>>6

	mm7 = _mm_setzero_si64();

	mm4 = _m_from_int(*(DWORD*) (pred));
	mm5 = _m_from_int(*(DWORD*) (pred+16));

	mm4 = _mm_unpacklo_pi8(mm4,mm7);
	mm5 = _mm_unpacklo_pi8(mm5,mm7);

	mm0 = _mm_add_pi16(mm0,mm4);
	mm1 = _mm_add_pi16(mm1,mm5);

	mm4 = _m_from_int(*(DWORD*) (pred+32));
	mm5 = _m_from_int(*(DWORD*) (pred+48));

	mm4 = _mm_unpacklo_pi8(mm4,mm7);
	mm5 = _mm_unpacklo_pi8(mm5,mm7);

	mm2 = _mm_add_pi16(mm2,mm4);
	mm3 = _mm_add_pi16(mm3,mm5);

	*(__m64*) (&src[ 0]) = mm0;	// src: word ptr
	*(__m64*) (&src[ 4]) = mm1;
	*(__m64*) (&src[ 8]) = mm2;
	*(__m64*) (&src[12]) = mm3;

//*********************************************

	mm0 = *((__m64*)&src[16]); //m0
	mm1 = *((__m64*)&src[20]); //m1
	mm2 = *((__m64*)&src[24]); //m2
	mm3 = *((__m64*)&src[28]); //m3

	// horizontal
	mm4 = mm0;
	mm0 = _mm_add_pi16(mm0, mm2);	// m0+m2   
	mm4 = _mm_sub_pi16(mm4, mm2);	// m0-m2  

	mm5 = mm1;
	mm1 = _mm_srai_pi16(mm1, 1);    //        
	mm1 = _mm_sub_pi16(mm1, mm3);	// 1/2m1-m3   
	mm3 = _mm_srai_pi16(mm3, 1);    //		  
	mm5 = _mm_add_pi16(mm5, mm3);	// m1+1/2m3   

	mm6 = mm0;                      //        
	mm2 = mm4;                      //        

	mm0 = _mm_add_pi16(mm0, mm5);	// m0+m1+m2+1/2m3  5555
	mm4 = _mm_add_pi16(mm4, mm1);	// m0+1/2m1-m2-m3  6666
	mm2 = _mm_sub_pi16(mm2, mm1);	// m0-1/2m1-m2+m3  7777	
	mm6 = _mm_sub_pi16(mm6, mm5);	// m0-m1+m2-1/2m3  8888

	mm1 = mm0;     
	mm3 = mm2;      
	mm0 = _mm_unpacklo_pi16(mm0, mm4);  // 5656
	mm2 = _mm_unpacklo_pi16(mm2, mm6);  // 7878
	mm1 = _mm_unpackhi_pi16(mm1, mm4);  // 5656
	mm3 = _mm_unpackhi_pi16(mm3, mm6);  // 7878

	mm4 = mm0;    
	mm5 = mm1;    
	mm0 = _mm_unpacklo_pi32(mm0, mm2); // 5678 n0
	mm1 = _mm_unpacklo_pi32(mm1, mm3); // 5678 n1
	mm4 = _mm_unpackhi_pi32(mm4, mm2); // 5678 n2
	mm5 = _mm_unpackhi_pi32(mm5, mm3); // 5678 n3

	// mm0, mm4, mm1, mm5
	//vertical

	mm2 = mm0;   
	mm0 = _mm_add_pi16(mm0, mm1); // n0+n1
	mm2 = _mm_sub_pi16(mm2, mm1); // n0-n1

	mm6 = mm4; 
	mm4 = _mm_srai_pi16(mm4, 1); 
	mm4 = _mm_sub_pi16(mm4, mm5); // 1/2n2 - n3						  
													
	
	mm5 = _mm_srai_pi16(mm5, 1);										  
	mm6 = _mm_add_pi16(mm6, mm5); // n2 + 1/2n3
	mm1 = mm2; 	
	mm3 = mm0; 	   
			   
	mm0 = _mm_add_pi16(mm0,mm6); // n0+n1+n2+1/2n3 p0
								
							    
	mm1 = _mm_add_pi16(mm1,mm4); // n0-n1+1/2n2-n3 p1
	mm2 = _mm_sub_pi16(mm2,mm4); // n0-n1-1/2n2+n3 p2
	mm3 = _mm_sub_pi16(mm3,mm6); // n0+n1-n2-1/2n3 p3

	mm7 = *(__m64*) ncoeff_32;

	mm0 = _mm_add_pi16(mm0,mm7); //p0+32
	mm1 = _mm_add_pi16(mm1,mm7); //p1+32
	mm2 = _mm_add_pi16(mm2,mm7); //p2+32
	mm3 = _mm_add_pi16(mm3,mm7); //p3+32

	mm0 = _mm_srai_pi16(mm0,6); // (p0+32)>>6
	mm1 = _mm_srai_pi16(mm1,6); // (p1+32)>>6
	mm2 = _mm_srai_pi16(mm2,6); // (p2+32)>>6
	mm3 = _mm_srai_pi16(mm3,6); // (p3+32)>>6

	mm7 = _mm_setzero_si64();

	mm4 = _m_from_int(*(DWORD*) (pred+4));
	mm5 = _m_from_int(*(DWORD*) (pred+20));

	mm4 = _mm_unpacklo_pi8(mm4,mm7);
	mm5 = _mm_unpacklo_pi8(mm5,mm7);

	mm0 = _mm_add_pi16(mm0,mm4);
	mm1 = _mm_add_pi16(mm1,mm5);

	mm4 = _m_from_int(*(DWORD*) (pred+36));
	mm5 = _m_from_int(*(DWORD*) (pred+52));

	mm4 = _mm_unpacklo_pi8(mm4,mm7);
	mm5 = _mm_unpacklo_pi8(mm5,mm7);

	mm2 = _mm_add_pi16(mm2,mm4);
	mm3 = _mm_add_pi16(mm3,mm5);

	mm4 = *((__m64*)&src[0]); 
	mm5 = *((__m64*)&src[4]); 
	mm6 = *((__m64*)&src[8]); 
	mm7 = *((__m64*)&src[12]);

//*********************************************

	mm0 = _mm_packs_pu16(mm4,mm0);
	mm1 = _mm_packs_pu16(mm5,mm1);
	mm2 = _mm_packs_pu16(mm6,mm2);
	mm3 = _mm_packs_pu16(mm7,mm3);

	mm7 = _mm_setzero_si64();
	*(__m64*) (&src[ 0]) = mm7;	// src: word ptr
	*(__m64*) (&src[ 4]) = mm7;
	*(__m64*) (&src[ 8]) = mm7;
	*(__m64*) (&src[12]) = mm7;
	*(__m64*) (&src[16]) = mm7;	// src: word ptr
	*(__m64*) (&src[20]) = mm7;
	*(__m64*) (&src[24]) = mm7;
	*(__m64*) (&src[28]) = mm7;
/*
	*((DWORD*)&dest[0])                 = _m_to_int(mm0);
	*((DWORD*)&dest[stride])            = _m_to_int(mm1);
	*((DWORD*)&dest[stride<<1])         = _m_to_int(mm2);
	*((DWORD*)&dest[(stride<<1)+stride])= _m_to_int(mm3);
*/
	*((__m64*)&dest[0]) = mm0;
	*((__m64*)&dest[stride]) = mm1;
	*((__m64*)&dest[stride<<1]) = mm2;
	*((__m64*)&dest[(stride<<1)+stride]) = mm3;
}

void inverse_transform4x4_sse(unsigned char *dest, byte *pred,
															short *src, int stride)
{
	__m64 mm0,  mm1,  mm2,  mm3,  mm4,  mm5,  mm6,  mm7;
	static const __declspec(align(8)) short ncoeff_32[] = { 32, 32, 32, 32};

#if !defined(_PRE_TRANSPOSE_)
	mm0 = *((__m64*)&src[0]);
	mm1 = *((__m64*)&src[4]);
	mm4 = *((__m64*)&src[8]);
	mm3 = *((__m64*)&src[12]);

	mm2 = mm0;
	mm6 = mm4;
	mm0 = _mm_unpacklo_pi16(mm0, mm1);
	mm4 = _mm_unpacklo_pi16(mm4, mm3);
	mm2 = _mm_unpackhi_pi16(mm2, mm1);
	mm6 = _mm_unpackhi_pi16(mm6, mm3);

	mm1 = mm0;
	mm3 = mm2;
	mm0 = _mm_unpacklo_pi32(mm0, mm4);
	mm2 = _mm_unpacklo_pi32(mm2, mm6);
	mm1 = _mm_unpackhi_pi32(mm1, mm4);
	mm3 = _mm_unpackhi_pi32(mm3, mm6);
#else
	mm0 = *((__m64*)&src[0]);
	mm1 = *((__m64*)&src[4]);
	mm2 = *((__m64*)&src[8]);
	mm3 = *((__m64*)&src[12]);
#endif
	// horizontal
	mm4 = mm0;
	mm0 = _mm_add_pi16(mm0, mm2);	// tmp0   
	mm4 = _mm_sub_pi16(mm4, mm2);	// tmp1   

	mm5 = mm1;
	mm1 = _mm_srai_pi16(mm1, 1);    //        
	mm1 = _mm_sub_pi16(mm1, mm3);	// tmp2   
	mm3 = _mm_srai_pi16(mm3, 1);    //		  
	mm5 = _mm_add_pi16(mm5, mm3);	// tmp3   

	mm6 = mm0;                      //        
	mm2 = mm4;                      //        

	mm0 = _mm_add_pi16(mm0, mm5);	// (old mm0) 
	mm4 = _mm_add_pi16(mm4, mm1);	// (old mm1) 
	mm2 = _mm_sub_pi16(mm2, mm1);	// (old mm2) 
	mm6 = _mm_sub_pi16(mm6, mm5);	// (old mm3) 

	mm1 = mm0;     
	mm3 = mm2;      
	mm0 = _mm_unpacklo_pi16(mm0, mm4);  
	mm2 = _mm_unpacklo_pi16(mm2, mm6);  
	mm1 = _mm_unpackhi_pi16(mm1, mm4);  
	mm3 = _mm_unpackhi_pi16(mm3, mm6);  

	mm4 = mm0;    
	mm5 = mm1;    
	mm0 = _mm_unpacklo_pi32(mm0, mm2); 
	mm1 = _mm_unpacklo_pi32(mm1, mm3); 
	mm4 = _mm_unpackhi_pi32(mm4, mm2); 
	mm5 = _mm_unpackhi_pi32(mm5, mm3); 

	// mm0, mm4, mm1, mm5
	//vertical

	mm2 = mm0;   
	mm0 = _mm_add_pi16(mm0, mm1); 
	mm2 = _mm_sub_pi16(mm2, mm1); 

	mm6 = mm4; 
	mm4 = _mm_srai_pi16(mm4, 1);
	mm4 = _mm_sub_pi16(mm4, mm5);						  
													
	
	mm5 = _mm_srai_pi16(mm5, 1);										  
	mm6 = _mm_add_pi16(mm6, mm5);
	mm1 = mm2; 	
	mm3 = mm0; 	   
			   
			   

	mm0 = _mm_add_pi16(mm0,mm6);
								
							    
								

	mm1 = _mm_add_pi16(mm1,mm4);
	mm2 = _mm_sub_pi16(mm2,mm4);
	mm3 = _mm_sub_pi16(mm3,mm6);

	mm7 = *(__m64*) ncoeff_32;

	mm0 = _mm_add_pi16(mm0,mm7);
	mm1 = _mm_add_pi16(mm1,mm7);
	mm2 = _mm_add_pi16(mm2,mm7);
	mm3 = _mm_add_pi16(mm3,mm7);

	mm0 = _mm_srai_pi16(mm0,6);
	mm1 = _mm_srai_pi16(mm1,6);
	mm2 = _mm_srai_pi16(mm2,6);
	mm3 = _mm_srai_pi16(mm3,6);

	mm7 = _mm_setzero_si64();

	mm4 = _m_from_int(*(DWORD*) (pred));
	mm5 = _m_from_int(*(DWORD*) (pred+16));

	mm4 = _mm_unpacklo_pi8(mm4,mm7);
	mm5 = _mm_unpacklo_pi8(mm5,mm7);

	mm0 = _mm_add_pi16(mm0,mm4);
	mm1 = _mm_add_pi16(mm1,mm5);

	mm4 = _m_from_int(*(DWORD*) (pred+32));
	mm5 = _m_from_int(*(DWORD*) (pred+48));

	mm4 = _mm_unpacklo_pi8(mm4,mm7);
	mm5 = _mm_unpacklo_pi8(mm5,mm7);

	mm2 = _mm_add_pi16(mm2,mm4);
	mm3 = _mm_add_pi16(mm3,mm5);

	*(__m64*) (&src[ 0]) = mm7;	// src: word ptr
	*(__m64*) (&src[ 4]) = mm7;
	*(__m64*) (&src[ 8]) = mm7;
	*(__m64*) (&src[12]) = mm7;

	mm0 = _mm_packs_pu16(mm0,mm0);
	mm1 = _mm_packs_pu16(mm1,mm1);
	mm2 = _mm_packs_pu16(mm2,mm2);
	mm3 = _mm_packs_pu16(mm3,mm3);

	*((DWORD*)&dest[0])                 = _m_to_int(mm0);
	*((DWORD*)&dest[stride])            = _m_to_int(mm1);
	*((DWORD*)&dest[stride<<1])         = _m_to_int(mm2);
	*((DWORD*)&dest[(stride<<1)+stride])= _m_to_int(mm3);
}
#endif

void inverse_transform4x4_2_sse2(unsigned char *dest, byte *pred,
																 short *src, int stride)
{

	const static unsigned short __declspec(align(16)) const_32[8] = { 32, 32, 32, 32, 32, 32, 32, 32 };
	__m128i mm0,  mm1,  mm2,  mm3;
	__m128i tmp0, tmp1, tmp2, tmp3, tmp4;


	mm0 = _mm_load_si128((__m128i*)&src[0]);
	mm1 = _mm_load_si128((__m128i*)&src[8]);
	mm2 = _mm_load_si128((__m128i*)&src[16]);
	mm3 = _mm_load_si128((__m128i*)&src[24]);

	tmp0=_mm_unpacklo_epi64(mm0,mm2);	//m0 m0'
	tmp2=_mm_unpacklo_epi64(mm1,mm3);	//m2 m2'
	tmp1=_mm_unpackhi_epi64(mm0,mm2);	//m1 m1'
	tmp3=_mm_unpackhi_epi64(mm1,mm3);	//m3 m3'

	// horizontal
	mm0 = _mm_adds_epi16(tmp0,tmp2);	//m0+m2
	mm2 = _mm_subs_epi16(_mm_srai_epi16(tmp1,1),tmp3); //  1/2m1-m3
	mm1 = _mm_subs_epi16(tmp0,tmp2);	//m0-m2
	mm3 = _mm_adds_epi16(tmp1,_mm_srai_epi16(tmp3,1)); //  m1+1/2m3

	tmp0 = _mm_adds_epi16(mm0,mm3);		//m0+m1+m2+1/2m3 11115555
	tmp1 = _mm_adds_epi16(mm1,mm2);		//m0+1/2m1-m2-m3 22226666	
	tmp3 = _mm_subs_epi16(mm0,mm3);     //m0-m1+m2-1/2m3 44448888
	tmp2 = _mm_subs_epi16(mm1,mm2);		//m0-1/2m1-m2+m3 33337777

	//transpose
	mm0 =_mm_unpacklo_epi16(tmp0,tmp1);	// 12121212
	mm2 =_mm_unpacklo_epi16(tmp2,tmp3); // 34343434
	mm1 =_mm_unpackhi_epi16(tmp0,tmp1);	// 56565656 
	mm3 =_mm_unpackhi_epi16(tmp2,tmp3);	// 78787878

	tmp0 =_mm_unpacklo_epi32(mm0,mm2);	// 12341234
	tmp2 =_mm_unpacklo_epi32(mm1,mm3);  // 56785678
	tmp1 =_mm_unpackhi_epi32(mm0,mm2);	// 12341234 
	tmp3 =_mm_unpackhi_epi32(mm1,mm3);	// 56785678

	mm0 =_mm_unpacklo_epi64(tmp0,tmp2);	// 12345678
	mm2 =_mm_unpacklo_epi64(tmp1,tmp3); // 12345678
	mm1 =_mm_unpackhi_epi64(tmp0,tmp2);	// 12345678 
	mm3 =_mm_unpackhi_epi64(tmp1,tmp3);	// 12345678

	//vertical
	tmp0 = _mm_adds_epi16(mm0,mm2);
	tmp2 = _mm_subs_epi16(_mm_srai_epi16(mm1,1),mm3);
	tmp1 = _mm_subs_epi16(mm0,mm2);
	tmp3 = _mm_adds_epi16(mm1,_mm_srai_epi16(mm3,1));


	mm0 = _mm_adds_epi16(tmp0,tmp3);
	mm1 = _mm_adds_epi16(tmp1,tmp2);
	mm3 = _mm_subs_epi16(tmp0,tmp3);
	mm2 = _mm_subs_epi16(tmp1,tmp2);


	tmp0 = _mm_load_si128((__m128i*)&const_32);
	mm0 = _mm_adds_epi16(mm0,tmp0);
	mm1 = _mm_adds_epi16(mm1,tmp0);
	mm2 = _mm_adds_epi16(mm2,tmp0);
	mm3 = _mm_adds_epi16(mm3,tmp0);
	mm0 = _mm_srai_epi16(mm0,6);
	mm1 = _mm_srai_epi16(mm1,6);
	mm2 = _mm_srai_epi16(mm2,6);
	mm3 = _mm_srai_epi16(mm3,6);

	tmp4 = _mm_setzero_si128();

	tmp0 = _mm_loadu_si128((__m128i*)pred);
	tmp0 =_mm_unpacklo_epi8(tmp0,tmp4);
	mm0 = _mm_adds_epi16(mm0,tmp0);

	tmp1 = _mm_loadu_si128((__m128i*)(pred+16));
	tmp1 =_mm_unpacklo_epi8(tmp1,tmp4);
	mm1 = _mm_adds_epi16(mm1,tmp1);

	tmp2 = _mm_loadu_si128((__m128i*)(pred+32));
	tmp2 =_mm_unpacklo_epi8(tmp2,tmp4);
	mm2 = _mm_adds_epi16(mm2,tmp2);

	tmp3 = _mm_loadu_si128((__m128i*)(pred+48));
	tmp3 =_mm_unpacklo_epi8(tmp3,tmp4);
	mm3 = _mm_adds_epi16(mm3,tmp3);

	*(__m128i*) (&src[0]) = tmp4;	
	*(__m128i*) (&src[8]) = tmp4;
	*(__m128i*) (&src[16]) = tmp4;
	*(__m128i*) (&src[24]) = tmp4;

	mm0 = _mm_packus_epi16(mm0,tmp4);
	mm1 = _mm_packus_epi16(mm1,tmp4);
	mm2 = _mm_packus_epi16(mm2,tmp4);
	mm3 = _mm_packus_epi16(mm3,tmp4);

	_mm_storel_epi64 ((__m128i*)&dest[0], mm0);
	_mm_storel_epi64 ((__m128i*)&dest[stride], mm1);
	_mm_storel_epi64 ((__m128i*)&dest[stride<<1], mm2);
	_mm_storel_epi64 ((__m128i*)&dest[(stride<<1)+stride], mm3);
}

void inverse_transform4x4_mergeuv_sse(unsigned char *dest, byte *pred,
																			 short *src_u, short *src_v, int stride)
{
	static const __declspec(align(16)) short ncoeff_32_8[] = { 32, 32, 32, 32, 32, 32, 32, 32};
	__m64 mm0,  mm1,  mm2,  mm3, mm4, mm5, mm6, mm7;
	__m64 tmp1, tmp2, tmp3, tmp4;

	mm0 = *((__m64*)&src_u[0]);
	mm1 = *((__m64*)&src_u[8]);
	mm2 = *((__m64*)&src_v[0]);
	mm3 = *((__m64*)&src_v[8]);

	mm4 = mm0;
	mm6 = mm1;
	mm5 = *((__m64*)&src_u[4]);
	mm7 = *((__m64*)&src_u[12]);

	// horizontal
	mm0 = _mm_add_pi16(mm4,mm6);	//m0+m2		u[0] + u[8]
	mm2 = _mm_sub_pi16(_mm_srai_pi16(mm5,1),mm7); //  1/2m1-m3    1/2u[4] - u[12] 
	mm1 = _mm_sub_pi16(mm4,mm6);	//m0-m2		u[0] - u[8]
	mm3 = _mm_add_pi16(mm5,_mm_srai_pi16(mm7,1)); //  m1+1/2m3	u[4] + 1/2u[12]

	mm4 = _mm_add_pi16(mm0,mm3);		//m0+m1+m2+1/2m3 11115555   u[0] + u[8] + u[4] + 1/2u[12]
	mm5 = _mm_add_pi16(mm1,mm2);		//m0+1/2m1-m2-m3 22226666	u[0] - u[8] + 1/2u[4] - u[12]
	mm7 = _mm_sub_pi16(mm0,mm3);     //m0-m1+m2-1/2m3 44448888	u[0] + u[8] - u[4] - 1/2u[12]
	mm6 = _mm_sub_pi16(mm1,mm2);		//m0-1/2m1-m2+m3 33337777	u[0] - u[8] - 1/2u[4] + u[12]

	tmp1 = mm4;
	tmp2 = mm5;
	tmp3 = mm7;
	tmp4 = mm6;

	mm0 = *((__m64*)&src_u[0]);
	mm1 = *((__m64*)&src_u[8]);
	mm2 = *((__m64*)&src_v[0]);
	mm3 = *((__m64*)&src_v[8]);

	mm4 = mm2;
	mm6 = mm3;
	mm5 = *((__m64*)&src_v[4]);
	mm7 = *((__m64*)&src_v[12]);

	// horizontal
	mm0 = _mm_add_pi16(mm4,mm6);	//m0+m2		u[0] + u[8]
	mm2 = _mm_sub_pi16(_mm_srai_pi16(mm5,1),mm7); //  1/2m1-m3    1/2u[4] - u[12] 
	mm1 = _mm_sub_pi16(mm4,mm6);	//m0-m2		u[0] - u[8]
	mm3 = _mm_add_pi16(mm5,_mm_srai_pi16(mm7,1)); //  m1+1/2m3	u[4] + 1/2u[12]

	mm4 = _mm_add_pi16(mm0,mm3);		//m0+m1+m2+1/2m3 11115555   u[0] + u[8] + u[4] + 1/2u[12]
	mm5 = _mm_add_pi16(mm1,mm2);		//m0+1/2m1-m2-m3 22226666	u[0] - u[8] + 1/2u[4] - u[12]
	mm7 = _mm_sub_pi16(mm0,mm3);     //m0-m1+m2-1/2m3 44448888	u[0] + u[8] - u[4] - 1/2u[12]
	mm6 = _mm_sub_pi16(mm1,mm2);		//m0-1/2m1-m2+m3 33337777	u[0] - u[8] - 1/2u[4] + u[12]

	// mm4: (tmp1,mm4) 11115555
	// mm5: (tmp2,mm5) 22226666 
	// mm7: (tmp3,mm7) 44448888
	// mm6: (tmp4,mm6) 33337777


	//transpose
	mm0 =_mm_unpacklo_pi16(tmp1,mm4);	// 1515
	mm1 =_mm_unpackhi_pi16(tmp1,mm4);	// 1'5'1'5'
	mm2 =_mm_unpacklo_pi16(tmp2,mm5);	// 2626
	mm3 =_mm_unpackhi_pi16(tmp2,mm5);	// 2'6'2'6'
	
	mm4 = _mm_unpacklo_pi32(mm0,mm2); // 1526
	mm5 = _mm_unpackhi_pi32(mm0,mm2); // 1526
	mm0 = _mm_unpacklo_pi32(mm1,mm3); // 1526
	mm2 = _mm_unpackhi_pi32(mm1,mm3); // 1526
	
	tmp1 = _mm_unpacklo_pi16(tmp4,mm6); // 3737
	tmp2 = _mm_unpackhi_pi16(tmp4,mm6); // 3'7'3'7'
	tmp4 = _mm_unpacklo_pi16(tmp3,mm7); // 4848
	mm7 = _mm_unpackhi_pi16(tmp3,mm7); // 4'8'4'8'
	
	mm1 = mm0;
	mm3 = mm2;
	mm0 = mm4;
	mm2 = mm5;

	mm5 = mm2;
	mm2 = mm1;
	mm1 = mm5;
	
	mm4 = _mm_unpacklo_pi32(tmp1,tmp4); // 3748
	mm5 = _mm_unpackhi_pi32(tmp1,tmp4); // 3748
	tmp3 = _mm_unpacklo_pi32(tmp2,mm7); // 3748
	tmp4 = _mm_unpackhi_pi32(tmp2,mm7); // 3748

	tmp1 = mm4;
	tmp2 = mm5;
	
	
	//vertical
	mm4 = _mm_add_pi16(mm0,mm2);
	mm6 = _mm_sub_pi16(_mm_srai_pi16(mm1,1),mm3);
	mm5 = _mm_sub_pi16(mm0,mm2);
	mm7 = _mm_add_pi16(mm1,_mm_srai_pi16(mm3,1));


	mm0 = _mm_add_pi16(mm4,mm7);
	mm1 = _mm_add_pi16(mm5,mm6);
	mm3 = _mm_sub_pi16(mm4,mm7); 
	mm2 = _mm_sub_pi16(mm5,mm6);	

	mm7 = *(__m64*) ncoeff_32_8;

	mm0 = _mm_add_pi16(mm0,mm7);
	mm1 = _mm_add_pi16(mm1,mm7);
	mm2 = _mm_add_pi16(mm2,mm7);
	mm3 = _mm_add_pi16(mm3,mm7);

	mm0 = _mm_srai_pi16(mm0,6);
	mm1 = _mm_srai_pi16(mm1,6);
	mm2 = _mm_srai_pi16(mm2,6);
	mm3 = _mm_srai_pi16(mm3,6);	

	//start to add
	mm7 = _mm_setzero_si64();	

	mm4 = *((__m64*)pred);
	mm5 = *((__m64*)(pred+16));

	mm4 = _mm_unpacklo_pi8(mm4, mm7); //to 16bits
	mm5 = _mm_unpacklo_pi8(mm5, mm7); //to 16bits	

	mm0 = _mm_add_pi16 (mm4, mm0);
	mm1 = _mm_add_pi16 (mm5, mm1);		

	mm4 = *((__m64*)(pred+32));
	mm5 = *((__m64*)(pred+48));

	mm4 = _mm_unpacklo_pi8(mm4, mm7); //to 16bits
	mm5 = _mm_unpacklo_pi8(mm5, mm7); //to 16bits	

	mm2 = _mm_add_pi16 (mm4, mm2);
	mm3 = _mm_add_pi16 (mm5, mm3);	

	mm4 = mm0;
	mm5 = mm1;
	mm6 = mm2;
	mm7 = mm3;
	
	mm0 = tmp1;
	mm1 = tmp2;
	mm2 = tmp3;
	mm3 = tmp4;
	
	tmp1 = mm4;   // mm0
	tmp2 = mm5;   // mm1
	tmp3 = mm6;   // mm2
	tmp4 = mm7;   // mm3

	mm4 = _mm_add_pi16(mm0,mm2);
	mm6 = _mm_sub_pi16(_mm_srai_pi16(mm1,1),mm3);
	mm5 = _mm_sub_pi16(mm0,mm2);
	mm7 = _mm_add_pi16(mm1,_mm_srai_pi16(mm3,1));


	mm0 = _mm_add_pi16(mm4,mm7);
	mm1 = _mm_add_pi16(mm5,mm6);
	mm3 = _mm_sub_pi16(mm4,mm7); 
	mm2 = _mm_sub_pi16(mm5,mm6);	

	mm7 = *(__m64*) ncoeff_32_8;

	mm0 = _mm_add_pi16(mm0,mm7);
	mm1 = _mm_add_pi16(mm1,mm7);
	mm2 = _mm_add_pi16(mm2,mm7);
	mm3 = _mm_add_pi16(mm3,mm7);

	mm0 = _mm_srai_pi16(mm0,6);
	mm1 = _mm_srai_pi16(mm1,6);
	mm2 = _mm_srai_pi16(mm2,6);
	mm3 = _mm_srai_pi16(mm3,6);	

	//start to add
	mm7 = _mm_setzero_si64();	

	mm4 = *((__m64*)(pred));
	mm5 = *((__m64*)(pred+16));

	mm4 = _mm_unpackhi_pi8(mm4, mm7); //to 16bits
	mm5 = _mm_unpackhi_pi8(mm5, mm7); //to 16bits	

	mm0 = _mm_add_pi16 (mm4, mm0);
	mm1 = _mm_add_pi16 (mm5, mm1);		

	mm4 = *((__m64*)(pred+32));
	mm5 = *((__m64*)(pred+48));

	mm4 = _mm_unpackhi_pi8(mm4, mm7); //to 16bits
	mm5 = _mm_unpackhi_pi8(mm5, mm7); //to 16bits	

	mm2 = _mm_add_pi16 (mm4, mm2);
	mm3 = _mm_add_pi16 (mm5, mm3);	

	*(__m64*) (&src_u[0]) = mm7;	
	*(__m64*) (&src_u[8]) = mm7;
	*(__m64*) (&src_v[0]) = mm7;	
	*(__m64*) (&src_v[8]) = mm7;
	*(__m64*) (&src_u[4]) = mm7;	
	*(__m64*) (&src_u[12]) = mm7;
	*(__m64*) (&src_v[4]) = mm7;	
	*(__m64*) (&src_v[12]) = mm7;

	mm0 = _mm_packs_pu16 (tmp1, mm0);	
	mm1 = _mm_packs_pu16 (tmp2, mm1);
	mm2 = _mm_packs_pu16 (tmp3, mm2);		
	mm3 = _mm_packs_pu16 (tmp4, mm3);

	*((__m64*)&dest[0]) = mm0;
	*((__m64*)&dest[stride]) = mm1;
	*((__m64*)&dest[stride<<1]) = mm2;
	*((__m64*)&dest[(stride<<1)+stride]) = mm3;
}

void inverse_transform4x4_mergeuv_sse2(unsigned char *dest, byte *pred,
																			 short *src_u, short *src_v, int stride)
{
	static const __declspec(align(16)) short ncoeff_32_8[] = { 32, 32, 32, 32, 32, 32, 32, 32};
	__m128i mm0,  mm1,  mm2,  mm3, mm4, mm5, mm6, mm7;

	mm0 = _mm_load_si128((__m128i*)&src_u[0]);	// u[0] u[1] u[2] u[3] u[4] u[5] u[6] u[7] 
	mm1 = _mm_load_si128((__m128i*)&src_u[8]);  // u[8] u[9] u[10] u[11] u[12] u[13] u[14] u[15] 
	mm2 = _mm_load_si128((__m128i*)&src_v[0]);	// v[0] v[1] v[2] v[3] v[4] v[5] v[6] v[7]
	mm3 = _mm_load_si128((__m128i*)&src_v[8]);	// v[8] v[9] v[10] v[11] v[12] v[13] v[14] v[15] 

	mm4=_mm_unpacklo_epi64(mm0,mm2);	//m0 m0'  u[0] u[1] u[2] u[3] v[0] v[1] v[2] v[3] 
	mm6=_mm_unpacklo_epi64(mm1,mm3);	//m2 m2'  u[8] u[9] u[10] u[11] v[8] v[9] v[10] v[11]
	mm5=_mm_unpackhi_epi64(mm0,mm2);	//m1 m1'  u[4] u[5] u[6] u[7] v[4] v[5] v[6] v[7]
	mm7=_mm_unpackhi_epi64(mm1,mm3);	//m3 m3'  u[12] u[13] u[14] u[15] v[12] v[13] v[14] v[15] 

	// horizontal
	mm0 = _mm_adds_epi16(mm4,mm6);	//m0+m2		u[0] + u[8]
	mm2 = _mm_subs_epi16(_mm_srai_epi16(mm5,1),mm7); //  1/2m1-m3    1/2u[4] - u[12] 
	mm1 = _mm_subs_epi16(mm4,mm6);	//m0-m2		u[0] - u[8]
	mm3 = _mm_adds_epi16(mm5,_mm_srai_epi16(mm7,1)); //  m1+1/2m3	u[4] + 1/2u[12]

	mm4 = _mm_adds_epi16(mm0,mm3);		//m0+m1+m2+1/2m3 11115555   u[0] + u[8] + u[4] + 1/2u[12]
	mm5 = _mm_adds_epi16(mm1,mm2);		//m0+1/2m1-m2-m3 22226666	u[0] - u[8] + 1/2u[4] - u[12]
	mm7 = _mm_subs_epi16(mm0,mm3);     //m0-m1+m2-1/2m3 44448888	u[0] + u[8] - u[4] - 1/2u[12]
	mm6 = _mm_subs_epi16(mm1,mm2);		//m0-1/2m1-m2+m3 33337777	u[0] - u[8] - 1/2u[4] + u[12]

	//transpose
	mm0 =_mm_unpacklo_epi16(mm4,mm5);	// 12121212
	mm2 =_mm_unpacklo_epi16(mm6,mm7); // 34343434
	mm1 =_mm_unpackhi_epi16(mm4,mm5);	// 56565656 
	mm3 =_mm_unpackhi_epi16(mm6,mm7);	// 78787878

	mm4 =_mm_unpacklo_epi32(mm0,mm2);	// 12341234
	mm6 =_mm_unpacklo_epi32(mm1,mm3);  // 56785678
	mm5 =_mm_unpackhi_epi32(mm0,mm2);	// 12341234 
	mm7 =_mm_unpackhi_epi32(mm1,mm3);	// 56785678

	

	mm0 =_mm_unpacklo_epi16(mm4,mm6);	// 15263748 ==> merge uv
	mm2 =_mm_unpacklo_epi16(mm5,mm7);   // 15263748 ==> merge uv
	mm1 =_mm_unpackhi_epi16(mm4,mm6);	// 15263748 ==> merge uv
	mm3 =_mm_unpackhi_epi16(mm5,mm7);	// 15263748 ==> merge uv

	//vertical
	mm4 = _mm_adds_epi16(mm0,mm2);
	mm6 = _mm_subs_epi16(_mm_srai_epi16(mm1,1),mm3);
	mm5 = _mm_subs_epi16(mm0,mm2);
	mm7 = _mm_adds_epi16(mm1,_mm_srai_epi16(mm3,1));


	mm0 = _mm_adds_epi16(mm4,mm7);
	mm1 = _mm_adds_epi16(mm5,mm6);
	mm3 = _mm_subs_epi16(mm4,mm7); 
	mm2 = _mm_subs_epi16(mm5,mm6);	

	mm7 = *(__m128i*) ncoeff_32_8;

	mm0 = _mm_add_epi16(mm0,mm7);
	mm1 = _mm_add_epi16(mm1,mm7);
	mm2 = _mm_add_epi16(mm2,mm7);
	mm3 = _mm_add_epi16(mm3,mm7);

	mm0 = _mm_srai_epi16(mm0,6);
	mm1 = _mm_srai_epi16(mm1,6);
	mm2 = _mm_srai_epi16(mm2,6);
	mm3 = _mm_srai_epi16(mm3,6);	

	//start to add
	mm7 = _mm_setzero_si128();	

	mm4 = _mm_loadu_si128 ((__m128i*)(pred)); //uvuvuvuv... 8bits	
	mm5 = _mm_loadu_si128 ((__m128i*)(pred+16)); //uvuvuvuv... 8bits

	mm4 = _mm_unpacklo_epi8(mm4, mm7); //to 16bits
	mm5 = _mm_unpacklo_epi8(mm5, mm7); //to 16bits	

	mm0 = _mm_add_epi16 (mm4, mm0);
	mm1 = _mm_add_epi16 (mm5, mm1);		

	mm4 = _mm_loadu_si128 ((__m128i*)(pred+32)); //uvuvuvuv... 8bits	
	mm5 = _mm_loadu_si128 ((__m128i*)(pred+48)); //uvuvuvuv... 8bits

	mm4 = _mm_unpacklo_epi8(mm4, mm7); //to 16bits
	mm5 = _mm_unpacklo_epi8(mm5, mm7); //to 16bits	

	mm2 = _mm_add_epi16 (mm4, mm2);
	mm3 = _mm_add_epi16 (mm5, mm3);	

	*(__m128i*) (&src_u[0]) = mm7;	
	*(__m128i*) (&src_u[8]) = mm7;
	*(__m128i*) (&src_v[0]) = mm7;	
	*(__m128i*) (&src_v[8]) = mm7;

	mm0 = _mm_packus_epi16 (mm0, mm0);	
	mm1 = _mm_packus_epi16 (mm1, mm1);
	mm2 = _mm_packus_epi16 (mm2, mm2);		
	mm3 = _mm_packus_epi16 (mm3, mm3);

	_mm_storel_epi64 ((__m128i*)&dest[0], mm0);
	_mm_storel_epi64 ((__m128i*)&dest[stride], mm1);
	_mm_storel_epi64 ((__m128i*)&dest[stride<<1], mm2);
	_mm_storel_epi64 ((__m128i*)&dest[(stride<<1)+stride], mm3);
}

#endif //H264_ENABLE_INTRINSICS
