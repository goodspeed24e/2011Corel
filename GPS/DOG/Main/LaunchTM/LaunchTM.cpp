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
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <io.h>
#include <time.h>
#include <Commdlg.h>
#include "RegUtil.h"
#include "CPU_Usage.h"

#include <vector>
using namespace std;  

#include "./CXml/MarkupMSXML.h"
#include "./MD5/md5.h"

char HelpText[] = 
"LaunchTM - Launch Time Monitor tool (C)Corel 2010\n\n"
"Syntax: LaunchTM [/AUTO_CLOSE:xxx] [/C] [/CMD:\"command_line\"] [/D:KeyValue] [/K:KeyFile] \n"
"                 [/L:\"path\"] [/O:OutputXMLFile] [/R:n] [/W:n] LaunchedFile \n\n"
"  LaunchedFile          : specify launched program file.\n"
"  /AUTO_CLOSE:xxx       : specify how long (xxx ms) to auto close this test process (TerminateProcess).\n"
"  /C                    : specify enable cold-startup mode.\n"
"  /CMD:\"command_line\" : specify launch command.\n"
"  /D:KeyName=KeyValue   : specify watch key value, if key name not found, key value as default.\n"
"  /K:KeyFile            : specify file which contains watch key value list. (ex:DOG_DEFINED_CHECK_POINT( KeyName, KeyValue)).\n"
"  /L:\"path\"           : specify start-up location path.\n"
"  /O:OutputXMLFile      : specify output log file name with XML format.\n"
"  /R:n                  : specify how many time to do-loop test, default value would be 1.\n"
"  /W:n                  : specify wait for start until system cpu usage lower than n%%.\n"
"  /H                    : print help information. \n";

struct trWATCH_OBJECT
{
	TCHAR szEventName[MAX_PATH];
	TCHAR szEventKey[MAX_PATH];
	LARGE_INTEGER liWatchTime;
};

struct MD5_CHECKSUM
{
	short ID;
	unsigned char signature[16];
};

trWATCH_OBJECT gWatchObjTable[] =
{
	{ _T("WinDVD10_WinMain"), _T("{7921B9AD-B319-4f02-8CC1-AC22ABF85566}"), {0} },
	{ _T("WinDVD10_createLogicLayer"), _T("{19256E49-39DB-4247-BCE4-35462F4D7B68}"), {0} },
	{ _T("WinDVD10_Logiclayer_CGPIControl"), _T("{798ECD66-3758-4dd0-98A4-95DC3CE52921}"), {0} },
	{ _T("WinDVD10_Logiclayer_InitGPICtrl"), _T("{15D220C2-CEEF-4ea9-8422-6F038E92E823}"), {0} },
};

typedef vector<trWATCH_OBJECT*> WATCH_OBJ_VECTOR;
typedef vector<trWATCH_OBJECT*>::iterator  WATCH_OBJ_IT;

#define SIZE_OF_WATCH_OBJECT  (sizeof(gWatchObjTable) / sizeof(trWATCH_OBJECT))
// #define MAX_SIZE_OF_WATCH_OBJECT  SIZE_OF_WATCH_OBJECT+2
#define MAX_SIZE_OF_WATCH_OBJECT 202
#define MAX_COUNT_OF_CPU_SAMPLE 20
#define INFO_BUFFER_SIZE 32767

// Global variables
WATCH_OBJ_VECTOR g_listWatcher;

TCHAR g_szLaunchPATH[MAX_PATH];
TCHAR g_szLocationPATH[MAX_PATH];
TCHAR g_szOutputLogFile[MAX_PATH];
TCHAR g_szWatchKeyFile[MAX_PATH];
TCHAR g_szDefinedKey[MAX_PATH * 2];
DWORD g_dwAutoCloseTimeout = 0;
BOOL  g_bColdStartupMode = FALSE;
UINT  g_nRepeateCount = 1;
UINT  g_nWaitForCPU_UnderValue = 0;
double g_nCPUSample[MAX_COUNT_OF_CPU_SAMPLE];

// Watch Handles
HANDLE ghWatchHandle[MAX_SIZE_OF_WATCH_OBJECT];

BOOL CalculateFileMD5(LPCTSTR lpszFileName, MD5_CHECKSUM *pReturnChecksum)
{
	BOOL bRet = FALSE;
	if ((lpszFileName) && (pReturnChecksum))
	{
		int nOrgIOMode = 0;
		int j;
		errno_t err;
		FILE *pRead;
		unsigned char buffer[16384];
		struct MD5Context md5c;

		pReturnChecksum->ID = 5;

		if ( (err = _tfopen_s(&pRead, lpszFileName, _T("rb"))) != 0 )
		{
			_tprintf( _T("ERROR: The file [%s] cannot be opened!.\n"), g_szWatchKeyFile);
		}
		else
		{
#ifdef _WIN32

	    /** Warning!  On systems which distinguish text mode and
		binary I/O (MS-DOS, Macintosh, etc.) the modes in the open
        	statement for "in" should have forced the input file into
        	binary mode.  But what if we're reading from standard
		input?  Well, then we need to do a system-specific tweak
        	to make sure it's in binary mode.  While we're at it,
        	let's set the mode to binary regardless of however fopen
		set it.

		The following code, conditional on _WIN32, sets binary
		mode using the method prescribed by Microsoft Visual C 7.0
        	("Monkey C"); this may require modification if you're
		using a different compiler or release of Monkey C.	If
        	you're porting this code to a different system which
        	distinguishes text and binary files, you'll need to add
		the equivalent call for that system. */

	    nOrgIOMode = _setmode(_fileno(pRead), _O_BINARY);
#endif
			MD5Init(&md5c);

			while ((j = (int)fread(buffer, 1, sizeof(buffer), pRead)) > 0) {
				MD5Update(&md5c, buffer, (unsigned) j);
			}
#ifdef _WIN32
		_setmode(_fileno(pRead), nOrgIOMode);
#endif
	    	fclose(pRead);
			
			MD5Final(pReturnChecksum->signature, &md5c);

			bRet = TRUE;
		}

	}
	return bRet;
}

