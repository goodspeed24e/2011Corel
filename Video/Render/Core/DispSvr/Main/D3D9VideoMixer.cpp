#include "stdafx.h"
#include "DispSvr.h"
#include "ResourceManager.h"
#include "DynLibManager.h"
#include "MathVideoMixing.h"
#include "Shaders/LumaKeying_0_255.h"
#include "Shaders/LumaKeying_16_235.h"
#include "Shaders/ConvertL8toP8.h"
#include "D3D9VideoMixer.h"
#include "D3D9VideoEffect3DManager.h"
#include "RegistryService.h"
#include "Imports/LibGPU/GPUID.h"

using namespace DispSvr;

// turn on the pound define will draw only wired frame of the primitives.
#ifdef _DEBUG_D3D
#	define DEBUG_TRANSFORM
#endif

CD3D9VideoMixer::CD3D9VideoMixer() :
// transform matrix to convert mixer coordinate [0 ~ 1] to d3d [-1 ~ 1]
	MIXING_TRANSFORM_MATRIX(
		2, 0, 0, 0,
		0, -2, 0, 0,
		0, 0, 1, 0,
		-1, 1, 0, 1
	),
	m_matrixWorld(MIXING_TRANSFORM_MATRIX),
	m_pLumaKeyPS(0),
	m_pL8toArgbPS(0),
	m_bNomialRange16_235(true)
{
	m_GUID = DISPSVR_RESOURCE_D3DVIDEOMIXER;
}

CD3D9VideoMixer::~CD3D9VideoMixer()
{
}

STDMETHODIMP CD3D9VideoMixer::_SetDevice(IUnknown *pDevice)
{
	HRESULT hr = CD3D9VideoMixerBase::_SetDevice(pDevice);
	if (CDynLibManager::GetInstance()->pfnDXVA2CreateVideoService == 0)
		CheckStretchRectFormula();
	LoadPixelShader();
	return hr;
}

STDMETHODIMP CD3D9VideoMixer::_ReleaseDevice()
{
	SAFE_RELEASE(m_pLumaKeyPS);
	SAFE_RELEASE(m_pL8toArgbPS);
	return CD3D9VideoMixerBase::_ReleaseDevice();
}

STDMETHODIMP CD3D9VideoMixer::_QueryPlaneCaps(PLANE_ID PlaneID, PLANE_FORMAT Format, PlaneCaps *pCap)
{
	pCap->dwFlags |= PLANE_CAP_HW_PARTIAL_BLENDING;
	return S_OK;
}

STDMETHODIMP CD3D9VideoMixer::SetDestination(IUnknown *pDestSurface, const NORMALIZEDRECT *pDestRect)
{
	HRESULT hr = CD3D9VideoMixerBase::SetDestination(pDestSurface, pDestRect);
	if (SUCCEEDED(hr) && (m_Property.dwFlags & MIXER_PROPERTY_VIRTUALIZATION) != 0)
	{

//		const D3DXMATRIX matrix(
//			pDestRect->right - pDestRect->left, 0, 0, 0,
//			0, pDestRect->bottom - pDestRect->top, 0, 0,
//			0, 0, 1, 0,
//			pDestRect->left, pDestRect->top, 0, 1
//			);
//		D3DXMatrixMultiply(&m_matrixWorld, &matrix, &MIXING_TRANSFORM_MATRIX);

		// only 4 elements of the transformation matrix are changed in the multiplying operation.
		m_matrixWorld(0, 0) = 2 * (pDestRect->right - pDestRect->left);
		m_matrixWorld(1, 1) = -2 * (pDestRect->bottom - pDestRect->top);
		m_matrixWorld(3, 0) = 2 * pDestRect->left - 1;
		m_matrixWorld(3, 1) = -2 * pDestRect->top + 1;
	}
	return hr;
}

STDMETHODIMP CD3D9VideoMixer::SetClearRectangles(UINT uCount, ClearRect *pRects)
{
	CAutoLock selfLock(&m_csObj);
	m_MainCRList.clear();
	m_SubCRList.clear();

	for (UINT i = 0; i < uCount; ++i, ++pRects)
	{
		PlaneVB<D3D9ClearRectVertex> vb;

		vb.LT.position.x = pRects->Rect.left;
		vb.LT.position.y = pRects->Rect.top;
		vb.LB.position.x = pRects->Rect.left;
		vb.LB.position.y = pRects->Rect.bottom;
		vb.RB.position.x = pRects->Rect.right;
		vb.RB.position.y = pRects->Rect.bottom;
		vb.RT.position.x = pRects->Rect.right;
		vb.RT.position.y = pRects->Rect.top;

		vb.LT.position.z = vb.LB.position.z = vb.RB.position.z = vb.RT.position.z = 0.5f;

		if (pRects->Target == CLEAR_RECT_TARGET_SUB)
			m_SubCRList.push_back(vb);
		else
			m_MainCRList.push_back(vb);
	}
	return S_OK;
}

