#include "DOGSession.h"

#ifdef CheckPointer
#undef CheckPointer
#endif
#define CheckPointer(p,ret) {if((p)==NULL) return (ret);}

// namespace Diagnosis-of-GPS
namespace dog
{

//-----------------------------------------------------------------------------
CDOGSession::CDOGSession() : m_cRef(1), m_Dispatcher(m_SessionBuf)
{

}

CDOGSession::~CDOGSession()
{

}

//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE CDOGSession::Make(CDOGSession** pp)
{
	IUnknown* pTemp= reinterpret_cast<IUnknown*>(*pp);
	if(pTemp)
	{
		*pp= 0;
		pTemp->Release();
	}
	*pp= new CDOGSession;
	if(!(*pp))
		return E_FAIL;
	(*pp)->AddRef();
	return S_OK;
}

//-----------------------------------------------------------------------------
// implement IUnknown
ULONG STDMETHODCALLTYPE CDOGSession::AddRef()
{
	LONG lRef = 0;
	lRef = InterlockedIncrement( const_cast<LONG*>(&m_cRef) );
	return m_cRef;
}
ULONG STDMETHODCALLTYPE CDOGSession::Release()
{
	LONG lRef = InterlockedDecrement( const_cast<LONG*>(&m_cRef) );
	if (lRef == 0) {
		delete this;
		return ULONG(0);
	}
	return m_cRef;
}
HRESULT STDMETHODCALLTYPE CDOGSession::QueryInterface(REFIID riid, void ** ppv)
{
	CheckPointer(ppv, E_POINTER);
	HRESULT hr = E_NOINTERFACE;
	if ( riid == IID_IEventTracing )
	{
		*ppv = static_cast<IEventTracing*>(this);
		AddRef();
		hr = S_OK;
	}
	else if ( riid == IID_IEventConsumer)
	{
		*ppv = static_cast<IEventConsumer*>(this);
		AddRef();
		hr = S_OK;
	}
	else if ( riid == IID_IDOGSessionConfig)
	{
		*ppv = static_cast<IDOGSessionConfig*>(this);
		AddRef();
		hr = S_OK;
	}
	else if ( riid == IID_IUnknown )
	{
		*ppv = static_cast<IUnknown*>(static_cast<IEventTracing*>(this));
		AddRef();
		hr = S_OK;
	}
	return hr;
}

//-----------------------------------------------------------------------------
// implement IEventTracing
void STDMETHODCALLTYPE CDOGSession::SetupDogConfigByFile(const char* filename)
{
	m_Config.ReadFromFile(filename);
}

void STDMETHODCALLTYPE CDOGSession::SetupDogConfigByString(const char* szConfigString)
{
	m_Config.ReadFromString(szConfigString);
}

bool STDMETHODCALLTYPE CDOGSession::GetEnableFlag(DWORD dwCategory, DWORD dwSubCategory)
{
	const CDogConfig::TProperty& property = m_Config.GetProperty(CDogConfig::EVENT_LOG, dwCategory, dwSubCategory);
	return property.bEnable;
}

void STDMETHODCALLTYPE CDOGSession::WriteEvent(const DogEvent& dogEvent)
{
	const CDogConfig::TProperty& property = m_Config.GetProperty(CDogConfig::EVENT_LOG, dogEvent.Category, dogEvent.SubCategory);
	if (!property.bEnable) {
		return;
	}

	LARGE_INTEGER timeStamp;
	LARGE_INTEGER baseTime;

	TimerType timerType = static_cast<TimerType>(m_Config.GetTimerType());
	switch(timerType)
	{
	default:
	case eTimeGetTime: timeStamp.QuadPart = m_TimeGetTimer.Now(); baseTime.QuadPart =  m_TimeGetTimer.GetStartTime(); break;
	case eFreqTimer:   timeStamp.QuadPart = m_FreqTimer.Now();    baseTime.QuadPart =  m_FreqTimer.GetStartTime();    break;
	case eRdtscTimer:  timeStamp.QuadPart = m_RdtscTimer.Now();   baseTime.QuadPart =  m_RdtscTimer.GetStartTime();   break;
	}

	m_SessionBuf.Push(dogEvent, GetCurrentProcessId(), GetCurrentThreadId(), timeStamp, baseTime);
}

void STDMETHODCALLTYPE CDOGSession::RegisterListener(IEventListener* listener)
{
	m_Dispatcher.RegisterListener(listener);
	if( m_Dispatcher.GetNumOfListener() >0)
	{
		m_Dispatcher.Start();
	}
}

void STDMETHODCALLTYPE CDOGSession::UnregisterListener(IEventListener* listener)
{
	m_Dispatcher.UnregisterListener(listener);
	if( m_Dispatcher.GetNumOfListener() == 0)
	{
		HANDLE hThread = m_Dispatcher.Stop();
		if(hThread) {
			WaitForSingleObject(hThread, INFINITE);
			CloseHandle(hThread);
		}
	}
}

//-----------------------------------------------------------------------------
// implement IEventConsumer
HRESULT STDMETHODCALLTYPE CDOGSession::GetAvailableRange(LONGLONG* pUpperBound, LONGLONG* pLowerBound)
{
	CheckPointer(pUpperBound, E_POINTER);
	CheckPointer(pLowerBound, E_POINTER);

	return m_SessionBuf.GetAvailableRange(pUpperBound, pLowerBound);
}

LONGLONG STDMETHODCALLTYPE CDOGSession::GetReadIndex()
{
	return m_SessionBuf.GetReadIndex();
}

LONGLONG STDMETHODCALLTYPE CDOGSession::SetReadIndex(LONGLONG readIdx)
{
	return m_SessionBuf.SetReadIndex(readIdx);
}


DWORD STDMETHODCALLTYPE CDOGSession::GetMissCount()
{
	return m_SessionBuf.GetMissCount();
}

//-----------------------------------------------------------------------------
// implement IDOGSessionConfig
void STDMETHODCALLTYPE CDOGSession::SetTimerType(TimerType timerType)
{
	m_Config.SetTimerType(timerType);
}

CDOGSession::TimerType STDMETHODCALLTYPE CDOGSession::GetTimerType()
{
	return static_cast<TimerType>(m_Config.GetTimerType());
}


} // namespace dog

