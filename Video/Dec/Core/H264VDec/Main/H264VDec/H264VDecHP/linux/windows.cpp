//
// Intervideo, Inc. All rights reserved.  $SrcId$
//
// windows.cpp
// windows' API emulattion.
//
#include "windows.h"
#include <sys/types.h>
#include <pthread.h>
#include <errno.h>
#include <cstdio>
using namespace std;

//#define  WINEMUVERBOSE

namespace WinEmu {

class SyncObj
{
public:
	virtual ~SyncObj() {};
	virtual DWORD Wait( unsigned long msTime ) = 0;
};

class CriticalSection : public SyncObj
{
public:
	CriticalSection() {
#ifdef PSYNC
		pthread_mutex_init( &mutex_, 0 );
#endif
	};
	virtual ~CriticalSection() {
#ifdef PSYNC
		Unlock();
		pthread_mutex_destroy( &mutex_ );
#endif
	}

	virtual DWORD Wait( unsigned long msTime )
	{
#ifdef PSYNC
		if( msTime != INFINITE ) {
			fprintf( stderr, "CriticalSection::Wait( %d ms ) missing impl.\n",
										msTime);
			return WAIT_ABANDONED;
		}
		Lock();
#endif

		return WAIT_OBJECT_0;
	}

#if 0
typedef struct
{
  int __mutex_reserved;               /* Reserved for future use */
  int __mutex_count;                  /* Depth of recursive locking */
  _pthread_descr __mutex_owner;       /* Owner thread (if recursive or errcheck) */
  int __mutex_kind;                   /* Mutex kind: fast, recursive or errcheck */
  struct _pthread_fastlock __mutex_lock; /* Underlying fast lock */
} pthread_mutex_t;

#endif

	void Lock()
	{
#ifdef PSYNC
#ifdef WINEMUVERBOSE
		int ret = pthread_mutex_trylock( &mutex_ );
		if( ret == EBUSY ) {
			fprintf( stderr, "[%p:%d] CriticalSection::Lock EBUSY \n", this, getpid()); fflush(stderr);
		} else 
			pthread_mutex_unlock( &mutex_ );
			fprintf( stderr, "2:[%p:%d] ownder=%p, cnt:%d, kind:%d\n", 
						this, getpid(),
						mutex_.__mutex_owner,
						mutex_.__mutex_count,
						mutex_.__mutex_kind
						);
#endif
		pthread_mutex_lock( &mutex_ );
#endif
	}
	void Unlock()
	{
#ifdef PSYNC
	pthread_mutex_unlock( &mutex_ );
#endif
	}

protected:

	pthread_mutex_t mutex_;
};


class LinuxThread : public SyncObj
{
public:
	LinuxThread()
	{
	}
	~LinuxThread()
	{
	}
	DWORD Wait( unsigned long msTime )
	{
		fprintf(stderr,"LinuxThread::Wait() in H264dec is not supported now!\n");
		return 0;
	}
	int Init(unsigned int (*start_address)(void *), 
					   void *arglist) 
	{
		if (!start_address)
			return -1;
		else
		{
			pthread_attr_init(&m_attr);
			if (pthread_create(&m_thread_id, &m_attr, (void * (*)(void *))start_address, arglist) != 0)
			{
				printf("Thread create failed!\n");
				return -1;
			}
			else
				pthread_detach(m_thread_id);
		}

		return m_thread_id;
	}
private:
	pthread_t m_thread_id;
	pthread_attr_t m_attr;
};

class Event : public SyncObj
{
public:
	Event( BOOL bManualReset ) : bManualReset_( bManualReset )
	{
#ifdef PSYNC
		pthread_mutex_init( &mutex_, 0 );
		pthread_cond_init( &cond_, 0 );
#endif
		bSignaled_ = FALSE;
	}
	~Event()
	{
#ifdef PSYNC
		pthread_cond_destroy( &cond_ );
		pthread_mutex_destroy( &mutex_ );
#endif
		bSignaled_ = FALSE;
	}

	DWORD Wait( unsigned long msTime )
	{
		int ret=0;
#ifdef PSYNC
		pthread_mutex_lock( &mutex_ );

		if( bSignaled_ ) {
			pthread_mutex_unlock( &mutex_ );
			if( !bManualReset_ )
				Reset();
			return WAIT_OBJECT_0;
		}

		if( msTime == INFINITE )
			ret = pthread_cond_wait( &cond_, &mutex_ );
		else {
			struct timespec future;
			unsigned long msCur = GetTickCount() + msTime;
			future.tv_sec = msCur/1000;
			future.tv_nsec = (msCur%1000)*1000;
			ret = pthread_cond_timedwait( &cond_, &mutex_, &future );
		}
		pthread_mutex_unlock( &mutex_ );
#endif

		if( !bManualReset_ )
			Reset();

		return ret==0 ? WAIT_OBJECT_0 : WAIT_TIMEOUT;
	}

	void Set()
	{
#ifdef PSYNC
		pthread_mutex_lock( &mutex_ );
		if( bSignaled_ == FALSE )
			pthread_cond_signal( &cond_ );
		bSignaled_ = TRUE;
		pthread_mutex_unlock( &mutex_ );
#endif
	}

	// reset conditional variable. don't care error on cond_wait_timeout.
	void Reset()
	{
#ifdef PSYNC
		struct timespec future;
		unsigned long msCur = GetTickCount() + 5;
		future.tv_sec = msCur/1000;
		future.tv_nsec = (msCur%1000)*1000;

		pthread_mutex_lock( &mutex_ );
		// make conditional variable unsignealled.
		if( bSignaled_ )
			pthread_cond_timedwait( &cond_, &mutex_, &future );
		bSignaled_ = FALSE;
		pthread_mutex_unlock( &mutex_ );
#endif
	}

protected:
	pthread_cond_t cond_;
	pthread_mutex_t mutex_;
	BOOL bManualReset_;
	BOOL bSignaled_;
};


pthread_mutex_t mutex_ = PTHREAD_MUTEX_INITIALIZER;

} // namespace WinEmu


