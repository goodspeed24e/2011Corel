#ifndef _DISPSVR_D3D9_VIDEO_PRESENTER_H_
#define _DISPSVR_D3D9_VIDEO_PRESENTER_H_

#include "D3D9VideoPresenterBase.h"

namespace DispSvr
{
	class CD3D9VideoPresenter : public CD3D9VideoPresenterBase
	{
	public:
		CD3D9VideoPresenter();
		virtual ~CD3D9VideoPresenter();

		STDMETHOD(SetDisplayRect)(const RECT *rcDst, const RECT *rcSrc);
		STDMETHOD(EndRender)();
		STDMETHOD(Clear)();
		STDMETHOD(Present)(const PresentHints *pHints);
		STDMETHOD(SetProperty)(const PresenterProperty *pProperty);
		STDMETHOD(SetScreenCaptureDefense)(BOOL bEnable);

		STDMETHOD(ProcessMessage)(RESOURCE_MESSAGE_TYPE msg, LPVOID ulParam);

	protected:
		STDMETHOD(_SetDevice)(IUnknown *pDevice);
		STDMETHOD(_ReleaseDevice)();
		STDMETHOD(_ResetDevice)();

	protected:
		void WaitUntilGpuIdle();
		HRESULT SetDwmQueuing(HWND hwnd);
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
        BOOL m_bDWMEnable;
		BOOL m_bDWMQueueEnabled;
        bool m_bFullScreen;
        DWORD m_dwOSVersion;
	};
}
#endif
