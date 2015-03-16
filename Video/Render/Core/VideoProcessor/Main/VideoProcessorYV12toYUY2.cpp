//=======================================================================================
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
// VideoProcessorYV12toYUY2.cpp: implementation of the display renderer for YUY2 surfaces
//
//////////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <windows.h>
#include <assert.h>
#include <streams.h>
#include <cmath>
#include "VideoProcessorYV12toYUY2.h"
#include "VideoProcessorIntrinsics.h"

static const __int64 ALIGN(16,x0000[2])= {I64(0), I64(0)}
,x8000[2]          = {I64(0x8000800080008000), I64(0x8000800080008000)}
,xExchangeMask1[2] = {I64(0x0000000000FF0000), I64(0x0000000000FF0000)}
,xExchangeMask2[2] = {I64(0x000000FF00000000), I64(0x000000FF00000000)}
,xExchangeMask3[2] = {I64(0xFFFFFF00FF00FFFF), I64(0xFFFFFF00FF00FFFF)}
,_x7f[2]           = {I64(0x7f7f7f7f7f7f7f7f), I64(0x7f7f7f7f7f7f7f7f)}
,_xfe[2]           = {I64(0xfefefefefefefefe), I64(0xfefefefefefefefe)};
static const __int64 cnst0000 = I64(0)
, cnst8000          = I64(0x8000800080008000)
, cnstExchangeMask1 = I64(0x0000000000FF0000)
, cnstExchangeMask2 = I64(0x000000FF00000000)
, cnstExchangeMask3 = I64(0xFFFFFF00FF00FFFF);

#define exchange64(srcdst) \
{\
	__m64 m_tmp1, m_tmp2;\
	m_tmp1 = _m_pand(srcdst, *(__m64 *)xExchangeMask1);\
	m_tmp2 = _m_pand(srcdst, *(__m64 *)xExchangeMask2);\
	srcdst = _m_pand(srcdst, *(__m64 *)xExchangeMask3);\
	m_tmp1 = _m_psllqi(m_tmp1, 16);\
	m_tmp2 = _m_psrlqi(m_tmp2, 16);\
	srcdst = _m_por(srcdst, m_tmp1);\
	srcdst = _m_por(srcdst, m_tmp2);\
}

#define exchange128(srcdst) \
{\
	__m128i mm_tmp1, mm_tmp2;\
	mm_tmp1 = _mm_pand(srcdst, *(__m128i *)xExchangeMask1);\
	mm_tmp2 = _mm_pand(srcdst, *(__m128i *)xExchangeMask2);\
	srcdst = _mm_pand(srcdst, *(__m128i *)xExchangeMask3);\
	mm_tmp1 = _mm_psllqi(mm_tmp1, 16);\
	mm_tmp2 = _mm_psrlqi(mm_tmp2, 16);\
	srcdst = _mm_por(srcdst, mm_tmp1);\
	srcdst = _mm_por(srcdst, mm_tmp2);\
}

#define DECLARE_CONVERTVARS\
	__m128i mm_tmpin, mm_tmpout, mm_y_a, mm_uv_a;\
	__m64 m_tmpin, m_tmpout, m_y_a, m_uv_a, m_y_b, m_uv_b;\
	unsigned char *dst, *y, *uv, *spic, *alpha;\
	bool b_IsHDSpic8bpp=false;\
	int _i, _j, cmd, stop, line0, offs, cmdarray[4], stoparray[4];

#define createcmds(width, stridey, stridec, dst_addr, y_addr, uv_addr, pspicinfo, rect, line)\
{\
	dst = dst_addr, y = y_addr, uv = uv_addr;\
	if(rect.top==-1 || line<rect.top || line>rect.bottom || pspicinfo->line_properties[line0 = line-rect.top]==0) \
	{/*no subpicture: single command to display everything*/\
		cmdarray[0] = 1;\
		stoparray[0] = width;\
		cmdarray[1] = 0;\
		stoparray[1] = 0;\
	}\
	else\
	{/*subpicture: three commands: display left, display spic, display right*/\
		cmdarray[0] = 1;\
		stoparray[0] = pspicinfo->line_properties[line0] & 0xfff8;\
		cmdarray[1] = 2;\
		stoparray[1] = pspicinfo->line_properties[line0]>>16 & 0xfff8;\
		cmdarray[2] = 1;\
		stoparray[2] = min(width,stridey);\
		cmdarray[3] = 0;\
		stoparray[3] = 0;\
		offs =  line0*(rect.right-rect.left+1) + (stoparray[0]-rect.left);\
		spic = pspicinfo->pixel + (offs<<1);\
		alpha = pspicinfo->alpha + offs;\
		b_IsHDSpic8bpp = pspicinfo->b_IsHD8bpp;\
	}\
}


#define convert_yuv(stridey, stridec,  loady, loaduv, store, spicblend, prefetch, adjust)\
{\
	for(_i=_j=0; (cmd=cmdarray[_i]); _i++)\
	{\
		stop = stoparray[_i];\
		if(cmd==1)\
		{\
			if(!m_bSSE2 || (_j|stop)&8)\
			{\
				for(; _j<stop; _j+=8, y+=8, uv+=UV_BLOCKWIDTH, dst+=16)\
				{\
					loady##y##_64(m_y_a, y, stridey);\
					loaduv##c##_64(m_uv_a, uv, stridec);\
					prefetch(y, stridey);\
					prefetch(uv, stridec);\
					m_tmpin = _m_punpcklbw(m_y_a, m_uv_a);\
					adjust##yuv_64(m_tmpout, m_tmpin);\
					store##_64(dst, m_tmpout);\
					m_tmpin = _m_punpckhbw(m_y_a, m_uv_a);\
					adjust##yuv_64(m_tmpout, m_tmpin);\
					store##_64(dst+8, m_tmpout);\
				}\
			}\
			else\
			{\
				for(; _j<stop; _j+=16, y+=16, uv+=(UV_BLOCKWIDTH<<1), dst+=32)\
				{\
					loady##y##_128(mm_y_a, y, stridey);\
					loaduv##c##_128(mm_uv_a, uv, stridec);\
					prefetch(y, stridey);\
					prefetch(uv, stridec);\
					mm_tmpin = _mm_punpcklbw(mm_y_a, mm_uv_a);\
					adjust##yuv_128(mm_tmpout, mm_tmpin);\
					store##_128(dst, mm_tmpout);\
					mm_tmpin = _mm_punpckhbw(mm_y_a, mm_uv_a);\
					adjust##yuv_128(mm_tmpout, mm_tmpin);\
					store##_128(dst+16, mm_tmpout);\
				}\
			}\
		}\
		else if(cmd==2)\
		{\
			m_uv_b = *(__m64 *)x0000;\
			for(; _j<stop; _j+=8, y+=8, uv+=UV_BLOCKWIDTH, alpha+=8, spic+=16, dst+=16)\
			{\
				loady##y##_64(m_y_a, y, stridey);\
				loaduv##c##_64(m_uv_a, uv, stridec);\
				prefetch(y, stridey);\
				prefetch(uv, stridec);\
				m_tmpin = _m_punpcklbw(m_y_a, m_uv_a);\
				if(b_IsHDSpic8bpp)\
				{\
					if(alpha)\
					{\
					spicblend##yuv_64_HD8bpp_AlphaAdd1(m_tmpin, alpha, spic, m_tmpout, m_y_b, m_uv_b, _m_punpcklbw)\
					adjust##yuv##_64(m_tmpout, m_tmpin);\
					}\
					else\
					{\
					spicblend##yuv_64_HD8bpp(m_tmpin, alpha, spic, m_tmpout, m_y_b, m_uv_b, _m_punpcklbw)\
					adjust##yuv##_64(m_tmpout, m_tmpin);\
					}\
				}\
				else\
				{\
					spicblend##yuv_64(m_tmpin, alpha, spic, m_tmpout, m_y_b, m_uv_b, _m_punpcklbw)\
					adjust##yuv##_64(m_tmpout, m_tmpin);\
				}\
				store##_64(dst, m_tmpout);\
				m_tmpin = _m_punpckhbw(m_y_a, m_uv_a);\
				if(b_IsHDSpic8bpp)\
				{\
					if(alpha)\
					{\
					spicblend##yuv_64_HD8bpp_AlphaAdd1(m_tmpin, alpha, spic+8, m_tmpout, m_y_b, m_uv_b, _m_punpckhbw)\
					adjust##yuv_64(m_tmpout, m_tmpin);\
					}\
					else\
					{\
					spicblend##yuv_64_HD8bpp(m_tmpin, alpha, spic+8, m_tmpout, m_y_b, m_uv_b, _m_punpckhbw)\
					adjust##yuv_64(m_tmpout, m_tmpin);\
					}\
				}\
				else\
				{\
					spicblend##yuv_64(m_tmpin, alpha, spic+8, m_tmpout, m_y_b, m_uv_b, _m_punpckhbw)\
					adjust##yuv_64(m_tmpout, m_tmpin);\
				}\
				store##_64(dst+8, m_tmpout);\
			}\
		}\
	}\
}

