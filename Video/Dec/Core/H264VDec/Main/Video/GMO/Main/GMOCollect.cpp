#include	"GMOCollect.h"

#include	<functional>
#include	<cassert>

#include	<boost/function.hpp>
#include	<boost/bind.hpp>

using namespace std;
using namespace boost;
using namespace GMediaObjectHelper;

HRESULT CGMOCollect::Make(CGMOCollect** pp, GMOListT GMOList)
{
	IUnknown* pTemp= reinterpret_cast<IUnknown*>(*pp);
	if(pTemp)
	{
		*pp= 0;
		pTemp->Release();
	}
	*pp= new CGMOCollect(GMOList);
	if(!(*pp))
		return E_FAIL;
	(*pp)->AddRef();
	return S_OK;
}

CGMOCollect::CGMOCollect(GMOListT GMOList) : CGMOUnknown(NULL), m_GMOList(GMOList)
{
	GetDecoderGMOs();
}

CGMOCollect::~CGMOCollect()
{

}

void CGMOCollect::GetDecoderGMOs()
{
	for(unsigned int i = 0;; ++i )
	{
		GMOCreateFuncT GMOCreator = m_GMOList(i);
		if ( !GMOCreator.empty() )
		{
			CGMOPtr<IGMediaObject> pGMO = GMOCreator();
			if ( pGMO )
			{
				GetAvailableTypes(pGMO, GMOCreator);
			}
		}
		else
			break;
	}
}

void CGMOCollect::GetAvailableTypes(CGMOPtr<IGMediaObject> pGMO, GMOCreateFuncT CreateFunc)
{
	assert(pGMO);

	HRESULT hr = S_OK;
	DWORD cbInputStreamCount, cbOutputStreamCount;
	hr = pGMO->GetStreamCount(&cbInputStreamCount, &cbOutputStreamCount);
	assert( hr == S_OK );

	for(DWORD StreamIndex = 0; StreamIndex < cbInputStreamCount; ++StreamIndex)
	{
		for(int TypeIndex = 0;; ++TypeIndex)
		{
			GMO_MEDIA_TYPE MediaType;
			if ( pGMO->GetInputType(StreamIndex, TypeIndex, &MediaType) == S_OK )
				m_InputTypeTable.push_back( make_pair(MediaType, CreateFunc) );
			else
				break;
		}
	}

	for(DWORD StreamIndex = 0; StreamIndex < cbOutputStreamCount; ++StreamIndex)
	{
		for(int TypeIndex = 0;; ++TypeIndex)
		{
			GMO_MEDIA_TYPE MediaType;
			if ( pGMO->GetOutputType(StreamIndex, TypeIndex, &MediaType) == S_OK )
				m_OutputTypeTable.push_back( make_pair(MediaType, CreateFunc) );
			else
				break;
		}
	}
}

HRESULT CGMOCollect::InternalCheckInputType(DWORD dwInputStreamIndex, const GMO_MEDIA_TYPE *pmt)
{
	return S_OK;
}

HRESULT CGMOCollect::InternalCheckOutputType(DWORD dwOutputStreamIndex, const GMO_MEDIA_TYPE *pmt)
{
	return S_OK;
}

