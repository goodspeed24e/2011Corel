#include "stdafx.h"

#include "HDMIStereoModeHelper.h"
#include "Imports/LibHijackDetect/HijackDetect.h"

#include "CompositeDO.h"
#include "ServerStateEventSink.h"
#include "DriverExtensionHelper.h"
#include "RenderClock.h"
#include "RenderEngine.h"

#define TIMEOUT_RENDERPRESENT_SEMAPHORE 200     // we set a timeout in case only one render call is received without according present call.


#define DISP_AUTO_PROFILE(id, subid) { if (m_bFirstTimeDisp) {AUTO_SCOPE_PROFILE(id, subid)}}

DEFINE_GUID(DXVADDI_Intel_ModeH264_E, 0x604f8e68, 0x4951, 0x4c54, 0x88, 0xfe, 0xab, 0xd2, 0x5c, 0x15, 0xb3, 0xd6);
DEFINE_GUID(DXVADDI_Intel_ModeH264_F, 0x604f8e69, 0x4951, 0x4c54, 0x88, 0xfe, 0xab, 0xd2, 0x5c, 0x15, 0xb3, 0xd6);
DEFINE_GUID(DXVA_ModeH264_MVC,   0x32fcfe3f, 0xde46,0x4a49,0x86,0x1b,0xac,0x71,0x11,0x6,0x49,0xd5);
DEFINE_GUID(DXVA_ModeH264_AMD_MVC, 0x9901ccd3, 0xca12, 0x4b7e, 0x86, 0x7a, 0xe2, 0x22, 0x3d, 0x92, 0x55, 0xc3);

using namespace DispSvr;


CRenderEngine::CRenderEngine(LPUNKNOWN pUnk, HRESULT *phr) : CUnknown(NAME("Display Render Engine"), pUnk)
{
    m_pD3D9Helper = new CD3D9Helper;
    m_bInitialized = FALSE;
	m_eDeviceStatus = eDeviceReady;
	m_bAutoRenderThread = TRUE;
	m_bOutOfMemory = FALSE;
	m_hRenderThread = NULL;
	m_bFreezeState = FALSE;
	m_bEnableRendering = TRUE;
	SetBackgroundColor(RGB(0x00, 0x00, 0x00));

	m_RenderThreadStatus = eNotStarted;
	ZeroMemory(&m_rectDisp,sizeof(m_rectDisp));
	ZeroMemory(&m_rectSrc, sizeof(m_rectSrc));
	ZeroMemory(&m_rectWindow, sizeof(m_rectWindow));

	// make sure timer is at least 2 ms accurate
	timeBeginPeriod(2);
	m_pHJDetect = NULL;

	m_hMonitor = 0;
	m_hwnVideoWindow = 0;
	m_pfnRenderFunc = &CRenderEngine::RenderDispObj;
	m_bHide = FALSE;
	m_pStateEventSink = CServerStateEventSink::GetInstance();
	m_dwInterval = 1000 / 30;	// default frame rate set to 30fps
    m_bForceClearRenderTarget = FALSE;
    m_dwClearRenderTargetTimes = 0;
	m_dwFrameRate = 60;
	m_dwVideoFormat = OR_FRAME_PROGRESSIVE;
	m_bCheckDisplayChange = FALSE;
    m_bStereoMode = FALSE;
	m_bUseMessageThread = FALSE;
	m_bIsOverlayPresenter = false;
    m_bIsD3D9RenderTarget = false;
    m_bFirstTimeDisp = true;
	m_nMonPowerStatus = 0;
    m_bExclusiveMode = FALSE;

    m_bRestoreMixerStereoEnable = FALSE;
    m_dwRestoreMixerStereoMode = MIXER_STEREO_MODE_DISABLED;
    m_hRenderPresentSemaphore = CreateSemaphore(NULL, 1, 1, NULL);
    m_bIsDWMEnabled = FALSE;
	memset(&m_DeviceInfo, 0, sizeof(m_DeviceInfo));
	CHDMIStereoModeHelper::GetHelper(&m_pHDMIHelper);
    m_eD3D9HelperRenderStateType = D3D9HELPER_RENDERSTATE_NO_CHANGE;
    m_eWindowState = RESOURCE_WINDOW_STATE_WINDOWED;
    m_dwColorKey = 0;
	m_bEnableHDMIStereoAfterDeviceReset = FALSE;
	m_bHideOverlay = FALSE;
}

CRenderEngine::~CRenderEngine()
{
	// call off the timer
	timeEndPeriod(2);
	Terminate();
    SAFE_RELEASE(m_pHDMIHelper);
    SAFE_DELETE(m_pD3D9Helper);
	m_hwnVideoWindow = 0;
    CloseHandle(m_hRenderPresentSemaphore);
}

//////////////////////////////////////////////////////////////////////////
// IUnknown

STDMETHODIMP CRenderEngine::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
	HRESULT hr = E_NOINTERFACE;
	*ppv = NULL;
	
	if (riid == IID_IDisplayRenderEngine)
	{
		hr = GetInterface((IDisplayRenderEngine*) this, ppv);
	}
    else if (riid == __uuidof(IDisplayRenderEngineProperty))
    {
        hr = GetInterface((IDisplayRenderEngineProperty *) this, ppv);
    }
    else if (riid == IID_IDisplayEventHost)
    {
        hr = GetInterface((IDisplayEventHost*) this, ppv);
    }
    else if (riid == IID_IDisplayServerStateEventSink)
    {
        hr = m_pStateEventSink->QueryInterface(riid, ppv);
    }
    else if (riid == __uuidof(IDispSvrVideoMixer))
    {
        hr = CResourceManager::GetInstance()->GetInterface(DISPSVR_RESOURCE_VIDEOMIXER, __uuidof(IDispSvrVideoMixer), (void **)ppv);
    }
    else if (riid == __uuidof(IDispSvrVideoPresenter))
    {
        hr = CResourceManager::GetInstance()->GetInterface(DISPSVR_RESOURCE_VIDEOPRESENTER, __uuidof(IDispSvrVideoPresenter), (void **)ppv);
    }
    else
    {
        hr = CUnknown::NonDelegatingQueryInterface(riid, ppv);
    }
    return hr;
}

//////////////////////////////////////////////////////////////////////////
// IDisplayRenderEngine

STDMETHODIMP CRenderEngine::Initialize(HWND hWnd, UINT BackBufferWidth, UINT BackBufferHeight, IDisplayLock* pLock, DWORD dwFlags)
{
	HRESULT hr = S_OK;

	if (FALSE == IsWindow(hWnd))
	{
		DbgMsg("CRenderEngine::Initialize: received invalid window handle");
		return E_INVALIDARG;
	}

	if (m_bInitialized)
	{
		DbgMsg("CRenderEngine::Initialize: method was already called");
		return VFW_E_WRONG_STATE;
	}
	
	m_pLock = pLock;
	// For BD+ DiscoveryRAM Tags, BD+ Content Code would check the pattern at runtime.
	static unsigned char DATA_TAG_6[] =
	{0xE2, 0x7A, 0xAA, 0xA8, 0xD4, 0xD6, 0x6F, 0x4A, 0x1D, 0xAE, 0x5B, 0x57, 0xEE, 0x5C, 0xE1, 0x81};
	try
	{
        SetBackBufferSize(BackBufferWidth, BackBufferHeight);

		CAutoDisplayLock displayLock(m_pLock);
		
		//When drag APP from primary to extend monitor or reverse or DeviceLost, CreateWindow will give the error window handle
		//If m_hwnVideoWindow already has the value, don't call SetDisplayWindow again.

		if(NULL == m_hwnVideoWindow)
		{
			RECT rc = {0, 0, 0, 0};
			hr = SetDisplayWindow(hWnd, &rc);
			CHECK_HR(hr, DbgMsg("CRenderEngine::Initialize: failed to set display window"));
		}

		if (dwFlags & DISPSVR_NO_WINDOW_OUTPUT)
		{
			m_pfnRenderFunc = &CRenderEngine::RenderNoWindowOutput;
		}

		if (dwFlags & DISPSVR_DETECT_D3D_HIJACK)
		{
			m_pHJDetect = new CHijackDetect;
		}

		if (dwFlags & DISPSVR_USE_MESSAGE_THREAD)
			m_bUseMessageThread = TRUE;

        if (dwFlags & DISPSVR_USE_EXCLUSIVE_MODE)
			m_bExclusiveMode = TRUE;

		m_dwConfigFlags = dwFlags;

		hr = CreateD3DDevice();
		CHECK_HR(hr, DbgMsg("CRenderEngine::Initialize: failed to create D3D device"));

		CComPtr<IDisplayObject> pDO;
		hr = pDO.CoCreateInstance(CLSID_CompositeDisplayObject);
		CHECK_HR(hr, DbgMsg("CRenderEngine::Initialize: failed to create root DO"));

		hr = SetRootObject(pDO);
		CHECK_HR(hr, DbgMsg("CRenderEngine::Initialize: failed to set root DO"));

		m_pStateEventSink->Notify(SERVER_STATE_RESET, 0, 0);

		if ((m_dwConfigFlags & DISPSVR_SUSPENDED) != 0)
		{
			m_bEnableRendering = FALSE;
		}

		m_bInitialized = TRUE;

		if (!(m_dwConfigFlags & DISPSVR_NO_RENDER_THREAD))
		{
			hr = StartRenderingThread();
			// the logo in AllGDI mode was drawed by GDI, we need to clear D3D device first.
			NodeRequest(DISPLAY_REQUEST_ClearRenderTarget, 0, 1, 0);
			CHECK_HR(hr, DbgMsg("CWizard::Initialize: failed in StartRenderingThread_, hr = 0x%08x", hr));
		}
	}
	catch (HRESULT hrFailed)
	{
		DATA_TAG_6[2] ^= DATA_TAG_6[13];
		DATA_TAG_6[2] ^= DATA_TAG_6[13];
		hr = hrFailed;
		Terminate();
	}

	return hr;
}

STDMETHODIMP CRenderEngine::Terminate()
{
	m_bInitialized = FALSE;
	StopRenderingThread();

    if (m_pHDMIHelper)
    {
        BOOL bReCreateDevice = FALSE;
        m_pHDMIHelper->EnableHDMIStereoMode(FALSE, NULL, &bReCreateDevice);
    }

	if (m_pRootDO)
		m_pRootDO->BeginDeviceLoss();	
	if (m_pOwner)
		m_pOwner->BeginDeviceLoss();

    //Send terminated message when terminating RenderEngine.
    if (CResourceManager::GetInstance())
        CResourceManager::GetInstance()->ProcessMessage(RESOURCE_MESSAGE_TERMINATE, NULL);

	SetRootObject(NULL);
	ReleaseD3DDevice();

	if (m_pHJDetect)
	{
		delete m_pHJDetect;
		m_pHJDetect = NULL;
	}

	m_pLock.Release();

	if(m_RenderEngineEventNotify.size() != 0)
		DbgMsg("RenderEngine remains some notify objects not unadvised! count = %d", m_RenderEngineEventNotify.size());

	return S_OK;
}

HRESULT CRenderEngine::WaitDeviceChange()
{
    // wait until the device change is done
    while (m_eDeviceStatus == eDeviceChanging)
    {
        Sleep(10);
    }

    if(m_eDeviceStatus == eDeviceRecoverable)
        return E_FAIL;

    return S_OK;
}

STDMETHODIMP CRenderEngine::SetBackBufferSize(UINT BackBufferWidth, UINT BackBufferHeight)
{
	return S_OK;
}

STDMETHODIMP CRenderEngine::GetBackBufferSize(UINT* pBackBufferWidth, UINT* pBackBufferHeight)
{
	if (!pBackBufferWidth || !pBackBufferHeight)
		return E_POINTER;
    
    CAutoDisplayLock displayLock(m_pLock);
    return m_pD3D9Helper->GetBackBufferSize(pBackBufferWidth, pBackBufferHeight);
}

static const NORMALIZEDRECT rectClippingSpace = { -1.0f, 1.0f, 1.0f, -1.0f };

