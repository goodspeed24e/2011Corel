#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "global.h"

#ifdef H264_ENABLE_INTRINSICS
#include <xmmintrin.h>
#include <emmintrin.h>
#endif

#include "mbuffer.h"
#include "elements.h"
#include "fmo.h"
#include "vlc.h"
#include "image.h"
#include "mb_access.h"
#include "biaridecod.h"
#include "clipping.h"
#include "mb_chroma.h"
#include "transform8x8.h"
#include "mb_block4x4.h"
#include "mb_average.h"
#include "get_block.h"
#include "mb_motcomp.h"

MB_itrans_Luma_t *MB_itrans4x4_Luma;
MB_itrans_Luma_t *MB_itrans8x8_Luma;
MB_itrans_Chroma_t *MB_itrans4x4_Chroma;

extern unsigned char cbp_blk_chroma[8][4];


static int get_block_4xh_iter[8]                      = {                        4,                        4,
4,                        4,
1,                        2,
2,                        4 };
static int get_block_4xh_height[8]                    = {                        4,                        4,
4,                        4,
8,                        4,
8,                        4 };
static int get_block_4xh_b4s[8][4]                    = {            { 0, 1, 2, 3},            { 0, 1, 2, 3},
{ 0, 1, 2, 3},            { 0, 1, 2, 3},
{ 0,-1,-1,-1},            { 0, 2,-1,-1},
{ 0, 1,-1,-1},            { 0, 1, 2, 3} };
// get_block_wxh* get_block_4xh_p[8];
// mb_chroma_2xH_p needs to be set in run-time
// mb_chroma_pred_t *mb_chroma_2xH_p[8];

// average_p needs to be set in run-time
static average_t* average_p[8][2]                     = {   {average_4, average_2},   {average_4, average_2}, 
{average_4, average_2},   {average_4, average_2},
{average_8, average_4},   {average_8, average_4},
{average_4, average_2},   {average_4, average_2}};
// weight_p needs to be set in run-time
static weight_t* weight_p[8][2]                       = {     {weight_4, weight_2},     {weight_4, weight_2}, 
{weight_4, weight_2},     {weight_4, weight_2},
{weight_8, weight_4},     {weight_8, weight_4},
{weight_4, weight_2},     {weight_4, weight_2}};
// weight_b_p needs to be set in run-time
static weight_b_t* weight_b_p[8][2]                   = { {weight_4_b, weight_2_b}, {weight_4_b, weight_2_b},
{weight_4_b, weight_2_b}, {weight_4_b, weight_2_b},
{weight_8_b, weight_4_b}, {weight_8_b, weight_4_b},
{weight_4_b, weight_2_b}, {weight_4_b, weight_2_b}};
static weight_uv_t* weight_uv_p[8] = {weight_2_uv, weight_2_uv, weight_2_uv, weight_2_uv, weight_4_uv, weight_4_uv, weight_2_uv, weight_2_uv};
static weight_b_uv_t* weight_b_uv_p[8] = {weight_2_b_uv, weight_2_b_uv, weight_2_b_uv, weight_2_b_uv, weight_4_b_uv, weight_4_b_uv, weight_2_b_uv, weight_2_b_uv};

void set_mb_comp_function_pointers()
{
	average_p[0][0]    = average_4;
	average_p[1][0]    = average_4;
	average_p[2][0]    = average_4;
	average_p[3][0]    = average_4;
	average_p[4][0]    = average_8;
	average_p[5][0]    = average_8;
	average_p[6][0]    = average_4;
	average_p[7][0]    = average_4;
	average_p[0][1]    = average_2;
	average_p[1][1]    = average_2;
	average_p[2][1]    = average_2;
	average_p[3][1]    = average_2;
	average_p[4][1]    = average_4;
	average_p[5][1]    = average_4;
	average_p[6][1]    = average_2;
	average_p[7][1]    = average_2;

	weight_p[0][0]     = weight_4;
	weight_p[1][0]     = weight_4;
	weight_p[2][0]     = weight_4;
	weight_p[3][0]     = weight_4;
	weight_p[4][0]     = weight_8;
	weight_p[5][0]     = weight_8;
	weight_p[6][0]     = weight_4;
	weight_p[7][0]     = weight_4;
	weight_p[0][1]     = weight_2;
	weight_p[1][1]     = weight_2;
	weight_p[2][1]     = weight_2;
	weight_p[3][1]     = weight_2;
	weight_p[4][1]     = weight_4;
	weight_p[5][1]     = weight_4;
	weight_p[6][1]     = weight_2;
	weight_p[7][1]     = weight_2;

	weight_b_p[0][0]   = weight_4_b;
	weight_b_p[1][0]   = weight_4_b;
	weight_b_p[2][0]   = weight_4_b;
	weight_b_p[3][0]   = weight_4_b;
	weight_b_p[4][0]   = weight_8_b;
	weight_b_p[5][0]   = weight_8_b;
	weight_b_p[6][0]   = weight_4_b;
	weight_b_p[7][0]   = weight_4_b;
	weight_b_p[0][1]   = weight_2_b;
	weight_b_p[1][1]   = weight_2_b;
	weight_b_p[2][1]   = weight_2_b;
	weight_b_p[3][1]   = weight_2_b;
	weight_b_p[4][1]   = weight_4_b;
	weight_b_p[5][1]   = weight_4_b;
	weight_b_p[6][1]   = weight_2_b;
	weight_b_p[7][1]   = weight_2_b;

	//merge uv
	weight_uv_p[0] = weight_2_uv;
	weight_uv_p[1] = weight_2_uv;
	weight_uv_p[2] = weight_2_uv;
	weight_uv_p[3] = weight_2_uv;
	weight_uv_p[4] = weight_4_uv;
	weight_uv_p[5] = weight_4_uv;
	weight_uv_p[6] = weight_2_uv;
	weight_uv_p[7] = weight_2_uv;

	weight_b_uv_p[0] = weight_2_b_uv;
	weight_b_uv_p[1] = weight_2_b_uv;
	weight_b_uv_p[2] = weight_2_b_uv;
	weight_b_uv_p[3] = weight_2_b_uv;
	weight_b_uv_p[4] = weight_4_b_uv;
	weight_b_uv_p[5] = weight_4_b_uv;
	weight_b_uv_p[6] = weight_2_b_uv;
	weight_b_uv_p[7] = weight_2_b_uv;

}

void set_4xH_mc_function_ptr PARGS0()
{
	get_block_4xh_p[0] = get_block_4xh;
	get_block_4xh_p[1] = get_block_4xh;
	get_block_4xh_p[2] = get_block_4xh;
	get_block_4xh_p[3] = get_block_4xh;
	get_block_4xh_p[4] = get_block_8xh;
	get_block_4xh_p[5] = get_block_8xh;
	get_block_4xh_p[6] = get_block_4xh;
	get_block_4xh_p[7] = get_block_4xh;
}

void set_2xH_mc_function_ptr PARGS0()
{
	mb_chroma_2xH_p[0] = mb_chroma_2xH;
	mb_chroma_2xH_p[1] = mb_chroma_2xH;
	mb_chroma_2xH_p[2] = mb_chroma_2xH;
	mb_chroma_2xH_p[3] = mb_chroma_2xH;
	mb_chroma_2xH_p[4] = mb_chroma_4xH;
	mb_chroma_2xH_p[5] = mb_chroma_4xH;
	mb_chroma_2xH_p[6] = mb_chroma_2xH;
	mb_chroma_2xH_p[7] = mb_chroma_2xH;
}

void MB_itrans4x4_Luma_c PARGS2(imgpel * imgY, int stride)
{
	int i,j,jj;	
	int cbp_blk = currMB_s_d->cbp_blk;
#if defined(ONE_COF)
	short *pcof = (short*)IMGPAR cof;
#else
	short *pcof = (short*) IMGPAR cof_d;
#endif
	imgpel *pImgY;

	for(j=0;j<16;j+=4)
	{		
		for(i=0;i<16;i+=4)
		{						
			if( cbp_blk & 1 )
			{
				inverse_transform4x4(imgY+i, //unsigned char *dest,
					&IMGPAR mpr[j][i], // byte *pred,
					pcof, //short *src,
					stride //int stride
					);
			}
			else
			{
				pImgY = imgY+i; 
				for(jj=0;jj<4;jj++)
				{				
					*((long*)(pImgY)) = *((long*)&IMGPAR mpr[jj + j][i]);
					pImgY += stride;				
				}
			}
			cbp_blk >>= 1;
			pcof += 16;
		}
		imgY += (stride<<2);
	}
}

#ifdef H264_ENABLE_INTRINSICS
void MB_itrans4x4_Luma_sse2 PARGS2(imgpel * imgY, int stride)
{
	int i,j,jj;	
	int cbp_blk = currMB_s_d->cbp_blk;
	int idx_a[16] = {0,0,4,4,0,0,4,4,8,8,12,12,8,8,12,12};
	int idx_b[16] = {0,4,0,4,8,12,8,12,0,4,0,4,8,12,8,12};
	int idx_d[16] = {0,1,4,5,2,3,6,7,8,9,12,13,10,11,14,15};

#if defined(ONE_COF)
	short *pcof = (short*)IMGPAR cof;
#else
	short *pcof = (short*) IMGPAR cof_d;
#endif

	int idx[4] = {0,4,8,12};

	if(cbp_blk&0xFFFF)
	{
		for(jj=0;jj<16;jj+=2)
		{
			i = idx[jj&3];
			j = idx[jj>>2];

			if(cbp_blk&3)
			{
				inverse_transform4x4_2_sse2(&IMGPAR mpr[j][i], //unsigned char *dest,
					&IMGPAR mpr[j][i], // byte *pred,
					pcof, //short *src,
					16 //int stride
					);
			}

			cbp_blk>>=2;
			pcof += 32;
		}
	}

	__m128i xmm0;
	for (jj=0; jj<16; jj++)
	{
		xmm0 = _mm_load_si128((__m128i *)&IMGPAR mpr[jj][0]);
		_mm_store_si128((__m128i *)(imgY), xmm0);
		imgY += stride;
	}
}

void MB_itrans4x4_Luma_sse PARGS2(imgpel * imgY, int stride)
{
	int i,j,jj;	
	int cbp_blk = currMB_s_d->cbp_blk;
	int idx_a[16] = {0,0,4,4,0,0,4,4,8,8,12,12,8,8,12,12};
	int idx_b[16] = {0,4,0,4,8,12,8,12,0,4,0,4,8,12,8,12};
	int idx_d[16] = {0,1,4,5,2,3,6,7,8,9,12,13,10,11,14,15};

#if defined(ONE_COF)
	short *pcof = (short*)IMGPAR cof;
#else
	short *pcof = (short*) IMGPAR cof_d;
#endif

	int idx[4] = {0,4,8,12};

	if(cbp_blk&0xFFFF)
	{
		for(jj=0;jj<16;jj+=2)
		{
			i = idx[jj&3];
			j = idx[jj>>2];

			if(cbp_blk&3)
			{	
				inverse_transform4x4_2_sse(&IMGPAR mpr[j][i], //unsigned char *dest,
					&IMGPAR mpr[j][i], // byte *pred,
					pcof, //short *src,
					16 //int stride
					);
			}

			cbp_blk>>=2;
			pcof += 32;
		}
	}

	//__m128i xmm0;
	__m64 mm0, mm1;
	for (jj=0; jj<16; jj++)
	{
		//xmm0 = _mm_load_si128((__m128i *)&IMGPAR mpr[jj][0]);
		//_mm_store_si128((__m128i *)(imgY), xmm0);
		mm0 = *((__m64*)&IMGPAR mpr[jj][0]);
		mm1 = *((__m64*)&IMGPAR mpr[jj][8]);
		*((__m64*)imgY) = mm0;
		*((__m64*)(imgY+8)) = mm1;
		imgY += stride;
	}
}

#endif //H264_ENABLE_INTRINSICS

void MB_itrans4x4_Chroma_c PARGS2(imgpel * imgUV, int stride_UV)
{

	int k;
	int cbp_blk = currMB_s_d->cbp_blk>>16;
	int b8;
	static const unsigned char idx[4] = {0,8,64,72};

	unsigned char *pmprUV = (unsigned char *)IMGPAR mprUV;	

	if(cbp_blk&0xFF)
	{
		for(b8=0;b8<4;b8++)
		{
			if(cbp_blk&0x11)
			{			
				k = idx[b8];
	
				
				inverse_transform4x4_mergeuv_c(pmprUV+k, //unsigned char *dest,
					pmprUV+k, // byte *pred,
#if defined(ONE_COF)
					&IMGPAR cof[4][b8][0][0], //short *src,
					&IMGPAR cof[5][b8][0][0], //short *src,
#else
					(IMGPAR cof_d + (256+(b8<<4))),  //short *src_u,
					(IMGPAR cof_d + (320+(b8<<4))),  //short *src_v,
#endif
					16 //int stride
					);
			}
			cbp_blk >>= 1;
		}
	}

	for (k=0; k<8; k++)
	{
		int j;
		for (j=0; j<16; j++)
			imgUV[j] = IMGPAR mprUV[k][j];
		imgUV += stride_UV;
	}

}