// convert_median, centered on top field for interlaced picture
#define convert_median_top(i, width, stridey, stridec, pitch, lines, pspicinfo, rect, adjustyuv)\
{\
	DECLARE_CONVERTVARS;\
	\
	for(; i<((lines-1)&~3); i+=4)\
	{\
		createcmds(width, stridey, stridec, m_dst1, m_y1, m_uv, pspicinfo, rect, i+0);\
		convert_yuv(stridey, stridec, loadmed0, loadmedint0, store, spicblend, prefetch_1, adjustyuv);\
		m_dst1 += pitch;	m_y1 += stridey;	m_uv += stridec;\
		\
		createcmds(width, stridey, stridec, m_dst1, m_y1, m_uv, pspicinfo, rect, i+1);\
		convert_yuv(stridey, stridec, loadmed1, loadmedint1, store, spicblend, prefetch_2, adjustyuv);\
		m_dst1 += pitch;	m_y1 += stridey;	m_uv -= stridec;\
		\
		createcmds(width, stridey, stridec, m_dst1, m_y1, m_uv, pspicinfo, rect, i+2);\
		convert_yuv(stridey, stridec, loadmed2, loadmedint2, store, spicblend, prefetch_1, adjustyuv);\
		m_dst1 += pitch;	m_y1 += stridey;	m_uv += stridec;\
		\
		createcmds(width, stridey, stridec, m_dst1, m_y1, m_uv, pspicinfo, rect, i+3);\
		convert_yuv(stridey, stridec, loadmed3, loadmedint3, store, spicblend, prefetch_2, adjustyuv);\
		m_dst1 += pitch;	m_y1 += stridey;	m_uv += stridec;\
	}\
	\
	for(; i<lines; i++)\
	{\
		createcmds(width, stridey, stridec, m_dst1, m_y1, m_uv, pspicinfo, rect, i);\
		convert_yuv(stridey, stridec, load11, load, store, spicblend, prefetch_1, adjustyuv);\
		if(i&1)\
		{\
			if((i%4)==3)\
				m_uv += stridec;\
			else\
				m_uv -= stridec;\
		}\
		else \
		{\
			m_uv += stridec;\
		}\
		m_y1 += stridey;\
		m_dst1 += pitch;\
	}\
}

// convert_median, centered on bottom field for interlaced picture
#define convert_median_bottom(i, width, stridey, stridec, pitch, lines, pspicinfo, rect, adjustyuv)\
{\
	DECLARE_CONVERTVARS;\
	\
	createcmds(width, stridey, stridec, m_dst1, m_y1, m_uv, pspicinfo, rect, i);\
	convert_yuv(stridey, stridec, loadn1, load, store, spicblend, prefetch_2, adjustyuv);\
	m_dst1 += pitch;	m_y1 += stridey;	m_uv += stridec;	i++;\
	\
	createcmds(width, stridey, stridec, m_dst1, m_y1, m_uv, pspicinfo, rect, i);\
	convert_yuv(stridey, stridec, loadmed0, loadmedint0, store, spicblend, prefetch_1, adjustyuv);\
	m_dst1 += pitch;	m_y1 += stridey;	m_uv -= stridec;	i++;\
	\
	createcmds(width, stridey, stridec, m_dst1, m_y1, m_uv, pspicinfo, rect, i);\
	convert_yuv(stridey, stridec, loadmed1, loadmedint0, store, spicblend, prefetch_2, adjustyuv);\
	m_dst1 += pitch;	m_y1 += stridey;	m_uv += stridec;	i++;\
	\
	createcmds(width, stridey, stridec, m_dst1, m_y1, m_uv, pspicinfo, rect, i);\
	convert_yuv(stridey, stridec, loadmed2, loadmedint2, store, spicblend, prefetch_1, adjustyuv);\
	m_dst1 += pitch;	m_y1 += stridey;	m_uv += stridec;	i++;\
	\
	createcmds(width, stridey, stridec, m_dst1, m_y1, m_uv, pspicinfo, rect, i);\
	convert_yuv(stridey, stridec, loadmed3, loadmedint3, store, spicblend, prefetch_2, adjustyuv);\
	m_dst1 += pitch;	m_y1 += stridey;	m_uv += stridec;	i++;\
	\
	for(; i<((lines-1)&~3); i+=4)\
	{\
		createcmds(width, stridey, stridec, m_dst1, m_y1, m_uv, pspicinfo, rect, i+0);\
		convert_yuv(stridey, stridec, loadmed0, loadmedint0, store, spicblend, prefetch_1, adjustyuv);\
		m_dst1 += pitch;	m_y1 += stridey;	m_uv -= stridec;\
		\
		createcmds(width, stridey, stridec, m_dst1, m_y1, m_uv, pspicinfo, rect, i+1);\
		convert_yuv(stridey, stridec, loadmed1, loadmedint1, store, spicblend, prefetch_2, adjustyuv);\
		m_dst1 += pitch;	m_y1 += stridey;	m_uv += stridec;\
		\
		createcmds(width, stridey, stridec, m_dst1, m_y1, m_uv, pspicinfo, rect, i+2);\
		convert_yuv(stridey, stridec, loadmed2, loadmedint2, store, spicblend, prefetch_1, adjustyuv);\
		m_dst1 += pitch;	m_y1 += stridey;	m_uv += stridec;\
		\
		createcmds(width, stridey, stridec, m_dst1, m_y1, m_uv, pspicinfo, rect, i+3);\
		convert_yuv(stridey, stridec, loadmed3, loadmedint3, store, spicblend, prefetch_2, adjustyuv);\
		m_dst1 += pitch;	m_y1 += stridey;	m_uv += stridec;\
	}\
	\
	for(; i<lines; i++)\
	{\
		createcmds(width, stridey, stridec, m_dst1, m_y1, m_uv, pspicinfo, rect, i);\
		convert_yuv(stridey, stridec, load11, load, store, spicblend, prefetch_1, adjustyuv);\
		if(i&1)\
		{\
			if((i%4)==3)\
				m_uv += stridec;\
			else\
				m_uv -= stridec;\
		}\
		else \
		{\
			m_uv += stridec;\
		}\
		m_y1 += stridey;\
		m_dst1 += pitch;\
	}\
}

