#pragma once

MIDL_INTERFACE("AE0E16AB-969E-4ee8-B4F9-5BCFCE8A5353") IDisplayRenderEngineProperty;

//GuidPropSet Item
// {C42102E9-2E31-4641-BFAF-294DE830E6EC}
DEFINE_GUID(DISPSVR_ENGINE_BASEPROPSET, 0xc42102e9, 0x2e31, 0x4641, 0xbf, 0xaf, 0x29, 0x4d, 0xe8, 0x30, 0xe6, 0xec);
// {C45132C3-D290-4bb1-8A44-24B7E244DE43}
DEFINE_GUID(DISPSVR_ENGINE_MIXERPROPSET, 0xc45132c3, 0xd290, 0x4bb1, 0x8a, 0x44, 0x24, 0xb7, 0xe2, 0x44, 0xde, 0x43);
// {C666F4F9-00E5-4fe1-8193-21D82A6FB91E}
DEFINE_GUID(DISPSVR_ENGINE_PRESENTERPROPSET,0xc666f4f9, 0xe5, 0x4fe1, 0x81, 0x93, 0x21, 0xd8, 0x2a, 0x6f, 0xb9, 0x1e);
// {AE3315A3-5DCB-4bd0-8B19-6FF5FD271639}
DEFINE_GUID(DISPSVR_ENGINE_DRIVERPROPSET,0xae3315a3, 0x5dcb, 0x4bd0, 0x8b, 0x19, 0x6f, 0xf5, 0xfd, 0x27, 0x16, 0x39);

//dwPropID Item
enum ENGINE_PROPERTY_ID
{
    ENGINE_PROPID_RESOURCE_ID  = 1,
};

interface IDisplayRenderEngineProperty : public IUnknown
{
	STDMETHOD(SetProperty)(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData) = 0;
	STDMETHOD(GetProperty)(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData, DWORD *pcbReturned) = 0;
	STDMETHOD(QueryPropertySupported)(REFGUID guidPropSet, DWORD dwPropID, DWORD *pTypeSupport) = 0;
};

// DispSvr resource videomixer group ID
DEFINE_GUID(DISPSVR_RESOURCE_VIDEOMIXER, 0xe51d87ea, 0x169b, 0x49c9, 0xbf, 0x70, 0x4a, 0x1d, 0x8f, 0x9b, 0x23, 0x17);
DEFINE_GUID(DISPSVR_RESOURCE_D3DVIDEOMIXER, 0xf1cc70d7, 0x28db, 0x4689, 0x9a, 0xca, 0xcc, 0x73, 0xbd, 0x30, 0xa8, 0xf7);
DEFINE_GUID(DISPSVR_RESOURCE_D3DDXVA2VIDEOMIXER, 0x1c6873fa, 0xd3a4, 0x4b5b, 0xb4, 0xf5, 0xca, 0xff, 0x82, 0x36, 0xff, 0x21);
DEFINE_GUID(DISPSVR_RESOURCE_INTELFASTCOMPVIDEOMIXER, 0x9af2daef, 0x8d2c, 0x46db, 0xb9, 0x12, 0x71, 0x2c, 0x27, 0xa5, 0x96, 0x5a);
DEFINE_GUID(DISPSVR_RESOURCE_AMDPCOMVIDEOMIXERPRESENTER, 0x531a7d21, 0x8b98, 0x4f6a, 0xb7, 0xd2, 0x3f, 0x62, 0x78, 0xcf, 0x8d, 0x97);
DEFINE_GUID(DISPSVR_RESOURCE_D3DDXVAHDVIDEOMIXER, 0x435c8918, 0x98ba, 0x4d13, 0xb3, 0x4e, 0xb8, 0x13, 0x7a, 0x5b, 0xb3, 0xc3);
DEFINE_GUID(DISPSVR_RESOURCE_STEREOVIDEOMIXER, 0x73b0051, 0x7ae, 0x4b98, 0x9e, 0x52, 0x41, 0xd4, 0x4d, 0x94, 0x72, 0x2b);