HRESULT CD3D9VideoMixer::_Execute(IDirect3DSurface9 *pDestSurface, const RECT &rcDst, const RECT &rcDstClip)
{
	HRESULT hr = S_OK;
	D3DMATRIX matrixWorld;

	D3DXMATRIX idMatrix;
	D3DXMatrixIdentity(&idMatrix);
	hr = m_pDevice->SetTransform(D3DTS_VIEW, &idMatrix);
	hr = m_pDevice->SetTransform(D3DTS_PROJECTION, &idMatrix);

	hr = m_pDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	hr = m_pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	hr = m_pDevice->GetTransform(D3DTS_WORLD, &matrixWorld);
	hr = m_pDevice->MultiplyTransform(D3DTS_WORLD, &m_matrixWorld);

	hr = m_pDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	hr = m_pDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
	hr = m_pDevice->SetSamplerState(0, D3DSAMP_BORDERCOLOR, 0x00000000);

	hr = m_pDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
	hr = m_pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	hr = m_pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

	hr = m_pDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

	hr = m_pDevice->SetRenderState(D3DRS_ALPHAREF, 0x0);
	hr = m_pDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
	hr = m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	hr = m_pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);

	hr = m_pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	hr = m_pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);

	hr = m_pDevice->SetFVF(D3D9VideoMixerVertex::FVF);

	hr = m_pDevice->SetRenderState(D3DRS_ZENABLE, FALSE);

	if (IsBackgroundVisible())
		hr = DrawTexturePlane(PLANE_BACKGROUND, rcDst, rcDstClip);
	hr = DrawVideoPlane(pDestSurface, rcDst, rcDstClip);

	hr = SetupClearRectStage();

	hr = m_pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	hr = m_pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	if (!m_MainCRList.empty())
	{
		hr = m_pDevice->SetRenderState(D3DRS_STENCILENABLE, TRUE);
		hr = m_pDevice->SetRenderState(D3DRS_STENCILREF, 0x2);
	}

	hr = DrawSubVideoPlane(pDestSurface, rcDst, rcDstClip);

	hr = m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	hr = m_pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);

	if (!m_SubCRList.empty())
	{
		hr = m_pDevice->SetRenderState(D3DRS_STENCILENABLE, TRUE);
		hr = m_pDevice->SetRenderState(D3DRS_STENCILREF, 0x1);
	}

	hr = DrawTexturePlane(PLANE_GRAPHICS, rcDst, rcDstClip);

	if (!m_MainCRList.empty() || !m_SubCRList.empty())
		hr = m_pDevice->SetRenderState(D3DRS_STENCILENABLE, FALSE);

	hr = DrawTexturePlane(PLANE_INTERACTIVE, rcDst, rcDstClip);
	hr = DrawTexturePlane(PLANE_OTHER, rcDst, rcDstClip);

	// Reset some render state because of weird issue happens with nvidia G8x/G9x + RGB overlay on XP if not set.
	hr = m_pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	hr = m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

	// restore the world matrix before exiting.
	hr = m_pDevice->SetTransform(D3DTS_WORLD, &matrixWorld);

	// Unbind texture. Doing so will ensure that dangling references to resources are removed before they cause
	// the resource manager to keep resources resident that are actually no longer in use.
	hr = m_pDevice->SetTexture(0, NULL);

	return hr;
}

