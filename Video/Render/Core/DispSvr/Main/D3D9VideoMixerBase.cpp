#include "stdafx.h"
#include "DispSvr.h"
#include "DynLibManager.h"
#include "ResourceManager.h"
#include "RegistryService.h"
#include "MathVideoMixing.h"
#include "D3D9VideoMixerBase.h"
#include "DXVA2VideoProcessor.h"
#include "D3D9VideoEffect3DManager.h"
#include "Imports/XVE/Inc/XVE.h"
#include "XVEManagerWrapper.h"

using namespace DispSvr;

// Recover colors by dividing with alpha.
class CSWInverseAlphaFilter : public ITextureFilter
{
public:
    CSWInverseAlphaFilter(D3D9Plane *pPlane) : m_pPlane(pPlane)
    {
        SetRectEmpty(&m_rcVisible);
    }

    virtual ~CSWInverseAlphaFilter() { }

    virtual HRESULT Process(const RECT &rcLocked, const LockedRect &lockedBuffer)
    {
        int Width = rcLocked.right - rcLocked.left;
        int Height = rcLocked.bottom - rcLocked.top;

        UCHAR *pLine = static_cast<UCHAR *> (lockedBuffer.pBuffer);
        UCHAR *pEnd = pLine + Height * lockedBuffer.uPitch;
        double r, g, b, a;
        bool bVisible = false;

        for ( ; pLine < pEnd; pLine += lockedBuffer.uPitch)
        {
            for (UCHAR *p = pLine; p < pLine + Width * 4; p += 4)
            {
                if (p[3])
                {
                    a = 255.0 / p[3];
                    b = p[0] * a;
                    g = p[1] * a;
                    r = p[2] * a;

                    ASSERT(b <= 255.1 && g <= 255.1 && r <= 255.1);
                    p[0] = UCHAR(b);
                    p[1] = UCHAR(g);
                    p[2] = UCHAR(r);
                    bVisible = true;
                }
            }
        }

        if (!bVisible)
            SubtractRect(&m_rcVisible, &m_rcVisible, &rcLocked);
        else
            UnionRect(&m_rcVisible, &m_rcVisible, &rcLocked);

        // return other than S_OK to make the plane invalid (invisible)
        return IsRectEmpty(&m_rcVisible) ? E_FAIL : S_OK;
    }

private:
    D3D9Plane *m_pPlane;
    RECT m_rcVisible;
};

static inline void LoadDefaultD3D9Plane(D3D9Plane &plane)
{
    ZeroMemory(&plane, sizeof(D3D9Plane));
    plane.fAlpha = 1.0f;
    LoadDefaultMixerCoordinate(plane.nrcDst);
    LoadDefaultMixerCoordinate(plane.nrcCrop);
}

ID3D9VBlt *CreateVBltFromVPStub(D3D9Plane *pPlane, VideoProcessorStub *pVPStub, CD3D9TexturePool *pPool)
{
    // some of video processor may not have factory a method.
    // plane.pVBlt should be set to null in those cases.
    if (!pVPStub->pfnVBltFactory)
    {
        if (pVPStub->pDelegateVPStub && pVPStub->pDelegateVPStub->pfnVBltFactory)
            pVPStub = pVPStub->pDelegateVPStub;
        else
            return NULL;
    }

    return pVPStub->pfnVBltFactory(pPlane, pVPStub, pPool);
}

//////////////////////////////////////////////////////////////////////////
// CD3D9VideoMixerPlane
CD3D9VideoMixerPlane::CD3D9VideoMixerPlane(const PlaneInit &InitOpt, CD3D9VideoMixerBase *pMixer)
: m_pMixer(pMixer), m_cRef(0), m_InitOpt(InitOpt)
{
    m_pMixer->AddRef();
    m_bExternalSurface = (m_InitOpt.dwFlags & PLANE_INIT_EXTERNAL_SURFACE) != 0;
}

CD3D9VideoMixerPlane::~CD3D9VideoMixerPlane()
{
    m_pMixer->_DestroyPlane(m_InitOpt.PlaneID);
    SAFE_RELEASE(m_pMixer);
}

STDMETHODIMP CD3D9VideoMixerPlane::QueryInterface(REFIID riid, void **ppv)
{
    HRESULT hr = E_NOINTERFACE;

    if (riid == __uuidof(IUnknown))
    {
        hr = GetInterface((IUnknown *)this, ppv);
    }
    else if (riid == __uuidof(IDispSvrVideoMixerPlane))
    {
        hr = GetInterface((IDispSvrVideoMixerPlane *)this, ppv);
    }
    else if (riid == __uuidof(IDispSvrVideoMixerVideoPlane))
    {
        if (m_InitOpt.PlaneID == PLANE_MAINVIDEO || m_InitOpt.PlaneID == PLANE_SUBVIDEO)
            hr = GetInterface((IDispSvrVideoMixerVideoPlane *)this, ppv);
    }
    return hr;
}

STDMETHODIMP_(ULONG) CD3D9VideoMixerPlane::AddRef()
{
    LONG lRef = InterlockedIncrement(&m_cRef);
    ASSERT(lRef > 0);
    return lRef;
}

STDMETHODIMP_(ULONG) CD3D9VideoMixerPlane::Release()
{
    LONG lRef = InterlockedDecrement(&m_cRef);
    ASSERT(lRef >= 0);

    if (lRef == 0)
        delete this;
    return lRef;
}

STDMETHODIMP CD3D9VideoMixerPlane::GetPlaneDesc(DispSvr::PlaneDesc *pDesc)
{
    return m_pMixer->_GetPlaneDesc(m_InitOpt.PlaneID, pDesc);
}

STDMETHODIMP CD3D9VideoMixerPlane::SetExternalSurface(const ExternalSurfaceDesc *pEx)
{
    if (m_bExternalSurface)
    {
        if (pEx && pEx->bQueryStatusOnly)
            return m_pMixer->_QueryStatusOfExternalSurface(m_InitOpt.PlaneID, pEx);
        else
            return m_pMixer->_SetExternalSurface(m_InitOpt.PlaneID, pEx);
    }
    else
        return E_INVALIDARG;
}

STDMETHODIMP CD3D9VideoMixerPlane::UpdateFromBuffer(UpdateBuffer *pBuffer)
{
    if (m_bExternalSurface)
        return E_INVALIDARG;
    else
        return m_pMixer->_UpdateFromBuffer(m_InitOpt.PlaneID, pBuffer);
}

STDMETHODIMP CD3D9VideoMixerPlane::LockBuffer(LockedRect *pOut, const RECT *rcSrc, DWORD dwFlags)
{
    if (m_bExternalSurface)
        return E_INVALIDARG;
    else
        return m_pMixer->_LockBuffer(m_InitOpt.PlaneID, pOut, rcSrc, dwFlags);
}

STDMETHODIMP CD3D9VideoMixerPlane::UnlockBuffer()
{
    if (m_bExternalSurface)
        return E_INVALIDARG;
    else
        return m_pMixer->_UnlockBuffer(m_InitOpt.PlaneID);
}

STDMETHODIMP CD3D9VideoMixerPlane::SetPlanarAlpha(float fAlpha)
{
    return m_pMixer->_SetPlanarAlpha(m_InitOpt.PlaneID, fAlpha);
}

STDMETHODIMP CD3D9VideoMixerPlane::GetPlanarAlpha(float *pfAlpha)
{
    return m_pMixer->_GetPlanarAlpha(m_InitOpt.PlaneID, pfAlpha);
}

STDMETHODIMP CD3D9VideoMixerPlane::SetPosition(const NORMALIZEDRECT *rcDst, const RECT *rcSrc, const NORMALIZEDRECT *rcCrop, float fAspectRatio)
{
    return m_pMixer->_SetPosition(m_InitOpt.PlaneID, rcDst, rcSrc, rcCrop, fAspectRatio);
}

STDMETHODIMP CD3D9VideoMixerPlane::GetPosition(NORMALIZEDRECT *rcDst, RECT *rcSrc, NORMALIZEDRECT *rcCrop, float *pfAspectRatio)
{
    return m_pMixer->_GetPosition(m_InitOpt.PlaneID, rcDst, rcSrc, rcCrop, pfAspectRatio);
}

STDMETHODIMP CD3D9VideoMixerPlane::SetDirtyRect(const RECT *rcDirty)
{
    return m_pMixer->_SetDirtyRect(m_InitOpt.PlaneID, rcDirty);
}

STDMETHODIMP CD3D9VideoMixerPlane::SetVideoProperty(const VideoProperty *pProperty)
{
    return m_pMixer->_SetVideoProperty(m_InitOpt.PlaneID, pProperty);
}

