#include "stdafx.h"
#include <mfapi.h>
#include <MFerror.h>
#include <deque>
#include "ServerStateEventSink.h"
#include "VideoSourceEvr.h"

using namespace DispSvr;

//#define VSE_DP(fmt, ...)	DbgMsg("CVideoSourceEvr::" fmt, __VA_ARGS__)
#ifndef VSE_DP
#	define VSE_DP(...)	do {} while(0);
#endif

enum {
    DEBUG_EVR_DROP_LATE_SAMPLE = 1 << 0
};

// At 2X, 4X, we may receive samples at 120Hz, 240Hz
// It does not make sense to display them all and some of the presenters can't handle
// display frequency so high. It is easier to drop the exceeding samples by not presenting
// all of the samples.

// (Duration - Threshold) must be lower than 4ms due to the duration might be 4ms while playing 240hz
//Therefore set 13ms for 60fps, and 30ms for 30fps
#define INITIATIVE_RENDERING_SAMPLE_DURATION_THRESHOLD_60FPS (130000LL)
#define INITIATIVE_RENDERING_SAMPLE_DURATION_THRESHOLD_30FPS  (300000LL)
#define VSYNC_DELAY_BOUND (333333LL) //use 10000000 / 30 vsync time, not 10000000 / 29.97

#define DISP_AUTO_PROFILE(id, subid) { if (m_bFirstTimeDisp) {AUTO_SCOPE_PROFILE(id, subid)}}

static inline BOOL IsYUVFormat(DWORD dwFormat)
{
    if (dwFormat == PLANE_FORMAT_NV12 || dwFormat == PLANE_FORMAT_NV24 ||
        dwFormat == PLANE_FORMAT_IMC3 || dwFormat == PLANE_FORMAT_YV12 ||
        dwFormat == PLANE_FORMAT_YUY2)
        return TRUE;
    else 
        return FALSE;
}

class CAutoLockEx {

    // make copy constructor and assignment operator inaccessible

    CAutoLockEx(const CAutoLockEx &refAutoLock);
    CAutoLockEx &operator=(const CAutoLockEx &refAutoLock);

protected:
	volatile BOOL m_bLocked;
    CCritSec * m_pLock;

public:
    CAutoLockEx(CCritSec * plock)
    {
        m_bLocked = FALSE;
        m_pLock = plock;
        if (m_pLock)
        {
            m_pLock->Lock();
            m_bLocked = TRUE;
        }
    };

    ~CAutoLockEx()
    {
        Unlock();
    };

    void Unlock()
    {
        if (m_pLock && m_bLocked)
        {
            m_bLocked = FALSE;
            m_pLock->Unlock();
        }
    }
};

class CVideoSamplePool
{
protected:
    // Internal representation of video sample in the sample pool
    struct CEvrVideoSample
    {
        CEvrVideoSample() : pSample(0), pTexture(0), pSurface(0), bFree(false) {}
        ~CEvrVideoSample() { Release(); }
        void Release()
        {
            if (pSample)
            {
                int ref = pSample->Release();
                ASSERT(ref == 0);
                pSample = 0;
            }

            if (pTexture)
            {
                int ref = pTexture->Release();
                ASSERT(ref == 0);
                pTexture = 0;
            }

            if (pSurface)
            {
                int ref = pSurface->Release();
                ASSERT(ref == 0);
                pSurface = 0;
            }
            bFree = false;
        }
        IMFSample *pSample;
        IDirect3DTexture9 *pTexture;
        IDirect3DSurface9 *pSurface;
        bool bFree;
    };

public:
    typedef std::vector<CEvrVideoSample> SampleList;
    explicit CVideoSamplePool(UINT n) : m_listSamples(n), m_bAlloc(false)
    {
    }

    ~CVideoSamplePool()
    {
        Clear();
    }

    HRESULT AllocateSamples(IDirect3DDevice9 *pDevice, UINT uWidth, UINT uHeight, DWORD dwFormat)
    {
        HRESULT hr = S_OK;
        CComPtr<IDirectXVideoAccelerationService> pAccelService;
        if (CDynLibManager::GetInstance()->pfnDXVA2CreateVideoService)
        {
            hr = CDynLibManager::GetInstance()->pfnDXVA2CreateVideoService(pDevice,
                __uuidof(IDirectXVideoAccelerationService), (VOID**)&pAccelService);
        }

        for (SampleList::iterator it = m_listSamples.begin(); it != m_listSamples.end(); ++it)
        {
            ASSERT(!it->pSample && !it->pTexture && !it->pSurface);
            D3DFORMAT d3dFormat = (D3DFORMAT)dwFormat;
            if (pAccelService && !(d3dFormat == D3DFMT_X8R8G8B8 || d3dFormat == D3DFMT_A8R8G8B8))
            {
                hr = pAccelService->CreateSurface(uWidth, uHeight, 0, d3dFormat, D3DPOOL_DEFAULT, 0, DXVA2_VideoProcessorRenderTarget, &it->pSurface, NULL);
                if (SUCCEEDED(hr))
                {
                    hr = CDynLibManager::GetInstance()->pfnMFCreateVideoSampleFromSurface(it->pSurface, &it->pSample);
                    if (FAILED(hr))
                        break;
                    it->bFree = TRUE;

                    continue;
                }
                it->pSurface = 0;
            }
            else if (d3dFormat == D3DFMT_X8R8G8B8 || d3dFormat == D3DFMT_A8R8G8B8)
            {
                CComPtr<IDirect3DSurface9> pSurface;
                hr = pDevice->CreateTexture(uWidth, uHeight, 1, D3DUSAGE_RENDERTARGET, d3dFormat, D3DPOOL_DEFAULT, &it->pTexture, NULL);
                if (FAILED(hr))
                    break;
                hr = it->pTexture->GetSurfaceLevel(0, &pSurface);
                if (FAILED(hr))
                    break;
                hr = CDynLibManager::GetInstance()->pfnMFCreateVideoSampleFromSurface(pSurface, &it->pSample);
                if (FAILED(hr))
                    break;
                it->bFree = TRUE;

                continue;
            }
            hr = E_FAIL;
            break;
        }

        if (SUCCEEDED(hr))
        {
            m_bAlloc = true;
        }
        return hr;
    }

    void Clear()
    {
        m_bAlloc = false;
        for (SampleList::iterator it = m_listSamples.begin(); it != m_listSamples.end(); ++it)
            it->Release();
    }

    HRESULT ReturnSample(const IMFSample *pSample)
    {
        for (SampleList::iterator it = m_listSamples.begin(); it != m_listSamples.end(); ++it)
            if (pSample == it->pSample)
            {
                ASSERT(it->bFree == false);
                it->bFree = true;
                return S_OK;
            }
            return E_UNEXPECTED;
    }

    HRESULT GetFreeSample(IMFSample **pSample)
    {
        *pSample = NULL;
        if (!m_bAlloc)
            return E_FAIL;

        for (SampleList::iterator it = m_listSamples.begin(); it != m_listSamples.end(); ++it)
        {
            if (it->bFree)
            {
                it->bFree = false;
                it->pSample->AddRef();
                *pSample = it->pSample;
                return S_OK;
            }
        }
        return MF_E_SAMPLEALLOCATOR_EMPTY;
    }

private:
    SampleList m_listSamples;
    bool m_bAlloc;
};

// Queue to store samples waiting for presentation.
class CPresentQueue
{
public:
    typedef std::deque<IMFSample *> SampleList;
    CPresentQueue() { }
    ~CPresentQueue() { Clear(); }

    void Clear()
    {
        while (!m_listSamples.empty())
        {
            IMFSample *pSample = m_listSamples.front();
            m_listSamples.pop_front();
            pSample->Release();
        }
    }

    HRESULT Push(IMFSample *pSample)
    {
        if (pSample == NULL)
            return E_POINTER;

        pSample->AddRef();
        m_listSamples.push_back(pSample);
        return S_OK;
    }

    void Pop()
    {
        if (!m_listSamples.empty())
        {
            m_listSamples.front()->Release();
            m_listSamples.pop_front();
        }
    }

    HRESULT PopFront(IMFSample **ppSample)
    {
        if (ppSample == NULL)
            return E_POINTER;

        if (!m_listSamples.empty())
        {
            *ppSample = m_listSamples.front();
            m_listSamples.pop_front();
            return S_OK;
        }
        *ppSample = NULL;
        return E_FAIL;
    }

    HRESULT PopBack(IMFSample **ppSample)
    {
        if (ppSample == NULL)
            return E_POINTER;

        if (!m_listSamples.empty())
        {
            *ppSample = m_listSamples.back();
            m_listSamples.pop_back();
            return S_OK;
        }
        *ppSample = NULL;
        return E_FAIL;
    }

    HRESULT Peek(IMFSample **ppSample)
    {
        if (!m_listSamples.empty())
        {
            *ppSample = m_listSamples.front();
            (*ppSample)->AddRef();
            return S_OK;
        }
        *ppSample = NULL;
        return E_FAIL;
    }

    bool Empty() const { return m_listSamples.empty(); }
    unsigned int Size() const { return m_listSamples.size(); }

private:
    SampleList m_listSamples;
};