#define convert_average(i, width, stridey, stridec, pitch, lines, pspicinfo, rect, adjustyuv)\
{\
	DECLARE_CONVERTVARS;\
	for(; i<(lines-1); i++)\
	{\
		createcmds(width, stridey, stridec, m_dst1, m_y1, m_uv, pspicinfo, rect, i);\
		convert_yuv(stridey, stridec, loadn1, load, store, spicblend, prefetch_2, adjustyuv);\
		if(i&1)\
		{\
			if((i%4)==3)\
				m_uv += stridec;\
			else\
				m_uv -= stridec;\
		}\
		else\
		{\
			m_uv += stridec;\
		}\
		m_y1 += stridey;\
		m_dst1 += pitch;\
	}\
	if(i==(lines-1))\
	{\
		createcmds(width, stridey, stridec, m_dst1, m_y1, m_uv, pspicinfo, rect, i);\
		convert_yuv(stridey, stridec, load11, load, store, spicblend, prefetch_1, adjustyuv);\
	}\
}

#define convert_bob_top(i, width, stridey, stridec, pitch, lines, pspicinfo, rect, adjustyuv)\
{\
	DECLARE_CONVERTVARS;\
	\
	for(; i<(lines-1); i++)\
	{\
		if(i&1)\
		{\
			if((i%4)==3)\
			{\
				m_uv += stridec;\
				createcmds(width, stridey, stridec, m_dst1, m_y1, m_uv, pspicinfo, rect, i);\
				convert_yuv(stridey, stridec, loadbob, loadbob, store, spicblend, prefetch_2, adjustyuv);\
				m_uv += stridec;\
			}\
			else \
			{\
				createcmds(width, stridey, stridec, m_dst1, m_y1, m_uv, pspicinfo, rect, i);\
				convert_yuv(stridey, stridec, loadbob, load, store, spicblend, prefetch_2, adjustyuv);\
			}\
		}\
		else \
		{\
			createcmds(width, stridey, stridec, m_dst1, m_y1, m_uv, pspicinfo, rect, i);\
			convert_yuv(stridey, stridec, load, load, store, spicblend, prefetch_1, adjustyuv);\
		}\
		m_y1 += stridey;\
		m_dst1 += pitch;\
	}\
	\
	if(i==(lines-1))\
	{\
		if(i&1)\
			m_uv += stridec;\
		createcmds(width, stridey, stridec, m_dst1, m_y1, m_uv, pspicinfo, rect, i);\
		convert_yuv(stridey, stridec, load11, load, store, spicblend, prefetch_1, adjustyuv);\
	}\
}

#define convert_bob_bottom(i, width, stridey, stridec, pitch, lines, pspicinfo, rect, adjustyuv)\
{\
	DECLARE_CONVERTVARS;\
	\
	createcmds(width, stridey, stridec, m_dst1, m_y1, m_uv, pspicinfo, rect, i);\
	convert_yuv(stridey, stridec, loadn1, load, store, spicblend, prefetch_2, adjustyuv);\
	m_y1 += stridey;	m_uv += stridec;	m_dst1 += pitch;	i++;\
	\
	for(; i<lines; i++)\
	{\
		if(i&1)\
		{\
			createcmds(width, stridey, stridec, m_dst1, m_y1, m_uv, pspicinfo, rect, i);\
			convert_yuv(stridey, stridec, load, load, store, spicblend, prefetch_1, adjustyuv);\
		}\
		else \
		{\
			if((i%4)==0)\
			{\
				m_uv += stridec;\
				createcmds(width, stridey, stridec, m_dst1, m_y1, m_uv, pspicinfo, rect, i);\
				convert_yuv(stridey, stridec, loadbob, loadbob, store, spicblend, prefetch_2, adjustyuv);\
				m_uv += stridec;\
			}\
			else \
			{\
				createcmds(width, stridey, stridec, m_dst1, m_y1, m_uv, pspicinfo, rect, i);\
				convert_yuv(stridey, stridec, loadbob, load, store, spicblend, prefetch_2, adjustyuv);\
			}\
		}\
		m_y1 += stridey;\
		m_dst1 += pitch;\
	}\
}

#define convert_weave_interlaced(i, width, stridey, stridec, pitch, lines, pspicinfo, rect, adjustyuv)\
{\
	DECLARE_CONVERTVARS;\
	for(; i<lines; i++)\
	{\
		createcmds(width, stridey, stridec, m_dst1, m_y1, m_uv, pspicinfo, rect, i);\
		convert_yuv(stridey, stridec, load, load, store, spicblend, prefetch_1, adjustyuv);\
		if(i&1)\
		{\
			if((i%4)==3)\
				m_uv += stridec;\
			else\
				m_uv -= stridec;\
		}\
		else\
		{\
			m_uv += stridec;\
		}\
		m_y1 += stridey;\
		m_dst1 += pitch;\
	}\
}

#define convert_weave_progressive(i, width, stridey, stridec, pitch, lines, pspicinfo, rect, adjustyuv)\
{\
	DECLARE_CONVERTVARS;\
	for(; i<lines; i++)\
	{\
		createcmds(width, stridey, stridec, m_dst1, m_y1, m_uv, pspicinfo, rect, i);\
		convert_yuv(stridey, stridec, load, load, store, spicblend, prefetch_1, adjustyuv);\
		if(i&1)\
			m_uv += stridec;\
		m_y1 += stridey;\
		m_dst1 += pitch;\
	}\
}


using namespace VideoProcessor;

