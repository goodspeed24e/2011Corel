#ifndef _DISPSVR_D3D9_OVERLAY_PRESENTER_H_
#define _DISPSVR_D3D9_OVERLAY_PRESENTER_H_

#include "D3D9VideoPresenterBase.h"

namespace DispSvr
{
	interface IDriverExtensionAdapter;
	class CD3D9OverlayPresenter : public CD3D9VideoPresenterBase
	{
	public:
		CD3D9OverlayPresenter();
		virtual ~CD3D9OverlayPresenter();

		// IDispSvrPlugin
		STDMETHOD(ProcessMessage)(RESOURCE_MESSAGE_TYPE msg, LPVOID ulParam);

		// IDispSvrVideoPresenter
		STDMETHOD(SetDisplayRect)(const RECT *rcDst, const RECT *rcSrc);
		STDMETHOD(Present)(const PresentHints *pHints);
		STDMETHOD(SetScreenCaptureDefense)(BOOL bEnable);
		STDMETHOD(SetGamutMetadata)(const DWORD dwFormat, void *pGamutMetadata);

	protected:
		STDMETHOD(_SetDevice)(IUnknown *pDevice);
		STDMETHOD(_ReleaseDevice)();
		STDMETHOD(_QueryCaps)(PresenterCaps* pCaps);

	private:
		BOOL m_bHideOverlay;
		BOOL m_bUpdateColorkey;
		BOOL m_bUpdateColorkeyOnly;
		IDriverExtensionAdapter *m_pDeviceCap;
	};
}
#endif	// _DISPSVR_D3D9_OVERLAY_PRESENTER_H_