// Browser executable file name
BOOL gfnGetFileName(TCHAR *pszOutput, size_t dwMaxOutputSize)
{
	OPENFILENAME ofn;       // common dialog box structure
	TCHAR szFile[260];       // buffer for file name
	TCHAR szActivePath[MAX_PATH];
	HWND hwnd = ::GetActiveWindow();              // owner window
	::GetCurrentDirectory(MAX_PATH, szActivePath);

	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	ofn.lpstrFile = szFile;
	// Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
	// use the contents of szFile to initialize itself.
	ofn.lpstrFile[0] = _T('\0');
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = _T("All\0*.*\0Executable files\0*.exe\0");
	ofn.nFilterIndex = 2;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	ofn.lpstrInitialDir = szActivePath;
	// Display the Open dialog box. 
	BOOL bRet = ::GetOpenFileName(&ofn);
	if (bRet == TRUE && pszOutput != NULL)
	{
		//lstrcpy(pszOutput, ofn.lpstrFile);
		_tcscpy_s(pszOutput, dwMaxOutputSize, ofn.lpstrFile);
	}
	return bRet;
}

int ProcessCommandLine(int argc, _TCHAR *argv[])
{
	int nRet = 0;
	int nTemp = 0;
    int i;

	memset(&g_szLaunchPATH[0], 0, sizeof(TCHAR)*MAX_PATH);
	memset(&g_szLocationPATH[0], 0, sizeof(TCHAR)*MAX_PATH);
	memset(&g_szDefinedKey[0], 0, sizeof(TCHAR)*MAX_PATH*2);

	TCHAR szBuf[MAX_PATH];
	TCHAR *pCmd = NULL;
	TCHAR *pPtr = NULL;
	TCHAR *pszTok1 = NULL;
	TCHAR *pszTok2 = NULL;
	TCHAR *pszNextTok = NULL;
	TCHAR szParser[] = _T("= ,\t\n");
	szBuf[0] = _T('\0');

    for ( i=1; i < argc; i++ )
    {
		pCmd = _tcsdup(argv[i]);
        _tcsupr(pCmd);
        
        // Is it a switch character?
        if ( (*pCmd == _T('-')) || (*pCmd == _T('/')) )
        {
			if (pCmd[1] == _T('C') && pCmd[2] == _T('M') && pCmd[3] == _T('D') && pCmd[4] == _T(':')) {
				// search base folder
				pPtr = argv[i];
				pPtr += 5;
				lstrcpy(g_szLaunchPATH, pPtr);
				nRet++;
            }
			else if (pCmd[1] == _T('A') && pCmd[2] == _T('U') && pCmd[3] == _T('T') && pCmd[4] == _T('O') &&
					pCmd[5] == _T('_') && pCmd[6] == _T('C') && pCmd[7] == _T('L') && pCmd[8] == _T('O') &&
					pCmd[9] == _T('S') && pCmd[10] == _T('E') && pCmd[11] == _T(':'))
			{
				pPtr = argv[i];
				pPtr += 12;
				nTemp = ::_tstoi(pPtr);
				if (nTemp > 0) {
					g_dwAutoCloseTimeout = nTemp;
					nRet++;
				}
				nRet++;
			}
			else if (pCmd[1] == _T('C') && pCmd[2] == _T('\0')) {   
				// Enable cold startup test
				g_bColdStartupMode = TRUE;
			}
			else if (pCmd[1] == _T('D') && pCmd[2] == _T(':')) {
				// hard-define key name
				pPtr = argv[i];
				pPtr += 3;
				pszTok1 = _tcstok_s(pPtr, szParser, &pszNextTok);
				if (pszTok1 != NULL)
				{
					pszTok2 = _tcstok_s(NULL, szParser, &pszNextTok);
					if (pszTok2 != NULL)
					{
						trWATCH_OBJECT *pNewItem = new trWATCH_OBJECT;
						if (pNewItem)
						{
							memset((void*)pNewItem, 0, sizeof(trWATCH_OBJECT));
							_tcscpy_s(pNewItem->szEventName, MAX_PATH, pszTok1);
							_tcscpy_s(pNewItem->szEventKey, MAX_PATH, pszTok2);
							_tcscat(g_szDefinedKey, pNewItem->szEventName);
							_tcscat(g_szDefinedKey, _T("="));
							_tcscat(g_szDefinedKey, pNewItem->szEventKey);
							_tcscat(g_szDefinedKey, _T(";"));
							g_listWatcher.push_back(pNewItem);
							nRet++;
						}
					}
				}
			}
			else if (pCmd[1] == _T('K') && pCmd[2] == _T(':')) {
				// watch key file name
				pPtr = argv[i];
				pPtr += 3;
				nRet++;
				lstrcpy(g_szWatchKeyFile, pPtr);
			}
            else if (pCmd[1] == _T('L') && pCmd[2] == _T(':')) {
				// what is the customized VCExt
				pPtr = argv[i];
				pPtr += 3;
				nRet++;
				lstrcpy(g_szLocationPATH, pPtr);
			}
			else if (pCmd[1] == _T('O') && pCmd[2] == _T(':')) {
				// output xml file
				pPtr = argv[i];
				pPtr += 3;
				nRet++;
				lstrcpy(g_szOutputLogFile, pPtr);
			}
			else if (pCmd[1] == _T('W') && pCmd[2] == _T(':')) {
				// atoi
				pPtr = argv[i];
				pPtr += 3;
				nTemp = ::_tstoi(pPtr);
				if (nTemp > 0) {
					g_nWaitForCPU_UnderValue = nTemp;
					nRet++;
				}
			}
			else if (pCmd[1] == _T('R') && pCmd[2] == _T(':')) {
				// output xml file
				pPtr = argv[i];
				pPtr += 3;
				nTemp = ::_tstoi(pPtr);
				if (nTemp > 1) {
					g_nRepeateCount = nTemp;
					nRet++;
				}
			}
			else if (pCmd[1] == _T('H') && pCmd[2] == _T('\0'))
			{
				printf( HelpText );
				if (pCmd) {
					free(pCmd);
					pCmd = NULL;
				}
				nRet = (-1);
				break;
			}
			else
			{
				_tprintf(_T("ERROR: Cannot recognize '%s'!\n"), argv[i]);
				printf( HelpText );
				if (pCmd) {
					free(pCmd);
					pCmd = NULL;
				}
				nRet = (-1);
				break;
			}
        }
        else    // Not a switch character.  Must be the filename
        {
			_tcscpy_s(g_szLaunchPATH, MAX_PATH, argv[i]);
			nRet++;
        }

		if (pCmd) {
			free(pCmd);
			pCmd = NULL;
		}
    }

	return nRet;
}

