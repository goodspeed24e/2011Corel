#include "stdafx.h"
#include <algorithm>
#include "D3D9VideoEffect3DManager.h"

using namespace DispSvr;

CD3D9VideoEffect3DManager::CD3D9VideoEffect3DManager() : m_uEnabledCount(0)
{
	ZeroMemory(&m_Property, sizeof(m_Property));
	m_GUID = DISPSVR_RESOURCE_VIDEOEFFECT3DMANAGER;
}

CD3D9VideoEffect3DManager::~CD3D9VideoEffect3DManager()
{
	CAutoLock lock(&m_csObj);
	while (!m_listEffect.empty())
	{
		EffectList::reference r = m_listEffect.back();
		if (r.bEnable)
		{
			ASSERT(m_uEnabledCount > 0);
			m_uEnabledCount--;
		}
		r.pInstance->ProcessMessage(VE3D_MESSAGE_TYPE_RELEASEDEVICE, 0);
		r.pInstance->Release();
		m_listEffect.pop_back();
	}
}

STDMETHODIMP CD3D9VideoEffect3DManager::QueryInterface(REFIID riid, void **ppv)
{
	HRESULT hr = E_NOINTERFACE;

	if (riid == __uuidof(IDispSvrVideoEffect3DManager))
	{
		hr = GetInterface((IDispSvrVideoEffect3DManager *)this, ppv);
	}
	else if (riid == __uuidof(ID3D9VideoEffect3DProcessor))
	{
		hr = GetInterface((ID3D9VideoEffect3DProcessor *) this, ppv);
	}
	else
	{
		hr = CD3D9PluginBase::QueryInterface(riid, ppv);
	}
	return hr;
}

// IDispSvrPlugin
STDMETHODIMP CD3D9VideoEffect3DManager::ProcessMessage(RESOURCE_MESSAGE_TYPE msg, LPVOID ulParam)
{
	CAutoLock lock(&m_csObj);
	HRESULT hr = CD3D9PluginBase::ProcessMessage(msg, ulParam);

	if (msg == RESOURCE_MESSAGE_SETDEVICE)
	{
		for (EffectList::const_iterator i = m_listEffect.begin(); i != m_listEffect.end(); ++i)
		{
			i->pInstance->ProcessMessage(VE3D_MESSAGE_TYPE_SETDEVICE, ulParam);
		}
	}
	else if (msg == RESOURCE_MESSAGE_RELEASEDEVICE)
	{
		for (EffectList::const_iterator i = m_listEffect.begin(); i != m_listEffect.end(); ++i)
		{
			i->pInstance->ProcessMessage(VE3D_MESSAGE_TYPE_RELEASEDEVICE, 0);
		}
	}
    else if (msg == RESOURCE_MESSAGE_TERMINATE)
    {
        for (EffectList::const_iterator i = m_listEffect.begin(); i != m_listEffect.end(); ++i)
        {
            Enable(i->pInstance, FALSE);
        }
    }
	return hr;
}

STDMETHODIMP CD3D9VideoEffect3DManager::Install(IDispSvrVideoEffect3DPlugin *pPlugin)
{
	CHECK_POINTER(pPlugin);
	CAutoLock lock(&m_csObj);

	if (std::find(m_listEffect.begin(), m_listEffect.end(), pPlugin) != m_listEffect.end())
		return S_FALSE;

	Effect eff = {0};
	HRESULT hr = pPlugin->QueryInterface(__uuidof(IDispSvrVideoEffect3DPlugin), (void **) &eff.pInstance);
	if (FAILED(hr))
		return hr;

//	eff.bEnable = FALSE;	// default to dormant until enabled specifically.
	hr = eff.pInstance->ProcessMessage(VE3D_MESSAGE_TYPE_INITIALIZE, static_cast<IDispSvrVideoEffect3DManager *> (this));
	if (SUCCEEDED(hr))
	{
		hr = eff.pInstance->ProcessMessage(VE3D_MESSAGE_TYPE_ENABLE, &eff.bEnable);
		if (SUCCEEDED(hr))
		{
			if (eff.bEnable)
				m_uEnabledCount++;
			m_listEffect.push_back(eff);
			return hr;
		}
	}

	pPlugin->Release();
	return hr;
}

STDMETHODIMP CD3D9VideoEffect3DManager::Uninstall(IDispSvrVideoEffect3DPlugin *pPlugin)
{
	CHECK_POINTER(pPlugin);
	CAutoLock lock(&m_csObj);
	EffectList::iterator i =  std::find(m_listEffect.begin(), m_listEffect.end(), pPlugin);

	if (i != m_listEffect.end())
	{
		if (i->bEnable)
		{
			ASSERT(m_uEnabledCount > 0);
			m_uEnabledCount--;
		}
		i->pInstance->ProcessMessage(VE3D_MESSAGE_TYPE_RELEASEDEVICE, 0);
		i->pInstance->Release();
		m_listEffect.erase(i);
		return S_OK;
	}
	return S_FALSE;
}