// DispSvr resource videopresenter group ID
DEFINE_GUID(DISPSVR_RESOURCE_VIDEOPRESENTER, 0x6015aa89, 0xf958, 0x4a3e, 0xa1, 0xa5, 0x93, 0xa9, 0x9, 0xca, 0xc3, 0xbc);
DEFINE_GUID(DISPSVR_RESOURCE_D3DVIDEOPRESENTER, 0x9dacc41f, 0x1e96, 0x4505, 0xa4, 0x80, 0x53, 0x3d, 0x32, 0x4e, 0x65, 0xc2);
DEFINE_GUID(DISPSVR_RESOURCE_AMDSORTVIDEOPRESENTER, 0x509113d5, 0x7a50, 0x4f1b, 0x81, 0xfd, 0x9b, 0x31, 0x8f, 0xd6, 0x1e, 0x42);
DEFINE_GUID(DISPSVR_RESOURCE_NVAPIVIDEOPRESENTER, 0xc1403ff8, 0x6348, 0x4d54, 0xaa, 0x8a, 0xf5, 0xf1, 0x14, 0xa6, 0xb, 0x14);
DEFINE_GUID(DISPSVR_RESOURCE_S3APIVIDEOPRESENTER, 0x56cb9e5c, 0xe2b0, 0x4c71, 0xbc, 0x16, 0x4e, 0x5b, 0xe2, 0xde, 0xa6, 0x67);
DEFINE_GUID(DISPSVR_RESOURCE_D3DOVERALYPRESENTER, 0x6b893a09, 0xde26, 0x4c46, 0x9c, 0xd4, 0x2b, 0x85, 0x13, 0xc9, 0x2b, 0x58);

/// guidPropSet = DISPSVR_ENGINE_DRIVERPROPSET
enum DRIVER_PROPERTY_ID
{
	DRIVER_PROPID_PAVP_CAP			= 0x00000001,
	DRIVER_PROPID_COPROC_CAP		= 0x00000002,
	DRIVER_PROPID_STEREO_VIDEO_CAP  = 0x00000100,
};

namespace DispSvr
{
	enum STEREO_VISION_MODE_FLAG
	{
		STEREO_VISION_ANAGLYPH              = 0x1 << 0,
		STEREO_VISION_NV_PRIVATE            = 0x1 << 1,
		STEREO_VISION_SIDE_BY_SIDE          = 0x1 << 2,
		STEREO_VISION_NV_STEREO_API         = 0x1 << 3,
		STEREO_VISION_CHECHERBOARD          = 0x1 << 4,
		STEREO_VISION_OPTIMIZED_ANAGLYPH    = 0x1 << 5,
		STEREO_VISION_HALFCOLOR_ANAGLYPH    = 0x1 << 6,
		STEREO_VISION_ROW_INTERLEAVED       = 0x1 << 7,
		STEREO_VISION_HALFCOLOR2_ANAGLYPH   = 0x1 << 8,
		STEREO_VISION_COLUMN_INTERLEAVED    = 0x1 << 9,
		STEREO_VISION_AMD_FRAME_SEQENTIAL   = 0x1 << 10,
		STEREO_VISION_HALF_SIDE_BY_SIDE_LR  = 0x1 << 11,
		STEREO_VISION_HALF_SIDE_BY_SIDE_RL  = 0x1 << 12,
		STEREO_VISION_HALF_TOP_BOTTOM_LR    = 0x1 << 13,
		STEREO_VISION_HALF_TOP_BOTTOM_RL    = 0x1 << 14,
		STEREO_VISION_HDMI_STEREO           = 0x1 << 15,
		STEREO_VISION_DP_STEREO             = 0x1 << 16,
	};

	/// DRIVER_PROPID_STEREO_VIDEO_CAP
	struct DriverStereoVideoCap
	{
		DWORD dwExclusiveModeCap; //< bitwise OR of STEREO_VISION_MODE_FLAG
		DWORD dwWindowModeCap; //< bitwise OR of STEREO_VISION_MODE_FLAG
		DWORD dwContentProtectionWindowModeCap; //< bitwise OR of STEREO_VISION_MODE_FLAG
		BOOL bMVCHardwareDecode;
        BOOL bAVCVLDHardwareDecode;
		DWORD dwReserved[27];
	};

	/// DRIVER_PROPID_PAVP_CAP
	struct DriverPavpCap
	{
		BOOL bSupport;				//< whether PAVP is supported from the device DispSvr runs on.
		BOOL bSupportAudio;			//< check PAVP for audio
		DWORD dwReserved[6];
	};

	/// DRIVER_PROPID_COPROC_CAP
	struct DriverCoprocCap
	{
		BOOL bSupport;				//< whether coproc mode is supported from the device DispSvr runs on.
		BOOL bEnabled;				//< if coproc mode is enabled.
		DWORD dwCoprocVendorID;		//< PCI vendor ID of coproc hardware.
		DWORD dwCoprocDeviceID;		//< PCI device ID of coproc hardware.
		DWORD dwReserved[28];
	};
}