#define DOG_DEFINED_MACRO_NAME _T("DOG_DEFINED_CHECK_POINT")

UINT ImportWatchKeyFile()
{
	UINT nRet = 0;
	FILE *pRead;
	errno_t err;
	TCHAR szline[MAX_PATH];
	TCHAR *pSearchNode;
	trWATCH_OBJECT *pNewItem;
	TCHAR *pszTok1 = NULL;
	TCHAR *pszTok2 = NULL;
	TCHAR *pszNextTok = NULL;
	TCHAR szParser[] = _T("() ,\t\n");

	if ( (err = _tfopen_s(&pRead, g_szWatchKeyFile, _T("r"))) != 0 )
	{
		_tprintf( _T("ERROR: The file [%s] cannot be opened!.\n"), g_szWatchKeyFile);
	}
	else
	{
		while( _fgetts(szline, MAX_PATH, pRead) != NULL )
		{
			pSearchNode = _tcsstr(szline, DOG_DEFINED_MACRO_NAME);
			if (pSearchNode)
			{
				pSearchNode += _tcslen(DOG_DEFINED_MACRO_NAME);
				pszTok1 = _tcstok_s(pSearchNode, szParser, &pszNextTok);
				if (pszTok1 != NULL)
				{
					pszTok2 = _tcstok_s(NULL, szParser, &pszNextTok);
					if ((pszTok1) && (pszTok2))
					{
						pNewItem = new trWATCH_OBJECT;
						if (pNewItem)
						{
							memset((void*)pNewItem, 0, sizeof(trWATCH_OBJECT));
							_tcscpy_s(pNewItem->szEventName, MAX_PATH, pszTok1);
							_tcscpy_s(pNewItem->szEventKey, MAX_PATH, pszTok2);
							g_listWatcher.push_back(pNewItem);
							_tprintf( _T("Import: \"%s\"<==>\"%s\"\n"), pszTok1, pszTok2);
							nRet++;
						}
					}
				}
			}

		}
		fclose(pRead);
	}
	return nRet;
}

/*
BOOL gfnPrepareUserEvent(HANDLE *pEventList, int nCount, SECURITY_ATTRIBUTES *pSA)
{
	if (NULL == pEventList) return FALSE;

	int i;
	HANDLE *pCreateEvent = pEventList;

	for (i = 0; i < nCount; i++)
	{
		*pCreateEvent = ::CreateEvent(pSA, FALSE, FALSE, gWatchObjTable[i].szEventKey);
		if (NULL == *pCreateEvent)
			goto error_out;
		pCreateEvent++;
	}

	return TRUE;

error_out:

	pCreateEvent = pEventList;
	for (i = 0; i < nCount; i++)
	{
		if (*pCreateEvent) { 
			::CloseHandle(*pCreateEvent);
			*pCreateEvent = NULL;
		}
	}
	return FALSE;
}
*/
BOOL gfnPrepareUserEvent(HANDLE *pEventList, SECURITY_ATTRIBUTES *pSA)
{
	if (NULL == pEventList) return FALSE;

	int i;
	int nCount = g_listWatcher.size();
	HANDLE *pCreateEvent = pEventList;
	trWATCH_OBJECT *pObject;

	for (WATCH_OBJ_IT IT = g_listWatcher.begin(); IT != g_listWatcher.end (); IT++)
	{
		pObject = *IT;
		if (pObject)
		{
			*pCreateEvent = ::CreateEvent(pSA, FALSE, FALSE, pObject->szEventKey);
			if (NULL == *pCreateEvent)
				goto error_out;
			pCreateEvent++;
		}
	}

	return TRUE;

error_out:

	pCreateEvent = pEventList;
	for (i = 0; i < nCount; i++)
	{
		if (*pCreateEvent) { 
			::CloseHandle(*pCreateEvent);
			*pCreateEvent = NULL;
		}
	}
	return FALSE;
}


