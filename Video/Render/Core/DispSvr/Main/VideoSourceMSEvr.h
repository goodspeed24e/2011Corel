#pragma once

#include "EvrHelper.h"
#include "EvrSamplePool.h"
#include "EvrPresentScheduler.h"
#include "VideoSourcePropertyExtension.h"
// RENDER_STATE: Defines the state of the presenter. 
enum RENDER_STATE
{
    RENDER_STATE_STARTED = 1,
    RENDER_STATE_STOPPED,
    RENDER_STATE_PAUSED,
    RENDER_STATE_SHUTDOWN,  // Initial state. 

    // State transitions:

    // InitServicePointers                  -> STOPPED
    // ReleaseServicePointers               -> SHUTDOWN
    // IMFClockStateSink::OnClockStart      -> STARTED
    // IMFClockStateSink::OnClockRestart    -> STARTED
    // IMFClockStateSink::OnClockPause      -> PAUSED
    // IMFClockStateSink::OnClockStop       -> STOPPED
};

// FRAMESTEP_STATE: Defines the presenter's state with respect to frame-stepping.
enum FRAMESTEP_STATE
{
    FRAMESTEP_NONE,             // Not frame stepping.
    FRAMESTEP_WAITING_START,    // Frame stepping, but the clock is not started.
    FRAMESTEP_PENDING,          // Clock is started. Waiting for samples.
    FRAMESTEP_SCHEDULED,        // Submitted a sample for rendering.
    FRAMESTEP_COMPLETE          // Sample was rendered. 

    // State transitions:

    // MFVP_MESSAGE_STEP                -> WAITING_START
    // OnClockStart/OnClockRestart      -> PENDING
    // MFVP_MESSAGE_PROCESSINPUTNOTIFY  -> SUBMITTED
    // OnSampleFree                     -> COMPLETE
    // MFVP_MESSAGE_CANCEL              -> NONE
    // OnClockStop                      -> NONE
    // OnClockSetRate( non-zero )       -> NONE
};

class CEvrCustomMixer;
//-----------------------------------------------------------------------------
//  EVRCustomPresenter class
//  Description: Implements the custom presenter.
//-----------------------------------------------------------------------------

class CVideoSourceMSEvr : 
 //   BaseObject,  
 //   RefCountedObject, 
    // COM interfaces:
	public CUnknown,
	public IDisplayVideoSource,
    public IMFVideoDeviceID,
    public IMFVideoPresenter, // Inherits IMFClockStateSink
