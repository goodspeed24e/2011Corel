#include "stdafx.h"
#include <windows.h>
#include <streams.h>
#include <tchar.h>
#include <d3d9.h>
#include <dxva.h>
#include "HVDGuids.h"
#include "Imports/LibGPU/UtilGPU.h"
#include "Imports/LibGPU/GPUID.h"
//#include "dxva.h"

//#define HAVE_GUID_H
//#define HAVE_LIBOS_H
//#define HAVE_UTILPCI_H
//#define HAVE_UTILVGA_H
//#include "../../Shared/Include/ivheaders.h"
//#include "vrdxva_Inv32.h"
#include "HVDDSAMVANotify.h"

//#include <initguid.h>
#include <dxva2api.h>

#define DXVA_INV32_SRC_BUFFER	(13)
#define DXVA_INV32_DST_BUFFER	(2)
#define DXVA_INV32_SPIC_BUFFER	(2)
#define DXVA_INV32_MIN_BUFFER	(DXVA_INV32_SRC_BUFFER+DXVA_INV32_DST_BUFFER)
#define DXVA_INV32_MAX_BUFFER	(DXVA_INV32_SRC_BUFFER+DXVA_INV32_DST_BUFFER+DXVA_INV32_SPIC_BUFFER)

// for AMD M56 
#define DXVA_ATI_RESTRICTED_MODE_H264_A					 0x101

using namespace HVDService;

CHVDDSAMVANotify::CHVDDSAMVANotify(CBasePin *popin, LPUNKNOWN pUnk) : CUnknown(_T("CHVDDSAMVANotify"),pUnk)
{
	m_poutputpin = popin;
	m_uUncompSurfacesAllocated = 0;
	ZeroMemory(&m_UncompSurfacesConfig, sizeof(HVDService::HVDDxva1UncompSurfConfig));
}

STDMETHODIMP CHVDDSAMVANotify::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
	CheckPointer(ppv,E_POINTER);
	ValidateReadWritePtr(ppv,sizeof(PVOID));
	*ppv = 0;
	if(riid==IID_IHVDDXVA1AMVANofityConfig)
		return GetInterface((IHVDDXVA1AMVANofityConfig *)this,ppv);
	if(riid==IID_IiviVideoAcceleratorNotify2)
		return GetInterface((IiviVideoAcceleratorNotify2 *)this,ppv);
	if(riid==IID_IiviVideoAcceleratorNotify)
		return GetInterface((IiviVideoAcceleratorNotify *)this,ppv);
	if(riid==IID_IAMVideoAcceleratorNotify)
		return GetInterface((IAMVideoAcceleratorNotify *)this,ppv);
	return CUnknown::NonDelegatingQueryInterface(riid, ppv);
}

//////////////////////////////////////////////////////////
// IAMVideoAcceleratorNotify
STDMETHODIMP CHVDDSAMVANotify::GetUncompSurfacesInfo
(const GUID *pGuid, LPAMVAUncompBufferInfo pUncompBufferInfo)
{
	IAMVideoAccelerator *pAccel;
	HRESULT hr;
	
	if(!m_poutputpin->GetConnected())
		return VFW_E_NOT_CONNECTED;
	
	// connect up
	hr = m_poutputpin->GetConnected()->QueryInterface(IID_IAMVideoAccelerator, (void **)&pAccel);
	if(FAILED(hr))
		return hr;
	LPDDPIXELFORMAT pFormats=NULL;
	DWORD dwFormats = 0;
	
	//  get number of supported formats 
	hr = pAccel->GetUncompFormatsSupported(pGuid, &dwFormats, NULL);
	if (FAILED(hr))
	{
		if(pFormats) {free(pFormats); pFormats = NULL;}
		pAccel->Release();
		return hr;
	}
	
	// get all supported formats
	pFormats = (LPDDPIXELFORMAT) malloc(dwFormats*sizeof(DDPIXELFORMAT));
	hr = pAccel->GetUncompFormatsSupported(pGuid, &dwFormats, pFormats);
	if (FAILED(hr) || dwFormats<1)
	{
		if(pFormats) {free(pFormats); pFormats = NULL;}
		pAccel->Release();
		return hr==S_OK ? E_FAIL : hr;
	}
	
	// propose format
	DWORD dwMin, dwMax;
	SetUncompSurfNum(pGuid, &dwMin, &dwMax);
	pUncompBufferInfo->dwMinNumSurfaces = dwMin;
	pUncompBufferInfo->dwMaxNumSurfaces = dwMax;
	pUncompBufferInfo->ddUncompPixelFormat = pFormats[0];
	if(pFormats) {free(pFormats); pFormats = NULL;}
	pAccel->Release();
	return S_OK;
}

STDMETHODIMP CHVDDSAMVANotify::SetUncompSurfacesInfo(DWORD dwActualUncompSurfacesAllocated)
{
	m_uUncompSurfacesAllocated = dwActualUncompSurfacesAllocated;
	return S_OK;
}

STDMETHODIMP CHVDDSAMVANotify::GetUncompSurfacesAllocated(DWORD *pUncompSurfacesAllocated)
{
	*pUncompSurfacesAllocated = m_uUncompSurfacesAllocated;
	return S_OK;
}

