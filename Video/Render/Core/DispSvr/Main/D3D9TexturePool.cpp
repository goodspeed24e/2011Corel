#include "stdafx.h"
#include "DispSvr.h"

#include <dxva2api.h>

#include "Exports/Inc/VideoMixer.h"
#include "MathVideoMixing.h"
#include "D3D9TexturePool.h"
#include "DynLibManager.h"

#include "Imports/LibGPU/GPUID.h"
#include "RegistryService.h"

using namespace DispSvr;

CD3D9TexturePool::CD3D9TexturePool()
{
	m_pDevice = 0;
	m_pAccelService = 0;
	m_bCreatePowerOf2Texture = false;
	m_uSID = 1;	// SID is always none zero
	m_ePoolUsage = TEXTURE_POOL_USAGE_D3D9;
	m_bDeferredSurfaceRelease = false;
}

CD3D9TexturePool::~CD3D9TexturePool()
{
	ASSERT(!m_pDevice);
	ASSERT(!m_pAccelService);
}

HRESULT CD3D9TexturePool::SetDevice(IDirect3DDevice9 *pDevice)
{
	HRESULT hr = E_FAIL;
	D3DCAPS9 deviceCaps;
	TextureCap textureCaps;
	CComPtr<IDirect3D9> pD3D9;

	m_TextureCap.clear();
	ASSERT(pDevice && m_pDevice == 0);
	ASSERT(m_Pool.empty());
	hr = pDevice->GetDirect3D(&pD3D9);
	if (FAILED(hr))
		return hr;

	hr = pDevice->GetDeviceCaps(&deviceCaps);
	if (FAILED(hr))
		return hr;

	m_bCreatePowerOf2Texture = (deviceCaps.TextureCaps & D3DPTEXTURECAPS_POW2) != 0
		&& (deviceCaps.TextureCaps & D3DPTEXTURECAPS_NONPOW2CONDITIONAL) == 0;

	ZeroMemory(&textureCaps, sizeof(textureCaps));
	textureCaps.uMaxWidth = deviceCaps.MaxTextureWidth;
	textureCaps.uMaxHeight = deviceCaps.MaxTextureHeight;
	textureCaps.bNativeTexture = true;
	textureCaps.d3dFormat = D3DFMT_A8R8G8B8;
	m_TextureCap[PLANE_FORMAT_ARGB] = textureCaps;

	const PLANE_FORMAT pFormatsToTry[] = { PLANE_FORMAT_NV12, PLANE_FORMAT_YV12, PLANE_FORMAT_YUY2, PLANE_FORMAT_IMC3, PLANE_FORMAT_NV24, PLANE_FORMAT_P8 };
	for (int i = 0; i < sizeof(pFormatsToTry) / sizeof(PLANE_FORMAT); i++)
	{
		textureCaps.d3dFormat = static_cast<D3DFORMAT> (pFormatsToTry[i]);

		hr = pD3D9->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, 0, D3DRTYPE_TEXTURE, textureCaps.d3dFormat);
		if (SUCCEEDED(hr))
		{
			m_TextureCap[pFormatsToTry[i]] = textureCaps;
			continue;
		}

		hr = pD3D9->CheckDeviceFormatConversion(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, textureCaps.d3dFormat, D3DFMT_A8R8G8B8);
		if (FAILED(hr))
			hr = pD3D9->CheckDeviceFormatConversion(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, textureCaps.d3dFormat, D3DFMT_X8R8G8B8);
		if (SUCCEEDED(hr))
		{
			m_TextureCap[pFormatsToTry[i]] = textureCaps;
			m_TextureCap[pFormatsToTry[i]].bNativeTexture = false;
		}
	}

	if (m_TextureCap.find(PLANE_FORMAT_P8) == m_TextureCap.end())
	{
		textureCaps.d3dFormat = D3DFMT_L8;
		hr = pD3D9->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, 0, D3DRTYPE_TEXTURE, D3DFMT_L8);
		if (SUCCEEDED(hr))
		{
			m_TextureCap[PLANE_FORMAT_P8] = textureCaps;
			m_TextureCap[PLANE_FORMAT_P8].bNativeTexture = false;
		}
	}

	m_pDevice = pDevice;
	m_pDevice->AddRef();

	if (CDynLibManager::GetInstance()->pfnDXVA2CreateVideoService)
	{
		hr = CDynLibManager::GetInstance()->pfnDXVA2CreateVideoService(m_pDevice,
			__uuidof(IDirectXVideoAccelerationService), (VOID**)&m_pAccelService);
	}

	DWORD dwOSVersion = 0;
	DWORD dwVendorID = 0;
	CRegistryService::GetInstance()->Get(REG_OS_VERSION, &dwOSVersion);
	CRegistryService::GetInstance()->Get(REG_VENDOR_ID, &dwVendorID);
	// Workaround for nvidia RGB overlay on XP.
	// The sympton is that overlay can't get updated after closing GPI. (eject CD or turn off/on DXVA)
	if (dwOSVersion <= OS_XP && dwVendorID == PCI_VENDOR_ID_NVIDIA)
		m_bDeferredSurfaceRelease = true;
	else
		m_bDeferredSurfaceRelease = false;

	return S_OK;
}