//    public IMFRateSupport,
    public IMFGetService,
    public IMFTopologyServiceLookupClient,
    public IMFVideoDisplayControl,
	public IQualProp,
	public IEvrSchedulerCallback,
	public IDisplayVideoSourcePropertyExtension
{

public:
//	CVideoSourceMSEvr(HRESULT& hr);
	CVideoSourceMSEvr(IDisplayLock* pLock, IDisplayVideoSink* pVideoSink);
	virtual ~CVideoSourceMSEvr();

	DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID, void**);
 
	// IDisplayVideoSource implementation
	STDMETHOD(GetGraph)(IFilterGraph** ppGraph);
	STDMETHOD(GetTexture)(IUnknown** ppTexture, NORMALIZEDRECT* lpNormRect);
	STDMETHOD(GetVideoSize)(LONG* plWidth, LONG* plHeight, float *pfAspectRatio);
	STDMETHOD(Attach)(IBaseFilter* pVMRFilter);
	STDMETHOD(Detach)();
	STDMETHOD(BeginDraw)();
	STDMETHOD(EndDraw)();
	STDMETHOD(IsValid)();
	STDMETHOD(ClearImage)();
	STDMETHOD(BeginDeviceLoss)();
	STDMETHOD(EndDeviceLoss)(IUnknown* pDevice);
	STDMETHOD(EnableInitiativeDisplay)(BOOL bEnable);

    // IMFGetService methods
    STDMETHOD(GetService)(REFGUID guidService, REFIID riid, LPVOID *ppvObject);

    // IMFVideoPresenter methods
    STDMETHOD(ProcessMessage)(MFVP_MESSAGE_TYPE eMessage, ULONG_PTR ulParam);
    STDMETHOD(GetCurrentMediaType)(IMFVideoMediaType** ppMediaType);

    // IMFClockStateSink methods
    STDMETHOD(OnClockStart)(MFTIME hnsSystemTime, LONGLONG llClockStartOffset);
    STDMETHOD(OnClockStop)(MFTIME hnsSystemTime);
    STDMETHOD(OnClockPause)(MFTIME hnsSystemTime);
    STDMETHOD(OnClockRestart)(MFTIME hnsSystemTime);
    STDMETHOD(OnClockSetRate)(MFTIME hnsSystemTime, float flRate);

	// We don't need rate support now
    // IMFRateSupport methods
    //STDMETHOD(GetSlowestRate)(MFRATE_DIRECTION eDirection, BOOL bThin, float *pflRate);
    //STDMETHOD(GetFastestRate)(MFRATE_DIRECTION eDirection, BOOL bThin, float *pflRate);
    //STDMETHOD(IsRateSupported)(BOOL bThin, float flRate, float *pflNearestSupportedRate);

    // IMFVideoDeviceID methods
    STDMETHOD(GetDeviceID)(IID* pDeviceID);

    // IMFTopologyServiceLookupClient methods
    STDMETHOD(InitServicePointers)(IMFTopologyServiceLookup *pLookup);
    STDMETHOD(ReleaseServicePointers)();

    // IMFVideoDisplayControl methods
    STDMETHOD(GetNativeVideoSize)(SIZE* pszVideo, SIZE* pszARVideo);
    STDMETHOD(GetIdealVideoSize)(SIZE* pszMin, SIZE* pszMax);
    STDMETHOD(SetVideoPosition)(const MFVideoNormalizedRect* pnrcSource, const LPRECT prcDest);
    STDMETHOD(GetVideoPosition)(MFVideoNormalizedRect* pnrcSource, LPRECT prcDest);
    STDMETHOD(SetAspectRatioMode)(DWORD dwAspectRatioMode);
    STDMETHOD(GetAspectRatioMode)(DWORD* pdwAspectRatioMode);
    STDMETHOD(SetVideoWindow)(HWND hwndVideo);
    STDMETHOD(GetVideoWindow)(HWND* phwndVideo);
    STDMETHOD(RepaintVideo)();
    STDMETHOD(GetCurrentImage)(BITMAPINFOHEADER* pBih, BYTE** pDib, DWORD* pcbDib, LONGLONG* pTimeStamp);
    STDMETHOD(SetBorderColor)(COLORREF Clr) { return E_NOTIMPL; }
    STDMETHOD(GetBorderColor)(COLORREF* pClr) { return E_NOTIMPL; }
    STDMETHOD(SetRenderingPrefs)(DWORD dwRenderFlags) { return E_NOTIMPL; }
    STDMETHOD(GetRenderingPrefs)(DWORD* pdwRenderFlags) { return E_NOTIMPL; }
    STDMETHOD(SetFullscreen)(BOOL bFullscreen) { return E_NOTIMPL; }
    STDMETHOD(GetFullscreen)(BOOL* pbFullscreen) { return E_NOTIMPL; }

	// IQualProp
	STDMETHOD(get_FramesDroppedInRenderer)(int *pcFrames);
	STDMETHOD(get_FramesDrawn)(int *pcFramesDrawn);
	STDMETHOD(get_AvgFrameRate)(int *piAvgFrameRate);
	STDMETHOD(get_Jitter)(int *iJitter);
	STDMETHOD(get_AvgSyncOffset)(int *piAvg);
	STDMETHOD(get_DevSyncOffset)(int *piDev);

	//IEvrSchdeuderCallback
	STDMETHOD(OnUpdateLastSample)(IMFSample *pSample, LONGLONG llDelta);
	STDMETHOD(OnSampleFree)(IMFSample *pSample);

	// IDisplayVideoSourcePropertyExtension
	STDMETHOD(GetSampleProperty)(SampleProperty *pProperty);

