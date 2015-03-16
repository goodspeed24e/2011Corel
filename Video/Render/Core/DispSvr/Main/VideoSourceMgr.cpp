#include "stdafx.h"
#include "VideoSourceMgr.h"
#include "VideoSource.h"
#include "VideoSourceEvr.h"

CVideoSourceManager::CVideoSourceManager(IDisplayLock* pLock, IDisplayVideoSink* pVideoSink) :
CUnknown(NAME("CVideoSourceManager"), NULL),
m_pLock(pLock),
m_pVideoSink(pVideoSink)
{
}

STDMETHODIMP CVideoSourceManager::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
	*ppv = NULL;

	if (riid == IID_IDisplayVideoSourceManager)
	{
		return GetInterface((CVideoSourceManager*) this, ppv);
	}
	else
	{
		return CUnknown::NonDelegatingQueryInterface(riid, ppv);
	}
}

CVideoSourceManager::~CVideoSourceManager()
{
	Cleanup();
}

void CVideoSourceManager::Cleanup()
{
	while (false == m_videoSources.empty())
	{
		IDisplayVideoSource *pSource = *m_videoSources.begin();
		if (pSource)
		{
			HRESULT hr = RemoveVideoSource(pSource);
			DbgMsg("Clean_: detaching %ld, return code 0x%08x", pSource, hr);
		}
	}
}

STDMETHODIMP CVideoSourceManager::AddVideoSource(IBaseFilter* pVMR, IDisplayVideoSource** ppVidSrc)
{
	HRESULT hr = S_OK;

	// check that pointers are valid
	if (!pVMR)
	{
		DbgMsg("CVideoSourceManager::Attach received NULL pointer");
		return E_POINTER;
	}

	IDisplayVideoSource* pVideoSource = NULL;
	try
	{
		CComQIPtr<IMFVideoRenderer> pEvr = pVMR;

		if (pEvr)
			pVideoSource = new CVideoSourceEvr(m_pLock, m_pVideoSink);
		else
			pVideoSource = new CVideoSource(m_pLock, m_pVideoSink);

		if (!pVideoSource)
			return E_OUTOFMEMORY;

		hr = pVideoSource->Attach(pVMR);
		CHECK_HR(hr, DbgMsg("CVideoSourceManager::Attach: unable to attach VMR/EVR"));

		// we successfully attached subgraph, last thing left is to save
		// pVideoSource in the list
		pVideoSource->AddRef();
		m_videoSources.push_back(pVideoSource);
		if (ppVidSrc)
		{
			(*ppVidSrc = pVideoSource)->AddRef();
		}
	}
	catch (HRESULT hrFailed)
	{
		hr = hrFailed;
		delete pVideoSource;
	}

	return hr;
}

STDMETHODIMP CVideoSourceManager::RemoveVideoSource(IDisplayVideoSource* pVideoSource)
{
	HRESULT hr = E_FAIL;
	try
	{
		hr = pVideoSource->Detach();
		CHECK_HR(hr, DbgMsg("CVideoSourceManager::Detach: cannot detach filter graph, hr = 0x%08x", hr));

		// we have to be thread safe only here when we actually mess up with shared data
		CAutoDisplayLock displayLock(m_pLock);

		// we unadvised custom allocator-presenter successfully, let's delete
		// video source structure from the list
		VideoSources::iterator it = m_videoSources.begin();
		VideoSources::iterator end = m_videoSources.end();

		bool bSourceWasDeleted = false;
		for (; it != end; ++it)
		{
			IDisplayVideoSource* pCurSource = (IDisplayVideoSource*)(*it);

			hr = (NULL == pCurSource) ? E_UNEXPECTED : S_OK;
			CHECK_HR(hr, DbgMsg("CVideoSourceManager::Detach: FATAL, m_videoSources contains NULL pointer"));

			if (pVideoSource == (IDisplayVideoSource*) pCurSource)
			{
				m_videoSources.erase(it);
				pCurSource->Release();
				pCurSource = NULL;
				bSourceWasDeleted = true;
				break;
			}
		}

		hr = (false == bSourceWasDeleted) ? VFW_E_NOT_FOUND : S_OK;
		CHECK_HR(hr, DbgMsg("CVideoSourceManager::Detach: FATAL, failed to delete source from the list (source was not found)"));
	}
	catch (HRESULT hrFailed)
	{
		hr = hrFailed;
	}

	return hr;
}

STDMETHODIMP CVideoSourceManager::GetVideoSourceCount(LONG* plCount)
{
	if (!plCount)
	{
		DbgMsg("CVideoSourceManager::GetVideoSourceCount: plCount is NULL");
		return E_POINTER;
	}

	*plCount = m_videoSources.size();
	return S_OK;
}

STDMETHODIMP CVideoSourceManager::GetVideoSourceByIndex(
	LONG lIndex,
	IDisplayVideoSource** ppVideoSource)
{
	if (!ppVideoSource)
	{
		DbgMsg("CVideoSourceManager::GetVideoSourceByIndex: ppVideoSource is NULL");
		return E_POINTER;
	}
	if (lIndex < 0 || (size_t) lIndex > m_videoSources.size())
	{
		DbgMsg("CVideoSourceManager::GetVideoSourceByIndex: invalid lIndex");
		return E_INVALIDARG;
	}

	*ppVideoSource = m_videoSources[lIndex];
	(*ppVideoSource)->AddRef();

	return S_OK;
}

STDMETHODIMP CVideoSourceManager::BeginDeviceLoss()
{
	// go through all the connected sources and reset the device
	for (VideoSources::iterator it = m_videoSources.begin();
		it != m_videoSources.end(); ++it)
	{
		ASSERT(*it);
		(*it)->BeginDeviceLoss();
	}
	return S_OK;
}

STDMETHODIMP CVideoSourceManager::EndDeviceLoss(IUnknown* pDevice)
{
	HRESULT hr = S_OK;

	if (!pDevice)
		return E_POINTER;

	for (VideoSources::iterator it = m_videoSources.begin();
		it != m_videoSources.end();
		++it)
	{
		ASSERT(*it);
		hr = (*it)->EndDeviceLoss(pDevice);
	}
	return hr;
}