HRESULT CD3D9TexturePool::ReleaseDevice()
{
	ReclaimResources();

	for (TextureMap::iterator it = m_Pool.begin(); it != m_Pool.end(); ++it)
	{
		ReleaseTexture(it->second);
	}
	m_Pool.clear();

	SAFE_RELEASE(m_pAccelService);
	SAFE_RELEASE(m_pDevice);
	return S_OK;
}

HRESULT CD3D9TexturePool::CreateTexture(CreateTextureParam *pParam, HANDLE *pTexture)
{
	CHECK_POINTER(pParam);
	ASSERT(pTexture);

	ReclaimResources();

	// Pool usage can be overwritten by creation parameter
	TEXTURE_POOL_USAGE ePoolUsage = (pParam->eUsage & TEXTURE_USAGE_TEXTURE) ? TEXTURE_POOL_USAGE_D3D9 : m_ePoolUsage;
	HRESULT hr = E_INVALIDARG;
	TextureCap textureCap = {0};

	// render target surface can't have a backstore
	if (((TEXTURE_USAGE_RENDERTARGET | TEXTURE_USAGE_BACKSTORE) & pParam->eUsage) == (TEXTURE_USAGE_RENDERTARGET | TEXTURE_USAGE_BACKSTORE))
		return hr;

	if (FAILED(QueryFormatCaps(pParam->Format, &textureCap)))
		return hr;

	*pTexture = 0;
	D3D9Texture texture = {0};
	switch (pParam->Format)
	{
	case PLANE_FORMAT_ARGB:	//< 32 bit, A8R8G8B8
		// create texturable surface for D3D9 DrawPrimitive.
		if (ePoolUsage == TEXTURE_POOL_USAGE_D3D9)
		{
			if (m_bCreatePowerOf2Texture)
				CalTexturePowerOf2Size(pParam->uWidth, pParam->uHeight);

			if (pParam->eUsage & TEXTURE_USAGE_RENDERTARGET)
			{
				// A render target texture is used for VBlt mostly.
				hr = m_pDevice->CreateTexture(pParam->uWidth, pParam->uHeight, 1, D3DUSAGE_RENDERTARGET, textureCap.d3dFormat, D3DPOOL_DEFAULT, &texture.pTextureVidMem, NULL);
			}
			else
			{
				hr = m_pDevice->CreateTexture(pParam->uWidth, pParam->uHeight, 1, 0, textureCap.d3dFormat, D3DPOOL_DEFAULT, &texture.pTextureVidMem, NULL);
				if (SUCCEEDED(hr))
				{
					hr = CreateSurface(pParam->uWidth, pParam->uHeight, textureCap.d3dFormat, D3DPOOL_SYSTEMMEM, false, &texture.pSurfaceSysMem);
					pParam->eUsage = TEXTURE_USAGE_LOCKABLE_BACKSTORE;
				}
			}

			if (SUCCEEDED(hr))
				hr = texture.pTextureVidMem->GetSurfaceLevel(0, &texture.pSurfaceVidMem);
		}
		else
		{
			if (pParam->eUsage & TEXTURE_USAGE_RENDERTARGET)
			{
				hr = CreateSurface(pParam->uWidth, pParam->uHeight, textureCap.d3dFormat, D3DPOOL_DEFAULT, true, &texture.pSurfaceVidMem);
			}
			else
			{
				hr = CreateSurface(pParam->uWidth, pParam->uHeight, textureCap.d3dFormat, D3DPOOL_DEFAULT, false, &texture.pSurfaceVidMem);
				if (FAILED(hr))
					break;

				if (pParam->eUsage & TEXTURE_USAGE_BACKSTORE)
					hr = CreateSurface(pParam->uWidth, pParam->uHeight, textureCap.d3dFormat, D3DPOOL_SYSTEMMEM, false, &texture.pSurfaceSysMem);
				else
					hr = texture.pSurfaceVidMem->QueryInterface(IID_IDirect3DSurface9, (void **)&texture.pSurfaceSysMem);
			}
		}
		break;

	case PLANE_FORMAT_AYUV:	//< 32 bit, A8Y8U8V8
	case PLANE_FORMAT_AV12:	//< 20 bit, NV12 4:2:0 + A8, or ANV12
	case PLANE_FORMAT_NV12:	//< 12 bit, 4:2:0
	case PLANE_FORMAT_NV24: //< 12 bit, 4:2:0
	case PLANE_FORMAT_IMC3:	//< 12 bit, 4:2:0
	case PLANE_FORMAT_YV12:	//< 12 bit, 4:2:0
	case PLANE_FORMAT_YUY2:	//< 16 bit, YUV 4:2:2
		if (pParam->eUsage & TEXTURE_USAGE_RENDERTARGET)
		{
			hr = CreateSurface(pParam->uWidth, pParam->uHeight, textureCap.d3dFormat, D3DPOOL_DEFAULT, true, &texture.pSurfaceVidMem);
		}
		else
		{
			hr = CreateSurface(pParam->uWidth, pParam->uHeight, textureCap.d3dFormat, D3DPOOL_DEFAULT, false, &texture.pSurfaceVidMem);
			if (FAILED(hr))
				break;

			if (pParam->eUsage & TEXTURE_USAGE_BACKSTORE)
				hr = CreateSurface(pParam->uWidth, pParam->uHeight, textureCap.d3dFormat, D3DPOOL_SYSTEMMEM, false, &texture.pSurfaceSysMem);
			else
				hr = texture.pSurfaceVidMem->QueryInterface(IID_IDirect3DSurface9, (void **)&texture.pSurfaceSysMem);
		}
		break;

	case PLANE_FORMAT_P8:	//< 8 bit, I8 indexed
		// P8 can't be render target.
		if (pParam->eUsage & TEXTURE_USAGE_RENDERTARGET)
			break;

		if (ePoolUsage == TEXTURE_POOL_USAGE_D3D9)
		{
			if (m_bCreatePowerOf2Texture)
				CalTexturePowerOf2Size(pParam->uWidth, pParam->uHeight);

			hr = m_pDevice->CreateTexture(pParam->uWidth, pParam->uHeight, 1, 0, textureCap.d3dFormat, D3DPOOL_DEFAULT, &texture.pTextureVidMem, NULL);
			if (SUCCEEDED(hr))
			{
				pParam->eUsage = TEXTURE_USAGE_LOCKABLE_BACKSTORE;
				// D3DFMT_L8 may not be created as offscreen plain suface.
				hr = m_pDevice->CreateTexture(pParam->uWidth, pParam->uHeight, 1, 0, textureCap.d3dFormat, D3DPOOL_SYSTEMMEM, &texture.pTextureSysMem, NULL);
				if (SUCCEEDED(hr))
					hr = texture.pTextureSysMem->GetSurfaceLevel(0, &texture.pSurfaceSysMem);
			}

			if (SUCCEEDED(hr))
			{
				hr = texture.pTextureVidMem->GetSurfaceLevel(0, &texture.pSurfaceVidMem);
				if (!textureCap.bNativeTexture)
				{
					// the auxiliary texture to store palette for pixel shader.
					hr = m_pDevice->CreateTexture(256, 1, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &texture.pPaletteTextureVidMem, NULL);
					if (SUCCEEDED(hr))
					{
						D3DLOCKED_RECT lockedRect;
						hr = texture.pPaletteTextureVidMem->LockRect(0, &lockedRect, NULL, 0);
						if (SUCCEEDED(hr))
						{
							long *pBits = static_cast<long *> (lockedRect.pBits);
							for (int i = 0; i < 256; i++)
								*pBits++ = AYUVSample8ToD3DCOLOR(pParam->Palette[i]);
							hr = texture.pPaletteTextureVidMem->UnlockRect(0);
						}
					}
				}
			}
		}
		else
		{
			textureCap.d3dFormat = D3DFMT_P8;
			hr = CreateSurface(pParam->uWidth, pParam->uHeight, textureCap.d3dFormat, D3DPOOL_DEFAULT, false, &texture.pSurfaceVidMem);
			if (FAILED(hr))
				break;

			if (pParam->eUsage & TEXTURE_USAGE_BACKSTORE)
				hr = CreateSurface(pParam->uWidth, pParam->uHeight, textureCap.d3dFormat, D3DPOOL_SYSTEMMEM, false, &texture.pSurfaceSysMem);
			else
				hr = texture.pSurfaceVidMem->QueryInterface(IID_IDirect3DSurface9, (void **)&texture.pSurfaceSysMem);
		}

		texture.bPaletted = true;
		break;

	default:
		;
	}

	if (SUCCEEDED(hr))
	{
		ASSERT(m_uSID != 0);
		texture.uWidth = pParam->uWidth;
		texture.uHeight = pParam->uHeight;
		texture.Format = pParam->Format;
		texture.ePoolUsage = ePoolUsage;
		if ((pParam->eUsage & TEXTURE_USAGE_RENDERTARGET) == 0)
		{
			texture.pLockSurfaceCritSec = new CCritSec;
			ASSERT(texture.pLockSurfaceCritSec);
		}
		if (texture.bPaletted)
			memcpy(texture.Palette, pParam->Palette, sizeof(texture.Palette));
		*pTexture = reinterpret_cast<HANDLE> (m_uSID);
		m_Pool[*pTexture] = texture;
		m_uSID++;
	}
	else
	{
		DbgMsg("CD3D9TexturePool::CreateTexture(w=%d, h=%d, fmt=0x%x, usage=0x%x) failed hr = 0x%x",
			pParam->uWidth, pParam->uHeight, pParam->Format, pParam->eUsage, hr);
		ReleaseTexture(texture);
	}
	return hr;
}

