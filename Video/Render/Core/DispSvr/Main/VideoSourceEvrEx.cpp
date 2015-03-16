#include "stdafx.h"
#include <dxva2api.h>
#include <mfapi.h>
#include <MFerror.h>
#include <vector>
#include <deque>
#include "DispSvr.h"
#include "DynLibManager.h"
#include "ServerStateEventSink.h"
#include "EvrCustomMixer.h"
#include "Exports/Inc/VideoMixer.h"
#include "VideoSourceEvrEx.h"

using namespace DispSvr;

//#define VSE_DP(fmt, ...)	DbgMsg("CVideoSourceEvrEx::" fmt, __VA_ARGS__)
#ifndef VSE_DP
#	define VSE_DP(...)	do {} while(0);
#endif


// {EE3AFF17-6499-415f-BDEF-B9F0BA0D1C01}
static const GUID VideoSourceEvrEx_FieldSelect = 
{ 0xee3aff17, 0x6499, 0x415f, { 0xbd, 0xef, 0xb9, 0xf0, 0xba, 0xd, 0x1c, 0x1 } };

CVideoSourceEvrEx::CVideoSourceEvrEx(IDisplayLock* pLock, IDisplayVideoSink* pVideoSink)
{
	m_uWidth = 0L;
	m_uHeight = 0L;
	m_fAspectRatio = m_fPixelAspectRatio = 0.f;

	// Because the life cycles of pLock and pVideoSink are supposed to be longer than
	// video source, we only keep the pointer values without extra references.
	m_pLock = pLock;
	m_pVideoSink = pVideoSink;

	m_bEndOfStreaming = true;
	m_bValid = TRUE;
	m_uDeviceManagerToken = 0;
	m_eThreadStatus = eNotStarted;
	m_dwOutputStreamId = 0;
	m_pVideoSink->QueryInterface(__uuidof(IDisplayObject), (void **)&m_pDispObj);
	m_pDispObj->Release();	// m_pDispObj's life cycle is the same as m_pVideoSink
	m_pStateEventSink = CServerStateEventSink::GetInstance();
	m_ePresenterState = PRESENTER_STATE_SHUTDOWN;
	m_pCustomMixer = 0;
	m_pSample = 0;
	m_hProcessInputNotify = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_cRef = 0;
	m_pBufferAtIndex0 = 0;
	m_hwndVideo = NULL;
	memset(&m_dstrect, 0, sizeof(m_dstrect));
	m_nrcTexture.left = m_nrcTexture.top = 0.0;
	m_nrcTexture.right = m_nrcTexture.bottom = 1.0;
	m_fNativeAspectRatio = 0.0;
	m_dwAspectRatioMode = MFVideoARMode_None;
}

CVideoSourceEvrEx::~CVideoSourceEvrEx()
{
	Cleanup();
	ASSERT(m_pCustomMixer == 0);
}

void CVideoSourceEvrEx::Cleanup()
{
	StopEvrPresentThread();
	ReleaseServicePointers();

	BeginDeviceLoss();
	m_pMediaType.Release();
	m_pGraph.Release();
	m_uDeviceManagerToken = 0;
	m_pDeviceManager.Release();
	IBaseFilter *pFilter = m_pEVR.Detach();
	if (pFilter)
	{
		int ref = pFilter->Release();
		if (ref > 0)
		{
			VSE_DP("Cleanup: m_pEVR ref count = %d, should be 0.\n", ref);
		}
	}

	m_uWidth = m_uHeight = 0L;
}

STDMETHODIMP_(ULONG) CVideoSourceEvrEx::AddRef()
{
	LONG lRef = InterlockedIncrement(&m_cRef);
	ASSERT(lRef > 0);
	return lRef;
}

STDMETHODIMP_(ULONG) CVideoSourceEvrEx::Release()
{
	LONG lRef = InterlockedDecrement(&m_cRef);
	ASSERT(lRef >= 0);
	if (lRef == 0)
		delete this;
	return lRef;
}

STDMETHODIMP CVideoSourceEvrEx::QueryInterface(REFIID riid, void ** ppv)
{
	HRESULT hr = E_NOINTERFACE;
	*ppv = NULL;

	if (riid == __uuidof(IDisplayVideoSource))
	{
		hr = GetInterface((IDisplayVideoSource *) this, ppv);
	}
	else if (riid == __uuidof(IMFGetService))
	{
		hr = GetInterface((IMFGetService *) this, ppv);
	}
	else if (riid == __uuidof(IMFTopologyServiceLookupClient))
	{
		hr = GetInterface((IMFTopologyServiceLookupClient *) this, ppv);
	}
	else if (riid == __uuidof(IMFVideoDeviceID))
	{
		hr = GetInterface((IMFVideoDeviceID *) this, ppv);
	}
	else if (riid == __uuidof(IMFVideoPresenter))
	{
		hr = GetInterface((IMFVideoPresenter *) this, ppv);
	}
	else if (riid == __uuidof(IDisplayVideoSourcePropertyExtension))
	{
		hr = GetInterface((IDisplayVideoSourcePropertyExtension *) this, ppv);
	}
	else if (riid == IID_IUnknown)
	{
		hr = GetInterface((IUnknown *) (IMFGetService *)this, ppv);
	}
	return hr;
}

//////////////////////////////////////////////////////////////////////////
// IDisplayVideoSource

