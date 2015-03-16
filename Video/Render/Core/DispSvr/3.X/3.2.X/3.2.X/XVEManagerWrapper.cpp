// XEVManagerWrapper.cpp : Implementation of CXEVManagerWrapper
#include "stdafx.h"
#include "D3D9PluginBase.h"
#include "XVEManagerWrapper.h"
#include "Imports/xve/Inc/XVE.h"

using namespace DispSvr;
CDispSvrXVEManagerWrapper::CDispSvrXVEManagerWrapper()
{
    m_bInitialized = FALSE;
    m_pXVEManager = NULL;
    m_GUID = DISPSVR_RESOURCE_XVIDEOEFFECTMANAGER;
}
CDispSvrXVEManagerWrapper::~CDispSvrXVEManagerWrapper()
{
    _ReleaseManager();
}

STDMETHODIMP CDispSvrXVEManagerWrapper::QueryInterface(REFIID riid, void **ppv)
{
    HRESULT hr = E_NOINTERFACE;

    if (riid == __uuidof(IDispSvrXVEManagerWrapper))
    {
        hr = GetInterface((IDispSvrXVEManagerWrapper *)this, ppv);
    }
    else
    {
        hr = CD3D9PluginBase::QueryInterface(riid, ppv);
    }
    return hr;
}

// IDispSvrPlugin
STDMETHODIMP CDispSvrXVEManagerWrapper::ProcessMessage(RESOURCE_MESSAGE_TYPE msg, LPVOID ulParam)
{
    HRESULT hr = S_OK;
    switch (msg)
    {
    case RESOURCE_MESSAGE_SETDEVICE:
        hr = _SetDevice(ulParam);
        break;
    case RESOURCE_MESSAGE_RELEASEDEVICE:
        hr = _ReleaseDevice(ulParam);
        break;
    case RESOURCE_MESSAGE_TERMINATE:
        hr = _ReleaseManager();
        break;
    }
	return CD3D9PluginBase::ProcessMessage(msg, ulParam);
}

//IDispSvrXVEManagerWrapper
STDMETHODIMP CDispSvrXVEManagerWrapper::SetXVEManager(IXVideoEffectManager *pManager)
{
    _ReleaseManager();
    m_pXVEManager = pManager;
    if (m_pXVEManager)
    {
        m_pXVEManager->AddRef();
        _InitManager();
    }
    return S_OK;
}

STDMETHODIMP CDispSvrXVEManagerWrapper::GetXVEManager(IXVideoEffectManager **ppManager)
{
    CHECK_POINTER(ppManager);

    HRESULT hr = E_FAIL;
    (*ppManager) = m_pXVEManager;
    if (*ppManager)
    {
        (*ppManager)->AddRef();
        hr = S_OK;
    }
    else
    {
        hr = S_FALSE;
    }

    return hr;
}

HRESULT CDispSvrXVEManagerWrapper::_InitManager()
{
    if (m_pXVEManager)
    {
        m_pXVEManager->ProcessMessage(XVE::XVE_MESSAGE_INITIALIZE, NULL, NULL);
		_SetDevice(m_pDevice);
    }
    return S_OK;
}

HRESULT CDispSvrXVEManagerWrapper::_ReleaseManager()
{
    _ReleaseDevice(NULL);
    _Terminate(NULL);
    SAFE_RELEASE(m_pXVEManager);

    return S_OK;
}

HRESULT CDispSvrXVEManagerWrapper::_SetDevice(LPVOID ulParam)
{
    if (m_pXVEManager)
    {
        m_pXVEManager->ProcessMessage(XVE::XVE_MESSAGE_SETDEVICE, (ULONGLONG)ulParam, NULL);
    }
    return S_OK;
}

HRESULT CDispSvrXVEManagerWrapper::_ReleaseDevice(LPVOID ulParam)
{
    if (m_pXVEManager)
    {
        m_pXVEManager->ProcessMessage(XVE::XVE_MESSAGE_RELEASEDEVICE, (ULONGLONG)ulParam, NULL);
    }
    return S_OK;
}

HRESULT CDispSvrXVEManagerWrapper::_Terminate(LPVOID ulParam)
{
    if (m_pXVEManager)
    {
        m_pXVEManager->ProcessMessage(XVE::XVE_MESSAGE_UNINITIALIZE, NULL, NULL);
    }
    return S_OK;
}
