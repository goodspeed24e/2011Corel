// DispSvrDSProxy.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "DispSvrDSProxy.h"
#include "DispSvrPropPage.h"
#include "EVRPresenterUuid.h"


#ifdef _MANAGED
#pragma managed(push, off)
#endif

extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID); // The streams.h DLL entrypoint.
//DllGetClassObject and DllCanUnloadNow are also implemented in DllEntry.cpp of DShow BaseClasses lib
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	//In DllEntry.cpp of DShow BaseClasses lib
	//Call it before calling AMovieDllRegisterServer2 function
	return DllEntryPoint(hModule, ul_reason_for_call, lpReserved);
}

#ifdef _MANAGED
#pragma managed(pop)
#endif

//// This is an example of an exported variable
//DISPSVRDSPROXY_API int nDispSvrDSProxy=0;
//
//// This is an example of an exported function.
//DISPSVRDSPROXY_API int fnDispSvrDSProxy(void)
//{
//	return 42;
//}

// This is the constructor of a class that has been exported.
// see DispSvrDSProxy.h for the class definition

AMOVIESETUP_FILTER CDispSvrDSProxy::m_FilterInfo =
{
	&CLSID_DispSvrProxy,				// clsID
	DISPSVR_PROXY_FILTER_NAME,		// strName
	MERIT_DO_NOT_USE,					// dwMerit (690000 isn't large enough to bypass Microsoft filter)
	NULL,
	NULL		// lPin
};
//g_Templates and g_cTemplates are fixed name used in dllsetup.cpp of DShow BaseClasses lib
//For DllGetClassObject in DllEntry.cpp
CFactoryTemplate g_Templates[] =
{
	{
		DISPSVR_PROXY_FILTER_NAME,
		&CLSID_DispSvrProxy,
		CDispSvrDSProxy::CreateInstance,
		NULL,
		&CDispSvrDSProxy::m_FilterInfo
	},
	{ 
		DISPSVR_PROXY_PROPERTY_PAGE,
		&CLSID_DispSvrProperty,
		CDispSvrPropPage::CreateInstance,
		NULL,
		NULL
	},
};

int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);
static CDispSvrDSProxy *g_pDispSvrProxy = NULL;
CDispSvrDSProxy::CDispSvrDSProxy(const TCHAR *pName,
								 LPUNKNOWN pUnk):CBaseFilter( pName, pUnk, &m_CritSec, *m_FilterInfo.clsID)
{
	m_hGraphEdit = NULL;
	m_pDispSvr = NULL;
	m_pRenderEngine = NULL;
	m_pVidSrc = NULL;
	m_pDispVidMixer = NULL;
	m_hVideo = NULL;
	m_pVideoRenderer = NULL;
	m_dwRendererMode = enRendererMode_None;
	m_pCustomPresenter = NULL;

	m_dwDispSvrInitFlags =	DISPSVR_USE_D3D9EX |
										DISPSVR_USE_STECIL_BUFFER |
										DISPSVR_DEVICE_LOST_NOTIFY |
										DISPSVR_DETECT_D3D_HIJACK;

	g_pDispSvrProxy = this;
}

CDispSvrDSProxy::~CDispSvrDSProxy()
{
	if (m_pCustomPresenter)
		m_pCustomPresenter->Release();

	TeminateDispSvr();
	g_pDispSvrProxy = NULL;
}


CUnknown * WINAPI CDispSvrDSProxy::CreateInstance(LPUNKNOWN pUnk, HRESULT *pHr) 
{
	CDispSvrDSProxy *pNewObject = new CDispSvrDSProxy(NAME("Corel DispSvr Proxy"), pUnk);
	if (pNewObject == NULL) 
	{
		*pHr = E_OUTOFMEMORY;
	}
	return pNewObject;
} 
/////////////////////////////////////////
//
//	CUnkown Implementation
//
/////////////////////////////////////////
STDMETHODIMP CDispSvrDSProxy::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
	if (riid == IID_ISpecifyPropertyPages)
	{
		return GetInterface((ISpecifyPropertyPages*)this, ppv);
	}
	else if (riid == IID_IKsPropertySet)
	{
		return GetInterface((IKsPropertySet*)this, ppv);
	}
	else
		return CBaseFilter::NonDelegatingQueryInterface(riid, ppv);
}