CVideoProcessorYV12toYUY2::CVideoProcessorYV12toYUY2()
{
	m_dwNumForwardRefSamples = 0;
	m_dwNumBackwardRefSamples = 0;

	ZeroMemory(&m_VideoProcessorCaps, sizeof(m_VideoProcessorCaps));
	m_VideoProcessorCaps.dwSrcFormat = VIDEO_FORMAT_PRIVATE_YV12;
	m_VideoProcessorCaps.dwRenderTargetFormat = VIDEO_FORMAT_YUY2;
	m_VideoProcessorCaps.dwDeinterlaceTechnology = DEINTERLACE_TECH_BOBLineReplicate | DEINTERLACE_TECH_MedianFiltering;
	m_VideoProcessorCaps.dwProcAmpControlCaps = PROCAMP_CONTROL_BRIGHTNESS | PROCAMP_CONTROL_CONTRAST | PROCAMP_CONTROL_SATURATION | PROCAMP_CONTROL_HUE |
												PROCAMP_CONTROL_GAMMA | PROCAMP_CONTROL_CGAMMA | PROCAMP_CONTROL_SHARPNESS | PROCAMP_CONTROL_UOFFSET | PROCAMP_CONTROL_VOFFSET;
	m_VideoProcessorCaps.dwNoiseFilterTechnology = NOISEFILTER_TECH_UNSUPPORTED;
	m_VideoProcessorCaps.dwDetailFilterTechnology = DETAILFILTER_TECH_UNSUPPORTED;
	m_VideoProcessorCaps.dwVideoEffectOperations = VIDEOEFFECT_TECH_UNSUPPORTED;

	ZeroMemory(&m_VideoProcessorModes, sizeof(m_VideoProcessorModes));
	m_VideoProcessorModes.dwDeinterlaceTechnology = DEINTERLACE_TECH_MedianFiltering;
	m_VideoProcessorModes.dwDetailFilterTechnology = DETAILFILTER_TECH_UNSUPPORTED;
	m_VideoProcessorModes.dwNoiseFilterTechnology = NOISEFILTER_TECH_UNSUPPORTED;
	m_VideoProcessorModes.dwVideoEffectOperations = VIDEOEFFECT_TECH_UNSUPPORTED;

	CPU_LEVEL eCPU_Level = GetCPULevel();
	switch(eCPU_Level)
	{
	case CPU_LEVEL_NONE:
	case CPU_LEVEL_MMX:
	case CPU_LEVEL_SSE:
		m_bSSE2 = FALSE;
		break;
	case CPU_LEVEL_SSE2:
	case CPU_LEVEL_SSE3:
	default:
		m_bSSE2 = TRUE;
		break;
	}

	m_dwColorEffectLevel = 0;
	m_adjust_up = (unsigned char *)_mm_malloc(16, 16);
	m_adjust_down = (unsigned char *)_mm_malloc(16, 16);
	m_adjust_multiply_yuv = (short *)_mm_malloc(16, 16);
	m_adjust_offset_yuv = (short *)_mm_malloc(16, 16);
	m_adjust_hue_yuv = (short *)_mm_malloc(16, 16);
	m_adjust_gamma_yuv = (short *)_mm_malloc(16, 16);
	int i;
	for(i=0; i<8; i++)
	{
		m_adjust_up[i] = 0;
		m_adjust_down[i] = 0;
		m_adjust_multiply_yuv[i] = 1;
		m_adjust_offset_yuv[i] = 0;
		m_adjust_hue_yuv[i] = 0;
		m_adjust_gamma_yuv[i] = 0;
	}
	for (; i<16; i++)
	{
		m_adjust_up[i] = 0;
		m_adjust_down[i] = 0;
	}
	ZeroMemory(&m_ProcAmpValues, sizeof(m_ProcAmpValues));
}

CVideoProcessorYV12toYUY2::~CVideoProcessorYV12toYUY2()
{
	if (m_adjust_up)
	{
		_mm_free(m_adjust_up);
		m_adjust_up = NULL;
	}
	if (m_adjust_down)
	{
		_mm_free(m_adjust_down);
		m_adjust_down = NULL;
	}

	if (m_adjust_multiply_yuv)
	{
		_mm_free(m_adjust_multiply_yuv);
		m_adjust_multiply_yuv = NULL;
	}
	if (m_adjust_offset_yuv)
	{
		_mm_free(m_adjust_offset_yuv);
		m_adjust_offset_yuv = NULL;
	}
	if (m_adjust_hue_yuv)
	{
		_mm_free(m_adjust_hue_yuv);
		m_adjust_hue_yuv = NULL;
	}
	if (m_adjust_gamma_yuv)
	{
		_mm_free(m_adjust_gamma_yuv);
		m_adjust_gamma_yuv = NULL;
	}
}

STDMETHODIMP CVideoProcessorYV12toYUY2::QueryInterface(REFIID riid, void **ppv)
{
	this->AddRef();
	return CVideoProcessorBase::QueryInterface(riid, ppv);
}

STDMETHODIMP_(ULONG) CVideoProcessorYV12toYUY2::AddRef()
{
	LONG lRef = InterlockedIncrement(&m_cRef);
	assert(lRef > 0);
	return lRef;
}

STDMETHODIMP_(ULONG) CVideoProcessorYV12toYUY2::Release()
{
	LONG lRef = InterlockedDecrement(&m_cRef);
	assert(lRef >= 0);
	if (lRef == 0)
		delete this;
	return lRef;
}


HRESULT CVideoProcessorYV12toYUY2::_QueryVideoProcessorCaps(VideoProcessorCaps **pCaps)
{
	*pCaps = &m_VideoProcessorCaps;
	return S_OK;
}

HRESULT CVideoProcessorYV12toYUY2::_GetFilterPropertyRange(VIDEO_FILTER VideoFilter, FilterValueRange* pFilterRange)
{
	return E_NOTIMPL;
}

HRESULT CVideoProcessorYV12toYUY2::_GetProcAmpRange(PROCAMP_CONTROL ProcAmpControl, ProcAmpValueRange* pProcAmpRange)
{
	return E_NOTIMPL;
}

HRESULT CVideoProcessorYV12toYUY2::_SetVideoProcessorMode(const VideoProcessorModes *pVPMode)
{
	if (!pVPMode)
		E_POINTER;

	if (!(pVPMode->dwDeinterlaceTechnology&&m_VideoProcessorCaps.dwDeinterlaceTechnology) || 
		pVPMode->dwDetailFilterTechnology != DETAILFILTER_TECH_UNSUPPORTED ||
		pVPMode->dwNoiseFilterTechnology != NOISEFILTER_TECH_UNSUPPORTED ||
		pVPMode->dwVideoEffectOperations != VIDEOEFFECT_TECH_UNSUPPORTED)
		return E_NOTIMPL;

	// Set reference frame num
	switch(pVPMode->dwDeinterlaceTechnology)
	{
	case DEINTERLACE_TECH_BOBLineReplicate:
	case DEINTERLACE_TECH_BOBVerticalStretch:
	case DEINTERLACE_TECH_BOBVerticalStretch4Tap:
	case DEINTERLACE_TECH_MedianFiltering:
		m_dwNumForwardRefSamples = 0;
		m_dwNumBackwardRefSamples = 0;
		break;
	case DEINTERLACE_TECH_EdgeFiltering:
	case DEINTERLACE_TECH_FieldAdaptive:
	case DEINTERLACE_TECH_PixelAdaptive:
	case DEINTERLACE_TECH_MotionVectorSteered:
	case DEINTERLACE_TECH_InverseTelecine:
		m_dwNumForwardRefSamples = 0;
		m_dwNumBackwardRefSamples = 1;
		break;
	default:
		m_dwNumForwardRefSamples = 0;
		m_dwNumBackwardRefSamples = 0;
		break;
	}

	m_VideoProcessorModes = *pVPMode;
	return S_OK;
}

