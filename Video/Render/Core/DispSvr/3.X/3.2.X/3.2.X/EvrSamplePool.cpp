#include "stdafx.h"
#include "EvrHelper.h"
#include "EVRSamplePool.h"

using namespace DispSvr;
//-----------------------------------------------------------------------------
// CEvrSamplePool class
//-----------------------------------------------------------------------------
#ifndef ESP_DP
//#define ESP_DP(fmt, ...)			DbgMsg("CEvrSamplePool::" fmt, __VA_ARGS__)
#define ESP_DP __noop
#endif
CEvrSamplePool::CEvrSamplePool(UINT nSampleSize) : m_nSampleSize(nSampleSize), m_bInitialized(FALSE), m_cPending(0)
{

}

CEvrSamplePool::~CEvrSamplePool()
{
}


//-----------------------------------------------------------------------------
// GetSample
//
// Gets a sample from the pool. If no samples are available, the method
// returns MF_E_SAMPLEALLOCATOR_EMPTY.
//-----------------------------------------------------------------------------

HRESULT CEvrSamplePool::GetSample(IMFSample **ppSample)
{
    CAutoLock lock(&m_lock);

    if (!m_bInitialized)
    {
        return MF_E_NOT_INITIALIZED;
    }

    if (m_VideoSampleQueue.IsEmpty())
    {
        return MF_E_SAMPLEALLOCATOR_EMPTY;
    }

    HRESULT hr = S_OK;
    IMFSample *pSample = NULL;

    // Get a sample from the allocated queue.

    // It doesn't matter if we pull them from the head or tail of the list,
    // but when we get it back, we want to re-insert it onto the opposite end.
    // (see ReturnSample)
	try
	{
		hr = m_VideoSampleQueue.RemoveFront(&pSample);
		if (FAILED(hr))
			throw;

		InterlockedIncrement(&m_cPending);
//		m_cPending++;

		ESP_DP("Pending = %d", m_cPending);
		// Give the sample to the caller.
		*ppSample = pSample;
		(*ppSample)->AddRef();
	}
	catch(...)
	{
		ESP_DP("GetSample Failed");
	}
    SAFE_RELEASE(pSample);
    return hr;
}

//-----------------------------------------------------------------------------
// ReturnSample
//
// Returns a sample to the pool.
//-----------------------------------------------------------------------------

HRESULT CEvrSamplePool::ReturnSample(IMFSample *pSample) 
{
    CAutoLock lock(&m_lock);

    if (!m_bInitialized)
    {
        return MF_E_NOT_INITIALIZED;
    }

    HRESULT hr = S_OK;

	hr = m_VideoSampleQueue.InsertBack(pSample);

	if (SUCCEEDED(hr))
	{
		InterlockedDecrement(&m_cPending);
	//    m_cPending--;
	}

	ESP_DP("Pending = %d", m_cPending);

    return hr;
}

//-----------------------------------------------------------------------------
// AreSamplesPending
//
// Returns TRUE if any samples are in use.
//-----------------------------------------------------------------------------

BOOL CEvrSamplePool::AreSamplesPending()
{
    CAutoLock lock(&m_lock);

    if (!m_bInitialized)
    {
        return FALSE;
    }

    return (m_cPending > 0);
}

//-----------------------------------------------------------------------------
// Initialize
//
// Initializes the whole pool with a external token
//-----------------------------------------------------------------------------
HRESULT CEvrSamplePool::Initialize(IDirect3DDevice9 *pDevice, UINT uWidth, UINT uHeight, UINT uToken, DWORD dwFormat) // combine default initialize and allocateSamples funciton
{
	CAutoLock lock(&m_lock);

	if (m_bInitialized)
	{
		return MF_E_INVALIDREQUEST;
	}

	CHECK_POINTER(pDevice)

		if (uWidth == 0 || uHeight == 0)
			return E_INVALIDARG;

	HRESULT hr = S_OK;

    CComPtr<IDirectXVideoAccelerationService> pAccelService;
    if (CDynLibManager::GetInstance()->pfnDXVA2CreateVideoService)
    {
        hr = CDynLibManager::GetInstance()->pfnDXVA2CreateVideoService(pDevice,
            __uuidof(IDirectXVideoAccelerationService), (VOID**)&pAccelService);
    }


	for (UINT i = 0; i < m_nSampleSize; i++)
	{
		IDirect3DTexture9 *pTexture = NULL;
		IDirect3DSurface9 *pSurface = NULL;
		IMFSample *pVideoSample = NULL;
        D3DFORMAT d3dFormat = (D3DFORMAT)dwFormat;

        if (pAccelService && !(d3dFormat == D3DFMT_X8R8G8B8 || d3dFormat == D3DFMT_A8R8G8B8))
        {
            hr = pAccelService->CreateSurface(uWidth, uHeight, 0, d3dFormat, D3DPOOL_DEFAULT, 0, DXVA2_VideoProcessorRenderTarget, &pSurface, NULL);
        }
        else 
        {
            if (d3dFormat != D3DFMT_X8R8G8B8 && d3dFormat != D3DFMT_A8R8G8B8)
            {
                d3dFormat = D3DFMT_X8R8G8B8;
            }
            hr = pDevice->CreateTexture(uWidth, uHeight, 1, D3DUSAGE_RENDERTARGET, d3dFormat, D3DPOOL_DEFAULT, &pTexture, NULL);
		if (FAILED(hr))
			break;

		hr = pTexture->GetSurfaceLevel(0, &pSurface);
		if (FAILED(hr))
			break;
       }

		hr = CDynLibManager::GetInstance()->pfnMFCreateVideoSampleFromSurface(pSurface, &pVideoSample);
		if (FAILED(hr))
			break;

		hr = pVideoSample->SetUINT32(MFSamplePresenter_SampleCounter, uToken);
		if (FAILED(hr))
			break;

		hr = m_VideoSampleQueue.InsertBack(pVideoSample);
		if (FAILED(hr))
			break;

		ULONG ref = 0;

		if (pVideoSample)
		{
			ref = pVideoSample->Release();
			pVideoSample = NULL;
		}
		if (pSurface)
		{
			ref = pSurface->Release();
			pSurface = NULL;
		}
		if (pTexture)
		{
			ref = pTexture->Release();
			pTexture = NULL;
		}
	}

	if (SUCCEEDED(hr))
		m_bInitialized = TRUE;

	return hr;
}

