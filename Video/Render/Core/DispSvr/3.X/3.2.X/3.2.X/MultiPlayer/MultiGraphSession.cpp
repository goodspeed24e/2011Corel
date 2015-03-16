//------------------------------------------------------------------------------
// File: MultiGraphSession.cpp
//
// Desc: Display Server MultiPlayer sample
//
// Copyright (c) InterVideo Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#include "stdafx.h"
#include "VMR9Subgraph.h"
#include "EvrSubgraph.h"
#include "MultigraphSession.h"

extern HINSTANCE g_hInstance;
const TCHAR g_achVideoWindowClass[] = TEXT("DispSvr Video Window class");

/******************************Public*Routine******************************\
* CMultigraphSession
*
* constructor
\**************************************************************************/
CMultigraphSession::CMultigraphSession()
    : m_hwndVideo( NULL )
{
}

/******************************Public*Routine******************************\
* ~CMultigraphSession
*
* destructor
\**************************************************************************/
CMultigraphSession::~CMultigraphSession()
{
    Terminate();
}

/******************************Public*Routine******************************\
* AddSource
*
* adds new source to the list, attaches it to the wizard
\**************************************************************************/
HRESULT CMultigraphSession::AddSource(WCHAR *wcPath, IDisplayVideoMixer** ppMixer,enum SUBGRAPH_DEMUX eDemux, enum SUBGRAPH_VIDEODECOER eVideo, enum SUBGRAPH_AUDIODECODER eAudio, enum SUBGRAPH_VIDEORENDERER eVideoRender, enum SUBGRAPH_VIDEOMIXER eVideoMixer)
{
    ISubgraph *pSubgraph = NULL;

    HRESULT hr = S_OK;
    if( !wcPath )
    {
        return E_POINTER;
    }
    if( !m_pWizard )
    {
        return VFW_E_WRONG_STATE;
    }

    try
    {
		// commented lines work fine on XP and NT4
		/*
        if( INVALID_FILE_ATTRIBUTES == ::GetFileAttributesW( wcPath))
        {
            throw VFW_E_NOT_FOUND;
        }*/

		// Unicode-related workaround for WindowsMe/98
		{
			char szPath[MAX_PATH];
			DWORD dwGFA = 0L;
			DWORD dwErr = 0L;

			WideCharToMultiByte(CP_ACP, 0, wcPath, -1, szPath, MAX_PATH, NULL, NULL);
			dwGFA = ::GetFileAttributesA( szPath);
			if( INVALID_FILE_ATTRIBUTES == dwGFA)
			{
				dwErr = GetLastError();
				throw VFW_E_NOT_FOUND;
			}
		}

		if (eVideoRender == SUBGRAPH_VIDEORENDERER_EVR)
		{
			SetWindowText(m_hwndVideo, TEXT("EVR Custom Presenter"));
			pSubgraph = ISubgraph::CreateSubgraph(SUBGRAPH_VIDEORENDERER_EVR);
		}
		else
		{
			SetWindowText(m_hwndVideo, TEXT("VMR Custom Allocator/Presenter"));
			pSubgraph = ISubgraph::CreateSubgraph(SUBGRAPH_VIDEORENDERER_VMR9);
		}

        if( !pSubgraph )
        {
            throw E_OUTOFMEMORY;
        }

		if (eVideoRender == SUBGRAPH_VIDEORENDERER_EVR)
		{
			hr = (dynamic_cast<CEvrSubgraph*>(pSubgraph))->BuildAndRender(wcPath, m_pVideoRoot, eDemux, eVideo, eAudio, eVideoMixer);
		}
		else
		{
			hr = (dynamic_cast<CVMR9Subgraph*>(pSubgraph))->BuildAndRender(wcPath, m_pVideoRoot, eDemux, eVideo, eAudio, eVideoMixer);
		}

        if( FAILED(hr))
        {
            throw hr;
        }

        //remember the ID, add to the list
		hr = pSubgraph->GetMixer(ppMixer);
		if (FAILED(hr))
		{
			throw hr;
		}

        m_sources.push_back( pSubgraph );
    }
    catch( HRESULT hr1 )
    {
        if( pSubgraph )
        {
            delete pSubgraph;
        }
        hr = hr1;
    }
	return hr;
}

/******************************Public*Routine******************************\
* DeleteSource
*
* deletes a source from the list, detaches it from the wizard
\**************************************************************************/
HRESULT CMultigraphSession::DeleteSource(IDisplayVideoMixer* pMixer)
{
    HRESULT hr = S_OK;

    ISubgraph *pSubgraph = NULL;

    try
    {
        if( !m_pWizard )
        {
            throw VFW_E_WRONG_STATE;
        }

        pSubgraph = GetSubgraph(pMixer);
        if( !pSubgraph )
        {
            throw VFW_E_NOT_FOUND;
        }

        hr = pSubgraph->DestroyGraph(m_pVideoRoot);
        if( FAILED(hr))
        {
            throw hr;
        }

        m_sources.remove( pSubgraph );

        delete pSubgraph;
        pSubgraph = NULL;
        hr = S_OK;
    }
    catch( HRESULT hr1 )
    {
        hr = hr1;
    }

    return hr;
}