STDMETHODIMP CVideoSourceEvrEx::GetGraph(IFilterGraph** ppGraph)
{
	if (!ppGraph)
	{
		DbgMsg("CVideoSourceEvrExEx::GetGraph: ppGraph is NULL");
		return E_POINTER;
	}

	CAutoLock lockPresent(&m_csEvrPresenting);
	if (m_pGraph)
	{
		m_pGraph.CopyTo(ppGraph);
		return S_OK;
	}

	DbgMsg("CVideoSourceEvrExEx::GetGraph: FATAL: contains NULL IFilterGraph pointer");
	return VFW_E_NOT_FOUND;
}

STDMETHODIMP CVideoSourceEvrEx::GetTexture(IUnknown** ppTexture, NORMALIZEDRECT* lpNormRect)
{
	if (!ppTexture)
		return E_POINTER;

	CAutoLock lock(&m_csEvrPresenting);
	HRESULT hr = E_FAIL;

	*ppTexture = 0;
	if (m_pBufferAtIndex0)
		hr = m_pBufferAtIndex0->QueryInterface(__uuidof(IUnknown), (void **)ppTexture);

	if (lpNormRect)
	{
		*lpNormRect = m_nrcTexture;
	}
	return hr;
}

STDMETHODIMP CVideoSourceEvrEx::IsValid()
{
	return m_bValid ? S_OK : E_FAIL;
}

STDMETHODIMP CVideoSourceEvrEx::ClearImage()
{
	return S_OK;
}

STDMETHODIMP CVideoSourceEvrEx::GetVideoSize(LONG* plWidth, LONG* plHeight, float *pfAspectRatio)
{
	ASSERT(plWidth && plHeight && pfAspectRatio);

	if (m_dwAspectRatioMode == MFVideoARMode_PreservePicture)
		m_fAspectRatio = m_fNativeAspectRatio;
	else if (m_dwAspectRatioMode ==MFVideoARMode_None)
		m_fAspectRatio = 0.0;

	if (m_pMediaType)
	{
		*plWidth = m_uWidth;
		*plHeight = m_uHeight;
		*pfAspectRatio = m_fAspectRatio;
		return S_OK;
	}
	return MF_E_TRANSFORM_TYPE_NOT_SET;
}

HRESULT CVideoSourceEvrEx::BeginDeviceLoss()
{
	CAutoLock lockPresent(&m_csEvrPresenting);
	m_bValid = FALSE;
	if (m_pSample)
	{
		m_pSample->Release();
		m_pSample = 0;
	}
	if (m_pBufferAtIndex0)
	{
		m_pBufferAtIndex0->Release();
		m_pBufferAtIndex0 = 0;
	}

	return S_OK;
}

HRESULT CVideoSourceEvrEx::EndDeviceLoss(IUnknown* pDevice)
{
	HRESULT hr = S_OK;
	if (m_pDeviceManager)
	{
		CComQIPtr<IDirect3DDevice9> pDevice9 = pDevice;
		hr = m_pDeviceManager->ResetDevice(pDevice9, m_uDeviceManagerToken);
		ASSERT(SUCCEEDED(hr));
	}
	if (m_pMediaEventSink)
		m_pMediaEventSink->Notify(EC_DISPLAY_CHANGED, 0, 0);
	return hr;
}

// Disconnects pins of VMR
HRESULT CVideoSourceEvrEx::DisconnectPins()
{
	HRESULT hr = S_OK;
	if (!m_pEVR)
	{
		return E_POINTER;
	}

	try
	{
		CComPtr<IEnumPins> pEnum;
		hr = m_pEVR->EnumPins(&pEnum);
		CHECK_HR(hr, DbgMsg("CVideoSourceEvrEx::DisconnectPins: failed to enumerate pins, hr = 0x%08x", hr));

		CComPtr<IPin> pPin;
		hr = pEnum->Next(1, &pPin, NULL);
		while (S_OK == hr && pPin)
		{
			hr = pPin->Disconnect();
			CHECK_HR(hr, DbgMsg("CVideoSourceEvrEx::DisconnectPins: failed to disconnect pin, hr = 0x%08x", hr));

			pPin.Release();
			hr = pEnum->Next(1, &pPin, NULL);
		}
	}
	catch (HRESULT hrFailed)
	{
		hr = hrFailed;
	}

	return hr;
}

