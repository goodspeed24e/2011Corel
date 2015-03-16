#ifndef __GMEDIAOBJECTIMPL_H
#define __GMEDIAOBJECTIMPL_H

#include <crtdbg.h>

#include "GMO_i.h"
#include "GMOHelper.h"

#define INTERNAL_CALL(_T_, _X_) \
    static_cast<_T_ *>(this)->Internal##_X_

//Generic Media Object Implement
template <class _DERIVED_, int NUMBEROFINPUTS, int NUMBEROFOUTPUTS>
class IGMediaObjectImpl: public IGMediaObject{
private:
    // Member variables
    struct {
        DWORD   fTypeSet:1;
        DWORD   fIncomplete:1;
        GMO_MEDIA_TYPE CurrentMediaType;
    } m_InputInfo[NUMBEROFINPUTS], m_OutputInfo[NUMBEROFOUTPUTS];

    bool m_fTypesSet;
    bool m_fFlushed;
    bool m_fResourcesAllocated;

protected:

    //  Helpers
    bool InputTypeSet(DWORD ulInputStreamIndex) const
    {
        _ASSERTE(ulInputStreamIndex < NUMBEROFINPUTS);
        return 0 != m_InputInfo[ulInputStreamIndex].fTypeSet;
    }

    bool OutputTypeSet(DWORD ulOutputStreamIndex) const
    {
        _ASSERTE(ulOutputStreamIndex < NUMBEROFOUTPUTS);
        return 0 != m_OutputInfo[ulOutputStreamIndex].fTypeSet;
    }
    const GMO_MEDIA_TYPE *InputType(DWORD ulInputStreamIndex)
    {
        if (!InputTypeSet(ulInputStreamIndex)) {
            return NULL;
        }
        return &m_InputInfo[ulInputStreamIndex].CurrentMediaType;
    }
    const GMO_MEDIA_TYPE *OutputType(DWORD ulOutputStreamIndex)
    {
        if (!OutputTypeSet(ulOutputStreamIndex)) {
            return NULL;
        }
        return &m_OutputInfo[ulOutputStreamIndex].CurrentMediaType;
    }


    class LockIt
    {
    public:
        LockIt(_DERIVED_ *p) : m_p(p)
        {
            static_cast<_DERIVED_ *>(m_p)->Lock();
        }
        ~LockIt()
        {
            static_cast<_DERIVED_ *>(m_p)->Unlock();
        }
        _DERIVED_ *const m_p;
    };

    bool CheckTypesSet()
    {
        m_fTypesSet = false;
        DWORD dw;
        for (dw = 0; dw < NUMBEROFINPUTS; dw++) {
            if (!InputTypeSet(dw)) {
                return false;
            }
        }
        for (dw = 0; dw < NUMBEROFOUTPUTS; dw++) {
            if (!OutputTypeSet(dw)) {
                //  Check if it's optional
                DWORD dwFlags;
#ifdef _DEBUG
                dwFlags = 0xFFFFFFFF;
#endif
                INTERNAL_CALL(_DERIVED_, GetOutputStreamInfo)(dw, &dwFlags);
                _ASSERTE(0 == (dwFlags & ~(GMO_OUTPUT_STREAMF_WHOLE_SAMPLES |
                                         GMO_OUTPUT_STREAMF_SINGLE_SAMPLE_PER_BUFFER |
                                         GMO_OUTPUT_STREAMF_FIXED_SAMPLE_SIZE |
                                         GMO_OUTPUT_STREAMF_DISCARDABLE |
                                         GMO_OUTPUT_STREAMF_OPTIONAL)));
                if (!(dwFlags & GMO_OUTPUT_STREAMF_OPTIONAL)) {
                    return false;
                }
            }
        }
        m_fTypesSet = true;
        return true;
    }


    IGMediaObjectImpl() :
        m_fTypesSet(false),
        m_fFlushed(true),
        m_fResourcesAllocated(false)
    {
        ZeroMemory(&m_InputInfo, sizeof(m_InputInfo));
        ZeroMemory(&m_OutputInfo, sizeof(m_OutputInfo));
    }

    virtual ~IGMediaObjectImpl() {
        DWORD dwCurrentType;

        for (dwCurrentType = 0; dwCurrentType < NUMBEROFINPUTS; dwCurrentType++) {
            if(InputTypeSet(dwCurrentType)) {
                GMediaObjectHelper::MoFreeMediaType(&m_InputInfo[dwCurrentType].CurrentMediaType);
            }
        }

        for (dwCurrentType = 0; dwCurrentType < NUMBEROFOUTPUTS; dwCurrentType++) {
            if(OutputTypeSet(dwCurrentType)) {
                GMediaObjectHelper::MoFreeMediaType(&m_OutputInfo[dwCurrentType].CurrentMediaType);
            }
        }
    }


