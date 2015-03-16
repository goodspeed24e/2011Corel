//=============================================================================
//	THIS SOURCE CODE IS PROPRIETARY INFORMATION BELONGING TO INTERVIDEO, INC.
// 	ANY USE INCLUDING BUT NOT LIMITED TO COPYING OF CODE, CONCEPTS, AND/OR
//	ALGORITHMS IS PROHIBITED EXCEPT WITH EXPRESS WRITTEN PERMISSION BY THE 
//	COMPANY.
//
// 	THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
// 	KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// 	IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
// 	PURPOSE.
//
// 	Copyright (c) 1998 - 2000  InterVideo Corporation.  All Rights Reserved.
//
//-----------------------------------------------------------------------------

// 
// UTILcrit.h
//
// Generic utility functions. The principal one here
// deals with critical-section automation classes
//
//

#ifndef _UTILCRIT_H
#define _UTILCRIT_H

//#define DEBUG_SPINLOCK 1
#ifdef DEBUG_SPINLOCK
#include "../LIBUTIL/utildbg.h"
#endif

class CUTILcrit
	{
	CRITICAL_SECTION m_cs;
	long m_recursion; //It seems WIN98 doesn't have RecursionCount in CRITICAL_SECTION
    DWORD   m_dwOwnerThdId;

public:
	CUTILcrit() : m_dwOwnerThdId(0)
		{
		InterlockedExchange(&m_recursion, 0);
		InitializeCriticalSection(&m_cs);
		}
	~CUTILcrit()
		{
		DeleteCriticalSection(&m_cs);
		}
	void Lock()
		{
		EnterCriticalSection(&m_cs);
		InterlockedIncrement(&m_recursion);
#ifdef _DEBUG
		m_dwOwnerThdId = GetCurrentThreadId();
#endif // #ifdef _DEBUG
		}
	void Unlock()
		{
		LeaveCriticalSection(&m_cs);
		InterlockedDecrement(&m_recursion);
#ifdef _DEBUG
		if(!m_recursion)
			m_dwOwnerThdId = 0;
#endif // #ifdef _DEBUG
		}
	int GetRecursion()	// 0 implies no recursion, >0 implies recursion
		{
		return m_recursion-1;
		}
	};

class CUTILautolock
	{
	CUTILcrit	*m_pCrit;
	
	public:
	CUTILautolock(CUTILcrit* pCrit)
		{
		m_pCrit = pCrit;
		if(m_pCrit)
			m_pCrit->Lock();
		}
	~CUTILautolock()
		{
		if(m_pCrit)
			m_pCrit->Unlock();
		}
	};

class CUtilAutoLock2
	{
	private:
	CUTILcrit	*m_pCrit;
	public:
	CUtilAutoLock2() : m_pCrit(new CUTILcrit)
		{
		m_pCrit->Lock();
		}
	~CUtilAutoLock2()
		{
		m_pCrit->Unlock();
		delete m_pCrit;
		}
	};

class TLock
{
public:
	TLock(LONG* pLock){ m_pLock= pLock; InterlockedIncrement(m_pLock); }
	~TLock(){ InterlockedDecrement(m_pLock); }
	bool IsLocked(){ return *m_pLock > 1; }
protected:
	LONG *m_pLock;
};



class CUTILautomutex
	{
	HANDLE hmutex;
	
	public:
	CUTILautomutex(HANDLE hmutex)
		{
		if(hmutex && WaitForSingleObject(hmutex,INFINITE)==WAIT_OBJECT_0)
			this->hmutex = hmutex;
		else
			this->hmutex = 0;
		}
	~CUTILautomutex()
		{
		if(hmutex)
			ReleaseMutex(hmutex);
		}
	inline BOOL IsLocked()
		{
		return hmutex!=0;
		}
	};

class CUtilAutoSemaphore
	{
	HANDLE m_hSemaphore;
	CUtilAutoSemaphore() {}
public:
	CUtilAutoSemaphore(const HANDLE hSemaphore)
		{
		if(hSemaphore && WAIT_OBJECT_0 == WaitForSingleObject(hSemaphore,INFINITE))
			m_hSemaphore = hSemaphore;
		else
			m_hSemaphore = 0;
		}
	~CUtilAutoSemaphore()
		{
		if(m_hSemaphore)
			ReleaseSemaphore(m_hSemaphore,1,0);
		}
	inline BOOL IsLocked()
		{
		return m_hSemaphore != 0;
		}
	};