STDMETHODIMP CD3D9VideoMixerPlane::GetVideoProperty(VideoProperty *pProperty)
{
    return m_pMixer->_GetVideoProperty(m_InitOpt.PlaneID, pProperty);
}

STDMETHODIMP CD3D9VideoMixerPlane::SetColorControl(const ColorControl *pCC)
{
    return E_NOTIMPL;
}

STDMETHODIMP CD3D9VideoMixerPlane::GetColorControl(ColorControl *pCC)
{
    return E_NOTIMPL;
}

//////////////////////////////////////////////////////////////////////////
// CD3D9VideoMixerBase
CD3D9VideoMixerBase::CD3D9VideoMixerBase()
{
    m_GUID = DISPSVR_RESOURCE_VIDEOMIXER;
    m_colorBackGround = 0;
    ZeroMemory(m_Planes, sizeof(m_Planes));
    ZeroMemory(m_PlaneConfigs, sizeof(m_PlaneConfigs));
    ZeroMemory(&m_Property, sizeof(m_Property));
    ZeroMemory(&m_LumaKey, sizeof(m_LumaKey));
    ZeroMemory(&m_rcMixingDstClip, sizeof(m_rcMixingDstClip));
    ZeroMemory(&m_rcMixingDst, sizeof(m_rcMixingDst));
    m_pTexturePool = new CD3D9TexturePool;		ASSERT(m_pTexturePool);
    CResourceManager::GetInstance()->GetInterface(DISPSVR_RESOURCE_VIDEOEFFECT3DMANAGER, __uuidof(ID3D9VideoEffect3DProcessor), (void **)&m_pVideoEffect3DBlt);
    LoadDefaultMixerCoordinate(m_nrcDst);
    ASSERT(m_pTexturePool);
    m_hSurfaceReadyEvent = ::CreateEvent(NULL, TRUE, TRUE, NULL);
    m_uClearNonVideoAreaListCount = 0;
    m_lLockingCount = 0;
    ZeroMemory(&m_MixerCaps, sizeof(MixerCaps));
    m_MixerCaps.uMaxPlaneNumber = PLANE_MAX;
    // MIXER_CAP_CAN_CHANGE_DESTINATION does not make sense for video mixer because of security concern.
    m_MixerCaps.dwFlags = MIXER_CAP_3D_RENDERTARGET | MIXER_CAP_CAN_VIRTUALIZE_FROM_ORIGIN;
    m_fWindowAspectRatio = 1.0f;
    m_bRecalculateVideoPosition = true;

    m_pXVEManager = NULL;
    CComPtr<IDispSvrXVEManagerWrapper> pXVEManagerWrapper;
    CResourceManager::GetInstance()->GetInterface(DISPSVR_RESOURCE_XVIDEOEFFECTMANAGER, __uuidof(IDispSvrXVEManagerWrapper), (void **)&pXVEManagerWrapper);
    if (pXVEManagerWrapper)
    {
        pXVEManagerWrapper->GetXVEManager(&m_pXVEManager);
    }
}

CD3D9VideoMixerBase::~CD3D9VideoMixerBase()
{
    ::CloseHandle(m_hSurfaceReadyEvent);
    _ReleaseDevice();
    SAFE_DELETE(m_pTexturePool);
    SAFE_RELEASE(m_pVideoEffect3DBlt);
    SAFE_RELEASE(m_pXVEManager);
}

STDMETHODIMP CD3D9VideoMixerBase::QueryInterface(REFIID riid, void **ppv)
{
    if (riid == __uuidof(IDispSvrVideoMixer))
    {
        return GetInterface((IDispSvrVideoMixer *)this, ppv);
    }
    else if (riid == __uuidof(IDispSvrVideoProcessor))
    {
        return GetInterface((IDispSvrVideoProcessor *) this, ppv);
    }
    else if (riid == __uuidof(IDispSvrVideoEffect3DManager))
    {
        // delegate to a global effect manager.
        return CResourceManager::GetInstance()->GetInterface(DISPSVR_RESOURCE_VIDEOEFFECT3DMANAGER, riid, ppv);
    }
    else if (riid == __uuidof(IDispSvrVideoEffectManager))
    {
        return GetInterface((IDispSvrVideoEffectManager *) this, ppv);
    }
    return CD3D9PluginBase::QueryInterface(riid, ppv);
}

STDMETHODIMP CD3D9VideoMixerBase::ProcessMessage(RESOURCE_MESSAGE_TYPE eMessage, LPVOID ulParam)
{
    HRESULT hr = E_NOTIMPL;

    switch (eMessage)
    {
    case RESOURCE_MESSAGE_SETDEVICE:
    case RESOURCE_MESSAGE_RELEASEDEVICE:
    case RESOURCE_MESSAGE_SETWINDOWHANDLE:
    case RESOURCE_MESSAGE_MOVEWINDOW:
        {
            CAutoLock selfLock(&m_csObj);
            m_bRecalculateVideoPosition = true;
        }
        hr = CD3D9PluginBase::ProcessMessage(eMessage, ulParam);
        break;

    case RESOURCE_MESSAGE_EVICTRESOURCES:
        {
            BOOL bEvictResource = *(reinterpret_cast<BOOL *> (ulParam));
            CAutoLock selfLock(&m_csObj);
            if (bEvictResource)
                hr = m_pTexturePool->EvictResources();
            else
                hr = m_pTexturePool->RestoreResources();
        }
        break;

    default:
        hr = CD3D9PluginBase::ProcessMessage(eMessage, ulParam);
        break;
    }
    return hr;
}

STDMETHODIMP CD3D9VideoMixerBase::_SetDevice(IUnknown *pDevice)
{
    HRESULT hr;

    CAutoLock selfLock(&m_csObj);
    hr = CD3D9PluginBase::_SetDevice(pDevice);
    if (FAILED(hr))
        return hr;

    ZeroMemory(m_PlaneConfigs, sizeof(m_PlaneConfigs));

    hr = m_pTexturePool->SetPoolUsage(TEXTURE_POOL_USAGE_D3D9);
    hr = m_pTexturePool->SetDevice(m_pDevice);

    VideoProcessorStub vp = {0};
    vp.guidVP = DispSvr_VideoProcStretchRect;
    vp.RenderTargetFormat = D3DFMT_X8R8G8B8;
    vp.sCaps.eType = PROCESSOR_TYPE_HARDWARE;
    vp.pfnVBltFactory = CD3D9VBlt::Create;
    m_VideoProcessorList.push_back(vp);
    SelectVideoProcessor();

    DWORD dwBBWidth = m_rcMonitor.right - m_rcMonitor.left, dwBBHeight = m_rcMonitor.bottom - m_rcMonitor.top;
    CRegistryService::GetInstance()->Get(REG_BACKBUFFER_WIDTH, &dwBBWidth);
    CRegistryService::GetInstance()->Get(REG_BACKBUFFER_HEIGHT, &dwBBHeight);
    SetRect(&m_rcMixingDstClip, 0, 0, dwBBWidth, dwBBHeight);
    return hr;
}

STDMETHODIMP CD3D9VideoMixerBase::_ReleaseDevice()
{
    CAutoLock selfLock(&m_csObj);
    HRESULT hr = m_pTexturePool->ReleaseDevice();
    m_ClearRectList.clear();
    m_uClearNonVideoAreaListCount = 0;
    m_VideoProcessorList.clear();
    return CD3D9PluginBase::_ReleaseDevice();
}

STDMETHODIMP CD3D9VideoMixerBase::SetDestination(IUnknown *pDestSurface, const NORMALIZEDRECT *pDestRect)
{
    CAutoLock selfLock(&m_csObj);
    NORMALIZEDRECT nrcDst;
    if (pDestRect)
        nrcDst = *pDestRect;
    else
        LoadDefaultMixerCoordinate(nrcDst);

    if (nrcDst != m_nrcDst)
    {
        m_nrcDst = nrcDst;
        // some application my need to know the destination rectangle or surface change
        // in order to use the mixer correctly.
        NotifyListeners(EVENT_VIDEO_MIXING_CHANGE_DESTINATION, 0, reinterpret_cast<DWORD> (&nrcDst));
    }

    m_bRecalculateVideoPosition = true;
    return S_OK;
}

STDMETHODIMP CD3D9VideoMixerBase::Execute()
{
    HRESULT hr = S_OK;

    // m_csExecuting serves as a barrier to prevent future lock-surface action until it releases mutex.
    CAutoLock lockExecuting(&m_csExecuting);

    // wait until all surfaces are ready to be used (no one locks any mixer surfaces).
    WaitForSingleObject(m_hSurfaceReadyEvent, INFINITE);

    NotifyListeners(EVENT_VIDEO_MIXING_BEGIN, 0, 0);
    {
        CAutoLock lock(&m_csObj);
        if (m_bRecalculateVideoPosition)
            hr = _OnMoveWindow();

        if (SUCCEEDED(hr))
        {
            CComPtr<IDirect3DSurface9> pDestSurface;

            if (MIXER_CAP_3D_RENDERTARGET & m_MixerCaps.dwFlags)
            {
                ClearNonVideoArea();
            }

            // Still pass down the current render target surface even if some mixer does not use it.
            hr = m_pDevice->GetRenderTarget(0, &pDestSurface);
            hr = _Execute(pDestSurface, m_rcMixingDst, m_rcMixingDstClip);
        }
    }
    NotifyListeners(EVENT_VIDEO_MIXING_END, 0, 0);

    return hr;
}

