#pragma once

#include <evr.h>
#include "VideoSourcePropertyExtension.h"

interface IDirect3DDeviceManager9;
class CVideoSamplePool;
class CPresentQueue;

class CVideoSourceEvr :
	public CUnknown,
	public IDisplayVideoSource,
	public IMFGetService,
	public IMFTopologyServiceLookupClient,
	public IMFVideoDeviceID,
	public IMFVideoPresenter,
	public IMFVideoDisplayControl,
	public IDisplayVideoSourcePropertyExtension
{
public:

	CVideoSourceEvr(IDisplayLock* pLock, IDisplayVideoSink* pVideoSink);
	virtual ~CVideoSourceEvr();

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

	// IMFGetService
	STDMETHOD(GetService)(REFGUID guidService, REFIID riid, LPVOID* ppvObject);

	// IMFTopologyServiceLookupClient
	STDMETHOD(InitServicePointers)(IMFTopologyServiceLookup *pLookup);
	STDMETHOD(ReleaseServicePointers)();

	// IMFVideoDeviceID
	STDMETHOD(GetDeviceID)(IID* pDeviceID);

	// IMFVideoPresenter
	STDMETHOD(GetCurrentMediaType)(IMFVideoMediaType** ppMediaType);
	STDMETHOD(ProcessMessage)(MFVP_MESSAGE_TYPE  eMessage, ULONG_PTR  ulParam);

	// IMFClockStateSink
	STDMETHOD(OnClockPause)(MFTIME hnsSystemTime);
	STDMETHOD(OnClockRestart)(MFTIME hnsSystemTime);
	STDMETHOD(OnClockSetRate)(MFTIME hnsSystemTime, float flRate);
	STDMETHOD(OnClockStart)(MFTIME hnsSystemTime, LONGLONG llClockStartOffset);
	STDMETHOD(OnClockStop)(MFTIME hnssSystemTime);

	// IMFVideoDisplayControl methods
	STDMETHOD(GetNativeVideoSize)(SIZE* pszVideo, SIZE* pszARVideo);
	STDMETHOD(GetIdealVideoSize)(SIZE* pszMin, SIZE* pszMax);
	STDMETHOD(SetVideoPosition)(const MFVideoNormalizedRect* pnrcSource, const LPRECT prcDest);
	STDMETHOD(GetVideoPosition)(MFVideoNormalizedRect* pnrcSource, LPRECT prcDest);
	STDMETHOD(SetAspectRatioMode)(DWORD dwAspectRatioMode);
	STDMETHOD(GetAspectRatioMode)(DWORD* pdwAspectRatioMode);
	STDMETHOD(SetVideoWindow)(HWND hwndVideo);
	STDMETHOD(GetVideoWindow)(HWND* phwndVideo) { return E_NOTIMPL; }
	STDMETHOD(RepaintVideo)();
	STDMETHOD(GetCurrentImage)(BITMAPINFOHEADER* pBih, BYTE** pDib, DWORD* pcbDib, LONGLONG* pTimeStamp);
	STDMETHOD(SetBorderColor)(COLORREF Clr) { return E_NOTIMPL; }
	STDMETHOD(GetBorderColor)(COLORREF* pClr) { return E_NOTIMPL; }
	STDMETHOD(SetRenderingPrefs)(DWORD dwRenderFlags) { return E_NOTIMPL; }
	STDMETHOD(GetRenderingPrefs)(DWORD* pdwRenderFlags) { return E_NOTIMPL; }
	STDMETHOD(SetFullscreen)(BOOL bFullscreen) { return E_NOTIMPL; }
	STDMETHOD(GetFullscreen)(BOOL* pbFullscreen) { return E_NOTIMPL; }

	// IDisplayVideoSourcePropertyExtension methods
	STDMETHOD(GetSampleProperty)(SampleProperty *pProperty);
	STDMETHOD(SetDispFrameRate)(DWORD dwFrameRate) { return E_NOTIMPL; }

private:
	static UINT WINAPI EvrPresentThread(LPVOID lpParameter);
	HRESULT StartEvrPresentThread();
	HRESULT StopEvrPresentThread();
	void PresentLoop();
	HRESULT ProcessInputNotify();
	HRESULT ProcessOutput();
	HRESULT PrepareOutputMediaSamples(IMFMediaType *pType);

	void FlushUnprocessedSamplesInMixer();
	void FlushPendingSamplesForPresent(UINT uFlushToNumberFrame);
	void Cleanup();
	HRESULT DisconnectPins();
	HRESULT RenegotiateMediaType();
	void SetMediaType(IMFMediaType *pType);
	void DumpOutputAvailableType();
    HRESULT CheckMaxDisplayFrameRate();
    inline BOOL IsOverDisplayFrameRate(LONGLONG hnsDuration);
    inline BOOL IsSkipFrameForVideoSync(LONGLONG hnsDuration, LONGLONG hnsDelta);

private:
	enum PRESENTER_STATE {
		PRESENTER_STATE_STARTED = 1,
		PRESENTER_STATE_STOPPED,
		PRESENTER_STATE_PAUSED,
		PRESENTER_STATE_SHUTDOWN,  // Initial state. 
	} m_ePresenterState;

	enum THREAD_STATUS {
		eNotStarted,
		eRunning,
		eWaitingToStop,
		eFinished		
	} m_eThreadStatus;

	LONG m_lImageWidth;
	LONG m_lImageHeight;
	FLOAT m_fAspectRatio;
	FLOAT m_fNativeAspectRatio;
	NORMALIZEDRECT m_nrcTexture;

	CComPtr<IDisplayVideoSink> m_pVideoSink;
	CComPtr<IDisplayLock> m_pLock;
	CComPtr<IDisplayObject> m_pDispObj;
	CComPtr<IDisplayServerStateEventSink> m_pStateEventSink;

	BOOL m_bValid;
	bool m_bFlushPresentQueue;
	UINT m_uFlushPendingPresentQueue;
	UINT m_nInputFramesAfterFlush;
	bool m_bEndOfStreaming;
	bool m_bSampleNotify;
	UINT m_uDebugFlags;
	UINT m_uPresentQueueSize;
	BOOL m_bInitiativeDisplay;
	CCritSec m_csEvrProcessing;
	CCritSec m_csEvrPresenting;
	HANDLE m_hEvrProcessNotify;
	HANDLE m_hEvrPresentFlushEvent;
	HANDLE m_hEvrPresentEvent;
	HANDLE m_hDeviceHandle;
	CVideoSamplePool *m_pSamplePool;
	CPresentQueue *m_pPresentQueue;

	DWORD m_dwOutputStreamId;
	UINT m_uDeviceManagerToken;
	CComPtr<IFilterGraph> m_pGraph;
	CComPtr<IBaseFilter> m_pVMR;
	CComPtr<IDirect3DDeviceManager9> m_pDeviceManager;
	CComPtr<IMediaEventSink> m_pMediaEventSink;
	CComPtr<IMFTransform> m_pMixer;
	CComPtr<IMFClock> m_pClock;
	CComPtr<IMFMediaType> m_pMediaType;

	HWND m_hwndVideo;
	RECT m_dstrect;
	DWORD  m_dwAspectRatioMode;
    DWORD m_dwMaxDisplayFrameRate;
    LONGLONG m_llMinDisplayDuration;
    LONGLONG m_hnsLastPts;
};
