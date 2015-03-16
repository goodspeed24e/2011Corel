#include "stdafx.h"
#include <mfapi.h>
#include <MFerror.h>
#include "EvrCustomMixer.h"


//#define CM_DP(fmt, ...)	DbgMsg(("CEvrCustomMixer::" fmt, __VA_ARGS__))
#ifndef CM_DP
#	define CM_DP(...)	do {} while(0);
#endif

static const GUID s_pAvailableInputTypes[] =
{
	MFVideoFormat_NV12,
	MFVideoFormat_YV12,
	MFVideoFormat_YUY2,
	MFVideoFormat_UYVY
};
static const int s_nAvailableInputTypes = sizeof(s_pAvailableInputTypes) / sizeof(GUID);

static const GUID s_pAvailableOutputTypes[] =
{
	MFVideoFormat_RGB32
};
static const int s_nAvailableOutputTypes = sizeof(s_pAvailableOutputTypes) / sizeof(GUID);

static void DumpMediaType(IMFMediaType *pType)
{
#ifdef _DEBUG
	if (!pType)
		return;

	void *pRepresentation = 0;
	HRESULT hr = pType->GetRepresentation(FORMAT_VideoInfo2, &pRepresentation);
	if (SUCCEEDED(hr))
	{
		AM_MEDIA_TYPE *pAmType = (AM_MEDIA_TYPE *)pRepresentation;
		VIDEOINFOHEADER2 *pInfo = (VIDEOINFOHEADER2 *)pAmType->pbFormat;

		pType->FreeRepresentation(FORMAT_VideoInfo2, pRepresentation);
	}
#endif
}

using namespace DispSvr;

CEvrCustomMixer::CEvrCustomMixer() :
	m_cRef(0),
	m_nStreams(1)	// at least one video stream is supported if no SetNumberOfStream() is called.
{
	ZeroMemory(m_StreamIDs, sizeof(m_StreamIDs));
	ZeroMemory(m_pInputTypes, sizeof(m_pInputTypes));
	ZeroMemory(m_pSamples, sizeof(m_pSamples));
	m_StreamIDs[0] = 0;
	m_pOutputType = NULL;
}

CEvrCustomMixer::~CEvrCustomMixer()
{
	SAFE_RELEASE(m_pOutputType);
}

STDMETHODIMP_(ULONG) CEvrCustomMixer::AddRef()
{
	LONG lRef = InterlockedIncrement(&m_cRef);
	ASSERT(lRef > 0);
	return lRef;
}

STDMETHODIMP_(ULONG) CEvrCustomMixer::Release()
{
	LONG lRef = InterlockedDecrement(&m_cRef);
	ASSERT(lRef >= 0);
	if (lRef == 0)
		delete this;
	return lRef;
}

STDMETHODIMP CEvrCustomMixer::QueryInterface(REFIID riid, void **ppv)
{
	HRESULT hr = E_NOINTERFACE;
	*ppv = NULL;

	if (riid == IID_IMFTransform)
	{
		hr = GetInterface((IMFTransform *)this, ppv);
	}
	else if (riid == IID_IMFTopologyServiceLookupClient)
	{
		hr = GetInterface((IMFTopologyServiceLookupClient *)this, ppv);
	}
	else if (riid == IID_IMFVideoDeviceID)
	{
		hr = GetInterface((IMFVideoDeviceID *)this, ppv);
	}
	else if (riid == IID_IUnknown)
	{
		hr = GetInterface((IUnknown *) (IMFTransform *)this, ppv);
	}
	return hr;
}

// IMFVideoDeviceID
STDMETHODIMP CEvrCustomMixer::GetDeviceID(IID* pDeviceID)
{
	if (pDeviceID == NULL)
		return E_POINTER;
	*pDeviceID = __uuidof(IDirect3DDevice9);
	return S_OK;
}

// IMFTransform
STDMETHODIMP CEvrCustomMixer::GetStreamLimits(DWORD *pdwInputMinimum, DWORD *pdwInputMaximum, DWORD *pdwOutputMinimum, DWORD *pdwOutputMaximum)
{
	return E_NOTIMPL;
}