HRESULT CD3D9VideoMixer::DrawVideoPlane(IDirect3DSurface9 *pDstSurface, const RECT &rcDst, const RECT &rcDstClip)
{
	HRESULT hr = S_OK;
	D3D9Plane &plane = m_Planes[PLANE_MAINVIDEO];
	RECT src = plane.rcSrc, dst = rcDst;

	if (!IsD3D9PlaneValid(&plane) || FAILED(PlaneToScreen(plane, src, dst, rcDstClip)))
		return S_FALSE;

	CComPtr<IDirect3DTexture9> pTexture;
	CComPtr<IDirect3DSurface9> pOffscreenSurface;
    CComPtr<IUnknown> pUnk;
    if (ProcessVideoEffect(PLANE_MAINVIDEO, pDstSurface, src, dst, plane.Format, &pUnk) != S_FALSE)
    {
        if (pUnk)
        {
            hr = pUnk->QueryInterface(IID_IDirect3DTexture9, (void **)&pTexture);
            if (FAILED(hr))
            {
                hr = pUnk->QueryInterface(IID_IDirect3DSurface9, (void **)&pOffscreenSurface);
                if (SUCCEEDED(hr) && (pDstSurface == pOffscreenSurface)) // render to dest surface already.
                {
                    return hr;
                }
            }
        }
    }
    else
    {
	hr = m_pTexturePool->GetRepresentation(plane.hTexture, IID_IDirect3DTexture9, (void **)&pTexture);
	if (FAILED(hr))
		hr = m_pTexturePool->GetRepresentation(plane.hTexture, IID_IDirect3DSurface9, (void **)&pOffscreenSurface);
    }
	// if pTexture is available, video has been post-processed and converted to a texture from raw YUV plane.
	if (pTexture)
	{
		PlaneVB<D3D9VideoMixerVertex> rectVB;
		AdjustVertexBuffer(plane, src, pTexture, dst, rcDst, rectVB);
		hr = m_pDevice->SetTexture(0, pTexture);
		hr = m_pDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, &rectVB, sizeof(D3D9VideoMixerVertex));

#ifdef DEBUG_TRANSFORM
		hr = m_pDevice->SetFVF(D3DFVF_XYZ);
		hr = m_pDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
		hr = m_pDevice->SetTexture(0, NULL);
		hr = m_pDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, &rectVB, sizeof(D3D9VideoMixerVertex));
#endif

		if (0)
		{
#ifndef _NO_USE_D3DXDLL
			hr = D3DXSaveTextureToFile(_T("C:\\do.bmp"), D3DXIFF_BMP, pTexture, NULL);
#endif
		}
	}
	else if (pOffscreenSurface)
	{
		hr = plane.pVBlt->DirectVBlt(pOffscreenSurface, src, pDstSurface, dst);
	}
	return hr;
}

HRESULT CD3D9VideoMixer::DrawSubVideoPlane(IDirect3DSurface9 *pDstSurface, const RECT &rcDst, const RECT &rcDstClip)
{
	HRESULT hr = S_OK;
	D3D9Plane &plane = m_Planes[PLANE_SUBVIDEO];
	RECT src = plane.rcSrc, dst = rcDst;

	if (!IsD3D9PlaneValid(&plane) || FAILED(PlaneToScreen(plane, src, dst, rcDstClip)))
		return S_FALSE;

	CComPtr<IDirect3DTexture9> pTexture;
	CComPtr<IDirect3DSurface9> pOffscreenSurface;

	hr = m_pTexturePool->GetRepresentation(plane.hTexture, __uuidof(IDirect3DTexture9), (void **)&pTexture);
	if (FAILED(hr))
	{
		hr = m_pTexturePool->GetRepresentation(plane.hTexture, __uuidof(IDirect3DSurface9), (void **)&pOffscreenSurface);
		if (FAILED(hr))
			return hr;

		// workaround for video processing which may produce garbage 
		BOOL bSurface2Texture = FALSE;
		{
			if (PCI_VENDOR_ID_NVIDIA == GetRegistry(REG_VENDOR_ID, 0))
			{
				if (0 < src.top || 0 < src.left || LONG(plane.uWidth) > src.right || LONG(plane.uHeight) > src.bottom)
				{
					bSurface2Texture = TRUE;
					DbgMsg("Workaround for Nvidia, can't render the partial surface to texture: bSurface2Texture = %i\n", bSurface2Texture);
				}
			}
		}
		//------------------------------------------------------------------------------------------------------------------------

		if (!m_MainCRList.empty() || m_LumaKey.bEnable || plane.fAlpha < 1.0f || bSurface2Texture)
		{
			CComPtr<IDirect3DSurface9> pIntermediateSurface;
			RECT rcSrc = {0, 0, plane.uWidth, plane.uHeight};
			hr = plane.pVBlt->IntermediateVBlt(pOffscreenSurface, rcSrc, PLANE_FORMAT_ARGB, &pIntermediateSurface);

			if (SUCCEEDED(hr))
				hr = pIntermediateSurface->GetContainer(__uuidof(IDirect3DTexture9), (void **)&pTexture);
		}
	}

	// if pTexture is available, video has been post-processed and converted to a texture from raw YUV plane.
	if (pTexture)
	{
		CComPtr<IDirect3DPixelShader9> pOrigPixelShader;
		PlaneVB<D3D9VideoMixerVertex> rectVB;

		AdjustVertexBuffer(plane, src, pTexture, dst, rcDst, rectVB);

		if (plane.fAlpha < 1.f)
		{
			hr = m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			hr = m_pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
			hr = m_pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
		}

		if (m_LumaKey.bEnable && m_pLumaKeyPS)
		{
			// luma keying range should be inclusive, so we enlarge the range a little bit
			// before passing it to the shader.
			// we use this function to derive Y (luma)
			// float y = offset + dot(c.rgb, coef * float3(0.299, 0.587, 0.114))
			float fConstData[4] = {
				float(m_LumaKey.uLower) - 0.1f, float(m_LumaKey.uUpper) + 0.1f,
				0, 0
			};
			hr = m_pDevice->SetPixelShaderConstantF(0, fConstData, 1);
			hr = m_pDevice->GetPixelShader(&pOrigPixelShader);
			hr = m_pDevice->SetPixelShader(m_pLumaKeyPS);
		}

		hr = m_pDevice->SetTexture(0, pTexture);
		hr = m_pDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, &rectVB, sizeof(D3D9VideoMixerVertex));

		if (m_LumaKey.bEnable && m_pLumaKeyPS)
		{
			hr = m_pDevice->SetPixelShader(pOrigPixelShader);
		}

		if (plane.fAlpha < 1.f)
		{
			hr = m_pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		}

#ifdef DEBUG_TRANSFORM
		hr = m_pDevice->SetFVF(D3DFVF_XYZ);
		hr = m_pDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
		hr = m_pDevice->SetTexture(0, NULL);
		hr = m_pDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, &rectVB, sizeof(D3D9VideoMixerVertex));
