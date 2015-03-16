#ifndef _DSVPCONFIG_H
#define _DSVPCONFIG_H

#include <dvp.h>
#include <vptype.h>
#include <vpconfig.h>

namespace HVDService
{
	class CHVDDSVPConfig : public IVPConfig, public CUnknown
	{
	public:
		CHVDDSVPConfig(CCritSec *pLock, LPUNKNOWN pUnk);
		DECLARE_IUNKNOWN
		// INonDelegatingUnknown
		STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);
		// IVPConfig
		STDMETHODIMP GetConnectInfo(LPDWORD pdwNumConnectInfo,LPDDVIDEOPORTCONNECT pddVPConnectInfo );
		STDMETHODIMP GetMaxPixelRate(LPAMVPSIZE pamvpSize,LPDWORD pdwMaxPixelsPerSecond );
		STDMETHODIMP GetOverlaySurface(LPDIRECTDRAWSURFACE *ppddOverlaySurface );
		STDMETHODIMP GetVideoFormats(LPDWORD pdwNumFormats,LPDDPIXELFORMAT pddPixelFormats);
		STDMETHODIMP GetVPDataInfo(LPAMVPDATAINFO pamvpDataInfo);
		STDMETHODIMP IsVPDecimationAllowed(LPBOOL pbIsDecimationAllowed);
		STDMETHODIMP InformVPInputFormats(DWORD dwNumFormats,LPDDPIXELFORMAT pDDPixelFormats);
		STDMETHODIMP SetConnectInfo(DWORD dwChosenEntry);
		STDMETHODIMP SetDDSurfaceKernelHandle(DWORD dwDDKernelHandle);
		STDMETHODIMP SetDDSurfaceKernelHandles(DWORD dwDDKernelHandles,DWORD *pDDKernelHandlers);
		STDMETHODIMP SetDirectDrawKernelHandle(DWORD dwDDKernelHandle);
		STDMETHODIMP SetInvertPolarity(void);
		STDMETHODIMP SetSurfaceParameters(DWORD dwPitch,DWORD dwXOrigin,DWORD dwYOrigin );
		STDMETHODIMP SetVideoPortID (DWORD dwVideoPortID);
		STDMETHODIMP SetVideoFormat (DWORD dwVideoPortID);
		STDMETHODIMP SetScalingFactors(LPAMVPSIZE pamvpSize);

	private:
		CCritSec	*m_pVPConfigLock;
	};
}
#endif /* _DSVPCONFIG_H */
