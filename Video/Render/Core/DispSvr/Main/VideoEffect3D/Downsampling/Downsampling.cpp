#include "../../stdafx.h"
#include <InitGuid.h>
#include "Downsampling.h"

using namespace DispSvr;

CDownsamplingEffect::CDownsamplingEffect()
{
	m_cRef = 1;
	m_pDevice = NULL;
	m_pDownsamplingTexture = NULL;
	m_bEnabled = FALSE;
    m_bNonPow2TextureSupported = FALSE;
    _CleanUpDownsamplingTexture();
}

CDownsamplingEffect::~CDownsamplingEffect()
{
    _CleanUpDownsamplingTexture();

	if (m_pDevice)
	{
		ASSERT(m_pDevice == NULL);
		m_pDevice->Release();
		m_pDevice = NULL;
	}
}

HRESULT CDownsamplingEffect::Create(IDispSvrVideoEffect3DPlugin **pEffectPlugin)
{
    *pEffectPlugin = new CDownsamplingEffect;
    if (*pEffectPlugin)
        return S_OK;
    return E_OUTOFMEMORY;
}

//////////////////////////////////////////////////////////////////////////
// IUnknown
STDMETHODIMP CDownsamplingEffect::QueryInterface(REFIID riid, void **ppv)
{
    HRESULT hr = E_NOINTERFACE;

    if (riid == __uuidof(IDispSvrVideoEffect3DPlugin))
    {
        hr = GetInterface(static_cast<IDispSvrVideoEffect3DPlugin *> (this), ppv);
    }
    else if (riid == __uuidof(IUnknown))
    {
        hr = GetInterface(static_cast<IUnknown *> (this), ppv);
    }
    return hr;
}

STDMETHODIMP_(ULONG) CDownsamplingEffect::AddRef()
{
    LONG lRef = 0;
    lRef = InterlockedIncrement(&m_cRef);
    ASSERT(lRef > 0);
    return lRef;
}

STDMETHODIMP_(ULONG) CDownsamplingEffect::Release()
{
    LONG lRef = InterlockedDecrement(&m_cRef);
    ASSERT(lRef >= 0);
    if (lRef == 0)
        delete this;
    return lRef;
}

//////////////////////////////////////////////////////////////////////////
// IDispSvrVideoEffect3DPlugin
STDMETHODIMP CDownsamplingEffect::ProcessMessage(VE3D_MESSAGE_TYPE eMessage, LPVOID ulParam)
{
    HRESULT hr = E_NOTIMPL;

    switch (eMessage)
    {
    case VE3D_MESSAGE_TYPE_INITIALIZE:
        if (ulParam)
            hr = _Initialize(reinterpret_cast<IDispSvrVideoEffect3DManager *> (ulParam));
        break;

    case VE3D_MESSAGE_TYPE_SETDEVICE:
        if (ulParam)
            hr = _SetDevice(reinterpret_cast<IUnknown *> (ulParam));
        break;

    case VE3D_MESSAGE_TYPE_RELEASEDEVICE:
        hr = _ReleaseDevice();
        break;

    case VE3D_MESSAGE_TYPE_ENABLE:
        if (ulParam)
            hr = _Enable(*reinterpret_cast<BOOL *> (ulParam));
        break;
    }

    return hr;
}

STDMETHODIMP CDownsamplingEffect::GetResourceId(GUID *pGUID)
{
    if (!pGUID)
        return E_POINTER;
    *pGUID = DispSvr_VideoEffectDownsampling;
    return S_OK;
}

STDMETHODIMP CDownsamplingEffect::SetParam(DWORD dwParam, DWORD dwValue)
{
    return E_NOTIMPL;
}

STDMETHODIMP CDownsamplingEffect::GetParam(DWORD dwParam, DWORD *pdwValue)
{
    if (pdwValue)
        *pdwValue = NULL;

    return E_NOTIMPL;
}

STDMETHODIMP CDownsamplingEffect::ProcessEffect(VE3DBuffer *pInput, VE3DBuffer *pOutput)
{
    HRESULT hr = E_FAIL;
    if (!pInput || !pOutput || !pInput->pSurface)
        return E_POINTER;

	if (!m_pDevice)
		return E_UNEXPECTED;

	if (!m_bEnabled)
		return S_FALSE;

    hr = _PrepareDownsamplingTexture( (pInput->uWidth / 2), (pInput->uHeight / 2));
    if (FAILED(hr))
        return hr;

    CComPtr<IDirect3DSurface9> pSrcSurface;
    CComPtr<IDirect3DSurface9> pDestSurface;
    hr = pInput->pSurface->QueryInterface(IID_IDirect3DSurface9, (VOID **)&pSrcSurface);
    if (FAILED(hr) || !pSrcSurface) // input surface is not Direct3DSurface9
    {
        CComPtr<IDirect3DTexture9> pSrcTexture;
        hr = pInput->pSurface->QueryInterface(IID_IDirect3DTexture9, (VOID **)&pSrcTexture);
        if (FAILED(hr) || !pSrcTexture)
        {
            return E_UNEXPECTED;
        }
        hr = pSrcTexture->GetSurfaceLevel( 0, &pSrcSurface);
    }

    if (m_pDownsamplingTexture)
    {
        m_pDownsamplingTexture->GetSurfaceLevel( 0, &pDestSurface);
    }
    else
    {
        return E_UNEXPECTED;
    }

    if (!pSrcSurface || !pDestSurface)
    {
        return E_UNEXPECTED;
    }

    m_uVideoWidth = pInput->uWidth / 2;
    m_uVideoHeight = pInput->uHeight / 2;

    RECT SrcRECT = { 0, 0, pInput->uWidth, pInput->uHeight};
    RECT DestRECT = { 0, 0, m_uVideoWidth, m_uVideoHeight};
    hr = m_pDevice->StretchRect( pSrcSurface, &SrcRECT, pDestSurface, &DestRECT, D3DTEXF_POINT);

    if (SUCCEEDED(hr)) // Downsampling done.
    {
        pOutput->uWidth = m_uVideoWidth;
        pOutput->uHeight = m_uVideoHeight;
        pOutput->pSurface = m_pDownsamplingTexture;
        pOutput->rcSource = DestRECT;
    }
    return hr;
}

