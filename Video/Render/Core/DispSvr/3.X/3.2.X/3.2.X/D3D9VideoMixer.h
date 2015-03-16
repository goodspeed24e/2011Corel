#ifndef __DISPSVR_D3D9_VIDEO_MIXER_H__
#define __DISPSVR_D3D9_VIDEO_MIXER_H__

#include "D3D9VideoMixerBase.h"

namespace DispSvr
{
	struct D3D9VideoMixerVertex
	{
		D3DVECTOR position;
		D3DCOLOR color;
		struct {
			FLOAT u, v;
		} tx [5];
		enum { FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1 };
		enum { FVF5 = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX5 };
	};

	struct D3D9ClearRectVertex
	{
		D3DVECTOR position;
		enum { FVF = D3DFVF_XYZ };
	};

	template<typename VertexType>
	union PlaneVB
	{
		struct { VertexType LT, LB, RB, RT; };
		VertexType VB[4];
	};

	class CD3D9VideoMixer : public CD3D9VideoMixerBase
	{
	public:
		CD3D9VideoMixer();
		virtual ~CD3D9VideoMixer();

		// only these two methods would affect vertex buffer position.
		STDMETHOD(SetDestination)(IUnknown *pSurface, const NORMALIZEDRECT *pDest);
//		STDMETHOD(_SetPosition)(PLANE_ID PlaneID, const NORMALIZEDRECT *rcDst, const RECT *rcSrc, const NORMALIZEDRECT *rcCrop, float fAspectRatio);
		STDMETHOD(SetClearRectangles)(UINT uCount, ClearRect *pRects);

	protected:
		STDMETHOD(_Execute)(IDirect3DSurface9 *pDestSurface, const RECT &rcDst, const RECT &rcDstClip);
		STDMETHOD(_SetDevice)(IUnknown *pDevice);
		STDMETHOD(_ReleaseDevice)();
		STDMETHOD(_QueryPlaneCaps)(PLANE_ID PlaneID, PLANE_FORMAT Format, PlaneCaps *pCap);

		HRESULT DrawVideoPlane(IDirect3DSurface9 *pDstSurface, const RECT &rcDst, const RECT &rcDstClip);
		HRESULT DrawSubVideoPlane(IDirect3DSurface9 *pDstSurface, const RECT &rcDst, const RECT &rcDstClip);
		HRESULT DrawTexturePlane(PLANE_ID PlaneID, const RECT &rcDst, const RECT &rcDstClip);
		void AdjustVertexBuffer(const D3D9Plane &, const RECT &rcSrc, IDirect3DTexture9 *pTexture, const RECT &rcDst, const RECT &rcDstClip, PlaneVB<D3D9VideoMixerVertex> &rectVB);
		HRESULT LoadPixelShader();
		HRESULT CheckStretchRectFormula();
		HRESULT SetupClearRectStage();

	protected:
		typedef std::list<PlaneVB<D3D9ClearRectVertex> > ClearRectVBList;

	protected:
		const D3DXMATRIX MIXING_TRANSFORM_MATRIX;
		bool m_bNomialRange16_235;
		D3DXMATRIX m_matrixWorld;
		IDirect3DPixelShader9 *m_pLumaKeyPS;
		IDirect3DPixelShader9 *m_pL8toArgbPS;
		ClearRectVBList m_MainCRList;
		ClearRectVBList m_SubCRList;
	};

	/// CD3D9Dxva2VideoMixer uses DXVA2 VideoProcessBlt when available.
	class CD3D9Dxva2VideoMixer : public CD3D9VideoMixer
	{
	public:
		CD3D9Dxva2VideoMixer();
		virtual ~CD3D9Dxva2VideoMixer();

	protected:
		STDMETHOD(_SetDevice)(IUnknown *pDevice);
	};
}

#endif	// __DISPSVR_D3D9_VIDEO_MIXER_H__