STDMETHODIMP CD3D9VideoEffect3DManager::Enable(const IDispSvrVideoEffect3DPlugin *pPlugin, BOOL bEnable)
{
	CHECK_POINTER(pPlugin);
	CAutoLock lock(&m_csObj);
	HRESULT hr = E_FAIL;
	EffectList::iterator i = std::find(m_listEffect.begin(), m_listEffect.end(), pPlugin);
	
	if (i != m_listEffect.end())
	{
		if (i->bEnable == bEnable)
			return S_FALSE;

		hr = i->pInstance->ProcessMessage(VE3D_MESSAGE_TYPE_ENABLE, &bEnable);
		if (FAILED(hr))
			return hr;

		i->bEnable = bEnable;
		if (i->bEnable)
		{
			i->pInstance->ProcessMessage(VE3D_MESSAGE_TYPE_SETDEVICE, m_pDevice);
			m_uEnabledCount++;
		}
		else
		{
			i->pInstance->ProcessMessage(VE3D_MESSAGE_TYPE_RELEASEDEVICE, 0);
			ASSERT(m_uEnabledCount > 0);
			m_uEnabledCount--;
		}
	}
	return hr;
}

STDMETHODIMP CD3D9VideoEffect3DManager::IsEnabled(const IDispSvrVideoEffect3DPlugin *pPlugin, BOOL *pbEnabled)
{
	CHECK_POINTER(pbEnabled);
	CHECK_POINTER(pPlugin);
	CAutoLock lock(&m_csObj);
	EffectList::const_iterator i = std::find(m_listEffect.begin(), m_listEffect.end(), pPlugin);
	
	if (i != m_listEffect.end())
	{
		*pbEnabled = i->bEnable;
		return S_OK;
	}
	return E_FAIL;
}

STDMETHODIMP CD3D9VideoEffect3DManager::GetEffectCount(UINT *puCount)
{
	CHECK_POINTER(puCount);
	CAutoLock lock(&m_csObj);
	*puCount = m_listEffect.size();
	return S_OK;
}

STDMETHODIMP CD3D9VideoEffect3DManager::GetEffectAt(UINT uIndex, IDispSvrVideoEffect3DPlugin **ppPlugin)
{
	CHECK_POINTER(ppPlugin);
	CAutoLock lock(&m_csObj);

	if (uIndex >= m_listEffect.size())
		return E_INVALIDARG;

	return m_listEffect[uIndex].pInstance->QueryInterface(__uuidof(IDispSvrVideoEffect3DPlugin), (void **)ppPlugin);
}

STDMETHODIMP CD3D9VideoEffect3DManager::GetCaps(VE3DManagerCaps *pCaps)
{
	if (!pCaps)
		return E_POINTER;

	pCaps->eType = EFFECT_MANAGER_TYPE_SHADER_BASED;
	pCaps->uInputBufferCaps = EFFECT_BUFFER_CAPS_TEXTURE;
	pCaps->uOutputBufferCaps = EFFECT_BUFFER_CAPS_TEXTURE;
	return S_OK;
}

STDMETHODIMP CD3D9VideoEffect3DManager::SetProperty(const VE3DManagerProperty *pProperty)
{
	CAutoLock lock(&m_csObj);
	memcpy(&m_Property, pProperty, sizeof(m_Property));
	return S_OK;
}

STDMETHODIMP CD3D9VideoEffect3DManager::GetProperty(VE3DManagerProperty *pProperty)
{
	CAutoLock lock(&m_csObj);
	return E_NOTIMPL;
}

STDMETHODIMP CD3D9VideoEffect3DManager::GetAvailableEffectGUID(UINT *puCount, GUID **ppAvailabeEffectGUID)
{
    CHECK_POINTER(puCount);
    CHECK_POINTER(ppAvailabeEffectGUID);
    CAutoLock lock(&m_csObj);

    GetEffectCount(puCount);

    if ((*puCount) > 0)
    {
        HRESULT hr = E_FAIL;
        IDispSvrVideoEffect3DPlugin *pPlugin = NULL;

        (*ppAvailabeEffectGUID) = (GUID *)CoTaskMemAlloc((*puCount) * sizeof(GUID));

        if ((*ppAvailabeEffectGUID) == NULL)
            return E_OUTOFMEMORY;

        for (UINT uIndex = 0; uIndex < (*puCount); uIndex++)
        {
            hr = m_listEffect[uIndex].pInstance->QueryInterface(__uuidof(IDispSvrVideoEffect3DPlugin), (void **)&pPlugin);
            if (SUCCEEDED(hr) && pPlugin)
            {
                pPlugin->GetResourceId(&((*ppAvailabeEffectGUID)[uIndex]));
                pPlugin->Release();
                pPlugin = NULL;
            }
        }
    }
    else
    {
        (*ppAvailabeEffectGUID) = NULL;
    }

    return S_OK;
}