STDMETHODIMP CD3D9VideoMixerBase::QueryCaps(MixerCaps *pCaps)
{
    CHECK_POINTER(pCaps);
    memcpy(pCaps, &m_MixerCaps, sizeof(MixerCaps));
    return S_OK;
}

STDMETHODIMP CD3D9VideoMixerBase::CreatePlane(PlaneInit *pInit, REFIID riid, void **ppPlane)
{
    HRESULT hr = _CreatePlane(pInit);
    if (SUCCEEDED(hr))
    {
        CD3D9VideoMixerPlane *pPlane = new CD3D9VideoMixerPlane(*pInit, this);
        if (pPlane == NULL)
        {
            _DestroyPlane(pInit->PlaneID);
            return E_OUTOFMEMORY;
        }

        hr = pPlane->QueryInterface(riid, ppPlane);
    }
    return hr;
}

STDMETHODIMP CD3D9VideoMixerBase::SetClearRectangles(UINT uCount, ClearRect *pRects)
{
    CAutoLock selfLock(&m_csObj);
    m_ClearRectList.clear();
    if (uCount > 0)
    {
        const ClearRect *p = pRects;
        for (UINT i = 0; i < uCount; i++)
            m_ClearRectList.push_back(*p++);
    }
    return S_OK;
}

STDMETHODIMP CD3D9VideoMixerBase::SetBackgroundColor(COLORREF Color)
{
    m_colorBackGround = Color;
    return S_OK;
}

STDMETHODIMP CD3D9VideoMixerBase::GetBackgroundColor(COLORREF *pColor)
{
    CHECK_POINTER(pColor);
    *pColor = m_colorBackGround;
    return S_OK;
}

STDMETHODIMP CD3D9VideoMixerBase::SetLumaKey(const LumaKey *pLumaKey)
{
    CHECK_POINTER(pLumaKey);
    m_LumaKey = *pLumaKey;
    return S_OK;
}

STDMETHODIMP CD3D9VideoMixerBase::GetLumaKey(LumaKey *pLumaKey)
{
    CHECK_POINTER(pLumaKey);
    *pLumaKey = m_LumaKey;
    return S_OK;
}

STDMETHODIMP CD3D9VideoMixerBase::SetProperty(const MixerProperty *pProperty)
{
    CHECK_POINTER(pProperty);
    if ((MIXER_PROPERTY_VIRTUALIZE_FROM_ORIGIN & pProperty->dwFlags) != 0
        && (MIXER_CAP_CAN_VIRTUALIZE_FROM_ORIGIN & m_MixerCaps.dwFlags) == 0)
        return E_INVALIDARG;
    memcpy(&m_Property, pProperty, sizeof(MixerProperty));
    return S_OK;
}

STDMETHODIMP CD3D9VideoMixerBase::GetProperty(MixerProperty *pProperty)
{
    CHECK_POINTER(pProperty);
    memcpy(pProperty, &m_Property, sizeof(MixerProperty));
    return S_OK;
}

STDMETHODIMP CD3D9VideoMixerBase::GetVideoEffectManager(IUnknown **ppManager)
{
    HRESULT hr = E_FAIL;
    CComPtr<IDispSvrXVEManagerWrapper> pXVEManagerWrapper;
    hr = CResourceManager::GetInstance()->GetInterface(DISPSVR_RESOURCE_XVIDEOEFFECTMANAGER,
        __uuidof(IDispSvrXVEManagerWrapper), (VOID **)&pXVEManagerWrapper);

    if (SUCCEEDED(hr) && pXVEManagerWrapper)
    {
        CComPtr<IXVideoEffectManager> pManager;
        hr = pXVEManagerWrapper->GetXVEManager(&pManager);
        if (SUCCEEDED(hr) && pManager)
        {
            hr = pManager->QueryInterface(IID_IUnknown, (VOID **)ppManager);
        }
    }
    return hr;
}

STDMETHODIMP CD3D9VideoMixerBase::SetVideoEffectManager(IUnknown *pManager)
{
    CHECK_POINTER(pManager);

    HRESULT hr = E_FAIL;

    if (CResourceManager::GetInstance())
    {
        CComPtr<IXVideoEffectManager> pXVEManager;
        hr = pManager->QueryInterface(__uuidof(IXVideoEffectManager), (VOID **)&pXVEManager);
        if (SUCCEEDED(hr) && pXVEManager)
        {
            CComPtr<IDispSvrXVEManagerWrapper> pIXVEManagerWrapper;
            hr = CResourceManager::GetInstance()->GetInterface(DISPSVR_RESOURCE_XVIDEOEFFECTMANAGER,
                __uuidof(IDispSvrXVEManagerWrapper), (VOID **)&pIXVEManagerWrapper);

            if (SUCCEEDED(hr) && pIXVEManagerWrapper)
            {
                CComPtr<IXVideoEffectManager> pManager;
                hr = pIXVEManagerWrapper->SetXVEManager(pManager);
            }
            else
            {
                CDispSvrXVEManagerWrapper *pCXVEManagerWrapper = new CDispSvrXVEManagerWrapper();
                pCXVEManagerWrapper->SetXVEManager(pXVEManager);
                CResourceManager::GetInstance()->Install(pCXVEManagerWrapper);
                pIXVEManagerWrapper = pCXVEManagerWrapper;
            }
            if (m_pXVEManager)
            {
                m_pXVEManager->Release();
            }

            m_pXVEManager = pXVEManager.Detach();
            // NO need to send XVE_MESSAGE_INITIALIZE message since wrapper will do it when SetXVEManager called.
            CComQIPtr<IDispSvrPlugin> pWrapperPlugIn = pIXVEManagerWrapper;
            if (pWrapperPlugIn)
            {
                pWrapperPlugIn->ProcessMessage(RESOURCE_MESSAGE_SETDEVICE, (LPVOID)m_pDevice);
            }

            hr = S_OK;
        }
    }
    else
        hr = E_UNEXPECTED;

    return hr;
}


STDMETHODIMP CD3D9VideoMixerBase::QueryPlaneFormatCount(PLANE_ID PlaneID, UINT *pCount)
{
    CHECK_POINTER(pCount);
    if (m_pTexturePool)
        return m_pTexturePool->QuerySupportedFormatCount(pCount);
    return E_FAIL;
}

STDMETHODIMP CD3D9VideoMixerBase::QueryPlaneFormat(PLANE_ID PlaneID, PLANE_FORMAT *pFormats)
{
    CHECK_POINTER(pFormats);
    if (m_pTexturePool)
        return m_pTexturePool->QuerySupportedFormats(pFormats);
    return E_FAIL;
}

STDMETHODIMP CD3D9VideoMixerBase::QueryPlaneCaps(PLANE_ID PlaneID, PLANE_FORMAT Format, PlaneCaps *pCap)
{
    CHECK_POINTER(pCap);
    if (!m_pTexturePool)
        return E_FAIL;

    TextureCap cap;

    // First check if texturepool supports the format.
    HRESULT hr = m_pTexturePool->QueryFormatCaps(Format, &cap);
    if (FAILED(hr))
        return hr;

    ZeroMemory(pCap, sizeof(PlaneCaps));
    pCap->dwFlags = PLANE_CAP_MAX_SIZE;
    pCap->uMaxWidth = cap.uMaxWidth;
    pCap->uMaxHeight = cap.uMaxHeight;

    // Then check if mixer supports the format.
    return _QueryPlaneCaps(PlaneID, Format, pCap);
}