CVideoSourceEvr::CVideoSourceEvr(IDisplayLock* pLock, IDisplayVideoSink* pVideoSink)
: CUnknown(NAME("CVideoSourceEvr"), NULL)
{
    m_lImageWidth = 0L;
    m_lImageHeight = 0L;

    m_pLock = pLock;
    m_pVideoSink = pVideoSink;

    m_bEndOfStreaming = true;
    m_uFlushPendingPresentQueue = 0;
    m_bSampleNotify = false;
    m_bValid = TRUE;
    m_bInitiativeDisplay = FALSE;
    m_nrcTexture.left = m_nrcTexture.top = 0.0;
    m_nrcTexture.right = m_nrcTexture.bottom = 1.0;
    m_uDeviceManagerToken = 0;
    m_eThreadStatus = eNotStarted;
    m_dwOutputStreamId = 0;
    m_hEvrPresentFlushEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    m_hEvrPresentEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    m_hDeviceHandle = NULL;
    m_pVideoSink->QueryInterface(__uuidof(IDisplayObject), (void **)&m_pDispObj);
    m_pStateEventSink = CServerStateEventSink::GetInstance();
    // it is advised to have queue number larger than 3
    m_uPresentQueueSize = 3; //GetRegInt(TEXT("DispSvrEvrQueueSize"), 3);
    m_pSamplePool = new CVideoSamplePool(m_uPresentQueueSize);
    m_pPresentQueue = new CPresentQueue;
    m_ePresenterState = PRESENTER_STATE_SHUTDOWN;
    m_uDebugFlags = 0;
    m_nInputFramesAfterFlush = 0;
    m_bFlushPresentQueue = false;
    m_hwndVideo	= 0;
    memset(&m_dstrect, 0, sizeof(m_dstrect));
    m_fNativeAspectRatio = 0.0;
    m_dwAspectRatioMode = MFVideoARMode_None;
    m_dwMaxDisplayFrameRate = 60;
    m_llMinDisplayDuration = 10000000LL/(m_dwMaxDisplayFrameRate-1);
    m_hnsLastPts = 0;
}

CVideoSourceEvr::~CVideoSourceEvr()
{
    Cleanup();
    CloseHandle(m_hEvrPresentFlushEvent);
    CloseHandle(m_hEvrPresentEvent);
    m_pDispObj.Release();
    delete m_pSamplePool;
    delete m_pPresentQueue;
}

void CVideoSourceEvr::Cleanup()
{
    StopEvrPresentThread();
    ReleaseServicePointers();

    BeginDeviceLoss();
    m_pMediaType.Release();
    m_pGraph.Release();
    m_uDeviceManagerToken = 0;
    m_hnsLastPts = 0;
    IBaseFilter *pVMR = m_pVMR.Detach();
    if (pVMR)
    {
        int ref = pVMR->Release();
        if (ref > 0)
        {
            VSE_DP("Cleanup: m_pVMR ref count = %d, should be 0.\n", ref);
        }
    }

    m_lImageWidth = m_lImageHeight = 0L;
}

STDMETHODIMP CVideoSourceEvr::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
    HRESULT hr = E_NOINTERFACE;
    *ppv = NULL;

    if (riid == __uuidof(IDisplayVideoSource))
    {
        hr = GetInterface((IDisplayVideoSource *) this, ppv);
    }
    else if (riid == __uuidof(IMFGetService))
    {
        hr = GetInterface((IMFGetService *) this, ppv);
    }
    else if (riid == __uuidof(IMFTopologyServiceLookupClient))
    {
        hr = GetInterface((IMFTopologyServiceLookupClient *) this, ppv);
    }
    else if (riid == __uuidof(IMFVideoDeviceID))
    {
        hr = GetInterface((IMFVideoDeviceID *) this, ppv);
    }
    else if (riid == __uuidof(IMFVideoPresenter))
    {
        hr = GetInterface((IMFVideoPresenter *) this, ppv);
    }
    else if (riid == __uuidof(IDisplayVideoSourcePropertyExtension))
    {
        hr = GetInterface((IDisplayVideoSourcePropertyExtension *) this, ppv);
    }
    else
    {
        hr = CUnknown::NonDelegatingQueryInterface(riid, ppv);
    }
    return hr;
}

//////////////////////////////////////////////////////////////////////////
// IDisplayVideoSource

STDMETHODIMP CVideoSourceEvr::GetGraph(IFilterGraph** ppGraph)
{
    if (!ppGraph)
    {
        DbgMsg("CVideoSourceEvr::GetGraph: ppGraph is NULL");
        return E_POINTER;
    }

    if (m_pGraph)
    {
        m_pGraph.CopyTo(ppGraph);
        return S_OK;
    }

    DbgMsg("CVideoSourceEvr::GetGraph: FATAL: contains NULL IFilterGraph pointer");
    return VFW_E_NOT_FOUND;
}

STDMETHODIMP CVideoSourceEvr::GetTexture(IUnknown** ppTexture, NORMALIZEDRECT* lpNormRect)
{
    if (!ppTexture)
        return E_POINTER;

    CComPtr<IMFSample> pSample;
    CAutoLock lockPresent(&m_csEvrPresenting);
    HRESULT hr = m_pPresentQueue->Peek(&pSample);

    if (SUCCEEDED(hr))
    {
        CComPtr<IMFMediaBuffer> pBuffer;
        CComPtr<IDirect3DSurface9> pSurface;

        hr = pSample->GetBufferByIndex(0, &pBuffer);
        if (SUCCEEDED(hr))
        {
            CComQIPtr<IMFGetService> pGetSurface = pBuffer;
            if (pGetSurface)
                hr = pGetSurface->GetService(MR_BUFFER_SERVICE, __uuidof(IDirect3DSurface9), (void **)&pSurface);
        }

        if (pSurface)
        {
            CComPtr<IDirect3DTexture9> pTexture;
            hr = pSurface->GetContainer(__uuidof(IDirect3DTexture9), (void**)&pTexture);
            if (FAILED(hr))
                hr = pSurface.CopyTo((IDirect3DSurface9**)ppTexture);
            else
                hr = pTexture.CopyTo((IDirect3DTexture9**)ppTexture);
        }
    }
    if (lpNormRect)
        *lpNormRect = m_nrcTexture;
    return hr;
}

STDMETHODIMP CVideoSourceEvr::BeginDraw()
{
    return S_OK;
}

STDMETHODIMP CVideoSourceEvr::EndDraw()
{
    return S_OK;
}

STDMETHODIMP CVideoSourceEvr::IsValid()
{
    return m_bValid ? S_OK : E_FAIL;
}

STDMETHODIMP CVideoSourceEvr::ClearImage()
{
    return S_OK;
}

STDMETHODIMP CVideoSourceEvr::GetVideoSize(LONG* plWidth, LONG* plHeight, float *pfAspectRatio)
{
    ASSERT(plWidth && plHeight && pfAspectRatio);

    if (m_dwAspectRatioMode == MFVideoARMode_PreservePicture)
        m_fAspectRatio = m_fNativeAspectRatio;
    else if (m_dwAspectRatioMode ==MFVideoARMode_None)
        m_fAspectRatio = 0.0;

    *plWidth = m_lImageWidth;
    *plHeight = m_lImageHeight;
    *pfAspectRatio = m_fAspectRatio;
    return S_OK;
}

HRESULT CVideoSourceEvr::BeginDeviceLoss()
{
    m_bValid = FALSE;

    CAutoLock lock(&m_csEvrProcessing);
    CAutoLock lockPresent(&m_csEvrPresenting);

    if (m_pDeviceManager && m_hDeviceHandle)
    {
        m_pDeviceManager->CloseDeviceHandle(m_hDeviceHandle);
        m_hDeviceHandle = NULL;
    }
    m_pPresentQueue->Clear();
    m_pSamplePool->Clear();
    return S_OK;
}

HRESULT CVideoSourceEvr::EndDeviceLoss(IUnknown* pDevice)
{
    CAutoLock lock(&m_csEvrProcessing);

    HRESULT hr = S_OK;
    if (m_pDeviceManager)
    {
        CComQIPtr<IDirect3DDevice9> pDevice9 = pDevice;
        hr = m_pDeviceManager->ResetDevice(pDevice9, m_uDeviceManagerToken);
        ASSERT(SUCCEEDED(hr));
        hr = m_pDeviceManager->OpenDeviceHandle(&m_hDeviceHandle);
        ASSERT(SUCCEEDED(hr));
    }
    if (m_pMediaEventSink)
        m_pMediaEventSink->Notify(EC_DISPLAY_CHANGED, 0, 0);
    return hr;
}

HRESULT CVideoSourceEvr::EnableInitiativeDisplay(BOOL bEnable)
{
    HRESULT hr = S_OK;
    m_bInitiativeDisplay = bEnable;
    return hr;
}

// Disconnects pins of VMR
HRESULT CVideoSourceEvr::DisconnectPins()
{
    HRESULT hr = S_OK;
    if (!m_pVMR)
    {
        return E_POINTER;
    }

    try
    {
        CComPtr<IEnumPins> pEnum;
        hr = m_pVMR->EnumPins(&pEnum);
        CHECK_HR(hr, DbgMsg("CVideoSourceEvr::DisconnectPins: failed to enumerate pins, hr = 0x%08x", hr));

        CComPtr<IPin> pPin;
        hr = pEnum->Next(1, &pPin, NULL);
        while (S_OK == hr && pPin)
        {
            hr = pPin->Disconnect();
            CHECK_HR(hr, DbgMsg("CVideoSourceEvr::DisconnectPins: failed to disconnect pin, hr = 0x%08x", hr));

            pPin.Release();
            hr = pEnum->Next(1, &pPin, NULL);
        }
    }
    catch (HRESULT hrFailed)
    {
        hr = hrFailed;
    }

    return hr;
}

