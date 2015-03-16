#include "global.h"
#include "get_block.h"
#ifdef H264_ENABLE_INTRINSICS
#include <mmintrin.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#endif

// Disable "No EMMS at end of function '<function name>'"
#pragma warning ( disable : 4799 )

get_block_4xh_t*  get_block_4xh_fp[16];
get_block_8xh_t*  get_block_8xh_fp[16];
get_block_16xh_t* get_block_16xh_fp[16];

get_block_4xh_t* get_block_4xh_c_fp[] = 
{	
	get_block_4xh_p00_c, get_block_4xh_p01_c, get_block_4xh_p02_c, get_block_4xh_p03_c, 
	get_block_4xh_p10_c, get_block_4xh_p11_c, get_block_4xh_p12_c, get_block_4xh_p13_c,
	get_block_4xh_p20_c, get_block_4xh_p21_c, get_block_4xh_p22_c, get_block_4xh_p23_c,
	get_block_4xh_p30_c, get_block_4xh_p31_c, get_block_4xh_p32_c, get_block_4xh_p33_c
};
get_block_8xh_t* get_block_8xh_c_fp[] = 
{	
	get_block_8xh_p00_c, get_block_8xh_p01_c, get_block_8xh_p02_c, get_block_8xh_p03_c, 
	get_block_8xh_p10_c, get_block_8xh_p11_c, get_block_8xh_p12_c, get_block_8xh_p13_c,
	get_block_8xh_p20_c, get_block_8xh_p21_c, get_block_8xh_p22_c, get_block_8xh_p23_c,
	get_block_8xh_p30_c, get_block_8xh_p31_c, get_block_8xh_p32_c, get_block_8xh_p33_c
};
get_block_16xh_t* get_block_16xh_c_fp[] = 
{	
	get_block_16xh_p00_c, get_block_16xh_p01_c, get_block_16xh_p02_c, get_block_16xh_p03_c, 
	get_block_16xh_p10_c, get_block_16xh_p11_c, get_block_16xh_p12_c, get_block_16xh_p13_c,
	get_block_16xh_p20_c, get_block_16xh_p21_c, get_block_16xh_p22_c, get_block_16xh_p23_c,
	get_block_16xh_p30_c, get_block_16xh_p31_c, get_block_16xh_p32_c, get_block_16xh_p33_c
};

#ifdef H264_ENABLE_INTRINSICS
get_block_4xh_t* get_block_4xh_mmx_fp[] = 
{	
	get_block_4xh_p00_mmx, get_block_4xh_p01_mmx, get_block_4xh_p02_mmx, get_block_4xh_p03_mmx, 
	get_block_4xh_p10_mmx, get_block_4xh_p11_mmx, get_block_4xh_p12_mmx, get_block_4xh_p13_mmx,
	get_block_4xh_p20_mmx, get_block_4xh_p21_mmx, get_block_4xh_p22_mmx, get_block_4xh_p23_mmx,
	get_block_4xh_p30_mmx, get_block_4xh_p31_mmx, get_block_4xh_p32_mmx, get_block_4xh_p33_mmx
};
get_block_8xh_t* get_block_8xh_sse2_fp[] = 
{	
	get_block_8xh_p00_sse2, get_block_8xh_p01_sse2, get_block_8xh_p02_sse2, get_block_8xh_p03_sse2, 
	get_block_8xh_p10_sse2, get_block_8xh_p11_sse2, get_block_8xh_p12_sse2, get_block_8xh_p13_sse2,
	get_block_8xh_p20_sse2, get_block_8xh_p21_sse2, get_block_8xh_p22_sse2, get_block_8xh_p23_sse2,
	get_block_8xh_p30_sse2, get_block_8xh_p31_sse2, get_block_8xh_p32_sse2, get_block_8xh_p33_sse2
};
get_block_8xh_t* get_block_8xh_sse_fp[] = 
{	
	get_block_8xh_p00_sse, get_block_8xh_p01_sse, get_block_8xh_p02_sse, get_block_8xh_p03_sse, 
	get_block_8xh_p10_sse, get_block_8xh_p11_sse, get_block_8xh_p12_sse, get_block_8xh_p13_sse,
	get_block_8xh_p20_sse, get_block_8xh_p21_sse, get_block_8xh_p22_sse, get_block_8xh_p23_sse,
	get_block_8xh_p30_sse, get_block_8xh_p31_sse, get_block_8xh_p32_sse, get_block_8xh_p33_sse
};
get_block_16xh_t* get_block_16xh_sse2_fp[] = 
{	
	get_block_16xh_p00_sse2, get_block_16xh_p01_sse2, get_block_16xh_p02_sse2, get_block_16xh_p03_sse2, 
	get_block_16xh_p10_sse2, get_block_16xh_p11_sse2, get_block_16xh_p12_sse2, get_block_16xh_p13_sse2,
	get_block_16xh_p20_sse2, get_block_16xh_p21_sse2, get_block_16xh_p22_sse2, get_block_16xh_p23_sse2,
	get_block_16xh_p30_sse2, get_block_16xh_p31_sse2, get_block_16xh_p32_sse2, get_block_16xh_p33_sse2
};
get_block_16xh_t* get_block_16xh_sse_fp[] = 
{	
	get_block_16xh_p00_sse, get_block_16xh_p01_sse, get_block_16xh_p02_sse, get_block_16xh_p03_sse, 
	get_block_16xh_p10_sse, get_block_16xh_p11_sse, get_block_16xh_p12_sse, get_block_16xh_p13_sse,
	get_block_16xh_p20_sse, get_block_16xh_p21_sse, get_block_16xh_p22_sse, get_block_16xh_p23_sse,
	get_block_16xh_p30_sse, get_block_16xh_p31_sse, get_block_16xh_p32_sse, get_block_16xh_p33_sse
};
#endif

void get_block PARGS6(StorablePicture *p, int base_x_pos, int base_y_pos, int mv_x, int mv_y, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE])
{
	byte *src;
	const int stride = p->Y_stride;
	//int dx     = mv_x&3;
	//int dy     = mv_y&3;
	int x_pos  = base_x_pos + (mv_x>>2);
	int y_pos  = base_y_pos + (mv_y>>2);
	x_pos = __fast_iclip(clip_min_x,clip_max_x,x_pos);

	int jj;
	// New code, y_pos attached to the boundary
#if defined (_COLLECT_PIC_)
	if ( IMGPAR smart_dec & SMART_DEC_BITBYBIT ) {
		EnterCriticalSection( &(IMGPAR stream_global->crit_dyna_expand) );
	}
#endif

	if(y_pos<2)
	{    // We need to worry about Frame/Field difference in extended area
		if(y_pos<(-(4+2))) 	// specific to 4x4 blocks
			y_pos = (-(4+2));
		src = p->imgY + x_pos-2;
		for(jj=y_pos-2;jj<0;jj++)
			memcpy(src+jj*p->Y_stride, src, 2+4+3); // specific to 4x4 blocks
	}
	else if (y_pos>clip_max_y-(4+2))	// specific to 4x4 blocks
	{
		if(y_pos>clip_max_y+2)
			y_pos = clip_max_y+2;
		src = p->imgY + clip_max_y*p->Y_stride + x_pos-2;
		for(jj=1;jj<=y_pos-clip_max_y+4+2;jj++)	// specific to 4x4 blocks
			memcpy(src+jj*p->Y_stride, src, 2+4+3); // specific to 4x4 blocks
	}

	src = p->imgY +y_pos*stride+x_pos;
#ifdef H264_ENABLE_INTRINSICS
	_mm_prefetch(((char*)(src)),3);
	_mm_prefetch(((char*)(src+stride)),3);
	_mm_prefetch(((char*)(src+stride*2)),3);
	_mm_prefetch(((char*)(src+stride*3)),3);		
#endif

	if(((mv_x&3)+(mv_y&3))==0)// mv_x=0 and mv_y=0
	{
		*((unsigned long*)&block[0]) = *((unsigned long*)(src     ));
		*((unsigned long*)&block[1]) = *((unsigned long*)(src+  stride));
		*((unsigned long*)&block[2]) = *((unsigned long*)(src+(stride<<1)));
		*((unsigned long*)&block[3]) = *((unsigned long*)(src+stride+(stride<<1)));
	} else {
		get_block_4xh_fp[((mv_x&3)<<2)+(mv_y&3)](src, block, stride, 4);
	}
#if defined (_COLLECT_PIC_)
	if ( IMGPAR smart_dec & SMART_DEC_BITBYBIT ) {
		LeaveCriticalSection( &(IMGPAR stream_global->crit_dyna_expand) );
	}
#endif

}

void  get_block_4xh_int PARGS7(StorablePicture *p, int base_x_pos, int base_y_pos, int mv_x, int mv_y, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int height)
{	
	byte *src;
	const int stride = p->Y_stride;
	int dx     = 0;
	int dy     = 0;
	int x_pos  = base_x_pos + (mv_x>>2);
	int y_pos  = base_y_pos + (mv_y>>2);
	x_pos = __fast_iclip(clip_min_x,clip_max_x,x_pos);

	int jj;
	// New code, y_pos attached to the boundary
#if defined (_COLLECT_PIC_)
	if ( IMGPAR smart_dec & SMART_DEC_BITBYBIT ) {
		EnterCriticalSection( &(IMGPAR stream_global->crit_dyna_expand) );
	}
#endif

	if(y_pos<2)
	{    // We need to worry about Frame/Field difference in extended area
		if(y_pos<(-(height+2))) 	// specific to 4xh blocks
			y_pos = (-(height+2));
		src = p->imgY + x_pos-2;
		for(jj=y_pos-2;jj<0;jj++)
			memcpy(src+jj*p->Y_stride, src, 2+4+3); // specific to 4xh blocks
	}
	else if (y_pos>clip_max_y-(height+2))	// specific to 4xh blocks
	{
		if(y_pos>clip_max_y+2)
			y_pos = clip_max_y+2;
		src = p->imgY + clip_max_y*p->Y_stride + x_pos-2;
		for(jj=1;jj<=y_pos-clip_max_y+height+2;jj++)	// specific to 4xh blocks
			memcpy(src+jj*p->Y_stride, src, 2+4+3); // specific to 4xh blocks
	}

	src = p->imgY +y_pos*stride+x_pos;

	/*
	_mm_prefetch(((char*)(src)),3);
	_mm_prefetch(((char*)(src+stride)),3);
	_mm_prefetch(((char*)(src+stride*2)),3);
	_mm_prefetch(((char*)(src+stride*3)),3);		
	*/

	imgpel *pblock = &block[0][0];
	for(int i = height; i > 0; i-=2)
	{
		*(unsigned long*)(pblock+0*LUMA_BLOCK_SIZE) = *(unsigned long*)(src);
		*(unsigned long*)(pblock+1*LUMA_BLOCK_SIZE) = *(unsigned long*)(src+stride);
		src    += 2*stride;
		pblock += 2*LUMA_BLOCK_SIZE;
	}

#if defined (_COLLECT_PIC_)
	if ( IMGPAR smart_dec & SMART_DEC_BITBYBIT ) {
		LeaveCriticalSection( &(IMGPAR stream_global->crit_dyna_expand) );
	}
#endif
}

