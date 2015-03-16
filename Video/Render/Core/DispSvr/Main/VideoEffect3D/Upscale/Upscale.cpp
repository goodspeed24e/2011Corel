#define _CRTDBG_MAP_ALLOC
#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <cassert>
#include <InitGuid.h>
#include "Upscale.h"
#include "interpolate.h"

#define MAX_UPSCALE_WIDTH	1024
#define MAX_UPSCALE_HEIGHT	768

#define DEFAULT_INTENSITY	12
#define DEFAULT_RANGE		12
#define DEFAULT_COMPLEXITY	COMPLEXITY_MAXIMUM

#ifdef _DEBUG
#define SAVE_TEXTURE(a)	D3DXSaveTextureToFileA("c:\\png\\" ## #a ## ".png", D3DXIFF_PNG, a, NULL);
#else
#define SAVE_TEXTURE(a) __noop
#endif

using namespace DispSvr;

int __cdecl CreatePlugin(IDispSvrVideoEffect3DPlugin **ppPlugin)
{
	return CUpscaleEffect::Create(ppPlugin);
}

static inline HRESULT GetInterface(LPUNKNOWN lpUnkwn, LPVOID *ppv)
{
	lpUnkwn->AddRef();
	*ppv = lpUnkwn;
	return S_OK;
}

HRESULT CUpscaleEffect::Create(IDispSvrVideoEffect3DPlugin **pEffectPlugin)
{
	*pEffectPlugin = new CUpscaleEffect;
	if (*pEffectPlugin)
		return S_OK;
	return E_OUTOFMEMORY;
}

CUpscaleEffect::CUpscaleEffect()
: m_pDevice(0), m_cRef(1),
m_dwDemoMode(0),
m_dwWidth(0), m_dwHeight(0), m_dwComplexity(DEFAULT_COMPLEXITY),
m_bSharpen(true), m_bBilateral(true), m_bIndependentWriteMasks(false),
m_bSkipDDTOnInterlacedFrame(false), m_bReducedDDT(false), m_bSkipDDT(false), m_bRestrictUpscaleSize(false),
m_dwOutputWidth(0), m_dwOutputHeight(0),
m_dwIntensity(DEFAULT_INTENSITY), m_dwRange(DEFAULT_RANGE),
m_pVertexBuffer(0),
m_pEffect(0)
{
}

CUpscaleEffect::~CUpscaleEffect()
{
	assert(m_pDevice == 0);
}

//////////////////////////////////////////////////////////////////////////
// IUnknown
STDMETHODIMP CUpscaleEffect::QueryInterface(REFIID riid, void **ppv)
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

STDMETHODIMP_(ULONG) CUpscaleEffect::AddRef()
{
	LONG lRef = 0;
	lRef = InterlockedIncrement(&m_cRef);
	assert(lRef > 0);
	return lRef;
}

STDMETHODIMP_(ULONG) CUpscaleEffect::Release()
{
	LONG lRef = InterlockedDecrement(&m_cRef);
	assert(lRef >= 0);
	if (lRef == 0)
		delete this;
	return lRef;
}

//////////////////////////////////////////////////////////////////////////
// IDispSvrVideoEffect3DPlugin
STDMETHODIMP CUpscaleEffect::ProcessMessage(VE3D_MESSAGE_TYPE eMessage, LPVOID ulParam)
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

STDMETHODIMP CUpscaleEffect::GetResourceId(GUID *pGUID)
{
	if (!pGUID)
		return E_POINTER;
	*pGUID = DispSvr_VideoEffectUpscale;
	return S_OK;
}

