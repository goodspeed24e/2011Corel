//=============================================================================
//	THIS SOURCE CODE IS PROPRIETARY INFORMATION BELONGING TO INTERVIDEO, INC.
// 	ANY USE INCLUDING BUT NOT LIMITED TO COPYING OF CODE, CONCEPTS, AND/OR
//	ALGORITHMS IS PROHIBITED EXCEPT WITH EXPRESS WRITTEN PERMISSION BY THE
//	COMPANY.
//
// 	THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
// 	KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// 	IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
// 	PURPOSE.
//
// 	Copyright (c) 1998 - 2000  InterVideo Corporation.  All Rights Reserved.
//
//-----------------------------------------------------------------------------
//
// DSHelper.cpp: implementation of the CHVDDSHelper class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "HVDDSHelper.h"
#include <tchar.h>
#include <stdio.h>
//#include "../../shared/include/iviStrDef.h"

using namespace HVDService;

#define MonikerDelimeter L"!"

STDMETHODIMP HVDService::AddIntoROT(IUnknown *pUnkGraph,
	DWORD *pdwRegister, PCWSTR wszRotName) 
	{
    IMoniker *pMoniker;
    IRunningObjectTable *pROT;
	PWSTR wszMonikerName;
	PCWSTR wszcFilterGraph = L"FilterGraph %08x pid %08x";
	const size_t cnFilterGraphLen = wcslen(wszcFilterGraph);
	HRESULT hr;

	if(0 == wszRotName)
	{
		wszMonikerName = new WCHAR[cnFilterGraphLen + 32];
		if(0 == wszMonikerName)
			return E_OUTOFMEMORY;
		swprintf(wszMonikerName, sizeof(WCHAR) * cnFilterGraphLen + 32, wszcFilterGraph, pUnkGraph, GetCurrentProcessId());
	}
	hr = GetRunningObjectTable(0, &pROT);
	if (SUCCEEDED(hr))
	{
		hr = CreateItemMoniker(MonikerDelimeter,
			wszRotName ? wszRotName : wszMonikerName, &pMoniker);
		if(SUCCEEDED(hr))
		{
			hr = pROT->Register(ROTFLAGS_REGISTRATIONKEEPSALIVE, pUnkGraph,
				pMoniker, pdwRegister);
			pMoniker->Release();
		}
		pROT->Release();
	}

	if(0 == wszRotName)
		delete[] wszMonikerName;

	return hr;
	}

void STDMETHODCALLTYPE HVDService::RemoveFromROT(DWORD pdwRegister)
	{
    IRunningObjectTable *pROT;

    if(SUCCEEDED(GetRunningObjectTable(0, &pROT)))
		{
        pROT->Revoke(pdwRegister);
        pROT->Release();
		}
	}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CHVDDSHelper::CHVDDSHelper(IGraphBuilder *piGraph)
{
	m_piGraph = piGraph;
}

CHVDDSHelper::~CHVDDSHelper()
{
}

//////////////////////////////////////////////////////////////////////
// Function		: HRESULT FindPin
//							(
//								IBaseFilter*	piFilter, 
//								IPin**			ppiPin, 
//								GUID			guid, 
//								BOOL			bCheckConnect
//							)
// Description	: Find the pin
// Comments		:
//////////////////////////////////////////////////////////////////////
HRESULT CHVDDSHelper::FindPin
					(
						IBaseFilter*	piFilter, 
						IPin**			ppiPin, 
						GUID			guid, 
						BOOL			bCheckConnect
					)
{
	HRESULT hr;
	IEnumPins *piEnumPins = NULL;
	IKsPropertySet *piPropertySet = NULL;
	GUID pinType;
	DWORD dwReturn;

	// Check arguments to make sure that they are correct
	if ( !piFilter || !ppiPin )
	{
		DbgLog( (LOG_TRACE,3,TEXT("[FindPin] Invalid arguments.")) );
		return E_INVALIDARG;
	}
	
	// Enumerate the pins in the base filter
	hr = piFilter->EnumPins(&piEnumPins);

	if ( FAILED(hr) || !piEnumPins )
	{
		DbgLog( (LOG_TRACE,3,TEXT("[FindPin] Unable to enumerate the pins on the base filter.")) );
		return E_FAIL;
	}

	// Start going through the pin list
	hr = piEnumPins->Reset();
	if ( SUCCEEDED(hr) )
	{
		while( ((hr=piEnumPins->Next(1, ppiPin, NULL))==S_OK) && *ppiPin )
		{
			hr = NOERROR;

			if ( bCheckConnect )
			{
				IPin *piConnectedPin=NULL;

				hr = (*ppiPin)->ConnectedTo(&piConnectedPin);

				if ( SUCCEEDED(hr) && piConnectedPin )
				{
					RELEASE(piConnectedPin);
					hr = E_FAIL;
				}
				else
					hr = NOERROR;
			}

			// Check that the property set matches
			if ( SUCCEEDED(hr) )
			{
				hr = (*ppiPin)->QueryInterface
									(
										__uuidof(IKsPropertySet),
										reinterpret_cast< void** >(&piPropertySet)
									);
				if ( SUCCEEDED(hr) )
				{
					hr = piPropertySet->Get
										(
											AMPROPSETID_Pin,
											AMPROPERTY_PIN_CATEGORY,
											NULL, 
											0,
											&pinType,
											sizeof(pinType),
											&dwReturn
										);
					RELEASE(piPropertySet);

					if ( SUCCEEDED(hr) )
					{
						if (pinType == guid )
						{
							hr = NOERROR;
							break;
						}
					}
				}
			}

			RELEASE(*ppiPin);
		}
	}
	RELEASE(piEnumPins);

	if ( hr == S_FALSE )
		hr = E_FAIL;

	return hr;

}

//////////////////////////////////////////////////////////////////////
// Function		: HRESULT FindPin
//							(
//								IBaseFilter*	piFilter, 
//								IPin**			ppiPin, 
//								GUID			guid, 
//								PIN_DIRECTION	pinDir,
//								BOOL			bCheckConnect
//							)
// Description	: Find the pin
// Comments		:
//////////////////////////////////////////////////////////////////////
HRESULT CHVDDSHelper::FindPin
					(
						IBaseFilter*	piFilter, 
						IPin**			ppiPin, 
						GUID			guid, 
						PIN_DIRECTION	pinDir,
						BOOL			bCheckConnect
					)
{
	HRESULT hr;
	IEnumPins *piEnumPins = NULL;
	IKsPropertySet *piPropertySet = NULL;
	GUID pinType;
	DWORD dwReturn;

	// Check arguments to make sure that they are correct
	if ( !piFilter || !ppiPin )
	{
		DbgLog( (LOG_TRACE,3,TEXT("[FindPin] Invalid arguments.")) );
		return E_INVALIDARG;
	}
	
	// Enumerate the pins in the base filter
	hr = piFilter->EnumPins(&piEnumPins);

	if ( FAILED(hr) || !piEnumPins )
	{
		DbgLog( (LOG_TRACE,3,TEXT("[FindPin] Unable to enumerate the pins on the base filter.")) );
		return E_FAIL;
	}

	// Start going through the pin list
	hr = piEnumPins->Reset();
	if ( SUCCEEDED(hr) )
	{
		while( ((hr=piEnumPins->Next(1, ppiPin, NULL))==S_OK) && *ppiPin )
		{
			hr = NOERROR;

			PIN_INFO pinInfo;

			if ( SUCCEEDED( (*ppiPin)->QueryPinInfo(&pinInfo) ) )
			{
				// Release AddRef'd filter we don't need it
				RELEASE(pinInfo.pFilter);

				if ( pinInfo.dir != pinDir )
				{
					RELEASE(*ppiPin);
					continue;
				}
			}

			if ( bCheckConnect )
			{
				IPin *piConnectedPin=NULL;

				hr = (*ppiPin)->ConnectedTo(&piConnectedPin);

				if ( SUCCEEDED(hr) && piConnectedPin )
				{
					RELEASE(piConnectedPin);
					hr = E_FAIL;
				}
				else
					hr = NOERROR;
			}

			// Check that the property set matches
			if ( SUCCEEDED(hr) )
			{
				hr = (*ppiPin)->QueryInterface
									(
										__uuidof(IKsPropertySet),
										reinterpret_cast< void** >(&piPropertySet)
									);
				if ( SUCCEEDED(hr) )
				{
					hr = piPropertySet->Get
										(
											AMPROPSETID_Pin,
											AMPROPERTY_PIN_CATEGORY,
											NULL, 
											0,
											&pinType,
											sizeof(pinType),
											&dwReturn
										);
					RELEASE(piPropertySet);

					if ( SUCCEEDED(hr) )
					{
						if (pinType == guid )
						{
							hr = NOERROR;
							break;
						}
					}
				}
			}

			RELEASE(*ppiPin);
		}
	}
	RELEASE(piEnumPins);

	if ( hr == S_FALSE )
		hr = E_FAIL;

	return hr;

}