BOOL gfnCloseUserEvent(HANDLE *pEventList, int nCount)
{
	if (NULL == pEventList) 
		return FALSE;

	int i;
	HANDLE *pCreateEvent = pEventList;
	for (i = 0; i < nCount; i++)
	{
		::CloseHandle(*pCreateEvent);
		*pCreateEvent = NULL;
		pCreateEvent++;
	}

	return TRUE;
}

DWORD WINAPI ThreadWaitForProcessIdle( LPVOID lpParam ) 
{ 
	HANDLE hIdelEvent = (HANDLE)lpParam;
	if (ghWatchHandle[0] != NULL)
	{
		DWORD dwRet = ::WaitForInputIdle( ghWatchHandle[0], INFINITE );
		if (0 == dwRet)
		{
			if (hIdelEvent)
				::SetEvent(hIdelEvent);
		}
		else
		{
#ifdef _DEBUG
			_tprintf(_T("WARN: WaitForInputIdle failed.\n"));
#else
			::OutputDebugString(_T("WARN: WaitForInputIdle failed.\n"));
#endif
		}
	}
	return 0;
}

#define KEY_LAUNCHTM_PATH       _T("Path")
#define KEY_LAUNCH_TARGET       _T("LaunchTarget")
#define KEY_LAUNCH_TARGET_PATH  _T("LaunchLocationPath")
#define KEY_HARD_DEFINED_KEY    _T("KeyDefined")
#define KEY_WATCH_KEY_FILE      _T("WatchKeyFile")
#define KEY_OUTPUT_LOG_FILE     _T("OutputLogFile")
#define KEY_AUTOCLOSE_TIMEOUT   _T("AutoCloseTimeOut")
#define KEY_WAIT_CPU_VALUE      _T("WaitCPU")
#define KEY_REPEAT_COUNT        _T("Repeat")
#define KEY_LOOP_INDEX          _T("LoopIndex")


