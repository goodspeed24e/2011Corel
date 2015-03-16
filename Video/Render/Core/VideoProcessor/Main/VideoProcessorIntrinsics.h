//=============================================================================
//	THIS SOURCE CODE IS PROPRIETARY INFORMATION BELONGING TO INTERVIDEO, INC.
// 	ANY USE INCLUDING BUT NOT LIMITED TO COPYING OF CODE, CONCEPTS, AND/OR
//	ALGORITHMS IS PROHIBITED EXCEPT WITH EXPRESS WRITTEN PERMISSION BY THE 
//	COMPANY.
//
// 	THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
// 	KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// 	IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
// 	PURPOSE.
//
// 	Copyright (c) 1998 - 2008  InterVideo Corporation.  All Rights Reserved.
//
//-----------------------------------------------------------------------------

#ifndef _VIDEOPROCESSOR_INTRINSICS_H_
#define _VIDEOPROCESSOR_INTRINSICS_H_

#include <emmintrin.h>

#define _mm_pavgb		_mm_avg_epu8
#define _mm_paddsb		_mm_adds_epi8
#define _mm_paddusb		_mm_adds_epu8
#define _mm_psubusb		_mm_subs_epu8
#define _mm_psubb		_mm_sub_epi8
#define _mm_pand		_mm_and_si128
#define _mm_por			_mm_or_si128
#define _mm_pxor		_mm_xor_si128
#define _mm_pmullw		_mm_mullo_epi16
#define _mm_pmulhw		_mm_mulhi_epi16
#define _mm_pmulhuw		_mm_mulhi_epu16
#define _mm_paddsw		_mm_add_epi16
#define _mm_packuswb	_mm_packus_epi16
#define _mm_punpcklbw	_mm_unpacklo_epi8
#define _mm_punpckhbw	_mm_unpackhi_epi8
#define _mm_punpcklwd	_mm_unpacklo_epi16
#define _mm_punpckhwd	_mm_unpackhi_epi16
#define _mm_punpckldq	_mm_unpacklo_epi32
#define _mm_punpckhdq	_mm_unpackhi_epi32
#define _mm_psllwi		_mm_slli_epi16
#define _mm_psllw		_mm_sll_epi16
#define _mm_psrlwi		_mm_srli_epi16
#define _mm_psrlqi		_mm_srli_epi64
#define _mm_psrlq		_mm_srl_epi64
#define _mm_psllqi		_mm_slli_epi64
#define _mm_pslldq		_mm_slli_si128
#define _mm_psrldq		_mm_srli_si128
#define _mm_pshufd		_mm_shuffle_epi32
#define _mm_pshuflw		_mm_shufflelo_epi16
#define _mm_pshufhw		_mm_shufflehi_epi16
#define _mm_pmaxub		_mm_max_epu8
#define _mm_pminub		_mm_min_epu8

#define prefetch_0(addr, stride)
#define prefetch_1(addr, stride) _mm_prefetch((char *)(addr)+32, _MM_HINT_T0)
#define prefetch_2(addr, stride) _mm_prefetch((char *)(addr)+stride+32, _MM_HINT_T0)

#define _m_movq(src)	*(__m64*)(src)
#define _m_movdq(src)	*(__m128i*)(src)

#define store_64(dst, src)  *(__m64*)(dst) = (src);
#define store_128(dst, src) *(__m128i*)(dst) = (src);

#define _m_pavgb_alt(dst, src)\
	_m_psubb(_m_por(dst,src),_m_psrlqi(_m_pand(_m_pxor(dst,src),_m_movq(_xfe)), 1))


#define load_64(dst, src, stride, mq)\
	(dst) = mq(src);
#define load_128(dst, src, stride, mdq)\
	(dst) = mdq(src);
#define load11_64(dst, src, stride, mq)\
	(dst) = _m_pavgb(mq(src),mq((src)-(stride)))
#define load11_128(dst, src, stride, mdq)\
	(dst) = _mm_pavgb(mdq(src),mdq((src)-(stride)))
#define loadbob_64(dst, src, stride, mq)\
	(dst) =  _m_pavgb(mq((src)-(stride)), mq((src)+(stride)));
#define loadbob_128(dst, src, stride, mdq)\
	(dst) =  _mm_pavgb(mdq((src)-(stride)), mdq((src)+(stride)));
#define loadn1_64(dst, src, stride, mq)\
	(dst) = _m_pavgb(mq(src),mq((src)+(stride)))
