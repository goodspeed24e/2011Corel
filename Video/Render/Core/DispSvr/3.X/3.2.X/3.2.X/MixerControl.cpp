#include "stdafx.h"
#include "VideoSourceMgr.h"
#include "MixerControl.h"


static inline void KeepAspectRatio(NORMALIZEDRECT* pRect, const RECT &rectDisplay, float fAspectRatio)
{
	if (rectDisplay.right <= rectDisplay.left || rectDisplay.bottom <= rectDisplay.top)
		return;

	float w_2 = (rectDisplay.right - rectDisplay.left) * 0.5f;
	float h_2 = (rectDisplay.bottom - rectDisplay.top) * 0.5f;

	pRect->left = rectDisplay.left + w_2 * (pRect->left + 1);
	pRect->top = rectDisplay.top + h_2 * (1 - pRect->top);
	pRect->right = rectDisplay.right - w_2 * (1 - pRect->right);
	pRect->bottom = rectDisplay.bottom - h_2 * (pRect->bottom + 1);

	float w = pRect->right - pRect->left;
	float h = pRect->bottom - pRect->top;
	if(h*fAspectRatio - w > 0.5f)
	{
		float cy = pRect->top + h/2;
		float height = w / fAspectRatio / 2;
		pRect->top = cy - height;
		pRect->bottom = cy + height;
	}
	else
	{
		float cx = pRect->left + w/2;
		float wide = h * fAspectRatio / 2;
		pRect->left = cx - wide;
		pRect->right = cx + wide;
	}

	pRect->left = -1 + (pRect->left - rectDisplay.left) / w_2;
	pRect->top = 1 - (pRect->top - rectDisplay.top) / h_2;
	pRect->right = 1 - (rectDisplay.right - pRect->right) / w_2;
	pRect->bottom = -1 + (rectDisplay.bottom - pRect->bottom) / h_2;
}

// custom vertex
struct MixerVertex
{
	D3DVECTOR position;
	D3DCOLOR color;
	FLOAT tu;
	FLOAT tv;
	enum { FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1 };
};

// custom primitive for a video source
class CVideoFrame
{
public:
	CVideoFrame();
	~CVideoFrame();

	HRESULT Initialize(IDisplayVideoSource* pVidSrc, FLOAT fAspectRatio);
	HRESULT Reload(IDisplayVideoSource* pVidSrc, FLOAT fAspectRatio);
	HRESULT UpdateOutputRect(IDisplayRenderEngine* pRenderEngine, NORMALIZEDRECT& newnrect);
	HRESULT UpdateVideoRect(IDisplayRenderEngine* pRenderEngine);

	void AdjustVertexTexuture(NORMALIZEDRECT* pRect, NORMALIZEDRECT* pRectTexture);	
	void ReleaseResource();

	// data
	float							m_fAspectRatio;

	CComPtr<IDirect3DVertexBuffer9> m_vertices;
	CComPtr<IDisplayRenderEngine>	m_pRenderEngine;
	CComPtr<IDisplayVideoSource>	m_pVidSrc;
	UCHAR							m_uAlpha;
	NORMALIZEDRECT					m_rectOutput;
	NORMALIZEDRECT					m_rectTexture;
	NORMALIZEDRECT					m_rectParent;
	NORMALIZEDRECT					m_rectZoom;
	NORMALIZEDRECT					m_rectCrop;
	DWORD							m_dwZOrder;
	BOOL							m_bLockRatio;
};

CVideoMixer::CVideoMixer(LPUNKNOWN pUnk, HRESULT *phr)
        : CUnknown(NAME("Display Mixer Control"), pUnk)
{
	m_pSourceMgr = new CVideoSourceManager(static_cast<IDisplayLock*>(this), static_cast<IDisplayVideoSink*>(this));
	m_pSourceMgr->AddRef();
}

CVideoMixer::~CVideoMixer()
{
    Terminate();
}

