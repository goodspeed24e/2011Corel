#ifndef EVRSUBGRAPH_H
#define EVRSUBGRAPH_H

#pragma once
#include "ISubgraph.h"

class CEvrSubgraph : public ISubgraph
{
public:
	CEvrSubgraph();
	virtual ~CEvrSubgraph();

	// public methods
	virtual HRESULT BuildAndRender(WCHAR* wcPath, IParentDisplayObject *pVideoRoot, enum SUBGRAPH_DEMUX, enum SUBGRAPH_VIDEODECOER, enum SUBGRAPH_AUDIODECODER, enum SUBGRAPH_VIDEOMIXER eVideoMixer);
	virtual HRESULT Run();
	virtual HRESULT Pause();
	virtual HRESULT Stop();
	virtual HRESULT SetTime(LONGLONG llCur);
	virtual HRESULT DestroyGraph(IParentDisplayObject *pVideoRoot);
	virtual HRESULT DisconnectPins(CComPtr<IBaseFilter> pFilter);
	HRESULT CheckConnection();

	OAFilterState GetState();
	HRESULT GetTimes(LONGLONG& llCur, LONGLONG& llDur);
	void GetPathT(TCHAR* achPath);
	void GetPathW(WCHAR* wcPath);

	HRESULT GetMixer(IDisplayVideoMixer** ppVideoMixer)
	{
		return m_pVideoMixer.CopyTo(ppVideoMixer);
	}

private:
	// private members

private:
	// private data
	CComPtr<IBaseFilter>    m_pEVR;     // EVR
	CComPtr<IMediaControl>  m_pMc;      // media control
	CComPtr<IMediaSeeking>  m_pMs;      // media seeking
	CComPtr<IDisplayVideoMixer> m_pVideoMixer;
	CComPtr<IBaseFilter>    m_pDemux;     // Demux
	CComPtr<IBaseFilter>    m_pAudio;     // Audio
	CComPtr<IBaseFilter>    m_pVideo;     // Video


	CComPtr<IDisplayVideoSource> m_pVidSrc;
};

#endif