STDMETHODIMP CUpscaleEffect::SetParam(DWORD dwParam, DWORD dwValue)
{
	switch (dwParam)
	{
	case UPSCALE_EFFECT_PARAM_DEMO:
		m_dwDemoMode = dwValue;
		return S_OK;

	case UPSCALE_EFFECT_PARAM_COMPLEXITY:
		m_dwComplexity = min(dwValue, COMPLEXITY_MAXIMUM);
		if (m_dwComplexity <= COMPLEXITY_MINIMUM)
		{
			m_bSharpen = true;
			m_bBilateral = false;
			m_bReducedDDT = false;
			m_bRestrictUpscaleSize = true;
			m_bSkipDDTOnInterlacedFrame = false;
			m_bSkipDDT = true;
		}
		else if (m_dwComplexity <= COMPLEXITY_INTEL_CANTIGA)
		{
			m_bSharpen = true;
			m_bBilateral = false;
			m_bReducedDDT = true;
			m_bRestrictUpscaleSize = true;
			m_bSkipDDTOnInterlacedFrame = true;
			m_bSkipDDT = false;
		}
		else if (m_dwComplexity > COMPLEXITY_INTEL_CANTIGA)
		{
			// full effect
			m_bSharpen = true;
			m_bBilateral = true;
			m_bReducedDDT = false;
			m_bRestrictUpscaleSize = false;
			m_bSkipDDTOnInterlacedFrame = false;
			m_bSkipDDT = false;
		}
		return S_OK;

	case UPSCALE_EFFECT_PARAM_INTENSITY:
		m_dwIntensity = dwValue;
		return S_OK;

	case UPSCALE_EFFECT_PARAM_RANGE:
		m_dwRange = dwValue;
		return S_OK;
	}
	return E_NOTIMPL;
}

STDMETHODIMP CUpscaleEffect::GetParam(DWORD dwParam, DWORD *pdwValue)
{
	if (!pdwValue)
		return E_INVALIDARG;

	switch (dwParam)
	{
	case UPSCALE_EFFECT_PARAM_DEMO:
		*pdwValue = m_dwDemoMode;
		return S_OK;

	case UPSCALE_EFFECT_PARAM_COMPLEXITY:
		*pdwValue = m_dwComplexity;
		return S_OK;

	case UPSCALE_EFFECT_PARAM_INTENSITY:
		*pdwValue = m_dwIntensity;
		return S_OK;

	case UPSCALE_EFFECT_PARAM_RANGE:
		*pdwValue = m_dwRange;
		return S_OK;

	}
	return E_NOTIMPL;
}

STDMETHODIMP CUpscaleEffect::ProcessEffect(VE3DBuffer *pInput, VE3DBuffer *pOutput)
{
	if (!pInput || !pOutput || !pInput->pSurface)
		return E_POINTER;

	IDirect3DTexture9 *pInputTexture = 0;
	IDirect3DTexture9 *pOutputTexture = 0;
	HRESULT hr = pInput->pSurface->QueryInterface(__uuidof(IDirect3DTexture9), (void **) &pInputTexture);

	ZeroMemory(pOutput, sizeof(VE3DBuffer));
	if (SUCCEEDED(hr))
	{
		// safe to release here since pInput->pSurface should keep at least another reference count.
		pInputTexture->Release();

		UINT uOutputWidth = pInput->rcTarget.right - pInput->rcTarget.left;
		UINT uOutputHeight = pInput->rcTarget.bottom - pInput->rcTarget.top;

		if (m_bRestrictUpscaleSize)
		{
			uOutputWidth = min(uOutputWidth, MAX_UPSCALE_WIDTH);
			uOutputHeight = min(uOutputHeight, MAX_UPSCALE_HEIGHT);
		}

		if (pInput->uWidth != m_dwWidth || pInput->uHeight != m_dwHeight
			|| m_dwOutputWidth != uOutputWidth || m_dwOutputHeight != uOutputHeight)
		{
			ReleaseAuxiliaryResources();
			hr = AllocAuxiliaryResources(pInput->uWidth, pInput->uHeight, uOutputWidth, uOutputHeight);
			if (FAILED(hr))
			{
				ReleaseAuxiliaryResources();
				return hr;
			}
			m_dwWidth = pInput->uWidth;
			m_dwHeight = pInput->uHeight;
			m_dwOutputWidth = uOutputWidth;
			m_dwOutputHeight = uOutputHeight;
		}

		IDirect3DSurface9 *pRT = 0;
		DWORD dwFVF = 0;
		bool bSkipDDT = m_bSkipDDT || !m_bIndependentWriteMasks;

		if (!bSkipDDT && m_bSkipDDTOnInterlacedFrame)
		{
			LONGLONG llDuration = pInput->rtEnd - pInput->rtStart;
			if (pInput->bInterlaced || llDuration < 300000)
				bSkipDDT = true;
		}

		hr = m_pDevice->GetRenderTarget(0, &pRT);
		if (FAILED(hr))
			return hr;

		hr = m_pDevice->GetFVF(&dwFVF);

		// pOutputTexture is a weak reference.
		hr = Process(pInputTexture, &pOutputTexture, uOutputWidth, uOutputHeight, bSkipDDT);

		hr = m_pDevice->SetFVF(dwFVF);
		hr = m_pDevice->SetRenderTarget(0, pRT);

		if (pRT)
			pRT->Release();

		if (SUCCEEDED(hr))
		{
			pOutput->uWidth = uOutputWidth;
			pOutput->uHeight = uOutputHeight;
			pOutput->pSurface = pOutputTexture;

			SetRect(&pOutput->rcSource, 0, 0, pOutput->uWidth, pOutput->uHeight);
			CopyRect(&pOutput->rcTarget, &pInput->rcTarget);
		}
#ifdef _DEBUG
		if (0)
		{
			pOutput->pSurface->QueryInterface(__uuidof(IDirect3DTexture9), (void **)&pInputTexture);
			D3DXSaveTextureToFile(TEXT("C:\\do.bmp"), D3DXIFF_BMP, pInputTexture, NULL);
			pInputTexture->Release();
		}
#endif
	}
	return hr;
}