HRESULT CVideoSourceEvrEx::Attach(IBaseFilter* pVMR)
{
	HRESULT hr = S_OK;
	FILTER_INFO fiVMR;
	ZeroMemory(&fiVMR, sizeof(fiVMR));

	try
	{
		CAutoLock lock(&m_csEvrPresenting);
		// check that provided VMR is part of the graph
		m_pEVR = pVMR;
		hr = m_pEVR->QueryFilterInfo(&fiVMR);
		hr = (NULL == fiVMR.pGraph) ? E_FAIL : S_OK;
		CHECK_HR(hr, DbgMsg("CVideoSourceEvrEx::Attach: provided VMR was not added to the graph"));

		m_pGraph = fiVMR.pGraph;

		CComPtr<IMediaControl> pMediaControl;
		hr = m_pGraph->QueryInterface(__uuidof(IMediaControl), (void**) & pMediaControl);
		CHECK_HR(hr, DbgMsg("CVideoSourceEvrEx::Attach: cannot QI IMediaControl"));

		OAFilterState state;
		hr = pMediaControl->GetState(100, &state);
		CHECK_HR(hr, DbgMsg("CVideoSourceEvrEx::Attach: failed to get state of IMediaControl, hr = 0x%08x", hr));

		hr = (state != State_Stopped) ? VFW_E_NOT_STOPPED : S_OK;
		CHECK_HR(hr, DbgMsg("CVideoSourceEvrEx::Attach: graph is not stopped, state = %ld", state));

		CComPtr<IMFVideoRenderer> pEvr;
		hr = m_pEVR->QueryInterface(__uuidof(IMFVideoRenderer), (void **)&pEvr);
		CHECK_HR(hr, DbgMsg("CVideoSourceEvrEx::Attach: filter is not an EVR"));

		CAutoDisplayLock displayLock(m_pLock);

		CComPtr<IDirect3DDevice9> pDevice;
		hr = m_pVideoSink->Get3DDevice((IUnknown**)&pDevice);
		CHECK_HR(hr, DbgMsg("CVideoSource::Attach: failed to obtain Direct3D device from the video sink, hr = 0x%08x", hr));

		if (!CDynLibManager::GetInstance()->pfnDXVA2CreateDirect3DDeviceManager9)
			hr = E_FAIL;
		CHECK_HR(hr, DbgMsg("CVideoSource::Attach: failed to link evr.dll, mfplat.dll or dxva2.dll"));

		if (SUCCEEDED(hr))
			hr = CDynLibManager::GetInstance()->pfnDXVA2CreateDirect3DDeviceManager9(&m_uDeviceManagerToken, &m_pDeviceManager);
		if (SUCCEEDED(hr))
			hr = m_pDeviceManager->ResetDevice(pDevice, m_uDeviceManagerToken);
		CHECK_HR(hr, DbgMsg("CVideoSourceEvrEx::Attach: unable to create device manager"));

		m_pCustomMixer = new CEvrCustomMixer;
		ASSERT(m_pCustomMixer);
		m_pCustomMixer->AddRef();

		hr = pEvr->InitializeRenderer(m_pCustomMixer, this);
		CHECK_HR(hr, DbgMsg("CVideoSourceEvrEx::Attach: IMFVideoRenderer::InitializeRenderer failed. hr=0x%x", hr));

		hr = StartEvrPresentThread();
		CHECK_HR(hr, DbgMsg("CVideoSourceEvrEx::Attach: unable to start present thread."));

		hr = m_pVideoSink->OnVideoSourceAdded(this,	m_fAspectRatio);
		CHECK_HR(hr, DbgMsg("CVideoSourceEvrEx::Attach: failed in IDisplayVideoSink::AddVideoSource(), hr = 0x%08x", hr));

		m_pStateEventSink->Notify(SERVER_STATE_VIDEO_SOURCE_ADD, reinterpret_cast<DWORD> (this), 0);
	}
	catch (HRESULT hrFailed)
	{
		Detach();
		hr = hrFailed;
	}

	SAFE_RELEASE(fiVMR.pGraph);

	return hr;
}

HRESULT CVideoSourceEvrEx::Detach()
{
	HRESULT hr = S_OK;

	{
		CAutoLock lock(&m_csEvrPresenting);
		if (!m_pVideoSink)
		{
			DbgMsg("CVideoSourceEvrEx::Detach: FATAL IDisplayVideoSink pointer is NULL!");
			return E_UNEXPECTED;
		}

		if (!m_pGraph)
		{
			DbgMsg("CVideoSourceEvrEx::Detach: video source info does not contain pointer to IFilterGraph!");
			return VFW_E_NOT_FOUND;
		}

		CComPtr<IMediaControl> pMC;
		hr = m_pGraph->QueryInterface(__uuidof(IMediaControl), (void**) & pMC);
		if (FAILED(hr))
		{
			DbgMsg("CVideoSourceEvrEx::Detach: cannot QI IMediaControl of the graph, hr = 0x%08x", hr);
			return hr;
		}

		OAFilterState state;
		hr = pMC->GetState(100, &state);
		if (FAILED(hr))
		{
			DbgMsg("CVideoSourceEvrEx::Detach: cannot obtain state from IMediaControl, hr = 0x%08x", hr);
			return hr;
		}

		if (State_Stopped != state)
		{
			DbgMsg("CVideoSourceEvrEx::Detach: correspondent graph was not stopped");
			return VFW_E_NOT_STOPPED;
		}

		hr = DisconnectPins();
		if (FAILED(hr))
			DbgMsg("CVideoSourceEvrEx::Detach: FATAL, failed to disconnect pins of VMR");

		SAFE_RELEASE(m_pCustomMixer);
	}

	Cleanup();
	m_pStateEventSink->Notify(SERVER_STATE_VIDEO_SOURCE_REMOVE, reinterpret_cast<DWORD> (this), 0);
	m_pVideoSink->OnVideoSourceRemoved(this);

	return hr;
}

//////////////////////////////////////////////////////////////////////////
// IMFGetService
STDMETHODIMP CVideoSourceEvrEx::GetService(REFGUID guidService, REFIID riid, LPVOID* ppv)
{
	HRESULT hr = E_NOINTERFACE;

	if (MR_VIDEO_RENDER_SERVICE == guidService)
	{
		if (riid == __uuidof(IDirect3DDeviceManager9))
		{
			hr = m_pDeviceManager->QueryInterface(__uuidof(IDirect3DDeviceManager9), ppv);
		}
		else if (riid == __uuidof(IMFVideoDisplayControl))
		{
			VSE_DP("interface IMFVideoDisplayControl is not implemented\n");
			return hr;
		}
		else if (riid == __uuidof(IDispSvrVideoMixer))
		{
			hr = m_pVideoSink->QueryInterface(riid, ppv);
		}
	}
	else if (MR_VIDEO_ACCELERATION_SERVICE == guidService)
	{
		if (riid == __uuidof(IDirect3DDeviceManager9))
		{
			hr = m_pDeviceManager->QueryInterface(__uuidof(IDirect3DDeviceManager9), ppv);
		}
	}
	ASSERT(SUCCEEDED(hr));	// alert if we miss handling certain services.
	return hr;
}

