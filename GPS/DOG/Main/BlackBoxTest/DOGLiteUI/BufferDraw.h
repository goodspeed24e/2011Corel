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

#include <deque>
#include <boost/format.hpp>
#include "DogUserEvent.h"

extern CBrush GreenBrush;
extern CBrush BlackBrush;

typedef LONGLONG REFERENCE_TIME;

class CBufferDraw
{
protected:
	static RECT Translate(const RECT& rect, int dx, int dy)
	{
		RECT tmp = {rect.left+dx, rect.top+dy, rect.right+dx, rect.bottom+dy};
		return tmp;
	}

	static POINT Translate(const POINT& pos, int dx, int dy)
	{
		POINT tmp = {pos.x +dx, pos.y +dy };
		return tmp;
	}

	static void PaintProgressBar(CDC& dc, DWORD cur, DWORD max, int x, int y, const RECT& rect, const CBrush& innerColor, const CBrush& frameColor)
	{
		const int nWidth  = std::max<LONG>(rect.right-rect.left, 0);
		const int nHeight = std::max<LONG>(rect.bottom-rect.top, 0);

		RECT rcInner = {rect.left, rect.top, rect.right, rect.bottom};
		RECT rcFrame = {rect.left, rect.top, rect.right, rect.bottom};
		rcInner.right = 0;

		if(cur > max) { cur = max; }
		if(max > 0) {
			rcInner.right = nWidth * cur / max;
		}

		rcInner = Translate(rcInner, x,y);
		rcFrame = Translate(rcFrame, x,y);
		dc.FillRect(&rcInner, innerColor);
		dc.FrameRect(&rcFrame, frameColor);
	}

	static void PaintProgressBarWithText(CDC& dc, DWORD cur, DWORD max, int x, int y, const CBrush& innerColor, const CBrush& frameColor)
	{
		const LONG vspace = 3;
		const RECT rcBar  = {0,0, 140,20};
		const RECT rcText = {0,0, 140,20};

		POINT pos = {0, 0};
		pos = Translate(pos, x,y);
		PaintProgressBar(dc, cur, max, pos.x, pos.y, rcBar, innerColor, frameColor);

		RECT rcDrawText;
		rcDrawText = Translate(rcText, pos.x, pos.y+rcBar.bottom+vspace);
		WTL::CString text;
		text.Format(_T("%6d / %6d"), cur, max);
		dc.DrawText(text, text.GetLength(), &rcDrawText, DT_RIGHT | DT_SINGLELINE | DT_VCENTER);
	}

protected:
	struct CBufferStatus
	{
		DWORD BufferSize; //current buffer data size in the process unit
		DWORD Capacity;   //capacity of the buffer of the process unit

		REFERENCE_TIME PTS_In;
		REFERENCE_TIME PTS_Out;

		DWORD Rx_In;
		DWORD Rx_Out;
	};

	WTL::CString  m_Title;
	CBufferStatus m_BufStatus;

	const size_t m_MaxEventQueueSize;
	std::deque<dog::BufferEvent> m_DataInCache;
	std::deque<dog::BufferEvent> m_DataOutCache;

	struct CGeometry
	{
		size_t Width, Heigth;

		CGeometry()
		{
			Width = 180;
			Heigth = 150;
		}
	};
	CGeometry m_Geomery;


protected:
	void UpdateBufferSize(DWORD bufSize, DWORD capacity)
	{
		m_BufStatus.BufferSize = bufSize;
		m_BufStatus.Capacity = capacity;
	}

	void UpdateInputStatus(REFERENCE_TIME pts_in, DWORD rx_in)
	{
		m_BufStatus.PTS_In = pts_in;
		m_BufStatus.Rx_In = rx_in;
	}

	void UpdateOutputStatus(REFERENCE_TIME pts_out, DWORD rx_out)
	{
		m_BufStatus.PTS_Out = pts_out;
		m_BufStatus.Rx_Out = rx_out;
	}

	DWORD CalcDataRate(std::deque<dog::BufferEvent>& dataCache)
	{
		const size_t cacheSize = dataCache.size();
		DWORD totalDataOut = 0;
		for(size_t i=1; i<cacheSize; ++i)
		{
			totalDataOut += dataCache[i].BufferSize;
		}
		DWORD dataRate = 0;
		if(cacheSize>2)
		{
			LARGE_INTEGER t0 = dataCache[0].TimeStamp;
			LARGE_INTEGER t1 = dataCache[cacheSize-1].TimeStamp;
			double duration_sec = ((double)(t1.QuadPart-t0.QuadPart) / 1e7);
			if(duration_sec >0.0f) {
				dataRate = (DWORD)((double)totalDataOut / duration_sec);
			}
		}
		return dataRate;
	}

public:
	CBufferDraw() : m_MaxEventQueueSize(5)
	{
		m_Title = "Buffer Status";
		memset(&m_BufStatus, 0, sizeof(m_BufStatus));

// 		UpdateBufferSize(1536*20, 96000);
// 		m_BufStatus.PTS_In = 12345678950LL;
// 		m_BufStatus.PTS_Out = 12345678950LL;
	}

