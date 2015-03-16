#include "stdafx.h"
#include "DXVA2VideoProcessor.h"
#include "StereoVideoMixer.h"
#include "Shaders/MergeSxSAnaglyph_ps.h"
#include "Shaders/MergeSxSHalfColorAnaglyph_ps.h"
#include "Shaders/MergeSxSHalfColor2Anaglyph_ps.h"
#include "Shaders/MergeSxSOptimizedAnaglyph_ps.h"
#include "Shaders/RowInterleavedStereo_ps.h"
#include "Shaders/ColumnInterleavedStereo_ps.h"
#include "Shaders/CheckerboardStereo_ps.h"
#include "Shaders/CheckerboardStereo_vs.h"
#include "Imports/ThirdParty/nVIDIA/NvAPI/nvStereoVideo.h"

using namespace DispSvr;

struct AnaglyphVertex
{
	D3DVECTOR position;
	FLOAT u, v;
	enum { FVF = D3DFVF_XYZ | D3DFVF_TEX1 };
};

CStereoVideoMixer::CStereoVideoMixer() :
	m_matrixWorldTranspose(
		2, 0, 0, -1,
		0, -2, 0, 1,
		0, 0, 1, 0,
		0, 0, 0, 1
	)
{
	m_GUID = DISPSVR_RESOURCE_STEREOVIDEOMIXER;
    // When stereo mixer is used, we may select DXVAHDVP with D3D9+DXVA2 implementation.
    m_VPInitParam.bUseNativeDXVAHDOnly = false;
	m_MixerCaps.dwFlags &= ~(MIXER_CAP_3D_RENDERTARGET);

	m_hSxSRT = NULL;
	m_pStereoVP = NULL;
	m_pViewHDVP[0] = m_pViewHDVP[1] = NULL;
	m_pVPStub = NULL;
	m_pMergeAnaglyphPS = NULL;
	m_pCheckerboardPS = NULL;
    m_pRowInterleavedPS = NULL;
    m_pColumnInterleavedPS = NULL;
	m_pVS = NULL;
    m_bInShutterMode = FALSE;
    m_eCurrentStereoMode = MIXER_STEREO_MODE_DISABLED;
    m_pS3DAuxiliaryDeviceService = NULL;
 

	const AnaglyphVertex vb[4] = {
	//    x, y, z,   u, v 
		{ 0, 0, 0.5, 0, 0 },
		{ 0, 1, 0.5, 0, 1 },
		{ 1, 1, 0.5, 0.5, 1 },
		{ 1, 0, 0.5, 0.5, 0 }
	};

	m_pAnaglyphVertexBuffer = new BYTE[sizeof(vb)];
	memcpy(m_pAnaglyphVertexBuffer, &vb[0], sizeof(vb));

	m_pNvControlSurface = NULL;
	ReleaseResources();
}

CStereoVideoMixer::~CStereoVideoMixer()
{
	_ReleaseDevice();
	SAFE_DELETE_ARRAY(m_pAnaglyphVertexBuffer);
}

STDMETHODIMP CStereoVideoMixer::_ReleaseDevice()
{
    SAFE_DELETE(m_pS3DAuxiliaryDeviceService);
    m_bInShutterMode = FALSE;
	ReleaseResources();
	return CD3D9Dxva2HDVideoMixer::_ReleaseDevice();
}

