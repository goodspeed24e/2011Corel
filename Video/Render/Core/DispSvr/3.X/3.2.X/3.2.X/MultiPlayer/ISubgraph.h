#ifndef SUBGRAPH_HEADER
#define SUBGRAPH_HEADER

enum SUBGRAPH_VIDEORENDERER {
	SUBGRAPH_VIDEORENDERER_VMR9,
	SUBGRAPH_VIDEORENDERER_EVR,
	SUBGRAPH_VIDEORENDERER_DEFAULT
};

enum SUBGRAPH_VIDEODECOER {
	SUBGRAPH_VIDEODECODER_IVI,
	SUBGRAPH_VIDEODECODER_CL,
	SUBGRAPH_VIDEODECODER_DEFAULT
};

enum SUBGRAPH_DEMUX {
	SUBGRAPH_DEMUX_IVI,
	SUBGRAPH_DEMUX_CL,
	SUBGRAPH_DEMUX_MS, 
	SUBGRAPH_DEMUX_DEFAULT
};

enum SUBGRAPH_AUDIODECODER{
	SUBGRAPH_AUDIODECODER_IVI,
	SUBGRAPH_AUDIODECODER_CL,
	SUBGRAPH_AUDIODECODER_DEFAULT,
};

enum SUBGRAPH_VIDEOMIXER{
	SUBGRAPH_VIDEOMIXER_DISPVIDEOMIXER,
	SUBGRAPH_VIDEOMIXER_VIDEOSOURCEDO,
};

class ISubgraph
{
public:
	virtual ~ISubgraph() {}

	virtual HRESULT BuildAndRender(WCHAR* wcPath, IParentDisplayObject *pVideoRoot, enum SUBGRAPH_DEMUX, enum SUBGRAPH_VIDEODECOER, enum SUBGRAPH_AUDIODECODER);
	virtual HRESULT Run() = 0;
	virtual HRESULT Pause() = 0;
	virtual HRESULT Stop() = 0;
	virtual HRESULT SetTime(LONGLONG llCur) = 0;
	virtual HRESULT DestroyGraph(IParentDisplayObject *p) = 0;
	virtual HRESULT DisconnectPins(CComPtr<IBaseFilter> pFilter) = 0;

	virtual OAFilterState GetState() = 0;
	virtual HRESULT GetTimes(LONGLONG& llCur, LONGLONG& llDur) = 0;
	virtual void GetPathT( TCHAR* achPath ) = 0;
	virtual void GetPathW( WCHAR* wcPath ) = 0;
	virtual HRESULT GetFilterConnections(enum SUBGRAPH_DEMUX &eDemux, enum SUBGRAPH_VIDEODECOER &eVideo, enum SUBGRAPH_AUDIODECODER &eAudio, enum SUBGRAPH_VIDEORENDERER &eVideoRender);

	virtual HRESULT GetMixer(IDisplayVideoMixer** ppVideoMixer) = 0;

	static ISubgraph *CreateSubgraph(SUBGRAPH_VIDEORENDERER vr);

protected:
	ISubgraph();

	WCHAR m_wcPath[MAX_PATH];   // path to the media file, wide char
	TCHAR m_achPath[MAX_PATH];  // path to the media file, TCHAR

	CComPtr<IFilterGraph>   m_pGraph;   // filter graph

	CComPtr<IBaseFilter>    m_pDemux;     // Demux
	CComPtr<IBaseFilter>    m_pAudio;     // Audio
	CComPtr<IBaseFilter>    m_pVideo;     // Video
};

#endif	// SUBGRAPH_HEADER
