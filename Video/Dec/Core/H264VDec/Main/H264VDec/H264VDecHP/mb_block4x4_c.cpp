
/**
* @file mb_block4x4_c.cpp
* Implementation of the various 4x4 block level texture transforms 
* and inverse transforms.
*
*/

#include "global.h"
#include "mb_block4x4.h"
#include "clipping.h"

inverse_transform4x4_t *inverse_transform4x4;

/**********************************************
* Auxilliary quantization related parameters *
**********************************************/
#define DQ_SHIFT 6
#define DQ_ADD   32 //(1<<(DQ_SHIFT-1))
#define BSIZE 4
#define QUANT_SHIFT 15

#define BLOCK_SIZE 4

static const unsigned int R = ~((int) 255); //!< auxilliary constant for faster clipping

/**
* Full 4x4 block texture reconstruction from inverse quantized transform coeffs
* and predictions - inverse 4x4 block transform, adding predictions to the result,
* final scaling of the resulting values
* @param dest Destination pixels - the result of the transform
* @param pred Texture predictions (located in 16x16 array)
* @param src Input coefficients (located in 4x4 array)
* @param stride Line offset of the destination pixels
*/
void inverse_transform4x4_c(unsigned char *dest, byte *pred,
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
	


	int m6[4];
	int m7[16];
	int m8[16];
	int i;
	int * mm;
	int * mm_stop;
	int x0, x1, x2, x3;  

#if defined(_PRE_TRANSPOSE_)
	for(i=0; i<4; i++)                
	{
		m8[0*4+i] = src[4*i+0];
		m8[1*4+i] = src[4*i+1];
		m8[2*4+i] = src[4*i+2];
		m8[3*4+i] = src[4*i+3];
	}

	// horizontal
	m6[0] = m8[0]+m8[2];            
	m6[1] = m8[0]-m8[2];
	m6[2] = (m8[1]>>1)-m8[3];
	m6[3] = m8[1]+(m8[3]>>1);
	m7[0] = m6[0]+m6[3];
	m7[12] = m6[0]-m6[3];
	m7[4] = m6[1]+m6[2];
	m7[8] = m6[1]-m6[2];

	m6[0] = m8[4]+m8[6];
	m6[1] = m8[4]-m8[6];
	m6[2] = (m8[5]>>1)-m8[7];
	m6[3] = m8[5]+(m8[7]>>1);
	m7[1] = m6[0]+m6[3];
	m7[13] = m6[0]-m6[3];
	m7[5] = m6[1]+m6[2];
	m7[9] = m6[1]-m6[2];

	m6[0] = m8[8]+m8[10];
	m6[1] = m8[8]-m8[10];
	m6[2] = (m8[9]>>1)-m8[11];
	m6[3] = m8[9]+(m8[11]>>1);
	m7[2] = m6[0]+m6[3];
	m7[14] = m6[0]-m6[3];
	m7[6] = m6[1]+m6[2];
	m7[10] = m6[1]-m6[2];

	m6[0] = m8[12]+m8[14];
	m6[1] = m8[12]-m8[14];
	m6[2] = (m8[13]>>1)-m8[15];
	m6[3] = m8[13]+(m8[15]>>1);
	m7[3] = m6[0]+m6[3];
	m7[15] = m6[0]-m6[3];
	m7[7] = m6[1]+m6[2];
	m7[11] = m6[1]-m6[2];
#else
	m6[0] = src[0]+src[2];            
	m6[1] = src[0]-src[2];
	m6[2] = (src[1]>>1)-src[3];
	m6[3] = src[1]+(src[3]>>1);
	m7[0] = m6[0]+m6[3];
	m7[12] = m6[0]-m6[3];
	m7[4] = m6[1]+m6[2];
	m7[8] = m6[1]-m6[2];

	m6[0] = src[4]+src[6];
	m6[1] = src[4]-src[6];
	m6[2] = (src[5]>>1)-src[7];
	m6[3] = src[5]+(src[7]>>1);
	m7[1] = m6[0]+m6[3];
	m7[13] = m6[0]-m6[3];
	m7[5] = m6[1]+m6[2];
	m7[9] = m6[1]-m6[2];

	m6[0] = src[8]+src[10];
	m6[1] = src[8]-src[10];
	m6[2] = (src[9]>>1)-src[11];
	m6[3] = src[9]+(src[11]>>1);
	m7[2] = m6[0]+m6[3];
	m7[14] = m6[0]-m6[3];
	m7[6] = m6[1]+m6[2];
	m7[10] = m6[1]-m6[2];

	m6[0] = src[12]+src[14];
	m6[1] = src[12]-src[14];
	m6[2] = (src[13]>>1)-src[15];
	m6[3] = src[13]+(src[15]>>1);
	m7[3] = m6[0]+m6[3];
	m7[15] = m6[0]-m6[3];
	m7[7] = m6[1]+m6[2];
	m7[11] = m6[1]-m6[2];

#endif


	memset(src, 0, 16*sizeof(short));

	// vertical
	for (mm = m7, mm_stop = m7 + 16; mm < mm_stop; mm+=4)
	{

		m6[0] = mm[0]+mm[2];
		m6[1] = mm[0]-mm[2];
		m6[2] = (mm[1]>>1)-mm[3];
		m6[3] = mm[1]+(mm[3]>>1);

		x0=((m6[0]+m6[3]+DQ_ADD)>>DQ_SHIFT)+pred[0];
		x1=((m6[1]+m6[2]+DQ_ADD)>>DQ_SHIFT)+pred[16];
		x2=((m6[1]-m6[2]+DQ_ADD)>>DQ_SHIFT)+pred[32];
		x3=((m6[0]-m6[3]+DQ_ADD)>>DQ_SHIFT)+pred[48];
		if (!((x0|x1|x2|x3)&R))
		{
			dest[0]=x0;
			dest[stride]=x1;
			dest[stride<<1]=x2;
			dest[3*stride]=x3;
		}
		else
		{
			dest[0]=CLIP0_255(x0);
			dest[stride]=CLIP0_255(x1);
			dest[stride<<1]=CLIP0_255(x2);
			dest[(stride<<1)+stride]=CLIP0_255(x3);
		}
		dest++;
		pred++;
	}



} 