HRESULT CRenderEngine::RenderNoWindowOutput()
{
    HRESULT hr = S_OK;

	if (m_pRootDO)
	{
        m_pD3D9Helper->SetRenderStates(D3D9HELPER_RENDERSTATE_NO_WINDOW_OUTPUT);
		hr = m_pRootDO->Render(m_pD3D9Helper->GetDevice(), &rectClippingSpace);
        m_pD3D9Helper->ResetRenderStates(D3D9HELPER_RENDERSTATE_NO_WINDOW_OUTPUT);
		if (FAILED(hr))
		{
			DbgMsg("CRenderEngine::Render: failed in IDisplayObject::Render, error code 0x%08x", hr);
		}
	}
	return hr;
}

HRESULT CRenderEngine::RenderDispObj()
{
	HRESULT hr = S_OK;

	// For BD+ DiscoveryRAM Tags, BD+ Content Code would check the pattern at runtime.
	static unsigned char DATA_TAG_16[] =
	{0xA6, 0x58, 0x29, 0xF7, 0xAD, 0x1E, 0x11, 0x4C, 0x56, 0x8B, 0xEE, 0xA2, 0xB0, 0x35, 0x99, 0xD8};

	if (m_pDispSvrVideoPresenter)
	{
		hr = m_pDispSvrVideoPresenter->BeginRender();
	}

	if (FAILED(hr))
	{
		DATA_TAG_16[4] ^= DATA_TAG_16[12];
		DATA_TAG_16[4] ^= DATA_TAG_16[12];
		DbgMsg("CRenderEngine::Render: failed in BeginScene(), hr = 0x%08x", hr);
		return hr;
	}

	// render the scene, if someone call NodeRequest-DISPLAY_REQUEST_ClearRenderTarget function, we need to clear RT.
    if (m_bIsD3D9RenderTarget && (m_bForceClearRenderTarget || m_dwClearRenderTargetTimes > 0))
    {
        if (m_dwClearRenderTargetTimes > 0)
            m_dwClearRenderTargetTimes--;

        m_pD3D9Helper->Clear();
    }

    if (m_pRootDO)
    {
        m_pD3D9Helper->SetRenderStates(m_eD3D9HelperRenderStateType);
        hr = m_pRootDO->Render(m_pD3D9Helper->GetDevice(), &rectClippingSpace);
        m_pD3D9Helper->ResetRenderStates(m_eD3D9HelperRenderStateType);
	    if (FAILED(hr))
	    {
		    DbgMsg("CRenderEngine::Render: failed in IDisplayObject::Render, error code 0x%08x", hr);
	    }
    }

	if (m_pDispSvrVideoPresenter)
	{
		hr = m_pDispSvrVideoPresenter->EndRender();
		if (FAILED(hr))
		{
			DbgMsg("CRenderEngine::Render: failed in EndScene(), hr = 0x%08x", hr);
			return hr;
		}
	}
	return hr;
}

STDMETHODIMP CRenderEngine::Render()
{
	return E_NOTIMPL;
}

HRESULT CRenderEngine::RenderScene(DWORD dwTimeOut)
{
    DWORD dwRet = WaitForSingleObject(m_hRenderPresentSemaphore, dwTimeOut);
    //ASSERT(WAIT_OBJECT_0 == dwRet);
    return RenderSceneUnordered();
}  

HRESULT CRenderEngine::Present()  
{  
    HRESULT hr = PresentUnordered();  
    ReleaseSemaphore(m_hRenderPresentSemaphore, 1, NULL);  
    return hr;  
}  

HRESULT CRenderEngine::RenderSceneUnordered()  
{
	HRESULT hr = S_OK;

	if (m_bHide)
		return hr;

	if (!m_bEnableRendering || m_eDeviceStatus != eDeviceReady)
		return S_FALSE;

	if (m_pHJDetect)
	{
		BOOL bHiJacked = FALSE;
		hr = m_pHJDetect->IsHiJacked(bHiJacked);
		if (SUCCEEDED(hr) && bHiJacked)
		{
			NotifyRenderEngineEvent(DISPSVR_RENDER_ENGINE_HIJACK_DETECTED, 0, 0);
			DbgMsg("CRenderEngine::RenderScene():Video Hijacked is detected!");
			return E_FAIL;
		}
	}

	{
		CAutoDisplayLock displayLock(m_pLock);

		m_pStateEventSink->Notify(SERVER_STATE_RENDER, 0, 0);

		hr = (this->*m_pfnRenderFunc)();
	}

	if (FAILED(hr))
	{
		if (FAILED(CheckDeviceState()))
		{
			if ((m_dwConfigFlags & DISPSVR_DEVICE_LOST_NOTIFY))
			{
				if (m_eDeviceStatus != eDeviceLost)
					m_eDeviceStatus = eDeviceLostDetected;
            }
			else
            {
				m_eDeviceStatus = eDeviceChanging;
            }
		}
        hr = E_FAIL;		// do not return D3D error to let GPI recover device lost.
	}
	return hr;
}

HRESULT CRenderEngine::PresentUnordered()
{
	HRESULT hr = S_OK;

	if (m_bHide || m_dwConfigFlags & DISPSVR_NO_WINDOW_OUTPUT)
		return hr;

	if (!m_bEnableRendering || m_eDeviceStatus != eDeviceReady)
		return S_FALSE;

	try
	{
		CAutoDisplayLock displayLock(m_pLock);

		m_pStateEventSink->Notify(SERVER_STATE_PRESENT, 0, 0);

		if (m_pDispSvrVideoPresenter)
		{
			hr = CheckDeviceState();
			CHECK_HR(hr, DbgMsg("CRenderEngine::Present: failed in CheckDeviceState, hr = 0x%08x", hr));

			PresentHints Hints;
			ZeroMemory(&Hints, sizeof(PresentHints));
			Hints.dwFrameRate = m_dwFrameRate;
			Hints.dwVideoFlags = m_dwVideoFormat;
			hr = m_pDispSvrVideoPresenter->Present(&Hints);
			CHECK_HR(hr, DbgMsg("CRenderEngine::Present: failed, hr = 0x%08x", hr));
		}
	}
	catch (HRESULT)
	{
		if (E_OUTOFMEMORY == hr && (GetRegistry(REG_VENDOR_ID, 0) == PCI_VENDOR_ID_INTEL))
			m_bOutOfMemory = TRUE;

		if (D3DERR_DEVICELOST == hr || S_PRESENT_MODE_CHANGED == hr || D3DERR_UNSUPPORTEDOVERLAY == hr ||  FAILED(CheckDeviceState()))
		{
			if ((m_dwConfigFlags & DISPSVR_DEVICE_LOST_NOTIFY))
			{
				if (m_eDeviceStatus != eDeviceLost)
					m_eDeviceStatus = eDeviceLostDetected;
            }
		    else
            {
			    m_eDeviceStatus = eDeviceChanging;
            }
        }
        hr = E_FAIL;		// do not return D3D error to let GPI recover device lost.
    }
	return hr;
}

STDMETHODIMP CRenderEngine::ProcessLostDevice()
{
	HRESULT hr = S_OK;
	{
		// BeginDeviceLoss() should be synchronized with filter graph thread or VMR
		// mixer thread so that device lost can be detected.
		
		CAutoDisplayLock displayLock(m_pLock);
	
		if (m_eDeviceStatus == eDeviceReady)
			return S_FALSE;

        if (m_eDeviceStatus != eDeviceChanging)
			return E_UNEXPECTED;

		DbgMsg("CRenderEngine::ProcessLostDevice: calling BeginDeviceLoss thread id=%d", ::GetCurrentThreadId());


		if (m_pD3D9Helper->GetDevice())
		{
			DbgMsg("CRenderEngine::ProcessLostDevice: create new Direct3D device thread id=%d", ::GetCurrentThreadId());

			if (!m_bUseMessageThread)
			{
				while (CheckDeviceState() == D3DERR_DEVICELOST)
					Sleep(50);
			}
			else // Called by message thread, we can't hold this thread, return E_ABORT to indicate that application should call processlostdevice again.
			{
				if (CheckDeviceState() == D3DERR_DEVICELOST)
				{
					if (m_eDeviceStatus != eDeviceLost)
						m_eDeviceStatus = eDeviceLostDetected;
					return E_ABORT;
				}
			}
		}

		hr = CreateD3DDevice();
	}

	if (SUCCEEDED(hr) && m_pD3D9Helper->GetDevice())
	{
		// Note that display server lock is NOT held when calling EndDeviceLoss() because
		// it may introduce some possible deadlock between render thread and filter graph thread.
	
		DbgMsg("CRenderEngine::ProcessLostDevice: calling EndDeviceLoss thread id=%d", ::GetCurrentThreadId());
		
		if (m_pRootDO)
			m_pRootDO->EndDeviceLoss(m_pD3D9Helper->GetDevice());
		if (m_pOwner)
			hr = m_pOwner->EndDeviceLoss(m_pD3D9Helper->GetDevice());

		memset(&m_DeviceInfo, 0, sizeof(m_DeviceInfo));
		m_eDeviceStatus = eDeviceReady;
		
		NotifyRenderEngineEvent(DISPSVR_RENDER_ENGINE_DEVICE_RECOVERED, NULL, NULL);
		hr = S_OK;
	}
    else
    {
        // if failed process re-create device
        // change device status to recoverable
        // thus WaitDeviceChange() can return E_FAIL to
        // the caller of device lost request function
        if(!m_pD3D9Helper->GetDevice())
        {   // Can't create D3D device so we have to ProcessDeviceLost again.
            m_eDeviceStatus = eDeviceLostDetected;
            hr = DISPSVR_DEVICE_E_FAIL;
        }

        DbgMsg("Caution!! CRenderEngine::ProcessLostDevice() failed!!, hr = 0x%08x", hr);
    }

	return hr;
}

STDMETHODIMP CRenderEngine::GetRootObject(REFIID riid, void** ppvObject)
{
	CHECK_POINTER(ppvObject);
	if (!m_pRootDO)
	{
		return VFW_E_WRONG_STATE;
	}

	CAutoDisplayLock displayLock(m_pLock);
	return m_pRootDO->QueryInterface(riid, ppvObject);
}

STDMETHODIMP CRenderEngine::SetRootObject(IDisplayObject* pObject)
{
	CAutoDisplayLock displayLock(m_pLock);
	
	if (m_pRootDO)
	{
		m_pRootDO->Terminate();
		m_pRootDO.Release();
	}

	HRESULT hr = S_OK;

	if (pObject)
	{
		hr = pObject->Initialize(this);
		if (SUCCEEDED(hr))
		{
			m_pRootDO = pObject;
		}
		else
		{
			DbgMsg("CRenderEngine::SetRootObject: failed to initialize DO, 0x%08x", hr);
		}
	}

	return hr;
}

STDMETHODIMP CRenderEngine::SetFrameRate(int nFramesPerSecBy100)
{
	if (FALSE == m_bInitialized)
	{
		DbgMsg("CRenderEngine::SetFrameRate: object is not initialized");
		return VFW_E_WRONG_STATE;
	}
	if (nFramesPerSecBy100 < 100 || nFramesPerSecBy100 > 6000)
	{
		DbgMsg("CRenderEngine::SetFrameRate: desired rate must be between 100 and 6000");
		return E_INVALIDARG;
	}

	CAutoDisplayLock displayLock(m_pLock);

	m_dwInterval = 1000 * 100 / nFramesPerSecBy100;

	return S_OK;
}

STDMETHODIMP CRenderEngine::GetFrameRate(int* pnFramesPerSecBy100)
{
	return E_NOTIMPL;
}

STDMETHODIMP CRenderEngine::GetFrameRateAvg(int* pnFramesPerSecBy100)
{
	return E_NOTIMPL;
}

STDMETHODIMP CRenderEngine::GetMixingPrefs(DWORD* pdwPrefs)
{
	if (FALSE == m_bInitialized)
	{
		DbgMsg("CRenderEngine::GetMixingPrefs: object is not initialized");
		return VFW_E_WRONG_STATE;
	}
	if (!pdwPrefs)
	{
		DbgMsg("CRenderEngine::GetMixingPrefs: received NULL pointer");
		return E_POINTER;
	}
	
	*pdwPrefs = m_dwConfigFlags;
	return S_OK;
}

