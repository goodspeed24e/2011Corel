#ifndef _VIDEO_PROCESSOR_NV12toNV12_H_
#define _VIDEO_PROCESSOR_NV12toNV12_H_

#include "VideoProcessorBase.h"

namespace VideoProcessor
{
	class CVideoProcessorNV12toNV12 : public CVideoProcessorBase
	{
	public:
		CVideoProcessorNV12toNV12();
		virtual ~CVideoProcessorNV12toNV12();

	public:
		// IUnknown
		STDMETHOD(QueryInterface)(REFIID riid, void **ppv);
		STDMETHOD_(ULONG, AddRef)();
		STDMETHOD_(ULONG, Release)();

	protected:
		HRESULT _QueryVideoProcessorCaps( 
			/* [out] */ 
			VideoProcessorCaps **ppCaps);

		HRESULT _GetFilterPropertyRange( 
			/* [in] */ 
			VIDEO_FILTER VideoFilter,
			/* [out] */ 
			FilterValueRange* pFilterRange);

		HRESULT _GetProcAmpRange( 
			/* [in] */ 
			PROCAMP_CONTROL ProcAmpControl,
			/* [out] */ 
			ProcAmpValueRange* pProcAmpRange);

		HRESULT _SetVideoProcessorMode( 
			/* [in] */ 
			const VideoProcessorModes *pVPMode);

		HRESULT _GetVideoProcessorMode( 
			/* [out] */ 
			VideoProcessorModes *pVPMode);

		HRESULT _GetNumReferentSamples( 
			/* [out] */ 
			DWORD	*pNumBackwardRefSamples,
			/* [out] */ 
			DWORD	*pNumForwardRefSamples);

		HRESULT _VideoProcessBlt( 
			/* [in] */ 
			VideoBuffer *pRenderTarget,
			/* [in] */ 
			const VideoProcessBltParams *pBltParams,
			/* [size_is][in] */ 
			const VideoSample *pVideoSamples,
			/* [in] */ 
			UINT uNumSamples);

	private:
		//void Mconvert_average_simple_b(int &i, int width, int stridey, int stridec, int pitch, int lines, unsigned char *dsty, unsigned char *dstuv, unsigned char *pSpicY, unsigned char *pSpicC, unsigned char *pSpicA, unsigned char *pSpicAC, unsigned long *pSpicProp, DWORD dwSpicWidth);
		//void Mconvert_average_simple_bcs(int &i, int width, int stridey, int stridec, int pitch, int lines, unsigned char *dsty, unsigned char *dstuv, unsigned char *pSpicY, unsigned char *pSpicC, unsigned char *pSpicA, unsigned char *pSpicAC, unsigned long *pSpicProp, DWORD dwSpicWidth);
		void Mconvert_weave_b(int &i, int width, int stridey, int stridec, int pitch, int lines, unsigned char *dsty, unsigned char *dstuv, unsigned char *pSpicY, unsigned char *pSpicC, unsigned char *pSpicA, unsigned char *pSpicAC, unsigned long *pSpicProp, DWORD dwSpicWidth);
		void Mconvert_weave_bcs(int &i, int width, int stridey, int stridec, int pitch, int lines, unsigned char *dsty, unsigned char *dstuv, unsigned char *pSpicY, unsigned char *pSpicC, unsigned char *pSpicA, unsigned char *pSpicAC, unsigned long *pSpicProp, DWORD dwSpicWidth);
		void Mconvert_weave_bcshg(int &i, int width, int stridey, int stridec, int pitch, int lines, unsigned char *dsty, unsigned char *dstuv, unsigned char *pSpicY, unsigned char *pSpicC, unsigned char *pSpicA, unsigned char *pSpicAC, unsigned long *pSpicProp, DWORD dwSpicWidth);
		void Mconvert_average_b(int &i, int width, int stridey, int stridec, int pitch, int lines, unsigned char *dsty, unsigned char *dstuv, unsigned char *pSpicY, unsigned char *pSpicC, unsigned char *pSpicA, unsigned char *pSpicAC, unsigned long *pSpicProp, DWORD dwSpicWidth);
		void Mconvert_average_bcs(int &i, int width, int stridey, int stridec, int pitch, int lines, unsigned char *dsty, unsigned char *dstuv, unsigned char *pSpicY, unsigned char *pSpicC, unsigned char *pSpicA, unsigned char *pSpicAC, unsigned long *pSpicProp, DWORD dwSpicWidth);
		void Mconvert_average_bcshg(int &i, int width, int stridey, int stridec, int pitch, int lines, unsigned char *dsty, unsigned char *dstuv, unsigned char *pSpicY, unsigned char *pSpicC, unsigned char *pSpicA, unsigned char *pSpicAC, unsigned long *pSpicProp, DWORD dwSpicWidth);
		void Mconvert_median_b(int &i, int width, int stridey, int stridec, int pitch, int lines, unsigned char *dsty, unsigned char *dstuv, unsigned char *pSpicY, unsigned char *pSpicC, unsigned char *pSpicA, unsigned char *pSpicAC, unsigned long *pSpicProp, DWORD dwSpicWidth, TARGET_FRAME TargetFrame);
		void Mconvert_median_bcs(int &i, int width, int stridey, int stridec, int pitch, int lines, unsigned char *dsty, unsigned char *dstuv, unsigned char *pSpicY, unsigned char *pSpicC, unsigned char *pSpicA, unsigned char *pSpicAC, unsigned long *pSpicProp, DWORD dwSpicWidth, TARGET_FRAME TargetFrame);
		void Mconvert_median_bcshg(int &i, int width, int stridey, int stridec, int pitch, int lines, unsigned char *dsty, unsigned char *dstuv, unsigned char *pSpicY, unsigned char *pSpicC, unsigned char *pSpicA, unsigned char *pSpicAC, unsigned long *pSpicProp, DWORD dwSpicWidth, TARGET_FRAME TargetFrame);

		// ColorEffect
		HRESULT	SetProcAmpValues(const ProcAmpValues* pProcAmpValues);
		unsigned char	*m_uv_tmp_buf;
		unsigned char	*m_adjust_up_y;
		unsigned char	*m_adjust_down_y;	//yv12/rgb
		unsigned char	*m_adjust_up_uv;
		unsigned char	*m_adjust_down_uv;
		short *m_adjust_multiply_y;
		short *m_adjust_multiply_uv;
		short *m_adjust_offset_y;
		short *m_adjust_offset_uv;			//yv12/rgb
		short *m_adjust_hue_uv;
		short *m_adjust_gamma_y;			//yv12/rgb

		unsigned char *m_y1, *m_uv;		//16b

		// for YV12 subpictures
		bool            m_bHDspic;
		unsigned char	*m_pSpicY;
		unsigned char	*m_pSpicC;
		unsigned char	*m_pSpicA;		// alphas for Y
		unsigned char	*m_pSpicAC;		// alphas for C
		DWORD           *m_pSpicProp;
		DWORD			m_dwSpicWidth;
		DWORD			m_dwSpicHeight;
	};
}

#endif // _VIDEO_PROCESSOR_NV12toNV12_H_