//////////////////////////////////////////////////////////////////////
// Function		: HRESULT FindPin
//							(
//								IBaseFilter*	piFilter, 
//								IPin**			ppiPin, 
//								PIN_DIRECTION	pinDir, 
//								PWCHAR			pwszName, 
//								BOOL			bExactNameMatch, 
//								PINCONNECTED	pinConnected
//							)
// Description	: Find the pin
// Comments		:
//////////////////////////////////////////////////////////////////////
HRESULT CHVDDSHelper::FindPin
					(
						IBaseFilter*	piFilter, 
						IPin**			ppiPin, 
						PIN_DIRECTION	pinDir, 
						PWCHAR			pwszName, 
						BOOL			bExactNameMatch, 
						PINCONNECTED	pinConnected
					)
{
	HRESULT hr;
	IEnumPins *piEnumPins=NULL;
	ULONG cFetchedPin=0;

	// Check arguments to make sure that they are correct
	if ( !piFilter || !ppiPin )
	{
		DbgLog( (LOG_TRACE,3,TEXT("[FindPin] Invalid arguments.")) );
		return E_INVALIDARG;
	}

	hr = piFilter->EnumPins(&piEnumPins);

	if ( FAILED(hr) || !piEnumPins )
	{
		DbgLog( (LOG_TRACE,3,TEXT("[FindPin] Unable to enumerate the pins on the base filter.")) );
		return E_FAIL;
	}

	hr = piEnumPins->Reset();
	if ( SUCCEEDED(hr) )
	{
		*ppiPin = NULL;

		while ( ((hr = piEnumPins->Next(1, ppiPin, &cFetchedPin))==NOERROR)
				&& (cFetchedPin==1) && *ppiPin )
		{
			PIN_INFO pinInfo;

			if ( SUCCEEDED( (*ppiPin)->QueryPinInfo(&pinInfo) ) )
			{
				// Release AddRef'd filter we don't need it
				RELEASE(pinInfo.pFilter);

				if ( pinInfo.dir == pinDir )
				{
					// Check pin's connection information
					if ( pinConnected != Pin_Ignore )
					{
						IPin *piConnectedPin=NULL;
						BOOL bSkip;

						if ( SUCCEEDED( (*ppiPin)->ConnectedTo(&piConnectedPin) ) 
								&& piConnectedPin )
						{
							bSkip = (pinConnected==Pin_Disconnected);
							RELEASE(piConnectedPin);
						}
						else
						{
							bSkip = (pinConnected==Pin_Connected);
						}

						if ( bSkip )
						{
							// Continue search if connection information is not satisfactory
							RELEASE(*ppiPin);
							continue;
						}
					}
					// Exit if
					// 1. We don't care about the pin's name
					if ( pwszName == NULL )
						break;
					// 2. pin's name are equal. If exact match is required, do plain
					//    string comparision. Otherwise just find specified string in
					//    pins name
					if ( bExactNameMatch )
					{
						if ( _wcsicmp(pinInfo.achName, pwszName) == 0 )
							break;
					}
					else
					{
						if( wcsstr( pinInfo.achName, pwszName ) )
							break;
					}
				}
			}
			RELEASE(*ppiPin);
		}
	}
	RELEASE(piEnumPins);

	if ( hr == S_FALSE )
		hr = E_FAIL;

	return hr;
}

//////////////////////////////////////////////////////////////////////
// Function		: HRESULT FindPin
//							(
//							IBaseFilter*	piFilter, 
//							IPin**			ppiPin, 
//							GUID			guid,
//							GUID			pinMajorType,
//							BOOL			bCheckConnect	= TRUE
//							)
// Description	: Find the pin based on major type and category
// Comments		:
//////////////////////////////////////////////////////////////////////

HRESULT CHVDDSHelper::FindPin
			(
				IBaseFilter*	piFilter, /*in*/
				IPin**			ppiPin, /*out*/
				GUID			guid,/*in*/
				GUID			pinMajorType,/*in*/
				BOOL			bCheckConnect
			)
{
	HRESULT hr;
	IEnumPins *piEnumPins = NULL;
	IKsPropertySet *piPropertySet = NULL;
	IEnumMediaTypes *piEnumMediaTypes = NULL;
	ULONG cFetched = 0;
	GUID pinType;
	DWORD dwReturn;

	// Check arguments to make sure that they are correct
	if ( !piFilter || !ppiPin )
	{
		DbgLog( (LOG_TRACE,3,TEXT("[FindPin] Invalid arguments.")) );
		return E_INVALIDARG;
	}
	
	// Enumerate the pins in the base filter
	hr = piFilter->EnumPins(&piEnumPins);

	if ( FAILED(hr) || !piEnumPins )
	{
		DbgLog( (LOG_TRACE,3,TEXT("[FindPin] Unable to enumerate the pins on the base filter.")) );
		return E_FAIL;
	}

	// Start going through the pin list
	hr = piEnumPins->Reset();
	if ( SUCCEEDED(hr) )
	{
		while( ((hr=piEnumPins->Next(1, ppiPin, NULL))==S_OK) && *ppiPin )
		{
			hr = NOERROR;

			if ( bCheckConnect )
			{
				IPin *piConnectedPin=NULL;

				hr = (*ppiPin)->ConnectedTo(&piConnectedPin);

				if ( SUCCEEDED(hr) && piConnectedPin )
				{
					RELEASE(piConnectedPin);
					hr = E_FAIL;
				}
				else
					hr = NOERROR;
			}

			// Check that the property set matches
			if ( SUCCEEDED(hr) )
			{
				hr = (*ppiPin)->QueryInterface
									(
										__uuidof(IKsPropertySet),
										reinterpret_cast< void** >(&piPropertySet)
									);
				if ( SUCCEEDED(hr) )
				{
					hr = piPropertySet->Get
										(
											AMPROPSETID_Pin,
											AMPROPERTY_PIN_CATEGORY,
											NULL, 
											0,
											&pinType,
											sizeof(pinType),
											&dwReturn
										);
					RELEASE(piPropertySet);

					if ( SUCCEEDED(hr) )
					{
						if (pinType == guid ) // category matches
						{
							// Check for Major type match
							hr = (*ppiPin)->EnumMediaTypes(&piEnumMediaTypes);
							if ( FAILED(hr) || !piEnumMediaTypes )
								{
								DbgLog( (LOG_TRACE,3,TEXT("[FindPin (AK)] Unable to enumerate the Media Types on this Pin")) );
								return E_FAIL;
								}		

							hr = piEnumMediaTypes->Reset();
							if (SUCCEEDED(hr))
								{
								AM_MEDIA_TYPE *pMediaType;

								hr = piEnumMediaTypes->Next(1, &pMediaType, &cFetched);
								RELEASE(piEnumMediaTypes);

								if (hr == S_OK)
									{
									if (pMediaType->majortype == pinMajorType)
										{
										DeleteMediaType(pMediaType);
										hr = NOERROR;
										break;
										}
									}
								else
									{
									DbgLog( (LOG_TRACE,3,TEXT("[FindPin (AK)] Unsuccessful in getting the next media")) );
									return E_FAIL;
									}
								}
						}
					}
				}
			}

			RELEASE(*ppiPin);
		}
	}
	RELEASE(piEnumPins);

	if ( hr == S_FALSE )
		hr = E_FAIL;

	return hr;
}
				
