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

extern CBrush GreenBrush;
extern CBrush BlackBrush;
extern CBrush GrayBrush;

#include <math.h>
#include <deque>

#include <boost/utility.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/shared_ptr.hpp>

#include "DogVariant.h"


class CCurveDraw
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

	static float LinearInterp(float val, float min, float max, float t0, float t1)
	{
		return (1.0f - (val-min)/(max-min)) * (t1-t0);
	}

	struct MyVariant : public dog::Variant
	{
		MyVariant() {}
		MyVariant(const dog::Variant& var) : dog::Variant(var) {}
		MyVariant operator= (const dog::Variant& var) { (*(dog::Variant*)this) = var;  return *this; }

		MyVariant operator += (int val) { *this = *this + val;  return *this; }
		MyVariant operator -= (int val) { *this = *this - val;  return *this; }

		MyVariant operator + (int val) const
		{
			switch(type)
			{
			default:
			case VT_INT:       return iVal   + val; break;
			case VT_UINT:      return uiVal  + val; break;
			case VT_LONGLONG:  return llVal  + val; break;
			case VT_ULONGLONG: return ullVal + val; break;
			case VT_FLOAT:     return fVal   + val; break;
			case VT_DOUBLE:    return dblVal + val; break;
			}
			return *this;
		}

		MyVariant operator - (int val) const
		{
			switch(type)
			{
			default:
			case VT_INT:       return iVal   - val; break;
			case VT_UINT:      return uiVal  - val; break;
			case VT_LONGLONG:  return llVal  - val; break;
			case VT_ULONGLONG: return ullVal - val; break;
			case VT_FLOAT:     return fVal   - val; break;
			case VT_DOUBLE:    return dblVal - val; break;
			}
			return *this;
		}

		operator const int() const                { return iVal;   }
		operator const unsigned int() const       { return uiVal;  }
		operator const long long() const          { return llVal;  }
		operator const unsigned long long() const { return ullVal; }
		operator const float() const              { return fVal;   }
		operator const double() const             { return dblVal; }

		std::wstring ToWString() const
		{
			switch(type)
			{
			case VT_INT:       return (boost::wformat(L"%d") % iVal).str();
			case VT_UINT:      return (boost::wformat(L"%d") % uiVal).str();
			case VT_LONGLONG:  return (boost::wformat(L"%d") % llVal).str();
			case VT_ULONGLONG: return (boost::wformat(L"%d") % ullVal).str();
			case VT_FLOAT:     return (boost::wformat(L"%f") % fVal).str();
			case VT_DOUBLE:    return (boost::wformat(L"%f") % dblVal).str();
			default:
				return L"";
			}
			return L"";
		}

		double ToDouble() const
		{
			switch(type)
			{
			default:
			case VT_INT:       return (double)iVal;
			case VT_UINT:      return (double)uiVal;
			case VT_LONGLONG:  return (double)llVal;
			case VT_ULONGLONG: return (double)ullVal;
			case VT_FLOAT:     return (double)fVal;
			case VT_DOUBLE:    return dblVal;
			}
			return 0.0f;
		}

		const bool operator < (const MyVariant& var) const
		{
			if(type == var.type)
			{
				switch(type)
				{
				default:
				case VT_INT:       return iVal   < var.iVal;   break;
				case VT_UINT:      return uiVal  < var.uiVal;  break;
				case VT_LONGLONG:  return llVal  < var.llVal;  break;
				case VT_ULONGLONG: return ullVal < var.ullVal; break;
				case VT_FLOAT:     return fVal   < var.fVal;   break;
				case VT_DOUBLE:    return dblVal < var.dblVal; break;
				}
			}
			else
			{
				return ToDouble() < var.ToDouble();
			}
			return false;
		}

		const bool operator == (const MyVariant& var) const
		{
			if(type == var.type) {return ullVal == var.ullVal; }
			else {return ToDouble() == var.ToDouble(); }
		}
	};


protected:
	WTL::CString  m_Title;

	const size_t m_MaxQueueSize;
	std::deque<MyVariant> m_DataQueue;
	MyVariant m_MinVal, m_MaxVal;

	MyVariant m_MinLine, m_MaxLine;
	std::deque<POINT> m_DrawCache;
	LONG m_NumOfCacheData;
	bool m_bCacheUpdated; // cache's dirty flag
	bool m_bMinMaxUpdated;

	size_t m_ReceivedDataNum;

	boost::recursive_mutex* m_pMtxDataQueue;
	boost::recursive_mutex* m_pMtxDrawCache;

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

	POINT m_InnerFramePos;
	RECT  m_rcInnerFrame;