#define loadn1_128(dst, src, stride, mdq)\
	(dst) = _mm_pavgb(mdq(src),mdq((src)+(stride)))
#define load11_alt_64(dst, src, stride, mq)\
	(dst) = _m_pavgb_alt(mq(src), mq((src)-(stride)))
#define loadmed_64(dst, src, stride, mq)\
	{\
	__m64 src2, src3;\
	src2 =  mq(src);\
	src3 =  mq((src)+(stride));\
	dst = _m_pmaxub(src2, src3);\
	src2 = _m_pminub(src2, src3);\
	dst = _m_pminub(dst, mq((src)-(stride)));\
	dst = _m_pmaxub(dst, src2);\
	}
#define loadmed_128(dst, src, stride, mdq)\
	{\
	__m128i src2, src3;\
	src2 =  mdq(src);\
	src3 =  mdq((src)+(stride));\
	dst = _mm_pmaxub(src2, src3);\
	src2 = _mm_pminub(src2, src3);\
	dst = _mm_pminub(dst, mdq((src)-(stride)));\
	dst = _mm_pmaxub(dst, src2);\
	}

#define loady_64(dst, src, stride) load_64(dst, src, stride, _m_movq)
#define loady_128(dst, src, stride) load_128(dst, src, stride, _m_movdq)
#define load11y_64(dst, src, stride) load11_64(dst, src, stride, _m_movq)
#define load11y_128(dst, src, stride) load11_128(dst, src, stride, _m_movdq)
#define loadboby_64(dst, src, stride) loadbob_64(dst, src, stride, _m_movq)
#define loadboby_128(dst, src, stride) loadbob_128(dst, src, stride, _m_movdq)
#define loadn1y_64(dst, src, stride) loadn1_64(dst, src, stride, _m_movq)
#define loadn1y_128(dst, src, stride) loadn1_128(dst, src, stride, _m_movdq)
#define load11_alty_64(dst, src, stride) load11_alt_64(dst, src, stride, _m_movq)
#define loadmedy_64(dst, src, stride) loadmed_64(dst, src, stride, _m_movq)
#define loadmedy_128(dst, src, stride) loadmed_128(dst, src, stride, _m_movdq)
#define loadmed0y_64(dst,src,stride) loady_64(dst,src,stride)
#define loadmed1y_64(dst,src,stride) loadmedy_64(dst,src,stride)
#define loadmed2y_64(dst,src,stride) loady_64(dst,src,stride)
#define loadmed3y_64(dst,src,stride) loadmedy_64(dst,src,stride)
#define loadmed0y_128(dst,src,stride) loady_128(dst,src,stride)
#define loadmed1y_128(dst,src,stride) loadmedy_128(dst,src,stride)
#define loadmed2y_128(dst,src,stride) loady_128(dst,src,stride)
#define loadmed3y_128(dst,src,stride) loadmedy_128(dst,src,stride)

#define loadc_64(dst, src, stride) load_64(dst, src, stride, _m_movqc)
#define loadc_128(dst, src, stride) load_128(dst, src, stride, _m_movdqc)
#define load11c_64(dst, src, stride) load11_64(dst, src, stride, _m_movqc)
#define load11c_128(dst, src, stride) load11_128(dst, src, stride, _m_movdqc)
#define loadbobc_64(dst, src, stride) loadbob_64(dst, src, stride, _m_movqc)
#define loadbobc_128(dst, src, stride) loadbob_128(dst, src, stride, _m_movdqc)
#define loadn1c_64(dst, src, stride) loadn1_64(dst, src, stride, _m_movqc)
#define loadn1c_128(dst, src, stride) loadn1_128(dst, src, stride, _m_movdqc)
#define load11_altc_64(dst, src, stride) load11_alt_64(dst, src, stride, _m_movqc)
#define loadmedc_64(dst, src, stride) loadc_64(dst, src, stride)
#define loadmed0c_64(dst, src, stride) loadc_64(dst, src, stride)
#define loadmed1c_64(dst, src, stride)\
	{\
	__m64 src1, src2, src3;\
	src1 =  _m_movqc((src)-(stride));\
	src2 =  _m_movqc(src);\
	src3 =  _m_movqc((src)+(stride));\
	dst = _m_pmaxub(src2, src3);\
	src3 = _m_pminub(src3, src2);\
	dst = _m_pminub(dst, src1);\
	dst = _m_pmaxub(dst, src3);\
	store_64(((unsigned char *)m_uv_store7_interlaced+_j),dst);\
	dst = _m_pavgb(dst, src1);\
	}
