#include "stdafx.h"
#include <functional>
#include <algorithm>
#include "CompositeDO.h"

using namespace DispSvr;

static inline void MapToParentRect(NORMALIZEDRECT* lpOutput, const NORMALIZEDRECT* lpParent, const NORMALIZEDRECT* lpThis)
{
	float w_2 = (lpParent->right - lpParent->left) * 0.5f;
	float h_2 = (lpParent->top - lpParent->bottom) * 0.5f;

	lpOutput->left = lpParent->left + (lpThis->left + 1.0f) * w_2;
	lpOutput->right = lpParent->right - (1.0f - lpThis->right) * w_2;
	lpOutput->top = lpParent->top - (1.0f - lpThis->top) * h_2;
	lpOutput->bottom = lpParent->bottom + (lpThis->bottom + 1.0f) * h_2;
}

IParentDisplayObjectImpl::IParentDisplayObjectImpl(LPUNKNOWN pUnk, HRESULT *phr)
	: CUnknown(NAME("ParentDO"), pUnk)
{
	m_bShow = TRUE;
	m_rectOutput.left = -1.0f;
	m_rectOutput.top = 1.0f;
	m_rectOutput.right = 1.0f;
	m_rectOutput.bottom = -1.0f;
}

IParentDisplayObjectImpl::~IParentDisplayObjectImpl()
{
    Terminate();
}

STDMETHODIMP IParentDisplayObjectImpl::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
	HRESULT hr = E_NOINTERFACE;
	*ppv = NULL;

	if (riid == IID_IDisplayObject)
	{
		hr = GetInterface((IDisplayObject *)this, ppv);
	}
	else if (riid == IID_IParentDisplayObject)
	{
		hr = GetInterface((IParentDisplayObject*) this, ppv);
	}
	else if (riid == IID_IDisplayProperties)
	{
		hr = GetInterface((IDisplayProperties*) this, ppv);
	}
	else
	{
		hr = CUnknown::NonDelegatingQueryInterface(riid, ppv);
	}
	return hr;
}

STDMETHODIMP IParentDisplayObjectImpl::Initialize(IDisplayRenderEngine* pRenderEngine)
{
	CHECK_POINTER(pRenderEngine);

    HRESULT hr = S_OK;

    DisplayObjects::iterator it = m_children.begin();
    for (; it != m_children.end(); ++it)
    {
        hr = it->pObj->Initialize(pRenderEngine);
        if (FAILED(hr))
        {
            DbgMsg("IParentDisplayObjectImpl::Initialize: failed in IDisplayObject::Initialize, hr = 0x%08x", hr);
            return hr;
        }
    }

    m_pOwner = pRenderEngine;
	m_bShow = TRUE;

    return hr;
}

STDMETHODIMP IParentDisplayObjectImpl::Terminate()
{
    DisplayObjects::iterator it = m_children.begin();
    for (; it != m_children.end(); ++it)
    {
        it->pObj->Terminate();
        it->pObj->Release();
    }
    m_children.clear();

    m_pOwner.Release();
    
    return S_OK;
}

STDMETHODIMP IParentDisplayObjectImpl::ProcessMessage(HWND hWnd, UINT msg, UINT wParam, LONG lParam)
{
    return S_OK;
}

STDMETHODIMP IParentDisplayObjectImpl::Render(IUnknown* pDevice, const NORMALIZEDRECT* lpParentRect)
{
    CAutoLock selfLock(&m_csObj);

    if (!m_pOwner)
        return VFW_E_WRONG_STATE;

	if (!m_bShow)
		return S_OK;

    HRESULT hr;
	NORMALIZEDRECT rectOutput;
	MapToParentRect(&rectOutput, lpParentRect, &m_rectOutput);

    DisplayObjects::reverse_iterator it = m_children.rbegin();
    for (; it != m_children.rend(); ++it)
    {
		hr = it->pObj->Render(pDevice, &rectOutput);
		if (FAILED(hr))
		{
			DbgMsg("IParentDisplayObjectImpl::Render: failed in IDisplayObject::Render, error code 0x%08x", hr);
		}
    }
    return S_OK;
}

