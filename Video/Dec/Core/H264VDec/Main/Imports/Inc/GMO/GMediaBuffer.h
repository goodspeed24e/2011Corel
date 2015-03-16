#ifndef __GMEDIA_BUF_H
#define __GMEDIA_BUF_H

#include "GMOBase.h"
#include "GMO_i.h"
#include <vector>

class CGMediaBuffer: public CGMOUnknown, IGMediaBuffer{
public:
	static  HRESULT STDMETHODCALLTYPE CreateBuffer(IGMediaBuffer** ppOutBuf, DWORD cbMaxLength=0, DWORD cbAlignment=4);
	DECLARE_IUNKNOWN
		
	STDMETHOD(SetLength)(DWORD cbLength);
	STDMETHOD(GetMaxLength)( /* [out] */ DWORD *pcbMaxLength);
	STDMETHOD(GetBufferAndLength)( /* [out] */ BYTE **ppBuffer,/* [out] */ DWORD *pcbLength);
protected:
	CGMediaBuffer() : CGMOUnknown(NULL), m_pBuffer(NULL), m_cbBufferSize(0), m_cbBufferCapacity(0) {}
	~CGMediaBuffer();
	
	BYTE	*m_pBuffer;
	DWORD	m_cbBufferSize;		//occupy size
	DWORD	m_cbBufferCapacity;	//maximum size
};

class CGMediaBufferMgr: protected std::vector<IGMediaBuffer*>
{
public:
	STDMETHOD(GetGMediaBuffer)(IGMediaBuffer** ppOutBuf, DWORD cbMaxLength=0, DWORD cbAlignment=4);
	CGMediaBufferMgr(){};
	~CGMediaBufferMgr();
};
namespace GMediaObjectHelper{

inline bool HaveEnoguhBuffer(const CGMOPtr<IGMediaBuffer>& pMediaBuffer, const CGMOPtr<IGMediaObject>& pMediaObject, DWORD dwOutputStreamIndex = 0, DWORD* pcbRequiredLength = NULL)
{
	DWORD cbRequiredLength = 0, cbAlignment;
	if ( pMediaObject->GetOutputSizeInfo(dwOutputStreamIndex, &cbRequiredLength, &cbAlignment) == S_OK ){
		if ( pcbRequiredLength )
			*pcbRequiredLength = cbRequiredLength;
		if ( pMediaBuffer ){
			DWORD cbBufferLength = 0;
			if ( pMediaBuffer->GetMaxLength(&cbBufferLength) == S_OK ){
				if ( cbBufferLength >= cbRequiredLength )
					return true;
			}
		}
	}
	return false;
}

inline HRESULT AllocateMediaBuffer(CGMOPtr<IGMediaBuffer> &pMediaBuffer, const CGMOPtr<IGMediaObject>& pMediaObject, DWORD dwOutputStreamIndex = 0, DWORD cbAlignment = 1)
{
	if ( !pMediaObject )
		return E_POINTER;
	HRESULT hr = S_OK;
	DWORD cbRequiredLength = 0;

	if ( HaveEnoguhBuffer(pMediaBuffer, pMediaObject, dwOutputStreamIndex, &cbRequiredLength) )
		return S_OK;
	if ( cbRequiredLength == 0)
		return E_FAIL;
	CGMOPtr<IGMediaBuffer> pMyMediaBuffer;
	if ( ( hr = CGMediaBuffer::CreateBuffer(&pMyMediaBuffer, cbRequiredLength, cbAlignment) ) != S_OK )
		return hr;
	pMediaBuffer = pMyMediaBuffer;
	return S_OK;
}

}	// namespace CGMediaObjectHelper

#endif	//__GMEDIA_BUF_H
