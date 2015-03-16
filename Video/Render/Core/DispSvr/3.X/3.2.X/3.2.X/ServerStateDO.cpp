#include "stdafx.h"
#include "ServerStateDO.h"
#include "Imports/LibD3D/d3dutil.h"


CServerStateDO::CServerStateDO(LPUNKNOWN pUnk, HRESULT *phr)
        : CUnknown(NAME("Server State DO"), pUnk)
{
    m_pFont = NULL;
	m_dwLastUpdate = 0;
	ZeroMemory(m_szDispSvr, sizeof(m_szDispSvr));
	ZeroMemory(m_szRuntime, sizeof(m_szRuntime));
}

CServerStateDO::~CServerStateDO()
{
    Terminate();
}

STDMETHODIMP CServerStateDO::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
    HRESULT hr = E_NOINTERFACE;
    *ppv = NULL;

    if (riid == IID_IDisplayObject)
    {
        hr = GetInterface((IDisplayObject *)this, ppv);
    }
    else
    {
        hr = CUnknown::NonDelegatingQueryInterface(riid, ppv);
    }
    return hr;
}

//////////////////////////////////////////////////////////////////////////
// IDisplayObject

STDMETHODIMP CServerStateDO::Initialize(IDisplayRenderEngine* pRenderEngine)
{
    if (m_pOwner) 
    {
        return S_FALSE;
    }

    if (!pRenderEngine)
    {
        return E_POINTER;
    }

    try
    {
        HRESULT hr;

        CComPtr<IUnknown> pDevice;
        hr = pRenderEngine->Get3DDevice(&pDevice);

		CComQIPtr<IDirect3DDevice9> pDevice9 = pDevice;
		if (!pDevice9)
			throw E_FAIL;

        hr = InitFont(pDevice9);

        m_pOwner = pRenderEngine;
		m_pOwner->QueryInterface(IID_IDisplayServerStateEventSink, (void **)&m_pStateEventSink);
    }
    catch (HRESULT hrFailed)
    {
        return hrFailed;
    }

    return S_OK;
}

STDMETHODIMP CServerStateDO::Terminate()
{
    UninitFont();

    m_pStateEventSink.Release();
    m_pOwner.Release();
    
    return S_OK;
}

STDMETHODIMP CServerStateDO::GetRenderEngineOwner(IDisplayRenderEngine** ppRenderEngine)
{
    if (!ppRenderEngine)
    {
        return E_POINTER;
    }
    if (!m_pOwner)
    {
        return VFW_E_NOT_FOUND;
    }

    *ppRenderEngine = m_pOwner;
    (*ppRenderEngine)->AddRef();

    return S_OK;
}

STDMETHODIMP CServerStateDO::ProcessMessage(HWND hWnd, UINT msg, UINT wParam, LONG lParam)
{
    return E_NOTIMPL;
}

STDMETHODIMP CServerStateDO::NotifyEvent(DWORD dwEvent, DWORD param1, DWORD param2, LPVOID pInstance)
{
	return E_NOTIMPL;
}

STDMETHODIMP CServerStateDO::Render(IUnknown* pDevice, const NORMALIZEDRECT* lpParentRect)
{
    CAutoLock Lock(&m_ObjectLock);

    if (!m_pOwner || !m_pFont || !m_pStateEventSink)
        return VFW_E_WRONG_STATE;

	DWORD dwCurrent = timeGetTime();
	if (dwCurrent - m_dwLastUpdate > 33)
	{
		m_pStateEventSink->GetStateText(m_szRuntime, sizeof(m_szRuntime) / sizeof(m_szRuntime[0]));
		m_dwLastUpdate = dwCurrent;
	}

	m_pFont->DrawText(0, 10, D3DCOLOR_ARGB(255,180,255,255), m_szDispSvr);
	m_pFont->DrawText(0, 30, D3DCOLOR_ARGB(180,255,255,255), m_szRuntime);
    return S_OK;
}

HRESULT CServerStateDO::InitFont(IDirect3DDevice9* pDevice)
{
    if (m_pFont)
    {
        return S_FALSE;
    }

    try
    {
        HRESULT hr;

        m_pFont = new CD3DFont( _T("Trebuchet MS"), 12, D3DFONT_BOLD);

        hr = m_pFont->InitDeviceObjects(pDevice);
        hr = m_pFont->RestoreDeviceObjects();

        hr = pDevice->GetDeviceCaps(&m_d3dCaps);
		hr = pDevice->GetDisplayMode(0, &m_d3dDisplayMode);

        // Store render target surface description
        CComPtr<IDirect3DSurface9> pBackBuffer;
        hr = pDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer);
        hr = pBackBuffer->GetDesc(&m_d3dsdBackBuffer);
		_sntprintf_s(m_szDispSvr, sizeof(m_szDispSvr) / sizeof(m_szDispSvr[0]), 100, TEXT("%dx%d, Backbuf %s, Adapter %s"),
			m_d3dsdBackBuffer.Width, m_d3dsdBackBuffer.Height,
			D3DUtil_D3DFormatToString(m_d3dsdBackBuffer.Format, false), 
			D3DUtil_D3DFormatToString(m_d3dDisplayMode.Format, false));
		m_szDispSvr[99] = '\0';

    }
    catch (HRESULT hrFailed)
    {
        delete m_pFont;
        m_pFont = NULL;
        return hrFailed;
    }

    return S_OK;
}

HRESULT CServerStateDO::UninitFont()
{
    if (m_pFont)
    {
        m_pFont->InvalidateDeviceObjects();
        m_pFont->DeleteDeviceObjects();
        delete m_pFont;
        m_pFont = NULL;
    }

    return S_OK;
}

STDMETHODIMP CServerStateDO::BeginDeviceLoss(void)
{
    CAutoLock Lock(&m_ObjectLock);

    return UninitFont();
}

STDMETHODIMP CServerStateDO::EndDeviceLoss(IUnknown* pDevice)
{
    CAutoLock Lock(&m_ObjectLock);

	CComQIPtr<IDirect3DDevice9> pDevice9 = pDevice;
    return InitFont(pDevice9);
}

STDMETHODIMP CServerStateDO::GetCLSID(
    CLSID* pClsid)
{
    if (!pClsid)
    {
        return E_POINTER;
    }

    *pClsid = CLSID_ServerStateDisplayObject;
    return S_OK;
}
