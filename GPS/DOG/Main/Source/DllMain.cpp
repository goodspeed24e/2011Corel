#include <windows.h>
#include <atlbase.h>
#include <atlcom.h>

#include "DOGSession.h"

HINSTANCE g_hInstance = 0;

/////////////////////////////////////////////////////////////////////////////
// DLL Entry Point

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID /*lpReserved*/)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hInstance);
	}
	return TRUE;    // ok
}

extern "C" 
void* STDAPICALLTYPE CreateComponent(void)
{
	dog::CDOGSession* pComponent = NULL;
	dog::CDOGSession::Make(&pComponent);
	return pComponent;
}
