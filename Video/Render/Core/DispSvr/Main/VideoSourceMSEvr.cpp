#include "stdafx.h"
#include "DispSvr.h"
#include "DynLibManager.h"
#include "ResourceManager.h"
#include "ServerStateEventSink.h"
#include "EvrHelper.h"
#include "EVRSamplePool.h"
#include "EvrPresentScheduler.h"
#include "VideoSourceDO.h"
#include "Exports/Inc/VideoMixer.h"
#include "Exports/Inc/VideoPresenter.h"
#include "EvrCustomMixer.h"
#include "VideoSourceMSEvr.h"
#include "RegistryService.h"

using namespace DispSvr;

#ifndef VSE_DP
//#define VSE_DP(fmt, ...)	DbgMsg("CVideoSourceMSEvr::" fmt, __VA_ARGS__)
#define VSE_DP __noop
#endif

#define IF_FAILED_GOTO(hr, label) if (FAILED(hr)) { goto label; }
#define CHECK_HR_GOTO(hr) IF_FAILED_GOTO(hr, done)

// Default frame rate.
const MFRatio g_DefaultFrameRate = { 30, 1 };
// present buffer count
const DWORD PRESENTER_BUFFER_COUNT = 4;

// IDisplayVideoSource

STDMETHODIMP CVideoSourceMSEvr::GetGraph(IFilterGraph** ppGraph)
{
	if (!ppGraph)
	{
		VSE_DP("GetGraph: ppGraph is NULL");
		return E_POINTER;
	}

	if (m_pGraph)
	{
		m_pGraph.CopyTo(ppGraph);
		return S_OK;
	}

	VSE_DP("GetGraph: FATAL: contains NULL IFilterGraph pointer");
	return VFW_E_NOT_FOUND;
}

STDMETHODIMP CVideoSourceMSEvr::GetTexture(IUnknown** ppTexture, NORMALIZEDRECT* lpNormRect)
{
	return E_NOTIMPL;
}

STDMETHODIMP CVideoSourceMSEvr::BeginDraw()
{
	return S_OK;
}

STDMETHODIMP CVideoSourceMSEvr::EndDraw()
{
	return S_OK;
}

STDMETHODIMP CVideoSourceMSEvr::IsValid()
{
	return m_bValid ? S_OK : E_FAIL;
}

STDMETHODIMP CVideoSourceMSEvr::ClearImage()
{
	return S_OK;
}

STDMETHODIMP CVideoSourceMSEvr::GetVideoSize(LONG* plWidth, LONG* plHeight, float *pfAspectRatio)
{
	return E_NOTIMPL;
}

HRESULT CVideoSourceMSEvr::BeginDeviceLoss()
{
	m_bValid = FALSE;

	CAutoLock lock(&m_ObjectLock);

	if (m_pDeviceManager && m_hDeviceHandle)
	{
		m_pDeviceManager->CloseDeviceHandle(m_hDeviceHandle);
		m_hDeviceHandle = NULL;
	}
	Flush();
	m_SamplePool.Clear();
	SetMediaType(NULL);
	return S_OK;
}

HRESULT CVideoSourceMSEvr::EndDeviceLoss(IUnknown* pDevice)
{
	CAutoLock lock(&m_ObjectLock);

	HRESULT hr = S_OK;
	if (m_pDeviceManager)
	{
		CComQIPtr<IDirect3DDevice9> pDevice9 = pDevice;
		hr = m_pDeviceManager->ResetDevice(pDevice9, m_uDeviceManagerToken);
		ASSERT(SUCCEEDED(hr));
		hr = m_pDeviceManager->OpenDeviceHandle(&m_hDeviceHandle);
		ASSERT(SUCCEEDED(hr));
	}
	if (m_pMediaEventSink)
		m_pMediaEventSink->Notify(EC_DISPLAY_CHANGED, 0, 0);

	m_bValid = TRUE;
	return hr;
}

HRESULT CVideoSourceMSEvr::EnableInitiativeDisplay(BOOL bEnable)
{
	HRESULT hr = S_OK;
	m_bInitiativeDisplay = bEnable;
	return hr;
}