STDMETHODIMP CStereoVideoMixer::_Execute(IDirect3DSurface9 *pDestSurface, const RECT &rcDst, const RECT &rcDstClip)
{
	HRESULT hr = E_FAIL;
	RECT rcView[2] = { rcDst, rcDst };
	RECT rcViewClip[2] = { rcDstClip, rcDstClip };
	CComPtr<IDirect3DSurface9> pSxSRT;
	MIXER_STEREO_MODE eStereoMode = m_Property.eStereoMode;

    if (m_eCurrentStereoMode != eStereoMode)
    {
        m_uRTWidth = NULL;
        m_uRTHeight = NULL; 
		m_eCurrentStereoMode = m_Property.bStereoEnable == TRUE ? eStereoMode : MIXER_STEREO_MODE_DISABLED;
    }

    if (eStereoMode == MIXER_STEREO_MODE_HDMI_STEREO)
    {
        DWORD dwVendor = GetRegistry(REG_VENDOR_ID, 0);
        DWORD dwCoprocVendor = GetRegistry(REG_COPROC_ACTIVE_VENDOR_ID, 0);
        if ((dwVendor == PCI_VENDOR_ID_NVIDIA)
            || ((dwVendor == PCI_VENDOR_ID_INTEL) && (dwCoprocVendor == PCI_VENDOR_ID_NVIDIA)))
        {
            ReleaseResources();
            return CD3D9Dxva2HDVideoMixer::_Execute(pDestSurface, rcDst, rcDstClip);
        }
        else if (dwVendor == PCI_VENDOR_ID_INTEL) //Use S3D
        {
            return IntelS3DOutput(pDestSurface, rcDst, rcDstClip);
        }
        else if (dwVendor == PCI_VENDOR_ID_ATI)
        {
        }
    }

	if ( !m_Property.bStereoEnable )
		eStereoMode = MIXER_STEREO_MODE_DISABLED;

	if (MIXER_STEREO_MODE_ANAGLYPH == eStereoMode)
		eStereoMode = MIXER_STEREO_MODE_HALFCOLOR2_ANAGLYPH;

    if (eStereoMode == MIXER_STEREO_MODE_COLOR_ANAGLYPH
		|| eStereoMode == MIXER_STEREO_MODE_NV_PRIVATE
		|| eStereoMode == MIXER_STEREO_MODE_SIDEBYSIDE
		|| eStereoMode == MIXER_STEREO_MODE_CHECKERBOARD
		|| eStereoMode == MIXER_STEREO_MODE_OPTIMIZED_ANAGLYPH
		|| eStereoMode == MIXER_STEREO_MODE_HALFCOLOR_ANAGLYPH
		|| eStereoMode == MIXER_STEREO_MODE_HALFCOLOR2_ANAGLYPH
        || eStereoMode == MIXER_STEREO_MODE_ROW_INTERLEAVED
        || eStereoMode == MIXER_STEREO_MODE_COLUMN_INTERLEAVED
		|| m_bInShutterMode)
    {
	    hr = CheckAndPrepareRenderTarget(eStereoMode, &pSxSRT, rcView[0], rcViewClip[0], rcView[1], rcViewClip[1]);
    }
    else
    {
        hr = E_FAIL;
    }

	if (FAILED(hr))
	{
		if (eStereoMode == MIXER_STEREO_MODE_NV_PRIVATE && pSxSRT)
		{
			EnableStereoInNvControlSurface(FALSE);
			NvStereoOutput(pSxSRT, pDestSurface, rcDstClip, rcDstClip);
		}
		ReleaseResources();
		return CD3D9Dxva2HDVideoMixer::_Execute(pDestSurface, rcDst, rcDstClip);
	}

	PlaneVPSample Sample[PLANE_MAX];

    UINT uSelectedViewid[2] = {0,1};
    if (eStereoMode == MIXER_STEREO_MODE_DISABLED && m_bInShutterMode)
    {
        uSelectedViewid[1] = 0;
    }

	bool pSkipStream[PLANE_MAX] = {0};

	// Background and sub-video can not co-exist, so we skip sub-video if background is visible, and vice versa.
	// This way we can use maximum 4 streams for BD and do not change stream states often.
	pSkipStream[PLANE_BACKGROUND] = !IsBackgroundVisible();
	pSkipStream[PLANE_SUBVIDEO] = !pSkipStream[PLANE_BACKGROUND];

	for (int vid = 0; vid < 2; vid++)
	{
		UINT uVPSampleCount = 0;
		ZeroMemory(Sample, sizeof(Sample));

		for (int planeID = 0; planeID < PLANE_MAX; planeID++)
		{
			if (pSkipStream[planeID])
				continue;

			hr = PreparePlaneVPSample(uSelectedViewid[vid], planeID, Sample[uVPSampleCount], rcView[vid], rcViewClip[vid]);
			uVPSampleCount++;
		}
		hr = ProcessViewDXVAHD(uSelectedViewid[vid], vid, pSxSRT, Sample, uVPSampleCount, rcView[vid], rcViewClip[vid]);
	}

	if (0)
	{
		DXToFile(pSxSRT);
	}

	hr = E_FAIL;
	switch (eStereoMode)
	{
	case MIXER_STEREO_MODE_NV_PRIVATE:
		hr = NvStereoOutput(pSxSRT, pDestSurface, rcDstClip, rcDstClip);
		break;

	case MIXER_STEREO_MODE_HALFCOLOR_ANAGLYPH:
	case MIXER_STEREO_MODE_OPTIMIZED_ANAGLYPH:
	case MIXER_STEREO_MODE_COLOR_ANAGLYPH:
		hr = MergeShaderOutput(m_pMergeAnaglyphPS, pSxSRT, pDestSurface, rcDst, rcDstClip);
		break;

	case MIXER_STEREO_MODE_HALFCOLOR2_ANAGLYPH:
		{
			float fPixelShaderConstantData[1] = { GetRegInt(TEXT("StereoMixerModeArg"), 150) / 100.f };
			hr = m_pDevice->SetPixelShaderConstantF(0, fPixelShaderConstantData, 1);
			hr = MergeShaderOutput(m_pMergeAnaglyphPS, pSxSRT, pDestSurface, rcDst, rcDstClip);
		}
		break;

	case MIXER_STEREO_MODE_CHECKERBOARD:
		hr = MergeShaderOutput(m_pCheckerboardPS, pSxSRT, pDestSurface, rcDst, rcDstClip);
		break;

    case MIXER_STEREO_MODE_SIDEBYSIDE:
        hr = E_FAIL; // use TestOutput function below.
        break;
    
    case MIXER_STEREO_MODE_ROW_INTERLEAVED:
        hr = MergeShaderOutput(m_pRowInterleavedPS, pSxSRT, pDestSurface, rcDst, rcDstClip);
        break;
    case MIXER_STEREO_MODE_COLUMN_INTERLEAVED:
        hr = MergeShaderOutput(m_pColumnInterleavedPS, pSxSRT, pDestSurface, rcDst, rcDstClip);
        break;
	default:
    case MIXER_STEREO_MODE_DISABLED:
        {
            if (m_bInShutterMode)
            {
                hr = NvStereoOutput(pSxSRT, pDestSurface, rcDstClip, rcDstClip);
                break;
            }
            else
                return CD3D9Dxva2HDVideoMixer::_Execute(pDestSurface, rcDst, rcDstClip);
        }
	}

	if (FAILED(hr))
		hr = TestOutput(pSxSRT, pDestSurface, rcDst, rcDstClip);

	return hr;
}