HRESULT CVideoSourceEvr::Attach(IBaseFilter* pVMR)
{
    HRESULT hr = S_OK;
    FILTER_INFO fiVMR;
    ZeroMemory(&fiVMR, sizeof(fiVMR));

    try
    {
        // check that provided VMR is part of the graph
        m_pVMR = pVMR;
        hr = m_pVMR->QueryFilterInfo(&fiVMR);
        hr = (NULL == fiVMR.pGraph) ? E_FAIL : S_OK;
        CHECK_HR(hr, DbgMsg("CVideoSourceEvr::Attach: provided VMR was not added to the graph"));

        m_pGraph = fiVMR.pGraph;

        CComPtr<IMediaControl> pMediaControl;
        hr = m_pGraph->QueryInterface(__uuidof(IMediaControl), (void**) & pMediaControl);
        CHECK_HR(hr, DbgMsg("CVideoSourceEvr::Attach: cannot QI IMediaControl"));

        OAFilterState state;
        hr = pMediaControl->GetState(100, &state);
        CHECK_HR(hr, DbgMsg("CVideoSourceEvr::Attach: failed to get state of IMediaControl, hr = 0x%08x", hr));

        hr = (state != State_Stopped) ? VFW_E_NOT_STOPPED : S_OK;
        CHECK_HR(hr, DbgMsg("CVideoSourceEvr::Attach: graph is not stopped, state = %ld", state));

        CComPtr<IMFVideoRenderer> pEvr;
        hr = m_pVMR->QueryInterface(__uuidof(IMFVideoRenderer), (void **)&pEvr);
        CHECK_HR(hr, DbgMsg("CVideoSourceEvr::Attach: filter is not an EVR"));

        CAutoDisplayLock displayLock(m_pLock);

        CComPtr<IDirect3DDevice9> pDevice;
        hr = m_pVideoSink->Get3DDevice((IUnknown**)&pDevice);
        CHECK_HR(hr, DbgMsg("CVideoSource::Attach: failed to obtain Direct3D device from the video sink, hr = 0x%08x", hr));

        if (!CDynLibManager::GetInstance()->pfnDXVA2CreateDirect3DDeviceManager9)
            hr = E_FAIL;
        CHECK_HR(hr, DbgMsg("CVideoSource::Attach: failed to link evr.dll, mfplat.dll or dxva2.dll"));

        if (SUCCEEDED(hr))
            hr = CDynLibManager::GetInstance()->pfnDXVA2CreateDirect3DDeviceManager9(&m_uDeviceManagerToken, &m_pDeviceManager);
        if (SUCCEEDED(hr))
            hr = m_pDeviceManager->ResetDevice(pDevice, m_uDeviceManagerToken);
        CHECK_HR(hr, DbgMsg("CVideoSourceEvr::Attach: unable to create device manager"));
        hr = m_pDeviceManager->OpenDeviceHandle(&m_hDeviceHandle);
        CHECK_HR(hr, DbgMsg("CVideoSourceEvr::Attach: unable to open device handle from manager"));

        hr = StartEvrPresentThread();
        CHECK_HR(hr, DbgMsg("CVideoSourceEvr::Attach: unable to start present thread."));

        hr = pEvr->InitializeRenderer(NULL, this);
        CHECK_HR(hr, DbgMsg("CVideoSourceEvr::Attach: IMFVideoRenderer::InitializeRenderer failed. hr=0x%x", hr));

        hr = m_pVideoSink->OnVideoSourceAdded(this,	m_fAspectRatio);
        CHECK_HR(hr, DbgMsg("CVideoSourceEvr::Attach: failed in IDisplayVideoSink::AddVideoSource(), hr = 0x%08x", hr));

        m_pStateEventSink->Notify(SERVER_STATE_VIDEO_SOURCE_ADD, reinterpret_cast<DWORD> (this), 0);
    }
    catch (HRESULT hrFailed)
    {
        Detach();
        hr = hrFailed;
    }

    SAFE_RELEASE(fiVMR.pGraph);

    return hr;
}

HRESULT CVideoSourceEvr::Detach()
{
    HRESULT hr = S_OK;

    if (!m_pVideoSink)
    {
        DbgMsg("CVideoSourceEvr::Detach: FATAL IDisplayVideoSink pointer is NULL!");
        return E_UNEXPECTED;
    }

    if (!m_pGraph)
    {
        DbgMsg("CVideoSourceEvr::Detach: video source info does not contain pointer to IFilterGraph!");
        return VFW_E_NOT_FOUND;
    }

    try
    {
        CComPtr<IMediaControl> pMC;
        hr = m_pGraph->QueryInterface(__uuidof(IMediaControl), (void**) & pMC);
        CHECK_HR(hr, DbgMsg("CVideoSourceEvr::Detach: cannot QI IMediaControl of the graph, hr = 0x%08x", hr));

        OAFilterState state;
        hr = pMC->GetState(100, &state);
        CHECK_HR(hr, DbgMsg("CVideoSourceEvr::Detach: cannot obtain state from IMediaControl, hr = 0x%08x", hr));

        hr = (State_Stopped != state) ? VFW_E_NOT_STOPPED : S_OK;
        CHECK_HR(hr, DbgMsg("CVideoSourceEvr::Detach: correspondent graph was not stopped"));

        // we have to be thread safe only here when we actually mess up with shared data
        CAutoDisplayLock displayLock(m_pLock);

        hr = DisconnectPins();
        CHECK_HR(hr, DbgMsg("CVideoSourceEvr::Detach: FATAL, failed to disconnect pins of VMR"));

        Cleanup();
        m_pStateEventSink->Notify(SERVER_STATE_VIDEO_SOURCE_REMOVE, reinterpret_cast<DWORD> (this), 0);
        m_pVideoSink->OnVideoSourceRemoved(this);
    }
    catch (HRESULT hrFailed)
    {
        hr = hrFailed;
    }

    return hr;
}

//////////////////////////////////////////////////////////////////////////
// IMFGetService
STDMETHODIMP CVideoSourceEvr::GetService(REFGUID guidService, REFIID riid, LPVOID* ppv)
{
    HRESULT hr = E_NOINTERFACE;

    if (MR_VIDEO_RENDER_SERVICE == guidService)
    {
        if (riid == __uuidof(IDirect3DDeviceManager9))
        {
            hr = m_pDeviceManager->QueryInterface(__uuidof(IDirect3DDeviceManager9), ppv);
        }
        else if (riid == __uuidof(IMFVideoDisplayControl))
        {
            hr = GetInterface((IMFVideoDisplayControl *)this, ppv);
        }
        else if (riid == __uuidof(IDispSvrVideoMixer))
        {
            hr = m_pVideoSink->QueryInterface(riid, ppv);
        }
    }
    else if (MR_VIDEO_ACCELERATION_SERVICE == guidService)
    {
        if (riid == __uuidof(IDirect3DDeviceManager9))
        {
            hr = m_pDeviceManager->QueryInterface(__uuidof(IDirect3DDeviceManager9), ppv);
        }
    }
    ASSERT(SUCCEEDED(hr));	// alert if we miss handling certain services.
    return hr;
}

//////////////////////////////////////////////////////////////////////////
// IMFTopologyServiceLookupClient
STDMETHODIMP CVideoSourceEvr::InitServicePointers(IMFTopologyServiceLookup *pLookup)
{
    HRESULT hr = E_POINTER;
    if (pLookup)
    {
        MF_SERVICE_LOOKUP_TYPE eType = MF_SERVICE_LOOKUP_ALL;	// type is currently ignored
        DWORD nObjects = 1;		// 1 is the current implementation of LookupService

        CAutoLock lock(&m_csEvrProcessing);
        CAutoLock lock2(&m_csEvrPresenting);

        ASSERT(!m_pMediaEventSink);
        hr = pLookup->LookupService(eType, 0, MR_VIDEO_RENDER_SERVICE, __uuidof(IMediaEventSink), (LPVOID *)&m_pMediaEventSink, &nObjects);
        if (FAILED(hr))
            return hr;

        hr = pLookup->LookupService(eType, 0, MR_VIDEO_RENDER_SERVICE, __uuidof(m_pClock), (LPVOID *)&m_pClock, &nObjects);
        if (m_pClock)
        {
            DWORD dwFlags;
            hr = m_pClock->GetClockCharacteristics(&dwFlags);
            if (FAILED(hr) || (MFCLOCK_CHARACTERISTICS_FLAG_FREQUENCY_10MHZ & dwFlags) == 0)
            {
                MFCLOCK_PROPERTIES prop;
                ASSERT("GetClockCharacteristics returns clock which is not supported currently.");
                hr = m_pClock->GetProperties(&prop);
            }
        }

        hr = pLookup->LookupService(eType, 0, MR_VIDEO_MIXER_SERVICE, __uuidof(IMFTransform), (LPVOID *)&m_pMixer, &nObjects);
        return hr;
    }
    return hr;
}

// Here we should release all pointers acquired by InitServicePointers() call
STDMETHODIMP CVideoSourceEvr::ReleaseServicePointers()
{
    VSE_DP("ReleaseServicePointers\n");
    CAutoLock lock(&m_csEvrProcessing);
    CAutoLock lock2(&m_csEvrPresenting);
    m_pMixer.Release();
    m_pClock.Release();
    m_pMediaEventSink.Release();
    return S_OK;
}