/******************************Public*Routine******************************\
* Terminate
*
* terminates wizard
\**************************************************************************/
HRESULT CMultigraphSession::Terminate()
{
    HRESULT hr = S_OK;
    list<ISubgraph*>::iterator it;

    ISubgraph* pSubgraph = NULL;

    // 1. Detach all the existing subgraphs
    while( false == m_sources.empty())
    {
        it = m_sources.begin();
        pSubgraph = (ISubgraph*)(*it);
        ASSERT( pSubgraph );

		CComPtr<IDisplayVideoMixer> pMixer;
		hr = pSubgraph->GetMixer(&pMixer);
        hr = DeleteSource(pMixer);
        ASSERT(SUCCEEDED(hr));
    }

    // 2. Terminate Wizard, if there is any
    if( m_pWizard )
    {
        hr = m_pWizard->Terminate();
        ASSERT(SUCCEEDED(hr));
    }

    // 3. Destroy video window
    if( IsWindow( m_hwndVideo ))
    {
        ::DestroyWindow( m_hwndVideo );
        m_hwndVideo = NULL;
    }

    UnregisterClass( g_achVideoWindowClass, g_hInstance);

    return S_OK;
}

/******************************Public*Routine******************************\
* GetSubgraph
*
* find subgraph in the list by dwID
\**************************************************************************/
ISubgraph* CMultigraphSession::GetSubgraph(IDisplayVideoMixer* pMixer)
{
    ISubgraph* pSubgraph = NULL;
    list<ISubgraph*>::iterator start, end, it;

    start = m_sources.begin();
    end = m_sources.end();

    for(it = start; it!= end; it++)
    {
		CComPtr<IDisplayVideoMixer> itMixer;
        if( (*it) && SUCCEEDED((*it)->GetMixer(&itMixer)) && itMixer == pMixer)
        {
            pSubgraph = (ISubgraph*)(*it);
            break;
        }
    }
    return pSubgraph;
}

/******************************Public*Routine******************************\
* GetWizard
*
\**************************************************************************/
IDisplayServer* CMultigraphSession::GetWizard( )
{
    return m_pWizard;
}

/******************************Public*Routine******************************\
* GetRenderEngine
*
\**************************************************************************/
IDisplayRenderEngine* CMultigraphSession::GetRenderEngine( )
{
    return m_pRenderEngine;
}

/******************************Public*Routine******************************\
* GetUILayer
*
\**************************************************************************/
IDisplayObject* CMultigraphSession::GetUILayer( )
{
    return m_pUILayer;
}

/******************************Public*Routine******************************\
* GetFrameRate
*
\**************************************************************************/
int CMultigraphSession::GetFrameRate()
{
    int nRes = 0;

    if( !m_pRenderEngine )
        return 30;

    m_pRenderEngine->GetFrameRate( &nRes );

	static BOOL bShow = FALSE;
	if (IsWindowVisible(m_hwndVideo) != bShow)
	{
		RECT rect;
		::GetClientRect(m_hwndVideo, &rect);
		m_pRenderEngine->SetDisplayWindow(m_hwndVideo, &rect);
		bShow = IsWindowVisible(m_hwndVideo);
	}
    return nRes;
}

/******************************Public*Routine******************************\
* SetColor
*
\**************************************************************************/
HRESULT CMultigraphSession::SetColor( COLORREF color )
{
    HRESULT hr = S_OK;

    if( !m_pRenderEngine )
        return E_FAIL;

    hr = m_pRenderEngine->SetBackgroundColor( color );

    return hr;
}

/******************************Public*Routine******************************\
* LoopSources
*
* go through the list and reset time to start if movie is close to the end
\**************************************************************************/
void CMultigraphSession::LoopSources()
{
    ISubgraph *pSource = NULL;
    LONGLONG llCur;
    LONGLONG llDur;

    list<ISubgraph*>::iterator start, end, it;
    start = m_sources.begin();
    end = m_sources.end();

    for( it = start; it != end; it++)
    {
        pSource = (ISubgraph*)(*it);
        if( !pSource )
            continue;

        pSource->GetTimes( llCur, llDur);
        // 100ms
        if( llDur - llCur < 1000000L )
        {
            pSource->SetTime( 0L );
        }
    }
}