STDMETHODIMP CVideoMixer::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
    HRESULT hr = E_NOINTERFACE;
    *ppv = NULL;

    if (riid == IID_IDisplayObject)
    {
        hr = GetInterface((IDisplayObject*) this, ppv);
    }
    else if (riid == IID_IDisplayVideoMixer)
    {
        hr = GetInterface((IDisplayVideoMixer*) this, ppv);
    }
    else if (riid == IID_IDisplayVideoSourceManager)
    {
        hr = GetInterface((IDisplayVideoSourceManager*) this, ppv);
    }
    else if (riid == IID_IDisplayVideoSink)
    {
        hr = GetInterface((IDisplayVideoSink*) this, ppv);
    }
	else if (riid == IID_IDisplayOptimizedRender)
    {
        hr = GetInterface((IDisplayOptimizedRender*) this, ppv);
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

//////////////////////////////////////////////////////////////////////////
// IDisplayVideoMixer

STDMETHODIMP CVideoMixer::GetRenderEngineOwner(
    IDisplayRenderEngine** ppRenderEngine)
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

STDMETHODIMP CVideoMixer::Initialize(IDisplayRenderEngine* pRenderEngine)
{
    if (!pRenderEngine)
    {
        return E_POINTER;
    }

    if (!m_pSourceMgr)
	{
		m_pSourceMgr = new CVideoSourceManager(static_cast<IDisplayLock*>(this), static_cast<IDisplayVideoSink*>(this));
		m_pSourceMgr->AddRef();
	}

	if (m_pOwner)
	{
		return S_FALSE;
	}

    m_pOwner = pRenderEngine;
    m_cAlpha = D3DXCOLOR(0, 0, 0, 0xFF);

	CComPtr<IUnknown> pDevice;
	D3DCAPS9 caps;		
	HRESULT hr = m_pOwner->Get3DDevice(&pDevice);
	if (FAILED(hr))
	{
		DbgMsg("CCompositeDisplayObject::Initialize: failed in IRenderEngine::Get3DDevice, hr = 0x%08x", hr);
		return hr;
	}
	CComQIPtr<IDirect3DDevice9> pDevice9 = pDevice;
	hr = pDevice9->GetDeviceCaps(&caps);
	if (FAILED(hr))
	{
		DbgMsg("CCompositeDisplayObject::Initialize: failed in IDirect3DDevice9::GetDeviceCaps, hr = 0x%08x", hr);
		return hr;
	}
	m_bOptimizedRenderSupport = (caps.PrimitiveMiscCaps & D3DPMISCCAPS_PERSTAGECONSTANT) != 0 ? TRUE : FALSE;

	m_bShow = TRUE;
	m_bLockRatio = TRUE;
    return S_OK;
}

// cleans data, releases interfaces
STDMETHODIMP CVideoMixer::Terminate()
{
    VideoFrames::iterator it;

	for (it = m_listFrames.begin(); it != m_listFrames.end(); ++it)
	{
		if (*it)
			delete *it;
	}
	m_listFrames.clear();
	
	for (it = m_listBackup.begin(); it != m_listBackup.end(); ++it)
	{
		if (*it)
			delete *it;
	}

	m_listBackup.clear();
	SAFE_RELEASE(m_pSourceMgr);
	m_pOwner.Release();	
	return S_OK;
}

STDMETHODIMP CVideoMixer::BeginDeviceLoss(void)
{
	BackupFrameState();
 	if (!m_pSourceMgr)
 	{
 	 	DbgMsg("CWizard::BeginDeviceLoss: method Initialize() was not called!");
 	 	return VFW_E_WRONG_STATE;
 	}
	return m_pSourceMgr->BeginDeviceLoss();
}

STDMETHODIMP CVideoMixer::EndDeviceLoss(IUnknown* pDevice)
{
 	if (!m_pSourceMgr)
 	{
 	 	DbgMsg("CWizard::EndDeviceLoss: method Initialize() was not called!");
 	 	return VFW_E_WRONG_STATE;
 	 }
    return m_pSourceMgr->EndDeviceLoss(pDevice);
}

STDMETHODIMP CVideoMixer::ProcessMessage(HWND hWnd, UINT msg, UINT wParam, LONG lParam)
{
    CAutoLock selfLock(&m_csObj);

#ifdef _DEBUG
    switch (msg)
    {
	case WM_LBUTTONDOWN:
    case WM_MOUSEMOVE:
		{
            if (m_pMovingVidSrc && msg == WM_LBUTTONDOWN)
            {
                m_pMovingVidSrc.Release();
                return S_FALSE; // release mouse message
            }

			int x = LOWORD(lParam);
			int y = HIWORD(lParam);

			HWND hWndDisplay = NULL;
			m_pOwner->GetDisplayWindow(&hWndDisplay, NULL);
			RECT rcDisplay;
			GetClientRect(hWndDisplay, &rcDisplay);

			float left = ((float)x / (rcDisplay.right));
			float top = ((float)y / (rcDisplay.bottom));

            if (!m_pMovingVidSrc && msg == WM_LBUTTONDOWN)
            {
                LONG cVidSrc = 0;
                m_pSourceMgr->GetVideoSourceCount(&cVidSrc);
                for (LONG i = cVidSrc - 1; i >= 0; i--)
                {
			        CComPtr<IDisplayVideoSource> pVidSrc;
			        GetVideoSourceByIndex(i, &pVidSrc);

			        NORMALIZEDRECT rect;
			        GetOutputRect(pVidSrc, &rect);

			        bool bInside = 
					        (left >= rect.left && left <= rect.right &&
					         top >= rect.top && top <= rect.bottom);

			        if (bInside)
			        {
                        m_pMovingVidSrc = pVidSrc;
				        m_xDrag = left - rect.left;
				        m_yDrag = top - rect.top;
				        return S_FALSE; // capture mouse message
			        }
                }
            }
			
			if (msg == WM_MOUSEMOVE && m_pMovingVidSrc)
			{
			    NORMALIZEDRECT rect;
			    GetOutputRect(m_pMovingVidSrc, &rect);

				float w = rect.right - rect.left;
				float h = rect.bottom - rect.top;
				rect.left = left - m_xDrag;
				rect.top = top - m_yDrag;
				rect.right = rect.left + w;
				rect.bottom = rect.top + h;
				SetOutputRect(m_pMovingVidSrc, &rect);
				return S_OK;
			}
		}

		break;
    }
#endif

    return E_NOTIMPL;
}

STDMETHODIMP CVideoMixer::Render(IUnknown *pDevice, const NORMALIZEDRECT* lpParentRect)
{
	CComQIPtr<IDirect3DDevice9> pDevice9 = pDevice;

	if (!m_pOwner)
    {
        return VFW_E_WRONG_STATE;
    }

    if (!pDevice9)
    {
        DbgMsg("CVideoMixer::Render: received NULL pointer");
        return E_POINTER;
    }

	if (!m_bShow)
	{
		DbgMsg("CVideoMixer::Render: Render skipped");
		return S_FALSE;
	}

	if (m_listFrames.size() <= 0)
		return S_OK;

	HRESULT hr = S_OK;
    try
    {
		CAutoLock selfLock(&m_csObj);

		D3DXMATRIX idMatrix;
		D3DXMatrixIdentity(&idMatrix);
		hr = pDevice9->SetTransform(D3DTS_VIEW, &idMatrix);
		hr = pDevice9->SetTransform(D3DTS_PROJECTION, &idMatrix);

		hr = pDevice9->SetRenderState(D3DRS_LIGHTING, FALSE);
		hr = pDevice9->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

		hr = pDevice9->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
		hr = pDevice9->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
		hr = pDevice9->SetSamplerState(0, D3DSAMP_BORDERCOLOR, 0x00000000);

		hr = pDevice9->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
		hr = pDevice9->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		hr = pDevice9->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

        for (VideoFrames::iterator it = m_listFrames.begin(); it != m_listFrames.end(); ++it)
        {
            CVideoFrame* pFrame = *it;
            CComPtr<IDisplayVideoSource> pVidSrc = pFrame->m_pVidSrc;

			if (pFrame->m_uAlpha == 0)
				continue;

            // we need to update video rect before drawing in case that display rect has changed.
			if (m_bLockRatio)
			{
				LONG wImage, hImage;
				pVidSrc->GetVideoSize(&wImage, &hImage, &pFrame->m_fAspectRatio);
			}
			else
				pFrame->m_fAspectRatio = 0.f;
			pFrame->m_rectParent = *lpParentRect;

			CComPtr<IUnknown> pTexture;
			hr = pVidSrc->GetTexture(&pTexture, &pFrame->m_rectTexture);
			CComQIPtr<IDirect3DTexture9> pTexture9 = pTexture;
			if (!pTexture9)
				continue;

			hr = pDevice9->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
			if (pFrame->m_uAlpha != 255)
			{
				hr = pDevice9->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
				hr = pDevice9->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
				hr = pDevice9->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
				hr = pDevice9->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
				hr = pDevice9->SetRenderState(D3DRS_ALPHAREF, 0x0);
				hr = pDevice9->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
				hr = pDevice9->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
			}
			else
			{
				hr = pDevice9->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);				
				hr = pDevice9->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
				hr = pDevice9->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
			}

			pFrame->UpdateVideoRect(m_pOwner);

			hr = pDevice9->SetTexture(0, pTexture9);
			hr = pDevice9->SetFVF(MixerVertex::FVF);
			hr = pDevice9->SetStreamSource(0, pFrame->m_vertices, 0, sizeof(MixerVertex)); 

			hr = pVidSrc->BeginDraw();
			// draw the primitive
			hr = pDevice9->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
			hr = pVidSrc->EndDraw();
        }
		hr = pDevice9->SetTexture(0, NULL);
    }
    catch (HRESULT hrFailed)
    {
        hr = hrFailed;
    }

    return hr;
}