//////////////////////////////////////////////////////////////////////////
// IMFTopologyServiceLookupClient
STDMETHODIMP CVideoSourceEvrEx::InitServicePointers(IMFTopologyServiceLookup *pLookup)
{
	HRESULT hr = E_POINTER;
	if (pLookup)
	{
		MF_SERVICE_LOOKUP_TYPE eType = MF_SERVICE_LOOKUP_ALL;	// type is currently ignored
		DWORD nObjects = 1;		// 1 is the current implementation of LookupService

		ASSERT(!m_pMediaEventSink);
		hr = pLookup->LookupService(eType, 0, MR_VIDEO_RENDER_SERVICE, __uuidof(IMediaEventSink), (LPVOID *)&m_pMediaEventSink, &nObjects);
		if (FAILED(hr))
			return hr;

		hr = pLookup->LookupService(eType, 0, MR_VIDEO_RENDER_SERVICE, __uuidof(m_pClock), (LPVOID *)&m_pClock, &nObjects);
		if (m_pClock)
		{
			DWORD dwFlags;
			hr = m_pClock->GetClockCharacteristics(&dwFlags);
			if (FAILED(hr) || (MFCLOCK_CHARACTERISTICS_FLAG_FREQUENCY_10MHZ & dwFlags) == 0)
			{
				MFCLOCK_PROPERTIES prop;
				ASSERT("GetClockCharacteristics returns clock which is not supported currently.");
				hr = m_pClock->GetProperties(&prop);
			}
		}

		hr = pLookup->LookupService(eType, 0, MR_VIDEO_MIXER_SERVICE, __uuidof(IMFTransform), (LPVOID *)&m_pMixer, &nObjects);
		return hr;
	}
	return hr;
}

