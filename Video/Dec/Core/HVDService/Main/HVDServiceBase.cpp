#include "stdafx.h"
#include <windows.h>
#include <streams.h>
#include <d3d9.h>
#include "Imports/LibGPU/UtilGPU.h"
#include "HVDServiceBase.h"

using namespace HVDService;

CHVDServiceBase::CHVDServiceBase() : m_cRef(0)
{
	m_dwVendorID = 0;
	m_dwDeviceID = 0;
	m_dwInitFlags = 0;
	m_dwService = HVD_SERVICE_UNKNOWN;
	m_dwHVDMode = HVD_MODE_UNKNOWN;
	m_dwSurfaceCount = 0;
	m_DecoderGuid = GUID_NULL;
	m_dwSupportedDecoderCount = 0;
	m_pSupportedDecoderGuids = 0;
	ZeroMemory(&m_FastCopyConfig, sizeof(m_FastCopyConfig));
	ZeroMemory(&m_HVDDecodeConfig, sizeof(HVDDecodeConfig));
	m_HVDDecodeConfig.dwLevel = HVD_LEVEL_UNKNOWN;
	m_HVDDecodeConfig.dwMode = HVD_MODE_UNKNOWN;
}

CHVDServiceBase::~CHVDServiceBase()
{
	ZeroMemory(&m_HVDDecodeConfig, sizeof(HVDDecodeConfig));
	m_HVDDecodeConfig.dwLevel = HVD_LEVEL_UNKNOWN;
	m_HVDDecodeConfig.dwMode = HVD_MODE_UNKNOWN;
}

STDMETHODIMP CHVDServiceBase::QueryInterface(REFIID riid, void **ppv)
{
	HRESULT hr = E_NOINTERFACE;

	if (riid == __uuidof(IHVDService))
	{
		hr = GetInterface((IHVDService *)this, ppv);
	}
	else if (riid == _uuidof(IUnknown))
	{
		hr = GetInterface((IUnknown *)this, ppv);
	}
	return hr;
}

STDMETHODIMP_(ULONG) CHVDServiceBase::AddRef()
{
	LONG lRef = InterlockedIncrement(&m_cRef);
	ASSERT(lRef > 0);
	return lRef;
}

STDMETHODIMP_(ULONG) CHVDServiceBase::Release()
{
	LONG lRef = InterlockedDecrement(&m_cRef);
	ASSERT(lRef >= 0);
	if (lRef == 0)
		delete this;
	return lRef;
}

STDMETHODIMP CHVDServiceBase::Initialize(HVDInitConfig* pInitConfig)
{
	CHECK_POINTER(pInitConfig);

	CDisplayDevItem DevItem;
	ZeroMemory(&DevItem, sizeof(CDisplayDevItem));
	if (CUtilGPU::GetDisplayDeviceType(TEXT(""), DevItem) >= 0)
	{
		m_dwVendorID = DevItem.m_VendorId;
		m_dwDeviceID = DevItem.m_DeviceId;
	}
	else
	{
		ASSERT(0 && "Fail to get vendor/device ID");
	}
	m_dwInitFlags = pInitConfig->dwFlags;
	if(pInitConfig->pHVDInitFastCopyConfig)
		memcpy(&m_FastCopyConfig, pInitConfig->pHVDInitFastCopyConfig, sizeof(m_FastCopyConfig));

	HRESULT hr = _Initialize(pInitConfig);
	return hr;
}

STDMETHODIMP CHVDServiceBase::Uninitialize()
{
	_StopService();
	return _Uninitialize();
}

STDMETHODIMP CHVDServiceBase::StartService(HVDDecodeConfig* pDecodeConfig)
{
	CHECK_POINTER(pDecodeConfig);
	_StopService();
	memcpy(&m_HVDDecodeConfig, pDecodeConfig, sizeof(HVDDecodeConfig));
	return _StartService();
}

STDMETHODIMP CHVDServiceBase::GetDecoderGuid(GUID* pGuid)
{
	CHECK_POINTER(pGuid);
	*pGuid = m_DecoderGuid;
	return S_OK;
}

STDMETHODIMP CHVDServiceBase::GetSupportedDecoderCount(UINT* pnCount)
{
	CHECK_POINTER(pnCount);
	*pnCount = m_dwSupportedDecoderCount;
	return S_OK;
}

STDMETHODIMP CHVDServiceBase::GetSupportedDecoderGUIDs(GUID* pGuids)
{
	CHECK_POINTER(pGuids);
	for (DWORD i=0; i<m_dwSupportedDecoderCount; i++)
	{
		pGuids[i] = m_pSupportedDecoderGuids[i];
	}
	return S_OK;
}

STDMETHODIMP CHVDServiceBase::AdviseEventNotify(IHVDServiceNotify* pNotify)
{
	CHECK_POINTER(pNotify);
	CAutoLock lock(&m_csObj);
	m_EventNotify.push_back(pNotify);

	return S_OK;
}

STDMETHODIMP CHVDServiceBase::UnadviseEventNotify(IHVDServiceNotify* pNotify)
{
	CHECK_POINTER(pNotify);
	CAutoLock lock(&m_csObj);
	HVDServiceEventNotify::iterator it = m_EventNotify.begin();
	for (; it != m_EventNotify.end(); ++it)
	{
		if ((*it) == pNotify)
		{
			m_EventNotify.erase(it);
			return S_OK;
		}
	}

	return E_INVALIDARG;
}

STDMETHODIMP CHVDServiceBase::GetHVDDecodeConfig(HVDDecodeConfig* pDecodeConfig)
{
	CHECK_POINTER(pDecodeConfig);	
	memcpy(pDecodeConfig, &m_HVDDecodeConfig, sizeof(HVDDecodeConfig));
	return S_OK;
}

HRESULT CHVDServiceBase::_Initialize(HVDInitConfig* pInitConfig)
{
	return E_NOTIMPL;
}

HRESULT CHVDServiceBase::_Uninitialize()
{
	return E_NOTIMPL;
}

HRESULT CHVDServiceBase::_StartService()
{
	_NotifyEvent(HVD_EVENT_SERVICE_START, 0, 0);
	return S_OK;
}

HRESULT CHVDServiceBase::_StopService()
{
	_NotifyEvent(HVD_EVENT_SERVICE_STOP, 0, 0);
	return S_OK;
}

HRESULT CHVDServiceBase::_NotifyEvent(DWORD dwEvent, DWORD dwParam1, DWORD dwParam2)
{
	HRESULT hr = S_OK;

	HVDServiceEventNotify::iterator it = m_EventNotify.begin();
	for (; it != m_EventNotify.end(); ++it)
	{
		hr |= (*it)->OnNotify((DWORD)dwEvent, dwParam1, dwParam2);
	}
	return hr;
}