STDMETHODIMP CVideoMixer::MultiTextureRender(IUnknown *pDevice, LONG lStageIdx, LONG lTexIdx, const NORMALIZEDRECT* lpParentRect)
{
	if (!m_pOwner)
    {
        return VFW_E_WRONG_STATE;
    }

    if (!pDevice)
    {
        DbgMsg("CVideoMixer::Render: received NULL pointer");
        return E_POINTER;
    }

	if (lStageIdx < 0 || lStageIdx > 8)
		return E_INVALIDARG;

	HRESULT hr = S_OK;

    try
    {
		CComQIPtr<IDirect3DDevice9> pDevice9 = pDevice;
		CAutoLock selfLock(&m_csObj);

		if (m_listFrames.begin() == m_listFrames.end())
			return S_FALSE;

		VideoFrames::iterator it;
		it = m_listFrames.begin() + lTexIdx;
		CVideoFrame* pFrame = (CVideoFrame*)(*it);
		
		// we need to update video rect before drawing in case that display rect has changed.
		pFrame->m_rectParent = *lpParentRect;
		pFrame->UpdateVideoRect(m_pOwner);
		
		CComPtr<IDisplayVideoSource> pVidSrc = pFrame->m_pVidSrc;
		
		CComPtr<IUnknown> pTexture;
		hr = pVidSrc->GetTexture(&pTexture, NULL);
		CComQIPtr<IDirect3DTexture9> pTexture9 = pTexture;
		CHECK_HR(hr, DbgMsg("CVideoMixer::Render: cannot find the texture!, hr = 0x%08x, pTexture = 0x%08x", hr, pTexture9));
		
		hr = pDevice9->SetTexture(lStageIdx, pTexture9);
		CHECK_HR(hr, DbgMsg("CVideoMixer::Render: failed to set texture for hr = 0x%08x, pTexture = 0x%08x", hr, pTexture9));
		
		hr = pDevice9->SetSamplerState(lStageIdx, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC);
		CHECK_HR(hr, DbgMsg("CRenderEngine::CreateD3DDevice: failed in SetSamplerState(D3DSAMP_MAGFILTER...)"));
		
		hr = pDevice9->SetSamplerState(lStageIdx, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);
		CHECK_HR(hr, DbgMsg("CRenderEngine::CreateD3DDevice: failed in SetSamplerState(D3DSAMP_MINFILTER...)"));
		
		hr = pDevice9->SetSamplerState(lStageIdx, D3DSAMP_ADDRESSU, D3DTADDRESS_BORDER);
		CHECK_HR(hr, DbgMsg("CRenderEngine::CreateD3DDevice: failed in SetSamplerState(D3DSAMP_ADDRESSU...)"));

		hr = pDevice9->SetSamplerState(lStageIdx, D3DSAMP_ADDRESSV, D3DTADDRESS_BORDER);
		CHECK_HR(hr, DbgMsg("CRenderEngine::CreateD3DDevice: failed in SetSamplerState(D3DSAMP_ADDRESSV...)"));

		hr = pDevice9->SetSamplerState(lStageIdx, D3DSAMP_BORDERCOLOR, 0x00000000);
		CHECK_HR(hr, DbgMsg("CRenderEngine::CreateD3DDevice: failed in SetSamplerState(D3DSAMP_BORDERCOLOR...)"));
		
		hr = pDevice9->SetTextureStageState(lStageIdx, D3DTSS_TEXCOORDINDEX, lStageIdx);
		CHECK_HR(hr, DbgMsg("CVideoMixer::Render: failed in SetTextureStageState(D3DTSS_TEXCOORDINDEX...), hr = 0x%08x", hr));

		// code block for alpha blending
		hr = pDevice9->SetTextureStageState(lStageIdx, D3DTSS_CONSTANT, m_cAlpha);
		CHECK_HR(hr, DbgMsg("CVideoMixer::Render: failed in SetTextureStageState(D3DTSS_CONSTANT...), hr = 0x%08x", hr));

//		hr = pDevice9->SetTextureStageState(lStageIdx, D3DTSS_ALPHAARG1, D3DTA_CONSTANT);
		CHECK_HR(hr, DbgMsg("CVideoMixer::Render: failed in SetTextureStageState(D3DTSS_ALPHAARG1...), hr = 0x%08x", hr));

//		hr = pDevice9->SetTextureStageState(lStageIdx, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
		CHECK_HR(hr, DbgMsg("CVideoMixer::Render: failed in SetTextureStageState(D3DTSS_ALPHAOP...), hr = 0x%08x", hr));
    }
    catch (HRESULT hrFailed)
    {
        hr = hrFailed;
    }

    return hr;
}