void  get_block_8xh_int PARGS7(StorablePicture *p, int base_x_pos, int base_y_pos, int mv_x, int mv_y, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int height)
{
	byte *src;
	const int stride = p->Y_stride;
	int dx     = 0;
	int dy     = 0;
	int x_pos  = base_x_pos + (mv_x>>2);
	int y_pos  = base_y_pos + (mv_y>>2);

	x_pos = __fast_iclip(clip_min_x,clip_max_x,x_pos);

	int jj;
	// New code, y_pos attached to the boundary
#if defined (_COLLECT_PIC_)
	if ( IMGPAR smart_dec & SMART_DEC_BITBYBIT ) {
		EnterCriticalSection( &(IMGPAR stream_global->crit_dyna_expand) );
	}
#endif

	if(y_pos<2)
	{    // We need to worry about Frame/Field difference in extended area
		if(y_pos<(-(height+2))) 	// specific to 8xh blocks
			y_pos = (-(height+2));
		src = p->imgY + x_pos-2;
		for(jj=y_pos-2;jj<0;jj++)
			memcpy(src+jj*stride, src, 2+8+3); // specific to 8xh blocks
	}
	else if (y_pos>clip_max_y-(height+2))	// specific to 8xh blocks
	{
		if(y_pos>clip_max_y+2)
			y_pos = clip_max_y+2;
		src = p->imgY + clip_max_y*stride + x_pos-2;
		for(jj=1;jj<=y_pos-clip_max_y+height+2;jj++)	// specific to 8xh blocks
			memcpy(src+jj*stride, src, 2+8+3); // specific to 8xh blocks
	}

	src = p->imgY +y_pos*stride+x_pos;

	/*
	const int range = (stride<<4);
	const int offset = (stride<<2);	
	for(i=stride ; i< range; i+=offset)
	{
	_mm_prefetch(((char*)(src+i)),3);
	_mm_prefetch(((char*)(src+i+stride)),3);
	_mm_prefetch(((char*)(src+i+stride*2)),3);
	_mm_prefetch(((char*)(src+i+stride*3)),3);		
	}
	*/

#ifdef H264_ENABLE_ASM
	__asm
	{
		mov  eax, src;
		mov  ebx, stride;
		mov  ecx, block;
		mov  edx, height;
LOOP_4:
		movq mm0, [eax];
		movq mm1, [eax+ebx];
		movq [ecx], mm0;
		movq [ecx+16], mm1;
		lea  eax, [eax+2*ebx];
		movq mm2, [eax];
		movq mm3, [eax+ebx];
		movq [ecx+32], mm2;
		movq [ecx+48], mm3;
		lea  eax, [eax+2*ebx];
		add  ecx, 64;
		sub  edx, 4;
		jg   LOOP_4;
	}
#else 
	if((cpu_type == CPU_LEVEL_MMX) ||(cpu_type == CPU_LEVEL_SSE) || (cpu_type == CPU_LEVEL_SSE2) || (cpu_type == CPU_LEVEL_SSE3))
	{
		imgpel *pblock = &block[0][0];
		for(int i = height; i > 0; i-=4)
		{
			//*((unsigned long long*)&block[i][0]) = *((unsigned long long*)src);
			*(__m64*)(pblock+0*LUMA_BLOCK_SIZE) = *((__m64*)&src[0*stride]);
			*(__m64*)(pblock+1*LUMA_BLOCK_SIZE) = *((__m64*)&src[1*stride]);
			*(__m64*)(pblock+2*LUMA_BLOCK_SIZE) = *((__m64*)&src[2*stride]);
			*(__m64*)(pblock+3*LUMA_BLOCK_SIZE) = *((__m64*)&src[3*stride]);
			src    += 4*stride;
			pblock += 4*LUMA_BLOCK_SIZE;
		}
	}
	else
	{
		for(int i = 0; i < height; i+=4) // loop unrolling for 8x16 and 8x8 and 8x4
		{
#ifdef _NO_INT64_
		memcpy((unsigned char *)&block[i]  , src, 8);
		memcpy((unsigned char *)&block[i+1], src+stride, 8);
		src += 2*stride;
		memcpy((unsigned char *)&block[i+2], src, 8);
		memcpy((unsigned char *)&block[i+3], src+stride, 8);
		src += 2*stride;
#else
			*((int64*)&block[i]) = *((int64*)(src));
			src += stride;
			*((int64*)&block[i+1]) = *((int64*)(src));
			src += stride;
			*((int64*)&block[i+2]) = *((int64*)(src));
			src += stride;
			*((int64*)&block[i+3]) = *((int64*)(src));
			src += stride;
		}
	}
#endif
	
#endif
#if defined (_COLLECT_PIC_)
	if ( IMGPAR smart_dec & SMART_DEC_BITBYBIT ) {
		LeaveCriticalSection( &(IMGPAR stream_global->crit_dyna_expand) );
	}
#endif

}

