#ifndef MyFunction_h__
#define MyFunction_h__

// #include <string>
// using namespace std;
#include <Psapi.h>

BOOL IsProcessRunning(LPCTSTR pszProcess)
{
	// Get the list of process identifiers...
	TCHAR szProcessName[MAX_PATH];
	DWORD dwProcessIds[128];
	DWORD dwSize;

	EnumProcesses(dwProcessIds, sizeof(dwProcessIds), &dwSize);

	// Calculate how many process identifiers were returned...
	int iCount = dwSize / sizeof(DWORD);

	// Check each process to see if the name matches...
	for (int i = 0; i < iCount; i++)
	{
		ZeroMemory(szProcessName, sizeof(szProcessName));

		// Get a handle to the process...
		HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, 
			FALSE, dwProcessIds[i]);

		// Get the process name...
		if (hProcess != NULL)
		{
			HMODULE hModule;

			if (EnumProcessModules(hProcess, &hModule, sizeof(hModule), &dwSize))
			{
				GetModuleBaseName(hProcess, hModule, szProcessName, 
					sizeof(szProcessName));	
			}

			// We're done querying the process...
			CloseHandle(hProcess);

			// Check to see if the names match...
			if (lstrlen(szProcessName) > 0 && 
				_tcsicmp(szProcessName, pszProcess) == 0)
				return TRUE;
		}	
	}

	return FALSE;
}
#endif // MyFunction_h__
