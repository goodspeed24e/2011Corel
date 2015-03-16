#ifndef WINDOWS_H_
#define WINDOWS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#ifndef __cdecl
#define __cdecl
#define _cdecl
#endif
#ifndef __fastcall
#define __fastcall
#define _fastcall
#endif
#ifndef __stdcall
#define __stdcall
#define _stdcall
#endif

#if !defined(__INTEL_COMPILER)
#define __forceinline inline
#define __declspec(x)
#endif

#ifndef UINT
typedef unsigned int UINT;
#endif
#ifndef UCHAR
typedef unsigned int UCHAR;
#endif
#ifndef BOOL
typedef int BOOL;
#endif
#ifndef BYTE
typedef unsigned char BYTE;
#endif
#ifndef DWORD
typedef unsigned long DWORD;
#endif
#ifndef WORD
typedef unsigned short WORD;
#endif
typedef void *HANDLE;
#define VOID  void
#ifndef LPVOID
typedef void *LPVOID;
#endif
#ifndef PVOID
typedef void *PVOID;
#endif
#ifndef PBYTE
typedef unsigned char *PBYTE;
#endif
#ifndef LPBYTE
typedef unsigned char *LPBYTE;
#endif
#ifndef ULONG
typedef unsigned long ULONG;
#endif
#ifndef LONG
typedef long LONG;
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef HRESULT
typedef long HRESULT;
#endif
#ifndef IN
#define IN
#endif
#ifndef OUT
#define OUT
#endif

#define VirtualAlloc(lpAddress, dwSize, flAllocationType, flProtect) \
valloc(dwSize)
#define VirtualFree(lpAddress,  dwSize, dwFreeType) \
free(lpAddress)
#define ZeroMemory(Destination,  Length) memset(Destination, 0x0, Length)
#define _vsnprintf snprintf

#if __GNUC__ < 4  
#define _mm_malloc(size, alignment)      memalign(alignment, size)
#define _mm_free(ptr)                    free(ptr)
#else
#include <mm_malloc.h>
#endif
#define _aligned_malloc(size,alignment)  _mm_malloc(size,alignment)
#define _aligned_free(ptr)               _mm_free(ptr)

#define _HRESULT_TYPEDEF_(_sc) ((HRESULT)_sc)
#define E_POINTER                        _HRESULT_TYPEDEF_(0x80004003L)
#define E_FAIL                           _HRESULT_TYPEDEF_(0x80004005L)
#define E_NOTIMPL                        _HRESULT_TYPEDEF_(0x80004001L)
#define E_OUTOFMEMORY                    _HRESULT_TYPEDEF_(0x8007000EL)
#define S_OK                                   ((HRESULT)0x00000000L)
#define S_FALSE                                ((HRESULT)0x00000001L)


#define STDMETHODIMP            HRESULT STDMETHODCALLTYPE
#define STDMETHODIMP_(type)     type STDMETHODCALLTYPE
#define STDMETHODCALLTYPE       
#define STDMETHODVCALLTYPE      

// for now


#define WINAPI
#define WINBASEAPI

typedef unsigned long ULONG_PTR, *PULONG_PTR;

typedef struct _RTL_CRITICAL_SECTION {
    //PRTL_CRITICAL_SECTION_DEBUG DebugInfo;

    //
    //  The following three fields control entering and exiting the critical
    //  section for the resource
    //

    LONG LockCount;
    LONG RecursionCount;
    HANDLE OwningThread;        // from the thread's ClientId->UniqueThread
    HANDLE LockSemaphore;
    ULONG_PTR SpinCount;        // force size on 64-bit systems when packed
} RTL_CRITICAL_SECTION, *PRTL_CRITICAL_SECTION;

typedef RTL_CRITICAL_SECTION CRITICAL_SECTION;
typedef PRTL_CRITICAL_SECTION PCRITICAL_SECTION;
typedef PRTL_CRITICAL_SECTION LPCRITICAL_SECTION;

WINBASEAPI
VOID
WINAPI
InitializeCriticalSection(
    OUT LPCRITICAL_SECTION lpCriticalSection
    );

WINBASEAPI
VOID
WINAPI
EnterCriticalSection(
    IN OUT LPCRITICAL_SECTION lpCriticalSection
    );

WINBASEAPI
VOID
WINAPI
LeaveCriticalSection(
    IN OUT LPCRITICAL_SECTION lpCriticalSection
    );

WINBASEAPI
VOID
WINAPI
DeleteCriticalSection(
    IN OUT LPCRITICAL_SECTION lpCriticalSection
    );

WINBASEAPI
LONG
WINAPI
InterlockedIncrement(
    IN OUT LONG volatile *lpAddend
    );

WINBASEAPI
LONG
WINAPI
InterlockedDecrement(
    IN OUT LONG volatile *lpAddend
    );

typedef struct _SECURITY_ATTRIBUTES
    {
    DWORD nLength;
    /* [size_is] */ LPVOID lpSecurityDescriptor;
    BOOL bInheritHandle;
    } 	SECURITY_ATTRIBUTES;

typedef struct _SECURITY_ATTRIBUTES *PSECURITY_ATTRIBUTES;

typedef struct _SECURITY_ATTRIBUTES *LPSECURITY_ATTRIBUTES;

typedef const char *LPCSTR, *PCSTR;

WINBASEAPI
HANDLE
WINAPI
CreateEvent(
    IN LPSECURITY_ATTRIBUTES lpEventAttributes,
    IN BOOL bManualReset,
    IN BOOL bInitialState,
    IN LPCSTR lpName
    );

WINBASEAPI
BOOL
WINAPI
CloseHandle(
    IN OUT HANDLE hObject
    );

WINBASEAPI
BOOL
WINAPI
SetEvent(
    IN HANDLE hEvent
    );

WINBASEAPI
BOOL
WINAPI
ResetEvent(
    IN HANDLE hEvent
    );

#define INFINITE            0xFFFFFFFF  // Infinite timeout

WINBASEAPI
DWORD
WINAPI
WaitForSingleObject(
    IN HANDLE hHandle,
    IN DWORD dwMilliseconds
    );

WINBASEAPI
DWORD
WINAPI
_beginthreadex(void *security,
				unsigned unused_stack_size,
				unsigned (*start_address)(void *),
				void *arglist, 
				unsigned initflag,
				unsigned *thrdaddr);


WINBASEAPI
VOID
WINAPI
_endthreadex(unsigned retval);

inline VOID Sleep(int dwMilliseconds)
{
	sleep(dwMilliseconds/1000);
}

#define OutputDebugString printf

#define SUCCEEDED(Status) ((HRESULT)(Status) >= 0)
#define FAILED(Status) ((HRESULT)(Status)<0)

WINBASEAPI
DWORD
WINAPI
GetTickCount(
    VOID
    );

#include <strings.h>

#define stricmp strcasecmp

#define WAIT_ABANDONED 	0x00000080L
#define WAIT_OBJECT_0 	0x00000000L
#define WAIT_TIMEOUT 	0x00000102L

#ifdef __cplusplus
}
#endif

#endif // WINDOWS_H_

