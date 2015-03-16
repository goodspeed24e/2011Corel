#include <streams.h>
#include <d3d9.h>
#include <dxva2api.h>
#include <assert.h>
#include "Imports/ThirdParty/NVIDIA/NvAPI/nvdxvahdapi.h"

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p) if(p) { (p)->Release(); p = NULL; }
#endif  // SAFE_RELEASE

#ifndef SAFE_DELETE
#define SAFE_DELETE(p)	if(p) { delete p; p = NULL; }
#endif // SAFE_DELETE


// Because nvdxvahdapi.h defines a set of class definition which conflicts microsoft's interfaces,
// we use the same vtable to fake microsoft's interfaces while using nvdxvahdapi's implementation.

// Define the same vtable for IDXVAHD_Device.
MIDL_INTERFACE("95f12dfd-d77e-49be-815f-57d579634d6d")
__IDXVAHD_Device : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE CreateVideoSurface( 
        /* [annotation][in] */ 
        __in  UINT Width,
        /* [annotation][in] */ 
        __in  UINT Height,
        /* [annotation][in] */ 
        __in  D3DFORMAT Format,
        /* [annotation][in] */ 
        __in  D3DPOOL Pool,
        /* [annotation][in] */ 
        __in  DWORD Usage,
        /* [annotation][in] */ 
        __in  DXVAHD_SURFACE_TYPE Type,
        /* [annotation][in] */ 
        __in  UINT NumSurfaces,
        /* [annotation][size_is][out] */ 
        __out_ecount(NumSurfaces)  IDirect3DSurface9 **ppSurfaces,
        /* [annotation][out][in] */ 
        __inout_opt  HANDLE *pSharedHandle) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE GetVideoProcessorDeviceCaps( 
        /* [annotation][out] */ 
        __out  DXVAHD_VPDEVCAPS *pCaps) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE GetVideoProcessorOutputFormats( 
        /* [annotation][in] */ 
        __in  UINT Count,
        /* [annotation][size_is][out] */ 
        __out_ecount(Count)  D3DFORMAT *pFormats) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE GetVideoProcessorInputFormats( 
        /* [annotation][in] */ 
        __in  UINT Count,
        /* [annotation][size_is][out] */ 
        __out_ecount(Count)  D3DFORMAT *pFormats) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE GetVideoProcessorCaps( 
        /* [annotation][in] */ 
        __in  UINT Count,
        /* [annotation][size_is][out] */ 
        __out_ecount(Count)  DXVAHD_VPCAPS *pCaps) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE GetVideoProcessorCustomRates( 
        /* [annotation][in] */ 
        __in  const GUID *pVPGuid,
        /* [annotation][in] */ 
        __in  UINT Count,
        /* [annotation][size_is][out] */ 
        __out_ecount(Count)  DXVAHD_CUSTOM_RATE_DATA *pRates) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE GetVideoProcessorFilterRange( 
        /* [annotation][in] */ 
        __in  DXVAHD_FILTER Filter,
        /* [annotation][out] */ 
        __out  DXVAHD_FILTER_RANGE_DATA *pRange) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE CreateVideoProcessor( 
        /* [annotation][in] */ 
        __in  const GUID *pVPGuid,
        /* [annotation][out] */ 
        __deref_out  IDXVAHD_VideoProcessor **ppVideoProcessor) = 0;
    
};

MIDL_INTERFACE("95f4edf4-6e03-4cd7-be1b-3075d665aa52")
__IDXVAHD_VideoProcessor : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE SetVideoProcessBltState( 
        /* [annotation][in] */ 
        __in  DXVAHD_BLT_STATE State,
        /* [annotation][in] */ 
        __in  UINT DataSize,
        /* [annotation][in] */ 
        __in_bcount(DataSize)  const void *pData) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE GetVideoProcessBltState( 
        /* [annotation][in] */ 
        __in  DXVAHD_BLT_STATE State,
        /* [annotation][in] */ 
        __in  UINT DataSize,
        /* [annotation][out] */ 
        __inout_bcount(DataSize)  void *pData) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE SetVideoProcessStreamState( 
        /* [annotation][in] */ 
        __in  UINT StreamNumber,
        /* [annotation][in] */ 
        __in  DXVAHD_STREAM_STATE State,
        /* [annotation][in] */ 
        __in  UINT DataSize,
        /* [annotation][in] */ 
        __in_bcount(DataSize)  const void *pData) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE GetVideoProcessStreamState( 
        /* [annotation][in] */ 
        __in  UINT StreamNumber,
        /* [annotation][in] */ 
        __in  DXVAHD_STREAM_STATE State,
        /* [annotation][in] */ 
        __in  UINT DataSize,
        /* [annotation][out] */ 
        __inout_bcount(DataSize)  void *pData) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE VideoProcessBltHD( 
        /* [annotation][in] */ 
        __in  IDirect3DSurface9 *pOutputSurface,
        /* [annotation][in] */ 
        __in  UINT OutputFrame,
        /* [annotation][in] */ 
        __in  UINT StreamCount,
        /* [annotation][size_is][in] */ 
        __in_ecount(StreamCount)  const DXVAHD_STREAM_DATA *pStreams) = 0;
    
};