void  get_block_16xh_int PARGS7(StorablePicture *p, int base_x_pos, int base_y_pos, int mv_x, int mv_y, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int height)
{
	byte *src;
	const int stride = p->Y_stride;
	int dx     = 0;
	int dy     = 0;
	int x_pos  = base_x_pos + (mv_x>>2);
	int y_pos  = base_y_pos + (mv_y>>2);
	x_pos = __fast_iclip(clip_min_x,clip_max_x,x_pos);

	int jj;
	// New code, y_pos attached to the boundary
#if defined (_COLLECT_PIC_)
	if ( IMGPAR smart_dec & SMART_DEC_BITBYBIT ) {
		EnterCriticalSection( &(IMGPAR stream_global->crit_dyna_expand) );
	}
#endif

	if(y_pos<2)
	{    // We need to worry about Frame/Field difference in extended area
		if(y_pos<(-(height+2))) 	// specific to 16xh blocks
			y_pos = (-(height+2));
		src = p->imgY + x_pos-2;
		for(jj=y_pos-2;jj<0;jj++)
			memcpy(src+jj*p->Y_stride, src, 2+16+3); // specific to 16xh blocks
	}
	else if (y_pos>clip_max_y-(height+2))	// specific to 16xh blocks
	{
		if(y_pos>clip_max_y+2)
			y_pos = clip_max_y+2;
		src = p->imgY + clip_max_y*p->Y_stride + x_pos-2;
		for(jj=1;jj<=y_pos-clip_max_y+height+2;jj++)	// specific to 16xh blocks
			memcpy(src+jj*p->Y_stride, src, 2+16+3); // specific to 16xh blocks
	}

	src = p->imgY +y_pos*stride+x_pos;
	/*
	const int range = (stride<<4);
	for(i=stride ; i< range; i+=stride)
	{
	_mm_prefetch(((char*)(src+i)),3);
	}
	*/

#ifdef H264_ENABLE_ASM
	__asm
	{
		mov  eax, src;
		mov  ebx, stride;
		mov  ecx, block;
		mov  edx, height;
LOOP_8:
		movq mm0, [eax];
		movq mm1, [eax+8];
		movq mm2, [eax+ebx];
		movq mm3, [eax+ebx+8];
		movq [ecx], mm0;
		movq [ecx+8], mm1;
		movq [ecx+16], mm2;
		movq [ecx+24], mm3;
		lea  eax, [eax+2*ebx];
		movq mm4, [eax];
		movq mm5, [eax+8];
		movq mm6, [eax+ebx];
		movq mm7, [eax+ebx+8];
		movq [ecx+32], mm4;
		movq [ecx+40], mm5;
		movq [ecx+48], mm6;
		movq [ecx+56], mm7;
		lea  eax, [eax+2*ebx];
		movq mm0, [eax];
		movq mm1, [eax+8];
		movq mm2, [eax+ebx];
		movq mm3, [eax+ebx+8];
		movq [ecx+64], mm0;
		movq [ecx+72], mm1;
		movq [ecx+80], mm2;
		movq [ecx+88], mm3;
		lea  eax, [eax+2*ebx];
		movq mm4, [eax];
		movq mm5, [eax+8];
		movq mm6, [eax+ebx];
		movq mm7, [eax+ebx+8];
		movq [ecx+96], mm4;
		movq [ecx+104], mm5;
		movq [ecx+112], mm6;
		movq [ecx+120], mm7;
		lea  eax, [eax+2*ebx];
		add  ecx, 128;
		sub  edx, 8;
		jg   LOOP_8;
	}
#else 
	if((cpu_type == CPU_LEVEL_MMX) ||(cpu_type == CPU_LEVEL_SSE) || (cpu_type == CPU_LEVEL_SSE2) || (cpu_type == CPU_LEVEL_SSE3))
	{
		imgpel *pblock = &block[0][0];
		for(int i = height; i > 0; i-=8)
		{
		//*((unsigned long long*)&block[i][0]) = *((unsigned long long*)src);
			*(__m64*)(pblock+0*LUMA_BLOCK_SIZE)   = *((__m64*)&src[0*stride]);
			*(__m64*)(pblock+0*LUMA_BLOCK_SIZE+8) = *((__m64*)&src[0*stride+8]);
			*(__m64*)(pblock+1*LUMA_BLOCK_SIZE)   = *((__m64*)&src[1*stride]);
			*(__m64*)(pblock+1*LUMA_BLOCK_SIZE+8) = *((__m64*)&src[1*stride+8]);
			*(__m64*)(pblock+2*LUMA_BLOCK_SIZE)   = *((__m64*)&src[2*stride]);
			*(__m64*)(pblock+2*LUMA_BLOCK_SIZE+8) = *((__m64*)&src[2*stride+8]);
			*(__m64*)(pblock+3*LUMA_BLOCK_SIZE)   = *((__m64*)&src[3*stride]);
			*(__m64*)(pblock+3*LUMA_BLOCK_SIZE+8) = *((__m64*)&src[3*stride+8]);
			*(__m64*)(pblock+4*LUMA_BLOCK_SIZE)   = *((__m64*)&src[4*stride]);
			*(__m64*)(pblock+4*LUMA_BLOCK_SIZE+8) = *((__m64*)&src[4*stride+8]);
			*(__m64*)(pblock+5*LUMA_BLOCK_SIZE)   = *((__m64*)&src[5*stride]);
			*(__m64*)(pblock+5*LUMA_BLOCK_SIZE+8) = *((__m64*)&src[5*stride+8]);
			*(__m64*)(pblock+6*LUMA_BLOCK_SIZE)   = *((__m64*)&src[6*stride]);
			*(__m64*)(pblock+6*LUMA_BLOCK_SIZE+8) = *((__m64*)&src[6*stride+8]);
			*(__m64*)(pblock+7*LUMA_BLOCK_SIZE)   = *((__m64*)&src[7*stride]);
			*(__m64*)(pblock+7*LUMA_BLOCK_SIZE+8) = *((__m64*)&src[7*stride+8]);
			src    += 8*stride;
			pblock += 8*LUMA_BLOCK_SIZE;
		}
	}
	else
	{
		for(int i = 0; i < height; i+=8) // loop unrolling for 16x16 and 16x8
		{
#ifdef _NO_INT64_
		memcpy((unsigned char *)&block[i  ][0], src, 16);
		memcpy((unsigned char *)&block[i+1][0], src+stride, 16);
		src += 2*stride;
		memcpy((unsigned char *)&block[i+2][0], src, 16);
		memcpy((unsigned char *)&block[i+3][0], src+stride, 16);
		src += 2*stride;
		memcpy((unsigned char *)&block[i+4][0], src, 16);
		memcpy((unsigned char *)&block[i+5][0], src+stride, 16);
		src += 2*stride;
		memcpy((unsigned char *)&block[i+6][0], src, 16);
		memcpy((unsigned char *)&block[i+7][0], src+stride, 16);
		src += 2*stride;
#else
			*((int64*)&block[i][0]) = *((int64*)(src));
			*((int64*)&block[i][8]) = *((int64*)(src+8));
			src += stride;
			*((int64*)&block[i+1][0]) = *((int64*)(src));
			*((int64*)&block[i+1][8]) = *((int64*)(src+8));
			src += stride;
			*((int64*)&block[i+2][0]) = *((int64*)(src));
			*((int64*)&block[i+2][8]) = *((int64*)(src+8));
			src	+= stride;
			*((int64*)&block[i+3][0]) = *((int64*)(src));
			*((int64*)&block[i+3][8]) = *((int64*)(src+8));
			src += stride;
			*((int64*)&block[i+4][0]) = *((int64*)(src));
			*((int64*)&block[i+4][8]) = *((int64*)(src+8));
			src += stride;
			*((int64*)&block[i+5][0]) = *((int64*)(src));
			*((int64*)&block[i+5][8]) = *((int64*)(src+8));
			src += stride;
			*((int64*)&block[i+6][0]) = *((int64*)(src));
			*((int64*)&block[i+6][8]) = *((int64*)(src+8));
			src += stride;
			*((int64*)&block[i+7][0]) = *((int64*)(src));
			*((int64*)&block[i+7][8]) = *((int64*)(src+8));
			src += stride;
		}
#endif
	}			
#endif
#if defined (_COLLECT_PIC_)
	if ( IMGPAR smart_dec & SMART_DEC_BITBYBIT ) {
		LeaveCriticalSection( &(IMGPAR stream_global->crit_dyna_expand) );
	}
#endif
}

void  get_block_4xh_full PARGS7(StorablePicture *p, int base_x_pos, int base_y_pos, int mv_x, int mv_y, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int height)
{	
	byte *src;
	const int stride = p->Y_stride;
	int dx     = mv_x&3;
	int dy     = mv_y&3;
	int x_pos  = base_x_pos + (mv_x>>2);
	int y_pos  = base_y_pos + (mv_y>>2);
	x_pos = __fast_iclip(clip_min_x,clip_max_x,x_pos);

	int jj;
	// New code, y_pos attached to the boundary
#if defined (_COLLECT_PIC_)
	if ( IMGPAR smart_dec & SMART_DEC_BITBYBIT ) {
		EnterCriticalSection( &(IMGPAR stream_global->crit_dyna_expand) );
	}
#endif

	if(y_pos<2)
	{    // We need to worry about Frame/Field difference in extended area
		if(y_pos<(-(height+2))) 	// specific to 4xh blocks
			y_pos = (-(height+2));
		src = p->imgY + x_pos-2;
		for(jj=y_pos-2;jj<0;jj++)
			memcpy(src+jj*p->Y_stride, src, 2+4+3); // specific to 4xh blocks
	}
	else if (y_pos>clip_max_y-(height+2))	// specific to 4xh blocks
	{
		if(y_pos>clip_max_y+2)
			y_pos = clip_max_y+2;
		src = p->imgY + clip_max_y*p->Y_stride + x_pos-2;
		for(jj=1;jj<=y_pos-clip_max_y+height+2;jj++)	// specific to 4xh blocks
			memcpy(src+jj*p->Y_stride, src, 2+4+3); // specific to 4xh blocks
	}

	src = p->imgY +y_pos*stride+x_pos;

	/*
	_mm_prefetch(((char*)(src)),3);
	_mm_prefetch(((char*)(src+stride)),3);
	_mm_prefetch(((char*)(src+stride*2)),3);
	_mm_prefetch(((char*)(src+stride*3)),3);		
	*/

	if((dx+dy)==0)// mv_x=0 and mv_y=0
	{
		imgpel *pblock = &block[0][0];
		for(int i = height; i > 0; i-=2)
		{
			*(unsigned long*)(pblock+0*LUMA_BLOCK_SIZE) = *(unsigned long*)(src);
			*(unsigned long*)(pblock+1*LUMA_BLOCK_SIZE) = *(unsigned long*)(src+stride);
			src    += 2*stride;
			pblock += 2*LUMA_BLOCK_SIZE;
		}
	} else {
		get_block_4xh_fp[(dx<<2)+dy](src, block, stride, height);
	}

#if defined (_COLLECT_PIC_)
	if ( IMGPAR smart_dec & SMART_DEC_BITBYBIT ) {
		LeaveCriticalSection( &(IMGPAR stream_global->crit_dyna_expand) );
	}
#endif
}

void  get_block_8xh_full PARGS7(StorablePicture *p, int base_x_pos, int base_y_pos, int mv_x, int mv_y, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int height)
{
	byte *src;
	const int stride = p->Y_stride;
	int dx     = mv_x&3;
	int dy     = mv_y&3;
	int x_pos  = base_x_pos + (mv_x>>2);
	int y_pos  = base_y_pos + (mv_y>>2);

	x_pos = __fast_iclip(clip_min_x,clip_max_x,x_pos);

	int jj;
	// New code, y_pos attached to the boundary
#if defined (_COLLECT_PIC_)
	if ( IMGPAR smart_dec & SMART_DEC_BITBYBIT ) {
		EnterCriticalSection( &(IMGPAR stream_global->crit_dyna_expand) );
	}
#endif

	if(y_pos<2)
	{    // We need to worry about Frame/Field difference in extended area
		if(y_pos<(-(height+2))) 	// specific to 8xh blocks
			y_pos = (-(height+2));
		src = p->imgY + x_pos-2;
		for(jj=y_pos-2;jj<0;jj++)
			memcpy(src+jj*stride, src, 2+8+3); // specific to 8xh blocks
	}
	else if (y_pos>clip_max_y-(height+2))	// specific to 8xh blocks
	{
		if(y_pos>clip_max_y+2)
			y_pos = clip_max_y+2;
		src = p->imgY + clip_max_y*stride + x_pos-2;
		for(jj=1;jj<=y_pos-clip_max_y+height+2;jj++)	// specific to 8xh blocks
			memcpy(src+jj*stride, src, 2+8+3); // specific to 8xh blocks
	}

	src = p->imgY +y_pos*stride+x_pos;

	/*
	const int range = (stride<<4);
	const int offset = (stride<<2);	
	for(i=stride ; i< range; i+=offset)
	{
	_mm_prefetch(((char*)(src+i)),3);
	_mm_prefetch(((char*)(src+i+stride)),3);
	_mm_prefetch(((char*)(src+i+stride*2)),3);
	_mm_prefetch(((char*)(src+i+stride*3)),3);		
	}
	*/

	if((dx+dy)==0)// mv_x=0 and mv_y=0
	{

#ifdef H264_ENABLE_ASM
		__asm
		{
			mov  eax, src;
			mov  ebx, stride;
			mov  ecx, block;
			mov  edx, height;
LOOP_4:
			movq mm0, [eax];
			movq mm1, [eax+ebx];
			movq [ecx], mm0;
			movq [ecx+16], mm1;
			lea  eax, [eax+2*ebx];
			movq mm2, [eax];
			movq mm3, [eax+ebx];
			movq [ecx+32], mm2;
			movq [ecx+48], mm3;
			lea  eax, [eax+2*ebx];
			add  ecx, 64;
			sub  edx, 4;
			jg   LOOP_4;
		}
#else 
		if((cpu_type == CPU_LEVEL_MMX) ||(cpu_type == CPU_LEVEL_SSE) || (cpu_type == CPU_LEVEL_SSE2) || (cpu_type == CPU_LEVEL_SSE3))
		{
			imgpel *pblock = &block[0][0];
			for(int i = height; i > 0; i-=4)
			{
				//*((unsigned long long*)&block[i][0]) = *((unsigned long long*)src);
				*(__m64*)(pblock+0*LUMA_BLOCK_SIZE) = *((__m64*)&src[0*stride]);
				*(__m64*)(pblock+1*LUMA_BLOCK_SIZE) = *((__m64*)&src[1*stride]);
				*(__m64*)(pblock+2*LUMA_BLOCK_SIZE) = *((__m64*)&src[2*stride]);
				*(__m64*)(pblock+3*LUMA_BLOCK_SIZE) = *((__m64*)&src[3*stride]);
				src    += 4*stride;
				pblock += 4*LUMA_BLOCK_SIZE;
			}
		}
		else
		{
			for(int i = 0; i < height; i+=4) // loop unrolling for 8x16 and 8x8 and 8x4
			{
#ifdef _NO_INT64_
			memcpy((unsigned char *)&block[i]  , src, 8);
			memcpy((unsigned char *)&block[i+1], src+stride, 8);
			src += 2*stride;
			memcpy((unsigned char *)&block[i+2], src, 8);
			memcpy((unsigned char *)&block[i+3], src+stride, 8);
			src += 2*stride;
#else
				*((int64*)&block[i]) = *((int64*)(src));
				src += stride;
				*((int64*)&block[i+1]) = *((int64*)(src));
				src += stride;
				*((int64*)&block[i+2]) = *((int64*)(src));
				src += stride;
				*((int64*)&block[i+3]) = *((int64*)(src));
				src += stride;
#endif
			}
		}
#endif
	} else {
		get_block_8xh_fp[(dx<<2)+dy](src, block, stride, height);
	}

#if defined (_COLLECT_PIC_)
	if ( IMGPAR smart_dec & SMART_DEC_BITBYBIT ) {
		LeaveCriticalSection( &(IMGPAR stream_global->crit_dyna_expand) );
	}
#endif
}

