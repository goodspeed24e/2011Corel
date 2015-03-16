#include "stdafx.h"
#include "DispSvr.h"
#include "ResourceManager.h"
#include <dxva.h>
#include "Imports/ThirdParty/S3/VideoProtection/S3dxvaEncrypt.h"
#include "S3APIPresenter.h"

using namespace DispSvr;

static const DWORD g_dwDecoderID = '0IVI', g_dwAcceleratorID = 'S3AC';
static const DWORD g_dwCurrBuff = 0, g_dwTargetBuff = 1;
static const GUID S3Encrypt_GUID = {0x1ff1beD1, 0xa0c7,  0x11d3,  {0xb9, 0x84, 0x00, 0xc0, 0x4f, 0x2e, 0x73, 0xc5}};
static const DXVA_EncryptProtocolFunc g_EncryptFunc = S3_OVERLAYDISPLAY_FUNCTION;

CS3APIPresenter::CS3APIPresenter()
{
	m_GUID = DISPSVR_RESOURCE_S3APIVIDEOPRESENTER;
	m_pS3EncryptBitstream = NULL;
	m_dwGPUCaps = S3_ONE_HD_DXVA;  //We assume the ASIC has one DXVA at least. 
	m_PresenterCaps.bCanVirtualizeFromOrigin = TRUE;
}

CS3APIPresenter::~CS3APIPresenter()
{
}

STDMETHODIMP CS3APIPresenter::_SetDevice(IUnknown *pDevice)
{
	HRESULT hr = E_FAIL;

	hr = InitS3API();
	if (FAILED(hr))
		return hr;

	IDirect3DDevice9* pDevice9 = NULL;
	hr = pDevice->QueryInterface(__uuidof(IDirect3DDevice9), (void **)&pDevice9);
	if (FAILED(hr))
		return hr;

	if (pDevice9 != m_pDevice)
	{
		hr = CD3D9VideoPresenterBase::_SetDevice(pDevice);
		if (SUCCEEDED(hr))
		{
			hr = CreateOverlay();
			if (SUCCEEDED(hr))
			{
				// Update presenter caps
				m_PresenterCaps.bIsOverlay = TRUE;
				m_PresenterCaps.dwFPS = (m_dwGPUCaps & S3_60FPS) ? 60:30;
				switch(m_dwGPUCaps & S3_RES_MASK)
				{
				case S3_RES_2560x1600:
					m_PresenterCaps.dwResPixels = 4096000; //2560x1600
					break;
				case S3_RES_2048x1536:
					m_PresenterCaps.dwResPixels = 3145728; //2048x1536
					break;
				case S3_RES_1920x1200:
					m_PresenterCaps.dwResPixels = 2304000; //1920x1200
					break;
				default:
					m_PresenterCaps.dwResPixels = 2073600; //19x10/16x12
					break;
				}
			}
		}
	}

	pDevice9->Release();
	return hr;
}

STDMETHODIMP CS3APIPresenter::_ReleaseDevice()
{
	HRESULT hr = EnableOverlay(false);
	if (FAILED(hr))
		ASSERT(hr==E_HANDLE);
	m_pS3EncryptBitstream = NULL;
	return CD3D9VideoPresenterBase::_ReleaseDevice();
}

STDMETHODIMP CS3APIPresenter::SetDisplayRect(const RECT *rcDst, const RECT *rcSrc)
{
	HRESULT	hr = CD3D9VideoPresenterBase::SetDisplayRect(rcDst, rcSrc);
	if (SUCCEEDED(hr))
	{
		S3G_OVERLAYFLAGS wOverlayFlags = {0};
		wOverlayFlags.Flags.SrcRectChanged = true;
		wOverlayFlags.Flags.DstRectChanged = true;
		hr = UpdateOverlay(wOverlayFlags);
	}
	return hr;
}

STDMETHODIMP CS3APIPresenter::Present(const PresentHints *pHints)
{
	return FlipOverlay();
}

STDMETHODIMP CS3APIPresenter::SetColorKey(const DWORD dwColorKey)
{
	HRESULT hr = CD3D9VideoPresenterBase::SetColorKey(dwColorKey);
	if (SUCCEEDED(hr))
	{
		S3G_OVERLAYFLAGS wOverlayFlags = {0};
		wOverlayFlags.Flags.DstColorKey = true;
		hr = UpdateOverlay(wOverlayFlags);
	}

	return hr;
}

