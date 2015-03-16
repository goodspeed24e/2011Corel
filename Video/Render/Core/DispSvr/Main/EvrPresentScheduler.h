#pragma once

//struct SchedulerCallback;
interface IDisplayObject;
interface IEvrSchedulerCallback;
//-----------------------------------------------------------------------------
// Scheduler class
//
// Schedules when a sample should be displayed.
//
// Note: Presentation of each sample is performed by another object which
// must implement SchedulerCallback::PresentSample.
//
// General design:
// The scheduler generally receives samples before their presentation time. It
// puts the samples on a queue and presents them in FIFO order on a worker 
// thread. The scheduler communicates with the worker thread by posting thread
// messages.
//
// The caller has the option of presenting samples immediately (for example,
// for repaints). 
//-----------------------------------------------------------------------------

struct QualPropInfo
{
	int FramesDropped;
	int FramesDrawn;
	int AvgFrameRate;
	int Jitter;
	int AvgSyncOffset;
	int DevSyncOffset;
	LONGLONG TotalFrame;
};

class CEvrPresentScheduler
{
public:
    CEvrPresentScheduler();
    virtual ~CEvrPresentScheduler();

    //void SetCallback(SchedulerCallback *pCB)
    //{
    //    m_pCB = pCB;
    //}

	void SetEvrSchedulerCallback(IEvrSchedulerCallback *pEvrCallback)
	{
		m_pEvrCallback = pEvrCallback;
	}
    void SetFrameRate(const MFRatio& fps);
    void SetClockRate(float fRate) { m_fRate = fRate; }

    const LONGLONG& LastSampleTime() const { return m_LastSampleTime; }
    const LONGLONG& FrameDuration() const { return m_PerFrameInterval; }

    HRESULT StartScheduler(IMFClock *pClock);
    HRESULT StopScheduler();

    HRESULT ScheduleSample(IMFSample *pSample, BOOL bPresentNow);
    HRESULT ProcessSamplesInQueue(LONG *plNextSleep);
    HRESULT ProcessSample(IMFSample *pSample, LONG *plNextSleep);
	HRESULT UpdatePresentSample(IMFSample *pSample, LONGLONG llDelta = 0);
	HRESULT Present(IMFSample *pSample, LONGLONG llDelta);
    HRESULT Flush(BOOL bIsClockStopped);

    // ThreadProc for the scheduler thread.
    static DWORD WINAPI SchedulerThreadProc(LPVOID lpParameter);

	HRESULT PeekPresentSample(IMFSample **ppSample);
	HRESULT PeekPresentTexture(IDirect3DTexture9	**ppTexture);

	//expose qualty prop funtion as IQualProp
	HRESULT get_FramesDroppedInRenderer(int *pcFrames);
	HRESULT get_FramesDrawn(int *pcFramesDrawn);
	HRESULT get_AvgFrameRate(int *piAvgFrameRate);
	HRESULT get_Jitter(int *iJitter);
	HRESULT get_AvgSyncOffset(int *piAvg);
	HRESULT get_DevSyncOffset(int *piDev);

private: 
    // non-static version of SchedulerThreadProc.
    DWORD SchedulerThreadProcPrivate();


private:
	ThreadSafeQueue<IMFSample>	m_ScheduledSamples;		// Samples waiting to be presented.

    CComPtr<IMFClock>   m_pClock;  // Presentation clock. Can be NULL.
//    SchedulerCallback   *m_pCB;     // Weak reference; do not delete.
	IEvrSchedulerCallback *m_pEvrCallback; // Weak reference; do not delete. 

    DWORD               m_dwThreadID;
    HANDLE              m_hSchedulerThread;
    HANDLE              m_hThreadReadyEvent;
    HANDLE              m_hFlushEvent;

    float               m_fRate;                // Playback rate.
    MFTIME              m_PerFrameInterval;     // Duration of each frame.
    LONGLONG            m_PerFrame_1_4th;       // 1/4th of the frame duration.
    MFTIME              m_LastSampleTime;       // Most recent sample time.
	CComPtr<IMFSample>	m_pPresentSample;
	CComPtr<IDirect3DTexture9> m_pPresentTexture;
	CCritSec			m_PresentLock;			// Used for Present function
	CCritSec			m_PresentSampleLock;	// Used for update/get present sample function

	QualPropInfo		m_QualProp;
};


//-----------------------------------------------------------------------------
// SchedulerCallback
//
// Defines the callback method to present samples. 
//-----------------------------------------------------------------------------
//
//struct SchedulerCallback
//{
//    virtual HRESULT PresentSample(IMFSample *pSample, LONGLONG llTarget) = 0;
//};
