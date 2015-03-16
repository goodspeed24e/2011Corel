#pragma once
#include "DispSvr.h"

#include <vector>

/// Basic implementation of IParentDisplayObject
class IParentDisplayObjectImpl :
	public CUnknown,
    public IParentDisplayObject,
	public IDisplayProperties
{
public:
	IParentDisplayObjectImpl(LPUNKNOWN pUnk, HRESULT *phr);
	virtual ~IParentDisplayObjectImpl();

	// IUnknown implementation
	DECLARE_IUNKNOWN
	STDMETHOD(NonDelegatingQueryInterface)(REFIID, void**);

	// IDisplayObject implementation
	STDMETHOD(Initialize)(IDisplayRenderEngine* pRenderEngine);
	STDMETHOD(GetRenderEngineOwner)(IDisplayRenderEngine** ppRenderEngine);
	STDMETHOD(Terminate)();
	STDMETHOD(ProcessMessage)(HWND hWnd, UINT msg, UINT wParam, LONG lParam);
	STDMETHOD(Render)(IUnknown *pDevice, const NORMALIZEDRECT* lpParentRect);
	STDMETHOD(BeginDeviceLoss)(void);
	STDMETHOD(EndDeviceLoss)( IUnknown* pDevice );
	STDMETHOD(GetCLSID)(CLSID* pClsid)	{ return E_NOTIMPL; }
	STDMETHOD(NotifyEvent)(DWORD event, DWORD param1, DWORD param2, LPVOID pInstance);

	// IParentDisplayObject implementation
	STDMETHOD(GetChildCount)(LONG* plCount);
	STDMETHOD(GetChild)(LONG lIndex, IDisplayObject** ppObject);
	STDMETHOD(IndexOf)(IUnknown* pObject, LONG* plIndex);
	STDMETHOD(AddChild)(LONG lZOrder, IUnknown* pObject);
	STDMETHOD(RemoveChild)(IUnknown* pObject);
	STDMETHOD(SetZOrder)(IUnknown* pObject, LONG lZOrder);
	STDMETHOD(GetZOrder)(IUnknown* pObject, LONG* plZOrder);
	STDMETHOD(Show)(BOOL bShow);

	// IDisplayProperties
	STDMETHOD(GetOutputRect)(NORMALIZEDRECT* lpNormRect);
	STDMETHOD(SetOutputRect)(NORMALIZEDRECT* lpNormRect);
	STDMETHOD(GetCropRect)(NORMALIZEDRECT* lpNormRect)						{ return E_NOTIMPL; }
	STDMETHOD(SetCropRect)(NORMALIZEDRECT* lpNormRect)						{ return E_NOTIMPL; }
	STDMETHOD(GetVertexBufferRect)(NORMALIZEDRECT* lpNormRect)				{ return E_NOTIMPL; }
	STDMETHOD(GetFrameColor)(COLORREF* pColor)								{ return E_NOTIMPL; }
	STDMETHOD(SetFrameColor)(COLORREF color)								{ return E_NOTIMPL; }
	STDMETHOD(CaptureFrame)(DWORD dwFormat, BYTE** ppFrame, UINT* pSize)	{ return E_NOTIMPL; }
	STDMETHOD(GetZoom)(NORMALIZEDRECT* lpNormRect)							{ return E_NOTIMPL; }
	STDMETHOD(SetZoom)(NORMALIZEDRECT* lpNormRect)							{ return E_NOTIMPL; }
	STDMETHOD(SetShow)(BOOL bShow);
	STDMETHOD(GetShow)(BOOL* bShow);
	STDMETHOD(ClearFrame)()													{ return E_NOTIMPL; }
	STDMETHOD(ClientToDO)(POINT* pPt);

public:

    struct DisplayObjectInfo
    {
		LONG lZOrder;
        IDisplayObject* pObj;
    };
	typedef std::vector<DisplayObjectInfo> DisplayObjects;

protected:
    DisplayObjects m_children;
    CComPtr<IDisplayRenderEngine> m_pOwner;
    CCritSec m_csObj; // this object has to be thread-safe
	BOOL m_bShow;
	NORMALIZEDRECT m_rectOutput;
};

/// Support multitexture rendering if a DO supports IDisplayOptimizedRender
class CCompositeDisplayObject :
	public IParentDisplayObjectImpl
{
public:
	CCompositeDisplayObject(LPUNKNOWN pUnk, HRESULT *phr);
	virtual ~CCompositeDisplayObject();

	STDMETHOD(Initialize)(IDisplayRenderEngine* pRenderEngine);
	STDMETHOD(ProcessMessage)(HWND hWnd, UINT msg, UINT wParam, LONG lParam);
	STDMETHOD(Render)(IUnknown *pDevice, const NORMALIZEDRECT* lpParentRect);
	STDMETHOD(GetCLSID)(CLSID* pClsid);

private:
	HRESULT SetMultiTextureCoord(LONG lStage, NORMALIZEDRECT nrUV);

	// custom vertex
	struct TextureCoord
	{
		FLOAT u, v;
	};

	struct MultiTex
	{
		D3DVECTOR position;
		D3DCOLOR color;
		TextureCoord tex[8];
	};	

	MultiTex m_vMultiTex[4];
	CComPtr<IDisplayObject> m_pMouseCapture;
	BOOL m_bOptimizedRenderSupport;
};

/// This is the root DO for all others.
class CRootDO :
	public IParentDisplayObjectImpl
{
public:
	CRootDO(LPUNKNOWN pUnk, HRESULT *phr);
	virtual ~CRootDO();
};

interface IDispSvrVideoMixer;

/// Specialized DO for HD/BD/DVD applications
class CVideoRootDO :
	public IParentDisplayObjectImpl
{
public:
	CVideoRootDO(LPUNKNOWN pUnk, HRESULT *phr);
	virtual ~CVideoRootDO();

	STDMETHOD(NonDelegatingQueryInterface)(REFIID, void**);

	STDMETHOD(Initialize)(IDisplayRenderEngine* pRenderEngine);
	STDMETHOD(Terminate)();
	STDMETHOD(Render)(IUnknown *pDevice, const NORMALIZEDRECT* lpParentRect);
	STDMETHOD(SetOutputRect)(NORMALIZEDRECT* lpNormRect);
	STDMETHOD(EndDeviceLoss)( IUnknown* pDevice );

protected:
	CComPtr<IDispSvrVideoMixer> m_pVideoMixer;
};