void  get_block_16xh_full PARGS7(StorablePicture *p, int base_x_pos, int base_y_pos, int mv_x, int mv_y, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int height)
{
	byte *src;
	const int stride = p->Y_stride;
	int dx     = mv_x&3;
	int dy     = mv_y&3;
	int x_pos  = base_x_pos + (mv_x>>2);
	int y_pos  = base_y_pos + (mv_y>>2);
	x_pos = __fast_iclip(clip_min_x,clip_max_x,x_pos);

	int jj;
	// New code, y_pos attached to the boundary
#if defined (_COLLECT_PIC_)
	if ( IMGPAR smart_dec & SMART_DEC_BITBYBIT ) {
		EnterCriticalSection( &(IMGPAR stream_global->crit_dyna_expand) );
	}
#endif
	if(y_pos<2)
	{    // We need to worry about Frame/Field difference in extended area
		if(y_pos<(-(height+2))) 	// specific to 16xh blocks
			y_pos = (-(height+2));
		src = p->imgY + x_pos-2;
		for(jj=y_pos-2;jj<0;jj++)
			memcpy(src+jj*p->Y_stride, src, 2+16+3); // specific to 16xh blocks
	}
	else if (y_pos>clip_max_y-(height+2))	// specific to 16xh blocks
	{
		if(y_pos>clip_max_y+2)
			y_pos = clip_max_y+2;
		src = p->imgY + clip_max_y*p->Y_stride + x_pos-2;
		for(jj=1;jj<=y_pos-clip_max_y+height+2;jj++)	// specific to 16xh blocks
			memcpy(src+jj*p->Y_stride, src, 2+16+3); // specific to 16xh blocks
	}

	src = p->imgY +y_pos*stride+x_pos;
	/*
	const int range = (stride<<4);
	for(i=stride ; i< range; i+=stride)
	{
	_mm_prefetch(((char*)(src+i)),3);
	}
	*/
	if((dx+dy)==0)// mv_x=0 and mv_y=0
	{			
#ifdef H264_ENABLE_ASM
		__asm
		{
			mov  eax, src;
			mov  ebx, stride;
			mov  ecx, block;
			mov  edx, height;
LOOP_8:
			movq mm0, [eax];
			movq mm1, [eax+8];
			movq mm2, [eax+ebx];
			movq mm3, [eax+ebx+8];
			movq [ecx], mm0;
			movq [ecx+8], mm1;
			movq [ecx+16], mm2;
			movq [ecx+24], mm3;
			lea  eax, [eax+2*ebx];
			movq mm4, [eax];
			movq mm5, [eax+8];
			movq mm6, [eax+ebx];
			movq mm7, [eax+ebx+8];
			movq [ecx+32], mm4;
			movq [ecx+40], mm5;
			movq [ecx+48], mm6;
			movq [ecx+56], mm7;
			lea  eax, [eax+2*ebx];
			movq mm0, [eax];
			movq mm1, [eax+8];
			movq mm2, [eax+ebx];
			movq mm3, [eax+ebx+8];
			movq [ecx+64], mm0;
			movq [ecx+72], mm1;
			movq [ecx+80], mm2;
			movq [ecx+88], mm3;
			lea  eax, [eax+2*ebx];
			movq mm4, [eax];
			movq mm5, [eax+8];
			movq mm6, [eax+ebx];
			movq mm7, [eax+ebx+8];
			movq [ecx+96], mm4;
			movq [ecx+104], mm5;
			movq [ecx+112], mm6;
			movq [ecx+120], mm7;
			lea  eax, [eax+2*ebx];
			add  ecx, 128;
			sub  edx, 8;
			jg   LOOP_8;
		}
#else 
		if((cpu_type == CPU_LEVEL_MMX) ||(cpu_type == CPU_LEVEL_SSE) || (cpu_type == CPU_LEVEL_SSE2) || (cpu_type == CPU_LEVEL_SSE3))
		{
			imgpel *pblock = &block[0][0];
			for(int i = height; i > 0; i-=8)
			{
			//*((unsigned long long*)&block[i][0]) = *((unsigned long long*)src);
				*(__m64*)(pblock+0*LUMA_BLOCK_SIZE)   = *((__m64*)&src[0*stride]);
				*(__m64*)(pblock+0*LUMA_BLOCK_SIZE+8) = *((__m64*)&src[0*stride+8]);
				*(__m64*)(pblock+1*LUMA_BLOCK_SIZE)   = *((__m64*)&src[1*stride]);
				*(__m64*)(pblock+1*LUMA_BLOCK_SIZE+8) = *((__m64*)&src[1*stride+8]);
				*(__m64*)(pblock+2*LUMA_BLOCK_SIZE)   = *((__m64*)&src[2*stride]);
				*(__m64*)(pblock+2*LUMA_BLOCK_SIZE+8) = *((__m64*)&src[2*stride+8]);
				*(__m64*)(pblock+3*LUMA_BLOCK_SIZE)   = *((__m64*)&src[3*stride]);
				*(__m64*)(pblock+3*LUMA_BLOCK_SIZE+8) = *((__m64*)&src[3*stride+8]);
				*(__m64*)(pblock+4*LUMA_BLOCK_SIZE)   = *((__m64*)&src[4*stride]);
				*(__m64*)(pblock+4*LUMA_BLOCK_SIZE+8) = *((__m64*)&src[4*stride+8]);
				*(__m64*)(pblock+5*LUMA_BLOCK_SIZE)   = *((__m64*)&src[5*stride]);
				*(__m64*)(pblock+5*LUMA_BLOCK_SIZE+8) = *((__m64*)&src[5*stride+8]);
				*(__m64*)(pblock+6*LUMA_BLOCK_SIZE)   = *((__m64*)&src[6*stride]);
				*(__m64*)(pblock+6*LUMA_BLOCK_SIZE+8) = *((__m64*)&src[6*stride+8]);
				*(__m64*)(pblock+7*LUMA_BLOCK_SIZE)   = *((__m64*)&src[7*stride]);
				*(__m64*)(pblock+7*LUMA_BLOCK_SIZE+8) = *((__m64*)&src[7*stride+8]);
				src    += 8*stride;
				pblock += 8*LUMA_BLOCK_SIZE;
			}
		}
		else
		{
			for(int i = 0; i < height; i+=8) // loop unrolling for 16x16 and 16x8
			{
#ifdef _NO_INT64_
			memcpy((unsigned char *)&block[i  ][0], src, 16);
			memcpy((unsigned char *)&block[i+1][0], src+stride, 16);
			src += 2*stride;
			memcpy((unsigned char *)&block[i+2][0], src, 16);
			memcpy((unsigned char *)&block[i+3][0], src+stride, 16);
			src += 2*stride;
			memcpy((unsigned char *)&block[i+4][0], src, 16);
			memcpy((unsigned char *)&block[i+5][0], src+stride, 16);
			src += 2*stride;
			memcpy((unsigned char *)&block[i+6][0], src, 16);
			memcpy((unsigned char *)&block[i+7][0], src+stride, 16);
			src += 2*stride;
#else
				*((int64*)&block[i][0]) = *((int64*)(src));
				*((int64*)&block[i][8]) = *((int64*)(src+8));
				src += stride;
				*((int64*)&block[i+1][0]) = *((int64*)(src));
				*((int64*)&block[i+1][8]) = *((int64*)(src+8));
				src += stride;
				*((int64*)&block[i+2][0]) = *((int64*)(src));
				*((int64*)&block[i+2][8]) = *((int64*)(src+8));
				src += stride;
				*((int64*)&block[i+3][0]) = *((int64*)(src));
				*((int64*)&block[i+3][8]) = *((int64*)(src+8));
				src += stride;
				*((int64*)&block[i+4][0]) = *((int64*)(src));
				*((int64*)&block[i+4][8]) = *((int64*)(src+8));
				src += stride;
				*((int64*)&block[i+5][0]) = *((int64*)(src));
				*((int64*)&block[i+5][8]) = *((int64*)(src+8));
				src += stride;
				*((int64*)&block[i+6][0]) = *((int64*)(src));
				*((int64*)&block[i+6][8]) = *((int64*)(src+8));
				src += stride;
				*((int64*)&block[i+7][0]) = *((int64*)(src));
				*((int64*)&block[i+7][8]) = *((int64*)(src+8));
				src += stride;
#endif
			}	
		}
#endif
	} else {
		get_block_16xh_fp[(dx<<2)+dy](src, block, stride, height);
	}
#if defined (_COLLECT_PIC_)
	if ( IMGPAR smart_dec & SMART_DEC_BITBYBIT ) {
		LeaveCriticalSection( &(IMGPAR stream_global->crit_dyna_expand) );
	}
#endif
}

