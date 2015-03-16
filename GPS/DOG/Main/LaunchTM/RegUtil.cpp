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
/*
    Module:    RegUtil.cpp (File ID: 14252)

    Purpose:    

    Function:

    Revision History:

    Author: 

    Copyright (C) 2005-2008 Corel, Corp.
*/

#include <stdafx.h>
#include <tchar.h>
#include <strsafe.h>
#include "RegUtil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#undef _AIKFILEID
#define _AIKFILEID 14252

#undef _AIKFUNCID
#define _AIKFUNCID 1

//////////////////////////////////////////////////////////////////////////////////////////
// Registry related functions
//////////////////////////////////////////////////////////////////////////////////////////

BOOL RootKeyExists(HKEY hKeyRoot)
{
	LONG	lResult;
	HKEY	hKey;

	//open the specified key
	lResult = RegOpenKeyEx( hKeyRoot,
							NULL,
							0,
							KEY_ENUMERATE_SUB_KEYS,
							&hKey);

	if (ERROR_SUCCESS != lResult)
		return FALSE;

	RegCloseKey(hKey);

	return TRUE;
}

BOOL Delete_REG_KeyValue(HKEY hKeyRoot, LPCTSTR lpMainKey, LPCTSTR lpValueName)
{
	HKEY hKey;

	LONG lResult = RegOpenKeyEx( hKeyRoot,
							NULL,
							0,
							KEY_READ | KEY_WRITE,
							&hKey);

	if (ERROR_SUCCESS != lResult)
		return FALSE;


	lResult = RegDeleteValue(hKey, lpValueName);
	
	RegCloseKey(hKey);

	if (lResult != ERROR_SUCCESS)
		return FALSE;

	return TRUE;
}

BOOL Save_REG_String(HKEY hKeyRoot, LPCTSTR lpMainKey, LPCTSTR lpValueName, LPCTSTR lpStringValue)
{
	if (_tcslen(lpMainKey) == 0 ||
		_tcslen(lpValueName) == 0 ||
		lpStringValue == NULL)
		return FALSE;

	HKEY	hKey;
	LONG	lResult;
	DWORD	dwDisp;

	lResult = RegCreateKeyEx(hKeyRoot,
								lpMainKey,
								0,
								NULL,
								REG_OPTION_NON_VOLATILE, 
								KEY_WRITE,
								NULL, 
								&hKey,
								&dwDisp);

	if (lResult != ERROR_SUCCESS)
		return FALSE;

	DWORD dwDataSize = (DWORD)((_tcslen(lpStringValue) + 1) * sizeof(TCHAR));

	lResult = RegSetValueEx(hKey,
							lpValueName,
							0,
							REG_SZ,
							(LPBYTE)lpStringValue,
							dwDataSize);

	RegCloseKey(hKey);

	if (lResult != ERROR_SUCCESS)
		return FALSE;

	return TRUE;
}

BOOL Save_REG_DWORD(HKEY hKeyRoot, LPCTSTR lpMainKey, LPCTSTR lpValueName, DWORD dwValue)
{
	if (_tcslen(lpMainKey) == 0 ||
		_tcslen(lpValueName) == 0)
		return FALSE;

	HKEY	hKey;
	LONG	lResult;
	DWORD	dwDisp;

	lResult = RegCreateKeyEx(hKeyRoot,
								lpMainKey,
								0,
								NULL,
								REG_OPTION_NON_VOLATILE, 
								KEY_WRITE,
								NULL, 
								&hKey,
								&dwDisp);

	if (lResult != ERROR_SUCCESS)
		return FALSE;


	lResult = RegSetValueEx(hKey,
							lpValueName,
							0,
							REG_DWORD,
							(CONST BYTE*)&dwValue,
							sizeof(DWORD));

	RegCloseKey(hKey);

	if (lResult != ERROR_SUCCESS)
		return FALSE;

	return TRUE;
}

BOOL Get_REG_String(HKEY hKeyRoot, LPCTSTR lpMainKey, LPCTSTR lpValueName, LPTSTR lpStringValue)
{
	if (_tcslen(lpMainKey) == 0 ||
		_tcslen(lpValueName) == 0 ||
		lpStringValue == NULL)
		return FALSE;

	HKEY	hKey;
	LONG	lResult;


	lResult = RegOpenKeyEx(hKeyRoot,
							lpMainKey,
							0,
							KEY_READ,
							&hKey);

	if (lResult != ERROR_SUCCESS)
		return FALSE;


	DWORD dwType = REG_SZ;
	DWORD dwSize = 255;//sizeof(dwArray);


	((LPBYTE)lpStringValue)[0] = '\0';
	lResult = RegQueryValueEx(hKey,
								lpValueName,
								NULL,
								&dwType,
								(LPBYTE)lpStringValue,
								&dwSize);

	RegCloseKey(hKey);

	if (lResult != ERROR_SUCCESS)
		return FALSE;

	return TRUE;
}