/////////////////////////////////////////
//
//	IBaseFilter Implementation
//
/////////////////////////////////////////
STDMETHODIMP CDispSvrDSProxy::JoinFilterGraph(IFilterGraph * pGraph, LPCWSTR pName)
{
	if (!pGraph) // GraphEdit tries to remove DispSvrDSProxy
	{
		TeminateDispSvr();
	}
	return CBaseFilter::JoinFilterGraph( pGraph, pName);
}
/////////////////////////////////////////
//
//	CBaseFilter Implementation
//
/////////////////////////////////////////
int CDispSvrDSProxy::GetPinCount()
{
	return NULL;
}

CBasePin* CDispSvrDSProxy::GetPin(int n)
{
	return NULL;
}

/////////////////////////////////////////
//
//	ISpecifyPropertyPages Implementation
//
/////////////////////////////////////////
STDMETHODIMP CDispSvrDSProxy::GetPages(CAUUID *pPages)
{
	if (!pPages) return E_POINTER;

	pPages->cElems = 1;
	pPages->pElems = reinterpret_cast<GUID*>(CoTaskMemAlloc(sizeof(GUID)));
	if (pPages->pElems == NULL) 
	{
		return E_OUTOFMEMORY;
	}
	*(pPages->pElems) = CLSID_DispSvrProperty;
	return S_OK;
}

/////////////////////////////////////////
//
//	IKsPropertySet Implementation
//
/////////////////////////////////////////
STDMETHODIMP CDispSvrDSProxy::Set(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData)
{
	if (IsEqualGUID( guidPropSet, PROPSET_DISPSVR))
	{
		if (dwPropID == PROP_GRAPHEDIT_HWND)
		{
			m_hGraphEdit = ((HWND)pPropData);
			return S_OK;
		}
		else if (dwPropID == PROP_LOAD_DISPSVR)
			return InitDispSvr();
		else if (dwPropID == PROP_LOAD_MS_CUSTOM_EVR) //Load Custom Presenter of MS
			return InitMSCustomPresenter();
		else if (dwPropID == PROP_PROCESS_DEVICE_LOST)
			return ProcessDeviceLost();
		else if (dwPropID == PROP_DETACH_DISPSVR)
			return DetachDispSvr();
		else if (dwPropID == PROP_DISPSVR_INIT_FLAG)
			if (pPropData)
			{
				 m_dwDispSvrInitFlags = *((DWORD *)pPropData);
				return S_OK;
			}
			else
				return E_INVALIDARG;
		else
			return E_UNEXPECTED;
	}
	else
		return E_NOTIMPL;
}

STDMETHODIMP CDispSvrDSProxy::Get(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData, DWORD *pcbReturned)
{
	if (IsEqualGUID( guidPropSet, PROPSET_DISPSVR))
	{
		if (dwPropID == PROP_DISPSVR_INIT_FLAG)
		{
			if (pPropData)
			{
				*((DWORD *)pPropData) = m_dwDispSvrInitFlags;
				if (pcbReturned)
					*pcbReturned = sizeof(m_dwDispSvrInitFlags);
				return S_OK;
			}
			else
				return E_INVALIDARG;
		}
		else
			return E_NOTIMPL;
	}
	else
		return E_NOTIMPL;
}
STDMETHODIMP CDispSvrDSProxy::QuerySupported(REFGUID guidPropSet, DWORD dwPropID, DWORD *pTypeSupport)
{
	return E_NOTIMPL;
}