//The C code of get_block_4xh_pxx
void  get_block_4xh_p00_c(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	for(int i = 0; i < height; i++)
	{
		*((unsigned long*)&block[i]) = *((unsigned long*)(pSrc));
		pSrc += stride;
	}
}
void  get_block_4xh_p01_c(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int i, j;
	int result;
	byte *src;
	const int w1 = stride;	
	const int w2 = stride + stride;
	const int w3 = (stride<<1) + stride;
	src = pSrc;	
	for (j = 0; j < height; j++) 
	{	    
		for (i = 0; i < 4; i++) 
		{
			result = q_interpol(src[i-w2], src[i-w1], src[i], src[i+w1], src[i+w2], src[i+w3]);
			block[j][i] = __fast_iclip0_255(((result+16)>>5)); // HP restriction
			block[j][i] = ((block[j][i] + src[i] +1 )>>1);			
		}		
		src += w1;
	}
}

void  get_block_4xh_p02_c(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int i, j;
	int result;
	byte *src;
	const int w1 = stride;	
	const int w2 = stride + stride;
	const int w3 = (stride<<1) + stride;
	src = pSrc;	
	for (j = 0; j < height; j++) 
	{
		for (i = 0; i < 4; i++) 
		{
			result = q_interpol(src[i-w2], src[i-w1], src[i], src[i+w1], src[i+w2], src[i+w3]);
			block[j][i] = __fast_iclip0_255(((result+16)>>5)); // HP restriction
		}
		src += w1;
	}
}
void  get_block_4xh_p03_c(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int i, j;
	int result;
	byte *src;
	const int w1 = stride;	
	const int w2 = stride + stride;
	const int w3 = (stride<<1) + stride;
	src = pSrc;

	for (j = 0; j < height; j++) 
	{
		for (i = 0; i < 4; i++) 
		{
			result = q_interpol(src[i-w2], src[i-w1], src[i], src[i+w1], src[i+w2], src[i+w3]);
			block[j][i] = __fast_iclip0_255(((result+16)>>5)); // HP restriction
		}
		src += w1;
	}

	src = pSrc+w1;
	for (j = 0; j < height; j++)
	{
		for (i = 0; i < 4; i++)
			block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		src += w1;
	}

}
void  get_block_4xh_p10_c(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int i, j; 
	int result;
	byte *src;
	const int w1 = stride;
	src = pSrc;

	for (j = 0; j < height; j++) 
	{				
		for (i = 0; i < 4; i++) 
		{
			result = q_interpol(src[i-2], src[i-1], src[i], src[i+1], src[i+2], src[i+3]);
			block[j][i] = __fast_iclip0_255((result+16)>>5); // HP restriction
			block[j][i] = ((block[j][i] + src[i] +1 )>>1);			
		}
		src += w1;
	}
}
void  get_block_4xh_p11_c(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int i, j;
	int result;
	byte *src;
	const int w1 = stride;	
	const int w2 = stride + stride;
	const int w3 = (stride<<1) + stride;

	src = pSrc;
	for (j = 0; j < height; j++) 
	{					
		for (i = 0; i < 4; i++) 
		{

			result = q_interpol(src[i-2], src[i-1], src[i], src[i+1], src[i+2], src[i+3]);
			block[j][i] = __fast_iclip0_255(((result+16)>>5)); // HP restriction
			result = q_interpol(src[i-w2], src[i-w1], src[i], src[i+w1], src[i+w2], src[i+w3]); 	
			block[j][i] = (block[j][i] + __fast_iclip0_255(((result+16)>>5)) +1 ) / 2; // HP restriction			
		}
		src += w1;
	}
}
void  get_block_4xh_p12_c(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	byte *src;			
	int i, j;
	int result;
	int tmp_res[8][9];
	const int w1 = stride;	
	const int w2 = stride + stride;
	const int w3 = (stride<<1) + stride;
	src = pSrc;

	for (j = 0; j < height; j++) 
	{			
		for (i = -2; i < 7; i++)
			tmp_res[j][i+2] = q_interpol(src[i-w2],src[i-w1], src[i], src[i+w1], src[i+w2], src[i+w3]);  	
		src += w1;
	}	
	for (j = 0; j < height; j++) 
	{
		for (i = 0; i < 4; i++) 
		{
			result = q_interpol(tmp_res[j][i], tmp_res[j][i+1], tmp_res[j][i+2], tmp_res[j][i+3], tmp_res[j][i+4], tmp_res[j][i+5]);  
			block[j][i] = __fast_iclip0_255(((result+512)>>10)); // HP restriction
			block[j][i] = (block[j][i] + __fast_iclip0_255(((tmp_res[j][i+2]+16)>>5))+1)/2; // HP restriction				
		}
	}
}
void  get_block_4xh_p13_c(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int i, j;
	int result;
	byte *src;
	const int w1 = stride;
	const int w2 = stride + stride;
	const int w3 = (stride<<1) + stride;

	src = pSrc+w1;

	for (j = 0; j < height; j++) 
	{					
		for (i = 0; i < 4; i++) 
		{

			result = q_interpol(src[i-2], src[i-1], src[i], src[i+1], src[i+2], src[i+3]);
			block[j][i] = __fast_iclip0_255(((result+16)>>5)); // HP restriction
		}
		src += w1;
	}

	src = pSrc;
	for (j = 0; j < height; j++) 
	{				
		for (i = 0; i < 4; i++) 
		{		
			result = q_interpol(src[i-w2], src[i-w1], src[i], src[i+w1], src[i+w2], src[i+w3]); 	
			block[j][i] = (block[j][i] + __fast_iclip0_255(((result+16)>>5)) +1 ) / 2; // HP restriction
		}
		src += w1;
	}
}
void  get_block_4xh_p20_c(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int i, j;
	int result;
	byte *src;
	const int w1 = stride;
	src = pSrc;

	for (j = 0; j < height; j++) 
	{				
		for (i = 0; i < 4; i++) 
		{
			result = q_interpol(src[i-2], src[i-1], src[i], src[i+1], src[i+2], src[i+3]);
			block[j][i] = __fast_iclip0_255((result+16)>>5); // HP restriction
		}
		src += w1;
	}
}
void  get_block_4xh_p21_c(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	byte *src;
	int i, j;
	int result;
	int tmp_res[13][4];
	const int w1 = stride;	

	src = pSrc-(2*w1);

	for (j = -2; j < height+3; j++) 
	{				
		for (i = 0; i < 4; i++)
			tmp_res[j+2][i] = q_interpol(src[i-2], src[i-1], src[i], src[i+1], src[i+2], src[i+3]);						
		src += w1;
	}	

	for (j = 0; j < height; j++) 
	{
		for (i = 0; i < 4; i++) 
		{
			result = q_interpol(tmp_res[j][i], tmp_res[j+1][i], tmp_res[j+2][i], tmp_res[j+3][i], tmp_res[j+4][i], tmp_res[j+5][i]); 
			block[j][i] = __fast_iclip0_255(((result+512)>>10)); // HP restriction
			block[j][i] = (block[j][i] + __fast_iclip0_255(((tmp_res[j+2][i]+16)>>5)) +1 )/2; // HP restriction
		}	 
	}	 
}
void  get_block_4xh_p22_c(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	byte *src;	
	int i, j;
	int result;
	int tmp_res[13][4];
	const int w1 = stride;	

	src = pSrc - (2*w1);
	for (j = -2; j < height+3; j++) 
	{				
		for (i = 0; i < 4; i++)
			tmp_res[j+2][i] = q_interpol(src[i-2], src[i-1], src[i], src[i+1], src[i+2], src[i+3]);								
		src += w1;
	}	
	for (j = 0; j < height; j++) 
	{
		for (i = 0; i < 4; i++) 
		{
			result = q_interpol(tmp_res[j][i], tmp_res[j+1][i], tmp_res[j+2][i], tmp_res[j+3][i], tmp_res[j+4][i], tmp_res[j+5][i]); 
			block[j][i] = __fast_iclip0_255(((result+512)>>10)); // HP restriction
		}	 
	}
}
void  get_block_4xh_p23_c(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{		
	int i, j;
	int result;
	int tmp_res[13][4];
	byte *src;
	const int w1 = stride;	

	src = pSrc -(2*w1);
	for (j = -2; j < height+3; j++) 
	{				
		for (i = 0; i < 4; i++)
			tmp_res[j+2][i] = q_interpol(src[i-2], src[i-1], src[i], src[i+1], src[i+2], src[i+3]);								
		src += w1;
	}	

	for (j = 0; j < height; j++) 
	{
		for (i = 0; i < 4; i++) 
		{
			result = q_interpol(tmp_res[j][i], tmp_res[j+1][i], tmp_res[j+2][i], tmp_res[j+3][i], tmp_res[j+4][i], tmp_res[j+5][i]); 
			block[j][i] = __fast_iclip0_255(((result+512)>>10)); // HP restriction
			block[j][i] = (block[j][i] + __fast_iclip0_255(((tmp_res[j+3][i]+16)>>5)) +1 )/2; // HP restriction			
		}	 
	}
}
void  get_block_4xh_p30_c(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int i, j; 
	int result;
	byte *src;
	const int w1 = stride;

	src = pSrc;
	for (j = 0; j < height; j++) 
	{				
		for (i = 0; i < 4; i++) 
		{
			result = q_interpol(src[i-2], src[i-1], src[i], src[i+1], src[i+2], src[i+3]);
			block[j][i] = __fast_iclip0_255((result+16)>>5); // HP restriction
			block[j][i] = ((block[j][i] + src[i+1] +1 )>>1);			
		}
		src += w1;
	}
}
void  get_block_4xh_p31_c(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int i, j;
	int result;
	byte *src;
	const int w1 = stride;	
	const int w2 = stride + stride;
	const int w3 = (stride<<1) + stride;

	src = pSrc;
	for (j = 0; j < height; j++) 
	{					
		for (i = 0; i < 4; i++) 
		{

			result = q_interpol(src[i-2], src[i-1], src[i], src[i+1], src[i+2], src[i+3]);
			block[j][i] = __fast_iclip0_255(((result+16)>>5)); // HP restriction
		}
		src += w1;
	}				

	src = pSrc + 1;	
	for (j = 0; j < height; j++) 
	{				
		for (i = 0; i < 4; i++) 
		{		
			result = q_interpol(src[i-w2], src[i-w1], src[i], src[i+w1], src[i+w2], src[i+w3]); 	
			block[j][i] = (block[j][i] + __fast_iclip0_255(((result+16)>>5)) +1 ) / 2; // HP restriction
		}
		src += w1;
	}
}
void  get_block_4xh_p32_c(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	byte *src;				
	int i, j;
	int result;
	int tmp_res[8][9];
	const int w1 = stride;		
	const int w2 = stride + stride;
	const int w3 = (stride<<1) + stride;

	src = pSrc;
	for (j = 0; j < height; j++) 
	{			
		for (i = -2; i < 7; i++)
			tmp_res[j][i+2] = q_interpol(src[i-w2],src[i-w1], src[i], src[i+w1], src[i+w2], src[i+w3]);  	
		src += w1;
	}	
	for (j = 0; j < height; j++) 
	{
		for (i = 0; i < 4; i++) 
		{
			result = q_interpol(tmp_res[j][i], tmp_res[j][i+1], tmp_res[j][i+2], tmp_res[j][i+3], tmp_res[j][i+4], tmp_res[j][i+5]);  
			block[j][i] = __fast_iclip0_255(((result+512)>>10)); // HP restriction
			block[j][i] = (block[j][i] + __fast_iclip0_255(((tmp_res[j][i+3]+16)>>5))+1)/2; // HP restriction			
		}
	}
}
void  get_block_4xh_p33_c(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	byte *src;
	int i, j;
	int result;
	const int w1 = stride;		
	const int w2 = stride + stride;
	const int w3 = (stride<<1) + stride;

	src = pSrc + w1;
	for (j = 0; j < height; j++) 
	{					
		for (i = 0; i < 4; i++) 
		{

			result = q_interpol(src[i-2], src[i-1], src[i], src[i+1], src[i+2], src[i+3]);
			block[j][i] = __fast_iclip0_255(((result+16)>>5)); // HP restriction
		}
		src += w1;
	}				

	src = pSrc + 1;

	for (j = 0; j < height; j++) 
	{				
		for (i = 0; i < 4; i++) 
		{		
			result = q_interpol(src[i-w2], src[i-w1], src[i], src[i+w1], src[i+w2], src[i+w3]); 	
			block[j][i] = (block[j][i] + __fast_iclip0_255(((result+16)>>5)) +1 ) / 2; // HP restriction
		}
		src += w1;
	}	
}

//The C code of get_block_8xh_pxx
void  get_block_8xh_p00_c(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int i;
	for(i = 0; i < height; i+=2)
	{
		*(long *)&block[i  ][0] = *(long *)(pSrc         );
		*(long *)&block[i  ][4] = *(long *)(pSrc       +4);
		*(long *)&block[i+1][0] = *(long *)(pSrc+stride  );
		*(long *)&block[i+1][4] = *(long *)(pSrc+stride+4);
		pSrc += 2*stride;
	}
}

void  get_block_8xh_p01_c(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int i, j;
	int result;
	byte *src;
	const int w1 = stride;	
	const int w2 = stride + stride;
	const int w3 = (stride<<1) + stride;
	src = pSrc;	
	for (j = 0; j < height; j++) 
	{	    
		for (i = 0; i < 8; i++) 
		{
			result = q_interpol(src[i-w2], src[i-w1], src[i], src[i+w1], src[i+w2], src[i+w3]);
			block[j][i] = __fast_iclip0_255(((result+16)>>5)); // HP restriction
			block[j][i] = ((block[j][i] + src[i] +1 )>>1);			
		}		
		src += w1;
	}
}

void  get_block_8xh_p02_c(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int i, j;
	int result;
	byte *src;
	const int w1 = stride;	
	const int w2 = stride + stride;
	const int w3 = (stride<<1) + stride;
	src = pSrc;	
	for (j = 0; j < height; j++) 
	{
		for (i = 0; i < 8; i++) 
		{
			result = q_interpol(src[i-w2], src[i-w1], src[i], src[i+w1], src[i+w2], src[i+w3]);
			block[j][i] = __fast_iclip0_255(((result+16)>>5)); // HP restriction
		}
		src += w1;
	}
}

void  get_block_8xh_p03_c(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int i, j;
	int result;
	byte *src;
	const int w1 = stride;	
	const int w2 = stride + stride;
	const int w3 = (stride<<1) + stride;
	src = pSrc;

	for (j = 0; j < height; j++) 
	{
		for (i = 0; i < 8; i++) 
		{
			result = q_interpol(src[i-w2], src[i-w1], src[i], src[i+w1], src[i+w2], src[i+w3]);
			block[j][i] = __fast_iclip0_255(((result+16)>>5)); // HP restriction
		}
		src += w1;
	}

	src = pSrc+w1;
	for (j = 0; j < height; j++)
	{
		for (i = 0; i < 8; i++)
			block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		src += w1;
	}
}

void  get_block_8xh_p10_c(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int i, j;
	int result;
	byte *src;
	const int w1 = stride;
	src = pSrc;

	for (j = 0; j < height; j++) 
	{				
		for (i = 0; i < 8; i++) 
		{
			result = q_interpol(src[i-2], src[i-1], src[i], src[i+1], src[i+2], src[i+3]);
			block[j][i] = __fast_iclip0_255((result+16)>>5); // HP restriction
			block[j][i] = ((block[j][i] + src[i] +1 )>>1);			
		}
		src += w1;
	}
}

void  get_block_8xh_p11_c(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int i, j;
	int result;
	byte *src;
	const int w1 = stride;	
	const int w2 = stride + stride;
	const int w3 = (stride<<1) + stride;

	src = pSrc;
	for (j = 0; j < height; j++) 
	{					
		for (i = 0; i < 8; i++) 
		{

			result = q_interpol(src[i-2], src[i-1], src[i], src[i+1], src[i+2], src[i+3]);
			block[j][i] = __fast_iclip0_255(((result+16)>>5)); // HP restriction
			result = q_interpol(src[i-w2], src[i-w1], src[i], src[i+w1], src[i+w2], src[i+w3]); 	
			block[j][i] = (block[j][i] + __fast_iclip0_255(((result+16)>>5)) +1 ) / 2; // HP restriction			
		}
		src += w1;
	}
}

void  get_block_8xh_p12_c(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	byte *src;	
	int i, j;
	int result;
	int tmp_res[16][13];
	const int w1 = stride;	
	const int w2 = stride + stride;
	const int w3 = (stride<<1) + stride;
	src = pSrc;

	for (j = 0; j < height; j++) 
	{			
		for (i = -2; i < 11; i++)
			tmp_res[j][i+2] = q_interpol(src[i-w2],src[i-w1], src[i], src[i+w1], src[i+w2], src[i+w3]);  	
		src += w1;
	}	
	for (j = 0; j < height; j++) 
	{
		for (i = 0; i < 8; i++) 
		{
			result = q_interpol(tmp_res[j][i], tmp_res[j][i+1], tmp_res[j][i+2], tmp_res[j][i+3], tmp_res[j][i+4], tmp_res[j][i+5]);  
			block[j][i] = __fast_iclip0_255(((result+512)>>10)); // HP restriction
			block[j][i] = (block[j][i] + __fast_iclip0_255(((tmp_res[j][i+2]+16)>>5))+1)/2; // HP restriction				
		}
	}
}

void  get_block_8xh_p13_c(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int i, j;
	int result;
	byte *src;
	const int w1 = stride;	
	const int w2 = stride + stride;
	const int w3 = (stride<<1) + stride;

	src = pSrc+w1;

	for (j = 0; j < height; j++) 
	{					
		for (i = 0; i < 8; i++) 
		{

			result = q_interpol(src[i-2], src[i-1], src[i], src[i+1], src[i+2], src[i+3]);
			block[j][i] = __fast_iclip0_255(((result+16)>>5)); // HP restriction
		}
		src += w1;
	}

	src = pSrc;	
	for (j = 0; j < height; j++) 
	{				
		for (i = 0; i < 8; i++) 
		{		
			result = q_interpol(src[i-w2], src[i-w1], src[i], src[i+w1], src[i+w2], src[i+w3]); 	
			block[j][i] = (block[j][i] + __fast_iclip0_255(((result+16)>>5)) +1 ) / 2; // HP restriction
		}
		src += w1;
	}
}

void  get_block_8xh_p20_c(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int i, j;
	int result;
	byte *src;
	const int w1 = stride;
	src = pSrc;

	for (j = 0; j < height; j++) 
	{				
		for (i = 0; i < 8; i++) 
		{
			result = q_interpol(src[i-2], src[i-1], src[i], src[i+1], src[i+2], src[i+3]);
			block[j][i] = __fast_iclip0_255((result+16)>>5); // HP restriction
		}
		src += w1;
	}
}

void  get_block_8xh_p21_c(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	byte *src;
	int i, j;
	int result;
	int tmp_res[21][8];
	const int w1 = stride;	

	src = pSrc-(2*w1);

	for (j = -2; j < height+3; j++) 
	{				
		for (i = 0; i < 8; i++)
			tmp_res[j+2][i] = q_interpol(src[i-2], src[i-1], src[i], src[i+1], src[i+2], src[i+3]);						
		src += w1;
	}	

	for (j = 0; j < height; j++) 
	{
		for (i = 0; i < 8; i++) 
		{
			result = q_interpol(tmp_res[j][i], tmp_res[j+1][i], tmp_res[j+2][i], tmp_res[j+3][i], tmp_res[j+4][i], tmp_res[j+5][i]); 
			block[j][i] = __fast_iclip0_255(((result+512)>>10)); // HP restriction
			block[j][i] = (block[j][i] + __fast_iclip0_255(((tmp_res[j+2][i]+16)>>5)) +1 )/2; // HP restriction
		}	 
	}	 
}

void  get_block_8xh_p22_c(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	byte *src;	
	int i, j;
	int result;
	int tmp_res[21][8];
	const int w1 = stride;	

	src = pSrc - (2*w1);
	for (j = -2; j < height+3; j++) 
	{				
		for (i = 0; i < 8; i++)
			tmp_res[j+2][i] = q_interpol(src[i-2], src[i-1], src[i], src[i+1], src[i+2], src[i+3]);								
		src += w1;
	}	
	for (j = 0; j < height; j++) 
	{
		for (i = 0; i < 8; i++) 
		{
			result = q_interpol(tmp_res[j][i], tmp_res[j+1][i], tmp_res[j+2][i], tmp_res[j+3][i], tmp_res[j+4][i], tmp_res[j+5][i]); 
			block[j][i] = __fast_iclip0_255(((result+512)>>10)); // HP restriction
		}	 
	}
}

void  get_block_8xh_p23_c(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int i, j;
	int result;
	int tmp_res[21][8];
	byte *src;
	const int w1 = stride;	

	src = pSrc -(2*w1);
	for (j = -2; j < height+3; j++) 
	{				
		for (i = 0; i < 8; i++)
			tmp_res[j+2][i] = q_interpol(src[i-2], src[i-1], src[i], src[i+1], src[i+2], src[i+3]);								
		src += w1;
	}	

	for (j = 0; j < height; j++) 
	{
		for (i = 0; i < 8; i++) 
		{
			result = q_interpol(tmp_res[j][i], tmp_res[j+1][i], tmp_res[j+2][i], tmp_res[j+3][i], tmp_res[j+4][i], tmp_res[j+5][i]); 
			block[j][i] = __fast_iclip0_255(((result+512)>>10)); // HP restriction
			block[j][i] = (block[j][i] + __fast_iclip0_255(((tmp_res[j+3][i]+16)>>5)) +1 )/2; // HP restriction			
		}	 
	}
}

void  get_block_8xh_p30_c(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int i, j;
	int result;
	byte *src;
	const int w1 = stride;

	src = pSrc;
	for (j = 0; j < height; j++) 
	{				
		for (i = 0; i < 8; i++) 
		{
			result = q_interpol(src[i-2], src[i-1], src[i], src[i+1], src[i+2], src[i+3]);
			block[j][i] = __fast_iclip0_255((result+16)>>5); // HP restriction
			block[j][i] = ((block[j][i] + src[i+1] +1 )>>1);			
		}
		src += w1;
	}
}

void  get_block_8xh_p31_c(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int i, j;
	int result;
	byte *src;
	const int w1 = stride;	
	const int w2 = stride + stride;
	const int w3 = (stride<<1) + stride;

	src = pSrc;
	for (j = 0; j < height; j++) 
	{					
		for (i = 0; i < 8; i++) 
		{

			result = q_interpol(src[i-2], src[i-1], src[i], src[i+1], src[i+2], src[i+3]);
			block[j][i] = __fast_iclip0_255(((result+16)>>5)); // HP restriction
		}
		src += w1;
	}				

	src = pSrc + 1;	
	for (j = 0; j < height; j++) 
	{				
		for (i = 0; i < 8; i++) 
		{		
			result = q_interpol(src[i-w2], src[i-w1], src[i], src[i+w1], src[i+w2], src[i+w3]); 	
			block[j][i] = (block[j][i] + __fast_iclip0_255(((result+16)>>5)) +1 ) / 2; // HP restriction
		}
		src += w1;
	}
}

void  get_block_8xh_p32_c(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	byte *src;	
	int i, j;
	int result;
	int tmp_res[16][13];
	const int w1 = stride;		
	const int w2 = stride + stride;
	const int w3 = (stride<<1) + stride;

	src = pSrc;
	for (j = 0; j < height; j++) 
	{			
		for (i = -2; i < 11; i++)
			tmp_res[j][i+2] = q_interpol(src[i-w2],src[i-w1], src[i], src[i+w1], src[i+w2], src[i+w3]);  	
		src += w1;
	}	
	for (j = 0; j < height; j++) 
	{
		for (i = 0; i < 8; i++) 
		{
			result = q_interpol(tmp_res[j][i], tmp_res[j][i+1], tmp_res[j][i+2], tmp_res[j][i+3], tmp_res[j][i+4], tmp_res[j][i+5]);  
			block[j][i] = __fast_iclip0_255(((result+512)>>10)); // HP restriction
			block[j][i] = (block[j][i] + __fast_iclip0_255(((tmp_res[j][i+3]+16)>>5))+1)/2; // HP restriction			
		}
	}
}

void  get_block_8xh_p33_c(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	byte *src;
	int i, j;
	int result;
	const int w1 = stride;		
	const int w2 = stride + stride;
	const int w3 = (stride<<1) + stride;

	src = pSrc + w1;
	for (j = 0; j < height; j++) 
	{					
		for (i = 0; i < 8; i++) 
		{

			result = q_interpol(src[i-2], src[i-1], src[i], src[i+1], src[i+2], src[i+3]);
			block[j][i] = __fast_iclip0_255(((result+16)>>5)); // HP restriction
		}
		src += w1;
	}				

	src = pSrc + 1;

	for (j = 0; j < height; j++) 
	{				
		for (i = 0; i < 8; i++) 
		{		
			result = q_interpol(src[i-w2], src[i-w1], src[i], src[i+w1], src[i+w2], src[i+w3]); 	
			block[j][i] = (block[j][i] + __fast_iclip0_255(((result+16)>>5)) +1 ) / 2; // HP restriction
		}
		src += w1;
	}
}

//The C code of get_block_16xh_pxx

void  get_block_16xh_p00_c(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	for(int i = 0; i < height; i++)
	{
#ifdef _NO_INT64_
		memcpy((unsigned char *)&block[i][0], pSrc, 16);
		pSrc += stride;
#else
		*((int64*)&block[i][0]) = *((int64*)pSrc);
		*((int64*)&block[i][8]) = *((int64*)(pSrc+8));
		pSrc += stride;
#endif
	}
}

void  get_block_16xh_p01_c(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int i, j;
	int result;
	byte *src;
	const int w1 = stride;	
	const int w2 = stride + stride;
	const int w3 = (stride<<1) + stride;
	src = pSrc;	
	for (j = 0; j < height; j++) 
	{	    
		for (i = 0; i < 16; i++) 
		{
			result = q_interpol(src[i-w2], src[i-w1], src[i], src[i+w1], src[i+w2], src[i+w3]);
			block[j][i] = __fast_iclip0_255(((result+16)>>5)); // HP restriction
			block[j][i] = ((block[j][i] + src[i] +1 )>>1);			
		}		
		src += w1;
	}
}

void  get_block_16xh_p02_c(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int i, j;
	int result;
	byte *src;
	const int w1 = stride;	
	const int w2 = stride + stride;
	const int w3 = (stride<<1) + stride;
	src = pSrc;	
	for (j = 0; j < height; j++) 
	{
		for (i = 0; i < 16; i++) 
		{
			result = q_interpol(src[i-w2], src[i-w1], src[i], src[i+w1], src[i+w2], src[i+w3]);
			block[j][i] = __fast_iclip0_255(((result+16)>>5)); // HP restriction
		}
		src += w1;
	}
}

void  get_block_16xh_p03_c(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int i, j;
	int result;
	byte *src;
	const int w1 = stride;	
	const int w2 = stride + stride;
	const int w3 = (stride<<1) + stride;
	src = pSrc;

	for (j = 0; j < height; j++) 
	{
		for (i = 0; i < 16; i++) 
		{
			result = q_interpol(src[i-w2], src[i-w1], src[i], src[i+w1], src[i+w2], src[i+w3]);
			block[j][i] = __fast_iclip0_255(((result+16)>>5)); // HP restriction
		}
		src += w1;
	}

	src = pSrc+w1;
	for (j = 0; j < height; j++)
	{
		for (i = 0; i < 16; i++)
			block[j][i] = ((block[j][i] + src[i] +1 )>>1);
		src += w1;
	}
}

void  get_block_16xh_p10_c(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int i, j;
	int result;
	byte *src;
	const int w1 = stride;
	src = pSrc;

	for (j = 0; j < height; j++) 
	{				
		for (i = 0; i < 16; i++) 
		{
			result = q_interpol(src[i-2], src[i-1], src[i], src[i+1], src[i+2], src[i+3]);
			block[j][i] = __fast_iclip0_255((result+16)>>5); // HP restriction
			block[j][i] = ((block[j][i] + src[i] +1 )>>1);			
		}
		src += w1;
	}
}

void  get_block_16xh_p11_c(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int i, j;
	int result;
	byte *src;
	const int w1 = stride;	
	const int w2 = stride + stride;
	const int w3 = (stride<<1) + stride;

	src = pSrc;
	for (j = 0; j < height; j++) 
	{					
		for (i = 0; i < 16; i++) 
		{

			result = q_interpol(src[i-2], src[i-1], src[i], src[i+1], src[i+2], src[i+3]);
			block[j][i] = __fast_iclip0_255(((result+16)>>5)); // HP restriction
			result = q_interpol(src[i-w2], src[i-w1], src[i], src[i+w1], src[i+w2], src[i+w3]); 	
			block[j][i] = (block[j][i] + __fast_iclip0_255(((result+16)>>5)) +1 ) / 2; // HP restriction			
		}
		src += w1;
	}
}

void  get_block_16xh_p12_c(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	byte *src;			
	int i, j;
	int result;
	int tmp_res[16][21];
	const int w1 = stride;	
	const int w2 = stride + stride;
	const int w3 = (stride<<1) + stride;
	src = pSrc;

	for (j = 0; j < height; j++) 
	{			
		for (i = -2; i < 19; i++)
			tmp_res[j][i+2] = q_interpol(src[i-w2],src[i-w1], src[i], src[i+w1], src[i+w2], src[i+w3]);  	
		src += w1;
	}	
	for (j = 0; j < height; j++) 
	{
		for (i = 0; i < 16; i++) 
		{
			result = q_interpol(tmp_res[j][i], tmp_res[j][i+1], tmp_res[j][i+2], tmp_res[j][i+3], tmp_res[j][i+4], tmp_res[j][i+5]);  
			block[j][i] = __fast_iclip0_255(((result+512)>>10)); // HP restriction
			block[j][i] = (block[j][i] + __fast_iclip0_255(((tmp_res[j][i+2]+16)>>5))+1)/2; // HP restriction				
		}
	}
}

void  get_block_16xh_p13_c(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int i, j;
	int result;
	byte *src;
	const int w1 = stride;	
	const int w2 = stride + stride;
	const int w3 = (stride<<1) + stride;

	src = pSrc+w1;

	for (j = 0; j < height; j++) 
	{					
		for (i = 0; i < 16; i++) 
		{

			result = q_interpol(src[i-2], src[i-1], src[i], src[i+1], src[i+2], src[i+3]);
			block[j][i] = __fast_iclip0_255(((result+16)>>5)); // HP restriction
		}
		src += w1;
	}

	src = pSrc;	
	for (j = 0; j < height; j++) 
	{				
		for (i = 0; i < 16; i++) 
		{		
			result = q_interpol(src[i-w2], src[i-w1], src[i], src[i+w1], src[i+w2], src[i+w3]); 	
			block[j][i] = (block[j][i] + __fast_iclip0_255(((result+16)>>5)) +1 ) / 2; // HP restriction
		}
		src += w1;
	}	
}

void  get_block_16xh_p20_c(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int i, j;
	int result;
	byte *src;
	const int w1 = stride;
	src = pSrc;

	for (j = 0; j < height; j++) 
	{				
		for (i = 0; i < 16; i++) 
		{
			result = q_interpol(src[i-2], src[i-1], src[i], src[i+1], src[i+2], src[i+3]);
			block[j][i] = __fast_iclip0_255((result+16)>>5); // HP restriction
		}
		src += w1;
	}
}

void  get_block_16xh_p21_c(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	byte *src;	
	int i, j;
	int result;
	int tmp_res[21][16];
	const int w1 = stride;	

	src = pSrc-(2*w1);

	for (j = -2; j < height+3; j++) 
	{				
		for (i = 0; i < 16; i++)
			tmp_res[j+2][i] = q_interpol(src[i-2], src[i-1], src[i], src[i+1], src[i+2], src[i+3]);						
		src += w1;
	}	

	for (j = 0; j < height; j++) 
	{
		for (i = 0; i < 16; i++) 
		{
			result = q_interpol(tmp_res[j][i], tmp_res[j+1][i], tmp_res[j+2][i], tmp_res[j+3][i], tmp_res[j+4][i], tmp_res[j+5][i]); 
			block[j][i] = __fast_iclip0_255(((result+512)>>10)); // HP restriction
			block[j][i] = (block[j][i] + __fast_iclip0_255(((tmp_res[j+2][i]+16)>>5)) +1 )/2; // HP restriction
		}	 
	}
}

void  get_block_16xh_p22_c(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	byte *src;	
	int i, j;
	int result;
	int tmp_res[21][16];
	const int w1 = stride;	

	src = pSrc - (2*w1);
	for (j = -2; j < height+3; j++) 
	{				
		for (i = 0; i < 16; i++)
			tmp_res[j+2][i] = q_interpol(src[i-2], src[i-1], src[i], src[i+1], src[i+2], src[i+3]);								
		src += w1;
	}	
	for (j = 0; j < height; j++) 
	{
		for (i = 0; i < 16; i++) 
		{
			result = q_interpol(tmp_res[j][i], tmp_res[j+1][i], tmp_res[j+2][i], tmp_res[j+3][i], tmp_res[j+4][i], tmp_res[j+5][i]); 
			block[j][i] = __fast_iclip0_255(((result+512)>>10)); // HP restriction
		}	 
	}
}

void  get_block_16xh_p23_c(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int i, j; 
	int result;
	int tmp_res[21][16];
	byte *src;
	const int w1 = stride;	

	src = pSrc -(2*w1);
	for (j = -2; j < height+3; j++) 
	{				
		for (i = 0; i < 16; i++)
			tmp_res[j+2][i] = q_interpol(src[i-2], src[i-1], src[i], src[i+1], src[i+2], src[i+3]);								
		src += w1;
	}	

	for (j = 0; j < height; j++) 
	{
		for (i = 0; i < 16; i++) 
		{
			result = q_interpol(tmp_res[j][i], tmp_res[j+1][i], tmp_res[j+2][i], tmp_res[j+3][i], tmp_res[j+4][i], tmp_res[j+5][i]); 
			block[j][i] = __fast_iclip0_255(((result+512)>>10)); // HP restriction
			block[j][i] = (block[j][i] + __fast_iclip0_255(((tmp_res[j+3][i]+16)>>5)) +1 )/2; // HP restriction			
		}	 
	}
}

void  get_block_16xh_p30_c(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int i, j;
	int result;
	byte *src;
	const int w1 = stride;

	src = pSrc;
	for (j = 0; j < height; j++) 
	{				
		for (i = 0; i < 16; i++) 
		{
			result = q_interpol(src[i-2], src[i-1], src[i], src[i+1], src[i+2], src[i+3]);
			block[j][i] = __fast_iclip0_255((result+16)>>5); // HP restriction
			block[j][i] = ((block[j][i] + src[i+1] +1 )>>1);			
		}
		src += w1;
	}
}

void  get_block_16xh_p31_c(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	int i, j; 
	int result;
	byte *src;
	const int w1 = stride;
	const int w2 = stride + stride;
	const int w3 = (stride<<1) + stride;

	src = pSrc;
	for (j = 0; j < height; j++) 
	{					
		for (i = 0; i < 16; i++) 
		{

			result = q_interpol(src[i-2], src[i-1], src[i], src[i+1], src[i+2], src[i+3]);
			block[j][i] = __fast_iclip0_255(((result+16)>>5)); // HP restriction
		}
		src += w1;
	}				

	src = pSrc + 1;	
	for (j = 0; j < height; j++) 
	{				
		for (i = 0; i < 16; i++) 
		{		
			result = q_interpol(src[i-w2], src[i-w1], src[i], src[i+w1], src[i+w2], src[i+w3]); 	
			block[j][i] = (block[j][i] + __fast_iclip0_255(((result+16)>>5)) +1 ) / 2; // HP restriction
		}
		src += w1;
	}
}

void  get_block_16xh_p32_c(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	byte *src;
	int i, j;
	int result;
	int tmp_res[16][21];
	const int w1 = stride;
	const int w2 = stride + stride;
	const int w3 = (stride<<1) + stride;

	src = pSrc;
	for (j = 0; j < height; j++) 
	{			
		for (i = -2; i < 19; i++)
			tmp_res[j][i+2] = q_interpol(src[i-w2],src[i-w1], src[i], src[i+w1], src[i+w2], src[i+w3]);  	
		src += w1;
	}	
	for (j = 0; j < height; j++) 
	{
		for (i = 0; i < 16; i++) 
		{
			result = q_interpol(tmp_res[j][i], tmp_res[j][i+1], tmp_res[j][i+2], tmp_res[j][i+3], tmp_res[j][i+4], tmp_res[j][i+5]);  
			block[j][i] = __fast_iclip0_255(((result+512)>>10)); // HP restriction
			block[j][i] = (block[j][i] + __fast_iclip0_255(((tmp_res[j][i+3]+16)>>5))+1)/2; // HP restriction			
		}
	}
}

void  get_block_16xh_p33_c(imgpel *pSrc, imgpel block[LUMA_BLOCK_SIZE][LUMA_BLOCK_SIZE], int stride, int height)
{
	byte *src;
	int i, j;
	int result;
	const int w1 = stride;		
	const int w2 = stride + stride;
	const int w3 = (stride<<1) + stride;

	src = pSrc + w1;
	for (j = 0; j < height; j++) 
	{					
		for (i = 0; i < 16; i++) 
		{

			result = q_interpol(src[i-2], src[i-1], src[i], src[i+1], src[i+2], src[i+3]);
			block[j][i] = __fast_iclip0_255(((result+16)>>5)); // HP restriction
		}
		src += w1;
	}				

	src = pSrc + 1;

	for (j = 0; j < height; j++) 
	{				
		for (i = 0; i < 16; i++) 
		{		
			result = q_interpol(src[i-w2], src[i-w1], src[i], src[i+w1], src[i+w2], src[i+w3]); 	
			block[j][i] = (block[j][i] + __fast_iclip0_255(((result+16)>>5)) +1 ) / 2; // HP restriction
		}
		src += w1;
	}	
}