HRESULT CD3D9TexturePool::CreateTexture(IUnknown *pObj, HANDLE *pTexture)
{
	HRESULT hr;
	D3D9Texture texture = {0};

	ASSERT(pObj && pTexture);

	hr = pObj->QueryInterface(__uuidof(IDirect3DSurface9), (void **)&texture.pSurfaceVidMem);
	if (SUCCEEDED(hr))
	{
		// the surface may not be a texture so do not check the returned value.
		texture.pSurfaceVidMem->GetContainer(__uuidof(IDirect3DTexture9), (void **)&texture.pTextureVidMem);
	}
	else
	{
		// if this is not a surface, it may be a texture.
		hr = pObj->QueryInterface(__uuidof(IDirect3DTexture9), (void **)&texture.pTextureVidMem);
		if (SUCCEEDED(hr))
		{
			// we must validate if the surface level 0 is a surface.
			hr = texture.pTextureVidMem->GetSurfaceLevel(0, &texture.pSurfaceVidMem);
		}
	}

	D3DSURFACE_DESC desc;
	hr = texture.pSurfaceVidMem->GetDesc(&desc);
	if (SUCCEEDED(hr))
	{
		ASSERT(m_uSID != 0);
		ASSERT(texture.pSurfaceVidMem);
		texture.uWidth = desc.Width;
		texture.uHeight = desc.Height;
		texture.Format = static_cast<PLANE_FORMAT> (desc.Format);
		*pTexture = reinterpret_cast<HANDLE> (m_uSID);
		m_Pool[*pTexture] = texture;
		m_uSID++;
	}
	else
	{
		ReleaseTexture(texture);
	}
	return hr;
}

