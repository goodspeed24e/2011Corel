#include "stdafx.h"
#include <windows.h>
#include <streams.h>
#include <tchar.h>
#include "HVDDSVPConfig.h"

#define INIT_DIRECTDRAW_STRUCT(x) (ZeroMemory(&x, sizeof(x)), x.dwSize=sizeof(x))

using namespace HVDService;

CHVDDSVPConfig::CHVDDSVPConfig(CCritSec *pLock, LPUNKNOWN pUnk) : CUnknown(_T("CHVDDSVPConfig"), pUnk)
	{
	m_pVPConfigLock = pLock;
	}

STDMETHODIMP CHVDDSVPConfig::NonDelegatingQueryInterface(REFIID riid, void **ppv)
	{
	CheckPointer(ppv,E_POINTER);
	ValidateReadWritePtr(ppv,sizeof(PVOID));
	*ppv = 0;
	if(riid==IID_IVPConfig)
		return GetInterface((IVPConfig *)this,ppv);
	return CUnknown::NonDelegatingQueryInterface(riid, ppv);
	}

STDMETHODIMP CHVDDSVPConfig::GetConnectInfo(LPDWORD pdwNumConnectInfo, LPDDVIDEOPORTCONNECT pddVPConnectInfo)
	{
	CAutoLock lock_it(m_pVPConfigLock);
	
	*pdwNumConnectInfo = 1;
	if (pddVPConnectInfo!=NULL && *pdwNumConnectInfo!=0)
		{			
		INIT_DIRECTDRAW_STRUCT((pddVPConnectInfo[0]));
		
		pddVPConnectInfo[0].guidTypeID  = DDVPTYPE_CCIR656;
		pddVPConnectInfo[0].dwPortWidth = 8;
		pddVPConnectInfo[0].dwFlags		= DDVPCONNECT_INTERLACED;
		}
	return S_OK;
	}

STDMETHODIMP CHVDDSVPConfig::GetMaxPixelRate(LPAMVPSIZE pamvpSize, LPDWORD pdwMaxPixelsPerSecond)
	{
	pamvpSize->dwWidth = 720;
	pamvpSize->dwHeight = 480;
	*pdwMaxPixelsPerSecond =
		(pamvpSize->dwWidth * pamvpSize->dwHeight * 30 * 2);
	return S_OK;
	}

STDMETHODIMP CHVDDSVPConfig::GetOverlaySurface(LPDIRECTDRAWSURFACE *ppddOverlaySurface)
	{
	return S_OK;
	}

STDMETHODIMP CHVDDSVPConfig::GetVideoFormats(LPDWORD pdwNumFormats, LPDDPIXELFORMAT pddPixelFormats)
	{
	CAutoLock lock_it(m_pVPConfigLock);
	
	*pdwNumFormats = 1;
	if (pddPixelFormats!=NULL && *pdwNumFormats!=0)
		{			
		INIT_DIRECTDRAW_STRUCT((pddPixelFormats[0]));
		
		pddPixelFormats[0].dwFourCC = MAKEFOURCC('U','Y','V','Y');
		pddPixelFormats[0].dwFlags = DDPF_FOURCC;
		pddPixelFormats[0].dwYUVBitCount = 16;
		}
	return S_OK;
	}

STDMETHODIMP CHVDDSVPConfig::GetVPDataInfo(LPAMVPDATAINFO pamvpDataInfo)
	{
	INIT_DIRECTDRAW_STRUCT((*pamvpDataInfo));
	
	pamvpDataInfo->dwMicrosecondsPerField	= 16667;	// Time taken by each field
	pamvpDataInfo->bEnableDoubleClock		= FALSE;	// Videoport should enable double clocking
	pamvpDataInfo->bEnableVACT				= FALSE;	// Videoport should use an external VACT signal
	pamvpDataInfo->bDataIsInterlaced		= TRUE;		// Indicates that the signal is interlaced
	pamvpDataInfo->bFieldPolarityInverted	= FALSE;	// Device inverts the polarity by default
	pamvpDataInfo->lHalfLinesOdd			= 1;		// number of halflines in the odd field
	pamvpDataInfo->lHalfLinesEven			= 0;		// number of halflines in the even field
	
	pamvpDataInfo->amvpDimInfo.dwFieldWidth   = 720;	// Field height of the data
	pamvpDataInfo->amvpDimInfo.dwFieldHeight  = 240;	// Field width of the data
	pamvpDataInfo->amvpDimInfo.dwVBIWidth     = 0;      //720 + 158 / 2;				// Width of the VBI data
	pamvpDataInfo->amvpDimInfo.dwVBIHeight    = 0;		// Height of the VBI data
	pamvpDataInfo->amvpDimInfo.rcValidRegion.left = 0;
	pamvpDataInfo->amvpDimInfo.rcValidRegion.top  = 0;
	pamvpDataInfo->amvpDimInfo.rcValidRegion.right = 720;
	pamvpDataInfo->amvpDimInfo.rcValidRegion.bottom  = 240;
	
	pamvpDataInfo->dwPictAspectRatioX = 4;			// X dimension of Picture Aspect Ratio
	pamvpDataInfo->dwPictAspectRatioY = 3;			// Y dimension of Picture Aspect Ratio
	pamvpDataInfo->dwNumLinesInVREF = 1;			// Number of lines of data in VREF 
	
	return S_OK;
	}

STDMETHODIMP CHVDDSVPConfig::IsVPDecimationAllowed(LPBOOL pbIsDecimationAllowed)
	{
	*pbIsDecimationAllowed = FALSE;
	return S_OK;
	}

STDMETHODIMP CHVDDSVPConfig::InformVPInputFormats(DWORD dwNumFormats, LPDDPIXELFORMAT pDDPixelFormats)
	{
	return S_OK;
	}

STDMETHODIMP CHVDDSVPConfig::SetConnectInfo(DWORD dwChosenEntry)
	{
	return S_OK;
	}

STDMETHODIMP CHVDDSVPConfig::SetDDSurfaceKernelHandle(DWORD dwDDKernelHandle)
	{
	return S_OK;
	}

STDMETHODIMP CHVDDSVPConfig::SetDDSurfaceKernelHandles(DWORD dwDDKernelHandles, DWORD *pDDKernelHandlers)
	{
	return S_OK;
	}

STDMETHODIMP CHVDDSVPConfig::SetDirectDrawKernelHandle(DWORD dwDDKernelHandle)
	{
	return S_OK;
	}

STDMETHODIMP CHVDDSVPConfig::SetInvertPolarity(void)
	{
	return S_OK;
	}

STDMETHODIMP CHVDDSVPConfig::SetSurfaceParameters(DWORD dwPitch, DWORD dwXOrigin, DWORD dwYOrigin)
	{
	return S_OK;
	}

STDMETHODIMP CHVDDSVPConfig::SetVideoPortID (DWORD dwVideoPortID)
	{
	return S_OK;
	}

STDMETHODIMP CHVDDSVPConfig::SetVideoFormat (DWORD dwChosenEntry)
	{
	return S_OK;
	}

STDMETHODIMP CHVDDSVPConfig::SetScalingFactors(LPAMVPSIZE pamvpSize)
	{
	return S_OK;
	}

