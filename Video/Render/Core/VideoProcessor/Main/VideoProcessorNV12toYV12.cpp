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
// VideoProcessorNV12toYV12.cpp: implementation of the display renderer for YV12 surfaces
//
//////////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <windows.h>
#include <assert.h>
#include <streams.h>
#include <cmath>
#include "VideoProcessorNV12toYV12.h"
#include "VideoProcessorIntrinsics.h"

#if defined (__linux__) && !defined (_USE_MSASM)
static const __int64 ALIGN(16,x0000[2]) = {0LL, 0LL},
x8000[2] = {0x8000800080008000LL,0x8000800080008000LL},
x00ff[2] = {0x00ff00ff00ff00ffLL,0x00ff00ff00ff00ffLL},
x1010[2] = {0x1010101010101010LL,0x1010101010101010LL};
#elif defined (_USE_MSASM)
static const __int64 ALIGN(16,x0000[2])= {0,0},
x8000[2] = {0x8000800080008000,0x8000800080008000},
x00ff[2] = {0x00ff00ff00ff00ff,0x00ff00ff00ff00ff},
x1010[2] = {0x1010101010101010,0x1010101010101010},
_xfe[2]  = {0xfefefefefefefefe,0xfefefefefefefefe};
#else
static const __int64 ALIGN(16,x0000[2]),
x8000[2] = {0x8000800080008000,0x8000800080008000},
x00ff[2] = {0x00ff00ff00ff00ff,0x00ff00ff00ff00ff},
x1010[2] = {0x1010101010101010,0x1010101010101010},
_xfe[2]  = {0xfefefefefefefefe,0xfefefefefefefefe};
#endif

#define DECLARE_CONVERTVARS\
	__m128i mm_tmpout, mm_y_a;\
	__m64 m_tmpin, m_tmpout, m_y_a, m_uv_a, m_y_b, m_uv_b, m_zero = *(__m64 *)x0000;\
	unsigned char *dst, *dst_u, *dst_v, *dst_u_tmp, *dst_v_tmp, *src, *srcu, *srcv, *spic, *alpha;\
	bool b_IsHDSpic8bpp=false;\
	int _i, _j, cmd, stop, left, right, cmdarray[4], stoparray[4];

#define createcmds_y(width, stridey, stridec, dst_addr, src_addr, sp_addr, a_addr, info)\
{\
	dst = dst_addr, src = src_addr, spic = sp_addr, alpha = a_addr;\
	if(!spic || !alpha || info==0) \
	{/*no subpicture: single command to display everything*/\
		cmdarray[0] = 1;\
		stoparray[0] = width;\
		cmdarray[1] = 0;\
		stoparray[1] = 0;\
	}\
	else\
	{/*subpicture: three commands: display left, display spic, display right*/\
		cmdarray[0] = 1;\
		stoparray[0] = info & 0xfff8;\
		cmdarray[1] = 2;\
		stoparray[1] = (info>>16 & 0xfff8)+8;\
		cmdarray[2] = 1;\
		stoparray[2] = stridey;\
		cmdarray[3] = 0;\
		stoparray[3] = 0;\
		b_IsHDSpic8bpp = m_bHDspic;\
	}\
}

#define createcmds_uv(width, dstu_addr, dstv_addr, srcu_addr, srcv_addr, sp_addr, a_addr, info)\
{\
	dst_u = dstu_addr, dst_v = dstv_addr;\
	dst_u_tmp = m_u_tmp_buf;\
	dst_v_tmp = m_v_tmp_buf;\
	srcu = srcu_addr, srcv = srcv_addr;\
	spic = sp_addr, alpha = a_addr;\
	if(spic && alpha)\
		left = info&0xfff8, right = (((info>>16)+1) & 0xfff8);\
	else\
		left = -1, right = -1;\
	b_IsHDSpic8bpp = m_bHDspic;\
}

#define convert_y(stridey, loady, store, spicblend, prefetch, adjusty)\
{\
	for(_i=_j=0; (cmd=cmdarray[_i]); _i++)\
	{\
		stop = stoparray[_i];\
		if(cmd==1)\
		{\
			if(!m_bSSE2 || (_j|stop)&8)\
			{\
				for(; _j<stop; _j+=8, src+=8, dst+=8)\
				{\
					loady##y##_64(m_y_a, src, stridey);\
					prefetch(src, stridey);\
					adjusty##_64(m_tmpout, m_y_a);\
					store##_64(dst, m_tmpout);\
				}\
			}\
			else\
			{\
				for(; _j<stop; _j+=16, src+=16, dst+=16)\
				{\
					loady##y##_128(mm_y_a, src, stridey);\
					prefetch(src, stridey);\
					adjusty##_128(mm_tmpout, mm_y_a);\
					store##_128(dst, mm_tmpout);\
				}\
			}\
		}\
		else if(cmd==2)\
		{\
			for(; _j<stop; _j+=8, src+=8, dst+=8)\
			{\
				loady##y##_64(m_y_a, src, stridey);\
				prefetch(src, stridey);\
				if(b_IsHDSpic8bpp)\
				{\
					spicblend##yv12_64HDAlphaPlus1(m_y_a, alpha+_j, spic+_j, m_tmpout, m_y_b, m_zero)\
					adjusty##_64(m_tmpout, m_y_a);\
				}\
				else\
				{\
					spicblend##yv12_64(m_y_a, alpha+_j, spic+_j, m_tmpout, m_y_b, m_zero)\
					adjusty##_64(m_tmpout, m_y_a);\
				}\
				store##_64(dst, m_tmpout);\
			}\
		}\
	}\
}