STDMETHODIMP CRenderEngine::SetDisplayServer(IDisplayServer* pDisplayServer)
{
	if (FALSE == m_bInitialized)
	{
		// If pDisplayServer is null, caller wants to clear the reference in RenderEngine.
		// We should clear the reference and return S_OK no matter RenderEngine is initialized or not.
		// Wizard will call this with null at CWizard::Terminate() after RenderEngine::Terminate().
		// And return VFW_E_WRONG_STATE will cause CWizard::Terminate() fail.
		// So we treat pDisplayServer == NULL as special case here.
		if (pDisplayServer == NULL)
		{
			m_pOwner = NULL;
			return S_OK;
		}

		DbgMsg("CRenderEngine::SetWizardOwner: object is not initialized");
		return VFW_E_WRONG_STATE;
	}

	CAutoDisplayLock displayLock(m_pLock);
	m_pOwner = pDisplayServer;

	return S_OK;
}

STDMETHODIMP CRenderEngine::GetDisplayServer(IDisplayServer** ppDisplayServer)
{
	if (FALSE == m_bInitialized)
	{
		DbgMsg("CRenderEngine::GetWizardOwner: object is not initialized");
		return VFW_E_WRONG_STATE;
	}

	if (!ppDisplayServer)
	{
		DbgMsg("CRenderEngine::GetWizardOwner: received NULL pointer");
		return E_POINTER;
	}

	if (m_pOwner)
	{
		*ppDisplayServer = m_pOwner;
		(*ppDisplayServer)->AddRef();
	}
	else
	{
		*ppDisplayServer = NULL;
	}
	return S_OK;
}

STDMETHODIMP CRenderEngine::SetDisplayWindow(HWND hwnd, RECT *pRect)
{
	HRESULT hr = S_OK;
	// In D3D9 Overlay, video window handle must be the same as the one creates device.
	// If video window handle changes, we should re-create device.
    bool bNeedRecreateDevice = m_pD3D9Helper->IsD3D9Overlay() && (hwnd != m_hwnVideoWindow);

    m_hwnVideoWindow = hwnd;
	m_rectDisp = *pRect;
	m_bHide = IsRectEmpty(&m_rectDisp);
    if (m_pD3D9Helper->IsD3D9Overlay())
		UpdateOverlayStatus(m_bHide);

	m_pD3D9Helper->SetDisplayWindow(hwnd, pRect);  // Update the position of window on FlipEx case.

	if (!m_DeviceInfo.bMonitorChanges && m_hwnVideoWindow && m_hMonitor)
	{
		HMONITOR hMonitor = MonitorFromWindow(m_hwnVideoWindow, MONITOR_DEFAULTTONEAREST);
		if (hMonitor != m_hMonitor)
		{
			m_hMonitor = hMonitor;
			bNeedRecreateDevice = true;
			m_DeviceInfo.bMonitorChanges = TRUE;
		}
		else
			m_DeviceInfo.bMonitorChanges = FALSE;
	}

	if (bNeedRecreateDevice)
	{
		CAutoDisplayLock displayLock(m_pLock);
		if (m_eDeviceStatus == eDeviceReady)
		{
			if ((m_dwConfigFlags & DISPSVR_DEVICE_LOST_NOTIFY))
			{
				if (m_eDeviceStatus != eDeviceLost)
					m_eDeviceStatus = eDeviceLostDetected;
                }
			else
				m_eDeviceStatus = eDeviceChanging;
            }

        // Ignore wait device change to avoid application hanging
        if (!m_bUseMessageThread)
            hr = WaitDeviceChange();
    }

    if (SUCCEEDED(hr))
    {
        POINT pt = {0};
        MONITORINFOEX MonInfo;

	    ZeroMemory(&MonInfo, sizeof(MONITORINFOEX));
	    MonInfo.cbSize = sizeof(MONITORINFOEX);

	    m_rectWindow = *pRect;
	    if (::ClientToScreen(hwnd, &pt) != 0 && GetMonitorInfo(m_hMonitor, &MonInfo) != 0)
	    {
		    ::OffsetRect(&m_rectWindow, pt.x - MonInfo.rcMonitor.left, pt.y - MonInfo.rcMonitor.top);
	    }

        m_rectSrc = m_rectWindow;
        // m_rectSrc may be changed according to render state type.
        m_pD3D9Helper->InitRenderStates(m_eD3D9HelperRenderStateType, m_rectSrc);
        if (m_bHide)
        {
            m_eWindowState = RESOURCE_WINDOW_STATE_MINIMIZED;
        }
        else if (m_rectWindow.left == 0 && m_rectWindow.top == 0
            && m_rectWindow.right == (MonInfo.rcMonitor.right - MonInfo.rcMonitor.left)
            && m_rectWindow.bottom == (MonInfo.rcMonitor.bottom - MonInfo.rcMonitor.top))
        {
            m_eWindowState = RESOURCE_WINDOW_STATE_FULLSCREEN;
        }
        else
        {
            m_eWindowState = RESOURCE_WINDOW_STATE_WINDOWED;
        }

        hr = UpdateDisplayWindow();
    }

	return hr;
}

STDMETHODIMP CRenderEngine::Get3DDevice(IUnknown ** ppDevice)
{
	CHECK_POINTER(ppDevice);

    if (m_eDeviceStatus != eDeviceReady)
        return E_FAIL;

    *ppDevice = m_pD3D9Helper->GetDevice();
    ASSERT(*ppDevice);
    (*ppDevice)->AddRef();
    return S_OK;
}

STDMETHODIMP CRenderEngine::Set3DDevice(IUnknown* pDevice)
{
    CHECK_POINTER(pDevice);
	HRESULT hr = S_OK;

	if (FALSE == m_bInitialized)
	{
		DbgMsg("CRenderEngine::Set3DDevice: object is not initialized");
		return VFW_E_WRONG_STATE;
	}

	CAutoDisplayLock displayLock(m_pLock);

    if (m_pD3D9Helper->GetDevice() == pDevice)
    {
	    DbgMsg("CRenderEngine::Set3DDevice: object is same");
	    return S_OK;
	}

	if (m_pRootDO)
		m_pRootDO->BeginDeviceLoss();
	if (m_pOwner)
		hr = m_pOwner->BeginDeviceLoss();

	ReleaseD3DDevice();

    hr = m_pD3D9Helper->SetDevice(pDevice);
	if (FAILED(hr))
	{
		DbgMsg("CRenderEngine::Set3DDevice: failed to get D3D9 device");
		return hr;
	}

	hr = ConfigureResource();
	if (FAILED(hr))
		return hr;

	if (m_pRootDO)
		m_pRootDO->EndDeviceLoss(m_pD3D9Helper->GetDevice());
	if (m_pOwner)
		hr = m_pOwner->EndDeviceLoss(m_pD3D9Helper->GetDevice());

    UpdateHJDetectDesc();

	return hr;
}

STDMETHODIMP CRenderEngine::GetDisplayWindow(HWND *phwnd, RECT *pRect)
{
	if (FALSE == m_bInitialized)
	{
		DbgMsg("CRenderEngine::GetDisplayWindow: object is not initialized");
		return VFW_E_WRONG_STATE;
	}

	if (phwnd)
	{
		*phwnd = m_hwnVideoWindow;
	}
	if (pRect)
	{
		*pRect = m_rectDisp;
	}
	return S_OK;
}

STDMETHODIMP CRenderEngine::GetBackgroundColor(COLORREF* pColor)
{
    return m_pD3D9Helper->GetBackgroundColor(pColor);
}

STDMETHODIMP CRenderEngine::SetBackgroundColor(COLORREF Color)
{
	CAutoDisplayLock displayLock(m_pLock);
    return m_pD3D9Helper->SetBackgroundColor(Color);
}

STDMETHODIMP CRenderEngine::GetLock(IDisplayLock** ppLock)
{
	return m_pLock.CopyTo(ppLock);
}

STDMETHODIMP CRenderEngine::EnableRendering(BOOL bEnableRendering)
{
	// The checking here is necessary. When the WinDVD play button is pressed,
	// UI will call this function to enable rendering.
	if (m_bEnableRendering != bEnableRendering)
	{
		CAutoDisplayLock displayLock(m_pLock);

		m_bEnableRendering = bEnableRendering;
		if (m_bIsD3D9RenderTarget)
		{
            m_pD3D9Helper->Clear();
		}

        // For nVidia optimized overlay, NVAPI will export 2~6 render targets.
        // To clear render target (RT)
		NodeRequest(DISPLAY_REQUEST_ClearRenderTarget, 0, 10, 0);

		NotifyDisplayEvent(DISPLAY_EVENT_EnableRendering, bEnableRendering, 0);

        if (m_eDeviceStatus == eDeviceReady)
        {
            // Config Video Presenter
            if (m_pDispSvrVideoPresenter)
            {
                m_pDispSvrVideoPresenter->Clear();
            }
        }
    }

	NotifyDWMQueuing(bEnableRendering);

    return S_OK;
}

STDMETHODIMP CRenderEngine::AdviseEventNotify(IDispSvrRenderEngineNotify *pEventNotify)
{
	CAutoDisplayLock displayLock(m_pLock);
	if (!pEventNotify)
		return E_POINTER;

	RenderEngineEventNotify::iterator it = m_RenderEngineEventNotify.begin();
	for (; it != m_RenderEngineEventNotify.end(); ++it)
	{
		if ((*it) == pEventNotify) //It should be a problem for double advise.
		{
			return E_UNEXPECTED;
		}
	}
	m_RenderEngineEventNotify.push_back(pEventNotify);
	return S_OK;
}

STDMETHODIMP CRenderEngine::UnAdviseEventNotify(IDispSvrRenderEngineNotify *pEventNotify)
{
	CAutoDisplayLock displayLock(m_pLock);
	if (!pEventNotify)
		return E_POINTER;

	RenderEngineEventNotify::iterator it = m_RenderEngineEventNotify.begin();
	for (; it != m_RenderEngineEventNotify.end(); ++it)
	{
		if ((*it) == pEventNotify)
		{
			m_RenderEngineEventNotify.erase(it);
			return S_OK;
		}
	}

	return E_INVALIDARG;
}

void CRenderEngine::NotifyRenderEngineEvent(DispSvrEngineEvent dwEvent, DWORD param1, DWORD param2)
{
	HRESULT hr = S_OK;

	RenderEngineEventNotify::iterator it = m_RenderEngineEventNotify.begin();
	for (; it != m_RenderEngineEventNotify.end(); ++it)
		hr = (*it)->OnNotify((DWORD)dwEvent, param1, param2);
}


STDMETHODIMP CRenderEngine::ProcessWindowMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HRESULT hr = E_FAIL;
	switch(uMsg)
	{
	case WM_DISPLAYCHANGE:
		{
			m_bCheckDisplayChange = TRUE;
			hr = S_OK;
		}
		break;
	case WM_PAINT:
		{
			if (m_pD3D9Helper->IsExclusiveMode())
			{
				HRESULT hrx = m_pD3D9Helper->CheckDeviceState();
				if (hrx != S_OK)
				{
				    m_eDeviceStatus = eDeviceLostDetected;
				}
			}

			// Workaround for bug#103829
			// By Intel suggestion, do device lost when presentEX return E_OUTOFMEMORY
			// And use m_bOutOfMemory to limit doing device lost once.
			if(m_bOutOfMemory == TRUE)
			{
				m_eDeviceStatus = eDeviceLostDetected;
				m_bOutOfMemory = FALSE;
			}

			hr = CResourceManager::GetInstance()->ProcessMessage(RESOURCE_MESSAGE_UPDATEWINDOW, NULL);
		}
		break;
	case WM_SIZE:
		{
			BOOL bHide = (SIZE_MINIMIZED == wParam) ? TRUE : FALSE;
			UpdateOverlayStatus(bHide);
		}
		break;
	case WM_MOVE:
	case WM_MOVING:
		{
			hr = CResourceManager::GetInstance()->ProcessMessage(RESOURCE_MESSAGE_MOVEWINDOW, NULL);
		}
		break;
	case WM_POWERBROADCAST:
		{
			if (wParam == PBT_APMRESUMESUSPEND || wParam == PBT_APMRESUMESTANDBY)
			{
				if (m_eDeviceStatus != eDeviceLost)
					m_eDeviceStatus = eDeviceLostDetected;
			}
			hr = S_OK;
		}
		break;
	case WM_SYSCOMMAND:
		hr = S_FALSE;
		if (wParam == SC_MONITORPOWER)
		{
			//lParam of SC_MONITORPOWER -- {-1: the display is powering on, 1: display going to low power, 2: display being shut off.}
			//Only apply device lost when the display is powering on.
			int nStatus = 0xff&lParam;
			if (m_nMonPowerStatus != nStatus && nStatus == 0xff && m_eDeviceStatus != eDeviceLost)
			{
				m_eDeviceStatus = eDeviceLostDetected;
			}
			m_nMonPowerStatus = nStatus;
			hr = S_OK;
		}
		break;
	default:
		hr = S_FALSE;
		break;
	}
	return hr;
}

