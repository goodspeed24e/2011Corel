#pragma once

#include <evr.h>
#include "VideoSourcePropertyExtension.h"

interface IDirect3DDeviceManager9;
class CEvrCustomMixer;

class CVideoSourceEvrEx :
	public IDisplayVideoSource,
	public IMFGetService,
	public IMFTopologyServiceLookupClient,
	public IMFVideoDeviceID,
	public IMFVideoPresenter,
	public IMFVideoDisplayControl,
	public IDisplayVideoSourcePropertyExtension
{
public:

	CVideoSourceEvrEx(IDisplayLock* pLock, IDisplayVideoSink* pVideoSink);
	virtual ~CVideoSourceEvrEx();

	// IUnknown
	STDMETHOD(QueryInterface)(REFIID riid, void **ppv);
	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();

	// IDisplayVideoSource implementation
	STDMETHOD(GetGraph)(IFilterGraph** ppGraph);
	STDMETHOD(GetTexture)(IUnknown** ppTexture, NORMALIZEDRECT* lpNormRect);
	STDMETHOD(GetVideoSize)(LONG* plWidth, LONG* plHeight, float *pfAspectRatio);
	STDMETHOD(Attach)(IBaseFilter* pVMRFilter);
	STDMETHOD(Detach)();
	STDMETHOD(BeginDraw)() { return S_OK; }
	STDMETHOD(EndDraw)() { return S_OK; }
	STDMETHOD(IsValid)();
	STDMETHOD(ClearImage)();
	STDMETHOD(BeginDeviceLoss)();
	STDMETHOD(EndDeviceLoss)(IUnknown* pDevice);
	STDMETHOD(EnableInitiativeDisplay)(BOOL bEnable) { return S_FALSE; }

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
	STDMETHOD(GetIdealVideoSize)(SIZE* pszMin, SIZE* pszMax);// { return E_NOTIMPL; }
	STDMETHOD(SetVideoPosition)(const MFVideoNormalizedRect* pnrcSource, const LPRECT prcDest);
	STDMETHOD(GetVideoPosition)(MFVideoNormalizedRect* pnrcSource, LPRECT prcDest);
	STDMETHOD(SetAspectRatioMode)(DWORD dwAspectRatioMode);
	STDMETHOD(GetAspectRatioMode)(DWORD* pdwAspectRatioMode);
	STDMETHOD(SetVideoWindow)(HWND hwndVideo);
	STDMETHOD(GetVideoWindow)(HWND* phwndVideo) { return E_NOTIMPL; }
	STDMETHOD(RepaintVideo)();
	STDMETHOD(GetCurrentImage)(BITMAPINFOHEADER* pBih, BYTE** pDib, DWORD* pcbDib, LONGLONG* pTimeStamp) { return E_NOTIMPL; }
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
	static DWORD WINAPI EvrPresentThread(LPVOID lpParameter);
	HRESULT StartEvrPresentThread();
	void StopEvrPresentThread();
	void PresentLoop();
	HRESULT ProcessOutputFromMixer(IMFSample **ppSample);
	void OnPresent(LONG lSleep);
	HRESULT OnIncomingSample(IMFSample *pSample);

	void Cleanup();
	HRESULT DisconnectPins();
	HRESULT RenegotiateMediaType();
	HRESULT CreateDesiredOutputType(IMFMediaType *pType, IMFMediaType **ppDesiredType);
	HRESULT SetMediaType(IMFMediaType *pType);

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

	LONG m_cRef;
	UINT m_uWidth;
	UINT m_uHeight;
	FLOAT m_fAspectRatio;
	FLOAT m_fPixelAspectRatio;
	FLOAT m_fNativeAspectRatio;

	IDisplayVideoSink *m_pVideoSink;
	IDisplayLock *m_pLock;
	IDisplayObject *m_pDispObj;
	CComPtr<IDisplayServerStateEventSink> m_pStateEventSink;

	BOOL m_bValid;
	bool m_bEndOfStreaming;

	DWORD m_dwOutputStreamId;
	UINT m_uDeviceManagerToken;
	CComPtr<IFilterGraph> m_pGraph;
	CComPtr<IBaseFilter> m_pEVR;
	CComPtr<IDirect3DDeviceManager9> m_pDeviceManager;
	CComPtr<IMediaEventSink> m_pMediaEventSink;
	CComPtr<IMFTransform> m_pMixer;
	CComPtr<IMFClock> m_pClock;
	CComPtr<IMFMediaType> m_pMediaType;
	IMFSample *m_pSample;
	IDirect3DSurface9 *m_pBufferAtIndex0;
	CEvrCustomMixer *m_pCustomMixer;
	CCritSec m_csEvrPresenting;
	HANDLE m_hProcessInputNotify;
	HANDLE m_hFlushNotify;

	HWND m_hwndVideo;
	RECT m_dstrect;
	NORMALIZEDRECT m_nrcTexture;
	DWORD  m_dwAspectRatioMode;
};