BOOL Get_REG_String2(HKEY hKeyRoot, LPCTSTR lpMainKey, LPCTSTR lpValueName, LPTSTR lpStringValue)
{
	if (_tcslen(lpMainKey) == 0 ||
		_tcslen(lpValueName) == 0 ||
		lpStringValue == NULL)
		return FALSE;

	HKEY	hKey;
	LONG	lResult;	
	DWORD	dwDisp;


	lResult = RegCreateKeyEx(hKeyRoot,
							 lpMainKey,
							 0,
							 NULL,
							 REG_OPTION_NON_VOLATILE,
							 KEY_READ,
							 NULL,
							 &hKey,
							&dwDisp);


	/*
	lResult = RegOpenKeyEx(hKeyRoot,
							lpMainKey,
							0,
							KEY_ALL_ACCESS,
							&hKey);
							*/

	if (lResult != ERROR_SUCCESS)
		return FALSE;


	DWORD dwType = REG_SZ;
	DWORD dwSize = 255;//sizeof(dwArray);


	((LPBYTE)lpStringValue)[0] = '\0';
	lResult = RegQueryValueEx(hKey,
								lpValueName,
								NULL,
								&dwType,
								(LPBYTE)lpStringValue,
								&dwSize);

	RegCloseKey(hKey);

	if (lResult != ERROR_SUCCESS)
		return FALSE;

	return TRUE;
}

BOOL Get_REG_DWORD(HKEY hKeyRoot, LPCTSTR lpMainKey, LPCTSTR lpValueName, DWORD *pdwValue)
{
	if (_tcslen(lpMainKey) == 0 ||
		_tcslen(lpValueName) == 0 ||
		pdwValue == NULL)
		return FALSE;

	HKEY	hKey;
	LONG	lResult;


	lResult = RegOpenKeyEx(hKeyRoot,
							lpMainKey,
							0,
							KEY_READ,
							&hKey);

	if (lResult != ERROR_SUCCESS)
		return FALSE;


	DWORD dwType = REG_DWORD;
	DWORD dwSize = 255;//sizeof(dwArray);


	lResult = RegQueryValueEx(hKey,
								lpValueName,
								NULL,
								&dwType,
								(unsigned char *)pdwValue,
								&dwSize);

	RegCloseKey(hKey);

	if (lResult != ERROR_SUCCESS)
		return FALSE;

	return TRUE;
}

BOOL Save_REG_Binary(HKEY hKeyRoot, LPCTSTR lpMainKey, LPCTSTR lpValueName, const LPBYTE pBtye, DWORD dwSize)
{
	if (_tcslen(lpMainKey) == 0 ||
		_tcslen(lpValueName) == 0 ||
		pBtye == NULL)
		return FALSE;


	HKEY	hKey;
	LONG	lResult;
	DWORD	dwDisp;

	lResult = RegCreateKeyEx(hKeyRoot,
								lpMainKey,
								0,
								NULL,
								REG_OPTION_NON_VOLATILE, 
								KEY_WRITE,
								NULL, 
								&hKey,
								&dwDisp);

	if (lResult != ERROR_SUCCESS)
		return FALSE;

	lResult = RegSetValueEx(hKey,
							lpValueName,
							0,
							REG_BINARY,
							pBtye,
							dwSize);

	RegCloseKey(hKey);

	if (lResult != ERROR_SUCCESS)
		return FALSE;

	return TRUE;
}

BOOL Get_REG_Binary(HKEY hKeyRoot, LPCTSTR lpMainKey, LPCTSTR lpValueName, LPBYTE pByte, DWORD *pdwSize)
{
	if (_tcslen(lpMainKey) == 0 ||
		_tcslen(lpValueName) == 0 ||
		pByte == NULL)
		return FALSE;

	HKEY	hKey;
	LONG	lResult;


	lResult = RegOpenKeyEx(hKeyRoot,
							lpMainKey,
							0,
							KEY_READ,
							&hKey);

	if (lResult != ERROR_SUCCESS)
		return FALSE;


	DWORD dwType = REG_BINARY;

	lResult = RegQueryValueEx(hKey,
								lpValueName,
								NULL,
								&dwType,
								pByte,
								pdwSize);

	RegCloseKey(hKey);

	if (lResult != ERROR_SUCCESS)
		return FALSE;

	return TRUE;
}