protected:
    // CheckShutdown: 
    //     Returns MF_E_SHUTDOWN if the presenter is shutdown.
    //     Call this at the start of any methods that should fail after shutdown.
    inline HRESULT CheckShutdown() const 
    {
        if (m_RenderState == RENDER_STATE_SHUTDOWN)
        {
            return MF_E_SHUTDOWN;
        }
        else
        {
            return S_OK;
        }
    }

    // IsActive: The "active" state is started or paused.
    inline BOOL IsActive() const
    {
        return ((m_RenderState == RENDER_STATE_STARTED) || (m_RenderState == RENDER_STATE_PAUSED));
    }

    // IsScrubbing: Scrubbing occurs when the frame rate is 0.
    inline BOOL IsScrubbing() const { return m_fRate == 0.0f; }

    // NotifyEvent: Send an event to the EVR through its IMediaEventSink interface.
    void NotifyEvent(long EventCode, LONG_PTR Param1, LONG_PTR Param2)
    {
        if (m_pMediaEventSink)
        {
            m_pMediaEventSink->Notify(EventCode, Param1, Param2);
        }
    }

    float GetMaxRate(BOOL bThin);

    // Mixer operations
    HRESULT ConfigureMixer(IMFTransform *pMixer);

    // Formats
    HRESULT CreateOptimalVideoType(IMFMediaType* pProposed, IMFMediaType **ppOptimal);
    HRESULT CalculateOutputRectangle(IMFMediaType *pProposed, RECT *prcOutput);
    HRESULT SetMediaType(IMFMediaType *pMediaType);
    HRESULT IsMediaTypeSupported(IMFMediaType *pMediaType);

    // Message handlers
    HRESULT Flush();
    HRESULT RenegotiateMediaType();
    HRESULT ProcessInputNotify();
    HRESULT BeginStreaming();
    HRESULT EndStreaming();
    HRESULT CheckEndOfStream();

    // Managing samples
    void    ProcessOutputLoop();   
	HRESULT ProcessOutput();
    HRESULT DeliverSample(IMFSample *pSample, BOOL bRepaint);
    HRESULT TrackSample(IMFSample *pSample);
    void    ReleaseResources();
	BOOL IsSampleBeingUsedInMixer(IMFSample *pSample);

	// Frame-stepping
    HRESULT PrepareFrameStep(DWORD cSteps);
    HRESULT StartFrameStep();
    HRESULT DeliverFrameStepSample(IMFSample *pSample);
    HRESULT CompleteFrameStep(IMFSample *pSample);
    HRESULT CancelFrameStep();

    // Callbacks

    // Callback when a video sample is released.
    HRESULT OnSampleFree(IMFAsyncResult *pResult);
	CAsyncCallback<CVideoSourceMSEvr>   m_SampleFreeCB;

	RECT    CorrectAspectRatio(const RECT& src, const MFRatio& srcPAR, const MFRatio& destPAR);
	BOOL    AreMediaTypesEqual(IMFMediaType *pType1, IMFMediaType *pType2);
	HRESULT ValidateVideoArea(const MFVideoArea& area, UINT32 width, UINT32 height);
	HRESULT SetDesiredSampleTime(IMFSample *pSample, const LONGLONG& hnsSampleTime, const LONGLONG& hnsDuration);
	HRESULT ClearDesiredSampleTime(IMFSample *pSample);
	BOOL    IsSampleTimePassed(IMFClock *pClock, IMFSample *pSample);
	HRESULT SetMixerSourceRect(IMFTransform *pMixer, const MFVideoNormalizedRect& nrcSource);

	inline void ReleaseEventCollection(DWORD cOutputBuffers, MFT_OUTPUT_DATA_BUFFER* pBuffers);

	inline float MFOffsetToFloat(const MFOffset& offset);

	inline HRESULT GetFrameRate(IMFMediaType *pType, MFRatio *pRatio);

	inline void DumpOutputAvailableType();

