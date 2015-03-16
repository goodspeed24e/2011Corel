//==========================================================================
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
//  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
//  PURPOSE.
//
//  Copyright (c) 1998 - 2000  InterVideo Corporation.  All Rights Reserved.
//
//--------------------------------------------------------------------------
//
// DSHelper.h: interface for the CHVDDSHelper class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __DSHELPER_H__
#define __DSHELPER_H__

#include <streams.h>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// Filter Helper Macro
#if !defined(RELEASE)
#define RELEASE(x)	{	if ( (x) )								\
						{										\
						    ULONG cnt = (x)->Release();			\
							DbgLog( (LOG_TRACE,3,TEXT(">>>>> [RELEASE] " #x ": count = %d"), cnt) );	\
							(x) = NULL;							\
						}										\
					}
#endif

typedef enum //_PINCONNECTED 
{ 
	Pin_Ignore, 
	Pin_Connected, 
	Pin_Disconnected 
} PINCONNECTED;

namespace HVDService
{
	STDMETHODIMP AddIntoROT(IUnknown *pUnkGraph, DWORD *pdwRegister, PCWSTR wszMonName = 0);
	void STDMETHODCALLTYPE RemoveFromROT(DWORD pdwRegister);

	class CHVDDSHelper  
	{
	public:
		CHVDDSHelper(IGraphBuilder *piGraph = NULL);
		~CHVDDSHelper();

	public:

		HRESULT STDMETHODCALLTYPE FindConnectedToFilter
			(
			IPin*			piPin,
			FILTER_INFO*	pFltInfo,
			PIN_INFO*		pPinInfo 
			);

		void STDMETHODCALLTYPE RemoveDownStream(IBaseFilter *piFilter);
		HRESULT STDMETHODCALLTYPE RemoveDownStream(IPin *piPin);
		HRESULT STDMETHODCALLTYPE RemoveDownStream(IBaseFilter *piFilter, GUID guidPin);

		HRESULT STDMETHODCALLTYPE CountFilterPins
			(
			IBaseFilter*	piFilter,
			UINT*			pnInPins,
			UINT*			pnOutPins
			);

		IBaseFilter* STDMETHODCALLTYPE AddFilterInRegistry
			(
			REFCLSID		clsid, 
			LPCSTR			lpstrFiltername, 
			IBaseFilter**	ppiFilter
			);
		BOOL STDMETHODCALLTYPE GetIKsPropertySetSupport
			(
			REFGUID				guidPropSet, 
			DWORD				dwPropID, 
			IBaseFilter**		ppiFilter, 
			IKsPropertySet**	ppiKsPropertySet
			);
		HRESULT STDMETHODCALLTYPE FindEnumeratedDevice
			(
			REFCLSID		clsid, 
			IBaseFilter**	ppiFilter, 
			UINT			nDeviceID, 
			LPTSTR			pszName		= NULL
			);

		HRESULT STDMETHODCALLTYPE RemoveAllFilters();
		HRESULT STDMETHODCALLTYPE RemoveFilter(IBaseFilter **ppiFilter);

		UINT STDMETHODCALLTYPE CountSystemDevices(REFCLSID clsid);

		HRESULT STDMETHODCALLTYPE ConnectFilters
			(
			IBaseFilter*	piOutputFilter, 
			IBaseFilter*	piInputFilter
			);

		HRESULT STDMETHODCALLTYPE DisconnectAllPins(IBaseFilter *piFilter);

		HRESULT STDMETHODCALLTYPE DisconnectPin(IPin* piPin);

		HRESULT STDMETHODCALLTYPE WdmProxyService
			(
			int*			pnDeviceIndex, 
			REFGUID			guidCategory, 
			LPCSTR			szName, 
			IBaseFilter**	ppiProxy, 
			LPTSTR			szFriendlyName
			);

		HRESULT STDMETHODCALLTYPE AddProxyService
			(
			REFGUID			guidCategory, 
			LPCTSTR			szName, 
			IBaseFilter**	ppiProxy
			);

		HRESULT STDMETHODCALLTYPE AddProxyServiceByIndex
			(
			REFGUID			guidCategory, 
			int*			pnDeviceIndex, 
			IBaseFilter**	ppiProxy,
			LPTSTR			szFriendlyName	= NULL
			);

		HRESULT STDMETHODCALLTYPE AddFilter
			(
			REFCLSID		rClsid, 
			IBaseFilter**	ppiFilter, 
			LPCWSTR			pName		= NULL
			);

		HRESULT STDMETHODCALLTYPE GetFilterClsid
			(
			IBaseFilter*	piFilter, 
			CLSID*			pClsid
			);

		HRESULT STDMETHODCALLTYPE FindFilter
			(
			void**			ppiFilter, 
			REFIID			riid, 
			REFCLSID		clsid		= GUID_NULL
			);

		HRESULT STDMETHODCALLTYPE FindFilterByClsid
			( 
			IBaseFilter**	ppiFilter,
			REFCLSID		clsid 
			);

		HRESULT STDMETHODCALLTYPE FindFilterByName
			(
			IBaseFilter**	ppiFilter,
			LPCWSTR			pName
			);

		HRESULT STDMETHODCALLTYPE ConnectPins
			(
			IBaseFilter*	piFilterFrom, 
			PWCHAR			pwszFrom, 
			IBaseFilter*	piFilterTo, 
			PWCHAR			pwszTo				= NULL, 
			BOOL			bExactNameMatch		= FALSE, 
			BOOL			bDirectConnection	= TRUE
			);
		HRESULT STDMETHODCALLTYPE ConnectPins
			(
			IBaseFilter*	piFilterFrom, 
			GUID			guid,
			IBaseFilter*	piFilterTo, 
			PWCHAR			pwszTo				= NULL, 
			BOOL			bExactNameMatch		= FALSE, 
			BOOL			bDirectConnection	= TRUE
			);

		HRESULT STDMETHODCALLTYPE FindPin
			(
			IBaseFilter*	piFilter, 
			IPin**			ppiPin, 
			PIN_DIRECTION	pinDir, 
			PWCHAR			pwszName		= NULL, 
			BOOL			bExactNameMatch	= FALSE, 
			PINCONNECTED	pinConnected	= Pin_Ignore
			);

		HRESULT STDMETHODCALLTYPE FindPin
			(
			IBaseFilter*	piFilter, 
			IPin**			ppiPin, 
			GUID			guid, 
			BOOL			bCheckConnect	= TRUE
			);

		HRESULT STDMETHODCALLTYPE FindPin
			(
			IBaseFilter*	piFilter, 
			IPin**			ppiPin, 
			GUID			guid, 
			PIN_DIRECTION	pinDir,
			BOOL			bCheckConnect	= TRUE
			);

		HRESULT STDMETHODCALLTYPE FindPin
			(
			IBaseFilter*	piFilter, 
			IPin**			ppiPin, 
			GUID			guid,
			GUID			pinMajorType,
			BOOL			bCheckConnect	= TRUE
			);

		inline void STDMETHODCALLTYPE AssignGraphBuilder(IGraphBuilder *piGraph)
		{
			m_piGraph = piGraph;
		}
	protected:
		IGraphBuilder *m_piGraph;
	};
}
/////////////////////////////////////////////////////////////////////
#endif // __DSHELPER_H__