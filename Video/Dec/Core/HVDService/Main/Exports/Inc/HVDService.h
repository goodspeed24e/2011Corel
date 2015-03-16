#ifndef _HARDWARE_DECODE_SERVICE_H_
#define _HARDWARE_DECODE_SERVICE_H_

struct __declspec(uuid("A44DA533-81CE-47a8-AC95-8A564B5A7F47")) IHVDService;
struct __declspec(uuid("D1476698-96ED-4649-985A-511660A1FE54")) IHVDServiceDxva;
struct __declspec(uuid("0BC8BC88-8B90-4641-87A6-2B8F795B4914")) IHVDServiceDxva1;
struct __declspec(uuid("1471D86B-DBAC-47f2-9A51-597C78DA215F")) IHVDServiceDxva2;
struct __declspec(uuid("97C76F3F-E803-4ed9-B81E-BF4718F7DA3A")) IHVDServiceNotify;
struct __declspec(uuid("59E39E4E-A77B-425B-946C-ACBCBEB9FF52")) IHVDServiceFastReadBack;

interface IBaseFilter;
interface IGraphBuilder;
interface IMediaSample;
interface ICPService;

namespace HVDService
{
	enum HVD_EVENT
	{
		HVD_EVENT_SERVICE_START,
		HVD_EVENT_SERVICE_STOP,
		HVD_EVENT_UNCOMPBUF_AVAILABILITY_QUERY
	};

	enum HVD_SERVICE
	{
		HVD_SERVICE_UNKNOWN = -1,
		HVD_SERVICE_DXVA1 = 0,
		HVD_SERVICE_DXVA2 = 1
	};

	enum HVD_INIT_FLAGS
	{
		HVD_INIT_D3DDEVICE					= 1 << 0,
		HVD_INIT_MFGETSERVICE				= 1 << 1,
		HVD_INIT_DXVA1_FLAGS				= 1 << 2,
		HVD_INIT_BASEFILTER					= 1 << 3,
		HVD_INIT_FASTREADBACK				= 1 << 4,
	};

	enum HVD_DXVA1_INIT_FLAGS
	{
		HVD_INIT_DXVA1_RENDERER_TYPE			= 1 << 0,
		HVD_INIT_DXVA1_VIDEO_OUTPUT_BUFFER		= 1 << 1,
		HVD_INIT_DXVA1_SUB_OUTPUT_BUFFER		= 1 << 2,
		HVD_INIT_DXVA1_FIXED_24FPS				= 1 << 3,
		HVD_INIT_DXVA1_RECOMMEND_SURFACE_COUNT	= 1 << 4,
		HVD_INIT_DXVA1_YUV_MIXING				= 1 << 5,
		HVD_INIT_DXVA1_YUV_DECIMATE_BY_2		= 1 << 6,
		HVD_INIT_DXVA1_SUB_PIC_PIN				= 1 << 7,
		HVD_INIT_DXVA1_INTERLACE_SAMPLE			= 1 << 8,
		HVD_INIT_DXVA1_NVIDIA_IDCT				= 1 << 9,
	};

	enum HVD_MODE
	{
		HVD_MODE_UNKNOWN = -1,
		HVD_MODE_MPEG2 = 0,
		HVD_MODE_VC1 = 1,
		HVD_MODE_H264 = 2,
		HVD_MODE_MPEG4 = 3,
		HVD_MODE_MPEG1 = 4,
		HVD_MODE_OTHERS = 255,
	};

	enum HVD_LEVEL
	{
		HVD_LEVEL_UNKNOWN = -1,
		HVD_LEVEL_MC = 0,
		HVD_LEVEL_IDCT = 1,
		HVD_LEVEL_VLD = 2,
		HVD_LEVEL_AUTO = 3,
		HVD_LEVEL_AUTO_CONSTRAIN_SURFACE_NUM = 4
	};

	enum HVD_DECODE_FLAGS
	{
		HVD_DECODE_MAXSURFACECOUNT = 1 << 0,
		HVD_DECODE_MINSURFACECOUNT = 1 << 1,
		HVD_DECODE_UABPROTECTLEVEL = 1 << 2,
		HVD_DECODE_ENCRYPTION_LEGACY = 1 << 3,
		HVD_DECODE_ENCRYPTION_PRIVATE = 1 << 4,
		HVD_DECODE_MODE = 1 << 5,
		HVD_DECODE_ENCRYPTION_GPUCP = 1 << 6,
	};

