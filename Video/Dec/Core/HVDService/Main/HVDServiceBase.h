#ifndef _HARDWARE_DECODE_SERVICE_BASE_H_
#define _HARDWARE_DECODE_SERVICE_BASE_H_

#include <vector>
#include "Exports\Inc\HVDService.h"

#define DP_FASTREADBACK		//DP	// debug information about the function of FastReadback

//#define DEBUG_TICKS_INFO

#if defined DEBUG_TICKS_INFO
#define INIT_TICKS	DOUBLE dwWritetotaltime =0;\
	DOUBLE dwWritetimeMax =0;\
	DOUBLE dwWritetimeMin =100;\
	LARGE_INTEGER sFrequency;\
	LARGE_INTEGER sStart;\
	LARGE_INTEGER sEnd;
#define START_TICKS	QueryPerformanceFrequency(&sFrequency);\
	QueryPerformanceCounter(&sStart);
#define STOP_TICKS	QueryPerformanceCounter(&sEnd);\
	double t = (1000.0 * (LONG)(sEnd.QuadPart - sStart.QuadPart))/ sFrequency.LowPart;\
	dwWritetotaltime += t;\
	if (t>dwWritetimeMax)\
{\
	dwWritetimeMax = t;\
}\
					else if (t<dwWritetimeMin)\
{\
	dwWritetimeMin = t;\
}
#define PRINT_TICKS printf("WritetimeMax:%6.3f WritetimeMin:%6.3f WritetimeTotal:%6.3f\n",dwWritetimeMax,dwWritetimeMin,dwWritetotaltime);\
	DP("WritetimeMax:%6.3f WritetimeMin:%6.3f WritetimeTotal:%6.3f\n",dwWritetimeMax,dwWritetimeMin,dwWritetotaltime);
#else
#define INIT_TICKS
#define START_TICKS
#define STOP_TICKS
#define PRINT_TICKS
#endif

#pragma warning(disable:4996)

#define MAX_VAR 1024
static void DP(char *pcFormat, ...)
{
	char t_cBuffer[MAX_VAR]; 
	char* t_pcVal=NULL;
	int t_nWritten;

	va_start(t_pcVal, pcFormat);
	t_nWritten = _vsnprintf(t_cBuffer, MAX_VAR-1, pcFormat, t_pcVal);
	t_nWritten = t_nWritten>=0 ? t_nWritten : MAX_VAR-2;
	t_cBuffer[t_nWritten]='\n';
	t_cBuffer[t_nWritten+1]='\0';

	OutputDebugString(t_cBuffer);
}

namespace HVDService
{
	class CHVDServiceBase : public IHVDService
	{
	public:
		CHVDServiceBase();
		virtual ~CHVDServiceBase();

	public:
		// IUnknown
		STDMETHOD(QueryInterface)(REFIID riid, void **ppv);
		STDMETHOD_(ULONG, AddRef)();
		STDMETHOD_(ULONG, Release)();

	public:
		// IHVDservice
		STDMETHOD(Initialize)(HVDInitConfig* pInitConfig);
		STDMETHOD(Uninitialize)();
		STDMETHOD(StartService)(HVDDecodeConfig* pDecodeConfig);
		STDMETHOD(GetDecoderGuid)(GUID* pGuid);
		STDMETHOD(GetSupportedDecoderCount)(UINT* pnCount);
		STDMETHOD(GetSupportedDecoderGUIDs)(GUID* pGuids);
		STDMETHOD(AdviseEventNotify)(IHVDServiceNotify* pNotify);
		STDMETHOD(UnadviseEventNotify)(IHVDServiceNotify* pNotify);
		STDMETHOD(GetHVDDecodeConfig)(HVDDecodeConfig* pDecoderConfig);

	protected:
		virtual HRESULT _Initialize(HVDInitConfig* pInitConfig);
		virtual HRESULT _Uninitialize();
		virtual HRESULT _StartService();
		virtual HRESULT _StopService();

		virtual HRESULT _NotifyEvent(DWORD dwEvent, DWORD dwParam1, DWORD dwParam2);

	typedef std::vector<IHVDServiceNotify *> HVDServiceEventNotify;

	protected:
		CCritSec m_csObj;
		LONG m_cRef;

		DWORD m_dwVendorID;
		DWORD m_dwDeviceID;
		DWORD m_dwService;
		DWORD m_dwInitFlags;
		DWORD m_dwHVDMode;
		DWORD m_dwSurfaceCount;
		GUID m_DecoderGuid;
		DWORD m_dwSupportedDecoderCount;
		GUID* m_pSupportedDecoderGuids;
		HVDDecodeConfig m_HVDDecodeConfig;
		HVDInitFastCopyConfig m_FastCopyConfig;

		HVDServiceEventNotify m_EventNotify;
	};
}

#endif