HRESULT CDownsamplingEffect::_Initialize(IDispSvrVideoEffect3DManager *pManager)
{
	VE3DManagerCaps caps;
	HRESULT hr = pManager->GetCaps(&caps);

	if (SUCCEEDED(hr))
	{
		if (caps.eType != EFFECT_MANAGER_TYPE_SHADER_BASED)
			return E_FAIL;
	}
	return hr;
}

HRESULT CDownsamplingEffect::_SetDevice(IUnknown *pDevice)
{
    CComPtr<IDirect3DDevice9> pNewDevice;
	HRESULT hr = pDevice->QueryInterface(__uuidof(IDirect3DDevice9), (void **)&pNewDevice);
    if (SUCCEEDED(hr) && pNewDevice)
    {
        if (pNewDevice != m_pDevice) // release old device
        {
            _ReleaseDevice();
            m_pDevice = pNewDevice.Detach();
            D3DCAPS9 Caps9;
            hr = m_pDevice->GetDeviceCaps(&Caps9);
            if ((Caps9.TextureCaps & D3DPTEXTURECAPS_POW2) == 0) // Device supports non Pow-2 texture.
            {
                m_bNonPow2TextureSupported = TRUE;
            }
        }
    }
	return hr;
}

HRESULT CDownsamplingEffect::_ReleaseDevice()
{
    _CleanUpDownsamplingTexture();

	if (m_pDevice)
	{
		m_pDevice->Release();
		m_pDevice = NULL;
	}
	return S_OK;
}

HRESULT CDownsamplingEffect::_Enable(BOOL bEnable)
{
	m_bEnabled = bEnable;
	return S_OK;
}

HRESULT CDownsamplingEffect::_PrepareDownsamplingTexture(UINT uVideoWidth, UINT uVideoHeight)
{
    HRESULT hr = E_FAIL;
    if ((uVideoWidth <= m_uTextureWidth) &&
        (uVideoHeight <= m_uTextureHeight) &&
        m_pDownsamplingTexture)
    {
        hr = S_OK;
    }
    else // need to create new texture
    {
        _CleanUpDownsamplingTexture();
        if (!m_pDevice)
        {
            return E_UNEXPECTED;
        }

        if (m_bNonPow2TextureSupported)
        {
            hr = m_pDevice->CreateTexture(uVideoWidth, uVideoHeight, 1, 
                                                            D3DUSAGE_RENDERTARGET,
                                                            D3DFMT_X8R8G8B8,
                                                            D3DPOOL_DEFAULT,
                                                            &m_pDownsamplingTexture, NULL);
            if (SUCCEEDED(hr) && m_pDownsamplingTexture)
            {
                m_uTextureWidth = uVideoWidth;
                m_uTextureHeight = uVideoHeight;
            }
        }

        if (!m_pDownsamplingTexture) // create Pow-2 texture if non Pow-2 texture failed or not supported.
        {
            UINT TextureWidth = 1;
            UINT TextureHeight = 1;

            while (uVideoWidth < TextureWidth)
            {
                TextureWidth <<= 1;
            }

            while (uVideoHeight < TextureHeight)
            {
                TextureHeight <<= 1;
            }

            hr = m_pDevice->CreateTexture(TextureWidth, TextureHeight, 1,
                                                            D3DUSAGE_RENDERTARGET,
                                                            D3DFMT_X8R8G8B8,
                                                            D3DPOOL_DEFAULT,
                                                            &m_pDownsamplingTexture, NULL);
            if (SUCCEEDED(hr) && m_pDownsamplingTexture)
            {
                m_uTextureWidth = TextureWidth;
                m_uTextureHeight = TextureHeight;
            }
            else
            {
                return E_FAIL;
            }
        }
    }

	return hr;
}

HRESULT CDownsamplingEffect::_CleanUpDownsamplingTexture()
{       
	m_uTextureWidth = 0;
	m_uTextureHeight = 0;
	m_uVideoWidth = 0;
	m_uVideoHeight = 0;

	if (m_pDownsamplingTexture)
	{
		m_pDownsamplingTexture->Release();
		m_pDownsamplingTexture = NULL;
	}
	return S_OK;
}