STDMETHODIMP CS3APIPresenter::_QueryCaps(PresenterCaps* pCaps)
{
	if(pCaps->VideoDecodeCaps > 0)
	{
		switch(m_dwGPUCaps & S3_DXVA_MASK)
		{
		case S3_TWO_HD_DXVA: // no restriction
			m_PresenterCaps.bHwDecode = TRUE;
			break;
		case S3_ONE_HD_ONE_SD_DXVA: //1 HD + 1 SD streams
			if((pCaps->VideoDecodeCaps & VIDEO_CAP_STREAM_SUB) && 
				(pCaps->VideoDecodeCaps & (VIDEO_CAP_FORMAT_1080|VIDEO_CAP_FORMAT_720)))
				m_PresenterCaps.bHwDecode = FALSE;
			else
				m_PresenterCaps.bHwDecode = TRUE;
			break;			
		case S3_ONE_HD_DXVA: //one HD stream
			m_PresenterCaps.bHwDecode = (pCaps->VideoDecodeCaps & VIDEO_CAP_STREAM_SUB) ? FALSE : TRUE;
			break;
		default:
			m_PresenterCaps.bHwDecode = FALSE;
			break;
		}
	}

	return S_OK;
}

HRESULT CS3APIPresenter::InitS3API()
{
	int nRet;
	ULONG EscType = S3_OVERLAYPROTECTION_FUNCTION;
	// Does Escape support overlay?
	nRet = S3VideoExtEscape(
		QUERYESCSUPPORT,
		sizeof(ULONG),
		(char *)&EscType,
		0,
		NULL);

	if (nRet <= 0)
		return E_FAIL;
	else
		return S_OK;
}

HRESULT CS3APIPresenter::CreateOverlay()
{
	int nRet = 0;
	DXVA_EncryptProtocolHeader EncryptInput = {0};
	DXVA_EncryptProtocolHeader EncryptOutput= {0};
	EncryptInput.dwFunction = g_EncryptFunc;

	if (!m_pS3EncryptBitstream)
	{
		nRet = S3VideoExtEscape(
			S3_OVERLAYPROTECTION_FUNCTION,
			sizeof(EncryptInput),
			(char *)&EncryptInput,
			sizeof(EncryptOutput),
			(char *)&EncryptOutput);

		if (S3_VIDEOPROTECTION_SUCCESS != nRet)
			return E_FAIL;

		m_pS3EncryptBitstream = *(S3DXVA_Encrypt**)&EncryptOutput.ReservedBits[0];
		if (EncryptOutput.dwFunction == S3_INITIALIZATION_SUCCESS)
			m_dwGPUCaps = EncryptOutput.ReservedBits[2];
	}

	HRESULT hr = E_FAIL;
	hr = EnableOverlay(true);
	if (FAILED(hr))
		return hr;

#define GetSource
#ifdef GetSource
	S3G_OVERLAYFLAGS wOverlayFlags = {0}; 
	wOverlayFlags.Flags.SrcRectChanged = true;
	hr = UpdateOverlay(wOverlayFlags);
	if (FAILED(hr))
		return hr;
#endif //GetSource
	return S_OK;
}

HRESULT CS3APIPresenter::EnableOverlay(bool bEnable)
{
	if (!m_pS3EncryptBitstream)
		return E_HANDLE; // has not been authenticated!

	DXVA_EncryptProtocolHeader EncryptInput = {0};
	DXVA_EncryptProtocolHeader EncryptOutput= {0};

	//Input
	EncryptInput.dwFunction = g_EncryptFunc;
	memcpy(&EncryptInput.ReservedBits[0], &m_pS3EncryptBitstream, sizeof(ULONG_PTR));    
	memcpy(&EncryptInput.guidEncryptProtocol, &S3Encrypt_GUID, sizeof(GUID));
	m_pS3EncryptBitstream->dwDecoderID = g_dwDecoderID;		//Decoder fourCC
	m_pS3EncryptBitstream->dwAcceleratorID = g_dwAcceleratorID;
	m_pS3EncryptBitstream->dwMethod = METHOD_DISPLAY_OVERLAY;  
	m_pS3EncryptBitstream->dwValidation = bEnable ? CURR_DISPLAY_OVERLAY_ENABLE : CURR_DISPLAY_OVERLAY_DISABLE;    

	int nRet = S3VideoExtEscape(
		S3_OVERLAYPROTECTION_FUNCTION,
		sizeof(EncryptInput),
		(char *)&EncryptInput,
		sizeof(EncryptOutput),
		(char *)&EncryptOutput);

	if (S3_VIDEOPROTECTION_SUCCESS == nRet)
		return S_OK;
	else
		return E_FAIL; 
}

