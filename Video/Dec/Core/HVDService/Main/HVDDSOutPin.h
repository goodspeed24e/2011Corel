#pragma once

namespace HVDService
{
	class CHVDDxva1Binder;

	class CHVDDSOutPin : public CBaseOutputPin
	{
	public:
		DECLARE_IUNKNOWN
		CHVDDSOutPin(TCHAR *pName, CHVDDxva1Binder *pFilter, const AMOVIESETUP_PIN *pPinInfo, LPWSTR pPinName);
		~CHVDDSOutPin();
		//INonDelegatingUnknown method
		STDMETHOD(NonDelegatingQueryInterface)(REFIID riid, void **ppv);

		//CBasePin method
		STDMETHOD(Notify)(IBaseFilter * pSender, Quality q);
		HRESULT Inactive();
		HRESULT CheckMediaType(const CMediaType *pmt);
		HRESULT GetMediaType(int iPosition, CMediaType *pMediaType);
		//CBaseOutputPin method
		HRESULT DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pProperties);
		//Other INonDelegatingUnknown releated method
		HRESULT AddObject(INonDelegatingUnknown *obj);
		HRESULT RemoveAllObject();
	public:
		HRESULT CurrentMediaType(AM_MEDIA_TYPE *pmt);

	protected:
		HRESULT AddMediaType(CMediaType const *pmt);

	protected:
		CGenericList<CMediaType>	m_MediaTypeList;
		CGenericList<INonDelegatingUnknown>	m_NonDelegatingInterfaceList;		// list of aggregated objects
		DWORD						m_dwMediaInterleave;
		CHVDDxva1Binder				*m_pDXVA1BinderFilter;

		Quality						m_Quality;
	};
}
