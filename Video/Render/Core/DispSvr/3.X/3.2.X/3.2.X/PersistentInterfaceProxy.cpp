#include "stdafx.h"
#include "D3D9VideoMixerModel.h"
#include "D3D9TexturePool.h"
#include "D3D9VBlt.h"
#include "D3D9VideoPlane.h"
#include "PersistentInterfaceProxy.h"

using namespace DispSvr;

CPersistentInterfaceProxy::CPersistentInterfaceProxy() : m_pVideoMixerProxy(new CVideoMixerInterfaceProxy)
{
}

CPersistentInterfaceProxy::~CPersistentInterfaceProxy()
{
    delete m_pVideoMixerProxy;
}

HRESULT CPersistentInterfaceProxy::GetInterface(REFGUID guidGroupId, REFIID riid, void **ppv)
{
    if (guidGroupId == DISPSVR_RESOURCE_VIDEOMIXER)
    {
        ASSERT(m_pVideoMixerProxy);
        return m_pVideoMixerProxy->QueryInterface(riid, ppv);
    }
    return E_NOINTERFACE;
}

HRESULT CPersistentInterfaceProxy::SetInstance(REFGUID guidGroupId, IUnknown *pInstance)
{
    if (guidGroupId == DISPSVR_RESOURCE_VIDEOMIXER)
    {
        ASSERT(m_pVideoMixerProxy);
        return m_pVideoMixerProxy->SetInstance(pInstance);
    }
    return S_FALSE;
}


///////////////////////////////////////////////////////////////////////////////////////////////
/// CVideoMixerInterfaceProxy

//#define DEBUG_INTERFACE_PROXY
#ifdef DEBUG_INTERFACE_PROXY
static int inline DbgMsgInterfaceName(LPSTR s, LPVOID obj, int hr)
{
    DbgMsg("Delegating (0x%x)->%s = 0x%x", obj, s, hr);
    ASSERT(obj);
    return hr;
}
#else
#define DbgMsgInterfaceName(s, obj, f) f
#endif

#define PROXYNS CPersistentInterfaceProxy::CVideoMixerInterfaceProxy
#define SEL(obj, func)    DbgMsgInterfaceName(__FUNCTION__, obj, (obj) ? (obj)->func : E_NOINTERFACE)


PROXYNS::CVideoMixerInterfaceProxy() : m_pMixer(0), m_pVideoProcessor(0), m_pEffMgr(0), m_pVideoPlaneCB(0), m_pInstance(0)
{
}

PROXYNS::~CVideoMixerInterfaceProxy()
{
}

HRESULT PROXYNS::SetInstance(IUnknown *pInstance)
{
    HRESULT hr = S_OK;

    m_pInstance = 0;
    m_pEffMgr = 0;
    m_pVideoProcessor = 0;
    m_pMixer = 0;
    m_pVideoPlaneCB = 0;

    if (pInstance)
    {
        hr = pInstance->QueryInterface(__uuidof(IUnknown), (void **)&m_pInstance);
        hr |= pInstance->QueryInterface(__uuidof(IDispSvrVideoMixer), (void **)&m_pMixer);
        hr |= pInstance->QueryInterface(__uuidof(IDispSvrVideoProcessor), (void **)&m_pVideoProcessor);
        hr |= pInstance->QueryInterface(__uuidof(IDispSvrVideoEffectManager), (void **)&m_pEffMgr);
        hr |= pInstance->QueryInterface(__uuidof(IDispSvrVideoPlaneCallback), (void **)&m_pVideoPlaneCB);
        if (m_pInstance)
            m_pInstance->Release();
        if (m_pMixer)
            m_pMixer->Release();
        if (m_pEffMgr)
            m_pEffMgr->Release();
        if (m_pVideoProcessor)
            m_pVideoProcessor->Release();
        if (m_pVideoPlaneCB)
            m_pVideoPlaneCB->Release();
    }

    DbgMsg("PROXYNS::SetInstance(0x%x) = 0x%x", pInstance, hr);
    ASSERT(SUCCEEDED(hr));  // all interfaces must be supported by the instance.
    return hr;
}

STDMETHODIMP PROXYNS::QueryInterface(REFIID riid, void **ppv)
{
	HRESULT hr = E_NOINTERFACE;

	if (riid == __uuidof(IDispSvrVideoMixer))
	{
        return ::GetInterface((IDispSvrVideoMixer *)this, ppv);
	}
	else if (riid == __uuidof(IDispSvrVideoProcessor))
	{
        return ::GetInterface((IDispSvrVideoProcessor *)this, ppv);
	}
    else if (riid == __uuidof(IDispSvrVideoEffectManager))
    {
        return ::GetInterface((IDispSvrVideoEffectManager *)this, ppv);
    }
    else if (riid == __uuidof(IDispSvrVideoPlaneCallback))
    {
        return ::GetInterface((IDispSvrVideoPlaneCallback *) this, ppv);
    }

    DbgMsg("PROXYNS::QueryInterfac can NOT find corresponding interface");
    ASSERT(0 && "Proxy can't find the correct interface to response. It will have problem.");
    return hr;
}

STDMETHODIMP_(ULONG) PROXYNS::AddRef()
{
	return 1;
}

STDMETHODIMP_(ULONG) PROXYNS::Release()
{
	return 1;
}

// IDispSvrVideoMixer
STDMETHODIMP PROXYNS::Execute()
{
    return SEL(m_pMixer, Execute());
}

STDMETHODIMP PROXYNS::SetDestination(IUnknown *pSurface, const NORMALIZEDRECT *pDest)
{
    return SEL(m_pMixer, SetDestination(pSurface, pDest));
}