	enum HVD_DXVA2_EXECUTE_FLAGS
	{
		HVD_DXVA2_EXECUTE_EXTDATA = 1 << 0,
	};

	enum HVD_DXVA1_VIDEO_RENDERER_TYPE
	{
		HVD_DXVA1_RENDERER_OVERLAY=0,
		HVD_DXVA1_RENDERER_VMR7,
		HVD_DXVA1_RENDERER_VMR9WIDNOWLESS,
		HVD_DXVA1_RENDERER_VMR9CUSTOM,
		HVD_DXVA1_RENDERER_EXTERNAL,
	};

    enum HVD_FB_PARAM_TYPE
    {
        HVD_FB_PARAM_TYPE_GET_NVIDIA_CAPS = 0,
        HVD_FB_PARAM_TYPE_GET_INTEL_CAPS = 1,
        HVD_FB_PARAM_TYPE_GET_AMD_SCALING_CAPS = 2,
        HVD_FB_PARAM_TYPE_SET_PICH_COPY = 3,
        HVD_FB_PARAM_TYPE_SET_MSDK_MEM_ALLOCATOR,
        HVD_FB_PARAM_TYPE_SET_DISPLAY_RECT,
        HVD_FB_PARAM_TYPE_SET_LOCK_SURFACE,
    };

	struct HVDDxva1InitFlag
	{
		DWORD dwFlags;
		DWORD dwDxva1RendererType;
		DWORD dwVideoOutputPinBuf; // = GetRegInt(TEXT("DsVideoBuf"), 6);
		DWORD dwSubOutputPinBuf; // = GetRegInt(TEXT("DsSpicBuf"), 2);
		BOOL  bFixFPS24Enabled;// = GetRegInt(TEXT("FIXFPS24"), 0);
		BOOL  bRecommendSurfCount;// = GetRegInt(TEXT("DisableSetDXVABUF"), 0);
		BOOL  bYUVMixing;// = GetRegInt(TEXT("VMRYUVMIXING"), 1) != 0;
		BOOL  bYUVDecimateBy2;// = GetRegInt(TEXT("VmrDecimateBy2"), 0) != 0;
		BOOL  bSubPicPin;// = GetRegInt("DispSvrSPIC", 1);	// m_dwDirectSPIC=1: turn on the CVRNDisplaySPIC, don't connect the sub-picture pin
		BOOL  bInterlaceSample; //= GetRegInt("INTERLACESAMPLE", 1)
		BOOL  bNVIDCT;// GetRegInt("NVIDCT", 1)
	};

	struct HVDInitFastCopyConfig
	{
		DWORD dwWidthReadback;
		DWORD dwHeightReadback;
        DWORD dwInputInterlaceFlag;		// Indicates input source is interlace content or not. [0,1].
        DWORD dwOutputInterlaceFlag;	// Indicates output source is interlace content or not. [0,1].
        DWORD dwExpectedReadbackWidth;
        DWORD dwExpectedReadbackHeight;
        DWORD dwReserved[11];
	};

	struct HVDInitConfig
	{
		DWORD dwFlags;
		IUnknown* pExternalDevice;
		HWND hwnd;
		HVDDxva1InitFlag m_Dxva1Flags;
		void* pHVDInitFastCopyConfig;
		DWORD dwReserved[11];
	};

	struct HVDDecodeConfig
	{
		DWORD dwFlags;
		DWORD dwMode;
		DWORD dwLevel;
		DWORD dwWidth;
		DWORD dwHeight;
		DWORD dwMaxSurfaceCount;
		DWORD dwMinSurfaceCount;
		DWORD dwUABProtectionLevel;
		DWORD dwReserved[25];
	};

	struct HVDDxvaCompBufLockInfo
	{
		LPVOID pBuffer;
		LONG lStride;
		UINT uSize;
	};

	struct HVDDxvaExecuteConfig
	{
		DWORD dwFlags;
		DWORD dwNumBuffers;
		LPVOID lpAmvaBufferInfo;
		// Extension Data
		DWORD dwFunction;
		LPVOID lpPrivateInputData;
		DWORD cbPrivateInputData;
		LPVOID lpPrivateOutputData;
		DWORD cbPrivateOutputData;
	};

	struct HVDDxva2UncompBufLockInfo
	{
		void* pBits;
		UINT uPitch;
		DWORD dwFourCC;
	};

