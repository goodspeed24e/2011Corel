//=============================================================================
//	THIS SOURCE CODE IS PROPRIETARY INFORMATION BELONGING TO INTERVIDEO, INC.
// 	ANY USE INCLUDING BUT NOT LIMITED TO COPYING OF CODE, CONCEPTS, AND/OR
//	ALGORITHMS IS PROHIBITED EXCEPT WITH EXPRESS WRITTEN PERMISSION BY THE 
//	COMPANY.
//
// 	THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
// 	KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// 	IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
// 	PURPOSE.
//
// 	Copyright (c) 2007 InterVideo Corporation.  All Rights Reserved.
//
//-----------------------------------------------------------------------------

#ifndef _VDO_CODEC_GMO_H_
#define _VDO_CODEC_GMO_H_

#include "GMO/GMOBase.h"
#include "GMO/GMOImpl.h"

class CVideoDropManager;

class CVdoCodecGMO: public IGMediaObjectImpl<CVdoCodecGMO, 1, 2>, 
	public CGMOUnknown
{
	friend class IGMediaObjectImpl<CVdoCodecGMO, 1, 2>;
	friend class LockIt;
	friend class CVdoCodecGMOAutoLock;
	friend class CGMOUnknown;
public:
	DECLARE_IUNKNOWN

protected:
	CVdoCodecGMO(IUnknown* pUnkOuter= NULL);
	virtual ~CVdoCodecGMO();

public:
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv)
	{
		CheckPointer(ppv, E_POINTER);
		if(riid == IID_IGMediaObject)
			return GetInterface((IGMediaObject*)this, ppv);
		else
			return CGMOUnknown::NonDelegatingQueryInterface(riid, ppv);
	}

protected:
	//IMethodImpl Methods
	virtual STDMETHODIMP InternalAllocateStreamingResources(void);
	virtual STDMETHODIMP InternalDiscontinuity(DWORD dwInputStreamIndex);
	virtual STDMETHODIMP InternalFlush(void);
	virtual STDMETHODIMP InternalFreeStreamingResources(void);
	virtual STDMETHODIMP InternalGetInputMaxLatency(DWORD dwInputStreamIndex, REFERENCE_TIME *prtMaxLatency);
	virtual STDMETHODIMP InternalSetInputMaxLatency(DWORD dwInputStreamIndex, REFERENCE_TIME rtMaxLatency);
	virtual STDMETHODIMP InternalGetInputSizeInfo(DWORD dwInputStreamIndex, DWORD *pcbSize, DWORD *pulSizeMaxLookahead, DWORD *pulSizeAlignment);
	virtual STDMETHODIMP InternalGetOutputSizeInfo(DWORD dwOutputStreamIndex, DWORD *pcbSize, DWORD *pulSizeAlignment);
	virtual STDMETHODIMP InternalGetInputStreamInfo(DWORD dwInputStreamIndex, DWORD *pdwFlags);
	virtual STDMETHODIMP InternalGetOutputStreamInfo(DWORD dwOutputStreamIndex, DWORD *pdwFlags);
	virtual STDMETHODIMP InternalGetInputType(DWORD dwInputStreamIndex, DWORD dwTypeIndex, GMO_MEDIA_TYPE *pmt);
	virtual STDMETHODIMP InternalGetOutputType(DWORD dwOutputStreamIndex, DWORD dwTypeIndex, GMO_MEDIA_TYPE *pmt);
	virtual STDMETHODIMP InternalProcessInput(DWORD dwInputStreamIndex, IGMediaBuffer *pBuffer, DWORD dwFlags, REFERENCE_TIME rtTimeStamp, REFERENCE_TIME rtTimeLength);
	virtual STDMETHODIMP InternalProcessOutput(DWORD dwFlags, DWORD OutputBufferCount, GMO_OUTPUT_DATA_BUFFER *pOutputBuffers, DWORD *pdwStatus);

	//IGMediaObjectImpl Required overides
	virtual STDMETHODIMP InternalAcceptingInput(DWORD dwInputStreamIndex) = 0;
	virtual STDMETHODIMP InternalCheckInputType(DWORD dwInputStreamIndex, const GMO_MEDIA_TYPE *pmt) = 0;
	virtual STDMETHODIMP InternalCheckOutputType(DWORD dwOutputStreamIndex, const GMO_MEDIA_TYPE *pmt) = 0;

	virtual void Lock(void){EnterCriticalSection(&m_cs);}
	virtual void Unlock(void){LeaveCriticalSection(&m_cs);}

	BYTE  *m_pbInputData; //Pointer to the input data buffer
	DWORD m_cbInputLength; //Length of the input data buffer
	CVideoDropManager* m_pVdoDropManager;

private:
	CRITICAL_SECTION m_cs;
};

class CVdoCodecGMOAutoLock
{
public:
	CVdoCodecGMOAutoLock(CVdoCodecGMO* pInGMO){ m_pGMO = pInGMO; Lock();}
	~CVdoCodecGMOAutoLock(){Unlock();}
protected:
	void Lock(){m_pGMO->Lock();}
	void Unlock(){m_pGMO->Unlock();}
private:
	CVdoCodecGMO *m_pGMO;
};

#endif //_VDO_CODEC_GMO_H_