void CStereoVideoMixer::ReleaseResources()
{
	SAFE_RELEASE(m_pMergeAnaglyphPS);
	SAFE_RELEASE(m_pCheckerboardPS);
    SAFE_RELEASE(m_pRowInterleavedPS);
    SAFE_RELEASE(m_pColumnInterleavedPS);
	SAFE_RELEASE(m_pVS);
	if (m_hSxSRT)
	{
		m_pTexturePool->ReleaseTexture(m_hSxSRT);
		m_hSxSRT = NULL;
	}
	m_uRTWidth = m_uRTHeight = 0;
	SAFE_DELETE(m_pViewHDVP[0]);
	SAFE_DELETE(m_pViewHDVP[1]);
	SAFE_DELETE(m_pStereoVP);
	SAFE_RELEASE(m_pNvControlSurface);
}

HRESULT CStereoVideoMixer::CheckAndPrepareRenderTarget(MIXER_STEREO_MODE eMode, IDirect3DSurface9 **ppSxSRT, RECT &rcBaseView, RECT &rcBaseViewClip, RECT &rcDependentView, RECT &rcDependentViewClip)
{
	HRESULT hr = E_FAIL;

	if (IsRectEmpty(&rcBaseViewClip))
		return hr;

	UINT uPlaneWidth = rcBaseViewClip.right - rcBaseViewClip.left;
	UINT uPlaneHeight = rcBaseViewClip.bottom - rcBaseViewClip.top;
	UINT uWidth = uPlaneWidth * 2;		// render target is side by side
	UINT uHeight = uPlaneHeight;

	if (m_uRTWidth != uWidth || m_uRTHeight != uHeight)
	{
		if (m_hSxSRT)
			ReleaseResources();

		m_pVPStub = m_PlaneConfigs[PLANE_MAINVIDEO].pVideoProcessorStub;
		for (int i = 0; i < 2; i++)
		{
			ASSERT(m_pViewHDVP[i] == 0);
			hr = CDXVAHDVP::CreateVP(m_pDeviceEx, NULL, &m_pViewHDVP[i]);
			if (FAILED(hr))
				return hr;
		}

		CreateTextureParam param = {0};
		param.uWidth = uWidth;
		param.uHeight = uHeight;
		param.Format = PLANE_FORMAT_ARGB;
		param.eUsage = TEXTURE_USAGE_RENDERTARGET_TEXTURE;

		if (MIXER_STEREO_MODE_COLOR_ANAGLYPH == eMode || MIXER_STEREO_MODE_OPTIMIZED_ANAGLYPH == eMode
			|| MIXER_STEREO_MODE_HALFCOLOR_ANAGLYPH == eMode || MIXER_STEREO_MODE_HALFCOLOR2_ANAGLYPH == eMode)
		{
			if (!m_pMergeAnaglyphPS)
			{
				if (MIXER_STEREO_MODE_OPTIMIZED_ANAGLYPH == eMode)
					hr = m_pDevice->CreatePixelShader((DWORD *)g_ps30_MergeSxSOptimizedAnaglyph_PS, &m_pMergeAnaglyphPS);
				else if (MIXER_STEREO_MODE_HALFCOLOR_ANAGLYPH == eMode)
					hr = m_pDevice->CreatePixelShader((DWORD *)g_ps30_MergeSxSHalfColorAnaglyph_PS, &m_pMergeAnaglyphPS);
				else if (MIXER_STEREO_MODE_HALFCOLOR2_ANAGLYPH == eMode)
					hr = m_pDevice->CreatePixelShader((DWORD *)g_ps30_MergeSxSHalfColor2Anaglyph_PS, &m_pMergeAnaglyphPS);
				else
					hr = m_pDevice->CreatePixelShader((DWORD *)g_ps30_MergeSxSAnaglyph_PS, &m_pMergeAnaglyphPS);
			}
			if (!m_pVS)
			{
				hr = m_pDevice->CreateVertexShader((DWORD *)g_vs30_MergeSxSCheckerboard_VS, &m_pVS);
			}
		}
		else if (MIXER_STEREO_MODE_CHECKERBOARD == eMode)
		{
			if (!m_pCheckerboardPS)
			{
				hr = m_pDevice->CreatePixelShader((DWORD *)g_ps30_MergeSxSCheckerboard_PS, &m_pCheckerboardPS);
			}
			if (!m_pVS)
			{
				hr = m_pDevice->CreateVertexShader((DWORD *)g_vs30_MergeSxSCheckerboard_VS, &m_pVS);
			}
		}
        else if (MIXER_STEREO_MODE_ROW_INTERLEAVED == eMode)
        {
            if (!m_pRowInterleavedPS)
            {
                hr = m_pDevice->CreatePixelShader((DWORD *)g_ps30_MergeSxSRowInterleaved_PS, &m_pRowInterleavedPS);
            }
            if (!m_pVS)
            {
                hr = m_pDevice->CreateVertexShader((DWORD *)g_vs30_MergeSxSCheckerboard_VS, &m_pVS);
            }
        }
        else if (MIXER_STEREO_MODE_COLUMN_INTERLEAVED == eMode)
        {
            if (!m_pColumnInterleavedPS)
            {
                hr = m_pDevice->CreatePixelShader((DWORD *)g_ps30_MergeSxSColumnInterleaved_PS, &m_pColumnInterleavedPS);
            }
            if (!m_pVS)
            {
                hr = m_pDevice->CreateVertexShader((DWORD *)g_vs30_MergeSxSCheckerboard_VS, &m_pVS);
            }
        }
		else
		{
			if (m_pViewHDVP[0]->IsNativeDXVAHD())
			{
				param.Format = PLANE_FORMAT_NV12;
				param.eUsage = TEXTURE_USAGE_RENDERTARGET;
			}
		}
		hr = m_pTexturePool->CreateTexture(&param, &m_hSxSRT);
		if (FAILED(hr))
			return hr;

		m_uRTWidth = uWidth;
		m_uRTHeight = uHeight;

		if (MIXER_STEREO_MODE_NV_PRIVATE == eMode)
		{
			m_pStereoVP = new CDXVA2VideoProcessor(m_pVPStub->guidVP, m_pVPStub->RenderTargetFormat);
			if (m_pStereoVP == 0)
				return E_FAIL;
		}
	}

	if (m_hSxSRT)
	{
		hr = m_pTexturePool->GetRepresentation(m_hSxSRT, IID_IDirect3DSurface9, (void **)ppSxSRT);
		OffsetRect(&rcDependentView, uPlaneWidth, 0);
		SetRect(&rcBaseViewClip, 0, 0, uPlaneWidth, uHeight);
		SetRect(&rcDependentViewClip, uPlaneWidth, 0, uWidth, uHeight);
	}

	return hr;
}