STDMETHODIMP IParentDisplayObjectImpl::GetRenderEngineOwner(IDisplayRenderEngine** ppRenderEngine)
{
	CHECK_POINTER(ppRenderEngine);

    if (!m_pOwner)
    {
        return VFW_E_NOT_FOUND;
    }

    *ppRenderEngine = m_pOwner;
    (*ppRenderEngine)->AddRef();

    return S_OK;
}

STDMETHODIMP IParentDisplayObjectImpl::BeginDeviceLoss(void)
{
    CAutoLock selfLock(&m_csObj);

    HRESULT hr = S_OK;

    DisplayObjects::iterator it = m_children.begin();
    for (; it != m_children.end(); ++it)
    {
        hr = it->pObj->BeginDeviceLoss();
    }

    return hr;
}

STDMETHODIMP IParentDisplayObjectImpl::EndDeviceLoss(IUnknown* pDevice)
{
    CAutoLock selfLock(&m_csObj);

    HRESULT hr = S_OK;

    DisplayObjects::iterator it = m_children.begin();
    for (; it != m_children.end(); ++it)
    {
        hr = it->pObj->EndDeviceLoss(pDevice);
    }

    return hr;
}

STDMETHODIMP IParentDisplayObjectImpl::NotifyEvent(DWORD event, DWORD param1, DWORD param2, LPVOID pInstance)
{
	CAutoLock selfLock(&m_csObj);

	HRESULT hr = S_OK;

	for (DisplayObjects::iterator it = m_children.begin(); it != m_children.end(); ++it)
		hr |= it->pObj->NotifyEvent(event, param1, param2, pInstance);
	return hr;
}

STDMETHODIMP IParentDisplayObjectImpl::GetChildCount(LONG* plCount)
{
	CHECK_POINTER(plCount);

    CAutoLock selfLock(&m_csObj);
    *plCount = m_children.size();
    return S_OK;
}

STDMETHODIMP IParentDisplayObjectImpl::IndexOf(IUnknown* pObject, LONG* plIndex)
{
	CHECK_POINTER(plIndex);

	CAutoLock selfLock(&m_csObj);
	CComQIPtr<IDisplayObject> pDO = pObject;

    DisplayObjects::iterator it = m_children.begin();
    for (; it != m_children.end(); ++it)
    {
        if (it->pObj == pDO)
        {
            *plIndex = it - m_children.begin();
            return S_OK;
        }
    }

    return E_INVALIDARG;

}

STDMETHODIMP IParentDisplayObjectImpl::GetChild(LONG lIndex, IDisplayObject** ppChild)
{
	CHECK_POINTER(ppChild);

    CAutoLock selfLock(&m_csObj);

    if (lIndex < 0 || (size_t) lIndex >= m_children.size())
    {
        return E_INVALIDARG;
    }

    *ppChild = m_children[lIndex].pObj;
    (*ppChild)->AddRef();

    return S_OK;
}

STDMETHODIMP IParentDisplayObjectImpl::AddChild(LONG lZOrder, IUnknown* pObject)
{
    CAutoLock selfLock(&m_csObj);

	CComQIPtr<IDisplayObject> pDO = pObject;
    if (!pDO || lZOrder < 0)
        return E_INVALIDARG;

    HRESULT hr = S_OK;
    if (m_pOwner)
    {
        // Initialize it if we have an owner already.
        hr = pDO->Initialize(m_pOwner);
    }

    if (SUCCEEDED(hr))
    {
        DisplayObjectInfo info;
        info.pObj = pDO.Detach();
		info.lZOrder = lZOrder;

		DisplayObjects::iterator it = m_children.begin();
		for (; it != m_children.end(); ++it)
		{
			if (lZOrder <= it->lZOrder)	{
				break;
			}
		}

		m_children.insert(it, info);
    }

    return hr;
}