	struct HVDFastReadbackOption
	{
		DWORD dwReturnFourcc; //0:D3D; Otherwise: FourCC code 
		void* pReturnSurface; 
		DWORD dwDelayPeriod;
		WORD  wWidth, wHeight;
		BYTE  bInputProgressive;
		BYTE  bOutputProgressive;
		BYTE  i8Reserved[2];
		void* pMskMemAllocator;
        DWORD dwReserved[2];
	};

	struct HVDDXVAConfigPictureDecode
	{
		// Operation Indicated
		DWORD dwFunction;

		// Alignment
		DWORD dwReservedBits[3];

		// Encryption GUIDs
		GUID guidConfigBitstreamEncryption;
		GUID guidConfigMBcontrolEncryption;
		GUID guidConfigResidDiffEncryption;

		// Bitstream Processing Indicator
		BYTE bConfigBitstreamRaw;

		// Macroblock Control Config
		BYTE bConfigMBcontrolRasterOrder;

		// Host Resid Diff Config
		BYTE bConfigResidDiffHost;
		BYTE bConfigSpatialResid8;
		BYTE bConfigResid8Subtraction;
		BYTE bConfigSpatialHost8or9Clipping;
		BYTE bConfigSpatialResidInterleaved;
		BYTE bConfigIntraResidUnsigned;

		// Accelerator Resid Diff Config
		BYTE bConfigResidDiffAccelerator;
		BYTE bConfigHostInverseScan;
		BYTE bConfigSpecificIDCT;
		BYTE bConfig4GroupedCoefs;
	};

	struct HVDDXVAConfigAlphaLoad
	{  
		DWORD  dwFunction;
		DWORD  dwReservedBits[3];
		BYTE  bConfigDataType;
	};

	struct HVDDXVAConfigAlphaCombine 
	{  
		DWORD  dwFunction;
		DWORD  dwReservedBits[3];
		BYTE  bConfigBlendType;
		BYTE  bConfigPictureResizing;
		BYTE  bConfigOnlyUsePicDestRectArea;
		BYTE  bConfigGraphicResizing;
		BYTE  bConfigWholePlaneAlpha;
	};

	struct HVDDXVAGeneralInfo
	{
		DWORD	m_dwSurfaceType;

		LPVOID	m_pCompBufferInfo; //AMVACompBufferInfo[16]

		BOOL	m_bDecodeBufHold;
		DWORD	m_dwAlphaBufWidth;
		DWORD	m_dwAlphaBufHeight;
		DWORD	m_dwAlphaBufSize;
		DWORD	m_dwDecodeFrame;
		DWORD	m_dwDecodeBufNum;
		BOOL	m_bUseEncryption;
		BOOL	m_bConfigDecoder;
		DWORD	m_dwSWDeinterlace;

		DWORD	m_dwVendorID;			// may be 0 if unknown
		DWORD	m_dwDeviceID;			// may be 0 if unknown
	};

	struct HVDDxva1UncompSurfConfig
	{
		DWORD dwDXVAInv32;
		DWORD dwMinDecodeCount;
		DWORD dwMinSurfaceCount;
		DWORD dwMaxSurfaceCount;
		DWORD dwMinsurfaceCountforBD;
		DWORD dwReserved[3];
	};

	enum HVDDxva1DecodeStatus
	{
		HVD_DXVA1_DECODE_PLAY = 0,
		HVD_DXVA1_DECODE_STOP,
		HVD_DXVA1_DECODE_FLUSH,
	};

	struct HVDDxva1VideoFrame
	{
		DWORD dwFrameIndex;
		DWORD dwFrameRate;
		DWORD dwDecodeStatus;
		DWORD dwRepeatFirstField;
		BOOL bNeedSyncStreamTime;
		BOOL bRefresh;
		BOOL bNormalPlayback;
		BOOL bDecodeHalfWidth;
		BOOL bDecodeHalfHeight;
		INT nVerticalSize;
		INT nHorizontalSize;
		IMediaSample *pMediaSample;
		DWORD dwMediaSampleProperty;
		DWORD dwPictAspectRatioX;
		DWORD dwPictAspectRatioY;
	};

	struct HVDDxva1SubFrame
	{

	};
}