WINBASEAPI
VOID
WINAPI
InitializeCriticalSection(
    OUT LPCRITICAL_SECTION lpCriticalSection
    )
{
	lpCriticalSection->LockSemaphore = (HANDLE) new WinEmu::CriticalSection();
	//fprintf(stderr, "[%d:%p] InitializeCriticalSection\n", GetTickCount(), lpCriticalSection->LockSemaphore);
}

WINBASEAPI
VOID
WINAPI
EnterCriticalSection(
    IN OUT LPCRITICAL_SECTION lpCriticalSection
    )
{
	WinEmu::CriticalSection * crit =
		(WinEmu::CriticalSection *)lpCriticalSection->LockSemaphore;
//	fprintf(stderr, "[%d:%p] EnterCriticalSection\n", GetTickCount(), crit);
	crit->Lock();	
}

WINBASEAPI
VOID
WINAPI
LeaveCriticalSection(
    IN OUT LPCRITICAL_SECTION lpCriticalSection
    ) 
{
	WinEmu::CriticalSection * crit =
		(WinEmu::CriticalSection *)lpCriticalSection->LockSemaphore;
	//fprintf(stderr, "[%d:%p] LeaveCriticalSection\n", GetTickCount(), crit);
	crit->Unlock();	
}

WINBASEAPI
VOID
WINAPI
DeleteCriticalSection(
    IN OUT LPCRITICAL_SECTION lpCriticalSection
    ) 
{
	WinEmu::CriticalSection * crit =
		(WinEmu::CriticalSection *)lpCriticalSection->LockSemaphore;
	//jfprintf(stderr, "[%d:%p] DeleteCriticalSection\n", GetTickCount(), crit);
	delete crit;
}


//
// FIXEME(07-23-2005, seungkoo): use atomic_interlocked_in/decrement function.
//
WINBASEAPI
LONG
WINAPI
InterlockedIncrement(
    IN OUT LONG volatile *lpAddend
    ) 
{ 
	long retVal = 0;
    	__asm {
    		mov ecx, lpAddend;
    		mov eax, 1;
		lock xadd dword ptr [ecx], eax;
		inc eax;
		mov retVal, eax;
	}
	return retVal;

}

WINBASEAPI
LONG
WINAPI
InterlockedDecrement(
    IN OUT LONG volatile *lpAddend
    ) 
{ 
	long retVal = 0;
    	__asm {
    		mov ecx, lpAddend;
    		mov eax, 0FFFFFFFFh;
		lock xadd dword ptr [ecx], eax;
		dec eax;
		mov retVal, eax;
	}
	return retVal;
}

WINBASEAPI
BOOL
WINAPI
CloseHandle(
    IN OUT HANDLE hObject
    ) 
{ 
	WinEmu::SyncObj * obj =
		(WinEmu::SyncObj *) hObject;
	delete obj;
	return TRUE;
}

WINBASEAPI
HANDLE
WINAPI
CreateEvent(
    IN LPSECURITY_ATTRIBUTES lpEventAttributes,
    IN BOOL bManualReset,
    IN BOOL bInitialState,
    IN LPCSTR lpName
    ) 
{ 
	WinEmu::Event * evt = new WinEmu::Event( bManualReset );
	if( bInitialState )
		evt->Set();
	else
		evt->Reset();
	return (HANDLE)evt;
}

WINBASEAPI
BOOL
WINAPI
SetEvent(
    IN HANDLE hEvent
    ) 
{ 
	WinEmu::Event * evt = (WinEmu::Event *)hEvent;
	evt->Set();
	return TRUE;
}

WINBASEAPI
BOOL
WINAPI
ResetEvent(
    IN HANDLE hEvent
    ) 
{ 
	WinEmu::Event * evt = (WinEmu::Event *)hEvent;
	evt->Reset();
	return TRUE;
}

WINBASEAPI
DWORD
WINAPI
WaitForSingleObject(
    IN HANDLE hHandle,
    IN DWORD dwMilliseconds
    ) 
{ 
	WinEmu::SyncObj * obj =
		(WinEmu::SyncObj *) hHandle;
	return obj->Wait( dwMilliseconds );
}

WINBASEAPI
DWORD
WINAPI
_beginthreadex(void *security,
				unsigned unused_stack_size,
				unsigned (*start_address)(void *),
				void *arglist, 
				unsigned initflag,
				unsigned *thrdaddr)
{
	int threadid = 0;
	WinEmu::LinuxThread *thread = new WinEmu::LinuxThread;
	if (thread)
	{
		threadid = thread->Init(start_address, arglist);
		if (threadid != -1)
		{
			if (thrdaddr)
				*thrdaddr = threadid;
			return (unsigned long)thread;
		}
		else
			return 0;
	}
	return (DWORD)(thread);
}

WINBASEAPI
VOID
WINAPI
_endthreadex(unsigned retval)
{
	pthread_exit((void *)(long)retval);
}

#include <sys/time.h>
WINBASEAPI
DWORD
WINAPI
GetTickCount(
    VOID
    ) 
{ 
	struct timeval cur;
	
	if(gettimeofday( &cur, 0 )!=0)
		memset( &cur, 0x0, sizeof(struct timeval) );

	return cur.tv_sec*1000 + cur.tv_usec/1000;
}


