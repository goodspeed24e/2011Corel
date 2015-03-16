#ifndef _CDXVAACC_H
#define _CDXVAACC_H

#include <initguid.h>
#include <videoacc.h>
#include "Exports\Inc\HVDService.h"

DEFINE_GUID(IID_IiviVideoAcceleratorNotify,
0x91252824, 0xaf21, 0x11d4, 0xa9, 0xc, 0x0, 0x50, 0xda, 0xce, 0x24, 0x36);

DECLARE_INTERFACE_(IiviVideoAcceleratorNotify, IAMVideoAcceleratorNotify)
	{
	STDMETHOD(GetUncompSurfacesAllocated)(
		THIS_
		OUT DWORD* pUncompSurfacesAllocated
		) PURE;
	};
// {9DD2F51F-EDAA-4d26-8727-4DFEB6FDC667}
DEFINE_GUID(IID_IiviVideoAcceleratorNotify2,
0x9dd2f51f, 0xedaa, 0x4d26, 0x87, 0x27, 0x4d, 0xfe, 0xb6, 0xfd, 0xc6, 0x67);

DECLARE_INTERFACE_(IiviVideoAcceleratorNotify2, IiviVideoAcceleratorNotify)
	{
	STDMETHOD(GetUncompSurfacesRequired)(
		THIS_
		IN	GUID* pDecGuid,
		OUT ULONG* pMin,
		OUT ULONG* pMax
		) PURE;
	};

// {1F0B4BE3-E8F8-4870-877D-3779A4F2EC86}
DEFINE_GUID(IID_IHVDDXVA1AMVANofityConfig, 
0x1f0b4be3, 0xe8f8, 0x4870, 0x87, 0x7d, 0x37, 0x79, 0xa4, 0xf2, 0xec, 0x86);

DECLARE_INTERFACE_(IHVDDXVA1AMVANofityConfig, IiviVideoAcceleratorNotify2)
	{
		STDMETHOD(SetUncompSurfacesConfig)(
			THIS_
			IN	HVDService::HVDDxva1UncompSurfConfig *pUncompSurfacesConfig
			) PURE;
	};

class CBasePin;

namespace HVDService
{
	class CHVDDSAMVANotify : public IHVDDXVA1AMVANofityConfig, public CUnknown
	{

	public:
		CHVDDSAMVANotify(CBasePin *popin, LPUNKNOWN pUnk);
		DECLARE_IUNKNOWN
		// INonDelegatingUnknown
		STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);
		// IAMVideoAcceleratorNotify
		STDMETHODIMP	GetUncompSurfacesInfo(const GUID *pGuid, LPAMVAUncompBufferInfo pUncompBufferInfo);
		STDMETHODIMP	SetUncompSurfacesInfo(DWORD dwActualUncompSurfacesAllocated);
		STDMETHODIMP	GetCreateVideoAcceleratorData(const GUID *pGuid, LPDWORD pdwSizeMiscData, LPVOID *ppMiscData);
		// IiviVideoAcceleratorNotify
		STDMETHODIMP	GetUncompSurfacesAllocated(DWORD *pUncompSurfacesAllocated);
		// IiviVideoAcceleratorNotify2
		STDMETHODIMP	GetUncompSurfacesRequired(GUID* pDecGuid, ULONG* pMin, ULONG* pMax);
		// IHVDDXVA1AMVANofityConfig
		STDMETHODIMP	SetUncompSurfacesConfig(HVDService::HVDDxva1UncompSurfConfig *pUncompSurfacesConfig);
		//	STDMETHODIMP	GetUncompSurfacesConfig(GUID* pDecGuid, ULONG* pMin, ULONG* pMax);

	private:
		CBasePin		*m_poutputpin;
		unsigned long	m_uUncompSurfacesAllocated;
		HVDService::HVDDxva1UncompSurfConfig	m_UncompSurfacesConfig;

		void			SetUncompSurfNum(const GUID *pGuid, unsigned long *pMin, unsigned long *pMax);
	};
}
#endif /* _CDXVAACC_H */