//*************************************************************
//
//  RegDelnodeRecurse()
//
//  Purpose:    Deletes a registry key and all it's subkeys / values.
//
//  Parameters: hKeyRoot    -   Root key
//              lpSubKey    -   SubKey to delete
//
//  Return:     TRUE if successful.
//              FALSE if an error occurs.
//
//*************************************************************

BOOL RegDelnodeRecurse (HKEY hKeyRoot, LPTSTR lpSubKey)
{
    LPTSTR lpEnd;
    LONG lResult;
    DWORD dwSize;
    TCHAR szName[MAX_PATH];
    HKEY hKey;
    FILETIME ftWrite;

    // First, see if we can delete the key without having
    // to recurse.

    lResult = RegDeleteKey(hKeyRoot, lpSubKey);

    if (lResult == ERROR_SUCCESS) 
        return TRUE;

    lResult = RegOpenKeyEx (hKeyRoot, lpSubKey, 0, KEY_READ, &hKey);

    if (lResult != ERROR_SUCCESS) 
    {
        if (lResult == ERROR_FILE_NOT_FOUND) {
            // printf("Key not found.\n");
            return TRUE;
        } 
        else {
            // printf("Error opening key.\n");
            return FALSE;
        }
    }

    // Check for an ending slash and add one if it is missing.

    lpEnd = lpSubKey + lstrlen(lpSubKey);

    if (*(lpEnd - 1) != TEXT('\\')) 
    {
        *lpEnd =  TEXT('\\');
        lpEnd++;
        *lpEnd =  TEXT('\0');
    }

    // Enumerate the keys

    dwSize = MAX_PATH;
    lResult = RegEnumKeyEx(hKey, 0, szName, &dwSize, NULL,
                           NULL, NULL, &ftWrite);

    if (lResult == ERROR_SUCCESS) 
    {
        do {

            StringCchCopy (lpEnd, MAX_PATH*2, szName);

            if (!RegDelnodeRecurse(hKeyRoot, lpSubKey)) {
                break;
            }

            dwSize = MAX_PATH;

            lResult = RegEnumKeyEx(hKey, 0, szName, &dwSize, NULL,
                                   NULL, NULL, &ftWrite);

        } while (lResult == ERROR_SUCCESS);
    }

    lpEnd--;
    *lpEnd = TEXT('\0');

    RegCloseKey (hKey);

    // Try again to delete the key.

    lResult = RegDeleteKey(hKeyRoot, lpSubKey);

    if (lResult == ERROR_SUCCESS) 
        return TRUE;

    return FALSE;
}

//*************************************************************
//
//  RegDelnode()
//
//  Purpose:    Deletes a registry key and all it's subkeys / values.
//
//  Parameters: hKeyRoot    -   Root key
//              lpSubKey    -   SubKey to delete
//
//  Return:     TRUE if successful.
//              FALSE if an error occurs.
//
//*************************************************************

BOOL RegDelnode (HKEY hKeyRoot, LPTSTR lpSubKey)
{
    TCHAR szDelKey[MAX_PATH*2];

    StringCchCopy (szDelKey, MAX_PATH*2, lpSubKey);
    return RegDelnodeRecurse(hKeyRoot, szDelKey);
}

#ifdef _UNICODE
#define DEFAULT_COMPANY_BASE_STR L"Software\\Corel"
#define DEF_THREE_LEVEL_STR      L"%s\\%s\\%s"
#define DEF_TWO_LEVEL_STR        L"%s\\%s"
#define DEF_ONE_LEVEL_STR        L"%s"
#else
#define DEFAULT_COMPANY_BASE_STR "Software\\Corel"
#define DEF_THREE_LEVEL_STR      "%s\\%s\\%s"
#define DEF_TWO_LEVEL_STR        "%s\\%s"
#define DEF_ONE_LEVEL_STR        "%s"
#endif