HRESULT CVideoProcessorYV12toYUY2::_GetVideoProcessorMode(VideoProcessorModes *pVPMode)
{
	*pVPMode = m_VideoProcessorModes;
	return S_OK;
}

HRESULT CVideoProcessorYV12toYUY2::_GetNumReferentSamples(DWORD	*pNumBackwardRefSamples,DWORD *pNumForwardRefSamples)
{
	*pNumBackwardRefSamples = m_dwNumBackwardRefSamples;
	*pNumForwardRefSamples  = m_dwNumForwardRefSamples;
	return S_OK;
}

HRESULT CVideoProcessorYV12toYUY2::_VideoProcessBlt(VideoBuffer *pRenderTarget, 
													const VideoProcessBltParams *pBltParams,
													const VideoSample *pVideoSamples,
													UINT uNumSamples)
{
	// Find processing VideoSample.
	const VideoSample *pVideoSample = NULL;
	for (UINT uIndex = 0; uIndex < uNumSamples; uIndex++)
	{
		if (pVideoSamples[uIndex].Surface.VideoFormat == VIDEO_FORMAT_PRIVATE_YV12 &&
			pBltParams->vtTargetFrame >= pVideoSamples[uIndex].vtStartTime && 
			pBltParams->vtTargetFrame < pVideoSamples[uIndex].vtEndTime)
		{
			pVideoSample = pVideoSamples+uIndex;
			break;
		}
	}
	if (pVideoSample == NULL)
		return E_FAIL; // TargetFrame time error.
	
	DWORD dwFrameStucture = pVideoSample->dwFrameStucture;

	// Find TargetFrame.
	TARGET_FRAME TargetFrame;
	if (dwFrameStucture == FRAME_STRUCTURE_PROGRESSIVE)
		TargetFrame = TARGET_FRAME_PROGRESSIVE;
	else // interlaced frame
	{
		if (pBltParams->vtTargetFrame < ((pVideoSample->vtStartTime + pVideoSample->vtEndTime)>>1))
			TargetFrame = (dwFrameStucture&FRAME_STRUCTURE_INTERLACED_EVEN_FIRST) ? TARGET_FRAME_TOP_FIELD : TARGET_FRAME_BOTTOM_FIELD;
		else if (pBltParams->vtTargetFrame >= ((pVideoSample->vtStartTime + pVideoSample->vtEndTime)>>1))
			TargetFrame = (dwFrameStucture&FRAME_STRUCTURE_INTERLACED_ODD_FIRST) ? TARGET_FRAME_TOP_FIELD : TARGET_FRAME_BOTTOM_FIELD;
	}

	// don't support subpic blending
	RECT rect; 
	rect.top = -1; 
	/*
	GetSpicinfo(&pspicinfo, &prect);
	rect = *prect;
	*/

	int nWidth		= pVideoSample->rcDisplayRect.right - pVideoSample->rcDisplayRect.left;
	int nHeight		= pVideoSample->rcDisplayRect.bottom - pVideoSample->rcDisplayRect.top;
	int nStride_Y	= pVideoSample->Surface.Video.iStride[0];
	int nStride_UV	= pVideoSample->Surface.Video.iStride[1];
	assert(pVideoSample->Surface.Video.iStride[1] == pVideoSample->Surface.Video.iStride[2]);
	int nRTPitch	= pRenderTarget->Video.iStride[0];
	m_y1			= (unsigned char *) pVideoSample->Surface.Video.pVideoPlane[0];
	m_uv			= (unsigned char *) pVideoSample->Surface.Video.pVideoPlane[1];
	m_dst1			= (unsigned char *) pRenderTarget->Video.pVideoPlane[0];
	m_voffs			= (int) (pVideoSample->Surface.Video.pVideoPlane[2] - pVideoSample->Surface.Video.pVideoPlane[1]);

	bool bDisplayWeaveMode;
	if (pBltParams->ProcessMode & PROCESS_MODE_FORCE_DEINTERLACING)
		bDisplayWeaveMode = false;
	else if (pBltParams->ProcessMode & PROCESS_MODE_FORCE_PROGRESSIVE)
		bDisplayWeaveMode = true;
	else //PROCESS_MODE_AUTO
		bDisplayWeaveMode = dwFrameStucture == FRAME_STRUCTURE_PROGRESSIVE || 
							nHeight == (pVideoSample->Surface.Video.iHeight[0]>>1); // Video frame down sample.

	DWORD	dwDeinterlaceTech = m_VideoProcessorModes.dwDeinterlaceTechnology;

	/* 
	// Video effect && Trim. All2HD
	if((mode&DISPREND_MODE_RENDER_TRIMENSION)==DISPREND_MODE_RENDER_TRIMENSION)
	{
		m_dst1 = (unsigned char *)f->dnm_surface[0]; 
	}
	*/

	SetProcAmpValues(&pBltParams->sProcAmpValues);

	mm_empty;
	int i = 0;
	if(m_dwColorEffectLevel == 2)
	{
		if(!bDisplayWeaveMode)
		{
			if(dwDeinterlaceTech == DEINTERLACE_TECH_MedianFiltering)
				convert_median_bcshg(i, nWidth, nStride_Y, nStride_UV, nRTPitch, nHeight, rect, TargetFrame);
			else if (dwDeinterlaceTech == DEINTERLACE_TECH_BOBLineReplicate)
				convert_bob_bcshg(i, nWidth, nStride_Y, nStride_UV, nRTPitch, nHeight, rect, TargetFrame);
			else
				convert_average_bcshg(i, nWidth, nStride_Y, nStride_UV, nRTPitch, nHeight, rect);
		}
		else
		{
			convert_weave_bcshg(i, nWidth, nStride_Y, nStride_UV, nRTPitch, nHeight, rect, TargetFrame);
		}
	}
	else if(m_dwColorEffectLevel == 1)
	{
		if(!bDisplayWeaveMode)
		{
			if(dwDeinterlaceTech == DEINTERLACE_TECH_MedianFiltering)
				convert_median_bcs(i, nWidth, nStride_Y, nStride_UV, nRTPitch, nHeight, rect, TargetFrame);
			else if (dwDeinterlaceTech == DEINTERLACE_TECH_BOBLineReplicate)
				convert_bob_bcs(i, nWidth, nStride_Y, nStride_UV, nRTPitch, nHeight, rect, TargetFrame);
			else
				convert_average_bcs(i, nWidth, nStride_Y, nStride_UV, nRTPitch, nHeight, rect);
		}
		else
		{
			convert_weave_bcs(i, nWidth, nStride_Y, nStride_UV, nRTPitch, nHeight, rect, TargetFrame);
		}
	}
	else
	{
		if(!bDisplayWeaveMode)
		{				
			if(dwDeinterlaceTech == DEINTERLACE_TECH_MedianFiltering)
				convert_median_b(i, nWidth, nStride_Y, nStride_UV, nRTPitch, nHeight, rect, TargetFrame);
			else if (dwDeinterlaceTech == DEINTERLACE_TECH_BOBLineReplicate)
				convert_bob_b(i, nWidth, nStride_Y, nStride_UV, nRTPitch, nHeight, rect, TargetFrame);
			else
				convert_average_b(i, nWidth, nStride_Y, nStride_UV, nRTPitch, nHeight, rect);
		}
		else
		{
			convert_weave_b(i, nWidth, nStride_Y, nStride_UV, nRTPitch, nHeight, rect, TargetFrame);
		}
	}

	return S_OK;
}


