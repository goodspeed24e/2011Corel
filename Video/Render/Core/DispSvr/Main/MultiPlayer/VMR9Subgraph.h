//------------------------------------------------------------------------------
// File: VMR9Subgraph.h
//
// Desc: Display Server MultiPlayer sample
//
// Copyright (c) InterVideo Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#ifndef VMR9SUBGRAPH_HEADER
#define VMR9SUBGRAPH_HEADER

#pragma once
#include "ISubgraph.h"

// CVMR9Subgraph

// This class represents a subgraph with VMR9 to be attached to the wizard
// This sample supports only media files, but any DirectShow filter graph
// with VMR9 can be used.

class CVMR9Subgraph : public ISubgraph
{
public:
    CVMR9Subgraph();
    virtual ~CVMR9Subgraph();

    // public methods
	virtual HRESULT BuildAndRender(WCHAR* wcPath, IParentDisplayObject *pVideoRoot, enum SUBGRAPH_DEMUX, enum SUBGRAPH_VIDEODECOER, enum SUBGRAPH_AUDIODECODER, enum SUBGRAPH_VIDEOMIXER eVideoMixer);
    virtual HRESULT Run();
    virtual HRESULT Pause();
    virtual HRESULT Stop();
    virtual HRESULT SetTime( LONGLONG llCur);
    virtual HRESULT DestroyGraph(IParentDisplayObject *pVideoRoot);
    virtual HRESULT DisconnectPins( CComPtr<IBaseFilter> pFilter);
    HRESULT CheckVMRConnection();

    OAFilterState GetState();
    HRESULT GetTimes( LONGLONG& llCur, LONGLONG& llDur);
    void GetPathT( TCHAR* achPath );
    void GetPathW( WCHAR* wcPath );

	HRESULT GetMixer(IDisplayVideoMixer** ppVideoMixer)
	{
		return m_pVideoMixer.CopyTo(ppVideoMixer);
	}

private:
    // private members

private:
    // private data
    CComPtr<IBaseFilter>    m_pVMR;     // VMR9
    CComPtr<IMediaControl>  m_pMc;      // media control
    CComPtr<IMediaSeeking>  m_pMs;      // media seeking
	CComPtr<IDisplayVideoMixer> m_pVideoMixer;

	CComPtr<IDisplayVideoSource> m_pVidSrc;
};

#endif