HRESULT CDispSvrDSProxy::InitDispSvr()
{

	if (m_pDispSvr)
	{
		m_pDispSvr->Terminate();
		m_pDispSvr->Release();
		m_pDispSvr = NULL;
	}
	if (FAILED(InitVideoDialog()))
		return E_FAIL;

	::CoCreateInstance(CLSID_DisplayServer, NULL, CLSCTX_INPROC_SERVER, IID_IDisplayServer, (LPVOID *)&m_pDispSvr);
	//m_pDispSvr.CoCreateInstance(CLSID_DisplayServer);
	if (m_pDispSvr)
	{
		HRESULT hr = E_FAIL;
/*
		DWORD ddFlags = 0, dwDefaultVal = 0;
		switch (dwDefaultVal)
		{
		case 0:
			break;
		case 2:
			ddFlags |= DISPSVR_USE_CUSTOMIZED_OUTPUT;
			break;
		default:
			ddFlags |= DISPSVR_USE_CUSTOMIZED_OUTPUT;
			break;
		}

		//	ddFlags |= DISPSVR_WAITING_FOR_VSYNC;

		ddFlags |= DISPSVR_DEVICE_LOST_NOTIFY;

		ddFlags |= DISPSVR_DETECT_D3D_HIJACK;

		ddFlags |= DISPSVR_USE_D3D9EX;

		//ddFlags |= DISPSVR_USE_RT_VIRTUALIZATION;

		ddFlags |= DISPSVR_USE_STECIL_BUFFER;

		//		ddFlags |= DISPSVR_SUPPORT_EXCLUSIVE_MODE;

		//		ddFlags |= DISPSVR_USE_MESSAGE_THREAD;

		//	ddFlags |= DISPSVR_USE_EXCLUSIVE_MODE;

		//		m_VideoDialog.ShowWindow(SW_SHOW);
*/
		hr = m_pDispSvr->Initialize(m_dwDispSvrInitFlags, m_hVideo, NULL);

		if (SUCCEEDED(hr))
			hr = m_pDispSvr->GetRenderEngine(&m_pRenderEngine);
		
		if (SUCCEEDED(hr))
		{
			RECT rect;
			::GetWindowRect(m_hVideo, &rect);
			//::ClientToScreen(m_hVideo, &rect);
			hr = m_pRenderEngine->SetDisplayWindow(m_hVideo, &rect);
		}

		if (SUCCEEDED(hr))
			hr = InsertRendererFilter();

		//VMR9,	Config VideoRenderer BEFORE attaching DispSvr
		//EVR,	Config VideoRenderer After attaching DispSvr
		if (m_dwRendererMode == enRendererMode_VMR9)
			hr = ConfigVideoRenderer();

		if (SUCCEEDED(hr))
			hr = AttachDispSvr();
		
		if (m_dwRendererMode == enRendererMode_EVR)
			hr = ConfigVideoRenderer();

		if (m_hGraphEdit)
		{
			::PostMessage( m_hGraphEdit, WM_KEYDOWN, VK_F5, NULL); 
			::PostMessage( m_hGraphEdit, WM_KEYUP, VK_F5, NULL);
		}
		return hr;

	}
	else
		return E_UNEXPECTED;

	return TRUE;
}
HRESULT CDispSvrDSProxy::TeminateDispSvr()
{
	//Add a ref count avoiding GraphEdit destroy DispSvrDSProxy while detaching dispsvr
	IFilterGraph *t_pGraph = NULL;
	if (m_pGraph)
	{
		t_pGraph = m_pGraph;
		t_pGraph->AddRef();
	}

	RemoveRendererFilter();

	if(m_pDispSvr)
	{
		if (m_pRenderEngine)
		{
			m_pRenderEngine->Terminate();
			m_pRenderEngine->Release();
			m_pRenderEngine = NULL;
		}

		m_pDispSvr->Terminate();
		m_pDispSvr->Release();
		m_pDispSvr = NULL;
	}
	if (m_hVideo)
	{
		DestroyWindow(m_hVideo);
		m_hVideo = NULL;
		//Unregister Window Class, otherwise InitVideoDialog will be failed next time.
		UnregisterClass ("VideoWndClass", GetModuleHandle(NULL));
	}

	if (t_pGraph)
	{
		t_pGraph->Release();
		t_pGraph = NULL;
	}

	if (m_hGraphEdit)
	{
		::PostMessage( m_hGraphEdit, WM_KEYDOWN, VK_F5, NULL); 
		::PostMessage( m_hGraphEdit, WM_KEYUP, VK_F5, NULL);
	}
	return S_OK;
}


