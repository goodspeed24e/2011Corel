#pragma once

#define STRICT
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <windows.h>
#include "resource/resource.h"

//-----------------------------------------------------------------------------
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit
#define _WTL_USE_CSTRING

#include <atlbase.h>       // base ATL classes
#include <atlapp.h>        // base WTL classes
#include <atlwin.h>        // ATL GUI classes
#include <atlframe.h>      // WTL frame window classes
#include <atlmisc.h>       // WTL utility classes like CString
#include <atlcrack.h>      // WTL enhanced msg map macros

extern CAppModule _Module; // WTL version of CComModule

#include <atlctrls.h>
#include <atldlgs.h>

//-----------------------------------------------------------------------------
#include <windows.h>
#include <tchar.h>
#include <strsafe.h>

#include "DOG.h"
#include "DogCategory.h"
#include "SubCategory.h"
#include "DogUserEvent.h"

#include <string>
#include <map>
#include "boost/tuple/tuple.hpp"

#include "BufferDraw.h"
#include "CurveDraw.h"

//-----------------------------------------------------------------------------
class CAudioPage : public CFrameWindowImpl<CAudioPage>
{
	friend class CFrameWindowImpl<CAudioPage>;
	DECLARE_WND_CLASS(_T("DOG Audio Page Class"))

public:
	void Init()
	{
		int displayPos = 0;

		m_BufStatusList[dog::AUDIO_DECODING_1ST_DATA_EVENT].DisplayPos = displayPos;
		m_BufStatusList[dog::AUDIO_DECODING_1ST_DATA_EVENT].Draw.SetTitle("Primary Decoder");
		++displayPos;

		m_BufStatusList[dog::AUDIO_DECODING_2ND_DATA_EVENT].DisplayPos = displayPos;
		m_BufStatusList[dog::AUDIO_DECODING_2ND_DATA_EVENT].Draw.SetTitle("Secondary Decoder");
		++displayPos;

		m_BufStatusList[dog::AUDIO_BACKEND_1ST_BUFFER_EVENT].DisplayPos = displayPos;
		m_BufStatusList[dog::AUDIO_BACKEND_1ST_BUFFER_EVENT].Draw.SetTitle("Backend Primary");
		++displayPos;

		m_BufStatusList[dog::AUDIO_BACKEND_2ND_BUFFER_EVENT].DisplayPos = displayPos;
		m_BufStatusList[dog::AUDIO_BACKEND_2ND_BUFFER_EVENT].Draw.SetTitle("Backend Secondary");
		++displayPos;

		m_BufStatusList[dog::AUDIO_BACKEND_METADATA_BUFFER_EVENT].DisplayPos = displayPos;
		m_BufStatusList[dog::AUDIO_BACKEND_METADATA_BUFFER_EVENT].Draw.SetTitle("Backend Metadata");
		++displayPos;

		m_BufStatusList[dog::AUDIO_RENDER_BUFFER_EVENT].DisplayPos = displayPos;
		m_BufStatusList[dog::AUDIO_RENDER_BUFFER_EVENT].Draw.SetTitle("Renderer");
		++displayPos;

		m_CurveWatchList[dog::AUDIO_AVSYNC_PTS_DELTA_UPDATE_EVENT].DisplayPos = displayPos;
		m_CurveWatchList[dog::AUDIO_AVSYNC_PTS_DELTA_UPDATE_EVENT].Draw.SetTitle("Renderer PTS diff");
		++displayPos;
	}

	void UpdateBufferEvent(const dog::BufferEvent& bufEvent)
	{
		std::map<int, CBufferStatus >::iterator iterBuf = m_BufStatusList.find(bufEvent.Id);
		if(iterBuf != m_BufStatusList.end())
		{
			CBufferStatus& bufStatus = m_BufStatusList[bufEvent.Id];
			bufStatus.Draw.UpdateBufferEvent(bufEvent);
		}
	}

	void UpdateDataEvent(const dog::VariantDataEvent& dataEvent)
	{
		std::map<int, CCurveWatch >::iterator iterCrv = m_CurveWatchList.find(dataEvent.Id);
		if(iterCrv != m_CurveWatchList.end())
		{
			CCurveWatch& crvWatch = m_CurveWatchList[dataEvent.Id];
			crvWatch.Draw.PushBackData(dataEvent.Data);
		}
	}

protected:
	CBufferDraw m_bufDraw;

	struct CBufferStatus
	{
		int DisplayPos;
		CBufferDraw Draw;
	};

	struct CCurveWatch
	{
		int DisplayPos;
		CCurveDraw Draw;
	};
	std::map<int, CBufferStatus > m_BufStatusList;
	std::map<int, CCurveWatch > m_CurveWatchList;

	BEGIN_MSG_MAP(CAudioPage)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MSG_WM_PAINT( OnPaint )
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		CHAIN_MSG_MAP(CFrameWindowImpl<CAudioPage>)
	END_MSG_MAP()

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return 0;
	}

	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return 0;
	}

	LRESULT OnEraseBackground(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		// disable background erasing
		return 1; // return WM_ERASEBKGND is handled
	}

	void OnPaint( HDC hdc)
	{
		Paint();
		SetMsgHandled(FALSE);
	}

public:
	void Paint()
	{
		RECT rcClient;
		GetClientRect(&rcClient);
		CClientDC dc(m_hWnd);

		CMemoryDC memDC(dc, rcClient);
		memDC.SelectStockFont( DEFAULT_GUI_FONT );

		// clear canvas
		memDC.FillSolidRect( &rcClient, RGB(0xff,0xff,0xff) );

		size_t w = m_bufDraw.GetWidth();
		size_t h = m_bufDraw.GetHeight();

		std::map<int, CBufferStatus >::iterator iterBuf;
		for(iterBuf=m_BufStatusList.begin(); iterBuf!=m_BufStatusList.end(); ++iterBuf)
		{
			int pos = iterBuf->second.DisplayPos;
			int x = 20 + (pos%4)* (w + 20);
			int y = 20 + (pos/4)* (h + 20);
			iterBuf->second.Draw.Paint(memDC, x, y);
		}

		std::map<int, CCurveWatch >::iterator iterCrv;
		for(iterCrv=m_CurveWatchList.begin(); iterCrv!=m_CurveWatchList.end(); ++iterCrv)
		{
			int pos = iterCrv->second.DisplayPos;
			int x = 20 + (pos%4)* (w + 20);
			int y = 20 + (pos/4)* (h + 20);
			iterCrv->second.Draw.Paint(memDC, x, y);
		}
	}
};
