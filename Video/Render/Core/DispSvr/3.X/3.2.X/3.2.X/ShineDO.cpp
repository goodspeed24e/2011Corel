#include "stdafx.h"
#include "ShineDO.h"
#include "Imports/LibD3D/d3dutil.h"

#include <algorithm>

// CShineDO


CShineDO::CShineDO(LPUNKNOWN pUnk, HRESULT *phr)
        : CUnknown(NAME("Shine DO"), pUnk)
{
    m_dwLastTextureTime = 0;
    m_iTexture = 0;
}

CShineDO::~CShineDO()
{
    Terminate();
}

STDMETHODIMP CShineDO::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
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

struct ShineVertex
{
	void Init(float x, float y, float z, D3DCOLOR color, float tu, float tv)
	{
		position.x = x;
		position.y = y;
		position.z = z;
		this->color = color;
		this->tu = tu;
		this->tv = tv;
	}

    D3DVECTOR position;
    D3DCOLOR color;
    FLOAT tu;
    FLOAT tv;

    enum 
    {
        FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1
    };
};

#define SHINE_W 0.3f
#define SHINE_H 0.4f

void MoveQuadVertices(ShineVertex* pVertices, int x, int y, int w, int h)
{
	pVertices[0].Init(
		-1.0f + (float) x / (w/2) - SHINE_W/2,
		1.0f - (float) y / (h/2) + SHINE_H/2,
		0.0f,
		D3DCOLOR_ARGB(100,255,255,255),
		0.0f,
		0.0f);
	pVertices[1].Init(
		pVertices[0].position.x + SHINE_W,
		pVertices[0].position.y,
		0.0f,
		D3DCOLOR_ARGB(100,255,255,255),
		1.0f,
		0.0f);
	pVertices[2].Init(
		pVertices[0].position.x,
		pVertices[0].position.y - SHINE_H,
		0.0f,
		D3DCOLOR_ARGB(100,255,255,255),
		0.0f,
		1.0f);
	pVertices[3].Init(
		pVertices[1].position.x,
		pVertices[2].position.y,
		0.0f,
		D3DCOLOR_ARGB(100,255,255,255),
		1.0f,
		1.0f);
}

STDMETHODIMP CShineDO::Initialize(IDisplayRenderEngine* pRenderEngine)
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
        CComPtr<IUnknown> pDevice;
        HRESULT hr = pRenderEngine->Get3DDevice(&pDevice);

		CComQIPtr<IDirect3DDevice9> pDevice9 = pDevice;
		if (!pDevice9)
			throw E_FAIL;

        hr = AllocDeviceResources(pDevice9);

		m_pOwner = pRenderEngine;

		OnMouseMove(0, 0);
    }
    catch (HRESULT hrFailed)
    {
        return hrFailed;
    }

    return S_OK;
}

HRESULT CShineDO::AllocDeviceResources(IDirect3DDevice9* pDevice)
{
#ifdef _NO_USE_D3DXDLL
	return E_NOTIMPL;
#else
    try
    {
        HRESULT hr;

        hr = D3DXCreateTextureFromFile(pDevice, _T("c:\\DX90SDK\\Samples\\Media\\shine1.bmp"), &m_pShine[0]);
        hr = D3DXCreateTextureFromFile(pDevice, _T("c:\\DX90SDK\\Samples\\Media\\shine2.bmp"), &m_pShine[1]);
        hr = D3DXCreateTextureFromFile(pDevice, _T("c:\\DX90SDK\\Samples\\Media\\shine3.bmp"), &m_pShine[2]);
        hr = D3DXCreateTextureFromFile(pDevice, _T("c:\\DX90SDK\\Samples\\Media\\shine4.bmp"), &m_pShine[3]);
        hr = D3DXCreateTextureFromFile(pDevice, _T("c:\\DX90SDK\\Samples\\Media\\shine5.bmp"), &m_pShine[4]);
        hr = D3DXCreateTextureFromFile(pDevice, _T("c:\\DX90SDK\\Samples\\Media\\shine6.bmp"), &m_pShine[5]);
		
	    hr = pDevice->CreateVertexBuffer( 
            4 * sizeof(ShineVertex), 
            D3DUSAGE_WRITEONLY, 
			ShineVertex::FVF, D3DPOOL_DEFAULT, 
            &m_pVertexBuffer, NULL);
    }
    catch (HRESULT /*hrFailed*/)
    {
        // Ignore initialization error temporarily
    }

    return S_OK;
#endif
}