STDMETHODIMP IParentDisplayObjectImpl::RemoveChild(IUnknown* pObject)
{
    CAutoLock selfLock(&m_csObj);

	CComQIPtr<IDisplayObject> pDO = pObject;

    DisplayObjects::iterator it = m_children.begin();
    for (; it != m_children.end(); ++it)
    {
        if (it->pObj == pDO)
        {
			if (m_pOwner)
			{
				// Terminate it when it is removed from its parent.
				it->pObj->Terminate();
			}

            it->pObj->Release();
            m_children.erase(it);
            return S_OK;
        }
    }

    return E_INVALIDARG;
}

template <bool bAscending = true>
struct CompareByZOrder : std::binary_function<IParentDisplayObjectImpl::DisplayObjectInfo*, IParentDisplayObjectImpl::DisplayObjectInfo*, bool> 
{
    bool operator()(const IParentDisplayObjectImpl::DisplayObjectInfo& left, const IParentDisplayObjectImpl::DisplayObjectInfo& right) const
	{
        return bAscending ? (left.lZOrder < right.lZOrder) : (right.lZOrder < left.lZOrder); 
    }
};

STDMETHODIMP IParentDisplayObjectImpl::SetZOrder(IUnknown* pObject, LONG lZOrder)
{
    CAutoLock selfLock(&m_csObj);

	CComQIPtr<IDisplayObject> pDO = pObject;

    for (DisplayObjects::iterator it = m_children.begin(); it != m_children.end(); ++it)
    {
        if (it->pObj == pDO)
        {
			it->lZOrder = lZOrder;
			stable_sort(m_children.begin(), m_children.end(), CompareByZOrder<true>());
			return S_OK;
        }
    }

    return E_INVALIDARG;
}

STDMETHODIMP IParentDisplayObjectImpl::GetZOrder(IUnknown* pObject, LONG* plZOrder)
{
	CHECK_POINTER(plZOrder);

	CAutoLock selfLock(&m_csObj);
	CComQIPtr<IDisplayObject> pDO = pObject;

    for (DisplayObjects::iterator it = m_children.begin(); it != m_children.end(); ++it)
    {
        if (it->pObj == pDO)
        {
			*plZOrder = it->lZOrder;
			return S_OK;
        }
    }

    return E_INVALIDARG;
}

STDMETHODIMP IParentDisplayObjectImpl::Show(BOOL bShow)
{
    CAutoLock selfLock(&m_csObj);

	m_bShow = bShow;
	
	return S_OK;
}

STDMETHODIMP IParentDisplayObjectImpl::GetOutputRect(NORMALIZEDRECT* lpNormRect)
{
	CHECK_POINTER(lpNormRect);
	memcpy(lpNormRect, &m_rectOutput, sizeof(NORMALIZEDRECT));
	return S_OK;
}

STDMETHODIMP IParentDisplayObjectImpl::SetOutputRect(NORMALIZEDRECT* lpNormRect)
{
	CHECK_POINTER(lpNormRect);

	CAutoLock selfLock(&m_csObj);
	memcpy(&m_rectOutput, lpNormRect, sizeof(NORMALIZEDRECT));
	return S_OK;
}


STDMETHODIMP IParentDisplayObjectImpl::SetShow(BOOL bShow)
{
	return Show(bShow);
}

STDMETHODIMP IParentDisplayObjectImpl::GetShow(BOOL* bShow)
{
	CHECK_POINTER(bShow);

	CAutoLock selfLock(&m_csObj);
	*bShow = m_bShow;
	return S_OK;
}

STDMETHODIMP IParentDisplayObjectImpl::ClientToDO(POINT* pPt)
{
	DisplayObjects::iterator it = m_children.begin();
	for (int i=0; it != m_children.end(); ++it, i++)
	{
		CComQIPtr<IDisplayProperties> pProp = it->pObj;
		if (pProp && SUCCEEDED(pProp->ClientToDO(pPt)))
			return S_OK;
	}
	return E_FAIL;
}

//////////////////////////////////////////////////////////////////////////
/// CCompositeDisplayObject
//////////////////////////////////////////////////////////////////////////