#define loadmed2c_64(dst, src, stride)\
	dst = _m_movq((unsigned char *)m_uv_store7_interlaced+_j);
#define loadmed3c_64(dst, src, stride)\
	{\
	__m64 src1, src2;\
	src1 = _m_movqc(src);\
	src2 = _m_movq((unsigned char *)m_uv_store7_interlaced+_j);\
	dst = _m_pavgb(src2, src1);\
	}
#define loadmed0c_128(dst, src, stride) loadc_128(dst, src, stride)
#define loadmed1c_128(dst, src, stride)\
	{\
	__m128i src1, src2, src3;\
	src1 =  _m_movdqc((src)-(stride));\
	src2 =  _m_movdqc(src);\
	src3 =  _m_movdqc((src)+(stride));\
	dst = _mm_pmaxub(src2, src3);\
	src3 = _mm_pminub(src3, src2);\
	dst = _mm_pminub(dst, src1);\
	dst = _mm_pmaxub(dst, src3);\
	store_128(((unsigned char *)m_uv_store7_interlaced+_j),dst);\
	dst = _mm_pavgb(dst, src1);\
	}
#define loadmed2c_128(dst, src, stride)\
	dst = _m_movdq((unsigned char *)m_uv_store7_interlaced+_j);
#define loadmed3c_128(dst, src, stride)\
	{\
	__m128i src1, src2;\
	src1 = _m_movdqc(src);\
	src2 = _m_movdq((unsigned char *)m_uv_store7_interlaced+_j);\
	dst = _mm_pavgb(src2, src1);\
	}
#define loadmedint0c_64(dst, src, stride) load_64(dst, src, stride, _m_movqc)
#define loadmedint1c_64(dst, src, stride) loadmed_64(dst, src, stride, _m_movqc)
#define loadmedint2c_64(dst, src, stride) load_64(dst, src, stride, _m_movqc)
#define loadmedint3c_64(dst, src, stride) loadmed_64(dst, src, stride, _m_movqc)
#define loadmedint0c_128(dst, src, stride) load_128(dst, src, stride, _m_movdqc)
#define loadmedint1c_128(dst, src, stride) loadmed_128(dst, src, stride, _m_movdqc)
#define loadmedint2c_128(dst, src, stride) load_128(dst, src, stride, _m_movdqc)
#define loadmedint3c_128(dst, src, stride) loadmed_128(dst, src, stride, _m_movdqc)

// yuv based blending

#define spicblendyuv_64(dst, alpha, spic, tmpsrc, tmpalpha, zero, unpackfunc)\
	tmpsrc	= dst;\
	dst		= _m_punpcklbw(dst, zero);\
	tmpsrc	= _m_punpckhbw(tmpsrc, zero);\
	tmpalpha= unpackfunc(*(__m64*)(alpha), zero);\
	dst		= _m_pmullw(dst, _m_punpcklwd(tmpalpha, tmpalpha));\
	tmpsrc	= _m_pmullw(tmpsrc, _m_punpckhwd(tmpalpha, tmpalpha));\
	dst		= _m_psrlwi(dst, 4);\
	tmpsrc	= _m_psrlwi(tmpsrc, 4);\
	dst		= _m_packuswb(dst, tmpsrc);\
	dst		= _m_paddusb(dst, *(__m64*)(spic));

#define spicblendyuv_64_HD8bpp(dst, alpha, spic, tmpsrc, tmpalpha, zero, unpackfunc)\
	tmpsrc	= dst;\
	dst		= _m_punpcklbw(dst, zero);\
	tmpsrc	= _m_punpckhbw(tmpsrc, zero);\
	tmpalpha= unpackfunc(*(__m64*)(alpha), zero);\
	dst		= _m_pmullw(dst, _m_punpcklwd(tmpalpha, tmpalpha));\
	tmpsrc	= _m_pmullw(tmpsrc, _m_punpckhwd(tmpalpha, tmpalpha));\
	dst		= _m_psrlwi(dst, 8);\
	tmpsrc	= _m_psrlwi(tmpsrc, 8);\
	dst		= _m_packuswb(dst, tmpsrc);\
	dst		= _m_paddusb(dst, *(__m64*)(spic));

