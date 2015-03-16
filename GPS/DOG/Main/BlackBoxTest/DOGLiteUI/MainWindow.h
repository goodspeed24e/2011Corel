#pragma once

#define STRICT
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

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

#include <atlctrls.h>
#include <atldlgs.h>

//-----------------------------------------------------------------------------
#include "AbougDlg.h"
#include "WTLTabViewCtrl.h"
#include "DemuxPage.h"
#include "AudioPage.h"
#include "VideoPage.h"
#include "Video2ndPage.h"
#include "DisplayPage.h"


#include <windows.h>
#include <tchar.h>
#include <strsafe.h>

#include "DOG.h"
#include "DogCategory.h"
#include "SubCategory.h"
#include "DogUserEvent.h"

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>

#include <string>

#include <atlbase.h>
#ifndef _delayimp_h
extern "C" IMAGE_DOS_HEADER __ImageBase;
#endif

inline dog::IEventTracing* CreateDOGFromLocalDirDLL(HINSTANCE* phDLL)
{
	const TCHAR* DOGDLL_FileName = L"DOG.DLL";

	std::basic_string<TCHAR> ModulePath(MAX_PATH, NULL);
	::GetModuleFileName(reinterpret_cast<HMODULE>(&__ImageBase), &ModulePath[0], MAX_PATH);
	ModulePath.replace(ModulePath.begin() + ModulePath.find_last_of(_T('\\')) + 1, ModulePath.end(), DOGDLL_FileName);
	HINSTANCE hDLL = LoadLibrary(ModulePath.c_str());

	dog::IEventTracing* pDOG = NULL;

	if(hDLL)
	{
		typedef void* (*LPCreateComponentT)(void);
		LPCreateComponentT pCreateComponent = reinterpret_cast<LPCreateComponentT>(GetProcAddress(hDLL, "CreateComponent"));
		if(pCreateComponent)
		{
			pDOG = reinterpret_cast<dog::IEventTracing*>(pCreateComponent());
		}
		else
		{
			::FreeLibrary(hDLL);
			hDLL = NULL;
		}
	}

	if(phDLL)
	{
		*phDLL = hDLL;
	}

	return pDOG;
}

//-----------------------------------------------------------------------------
class CMyRichEditCtrl : public CWindowImpl<CMyRichEditCtrl, CRichEditCtrl>
{
public:
	DECLARE_WND_SUPERCLASS(NULL, CRichEditCtrl::GetWndClassName())

	BEGIN_MSG_MAP(CMyRichEditCtrl)
		// put your message handler entries here
	END_MSG_MAP()
};

class CMyTabViewCtrl : public CWTLTabViewCtrl
{
public:
	DECLARE_WND_SUPERCLASS(NULL, CWTLTabViewCtrl::GetWndClassName())

	BOOL PreTranslateMessage(MSG* pMsg)
	{
		pMsg;
		return FALSE;
	}

	BEGIN_MSG_MAP_EX(CMyTabViewCtrl)
		CHAIN_MSG_MAP(CWTLTabViewCtrl)
	END_MSG_MAP()
};

//-----------------------------------------------------------------------------
class CMyWindow : public CFrameWindowImpl<CMyWindow>,
				  public CUpdateUI<CMyWindow>, // support a UI update map
				  public CMessageFilter,       // prefilter the frame messages
				  public CIdleHandler,         // perform idle-time processing
				  public dog::IEventListener
{
	friend class CFrameWindowImpl<CMyWindow>;
	friend class CUpdateUI<CMyWindow>;
	DECLARE_FRAME_WND_CLASS(_T("DOG Lite UI Class"), IDR_MAINFRAME);

public:
	CMyWindow() : m_hDOGDLL(NULL), m_bRunFlag(false)
	{}

	~CMyWindow()
	{}

	void MainLoop()
	{
		while(m_bRunFlag)
		{
			if(m_AudioPage.IsWindowVisible())   { m_AudioPage.Paint();   }
			if(m_DemuxPage.IsWindowVisible())   { m_DemuxPage.Paint();   }
			if(m_DisplayPage.IsWindowVisible()) { m_DisplayPage.Paint(); }
			if(m_VideoPage.IsWindowVisible())   { m_VideoPage.Paint();   }
			if(m_Video2ndPage.IsWindowVisible()){ m_Video2ndPage.Paint();}
			boost::this_thread::sleep(boost::posix_time::milliseconds(50));
		}
	}

protected:
	// as required by CMessageFilter...
	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		if(CFrameWindowImpl<CMyWindow>::PreTranslateMessage(pMsg))
			return TRUE;

		return m_TabCtrl.PreTranslateMessage(pMsg);
	}

	// as required by CIdleHandler...
	virtual BOOL OnIdle()
	{
		UIUpdateToolBar();
		return FALSE;
	}

