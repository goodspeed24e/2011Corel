//////////////////////////////////////////////////////////////////////////
//
// This GMO collector is a helper that can merge multiple GMOs to one GMO.
// To compile this helper, you need to add "//depot/ThirdParty/Boost/1_33_1/include" into the additional include directories of your project.
//
// Use this helper by calling CGMOCollect::Make(CGMOCollect** pp, GMOListT GMOList), where
//		CGMOCollect **pp is a pointer to pointer to CGMOCollect instance
//		GMOListT GMOList is a function object that accept an unsigned integer i and returns a function object of type GMOCreateFuncT, where
//			unsigned i is to indicate which GMO is going to be created
//			GMOCreateFuncT is a function object type which has no parameter and returns a CGMOPtr<IGMediaObject>
// GMOs created by the GMOCreateFuncT which is obtained from GMOListT will be released before acquiring another GMOCreateFuncT
// This means that you can unload the DLL ( if the GMO is created from DLL ) which is loaded last time safely.
//
//////////////////////////////////////////////////////////////////////////

#ifndef _GMO_COLLECT_H_
#define _GMO_COLLECT_H_

#include	<windows.h>

#include	<vector>
#include	<utility>

#include	<boost/function.hpp>

#include	"GMOBase.h"
#include	"GMO_i.h"
#include	"GMOImpl.h"

typedef void* (STDAPICALLTYPE * LPFNDLLCREATECOMPONENT)(void);

class CGMOCollect:	public IGMediaObjectImpl<CGMOCollect, 1, 1>, 
					public CGMOUnknown
{
	typedef IGMediaObjectImpl<CGMOCollect, 1, 1> BaseImpl;
	friend class BaseImpl;
	friend class LockIt;
	friend class CGMOUnknown;

public:
	typedef boost::function< CGMOPtr<IGMediaObject>() > GMOCreateFuncT;
	typedef boost::function< GMOCreateFuncT(unsigned) > GMOListT;

	DECLARE_IUNKNOWN
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv)
	{
		CheckPointer(ppv, E_POINTER);
		if ( riid == IID_IGMediaObject )
			return CGMOUnknown::GetInterface((IGMediaObject*)this, ppv);
		else
			return CGMOUnknown::NonDelegatingQueryInterface(riid, ppv);
	}
	static HRESULT STDMETHODCALLTYPE Make(CGMOCollect** pp, GMOListT GMOList);
	virtual IUnknown* GetInterface(REFIID riid);

	// IGMediaObject overides
	virtual STDMETHODIMP SetInputType(DWORD dwInputStreamIndex, const GMO_MEDIA_TYPE *pmt, DWORD dwFlags);
	virtual STDMETHODIMP SetOutputType(DWORD dwOutputStreamIndex, const GMO_MEDIA_TYPE *pmt, DWORD dwFlags);

protected:
	//IGMediaObjectImpl Required overides
	virtual STDMETHODIMP InternalAcceptingInput(DWORD dwInputStreamIndex);
	virtual STDMETHODIMP InternalCheckInputType(DWORD dwInputStreamIndex, const GMO_MEDIA_TYPE *pmt);
	virtual STDMETHODIMP InternalCheckOutputType(DWORD dwOutputStreamIndex, const GMO_MEDIA_TYPE *pmt);	
	virtual STDMETHODIMP InternalGetInputType(DWORD dwInputStreamIndex, DWORD dwTypeIndex, GMO_MEDIA_TYPE *pmt);
	virtual STDMETHODIMP InternalGetOutputType(DWORD dwOutputStreamIndex, DWORD dwTypeIndex, GMO_MEDIA_TYPE *pmt);
	virtual STDMETHODIMP InternalGetInputSizeInfo(DWORD dwInputStreamIndex, DWORD *pcbSize, DWORD *pulSizeMaxLookahead, DWORD *pulSizeAlignment);
	virtual STDMETHODIMP InternalGetOutputSizeInfo(DWORD dwOutputStreamIndex, DWORD *pcbSize, DWORD *pulSizeAlignment);
	virtual STDMETHODIMP InternalGetInputStreamInfo(DWORD dwInputStreamIndex, DWORD *pdwFlags);
	virtual STDMETHODIMP InternalGetOutputStreamInfo(DWORD dwOutputStreamIndex, DWORD *pdwFlags);
	virtual STDMETHODIMP InternalProcessInput(DWORD dwInputStreamIndex, IGMediaBuffer *pBuffer, DWORD dwFlags, REFERENCE_TIME rtTimeStamp, REFERENCE_TIME rtTimeLength);
	virtual STDMETHODIMP InternalProcessOutput(DWORD dwFlags, DWORD OutputBufferCount, GMO_OUTPUT_DATA_BUFFER *pOutputBuffers, DWORD *pdwStatus);
	virtual STDMETHODIMP InternalAllocateStreamingResources(void);
	virtual STDMETHODIMP InternalFreeStreamingResources(void);
	virtual STDMETHODIMP InternalDiscontinuity(DWORD dwInputStreamIndex);
	virtual STDMETHODIMP InternalFlush(void);
	virtual STDMETHODIMP InternalGetInputMaxLatency(DWORD dwInputStreamIndex, REFERENCE_TIME *prtMaxLatency);
	virtual STDMETHODIMP InternalSetInputMaxLatency(DWORD dwInputStreamIndex, REFERENCE_TIME rtMaxLatency);


	CGMOCollect(GMOListT GMOList);
	virtual ~CGMOCollect();

	virtual void Lock(void){}
	virtual void Unlock(void){}

private: 

	typedef std::pair<GMO_MEDIA_TYPE, GMOCreateFuncT> MedaitTypeTuple;

	void GetDecoderGMOs();
	void GetAvailableTypes(CGMOPtr<IGMediaObject> pGMO, GMOCreateFuncT CreateFunc);

	GMOListT m_GMOList;

	CGMOPtr<IGMediaObject> m_pGMO;

	std::vector< MedaitTypeTuple > m_InputTypeTable;
	std::vector< MedaitTypeTuple > m_OutputTypeTable;

};


#endif // _GMO_COLLECT_H_