HRESULT CUpscaleEffect::_Initialize(IDispSvrVideoEffect3DManager *pManager)
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

HRESULT CUpscaleEffect::_SetDevice(IUnknown *pDevice)
{
	assert(m_pDevice == 0);
	HRESULT hr = pDevice->QueryInterface(__uuidof(IDirect3DDevice9), (void **)&m_pDevice);

	if (SUCCEEDED(hr))
	{
		D3DCAPS9 caps;
		hr = m_pDevice->GetDeviceCaps(&caps);
		if (SUCCEEDED(hr))
		{
			m_bIndependentWriteMasks = (caps.PrimitiveMiscCaps & D3DPMISCCAPS_INDEPENDENTWRITEMASKS) != 0;
		}

		ID3DXBuffer *pErrorBuffer = 0;
		hr = D3DXCreateEffect(m_pDevice, g_main, sizeof(g_main), NULL, NULL, 0, NULL, &m_pEffect, &pErrorBuffer);
#ifdef _DEBUG
		if (pErrorBuffer)
		{
			void *pPointer = pErrorBuffer->GetBufferPointer();
			DWORD dwSize = pErrorBuffer->GetBufferSize();
		}
#endif

		if (SUCCEEDED(hr))
		{
			SetParam(UPSCALE_EFFECT_PARAM_COMPLEXITY, m_dwComplexity);
		}
	}
	return hr;
}

HRESULT CUpscaleEffect::_ReleaseDevice()
{
	ReleaseAuxiliaryResources();
	if (m_pEffect)
	{
		m_pEffect->Release();
		m_pEffect = 0;
	}
	if (m_pDevice)
	{
		m_pDevice->Release();
		m_pDevice = 0;
	}
	return S_OK;
}

HRESULT CUpscaleEffect::_Enable(BOOL bEnable)
{
	if (bEnable == FALSE)
	{
		return _ReleaseDevice();
	}
	return S_OK;
}

struct float3
{
	float x, y, z;
};

struct float2
{
	float u, v;
};

struct VB
{
	float3 xyz;
	float2 uv[1];

	enum { FVF = D3DFVF_XYZ | D3DFVF_TEX1 };
};

