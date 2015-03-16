#include "Imports/ThirdParty/Microsoft/DXVAHD/d3d9.h"
#include "Imports/ThirdParty/Microsoft/DXVAHD/d3d9types.h"
#include "stdafx.h"

#include <process.h>

#include "Imports/LibHijackDetect/HijackDetect.h"
#include "Imports/LibGPU/UtilGPU.h"
#include "Imports/LibGPU/GPUID.h"
#include "Exports/Inc/VideoMixer.h"
#include "Exports/Inc/VideoPresenter.h"

#include "DispSvr.h"
#include "ResourceManager.h"
#include "CompositeDO.h"
#include "ServerStateEventSink.h"
#include "DynLibManager.h"
#include "RegistryService.h"
#include "RenderClock.h"
#include "RenderEngine.h"

#define GetRValue_D3DCOLOR_XRGB(rgb)      (LOBYTE((rgb)>>16))
#define GetGValue_D3DCOLOR_XRGB(rgb)      (LOBYTE(((WORD)(rgb)) >> 8))
#define GetBValue_D3DCOLOR_XRGB(rgb)      (LOBYTE(rgb))
#define DISP_AUTO_PROFILE(id, subid) { if (m_bFirstTimeDisp) {AUTO_SCOPE_PROFILE(id, subid)}}
const int _BACK_BUFFER_COUNT_ = 3;

using namespace DispSvr;

static inline void CalculateTransformToRtv(D3DXMATRIX &matrix, RECT &rectSrc, const RECT &rectWindow,
                                           const SIZE &sizeRt, const D3DDISPLAYMODE &displayMode, bool bOriginRTV)
{
    double w = sizeRt.cx / double(displayMode.Width);
    double h = sizeRt.cy / double(displayMode.Height);

    rectSrc.left = LONG(rectWindow.left * w);
    rectSrc.right = LONG(rectWindow.right * w);
    rectSrc.top = LONG(rectWindow.top * h);
    rectSrc.bottom = LONG(rectWindow.bottom * h);

    if (bOriginRTV)
    {
        if (rectSrc.left > 0)
        {
            rectSrc.right -= rectSrc.left;
            rectSrc.left = 0;
        }

        if (rectSrc.top > 0)
        {
            rectSrc.bottom -= rectSrc.top;
            rectSrc.top = 0;
        }
    }

    // Produce the transformation matrix to transform vertices to window position
    // Texture coordinates are measured from the center of the texel, while vertex
    // coordinates are absolute.
    // We should subtract all vertices by 0.5 to show texel in 2D correctly.
    // For the sake of later vertex calculation, we offset the 0.5 in the translation.
    //
    // The simplified matrix is derived from:
    //
    // D3DXMatrixTranslation(m1, 1, -1, 0);
    // D3DXMatrixScaling(m2,
    //   FLOAT(rectSrc.right - rectSrc.left) / sizeRt.cx
    //   FLOAT(rectSrc.bottom - rectSrc.top) / sizeRt.cy,
    //   0);
    // D3DXMatrixTranslation(m3,
    //   FLOAT(rectSrc.left - 0.5) * 2 / sizeRt.cx - 1,
    //   -FLOAT(rectSrc.top - 0.5) * 2 / sizeRt.cy + 1,
    //   0);
    // matrix = m1 * m2 * m3;
    D3DXMatrixIdentity(&matrix);
    matrix(0, 0) = FLOAT(rectSrc.right - rectSrc.left) / sizeRt.cx;
    matrix(1, 1) = FLOAT(rectSrc.bottom - rectSrc.top) / sizeRt.cy;
    matrix(3, 0) = matrix(0, 0) + FLOAT(rectSrc.left - 0.5) * 2 / sizeRt.cx - 1;
    matrix(3, 1) = -matrix(1, 1) - FLOAT(rectSrc.top - 0.5) * 2 / sizeRt.cy + 1;
}

CRenderEngine::CRenderEngine(LPUNKNOWN pUnk, HRESULT *phr) : CUnknown(NAME("Display Render Engine"), pUnk)
{
    m_bInitialized = FALSE;
    m_eDeviceStatus = eDeviceReady;
    m_bAutoRenderThread = TRUE;
    m_hRenderThread = NULL;
    m_bFreezeState = FALSE;
    m_bEnableRendering = TRUE;
    SetBackgroundColor(RGB(0x00, 0x00, 0x00));

    m_RenderThreadStatus = eNotStarted;
    ZeroMemory(&m_rectDisp,sizeof(m_rectDisp));
    ZeroMemory(&m_rectSrc, sizeof(m_rectSrc));
    ZeroMemory(&m_rectWindow, sizeof(m_rectWindow));

    m_eRTSize = (RenderTargetSize) GetRegInt(_T("DispSvrRTSize"), 1);

    // make sure timer is at least 2 ms accurate
    timeBeginPeriod(2);
    m_pHJDetect = NULL;

    ZeroMemory(&m_d3dDisplayMode, sizeof(D3DDISPLAYMODE));
    m_hMonitor = 0;
    m_hwndMessage = 0;
    m_hwnVideoWindow = 0;
    m_pfnRenderFunc = &CRenderEngine::RenderDispObj;
    m_bICT = FALSE;
    m_bHide = FALSE;
    m_pStateEventSink = CServerStateEventSink::GetInstance();
    m_dwInterval = 1000 / 30;	// default frame rate set to 30fps
    m_dwColorFillTimes = 0;
    m_dwClearTimes = GetRegInt(_T("DispSvrClearRTTimes"), 10);
    m_bEnableColorFill = TRUE;
    m_dwFrameRate = 60;
    m_dwVideoFormat = OR_FRAME_PROGRESSIVE;
    m_bCheckDisplayChange = FALSE;
    m_bExclusiveMode = FALSE;
    m_bUseMessageThread = FALSE;
	m_bIsD3D9RenderTarget = false;
    m_bUseD3D9Overlay = false;
	m_bUseRTV = false;
    m_bRTVFromOrigin = false;
    m_bIsOverlayPresenter = false;
    m_bFirstTimeDisp = true;
}