STDMETHODIMP CRenderEngine::ProcessRequest(DWORD request, DWORD param1, DWORD param2)
{
	if (m_bInitialized == FALSE)
		return VFW_E_WRONG_STATE;

    HRESULT hr = S_OK;
    switch(request) 
    {
    case DISPLAY_REQUEST_FreezeState:
        {
            BOOL bFreezeState = param1 ? TRUE : FALSE;
            BOOL bTryEvictResources = param2 ? TRUE : FALSE;			// the value is 0 in previous windvd.
            if (m_bFreezeState != bFreezeState)
            {
                //Ignore wait device change to avoid application hanging
                if (!m_bUseMessageThread)
                    hr = WaitDeviceChange();
                m_bFreezeState = bFreezeState;

                // We only try to evict and restore resource when freezing state.
                if (bFreezeState && bTryEvictResources)
                {
                    DWORD dwLocalVidMem = 256;//GetRegistry(REG_LOCAL_VIDMEM, 256);
                    DWORD dwVendor = GetRegistry(REG_VENDOR_ID, 0);
                    DWORD dwOSVersion = GetRegistry(REG_OS_VERSION, 0);

                    // reallocate resources to avoid fragmentation.
                    if (dwOSVersion <= OS_XP			// OS is XP or before, vista or later OS has video memory virtualization.
                        && dwLocalVidMem < 192		// less than 192 mb, mainly 128 mb
                        && dwVendor == PCI_VENDOR_ID_ATI)	// ATI has poor video memory management.
                    {
                        CAutoDisplayLock displayLock(m_pLock);
                        BOOL bEvict = TRUE;
                        CResourceManager::GetInstance()->ProcessMessage(RESOURCE_MESSAGE_EVICTRESOURCES, &bEvict);
                        // restore right after eviction.
                        bEvict = FALSE;
                        CResourceManager::GetInstance()->ProcessMessage(RESOURCE_MESSAGE_EVICTRESOURCES, &bEvict);
                    }
                }
            }
        }
        break;
    case DISPLAY_REQUEST_AutoRender:
        m_bAutoRenderThread = param1;
        break;
    case DISPLAY_REQUEST_FrameProperty:
        // for this switch, param1 is polarity, param2 is frame rate
        m_dwVideoFormat = param1;
        m_dwFrameRate = param2;
        // returning S_OK to prevent app from deinterlacing thus return S_FALSE
        hr = S_FALSE;
        break;

	case DISPLAY_REQUEST_ICT:
        hr = E_NOTIMPL;
		break;
	case DISPLAY_REQUEST_CustomizedOutput:
		hr = SetCustomizedOutput(param1 ? TRUE : FALSE, param2);
		break;
	case DISPLAY_REQUEST_ScreenCaptureDefense:
		{
			CAutoDisplayLock displayLock(m_pLock);
			CComPtr<IDispSvrVideoPresenter> pDispSvrVideoPresenter;
			hr = CResourceManager::GetInstance()->GetInterface(DISPSVR_RESOURCE_VIDEOPRESENTER, __uuidof(IDispSvrVideoPresenter), (void**)&pDispSvrVideoPresenter);
			if (SUCCEEDED(hr) && pDispSvrVideoPresenter)
			{
				hr = pDispSvrVideoPresenter->SetScreenCaptureDefense(param1 ? TRUE : FALSE);
			}
			break;
		}
	case DISPLAY_REQUEST_ClearRenderTarget:
        {
            m_bForceClearRenderTarget = param1;
            m_dwClearRenderTargetTimes = param2;
            hr = S_OK;
        }
		break;
	case DISPLAY_REQUEST_ProcessLostDevice:
		{
			hr = S_FALSE;
			if (m_eDeviceStatus == eDeviceRecoverable)
			{
				{
                    CAutoDisplayLock displayLock(m_pLock); // only changing state needs to lock Critical Section.
                    m_hwnVideoWindow = IsWindow((HWND)param1) ? (HWND)param1 : m_hwnVideoWindow;
                    m_eDeviceStatus = eDeviceChanging;
				}
				if (!m_bUseMessageThread)
                    hr = WaitDeviceChange();
                else
                    hr = ProcessLostDevice();
			}
		}
		break;
	case DISPLAY_REQUEST_CheckDisplayModeChange:
		{
			m_bCheckDisplayChange = TRUE;
			hr = S_OK;
		}
		break;
    case DISPLAY_REQUEST_DWMCompositionChanged:
        {
            BOOL bEnable = (BOOL)param1;
            hr = ProcessDWM(bEnable);            
        }
        break;
	case DISPLAY_REQUEST_ExclusiveMode: // switch exclusive mode on the fly. param1 = true/false means turning on/off
		{
			BOOL bExclusiveMode = (BOOL)param1;
			CAutoDisplayLock lock(m_pLock);
			if (m_bExclusiveMode != bExclusiveMode)
			{
				m_bExclusiveMode = bExclusiveMode;
                if (m_bExclusiveMode)
                    m_dwConfigFlags |= DISPSVR_USE_EXCLUSIVE_MODE;
                else
                    m_dwConfigFlags &= ~DISPSVR_USE_EXCLUSIVE_MODE;

				if (m_eDeviceStatus != eDeviceLost)
					m_eDeviceStatus = eDeviceLostDetected;
			}
			hr = S_OK;
		}
		break;
	case DISPLAY_REQUEST_WaitEngineReady: //wait until RenderEngine ready.
		{
			if (m_bInitialized == FALSE)
				return VFW_E_WRONG_STATE;

			while (m_eDeviceStatus != eDeviceReady)
			{
				Sleep(10);
			}
			hr = S_OK;
		}
		break;
	case DISPLAY_REQUEST_TestCooperativeLevel:
		{
			if (m_bInitialized == FALSE)
				return VFW_E_WRONG_STATE;

			CAutoDisplayLock lock(m_pLock);
			hr = CheckDeviceState();
			if (m_eDeviceStatus == eDeviceLost && hr != D3DERR_DEVICELOST)
			{
				m_eDeviceStatus = eDeviceRecoverable;
				NotifyRenderEngineEvent(DISPSVR_RENDER_ENGINE_DEVICE_LOST, NULL, NULL);
			}
		}
		break;
	case DISPLAY_REQUEST_ClearPresenterBuffer:
		{
			if (m_pDispSvrVideoPresenter)
				hr = m_pDispSvrVideoPresenter->Clear();
		}
		break;
    case DISPLAY_REQUEST_VideoFormat:
        {
            hr = S_FALSE;
            if (m_pHDMIHelper)
            {
                DWORD *pFormat = (DWORD *)param1;
                m_pHDMIHelper->UpdateContentFormat(pFormat[0], pFormat[1], pFormat[2]);
                if (m_pHDMIHelper->IsHDMIStereoModeEnabled())
                {
                    DbgMsg("CRenderEngine::ProcessRequest - DISPLAY_REQUEST_VideoFormat");
                    DriverExtHDMIStereoModeCap StereoMode = {0};
                    StereoMode.uWidth = GetRegistry(REG_DISPLAY_WIDTH, 0);//pp.BackBufferWidth;
                    StereoMode.uHeight = GetRegistry(REG_DISPLAY_HEIGHT, 0);// pp.BackBufferHeight;
                    StereoMode.uRefreshRate = GetRegistry(REG_DISPLAY_REFRESH_RATE, 0);
                    hr = m_pHDMIHelper->GetAppropriateHDMIDisplayMode(&StereoMode);
                    if (SUCCEEDED(hr))
                    {
                        BOOL bReCreateDevice = FALSE;
						m_DeviceInfo.bStereoModeChanges = TRUE;
                        hr = m_pHDMIHelper->EnableHDMIStereoMode(TRUE, &StereoMode, &bReCreateDevice);
                        if (hr == S_OK)
                        {
                            // Set HDMI Stereo Mode will trigger device lost we don't need do it again
                            if (bReCreateDevice)
                            {
                                if (m_eDeviceStatus != eDeviceLost)
                                    m_eDeviceStatus = eDeviceLostDetected;
                            }
                        }
						else
							m_DeviceInfo.bStereoModeChanges = FALSE;
                    }
                }
            }
        }
        break;
	case DISPLAY_REQUEST_ResetDevice:
		{
			CAutoDisplayLock displayLock(m_pLock);
            m_eDeviceStatus = eDeviceChanging;
            hr = ResetDevice();
		}
		break;
	default:
		return E_INVALIDARG;
	}
	return hr;
}
//////////////////////////////////////////////////////////////////////////
// IDisplayEventHost
STDMETHODIMP CRenderEngine::Register(IDisplayEventHandler* pHandler, LPVOID pInstance)
{
    CHECK_POINTER(pHandler);
    CAutoDisplayLock displayLock(m_pLock);
	EventHandler eh;
	eh.pHandler = pHandler;
	eh.pInstance = pInstance;
	m_EventHandlers.insert(m_EventHandlers.end(), eh);
    return S_OK;
}

STDMETHODIMP CRenderEngine::Unregister(IDisplayEventHandler* pHandler)
{
    CHECK_POINTER(pHandler);
    CAutoDisplayLock displayLock(m_pLock);

	EventHandlers::iterator it = m_EventHandlers.begin();
    for (; it != m_EventHandlers.end(); ++it)
    {
        if (it->pHandler == pHandler)
        {
            m_EventHandlers.erase(it);
            return S_OK;
        }
    }

    return E_INVALIDARG;
}

void CRenderEngine::NotifyDisplayEvent(DisplayEvent event, DWORD param1, DWORD param2)
{
	HRESULT hr = S_OK;

	if (m_pRootDO)
		hr = m_pRootDO->NotifyEvent(event, param1, param2, this);

	EventHandlers::iterator it = m_EventHandlers.begin();
	for (; it != m_EventHandlers.end(); ++it)
		hr = it->pHandler->NotifyEvent((DWORD)event, param1, param2, it->pInstance);
}

//////////////////////////////////////////////////////////////////////////
// Private routines
HRESULT CRenderEngine::CreateD3DDevice()
{
    HRESULT hr = ReleaseD3DDevice();

    // For BD+ DiscoveryRAM Tags, BD+ Content Code would check the pattern at runtime.
	static unsigned char DATA_TAG_15[] =
	{0x95, 0xF4, 0x69, 0xBA, 0x4A, 0x5C, 0x68, 0xBC, 0x08, 0x70, 0x82, 0x56, 0x80, 0x62, 0xA3, 0xC7};

    CD3D9Helper::ConfigParam config = {0};
    GenerateConfigParamFromEngine(config);

    hr = m_pD3D9Helper->CreateDevice(m_hwnVideoWindow, config);
    if (SUCCEEDED(hr))
        hr = InitResources();

	if (FAILED(hr))
		DATA_TAG_15[13] -= DATA_TAG_15[0];

    if(CDynLibManager::GetInstance()->pfnDwmIsCompositionEnabled)
        CDynLibManager::GetInstance()->pfnDwmIsCompositionEnabled(&m_bIsDWMEnabled);

    m_bFirstTimeDisp = true;

    return hr;
}

HRESULT CRenderEngine::ReleaseD3DDevice()
{
    ClearRegistry(REG_DISPLAY_XVYCC_GAMUT);

	if (CResourceManager::GetInstance())
		CResourceManager::GetInstance()->ProcessMessage(RESOURCE_MESSAGE_RELEASEDEVICE, 0);

    return m_pD3D9Helper->ReleaseDevice();
}

