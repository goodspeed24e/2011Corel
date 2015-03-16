#pragma once

#include "DOG.h"
#include "DogConfig.h"
#include "SessionBuffer.h"
#include "ListenThread.h"
#include "DOGTimer.h"

#include <boost/utility.hpp>

// namespace Diagnosis-of-GPS
namespace dog
{

/**
 * CDOGSession implements IEventTracing.
 * It's in charge of receiving events from event provides and dispatching events
 * to event consumers. A event provides just call WriteEvent to send events to
 * the CDOGSession, and a event consumer needs to implement IEventListener and
 * register the  listener to CDOGSession to receive events.
 *
 * CDOGSession will create or open memory mapping file as session buffer, which
 * provides a very fast and low performance impact IPC (inter-process-communication)
 * infrastructure.
 */
class CDOGSession : public IEventTracing, public IEventConsumer, public IDOGSessionConfig, public boost::noncopyable
{
public:
	CDOGSession();
	~CDOGSession();

	static HRESULT STDMETHODCALLTYPE Make(CDOGSession** pp);

	// implement IUnknown
	virtual ULONG STDMETHODCALLTYPE AddRef();
	virtual ULONG STDMETHODCALLTYPE Release();
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void ** ppv);

	// implement IEventTracing
	void STDMETHODCALLTYPE SetupDogConfigByFile(const char* filename);
	void STDMETHODCALLTYPE SetupDogConfigByString(const char* szConfigString);
	bool STDMETHODCALLTYPE GetEnableFlag(DWORD dwCategory, DWORD dwSubCategory);
	void STDMETHODCALLTYPE WriteEvent(const DogEvent& dogEvent);
	void STDMETHODCALLTYPE RegisterListener(IEventListener* listener);
	void STDMETHODCALLTYPE UnregisterListener(IEventListener* listener);

	// implement IEventConsumer
	HRESULT STDMETHODCALLTYPE GetAvailableRange(LONGLONG* pUpperBound, LONGLONG* pLowerBound);
	LONGLONG STDMETHODCALLTYPE GetReadIndex();
	LONGLONG STDMETHODCALLTYPE SetReadIndex(LONGLONG readIdx);
	DWORD STDMETHODCALLTYPE GetMissCount();

	// implement IDOGSessionConfig
	virtual void STDMETHODCALLTYPE SetTimerType(TimerType timerType);
	virtual TimerType STDMETHODCALLTYPE GetTimerType();

protected:
	LONG m_cRef;
	CSessionBuffer m_SessionBuf;
	CDogConfig m_Config;
	CListenThread m_Dispatcher;

	CTimeGetTimer m_TimeGetTimer; ///< low overhead, low-resolution
	CFreqTimer    m_FreqTimer;    ///< QueryPerformanceFrequency takes 1.927E-05 seconds as API overhead
	CRdtscTimer   m_RdtscTimer;   ///< high-resolution, low overhead
};


} // namespace dog

