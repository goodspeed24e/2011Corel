//  THIS SOURCE CODE IS PROPRIETARY INFORMATION BELONGING TO COREL CORP.
//  ANY USE INCLUDING BUT NOT LIMITED TO COPYING OF CODE, CONCEPTS, AND/OR
//  ALGORITHMS IS PROHIBITED EXCEPT WITH EXPRESS WRITTEN PERMISSION BY THE
//  COMPANY.
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
//  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
//  PURPOSE.
//
//  Copyright (c) 2007 - 2012  Corel Corporation.  All Rights Reserved.
//
//--------------------------------------------------------------------------

#include "stdafx.h"
#include <windows.h>
#include "CPU_Usage.h"

// using undocumented functions and structures

#define SystemBasicInformation		0
#define	SystemPerformanceInformation	2
#define SystemTimeInformation		3

#define Li2Double(x)	((double)((x).HighPart) * 4.294967296E9 + (double)((x).LowPart))

typedef struct
{
	DWORD	dwUnknown1;
	ULONG	uKeMaximumIncrement;
	ULONG	uPageSize;
	ULONG	uMmNumberOfPhysicalPages;
	ULONG	uMmLowestPhysicalPage;
	ULONG	UMmHighestPhysicalPage;
	ULONG	uAllocationGranularity;
	PVOID	pLowestUserAddress;
	PVOID	pMmHighestUserAddress;
	ULONG	uKeActiveProcessors;
	BYTE	bKeNumberProcessors;
	BYTE	bUnknown2;
	WORD	bUnknown3;
} SYSTEM_BASIC_INFORMATION;

typedef struct
{
	LARGE_INTEGER	liIdleTime;
	DWORD		dwSpare[76];
} SYSTEM_PERFORMANCE_INFORMATION;

typedef struct
{
	LARGE_INTEGER	liKeBootTime;
	LARGE_INTEGER	liKeSystemTime;
	LARGE_INTEGER	liExpTimeZoneBias;
	ULONG			uCurrentTimeZoneID;
	DWORD			dwReserved;
} SYSTEM_TIME_INFORMATION;


CCPU_Usage::CCPU_Usage(UINT nINTERVAL) : m_pfnNtQuerySystemInformation(NULL)
{
	HMODULE hNTDLL = ::GetModuleHandle(_T("ntdll"));
	if (hNTDLL)
	{
		if (NULL != ::GetProcAddress(hNTDLL, "NtQuerySystemInformationEx")) // check win7 present
			m_pfnNtQuerySystemInformation = NULL;
		else
			m_pfnNtQuerySystemInformation = (PVOID)GetProcAddress(hNTDLL, "NtQuerySystemInformation");
	}
	// NtQuerySystemInformation = (PROCNTQSI)GetProcAddress(hNTDLL, "NtQuerySystemInformation");
	m_nInterval = nINTERVAL;
}

CCPU_Usage::~CCPU_Usage()
{
	m_pfnNtQuerySystemInformation = NULL;
}

double CCPU_Usage::GetCPUUsages()
{
	double dResult = 0.0;
	if (m_pfnNtQuerySystemInformation == NULL) // call GetSystemTimes
	{
		dResult = _Win7_CPU();
	}
	else
	{
		dResult = _XP_Vista_CPU();
	}
	return dResult;
}

ULONGLONG __inline CompareFileTime ( FILETIME time1, FILETIME time2 )
{
       ULONGLONG a = ((ULONGLONG)time1.dwHighDateTime << 32) | time1.dwLowDateTime ;
       ULONGLONG b = ((ULONGLONG)time2.dwHighDateTime << 32) | time2.dwLowDateTime ;
       return   (b - a);
}

double CCPU_Usage::_Win7_CPU()
{
	BOOL bSuccess = FALSE;
	double dbIdleTime = 0.0;
	FILETIME OLD_idleTime = {0};
	FILETIME OLD_kernelTime = {0};
	FILETIME OLD_userTime = {0};

	FILETIME idleTime;
	FILETIME kernelTime;
	FILETIME userTime;

	ULONGLONG uIdleTime = 0L;
	ULONGLONG uKernelTime = 0L;
	ULONGLONG uUserTime = 0L;

	while (bSuccess == FALSE)
	{
		if (GetSystemTimes( &idleTime, &kernelTime, &userTime ))
		{
			OLD_idleTime = idleTime;
			OLD_kernelTime = kernelTime;
			OLD_userTime = userTime;

			// wait one second
			::Sleep(m_nInterval);

			if (GetSystemTimes( &idleTime, &kernelTime, &userTime ))
			{
				uIdleTime = CompareFileTime(OLD_idleTime, idleTime);
				uKernelTime = CompareFileTime(OLD_kernelTime, kernelTime);
				uUserTime = CompareFileTime(OLD_userTime, userTime);
				if ((uKernelTime + uUserTime) > 1L)
				{
					dbIdleTime = (double)(uKernelTime + uUserTime - uIdleTime) * 100.0 / (double)(uKernelTime + uUserTime);
					dbIdleTime += 0.25;
					bSuccess = TRUE;
				}
			}
		}
	}
	return dbIdleTime;
}

// NtQuerySystemInformation
// The function copies the system information of the specified type into a buffer
// NTSYSAPI 
// NTSTATUS
// NTAPI
// NtQuerySystemInformation(
//		IN UINT SystemInformationClass,		// information type
//		OUT PVOID SystemInformation,		// pointer to buffer
//		IN ULONG SystemInformationLength,	// buffer size in bytes
//		OUT PULONG ReturnLength OPTIONAL	// pointer to a 32 bit variable that
//											// receives the number of bytes written
//											// to the buffer
// );