#define spicblendyuv_64_HD8bpp_AlphaAdd1(dst, alpha, spic, tmpsrc, tmpalpha, zero, unpackfunc)\
	tmpsrc	= dst;\
	dst		= _m_punpcklbw(dst, zero);\
	tmpsrc	= _m_punpckhbw(tmpsrc, zero);\
	tmpalpha= unpackfunc(*(__m64*)(alpha), zero);\
	tmpalpha= _m_paddw(tmpalpha, _mm_set_pi16(1,1,1,1));\
	dst		= _m_pmullw(dst, _m_punpcklwd(tmpalpha, tmpalpha));\
	tmpsrc	= _m_pmullw(tmpsrc, _m_punpckhwd(tmpalpha, tmpalpha));\
	dst		= _m_psrlwi(dst, 8);\
	tmpsrc	= _m_psrlwi(tmpsrc, 8);\
	dst		= _m_packuswb(dst, tmpsrc);\
	dst		= _m_paddusb(dst, *(__m64*)(spic));

#define adjust_byuv_64(dst, src)\
	dst = _m_paddusb(src, *(__m64 *)m_adjust_up);\
	dst = _m_psubusb(dst, *(__m64 *)m_adjust_down);
#define adjust_byuv_128(dst, src)\
	dst = _mm_paddusb(src, *(__m128i *)m_adjust_up);\
	dst = _mm_psubusb(dst, *(__m128i *)m_adjust_down);

#define adjust_bcsyuv_64(dst, src)\
	dst = _m_punpcklbw(src, src);\
	src = _m_punpckhbw(src, src);\
	src = _m_pxor(src, *(__m64 *)x8000);\
	dst = _m_pxor(dst, *(__m64 *)x8000);\
	src = _m_pmulhw(src, *(__m64 *)m_adjust_multiply_yuv);\
	dst = _m_pmulhw(dst, *(__m64 *)m_adjust_multiply_yuv);\
	src = _m_paddsw(src, *(__m64 *)m_adjust_offset_yuv);\
	dst = _m_paddsw(dst, *(__m64 *)m_adjust_offset_yuv);\
	dst = _m_packuswb(dst, src);
#define adjust_bcsyuv_128(dst, src)\
	dst = _mm_punpcklbw(src, src);\
	src = _mm_punpckhbw(src, src);\
	src = _mm_pxor(src, *(__m128i *)x8000);\
	dst = _mm_pxor(dst, *(__m128i *)x8000);\
	src = _mm_pmulhw(src, *(__m128i *)m_adjust_multiply_yuv);\
	dst = _mm_pmulhw(dst, *(__m128i *)m_adjust_multiply_yuv);\
	src = _mm_paddsw(src, *(__m128i *)m_adjust_offset_yuv);\
	dst = _mm_paddsw(dst, *(__m128i *)m_adjust_offset_yuv);\
	dst = _mm_packuswb(dst, src);

#define adjust_bcshgyuv_64(dst, src)\
	{\
	__m64 tmp1, tmp2, tmp3, tmp4;\
	dst = _m_punpcklbw(src, src);\
	src = _m_punpckhbw(src, src);\
	tmp1 = _m_pmulhuw(src, src);\
	tmp2 = _m_pmulhuw(dst, dst);\
	src = _m_pxor(src, *(__m64 *)x8000);\
	dst = _m_pxor(dst, *(__m64 *)x8000);\
	tmp3 = _m_pmulhw(src, *(__m64 *)m_adjust_hue_yuv);\
	tmp4 = _m_pmulhw(dst, *(__m64 *)m_adjust_hue_yuv);\
	src = _m_pmulhw(src, *(__m64 *)m_adjust_multiply_yuv);\
	dst = _m_pmulhw(dst, *(__m64 *)m_adjust_multiply_yuv);\
	tmp1 = _m_psrlwi(tmp1,1);\
	tmp2 = _m_psrlwi(tmp2,1);\
	tmp1 = _m_pmulhw(tmp1, *(__m64 *)m_adjust_gamma_yuv);\
	tmp2 = _m_pmulhw(tmp2, *(__m64 *)m_adjust_gamma_yuv);\
	src = _m_paddsw(src, *(__m64 *)m_adjust_offset_yuv);\
	dst = _m_paddsw(dst, *(__m64 *)m_adjust_offset_yuv);\
	tmp3 = _m_pshufw(tmp3,0x4e);\
	tmp4 = _m_pshufw(tmp4,0x4e);\
	src = _m_paddsw(src, tmp3);\
	dst = _m_paddsw(dst, tmp4);\
	src = _m_paddsw(src, tmp1);\
	dst = _m_paddsw(dst, tmp2);\
	dst = _m_packuswb(dst, src);\
	}