//////////////////////////////////////////////////////////////////////
// Function		: HRESULT ConnectPins
//								(
//									IBaseFilter*	piFilterFrom, 
//									PWCHAR			pwszFrom, 
//									IBaseFilter*	piFilterTo, 
//									PWCHAR			pwszTo, 
//									BOOL			bExactNameMatch, 
//									BOOL			bDirectConnection
//								)
// Description	: Connect pins
// Comments		:
//////////////////////////////////////////////////////////////////////
HRESULT CHVDDSHelper::ConnectPins
						(
							IBaseFilter*	piFilterFrom, 
							PWCHAR			pwszFrom, 
							IBaseFilter*	piFilterTo, 
							PWCHAR			pwszTo, 
							BOOL			bExactNameMatch, 
							BOOL			bDirectConnection
						)
{
	IPin	*piOutputPin=NULL;
	IPin	*piInputPin=NULL;
	HRESULT hr;

	if ( m_piGraph==NULL )
		return E_INVALIDARG;

	hr = FindPin
			(
				piFilterFrom, 
				&piOutputPin, 
				PINDIR_OUTPUT, 
				pwszFrom, 
				bExactNameMatch,
				Pin_Disconnected
			);

	if ( (hr == NOERROR) && piOutputPin )
	{
		hr = FindPin
				(
					piFilterTo, 
					&piInputPin, 
					PINDIR_INPUT, 
					pwszTo, 
					bExactNameMatch, 
					Pin_Disconnected
				);

		if ( (hr==NOERROR) && piInputPin )
		{
			// Connect output pin to input pin. Perform direct connection if
			// bDirectConnection is specified. Otherwise rely on DirectShow to add necessary
			// intermediate filter(s)
			if ( bDirectConnection )
				hr = m_piGraph->ConnectDirect(piOutputPin, piInputPin, NULL);
			else
				hr = m_piGraph->Connect(piOutputPin, piInputPin);
		RELEASE(piInputPin);
		}
	RELEASE(piOutputPin);
	}

	return hr;
}

//////////////////////////////////////////////////////////////////////
// Function		: HRESULT ConnectPins
//								(
//									IBaseFilter*	piFilterFrom, 
//									GUID			guid,
//									IBaseFilter*	piFilterTo, 
//									PWCHAR			pwszTo, 
//									BOOL			bExactNameMatch, 
//									BOOL			bDirectConnection
//								)
// Description	: Connect pins
// Comments		:
//////////////////////////////////////////////////////////////////////
HRESULT CHVDDSHelper::ConnectPins
						(
							IBaseFilter*	piFilterFrom, 
							GUID			guid,
							IBaseFilter*	piFilterTo, 
							PWCHAR			pwszTo, 
							BOOL			bExactNameMatch, 
							BOOL			bDirectConnection
						)
{
	IPin	*piOutputPin=NULL;
	IPin	*piInputPin=NULL;
	HRESULT hr;

	if ( m_piGraph==NULL )
		return E_INVALIDARG;

	hr = FindPin
			(
				piFilterFrom, 
				&piOutputPin, 
				guid, 
				PINDIR_OUTPUT, 
				TRUE
			);

	if ( (hr == NOERROR) && piOutputPin )
	{
		hr = FindPin
				(
					piFilterTo, 
					&piInputPin, 
					PINDIR_INPUT, 
					pwszTo, 
					bExactNameMatch, 
					Pin_Disconnected
				);

		if ( (hr==NOERROR) && piInputPin )
		{
			// Connect output pin to input pin. Perform direct connection if
			// bDirectConnection is specified. Otherwise rely on DirectShow to add necessary
			// intermediate filter(s)
			if ( bDirectConnection )
				hr = m_piGraph->ConnectDirect(piOutputPin, piInputPin, NULL);
			else
				hr = m_piGraph->Connect(piOutputPin, piInputPin);
		RELEASE(piInputPin);
		}
	RELEASE(piOutputPin);
	}

	return hr;
}

//////////////////////////////////////////////////////////////////////
// Function		: HRESULT FindFilter
//								(
//									void **ppiFilter, 
//									REFIID riid, 
//									REFCLSID clsid
//								)
// Description	: Find filter by REFIID and/or REFCLSID
// Comments		:
//
// If riid is GUID_NULL then 
//		ppiFilter is set to IBaseFilter
// otherwise 
//		ppiFilter is set to the appropriate interface.
//
// returns S_OK if the interface is found and ppiFilter is set.
//			an error code otherwise.
// 
//////////////////////////////////////////////////////////////////////

HRESULT CHVDDSHelper::FindFilter
						(
							void **ppiFilter, 
							REFIID riid, 
							REFCLSID clsid
						)
	{
	HRESULT hr;
	ULONG cFetched = 0;
	IBaseFilter *piNextFilter = 0;
	IEnumFilters *piEnumFilters = 0;
	
	if(m_piGraph==0 || ppiFilter==0)
		return E_INVALIDARG;
	*ppiFilter = 0;
	hr = m_piGraph->EnumFilters(&piEnumFilters);
	if(FAILED(hr))
		return hr;
	hr = piEnumFilters->Reset();
	if(FAILED(hr))
		{
		RELEASE(piEnumFilters);
		return hr;
		}
	while((hr=piEnumFilters->Next(1, &piNextFilter, &cFetched))==S_OK)
		{
		if(riid!=IID_NULL)
			{	// query interface
			hr = piNextFilter->QueryInterface(riid, ppiFilter);
			if(SUCCEEDED(hr))
				{
				RELEASE(piNextFilter);
				hr = S_OK;
				break;
				}
			}
		else
			{	// if no interface, can return IBaseFilter if CLSID matches
			CLSID id;

			hr = GetFilterClsid(piNextFilter, &id);
			if(SUCCEEDED(hr))
				{
				if(clsid==id)
					{
					*ppiFilter = piNextFilter;
					hr = S_OK;
					break;
					}
				}
			}
		RELEASE(piNextFilter);
		}
	if(hr==S_FALSE)		//enumerator returns s_false if no more filters.
		hr = E_FAIL;
	RELEASE(piEnumFilters);
	return hr;
	}

//////////////////////////////////////////////////////////////////////
// Function		: HRESULT FindFilterByClsid
//								( 
//									IBaseFilter**	ppiFilter,
//									REFCLSID		clsid 
//								)
// Description	: Find filter by CLSID
// Comments		:
//////////////////////////////////////////////////////////////////////
HRESULT CHVDDSHelper::FindFilterByClsid
								( 
									IBaseFilter**	ppiFilter,
									REFCLSID		clsid 
								)
{
	return FindFilter
				(
					reinterpret_cast< void** >( ppiFilter ), 
					IID_NULL, 
					clsid 
				);
}