CRenderEngine::~CRenderEngine()
{
    // call off the timer
    timeEndPeriod(2);
    Terminate();
    m_hwnVideoWindow = 0;
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
    else if (riid == __uuidof(IDispSvrVideoMixer) && GetRegInt(_T("DispSvrMixer"), 1))
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
        CAutoDisplayLock displayLock(m_pLock);

        D3DXMatrixIdentity(&m_matrixWorld);
        //When drag APP from primary to extend monitor or reverse or DeviceLost, CreateWindow will give the error window handle
        //If m_hwnVideoWindow already has the value, don't call SetDisplayWindow again.

        if(NULL == m_hwnVideoWindow)
        {
            RECT rc = {0, 0, 0, 0};
            hr = SetDisplayWindow(hWnd, &rc);
            CHECK_HR(hr, DbgMsg("CRenderEngine::Initialize: failed to set display window"));
        }

        if (dwFlags & DISPSVR_USE_D3D9EX)
        {
            if (!CDynLibManager::GetInstance()->pfnDirect3DCreate9Ex)
                dwFlags &= ~DISPSVR_USE_D3D9EX;
        }

        if (dwFlags & DISPSVR_NO_WINDOW_OUTPUT)
        {
            m_pfnRenderFunc = &CRenderEngine::RenderNoWindowOutput;
        }

        if (dwFlags & DISPSVR_DETECT_D3D_HIJACK)
        {
            m_pHJDetect = new CHijackDetect;
        }
        m_sizeBackBuffer.cx = BackBufferWidth;
        m_sizeBackBuffer.cy = BackBufferHeight;

        if (dwFlags & DISPSVR_USE_MESSAGE_THREAD)
            m_bUseMessageThread = TRUE;

        if (dwFlags & DISPSVR_USE_EXCLUSIVE_MODE)
            m_bExclusiveMode = TRUE;

        // TODO: check flags
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
    m_pD3D9.Release();
    m_pD3D9Ex.Release();

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

#define MAX_RENDERTARGET_WIDTH	1920
#define MAX_RENDERTARGET_HEIGHT	1080
#define MIN_RENDERTARGET_WIDTH	MAX_RENDERTARGET_WIDTH
#define MIN_RENDERTARGET_HEIGHT	MAX_RENDERTARGET_HEIGHT

HRESULT CRenderEngine::GetRecommendedBackBufferSize(UINT *pWidth, UINT *pHeight)
{
    // For BD+ DiscoveryRAM Tags, BD+ Content Code would check the pattern at runtime.
    static unsigned char DATA_TAG_10[] =
    {0x17, 0x82, 0x76, 0x4B, 0x60, 0x38, 0x96, 0xDE, 0xF0, 0x18, 0x60, 0x3D, 0x58, 0x62, 0x25, 0x8C};
    if (!m_pDevice)
    {
        DATA_TAG_10[2] ^= DATA_TAG_10[14];
        DATA_TAG_10[2] ^= DATA_TAG_10[14];
        return S_FALSE;
    }

    UINT uDisplayWidth = m_d3dDisplayMode.Width;
    UINT uDisplayHeight = m_d3dDisplayMode.Height;
    UINT uMaxWidth = MAX_RENDERTARGET_WIDTH;
    UINT uMaxHeight = MAX_RENDERTARGET_HEIGHT;
    MONITORINFOEX MonInfo;

    MonInfo.cbSize = sizeof(MONITORINFOEX);
    if (GetMonitorInfo(m_hMonitor, &MonInfo) != 0)
    {
        uDisplayWidth = MonInfo.rcMonitor.right - MonInfo.rcMonitor.left;
        uDisplayHeight = MonInfo.rcMonitor.bottom - MonInfo.rcMonitor.top;
    }

    switch(m_eRTSize)
    {
    case eScreenResolutionFixed:
        {
            uMaxWidth = (uDisplayWidth>MAX_RENDERTARGET_WIDTH) ? uDisplayWidth : min(uDisplayWidth, MAX_RENDERTARGET_WIDTH);
            uMaxHeight = (uDisplayHeight>MAX_RENDERTARGET_HEIGHT) ? uDisplayHeight : min(uDisplayHeight, MAX_RENDERTARGET_HEIGHT);
            if(m_bICT)
            {
                *pWidth = min(*pWidth, uMaxWidth);
                *pHeight = min(*pHeight, uMaxHeight);
            }
            else
            {
                *pWidth = uMaxWidth;
                *pHeight = uMaxHeight;
            }
        }
        break;
    case eScreenResolutionMax:
        uMaxWidth = min(uDisplayWidth, MAX_RENDERTARGET_WIDTH);
        uMaxHeight = min(uDisplayHeight, MAX_RENDERTARGET_HEIGHT);
        *pWidth = *pWidth > uMaxWidth ? uMaxWidth:*pWidth;
        *pHeight = *pHeight > uMaxHeight ? uMaxHeight:*pHeight;
        break;
    }

    return S_OK;
}

STDMETHODIMP CRenderEngine::SetBackBufferSize(UINT BackBufferWidth, UINT BackBufferHeight)
{
    GetRecommendedBackBufferSize(&BackBufferWidth, &BackBufferHeight);
    if (m_sizeBackBuffer.cx == (INT) BackBufferWidth && 
        m_sizeBackBuffer.cy == (INT) BackBufferHeight && m_bEnableRendering == TRUE && 
        m_eDeviceStatus != eDeviceRecoverable)
    {
        return S_FALSE;
    }

    {
        CAutoDisplayLock displayLock(m_pLock);
        m_sizeBackBuffer.cx = BackBufferWidth;
        m_sizeBackBuffer.cy = BackBufferHeight;
        SetRegistry(REG_BACKBUFFER_WIDTH, m_sizeBackBuffer.cx);
        SetRegistry(REG_BACKBUFFER_HEIGHT, m_sizeBackBuffer.cy);
        m_eDeviceStatus = eDeviceChanging;
    }

    //Ignore wait device change to avoid application hanging
    if (!m_bUseMessageThread)
        return WaitDeviceChange();

    return S_OK;
}

STDMETHODIMP CRenderEngine::GetBackBufferSize(UINT* BackBufferWidth, UINT* BackBufferHeight)
{
    CAutoDisplayLock displayLock(m_pLock);

    if (BackBufferWidth == NULL || BackBufferHeight == NULL)
    {
        return E_POINTER;
    }

    *BackBufferWidth = m_sizeBackBuffer.cx;
    *BackBufferHeight = m_sizeBackBuffer.cy;

    return S_OK;
}

static const NORMALIZEDRECT rectClippingSpace = { -1.0f, 1.0f, 1.0f, -1.0f };

HRESULT CRenderEngine::RenderNoWindowOutput()
{
    // Set appropriate state
    D3DXMATRIX idMatrix;
    D3DXMatrixIdentity(&idMatrix);
    HRESULT hr = m_pDevice->SetTransform(D3DTS_WORLD, &idMatrix);
    hr = m_pDevice->SetTransform(D3DTS_VIEW, &idMatrix);
    hr = m_pDevice->SetTransform(D3DTS_PROJECTION, &idMatrix);
    hr = m_pDevice->SetRenderState(D3DRS_LIGHTING, FALSE);

    if (m_pRootDO)
    {
        hr = m_pRootDO->Render(m_pDevice, &rectClippingSpace);
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

    // rendertarget may be changed by VMR9 in RGB mixing mode.
    hr = m_pDevice->SetRenderTarget(0, m_pRenderTarget);

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

    if(m_bEnableColorFill)
        m_dwColorFillTimes = 1;
    // render the scene, if someone call NodeRequest-DISPLAY_REQUEST_ClearRenderTarget function, we need to clear RT.
    if (m_bIsD3D9RenderTarget && m_dwColorFillTimes > 0)
    {
        if (hr == S_OK)
            m_dwColorFillTimes--;

        DWORD dwFlags = (m_dwConfigFlags & DISPSVR_USE_STENCIL_BUFFER) ? D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL : D3DCLEAR_TARGET;

        hr = m_pDevice->Clear(0L,	// no rects (clear all)
            NULL,					// clear entire viewport
            dwFlags,		// clear render target
            m_d3dBackgroundColor,
            1.0f,					// z buffer depth
            0L);					// no stencil
        if (FAILED(hr))
        {
            DbgMsg("CRenderEngine::Render: failed in IDirect3DDevice9::Clear, hr = 0x%08x", hr);
            return hr;
        }

		//Calling Clear API does not work correct on special NV driver(185.xx).
		//We add to call API ¡V ColorFill for clear render target to avoid this issue.
		IDirect3DSurface9 *pRenderTarget = 0;
		hr = m_pDevice->GetRenderTarget(0, &pRenderTarget);
		if (FAILED(hr))
		{
			DbgMsg("CRenderEngine::Render: failed in IDirect3DDevice9::pRenderTarget, hr = 0x%08x", hr);
			return hr;
		}
		else
		{
			hr = m_pDevice->ColorFill(pRenderTarget, 0, 0);
			SAFE_RELEASE(pRenderTarget);

			if (FAILED(hr))
			{
				DbgMsg("CRenderEngine::Render: failed in IDirect3DDevice9::ColorFill, hr = 0x%08x", hr);
				return hr;
			}
		}
    }

    if (m_pRootDO)
    {
        if (m_bUseRTV)
        {
            D3DXMATRIX mx;
            m_pDevice->GetTransform(D3DTS_WORLD, &mx);
            m_pDevice->SetTransform(D3DTS_WORLD, &m_matrixWorld);
            hr = m_pRootDO->Render(m_pDevice, &rectClippingSpace);
            m_pDevice->SetTransform(D3DTS_WORLD, &mx);
        }
        else
        {
            hr = m_pRootDO->Render(m_pDevice, &rectClippingSpace);
        }

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

HRESULT CRenderEngine::RenderScene()
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
        if (!m_pDevice)
        {
            DbgMsg("CRenderEngine::Render: no Direct3D device is available");
            return VFW_E_WRONG_STATE;
        }
        hr = (this->*m_pfnRenderFunc)();
    }

    if (FAILED(hr))
    {
        if (FAILED(CheckDeviceState()))
        {
            if ((m_dwConfigFlags & DISPSVR_DEVICE_LOST_NOTIFY)/* && m_hwndMessage*/)
            {
                if (m_eDeviceStatus != eDeviceLost)
                    m_eDeviceStatus = eDeviceLostDetected;
            }
            else
                m_eDeviceStatus = eDeviceChanging;
        }
        hr = E_FAIL;		// do not return D3D error to let GPI recover device lost.
    }
    return hr;
}

HRESULT CRenderEngine::Present()
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
            if (m_pD3D9Ex)
            {
            	hr = CheckModeSwitch();
                if (hr == S_PRESENT_MODE_CHANGED)
                    throw hr;
            }
            else
            {
                hr = CheckDeviceState();
                CHECK_HR(hr, DbgMsg("CRenderEngine::Present: failed in CheckDeviceState, hr = 0x%08x", hr));
            }

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
        if (FAILED(CheckDeviceState()) || S_PRESENT_MODE_CHANGED == hr)
        {
            if ((m_dwConfigFlags & DISPSVR_DEVICE_LOST_NOTIFY)/* && m_hwndMessage*/)
            {
                if (m_eDeviceStatus != eDeviceLost)
                    m_eDeviceStatus = eDeviceLostDetected;
            }
            else
                m_eDeviceStatus = eDeviceChanging;
        }
        hr = E_FAIL;		// do not return D3D error to let GPI recover device lost.
    }
    return hr;
}