STDMETHODIMP CD3D9VideoMixerBase::_CreatePlane(PlaneInit *pInit)
{
    HRESULT hr = E_FAIL;

    CHECK_POINTER(pInit);
    if (pInit->PlaneID < 0 || pInit->PlaneID >= PLANE_MAX)
        return E_INVALIDARG;

    CAutoLock selfLock(&m_csObj);
    D3D9Plane &plane = m_Planes[pInit->PlaneID];
    D3D9PlaneConfig &cfg = m_PlaneConfigs[pInit->PlaneID];

    if (plane.bCreated)
    {
        ASSERT(0 && "CD3D9VideoMixerBase::_CreatePlane: plane is already created!");
        return E_FAIL;
    }

    LoadDefaultD3D9Plane(plane);

    PlaneCaps caps;
    hr = QueryPlaneCaps(pInit->PlaneID, pInit->Format, &caps);
    if (FAILED(hr))
    {
        ASSERT(0 && "CD3D9VideoMixerBase::_CreatePlane: plane format is not supported!");
        return hr;
    }

    // if size exceeds max cap, we treat it as an error.
    if (PLANE_CAP_MAX_SIZE & caps.dwFlags)
        if (pInit->uWidth > caps.uMaxWidth || pInit->uHeight > caps.uMaxHeight)
            return E_INVALIDARG;

    if (pInit->pListener)
    {
        // we don't add reference to listener, users should be responsible for the life time of the listener.
        if (SUCCEEDED(pInit->pListener->QueryInterface(__uuidof(IDispSvrVideoMixerEventListener), (void **)&plane.pListener)))
            plane.pListener->Release();
    }

    if (pInit->dwFlags & PLANE_INIT_EXTERNAL_SURFACE)
    {
        plane.bExternal = true;
        plane.rcSrc.right = pInit->uWidth;
        plane.rcSrc.bottom = pInit->uHeight;
        hr = S_OK;
    }
    else
    {
        if (PLANE_CAP_MIN_SIZE & caps.dwFlags)
        {
            if (pInit->uWidth < caps.uMinWidth)
                pInit->uWidth = caps.uMinWidth;
            if (pInit->uHeight < caps.uMinHeight)
                pInit->uHeight = caps.uMinHeight;
        }

        CreateTextureParam sParam = {0};

        sParam.uWidth = pInit->uWidth;
        sParam.uHeight = pInit->uHeight;
        sParam.Format = pInit->Format;
        memcpy(sParam.Palette, pInit->Palette, sizeof(sParam.Palette));
        // if we need to post processing on a lockable surface, we create a backing store on system memory.
        sParam.eUsage = plane.pPostTextureFilter ? TEXTURE_USAGE_LOCKABLE_BACKSTORE : TEXTURE_USAGE_LOCKABLE;
        hr = m_pTexturePool->CreateTexture(&sParam, &plane.hTexture);
        if (SUCCEEDED(hr))
        {
            LockedRect lockedRect;
            if (pInit->Format == PLANE_FORMAT_ARGB || pInit->Format == PLANE_FORMAT_P8)
            {
                // Clear texture while first initialized, because the later updates only updates the dirty region.
                RECT rect = {0, 0, pInit->uWidth, pInit->uHeight};
                hr = m_pTexturePool->LockBuffer(plane.hTexture, &lockedRect, &rect, 0);	ASSERT(SUCCEEDED(hr));
                memset(lockedRect.pBuffer, 0, lockedRect.uPitch*pInit->uHeight);
            }
            else 
            {
                hr = m_pTexturePool->LockBuffer(plane.hTexture, &lockedRect, NULL, PLANE_LOCK_READONLY);	ASSERT(SUCCEEDED(hr));
            }
            hr = m_pTexturePool->UnlockBuffer(plane.hTexture);	ASSERT(SUCCEEDED(hr));
            plane.uPitch = lockedRect.uPitch;
            plane.rcSrc.right = sParam.uWidth;
            plane.rcSrc.bottom = sParam.uHeight;
            plane.bHasBackingStore = (sParam.eUsage & TEXTURE_USAGE_BACKSTORE) != 0;
            plane.bHDVideo = IsHDVideo(sParam.uWidth, sParam.uHeight);

            // save current sample into samples lists
            VideoReferenceSample Sample;
            ZeroMemory(&Sample, sizeof(VideoReferenceSample));
            Sample.hTexture = plane.hTexture;
            plane.VideoSamples.push_front(Sample);
            plane.uiPlaneSampleIndex = 0;
        }
    }

    if (SUCCEEDED(hr))
    {
        if (PLANE_MAINVIDEO == pInit->PlaneID)
            m_bRecalculateVideoPosition = true;

        if (pInit->dwFlags & PLANE_INIT_PARTIAL_BLENDING)
        {
            plane.bPartialBlending = true;
#ifdef _EXPERIMENTING_
            if (pInit->Format == PLANE_FORMAT_ARGB && (caps.dwFlags & PLANE_CAP_HW_PARTIAL_BLENDING) == 0)
                plane.pPostTextureFilter = new CSWInverseAlphaFilter(&plane);
#endif
        }

        if (pInit->dwFlags & PLANE_INIT_FULLSCREEN_MIXING)
        {
            plane.bFullScreenMixing = true;
        }

        plane.bCreated = true;
        plane.uWidth = pInit->uWidth;
        plane.uHeight = pInit->uHeight;
        plane.Format = pInit->Format;
        plane.dwLastUpdateTS++;

        if (pInit->Format == PLANE_FORMAT_P8)
        {
            plane.bPalettized = true;
            memcpy(plane.Palette, pInit->Palette, sizeof(pInit->Palette));
        }

        // pVBlt is used as hardware deinterlacer or color space converter.
        if (cfg.pVideoProcessorStub)
        {
            ASSERT(plane.pVBlt == 0);
            plane.pVBlt = CreateVBltFromVPStub(&plane, cfg.pVideoProcessorStub, m_pTexturePool);
        }
    }

    return hr;
}

STDMETHODIMP CD3D9VideoMixerBase::_DestroyPlane(PLANE_ID PlaneID)
{
    CAutoLock selfLock(&m_csObj);
    D3D9Plane &plane = m_Planes[PlaneID];
    HRESULT hr = E_FAIL;

    if (!plane.bCreated)
        return E_INVALIDARG;

    ReleaseVideoSamples(PlaneID);

    SAFE_DELETE(plane.pPostTextureFilter);
    SAFE_DELETE(plane.pVBlt);
    plane.bValid = false;
    plane.bCreated = false;
    return S_OK;
}

STDMETHODIMP CD3D9VideoMixerBase::_GetPlaneDesc(PLANE_ID PlaneID, PlaneDesc *pDesc)
{
    CHECK_POINTER(pDesc);
    CAutoLock selfLock(&m_csObj);
    D3D9Plane &plane = m_Planes[PlaneID];

    if (plane.bCreated)
    {
        pDesc->uWidth = plane.uWidth;
        pDesc->uHeight = plane.uHeight;
        pDesc->uPitch = plane.uPitch;
        pDesc->Format = plane.Format;
        return S_OK;
    }
    return E_INVALIDARG;
}

STDMETHODIMP CD3D9VideoMixerBase::_SetExternalSurface(PLANE_ID PlaneID, const ExternalSurfaceDesc *pEx)
{
    CAutoLock selfLock(&m_csObj);
    D3D9Plane &plane = m_Planes[PlaneID];
    D3D9PlaneConfig &cfg = m_PlaneConfigs[PlaneID];
    UINT uNumBackwardSamples = cfg.pVideoProcessorStub ? cfg.pVideoProcessorStub->sCaps.uNumBackwardSamples : 0;
    // force forward ref samples = 0 until SetVideoProperty() interface re-defined properly.
    // for interlaced content, the target frame is decided by VideoProperty.dwFieldSelect
    // the first field will never be shown if SetVideoProperty() takes effect on future frame.
    UINT uNumForwardSamples = 0; 
    HRESULT hr = S_OK;

    ASSERT(plane.bCreated);

    // if pEx == NULL, it is to unregister the external surface.
    if (pEx == NULL)
    {
        ReleaseVideoSamples(PlaneID);
    }
    else
    {
        // check the previous registered surface.
        if (plane.VideoProperty.Format == VIDEO_FORMAT_PROGRESSIVE)
            uNumBackwardSamples = 0;	// progressive samples do not require references.

        // if width/height/format changes, we should discard all samples.
        if (plane.hTexture && (plane.uWidth != pEx->uWidth || plane.uHeight != pEx->uHeight || plane.Format != pEx->Format))
            ReleaseVideoSamples(PlaneID);

        // Check if there has same surface in sample lists
        for (VideoSampleList::const_iterator it = plane.VideoSamples.begin();
            it != plane.VideoSamples.end(); ++it)
        {
            CComPtr<IUnknown> pObj;
            hr = m_pTexturePool->GetRepresentation(it->hTexture, __uuidof(IUnknown), (void **) &pObj);
            if (SUCCEEDED(hr) && pObj == pEx->pSurface)
            {
                ASSERT(plane.uWidth == pEx->uWidth && plane.uHeight == pEx->uHeight && plane.Format == pEx->Format);
                return hr;
            }
        }

        // only reset rcSrc when input surface size is changed.
        if (plane.uWidth != pEx->uWidth || plane.uHeight != pEx->uHeight)
            SetRect(&plane.rcSrc, 0, 0, pEx->uWidth, pEx->uHeight);

        plane.uWidth = pEx->uWidth;
        plane.uHeight = pEx->uHeight;
        plane.Format = pEx->Format;
        plane.dwLastUpdateTS++;
        plane.bHDVideo = IsHDVideo(pEx->uWidth, pEx->uHeight);
        HANDLE hTexture;

        // register a surface with the texture pool.
        hr = m_pTexturePool->CreateTexture(pEx->pSurface, &hTexture);
        if (SUCCEEDED(hr))
        {
            // save current sample into samples lists
            VideoReferenceSample Sample;
            ZeroMemory(&Sample, sizeof(VideoReferenceSample));
            Sample.VideoProperty = plane.VideoSamples.size() ? plane.VideoSamples.front().VideoProperty : plane.VideoProperty;
            Sample.hTexture = hTexture;
            plane.VideoSamples.push_front(Sample);

            // discard excessive samples in queue.
            while (plane.VideoSamples.size() > uNumBackwardSamples+uNumForwardSamples+1)
            {
                m_pTexturePool->ReleaseTexture(plane.VideoSamples.back().hTexture);
                plane.VideoSamples.pop_back();
            }

            // setting plane sample
            plane.uiPlaneSampleIndex = uNumForwardSamples ? min(uNumForwardSamples, plane.VideoSamples.size()-1): 0;
            plane.hTexture = plane.VideoSamples[plane.uiPlaneSampleIndex].hTexture;
        }
    }
    plane.bValid = !!plane.hTexture;
    return hr;
}