protected:
	CStatusBarCtrl m_StatusBar;
	CComPtr<dog::IEventTracing> m_pDOG;
	HINSTANCE m_hDOGDLL;

	boost::shared_ptr<boost::thread> m_pThread;
	bool m_bRunFlag;

	CMyTabViewCtrl m_TabCtrl;
	CMyRichEditCtrl m_RichEdit;
	CAudioPage m_AudioPage;
	CDemuxPage m_DemuxPage;
	CVideoPage m_VideoPage;
	CVideo2ndPage m_Video2ndPage;
	CDisplayPage m_DisplayPage;

	BEGIN_MSG_MAP(CMyWindow)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		COMMAND_ID_HANDLER(ID_APP_EXIT, OnExit)
		COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAbout)
		COMMAND_ID_HANDLER(ID_VIEW_STAY_ON_TOP, OnStayOnTop)
		REFLECT_NOTIFICATIONS()
		CHAIN_MSG_MAP(CUpdateUI<CMyWindow>)
		CHAIN_MSG_MAP(CFrameWindowImpl<CMyWindow>)
	END_MSG_MAP()

	// UI update handlers for the View|Toolbar and View|Status Bar menu items
	BEGIN_UPDATE_UI_MAP(CMyWindow)
		UPDATE_ELEMENT(ID_VIEW_TOOLBAR, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_VIEW_STATUS_BAR, UPDUI_MENUPOPUP)
	END_UPDATE_UI_MAP()

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		HMENU hmenu = LoadMenu ( _Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME) );
		SetMenu ( hmenu );

 		//CreateSimpleToolBar();

		CreateSimpleStatusBar(_T("SimpleStatusBar"));
		m_StatusBar.Attach(m_hWndStatusBar);

		m_hWndClient = m_TabCtrl.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE );
 		m_TabCtrl.ModifyTabStyle(0, TCS_BOTTOM, 0);
 		m_TabCtrl.ShowWindow(SW_SHOW);

		m_AudioPage.Create(m_TabCtrl.m_hWnd, rcDefault, NULL, WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, WS_EX_CLIENTEDGE);
		m_AudioPage.Init();
		m_TabCtrl.AddTab(L"Audio", m_AudioPage);

		m_DemuxPage.Create(m_TabCtrl.m_hWnd, rcDefault, NULL, WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, WS_EX_CLIENTEDGE);
		m_DemuxPage.Init();
		m_TabCtrl.AddTab(L"Demux", m_DemuxPage);

		m_VideoPage.Create(m_TabCtrl.m_hWnd, rcDefault, NULL, WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, WS_EX_CLIENTEDGE);
		m_VideoPage.Init();
		m_TabCtrl.AddTab(L"Video", m_VideoPage);

		m_Video2ndPage.Create(m_TabCtrl.m_hWnd, rcDefault, NULL, WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, WS_EX_CLIENTEDGE);
		m_Video2ndPage.Init();
		m_TabCtrl.AddTab(L"Video (2nd)", m_Video2ndPage);

		m_DisplayPage.Create(m_TabCtrl.m_hWnd, rcDefault, NULL, WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, WS_EX_CLIENTEDGE);
		m_DisplayPage.Init();
		m_TabCtrl.AddTab(L"Display", m_DisplayPage);

		m_RichEdit.Create(m_TabCtrl.m_hWnd, rcDefault, NULL, WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_HSCROLL | WS_VSCROLL | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | ES_NOHIDESEL | ES_SAVESEL, WS_EX_CLIENTEDGE);
		m_RichEdit.SetFont( HFONT(GetStockObject(DEFAULT_GUI_FONT)) );
		m_RichEdit.SetReadOnly(TRUE);
		m_TabCtrl.AddTab(L"Message", m_RichEdit);

		m_TabCtrl.SetActiveTab(0);


		// register object for message filtering and idle updates
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		ATLASSERT(pLoop != NULL);
		pLoop->AddMessageFilter(this);
		pLoop->AddIdleHandler(this);


		m_pDOG = CreateDOGFromLocalDirDLL(&m_hDOGDLL);
		m_pDOG->RegisterListener(this);
		m_pDOG->SetupDogConfigByString("log = all");

		m_bRunFlag = true;
		if(NULL == m_pThread) {
			m_pThread = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&CMyWindow::MainLoop, this)));
		}

		return 0;
	}

	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		m_bRunFlag = false;
		if(m_pThread) {
			m_pThread->join();
		}

		PostQuitMessage(0);
		return 0;
	}

	LRESULT OnExit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		PostMessage(WM_CLOSE);
		return 0;
	}

	LRESULT OnStayOnTop(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		RECT rcWindow;
		CMenuHandle menubar(GetMenu());
		DWORD state = menubar.CheckMenuItem(ID_VIEW_STAY_ON_TOP, 0);
		if(MF_UNCHECKED == state)
		{
			SetWindowPos(HWND_TOPMOST, &rcWindow, SWP_NOMOVE | SWP_NOSIZE);
			menubar.CheckMenuItem(ID_VIEW_STAY_ON_TOP, MF_CHECKED);
		}
		else
		{
			SetWindowPos(HWND_NOTOPMOST, &rcWindow, SWP_NOMOVE | SWP_NOSIZE);
			menubar.CheckMenuItem(ID_VIEW_STAY_ON_TOP, MF_UNCHECKED);
		}

		return 0;
	}

	LRESULT OnAbout(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		CAbougDlg abougDlg;
		abougDlg.DoModal();
		return 0;
	}

