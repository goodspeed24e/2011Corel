#ifndef _VIDEO_EFFECT3D_FOR3D_H_
#define _VIDEO_EFFECT3D_FOR3D_H_

#include "../../Exports/Inc/VideoEffect3D.h"

// {90403459-B8A3-4de2-A255-FA972B0982D5}
DEFINE_GUID(DispSvr_VideoEffectFor3D, 0x90403459, 0xb8a3, 0x4de2, 0xa2, 0x55, 0xfa, 0x97, 0x2b, 0x9, 0x82, 0xd5);

class CFor3DEffect : public IDispSvrVideoEffect3DPlugin
{
public:
	virtual ~CFor3DEffect();
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
	HRESULT _SetDevice(IUnknown *pDevice);
	HRESULT _ReleaseDevice();
	HRESULT _Initialize(IDispSvrVideoEffect3DManager *pManager);
	HRESULT _Enable(BOOL bEnable);

	CFor3DEffect();
	HRESULT DeinitFor3D();
	
private:
	IDirect3DDevice9 *m_pDevice;
	DWORD m_dwWidth, m_dwHeight;
	LONG m_cRef;
};

#endif	// _VIDEO_EFFECT3D_FOR3D_H_