interface IHVDService : public IUnknown
{
	STDMETHOD(Initialize)(HVDService::HVDInitConfig* pInitConfig) = 0;
	STDMETHOD(Uninitialize)() = 0;
	STDMETHOD(StartService)(HVDService::HVDDecodeConfig* pDecoderConfig) = 0;
	STDMETHOD(GetDecoderGuid)(GUID* pGuid) = 0;
	STDMETHOD(GetSupportedDecoderCount)(UINT* pnCount) = 0;
	STDMETHOD(GetSupportedDecoderGUIDs)(GUID* pGuids) = 0;
	STDMETHOD(AdviseEventNotify)(IHVDServiceNotify* pNotify) = 0;
	STDMETHOD(UnadviseEventNotify)(IHVDServiceNotify* pNotify) = 0;
	STDMETHOD(GetHVDDecodeConfig)(HVDService::HVDDecodeConfig* pDecoderConfig) = 0;
};

interface IHVDServiceDxva : public IUnknown
{
	STDMETHOD(LockCompressBuffer)(DWORD dwType, DWORD dwIndex, HVDService::HVDDxvaCompBufLockInfo *pInfo, BOOL bReadOnly=FALSE) = 0;
	STDMETHOD(UnlockCompressBuffer)(DWORD dwType, DWORD dwIndex) = 0;
	STDMETHOD(BeginFrame)(DWORD dwDstSurfIndex) = 0;
	STDMETHOD(Execute)(HVDService::HVDDxvaExecuteConfig* pExecuteConfig) = 0;
	STDMETHOD(EndFrame)(DWORD dwDstSurfIndex = 0) = 0;
	STDMETHOD(GetUncompressedBufferCount)(DWORD* pdwCount) = 0;
	STDMETHOD(GetUncompresesdBufferFormat)(DWORD* pdwFourCC) = 0;
	STDMETHOD(GetAccel)(IUnknown** ppAccel) = 0;
	STDMETHOD(GetDxvaConfigPictureDecode)(HVDService::HVDDXVAConfigPictureDecode* pDxvaConfigPictureDecode) = 0;
};

interface IHVDServiceDxva1 : public IHVDServiceDxva
{
	STDMETHOD(GetRendererType)(DWORD *dwRendererType) = 0;
	STDMETHOD(GetVideoRenderer)(IBaseFilter** ppFilter) = 0;
	STDMETHOD(SetDisplaySize)(const LPRECT pRect) = 0;
	STDMETHOD(SetVideoWindow)(HWND hWnd) = 0;
	STDMETHOD(DisplayFrame)(HVDService::HVDDxva1VideoFrame *pFrame, HVDService::HVDDxva1SubFrame *pSub) = 0;
	STDMETHOD(Repaint)(HDC hdc, LPRECT pRect) = 0;
	STDMETHOD(GetInternalCompBufferInfo)(DWORD* pdwNumTypesCompBuffers, struct _tag_AMVACompBufferInfo *pamvaCompBufferInfo) = 0;
	STDMETHOD(QueryRenderStatus)(DWORD dwTypeIndex, DWORD dwBufferIndex, DWORD dwFlags) = 0;
	STDMETHOD(RunDXVAGraph)() = 0;
};

interface IHVDServiceDxva2 : public IHVDServiceDxva
{
	STDMETHOD(GetUncompressedBuffer)(DWORD dwIndex, IDirect3DSurface9** ppSurface) = 0;
	STDMETHOD(LockUncompressedBuffer)(DWORD dwIndex, HVDService::HVDDxva2UncompBufLockInfo* pInfo) = 0;
	STDMETHOD(UnlockUncompressedBuffer)(DWORD dwIndex) = 0;
	STDMETHOD(SetCP)(LPVOID pIviCP) = 0;
	STDMETHOD(FastReadback)(DWORD dwIndex, HVDService::HVDDxva2UncompBufLockInfo* pInfo, HVDService::HVDFastReadbackOption* pReadbackOption) = 0;
};

interface IHVDServiceNotify : public IUnknown
{
	STDMETHOD(OnNotify)(DWORD dwEvent, DWORD dwParam1, DWORD dwParam2) = 0;
};

interface IHVDServiceFastReadBack : public IUnknown
{
    STDMETHOD(SetParameter)(DWORD dwParamType, void* pvdParam, DWORD* pdwParamSize) = 0;
    STDMETHOD(GetParameter)(DWORD dwParamType, void* pvdParam, DWORD* pdwParamSize) = 0;
};

#ifdef HVDSERVICE_EXPORTS
#define HVDSERVICE_API __declspec(dllexport)
#else
#define HVDSERVICE_API __declspec(dllimport)
#endif

extern "C" HVDSERVICE_API int CreateHVDService(DWORD dwService, void** ppHVDService);

#endif