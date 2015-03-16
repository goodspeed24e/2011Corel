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

#ifndef _INC_CPU_USAGE_INCLUDE
#define _INC_CPU_USAGE_INCLUDE

class CCPU_Usage
{
public:
	CCPU_Usage(UINT nINTERVAL = 1000);
	virtual ~CCPU_Usage();

	double GetCPUUsages();

protected:
	double _Win7_CPU();
	double _XP_Vista_CPU();
	PVOID m_pfnNtQuerySystemInformation;
	UINT  m_nInterval;
};

BOOL gfnSafeTerminateProcess(HANDLE hProcess, UINT uExitCode, DWORD dwTimeout);
BOOL gfnIsFileExist(LPCTSTR lpszFileName);

#endif // _INC_CPU_USAGE_INCLUDE