// Here we should release all pointers acquired by InitServicePointers() call
STDMETHODIMP CVideoSourceEvrEx::ReleaseServicePointers()
{
	VSE_DP("ReleaseServicePointers\n");
	m_pMixer.Release();
	m_pClock.Release();
	m_pMediaEventSink.Release();
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
// IMFVideoDeviceID
STDMETHODIMP CVideoSourceEvrEx::GetDeviceID(IID* pDeviceID)
{
	if (pDeviceID == NULL)
		return E_POINTER;
	*pDeviceID = __uuidof(IDirect3DDevice9);
	return S_OK;
}

// IMFVideoPresenter
STDMETHODIMP CVideoSourceEvrEx::GetCurrentMediaType(IMFVideoMediaType** ppMediaType)
{
	HRESULT hr = S_OK;

	if (ppMediaType == NULL)
		return E_POINTER;
	if (m_pMediaType == NULL)
		return MF_E_NOT_INITIALIZED;
	hr = m_pMediaType.QueryInterface(ppMediaType);
	return hr;
}

STDMETHODIMP CVideoSourceEvrEx::ProcessMessage(MFVP_MESSAGE_TYPE eMessage, ULONG_PTR ulParam)
{
	HRESULT hr = S_FALSE;

	switch (eMessage)
	{
	// The EVR switched from stopped to paused. The presenter should allocate resources.
	case MFVP_MESSAGE_BEGINSTREAMING:
		VSE_DP("MFVP_MESSAGE_BEGINSTREAMING\n");
		m_bEndOfStreaming = false;
		break;

	// Cancels a frame step.
	case MFVP_MESSAGE_CANCELSTEP:
		VSE_DP("MFVP_MESSAGE_CANCELSTEP\n");
		// indicates a frame step is cancelled.
		hr = m_pMediaEventSink->Notify(EC_STEP_COMPLETE, TRUE, 0);
		break;

	// All input streams have ended.
	case MFVP_MESSAGE_ENDOFSTREAM:
		VSE_DP("MFVP_MESSAGE_ENDOFSTREAM\n");
		hr = m_pMediaEventSink->Notify(EC_COMPLETE, 0, 0);
		break;

	// The EVR switched from running or paused to stopped. The presenter should free resources.
	case MFVP_MESSAGE_ENDSTREAMING:
		VSE_DP("MFVP_MESSAGE_ENDSTREAMING\n");
		m_bEndOfStreaming = true;
		break;

	// The presenter should discard any pending samples.
	case MFVP_MESSAGE_FLUSH:
		VSE_DP("MFVP_MESSAGE_FLUSH\n");
		break;

	// The mixer's output format has changed. The EVR will initiate format negotiation, as described previously.
	case MFVP_MESSAGE_INVALIDATEMEDIATYPE:
		VSE_DP("MFVP_MESSAGE_INVALIDATEMEDIATYPE\n");
		hr = RenegotiateMediaType();
		break;

	// One input stream on the mixer has received a new sample.
	case MFVP_MESSAGE_PROCESSINPUTNOTIFY:
		VSE_DP("MFVP_MESSAGE_PROCESSINPUTNOTIFY\n");
		SetEvent(m_hProcessInputNotify);
		break;

	// Requests a frame step.
	case MFVP_MESSAGE_STEP:
		VSE_DP("MFVP_MESSAGE_STEP\n");
		hr = m_pMediaEventSink->Notify(EC_STEP_COMPLETE, TRUE, 0);
		break;

	default:
		VSE_DP("ProcessMessage unhandled 0x%x, ulParam=0x%x, process hr=0x%x\n", (int)eMessage, ulParam, hr);
		break;
	}

	return hr;
}

HRESULT CVideoSourceEvrEx::SetMediaType(IMFMediaType *pType)
{
	HRESULT hr = S_OK;
	UINT uNumerator, uDenominator;

	m_fPixelAspectRatio = 0.f;
	uNumerator = uDenominator = m_uWidth = m_uHeight = 0;
	hr = MFGetAttributeSize(pType, MF_MT_FRAME_SIZE, &m_uWidth, &m_uHeight);
	if (FAILED(hr))
		return hr;

	hr = MFGetAttributeRatio(pType, MF_MT_PIXEL_ASPECT_RATIO, &uNumerator, &uDenominator);
	if (SUCCEEDED(hr) && uDenominator)
		m_fPixelAspectRatio = FLOAT(uNumerator) / uDenominator;

	m_pMediaType = pType;
	return hr;
}

HRESULT CVideoSourceEvrEx::RenegotiateMediaType()
{
	HRESULT hr;
	DWORD pdwInputIDs[MAX_STREAMS], pdwOutputIDs[MAX_STREAMS];

	m_pMediaType.Release();
	hr = m_pMixer->GetStreamIDs(MAX_STREAMS, pdwInputIDs, MAX_STREAMS, pdwOutputIDs);
	if (FAILED(hr))
		return hr;

	DWORD dwTypeIndex = 0;

	do 
	{
		CComPtr<IMFMediaType> pOutputType;
		CComPtr<IMFMediaType> pDesiredType;

		hr = m_pMixer->GetOutputAvailableType(0, dwTypeIndex++, &pOutputType);
		if (FAILED(hr))
			continue;

		hr = CreateDesiredOutputType(pOutputType, &pDesiredType);
		if (FAILED(hr))
			continue;

		hr = m_pMixer->SetOutputType(pdwOutputIDs[0], pDesiredType, MFT_SET_TYPE_TEST_ONLY);
		if (FAILED(hr))
			continue;

		m_dwOutputStreamId = pdwOutputIDs[0];
		hr = m_pMixer->SetOutputType(pdwOutputIDs[0], pDesiredType, 0);
		if (FAILED(hr))
			continue;

		hr = SetMediaType(pDesiredType);

	} while(FAILED(hr) && MF_E_NO_MORE_TYPES != hr);

	return hr;
}

HRESULT CVideoSourceEvrEx::CreateDesiredOutputType(IMFMediaType *pType, IMFMediaType **ppDesiredType)
{
	HRESULT hr;
	IMFMediaType *pmt;
	GUID guidMainType = GUID_NULL;
	GUID guidSubType = GUID_NULL;
	UINT uWidth = 0, uHeight = 0;
	UINT uNumerator = 0, uDenominator = 0;

	hr = pType->GetGUID(MF_MT_MAJOR_TYPE, &guidMainType);
	ASSERT(SUCCEEDED(hr));

	hr = pType->GetGUID(MF_MT_SUBTYPE, &guidSubType);
	ASSERT(SUCCEEDED(hr));

	hr = MFGetAttributeSize(pType, MF_MT_FRAME_SIZE, &uWidth, &uHeight);
	if (FAILED(hr))
		return hr;

	hr = MFGetAttributeRatio(pType, MF_MT_PIXEL_ASPECT_RATIO, &uNumerator, &uDenominator);
	if (FAILED(hr))
		return hr;

	hr = CDynLibManager::GetInstance()->pfnMFCreateMediaType(&pmt);
	if (SUCCEEDED(hr))
	{
		hr = pmt->SetGUID(MF_MT_MAJOR_TYPE, guidMainType);
		ASSERT(SUCCEEDED(hr));
		hr = pmt->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32);
		ASSERT(SUCCEEDED(hr));

		hr = MFSetAttributeSize(pmt, MF_MT_FRAME_SIZE, uWidth, uHeight);
		ASSERT(SUCCEEDED(hr));
		hr = MFSetAttributeRatio(pmt, MF_MT_PIXEL_ASPECT_RATIO, uNumerator, uDenominator);
		ASSERT(SUCCEEDED(hr));

		*ppDesiredType = pmt;
	}
	return hr;
}


//////////////////////////////////////////////////////////////////////////
// IMFClockStateSink
STDMETHODIMP CVideoSourceEvrEx::OnClockPause(MFTIME hnsSystemTime)
{
	VSE_DP("OnClockPause at %I64d\n", hnsSystemTime);
	m_ePresenterState = PRESENTER_STATE_PAUSED;
	return S_OK;
}

STDMETHODIMP CVideoSourceEvrEx::OnClockRestart(MFTIME hnsSystemTime)
{
	VSE_DP("OnClockRestart at %I64d\n", hnsSystemTime);
	m_ePresenterState = PRESENTER_STATE_STARTED;
	SetEvent(m_hProcessInputNotify);
	return S_OK;
}

STDMETHODIMP CVideoSourceEvrEx::OnClockSetRate(MFTIME hnsSystemTime, float flRate)
{
	VSE_DP("OnClockSetRate at %I64d, rate=%f\n", hnsSystemTime, flRate);
	return S_OK;
}

STDMETHODIMP CVideoSourceEvrEx::OnClockStart(MFTIME hnsSystemTime, LONGLONG llClockStartOffset)
{
	VSE_DP("OnClockStart at sys=%I64d, offset=%I64d\n", hnsSystemTime, llClockStartOffset);
	m_ePresenterState = PRESENTER_STATE_STARTED;
	return S_OK;
}

