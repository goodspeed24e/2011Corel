#define _CRTDBG_MAP_ALLOC
#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <cassert>
#include "lib2d3d.h"
#include <InitGuid.h>
#include "For3D.h"

using namespace DispSvr;

static inline HRESULT GetInterface(LPUNKNOWN lpUnkwn, LPVOID *ppv)
{
	lpUnkwn->AddRef();
	*ppv = lpUnkwn;
	return S_OK;
}

HRESULT CFor3DEffect::Create(IDispSvrVideoEffect3DPlugin **pEffectPlugin)
{
	*pEffectPlugin = new CFor3DEffect;
	if (*pEffectPlugin)
		return S_OK;
	return E_OUTOFMEMORY;
}

CFor3DEffect::CFor3DEffect() : m_pDevice(0), m_cRef(1), m_dwWidth(0), m_dwHeight(0)
{
}

CFor3DEffect::~CFor3DEffect()
{
	assert(m_pDevice == 0);
}

//////////////////////////////////////////////////////////////////////////
// IUnknown
STDMETHODIMP CFor3DEffect::QueryInterface(REFIID riid, void **ppv)
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

STDMETHODIMP_(ULONG) CFor3DEffect::AddRef()
{
	LONG lRef = 0;
	lRef = InterlockedIncrement(&m_cRef);
	assert(lRef > 0);
	return lRef;
}

STDMETHODIMP_(ULONG) CFor3DEffect::Release()
{
	LONG lRef = InterlockedDecrement(&m_cRef);
	assert(lRef >= 0);
	if (lRef == 0)
		delete this;
	return lRef;
}

//////////////////////////////////////////////////////////////////////////
// IDispSvrVideoEffect3DPlugin
STDMETHODIMP CFor3DEffect::ProcessMessage(VE3D_MESSAGE_TYPE eMessage, LPVOID ulParam)
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

STDMETHODIMP CFor3DEffect::GetResourceId(GUID *pGUID)
{
	if (!pGUID)
		return E_POINTER;
	*pGUID = DispSvr_VideoEffectFor3D;
	return S_OK;
}

STDMETHODIMP CFor3DEffect::SetParam(DWORD dwParam, DWORD dwValue)
{
	return E_NOTIMPL;
}

STDMETHODIMP CFor3DEffect::GetParam(DWORD dwParam, DWORD *pdwValue)
{
	return E_NOTIMPL;
}

STDMETHODIMP CFor3DEffect::ProcessEffect(VE3DBuffer *pInput, VE3DBuffer *pOutput)
{
	if (!pInput || !pOutput || !pInput->pSurface)
		return E_POINTER;

	IDirect3DTexture9 *pInputTexture = 0;
	HRESULT hr = pInput->pSurface->QueryInterface(__uuidof(IDirect3DTexture9), (void **) &pInputTexture);

	ZeroMemory(pOutput, sizeof(VE3DBuffer));
	if (SUCCEEDED(hr))
	{
		// safe to release here since pInput->pSurface should keep at least another reference count.
		pInputTexture->Release();

		if (pInput->uWidth != m_dwWidth || pInput->uHeight != m_dwHeight)
		{
			DeinitFor3D();

			hr = lib2d3d_Init(m_pDevice, pInput->uWidth, pInput->uHeight);
			if (FAILED(hr))
				return hr;
			m_dwWidth = pInput->uWidth;
			m_dwHeight = pInput->uHeight;
		}

		pOutput->pSurface = lib2d3d_ModifyTexture(m_pDevice, pInputTexture);
		if (!pOutput->pSurface)
		{
			assert(0 && __FUNCTION__ ": lib2d3d_ModifyTexture() returns null pointer.");
			return E_FAIL;
		}

		pOutput->uWidth = m_dwWidth;
		pOutput->uHeight = m_dwHeight;
		pOutput->rcSource = pInput->rcSource;
		pOutput->rcTarget = pInput->rcTarget;
		if (0)
		{
			pOutput->pSurface->QueryInterface(__uuidof(IDirect3DTexture9), (void **)&pInputTexture);
			D3DXSaveTextureToFile(TEXT("C:\\do.bmp"), D3DXIFF_BMP, pInputTexture, NULL);
			pInputTexture->Release();
		}
	}
	return hr;
}

HRESULT CFor3DEffect::_Initialize(IDispSvrVideoEffect3DManager *pManager)
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

HRESULT CFor3DEffect::_SetDevice(IUnknown *pDevice)
{
	assert(m_pDevice == 0);
	HRESULT hr = pDevice->QueryInterface(__uuidof(IDirect3DDevice9), (void **)&m_pDevice);
	return hr;
}

HRESULT CFor3DEffect::_ReleaseDevice()
{
	DeinitFor3D();
	if (m_pDevice)
	{
		m_pDevice->Release();
		m_pDevice = 0;
	}
	return S_OK;
}

HRESULT CFor3DEffect::_Enable(BOOL bEnable)
{
	if (bEnable == FALSE)
	{
		return _ReleaseDevice();
	}
	return S_OK;
}

HRESULT CFor3DEffect::DeinitFor3D()
{
	HRESULT hr = S_OK;

	// For3D is not initialized.
	if (m_dwWidth == 0 || m_dwHeight == 0)
	{
		assert(m_dwWidth == m_dwHeight);
		return hr;
	}

	if (m_pDevice)
	{
		hr = lib2d3d_Deinit(m_pDevice);
	}
	m_dwWidth =	m_dwHeight = 0;
	return hr;
}

int __cdecl CreatePlugin(IDispSvrVideoEffect3DPlugin **ppPlugin)
{
	return CFor3DEffect::Create(ppPlugin);
}