HRESULT CD3D9TexturePool::CreateSurface(UINT uWidth, UINT uHeight, D3DFORMAT Format, D3DPOOL Pool, bool bRenderTarget, IDirect3DSurface9** ppSurface)
{
	HRESULT hr;

	// We need to create surface by IDirectXVideoAccelerationService to get surface with usage render target for
	// DXVA2 video processor. It does not seem to have effects by doing so so far.
	if (bRenderTarget && Pool == D3DPOOL_DEFAULT && m_pAccelService)
		hr = m_pAccelService->CreateSurface(uWidth, uHeight, 0, Format, Pool, 0, DXVA2_VideoProcessorRenderTarget, ppSurface, NULL);
	else
		hr = m_pDevice->CreateOffscreenPlainSurface(uWidth, uHeight, Format, Pool, ppSurface, NULL);

	return hr;
}

void CD3D9TexturePool::ReleaseTexture(D3D9Texture &t)
{
	int ref = 0;

	ASSERT(t.bLocked == false);
	if (t.pLockSurfaceCritSec)
		t.pLockSurfaceCritSec->Lock();

	if (t.pSurfaceSysMem)
		ref = t.pSurfaceSysMem->Release();
	if (t.pSurfaceVidMem)
		ref = t.pSurfaceVidMem->Release();
	if (t.pTextureVidMem)
		ref = t.pTextureVidMem->Release();
	if (t.pTextureSysMem)
		ref = t.pTextureSysMem->Release();
	if (t.pPaletteTextureVidMem)
		ref = t.pPaletteTextureVidMem->Release();

	if (t.pLockSurfaceCritSec)
	{
		t.pLockSurfaceCritSec->Unlock();
		delete t.pLockSurfaceCritSec;
		t.pLockSurfaceCritSec = NULL;
	}
}