void inverse_transform4x4_mergeuv_c(unsigned char *dest, byte *pred,
																			 short *src_u, short *src_v, int stride)
{
	int i;
	int m6[32];
	
	int m8[64];

#if defined(_PRE_TRANSPOSE_)
		m8[0] = src_u[0];
		m8[1] = src_u[1];
		m8[2] = src_u[2];
		m8[3] = src_u[3];
		m8[4] = src_u[4];
		m8[5] = src_u[5];
		m8[6] = src_u[6];
		m8[7] = src_u[7];

		m8[8] = src_u[8];
		m8[9] = src_u[9];
		m8[10] = src_u[10];
		m8[11] = src_u[11];
		m8[12] = src_u[12];
		m8[13] = src_u[13];
		m8[14] = src_u[14];
		m8[15] = src_u[15];

		m8[16] = src_v[0];
		m8[17] = src_v[1];
		m8[18] = src_v[2];
		m8[19] = src_v[3];
		m8[20] = src_v[4];
		m8[21] = src_v[5];
		m8[22] = src_v[6];
		m8[23] = src_v[7];

		m8[24] = src_v[8];		// u[0] u[1] u[2] u[3] u[4] u[5] u[6] u[7] 
		m8[25] = src_v[9];		// u[8] u[9] u[10] u[11] u[12] u[13] u[14] u[15] 
		m8[26] = src_v[10];		// v[0] v[1] v[2] v[3] v[4] v[5] v[6] v[7]
		m8[27] = src_v[11];		// v[8] v[9] v[10] v[11] v[12] v[13] v[14] v[15]
		m8[28] = src_v[12];
		m8[29] = src_v[13];
		m8[30] = src_v[14];
		m8[31] = src_v[15];
#else
		m8[0] = src_u[0];
		m8[1] = src_u[4];
		m8[2] = src_u[8];
		m8[3] = src_u[12];
		m8[4] = src_u[1];
		m8[5] = src_u[5];
		m8[6] = src_u[9];
		m8[7] = src_u[13];

		m8[8] = src_u[2];
		m8[9] = src_u[6];
		m8[10] = src_u[10];
		m8[11] = src_u[14];
		m8[12] = src_u[3];
		m8[13] = src_u[7];
		m8[14] = src_u[11];
		m8[15] = src_u[15];

		m8[16] = src_v[0];
		m8[17] = src_v[4];
		m8[18] = src_v[8];
		m8[19] = src_v[12];
		m8[20] = src_v[1];
		m8[21] = src_v[5];
		m8[22] = src_v[9];
		m8[23] = src_v[13];

		m8[24] = src_v[2];		// u[0] u[1] u[2] u[3] u[4] u[5] u[6] u[7] 
		m8[25] = src_v[6];		// u[8] u[9] u[10] u[11] u[12] u[13] u[14] u[15] 
		m8[26] = src_v[10];		// v[0] v[1] v[2] v[3] v[4] v[5] v[6] v[7]
		m8[27] = src_v[14];		// v[8] v[9] v[10] v[11] v[12] v[13] v[14] v[15]
		m8[28] = src_v[3];
		m8[29] = src_v[7];
		m8[30] = src_v[11];
		m8[31] = src_v[15];
		
#endif

		memset(src_u, 0, 16*sizeof(short));
		memset(src_v, 0, 16*sizeof(short));

		m6[0] = m8[16];
		m6[1] = m8[17];
		m6[2] = m8[18];
		m6[3] = m8[19];

		m8[16] = m8[4];		// u[0] u[1] u[2] u[3] u[4] u[5] u[6] u[7] 
		m8[17] = m8[5];		// u[8] u[9] u[10] u[11] u[12] u[13] u[14] u[15] 
		m8[18] = m8[6];		// u[4] u[5] u[6] u[7] v[4] v[5] v[6] v[7]
		m8[19] = m8[7];		// v[8] v[9] v[10] v[11] v[12] v[13] v[14] v[15]

		m8[4] = m6[0];		
		m8[5] = m6[1];
		m8[6] = m6[2];
		m8[7] = m6[3];

		m6[0] = m8[24];
		m6[1] = m8[25];
		m6[2] = m8[26];
		m6[3] = m8[27];

		m8[24] = m8[12];
		m8[25] = m8[13];
		m8[26] = m8[14];
		m8[27] = m8[15];

		m8[12] = m6[0];		//m8:   u[0] u[1] u[2] u[3] v[0] v[1] v[2] v[3]
		m8[13] = m6[1];		//      u[8] u[9] u[10] u[11] v[8] v[9] v[10] v[11]
		m8[14] = m6[2];		//		u[4] u[5] u[6] u[7] v[4] v[5] v[6] v[7]
		m8[15] = m6[3];		//		u[12] u[13] u[14] u[15] v[12] v[13] v[14] v[15] 

		//horizontal
		for(i=0; i<8; i++)
		{
			m8[32+i] = m8[0+i] + m8[8+i];			// u[0] + u[8]
			m8[40+i] = (m8[16+i]>>1) - m8[24+i];	//  1/2u[4] - u[12] 
			m8[48+i] = m8[0+i] - m8[8+i];			// u[0] - u[8]
			m8[56+i] = m8[16+i] + (m8[24+i]>>1)	;	// u[4] + 1/2u[12]
		}

		for(i=0; i<8; i++)
		{
			m6[0+i] = m8[32+i] + m8[56+i];   // 11115555	u[0] + u[8] + u[4] + 1/2u[12]
			m6[8+i] = m8[48+i] + m8[40+i];   // 22226666	u[0] - u[8] + 1/2u[4] - u[12]
			m6[16+i] = m8[48+i] - m8[40+i];	 // 33337777	u[0] - u[8] - 1/2u[4] + u[12]
			m6[24+i] = m8[32+i] - m8[56+i];	 // 44448888	u[0] + u[8] - u[4] - 1/2u[12]
		}

		for(i=0; i<8; i++)
		{
			m8[32+i] =	m6[0+i*4] + m6[2+i*4];		// 0 + 2
			m8[40+i] = (m6[1+i*4]>>1) - m6[3+i*4];	// 1>>1 -3
			m8[48+i] =  m6[0+i*4] - m6[2+i*4];		// 0 - 2
			m8[56+i] =  (m6[3+i*4]>>1) + m6[1+i*4];	// 3>>1 + 1
		}

		for(i=0; i<8; i++)
		{
			m6[0+i] = (m8[32+i] + m8[56+i]+(short)32)>>6;
			m6[8+i] = (m8[40+i] + m8[48+i]+(short)32)>>6;
			m6[24+i] = (m8[32+i] - m8[56+i]+(short)32)>>6;
			m6[16+i] = (m8[48+i] - m8[40+i]+(short)32)>>6;
		}
		for(i=0; i<8; i++)
		{
			m6[0+i] = m6[0+i]+pred[0];
			m6[8+i] = m6[8+i]+pred[16];
			m6[16+i] = m6[16+i]+pred[32];
			m6[24+i] = m6[24+i]+pred[48];

				pred++;


		}

		

		for(i=0; i<8; i++)
		{

			if (!((m6[0+i]|m6[8+i]|m6[16+i]|m6[24+i])&R))
			{
				dest[0]=m6[0+i];
				dest[stride]=m6[8+i];
				dest[stride<<1]=m6[16+i];
				dest[3*stride]=m6[24+i];
			}
			else
			{
				dest[0]=CLIP0_255(m6[0+i]);
				dest[stride]=CLIP0_255(m6[8+i]);
				dest[stride<<1]=CLIP0_255(m6[16+i]);
				dest[(stride<<1)+stride]=CLIP0_255(m6[24+i]);
			}
		
			dest++;

		}
		pred+=8;

}