STDMETHODIMP CVideoMixer::AddVideoSource(IBaseFilter* pVMR, IDisplayVideoSource** ppVidSrc)
{
    if (!m_pSourceMgr)
    {
        DbgMsg("CWizard::RemoveVideoSource: method Initialize() was not called!");
        return VFW_E_WRONG_STATE;
    }

    return m_pSourceMgr->AddVideoSource(pVMR, ppVidSrc);
}

STDMETHODIMP CVideoMixer::RemoveVideoSource(IDisplayVideoSource* pVidSrc)
{
    if (!m_pSourceMgr)
    {
        DbgMsg("CWizard::RemoveVideoSource: method 'Initialize' was never called");
        return VFW_E_WRONG_STATE;
    }

    return m_pSourceMgr->RemoveVideoSource(pVidSrc);
}

STDMETHODIMP CVideoMixer::GetVideoSourceCount(LONG* plCount)
{
    if (!m_pSourceMgr)
    {
        DbgMsg("CWizard::GetVideoSourceCount: Method 'Initialize' was never called");
        return VFW_E_WRONG_STATE;
    }

    return m_pSourceMgr->GetVideoSourceCount(plCount);
}

STDMETHODIMP CVideoMixer::GetVideoSourceByIndex(
    LONG lIndex,
    IDisplayVideoSource** ppVideoSource)
{
    if (!m_pSourceMgr)
    {
        DbgMsg("CWizard::GetVideoSourceByIndex: Method 'Initialize' was never called");
        return VFW_E_WRONG_STATE;
    }

    return m_pSourceMgr->GetVideoSourceByIndex(lIndex, ppVideoSource);
}

//////////////////////////////////////////////////////////////////////////
// IDisplayVideoSink

STDMETHODIMP CVideoMixer::OnVideoSourceAdded(
    IDisplayVideoSource* pVidSrc,
	FLOAT fAspectRatio)
{
    HRESULT hr = S_OK;

    if (!m_pOwner)
    {
        return VFW_E_WRONG_STATE;
    }

    try
    {
        CAutoLock selfLock(&m_csObj);

        // check if we already have this frame
        CVideoFrame* pFrame = GetVideoFrame(pVidSrc);
        if (pFrame)
        {
            hr = pFrame->Reload(pVidSrc, fAspectRatio);
            CHECK_HR(hr, DbgMsg("CVideoMixer::AddVideoSource: failed to reload frame"));
            return S_OK;
        }

        // ok, create new frame

		// restore if frame has previous state
		VideoFrames::iterator it = m_listBackup.begin();
		for (; it != m_listBackup.end(); it++)
		{
			CVideoFrame* pFrameBackup = *it;
			if (pVidSrc == pFrameBackup->m_pVidSrc)
			{
				pFrame = new CVideoFrame(*pFrameBackup);
				break;
			}
		}

		if (!pFrame)
		{
            pFrame = new CVideoFrame();
			hr = pFrame->Initialize(pVidSrc, fAspectRatio);
            if (FAILED(hr))
            {
				delete pFrame;
                DbgMsg("CVideoMixer::AddVideoSource: failed in CVideoFrame::Initialize, hr = 0x%08x", hr);
                throw hr;
            }
			
			// now add the frame to the end of the list, incrementing Z-Order of already
			// existing frames
			for (it = m_listFrames.begin(); it != m_listFrames.end(); ++it)
			{
				(*it)->m_dwZOrder++;
			}
			pFrame->m_dwZOrder = 0;
		}
        m_listFrames.push_back(pFrame);

		pFrame->UpdateVideoRect(m_pOwner);
    }
    catch (HRESULT hrFailed)
    {
        hr = hrFailed;
    }

    return hr;
}

STDMETHODIMP CVideoMixer::OnVideoSourceRemoved(IDisplayVideoSource* pVidSrc)
{
    if (!m_pOwner)
    {
        return VFW_E_WRONG_STATE;
    }

	CAutoLock selfLock(&m_csObj);

	// Workaround: backup frame state before video removed
	BackupFrameState();

    // we do not have to check if this pVidSrc is registered with the wizard, so
    // just go through the list and delete; decrease Z-order of all preceding frames
	VideoFrames::iterator it;
	for (it = m_listFrames.begin(); it != m_listFrames.end(); ++it)
    {
        if (pVidSrc == (*it)->m_pVidSrc)
        {
			delete *it;
            break;
        }
    }

    if (it == m_listFrames.end()) // we did not find the frame
    {
        DbgMsg("CVideoMixer::OnVideoSourceRemoved: cannot find frame for source 0x%08x", pVidSrc);
        return VFW_E_NOT_FOUND;
    }

	m_listFrames.erase(it);
    int nCounter = m_listFrames.size() - 1;

    for (it = m_listFrames.begin(); it != m_listFrames.end(); ++it, --nCounter)
    {
        (*it)->m_dwZOrder = nCounter;
    }
    return S_OK;
}