HRESULT CD3D9TexturePool::ReleaseTexture(HANDLE hTexture)
{
	TextureMap::iterator i = m_Pool.find(hTexture);

	if (i != m_Pool.end())
	{
		if (m_bDeferredSurfaceRelease)
			m_RetainedTextureList.push_back(i->second);
		else
			ReleaseTexture(i->second);
		m_Pool.erase(i);
		return S_OK;
	}
	return E_FAIL;
}

HRESULT CD3D9TexturePool::GetRepresentation(HANDLE hTexture, REFIID riid, void **pOut)
{
	HRESULT hr = E_FAIL;
	TextureMap::iterator i = m_Pool.find(hTexture);

	if (i != m_Pool.end())
	{
		D3D9Texture &t = i->second;

		if (__uuidof(IDirect3DSurface9) == riid && t.pSurfaceVidMem)
			hr = t.pSurfaceVidMem->QueryInterface(riid, pOut);
		else if (__uuidof(IDirect3DTexture9) == riid && t.pTextureVidMem)
			hr = t.pTextureVidMem->QueryInterface(riid, pOut);
		else if (__uuidof(IDirect3DTexturePalette9) == riid && t.pPaletteTextureVidMem)
			hr = t.pPaletteTextureVidMem->QueryInterface(__uuidof(IDirect3DTexture9), pOut);
		else if (t.pSurfaceVidMem)
			hr = t.pSurfaceVidMem->QueryInterface(riid, pOut);
		else
			hr = E_NOINTERFACE;
	}
	return hr;
}

HRESULT CD3D9TexturePool::QuerySupportedFormatCount(UINT *pCount)
{
	if (m_pDevice)
	{
		*pCount = m_TextureCap.size();
		return S_OK;
	}
	return E_FAIL;
}

HRESULT CD3D9TexturePool::QuerySupportedFormats(PLANE_FORMAT *pFormat)
{
	if (m_pDevice)
	{
		for (TextureCapMap::iterator i = m_TextureCap.begin(); i != m_TextureCap.end(); ++i)
		{
			*pFormat++ = i->first;
		}
	}
	return S_OK;
}

HRESULT CD3D9TexturePool::QueryFormatCaps(PLANE_FORMAT Format, TextureCap *pCap)
{
	TextureCapMap::iterator i = m_TextureCap.find(Format);
	if (i != m_TextureCap.end())
	{
		*pCap = i->second;
		return S_OK;
	}
	return E_FAIL;
}

