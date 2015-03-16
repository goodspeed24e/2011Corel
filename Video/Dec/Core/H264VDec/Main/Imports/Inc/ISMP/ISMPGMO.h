#ifndef _CISMPGMO_H
#define _CISMPGMO_H

//Before using ISMPGMO.h, you need to include GMO_i.h and GMOBase.h.
#include <windows.h>
#include "ISMPGMOBridge.h"
#include "ISMPGMOGUID.h"

template<typename T>
class CISMPGMO : public IGMediaObject, public ISMPGMO, public CGMOUnknown
{
public:
	IMPLEMENT_MAKE(CISMPGMO);
	IMPLEMENT_CREATEINSTANCE(CISMPGMO, NULL)
	DECLARE_IUNKNOWN
public:
	CISMPGMO(IUnknown* pUnkOuter= NULL):CGMOUnknown(pUnkOuter),m_pGMO(0),m_pISMP(new ISMPGMOBridge()),m_nEnable(0),m_nSize(0),m_nOffset(0)
	{
		T::CreateInstance(&m_pGMO, reinterpret_cast<IUnknown*>(this));
		m_pGMO->NonDelegatingQueryInterface(IID_IGMediaObject, (void**)&m_pIGMediaObject);
		m_cRef= 0;	//m_pGMO life cycle is controlled by CISMPGMO
	}
	virtual ~CISMPGMO(){
		//release m_pGMO via original object destroy method
		m_pGMO->NonDelegatingAddRef();
		m_pGMO->NonDelegatingRelease();
		if(m_pISMP)
		{
			delete m_pISMP;
			m_pISMP=NULL;
		}
	}
	virtual STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv)
	{
		CheckPointer(ppv, E_POINTER);
		if(riid == IID_ISMPGMO)
		{
			return GetInterface((IUnknown*)(ISMPGMO*) this, ppv);
		}
		else if(riid == IID_IGMediaObject)
		{
			return GetInterface((IUnknown*)(IGMediaObject*) this, ppv);
		}
		return m_pGMO->NonDelegatingQueryInterface(riid, ppv);
	}
//IISMPSimpleEncryptor 
public:
	STDMETHODIMP SPConnect(void *pData)
	{
		return (m_pISMP->SPConnect( pData, SPCONNECTION_MAGIC_CODE));
	}
