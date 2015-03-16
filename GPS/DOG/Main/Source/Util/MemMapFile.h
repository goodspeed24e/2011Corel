#pragma once

#include <windows.h>

// namespace Diagnosis-of-GPS
namespace dog {

/**
 * CMemMapFile is an utility class, which wraps windows memory mapping file operations.
 */
class CMemMapFile
{
public:
	CMemMapFile()  : m_hFile(NULL), m_lpView(NULL) {}
	~CMemMapFile() { Close(); }

	/// Creates or opens a file mapping object with specified name and size.
	/// If the object exists before the function call, the function returns
	/// a handle to the existing object (with its current size, not the
	/// specified size), and GetLastError returns ERROR_ALREADY_EXISTS
	///
	/// @param lpFileName: The name of the file mapping object to be opened.
	/// @param dwSize: Size of the file mapping object.
	bool Create(LPCTSTR lpFileName, DWORD dwSize)
	{
		if(m_hFile) {
			Close();
		}

		m_hFile = CreateFileMapping(INVALID_HANDLE_VALUE, 0, PAGE_READWRITE, 0,dwSize, lpFileName);
		if(NULL == m_hFile) {
			return false;
		}

		m_lpView = MapViewOfFile(m_hFile, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
		return true;
	}

	/// Opens a named file mapping object.
	/// @param lpFileName: The name of the file mapping object to be opened.
	bool Open(LPCTSTR lpFileName)
	{
		if(m_hFile) {
			Close();
		}

		m_hFile = OpenFileMapping( FILE_MAP_READ | FILE_MAP_WRITE,FALSE, lpFileName);
		if(NULL == m_hFile) {
			return false;
		}

		m_lpView = MapViewOfFile(m_hFile, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
		return true;
	}


	/// Close the file mapping object.
	void Close()
	{
		if(m_lpView)
		{
			UnmapViewOfFile((LPVOID)m_lpView);
			m_lpView = NULL;
		}

		if(m_hFile)
		{
			CloseHandle(m_hFile);
			m_hFile = NULL;
		}
	}

	/// Get the starting address of the mapped view.
	LPVOID GetData() { return m_lpView; }

protected:
	HANDLE m_hFile;   ///< A handle to a file mapping object.
	LPVOID m_lpView;  ///< The starting address of the mapped view.
};


} //namespace dog