HRESULT CGMOCollect::SetInputType(DWORD dwInputStreamIndex, const GMO_MEDIA_TYPE *pmt, DWORD dwFlags)
{
	// a trival binary functor for the following find_if algorithm to compare MediaTypeTuple.first with GMO_MEDIA_TYPE
	struct equl_test : public binary_function<MedaitTypeTuple, GMO_MEDIA_TYPE, bool>
	{
		bool operator()(const MedaitTypeTuple& lhs, const GMO_MEDIA_TYPE& rhs)
		{
			return IsEqualGUID( lhs.first.majortype, rhs.majortype ) && IsEqualGUID( lhs.first.subtype, rhs.subtype );
		}
	};

	HRESULT hr = S_OK;

	hr = BaseImpl::SetInputType(dwInputStreamIndex, pmt, dwFlags);
	if ( FAILED(hr) )
		return hr;

	vector< MedaitTypeTuple >::iterator it = find_if(m_InputTypeTable.begin(), m_InputTypeTable.end(), bind(equl_test(), _1, *pmt) );

	if ( it == m_InputTypeTable.end() )
		return GMO_E_INVALIDTYPE;

	if ( dwFlags == GMO_SET_TYPEF_TEST_ONLY )
		return S_OK;

	if ( m_pGMO )
	{
		m_pGMO->Flush();
		hr = m_pGMO->SetInputType(dwInputStreamIndex, pmt, 0);
	}

	if ( hr == GMO_E_TYPE_NOT_ACCEPTED || hr == S_FALSE || !m_pGMO )
	{
		m_pGMO = NULL;		// must be released before Creating another GMO
		m_pGMO = it->second();			// find GMO create function and create GMO instance

		assert(m_pGMO);
		hr = m_pGMO->SetInputType(dwInputStreamIndex, pmt, 0);
		GMO_MEDIA_TYPE output_media_type;
		if ( SUCCEEDED(hr) && this->OutputTypeSet(dwInputStreamIndex) )
		{
			hr = this->GetOutputCurrentType(dwInputStreamIndex, &output_media_type);
			if ( SUCCEEDED(hr) )
			{
				hr = this->SetOutputType(dwInputStreamIndex, &output_media_type, 0);
				GMediaObjectHelper::MoFreeMediaType(&output_media_type);
			}
		}
	}

	return hr;
}

HRESULT CGMOCollect::SetOutputType(DWORD dwOutputStreamIndex, const GMO_MEDIA_TYPE *pmt, DWORD dwFlags)
{
	HRESULT hr = S_OK;
	hr = BaseImpl::SetOutputType(dwOutputStreamIndex, pmt, dwFlags);
	if ( FAILED(hr) )
		return hr;
	if ( !m_pGMO )
		return GMO_E_TYPE_NOT_ACCEPTED;
	return m_pGMO->SetOutputType(dwOutputStreamIndex, pmt, 0);
}

HRESULT CGMOCollect::InternalGetInputType(DWORD dwInputStreamIndex, DWORD dwTypeIndex, GMO_MEDIA_TYPE *pmt)
{
	if ( dwInputStreamIndex >= 1 )
		return GMO_E_INVALIDSTREAMINDEX;

	if ( dwTypeIndex >= m_InputTypeTable.size() )
		return GMO_E_NO_MORE_ITEMS;

	MoCopyMediaType(pmt, &m_InputTypeTable[dwTypeIndex].first);

	return S_OK;
}

HRESULT CGMOCollect::InternalGetOutputType(DWORD dwOutputStreamIndex, DWORD dwTypeIndex, GMO_MEDIA_TYPE *pmt)
{
	if ( dwOutputStreamIndex >= 1 )
		return GMO_E_INVALIDSTREAMINDEX;

	if ( dwTypeIndex >= m_OutputTypeTable.size() )
		return GMO_E_NO_MORE_ITEMS;

	MoCopyMediaType(pmt, &m_OutputTypeTable[dwTypeIndex].first);

	return S_OK;
}

HRESULT CGMOCollect::InternalFlush(void)
{
	if ( !m_pGMO )
		return E_FAIL;
	return m_pGMO->Flush();
}

HRESULT CGMOCollect::InternalGetInputSizeInfo(DWORD dwInputStreamIndex, DWORD *pcbSize, DWORD *pulSizeMaxLookahead, DWORD *pulSizeAlignment)
{
	if ( !m_pGMO )
		return E_FAIL;
	return m_pGMO->GetInputSizeInfo(dwInputStreamIndex, pcbSize, pulSizeMaxLookahead, pulSizeAlignment);
}