STDMETHODIMP CD3D9VideoEffect3DManager::GetEffectByGUID(const GUID *pGUID, IDispSvrVideoEffect3DPlugin **ppPlugin)
{
    CHECK_POINTER(pGUID);
    CHECK_POINTER(ppPlugin);
    CAutoLock lock(&m_csObj);

    BOOL bFoundEffect = FALSE;
    HRESULT hr = E_FAIL;
    GUID guidEffectID = GUID_NULL;
    IDispSvrVideoEffect3DPlugin *pPlugin = NULL;
    for (UINT uIndex = 0; uIndex < m_listEffect.size(); uIndex++)
    {
        hr = m_listEffect[uIndex].pInstance->QueryInterface(__uuidof(IDispSvrVideoEffect3DPlugin), (void **)&pPlugin);
        if (SUCCEEDED(hr) && pPlugin)
        {
            hr = pPlugin->GetResourceId(&guidEffectID);
            if (memcmp(pGUID, &guidEffectID, sizeof(GUID)) == 0)
            {
                *ppPlugin = pPlugin;
                (*ppPlugin)->AddRef();
                bFoundEffect = TRUE;
            }
            pPlugin->Release();
            pPlugin = NULL;

            if (bFoundEffect)
                return S_OK;
        }
    }

    return E_FAIL;
}

STDMETHODIMP_(BOOL) CD3D9VideoEffect3DManager::IsEffectEnabled()
{
	return m_uEnabledCount > 0;
}

STDMETHODIMP CD3D9VideoEffect3DManager::ProcessEffect(ProcessEffectRequest *pRequest)
{
	if (pRequest == 0
		|| pRequest->pInput == 0 || pRequest->ppOutput == 0
		|| pRequest->lpSourceRect == 0 || IsRectEmpty(pRequest->lpSourceRect))
		return E_INVALIDARG;

	CAutoLock lock(&m_csObj);
	VE3DBuffer buffer[2];
	VE3DBuffer *pInputBuffer = &buffer[1], *pOutputBuffer = &buffer[0];
	// check if input surface is a texture.
	HRESULT hr = pRequest->pInput->QueryInterface(IID_IDirect3DTexture9, (void **) pRequest->ppOutput);
	if (FAILED(hr))
		return E_INVALIDARG;

	(*pRequest->ppOutput)->Release();
	ZeroMemory(buffer, sizeof(buffer));
	buffer[0].pSurface = pRequest->pInput;
	buffer[0].uWidth = pRequest->lpSourceRect->right - pRequest->lpSourceRect->left;
	buffer[0].uHeight = pRequest->lpSourceRect->bottom - pRequest->lpSourceRect->top;
	buffer[0].rcSource = *pRequest->lpSourceRect;
	if (pRequest->lpTargetRect)
		memcpy(&buffer[0].rcTarget, pRequest->lpTargetRect, sizeof(RECT));
	if (pRequest->pVideoProperty)
	{
		buffer[0].rtStart = pRequest->pVideoProperty->rtStart;
		buffer[0].rtEnd = pRequest->pVideoProperty->rtEnd;
		buffer[0].bInterlaced = pRequest->pVideoProperty->Format != VIDEO_FORMAT_PROGRESSIVE;
		buffer[0].bBottomFieldFirst = pRequest->pVideoProperty->Format == VIDEO_FORMAT_FIELD_INTERLEAVED_ODD_FIRST;
		buffer[0].bSelectSecondField = pRequest->pVideoProperty->dwFieldSelect == FIELD_SELECT_SECOND;
	}

	for (EffectList::const_iterator i = m_listEffect.begin(); i != m_listEffect.end(); ++i)
	{
		if (!i->bEnable)
			continue;

		std::swap(pInputBuffer, pOutputBuffer);
		if (!pInputBuffer->pSurface)
		{
			hr = E_UNEXPECTED;
			break;
		}

		ZeroMemory(pOutputBuffer, sizeof(VE3DBuffer));
		hr = i->pInstance->ProcessEffect(pInputBuffer, pOutputBuffer);
		if (FAILED(hr))
			break;
	}

	if (SUCCEEDED(hr))
	{
		ASSERT(pOutputBuffer->pSurface);
		hr = pOutputBuffer->pSurface->QueryInterface(IID_IUnknown, (void **) pRequest->ppOutput);
		CopyRect(pRequest->lpSourceRect, &pOutputBuffer->rcSource);
	}
	else
		*pRequest->ppOutput = NULL;
	return hr;
}
