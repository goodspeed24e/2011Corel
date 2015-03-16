#pragma once

//#include "vrnframe.h"
//#include "../LIBDS/dsfilter.h"
//#include "../LIBDS/dxvaex.h"
#include <mpconfig.h>
#include "Exports\Inc\HVDService.h"
//#include "../ivivideo/ivividef.h"

// The filter that is used to connect with VMR9 for DXVA access. 
// Most implementation is copied from ivivideo.

//defined in ivividef.h---------------
#define VIDEO_VIDOUT_NAME				"Video Output"
#define VIDEO_VIDOUT_LNAME				L"Video Output"
#define VIDEO_SUBOUT_NAME				"~Subpicture Output"
#define VIDEO_SUBOUT_LNAME				L"~Subpicture Output"

#define VIDEO_FILTER_NAME			"Corel Video Decoder"
#define VIDEO_FILTER_LNAME			L"Corel Video Decoder"
//No CSS
#define VIDEO_FILTER_MERIT				0x700000
//else
//#define VIDEO_FILTER_MERIT				0x400000,MERIT_DO_NOT_USE,MERIT_UNLIKELY
//--------------------------------------

namespace HVDService
{
	class CHVDDSOutPin;

	class CHVDDxva1Binder : public CBaseFilter
	{
	public:

		CHVDDxva1Binder();
		~CHVDDxva1Binder();

		DECLARE_IUNKNOWN;

		virtual HRESULT GetMediaType(CBasePin *pPin, int pos, CMediaType *pmt);
		virtual HRESULT DecideBufferSize(CBasePin *pPin, IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pProperties);
		virtual HRESULT CompleteConnect(CBasePin *pPin, IPin *pReceivePin);
		virtual HRESULT BreakConnect(CBasePin *pPin);
		//CBaseFileter method
		virtual int GetPinCount();
		virtual CBasePin *GetPin(int n);
		STDMETHODIMP Run(REFERENCE_TIME tStart);
		STDMETHODIMP Pause();
		STDMETHODIMP Stop();

		CHVDDSOutPin *CreateNextOutputPin(TCHAR *pName, const AMOVIESETUP_PIN *pPinInfo, LPWSTR pPinName = NULL);
		CHVDDSOutPin *GetOutputPin(int n);
		HRESULT GetStartTime(REFERENCE_TIME *pStartTime);
		void SetVideoDecodeInfo(int width, int height, HVDService::HVD_MODE dwDecoderId);
		HRESULT SetVideoRender(IBaseFilter *pVideoRender);
		HRESULT SetUncompSurfacesConfig(HVDService::HVDDxva1UncompSurfConfig *pUncompSurfacesConfig);
		HRESULT SetOutputPinBufferCount(int nPinID, DWORD dwBufCount);
		HRESULT Destroy();

	public:
		CCritSec		m_CritSec;
		HRESULT			m_hr;

	private:

		HRESULT SetVideoInfo(int iWidth, int iHeight, CMediaType *pmt, CBasePin *pPin, int pos);
		HRESULT SetDeinterlaceMode(VIDEOINFOHEADER2 *header2);

	private:

		static AMOVIESETUP_FILTER m_FilterInfo;
		static AMOVIESETUP_MEDIATYPE m_pVidOutType[];
		static AMOVIESETUP_MEDIATYPE m_pSubOutType[];
		static const AMOVIESETUP_PIN m_pPinInfo[];

		CHVDDSOutPin* m_pVidOut;
		CHVDDSOutPin* m_pSubOut;

		CGenericList <CHVDDSOutPin>	m_OutputPinList;		
		int m_NextOutputPinNumber;
		int m_NumOutputPins;

		int m_InterlaceSample;
		int	m_width_vid;
		int	m_height_vid;
		int m_width_spic;
		int m_height_spic;

		DWORD m_dwVideoOutputBufCount;
		DWORD m_dwSubOutputBufCount;

		REFERENCE_TIME m_rtAvgTimeIn;
		REFERENCE_TIME m_rtStartTime;
		HVDService::HVD_MODE m_dwDecoderId;

		IBaseFilter *m_pVideoRender;
	};
}