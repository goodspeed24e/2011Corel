#ifndef __DISPSVR_EVR_CUSTOM_MIXER_H__
#define __DISPSVR_EVR_CUSTOM_MIXER_H__

#include <evr.h>
#include <mftransform.h>

// EVR supports a max of 16 input streams and one output to presenter.
#ifndef MAX_STREAMS
#define MAX_STREAMS 16
#endif

class CEvrCustomMixer :
	public IMFVideoDeviceID,
	public IMFTransform,
	public IMFTopologyServiceLookupClient
{
public:
	CEvrCustomMixer();
	virtual ~CEvrCustomMixer();

	// IUnknown
	STDMETHOD(QueryInterface)(REFIID riid, void **ppv);
	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();

	// IMFVideoDeviceID
	STDMETHODIMP GetDeviceID(IID* pDeviceID);

	// IMFTransform
	STDMETHODIMP GetStreamLimits( 
		DWORD   *pdwInputMinimum,
		DWORD   *pdwInputMaximum,
		DWORD   *pdwOutputMinimum,
		DWORD   *pdwOutputMaximum
		);

	STDMETHODIMP GetStreamCount(
		DWORD   *pcInputStreams,
		DWORD   *pcOutputStreams
		);

	STDMETHODIMP GetStreamIDs(
		DWORD	dwInputIDArraySize,
		DWORD   *pdwInputIDs,
		DWORD	dwOutputIDArraySize,
		DWORD	*pdwOutputIDs
		);

	STDMETHODIMP GetInputStreamInfo(
		DWORD                     dwInputStreamID,
		MFT_INPUT_STREAM_INFO *   pStreamInfo
		);

	STDMETHODIMP GetOutputStreamInfo(
		DWORD                     dwOutputStreamID, 
		MFT_OUTPUT_STREAM_INFO *  pStreamInfo      
		);

	STDMETHODIMP GetAttributes(IMFAttributes** pAttributes);

	STDMETHODIMP GetInputStreamAttributes(
		DWORD           dwInputStreamID,
		IMFAttributes   **ppAttributes
		);

	STDMETHODIMP GetOutputStreamAttributes(
		DWORD           dwOutputStreamID,
		IMFAttributes   **ppAttributes
		);

	STDMETHODIMP DeleteInputStream(DWORD dwStreamID);

	STDMETHODIMP AddInputStreams( 
		DWORD   cStreams,
		DWORD   *adwStreamIDs
		);

	STDMETHODIMP GetInputAvailableType(
		DWORD           dwInputStreamID,
		DWORD           dwTypeIndex, // 0-based
		IMFMediaType    **ppType
		);

	STDMETHODIMP GetOutputAvailableType(
		DWORD           dwOutputStreamID,
		DWORD           dwTypeIndex, // 0-based
		IMFMediaType    **ppType
		);

	STDMETHODIMP SetInputType(
		DWORD           dwInputStreamID,
		IMFMediaType    *pType,
		DWORD           dwFlags 
		);

	STDMETHODIMP SetOutputType(
		DWORD           dwOutputStreamID,
		IMFMediaType    *pType,
		DWORD           dwFlags 
		);

	STDMETHODIMP GetInputCurrentType(
		DWORD           dwInputStreamID,
		IMFMediaType    **ppType
		);

	STDMETHODIMP GetOutputCurrentType(
		DWORD           dwOutputStreamID,
		IMFMediaType    **ppType
		);

	STDMETHODIMP GetInputStatus(
		DWORD			dwInputStreamID,
		DWORD           *pdwFlags 
		);

	STDMETHODIMP GetOutputStatus(DWORD *pdwFlags);

	STDMETHODIMP SetOutputBounds(
		LONGLONG        hnsLowerBound,
		LONGLONG        hnsUpperBound
		);

	STDMETHODIMP ProcessEvent(
		DWORD              dwInputStreamID,
		IMFMediaEvent      *pEvent 
		);

	STDMETHODIMP ProcessMessage(
		MFT_MESSAGE_TYPE	eMessage,
		ULONG_PTR			ulParam
		);

	STDMETHODIMP ProcessInput(
		DWORD				dwInputStreamID,
		IMFSample           *pSample, 
		DWORD				dwFlags 
		);

	STDMETHODIMP ProcessOutput(
		DWORD					dwFlags, 
		DWORD					cOutputBufferCount,
		MFT_OUTPUT_DATA_BUFFER  *pOutputSamples, // one per stream
		DWORD                   *pdwStatus  
		);

	// IMFTopologyServiceLookupClient
	STDMETHODIMP InitServicePointers(IMFTopologyServiceLookup* pLookup);
	STDMETHODIMP ReleaseServicePointers();

protected:
	HRESULT CheckInputMediaType(IMFMediaType *pType);
	HRESULT CheckOutputMediaType(IMFMediaType *pType);
	HRESULT VerifyInputStreamID(DWORD dwID);
	HRESULT VerifyOutputStreamID(DWORD dwID);
	HRESULT GetInputAvailableType(DWORD dwIndex, IMFMediaType **ppType);
	HRESULT GetOutputAvailableType(DWORD dwIndex, IMFMediaType **ppType);
	HRESULT OnFlush();

protected:
	LONG m_cRef;
	CCritSec m_csMixerLock;
	DWORD m_nStreams;
	DWORD m_StreamIDs[MAX_STREAMS];
	IMFMediaType *m_pInputTypes[MAX_STREAMS];
	IMFMediaType *m_pOutputType;
	IMFSample *m_pSamples[MAX_STREAMS];
	CComPtr<IMediaEventSink> m_pMediaEventSink;
};

#endif	//__DISPSVR_EVR_CUSTOM_MIXER_H__