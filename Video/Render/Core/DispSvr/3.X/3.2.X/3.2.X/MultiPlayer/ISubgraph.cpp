#include "stdafx.h"
#include "VMR9Subgraph.h"
#include "EvrSubgraph.h"
#include "Imports\IviDsDemultiplexer\IviDsDemuxPublicEnumerations.h"
#include "Imports\IviDsDemultiplexer\IviDsDemuxPublicGuids.h"

/*
DEFINE_GUID(CLSID_IviDsDemultiplexer,0x5723ab6c, 0x81b8, 0x41fa, 0x99, 0x88, 0x9b, 0xbf, 0x25, 0x5f, 0xcb, 0xd2);

DEFINE_GUID(CLSID_VIDEO,0x246ca20, 0x776d, 0x11d2, 0x80, 0x10, 0x0, 0x10, 0x4b, 0x9b, 0x85, 0x92);

DEFINE_GUID(CLSID_CLdemux, 0x555c90cd, 0xd094, 0x4672, 0x9b, 0x26, 0xb7, 0x3b, 0xdd, 0x38, 0xfe, 0xee);

DEFINE_GUID(CLSID_CLAudio, 0x284dc28a, 0x4a7d, 0x442c, 0xbc, 0x2e, 0xd7, 0x48, 0x05, 0x56, 0xe4, 0xd8);

DEFINE_GUID(CLSID_CLVideo,0x8acd52ed, 0x9c2d, 0x4008, 0x91, 0x29, 0xdc, 0xe9, 0x55, 0xd8, 0x60, 0x65);

DEFINE_GUID(CLSID_MSDEMUX, 0xafb6c280,0x2c41, 0x11d3, 0x8a, 0x60, 0x00, 0x00, 0xf8, 0x1e, 0x0e,0x4a);

DEFINE_GUID(CLSID_CIVIAudioFilter,0x7e2e0dc1, 0x31fd, 0x11d2, 0x9c, 0x21, 0x0, 0x10, 0x4b, 0x38, 0x1, 0xf6);

DEFINE_GUID(AM_KSPROPSETID_DemuxConfiguration,	0xcbf67519, 0x1e9d, 0x47f6, 0xb7, 0xa2, 0x5d, 0xd7, 0x9f, 0x3a, 0xe9, 0x67);
*/


/*
*  IviDsDemultiplexer filter's related GUIDs
*/

// {5723AB6C-81B8-41fa-9988-9BBF255FCBD2}

EXTERN_C const GUID CLSID_IviDsDemultiplexer={0x5723ab6c, 0x81b8, 0x41fa, 0x99, 0x88, 0x9b, 0xbf, 0x25, 0x5f, 0xcb, 0xd2};

EXTERN_C const GUID CLSID_VIDEO={0x246ca20, 0x776d, 0x11d2, 0x80, 0x10, 0x0, 0x10, 0x4b, 0x9b, 0x85, 0x92};

EXTERN_C const GUID CLSID_CLdemux={0x555c90cd, 0xd094, 0x4672, 0x9b, 0x26, 0xb7, 0x3b, 0xdd, 0x38, 0xfe, 0xee};

EXTERN_C const GUID CLSID_CLAudio={0x284dc28a, 0x4a7d, 0x442c, 0xbc, 0x2e, 0xd7, 0x48, 0x05, 0x56, 0xe4, 0xd8};

EXTERN_C const GUID CLSID_CLVideo={0x8acd52ed, 0x9c2d, 0x4008, 0x91, 0x29, 0xdc, 0xe9, 0x55, 0xd8, 0x60, 0x65};

EXTERN_C const GUID CLSID_MSDEMUX={0xafb6c280,0x2c41, 0x11d3, 0x8a, 0x60, 0x00, 0x00, 0xf8, 0x1e, 0x0e,0x4a};

EXTERN_C const GUID CLSID_CIVIAudioFilter={0x7e2e0dc1, 0x31fd, 0x11d2, 0x9c, 0x21, 0x0, 0x10, 0x4b, 0x38, 0x1, 0xf6};