HRESULT CD3D9TexturePool::LockBuffer(HANDLE hTexture, LockedRect *pOut, const RECT *pRectSrc, DWORD dwFlags)
{
	HRESULT hr = E_FAIL;
	if (!pOut || (pRectSrc && IsRectEmpty(pRectSrc)))
		return E_INVALIDARG;

	TextureMap::iterator i = m_Pool.find(hTexture);

	if (i != m_Pool.end())
	{
		D3D9Texture &t = i->second;
		ASSERT(t.pSurfaceSysMem && t.pLockSurfaceCritSec);
		D3DLOCKED_RECT lockedRect;
		DWORD dwLockFlag = 0;

		if (t.bLocked)
			return E_FAIL;

		t.pLockSurfaceCritSec->Lock();
		t.dwUnlockAction = 0;

		if (dwFlags & PLANE_LOCK_READONLY)
		{
			dwLockFlag |= D3DLOCK_READONLY;
			t.dwUnlockAction = UNLOCK_ACTION_READONLY;
		}
		else if (t.pSurfaceSysMem != t.pSurfaceVidMem)
		{
			t.dwUnlockAction = UNLOCK_ACTION_UPDATESURFACE;
		}

		hr = t.pSurfaceSysMem->LockRect(&lockedRect, pRectSrc, dwLockFlag);
		if (SUCCEEDED(hr))
		{
			t.LockedBuffer.uPitch = pOut->uPitch = static_cast<UINT> (lockedRect.Pitch);
			t.LockedBuffer.pBuffer = pOut->pBuffer = lockedRect.pBits;
			if (pRectSrc)
				t.rcLockedRect = *pRectSrc;
			else
				SetRect(&t.rcLockedRect, 0, 0, t.uWidth, t.uHeight);
			ASSERT(IsRectEmpty(&t.rcLockedRect) != TRUE);
			t.bLocked = true;
		}
		else
		{
			t.pLockSurfaceCritSec->Unlock();
		}
	}
	return hr;
}

HRESULT CD3D9TexturePool::UnlockBuffer(HANDLE hTexture, ITextureFilter *pPostFilter)
{
	HRESULT hr = E_FAIL;
	TextureMap::iterator i = m_Pool.find(hTexture);

	if (i != m_Pool.end())
	{
		D3D9Texture &t = i->second;

		ASSERT(t.pSurfaceSysMem && t.pLockSurfaceCritSec);
		ASSERT(t.bLocked);

		if ((t.dwUnlockAction & PLANE_LOCK_READONLY) == 0)
		{
			if (pPostFilter)
			{
				t.bValid = (pPostFilter->Process(t.rcLockedRect, t.LockedBuffer) == S_OK);
			}
			else
			{
				// assuming the first lockbuffer call fills data.
				t.bValid = true;
			}
		}

		hr = t.pSurfaceSysMem->UnlockRect();
		if (SUCCEEDED(hr))
		{
			if ((t.dwUnlockAction & UNLOCK_ACTION_READONLY) == 0)
			{
				if (t.dwUnlockAction & UNLOCK_ACTION_UPDATESURFACE && t.pSurfaceVidMem)
				{
					const POINT pt = { t.rcLockedRect.left, t.rcLockedRect.top };
					hr = m_pDevice->UpdateSurface(t.pSurfaceSysMem, &t.rcLockedRect, t.pSurfaceVidMem, &pt);
				}

				//			if (t.dwUnlockAction & UNLOCK_ACTION_CSC)
			}

			t.bLocked = false;
			t.pLockSurfaceCritSec->Unlock();
			if (!t.bValid)
				hr = S_FALSE;
		}
	}
	return hr;
}

HRESULT CD3D9TexturePool::SetPoolUsage(TEXTURE_POOL_USAGE ePoolUsage)
{
	m_ePoolUsage = ePoolUsage;
	return S_OK;
}

HRESULT CD3D9TexturePool::GetPoolUsage(TEXTURE_POOL_USAGE *lpePoolUsage)
{
	CHECK_POINTER(lpePoolUsage);
	*lpePoolUsage = m_ePoolUsage;
	return S_OK;
}

