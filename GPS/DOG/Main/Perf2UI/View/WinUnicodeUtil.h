#ifndef _Windows_Unicode_Util_h_
#define _Windows_Unicode_Util_h_

#include <string>

// Convert an ANSI single-character string to an Unicode wide-character string.
// It is a helper function by wrapping MultiByteToWideChar.
std::wstring AnsiToUnicode(const char* ansi_str);

// Convert an Unicode wide-character string to an ANSI single-character string.
// It is a helper function by wrapping WideCharToMultiByte.
std::string UnicodeToAnsi(const wchar_t* uni_str);


inline std::string ToAnsiString(const char* ansi_str)
{
	return std::string(ansi_str);
}

inline std::string ToAnsiString(const wchar_t* uni_str)
{
	return UnicodeToAnsi(uni_str);
}


#ifdef UNICODE

inline std::wstring ToTcharString(const char* ansi_str)
{
	return AnsiToUnicode(ansi_str);
}

inline std::wstring ToTcharString(const wchar_t* uni_str)
{
	return std::wstring(uni_str);
}

#else // UNICODE

inline std::string ToTcharString(const char* ansi_str)
{
	return std::string(ansi_str);
}

inline std::string ToTcharString(const wchar_t* uni_str)
{
	return UnicodeToAnsi(uni_str);
}

#endif //UNICODE

#endif //_Windows_Unicode_Util_h_