EXTERN_C const GUID AM_KSPROPSETID_DemuxConfiguration={0xcbf67519, 0x1e9d, 0x47f6, 0xb7, 0xa2, 0x5d, 0xd7, 0x9f, 0x3a, 0xe9, 0x67};



ISubgraph::ISubgraph()
{
	m_wcPath[0] = L'\0';
	m_achPath[0] = TEXT('\0');

	m_pGraph=NULL;
	m_pDemux=NULL;
	m_pAudio=NULL;
	m_pVideo=NULL;
}


ISubgraph *ISubgraph::CreateSubgraph(SUBGRAPH_VIDEORENDERER vr)
{
	ISubgraph *pGraph = 0;

	switch (vr)
	{
	case SUBGRAPH_VIDEORENDERER_VMR9:
		pGraph = new CVMR9Subgraph();
		break;
	case SUBGRAPH_VIDEORENDERER_EVR:
		pGraph = new CEvrSubgraph();
		break;
	}
	
	return pGraph;
}

/*void DbgMsg(char* szMessage, ...)
{
	char szFullMessage[MAX_PATH];
	char szFormatMessage[MAX_PATH];

	// format message
	va_list ap;
	va_start(ap, szMessage);
	_vsnprintf(szFormatMessage, MAX_PATH, szMessage, ap);
	va_end(ap);
	strncat(szFormatMessage, "\n", MAX_PATH);
	strcpy(szFullMessage, "---->");
	strcat(szFullMessage, szFormatMessage);
	OutputDebugStringA(szFullMessage);
}*/

HRESULT ISubgraph::GetFilterConnections(enum SUBGRAPH_DEMUX &eDemux, enum SUBGRAPH_VIDEODECOER &eVideo, enum SUBGRAPH_AUDIODECODER &eAudio, enum SUBGRAPH_VIDEORENDERER &eVideoRender)
{
	if ( m_pGraph )
	{
		CComPtr<IEnumFilters> pEnum;
		CComPtr<IBaseFilter> pFilter;
		FILTER_INFO fi;

		eDemux=SUBGRAPH_DEMUX_DEFAULT;
		eVideo=SUBGRAPH_VIDEODECODER_DEFAULT;
		eAudio=SUBGRAPH_AUDIODECODER_DEFAULT;
		eVideoRender=SUBGRAPH_VIDEORENDERER_DEFAULT;

		HRESULT hr = m_pGraph->EnumFilters( &(pEnum.p) );
		if( FAILED(hr))
		{
			return hr;
		}

		hr = pEnum->Next(1, &(pFilter.p), NULL);
		while( S_OK == hr && pFilter )
		{
			hr = pFilter->QueryFilterInfo( &fi);
			if( fi.pGraph)
				fi.pGraph->Release();

			if( 0 == wcscmp( fi.achName, L"VMR9"))
			{
				eVideoRender=SUBGRAPH_VIDEORENDERER_VMR9;
			}
			else if( 0 == wcscmp( fi.achName, L"EVR"))
			{
				eVideoRender=SUBGRAPH_VIDEORENDERER_EVR;
			}
			
			if ( 0 == wcscmp( fi.achName, L"InterVideo MPEG Demultiplexer"))
			{
				eDemux=SUBGRAPH_DEMUX_IVI;
			}
			else if ( 0 == wcsicmp( fi.achName, L"Cyberlink Demultiplexer"))
			{
				eDemux=SUBGRAPH_DEMUX_CL;		
			}
			else if ( 0 == wcsicmp( fi.achName, L"MPEG-2 Splitter") ||  0 == wcscmp( fi.achName, L"MPEG-2 Demultiplexer") )
			{
				eDemux=SUBGRAPH_DEMUX_MS;		
			}

			if ( 0 == wcsicmp( fi.achName, L"InterVideo Video Decoder"))
			{
				eVideo=SUBGRAPH_VIDEODECODER_IVI;
			}
			else if ( 0 == wcsnicmp( fi.achName, L"Cyberlink Video/SP Decoder", 26))
			{
				eVideo=SUBGRAPH_VIDEODECODER_CL;		
			}


			if ( 0 == wcsicmp( fi.achName, L"InterVideo Audio Decoder"))
			{
				eAudio=SUBGRAPH_AUDIODECODER_IVI;		
			}
			else if ( 0 == wcsicmp( fi.achName, L"Cyberlink Audio Decoder"))
			{
				eAudio=SUBGRAPH_AUDIODECODER_CL;		
			}

			pFilter = NULL;
			hr = pEnum->Next(1, &(pFilter.p), NULL);
		}
		
		pFilter = NULL;
		pEnum = NULL;
	}
	else
	{
	}
	return S_OK;
}