HRESULT CDispSvrDSProxy::InitMSCustomPresenter()
{
	HRESULT hr = E_FAIL;
	if (FAILED(InitVideoDialog()))
		return E_FAIL;

	hr = ::CoCreateInstance(CLSID_CustomEVRPresenter, NULL, CLSCTX_INPROC_SERVER, IID_IMFVideoPresenter, (LPVOID *)&m_pCustomPresenter);

	if (!m_pCustomPresenter)
		return E_FAIL;

	if (SUCCEEDED(hr))
		hr = InsertRendererFilter();

	if (SUCCEEDED(hr))
	{
		IMFVideoRenderer *pEvr = NULL;
		hr = m_pVideoRenderer->QueryInterface(IID_IMFVideoRenderer, (void **)&pEvr);
		if (SUCCEEDED(hr) && pEvr)
		{
			hr = pEvr->InitializeRenderer( NULL, m_pCustomPresenter);

			hr = ConfigVideoRenderer();

			IMFGetService *pService = NULL;
			hr = m_pCustomPresenter->QueryInterface(__uuidof(IMFGetService), (void **)&pService);
			if (SUCCEEDED(hr) && pService)
			{
				IMFVideoDisplayControl *pDisplayControl;
				hr = pService->GetService(MR_VIDEO_RENDER_SERVICE, IID_IMFVideoDisplayControl, (void **) &pDisplayControl);
				if (SUCCEEDED(hr) && pDisplayControl)
				{
					hr = pDisplayControl->SetVideoWindow(m_hVideo);
					MFVideoNormalizedRect VideoSrcRect = {0};
					VideoSrcRect.right = 1.0;
					VideoSrcRect.bottom = 1.0;
					RECT rect;
					::GetWindowRect(m_hVideo, &rect);
					hr = pDisplayControl->SetVideoPosition( NULL, &rect);

					pDisplayControl->Release();
				}
				pService->Release();
			}
			pEvr->Release();
		}
	}
	return S_OK;
}

LRESULT CALLBACK CDispSvrDSProxy::VideoWndProc(HWND hwnd , UINT msg , WPARAM wp , LPARAM lp)
{

	switch (msg) {
		case WM_MOVE:
			{
				if (g_pDispSvrProxy)
				{
					IDisplayRenderEngine *pEngine = NULL;
					g_pDispSvrProxy->GetRenderEngine(&pEngine);
					if (pEngine)
					{
						RECT tempRect;
						RECT rect;
						::GetClientRect(hwnd, &tempRect);
						rect.left = (int)(short) LOWORD(lp);   // horizontal position 
						rect.top = (int)(short) HIWORD(lp);   // vertical position 
						rect.right = rect.left + tempRect.right;
						rect.bottom = rect.top + tempRect.bottom;
						pEngine->SetDisplayWindow(hwnd, &rect);

						pEngine->Release();
						pEngine = NULL;
					}
				}
			}
				break;
	//case WM_DESTROY:
	//	PostQuitMessage(0);
	//	break;
//		return 0;
	//case WM_CREATE:
	//	return 0;
	//case WM_COMMAND:
	//	switch(LOWORD(wp)) {
	//	return 0;
	case WM_CLOSE:
		::PostMessage( hwnd, WM_SYSCOMMAND, SC_MINIMIZE, NULL);
		return 0;
	}
	return DefWindowProc(hwnd , msg , wp , lp);
}

HRESULT CDispSvrDSProxy::InitVideoDialog()
{

	WNDCLASS wc; 

	// Register the main window class. 
	wc.style = CS_HREDRAW | CS_VREDRAW; 
	wc.lpfnWndProc = (WNDPROC)CDispSvrDSProxy::VideoWndProc; 
	wc.cbClsExtra = 0; 
	wc.cbWndExtra = 0; 
	wc.hInstance = GetModuleHandle(NULL); 
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION); 
	wc.hCursor = LoadCursor(NULL, IDC_ARROW); 
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH); 
	wc.lpszMenuName =  NULL; 
	wc.lpszClassName = "VideoWndClass"; 

	if (!RegisterClass(&wc)) 
		return FALSE; 


	m_hVideo = CreateWindow(
		"VideoWndClass",			/*class*/
		"Corel Video Window",		/*window name*/
		WS_CAPTION | WS_VISIBLE | WS_OVERLAPPEDWINDOW,			/*class*/
		0,				/* x pos*/
		0,				/* y pos*/
		640,				/*width*/
		480,					/*height*/
		NULL,/*parent handle*/
		NULL,				/*child window id*/
		GetModuleHandle(NULL),			/*application instance id*/
		NULL				/*window data*/
		);

	return TRUE;
}