STDMETHODIMP CEvrCustomMixer::GetStreamCount(DWORD *pcInputStreams, DWORD *pcOutputStreams)
{
	if (pcInputStreams)
		*pcInputStreams = m_nStreams;
	if (pcOutputStreams)
		*pcOutputStreams = 1;
	return S_OK;
}

STDMETHODIMP CEvrCustomMixer::GetStreamIDs(DWORD dwInputIDArraySize, DWORD *pdwInputIDs, DWORD dwOutputIDArraySize, DWORD *pdwOutputIDs)
{
	int max = min(m_nStreams, dwInputIDArraySize);
	for (int i = 0; i < max; i++)
		pdwInputIDs[i] = i;

	if (dwOutputIDArraySize > 0)
		pdwOutputIDs[0] = 0;
	return S_OK;
}

STDMETHODIMP CEvrCustomMixer::GetInputStreamInfo(DWORD dwInputStreamID, MFT_INPUT_STREAM_INFO *pStreamInfo)
{
	HRESULT hr;

	hr = VerifyInputStreamID(dwInputStreamID);
	if (FAILED(hr))
		return hr;

	pStreamInfo->hnsMaxLatency = 0;
	pStreamInfo->dwFlags =
		MFT_INPUT_STREAM_WHOLE_SAMPLES
		| MFT_INPUT_STREAM_SINGLE_SAMPLE_PER_BUFFER; //MFT_INPUT_STREAM_OPTIONAL ;//
	pStreamInfo->cbSize = 0;	//m_cbImageSize;
	pStreamInfo->cbMaxLookahead = 0;
	pStreamInfo->cbAlignment = 0;

	return hr;
}

STDMETHODIMP CEvrCustomMixer::GetOutputStreamInfo(DWORD dwOutputStreamID, MFT_OUTPUT_STREAM_INFO *  pStreamInfo)
{
	// only one output stream to the presenter
	if (dwOutputStreamID != 0)
		return MF_E_INVALIDSTREAMNUMBER;

	CAutoLock __lock(&m_csMixerLock);
	if (m_pOutputType == NULL)
	{
		return MF_E_TRANSFORM_TYPE_NOT_SET;
	}

	pStreamInfo->dwFlags = 
		MFT_OUTPUT_STREAM_WHOLE_SAMPLES
		| MFT_OUTPUT_STREAM_SINGLE_SAMPLE_PER_BUFFER
		| MFT_OUTPUT_STREAM_FIXED_SAMPLE_SIZE;
	pStreamInfo->cbSize = 0;
	pStreamInfo->cbAlignment = 0;

	return S_OK;
}

STDMETHODIMP CEvrCustomMixer::GetAttributes(IMFAttributes** pAttributes)
{
	return E_NOTIMPL;
}

STDMETHODIMP CEvrCustomMixer::GetInputStreamAttributes(DWORD dwInputStreamID, IMFAttributes **ppAttributes)
{
	HRESULT hr;

	hr = VerifyInputStreamID(dwInputStreamID);
	if (FAILED(hr))
		return hr;

	CAutoLock __lock(&m_csMixerLock);
	if (m_pInputTypes[dwInputStreamID] == 0)
		return MF_E_TRANSFORM_TYPE_NOT_SET;

	m_pInputTypes[dwInputStreamID]->SetUINT32(MF_SA_REQUIRED_SAMPLE_COUNT ,1);
	m_pInputTypes[dwInputStreamID]->AddRef();
	*ppAttributes = m_pInputTypes[dwInputStreamID];
	return hr;
}