    // IMediaObject methods


    //
    // IMediaObject methods
    //
    STDMETHODIMP GetStreamCount(unsigned long *pulNumberOfInputStreams, unsigned long *pulNumberOfOutputStreams)
    {
        LockIt lck(static_cast<_DERIVED_ *>(this));
        if (pulNumberOfInputStreams == NULL ||
            pulNumberOfOutputStreams == NULL) {
            return E_POINTER;
        }
        *pulNumberOfInputStreams  = NUMBEROFINPUTS;
        *pulNumberOfOutputStreams = NUMBEROFOUTPUTS;
        return S_OK;
    }

    STDMETHODIMP GetInputStreamInfo(ULONG ulStreamIndex, DWORD *pdwFlags)
    {
        LockIt lck(static_cast<_DERIVED_ *>(this));
        if (ulStreamIndex >= NUMBEROFINPUTS) {
            return GMO_E_INVALIDSTREAMINDEX;
        }
        if (pdwFlags == NULL) {
            return E_POINTER;
        }
        HRESULT hr = INTERNAL_CALL(_DERIVED_, GetInputStreamInfo)(ulStreamIndex, pdwFlags);
        _ASSERTE(0 == (*pdwFlags & ~(GMO_INPUT_STREAMF_WHOLE_SAMPLES |
                                   GMO_INPUT_STREAMF_SINGLE_SAMPLE_PER_BUFFER |
                                   GMO_INPUT_STREAMF_FIXED_SAMPLE_SIZE |
                                   GMO_INPUT_STREAMF_HOLDS_BUFFERS)));
        return hr;
    }

    STDMETHODIMP GetOutputStreamInfo(ULONG ulStreamIndex, DWORD *pdwFlags)
    {
        LockIt lck(static_cast<_DERIVED_ *>(this));
        if (ulStreamIndex >= NUMBEROFOUTPUTS) {
            return GMO_E_INVALIDSTREAMINDEX;
        }
        if (pdwFlags == NULL) {
            return E_POINTER;
        }
        HRESULT hr = INTERNAL_CALL(_DERIVED_, GetOutputStreamInfo)(ulStreamIndex, pdwFlags);
        _ASSERTE(0 == (*pdwFlags & ~(GMO_OUTPUT_STREAMF_WHOLE_SAMPLES |
                                   GMO_OUTPUT_STREAMF_SINGLE_SAMPLE_PER_BUFFER |
                                   GMO_OUTPUT_STREAMF_FIXED_SAMPLE_SIZE |
                                   GMO_OUTPUT_STREAMF_DISCARDABLE |
                                   GMO_OUTPUT_STREAMF_OPTIONAL)));
        return hr;
    }

    STDMETHODIMP GetInputType(ULONG ulStreamIndex, ULONG ulTypeIndex, GMO_MEDIA_TYPE *pmt) {
        if (ulStreamIndex >= NUMBEROFINPUTS) {
            return GMO_E_INVALIDSTREAMINDEX;
        }
        LockIt lck(static_cast<_DERIVED_ *>(this));
        return INTERNAL_CALL(_DERIVED_, GetInputType)(ulStreamIndex, ulTypeIndex, pmt);
    }

    STDMETHODIMP GetOutputType(ULONG ulStreamIndex, ULONG ulTypeIndex, GMO_MEDIA_TYPE *pmt) {
        if (ulStreamIndex >= NUMBEROFOUTPUTS) {
            return GMO_E_INVALIDSTREAMINDEX;
        }
        LockIt lck(static_cast<_DERIVED_ *>(this));
        return INTERNAL_CALL(_DERIVED_, GetOutputType)(ulStreamIndex, ulTypeIndex, pmt);
    }

    STDMETHODIMP GetInputCurrentType(ULONG ulStreamIndex, GMO_MEDIA_TYPE *pmt) {
        if (ulStreamIndex >= NUMBEROFINPUTS) {
            return GMO_E_INVALIDSTREAMINDEX;
        }
        if (NULL == pmt) {
            return E_POINTER;
        }
        LockIt lck(static_cast<_DERIVED_ *>(this));
        if (InputTypeSet(ulStreamIndex))
            return GMediaObjectHelper::MoCopyMediaType(pmt,
                                   &m_InputInfo[ulStreamIndex].CurrentMediaType);
        else
           return GMO_E_TYPE_NOT_SET;
    }

