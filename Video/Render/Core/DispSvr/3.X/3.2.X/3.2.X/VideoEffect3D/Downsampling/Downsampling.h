#pragma once

#include "../../Exports/Inc/VideoEffect3D.h"

class CDownsamplingEffect : public IDispSvrVideoEffect3DPlugin
{
private:
    CDownsamplingEffect(); // only can be created from calling Create method.
public:
    virtual ~CDownsamplingEffect();
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

protected:
	HRESULT _SetDevice(IUnknown *pDevice);
	HRESULT _ReleaseDevice();
	HRESULT _Initialize(IDispSvrVideoEffect3DManager *pManager);
	HRESULT _Enable(BOOL bEnable);
	HRESULT _PrepareDownsamplingTexture(UINT uVideoWidth, UINT uVideoHeight);
	HRESULT _CleanUpDownsamplingTexture();
protected:
    volatile LONG m_cRef;
    IDirect3DDevice9 *m_pDevice;
	IDirect3DTexture9 *m_pDownsamplingTexture;
    BOOL m_bNonPow2TextureSupported;
	volatile BOOL m_bEnabled;
	UINT m_uTextureWidth;
	UINT m_uTextureHeight;
	UINT m_uVideoWidth;
	UINT m_uVideoHeight;
    UINT m_uPixelThreshold;
};
