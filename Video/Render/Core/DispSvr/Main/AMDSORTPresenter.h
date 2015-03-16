#ifndef _DISPSVR_AMDSORT_VIDEO_PRESENTER_
#define _DISPSVR_AMDSORT_VIDEO_PRESENTER_

#include "D3D9VideoPresenterBase.h"

namespace DispSvr
{
	class CAMDSORTVideoPresenter : public CD3D9VideoPresenterBase
	{
	public:
		CAMDSORTVideoPresenter();
		virtual ~CAMDSORTVideoPresenter();

		STDMETHOD(SetDisplayRect)(const RECT *rcDst, const RECT *rcSrc);
		STDMETHOD(BeginRender)();
		STDMETHOD(EndRender)();
		STDMETHOD(Present)(const PresentHints *pHints);
		STDMETHOD(SetColorKey)(const DWORD dwColorKey);

	protected:
		STDMETHOD(_SetDevice)(IUnknown *pDevice);
		STDMETHOD(_ReleaseDevice)();
		//STDMETHOD(_SetWindow)(HWND hwnd);
	
		HRESULT CreateEAPI();
		HRESULT CreateSORT();
		HRESULT ExecSORT(DWORD dwCommand, DWORD dwFlags);
		HRESULT Flip();

	private:
		bool m_bPreFlip;
		bool m_bOverlyHided;
		DWORD m_dwCapsEAPI;
		DWORD m_dwCapsSORT;
		DWORD m_dwStatus;
		DWORD m_dwStatusEx;
		DWORD m_dwNumOfQueuedUpPreflips;
		RECT m_rcClip;

		IDirect3DSurface9 *m_pEAPI;
		IDirect3DSurface9 *m_pSORT;
		IDirect3DSurface9 *m_pRenderTarget;
	};
}
#endif