STDMETHODIMP CEvrCustomMixer::GetOutputStreamAttributes(DWORD dwOutputStreamID, IMFAttributes **ppAttributes)
{
	HRESULT hr;
	UINT uCount = 0;

	hr = VerifyOutputStreamID(dwOutputStreamID);
	if (FAILED(hr))
		return hr;

	CAutoLock __lock(&m_csMixerLock);
	if (m_pOutputType == 0)
		return MF_E_TRANSFORM_TYPE_NOT_SET;

	hr = m_pOutputType->GetCount(&uCount);
	if (FAILED(hr))
		return hr;
	if (uCount == 0)
	{
		 hr = m_pOutputType->SetUINT32(MF_SA_REQUIRED_SAMPLE_COUNT ,1);
		 if (FAILED(hr))
			 return hr;
	}
	m_pOutputType->AddRef();
	*ppAttributes = m_pOutputType;
	return hr;
}

STDMETHODIMP CEvrCustomMixer::DeleteInputStream(DWORD dwStreamID)
{
	return E_NOTIMPL;
}

STDMETHODIMP CEvrCustomMixer::AddInputStreams(DWORD cStreams, DWORD *adwStreamIDs)
{
	CAutoLock __lock(&m_csMixerLock);
	if(m_nStreams + cStreams > MAX_STREAMS)
		return E_INVALIDARG;

	for(DWORD i = 0; i < cStreams; i++)
		m_StreamIDs[i + m_nStreams] = adwStreamIDs[i];

	m_nStreams += cStreams;
	return S_OK;
}

STDMETHODIMP CEvrCustomMixer::GetInputAvailableType(DWORD dwInputStreamID,
								   DWORD dwTypeIndex, // 0-based
								   IMFMediaType **ppType)
{
	HRESULT hr;

	if (ppType == 0)
		return E_INVALIDARG;

	hr = VerifyInputStreamID(dwInputStreamID);
	if (SUCCEEDED(hr))
	{
		CAutoLock __lock(&m_csMixerLock);
		hr = GetInputAvailableType(dwTypeIndex, ppType);
	}
	return hr;
}

STDMETHODIMP CEvrCustomMixer::GetOutputAvailableType(DWORD dwOutputStreamID,
									DWORD dwTypeIndex, // 0-based
									IMFMediaType **ppType)
{
	HRESULT hr;

	hr = VerifyOutputStreamID(dwOutputStreamID);
	if (FAILED(hr))
		return hr;

	if (ppType == 0)
		return E_INVALIDARG;

	CAutoLock __lock(&m_csMixerLock);
	hr = GetOutputAvailableType(dwTypeIndex, ppType);
	return hr;
}

STDMETHODIMP CEvrCustomMixer::SetInputType(DWORD dwInputStreamID, IMFMediaType *pType, DWORD dwFlags)
{
	HRESULT hr;

	CM_DP("SetInputType[%d] MediaType: %x, dwFlags: %x", dwInputStreamID, pType, dwFlags);
	DumpMediaType(pType);

	hr = VerifyInputStreamID(dwInputStreamID);
	if (FAILED(hr))
		return hr;

	CAutoLock __lock(&m_csMixerLock);
	// If we have an input sample, the client cannot change the type now.
	if (m_pSamples[0] != 0)
		return MF_E_INVALIDMEDIATYPE;

	if (pType)
	{
		hr = CheckInputMediaType(pType);
		if (FAILED(hr))
			return hr;
	}

	if ((dwFlags & MFT_SET_TYPE_TEST_ONLY) == 0)
	{
		SAFE_RELEASE(m_pInputTypes[dwInputStreamID]);
		if (pType)
		{
			pType->AddRef();
			m_pInputTypes[dwInputStreamID] = pType;
		}
	}
	return hr;
}

STDMETHODIMP CEvrCustomMixer::SetOutputType(DWORD dwOutputStreamID, IMFMediaType *pType, DWORD dwFlags)
{
	HRESULT hr;

	CM_DP("SetOutputType[%d] MediaType: %x, dwFlags: %x", dwOutputStreamID, pType, dwFlags);
	DumpMediaType(pType);

	hr = VerifyOutputStreamID(dwOutputStreamID);
	if (FAILED(hr))
		return hr;

	CAutoLock __lock(&m_csMixerLock);
	// If we have an input sample, the client cannot change the type now.
	if (m_pSamples[0] != 0)
		return MF_E_INVALIDMEDIATYPE;

	if (pType)
	{
		hr = CheckOutputMediaType(pType);
		if (FAILED(hr))
			return hr;
	}

	if ((dwFlags & MFT_SET_TYPE_TEST_ONLY) == 0)
	{
		SAFE_RELEASE(m_pOutputType);
		if (pType)
		{
			pType->AddRef();
			m_pOutputType = pType;
		}
	}
	return hr;
}