#if _MSC_VER>=1400
#define DEF_MERGEPATH_STR(_Target, _Base, _Sub) \
	if (_Sub && _tcslen(_Sub) > 0 && _Base && _tcslen(_Base) > 0) \
		_stprintf_s(_Target, DEF_TWO_LEVEL_STR, _Base, _Sub); \
	else if (_Base && _tcslen(_Base) > 0) \
		_tcscpy_s(_Target, _Base); \
	else if (_Sub && _tcslen(_Sub) > 0) \
		_tcscpy_s(_Target, _Sub); \
	else \
	    _Target[0] = 0;
#else
#define DEF_MERGEPATH_STR(_Target, _Base, _Sub) \
	if (_Sub && _tcslen(_Sub) > 0 && _Base && _tcslen(_Base) > 0) \
		_stprintf(_Target, DEF_TWO_LEVEL_STR, _Base, _Sub); \
	else if (_Base && _tcslen(_Base) > 0)\
		_tcscpy(_Target, _Base); \
	else if (_Sub && _tcslen(_Sub) > 0)\
		_tcscpy(_Target, _Sub); \
	else \
	    _Target[0] = 0;
#endif


CRegUtil::CRegUtil(HKEY hKey, const TCHAR *pszProductName, const TCHAR *pszProductVer) 
{
#if _MSC_VER>=1400
// VS 2005 events
    _tcscpy_s(m_szProductBase, MAX_PATH, DEFAULT_COMPANY_BASE_STR);
#else
	::_tcscpy(m_szProductBase, DEFAULT_COMPANY_BASE_STR);
#endif
	SetRootKey(hKey);
	m_szProductKey[0] = 0;
	if (pszProductName)
		SetProductName(pszProductName);
	else
		m_szProductName[0] = 0;
	if (pszProductVer)
		SetProductVersion(pszProductVer);
	else
		m_szProductVersion[0] = 0;
	GenerateProductKey();
}

CRegUtil::~CRegUtil()
{
}

void CRegUtil::GenerateProductKey(void)
{
	if (_tcslen(m_szProductBase) == 0)
	{
		//::MessageBox(NULL, _T("Product key cannot be NULL."), _T("Error"), MB_OK);
		//wprintf_s(_T("Product key cannot be NULL"));
		return;
	}
	else
	{
		if (_tcslen(m_szProductVersion) == 0 && _tcslen(m_szProductName) == 0)
		{
		#if _MSC_VER>=1400
		// VS 2005 events
			_stprintf_s(m_szProductKey, DEF_ONE_LEVEL_STR, m_szProductBase);
		#else
			_stprintf(m_szProductKey, DEF_ONE_LEVEL_STR, m_szProductBase);
		#endif
		}
		else if (_tcslen(m_szProductVersion) == 0)
		{
		#if _MSC_VER>=1400
		// VS 2005 events
			_stprintf_s(m_szProductKey, DEF_TWO_LEVEL_STR, m_szProductBase, m_szProductName);
		#else
			_stprintf(m_szProductKey, DEF_TWO_LEVEL_STR, m_szProductBase, m_szProductName);
		#endif
		}
		else
		{
		#if _MSC_VER>=1400
		// VS 2005 events
			_stprintf_s(m_szProductKey, DEF_THREE_LEVEL_STR, m_szProductBase, m_szProductName, m_szProductVersion);
		#else
			_stprintf(m_szProductKey, DEF_THREE_LEVEL_STR, m_szProductBase, m_szProductName, m_szProductVersion);
		#endif
		}
	}
}

BOOL CRegUtil::LoadProductName(HINSTANCE hInstRC, UINT nID)
{
	TCHAR szName[255];

	if (::LoadString(hInstRC, nID, szName, 255) == 0)
		return FALSE;
	else
#if _MSC_VER>=1400
// VS 2005 events
        _tcscpy_s(m_szProductName, MAX_PATH, szName);
#else
		_tcscpy(m_szProductName, szName);
#endif

	GenerateProductKey();

	return TRUE;
}

BOOL CRegUtil::LoadProductVersion(HINSTANCE hInstRC, UINT nID)
{
	TCHAR szName[20];

	if (::LoadString(hInstRC, nID, szName, 20) == 0)
		return FALSE;
	else
#if _MSC_VER>=1400
// VS 2005 events
        _tcscpy_s(m_szProductVersion, 20, szName);
#else
		_tcscpy(m_szProductVersion, szName);
#endif

	GenerateProductKey();

	return TRUE;
}

BOOL CRegUtil::SetProductBase(const TCHAR *pszBase)
{
	if (!pszBase)
		return FALSE;

#if _MSC_VER>=1400
// VS 2005 events
    _tcscpy_s(m_szProductBase, MAX_PATH, pszBase);
#else
	::_tcscpy(m_szProductBase, pszBase);
#endif

	GenerateProductKey();

	return TRUE;
}