    STDMETHODIMP GetOutputCurrentType(ULONG ulStreamIndex, GMO_MEDIA_TYPE *pmt) {
        if (ulStreamIndex >= NUMBEROFOUTPUTS) {
            return GMO_E_INVALIDSTREAMINDEX;
        }
        if (NULL == pmt) {
            return E_POINTER;
        }
        LockIt lck(static_cast<_DERIVED_ *>(this));
        if (OutputTypeSet(ulStreamIndex))
            return GMediaObjectHelper::MoCopyMediaType(pmt,
                                   &m_OutputInfo[ulStreamIndex].CurrentMediaType);
        else
           return GMO_E_TYPE_NOT_SET;
    }

    STDMETHODIMP GetInputSizeInfo(ULONG ulStreamIndex, ULONG *pulSize, ULONG *pcbMaxLookahead, ULONG *pulAlignment) {
        if (ulStreamIndex >= NUMBEROFINPUTS) {
            return GMO_E_INVALIDSTREAMINDEX;
        }
        if (NULL == pulSize || NULL == pulAlignment ||
            NULL == pcbMaxLookahead) {
            return E_POINTER;
        }
        LockIt lck(static_cast<_DERIVED_ *>(this));
        if (!InputTypeSet(ulStreamIndex)) {
           return GMO_E_TYPE_NOT_SET;
        }
        return INTERNAL_CALL(_DERIVED_, GetInputSizeInfo)(ulStreamIndex, pulSize, pcbMaxLookahead, pulAlignment);
    }

    STDMETHODIMP GetOutputSizeInfo(ULONG ulStreamIndex, ULONG *pulSize, ULONG *pulAlignment) {
        if (ulStreamIndex >= NUMBEROFOUTPUTS) {
            return GMO_E_INVALIDSTREAMINDEX;
        }
        if (NULL == pulSize || NULL == pulAlignment) {
            return E_POINTER;
        }
        LockIt lck(static_cast<_DERIVED_ *>(this));
        if (!m_fTypesSet || !OutputTypeSet(ulStreamIndex)) {
           return GMO_E_TYPE_NOT_SET;
        }
        return INTERNAL_CALL(_DERIVED_, GetOutputSizeInfo)(ulStreamIndex, pulSize, pulAlignment);
    }

    STDMETHODIMP SetInputType(ULONG ulStreamIndex, const GMO_MEDIA_TYPE *pmt, DWORD dwFlags) {
        if (ulStreamIndex >= NUMBEROFINPUTS) {
            return GMO_E_INVALIDSTREAMINDEX;
        }
        if (dwFlags & ~ (GMO_SET_TYPEF_CLEAR | GMO_SET_TYPEF_TEST_ONLY)) {
            return E_INVALIDARG;
        }

        LockIt lck(static_cast<_DERIVED_ *>(this));

        if (dwFlags & GMO_SET_TYPEF_CLEAR) {
            GMediaObjectHelper::MoFreeMediaType(&m_InputInfo[ulStreamIndex].CurrentMediaType);
            m_InputInfo[ulStreamIndex].fTypeSet = FALSE;
            if (!CheckTypesSet()) {
                Flush();
                FreeStreamingResources();
            }
            return NOERROR;
        }
        if (NULL == pmt) {
            return E_POINTER;
        }
        HRESULT hr = INTERNAL_CALL(_DERIVED_, CheckInputType)(ulStreamIndex, pmt);
        if (FAILED(hr))
           return hr;

        if (dwFlags & GMO_SET_TYPEF_TEST_ONLY) {
           return NOERROR;
        }


        // actually set the type
        GMO_MEDIA_TYPE mtTemp;
        if (S_OK == GMediaObjectHelper::MoCopyMediaType(&mtTemp, pmt)) {
            // Free any previous mediatype
            if (InputTypeSet(ulStreamIndex)) {
                GMediaObjectHelper::MoFreeMediaType(&m_InputInfo[ulStreamIndex].CurrentMediaType);
            }
            m_InputInfo[ulStreamIndex].CurrentMediaType = mtTemp;
            m_InputInfo[ulStreamIndex].fTypeSet = TRUE;
            CheckTypesSet();
        } else {
            return E_OUTOFMEMORY;
        }

        return NOERROR;
    }

