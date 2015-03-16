#include "stdafx.h"
#include "EvrHelper.h"
#include "EvrPresentScheduler.h"

#ifndef EPS_DP
//#define EPS_DP(fmt, ...)			DbgMsg("CEvrPresentScheduler::" fmt, __VA_ARGS__)
#define EPS_DP __noop
#endif


// ScheduleEvent
// Messages for the scheduler thread.
enum ScheduleEvent
{
    eTerminate =    WM_USER,
    eSchedule =     WM_USER + 1,
    eFlush =        WM_USER + 2
};

const DWORD SCHEDULER_TIMEOUT = 5000;

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------

CEvrPresentScheduler::CEvrPresentScheduler() :
	m_pEvrCallback(NULL),
//    m_pCB(NULL),
//    m_pClock(NULL), 
    m_dwThreadID(0),
    m_hSchedulerThread(NULL),
    m_hThreadReadyEvent(NULL),
    m_hFlushEvent(NULL),
    m_fRate(1.0f),
    m_LastSampleTime(0), 
	m_pPresentSample(0),
    m_bEnableFRC(FALSE)
{
	ZeroMemory(&m_QualProp, sizeof(m_QualProp));
}


//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------

CEvrPresentScheduler::~CEvrPresentScheduler()
{
	m_pClock = NULL;
//    SAFE_RELEASE(m_pClock);
}


//-----------------------------------------------------------------------------
// SetFrameRate
// Specifies the frame rate of the video, in frames per second.
//-----------------------------------------------------------------------------

void CEvrPresentScheduler::SetFrameRate(const MFRatio& fps)
{
    m_frContent.SetRate(fps);
}

void CEvrPresentScheduler::EnableFRC(const MFRatio *pSuggestedFPS)
{
    if (pSuggestedFPS)
    {
        m_bEnableFRC = TRUE;
        m_frDisplay.SetRate(*pSuggestedFPS);
    }
    else
    {
        m_bEnableFRC = FALSE;
    }
}

//-----------------------------------------------------------------------------
// StartScheduler
// Starts the scheduler's worker thread.
//
// IMFClock: Pointer to the EVR's presentation clock. Can be NULL.
//-----------------------------------------------------------------------------

HRESULT CEvrPresentScheduler::StartScheduler(IMFClock *pClock)
{
    if (m_hSchedulerThread != NULL)
    {
        return E_UNEXPECTED;
    }

    HRESULT hr = S_OK;
    DWORD dwID = 0;

	m_pClock = pClock;
//    CopyComPointer(m_pClock, pClock);

	//Clean Quality Property
	ZeroMemory(&m_QualProp, sizeof(m_QualProp));

    // Set a high the timer resolution (ie, short timer period).
    timeBeginPeriod(1);

    // Create an event to wait for the thread to start.
	try 
	{
		m_hThreadReadyEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		if (m_hThreadReadyEvent == NULL)
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
			throw;
		}

		// Create an event to wait for flush commands to complete.
		m_hFlushEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		if (m_hFlushEvent == NULL)
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
			throw;
		}

		// Create the scheduler thread.
		m_hSchedulerThread = CreateThread(NULL, 0, SchedulerThreadProc, (LPVOID)this, 0, &dwID);
		if (m_hSchedulerThread == NULL)
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
			throw;
		}

		HANDLE hObjects[] = { m_hThreadReadyEvent, m_hSchedulerThread };
		DWORD dwWait = 0;

		// Wait for the thread to signal the "thread ready" event.
		dwWait = WaitForMultipleObjects(2, hObjects, FALSE, INFINITE);  // Wait for EITHER of these handles.
		if (WAIT_OBJECT_0 != dwWait)
		{
			// The thread terminated early for some reason. This is an error condition.
			CloseHandle(m_hSchedulerThread);
			m_hSchedulerThread = NULL;
			hr = E_UNEXPECTED;
			throw;
		}
		m_dwThreadID = dwID;
	}
	catch(...)
	{
		EPS_DP("StartScheduler Failed hr= 0x%08x", hr);
	}
    // Regardless success/failure, we are done using the "thread ready" event.
    if (m_hThreadReadyEvent)
    {
        CloseHandle(m_hThreadReadyEvent);
        m_hThreadReadyEvent = NULL;
    }

    return hr;
}


//-----------------------------------------------------------------------------
// StopScheduler
//
// Stops the scheduler's worker thread.
//-----------------------------------------------------------------------------

