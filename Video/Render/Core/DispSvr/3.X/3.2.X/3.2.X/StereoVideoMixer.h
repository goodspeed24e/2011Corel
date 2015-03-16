#ifndef __DISPSVR_STEREO_VIDEO_MIXER_H__
#define __DISPSVR_STEREO_VIDEO_MIXER_H__

#include "D3D9Dxva2HDVideoMixer.h"
#include "IntelDxva2Device.h"

namespace DispSvr
{
	struct PlaneVPSample
	{
		PLANE_ID PlaneID;
		RECT rcSrc;
		RECT rcDst;
		IDirect3DSurface9 *pSurface;
	};

    //Intel S3D structure

    // Private Query Header
    typedef struct _QUERY_HEADER
    {
        INT             iPrivateQueryID;    // [IN    ] Must be QUERY_ID_*
        INT             iReserved;          // [IN/OUT] Reserved, MBZ
    } QUERY_HEADER, *PQUERY_HEADER;

    // Private query S3D operation
    typedef enum _QUERY_S3D_MODE
    {
        QUERY_S3D_MODE_NONE          = 0,
        QUERY_S3D_MODE_L             = 1,
        QUERY_S3D_MODE_R             = 2
    } QUERY_S3D_MODE;

    // Private Query for setting up S3D mode for VPP creation
    typedef struct _QUERY_SET_VPP_S3D_MODE
    {
        QUERY_HEADER        Header;     // [IN] Header - QUERY_ID_S3D
        QUERY_S3D_MODE      S3D_mode;   // [IN] Indicate L or R view
    } QUERY_SET_VPP_S3D_MODE, *PQUERY_SET_VPP_S3D_MODE;


	class CStereoVideoMixer : public CD3D9Dxva2HDVideoMixer
	{
        friend class CIntelAuxiliaryDevice;

	public:
		CStereoVideoMixer();
		virtual ~CStereoVideoMixer();

	protected:
		STDMETHOD(_Execute)(IDirect3DSurface9 *pDestSurface, const RECT &rcDst, const RECT &rcDstClip);
		STDMETHOD(_ReleaseDevice)();

		void ReleaseResources();
		void EnableStereoInNvControlSurface(BOOL bEnable);
		HRESULT CheckAndPrepareRenderTarget(MIXER_STEREO_MODE eStereMode, IDirect3DSurface9 **ppSxSRT, RECT &rcBaseView, RECT &rcBaseViewClip, RECT &rcDependentView, RECT &rcDependentViewClip);
		HRESULT PreparePlaneVPSample(UINT uViewID, PLANE_ID PlaneID, PlaneVPSample &Sample, const RECT &rcDst, const RECT &rcDstClip);
		HRESULT ProcessViewDXVAHD(UINT uViewID, UINT uVPID, IDirect3DSurface9 *pDst, PlaneVPSample *pSubSamples, UINT uNumSubSamples, const RECT &rcDst, const RECT &rcDstClip);

		HRESULT TestOutput(IDirect3DSurface9 *pSxSRT, IDirect3DSurface9 *pDestSurface, const RECT &rcDst, const RECT &rcDstClip);
		HRESULT MergeShaderOutput(IDirect3DPixelShader9 *pShader, IDirect3DSurface9 *pSxSRT, IDirect3DSurface9 *pDestSurface, const RECT &rcDst, const RECT &rcDstClip);
		HRESULT NvStereoOutput(IDirect3DSurface9 *pSxSRT, IDirect3DSurface9 *pDestSurface, const RECT &rcDst, const RECT &rcDstClip);
        HRESULT PrepareIntelS3DAuxiliaryDevice(IDirect3DDevice9 *pDevice);
        HRESULT PrepareIntelS3DVP();
        HRESULT IntelS3DOutput(IDirect3DSurface9 *pDestSurface, const RECT &rcDst, const RECT &rcDstClip);

	protected:
		D3DXMATRIX m_matrixWorldTranspose;
		UINT m_uRTWidth;
		UINT m_uRTHeight;
		HANDLE m_hSxSRT;
		VideoProcessorStub *m_pVPStub;
		CDXVAHDVP *m_pViewHDVP[2];
		CDXVA2VideoProcessor *m_pStereoVP;
		IDirect3DPixelShader9 *m_pMergeAnaglyphPS;
		IDirect3DPixelShader9 *m_pCheckerboardPS;
        IDirect3DPixelShader9 *m_pRowInterleavedPS;
        IDirect3DPixelShader9 *m_pColumnInterleavedPS;
		IDirect3DVertexShader9 *m_pVS;
		void *m_pAnaglyphVertexBuffer;
		IDirect3DSurface9 *m_pNvControlSurface;
        BOOL m_bInShutterMode;
        MIXER_STEREO_MODE m_eCurrentStereoMode;
        CIntelAuxiliaryDeviceService *m_pS3DAuxiliaryDeviceService;
	};
}


#endif	// __DISPSVR_STEREO_VIDEO_MIXER_H__