HRESULT CRenderEngine::ConfigureResource(bool bResetCase)
{
    GUID guidMixer, guidPresenter;
    HRESULT hr = E_FAIL;
    DWORD dwOSVersion = GetRegistry(REG_OS_VERSION, 0);

	// For BD+ DiscoveryRAM Tags, BD+ Content Code would check the pattern at runtime.
	static unsigned char DATA_TAG_10[] =
	{0x17, 0x82, 0x76, 0x4B, 0x60, 0x38, 0x96, 0xDE, 0xF0, 0x18, 0x60, 0x3D, 0x58, 0x62, 0x25, 0x8C};

    if (m_bStereoMode && dwOSVersion >= OS_WIN7)
    {
        DWORD dwVendor = GetRegistry(REG_VENDOR_ID, 0);
        if(PCI_VENDOR_ID_ATI == dwVendor)
        {
            CComPtr<IDispSvrDriverExtension> pDrvExt;
            DrvExtAdapterInfo AdapterInfo = {0};

            if (FAILED(CResourceManager::GetInstance()->GetInterface(DISPSVR_RESOURCE_DRIVEREXTENSION, __uuidof(IDispSvrDriverExtension), (void **)&pDrvExt))
                || FAILED(pDrvExt->QueryAdapterInfo(m_hwnVideoWindow, m_hMonitor, &AdapterInfo))
                || AdapterInfo.dwSupportStereoFormats == 0)  // We should judge AdapterInfo.dwSupportStereoFormats & m_dwRestoreMixerStereoMode.
            {
                guidMixer = DISPSVR_RESOURCE_STEREOVIDEOMIXER;
            }
            else
            {
                guidMixer = GUID_NULL; 
            }
        }
        else
        {
            guidMixer = DISPSVR_RESOURCE_STEREOVIDEOMIXER;
        }
    }
    else
    {
        guidMixer = GUID_NULL;
    }

    if (m_pD3D9Helper->IsD3D9Overlay())
    {
        guidPresenter = DISPSVR_RESOURCE_D3DOVERALYPRESENTER;
    }
    else if (m_dwConfigFlags&DISPSVR_USE_CUSTOMIZED_OUTPUT)
    {
        guidPresenter = GUID_NULL;
    }
    else
    {
        guidPresenter = DISPSVR_RESOURCE_D3DVIDEOPRESENTER;
    }

    hr = SetPreferredMixerPresenter(guidMixer, guidPresenter, bResetCase);
    if(FAILED(hr))
    {
        // For BD+ DiscoveryRAM Tags, BD+ Content Code would check the pattern at runtime.
		DATA_TAG_10[2] ^= DATA_TAG_10[14];
		DATA_TAG_10[2] ^= DATA_TAG_10[14];

        // Don't notify the fail message to UI on reset case, that will cause warning page pop twice.
        if (m_pD3D9Helper->GetDevice() && !bResetCase)
        {   //Notify UI has to pop message for User.
            hr = DISPSVR_DEVICE_MIXER_PRESENTER_FAIL;
        }
    }
    return hr;
}

HRESULT CRenderEngine::SetPreferredMixerPresenter(GUID &guidMixer, GUID& guidPresenter, bool bResetCase)
{
    HRESULT hr;

    hr = CResourceManager::GetInstance()->SetPreferredResource(guidMixer, DISPSVR_RESOURCE_VIDEOMIXER);
    if (FAILED(hr))
    {
        DbgMsg("CRenderEngine::SetPreferredMixerPresenter: fail to set preferred resource.");
        return hr;
    }

	hr = CResourceManager::GetInstance()->SetPreferredResource(guidPresenter, DISPSVR_RESOURCE_VIDEOPRESENTER);
	if (FAILED(hr))
	{
        DbgMsg("CRenderEngine::SetPreferredMixerPresenter: fail to set preferred resource.");
		return hr;
	}

    if (bResetCase)
    {
        hr = CResourceManager::GetInstance()->ProcessMessage(RESOURCE_MESSAGE_RESETDEVICE, m_pD3D9Helper->GetDevice());
    }
    else
    {
    	hr = CResourceManager::GetInstance()->ProcessMessage(RESOURCE_MESSAGE_SETDEVICE, m_pD3D9Helper->GetDevice());
	    if (FAILED(hr))
	    {
            DbgMsg("CRenderEngine::SetPreferredMixerPresenter: unable to set D3D device to resource manager.");
		    return hr;
	    }
    }

	// Config Video Mixer
	CComPtr<IDispSvrVideoMixer> pDispSvrVideoMixer;
	hr = CResourceManager::GetInstance()->GetInterface(DISPSVR_RESOURCE_VIDEOMIXER, __uuidof(IDispSvrVideoMixer), (void **)&pDispSvrVideoMixer);
	if (FAILED(hr))
	{
        DbgMsg("CRenderEngine::SetPreferredMixerPresenter: unable to get any video mixer.");
		return hr;
	}
	
	// Config Video Presenter
	m_pDispSvrVideoPresenter.Release();
	hr = CResourceManager::GetInstance()->GetInterface(DISPSVR_RESOURCE_VIDEOPRESENTER, __uuidof(IDispSvrVideoPresenter), (void**)&m_pDispSvrVideoPresenter);
	if (FAILED(hr))
	{
        DbgMsg("CRenderEngine::SetPreferredMixerPresenter: unable to get any video presenter.");
		return hr;
	}

    // Init Video Presenter Caps
    PresenterCaps presenterCaps = {0};
    presenterCaps.dwSize = sizeof(PresenterCaps);
    presenterCaps.VideoDecodeCaps = 0;
    hr = m_pDispSvrVideoPresenter->QueryCaps(&presenterCaps);
    if (FAILED(hr))
    {
        DbgMsg("CRenderEngine::SetPreferredMixerPresenter: unable to query video presenter caps.");
        return hr;
    }

	MixerCaps mixerCaps = {0};
	hr = pDispSvrVideoMixer->QueryCaps(&mixerCaps);

	MixerProperty prop;
    m_eD3D9HelperRenderStateType = D3D9HELPER_RENDERSTATE_NO_CHANGE;
	hr = pDispSvrVideoMixer->GetProperty(&prop);
	if (SUCCEEDED(hr))
	{
        prop.dwFlags &= ~MIXER_PROPERTY_VIRTUALIZATION;
        prop.dwFlags &= ~MIXER_PROPERTY_VIRTUALIZE_FROM_ORIGIN;

		if (m_dwConfigFlags & DISPSVR_USE_RT_VIRTUALIZATION)
		{
            // in flip operation, such as FlipEx or exclusive mode, we do not need virtualization.
            if (!m_pD3D9Helper->IsFlipEx() && !m_pD3D9Helper->IsExclusiveMode())
			{
				prop.dwFlags |= MIXER_PROPERTY_VIRTUALIZATION;
                m_eD3D9HelperRenderStateType = D3D9HELPER_RENDERSTATE_RTV;
				if (presenterCaps.bCanVirtualizeFromOrigin)
				{
					// Workaround for bug#91143, menlow cannot present on right window position when other window overlay AP
					if ( !((m_dwConfigFlags & DISPSVR_FORCE_DISABLE_RTV_FROM_ORIGIN) && (guidPresenter == DISPSVR_RESOURCE_D3DVIDEOPRESENTER)) )
					{
						if (mixerCaps.dwFlags & MIXER_CAP_CAN_VIRTUALIZE_FROM_ORIGIN)
						{
							prop.dwFlags |= MIXER_PROPERTY_VIRTUALIZE_FROM_ORIGIN;
							m_eD3D9HelperRenderStateType = D3D9HELPER_RENDERSTATE_RTV_FROM_ORIGIN;
						}
					}
				}
			}
		}

		prop.dwFlags &= ~MIXER_PROPERTY_RENDER_TARGET;
		prop.dwFlags &= ~MIXER_PROPERTY_SWAP_CHAIN;

		// Bug#111343, using render target for NVAPIPresenter
		if (presenterCaps.dwPresenterInfo == PRESENTER_PROPRIETARYOVERLAY && PCI_VENDOR_ID_NVIDIA == GetRegistry(REG_VENDOR_ID, 0))
			prop.dwFlags |= MIXER_PROPERTY_RENDER_TARGET;
		else
			prop.dwFlags |= MIXER_PROPERTY_SWAP_CHAIN;

        prop.dwFlags |= MIXER_PROPERTY_CLEARNONVIDEOAREA;

		hr = pDispSvrVideoMixer->SetProperty(&prop);
	}

	PresenterProperty PresenterProp = {0};
	PresenterProp.dwSize = sizeof(PresenterProperty);
	hr = m_pDispSvrVideoPresenter->GetProperty(&PresenterProp);
	if (SUCCEEDED(hr))
	{
		if (!bResetCase)
		{
			if (m_dwConfigFlags & DISPSVR_WAITING_FOR_VSYNC)
				PresenterProp.dwFlags |= PRESENTER_PROPERTY_WAITUNTILPRESENTABLE;
			else
				PresenterProp.dwFlags &= ~PRESENTER_PROPERTY_WAITUNTILPRESENTABLE;
		}

		if (m_bExclusiveMode)
			PresenterProp.dwFlags |= PRESENTER_PROPERTY_EXCLUSIVE_MODE;
		else
			PresenterProp.dwFlags &= ~PRESENTER_PROPERTY_EXCLUSIVE_MODE;

		hr = m_pDispSvrVideoPresenter->SetProperty(&PresenterProp);
	}

	if (m_dwConfigFlags & DISPSVR_USE_CUSTOMIZED_OUTPUT)
	{
		hr = m_pDispSvrVideoPresenter->SetScreenCaptureDefense(TRUE);
		if (FAILED(hr))
		{
            DbgMsg("CRenderEngine::SetPreferredMixerPresenter: presenter doesn't support private overlay output");
			return hr;
		}
	}

	m_bIsOverlayPresenter = presenterCaps.bIsOverlay ? true : false;
	DbgMsg("CRenderEngine::SetPreferredMixerPresenter: use %s mode.", m_bIsOverlayPresenter ? "overlay" : "d3d");

	m_bIsD3D9RenderTarget = (mixerCaps.dwFlags & MIXER_CAP_3D_RENDERTARGET) != 0;

	return hr;
}

//////////////////////////////////////////////////////////////////////////
// Rendering Thread

// spins off rendering thread
STDMETHODIMP CRenderEngine::StartRenderingThread()
{
	UINT tid = NULL;

	CAutoDisplayLock lock(m_pLock);
	if (!m_bInitialized)
	{
		DbgMsg("CRenderEngine::StartRenderingThread: function called when wizard is not initialized");
		return VFW_E_WRONG_STATE;
	}
	if (m_RenderThreadStatus != eNotStarted)
	{
		DbgMsg("CRenderEngine::StartRenderingThread: render thread is already running / closed");
		return S_FALSE;
	}

	// since we initialized successfully, spin off rendering thread
	m_hRenderThread = (HANDLE) _beginthreadex(NULL, NULL, RenderThreadProc, this, NULL, &tid);
	if (INVALID_HANDLE_VALUE == m_hRenderThread)
	{
		DbgMsg("CRenderEngine::Initialize: failed to create rendering thread");
		return E_UNEXPECTED;
	}

	m_RenderThreadStatus = eRunning;
	return S_OK;
}

// fires the end of the rendering thread and waits until render thread closes
STDMETHODIMP CRenderEngine::StopRenderingThread()
{
	if (m_RenderThreadStatus != eRunning)
	{
		return S_FALSE;
	}

	m_RenderThreadStatus = eWaitingToStop;
	while (m_RenderThreadStatus != eFinished)
	{
		Sleep(50);
	}
	WaitForSingleObject( m_hRenderThread, INFINITE );
	CloseHandle(m_hRenderThread);
	m_RenderThreadStatus = eNotStarted;
	
	return S_OK;
}