STDMETHODIMP CD3D9VideoMixerBase::_QueryStatusOfExternalSurface(PLANE_ID PlaneID, const ExternalSurfaceDesc *pEx)
{
    if (!pEx)
        return E_INVALIDARG;

    CAutoLock selfLock(&m_csObj);
    D3D9Plane &plane = m_Planes[PlaneID];
    HRESULT hr = S_OK;
    CComPtr<IUnknown> pObj;

    for (VideoSampleList::const_iterator it = plane.VideoSamples.begin();
        it != plane.VideoSamples.end();
        ++it)
    {
        pObj.Release();
        hr = m_pTexturePool->GetRepresentation(it->hTexture, __uuidof(IUnknown), (void **) &pObj);
        if (SUCCEEDED(hr) && pObj == pEx->pSurface)
            return DDERR_WASSTILLDRAWING;
    }
    return S_OK;
}

STDMETHODIMP CD3D9VideoMixerBase::_UpdateFromBuffer(PLANE_ID PlaneID, UpdateBuffer *pBuffer)
{
    return E_NOTIMPL;
}

STDMETHODIMP CD3D9VideoMixerBase::_LockBuffer(PLANE_ID PlaneID, LockedRect *pOut, const RECT *rcSrc, DWORD dwFlags)
{
    // When executing, the event is reset and all pending lock request should be blocked until execution finishes.
    CAutoLock lockExecuting(&m_csExecuting);

    CAutoLock selfLock(&m_csObj);
    const D3D9Plane &plane = m_Planes[PlaneID];

    HRESULT hr = m_pTexturePool->LockBuffer(plane.hTexture, pOut, rcSrc, dwFlags);
    if (SUCCEEDED(hr))
    {
        if (!plane.bHasBackingStore)
        {
            ASSERT(m_lLockingCount >= 0);
            m_lLockingCount++;
            ResetEvent(m_hSurfaceReadyEvent);
        }
    }
    return hr;
}

STDMETHODIMP CD3D9VideoMixerBase::_UnlockBuffer(PLANE_ID PlaneID)
{
    CAutoLock selfLock(&m_csObj);
    D3D9Plane &plane = m_Planes[PlaneID];

    HRESULT hr = m_pTexturePool->UnlockBuffer(plane.hTexture, plane.pPostTextureFilter);
    if (SUCCEEDED(hr))
    {
        if (S_OK == hr)
        {
            plane.bValid = true;
            plane.dwLastUpdateTS++;
        }
        else
        {
            plane.bValid = false;
        }

        if (!plane.bHasBackingStore)
        {
            m_lLockingCount--;
            ASSERT(m_lLockingCount >= 0);
            if (m_lLockingCount <= 0)
                SetEvent(m_hSurfaceReadyEvent);
        }
    }
    return hr;
}

STDMETHODIMP CD3D9VideoMixerBase::_SetPlanarAlpha(PLANE_ID PlaneID, float fAlpha)
{
    if (fAlpha < 0.0f || fAlpha > 1.0f)
        return E_INVALIDARG;

    CAutoLock selfLock(&m_csObj);
    m_Planes[PlaneID].fAlpha = fAlpha;
    return S_OK;
}

STDMETHODIMP CD3D9VideoMixerBase::_GetPlanarAlpha(PLANE_ID PlaneID, float *pfAlpha)
{
    CHECK_POINTER(pfAlpha);
    CAutoLock selfLock(&m_csObj);
    *pfAlpha = m_Planes[PlaneID].fAlpha;
    return S_OK;
}

STDMETHODIMP CD3D9VideoMixerBase::_SetPosition(PLANE_ID PlaneID, const NORMALIZEDRECT *lprcDst, const RECT *lprcSrc, const NORMALIZEDRECT *lprcCrop, float fAspectRatio)
{
    if (lprcDst == 0 || lprcSrc == 0 || lprcCrop == 0
        || lprcCrop->right <= lprcCrop->left || lprcCrop->bottom <= lprcCrop->top
        || lprcDst->right <= lprcDst->left || lprcDst->bottom <= lprcDst->top)
        return E_INVALIDARG;

    CAutoLock selfLock(&m_csObj);
    D3D9Plane &plane = m_Planes[PlaneID];

    plane.nrcDst = *lprcDst;
    plane.nrcCrop = *lprcCrop;
    if (IsRectEmpty(lprcSrc))
        SetRect(&plane.rcSrc, 0, 0, plane.uWidth, plane.uHeight);
    else
        plane.rcSrc = *lprcSrc;
    plane.fAspectRatio = fAspectRatio;
    if (PLANE_MAINVIDEO == PlaneID)
        m_bRecalculateVideoPosition = true;
    return S_OK;
}

STDMETHODIMP CD3D9VideoMixerBase::_GetPosition(PLANE_ID PlaneID, NORMALIZEDRECT *rcDst, RECT *rcSrc, NORMALIZEDRECT *rcCrop, float *pfAspectRatio)
{
    CAutoLock selfLock(&m_csObj);
    D3D9Plane &plane = m_Planes[PlaneID];

    if (rcDst)
        *rcDst = plane.nrcDst;
    if (rcCrop)
        *rcCrop = plane.nrcCrop;
    if (rcSrc)
        *rcSrc = plane.rcSrc;
    if (pfAspectRatio)
        *pfAspectRatio = plane.fAspectRatio;
    return S_OK;
}

STDMETHODIMP CD3D9VideoMixerBase::_SetDirtyRect(PLANE_ID PlaneID, const RECT *rcDirty)
{
    CHECK_POINTER(rcDirty);
    CAutoLock selfLock(&m_csObj);
    D3D9Plane &plane = m_Planes[PlaneID];

    if (rcDirty)
        plane.rcDirty = *rcDirty;
    else
        ZeroMemory(&plane.nrcDst, sizeof(plane.nrcDst));
    return S_OK;
}

STDMETHODIMP CD3D9VideoMixerBase::_SetVideoProperty(PLANE_ID PlaneID, const VideoProperty *pProperty)
{
    ASSERT(PlaneID == PLANE_MAINVIDEO || PlaneID == PLANE_SUBVIDEO);

    CHECK_POINTER(pProperty);
    CAutoLock selfLock(&m_csObj);
    D3D9Plane &plane = m_Planes[PlaneID];

    if (plane.VideoSamples.empty())
        return E_FAIL;

    BOOL bContinuous = FALSE;
    if (plane.VideoSamples.size() > 1)
    {
        int iPtsDiff = abs((int)(pProperty->rtStart - plane.VideoSamples.front().VideoProperty.rtEnd));
        // regard the two frame are continuous if the pts difference < 1 ms
        if (iPtsDiff < 10000)
        {
            bContinuous = TRUE;
        }
    }

    // check if samples are continuous; otherwise clear the reference samples.
    if (pProperty->bStillMenuHint || !bContinuous)
    {
        while (plane.VideoSamples.size() > 1)
        {
            m_pTexturePool->ReleaseTexture(plane.VideoSamples.back().hTexture);
            plane.VideoSamples.pop_back();
        }
        plane.uiPlaneSampleIndex = 0;
        plane.hTexture = plane.VideoSamples[plane.uiPlaneSampleIndex].hTexture;
    }

    memcpy(&plane.VideoSamples.front().VideoProperty, pProperty, sizeof(VideoProperty));
    memcpy(&plane.VideoProperty, &plane.VideoSamples[plane.uiPlaneSampleIndex].VideoProperty, sizeof(VideoProperty));
    plane.dwLastUpdateTS++;

    return S_OK;
}