STDMETHODIMP CVideoSourceEvrEx::OnClockStop(MFTIME hnsSystemTime)
{
	VSE_DP("OnClockStop at %I64d\n", hnsSystemTime);
	m_ePresenterState = PRESENTER_STATE_STOPPED;
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
// IMFVideoDisplayControl

STDMETHODIMP CVideoSourceEvrEx::SetVideoWindow(HWND hwndVideo)
{
	m_hwndVideo = hwndVideo;
	return S_OK;
}

STDMETHODIMP CVideoSourceEvrEx::RepaintVideo()
{
	CComQIPtr<IDisplayObject> pObj = m_pVideoSink;
	if (pObj)
	{
		pObj->NotifyEvent(DISPLAY_EVENT_VideoSourceRender, 0, 0, this);
		pObj->NotifyEvent(DISPLAY_EVENT_VideoSourcePresent, 0, 0, this);
	}
	return S_OK;
}

STDMETHODIMP CVideoSourceEvrEx::SetAspectRatioMode(DWORD dwAspectRatioMode)
{
	if (dwAspectRatioMode == MFVideoARMode_None || dwAspectRatioMode == MFVideoARMode_PreservePicture)
		m_dwAspectRatioMode = dwAspectRatioMode;
	else
		return E_NOTIMPL;

	return S_OK;
}

STDMETHODIMP CVideoSourceEvrEx::GetAspectRatioMode(DWORD*  pdwAspectRatioMode)
{
	ASSERT(pdwAspectRatioMode);
	*pdwAspectRatioMode = m_dwAspectRatioMode;
	return S_OK;
}

STDMETHODIMP CVideoSourceEvrEx::GetNativeVideoSize(SIZE* pszVideo, SIZE* pszARVideo)
{
	if (m_pMediaType)
	{
		if(pszVideo)
		{
			pszVideo->cx = m_uWidth;
			pszVideo->cy = m_uHeight;
		}
		if(pszARVideo)
		{
			pszARVideo->cx = 0;
			pszARVideo->cy = 0;
		}
	}
	return S_OK;
}

STDMETHODIMP CVideoSourceEvrEx::GetIdealVideoSize(SIZE* pszMin, SIZE* pszMax)
{

	if (pszMax)
	{
		pszMax->cx = m_uWidth;
		pszMax->cy = m_uHeight;
	}
	if (pszMin)
	{
		pszMax->cx = m_uWidth;
		pszMax->cy = m_uHeight;
	}
	return S_OK;
}

STDMETHODIMP CVideoSourceEvrEx::SetVideoPosition(const MFVideoNormalizedRect* pnrcSource, const LPRECT prcDest)
{
	// One parameter can be NULL, but not both.
	if (pnrcSource == NULL && prcDest == NULL)
	{
		return E_POINTER;
	}

	HRESULT hr = S_OK;

	// Validate the rectangles.
	if (pnrcSource)
	{
		// The source rectangle cannot be flipped.
		if ((pnrcSource->left > pnrcSource->right) ||
			(pnrcSource->top > pnrcSource->bottom))
		{
			return E_INVALIDARG;
		}

		// The source rectangle has range (0..1)
		if ((pnrcSource->left < 0) || (pnrcSource->right > 1) ||
			(pnrcSource->top < 0) || (pnrcSource->bottom > 1))
		{
			return E_INVALIDARG;
		}

		// Update the source rectangle. Source clipping is performed by the mixer.
		if (m_pMixer)
		{
			m_nrcTexture.left = pnrcSource->left;
			m_nrcTexture.top = pnrcSource->top;
			m_nrcTexture.right = pnrcSource->right;
			m_nrcTexture.bottom = pnrcSource->bottom;
		}

		if(m_pDispObj)
		{
			CComQIPtr<IDisplayProperties> pProp;
			if (SUCCEEDED(m_pDispObj->QueryInterface(__uuidof(IDisplayProperties), (void **)&pProp)))
			{
				NORMALIZEDRECT nrcZoom;
				nrcZoom.left = m_nrcTexture.left * 2 - 1.0f;
				nrcZoom.right = m_nrcTexture.right * 2 - 1.0f;
				nrcZoom.top = 1.0f - m_nrcTexture.top * 2;
				nrcZoom.bottom = 1.0f - m_nrcTexture.bottom * 2;
				pProp->SetZoom(&nrcZoom);
			}
		}
	}

	if (prcDest)
	{
		// The destination rectangle cannot be flipped.
		if ((prcDest->left > prcDest->right) ||
			(prcDest->top > prcDest->bottom))
		{
			return E_INVALIDARG;
		}

		// Update the destination rectangle.
		m_dstrect.left = prcDest->left;
		m_dstrect.top = prcDest->top;
		m_dstrect.right = prcDest->right;
		m_dstrect.bottom = prcDest->bottom;
	}

	return hr;
}

STDMETHODIMP CVideoSourceEvrEx::GetVideoPosition(MFVideoNormalizedRect* pnrcSource, LPRECT prcDest)
{
	if (pnrcSource)
	{
		pnrcSource->left = m_nrcTexture.left;
		pnrcSource->top = m_nrcTexture.top;
		pnrcSource->right = m_nrcTexture.right;
		pnrcSource->bottom = m_nrcTexture.bottom;
	}

	if (prcDest)
	{
		prcDest->left = m_dstrect.left;
		prcDest->top = m_dstrect.top;
		prcDest->right = m_dstrect.right;
		prcDest->bottom = m_dstrect.bottom;
	}

	return S_OK;
}
//////////////////////////////////////////////////////////////////////////
// EVR Processing thread
HRESULT CVideoSourceEvrEx::StartEvrPresentThread()
{
	StopEvrPresentThread();

	if (m_eThreadStatus == eNotStarted)
	{
		DWORD tid = 0;
		HANDLE hThread = CreateThread(NULL, NULL, EvrPresentThread, this, NULL, &tid);
		if (INVALID_HANDLE_VALUE == hThread)
		{
			DbgMsg("CVideoSourceEvrEx failed to create present output thread.");
			return E_UNEXPECTED;
		}
		m_eThreadStatus = eRunning;
		CloseHandle(hThread);
	}
	return S_OK;
}

void CVideoSourceEvrEx::StopEvrPresentThread()
{
	if (m_eThreadStatus == eRunning)
	{
		m_eThreadStatus = eWaitingToStop;
		while (m_eThreadStatus != eFinished)
		{
			SetEvent(m_hProcessInputNotify);
			Sleep(50);
		}
		m_eThreadStatus = eNotStarted;
	}
}

// Process output from the mixer
HRESULT CVideoSourceEvrEx::ProcessOutputFromMixer(IMFSample **ppSample)
{
	DWORD dwStatus = 0;
	MFT_OUTPUT_DATA_BUFFER Sample = {0}, *pSamples = &Sample;
	DWORD nSamples = 1;		// output stream from mixer should be 1.
	HRESULT hr = E_FAIL;

	*ppSample = 0;
	if (!m_pMixer)
		return E_FAIL;

	if (m_ePresenterState != PRESENTER_STATE_STARTED)
		return E_FAIL;

	// With custom mixer, the original sample will be bypassed from mixer to the presenter.
	// Therefore, Sample.pSample is to be filled by the mixer.
	Sample.pSample = NULL;
	Sample.dwStreamID = m_dwOutputStreamId;
	Sample.dwStatus = 0;

	hr = m_pMixer->ProcessOutput(0, nSamples, pSamples, &dwStatus);

	if (FAILED(hr))
	{
		// Handle some known error codes from ProcessOutput.
		if (hr == MF_E_TRANSFORM_TYPE_NOT_SET)
		{
			// perhaps it is not a good idea to renegotiate a new media type in a separated thread like here.
			//			hr = RenegotiateMediaType();
		}
		else if (hr == MF_E_TRANSFORM_STREAM_CHANGE)
		{
			// Dynamic media type change. Clear our media type.
			SetMediaType(NULL);
		}
		else if (hr == MF_E_TRANSFORM_NEED_MORE_INPUT)
		{
			// The mixer needs more input.
		}
	}
	else
	{
		m_pStateEventSink->Notify(SERVER_STATE_VIDEO_SOURCE_PROCESS_INPUT, reinterpret_cast<DWORD> (this), 0);

		*ppSample = Sample.pSample;
		(*ppSample)->AddRef();
		Sample.pSample->Release();
		Sample.pSample = 0;
	}

	// we are responsible for releasing events if any is returned.
	for (DWORD i = 0; i < nSamples; i++)
	{
		if (pSamples[i].pEvents)
		{
			pSamples[i].pEvents->Release();
			pSamples[i].pEvents = NULL;
		}
	}

	return hr;
}

//////////////////////////////////////////////////////////////////////////
// EVR present thread

DWORD WINAPI CVideoSourceEvrEx::EvrPresentThread(LPVOID lpParameter)
{
	CVideoSourceEvrEx *pThis = static_cast<CVideoSourceEvrEx *> (lpParameter);
	ASSERT(pThis);
	pThis->PresentLoop();
	return 0;
}

void CVideoSourceEvrEx::PresentLoop()
{
	{
		// the lock serves as a barrier when the thread is created.
		CAutoLock lock(&m_csEvrPresenting);
		ASSERT(m_eThreadStatus == eRunning);
	}

	HRESULT hr = S_OK;
	LONGLONG hnsPresent, hnsNow, hnsDuration, hnsDelta, hnsLastPts = 0;
	LONG lSleep;
	MFTIME hnsSystem;
	IMFSample *pSample = 0;
	UINT uPullDown = 0;

	while (m_eThreadStatus == eRunning)
	{
		WaitForSingleObject(m_hProcessInputNotify, INFINITE);

		{
			CAutoLock lockPresent(&m_csEvrPresenting);
			ASSERT(pSample == 0);
			hr = ProcessOutputFromMixer(&pSample);
			if (FAILED(hr))
			{
				ResetEvent(m_hProcessInputNotify);
				continue;
			}

			OnIncomingSample(pSample);
			pSample = 0;

			uPullDown = 1;
			if (m_pSample)
			{
				hnsDelta = hnsDuration = hnsPresent = 0;
				hnsSystem = 0;
				lSleep = 0;

				hr = m_pSample->GetSampleTime(&hnsPresent);
				if (SUCCEEDED(hr) && m_pClock && m_ePresenterState == PRESENTER_STATE_STARTED)
				{
					if (MFGetAttributeUINT32(m_pSample, MFSampleExtension_Interlaced, FALSE) != FALSE)
					{
						if (MFGetAttributeUINT32(m_pSample, MFSampleExtension_RepeatFirstField, FALSE) != FALSE)
							uPullDown = 3;
						else
							uPullDown = 2;
					}

					hr = m_pSample->GetSampleDuration(&hnsDuration);
					hr = m_pClock->GetCorrelatedTime(0, &hnsNow, &hnsSystem);
					hnsDelta = hnsPresent - hnsNow;
					lSleep = LONG(hnsDelta / 10000);
					VSE_DP("GetCorrelatedTime sleep=%d, diffpts=%I64d, diff=%I64d, sample=%I64d, clock=%I64d, duration=%I64d sys=%I64d\n",
						lSleep, hnsPresent - hnsLastPts, hnsDelta, hnsPresent, hnsNow, hnsDuration, hnsSystem);
					hnsLastPts = hnsPresent;

					hr = m_pSample->SetUINT32(VideoSourceEvrEx_FieldSelect, 0);	ASSERT(SUCCEEDED(hr));
				}
			}
		}

		OnPresent(lSleep);

		if (uPullDown > 1)
		{
			hnsDuration /= uPullDown;

			while (--uPullDown > 0)
			{
				{
					CAutoLock lockPresent(&m_csEvrPresenting);
					hr = m_pClock->GetCorrelatedTime(0, &hnsNow, &hnsSystem);
					hnsPresent += hnsDuration;
					hnsDelta = hnsPresent - hnsNow;
					lSleep = LONG(hnsDelta / 10000);
					hr = m_pSample->SetUINT32(VideoSourceEvrEx_FieldSelect, uPullDown);	ASSERT(SUCCEEDED(hr));
				}
				OnPresent(lSleep);
			}
		}
	}

	m_eThreadStatus = eFinished;
}

HRESULT CVideoSourceEvrEx::OnIncomingSample(IMFSample *pSample)
{
	// release held sample.
	if (m_pSample)
		m_pSample->Release();
	if (m_pBufferAtIndex0)
	{
		m_pBufferAtIndex0->Release();
		m_pBufferAtIndex0 = 0;
	}

	m_pSample = pSample;

	CComPtr<IMFMediaBuffer> pBuffer;
	CComPtr<IDirect3DSurface9> pSurface;
	HRESULT hr = E_FAIL;
	FLOAT fAspectRatio = 0.0;

	if (m_pSample)
		hr = m_pSample->GetBufferByIndex(0, &pBuffer);
	if (SUCCEEDED(hr))
	{
		CComQIPtr<IMFGetService> pGetSurface = pBuffer;
		if (pGetSurface)
			hr = pGetSurface->GetService(MR_BUFFER_SERVICE, __uuidof(IDirect3DSurface9), (void **)&m_pBufferAtIndex0);

		if (m_pBufferAtIndex0)
		{
			D3DSURFACE_DESC desc;
			m_pBufferAtIndex0->GetDesc(&desc);
			if (desc.Width < m_uWidth)
				m_uWidth = desc.Width;
			if (desc.Height < m_uHeight)
				m_uHeight = desc.Height;

			if (m_fPixelAspectRatio > 0)
				fAspectRatio = m_uWidth * m_fPixelAspectRatio / m_uHeight;

			m_fAspectRatio = m_fNativeAspectRatio = fAspectRatio;
//			DbgMsg("surface width, height = %dx%d, aspect=%.3f", desc.Width, desc.Height, m_fAspectRatio);
		}
	}
	return hr;
}

void CVideoSourceEvrEx::OnPresent(LONG lSleep)
{
	DWORD dwRenderDiff = timeGetTime();
	m_pDispObj->NotifyEvent(DISPLAY_EVENT_VideoSourceRender, 0, 0, this);
	dwRenderDiff = timeGetTime() - dwRenderDiff;

	if (lSleep > 0)
	{
		lSleep -= dwRenderDiff;
		if (lSleep > 0)
			Sleep(lSleep);
	}

	m_pStateEventSink->Notify(SERVER_STATE_VIDEO_SOURCE_PROCESS_OUTPUT, reinterpret_cast<DWORD> (this), 0);
	m_pDispObj->NotifyEvent(DISPLAY_EVENT_VideoSourcePresent, 0, 0, this);
}

STDMETHODIMP CVideoSourceEvrEx::GetSampleProperty(SampleProperty *pProp)
{
	if (!pProp)
		return E_POINTER;

	CAutoLock lockPresent(&m_csEvrPresenting);
	HRESULT hr = E_FAIL;

	if (m_pBufferAtIndex0 && m_pSample)
		hr = m_pBufferAtIndex0->QueryInterface(__uuidof(IUnknown), (void **) &pProp->pSurface);

	if (SUCCEEDED(hr))
	{
		pProp->fAspectRatio = m_fAspectRatio;
		pProp->uWidth = m_uWidth;
		pProp->uHeight = m_uHeight;
		m_pSample->GetSampleTime(&pProp->rtStart);
		m_pSample->GetSampleDuration(&pProp->rtEnd);
		pProp->rtEnd += pProp->rtStart;

		if (MFGetAttributeUINT32(m_pSample, MFSampleExtension_Interlaced, FALSE) != FALSE)
			pProp->dwFlags |= SAMPLE_FLAG_INTERLACED;

		if (MFGetAttributeUINT32(m_pSample, MFSampleExtension_Discontinuity, FALSE) != FALSE)
			pProp->dwFlags |= SAMPLE_FLAG_DISCONTINUITY;

		if (MFGetAttributeUINT32(m_pSample, MFSampleExtension_BottomFieldFirst, FALSE) != FALSE)
			pProp->dwFlags |= SAMPLE_FLAG_BOTTOMFIELDFIRST;

		if (MFGetAttributeUINT32(m_pSample, MFSampleExtension_RepeatFirstField, FALSE) != FALSE)
			pProp->dwFlags |= SAMPLE_FLAG_REPEATFIRSTFIELD;

		if (MFGetAttributeUINT32(m_pSample, VideoSourceEvrEx_FieldSelect, 0) == 1)
			pProp->dwFlags |= SAMPLE_FLAG_SELECTSECONDFIELD;
	}
	return hr;
}