#ifdef H264_ENABLE_INTRINSICS
void MB_itrans4x4_Chroma_mmx PARGS2(imgpel * imgUV, int stride_UV)
{
	int k;
	int cbp_blk = currMB_s_d->cbp_blk>>16;
	int b8;
	static const unsigned char idx[4] = {0,8,64,72};

	unsigned char *pmprUV = (unsigned char *)IMGPAR mprUV;	

	if(cbp_blk&0xFF)
	{
		for(b8=0;b8<4;b8++)
		{
			if(cbp_blk&0x11)
			{			
				k = idx[b8];
				inverse_transform4x4_mergeuv_sse(pmprUV+k, //unsigned char *dest,
					pmprUV+k, // byte *pred,
#if defined(ONE_COF)
					&IMGPAR cof[4][b8][0][0], //short *src,
					&IMGPAR cof[5][b8][0][0], //short *src,
#else
					(IMGPAR cof_d + (256+(b8<<4))),  //short *src_u,
					(IMGPAR cof_d + (320+(b8<<4))),  //short *src_v,
#endif
					16 //int stride
					);
			}
			cbp_blk >>= 1;
		}
	}

	//__m128i xmm0;
	__m64 mm0, mm1;
	for (k=0; k<8; k++)
	{
		//xmm0 = _mm_load_si128((__m128i *)&IMGPAR mprUV[k][0]);
		//_mm_store_si128((__m128i *)(imgUV), xmm0);
		mm0 = *((__m64*)&IMGPAR mprUV[k][0]);
		mm1 = *((__m64*)&IMGPAR mprUV[k][8]);
		*((__m64*)imgUV) = mm0;
		*((__m64*)(imgUV+8)) = mm1;
		imgUV += stride_UV;
	}	
}
void MB_itrans4x4_Chroma_sse2 PARGS2(imgpel * imgUV, int stride_UV)
{
	int k;
	int cbp_blk = currMB_s_d->cbp_blk>>16;
	int b8;
	static const unsigned char idx[4] = {0,8,64,72};

	unsigned char *pmprUV = (unsigned char *)IMGPAR mprUV;	

	if(cbp_blk&0xFF)
	{
		for(b8=0;b8<4;b8++)
		{
			if(cbp_blk&0x11)
			{			
				k = idx[b8];
				inverse_transform4x4_mergeuv_sse2(pmprUV+k, //unsigned char *dest,
					pmprUV+k, // byte *pred,
#if defined(ONE_COF)
					&IMGPAR cof[4][b8][0][0], //short *src,
					&IMGPAR cof[5][b8][0][0], //short *src,
#else
					(IMGPAR cof_d + (256+(b8<<4))),  //short *src_u,
					(IMGPAR cof_d + (320+(b8<<4))),  //short *src_v,
#endif
					16 //int stride
					);
			}
			cbp_blk >>= 1;
		}
	}

	__m128i xmm0;
	for (k=0; k<8; k++)
	{
		xmm0 = _mm_load_si128((__m128i *)&IMGPAR mprUV[k][0]);
		_mm_store_si128((__m128i *)(imgUV), xmm0);
		imgUV += stride_UV;
	}	
}
#endif // H264_ENABLE_ASM

void MB_itrans8x8_Luma_c PARGS2(imgpel * imgY, int stride)
{
	int ii,jj;	
	int ioff,joff;
	int b8;
	int offset;

	for (b8=0; b8<4; b8++)
	{
		//Luma
		ioff = (b8  & 1)<<3;
		joff = (b8 >> 1)<<3;

		offset = joff*stride + ioff;

		if (currMB_d->cbp&(1<<b8))
		{	
#if defined(ONE_COF)
			inverse_transform8x8 ARGS4(imgY + offset,
				&IMGPAR mpr[joff][ioff], &IMGPAR cof[b8][0][0][0],stride);
#else
			inverse_transform8x8 ARGS4(imgY + offset,
				&IMGPAR mpr[joff][ioff], (IMGPAR cof_d+ (b8<<6)), stride);
#endif
			//memset(&IMGPAR cof[b8][0][0][0],0,8*8*sizeof(IMGPAR cof[0][0][0][0]));
		}
		else
		{
			for(jj=0;jj<8;jj++,offset+=stride)
			{
				for(ii=0;ii<8;ii++)
				{
					*(imgY+offset+ii)=IMGPAR mpr[jj + joff][ii + ioff]; // construct picture from 4x4 blocks
				}
			}	 
		}
	}
}

#ifdef H264_ENABLE_INTRINSICS
void MB_itrans8x8_Luma_sse2 PARGS2(imgpel * imgY, int stride)
{
	int i, j;
	int cbp_blk = currMB_d->cbp;
	static const int lookup_s[4] = {0, 8, 128, 136};
	int lookup_d[4] = {0, 1, stride, stride+1};

	for (i=0; i<4; i++)
	{
		//unsigned char *dptr = imgY + ((i&1)<<3) + ((i&2)<<2)*stride;
		unsigned char *dptr = &imgY[lookup_d[i]<<3];
		unsigned char *sptr = &IMGPAR mpr[0][0] + lookup_s[i];
		if (cbp_blk & 1)
		{
#if defined(ONE_COF)
			inverse_transform8x8 ARGS4(dptr, sptr, &IMGPAR cof[i][0][0][0], stride);
#else		
			inverse_transform8x8 ARGS4(dptr, sptr,(IMGPAR cof_d+ (i<<6)), stride);
#endif
		}
		else
		{
			__m64 mm0;
			for(j=0;j<8;j++)
			{
				mm0 = *(__m64 *) sptr;
				*(__m64 *) dptr = mm0;
				sptr += 16;
				dptr += stride;
			}
		}
		cbp_blk >>= 1;
	}
}

void MB_itrans8x8_Luma_sse PARGS2(imgpel * imgY, int stride)
{
	int i, j;
	int cbp_blk = currMB_d->cbp;
	static const int lookup_s[4] = {0, 8, 128, 136};
	int lookup_d[4] = {0, 1, stride, stride+1};

	for (i=0; i<4; i++)
	{
		//unsigned char *dptr = imgY + ((i&1)<<3) + ((i&2)<<2)*stride;
		unsigned char *dptr = &imgY[lookup_d[i]<<3];
		unsigned char *sptr = &IMGPAR mpr[0][0] + lookup_s[i];
		if (cbp_blk & 1)
		{
#if defined(ONE_COF)
			inverse_transform8x8 ARGS4(dptr, sptr, &IMGPAR cof[i][0][0][0], stride);
#else		
			inverse_transform8x8 ARGS4(dptr, sptr,(IMGPAR cof_d+ (i<<6)), stride);
#endif
		}
		else
		{
			__m64 mm0;
			for(j=0;j<8;j++)
			{
				mm0 = *(__m64 *) sptr;
				*(__m64 *) dptr = mm0;
				sptr += 16;
				dptr += stride;
			}
		}
		cbp_blk >>= 1;
	}
}
#endif //H264_ENABLE_INTRINSICS

CREL_RETURN MB_I4MB_Luma PARGS2(imgpel * imgY, int stride)
{
	int k,jj;	
	int ioff,joff;	
	CREL_RETURN ret;
	unsigned long cbp_blk = currMB_s_d->cbp_blk;
#if defined(ONE_COF)
	short *pcof = (short*)IMGPAR cof;
#else
	short *pcof = (short*)IMGPAR cof_d;
#endif
	const int idx1[4] = {0,4,8,12};	

	for(k=0;k<16;k++)
	{
		ioff = idx1[k&3];
		joff = idx1[k>>2];		

		//===== INTRA PREDICTION =====
		ret = intrapred ARGS2(ioff,joff);  /* make 4x4 prediction block mpr from given prediction IMGPAR mb_mode */
		if (FAILED(ret)) {
			return ret;// SEARCH_SYNC;																			/* bit error */
		}

		

		if( cbp_blk & (1<<k) )
		{
			inverse_transform4x4(&IMGPAR mpr[joff][ioff], //unsigned char *dest,
				&IMGPAR mpr[joff][ioff], // byte *pred,
				pcof, //short *src,				
				16 //int stride
				);
		}		

		pcof += 16;	
	}

	for (jj=0; jj<16; jj++)
	{
		memcpy(imgY,&IMGPAR mpr[jj][0],16);
		imgY += stride;
	}
	return CREL_OK;
}

CREL_RETURN MB_I8MB_Luma PARGS2(imgpel * imgY,
																int stride)
{
	int jj;	
	int ioff,joff;
	int b8;
	int cbp=currMB_d->cbp;
	CREL_RETURN ret;

	for(b8=0;b8<4;b8++)
	{
		//=========== 8x8 BLOCK TYPE ============
		ioff = (b8&1)<<3;
		joff = (b8>>1)<<3;

		//PREDICTION
		ret = intrapred8x8 ARGS1(b8);		
		if (FAILED(ret)) {
			return ret;
		}

		// use cbp to determine if we need transform	
		if (cbp&(1<<b8))
		{
#if defined(ONE_COF)
			inverse_transform8x8 ARGS4(&IMGPAR mpr[joff][ioff],
				&IMGPAR mpr[joff][ioff], &IMGPAR cof[b8][0][0][0], 16);
#else
			inverse_transform8x8 ARGS4(&IMGPAR mpr[joff][ioff],
				&IMGPAR mpr[joff][ioff], (IMGPAR cof_d+(b8<<6)), 16);
#endif			
		}
	}

	for (jj=0; jj<16; jj++)
	{
		memcpy(imgY,&IMGPAR mpr[jj][0],16);
		imgY += stride;
	}

	return CREL_OK;
}

CREL_RETURN MB_I16MB_Luma PARGS2(imgpel * imgY, int stride)
{
	CREL_RETURN ret;
	ret =intrapred_luma_16x16 ARGS1(currMB_d->i16mode);
	if (FAILED(ret)) {
		return ret;
	}

	MB_itrans4x4_Luma ARGS2(imgY, stride);
	return CREL_OK;
}

CREL_RETURN MB_I4MB_Chroma PARGS2(imgpel * imgUV, int stride_UV)
{
	CREL_RETURN ret;
	ret = intrapred_chroma ARGS0();
	if (FAILED(ret)) {
		return ret;
	}
	MB_itrans4x4_Chroma ARGS2(imgUV, stride_UV);

	return CREL_OK;

}

void MB_InterPred16x16 PARGS3(int vec_x_base,
															int vec_y_base,
															int list_offset)
{
	int pred_dir = currMB_d->b8pdir[0];
	int ref_idx, ref_idx_bw;
	int mv_x, mv_y, mv_x_bw, mv_y_bw;	

	if(pred_dir==2)
	{
		ref_idx = currMB_s_d->pred_info.ref_idx[LIST_0][0];	
		//To conceal the error from reference index or mb sub_type
		if (ref_idx < 0) {
			ref_idx = currMB_s_d->pred_info.ref_idx[LIST_0][0] = 0;	
		}

		mv_x = currMB_s_d->pred_info.mv[LIST_0][0].x;
		mv_y = currMB_s_d->pred_info.mv[LIST_0][0].y;
		//forward
		get_block_16xh ARGS7(listX[list_offset + LIST_0][ref_idx], vec_x_base, vec_y_base, 
			mv_x, mv_y, 
			reinterpret_cast<unsigned char(*)[16]>(&IMGPAR mpr[0][0]), 16);

		//backward
		ref_idx_bw = currMB_s_d->pred_info.ref_idx[LIST_1][0];
		//To conceal the error from reference index or mb type/sub_type
		if (ref_idx_bw < 0) {
			ref_idx_bw = currMB_s_d->pred_info.ref_idx[LIST_1][0] = 0;	
		}
		mv_x_bw = currMB_s_d->pred_info.mv[LIST_1][0].x;
		mv_y_bw = currMB_s_d->pred_info.mv[LIST_1][0].y;

		get_block_16xh ARGS7(listX[list_offset + LIST_1][ref_idx_bw], vec_x_base, vec_y_base, 
			mv_x_bw, mv_y_bw, 
			reinterpret_cast<unsigned char(*)[16]>(&bw_block[0][0]), 16);
		average_16(&IMGPAR mpr[0][0], 16, (imgpel *) &IMGPAR mpr[0][0], (imgpel *) bw_block, LUMA_BLOCK_SIZE, LUMA_BLOCK_SIZE);

#ifdef __SUPPORT_YUV400__
		if (dec_picture->chroma_format_idc != YUV400)
		{
#endif
			vec_x_base >>= 1;
			vec_y_base >>= 1;
			//mv_y += listX[list_offset][ref_idx]->chroma_vector_adjustment;
			mv_y += IMGPAR cr_vector_adjustment[list_offset][ref_idx];

			mb_chroma_8xH ARGS8(listX[list_offset][ref_idx], &IMGPAR mprUV[0][0], 16, 
				vec_x_base, vec_y_base, mv_x, mv_y, 8);

			//mv_y_bw += listX[list_offset + 1][ref_idx_bw]->chroma_vector_adjustment;
			mv_y_bw += IMGPAR cr_vector_adjustment[list_offset + 1][ref_idx_bw];

			mb_chroma_8xH ARGS8(listX[list_offset+1][ref_idx_bw], (unsigned char *)bw_block, 16,
				vec_x_base, vec_y_base, mv_x_bw, mv_y_bw, 8);

			average_16(&IMGPAR mprUV[0][0], 16, (imgpel *) &IMGPAR mprUV[0][0], (imgpel *) bw_block+0, LUMA_BLOCK_SIZE, 8);
#ifdef __SUPPORT_YUV400__
		}
#endif
	}
	else
	{
		ref_idx = currMB_s_d->pred_info.ref_idx[LIST_0+pred_dir][0];
		if (ref_idx < 0) {
			ref_idx = currMB_s_d->pred_info.ref_idx[LIST_0+pred_dir][0] = 0;
		}
		mv_x = currMB_s_d->pred_info.mv[LIST_0+pred_dir][0].x;
		mv_y = currMB_s_d->pred_info.mv[LIST_0+pred_dir][0].y;

		get_block_16xh ARGS7(listX[list_offset + pred_dir][ref_idx], vec_x_base, vec_y_base, 
			mv_x, mv_y, 
			reinterpret_cast<unsigned char(*)[16]>(&IMGPAR mpr[0][0]), 16);

#ifdef __SUPPORT_YUV400__
		if (dec_picture->chroma_format_idc != YUV400)
		{
#endif
			//mv_y += listX[list_offset + pred_dir][ref_idx]->chroma_vector_adjustment;
			mv_y += IMGPAR cr_vector_adjustment[list_offset + pred_dir][ref_idx];

			mb_chroma_8xH ARGS8(listX[list_offset + pred_dir][ref_idx], &IMGPAR mprUV[0][0], 16,
				vec_x_base>>1, vec_y_base>>1, mv_x, mv_y, 8);

#ifdef __SUPPORT_YUV400__
		}
#endif
	}
}