public:
	HRESULT STDMETHODCALLTYPE GetStreamCount( 
		/* [out] */ DWORD *pcInputStreams,
		/* [out] */ DWORD *pcOutputStreams)
	{
		return m_pIGMediaObject->GetStreamCount(pcInputStreams, pcOutputStreams);
	}
	HRESULT STDMETHODCALLTYPE GetInputStreamInfo( 
		DWORD dwInputStreamIndex,
		/* [out] */ DWORD *pdwFlags)
	{
		return m_pIGMediaObject->GetInputStreamInfo(dwInputStreamIndex, pdwFlags);
	}
	HRESULT STDMETHODCALLTYPE GetOutputStreamInfo( 
		DWORD dwOutputStreamIndex,
		/* [out] */ DWORD *pdwFlags)
	{
		return m_pIGMediaObject->GetOutputStreamInfo(dwOutputStreamIndex, pdwFlags);
	}

	HRESULT STDMETHODCALLTYPE GetInputType( 
		DWORD dwInputStreamIndex,
		DWORD dwTypeIndex,
		/* [out] */ GMO_MEDIA_TYPE *pmt)
	{
		return m_pIGMediaObject->GetInputType(dwInputStreamIndex, dwTypeIndex, pmt);
	}
	HRESULT STDMETHODCALLTYPE GetOutputType( 
		DWORD dwOutputStreamIndex,
		DWORD dwTypeIndex,
		/* [out] */ GMO_MEDIA_TYPE *pmt)
	{
		return m_pIGMediaObject->GetOutputType(dwOutputStreamIndex, dwTypeIndex, pmt);
	}
	HRESULT STDMETHODCALLTYPE SetInputType( 
		DWORD dwInputStreamIndex,
		/* [in] */ const GMO_MEDIA_TYPE *pmt,
		DWORD dwFlags)
	{
		return m_pIGMediaObject->SetInputType(dwInputStreamIndex, pmt, dwFlags);
	}
	HRESULT STDMETHODCALLTYPE SetOutputType( 
		DWORD dwOutputStreamIndex,
		/* [in] */ const GMO_MEDIA_TYPE *pmt,
		DWORD dwFlags) 
	{
		return m_pIGMediaObject->SetOutputType(dwOutputStreamIndex, pmt, dwFlags);
	}
	HRESULT STDMETHODCALLTYPE GetInputCurrentType( 
		DWORD dwInputStreamIndex,
		/* [out] */ GMO_MEDIA_TYPE *pmt)
	{
		return m_pIGMediaObject->GetInputCurrentType(dwInputStreamIndex, pmt);
	}
	HRESULT STDMETHODCALLTYPE GetOutputCurrentType( 
		DWORD dwOutputStreamIndex,
		/* [out] */ GMO_MEDIA_TYPE *pmt)
	{
		return m_pIGMediaObject->GetOutputCurrentType(dwOutputStreamIndex, pmt);
	}
	HRESULT STDMETHODCALLTYPE GetInputSizeInfo( 
		DWORD dwInputStreamIndex,
		/* [out] */ DWORD *pcbSize,
		/* [out] */ DWORD *pcbMaxLookahead,
		/* [out] */ DWORD *pcbAlignment)
	{
		return m_pIGMediaObject->GetInputSizeInfo(dwInputStreamIndex, pcbSize, pcbMaxLookahead, pcbAlignment);
	}
	HRESULT STDMETHODCALLTYPE GetOutputSizeInfo( 
		DWORD dwOutputStreamIndex,
		/* [out] */ DWORD *pcbSize,
		/* [out] */ DWORD *pcbAlignment)
	{
		return m_pIGMediaObject->GetOutputSizeInfo(dwOutputStreamIndex, pcbSize, pcbAlignment);
	}
	HRESULT STDMETHODCALLTYPE GetInputMaxLatency( 
		DWORD dwInputStreamIndex,
		/* [out] */ REFERENCE_TIME *prtMaxLatency)
	{
		return m_pIGMediaObject->GetInputMaxLatency(dwInputStreamIndex, prtMaxLatency);
	}
	HRESULT STDMETHODCALLTYPE SetInputMaxLatency( 
		DWORD dwInputStreamIndex,
		REFERENCE_TIME rtMaxLatency)
	{
		return m_pIGMediaObject->SetInputMaxLatency(dwInputStreamIndex, rtMaxLatency);
	}
	HRESULT STDMETHODCALLTYPE Flush(void) 
	{
		return m_pIGMediaObject->Flush();
	}
	HRESULT STDMETHODCALLTYPE Discontinuity( 
		DWORD dwInputStreamIndex)
	{
		return m_pIGMediaObject->Discontinuity(dwInputStreamIndex);
	}
	HRESULT STDMETHODCALLTYPE AllocateStreamingResources( void)
	{
		return m_pIGMediaObject->AllocateStreamingResources();
	}
	HRESULT STDMETHODCALLTYPE FreeStreamingResources( void)
	{
		return m_pIGMediaObject->FreeStreamingResources();
	}
	HRESULT STDMETHODCALLTYPE GetInputStatus( 
		DWORD dwInputStreamIndex,
		/* [out] */ DWORD *dwFlags)
	{
		return m_pIGMediaObject->GetInputStatus(dwInputStreamIndex, dwFlags);
	}
	HRESULT STDMETHODCALLTYPE ProcessInput( 
		DWORD dwInputStreamIndex,
		IGMediaBuffer *pBuffer,
		DWORD dwFlags,
		REFERENCE_TIME rtTimestamp,
		REFERENCE_TIME rtTimelength)
	{
		m_pISMP->DescrambleGMediaBuffer(pBuffer, &m_nEnable, &m_nSize, &m_nOffset);
		HRESULT hr= m_pIGMediaObject->ProcessInput(dwInputStreamIndex, pBuffer, dwFlags, rtTimestamp, rtTimelength);
		return hr;
	}
	HRESULT STDMETHODCALLTYPE ProcessOutput( 
		DWORD dwFlags,
		DWORD cOutputBufferCount,
		/* [size_is][out][in] */ GMO_OUTPUT_DATA_BUFFER *pOutputBuffers,
		/* [out] */ DWORD *pdwStatus)
	{
		HRESULT hr= m_pIGMediaObject->ProcessOutput(dwFlags, cOutputBufferCount, pOutputBuffers, pdwStatus);
		BYTE *pEncryptBuffer;
		DWORD cbLength;
		(pOutputBuffers->pBuffer)->GetBufferAndLength(&pEncryptBuffer, &cbLength);
		if(cbLength>0)
		{	
			if(m_nEnable > 0)
				m_pISMP->ScrambleGMediaBuffer(pOutputBuffers->pBuffer,ISMPGMO_DATA_ENCRYPT,m_nSize,m_nOffset);
			else
				m_pISMP->ScrambleGMediaBuffer(pOutputBuffers->pBuffer,ISMPGMO_DATA_NO_ENCRYPT,0,0);

			m_nEnable = m_nSize = m_nOffset = 0;				
		}
		return hr;
	}
	HRESULT STDMETHODCALLTYPE Lock( 
		LONG bLock)
	{
		return m_pIGMediaObject->Lock(bLock);
	}
private:
	IGMediaObject* m_pIGMediaObject;
	T* m_pGMO;

	//ISMP related member
	ISMPGMOBridge*  m_pISMP;
	unsigned int m_nEnable;
	unsigned int m_nSize;
	unsigned int m_nOffset;
};
#endif