//////////////////////////////////////////////////////////////////////
// Function		: HRESULT FindFilterByName
//								( 
//									IBaseFilter**	ppiFilter,
//									LPCWSTR			pName
//								)
// Description	: Find filter by name
// Comments		:
//////////////////////////////////////////////////////////////////////
HRESULT CHVDDSHelper::FindFilterByName
								(
									IBaseFilter**	ppiFilter,
									LPCWSTR			pName
								)
{
	if ( m_piGraph==NULL || !ppiFilter || !pName)
		return E_INVALIDARG;

	return m_piGraph->FindFilterByName(pName, ppiFilter);
}

//////////////////////////////////////////////////////////////////////
// Function		: HRESULT GetFilterClsid
//									(
//										IBaseFilter *piFilter, 
//										CLSID *pClsid
//									)
// Description	: Get filter's CLSID
// Comments		:
//////////////////////////////////////////////////////////////////////
HRESULT CHVDDSHelper::GetFilterClsid
							(
								IBaseFilter *piFilter, 
								CLSID *pClsid
							)
{
	// Ask for IPersist interface and get class ID from it
	IPersist*	piPersist;
	HRESULT		hr = piFilter->QueryInterface
								(
									__uuidof( IPersist ),
									reinterpret_cast< void** >( &piPersist )
								);
	if( SUCCEEDED( hr ) )
	{
		hr = piPersist->GetClassID( pClsid );
		RELEASE(piPersist);
	}

	return hr;
}

//////////////////////////////////////////////////////////////////////
// Function		: HRESULT AddFilter
//								(
//									REFCLSID rClsid, 
//									IBaseFilter **ppiFilter, 
//									LPCWSTR pName
//								)
// Description	: Add filter into filter graph
// Comments		:
//////////////////////////////////////////////////////////////////////
HRESULT CHVDDSHelper::AddFilter
						(
							REFCLSID rClsid, 
							IBaseFilter **ppiFilter, 
							LPCWSTR pName
						)
{
	HRESULT hr = E_FAIL;

	if ( (m_piGraph == NULL) || !ppiFilter )
	{
		return E_INVALIDARG;
	}

	hr = CoCreateInstance
					(
						rClsid,
						NULL,
						CLSCTX_INPROC_SERVER,
						__uuidof( IBaseFilter ),
						reinterpret_cast< void** >( ppiFilter )
					);
	if( SUCCEEDED( hr ) )
	{
		hr = m_piGraph->AddFilter( *ppiFilter, pName );
	}

	return hr;
}

//////////////////////////////////////////////////////////////////////
// Function		: HRESULT AddProxyServiceByIndex
//								(
//									REFGUID			guidCategory, 
//									int*			pnDeviceIndex, 
//									IBaseFilter**	ppiProxy,
//									LPTSTR			szFriendlyName
//								)
// Description	: Add proxy service into filter graph
// Comments		:
//////////////////////////////////////////////////////////////////////
HRESULT CHVDDSHelper::AddProxyServiceByIndex
							(
								REFGUID			guidCategory, 
								int*			pnDeviceIndex, 
								IBaseFilter**	ppiProxy,
								LPTSTR			szFriendlyName
							)
{
	if ( !m_piGraph || !ppiProxy )
	{
		DbgLog( (LOG_TRACE,3,TEXT("[AddProxyServiceByIndex] Invalid arguments.")) );
		return E_INVALIDARG;
	}

	HRESULT hr = WdmProxyService( pnDeviceIndex, guidCategory, NULL, ppiProxy, szFriendlyName );
	if( SUCCEEDED(hr) )
	{
		if ( szFriendlyName )
		{
			WCHAR wPath[MAX_PATH];
			MultiByteToWideChar( CP_ACP, 0, szFriendlyName, -1, wPath, MAX_PATH );
			hr = m_piGraph->AddFilter( *ppiProxy, wPath );
		}
		else
			hr = m_piGraph->AddFilter( *ppiProxy, NULL );
	}

	return hr;
}
//////////////////////////////////////////////////////////////////////
// Function		: HRESULT AddProxyService
//								(
//									REFGUID guidCategory, 
//									LPCTSTR szName, 
//									IBaseFilter **ppiProxy
//								)
// Description	: Add proxy service into filter graph
// Comments		:
//////////////////////////////////////////////////////////////////////
HRESULT CHVDDSHelper::AddProxyService
							(
								REFGUID			guidCategory, 
								LPCTSTR			szName, 
								IBaseFilter**	ppiProxy
							)
{
	HRESULT hr;
	TCHAR szFriendlyName[MAX_PATH];

	if ( !m_piGraph || !ppiProxy )
	{
		DbgLog( (LOG_TRACE,3,TEXT("[AddProxyService] Invalid arguments.")) );
		return E_INVALIDARG;
	}

	hr = WdmProxyService(NULL, guidCategory, szName, ppiProxy, szFriendlyName);

	if ( SUCCEEDED(hr) )
	{
		WCHAR wName[160];
		MultiByteToWideChar(CP_ACP, 0, szFriendlyName, -1, wName, MAX_PATH);
		hr = m_piGraph->AddFilter(*ppiProxy, wName);
	}

	return hr;
}