void MB_InterPred16x8 PARGS3(int vec_x_base,
														 int vec_y_base,
														 int list_offset)
{
	int joff, k;
	int pred_dir;
	int ref_idx, ref_idx_bw;
	int mv_x, mv_y, mv_x_bw, mv_y_bw;	

	for(k=0;k<2;k++)
	{
		joff = (k<<3);
		pred_dir = currMB_d->b8pdir[k<<1];
		if(pred_dir==2)
		{
			//forward
			ref_idx = currMB_s_d->pred_info.ref_idx[LIST_0][k<<1];
			if (ref_idx < 0) {
				ref_idx = currMB_s_d->pred_info.ref_idx[LIST_0][k<<1] = 0;
			}
			mv_x = currMB_s_d->pred_info.mv[LIST_0][joff].x;
			mv_y = currMB_s_d->pred_info.mv[LIST_0][joff].y;

			get_block_16xh ARGS7(listX[list_offset + LIST_0][ref_idx], vec_x_base, vec_y_base+joff, 
				mv_x, mv_y, 
				reinterpret_cast<unsigned char(*)[16]>(&IMGPAR mpr[joff][0]), 8);

			//backward
			ref_idx_bw = currMB_s_d->pred_info.ref_idx[LIST_1][k<<1];
			if ( ref_idx_bw < 0 ){
				ref_idx_bw = currMB_s_d->pred_info.ref_idx[LIST_1][k<<1] = 0;
			}
			mv_x_bw = currMB_s_d->pred_info.mv[LIST_1][joff].x;
			mv_y_bw = currMB_s_d->pred_info.mv[LIST_1][joff].y;

			get_block_16xh ARGS7(listX[list_offset + LIST_1][ref_idx_bw], vec_x_base, vec_y_base+joff, 
				mv_x_bw, mv_y_bw,
				reinterpret_cast<unsigned char(*)[16]>(&bw_block[joff][0]), 8);

			average_16(&IMGPAR mpr[joff][0], 16, (imgpel *) &IMGPAR mpr[joff][0], (imgpel *) &bw_block[joff][0], LUMA_BLOCK_SIZE, 8);			

#ifdef __SUPPORT_YUV400__
			if (dec_picture->chroma_format_idc != YUV400)
			{
#endif
				int chroma_vec_x_base = (vec_x_base>>1);
				joff >>= 1;
				int chroma_vec_y_base = (vec_y_base>>1)+joff;
				//mv_y += listX[list_offset][ref_idx]->chroma_vector_adjustment;
				mv_y += IMGPAR cr_vector_adjustment[list_offset][ref_idx];

				mb_chroma_8xH ARGS8(listX[list_offset][ref_idx], &IMGPAR mprUV[joff][0], 16,
					chroma_vec_x_base, chroma_vec_y_base, mv_x, mv_y, 4);			

				//mv_y_bw += listX[list_offset + 1][ref_idx_bw]->chroma_vector_adjustment;
				mv_y_bw += IMGPAR cr_vector_adjustment[list_offset + 1][ref_idx_bw];

				mb_chroma_8xH ARGS8(listX[list_offset + 1][ref_idx_bw], (unsigned char *)bw_block, 16,
					chroma_vec_x_base, chroma_vec_y_base, mv_x_bw, mv_y_bw, 4);

				average_16(&IMGPAR mprUV[joff][0], 16, (imgpel *) &IMGPAR mprUV[joff][0], (imgpel *) bw_block+0, LUMA_BLOCK_SIZE, 4);
#ifdef __SUPPORT_YUV400__
			}
#endif
		}
		else
		{
			ref_idx = currMB_s_d->pred_info.ref_idx[LIST_0+pred_dir][k<<1];
			if (ref_idx < 0) {
				ref_idx = currMB_s_d->pred_info.ref_idx[LIST_0+pred_dir][k<<1] = 0;
			}
			mv_x = currMB_s_d->pred_info.mv[LIST_0+pred_dir][joff].x;
			mv_y = currMB_s_d->pred_info.mv[LIST_0+pred_dir][joff].y;

			get_block_16xh ARGS7(listX[list_offset + pred_dir][ref_idx], vec_x_base, vec_y_base+joff, 
				mv_x, mv_y, 
				reinterpret_cast<unsigned char(*)[16]>(&IMGPAR mpr[joff][0]), 8);

#ifdef __SUPPORT_YUV400__
			if (dec_picture->chroma_format_idc != YUV400)
			{
#endif
				int chroma_vec_x_base = (vec_x_base>>1);
				joff >>= 1;
				int chroma_vec_y_base = (vec_y_base>>1)+joff;
				//mv_y += listX[list_offset + pred_dir][ref_idx]->chroma_vector_adjustment;
				mv_y += IMGPAR cr_vector_adjustment[list_offset + pred_dir][ref_idx];

				mb_chroma_8xH ARGS8(listX[list_offset + pred_dir][ref_idx], &IMGPAR mprUV[joff][0], 16,
					chroma_vec_x_base, chroma_vec_y_base, mv_x, mv_y, 4);

#ifdef __SUPPORT_YUV400__
			}
#endif
		}
	}
}

void MB_InterPred8x16 PARGS3(int vec_x_base,
														 int vec_y_base,
														 int list_offset)
{
	int i4, ioff, k;
	int pred_dir;
	int ref_idx, ref_idx_bw;
	int mv_x, mv_y, mv_x_bw, mv_y_bw;	

	for(k=0;k<2;k++)
	{
		i4 = (k<<1);
		ioff = (i4<<2);
		pred_dir = currMB_d->b8pdir[k];
		if(pred_dir==2)
		{
			//forward
			ref_idx = currMB_s_d->pred_info.ref_idx[LIST_0][k];
			if (ref_idx < 0) {
				ref_idx = currMB_s_d->pred_info.ref_idx[LIST_0][k] = 0;
			}
			mv_x = currMB_s_d->pred_info.mv[LIST_0][i4].x;
			mv_y = currMB_s_d->pred_info.mv[LIST_0][i4].y;
			get_block_8xh ARGS7(listX[list_offset + LIST_0][ref_idx], vec_x_base+ioff, vec_y_base, 
				mv_x, mv_y, 
				reinterpret_cast<unsigned char(*)[16]>(&IMGPAR mpr[0][ioff]), 16);

			//backward
			ref_idx_bw = currMB_s_d->pred_info.ref_idx[LIST_1][k];
			if (ref_idx_bw < 0) {
				ref_idx_bw = currMB_s_d->pred_info.ref_idx[LIST_1][k] = 0;
			}
			mv_x_bw = currMB_s_d->pred_info.mv[LIST_1][i4].x;
			mv_y_bw = currMB_s_d->pred_info.mv[LIST_1][i4].y;

			get_block_8xh ARGS7(listX[list_offset + LIST_1][ref_idx_bw], vec_x_base+ioff, vec_y_base, 
				mv_x_bw, mv_y_bw,
				reinterpret_cast<unsigned char(*)[16]>(&bw_block[0][ioff]), 16);

			average_8(&IMGPAR mpr[0][ioff], 16, (imgpel *) &IMGPAR mpr[0][ioff], (imgpel *) &bw_block[0][ioff], LUMA_BLOCK_SIZE, LUMA_BLOCK_SIZE);

#ifdef __SUPPORT_YUV400__
			if (dec_picture->chroma_format_idc != YUV400)
			{
#endif
				//chroma
				ioff >>= 1;
				int chroma_vec_x_base = (vec_x_base>>1)+ioff;
				int chroma_vec_y_base = (vec_y_base>>1);
				//mv_y += listX[list_offset][ref_idx]->chroma_vector_adjustment;
				mv_y += IMGPAR cr_vector_adjustment[list_offset][ref_idx];

				mb_chroma_4xH ARGS8(listX[list_offset][ref_idx], &IMGPAR mprUV[0][ioff<<1], 16,
					chroma_vec_x_base, chroma_vec_y_base, mv_x, mv_y, 8);			

				//mv_y_bw += listX[list_offset + 1][ref_idx_bw]->chroma_vector_adjustment;
				mv_y_bw += IMGPAR cr_vector_adjustment[list_offset + 1][ref_idx_bw];

				mb_chroma_4xH ARGS8(listX[list_offset + 1][ref_idx_bw], (unsigned char *)bw_block, 16,
					chroma_vec_x_base, chroma_vec_y_base, mv_x_bw, mv_y_bw, 8);

				average_8(&IMGPAR mprUV[0][ioff<<1], 16, (imgpel *) &IMGPAR mprUV[0][ioff<<1], (imgpel *) bw_block,   LUMA_BLOCK_SIZE, 8);
#ifdef __SUPPORT_YUV400__
			}
#endif
		}
		else
		{
			ref_idx = currMB_s_d->pred_info.ref_idx[LIST_0+pred_dir][k];
			if (ref_idx < 0) {
				ref_idx = currMB_s_d->pred_info.ref_idx[LIST_0+pred_dir][k] = 0;
			}

			mv_x = currMB_s_d->pred_info.mv[LIST_0+pred_dir][i4].x;
			mv_y = currMB_s_d->pred_info.mv[LIST_0+pred_dir][i4].y;

			get_block_8xh ARGS7(listX[list_offset + pred_dir][ref_idx], vec_x_base+ioff, vec_y_base, 
				mv_x, mv_y, 
				reinterpret_cast<unsigned char(*)[16]>(&IMGPAR mpr[0][ioff]), 16);

#ifdef __SUPPORT_YUV400__
			if (dec_picture->chroma_format_idc != YUV400)
			{
#endif
				ioff >>= 1;
				int chroma_vec_x_base = (vec_x_base>>1)+ioff;
				int chroma_vec_y_base = (vec_y_base>>1);
				//mv_y += listX[list_offset + pred_dir][ref_idx]->chroma_vector_adjustment;
				mv_y += IMGPAR cr_vector_adjustment[list_offset + pred_dir][ref_idx];

				mb_chroma_4xH ARGS8(listX[list_offset + pred_dir][ref_idx], &IMGPAR mprUV[0][ioff<<1], 16,
					chroma_vec_x_base, chroma_vec_y_base, mv_x, mv_y, 8);

#ifdef __SUPPORT_YUV400__
			}
#endif
		}
	}
}


//For weighting
void MB_InterPred16x16_1 PARGS3(int vec_x_base, int vec_y_base, int list_offset)
{
	int pred_dir = currMB_d->b8pdir[0];
	int ref_idx, fw_ref_idx, bw_ref_idx;
	int mv_x, mv_y, mv_x_bw, mv_y_bw;
	//for weighting
	int ref_idx_w, fw_ref_idx_w, bw_ref_idx_w;
	int curr_mb_field = ((IMGPAR MbaffFrameFlag)&&(currMB_d->mb_field));
	int weight;
	int weight_u, weight_v;
	int rounding_offset;
	unsigned int down_shift;
	int final_offset;
	int final_offset_u, final_offset_v;

	if(pred_dir==2)
	{
		fw_ref_idx_w = fw_ref_idx = currMB_s_d->pred_info.ref_idx[LIST_0][0];
		if (currMB_s_d->pred_info.ref_idx[LIST_0][0] < 0 ) {
			fw_ref_idx_w = fw_ref_idx = currMB_s_d->pred_info.ref_idx[LIST_0][0] = 0;
		}
		bw_ref_idx_w = bw_ref_idx = currMB_s_d->pred_info.ref_idx[LIST_1][0];
		if (currMB_s_d->pred_info.ref_idx[LIST_1][0] < 0) {
			bw_ref_idx_w = bw_ref_idx = currMB_s_d->pred_info.ref_idx[LIST_1][0] = 0;
		}
		int alpha_fw, alpha_bw;
		int alpha_fw_u, alpha_bw_u, alpha_fw_v, alpha_bw_v;		
		int wt_list_offset = (active_pps.weighted_bipred_idc==2)?list_offset:0;
		if (active_pps.weighted_bipred_idc==1 && curr_mb_field)
		{
			fw_ref_idx_w >>=1;
			bw_ref_idx_w >>=1;
		}

		//Luma
		//forward	
		mv_x = currMB_s_d->pred_info.mv[LIST_0][0].x;
		mv_y = currMB_s_d->pred_info.mv[LIST_0][0].y;

		get_block_16xh ARGS7(listX[list_offset + LIST_0][fw_ref_idx], vec_x_base, vec_y_base, 
			mv_x, mv_y, 
			reinterpret_cast<unsigned char(*)[16]>(&IMGPAR mpr[0][0]), 16);

		//backward
		mv_x_bw = currMB_s_d->pred_info.mv[LIST_1][0].x;
		mv_y_bw = currMB_s_d->pred_info.mv[LIST_1][0].y;

		get_block_16xh ARGS7(listX[list_offset + LIST_1][bw_ref_idx], vec_x_base, vec_y_base, 
			mv_x_bw, mv_y_bw, 
			reinterpret_cast<unsigned char(*)[16]>(&bw_block[0][0]), 16);

		//weighting
		rounding_offset = (1<<IMGPAR luma_log2_weight_denom);
		down_shift      = (IMGPAR luma_log2_weight_denom+1);

		alpha_fw = *(IMGPAR wbp_weight+(((0+wt_list_offset)*MAX_REFERENCE_PICTURES+fw_ref_idx_w)*MAX_REFERENCE_PICTURES+bw_ref_idx_w)*3+0);
		alpha_bw = *(IMGPAR wbp_weight+(((1+wt_list_offset)*MAX_REFERENCE_PICTURES+fw_ref_idx_w)*MAX_REFERENCE_PICTURES+bw_ref_idx_w)*3+0);
		final_offset	= (((*(IMGPAR wp_offset+((wt_list_offset+0)*MAX_REFERENCE_PICTURES+fw_ref_idx_w)*3+0)) + 
			(*(IMGPAR wp_offset+((wt_list_offset+1)*MAX_REFERENCE_PICTURES+bw_ref_idx_w)*3+0)) + 1) >>1);

		weight_16_b(&IMGPAR mpr[0][0], LUMA_BLOCK_SIZE, &IMGPAR mpr[0][0], &bw_block[0][0], alpha_fw, alpha_bw, rounding_offset, down_shift, final_offset, LUMA_BLOCK_SIZE, 16);

#ifdef __SUPPORT_YUV400__
		if (dec_picture->chroma_format_idc != YUV400)
		{
#endif
			//Chroma
			vec_x_base >>= 1;
			vec_y_base >>= 1;
			//mv_y += listX[list_offset][fw_ref_idx]->chroma_vector_adjustment;
			mv_y += IMGPAR cr_vector_adjustment[list_offset][fw_ref_idx];

			mb_chroma_8xH ARGS8(listX[list_offset][fw_ref_idx], &IMGPAR mprUV[0][0], 16,
				vec_x_base, vec_y_base, mv_x, mv_y, 8);

			//mv_y_bw += listX[list_offset + 1][bw_ref_idx]->chroma_vector_adjustment;
			mv_y_bw += IMGPAR cr_vector_adjustment[list_offset + 1][bw_ref_idx];

			mb_chroma_8xH ARGS8(listX[list_offset+1][bw_ref_idx], (unsigned char *)bw_block, 16,
				vec_x_base, vec_y_base, mv_x_bw, mv_y_bw, 8);

			//weighting
			rounding_offset = (1<<IMGPAR chroma_log2_weight_denom);
			down_shift      = (IMGPAR chroma_log2_weight_denom + 1);

			//u
			alpha_fw_u = *(IMGPAR wbp_weight+(((0+wt_list_offset)*MAX_REFERENCE_PICTURES+fw_ref_idx_w)*MAX_REFERENCE_PICTURES+bw_ref_idx_w)*3+0+1);
			alpha_bw_u = *(IMGPAR wbp_weight+(((1+wt_list_offset)*MAX_REFERENCE_PICTURES+fw_ref_idx_w)*MAX_REFERENCE_PICTURES+bw_ref_idx_w)*3+0+1);
			final_offset_u	= (((*(IMGPAR wp_offset+((wt_list_offset + 0)*MAX_REFERENCE_PICTURES+fw_ref_idx_w)*3+0+1))
				+ (*(IMGPAR wp_offset+((wt_list_offset + 1)*MAX_REFERENCE_PICTURES+bw_ref_idx)*3+0+1)) + 1)>>1);

			//v
			alpha_fw_v = *(IMGPAR wbp_weight+(((0+wt_list_offset)*MAX_REFERENCE_PICTURES+fw_ref_idx_w)*MAX_REFERENCE_PICTURES+bw_ref_idx_w)*3+1+1);
			alpha_bw_v = *(IMGPAR wbp_weight+(((1+wt_list_offset)*MAX_REFERENCE_PICTURES+fw_ref_idx_w)*MAX_REFERENCE_PICTURES+bw_ref_idx_w)*3+1+1);
			final_offset_v    = (((*(IMGPAR wp_offset+((wt_list_offset + 0)*MAX_REFERENCE_PICTURES+fw_ref_idx_w)*3+1+1))
				+ (*(IMGPAR wp_offset+((wt_list_offset + 1)*MAX_REFERENCE_PICTURES+bw_ref_idx_w)*3+1+1)) + 1)>>1);

			weight_8_b_uv(&IMGPAR mprUV[0][0], LUMA_BLOCK_SIZE, &IMGPAR mprUV[0][0], &bw_block[0][0], alpha_fw_u, alpha_bw_u, alpha_fw_v, alpha_bw_v, rounding_offset, down_shift, final_offset_u, final_offset_v, LUMA_BLOCK_SIZE, 8);

#ifdef __SUPPORT_YUV400__
		}
#endif
	}
	else
	{
		ref_idx_w = ref_idx = currMB_s_d->pred_info.ref_idx[LIST_0+pred_dir][0];
		if (currMB_s_d->pred_info.ref_idx[LIST_0+pred_dir][0] < 0) {
			ref_idx_w = ref_idx = currMB_s_d->pred_info.ref_idx[LIST_0+pred_dir][0] = 0;
		}
		if (active_pps.weighted_pred_flag && curr_mb_field)
			ref_idx_w >>=1;

		//Luma
		mv_x = currMB_s_d->pred_info.mv[LIST_0+pred_dir][0].x;
		mv_y = currMB_s_d->pred_info.mv[LIST_0+pred_dir][0].y;

		get_block_16xh ARGS7(listX[list_offset + pred_dir][ref_idx], vec_x_base, vec_y_base, 
			mv_x, mv_y, 
			reinterpret_cast<unsigned char(*)[16]>(&IMGPAR mpr[0][0]), 16);

		//weighting
		rounding_offset = IMGPAR wp_round_luma;
		down_shift      = IMGPAR luma_log2_weight_denom;

		weight          = (*(IMGPAR wp_weight+(pred_dir*MAX_REFERENCE_PICTURES+ref_idx_w)*3+0));
		final_offset    = (*(IMGPAR wp_offset+(pred_dir*MAX_REFERENCE_PICTURES+ref_idx_w)*3+0));

		weight_16(&IMGPAR mpr[0][0], weight, rounding_offset, down_shift, final_offset, LUMA_BLOCK_SIZE, 16);

#ifdef __SUPPORT_YUV400__
		if (dec_picture->chroma_format_idc != YUV400)
		{
#endif
			//Chroma
			int chroma_vec_x_base = (vec_x_base>>1);
			int chroma_vec_y_base = (vec_y_base>>1);
			//mv_y += listX[list_offset + pred_dir][ref_idx]->chroma_vector_adjustment;
			mv_y += IMGPAR cr_vector_adjustment[list_offset + pred_dir][ref_idx];

			mb_chroma_8xH ARGS8(listX[list_offset + pred_dir][ref_idx], &IMGPAR mprUV[0][0], 16,
				chroma_vec_x_base, chroma_vec_y_base, mv_x, mv_y, 8);

			//weighting
			rounding_offset = IMGPAR wp_round_chroma;
			down_shift      = IMGPAR chroma_log2_weight_denom;

			//u
			weight_u          = (*(IMGPAR wp_weight+(pred_dir*MAX_REFERENCE_PICTURES+ref_idx_w)*3+0+1));
			final_offset_u    = (*(IMGPAR wp_offset+(pred_dir*MAX_REFERENCE_PICTURES+ref_idx_w)*3+0+1));

			//v
			weight_v          = (*(IMGPAR wp_weight+(pred_dir*MAX_REFERENCE_PICTURES+ref_idx_w)*3+1+1));
			final_offset_v    = (*(IMGPAR wp_offset+(pred_dir*MAX_REFERENCE_PICTURES+ref_idx_w)*3+1+1));


			weight_8_uv(&IMGPAR mprUV[0][0], weight_u, weight_v, rounding_offset, down_shift, final_offset_u, final_offset_v, LUMA_BLOCK_SIZE, 8);
#ifdef __SUPPORT_YUV400__
		}
#endif
	}
}