HRESULT CEvrPresentScheduler::StopScheduler()
{
    if (m_hSchedulerThread == NULL)
    {
        return S_OK;
    }

    // disable FRC since we are going to stop playback.
    m_bEnableFRC = FALSE;

    // Ask the scheduler thread to exit.
    PostThreadMessage(m_dwThreadID, eTerminate, 0, 0);
    
    // Wait for the thread to exit.
    WaitForSingleObject(m_hSchedulerThread, INFINITE);

    // Close handles.
    CloseHandle(m_hSchedulerThread);
    m_hSchedulerThread = NULL;

    CloseHandle(m_hFlushEvent);
    m_hFlushEvent = NULL;

	UpdatePresentSample(NULL);

    // Discard samples.
    m_ScheduledSamples.Clear();

	//Clean Quality Property
	ZeroMemory(&m_QualProp, sizeof(m_QualProp));

    // Restore the timer resolution.
    timeEndPeriod(1);

    return S_OK;
}


//-----------------------------------------------------------------------------
// Flush
//
// Flushes all samples that are queued for presentation.
//
// Note: This method is synchronous; ie., it waits for the flush operation to 
// complete on the worker thread.
//-----------------------------------------------------------------------------

HRESULT CEvrPresentScheduler::Flush(BOOL bIsClockStopped)
{
    DbgMsg("Scheduler::Flush");

    if (m_hSchedulerThread == NULL)
    {
        DbgMsg("No scheduler thread!");
    }

    if (m_hSchedulerThread)
    {
        // Ask the scheduler thread to flush.
        PostThreadMessage(m_dwThreadID, eFlush, bIsClockStopped, 0);

        // Wait for the scheduler thread to signal the flush event,
        // OR for the thread to terminate.
        HANDLE objects[] = { m_hFlushEvent, m_hSchedulerThread };

        WaitForMultipleObjects((sizeof(objects) / sizeof(objects[0])), objects, FALSE, SCHEDULER_TIMEOUT); 

        DbgMsg("Scheduler::Flush completed.");
    }

    return S_OK;
}


//-----------------------------------------------------------------------------
// ScheduleSample
//
// Schedules a new sample for presentation.
//
// pSample:     Pointer to the sample.
// bPresentNow: If TRUE, the sample is presented immediately. Otherwise, the
//              sample's time stamp is used to schedule the sample.
//-----------------------------------------------------------------------------

HRESULT CEvrPresentScheduler::ScheduleSample(IMFSample *pSample, BOOL bPresentNow)
{
    if (m_hSchedulerThread == NULL)
    {
        return MF_E_NOT_INITIALIZED;
    }

    HRESULT hr = S_OK;
    DWORD dwExitCode = 0;

    GetExitCodeThread(m_hSchedulerThread, &dwExitCode);
    if (dwExitCode != STILL_ACTIVE)
    {
        return E_FAIL;
    }

    if (bPresentNow || (m_pClock == NULL))
    {
        // Present the sample immediately.
		Present(pSample, 0);
        //m_pCB->PresentSample(pSample, 0);
    }
    else
    {
        // Queue the sample and ask the scheduler thread to wake up.
        hr = m_ScheduledSamples.Queue(pSample);

        if (SUCCEEDED(hr))
        {
            PostThreadMessage(m_dwThreadID, eSchedule, 0, 0);
        }
    }

	if (FAILED(hr))
	    DbgMsg("Scheduler::ScheduleSample failed");

    return hr;
}

//-----------------------------------------------------------------------------
// ProcessSamplesInQueue
//
// Processes all the samples in the queue.
//
// plNextSleep: Receives the length of time the scheduler thread should sleep
//              before it calls ProcessSamplesInQueue again.
//-----------------------------------------------------------------------------

HRESULT CEvrPresentScheduler::ProcessSamplesInQueue(LONG *plNextSleep)
{
    HRESULT hr = S_OK;
    LONG lWait = 0;
    IMFSample *pSample = NULL;

    // Process samples until the queue is empty or until the wait time > 0.

    // Note: Dequeue returns S_FALSE when the queue is empty.
    while (m_ScheduledSamples.Dequeue(&pSample) == S_OK) 
    {
        // Process the next sample in the queue. If the sample is not ready
        // for presentation. the value returned in lWait is > 0, which
        // means the scheduler should sleep for that amount of time.

        if (m_bEnableFRC)
            hr = ProcessSampleFRC(pSample, &lWait, m_frDisplay);
        else
            hr = ProcessSample(pSample, &lWait, m_frContent);
        SAFE_RELEASE(pSample);

        if (FAILED(hr))
        {
            break;
        }
        if (lWait > 0)
        {
            break;
        }
    }

    // If the wait time is zero, it means we stopped because the queue is
    // empty (or an error occurred). Set the wait time to infinite; this will
    // make the scheduler thread sleep until it gets another thread message.
    if (lWait == 0)
    {
        lWait = INFINITE;
    }

    *plNextSleep = lWait;
    return hr;
}