int _tmain(int argc, _TCHAR* argv[])
{
	TCHAR szCurrentWorkingFolder[MAX_PATH];
	TCHAR szRunBeginTime[128];
	TCHAR szRunEndTime[128];
	CMarkupMSXML Xmldoc;
	MD5_CHECKSUM CurrentPrjMD5;
	CRegUtil ProductKey(HKEY_CURRENT_USER, _T("LaunchTM"), _T("1.0"));

	int nRet = 0;
	int nTriggered = 0;
	int nRepeateCount = 0;
	int nLoopIndex = 0;
	DWORD dwEvent = 0;
	DWORD dwWatchHandleCount = 0;
	DWORD dwWaitProcessTimeOut = INFINITE;
	BOOL bDetected = FALSE;
	BOOL bOutputXMLOpened = FALSE;
	BOOL bExecuteReboot = FALSE;
	BOOL bColdStart_Reboot = FALSE;
	LARGE_INTEGER liPerfFreq = {0};
	LARGE_INTEGER liPerfStart = {0};
	LARGE_INTEGER liPerfIdle = {0};
	LARGE_INTEGER liPerfObejct = {0};

	g_szLaunchPATH[0] = _T('\0');
	g_szLocationPATH[0] = _T('\0');
	g_szOutputLogFile[0] = _T('\0');
	g_szDefinedKey[0] = _T('\0');
	g_szWatchKeyFile[0] = _T('\0');
	g_listWatcher.clear();

	if (!Xmldoc.IsInited())
	{
		_tprintf( _T("ERROR: Please check MSXML runtime dll installed!.\n"));
		nRet = 1;
		goto error_out;
	}

	// Cold startup Check
	if ((2 == argc) && (0 == _tcsicmp(argv[1], _T("-c")) || 0 == _tcsicmp(argv[1], _T("/c"))))
	{
		g_bColdStartupMode = TRUE;
		// Retrieve command-line from registry
		ProductKey.GetStringValue(KEY_LAUNCHTM_PATH, szCurrentWorkingFolder);
		if (_tcslen(szCurrentWorkingFolder) > 0)
			::SetCurrentDirectory(szCurrentWorkingFolder);

		ProductKey.GetStringValue(KEY_LAUNCH_TARGET, g_szLaunchPATH);
		ProductKey.GetStringValue(KEY_LAUNCH_TARGET_PATH, g_szLocationPATH);
		ProductKey.GetStringValue(KEY_HARD_DEFINED_KEY, g_szDefinedKey);
		if (_tcslen(g_szDefinedKey) > 0) // import pre-defined key
		{
			TCHAR *pPtr = g_szDefinedKey;
			TCHAR *pszTok1 = NULL;
			TCHAR *pszTok2 = NULL;
			TCHAR *pszNextTok = NULL;
			TCHAR szParser1[] = _T("= ,;\t");
			TCHAR szParser2[] = _T(" ,;\t");
			trWATCH_OBJECT *pNewItem = NULL;

			pszTok1 = _tcstok_s(pPtr, szParser1, &pszNextTok);
			while (pszTok1 != NULL)
			{
				pszTok2 = _tcstok_s(NULL, szParser2, &pszNextTok);
				if (pszTok1 != NULL && pszTok2 != NULL)
				{
					pNewItem = new trWATCH_OBJECT;
					if (pNewItem)
					{
						memset((void*)pNewItem, 0, sizeof(trWATCH_OBJECT));
						_tcscpy_s(pNewItem->szEventName, MAX_PATH, pszTok1);
						_tcscpy_s(pNewItem->szEventKey, MAX_PATH, pszTok2);
						g_listWatcher.push_back(pNewItem);
						pNewItem = NULL;
					}
					pszTok1 = _tcstok_s(NULL, szParser1, &pszNextTok);
				}
			}

		}
		ProductKey.GetStringValue(KEY_WATCH_KEY_FILE, g_szWatchKeyFile);
		ProductKey.GetStringValue(KEY_OUTPUT_LOG_FILE, g_szOutputLogFile);
		// use dwEvent as temp dword
		ProductKey.GetDWORDValue(KEY_AUTOCLOSE_TIMEOUT, &dwEvent);
		g_dwAutoCloseTimeout = dwEvent;
		ProductKey.GetDWORDValue(KEY_WAIT_CPU_VALUE, &dwEvent);
		g_nWaitForCPU_UnderValue = dwEvent;
		ProductKey.GetDWORDValue(KEY_REPEAT_COUNT, &dwEvent);
		g_nRepeateCount = dwEvent;
		ProductKey.GetDWORDValue(KEY_LOOP_INDEX, &dwEvent);
		nLoopIndex = dwEvent;
		dwEvent = 0;
		// If nothing found from registry, show up 'no execution command' found
		if (!gfnIsFileExist(g_szLaunchPATH))
		{
			_tprintf( _T("ERROR: Launch file [%s] not found!.\n"), g_szLaunchPATH);
			nRet = 1;
			goto error_out;
		}
	}
	else
	{
		if (0 > ProcessCommandLine(argc, argv))
		{
			nRet = 1;
			goto error_out;
		}

		if (_tcslen(g_szLaunchPATH) == 0)
		{
			if (!gfnGetFileName(g_szLaunchPATH, MAX_PATH))
			{
				printf( HelpText );
				nRet = 1;
				goto error_out;
			}
		}
		if (!gfnIsFileExist(g_szLaunchPATH))
		{
			_tprintf( _T("ERROR: Launch file [%s] not found!.\n"), g_szLaunchPATH);
			nRet = 1;
			goto error_out;
		}

		if (g_bColdStartupMode == TRUE)
		{
			// write cold-startup registry
			::GetCurrentDirectory(MAX_PATH, szCurrentWorkingFolder);
			ProductKey.SetStringValue(KEY_LAUNCHTM_PATH, szCurrentWorkingFolder);
			ProductKey.SetStringValue(KEY_LAUNCH_TARGET, g_szLaunchPATH);
			if (_tcslen(g_szLocationPATH) > 0)
				ProductKey.SetStringValue(KEY_LAUNCH_TARGET_PATH, g_szLocationPATH);
			ProductKey.SetStringValue(KEY_HARD_DEFINED_KEY, g_szDefinedKey);
			if (_tcslen(g_szOutputLogFile) > 0)
				ProductKey.SetStringValue(KEY_OUTPUT_LOG_FILE, g_szOutputLogFile);
			ProductKey.SetDWORDValue(KEY_AUTOCLOSE_TIMEOUT, g_dwAutoCloseTimeout);
			ProductKey.SetDWORDValue(KEY_WAIT_CPU_VALUE, g_nWaitForCPU_UnderValue);
			ProductKey.SetDWORDValue(KEY_REPEAT_COUNT, g_nRepeateCount);
			ProductKey.SetDWORDValue(KEY_LOOP_INDEX, 0);
			bExecuteReboot = TRUE;
			bColdStart_Reboot = TRUE;
			// goto error_out;
		}
	}

	// Parsing watch key file for adding new key define
	if (_tcslen(g_szWatchKeyFile) > 0 && gfnIsFileExist(g_szWatchKeyFile))
	{
		if (0 == ImportWatchKeyFile())
		{
			_tprintf( _T("WARNING: There is no key imported from file [%s] !.\n"), g_szWatchKeyFile);
		}
		else
		{
			if (g_bColdStartupMode == TRUE)
			{
				ProductKey.SetStringValue(KEY_WATCH_KEY_FILE, g_szWatchKeyFile);
			}
		}
	}

	// Check watch key count is zero?
	if (g_listWatcher.empty())
	{
		_tprintf( _T("ERROR: There is no watch key defined!.\n"));
		bExecuteReboot = FALSE;
		nRet = 1;
		goto error_out;
	}
	else if (g_listWatcher.size() >= (MAX_SIZE_OF_WATCH_OBJECT - 2))
	{
		_tprintf( _T("ERROR: There is no too much key defined! (exceed 200).\n"));
		bExecuteReboot = FALSE;
		nRet = 1;
		goto error_out;
	}

	if (bColdStart_Reboot)
		goto error_out;

	ZeroMemory(&ghWatchHandle, sizeof(HANDLE)*MAX_SIZE_OF_WATCH_OBJECT);
	ZeroMemory(&CurrentPrjMD5, sizeof(MD5_CHECKSUM));
	if (!CalculateFileMD5(g_szLaunchPATH, &CurrentPrjMD5))
	{
		_tprintf( _T("ERROR: CalculateFileMD5() failed.\n"));
		bExecuteReboot = FALSE;
		nRet = 1;
		goto error_out;
	}

	// Open if output xml file existed
	if (_tcslen(g_szOutputLogFile) > 0)
	{
		TCHAR drive[_MAX_DRIVE];
		TCHAR dir[_MAX_DIR];
		TCHAR fname[_MAX_FNAME];
		TCHAR ext[_MAX_EXT];
		TCHAR szMyComputer[MAX_PATH];
		TCHAR szMD5[MAX_PATH];
		TCHAR infoBuf[INFO_BUFFER_SIZE];
		DWORD bufCharCount = INFO_BUFFER_SIZE;
		BOOL  bfindProject = FALSE;

		_tsplitpath_s(g_szLaunchPATH, drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, ext, _MAX_EXT);

		if (!GetComputerName( infoBuf, &bufCharCount ))
			_stprintf_s(szMyComputer, MAX_PATH, _T("%s%s"), fname, ext);
		else
			_stprintf_s(szMyComputer, MAX_PATH, _T("%s%s@%s"), fname, ext, infoBuf);

		// create MD5 string
		_stprintf_s(szMD5, MAX_PATH, _T("%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"), 
			CurrentPrjMD5.signature[0], CurrentPrjMD5.signature[1], CurrentPrjMD5.signature[2],
			CurrentPrjMD5.signature[3], CurrentPrjMD5.signature[4], CurrentPrjMD5.signature[5],
			CurrentPrjMD5.signature[6], CurrentPrjMD5.signature[7], CurrentPrjMD5.signature[8],
			CurrentPrjMD5.signature[9], CurrentPrjMD5.signature[10], CurrentPrjMD5.signature[11],
			CurrentPrjMD5.signature[12], CurrentPrjMD5.signature[13], CurrentPrjMD5.signature[14],
			CurrentPrjMD5.signature[15], CurrentPrjMD5.signature[16]
			);

		if (gfnIsFileExist(g_szOutputLogFile))
		{
			bOutputXMLOpened = Xmldoc.Load(g_szOutputLogFile);
			Xmldoc.FindElem(_T("xmlRoot"));
		}
		else
		{
			Xmldoc.InsertNode( CMarkupMSXML::MNT_PROCESSING_INSTRUCTION, _T("xml version = \"1.0\"") );
			Xmldoc.AddElem(_T("xmlRoot"));
			bOutputXMLOpened = TRUE;
		}

		if (bOutputXMLOpened)
		{
			Xmldoc.IntoElem(); // inside xmlRoot

			if (g_bColdStartupMode == TRUE && g_nRepeateCount > 1)
			{
				while (Xmldoc.FindElem(_T("Project")))
				{
					if (0 == Xmldoc.GetAttrib(_T("name")).CompareNoCase(szMyComputer) &&
						0 == Xmldoc.GetAttrib(_T("md5")).CompareNoCase(szMD5) &&
						0 == Xmldoc.GetAttrib(_T("done")).CompareNoCase(_T("false")))
					{
						bfindProject = TRUE;
						break;
					}
				}
			}
			if (!bfindProject)
			{
				Xmldoc.AddElem(_T("Project"));
				Xmldoc.SetAttrib(_T("name"), szMyComputer);
				Xmldoc.SetAttrib(_T("md5"), szMD5);
				Xmldoc.SetAttrib(_T("mode"), ((g_bColdStartupMode) ? _T("ColdStart") : _T("WarmStart")));
				Xmldoc.SetAttrib(_T("repeat"), (int)g_nRepeateCount);
				if (g_dwAutoCloseTimeout > 0)
					Xmldoc.SetAttrib(_T("auto_close"), g_dwAutoCloseTimeout);
				if (g_nWaitForCPU_UnderValue > 0)
					Xmldoc.SetAttrib(_T("cpu_upperbound"), (int)g_nWaitForCPU_UnderValue);
				if (g_bColdStartupMode == TRUE && g_nRepeateCount > 1)
				{
					Xmldoc.SetAttrib(_T("done"), _T("false"));
				}
			}

			if (g_bColdStartupMode == TRUE && g_nRepeateCount > 1)
			{
				if (nLoopIndex >= (g_nRepeateCount-1)) // at the end
				{
					Xmldoc.SetAttrib(_T("done"), _T("true"));
				}
			}

			Xmldoc.IntoElem(); // inside Project node
		}
	}

	// Init Repeat Count
	if (g_bColdStartupMode == TRUE)
		nRepeateCount = g_nRepeateCount - nLoopIndex;
	else
		nRepeateCount = g_nRepeateCount;

	// Set Single CPU running status
	::SetProcessAffinityMask(::GetCurrentProcess(), 0x00000001);

_Startup:
	// Loop mode start
	nLoopIndex = g_nRepeateCount - nRepeateCount + 1;

	if ((g_bColdStartupMode == FALSE) && (g_nRepeateCount > 1))
	{
		_tprintf(_T("\n >>>>> Start loop test [%d%] >>>>>\n\n"), nLoopIndex);
		::Sleep(2000);
	}

	// Standby for waiting CPU usage
	if (g_nWaitForCPU_UnderValue > 0)
	{
		_tprintf( _T("Wait for CPU usage below %d%% ...\n"), g_nWaitForCPU_UnderValue);
		CCPU_Usage *pCPU_Usage = NULL;
		double dAverageValue = 0.0;
		int i = 0;
		int j = 0;
		int nAverageValue = 0;
		for (i = 0; i < MAX_COUNT_OF_CPU_SAMPLE; i++)
		{
			g_nCPUSample[i] = -1.0;
		}
		if (g_nWaitForCPU_UnderValue > 100)
			g_nWaitForCPU_UnderValue = 100;
		pCPU_Usage = new CCPU_Usage();
		if (pCPU_Usage)
		{
			i = 0;
			while (1)
			{
				g_nCPUSample[i++] = pCPU_Usage->GetCPUUsages();
				j = 0;
				dAverageValue = 0.0;
				while (g_nCPUSample[j] > 0.00000001 && j < MAX_COUNT_OF_CPU_SAMPLE)
				{
					dAverageValue += g_nCPUSample[j];
					j++;
				}
				dAverageValue = (dAverageValue) / (double)j;
				_tprintf( _T("Update current CPU usage: %.2f%%\n"), g_nCPUSample[i-1]);
				if ((dAverageValue < (double)g_nWaitForCPU_UnderValue) && (j >= MAX_COUNT_OF_CPU_SAMPLE))
					break;
				if (i == MAX_COUNT_OF_CPU_SAMPLE)
					i = 0;
				::Sleep(100);
			}
			delete pCPU_Usage;
		}
		_tprintf( _T("Kick-Off: Average CPU usage: %.2f%%\n"), dAverageValue);
	}

	QueryPerformanceFrequency(&liPerfFreq);

	// prepare IdleEvent
	HANDLE hIdleEvent = ::CreateEvent( 
        NULL,               // default security attributes
        FALSE,              // manual-reset event
        FALSE,              // initial state is nonsignaled
        NULL                // object name
        ); 

	if  (NULL == hIdleEvent)
	{
		_tprintf( _T("ERROR: CreateIdleEvent failed (%d).\n"), GetLastError() );
		return 1;
	}

	// Set Idle Event on number 1
	ghWatchHandle[1] = hIdleEvent;

	// prepare security attribs
	SECURITY_ATTRIBUTES SecurityAttributes;
	ZeroMemory( &SecurityAttributes, sizeof(SecurityAttributes) );
	SecurityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
	SecurityAttributes.bInheritHandle = TRUE;
	SecurityAttributes.lpSecurityDescriptor = NULL;

	// prepare user customized Watch event
	// if (gfnPrepareUserEvent(&ghWatchHandle[2], SIZE_OF_WATCH_OBJECT, &SecurityAttributes))
	if (gfnPrepareUserEvent(&ghWatchHandle[2], &SecurityAttributes))
	{
		// Init wait process timeout sec
		dwWaitProcessTimeOut = (g_dwAutoCloseTimeout > 0) ? g_dwAutoCloseTimeout : INFINITE;
		dwWatchHandleCount = g_listWatcher.size() + 2;
		// create wait process idle thread
		DWORD dwThreadID = 0;
		HANDLE hWaitForIdleThread = CreateThread( 
				NULL,                   // default security attributes
				0,                      // use default stack size  
				ThreadWaitForProcessIdle,  // thread function name
				(LPVOID)hIdleEvent,     // argument to thread function 
				CREATE_SUSPENDED,       // use default creation flags 
				&dwThreadID);           // returns the thread identifier 


		_tstrtime_s( szRunBeginTime, 128 );

		QueryPerformanceCounter(&liPerfStart);

		// start creat process
		TCHAR *pszStartDirectory = (_tcslen(g_szLocationPATH) > 0) ? g_szLocationPATH : NULL;
		STARTUPINFO si;
		PROCESS_INFORMATION pi;

		ZeroMemory( &si, sizeof(si) );
		si.cb = sizeof(si);
		ZeroMemory( &pi, sizeof(pi) );

		// Start the child process. 
		if( CreateProcess( NULL,   // No module name (use command line)
			g_szLaunchPATH,         // Command line
			NULL,                   // Process handle not inheritable
			NULL,                   // Thread handle not inheritable
			TRUE,                   // Set handle inheritance to FALSE
			0,                      // No creation flags
			NULL,                   // Use parent's environment block
			pszStartDirectory,      // Use parent's starting directory 
			&si,                    // Pointer to STARTUPINFO structure
			&pi )                   // Pointer to PROCESS_INFORMATION structure
		)
		{
			ghWatchHandle[0] = pi.hProcess; // // Set Running process handle on number 0
			// ghChildThread = pi.hThread;

			// Kick-off Waiting for Idle Thread
			if (hWaitForIdleThread)
				::ResumeThread(hWaitForIdleThread);

			{
				BOOL bContinue = TRUE;
				while (bContinue)
				{
					dwEvent = ::WaitForMultipleObjects( 
							dwWatchHandleCount,         // number of objects in array
							ghWatchHandle,				// array of objects
							FALSE,						// wait for any object
							dwWaitProcessTimeOut);      // wait second
					if (dwEvent == WAIT_TIMEOUT)
					{
						// force process terminated
						if (pi.hProcess)
						{
							if (!gfnSafeTerminateProcess(pi.hProcess, 1, 1000))
							{
								::TerminateProcess(pi.hProcess, 1);
							}
						}
						bContinue = FALSE;
					}
					else if (dwEvent == WAIT_OBJECT_0) // Running process exit event
					{
						::CloseHandle(pi.hProcess);
						::CloseHandle(pi.hThread);
						ghWatchHandle[0] = NULL;
						bContinue = FALSE;
					}
					else if (dwEvent == (WAIT_OBJECT_0 + 1)) // WM_IDLE event
					{
						QueryPerformanceCounter(&liPerfIdle);
					}
					else
					{
						nTriggered = 0;
						nTriggered += (dwEvent - (WAIT_OBJECT_0 + 2));
						if (nTriggered >= 0)
						{
							// QueryPerformanceCounter(&(gWatchObjTable[nTriggered].liWatchTime));
							QueryPerformanceCounter(&liPerfObejct);
							((trWATCH_OBJECT*)g_listWatcher[nTriggered])->liWatchTime = liPerfObejct;
						}
					}
				}
			}
			// To minimize every interleave duration, you can control below sleep time
			Sleep(100);
			if (hWaitForIdleThread)
			{
				::CloseHandle(hWaitForIdleThread);
				hWaitForIdleThread = NULL;
			}
			bDetected = TRUE;
		}
		else
		{
			_tprintf(_T("ERROR: CreateProcess failed (%d).\n"), GetLastError() );
			nRet = 1;
		}

		_tstrtime_s( szRunEndTime, 128 );
		gfnCloseUserEvent(&ghWatchHandle[2], g_listWatcher.size());
	}

	if (bDetected)
	{
		int j = 0;
		long IdleTime;
		long EventTime;

		trWATCH_OBJECT *pItem;

		if (bOutputXMLOpened)
		{
			Xmldoc.AddElem(_T("run")); // new child 'run' node
			Xmldoc.SetAttrib(_T("loop"), (int)nLoopIndex);
			Xmldoc.SetAttrib(_T("begin"), szRunBeginTime);
			Xmldoc.SetAttrib(_T("end"), szRunEndTime);
			Xmldoc.IntoElem();
		}

		IdleTime = (((liPerfIdle.QuadPart - liPerfStart.QuadPart) * 1000) / liPerfFreq.QuadPart);

		_tprintf(_T(">>>>>> LaunchTM SUMMARY <<<<<<\n"));
		_tprintf(_T("From AP Launch to AP Idle: %d (ms)\n"), IdleTime);
		_tprintf(_T("==================================\n"));

		if (bOutputXMLOpened)
		{
			// Xmldoc.AddChildElem(_T("checkpoint"), (int)IdleTime); // new child 'checkpoint' node
			// Xmldoc.SetChildAttrib(_T("name"), _T("[WaitForInputIdle]"));
			Xmldoc.AddElem(_T("checkpoint"));
			Xmldoc.IntoElem();
			Xmldoc.AddElem(_T("ck_name"), _T("[WaitForInputIdle]"));
			Xmldoc.AddElem(_T("ck_value"), IdleTime);
			Xmldoc.OutOfElem();
		}

		for (WATCH_OBJ_IT IT = g_listWatcher.begin(); IT != g_listWatcher.end (); IT++)
		{
			pItem = *IT;
			EventTime = 0L;

			if (bOutputXMLOpened)
			{
				Xmldoc.AddElem(_T("checkpoint"));
				Xmldoc.IntoElem();
			}

			if (pItem->liWatchTime.QuadPart == 0L)
			{
				_tprintf(_T("From AP Launch to Event(%d)[%s]: N/A. \n"), j, pItem->szEventName);
				if (bOutputXMLOpened)
				{
					// Xmldoc.AddChildElem(_T("checkpoint"), _T("N/A")); // new child 'checkpoint' node
					// Xmldoc.SetChildAttrib(_T("name"), pItem->szEventName);
					Xmldoc.AddElem(_T("ck_name"), pItem->szEventName);
					Xmldoc.AddElem(_T("ck_value"), _T("?"));
				}
			}
			else
			{
				EventTime = (((pItem->liWatchTime.QuadPart - liPerfStart.QuadPart) * 1000) / liPerfFreq.QuadPart);
				_tprintf(_T("From AP Launch to Event(%d)[%s]: %d (ms)\n"), j, pItem->szEventName, EventTime);
				if (bOutputXMLOpened)
				{
					// Xmldoc.AddChildElem(_T("checkpoint"), (int)EventTime); // new child 'checkpoint' node
					// Xmldoc.SetChildAttrib(_T("name"), pItem->szEventName);
					Xmldoc.AddElem(_T("ck_name"), pItem->szEventName);
					Xmldoc.AddElem(_T("ck_value"), (int)EventTime);
				}
			}

			if (bOutputXMLOpened)
			{
				Xmldoc.OutOfElem();
			}
			j++;
		}

		if (bOutputXMLOpened)
		{
			Xmldoc.OutOfElem();
		}

		_tprintf(_T("==================================\n"));
	}
	
	::CloseHandle(hIdleEvent);
	hIdleEvent = NULL;

	if ((g_bColdStartupMode == FALSE) && (nRepeateCount > 1))
	{
		nRepeateCount--;
		goto _Startup;
	}
	if ((g_bColdStartupMode == TRUE) && (nRepeateCount > 0))
	{
		if (nRepeateCount == 1)
		{
			//clean up registry
			Delete_REG_KeyValue(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"), _T("LaunchTM"));
			ProductKey.DeleteKeyAll(NULL);
			::MessageBox(NULL, _T("Cold-Startup: Run test finished!"), _T("LaunchTM"), MB_OK);
			bExecuteReboot = FALSE;
		}
		else
		{
			ProductKey.SetDWORDValue(KEY_LOOP_INDEX, (g_nRepeateCount - nRepeateCount + 1));
			bExecuteReboot = TRUE;
		}
	}

error_out:
	// Remove all definition
	for (WATCH_OBJ_IT IT = g_listWatcher.begin(); IT != g_listWatcher.end (); IT++)
	{ 
		delete *IT; 
	}
    g_listWatcher.clear();

	if (bOutputXMLOpened)
	{
		Xmldoc.Save(g_szOutputLogFile);
		bOutputXMLOpened = FALSE;
	}

	if (g_bColdStartupMode == TRUE && nRet > 0) // error out in cold start
	{
		ProductKey.DeleteKeyAll(NULL);
	}

	if (bExecuteReboot)
	{
		// prepare auto run
		TCHAR szRebootCommand[MAX_PATH];
		TCHAR szExeFileName[MAX_PATH];
		::GetModuleFileName(NULL, szExeFileName, MAX_PATH);
		_stprintf_s(szRebootCommand, MAX_PATH, _T("\"%s\" -c"), szExeFileName);
		Save_REG_String(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"), _T("LaunchTM"), szRebootCommand);

		// execute reboot
		::ShellExecute(NULL, NULL, _T("shutdown.exe"), _T("/r /t 5 /f"), NULL, SW_SHOWNORMAL);
	}

	return nRet;
}