HRESULT CStereoVideoMixer::PreparePlaneVPSample(UINT uViewID, PLANE_ID PlaneID, PlaneVPSample &Sample, const RECT &rcDst, const RECT &rcDstClip)
{
	D3D9Plane &plane = m_pModel->GetPlane(PlaneID);
	HRESULT hr = E_FAIL;

	Sample.PlaneID = PlaneID;
	Sample.rcSrc = plane.rcSrc;
	if (plane.bFullScreenMixing && m_eWindowState == RESOURCE_WINDOW_STATE_FULLSCREEN)
		Sample.rcDst = rcDstClip;
	else
		Sample.rcDst = rcDst;
    if (!plane.IsValidView(uViewID))
		return hr;

    if(PlaneID == PLANE_MAINVIDEO)
    {
        switch(plane.eStereoStreamMode)
        {
        case STEREO_STREAM_SIDEBYSIDE_LR:
            {
                UINT nWidth = Sample.rcSrc.right - Sample.rcSrc.left;
                if (uViewID == 0)
                    Sample.rcSrc.right = Sample.rcSrc.left + nWidth / 2;
                else //if (uViewID == 1)
                    Sample.rcSrc.left = Sample.rcSrc.right - nWidth / 2;
            }
            break;
        case STEREO_STREAM_SIDEBYSIDE_RL:
            {
                UINT nWidth = Sample.rcSrc.right - Sample.rcSrc.left;
                if (uViewID == 0)
                    Sample.rcSrc.left = Sample.rcSrc.right - nWidth / 2;
                else //if (uViewID == 1)
                    Sample.rcSrc.right = Sample.rcSrc.left + nWidth / 2;
            }
            break;
        case STEREO_STREAM_TOPBOTTOM_LR:
            {
                UINT nHeight = Sample.rcSrc.bottom - Sample.rcSrc.top;
                if (uViewID == 0)
                    Sample.rcSrc.bottom = Sample.rcSrc.top + nHeight / 2;
                else //if (uViewID == 1)
                    Sample.rcSrc.top = Sample.rcSrc.bottom - nHeight / 2;
            }
            break;
        case STEREO_STREAM_TOPBOTTOM_RL:
            {
                UINT nHeight = Sample.rcSrc.bottom - Sample.rcSrc.top;
                if (uViewID == 0)
                    Sample.rcSrc.top = Sample.rcSrc.bottom - nHeight / 2;
                else //if (uViewID == 1)
                    Sample.rcSrc.bottom = Sample.rcSrc.top + nHeight / 2;
            }
            break;
        default:
            break;
        }
    }

	hr = m_pTexturePool->GetRepresentation(plane.GetViewTextureHandle(uViewID), IID_IDirect3DSurface9, (void **)&Sample.pSurface);
	if (SUCCEEDED(hr))
	{
		Sample.pSurface->Release();
	}
	return hr;
}

