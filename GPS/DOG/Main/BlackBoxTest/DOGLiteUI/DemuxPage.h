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
class CDemuxPage : public CFrameWindowImpl<CDemuxPage>
{
	friend class CFrameWindowImpl<CDemuxPage>;
	DECLARE_WND_CLASS(_T("DOG Demux Page Class"))

public:
	void Init()
	{
		int displayPos = 0;

		m_BufStatusList[dog::DEMUX_VIDEO_EB_EVENT].DisplayPos = displayPos;
		m_BufStatusList[dog::DEMUX_VIDEO_EB_EVENT].Draw.SetTitle("Video EB");
		++displayPos;

		m_BufStatusList[dog::DEMUX_2ND_VIDEO_EB_EVENT].DisplayPos = displayPos;
		m_BufStatusList[dog::DEMUX_2ND_VIDEO_EB_EVENT].Draw.SetTitle("Video EB (2nd)");
		++displayPos;

		m_BufStatusList[dog::DEMUX_AUDIO_EB_EVENT].DisplayPos = displayPos;
		m_BufStatusList[dog::DEMUX_AUDIO_EB_EVENT].Draw.SetTitle("Audio EB");
		++displayPos;

		m_BufStatusList[dog::DEMUX_2ND_AUDIO_EB_EVENT].DisplayPos = displayPos;
		m_BufStatusList[dog::DEMUX_2ND_AUDIO_EB_EVENT].Draw.SetTitle("Audio EB (2nd)");
		++displayPos;

		m_BufStatusList[dog::DEMUX_PG_EB_EVENT].DisplayPos = displayPos;
		m_BufStatusList[dog::DEMUX_PG_EB_EVENT].Draw.SetTitle("PG EB");
		++displayPos;

		m_BufStatusList[dog::DEMUX_2ND_PG_EB_EVENT].DisplayPos = displayPos;
		m_BufStatusList[dog::DEMUX_2ND_PG_EB_EVENT].Draw.SetTitle("PG EB (2nd)");
		++displayPos;

		m_BufStatusList[dog::DEMUX_IG_EB_EVENT].DisplayPos = displayPos;
		m_BufStatusList[dog::DEMUX_IG_EB_EVENT].Draw.SetTitle("IG EB");
		++displayPos;
	}

	void UpdateBufferEvent(const dog::BufferEvent& bufEvent)
	{
		std::map<int, CBufferStatus >::iterator iter = m_BufStatusList.find(bufEvent.Id);
		if(iter == m_BufStatusList.end()) {
			return;
		}

		CBufferStatus& bufStatus = m_BufStatusList[bufEvent.Id];
		bufStatus.Draw.UpdateBufferEvent(bufEvent);
	}

protected:
	CBufferDraw m_bufDraw;

	struct CBufferStatus
	{
		int DisplayPos;
		CBufferDraw Draw;
	};
	std::map<int, CBufferStatus > m_BufStatusList;

	BEGIN_MSG_MAP(CDemuxPage)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MSG_WM_PAINT( OnPaint )
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		CHAIN_MSG_MAP(CFrameWindowImpl<CDemuxPage>)
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

		std::map<int, CBufferStatus >::iterator iter;
		for(iter=m_BufStatusList.begin(); iter!=m_BufStatusList.end(); ++iter)
		{
			int pos = iter->second.DisplayPos;
			int x = 20 + (pos%4)* (w + 20);
			int y = 20 + (pos/4)* (h + 20);
			iter->second.Draw.Paint(memDC, x, y);
		}
	}
};