STDMETHODIMP CRenderEngine::NodeRequest(DWORD request, DWORD param1, DWORD param2, IDisplayObject* pObject)
{
    if (m_bInitialized == FALSE)
        return VFW_E_WRONG_STATE;

    HRESULT hr = S_OK;
    switch(request) // only handle requests from DisplayObject, bypass other messages to ProcessRequest for backward compatible.
    {
    case DISPLAY_REQUEST_Render:
        {
        DISP_AUTO_PROFILE(dog::CAT_VIDEO, dogsub::CAT_VIDEO_DISPSVR_1ST_FRAME_PREPARE_BEGIN);
        // only main video is granted to render and present.
        // If no rendering thread, MainVideotime will not be expired.
        // So we'll accept request at anytime.
            DWORD dwTimeOut = CRenderClock::GetInstance()->Expired() ? TIMEOUT_RENDERPRESENT_SEMAPHORE : 0;
            if (m_RenderThreadStatus != eRunning || SUCCEEDED(UpdateMainVideoTime(pObject)))
                hr = RenderScene(dwTimeOut);
        DISP_AUTO_PROFILE(dog::CAT_VIDEO, dogsub::CAT_VIDEO_DISPSVR_1ST_FRAME_PREPARE_END);
        break;
        }        
    case DISPLAY_REQUEST_Present:
        DISP_AUTO_PROFILE(dog::CAT_VIDEO, dogsub::CAT_VIDEO_DISPSVR_1ST_FRAME_DISPLAY_BEGIN);
        // If no rendering thread, MainVideotime will not be expired.
        // So we'll accept request at anytime.
        if (m_RenderThreadStatus != eRunning || 
            SUCCEEDED(UpdateMainVideoTime(pObject)))
            hr = Present();
        DISP_AUTO_PROFILE(dog::CAT_VIDEO, dogsub::CAT_VIDEO_DISPSVR_1ST_FRAME_DISPLAY_END);
        m_bFirstTimeDisp = false;
        break;
    default:
        return ProcessRequest( request, param1, param2);
    }
    return hr;
}

STDMETHODIMP CRenderEngine::SetColorKey(DWORD dwColorKey)
{
    HRESULT hr = E_FAIL;

    if (m_pDispSvrVideoPresenter)
    {
        hr = m_pDispSvrVideoPresenter->SetColorKey(dwColorKey);
    }
    m_dwColorKey = dwColorKey;
    return hr;
}

STDMETHODIMP CRenderEngine::GetColorKey(DWORD* pdwColorKey)
{
	HRESULT hr = E_FAIL;

	if (m_pDispSvrVideoPresenter)
	{
		hr = m_pDispSvrVideoPresenter->GetColorKey(pdwColorKey);
	}
	return hr;	
}

STDMETHODIMP CRenderEngine::GetMessageWindow(HWND *phwnd)
{
	return E_NOTIMPL;
}

STDMETHODIMP CRenderEngine::SetMessageWindow(HWND hwnd)
{
	return E_NOTIMPL;
}

STDMETHODIMP CRenderEngine::GetConfigFlags(DWORD *pConfigs)
{
	if (!pConfigs)
		return E_POINTER;

	if (FALSE == m_bInitialized)
	{
		DbgMsg("CRenderEngine::GetConfigFlags: object is not initialized");
		return VFW_E_WRONG_STATE;
	}

	CAutoDisplayLock displayLock(m_pLock);

	*pConfigs = m_dwConfigFlags;
	return S_OK;
}

STDMETHODIMP CRenderEngine::SetProperty( REFGUID guidPropSet, DWORD dwPropID,
                                        LPVOID pInstanceData, DWORD cbInstanceData,
                                        LPVOID pPropData, DWORD cbPropData)
{
    return E_NOTIMPL;
}

STDMETHODIMP CRenderEngine::GetProperty( REFGUID guidPropSet, DWORD dwPropID,
                                        LPVOID pInstanceData, DWORD cbInstanceData,
                                        LPVOID pPropData, DWORD cbPropData, DWORD *pcbReturned)
{
    CHECK_POINTER(pPropData);
    CHECK_POINTER(pcbReturned);

    if (IsEqualGUID( guidPropSet, DISPSVR_ENGINE_MIXERPROPSET))
    {
        switch (dwPropID)
        {
        case ENGINE_PROPID_RESOURCE_ID:
            {
                if (cbPropData != sizeof(GUID)) // request GUID size
                    return E_INVALIDARG;

                if (SUCCEEDED(CResourceManager::GetInstance()->GetActiveResrouceGUID(DISPSVR_RESOURCE_VIDEOMIXER, (GUID *)pPropData)))
                {
                    (*pcbReturned) = sizeof(GUID);
                    return S_OK;
                }
                return E_UNEXPECTED;
            }
        default :
            return E_INVALIDARG;
        }
    }

    else if (IsEqualGUID( guidPropSet, DISPSVR_ENGINE_PRESENTERPROPSET))
    {
        switch (dwPropID)
        {
        case ENGINE_PROPID_RESOURCE_ID:
            {
                if (cbPropData != sizeof(GUID)) // request GUID size
                    return E_INVALIDARG;

                // Need to fix GUID query issue.
                if (SUCCEEDED(CResourceManager::GetInstance()->GetActiveResrouceGUID(DISPSVR_RESOURCE_VIDEOMIXER, (GUID *)pPropData)))
                {
                    (*pcbReturned) = sizeof(GUID);
                    return S_OK;
                }

                return E_UNEXPECTED;
            }
        default :
            return E_INVALIDARG;
        }
    }

    else if (IsEqualGUID( guidPropSet, DISPSVR_ENGINE_DRIVERPROPSET))
	{
        switch (dwPropID)
        {
        case DRIVER_PROPID_COPROC_CAP:
            return E_NOTIMPL;

        case DRIVER_PROPID_PAVP_CAP:
            {
                if (cbPropData != sizeof(DriverPavpCap))
                    return E_INVALIDARG;

                CComPtr<IDispSvrDriverExtension> pDrvExt;
                DriverExtContentProtectionCaps cpcaps;

                ZeroMemory(&cpcaps, sizeof(cpcaps));
                if (FAILED(CResourceManager::GetInstance()->GetInterface(DISPSVR_RESOURCE_DRIVEREXTENSION, __uuidof(IDispSvrDriverExtension), (void **)&pDrvExt))
                    || FAILED(pDrvExt->QueryContentProtectionCaps(&cpcaps)))
                    return E_FAIL;

                DriverPavpCap *pCap = (DriverPavpCap *) pPropData;
                ZeroMemory(pCap, sizeof(DriverPavpCap));

                if (DRIVER_EXT_CP_PAVP == cpcaps.eType)
                {
                    pCap->bSupport = TRUE;
                    pCap->bSupportAudio = cpcaps.sPavpCaps.bSupportAudio;
                }
                if (pcbReturned)
                    *pcbReturned = sizeof(DriverPavpCap);
            }
            return S_OK;
        case DRIVER_PROPID_STEREO_VIDEO_CAP:
			{
                if (cbPropData != sizeof(DriverStereoVideoCap)) // request same size with DriverStereoVideoCap structure
                    return E_INVALIDARG;

				DWORD dwVendor = GetRegistry(REG_VENDOR_ID, 0);
				DWORD dwDevice = GetRegistry(REG_DEVICE_ID, 0);
                DWORD dwCoProcVendor = GetRegistry(REG_COPROC_ACTIVE_VENDOR_ID, 0);
                if ((dwVendor == PCI_VENDOR_ID_INTEL) && (dwCoProcVendor == PCI_VENDOR_ID_NVIDIA))
                {
                    dwVendor = dwCoProcVendor;
                }
				DriverStereoVideoCap *pDriverStereoVideoCap = (DriverStereoVideoCap *)pPropData;
				ZeroMemory(pDriverStereoVideoCap, sizeof(DriverStereoVideoCap));
				// Anaglyph / Side-by-Side / Checkerboard support window and exclusive mode
                DWORD DefaultCaps = (STEREO_VISION_ANAGLYPH
                                                    | STEREO_VISION_SIDE_BY_SIDE
                                                    | STEREO_VISION_CHECHERBOARD
                                                    | STEREO_VISION_OPTIMIZED_ANAGLYPH
                                                    | STEREO_VISION_HALFCOLOR_ANAGLYPH
                                                    | STEREO_VISION_ROW_INTERLEAVED
                                                    | STEREO_VISION_HALFCOLOR2_ANAGLYPH
                                                    | STEREO_VISION_COLUMN_INTERLEAVED);
                pDriverStereoVideoCap->dwWindowModeCap = DefaultCaps;
                pDriverStereoVideoCap->dwExclusiveModeCap = DefaultCaps;

				switch(dwVendor)
				{
				case PCI_VENDOR_ID_NVIDIA:
					{
						pDriverStereoVideoCap->dwContentProtectionWindowModeCap =  pDriverStereoVideoCap->dwWindowModeCap;

						CComPtr<IDispSvrDriverExtension> pDrvExt;
						DrvExtAdapterInfo AdapterInfo = {0};

						// Query 3D Vision Support
						if ( SUCCEEDED( CResourceManager::GetInstance()->GetInterface(DISPSVR_RESOURCE_DRIVEREXTENSION, __uuidof(IDispSvrDriverExtension), (void **)&pDrvExt) ) )
							if ( pDrvExt && SUCCEEDED( pDrvExt->QueryAdapterInfo(m_hwnVideoWindow, m_hMonitor, &AdapterInfo) ) )
								pDriverStereoVideoCap->dwExclusiveModeCap |= AdapterInfo.dwSupportStereoFormats;

						// Query HDMI Stereo Support
                        if(AdapterInfo.dwSupportStereoFormats & (STEREO_VISION_NV_PRIVATE | STEREO_VISION_NV_STEREO_API))
                        {
                            if (m_pHDMIHelper->IsHDMIStereoModeSupported(NULL))
                                pDriverStereoVideoCap->dwExclusiveModeCap |= STEREO_VISION_HDMI_STEREO;
                        }
						break;
					}                    
				case PCI_VENDOR_ID_INTEL:
					{
						pDriverStereoVideoCap->dwContentProtectionWindowModeCap =  pDriverStereoVideoCap->dwWindowModeCap;
						if (m_pHDMIHelper->IsHDMIStereoModeSupported(NULL))
							pDriverStereoVideoCap->dwContentProtectionWindowModeCap |= STEREO_VISION_HDMI_STEREO;
						break;
					}                    
				case PCI_VENDOR_ID_ATI:
					{
						CComPtr<IDispSvrDriverExtension> pDrvExt;
						DrvExtAdapterInfo AdapterInfo = {0};

						if (FAILED(CResourceManager::GetInstance()->GetInterface(DISPSVR_RESOURCE_DRIVEREXTENSION, __uuidof(IDispSvrDriverExtension), (void **)&pDrvExt))
							|| FAILED(pDrvExt->QueryAdapterInfo(m_hwnVideoWindow, m_hMonitor, &AdapterInfo)))
							return E_FAIL;
						pDriverStereoVideoCap->dwContentProtectionWindowModeCap |= AdapterInfo.dwSupportStereoFormats;
						break;
					}
				default:
					break;
				}

                if (CComQIPtr<IDirect3DDevice9> pDevice = m_pD3D9Helper->GetDevice())
                {
                    HRESULT t_hr = E_FAIL;
                    CComPtr<IDirectXVideoDecoderService> pVideoDecoderService;
					CDynLibManager *pCDynLibManager = CDynLibManager::GetInstance();
					if (pCDynLibManager && pCDynLibManager->pfnDXVA2CreateVideoService != NULL)
					{
						t_hr = pCDynLibManager->pfnDXVA2CreateVideoService(pDevice, __uuidof(IDirectXVideoDecoderService), (void**) &pVideoDecoderService);
						if (SUCCEEDED(t_hr))
						{
							UINT uCount = 0;
							GUID *pGuids = NULL;
							t_hr = pVideoDecoderService->GetDecoderDeviceGuids( &uCount, &pGuids);
							if (SUCCEEDED(t_hr))
							{
								for (UINT i = 0; i < uCount; i++)
								{
									if ((IsEqualGUID(DXVA_ModeH264_MVC, pGuids[i])) || (IsEqualGUID(DXVA_ModeH264_AMD_MVC, pGuids[i])))
									{
										pDriverStereoVideoCap->bMVCHardwareDecode = TRUE;
									}
									//Ignore ATi/nVidia because we cannot use VLD codec for MVC decode on ATi/nVidia platform.
									else if ((dwVendor == PCI_VENDOR_ID_INTEL)
										&& ((IsEqualGUID(DXVA2_ModeH264_E , pGuids[i])
										|| IsEqualGUID(DXVA2_ModeH264_F , pGuids[i])
										|| IsEqualGUID(DXVADDI_Intel_ModeH264_E, pGuids[i])
										|| IsEqualGUID(DXVADDI_Intel_ModeH264_F, pGuids[i]))))
									{
										pDriverStereoVideoCap->bAVCVLDHardwareDecode = TRUE;
									}
								}
								CoTaskMemFree(pGuids);
							}
						}
					}
					else
					{
						return E_INVALIDARG;
					}
                }
                (*pcbReturned) = sizeof(DriverStereoVideoCap);
				DbgMsg("The DXVA of MVC is %d", pDriverStereoVideoCap->bMVCHardwareDecode);
			}
			return S_OK;
		default:
			return E_INVALIDARG;
		}
	}

    return E_INVALIDARG;
}