STDMETHODIMP CVideoMixer::GetOutputRect(IDisplayVideoSource* pVidSrc, NORMALIZEDRECT* lpNormRect)
{
    if (!m_pOwner)
    {
        return VFW_E_WRONG_STATE;
    }

    if (!lpNormRect)
    {
        DbgMsg("CVideoMixer::GetOutputRect: received NULL pointer");
        return E_POINTER;
    }
    
	if (CVideoFrame *pFrameRequested = GetVideoFrame(pVidSrc))
	{
		*lpNormRect = pFrameRequested->m_rectOutput;
		return S_OK;
    }
	return VFW_E_NOT_FOUND;
}

STDMETHODIMP CVideoMixer::SetOutputRect(
    IDisplayVideoSource* pVidSrc,
    NORMALIZEDRECT* lpNormRect)
{
    if (!m_pOwner)
    {
        return VFW_E_WRONG_STATE;
    }

    if (!lpNormRect)
    {
        DbgMsg("CVideoMixer::SetOutputRect: received NULL pointer");
        return E_POINTER;
    }

    CVideoFrame *pFrameRequested = GetVideoFrame(pVidSrc);

    if (!pFrameRequested)
    {
        DbgMsg("CVideoMixer::SetOutputRect: requested video source 0x%08x was not found", pVidSrc);
        return VFW_E_NOT_FOUND;
    }

    CAutoLock selfLock(&m_csObj);

    HRESULT hr = pFrameRequested->UpdateOutputRect(m_pOwner, *lpNormRect);
    if (FAILED(hr))
    {
        DbgMsg("CVideoMixer::SetOutputRect: failed in CVideoFrame::UpdateOutputRect, "\
                 "hr = 0x%08x", hr);
    }

    return hr;
}

STDMETHODIMP CVideoMixer::GetIdealOutputRect(
    IDisplayVideoSource* pVidSrc,
    DWORD dwWidth,
    DWORD dwHeight,
    NORMALIZEDRECT* lpNormRect)
{
    if (!m_pOwner)
    {
        return VFW_E_WRONG_STATE;
    }

    if (!lpNormRect)
    {
        DbgMsg("CVideoMixer::GetIdealOutputRect: received NULL pointer");
        return E_POINTER;
    }

    lpNormRect->left = -1;
    lpNormRect->right = 1;
    lpNormRect->top = 1;
    lpNormRect->bottom = -1;

    return S_OK;
}

STDMETHODIMP CVideoMixer::GetZOrder(IDisplayVideoSource* pVidSrc, DWORD *pdwZ)
{
    if (!m_pOwner)
    {
        return VFW_E_WRONG_STATE;
    }

    if (!pdwZ)
    {
        DbgMsg("CVideoMixer::GetZOrder: received NULL pointer");
        return E_POINTER;
    }
    
	CVideoFrame *pFrameRequested = GetVideoFrame(pVidSrc);

    if (!pFrameRequested)
    {
        DbgMsg("CVideoMixer::GetZOrder: requested video source 0x%08x was not found", pVidSrc);
        return VFW_E_NOT_FOUND;
    }

    *pdwZ = pFrameRequested->m_dwZOrder;
    return S_OK;
}

STDMETHODIMP CVideoMixer::SetZOrder(IDisplayVideoSource* pVidSrc, DWORD dwZ)
{
    CVideoFrame* pFrameRequested = NULL;
    CVideoFrame* pFrame = NULL;
    CVideoFrame* pFrameTo = NULL;

    int nCounter;

    if (!m_pOwner)
    {
        return VFW_E_WRONG_STATE;
    }

    if (dwZ > m_listFrames.size() - 1)
    {
        dwZ = m_listFrames.size() - 1;
    }

    VideoFrames::iterator begin, end, it, itRequested;

    begin = m_listFrames.begin();
    itRequested = end = m_listFrames.end();

    for (it = begin; it != end; it++)
    {
        pFrame = (CVideoFrame*)(*it);
        if (pVidSrc == pFrame->m_pVidSrc)
        {
            pFrameRequested = pFrame;
            itRequested = it;
        }
        if (pFrame->m_dwZOrder == dwZ)
        {
            pFrameTo = pFrame;
        }
    }
    if (!pFrameRequested)
    {
        DbgMsg("CVideoMixer::SetZOrder: requested video source 0x%08x was not found", pVidSrc);
        return VFW_E_NOT_FOUND;
    }

    CAutoLock selfLock(&m_csObj);

    if (dwZ == pFrameRequested->m_dwZOrder)
    {
        return S_FALSE;
    }

    // 1. remove pFrameRequested
    m_listFrames.erase(itRequested);

    // 2. insert
    begin = m_listFrames.begin();
    end = m_listFrames.end();
    for (it = begin; it != end; ++it)
    {
        pFrame = *it;
        if (pFrame == pFrameTo)
        {
            if (dwZ < pFrameRequested->m_dwZOrder)
            {
                ++it;
            }
            if (it == end)
            {
                m_listFrames.push_back(pFrameRequested);
            }
            else
            {
                m_listFrames.insert(it, pFrameRequested);
            }
            break;
        }
    }

    nCounter = m_listFrames.size() - 1;
    begin = m_listFrames.begin();
    end = m_listFrames.end();

    for (it = begin; it != end; ++it)
    {
        pFrame = *it;
        pFrame->m_dwZOrder = (DWORD)nCounter;
        nCounter--;
    }

    return S_OK;
}

