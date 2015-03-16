// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER				// Allow use of features specific to Windows XP or later.
#define WINVER 0x0501		// Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE 0x0600	// Change this to the appropriate value to target other versions of IE.
#endif

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>



// TODO: reference additional headers your program requires here
//#include <initguid.h>
#include <tchar.h>
#include <dshow.h>
#include <streams.h>
#include <initguid.h>
#include <d3d9.h>
#include <vmr9.h>
#include <evr.h>
#include <Mfidl.h>
#include "../../Exports/Inc/DispSvr.h"

// {CDECA9EE-D6C6-4b78-9B14-87639C4C65A3}
DEFINE_GUID(CLSID_DispSvrProxy, 
			0xcdeca9ee, 0xd6c6, 0x4b78, 0x9b, 0x14, 0x87, 0x63, 0x9c, 0x4c, 0x65, 0xa3);

// {CFAFAFE8-F897-4049-8FBF-DB92DDF94C92}
DEFINE_GUID(CLSID_DispSvrProperty, 
			0xcfafafe8, 0xf897, 0x4049, 0x8f, 0xbf, 0xdb, 0x92, 0xdd, 0xf9, 0x4c, 0x92);

// {2EA2F54A-F293-4e37-9ECF-5E413591E8E5}
DEFINE_GUID(PROPSET_DISPSVR, 
			0x2ea2f54a, 0xf293, 0x4e37, 0x9e, 0xcf, 0x5e, 0x41, 0x35, 0x91, 0xe8, 0xe5);


#define PROP_GRAPHEDIT_HWND											0
#define PROP_LOAD_DISPSVR												1
#define PROP_LOAD_MS_CUSTOM_EVR									2
#define PROP_PROCESS_DEVICE_LOST									3
#define PROP_DETACH_DISPSVR											4
#define PROP_DISPSVR_INIT_FLAG										5


#define DISPSVR_PROXY_FILTER_NAME				L"Corel DispSvr Proxy"
#define DISPSVR_PROXY_PROPERTY_PAGE			L"DispSvr Property"
#define	GPI_DISPSVR_ZORDER_MAIN_VIDEO		800