HRESULT ISubgraph::BuildAndRender(WCHAR *wcPath, IParentDisplayObject *pVideoRoot, SUBGRAPH_DEMUX eDemux, SUBGRAPH_VIDEODECOER eVideo, SUBGRAPH_AUDIODECODER eAudio)
{
	HRESULT hr;

	if( !wcPath )
	{
		return E_POINTER;
	}

	USES_CONVERSION;
	wcsncpy( m_wcPath, wcPath, MAX_PATH);
	_tcsncpy( m_achPath, W2T(wcPath), MAX_PATH);

	// first, check that file exists
	if( INVALID_FILE_ATTRIBUTES == GetFileAttributes( m_achPath))
	{
		MessageBox(NULL, TEXT("Requested media file was not found"), TEXT("Error"), MB_OK);
		return VFW_E_NOT_FOUND;
	}

	// create graph
	hr = CoCreateInstance( CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, 
		IID_IFilterGraph, (void**)&(m_pGraph.p) );
	if( FAILED(hr))
	{
		MessageBox(NULL, TEXT("Failed to create the filter graph"), TEXT("Error"), MB_OK);
		return hr;
	}

	// ====== Create and add Demux filter =======================================
	if ( eDemux == SUBGRAPH_DEMUX_MS )
	{
		hr = CoCreateInstance( CLSID_MSDEMUX, NULL, CLSCTX_INPROC_SERVER,	IID_IBaseFilter, (void**)&(m_pDemux.p) );
		if( FAILED(hr))
		{
			MessageBox(NULL, TEXT("Failed to create instance of Demux"), TEXT("Error"), MB_OK);
			return hr;
		}

		hr = m_pGraph->AddFilter( m_pDemux, L"MPEG-2 Demultiplexer");
		if( FAILED(hr))
		{
			MessageBox(NULL, TEXT("Failed to add MS Demux to the graph"), TEXT("Error"), MB_OK);
			return hr;
		}
	}
	else if ( eDemux == SUBGRAPH_DEMUX_IVI )
	{
		hr = CoCreateInstance( CLSID_IviDsDemultiplexer, NULL, CLSCTX_INPROC_SERVER,	IID_IBaseFilter, (void**)&(m_pDemux.p) );
		if( FAILED(hr))
		{
			MessageBox(NULL, TEXT("Failed to create instance of Demux"), TEXT("Error"), MB_OK);
			return hr;
		}

		hr = m_pGraph->AddFilter( m_pDemux, L"InterVideo MPEG Demultiplexer");
		if( FAILED(hr))
		{
			MessageBox(NULL, TEXT("Failed to add IVI Demux to the graph"), TEXT("Error"), MB_OK);
			return hr;
		}

		// For IviDsDemultiplexer, we set the operation mode to AUTO that the filter
		// can automatically create output pins.
		CComPtr<IKsPropertySet> t_pPropertySet;
		hr = m_pDemux->QueryInterface(IID_IKsPropertySet, (void**)&t_pPropertySet);
		if (FAILED(hr))
			return hr;		// make sure we get the interface properly.

		EDEMUX_FILTER_OPERATION_MODE t_eOpMode = E_DFOM_AUTO_MODE;
		hr = t_pPropertySet->Set(AM_KSPROPSETID_DemuxConfiguration, 
			E_DFCC_OPERATION_MODE, &t_eOpMode, 
			sizeof(EDEMUX_FILTER_OPERATION_MODE), NULL, 0);

	}
	else if ( eDemux == SUBGRAPH_DEMUX_CL )
	{
		hr = CoCreateInstance( CLSID_CLdemux, NULL, CLSCTX_INPROC_SERVER,	IID_IBaseFilter, (void**)&(m_pDemux.p) );
		if( FAILED(hr))
		{
			MessageBox(NULL, TEXT("Failed to create instance of Demux"), TEXT("Error"), MB_OK);
			return hr;
		}

		hr = m_pGraph->AddFilter( m_pDemux, L"Cyberlink Demultiplexer");
		if( FAILED(hr))
		{
			MessageBox(NULL, TEXT("Failed to add CL Demux to the graph"), TEXT("Error"), MB_OK);
			return hr;
		}
	}

	// ====== Create and add video filter =======================================
	if ( eVideo == SUBGRAPH_VIDEODECODER_IVI )
	{
		hr = CoCreateInstance( CLSID_VIDEO, NULL, CLSCTX_INPROC_SERVER,	IID_IBaseFilter, (void**)&(m_pVideo.p) );
		if( FAILED(hr))
		{
			MessageBox(NULL, TEXT("Failed to create instance of Video"), TEXT("Error"), MB_OK);
			return hr;
		}

		hr = m_pGraph->AddFilter( m_pVideo, L"InterVideo Video Decoder");
		if( FAILED(hr))
		{
			MessageBox(NULL, TEXT("Failed to add IVI Video to the graph"), TEXT("Error"), MB_OK);
			return hr;
		}
	}
	else if ( eVideo == SUBGRAPH_VIDEODECODER_CL )
	{
		hr = CoCreateInstance( CLSID_CLVideo, NULL, CLSCTX_INPROC_SERVER,	IID_IBaseFilter, (void**)&(m_pVideo.p) );
		if( FAILED(hr))
		{
			MessageBox(NULL, TEXT("Failed to create instance of Video"), TEXT("Error"), MB_OK);
			return hr;
		}

		hr = m_pGraph->AddFilter( m_pVideo, L"Cyberlink Video/SP Decoder");
		if( FAILED(hr))
		{
			MessageBox(NULL, TEXT("Failed to add CL Video to the graph"), TEXT("Error"), MB_OK);
			return hr;
		}
	}

	// ====== Create and add audio filter =======================================
	if ( eAudio == SUBGRAPH_AUDIODECODER_IVI )
	{
		hr = CoCreateInstance( CLSID_CIVIAudioFilter, NULL, CLSCTX_INPROC_SERVER,	IID_IBaseFilter, (void**)&(m_pVideo.p) );
		if( FAILED(hr))
		{
			MessageBox(NULL, TEXT("Failed to create instance of audio"), TEXT("Error"), MB_OK);
			return hr;
		}

		hr = m_pGraph->AddFilter( m_pVideo, L"InterVideo Audio Decoder");
		if( FAILED(hr))
		{
			MessageBox(NULL, TEXT("Failed to add IVI Audio to the graph"), TEXT("Error"), MB_OK);
			return hr;
		}
	}
	else if ( eAudio == SUBGRAPH_AUDIODECODER_CL )
	{
		hr = CoCreateInstance( CLSID_CLAudio, NULL, CLSCTX_INPROC_SERVER,	IID_IBaseFilter, (void**)&(m_pVideo.p) );
		if( FAILED(hr))
		{
			MessageBox(NULL, TEXT("Failed to create instance of audio"), TEXT("Error"), MB_OK);
			return hr;
		}

		hr = m_pGraph->AddFilter( m_pVideo, L"Cyberlink Audio Decoder");
		if( FAILED(hr))
		{
			MessageBox(NULL, TEXT("Failed to add CL Audio to the graph"), TEXT("Error"), MB_OK);
			return hr;
		}
	}

	return S_OK;
}