STDMETHODIMP CEvrCustomMixer::GetInputCurrentType(DWORD dwInputStreamID, IMFMediaType **ppType)
{
	HRESULT hr;

	hr = VerifyInputStreamID(dwInputStreamID);
	if (ppType == 0)
		return E_INVALIDARG;

	CAutoLock __lock(&m_csMixerLock);
	if (m_pInputTypes[dwInputStreamID] == 0)
		return MF_E_TRANSFORM_TYPE_NOT_SET;

	m_pInputTypes[dwInputStreamID]->AddRef();
	*ppType = m_pInputTypes[dwInputStreamID];
	return hr;
}

STDMETHODIMP CEvrCustomMixer::GetOutputCurrentType(DWORD dwOutputStreamID, IMFMediaType **ppType)
{
	HRESULT hr;

	hr = VerifyOutputStreamID(dwOutputStreamID);
	if (ppType == 0)
		return E_INVALIDARG;

	CAutoLock __lock(&m_csMixerLock);
	if (m_pOutputType == 0)
		return MF_E_TRANSFORM_TYPE_NOT_SET;

	m_pOutputType->AddRef();
	*ppType = m_pOutputType;
	return hr;
}

STDMETHODIMP CEvrCustomMixer::GetInputStatus(DWORD dwInputStreamID, DWORD *pdwFlags)
{
	HRESULT hr;

	if (pdwFlags == 0)
		return E_INVALIDARG;

	hr = VerifyInputStreamID(dwInputStreamID);
	if (FAILED(hr))
		return hr;

	CAutoLock __lock(&m_csMixerLock);
	if (m_pSamples[dwInputStreamID] == NULL)
		*pdwFlags = MFT_INPUT_STATUS_ACCEPT_DATA;
	else
		*pdwFlags = 0;
	return hr;
}

STDMETHODIMP CEvrCustomMixer::GetOutputStatus(DWORD *pdwFlags)
{
	if (pdwFlags == 0)
		return E_INVALIDARG;

	CAutoLock __lock(&m_csMixerLock);
	if (m_pSamples[0] != NULL)
		*pdwFlags = MFT_OUTPUT_STATUS_SAMPLE_READY;
	else
		*pdwFlags = 0;

	return S_OK;
}

STDMETHODIMP CEvrCustomMixer::SetOutputBounds(LONGLONG hnsLowerBound, LONGLONG hnsUpperBound)
{
	return E_NOTIMPL;
}

STDMETHODIMP CEvrCustomMixer::ProcessEvent(DWORD dwInputStreamID, IMFMediaEvent *pEvent)
{
	return E_NOTIMPL;
}

STDMETHODIMP CEvrCustomMixer::ProcessMessage(MFT_MESSAGE_TYPE eMessage, ULONG_PTR ulParam)
{
	HRESULT hr = S_OK;

	CM_DP("ProcessMessage message = %x, param = %x", eMessage, ulParam);
	switch (eMessage)
	{
	case MFT_MESSAGE_NOTIFY_BEGIN_STREAMING:
		break;
	case MFT_MESSAGE_COMMAND_FLUSH:
		hr = OnFlush();
		break;
	default:
		;
	}
	return hr;
}

