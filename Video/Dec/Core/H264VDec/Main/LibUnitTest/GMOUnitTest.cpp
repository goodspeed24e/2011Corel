#include <vector>

#include <windows.h>
#include <strmif.h>
#include <mtype.h>

#include <boost/test/unit_test.hpp>

#include "GMO/GMediaBuffer.h"
#include "GMO/GMOHelper.h"
#include "../LibSmartPtr/SmartGMediatype.h"

using namespace IVI;

namespace UnitTest
{

	inline bool operator==(const GMO_MEDIA_TYPE& lhs, const GMO_MEDIA_TYPE& rhs)
	{
		return ((IsEqualGUID(lhs.majortype,rhs.majortype) == TRUE) &&
			(IsEqualGUID(lhs.subtype,rhs.subtype) == TRUE) &&
			(IsEqualGUID(lhs.formattype,rhs.formattype) == TRUE) &&
			(lhs.cbFormat == rhs.cbFormat) &&
			( (lhs.cbFormat == 0) ||
			lhs.pbFormat != NULL && rhs.pbFormat != NULL &&
			(memcmp(lhs.pbFormat, rhs.pbFormat, lhs.cbFormat) == 0)));
	}

	typedef std::vector<GMO_MEDIA_TYPE> TypeList;
	std::vector<TypeList> InputTypeLists, OutputTypeLists;

	void TestInput(CGMOPtr<IGMediaObject> pGMO, DWORD StreamCount)
	{
		unsigned StreamIndex = 0;
		DWORD dwFlags, cbSize, cbMaxLookahead, cbAlignment;
		REFERENCE_TIME rtMaxLatency = 0;
		HRESULT hr;

		for(StreamIndex = 0; StreamIndex < StreamCount; ++StreamIndex)
		{
			TypeList types;
			for(int TypeIndex = 0;; ++TypeIndex)
			{
				SmartGMediatype pMediaType, pTmpMediaType;
				GMediaObjectHelper::MoCreateMediaType(&pMediaType);
				GMediaObjectHelper::MoCreateMediaType(&pTmpMediaType);
				if ( pGMO->GetInputType(StreamIndex, TypeIndex, pMediaType) == S_OK )
				{
					types.push_back(*pMediaType);
					hr = pGMO->SetInputType(StreamIndex, pMediaType, 0);
					BOOST_CHECK( SUCCEEDED(hr) );
					if ( hr == S_OK )
					{
						BOOST_CHECK( pGMO->GetInputCurrentType(StreamIndex, pTmpMediaType) == S_OK );
						BOOST_CHECK( *pMediaType == *pTmpMediaType);
					}
				}
				else
					break;
			}
			if ( types.size() != 0 )
			{
				InputTypeLists.push_back(types);
				BOOST_CHECK( pGMO->SetInputType(StreamIndex, &types[0], 0) == S_OK );

				BOOST_CHECK( pGMO->GetInputStreamInfo(StreamIndex, 0) == E_POINTER );
				BOOST_CHECK( pGMO->GetInputStreamInfo(StreamIndex, &dwFlags) == S_OK );

				BOOST_CHECK( pGMO->GetInputStatus(StreamIndex, 0) == E_POINTER );
				BOOST_CHECK( pGMO->GetInputStatus(StreamIndex, &dwFlags) == S_OK );

				BOOST_CHECK( pGMO->GetInputSizeInfo(StreamIndex, NULL, NULL, NULL) == E_POINTER );
				BOOST_CHECK( pGMO->GetInputSizeInfo(StreamIndex, &cbSize, NULL, NULL) == E_POINTER );
				BOOST_CHECK( pGMO->GetInputSizeInfo(StreamIndex, &cbSize, &cbMaxLookahead, NULL) == E_POINTER );
				BOOST_CHECK( pGMO->GetInputSizeInfo(StreamIndex, &cbSize, &cbMaxLookahead, &cbAlignment) == S_OK );

				BOOST_CHECK( pGMO->GetInputMaxLatency(StreamIndex, 0) == E_POINTER );
				hr = pGMO->GetInputMaxLatency(StreamIndex, &rtMaxLatency);
				BOOST_CHECK( hr == S_OK || hr == E_NOTIMPL );	// E_NOTIMPL means 0 latency
				if ( hr == S_OK )
					BOOST_CHECK( pGMO->SetInputMaxLatency(StreamIndex, rtMaxLatency) );

			}
		}
	}

	void TestOutput(CGMOPtr<IGMediaObject> pGMO, DWORD StreamCount)
	{
		unsigned StreamIndex = 0;
		DWORD dwFlags, cbSize, cbAlignment;
		REFERENCE_TIME rtMaxLatency = 0;
		HRESULT hr = S_OK;

		for(StreamIndex = 0; StreamIndex < StreamCount; ++StreamIndex)
		{
			TypeList types;
			for(int TypeIndex = 0;; ++TypeIndex)
			{
				SmartGMediatype pMediaType, pTmpMediaType;
				GMediaObjectHelper::MoCreateMediaType(&pMediaType);
				GMediaObjectHelper::MoCreateMediaType(&pTmpMediaType);
				if ( pGMO->GetOutputType(StreamIndex, TypeIndex, pMediaType) == S_OK )
				{
					types.push_back(*pMediaType);
					hr = pGMO->SetOutputType(StreamIndex, pMediaType, 0);
					BOOST_CHECK( SUCCEEDED(hr) );
					if ( hr == S_OK )
					{
						BOOST_CHECK( pGMO->GetOutputCurrentType(StreamIndex, pTmpMediaType) == S_OK );
						BOOST_CHECK( *pMediaType == *pTmpMediaType);
					}
				}
				else
					break;
			}
			if ( types.size() != 0 )
			{
				OutputTypeLists.push_back(types);
				BOOST_CHECK( pGMO->SetOutputType(StreamIndex, &types[0], 0) == S_OK );

				BOOST_CHECK( pGMO->GetOutputStreamInfo(StreamIndex, 0) == E_POINTER );
				BOOST_CHECK( pGMO->GetOutputStreamInfo(StreamIndex, &dwFlags) == S_OK );

				BOOST_CHECK( pGMO->GetOutputSizeInfo(StreamIndex, NULL, NULL) == E_POINTER );
				BOOST_CHECK( pGMO->GetOutputSizeInfo(StreamIndex, &cbSize, NULL) == E_POINTER );
				BOOST_CHECK( pGMO->GetOutputSizeInfo(StreamIndex, &cbSize, NULL) == E_POINTER );
				BOOST_CHECK( pGMO->GetOutputSizeInfo(StreamIndex, &cbSize, &cbAlignment) == S_OK );
			}
		}
	}


