#include "stdafx.h"
#include <windows.h>
#include <assert.h>
#include <streams.h>
#include "VideoProcessorBase.h"

using namespace VideoProcessor;

CVideoProcessorBase::CVideoProcessorBase() : m_cRef(0)
{
}

CVideoProcessorBase::~CVideoProcessorBase()
{
}

STDMETHODIMP CVideoProcessorBase::QueryInterface(REFIID riid, void **ppv)
{
	HRESULT hr = E_NOINTERFACE;

	if (riid == __uuidof(IVideoProcessor))
	{
		hr = GetInterface((IVideoProcessor *)this, ppv);
	}
	else if (riid == _uuidof(IUnknown))
	{
		hr = GetInterface((IVideoProcessor *)this, ppv);
	}
	return hr;
}

STDMETHODIMP_(ULONG) CVideoProcessorBase::AddRef()
{
	LONG lRef = InterlockedIncrement(&m_cRef);
	assert(lRef > 0);
	return lRef;
}

STDMETHODIMP_(ULONG) CVideoProcessorBase::Release()
{
	LONG lRef = InterlockedDecrement(&m_cRef);
	assert(lRef >= 0);
	if (lRef == 0)
		delete this;
	return lRef;
}

STDMETHODIMP CVideoProcessorBase::QueryVideoProcessorCaps(VideoProcessorCaps **ppCaps)
{
	return _QueryVideoProcessorCaps(ppCaps);
}

STDMETHODIMP CVideoProcessorBase::GetFilterPropertyRange(VIDEO_FILTER VideoFilter, FilterValueRange* pFilterRange)
{
	return _GetFilterPropertyRange(VideoFilter, pFilterRange);
}

STDMETHODIMP CVideoProcessorBase::GetProcAmpRange(PROCAMP_CONTROL ProcAmpControl, ProcAmpValueRange* pProcAmpRange)
{
	return _GetProcAmpRange(ProcAmpControl, pProcAmpRange);
}

STDMETHODIMP CVideoProcessorBase::SetVideoProcessorMode(const VideoProcessorModes *pVPMode)
{
	return _SetVideoProcessorMode(pVPMode);
}

STDMETHODIMP CVideoProcessorBase::GetVideoProcessorMode(VideoProcessorModes *pVPMode)
{
	return _GetVideoProcessorMode(pVPMode);
}

STDMETHODIMP CVideoProcessorBase::GetNumReferentSamples(DWORD	*pNumBackwardRefSamples,DWORD	*pNumForwardRefSamples)
{
	return _GetNumReferentSamples(pNumBackwardRefSamples, pNumForwardRefSamples);
}

STDMETHODIMP CVideoProcessorBase::VideoProcessBlt( VideoBuffer *pRenderTarget, 
											    const VideoProcessBltParams *pBltParams,
												const VideoSample *pVideoSamples,
												UINT uNumSamples)
{
	return _VideoProcessBlt(pRenderTarget, pBltParams, pVideoSamples, uNumSamples);
}


HRESULT CVideoProcessorBase::_QueryVideoProcessorCaps(VideoProcessorCaps **pCaps)
{
	return E_NOTIMPL;
}

HRESULT CVideoProcessorBase::_GetFilterPropertyRange(VIDEO_FILTER VideoFilter, FilterValueRange* pFilterRange)
{
	return E_NOTIMPL;
}

HRESULT CVideoProcessorBase::_GetProcAmpRange(PROCAMP_CONTROL ProcAmpControl, ProcAmpValueRange* pProcAmpRange)
{
	return E_NOTIMPL;
}

HRESULT CVideoProcessorBase::_SetVideoProcessorMode(const VideoProcessorModes *pVPMode)
{
	return E_NOTIMPL;
}

HRESULT CVideoProcessorBase::_GetVideoProcessorMode(VideoProcessorModes *pVPMode)
{
	return E_NOTIMPL;
}

HRESULT CVideoProcessorBase::_GetNumReferentSamples(DWORD	*pNumBackwardRefSamples,DWORD	*pNumForwardRefSamples)
{
	*pNumBackwardRefSamples = m_dwNumBackwardRefSamples;
	*pNumForwardRefSamples  = m_dwNumForwardRefSamples;
	return S_OK;
}

HRESULT CVideoProcessorBase::_VideoProcessBlt(	VideoBuffer *pRenderTarget, 
												const VideoProcessBltParams *pBltParams,
												const VideoSample *pVideoSamples,
												UINT uNumSamples)
{
	return E_NOTIMPL;
}