#define convert_uv(width, stridec, loaduv, store, spicblend, prefetch, adjustuv)\
{\
	for(_j=0; _j<width; _j+=16, srcu+=UV_BLOCKWIDTH, srcv+=UV_BLOCKWIDTH, dst_u_tmp+=8, dst_v_tmp+=8)\
	{\
		loaduv##y##_64(m_uv_a, srcu, stridec);\
		loaduv##y##_64(m_uv_b, srcv, stridec);\
		prefetch(srcu, stridec);\
		prefetch(srcv, stridec);\
		\
		if(_j<right && _j>=left)\
		{\
			if(b_IsHDSpic8bpp)\
			{\
				spicblend##yv12_64HDAlphaPlus1(m_uv_a, alpha+_j, spic+_j, m_tmpout, m_y_b, m_zero)\
			}\
			else\
			{\
				spicblend##yv12_64(m_uv_a, alpha+_j, spic+_j, m_tmpout, m_y_b, m_zero)\
			}\
		}\
		if(_j<(right-8) && _j>=(left-8))\
		{\
			if(b_IsHDSpic8bpp)\
			{\
				spicblend##yv12_64HDAlphaPlus1(m_uv_b, alpha+_j+8, spic+_j+8, m_tmpout, m_y_b, m_zero)\
			}\
			else\
			{\
				spicblend##yv12_64(m_uv_b, alpha+_j+8, spic+_j+8, m_tmpout, m_y_b, m_zero)\
			}\
		}\
		\
		adjustuv##_64(m_tmpin, m_uv_a);\
		adjustuv##_64(m_tmpout, m_uv_b);\
		m_y_a = _m_psrlwi(m_tmpin,8);\
		m_uv_a = _m_pand(m_tmpin,*(__m64*)x00ff);\
		m_y_b = _m_psrlwi(m_tmpout,8);\
		m_uv_b = _m_pand(m_tmpout,*(__m64*)x00ff);\
		m_tmpout = _m_packuswb(m_uv_a, m_uv_b);\
		store##_64(dst_u_tmp, m_tmpout);\
		m_tmpout = _m_packuswb(m_y_a, m_y_b);\
		store##_64(dst_v_tmp, m_tmpout);\
	}\
	\
	memcpy(dst_u, m_u_tmp_buf, width>>1);\
	memcpy(dst_v, m_v_tmp_buf, width>>1);\
	\
	/*Optimize memory copy with SSE2 - Note: all buffer must align 16bytes.*/\
	/*int			  length = (width>>1);\
	unsigned char *tdst1 = dst_u, *tsrc1 = m_u_tmp_buf;\
	unsigned char *tdst2 = dst_v, *tsrc2 = m_v_tmp_buf;\
	if(m_bSSE2)\
	{\
		int _k=0;\
		for(; (length-_k)>=16; _k+=16)\
		{\
			*(__m128i*)(tdst1) = *(__m128i*)(tsrc1);\
			*(__m128i*)(tdst2) = *(__m128i*)(tsrc2);\
			tdst1+=16; tsrc1+=16;\
			tdst2+=16; tsrc2+=16;\
		}\
		if(_k < length)\
		{\
			memcpy(tdst1, tsrc1, length-_k);\
			memcpy(tdst2, tsrc2, length-_k);\
		}\
	}\
	else if(m_bSSE)\
	{\
		int _k=0;\
		for(; (length-_k)>=8; _k+=8)\
		{\
			*(__m64*)(tdst1) = *(__m64*)(tsrc1);\
			*(__m64*)(tdst2) = *(__m64*)(tsrc2);\
			tdst1+=8; tsrc1+=8;\
			tdst2+=8; tsrc2+=8;\
		}\
		if(_k < length)\
		{\
			memcpy(tdst1, tsrc1, length-_k);\
			memcpy(tdst2, tsrc2, length-_k);\
		}\
	}\
	else\
	{\
		memcpy(tdst1, tsrc1, length);\
		memcpy(tdst2, tsrc2, length);\
	}*/\
}