// old implementation, will not lock if alternating threads!
class CUTILautospinlock
	{
	LPLONG	m_pVal;
	LONG	m_OldVal;

	public:
	CUTILautospinlock(LPLONG pVal, unsigned long count = 0xffffffff, unsigned long sleepms = 5)
		{
		long id;
		unsigned long i;
		
		m_pVal = 0;
#ifdef __linux__
		id = pthread_self() + 1;    // first thread is 0, we're unlikely to get -1
#else
		id = GetCurrentThreadId();
#endif
		if(id==0)
			return;	// don't bother to lock.
		i = 0;
		do
			{
			m_OldVal = InterlockedExchange(pVal, id);
			if(m_OldVal==0 || m_OldVal==id)
				{
				m_pVal = pVal;
				break;
				}
			Sleep(sleepms);
			}
		while(++i<count);
		}
	~CUTILautospinlock()
		{
		if(m_pVal)
			InterlockedExchange(m_pVal, m_OldVal);
		}
	BOOL IsLocked()
		{
		return m_pVal!=0;
		}
	};

// CUTILautospinlock_nr and CUTILautospinlock_r are newer implementations
// intended to replace CUTILautospinlock
class CUTILautospinlock_nr  /* non-reentrant */
	{
	LPLONG  m_pVal;

	public:
	CUTILautospinlock_nr(LPLONG pVal, unsigned long count = 0xffffffff,
						 unsigned long sleepms = 5, unsigned long *residual = NULL)
		{
		unsigned long i, flag;

		m_pVal = 0;
		i = 0;
		do
			{
			flag = InterlockedExchange(pVal, 1);
			if(flag==0)
				{
				m_pVal = pVal;
				break;
				}
			Sleep(sleepms);
			}
		while(++i<=count);

		// will not get here if count is 0xffffffff
		if (residual)
			*residual = count - i;
		}
	~CUTILautospinlock_nr()
		{
		if (IsLocked())
			InterlockedExchange(m_pVal, 0);
		}
	BOOL IsLocked()
		{
		return m_pVal!=0;
		}
	};

class CUTILautospinlock_r    /* reentrant */
	{
	// m_pVal[0] == mutex flag 0 or 1; m_pVal[1] ==	thread ID of owner or 0 if none
	LPLONG  m_pVal; 
    LONG    m_OldVal;

	public:
	CUTILautospinlock_r(__int64 *pllVal, unsigned long count = 0xffffffff, 
						  unsigned long sleepms = 5)
		{
		long id;
		unsigned long i;

		LPLONG pVal = (LPLONG)pllVal;
		m_pVal = 0;
#ifdef __linux__
		id = pthread_self() + 1;  // first thread is 0, we're unlikely to get -1
#else
		id = GetCurrentThreadId();
#endif
		if(id==0)
			return; // don't bother to lock.
		i = 0;
		do
			{
				{
				CUTILautospinlock_nr  lock(pVal,count,sleepms,&count);
				if (!lock.IsLocked())
					break;                  // timed out already

				m_OldVal = pVal[1];         // assume this is atomic copy
				if(m_OldVal==0 || m_OldVal==id)
					{
					m_pVal = pVal;
					pVal[1] = id;           // assume this is atomic copy
#ifdef DEBUG_SPINLOCK
					DP("**threadid = %d, obtaining lock\n", (int)pthread_self());
#endif
					break;
					}
				}
				Sleep(sleepms);
			}
		while(++i<=count);
		}
	~CUTILautospinlock_r()
		{
#ifdef DEBUG_SPINLOCK
		DP("**threadid = %d, releasing lock\n", (int)pthread_self());
#endif
		if(m_pVal)
			m_pVal[1] = m_OldVal;  // assume this is atomic copy - can do this since we are the owner.
		}
	BOOL IsLocked()
		{
		return m_pVal!=0;
		}
	};