BOOL CRegUtil::SetProductName(const TCHAR *pszName)
{
	if (pszName == NULL)
		return FALSE;

#if _MSC_VER>=1400
// VS 2005 events
    _tcscpy_s(m_szProductName, MAX_PATH, pszName);
#else
	_tcscpy(m_szProductName, pszName);
#endif

	GenerateProductKey();

	return TRUE;
}	

BOOL CRegUtil::SetProductVersion(const TCHAR *pszVersion)
{
	if (pszVersion == NULL)
		return FALSE;

#if _MSC_VER>=1400
// VS 2005 events
    _tcscpy_s(m_szProductVersion, 20, pszVersion);
#else
	_tcscpy(m_szProductVersion, pszVersion);
#endif

	GenerateProductKey();

	return TRUE;
}	

void CRegUtil::GetProductBase(TCHAR *pszProductRoot) const
{
	if (pszProductRoot == NULL)
		return;

#if _MSC_VER>=1400
// VS 2005 events
    _tcscpy_s(pszProductRoot, MAX_PATH, m_szProductBase);
#else
	_tcscpy(pszProductRoot, m_szProductBase);
#endif
}

void CRegUtil::GetProductName(TCHAR *pszProductName) const
{
	if (pszProductName == NULL)
		return;

#if _MSC_VER>=1400
// VS 2005 events
    _tcscpy_s(pszProductName, MAX_PATH, m_szProductName);
#else
	_tcscpy(pszProductName, m_szProductName);
#endif
}

void CRegUtil::GetProductVersion(TCHAR *pszProductVersion) const
{
	if (pszProductVersion == NULL)
		return;

#if _MSC_VER>=1400
// VS 2005 events
    _tcscpy_s(pszProductVersion, 20, m_szProductVersion);
#else
	_tcscpy(pszProductVersion, m_szProductVersion);
#endif
}

void CRegUtil::GetProductKey(TCHAR *pszProductKey)
{
	if (pszProductKey == NULL)
		return;

#if _MSC_VER>=1400
// VS 2005 events
    _tcscpy_s(pszProductKey, MAX_PATH + 20, m_szProductKey);
#else
	_tcscpy(pszProductKey, m_szProductKey);
#endif
}

BOOL CRegUtil::SetProductKey(const TCHAR *pszProductKey)
{
	if (pszProductKey == NULL)
		return FALSE;

#if _MSC_VER>=1400
// VS 2005 events
    _tcscpy_s(m_szProductKey, MAX_PATH + 20, pszProductKey);
#else
	_tcscpy(m_szProductKey, pszProductKey);
#endif

	GenerateProductKey();

	return TRUE;
}

void CRegUtil::SetRootKey(HKEY hKey)
{
	m_hRootKey = hKey;
}

BOOL CRegUtil::GetDWORDValue(LPCTSTR lpValueName, DWORD *pdwValue, LPCTSTR lpszSubKey) const
{
	TCHAR szKey[255];

	DEF_MERGEPATH_STR(szKey, m_szProductKey, lpszSubKey)

	return Get_REG_DWORD(m_hRootKey, szKey, lpValueName, pdwValue);
}

BOOL CRegUtil::GetStringValue(LPCTSTR lpValueName, LPTSTR lpStringValue, LPCTSTR lpszSubKey) const
{
	TCHAR szKey[255];

	DEF_MERGEPATH_STR(szKey, m_szProductKey, lpszSubKey)

	return Get_REG_String(m_hRootKey, szKey, lpValueName, lpStringValue);
}

BOOL CRegUtil::GetStringValue2(LPCTSTR lpValueName, LPTSTR lpStringValue, LPCTSTR lpszSubKey) const
{
	TCHAR szKey[255];

	DEF_MERGEPATH_STR(szKey, m_szProductKey, lpszSubKey)

	return Get_REG_String2(m_hRootKey, szKey, lpValueName, lpStringValue);
}

BOOL CRegUtil::SetDWORDValue(LPCTSTR lpValueName, DWORD dwValue, LPCTSTR lpszSubKey)
{
	TCHAR szKey[255];

	DEF_MERGEPATH_STR(szKey, m_szProductKey, lpszSubKey)

	return Save_REG_DWORD(m_hRootKey, szKey, lpValueName, dwValue);
}

