#include "stdafx.h"
#include <windows.h>
#include <streams.h>
#include <dvdmedia.h>
#include <d3d9.h>
#include "HVDDSOutPin.h"
#include "HVDDxva1Binder.h"

using namespace HVDService;

CHVDDSOutPin::CHVDDSOutPin(TCHAR *pName, CHVDDxva1Binder *pFilter, const AMOVIESETUP_PIN *pPinInfo, LPWSTR pPinName):
CBaseOutputPin(pName,(CBaseFilter *)pFilter,
			   &pFilter->m_CritSec,&pFilter->m_hr,
			   pPinName?pPinName:((pPinInfo && pPinInfo->strName)?pPinInfo->strName:0)),
			   m_MediaTypeList(NAME("CHVDDSOutPin::m_MediaTypeList")),
			   m_NonDelegatingInterfaceList(NAME("CHVDDSOutPin::m_NonDelegatingInterfaceList"))
{
	m_dwMediaInterleave = 1;
	m_pDXVA1BinderFilter = pFilter;
	if(pPinInfo)
	{	// add mediatypes
		CMediaType mt;
		for(unsigned int i=0; i<pPinInfo->nMediaTypes; i++)
		{
			mt.SetType(pPinInfo->lpMediaType[i].clsMajorType);
			mt.SetSubtype(pPinInfo->lpMediaType[i].clsMinorType);
			AddMediaType(&mt); 
		}
	}
	ZeroMemory(&m_Quality,sizeof(m_Quality));
}

CHVDDSOutPin::~CHVDDSOutPin()
{
	while(m_MediaTypeList.GetCount() != 0) 
		delete m_MediaTypeList.RemoveHead();
}

STDMETHODIMP CHVDDSOutPin::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
	HRESULT hr;
	POSITION index;
	INonDelegatingUnknown *pUnk;

	CheckPointer(ppv,E_POINTER);
	ValidateReadWritePtr(ppv,sizeof(PVOID));
	*ppv = 0;
	if(riid==IID_IUnknown)	// if IID_IUnknown, must handle here (bypass any aggregate objects)
		return CBaseOutputPin::NonDelegatingQueryInterface(riid, ppv);
	for(index=m_NonDelegatingInterfaceList.GetHeadPosition(); index;) 
	{	// check aggregated object list
		pUnk = m_NonDelegatingInterfaceList.GetNext(index);
		hr = pUnk->NonDelegatingQueryInterface(riid,ppv);
		if(SUCCEEDED(hr))
			return hr;
	}
	return CBaseOutputPin::NonDelegatingQueryInterface(riid, ppv);
}

STDMETHODIMP CHVDDSOutPin::Notify(IBaseFilter *pSender, Quality q)
{
	m_Quality = q;
	return S_OK;
}

HRESULT CHVDDSOutPin::Inactive()
{
	CAutoLock lock_it(m_pLock);
	ZeroMemory(&m_Quality,sizeof(m_Quality));
	CBaseOutputPin::Inactive();
	return S_OK;
} 

HRESULT CHVDDSOutPin::CheckMediaType(const CMediaType *pmt)
{
	POSITION index;
	CMediaType *pmtList;

	index = m_MediaTypeList.GetHeadPosition();
	while(index) 
	{
		pmtList = m_MediaTypeList.GetNext(index);
		if(*pmtList->Type()==*pmt->Type() && 
			(*pmtList->Subtype()==*pmt->Subtype() || *pmtList->Subtype()==MEDIASUBTYPE_NULL))
		{
			//if(GetPincom()->CheckMediaType(this,pmt)==S_OK)
			return S_OK;
		}
	}
	return S_FALSE;
}

HRESULT CHVDDSOutPin::GetMediaType(int iPosition, CMediaType *pMediaType)
{
	POSITION index;
	CMediaType *pmtList;
	DWORD icount = m_dwMediaInterleave;
	int lpos = iPosition;

	if(iPosition<0)
		return E_INVALIDARG;

	index = m_MediaTypeList.GetHeadPosition();
	while(index)
	{
		if (icount==m_dwMediaInterleave)
		{
			pmtList = m_MediaTypeList.GetNext(index);
			icount = 0;
		}
		if (m_pDXVA1BinderFilter)
		{
			if(m_pDXVA1BinderFilter->GetMediaType(this, iPosition, pmtList)==S_OK)
			{
				if(lpos--==0)
				{
					*pMediaType = *pmtList;
					return S_OK;
				}
			}
		}
		icount++;
	}
	return VFW_S_NO_MORE_ITEMS;
}

HRESULT CHVDDSOutPin::DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pProperties)
{
	ALLOCATOR_PROPERTIES Actual;
	HRESULT hr;

	if(pAlloc==0 || pProperties==0)
		return E_INVALIDARG;
	if (m_pDXVA1BinderFilter)
		m_pDXVA1BinderFilter->DecideBufferSize(this,pAlloc,pProperties);
	hr = pAlloc->SetProperties(pProperties,&Actual);
	if(FAILED(hr))
		return hr;
	if(Actual.cbBuffer<pProperties->cbBuffer || Actual.cBuffers<pProperties->cBuffers)
		return E_INVALIDARG;
	return S_OK;
}

HRESULT CHVDDSOutPin::AddObject(INonDelegatingUnknown *obj)
{
	if(obj==0)
		return E_INVALIDARG;
	if(m_NonDelegatingInterfaceList.AddTail(obj)) 
	{
		obj->NonDelegatingAddRef();
		return S_OK;
	}
	return E_OUTOFMEMORY;
}

HRESULT CHVDDSOutPin::RemoveAllObject()
{
	if (m_NonDelegatingInterfaceList.GetCount() == 0)
		return S_FALSE;

	POSITION pos;
	INonDelegatingUnknown *pIUnknown;
	ULONG ulref;
	pos = m_NonDelegatingInterfaceList.GetHeadPosition();
	while(pos)
	{
		pIUnknown = m_NonDelegatingInterfaceList.GetNext(pos);
		ulref = pIUnknown->NonDelegatingRelease();
	}
	m_NonDelegatingInterfaceList.RemoveAll();

	return S_OK;
}

HRESULT CHVDDSOutPin::AddMediaType(CMediaType const *pmt)
{
	CMediaType *pmtNew = new CMediaType(*pmt);
	if (pmtNew) 
	{
		if (m_MediaTypeList.AddTail(pmtNew)) 
		{
			return S_OK;
		}
	}
	delete pmtNew;
	return E_OUTOFMEMORY;
} 