HRESULT CStereoVideoMixer::ProcessViewDXVAHD(UINT uViewID, UINT uVPID, IDirect3DSurface9 *pDst, PlaneVPSample *pSubSamples, UINT uNumSubSamples, const RECT &rcDst, const RECT &rcDstClip)
{
	HRESULT hr = S_OK;
	UINT uStreamID = 0;
	DXVAHDVP_StreamState state;
	DXVAHDVP_BltState bltState;

	ZeroMemory(&bltState, sizeof(bltState));

	for (UINT i = 0; i < uNumSubSamples; i++)
	{
		const PlaneVPSample &sample = pSubSamples[i];
		const D3D9Plane &plane = m_pModel->GetPlane(sample.PlaneID);
		ZeroMemory(&state, sizeof(state));

		state.dwLastUpdateTS = plane.dwLastUpdateTS;
		state.SourceRect = sample.rcSrc;
		state.DestinationRect = sample.rcDst;

		state.pInputSurface = sample.pSurface;

		if (sample.PlaneID == PLANE_MAINVIDEO || sample.PlaneID == PLANE_SUBVIDEO)
			hr = SetSingleVideoStreamStates(sample.PlaneID, state, uViewID);
		else
			hr = SetSingleStreamStates(sample.PlaneID, state);

		hr |= StereoPlaneToScreen(uViewID, plane, state.SourceRect, state.DestinationRect, rcDstClip);
		if (SUCCEEDED(hr))
			state.bEnable = TRUE;

		hr = m_pViewHDVP[uVPID]->SetStreamState(uStreamID, state);
		uStreamID++;
	}

	bltState.TargetRect = rcDstClip;

//	bltState.OutputColorSpace.bVideoProcessing = 0;
//	bltState.OutputColorSpace.bLimitedRange = 0;
	if (GetRegistry(REG_DISPLAY_XVYCC_MONITOR_TYPE, DISPLAY_XVYCC_MONITOR_NOT_SUPPORT) == DISPLAY_XVYCC_MONITOR_BT709)
		bltState.OutputColorSpace.bBT709 = 1;
	if (GetRegistry(REG_DISPLAY_XVYCC_GAMUT, FALSE))
		bltState.OutputColorSpace.bXvYCC = 1;

	hr = m_pViewHDVP[uVPID]->VBltHD(pDst, bltState);
	return hr;
}