//-----------------------------------------------------------------------------
// ProcessSample
//
// Processes a sample.
//
// plNextSleep: Receives the length of time the scheduler thread should sleep.
//-----------------------------------------------------------------------------


HRESULT CEvrPresentScheduler::ProcessSample(IMFSample *pSample, LONG *plNextSleep, const FrameRateInfo &fr)
{
    HRESULT hr = S_OK;

    LONGLONG hnsPresentationTime = 0;
	LONGLONG hnsDelta = 0;
    LONGLONG hnsTimeNow = 0;
    MFTIME   hnsSystemTime = 0;
    LONGLONG hnsPresentNowThreshold = fr.PerFrame_1_4th;

    BOOL bPresentNow = TRUE;
    LONG lNextSleep = 0;

    if (m_pClock)
    {
        // Get the sample's time stamp. It is valid for a sample to
        // have no time stamp.
        hr = pSample->GetSampleTime(&hnsPresentationTime);
    
        // Get the clock time. (But if the sample does not have a time stamp, 
        // we don't need the clock time.)
        if (SUCCEEDED(hr))
        {
            hr = m_pClock->GetCorrelatedTime(0, &hnsTimeNow, &hnsSystemTime);
        }

        // Calculate the time until the sample's presentation time. 
        // A negative value means the sample is late.
        hnsDelta = hnsPresentationTime - hnsTimeNow;   
        if (m_fRate < 0)
        {
            // For reverse playback, the clock runs backward. Therefore the delta is reversed.
            hnsDelta = - hnsDelta;
        }

        if (hnsDelta < - hnsPresentNowThreshold)
        {
            // This sample is late. 
            bPresentNow = TRUE;
        }
        else if (hnsDelta > (3 * hnsPresentNowThreshold))
        {
            // This sample is still too early. Go to sleep.
			// 100 nanoseconds to millisecond
            lNextSleep = (LONG)((hnsDelta - (3 * hnsPresentNowThreshold)) / 10000);

            // Adjust the sleep time for the clock rate. (The presentation clock runs
            // at m_fRate, but sleeping uses the system clock.)
            lNextSleep = (LONG)(lNextSleep / fabsf(m_fRate));

            // Don't present yet.
            bPresentNow = FALSE;
        }
    }

    if (bPresentNow)
    {
        hr = pSample->SetUINT64(MFSamplePresenter_DisplayTargetTime, hnsPresentationTime);
		hr = Present(pSample, hnsDelta);
//        hr = m_pCB->PresentSample(pSample, hnsPresentationTime);
    }
    else
    {
        // The sample is not ready yet. Return it to the queue.
        hr = m_ScheduledSamples.PutBack(pSample);
    }

    *plNextSleep = lNextSleep;

    return hr;
}