HRESULT CDispSvrDSProxy::InsertRendererFilter()
{
	HRESULT hr = E_FAIL;
	if(!m_pGraph)
		return E_UNEXPECTED;

//	m_pEvr = NULL;
//	m_pEvr.CoCreateInstance(CLSID_EnhancedVideoRenderer);
	//OSVERSIONINFOEX VerEX;
	//VerEX.dwOSVersionInfoSize = sizeof(VerEX);
	//GetVersionEx((OSVERSIONINFO *)&VerEX);

	GUID CLSID_SelectedVideoRenderer;
	LPCWSTR pwsSelectedString;
	//g_osInfo is global variable in ellentry.cpp of BaseClasses, load OS info when DllEntryPoint called.
	if (g_osInfo.dwMajorVersion >= 6)
	{
		m_dwRendererMode = enRendererMode_EVR;
		memcpy_s(&CLSID_SelectedVideoRenderer, sizeof(GUID), &CLSID_EnhancedVideoRenderer, sizeof(CLSID_EnhancedVideoRenderer));
		pwsSelectedString = L"Custom EVR";
	}
	else
	{
		m_dwRendererMode = enRendererMode_VMR9;
		memcpy_s(&CLSID_SelectedVideoRenderer, sizeof(GUID), &CLSID_VideoMixingRenderer9, sizeof(CLSID_VideoMixingRenderer9));
		pwsSelectedString = L"Custom VMR 9";
	}

	::CoCreateInstance(CLSID_SelectedVideoRenderer, NULL , CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&m_pVideoRenderer);
	if (!m_pVideoRenderer)
		return E_UNEXPECTED;

//	hr = ConfigVideoRenderer();
	hr = m_pGraph->AddFilter(m_pVideoRenderer, pwsSelectedString);

	return hr;
}

HRESULT CDispSvrDSProxy::RemoveRendererFilter()
{
	if (m_pVideoRenderer)
	{
		if (m_pGraph)
			m_pGraph->RemoveFilter(m_pVideoRenderer);

		m_pVideoRenderer->Release();
		m_pVideoRenderer = NULL;
	}
	return DetachDispSvr();
}

HRESULT CDispSvrDSProxy::ConfigVideoRenderer()
{
	HRESULT hr = E_FAIL;
	if (!m_pVideoRenderer)
		return E_UNEXPECTED;
	ULONG ref = 0;
	if (m_dwRendererMode == enRendererMode_VMR9)
	{
		IVMRFilterConfig9 *pVMRConfig9;
		hr = m_pVideoRenderer->QueryInterface(IID_IVMRFilterConfig9, (void **)&pVMRConfig9);
		if (SUCCEEDED(hr) && pVMRConfig9)
		{
			hr = pVMRConfig9->SetNumberOfStreams(2);
			ref = pVMRConfig9->Release();
			pVMRConfig9 = NULL;
		}
		return hr;
	}
	else if (m_dwRendererMode == enRendererMode_EVR)
	{
		IEVRFilterConfig *pEVRConfig;
		hr = m_pVideoRenderer->QueryInterface(IID_IEVRFilterConfig, (void **)&pEVRConfig);
		if (SUCCEEDED(hr) && pEVRConfig)
		{
			hr = pEVRConfig->SetNumberOfStreams(2);
			ref = pEVRConfig->Release();
			pEVRConfig = NULL;
		}
		return hr;
	}
	else
		return E_UNEXPECTED;
}