#define adjust_bcshgyuv_128(dst, src)\
	{\
	__m128i tmp1, tmp2, tmp3, tmp4;\
	dst = _mm_punpcklbw(src, src);\
	src = _mm_punpckhbw(src, src);\
	tmp1 = _mm_pmulhuw(src, src);\
	tmp2 = _mm_pmulhuw(dst, dst);\
	src = _mm_pxor(src, *(__m128i *)x8000);\
	dst = _mm_pxor(dst, *(__m128i *)x8000);\
	tmp3 = _mm_pmulhw(src, *(__m128i *)m_adjust_hue_yuv);\
	tmp4 = _mm_pmulhw(dst, *(__m128i *)m_adjust_hue_yuv);\
	src = _mm_pmulhw(src, *(__m128i *)m_adjust_multiply_yuv);\
	dst = _mm_pmulhw(dst, *(__m128i *)m_adjust_multiply_yuv);\
	tmp1 = _mm_psrlwi(tmp1,1);\
	tmp2 = _mm_psrlwi(tmp2,1);\
	tmp1 = _mm_pmulhw(tmp1, *(__m128i *)m_adjust_gamma_yuv);\
	tmp2 = _mm_pmulhw(tmp2, *(__m128i *)m_adjust_gamma_yuv);\
	src = _mm_paddsw(src, *(__m128i *)m_adjust_offset_yuv);\
	dst = _mm_paddsw(dst, *(__m128i *)m_adjust_offset_yuv);\
	tmp3 = _mm_pshufd(tmp3,0xb1);\
	tmp4 = _mm_pshufd(tmp4,0xb1);\
	src = _mm_paddsw(src, tmp3);\
	dst = _mm_paddsw(dst, tmp4);\
	src = _mm_paddsw(src, tmp1);\
	dst = _mm_paddsw(dst, tmp2);\
	dst = _mm_packuswb(dst, src);\
	}

// yv12 based

#define spicblendyv12_64(dst, alpha, spic, tmpsrc, tmpalpha, zero)\
	tmpsrc	= dst;\
	dst		= _m_punpcklbw(dst, zero);\
	tmpsrc	= _m_punpckhbw(tmpsrc, zero);\
	tmpalpha= *(__m64*)(alpha);\
	dst		= _m_pmullw(dst, _m_punpcklbw(tmpalpha, zero));\
	tmpsrc	= _m_pmullw(tmpsrc, _m_punpckhbw(tmpalpha, zero));\
	dst		= _m_psrlwi(dst, 4);\
	tmpsrc	= _m_psrlwi(tmpsrc, 4);\
	dst		= _m_packuswb(dst, tmpsrc);\
	dst		= _m_paddusb(dst, *(__m64*)(spic));
    
#define spicblendyv12_64HDAlphaPlus1(dst, alpha, spic, tmpsrc, tmpalpha, zero)\
	tmpsrc	= dst;\
	dst		= _m_punpcklbw(dst, zero);\
	tmpsrc	= _m_punpckhbw(tmpsrc, zero);\
	tmpalpha= _m_punpcklbw(*(__m64*)(alpha), zero);\
	tmpalpha= _m_paddw(tmpalpha, _mm_set_pi16(1,1,1,1));\
	dst		= _m_pmullw(dst, tmpalpha);\
	tmpalpha= _m_punpckhbw(*(__m64*)(alpha), zero);\
	tmpalpha= _m_paddw(tmpalpha, _mm_set_pi16(1,1,1,1));\
	tmpsrc	= _m_pmullw(tmpsrc, tmpalpha);\
	dst		= _m_psrlwi(dst, 8);\
	tmpsrc	= _m_psrlwi(tmpsrc, 8);\
	dst		= _m_packuswb(dst, tmpsrc);\
	dst		= _m_paddusb(dst, *(__m64*)(spic));


#define adjust_by_64(dst, src)\
	dst = _m_paddusb(src, *(__m64 *)m_adjust_up_y);\
	dst = _m_psubusb(dst, *(__m64 *)m_adjust_down_y);