HRESULT CStereoVideoMixer::TestOutput(IDirect3DSurface9 *pSxSRT, IDirect3DSurface9 *pDestSurface, const RECT &rcDst, const RECT &rcDstClip)
{
	HRESULT hr = m_pDevice->StretchRect(pSxSRT, NULL, pDestSurface, &rcDst, D3DTEXF_POINT);

	return hr;
}

HRESULT CStereoVideoMixer::MergeShaderOutput(IDirect3DPixelShader9 *pShader, IDirect3DSurface9 *pSxSRT, IDirect3DSurface9 *pDestSurface, const RECT &rcDst, const RECT &rcDstClip)
{
	CHECK_POINTER(pShader);

	CComPtr<IDirect3DTexture9> pSxSTexture;
	HRESULT hr = pSxSRT->GetContainer(IID_IDirect3DTexture9, (void **) &pSxSTexture);

	if (FAILED(hr))
		return hr;

	CComPtr<IDirect3DPixelShader9> pOriginalPS;
	CComPtr<IDirect3DVertexShader9> pOriginalVS;

	hr = m_pDevice->SetVertexShaderConstantF(0, (CONST FLOAT *)&m_matrixWorldTranspose, 4);

	hr = m_pDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	hr = m_pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

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
	
	hr = m_pDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
	hr = m_pDevice->GetPixelShader(&pOriginalPS);
	hr = m_pDevice->GetVertexShader(&pOriginalVS);

	hr = m_pDevice->SetFVF(AnaglyphVertex::FVF);
	hr = m_pDevice->SetPixelShader(pShader);
	hr = m_pDevice->SetVertexShader(m_pVS);
	hr = m_pDevice->SetTexture(0, pSxSTexture);

	hr = m_pDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, m_pAnaglyphVertexBuffer, sizeof(AnaglyphVertex));

	hr = m_pDevice->SetPixelShader(pOriginalPS);
	hr = m_pDevice->SetVertexShader(pOriginalVS);
	return hr;
}

HRESULT CStereoVideoMixer::NvStereoOutput(IDirect3DSurface9 *pSxSRT, IDirect3DSurface9 *pDestSurface, const RECT &rcDst, const RECT &rcDstClip)
{
	CHECK_POINTER(pSxSRT);
	CHECK_POINTER(pDestSurface);

	HRESULT hr = E_FAIL;
	D3D9Plane &plane = m_pModel->GetPlane(PLANE_MAINVIDEO);
	UINT nRefSamples = 0;
	const UINT uiMaxRefSamples = 2;
	DXVA2VP_RefSample *pRefSamples = new DXVA2VP_RefSample[uiMaxRefSamples];	// input SxS image + control surface

	ZeroMemory(pRefSamples, sizeof(DXVA2VP_RefSample) * uiMaxRefSamples);

	pRefSamples[nRefSamples].pSurface = pSxSRT;
	pRefSamples[nRefSamples].VideoProperty = plane.VideoProperty;
	pRefSamples[nRefSamples].VideoProperty.Format = VIDEO_FORMAT_PROGRESSIVE;
	nRefSamples++;

	if (!m_pNvControlSurface)
	{
		CComPtr<IDirectXVideoAccelerationService> pAccelService;

		hr = CDynLibManager::GetInstance()->pfnDXVA2CreateVideoService(m_pDevice,__uuidof(IDirectXVideoAccelerationService), (VOID**)&pAccelService);
		if (FAILED(hr))
			return hr;

		hr = pAccelService->CreateSurface(
			NV_STEREO_VIDEO_CONTROL_SURFACE_WIDTH,
			NV_STEREO_VIDEO_CONTROL_SURFACE_HEIGHT,
			0,
			NV_STEREO_VIDEO_CONTROL_SURFACE_FORMAT,
			D3DPOOL_DEFAULT,		// Default pool.
			0,
			DXVA2_VideoProcessorRenderTarget,
			&m_pNvControlSurface,
			NULL);
		if (FAILED(hr))
			return hr;

		EnableStereoInNvControlSurface(TRUE);
	}

	pRefSamples[nRefSamples].eType = DXVA2VP_REFSAMPLE_TYPE_GRAPHICS;
	pRefSamples[nRefSamples].pSurface = m_pNvControlSurface;
	SetRect(&pRefSamples[nRefSamples].GraphicsProperty.rcSrc, 0, 0, NV_STEREO_VIDEO_CONTROL_SURFACE_WIDTH, NV_STEREO_VIDEO_CONTROL_SURFACE_HEIGHT);
	pRefSamples[nRefSamples].GraphicsProperty.rcDst = rcDst;
	nRefSamples++;

	ASSERT(m_pVPStub->pDelegateVPStub);
	RECT rcSrc = { 0, 0, m_uRTWidth, m_uRTHeight };
	hr = m_pStereoVP->VideoProcessBlt(pSxSRT, rcSrc,
		pDestSurface, rcDst,
		plane.VideoProperty, pRefSamples, nRefSamples,
		m_pVPStub->pDelegateVPStub->fFilterValue);

	SAFE_DELETE_ARRAY(pRefSamples);
    if (SUCCEEDED(hr))
    {
        m_bInShutterMode = TRUE;
    }
	return hr;
}

