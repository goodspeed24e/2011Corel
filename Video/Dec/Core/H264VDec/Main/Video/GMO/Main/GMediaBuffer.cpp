#include "GMediaBuffer.h"
#include <malloc.h>

#define GMediaBufAlignedAlloc(a, b)			_aligned_malloc(a, b)
#define GMediaBufAlignedFree(a)				_aligned_free(a)
#define GMediaBufAlignedReAlloc(a, b, c)		_aligned_realloc(a, b, c)

CGMediaBuffer::~CGMediaBuffer()
{
	if(m_pBuffer)
		GMediaBufAlignedFree(m_pBuffer);
}

HRESULT CGMediaBuffer::CreateBuffer(IGMediaBuffer** ppOutBuf, DWORD cbMaxLength, DWORD cbAlignment)
{
	CGMediaBuffer* pOutBuf= new CGMediaBuffer;
	if(!pOutBuf)
		return E_FAIL;
	if((cbAlignment & cbAlignment-1) != 0)
		cbAlignment = (cbAlignment & cbAlignment-1);
	//<Add more 32 bytes for ISMP information chunk
	pOutBuf->m_pBuffer= static_cast<BYTE*>(GMediaBufAlignedAlloc(cbMaxLength+32, cbAlignment));
	if(!pOutBuf->m_pBuffer)
	{
		delete pOutBuf;
		return E_OUTOFMEMORY;
	}
	
	pOutBuf->m_cbBufferSize= 0;
	pOutBuf->m_cbBufferCapacity= cbMaxLength;
	pOutBuf->AddRef();
	*ppOutBuf= pOutBuf;
	return S_OK;
}

STDMETHODIMP CGMediaBuffer::SetLength(DWORD cbLength)
{
	if(cbLength > m_cbBufferCapacity)
		return E_FAIL;
	
	m_cbBufferSize= cbLength;
	
	return S_OK;
}
STDMETHODIMP CGMediaBuffer::GetMaxLength( /* [out] */ DWORD *pcbMaxLength)
{
	if(!pcbMaxLength)
		return E_FAIL;
	
	*pcbMaxLength= m_cbBufferCapacity;
	
	return S_OK;
}
STDMETHODIMP CGMediaBuffer::GetBufferAndLength( /* [out] */ BYTE **ppBuffer,/* [out] */ DWORD *pcbLength)
{
	if(!m_pBuffer || !pcbLength)
		return E_FAIL;
	
	*ppBuffer= m_pBuffer;
	*pcbLength= m_cbBufferSize;
	
	return S_OK;
}
CGMediaBufferMgr::~CGMediaBufferMgr()
{
	for(iterator iter=begin(); iter!=end(); iter++)
	{
		(*iter)->Release();
	}
}
STDMETHODIMP CGMediaBufferMgr::GetGMediaBuffer(IGMediaBuffer** ppOutBuf, DWORD cbMaxLength, DWORD cbAlignment)
{
	for(iterator iter=begin(); iter!=end(); iter++)
	{
		//if reference count no more than 2(one is push_back, one is this time add)
		//it means no one use this buffer, check whether it can be reused.
		if((*iter)->AddRef() <= 2)
		{
			DWORD tmpcbMaxLength=0;
			(*iter)->GetMaxLength(&tmpcbMaxLength);
			if(tmpcbMaxLength >= cbMaxLength)
			{
				(*iter)->SetLength(0);	//Clean buffer
				*ppOutBuf= (*iter);
				return S_OK;
			}

		}
		(*iter)->Release();
	}

	if(SUCCEEDED(CGMediaBuffer::CreateBuffer(ppOutBuf, cbMaxLength, cbAlignment)))
	{
		(*ppOutBuf)->AddRef();	//Add it for CGMediaBufferMgr used 
		push_back(*ppOutBuf);
		return S_OK;
	}
	return E_FAIL;
}