typedef LONG (WINAPI *PROCNTQSI) (UINT, PVOID, ULONG, PULONG);

double CCPU_Usage::_XP_Vista_CPU()
{
	SYSTEM_BASIC_INFORMATION		SysBaseInfo;
	SYSTEM_TIME_INFORMATION			SysTimeInfo;
	SYSTEM_PERFORMANCE_INFORMATION	SysPerfInfo;
	LONG							status;
	LARGE_INTEGER					liOldIdleTime = {0, 0};
	LARGE_INTEGER					liOldSystemTime = {0, 0};
	double							dbIdleTime = 0.0;
	double							dbSystemTime = 0.0;
	PROCNTQSI						NtQuerySystemInformation = (PROCNTQSI)m_pfnNtQuerySystemInformation;
		if (NtQuerySystemInformation)
	{
		status = NtQuerySystemInformation(SystemBasicInformation, &SysBaseInfo, 
			sizeof(SysBaseInfo), NULL);

		if (status == NO_ERROR)
		{
			// get system time
			status = NtQuerySystemInformation(SystemTimeInformation, &SysTimeInfo, 
				sizeof(SysTimeInfo), NULL);

			if (status == NO_ERROR)
			{
				// get system idle time
				status = NtQuerySystemInformation(SystemPerformanceInformation,
					&SysPerfInfo, sizeof(SysPerfInfo), NULL);

				if (status == NO_ERROR)
				{
					liOldIdleTime = SysPerfInfo.liIdleTime;
					liOldSystemTime = SysTimeInfo.liKeSystemTime;

					// wait one second
					::Sleep(m_nInterval);

					// get new System time
					status = NtQuerySystemInformation(SystemTimeInformation, &SysTimeInfo,
						sizeof(SysTimeInfo), NULL);

					if (status == NO_ERROR)
					{
						// get new system idle time

						status = NtQuerySystemInformation(SystemPerformanceInformation,
							&SysPerfInfo, sizeof(SysPerfInfo), NULL);

						if (status == NO_ERROR)
						{
							// current value = new value - old value
							dbIdleTime = Li2Double(SysPerfInfo.liIdleTime) - Li2Double(liOldIdleTime);
							dbSystemTime = Li2Double(SysTimeInfo.liKeSystemTime) - Li2Double(liOldSystemTime);

							// currentCpuIdle = IdleTime / SystemTime;
							dbIdleTime = dbIdleTime / dbSystemTime;

							// currentCpuUsage% = 100 - (CurrentCpuIdle * 100) / NumberOfProcessors
							dbIdleTime = 100.0 - dbIdleTime * 100.0 / (double)SysBaseInfo.bKeNumberProcessors + 0.25;
						}
					}
				}
			}
		}
	}

	return dbIdleTime;
}

// Terminate a running process safely
BOOL gfnSafeTerminateProcess(HANDLE hProcess, UINT uExitCode, DWORD dwTimeout)
{
    DWORD dwTID, dwCode, dwErr = 0;
    HANDLE hProcessDup = INVALID_HANDLE_VALUE;
    HANDLE hRT = NULL;
    HINSTANCE hKernel = GetModuleHandle(_T("Kernel32"));
    BOOL bSuccess = FALSE;

    BOOL bDup = DuplicateHandle(GetCurrentProcess(), 
                                hProcess, 
                                GetCurrentProcess(), 
                                &hProcessDup, 
                                PROCESS_ALL_ACCESS, 
                                FALSE, 
                                0);

    // Detect the special case where the process is 
    // already dead...
    if ( GetExitCodeProcess((bDup) ? hProcessDup : hProcess, &dwCode) && 
         (dwCode == STILL_ACTIVE) ) 
    {
        FARPROC pfnExitProc;
           
        pfnExitProc = GetProcAddress(hKernel, "ExitProcess");

        hRT = CreateRemoteThread((bDup) ? hProcessDup : hProcess, 
                                 NULL, 
                                 0, 
                                 (LPTHREAD_START_ROUTINE)pfnExitProc,
                                 (PVOID)uExitCode, 0, &dwTID);

        if ( hRT == NULL )
            dwErr = GetLastError();
    }
    else
    {
        dwErr = ERROR_PROCESS_ABORTED;
    }


    if ( hRT )
    {
        // Must wait process to terminate to 
        // guarantee that it has exited...
        DWORD dwResult = WaitForSingleObject(
			(bDup) ? hProcessDup : hProcess,  dwTimeout);

		bSuccess = dwResult == WAIT_OBJECT_0; 

		if (!bSuccess)
			dwErr = ERROR_PROCESS_ABORTED;

        CloseHandle(hRT);
    }

    if ( bDup )
        CloseHandle(hProcessDup);

    if ( !bSuccess )
        SetLastError(dwErr);

    return bSuccess;
}

BOOL gfnIsFileExist(LPCTSTR lpszFileName)
{
	BOOL bRet = FALSE;
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	if (lpszFileName)
	{
		hFind = ::FindFirstFile(lpszFileName, &FindFileData);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			if (FindFileData.dwFileAttributes = FILE_ATTRIBUTE_NORMAL)
				bRet = TRUE;
			::FindClose(hFind);
		}
	}
	return bRet;
}