// Process sample for frame rate conversion.
HRESULT CEvrPresentScheduler::ProcessSampleFRC(IMFSample *pSample, LONG *plNextSleep, const FrameRateInfo &fr)
{
    HRESULT hr = S_OK;

    LONGLONG hnsPresentationTime = 0;
    LONGLONG hnsPresentationDuration = 0;
    LONGLONG hnsPresentationEndTime = 0;
	LONGLONG hnsDelta = 0;
    LONGLONG hnsTimeNow = 0;
    MFTIME hnsSystemTime = 0;
    LONGLONG hnsDisplayTargetTime = 0;
    LONGLONG hnsPresentNowThreshold = fr.PerFrame_1_4th;

    BOOL bPresentNow = TRUE;
    BOOL bRetainSample = TRUE;
    BOOL bRepeatFrame = FALSE;
    LONG lNextSleep = 0;

    if (m_pClock)
    {
        hr = pSample->GetUINT64(MFSamplePresenter_DisplayTargetTime, (UINT64 *)&hnsDisplayTargetTime);
        // At first, each sample does not contain MFSamplePresenter_DisplayTargetTime attribute.
        if (SUCCEEDED(hr))
            bRepeatFrame = TRUE;

        // Get the sample's time stamp. It is valid for a sample to
        // have no time stamp.
        hr = pSample->GetSampleTime(&hnsPresentationTime);

        hr = pSample->GetSampleDuration(&hnsPresentationDuration);

        hnsPresentationEndTime = hnsPresentationTime + hnsPresentationDuration - hnsPresentNowThreshold;

        if (hnsDisplayTargetTime <= 0)
        {
            hnsDisplayTargetTime = hnsPresentationTime;
        }
        else
        {
            if (hnsDisplayTargetTime + fr.PerFrameInterval < hnsPresentationEndTime)
                hnsDisplayTargetTime += fr.PerFrameInterval;
            else
                bRetainSample = FALSE;
        }

        // Get the clock time. (But if the sample does not have a time stamp, 
        // we don't need the clock time.)
        if (SUCCEEDED(hr))
        {
            hr = m_pClock->GetCorrelatedTime(0, &hnsTimeNow, &hnsSystemTime);
        }

        // Calculate the time until the sample's presentation time. 
        // A negative value means the sample is late.
        hnsDelta = hnsDisplayTargetTime - hnsTimeNow;   
        if (m_fRate < 0)
        {
            // For reverse playback, the clock runs backward. Therefore the delta is reversed.
            hnsDelta = - hnsDelta;
        }

        if (hnsDelta < - hnsPresentNowThreshold)
        {
            // This sample is late. 
            bPresentNow = TRUE;
        }
        else if (hnsDelta > (3 * hnsPresentNowThreshold))
        {
            // This sample is still too early. Go to sleep.
			// 100 nanoseconds to millisecond
            lNextSleep = (LONG)((hnsDelta - (3 * hnsPresentNowThreshold)) / 10000);

            // Adjust the sleep time for the clock rate. (The presentation clock runs
            // at m_fRate, but sleeping uses the system clock.)
            lNextSleep = (LONG)(lNextSleep / fabsf(m_fRate));

            // Don't present yet.
            bPresentNow = FALSE;
        }
    }

    if (bPresentNow)
    {
        if (bRetainSample)
        {
            hr = pSample->SetUINT64(MFSamplePresenter_DisplayTargetTime, hnsDisplayTargetTime);
            hr = pSample->SetUINT32(MFSamplePresenter_RepeatFrame, bRepeatFrame);
            hr = Present(pSample, hnsDelta);
            hr = m_ScheduledSamples.PutBack(pSample);
        }
    }
    else
    {
        // The sample is not ready yet. Return it to the queue.
        hr = m_ScheduledSamples.PutBack(pSample);
    }

    *plNextSleep = lNextSleep;

    return hr;
}


//-----------------------------------------------------------------------------
// SchedulerThreadProc (static method)
// 
// ThreadProc for the scheduler thread.
//-----------------------------------------------------------------------------

DWORD WINAPI CEvrPresentScheduler::SchedulerThreadProc(LPVOID lpParameter)
{
    CEvrPresentScheduler* pScheduler = reinterpret_cast<CEvrPresentScheduler*>(lpParameter);
    if (pScheduler == NULL)
    {
        return -1;
    }
    return pScheduler->SchedulerThreadProcPrivate();
}

//-----------------------------------------------------------------------------
// SchedulerThreadProcPrivate
// 
// Non-static version of the ThreadProc.
//-----------------------------------------------------------------------------

DWORD CEvrPresentScheduler::SchedulerThreadProcPrivate()
{
    HRESULT hr = S_OK;
    MSG     msg;
    LONG    lWait = INFINITE;
    BOOL    bExitThread = FALSE;

    // Force the system to create a message queue for this thread.
    // (See MSDN documentation for PostThreadMessage.)
    PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);

    // Signal to the scheduler that the thread is ready.
    SetEvent(m_hThreadReadyEvent);

    while( !bExitThread )
    {
        // Wait for a thread message OR until the wait time expires.
        DWORD dwResult = MsgWaitForMultipleObjects(0, NULL, FALSE, lWait, QS_POSTMESSAGE);

        if (dwResult == WAIT_TIMEOUT)
        {
            // If we timed out, then process the samples in the queue
            hr = ProcessSamplesInQueue(&lWait);
            if (FAILED(hr))
            {
                bExitThread = TRUE;
            }
        }

        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            BOOL bProcessSamples = TRUE;

            switch (msg.message) 
            {
            case eTerminate:
                DbgMsg("eTerminate");
                bExitThread = TRUE;
                break;

			// Flushing: Clear the sample queue and set the event.
            case eFlush:
				// We need to keep one sample unless flush is called when clock is stopped (msg.wParam = TRUE).
				if (msg.wParam == TRUE)	// flush all samples
				{
					// Unregister video sample and release m_pPresentSample.
					UpdatePresentSample(NULL);
				}

				while (true)
				{
					CComPtr<IMFSample> pSample;
					m_ScheduledSamples.Dequeue(&pSample);
					if (pSample == NULL)
						break;

					if (m_pEvrCallback)
						m_pEvrCallback->OnSampleFree(pSample);
				}
//              m_ScheduledSamples.Clear();

                lWait = INFINITE;
                SetEvent(m_hFlushEvent);
                break;

            case eSchedule:
                // Process as many samples as we can.
                if (bProcessSamples)
                {
                    hr = ProcessSamplesInQueue(&lWait);
                    if (FAILED(hr))
                    {
                        bExitThread = TRUE;
                    }
                    bProcessSamples = (lWait != INFINITE); 
                }
                break;
            } // switch  

        } // while PeekMessage
    
    }  // while (!bExitThread)

    DbgMsg("Exit scheduler thread.");
    return (SUCCEEDED(hr) ? 0 : 1);
}