//////////////////////////////////////////////////////////////////////
// Function		: HRESULT WdmProxyService
//								(
//									int*			pnDeviceIndex, 
//									REFGUID			guidCategory, 
//									LPCSTR			szName, 
//									IBaseFilter**	ppiProxy, 
//									LPTSTR			szFriendlyName
//								)
// Description	: Find proxy filter from WDM proxy service
// Comments		:
//////////////////////////////////////////////////////////////////////
HRESULT CHVDDSHelper::WdmProxyService
						(
							int*			pnDeviceIndex, 
							REFGUID			guidCategory, 
							LPCSTR			szName, 
							IBaseFilter**	ppiProxy, 
							LPTSTR			szFriendlyName
						)
{
	HRESULT			hr;
	IEnumMoniker*	piEnumMoniker=NULL;
	ICreateDevEnum*	piCreateDevEnum=NULL;
	ULONG			cFetched=0;
	IMoniker*		piMoniker=NULL;
	int				nIndex = -1;
	int				nDeviceIndex = -1;
#ifdef _DEBUG
	BOOL			bOnceRestarted	= FALSE;
#endif

	if ( pnDeviceIndex != NULL )
		nDeviceIndex = *pnDeviceIndex;
	if ( *ppiProxy != NULL )
		ppiProxy = NULL;

	// Create device enumerator object
	hr = CoCreateInstance
					(
						CLSID_SystemDeviceEnum,
						NULL,
						CLSCTX_INPROC_SERVER,
						__uuidof( ICreateDevEnum ),
						reinterpret_cast< void** >( &piCreateDevEnum )
					);
	if ( FAILED(hr) || !piCreateDevEnum )
	{
		DbgLog( (LOG_TRACE,3,TEXT("[WdmProxyService] Unable to enumerate system devices.")) );
		return CLASS_E_CLASSNOTAVAILABLE;
	}

	// Get class enumerator for specified WDM category
	hr = piCreateDevEnum->CreateClassEnumerator(guidCategory, &piEnumMoniker, 0);
	RELEASE(piCreateDevEnum);

	if( hr != NOERROR )
		return HRESULT_FROM_WIN32( ERROR_BAD_UNIT );

	if ( SUCCEEDED(piEnumMoniker->Reset()) )
	{
		for (;;)
		{
			// Enumerate WDM devices
			DbgLog( (LOG_TRACE,3,TEXT("[WdmProxyService] Enumerating WDM devices")) );
			while ( ((hr=piEnumMoniker->Next(1, &piMoniker, &cFetched)) == NOERROR)
						&& (cFetched==1) && piMoniker )
			{
				++nIndex;
				// If we need to fill out combo box with WDM devices names or current
				// WDM device has not been selected yet we have to extract the name
				// of this device to put it into combo box or/and to check whether it's an
				// AV device or not. But generally speaking we have to perform this checks
				// in any case because we probab;y have to skip some devices that we don't
				// recognize to mantain correct device index
				TCHAR			szDevice[512];
				IPropertyBag*	piBag=NULL;

				hr = piMoniker->BindToStorage
										(
											0,
											0,
											__uuidof( IPropertyBag ),
											reinterpret_cast< void** >( &piBag )
										);
				if( SUCCEEDED(hr) && piBag )
				{
					VARIANT var;
					var.vt = VT_BSTR;
					if( (hr = piBag->Read( L"FriendlyName", &var, NULL )) == NOERROR )
					{
#ifndef _UNICODE
						WideCharToMultiByte
									(
										CP_ACP,
										0,
										var.bstrVal,
										-1,
										szDevice,
										sizeof( szDevice ),
										NULL,
										NULL
									);
#else
						wcsncpy( szDevice, sizeof(WCHAR) * MAX_PATH, var.bstrVal, numof( szDevice ) );
#endif
						SysFreeString( var.bstrVal );
						if (szFriendlyName)
							strcpy_s(szFriendlyName, MAX_PATH, szDevice);
					}
					RELEASE(piBag);
				}

				if( SUCCEEDED( hr ) )
				{
					DbgLog( (LOG_TRACE,3,TEXT("[WdmProxyService] Device #%d: %s"), nIndex, szDevice) );

					if( nDeviceIndex < 0 && (szName == NULL || _tcsstr( szDevice, szName )) )
					{
						// If no device has been selected and this is AV device choose it
						// as default selection
						DbgLog( (LOG_TRACE,3,TEXT("[WdmProxyService] Device #%d is our device"), nIndex) );
						nDeviceIndex = nIndex;
					}
				}
				else
				{
					// We don't know how to handle this device and it's parameters so
					// just skip it and pretent that we didn't encounter it by
					// decrementing nIndex. Note that nIndex is not being decremented!
					--nIndex;
					RELEASE(piMoniker);
					cFetched = 0;
					continue;
				}

				if( nDeviceIndex >= 0 && nDeviceIndex == nIndex )
				{
					if( ppiProxy != NULL )
					{
						// Create filter object
						DbgLog( (LOG_TRACE,3,TEXT("[WdmProxyService] Creating instance of chosen device #%d"), nIndex) );
						hr = piMoniker->BindToObject
												(
													NULL,
													NULL,
													__uuidof( IBaseFilter ),
													reinterpret_cast< void** >( ppiProxy )
												);
					}
				}
				RELEASE(piMoniker);
				cFetched = 0;

				if( ppiProxy != NULL && *ppiProxy != NULL )
					break;
			}
			if( nIndex >= 0 && pnDeviceIndex != NULL )
			{
				// Some WDM devices are installed in this system
				if( nDeviceIndex < 0 || nDeviceIndex > nIndex )
				{
					// But
					//  - or default device has to been selected and we was not able to
					//    find an AV device 
					//  - or selected device index does not correspond to existing device
					// In this case we choose the very first found device as our default
					// selection
					nDeviceIndex = 0;			// It's our default selection
					if( ppiProxy != NULL )
					{
						// No proxy filter was instantiated due to incorrect device index
						// settings or due to absence of AV device. We have to repeat the
						// procedure
						nIndex = -1;
						piEnumMoniker->Reset();
#ifdef _DEBUG
						// We don't want to go into infinite cycle so set a debug check
						ASSERT( !bOnceRestarted );
						bOnceRestarted = TRUE;
#endif
						continue;
					}
				}
			}
			//else
				break;
		}
	}
	RELEASE(piEnumMoniker);

	return hr;
}

//////////////////////////////////////////////////////////////////////
// Function		: HRESULT DisconnectPin(IPin *piPin)
// Description	: Disconnect the pin
// Comments		:
//////////////////////////////////////////////////////////////////////
HRESULT CHVDDSHelper::DisconnectPin(IPin *piPin)
{
	HRESULT hr;
	IPin * pOtherPin = NULL;

	if ( (m_piGraph==NULL) || !piPin )
		return E_INVALIDARG;

	hr = piPin->ConnectedTo(&pOtherPin);

	if ( FAILED(hr) || !pOtherPin )
	{
		// If we come in here it means that the pin isnt connected
		//DbgLog( (LOG_TRACE,3,TEXT("[DisconnectPin] The pin not connected")) );
		return NOERROR;
	}

	hr = m_piGraph->Disconnect(piPin);
	if ( FAILED(hr) )
	{
		DbgLog( (LOG_TRACE,3,TEXT("[DisconnectPin] Unable to disconnect the pin")) );
		RELEASE(pOtherPin);
		return E_FAIL;
	}

	hr = m_piGraph->Disconnect(pOtherPin);
	RELEASE(pOtherPin);

	if ( FAILED(hr) )
	{
		DbgLog( (LOG_TRACE,3,TEXT("[DisconnectPin] Unable to diconnect the other pin")) );
		return E_FAIL;
	}

	return NOERROR;
}

//////////////////////////////////////////////////////////////////////
// Function		: HRESULT DisconnectAllPins(IBaseFilter *piFilter)
// Description	: Disconnect all the pins
// Comments		:
//////////////////////////////////////////////////////////////////////
HRESULT CHVDDSHelper::DisconnectAllPins(IBaseFilter *piFilter)
{
	IEnumPins *piEnumPins = NULL;
	IPin *piPin = NULL;
	ULONG cFetchedPin=0;
	HRESULT hr;

	if ( (m_piGraph==NULL) || !piFilter )
		return E_INVALIDARG;

	hr = piFilter->EnumPins(&piEnumPins);
	if ( FAILED(hr) || !piEnumPins )
	{
		DbgLog( (LOG_TRACE,3,TEXT("[DisconnectAllPins] Unable to enumerate the pins")) );
		return E_FAIL;
	}

	while ( ((hr=piEnumPins->Next(1, &piPin, &cFetchedPin)) == S_OK)
				&& (cFetchedPin==1) && piPin )
	{
		hr = DisconnectPin(piPin);
		if ( FAILED(hr) )
		{
			DbgLog( (LOG_TRACE,3,TEXT("[DisconnectAllPins] Unable to disconnect pin")) );
		}

		RELEASE(piPin);
		cFetchedPin = 0;
	}

	RELEASE(piEnumPins);
	
	return NOERROR;
}

//////////////////////////////////////////////////////////////////////
// Function		: HRESULT ConnectFilters(IBaseFilter *piOutputFilter, IBaseFilter *piInputFilter)
// Description	: Connect two filters
// Comments		:
//////////////////////////////////////////////////////////////////////
HRESULT CHVDDSHelper::ConnectFilters(IBaseFilter *piOutputFilter, IBaseFilter *piInputFilter)
{
	if (!piOutputFilter || !piInputFilter)
	{
		DbgLog( (LOG_TRACE,3,TEXT("[ConnectFilters] Invalid parameters")) );
		return E_INVALIDARG;
	}

	IPin *piOutputPin = NULL;
	IPin *piInputPin = NULL;
	HRESULT hr;

	// Search the filters for the correct pins
	hr = FindPin(piOutputFilter, &piOutputPin, PINDIR_OUTPUT);
	if ( FAILED(hr) || !piOutputPin )
	{
		DbgLog( (LOG_TRACE,3,TEXT("[ConnectFilters] Unable to find correct pin in output filter.")) );
		goto ConnectFiltersFail;
	}
	
	hr = FindPin(piInputFilter, &piInputPin, PINDIR_INPUT);
	if ( FAILED(hr) || !piInputPin )
	{
		DbgLog( (LOG_TRACE,3,TEXT("[ConnectFilters] Unable to find correct pin in input filter.")) );
		goto ConnectFiltersFail;
	}

	// MOMENT OF TRUTH!! Connect the pins
	hr = m_piGraph->Connect(piOutputPin, piInputPin);
	if ( FAILED(hr) )
	{
		DbgLog( (LOG_TRACE,3,TEXT("[ConnectFilters] Unable to connect the pins.")) );
		goto ConnectFiltersFail;
	}

	// Cleanup
	RELEASE(piOutputPin);
	RELEASE(piInputPin);

	return NOERROR;

ConnectFiltersFail:

	RELEASE(piOutputPin);
	RELEASE(piInputPin);

	return E_FAIL;
}