HRESULT CD3D9TexturePool::EvictResources()
{
	HRESULT hr = S_OK;
	int ref = 0;

	for (TextureMap::iterator i = m_Pool.begin(); i != m_Pool.end(); ++i)
	{
		D3D9Texture &t = i->second;

		// it is not a lockable resource, we only evict lockable surfaces now.
		if (t.pSurfaceSysMem == 0 || t.pSurfaceVidMem == 0)
			continue;

		// we need to explicit handle surfaces without system memory back store.
		t.pLockSurfaceCritSec->Lock();
		if (t.pSurfaceSysMem == t.pSurfaceVidMem)
		{
			D3DSURFACE_DESC desc;
			ref = t.pSurfaceSysMem->Release();	ASSERT(ref > 0);

			hr = t.pSurfaceVidMem->GetDesc(&desc);	ASSERT(SUCCEEDED(hr));
			hr = CreateSurface(desc.Width, desc.Height, desc.Format, D3DPOOL_SYSTEMMEM, false, &t.pSurfaceSysMem); ASSERT(SUCCEEDED(hr));
			hr = m_pDevice->GetRenderTargetData(t.pSurfaceVidMem, t.pSurfaceSysMem);
			// manually backup data
			if (FAILED(hr))
			{
				D3DLOCKED_RECT lockedVidMem;
				D3DLOCKED_RECT lockedSysMem;

				ASSERT(t.bLocked == false);
				hr = t.pSurfaceVidMem->LockRect(&lockedVidMem, NULL, D3DLOCK_READONLY);
				if (SUCCEEDED(hr))
				{
					hr = t.pSurfaceSysMem->LockRect(&lockedSysMem, NULL, 0);
					if (SUCCEEDED(hr))
					{
						// we mainly evict ARGB surfaces, so this code should work for now but may not work in other formats.
						if (lockedSysMem.Pitch == lockedVidMem.Pitch)
						{
							memcpy(lockedSysMem.pBits, lockedVidMem.pBits, lockedSysMem.Pitch * desc.Height);
						}
						else
						{
							unsigned char *pDst = static_cast<unsigned char *> (lockedSysMem.pBits);
							unsigned char *pSrc = static_cast<unsigned char *> (lockedVidMem.pBits);
							for (unsigned int k = 0; k < desc.Height; k++)
							{
								memcpy(pDst, pSrc, lockedVidMem.Pitch);
								pDst += lockedSysMem.Pitch;
								pSrc += lockedVidMem.Pitch;
							}
						}
						t.pSurfaceSysMem->UnlockRect();
					}
					t.pSurfaceVidMem->UnlockRect();
				}
			}
		}

		SAFE_RELEASE(t.pTextureVidMem);
		ref = t.pSurfaceVidMem->Release();	ASSERT(ref == 0);
		t.pSurfaceVidMem = 0;
		t.pLockSurfaceCritSec->Unlock();
	}
	return hr;
}

HRESULT CD3D9TexturePool::RestoreResources()
{
	HRESULT hr = S_OK;
	D3DSURFACE_DESC desc;

	for (TextureMap::iterator i = m_Pool.begin(); i != m_Pool.end(); ++i)
	{
		D3D9Texture &t = i->second;

		// it is not a lockable resource
		if (t.pSurfaceSysMem == 0)
			continue;

		ASSERT(t.pSurfaceVidMem == 0 && t.pTextureVidMem == 0);
		t.pLockSurfaceCritSec->Lock();
		hr = t.pSurfaceSysMem->GetDesc(&desc);	ASSERT(SUCCEEDED(hr));
		if (PLANE_FORMAT_ARGB == t.Format && t.ePoolUsage == TEXTURE_POOL_USAGE_D3D9)
		{
			hr = m_pDevice->CreateTexture(desc.Width, desc.Height, 1, 0, desc.Format, D3DPOOL_DEFAULT, &t.pTextureVidMem, NULL);
			if (SUCCEEDED(hr))
			{
				hr = t.pTextureVidMem->GetSurfaceLevel(0, &t.pSurfaceVidMem);	ASSERT(SUCCEEDED(hr));
				hr = m_pDevice->UpdateSurface(t.pSurfaceSysMem, NULL, t.pSurfaceVidMem, NULL);	ASSERT(SUCCEEDED(hr));
			}
		}
		else
		{
			hr = CreateSurface(desc.Width, desc.Height, desc.Format, D3DPOOL_DEFAULT, false, &t.pSurfaceVidMem);
			if (SUCCEEDED(hr) && t.pSurfaceVidMem)
			{
				hr = m_pDevice->UpdateSurface(t.pSurfaceSysMem, NULL, t.pSurfaceVidMem, NULL);	ASSERT(SUCCEEDED(hr));
				int ref = t.pSurfaceSysMem->Release();	ASSERT(ref == 0);
				hr = t.pSurfaceVidMem->QueryInterface(IID_IDirect3DSurface9, (void **)&t.pSurfaceSysMem);	ASSERT(SUCCEEDED(hr));
			}
		}
		t.pLockSurfaceCritSec->Unlock();
		ASSERT(SUCCEEDED(hr));
	}
	return hr;
}

void CD3D9TexturePool::ReclaimResources()
{
	while (!m_RetainedTextureList.empty())
	{
		ReleaseTexture(m_RetainedTextureList.front());
		m_RetainedTextureList.pop_front();
	}
}