HRESULT CUpscaleEffect::AllocAuxiliaryResources(UINT uWidth, UINT uHeight, UINT uOutputWidth, UINT uOutputHeight)
{
	if (uOutputWidth <= 0 || uOutputHeight <= 0)
		return E_INVALIDARG;

	HRESULT hr;
	void *pData = 0;
	const float fLeftRatio = 0.495f;
	const float fBarRatio = 0.01f;
	const float fRightRatio = fLeftRatio + fBarRatio;
	const float z = 0.5f;

	// 0 -- 1
	// |    |
	// 3 -- 2
	VB pVB[] = {
		// offset: 0
		{ -1, 1, z, 0, 0 },
		{ 1, 1, z, 1, 0 },
		{ 1, -1, z, 1, 1 },
		{ -1, -1, z, 0, 1 },

		// right source, right screen, offset: 1
		{ 2 * fRightRatio - 1, 1, z, 1 * fRightRatio, 0 },
		{ 1, 1, z, 1, 0 },
		{ 1, -1, z, 1, 1 },
		{ 2 * fRightRatio - 1, -1, z, 1 * fRightRatio, 1},

		// left source, right screen, offset: 2
		{ 2 * fRightRatio - 1, 1, z, 0, 0 },
		{ 1, 1, z, 1 * fLeftRatio, 0 },
		{ 1, -1, z, 1 * fLeftRatio, 1 },
		{ 2 * fRightRatio - 1, -1, z, 0, 1 },

		// left source, left screen, offset: 3
		{ -1, 1, z, 0, 0 },
		{ 2 * fLeftRatio - 1, 1, z, 1 * fLeftRatio, 0 },
		{ 2 * fLeftRatio - 1, -1, z, 1 * fLeftRatio, 1 },
		{ -1, -1, z, 0, 1 },

		// right source, left screen, offset: 4
		{ -1, 1, z, 1 * fRightRatio, 0 },
		{ 2 * fLeftRatio - 1, 1, z, 1, 0 },
		{ 2 * fLeftRatio - 1, -1, z, 1, 1 },
		{ -1, -1, z, 1 * fRightRatio, 1 },

		// middle bar, offset: 5
		{ 2 * fLeftRatio - 1, 1, z, 0, 0 },
		{ 2 * fRightRatio - 1, 1, z, 1 * fLeftRatio, 0 },
		{ 2 * fRightRatio - 1, -1, z, 1 * fLeftRatio, 1 },
		{ 2 * fLeftRatio - 1, -1, z, 0, 1 }
	};

	const unsigned int uSizeOfVB = sizeof(pVB);

	assert(4096 >= uSizeOfVB);
	hr = m_pDevice->CreateVertexBuffer(4096, D3DUSAGE_DYNAMIC, VB::FVF, D3DPOOL_DEFAULT, &m_pVertexBuffer, NULL);
	if (FAILED(hr))
		return hr;

	hr = m_pVertexBuffer->Lock(0, uSizeOfVB, &pData, D3DLOCK_DISCARD);
	if (FAILED(hr))
		return hr;

	memcpy(pData, pVB, sizeof(pVB));
	m_pVertexBuffer->Unlock();

	hr = m_BilateralX.Create(m_pDevice, uWidth, uHeight, D3DFMT_A8R8G8B8);
	if (FAILED(hr))
		return hr;

	hr = m_BilateralY.Create(m_pDevice, uWidth, uHeight, D3DFMT_A8R8G8B8);
	if (FAILED(hr))
		return hr;

	hr = m_SharpenTexture.Create(m_pDevice, uWidth, uHeight, D3DFMT_A8R8G8B8);
	if (FAILED(hr))
		return hr;

	if (m_bIndependentWriteMasks)
	{
		hr = m_DirectionTexture.Create(m_pDevice, uWidth, uHeight, D3DFMT_R5G6B5);
		if (FAILED(hr))
			hr = m_DirectionTexture.Create(m_pDevice, uWidth, uHeight, D3DFMT_A8R8G8B8);
		if (FAILED(hr))
			return hr;

		hr = m_UpscaledTexture.Create(m_pDevice, uOutputWidth, uOutputHeight, D3DFMT_A8R8G8B8);
		if (FAILED(hr))
			return hr;

		// texture may not be cleared on XP when created.
		m_pDevice->ColorFill(m_UpscaledTexture.pLevel0, NULL, 0);
	}

	D3DXMATRIX mxIdentity;
	D3DXMatrixIdentity(&mxIdentity);
	float fRange = 0.5f/(m_dwRange*m_dwRange);
	float fIntensity = 0.5f*(255*255)/(m_dwIntensity*m_dwIntensity);
	float pfTexScale[2] = { 1 / float(uWidth), 1 / float(uHeight) };
	float pfRangeWeight[4] = { -1 * fRange, -4 * fRange, -9 * fRange, -16 * fRange };

	hr = m_pEffect->SetFloatArray("g_tex_scale", pfTexScale, 2);
	hr = m_pEffect->SetFloatArray("g_range_weight", pfRangeWeight, 4);
	hr = m_pEffect->SetFloat("g_intensity", fIntensity);
	hr = m_pEffect->SetMatrix("g_mWorldViewProjection", &mxIdentity);
	return hr;
}