#define adjust_by_128(dst, src)\
	dst = _mm_paddusb(src, *(__m128i *)m_adjust_up_y);\
	dst = _mm_psubusb(dst, *(__m128i *)m_adjust_down_y);

#define adjust_buv_64(dst, src)\
	dst = _m_paddusb(src, *(__m64 *)m_adjust_up_uv);\
	dst = _m_psubusb(dst, *(__m64 *)m_adjust_down_uv);
#define adjust_buv_128(dst, src)\
	dst = _mm_paddusb(src, *(__m128i *)m_adjust_up_uv);\
	dst = _mm_psubusb(dst, *(__m128i *)m_adjust_down_uv);

#define adjust_bcsy_64(dst, src)\
	dst = _m_punpcklbw(src, src);\
	src = _m_punpckhbw(src, src);\
	src = _m_pxor(src, *(__m64 *)x8000);\
	dst = _m_pxor(dst, *(__m64 *)x8000);\
	src = _m_pmulhw(src, *(__m64 *)m_adjust_multiply_y);\
	dst = _m_pmulhw(dst, *(__m64 *)m_adjust_multiply_y);\
	src = _m_paddsw(src, *(__m64 *)m_adjust_offset_y);\
	dst = _m_paddsw(dst, *(__m64 *)m_adjust_offset_y);\
	dst = _m_packuswb(dst, src);
#define adjust_bcsy_128(dst, src)\
	dst = _mm_punpcklbw(src, src);\
	src = _mm_punpckhbw(src, src);\
	src = _mm_pxor(src, *(__m128i *)x8000);\
	dst = _mm_pxor(dst, *(__m128i *)x8000);\
	src = _mm_pmulhw(src, *(__m128i *)m_adjust_multiply_y);\
	dst = _mm_pmulhw(dst, *(__m128i *)m_adjust_multiply_y);\
	src = _mm_paddsw(src, *(__m128i *)m_adjust_offset_y);\
	dst = _mm_paddsw(dst, *(__m128i *)m_adjust_offset_y);\
	dst = _mm_packuswb(dst, src);

#define adjust_bcsuv_64(dst, src)\
	dst = _m_punpcklbw(src, src);\
	src = _m_punpckhbw(src, src);\
	src = _m_pxor(src, *(__m64 *)x8000);\
	dst = _m_pxor(dst, *(__m64 *)x8000);\
	src = _m_pmulhw(src, *(__m64 *)m_adjust_multiply_uv);\
	dst = _m_pmulhw(dst, *(__m64 *)m_adjust_multiply_uv);\
	src = _m_paddsw(src, *(__m64 *)m_adjust_offset_uv);\
	dst = _m_paddsw(dst, *(__m64 *)m_adjust_offset_uv);\
	dst = _m_packuswb(dst, src);
#define adjust_bcsuv_128(dst, src)\
	dst = _mm_punpcklbw(src, src);\
	src = _mm_punpckhbw(src, src);\
	src = _mm_pxor(src, *(__m128i *)x8000);\
	dst = _mm_pxor(dst, *(__m128i *)x8000);\
	src = _mm_pmulhw(src, *(__m128i *)m_adjust_multiply_uv);\
	dst = _mm_pmulhw(dst, *(__m128i *)m_adjust_multiply_uv);\
	src = _mm_paddsw(src, *(__m128i *)m_adjust_offset_uv);\
	dst = _mm_paddsw(dst, *(__m128i *)m_adjust_offset_uv);\
	dst = _mm_packuswb(dst, src);


#define adjust_bcshgy_64(dst, src)\
	{\
	__m64 tmp1, tmp2;\
	dst = _m_punpcklbw(src, src);\
	src = _m_punpckhbw(src, src);\
	tmp1 = _m_pmulhuw(src, src);\
	tmp2 = _m_pmulhuw(dst, dst);\
	src = _m_pxor(src, *(__m64 *)x8000);\
	dst = _m_pxor(dst, *(__m64 *)x8000);\
	src = _m_pmulhw(src, *(__m64 *)m_adjust_multiply_y);\
	dst = _m_pmulhw(dst, *(__m64 *)m_adjust_multiply_y);\
	tmp1 = _m_psrlwi(tmp1,1);\
	tmp2 = _m_psrlwi(tmp2,1);\
	tmp1 = _m_pmulhw(tmp1, *(__m64 *)m_adjust_gamma_y);\
	tmp2 = _m_pmulhw(tmp2, *(__m64 *)m_adjust_gamma_y);\
	src = _m_paddsw(src, *(__m64 *)m_adjust_offset_y);\
	dst = _m_paddsw(dst, *(__m64 *)m_adjust_offset_y);\
	src = _m_paddsw(src, tmp1);\
	dst = _m_paddsw(dst, tmp2);\
	dst = _m_packuswb(dst, src);\
	}