//////////////////////////////////////////////////////////////////////////
// IMFVideoDeviceID
STDMETHODIMP CVideoSourceEvr::GetDeviceID(IID* pDeviceID)
{
    if (pDeviceID == NULL)
        return E_POINTER;
    *pDeviceID = __uuidof(IDirect3DDevice9);
    return S_OK;
}

// IMFVideoPresenter
STDMETHODIMP CVideoSourceEvr::GetCurrentMediaType(IMFVideoMediaType** ppMediaType)
{
    HRESULT hr = S_OK;

    CAutoLock lock(&m_csEvrProcessing);
    if (ppMediaType == NULL)
        return E_POINTER;
    if (m_pMediaType == NULL)
        return MF_E_NOT_INITIALIZED;
    hr = m_pMediaType.QueryInterface(ppMediaType);
    return hr;
}

STDMETHODIMP CVideoSourceEvr::ProcessMessage(MFVP_MESSAGE_TYPE eMessage, ULONG_PTR ulParam)
{
    HRESULT hr = S_FALSE;

    switch (eMessage)
    {
        // The EVR switched from stopped to paused. The presenter should allocate resources.
    case MFVP_MESSAGE_BEGINSTREAMING:
        VSE_DP("MFVP_MESSAGE_BEGINSTREAMING\n");
        m_bEndOfStreaming = false;
        m_nInputFramesAfterFlush = 0;
        break;

        // Cancels a frame step.
    case MFVP_MESSAGE_CANCELSTEP:
        VSE_DP("MFVP_MESSAGE_CANCELSTEP\n");
        // indicates a frame step is cancelled.
        hr = m_pMediaEventSink->Notify(EC_STEP_COMPLETE, TRUE, 0);
        break;

        // All input streams have ended.
    case MFVP_MESSAGE_ENDOFSTREAM:
        VSE_DP("MFVP_MESSAGE_ENDOFSTREAM\n");
        FlushUnprocessedSamplesInMixer();
        hr = m_pMediaEventSink->Notify(EC_COMPLETE, 0, 0);
        break;

        // The EVR switched from running or paused to stopped. The presenter should free resources.
    case MFVP_MESSAGE_ENDSTREAMING:
        FlushUnprocessedSamplesInMixer();
        FlushPendingSamplesForPresent(0);
        m_bEndOfStreaming = true;
        VSE_DP("MFVP_MESSAGE_ENDSTREAMING\n");
        break;

        // The presenter should discard any pending samples.
    case MFVP_MESSAGE_FLUSH:
        VSE_DP("MFVP_MESSAGE_FLUSH\n");
        FlushPendingSamplesForPresent(1);
        m_nInputFramesAfterFlush = 0;
        m_hnsLastPts = 0;
        break;

        // The mixer's output format has changed. The EVR will initiate format negotiation, as described previously.
    case MFVP_MESSAGE_INVALIDATEMEDIATYPE:
        VSE_DP("MFVP_MESSAGE_INVALIDATEMEDIATYPE\n");
        FlushUnprocessedSamplesInMixer();
        FlushPendingSamplesForPresent(0);
        hr = RenegotiateMediaType();
        break;

        // One input stream on the mixer has received a new sample.
    case MFVP_MESSAGE_PROCESSINPUTNOTIFY:
        VSE_DP("MFVP_MESSAGE_PROCESSINPUTNOTIFY\n");
        ProcessInputNotify();
        break;

        // Requests a frame step.
    case MFVP_MESSAGE_STEP:
        VSE_DP("MFVP_MESSAGE_STEP\n");
        hr = m_pMediaEventSink->Notify(EC_STEP_COMPLETE, TRUE, 0);
        break;

    default:
        VSE_DP("ProcessMessage unhandled 0x%x, ulParam=0x%x, process hr=0x%x\n", (int)eMessage, ulParam, hr);
        break;
    }

    return hr;
}

void CVideoSourceEvr::SetMediaType(IMFMediaType *pType)
{
    m_pMediaType = pType;
    if (pType)
    {
        CheckMaxDisplayFrameRate();
    }
}

HRESULT CVideoSourceEvr::RenegotiateMediaType()
{
    HRESULT hr;
    DWORD dwInputIDArraySize, dwOutputIDArraySize;
    DWORD *pdwInputIDs = 0, *pdwOutputIDs = 0;

    CAutoLock lock(&m_csEvrProcessing);
    CAutoLock lock2(&m_csEvrPresenting);
    m_pMediaType.Release();
    hr = m_pMixer->GetStreamCount(&dwInputIDArraySize, &dwOutputIDArraySize);
    if (SUCCEEDED(hr))
    {
        pdwInputIDs = new DWORD[dwInputIDArraySize];
        pdwOutputIDs = new DWORD[dwOutputIDArraySize];
        hr = m_pMixer->GetStreamIDs(dwInputIDArraySize, pdwInputIDs, dwOutputIDArraySize, pdwOutputIDs);
    }

    if (SUCCEEDED(hr))
    {
        BOOL bIsSORTPresenter = FALSE;
        GUID guidResID;
        hr = CResourceManager::GetInstance()->GetActiveResrouceGUID(DISPSVR_RESOURCE_VIDEOPRESENTER, &guidResID);
        if (SUCCEEDED(hr))
        {
            bIsSORTPresenter = IsEqualGUID(guidResID, DISPSVR_RESOURCE_AMDSORTVIDEOPRESENTER) ? TRUE : FALSE;
        }

        CComPtr<IMFMediaType> pType;
        hr = S_OK;
        UINT i = 0;
        int iRGBIdx = -1;
#ifdef _DEBUG
        DumpOutputAvailableType();
#endif
        while (hr != MF_E_NO_MORE_TYPES)
        {
            pType.Release();
            hr = m_pMixer->GetOutputAvailableType(pdwOutputIDs[0], i++, &pType);
            if (SUCCEEDED(hr))
            {
                GUID guidSubtype = GUID_NULL;
                if (SUCCEEDED(pType->GetGUID(MF_MT_SUBTYPE, &guidSubtype)))
                {
                    DWORD FourCC = guidSubtype.Data1;
                    if (FourCC == D3DFMT_X8R8G8B8 || FourCC == D3DFMT_A8R8G8B8)
                    {
                        iRGBIdx = i-1;
                    }
                    else if (IsYUVFormat(FourCC) && !(bIsSORTPresenter && FourCC == PLANE_FORMAT_NV12))
                    {
                        hr = m_pMixer->SetOutputType(pdwOutputIDs[0], pType, 0);
                        if (FAILED(hr))
                            continue;
                        m_dwOutputStreamId = pdwOutputIDs[0];
                        hr = PrepareOutputMediaSamples(pType);
                        if (FAILED(hr))
                            continue;
                        break;
                    }
                }
            }
        }

        if (!SUCCEEDED(hr) && iRGBIdx >= 0)
        {
            VSE_DP("Mixer doesn't support YUV format. use RGB format.\n");
            pType.Release();
            hr = m_pMixer->GetOutputAvailableType(pdwOutputIDs[0], iRGBIdx, &pType);
            if (SUCCEEDED(hr))
            {
                hr = m_pMixer->SetOutputType(pdwOutputIDs[0], pType, 0);
                if (SUCCEEDED(hr))
                {
                    m_dwOutputStreamId = pdwOutputIDs[0];
                    hr = PrepareOutputMediaSamples(pType);
                    if (FAILED(hr))
                        VSE_DP("ProcessMessage: MFVP_MESSAGE_INVALIDATEMEDIATYPE PrepareOutputMediaSample failed. hr=0x%x\n", hr);
                }
            }
        }

        if (SUCCEEDED(hr))
        {
            CComPtr<IMFMediaType> pCurrentType;
            hr = m_pMixer->GetOutputCurrentType(pdwOutputIDs[0], &pCurrentType);
            if (SUCCEEDED(hr))
            {
                SetMediaType(pCurrentType);
            }
        }

#ifdef _DEBUG
        if (SUCCEEDED(hr))
        {
            MFT_OUTPUT_STREAM_INFO StreamInfo;
            hr = m_pMixer->GetOutputStreamInfo(pdwOutputIDs[0], &StreamInfo);

            // presenter always needs to supply output media sample.
            ASSERT((StreamInfo.dwFlags & (MFT_OUTPUT_STREAM_PROVIDES_SAMPLES | MFT_OUTPUT_STREAM_CAN_PROVIDE_SAMPLES)) == 0);
        }
#endif	// _DEBUG
    }

    if (pdwInputIDs)
        delete [] pdwInputIDs;
    if (pdwOutputIDs)
        delete [] pdwOutputIDs;
    return hr;
}

//////////////////////////////////////////////////////////////////////////
// IMFClockStateSink
STDMETHODIMP CVideoSourceEvr::OnClockPause(MFTIME hnsSystemTime)
{
    VSE_DP("OnClockPause at %I64d\n", hnsSystemTime);
    ResetEvent(m_hEvrPresentEvent);
    m_ePresenterState = PRESENTER_STATE_PAUSED;
    m_hnsLastPts = 0;
    return S_OK;
}

STDMETHODIMP CVideoSourceEvr::OnClockRestart(MFTIME hnsSystemTime)
{
    VSE_DP("OnClockRestart at %I64d\n", hnsSystemTime);
    m_ePresenterState = PRESENTER_STATE_STARTED;

    // There may be some pending samples which are send before the clock is paused.
    m_hnsLastPts = 0;
    if (m_bSampleNotify)
        ProcessInputNotify();
    return S_OK;
}