void CUpscaleEffect::ReleaseAuxiliaryResources()
{
	if (m_pVertexBuffer)
	{
		m_pVertexBuffer->Release();
		m_pVertexBuffer = 0;
	}

	m_DirectionTexture.Release();
	m_UnidirectionTexture.Release();
	m_UpscaledTexture.Release();
	m_SharpenTexture.Release();
	m_BilateralX.Release();
	m_BilateralY.Release();
}

HRESULT CUpscaleEffect::Process(IDirect3DTexture9 *pInput, IDirect3DTexture9 **ppOutputTexture, UINT &uOutputWidth, UINT &uOutputHeight, bool bSkipDDT)
{
	HRESULT hr = E_FAIL;
	IDirect3DTexture9 *pReferenceVideo = pInput;
	UINT cPass = 0;

	uOutputWidth = m_dwWidth;
	uOutputHeight = m_dwHeight;

	hr = m_pDevice->SetFVF(VB::FVF);
	hr = m_pDevice->SetStreamSource(0, m_pVertexBuffer, 0, sizeof(VB));

	if (m_bBilateral)
	{
		hr = m_pDevice->SetRenderTarget(0, m_BilateralX.pLevel0);	assert(SUCCEEDED(hr));
		hr = m_pEffect->SetTechnique("bilateral");	assert(SUCCEEDED(hr));
		hr = m_pEffect->Begin(&cPass, 0);	assert(SUCCEEDED(hr));
		if (SUCCEEDED(hr))
		{
			hr = m_pEffect->SetTexture("tex_rgb", pReferenceVideo);	assert(SUCCEEDED(hr));
			hr = m_pEffect->BeginPass(0); 	assert(SUCCEEDED(hr));
			hr = m_pDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);
			m_pEffect->EndPass();

			hr = m_pDevice->SetRenderTarget(0, m_BilateralY.pLevel0);	assert(SUCCEEDED(hr));
			hr = m_pEffect->SetTexture("tex_rgb", m_BilateralX.pTexture);	assert(SUCCEEDED(hr));
			hr = m_pEffect->BeginPass(1); 	assert(SUCCEEDED(hr));
			hr = m_pDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);
			m_pEffect->EndPass();
			hr = m_pEffect->End();
			pReferenceVideo = m_BilateralY.pTexture;
		}
	}

	if (m_bSharpen)
	{
		hr = m_pEffect->SetTexture("tex_rgb", pReferenceVideo);	assert(SUCCEEDED(hr));
		hr = ApplyTechnique("sharpen", m_SharpenTexture);
		pReferenceVideo = m_SharpenTexture.pTexture;
	}

	*ppOutputTexture = pReferenceVideo;

	if (!bSkipDDT)
	{
		m_UnidirectionTexture.SetTexture(pReferenceVideo);

		hr = m_pEffect->SetTechnique("upscaling");	assert(SUCCEEDED(hr));
		hr = m_pEffect->SetTexture("tex_rgb", pReferenceVideo);	assert(SUCCEEDED(hr));
		hr = m_pEffect->SetTexture("tex_dir", m_DirectionTexture.pTexture);	assert(SUCCEEDED(hr));
		hr = m_pEffect->Begin(&cPass, 0);	assert(SUCCEEDED(hr));
		if (SUCCEEDED(hr))
		{
			hr = m_pDevice->SetRenderTarget(0, m_DirectionTexture.pLevel0);	assert(SUCCEEDED(hr));
			hr = m_pEffect->BeginPass(0);	assert(SUCCEEDED(hr));
			hr = m_pDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);
			m_pEffect->EndPass();

			// reduced DDT omits unidirection pass.
			if (m_bReducedDDT)
			{
				hr = m_pDevice->SetRenderTarget(0, m_UpscaledTexture.pLevel0);	assert(SUCCEEDED(hr));
				hr = m_pEffect->BeginPass(3);	assert(SUCCEEDED(hr));
				hr = m_pDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);
				m_pEffect->EndPass();
			}
			else
			{
				hr = m_pDevice->SetRenderTarget(0, m_UnidirectionTexture.pLevel0);	assert(SUCCEEDED(hr));
				hr = m_pEffect->BeginPass(1);	assert(SUCCEEDED(hr));
				hr = m_pDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);
				m_pEffect->EndPass();

				hr = m_pDevice->SetRenderTarget(0, m_UpscaledTexture.pLevel0);	assert(SUCCEEDED(hr));
				hr = m_pEffect->BeginPass(2);	assert(SUCCEEDED(hr));
				hr = m_pDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);
				m_pEffect->EndPass();
			}

			hr = m_pEffect->End();
		}

		*ppOutputTexture = m_UpscaledTexture.pTexture;
		uOutputWidth = m_dwOutputWidth;
		uOutputHeight = m_dwOutputHeight;
	}

	if (m_dwDemoMode > 0)
	{
		IDirect3DSurface9 *pDemoRT = 0;
		hr = (*ppOutputTexture)->GetSurfaceLevel(0, &pDemoRT);
		if (pDemoRT)
		{
			hr = m_pDevice->SetRenderTarget(0, pDemoRT);
			hr = m_pEffect->SetTechnique("demo");	assert(SUCCEEDED(hr));

			hr = m_pEffect->SetTexture("tex_dir", pInput);	assert(SUCCEEDED(hr));
			hr = m_pEffect->Begin(&cPass, 0);	assert(SUCCEEDED(hr));
			if (SUCCEEDED(hr))
			{
				hr = m_pEffect->BeginPass(0);		assert(SUCCEEDED(hr));
				hr = m_pDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, 4 * m_dwDemoMode, 2);
				m_pEffect->EndPass();

				// middle bar
				hr = m_pEffect->BeginPass(1);		assert(SUCCEEDED(hr));
				hr = m_pDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, 4 * 5, 2);
				m_pEffect->EndPass();
				hr = m_pEffect->End();
			}
			pDemoRT->Release();
		}
	}