//////////////////////////////////////////////////////////////////////
// Function		: UINT CountSystemDevices(REFCLSID clsid)
// Description	: Count the system device by category
// Comments		:
//////////////////////////////////////////////////////////////////////
UINT CHVDDSHelper::CountSystemDevices(REFCLSID clsid)
{
	ICreateDevEnum *piCreateDevEnum = NULL;
	IEnumMoniker *piEnumMoniker = NULL;
	IMoniker *piMoniker = NULL;
	HRESULT hr;
	UINT nCount = 0;

	// Enumerate all devices
	hr = ::CoCreateInstance
					(
						CLSID_SystemDeviceEnum,
						NULL,
						CLSCTX_INPROC_SERVER,
						IID_ICreateDevEnum,
						(void**) &piCreateDevEnum
					);
	if ( FAILED(hr) || !piCreateDevEnum )
	{
		DbgLog( (LOG_TRACE,3,TEXT("[CountSystemDevices] Unable to enumerate system devices.")) );
		return 0;
	}

	// Create enum moniker for video input devices
	hr = piCreateDevEnum->CreateClassEnumerator(clsid, &piEnumMoniker, 0);
	RELEASE(piCreateDevEnum);

	if ( FAILED(hr) || !piEnumMoniker )
	{
		DbgLog( (LOG_TRACE,3,TEXT("[CountSystemDevices] Unable to create enum moniker for video input devices.")) );
		return 0;
	}

	if ( SUCCEEDED(piEnumMoniker->Reset()) )
	{
		ULONG cFetched=0;
		while ( ((hr=piEnumMoniker->Next(1, &piMoniker, &cFetched)) == S_OK) 
					&& (cFetched==1) && piMoniker)
		{
			RELEASE(piMoniker);
			cFetched = 0;
			nCount++;
		}
	}

	RELEASE(piEnumMoniker);

	return nCount;
}

//////////////////////////////////////////////////////////////////////
// Function		: HRESULT RemoveAllFilters()
// Description	: Remove all filters from the graph
// Comments		:
//////////////////////////////////////////////////////////////////////
HRESULT CHVDDSHelper::RemoveAllFilters()
{
	if ( !m_piGraph )
	{
		DbgLog( (LOG_TRACE,3,TEXT("[RemoveAllFilters] Invalid parameters")) );
		return E_INVALIDARG;
	}

	HRESULT hr;
	IEnumFilters *piEnumFilters=NULL;

	hr = m_piGraph->EnumFilters(&piEnumFilters);

    if ( SUCCEEDED(hr) && piEnumFilters )
    {
        CGenericList<IBaseFilter> FilterList("List of filters to be removed");

        IBaseFilter * piFilter;
        while ( piEnumFilters->Next(1, &piFilter, 0) == NOERROR )
        {
            FilterList.AddTail( piFilter );
        }
        RELEASE(piEnumFilters);
        while ( piFilter = FilterList.RemoveHead() )
        {
            hr = RemoveFilter( &piFilter );
        }
    }
    return hr;
}

//////////////////////////////////////////////////////////////////////
// Function		: HRESULT RemoveFilter(IBaseFilter *piFilter)
// Description	: Remove filter from the graph
// Comments		:
//////////////////////////////////////////////////////////////////////
HRESULT CHVDDSHelper::RemoveFilter(IBaseFilter **ppiFilter)
{
	HRESULT hr;

	if ( (m_piGraph==NULL) || !(*ppiFilter) )
	{
		DbgLog( (LOG_TRACE,3,TEXT("[RemoveFilter] Invalid arguments.")) );
		return E_INVALIDARG;
	}

#ifdef DEBUG
	{
		FILTER_INFO filterInfo;

		hr = (*ppiFilter)->QueryFilterInfo(&filterInfo);
		RELEASE(filterInfo.pGraph);
		DbgLog( (LOG_TRACE,3,TEXT("[RemoveFilter] %ls."), filterInfo.achName) );
	}
#endif

	hr = DisconnectAllPins(*ppiFilter);
	if ( SUCCEEDED(hr) )
	{
		hr = m_piGraph->RemoveFilter(*ppiFilter);
		if ( SUCCEEDED(hr) )
		{
			RELEASE(*ppiFilter);
		}
	}

	return hr;
}


//////////////////////////////////////////////////////////////////////
// Function		: HRESULT FindEnumeratedDevice
//									(
//										REFCLSID		clsid, 
//										IBaseFilter**	ppiFilter, 
//										UINT			nDeviceID, 
//										LPSTR			pszName
//									)
// Description	: Find enumerate device by index
// Comments		:
//////////////////////////////////////////////////////////////////////
HRESULT CHVDDSHelper::FindEnumeratedDevice
							(
								REFCLSID		clsid, 
								IBaseFilter**	ppiFilter, 
								UINT			nDeviceID, 
								LPTSTR			pszName
							)
{
	if ( !ppiFilter )
	{
		DbgLog( (LOG_TRACE,3,TEXT("[FindEnumeratedDevice] Invalid arguments.")) );
		return E_INVALIDARG;
	}

	HRESULT hr;
	ICreateDevEnum *piCreateDevEnum = NULL;
	IEnumMoniker *piEnumMoniker = NULL;
	IMoniker *piMoniker = NULL;
	char szName[_MAX_PATH];
	UINT nIndex = 0;
	ULONG cFetched=0;

	*ppiFilter = NULL;

	// Enumerate all devices
	hr = ::CoCreateInstance
					(
						CLSID_SystemDeviceEnum,
						NULL,
						CLSCTX_INPROC_SERVER,
						IID_ICreateDevEnum,
						(void**) &piCreateDevEnum
					);
	if ( FAILED(hr) || !piCreateDevEnum )
	{
		DbgLog( (LOG_TRACE,3,TEXT("[FindEnumeratedDevice] Unable to enumerate system devices.")) );
		return CLASS_E_CLASSNOTAVAILABLE;
	}

	// Create enum moniker for video input devices
	hr = piCreateDevEnum->CreateClassEnumerator(clsid, &piEnumMoniker, 0);
	RELEASE(piCreateDevEnum);

	if ( FAILED(hr) || !piEnumMoniker )
	{
		DbgLog( (LOG_TRACE,3,TEXT("[FindEnumeratedDevice] Unable to create enum moniker for video input devices.")) );
		return E_FAIL;
	}

	hr = piEnumMoniker->Reset();
	if ( SUCCEEDED(hr) )
	{
		while( ((hr=piEnumMoniker->Next(1, &piMoniker, &cFetched)) == S_OK)
					&& (cFetched==1) && piMoniker )
		{
			if ( nIndex == nDeviceID )
			{
				// Output name of bound object for logging
				IPropertyBag * piPropertyBag = NULL;
				VARIANT var;

				hr = piMoniker->BindToStorage
										(
											NULL, 
											NULL, 
											IID_IPropertyBag,
											(void**) &piPropertyBag
										);
				if ( SUCCEEDED(hr) )
				{
					var.vt = VT_BSTR;
					hr = piPropertyBag->Read(L"FriendlyName", &var, NULL);
					RELEASE(piPropertyBag);

					if ( SUCCEEDED(hr) )
					{
						if(!pszName)
							pszName = (LPSTR) szName;

						WideCharToMultiByte(CP_ACP, 0, var.bstrVal, -1, pszName, _MAX_PATH, NULL, NULL);
						SysFreeString(var.bstrVal);

						DbgLog( (LOG_TRACE,3,TEXT("[FindEnumeratedDevice] Initialized device %s."), pszName) );
					}
				}

				hr = piMoniker->BindToObject
										(
											NULL, 
											NULL, 
											IID_IBaseFilter, 
											(void**) ppiFilter
										);
				RELEASE(piMoniker);
				break;
			}

			RELEASE(piMoniker);
			cFetched = 0;
			nIndex++;
		}
	}

	RELEASE(piEnumMoniker);

	if ( FAILED(hr) || !(*ppiFilter) )
	{
		DbgLog( (LOG_TRACE,3,TEXT("[FindEnumeratedDevice] Unable to initialise device.")) );
		return E_FAIL;
	}

	return NOERROR;
}