void MB_InterPred16x8_1 PARGS3(int vec_x_base, int vec_y_base, int list_offset)
{
	int joff, k;
	int pred_dir;
	int ref_idx, fw_ref_idx, bw_ref_idx;
	int mv_x, mv_y, mv_x_bw, mv_y_bw;
	// For weighting
	int ref_idx_w, fw_ref_idx_w, bw_ref_idx_w;
	int curr_mb_field = ((IMGPAR MbaffFrameFlag)&&(currMB_d->mb_field));
	int weight;
	int weight_u, weight_v;
	int rounding_offset;
	unsigned int down_shift;
	int final_offset;
	int final_offset_u, final_offset_v;

	for(k=0;k<2;k++)
	{
		joff = (k<<3);
		pred_dir = currMB_d->b8pdir[k<<1];
		if(pred_dir==2)
		{
			fw_ref_idx_w = fw_ref_idx = currMB_s_d->pred_info.ref_idx[LIST_0][k<<1];
			if (fw_ref_idx_w < 0) {
				fw_ref_idx_w = fw_ref_idx = currMB_s_d->pred_info.ref_idx[LIST_0][k<<1] = 0;
			}
			bw_ref_idx_w = bw_ref_idx = currMB_s_d->pred_info.ref_idx[LIST_1][k<<1];
			if (bw_ref_idx_w < 0) {
				bw_ref_idx_w = bw_ref_idx = currMB_s_d->pred_info.ref_idx[LIST_1][k<<1] = 0;
			}
			int alpha_fw, alpha_bw;
			int alpha_fw_u, alpha_bw_u, alpha_fw_v, alpha_bw_v;	
			int wt_list_offset = (active_pps.weighted_bipred_idc==2)?list_offset:0;
			if (active_pps.weighted_bipred_idc==1 && curr_mb_field)
			{
				fw_ref_idx_w >>=1;
				bw_ref_idx_w >>=1;
			}

			//Luma
			//forward
			mv_x = currMB_s_d->pred_info.mv[LIST_0][joff].x;
			mv_y = currMB_s_d->pred_info.mv[LIST_0][joff].y;

			get_block_16xh ARGS7(listX[list_offset + LIST_0][fw_ref_idx], vec_x_base, vec_y_base+joff, 
				mv_x, mv_y, 
				reinterpret_cast<unsigned char(*)[16]>(&IMGPAR mpr[joff][0]), 8);

			//backward
			mv_x_bw = currMB_s_d->pred_info.mv[LIST_1][joff].x;
			mv_y_bw = currMB_s_d->pred_info.mv[LIST_1][joff].y;

			get_block_16xh ARGS7(listX[list_offset + LIST_1][bw_ref_idx], vec_x_base, vec_y_base+joff, 
				mv_x_bw, mv_y_bw, 
				reinterpret_cast<unsigned char(*)[16]>(&bw_block[joff][0]), 8);

			//weighting
			rounding_offset = (1<<IMGPAR luma_log2_weight_denom);
			down_shift      = (IMGPAR luma_log2_weight_denom+1);

			alpha_fw = *(IMGPAR wbp_weight+(((0+wt_list_offset)*MAX_REFERENCE_PICTURES+fw_ref_idx_w)*MAX_REFERENCE_PICTURES+bw_ref_idx_w)*3+0);
			alpha_bw = *(IMGPAR wbp_weight+(((1+wt_list_offset)*MAX_REFERENCE_PICTURES+fw_ref_idx_w)*MAX_REFERENCE_PICTURES+bw_ref_idx_w)*3+0);
			final_offset    = (((*(IMGPAR wp_offset+((wt_list_offset+0)*MAX_REFERENCE_PICTURES+fw_ref_idx_w)*3+0)) + 
				(*(IMGPAR wp_offset+((wt_list_offset+1)*MAX_REFERENCE_PICTURES+bw_ref_idx_w)*3+0)) + 1) >>1);

			weight_16_b(&IMGPAR mpr[joff][0], LUMA_BLOCK_SIZE, &IMGPAR mpr[joff][0], &bw_block[joff][0], alpha_fw, alpha_bw, rounding_offset, down_shift, final_offset, LUMA_BLOCK_SIZE, 8);

#ifdef __SUPPORT_YUV400__
			if (dec_picture->chroma_format_idc != YUV400)
			{
#endif
				//Chroma
				int chroma_vec_x_base = (vec_x_base>>1);
				joff >>= 1;
				int chroma_vec_y_base = (vec_y_base>>1)+joff;
				//mv_y += listX[list_offset][fw_ref_idx]->chroma_vector_adjustment;
				mv_y += IMGPAR cr_vector_adjustment[list_offset][fw_ref_idx];

				mb_chroma_8xH ARGS8(listX[list_offset][fw_ref_idx], &IMGPAR mprUV[joff][0], 16,
					chroma_vec_x_base, chroma_vec_y_base, mv_x, mv_y, 4);			

				//mv_y_bw += listX[list_offset + 1][bw_ref_idx]->chroma_vector_adjustment;
				mv_y_bw += IMGPAR cr_vector_adjustment[list_offset + 1][bw_ref_idx];

				mb_chroma_8xH ARGS8(listX[list_offset + 1][bw_ref_idx], (unsigned char *)bw_block, 16,
					chroma_vec_x_base, chroma_vec_y_base, mv_x_bw, mv_y_bw, 4);

				//weighting
				rounding_offset = (1<<IMGPAR chroma_log2_weight_denom);
				down_shift      = (IMGPAR chroma_log2_weight_denom + 1);

				//u
				alpha_fw_u = *(IMGPAR wbp_weight+(((0+wt_list_offset)*MAX_REFERENCE_PICTURES+fw_ref_idx_w)*MAX_REFERENCE_PICTURES+bw_ref_idx_w)*3+0+1);
				alpha_bw_u = *(IMGPAR wbp_weight+(((1+wt_list_offset)*MAX_REFERENCE_PICTURES+fw_ref_idx_w)*MAX_REFERENCE_PICTURES+bw_ref_idx_w)*3+0+1);
				final_offset_u    = (((*(IMGPAR wp_offset+((wt_list_offset + 0)*MAX_REFERENCE_PICTURES+fw_ref_idx_w)*3+0+1))
					+ (*(IMGPAR wp_offset+((wt_list_offset + 1)*MAX_REFERENCE_PICTURES+bw_ref_idx)*3+0+1)) + 1)>>1);

				//v
				alpha_fw_v = *(IMGPAR wbp_weight+(((0+wt_list_offset)*MAX_REFERENCE_PICTURES+fw_ref_idx_w)*MAX_REFERENCE_PICTURES+bw_ref_idx_w)*3+1+1);
				alpha_bw_v = *(IMGPAR wbp_weight+(((1+wt_list_offset)*MAX_REFERENCE_PICTURES+fw_ref_idx_w)*MAX_REFERENCE_PICTURES+bw_ref_idx_w)*3+1+1);
				final_offset_v    = (((*(IMGPAR wp_offset+((wt_list_offset + 0)*MAX_REFERENCE_PICTURES+fw_ref_idx_w)*3+1+1))
					+ (*(IMGPAR wp_offset+((wt_list_offset + 1)*MAX_REFERENCE_PICTURES+bw_ref_idx_w)*3+1+1)) + 1)>>1);

				weight_8_b_uv(&IMGPAR mprUV[joff][0], LUMA_BLOCK_SIZE, &IMGPAR mprUV[joff][0], &bw_block[0][0], alpha_fw_u, alpha_bw_u, alpha_fw_v, alpha_bw_v, rounding_offset, down_shift, final_offset_u, final_offset_v, LUMA_BLOCK_SIZE, 4);				
#ifdef __SUPPORT_YUV400__
			}
#endif
		}
		else
		{
			ref_idx_w = ref_idx = currMB_s_d->pred_info.ref_idx[LIST_0+pred_dir][k<<1];
			if (ref_idx_w < 0) {
				ref_idx_w = ref_idx = currMB_s_d->pred_info.ref_idx[LIST_0+pred_dir][k<<1] = 0;
			}
			if (active_pps.weighted_pred_flag && curr_mb_field)
				ref_idx_w >>=1;

			//Luma
			mv_x = currMB_s_d->pred_info.mv[LIST_0+pred_dir][joff].x;
			mv_y = currMB_s_d->pred_info.mv[LIST_0+pred_dir][joff].y;

			get_block_16xh ARGS7(listX[list_offset + pred_dir][ref_idx], vec_x_base, vec_y_base+joff, 
				mv_x, mv_y, 
				reinterpret_cast<unsigned char(*)[16]>(&IMGPAR mpr[joff][0]), 8);

			//weighting
			rounding_offset = IMGPAR wp_round_luma;
			down_shift      = IMGPAR luma_log2_weight_denom;

			weight          = (*(IMGPAR wp_weight+(pred_dir*MAX_REFERENCE_PICTURES+ref_idx_w)*3+0));
			final_offset    = (*(IMGPAR wp_offset+(pred_dir*MAX_REFERENCE_PICTURES+ref_idx_w)*3+0));

			weight_16(&IMGPAR mpr[joff][0], weight, rounding_offset, down_shift, final_offset, LUMA_BLOCK_SIZE, 8);

#ifdef __SUPPORT_YUV400__
			if (dec_picture->chroma_format_idc != YUV400)
			{
#endif
				//Chroma
				int chroma_vec_x_base = (vec_x_base>>1);
				joff >>= 1;
				int chroma_vec_y_base = (vec_y_base>>1)+joff;
				//mv_y += listX[list_offset + pred_dir][ref_idx]->chroma_vector_adjustment;
				mv_y += IMGPAR cr_vector_adjustment[list_offset + pred_dir][ref_idx];

				mb_chroma_8xH ARGS8(listX[list_offset + pred_dir][ref_idx], &IMGPAR mprUV[joff][0], 16,
					chroma_vec_x_base, chroma_vec_y_base, mv_x, mv_y, 4);

				//weighting
				rounding_offset = IMGPAR wp_round_chroma;
				down_shift      = IMGPAR chroma_log2_weight_denom;

				//u
				weight_u          = (*(IMGPAR wp_weight+(pred_dir*MAX_REFERENCE_PICTURES+ref_idx_w)*3+0+1));
				final_offset_u    = (*(IMGPAR wp_offset+(pred_dir*MAX_REFERENCE_PICTURES+ref_idx_w)*3+0+1));

				//v
				weight_v          = (*(IMGPAR wp_weight+(pred_dir*MAX_REFERENCE_PICTURES+ref_idx_w)*3+1+1));
				final_offset_v    = (*(IMGPAR wp_offset+(pred_dir*MAX_REFERENCE_PICTURES+ref_idx_w)*3+1+1));

				weight_8_uv(&IMGPAR mprUV[joff][0], weight_u, weight_v, rounding_offset, down_shift, final_offset_u, final_offset_v, LUMA_BLOCK_SIZE, 4);
#ifdef __SUPPORT_YUV400__
			}
#endif
		}
	}
}