STDMETHODIMP CVideoMixer::GetAlpha(IDisplayVideoSource* pVidSrc, float* pAlpha)
{
    if (!m_pOwner)
    {
        return VFW_E_WRONG_STATE;
    }
    if (!pAlpha)
    {
        DbgMsg("CVideoMixer::GetAlpha: received NULL pointer");
        return E_POINTER;
    }
    
	CVideoFrame *pFrameRequested = GetVideoFrame(pVidSrc);
    if (!pFrameRequested)
    {
        DbgMsg("CVideoMixer::GetAlpha: requested video source 0x%08x was not found", pVidSrc);
        return VFW_E_NOT_FOUND;
    }

    *pAlpha = float(pFrameRequested->m_uAlpha) / 255;
    return S_OK;
}

STDMETHODIMP CVideoMixer::SetAlpha(IDisplayVideoSource* pVidSrc, float Alpha)
{
    if (!m_pOwner)
    {
        return VFW_E_WRONG_STATE;
    }

	if (Alpha < 0.0f)
		Alpha = 0.0f;
	if (Alpha > 1.0f)
		Alpha = 1.0f;

	CVideoFrame *pFrameRequested = GetVideoFrame(pVidSrc);
    if (!pFrameRequested)
    {
        DbgMsg("CVideoMixer::SetAlpha: requested video source 0x%08x was not found", pVidSrc);
        return VFW_E_NOT_FOUND;
    }

    CAutoLock selfLock(&m_csObj);

    pFrameRequested->m_uAlpha = static_cast<UCHAR>(Alpha * 255);
	m_cAlpha = D3DXCOLOR(0,0,0,Alpha);
    return S_OK;
}

STDMETHODIMP CVideoMixer::GetCLSID(CLSID* pClsid)
{
    if (!pClsid) {
        return E_POINTER;
    }

    *pClsid = CLSID_DisplayVideoMixer;
    return S_OK;
}

STDMETHODIMP CVideoMixer::NotifyEvent(DWORD event, DWORD param1, DWORD param2, LPVOID pInstance)
{
	// clean up the video frame when stopping rendering
	HRESULT hr = S_OK;

	switch (event)
	{
	case DISPLAY_EVENT_EnableRendering:
		if (param1 == FALSE)
			hr = ClearFrame();
		break;
	case DISPLAY_EVENT_VideoSourceRender:
		hr = m_pOwner->NodeRequest(DISPLAY_REQUEST_Render, param1, param2, reinterpret_cast<IDisplayObject *> (pInstance));
		break;
	case DISPLAY_EVENT_VideoSourcePresent:
		hr = m_pOwner->NodeRequest(DISPLAY_REQUEST_Present, param1, param2, reinterpret_cast<IDisplayObject *> (pInstance));
		break;
	}
	return hr;
}


STDMETHODIMP CVideoMixer::GetTextureCount(LONG* plCount)
{
	if (!m_bOptimizedRenderSupport)
		return E_FAIL;
	
	*plCount = m_listFrames.size();
	return S_OK;
}

STDMETHODIMP CVideoMixer::GetTextureCoord(LONG lIndex, NORMALIZEDRECT* lpNormRect)
{
	if (lIndex >= (LONG) m_listFrames.size())
		return E_INVALIDARG;

	// convert from [-1,1] [1,-1] to [0,0] [1,1]
	NORMALIZEDRECT rectTexture;
	m_listFrames[lIndex]->AdjustVertexTexuture(lpNormRect, &rectTexture);

	float fZoomFactorH = (lpNormRect->right-lpNormRect->left)/2;
	float fZoomFactorV = (lpNormRect->top-lpNormRect->bottom)/2;
	float fPanFactorH = -(lpNormRect->left + (lpNormRect->right-lpNormRect->left)/2)/(lpNormRect->right-lpNormRect->left);
	float fPanFactorV = (lpNormRect->bottom + (lpNormRect->top-lpNormRect->bottom)/2)/(lpNormRect->top-lpNormRect->bottom);

	lpNormRect->left	= -(1/fZoomFactorH-1)/2 + fPanFactorH;
	lpNormRect->top		= -(1/fZoomFactorV-1)/2 + fPanFactorV;
	lpNormRect->right	= (1/fZoomFactorH-1)/2 + 1 + fPanFactorH;
	lpNormRect->bottom	= (1/fZoomFactorV-1)/2 + 1 + fPanFactorV;

	return S_OK;
}

STDMETHODIMP CVideoMixer::KeepAspectRatio(IDisplayVideoSource* pVidSrc, BOOL bLockRatio)
{
    if (!m_pOwner)
    {
        return VFW_E_WRONG_STATE;
    }

	if (pVidSrc)
	{
		CVideoFrame *pFrameRequested = GetVideoFrame(pVidSrc);
		if (!pFrameRequested)
		{
			DbgMsg("CVideoMixer::SetAlpha: requested video source 0x%08x was not found", pVidSrc);
			return VFW_E_NOT_FOUND;
		}

		CAutoLock selfLock(&m_csObj);

		pFrameRequested->m_bLockRatio = bLockRatio;
	}
	else
	{
		// disable the entire ratio calculation of the mixer.
		m_bLockRatio = bLockRatio;
	}

	return S_OK;
}

STDMETHODIMP CVideoMixer::Lock()
{
    m_csObj.Lock();
    return S_OK;
}

STDMETHODIMP CVideoMixer::Unlock()
{
    m_csObj.Unlock();
    return S_OK;
}

STDMETHODIMP CVideoMixer::TryLock()
{
	return E_NOTIMPL;
}

//////////////////////////////////////////////////////////////////////////
// IDisplayProperties
//
// Note that a call to video mixer's IDisplayProperties will succeed only
// when there is only one video source inside the mixer.