#define adjust_bcshgy_128(dst, src)\
	{\
	__m128i tmp1, tmp2;\
	dst = _mm_punpcklbw(src, src);\
	src = _mm_punpckhbw(src, src);\
	tmp1 = _mm_pmulhuw(src, src);\
	tmp2 = _mm_pmulhuw(dst, dst);\
	src = _mm_pxor(src, *(__m128i *)x8000);\
	dst = _mm_pxor(dst, *(__m128i *)x8000);\
	src = _mm_pmulhw(src, *(__m128i *)m_adjust_multiply_y);\
	dst = _mm_pmulhw(dst, *(__m128i *)m_adjust_multiply_y);\
	tmp1 = _mm_psrlwi(tmp1,1);\
	tmp2 = _mm_psrlwi(tmp2,1);\
	tmp1 = _mm_pmulhw(tmp1, *(__m128i *)m_adjust_gamma_y);\
	tmp2 = _mm_pmulhw(tmp2, *(__m128i *)m_adjust_gamma_y);\
	src = _mm_paddsw(src, *(__m128i *)m_adjust_offset_y);\
	dst = _mm_paddsw(dst, *(__m128i *)m_adjust_offset_y);\
	src = _mm_paddsw(src, tmp1);\
	dst = _mm_paddsw(dst, tmp2);\
	dst = _mm_packuswb(dst, src);\
	}

#define adjust_bcshguv_64(dst, src)\
	{\
	__m64 tmp3, tmp4;\
	dst = _m_punpcklbw(src, src);\
	src = _m_punpckhbw(src, src);\
	src = _m_pxor(src, *(__m64 *)x8000);\
	dst = _m_pxor(dst, *(__m64 *)x8000);\
	tmp3 = _m_pmulhw(src, *(__m64 *)m_adjust_hue_uv);\
	tmp4 = _m_pmulhw(dst, *(__m64 *)m_adjust_hue_uv);\
	src = _m_pmulhw(src, *(__m64 *)m_adjust_multiply_uv);\
	dst = _m_pmulhw(dst, *(__m64 *)m_adjust_multiply_uv);\
	src = _m_paddsw(src, *(__m64 *)m_adjust_offset_uv);\
	dst = _m_paddsw(dst, *(__m64 *)m_adjust_offset_uv);\
	tmp3 = _m_pshufw(tmp3,0xb1);\
	tmp4 = _m_pshufw(tmp4,0xb1);\
	src = _m_paddsw(src, tmp3);\
	dst = _m_paddsw(dst, tmp4);\
	dst = _m_packuswb(dst, src);\
	}
#define adjust_bcshguv_128(dst, src)\
	{\
	__m128i tmp3, tmp4;\
	dst = _mm_punpcklbw(src, src);\
	src = _mm_punpckhbw(src, src);\
	src = _mm_pxor(src, *(__m128i *)x8000);\
	dst = _mm_pxor(dst, *(__m128i *)x8000);\
	tmp3 = _mm_pmulhw(src, *(__m128i *)m_adjust_hue_uv);\
	tmp4 = _mm_pmulhw(dst, *(__m128i *)m_adjust_hue_uv);\
	src = _mm_pmulhw(src, *(__m128i *)m_adjust_multiply_uv);\
	dst = _mm_pmulhw(dst, *(__m128i *)m_adjust_multiply_uv);\
	src = _mm_paddsw(src, *(__m128i *)m_adjust_offset_uv);\
	dst = _mm_paddsw(dst, *(__m128i *)m_adjust_offset_uv);\
	tmp3 = _mm_pshuflw(tmp3,0xb1);\
	tmp4 = _mm_pshuflw(tmp4,0xb1);\
	tmp3 = _mm_pshufhw(tmp3,0xb1);\
	tmp4 = _mm_pshufhw(tmp4,0xb1);\
	src = _mm_paddsw(src, tmp3);\
	dst = _mm_paddsw(dst, tmp4);\
	dst = _mm_packuswb(dst, src);\
	}

#endif //_VIDEOPROCESSOR_INTRINSICS_H_
