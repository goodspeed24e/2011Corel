#pragma once
#ifndef _DISPSVR_S3API_VIDEO_PRESENTER_
#define _DISPSVR_S3API_VIDEO_PRESENTER_

#include "D3D9VideoPresenterBase.h"

typedef struct _S3DXVA_Encrypt S3DXVA_Encrypt;
typedef union _S3G_OVERLAYFLAGS S3G_OVERLAYFLAGS;

#define S3_OVERLAYDISPLAY_FUNCTION			0xFFFF0000 //(EncryptProtocolFlag = 0xFFFF00 Host and bDXVA_Func = 0)
#define S3_INITIALIZATION_SUCCESS			0xFFFF0801

#define S3_DXVA_MASK						0x0C000000
#define S3_TWO_HD_DXVA						0x0C000000
#define S3_ONE_HD_ONE_SD_DXVA				0x08000000
#define S3_ONE_HD_DXVA						0x04000000

#define S3_60FPS							0x02000000

#define S3_RES_MASK							0x000C0000
#define S3_RES_2560x1600					0x000C0000
#define S3_RES_2048x1536					0x00080000
#define S3_RES_1920x1200					0x00040000

namespace DispSvr
{
	class CS3APIPresenter : public CD3D9VideoPresenterBase
	{
	public:
		CS3APIPresenter();
		virtual ~CS3APIPresenter();

		STDMETHOD(SetDisplayRect)(const RECT *rcDst, const RECT *rcSrc);
		STDMETHOD(Present)(const PresentHints *pHints);
		STDMETHOD(SetColorKey)(const DWORD dwColorKey);

	protected:
		STDMETHOD(_SetDevice)(IUnknown *pDevice);
		STDMETHOD(_ReleaseDevice)();
		STDMETHOD(_QueryCaps)(PresenterCaps* pCaps);

		HRESULT InitS3API();		
		HRESULT	CreateOverlay();
		HRESULT EnableOverlay(bool bEnable);
		HRESULT UpdateOverlay(S3G_OVERLAYFLAGS	wOverlayFlags);
		HRESULT FlipOverlay();
		HRESULT FlushD3D();

	private:
		S3DXVA_Encrypt *m_pS3EncryptBitstream;
		DWORD m_dwGPUCaps;
	};
}
#endif
