#ifndef _DISPSVR_AMD_PCOM_MIXER_PRESENTER_H_
#define _DISPSVR_AMD_PCOM_MIXER_PRESENTER_H_

#include "Imports/ThirdParty/AMD/PCOM/amdpcom.h"
#include "Imports/ThirdParty/AMD/MCOM/amdmcom.h"
#include "D3D9VideoMixerBase.h"
#include "D3D9VideoPresenterBase.h"

/////////////////////////////////////////////////////////////////////////////

namespace DispSvr
{
	class CAMDPCOMMixerPresenter :
		public virtual CD3D9VideoPresenterBase,
		public virtual CD3D9VideoMixerBase
	{
	public:
		CAMDPCOMMixerPresenter();
		virtual ~CAMDPCOMMixerPresenter();

		// IUnkonwn
		STDMETHOD(QueryInterface)(REFIID riid, void **ppv);
		STDMETHOD_(ULONG, AddRef)() { return CD3D9PluginBase::AddRef();	}
		STDMETHOD_(ULONG, Release)() { return CD3D9PluginBase::Release(); }

		// IDispSvrPlugin
		STDMETHOD(ProcessMessage)(RESOURCE_MESSAGE_TYPE msg, LPVOID ulParam);

		// CD3D9VideoPresenterBase
		STDMETHOD(SetDisplayRect)(const RECT *rcDst, const RECT *rcSrc);
		STDMETHOD(Present)(const PresentHints *pHints);
		STDMETHOD(Clear)();
		STDMETHOD(SetColorKey)(const DWORD dwColorKey);
		STDMETHOD(SetGamutMetadata)(const DWORD dwFormat, void *pGamutMetadata);

	private:

		// CD3D9VideoMixerBase & CD3D9VideoPresenterBase
		STDMETHOD(_Execute)(IDirect3DSurface9 *pDestSurface, const RECT &rcDst, const RECT &rcDstClip);
		STDMETHOD(_SetDevice)(IUnknown *pDevice);
		STDMETHOD(_ReleaseDevice)();
		STDMETHOD(_SetWindow)(HWND hwnd);
		STDMETHOD(_QueryCaps)(PresenterCaps* pCaps);
		STDMETHOD(_QueryPlaneCaps)(PLANE_ID PlaneID, PLANE_FORMAT Format, PlaneCaps *pCap);

		HRESULT CreatePCOM();
		HRESULT DestroyPCOM();
		HRESULT BeginFramePCOM(const RECT &rcDst);
		HRESULT ExecutePCOM(PPCOM_EXECUTE_INPUT pExecuteIn);
		HRESULT	EndFramePCOM();
		HRESULT PresentPCOM();
		HRESULT ClearPCOM();
		HRESULT PlaneToPCOMPlane(PLANE_ID id, PCOM_PLANE &PCOMPlane, const RECT &rcDst, const RECT &rcDstClip, const float fWindowAspectRatio);
		HRESULT CreateMCOM();
		HRESULT DestroyMCOM();
		HRESULT QueryDecodeStreamCaps();

	private:	
		DWORD m_dwFlipChainSize;
		DWORD m_dwQueuedFrameCount;
		bool m_bFullScreen;

		// PCOM session handle
		void* m_pSession;
		XVYCC_GAMUT_METADATA m_XvYCCGamutMetaData;
		// PCOM clear rectangle list
		DWORD m_dwPCOMClearRecCount;
		PCOM_CLEAR_RECTANGLE m_PCOMClearRecList[PCOM_MAX_CLEAR_RECTANGLES_COUNT];
		bool m_bSpatialDeinterlacing;
		bool m_bAsyncPresent;
		// MCOM session handle
		void* m_pMcomSession;
		MCOM_DECODE_STREAM_CAPS m_DecodeStreamCaps;
	};
}

#endif //_AMDPCOMMIXERRENDER_H_