    STDMETHODIMP SetOutputType(ULONG ulStreamIndex, const GMO_MEDIA_TYPE *pmt, DWORD dwFlags) {
        if (ulStreamIndex >= NUMBEROFOUTPUTS) {
            return GMO_E_INVALIDSTREAMINDEX;
        }
        if (dwFlags & ~ (GMO_SET_TYPEF_CLEAR | GMO_SET_TYPEF_TEST_ONLY)) {
            return E_INVALIDARG;
        }

        LockIt lck(static_cast<_DERIVED_ *>(this));

        if (dwFlags & GMO_SET_TYPEF_CLEAR) {
            GMediaObjectHelper::MoFreeMediaType(&m_OutputInfo[ulStreamIndex].CurrentMediaType);
            m_OutputInfo[ulStreamIndex].fTypeSet = FALSE;
            if (!CheckTypesSet()) {
                Flush();
                FreeStreamingResources();
            }
            return NOERROR;
        }
        if (NULL == pmt) {
            return E_POINTER;
        }
        HRESULT hr = INTERNAL_CALL(_DERIVED_, CheckOutputType)(ulStreamIndex, pmt);
        if (FAILED(hr)) {
           return hr;
        }

        if (dwFlags & GMO_SET_TYPEF_TEST_ONLY) {
           return NOERROR;
        }


        // actually set the type
        GMO_MEDIA_TYPE mtTemp;
        if (S_OK == GMediaObjectHelper::MoCopyMediaType(&mtTemp, pmt)) {
            // Free any previous mediatype
            if (OutputTypeSet(ulStreamIndex)) {
                GMediaObjectHelper::MoFreeMediaType(&m_OutputInfo[ulStreamIndex].CurrentMediaType);
            }
            m_OutputInfo[ulStreamIndex].CurrentMediaType = mtTemp;
            m_OutputInfo[ulStreamIndex].fTypeSet = TRUE;
            CheckTypesSet();
        } else {
            return E_OUTOFMEMORY;
        }

        return NOERROR;
    }

    STDMETHODIMP GetInputStatus(
        ULONG ulStreamIndex,
        DWORD *pdwStatus
    ) {
        if (ulStreamIndex >= NUMBEROFINPUTS) {
            return GMO_E_INVALIDSTREAMINDEX;
        }
        if (NULL == pdwStatus) {
            return E_POINTER;
        }
        *pdwStatus = 0;

        LockIt lck(static_cast<_DERIVED_ *>(this));

        if (!m_fTypesSet) {
            return GMO_E_TYPE_NOT_SET;
        }

        if (INTERNAL_CALL(_DERIVED_, AcceptingInput)(ulStreamIndex) == S_OK) {
           *pdwStatus |= GMO_INPUT_STATUSF_ACCEPT_DATA;
        }
        return NOERROR;
    }

    STDMETHODIMP GetInputMaxLatency(unsigned long ulStreamIndex, REFERENCE_TIME *prtLatency) {

        if (prtLatency == NULL) {
            return E_POINTER;
        }
        if (ulStreamIndex >= NUMBEROFINPUTS) {
            return GMO_E_INVALIDSTREAMINDEX;
        }

        LockIt lck(static_cast<_DERIVED_ *>(this));

        return INTERNAL_CALL(_DERIVED_, GetInputMaxLatency)(ulStreamIndex, prtLatency);
    }

    STDMETHODIMP SetInputMaxLatency(unsigned long ulStreamIndex, REFERENCE_TIME rtLatency) {
        if (ulStreamIndex >= NUMBEROFINPUTS) {
            return GMO_E_INVALIDSTREAMINDEX;
        }

        LockIt lck(static_cast<_DERIVED_ *>(this));

        return INTERNAL_CALL(_DERIVED_, SetInputMaxLatency)(ulStreamIndex, rtLatency);
    }

    STDMETHODIMP Discontinuity(ULONG ulStreamIndex) {
        if (ulStreamIndex >= NUMBEROFINPUTS) {
            return GMO_E_INVALIDSTREAMINDEX;
        }

        LockIt lck(static_cast<_DERIVED_ *>(this));

        if (!m_fTypesSet) {
            return GMO_E_TYPE_NOT_SET;
        }

        if (S_OK != INTERNAL_CALL(_DERIVED_, AcceptingInput)(ulStreamIndex)) {
            return GMO_E_NOTACCEPTING;
        }

        return INTERNAL_CALL(_DERIVED_, Discontinuity)(ulStreamIndex);
    }

    STDMETHODIMP Flush()
    {
        LockIt lck(static_cast<_DERIVED_ *>(this));

        if (!m_fTypesSet) {
            return S_OK;
        }
        if (m_fFlushed) {
            return S_OK;
        }
        HRESULT hr =  INTERNAL_CALL(_DERIVED_, Flush)();
        m_fFlushed = true;
        return hr;
    }

