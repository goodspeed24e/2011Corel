#include "stdafx.h"
#include "ServerStateEventSink.h"

using namespace DispSvr;

CServerStateEventSink::CServerStateEventSink()
{
	ZeroMemory(m_szText, sizeof(m_szText));
}

CServerStateEventSink::~CServerStateEventSink()
{

}

STDMETHODIMP CServerStateEventSink::QueryInterface(REFIID riid, void **ppv)
{
	if (riid == __uuidof(IDisplayServerStateEventSink))
	{
		this->AddRef();
		*ppv = this;
		return S_OK;
	}
	*ppv = NULL;
	return E_NOINTERFACE;
}

HRESULT CServerStateEventSink::Notify(DWORD eEvent, DWORD dwParam1, DWORD dwParam2)
{
	CAutoLock lock(&m_cs);

	switch (eEvent)
	{
	case SERVER_STATE_RESET:
		m_PresentRate.Reset();
		for (VidSrcRateMap::iterator it = m_mapVideoSourceRate.begin();
			it != m_mapVideoSourceRate.end(); ++it)
		{
			it->second.InputRate.Reset();
			it->second.OutputRate.Reset();
			it->second.DropRate.Reset();
		}
		break;
	case SERVER_STATE_PRESENT:
		m_PresentRate.Update();
		break;
	case SERVER_STATE_RENDER:
		break;
	case SERVER_STATE_VIDEO_SOURCE_ADD:
		m_mapVideoSourceRate[dwParam1];
		break;
	case SERVER_STATE_VIDEO_SOURCE_REMOVE:
		{
			VidSrcRateMap::iterator it = m_mapVideoSourceRate.find(dwParam1);
			if (it != m_mapVideoSourceRate.end())
				m_mapVideoSourceRate.erase(it);
		}
		break;
	case SERVER_STATE_VIDEO_SOURCE_PROCESS_INPUT:
		ASSERT(m_mapVideoSourceRate.find(dwParam1) != m_mapVideoSourceRate.end());
		m_mapVideoSourceRate[dwParam1].InputRate.Update();
		break;
	case SERVER_STATE_VIDEO_SOURCE_PROCESS_OUTPUT:
		ASSERT(m_mapVideoSourceRate.find(dwParam1) != m_mapVideoSourceRate.end());
		m_mapVideoSourceRate[dwParam1].OutputRate.Update();
		break;
	case SERVER_STATE_VIDEO_SOURCE_DROP_INPUT:
		ASSERT(m_mapVideoSourceRate.find(dwParam1) != m_mapVideoSourceRate.end());
		m_mapVideoSourceRate[dwParam1].DropRate.Update();
		break;
	default:
		return E_NOTIMPL;
	};

	return S_OK;
}


HRESULT CServerStateEventSink::GetStateText(LPTSTR pStr, UINT uLen)
{
	CAutoLock lock(&m_cs);

	int n = _stprintf_s(m_szText, sizeof(m_szText) / sizeof(m_szText[0]), _T("DispSvr FPS: %.2f Avg: %.2f\n"),
		m_PresentRate.GetInstantRate(),
		m_PresentRate.GetAverageRate());

	ASSERT(uLen < sizeof(m_szText));
	for (VidSrcRateMap::iterator it = m_mapVideoSourceRate.begin(); it != m_mapVideoSourceRate.end(); ++it)
	{
		n += _stprintf_s(m_szText + n, (sizeof(m_szText) / sizeof(m_szText[0])) - n, _T("- VID(0x%x) Input: %.2f, Output: %.2f, Drop: %.2f\n"),
			it->first,
			it->second.InputRate.GetAverageRate(),
			it->second.OutputRate.GetAverageRate(),
			it->second.DropRate.GetAverageRate()
			);
		if (n >= static_cast<int> (uLen))
		{
			m_szText[uLen] = '\0';
			break;
		}
	}
	lstrcpyn(pStr, m_szText, uLen);
	return S_OK;
}

const UINT CServerStateEventSink::CRateCalculator::SAMPLING_COUNT = 33;

void CServerStateEventSink::CRateCalculator::Update()
{
	DWORD dwCurTime = timeGetTime();
	DWORD dwDiff = dwCurTime - dwLastRender;

	uFramesDrawn++;
	if (uFramesDrawn % 5 == 1)
		fInterframeInstant = (fInterframeInstant + dwDiff) / 2;

	dwAccumulatedDiff += dwDiff;
	if (uFramesDrawn % SAMPLING_COUNT == 0)
	{
		fInterframeAvg = dwAccumulatedDiff / SAMPLING_COUNT;
		dwAccumulatedDiff = 0;
		uFramesDrawn = 0;
	}
	dwLastRender = dwCurTime;
}

void CServerStateEventSink::CRateCalculator::Reset()
{
	uFramesDrawn = 0;
	dwAccumulatedDiff = 0;
	dwLastRender = 0;
	fInterframeInstant = fInterframeAvg = 0;
}