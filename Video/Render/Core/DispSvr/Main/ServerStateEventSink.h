#ifndef _DISPSVR_SERVER_STATE_EVENT_SINK_H_
#define _DISPSVR_SERVER_STATE_EVENT_SINK_H_

#include <map>
#include "DispSvr.h"
#include "Singleton.h"

namespace DispSvr
{
	class CServerStateEventSink :
		public IDisplayServerStateEventSink,
		public Singleton<CServerStateEventSink>
	{
	public:
		// IDisplayServerStateEventSink
		STDMETHOD(Notify)(DWORD eEvent, DWORD dwParam1, DWORD dwParam2);
		STDMETHOD(GetStateText)(LPTSTR pStr, UINT uLen);

		// IUknown
		STDMETHOD(QueryInterface)(REFIID riid, void **ppv);
		STDMETHODIMP_(ULONG) AddRef() { return 0; }
		STDMETHODIMP_(ULONG) Release() { return 0; }

		CServerStateEventSink();
		virtual ~CServerStateEventSink();

	protected:
		struct CRateCalculator
		{
			CRateCalculator() { Reset(); }
			void Update();
			void Reset();
			double GetInstantRate() const { return Interval2Rate(fInterframeInstant); }
			double GetAverageRate() const { return Interval2Rate(fInterframeAvg); }
			static inline double Interval2Rate(double fInterval)
			{
				return fInterval > 0 ? 1000.0 / fInterval : 0;
			}

			UINT uFramesDrawn;
			DWORD dwAccumulatedDiff;
			DWORD dwLastRender;
			double fInterframeInstant, fInterframeAvg;
			static const UINT SAMPLING_COUNT;
		};

		struct VideoSourceRate
		{
			CRateCalculator InputRate;
			CRateCalculator OutputRate;
			CRateCalculator DropRate;
		};

	protected:
		CCritSec m_cs;
		typedef std::map<DWORD, VideoSourceRate> VidSrcRateMap;
		CRateCalculator m_PresentRate;
		VidSrcRateMap m_mapVideoSourceRate;
		TCHAR m_szText[2048];
	};
}

#endif _DISPSVR_SERVER_STATE_EVENT_SINK_H_