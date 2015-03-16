#ifndef __GENERIC_MEDIA_OBJECT_H
#define __GENERIC_MEDIA_OBJECT_H

/* Forward Declarations */ 
#ifndef __IGMediaBuffer_FWD_DEFINED__
#define __IGMediaBuffer_FWD_DEFINED__
typedef class IGMediaBuffer IGMediaBuffer;
#endif 	/* __IGMediaBuffer_FWD_DEFINED__ */


#ifndef __IGMediaObject_FWD_DEFINED__
#define __IGMediaObject_FWD_DEFINED__
typedef class IGMediaObject IGMediaObject;
#endif 	/* __IGMediaObject_FWD_DEFINED__ */

typedef struct _GMOMediaType
    {
    GUID majortype;
    GUID subtype;
    BOOL bFixedSizeSamples;
    BOOL bTemporalCompression;
    ULONG lSampleSize;
    GUID formattype;
    IUnknown *pUnk;
    ULONG cbFormat;
    /* [size_is] */ BYTE *pbFormat;
    }GMO_MEDIA_TYPE;

typedef LONGLONG REFERENCE_TIME;

typedef struct _GMO_OUTPUT_DATA_BUFFER
    {
    IGMediaBuffer *pBuffer;
    unsigned int dwStatus;
    REFERENCE_TIME rtTimestamp;
    REFERENCE_TIME rtTimelength;
    }GMO_OUTPUT_DATA_BUFFER;

typedef struct _GMO_OUTPUT_DATA_BUFFER *PGMO_OUTPUT_DATA_BUFFER;

enum _GMO_INPUT_DATA_BUFFER_FLAGS
    {	GMO_INPUT_DATA_BUFFERF_SYNCPOINT	= 0x1,
	GMO_INPUT_DATA_BUFFERF_TIME	= 0x2,
	GMO_INPUT_DATA_BUFFERF_TIMELENGTH	= 0x4
    } ;

enum _GMO_OUTPUT_DATA_BUFFER_FLAGS
    {	GMO_OUTPUT_DATA_BUFFERF_SYNCPOINT	= 0x1,
	GMO_OUTPUT_DATA_BUFFERF_TIME	= 0x2,
	GMO_OUTPUT_DATA_BUFFERF_TIMELENGTH	= 0x4,
	GMO_OUTPUT_DATA_BUFFERF_INCOMPLETE	= 0x1000000
    } ;

enum _GMO_INPUT_STATUS_FLAGS
    {	GMO_INPUT_STATUSF_ACCEPT_DATA	= 0x1
    } ;

enum _GMO_INPUT_STREAM_INFO_FLAGS
    {	GMO_INPUT_STREAMF_WHOLE_SAMPLES	= 0x1,
	GMO_INPUT_STREAMF_SINGLE_SAMPLE_PER_BUFFER	= 0x2,
	GMO_INPUT_STREAMF_FIXED_SAMPLE_SIZE	= 0x4,
	GMO_INPUT_STREAMF_HOLDS_BUFFERS	= 0x8
    } ;

enum _GMO_OUTPUT_STREAM_INFO_FLAGS
    {	GMO_OUTPUT_STREAMF_WHOLE_SAMPLES	= 0x1,
	GMO_OUTPUT_STREAMF_SINGLE_SAMPLE_PER_BUFFER	= 0x2,
	GMO_OUTPUT_STREAMF_FIXED_SAMPLE_SIZE	= 0x4,
	GMO_OUTPUT_STREAMF_DISCARDABLE	= 0x8,
	GMO_OUTPUT_STREAMF_OPTIONAL	= 0x10
    } ;

enum _GMO_SET_TYPE_FLAGS
    {	GMO_SET_TYPEF_TEST_ONLY	= 0x1,
	GMO_SET_TYPEF_CLEAR	= 0x2
    } ;

enum _GMO_PROCESS_OUTPUT_FLAGS
    {	GMO_PROCESS_OUTPUT_DISCARD_WHEN_NO_BUFFER	= 0x1,
		GMO_PROCESS_OUTPUT_DRAIN_WHEN_NO_INPUT		= 0x2	//often happen on last frame
    } ;

