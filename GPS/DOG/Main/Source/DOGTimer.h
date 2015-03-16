#pragma once

#include <windows.h>

// namespace Diagnosis-of-GPS
namespace dog
{

/// QueryPerformanceFrequency has high time resolution, but it takes 1.927E-05 seconds as API overhead
/// http://support.microsoft.com/kb/172338/en-us/
class CFreqTimer
{
public:
	CFreqTimer()
	{
		QueryPerformanceFrequency(&m_liPerfFreq);
		QueryPerformanceCounter(&m_liPerfStart);
	}

	/// Returns the current time in 100-nanoseconds
	LONGLONG Now() const
	{
		LARGE_INTEGER liPerfNow;
		QueryPerformanceCounter(&liPerfNow);
		return (liPerfNow.QuadPart * 10000000LL) / m_liPerfFreq.QuadPart;
	}

	/// Returns elapsed time from the timer creating (in 100-nanoseconds)
	LONGLONG Elapse() const 
	{
		LARGE_INTEGER liPerfNow;
		QueryPerformanceCounter(&liPerfNow);
		return ((liPerfNow.QuadPart - m_liPerfStart.QuadPart) * 10000000LL) / m_liPerfFreq.QuadPart;
	}

	/// Return start time when the timer is created
	LONGLONG GetStartTime() const
	{
		return m_liPerfStart.QuadPart;
	}

protected:
	LARGE_INTEGER m_liPerfFreq;   // Counts per second
	LARGE_INTEGER m_liPerfStart;
};


/// RDTSC - Read Time Stamp Counter
/// Pentium RDTSC cycle counter
/// http://msdn.microsoft.com/en-us/library/twchhe95.aspx
class CRdtscTimer
{
public:
	CRdtscTimer()
	{
		QueryPerformanceFrequency(&m_liPerfFreq);
		QueryPerformanceCounter(&m_liPerfStart);
		m_liRdtscStart = GetTicks();

		Sleep(10);

		LARGE_INTEGER liPerfNow;
		QueryPerformanceCounter(&liPerfNow);
		LARGE_INTEGER liRdtscNow = GetTicks();

		// get RDTSC frequency
		double elapseInv = (double)m_liPerfFreq.QuadPart / (double)(liPerfNow.QuadPart - m_liPerfStart.QuadPart) ;
		m_liRdtscFreq.QuadPart = static_cast<LONGLONG>((liRdtscNow.QuadPart - m_liRdtscStart.QuadPart) * elapseInv);
	}

	/// Returns the current time in 100-nanoseconds
	LONGLONG Now() const
	{
		LARGE_INTEGER liRdtscNow = GetTicks();
		return (liRdtscNow.QuadPart * 10000000LL) / m_liRdtscFreq.QuadPart;
	}

	/// Returns elapsed time from the timer creating (in 100-nanoseconds)
	LONGLONG Elapse() const 
	{
		LARGE_INTEGER liRdtscNow = GetTicks();
		return ((liRdtscNow.QuadPart - m_liRdtscStart.QuadPart) * 10000000LL) / m_liRdtscFreq.QuadPart;
	}

	/// Return start time when the timer is created
	LONGLONG GetStartTime() const
	{
		return m_liRdtscStart.QuadPart;
	}

protected:
	LARGE_INTEGER m_liPerfFreq;   // Counts per second
	LARGE_INTEGER m_liPerfStart;

	LARGE_INTEGER m_liRdtscStart;
	LARGE_INTEGER m_liRdtscFreq;

	#define RDTSC __asm __emit 0fh __asm __emit 031h // hack for VC++ 5.0
	static __inline LARGE_INTEGER GetTicks(void)
	{
		LARGE_INTEGER retval;
		__asm
		{
			RDTSC
			mov retval.HighPart, edx
			mov retval.LowPart, eax
		}
		return retval;
	}

	UINT GetCPUFrequency()
	{
		LARGE_INTEGER nCntFrequency;
		if(!::QueryPerformanceFrequency(&nCntFrequency))
		{
			return 0; // high-resolution performance counter
			// not supported
		}
		LARGE_INTEGER nCnt0, nCnt1;
		ULONG nTs0, nTs1;

		HANDLE hThread = ::GetCurrentThread();
		int nPriority = ::GetThreadPriority(hThread);
		::SetThreadPriority(hThread, THREAD_PRIORITY_TIME_CRITICAL);
		::QueryPerformanceCounter(&nCnt1);

		__asm
		{
			RDTSC
				mov nTs0, EAX
		}
		nCnt0.LowPart = nCnt1.LowPart;

		while(((ULONG)nCnt1.LowPart - (ULONG)nCnt0.LowPart) < 10000)
		{
			::QueryPerformanceCounter(&nCnt1);
			__asm
			{
				RDTSC
					mov nTs1, EAX
			}
		}
		::SetThreadPriority(hThread, nPriority);

		ULONG nCycles = nTs1 - nTs0;
		ULONG nTicks = (ULONG)nCnt1.LowPart - nCnt0.LowPart;
		nTicks *= 10000;
		nTicks /= (nCntFrequency.LowPart / 100);

		UINT nFrequency = nCycles / nTicks;
		return nFrequency; // MHz
	}
};


/// timeGetTime() is a very light-weight timer, it takes 0.127615 (100-nano) per call, but its resolution is 10ms.
class CTimeGetTimer
{
public:
	CTimeGetTimer()
	{
		m_StartTime = timeGetTime();
	}

	/// Returns the current time in 100-nanoseconds
	LONGLONG Now() const
	{
		return timeGetTime() * 10000LL;
	}

	/// Returns elapsed time from the timer creating (in 100-nanoseconds)
	LONGLONG Elapse() const 
	{
		return (timeGetTime() - m_StartTime) * 10000LL;
	}

	/// Return start time when the timer is created
	LONGLONG GetStartTime() const
	{
		return  m_StartTime * 10000LL;
	}

protected:
	DWORD m_StartTime;
};


} // namespace dog