STDMETHODIMP CVideoSourceEvr::OnClockSetRate(MFTIME hnsSystemTime, float flRate)
{
    VSE_DP("OnClockSetRate at %I64d, rate=%f\n", hnsSystemTime, flRate);
    return S_OK;
}

STDMETHODIMP CVideoSourceEvr::OnClockStart(MFTIME hnsSystemTime, LONGLONG llClockStartOffset)
{
    VSE_DP("OnClockStart at sys=%I64d, offset=%I64d\n", hnsSystemTime, llClockStartOffset);
    m_hnsLastPts = 0;
    m_ePresenterState = PRESENTER_STATE_STARTED;
    return S_OK;
}

STDMETHODIMP CVideoSourceEvr::OnClockStop(MFTIME hnsSystemTime)
{
    VSE_DP("OnClockStop at %I64d\n", hnsSystemTime);
    m_hnsLastPts = 0;
    m_ePresenterState = PRESENTER_STATE_STOPPED;
    return S_OK;
}

HRESULT CVideoSourceEvr::PrepareOutputMediaSamples(IMFMediaType *pType)
{
    CComPtr<IMFMediaBuffer> pBuffer;
    CComPtr<IDirect3DDevice9> pDevice;
    CComPtr<IDirect3DSurface9> pSurface;
    HRESULT hr = E_FAIL;
    UINT uWidth, uHeight;
    UINT uNumerator, uDenominator;
    FLOAT fAspectRatio = 0.0;
    DWORD dwFormat = 0;
    GUID guidSubtype = GUID_NULL;

    if (!pType)
        return E_POINTER;

    CAutoLock lock(&m_csEvrProcessing);
    hr = MFGetAttributeSize(pType, MF_MT_FRAME_SIZE, &uWidth, &uHeight);
    if (FAILED(hr))
        return hr;

    if (!m_hDeviceHandle || uWidth == 0 || uHeight == 0)
        return E_FAIL;

    hr = m_pDeviceManager->LockDevice(m_hDeviceHandle, &pDevice, TRUE);
    if (SUCCEEDED(hr))
    {
        m_pSamplePool->Clear();
        hr = pType->GetGUID(MF_MT_SUBTYPE, &guidSubtype);
        if (FAILED(hr))
            return hr;

        dwFormat = guidSubtype.Data1;
        hr = m_pSamplePool->AllocateSamples(pDevice, uWidth, uHeight, dwFormat);
        if (FAILED(hr))
            m_pSamplePool->Clear();
        m_pDeviceManager->UnlockDevice(m_hDeviceHandle, TRUE);
    }

    if (FAILED(hr))
        return hr;

    m_lImageWidth = uWidth;
    m_lImageHeight = uHeight;

    hr = MFGetAttributeRatio(pType, MF_MT_PIXEL_ASPECT_RATIO, &uNumerator, &uDenominator);
    if (SUCCEEDED(hr) && uDenominator)
        fAspectRatio = FLOAT(m_lImageWidth * uNumerator) / FLOAT(m_lImageHeight * uDenominator);
    else
        fAspectRatio = 0.f;

    m_fAspectRatio = m_fNativeAspectRatio = fAspectRatio;
    return hr;
}

//////////////////////////////////////////////////////////////////////////
// EVR Processing thread
HRESULT CVideoSourceEvr::StartEvrPresentThread()
{
    StopEvrPresentThread();

    HANDLE hThread = NULL;
    UINT tid = 0;

    if (m_eThreadStatus == eNotStarted)
    {
        hThread = (HANDLE) _beginthreadex(NULL, NULL, EvrPresentThread, this, NULL, &tid);
        if (INVALID_HANDLE_VALUE == hThread)
        {
            DbgMsg("CVideoSourceEvr failed to create present output thread.");
            return E_UNEXPECTED;
        }
        m_eThreadStatus = eRunning;
        CloseHandle(hThread);
    }
    return S_OK;
}

HRESULT CVideoSourceEvr::StopEvrPresentThread()
{
    if (m_eThreadStatus == eRunning)
    {
        m_eThreadStatus = eWaitingToStop;
        SetEvent(m_hEvrPresentEvent);
        SetEvent(m_hEvrPresentFlushEvent);
        while (m_eThreadStatus != eFinished)
            Sleep(50);
        ResetEvent(m_hEvrPresentFlushEvent);
        ResetEvent(m_hEvrPresentEvent);
        m_eThreadStatus = eNotStarted;
    }

    // flush sample queue.
    m_pPresentQueue->Clear();
    return S_OK;
}

HRESULT CVideoSourceEvr::ProcessInputNotify()
{
    HRESULT hr = S_OK;

    m_bSampleNotify = true;

    if (!m_hDeviceHandle)
    {
        // doing nothing if device is Lost
        hr = S_OK;
    }
    else if (m_pMediaType == NULL)
    {
        // No media type yet.
        hr = MF_E_TRANSFORM_TYPE_NOT_SET;
    }
    else
    {
        while (S_OK == hr || MF_E_SAMPLEALLOCATOR_EMPTY == hr)
        {
            if (!m_bSampleNotify)
            {
                hr = MF_E_TRANSFORM_NEED_MORE_INPUT;
                break;
            }

            hr = ProcessOutput();

            // we don't want to pull too fast.
            Sleep(MF_E_SAMPLEALLOCATOR_EMPTY == hr ? 2 : 0);
        }
    }
    return hr;
}

// Process output from the mixer
HRESULT CVideoSourceEvr::ProcessOutput()
{
    DWORD dwStatus = 0;
    MFT_OUTPUT_DATA_BUFFER Sample = {0};
    MFT_OUTPUT_DATA_BUFFER *pSamples = &Sample;
    DWORD nSamples = 1;		// output stream from mixer should be 1.
    HRESULT hr = E_FAIL;
    CComPtr<IMFSample> pVideoSample;

    CAutoLock lock(&m_csEvrProcessing);
    if (!m_pMixer)
        return E_FAIL;

    // When clock is paused or stopped, we would want to display at least a sample after flush
    // and do not process the rest samples for FF and RF cases.
    if (m_ePresenterState != PRESENTER_STATE_STARTED && m_nInputFramesAfterFlush >= 1)
        return E_FAIL;

    hr = m_pSamplePool->GetFreeSample(&pVideoSample);
    if (FAILED(hr))
        return hr;

    Sample.pSample = pVideoSample;
    Sample.dwStreamID = m_dwOutputStreamId;
    Sample.dwStatus = 0;
    // Todo: fix this.
    // send processing latency is problematic for now.
#if 0
    if (m_pClock)
    {
        LONGLONG hnsT1 = 0, hnsT2 = 0;
        MFTIME hnsSys;
        HRESULT hrClock;

        hrClock = m_pClock->GetCorrelatedTime(0, &hnsT1, &hnsSys);
        hr = m_pMixer->ProcessOutput(0, nSamples, pSamples, &dwStatus);
        if (SUCCEEDED(hrClock))
            hrClock = m_pClock->GetCorrelatedTime(0, &hnsT2, &hnsSys);
        if (SUCCEEDED(hrClock))
        {
            hnsT2 -= hnsT1;
            if (hnsT2 > 0)
                hr = m_pMediaEventSink->Notify(EC_PROCESSING_LATENCY, (LONG_PTR)&hnsT2, 0);
        }
    }
    else
#endif
    {
        hr = m_pMixer->ProcessOutput(0, nSamples, pSamples, &dwStatus);
    }

    if (FAILED(hr))
    {
        m_pSamplePool->ReturnSample(pVideoSample);

        // Handle some known error codes from ProcessOutput.
        if (hr == MF_E_TRANSFORM_TYPE_NOT_SET)
        {
            // perhaps it is not a good idea to renegotiate a new media type in a seperated thread like here.
            //			hr = RenegotiateMediaType();
        }
        else if (hr == MF_E_TRANSFORM_STREAM_CHANGE)
        {
            // Dynamic media type change. Clear our media type.
            SetMediaType(NULL);
        }
        else if (hr == MF_E_TRANSFORM_NEED_MORE_INPUT)
        {
            // The mixer needs more input.
            m_bSampleNotify = false;
        }
    }
    else
    {
        CAutoLock lockPresent(&m_csEvrPresenting);
#if 0
        LONGLONG hnsPresent = 0;
        hr = pVideoSample->GetSampleTime(&hnsPresent);
        VSE_DP("Push to present queue, sample=%I64d\n", hnsPresent);
#endif
        m_pStateEventSink->Notify(SERVER_STATE_VIDEO_SOURCE_PROCESS_INPUT, reinterpret_cast<DWORD> (this), 0);
        m_pPresentQueue->Push(pVideoSample);
        SetEvent(m_hEvrPresentEvent);
        m_nInputFramesAfterFlush++;
    }

    // we are responsible for releasing events if any is returned.
    for (DWORD i = 0; i < nSamples; i++)
    {
        if (pSamples[i].pEvents)
        {
            pSamples[i].pEvents->Release();
            pSamples[i].pEvents = NULL;
        }
    }
    return hr;
}

//////////////////////////////////////////////////////////////////////////
// EVR present thread

UINT WINAPI CVideoSourceEvr::EvrPresentThread(LPVOID lpParameter)
{
    CVideoSourceEvr *pThis = static_cast<CVideoSourceEvr *> (lpParameter);
    ASSERT(pThis);
    pThis->PresentLoop();
    return 0;
}