void MB_InterPred8x16_1 PARGS3(int vec_x_base, int vec_y_base, int list_offset)
{
	int i4, ioff, k;
	int pred_dir;
	int ref_idx, fw_ref_idx, bw_ref_idx;
	int mv_x, mv_y, mv_x_bw, mv_y_bw;
	// For weighting
	int ref_idx_w, fw_ref_idx_w, bw_ref_idx_w;
	int curr_mb_field = ((IMGPAR MbaffFrameFlag)&&(currMB_d->mb_field));
	int weight;
	int weight_u, weight_v;
	int rounding_offset;
	unsigned int down_shift;
	int final_offset;
	int final_offset_u, final_offset_v;

	for(k=0;k<2;k++)
	{
		i4 = (k<<1);
		ioff = (i4<<2);
		pred_dir = currMB_d->b8pdir[k];
		if(pred_dir==2)
		{
			fw_ref_idx_w = fw_ref_idx = currMB_s_d->pred_info.ref_idx[LIST_0][k];
			if (fw_ref_idx_w < 0) {
				fw_ref_idx_w = fw_ref_idx = currMB_s_d->pred_info.ref_idx[LIST_0][k] = 0;
			}
			bw_ref_idx_w = bw_ref_idx = currMB_s_d->pred_info.ref_idx[LIST_1][k];
			if (bw_ref_idx_w < 0) {
				bw_ref_idx_w = bw_ref_idx = currMB_s_d->pred_info.ref_idx[LIST_1][k] = 0;
			}
			int alpha_fw, alpha_bw;
			int alpha_fw_u, alpha_bw_u, alpha_fw_v, alpha_bw_v;
			int wt_list_offset = (active_pps.weighted_bipred_idc==2)?list_offset:0;
			if (active_pps.weighted_bipred_idc==1 && curr_mb_field)
			{
				fw_ref_idx_w >>=1;
				bw_ref_idx_w >>=1;
			}

			//Luma
			//forward			
			mv_x = currMB_s_d->pred_info.mv[LIST_0][i4].x;
			mv_y = currMB_s_d->pred_info.mv[LIST_0][i4].y;
			get_block_8xh ARGS7(listX[list_offset + LIST_0][fw_ref_idx], vec_x_base+ioff, vec_y_base, 
				mv_x, mv_y, 
				reinterpret_cast<unsigned char(*)[16]>(&IMGPAR mpr[0][ioff]), 16);

			//backward
			mv_x_bw = currMB_s_d->pred_info.mv[LIST_1][i4].x;
			mv_y_bw = currMB_s_d->pred_info.mv[LIST_1][i4].y;

			get_block_8xh ARGS7(listX[list_offset + LIST_1][bw_ref_idx], vec_x_base+ioff, vec_y_base, 
				mv_x_bw, mv_y_bw, 
				reinterpret_cast<unsigned char(*)[16]>(&bw_block[0][ioff]), 16);

			//weighting
			rounding_offset = (1<<IMGPAR luma_log2_weight_denom);
			down_shift      = (IMGPAR luma_log2_weight_denom+1);

			alpha_fw = *(IMGPAR wbp_weight+(((0+wt_list_offset)*MAX_REFERENCE_PICTURES+fw_ref_idx_w)*MAX_REFERENCE_PICTURES+bw_ref_idx_w)*3+0);
			alpha_bw = *(IMGPAR wbp_weight+(((1+wt_list_offset)*MAX_REFERENCE_PICTURES+fw_ref_idx_w)*MAX_REFERENCE_PICTURES+bw_ref_idx_w)*3+0);
			final_offset	= (((*(IMGPAR wp_offset+((wt_list_offset+0)*MAX_REFERENCE_PICTURES+fw_ref_idx_w)*3+0)) + 
				(*(IMGPAR wp_offset+((wt_list_offset+1)*MAX_REFERENCE_PICTURES+bw_ref_idx_w)*3+0)) + 1) >>1);

			weight_8_b(&IMGPAR mpr[0][ioff], LUMA_BLOCK_SIZE, &IMGPAR mpr[0][ioff], &bw_block[0][ioff], alpha_fw, alpha_bw, rounding_offset, down_shift, final_offset, LUMA_BLOCK_SIZE, 16);

#ifdef __SUPPORT_YUV400__
			if (dec_picture->chroma_format_idc != YUV400)
			{
#endif
				//Chroma
				ioff >>= 1;
				int chroma_vec_x_base = (vec_x_base>>1)+ioff;
				int chroma_vec_y_base = (vec_y_base>>1);
				//mv_y += listX[list_offset][fw_ref_idx]->chroma_vector_adjustment;
				mv_y += IMGPAR cr_vector_adjustment[list_offset][fw_ref_idx];

				mb_chroma_4xH ARGS8(listX[list_offset][fw_ref_idx], &IMGPAR mprUV[0][ioff<<1], 16,
					chroma_vec_x_base, chroma_vec_y_base, mv_x, mv_y, 8);			

				//mv_y_bw += listX[list_offset + 1][bw_ref_idx]->chroma_vector_adjustment;
				mv_y_bw += IMGPAR cr_vector_adjustment[list_offset + 1][bw_ref_idx];

				mb_chroma_4xH ARGS8(listX[list_offset + 1][bw_ref_idx], (unsigned char *)bw_block, 16,
					chroma_vec_x_base, chroma_vec_y_base, mv_x_bw, mv_y_bw, 8);

				//weighting
				rounding_offset = (1<<IMGPAR chroma_log2_weight_denom);
				down_shift      = (IMGPAR chroma_log2_weight_denom + 1);

				//u
				alpha_fw_u = *(IMGPAR wbp_weight+(((0+wt_list_offset)*MAX_REFERENCE_PICTURES+fw_ref_idx_w)*MAX_REFERENCE_PICTURES+bw_ref_idx_w)*3+0+1);
				alpha_bw_u = *(IMGPAR wbp_weight+(((1+wt_list_offset)*MAX_REFERENCE_PICTURES+fw_ref_idx_w)*MAX_REFERENCE_PICTURES+bw_ref_idx_w)*3+0+1);
				final_offset_u	= (((*(IMGPAR wp_offset+((wt_list_offset + 0)*MAX_REFERENCE_PICTURES+fw_ref_idx_w)*3+0+1))
					+ (*(IMGPAR wp_offset+((wt_list_offset + 1)*MAX_REFERENCE_PICTURES+bw_ref_idx)*3+0+1)) + 1)>>1);

				//v
				alpha_fw_v = *(IMGPAR wbp_weight+(((0+wt_list_offset)*MAX_REFERENCE_PICTURES+fw_ref_idx_w)*MAX_REFERENCE_PICTURES+bw_ref_idx_w)*3+1+1);
				alpha_bw_v = *(IMGPAR wbp_weight+(((1+wt_list_offset)*MAX_REFERENCE_PICTURES+fw_ref_idx_w)*MAX_REFERENCE_PICTURES+bw_ref_idx_w)*3+1+1);
				final_offset_v	= (((*(IMGPAR wp_offset+((wt_list_offset + 0)*MAX_REFERENCE_PICTURES+fw_ref_idx_w)*3+1+1))
					+ (*(IMGPAR wp_offset+((wt_list_offset + 1)*MAX_REFERENCE_PICTURES+bw_ref_idx_w)*3+1+1)) + 1)>>1);

				weight_4_b_uv(&IMGPAR mprUV[0][ioff<<1], LUMA_BLOCK_SIZE, &IMGPAR mprUV[0][ioff<<1], &bw_block[0][0], alpha_fw_u, alpha_bw_u, alpha_fw_v, alpha_bw_v, rounding_offset, down_shift, final_offset_u, final_offset_v, LUMA_BLOCK_SIZE, 8);
#ifdef __SUPPORT_YUV400__
			}
#endif
		}
		else
		{
			ref_idx_w = ref_idx = currMB_s_d->pred_info.ref_idx[LIST_0+pred_dir][k];
			if (ref_idx_w < 0) {
				ref_idx_w = ref_idx = currMB_s_d->pred_info.ref_idx[LIST_0+pred_dir][k] = 0;
			}
			if (active_pps.weighted_pred_flag && curr_mb_field)
				ref_idx_w >>=1;

			//Luma
			mv_x = currMB_s_d->pred_info.mv[LIST_0+pred_dir][i4].x;
			mv_y = currMB_s_d->pred_info.mv[LIST_0+pred_dir][i4].y;

			get_block_8xh ARGS7(listX[list_offset + pred_dir][ref_idx], vec_x_base+ioff, vec_y_base, 
				mv_x, mv_y, 
				reinterpret_cast<unsigned char(*)[16]>(&IMGPAR mpr[0][ioff]), 16);

			//weighting
			rounding_offset = IMGPAR wp_round_luma;
			down_shift      = IMGPAR luma_log2_weight_denom;

			weight          = (*(IMGPAR wp_weight+(pred_dir*MAX_REFERENCE_PICTURES+ref_idx_w)*3+0));
			final_offset    = (*(IMGPAR wp_offset+(pred_dir*MAX_REFERENCE_PICTURES+ref_idx_w)*3+0));

			weight_8(&IMGPAR mpr[0][ioff], weight, rounding_offset, down_shift, final_offset, LUMA_BLOCK_SIZE, 16);

#ifdef __SUPPORT_YUV400__
			if (dec_picture->chroma_format_idc != YUV400)
			{
#endif
				//Chroma
				ioff >>= 1;
				int chroma_vec_x_base = (vec_x_base>>1)+ioff;
				int chroma_vec_y_base = (vec_y_base>>1);
				//mv_y += listX[list_offset + pred_dir][ref_idx]->chroma_vector_adjustment;
				mv_y += IMGPAR cr_vector_adjustment[list_offset + pred_dir][ref_idx];

				mb_chroma_4xH ARGS8(listX[list_offset + pred_dir][ref_idx], &IMGPAR mprUV[0][ioff<<1], 16,
					chroma_vec_x_base, chroma_vec_y_base, mv_x, mv_y, 8);

				//weighting
				rounding_offset = IMGPAR wp_round_chroma;
				down_shift      = IMGPAR chroma_log2_weight_denom;

				//u
				weight_u          = (*(IMGPAR wp_weight+(pred_dir*MAX_REFERENCE_PICTURES+ref_idx_w)*3+0+1));
				final_offset_u    = (*(IMGPAR wp_offset+(pred_dir*MAX_REFERENCE_PICTURES+ref_idx_w)*3+0+1));

				//v
				weight_v          = (*(IMGPAR wp_weight+(pred_dir*MAX_REFERENCE_PICTURES+ref_idx_w)*3+1+1));
				final_offset_v    = (*(IMGPAR wp_offset+(pred_dir*MAX_REFERENCE_PICTURES+ref_idx_w)*3+1+1));

				weight_4_uv(&IMGPAR mprUV[0][ioff<<1], weight_u, weight_v, rounding_offset, down_shift, final_offset_u, final_offset_v, LUMA_BLOCK_SIZE, 8);
#ifdef __SUPPORT_YUV400__
			}
#endif
		}
	}
}

void MB_InterPred8x8_BiDir PARGS4(int vec_x_base,
																	int vec_y_base,
																	int list_offset,
																	int b8)
{
	int mv_x, mv_y, mv_x_bw, mv_y_bw;
	int fw_refframe, bw_refframe;

	int i_1    = ((b8&1)<<1);
	int j_1    = (b8&2);
	int ioff = i_1<<2;
	int joff = j_1<<2;

	fw_refframe = currMB_s_d->pred_info.ref_idx[LIST_0][b8];
	if (fw_refframe < 0) {
		fw_refframe = currMB_s_d->pred_info.ref_idx[LIST_0][b8] = 0;
	}
	bw_refframe = currMB_s_d->pred_info.ref_idx[LIST_1][b8];
	if (bw_refframe < 0) {
		bw_refframe = currMB_s_d->pred_info.ref_idx[LIST_1][b8] = 0;
	}

	mv_x = currMB_s_d->pred_info.mv[LIST_0][joff+i_1].x;
	mv_y = currMB_s_d->pred_info.mv[LIST_0][joff+i_1].y;

	get_block_8xh ARGS7(listX[list_offset][fw_refframe], vec_x_base + ioff, vec_y_base + joff, mv_x, mv_y, fw_block, 8);

	mv_x_bw = currMB_s_d->pred_info.mv[LIST_1][joff+i_1].x;
	mv_y_bw = currMB_s_d->pred_info.mv[LIST_1][joff+i_1].y;

	get_block_8xh ARGS7(listX[1+list_offset][bw_refframe], vec_x_base + ioff, vec_y_base + joff, mv_x_bw, mv_y_bw, bw_block, 8);

	average_8(&IMGPAR mpr[joff][ioff], 16, (imgpel *) fw_block, (imgpel *) bw_block, LUMA_BLOCK_SIZE, 8);

#ifdef __SUPPORT_YUV400__
	if (dec_picture->chroma_format_idc != YUV400)
	{
#endif
		//chroma
		ioff >>= 1;
		joff >>= 1;
		vec_x_base >>= 1;
		vec_y_base >>= 1;
		int chroma_vec_x_base = vec_x_base+ioff;
		int chroma_vec_y_base = vec_y_base+joff;
		//mv_y += listX[list_offset][fw_refframe]->chroma_vector_adjustment;
		mv_y += IMGPAR cr_vector_adjustment[list_offset][fw_refframe];

		mb_chroma_4xH ARGS8(listX[list_offset][fw_refframe], &fw_block[0][0], LUMA_BLOCK_SIZE,
			chroma_vec_x_base, chroma_vec_y_base, mv_x, mv_y, 4);

		//mv_y_bw += listX[1+list_offset][bw_refframe]->chroma_vector_adjustment;
		mv_y_bw += IMGPAR cr_vector_adjustment[1+list_offset][bw_refframe];

		mb_chroma_4xH ARGS8(listX[1+list_offset][bw_refframe], &bw_block[0][0], LUMA_BLOCK_SIZE, 
			chroma_vec_x_base, chroma_vec_y_base, mv_x_bw, mv_y_bw, 4);

		average_8(&IMGPAR mprUV[joff][ioff<<1], 16, (imgpel *) fw_block+0, (imgpel *) bw_block+0, LUMA_BLOCK_SIZE, BLOCK_SIZE);
#ifdef __SUPPORT_YUV400__
	}
#endif

}