protected:
	void RefreshMinMax()
	{
		boost::recursive_mutex::scoped_lock lock(*m_pMtxDataQueue);

		if(!m_DataQueue.empty()) {
			m_MinVal = m_MaxVal = m_DataQueue[0];
		}

		for(size_t i=0; i<m_DataQueue.size(); ++i)
		{
			m_MinVal = (std::min)(m_MinVal, m_DataQueue[i]);
			m_MaxVal = (std::max)(m_MaxVal, m_DataQueue[i]);
		}
	}

	template<typename T>
	void RefreshCacheImpl(LONG x, LONG y, LONG h)
	{
		m_DrawCache.resize(m_MaxQueueSize);

		T minVal = static_cast<T>(m_MinLine);
		T maxVal = static_cast<T>(m_MaxLine);
		const T len = maxVal-minVal;

		for(size_t i=0; i<m_DataQueue.size(); ++i)
		{
			POINT posVal = {i, 0};
			const T val = static_cast<T>(m_DataQueue[i]);

			if(len > 0){
				posVal.y = (LONG)((1.0f - (val-minVal)/(float)len) * h);
			} else {
				posVal.y = 0;
			}
			m_DrawCache[i] = Translate(posVal, x, y);
			InterlockedIncrement(&m_NumOfCacheData);
			m_NumOfCacheData = (std::min)(m_NumOfCacheData, (LONG)m_MaxQueueSize);
		}
	}

	void RefreshCache(LONG x, LONG y, LONG h)
	{
		boost::recursive_mutex::scoped_lock lock1(*m_pMtxDrawCache);
		boost::recursive_mutex::scoped_lock lock2(*m_pMtxDataQueue);

		if(m_DataQueue.empty()) {
			return;
		}

		switch(m_DataQueue[0].type)
		{
		default:
		case dog::Variant::VT_INT:       RefreshCacheImpl<int>(x, y, h);                break;
		case dog::Variant::VT_UINT:      RefreshCacheImpl<unsigned int>(x, y, h);       break;
		case dog::Variant::VT_LONGLONG:  RefreshCacheImpl<long long>(x, y, h);          break;
		case dog::Variant::VT_ULONGLONG: RefreshCacheImpl<unsigned long long>(x, y, h); break;
		case dog::Variant::VT_FLOAT:     RefreshCacheImpl<float>(x, y, h);              break;
		case dog::Variant::VT_DOUBLE:    RefreshCacheImpl<double>(x, y, h);             break;
		}
	}

	void CheckAndUpdateCache(const POINT& innerFramePos)
	{
		const MyVariant oldMinLine = m_MinLine;
		const MyVariant oldMaxLine = m_MaxLine;

		if(!m_bMinMaxUpdated && m_ReceivedDataNum>=m_MaxQueueSize)
		{
			RefreshMinMax();
			m_bMinMaxUpdated = true;
		}

		m_MinLine = m_MinVal;
		m_MaxLine = m_MaxVal;

		if(m_MinLine == m_MaxLine)
		{
			m_MinLine -= 50;
			m_MaxLine += 50;
		}
		m_bCacheUpdated &= (oldMinLine==m_MinLine && oldMaxLine==m_MaxLine);

		if(!m_bCacheUpdated)
		{
			const LONG heightOfCurve = m_rcInnerFrame.bottom;
			RefreshCache(innerFramePos.x, innerFramePos.y, heightOfCurve);
			m_bCacheUpdated = true;
		}
	}

	void Init()
	{
		m_Title = "Curve Draw";
		m_DrawCache.resize(m_MaxQueueSize);

		m_pMtxDataQueue = new boost::recursive_mutex;
		m_pMtxDrawCache = new boost::recursive_mutex;

		RECT rcInner = {0, 0, m_Geomery.Width, 70};
		m_rcInnerFrame = rcInner;

// 		const double PI = 3.14159265358979323846;
// 		for(int i=0; i<180+20; ++i)
// 		//for(int i=0; i<80; ++i)
// 		{
// 			const double val = sin(PI * i/45.0f);
// 			PushBackData( val );
	}