HRESULT CGMOCollect::InternalGetOutputSizeInfo(DWORD dwOutputStreamIndex, DWORD *pcbSize, DWORD *pulSizeAlignment)
{
	if ( !m_pGMO )
		return E_FAIL;
	return m_pGMO->GetOutputSizeInfo(dwOutputStreamIndex, pcbSize, pulSizeAlignment);
}

HRESULT CGMOCollect::InternalProcessInput(DWORD dwInputStreamIndex, IGMediaBuffer *pBuffer, DWORD dwFlags, REFERENCE_TIME rtTimeStamp, REFERENCE_TIME rtTimeLength)
{
	if ( !m_pGMO )
		return E_FAIL;
	return m_pGMO->ProcessInput(dwInputStreamIndex, pBuffer, dwFlags, rtTimeStamp, rtTimeLength);
}

STDMETHODIMP CGMOCollect::InternalProcessOutput(DWORD dwFlags, DWORD OutputBufferCount, GMO_OUTPUT_DATA_BUFFER *pOutputBuffers, DWORD *pdwStatus)
{
	if ( !m_pGMO )
		return E_FAIL;
	return m_pGMO->ProcessOutput(dwFlags, OutputBufferCount, pOutputBuffers, pdwStatus);
}

STDMETHODIMP CGMOCollect::InternalAcceptingInput(DWORD dwInputStreamIndex)
{
	return dwInputStreamIndex == 0 ? S_OK : E_FAIL;
}

STDMETHODIMP CGMOCollect::InternalGetInputStreamInfo(DWORD dwInputStreamIndex, DWORD *pdwFlags)
{
	*pdwFlags = 0;
	if ( !m_pGMO )
		return GMO_E_TYPE_NOT_ACCEPTED;
	return m_pGMO->GetInputStreamInfo(dwInputStreamIndex, pdwFlags);
}

STDMETHODIMP CGMOCollect::InternalGetOutputStreamInfo(DWORD dwOutputStreamIndex, DWORD *pdwFlags)
{
	*pdwFlags = 0;
	if ( !m_pGMO )
		return GMO_E_TYPE_NOT_ACCEPTED;
	return m_pGMO->GetOutputStreamInfo(dwOutputStreamIndex, pdwFlags);
}

HRESULT CGMOCollect::InternalAllocateStreamingResources(void)
{
	if ( !m_pGMO )
		return GMO_E_TYPE_NOT_ACCEPTED;
	return m_pGMO->AllocateStreamingResources();
}

HRESULT CGMOCollect::InternalFreeStreamingResources(void)
{
	if ( !m_pGMO )
		return GMO_E_TYPE_NOT_ACCEPTED;
	return m_pGMO->FreeStreamingResources();
}

HRESULT CGMOCollect::InternalDiscontinuity(DWORD dwInputStreamIndex)
{
	if ( !m_pGMO )
		return GMO_E_TYPE_NOT_ACCEPTED;
	return m_pGMO->Discontinuity(dwInputStreamIndex);
}

HRESULT CGMOCollect::InternalGetInputMaxLatency(DWORD dwInputStreamIndex, REFERENCE_TIME *prtMaxLatency)
{
	if ( !m_pGMO )
		return GMO_E_TYPE_NOT_ACCEPTED;
	return m_pGMO->GetInputMaxLatency(dwInputStreamIndex, prtMaxLatency);
}

HRESULT CGMOCollect::InternalSetInputMaxLatency(DWORD dwInputStreamIndex, REFERENCE_TIME rtMaxLatency)
{
	if ( !m_pGMO )
		return GMO_E_TYPE_NOT_ACCEPTED;
	return m_pGMO->SetInputMaxLatency(dwInputStreamIndex, rtMaxLatency);
}

IUnknown* CGMOCollect::GetInterface(REFIID riid)
{
	IUnknown *param = NULL;
	if ( m_pGMO )
		m_pGMO->QueryInterface( riid, reinterpret_cast<void**>(&param) );

	return param;
}
