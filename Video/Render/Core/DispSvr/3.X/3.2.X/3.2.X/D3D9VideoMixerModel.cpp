#include "stdafx.h"
#include "D3D9VideoMixerModel.h"

using namespace DispSvr;



CD3D9VideoMixerModel::CD3D9VideoMixerModel()
{
    m_GUID = DISPSVR_RESOURCE_VIDEOMIXERMODEL;
	ZeroMemory(m_Planes, sizeof(m_Planes));
	m_colorBackGround = 0;
	ZeroMemory(&m_LumaKey, sizeof(m_LumaKey));
	LoadDefaultMixerCoordinate(m_nrcDst);
}

CD3D9VideoMixerModel::~CD3D9VideoMixerModel()
{
}

STDMETHODIMP CD3D9VideoMixerModel::QueryInterface(REFIID riid, void **ppv)
{
	HRESULT hr = E_NOINTERFACE;

	if (riid == __uuidof(IDispSvrVideoMixerModel))
	{
		hr = GetInterface((IDispSvrVideoMixerModel *)this, ppv);
	}
	else
	{
		hr = CD3D9PluginBase::QueryInterface(riid, ppv);
	}
	return hr;
}

STDMETHODIMP CD3D9VideoMixerModel::_SetDevice(IUnknown *pDevice)
{
    return CD3D9PluginBase::_SetDevice(pDevice);
}

STDMETHODIMP CD3D9VideoMixerModel::_ReleaseDevice()
{
	m_ClearRectList.clear();

    return CD3D9PluginBase::_ReleaseDevice();
}

void CD3D9VideoMixerModel::SetDestination(const NORMALIZEDRECT &DestRect)
{
    m_nrcDst = DestRect;
}

STDMETHODIMP CD3D9VideoMixerModel::SetClearRectangles(UINT uCount, ClearRect *pRects)
{
	m_ClearRectList.clear();
	if (uCount > 0)
	{
        CHECK_POINTER(pRects);
		const ClearRect *p = pRects;
		for (UINT i = 0; i < uCount; i++)
			m_ClearRectList.push_back(*p++);
	}
    return S_OK;
}

STDMETHODIMP CD3D9VideoMixerModel::SetBackgroundColor(COLORREF Color)
{
	m_colorBackGround = Color;
    ConvertArgbToAyuv16(Color, m_ayuv16Background);
    return S_OK;
}

STDMETHODIMP CD3D9VideoMixerModel::GetBackgroundColor(COLORREF *pColor)
{
	CHECK_POINTER(pColor);
	*pColor = m_colorBackGround;
	return S_OK;
}

STDMETHODIMP CD3D9VideoMixerModel::SetLumaKey(const LumaKey *pLumaKey)
{
	CHECK_POINTER(pLumaKey);
	m_LumaKey = *pLumaKey;
	return S_OK;
}

STDMETHODIMP CD3D9VideoMixerModel::GetLumaKey(LumaKey *pLumaKey)
{
	CHECK_POINTER(pLumaKey);
	*pLumaKey = m_LumaKey;
	return S_OK;
}