STDMETHODIMP CHVDDSAMVANotify::GetUncompSurfacesRequired(GUID* pDecGuid, ULONG* pMin, ULONG* pMax)
{
	SetUncompSurfNum(pDecGuid, pMin, pMax);
	return S_OK;
}

STDMETHODIMP CHVDDSAMVANotify::GetCreateVideoAcceleratorData
(const GUID *pGuid, LPDWORD pdwSizeMiscData, LPVOID *ppMiscData)
{
	DXVA_ConnectMode	*pConnectMode;
	WORD				dxva_mode;
	
	if(*pGuid == DXVA_ModeMPEG2_D)	
	{
		dxva_mode = DXVA_RESTRICTED_MODE_MPEG2_D;
	}
	else if(*pGuid == DXVA_ModeMPEG2_C)	
	{
		dxva_mode = DXVA_RESTRICTED_MODE_MPEG2_C;
	}
	else if(*pGuid == DXVA_ModeMPEG2_B)	
	{
		dxva_mode = DXVA_RESTRICTED_MODE_MPEG2_B;
	}
	else if(*pGuid == DXVA_ModeMPEG2_A)	
	{
		dxva_mode = DXVA_RESTRICTED_MODE_MPEG2_A;
	}
	else if(*pGuid == DXVA_ModeMPEG1_A)	
	{
		dxva_mode = DXVA_RESTRICTED_MODE_MPEG1_A;
	}
	else if(*pGuid == DXVA_ModeVC1_A)
	{
		dxva_mode = DXVA_RESTRICTED_MODE_VC1_A;
	}
	else if(*pGuid == DXVA_ModeVC1_B)
	{
		dxva_mode = DXVA_RESTRICTED_MODE_VC1_B;
	}
	else if(*pGuid == DXVA_ModeVC1_C)
	{
		dxva_mode = DXVA_RESTRICTED_MODE_VC1_C;
	}
	else if(*pGuid == DXVA_ModeVC1_D)
	{
		dxva_mode = DXVA_RESTRICTED_MODE_VC1_D;
	}
	else if(*pGuid == DXVA2_Intel_ModeVC1_D)
	{
		dxva_mode = DXVA_RESTRICTED_INTEL_MODE_VC1_D;
	}
	else if(*pGuid == DXVA_ModeH264_ATI_A)
	{
		dxva_mode = DXVA_ATI_RESTRICTED_MODE_H264_A; //MS: DXVA_RESTRICTED_MODE_H264_A
	}
	else if(*pGuid == DXVA_ModeH264_ATI_B)
	{
		dxva_mode = DXVA_RESTRICTED_MODE_H264_B;
	}
	else if(*pGuid == DXVA_ModeH264_E)
	{
		dxva_mode = DXVA_RESTRICTED_MODE_H264_E;
	}
	else if(*pGuid == DXVA_ATI_BA_H264)
	{
		dxva_mode = DXVA_RESTRICTED_MODE_H264_C;
	}
	else if(*pGuid == DXVA_ModeH264_F)
	{
		dxva_mode = DXVA_RESTRICTED_MODE_H264_F;
	}
	else	// original HVA code
	{
		*pdwSizeMiscData = 0;
		ppMiscData = NULL;
		return S_OK;
	}
	
	if(!(pConnectMode = (DXVA_ConnectMode *)CoTaskMemAlloc(sizeof(DXVA_ConnectMode))))
	{
		return E_FAIL;
	}
	
	ZeroMemory(pConnectMode, sizeof(DXVA_ConnectMode));
	pConnectMode->guidMode			= *pGuid;
	pConnectMode->wRestrictedMode	= dxva_mode;
	*pdwSizeMiscData				= sizeof(DXVA_ConnectMode);
	*ppMiscData						= (void*)pConnectMode;
	
	return S_OK;
}