BOOL CRegUtil::SetStringValue(LPCTSTR lpValueName, LPCTSTR lpStringValue, LPCTSTR lpszSubKey)
{
	TCHAR szKey[255];

	DEF_MERGEPATH_STR(szKey, m_szProductKey, lpszSubKey)

	return Save_REG_String(m_hRootKey, szKey, lpValueName, lpStringValue);
}


BOOL CRegUtil::GetDWORDValueByID(HINSTANCE hInstRC, UINT nID, DWORD *pdwValue, LPCTSTR lpszSubKey) const
{
	TCHAR szValueName[255];

	if (::LoadString(hInstRC, nID, szValueName, 255) == 0)
		return FALSE;

	TCHAR szKey[255];

	DEF_MERGEPATH_STR(szKey, m_szProductKey, lpszSubKey)

	return Get_REG_DWORD(m_hRootKey, szKey, szValueName, pdwValue);
}

BOOL CRegUtil::GetStringValueByID(HINSTANCE hInstRC, UINT nID, LPTSTR lpStringValue, LPCTSTR lpszSubKey) const
{
	TCHAR szValueName[255];

	if (::LoadString(hInstRC, nID, szValueName, 255) == 0)
		return FALSE;
	TCHAR szKey[255];

	DEF_MERGEPATH_STR(szKey, m_szProductKey, lpszSubKey)

	return Get_REG_String(m_hRootKey, szKey, szValueName, lpStringValue);
}

BOOL CRegUtil::SetDWORDValueByID(HINSTANCE hInstRC, UINT nID, DWORD dwValue, LPCTSTR lpszSubKey)
{
	TCHAR szValueName[255];

	if (::LoadString(hInstRC, nID, szValueName, 255) == 0)
		return FALSE;

	TCHAR szKey[255];

	DEF_MERGEPATH_STR(szKey, m_szProductKey, lpszSubKey)

	return Save_REG_DWORD(m_hRootKey, szKey, szValueName, dwValue);
}

BOOL CRegUtil::SetStringValueByID(HINSTANCE hInstRC, UINT nID, LPCTSTR lpStringValue, LPCTSTR lpszSubKey)
{
	TCHAR szValueName[255];

	if (::LoadString(hInstRC, nID, szValueName, 255) == 0)
		return FALSE;
	TCHAR szKey[255];

	DEF_MERGEPATH_STR(szKey, m_szProductKey, lpszSubKey)

	return Save_REG_String(m_hRootKey, szKey, szValueName, lpStringValue);
}

HKEY CRegUtil::GetKeyHandle(LPCTSTR lpMainKey) const
{
	HKEY hKey = NULL;

	if (_tcslen(lpMainKey) == 0 || lpMainKey == NULL)
		return FALSE;

	LONG	lResult;
	TCHAR szKey[255];

#if _MSC_VER>=1400
	_stprintf_s(szKey, DEF_TWO_LEVEL_STR, m_szProductKey, lpMainKey);
#else
	_stprintf(szKey, DEF_TWO_LEVEL_STR, m_szProductKey, lpMainKey);
#endif

	lResult = RegOpenKeyEx(m_hRootKey,
							szKey,
							0,
							KEY_READ,
							&hKey);

	if (lResult != ERROR_SUCCESS)
		return NULL;
	else
		return hKey;

}

BOOL CRegUtil::DeleteKeyAll(LPCTSTR lpMainKey)
{
	BOOL bRet = FALSE;
	TCHAR szKey[255];

	DEF_MERGEPATH_STR(szKey, m_szProductKey, lpMainKey)

	bRet = RegDelnode(m_hRootKey, szKey);

	return bRet;
}

BOOL CRegUtil::SetBinaryValue(LPCTSTR lpValueName, LPBYTE pData, DWORD dwSize, LPCTSTR lpszSubKey)
{
	TCHAR szKey[255];

	DEF_MERGEPATH_STR(szKey, m_szProductKey, lpszSubKey)

	return Save_REG_Binary(m_hRootKey, szKey, lpValueName, pData, dwSize);
}

BOOL CRegUtil::GetBinaryValue(LPCTSTR lpValueName, LPBYTE pData, DWORD *pdwSize, LPCTSTR lpszSubKey) const
{
	TCHAR szKey[255];

	DEF_MERGEPATH_STR(szKey, m_szProductKey, lpszSubKey)

	return Get_REG_Binary(m_hRootKey, szKey, lpValueName, pData, pdwSize);
}