void CStereoVideoMixer::EnableStereoInNvControlSurface(BOOL bEnable)
{
	D3DLOCKED_RECT ddsd;
	if (m_pNvControlSurface && m_pNvControlSurface->LockRect(&ddsd, NULL, D3DLOCK_NOSYSLOCK) == S_OK)
	{
		LPBYTE pDst = (LPBYTE)ddsd.pBits;
		NV_VIDEO_PROCESS_PRIVATE_DATA VPData;

		NV_STEREO_VIDEO_DATA sStereoControlData;

		ZeroMemory(&sStereoControlData, sizeof(sStereoControlData));
		sStereoControlData.version = NV_STEREO_VIDEO_VERSION;
		sStereoControlData.filtering = NV_STEREO_VIDEO_FILTERING_DEFAULT;
		sStereoControlData.format = NV_STEREO_VIDEO_FORMAT_DEFAULT;
		sStereoControlData.states = bEnable ? NV_STEREO_VIDEO_STATE_ENABLE_ON : NV_STEREO_VIDEO_STATE_ENABLE_OFF;

		VPData.Guid = NVDA_StereoVideoControl;
		VPData.DataSize = sizeof(sStereoControlData);
		memset(&VPData.Data, 0, MAX_NV_VIDEO_PROCESS_PRIVATE_DATA_SIZE);
		memcpy(&VPData.Data, &sStereoControlData, VPData.DataSize);

		memcpy(pDst, &VPData, sizeof(NV_VIDEO_PROCESS_PRIVATE_DATA));
		m_pNvControlSurface->UnlockRect();
	}
}

HRESULT CStereoVideoMixer::PrepareIntelS3DAuxiliaryDevice(IDirect3DDevice9 *pDevice)
{
    if (m_pS3DAuxiliaryDeviceService)
        return S_OK;

    HRESULT hr = E_FAIL;

    DbgMsg("[S3D] Flow 5-1, Create DXVA2_Intel_Auxiliary_Device.\n");
    hr = CIntelAuxiliaryDeviceService::Create(pDevice, &m_pS3DAuxiliaryDeviceService);
    if (FAILED(hr))
    {
        return hr;
    }
    //    m_pS3DAuxiliaryDevice = new CIntelAuxiliaryDevice(pDevice);
    if (!m_pS3DAuxiliaryDeviceService)
        return E_OUTOFMEMORY;
    /*
    hr = m_pS3DAuxiliaryDevice->CreateAuxiliaryDevice();
    if (FAILED(hr))
    {
    SAFE_DELETE(m_pS3DAuxiliaryDeviceService);
    return hr;
    }
    */
    BOOL bHasGuid = FALSE;
    DbgMsg("[S3D] Flow 5-2 Check DXVA2_Intel_PrivateQuery_Device.");
    hr = m_pS3DAuxiliaryDeviceService->HasAccelGuids(DXVA2_Intel_PrivateQuery_Device, &bHasGuid);

    if (FAILED(hr))
    {
        DbgMsg("[S3D] Flow 5-2 result : ERROR!! check function failed");
        SAFE_DELETE(m_pS3DAuxiliaryDeviceService);
        return hr;
    }

    if (!bHasGuid)
    {
        DbgMsg("[S3D] Flow 5-2 result : ERROR!! DXVA2_Intel_PrivateQuery_Device is NOT present.");
        SAFE_DELETE(m_pS3DAuxiliaryDeviceService);
        return E_INVALIDARG;
    }
    else
    {
        DbgMsg("[S3D] Flow 5-2 result : DXVA2_Intel_PrivateQuery_Device is present.");
    }

    return hr;
}

