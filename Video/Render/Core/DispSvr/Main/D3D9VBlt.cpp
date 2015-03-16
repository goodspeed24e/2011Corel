#include "stdafx.h"
#include "DispSvr.h"
#include "ResourceManager.h"
#include "DynLibManager.h"
#include "MathVideoMixing.h"
#include "D3D9VideoMixerBase.h"
#include "DXVA2VideoProcessor.h"

using namespace DispSvr;

ID3D9VBlt *CD3D9VBlt::Create(D3D9Plane *pPlane, VideoProcessorStub *pVPStub, CD3D9TexturePool *pPool)
{
	return new CD3D9VBlt(pPlane, pVPStub, pPool);
}

CD3D9VBlt::CD3D9VBlt(D3D9Plane *pPlane, VideoProcessorStub *pVPStub, CD3D9TexturePool *pPool)
	: m_pPlane(pPlane), m_pVPStub(pVPStub), m_pTexturePool(pPool),
	m_hIVPBTexture(0), m_uIVPBWidth(0), m_uIVPBHeight(0),
	m_IVPBFormat(PLANE_FORMAT_UNKNOWN), m_bUsageAsTexture(false)
{
}

CD3D9VBlt::~CD3D9VBlt()
{
	if (m_hIVPBTexture)
	{
		m_pTexturePool->ReleaseTexture(m_hIVPBTexture);
		m_hIVPBTexture = 0;
	}
}

HRESULT CD3D9VBlt::IntermediateTextureVBlt(IDirect3DSurface9 *pSrc, const RECT &rcSrc, IDirect3DTexture9 **pIntermediate)
{
	IDirect3DSurface9 *pIntermediateSurface = 0;
	// ARGB is usually one of the format to create render target texture.
	HRESULT hr = IntermediateVBlt(pSrc, rcSrc, PLANE_FORMAT_ARGB, &pIntermediateSurface, true);
	if (SUCCEEDED(hr))
	{
		hr = pIntermediateSurface->GetContainer(IID_IDirect3DTexture9, (void **) pIntermediate);
		pIntermediateSurface->Release();
	}
	return hr;
}

HRESULT CD3D9VBlt::IntermediateVBlt(IDirect3DSurface9 *pSrc, const RECT &rcSrc, PLANE_FORMAT fmtInter, IDirect3DSurface9 **pIntermediate)
{
	return IntermediateVBlt(pSrc, rcSrc, fmtInter, pIntermediate, false);
}

HRESULT CD3D9VBlt::IntermediateVBlt(IDirect3DSurface9 *pSrc, const RECT &rcSrc, PLANE_FORMAT fmtInter, IDirect3DSurface9 **pIntermediate, bool bTexture)
{
	HRESULT hr = S_OK;
	UINT uWidth = rcSrc.right - rcSrc.left;
	UINT uHeight = rcSrc.bottom - rcSrc.top;

	ASSERT(rcSrc.right > rcSrc.left && rcSrc.bottom > rcSrc.top);

	*pIntermediate = 0;
	if (m_hIVPBTexture)
	{
		if (m_uIVPBWidth != uWidth || m_uIVPBHeight != uHeight || m_IVPBFormat != fmtInter || m_bUsageAsTexture != bTexture)
		{
			hr = m_pTexturePool->ReleaseTexture(m_hIVPBTexture);
			m_hIVPBTexture = 0;
		}
	}

	if (!m_hIVPBTexture)
	{
		CreateTextureParam param = {0};
		param.uWidth = uWidth;
		param.uHeight = uHeight;
		param.Format = fmtInter;
		param.eUsage = bTexture ? TEXTURE_USAGE_RENDERTARGET_TEXTURE : TEXTURE_USAGE_RENDERTARGET;
		hr = m_pTexturePool->CreateTexture(&param, &m_hIVPBTexture);
		if (FAILED(hr))
			return hr;
		m_uIVPBWidth = uWidth;
		m_uIVPBHeight = uHeight;
		m_IVPBFormat = fmtInter;
		m_bUsageAsTexture = bTexture;
	}

	if (SUCCEEDED(hr))
	{
		ASSERT(m_hIVPBTexture);

		hr = m_pTexturePool->GetRepresentation(m_hIVPBTexture, __uuidof(IDirect3DSurface9), (void **)pIntermediate);
		if (SUCCEEDED(hr))
		{
			RECT dst = {0, 0, uWidth, uHeight};
            if (m_pPlane && (m_pPlane->VideoProperty.Format == VIDEO_FORMAT_PROGRESSIVE))
            {
                hr = CD3D9VBlt::DirectVBlt(pSrc, rcSrc, *pIntermediate, dst);
            }
            else
            {
			    hr = DirectVBlt(pSrc, rcSrc, *pIntermediate, dst);
            }
		}
	}

	return hr;
}