//
// CUTILrwcrit 
//
// is a fast multiple reader single writer critical section
// supporting reentrant readers and reentrant writers.
// 
// APPLICATION NOTES:
//
// 1) YOU CANNOT AUTOUPGRADE LOCK TYPE: 
// do not lock as a reader, then lock as a writer.
// You must release all reader locks and then lock
// as a writer.
// [Note: autolock upgrades are a bit more work, requiring
// saved thread id/count.  I don't know if it is worth the
// overhead.]
//
// 2) Nested read locks have the same thread affinity as
// critical sections. This true with nested write locks.
//
// 3) A write lock can be subsequently read locked
// and write locked. Avoid doing this as a general practice.
// Your locking privilege will remain that of a writer.
// [ Permitted example:
//  write lock -> read lock -> write lock -> read lock
//  followed by
//  read unlock->write unlock->read unlock->write unlock]
//
// 4) The default handling is spinlocking for writelock.
// That will make your code VERY efficient for readers. 
// Locking and unlocking will be about 120 cycles on a P3
// unless you seize a write lock with active readers and
// then it is moderately efficient spinning.
//
// 5) If you pass in a spinms of 0, then you will use
// event handling, which is much slower for readers but
// will not spin.  This is for apps which use many
// write locks. Locking and unlocking will be about
// 2000 cycles on a P3; hence is NOT recommended
// for the many read locks, few write locks case.
//
// 6) Thread fairness is not guaranteed for the write 
// lock.
//

class CUTILrwcrit
	{
	int spinms;  // variable can be read anytime;
	int readers; // variable can be read/written iff var lock held
	int writers; // variable can be read/written iff write lock held
	CUTILcrit read;		// allows exclusion while reading "readers" var.
	CUTILcrit write;	// handles basic thread locking
	HANDLE noreaders;	// if no spin locking, we need event that indicates no readers.

	public:
	CUTILrwcrit(int spinms = 1)
		{
		readers = 0;
		writers = 0;
		noreaders = 0;
		if(spinms==0)
			noreaders = CreateEvent(0,TRUE,TRUE,0);
		this->spinms = spinms;
		}
	~CUTILrwcrit()
		{
		if(noreaders)
			CloseHandle(noreaders);
		}
	void ReadLock()
		{
		CUTILautolock lck1(&write);
		CUTILautolock lck2(&read);

		if(writers==0)
			{// I don't own exclusive write lock, must increment #readers
			if(readers==0 && noreaders)
				ResetEvent(noreaders);
			readers++;
			}
#ifdef _DEBUG
		else
			OutputDebugString(TEXT("CUTILrw warning: read lock taken after write lock\n"));
#endif
		}
	void ReadUnlock()
		{
		CUTILautolock lck1(&read);

		if(readers>0)
			{// only decrement read count if I don't own exclusive write lock
			readers--;
			if(readers==0 && noreaders)
				SetEvent(noreaders);
			}
#ifdef _DEBUG
		else
			OutputDebugString(TEXT("CUTILrw warning: unpaired read unlock, or read unlock called during write lock\n"));
#endif
		}
	void WriteLock()
		{
		write.Lock();
		CUTILautolock lck1(&read);

		while(readers)
			{
			read.Unlock();
			write.Unlock();	// have to unlock for reentrant readers
			if(noreaders)
				WaitForSingleObject(noreaders,INFINITE);
			else
				Sleep(spinms);
			write.Lock();
			read.Lock();
			}
		writers = 1;		// i own the exclusive write lock now.
		}
	void WriteUnlock()
		{
		writers = 0;		// i no longer own the exclusive write lock.
		write.Unlock();
		}
	};

class CUTILautoreadlock
	{
	CUTILrwcrit	*m_pCrit;
	
	public:
	CUTILautoreadlock(CUTILrwcrit* pCrit)
		{
		m_pCrit = pCrit;
		if(m_pCrit)
			m_pCrit->ReadLock();
		};
	~CUTILautoreadlock()
		{
		if(m_pCrit)
			m_pCrit->ReadUnlock();
		};
	};

class CUTILautowritelock
	{
	CUTILrwcrit	*m_pCrit;
	
	public:
	CUTILautowritelock(CUTILrwcrit* pCrit)
		{
		m_pCrit = pCrit;
		if(m_pCrit)
			m_pCrit->WriteLock();
		};
	~CUTILautowritelock()
		{
		if(m_pCrit)
			m_pCrit->WriteUnlock();
		};
	};

class CUTILautoincr
	{
	long *m_ptr;

	public:
	CUTILautoincr(long *ptr)
		{
		m_ptr = ptr;
		if(m_ptr)
			InterlockedIncrement(m_ptr);	// should be interlocked increment
		}
	~CUTILautoincr()
		{
		if(m_ptr)
			InterlockedDecrement(m_ptr);	// should be interlocked decrement
		}
	};

#endif //_UTILCRIT_H