HRESULT CS3APIPresenter::UpdateOverlay(S3G_OVERLAYFLAGS	wOverlayFlags)
{
	if (!m_pS3EncryptBitstream)
		return E_HANDLE; // has not been authenticated!

	DXVA_EncryptProtocolHeader EncryptInput = {0};
	DXVA_EncryptProtocolHeader EncryptOutput= {0};

	//Input
	EncryptInput.dwFunction = g_EncryptFunc;
	memcpy(&EncryptInput.ReservedBits[0], &m_pS3EncryptBitstream, sizeof(ULONG_PTR));    
	memcpy(&EncryptInput.guidEncryptProtocol, &S3Encrypt_GUID, sizeof(GUID));
	m_pS3EncryptBitstream->dwDecoderID = g_dwDecoderID;
	m_pS3EncryptBitstream->dwAcceleratorID = g_dwAcceleratorID;
	m_pS3EncryptBitstream->dwMethod = METHOD_DISPLAY_OVERLAY;  
	m_pS3EncryptBitstream->dwValidation = CURR_DISPLAY_OVERLAY_UPDATE;   
	m_pS3EncryptBitstream->Overlay.OverlayFlags.Value = wOverlayFlags.Value;
	m_pS3EncryptBitstream->Overlay.CurrBuffIndex =	g_dwCurrBuff;

	if(wOverlayFlags.Flags.SrcRectChanged)
	{
		m_pS3EncryptBitstream->Overlay.SrcRectTop = static_cast<WORD>(m_rcSrc.top);
		m_pS3EncryptBitstream->Overlay.SrcRectLeft = static_cast<WORD>(m_rcSrc.left);
		m_pS3EncryptBitstream->Overlay.SrcRectBottom = static_cast<WORD>(m_rcSrc.bottom);
		m_pS3EncryptBitstream->Overlay.SrcRectRight = static_cast<WORD>(m_rcSrc.right);
	}

	if(wOverlayFlags.Flags.DstRectChanged)
	{
		m_pS3EncryptBitstream->Overlay.DstRectTop =	static_cast<WORD>(m_rcDst.top);
		m_pS3EncryptBitstream->Overlay.DstRectLeft = static_cast<WORD>(m_rcDst.left);  
		m_pS3EncryptBitstream->Overlay.DstRectBottom = static_cast<WORD>(m_rcDst.bottom);
		m_pS3EncryptBitstream->Overlay.DstRectRight = static_cast<WORD>(m_rcDst.right);
	}
	m_pS3EncryptBitstream->Overlay.DstColorKey = m_dwColorKey; 

	int nRet = S3VideoExtEscape(
		S3_OVERLAYPROTECTION_FUNCTION,
		sizeof(EncryptInput),
		(char *)&EncryptInput,
		sizeof(EncryptOutput),
		(char *)&EncryptOutput);

	if (S3_VIDEOPROTECTION_SUCCESS != nRet)
		return E_FAIL; 

	return FlushD3D();
}

HRESULT CS3APIPresenter::FlipOverlay()
{
	if (!m_pS3EncryptBitstream)
		return E_HANDLE; // has not been authenticated!

	DXVA_EncryptProtocolHeader EncryptInput = {0};
	DXVA_EncryptProtocolHeader EncryptOutput= {0};

	//Input
	EncryptInput.dwFunction = g_EncryptFunc;
	memcpy(&EncryptInput.ReservedBits[0], &m_pS3EncryptBitstream, sizeof(ULONG_PTR));    
	memcpy(&EncryptInput.guidEncryptProtocol, &S3Encrypt_GUID, sizeof(GUID));
	m_pS3EncryptBitstream->dwDecoderID = g_dwDecoderID;
	m_pS3EncryptBitstream->dwAcceleratorID = g_dwAcceleratorID;
	m_pS3EncryptBitstream->dwMethod = METHOD_DISPLAY_OVERLAY;  
	m_pS3EncryptBitstream->dwValidation = CURR_DISPLAY_OVERLAY_FLIP;   
	m_pS3EncryptBitstream->Flip.TargetBuffIndex = g_dwTargetBuff;
	m_pS3EncryptBitstream->Flip.CurrBuffIndex = g_dwCurrBuff;    

	int nRet = S3VideoExtEscape(
		S3_OVERLAYPROTECTION_FUNCTION,
		sizeof(EncryptInput),
		(char *)&EncryptInput,
		sizeof(EncryptOutput),
		(char *)&EncryptOutput);

	if (S3_VIDEOPROTECTION_SUCCESS != nRet)
		return E_FAIL; 

	return FlushD3D();
}

HRESULT CS3APIPresenter::FlushD3D()
{
	// When DWM is on, present does not flip to front buffer but to an intermediate buffer and then
	// composited while DWM compositing. In this case we should set source rectangle to have
	// the correct first composition.
	const RECT *pSrcRect = m_rcSrc.left == m_rcSrc.right || m_rcSrc.top == m_rcSrc.bottom ? NULL : &m_rcSrc;

	// Need to call a D3D present to flush d3d commands in runtime.
	HRESULT hr = m_pDevice->Present(pSrcRect, 0, m_hwnd, 0);
	return hr;
}