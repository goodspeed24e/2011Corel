#define STRICT
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <windows.h>
//-----------------------------------------------------------------------------
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit
#define _WTL_USE_CSTRING

#include <atlbase.h>       // base ATL classes
#include <atlapp.h>        // base WTL classes

CAppModule _Module;        // WTL version of CComModule

//-----------------------------------------------------------------------------
#include "MainWindow.h"

CBrush GreenBrush;
CBrush BlackBrush;
CBrush GrayBrush;

//-----------------------------------------------------------------------------
int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	_Module.Init ( NULL, hInstance );
	HMODULE hInstRich = ::LoadLibrary(CRichEditCtrl::GetLibraryName());

	GreenBrush.CreateSolidBrush(RGB(0,0xbb,0));
	BlackBrush.CreateSolidBrush(RGB(0,0,0));
	GrayBrush.CreateSolidBrush(RGB(0xcc,0xcc,0xcc));

	CMessageLoop theLoop;
	_Module.AddMessageLoop(&theLoop);

	CMyWindow mainWindow;
	MSG msg;

	// Create the main window	
	if ( NULL == mainWindow.Create( NULL, CWindow::rcDefault, _T("DOG UI Lite")))
		return 1;       // Window creation failed

	// Show the window
	mainWindow.ShowWindow ( nCmdShow );
	mainWindow.UpdateWindow();

	// Standard Win32 message loop
	while ( GetMessage ( &msg, NULL, 0, 0 ) > 0 )
	{
		TranslateMessage ( &msg );
		DispatchMessage ( &msg );
	}

	_Module.RemoveMessageLoop();	
	_Module.Term();
	::FreeLibrary(hInstRich);
	return (int)msg.wParam;
}