void CHVDDSAMVANotify::SetUncompSurfNum(const GUID *pGuid, unsigned long *pMin, unsigned long *pMax)
{
	unsigned long dmin=0, bmin=0, bmax=0, bminbd=0;
	IAMVideoAccelerator *pAccel = 0;
	CDisplayDevItem disp_dev;
	HRESULT hr;
	
	// Handle special cases here
	if(SUCCEEDED(CUtilGPU::GetDisplayDeviceType("", disp_dev)))
	{
		// 1. Work around for VIA VT7002 DXVA issue. The frame buffer is only 4 buffers.
		if(disp_dev.m_VendorId == PCI_VENDOR_ID_WLKN || disp_dev.m_VendorId == PCI_VENDOR_ID_VIA )
		{
			if( disp_dev.m_DeviceId == PCI_DEVICE_ID_WLKN_VT3122 || 
				disp_dev.m_DeviceId == PCI_DEVICE_ID_VT3123 )
			{
				dmin = bmin = 4;
			}
		}
		// 2. Detection of Nvidia Inv32 Capable Driver and GPU
		if(disp_dev.m_VendorId == PCI_VENDOR_ID_NVIDIA)
		{
			// the following calls may cause trouble with intel driver
			hr = m_poutputpin->GetConnected()->QueryInterface(IID_IAMVideoAccelerator, (void **)&pAccel);
			if(SUCCEEDED(hr) && (2 == m_UncompSurfacesConfig.dwDXVAInv32) && pAccel )
			{
				DWORD	dwNsGUID = 20;
				GUID*	guid = new GUID[dwNsGUID]; 
				
				memset(guid,0,sizeof(guid[0])*dwNsGUID);
				pAccel->GetVideoAcceleratorGUIDs(&dwNsGUID,guid);
				for(unsigned int i=0;i<dwNsGUID;i++)
				{
					if(guid[i]==DXVA_Inverse32PullDown)
					{
						if(*pGuid==DXVA_ModeMPEG2_A || *pGuid==DXVA_ModeMPEG2_C  || *pGuid==DXVA_ModeMPEG2_VLD)
						{
							bmax = DXVA_INV32_MIN_BUFFER;
							bmin = DXVA_INV32_MIN_BUFFER;
						}
						if(*pGuid==DXVA_ModeMPEG2_B || *pGuid==DXVA_ModeMPEG2_D)
						{
							bmax = DXVA_INV32_MAX_BUFFER;
							bmin = DXVA_INV32_MIN_BUFFER;
						}
						break;
					}
				}
				delete[]guid;
			}

			if (pAccel)
			{
				pAccel->Release();
				pAccel = NULL;
			}
		}
	}
	
	dmin = m_UncompSurfacesConfig.dwMinDecodeCount ? m_UncompSurfacesConfig.dwMinDecodeCount : dmin;
	bmin = m_UncompSurfacesConfig.dwMinSurfaceCount ? m_UncompSurfacesConfig.dwMinSurfaceCount : bmin;
	bmax = m_UncompSurfacesConfig.dwMaxSurfaceCount ? m_UncompSurfacesConfig.dwMaxSurfaceCount : bmax;
	bminbd = m_UncompSurfacesConfig.dwMinsurfaceCountforBD ? m_UncompSurfacesConfig.dwMinsurfaceCountforBD : bminbd;

	if(dmin && dmin<4)
		dmin = 4;
	if(bmin<dmin)
		bmin = dmin;
	if(bmax<bmin)
		bmax = bmin;
	
	
	if (*pGuid==DXVA_ATI_BA_H264 || *pGuid==DXVA_ModeH264_E || *pGuid==DXVA_ModeH264_F)
	{
		*pMin = bmin ? bmin : 9;
		*pMax = bmax ? bmax : 12;
	}
	else if (*pGuid==DXVA_ModeH264_ATI_A || *pGuid==DXVA_ModeH264_ATI_B)
	{
		*pMin = bmin ? bmin : 9;
		*pMax = bmax ? bmax : 12;
	}
	else if (*pGuid==DXVA_ModeH264_VP1)
	{
		*pMin = bmin ? bmin : 12;
		*pMax = bmax ? bmax : 15; 
	}
	else if (*pGuid==DXVA_ModeVC1_A || *pGuid==DXVA_ModeVC1_B || *pGuid==DXVA_ModeVC1_C || *pGuid==DXVA_ModeVC1_D || *pGuid==DXVA2_Intel_ModeVC1_D)
	{
		*pMin = bmin ? bmin : 12;
		*pMax = bmax ? bmax : 15; 
	}
	// default min should be 5 to be consistant with decoder
	// otherwise, use DXVADECMIN to change both sides
	else if (*pGuid==DXVA_ModeMPEG2_B)
	{
		*pMin = bminbd ? bminbd : (bmin ? bmin : 5);
		*pMax = bmax ? bmax : 8;
	}
	else  if(*pGuid==DXVA_ModeMPEG2_D)
	{
		*pMin = bminbd ? bminbd : (bmin ? bmin : 5);
		*pMax = bmax ? bmax : 8;
	}
	else if(*pGuid==DXVA_ModeMPEG2_A 
		 || *pGuid==DXVA_ModeMPEG2_C
		 || *pGuid==DXVA2_ModeMPEG2_IDCT
		 || *pGuid==DXVA2_ModeMPEG2_MOCOMP
		 || *pGuid==DXVA2_ModeMPEG2_VLD)
	{
		*pMin = bmin ? bmin : (dmin ? dmin : 5);
		*pMax = bmax ? bmax : 8;
	}
	else if(*pGuid==DXVA_ModeMPEG1_A)
	{
		*pMin = 4;
		*pMax = 5;
	}
	// original HVA code
	else if (*pGuid==MEDIASUBTYPE_I81X) //  i81x HVA
	{
		*pMin = 4;
		*pMax = 5;
	}
	else
	{
		*pMin = 4;
		*pMax = 4;
	}
}
	
STDMETHODIMP CHVDDSAMVANotify::SetUncompSurfacesConfig(HVDService::HVDDxva1UncompSurfConfig *pUncompSurfacesConfig)
{
	memcpy(&m_UncompSurfacesConfig, pUncompSurfacesConfig, sizeof(HVDService::HVDDxva1UncompSurfConfig));
	return S_OK;
}