#endif

		if (0)
		{
#ifndef _NO_USE_D3DXDLL
			hr = D3DXSaveTextureToFile(_T("C:\\do.bmp"), D3DXIFF_BMP, pTexture, NULL);
#endif
		}
	}
	else if (pOffscreenSurface)
	{
		hr = plane.pVBlt->DirectVBlt(pOffscreenSurface, src, pDstSurface, dst);
	}
	return hr;
}

HRESULT CD3D9VideoMixer::DrawTexturePlane(PLANE_ID PlaneID, const RECT &rcDst, const RECT &rcDstClip)
{
	const D3D9Plane &plane = m_Planes[PlaneID];
	RECT src = plane.rcSrc, dst = (plane.bFullScreenMixing && m_eWindowState == RESOURCE_WINDOW_STATE_FULLSCREEN) ? rcDstClip : rcDst;

	if (!IsD3D9PlaneValid(&plane) || FAILED(PlaneToScreen(plane, src, dst, rcDstClip)))
		return S_FALSE;

	CComPtr<IDirect3DPixelShader9> pOrigPixelShader;
	CComPtr<IDirect3DTexture9> pTexture;
	bool bUseL8toArgbPS = (plane.Format == PLANE_FORMAT_P8) && m_pL8toArgbPS;
	HRESULT hr;

	hr = m_pTexturePool->GetRepresentation(plane.hTexture, __uuidof(IDirect3DTexture9), (void **)&pTexture);
	if (FAILED(hr))
		return hr;

	PlaneVB<D3D9VideoMixerVertex> rectVB;
	AdjustVertexBuffer(plane, src, pTexture, dst, rcDst, rectVB);

	hr = m_pDevice->SetTexture(0, pTexture);

	if (bUseL8toArgbPS)
	{
		CComPtr<IDirect3DTexture9> pPaletteTexture;
		hr = m_pTexturePool->GetRepresentation(plane.hTexture, __uuidof(IDirect3DTexturePalette9), (void **)&pPaletteTexture);
		if (pPaletteTexture)
		{
			hr = m_pDevice->GetPixelShader(&pOrigPixelShader);

			float dx = 1.f / plane.uWidth;
			float dy = 1.f / plane.uHeight;

			// tx0 is the left top corner point, tx1 is right top.
			for (int i = 0; i < 4; i++)
			{
				D3D9VideoMixerVertex &v = rectVB.VB[i];
				v.tx[1].u = v.tx[0].u + dx;
				v.tx[1].v = v.tx[0].v;
				v.tx[2].u = v.tx[0].u;
				v.tx[2].v = v.tx[0].v + dy;
				v.tx[3].u = v.tx[0].u + dx;
				v.tx[3].v = v.tx[0].v + dy;
			}
			// weight
			rectVB.LT.tx[4].u = rectVB.LB.tx[4].u = rectVB.RT.tx[4].v = rectVB.LT.tx[4].v = 0;
			rectVB.RT.tx[4].u = rectVB.RB.tx[4].u = float(plane.uWidth);
			rectVB.LB.tx[4].v = rectVB.RB.tx[4].v = float(plane.uHeight);

			for (int i = 0; i < 2; i++)
			{
				hr = m_pDevice->SetSamplerState(i, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
				hr = m_pDevice->SetSamplerState(i, D3DSAMP_MINFILTER, D3DTEXF_POINT);
				hr = m_pDevice->SetSamplerState(i, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
			}

			hr = m_pDevice->SetTexture(1, pPaletteTexture);
			hr = m_pDevice->SetPixelShader(m_pL8toArgbPS);
			hr = m_pDevice->SetFVF(D3D9VideoMixerVertex::FVF5);
		}
	}

	if (plane.bPartialBlending)
	{
		// D = S + (1 - A) * D
		hr = m_pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
		hr = m_pDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, &rectVB, sizeof(D3D9VideoMixerVertex));
		hr = m_pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	}
	else
	{
		hr = m_pDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, &rectVB, sizeof(D3D9VideoMixerVertex));
	}

	if (bUseL8toArgbPS)
	{
		hr = m_pDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
		hr = m_pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		hr = m_pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		hr = m_pDevice->SetTexture(1, NULL);
		hr = m_pDevice->SetPixelShader(pOrigPixelShader);
		hr = m_pDevice->SetFVF(D3D9VideoMixerVertex::FVF);
	}

#ifdef DEBUG_TRANSFORM
	hr = m_pDevice->SetFVF(D3DFVF_XYZ);
	hr = m_pDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
	hr = m_pDevice->SetTexture(0, NULL);
	hr = m_pDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, &rectVB, sizeof(D3D9VideoMixerVertex));