public:
	CCurveDraw() : m_MaxQueueSize(180), m_bCacheUpdated(false), m_bMinMaxUpdated(false), m_ReceivedDataNum(0), m_NumOfCacheData(0), m_pMtxDataQueue(NULL), m_pMtxDrawCache(NULL)
	{
		Init();
	}

	CCurveDraw(const CCurveDraw& const) : m_MaxQueueSize(180), m_bCacheUpdated(false), m_bMinMaxUpdated(false), m_ReceivedDataNum(0), m_NumOfCacheData(0), m_pMtxDataQueue(NULL), m_pMtxDrawCache(NULL)
	{
		Init();
	}

	~CCurveDraw()
	{
		if(m_pMtxDataQueue)
		{
			delete m_pMtxDataQueue;
			m_pMtxDataQueue = NULL;
		}

		if(m_pMtxDrawCache)
		{
			delete m_pMtxDrawCache;
			m_pMtxDrawCache = NULL;
		}
	}



	size_t GetWidth() const  { return m_Geomery.Width; }
	size_t GetHeight() const { return m_Geomery.Heigth; }

	void PushBackData(const dog::Variant& var)
	{
		if(m_DataQueue.empty()) {
			m_MinVal = m_MaxVal = var;
		}

		{
			boost::recursive_mutex::scoped_lock lock(*m_pMtxDataQueue);
			m_DataQueue.push_back(var);
			if(m_DataQueue.size() > m_MaxQueueSize) {
				m_DataQueue.pop_front();
			}
		}

		m_MinVal = (std::min)(m_MinVal, (MyVariant)var);
		m_MaxVal = (std::max)(m_MaxVal, (MyVariant)var);
		m_bCacheUpdated = false;

		// update min, max values for every 30 data
		++m_ReceivedDataNum;
		if(0 == m_ReceivedDataNum % 30) {
			m_bMinMaxUpdated = false;
		}
	}

	void SetTitle(const WTL::CString& title)
	{
		m_Title = title;
	}

	void Paint(CDC& dc, int x, int y)
	{
		const WTL::CString txtTitle = m_Title;

		const LONG hspace = 10;
		const LONG vspace = 10;
		POINT pos = {hspace, vspace};
		pos = Translate(pos, x, y);

		const RECT rcTitle = {0,0, 140, 15};
		const RECT rcText  = {0,0, 140, 15};
		const RECT rcTxtVal= {0,0, (m_rcInnerFrame.right/2)-1, 15};
		const POINT titlePos = pos;
		const POINT txtMaxPos = {x, titlePos.y+rcTitle.bottom};
		const POINT innerFramePos = {x, txtMaxPos.y+rcText.bottom};
		const POINT txtMinPos = {x, innerFramePos.y+m_rcInnerFrame.bottom};
		const POINT txtValPos = {x+m_rcInnerFrame.right/2, txtMinPos.y};

		m_InnerFramePos = innerFramePos;

		RECT rcDrawText;
		rcDrawText = Translate(rcTitle, titlePos.x, titlePos.y);
		dc.DrawText(m_Title, m_Title.GetLength(), &rcDrawText, DT_CENTER | DT_SINGLELINE | DT_VCENTER);


		WTL::CString txtMax;
		WTL::CString txtMin;
		txtMax.Format(_T("max= %s"), m_MaxLine.ToWString().c_str());
		txtMin.Format(_T("min= %s"), m_MinLine.ToWString().c_str());

		dc.SetTextColor( (COLORREF) RGB(0xaa,0xaa,0xaa) );
		rcDrawText = Translate(rcText, txtMaxPos.x, txtMaxPos.y);
		dc.DrawText(txtMax, txtMax.GetLength(), &rcDrawText, DT_LEFT | DT_SINGLELINE | DT_VCENTER);
		rcDrawText = Translate(rcText, txtMinPos.x, txtMinPos.y);
		dc.DrawText(txtMin, txtMin.GetLength(), &rcDrawText, DT_LEFT | DT_SINGLELINE | DT_VCENTER);
		dc.SetTextColor( (COLORREF) RGB(0, 0, 0) );


		RECT rcDrawInnerFrame = m_rcInnerFrame;
		rcDrawInnerFrame = Translate(rcDrawInnerFrame, innerFramePos.x, innerFramePos.y);
		dc.FrameRect(&rcDrawInnerFrame, GrayBrush);


		CheckAndUpdateCache(innerFramePos);

		if(!m_DrawCache.empty())
		{
			boost::recursive_mutex::scoped_lock lock(*m_pMtxDrawCache);
			dc.MoveTo(m_DrawCache[0]);
			for(size_t i=1; i<m_NumOfCacheData && i<m_DrawCache.size(); ++i)
			{
				POINT tmp = m_DrawCache[i];
				if(m_DrawCache[i].x !=0 && m_DrawCache[i].y !=0 )
				{
					dc.LineTo(m_DrawCache[i]);
				}
			}
		}



		WTL::CString txtVal;
		MyVariant val;
		{
			boost::recursive_mutex::scoped_lock lock(*m_pMtxDataQueue);
			if(!m_DataQueue.empty()) {
				val = m_DataQueue.back();
			}
		}
		txtVal.Format(_T("val= %s"), val.ToWString().c_str());
		rcDrawText = Translate(rcTxtVal, txtValPos.x, txtValPos.y);
		dc.DrawText(txtVal, txtVal.GetLength(), &rcDrawText, DT_RIGHT | DT_SINGLELINE | DT_VCENTER);


		RECT rcOuterFrame = {0,0, m_Geomery.Width, m_Geomery.Heigth};
		rcOuterFrame = Translate(rcOuterFrame, x,y);
		dc.FrameRect(&rcOuterFrame, BlackBrush);
	}
};