class DECLSPEC_NOVTABLE __CUnknown : public IUnknown
{
public:
	__CUnknown() : m_cRef(0) { }
	virtual ~__CUnknown() {}

	// IUnknown
	STDMETHOD(QueryInterface)(REFIID riid, void **ppv)
	{
		HRESULT hr = E_NOINTERFACE;

		if (riid == IID_IUnknown)
		{
			hr = GetInterface((IUnknown *)this, ppv);
		}
		return hr;
	}

	STDMETHOD_(ULONG, AddRef)()
	{
		LONG lRef = InterlockedIncrement(&m_cRef);
		assert(lRef > 0);
		return lRef;
	}

	STDMETHOD_(ULONG, Release)()
	{
		LONG lRef = InterlockedDecrement(&m_cRef);
		ASSERT(lRef >= 0);
		if (lRef == 0)
			delete this;
		return lRef;
	}

protected:
	LONG m_cRef;
};

class CDXVAHD_VideoProcessor : public __CUnknown, public __IDXVAHD_VideoProcessor
{
public:
	CDXVAHD_VideoProcessor(IDXVAHD_VideoProcessor *pVP) : m_pVP(pVP) {}
	~CDXVAHD_VideoProcessor()
	{
		SAFE_RELEASE(m_pVP->m_pVideodProcessDevice);
		SAFE_RELEASE(m_pVP->m_pSysMemControl);
		SAFE_DELETE(m_pVP);
	}

public:

	// IUnknown
	STDMETHOD(QueryInterface)(REFIID riid, void **ppv)
	{
		HRESULT hr = E_NOINTERFACE;

		if (riid == __uuidof(__IDXVAHD_VideoProcessor))
		{
			hr = GetInterface((__IDXVAHD_VideoProcessor *)this, ppv);
		}
		else
		{
			hr = __CUnknown::QueryInterface(riid, ppv);
		}
		return hr;
	}
	STDMETHOD_(ULONG, AddRef)() { return __CUnknown::AddRef(); }
	STDMETHOD_(ULONG, Release)() { return __CUnknown::Release(); }

	// IDXVAHD_VideoProcessor
	virtual HRESULT STDMETHODCALLTYPE SetVideoProcessBltState(DXVAHD_BLT_STATE State, UINT DataSize, const void *pData)
	{
		return GetVP()->SetVideoProcessBltState(State, DataSize, pData);
	}

	virtual HRESULT STDMETHODCALLTYPE GetVideoProcessBltState(DXVAHD_BLT_STATE State, UINT DataSize, void *pData)
	{
		return GetVP()->GetVideoProcessBltState(State, DataSize, pData);
	}

	virtual HRESULT STDMETHODCALLTYPE SetVideoProcessStreamState(UINT StreamNumber, DXVAHD_STREAM_STATE State, UINT DataSize, const void *pData)
	{
		return GetVP()->SetVideoProcessStreamState(StreamNumber, State, DataSize, pData);
	}

	virtual HRESULT STDMETHODCALLTYPE GetVideoProcessStreamState(UINT StreamNumber, DXVAHD_STREAM_STATE State, UINT DataSize, void *pData)
	{
		return GetVP()->GetVideoProcessStreamState(StreamNumber, State, DataSize, pData);
	}

	virtual HRESULT STDMETHODCALLTYPE VideoProcessBltHD(IDirect3DSurface9 *pOutputSurface, UINT OutputFrame, UINT StreamCount, const DXVAHD_STREAM_DATA *pStreams)
	{
		// Note, here is an API difference.
		return GetVP()->VideoProcessBltHD(pOutputSurface, OutputFrame, StreamCount, const_cast<DXVAHD_STREAM_DATA *> (pStreams));
	}

protected:
	IDXVAHD_VideoProcessor *GetVP() const { ASSERT(m_pVP); return m_pVP; }
	IDXVAHD_VideoProcessor *m_pVP;
};

class CDXVAHD_Device : public __CUnknown, public __IDXVAHD_Device
{
public:
	static HRESULT CreateDevice(
		IDirect3DDevice9Ex* pD3DDevice,
		const DXVAHD_CONTENT_DESC* pContentDesc,
		DXVAHD_DEVICE_USAGE Usage,
		void** ppDevice
    )
	{
		IDXVAHD_Device *pDevice = 0;
		// Note, here is an API difference.
		HRESULT hr = DXVAHD_CreateDevice(pD3DDevice, const_cast<DXVAHD_CONTENT_DESC *> (pContentDesc), Usage, NULL, &pDevice);
		if (SUCCEEDED(hr))
		{
			CDXVAHD_Device *pWrapper = new CDXVAHD_Device(pDevice);	ASSERT(pWrapper);
			hr = pWrapper->QueryInterface(__uuidof(__IDXVAHD_Device), ppDevice); ASSERT(SUCCEEDED(hr));
		}
		return hr;
	}