HRESULT CD3D9VBlt::DirectVBlt(IDirect3DSurface9 *pSrc, const RECT &rcSrc, IDirect3DSurface9 *pDst, const RECT &rcDst)
{
	RECT src = rcSrc;
	CComPtr<IDirect3DDevice9> pDevice;

	HRESULT hr = pSrc->GetDevice(&pDevice);
	if (SUCCEEDED(hr))
	{
		// source rectangle for StretchRect must be even.
		RoundUpTo2Rect(src);
		hr = pDevice->StretchRect(pSrc, &src, pDst, &rcDst, D3DTEXF_LINEAR);
	}
	return hr;
}

/////////////////////////////////////////////////////////////////////////////////////////////

ID3D9VBlt *CDXVA2VBlt::Create(D3D9Plane *pPlane, VideoProcessorStub *pVPStub, CD3D9TexturePool *pPool)
{
	if (CDynLibManager::GetInstance()->pfnDXVA2CreateVideoService)
		return new CDXVA2VBlt(pPlane, pVPStub, pPool);
	return NULL;
}

CDXVA2VBlt::CDXVA2VBlt(D3D9Plane *pPlane, VideoProcessorStub *pVPStub, CD3D9TexturePool *pPool)
: CD3D9VBlt(pPlane, pVPStub, pPool), m_pRefSamples(NULL)
{
	m_pVP = new CDXVA2VideoProcessor(pVPStub->guidVP, pVPStub->RenderTargetFormat);
    
    // number of samples = number of forward ref samples + number of backward ref samples + plane sample
    const UINT uiNumSamples = m_pVPStub->sCaps.uNumBackwardSamples + m_pVPStub->sCaps.uNumForwardSamples + 1;
    if (uiNumSamples > 0)
        m_pRefSamples = new DXVA2VP_RefSample[uiNumSamples];
}

CDXVA2VBlt::~CDXVA2VBlt()
{
	delete m_pVP;
	if (m_pRefSamples)
		delete [] m_pRefSamples;
}

// note: on MSDN, we should provide DXVA2_SampleUnknown samples to fill the number of reference samples required
// even if we currently have less than required number. However, it seems ok not to comply with the documentation
// by sending the samples we have only.
HRESULT CDXVA2VBlt::DirectVBlt(IDirect3DSurface9 *pSrc, const RECT &rcSrc, IDirect3DSurface9 *pDst, const RECT &rcDst)
{
	HRESULT hr = E_FAIL;
	UINT nRefSamples = 0;
    // number of samples = number of forward ref samples + number of backward ref samples + plane sample
    const UINT &uiNumVideoSamples = m_pVPStub->sCaps.uNumBackwardSamples + m_pVPStub->sCaps.uNumForwardSamples + 1;
    VideoSampleList::const_reverse_iterator it = m_pPlane->VideoSamples.rbegin();

    if (m_pPlane->VideoSamples.size() > uiNumVideoSamples)
        it +=  m_pPlane->VideoSamples.size() - uiNumVideoSamples;

    for (; nRefSamples < uiNumVideoSamples && it != m_pPlane->VideoSamples.rend(); ++it)
	{
		if (SUCCEEDED(m_pTexturePool->GetRepresentation(it->hTexture, IID_IDirect3DSurface9,
			(void **) &m_pRefSamples[nRefSamples].pSurface)))
		{
			m_pRefSamples[nRefSamples].pSurface->Release();
			m_pRefSamples[nRefSamples].VideoProperty = it->VideoProperty;
			nRefSamples++;
		}
	}

    ASSERT(nRefSamples <= uiNumVideoSamples);

	hr = m_pVP->VideoProcessBlt(pSrc, rcSrc, pDst, rcDst, m_pPlane->VideoProperty, m_pRefSamples, nRefSamples, m_pVPStub->fFilterValue);
	return hr;
}