#ifdef _DEBUG
	if (0)
	{
		SAVE_TEXTURE(pInput);
		SAVE_TEXTURE(pReferenceVideo);
		SAVE_TEXTURE(m_BilateralX.pTexture);
		SAVE_TEXTURE(m_BilateralY.pTexture);
		SAVE_TEXTURE(m_SharpenTexture.pTexture);
		SAVE_TEXTURE(m_DirectionTexture.pTexture);
		SAVE_TEXTURE(m_UnidirectionTexture.pTexture);
		SAVE_TEXTURE(m_UpscaledTexture.pTexture);

		hr = m_pDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
		hr = m_pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		hr = m_pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

		hr = m_pDevice->SetRenderTarget(0, m_UpscaledTexture.pLevel0);
		hr = m_pDevice->SetTexture(0, pInput);
		hr = m_pDevice->SetPixelShader(NULL);
		hr = m_pDevice->SetStreamSource(0, m_pVertexBuffer, 0, sizeof(VB));
		hr = m_pDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);
		D3DXSaveTextureToFile(TEXT("C:\\bilinear.png"), D3DXIFF_PNG, m_UpscaledTexture.pTexture, NULL);
	}
#endif
	return hr;
}

HRESULT CUpscaleEffect::ApplyTechnique(D3DXHANDLE hTechnique, Texture &output, int nQuadOffsetToVB)
{
	HRESULT hr;
	UINT cPass, iPass;

	hr = m_pDevice->SetRenderTarget(0, output.pLevel0);	assert(SUCCEEDED(hr));
	hr = m_pEffect->SetTechnique(hTechnique);	assert(SUCCEEDED(hr));
	hr = m_pEffect->Begin(&cPass, 0);	assert(SUCCEEDED(hr));
	if (SUCCEEDED(hr))
	{
		for (iPass = 0; iPass < cPass; iPass++)
		{
			hr = m_pEffect->BeginPass(iPass);		assert(SUCCEEDED(hr));
			hr = m_pDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, 4 * nQuadOffsetToVB, 2);
			m_pEffect->EndPass();
		}
		hr = m_pEffect->End();
	}
	return hr;
}