void MB_InterPred8x8_1Dir PARGS5(int vec_x_base,
																 int vec_y_base,
																 int list_offset,
																 int b8,
																 int dir)
{
	int mv_x, mv_y;
	int refframe;

	int i_1    = ((b8&1)<<1);
	int j_1    = (b8&2);
	int ioff = i_1<<2;
	int joff = j_1<<2;

	refframe = currMB_s_d->pred_info.ref_idx[dir][b8];
	if (refframe < 0) {
		refframe = currMB_s_d->pred_info.ref_idx[dir][b8] = 0;
	}
	mv_x = currMB_s_d->pred_info.mv[dir][joff+i_1].x;
	mv_y = currMB_s_d->pred_info.mv[dir][joff+i_1].y;

	get_block_8xh ARGS7(listX[list_offset+dir][refframe], vec_x_base + ioff, vec_y_base + joff, mv_x, mv_y, 
		reinterpret_cast<unsigned char(*)[16]>(&IMGPAR mpr[joff][ioff]), 8);

#ifdef __SUPPORT_YUV400__
	if (dec_picture->chroma_format_idc != YUV400)
	{
#endif
		//chroma
		ioff >>= 1;
		joff >>= 1;
		vec_x_base >>= 1;
		vec_y_base >>= 1;
		int chroma_vec_x_base = vec_x_base+ioff;
		int chroma_vec_y_base = vec_y_base+joff;
		//mv_y += listX[list_offset+dir][refframe]->chroma_vector_adjustment;
		mv_y += IMGPAR cr_vector_adjustment[list_offset+dir][refframe];

		mb_chroma_4xH ARGS8(listX[list_offset+dir][refframe], &IMGPAR mprUV[joff][ioff<<1], LUMA_BLOCK_SIZE,
			chroma_vec_x_base, chroma_vec_y_base, mv_x, mv_y, 4);

#ifdef __SUPPORT_YUV400__
	}
#endif
}

void MB_InterPred4x4_BiDir PARGS4(int vec_x_base,
																	int vec_y_base,
																	int list_offset,
																	int b8)
{
	int mv_x, mv_y, mv_x_bw, mv_y_bw;
	int b4;
	int i_1    = ((b8&1)<<1);
	int j_1    = (b8&2);
	int ioff;
	int joff;
	int i;
	int j;
	int fw_refframe = currMB_s_d->pred_info.ref_idx[LIST_0][b8];
	int bw_refframe = currMB_s_d->pred_info.ref_idx[LIST_1][b8];

	//int chroma_vector_adjustment = listX[list_offset][fw_refframe]->chroma_vector_adjustment;
	//int chroma_vector_adjustment_bw = listX[1+list_offset][bw_refframe]->chroma_vector_adjustment;
	int chroma_vector_adjustment = IMGPAR cr_vector_adjustment[list_offset][fw_refframe];
	int chroma_vector_adjustment_bw = IMGPAR cr_vector_adjustment[1+list_offset][bw_refframe];

	int mode                                      = currMB_d->b8mode[b8];
	int nofiter                                   = get_block_4xh_iter[mode];
	int height                                    = get_block_4xh_height[mode];
	int *get_block_4xh_b4s_ptr                    = get_block_4xh_b4s[mode];
	get_block_wxh* get_block_4xh_ptr              = get_block_4xh_p[mode];
	average_t *average_4_ptr                      = average_p[mode][0];
	mb_chroma_pred_t *mb_chroma_2xH_ptr           = mb_chroma_2xH_p[mode];
	//mb_chroma_pred_mv00_t *mb_chroma_2xH_mv00_ptr = mb_chroma_2xH_mv00_p[mode];
	average_t *average_2_ptr                      = average_p[mode][1];
	int iter;

	if (fw_refframe < 0) {
		fw_refframe = currMB_s_d->pred_info.ref_idx[LIST_0][b8] = 0;
	}
	if (bw_refframe < 0) {
		bw_refframe = currMB_s_d->pred_info.ref_idx[LIST_1][b8] = 0;
	}

	for (iter = 0; iter <nofiter; iter ++)
	{
		b4   = get_block_4xh_b4s_ptr[iter];
		i    = i_1 + (b4&1);
		j    = j_1 + (b4>>1);
		ioff = i<<2;
		joff = j<<2;

		mv_x = currMB_s_d->pred_info.mv[LIST_0][joff+i].x;
		mv_y = currMB_s_d->pred_info.mv[LIST_0][joff+i].y;

		get_block_4xh_ptr ARGS7(listX[list_offset][fw_refframe], vec_x_base + ioff, vec_y_base + joff, mv_x, mv_y, fw_block, height);

		mv_x_bw = currMB_s_d->pred_info.mv[LIST_1][joff+i].x;
		mv_y_bw = currMB_s_d->pred_info.mv[LIST_1][joff+i].y;

		get_block_4xh_ptr ARGS7(listX[1+list_offset][bw_refframe], vec_x_base + ioff, vec_y_base + joff, mv_x_bw, mv_y_bw, bw_block, height);

		average_4_ptr(&IMGPAR mpr[joff][ioff], 16, (imgpel *) fw_block, (imgpel *) bw_block, LUMA_BLOCK_SIZE, height);

#ifdef __SUPPORT_YUV400__
		if (dec_picture->chroma_format_idc != YUV400)
		{
#endif
			//chroma
			ioff >>= 1;
			joff >>= 1;		
			int chroma_vec_x_base = (vec_x_base>>1)+ioff;
			int chroma_vec_y_base = (vec_y_base>>1)+joff;

			mv_y += chroma_vector_adjustment;

			mb_chroma_2xH_ptr ARGS8(listX[list_offset][fw_refframe], &fw_block[0][0], LUMA_BLOCK_SIZE,
				chroma_vec_x_base, chroma_vec_y_base, mv_x, mv_y, height>>1);

			mv_y_bw += chroma_vector_adjustment_bw;

			mb_chroma_2xH_ptr ARGS8(listX[1+list_offset][bw_refframe], &bw_block[0][0], LUMA_BLOCK_SIZE,
				chroma_vec_x_base, chroma_vec_y_base, mv_x_bw, mv_y_bw, height>>1);

			average_4_ptr(&IMGPAR mprUV[joff][ioff<<1], 16, (imgpel *) fw_block+0, (imgpel *) bw_block+0, LUMA_BLOCK_SIZE, height>>1);
#ifdef __SUPPORT_YUV400__
		}
#endif
	}

}


void MB_InterPred4x4_1Dir PARGS5(int vec_x_base,
																 int vec_y_base,
																 int list_offset,
																 int b8,
																 int dir)
{
	int mv_x, mv_y;
	int b4;
	int i_1    = ((b8&1)<<1);
	int j_1    = (b8&2);
	int ioff;
	int joff;
	int i;
	int j;
	int refframe = currMB_s_d->pred_info.ref_idx[dir][b8];
	//int chroma_vector_adjustment = listX[list_offset+dir][refframe]->chroma_vector_adjustment;
	int chroma_vector_adjustment = IMGPAR cr_vector_adjustment[list_offset+dir][refframe];

	int mode                                      = currMB_d->b8mode[b8];
	int nofiter                                   = get_block_4xh_iter[mode];
	int height                                    = get_block_4xh_height[mode];
	int *get_block_4xh_b4s_ptr                    = get_block_4xh_b4s[mode];
	get_block_wxh* get_block_4xh_ptr              = get_block_4xh_p[mode];
	mb_chroma_pred_t *mb_chroma_2xH_ptr           = mb_chroma_2xH_p[mode];
	//mb_chroma_pred_mv00_t *mb_chroma_2xH_mv00_ptr = mb_chroma_2xH_mv00_p[mode];
	int iter;

	if (refframe < 0) {
		refframe = currMB_s_d->pred_info.ref_idx[dir][b8] = 0;
	}
	for (iter = 0; iter <nofiter; iter ++)
	{
		b4   = get_block_4xh_b4s_ptr[iter];
		i    = i_1 + (b4&1);
		j    = j_1 + (b4>>1);
		ioff = i<<2;
		joff = j<<2;

		mv_x = currMB_s_d->pred_info.mv[dir][joff+i].x;
		mv_y = currMB_s_d->pred_info.mv[dir][joff+i].y;

		get_block_4xh_ptr ARGS7(listX[list_offset+dir][refframe], vec_x_base + ioff, vec_y_base + joff, mv_x, mv_y, 
			reinterpret_cast<unsigned char(*)[16]>(&IMGPAR mpr[joff][ioff]), height);

#ifdef __SUPPORT_YUV400__
		if (dec_picture->chroma_format_idc != YUV400)
		{
#endif
			//chroma
			ioff >>= 1;
			joff >>= 1;		
			int chroma_vec_x_base = (vec_x_base>>1)+ioff;
			int chroma_vec_y_base = (vec_y_base>>1)+joff;

			mv_y += chroma_vector_adjustment;

			mb_chroma_2xH_ptr ARGS8(listX[list_offset+dir][refframe], &IMGPAR mprUV[joff][ioff<<1], LUMA_BLOCK_SIZE,
				chroma_vec_x_base, chroma_vec_y_base, mv_x, mv_y, height>>1);

#ifdef __SUPPORT_YUV400__
		}
#endif
	}
}

//For weighting
void MB_InterPred8x8_BiDir_1 PARGS4(int vec_x_base, int vec_y_base, int list_offset, int b8)
{
	int mv_x, mv_y, mv_x_bw, mv_y_bw;
	int fw_ref_idx, bw_ref_idx;
	// For weighting
	int fw_ref_idx_w, bw_ref_idx_w;
	int curr_mb_field = ((IMGPAR MbaffFrameFlag)&&(currMB_d->mb_field));
	int rounding_offset;
	unsigned int down_shift;
	int final_offset;
	int final_offset_u, final_offset_v;

	int i_1    = ((b8&1)<<1);
	int j_1    = (b8&2);
	int ioff = i_1<<2;
	int joff = j_1<<2;

	fw_ref_idx_w = fw_ref_idx = currMB_s_d->pred_info.ref_idx[LIST_0][b8];
	if (fw_ref_idx_w < 0) {
		fw_ref_idx_w = fw_ref_idx = currMB_s_d->pred_info.ref_idx[LIST_0][b8] = 0;
	}
	bw_ref_idx_w = bw_ref_idx = currMB_s_d->pred_info.ref_idx[LIST_1][b8];
	if (bw_ref_idx_w < 0) {
		bw_ref_idx_w = bw_ref_idx = currMB_s_d->pred_info.ref_idx[LIST_1][b8] = 0;
	}
	int alpha_fw, alpha_bw;
	int alpha_fw_u, alpha_bw_u, alpha_fw_v, alpha_bw_v;
	int wt_list_offset = (active_pps.weighted_bipred_idc==2)?list_offset:0;
	if (active_pps.weighted_bipred_idc==1 && curr_mb_field)
	{
		fw_ref_idx_w >>=1;
		bw_ref_idx_w >>=1;
	}

	//Luma
	mv_x = currMB_s_d->pred_info.mv[LIST_0][joff+i_1].x;
	mv_y = currMB_s_d->pred_info.mv[LIST_0][joff+i_1].y;

	get_block_8xh ARGS7(listX[list_offset][fw_ref_idx], vec_x_base + ioff, vec_y_base + joff, mv_x, mv_y, fw_block, 8);

	mv_x_bw = currMB_s_d->pred_info.mv[LIST_1][joff+i_1].x;
	mv_y_bw = currMB_s_d->pred_info.mv[LIST_1][joff+i_1].y;

	get_block_8xh ARGS7(listX[1+list_offset][bw_ref_idx], vec_x_base + ioff, vec_y_base + joff, mv_x_bw, mv_y_bw, bw_block, 8);

	//weighting
	rounding_offset = (1<<IMGPAR luma_log2_weight_denom);
	down_shift      = (IMGPAR luma_log2_weight_denom+1);

	alpha_fw = *(IMGPAR wbp_weight+(((0+wt_list_offset)*MAX_REFERENCE_PICTURES+fw_ref_idx_w)*MAX_REFERENCE_PICTURES+bw_ref_idx_w)*3+0);
	alpha_bw = *(IMGPAR wbp_weight+(((1+wt_list_offset)*MAX_REFERENCE_PICTURES+fw_ref_idx_w)*MAX_REFERENCE_PICTURES+bw_ref_idx_w)*3+0);
	final_offset  = (((*(IMGPAR wp_offset+((wt_list_offset+0)*MAX_REFERENCE_PICTURES+fw_ref_idx_w)*3+0)) + 
		(*(IMGPAR wp_offset+((wt_list_offset+1)*MAX_REFERENCE_PICTURES+bw_ref_idx_w)*3+0)) + 1) >>1);

	weight_8_b(&IMGPAR mpr[joff][ioff], LUMA_BLOCK_SIZE, &fw_block[0][0], &bw_block[0][0], alpha_fw, alpha_bw, rounding_offset, down_shift, final_offset, LUMA_BLOCK_SIZE, 8);

	//	average_8(&IMGPAR mpr[joff][ioff], 16, (imgpel *) fw_block, (imgpel *) bw_block, LUMA_BLOCK_SIZE, 8);

#ifdef __SUPPORT_YUV400__
	if (dec_picture->chroma_format_idc != YUV400)
	{
#endif
		//Chroma
		ioff>>=1;
		joff>>=1;
		vec_x_base >>= 1;
		vec_y_base >>= 1;
		int chroma_vec_x_base = vec_x_base+ioff;
		int chroma_vec_y_base = vec_y_base+joff;
		//mv_y += listX[list_offset][fw_ref_idx]->chroma_vector_adjustment;
		mv_y += IMGPAR cr_vector_adjustment[list_offset][fw_ref_idx];

		mb_chroma_4xH ARGS8(listX[list_offset][fw_ref_idx], &fw_block[0][0], LUMA_BLOCK_SIZE,
			chroma_vec_x_base, chroma_vec_y_base, mv_x, mv_y, 4);

		//mv_y_bw += listX[1+list_offset][bw_ref_idx]->chroma_vector_adjustment;
		mv_y_bw += IMGPAR cr_vector_adjustment[1+list_offset][bw_ref_idx];

		mb_chroma_4xH ARGS8(listX[1+list_offset][bw_ref_idx], &bw_block[0][0], LUMA_BLOCK_SIZE, 
			chroma_vec_x_base, chroma_vec_y_base, mv_x_bw, mv_y_bw, 4);

		//weighting
		rounding_offset = (1<<IMGPAR chroma_log2_weight_denom);
		down_shift      = (IMGPAR chroma_log2_weight_denom + 1);

		//u
		alpha_fw_u = *(IMGPAR wbp_weight+(((0+wt_list_offset)*MAX_REFERENCE_PICTURES+fw_ref_idx_w)*MAX_REFERENCE_PICTURES+bw_ref_idx_w)*3+0+1);
		alpha_bw_u = *(IMGPAR wbp_weight+(((1+wt_list_offset)*MAX_REFERENCE_PICTURES+fw_ref_idx_w)*MAX_REFERENCE_PICTURES+bw_ref_idx_w)*3+0+1);
		final_offset_u  = (((*(IMGPAR wp_offset+((wt_list_offset + 0)*MAX_REFERENCE_PICTURES+fw_ref_idx_w)*3+0+1))
			+ (*(IMGPAR wp_offset+((wt_list_offset + 1)*MAX_REFERENCE_PICTURES+bw_ref_idx_w)*3+0+1)) + 1)>>1);

		//v
		alpha_fw_v = *(IMGPAR wbp_weight+(((0+wt_list_offset)*MAX_REFERENCE_PICTURES+fw_ref_idx_w)*MAX_REFERENCE_PICTURES+bw_ref_idx_w)*3+1+1);
		alpha_bw_v = *(IMGPAR wbp_weight+(((1+wt_list_offset)*MAX_REFERENCE_PICTURES+fw_ref_idx_w)*MAX_REFERENCE_PICTURES+bw_ref_idx_w)*3+1+1);
		final_offset_v  = (((*(IMGPAR wp_offset+((wt_list_offset + 0)*MAX_REFERENCE_PICTURES+fw_ref_idx_w)*3+1+1))
			+ (*(IMGPAR wp_offset+((wt_list_offset + 1)*MAX_REFERENCE_PICTURES+bw_ref_idx_w)*3+1+1)) + 1)>>1);

		weight_4_b_uv(&IMGPAR mprUV[joff][ioff<<1], LUMA_BLOCK_SIZE, &fw_block[0][0], &bw_block[0][0], 
			alpha_fw_u, alpha_bw_u, alpha_fw_v, alpha_bw_v, rounding_offset, down_shift, final_offset_u, final_offset_v, LUMA_BLOCK_SIZE, 4);

#ifdef __SUPPORT_YUV400__
	}
#endif
}

