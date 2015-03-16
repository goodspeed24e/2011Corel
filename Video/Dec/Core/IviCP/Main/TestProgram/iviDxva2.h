#pragma once

interface IMFGetService;
interface IDirectXVideoDecoderService;
interface IDirectXVideoProcessorService;
interface IDirect3DDevice9;
interface IDirect3DDeviceManager9;
interface IDirect3DSurface9;
interface IDirectXVideoMemoryConfiguration;
interface IDirectXVideoDecoder;
interface IDirectXVideoProcessor;
interface IDirectXVideoAccelerationService;

enum _D3DFORMAT;
enum _D3DPOOL;
struct _DXVA2_ConfigPictureDecode;
struct _DXVA2_VideoDesc;
struct _DXVA2_VideoProcessorCaps;

typedef HRESULT  (__stdcall *TpIviFnDXVA2CreateVideoService)(
    IDirect3DDevice9* pDD,
    REFIID riid,
    void** ppService
);


class CiviDxva2
{
private:
	volatile LONG m_cRef;       /* Number of reference counts */
	HMODULE m_hDxva2Lib;
protected:
	IDirect3DDevice9*			m_pD3DDev9; // stand-alone usage - IVI solely controls the device
	IDirect3DDeviceManager9*	m_pD3DDev9Man; // shared usage - Dshow or MF.
	IDirect3DSurface9**			m_ppRenderTarget;
	HANDLE*						m_aEvent;
	const bool					m_bRenderTargetSynch;
	UINT						m_nWidth, m_nHeight;
	HANDLE						m_hD3DDevice;
	GUID						m_DeviceGuid;
	// keep surface creating param
	enum _D3DFORMAT				m_nFormat, m_nLastFormat;
	GUID						m_guidVideoSubType;
	GUID*						m_pguidDecGuids;
	WORD						m_nSurfAllocated, m_nLastSurfAllocated, m_nNumOfDecGuids;

	TpIviFnDXVA2CreateVideoService	pfnDXVA2CreateVideoService;

	virtual ~CiviDxva2(); // class can only be destroyed via Release();
	// allocate uncompressed surfaces
	STDMETHOD(AllocRenderTargets)(UINT nSurf, enum _D3DFORMAT n3DFormat) = 0;
	void CleanUpLastAllocInfo();
	void CleanUp(); // release all resources
public:
	CiviDxva2(bool bRenderTargetSynch = false);
	STDMETHOD_(ULONG,AddRef)(); // we only need ref count, not whole COM infrastructure
	STDMETHOD_(ULONG,Release)(); // at least so far

	//for XP compatibility we cannot link to dxva2.dll, so get API func at run-time
	STDMETHOD(LoadDxva2Lib)();
	STDMETHOD(SetRenderTargetInfo)(UINT  nWidth, UINT  nHeight, GUID* pVideoSubType = 0,  GUID* pDxvaGuid = 0, int iNumOfGuid=0);
	STDMETHOD(GetRenderTargetInfo)(UINT* pWidth, UINT* pHeight, GUID* pVideoSubType = 0);
	// get all needed interfaces (except creating accelerator device)
	STDMETHOD(GetVideoAccelService)() = 0;
	STDMETHOD_(void,ReleaseVideoAccelService)() = 0;

	STDMETHOD_(GUID,GetDeviceGuid)() const {return m_DeviceGuid;}
	STDMETHOD_(WORD,SurfacesAllocated)() const {return m_nSurfAllocated;}
	STDMETHOD_(const enum _D3DFORMAT*,GetRenderTargetFormat)() const {return &m_nFormat;}

	enum TSurfaceLock {NoLock, Wait, ReSet};
	STDMETHOD(GetSurface)(WORD index, IDirect3DSurface9** ppD3DSurf9, TSurfaceLock lock = NoLock) const;
	STDMETHOD(ReleaseSurface)(WORD index) const;

	STDMETHOD(ReAllocRenderTargets)(WORD nExraTargets = 0);
	STDMETHOD(ReSize)(long nWidth, long nHeight);
	STDMETHODIMP_(void) ReleaseRenderTargets();

	STDMETHOD(BeginDeviceLoss)();
	STDMETHOD(EndDeviceLoss)(IDirect3DDevice9* pDevice);
};

class CiviDxva2Decoder : public CiviDxva2
{
private:
	IDirectXVideoDecoderService*		m_pVidDecService;
	IDirectXVideoMemoryConfiguration*	m_pDxVideoMemConfig;
protected:
	~CiviDxva2Decoder();
	CiviDxva2Decoder() {}
	STDMETHODIMP AllocRenderTargets(UINT nSurf, enum _D3DFORMAT n3DFormat);
public:
	CiviDxva2Decoder(IMFGetService* pMfGetService);
	CiviDxva2Decoder(IDirect3DDevice9* pD3DDev9);

	// configure video memory for dxva2
	STDMETHODIMP ConfigureVideoMemory();

	STDMETHODIMP AllocRenderTargets(ULONG nSurfMin, ULONG nSurfMax);
	STDMETHODIMP GetVideoAccelService();
	STDMETHODIMP_(void) ReleaseVideoAccelService();
	//merely wrappers around iriginal interface in order encapsulate other params
	STDMETHODIMP GetDecoderConfigurations(	const struct _DXVA2_VideoDesc *pVideoDesc,
					UINT *pCount, struct _DXVA2_ConfigPictureDecode **ppConfigs);
	STDMETHODIMP CreateVideoDecoder(		const struct _DXVA2_VideoDesc *pVideoDesc,
		const struct _DXVA2_ConfigPictureDecode *pConfig, IDirectXVideoDecoder **ppDecode);

	STDMETHODIMP RecoverFromD3DeviceChange();
	STDMETHOD(BeginDeviceLoss)();
	STDMETHOD(EndDeviceLoss)(IDirect3DDevice9* pDevice);

	STDMETHOD(GetDecoderDeviceSupportCount)(UINT* pnCount);
	STDMETHOD(GetDecoderDeviceSupportGUIDs)(GUID* pGUIDs);	
	STDMETHOD(GetDecoderDeviceGUIDs)(UINT* pnCount, GUID **ppGuids);
};

class CiviDxva2VidProc : public CiviDxva2
{
private:
	IDirectXVideoProcessorService *m_pVidProcService;

protected:
	CiviDxva2VidProc() {}
	~CiviDxva2VidProc();
	STDMETHOD(AllocRenderTargets)(UINT nSurf, enum _D3DFORMAT n3DFormat);
	STDMETHOD_(bool, IsPreferredVideoCaps)(const struct _DXVA2_VideoProcessorCaps &caps) const;

public:
	explicit CiviDxva2VidProc(IDirect3DDevice9 *pDevice9);
	STDMETHODIMP GetVideoAccelService();
	STDMETHODIMP_(void) ReleaseVideoAccelService();
	STDMETHOD(BeginDeviceLoss)();
	STDMETHOD(EndDeviceLoss)(IDirect3DDevice9* pDevice);
	STDMETHODIMP CreateVideoProcessor(const struct _DXVA2_VideoDesc *pVideoDesc, IDirectXVideoProcessor **ppVidProcess);
	STDMETHODIMP CreateVideoProcessor(const struct _DXVA2_VideoDesc *pVideoDesc, IDirectXVideoProcessor **ppVidProcess, const GUID guidDesired, UINT nSubStream);
	STDMETHODIMP CreateSurface(UINT width, UINT height, enum _D3DFORMAT format, enum _D3DPOOL Pool, IDirect3DSurface9 **ppSurface);
	bool IsRgbRenderTarget() const;
};