CPU_LEVEL CVideoProcessorBase::GetCPULevel()
{	
#if defined(_WIN32) || defined(__INTEL_COMPILER)
#define GetCPUIDRegs(IDReg, SRegs) {\
	_asm xor edx, edx\
	_asm xor ecx, ecx\
	_asm xor ebx, ebx\
	_asm mov eax, IDReg\
	_asm _emit 0x0f\
	_asm _emit 0xa2\
	_asm mov SRegs.uiEAX, eax\
	_asm mov SRegs.uiEBX, ebx\
	_asm mov SRegs.uiECX, ecx\
	_asm mov SRegs.uiEDX, edx\
}
#define GetCPUIDCapability(bResult) {\
	_asm pushfd\
	_asm pop eax\
	_asm mov ebx, eax\
	_asm xor eax, (1<<21)\
	_asm push eax\
	_asm popfd\
	_asm pushfd\
	_asm pop eax\
	_asm xor eax, ebx\
	_asm shr eax, 21\
	_asm and eax, 1\
	_asm mov bResult, eax\
}
#elif defined(__GNUC__) && defined(__i386__)
	// assuming gcc compiler for all Intel x86 CPUs (ie. Linux x86)
#define GetCPUIDRegs(IDReg, SRegs) {          \
	__asm__ (                                 \
	"xorl %%edx, %%edx\n\t"                   \
	"xorl %%ecx, %%ecx\n\t"                   \
	"xorl %%ebx, %%ebx\n\t"                   \
	".byte 0x0f\n\t"                          \
	".byte 0xa2\n\t"                          \
	: "=a" (SRegs.uiEAX), "=b" (SRegs.uiEBX), \
	"=c" (SRegs.uiECX), "=d" (SRegs.uiEDX)  \
	: "a" (IDReg) );                          \
}

#define GetCPUIDCapability(bResult) { \
	__asm__ (                         \
	"pushf\n\t"                       \
	"popl %%eax\n\t"                  \
	"movl %%eax, %%ebx\n\t"           \
	"xorl %1, %%eax\n\t"              \
	"pushl %%eax\n\t"                 \
	"popf\n\t"                        \
	"pushf\n\t"                       \
	"popl %%eax\n\t"                  \
	"xorl %%ebx, %%eax\n\t"           \
	"shrl $21, %%eax\n\t"             \
	"andl $1, %%eax\n\t"              \
	: "=a" (bResult)                  \
	: "i" (1<<21)                     \
	);                                \
}
#else
#define GetCPUIDCapability(bCPUIDCapable)
#define GetCPUIDRegs(a,b)
#warning GetCPUIDRegs/GetCPUIDCapability not defined for your architecture!
#endif
	enum CPUIDfeature
	{
		/* for feature register ecx */
		CPUID_PRESCOTT_FEATURE_SSE3						= (1<<0),
		/* for feature register edx */
		CPUID_STANDARD_FEATURE_MMX						= (1<<23),
		CPUID_STANDARD_FEATURE_SSE						= (1<<25),
		CPUID_STANDARD_FEATURE_SSE2						= (1<<26),
		CPUID_STANDARD_FEATURE_HT						= (1<<28),
		/* for extended feature register edx */
		CPUID_EXTENDED_FEATURE_AMD_MMX_EXT		= (1<<22),
		CPUID_EXTENDED_FEATURE_3DNOW_EXT		= (1<<30),
		CPUID_EXTENDED_FEATURE_3DNOW			= (1<<31)
	};
	union _uStateTag
	{
		struct 
		{
			unsigned int uiEBX;
			unsigned int uiEDX;
			unsigned int uiECX;
			unsigned int uiEAX;
		} Regs;
		char	 szIdString[16];
	} uState;
	unsigned long bCPUIDCapable = 0;
	unsigned int m_uiCPUIDMaxStandardLevel;
	unsigned int m_uiCPUIDPrescottFeature;
	unsigned int m_uiCPUIDStandardFeature;

	GetCPUIDCapability(bCPUIDCapable);
	if(!bCPUIDCapable)
		return CPU_LEVEL_NONE;
	{
		GetCPUIDRegs(0,uState.Regs);
		m_uiCPUIDMaxStandardLevel = uState.Regs.uiEAX;
		if(m_uiCPUIDMaxStandardLevel>=1)
		{
			GetCPUIDRegs(1,uState.Regs);
			m_uiCPUIDPrescottFeature = uState.Regs.uiECX;
			m_uiCPUIDStandardFeature = uState.Regs.uiEDX;
			if(m_uiCPUIDPrescottFeature&CPUID_PRESCOTT_FEATURE_SSE3)
				return CPU_LEVEL_SSE3;
			if(m_uiCPUIDStandardFeature&CPUID_STANDARD_FEATURE_SSE2)
				return CPU_LEVEL_SSE2;
			if(m_uiCPUIDStandardFeature&CPUID_STANDARD_FEATURE_SSE)
				return CPU_LEVEL_SSE;
			if(m_uiCPUIDStandardFeature&CPUID_STANDARD_FEATURE_MMX)
				return CPU_LEVEL_MMX;
		}
	}
	return CPU_LEVEL_NONE;
}