protected:

    // FrameStep: Holds information related to frame-stepping. 
    // Note: The purpose of this structure is simply to organize the data in one variable.
    struct FrameStep
    {
        FrameStep() : state(FRAMESTEP_NONE), steps(0), pSampleNoRef(NULL)
        {
        }

        FRAMESTEP_STATE     state;          // Current frame-step state.
        CEvrVideoSampleList     samples;        // List of pending samples for frame-stepping.
        DWORD               steps;          // Number of steps left.
        DWORD_PTR           pSampleNoRef;   // Identifies the frame-step sample.
    };


protected:

    RENDER_STATE                m_RenderState;          // Rendering state.
    FrameStep                   m_FrameStep;            // Frame-stepping information.

    CCritSec                     m_ObjectLock;			// Serializes our public methods.  

	// Samples and scheduling
    CEvrPresentScheduler    m_scheduler;			// Manages scheduling of samples.
    CEvrSamplePool          m_SamplePool;           // Pool of allocated samples.
    DWORD                       m_TokenCounter;         // Counter. Incremented whenever we create new samples.

	// Rendering state
	BOOL						m_bSampleNotify;		// Did the mixer signal it has an input sample?
	BOOL						m_bRepaint;				// Do we need to repaint the last sample?
	BOOL						m_bPrerolled;	        // Have we presented at least one sample?
    BOOL                        m_bEndStreaming;		// Did we reach the end of the stream (EOS)?

    MFVideoNormalizedRect       m_nrcSource;            // Source rectangle.
    float                       m_fRate;                // Playback rate.

    // Deletable objects.
//    D3DPresentEngine            *m_pD3DPresentEngine;    // Rendering engine. (Never null if the constructor succeeds.)

    // COM interfaces.
    IMFClock                    *m_pClock;               // The EVR's clock.
    IMFTransform                *m_pMixer;               // The mixer.
    IMediaEventSink             *m_pMediaEventSink;      // The EVR's event-sink interface.
    IMFMediaType                *m_pMediaType;           // Output media type
//Integration from VideoSourceEvr
protected:
	void Cleanup();
	HRESULT DisconnectPins();

	BOOL m_bValid;
	LONG m_lImageWidth;
	LONG m_lImageHeight;
	FLOAT m_fAspectRatio;
	FLOAT m_fNativeAspectRatio;

	NORMALIZEDRECT m_nrcTexture;

	UINT m_uDeviceManagerToken;

	CComPtr<IFilterGraph> m_pGraph;
	CComPtr<IBaseFilter> m_pVMR;
	CComPtr<IDirect3DDeviceManager9> m_pDeviceManager;
	CEvrCustomMixer *m_pCustomMixer;

	CComPtr<IDisplayVideoSink> m_pVideoSink;
	CComPtr<IDisplayLock> m_pLock;
	CComPtr<IDisplayServerStateEventSink> m_pStateEventSink;
	CComPtr<IDisplayObject> m_pDispObj;
	BOOL m_bInitiativeDisplay;
	HANDLE m_hDeviceHandle;

	HWND m_hwndVideo;
	RECT m_dstrect;
	DWORD  m_dwAspectRatioMode;
	DWORD m_dwOutputStreamId;

	BOOL m_bHoldPresentSample;
	CVideoSourceDisplayHelper m_DisplayHelper;
	BOOL m_bDShowWorkaround;
	CEvrVideoSampleList m_SampleInMixerList;
};