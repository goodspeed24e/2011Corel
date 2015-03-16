#ifndef _DISPSVR_D3D9_VIDEO_PRESENTER_BASE_H_
#define _DISPSVR_D3D9_VIDEO_PRESENTER_BASE_H_

#include "D3D9PluginBase.h"

namespace DispSvr
{
	class DECLSPEC_NOVTABLE CD3D9VideoPresenterBase : 
		public virtual CD3D9PluginBase,
		public virtual IDispSvrVideoPresenter
	{
	public:
		CD3D9VideoPresenterBase();
		virtual ~CD3D9VideoPresenterBase();

	public:
		// IUnkonwn
		STDMETHOD(QueryInterface)(REFIID riid, void **ppv);
		STDMETHOD_(ULONG, AddRef)()	{ return CD3D9PluginBase::AddRef();	}
		STDMETHOD_(ULONG, Release)() { return CD3D9PluginBase::Release(); }

		// IDispSvrPrivateRender
		STDMETHOD(SetDisplayRect)(const RECT *rcDst, const RECT *rcSrc);
		STDMETHOD(BeginRender)();
		STDMETHOD(EndRender)();
		STDMETHOD(Present)(const PresentHints *pHints);
		STDMETHOD(Clear)();
		STDMETHOD(SetProperty)(const PresenterProperty *pProperty);
		STDMETHOD(GetProperty)(PresenterProperty *pProperty);
		STDMETHOD(SetColorKey)(const DWORD dwColorKey);
		STDMETHOD(GetColorKey)(DWORD* pdwColorKey);
		STDMETHOD(QueryCaps)(PresenterCaps* pCaps);
		STDMETHOD(SetScreenCaptureDefense)(BOOL bEnable);
		STDMETHOD(SetGamutMetadata)(const DWORD dwFormat, void *pGamutMetadata);

	protected:
		STDMETHOD(_SetDevice)(IUnknown *pDevice);
		STDMETHOD(_ReleaseDevice)();
		STDMETHOD(_QueryCaps)(PresenterCaps* pCaps);

		HRESULT CaculateDstClipRect(RECT *rcClip, RECT *rcDst, const RECT *rcSrc, const SIZE szDisplay);
		void UpdateScanLineProperty();
		bool IsWindowFullScreen();
		bool IsWindowMinimized();
		void WaitUntilPresentable();

	protected:
		CCritSec m_csLock;
		RECT m_rcSrc;
		RECT m_rcDst;
		SIZE m_szSrc;
		float m_fScanLineInterval; // Time cost of scanning 1 line in ms.
		DWORD m_dwSafePresentOffset;
		DWORD m_dwColorKey;
		DWORD m_dwColorKey32;
		PresenterProperty m_PresenterProperty;
		PresenterCaps m_PresenterCaps;
		GamutMetadataRange m_GamutRange;
		GamutMetadataVertices m_GamutVertices;
		DWORD m_dwGamutFormat;
		HANDLE m_hPresent;
		LPDIRECTDRAW m_lpDDraw;
	};
}

#endif