HRESULT CVideoProcessorYV12toYUY2::SetProcAmpValues(const ProcAmpValues* pProcAmpValues)
{
	if (m_ProcAmpValues.iBrightness == pProcAmpValues->iBrightness && 
		m_ProcAmpValues.iCGamma == pProcAmpValues->iCGamma && 
		m_ProcAmpValues.iContrast == pProcAmpValues->iContrast && 
		m_ProcAmpValues.iGamma == pProcAmpValues->iGamma && 
		m_ProcAmpValues.iHue == pProcAmpValues->iHue && 
		m_ProcAmpValues.iSaturation == pProcAmpValues->iSaturation && 
		m_ProcAmpValues.iSharpness == pProcAmpValues->iSharpness && 
		m_ProcAmpValues.iUOffset == pProcAmpValues->iUOffset && 
		m_ProcAmpValues.iVOffset == pProcAmpValues->iVOffset)
		return S_OK;

	int brightness, uoffset, voffset;	// level0
	int contrast, saturation;			// level1

	m_dwColorEffectLevel = 0;
	brightness = pProcAmpValues->iBrightness;
	uoffset = pProcAmpValues->iUOffset;
	voffset = pProcAmpValues->iVOffset;
	if(brightness>=0)
	{
		m_adjust_up[0] = m_adjust_up[2] = m_adjust_up[4] = m_adjust_up[6] = brightness;
		m_adjust_down[0] = m_adjust_down[2] = m_adjust_down[4] = m_adjust_down[6] = 0;
	}
	else
	{
		m_adjust_up[0] = m_adjust_up[2] = m_adjust_up[4] = m_adjust_up[6] = 0;
		m_adjust_down[0] = m_adjust_down[2] = m_adjust_down[4] = m_adjust_down[6] = -brightness;
	}

	if(uoffset>=0)
	{
		m_adjust_up[1] = m_adjust_up[5] = uoffset;
		m_adjust_down[1] = m_adjust_down[5] = 0;
	}
	else
	{
		m_adjust_up[1] = m_adjust_up[5] = 0;
		m_adjust_down[1] = m_adjust_down[5] = -uoffset;
	}

	if(voffset>=0)
	{
		m_adjust_up[3] = m_adjust_up[7] = voffset;
		m_adjust_down[3] = m_adjust_down[7] = 0;
	}
	else
	{
		m_adjust_up[3] = m_adjust_up[7] = 0;
		m_adjust_down[3] = m_adjust_down[7] = -voffset;
	}

	m_adjust_offset_yuv[0] = m_adjust_offset_yuv[2] = brightness+128;
	m_adjust_offset_yuv[1] = uoffset+128;
	m_adjust_offset_yuv[3] = voffset+128;
	contrast = pProcAmpValues->iContrast;
	saturation = pProcAmpValues->iSaturation;
	m_adjust_multiply_yuv[0] = m_adjust_multiply_yuv[2] = contrast;
	m_adjust_multiply_yuv[1] = m_adjust_multiply_yuv[3] = saturation;
	if(contrast!=256 || saturation!=256)
		m_dwColorEffectLevel = 1;

	if(m_bSSE2)
	{
		int hue, gamma, cgamma, sharpness;	// level2

		hue = pProcAmpValues->iHue;
		gamma = pProcAmpValues->iGamma;
		cgamma = pProcAmpValues->iCGamma;
		sharpness = pProcAmpValues->iSharpness;
		if(hue)
		{
			double dsin, dcos, drot;

			m_dwColorEffectLevel = 2;
			drot = (double)hue * (double)(2.0 * 3.14159265358979/GPI_VCC_RANGE);
			dsin = std::sin(drot);
			dcos = std::cos(drot);
			m_adjust_hue_yuv[1] = (short)(-dsin * m_adjust_multiply_yuv[3]);
			m_adjust_hue_yuv[3] = (short)(dsin * m_adjust_multiply_yuv[1]);
			m_adjust_multiply_yuv[1] = (short)(dcos * m_adjust_multiply_yuv[1]);
			m_adjust_multiply_yuv[3] = (short)(dcos * m_adjust_multiply_yuv[3]);
		}
		else
			m_adjust_hue_yuv[1] = m_adjust_hue_yuv[3] = 0;

		if(gamma)
		{
			m_dwColorEffectLevel = 2;
			gamma = -gamma*512/GPI_VCC_RANGE;	// scale gamma to -256 to +256 (squared output range is +-127).
			m_adjust_gamma_yuv[0] = m_adjust_gamma_yuv[2] = gamma;
			m_adjust_offset_yuv[0] -= gamma/4;	// readjust offset
			m_adjust_offset_yuv[2] -= gamma/4;
			m_adjust_multiply_yuv[0] -= gamma/2;// readjust slope
			m_adjust_multiply_yuv[2] -= gamma/2;
		}
		else
			m_adjust_gamma_yuv[0] = m_adjust_gamma_yuv[2] = 0;

		if(cgamma)
		{
			m_dwColorEffectLevel = 2;
			cgamma = -cgamma*512/GPI_VCC_RANGE;			// scale gamma to -256 to +256 (squared output range is +-127).
			m_adjust_gamma_yuv[1] = m_adjust_gamma_yuv[3] = cgamma;
			m_adjust_offset_yuv[1] -= cgamma/4;	// readjust offset
			m_adjust_offset_yuv[3] -= cgamma/4;
			m_adjust_multiply_yuv[1] -= cgamma/2;// readjust slope
			m_adjust_multiply_yuv[3] -= cgamma/2;
		}
		else
			m_adjust_gamma_yuv[1] = m_adjust_gamma_yuv[3] = 0;

		if(sharpness)
		{
			m_dwColorEffectLevel = 2;
			sharpness = sharpness*128/GPI_VCC_RANGE;		// scale sharpness to -64 to +64
			m_adjust_hue_yuv[0] = m_adjust_hue_yuv[2] = sharpness;
		}
		else
			m_adjust_hue_yuv[0] = m_adjust_hue_yuv[2] = 0;
	}

	if(m_bSSE2)
	{	// SSE2 uses double width registers so we copy low to high.
		mm_empty;
		((__m64 *)m_adjust_up)[1]			= ((__m64 *)m_adjust_up)[0];
		((__m64 *)m_adjust_down)[1]			= ((__m64 *)m_adjust_down)[0];
		((__m64 *)m_adjust_offset_yuv)[1]	= ((__m64 *)m_adjust_offset_yuv)[0];
		((__m64 *)m_adjust_multiply_yuv)[1]	= ((__m64 *)m_adjust_multiply_yuv)[0];
		((__m64 *)m_adjust_gamma_yuv)[1]	= ((__m64 *)m_adjust_gamma_yuv)[0];
		((__m64 *)m_adjust_hue_yuv)[1]		= ((__m64 *)m_adjust_hue_yuv)[0];
		mm_empty;
	}

	m_ProcAmpValues = *pProcAmpValues;

	return S_OK;
}