HRESULT CEvrPresentScheduler::PeekPresentSample(IMFSample **ppSample)
{
	CHECK_POINTER(ppSample)

	CAutoLock lock(&m_PresentSampleLock);
	*ppSample = m_pPresentSample;
	if (*ppSample)
	{
		(*ppSample)->AddRef();
		return S_OK;
	}
	else
		return S_FALSE;
}

HRESULT CEvrPresentScheduler::PeekPresentTexture(IDirect3DTexture9 **ppTexture)
{
	CHECK_POINTER(ppTexture)

//	EPS_DP("UpdatePresentSample m_pPresentTexture = 0x%08x", m_pPresentTexture);
	CAutoLock lock(&m_PresentSampleLock);
	if (!m_pPresentTexture)
		return S_FALSE;
	m_pPresentTexture.CopyTo(ppTexture);
	return S_OK;
}

HRESULT CEvrPresentScheduler::UpdatePresentSample(IMFSample *pSample, LONGLONG llDelta)
{
	CComPtr<IMFSample>  pPreviousSample;
	{
		CAutoLock lock(&m_PresentSampleLock);
		EPS_DP("UpdatePresentSample pSample = 0x%08x", pSample);

// In FRUC case, the same sample can be presented multiple times.
//		if (pSample != NULL && m_pPresentSample == pSample)
//			return S_FALSE;

		pPreviousSample = m_pPresentSample;

		m_pPresentSample = pSample;
	}

	if (m_pEvrCallback)
	{
		m_pEvrCallback->OnUpdateLastSample(m_pPresentSample, llDelta);

		if (pPreviousSample != pSample && pPreviousSample)
		{
			m_pEvrCallback->OnSampleFree(pPreviousSample);
		}
	}

	return S_OK;
}

HRESULT CEvrPresentScheduler::Present(IMFSample *pSample, LONGLONG llDelta)
{
	HRESULT hr = E_FAIL;

	// avoid present function called at the same time
	CAutoLock lock(&m_PresentLock);

	hr = UpdatePresentSample(pSample, llDelta);

	m_QualProp.FramesDrawn++;
	return S_OK;
}

HRESULT CEvrPresentScheduler::get_FramesDroppedInRenderer(int *pcFrames)
{
	*pcFrames = m_QualProp.FramesDropped;
	return S_OK;
}

HRESULT CEvrPresentScheduler::get_FramesDrawn(int *pcFramesDrawn)
{
	*pcFramesDrawn = m_QualProp.FramesDrawn;
	return S_OK;
}

HRESULT CEvrPresentScheduler::get_AvgFrameRate(int *piAvgFrameRate)
{
	*piAvgFrameRate = m_QualProp.AvgFrameRate;
	return S_OK;
}

HRESULT CEvrPresentScheduler::get_Jitter(int *iJitter)
{
	*iJitter = m_QualProp.Jitter;
	return S_OK;
}

HRESULT CEvrPresentScheduler::get_AvgSyncOffset(int *piAvg)
{
	*piAvg = m_QualProp.AvgSyncOffset;
	return S_OK;
}

HRESULT CEvrPresentScheduler::get_DevSyncOffset(int *piDev)
{
	*piDev = m_QualProp.FramesDropped;
	return S_OK;
}

void CEvrPresentScheduler::FrameRateInfo::SetRate(const MFRatio& fps)
{
    // Convert to a duration.
//    MFFrameRateToAverageTimePerFrame(fps.Numerator, fps.Denominator, &AvgTimePerFrame);
	UINT64 Denominator = (UINT64)fps.Denominator * 100000000;
	UINT64 Temp = Denominator / fps.Numerator;
	UINT64 AvgTimePerFrame = (Temp / 10) + ((Temp % 10) >= 5 ? 1 : 0);

    PerFrameInterval = (MFTIME)AvgTimePerFrame;

    // Calculate 1/4th of this value, because we use it frequently.
    PerFrame_1_4th = PerFrameInterval / 4;
}