#endif

	if (0)
	{
#ifndef _NO_USE_D3DXDLL
		hr = D3DXSaveTextureToFile(_T("C:\\do.bmp"), D3DXIFF_BMP, pTexture, NULL);
#endif
	}
	return hr;
}

void CD3D9VideoMixer::AdjustVertexBuffer(
	const D3D9Plane &plane,
	const RECT &rcSrc,
	IDirect3DTexture9 *pTexture,
	const RECT &rcDst,
	const RECT &rcDstClip,
	PlaneVB<D3D9VideoMixerVertex> &rect)
{
	const UCHAR ucAlpha = static_cast<UCHAR> (0xff * plane.fAlpha);
	const D3DCOLOR color = D3DCOLOR_ARGB(ucAlpha, 0xff, 0xff, 0xff);
	NORMALIZEDRECT nrcTexture;
	NORMALIZEDRECT nrcOutput;
	const float z = 0.5f;

	NormalizeRect(nrcOutput, rcDst, rcDstClip);

	D3DSURFACE_DESC desc;
	if (FAILED(pTexture->GetLevelDesc(0, &desc)))
	{
		desc.Width = plane.uWidth;
		desc.Height = plane.uHeight;
	}

	// nrcTexture is the normalized source rectangle.
	nrcTexture.left = FLOAT(rcSrc.left) / desc.Width;
	nrcTexture.top = FLOAT(rcSrc.top) / desc.Height;
	nrcTexture.right = FLOAT(rcSrc.right) / desc.Width;
	nrcTexture.bottom = FLOAT(rcSrc.bottom) / desc.Height;

	rect.LT.position.x = nrcOutput.left;
	rect.LT.position.y = nrcOutput.top;
	rect.LT.position.z = z;
	rect.LT.color = color;
	rect.LT.tx[0].u = nrcTexture.left;
	rect.LT.tx[0].v = nrcTexture.top;

	rect.LB.position.x = nrcOutput.left;
	rect.LB.position.y = nrcOutput.bottom;
	rect.LB.position.z = z;
	rect.LB.color = color;
	rect.LB.tx[0].u = nrcTexture.left;
	rect.LB.tx[0].v = nrcTexture.bottom;

	rect.RB.position.x = nrcOutput.right;
	rect.RB.position.y = nrcOutput.bottom;
	rect.RB.position.z = z;
	rect.RB.color = color;
	rect.RB.tx[0].u = nrcTexture.right;
	rect.RB.tx[0].v = nrcTexture.bottom;

	rect.RT.position.x = nrcOutput.right;
	rect.RT.position.y = nrcOutput.top;
	rect.RT.position.z = z;
	rect.RT.color = color;
	rect.RT.tx[0].u = nrcTexture.right;
	rect.RT.tx[0].v = nrcTexture.top;
}