CCompositeDisplayObject::CCompositeDisplayObject(LPUNKNOWN pUnk, HRESULT *phr)
	: IParentDisplayObjectImpl(pUnk, phr)
{
	ZeroMemory(m_vMultiTex, sizeof(m_vMultiTex));
	m_bOptimizedRenderSupport = false;
}

CCompositeDisplayObject::~CCompositeDisplayObject()
{
	Terminate();
}

HRESULT CCompositeDisplayObject::SetMultiTextureCoord(LONG lStage, NORMALIZEDRECT nrUV)
{
	if (lStage < 0 || lStage > 7)
		return E_INVALIDARG;

	m_vMultiTex[0].tex[lStage].u = nrUV.left;
	m_vMultiTex[0].tex[lStage].v = nrUV.top;
	m_vMultiTex[1].tex[lStage].u = nrUV.right;
	m_vMultiTex[1].tex[lStage].v = nrUV.top;
	m_vMultiTex[2].tex[lStage].u = nrUV.left;
	m_vMultiTex[2].tex[lStage].v = nrUV.bottom;
	m_vMultiTex[3].tex[lStage].u = nrUV.right;
	m_vMultiTex[3].tex[lStage].v = nrUV.bottom;

	m_vMultiTex[0].position.x = -1;
	m_vMultiTex[0].position.y = 1;
	m_vMultiTex[0].position.z = 0.5f;
	m_vMultiTex[0].color = D3DCOLOR_RGBA(0xFF, 0xFF, 0xFF, 0);

	m_vMultiTex[1].position.x = 1;
	m_vMultiTex[1].position.y = 1;
	m_vMultiTex[1].position.z = 0.5f;
	m_vMultiTex[1].color = D3DCOLOR_RGBA(0xFF, 0xFF, 0xFF, 0);

	m_vMultiTex[2].position.x = -1;
	m_vMultiTex[2].position.y = -1;
	m_vMultiTex[2].position.z = 0.5f;
	m_vMultiTex[2].color = D3DCOLOR_RGBA(0xFF, 0xFF, 0xFF, 0);

	m_vMultiTex[3].position.x = 1;
	m_vMultiTex[3].position.y = -1;
	m_vMultiTex[3].position.z = 0.5f;
	m_vMultiTex[3].color = D3DCOLOR_RGBA(0xFF, 0xFF, 0xFF, 0);

	return S_OK;
}

STDMETHODIMP CCompositeDisplayObject::Initialize(IDisplayRenderEngine* pRenderEngine)
{
	HRESULT hr = IParentDisplayObjectImpl::Initialize(pRenderEngine);

	// m_bOptimizedRenderSupport is currently disabled
/*
	CComPtr<IUnknown> pDevice;
	D3DCAPS9 caps;		
	ZeroMemory(&caps, sizeof(caps));
	hr = m_pOwner->Get3DDevice(&pDevice);
	if (FAILED(hr))
	{
		DbgMsg("IParentDisplayObjectImpl::Initialize: failed in IRenderEngine::Get3DDevice, hr = 0x%08x", hr);
		return hr;
	}
	CComQIPtr<IDirect3DDevice9> pDevice9 = pDevice;
	if (pDevice9)
	{
		hr = pDevice9->GetDeviceCaps(&caps);
		if (FAILED(hr))
		{
			DbgMsg("IParentDisplayObjectImpl::Initialize: failed in IDirect3DDevice9::GetDeviceCaps, hr = 0x%08x", hr);
			return hr;
		}
	}
	m_bOptimizedRenderSupport = (caps.TextureAddressCaps & D3DPTADDRESSCAPS_BORDER) != 0 ? TRUE : FALSE;
*/
	return hr;
}


