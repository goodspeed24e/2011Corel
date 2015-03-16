#ifndef __DISPSVR_D3D9_VBLT_H__
#define __DISPSVR_D3D9_VBLT_H__

#define S_D3D9VBLT_RETURN_CACHE     0x01010101

namespace DispSvr
{
	/// Abstract video blitting processing and encapsulate related resources used by the VBlt.
	struct ID3D9VBlt
	{
		virtual ~ID3D9VBlt() {}

		/// VBlt to an intermediate texture, which is destroyed or created when necessary.
		/// Generated texture format should be one of those possible formats.
		/// @param pSrc source surface for vblt.
		/// @param rcSrc rectangle of source surface, this implies intermediate surface will be created as the same size of rcSrc.
		/// @param pIntermediate output of the created intermediate texture. caller must call Release() on the texture when done.
        virtual HRESULT IntermediateTextureVBlt(IDirect3DSurface9 *pSrc, const RECT &rcSrc, IDirect3DTexture9 **pIntermediate) = 0;

		/// VBlt to an intermediate surface. The intermediate surface is destroyed or created when necessary.
		/// @param pSrc source surface for vblt.
		/// @param rcSrc rectangle of source surface, this implies intermediate surface will be created as the same size of rcSrc.
		/// @param fmtInter intermediate surface format.
		/// @param pIntermediate output of the created intermediate surface. caller must call Release() on the surface when done.
        virtual HRESULT IntermediateVBlt(IDirect3DSurface9 *pSrc, const RECT &rcSrc, PLANE_FORMAT fmtInter, IDirect3DSurface9 **pIntermediate) = 0;

		/// Directly VBlt to destination surface.
		/// @param pSrc source surface for vblt.
		/// @param rcSrc rectangle of source surface.
		/// @param pDst destination surface for vblt.
		/// @param rcDst rectangle of destination surface.
		virtual HRESULT DirectVBlt(IDirect3DSurface9 *pSrc, const RECT &rcSrc, IDirect3DSurface9 *pDst, const RECT &rcDst) = 0;


        // Set Filter value for DXVA2 Processing.
        virtual HRESULT SetFilterValue(VIDEO_FILTER eFilter, float fValue)=0;
	};

	struct D3D9Plane;
	struct VideoProcessorStub;
	class CD3D9TexturePool;

	typedef ID3D9VBlt *(*TpfnVBltFactoryMethod)(D3D9Plane *pPlane, VideoProcessorStub *pVPStub, CD3D9TexturePool *pPool);

	/// D3D9 blit, use StretchRect to blit a source surface to destination render target.
	class CD3D9VBlt : public ID3D9VBlt
	{
	public:
		static ID3D9VBlt *Create(D3D9Plane *pPlane, VideoProcessorStub *pVPStub, CD3D9TexturePool *pPool);
		virtual ~CD3D9VBlt();
        virtual HRESULT IntermediateTextureVBlt(IDirect3DSurface9 *pSrc, const RECT &rcSrc, IDirect3DTexture9 **pIntermediate);
        virtual HRESULT IntermediateVBlt(IDirect3DSurface9 *pSrc, const RECT &rcSrc, PLANE_FORMAT fmtInter, IDirect3DSurface9 **pIntermediate);
		virtual HRESULT DirectVBlt(IDirect3DSurface9 *pSrc, const RECT &rcSrc, IDirect3DSurface9 *pDst, const RECT &rcDst);
        virtual HRESULT SetFilterValue(VIDEO_FILTER eFilter, float fValue) { return E_NOTIMPL; }

	protected:
		CD3D9VBlt(D3D9Plane *, VideoProcessorStub *, CD3D9TexturePool *);
        HRESULT IntermediateVBlt(IDirect3DSurface9 *pSrc, const RECT &rcSrc, PLANE_FORMAT fmtInter, IDirect3DSurface9 **pIntermediate, bool bTexture, bool bUseVP = 0);

	protected:
		D3D9Plane *m_pPlane;
		VideoProcessorStub *m_pVPStub;
		CD3D9TexturePool *m_pTexturePool;
		HANDLE m_hIVPBTexture;
		UINT m_uIVPBWidth;
		UINT m_uIVPBHeight;
		PLANE_FORMAT m_IVPBFormat;
		bool m_bUsageAsTexture;
        DWORD m_dwLastVPTS;
	};

	class CDXVA2VideoProcessor;
	struct DXVA2VP_RefSample;

	/// DXVA2 blit, use VideoProcessBlt to blit a source surface to destination render target.
	class CDXVA2VBlt : public CD3D9VBlt
	{
	public:
		static ID3D9VBlt *Create(D3D9Plane *pPlane, VideoProcessorStub *pVPStub, CD3D9TexturePool *pPool);
		virtual ~CDXVA2VBlt();
		virtual HRESULT DirectVBlt(IDirect3DSurface9 *pSrc, const RECT &rcSrc, IDirect3DSurface9 *pDst, const RECT &rcDst);
        virtual HRESULT SetFilterValue(VIDEO_FILTER eFilter, float fValue);

	protected:
		CDXVA2VBlt(D3D9Plane *, VideoProcessorStub *, CD3D9TexturePool *);

	protected:
		CDXVA2VideoProcessor *m_pVP;
		DXVA2VP_RefSample *m_pRefSamples;
	};
}

#endif	// __DISPSVR_D3D9_VBLT_H__