/******************************Public*Routine******************************\
* CreateVideoWindow_
*
* creates video window
\**************************************************************************/
HRESULT CMultigraphSession::CreateVideoWindow_(UINT Width, UINT Height, DWORD dwStyle)
{
    HRESULT hr = S_OK;
    WNDCLASSEX wcex;
    RECT rc;
    RECT rcClient;
    int dx, dy;

    try
    {
        if( IsWindow( m_hwndVideo ))
            throw E_UNEXPECTED;

        wcex.cbSize = sizeof(WNDCLASSEX); 
        wcex.style          = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc    = (WNDPROC)CMultigraphSession::VideoWndProc;
        wcex.cbClsExtra     = 0;
        wcex.cbWndExtra     = 0;
        wcex.hInstance      = g_hInstance;
        wcex.hIcon          = NULL;
        wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
        wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
        wcex.lpszMenuName   = TEXT("");
        wcex.lpszClassName  = g_achVideoWindowClass;
        wcex.hIconSm        = NULL;

        RegisterClassEx( &wcex);

        m_hwndVideo  = CreateWindow(    g_achVideoWindowClass, 
                                        TEXT("VMR9 Custom Allocator-Presenter"), 
                                        dwStyle,
                                        100, 100, 
                                        800, 600, 
                                        NULL, 
                                        NULL, 
                                        g_hInstance, 
                                        NULL);
        if( !IsWindow( m_hwndVideo ))
            throw E_FAIL;

        ::SetWindowLong( m_hwndVideo, GWL_USERDATA, (LONG)this);

        // resize the window so that client area be 800x600
        ::GetClientRect( m_hwndVideo, &rcClient);

        ::GetWindowRect( m_hwndVideo, &rc);
        dx = rc.right - rc.left - rcClient.right;
        dy = rc.bottom - rc.top - rcClient.bottom;

        ::SetWindowPos( m_hwndVideo, HWND_TOP, 0, 0, Width+dx, Height+dy, SWP_NOMOVE);

    }// try
    catch( HRESULT hr1 )
    {
        hr = hr1;
    }

    return hr;
}

/******************************Public*Routine******************************\
* VideoWndProc
*
\**************************************************************************/
LRESULT CALLBACK CMultigraphSession::VideoWndProc(
                                        HWND hwnd,      // handle to window
                                        UINT uMsg,      // message identifier
                                        WPARAM wParam,  // first message parameter
                                        LPARAM lParam   // second message parameter
                                        )
{
    CMultigraphSession* This = NULL;
    CComPtr<IDisplayObject> pUI;
    LRESULT lRes = 0;

    This = (CMultigraphSession*)::GetWindowLong( hwnd, GWL_USERDATA);
    if( This )
    {
        pUI = This->GetUILayer();
    }

    switch (uMsg)
    {
	case WM_PAINT:
		if (This)
		{
			CComPtr<IDisplayRenderEngine> pRenderEngine = This->GetRenderEngine();
			if (pRenderEngine)
			{
				HRESULT hr = E_FAIL;
				hr = pRenderEngine->SetColorKey(RGB(0x08, 0x08, 0x08));
				if (SUCCEEDED(hr))
				{
					RECT rect = {0, 0, 0, 0};
					::GetClientRect(hwnd, &rect);
					PAINTSTRUCT ps;
					HDC hdc = BeginPaint(hwnd, &ps);
					if (hdc)
					{
						HBRUSH hBrush = CreateSolidBrush(RGB(0x08, 0x08, 0x08));
						::FillRect(hdc, &rect, hBrush);
					}
					EndPaint(hwnd, &ps);
				}
			}
		}
		break;
	case WM_SHOWWINDOW:
	case WM_MOVE:
        if (This)
        {
            CComPtr<IDisplayRenderEngine> pRenderEngine = This->GetRenderEngine();
            if (pRenderEngine)
            {
				CRect rcNew(0, 0, 0, 0);
				::GetClientRect(hwnd, &rcNew);
				pRenderEngine->SetDisplayWindow(hwnd, &rcNew);
            }
        }
		break;

	case WM_CLOSE:
		// video window should not be closed from sys menu.
		return lRes;

// For testing purpose only
    case WM_SIZE:
        if (This)
        {
            CComPtr<IDisplayRenderEngine> pRenderEngine = This->GetRenderEngine();
            if (pRenderEngine)
            {
                UINT w = LOWORD(lParam);
                UINT h = HIWORD(lParam);
#ifdef _DEBUG
                pRenderEngine->SetBackBufferSize(w, h);
#endif
				CRect rcNew(0, 0, w, h);
				pRenderEngine->SetDisplayWindow(hwnd, &rcNew);
            }
        }
    }
	lRes = DefWindowProc( hwnd, uMsg, wParam, lParam);
	if( pUI )
	{
		pUI->ProcessMessage( hwnd, uMsg, (UINT)wParam, (LONG)lParam);
	}

    return lRes;
}