STDMETHODIMP CCompositeDisplayObject::ProcessMessage(HWND hWnd, UINT msg, UINT wParam, LONG lParam)
{
	HRESULT hr = E_FAIL;

	if (msg >= WM_MOUSEFIRST && msg <= WM_MOUSELAST)
	{
		if (m_pMouseCapture) // Mouse message is captured by some DO.
		{
			if (m_pMouseCapture->ProcessMessage(hWnd, msg, wParam, lParam) == S_FALSE)
			{
				m_pMouseCapture.Release();
			}
			return S_OK;
		}
		else
		{
			DisplayObjects::iterator it = m_children.begin();
			for (; it != m_children.end(); ++it)
			{
				hr = it->pObj->ProcessMessage(hWnd, msg, wParam, lParam);
				if (hr == S_FALSE)
				{
					// The DO want to capture the mouse message.
					m_pMouseCapture = it->pObj;
					break;
				}
				else if (SUCCEEDED(hr))
				{
					return hr;
				}
			}
		}

		return E_FAIL;
	}

	// Other messages...
	DisplayObjects::iterator it = m_children.begin();
	for (; it != m_children.end(); ++it)
	{
		hr = it->pObj->ProcessMessage(hWnd, msg, wParam, lParam);
		if (SUCCEEDED(hr))
		{
			break;
		}
	}

	return hr;
}

STDMETHODIMP CCompositeDisplayObject::Render(IUnknown* pDevice, const NORMALIZEDRECT* lpParentRect)
{
	CAutoLock selfLock(&m_csObj);

	if (!m_pOwner)
	{
		return VFW_E_WRONG_STATE;
	}

	if (!m_bShow)
		return S_OK;

	HRESULT hr;

	LONG lMultiTexStage = 0; // the current number of texture stages

	NORMALIZEDRECT rectOutput;
	MapToParentRect(&rectOutput, lpParentRect, &m_rectOutput);

	DisplayObjects::reverse_iterator it = m_children.rbegin();
	for (; it != m_children.rend(); ++it)
	{
		LONG lTextureCount=0;
		CComQIPtr<IDisplayOptimizedRender> pOptRender = it->pObj;

		if (pOptRender == NULL || 
			!m_bOptimizedRenderSupport || 
			FAILED(pOptRender->GetTextureCount(&lTextureCount)) || 
			(lTextureCount >= 8 || lTextureCount <= 0))
		{
			// Use old method
			hr = it->pObj->Render(pDevice, &rectOutput);
			if (FAILED(hr))
			{
				DbgMsg("IParentDisplayObjectImpl::Render: failed in IDisplayObject::Render, error code 0x%08x", hr);
			}
		}
		else
		{
			int i;
			// get coord of each texture
			for (i = 0; i < lTextureCount; i++)
			{
				NORMALIZEDRECT nrTex;
				pOptRender->GetTextureCoord(i, &nrTex);

				// set coord into parent's vertex
				SetMultiTextureCoord(lMultiTexStage, nrTex);

				// call new render for each texture
				pOptRender->MultiTextureRender(pDevice, lMultiTexStage, i, &rectOutput);

				lMultiTexStage++;
			}

			CComQIPtr<IDirect3DDevice9> pDevice9 = pDevice;

			LONG lNextCount;
			CComPtr<IDisplayOptimizedRender> pNextRender;

			if (lMultiTexStage == 7 || // if we already collected 7 textures
				(it+1) == m_children.rend() || // if current item is last child
				FAILED((it+1)->pObj->QueryInterface(IID_IDisplayOptimizedRender, (void**)&pNextRender)) || // if next child doesn't support MTR
				FAILED(pNextRender->GetTextureCount(&lNextCount)) ||
				lNextCount + lMultiTexStage >= 8) // if next child contains too many textures
			{
				// Set alphablending
				hr = pDevice9->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
				hr = pDevice9->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
				hr = pDevice9->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
				hr = pDevice9->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
				hr = pDevice9->SetRenderState(D3DRS_ALPHAREF, 0x0);
				hr = pDevice9->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);

				// draw				
				DWORD dwFVF = D3DFVF_XYZ | D3DFVF_DIFFUSE;
				dwFVF |= (lMultiTexStage << D3DFVF_TEXCOUNT_SHIFT);

				hr = pDevice9->SetFVF(dwFVF);
				hr = pDevice9->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, (LPVOID)(m_vMultiTex), sizeof(m_vMultiTex[0]));

				// clear texture
				for (i = 0; i < lMultiTexStage; i++)
				{
					hr = pDevice9->SetTexture(i, NULL);
				}

				// remove alphablending
				hr = pDevice9->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
				hr = pDevice9->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

				lMultiTexStage = 0;
				ZeroMemory(m_vMultiTex, sizeof(m_vMultiTex));
			}
		}
	}
	return S_OK;
}