	void CheckInputStreamIndex(CGMOPtr<IGMediaObject> pGMO, DWORD StreamCount)
	{
		DWORD dwFlags, cbSize, cbMaxLookahead, cbAlignment;
		GMO_MEDIA_TYPE MediaType;
		REFERENCE_TIME rtMaxLatency = 0;
		CGMOPtr<IGMediaBuffer> pMediaBuffer;
		CGMediaBuffer::CreateBuffer(&pMediaBuffer, 4096);
		HRESULT hr;

		BOOST_CHECK ( pGMO->GetInputStreamInfo(StreamCount, &dwFlags) == GMO_E_INVALIDSTREAMINDEX );
		BOOST_CHECK ( pGMO->GetInputType(StreamCount, 0, &MediaType) == GMO_E_INVALIDSTREAMINDEX );
		BOOST_CHECK ( pGMO->SetInputType(StreamCount, &MediaType, 0) == GMO_E_INVALIDSTREAMINDEX );
		BOOST_CHECK ( pGMO->GetInputCurrentType(StreamCount, &MediaType) == GMO_E_INVALIDSTREAMINDEX );
		BOOST_CHECK ( pGMO->GetInputSizeInfo(StreamCount, &cbSize, &cbMaxLookahead, &cbAlignment) == GMO_E_INVALIDSTREAMINDEX );
		BOOST_CHECK ( pGMO->ProcessInput(StreamCount, pMediaBuffer, 0, 0, 0) == GMO_E_INVALIDSTREAMINDEX );
		BOOST_CHECK ( pGMO->Discontinuity(StreamCount) == GMO_E_INVALIDSTREAMINDEX );

		hr = pGMO->GetInputMaxLatency(StreamCount, &rtMaxLatency);
		BOOST_CHECK ( hr == E_NOTIMPL || hr == GMO_E_INVALIDSTREAMINDEX );
		hr = pGMO->SetInputMaxLatency(StreamCount, rtMaxLatency);
		BOOST_CHECK ( hr == E_NOTIMPL || hr == GMO_E_INVALIDSTREAMINDEX );
	}

	void CheckOutputStreamIndex(CGMOPtr<IGMediaObject> pGMO, DWORD StreamCount)
	{
		DWORD dwFlags, cbSize, cbAlignment;
		GMO_MEDIA_TYPE MediaType;
		REFERENCE_TIME rtMaxLatency = 0;
		CGMOPtr<IGMediaBuffer> pMediaBuffer;
		CGMediaBuffer::CreateBuffer(&pMediaBuffer, 4096);

		BOOST_CHECK ( pGMO->GetOutputStreamInfo(StreamCount, &dwFlags) == GMO_E_INVALIDSTREAMINDEX );
		BOOST_CHECK ( pGMO->GetOutputType(StreamCount, 0, &MediaType) == GMO_E_INVALIDSTREAMINDEX );
		BOOST_CHECK ( pGMO->SetOutputType(StreamCount, &MediaType, 0) == GMO_E_INVALIDSTREAMINDEX );
		BOOST_CHECK ( pGMO->GetOutputCurrentType(StreamCount, &MediaType) == GMO_E_INVALIDSTREAMINDEX );
		BOOST_CHECK ( pGMO->GetOutputSizeInfo(StreamCount, &cbSize, &cbAlignment) == GMO_E_INVALIDSTREAMINDEX );
		BOOST_CHECK ( pGMO->ProcessInput(StreamCount, pMediaBuffer, 0, 0, 0) == GMO_E_INVALIDSTREAMINDEX );
	}



	void BasicGMOTest(CGMOPtr<IGMediaObject> pGMO)
	{
		using namespace std;
		using namespace boost::unit_test;

		BOOST_CHECK( pGMO->GetStreamCount(NULL, NULL) == E_POINTER );

		DWORD InputStreamCount, OutputStreamCount;

		BOOST_CHECK( pGMO->GetStreamCount(&InputStreamCount, &OutputStreamCount) == S_OK );

		TestInput(pGMO, InputStreamCount);
		TestOutput(pGMO, OutputStreamCount);

		CheckInputStreamIndex(pGMO, InputStreamCount);
		CheckOutputStreamIndex(pGMO, OutputStreamCount);

		if ( InputTypeLists.size() )
		{
			BOOST_CHECK( pGMO->Flush() == S_OK );
			BOOST_CHECK( pGMO->AllocateStreamingResources () == S_OK );
			BOOST_CHECK( pGMO->FreeStreamingResources() == S_OK );
		}

	}

}