// convert_median, centered on top field
#define convert_median_top(i, width, stridey, stridec, pitch, lines, adjust, dwSpicWidth)\
{\
	DECLARE_CONVERTVARS;\
	for(; i<lines; i++)\
	{\
		createcmds_y(width, stridey, stridec, dsty, m_y1, pSpicY, pSpicA, pSpicProp[i]);\
		if((i&1)==0)\
		{\
			convert_y(width, load, store, spicblend, prefetch_1, adjust##y);\
			createcmds_uv(width, dstu, dstv, m_u, m_v, pSpicC, pSpicAC, pSpicProp[i]);\
			if((i&3)==0)\
			{\
				convert_uv(width, stridec, load, store, spicblend, prefetch_1, adjust##uv);\
			}\
			else\
			{\
				convert_uv(width, stridec, loadmed, store, spicblend, prefetch_1, adjust##uv);\
			}\
			m_u += stridec;\
			m_v += stridec;\
			dstu += pitch>>1;\
			dstv += pitch>>1;\
			/*pSpicC += dwSpicWidth;*/\
			/*pSpicAC += dwSpicWidth;*/\
		}\
		else\
		{\
			convert_y(width, loadmed, store, spicblend, prefetch_1, adjust##y);\
		}\
		m_y1 += stridey;\
		dsty += pitch;\
		/*pSpicY += dwSpicWidth;*/\
		/*pSpicA += dwSpicWidth;*/\
	}\
}

// convert_median, centered on bottom field
#define convert_median_bottom(i, width, stridey, stridec, pitch, lines, adjust, dwSpicWidth)\
{\
	DECLARE_CONVERTVARS;\
	if(i==0 && i<lines)\
	{\
		createcmds_y(width, stridey, stridec, dsty, m_y1, pSpicY, pSpicA, pSpicProp[i]);\
		convert_y(width, load, store, spicblend, prefetch_1, adjust##y);\
		m_y1 += stridey;\
		dsty += pitch;\
		/*pSpicY += dwSpicWidth;*/\
		/*pSpicA += dwSpicWidth;*/\
		i++;\
	}\
	for(; i<lines; i++)\
	{\
		createcmds_y(width, stridey, stridec, dsty, m_y1, pSpicY, pSpicA, pSpicProp[i]);\
		if((i&1))\
		{\
			convert_y(width, load, store, spicblend, prefetch_1, adjust##y);\
			createcmds_uv(width, dstu, dstv, m_u, m_v, pSpicC, pSpicAC, pSpicProp[i]);\
			if((i&3)==3)\
			{\
				convert_uv(width, stridec, load, store, spicblend, prefetch_1, adjust##uv);\
			}\
			else\
			{\
				convert_uv(width, stridec, loadmed, store, spicblend, prefetch_1, adjust##uv);\
			}\
			m_u += stridec;\
			m_v += stridec;\
			dstu += pitch>>1;\
			dstv += pitch>>1;\
			/*pSpicC += dwSpicWidth;*/\
			/*pSpicAC += dwSpicWidth;*/\
		}\
		else\
		{\
			convert_y(width, loadmed, store, spicblend, prefetch_1, adjust##y);\
		}\
		m_y1 += stridey;\
		dsty += pitch;\
		/*pSpicY += dwSpicWidth;*/\
		/*pSpicA += dwSpicWidth;*/\
	}\
}

#define convert_average(i, width, stridey, stridec, pitch, lines, adjust, dwSpicWidth)\
{\
	DECLARE_CONVERTVARS;\
	if(i==0 && i<lines)\
	{\
		createcmds_y(width, stridey, stridec, dsty, m_y1, pSpicY, pSpicA, pSpicProp[i]);\
		convert_y(width, load, store, spicblend, prefetch_1, adjust##y);\
		m_y1 += stridey;\
		dsty += pitch;\
		/*pSpicY += dwSpicWidth;*/\
		/*pSpicA += dwSpicWidth;*/\
		createcmds_uv(width, dstu, dstv, m_u, m_v, pSpicC, pSpicAC, pSpicProp[i]);\
		convert_uv(width, stridec, load, store, spicblend, prefetch_1, adjust##uv);\
		m_u += stridec;\
		m_v += stridec;\
		dstu += pitch>>1;\
		dstv += pitch>>1;\
		/*pSpicC += dwSpicWidth;*/\
		/*pSpicAC += dwSpicWidth;*/\
		i++;\
	}\
	for(; i<lines; i++)\
	{\
		createcmds_y(width, stridey, stridec, dsty, m_y1, pSpicY, pSpicA, pSpicProp[i]);\
		convert_y(width, load11, store, spicblend, prefetch_1, adjust##y);\
		m_y1 += stridey;\
		dsty += pitch;\
		/*pSpicY += dwSpicWidth;*/\
		/*pSpicA += dwSpicWidth;*/\
		if((i&1)==0)\
		{\
			createcmds_uv(width, dstu, dstv, m_u, m_v, pSpicC, pSpicAC, pSpicProp[i]);\
			convert_uv(width, stridec, load11, store, spicblend, prefetch_1, adjust##uv);\
			m_u += stridec;\
			m_v += stridec;\
			dstu += pitch>>1;\
			dstv += pitch>>1;\
			/*pSpicC += dwSpicWidth;*/\
			/*pSpicAC += dwSpicWidth;*/\
		}\
	}\
}

#define convert_weave(i, width, stridey, stridec, pitch, lines, adjust, dwSpicWidth)\
{\
	DECLARE_CONVERTVARS;\
	for(; i<lines; i++)\
	{\
		createcmds_y(width, stridey, stridec, dsty, m_y1, pSpicY, pSpicA, pSpicProp[i]);\
		convert_y(width, load, store, spicblend, prefetch_1, adjust##y);\
		m_y1 += stridey;\
		dsty += pitch;\
		/*pSpicY += dwSpicWidth;*/\
		/*pSpicA += dwSpicWidth;*/\
		if((i&1)==0)\
		{\
			createcmds_uv(width, dstu, dstv, m_u, m_v, pSpicC, pSpicAC, pSpicProp[i]);\
			convert_uv(width, stridec, load, store, spicblend, prefetch_1, adjust##uv);\
			m_u += stridec;\
			m_v += stridec;\
			dstu += pitch>>1;\
			dstv += pitch>>1;\
			/*pSpicC += dwSpicWidth;*/\
			/*pSpicAC += dwSpicWidth;*/\
		}\
	}\
}

#define convert_average_simple(i, width, stridey, stridec, pitch, lines, adjust, dwSpicWidth)\
{\
	DECLARE_SIMPLECONVERTVARS;\
	if(i==0 && i<lines)\
	{\
		createcmds_y(width, stridey, stridec, dsty, m_y1, pSpicY, pSpicA, pSpicProp[i]);\
		convert_y_simple(width, load, store, spicblend, prefetch_1, adjust##y);\
		m_y1 += stridey;\
		dsty += pitch;\
		/*pSpicY += dwSpicWidth;*/\
		/*pSpicA += dwSpicWidth;*/\
		createcmds_uv(width, dstu, dstv, m_u, m_v, pSpicC, pSpicAC, pSpicProp[i]);\
		convert_uv(width, stridec, load, store, spicblend, prefetch_1, adjust##uv);\
		m_u += stridec;\
		m_v += stridec;\
		dstu += pitch>>1;\
		dstv += pitch>>1;\
		/*pSpicC += dwSpicWidth;*/\
		/*pSpicAC += dwSpicWidth;*/\
		i++;\
	}\
	for(; i<lines; i++)\
	{\
		createcmds_y(width, stridey, stridec, dsty, m_y1, pSpicY, pSpicA, pSpicProp[i]);\
		convert_y_simple(width, load11_alt, store, spicblend, prefetch_1, adjust##y);\
		m_y1 += stridey;\
		dsty += pitch;\
		/*pSpicY += dwSpicWidth;*/\
		/*pSpicA += dwSpicWidth;*/\
		if((i&1)==0)\
		{\
			createcmds_uv(width, dstu, dstv, m_u, m_v, pSpicC, pSpicAC, pSpicProp[i]);\
			convert_uv(width, stridec, load11, store, spicblend, prefetch_1, adjust##uv);\
			m_u += stridec;\
			m_v += stridec;\
			dstu += pitch>>1;\
			dstv += pitch>>1;\
			/*pSpicC += dwSpicWidth;*/\
			/*pSpicAC += dwSpicWidth;*/\
		}\
	}\
}

using namespace VideoProcessor;

CVideoProcessorNV12toYV12::CVideoProcessorNV12toYV12()
{
	m_dwNumForwardRefSamples = 0;
	m_dwNumBackwardRefSamples = 0;

	ZeroMemory(&m_VideoProcessorCaps, sizeof(m_VideoProcessorCaps));
	m_VideoProcessorCaps.dwSrcFormat = VIDEO_FORMAT_PRIVATE_NV12;
	m_VideoProcessorCaps.dwRenderTargetFormat = VIDEO_FORMAT_YV12;
	m_VideoProcessorCaps.dwDeinterlaceTechnology = DEINTERLACE_TECH_MedianFiltering;
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
	m_u_tmp_buf =			(unsigned char *) _mm_malloc(1920,16);
	m_v_tmp_buf =			(unsigned char *) _mm_malloc(1920,16);
	m_adjust_up_y =	     	(unsigned char *) _mm_malloc(16,16);
	m_adjust_down_y =		(unsigned char *) _mm_malloc(16,16);
	m_adjust_up_uv =		(unsigned char *) _mm_malloc(16,16);
	m_adjust_down_uv =		(unsigned char *) _mm_malloc(16,16);
	m_adjust_multiply_y =	(short *) _mm_malloc(16,16);
	m_adjust_multiply_uv =  (short *) _mm_malloc(16,16);
	m_adjust_offset_y =		(short *) _mm_malloc(16,16);
	m_adjust_offset_uv =	(short *) _mm_malloc(16,16);
	m_adjust_gamma_y =		(short *) _mm_malloc(16,16);
	m_adjust_hue_uv =		(short *) _mm_malloc(16,16);
	memset(m_u_tmp_buf, 0, 1920);
	memset(m_v_tmp_buf, 0, 1920);
	memset(m_adjust_up_y,0,16);
	memset(m_adjust_down_y,0,16);
	memset(m_adjust_up_uv,0,16);
	memset(m_adjust_down_uv,0,16);
	int tmp = sizeof(short)*8;
	memset(m_adjust_multiply_y,0,tmp);
	memset(m_adjust_multiply_uv,0,tmp);
	memset(m_adjust_offset_y,0,tmp);
	memset(m_adjust_offset_uv,0,tmp);
	memset(m_adjust_gamma_y,0,tmp);
	memset(m_adjust_hue_uv,0,tmp);

	ZeroMemory(&m_ProcAmpValues, sizeof(m_ProcAmpValues));

	// for YV12 subpictures
	m_bHDspic		= false;
	m_pSpicY		= NULL;
	m_pSpicC		= NULL;
	m_pSpicA		= NULL;		// alphas for Y
	m_pSpicAC		= NULL;		// alphas for C
	m_pSpicProp		= NULL;
	m_dwSpicWidth	= 0;
	m_dwSpicHeight	= 0;

}

CVideoProcessorNV12toYV12::~CVideoProcessorNV12toYV12()
{
	_mm_free(m_u_tmp_buf);
	m_u_tmp_buf = 0;
	_mm_free(m_v_tmp_buf);
	m_v_tmp_buf = 0;
	_mm_free(m_adjust_up_y);
	m_adjust_up_y = 0;
	_mm_free(m_adjust_down_y);
	m_adjust_down_y = 0;
	_mm_free(m_adjust_up_uv);
	m_adjust_up_uv = 0;
	_mm_free(m_adjust_down_uv);
	m_adjust_down_uv = 0;

	_mm_free(m_adjust_multiply_y);
	m_adjust_multiply_y = 0;
	_mm_free(m_adjust_multiply_uv);
	m_adjust_multiply_uv = 0;
	_mm_free(m_adjust_offset_y);
	m_adjust_offset_y = 0;
	_mm_free(m_adjust_offset_uv);
	m_adjust_offset_uv = 0;
	_mm_free(m_adjust_gamma_y);
	m_adjust_gamma_y = 0;
	_mm_free(m_adjust_hue_uv);
	m_adjust_hue_uv = 0;
}

STDMETHODIMP CVideoProcessorNV12toYV12::QueryInterface(REFIID riid, void **ppv)
{
	this->AddRef();
	return CVideoProcessorBase::QueryInterface(riid, ppv);
}

STDMETHODIMP_(ULONG) CVideoProcessorNV12toYV12::AddRef()
{
	LONG lRef = InterlockedIncrement(&m_cRef);
	assert(lRef > 0);
	return lRef;
}

STDMETHODIMP_(ULONG) CVideoProcessorNV12toYV12::Release()
{
	LONG lRef = InterlockedDecrement(&m_cRef);
	assert(lRef >= 0);
	if (lRef == 0)
		delete this;
	return lRef;
}


HRESULT CVideoProcessorNV12toYV12::_QueryVideoProcessorCaps(VideoProcessorCaps **pCaps)
{
	*pCaps = &m_VideoProcessorCaps;
	return S_OK;
}

HRESULT CVideoProcessorNV12toYV12::_GetFilterPropertyRange(VIDEO_FILTER VideoFilter, FilterValueRange* pFilterRange)
{
	return E_NOTIMPL;
}

HRESULT CVideoProcessorNV12toYV12::_GetProcAmpRange(PROCAMP_CONTROL ProcAmpControl, ProcAmpValueRange* pProcAmpRange)
{
	return E_NOTIMPL;
}

HRESULT CVideoProcessorNV12toYV12::_SetVideoProcessorMode(const VideoProcessorModes *pVPMode)
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

HRESULT CVideoProcessorNV12toYV12::_GetVideoProcessorMode(VideoProcessorModes *pVPMode)
{
	*pVPMode = m_VideoProcessorModes;
	return S_OK;
}

HRESULT CVideoProcessorNV12toYV12::_GetNumReferentSamples(DWORD	*pNumBackwardRefSamples,DWORD *pNumForwardRefSamples)
{
	*pNumBackwardRefSamples = m_dwNumBackwardRefSamples;
	*pNumForwardRefSamples  = m_dwNumForwardRefSamples;
	return S_OK;
}

HRESULT CVideoProcessorNV12toYV12::_VideoProcessBlt(VideoBuffer *pRenderTarget, 
													const VideoProcessBltParams *pBltParams,
													const VideoSample *pVideoSamples,
													UINT uNumSamples)
{
	// Find processing VideoSample.
	const VideoSample *pVideoSample = NULL;
	for (UINT uIndex = 0; uIndex < uNumSamples; uIndex++)
	{
		if (pVideoSamples[uIndex].Surface.VideoFormat == VIDEO_FORMAT_PRIVATE_NV12 &&
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

	int nWidth		= pVideoSample->rcDisplayRect.right - pVideoSample->rcDisplayRect.left;
	int nHeight		= pVideoSample->rcDisplayRect.bottom - pVideoSample->rcDisplayRect.top;
	int nStride_Y	= pVideoSample->Surface.Video.iStride[0];
	int nStride_UV	= pVideoSample->Surface.Video.iStride[1];
	int nRTPitch	= pRenderTarget->Video.iStride[0];
	m_y1			= (unsigned char *) pVideoSample->Surface.Video.pVideoPlane[0];
	//m_uv			= (unsigned char *) pVideoSample->Surface.Video.pVideoPlane[1];
	m_u				= (unsigned char *) pVideoSample->Surface.Video.pVideoPlane[1];
	m_v				= m_u + 8;

	unsigned char *dsty, *dstv, *dstu;
	//if((mode&DISPREND_MODE_RENDER_TRIMENSION)==DISPREND_MODE_RENDER_TRIMENSION)
	//{
	//	dsty = (unsigned char *)f->dnm_surface[0]; 
	//	dstu = (unsigned char *)f->dnm_surface[1]; 
	//	dstv = (unsigned char *)f->dnm_surface[2]; 
	//}
	//else
	{		
		dsty = (unsigned char *) pRenderTarget->Video.pVideoPlane[0];
		dstv = dsty + nHeight*nRTPitch;
		dstu = dsty + nHeight*nRTPitch*5/4;	
	}

	bool bDisplayWeaveMode;
	if (pBltParams->ProcessMode & PROCESS_MODE_FORCE_DEINTERLACING)
		bDisplayWeaveMode = false;
	else if (pBltParams->ProcessMode & PROCESS_MODE_FORCE_PROGRESSIVE)
		bDisplayWeaveMode = true;
	else //PROCESS_MODE_AUTO
		bDisplayWeaveMode = dwFrameStucture == FRAME_STRUCTURE_PROGRESSIVE || 
		nHeight == (pVideoSample->Surface.Video.iHeight[0]>>1); // Video frame down sample.

	DWORD	dwDeinterlaceTech = m_VideoProcessorModes.dwDeinterlaceTechnology;

	//memset(m_u_tmp_buf, 0, 1920);
	//memset(m_v_tmp_buf, 0, 1920);

	SetProcAmpValues(&pBltParams->sProcAmpValues);

	mm_empty;
	int i = 0;
	if(m_dwColorEffectLevel==2)
	{
		if (!bDisplayWeaveMode)
		{
			if(dwDeinterlaceTech==DEINTERLACE_TECH_MedianFiltering)
			{
				Mconvert_median_bcshg(i, nWidth, nStride_Y, nStride_UV, nRTPitch, nHeight, dsty, dstu, dstv, m_pSpicY, m_pSpicC, m_pSpicA, m_pSpicAC, m_pSpicProp, m_dwSpicWidth, TargetFrame);
			}
			else
			{
				Mconvert_average_bcshg(i, nWidth, nStride_Y, nStride_UV, nRTPitch, nHeight, dsty, dstu, dstv, m_pSpicY, m_pSpicC, m_pSpicA, m_pSpicAC, m_pSpicProp, m_dwSpicWidth);
			}
		}
		else
		{
			Mconvert_weave_bcshg(i, nWidth, nStride_Y, nStride_UV, nRTPitch, nHeight, dsty, dstu, dstv, m_pSpicY, m_pSpicC, m_pSpicA, m_pSpicAC, m_pSpicProp, m_dwSpicWidth);
		}
	}
	else if(m_dwColorEffectLevel==1)
	{
		if (!bDisplayWeaveMode)
		{
			if(dwDeinterlaceTech==DEINTERLACE_TECH_MedianFiltering)
			{
				Mconvert_median_bcs(i, nWidth, nStride_Y, nStride_UV, nRTPitch, nHeight, dsty, dstu, dstv, m_pSpicY, m_pSpicC, m_pSpicA, m_pSpicAC, m_pSpicProp, m_dwSpicWidth, TargetFrame);
			}
			else
			{
				Mconvert_average_bcs(i, nWidth,  nStride_Y, nStride_UV, nRTPitch, nHeight, dsty, dstu, dstv, m_pSpicY, m_pSpicC, m_pSpicA, m_pSpicAC, m_pSpicProp, m_dwSpicWidth);
			}
		}
		else
		{
			Mconvert_weave_bcs(i, nWidth, nStride_Y, nStride_UV, nRTPitch, nHeight, dsty, dstu, dstv, m_pSpicY, m_pSpicC, m_pSpicA, m_pSpicAC, m_pSpicProp, m_dwSpicWidth);
		}
	}
	else
	{
		if (!bDisplayWeaveMode)
		{
			if(dwDeinterlaceTech==DEINTERLACE_TECH_MedianFiltering)
			{
				Mconvert_median_b(i, nWidth, nStride_Y, nStride_UV, nRTPitch, nHeight, dsty, dstu, dstv, m_pSpicY, m_pSpicC, m_pSpicA, m_pSpicAC, m_pSpicProp, m_dwSpicWidth, TargetFrame);
			}
			else
			{
				Mconvert_average_b(i, nWidth,  nStride_Y, nStride_UV, nRTPitch, nHeight, dsty, dstu, dstv, m_pSpicY, m_pSpicC, m_pSpicA, m_pSpicAC, m_pSpicProp, m_dwSpicWidth);
			}
		}
		else
		{
			Mconvert_weave_b(i, nWidth, nStride_Y, nStride_UV, nRTPitch, nHeight, dsty, dstu, dstv, m_pSpicY, m_pSpicC, m_pSpicA, m_pSpicAC, m_pSpicProp, m_dwSpicWidth);
		}
	}

	return S_OK;
}


HRESULT CVideoProcessorNV12toYV12::SetProcAmpValues(const ProcAmpValues* pProcAmpValues)
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

	int i;
	int brightness, uoffset, voffset;	// level0
	int contrast, saturation;			// level1

	m_dwColorEffectLevel = 0;
	brightness = pProcAmpValues->iBrightness;
	uoffset = pProcAmpValues->iUOffset;
	voffset = pProcAmpValues->iVOffset;
	if(brightness>=0)
	{
		for(i=0;i<8;i++)
		{
			m_adjust_up_y[i] = brightness;
			m_adjust_down_y[i] = 0;
		}
	}
	else
	{
		for(i=0;i<8;i++)
		{
			m_adjust_up_y[i] = 0;
			m_adjust_down_y[i] = -brightness;
		}
	}
	if(uoffset>=0)
	{
		m_adjust_up_uv[0] = m_adjust_up_uv[2] = m_adjust_up_uv[4] = m_adjust_up_uv[6] = uoffset;
		m_adjust_down_uv[0] = m_adjust_down_uv[2] = m_adjust_down_uv[4] = m_adjust_down_uv[6] = 0;
	}
	else
	{
		m_adjust_up_uv[0] = m_adjust_up_uv[2] = m_adjust_up_uv[4] = m_adjust_up_uv[6] = 0;
		m_adjust_down_uv[0] = m_adjust_down_uv[2] = m_adjust_down_uv[4] = m_adjust_down_uv[6] = -uoffset;
	}
	if(voffset>=0)
	{
		m_adjust_up_uv[1] = m_adjust_up_uv[3] = m_adjust_up_uv[5] = m_adjust_up_uv[7] = voffset;
		m_adjust_down_uv[1] = m_adjust_down_uv[3] = m_adjust_down_uv[5] = m_adjust_down_uv[7] = 0;
	}
	else
	{
		m_adjust_up_uv[1] = m_adjust_up_uv[3] = m_adjust_up_uv[5] = m_adjust_up_uv[7] = 0;
		m_adjust_down_uv[1] = m_adjust_down_uv[3] = m_adjust_down_uv[5] = m_adjust_down_uv[7] = -voffset;
	}
	for(i=0;i<4;i++)
		m_adjust_offset_y[i] = brightness+128;
	m_adjust_offset_uv[0] = m_adjust_offset_uv[2] = uoffset+128;
	m_adjust_offset_uv[1] = m_adjust_offset_uv[3] = voffset+128;
	contrast = pProcAmpValues->iContrast;
	saturation = pProcAmpValues->iSaturation;
	for(i=0;i<4;i++)
	{
		m_adjust_multiply_y[i] = contrast;
		m_adjust_multiply_uv[i] = saturation;
	}
	if(contrast!=256 || saturation!=256)
		m_dwColorEffectLevel = 1;
	if(m_bSSE2)
	{
		int hue, gamma;	// level2

		hue = pProcAmpValues->iHue;
		gamma = pProcAmpValues->iGamma;
		if(hue)
		{
			double dsin, dcos, drot;

			m_dwColorEffectLevel = 2;
			drot = (double)hue * (double)(2.0 * 3.14159265358979/GPI_VCC_RANGE);
			dsin = std::sin(drot);
			dcos = std::cos(drot);
			m_adjust_hue_uv[0] = m_adjust_hue_uv[2] = (short)(-dsin * saturation);
			m_adjust_hue_uv[1] = m_adjust_hue_uv[3] = (short)(dsin * saturation);
			m_adjust_multiply_uv[0] = m_adjust_multiply_uv[2] = (short)(dcos * saturation);
			m_adjust_multiply_uv[1] = m_adjust_multiply_uv[3] = (short)(dcos * saturation);
		}
		else
		{
			for(i=0;i<4;i++)
				m_adjust_hue_uv[i] = 0;
		}
		if(gamma)
		{
			int delta2, delta4;

			m_dwColorEffectLevel = 2;
			gamma = -gamma*512/GPI_VCC_RANGE;			// scale gamma to -256 to +256 (squared output range is +-127).
			delta2 = gamma/2;
			delta4 = gamma/4;
			for(i=0;i<4;i++)
			{
				m_adjust_gamma_y[i] = gamma;
				m_adjust_offset_y[i] -= delta4;	// readjust offset
				m_adjust_multiply_y[i] -= delta4;// readjust slope
			}
		}
		else
		{
			for(i=0;i<4;i++)
				m_adjust_gamma_y[i] = 0;
		}
	}
	if(m_bSSE2)
	{	
		// SSE2 uses double width registers so we copy low to high.
		mm_empty;
		((__m64 *)m_adjust_up_y)[1]			= ((__m64 *)m_adjust_up_y)[0];
		((__m64 *)m_adjust_down_y)[1]		= ((__m64 *)m_adjust_down_y)[0];
		((__m64 *)m_adjust_up_uv)[1]		= ((__m64 *)m_adjust_up_uv)[0];
		((__m64 *)m_adjust_down_uv)[1]		= ((__m64 *)m_adjust_down_uv)[0];
		((__m64 *)m_adjust_offset_y)[1]		= ((__m64 *)m_adjust_offset_y)[0];
		((__m64 *)m_adjust_offset_uv)[1]	= ((__m64 *)m_adjust_offset_uv)[0];
		((__m64 *)m_adjust_multiply_y)[1]	= ((__m64 *)m_adjust_multiply_y)[0];
		((__m64 *)m_adjust_multiply_uv)[1]	= ((__m64 *)m_adjust_multiply_uv)[0];
		((__m64 *)m_adjust_gamma_y)[1]		= ((__m64 *)m_adjust_gamma_y)[0];
		((__m64 *)m_adjust_hue_uv)[1]		= ((__m64 *)m_adjust_hue_uv)[0];
		mm_empty;
	}

	m_ProcAmpValues = *pProcAmpValues;

	return S_OK;
}

// optimize the following code even for debug
#if defined(_DEBUG) && !defined(__linux__)
#pragma optimize("gsw",on)
#endif

#define UV_BLOCKWIDTH 16

void CVideoProcessorNV12toYV12::Mconvert_weave_b(int &i, int width, int stridey, int stridec, int pitch, int lines, unsigned char *dsty, unsigned char *dstu, unsigned char *dstv, unsigned char *pSpicY, unsigned char *pSpicC, unsigned char *pSpicA, unsigned char *pSpicAC, unsigned long *pSpicProp, DWORD dwSpicWidth)
{
	convert_weave(i, width, stridey, stridec, pitch, lines, adjust_b, dwSpicWidth);
#if 0
	DECLARE_CONVERTVARS;
	for(; i<lines; i++)
	{
		createcmds_y(width, stridey, stridec, dsty, m_y1, pSpicY, pSpicA, pSpicProp[i]);
		convert_y(width, load, store, spicblend, prefetch_1, adjust_by);
		m_y1 += stridey;
		dsty += pitch;
		//pSpicY += dwSpicWidth;
		//pSpicA += dwSpicWidth;
		if((i&1)==0)
		{
			createcmds_uv(width, dstu, dstv, m_u, m_v, pSpicC, pSpicAC, pSpicProp[i]);
			convert_uv(width, stridec, load, store, spicblend, prefetch_1, adjust_buv);
			m_u += stridec;
			m_v += stridec;
			dstu += pitch>>1;
			dstv += pitch>>1;
		//	pSpicC += dwSpicWidth;
		//	pSpicAC += dwSpicWidth;
		}
	}
#endif
	mm_empty;
}

void CVideoProcessorNV12toYV12::Mconvert_weave_bcs(int &i, int width,  int stridey, int stridec, int pitch, int lines, unsigned char *dsty, unsigned char *dstu, unsigned char *dstv, unsigned char *pSpicY, unsigned char *pSpicC, unsigned char *pSpicA, unsigned char *pSpicAC, unsigned long *pSpicProp, DWORD dwSpicWidth)
{
	convert_weave(i, width, stridey, stridec, pitch, lines, adjust_bcs, dwSpicWidth);
	mm_empty;
}
void CVideoProcessorNV12toYV12::Mconvert_weave_bcshg(int &i, int width, int stridey, int stridec, int pitch, int lines, unsigned char *dsty, unsigned char *dstu, unsigned char *dstv, unsigned char *pSpicY, unsigned char *pSpicC, unsigned char *pSpicA, unsigned char *pSpicAC, unsigned long *pSpicProp, DWORD dwSpicWidth)
{
	convert_weave(i, width, stridey, stridec, pitch, lines, adjust_bcshg, dwSpicWidth);
	mm_empty;
}

void CVideoProcessorNV12toYV12::Mconvert_average_b(int &i, int width, int stridey, int stridec, int pitch, int lines, unsigned char *dsty, unsigned char *dstu, unsigned char *dstv, unsigned char *pSpicY, unsigned char *pSpicC, unsigned char *pSpicA, unsigned char *pSpicAC, unsigned long *pSpicProp, DWORD dwSpicWidth)
{
	convert_average(i, width, stridey, stridec, pitch, lines, adjust_b, dwSpicWidth);
	mm_empty;
}
void CVideoProcessorNV12toYV12::Mconvert_average_bcs(int &i, int width, int stridey, int stridec, int pitch, int lines, unsigned char *dsty, unsigned char *dstu, unsigned char *dstv, unsigned char *pSpicY, unsigned char *pSpicC, unsigned char *pSpicA, unsigned char *pSpicAC, unsigned long *pSpicProp, DWORD dwSpicWidth)
{
	convert_average(i, width, stridey, stridec, pitch, lines, adjust_bcs, dwSpicWidth);
	mm_empty;
}
void CVideoProcessorNV12toYV12::Mconvert_average_bcshg(int &i, int width,  int stridey, int stridec, int pitch, int lines, unsigned char *dsty, unsigned char *dstu, unsigned char *dstv, unsigned char *pSpicY, unsigned char *pSpicC, unsigned char *pSpicA, unsigned char *pSpicAC, unsigned long *pSpicProp, DWORD dwSpicWidth)
{
	convert_average(i, width, stridey, stridec, pitch, lines, adjust_bcshg, dwSpicWidth);
	mm_empty;
}

void CVideoProcessorNV12toYV12::Mconvert_median_b(int &i, int width, int stridey, int stridec, int pitch, int lines, unsigned char *dsty, unsigned char *dstu, unsigned char *dstv, unsigned char *pSpicY, unsigned char *pSpicC, unsigned char *pSpicA, unsigned char *pSpicAC, unsigned long *pSpicProp, DWORD dwSpicWidth, TARGET_FRAME TargetFrame)
{
	if (TargetFrame == TARGET_FRAME_TOP_FIELD)
	{
		convert_median_bottom(i, width,  stridey, stridec, pitch, lines, adjust_b, dwSpicWidth);
	}
	else
	{
		convert_median_top(i, width, stridey, stridec, pitch, lines, adjust_b, dwSpicWidth);
	}
	mm_empty;
}
void CVideoProcessorNV12toYV12::Mconvert_median_bcs(int &i, int width, int stridey, int stridec, int pitch, int lines, unsigned char *dsty, unsigned char *dstu, unsigned char *dstv, unsigned char *pSpicY, unsigned char *pSpicC, unsigned char *pSpicA, unsigned char *pSpicAC, unsigned long *pSpicProp, DWORD dwSpicWidth, TARGET_FRAME TargetFrame)
{
	if (TargetFrame == TARGET_FRAME_TOP_FIELD)
	{
		convert_median_bottom(i, width, stridey, stridec, pitch, lines, adjust_bcs, dwSpicWidth);
	}
	else
	{
		convert_median_top(i, width, stridey, stridec, pitch, lines, adjust_bcs, dwSpicWidth);
	}
	mm_empty;
}
void CVideoProcessorNV12toYV12::Mconvert_median_bcshg(int &i, int width, int stridey, int stridec, int pitch, int lines, unsigned char *dsty, unsigned char *dstu, unsigned char *dstv, unsigned char *pSpicY, unsigned char *pSpicC, unsigned char *pSpicA, unsigned char *pSpicAC, unsigned long *pSpicProp, DWORD dwSpicWidth, TARGET_FRAME TargetFrame)
{
	if (TargetFrame == TARGET_FRAME_TOP_FIELD)
	{
		convert_median_bottom(i, width, stridey, stridec, pitch, lines, adjust_bcshg, dwSpicWidth);
	}
	else
	{
		convert_median_top(i, width, stridey, stridec, pitch, lines, adjust_bcshg, dwSpicWidth);
	}
	mm_empty;
}

#undef UV_BLOCKWIDTH

#if defined(_DEBUG) && !defined(__linux__)
#pragma optimize("",on)
#endif