STDMETHODIMP CCompositeDisplayObject::GetCLSID(CLSID* pClsid)
{
	CHECK_POINTER(pClsid);
	*pClsid = CLSID_CompositeDisplayObject;
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
/// CRootDO
//////////////////////////////////////////////////////////////////////////

CRootDO::CRootDO(LPUNKNOWN pUnk, HRESULT *phr)
: IParentDisplayObjectImpl(pUnk, phr)
{

}

CRootDO::~CRootDO()
{

}

//////////////////////////////////////////////////////////////////////////
/// CVideoRootDO
//////////////////////////////////////////////////////////////////////////


CVideoRootDO::CVideoRootDO(LPUNKNOWN pUnk, HRESULT *phr)
: IParentDisplayObjectImpl(pUnk, phr)
{
}

CVideoRootDO::~CVideoRootDO()
{
}

STDMETHODIMP CVideoRootDO::NonDelegatingQueryInterface(REFIID iid, void **ppv)
{
	if (iid == __uuidof(IDispSvrVideoMixer))
	{
		return CResourceManager::GetInstance()->GetInterface(DISPSVR_RESOURCE_VIDEOMIXER, iid, ppv);
	}
	return IParentDisplayObjectImpl::NonDelegatingQueryInterface(iid, ppv);
}

STDMETHODIMP CVideoRootDO::Render(IUnknown *pDevice, const NORMALIZEDRECT* lpParentRect)
{
	HRESULT hr = S_OK;
    CComPtr<IDispSvrVideoMixer> pVideoMixer;

    CResourceManager::GetInstance()->GetInterface(DISPSVR_RESOURCE_VIDEOMIXER, __uuidof(IDispSvrVideoMixer), (void **)&pVideoMixer);
	if (pVideoMixer)
	{
		hr = pVideoMixer->Execute();

		CAutoLock selfLock(&m_csObj);

		if (!m_pOwner)
			return VFW_E_WRONG_STATE;

		if (!m_bShow)
			return S_OK;

		// this is a temporary workaround for OSD layer
		NORMALIZEDRECT rectOutput;
		MapToParentRect(&rectOutput, lpParentRect, &m_rectOutput);

		DisplayObjects::reverse_iterator it = m_children.rbegin();
		for (; it != m_children.rend(); ++it)
			if (it->lZOrder < 200)
				hr = it->pObj->Render(pDevice, &rectOutput);
		return S_OK;
	}
	else
	{
		hr = IParentDisplayObjectImpl::Render(pDevice, lpParentRect);
	}
	return hr;
}

STDMETHODIMP CVideoRootDO::SetOutputRect(NORMALIZEDRECT* lpNormRect)
{
	HRESULT hr = E_FAIL;
    CComPtr<IDispSvrVideoMixer> pVideoMixer;

    CResourceManager::GetInstance()->GetInterface(DISPSVR_RESOURCE_VIDEOMIXER, __uuidof(IDispSvrVideoMixer), (void **)&pVideoMixer);
	if (pVideoMixer)
	{
		// by default, we only draw video on current render target.
		NORMALIZEDRECT nrcOutput;
		ConvertD3DCoordToMixer(nrcOutput, *lpNormRect);
		hr = pVideoMixer->SetDestination(NULL, &nrcOutput);
		if (SUCCEEDED(hr))
			hr = IParentDisplayObjectImpl::SetOutputRect(lpNormRect);
	}
	return hr;
}