	size_t GetWidth() const  { return m_Geomery.Width; }
	size_t GetHeight() const { return m_Geomery.Heigth; }

	void SetTitle(const WTL::CString& title)
	{
		m_Title = title;
	}

	void UpdateBufferEvent(const dog::BufferEvent& bufEvent)
	{
		switch(bufEvent.Operation)
		{
		case dog::BufferEvent::DATA_IN:
			{
				m_DataInCache.push_back(bufEvent);
				if(m_DataInCache.size() > m_MaxEventQueueSize) {
					m_DataInCache.pop_front();
				}
				UpdateInputStatus(bufEvent.DataPTS, CalcDataRate(m_DataInCache));
			}
			break;
		case dog::BufferEvent::DATA_OUT:
			{
				m_DataOutCache.push_back(bufEvent);
				if(m_DataOutCache.size() > m_MaxEventQueueSize) {
					m_DataOutCache.pop_front();
				}
				UpdateOutputStatus(bufEvent.DataPTS, CalcDataRate(m_DataOutCache));
			}
			break;
		}
		UpdateBufferSize(bufEvent.BufferSize, bufEvent.Capacity);
	}

	void Paint(CDC& dc, int x, int y)
	{
		const WTL::CString txtTitle = m_Title;
		const size_t curDataSize = m_BufStatus.BufferSize;
		const size_t maxSize = m_BufStatus.Capacity;
		const REFERENCE_TIME pts_in = m_BufStatus.PTS_In;
		const REFERENCE_TIME pts_out= m_BufStatus.PTS_Out;

		const LONG hspace = 10;
		const LONG vspace = 10;
		POINT pos = {hspace*2, vspace};
		pos = Translate(pos, x, y);

		const RECT rcTitle = {0,0, 140, 15};
		const RECT rcText  = {0,0, 140, 15};

		const POINT titlePos = pos;
		const POINT ptsPos = {pos.x, pos.y + rcTitle.bottom + vspace};
		const POINT barPos = {pos.x, ptsPos.y + rcText.bottom*2 + vspace/2};

		const size_t barHeight = 20+20+3;
		const POINT ratePos= {pos.x, barPos.y + barHeight};


		RECT rcDrawText;
		rcDrawText = Translate(rcTitle, titlePos.x, titlePos.y);
		dc.DrawText(m_Title, m_Title.GetLength(), &rcDrawText, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

		WTL::CString txtPTS_in;
		WTL::CString txtPTS_out;

		const REFERENCE_TIME one_sec = 10000000;
		const DWORD pts_in_sec  = (DWORD)(pts_in/one_sec);
		const DWORD pts_out_sec = (DWORD)(pts_out/one_sec);
		const DWORD pts_in_mat  = (DWORD)(pts_in - pts_in_sec * one_sec);
		const DWORD pts_out_mat = (DWORD)(pts_out- pts_out_sec* one_sec);

		txtPTS_in.Format(_T("PTS_in = %d.%d"), pts_in_sec, pts_in_mat);
		txtPTS_out.Format(_T("PTS_out= %d.%d"), pts_out_sec, pts_out_mat);

		rcDrawText = Translate(rcText, ptsPos.x, ptsPos.y);
		dc.DrawText(txtPTS_in, txtPTS_in.GetLength(), &rcDrawText, DT_LEFT | DT_SINGLELINE | DT_VCENTER);
		rcDrawText = Translate(rcText, ptsPos.x, ptsPos.y+rcText.bottom);
		dc.DrawText(txtPTS_out, txtPTS_out.GetLength(), &rcDrawText, DT_LEFT | DT_SINGLELINE | DT_VCENTER);

		PaintProgressBarWithText(dc, curDataSize, maxSize, barPos.x, barPos.y, GreenBrush, BlackBrush);


		WTL::CString txtRx_in;
		WTL::CString txtRx_out;
		txtRx_in.Format(_T("Rx_in = %d"), m_BufStatus.Rx_In);
		txtRx_out.Format(_T("Rx_out = %d"), m_BufStatus.Rx_Out);

		rcDrawText = Translate(rcText, ratePos.x, ratePos.y);
		dc.DrawText(txtRx_in, txtRx_in.GetLength(), &rcDrawText, DT_LEFT | DT_SINGLELINE | DT_VCENTER);
		rcDrawText = Translate(rcText, ratePos.x, ratePos.y+rcText.bottom);
		dc.DrawText(txtRx_out, txtRx_out.GetLength(), &rcDrawText, DT_LEFT | DT_SINGLELINE | DT_VCENTER);


		RECT rcOuterFrame = {0,0, m_Geomery.Width, m_Geomery.Heigth};
		rcOuterFrame = Translate(rcOuterFrame, x,y);
		dc.FrameRect(&rcOuterFrame, BlackBrush);
	}
};