STDMETHODIMP CVideoMixer::GetOutputRect(NORMALIZEDRECT* lpNormRect)
{
	if (lpNormRect == NULL) {
		return E_POINTER;
	}

    if (m_listFrames.size() <= 0) {
        return E_FAIL;
    }

	*lpNormRect = m_listFrames.front()->m_rectOutput;
	return S_OK;
}

STDMETHODIMP CVideoMixer::SetOutputRect(NORMALIZEDRECT* lpNormRect)
{
	if (lpNormRect == NULL) {
		return E_POINTER;
	}

    if (m_listFrames.size() <= 0) {
        return E_FAIL;
    }

    CComPtr<IDisplayVideoSource> pVidSrc;
    HRESULT hr = m_pSourceMgr->GetVideoSourceByIndex(0, &pVidSrc);
    if (SUCCEEDED(hr))
    {
        hr = SetOutputRect(pVidSrc, lpNormRect);
    }
	
	return hr;
}

STDMETHODIMP CVideoMixer::GetCropRect(NORMALIZEDRECT* lpNormRect)
{
	if (lpNormRect == NULL) {
		return E_POINTER;
	}

    if (m_listFrames.size() <= 0) {
        return E_FAIL;
    }

    *lpNormRect = m_listFrames.front()->m_rectCrop;
    return S_OK;
}

STDMETHODIMP CVideoMixer::SetCropRect(NORMALIZEDRECT* lpNormRect)
{
	if (lpNormRect == NULL) {
		return E_POINTER;
	}

    if (m_listFrames.size() <= 0) {
        return E_FAIL;
    }

	VideoFrames::iterator it = m_listFrames.begin();	
	for (; it != m_listFrames.end(); ++it)
	{
		(*it)->m_rectCrop = *lpNormRect;
	}
    return S_OK;
}

STDMETHODIMP CVideoMixer::GetFrameColor(COLORREF* pColor)
{
    return E_NOTIMPL;
}

STDMETHODIMP CVideoMixer::SetFrameColor(COLORREF color)
{
	return E_NOTIMPL;
}

STDMETHODIMP CVideoMixer::CaptureFrame(DWORD dwFormat, BYTE** ppFrame, UINT* pSize)
{
    return E_NOTIMPL;
}

STDMETHODIMP CVideoMixer::GetZoom(NORMALIZEDRECT* lpNormRect)
{
	if (lpNormRect == NULL) {
        return E_POINTER;
    }

    *lpNormRect = m_listFrames.front()->m_rectZoom;
    return S_OK;
}

STDMETHODIMP CVideoMixer::SetZoom(NORMALIZEDRECT* lpNormRect)
{
	if (lpNormRect == NULL) {
        return E_POINTER;
    }
	
	VideoFrames::iterator it = m_listFrames.begin();	
	for (; it != m_listFrames.end(); ++it)
	{
		(*it)->m_rectZoom = *lpNormRect;
	}
	return S_OK;
}

STDMETHODIMP CVideoMixer::SetShow(BOOL bShow)
{
	m_bShow = bShow;
	return S_OK;
}

STDMETHODIMP CVideoMixer::GetShow(BOOL* bShow)
{
	if (bShow == NULL) {
		return E_POINTER;
	}

	*bShow = m_bShow;
	return S_OK;
}

STDMETHODIMP CVideoMixer::ClearFrame()
{
	HRESULT hr = S_OK;
	CAutoLock selfLock(&m_csObj);

	for (VideoFrames::iterator it = m_listFrames.begin(); it != m_listFrames.end(); ++it)
	{
		CComPtr<IUnknown> pUnknown;

		hr = (*it)->m_pVidSrc->GetTexture(&pUnknown, NULL);
		if (SUCCEEDED(hr))
		{
			CComQIPtr<IDirect3DTexture9> pTexture = pUnknown;
			if (pTexture)
			{
				CComPtr<IDirect3DDevice9> pDevice9;
				CComPtr<IDirect3DSurface9> pSurface;

				hr = pTexture->GetDevice(&pDevice9);
				hr = pTexture->GetSurfaceLevel(0, &pSurface);
				if (pSurface && pDevice9)
					hr = pDevice9->ColorFill(pSurface, NULL, D3DCOLOR_XRGB(0x00, 0x00, 0x00));
			}
		}
		pUnknown.Release();
	}
	return hr;
}

/////////////////////// Private routine ///////////////////////////////////////

CVideoFrame::CVideoFrame() :
	m_fAspectRatio(0.f),
	m_uAlpha(255),
	m_dwZOrder(0L)
{
}

CVideoFrame::~CVideoFrame()
{}

HRESULT CVideoFrame::Initialize(IDisplayVideoSource* pVidSrc, FLOAT fAspectRatio)
{
    m_pVidSrc = pVidSrc;
	m_bLockRatio = TRUE;

	m_uAlpha = 255;
	m_fAspectRatio = fAspectRatio;

    // coordinates in composition space [-1,1]x[1,-1]
    m_rectOutput.left = m_rectOutput.bottom = -1.f;
    m_rectOutput.top = m_rectOutput.right = 1.f;

    m_rectTexture.left = m_rectTexture.top = 0.f;
    m_rectTexture.right = m_rectTexture.bottom = 1.f;

	m_rectZoom = m_rectParent = m_rectOutput;
	m_rectCrop = m_rectTexture;
	m_vertices.Release();
    return S_OK;
}

HRESULT CVideoFrame::Reload(IDisplayVideoSource* pVidSrc, FLOAT fAspectRatio)
{
    if (m_pVidSrc != pVidSrc)
        return E_INVALIDARG;

	m_fAspectRatio = fAspectRatio;

    m_rectOutput.left	= -1.f;
    m_rectOutput.top	= 1.f;
    m_rectOutput.right	= 1.f;
    m_rectOutput.bottom	= -1.f;
	m_vertices.Release();
    return S_OK;
}