//////////////////////////////////////////////////////////////////////
// Function		: BOOL GetIKsPropertySetSupport
//									(
//										REFGUID				guidPropSet, 
//										DWORD				dwPropID, 
//										IBaseFilter**		ppiFilter, 
//										IKsPropertySet**	ppiKsPropertySet
//									)
// Description	: Get IKsPropertySet support
// Comments		:
//////////////////////////////////////////////////////////////////////
BOOL CHVDDSHelper::GetIKsPropertySetSupport
							(
								REFGUID				guidPropSet, 
								DWORD				dwPropID, 
								IBaseFilter**		ppiFilter, 
								IKsPropertySet**	ppiKsPropertySet
							)
{
	if ( !m_piGraph || !ppiFilter || !ppiKsPropertySet )
	{
		DbgLog( (LOG_TRACE,3,TEXT("[GetIKsPropertySetSupport] Invalid arguments.")) );
		return E_INVALIDARG;
	}

	(*ppiFilter) = NULL;
	(*ppiKsPropertySet) = NULL;

	IEnumFilters	*piEnumFilters = NULL;
	HRESULT hr = m_piGraph->EnumFilters(&piEnumFilters);

	if ( SUCCEEDED(hr) && piEnumFilters )
	{
		ULONG cFetchedFilter = 0;

		// enum thru all filters
		while( SUCCEEDED( hr=piEnumFilters->Next(1, ppiFilter, &cFetchedFilter ) ) 
					&& (cFetchedFilter == 1) && (*ppiFilter) )
		{
			IEnumPins *piEnumPins = NULL;

			hr = (*ppiFilter)->EnumPins(&piEnumPins);
			if ( SUCCEEDED(hr) && piEnumPins )
			{
				IPin	*piPin = NULL;
				ULONG	cFetchedPin = 0;

				// enum thru all Pins to chk for support of this IKsPropertySet
				while( SUCCEEDED( hr=piEnumPins->Next(1, &piPin, &cFetchedPin ) ) 
							&& (cFetchedPin==1) && piPin )
				{
					hr = piPin->QueryInterface
										(
											__uuidof(IKsPropertySet), 
											reinterpret_cast< void** >(ppiKsPropertySet)
										);
					if( SUCCEEDED(hr) && (*ppiKsPropertySet) )
					{
						DWORD	dwTypeSupport = 0;

						hr = (*ppiKsPropertySet)->QuerySupported(guidPropSet, dwPropID, &dwTypeSupport);
						if ( SUCCEEDED( hr ) 
								&& (dwTypeSupport & KSPROPERTY_SUPPORT_GET) 
								&& (dwTypeSupport & KSPROPERTY_SUPPORT_SET) )
						{
							FILTER_INFO filterInfo;

							if( SUCCEEDED( hr=(*ppiFilter)->QueryFilterInfo(&filterInfo) ) )
							{
								RELEASE(filterInfo.pGraph);
								DbgLog( (LOG_TRACE,3,TEXT("[GetIKsPropertySetSupport] Found Filter supporting this IKsPropertySet !!! -> %ls"), filterInfo.achName) );
							}

							RELEASE(piPin);
							break;
						}
						else
						{
							RELEASE(*ppiKsPropertySet);
						}
					}

					RELEASE(piPin);
					cFetchedPin = 0;
				}

				RELEASE(piEnumPins);

				if ( (*ppiKsPropertySet) )
				{
					break;
				}
			}

			RELEASE(*ppiFilter);
			cFetchedFilter = 0;
		}

		RELEASE(piEnumFilters);
	}

	return ( (*ppiFilter) && (*ppiKsPropertySet) ) ? TRUE : FALSE;

}

//////////////////////////////////////////////////////////////////////
// Function		: IBaseFilter* AddFilterInRegistry
//									(
//										REFCLSID		clsid, 
//										LPCSTR			lpstrFiltername, 
//										IBaseFilter**	ppiFilter
//									)
// Description	: Add filter from registry
// Comments		:
//////////////////////////////////////////////////////////////////////
IBaseFilter* CHVDDSHelper::AddFilterInRegistry
								(
									REFCLSID		clsid, 
									LPCSTR			lpstrFiltername, 
									IBaseFilter**	ppiFilter
								)
{
	HRESULT hr;
    ULONG cFetched=0;
	TCHAR szFilterFriendlyName[80];
	IMoniker *piMoniker = NULL;
	IEnumMoniker *piEnumMoniker = NULL;
	IPropertyBag *piBag = NULL;
    ICreateDevEnum *piCreateDevEnum = NULL;

	if ( !m_piGraph || !ppiFilter)
	{
		DbgLog( (LOG_TRACE,3,TEXT("[AddFilterInRegistry] Invalid arguments.")) );
		return NULL;
	}

	// we need to go thru the registry since GUID will change on different machines
    hr = CoCreateInstance
				(
					CLSID_SystemDeviceEnum, 
					NULL,
					CLSCTX_INPROC_SERVER,
					IID_ICreateDevEnum, 
					(void**)&piCreateDevEnum
				);
    if ( FAILED(hr) || !piCreateDevEnum )
	{
		DbgLog( (LOG_TRACE,3,TEXT("[AddFilterInRegistry] Unable to enumerate system devices.")) );
		return NULL;
	}

	hr = piCreateDevEnum->CreateClassEnumerator(clsid, &piEnumMoniker, 0);
    RELEASE(piCreateDevEnum);

    if ( FAILED(hr) || !piEnumMoniker )
	{
		DbgLog( (LOG_TRACE,3,TEXT("[AddFilterInRegistry] Unable to create enum moniker for video input devices.")) );
		return NULL;
	}

	// start at the first one
    hr = piEnumMoniker->Reset();
	if ( SUCCEEDED(hr) )
	{
		*ppiFilter = NULL;

		while( (*ppiFilter==NULL) && ((hr=piEnumMoniker->Next(1, &piMoniker, &cFetched)) == S_OK)
					&& (cFetched==1) && piMoniker )
		{
			memset(szFilterFriendlyName, NULL, sizeof(szFilterFriendlyName));

			hr = piMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&piBag);
			if ( SUCCEEDED(hr) && piBag )
			{
				VARIANT var;
				var.vt = VT_BSTR;
				hr = piBag->Read(L"FriendlyName", &var, NULL);
				if (hr == NOERROR)
				{	
					// get the friendly name in registry
					WideCharToMultiByte(CP_ACP, 0, var.bstrVal, -1,	szFilterFriendlyName,
										80, NULL, NULL);
					SysFreeString(var.bstrVal);

					if (!strcmp(szFilterFriendlyName, lpstrFiltername))
					{
			   			piMoniker->BindToObject(0, 0, IID_IBaseFilter, (void **)ppiFilter);

						if ( *ppiFilter != NULL )
						{
							WCHAR wName[80];
							MultiByteToWideChar(CP_ACP, 0, lpstrFiltername, -1, wName, MAX_PATH);
							hr = m_piGraph->AddFilter(*ppiFilter, wName);
							if (hr != NOERROR)
							{
								DbgLog( (LOG_TRACE,3,TEXT("[AddFilterInRegistry] Unable to add filter: %s."),lpstrFiltername) );
								//return NULL;
							}
						}
					}
				}
				RELEASE(piBag);
			}
			RELEASE(piMoniker);
			cFetched = 0;
		}
	}
    RELEASE(piEnumMoniker);

	return *ppiFilter;
}