protected:
	void STDCALL NotifyEvent(const dog::DogEvent& event)
	{
		using namespace dog;
		switch(event.DataType)
		{
		case STRING_MESSAGE_EVENT_TYPE:			
			{
				const StringEvent& strEvent = (StringEvent&)(event);
				if( 1 == strEvent.nBytesOfChar)
				{
					const char* pAnsiStr = strEvent.szMessage;
					wchar_t pWcharStr[256] =  L"";
					MultiByteToWideChar(CP_ACP, 0, pAnsiStr, (int)strlen(pAnsiStr)+1, pWcharStr, sizeof(pWcharStr)/sizeof(wchar_t));
					StatusMessage(L"received StringEvent: time=%f, %s \n", event.TimeStamp.QuadPart/1e7, pWcharStr);
					m_RichEdit.AppendText(pWcharStr);
					m_RichEdit.AppendText(L"\n");
				}
				else
				{
					StatusMessage(L"received StringEvent: time=%f, %s \n", event.TimeStamp.QuadPart/1e7, strEvent.szMessageW);
					m_RichEdit.AppendText(strEvent.szMessageW);
					m_RichEdit.AppendText(L"\n");
				}

			}
			break;

		case DATA_PROCESS_EVENT_TYPE:
		case BUFFER_EVENT_TYPE:
			if(sizeof(BufferEvent) == event.Size)
			{
				const BufferEvent& bufEvent = (BufferEvent&)(event);
				m_AudioPage.UpdateBufferEvent(bufEvent);
				m_DemuxPage.UpdateBufferEvent(bufEvent);
			}
			break;
		case VARIANT_DATA_EVENT_TYPE:
			if(sizeof(VariantDataEvent) == event.Size)
			{
				const VariantDataEvent& dataEvent = (VariantDataEvent&)(event);
				m_AudioPage.UpdateDataEvent(dataEvent);
				m_VideoPage.UpdateDataEvent(dataEvent);
				m_Video2ndPage.UpdateDataEvent(dataEvent);
				m_DisplayPage.UpdateDataEvent(dataEvent);
			}
		default:
			break;
		}

	}

	void StatusMessage(const TCHAR* fmt, ...)
	{
		va_list va;
		TCHAR szBuffer[512] = _T("");
		const size_t nSize = sizeof(szBuffer)/sizeof(TCHAR) -1;

		va_start(va, fmt);
		_vsntprintf_s(szBuffer, nSize, nSize-1, fmt, va);
		va_end(va);

		szBuffer[nSize] = 0;
		m_StatusBar.SetText(0, szBuffer, 0);
	}
};