//-----------------------------------------------------------------------------
// Initialize
//
// Initializes the pool with a list of samples.
//-----------------------------------------------------------------------------

HRESULT CEvrSamplePool::Initialize(CEvrVideoSampleList& samples)
{
    CAutoLock lock(&m_lock);

    if (m_bInitialized)
    {
        return MF_E_INVALIDREQUEST;
    }

    HRESULT hr = S_OK;
    IMFSample *pSample = NULL;

    // Move these samples into our allocated queue.
    CEvrVideoSampleList::POSITION pos = samples.FrontPosition();

	try
	{
		while (pos != samples.EndPosition())
		{
			hr = samples.GetItemPos(pos, &pSample);
			if (FAILED(hr))
				throw;
			hr = m_VideoSampleQueue.InsertBack(pSample);
			if (FAILED(hr))
				throw;

			pos = samples.Next(pos);
			SAFE_RELEASE(pSample);
		}

		m_bInitialized = TRUE;
	}
	catch(...)
	{
		ESP_DP("Initialize Failed");
	}

    samples.Clear();

    SAFE_RELEASE(pSample);
    return hr;
}


//-----------------------------------------------------------------------------
// Clear
//
// Releases all samples.
//-----------------------------------------------------------------------------

HRESULT CEvrSamplePool::Clear()
{
    HRESULT hr = S_OK;

    CAutoLock lock(&m_lock);

    m_VideoSampleQueue.Clear();
    m_bInitialized = FALSE;
    m_cPending = 0;
    return S_OK;
}

HRESULT CEvrSamplePool::AllocateSamples(IDirect3DDevice9 *pDevice, UINT uWidth, UINT uHeight, CEvrVideoSampleList& videoSampleQueue)
{
	HRESULT hr = E_FAIL;
	CHECK_POINTER(pDevice)

	if (uWidth == 0 || uHeight == 0)
		return E_INVALIDARG;

	for (UINT i = 0; i < m_nSampleSize; i++)
	{
		IDirect3DTexture9 *pTexture = NULL;
		IDirect3DSurface9 *pSurface = NULL;
		IMFSample *pVideoSample = NULL;

		hr = pDevice->CreateTexture(uWidth, uHeight, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &pTexture, NULL);
		if (FAILED(hr))
			break;

		hr = pTexture->GetSurfaceLevel(0, &pSurface);
		if (FAILED(hr))
			break;

		hr = CDynLibManager::GetInstance()->pfnMFCreateVideoSampleFromSurface(pSurface, &pVideoSample);
		if (FAILED(hr))
			break;

		hr = videoSampleQueue.InsertBack(pVideoSample);
		if (FAILED(hr))
			break;

		ULONG ref = 0;

		if (pVideoSample)
		{
			ref = pVideoSample->Release();
			pVideoSample = NULL;
		}
		if (pSurface)
		{
			ref = pSurface->Release();
			pSurface = NULL;
		}
		if (pTexture)
		{
			ref = pTexture->Release();
			pTexture = NULL;
		}
	}

	return hr;
}

LONG CEvrSamplePool::GetSamplePendingCount()
{
	CAutoLock lock(&m_lock);

	if (!m_bInitialized)
	{
		return FALSE;
	}

	return m_cPending;
}