STDMETHODIMP CD3D9VideoMixerBase::_GetVideoProperty(PLANE_ID PlaneID, VideoProperty *pProperty)
{
    ASSERT(PlaneID == PLANE_MAINVIDEO || PlaneID == PLANE_SUBVIDEO);

    CHECK_POINTER(pProperty);
    CAutoLock selfLock(&m_csObj);
    memcpy(pProperty, &m_Planes[PlaneID].VideoProperty, sizeof(VideoProperty));
    return S_OK;
}

STDMETHODIMP CD3D9VideoMixerBase::_QueryPlaneCaps(PLANE_ID PlaneID, PLANE_FORMAT Format, PlaneCaps *pCap)
{
    return S_OK;
}

STDMETHODIMP CD3D9VideoMixerBase::_OnMoveWindow()
{
    HRESULT hr;

    // clear and then calculate non video area list
    m_uClearNonVideoAreaListCount = 0;

    hr = GetClientRect(m_hwnd, &m_rcMixingDst);
    if (hr == 0)
        return E_FAIL;

    // window aspect ratio
    m_fWindowAspectRatio = float(m_rcMixingDst.right) * (m_nrcDst.right - m_nrcDst.left) / (m_rcMixingDst.bottom * (m_nrcDst.bottom - m_nrcDst.top));

    // Bug#81775
    // because of primary monitor change,
    // rcMonitor will change and need an update
    // when primary monitor change,
    // app should call OnMoveWindow() to update window info in Mixer
    HMONITOR hMonitor = MonitorFromWindow(m_hwnd, (m_hwnd) ? MONITOR_DEFAULTTONEAREST : MONITOR_DEFAULTTOPRIMARY);
    MONITORINFOEX MonInfo;
    ZeroMemory(&MonInfo, sizeof(MONITORINFOEX));
    MonInfo.cbSize = sizeof(MONITORINFOEX);
    if (GetMonitorInfo(hMonitor, &MonInfo) == TRUE)
    {
        m_rcMonitor = MonInfo.rcMonitor;
    }

    // when using virtualization, render target is supposed to be the same as desktop resolution.
    if (m_Property.dwFlags & MIXER_PROPERTY_VIRTUALIZATION)
    {
        POINT pt = {0};
        hr = ClientToScreen(m_hwnd, &pt);	ASSERT(hr != 0);
        if (m_Property.dwFlags & MIXER_PROPERTY_VIRTUALIZE_FROM_ORIGIN)
        {
            if (pt.x > m_rcMonitor.left)
                pt.x = m_rcMonitor.left;
            if (pt.y > m_rcMonitor.top)
                pt.y = m_rcMonitor.top;
        }
        OffsetRect(&m_rcMixingDst, pt.x - m_rcMonitor.left, pt.y - m_rcMonitor.top);
    }
    else
    {
        CopyRect(&m_rcMixingDst, &m_rcMixingDstClip);
    }

    // get window rectangle
    RECT rcWndRect = m_rcMixingDst;

    // now we have m_rcMixingDst as video mixer's mixing area.
    NRectToRect(m_rcMixingDst, m_nrcDst);

    // We are done calculation for mixing. (m_rcMixingDst and m_rcMixingDstClip are valid)
    m_bRecalculateVideoPosition = false;

    // check mixing area is valid
    if (IsRectEmpty(&m_rcMixingDst))
        return E_FAIL;

    if ((MIXER_PROPERTY_CLEARNONVIDEOAREA & m_Property.dwFlags) == 0
        || (MIXER_CAP_3D_RENDERTARGET & m_MixerCaps.dwFlags) == 0)
        return S_OK;

    // we will try to calculate video area if relying on IDirect3DDevice9->Clear().
    const D3D9Plane &plane = m_Planes[PLANE_MAINVIDEO];
    RECT rcSrc = plane.rcSrc;
    RECT rcVideoDst = m_rcMixingDst;

    NRectToRect(rcVideoDst, plane.nrcDst);

    hr = PlaneToScreen(plane, rcSrc, rcVideoDst, m_rcMixingDstClip);
    if (SUCCEEDED(hr))
    {
        D3DRECT *pArea = m_ClearNonVideoAreaList;
        if (rcVideoDst.top > rcWndRect.top)
        {
            pArea->x1 = rcWndRect.left;
            pArea->y1 = rcWndRect.top;
            pArea->x2 = rcWndRect.right;
            pArea->y2 = rcVideoDst.top;
            pArea++;
        }

        if (rcVideoDst.bottom < rcWndRect.bottom)
        {
            pArea->x1 = rcWndRect.left;
            pArea->y1 = rcVideoDst.bottom;
            pArea->x2 = rcWndRect.right;
            pArea->y2 = rcWndRect.bottom;
            pArea++;
        }

        if (rcVideoDst.left > rcWndRect.left)
        {
            pArea->x1 = rcWndRect.left;
            pArea->y1 = rcWndRect.top;
            pArea->x2 = rcVideoDst.left;
            pArea->y2 = rcWndRect.bottom;
            pArea++;
        }

        if (rcVideoDst.right < rcWndRect.right)
        {
            pArea->x1 = (rcVideoDst.right-1);
            pArea->y1 = rcWndRect.top;
            pArea->x2 = rcWndRect.right;
            pArea->y2 = rcWndRect.bottom;
            pArea++;
        }
        ASSERT(pArea - m_ClearNonVideoAreaList <= 4 && pArea - m_ClearNonVideoAreaList >= 0);
        m_uClearNonVideoAreaListCount = pArea - m_ClearNonVideoAreaList;
    }

    return S_OK;
}

HRESULT CD3D9VideoMixerBase::NotifyListeners(DispSvr::EVENT_VIDEO_MIXING event, DWORD dwParam1, DWORD dwParam2)
{
    HRESULT hr = S_OK;

    for (int i = 0; i < PLANE_MAX; i++)
    {
        if (m_Planes[i].bCreated && m_Planes[i].pListener)
        {
            hr |= m_Planes[i].pListener->Notify(event, dwParam1, dwParam2);
        }
    }
    return hr;
}

// check if main video exists and covers the whole background visible area.
bool CD3D9VideoMixerBase::IsBackgroundVisible() const
{
    const D3D9Plane &mv = m_Planes[PLANE_MAINVIDEO];
    const D3D9Plane &sv = m_Planes[PLANE_SUBVIDEO];
    const D3D9Plane &bk = m_Planes[PLANE_BACKGROUND];

    if (!IsD3D9PlaneValid(&bk))
        return false;

    // sub-video and background can't coexist.
    if (IsD3D9PlaneValid(&sv))
        return false;

    // test if main video covers whole background
    if (IsD3D9PlaneValid(&mv))
    {
        return mv.nrcDst.left < bk.nrcDst.left
            || mv.nrcDst.right > bk.nrcDst.right
            || mv.nrcDst.top > bk.nrcDst.top
            || mv.nrcDst.bottom < bk.nrcDst.bottom;
    }

    return true;
}

void CD3D9VideoMixerBase::ClearNonVideoArea()
{
    if (m_uClearNonVideoAreaListCount > 0)
    {
        DWORD dwValue = 0;
        HRESULT hr = m_pDevice->GetRenderState(D3DRS_STENCILENABLE, &dwValue);
        dwValue = dwValue ? D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER : D3DCLEAR_TARGET;

        hr = m_pDevice->Clear(m_uClearNonVideoAreaListCount,
            m_ClearNonVideoAreaList,
            dwValue,
            D3DCOLOR_XRGB(0, 0, 0),
            1.0f,
            0L);
    }
}

/// Switch current video processor to the new one and update the plane's properties.
static inline void SetVideoProcessor(D3D9PlaneConfig &cfg, D3D9Plane &plane, VideoProcessorStub *pStub, CD3D9TexturePool *pPool)
{
    ASSERT(pStub);
    if (cfg.pVideoProcessorStub == pStub)
        return;

    cfg.pVideoProcessorStub = pStub;
    // if plane.pVBlt already exists and the plane is in use,
    // change the current video processor to the updated one when necessary.
    if (plane.bCreated)
    {
        SAFE_DELETE(plane.pVBlt);
        plane.pVBlt = CreateVBltFromVPStub(&plane, pStub, pPool);
    }
}