HRESULT CDispSvrDSProxy::ChildObjectControl(IUnknown *pUnk, BOOL bInsert, ULONG lZOrder)
{
	try
	{
		if (!m_pDispSvr || !m_pRenderEngine)
			throw;

		HRESULT hr = E_FAIL;
		IParentDisplayObject *pParentDO;

		m_pRenderEngine->GetRootObject(IID_IParentDisplayObject, (void**) &pParentDO);

		if (!pParentDO)
			throw;

		if (bInsert)
			pParentDO->AddChild(lZOrder, pUnk);
		else
			pParentDO->RemoveChild(pUnk);

		pParentDO->Release();
		pParentDO = NULL;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}

	return S_OK;
}

HRESULT CDispSvrDSProxy::AttachDispSvr()
{
	HRESULT hr = E_FAIL;

	if (!m_pVideoRenderer)
		return E_UNEXPECTED;

	hr = ::CoCreateInstance(CLSID_VideoSourceDisplayObject, NULL, CLSCTX_INPROC_SERVER, IID_IDisplayVideoMixer, (void**)&m_pDispVidMixer);

	if (FAILED(hr) || !m_pDispVidMixer)
		return hr;

	hr = ChildObjectControl(m_pDispVidMixer, TRUE, GPI_DISPSVR_ZORDER_MAIN_VIDEO);
	if (FAILED(hr))
		return hr;

	hr = m_pDispVidMixer->AddVideoSource(m_pVideoRenderer, &m_pVidSrc);
	if (FAILED(hr))
		return hr;

	return S_OK;
}

HRESULT CDispSvrDSProxy::DetachDispSvr()
{
	if (m_pDispVidMixer)
	{
		m_pDispVidMixer->RemoveVideoSource(m_pVidSrc);
		ChildObjectControl(m_pDispVidMixer, FALSE);

		m_pVidSrc = NULL;
		m_pDispVidMixer = NULL;
	}
	return S_OK;
}


HRESULT CDispSvrDSProxy::ProcessDeviceLost()
{
	if (m_pRenderEngine)
		m_pRenderEngine->ProcessRequest(DISPLAY_REQUEST_ProcessLostDevice, NULL, NULL);

	return S_OK;
}

HRESULT CDispSvrDSProxy::GetRenderEngine(IDisplayRenderEngine **pEngine)
{
	if (!pEngine)
		return E_INVALIDARG;

	*pEngine = m_pRenderEngine;
	if (*pEngine)
	{
		(*pEngine)->AddRef();
		return S_OK;
	}
	else
		return S_FALSE;
}

STDAPI DllRegisterServer()
{
	return AMovieDllRegisterServer2( TRUE );
}

STDAPI DllUnregisterServer()
{
	return AMovieDllRegisterServer2( FALSE );
}

//STDAPI
//DllCanUnloadNow()
//{
//	UINT count = CBaseObject::ObjectsActive();
//	DbgLog((LOG_MEMORY,2,TEXT("DLLCanUnloadNow called - DispSvrDSProxy = %s, Active objects = %d"),g_pDispSvrProxy ? "TRUE" : "FALSE" ,count));
//
//	if (count > 0)
//		return S_FALSE;
//	else
//		return S_OK;
///*
//	if (!g_pDispSvrProxy)
//	{
//		if (count > 0)
//			return S_FALSE;
//		else
//			return S_OK;
//	}
//	else
//	{
//		if (count > 1) // PropertyPage opened. ignore this time and return S_FALSE;
//			return S_FALSE;
//
//		DbgLog((LOG_MEMORY,2,TEXT("DLLCanUnloadNow called - TeminateDispSvr")));
//		g_pDispSvrProxy->TeminateDispSvr(); //DispSvrDSProxy is indirectly deleted in this function.
//
//		count = CBaseObject::ObjectsActive(); // re-check.
//
//		if (count > 0)
//			return S_FALSE;
//		else
//			return S_OK;
//	}
//	//	CClassFactory::IsLocked(),
//	//	CBaseObject::ObjectsActive()));
//
//	//if (CClassFactory::IsLocked() || CBaseObject::ObjectsActive()) {
//	//	return S_FALSE;
//	//} else {
//	//	return S_OK;
//	//}
//	return S_OK;
//*/
//}
