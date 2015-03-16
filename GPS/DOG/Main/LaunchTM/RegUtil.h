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

#ifndef __REGUTIL__
#define __REGUTIL__

#include <windows.h>
#include <tchar.h>

class CRegUtil
{
public:
	CRegUtil(HKEY hKey = HKEY_LOCAL_MACHINE, const TCHAR *pszProductName = NULL, const TCHAR *pszProductVer = NULL);
	~CRegUtil();

public:
	// Product key is composed by two part, Product name and product version
	// i.e. Software\\Corel\\DVD Wizard\\1.0

	// Load product name and version from resource file
	BOOL LoadProductName(HINSTANCE hInstRC, UINT nID);
	BOOL LoadProductVersion(HINSTANCE hInstRC, UINT nID);

	// set product name and version directly 
	BOOL SetProductBase(const TCHAR *pszProductBase);
	BOOL SetProductName(const TCHAR *pszProductKey);
	BOOL SetProductVersion(const TCHAR *pszProductVersion);

	// Get product name and version
	void GetProductBase(TCHAR *pszProductBase) const;
	void GetProductName(TCHAR *pszProductKey) const;
	void GetProductVersion(TCHAR *pszProductVersion) const;

	// Get Product key
	void GetProductKey(TCHAR *pszProductKey);
	// Set Product key
	BOOL SetProductKey(const TCHAR *pszProductKey);

	// get root key, for testing
	void SetRootKey(HKEY hKeyRoot);
	HKEY GetRootKey(void) const { return m_hRootKey; }
	// Get registry value from product key and value name
	// value name is assigned in program directly
	BOOL GetDWORDValue(LPCTSTR lpValueName, DWORD *pdwValue, LPCTSTR lpszSubKey = NULL) const;
	BOOL GetStringValue(LPCTSTR lpValueName, LPTSTR lpStringValue, LPCTSTR lpszSubKey = NULL) const;
	BOOL GetStringValue2(LPCTSTR lpValueName, LPTSTR lpStringValue, LPCTSTR lpszSubKey = NULL) const;
	// Set registry value from product key and value name
	// value name is assigned in program directly
	BOOL SetDWORDValue(LPCTSTR lpValueName, DWORD dwValue, LPCTSTR lpszSubKey = NULL);
	BOOL SetStringValue(LPCTSTR lpValueName, LPCTSTR lpStringValue, LPCTSTR lpszSubKey = NULL);

	// Get registry value from product key and value name
	// value name is loaded from resource file
	BOOL GetDWORDValueByID(HINSTANCE hInstRC, UINT nID, DWORD *pdwValue, LPCTSTR lpszSubKey = NULL) const;
	BOOL GetStringValueByID(HINSTANCE hInstRC, UINT nID, LPTSTR lpStringValue, LPCTSTR lpszSubKey = NULL) const;
	// Set registry value from product key and value name
	// value name is loaded from resource file
	BOOL SetDWORDValueByID(HINSTANCE hInstRC, UINT nID, DWORD dwValue, LPCTSTR lpszSubKey = NULL);
	BOOL SetStringValueByID(HINSTANCE hInstRC, UINT nID, LPCTSTR lpStringValue, LPCTSTR lpszSubKey = NULL);

	// Given a sub-key name, return its key handle. caller should close this key handle.
	HKEY GetKeyHandle(LPCTSTR lpMainKey) const;
	BOOL DeleteKeyAll(LPCTSTR lpMainKey); // ie: delete Software\\Corel\ProductName\lpMainKey
	//void gfnShowMsg(int nID);

	BOOL SetBinaryValue(LPCTSTR lpValueName, LPBYTE pData, DWORD dwSize, LPCTSTR lpszSubKey = NULL);
	BOOL GetBinaryValue(LPCTSTR lpValueName, LPBYTE pData, DWORD *pdwSize, LPCTSTR lpszSubKey = NULL) const;

private:
	HKEY	m_hRootKey; // Should be HKEY_LOCAL_MACHINE, HKEY_CURRENT_USER, .....
						// default is HKEY_LOCAL_MACHINE
	TCHAR	m_szProductBase[MAX_PATH];	// i.e.	"Software\\Corel
	TCHAR	m_szProductVersion[20];		// i.e. 1.0 1.02,.....
	TCHAR	m_szProductName[MAX_PATH];	// i.e. "Software\\Corel\\DVD Wizard"

	TCHAR 	m_szProductKey[MAX_PATH + 20]; // i.e. "RootKey\\ProductBase\\ProductName\\Version"

	void    GenerateProductKey(void);
};

BOOL RegDelnode(HKEY hKeyRoot, LPTSTR lpSubKey);
BOOL Delete_REG_KeyValue(HKEY hKeyRoot, LPCTSTR lpMainKey, LPCTSTR lpValueName);
BOOL Save_REG_String(HKEY hKeyRoot, LPCTSTR lpMainKey, LPCTSTR lpValueName, LPCTSTR lpStringValue);
BOOL Get_REG_String(HKEY hKeyRoot, LPCTSTR lpMainKey, LPCTSTR lpValueName, LPTSTR lpStringValue);
BOOL Get_REG_String2(HKEY hKeyRoot, LPCTSTR lpMainKey, LPCTSTR lpValueName, LPTSTR lpStringValue);
BOOL Save_REG_DWORD(HKEY hKeyRoot, LPCTSTR lpMainKey, LPCTSTR lpValueName, DWORD dwValue);
BOOL Get_REG_DWORD(HKEY hKeyRoot, LPCTSTR lpMainKey, LPCTSTR lpValueName, DWORD *pdwValue);
BOOL Save_REG_Binary(HKEY hKeyRoot, LPCTSTR lpMainKey, LPCTSTR lpValueName, const LPBYTE pBtye, DWORD dwSize);
BOOL Get_REG_Binary(HKEY hKeyRoot, LPCTSTR lpMainKey, LPCTSTR lpValueName, LPBYTE pByte, DWORD *pdwSize);

#endif