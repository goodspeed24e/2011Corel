#pragma once

#include <windows.h>
#include "resource/resource.h"

//-----------------------------------------------------------------------------
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit
#define _WTL_USE_CSTRING

#include <atlbase.h>       // base ATL classes
#include <atlapp.h>        // base WTL classes
#include <atlwin.h>        // ATL GUI classes
#include <atlframe.h>      // WTL frame window classes
#include <atlmisc.h>       // WTL utility classes like CString
#include <atlcrack.h>      // WTL enhanced msg map macros

extern CAppModule _Module; // WTL version of CComModule

//-----------------------------------------------------------------------------
class CAbougDlg : public CDialogImpl<CAbougDlg>
{
	friend class CDialogImpl<CAbougDlg>;
	enum { IDD = IDD_ABOUTBOX };
public:

protected:
	BEGIN_MSG_MAP_EX(CAbougDlg)
		COMMAND_ID_HANDLER(IDOK, OnOk)
	END_MSG_MAP()

	LRESULT OnOk( WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL &bHandled )
	{
		EndDialog( wID );
		return 0;
	}
};