STDMETHODIMP PROXYNS::QueryCaps(MixerCaps *pCaps)
{
    return SEL(m_pMixer, QueryCaps(pCaps));
}

STDMETHODIMP PROXYNS::CreatePlane(PlaneInit *pInit, REFIID riid, void **ppPlane)
{
    return SEL(m_pMixer, CreatePlane(pInit, riid, ppPlane));
}

STDMETHODIMP PROXYNS::QueryPlaneFormatCount(PLANE_ID PlaneID, UINT *pCount)
{
    return SEL(m_pMixer, QueryPlaneFormatCount(PlaneID, pCount));
}

STDMETHODIMP PROXYNS::QueryPlaneFormat(PLANE_ID PlaneID, PLANE_FORMAT *pFormats)
{
    return SEL(m_pMixer, QueryPlaneFormat(PlaneID, pFormats));
}

STDMETHODIMP PROXYNS::QueryPlaneCaps(PLANE_ID PlaneID, PLANE_FORMAT Format, PlaneCaps *pCap)
{
    return SEL(m_pMixer, QueryPlaneCaps(PlaneID, Format, pCap));
}

STDMETHODIMP PROXYNS::SetClearRectangles(UINT uCount, ClearRect *pRects)
{
    return SEL(m_pMixer, SetClearRectangles(uCount, pRects));
}

STDMETHODIMP PROXYNS::SetBackgroundColor(COLORREF Color)
{
    return SEL(m_pMixer, SetBackgroundColor(Color));
}

STDMETHODIMP PROXYNS::GetBackgroundColor(COLORREF *pColor)
{
    return SEL(m_pMixer, GetBackgroundColor(pColor));
}

STDMETHODIMP PROXYNS::SetLumaKey(const LumaKey *pLumaKey)
{
    return SEL(m_pMixer, SetLumaKey(pLumaKey));
}

STDMETHODIMP PROXYNS::GetLumaKey(LumaKey *pLumaKey)
{
    return SEL(m_pMixer, GetLumaKey(pLumaKey));
}

STDMETHODIMP PROXYNS::SetProperty(const MixerProperty *pProperty)
{
    return SEL(m_pMixer, SetProperty(pProperty));
}

STDMETHODIMP PROXYNS::GetProperty(MixerProperty *pProperty)
{
    return SEL(m_pMixer, GetProperty(pProperty));
}

// IDispSvrVideoProcessor
STDMETHODIMP PROXYNS::GetAvailableVideoProcessorModeCount(UINT *pCount)
{
    return SEL(m_pVideoProcessor, GetAvailableVideoProcessorModeCount(pCount));
}

STDMETHODIMP PROXYNS::GetAvailableVideoProcessorModes(GUID *pGUID)
{
    return SEL(m_pVideoProcessor, GetAvailableVideoProcessorModes(pGUID));
}

STDMETHODIMP PROXYNS::GetVideoProcessorCaps(LPCGUID lpGUID, VideoProcessorCaps *pCaps)
{
    return SEL(m_pVideoProcessor, GetVideoProcessorCaps(lpGUID, pCaps));
}

STDMETHODIMP PROXYNS::GetVideoProcessorMode(LPGUID lpGUID)
{
    return SEL(m_pVideoProcessor, GetVideoProcessorMode(lpGUID));
}

STDMETHODIMP PROXYNS::SetVideoProcessorMode(LPCGUID lpGUID)
{
    return SEL(m_pVideoProcessor, SetVideoProcessorMode(lpGUID));
}

STDMETHODIMP PROXYNS::GetFilterValueRange(VIDEO_FILTER eFilter, ValueRange *pValueRange)
{
    return SEL(m_pVideoProcessor, GetFilterValueRange(eFilter, pValueRange));
}

STDMETHODIMP PROXYNS::SetFilterValue(VIDEO_FILTER eFilter, float fValue)
{
    return SEL(m_pVideoProcessor, SetFilterValue(eFilter, fValue));
}

STDMETHODIMP PROXYNS::GetFilterValue(VIDEO_FILTER eFilter, float *pfValue)
{
    return SEL(m_pVideoProcessor, GetFilterValue(eFilter, pfValue));
}

// IDispSvrVideoEffectManager
STDMETHODIMP PROXYNS::GetVideoEffectManager(IUnknown **ppManager)
{
    return SEL(m_pEffMgr, GetVideoEffectManager(ppManager));
}

STDMETHODIMP PROXYNS::SetVideoEffectManager(IUnknown *pManager)
{
    return SEL(m_pEffMgr, SetVideoEffectManager(pManager));
}

// IDispSvrVideoPlaneCallback
const D3D9PlaneConfig &PROXYNS::GetPlaneConfig(PLANE_ID PlaneID)
{
    static const D3D9PlaneConfig s_EmptyConfig = {0};
    if (m_pVideoPlaneCB)
        return m_pVideoPlaneCB->GetPlaneConfig(PlaneID);
    else
        return s_EmptyConfig;
}

STDMETHODIMP_(void) PROXYNS::OnVideoPositionChange()
{
    if (m_pVideoPlaneCB)
        m_pVideoPlaneCB->OnVideoPositionChange();
}

STDMETHODIMP_(void) PROXYNS::OnDestroyPlane(PLANE_ID PlaneID)
{
    if (m_pVideoPlaneCB)
        m_pVideoPlaneCB->OnDestroyPlane(PlaneID);
}

STDMETHODIMP PROXYNS::OnCreatePlane(const DispSvr::PlaneInit &pInit)
{
    return SEL(m_pVideoPlaneCB, OnCreatePlane(pInit));
}