// Binary predicate for sorting, return true if a > b.
bool VidProcSelectingPredicate(const VideoProcessorStub &a, const VideoProcessorStub &b)
{
    if (a.sCaps.eType == b.sCaps.eType)
    {
        // make dedicated 3rd party mixer the preferred mixer.
        bool bIsAPreferred = a.guidVP == DispSvr_VideoProcDxvaHD || a.guidVP == DispSvr_VideoProcFastComp || a.guidVP == DispSvr_VideoProcPCOM;
        bool bIsBPreferred = b.guidVP == DispSvr_VideoProcDxvaHD || b.guidVP == DispSvr_VideoProcFastComp || b.guidVP == DispSvr_VideoProcPCOM;
        if (bIsAPreferred ^ bIsBPreferred)
            return bIsAPreferred;

        if (a.sCaps.uProcessorCaps == b.sCaps.uProcessorCaps)
        {
            // return false to keep original VP sequence if these two VP has same caps.
            if (a.sCaps.uFilterCaps == b.sCaps.uFilterCaps)
                return false;
            return a.sCaps.uFilterCaps > b.sCaps.uFilterCaps;
        }
        return a.sCaps.uProcessorCaps > b.sCaps.uProcessorCaps;
    }
    // PROCESSOR_TYPE_HARDWARE = 0, PROCESSOR_TYPE_SOFTWARE = 1
    return a.sCaps.eType > b.sCaps.eType;
}

void CD3D9VideoMixerBase::SelectVideoProcessor(VideoProcessorStub *pMainVP, VideoProcessorStub *pSubVP)
{
    if (m_VideoProcessorList.empty())
        return;

    // sort the list by VP capabilities, the best is at the beginning.
    m_VideoProcessorList.sort(VidProcSelectingPredicate);

    if (pMainVP == NULL)
        pMainVP = &m_VideoProcessorList.front();

    // Sub VP may not be used depending on mixer implementation.
    // Perhaps we should select simpler VP instead of the best available.
    if (pSubVP == NULL)
        pSubVP = &m_VideoProcessorList.front();

    SetVideoProcessor(m_PlaneConfigs[PLANE_MAINVIDEO], m_Planes[PLANE_MAINVIDEO], pMainVP, m_pTexturePool);
    SetVideoProcessor(m_PlaneConfigs[PLANE_SUBVIDEO], m_Planes[PLANE_SUBVIDEO], pSubVP, m_pTexturePool);
}

HRESULT CD3D9VideoMixerBase::GenerateDxva2VPList()
{
    UINT uCount = 0;
    DXVA2VP_Caps *pCaps = 0;
    HRESULT hr;

    hr = CDXVA2VideoProcessor::GetDXVA2VPCaps(m_pDevice, &uCount, &pCaps);
    if (SUCCEEDED(hr))
    {
        // DXVA2 VP must support at least progressive and bob devices.
        ASSERT(uCount >= 2 && pCaps);

        VideoProcessorStub vp;
        for (UINT i = 0; i < uCount; i++)
        {
            ZeroMemory(&vp, sizeof(vp));
            vp.guidVP = pCaps[i].guidVP;
            vp.RenderTargetFormat = pCaps[i].RenderTargetFormat;
            vp.sCaps = pCaps[i].sCaps;
            memcpy(vp.FilterRanges, pCaps[i].FilterRanges, sizeof(vp.FilterRanges));
            for (int j = 0; j < sizeof(vp.FilterRanges) / sizeof(vp.FilterRanges[0]); j++)
            {
                vp.fFilterValue[j] = vp.FilterRanges[j].fDefaultValue;
            }
            vp.pfnVBltFactory = CDXVA2VBlt::Create;
            m_VideoProcessorList.push_back(vp);
        }
        delete [] pCaps;	// caller should release pCaps returned by GetDXVA2VPCaps()
    }
    m_VideoProcessorList.sort(VidProcSelectingPredicate);
    return hr;
}

HRESULT CD3D9VideoMixerBase::PlaneToScreen(const D3D9Plane &plane, RECT &rcSrc, RECT &rcDst, const RECT &rcDstClip)
{
    NORMALIZEDRECT nrcOutput = plane.nrcDst;

    //Each plane should refer to the their own mixing area's(viewport) aspect ratio, not whole mixing area(Window)
    // mixing area's aspect ratio = (window(whole mixing area) width * viewport's normalized width) /
    //                                             (window(whole mixing area) height * viewport's normalized height)
    float fMixingAreaAspectRatio = m_fWindowAspectRatio * (nrcOutput.right - nrcOutput.left) / (nrcOutput.bottom - nrcOutput.top);
    CorrectAspectRatio(nrcOutput, plane.fAspectRatio / fMixingAreaAspectRatio);
    // cropping applies to both source and destination.
    CropRect(nrcOutput, plane.nrcCrop);
    CropRect(rcSrc, plane.nrcCrop);

    // map normailzed destination rectangle to actual rectangle in pixel.
    NRectToRect(rcDst, nrcOutput);

    // check and clip source/destination rectangles when rcDst is outside of rcDstClip.
    ClipRect(rcDst, rcSrc, rcDstClip);

    // fail if dst is not viewable.
    return (rcDst.right <= rcDst.left || rcDst.bottom <= rcDst.top) ? E_FAIL : S_OK;
}

STDMETHODIMP CD3D9VideoMixerBase::GetAvailableVideoProcessorModeCount(UINT *pCount)
{
    CHECK_POINTER(pCount);
    CAutoLock selfLock(&m_csObj);
    *pCount = m_VideoProcessorList.size();
    return S_OK;
}

STDMETHODIMP CD3D9VideoMixerBase::GetAvailableVideoProcessorModes(GUID *pGUID)
{
    CHECK_POINTER(pGUID);
    CAutoLock selfLock(&m_csObj);
    for (VideoProcessorList::const_iterator i = m_VideoProcessorList.begin();
        i != m_VideoProcessorList.end();
        ++i)
    {
        *pGUID++ = i->guidVP;
    }
    return S_OK;
}

STDMETHODIMP CD3D9VideoMixerBase::GetVideoProcessorCaps(LPCGUID lpGUID, VideoProcessorCaps *pCaps)
{
    CHECK_POINTER(lpGUID);
    CAutoLock selfLock(&m_csObj);

    VideoProcessorList::const_iterator i = m_VideoProcessorList.begin();
    for (; i != m_VideoProcessorList.end(); ++i)
    {
        if (i->guidVP == *lpGUID)
            break;
    }
    if (i == m_VideoProcessorList.end())
        return E_FAIL;
    memcpy(pCaps, &i->sCaps, sizeof(VideoProcessorCaps));
    return S_OK;
}

STDMETHODIMP CD3D9VideoMixerBase::GetVideoProcessorMode(LPGUID lpGUID)
{
    CHECK_POINTER(lpGUID);
    CAutoLock selfLock(&m_csObj);
    const VideoProcessorStub *pVP = m_PlaneConfigs[PLANE_MAINVIDEO].pVideoProcessorStub;

    if (pVP)
    {
        *lpGUID = pVP->guidVP;
        return S_OK;
    }
    return E_FAIL;
}

STDMETHODIMP CD3D9VideoMixerBase::SetVideoProcessorMode(LPCGUID lpGUID)
{
    CHECK_POINTER(lpGUID);
    CAutoLock selfLock(&m_csObj);
    const VideoProcessorStub *pVP = m_PlaneConfigs[PLANE_MAINVIDEO].pVideoProcessorStub;

    if (pVP && IsEqualGUID(pVP->guidVP, *lpGUID))
        return S_FALSE;

    VideoProcessorList::iterator i = m_VideoProcessorList.begin();
    for (; i != m_VideoProcessorList.end(); ++i)
        if (i->guidVP == *lpGUID)
            break;

    if (i == m_VideoProcessorList.end())
        return E_INVALIDARG;

    SelectVideoProcessor(&*i);
    return S_OK;
}

STDMETHODIMP CD3D9VideoMixerBase::GetFilterValueRange(VIDEO_FILTER eFilter, ValueRange *pValueRange)
{
    CHECK_POINTER(pValueRange);
    CAutoLock selfLock(&m_csObj);
    const VideoProcessorStub *pVP = m_PlaneConfigs[PLANE_MAINVIDEO].pVideoProcessorStub;

    if (pVP)
    {
        if ((1 << eFilter) & pVP->sCaps.uFilterCaps)
        {
            memcpy(pValueRange, &pVP->FilterRanges[eFilter], sizeof(ValueRange));
            return S_OK;
        }
        return E_INVALIDARG;
    }
    return E_FAIL;
}