struct CSPICinfo
{
	unsigned long	flags;				// RETURN: status flags
	unsigned char	*pixel;				// RETURN: pixel array returned by GetSpic (short for YUY2, 16bpp, char for RGB 8bpp, 24bpp)
	unsigned char	*alpha;				// RETURN: alpha array returned by GetSpic
	unsigned long	*line_properties;	// RETURN: line properties
	unsigned long	*palette;			// RETURN: yuv palette used (16 entries) -- useful for 8bpp.
	bool			b_IsHD8bpp;
};


// optimize the following code even for debug
#if defined(_DEBUG) && !defined(__linux__)
#pragma optimize("gsw",on)
#endif

#define _m_movqc(src)	_m_punpcklbw(_m_from_int(*(DWORD *)(src)),_m_from_int(*(DWORD *)(src+m_voffs)))
#define _m_movdqc(src)	_mm_punpcklbw(_mm_movpi64_epi64(*(__m64*)(src)),_mm_movpi64_epi64(*(__m64*)(src+m_voffs)))
#define UV_BLOCKWIDTH 4

void CVideoProcessorYV12toYUY2::convert_weave_b(int &i, int width, int stridey, int stridec, int pitch, int lines, const RECT &rect, TARGET_FRAME TargetFrame)
{
	CSPICinfo *pspicinfo = NULL;//need remove

	if(TargetFrame == TARGET_FRAME_PROGRESSIVE)
	{
		convert_weave_progressive(i, width, stridey, stridec, pitch, lines, pspicinfo, rect, adjust_b);
#if 0
		__m128i mm_tmpin, mm_tmpout, mm_y_a, mm_uv_a;
		__m64 m_tmpin, m_tmpout, m_y_a, m_uv_a, m_y_b, m_uv_b;
		unsigned char *dst, *y, *uv, *spic, *alpha;
		bool b_IsHDSpic8bpp=false;
		int _i, _j, cmd, stop, line0, offs, cmdarray[4], stoparray[4];

		for(; i<lines; i++)
		{
			//createcmds(width, stridey, stridec, m_dst1, m_y1, m_uv, pspicinfo, rect, i);
			dst = m_dst1, y = m_y1, uv = m_uv;
			if(rect.top==-1 || i<rect.top || i>rect.bottom || pspicinfo->line_properties[line0 = i-rect.top]==0)
			{//no subpicture: single command to display everything
				cmdarray[0] = 1;
				stoparray[0] = width;
				cmdarray[1] = 0;
				stoparray[1] = 0;
			}
			else
			{//subpicture: three commands: display left, display spic, display right
				cmdarray[0] = 1;
				stoparray[0]	= pspicinfo->line_properties[line0] & 0xfff8;
				cmdarray[1] = 2;
				stoparray[1] = pspicinfo->line_properties[line0]>>16 & 0xfff8;
				cmdarray[2] = 1;
				stoparray[2] = min(width,stridey);
				cmdarray[3] = 0;
				stoparray[3] = 0;
				offs =  line0*(rect.right-rect.left+1) + (stoparray[0]-rect.left);
				spic = pspicinfo->pixel + (offs<<1);
				alpha = pspicinfo->alpha + offs;
				b_IsHDSpic8bpp = pspicinfo->b_IsHD8bpp;
			}

			//convert_yuv(stridey, stridec, load, load, store, spicblend, prefetch_1, adjust_b);
			for(_i=_j=0; (cmd=cmdarray[_i]); _i++)
			{
				stop = stoparray[_i];
				if(cmd==1)
				{
					if (!m_bSSE2 || (_j|stop)&8)
					{
						for(; _j<stop; _j+=8, y+=8, uv+=UV_BLOCKWIDTH, dst+=16)
						{
							loady_64(m_y_a, y, stridey);
							loadc_64(m_uv_a, uv, stridec);
							prefetch_1(y, stridey);
							prefetch_1(uv, stridec);
							m_tmpin = _m_punpcklbw(m_y_a, m_uv_a);
							adjust_byuv_64(m_tmpout, m_tmpin);
							store_64(dst, m_tmpout);
							m_tmpin = _m_punpckhbw(m_y_a, m_uv_a);
							adjust_byuv_64(m_tmpout, m_tmpin);
							store_64(dst+8, m_tmpout);
						}
					}
					else
					{
						for(; _j<stop; _j+=16, y+=16, uv+=(UV_BLOCKWIDTH<<1), dst+=32)
						{
							loady_128(mm_y_a, y, stridey);
							loadc_128(mm_uv_a, uv, stridec);
							prefetch_1(y, stridey);
							prefetch_1(uv, stridec);
							mm_tmpin = _mm_punpcklbw(mm_y_a, mm_uv_a);
							adjust_byuv_128(mm_tmpout, mm_tmpin);
							store_128(dst, mm_tmpout);
							mm_tmpin = _mm_punpckhbw(mm_y_a, mm_uv_a);
							adjust_byuv_128(mm_tmpout, mm_tmpin);
							store_128(dst+16, mm_tmpout);
						}
					}
				}
				else if(cmd==2)
				{
					m_uv_b = *(__m64 *)x0000;
					for(; _j<stop; _j+=8, y+=8, uv+=UV_BLOCKWIDTH, alpha+=8, spic+=16, dst+=16)
					{
						loady_64(m_y_a, y, stridey);
						loadc_64(m_uv_a, uv, stridec);
						prefetch_1(y, stridey);
						prefetch_1(uv, stridec);
						m_tmpin = _m_punpcklbw(m_y_a, m_uv_a);
						if(b_IsHDSpic8bpp)
						{
							if(alpha)
							{
								spicblendyuv_64_HD8bpp_AlphaAdd1(m_tmpin, alpha, spic, m_tmpout, m_y_b, m_uv_b, _m_punpcklbw)
									adjust_byuv_64(m_tmpout, m_tmpin);
							}
							else
							{
								spicblendyuv_64_HD8bpp(m_tmpin, alpha, spic, m_tmpout, m_y_b, m_uv_b, _m_punpcklbw)
									adjust_byuv_64(m_tmpout, m_tmpin);
							}
						}
						else
						{
							spicblendyuv_64(m_tmpin, alpha, spic, m_tmpout, m_y_b, m_uv_b, _m_punpcklbw)
								adjust_byuv_64(m_tmpout, m_tmpin);
						}
						store_64(dst, m_tmpout);
						m_tmpin = _m_punpckhbw(m_y_a, m_uv_a);
						if(b_IsHDSpic8bpp)
						{
							if(alpha)
							{
								spicblendyuv_64_HD8bpp_AlphaAdd1(m_tmpin, alpha, spic+8, m_tmpout, m_y_b, m_uv_b, _m_punpckhbw)
									adjust_byuv_64(m_tmpout, m_tmpin);
							}
							else
							{
								spicblendyuv_64_HD8bpp(m_tmpin, alpha, spic+8, m_tmpout, m_y_b, m_uv_b, _m_punpckhbw)
									adjust_byuv_64(m_tmpout, m_tmpin);
							}
						}
						else
						{
							spicblendyuv_64(m_tmpin, alpha, spic+8, m_tmpout, m_y_b, m_uv_b, _m_punpckhbw)
								adjust_byuv_64(m_tmpout, m_tmpin);
						}
						store_64(dst+8, m_tmpout);
					}
				}
			}

			if(i&1)
				m_uv += stridec;
			m_y1 += stridey;
			m_dst1 += pitch;
		}
#endif
	}
	else
	{
		convert_weave_interlaced(i, width, stridey, stridec, pitch, lines, pspicinfo, rect, adjust_b);
	}

	mm_empty;
}