//////////////////////////////////////////////////////////////////////
// Function		: HRESULT CountFilterPins
//								(
//									IBaseFilter *piFilter, 
//									UINT *pnInPins, 
//									UINT *pnOutPins
//								)
// Description	: Count filter's input and output pins
// Comments		:
//////////////////////////////////////////////////////////////////////
HRESULT CHVDDSHelper::CountFilterPins
							(
								IBaseFilter *piFilter, 
								UINT *pnInPins, 
								UINT *pnOutPins
							)
{
	if ( !piFilter )
	{
		DbgLog( (LOG_TRACE,3,TEXT("[CountFilterPins] Invalid arguments.")) );
		return E_INVALIDARG;
	}

	IEnumPins *piEnumPins=NULL;
	HRESULT hr;

	hr = piFilter->EnumPins(&piEnumPins);
	if ( SUCCEEDED(hr) && piEnumPins )
	{
		IPin *piPin=NULL;
		ULONG cFetched=0;
		PIN_DIRECTION pinDir;

		(*pnInPins) = 0;
		(*pnOutPins) = 0;

		while ( SUCCEEDED( piEnumPins->Next(1, &piPin, &cFetched) )
					&& (cFetched==1) && piPin )
		{
			hr = piPin->QueryDirection(&pinDir);
			if ( SUCCEEDED(hr) )
			{
				if ( pinDir == PINDIR_INPUT )
					(*pnInPins)++;
				else
					(*pnOutPins)++;
			}
			RELEASE(piPin);
			cFetched = 0;
		}

	}

	return hr;
}


//////////////////////////////////////////////////////////////////////
// Function		: void RemoveDownStream(IBaseFilter *piFilter)
// Description	: Remove down stream of the filter
// Comments		:
//////////////////////////////////////////////////////////////////////
void CHVDDSHelper::RemoveDownStream(IBaseFilter *piFilter)
{
    IPin *piPin = NULL;
	IPin *piPinTo = NULL;
	ULONG cFetched = 0;
    IEnumPins *piEnumPins = NULL;
    PIN_INFO pinInfo;
    HRESULT hr;
	BOOL bRemoveFilter=FALSE;

	if ( !m_piGraph || !piFilter )
	{
		DbgLog( (LOG_TRACE,3,TEXT("[RemoveDownStream] Invalid arguments.")) );
		return;
	}
	
#ifdef DEBUG
	{
		FILTER_INFO filterInfo;

		hr = piFilter->QueryFilterInfo(&filterInfo);
		RELEASE(filterInfo.pGraph);
		DbgLog( (LOG_TRACE,3,TEXT("[RemoveDownStream] of %ls."), filterInfo.achName) );
	}
#endif

	hr = piFilter->EnumPins(&piEnumPins);

	if ( SUCCEEDED(hr) && piEnumPins )
	{
		hr = piEnumPins->Reset();

		if ( SUCCEEDED(hr) )
		{
			while ( SUCCEEDED(hr=piEnumPins->Next(1, &piPin, &cFetched))
						&& (cFetched==1) && piPin )
			{
				hr = piPin->ConnectedTo(&piPinTo);
				if ( SUCCEEDED(hr) && piPinTo ) 
				{
					hr = piPinTo->QueryPinInfo(&pinInfo);
					if ( hr == NOERROR )
					{
						if (pinInfo.dir == PINDIR_INPUT) 
						{
							RemoveDownStream(pinInfo.pFilter);
							//m_piGraph->Disconnect(piPinTo);
							//m_piGraph->Disconnect(piPin);
							//m_piGraph->RemoveFilter(pinInfo.pFilter);
							RemoveFilter(&(pinInfo.pFilter));
							bRemoveFilter = TRUE;
						}
						RELEASE(pinInfo.pFilter);
					}
					RELEASE(piPinTo);
				}
				RELEASE(piPin);

				if ( bRemoveFilter )
				{
					hr = piEnumPins->Reset();
					bRemoveFilter = FALSE;
				}
			}
		}
		RELEASE(piEnumPins);
	}
}

//////////////////////////////////////////////////////////////////////
// Function		: HRESULT RemoveDownStream(IPin *piPin)
//				  HRESULT RemoveDownStream(IBaseFilter *piFilter, GUID guidPin)
//
// Description	: Remove down stream of output pin or pin category
// Comments		:
//////////////////////////////////////////////////////////////////////
HRESULT CHVDDSHelper::RemoveDownStream(IPin *piPin)
	{
	IPin *piPinTo=NULL;
    PIN_INFO pinInfo;
	HRESULT hr;

	hr = piPin->ConnectedTo(&piPinTo);
	if (FAILED(hr))
		{
		// not connected
		return S_OK;
		}
	
	if (SUCCEEDED(hr) && piPinTo ) 
		{
		hr = piPinTo->QueryPinInfo(&pinInfo);
		if (SUCCEEDED(hr))
			{
			if (pinInfo.dir == PINDIR_INPUT) 
				{
				RemoveDownStream(pinInfo.pFilter);
				RemoveFilter(&(pinInfo.pFilter));
				}
			else
				{
				hr = E_FAIL;
				}
			RELEASE(pinInfo.pFilter);
			}
		RELEASE(piPinTo);
		}
	return hr;
	}

HRESULT CHVDDSHelper::RemoveDownStream(IBaseFilter *piFilter, GUID guidPin)
	{
	IPin *piPin=NULL;
	HRESULT hr;
	hr = FindPin(piFilter, &piPin, guidPin, FALSE);
	if (SUCCEEDED(hr))
		{
		hr = RemoveDownStream(piPin);
		RELEASE(piPin);
		}
	return hr;
	}

//////////////////////////////////////////////////////////////////////
// Function		: HRESULT FindConnectedToFilter
//								(
//									IPin*			piPin, 
//									FILTER_INFO*	pFltInfo,
//									PIN_INFO*		pPinInfo 
//								)
// Description	: Find the filter that connect from the pin
// Comments		:
//////////////////////////////////////////////////////////////////////
HRESULT CHVDDSHelper::FindConnectedToFilter
							(
								IPin*			piPin, 
								FILTER_INFO*	pFltInfo,
								PIN_INFO*		pPinInfo 
							)
{
	if ( !piPin )
	{
		DbgLog( (LOG_TRACE,3,TEXT("[FindConnectedToFilter] Invalid arguments.")) );
		return E_INVALIDARG;
	}

	IPin *piConnectedTo=NULL;
	HRESULT hr;

	hr = piPin->ConnectedTo(&piConnectedTo);

	if ( SUCCEEDED(hr) && piConnectedTo )
	{
		hr = piConnectedTo->QueryPinInfo(pPinInfo);

		if ( SUCCEEDED(hr) && pPinInfo->pFilter )
		{
			hr = pPinInfo->pFilter->QueryFilterInfo(pFltInfo);
		}

		RELEASE(piConnectedTo);
	}

	return hr;
}