HRESULT CStereoVideoMixer::PrepareIntelS3DVP()
{
    HRESULT hr = E_FAIL;
    if (!m_pViewHDVP[0] || !m_pViewHDVP[1] || !m_pS3DAuxiliaryDeviceService)
    {
        DbgMsg("VP[0]=%d, VP[1]=%d, m_pS3DAuxiliaryDeviceService=%d", m_pViewHDVP[0], m_pViewHDVP[1], m_pS3DAuxiliaryDeviceService);
        ReleaseResources();

        hr = PrepareIntelS3DAuxiliaryDevice(m_pDevice);
        if (FAILED(hr) || !m_pS3DAuxiliaryDeviceService)
            return E_FAIL;

        for (int i = 0; i < 2; i++)
        {
            ASSERT(m_pViewHDVP[i] == 0);
            _QUERY_SET_VPP_S3D_MODE VPPS3DMode;
            UINT uCreationModeSize = sizeof(_QUERY_SET_VPP_S3D_MODE);
            ZeroMemory(&VPPS3DMode, uCreationModeSize);
            VPPS3DMode.Header.iPrivateQueryID = 3;
//            VPPS3DMode.S3D_mode = i+1; //i+1; // 1 for L view, 2 for R view;
            if (i == 0)
            {
                VPPS3DMode.S3D_mode = QUERY_S3D_MODE_L;
            }
            else if (i == 1)
            {
                VPPS3DMode.S3D_mode = QUERY_S3D_MODE_R;
            }
            DbgMsg("[S3D] Flow 5-3 Set S3D mode for Creating VPP, QueryID=3, S3D_mode=%d", VPPS3DMode.S3D_mode);

            hr = m_pS3DAuxiliaryDeviceService->QueryAccelCaps(&DXVA2_Intel_PrivateQuery_Device, &VPPS3DMode, &uCreationModeSize);

            if (i == 0)
            {
                DbgMsg("[S3D] Flow 5-4 Create VPP of L View\n");
            }
            else if (i == 1)
            {
                DbgMsg("[S3D] Flow 5-6 Create VPP of R View\n");
            }

            hr = CDXVAHDVP::CreateVP(m_pDeviceEx, NULL, &m_pViewHDVP[i]);
            if (FAILED(hr))
                return hr;
        }
    }
    else 
        hr = S_OK;

    return hr;
}

HRESULT CStereoVideoMixer::IntelS3DOutput(IDirect3DSurface9 *pDestSurface, const RECT &rcDst, const RECT &rcDstClip)
{
    CHECK_POINTER(pDestSurface);
    HRESULT hr = E_FAIL;

    hr = PrepareIntelS3DVP();
    if (FAILED(hr))
        return hr;

    PlaneVPSample Sample[PLANE_MAX];

    UINT uSelectedViewid[2] = {0,1};
    MIXER_STEREO_MODE eStereoMode = (MIXER_STEREO_MODE) GetRegInt(TEXT("StereoMixerMode"), GetMixerStereoMode());
    if (!m_Property.bStereoEnable)
    {
        uSelectedViewid[1] = 0;
    }

    bool pSkipStream[PLANE_MAX] = {0};

    // Background and sub-video can not co-exist, so we skip sub-video if background is visible, and vice versa.
    // This way we can use maximum 4 streams for BD and do not change stream states often.
    pSkipStream[PLANE_BACKGROUND] = !IsBackgroundVisible();
    pSkipStream[PLANE_SUBVIDEO] = !pSkipStream[PLANE_BACKGROUND];

    for (int vid = 0; vid < 2; vid++)
    {
        UINT uVPSampleCount = 0;
        ZeroMemory(Sample, sizeof(Sample));

        for (int planeID = 0; planeID < PLANE_MAX; planeID++)
        {
            if (pSkipStream[planeID])
                continue;

            hr = PreparePlaneVPSample(uSelectedViewid[vid], planeID, Sample[uVPSampleCount], rcDst, rcDstClip);
            uVPSampleCount++;
        }
        hr = ProcessViewDXVAHD(uSelectedViewid[vid], vid, pDestSurface, Sample, uVPSampleCount, rcDst, rcDstClip);
    }
    return hr;
}