HRESULT CShineDO::FreeDeviceResources()
{
    m_pVertexBuffer.Release();
    for (int i = 0; i < 6; i++)
        m_pShine[i].Release();

    return S_OK;
}

STDMETHODIMP CShineDO::Terminate()
{
    FreeDeviceResources();

    m_pOwner.Release();

    return S_OK;
}

STDMETHODIMP CShineDO::GetRenderEngineOwner(IDisplayRenderEngine** ppRenderEngine)
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

STDMETHODIMP CShineDO::ProcessMessage(HWND hWnd, UINT msg, UINT wParam, LONG lParam)
{
    CAutoLock Lock(&m_ObjectLock);

    switch (msg)
    {
    case WM_MOUSEMOVE:
		return OnMouseMove(LOWORD(lParam), HIWORD(lParam));
    }

    return E_FAIL;
}

HRESULT CShineDO::OnMouseMove(int x, int y)
{
	HRESULT hr;

	if (m_pVertexBuffer)
    {
		HWND hWndDisplay = NULL;
		hr = m_pOwner->GetDisplayWindow(&hWndDisplay, NULL);
		RECT rcDisplay;
		GetClientRect(hWndDisplay, &rcDisplay);

        ShineVertex* pVertices = NULL;
        hr = m_pVertexBuffer->Lock(0, sizeof(ShineVertex)*4, (void**)&pVertices, 0);
        MoveQuadVertices(pVertices, x, y, rcDisplay.right, rcDisplay.bottom);
        hr = m_pVertexBuffer->Unlock();
    }

	return E_FAIL;
}

STDMETHODIMP CShineDO::Render(IUnknown* pDevice, const NORMALIZEDRECT* lpParentRect)
{
    CAutoLock Lock(&m_ObjectLock);

    if (!m_pOwner)
    {
        return VFW_E_WRONG_STATE;
    }

    try
    {
        HRESULT hr;

        DWORD dwNow = GetTickCount();
        if (dwNow - m_dwLastTextureTime > 50)
        {
            m_iTexture = (m_iTexture + 1) % 6;
            m_dwLastTextureTime = dwNow;
        }
        if (m_pShine[m_iTexture])
        {
			CComQIPtr<IDirect3DDevice9> pDevice9 = pDevice;			
			hr = pDevice9->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);

			hr = pDevice9->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			hr = pDevice9->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
			hr = pDevice9->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
			hr = pDevice9->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

            hr = pDevice9->SetTexture( 0, m_pShine[m_iTexture]);
            hr = pDevice9->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	        hr = pDevice9->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
            hr = pDevice9->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCCOLOR);
            hr = pDevice9->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCCOLOR);

            hr = pDevice9->SetStreamSource(0, m_pVertexBuffer, 0, sizeof(ShineVertex));
            hr = pDevice9->SetFVF(ShineVertex::FVF);
	        hr = pDevice9->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);

			hr = pDevice9->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        }
    }
    catch (HRESULT hrFailed)
    {
        return hrFailed;
    }

    return S_OK;
}

STDMETHODIMP CShineDO::BeginDeviceLoss(void)
{
    CAutoLock Lock(&m_ObjectLock);
    return FreeDeviceResources();
}

STDMETHODIMP CShineDO::EndDeviceLoss(IUnknown* pDevice)
{
    CAutoLock Lock(&m_ObjectLock);
	CComQIPtr<IDirect3DDevice9> pDevice9 = pDevice;
    return AllocDeviceResources(pDevice9);
}

STDMETHODIMP CShineDO::GetCLSID(CLSID* pClsid)
{
    if (!pClsid)
    {
        return E_POINTER;
    }

    *pClsid = CLSID_ShineDisplayObject;
    return S_OK;
}