void CVideoSourceEvr::PresentLoop()
{
    {
        // the lock serves as a barrier when the thread is created.
        CAutoLock lock(&m_csEvrPresenting);
        ASSERT(m_eThreadStatus == eRunning);
    }

    HRESULT hr = S_OK;
    LONGLONG hnsPresent, hnsNow, hnsDuration, hnsDelta;
    LONG lSleep;
    MFTIME hnsSystem;
    unsigned int uSize;
    void *pLastPresentedSample = NULL;
    m_hnsLastPts = 0;
    bool m_bFirstTimeDisp = true;

    while (m_eThreadStatus == eRunning)
    {
        if (WaitForSingleObject(m_hEvrPresentEvent, INFINITE) != WAIT_OBJECT_0)
            break;

        {
            CAutoLockEx lockPresent(&m_csEvrPresenting);
            CComPtr<IMFSample> pSample;
            hnsDelta = hnsDuration = hnsPresent = 0;
            hnsSystem = 0;
            lSleep = 0;

            if (m_bFlushPresentQueue)
            {
                while (m_pPresentQueue->Size() > m_uFlushPendingPresentQueue)
                {
                    m_pPresentQueue->PopBack(&pSample);
                    hr = m_pSamplePool->ReturnSample(pSample);
                    pSample.Release();
                }
                m_bFlushPresentQueue = false;
            }

            uSize = m_pPresentQueue->Size();
            if (uSize == 0)
            {
                ResetEvent(m_hEvrPresentEvent);
                continue;
            }

            hr = m_pPresentQueue->Peek(&pSample);
            ASSERT(pSample);

            // the sample has been presented before.
            if (pLastPresentedSample == pSample)
            {
                if (uSize <= 1)
                {
                    ResetEvent(m_hEvrPresentEvent);
                }
                else
                {
                    m_pPresentQueue->Pop();
                    lockPresent.Unlock();
                    CAutoLock lockProcessing(&m_csEvrProcessing);
                    hr = m_pSamplePool->ReturnSample(pSample);
                    pLastPresentedSample = 0;
                }
                continue;
            }

            hr = pSample->GetSampleTime(&hnsPresent);
            if (SUCCEEDED(hr) && m_pClock && m_ePresenterState == PRESENTER_STATE_STARTED)
            {
                hr = pSample->GetSampleDuration(&hnsDuration);
                hr = m_pClock->GetCorrelatedTime(0, &hnsNow, &hnsSystem);
                hnsDelta = hnsPresent - hnsNow;
                lSleep = DWORD(hnsDelta / 10000);
                VSE_DP("GetCorrelatedTime queue=%d, sleep=%d, diffpts=%I64d, diff=%I64d, sample=%I64d, clock=%I64d, duration=%I64d sys=%I64d\n",
                    m_pPresentQueue->Size(), lSleep, hnsPresent - m_hnsLastPts, hnsDelta, hnsPresent, hnsNow, hnsDuration, hnsSystem);
                if (IsOverDisplayFrameRate(hnsDuration) && IsOverDisplayFrameRate(hnsPresent - m_hnsLastPts))
                {
                    //Over Frame Rate, Set lastpresentedsaample but not call render/present
                    VSE_DP("Skip Frame by present time\n");
                    pLastPresentedSample = pSample;
                    continue;
                }
            }
            pLastPresentedSample = pSample;
        }

        DISP_AUTO_PROFILE(dog::CAT_VIDEO, dogsub::CAT_VIDEO_DISPSVR_DSHOW_1ST_PRESENTIMAGE_BEGIN);
        if (lSleep > 0)
        {
            m_hnsLastPts = hnsPresent;
            DWORD dwRenderDiff = timeGetTime();
            m_pDispObj->NotifyEvent(DISPLAY_EVENT_VideoSourceRender, uSize, m_uPresentQueueSize, this);
            dwRenderDiff = timeGetTime() - dwRenderDiff;
            lSleep -= dwRenderDiff;
            if (lSleep > 0)
                WaitForSingleObject(m_hEvrPresentFlushEvent, lSleep);
            m_pStateEventSink->Notify(SERVER_STATE_VIDEO_SOURCE_PROCESS_OUTPUT, reinterpret_cast<DWORD> (this), 0);
            m_pDispObj->NotifyEvent(DISPLAY_EVENT_VideoSourcePresent, 0, 0, this);
        }
        // display the sample immediately, otherwise we may drop samples.
        // one exception is when clock is not present, we should display anyways.
        else if (m_pClock && (DEBUG_EVR_DROP_LATE_SAMPLE & m_uDebugFlags) && hnsDelta < -hnsDuration)
        {
            m_pStateEventSink->Notify(SERVER_STATE_VIDEO_SOURCE_DROP_INPUT, reinterpret_cast<DWORD> (this), 0);
            Sleep(1);
            VSE_DP("Drop because of late = %I64d\n", hnsDelta);
        }
        else
        {
            if (IsOverDisplayFrameRate(hnsDuration) || IsSkipFrameForVideoSync(hnsDuration, hnsDelta))
            {
                //Skip.
                VSE_DP("Skip Frame...\n");
            }
            else
            {
                m_hnsLastPts = hnsPresent;
                m_pDispObj->NotifyEvent(DISPLAY_EVENT_VideoSourceRender, uSize, m_uPresentQueueSize, this);
                m_pStateEventSink->Notify(SERVER_STATE_VIDEO_SOURCE_PROCESS_OUTPUT, reinterpret_cast<DWORD> (this), 0);
                m_pDispObj->NotifyEvent(DISPLAY_EVENT_VideoSourcePresent, 0, 0, this);
            }
        }
        DISP_AUTO_PROFILE(dog::CAT_VIDEO, dogsub::CAT_VIDEO_DISPSVR_DSHOW_1ST_PRESENTIMAGE_END);
        m_bFirstTimeDisp = false;
    }

    m_eThreadStatus = eFinished;
}

void CVideoSourceEvr::FlushUnprocessedSamplesInMixer()
{
    HRESULT hr = S_OK;

    while (hr == S_OK || hr == MF_E_SAMPLEALLOCATOR_EMPTY)
    {
        hr = ProcessOutput();
        Sleep(0);
    }
}

void CVideoSourceEvr::FlushPendingSamplesForPresent(UINT uFlushToNumberFrame)
{
    if (m_eThreadStatus != eRunning)
        return;

    {
        CAutoLock lockPresent(&m_csEvrPresenting);
        m_bFlushPresentQueue = true;
        m_uFlushPendingPresentQueue = uFlushToNumberFrame;
    }

    // flush the present queue
    SetEvent(m_hEvrPresentEvent);
    SetEvent(m_hEvrPresentFlushEvent);
    while (m_bFlushPresentQueue)
        Sleep(2);
    ResetEvent(m_hEvrPresentFlushEvent);
    ResetEvent(m_hEvrPresentEvent);
}

//////////////////////////////////////////////////////////////////////////
// IMFVideoDisplayControl

STDMETHODIMP CVideoSourceEvr::SetVideoWindow(HWND hwndVideo)
{
    m_hwndVideo = hwndVideo;
    return S_OK;
}

STDMETHODIMP CVideoSourceEvr::RepaintVideo()
{
    CComQIPtr<IDisplayObject> pObj = m_pVideoSink;
    if (pObj)
    {
        pObj->NotifyEvent(DISPLAY_EVENT_VideoSourceRender, 0, 0, this);
        pObj->NotifyEvent(DISPLAY_EVENT_VideoSourcePresent, 0, 0, this);
    }
    return S_OK;
}

STDMETHODIMP CVideoSourceEvr::SetAspectRatioMode(DWORD dwAspectRatioMode)
{
    if (dwAspectRatioMode == MFVideoARMode_None || dwAspectRatioMode == MFVideoARMode_PreservePicture)
        m_dwAspectRatioMode = dwAspectRatioMode;
    else
        return E_NOTIMPL;

    return S_OK;
}

STDMETHODIMP CVideoSourceEvr::GetAspectRatioMode(DWORD*  pdwAspectRatioMode)
{
    ASSERT(pdwAspectRatioMode);
    *pdwAspectRatioMode = m_dwAspectRatioMode;
    return S_OK;
}

STDMETHODIMP CVideoSourceEvr::GetNativeVideoSize(SIZE* pszVideo, SIZE* pszARVideo)
{
    if (m_pMediaType)
    {
        if(pszVideo)
        {
            pszVideo->cx = m_lImageWidth;
            pszVideo->cy = m_lImageHeight;
        }
        if(pszARVideo)
        {
            GetIdealVideoSize(0, pszARVideo);
        }
    }
    else
    {
        //Clear if media type not exist.
        if (pszVideo)
        {
            ZeroMemory(pszVideo, sizeof(SIZE));
        }
        if (pszARVideo)
        {
            ZeroMemory(pszARVideo, sizeof(SIZE));
        }
    }
    return S_OK;
}

STDMETHODIMP CVideoSourceEvr::GetIdealVideoSize(SIZE* pszMin, SIZE* pszMax)
{

    if (pszMax)
    {
        if (m_fNativeAspectRatio)
            pszMax->cx = (INT)ceil((float)m_lImageHeight * m_fNativeAspectRatio);
        else
            pszMax->cx = m_lImageWidth;

        pszMax->cy = m_lImageHeight;
    }
    if (pszMin)
    {
        if (m_fNativeAspectRatio)
            pszMax->cx = (INT)ceil((float)m_lImageHeight * m_fNativeAspectRatio);
        else
            pszMax->cx = m_lImageWidth;

        pszMax->cy = m_lImageHeight;
    }
    return S_OK;
}