STDMETHODIMP CRenderEngine::ProcessLostDevice()
{
    HRESULT hr = S_OK;
    DWORD dwColorKey;
    {
        // BeginDeviceLoss() should be synchronized with filter graph thread or VMR
        // mixer thread so that device lost can be detected.

        CAutoDisplayLock displayLock(m_pLock);

        if (m_eDeviceStatus == eDeviceReady)
            return S_FALSE;

        if (m_eDeviceStatus != eDeviceChanging)
            return E_UNEXPECTED;

        DbgMsg("CRenderEngine::ProcessLostDevice: calling BeginDeviceLoss thread id=%d", ::GetCurrentThreadId());

        // Get Color Key before process device lost
        if (m_pDispSvrVideoPresenter)
        {
            hr = m_pDispSvrVideoPresenter->GetColorKey(&dwColorKey);
        }

        if (m_pDevice)
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

    if (SUCCEEDED(hr) && m_pDevice)
    {
        // Note that display server lock is NOT held when calling EndDeviceLoss() because
        // it may introduce some possible deadlock between render thread and filter graph thread.

        DbgMsg("CRenderEngine::ProcessLostDevice: calling EndDeviceLoss thread id=%d", ::GetCurrentThreadId());

        if (m_pRootDO)
            m_pRootDO->EndDeviceLoss(m_pDevice);
        if (m_pOwner)
            hr = m_pOwner->EndDeviceLoss(m_pDevice);

        // Set window destination and video source rectangles to video mixer after device is recovered.
        // Set Color Key after device is recovered.
		// Update Display Rect
		if (m_bUseRTV)
		{
			CalculateTransformToRtv(m_matrixWorld, m_rectSrc, m_rectWindow, m_sizeBackBuffer, m_d3dDisplayMode, m_bRTVFromOrigin);
		}

        if (m_pDispSvrVideoPresenter)
        {
            hr = m_pDispSvrVideoPresenter->SetDisplayRect(&m_rectWindow, &m_rectSrc);
            hr = m_pDispSvrVideoPresenter->SetColorKey(dwColorKey);
        }

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
        m_eDeviceStatus = eDeviceLostDetected;

        DbgMsg("Caution!! CRenderEngine::ProcessLostDevice() failed!!\n");
    }

    return hr;
}

STDMETHODIMP CRenderEngine::GetRootObject(REFIID riid, void** ppvObject)
{
    if (!ppvObject)
    {
        return E_POINTER;
    }

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
    if (nFramesPerSecBy100 < 100 ||
        nFramesPerSecBy100 > 6000)
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
    bool bNeedRecreateDevice = m_bUseD3D9Overlay && (hwnd != m_hwnVideoWindow);

    m_hwnVideoWindow = hwnd;
    m_rectDisp = *pRect;
    m_bHide = m_rectDisp.bottom < 0 && m_rectDisp.left < 0 && m_rectDisp.right < 0 && m_rectDisp.top < 0;

    if (m_hwnVideoWindow && m_hMonitor)
    {
        HMONITOR hMonitor = MonitorFromWindow(m_hwnVideoWindow, MONITOR_DEFAULTTONEAREST);
        if (hMonitor != m_hMonitor)
        {
            m_hMonitor = hMonitor;
            bNeedRecreateDevice = true;
        }
    }

    if (bNeedRecreateDevice)
    {
        CAutoDisplayLock displayLock(m_pLock);
        if (m_eDeviceStatus == eDeviceReady)
        {
            m_pD3D9.Release();
            m_pD3D9Ex.Release();

            if ((m_dwConfigFlags & DISPSVR_DEVICE_LOST_NOTIFY)/* && m_hwndMessage*/)
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

        if (m_bUseRTV)
        {
            CalculateTransformToRtv(m_matrixWorld, m_rectSrc, m_rectWindow, m_sizeBackBuffer, m_d3dDisplayMode, m_bRTVFromOrigin);
        }
        else
        {
            m_rectSrc.left = m_rectSrc.top = 0;
            m_rectSrc.right = m_sizeBackBuffer.cx;
            m_rectSrc.bottom = m_sizeBackBuffer.cy;
        }

        if (m_bInitialized)
        {
            hr = CResourceManager::GetInstance()->ProcessMessage(RESOURCE_MESSAGE_SETWINDOWHANDLE, hwnd);

            RESOURCE_WINDOW_STATE eWindowState = RESOURCE_WINDOW_STATE_WINDOWED;
            if (m_bHide)
            {
                eWindowState = RESOURCE_WINDOW_STATE_MINIMIZED;
            }
            else if (m_rectWindow.left == 0 && m_rectWindow.top == 0
                && m_rectWindow.right == (MonInfo.rcMonitor.right - MonInfo.rcMonitor.left)
                && m_rectWindow.bottom == (MonInfo.rcMonitor.bottom - MonInfo.rcMonitor.top))
            {
                eWindowState = RESOURCE_WINDOW_STATE_FULLSCREEN;
            }
            hr = CResourceManager::GetInstance()->ProcessMessage(RESOURCE_MESSAGE_SETWINDOWSTATE, reinterpret_cast<RESOURCE_WINDOW_STATE *> (&eWindowState));

            if (m_pDispSvrVideoPresenter)
            {
                hr = m_pDispSvrVideoPresenter->SetDisplayRect(&m_rectWindow, &m_rectSrc);
            }
        }
    }

    return hr;
}

STDMETHODIMP CRenderEngine::Get3DDevice(IUnknown ** ppDevice)
{
    if (!ppDevice)
    {
        DbgMsg("CRenderEngine::Get3DDevice: received NULL pointer");
        return E_POINTER;
    }

    if (m_pDevice)
    {
        *ppDevice = m_pDevice;
        (*ppDevice)->AddRef();
        return S_OK;
    }

    DbgMsg("CRenderEngine::Get3DDevice: no 3D device is available");
    *ppDevice = NULL;
    return VFW_E_WRONG_STATE;
}

STDMETHODIMP CRenderEngine::Set3DDevice(IUnknown* pDevice)
{
    HRESULT hr = S_OK;
    if (!pDevice)
    {
        DbgMsg("CRenderEngine::Set3DDevice: received NULL pointer");
        return E_POINTER;
    }

    if (FALSE == m_bInitialized)
    {
        DbgMsg("CRenderEngine::Set3DDevice: object is not initialized");
        return VFW_E_WRONG_STATE;
    }

    if (m_pDevice.IsEqualObject(pDevice))
    {
        DbgMsg("CRenderEngine::Set3DDevice: object is same");
        return S_OK;
    }

    CAutoDisplayLock displayLock(m_pLock);

    if (m_pRootDO)
        m_pRootDO->BeginDeviceLoss();
    if (m_pOwner)
        hr = m_pOwner->BeginDeviceLoss();

    ReleaseD3DDevice();

    hr = pDevice->QueryInterface(IID_IDirect3DDevice9, (void**)&m_pDevice);
    if (FAILED(hr))
    {
        DbgMsg("CRenderEngine::Set3DDevice: failed to get D3D9 device");
        return hr;
    }

    hr = ConfigureResource();
    if (FAILED(hr))
        return hr;

    // save the default render target so that we can restore it later (other guy may change it).
    hr = m_pDevice->GetRenderTarget(0, &m_pRenderTarget);
    if (FAILED(hr))
    {
        DbgMsg("CRenderEngine::Set3DDevice: failed to get render target");
        return hr;
    }

    // Update display window.
    D3DDEVICE_CREATION_PARAMETERS cp;
    RECT rect;
    hr = m_pDevice->GetCreationParameters(&cp);	
    GetClientRect(cp.hFocusWindow, &rect);
    SetDisplayWindow(cp.hFocusWindow, &rect);

    if (m_pRootDO)
        m_pRootDO->EndDeviceLoss(m_pDevice);
    if (m_pOwner)
        hr = m_pOwner->EndDeviceLoss(m_pDevice);

    if (m_pHJDetect)
    {
        HJDETDESC HJDD;
        ZeroMemory(&HJDD, sizeof(HJDETDESC));
        HJDD.Targets = HJ_TARGET_D3D9;
        HJDD.hWnd = m_hwnVideoWindow;
        HJDD.pInterface = (LPDWORD)m_pDevice.p;
        m_pHJDetect->SetDesc(&HJDD);
    }

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
    if (!pColor)
    {
        DbgMsg("CRenderEngine::GetBackgroundColor: received NULL pointer");
        return E_POINTER;
    }
    *pColor = RGB(GetRValue_D3DCOLOR_XRGB(m_d3dBackgroundColor), GetGValue_D3DCOLOR_XRGB(m_d3dBackgroundColor), GetBValue_D3DCOLOR_XRGB(m_d3dBackgroundColor));
    return S_OK;
}

STDMETHODIMP CRenderEngine::SetBackgroundColor(COLORREF Color)
{
    CAutoDisplayLock displayLock(m_pLock);

    m_d3dBackgroundColor = D3DCOLOR_XRGB(GetRValue(Color), GetGValue(Color), GetBValue(Color));
    return S_OK;
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
        if (m_bIsD3D9RenderTarget && m_pDevice)
        {
            DWORD dwFlags = (m_dwConfigFlags & DISPSVR_USE_STENCIL_BUFFER) ? D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL : D3DCLEAR_TARGET;

            m_pDevice->Clear(0L,		// no rects (clear all)
                NULL,					// clear entire viewport
                dwFlags,		// clear render target
                m_d3dBackgroundColor,
                1.0f,					// z buffer depth
                0L);					// no stencil
        }
        // To clear render target (RT)
        NodeRequest(DISPLAY_REQUEST_ClearRenderTarget, 0, 0, 0);

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
    return S_OK;
}

STDMETHODIMP CRenderEngine::AdviseEventNotify(IDispSvrRenderEngineNotify *pEventNotify)
{
    CAutoDisplayLock displayLock(m_pLock);
    if (!pEventNotify)
        return E_POINTER;
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
            hr = CResourceManager::GetInstance()->ProcessMessage(RESOURCE_MESSAGE_UPDATEWINDOW, NULL);
        }
        break;
    case WM_SIZE:
        {
            BOOL bHide = (SIZE_MINIMIZED == wParam) ? TRUE : FALSE;
            hr = CResourceManager::GetInstance()->ProcessMessage(RESOURCE_MESSAGE_HIDEWINDOW, &bHide);
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
                    DWORD dwLocalVidMem = GetRegistry(REG_LOCAL_VIDMEM, 256);
                    DWORD dwVendor = GetRegistry(REG_VENDOR_ID, 0);
                    DWORD dwOSVersion = GetRegistry(REG_OS_VERSION, 0);

                    // reallocate resources to avoid fragmentation.
                    if (dwOSVersion <= OS_XP				// OS is XP or before, vista or later OS has video memory virtualization.
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
        //when ICT == TRUE, need SetBackBufferSize to 1/4 resolution, param1 is width and ICT, param2 is height
        if(param1)
        {
            m_bICT = TRUE;
            hr = SetBackBufferSize(param1,param2);
        }
        else
        {
            m_bICT = FALSE;
            hr = SetBackBufferSize(m_d3dDisplayMode.Width, m_d3dDisplayMode.Height);
        }
        break;
    case DISPLAY_REQUEST_CustomizedOutput:
        hr = SetCustomizedOutput(param1 ? TRUE : FALSE);
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
        m_bEnableColorFill = FALSE;
        if(param1)
            m_dwColorFillTimes = param1;
        else
            m_dwColorFillTimes = m_dwClearTimes;
        hr = S_OK;
        break;
    case DISPLAY_REQUEST_ProcessLostDevice:
        {
            hr = S_FALSE;
            if (m_eDeviceStatus == eDeviceRecoverable)
            {
                {
                    CAutoDisplayLock displayLock(m_pLock); // only changing state needs to lock Critical Section.
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
    default:
        return E_INVALIDARG;
    }
    return hr;
}
//////////////////////////////////////////////////////////////////////////
// IDisplayEventHost
STDMETHODIMP CRenderEngine::Register(IDisplayEventHandler* pHandler, LPVOID pInstance)
{
    CAutoDisplayLock displayLock(m_pLock);
    if (!pHandler)
        return E_POINTER;
    EventHandler eh;
    eh.pHandler = pHandler;
    eh.pInstance = pInstance;
    m_EventHandlers.insert(m_EventHandlers.end(), eh);
    return S_OK;
}

STDMETHODIMP CRenderEngine::Unregister(IDisplayEventHandler* pHandler)
{
    CAutoDisplayLock displayLock(m_pLock);
    if (!pHandler)
        return E_POINTER;

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

HRESULT CRenderEngine::CreateD3DDevice9(UINT uDeviceCreateFlag, const D3DFORMAT *pFmts, DisplayDeviceEnv *pEnv)
{
    HRESULT hr = E_FAIL;

    if (!m_pD3D9)
    {
        m_pD3D9 = Direct3DCreate9(D3D_SDK_VERSION);
        if (!m_pD3D9)
        {
            DbgMsg("CRenderEngine::CreateD3DDevice9: failed to create Direct3D9 object");
            return hr;
        }
    }

    UINT nAdapter = D3DADAPTER_DEFAULT;
    D3DDEVTYPE type = D3DDEVTYPE_HAL;

    m_hMonitor = MonitorFromWindow(m_hwnVideoWindow, (m_hwnVideoWindow) ? MONITOR_DEFAULTTONEAREST : MONITOR_DEFAULTTOPRIMARY);

    hr = GetAdapterEx(&nAdapter, &type, m_hMonitor);
    CHECK_HR(hr, DbgMsg("CRenderEngine::GetAdapterEx: failed to get the correct Adapter handle."));

    hr = m_pD3D9->GetAdapterDisplayMode(nAdapter, &m_d3dDisplayMode);
    CHECK_HR(hr, DbgMsg("CRenderEngine::GetAdapterDisplayMode: failed to get display mode."));

    if (m_sizeBackBuffer.cx <= 0 && m_sizeBackBuffer.cy <= 0 || m_bUseRTV)
    {
        m_sizeBackBuffer.cx = m_d3dDisplayMode.Width;
        m_sizeBackBuffer.cy = m_d3dDisplayMode.Height;
    }

    D3DCAPS9 caps;
    hr = m_pD3D9->GetDeviceCaps(nAdapter, type, &caps);

    if (caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
        uDeviceCreateFlag |= D3DCREATE_HARDWARE_VERTEXPROCESSING;
    else
        uDeviceCreateFlag |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;

    for (const D3DFORMAT *pFmt = pFmts; *pFmt != 0; pFmt++)
    {
        D3DPRESENT_PARAMETERS pp = {0};

        pp.Windowed = !m_bExclusiveMode;
        pp.hDeviceWindow = m_hwnVideoWindow;

        // Side Effect of NV6200,Bug#67543
        // need to do further test about setting back buffer size =1 and create stencil buffer by
        //IDirect3DDevice9::CreateDepthStencilSurface() And IDirect3DDevice9::SetDepthStencilSurface()

        //if ((DISPSVR_USE_CUSTOMIZED_OUTPUT & m_dwConfigFlags)
        //	&& (dwVendorID == PCI_VENDOR_ID_ATI || dwVendorID == PCI_VENDOR_ID_NVIDIA))
        //{
        //	pp.BackBufferWidth = 1;
        //	pp.BackBufferHeight = 1;
        //}
        //else
        {
            pp.BackBufferWidth = m_sizeBackBuffer.cx;
            pp.BackBufferHeight = m_sizeBackBuffer.cy;
        }
        pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
        pp.BackBufferCount = 0;
        pp.BackBufferFormat = *pFmt;
        pp.EnableAutoDepthStencil = (m_dwConfigFlags & DISPSVR_USE_STENCIL_BUFFER) ? TRUE : FALSE;
        pp.AutoDepthStencilFormat = (m_dwConfigFlags & DISPSVR_USE_STENCIL_BUFFER) ? D3DFMT_D24S8 : D3DFMT_D16;	// ignored if EnableAutoDepthStencil == FALSE
        pp.Flags = D3DPRESENTFLAG_DEVICECLIP;
        pp.FullScreen_RefreshRateInHz = m_bExclusiveMode ? m_d3dDisplayMode.RefreshRate	: 0;

        if (PCI_VENDOR_ID_S3 == pEnv->dwVendorId)
            pp.Flags |= D3DPRESENTFLAG_VIDEO;	// S3 relies on D3DPRESENTFLAG_VIDEO to create overlay render-target

        if (m_dwConfigFlags & DISPSVR_WAITING_FOR_VSYNC)
            pp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
        else
            pp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

        hr = m_pD3D9->CreateDevice(nAdapter, type, m_hwnVideoWindow, uDeviceCreateFlag, &pp, &m_pDevice);
        if (SUCCEEDED(hr))
        {
            break;
        }
    }
    return hr;
}

HRESULT CRenderEngine::CreateD3DDevice9Ex(UINT uDeviceCreateFlag, const D3DFORMAT *pFmts, DisplayDeviceEnv *pEnv)
{
    HRESULT hr = E_FAIL;
    DWORD dwOSVersion = GetRegistry(REG_OS_VERSION, 0);
	const D3DFORMAT *pFmt = pFmts;
	const D3DFORMAT D3D9OverlayFormatAlternatives[] =
    {
		D3DFMT_YUY2,
        D3DFMT_X8R8G8B8,
        D3DFMT_A8R8G8B8,
        D3DFMT_R5G6B5,
        D3DFMT_X4R4G4B4,
        D3DFMT_A4R4G4B4,
        D3DFMT_A1R5G5B5,
        D3DFMT_X1R5G5B5,
		D3DFORMAT(0)
    };

    if (!m_pD3D9Ex)
    {
        if (CDynLibManager::GetInstance()->pfnDirect3DCreate9Ex)
            hr = CDynLibManager::GetInstance()->pfnDirect3DCreate9Ex(D3D_SDK_VERSION, &m_pD3D9Ex);
        if (FAILED(hr))
        {
            DbgMsg("CRenderEngine::CreateD3DDevice9Ex: failed to create Direct3D9Ex object, version = %d", D3D_SDK_VERSION);
            return hr;
        }
    }
    UINT nAdapter = D3DADAPTER_DEFAULT;
    D3DDEVTYPE type = D3DDEVTYPE_HAL;

    m_hMonitor = MonitorFromWindow(m_hwnVideoWindow, (m_hwnVideoWindow) ? MONITOR_DEFAULTTONEAREST : MONITOR_DEFAULTTOPRIMARY);

    hr = GetAdapterEx(&nAdapter, &type, m_hMonitor);
    CHECK_HR(hr, DbgMsg("CRenderEngine::GetAdapterEx: failed to get the correct Adapter handle."));

    hr = m_pD3D9Ex->GetAdapterDisplayMode(nAdapter, &m_d3dDisplayMode);
    CHECK_HR(hr, DbgMsg("CRenderEngine::GetAdapterDisplayMode: failed to get display mode."));

    D3DDISPLAYMODEEX DispModeEx = {0};
    D3DDISPLAYROTATION Rotation = D3DDISPLAYROTATION_IDENTITY;
    // must set the size or GetAdapterDisplayModeEx will fail.
    DispModeEx.Size = sizeof(D3DDISPLAYMODEEX);
    hr = m_pD3D9Ex->GetAdapterDisplayModeEx(nAdapter, &DispModeEx, &Rotation);

    if (m_sizeBackBuffer.cx <= 0 && m_sizeBackBuffer.cy <= 0 || m_bUseRTV)
    {
        m_sizeBackBuffer.cx = m_d3dDisplayMode.Width;
        m_sizeBackBuffer.cy = m_d3dDisplayMode.Height;
    }

    D3DCAPS9 caps;
    hr = m_pD3D9Ex->GetDeviceCaps(nAdapter, type, &caps);

    uDeviceCreateFlag |= D3DCREATE_DISABLE_DRIVER_MANAGEMENT_EX;
    if (caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
        uDeviceCreateFlag |= D3DCREATE_HARDWARE_VERTEXPROCESSING;
    else
        uDeviceCreateFlag |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;

    CComPtr<IDirect3D9ExOverlayExtension> pOverlayExtension;
    if ((caps.Caps & D3DCAPS_OVERLAY) && (m_dwConfigFlags & DISPSVR_USE_CUSTOMIZED_OUTPUT))
    {
        // On NVIDIA, we use proprietary NvAPI for XvYCC output.
        if (PCI_VENDOR_ID_NVIDIA != pEnv->dwVendorId || !pEnv->bIsXvYCCMonitor)
            m_pD3D9Ex.QueryInterface(&pOverlayExtension);

		if (PCI_VENDOR_ID_INTEL == pEnv->dwVendorId && pEnv->bIsXvYCCMonitor)
			pFmt = D3D9OverlayFormatAlternatives;
    }

    for (; *pFmt != 0; pFmt++)
    {
        D3DPRESENT_PARAMETERS pp = {0};

        pp.Windowed = !m_bExclusiveMode;
        pp.hDeviceWindow = m_hwnVideoWindow;
        pp.BackBufferWidth = m_sizeBackBuffer.cx;
        pp.BackBufferHeight = m_sizeBackBuffer.cy;
		if ((m_dwConfigFlags & DISPSVR_USE_CUSTOMIZED_OUTPUT) == 0 && dwOSVersion >= OS_WIN7)
		{
			pp.SwapEffect = D3DSWAPEFFECT_FLIPEX;
			pp.BackBufferCount = _BACK_BUFFER_COUNT_;
			pp.Flags = 0; // FlipEx fails if D3DPRESENTFLAG_DEVICECLIP is set.
		}
		else
		{
			pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
			pp.BackBufferCount = 0;
			pp.Flags = D3DPRESENTFLAG_DEVICECLIP;
		}
        pp.BackBufferFormat = *pFmt;

		// for YUY2 back buffer, AutoDepthStencil may fail CreateDevice()
		if (m_dwConfigFlags & DISPSVR_USE_STENCIL_BUFFER)
		{
			pp.AutoDepthStencilFormat = D3DFMT_D24S8;	// ignored if EnableAutoDepthStencil == FALSE
			hr = m_pD3D9Ex->CheckDepthStencilMatch(nAdapter, D3DDEVTYPE_HAL, DispModeEx.Format, pp.BackBufferFormat, pp.AutoDepthStencilFormat);
			if (SUCCEEDED(hr))
				pp.EnableAutoDepthStencil = TRUE;
		}
        pp.FullScreen_RefreshRateInHz = m_bExclusiveMode ? m_d3dDisplayMode.RefreshRate	: 0;

        if (PCI_VENDOR_ID_S3 == pEnv->dwVendorId ||  // S3 relies on D3DPRESENTFLAG_VIDEO to create overlay render-target.
            (dwOSVersion >= OS_WIN7 && PCI_VENDOR_ID_NVIDIA == pEnv->dwVendorId))  // On Win7(DXVAHD and D3D9Overlay), overlay surface must be video surface, otherwise, driver will fail the creation.)
            pp.Flags |= D3DPRESENTFLAG_VIDEO;

        if (m_dwConfigFlags & DISPSVR_WAITING_FOR_VSYNC)
            pp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
        else
            pp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

        // Workaround for Intel Cantiga/Eaglelake system for present time is too large with full screen when DWM is off.
        if (pEnv->dwVendorId == PCI_VENDOR_ID_INTEL)
            pp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

        if (pOverlayExtension)
        {
            D3DOVERLAYCAPS overlayCaps;
            hr = pOverlayExtension->CheckDeviceOverlayType(nAdapter, type,
                pp.BackBufferWidth, pp.BackBufferHeight, pp.BackBufferFormat,
                NULL, Rotation, &overlayCaps);
            if (SUCCEEDED(hr))
            {
				if (pEnv->bIsXvYCCMonitor && ((D3DOVERLAYCAPS_YCbCr_BT709_xvYCC | D3DOVERLAYCAPS_YCbCr_BT601_xvYCC) & overlayCaps.Caps) != 0)
					pp.Flags |= D3DPRESENTFLAG_OVERLAY_YCbCr_xvYCC;

                ASSERT(pp.Windowed != FALSE && "D3D9 Overlay can't be used in exclusive mode!");
                ASSERT(pp.BackBufferWidth <= overlayCaps.MaxOverlayDisplayWidth && pp.BackBufferHeight <= overlayCaps.MaxOverlayDisplayHeight);
                pp.SwapEffect = D3DSWAPEFFECT_OVERLAY;
                // According to NV's suggestion, we set to 3 or more for better performance.
                // #80137, inverse telecine operation may be backed up waiting for other operations to continue and VPBltHD calls can be blocked.
                // MSDN : http://msdn.microsoft.com/en-us/library/bb172588(VS.85).aspx
                if (PCI_VENDOR_ID_NVIDIA == pEnv->dwVendorId)
                    pp.BackBufferCount = _BACK_BUFFER_COUNT_;
                else if(PCI_VENDOR_ID_INTEL == pEnv->dwVendorId) //We don't know why the video is black on Intel chip when setting back buffer 3. Need to check with Intel.
                    pp.BackBufferCount = _BACK_BUFFER_COUNT_-1;
                else
                    pp.BackBufferCount = 0;

                // in windowed mode, pFullscreenDisplayMode should set to NULL or it may fail.
                if (m_bExclusiveMode)
                    hr = m_pD3D9Ex->CreateDeviceEx(nAdapter, type, m_hwnVideoWindow, uDeviceCreateFlag, &pp, &DispModeEx, &m_pDeviceEx);
                else 
                    hr = m_pD3D9Ex->CreateDeviceEx(nAdapter, type, m_hwnVideoWindow, uDeviceCreateFlag, &pp, NULL, &m_pDeviceEx);

                if (FAILED(hr))
                {
                    pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
                    pp.BackBufferCount = 0;  // reset to 0 if trying to create a device without overlay.
					if (pEnv->bIsXvYCCMonitor)
						pp.Flags &= ~D3DPRESENTFLAG_OVERLAY_YCbCr_xvYCC;
                }
                else
                {
                    m_bUseD3D9Overlay = true;
					if (pp.Flags & D3DPRESENTFLAG_OVERLAY_YCbCr_xvYCC)
					{
						if (overlayCaps.Caps & D3DOVERLAYCAPS_YCbCr_BT709_xvYCC)
							SetRegistry(REG_DISPLAY_XVYCC_MONITOR_TYPE, DISPLAY_XVYCC_MONITOR_BT709);
						else
							SetRegistry(REG_DISPLAY_XVYCC_MONITOR_TYPE, DISPLAY_XVYCC_MONITOR_BT601);
					}
                }
            }
        }

        // in windowed mode, pFullscreenDisplayMode should set to NULL or it may fail.
        if (!m_pDeviceEx)
        {
            // Side Effect of NV6200,Bug#67543
            // need to do further test about setting back buffer size =1 and create stencil buffer by
            //IDirect3DDevice9::CreateDepthStencilSurface() And IDirect3DDevice9::SetDepthStencilSurface()

            //if ((DISPSVR_USE_CUSTOMIZED_OUTPUT & m_dwConfigFlags)
            //	&& (dwVendorID == PCI_VENDOR_ID_ATI || dwVendorID == PCI_VENDOR_ID_NVIDIA))
            //{
            //	pp.BackBufferWidth = 1;
            //	pp.BackBufferHeight = 1;
            //}
            //else
            {
                pp.BackBufferWidth = m_sizeBackBuffer.cx;
                pp.BackBufferHeight = m_sizeBackBuffer.cy;
            }

            if (m_bExclusiveMode)
                hr = m_pD3D9Ex->CreateDeviceEx(nAdapter, type, m_hwnVideoWindow, uDeviceCreateFlag, &pp, &DispModeEx, &m_pDeviceEx);
            else
                hr = m_pD3D9Ex->CreateDeviceEx(nAdapter, type, m_hwnVideoWindow, uDeviceCreateFlag, &pp, NULL, &m_pDeviceEx);
			if (FAILED(hr) && pp.SwapEffect == D3DSWAPEFFECT_FLIPEX)
			{
				// cancel flipex and try again.
				pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
				pp.BackBufferCount = 0;
				pp.Flags = D3DPRESENTFLAG_DEVICECLIP;
	            hr = m_pD3D9Ex->CreateDeviceEx(nAdapter, type, m_hwnVideoWindow, uDeviceCreateFlag, &pp, NULL, &m_pDeviceEx);
			}
        }

        if (m_pDeviceEx)
        {
            hr = m_pDeviceEx->QueryInterface(IID_IDirect3DDevice9, (void**)&m_pDevice);
            ASSERT(SUCCEEDED(hr));
            break;
        }

    }
    return hr;
}

// creates IDirect3DDevice9
HRESULT CRenderEngine::CreateD3DDevice()
{
    HRESULT hr = S_OK;
    UINT uDeviceCreateFlag = D3DCREATE_MULTITHREADED;
    const D3DFORMAT D3DFormatAlternatives[] =
    {
        D3DFMT_X8R8G8B8,
        D3DFMT_A8R8G8B8,
        D3DFMT_R5G6B5,
        D3DFMT_X4R4G4B4,
        D3DFMT_A4R4G4B4,
        D3DFMT_A1R5G5B5,
        D3DFMT_X1R5G5B5,
        D3DFORMAT(0)
    };

    // For BD+ DiscoveryRAM Tags, BD+ Content Code would check the pattern at runtime.
    static unsigned char DATA_TAG_15[] =
    {0x95, 0xF4, 0x69, 0xBA, 0x4A, 0x5C, 0x68, 0xBC, 0x08, 0x70, 0x82, 0x56, 0x80, 0x62, 0xA3, 0xC7};

    // Set this flag when creating D3D device to prevent CreateDevice from changing FPU state.
    if (m_dwConfigFlags & DISPSVR_FPU_PRESERVE)
        uDeviceCreateFlag |= D3DCREATE_FPU_PRESERVE;

    DisplayDeviceEnv sDeviceEnv = {0};
    sDeviceEnv.dwTotalLocalMem = 256;

    hr = CUtilGPU::GetActiveDisplayDeviceParameters(m_hwnVideoWindow, sDeviceEnv);
    if (FAILED(hr))
    {
        DbgMsg("CRenderEngine::CreateD3DDevice: fail to QueryActiveDisplayDeviceParameters.");
        return hr;
    }
    SetRegistry(REG_VENDOR_ID, sDeviceEnv.dwVendorId);
    SetRegistry(REG_DEVICE_ID, sDeviceEnv.dwDeviceId);
    SetRegistry(REG_LOCAL_VIDMEM, sDeviceEnv.dwTotalLocalMem);
	SetRegistry(REG_DISPLAY_XVYCC_MONITOR_TYPE, DISPLAY_XVYCC_MONITOR_NOT_SUPPORT);
	SetRegistry(REG_DISPLAY_XVYCC_GAMUT, FALSE);

    try
    {
        // release interfaces belong to the old device.
        ReleaseD3DDevice();

        if (m_dwConfigFlags & DISPSVR_USE_D3D9EX)
        {
            hr = CreateD3DDevice9Ex(uDeviceCreateFlag, D3DFormatAlternatives, &sDeviceEnv);
            if(FAILED(hr))
                m_dwConfigFlags &= ~DISPSVR_USE_D3D9EX;
        }

        if (!m_pDevice)
            hr = CreateD3DDevice9(uDeviceCreateFlag, D3DFormatAlternatives, &sDeviceEnv);

        hr = m_pDevice ? S_OK : hr;
        CHECK_HR(hr, DbgMsg("CRenderEngine::CreateD3DDevice: failed to create device, error code 0x%08x", hr));

        // recalculate transformation matrix again in case of backbuffer size change.
        if (m_bUseRTV)
        {
            CalculateTransformToRtv(m_matrixWorld, m_rectSrc, m_rectWindow, m_sizeBackBuffer, m_d3dDisplayMode, m_bRTVFromOrigin);
        }

        // We should have decided back buffer size by now.
        SetRegistry(REG_BACKBUFFER_WIDTH, m_sizeBackBuffer.cx);
        SetRegistry(REG_BACKBUFFER_HEIGHT, m_sizeBackBuffer.cy);
        SetRegistry(REG_DISPLAY_WIDTH, m_d3dDisplayMode.Width);
        SetRegistry(REG_DISPLAY_HEIGHT, m_d3dDisplayMode.Height);
        SetRegistry(REG_DISPLAY_REFRESH_RATE, m_d3dDisplayMode.RefreshRate);

        hr = ConfigureResource();
        CHECK_HR(hr, DbgMsg("CRenderEngine::CreateD3DDevice: failed in ConfigureResource"));

        // maximum ambient light
        hr = m_pDevice->SetRenderState(D3DRS_AMBIENT, RGB(255, 255, 255));
        CHECK_HR(hr, DbgMsg("CRenderEngine::CreateD3DDevice: failed in SetRenderState(D3DRS_AMBIENT...)"));

        // lighting disabled
        hr = m_pDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
        CHECK_HR(hr, DbgMsg("CRenderEngine::CreateD3DDevice: failed in SetRenderState(D3DRS_LIGHTING...)"));

        // don't cull backside
        hr = m_pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
        CHECK_HR(hr, DbgMsg("CRenderEngine::CreateD3DDevice: failed in SetRenderState(D3DRS_CULLMODE...)"));

        // DISABLE depth buffering
        hr = m_pDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
        CHECK_HR(hr, DbgMsg("CRenderEngine::CreateD3DDevice: failed in SetRenderState(D3DRS_ZENABLE...)"));

        // enable dithering
        hr = m_pDevice->SetRenderState(D3DRS_DITHERENABLE, TRUE);
        CHECK_HR(hr, DbgMsg("CRenderEngine::CreateD3DDevice: failed in SetRenderState(D3DRS_DITHERENABLE...)"));

        // disable stencil
        hr = m_pDevice->SetRenderState(D3DRS_STENCILENABLE, FALSE);
        CHECK_HR(hr, DbgMsg("CRenderEngine::CreateD3DDevice: failed in SetRenderState(D3DRS_STENCILENABLE...)"));

        // manage blending
        hr = m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        CHECK_HR(hr, DbgMsg("CRenderEngine::CreateD3DDevice: failed in SetRenderState(D3DRS_ALPHABLENDENABLE...)"));

        hr = m_pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        CHECK_HR(hr, DbgMsg("CRenderEngine::CreateD3DDevice: failed in SetRenderState(D3DRS_SRCBLEND...)"));

        hr = m_pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        CHECK_HR(hr, DbgMsg("CRenderEngine::CreateD3DDevice: failed in SetRenderState(D3DRS_DESTBLEND...)"));

        hr = m_pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
        CHECK_HR(hr, DbgMsg("CRenderEngine::CreateD3DDevice: failed in SetRenderState(D3DRS_ALPHATESTENABLE...)"));

        hr = m_pDevice->SetRenderState(D3DRS_ALPHAREF, 0x10);
        CHECK_HR(hr, DbgMsg("CRenderEngine::CreateD3DDevice: failed in SetRenderState(D3DRS_ALPHAREF...)"));

        hr = m_pDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
        CHECK_HR(hr, DbgMsg("CRenderEngine::CreateD3DDevice: failed in SetRenderState(D3DRS_ALPHAFUNC...)"));

        // set up sampler

        hr = m_pDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_BORDER);
        CHECK_HR(hr, DbgMsg("CRenderEngine::CreateD3DDevice: failed in SetSamplerState(D3DSAMP_ADDRESSU...)"));

        hr = m_pDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_BORDER);
        CHECK_HR(hr, DbgMsg("CRenderEngine::CreateD3DDevice: failed in SetSamplerState(D3DSAMP_ADDRESSV...)"));

        hr = m_pDevice->SetSamplerState(0, D3DSAMP_BORDERCOLOR, 0x00000000);
        CHECK_HR(hr, DbgMsg("CRenderEngine::CreateD3DDevice: failed in SetSamplerState(D3DSAMP_BORDERCOLOR...)"));

        hr = m_pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
        CHECK_HR(hr, DbgMsg("CRenderEngine::CreateD3DDevice: failed in SetSamplerState(D3DSAMP_MAGFILTER...)"));

        hr = m_pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
        CHECK_HR(hr, DbgMsg("CRenderEngine::CreateD3DDevice: failed in SetSamplerState(D3DSAMP_MINFILTER...)"));

        // save the default render target so that we can restore it later (other guy may change it).
        hr = m_pDevice->GetRenderTarget(0, &m_pRenderTarget);
        CHECK_HR(hr, DbgMsg("CRenderEngine::CreateD3DDevice: failed to get render target"));
    }
    catch (HRESULT hrFailed)
    {
        // For BD+ DiscoveryRAM Tags, BD+ Content Code would check the pattern at runtime.
        DATA_TAG_15[13] -= DATA_TAG_15[0];
        hr = hrFailed;
    }

    if (m_pHJDetect)
    {
        HJDETDESC HJDD;
        ZeroMemory(&HJDD, sizeof(HJDETDESC));
        HJDD.Targets = HJ_TARGET_D3D9;
        HJDD.hWnd = m_hwnVideoWindow;
        HJDD.pInterface = (LPDWORD)m_pDevice.p;
        m_pHJDetect->SetDesc(&HJDD);
    }

    m_bFirstTimeDisp = true;
    return hr;
}

HRESULT CRenderEngine::ReleaseD3DDevice()
{
    m_bUseD3D9Overlay = false;
    if (CResourceManager::GetInstance())
        CResourceManager::GetInstance()->ProcessMessage(RESOURCE_MESSAGE_RELEASEDEVICE, 0);
    m_pRenderTarget.Release();
    m_pDeviceEx.Release();

    if (m_pDevice)
    {
        IDirect3DDevice9 *pDevice = m_pDevice.Detach();
        int ref = pDevice->Release();
        ASSERT(ref == 0);	// this is a fatal error.
    }
    return S_OK;
}

HRESULT CRenderEngine::ConfigureResource()
{
    GUID guidMixer, guidPresenter;

    switch (GetRegInt(_T("DispSvrMixer"), 1))
    {
    case 0: // To avoid auto select in while not using Mixer.
    case 2: // D3D
        guidMixer = DISPSVR_RESOURCE_D3DVIDEOMIXER;
        break;
    default: // Auto Select
        guidMixer = GUID_NULL;
        break;
    }

    if (m_bUseD3D9Overlay)
    {
        guidPresenter = DISPSVR_RESOURCE_D3DOVERALYPRESENTER;
    }
    else
    {
        switch (GetRegInt(_T("DispSvrPresenter"), (m_dwConfigFlags&DISPSVR_USE_CUSTOMIZED_OUTPUT) ? 0 : 2))
        {
        case 2: // Default D3D presenter
            guidPresenter = DISPSVR_RESOURCE_D3DVIDEOPRESENTER;
            break;
        case 3: // AMD SORT/EAPI -> For debug purpose
            guidPresenter = DISPSVR_RESOURCE_AMDSORTVIDEOPRESENTER;
            break;
        default: // Auto select
            guidPresenter = GUID_NULL;
            break;
        }
    }

    return SetPreferredMixerPresenter(guidMixer, guidPresenter);
}

HRESULT CRenderEngine::SetPreferredMixerPresenter(GUID &guidMixer, GUID& guidPresenter)
{
    HRESULT hr;

    hr = CResourceManager::GetInstance()->SetPreferredResource(guidMixer, RESOURCE_TYPE_VIDEOMIXER);
    if (FAILED(hr))
    {
        DbgMsg("CRenderEngine::SetPreferredMixerPresenter: fail to set preferred resource.");
        return hr;
    }

    hr = CResourceManager::GetInstance()->SetPreferredResource(guidPresenter, RESOURCE_TYPE_VIDEOPRESENTER);
    if (FAILED(hr))
    {
        DbgMsg("CRenderEngine::SetPreferredMixerPresenter: fail to set preferred resource.");
        return hr;
    }

    hr = CResourceManager::GetInstance()->ProcessMessage(RESOURCE_MESSAGE_SETDEVICE, m_pDevice);
    if (FAILED(hr))
    {
        DbgMsg("CRenderEngine::SetPreferredMixerPresenter: unable to set D3D device to resource manager.");
        return hr;
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
    presenterCaps.VideoDecodeCaps = VIDEO_CAP_FORMAT_1080 | VIDEO_CAP_CODEC_MPEG2;
    hr = m_pDispSvrVideoPresenter->QueryCaps(&presenterCaps);
    if (FAILED(hr))
    {
        DbgMsg("CRenderEngine::SetPreferredMixerPresenter: unable to query video presenter caps.");
        return hr;
    }

    MixerCaps mixerCaps = {0};
    hr = pDispSvrVideoMixer->QueryCaps(&mixerCaps);

    MixerProperty prop;
    hr = pDispSvrVideoMixer->GetProperty(&prop);
    if (SUCCEEDED(hr))
    {
        prop.dwFlags &= ~MIXER_PROPERTY_VIRTUALIZATION;
        prop.dwFlags &= ~MIXER_PROPERTY_VIRTUALIZE_FROM_ORIGIN;
		m_bUseRTV = false;
        m_bRTVFromOrigin = false;

        if (m_dwConfigFlags & DISPSVR_USE_RT_VIRTUALIZATION)
        {
			D3DPRESENT_PARAMETERS sPresentParam;
			CComPtr<IDirect3DSwapChain9> pSwapChain;

			hr = m_pDevice->GetSwapChain(0, &pSwapChain);
			// in flip operation, such as FlipEx or exclusive mode, we do not need virtualization.
			if (FAILED(hr) || FAILED(pSwapChain->GetPresentParameters(&sPresentParam))
				|| sPresentParam.SwapEffect != D3DSWAPEFFECT_FLIPEX || sPresentParam.Windowed == FALSE)
			{
	            prop.dwFlags |= MIXER_PROPERTY_VIRTUALIZATION;
				m_bUseRTV = true;
				if (presenterCaps.bCanVirtualizeFromOrigin)
				{
					if (mixerCaps.dwFlags & MIXER_CAP_CAN_VIRTUALIZE_FROM_ORIGIN)
					{
						prop.dwFlags |= MIXER_PROPERTY_VIRTUALIZE_FROM_ORIGIN;
						m_bRTVFromOrigin = true;
					}
				}
			}
        }

        prop.dwFlags |= MIXER_PROPERTY_CLEARNONVIDEOAREA;

        hr = pDispSvrVideoMixer->SetProperty(&prop);
    }

    PresenterProperty PresenterProp = {0};
    PresenterProp.dwSize = sizeof(PresenterProperty);
    hr = m_pDispSvrVideoPresenter->GetProperty(&PresenterProp);
    if (SUCCEEDED(hr))
    {
        if (m_dwConfigFlags & DISPSVR_WAITING_FOR_VSYNC)
            PresenterProp.dwFlags |= PRESENTER_PROPERTY_WAITUNTILPRESENTABLE;
        else
            PresenterProp.dwFlags &= ~PRESENTER_PROPERTY_WAITUNTILPRESENTABLE;

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
	m_bIsD3D9RenderTarget = (mixerCaps.dwFlags & MIXER_CAP_3D_RENDERTARGET) != 0;

    return hr;
}

HRESULT CRenderEngine::CheckDeviceState()
{
    HRESULT hr = D3DERR_DEVICELOST;

    if (m_pDeviceEx)
    {
        // We do not consider occlusion here so that NULL is passed to CheckDeviceState.
        // If a window handle is passed in, CheckDeviceState can report occlusion but not mode switch.
        hr = m_pDeviceEx->CheckDeviceState(m_hwnVideoWindow);

        // When using 3rd party overlay solutions, d3d Present/PresentEx is not called so
        // that CheckDeviceState does not return correct state.
        // Use GetAdapterDisplayModeEx to workaround this issue.
        if (SUCCEEDED(hr) && hr != S_PRESENT_MODE_CHANGED && hr != S_PRESENT_OCCLUDED && m_pD3D9Ex && m_bIsOverlayPresenter)
        {
            hr = CheckModeSwitch();
        }

        // We treat mode switch as device lost to reset swap chains by the current display size
        // or format and also reset color key when changing color depth in overlay mode.
        if (hr == S_PRESENT_MODE_CHANGED && m_eDeviceStatus == eDeviceReady)
        {
            hr = D3DERR_DEVICELOST;
        }
    }
    else
    {
        if (m_pDevice)
            hr = m_pDevice->TestCooperativeLevel();
    }
    return hr;
}

HRESULT CRenderEngine::CheckModeSwitch()
{
    if (!m_bCheckDisplayChange)
        return S_OK;

    D3DDISPLAYMODEEX DispModeEx = {0};
    UINT nAdapter = D3DADAPTER_DEFAULT;
    D3DDEVTYPE type = D3DDEVTYPE_HAL;
    D3DDISPLAYROTATION Rotation = D3DDISPLAYROTATION_IDENTITY;
    // must set the size or GetAdapterDisplayModeEx will fail.
    DispModeEx.Size = sizeof(D3DDISPLAYMODEEX);
    HMONITOR hMonitor = MonitorFromWindow(m_hwnVideoWindow, (m_hwnVideoWindow) ? MONITOR_DEFAULTTONEAREST : MONITOR_DEFAULTTOPRIMARY);

    HRESULT hr = GetAdapterEx(&nAdapter, &type, hMonitor);
    if (FAILED(hr))
    {
        DbgMsg("CRenderEngine::CheckModeSwitch: failed to get the correct Adapter, we will use the default value.");
    }

    hr = m_pD3D9Ex->GetAdapterDisplayModeEx(nAdapter, &DispModeEx, &Rotation);
    if (SUCCEEDED(hr))
    {
        m_bCheckDisplayChange = FALSE;
        // 3 situations should set present mode changed:
        // a. color depth changed, 
        // b. current display mode is lower than new resolution
        // c. Monitor handle has hanged 
        if(m_d3dDisplayMode.Format != DispModeEx.Format ||
            m_d3dDisplayMode.Width != DispModeEx.Width ||
            m_d3dDisplayMode.Height != DispModeEx.Height || 
            m_hMonitor != hMonitor)
        {
            hr = S_PRESENT_MODE_CHANGED;
        }
    }
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
        DISP_AUTO_PROFILE(dog::CAT_VIDEO, dogsub::CAT_VIDEO_DISPSVR_1ST_FRAME_PREPARE_BEGIN);
        // only main video is granted to render and present.
        // If no rendering thread, MainVideotime will not be expired.
        // So we'll accept request at anytime.
        if (m_RenderThreadStatus != eRunning || 
            SUCCEEDED(UpdateMainVideoTime(pObject)))
            hr = RenderScene();
        NotifyDisplayEvent(DISPLAY_EVENT_MainVideoDecoded, param1, param2);
        DISP_AUTO_PROFILE(dog::CAT_VIDEO, dogsub::CAT_VIDEO_DISPSVR_1ST_FRAME_PREPARE_END);
        break;
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
    if (!phwnd)
    {
        DbgMsg("CRenderEngine::GetMessageWindow: received NULL pointer");
        return E_POINTER;
    }
    if (FALSE == m_bInitialized)
    {
        DbgMsg("CRenderEngine::GetMessageWindow: object is not initialized");
        return VFW_E_WRONG_STATE;
    }

    *phwnd = m_hwndMessage;

    return S_OK;
}

STDMETHODIMP CRenderEngine::SetMessageWindow(HWND hwnd)
{
    if (FALSE == IsWindow(hwnd))
    {
        DbgMsg("CRenderEngine::SetMessageWindow: received invalid window handle");
        return E_INVALIDARG;
    }
    if (FALSE == m_bInitialized)
    {
        DbgMsg("CRenderEngine::SetMessageWindow: object is not initialized");
        return VFW_E_WRONG_STATE;
    }

    CAutoDisplayLock displayLock(m_pLock);

    m_hwndMessage = hwnd;

    return S_OK;
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

                GUID *pGUID = (GUID *)pPropData;
                CComPtr<IDispSvrVideoMixer> pVideoMixer;
                QueryInterface(__uuidof(IDispSvrVideoMixer), (VOID **)&pVideoMixer);
                if (pVideoMixer)
                {
                    CComQIPtr<IDispSvrPlugin> pPlugIn = pVideoMixer;
                    if (pPlugIn)
                    {
                        if (SUCCEEDED(pPlugIn->GetResourceId(pGUID)))
                        {
                            (*pcbReturned) = sizeof(GUID);
                            return S_OK;
                        }
                        return E_UNEXPECTED;
                    }
                    else
                    {
                        return E_UNEXPECTED;
                    }
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

                GUID *pGUID = (GUID *)pPropData;
                CComPtr<IDispSvrVideoPresenter> pVideoPresenter;
                QueryInterface( __uuidof(IDispSvrVideoPresenter), (VOID **)&pVideoPresenter);
                if (pVideoPresenter)
                {
                    CComQIPtr<IDispSvrPlugin> pPlugIn = pVideoPresenter;
                    if (pPlugIn)
                    {
                        if (SUCCEEDED(pPlugIn->GetResourceId(pGUID)))
                        {
                            (*pcbReturned) = sizeof(GUID);
                            return S_OK;
                        }
                        return E_UNEXPECTED;
                    }
                    else
                    {
                        return E_UNEXPECTED;
                    }
                }

                return E_UNEXPECTED;
            }
        default :
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
                m_eDeviceStatus = eDeviceLost;
                NotifyRenderEngineEvent(DISPSVR_RENDER_ENGINE_DEVICE_LOST_DETECTED, 0, 0);

                // tell all the layers that we are lost. so that they would release all the
                // allocated video resources
                if (m_pRootDO)
                    m_pRootDO->BeginDeviceLoss();
                if (m_pOwner)
                    hr = m_pOwner->BeginDeviceLoss();
            }
            else if (m_eDeviceStatus == eDeviceLost)
            {
                // test if device is recoverable and then post message.
                if (!m_pDevice || CheckDeviceState() != D3DERR_DEVICELOST)
                {
                    m_eDeviceStatus = eDeviceRecoverable;
                    NotifyRenderEngineEvent(DISPSVR_RENDER_ENGINE_DEVICE_LOST, 0, 0);
                }
            }
            else if (m_eDeviceStatus == eDeviceChanging)
            {
                if (!m_bUseMessageThread)
                {
                    hr = ProcessLostDevice();
                }
            }

            // only render when main video time is not updated.
            if (CRenderClock::GetInstance()->Expired())
            {
                //For nVidia optimized overlay
				if (m_dwColorFillTimes <= _BACK_BUFFER_COUNT_)
	                m_dwColorFillTimes++;
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

HRESULT CRenderEngine::SetCustomizedOutput(BOOL bFlag)
{
    CAutoDisplayLock displayLock(m_pLock);
    BOOL IsCustomizedOutput = (m_dwConfigFlags & DISPSVR_USE_CUSTOMIZED_OUTPUT) ? TRUE : FALSE;
    if (IsCustomizedOutput == bFlag)
        return S_OK;

	DWORD dwVendor = GetRegistry(REG_VENDOR_ID, 0);
	DWORD dwOSVersion = GetRegistry(REG_OS_VERSION, 0);
    
	// for WinXP and Intel card and use Customized Output(BD case)
	if(dwVendor == PCI_VENDOR_ID_INTEL && dwOSVersion <= OS_XP && bFlag == TRUE)
		return E_FAIL;

    if (bFlag)
        m_dwConfigFlags |= DISPSVR_USE_CUSTOMIZED_OUTPUT;
    else
        m_dwConfigFlags &= ~DISPSVR_USE_CUSTOMIZED_OUTPUT;

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

HRESULT CRenderEngine::GetAdapterEx(UINT *pAdapter, D3DDEVTYPE *pType, HMONITOR hMon)
{
    CHECK_POINTER(pAdapter);
    CHECK_POINTER(pType);

    IDirect3D9 *pD3D9 = NULL;
    HRESULT hr = S_OK;

    if (m_pD3D9Ex)
        hr = m_pD3D9Ex->QueryInterface(IID_IDirect3D9, (void**)&pD3D9);
    else if(m_pD3D9)
        hr = m_pD3D9->QueryInterface(IID_IDirect3D9, (void**)&pD3D9);
    else
        hr = E_FAIL;

    if(SUCCEEDED(hr) && pD3D9)
    {
        for (UINT Adapter = 0; Adapter < pD3D9->GetAdapterCount(); Adapter++)
        {
            HMONITOR hmAdaptor = pD3D9->GetAdapterMonitor(Adapter);
            if (hmAdaptor == hMon)
                *pAdapter = Adapter;
#ifdef _DEBUG
            // Get adapter of nVidia NVPerfHUD
            D3DADAPTER_IDENTIFIER9 Identifier;
            hr = pD3D9->GetAdapterIdentifier(Adapter,0,&Identifier);		
            if (strcmp(Identifier.Description,"NVIDIA NVPerfHUD") == 0)
            {
                *pAdapter = Adapter;
                *pType = D3DDEVTYPE_REF;
                DbgMsg("CRenderEngine::CreateD3DDevice9Ex: %s attached!", Identifier.Description);
                break;
            }
#endif
        }
    }

    SAFE_RELEASE(pD3D9);
    return hr;
}