// Disconnects pins of VMR
HRESULT CVideoSourceMSEvr::DisconnectPins()
{
	HRESULT hr = S_OK;
	if (!m_pVMR)
	{
		return E_POINTER;
	}

	try
	{
		CComPtr<IEnumPins> pEnum;
		hr = m_pVMR->EnumPins(&pEnum);
		CHECK_HR(hr, VSE_DP("DisconnectPins: failed to enumerate pins, hr = 0x%08x", hr));

		CComPtr<IPin> pPin;
		hr = pEnum->Next(1, &pPin, NULL);
		while (S_OK == hr && pPin)
		{
			hr = pPin->Disconnect();
			CHECK_HR(hr, VSE_DP("DisconnectPins: failed to disconnect pin, hr = 0x%08x", hr));

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

HRESULT CVideoSourceMSEvr::Attach(IBaseFilter* pVMR)
{
	HRESULT hr = S_OK;

	try
	{
		CComPtr<IEnumPins>			pEnumPins;
		CComPtr<IMFVideoRenderer>	pEvr;

		hr = pVMR->QueryInterface(__uuidof(IMFVideoRenderer), (void **)&pEvr);
		CHECK_HR(hr, VSE_DP("Attach: filter is not an EVR"));

		// in media foundation, a fake IBaseFilter is used thus we disable dshow workaround.
		if(SUCCEEDED(pVMR->EnumPins(&pEnumPins)))
		{
			m_bDShowWorkaround = true;
			m_pVMR = pVMR;				// no need to keep EVR pointer for MF
		}
		CAutoDisplayLock displayLock(m_pLock);

		CComPtr<IDirect3DDevice9> pDevice;
		hr = m_pVideoSink->Get3DDevice((IUnknown**)&pDevice);
		CHECK_HR(hr, VSE_DP("Attach: failed to obtain Direct3D device from the video sink, hr = 0x%08x", hr));

		if (!CDynLibManager::GetInstance()->pfnDXVA2CreateDirect3DDeviceManager9)
			hr = E_FAIL;
		CHECK_HR(hr, VSE_DP("Attach: failed to link evr.dll, mfplat.dll or dxva2.dll"));

		if (SUCCEEDED(hr))
			hr = CDynLibManager::GetInstance()->pfnDXVA2CreateDirect3DDeviceManager9(&m_uDeviceManagerToken, &m_pDeviceManager);
		if (SUCCEEDED(hr))
			hr = m_pDeviceManager->ResetDevice(pDevice, m_uDeviceManagerToken);
		CHECK_HR(hr, VSE_DP("Attach: unable to create device manager"));
		hr = m_pDeviceManager->OpenDeviceHandle(&m_hDeviceHandle);
		CHECK_HR(hr, VSE_DP("Attach: unable to open device handle from manager"));

//		hr = StartEvrPresentThread();
		CHECK_HR(hr, VSE_DP("Attach: unable to start present thread."));

		if (0)
		{
			(m_pCustomMixer = new CEvrCustomMixer)->AddRef();
			// if using custom mixer, we can't use TrackedSample callback because samples from EVR are already SetAllocator()'ed.
			m_bDShowWorkaround = true;
		}

		hr = pEvr->InitializeRenderer(m_pCustomMixer, this);
		CHECK_HR(hr, VSE_DP("Attach: IMFVideoRenderer::InitializeRenderer failed. hr=0x%x", hr));

		hr = m_pVideoSink->OnVideoSourceAdded(this,	m_fAspectRatio);
		CHECK_HR(hr, VSE_DP("Attach: failed in IDisplayVideoSink::AddVideoSource(), hr = 0x%08x", hr));

		m_pStateEventSink->Notify(SERVER_STATE_VIDEO_SOURCE_ADD, reinterpret_cast<DWORD> (this), 0);
	}
	catch (HRESULT hrFailed)
	{
		Detach();
		hr = hrFailed;
	}

	return hr;
}

HRESULT CVideoSourceMSEvr::Detach()
{
	HRESULT hr = S_OK;

	if (!m_pVideoSink)
	{
		VSE_DP("Detach: FATAL IDisplayVideoSink pointer is NULL!");
		return E_UNEXPECTED;
	}

	try
	{
		// we have to be thread safe only here when we actually mess up with shared data
		CAutoDisplayLock displayLock(m_pLock);
		hr = DisconnectPins();
		//will fail in MF case CHECK_HR(hr, VSE_DP("Detach: FATAL, failed to disconnect pins of VMR"));

		Cleanup();
		m_pStateEventSink->Notify(SERVER_STATE_VIDEO_SOURCE_REMOVE, reinterpret_cast<DWORD> (this), 0);
		m_pVideoSink->OnVideoSourceRemoved(this);
	}
	catch (HRESULT hrFailed)
	{
		hr = hrFailed;
	}

	return hr;
}

///////////////////////////////////////////////////////////////////////////////
//
// IUnknown methods
//
///////////////////////////////////////////////////////////////////////////////

HRESULT CVideoSourceMSEvr::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
    CheckPointer(ppv, E_POINTER);

	HRESULT hr = E_NOINTERFACE;
	*ppv = NULL;

    if (riid == __uuidof(IMFVideoDeviceID))
    {
		hr = GetInterface((IMFVideoDeviceID*) this, ppv);
    }
    else if (riid == __uuidof(IMFVideoPresenter))
    {
		hr = GetInterface((IMFVideoPresenter*) this, ppv);
    }
    else if (riid == __uuidof(IMFClockStateSink))    // Inherited from IMFVideoPresenter
    {
		hr = GetInterface((IMFClockStateSink*) this, ppv);
    }
  //  else if (riid == __uuidof(IMFRateSupport))
  //  {
		//hr = GetInterface((IMFRateSupport*) this, ppv);
  //  }
    else if (riid == __uuidof(IMFGetService))
    {
		hr = GetInterface((IMFGetService*) this, ppv);
    }
    else if (riid == __uuidof(IMFTopologyServiceLookupClient))
    {
		hr = GetInterface((IMFTopologyServiceLookupClient*) this, ppv);
    }
    else if (riid == __uuidof(IMFVideoDisplayControl))
    {
		hr = GetInterface((IMFVideoDisplayControl*) this, ppv);
    }
	else if (riid == IID_IQualProp)
	{
		hr = GetInterface((IQualProp *) this, ppv);
	}
	else if (riid == __uuidof(IEvrSchedulerCallback))
	{
		hr = GetInterface((IEvrSchedulerCallback*) this, ppv);
	}
    else if (riid == __uuidof(IDisplayVideoSourcePropertyExtension))
    {
        hr = GetInterface((IDisplayVideoSourcePropertyExtension *) this, ppv);
    }
    else
    {
		hr = CUnknown::NonDelegatingQueryInterface(riid, ppv);
    }
    return hr;
}

///////////////////////////////////////////////////////////////////////////////
//
// IMFGetService methods
//
///////////////////////////////////////////////////////////////////////////////

HRESULT CVideoSourceMSEvr::GetService(REFGUID guidService, REFIID riid, LPVOID *ppvObject)
{
    HRESULT hr = S_OK;

    CheckPointer(ppvObject, E_POINTER);

	if (MR_VIDEO_RENDER_SERVICE == guidService)
	{
		if (riid == __uuidof(IDirect3DDeviceManager9))
		{
			hr = m_pDeviceManager->QueryInterface(__uuidof(IDirect3DDeviceManager9), ppvObject);
		}
        else if (riid == __uuidof(IDispSvrVideoMixer))
        {
            hr = m_pVideoSink->QueryInterface(riid, ppvObject);
        }
		else if(riid == __uuidof(IMediaEventSink))
		{
			return E_NOINTERFACE;
		}
		else
	        hr = QueryInterface(riid, ppvObject);
	}
	else if (MR_VIDEO_ACCELERATION_SERVICE == guidService)
	{
		if (riid == __uuidof(IDirect3DDeviceManager9))
		{
			hr = m_pDeviceManager->QueryInterface(__uuidof(IDirect3DDeviceManager9), ppvObject);
		}
	}
	else 
		return MF_E_UNSUPPORTED_SERVICE;

	ASSERT(SUCCEEDED(hr));	// alert if we miss handling certain services.
	return hr;

//    // The only service GUID that we support is MR_VIDEO_RENDER_SERVICE.
//    if (guidService != MR_VIDEO_RENDER_SERVICE)
//    {
//        return MF_E_UNSUPPORTED_SERVICE;
//    }
//
//    // First try to get the service interface from the D3DPresentEngine object.
////    hr = m_pD3DPresentEngine->GetService(guidService, riid, ppvObject);
//    if (FAILED(hr))
//    {
//        // Next, QI to check if this object supports the interface.
//        hr = QueryInterface(riid, ppvObject);
//    }

    //return hr;
}


///////////////////////////////////////////////////////////////////////////////
//
// IMFVideoDeviceID methods
//
//////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
// GetDeviceID
//
// Returns the presenter's device ID. 
// The presenter and mixer must have matching device IDs. 
//-----------------------------------------------------------------------------

HRESULT CVideoSourceMSEvr::GetDeviceID(IID* pDeviceID)
{
    // This presenter is built on Direct3D9, so the device ID is 
    // IID_IDirect3DDevice9. (Same as the standard presenter.)

    if (pDeviceID == NULL)
    {
        return E_POINTER;
    }
    *pDeviceID = __uuidof(IDirect3DDevice9);
    return S_OK;
}


///////////////////////////////////////////////////////////////////////////////
//
// IMFTopologyServiceLookupClient methods.
//
///////////////////////////////////////////////////////////////////////////////


//-----------------------------------------------------------------------------
// InitServicePointers
//
// Enables the presenter to get various interfaces from the EVR and mixer.
//-----------------------------------------------------------------------------

HRESULT CVideoSourceMSEvr::InitServicePointers(IMFTopologyServiceLookup *pLookup)
{
    VSE_DP("InitServicePointers");
    CheckPointer(pLookup, E_POINTER);

    HRESULT             hr = S_OK;
    DWORD               dwObjectCount = 0;

    CAutoLock lock(&m_ObjectLock);

    // Do not allow initializing when playing or paused.
    if (IsActive())
    {
        CHECK_HR_GOTO(hr = MF_E_INVALIDREQUEST);
    }

    SAFE_RELEASE(m_pClock);
    SAFE_RELEASE(m_pMixer);
    SAFE_RELEASE(m_pMediaEventSink);

    // Ask for the clock. Optional, because the EVR might not have a clock.
    dwObjectCount = 1;

    (void)pLookup->LookupService(      
        MF_SERVICE_LOOKUP_GLOBAL,   // Not used.
        0,                          // Reserved.
        MR_VIDEO_RENDER_SERVICE,    // Service to look up.
        __uuidof(IMFClock),         // Interface to look up.
        (void**)&m_pClock,
        &dwObjectCount              // Number of elements in the previous parameter.
        );

    // Ask for the mixer. (Required.)
    dwObjectCount = 1; 

    CHECK_HR_GOTO(hr = pLookup->LookupService(
        MF_SERVICE_LOOKUP_GLOBAL, 
        0, 
        MR_VIDEO_MIXER_SERVICE,
        __uuidof(IMFTransform), 
        (void**)&m_pMixer, 
        &dwObjectCount
        ));

    // Make sure that we can work with this mixer.
    CHECK_HR_GOTO(ConfigureMixer(m_pMixer));

    // Ask for the EVR's event-sink interface. (Required.)
    dwObjectCount = 1;

    CHECK_HR_GOTO(hr = pLookup->LookupService(
        MF_SERVICE_LOOKUP_GLOBAL,
        0,                                  
        MR_VIDEO_RENDER_SERVICE,            
        __uuidof(IMediaEventSink),                
        (void**)&m_pMediaEventSink,          
        &dwObjectCount                      
        ));

    // Successfully initialized. Set the state to "stopped."
    m_RenderState = RENDER_STATE_STOPPED;

done:
    return hr;
}

//-----------------------------------------------------------------------------
// ReleaseServicePointers
// 
// Release all pointers obtained during the InitServicePointers method. 
//-----------------------------------------------------------------------------

HRESULT CVideoSourceMSEvr::ReleaseServicePointers()
{
    VSE_DP("ReleaseServicePointers");

    HRESULT hr = S_OK;

    // Enter the shut-down state.
    {
        CAutoLock lock(&m_ObjectLock);
        m_RenderState = RENDER_STATE_SHUTDOWN;
    }

    // Flush any samples that were scheduled.
    Flush();

    // Clear the media type and release related resources (surfaces, etc).
// We want to keep the media type until the next valid media type is set to avoid control/navigation being unable to
// set/get using IMFVideoDisplayControl interface during media type renegotiation.
//    SetMediaType(NULL);

    // Release all services that were acquired from InitServicePointers.
    SAFE_RELEASE(m_pClock);
    SAFE_RELEASE(m_pMixer);
    SAFE_RELEASE(m_pMediaEventSink);

    return hr;
}


///////////////////////////////////////////////////////////////////////////////
// IMFVideoPresenter methods
//////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
// ProcessMessage
//
// Handles various messages from the EVR.
// This method delegates all of the work to other class methods.
//-----------------------------------------------------------------------------

HRESULT CVideoSourceMSEvr::ProcessMessage(MFVP_MESSAGE_TYPE eMessage, ULONG_PTR ulParam)
{
    HRESULT hr = S_OK;


    CHECK_HR_GOTO(hr = CheckShutdown());

    switch (eMessage)
    {
    // Flush all pending samples.
    case MFVP_MESSAGE_FLUSH:
		VSE_DP("MFVP_MESSAGE_FLUSH");
        hr = Flush();
        break;

    // Renegotiate the media type with the mixer.
    case MFVP_MESSAGE_INVALIDATEMEDIATYPE:
		VSE_DP("MFVP_MESSAGE_INVALIDATEMEDIATYPE");
        hr = RenegotiateMediaType();
        break;

    // The mixer received a new input sample. 
    case MFVP_MESSAGE_PROCESSINPUTNOTIFY:
//		VSE_DP("MFVP_MESSAGE_PROCESSINPUTNOTIFY");
        hr = ProcessInputNotify();
        break;

    // Streaming is about to start.
    case MFVP_MESSAGE_BEGINSTREAMING:
		VSE_DP("MFVP_MESSAGE_BEGINSTREAMING");
        hr = BeginStreaming();
        break;

    // Streaming has ended. (The EVR has stopped.)
    case MFVP_MESSAGE_ENDSTREAMING:
		VSE_DP("MFVP_MESSAGE_ENDSTREAMING");
        hr = EndStreaming();
        break;

    // All input streams have ended.
    case MFVP_MESSAGE_ENDOFSTREAM:
        // Set the EOS flag. 
		VSE_DP("MFVP_MESSAGE_ENDOFSTREAM");
        m_bEndStreaming = TRUE; 
        // Check if it's time to send the EC_COMPLETE event to the EVR.
        hr = CheckEndOfStream();
        break;

    // Frame-stepping is starting.
    case MFVP_MESSAGE_STEP:
		VSE_DP("MFVP_MESSAGE_STEP");
		{
			CAutoLock lock(&m_ObjectLock);
	        hr = PrepareFrameStep(LODWORD(ulParam));
		}
        break;

    // Cancels frame-stepping.
    case MFVP_MESSAGE_CANCELSTEP:
		VSE_DP("MFVP_MESSAGE_CANCELSTEP");
		{
			CAutoLock lock(&m_ObjectLock);
	        hr = CancelFrameStep();
		}
        break;

    default:
		VSE_DP("MFVP_MESSAGE_UNKNOWN");
        hr = E_INVALIDARG; // Unknown message. (This case should never occur.)
        break;
    }

done:
    ASSERT(SUCCEEDED(hr));
    return hr;
}


//-----------------------------------------------------------------------------
// GetCurrentMediaType
// 
// Returns the current render format (the mixer's output format).
//-----------------------------------------------------------------------------

HRESULT CVideoSourceMSEvr::GetCurrentMediaType(IMFVideoMediaType** ppMediaType)
{
    HRESULT hr = S_OK;

    CAutoLock lock(&m_ObjectLock);

    if (ppMediaType == NULL)
    {
        return E_POINTER;
    }

    CHECK_HR_GOTO(hr = CheckShutdown());

    if (m_pMediaType == NULL)
    {
        CHECK_HR_GOTO(hr = MF_E_NOT_INITIALIZED);
    }

    // The function returns an IMFVideoMediaType pointer, and we store our media
    // type as an IMFMediaType pointer, so we need to QI.

    CHECK_HR_GOTO(hr = m_pMediaType->QueryInterface(__uuidof(IMFVideoMediaType), (void**)&ppMediaType));

done:
    return hr;
}


///////////////////////////////////////////////////////////////////////////////
//
// IMFClockStateSink methods
//
//////////////////////////////////////////////////////////////////////////////

// Note: The IMFClockStateSink interface handles state changes from the EVR,
// such as stopping, starting, and pausing.

//-----------------------------------------------------------------------------
// OnClockStart
// 
// Called when:
// (1) The clock starts from the stopped state, or
// (2) The clock seeks (jumps to a new position) while running or paused.
//-----------------------------------------------------------------------------

HRESULT CVideoSourceMSEvr::OnClockStart(MFTIME hnsSystemTime, LONGLONG llClockStartOffset)
{
    VSE_DP("OnClockStart (offset = %I64d)", llClockStartOffset);


    HRESULT hr = S_OK;

    CAutoLock lock(&m_ObjectLock);

    // We cannot start after shutdown.
    CHECK_HR_GOTO(hr = CheckShutdown());

    m_RenderState = RENDER_STATE_STARTED;

    // Check if the clock is already active (not stopped). 
    if (IsActive())
    {
        // If the clock position changes while the clock is active, it 
        // is a seek request. We need to flush all pending samples.
        if (llClockStartOffset != PRESENTATION_CURRENT_POSITION)
        {
            Flush();
        }
    }
    else
    {
        // The clock has started from the stopped state. 

        // Possibly we are in the middle of frame-stepping OR have samples waiting 
        // in the frame-step queue. Deal with these two cases first:
        CHECK_HR_GOTO(hr = StartFrameStep());
    }

    // Now try to get new output samples from the mixer.
    ProcessOutputLoop();

done:
    return hr;
}


//-----------------------------------------------------------------------------
// OnClockRestart
//
// Called when the clock restarts from the current position while paused.
//-----------------------------------------------------------------------------

HRESULT CVideoSourceMSEvr::OnClockRestart(MFTIME hnsSystemTime)
{
    VSE_DP("OnClockRestart");


    CAutoLock lock(&m_ObjectLock);

    HRESULT hr = S_OK;
    CHECK_HR_GOTO(hr = CheckShutdown());

    // The EVR calls OnClockRestart only while paused.
    ASSERT(m_RenderState == RENDER_STATE_PAUSED);

    m_RenderState = RENDER_STATE_STARTED;

    // Possibly we are in the middle of frame-stepping OR we have samples waiting 
    // in the frame-step queue. Deal with these two cases first:
    CHECK_HR_GOTO(hr = StartFrameStep());

    // Now resume the presentation loop.
    ProcessOutputLoop();

done:

    VSE_DP("OnClockRestart - Done");
    return hr;
}


//-----------------------------------------------------------------------------
// OnClockStop
//
// Called when the clock stops.
//-----------------------------------------------------------------------------

HRESULT CVideoSourceMSEvr::OnClockStop(MFTIME hnsSystemTime)
{
    VSE_DP("OnClockStop");

    CAutoLock lock(&m_ObjectLock);

    HRESULT hr = S_OK;
    CHECK_HR_GOTO(hr = CheckShutdown());

    if (m_RenderState != RENDER_STATE_STOPPED)
    {
        m_RenderState = RENDER_STATE_STOPPED;
        Flush();

        // If we are in the middle of frame-stepping, cancel it now.
        if (m_FrameStep.state != FRAMESTEP_NONE)
        {
            CancelFrameStep();
        }
    }

done:
    return hr;
}

//-----------------------------------------------------------------------------
// OnClockPause
//
// Called when the clock is paused.
//-----------------------------------------------------------------------------

HRESULT CVideoSourceMSEvr::OnClockPause(MFTIME hnsSystemTime)
{
    VSE_DP("OnClockPause");

    HRESULT hr = S_OK;
    
    CAutoLock lock(&m_ObjectLock);

    // We cannot pause the clock after shutdown.
    CHECK_HR_GOTO(hr = CheckShutdown());

    // Set the state. (No other actions are necessary.)
    m_RenderState = RENDER_STATE_PAUSED;

done:
    return hr;
}


//-----------------------------------------------------------------------------
// OnClockSetRate
//
// Called when the clock rate changes.
//-----------------------------------------------------------------------------

HRESULT CVideoSourceMSEvr::OnClockSetRate(MFTIME hnsSystemTime, float fRate)
{
    VSE_DP("OnClockSetRate (rate=%f)", fRate);

    // Note: 
    // The presenter reports its maximum rate through the IMFRateSupport interface.
    // Here, we assume that the EVR honors the maximum rate.

    CAutoLock lock(&m_ObjectLock);

    HRESULT hr = S_OK;
    CHECK_HR_GOTO(hr = CheckShutdown());

    // If the rate is changing from zero (scrubbing) to non-zero, cancel the 
    // frame-step operation.
    if ((m_fRate == 0.0f) && (fRate != 0.0f))
    {
        CancelFrameStep();
        m_FrameStep.samples.Clear();
    }

    m_fRate = fRate;

    // Tell the scheduler about the new rate.
    m_scheduler.SetClockRate(fRate);

done:
    return hr;
}


/////////////////////////////////////////////////////////////////////////////////
////
//// IMFRateSupport methods
////
////////////////////////////////////////////////////////////////////////////////
//
//
////-----------------------------------------------------------------------------
//// GetSlowestRate
////
//// Returns the slowest playback rate that the presenter supports.
////-----------------------------------------------------------------------------
//
//HRESULT CVideoSourceMSEvr::GetSlowestRate(MFRATE_DIRECTION eDirection, BOOL bThin, float *pfRate)
//{
//    CAutoLock lock(&m_ObjectLock);
//
//    HRESULT hr = S_OK;
//
//    CHECK_HR_GOTO(hr = CheckShutdown());
//    CheckPointer(pfRate, E_POINTER);
//
//    // There is no minimum playback rate, so the minimum is zero.
//    *pfRate = 0; 
//
//done:
//    return S_OK;
//}
//
//
////-----------------------------------------------------------------------------
//// GetFastestRate
////
//// Returns the fastest playback rate that the presenter supports.
////-----------------------------------------------------------------------------
//
//HRESULT CVideoSourceMSEvr::GetFastestRate(MFRATE_DIRECTION eDirection, BOOL bThin, float *pfRate)
//{
//    CAutoLock lock(&m_ObjectLock);
//
//    HRESULT hr = S_OK;
//    float   fMaxRate = 0.0f;
//
//    CHECK_HR_GOTO(hr = CheckShutdown());
//    CheckPointer(pfRate, E_POINTER);
//
//    // Get the maximum *forward* rate.
//    fMaxRate = GetMaxRate(bThin);
//
//    // For reverse playback, it's the negative of fMaxRate.
//    if (eDirection == MFRATE_REVERSE)
//    {
//        fMaxRate = -fMaxRate;
//    }
//
//    *pfRate = fMaxRate;
//
//done:
//    return hr;
//}
//
//
////-----------------------------------------------------------------------------
//// IsRateSupported
////
//// Checks whether a specified playback rate is supported.
////
//// bThin: If TRUE, the query is for thinned playback. Otherwise, the query
////        is for non-thinned playback.
//// fRate: Playback rate. This value is negative for reverse playback.
//// pfNearestSupportedRate: 
////        Receives the rate closest to fRate that the presenter supports.
////        This parameter can be NULL.
////-----------------------------------------------------------------------------
//
//HRESULT CVideoSourceMSEvr::IsRateSupported(BOOL bThin, float fRate, float *pfNearestSupportedRate)
//{
//    CAutoLock lock(&m_ObjectLock);
//
//    HRESULT hr = S_OK;
//    float   fMaxRate = 0.0f;
//    float   fNearestRate = fRate;   // If we support fRate, then fRate *is* the nearest.
//
//    CHECK_HR_GOTO(hr = CheckShutdown());
//
//    // Find the maximum forward rate.
//    // Note: We have no minimum rate (ie, we support anything down to 0).
//    fMaxRate = GetMaxRate(bThin);
//
//    if (fabsf(fRate) > fMaxRate)
//    {
//        // The (absolute) requested rate exceeds the maximum rate.
//        hr = MF_E_UNSUPPORTED_RATE;
//
//        // The nearest supported rate is fMaxRate.
//        fNearestRate = fMaxRate;
//        if (fRate < 0)
//        {
//            // Negative for reverse playback.
//            fNearestRate = -fNearestRate;
//        }
//    }
//
//    // Return the nearest supported rate.
//    if (pfNearestSupportedRate != NULL)
//    {
//        *pfNearestSupportedRate = fNearestRate;
//    }
//
//done:
//    return hr;
//}
//

///////////////////////////////////////////////////////////////////////////////
//
// IMFVideoDisplayControl methods
//
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
// GetNativeVideoSize
//-----------------------------------------------------------------------------

STDMETHODIMP CVideoSourceMSEvr::GetNativeVideoSize(SIZE* pszVideo, SIZE* pszARVideo)
{
	if (m_pMediaType)
	{
		if(pszVideo)
		{
			pszVideo->cx = m_lImageWidth;
			pszVideo->cy = m_lImageHeight;
		}
		if(pszARVideo)
		{
			GetIdealVideoSize(0, pszARVideo);
		}
	}
	else
	{
	    //Clear if media type not exist.
        if (pszVideo)
        {
            ZeroMemory(pszVideo, sizeof(SIZE));
        }
        if (pszARVideo)
        {
            ZeroMemory(pszARVideo, sizeof(SIZE));
        }
	}
	return S_OK;
}

//-----------------------------------------------------------------------------
// GetIdealVideoSize
//-----------------------------------------------------------------------------

STDMETHODIMP CVideoSourceMSEvr::GetIdealVideoSize(SIZE* pszMin, SIZE* pszMax)
{
    if (pszMax)
    {
        if (m_fNativeAspectRatio)
            pszMax->cx = (INT)ceil((float)m_lImageHeight * m_fNativeAspectRatio);
        else
            pszMax->cx = m_lImageWidth;

        pszMax->cy = m_lImageHeight;
    }
    if (pszMin)
    {
        if (m_fNativeAspectRatio)
            pszMax->cx = (INT)ceil((float)m_lImageHeight * m_fNativeAspectRatio);
        else
            pszMax->cx = m_lImageWidth;

        pszMax->cy = m_lImageHeight;
    }
    return S_OK;
}

//-----------------------------------------------------------------------------
// SetVideoWindow
//
// Sets the window where the presenter will draw video frames.
// Note: Does not fail after shutdown.
//-----------------------------------------------------------------------------

HRESULT CVideoSourceMSEvr::SetVideoWindow(HWND hwndVideo)
{
    CAutoLock lock(&m_ObjectLock);
	//Integrated from CVideoSourceEvr
    if (!IsWindow(hwndVideo))
    {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
//     NotifyEvent(EC_DISPLAY_CHANGED, 0, 0);  
	m_hwndVideo = hwndVideo;
    return hr;
}

//-----------------------------------------------------------------------------
// GetVideoWindow
//
// Returns a handle to the video window. 
// Note: Does not fail after shutdown.
//-----------------------------------------------------------------------------

HRESULT CVideoSourceMSEvr::GetVideoWindow(HWND* phwndVideo)
{
    CAutoLock lock(&m_ObjectLock);
	//Integrated from CVideoSourceEvr
	return E_NOTIMPL;
}


//-----------------------------------------------------------------------------
// SetVideoPosition
//
// Sets the source and target rectangles for the video window.
// Note: Does not fail after shutdown.
//-----------------------------------------------------------------------------

HRESULT CVideoSourceMSEvr::SetVideoPosition(const MFVideoNormalizedRect* pnrcSource, const LPRECT prcDest)
{
    CAutoLock lock(&m_ObjectLock);
	//Integrated from CVideoSourceEvr

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
			return E_FAIL;
		}

		// The source rectangle has range (0..1)
		if ((pnrcSource->left < 0) || (pnrcSource->right > 1) ||
			(pnrcSource->top < 0) || (pnrcSource->bottom > 1))
		{
			return E_FAIL;
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
			return E_FAIL;
		}

		// Update the destination rectangle.
		m_dstrect.left = prcDest->left;
		m_dstrect.top = prcDest->top;
		m_dstrect.right = prcDest->right;
		m_dstrect.bottom = prcDest->bottom;
	}

	return S_OK;
}


//-----------------------------------------------------------------------------
// GetVideoPosition
//
// Gets the current source and target rectangles.
// Note: Does not fail after shutdown.
//-----------------------------------------------------------------------------

HRESULT CVideoSourceMSEvr::GetVideoPosition(MFVideoNormalizedRect* pnrcSource, LPRECT prcDest)
{
    CAutoLock lock(&m_ObjectLock);
	//Integrated from CVideoSourceEvr

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

//-----------------------------------------------------------------------------
// SetAspectRatioMode
//-----------------------------------------------------------------------------

HRESULT CVideoSourceMSEvr::SetAspectRatioMode(DWORD dwAspectRatioMode)
{
	if (dwAspectRatioMode == MFVideoARMode_None || dwAspectRatioMode == MFVideoARMode_PreservePicture)
		m_dwAspectRatioMode = dwAspectRatioMode;
	else
		return E_NOTIMPL;

	return S_OK;
}

//-----------------------------------------------------------------------------
// GetAspectRatioMode
//-----------------------------------------------------------------------------

HRESULT CVideoSourceMSEvr::GetAspectRatioMode(DWORD*  pdwAspectRatioMode)
{
	ASSERT(pdwAspectRatioMode);
	*pdwAspectRatioMode = m_dwAspectRatioMode;
	return S_OK;
}

//-----------------------------------------------------------------------------
// RepaintVideo
// Repaints the most recent video frame.
//-----------------------------------------------------------------------------

HRESULT CVideoSourceMSEvr::RepaintVideo()
{
	VSE_DP("RepaintVideo");

	CAutoLock lock(&m_ObjectLock);

    HRESULT hr = S_OK;

    hr = CheckShutdown();
	if (FAILED(hr))
		return hr;

    // Ignore the request if we have not presented any samples yet.
    if (m_bPrerolled)
    {
        m_bRepaint = TRUE;
        (void)ProcessOutput();
		if (m_pDispObj)
		{
			hr = m_pDispObj->ProcessMessage(m_hwndVideo, VIDEOSOURCEDO_REQUEST_UPDATE_SURFACE, 0, 0);
			hr = m_pDispObj->NotifyEvent(DISPLAY_EVENT_VideoSourceRender, 0, 0, this);
			hr = m_pDispObj->NotifyEvent(DISPLAY_EVENT_VideoSourcePresent, 0, 0, this);
		}
    }

    return S_OK;
}

//-----------------------------------------------------------------------------
// RepaintVideo
// Repaints the most recent video frame.
//-----------------------------------------------------------------------------
HRESULT CVideoSourceMSEvr::GetCurrentImage(BITMAPINFOHEADER* pBih, BYTE** pDib, DWORD* pcbDib, LONGLONG* pTimeStamp)
{
	CHECK_POINTER(pBih)
	CHECK_POINTER(pDib)
	CHECK_POINTER(pcbDib)
	if (pBih->biSize != sizeof(BITMAPINFOHEADER))
		return E_INVALIDARG;

	CComPtr<IDirect3DSurface9> pSurface9;
	CComPtr<IMFSample> pSample;
    HRESULT hr = m_scheduler.PeekPresentSample(&pSample);

    if (SUCCEEDED(hr) && pSample)
    {
        CComPtr<IMFMediaBuffer> pBuffer;

        hr = pSample->GetBufferByIndex(0, &pBuffer);
        if (SUCCEEDED(hr) && pBuffer)
        {
            CComQIPtr<IMFGetService> pGetSurface = pBuffer;
            if (pGetSurface)
            {
                hr = pGetSurface->GetService(MR_BUFFER_SERVICE, __uuidof(IDirect3DSurface9), (void **)&pSurface9);
            }
        }
    }

	if (FAILED(hr))
        return E_FAIL;

	D3DSURFACE_DESC desc;

	hr = pSurface9->GetDesc(&desc);

	CComPtr<IDirect3DDevice9> pDevice;
	hr = m_pVideoSink->Get3DDevice((IUnknown**)&pDevice);

	if (FAILED(hr) || !pDevice)
		return E_FAIL;

	RECT rectSrc = {0, 0, m_lImageWidth, m_lImageHeight},rectDest = {0};

	D3DTEXTUREFILTERTYPE TexFilterType = D3DTEXF_LINEAR;
	rectDest.bottom = m_lImageHeight;

	if (m_fNativeAspectRatio != 0)
		rectDest.right = (LONG)ceil((float)m_lImageHeight * m_fNativeAspectRatio);
	else
		rectDest.right = m_lImageWidth;

	UINT OutputWidth = rectDest.right, OutputHeight = rectDest.bottom;

	if (memcmp(&rectSrc,&rectDest, sizeof(RECT)) == 0)
		TexFilterType = D3DTEXF_POINT;

	unsigned int w, h;
	w = 2;
	while (w < OutputWidth)
		w <<= 1;

	h = 2;
	while (h < OutputHeight)
		h <<= 1;

	CComPtr<IDirect3DTexture9> pARGBTexture9;
	CComPtr<IDirect3DSurface9> pARGBSurface9;

	CComPtr<IDirect3DTexture9> pDestTexture9;
	CComPtr<IDirect3DSurface9> pDestSurface9;

	hr = pDevice->CreateTexture(w, h, 0, D3DUSAGE_RENDERTARGET, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &pARGBTexture9, NULL);
	hr = pDevice->CreateTexture(w, h, 0, D3DUSAGE_DYNAMIC, D3DFMT_X8R8G8B8, D3DPOOL_SYSTEMMEM, &pDestTexture9, NULL);
	if (!pARGBTexture9 && !pDestTexture9)
		return E_FAIL;

	hr = pARGBTexture9->GetSurfaceLevel( 0, &pARGBSurface9);
	hr = pDestTexture9->GetSurfaceLevel( 0, &pDestSurface9);
	if (!pARGBSurface9 && !pDestSurface9)
		return E_FAIL;

	hr = pDevice->StretchRect(pSurface9, &rectSrc, pARGBSurface9, &rectDest, TexFilterType);
	if (FAILED(hr))
		return hr;

	hr = pDevice->GetRenderTargetData(pARGBSurface9, pDestSurface9);

	//D3DXSaveSurfaceToFile("c:\\texture.bmp", D3DXIFF_BMP, pDestSurface9, NULL, &rectDest);

	UINT BmpheaderSize = sizeof(BITMAPINFOHEADER);
	UINT BufSize = (OutputWidth * OutputHeight * 4);
	*pDib = (BYTE *)CoTaskMemAlloc(BufSize);
	if ((*pDib) == NULL)
		return E_UNEXPECTED;

	pBih->biWidth = OutputWidth;
	pBih->biHeight = OutputHeight;
	pBih->biPlanes = 1;
	pBih->biBitCount = 32;
	pBih->biCompression = BI_RGB;
	pBih->biSizeImage = BufSize;
	pBih->biXPelsPerMeter = 0;
	pBih->biYPelsPerMeter = 0;
	pBih->biClrUsed = 0;
	pBih->biClrImportant = 0;

	D3DLOCKED_RECT LockedTex;
	hr = pDestSurface9->LockRect(&LockedTex, NULL, D3DLOCK_READONLY);
	if (SUCCEEDED(hr))
	{
		BYTE* pDestBuf = (*pDib);
		BYTE* pStartBuf = (BYTE *)LockedTex.pBits;
		BYTE* pSourceBuf = pStartBuf;

		for (UINT i = 1;i <= OutputHeight; i++)
		{
			pSourceBuf = pStartBuf + (LockedTex.Pitch * (OutputHeight - i));
			memcpy(pDestBuf, pSourceBuf, (OutputWidth * 4));
			pDestBuf += (OutputWidth * 4);
		}
		pDestSurface9->UnlockRect();

		*pcbDib = BufSize;
	}

	////Save to File
	//{
	//	HANDLE fh;
	//	DWORD nWritten;
	//	BITMAPFILEHEADER bmpfileheader;
	//	ZeroMemory( &bmpfileheader , sizeof(BITMAPFILEHEADER));
	//	{
	//		bmpfileheader.bfType = ('M' << 8) | 'B';
	//		bmpfileheader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + BufSize;
	//		bmpfileheader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	//		fh = CreateFile("c:\\capture.bmp",
	//			GENERIC_WRITE, 0, NULL,
	//			CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	//		WriteFile(fh, &bmpfileheader, sizeof(bmpfileheader), &nWritten, NULL);
	//		WriteFile(fh, pBih, sizeof(BITMAPINFOHEADER), &nWritten, NULL);
	//		WriteFile(fh, *pDib, BufSize, &nWritten, NULL);
	//		CloseHandle(fh);
	//	}
	//}
	return hr;

}

//////////////////////////////////////////////////////////////////////////
// IQualProp
STDMETHODIMP CVideoSourceMSEvr::get_FramesDroppedInRenderer(int *pcFrames)
{
	CHECK_POINTER(pcFrames)
		VSE_DP("get_FramesDroppedInRenderer");
	return  m_scheduler.get_FramesDroppedInRenderer(pcFrames);
}
STDMETHODIMP CVideoSourceMSEvr::get_FramesDrawn(int *pcFramesDrawn)
{
	CHECK_POINTER(pcFramesDrawn)

		VSE_DP("get_FramesDrawn");

	return m_scheduler.get_FramesDrawn(pcFramesDrawn);
//	*pcFramesDrawn = m_QualPropInfo.FramesDrawn;
//	return S_OK;
}
STDMETHODIMP CVideoSourceMSEvr::get_AvgFrameRate(int *piAvgFrameRate)
{
	CHECK_POINTER(piAvgFrameRate)

		VSE_DP("get_AvgFrameRate");

	return m_scheduler.get_AvgFrameRate(piAvgFrameRate);
//	*piAvgFrameRate = m_QualPropInfo.AvgFrameRate; //according to spec, multiply 100.
//	return S_OK;
}
STDMETHODIMP CVideoSourceMSEvr::get_Jitter(int *iJitter)
{
	CHECK_POINTER(iJitter)

		VSE_DP("get_Jitter");

	return m_scheduler.get_Jitter(iJitter);
//	*iJitter = m_QualPropInfo.Jitter;
//	return S_OK;
}
STDMETHODIMP CVideoSourceMSEvr::get_AvgSyncOffset(int *piAvg)
{
	CHECK_POINTER(piAvg)

		VSE_DP("get_AvgSyncOffset");

	return m_scheduler.get_AvgSyncOffset(piAvg);
//	*piAvg = m_QualPropInfo.AvgSyncOffset;
//	return S_OK;
}
STDMETHODIMP CVideoSourceMSEvr::get_DevSyncOffset(int *piDev)
{
	CHECK_POINTER(piDev)

		VSE_DP("get_DevSyncOffset");

	return m_scheduler.get_DevSyncOffset(piDev);

//	*piDev = m_QualPropInfo.FramesDropped;
//	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
// IEvrSchedulerCallback
//////////////////////////////////////////////////////////////////////////
STDMETHODIMP CVideoSourceMSEvr::OnUpdateLastSample(IMFSample *pSample, LONGLONG llDelta)
{
	VSE_DP("OnUpdateLastSample pSample = 0x%x", pSample);
	//move codes from OnSampleFree
	//We keep last sample in Scheduler, therefore we cannot handle completeframestep in OnSampleFree.
	HRESULT hr = E_FAIL;
	IUnknown *pUnk = NULL;

	m_bHoldPresentSample = pSample ? TRUE : FALSE;

	if (m_FrameStep.state == FRAMESTEP_SCHEDULED) 
	{
		// QI the sample for IUnknown and compare it to our cached value.
		hr = pSample->QueryInterface(__uuidof(IMFSample), (void**)&pUnk);
		if (SUCCEEDED(hr))
		{
			if (m_FrameStep.pSampleNoRef == (DWORD_PTR)pUnk)
			{
				// Notify the EVR. 
				hr = CompleteFrameStep(pSample);
			}
			SAFE_RELEASE(pUnk);
		}
		// Note: Although pObject is also an IUnknown pointer, it's not guaranteed
		// to be the exact pointer value returned via QueryInterface, hence the 
		// need for the second QI.
	}
	else
		hr = S_OK;

	m_pDispObj->ProcessMessage(m_hwndVideo, VIDEOSOURCEDO_REQUEST_UPDATE_SURFACE, 0, 0);

	if (m_bHoldPresentSample)
	{
		LONGLONG llDuration = 0;

		pSample->GetSampleDuration(&llDuration);
		if (!m_DisplayHelper.IsOverDisplayFrameRate(llDuration) && !m_DisplayHelper.IsSkipFrameForVideoSync(llDuration, llDelta))
		{
			hr = m_pDispObj->NotifyEvent(DISPLAY_EVENT_VideoSourceRender, 0, 0, m_pDispObj);
			hr = m_pDispObj->NotifyEvent(DISPLAY_EVENT_VideoSourcePresent, 0, 0, m_pDispObj);
		}
	}

	return hr;
}

///////////////////////////////////////////////////////////////////////////////
//
// Private / Protected methods
//
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------

//CVideoSourceMSEvr::CVideoSourceMSEvr(HRESULT& hr) :
CVideoSourceMSEvr::CVideoSourceMSEvr(IDisplayLock* pLock, IDisplayVideoSink* pVideoSink)
: CUnknown(NAME("CVideoSourceMSEvr"), NULL),
    m_RenderState(RENDER_STATE_SHUTDOWN),
//    m_pD3DPresentEngine(NULL),
    m_pClock(NULL),
    m_pMixer(NULL),
    m_pMediaEventSink(NULL),
    m_pMediaType(NULL),
    m_bSampleNotify(FALSE),
    m_bRepaint(FALSE),
    m_bEndStreaming(FALSE),
    m_bPrerolled(FALSE),
    m_fRate(1.0f),
    m_TokenCounter(0),
	m_bDShowWorkaround(FALSE),
	m_pCustomMixer(NULL),
    m_SampleFreeCB(this, &CVideoSourceMSEvr::OnSampleFree),
	m_SamplePool(PRESENTER_BUFFER_COUNT) 	// it is advised to have queue number larger than 3
{
	m_lImageWidth = 0L;
	m_lImageHeight = 0L;
	m_dwOutputStreamId = 0;

	m_pLock = pLock;
	m_pVideoSink = pVideoSink;

	m_bValid = TRUE;
	m_uDeviceManagerToken = 0;
	m_hDeviceHandle = NULL;
	m_bInitiativeDisplay = FALSE;
	m_pVideoSink->QueryInterface(__uuidof(IDisplayObject), (void **)&m_pDispObj);
	m_pStateEventSink = CServerStateEventSink::GetInstance();

    // Initial source rectangle = (0,0,1,1)
    m_nrcSource.top = m_nrcSource.left = 0;
    m_nrcSource.bottom = m_nrcSource.right = 1;
    m_nrcTexture.left = m_nrcTexture.top = 0.0;
    m_nrcTexture.right = m_nrcTexture.bottom = 1.0;
	m_hwndVideo	= 0;
	memset(&m_dstrect, 0, sizeof(m_dstrect));
	m_fAspectRatio = 0.0;
	m_fNativeAspectRatio = 0.0;
	m_dwAspectRatioMode = MFVideoARMode_None;
	m_bHoldPresentSample = FALSE;
   // m_pD3DPresentEngine = new D3DPresentEngine(hr);
   // if (m_pD3DPresentEngine == NULL)
   //{
   //     hr = E_OUTOFMEMORY;
   // }
   // CHECK_HR_GOTO(hr);

   //m_scheduler.SetCallback(m_pD3DPresentEngine);
	m_scheduler.SetEvrSchedulerCallback(this);

//done:
//    if (FAILED(hr))
//    {
////        SAFE_DELETE(m_pD3DPresentEngine);
//    }
}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------

CVideoSourceMSEvr::~CVideoSourceMSEvr()
{
	Cleanup();
    // COM interfaces
    SAFE_RELEASE(m_pClock);
    SAFE_RELEASE(m_pMixer);
    SAFE_RELEASE(m_pMediaEventSink);
    SAFE_RELEASE(m_pMediaType);
    // Deletable objects
//    SAFE_DELETE(m_pD3DPresentEngine);
}

void CVideoSourceMSEvr::Cleanup()
{
//	StopEvrPresentThread();
//	ReleaseServicePointers();
	Flush();

	BeginDeviceLoss();
//	m_pMediaType.Release();
	SAFE_RELEASE(m_pCustomMixer);
	m_pGraph.Release();
	IDirect3DDeviceManager9 *pDeviceManager = m_pDeviceManager.Detach();
	if (pDeviceManager)
	{
		int ref = pDeviceManager->Release();
		if (ref > 0)
		{
			VSE_DP("Cleanup: m_pDeviceManager ref count = %d, should be 0.\n", ref);
		}
	}
	m_uDeviceManagerToken = 0;
	IBaseFilter *pVMR = m_pVMR.Detach();
	if (pVMR)
	{
		int ref = pVMR->Release();
		if (ref > 0)
		{
			VSE_DP("Cleanup: m_pVMR ref count = %d, should be 0.\n", ref);
		}
	}
	m_lImageWidth = m_lImageHeight = 0L;
}
//-----------------------------------------------------------------------------
// ConfigureMixer
//
// Initializes the mixer. Called from InitServicePointers.
//-----------------------------------------------------------------------------

HRESULT CVideoSourceMSEvr::ConfigureMixer(IMFTransform *pMixer)
{
    HRESULT             hr = S_OK;
    IID                 deviceID = GUID_NULL;

    IMFVideoDeviceID    *pDeviceID = NULL;

    // Make sure that the mixer has the same device ID as ourselves.
    CHECK_HR_GOTO(hr = pMixer->QueryInterface(__uuidof(IMFVideoDeviceID), (void**)&pDeviceID));
    CHECK_HR_GOTO(hr = pDeviceID->GetDeviceID(&deviceID));

    if (!IsEqualGUID(deviceID, __uuidof(IDirect3DDevice9)))
    {
        CHECK_HR_GOTO(hr = MF_E_INVALIDREQUEST);
    }

    // Set the zoom rectangle (ie, the source clipping rectangle).
    SetMixerSourceRect(pMixer, m_nrcSource);

done:
    SAFE_RELEASE(pDeviceID);
    return hr;
}



//-----------------------------------------------------------------------------
// RenegotiateMediaType
//
// Attempts to set an output type on the mixer.
//-----------------------------------------------------------------------------

HRESULT CVideoSourceMSEvr::RenegotiateMediaType()
{
    VSE_DP("RenegotiateMediaType");

    HRESULT hr;
    DWORD dwInputIDArraySize, dwOutputIDArraySize;
    DWORD *pdwInputIDs = 0, *pdwOutputIDs = 0;

	CAutoLock lock(&m_ObjectLock);

// Should not clean current media type, 
//	if media type doesn't change, we don't need to re-allocate samples.
//	m_pMediaType = NULL;

    hr = m_pMixer->GetStreamCount(&dwInputIDArraySize, &dwOutputIDArraySize);
    if (SUCCEEDED(hr))
    {
        pdwInputIDs = new DWORD[dwInputIDArraySize];
        pdwOutputIDs = new DWORD[dwOutputIDArraySize];
        hr = m_pMixer->GetStreamIDs(dwInputIDArraySize, pdwInputIDs, dwOutputIDArraySize, pdwOutputIDs);
    }

    if (SUCCEEDED(hr))
    {
		CComPtr<IDispSvrPlugin> pDispSvrVideoPresenter;
		BOOL bIsSORTPresenter = FALSE;
		hr = CResourceManager::GetInstance()->GetInterface(DISPSVR_RESOURCE_VIDEOPRESENTER, __uuidof(IDispSvrPlugin), (void**)&pDispSvrVideoPresenter);
		if (SUCCEEDED(hr))
		{
			GUID guidResID = GUID_NULL;
			pDispSvrVideoPresenter->GetResourceId(&guidResID);
			if (DISPSVR_RESOURCE_AMDSORTVIDEOPRESENTER == guidResID)
				bIsSORTPresenter = TRUE;
		}

        CComPtr<IMFMediaType> pType, pMixerCurrentType;
        hr = S_OK;
        UINT i = 0;
		int iRGBIdx = -1, iYUVIdx = -1, iConnectMTIdx = -1;
		bool bForceUseRGBOutput = false;
		DWORD dwCurInSubFourCC = 0;

		if (SUCCEEDED(m_pMixer->GetInputCurrentType(pdwInputIDs[0], &pMixerCurrentType)))
		{
			GUID guidInSubtype = GUID_NULL;
			if (SUCCEEDED(pMixerCurrentType->GetGUID(MF_MT_SUBTYPE, &guidInSubtype)))
				dwCurInSubFourCC = guidInSubtype.Data1;
		}

#ifdef _DEBUG
        DumpOutputAvailableType();
#endif

        while (hr != MF_E_NO_MORE_TYPES)
        {
			GUID guidOutSubtype = GUID_NULL;
            pType.Release();
            hr = m_pMixer->GetOutputAvailableType(pdwOutputIDs[0], i++, &pType);
            if (SUCCEEDED(hr) && SUCCEEDED(pType->GetGUID(MF_MT_SUBTYPE, &guidOutSubtype)))
            {
				DWORD dwOutSubFourCC = guidOutSubtype.Data1;

				if (D3DFMT_X8R8G8B8 == dwOutSubFourCC || D3DFMT_A8R8G8B8 == dwOutSubFourCC)
				{
					iRGBIdx = i-1;
				}
				else if (CVideoSourceDisplayHelper::IsYUVFormat(dwOutSubFourCC))
				{
					if (bIsSORTPresenter && PLANE_FORMAT_NV12 == dwOutSubFourCC)
					{
						bForceUseRGBOutput = true;
					}
					else
					{
						hr = m_pMixer->SetOutputType(pdwOutputIDs[0], pType, MFT_SET_TYPE_TEST_ONLY);  //Test if media type can connect, not change
						if (FAILED(hr))
							continue;

						if (dwOutSubFourCC == dwCurInSubFourCC)
						{
							iConnectMTIdx = i-1;
							break;
						}
						else
						{
							iYUVIdx = i-1;
						}
					}
				}
			}
		}

		if (-1 == iConnectMTIdx)
		{
			iConnectMTIdx = (bForceUseRGBOutput || (-1 == iYUVIdx)) ? iRGBIdx : iYUVIdx;
		}

		if (iConnectMTIdx >= 0)
		{
			if (iRGBIdx == iConnectMTIdx)
			{
				VSE_DP("Mixer doesn't support YUV format. use RGB format.\n");
			}
			pType.Release();
			hr = m_pMixer->GetOutputAvailableType(pdwOutputIDs[0], iConnectMTIdx, &pType);
			if (SUCCEEDED(hr))
			{
				hr = m_pMixer->SetOutputType(pdwOutputIDs[0], pType, 0);
				if (SUCCEEDED(hr))
				{
					m_dwOutputStreamId = pdwOutputIDs[0];
//                  hr = PrepareOutputMediaSamples(pType);
//                  if (FAILED(hr))
//						VSE_DP("ProcessMessage: MFVP_MESSAGE_INVALIDATEMEDIATYPE PrepareOutputMediaSample failed. hr=0x%x\n", hr);
				}
			}
		}

        if (SUCCEEDED(hr))
        {
            CComPtr<IMFMediaType> pCurrentType;
            hr = m_pMixer->GetOutputCurrentType(pdwOutputIDs[0], &pCurrentType);
            if (SUCCEEDED(hr))
            {
                SetMediaType(pCurrentType);
            }
        }

#ifdef _DEBUG
        if (SUCCEEDED(hr))
        {
            MFT_OUTPUT_STREAM_INFO StreamInfo;
            hr = m_pMixer->GetOutputStreamInfo(pdwOutputIDs[0], &StreamInfo);

            // presenter always needs to supply output media sample.
            ASSERT((StreamInfo.dwFlags & (MFT_OUTPUT_STREAM_PROVIDES_SAMPLES | MFT_OUTPUT_STREAM_CAN_PROVIDE_SAMPLES)) == 0);
        }
#endif	// _DEBUG
    }

    if (pdwInputIDs)
        delete [] pdwInputIDs;
    if (pdwOutputIDs)
        delete [] pdwOutputIDs;
    return hr;

    //HRESULT hr = S_OK;
    //BOOL bFoundMediaType = FALSE;

    //IMFMediaType *pMixerType = NULL;
    //IMFMediaType *pOptimalType = NULL;
    //IMFVideoMediaType *pVideoType = NULL;

    //if (!m_pMixer)
    //{
    //    return MF_E_INVALIDREQUEST;
    //}

    //// Loop through all of the mixer's proposed output types.
    //DWORD iTypeIndex = 0;
    //while (!bFoundMediaType && (hr != MF_E_NO_MORE_TYPES))
    //{
    //    SAFE_RELEASE(pMixerType);
    //    SAFE_RELEASE(pOptimalType);

    //    // Step 1. Get the next media type supported by mixer.
    //    hr = m_pMixer->GetOutputAvailableType(0, iTypeIndex++, &pMixerType);
    //    if (FAILED(hr))
    //    {
    //        break;
    //    }

    //    // From now on, if anything in this loop fails, try the next type,
    //    // until we succeed or the mixer runs out of types.

    //    // Step 2. Check if we support this media type. 
    //    if (SUCCEEDED(hr))
    //    {
    //        // Note: None of the modifications that we make later in CreateOptimalVideoType
    //        // will affect the suitability of the type, at least for us. (Possibly for the mixer.)
    //        hr = IsMediaTypeSupported(pMixerType);
    //    }

    //    // Step 3. Adjust the mixer's type to match our requirements.
    //    if (SUCCEEDED(hr))
    //    {
    //        hr = CreateOptimalVideoType(pMixerType, &pOptimalType);
    //    }

    //    // Step 4. Check if the mixer will accept this media type.
    //    if (SUCCEEDED(hr))
    //    {
    //        hr = m_pMixer->SetOutputType(0, pOptimalType, MFT_SET_TYPE_TEST_ONLY);
    //    }

    //    // Step 5. Try to set the media type on ourselves.
    //    if (SUCCEEDED(hr))
    //    {
    //        hr = SetMediaType(pOptimalType);
    //    }

    //    // Step 6. Set output media type on mixer.
    //    if (SUCCEEDED(hr))
    //    {
    //        hr = m_pMixer->SetOutputType(0, pOptimalType, 0);

    //        assert(SUCCEEDED(hr)); // This should succeed unless the MFT lied in the previous call.

    //        // If something went wrong, clear the media type.
    //        if (FAILED(hr))
    //        {
    //            SetMediaType(NULL);
    //        }
    //    }

    //    if (SUCCEEDED(hr))
    //    {
    //        bFoundMediaType = TRUE;
    //    }
    //}

    //SAFE_RELEASE(pMixerType);
    //SAFE_RELEASE(pOptimalType);
    //SAFE_RELEASE(pVideoType);

    //return hr;
}


//-----------------------------------------------------------------------------
// Flush
//
// Flushes any samples that are waiting to be presented.
//-----------------------------------------------------------------------------

HRESULT CVideoSourceMSEvr::Flush()
{
    m_bPrerolled = FALSE;

    // The scheduler might have samples that are waiting for
    // their presentation time. Tell the scheduler to flush.

    // This call blocks until the scheduler threads discards all scheduled samples.
    m_scheduler.Flush(m_RenderState == RENDER_STATE_STOPPED);

    // Flush the frame-step queue.
    m_FrameStep.samples.Clear();
	m_SampleInMixerList.Clear();

    if (m_RenderState == RENDER_STATE_STOPPED)
    {
        // Repaint with black.
        //(void)m_pD3DPresentEngine->PresentSample(NULL, 0);
    }

    return S_OK; 
}

//-----------------------------------------------------------------------------
// ProcessInputNotify
//
// Attempts to get a new output sample from the mixer.
//
// This method is called when the EVR sends an MFVP_MESSAGE_PROCESSINPUTNOTIFY 
// message, which indicates that the mixer has a new input sample. 
//
// Note: If there are multiple input streams, the mixer might not deliver an 
// output sample for every input sample. 
//-----------------------------------------------------------------------------

HRESULT CVideoSourceMSEvr::ProcessInputNotify()
{
    HRESULT hr = S_OK;

    // Set the flag that says the mixer has a new sample.
    m_bSampleNotify = TRUE;

    if (!m_hDeviceHandle)
    {
        // doing nothing if device is Lost
        hr = S_OK;
    }
    else if (m_pMediaType == NULL)
    {
        // We don't have a valid media type yet.
        hr = MF_E_TRANSFORM_TYPE_NOT_SET;
    }
    else
    {
        // Try to process an output sample.
        ProcessOutputLoop();
    }
    return hr;
}

//-----------------------------------------------------------------------------
// BeginStreaming
// 
// Called when streaming begins.
//-----------------------------------------------------------------------------

HRESULT CVideoSourceMSEvr::BeginStreaming()
{
    HRESULT hr = S_OK;

    // Start the scheduler thread. 
    hr = m_scheduler.StartScheduler(m_pClock);

    return hr;
}

//-----------------------------------------------------------------------------
// EndStreaming
// 
// Called when streaming ends.
//-----------------------------------------------------------------------------

HRESULT CVideoSourceMSEvr::EndStreaming()
{
    HRESULT hr = S_OK;
    
    // Stop the scheduler thread.
    hr = m_scheduler.StopScheduler();

    return hr;
}


//-----------------------------------------------------------------------------
// CheckEndOfStream
// Performs end-of-stream actions if the EOS flag was set.
//
// Note: The presenter can receive the EOS notification before it has finished 
// presenting all of the scheduled samples. Therefore, signaling EOS and 
// handling EOS are distinct operations.
//-----------------------------------------------------------------------------

HRESULT CVideoSourceMSEvr::CheckEndOfStream()
{
    if (!m_bEndStreaming)
    {
        // The EVR did not send the MFVP_MESSAGE_ENDOFSTREAM message.
		VSE_DP("CheckEndOfStream - m_bEndStreaming is false");
        return S_OK; 
    }

    if (m_bSampleNotify)
    {
        // The mixer still has input. 
		VSE_DP("CheckEndOfStream - m_bSampleNotify is true");
        return S_OK;
    }
  // We have a sample which is hold by Scheduler, therefore m_SamplePool.AreSamplesPending() is always higher than 0
  //  if (m_SamplePool.AreSamplesPending())
  //  {
  //      // Samples are still scheduled for rendering.
		//VSE_DP("CheckEndOfStream - m_SamplePool.AreSamplesPending() is true");
  //      return S_OK;
  //  }
	if (!m_pCustomMixer)
	{
		LONG PendingCount = m_SamplePool.GetSamplePendingCount();
		VSE_DP("CheckEndOfStream - m_SamplePool SamplePendingCount() =%d ", PendingCount);

		// Scheduler may hold one presentsample for preseting, so we need to decrease 1;
		// return S_OK and wait pending sample returns.

		PendingCount -= m_bHoldPresentSample ? 1 : 0;
		if (PendingCount > 0)
		{
			return S_OK;
		}
	}

    // Everything is complete. Now we can tell the EVR that we are done.
	VSE_DP("CheckEndOfStream - NotifyEvent");
    NotifyEvent(EC_COMPLETE, (LONG_PTR)S_OK, 0);
    m_bEndStreaming = FALSE;
    return S_OK;
}



//-----------------------------------------------------------------------------
// PrepareFrameStep
//
// Gets ready to frame step. Called when the EVR sends the MFVP_MESSAGE_STEP
// message.
//
// Note: The EVR can send the MFVP_MESSAGE_STEP message before or after the 
// presentation clock starts. 
//-----------------------------------------------------------------------------
HRESULT CVideoSourceMSEvr::PrepareFrameStep(DWORD cSteps)
{
	HRESULT hr = S_OK;

    // Cache the step count.
    m_FrameStep.steps += cSteps;

    // Set the frame-step state. 
    m_FrameStep.state = FRAMESTEP_WAITING_START;

    // If the clock is are already running, we can start frame-stepping now.
    // Otherwise, we will start when the clock starts.
    if (m_RenderState == RENDER_STATE_STARTED)
    {
        hr = StartFrameStep();       
    }

    return hr;
}

//-----------------------------------------------------------------------------
// StartFrameStep
//
// If the presenter is waiting to frame-step, this method starts the frame-step 
// operation. Called when the clock starts OR when the EVR sends the 
// MFVP_MESSAGE_STEP message (see PrepareFrameStep).
//-----------------------------------------------------------------------------

HRESULT CVideoSourceMSEvr::StartFrameStep()
{
    ASSERT(m_RenderState == RENDER_STATE_STARTED);

    HRESULT hr = S_OK;
    IMFSample *pSample = NULL;

    if (m_FrameStep.state == FRAMESTEP_WAITING_START)
    {

        // We have a frame-step request, and are waiting for the clock to start.
        // Set the state to "pending," which means we are waiting for samples.
        m_FrameStep.state = FRAMESTEP_PENDING;

        // If the frame-step queue already has samples, process them now.
        while (!m_FrameStep.samples.IsEmpty() && (m_FrameStep.state == FRAMESTEP_PENDING))
        {
            CHECK_HR_GOTO(hr = m_FrameStep.samples.RemoveFront(&pSample));
            CHECK_HR_GOTO(hr = DeliverFrameStepSample(pSample));
            SAFE_RELEASE(pSample);

            // We break from this loop when:
            //   (a) the frame-step queue is empty, or
            //   (b) the frame-step operation is complete.
        }
    }
    else if (m_FrameStep.state == FRAMESTEP_NONE)
    {
        // We are not frame stepping. Therefore, if the frame-step queue has samples, 
        // we need to process them normally.
        while (!m_FrameStep.samples.IsEmpty())
        {
            CHECK_HR_GOTO(hr = m_FrameStep.samples.RemoveFront(&pSample));
            CHECK_HR_GOTO(hr = DeliverSample(pSample, FALSE));
            SAFE_RELEASE(pSample);
        }
    }

done:
    SAFE_RELEASE(pSample);
    return hr;
}

//-----------------------------------------------------------------------------
// CompleteFrameStep
//
// Completes a frame-step operation. Called after the frame has been
// rendered.
//-----------------------------------------------------------------------------

HRESULT CVideoSourceMSEvr::CompleteFrameStep(IMFSample *pSample)
{
	VSE_DP("CompleteFrameStep");
    HRESULT hr = S_OK;
    MFTIME hnsSampleTime = 0;
    MFTIME hnsSystemTime = 0;

    // Update our state.
    m_FrameStep.state = FRAMESTEP_COMPLETE;
    m_FrameStep.pSampleNoRef = NULL;

    // Notify the EVR that the frame-step is complete.
    NotifyEvent(EC_STEP_COMPLETE, FALSE, 0); // FALSE = completed (not cancelled)

    // If we are scrubbing (rate == 0), also send the "scrub time" event.
    if (IsScrubbing())
    {
        // Get the time stamp from the sample.
        hr = pSample->GetSampleTime(&hnsSampleTime);
        if (FAILED(hr))
        {
            // No time stamp. Use the current presentation time.
            if (m_pClock)
            {
                (void)m_pClock->GetCorrelatedTime(0, &hnsSampleTime, &hnsSystemTime);
            }
            hr = S_OK; // (Not an error condition.)
        }

        NotifyEvent(EC_SCRUB_TIME, LODWORD(hnsSampleTime), HIDWORD(hnsSampleTime));
    }
    return hr;
}

//-----------------------------------------------------------------------------
// CancelFrameStep
//
// Cancels the frame-step operation.
//-----------------------------------------------------------------------------

HRESULT CVideoSourceMSEvr::CancelFrameStep()
{
	VSE_DP("CancelFrameStep");
    FRAMESTEP_STATE oldState = m_FrameStep.state;

    m_FrameStep.state = FRAMESTEP_NONE;
    m_FrameStep.steps = 0;
    m_FrameStep.pSampleNoRef = NULL;
    // Don't clear the frame-step queue yet, because we might frame step again.

	VSE_DP("CancelFrameStep - oldState = %d", oldState);

    if (oldState > FRAMESTEP_NONE && oldState < FRAMESTEP_COMPLETE)
    {
        // We were in the middle of frame-stepping when it was cancelled.
        // Notify the EVR.
        NotifyEvent(EC_STEP_COMPLETE, TRUE, 0); // TRUE = cancelled
    }
    return S_OK;
}


//-----------------------------------------------------------------------------
// CreateOptimalVideoType
//
// Converts a proposed media type from the mixer into a type that is suitable for the presenter.
// 
// pProposedType: Media type that we got from the mixer.
// ppOptimalType: Receives the modfied media type.
//
// The presenter will attempt to set ppOptimalType as the mixer's output format.
//-----------------------------------------------------------------------------

HRESULT CVideoSourceMSEvr::CreateOptimalVideoType(IMFMediaType* pProposedType, IMFMediaType **ppOptimalType)
{
    HRESULT hr = S_OK;
    /*
    RECT rcOutput;
    ZeroMemory(&rcOutput, sizeof(rcOutput));

    MFVideoArea displayArea;
    ZeroMemory(&displayArea, sizeof(displayArea));

    IMFMediaType *pOptimalType = NULL;
    VideoTypeBuilder *pmtOptimal = NULL;

    // Create the helper object to manipulate the optimal type.
    CHECK_HR_GOTO(hr = MediaTypeBuilder::Create(&pmtOptimal));

    // Clone the proposed type.
    CHECK_HR_GOTO(hr = pmtOptimal->CopyFrom(pProposedType));

    // Modify the new type.

    // For purposes of this SDK sample, we assume 
    // 1) The monitor's pixels are square.
    // 2) The presenter always preserves the pixel aspect ratio.

    // Set the pixel aspect ratio (PAR) to 1:1 (see assumption #1, above)
    CHECK_HR_GOTO(hr = pmtOptimal->SetPixelAspectRatio(1, 1));

    // Get the output rectangle.
    rcOutput = m_pD3DPresentEngine->GetDestinationRect();
    if (IsRectEmpty(&rcOutput))
    {
        // Calculate the output rectangle based on the media type.
        CHECK_HR_GOTO(hr = CalculateOutputRectangle(pProposedType, &rcOutput));
    }

    // Set the extended color information: Use BT.709 
    CHECK_HR_GOTO(hr = pmtOptimal->SetYUVMatrix(MFVideoTransferMatrix_BT709));
    CHECK_HR_GOTO(hr = pmtOptimal->SetTransferFunction(MFVideoTransFunc_709));
    CHECK_HR_GOTO(hr = pmtOptimal->SetVideoPrimaries(MFVideoPrimaries_BT709));
    CHECK_HR_GOTO(hr = pmtOptimal->SetVideoNominalRange(MFNominalRange_16_235));
    CHECK_HR_GOTO(hr = pmtOptimal->SetVideoLighting(MFVideoLighting_dim));

    // Set the target rect dimensions. 
    CHECK_HR_GOTO(hr = pmtOptimal->SetFrameDimensions(rcOutput.right, rcOutput.bottom));

    // Set the geometric aperture, and disable pan/scan.
    displayArea = MakeArea(0, 0, rcOutput.right, rcOutput.bottom);

    CHECK_HR_GOTO(hr = pmtOptimal->SetPanScanEnabled(FALSE));

    CHECK_HR_GOTO(hr = pmtOptimal->SetGeometricAperture(displayArea));

    // Set the pan/scan aperture and the minimum display aperture. We don't care
    // about them per se, but the mixer will reject the type if these exceed the 
    // frame dimentions.
    CHECK_HR_GOTO(hr = pmtOptimal->SetPanScanAperture(displayArea));
    CHECK_HR_GOTO(hr = pmtOptimal->SetMinDisplayAperture(displayArea));

    // Return the pointer to the caller.
    CHECK_HR_GOTO(hr = pmtOptimal->GetMediaType(&pOptimalType));

    *ppOptimalType = pOptimalType;
    (*ppOptimalType)->AddRef();

done:
    SAFE_RELEASE(pOptimalType);
    SAFE_RELEASE(pmtOptimal);
	*/
    return hr;

}

//-----------------------------------------------------------------------------
// CalculateOutputRectangle
// 
// Calculates the destination rectangle based on the mixer's proposed format.
// This calculation is used if the application did not specify a destination 
// rectangle.
//
// Note: The application sets the destination rectangle by calling 
// IMFVideoDisplayControl::SetVideoPosition.
//
// This method finds the display area of the mixer's proposed format and
// converts it to the pixel aspect ratio (PAR) of the display.
//-----------------------------------------------------------------------------

HRESULT CVideoSourceMSEvr::CalculateOutputRectangle(IMFMediaType *pProposedType, RECT *prcOutput)
{
    HRESULT hr = S_OK;
	/*
    UINT32  srcWidth = 0, srcHeight = 0;

    MFRatio inputPAR = { 0, 0 };
    MFRatio outputPAR = { 0, 0 };
    RECT    rcOutput = { 0, 0, 0, 0};

    MFVideoArea displayArea;
    ZeroMemory(&displayArea, sizeof(displayArea));

    VideoTypeBuilder *pmtProposed = NULL;

    // Helper object to read the media type.
    CHECK_HR_GOTO(hr = MediaTypeBuilder::Create(pProposedType, &pmtProposed));

    // Get the source's frame dimensions.
    CHECK_HR_GOTO(hr = pmtProposed->GetFrameDimensions(&srcWidth, &srcHeight));

    // Get the source's display area. 
    CHECK_HR_GOTO(hr = pmtProposed->GetVideoDisplayArea(&displayArea));

    // Calculate the x,y offsets of the display area.
    LONG offsetX = GetOffset(displayArea.OffsetX);
    LONG offsetY = GetOffset(displayArea.OffsetY);

    // Use the display area if valid. Otherwise, use the entire frame.
    if (displayArea.Area.cx != 0 &&
        displayArea.Area.cy != 0 &&
        offsetX + displayArea.Area.cx <= (LONG)(srcWidth) &&
        offsetY + displayArea.Area.cy <= (LONG)(srcHeight))
    {
        rcOutput.left   = offsetX;
        rcOutput.right  = offsetX + displayArea.Area.cx;
        rcOutput.top    = offsetY;
        rcOutput.bottom = offsetY + displayArea.Area.cy;
    }
    else
    {
        rcOutput.left = 0;
        rcOutput.top = 0;
        rcOutput.right = srcWidth;
        rcOutput.bottom = srcHeight;
    }

    // rcOutput is now either a sub-rectangle of the video frame, or the entire frame.

    // If the pixel aspect ratio of the proposed media type is different from the monitor's, 
    // letterbox the video. We stretch the image rather than shrink it.

    inputPAR = pmtProposed->GetPixelAspectRatio();    // Defaults to 1:1

    outputPAR.Denominator = outputPAR.Numerator = 1; // This is an assumption of the sample.

    // Adjust to get the correct picture aspect ratio.
    *prcOutput = CorrectAspectRatio(rcOutput, inputPAR, outputPAR);

done:
    SAFE_RELEASE(pmtProposed);
	*/
    return hr;
}


//-----------------------------------------------------------------------------
// SetMediaType
//
// Sets or clears the presenter's media type. 
// The type has already been validated.
//-----------------------------------------------------------------------------
HRESULT CVideoSourceMSEvr::SetMediaType(IMFMediaType *pMediaType)
{
    // Note: pMediaType can be NULL (to clear the type)

    // Clearing the media type is allowed in any state (including shutdown).
    if (pMediaType == NULL)
    {
        SAFE_RELEASE(m_pMediaType);
        ReleaseResources();
        return S_OK;
    }

    HRESULT hr = S_OK;
    MFRatio fps = { 0, 0 };
	UINT uWidth, uHeight;
	UINT uNumerator, uDenominator;
	CComPtr<IDirect3DDevice9> pDevice;

    IMFSample *pSample = NULL;

    // Cannot set the media type after shutdown.
    CHECK_HR_GOTO(hr = CheckShutdown());

    // Check if the new type is actually different.
    // Note: This function safely handles NULL input parameters.
    if (AreMediaTypesEqual(m_pMediaType, pMediaType))  
    {
        return S_OK; // Nothing more to do.
    }

    // We're really changing the type. First get rid of the old type.
    SAFE_RELEASE(m_pMediaType);
    ReleaseResources();

    // Initialize the presenter engine with the new media type.
    // The presenter engine allocates the samples. 

//    CHECK_HR_GOTO(hr = m_pD3DPresentEngine->CreateVideoSamples(pMediaType, sampleQueue));
	hr = MFGetAttributeSize(pMediaType, MF_MT_FRAME_SIZE, &uWidth, &uHeight);
	if (FAILED(hr))
		return hr;

	hr = m_pDeviceManager->LockDevice(m_hDeviceHandle, &pDevice, TRUE);
	if (SUCCEEDED(hr))
	{
		m_SamplePool.Clear();
//		hr = m_SamplePool.AllocateSamples(pDevice, uWidth, uHeight, sampleQueue);
        GUID guidSubtype = GUID_NULL;
        DWORD dwFormat = 0;
        hr = pMediaType->GetGUID(MF_MT_SUBTYPE, &guidSubtype);
        if (FAILED(hr))
            return hr;

        dwFormat = guidSubtype.Data1;
		if (!m_pCustomMixer)
		{
			hr = m_SamplePool.Initialize(pDevice, uWidth, uHeight, m_TokenCounter, dwFormat);
			if (FAILED(hr))
				m_SamplePool.Clear();
		}
		m_pDeviceManager->UnlockDevice(m_hDeviceHandle, TRUE);
	}

    // Mark each sample with our token counter. If this batch of samples becomes
    // invalid, we increment the counter, so that we know they should be discarded. 
    //for (EvrVideoSampleList::POSITION pos = sampleQueue.FrontPosition();
    //     pos != sampleQueue.EndPosition();
    //     pos = sampleQueue.Next(pos))
    //{
    //    CHECK_HR_GOTO(hr = sampleQueue.GetItemPos(pos, &pSample));
    //    CHECK_HR_GOTO(hr = pSample->SetUINT32(MFSamplePresenter_SampleCounter, m_TokenCounter));

    //    SAFE_RELEASE(pSample);
    //}


    // Set the frame rate on the scheduler. 
    if (SUCCEEDED(GetFrameRate(pMediaType, &fps)) && (fps.Numerator != 0) && (fps.Denominator != 0))
    {
        m_scheduler.SetFrameRate(fps);
    }
    else
    {
        // NOTE: The mixer's proposed type might not have a frame rate, in which case 
        // we'll use an arbitary default. (Although it's unlikely the video source
        // does not have a frame rate.)
        m_scheduler.SetFrameRate(g_DefaultFrameRate);
    }

	m_lImageWidth = uWidth;
	m_lImageHeight = uHeight;

	hr = MFGetAttributeRatio(pMediaType, MF_MT_PIXEL_ASPECT_RATIO, &uNumerator, &uDenominator);
	if (SUCCEEDED(hr) && uDenominator)
		m_fNativeAspectRatio = FLOAT(m_lImageWidth * uNumerator) / FLOAT(m_lImageHeight * uDenominator);
	else
		m_fNativeAspectRatio = 0.f;

	m_fAspectRatio = m_fNativeAspectRatio;

    // Store the media type.
    ASSERT(pMediaType != NULL);
    m_pMediaType = pMediaType;
    m_pMediaType->AddRef();
	m_DisplayHelper.UpdateMaxDisplayFrameRate(m_lImageWidth, m_lImageHeight);

done:
    if (FAILED(hr))
    {
        ReleaseResources();
    }
    return hr;
}

//-----------------------------------------------------------------------------
// IsMediaTypeSupported
//
// Queries whether the presenter can use a proposed format from the mixer.
//-----------------------------------------------------------------------------

HRESULT CVideoSourceMSEvr::IsMediaTypeSupported(IMFMediaType *pMediaType)
{
	return S_OK;
	/*
    VideoTypeBuilder        *pProposed = NULL;

    HRESULT                 hr = S_OK;
    D3DFORMAT               d3dFormat = D3DFMT_UNKNOWN;
    BOOL                    bCompressed = FALSE;
    MFVideoInterlaceMode    InterlaceMode = MFVideoInterlace_Unknown;
    MFVideoArea             VideoCropArea;
    UINT32                  width = 0, height = 0;

    // Helper object for reading the proposed type.
    CHECK_HR_GOTO(hr = MediaTypeBuilder::Create(pMediaType, &pProposed));

    // Reject compressed media types.
    CHECK_HR_GOTO(hr = pProposed->IsCompressedFormat(&bCompressed));
    if (bCompressed)
    {
        CHECK_HR_GOTO(hr = MF_E_INVALIDMEDIATYPE);
    }

    // Validate the format.
    CHECK_HR_GOTO(hr = pProposed->GetFourCC((DWORD*)&d3dFormat));

    // The D3DPresentEngine checks whether the format can be used as
    // the back-buffer format for the swap chains.
    CHECK_HR_GOTO(hr = m_pD3DPresentEngine->CheckFormat(d3dFormat));

    // Reject interlaced formats.
    CHECK_HR_GOTO(hr = pProposed->GetInterlaceMode(&InterlaceMode));
    if (InterlaceMode != MFVideoInterlace_Progressive)
    {
        CHECK_HR_GOTO(hr = MF_E_INVALIDMEDIATYPE);
    }

    CHECK_HR_GOTO(hr = pProposed->GetFrameDimensions(&width, &height));

    // Validate the various apertures (cropping regions) against the frame size.
    // Any of these apertures may be unspecified in the media type, in which case 
    // we ignore it. We just want to reject invalid apertures.
    if (SUCCEEDED(pProposed->GetPanScanAperture(&VideoCropArea)))
    {
        ValidateVideoArea(VideoCropArea, width, height);
    }
    if (SUCCEEDED(pProposed->GetGeometricAperture(&VideoCropArea)))
    {
        ValidateVideoArea(VideoCropArea, width, height);
    }
    if (SUCCEEDED(pProposed->GetMinDisplayAperture(&VideoCropArea)))
    {
        ValidateVideoArea(VideoCropArea, width, height);
    }

done:
    SAFE_RELEASE(pProposed);
    return hr;
	*/
}


//-----------------------------------------------------------------------------
// ProcessOutputLoop
//
// Get video frames from the mixer and schedule them for presentation.
//-----------------------------------------------------------------------------

void CVideoSourceMSEvr::ProcessOutputLoop()
{
    HRESULT hr = S_OK;

    // Process as many samples as possible.
    while (hr == S_OK)
    {
        // If the mixer doesn't have a new input sample, break from the loop.
        if (!m_bSampleNotify)
        {
            hr = MF_E_TRANSFORM_NEED_MORE_INPUT;
            break;
        }

        // Try to process a sample.
        hr = ProcessOutput();

		if (m_bDShowWorkaround)
		{
			DWORD dwSleep = 0;
		// we don't want to pull too fast.
			if (MF_E_SAMPLEALLOCATOR_EMPTY == hr)
			{
				dwSleep = 2;
				hr = S_OK;
			}
			Sleep(dwSleep);
		}

        // NOTE: ProcessOutput can return S_FALSE to indicate it did not process a sample.
        // If so, we break out of the loop.
    }

    if (hr == MF_E_TRANSFORM_NEED_MORE_INPUT)
    {
        // The mixer has run out of input data. Check if we're at the end of the stream.
        CheckEndOfStream();
    }
}

//-----------------------------------------------------------------------------
// ProcessOutput
//
// Attempts to get a new output sample from the mixer.
//
// Called in two situations: 
// (1) ProcessOutputLoop, if the mixer has a new input sample. (m_bSampleNotify)
// (2) Repainting the last frame. (m_bRepaint)
//-----------------------------------------------------------------------------

HRESULT CVideoSourceMSEvr::ProcessOutput()
{
    CAutoLock lock(&m_ObjectLock);
    //ASSERT(m_bSampleNotify || m_bRepaint);  // See note above.

    HRESULT     hr = S_OK;
    DWORD       dwStatus = 0;
    LONGLONG    mixerStartTime = 0, mixerEndTime = 0;
    MFTIME      systemTime = 0;
    BOOL        bRepaint = m_bRepaint; // Temporarily store this state flag.  

    MFT_OUTPUT_DATA_BUFFER dataBuffer;
    ZeroMemory(&dataBuffer, sizeof(dataBuffer));

    IMFSample *pSample = NULL;

    // If the clock is not running, we present the first sample,
    // and then don't present any more until the clock starts. 

    if ((m_RenderState != RENDER_STATE_STARTED) &&  // Not running.
         !m_bRepaint &&                             // Not a repaint request.
         m_bPrerolled                               // At least one sample has been presented.
         )
    {
        return S_FALSE;
    }

    // Make sure we have a pointer to the mixer.
    if (m_pMixer == NULL)
    {
        return MF_E_INVALIDREQUEST;
    }

	if (!m_pCustomMixer)
	{
		// Try to get a free sample from the video sample pool.
		hr = m_SamplePool.GetSample(&pSample);
		if (hr == MF_E_SAMPLEALLOCATOR_EMPTY)
		{
			// No free samples. We'll try again when a sample is released.
			//
			// We return MF_E_SAMPLEALLOCATOR_EMPTY to allow ProcessOutputLoop to keep pulling samples from mixer
			// until mixer is drain. Somehow this can avoid certain runtime errors inside EVR which may be a result
			// of too many samples queued in the mixer, especially after flush.
			//
			// If in frame stepping, we want to keep our present queue full and then stop pulling from mixer, so we
			// return S_FALSE to break the loop.
			return (m_FrameStep.state == FRAMESTEP_NONE) ? MF_E_SAMPLEALLOCATOR_EMPTY : S_FALSE;
		}
		CHECK_HR_GOTO(hr);   // Fail on any other error code.

		// From now on, we have a valid video sample pointer, where the mixer will
		// write the video data.
		ASSERT(pSample != NULL);

		// (If the following assertion fires, it means we are not managing the sample pool correctly.)
		ASSERT(MFGetAttributeUINT32(pSample, MFSamplePresenter_SampleCounter, (UINT32)-1) == m_TokenCounter);

		if (m_bRepaint)
		{
			// Repaint request. Ask the mixer for the most recent sample.
			SetDesiredSampleTime(pSample, m_scheduler.LastSampleTime(), m_scheduler.FrameDuration());
			m_bRepaint = FALSE; // OK to clear this flag now.
		}
		else
		{
			// Not a repaint request. Clear the desired sample time; the mixer will
			// give us the next frame in the stream.
			ClearDesiredSampleTime(pSample);
		}
	}

	if (!m_bRepaint && m_pClock)
	{
		// Latency: Record the starting time for the ProcessOutput operation. 
		(void)m_pClock->GetCorrelatedTime(0, &mixerStartTime, &systemTime);
	}

    // Now we are ready to get an output sample from the mixer. 
    dataBuffer.dwStreamID = 0;
    dataBuffer.pSample = pSample;
    dataBuffer.dwStatus = 0;

    hr = m_pMixer->ProcessOutput(0, 1, &dataBuffer, &dwStatus);

    if (FAILED(hr))
    {
		if (!m_pCustomMixer)
		{
			// Return the sample to the pool.
			HRESULT hr2 = m_SamplePool.ReturnSample(pSample);
			if (FAILED(hr2))
			{
				CHECK_HR_GOTO(hr = hr2);
			}
		}

        // Handle some known error codes from ProcessOutput.
        if (hr == MF_E_TRANSFORM_TYPE_NOT_SET)
        {
            // The mixer's format is not set. Negotiate a new format.
            hr = RenegotiateMediaType();
        }
        else if (hr == MF_E_TRANSFORM_STREAM_CHANGE)
        {
            // There was a dynamic media type change. Clear our media type.
            SetMediaType(NULL);
        }
        else if (hr == MF_E_TRANSFORM_NEED_MORE_INPUT)
        {
            // The mixer needs more input. 
            // We have to wait for the mixer to get more input.
            m_bSampleNotify = FALSE; 
        }
    }
    else
    {
        // We got an output sample from the mixer.

        if (m_pClock && !bRepaint)
        {
            // Latency: Record the ending time for the ProcessOutput operation,
            // and notify the EVR of the latency. 

            (void)m_pClock->GetCorrelatedTime(0, &mixerEndTime, &systemTime);

            LONGLONG latencyTime = mixerEndTime - mixerStartTime;
            NotifyEvent(EC_PROCESSING_LATENCY, (LONG_PTR)&latencyTime, 0);
        }

		if (!m_bDShowWorkaround)
		{
		// NOTE: we don't use TrackSample call back because a critical runtime error in EVR.
		//
        // Set up notification for when the sample is released.
			CHECK_HR_GOTO(hr = TrackSample(dataBuffer.pSample));
		}

        // Schedule the sample.
        if ((m_FrameStep.state == FRAMESTEP_NONE) || bRepaint)
        {
            CHECK_HR_GOTO(hr = DeliverSample(dataBuffer.pSample, bRepaint));
        }
        else
        {
            // We are frame-stepping (and this is not a repaint request).
            CHECK_HR_GOTO(hr = DeliverFrameStepSample(dataBuffer.pSample));
        }
        m_bPrerolled = TRUE; // We have presented at least one sample now.
    }

done:
    // Release any events that were returned from the ProcessOutput method. 
    // (We don't expect any events from the mixer, but this is a good practice.)
    ReleaseEventCollection(1, &dataBuffer); 

    SAFE_RELEASE(pSample);

    return hr;
}


//-----------------------------------------------------------------------------
// DeliverSample
//
// Schedule a video sample for presentation.
//
// Called from:
// - ProcessOutput
// - DeliverFrameStepSample
//-----------------------------------------------------------------------------

HRESULT CVideoSourceMSEvr::DeliverSample(IMFSample *pSample, BOOL bRepaint)
{
    ASSERT(pSample != NULL);

    HRESULT hr = S_OK;
//    D3DPresentEngine::DeviceState state = D3DPresentEngine::DeviceOK;

    // If we are not actively playing, OR we are scrubbing (rate = 0) OR this is a 
    // repaint request, then we need to present the sample immediately. Otherwise, 
    // schedule it normally.

    BOOL bPresentNow = ((m_RenderState != RENDER_STATE_STARTED) ||  IsScrubbing() || bRepaint);

	if (SUCCEEDED(IsValid()))
	{
		hr = m_scheduler.ScheduleSample(pSample, bPresentNow);
	}
	else 
		hr = E_FAIL;
	
	if (FAILED(hr))
	{
		NotifyEvent(EC_ERRORABORT, hr, 0);
	}
    // Check the D3D device state.
    //hr = m_pD3DPresentEngine->CheckDeviceState(&state);

    //if (SUCCEEDED(hr))
    //{
    //    hr = m_scheduler.ScheduleSample(pSample, bPresentNow);
    //}

    //if (FAILED(hr))
    //{
    //    // Notify the EVR that we have failed during streaming. The EVR will notify the 
    //    // pipeline (ie, it will notify the Filter Graph Manager in DirectShow or the 
    //    // Media Session in Media Foundation).

    //    NotifyEvent(EC_ERRORABORT, hr, 0);
    //}
    //else if (state == D3DPresentEngine::DeviceReset)
    //{
    //    // The Direct3D device was re-set. Notify the EVR.
    //    NotifyEvent(EC_DISPLAY_CHANGED, S_OK, 0);
    //}

    return hr;
}

//-----------------------------------------------------------------------------
// DeliverFrameStepSample
//
// Process a video sample for frame-stepping.
//-----------------------------------------------------------------------------

HRESULT CVideoSourceMSEvr::DeliverFrameStepSample(IMFSample *pSample)
{
    HRESULT hr = S_OK;
    IUnknown *pUnk = NULL;

    // For rate 0, discard any sample that ends earlier than the clock time.
    if (IsScrubbing() && m_pClock && IsSampleTimePassed(m_pClock, pSample))
    {
        // Discard this sample.
    }
    else if (m_FrameStep.state >= FRAMESTEP_SCHEDULED)
    {
        // A frame was already submitted. Put this sample on the frame-step queue, 
        // in case we are asked to step to the next frame. If frame-stepping is
        // cancelled, this sample will be processed normally.
        CHECK_HR_GOTO(hr = m_FrameStep.samples.InsertBack(pSample));
    }
    else
    {
        // We're ready to frame-step.

        // Decrement the number of steps.
        if (m_FrameStep.steps > 0)
        {
            m_FrameStep.steps--;
        }

        if (m_FrameStep.steps > 0)
        {
            // This is not the last step. Discard this sample.
        }
        else if (m_FrameStep.state == FRAMESTEP_WAITING_START)
        {
            // This is the right frame, but the clock hasn't started yet. Put the
            // sample on the frame-step queue. When the clock starts, the sample
            // will be processed.
            CHECK_HR_GOTO(hr = m_FrameStep.samples.InsertBack(pSample));
        }
        else
        {
            // This is the right frame *and* the clock has started. Deliver this sample.
            CHECK_HR_GOTO(hr = DeliverSample(pSample, FALSE));

            // QI for IUnknown so that we can identify the sample later.
            // (Per COM rules, an object alwayss return the same pointer when QI'ed for IUnknown.)
            CHECK_HR_GOTO(hr = pSample->QueryInterface(__uuidof(IUnknown), (void**)&pUnk));

            // Save this value.
            m_FrameStep.pSampleNoRef = (DWORD_PTR)pUnk; // No add-ref. 

            // NOTE: We do not AddRef the IUnknown pointer, because that would prevent the 
            // sample from invoking the OnSampleFree callback after the sample is presented. 
            // We use this IUnknown pointer purely to identify the sample later; we never
            // attempt to dereference the pointer.

            // Update our state.
            m_FrameStep.state = FRAMESTEP_SCHEDULED;
        }
    }
done:
    SAFE_RELEASE(pUnk);
    return hr;
}


//-----------------------------------------------------------------------------
// TrackSample
//
// Given a video sample, sets a callback that is invoked when the sample is no 
// longer in use. 
//
// Note: The callback method returns the sample to the pool of free samples; for
// more information, see CVideoSourceMSEvr::OnSampleFree(). 
//
// This method uses the IMFTrackedSample interface on the video sample.
//-----------------------------------------------------------------------------

HRESULT CVideoSourceMSEvr::TrackSample(IMFSample *pSample)
{
    HRESULT hr = S_OK;
    IMFTrackedSample *pTracked = NULL;

    CHECK_HR_GOTO(hr = pSample->QueryInterface(__uuidof(IMFTrackedSample), (void**)&pTracked));
    CHECK_HR_GOTO(hr = pTracked->SetAllocator(&m_SampleFreeCB, NULL)); 

done:
    SAFE_RELEASE(pTracked);
    return hr;
}


//-----------------------------------------------------------------------------
// ReleaseResources
//
// Releases resources that the presenter uses to render video. 
//
// Note: This method flushes the scheduler queue and releases the video samples.
// It does not release helper objects such as the D3DPresentEngine, or free
// the presenter's media type.
//-----------------------------------------------------------------------------

void CVideoSourceMSEvr::ReleaseResources()
{
    // Increment the token counter to indicate that all existing video samples
    // are "stale." As these samples get released, we'll dispose of them. 
    //
    // Note: The token counter is required because the samples are shared
    // between more than one thread, and they are returned to the presenter 
    // through an asynchronous callback (OnSampleFree). Without the token, we
    // might accidentally re-use a stale sample after the ReleaseResources
    // method returns.

    m_TokenCounter++;

    Flush();

    m_SamplePool.Clear();
// we keep last surface in Scheduler.
  //  m_pD3DPresentEngine->ReleaseResources();
}

BOOL CVideoSourceMSEvr::IsSampleBeingUsedInMixer(IMFSample *pSample)
{
	CComPtr<IMFMediaBuffer> pBuffer;
	CComPtr<IDirect3DSurface9> pSurface;

	HRESULT hr = pSample->GetBufferByIndex(0, &pBuffer);
	if (SUCCEEDED(hr))
	{
		CComQIPtr<IMFGetService> pGetSurface = pBuffer;
		if (pGetSurface)
			hr = pGetSurface->GetService(MR_BUFFER_SERVICE, __uuidof(IDirect3DSurface9), (void **)&pSurface);

		hr = m_pDispObj->ProcessMessage(m_hwndVideo, VIDEOSOURCEDO_REQUEST_QUERY_SURFACE_STATUS, 0, reinterpret_cast<LONG>(pSurface.p));
	}

	// only ProcessMessage() can return DDERR_WASSTILLDRAWING for mixer being using the surface.
	return hr == DDERR_WASSTILLDRAWING ? TRUE : FALSE;
}

//-----------------------------------------------------------------------------
// OnSampleFree
//
// Callback that is invoked when a sample is released. For more information,
// see CVideoSourceMSEvrTrackSample().
//-----------------------------------------------------------------------------
HRESULT CVideoSourceMSEvr::OnSampleFree(IMFSample *pSample)
{
	if (!m_bDShowWorkaround)
		return E_NOTIMPL;

	VSE_DP("OnSampleFree pSample = 0x%x", pSample);
	HRESULT hr = S_OK;

    if (MFGetAttributeUINT32(pSample, MFSamplePresenter_SampleCounter, (UINT32)-1) == m_TokenCounter)
    {
		ASSERT(IsSampleBeingUsedInMixer(pSample) != TRUE);

		// in custom mixer, the sample was by passed without default mixer, thus no MFSamplePresenter_SampleCounter is set
		// it is impossible to get here when using custom mixer.
		ASSERT(m_pCustomMixer == 0);
		hr = m_SamplePool.ReturnSample(pSample);
	}
	else if (m_pCustomMixer)
	{
		IMFSample *pSampleInMixer = 0;
		while (SUCCEEDED(m_SampleInMixerList.RemoveFront(&pSampleInMixer)))
		{
			if (IsSampleBeingUsedInMixer(pSampleInMixer))
			{
				m_SampleInMixerList.InsertFront(pSampleInMixer);
				break;
			}

			SAFE_RELEASE(pSampleInMixer);
		}

		if (IsSampleBeingUsedInMixer(pSample))
			m_SampleInMixerList.InsertBack(pSample);

		SAFE_RELEASE(pSample);
	}

	CheckEndOfStream();
    return hr;
}

HRESULT CVideoSourceMSEvr::OnSampleFree(IMFAsyncResult *pResult)
{
//	VSE_DP("OnSampleFree");
    HRESULT hr = S_OK;
    IUnknown *pObject = NULL;
    IMFSample *pSample = NULL;
//    IUnknown *pUnk = NULL;

	ASSERT(m_pCustomMixer == 0);	// custom mixer should not take this path.
    // Get the sample from the async result object.
    CHECK_HR_GOTO(hr = pResult->GetObject(&pObject));
    CHECK_HR_GOTO(hr = pObject->QueryInterface(__uuidof(IMFSample), (void**)&pSample));

/* Move these codes to OnUpdateLastSample.
    // If this sample was submitted for a frame-step, then the frame step is complete.
    if (m_FrameStep.state == FRAMESTEP_SCHEDULED) 
    {
        // QI the sample for IUnknown and compare it to our cached value.
        CHECK_HR_GOTO(hr = pSample->QueryInterface(__uuidof(IMFSample), (void**)&pUnk));

        if (m_FrameStep.pSampleNoRef == (DWORD_PTR)pUnk)
        {
            // Notify the EVR. 
            CHECK_HR_GOTO(hr = CompleteFrameStep(pSample));
        }

        // Note: Although pObject is also an IUnknown pointer, it's not guaranteed
        // to be the exact pointer value returned via QueryInterface, hence the 
        // need for the second QI.
    }
*/
//   Perhaps we don't need to lock here, Scheduler has a lock to avoid presenting at the same time.
    //m_ObjectLock.Lock();

    if (MFGetAttributeUINT32(pSample, MFSamplePresenter_SampleCounter, (UINT32)-1) == m_TokenCounter)
    {
        // Return the sample to the sample pool.
		VSE_DP("Return Sample = 0x%08x", pSample );

        CHECK_HR_GOTO(hr = m_SamplePool.ReturnSample(pSample));

		// Now that a free sample is available, process more data if possible.
		(void)ProcessOutputLoop();
    }
    //m_ObjectLock.Unlock();

done:
    if (FAILED(hr))
    {
        NotifyEvent(EC_ERRORABORT, hr, 0);
    }
    SAFE_RELEASE(pObject);
    SAFE_RELEASE(pSample);
//    SAFE_RELEASE(pUnk);
    return hr;
}


////-----------------------------------------------------------------------------
//// GetMaxRate
////
//// Returns the maximum forward playback rate. 
//// Note: The maximum reverse rate is -1 * MaxRate().
////-----------------------------------------------------------------------------
//
//float CVideoSourceMSEvr::GetMaxRate(BOOL bThin)
//{
//    // Non-thinned:
//    // If we have a valid frame rate and a monitor refresh rate, the maximum 
//    // playback rate is equal to the refresh rate. Otherwise, the maximum rate 
//    // is unbounded (FLT_MAX).
//
//    // Thinned: The maximum rate is unbounded.
//
//    float   fMaxRate = FLT_MAX;
//    MFRatio fps = { 0, 0 };
//    UINT    MonitorRateHz = 0; 
//
//    if (!bThin && (m_pMediaType != NULL))
//    {
//        GetFrameRate(m_pMediaType, &fps);
//        MonitorRateHz = m_pD3DPresentEngine->RefreshRate();
//
//        if (fps.Denominator && fps.Numerator && MonitorRateHz)
//        {
//            // Max Rate = Refresh Rate / Frame Rate
//            fMaxRate = (float)MulDiv(MonitorRateHz, fps.Denominator, fps.Numerator);
//        }
//    }
//
//    return fMaxRate;
//}


//-----------------------------------------------------------------------------
// Static functions
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// CorrectAspectRatio
//
// Converts a rectangle from one pixel aspect ratio (PAR) to another PAR.
// Returns the corrected rectangle.
//
// For example, a 720 x 486 rect with a PAR of 9:10, when converted to 1x1 PAR, must 
// be stretched to 720 x 540. 
//-----------------------------------------------------------------------------

RECT CVideoSourceMSEvr::CorrectAspectRatio(const RECT& src, const MFRatio& srcPAR, const MFRatio& destPAR)
{
    // Start with a rectangle the same size as src, but offset to the origin (0,0).
    RECT rc = {0, 0, src.right - src.left, src.bottom - src.top};

    // If the source and destination have the same PAR, there is nothing to do.
    // Otherwise, adjust the image size, in two steps:
    //  1. Transform from source PAR to 1:1
    //  2. Transform from 1:1 to destination PAR.

    if ((srcPAR.Numerator != destPAR.Numerator) || (srcPAR.Denominator != destPAR.Denominator))
    {
        // Correct for the source's PAR.

        if (srcPAR.Numerator > srcPAR.Denominator)
        {
            // The source has "wide" pixels, so stretch the width.
            rc.right = MulDiv(rc.right, srcPAR.Numerator, srcPAR.Denominator);
        }
        else if (srcPAR.Numerator > srcPAR.Denominator)
        {
            // The source has "tall" pixels, so stretch the height.
            rc.bottom = MulDiv(rc.bottom, srcPAR.Denominator, srcPAR.Numerator);
        }
        // else: PAR is 1:1, which is a no-op.


        // Next, correct for the target's PAR. This is the inverse operation of the previous.

        if (destPAR.Numerator > destPAR.Denominator)
        {
            // The destination has "tall" pixels, so stretch the width.
            rc.bottom = MulDiv(rc.bottom, destPAR.Denominator, destPAR.Numerator);
        }
        else if (destPAR.Numerator > destPAR.Denominator)
        {
            // The destination has "fat" pixels, so stretch the height.
            rc.right = MulDiv(rc.right, destPAR.Numerator, destPAR.Denominator);
        }
        // else: PAR is 1:1, which is a no-op.
    }

    return rc;
}


//-----------------------------------------------------------------------------
// AreMediaTypesEqual
//
// Tests whether two IMFMediaType's are equal. Either pointer can be NULL.
// (If both pointers are NULL, returns TRUE)
//-----------------------------------------------------------------------------

BOOL CVideoSourceMSEvr::AreMediaTypesEqual(IMFMediaType *pType1, IMFMediaType *pType2)
{
    if ((pType1 == NULL) && (pType2 == NULL))
    {
        return TRUE; // Both are NULL.
    }
    else if ((pType1 == NULL) || (pType2 == NULL))
    {
        return FALSE; // One is NULL.
    }

    DWORD dwFlags = 0;
    HRESULT hr = pType1->IsEqual(pType2, &dwFlags);

    return (hr == S_OK);
}


//-----------------------------------------------------------------------------
// ValidateVideoArea:
//
// Returns S_OK if an area is smaller than width x height. 
// Otherwise, returns MF_E_INVALIDMEDIATYPE.
//-----------------------------------------------------------------------------

HRESULT CVideoSourceMSEvr::ValidateVideoArea(const MFVideoArea& area, UINT32 width, UINT32 height)
{

    float fOffsetX = MFOffsetToFloat(area.OffsetX);
    float fOffsetY = MFOffsetToFloat(area.OffsetY);

    if ( ((LONG)fOffsetX + area.Area.cx > (LONG)width) ||
         ((LONG)fOffsetY + area.Area.cy > (LONG)height) )
    {
        return MF_E_INVALIDMEDIATYPE;
    }
    else
    {
        return S_OK;
    }
}


//-----------------------------------------------------------------------------
// SetDesiredSampleTime
//
// Sets the "desired" sample time on a sample. This tells the mixer to output 
// an earlier frame, not the next frame. (Used when repainting a frame.)
//
// This method uses the sample's IMFDesiredSample interface.
//
// hnsSampleTime: Time stamp of the frame that the mixer should output.
// hnsDuration: Duration of the frame.
//
// Note: Before re-using the sample, call ClearDesiredSampleTime to clear
// the desired time.
//-----------------------------------------------------------------------------

HRESULT CVideoSourceMSEvr::SetDesiredSampleTime(IMFSample *pSample, const LONGLONG& hnsSampleTime, const LONGLONG& hnsDuration)
{
    if (pSample == NULL)
    {
        return E_POINTER;
    }

    HRESULT hr = S_OK;
    IMFDesiredSample *pDesired = NULL;

    hr = pSample->QueryInterface(__uuidof(IMFDesiredSample), (void**)&pDesired);
    if (SUCCEEDED(hr))
    {
        // This method has no return value.
        (void)pDesired->SetDesiredSampleTimeAndDuration(hnsSampleTime, hnsDuration);
    }

    SAFE_RELEASE(pDesired);
    return hr;
}


//-----------------------------------------------------------------------------
// ClearDesiredSampleTime
//
// Clears the desired sample time. See SetDesiredSampleTime.
//-----------------------------------------------------------------------------

HRESULT CVideoSourceMSEvr::ClearDesiredSampleTime(IMFSample *pSample)
{
    if (pSample == NULL)
    {
        return E_POINTER;
    }

    HRESULT hr = S_OK;
    
    IMFDesiredSample *pDesired = NULL;
    
    // We store some custom attributes on the sample, so we need to cache them
    // and reset them.
    //
    // This works around the fact that IMFDesiredSample::Clear() removes all of the
    // attributes from the sample. 

    UINT32 counter = MFGetAttributeUINT32(pSample, MFSamplePresenter_SampleCounter, (UINT32)-1);

    hr = pSample->QueryInterface(__uuidof(IMFDesiredSample), (void**)&pDesired);
    if (SUCCEEDED(hr))
    {
        // This method has no return value.
        (void)pDesired->Clear();

        hr = pSample->SetUINT32(MFSamplePresenter_SampleCounter, counter);
    }

    SAFE_RELEASE(pDesired);
    return hr;
}


//-----------------------------------------------------------------------------
// IsSampleTimePassed
//
// Returns TRUE if the entire duration of pSample is in the past.
//
// Returns FALSE if all or part of the sample duration is in the future, or if
// the function cannot determined (e.g., if the sample lacks a time stamp).
//-----------------------------------------------------------------------------

BOOL CVideoSourceMSEvr::IsSampleTimePassed(IMFClock *pClock, IMFSample *pSample)
{
    ASSERT(pClock != NULL);
    ASSERT(pSample != NULL);

    if (pSample == NULL || pClock == NULL)
    {
        return E_POINTER;
    }


    HRESULT hr = S_OK;
    MFTIME hnsTimeNow = 0;
    MFTIME hnsSystemTime = 0;
    MFTIME hnsSampleStart = 0;
    MFTIME hnsSampleDuration = 0;

    // The sample might lack a time-stamp or a duration, and the
    // clock might not report a time.

    hr = pClock->GetCorrelatedTime(0, &hnsTimeNow, &hnsSystemTime);

    if (SUCCEEDED(hr))
    {
        hr = pSample->GetSampleTime(&hnsSampleStart);
    }
    if (SUCCEEDED(hr))
    {
        hr = pSample->GetSampleDuration(&hnsSampleDuration);
    }

    if (SUCCEEDED(hr))
    {
        if (hnsSampleStart + hnsSampleDuration < hnsTimeNow)
        {
            return TRUE; 
        }
    }

    return FALSE;
}


//-----------------------------------------------------------------------------
// SetMixerSourceRect
//
// Sets the ZOOM rectangle on the mixer.
//-----------------------------------------------------------------------------

HRESULT CVideoSourceMSEvr::SetMixerSourceRect(IMFTransform *pMixer, const MFVideoNormalizedRect& nrcSource)
{
    if (pMixer == NULL)
    {
        return E_POINTER;
    }

    HRESULT hr = S_OK;
    IMFAttributes *pAttributes = NULL;

    CHECK_HR_GOTO(hr = pMixer->GetAttributes(&pAttributes));

    CHECK_HR_GOTO(hr = pAttributes->SetBlob(VIDEO_ZOOM_RECT, (const UINT8*)&nrcSource, sizeof(nrcSource)));
        
done:
    SAFE_RELEASE(pAttributes);
    return hr;
}

void CVideoSourceMSEvr::ReleaseEventCollection(DWORD cOutputBuffers, MFT_OUTPUT_DATA_BUFFER* pBuffers)
{
	for (DWORD i = 0; i < cOutputBuffers; i++)
	{
		SAFE_RELEASE(pBuffers->pEvents);
	}
}

float CVideoSourceMSEvr::MFOffsetToFloat(const MFOffset& offset)
{
	return (float)offset.value + (float)offset.value / 65536.0f;
}

HRESULT CVideoSourceMSEvr::GetFrameRate(IMFMediaType *pType, MFRatio *pRatio)
{
	return MFGetAttributeRatio(pType, MF_MT_FRAME_RATE, (UINT32*)&pRatio->Numerator, (UINT32*)&pRatio->Denominator);
}

STDMETHODIMP CVideoSourceMSEvr::GetSampleProperty(SampleProperty *pProp)
{
    if (!pProp)
        return E_POINTER;

	CComPtr<IMFSample> pSample;
	//Lock sample in scheduler class
	HRESULT hr = m_scheduler.PeekPresentSample(&pSample);

    if (SUCCEEDED(hr) && pSample)
    {
        CComPtr<IMFMediaBuffer> pBuffer;

        hr = pSample->GetBufferByIndex(0, &pBuffer);
        if (SUCCEEDED(hr))
        {
            CComQIPtr<IMFGetService> pGetSurface = pBuffer;
            if (pGetSurface)
                hr = pGetSurface->GetService(MR_BUFFER_SERVICE, __uuidof(IDirect3DSurface9), (void **)&pProp->pSurface);
        }

        if (m_dwAspectRatioMode == MFVideoARMode_PreservePicture)
            m_fAspectRatio = m_fNativeAspectRatio;
        else if (m_dwAspectRatioMode ==MFVideoARMode_None)
            m_fAspectRatio = 0.0;

        pProp->fAspectRatio = m_fAspectRatio;
        pProp->uWidth = m_lImageWidth;
        pProp->uHeight = m_lImageHeight;
        pSample->GetSampleTime(&pProp->rtStart);
        pSample->GetSampleDuration(&pProp->rtEnd);
        pProp->rtEnd += pProp->rtStart;

		if (MFGetAttributeUINT32(pSample, MFSamplePresenter_FieldSelect, 0) == 1)
			pProp->dwFlags |= SAMPLE_FLAG_SELECTSECONDFIELD;
		
        if (MFGetAttributeUINT32(pSample, MFSampleExtension_Interlaced, FALSE) != FALSE)
            pProp->dwFlags |= SAMPLE_FLAG_INTERLACED;

        if (MFGetAttributeUINT32(pSample, MFSampleExtension_Discontinuity, FALSE) != FALSE)
            pProp->dwFlags |= SAMPLE_FLAG_DISCONTINUITY;

        if (MFGetAttributeUINT32(pSample, MFSampleExtension_BottomFieldFirst, FALSE) != FALSE)
            pProp->dwFlags |= SAMPLE_FLAG_BOTTOMFIELDFIRST;

        if (MFGetAttributeUINT32(pSample, MFSampleExtension_RepeatFirstField, FALSE) != FALSE)
            pProp->dwFlags |= SAMPLE_FLAG_REPEATFIRSTFIELD;
    }
	else
	{
		hr = E_FAIL;
	}

    return hr;
}

void CVideoSourceMSEvr::DumpOutputAvailableType()
{
#ifdef _DEBUG
    if (m_pMixer)
    {
        CComPtr<IMFMediaType> pType;
        HRESULT hr = S_OK;
        GUID guidSubtype = GUID_NULL;
        UINT i = 0;
        while (hr != MF_E_NO_MORE_TYPES)
        {
            hr = m_pMixer->GetOutputAvailableType(0, i, &pType);
            if (SUCCEEDED(hr))
            {
                if (SUCCEEDED(pType->GetGUID(MF_MT_SUBTYPE, &guidSubtype)))
                {
                    if (guidSubtype.Data1 < 0x100) //D3D Type
                    {
                        BOOL bFoundType = FALSE;
                        CHAR str[20] = {0};
                        switch (guidSubtype.Data1)
                        {
                        case D3DFMT_X8R8G8B8:
                            strcpy_s(str, 20, "D3DFMT_X8R8G8B8");
                            bFoundType = TRUE;
                            break;
                        case D3DFMT_A8R8G8B8:
                            strcpy_s(str, 20, "D3DFMT_A8R8G8B8");
                            bFoundType = TRUE;
                            break;
                        case D3DFMT_R8G8B8:
                            strcpy_s(str, 20, "D3DFMT_R8G8B8");
                            bFoundType = TRUE;
                            break;
                        case D3DFMT_X1R5G5B5:
                            strcpy_s(str, 20, "D3DFMT_X1R5G5B5");
                            bFoundType = TRUE;
                            break;
                        case D3DFMT_R5G6B5:
                            strcpy_s(str, 20, "D3DFMT_R5G6B5");
                            bFoundType = TRUE;
                            break;
                        default:
                            break;
                        }
                        if (!bFoundType)
                        {
                            DbgMsg("EVR Mixer OutputType : Index=%d, Type = %d", i, guidSubtype.Data1);
                        }
                        else
                        {
                            DbgMsg("EVR Mixer OutputType : Index=%d, Type = %s", i, str);
                        }
                    }	
                    else
                    {
                        DbgMsg("EVR OutputType : Index=%d, Type = %c%c%c%c", i,
                            (guidSubtype.Data1 >>  0)  & 0xff,
                            (guidSubtype.Data1 >>  8)  & 0xff,
                            (guidSubtype.Data1 >> 16) & 0xff,
                            (guidSubtype.Data1 >> 24) & 0xff);
                    }
                }
                pType.Release();
            }

            i++;
        }
    }
#endif
}

//#pragma warning( pop )