//------------------------------------------------------------------------------
// File: MultiGraphSession.h
//
// Desc: Display Server MultiPlayer sample
//
// Copyright (c) InterVideo Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#include "VMR9Subgraph.h"
#include <list>

using namespace std;

class CMultigraphSession
{
public:

    CMultigraphSession();
    virtual ~CMultigraphSession();

    virtual HRESULT Initialize()=0;
    virtual HRESULT Terminate();

	HRESULT AddSource(WCHAR *wcPath, IDisplayVideoMixer** ppMixer, enum SUBGRAPH_DEMUX eDemux, enum SUBGRAPH_VIDEODECOER eVideo, enum SUBGRAPH_AUDIODECODER eAudio, enum SUBGRAPH_VIDEORENDERER eVideoRender, enum SUBGRAPH_VIDEOMIXER eVideoMixer);
    HRESULT DeleteSource(IDisplayVideoMixer* pMixer);

    // get functions
    ISubgraph*   GetSubgraph(IDisplayVideoMixer* pMixer);
    int              GetFrameRate();
    int              GetSize(){ return (int)(m_sources.size()); }
    HWND             GetWindow(){ return m_hwndVideo; }


    IDisplayServer* GetWizard();
    IDisplayRenderEngine* GetRenderEngine();
    IDisplayObject* GetUILayer();
	IParentDisplayObject *GetVideoRoot() { return m_pVideoRoot; }

    // set functions
    HRESULT SetFrameRate( int nFPS);
    HRESULT SetColor( COLORREF color );

    void LoopSources();

    // video window processing
    static LRESULT CALLBACK VideoWndProc(
                                        HWND hwnd,      // handle to window
                                        UINT uMsg,      // message identifier
                                        WPARAM wParam,  // first message parameter
                                        LPARAM lParam   // second message parameter
                                        );

    // private methods
protected:

    HRESULT CreateVideoWindow_(UINT Width, UINT Height, DWORD dwStyle);

    // private data 
protected:
    list<ISubgraph *>        m_sources;
    CComPtr<IDisplayServer>		m_pWizard;
    CComPtr<IDisplayRenderEngine> m_pRenderEngine;
    CComPtr<IDisplayObject>	    m_pUILayer;
	CComPtr<IParentDisplayObject>		m_pVideoRoot;
    HWND                        m_hwndVideo;
};

