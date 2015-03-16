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

//-----------------------------------------------------------------------------
class CVideo2ndPage : public CFrameWindowImpl<CVideo2ndPage>
{
	friend class CFrameWindowImpl<CVideo2ndPage>;
	DECLARE_WND_CLASS(_T("DOG Video (2nd) Decoder Page Class"))

public:
	void Init()
	{
		int displayPos = 0;

		// secondary video
		m_CurveWatchList[dog::VIDEO_2ND_BITRATE_UPDATE_EVENT].DisplayPos = displayPos;
		m_CurveWatchList[dog::VIDEO_2ND_BITRATE_UPDATE_EVENT].Draw.SetTitle("video bit rate (2nd)");
		++displayPos;

		m_CurveWatchList[dog::VIDEO_2ND_TOTAL_FRAMES_UPDATE_EVENT].DisplayPos = displayPos;
		m_CurveWatchList[dog::VIDEO_2ND_TOTAL_FRAMES_UPDATE_EVENT].Draw.SetTitle("total frames (2nd)");
		++displayPos;

		m_CurveWatchList[dog::VIDEO_2ND_I_FRAMES_UPDATE_EVENT].DisplayPos = displayPos;
		m_CurveWatchList[dog::VIDEO_2ND_I_FRAMES_UPDATE_EVENT].Draw.SetTitle("I frames (2nd)");
		++displayPos;

		m_CurveWatchList[dog::VIDEO_2ND_P_FRAMES_UPDATE_EVENT].DisplayPos = displayPos;
		m_CurveWatchList[dog::VIDEO_2ND_P_FRAMES_UPDATE_EVENT].Draw.SetTitle("P frames (2nd)");
		++displayPos;

		m_CurveWatchList[dog::VIDEO_2ND_B_FRAMES_UPDATE_EVENT].DisplayPos = displayPos;
		m_CurveWatchList[dog::VIDEO_2ND_B_FRAMES_UPDATE_EVENT].Draw.SetTitle("B frames (2nd)");
		++displayPos;

		m_CurveWatchList[dog::VIDEO_2ND_INTERLACED_FRAMES_UPDATE_EVENT].DisplayPos = displayPos;
		m_CurveWatchList[dog::VIDEO_2ND_INTERLACED_FRAMES_UPDATE_EVENT].Draw.SetTitle("interlaced frames (2nd)");
		++displayPos;

		m_CurveWatchList[dog::VIDEO_2ND_ENCRYPTED_FRAMES_UPDATE_EVENT].DisplayPos = displayPos;
		m_CurveWatchList[dog::VIDEO_2ND_ENCRYPTED_FRAMES_UPDATE_EVENT].Draw.SetTitle("encrypted frames (2nd)");
		++displayPos;

		m_CurveWatchList[dog::VIDEO_2ND_FRAME_RATE_UPDATE_EVENT].DisplayPos = displayPos;
		m_CurveWatchList[dog::VIDEO_2ND_FRAME_RATE_UPDATE_EVENT].Draw.SetTitle("frame rate (2nd)");
		++displayPos;

		m_CurveWatchList[dog::VIDEO_2ND_PRESENTATION_JITTER_UPDATE_EVENT].DisplayPos = displayPos;
		m_CurveWatchList[dog::VIDEO_2ND_PRESENTATION_JITTER_UPDATE_EVENT].Draw.SetTitle("presentation jitter (2nd)");
		++displayPos;

		m_CurveWatchList[dog::VIDEO_2ND_DECODE_MSEC_UPDATE_EVENT].DisplayPos = displayPos;
		m_CurveWatchList[dog::VIDEO_2ND_DECODE_MSEC_UPDATE_EVENT].Draw.SetTitle("video decode (2nd)");
		++displayPos;
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

	struct CCurveWatch
	{
		int DisplayPos;
		CCurveDraw Draw;
	};
	std::map<int, CCurveWatch > m_CurveWatchList;


	BEGIN_MSG_MAP(CVideo2ndPage)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MSG_WM_PAINT( OnPaint )
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		CHAIN_MSG_MAP(CFrameWindowImpl<CVideo2ndPage>)
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