// ProcessInput is called by the EVR
STDMETHODIMP CEvrCustomMixer::ProcessInput(DWORD dwInputStreamID, IMFSample *pSample, DWORD dwFlags)
{
	HRESULT hr;

	CM_DP("ProcessInput: ID: %d, pSample: %x, dwFlags: %x", dwInputStreamID, pSample, dwFlags);
	if (pSample == 0 || dwFlags != 0)
		return E_INVALIDARG;

	hr = VerifyInputStreamID(dwInputStreamID);
	if (FAILED(hr))
		return hr;

	CAutoLock __lock(&m_csMixerLock);
	if (!m_pInputTypes[dwInputStreamID] || !m_pOutputType)
		return MF_E_TRANSFORM_TYPE_NOT_SET;

	// we already have a input sample
	if (m_pSamples[dwInputStreamID])
		return MF_E_NOTACCEPTING;

	pSample->AddRef();
	m_pSamples[dwInputStreamID] = pSample;

	return hr;
}

// ProcessOutput is called by the presenter
STDMETHODIMP CEvrCustomMixer::ProcessOutput(
	DWORD dwFlags,
	DWORD cOutputBufferCount,
	MFT_OUTPUT_DATA_BUFFER *pOutputSamples, // one per stream
	DWORD *pdwStatus)
{
	if (dwFlags != 0 || pOutputSamples == 0 || pdwStatus == 0)
		return E_INVALIDARG;

	CM_DP("ProcessOutput: cOutputBufferCount: %d, pOutputSamples: %x, dwFlags: %x", cOutputBufferCount, pOutputSamples, dwFlags);
	CAutoLock __lock(&m_csMixerLock);
	// only process when main video sample is pending.
	if (m_pSamples[0] == 0)
		return MF_E_TRANSFORM_NEED_MORE_INPUT;

	DWORD i = 0;
	for (; i < cOutputBufferCount && i < m_nStreams; i++)
	{
		pOutputSamples[i].pSample = m_pSamples[i];
		pOutputSamples[i].pSample->AddRef();
		pOutputSamples[i].dwStatus = 0;
	}

	if (pOutputSamples[0].pSample != 0)
		*pdwStatus = 0;

	for (i = 0; i < m_nStreams; i++)
		SAFE_RELEASE(m_pSamples[i]);

	m_pMediaEventSink->Notify(EC_SAMPLE_NEEDED, m_StreamIDs[0], 0);
	return S_OK;
}

// IMFTopologyServiceLookupClient
STDMETHODIMP CEvrCustomMixer::InitServicePointers(IMFTopologyServiceLookup* pLookup)
{
	HRESULT hr = E_POINTER;

	if (!pLookup)
		return hr;

	MF_SERVICE_LOOKUP_TYPE eType = MF_SERVICE_LOOKUP_ALL;	// type is currently ignored
	DWORD nObjects = 1;		// 1 is the current implementation of LookupService

	ASSERT(!m_pMediaEventSink);
	hr = pLookup->LookupService(eType, 0, MR_VIDEO_RENDER_SERVICE, __uuidof(IMediaEventSink), (LPVOID *)&m_pMediaEventSink, &nObjects);
	if (FAILED(hr))
		return hr;
	return hr;
}

STDMETHODIMP CEvrCustomMixer::ReleaseServicePointers()
{
	m_pMediaEventSink.Release();
	return S_OK;
}

HRESULT CEvrCustomMixer::VerifyInputStreamID(DWORD dwID)
{
	for (DWORD i = 0; i < m_nStreams; i++)
		if (dwID == m_StreamIDs[i])
			return S_OK;
	return MF_E_INVALIDSTREAMNUMBER;
}

HRESULT CEvrCustomMixer::VerifyOutputStreamID(DWORD dwID)
{
	// there is only one output stream to the presenter
	if (dwID == 0)
		return S_OK;
	return MF_E_INVALIDSTREAMNUMBER;
}

HRESULT CEvrCustomMixer::CheckInputMediaType(IMFMediaType *pType)
{
	HRESULT hr;
	GUID guidMajorType = GUID_NULL;
	GUID guidSubtype = GUID_NULL;

	ASSERT(pType);
	hr = pType->GetGUID(MF_MT_MAJOR_TYPE, &guidMajorType);
	if (SUCCEEDED(hr))
	{
		if (guidMajorType != MFMediaType_Video)
			return MF_E_INVALIDTYPE;

		hr = pType->GetGUID(MF_MT_SUBTYPE, &guidSubtype);
		if (SUCCEEDED(hr))
		{
			for (int i = 0; i < s_nAvailableInputTypes; i++)
			{
				if (s_pAvailableInputTypes[i] == guidSubtype)
					return hr;
			}
			return MF_E_INVALIDTYPE;
		}
	}
	return hr;
}

