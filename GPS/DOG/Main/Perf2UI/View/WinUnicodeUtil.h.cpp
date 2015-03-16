#include "WinUnicodeUtil.h"
#include <windows.h>

std::wstring AnsiToUnicode(const char* ansi_str)
{
	wchar_t* uni_str = NULL; 
    ULONG cCharacters;
    DWORD dwError;

    // If input is null then just return the same.
    if (NULL == ansi_str)
    {        
        return L"";
    }

    // Determine number of wide characters to be allocated for the
    // Unicode string.
    cCharacters = (ULONG) strlen(ansi_str)+1;

    // Use of the OLE allocator is required if the resultant Unicode
    // string will be passed to another COM component and if that
    // component will free it. Otherwise you can use your own allocator.
    uni_str = (LPOLESTR) CoTaskMemAlloc(cCharacters*2);
	if (NULL == uni_str) 
	{
		//out of memory
        return L"";
	}

    // Covert to Unicode.
    if (0 == MultiByteToWideChar(CP_ACP, 0, ansi_str, cCharacters,
                  uni_str, cCharacters))
    {
        dwError = GetLastError();
		HRESULT hr = HRESULT_FROM_WIN32(dwError);
        CoTaskMemFree(uni_str);
        return L"";
    }

	std::wstring ret_str = uni_str;
	CoTaskMemFree(uni_str);	
	return ret_str;	
}

std::string UnicodeToAnsi(const wchar_t* uni_str)
{
	char* ansi_str = NULL;
    ULONG cbAnsi, cCharacters;
    DWORD dwError;

    // If input is null then just return the same.
    if (uni_str == NULL)
    {        
        return "";
    }    

    // Determine number of bytes to be allocated for ANSI string. 
    // An ANSI string can have at most 2 bytes per character 
	// (for Double Byte Character Strings.)
	cCharacters = (ULONG) wcslen(uni_str)+1;
    cbAnsi = cCharacters*2;

    // Use of the OLE allocator is not required because the resultant
    // ANSI string will never be passed to another COM component. 
	// You can use your own allocator.
    ansi_str = (LPSTR) CoTaskMemAlloc(cbAnsi);	
	if (NULL == ansi_str) 
	{
		//error: out of memory
		return "";
	}

    // Convert to ANSI.
    if (0 == WideCharToMultiByte(CP_ACP, 0, uni_str, cCharacters, 
		                         ansi_str, cbAnsi, NULL, NULL))
    {
        dwError = GetLastError();
		HRESULT hr = HRESULT_FROM_WIN32(dwError);
        CoTaskMemFree(ansi_str);
		return "";
    }

	std::string ret_str = ansi_str;
	CoTaskMemFree(ansi_str);
	return ret_str;
}
