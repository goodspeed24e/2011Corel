#ifndef _HARDWARE_DECODE_SERVICE_DXVA1_H_
#define _HARDWARE_DECODE_SERVICE_DXVA1_H_

#include <dxva.h>
#include <videoacc.h>
#include "HVDServiceBase.h"


namespace HVDService
{
#define NUM_COMP_BUFFER_TYPE 16

	class CHVDDxva1Binder;

	class CHVDServiceDxva1 : 
		public CHVDServiceBase,
		public IHVDServiceDxva1
	{
	public:
		CHVDServiceDxva1();
		virtual ~CHVDServiceDxva1();

		STDMETHOD(QueryInterface)(REFIID riid, void **ppv);
		STDMETHOD_(ULONG, AddRef)();
		STDMETHOD_(ULONG, Release)();

		// CHVDServiceBase
		HRESULT _Initialize(HVDInitConfig* pInitConfig);
		HRESULT _Uninitialize();
		HRESULT _StartService();
		HRESULT _StopService();

		// IHVDServiceDxva
		STDMETHOD(LockCompressBuffer)(DWORD dwType, DWORD dwIndex, HVDDxvaCompBufLockInfo *pInfo, BOOL bReadOnly=FALSE);
		STDMETHOD(UnlockCompressBuffer)(DWORD dwType, DWORD dwIndex);
		STDMETHOD(BeginFrame)(DWORD dwDstSurfIndex);
		STDMETHOD(Execute)(HVDDxvaExecuteConfig* pExecuteConfig);
		STDMETHOD(EndFrame)(DWORD dwDstSurfIndex);
		STDMETHOD(GetUncompressedBufferCount)(DWORD *pdwCount);
		STDMETHOD(GetUncompresesdBufferFormat)(DWORD* pdwFourCC);
		STDMETHOD(GetAccel)(IUnknown** ppAccel);
		STDMETHOD(GetDxvaConfigPictureDecode)(HVDDXVAConfigPictureDecode* pDxvaConfigPictureDecode);

		//IHVDServiceDxva1
		STDMETHOD(GetRendererType)(DWORD *dwRendererType);
		STDMETHOD(GetVideoRenderer)(IBaseFilter** ppFilter);
		STDMETHOD(SetDisplaySize)(const LPRECT pRect);
		STDMETHOD(SetVideoWindow)(HWND hWnd);
		STDMETHOD(DisplayFrame)(HVDService::HVDDxva1VideoFrame *pFrame, HVDService::HVDDxva1SubFrame *pSub);
		STDMETHOD(Repaint)(HDC hdc, LPRECT pRect);
		STDMETHOD(GetInternalCompBufferInfo)(DWORD* pdwNumTypesCompBuffers, struct _tag_AMVACompBufferInfo *pamvaCompBufferInfo);
		STDMETHOD(QueryRenderStatus)(DWORD dwTypeIndex, DWORD dwBufferIndex, DWORD dwFlags);
		STDMETHOD(RunDXVAGraph)();

	protected:
		HRESULT CreateGraphBuilder();
		HRESULT CreateVideoRenderer();
		HRESULT DestroyGraph();
		HRESULT ConnectGraph();			// cnonect filters
		HRESULT DisconnectGraph();		// disconnect filters
		HRESULT DestroyBinder();
		HRESULT UpdateAMVideoAccelerator();
		HRESULT CheckDeinterlaceMode();
		HRESULT DSUpdateOutputSample(HVDService::HVDDxva1VideoFrame *pFrame);
		HRESULT SetMixingPref(DWORD dwPref, BOOL bBehavior = TRUE);

		HRESULT Run();
		//HRESULT Pause();				//switch between run and stop should pause first.
		HRESULT Stop();

		DWORD GetUncompSurfNum();
		BOOL GetRecommendedTextureSize(DWORD *pWidth, DWORD *pHeight);

		int	InitializeDXVAConfig();
		int	InitializeDXVAConfigPicture();
		int	InitializeDXVAConfigAlphaLoad();
		int	InitializeDXVAConfigAlphaComb();

		VOID CHVDServiceDxva1::DebugPrintMediaSample(IMediaSample *pSample);
	protected:

		HWND m_hWnd;

		DWORD m_dwWidth;
		DWORD m_dwHeight;
		
		CHVDDxva1Binder* m_pBinder;
		IAMVideoAccelerator *m_pVideoAccel;
		IReferenceClock *m_pClock;
		//GUID m_Guid;
		CMediaType	m_MediaType;

		AMVACompBufferInfo m_pCompBufferInfo[NUM_COMP_BUFFER_TYPE];
		HVDDxva1UncompSurfConfig m_UncompSurfacesConfig; 
		DWORD m_dwSurfaceType;
		BOOL m_bConfigDecoder;
		DWORD m_dwAlphaBufWidth;
		DWORD m_dwAlphaBufHeight;
		DWORD m_dwAlphaBufSize;
		// Unused variable
		BOOL m_bDecodeBufHold;
		DWORD m_dwDecodeFrame;
		DWORD m_dwDecodeBufNum;
		BOOL m_bUseEncryption;
		DWORD m_dwSWDeinterlace;

		DXVA_ConfigPictureDecode m_DxvaConfigPictureDecode;
		DXVA_ConfigAlphaLoad m_DxvaConfigAlphaLoad;
		DXVA_ConfigAlphaCombine m_DxvaConfigAlphaCombine;

		DWORD m_dwMinSurfaceCount;

		IGraphBuilder *m_pGraph;
		IBaseFilter *m_pVideoRender;

		REFERENCE_TIME m_rtStart;// = 0;
		REFERENCE_TIME m_rtAvgTime; // = 0;
		BOOL m_bSyncStreamTime;// = FALSE;

		//Register Rot
		DWORD m_dwRotReg;
		// registry value
		DWORD m_dwVideoRendererType;
		DWORD m_dwVideoOutputPinBuffer;
		DWORD m_dwSubOutputPinBuffer;
		BOOL m_bFIXFPS24Enabled;// = GetRegInt(TEXT("FIXFPS24"), 0);
		BOOL m_bRecommendSurfCount;// = GetRegInt(TEXT("DisableSetDXVABUF"), 0);
		BOOL m_bYUVMixing;// = GetRegInt(TEXT("VMRYUVMIXING"), 1) != 0;
		BOOL m_bYUVDecimateBy2;// = GetRegInt(TEXT("VmrDecimateBy2"), 0) != 0;
		BOOL m_bSubPicPin;// = GetRegInt("DispSvrSPIC", 1);	// m_dwDirectSPIC=1: turn on the CVRNDisplaySPIC, don't connect the sub-picture pin
		BOOL m_bInterlaceSample; //= GetRegInt("INTERLACESAMPLE", 1)
		BOOL m_bNVIDCT;// GetRegInt("NVIDCT", 1)
		//DWORD m_dwMinDecBuf;// = GetRegInt(TEXT("DXVADECMIN"), 5);
		//BOOL m_bHD1VMR7Enabled;// = GetRegInt(TEXT("HD1VMR7"), 0);
		//BOOL m_bHD1VMR9Enabled;// = GetRegInt(TEXT("HD1VMR9"), 0);
		//DWORD m_dwBlendLast; //= GetRegInt("DXVABLENDLAST", 2)
		//DWORD m_bFrontBlend; //GetRegInt("DXVAFRONTBLEND", 0)
	};
}
#endif