    STDMETHODIMP AllocateStreamingResources() {
        LockIt lck(static_cast<_DERIVED_ *>(this));
        if (!m_fTypesSet) {
            return GMO_E_TYPE_NOT_SET;
        }
        if (m_fResourcesAllocated) {
            return S_OK;
        }
        HRESULT hr = INTERNAL_CALL(_DERIVED_, AllocateStreamingResources)();
        if (SUCCEEDED(hr)) {
            m_fResourcesAllocated = true;
        }
        return hr;
    }

    STDMETHODIMP FreeStreamingResources()
    {
        LockIt lck(static_cast<_DERIVED_ *>(this));
        if (m_fResourcesAllocated) {
            m_fResourcesAllocated = false;
            INTERNAL_CALL(_DERIVED_, Flush)();
            return INTERNAL_CALL(_DERIVED_, FreeStreamingResources)();
        }
        return S_OK;
    }

    //
    // Processing methods - public entry points
    //
    STDMETHODIMP ProcessInput(
        DWORD ulStreamIndex,
        IGMediaBuffer *pBuffer, // [in], must not be NULL
        DWORD dwFlags, // [in] - discontinuity, timestamp, etc.
        REFERENCE_TIME rtTimestamp, // [in], valid if flag set
        REFERENCE_TIME rtTimelength // [in], valid if flag set
    ) {
        if (!pBuffer) {
            return E_POINTER;
        }
        if (ulStreamIndex >= NUMBEROFINPUTS) {
            return GMO_E_INVALIDSTREAMINDEX;
        }
        if (dwFlags & ~(GMO_INPUT_DATA_BUFFERF_SYNCPOINT |
                        GMO_INPUT_DATA_BUFFERF_TIME |
                        GMO_INPUT_DATA_BUFFERF_TIMELENGTH)) {
            return E_INVALIDARG;
        }

        LockIt lck(static_cast<_DERIVED_ *>(this));

        //  Make sure all streams have media types set and resources are allocated
        HRESULT hr = AllocateStreamingResources();
        if (FAILED(hr)) {
            return hr;
        }
        if (INTERNAL_CALL(_DERIVED_, AcceptingInput)(ulStreamIndex) != S_OK) {
            return GMO_E_NOTACCEPTING;
        }

        m_fFlushed = false;

        return INTERNAL_CALL(_DERIVED_, ProcessInput)(
                                    ulStreamIndex,
                                    pBuffer,
                                    dwFlags,
                                    rtTimestamp,
                                    rtTimelength);
    }

    STDMETHODIMP ProcessOutput(
                    DWORD dwFlags,
                    DWORD ulOutputBufferCount,
                    GMO_OUTPUT_DATA_BUFFER *pOutputBuffers,
                    DWORD *pdwStatus)
    {
        if (pdwStatus == NULL) {
            return E_POINTER;
        }

		DWORD dw;

        if (ulOutputBufferCount > NUMBEROFOUTPUTS || (dwFlags & ~GMO_PROCESS_OUTPUT_DISCARD_WHEN_NO_BUFFER && dwFlags & ~GMO_PROCESS_OUTPUT_DRAIN_WHEN_NO_INPUT)) {
           return E_INVALIDARG;
        }

        if (NUMBEROFOUTPUTS != 0 && pOutputBuffers == NULL) {
            return E_POINTER;
        }

        *pdwStatus = 0;

        LockIt lck(static_cast<_DERIVED_ *>(this));

        HRESULT hr = AllocateStreamingResources();
        if (FAILED(hr)) {
            return hr;
        }

        for (dw = 0; dw < NUMBEROFOUTPUTS; dw++) {
            pOutputBuffers[dw].dwStatus = 0;
        }

        hr = INTERNAL_CALL(_DERIVED_, ProcessOutput)(
                           dwFlags,
                           ulOutputBufferCount,
                           pOutputBuffers,
                           pdwStatus);

        // remember the GMO's incomplete status
        for (dw = 0; dw < NUMBEROFOUTPUTS; dw++) {
            if (pOutputBuffers[dw].dwStatus & GMO_OUTPUT_DATA_BUFFERF_INCOMPLETE) {
                m_OutputInfo[dw].fIncomplete = TRUE;
            } else {
                m_OutputInfo[dw].fIncomplete = FALSE;
            }
        }

        return hr;
    }

    STDMETHODIMP Lock(LONG lLock)
    {
        if (lLock) {
            static_cast<_DERIVED_ *>(this)->Lock();
        } else {
            static_cast<_DERIVED_ *>(this)->Unlock();
        }
        return S_OK;
    }
	
};
#endif //__GMEDIAOBJECTIMPL_H