HRESULT CEvrCustomMixer::CheckOutputMediaType(IMFMediaType *pType)
{
	HRESULT hr;
	GUID guidMajorType = GUID_NULL;
	GUID guidSubtype = GUID_NULL;

	ASSERT(pType);
	hr = pType->GetGUID(MF_MT_MAJOR_TYPE, &guidMajorType);
	if (SUCCEEDED(hr))
	{
		if (guidMajorType != MFMediaType_Video)
			return MF_E_INVALIDTYPE;

		hr = pType->GetGUID(MF_MT_SUBTYPE, &guidSubtype);
		if (SUCCEEDED(hr))
		{
			for (int i = 0; i < s_nAvailableOutputTypes; i++)
			{
				if (s_pAvailableOutputTypes[i] == guidSubtype)
					return hr;
			}
			return MF_E_INVALIDTYPE;
		}
	}
	return hr;
}

HRESULT CEvrCustomMixer::GetInputAvailableType(DWORD dwIndex, IMFMediaType **ppType)
{
	HRESULT hr;
	IMFMediaType *pmt;

	*ppType = 0;
	if (dwIndex >= s_nAvailableInputTypes)
		return MF_E_NO_MORE_TYPES;

	hr = CDynLibManager::GetInstance()->pfnMFCreateMediaType(&pmt);
	if (SUCCEEDED(hr))
	{
		hr = pmt->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
		ASSERT(SUCCEEDED(hr));
		hr = pmt->SetGUID(MF_MT_SUBTYPE, s_pAvailableInputTypes[dwIndex]);
		ASSERT(SUCCEEDED(hr));
		*ppType = pmt;
	}
	return hr;
}

HRESULT CEvrCustomMixer::GetOutputAvailableType(DWORD dwIndex, IMFMediaType **ppType)
{
	HRESULT hr;
	IMFMediaType *pmt;

	*ppType = 0;
	if (dwIndex >= s_nAvailableOutputTypes)
		return MF_E_NO_MORE_TYPES;

	hr = CDynLibManager::GetInstance()->pfnMFCreateMediaType(&pmt);
	if (SUCCEEDED(hr))
	{
		hr = pmt->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
		ASSERT(SUCCEEDED(hr));
		hr = pmt->SetGUID(MF_MT_SUBTYPE, s_pAvailableOutputTypes[dwIndex]);
		ASSERT(SUCCEEDED(hr));
		*ppType = pmt;

		UINT uWidth, uHeight, uNumerator, uDenominator;
		hr = MFGetAttributeSize(m_pInputTypes[0], MF_MT_FRAME_SIZE, &uWidth, &uHeight);
		ASSERT(SUCCEEDED(hr));
		hr = MFGetAttributeRatio(m_pInputTypes[0], MF_MT_PIXEL_ASPECT_RATIO, &uNumerator, &uDenominator);
		ASSERT(SUCCEEDED(hr));
		hr = MFSetAttributeSize(pmt, MF_MT_FRAME_SIZE, uWidth, uHeight);
		ASSERT(SUCCEEDED(hr));
		hr = MFSetAttributeRatio(pmt, MF_MT_PIXEL_ASPECT_RATIO, uNumerator, uDenominator);
		ASSERT(SUCCEEDED(hr));
	}
	return hr;
}

HRESULT CEvrCustomMixer::OnFlush()
{
	CAutoLock __lock(&m_csMixerLock);
	for (DWORD i = 0; i < m_nStreams; i++)
	{
		if (m_pSamples[i])
		{
			m_pSamples[i]->Release();
			m_pSamples[i] = 0;
		}
	}
	return S_OK;
}