void MB_InterPred8x8_1Dir_1 PARGS5(int vec_x_base, int vec_y_base, int list_offset, int b8, int dir)
{
	int mv_x, mv_y;
	int ref_idx;
	// For weighting
	int ref_idx_w;
	int curr_mb_field = ((IMGPAR MbaffFrameFlag)&&(currMB_d->mb_field));
	int weight;
	int weight_u, weight_v;
	int rounding_offset;
	unsigned int down_shift;
	int final_offset;
	int final_offset_u, final_offset_v;

	int i_1    = ((b8&1)<<1);
	int j_1    = (b8&2);
	int ioff = i_1<<2;
	int joff = j_1<<2;

	ref_idx_w = ref_idx = currMB_s_d->pred_info.ref_idx[dir][b8];
	if (ref_idx_w < 0) {
		ref_idx_w = ref_idx = currMB_s_d->pred_info.ref_idx[dir][b8] = 0;
	}

	if (active_pps.weighted_pred_flag && curr_mb_field)
		ref_idx_w >>=1;

	//Luma
	mv_x = currMB_s_d->pred_info.mv[dir][joff+i_1].x;
	mv_y = currMB_s_d->pred_info.mv[dir][joff+i_1].y;

	get_block_8xh ARGS7(listX[list_offset+dir][ref_idx], vec_x_base + ioff, vec_y_base + joff, mv_x, mv_y, 
		reinterpret_cast<unsigned char(*)[16]>(&IMGPAR mpr[joff][ioff]), 8);

	//weighting
	rounding_offset = IMGPAR wp_round_luma;
	down_shift      = IMGPAR luma_log2_weight_denom;

	weight          = (*(IMGPAR wp_weight+(dir*MAX_REFERENCE_PICTURES+ref_idx_w)*3+0));
	final_offset    = (*(IMGPAR wp_offset+(dir*MAX_REFERENCE_PICTURES+ref_idx_w)*3+0));

	weight_8(&IMGPAR mpr[joff][ioff], weight, rounding_offset, down_shift, final_offset, LUMA_BLOCK_SIZE, 8);

#ifdef __SUPPORT_YUV400__
	if (dec_picture->chroma_format_idc != YUV400)
	{
#endif
		//Chroma
		ioff>>=1;
		joff>>=1;
		vec_x_base >>= 1;
		vec_y_base >>= 1;
		int chroma_vec_x_base = vec_x_base+ioff;
		int chroma_vec_y_base = vec_y_base+joff;

		//mv_y += listX[list_offset+dir][ref_idx]->chroma_vector_adjustment;
		mv_y += IMGPAR cr_vector_adjustment[list_offset+dir][ref_idx];

		mb_chroma_4xH ARGS8(listX[list_offset+dir][ref_idx], &IMGPAR mprUV[joff][ioff<<1], LUMA_BLOCK_SIZE,
			chroma_vec_x_base, chroma_vec_y_base, mv_x, mv_y, 4);

		//weighting
		rounding_offset = IMGPAR wp_round_chroma;
		down_shift      = IMGPAR chroma_log2_weight_denom;

		//u
		weight_u          = (*(IMGPAR wp_weight+(dir*MAX_REFERENCE_PICTURES+ref_idx_w)*3+0+1));
		final_offset_u    = (*(IMGPAR wp_offset+(dir*MAX_REFERENCE_PICTURES+ref_idx_w)*3+0+1));

		//v
		weight_v          = (*(IMGPAR wp_weight+(dir*MAX_REFERENCE_PICTURES+ref_idx_w)*3+1+1));
		final_offset_v    = (*(IMGPAR wp_offset+(dir*MAX_REFERENCE_PICTURES+ref_idx_w)*3+1+1));

		weight_4_uv(&IMGPAR mprUV[joff][ioff<<1], weight_u, weight_v, rounding_offset, down_shift, final_offset_u, final_offset_v, LUMA_BLOCK_SIZE, 4);
#ifdef __SUPPORT_YUV400__
	}
#endif
}


void MB_InterPred4x4_BiDir_1 PARGS4(int vec_x_base, int vec_y_base, int list_offset, int b8)
{
	int mv_x, mv_y, mv_x_bw, mv_y_bw;
	int fw_ref_idx, bw_ref_idx;
	int b4;
	// For weighting
	int fw_ref_idx_w, bw_ref_idx_w;
	int curr_mb_field = ((IMGPAR MbaffFrameFlag)&&(currMB_d->mb_field));
	int rounding_offset;
	unsigned int down_shift;
	int final_offset;
	int final_offset_u, final_offset_v;

	int i_1    = ((b8&1)<<1);
	int j_1    = (b8&2);
	int ioff;
	int joff;
	int i;
	int j;

	fw_ref_idx_w = fw_ref_idx = currMB_s_d->pred_info.ref_idx[LIST_0][b8];
	if (fw_ref_idx_w < 0) {
		fw_ref_idx_w = fw_ref_idx = currMB_s_d->pred_info.ref_idx[LIST_0][b8] = 0;
	}
	bw_ref_idx_w = bw_ref_idx = currMB_s_d->pred_info.ref_idx[LIST_1][b8];
	if (bw_ref_idx_w < 0) {
		bw_ref_idx_w = bw_ref_idx = currMB_s_d->pred_info.ref_idx[LIST_1][b8] = 0;
	}
	int alpha_fw, alpha_bw;
	int alpha_fw_u, alpha_bw_u, alpha_fw_v, alpha_bw_v;
	int wt_list_offset = (active_pps.weighted_bipred_idc==2)?list_offset:0;
	if (active_pps.weighted_bipred_idc==1 && curr_mb_field)
	{
		fw_ref_idx_w >>=1;
		bw_ref_idx_w >>=1;
	}

	//int chroma_vector_adjustment = listX[list_offset][fw_ref_idx]->chroma_vector_adjustment;
	//int chroma_vector_adjustment_bw = listX[1+list_offset][bw_ref_idx]->chroma_vector_adjustment;
	int chroma_vector_adjustment = IMGPAR cr_vector_adjustment[list_offset][fw_ref_idx];
	int chroma_vector_adjustment_bw = IMGPAR cr_vector_adjustment[1+list_offset][bw_ref_idx];

	int mode                                      = currMB_d->b8mode[b8];
	int nofiter                                   = get_block_4xh_iter[mode];
	int height                                    = get_block_4xh_height[mode];
	int *get_block_4xh_b4s_ptr                    = get_block_4xh_b4s[mode];
	get_block_wxh* get_block_4xh_ptr              = get_block_4xh_p[mode];
	weight_b_t *weight_4_b_ptr                    = weight_b_p[mode][0];
	mb_chroma_pred_t *mb_chroma_2xH_ptr           = mb_chroma_2xH_p[mode];
	//mb_chroma_pred_mv00_t *mb_chroma_2xH_mv00_ptr = mb_chroma_2xH_mv00_p[mode];
	weight_b_uv_t *weight_2_b_uv_ptr                    = weight_b_uv_p[mode];
	int iter;
	for (iter = 0; iter <nofiter; iter ++)
	{
		b4   = get_block_4xh_b4s_ptr[iter];
		i    = i_1 + (b4&1);
		j    = j_1 + (b4>>1);
		ioff = i<<2;
		joff = j<<2;

		//Luma
		mv_x = currMB_s_d->pred_info.mv[LIST_0][joff+i].x;
		mv_y = currMB_s_d->pred_info.mv[LIST_0][joff+i].y;

		get_block_4xh_ptr ARGS7(listX[list_offset][fw_ref_idx], vec_x_base + ioff, vec_y_base + joff, mv_x, mv_y, fw_block, height);

		mv_x_bw = currMB_s_d->pred_info.mv[LIST_1][joff+i].x;
		mv_y_bw = currMB_s_d->pred_info.mv[LIST_1][joff+i].y;

		get_block_4xh_ptr ARGS7(listX[1+list_offset][bw_ref_idx], vec_x_base + ioff, vec_y_base + joff, mv_x_bw, mv_y_bw, bw_block, height);

		//weighting
		rounding_offset = (1<<IMGPAR luma_log2_weight_denom);
		down_shift      = (IMGPAR luma_log2_weight_denom+1);

		alpha_fw = *(IMGPAR wbp_weight+(((0+wt_list_offset)*MAX_REFERENCE_PICTURES+fw_ref_idx_w)*MAX_REFERENCE_PICTURES+bw_ref_idx_w)*3+0);
		alpha_bw = *(IMGPAR wbp_weight+(((1+wt_list_offset)*MAX_REFERENCE_PICTURES+fw_ref_idx_w)*MAX_REFERENCE_PICTURES+bw_ref_idx_w)*3+0);
		final_offset    = (((*(IMGPAR wp_offset+((wt_list_offset+0)*MAX_REFERENCE_PICTURES+fw_ref_idx_w)*3+0)) + 
			(*(IMGPAR wp_offset+((wt_list_offset+1)*MAX_REFERENCE_PICTURES+bw_ref_idx_w)*3+0)) + 1) >>1);

		weight_4_b_ptr(&IMGPAR mpr[joff][ioff], LUMA_BLOCK_SIZE, &fw_block[0][0], &bw_block[0][0], 
			alpha_fw, alpha_bw, rounding_offset, down_shift, final_offset, LUMA_BLOCK_SIZE, height);

		//		average_4(&IMGPAR mpr[joff][ioff], 16, (imgpel *) fw_block, (imgpel *) bw_block, LUMA_BLOCK_SIZE, BLOCK_SIZE);

#ifdef __SUPPORT_YUV400__
		if (dec_picture->chroma_format_idc != YUV400)
		{
#endif
			//Chroma
			ioff >>= 1;
			joff >>= 1;		
			int chroma_vec_x_base = (vec_x_base>>1)+ioff;
			int chroma_vec_y_base = (vec_y_base>>1)+joff;

			mv_y += chroma_vector_adjustment;

			mb_chroma_2xH_ptr ARGS8(listX[list_offset][fw_ref_idx], &fw_block[0][0], LUMA_BLOCK_SIZE,
				chroma_vec_x_base, chroma_vec_y_base, mv_x, mv_y, height>>1);

			mv_y_bw += chroma_vector_adjustment_bw;

			mb_chroma_2xH_ptr ARGS8(listX[1+list_offset][bw_ref_idx], &bw_block[0][0], LUMA_BLOCK_SIZE,
				chroma_vec_x_base, chroma_vec_y_base, mv_x_bw, mv_y_bw, height>>1);

			//weighting
			rounding_offset = (1<<IMGPAR chroma_log2_weight_denom);
			down_shift      = (IMGPAR chroma_log2_weight_denom + 1);

			//u
			alpha_fw_u = *(IMGPAR wbp_weight+(((0+wt_list_offset)*MAX_REFERENCE_PICTURES+fw_ref_idx_w)*MAX_REFERENCE_PICTURES+bw_ref_idx_w)*3+0+1);
			alpha_bw_u = *(IMGPAR wbp_weight+(((1+wt_list_offset)*MAX_REFERENCE_PICTURES+fw_ref_idx_w)*MAX_REFERENCE_PICTURES+bw_ref_idx_w)*3+0+1);
			final_offset_u    = (((*(IMGPAR wp_offset+((wt_list_offset + 0)*MAX_REFERENCE_PICTURES+fw_ref_idx_w)*3+0+1))
				+ (*(IMGPAR wp_offset+((wt_list_offset + 1)*MAX_REFERENCE_PICTURES+bw_ref_idx_w)*3+0+1)) + 1)>>1);

			//v
			alpha_fw_v = *(IMGPAR wbp_weight+(((0+wt_list_offset)*MAX_REFERENCE_PICTURES+fw_ref_idx_w)*MAX_REFERENCE_PICTURES+bw_ref_idx_w)*3+1+1);
			alpha_bw_v = *(IMGPAR wbp_weight+(((1+wt_list_offset)*MAX_REFERENCE_PICTURES+fw_ref_idx_w)*MAX_REFERENCE_PICTURES+bw_ref_idx_w)*3+1+1);
			final_offset_v    = (((*(IMGPAR wp_offset+((wt_list_offset + 0)*MAX_REFERENCE_PICTURES+fw_ref_idx_w)*3+1+1))
				+ (*(IMGPAR wp_offset+((wt_list_offset + 1)*MAX_REFERENCE_PICTURES+bw_ref_idx_w)*3+1+1)) + 1)>>1);

			weight_2_b_uv_ptr(&IMGPAR mprUV[joff][ioff<<1], LUMA_BLOCK_SIZE, &fw_block[0][0], &bw_block[0][0], 
				alpha_fw_u, alpha_bw_u, alpha_fw_v, alpha_bw_v, rounding_offset, down_shift, final_offset_u, final_offset_v, LUMA_BLOCK_SIZE, height>>1);

#ifdef __SUPPORT_YUV400__
		}
#endif
	}
}