HRESULT CD3D9VideoMixer::LoadPixelShader()
{
	HRESULT hr;

	ASSERT(m_pLumaKeyPS == 0 && m_pL8toArgbPS == 0);

	// if nomial range is 16 - 235, we use the offset 0, scale 255 luma key shader.
	if (m_bNomialRange16_235)
		hr = m_pDevice->CreatePixelShader(g_ps20_LumaKeying_0_255_PS, &m_pLumaKeyPS);
	else
		hr = m_pDevice->CreatePixelShader(g_ps20_LumaKeying_16_235_PS, &m_pLumaKeyPS);
	ASSERT(SUCCEEDED(hr));

	hr = m_pDevice->CreatePixelShader(g_ps20_ConvertL8toP8_AA_PS, &m_pL8toArgbPS);
	ASSERT(SUCCEEDED(hr));

	return hr;
}

HRESULT CD3D9VideoMixer::CheckStretchRectFormula()
{
	CComPtr<IDirect3DSurface9> pYuvSurface, pRgbSurface;
	HRESULT hr;
	D3DLOCKED_RECT d3dLockRect;

	hr = m_pDevice->CreateOffscreenPlainSurface(720, 480, D3DFMT_YUY2, D3DPOOL_DEFAULT, &pYuvSurface, NULL);
	hr = pYuvSurface->LockRect(&d3dLockRect, NULL, 0);
	if (SUCCEEDED(hr))
	{
		unsigned char *p = static_cast<unsigned char *> (d3dLockRect.pBits);
		*p++ = unsigned char(16);
		*p++ = unsigned char(128);
		*p++ = unsigned char(16);
		*p++ = unsigned char(235);
		hr = pYuvSurface->UnlockRect();
	}

	hr = m_pDevice->CreateOffscreenPlainSurface(720, 480, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &pRgbSurface, NULL);
	hr = m_pDevice->StretchRect(pYuvSurface, NULL, pRgbSurface, NULL, D3DTEXF_POINT);
	if (FAILED(hr))
		return hr;

	hr = pRgbSurface->LockRect(&d3dLockRect, NULL, D3DLOCK_READONLY);
	if (SUCCEEDED(hr))
	{
		unsigned char *p = static_cast<unsigned char *> (d3dLockRect.pBits);
		if (p[0] < 16)
			m_bNomialRange16_235 = false;
		hr = pRgbSurface->UnlockRect();
	}
	return hr;
}

HRESULT CD3D9VideoMixer::SetupClearRectStage()
{
	HRESULT hr = S_OK;

	if (m_MainCRList.empty() && m_SubCRList.empty())
		return hr;

	hr = m_pDevice->SetRenderState(D3DRS_STENCILENABLE, TRUE);
	// prepare stencil buffer for clear rectangles
	hr = m_pDevice->Clear(0, NULL, D3DCLEAR_STENCIL, 0, 0, 0);

	hr = m_pDevice->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
	hr = m_pDevice->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_REPLACE);
	hr = m_pDevice->SetRenderState(D3DRS_STENCILMASK, 0xffffffff);
	hr = m_pDevice->SetRenderState(D3DRS_STENCILWRITEMASK, 0xffffffff);
	hr = m_pDevice->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_GREATER);

	// turn off color buffer
	hr = m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	hr = m_pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
	hr = m_pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

	hr = m_pDevice->SetFVF(D3D9ClearRectVertex::FVF);

	ClearRectVBList::const_iterator i;

	hr = m_pDevice->SetRenderState(D3DRS_STENCILREF, 0x2);
	for (i = m_MainCRList.begin(); i != m_MainCRList.end(); ++i)
		hr = m_pDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, &*i, sizeof(D3D9ClearRectVertex));

	hr = m_pDevice->SetRenderState(D3DRS_STENCILREF, 0x1);
	for (i = m_SubCRList.begin(); i != m_SubCRList.end(); ++i)
		hr = m_pDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, &*i, sizeof(D3D9ClearRectVertex));

	hr = m_pDevice->SetFVF(D3D9VideoMixerVertex::FVF);
	// disable writing to stencil buffer.
	hr = m_pDevice->SetRenderState(D3DRS_STENCILWRITEMASK, 0x00);

	return hr;
}