STDMETHODIMP CRenderEngine::QueryPropertySupported(REFGUID guidPropSet, DWORD dwPropID, DWORD *pTypeSupport)
{
    CHECK_POINTER(pTypeSupport);

    (*pTypeSupport) = FALSE;

    if (IsEqualGUID( guidPropSet, DISPSVR_ENGINE_MIXERPROPSET))
    {
        switch (dwPropID)
        {
            case ENGINE_PROPID_RESOURCE_ID:
                {
                    (*pTypeSupport) = TRUE;
                }
                return S_OK;
            default :
                return E_NOTIMPL;
        }
    }
    else if (IsEqualGUID( guidPropSet, DISPSVR_ENGINE_PRESENTERPROPSET))
    {
        switch (dwPropID)
        {
        case ENGINE_PROPID_RESOURCE_ID:
            {
                (*pTypeSupport) = TRUE;
            }
            return S_OK;
        default :
            return E_NOTIMPL;
        }
    }
    else if (IsEqualGUID( guidPropSet, DISPSVR_ENGINE_DRIVERPROPSET))
    {
        switch (dwPropID)
        {
        case DRIVER_PROPID_PAVP_CAP:
		case DRIVER_PROPID_COPROC_CAP:
		case DRIVER_PROPID_STEREO_VIDEO_CAP:
            {
                (*pTypeSupport) = TRUE;
            }
            return S_OK;
        default :
            return E_NOTIMPL;
        }
    }

    return E_NOTIMPL;
}

HRESULT CRenderEngine::GetRecomFPS(DWORD *pdwRecomFPS)
{
	HRESULT hr = E_FAIL;

	if (m_pDispSvrVideoPresenter)
	{
		PresenterCaps Caps = {0};
		Caps.dwSize = sizeof(PresenterCaps);
		hr = m_pDispSvrVideoPresenter->QueryCaps(&Caps);
		if (SUCCEEDED(hr))
		{
			*pdwRecomFPS = Caps.dwFPS;
		}
	}
	return hr;	
}

HRESULT CRenderEngine::UpdateMainVideoTime(void *pObject)
{
	if (CRenderClock::GetInstance()->Update(pObject))
		return S_OK;
	return E_FAIL;
}

HRESULT CRenderEngine::RenderLoop()
{
	HRESULT hr = S_OK;
	DWORD dwStart, dwRender;

	{
		// use display lock as an barrier to ensure m_RenderThreadStatus == eRunning
		CAutoDisplayLock lock(m_pLock);
		if (m_RenderThreadStatus != eRunning)
		{
			m_RenderThreadStatus = eFinished;
			return S_FALSE;
		}
		ASSERT(eRunning == m_RenderThreadStatus);
		ASSERT(TRUE == m_bInitialized);
	}

	while (m_RenderThreadStatus == eRunning)
	{
		dwStart = timeGetTime();

		if (!m_bAutoRenderThread)
		{
			Sleep(m_dwInterval);
			continue;
		}

		// freeze state to prevent the device from losing when open video renderer in gpidisplay.
		if (m_bFreezeState == FALSE)
		{
			if (m_eDeviceStatus == eDeviceLostDetected)
			{
				DbgMsg("Device Lost Detected, Time: %d.", timeGetTime());

        HMONITOR hMonitor = MonitorFromWindow(m_hwnVideoWindow, (m_hwnVideoWindow) ? MONITOR_DEFAULTTONEAREST : MONITOR_DEFAULTTOPRIMARY);
				if (m_DeviceInfo.bMonitorChanges || !m_pD3D9Helper->IsD3D9Ex() || hMonitor != m_hMonitor)
					m_eDeviceStatus = eDeviceLost;
				else 
					m_eDeviceStatus = eDeviceNotReset;

				ConfigureHDMIStereoBeforeDeviceReset();
			}
			else if (m_eDeviceStatus == eDeviceNotReset)
			{
				DbgMsg("Device Not Reset, Time: %d.", timeGetTime());

				m_eDeviceStatus = eDeviceRecoverable;

				NotifyRenderEngineEvent(DISPSVR_RENDER_ENGINE_DEVICE_NEED_RESET, 0, 0);
			}
			else if (m_eDeviceStatus == eDeviceLost)
			{
				// tell all the layers that we are lost. so that they would release all the
				// allocated video resources
				if (m_pRootDO)
					m_pRootDO->BeginDeviceLoss();
				if (m_pOwner)
					hr = m_pOwner->BeginDeviceLoss();

				// test if device is recoverable and then post message.
                if (CheckDeviceState() != D3DERR_DEVICELOST)
				{
					m_eDeviceStatus = eDeviceRecoverable;
					NotifyRenderEngineEvent(DISPSVR_RENDER_ENGINE_DEVICE_LOST, 0, 0);
				}
			}
			else if (m_eDeviceStatus == eDeviceChanging)
			{
                MixerProperty MixerProp;
                CComPtr<IDispSvrVideoMixer> pDispSvrVideoMixer;
                if(SUCCEEDED(CResourceManager::GetInstance()->GetInterface(DISPSVR_RESOURCE_VIDEOMIXER, __uuidof(IDispSvrVideoMixer), (void **)&pDispSvrVideoMixer)))
                {
	                pDispSvrVideoMixer->GetProperty(&MixerProp);
	                m_bRestoreMixerStereoEnable = MixerProp.bStereoEnable;
	                m_dwRestoreMixerStereoMode = (DWORD)MixerProp.eStereoMode;
                }

                if (!m_bUseMessageThread)
				{
                    hr = ProcessLostDevice();
				}
            }

			// only render when main video time is not updated.
			if (CRenderClock::GetInstance()->Expired())
			{
				hr = RenderScene();
			    hr = Present();
			}
		}

		dwRender = timeGetTime() - dwStart;
		if (dwRender < m_dwInterval)
		{
			Sleep(m_dwInterval - dwRender);
		}
	}
	
	{
		CAutoDisplayLock displayLock(m_pLock);
		m_RenderThreadStatus = eFinished;
	}
	return hr;
}

// ThreadProc processing rendering of the Render Engine: calls for Render
UINT WINAPI CRenderEngine::RenderThreadProc(LPVOID lpParameter)
{
	CRenderEngine* pThis = (CRenderEngine*)lpParameter;
	if (!pThis)
	{
		DbgMsg("CRenderEngine::RenderThreadProc: parameter is NULL");
		return 0;
	}
	pThis->AddRef();
	::CoInitialize(NULL);
	pThis->RenderLoop();
	pThis->Release();
	::CoUninitialize();

	_endthreadex( 0 );
	return 0;
}

HRESULT CRenderEngine::SetCustomizedOutput(BOOL bFlag, DWORD dwExtOption)
{
	CAutoDisplayLock displayLock(m_pLock);
	BOOL IsCustomizedOutput = (m_dwConfigFlags & DISPSVR_USE_CUSTOMIZED_OUTPUT) ? TRUE : FALSE;
    BOOL bStereoMode = (dwExtOption & DISPLAY_CUSTOMIZEDOUTPUT_StereoMode) ? TRUE : FALSE;
    BOOL bExclusiveMode = (dwExtOption & DISPLAY_CUSTOMIZEDOUTPUT_ExclusiveMode) ? TRUE : FALSE;
	if ((IsCustomizedOutput == bFlag) &&  (m_bStereoMode == bStereoMode) && (m_bExclusiveMode == bExclusiveMode))
    {
        if(m_eDeviceStatus == eDeviceReady) //Fix bug#104007
            return S_OK;
    }

	DWORD dwVendor = GetRegistry(REG_VENDOR_ID, 0);
	DWORD dwOSVersion = GetRegistry(REG_OS_VERSION, 0);

	// for WinXP and Intel card and use Customized Output(BD case)
	if(dwVendor == PCI_VENDOR_ID_INTEL && dwOSVersion <= OS_XP && bFlag == TRUE)
		return E_FAIL;

    m_DeviceInfo.bCustomizedOutputChanges = (IsCustomizedOutput != bFlag) ? TRUE : FALSE;
	m_DeviceInfo.bStereoModeChanges = (m_bStereoMode != bStereoMode) ? TRUE : FALSE;
    m_DeviceInfo.bExclusiveWindowMode = (m_bExclusiveMode != bExclusiveMode) ? TRUE : FALSE;

    m_bStereoMode = bStereoMode;
    m_bExclusiveMode = bExclusiveMode;    
	// customized output cannot be used in exclusive mode, use exclusive mode.
	if (bFlag && !(m_bExclusiveMode))
		m_dwConfigFlags |= DISPSVR_USE_CUSTOMIZED_OUTPUT;
	else
		m_dwConfigFlags &= ~DISPSVR_USE_CUSTOMIZED_OUTPUT;

    if (m_eDeviceStatus != eDeviceLost)
        m_eDeviceStatus = eDeviceLostDetected;

		return S_FALSE;
	}

HRESULT CRenderEngine::GetRecomVideoPixelArea(DWORD* pdwRecomVideoPixelArea)
{
	HRESULT hr = E_FAIL;

	if (m_pDispSvrVideoPresenter)
	{
		PresenterCaps Caps = {0};
		Caps.dwSize = sizeof(PresenterCaps);
		hr = m_pDispSvrVideoPresenter->QueryCaps(&Caps);
		if (SUCCEEDED(hr))
		{
			*pdwRecomVideoPixelArea = Caps.dwResPixels;
		}
	}
	return hr;
}

HRESULT CRenderEngine::NotifyDWMQueuing(BOOL bEnable)
{
	HRESULT hr = E_FAIL;
    BOOL bDWMQueuing = FALSE;

	CComPtr<IDispSvrVideoPresenter> pDispSvrVideoPresenter;
	hr = CResourceManager::GetInstance()->GetInterface(DISPSVR_RESOURCE_VIDEOPRESENTER, __uuidof(IDispSvrVideoPresenter), (void**)&pDispSvrVideoPresenter);

	if (SUCCEEDED(hr) && pDispSvrVideoPresenter)
	{
       	PresenterProperty VideoPresenterProperty = {0};
     	VideoPresenterProperty.dwSize = sizeof(PresenterProperty);
	    hr = pDispSvrVideoPresenter->GetProperty(&VideoPresenterProperty);

	    bDWMQueuing = (VideoPresenterProperty.dwFlags & PRESENTER_PROPERTY_DWMQUEUINGENABLED) ? TRUE : FALSE;
		if (bEnable != bDWMQueuing)
		{
			if (bEnable)
				VideoPresenterProperty.dwFlags |= PRESENTER_PROPERTY_DWMQUEUINGENABLED;
			else
				VideoPresenterProperty.dwFlags &= ~PRESENTER_PROPERTY_DWMQUEUINGENABLED;
			hr = pDispSvrVideoPresenter->SetProperty(&VideoPresenterProperty);
		}
	}
	return hr;
}

