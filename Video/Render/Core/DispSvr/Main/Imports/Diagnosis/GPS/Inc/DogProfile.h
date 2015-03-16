#ifndef _DIAGNOSIS_OF_GPS_PROFILE_H_
#define _DIAGNOSIS_OF_GPS_PROFILE_H_

#include <windows.h>
#include <strsafe.h>

#if defined(_WIN32)
#if _MSC_VER >= 1300
#ifndef _delayimp_h
extern "C" IMAGE_DOS_HEADER __ImageBase;
#endif
#endif
#endif //defined(_WIN32)

#define WIDEN2(x) L ## x
#define WIDEN(x) WIDEN2(x)

#define __MYFUNCTION__ WIDEN(__FUNCTION__)
#define __MYFILE__ WIDEN(__FILE__)

// namespace Diagnosis-of-GPS
namespace dog
{

struct ProfilerTrack
{
	ULONGLONG m_ullProfilerAddr;    ///< address of profile start point
	ULONGLONG m_ullFuncCallerAddr;  ///< profiled function caller address
	ULONGLONG m_ullImageBaseAddr;   ///< image base address
	DWORD m_dwThreadId;             ///< thread Id
	DWORD m_dwStartTime;            ///< start time (in ms)
	DWORD m_dwEndTime;              ///< end time (in ms)
	DWORD m_dwCategory;             ///< user define field
	DWORD m_dwSubCategory;          ///< user define field
	DWORD m_nSourceLine;            ///< info: line number in the source file
	WCHAR  m_szFunction[64];         ///< info: function name
	BYTE  Reserved[64];             ///< Reserved for future using
};

/**
 * CAutoScopeProfile automates profile data collection.
 * It will record start time when constructing, and calculate duration when destructing or EndProfile() is called.
 */
class CAutoScopeProfile
{
public:
	CAutoScopeProfile(DWORD dwCategory, DWORD dwSubCategory, const WCHAR* szFunction, const WCHAR* szSourceFile, DWORD nLine)
	{
		ZeroMemory(&m_Track, sizeof(ProfilerTrack));

		BYTE* pEBP = 0;
		__asm { mov pEBP, ebp }
		const DWORD retAddr = *((DWORD*)(pEBP+4));

		m_Track.m_ullProfilerAddr = retAddr;
		m_Track.m_ullImageBaseAddr = reinterpret_cast<ULONGLONG>(&__ImageBase);

		m_Track.m_dwCategory = dwCategory;
		m_Track.m_dwSubCategory = dwSubCategory;
		StringCchCopyW(m_Track.m_szFunction, _countof(m_Track.m_szFunction), szFunction);
		m_Track.m_nSourceLine = nLine;

		m_Track.m_dwThreadId = GetCurrentThreadId();
		m_Track.m_dwStartTime = m_Timer.Now();
	}
	~CAutoScopeProfile()
	{
		EndProfile();
	}
	void EndProfile()
	{
		if(0 == m_Track.m_dwEndTime)
		{
			m_Track.m_dwEndTime = m_Timer.Now();

			WCHAR localExecutionTimeDbgInfo[512];
			StringCchPrintfW(localExecutionTimeDbgInfo, _countof(localExecutionTimeDbgInfo),
				L"[TID:%d][CAT=%d.%d][Profiler=%#I64x][Base=%#I64x][%s:%d] (start=%d, end=%d, elapse=%d) ms\n",
				m_Track.m_dwThreadId,
				m_Track.m_dwCategory,
				m_Track.m_dwSubCategory,
				m_Track.m_ullProfilerAddr,
				m_Track.m_ullImageBaseAddr,
				m_Track.m_szFunction,
				m_Track.m_nSourceLine,
				m_Track.m_dwStartTime,
				m_Track.m_dwEndTime,
				m_Track.m_dwEndTime - m_Track.m_dwStartTime	);
			OutputDebugStringW(localExecutionTimeDbgInfo);
		}
	}

protected:
	class CTimer
	{
	public:
		CTimer() { QueryPerformanceFrequency(&m_liPerfFreq); }

		/// Returns the current time in milliseconds
		DWORD Now() const
		{
			LARGE_INTEGER liPerfNow;
			QueryPerformanceCounter(&liPerfNow);
			return(DWORD)(((liPerfNow.QuadPart) * 1000) / m_liPerfFreq.QuadPart);
		}
	protected:
		LARGE_INTEGER m_liPerfFreq;   // Counts per second
	};

	 ProfilerTrack m_Track;
	 CTimer m_Timer;
};


/// User needs to initialize profile infrastructure before starting profile
inline void InitializeProfile() {}


} // namespace dog


/// Macros for profiling
/// There two ways to use the profiler:
///
///  1. AUTO_SCOPE_PROFILE(Category, SubCategory);
///     This macro evaluates elapse time of executing from this profiler to the end of the scope which the profiler locates.
///
///  2. BEGIN_PROFILE(Category, SubCategory);
///     ...
///     END_PROFILE();
///     Evaluates elapse time of executing from BEGIN_PROFILE to END_PROFILE
///

#if defined(ENABLE_DOG_PROFILE)
#define AUTO_SCOPE_PROFILE(Category, SubCategory) dog::CAutoScopeProfile _autoScopeProfiler_(Category, SubCategory, __MYFUNCTION__, __MYFILE__, __LINE__);
#define BEGIN_PROFILE(Category, SubCategory) { AUTO_SCOPE_PROFILE(Category, SubCategory)
#define END_PROFILE() }
#else
#define AUTO_SCOPE_PROFILE(Category, SubCategory)
#define BEGIN_PROFILE(Category, SubCategory) {
#define END_PROFILE() }
#endif

#endif // _DIAGNOSIS_OF_GPS_PROFILE_H_