STDMETHODIMP CVideoSourceEvr::SetVideoPosition(const MFVideoNormalizedRect* pnrcSource, const LPRECT prcDest)
{
    // One parameter can be NULL, but not both.
    if (pnrcSource == NULL && prcDest == NULL)
    {
        return E_POINTER;
    }

    HRESULT hr = S_OK;

    // Validate the rectangles.
    if (pnrcSource)
    {
        // The source rectangle cannot be flipped.
        if ((pnrcSource->left > pnrcSource->right) ||
            (pnrcSource->top > pnrcSource->bottom))
        {
            return E_INVALIDARG;
        }

        // The source rectangle has range (0..1)
        if ((pnrcSource->left < 0) || (pnrcSource->right > 1) ||
            (pnrcSource->top < 0) || (pnrcSource->bottom > 1))
        {
            return E_INVALIDARG;
        }

        // Update the source rectangle. Source clipping is performed by the mixer.
        if (m_pMixer)
        {
            m_nrcTexture.left = pnrcSource->left;
            m_nrcTexture.top = pnrcSource->top;
            m_nrcTexture.right = pnrcSource->right;
            m_nrcTexture.bottom = pnrcSource->bottom;
        }

        if(m_pDispObj)
        {
            CComQIPtr<IDisplayProperties> pProp;
            if (SUCCEEDED(m_pDispObj->QueryInterface(__uuidof(IDisplayProperties), (void **)&pProp)))
            {
                NORMALIZEDRECT nrcZoom;
                nrcZoom.left = m_nrcTexture.left * 2 - 1.0f;
                nrcZoom.right = m_nrcTexture.right * 2 - 1.0f;
                nrcZoom.top = 1.0f - m_nrcTexture.top * 2;
                nrcZoom.bottom = 1.0f - m_nrcTexture.bottom * 2;
                pProp->SetZoom(&nrcZoom);
            }
        }
    }

    if (prcDest)
    {
        // The destination rectangle cannot be flipped.
        if ((prcDest->left > prcDest->right) ||
            (prcDest->top > prcDest->bottom))
        {
            return E_INVALIDARG;
        }

        // Update the destination rectangle.
        m_dstrect.left = prcDest->left;
        m_dstrect.top = prcDest->top;
        m_dstrect.right = prcDest->right;
        m_dstrect.bottom = prcDest->bottom;
    }

    return hr;
}

STDMETHODIMP CVideoSourceEvr::GetVideoPosition(MFVideoNormalizedRect* pnrcSource, LPRECT prcDest)
{
    if (pnrcSource)
    {
        pnrcSource->left = m_nrcTexture.left;
        pnrcSource->top = m_nrcTexture.top;
        pnrcSource->right = m_nrcTexture.right;
        pnrcSource->bottom = m_nrcTexture.bottom;
    }

    if (prcDest)
    {
        prcDest->left = m_dstrect.left;
        prcDest->top = m_dstrect.top;
        prcDest->right = m_dstrect.right;
        prcDest->bottom = m_dstrect.bottom;
    }

    return S_OK;
}

STDMETHODIMP CVideoSourceEvr::GetCurrentImage(BITMAPINFOHEADER* pBih, BYTE** pDib, DWORD* pcbDib, LONGLONG* pTimeStamp)
{
    CHECK_POINTER(pBih)
        CHECK_POINTER(pDib)
        CHECK_POINTER(pcbDib)
        if (pBih->biSize != sizeof(BITMAPINFOHEADER))
            return E_INVALIDARG;

    HRESULT hr = E_FAIL;
    CComPtr<IUnknown> pUnk;

    hr = GetTexture(&pUnk, NULL);

    if (FAILED(hr))
        return hr;

    CComQIPtr<IDirect3DTexture9> pTexture9 = pUnk;
    CComPtr<IDirect3DSurface9> pSurface9;

    if (pTexture9)
        hr = pTexture9->GetSurfaceLevel( 0, &pSurface9);
    else
        hr = pUnk->QueryInterface(__uuidof(IDirect3DSurface9), (void **)&pSurface9);

    if (FAILED(hr) || !pSurface9)
        return E_FAIL;

    D3DSURFACE_DESC desc;
    hr = pSurface9->GetDesc(&desc);

    CComPtr<IDirect3DDevice9> pDevice;
    hr = m_pVideoSink->Get3DDevice((IUnknown**)&pDevice);

    if (FAILED(hr) || !pDevice)
        return E_FAIL;

    RECT rectSrc = {0, 0, m_lImageWidth, m_lImageHeight},rectDest = {0};

    D3DTEXTUREFILTERTYPE TexFilterType = D3DTEXF_LINEAR;
    rectDest.bottom = m_lImageHeight;

    if (m_fNativeAspectRatio != 0)
        rectDest.right = (LONG)ceil((float)m_lImageHeight * m_fNativeAspectRatio);
    else
        rectDest.right = m_lImageWidth;

    UINT OutputWidth = rectDest.right, OutputHeight = rectDest.bottom;

    if (memcmp(&rectSrc,&rectDest, sizeof(RECT)) == 0)
        TexFilterType = D3DTEXF_POINT;

    unsigned int w, h;
    w = 2;
    while (w < OutputWidth)
        w <<= 1;

    h = 2;
    while (h < OutputHeight)
        h <<= 1;

    CComPtr<IDirect3DTexture9> pARGBTexture9;
    CComPtr<IDirect3DSurface9> pARGBSurface9;

    CComPtr<IDirect3DTexture9> pDestTexture9;
    CComPtr<IDirect3DSurface9> pDestSurface9;

    hr = pDevice->CreateTexture(w, h, 0, D3DUSAGE_RENDERTARGET, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &pARGBTexture9, NULL);
    hr = pDevice->CreateTexture(w, h, 0, D3DUSAGE_DYNAMIC, D3DFMT_X8R8G8B8, D3DPOOL_SYSTEMMEM, &pDestTexture9, NULL);
    if (!pARGBTexture9 && !pDestTexture9)
        return E_FAIL;

    hr = pARGBTexture9->GetSurfaceLevel( 0, &pARGBSurface9);
    hr = pDestTexture9->GetSurfaceLevel( 0, &pDestSurface9);
    if (!pARGBSurface9 && !pDestSurface9)
        return E_FAIL;

    hr = pDevice->StretchRect(pSurface9, &rectSrc, pARGBSurface9, &rectDest, TexFilterType);
    if (FAILED(hr))
        return hr;

    hr = pDevice->GetRenderTargetData(pARGBSurface9, pDestSurface9);

    //D3DXSaveSurfaceToFile("c:\\texture.bmp", D3DXIFF_BMP, pDestSurface9, NULL, &rectDest);

    UINT BmpheaderSize = sizeof(BITMAPINFOHEADER);
    UINT BufSize = (OutputWidth * OutputHeight * 4);
    *pDib = (BYTE *)CoTaskMemAlloc(BufSize);
    if ((*pDib) == NULL)
        return E_UNEXPECTED;

    pBih->biWidth = OutputWidth;
    pBih->biHeight = OutputHeight;
    pBih->biPlanes = 1;
    pBih->biBitCount = 32;
    pBih->biCompression = BI_RGB;
    pBih->biSizeImage = BufSize;
    pBih->biXPelsPerMeter = 0;
    pBih->biYPelsPerMeter = 0;
    pBih->biClrUsed = 0;
    pBih->biClrImportant = 0;

    D3DLOCKED_RECT LockedTex;
    hr = pDestSurface9->LockRect(&LockedTex, NULL, D3DLOCK_READONLY);
    if (SUCCEEDED(hr))
    {
        BYTE* pDestBuf = (*pDib);
        BYTE* pStartBuf = (BYTE *)LockedTex.pBits;
        BYTE* pSourceBuf = pStartBuf;

        for (UINT i = 1;i <= OutputHeight; i++)
        {
            pSourceBuf = pStartBuf + (LockedTex.Pitch * (OutputHeight - i));
            memcpy(pDestBuf, pSourceBuf, (OutputWidth * 4));
            pDestBuf += (OutputWidth * 4);
        }
        pDestSurface9->UnlockRect();

        *pcbDib = BufSize;
    }

    ////Save to File
    //{
    //	HANDLE fh;
    //	DWORD nWritten;
    //	BITMAPFILEHEADER bmpfileheader;
    //	ZeroMemory( &bmpfileheader , sizeof(BITMAPFILEHEADER));
    //	{
    //		bmpfileheader.bfType = ('M' << 8) | 'B';
    //		bmpfileheader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + BufSize;
    //		bmpfileheader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    //		fh = CreateFile("c:\\capture.bmp",
    //			GENERIC_WRITE, 0, NULL,
    //			CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    //		WriteFile(fh, &bmpfileheader, sizeof(bmpfileheader), &nWritten, NULL);
    //		WriteFile(fh, pBih, sizeof(BITMAPINFOHEADER), &nWritten, NULL);
    //		WriteFile(fh, *pDib, BufSize, &nWritten, NULL);
    //		CloseHandle(fh);
    //	}
    //}
    return hr;
}