void MB_InterPred4x4_1Dir_1 PARGS5(int vec_x_base, int vec_y_base, int list_offset, int b8, int dir)
{
	int mv_x, mv_y;
	int ref_idx;
	int b4;
	// For weighting
	int ref_idx_w;
	int curr_mb_field = ((IMGPAR MbaffFrameFlag)&&(currMB_d->mb_field));
	int weight;
	int weight_u, weight_v;
	int rounding_offset;
	unsigned int down_shift;
	int final_offset;
	int final_offset_u, final_offset_v;

	int i_1    = ((b8&1)<<1);
	int j_1    = (b8&2);
	int ioff;
	int joff;
	int i;
	int j;

	ref_idx_w = ref_idx = currMB_s_d->pred_info.ref_idx[dir][b8];
	if (ref_idx_w < 0) {
		ref_idx_w = ref_idx = currMB_s_d->pred_info.ref_idx[dir][b8] = 0;
	}
	if (active_pps.weighted_bipred_idc==1 && curr_mb_field)
	{
		ref_idx_w >>=1;
	}
	//int chroma_vector_adjustment = listX[list_offset+dir][ref_idx]->chroma_vector_adjustment;
	int chroma_vector_adjustment = IMGPAR cr_vector_adjustment[list_offset+dir][ref_idx];

	int mode                                      = currMB_d->b8mode[b8];
	int nofiter                                   = get_block_4xh_iter[mode];
	int height                                    = get_block_4xh_height[mode];
	int *get_block_4xh_b4s_ptr                    = get_block_4xh_b4s[mode];
	get_block_wxh* get_block_4xh_ptr              = get_block_4xh_p[mode];
	weight_t *weight_4_ptr                        = weight_p[mode][0];
	mb_chroma_pred_t *mb_chroma_2xH_ptr           = mb_chroma_2xH_p[mode];
	//mb_chroma_pred_mv00_t *mb_chroma_2xH_mv00_ptr = mb_chroma_2xH_mv00_p[mode];
	weight_uv_t *weight_2_ptr                        = weight_uv_p[mode];
	int iter;
	for (iter = 0; iter <nofiter; iter ++)
	{
		b4   = get_block_4xh_b4s_ptr[iter];
		i    = i_1 + (b4&1);
		j    = j_1 + (b4>>1);
		ioff = i<<2;
		joff = j<<2;

		//Luma
		mv_x = currMB_s_d->pred_info.mv[dir][joff+i].x;
		mv_y = currMB_s_d->pred_info.mv[dir][joff+i].y;

		get_block_4xh_ptr ARGS7(listX[list_offset+dir][ref_idx], vec_x_base + ioff, vec_y_base + joff, mv_x, mv_y, 
			reinterpret_cast<unsigned char(*)[16]>(&IMGPAR mpr[joff][ioff]), height);

		//weighting
		rounding_offset = IMGPAR wp_round_luma;
		down_shift      = IMGPAR luma_log2_weight_denom;

		weight          = (*(IMGPAR wp_weight+(dir*MAX_REFERENCE_PICTURES+ref_idx_w)*3+0));
		final_offset    = (*(IMGPAR wp_offset+(dir*MAX_REFERENCE_PICTURES+ref_idx_w)*3+0));
		weight_4_ptr(&IMGPAR mpr[joff][ioff], weight, rounding_offset, down_shift, final_offset, LUMA_BLOCK_SIZE, height);

#ifdef __SUPPORT_YUV400__
		if (dec_picture->chroma_format_idc != YUV400)
		{
#endif
			//Chroma
			ioff >>= 1;
			joff >>= 1;		
			int chroma_vec_x_base = (vec_x_base>>1)+ioff;
			int chroma_vec_y_base = (vec_y_base>>1)+joff;
			mv_y += chroma_vector_adjustment;

			mb_chroma_2xH_ptr ARGS8(listX[list_offset+dir][ref_idx], &IMGPAR mprUV[joff][ioff<<1], LUMA_BLOCK_SIZE,
				chroma_vec_x_base, chroma_vec_y_base, mv_x, mv_y, height>>1);

			//weighting
			rounding_offset = IMGPAR wp_round_chroma;
			down_shift      = IMGPAR chroma_log2_weight_denom;

			//u
			weight_u          = (*(IMGPAR wp_weight+(dir*MAX_REFERENCE_PICTURES+ref_idx_w)*3+0+1));
			final_offset_u    = (*(IMGPAR wp_offset+(dir*MAX_REFERENCE_PICTURES+ref_idx_w)*3+0+1));

			//v
			weight_v          = (*(IMGPAR wp_weight+(dir*MAX_REFERENCE_PICTURES+ref_idx_w)*3+1+1));
			final_offset_v    = (*(IMGPAR wp_offset+(dir*MAX_REFERENCE_PICTURES+ref_idx_w)*3+1+1));

			weight_2_ptr(&IMGPAR mprUV[joff][ioff<<1], weight_u, weight_v, rounding_offset, down_shift, final_offset_u, final_offset_v, LUMA_BLOCK_SIZE, height>>1);			
#ifdef __SUPPORT_YUV400__
		}
#endif
	}
}



void MB_InterPred_b8mode_P0 PARGS3(int vec_x_base, int vec_y_base, int list_offset)
{
	for (int b8=0; b8<4; b8++)
	{
		int mv_mode = currMB_d->b8mode[b8];

		if(mv_mode==4)
		{
			MB_InterPred8x8_1Dir ARGS5(vec_x_base, vec_y_base, list_offset, b8, 0);
		}
		else
		{
			MB_InterPred4x4_1Dir ARGS5(vec_x_base, vec_y_base, list_offset, b8, 0);
		}
	} //b8
}

void MB_InterPred_b8mode_P1 PARGS3(int vec_x_base, int vec_y_base, int list_offset)
{
	for (int b8=0; b8<4; b8++)
	{
		int mv_mode = currMB_d->b8mode[b8];

		if(mv_mode==4)
		{
			MB_InterPred8x8_1Dir_1 ARGS5(vec_x_base, vec_y_base, list_offset, b8, 0);
		}
		else
		{
			MB_InterPred4x4_1Dir_1 ARGS5(vec_x_base, vec_y_base, list_offset, b8, 0);
		}
	}//b8
}
void MB_InterPred_b8mode_B0 PARGS3(int vec_x_base, int vec_y_base, int list_offset)
{
	int fw_refframe, bw_refframe;

	for (int b8=0; b8<4; b8++)
	{
		int mv_mode = currMB_d->b8mode[b8];
		int pred_dir = currMB_d->b8pdir[b8];
		assert (pred_dir<=2);

		if (pred_dir != 2) //P
		{
			if(mv_mode==4)
			{
				MB_InterPred8x8_1Dir ARGS5(vec_x_base, vec_y_base, list_offset, b8, pred_dir);
			}
			else
			{
				MB_InterPred4x4_1Dir ARGS5(vec_x_base, vec_y_base, list_offset, b8, pred_dir);
			}
		}
		else if (mv_mode != 0) //B
		{
			if(mv_mode==4)
			{
				MB_InterPred8x8_BiDir ARGS4(vec_x_base, vec_y_base, list_offset, b8);
			}
			else
			{
				MB_InterPred4x4_BiDir ARGS4(vec_x_base, vec_y_base, list_offset, b8);
			}
		}
		else //B_Direct
		{
			if (IMGPAR direct_spatial_mv_pred_flag) //equal to direct_flag is true in uv loop
			{
				fw_refframe = currMB_s_d->pred_info.ref_idx[LIST_0][b8];
				bw_refframe = currMB_s_d->pred_info.ref_idx[LIST_1][b8];

				//direct spatial
				if (currMB_d->NoMbPartLessThan8x8Flag)
				{
					//if (bw_refframe==-1)
					if (bw_refframe <= -1)
					{
						//direct_pdir = 0;
						MB_InterPred8x8_1Dir ARGS5(vec_x_base, vec_y_base, list_offset, b8, 0);
					}
					//else if (fw_refframe==-1)
					else if (fw_refframe <= -1)
					{
						//direct_pdir = 1;
						//luma
						MB_InterPred8x8_1Dir ARGS5(vec_x_base, vec_y_base, list_offset, b8, 1);
					}
					else
					{
						//direct_pdir = 2;
						MB_InterPred8x8_BiDir ARGS4(vec_x_base, vec_y_base, list_offset, b8);
					}
				}
				else
				{
					//if (bw_refframe==-1)
					if (bw_refframe <= -1)
					{
						MB_InterPred4x4_1Dir ARGS5(vec_x_base, vec_y_base, list_offset, b8, 0);
					}
					//else if (fw_refframe==-1)
					else if (fw_refframe <= -1)
					{
						MB_InterPred4x4_1Dir ARGS5(vec_x_base, vec_y_base, list_offset, b8, 1);
					}
					else
					{
						//direct_pdir = 2;
						MB_InterPred4x4_BiDir ARGS4(vec_x_base, vec_y_base, list_offset, b8);
					}
				}
			}
			else
			{
				//direct temporal
				if(currMB_d->NoMbPartLessThan8x8Flag)
				{
					MB_InterPred8x8_BiDir ARGS4(vec_x_base, vec_y_base, list_offset, b8);
				}
				else
				{
					MB_InterPred4x4_BiDir ARGS4(vec_x_base, vec_y_base, list_offset, b8);
				}
			}
		}
	}//b8
}

void MB_InterPred_b8mode_B1 PARGS3(int vec_x_base, int vec_y_base, int list_offset)
{
	int fw_refframe, bw_refframe;

	for (int b8=0; b8<4; b8++)
	{
		int mv_mode = currMB_d->b8mode[b8];
		int pred_dir = currMB_d->b8pdir[b8];
		assert (pred_dir<=2);

		if (pred_dir != 2) //P
		{
			if(mv_mode==4)
			{
				MB_InterPred8x8_1Dir_1 ARGS5(vec_x_base, vec_y_base, list_offset, b8, pred_dir);
			}
			else
			{
				MB_InterPred4x4_1Dir_1 ARGS5(vec_x_base, vec_y_base, list_offset, b8, pred_dir);
			}
		}
		else if (mv_mode != 0) //B
		{
			if(mv_mode==4)
			{
				MB_InterPred8x8_BiDir_1 ARGS4(vec_x_base, vec_y_base, list_offset, b8);
			}
			else
			{
				MB_InterPred4x4_BiDir_1 ARGS4(vec_x_base, vec_y_base, list_offset, b8);	
			}
		}
		else //B_Direct
		{
			if (IMGPAR direct_spatial_mv_pred_flag) //equal to direct_flag is true in uv loop
			{
				fw_refframe = currMB_s_d->pred_info.ref_idx[LIST_0][b8];
				bw_refframe = currMB_s_d->pred_info.ref_idx[LIST_1][b8];

				//direct spatial
				if (currMB_d->NoMbPartLessThan8x8Flag)
				{
					if (bw_refframe <= -1)
					{
						//direct_pdir = 0;
						MB_InterPred8x8_1Dir_1 ARGS5(vec_x_base, vec_y_base, list_offset, b8, 0);
					}
					else if (fw_refframe <= -1) 
					{
						//direct_pdir = 1;
						MB_InterPred8x8_1Dir_1 ARGS5(vec_x_base, vec_y_base, list_offset, b8, 1);
					}
					else
					{
						//direct_pdir = 2;
						MB_InterPred8x8_BiDir_1 ARGS4(vec_x_base, vec_y_base, list_offset, b8);
					}
				}
				else
				{
					if (bw_refframe <= -1)
					{
						//direct_pdir = 0;
						MB_InterPred4x4_1Dir_1 ARGS5(vec_x_base, vec_y_base, list_offset, b8, 0);
					}
					else if (fw_refframe <= -1)
					{
						//direct_pdir = 1;
						MB_InterPred4x4_1Dir_1 ARGS5(vec_x_base, vec_y_base, list_offset, b8, 1);
					}
					else
					{
						//direct_pdir = 2;
						MB_InterPred4x4_BiDir_1 ARGS4(vec_x_base, vec_y_base, list_offset, b8);
					}
				}
			}
			else
			{
				//direct temporal
				if(currMB_d->NoMbPartLessThan8x8Flag)
				{
					MB_InterPred8x8_BiDir_1 ARGS4(vec_x_base, vec_y_base, list_offset, b8);
				}
				else
				{
					MB_InterPred4x4_BiDir_1 ARGS4(vec_x_base, vec_y_base, list_offset, b8);
				}
			}
		}
	}//b8
}
//#endif
