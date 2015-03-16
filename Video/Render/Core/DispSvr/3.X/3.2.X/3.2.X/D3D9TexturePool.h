#ifndef _DISPSVR_D3D9TEXTURE_POOL_H_
#define _DISPSVR_D3D9TEXTURE_POOL_H_

#include <map>
#include <list>
#include "D3D9PluginBase.h"

interface IDirectXVideoAccelerationService;

MIDL_INTERFACE("DF68F9F1-9E3D-4d0a-84E9-20CD0EA40774")
IDirect3DTexturePalette9 : IDirect3DTexture9
{
};

MIDL_INTERFACE("9FCCB988-85D1-42d3-AA33-C39CCA27FFD8") IDispSvrTexturePool : public IUnknown
{
};

namespace DispSvr
{
	struct ITextureFilter
	{
		virtual ~ITextureFilter() {}
		virtual HRESULT Process(const RECT &rcLocked, const LockedRect &lockedBuffer) = 0;
	};

	struct TextureCap
	{
		UINT uMaxWidth;
		UINT uMaxHeight;
		bool bNativeTexture;
		D3DFORMAT d3dFormat;	//< d3d representation.
	};

	enum TEXTURE_POOL_USAGE
	{
		TEXTURE_POOL_USAGE_D3D9 = 0,					//< using standard D3D9 APIs mainly.
		TEXTURE_POOL_USAGE_OFFSCREENPLANESURFACE	= 1	//< prefer offscreen plane surface for custom mixer.
	};

	enum TEXTURE_USAGE_FLAG
	{
		/// TEXTURE_USAGE_LOCKABLE and TEXTURE_USAGE_RENDERTARGET are exclusive.
		TEXTURE_USAGE_LOCKABLE				= 0,
		/// TEXTURE_USAGE_LOCKABLE and TEXTURE_USAGE_RENDERTARGET are exclusive.
		TEXTURE_USAGE_RENDERTARGET			= 1 << 0,
		/// Create a backstore on system memory.
		TEXTURE_USAGE_BACKSTORE				= 1 << 1,
		TEXTURE_USAGE_TEXTURE				= 1 << 2,
        TEXTURE_USAGE_RESTRICTED_CONTENT    = 1 << 3,

        /// Create a lockable surface with backstore on system memory.
		TEXTURE_USAGE_LOCKABLE_BACKSTORE	= TEXTURE_USAGE_BACKSTORE | TEXTURE_USAGE_LOCKABLE,
		TEXTURE_USAGE_RENDERTARGET_TEXTURE	= TEXTURE_USAGE_RENDERTARGET | TEXTURE_USAGE_TEXTURE
	};

	struct CreateTextureParam
	{
		UINT uWidth;
		UINT uHeight;
		PLANE_FORMAT Format;
		AYUVSample8 Palette[256];
		DWORD eUsage;
	};

	// before we write texture manager, it is better to centerize all creation of video memory
	// surfaces in a single class.
    class CD3D9TexturePool : public CD3D9PluginBase
	{
	protected:
		enum UNLOCK_ACTION {
			UNLOCK_ACTION_READONLY			=	1 << 0,
			UNLOCK_ACTION_UPDATESURFACE		=	1 << 1,
			UNLOCK_ACTION_CSC				=	1 << 2
		};

		struct D3D9Texture
		{
			IDirect3DSurface9 *pSurfaceSysMem;	// off-screen surface on system memory pool
			IDirect3DSurface9 *pSurfaceVidMem;	// off-screen surface on default pool
			IDirect3DTexture9 *pTextureVidMem;	// texture on default memory pool
			IDirect3DTexture9 *pTextureSysMem;
			IDirect3DTexture9 *pPaletteTextureVidMem;
			PLANE_FORMAT Format;
			UINT uWidth;
			UINT uHeight;
			AYUVSample8	Palette[256];
			RECT rcLockedRect;
			LockedRect LockedBuffer;
			DWORD dwUnlockAction;
			CCritSec *pLockSurfaceCritSec;
			bool bLocked;
			bool bPaletted;
			bool bValid;					// if the content is valid to display.
			TEXTURE_POOL_USAGE ePoolUsage;
		};

	public:
		CD3D9TexturePool();
		virtual ~CD3D9TexturePool();

        // IUnknown
		STDMETHOD(QueryInterface)(REFIID riid, void **ppv);

		/// Create associated resources for requested format with the usage as render target.
		///
		/// @param pCreateParam param to create texture.
		/// @param hTexture	a handle to the resource to be returned on success.
		/// @param pCreateParam param to create texture.
		HRESULT CreateTexture(CreateTextureParam *pParam, HANDLE *hTexture);

		/// Create texture resources from an existing external surface.
		HRESULT CreateTexture(IUnknown *pObj, HANDLE *hTexture);
		HRESULT ReleaseTexture(HANDLE hTexture);

		/// Lock texture buffer. It may returns video or system memory address based on how texture is created.
		HRESULT LockBuffer(HANDLE hTex, LockedRect *pOut, const RECT *rcSrc, DWORD dwFlags);

		/// Unlock texture buffer.
		/// @return S_OK, the buffer is unlocked and updated to video memory pair if needed.
		///		S_FALSE, the buffer is unlocked and is not changed when locked readonly especially.
		///		otherwise, error code is returned.
		HRESULT UnlockBuffer(HANDLE hTex, ITextureFilter *pPostFilter = 0);

		HRESULT GetRepresentation(HANDLE hTex, REFIID riid, void **pOut);
		HRESULT QuerySupportedFormatCount(UINT *pCount);
		HRESULT QuerySupportedFormats(PLANE_FORMAT *pFormat);
		HRESULT QueryFormatCaps(PLANE_FORMAT Format, TextureCap *pCap);

		HRESULT SetPoolUsage(TEXTURE_POOL_USAGE ePoolUsage);
		HRESULT GetPoolUsage(TEXTURE_POOL_USAGE *lpePoolUsage);

		HRESULT EvictResources();
		HRESULT RestoreResources();
		void ReclaimResources();

	protected:
        // CD3D9PluginBase
		STDMETHOD(_SetDevice)(IUnknown *pDevice);
		STDMETHOD(_ReleaseDevice)();
//		STDMETHOD(_SetWindow)(HWND hwnd);

        void CheckTexturePowerOf2Size(UINT &w, UINT &h);
		void ReleaseTexture(D3D9Texture &texture);
		HRESULT CreateSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool, bool bRenderTarget, IDirect3DSurface9** ppSurface, DWORD dwD3DUsage = 0);

	protected:
		typedef std::map<HANDLE, D3D9Texture> TextureMap;
		typedef std::map<PLANE_FORMAT, TextureCap> TextureCapMap;
		typedef std::list<D3D9Texture> TextureList;

		IDirectXVideoAccelerationService *m_pAccelService;
		bool m_bCreatePowerOf2Texture;
		bool m_bDeferredSurfaceRelease;
		TextureMap m_Pool;
		TextureCapMap m_TextureCap;
		TextureList m_RetainedTextureList;
		TEXTURE_POOL_USAGE m_ePoolUsage;
		UINT m_uSID;
	};
} // namespace DispSvr

#endif	// _DISPSVR_D3D9TEXTURE_POOL_H_