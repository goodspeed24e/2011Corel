#ifndef _VIDEO_PROCESSOR_BASE_H_
#define _VIDEO_PROCESSOR_BASE_H_

#include "Exports/Inc/VideoProcessor.h"

#ifdef __linux__
#	define ALIGN(bytes,lval) lval __attribute__((aligned(bytes)))
#	define I64(x) x##LL
#else
#	define ALIGN(bytes,lval) __declspec(align(bytes)) lval
#	define I64(x) x
#endif

#ifdef INTEL_CVT
#	define GPI_VCC_MIN				(-20)
#	define GPI_VCC_MAX				20
#else
#	define GPI_VCC_MIN				(-32)
#	define GPI_VCC_MAX				32
#endif
#	define GPI_VCC_RANGE			(GPI_VCC_MAX-GPI_VCC_MIN)

#if defined (__linux__) && !defined(_USE_MSASM)
#	define mm_empty asm ("emms")
#else
#	define mm_empty _m_empty()
#endif

//for detecting  the type of CPU (supports MMX, SSE, SSE2 and SSE3)
typedef enum
{
	CPU_LEVEL_NONE,		//supports nothing
	CPU_LEVEL_MMX,
	CPU_LEVEL_SSE,
	CPU_LEVEL_SSE2,
	CPU_LEVEL_SSE3
} CPU_LEVEL;

namespace VideoProcessor
{
	enum TARGET_FRAME
	{
		TARGET_FRAME_PROGRESSIVE	= 0,
		TARGET_FRAME_TOP_FIELD		= 0x1,
		TARGET_FRAME_BOTTOM_FIELD	= 0x2,
	};

	class CVideoProcessorBase : public IVideoProcessor
	{
	public:
		CVideoProcessorBase();
		virtual ~CVideoProcessorBase();

	public:
		// IUnknown
		STDMETHOD(QueryInterface)(REFIID riid, void **ppv);
		STDMETHOD_(ULONG, AddRef)();
		STDMETHOD_(ULONG, Release)();

	public:
		// IVideoProcessor
		STDMETHOD(QueryVideoProcessorCaps) ( 
			/* [out] */ 
			VideoProcessorCaps **ppCaps);

		STDMETHOD(GetFilterPropertyRange) ( 
			/* [in] */ 
			VIDEO_FILTER VideoFilter,
			/* [out] */ 
			FilterValueRange* pFilterRange);

		STDMETHOD(GetProcAmpRange) ( 
			/* [in] */ 
			PROCAMP_CONTROL ProcAmpControl,
			/* [out] */ 
			ProcAmpValueRange* pProcAmpRange);

		STDMETHOD(SetVideoProcessorMode) ( 
			/* [in] */ 
			const VideoProcessorModes *pVPMode);

		STDMETHOD(GetVideoProcessorMode) ( 
			/* [out] */ 
			VideoProcessorModes *pVPMode);

		STDMETHOD(GetNumReferentSamples) ( 
			/* [out] */ 
			DWORD	*pNumBackwardRefSamples,
			/* [out] */ 
			DWORD	*pNumForwardRefSamples);

		STDMETHOD(VideoProcessBlt) ( 
			/* [in] */ 
			VideoBuffer *pRenderTarget,
			/* [in] */ 
			const VideoProcessBltParams *pBltParams,
			/* [size_is][in] */ 
			const VideoSample *pVideoSamples,
			/* [in] */ 
			UINT uNumSamples);

	protected:
		virtual HRESULT _QueryVideoProcessorCaps( 
			/* [out] */ 
			VideoProcessorCaps **ppCaps);

		virtual HRESULT _GetFilterPropertyRange( 
			/* [in] */ 
			VIDEO_FILTER VideoFilter,
			/* [out] */ 
			FilterValueRange* pFilterRange);

		virtual HRESULT _GetProcAmpRange( 
			/* [in] */ 
			PROCAMP_CONTROL ProcAmpControl,
			/* [out] */ 
			ProcAmpValueRange* pProcAmpRange);

		virtual HRESULT _SetVideoProcessorMode( 
			/* [in] */ 
			const VideoProcessorModes *pVPMode);

		virtual HRESULT _GetVideoProcessorMode( 
			/* [out] */ 
			VideoProcessorModes *pVPMode);

		virtual HRESULT _GetNumReferentSamples( 
			/* [out] */ 
			DWORD	*pNumBackwardRefSamples,
			/* [out] */ 
			DWORD	*pNumForwardRefSamples);

		virtual HRESULT _VideoProcessBlt( 
			/* [in] */ 
			VideoBuffer *pRenderTarget,
			/* [in] */ 
			const VideoProcessBltParams *pBltParams,
			/* [size_is][in] */ 
			const VideoSample *pVideoSamples,
			/* [in] */ 
			UINT uNumSamples);

	protected:
		CPU_LEVEL	GetCPULevel();
		VideoProcessorCaps	m_VideoProcessorCaps;
		VideoProcessorModes	m_VideoProcessorModes;
		LONG			m_cRef;
		BOOL			m_bSSE2;
		DWORD			m_dwNumForwardRefSamples;
		DWORD			m_dwNumBackwardRefSamples;
		ProcAmpValues	m_ProcAmpValues;
		DWORD			m_dwColorEffectLevel;
	};
}

#endif // _VIDEO_PROCESSOR_BASE_H_