void CVideoSourceEvr::DumpOutputAvailableType()
{
    if (m_pMixer)
    {
        CComPtr<IMFMediaType> pType;
        HRESULT hr = S_OK;
        GUID guidSubtype = GUID_NULL;
        UINT i = 0;
        while (hr != MF_E_NO_MORE_TYPES)
        {
            hr = m_pMixer->GetOutputAvailableType(0, i, &pType);
            if (SUCCEEDED(hr))
            {
                if (SUCCEEDED(pType->GetGUID(MF_MT_SUBTYPE, &guidSubtype)))
                {
                    if (guidSubtype.Data1 < 0x100) //D3D Type
                    {
                        BOOL bFoundType = FALSE;
                        CHAR str[20] = {0};
                        switch (guidSubtype.Data1)
                        {
                        case D3DFMT_X8R8G8B8:
                            strcpy_s(str, 20, "D3DFMT_X8R8G8B8");
                            bFoundType = TRUE;
                            break;
                        case D3DFMT_A8R8G8B8:
                            strcpy_s(str, 20, "D3DFMT_A8R8G8B8");
                            bFoundType = TRUE;
                            break;
                        case D3DFMT_R8G8B8:
                            strcpy_s(str, 20, "D3DFMT_R8G8B8");
                            bFoundType = TRUE;
                            break;
                        case D3DFMT_X1R5G5B5:
                            strcpy_s(str, 20, "D3DFMT_X1R5G5B5");
                            bFoundType = TRUE;
                            break;
                        case D3DFMT_R5G6B5:
                            strcpy_s(str, 20, "D3DFMT_R5G6B5");
                            bFoundType = TRUE;
                            break;
                        default:
                            break;
                        }
                        if (!bFoundType)
                        {
                            DbgMsg("EVR Mixer OutputType : Index=%d, Type = %d", i, guidSubtype.Data1);
                        }
                        else
                        {
                            DbgMsg("EVR Mixer OutputType : Index=%d, Type = %s", i, str);
                        }
                    }	
                    else
                    {
                        DbgMsg("EVR OutputType : Index=%d, Type = %c%c%c%c", i,
                            (guidSubtype.Data1 >>  0)  & 0xff,
                            (guidSubtype.Data1 >>  8)  & 0xff,
                            (guidSubtype.Data1 >> 16) & 0xff,
                            (guidSubtype.Data1 >> 24) & 0xff);
                    }
                }
                pType.Release();
            }

            i++;
        }
    }
}

HRESULT CVideoSourceEvr::CheckMaxDisplayFrameRate()
{
    HRESULT hr = E_FAIL;
    CComPtr<IDispSvrVideoPresenter> pDispSvrVideoPresenter;

    hr = CResourceManager::GetInstance()->GetInterface(DISPSVR_RESOURCE_VIDEOPRESENTER, __uuidof(IDispSvrVideoPresenter), (void**)&pDispSvrVideoPresenter);
    if (SUCCEEDED(hr))
    {
        PresenterCaps Caps = {0};
        Caps.dwSize = sizeof(PresenterCaps);

        Caps.VideoDecodeCaps = VIDEO_CAP_CODEC_MPEG2; //default value
        if (m_lImageHeight > 720) // 1920x1080 and 1440x1080
        {
            Caps.VideoDecodeCaps |= VIDEO_CAP_FORMAT_1080;
        }
        else if (m_lImageHeight > 576) //1280x720
        {
            Caps.VideoDecodeCaps |= VIDEO_CAP_FORMAT_720;
        }
        else if (m_lImageHeight > 480)
        {
            Caps.VideoDecodeCaps |= VIDEO_CAP_FORMAT_576;
        }
        else
        {
            Caps.VideoDecodeCaps |= VIDEO_CAP_FORMAT_480;
        }

        hr = pDispSvrVideoPresenter->QueryCaps(&Caps);
        if (SUCCEEDED(hr))
        {
            if (Caps.bIsOverlay)
            {
                m_dwMaxDisplayFrameRate = Caps.dwFPS;
            }
            else
            {
                BOOL bIsGeneralD3DMode = FALSE;
                GUID guidResID;
                hr = CResourceManager::GetInstance()->GetActiveResrouceGUID(DISPSVR_RESOURCE_VIDEOMIXER, &guidResID);
                if (SUCCEEDED(hr))
                {
                    if (IsEqualGUID(guidResID, DISPSVR_RESOURCE_INTELFASTCOMPVIDEOMIXER))
                        m_dwMaxDisplayFrameRate = 60;
                    else
                        m_dwMaxDisplayFrameRate = Caps.dwFPS;
                }

                /*
                CComPtr<IDispSvrPlugin> pPlugIn;
                BOOL bIsGeneralD3DMode = TRUE;
                hr = CResourceManager::GetInstance()->GetInterface(DISPSVR_RESOURCE_VIDEOMIXER, __uuidof(IDispSvrPlugin), (void**)&pPlugIn);
                if (SUCCEEDED(hr) && pPlugIn)
                {
                    GUID guidResID;
                    if (SUCCEEDED(pPlugIn->GetResourceId(&guidResID)))
                    {
                        if (IsEqualGUID(guidResID, DISPSVR_RESOURCE_INTELFASTCOMPVIDEOMIXER))
                        {
                            bIsGeneralD3DMode = FALSE;
                        }
                    }
                }
                
    			if (bIsGeneralD3DMode) // D3D mode, always set 60 fps since we cannot query max flip rate.
    				m_dwMaxDisplayFrameRate = 60;
    			else
	    			m_dwMaxDisplayFrameRate = Caps.dwFPS;
                */
			}
        }

		PresenterProperty PresenterProp = {0};
		PresenterProp.dwSize = sizeof(PresenterProperty);
		hr = pDispSvrVideoPresenter->GetProperty(&PresenterProp);
		if (SUCCEEDED(hr))
		{
			PresenterProp.dwFlags &= ~PRESENTER_PROPERTY_WAITUNTILPRESENTABLE;
			hr = pDispSvrVideoPresenter->SetProperty(&PresenterProp);
		}
    }

    m_llMinDisplayDuration = 10000000LL/(m_dwMaxDisplayFrameRate-1);

    return hr;
}

BOOL CVideoSourceEvr::IsOverDisplayFrameRate(LONGLONG hnsDuration)
{
    if (hnsDuration < 0)
        return FALSE;

    LONGLONG llThrethold = 0;
    if (m_dwMaxDisplayFrameRate == 30)
    {
        llThrethold = INITIATIVE_RENDERING_SAMPLE_DURATION_THRESHOLD_30FPS;
    }
    else
    {
        llThrethold = INITIATIVE_RENDERING_SAMPLE_DURATION_THRESHOLD_60FPS;
    }

    if (hnsDuration < llThrethold)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

BOOL CVideoSourceEvr::IsSkipFrameForVideoSync(LONGLONG hnsDuration, LONGLONG hnsDelta)
{
    if (hnsDelta >= 0)
        return FALSE;

    //allow one vsync delay to decrease frequency of frame skip.
    LONGLONG DelayDuration = hnsDuration > VSYNC_DELAY_BOUND ? hnsDuration : VSYNC_DELAY_BOUND;

    if ((hnsDuration < m_llMinDisplayDuration) && (hnsDelta < -DelayDuration))
        return TRUE;

    return FALSE;
}


STDMETHODIMP CVideoSourceEvr::GetSampleProperty(SampleProperty *pProp)
{
    if (!pProp)
        return E_POINTER;

    CComPtr<IMFSample> pSample;
    CAutoLock lockPresent(&m_csEvrPresenting);
    HRESULT hr = m_pPresentQueue->Peek(&pSample);

    if (SUCCEEDED(hr))
    {
        CComPtr<IMFMediaBuffer> pBuffer;

        hr = pSample->GetBufferByIndex(0, &pBuffer);
        if (SUCCEEDED(hr))
        {
            CComQIPtr<IMFGetService> pGetSurface = pBuffer;
            if (pGetSurface)
                hr = pGetSurface->GetService(MR_BUFFER_SERVICE, __uuidof(IDirect3DSurface9), (void **)&pProp->pSurface);
        }

        if (m_dwAspectRatioMode == MFVideoARMode_PreservePicture)
            m_fAspectRatio = m_fNativeAspectRatio;
        else if (m_dwAspectRatioMode ==MFVideoARMode_None)
            m_fAspectRatio = 0.0;

        pProp->fAspectRatio = m_fAspectRatio;
        pProp->uWidth = m_lImageWidth;
        pProp->uHeight = m_lImageHeight;
        pSample->GetSampleTime(&pProp->rtStart);
        pSample->GetSampleDuration(&pProp->rtEnd);
        pProp->rtEnd += pProp->rtStart;

        if (MFGetAttributeUINT32(pSample, MFSampleExtension_Interlaced, FALSE) != FALSE)
            pProp->dwFlags |= SAMPLE_FLAG_INTERLACED;

        if (MFGetAttributeUINT32(pSample, MFSampleExtension_Discontinuity, FALSE) != FALSE)
            pProp->dwFlags |= SAMPLE_FLAG_DISCONTINUITY;

        if (MFGetAttributeUINT32(pSample, MFSampleExtension_BottomFieldFirst, FALSE) != FALSE)
            pProp->dwFlags |= SAMPLE_FLAG_BOTTOMFIELDFIRST;

        if (MFGetAttributeUINT32(pSample, MFSampleExtension_RepeatFirstField, FALSE) != FALSE)
            pProp->dwFlags |= SAMPLE_FLAG_REPEATFIRSTFIELD;
    }

    return hr;
}