void CVideoProcessorYV12toYUY2::convert_weave_bcs(int &i, int width, int stridey, int stridec, int pitch, int lines, const RECT &rect, TARGET_FRAME TargetFrame)
{
	CSPICinfo *pspicinfo = NULL;//need remove

	if(TargetFrame == TARGET_FRAME_PROGRESSIVE)
	{
		convert_weave_progressive(i, width, stridey, stridec, pitch, lines, pspicinfo, rect, adjust_bcs);
	}
	else
	{
		convert_weave_interlaced(i, width, stridey, stridec, pitch, lines, pspicinfo, rect, adjust_bcs);
	}

	mm_empty;
}

void CVideoProcessorYV12toYUY2::convert_weave_bcshg(int &i, int width, int stridey, int stridec, int pitch, int lines, const RECT &rect, TARGET_FRAME TargetFrame)
{
	CSPICinfo *pspicinfo = NULL;//need remove

	if(TargetFrame == TARGET_FRAME_PROGRESSIVE)
	{
		convert_weave_progressive(i, width, stridey, stridec, pitch, lines, pspicinfo, rect, adjust_bcshg);
	}
	else
	{
		convert_weave_interlaced(i, width, stridey, stridec, pitch, lines, pspicinfo, rect, adjust_bcshg);
	}

	mm_empty;
}

void CVideoProcessorYV12toYUY2::convert_bob_b(int &i, int width, int stridey, int stridec, int pitch, int lines, const RECT &rect, TARGET_FRAME TargetFrame)
{
	CSPICinfo *pspicinfo = NULL;//need remove

	if(TargetFrame == TARGET_FRAME_BOTTOM_FIELD)
	{
		convert_bob_bottom(i, width, stridey, stridec, pitch, lines, pspicinfo, rect, adjust_b);
	}
	else
	{
		convert_bob_top(i, width, stridey, stridec, pitch, lines, pspicinfo, rect, adjust_b);
	}

	mm_empty;
}

void CVideoProcessorYV12toYUY2::convert_bob_bcs(int &i, int width, int stridey, int stridec, int pitch, int lines, const RECT &rect, TARGET_FRAME TargetFrame)
{
	CSPICinfo *pspicinfo = NULL;//need remove

	if(TargetFrame == TARGET_FRAME_BOTTOM_FIELD)
	{
		convert_bob_bottom(i, width, stridey, stridec, pitch, lines, pspicinfo, rect, adjust_bcs);
	}
	else
	{
		convert_bob_top(i, width, stridey, stridec, pitch, lines, pspicinfo, rect, adjust_bcs);
	}

	mm_empty;
}

void CVideoProcessorYV12toYUY2::convert_bob_bcshg(int &i, int width, int stridey, int stridec, int pitch, int lines, const RECT &rect, TARGET_FRAME TargetFrame)
{
	CSPICinfo *pspicinfo = NULL;//need remove

	if(TargetFrame == TARGET_FRAME_BOTTOM_FIELD)
	{
		convert_bob_bottom(i, width, stridey, stridec, pitch, lines, pspicinfo, rect, adjust_bcshg);
	}
	else
	{
		convert_bob_top(i, width, stridey, stridec, pitch, lines, pspicinfo, rect, adjust_bcshg);
	}

	mm_empty;
}

void CVideoProcessorYV12toYUY2::convert_average_b(int &i, int width, int stridey, int stridec, int pitch, int lines, const RECT &rect)
{
	CSPICinfo *pspicinfo = NULL;//need remove

	convert_average(i, width, stridey, stridec, pitch, lines, pspicinfo, rect, adjust_b);

	mm_empty;
}

void CVideoProcessorYV12toYUY2::convert_average_bcs(int &i, int width, int stridey, int stridec, int pitch, int lines, const RECT &rect)
{
	CSPICinfo *pspicinfo = NULL;//need remove

	convert_average(i, width, stridey, stridec, pitch, lines, pspicinfo, rect, adjust_bcs);

	mm_empty;
}

void CVideoProcessorYV12toYUY2::convert_average_bcshg(int &i, int width, int stridey, int stridec, int pitch, int lines, const RECT &rect)
{
	CSPICinfo *pspicinfo = NULL;//need remove

	convert_average(i, width, stridey, stridec, pitch, lines, pspicinfo, rect, adjust_bcshg);

	mm_empty;
}

void CVideoProcessorYV12toYUY2::convert_median_b(int &i, int width, int stridey, int stridec, int pitch, int lines, const RECT &rect, TARGET_FRAME TargetFrame)
{
	CSPICinfo *pspicinfo = NULL;//need remove

	if(TargetFrame == TARGET_FRAME_BOTTOM_FIELD)
	{
		convert_median_bottom(i, width, stridey, stridec, pitch, lines, pspicinfo, rect, adjust_b);
	}
	else
	{
		convert_median_top(i, width, stridey, stridec, pitch, lines, pspicinfo, rect, adjust_b);
	}

	mm_empty;
}

void CVideoProcessorYV12toYUY2::convert_median_bcs(int &i, int width, int stridey, int stridec, int pitch, int lines, const RECT &rect, TARGET_FRAME TargetFrame)
{
	CSPICinfo *pspicinfo = NULL;//need remove

	if(TargetFrame == TARGET_FRAME_BOTTOM_FIELD)
	{
		convert_median_bottom(i, width, stridey, stridec, pitch, lines, pspicinfo, rect, adjust_bcs);
	}
	else
	{
		convert_median_top(i, width, stridey, stridec, pitch, lines, pspicinfo, rect, adjust_bcs);
	}

	mm_empty;
}

void CVideoProcessorYV12toYUY2::convert_median_bcshg(int &i, int width, int stridey, int stridec, int pitch, int lines, const RECT &rect, TARGET_FRAME TargetFrame)
{
	CSPICinfo *pspicinfo = NULL;//need remove

	if(TargetFrame == TARGET_FRAME_BOTTOM_FIELD)
	{
		convert_median_bottom(i, width, stridey, stridec, pitch, lines, pspicinfo, rect, adjust_bcshg);
	}
	else
	{
		convert_median_top(i, width, stridey, stridec, pitch, lines, pspicinfo, rect, adjust_bcshg);
	}

	mm_empty;
}

#undef _m_movqc
#undef _m_movdqc
#undef UV_BLOCKWIDTH

#if defined(_DEBUG) && !defined(__linux__)
#pragma optimize("",on)
#endif