	CDXVAHD_Device(IDXVAHD_Device *pDevice) : m_pDevice(pDevice) {}
	~CDXVAHD_Device()
	{
		SAFE_RELEASE(m_pDevice->m_pAccelServices);
		SAFE_DELETE(m_pDevice);
	}

public:
	// IUnknown
	STDMETHOD(QueryInterface)(REFIID riid, void **ppv)
	{
		HRESULT hr = E_NOINTERFACE;

		if (riid == __uuidof(__IDXVAHD_Device))
		{
			hr = GetInterface((__IDXVAHD_Device *)this, ppv);
		}
		else
		{
			hr = __CUnknown::QueryInterface(riid, ppv);
		}
		return hr;
	}
	STDMETHOD_(ULONG, AddRef)() { return __CUnknown::AddRef(); }
	STDMETHOD_(ULONG, Release)() { return __CUnknown::Release(); }

	// IDXVAHD_Device
	virtual HRESULT STDMETHODCALLTYPE CreateVideoSurface(UINT Width, UINT Height, D3DFORMAT Format,
		D3DPOOL Pool, DWORD Usage, DXVAHD_SURFACE_TYPE Type, UINT NumSurfaces, IDirect3DSurface9 **ppSurfaces, HANDLE *pSharedHandle)
	{
		return GetDevice()->CreateVideoSurface(Width, Height, Format, Pool, Usage, Type, NumSurfaces, ppSurfaces, pSharedHandle);
	}

	virtual HRESULT STDMETHODCALLTYPE GetVideoProcessorDeviceCaps(DXVAHD_VPDEVCAPS *pCaps)
	{
		return GetDevice()->GetVideoProcessorDeviceCaps(pCaps);
	}

	virtual HRESULT STDMETHODCALLTYPE GetVideoProcessorOutputFormats(UINT Count, D3DFORMAT *pFormats)
	{
		return GetDevice()->GetVideoProcessorOutputFormats(Count, pFormats);
	}

	virtual HRESULT STDMETHODCALLTYPE GetVideoProcessorInputFormats(UINT Count, D3DFORMAT *pFormats)
	{
		return GetDevice()->GetVideoProcessorInputFormats(Count, pFormats);
	}

	virtual HRESULT STDMETHODCALLTYPE GetVideoProcessorCaps(UINT Count, DXVAHD_VPCAPS *pCaps)
	{
		return GetDevice()->GetVideoProcessorCaps(Count, pCaps);
	}

	virtual HRESULT STDMETHODCALLTYPE GetVideoProcessorCustomRates(const GUID *pVPGuid, UINT Count, DXVAHD_CUSTOM_RATE_DATA *pRates)
	{
		return GetDevice()->GetVideoProcessorCustomRates(pVPGuid, Count, pRates);
	}

	virtual HRESULT STDMETHODCALLTYPE GetVideoProcessorFilterRange(DXVAHD_FILTER Filter, DXVAHD_FILTER_RANGE_DATA *pRange)
	{
		return GetDevice()->GetVideoProcessorFilterRange(Filter, pRange);
	}

	virtual HRESULT STDMETHODCALLTYPE CreateVideoProcessor(const GUID *pVPGuid, IDXVAHD_VideoProcessor **ppVideoProcessor)
	{
		IDXVAHD_VideoProcessor *pVP = 0;
		// Note, here is an API difference.
		HRESULT hr = GetDevice()->CreateVideoProcessor(*pVPGuid, &pVP);
		if (SUCCEEDED(hr))
		{
			CDXVAHD_VideoProcessor *pWrapper = new CDXVAHD_VideoProcessor(pVP);	ASSERT(pWrapper);
			hr = pWrapper->QueryInterface(__uuidof(__IDXVAHD_VideoProcessor), (void **)ppVideoProcessor); ASSERT(SUCCEEDED(hr));
		}
		return hr;
	}

protected:
	IDXVAHD_Device *GetDevice() const { ASSERT(m_pDevice); return m_pDevice; }
	IDXVAHD_Device *m_pDevice;
};


extern "C" HRESULT WINAPI
NV_DXVAHD_CreateDevice(IDirect3DDevice9Ex* pD3DDevice, const DXVAHD_CONTENT_DESC* pContentDesc, DXVAHD_DEVICE_USAGE Usage, void** ppDevice)
{
	return CDXVAHD_Device::CreateDevice(pD3DDevice, pContentDesc, Usage, ppDevice);
}
