#ifndef _DISPSVR_D3D9_VIDEO_PRESENTER_H_
#define _DISPSVR_D3D9_VIDEO_PRESENTER_H_

#include "D3D9VideoPresenterBase.h"

namespace DispSvr
{
    interface IDriverExtensionAdapter;
	class CD3D9VideoPresenter : public CD3D9VideoPresenterBase
	{
	public:
		CD3D9VideoPresenter();
		virtual ~CD3D9VideoPresenter();

		STDMETHOD(SetDisplayRect)(const RECT *rcDst, const RECT *rcSrc);
		STDMETHOD(EndRender)();
		STDMETHOD(Clear)();
		STDMETHOD(Present)(const PresentHints *pHints);
		STDMETHOD(SetScreenCaptureDefense)(BOOL bEnable);

	protected:
		STDMETHOD(_SetDevice)(IUnknown *pDevice);
		STDMETHOD(_ReleaseDevice)();
        STDMETHOD(_QueryCaps)(PresenterCaps* pCaps);

	protected:
		void WaitUntilGpuIdle();
		HRESULT EnableDwmQueuing(HWND hwnd);
		HRESULT D3D9Present(const RECT *pSourceRect, const RECT *pDestRect);

	private:
		enum D3D9_PRESENT_MODE
		{
			D3D9_PRESENT_MODE_WINDOWED = 0,
			D3D9_PRESENT_MODE_FULLSCREEN = 1,
			D3D9_PRESENT_MODE_FLIPEX = 2
		} m_ePresentMode;
		bool m_bNeedEnableDwmQueuing;
		bool m_bDwmEnableMMCSS;
		IDirect3DSwapChain9 *m_pSwapChain;
        IDriverExtensionAdapter *m_pDriverExtAdapter;
	};
}
#endif