// Updates destination normrect and (x,y) coordinates. 
HRESULT CVideoFrame::UpdateOutputRect(
	IDisplayRenderEngine* pRenderEngine, 
    NORMALIZEDRECT& newnrect)
{
	m_rectOutput = newnrect;
	return UpdateVideoRect(pRenderEngine);
}

HRESULT CVideoFrame::UpdateVideoRect(IDisplayRenderEngine* pRenderEngine)
{
	HRESULT hr = E_FAIL;
	NORMALIZEDRECT rect, rectTexture;

	AdjustVertexTexuture(&rect, &rectTexture);
    MixerVertex vertices[] =
    {
        { rect.left,	rect.top,	0.5f, D3DCOLOR_ARGB(m_uAlpha, 255, 255, 255),	rectTexture.left,	rectTexture.top },
        { rect.right,	rect.top,	0.5f, D3DCOLOR_ARGB(m_uAlpha, 255, 255, 255),	rectTexture.right,	rectTexture.top },
        { rect.left,	rect.bottom,0.5f, D3DCOLOR_ARGB(m_uAlpha, 255, 255, 255),	rectTexture.left,	rectTexture.bottom },
        { rect.right,	rect.bottom,0.5f, D3DCOLOR_ARGB(m_uAlpha, 255, 255, 255),	rectTexture.right,	rectTexture.bottom },
    };

	m_pRenderEngine = pRenderEngine;
	if (!m_vertices && m_pRenderEngine)
	{
		CComPtr<IUnknown> pDevice;
		hr = m_pRenderEngine->Get3DDevice(&pDevice);
		CComQIPtr<IDirect3DDevice9> pDevice9 = pDevice;
		if (FAILED(hr)) { // it may fail it device is lost.
			return hr;
		}
		
		hr = pDevice9->CreateVertexBuffer(4*sizeof(MixerVertex), D3DUSAGE_DYNAMIC, MixerVertex::FVF, D3DPOOL_DEFAULT, &m_vertices, NULL); 
		if (FAILED(hr)) {
			return hr;
		}
	}
	
	if (m_vertices)
	{
		MixerVertex* pVertices = NULL;
		if (SUCCEEDED(m_vertices->Lock(0, 4*sizeof(MixerVertex), (void**)&pVertices, D3DLOCK_DISCARD))) 
		{
			memcpy(pVertices, vertices, sizeof(vertices));
			hr = m_vertices->Unlock();
		}
	}
	
	return S_OK;
}

void CVideoFrame::AdjustVertexTexuture(NORMALIZEDRECT *pRect, NORMALIZEDRECT *pRectTexture)
{
	float w_2 = (m_rectParent.right - m_rectParent.left) * 0.5f;
	float h_2 = (m_rectParent.top - m_rectParent.bottom) * 0.5f;

	pRect->left = m_rectParent.left + (m_rectOutput.left + 1.0f) * w_2;
	pRect->top = m_rectParent.top - (1.0f - m_rectOutput.top) * h_2;
	pRect->right = m_rectParent.right - (1.0f - m_rectOutput.right) * w_2;
	pRect->bottom = m_rectParent.bottom + (m_rectOutput.bottom + 1.0f) * h_2;

	if (m_fAspectRatio > 0.00001 && m_bLockRatio && m_pRenderEngine)
	{
		HWND hWnd;
		RECT rectDisplay;
		m_pRenderEngine->GetDisplayWindow(&hWnd, &rectDisplay);

		// if display rect invalid, try to use hWnd's rect
		if (hWnd && (rectDisplay.right == rectDisplay.left || rectDisplay.bottom == rectDisplay.top))
			GetClientRect(hWnd, &rectDisplay);

		KeepAspectRatio(pRect, rectDisplay, m_fAspectRatio);
	}

	*pRectTexture = m_rectTexture;
	DispSvr::CropRect(*pRect, m_rectCrop);
	DispSvr::CropRect(*pRectTexture, m_rectCrop);

	w_2 = (m_rectZoom.right - m_rectZoom.left) / 2;
	h_2 = (m_rectZoom.top - m_rectZoom.bottom) / 2;

	// Apply zoom after m_rectOutput and m_rectTexture are set correctly.
	pRect->left = -1 + (pRect->left - m_rectZoom.left) / w_2;
	pRect->bottom = -1 + (pRect->bottom - m_rectZoom.bottom) / h_2;
	pRect->top = 1 - (m_rectZoom.top - pRect->top) / h_2;
	pRect->right = 1 - (m_rectZoom.right - pRect->right) / w_2;
}

void CVideoFrame::ReleaseResource()
{
	m_vertices.Release();
}

STDMETHODIMP CVideoMixer::Get3DDevice(IUnknown** ppDevice)
{
    if (!m_pOwner)
    {
        return VFW_E_WRONG_STATE;
    }

    return m_pOwner->Get3DDevice(ppDevice);
}

CVideoFrame* CVideoMixer::GetVideoFrame(IDisplayVideoSource* pVidSrc)
{
    for (VideoFrames::iterator it = m_listFrames.begin(); it != m_listFrames.end(); ++it)
    {
        if (pVidSrc == (*it)->m_pVidSrc)
            return *it;
    }
	return NULL;
}

void CVideoMixer::BackupFrameState()
{
	// if there's nothing to backup, don't clear backup list.
	if (m_listFrames.size() <= 0)
		return;

	VideoFrames::iterator it;

	for (it = m_listBackup.begin(); it != m_listBackup.end(); ++it)
	{
		delete *it;
	}
	m_listBackup.clear();

	for (it = m_listFrames.begin(); it != m_listFrames.end(); ++it)
	{
		(*it)->ReleaseResource();
		m_listBackup.push_back(new CVideoFrame(*(*it)));
	}
}