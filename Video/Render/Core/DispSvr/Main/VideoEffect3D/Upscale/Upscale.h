#ifndef _VIDEO_EFFECT3D_UPSCALE_H_
#define _VIDEO_EFFECT3D_UPSCALE_H_

#include "../../Exports/Inc/VideoEffect3D.h"

// {EC477BCE-F519-42ce-9461-43B4CB5F4A21}
DEFINE_GUID(DispSvr_VideoEffectUpscale, 0xec477bce, 0xf519, 0x42ce, 0x94, 0x61, 0x43, 0xb4, 0xcb, 0x5f, 0x4a, 0x21);

enum UPSCALE_EFFECT_PARAM
{
	// Demo mode:
	// 0: disable
	//		[left screen]				[right screen]
	// 1: effect on left source			original left source
	// 2: effect on left source			original right source
	// 3: original left source			effect on right source
	// 4: original right source			effect on right source
	UPSCALE_EFFECT_PARAM_DEMO = 0,
	UPSCALE_EFFECT_PARAM_COMPLEXITY = 1,	// complexity level, see COMPLEXITY_LEVEL
	UPSCALE_EFFECT_PARAM_INTENSITY = 2,		// intensity parameter for biletaral filter [0 - 128]
	UPSCALE_EFFECT_PARAM_RANGE = 3			// range parameter for bileteral filter [0 - 64]
};

enum COMPLEXITY_LEVEL
{
	COMPLEXITY_MINIMUM = 0,
	COMPLEXITY_INTEL_CANTIGA = 3,
	COMPLEXITY_MAXIMUM = 10
};

class CUpscaleEffect : public IDispSvrVideoEffect3DPlugin
{
public:
	virtual ~CUpscaleEffect();
	static HRESULT Create(IDispSvrVideoEffect3DPlugin **pEffectPlugin);

	// IUnknown
	STDMETHOD(QueryInterface)(REFIID riid, void **ppv);
	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();

	// IDispSvrVideoEffect3DPlugin
	STDMETHOD(ProcessMessage)(DispSvr::VE3D_MESSAGE_TYPE eMessage, LPVOID ulParam);
	STDMETHOD(GetResourceId)(GUID *pGUID);
	STDMETHOD(SetParam)(DWORD dwParam, DWORD dwValue);
	STDMETHOD(GetParam)(DWORD dwParam, DWORD *pdwValue);
	STDMETHOD(ProcessEffect)(DispSvr::VE3DBuffer *pInput, DispSvr::VE3DBuffer *pOutput);

private:
	struct Texture
	{
		IDirect3DTexture9 *pTexture;
		IDirect3DSurface9 *pLevel0;

		Texture() : pTexture(0), pLevel0(0) { }
		~Texture() { Release(); }

		HRESULT SetTexture(IDirect3DTexture9 *p)
		{
			HRESULT hr;

			Release();
			hr = p->QueryInterface(__uuidof(IDirect3DTexture9), (void **)&pTexture);
			if (SUCCEEDED(hr))
			{
				hr = pTexture->GetSurfaceLevel(0, &pLevel0);
			}
			return hr;
		}

		HRESULT Create(IDirect3DDevice9 *pDevice, UINT uWidth, UINT uHeight, D3DFORMAT d3dfmt)
		{
			HRESULT hr;

			hr = pDevice->CreateTexture(uWidth, uHeight, 1, D3DUSAGE_RENDERTARGET, d3dfmt, D3DPOOL_DEFAULT, &pTexture, NULL);
			if (FAILED(hr))
				return hr;

			hr = pTexture->GetSurfaceLevel(0, &pLevel0);
			return hr;
		}

		void Release()
		{
			if (pTexture)
			{
				pTexture->Release();
				pTexture = 0;
			}
			
			if (pLevel0)
			{
				pLevel0->Release();
				pLevel0 = 0;
			}
		}
	};

private:
	HRESULT _SetDevice(IUnknown *pDevice);
	HRESULT _ReleaseDevice();
	HRESULT _Initialize(IDispSvrVideoEffect3DManager *pManager);
	HRESULT _Enable(BOOL bEnable);
	HRESULT AllocAuxiliaryResources(UINT uWidth, UINT uHeight, UINT uOutputWidth, UINT uOutputHeight);
	void ReleaseAuxiliaryResources();
	HRESULT Process(IDirect3DTexture9 *pInput, IDirect3DTexture9 **ppOutputTexture, UINT &uOutputWidth, UINT &uOutputHeight, bool bSkipDDT);
	HRESULT ApplyTechnique(D3DXHANDLE handle, Texture &output, int nQuadOffsetToVB = 0);

	CUpscaleEffect();

private:
	LONG m_cRef;
	IDirect3DDevice9 *m_pDevice;

	DWORD m_dwDemoMode;

	DWORD m_dwComplexity;
	DWORD m_dwIntensity;
	DWORD m_dwRange;

	// configurable complexity
	bool m_bSharpen;
	bool m_bBilateral;
	bool m_bSkipDDTOnInterlacedFrame;
	bool m_bSkipDDT;
	bool m_bReducedDDT;
	bool m_bRestrictUpscaleSize;

	bool m_bIndependentWriteMasks;
	DWORD m_dwWidth;
	DWORD m_dwHeight;
	DWORD m_dwOutputWidth;
	DWORD m_dwOutputHeight;
	IDirect3DVertexBuffer9 *m_pVertexBuffer;
	ID3DXEffect *m_pEffect;
	Texture m_DirectionTexture;
	Texture m_UnidirectionTexture;
	Texture m_UpscaledTexture;
	Texture m_SharpenTexture;
	Texture m_BilateralX;
	Texture m_BilateralY;
};

#endif	// _VIDEO_EFFECT3D_UPSCALE_H_