STDMETHODIMP CD3D9VideoMixerBase::SetFilterValue(VIDEO_FILTER eFilter, float fValue)
{
    CAutoLock selfLock(&m_csObj);
    const VideoProcessorStub *pVP = m_PlaneConfigs[PLANE_MAINVIDEO].pVideoProcessorStub;

    if (pVP)
    {
        if ((1 << eFilter) & pVP->sCaps.uFilterCaps)
        {
            if (pVP->FilterRanges[eFilter].fMinValue <= fValue
                && pVP->FilterRanges[eFilter].fMaxValue >= fValue)
            {
                m_PlaneConfigs[PLANE_MAINVIDEO].pVideoProcessorStub->fFilterValue[eFilter] = fValue;
                return S_OK;
            }
        }
        return E_INVALIDARG;
    }
    return E_FAIL;
}

STDMETHODIMP CD3D9VideoMixerBase::GetFilterValue(VIDEO_FILTER eFilter, float *pfValue)
{
    CHECK_POINTER(pfValue);
    CAutoLock selfLock(&m_csObj);
    const VideoProcessorStub *pVP = m_PlaneConfigs[PLANE_MAINVIDEO].pVideoProcessorStub;

    if (pVP)
    {
        if ((1 << eFilter) & pVP->sCaps.uFilterCaps)
        {
            *pfValue = pVP->fFilterValue[eFilter];
            return S_OK;
        }
        return E_INVALIDARG;
    }
    return E_FAIL;
}

HRESULT CD3D9VideoMixerBase::ProcessVideoEffect(PLANE_ID PlaneID, IDirect3DSurface9 *pDestSurface, RECT &rcSrc,const RECT &rcDst, PLANE_FORMAT IntermediateFormat, IUnknown **ppDestUnk)
{
    CHECK_POINTER(ppDestUnk);

    HRESULT hr = E_FAIL;

    // S_OK indicates video effect processing done.
    // S_FALSE indicates video effect is not enabled.
    if (!m_pXVEManager)
        return S_FALSE;

    BOOL bEnabled = FALSE;
    hr = m_pXVEManager->IsEnabled(PlaneID, &bEnabled);
    if (FAILED(hr) || !bEnabled)
        return S_FALSE;

    CComPtr<IDirect3DTexture9> pTexture;
    CComPtr<IDirect3DSurface9> pSurface;
    D3DSURFACE_DESC desc;
    D3D9Plane &plane = m_Planes[PlaneID];

    hr = m_pTexturePool->GetRepresentation(plane.hTexture, IID_IDirect3DTexture9, (void **)&pTexture);
    hr = m_pTexturePool->GetRepresentation(plane.hTexture, IID_IDirect3DSurface9, (void **)&pSurface);

    if (!pTexture && !pSurface)
        return E_FAIL;


    CComPtr<IXVESample> pInputSample;
    CComPtr<IXVESample> pOutputSample;
    CComPtr<IXVEType> pInputType;
    m_pXVEManager->CreateEmptyContainer(__uuidof(IXVESample), (VOID **)&pInputSample);
    m_pXVEManager->CreateEmptyContainer(__uuidof(IXVESample), (VOID **)&pOutputSample);

    if (pInputSample && pOutputSample)
    {
        hr = S_OK;
        UINT uIndex = 0;
        BOOL bSurface = FALSE;
        BOOL bTexture = FALSE;

        m_pXVEManager->LockStore();
        do 
        {
            pInputType = NULL;
            hr = m_pXVEManager->GetInputAvailableType(PlaneID, uIndex, &pInputType);
            if (SUCCEEDED(hr))
            {
                XVE::XVE_FORMAT_TYPE xveFormatType;
                if (FAILED(pInputType->GetFrameType(&xveFormatType)))
                    break;
                if (xveFormatType == XVE::XVE_FORMAT_UNKNOWN)
                {
                    bSurface = TRUE;
                    bTexture = TRUE;
                }
                else if (xveFormatType == XVE::XVE_FORMAT_TEXTURE)
                {
                    bTexture = TRUE;
                }
                else if (xveFormatType == XVE::XVE_FORMAT_SURFACE)
                {
                    bSurface = TRUE;
                }
            }
            uIndex++;

        } while (SUCCEEDED(hr));
        m_pXVEManager->UnLockStore();

        if (!bSurface && !bTexture) //video effect has to support at least one type.
            return E_FAIL;

        ZeroMemory(&desc, sizeof(D3DSURFACE_DESC));
        if (pSurface)
        {
            if (bSurface)
            {
                if (plane.VideoProperty.Format != VIDEO_FORMAT_PROGRESSIVE) //change interlaced frame to progressive
                {
                    CComPtr<IDirect3DSurface9> pProgressiveSurface;
                    hr = plane.pVBlt->IntermediateVBlt(pSurface, rcSrc, PLANE_FORMAT_YUY2/*IntermediateFormat*/, &pProgressiveSurface);
                    if (SUCCEEDED(hr) && pProgressiveSurface)
                    {
                        pSurface = pProgressiveSurface;
                        pProgressiveSurface = NULL;
                    }
                    else
                        return hr;
                }

                pSurface->GetDesc(&desc);
                pInputSample->SetFrame(pSurface);
                pInputSample->SetFrameType(XVE::XVE_FORMAT_SURFACE);
            }
            else // bTexture == TRUE
            {
                if ((plane.VideoProperty.Format != VIDEO_FORMAT_PROGRESSIVE) || !pTexture) // interlaced or texture not exist.
                {
                    CComPtr<IDirect3DTexture9> pProgressiveTexture;
                    hr = plane.pVBlt->IntermediateTextureVBlt(pSurface, rcSrc, &pProgressiveTexture);
                    pTexture = pProgressiveTexture;
                    pProgressiveTexture = NULL;
#ifndef _NO_USE_D3DXDLL
                    if (0)
                    {
                        hr = D3DXSaveTextureToFile(_T("C:\\Intermediate.bmp"), D3DXIFF_BMP, pProgressiveTexture, NULL);
                    }
#endif
                }
                pTexture->GetLevelDesc(0, &desc);
                pInputSample->SetFrame(pTexture);
                pInputSample->SetFrameType(XVE::XVE_FORMAT_TEXTURE);
            }
        }
        else /*pTexture*/
        {
            if (bTexture) //try same type first
            {
                if (plane.VideoProperty.Format != VIDEO_FORMAT_PROGRESSIVE) // interlaced
                {
                    pSurface = NULL;
                    pTexture->GetSurfaceLevel(0, &pSurface);

                    CComPtr<IDirect3DTexture9> pProgressiveTexture;
                    hr = plane.pVBlt->IntermediateTextureVBlt(pSurface, rcSrc, &pProgressiveTexture);
                    pTexture = pProgressiveTexture;
                    pProgressiveTexture = NULL;
                }
                pTexture->GetLevelDesc(0, &desc);
                pInputSample->SetFrame(pTexture);
                pInputSample->SetFrameType(XVE::XVE_FORMAT_TEXTURE);
            }
            else //bSurface == TRUE, need to convert texture to surface.
            {
                CComPtr<IDirect3DSurface9> pProgressiveSurface;

                pSurface = NULL;
                pTexture->GetSurfaceLevel(0, &pSurface);

                hr = plane.pVBlt->IntermediateVBlt(pSurface, rcSrc, PLANE_FORMAT_YUY2/*IntermediateFormat*/, &pProgressiveSurface);
                if (SUCCEEDED(hr) && pProgressiveSurface)
                {
                    pSurface = pProgressiveSurface;
                    pProgressiveSurface = NULL;
                }
                else
                {
                    return hr;
                }
                pSurface->GetDesc(&desc);
                pInputSample->SetFrame(pSurface);
                pInputSample->SetFrameType(XVE::XVE_FORMAT_SURFACE);
            }
        }

        pInputSample->SetFrameFormat(desc.Format);
        pInputSample->SetDisplaySize((m_rcMixingDst.right - m_rcMixingDst.left), (m_rcMixingDst.bottom - m_rcMixingDst.top));
        pInputSample->SetFrameSize(rcSrc.right - rcSrc.left, rcSrc.bottom - rcSrc.top);
        hr = m_pXVEManager->ProcessEffect(PlaneID, pInputSample, pOutputSample);
        if (SUCCEEDED(hr))
        {
            UINT uWidth = 0, uHeight = 0;
            hr = pOutputSample->GetFrame(ppDestUnk);
            hr = pOutputSample->GetFrameSize(&uWidth, &uHeight);
            rcSrc.left = 0;
            rcSrc.right = uWidth;
            rcSrc.top = 0;
            rcSrc.bottom = uHeight;
        }
    }
    return hr;
}

void CD3D9VideoMixerBase::ReleaseVideoSamples(PLANE_ID PlaneID)
{
    D3D9Plane &plane = m_Planes[PlaneID];

    while (!plane.VideoSamples.empty())
    {
        m_pTexturePool->ReleaseTexture(plane.VideoSamples.back().hTexture);
        plane.VideoSamples.pop_back();
    }
    plane.uiPlaneSampleIndex = 0;
    plane.hTexture = 0;
}