//GMO error codes
#define GMO_E_INVALIDSTREAMINDEX 0x80040201
#define GMO_E_INVALIDTYPE        0x80040202
#define GMO_E_TYPE_NOT_SET       0x80040203
#define GMO_E_NOTACCEPTING       0x80040204
#define GMO_E_TYPE_NOT_ACCEPTED  0x80040205
#define GMO_E_NO_MORE_ITEMS      0x80040206
   
//Generic Media Buffer
	class IGMediaBuffer : public IUnknown
    {
	public:
        virtual HRESULT STDMETHODCALLTYPE SetLength( 
            DWORD cbLength) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMaxLength( 
            /* [out] */ DWORD *pcbMaxLength) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetBufferAndLength( 
            /* [out] */ BYTE **ppBuffer,
            /* [out] */ DWORD *pcbLength) = 0;
        
    };

static const IID IID_IGMediaObject = 
{ 0xefb749cf, 0x2434, 0x4480, { 0x80, 0xe, 0xdb, 0xcc, 0x80, 0x6d, 0xfe, 0xf9 } };


//Generic Media Object
	class IGMediaObject : public IUnknown
    {
	public:
        virtual HRESULT STDMETHODCALLTYPE GetStreamCount( 
            /* [out] */ DWORD *pcInputStreams,
            /* [out] */ DWORD *pcOutputStreams) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetInputStreamInfo( 
            DWORD dwInputStreamIndex,
            /* [out] */ DWORD *pdwFlags)  = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetOutputStreamInfo( 
            DWORD dwOutputStreamIndex,
            /* [out] */ DWORD *pdwFlags)  = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetInputType( 
            DWORD dwInputStreamIndex,
            DWORD dwTypeIndex,
            /* [out] */ GMO_MEDIA_TYPE *pmt)  = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetOutputType( 
            DWORD dwOutputStreamIndex,
            DWORD dwTypeIndex,
            /* [out] */ GMO_MEDIA_TYPE *pmt)  = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetInputType( 
            DWORD dwInputStreamIndex,
            /* [in] */ const GMO_MEDIA_TYPE *pmt,
            DWORD dwFlags)  = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetOutputType( 
            DWORD dwOutputStreamIndex,
            /* [in] */ const GMO_MEDIA_TYPE *pmt,
            DWORD dwFlags)  = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetInputCurrentType( 
            DWORD dwInputStreamIndex,
            /* [out] */ GMO_MEDIA_TYPE *pmt)  = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetOutputCurrentType( 
            DWORD dwOutputStreamIndex,
            /* [out] */ GMO_MEDIA_TYPE *pmt)  = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetInputSizeInfo( 
            DWORD dwInputStreamIndex,
            /* [out] */ DWORD *pcbSize,
            /* [out] */ DWORD *pcbMaxLookahead,
            /* [out] */ DWORD *pcbAlignment)  = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetOutputSizeInfo( 
            DWORD dwOutputStreamIndex,
            /* [out] */ DWORD *pcbSize,
            /* [out] */ DWORD *pcbAlignment)  = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetInputMaxLatency( 
            DWORD dwInputStreamIndex,
            /* [out] */ REFERENCE_TIME *prtMaxLatency) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetInputMaxLatency( 
            DWORD dwInputStreamIndex,
            REFERENCE_TIME rtMaxLatency)  = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Flush( void)  = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Discontinuity( 
            DWORD dwInputStreamIndex) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AllocateStreamingResources( void)  = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FreeStreamingResources( void)  = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetInputStatus( 
            DWORD dwInputStreamIndex,
            /* [out] */ DWORD *dwFlags)  = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ProcessInput( 
            DWORD dwInputStreamIndex,
            IGMediaBuffer *pBuffer,
            DWORD dwFlags,
            REFERENCE_TIME rtTimestamp,
            REFERENCE_TIME rtTimelength) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ProcessOutput( 
            DWORD dwFlags,
            DWORD cOutputBufferCount,
            /* [size_is][out][in] */ GMO_OUTPUT_DATA_BUFFER *pOutputBuffers,
            /* [out] */ DWORD *pdwStatus)  = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Lock( 
            LONG bLock)  = 0;
        
    };


#endif