HRESULT CRenderEngine::ProcessDWM(BOOL bEnable)
{
    HRESULT hr = S_OK;
    DWORD dwOSVersion = GetRegistry(REG_OS_VERSION, 0);
    DWORD dwVendorID = GetRegistry(REG_VENDOR_ID, 0);

    PresenterProperty VideoPresenterProperty;
    ZeroMemory(&VideoPresenterProperty, sizeof(PresenterProperty));
    VideoPresenterProperty.dwSize = sizeof(PresenterProperty);
    m_pDispSvrVideoPresenter->GetProperty(&VideoPresenterProperty);    
    
    BOOL bIsDWMEnable = FALSE;
    if(CDynLibManager::GetInstance()->pfnDwmIsCompositionEnabled)
        CDynLibManager::GetInstance()->pfnDwmIsCompositionEnabled(&bIsDWMEnable);

    VideoPresenterProperty.dwFlags &= ~PRESENTER_PROPERTY_DWMQUEUINGENABLED;
    if(bIsDWMEnable && bEnable)
        VideoPresenterProperty.dwFlags |= PRESENTER_PROPERTY_DWMQUEUINGENABLED;

    hr = m_pDispSvrVideoPresenter->SetProperty(&VideoPresenterProperty);

    if (bEnable != m_bIsDWMEnabled && dwOSVersion >= OS_WIN7 && PCI_VENDOR_ID_ATI == dwVendorID && m_dwConfigFlags & DISPSVR_USE_CUSTOMIZED_OUTPUT)
    {
        if (m_dwConfigFlags & DISPSVR_DEVICE_LOST_NOTIFY)
        {
            if (m_eDeviceStatus != eDeviceLost)
				m_eDeviceStatus = eDeviceLostDetected;
            m_bIsDWMEnabled = bIsDWMEnable;
        }
    }

    return hr;
}

HRESULT CRenderEngine::UpdateHJDetectDesc()
{
	if (!m_pHJDetect)
        return S_FALSE;

    HJDETDESC HJDD;
	ZeroMemory(&HJDD, sizeof(HJDETDESC));
	HJDD.Targets = HJ_TARGET_D3D9;
	HJDD.hWnd = m_hwnVideoWindow;
	HJDD.pInterface = (LPDWORD)(IDirect3DDevice9 *)m_pD3D9Helper->GetDevice();
	return m_pHJDetect->SetDesc(&HJDD);
}

HRESULT CRenderEngine::ResetDevice()
{
	if (m_eDeviceStatus != eDeviceChanging)
		return E_UNEXPECTED;

    CD3D9Helper::ConfigParam config = {0};
    HRESULT hr = S_OK;

    hr = GenerateConfigParamFromEngine(config);
	hr = m_pD3D9Helper->ResetDevice(config);

	if (SUCCEEDED(hr))
        hr = InitResources(true);

    if (FAILED(hr))
		m_eDeviceStatus = eDeviceLost;
	else
		NotifyRenderEngineEvent(DISPSVR_RENDER_ENGINE_DEVICE_RECOVERED, 0, 0);

    DbgMsg("CRenderEngine::ProcessLostDevice; ResetEx %s", FAILED(hr) ? "FAILED" : "SUCCEEDED");
	return hr;
}

HRESULT CRenderEngine::InitResources(bool bResetCase)
{
    HRESULT hr = UpdateHJDetectDesc();

	m_hMonitor = MonitorFromWindow(m_hwnVideoWindow, MONITOR_DEFAULTTONEAREST);

    // enable HDMI 1.4 Stereo mode
	if (m_pHDMIHelper && m_bStereoMode && (m_bEnableHDMIStereoAfterDeviceReset || m_pHDMIHelper->IsHDMIStereoModeEnabled()))
	{
		DriverExtHDMIStereoModeCap StereoMode = {0};
		StereoMode.uWidth = GetRegistry(REG_DISPLAY_WIDTH, 0);
		StereoMode.uHeight = GetRegistry(REG_DISPLAY_HEIGHT, 0);
		StereoMode.uRefreshRate = GetRegistry(REG_DISPLAY_REFRESH_RATE, 0);
		if (SUCCEEDED(m_pHDMIHelper->GetAppropriateHDMIDisplayMode(&StereoMode)))
		{
			BOOL bRecreateDevice = FALSE;
			if (SUCCEEDED(m_pHDMIHelper->EnableHDMIStereoMode(TRUE, &StereoMode, &bRecreateDevice)))
				m_bEnableHDMIStereoAfterDeviceReset = FALSE;
		}
	}

    hr = ConfigureResource(bResetCase);
    if (SUCCEEDED(hr))
    {
        UpdateDisplayWindow();

		if (m_pDispSvrVideoPresenter)
			m_pDispSvrVideoPresenter->SetColorKey(m_dwColorKey);

        {
            MixerProperty MixerProp;
            CComPtr<IDispSvrVideoMixer> pDispSvrVideoMixer;
            if(SUCCEEDED(CResourceManager::GetInstance()->GetInterface(DISPSVR_RESOURCE_VIDEOMIXER, __uuidof(IDispSvrVideoMixer), (void **)&pDispSvrVideoMixer)))
            {
                pDispSvrVideoMixer->GetProperty(&MixerProp);
                MixerProp.bStereoEnable = m_bRestoreMixerStereoEnable;
                MixerProp.eStereoMode =  (MIXER_STEREO_MODE)m_dwRestoreMixerStereoMode;
                pDispSvrVideoMixer->SetProperty(&MixerProp);
            }
        }

        memset(&m_DeviceInfo, 0, sizeof(m_DeviceInfo));
	    m_eDeviceStatus = eDeviceReady;
    }
    return hr;
}

HRESULT CRenderEngine::CheckDeviceState()
{
    // workaround for bug 80309, 81780, 65174
    // because device lost on WinXP will never get D3DERR_DEVICENOTRESET by m_pDevice->TestCooperativeLevel();
    DWORD dwOSVersion = GetRegistry(REG_OS_VERSION, 0);
    if(dwOSVersion <= OS_XP && (m_eDeviceStatus == eDeviceLost || m_eDeviceStatus == eDeviceChanging))
        return D3DERR_DEVICENOTRESET;

    HRESULT hr = m_pD3D9Helper->CheckDeviceState();

	UpdateOverlayStatus(FALSE, hr);

	// When using 3rd party overlay solutions, d3d Present/PresentEx is not called so
	// that CheckDeviceState does not return correct state.
	// Use GetAdapterDisplayModeEx to workaround this issue.
    if (m_bCheckDisplayChange && m_bIsOverlayPresenter && SUCCEEDED(hr) && hr != S_PRESENT_MODE_CHANGED)
	{
		hr = m_pD3D9Helper->CheckModeSwitch();
        m_bCheckDisplayChange = FALSE;
	}

	// We treat mode switch as device lost to reset swap chains by the current display size
	// or format and also reset color key when changing color depth in overlay mode.
	if (hr == S_PRESENT_MODE_CHANGED && m_eDeviceStatus == eDeviceReady)
	{
		hr = D3DERR_DEVICELOST;
	}

    return hr;
}

HRESULT CRenderEngine::GenerateConfigParamFromEngine(CD3D9Helper::ConfigParam &config)
{
    HRESULT hr = S_OK;

    config.bUseD3D9Ex = (m_dwConfigFlags & DISPSVR_USE_D3D9EX) != 0;
    config.bPreserveFPU = (m_dwConfigFlags & DISPSVR_FPU_PRESERVE) != 0;
    config.bUseProtectedOutput = (m_dwConfigFlags & DISPSVR_USE_CUSTOMIZED_OUTPUT) != 0;
    config.bWaitForVsync = (m_dwConfigFlags & DISPSVR_WAITING_FOR_VSYNC) != 0;
    config.bUseStencilDepthBuffer = (m_dwConfigFlags & DISPSVR_USE_STENCIL_BUFFER) != 0;
    config.bUseExclusiveMode = m_bExclusiveMode != FALSE;
    config.bUseHDMIStereoMode = m_bStereoMode && (m_bEnableHDMIStereoAfterDeviceReset || m_pHDMIHelper->IsHDMIStereoModeEnabled());

    return S_OK;
}

HRESULT CRenderEngine::UpdateDisplayWindow()
{
    HRESULT hr = S_OK;

    if (m_bInitialized)
    {
        hr = CResourceManager::GetInstance()->ProcessMessage(RESOURCE_MESSAGE_SETWINDOWHANDLE, m_hwnVideoWindow);
        hr = CResourceManager::GetInstance()->ProcessMessage(RESOURCE_MESSAGE_SETWINDOWSTATE, reinterpret_cast<RESOURCE_WINDOW_STATE *> (&m_eWindowState));

        if (m_pDispSvrVideoPresenter)
        {
            hr = m_pDispSvrVideoPresenter->SetDisplayRect(&m_rectWindow, &m_rectSrc);
        }
    }
    return hr;
}

HRESULT CRenderEngine::ConfigureHDMIStereoBeforeDeviceReset()
{
	if (!m_pHDMIHelper)
		return E_FAIL;

	HRESULT hr = S_FALSE;
	m_bEnableHDMIStereoAfterDeviceReset = FALSE;

	MixerProperty MixerProp;
	ZeroMemory( &MixerProp, sizeof(MixerProperty));
	CComPtr<IDispSvrVideoMixer> pDispSvrVideoMixer;
	if(SUCCEEDED(CResourceManager::GetInstance()->GetInterface(DISPSVR_RESOURCE_VIDEOMIXER, __uuidof(IDispSvrVideoMixer), (void **)&pDispSvrVideoMixer)))
	{
		pDispSvrVideoMixer->GetProperty(&MixerProp);
	}

	if (m_DeviceInfo.bStereoModeChanges)
	{
		if (GetRegistry(REG_VENDOR_ID, 0) == PCI_VENDOR_ID_NVIDIA && m_eDeviceStatus == eDeviceLost)
		{
			m_bEnableHDMIStereoAfterDeviceReset = TRUE;
		}

		//Set HDMI Stereo mode if stereo mode enabled and mode equals MIXER_STEREO_MODE_HDMI_STEREO
		if ( !m_bEnableHDMIStereoAfterDeviceReset && m_bStereoMode && MixerProp.eStereoMode == MIXER_STEREO_MODE_HDMI_STEREO)
		{
			DriverExtHDMIStereoModeCap StereoMode = {0};
			StereoMode.uWidth = GetRegistry(REG_DISPLAY_WIDTH, 0);
			StereoMode.uHeight = GetRegistry(REG_DISPLAY_HEIGHT, 0);
			StereoMode.uRefreshRate = GetRegistry(REG_DISPLAY_REFRESH_RATE, 0);
			if (!m_pHDMIHelper->IsHDMIStereoModeEnabled())
			{
				m_pHDMIHelper->SetDefaultDisplayMode(&StereoMode);
			}
			if (SUCCEEDED(m_pHDMIHelper->GetAppropriateHDMIDisplayMode(&StereoMode)))
			{
				if (m_pHDMIHelper->IsHDMIStereoModeSupported(NULL)) // support HDMI stereo mode
				{
					BOOL bReCreateDevice = FALSE;
					hr = m_pHDMIHelper->EnableHDMIStereoMode(TRUE, &StereoMode, &bReCreateDevice);
					if (SUCCEEDED(hr))
					{
						hr = S_OK;
					}
					else
					{
						m_bEnableHDMIStereoAfterDeviceReset = TRUE;
					}
				}
			}
		}
	}

	if (hr != S_OK && m_pHDMIHelper->IsHDMIStereoModeEnabled())
	{
		DbgMsg("CRenderEngine::SetCustomizedOutput - Disable HDMI Stereo");
		BOOL bReCreateDevice = FALSE;
		hr = m_pHDMIHelper->EnableHDMIStereoMode(FALSE, NULL, &bReCreateDevice);
		if (SUCCEEDED(hr) && MixerProp.eStereoMode == MIXER_STEREO_MODE_HDMI_STEREO)
		{
			MixerProp.eStereoMode = MIXER_STEREO_MODE_DISABLED;
			pDispSvrVideoMixer->SetProperty(&MixerProp);
		}
	}

	return hr;
}

void CRenderEngine::UpdateOverlayStatus(BOOL bHide, long hrStatus)
{
	HRESULT hr = S_OK;
	if (hrStatus == S_PRESENT_OCCLUDED)
	{
		m_bHideOverlay = TRUE;
		CResourceManager::GetInstance()->ProcessMessage(RESOURCE_MESSAGE_HIDEWINDOW, &m_bHideOverlay);
		return;
	}

	if (bHide)
	{
		m_bHideOverlay = TRUE;
		CResourceManager::GetInstance()->ProcessMessage(RESOURCE_MESSAGE_HIDEWINDOW, &m_bHideOverlay);
	}
	else
	{
		m_bHideOverlay = FALSE;
		CResourceManager::GetInstance()->ProcessMessage(RESOURCE_MESSAGE_HIDEWINDOW, &m_bHideOverlay);
	}
}