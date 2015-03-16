
#include "GPUCPGuids.h"
#include "GPUCPDefines.h"

struct GPUCPOpenOptions
{
	IUnknown *pVA;
	IUnknown* pD3D9;
	GUID *pDecodeProfile;
	DWORD dwDeviceID;
	DWORD dwVenderID;
	BOOL bIsOverlay;
	DWORD dwCPOption;	
    BOOL bDShowMode;
};

DECLARE_INTERFACE_(IGPUCPService, IUnknown)
{
	STDMETHOD(Open)(GPUCPOpenOptions *pOpenOptions) PURE;
	STDMETHOD(Close)() PURE;
	STDMETHOD(Associate)(GPUCPOpenOptions *pOpenOptions) PURE;
	STDMETHOD(FreeResources)() PURE;
	STDMETHOD(EnableScrambling)() PURE;
	STDMETHOD(DisableScrambling)() PURE;
	STDMETHOD(ScrambleData)(BYTE *pDataOut, const BYTE *pDataIn, DWORD dwDataLen) PURE;	
};

DECLARE_INTERFACE_(IGPUCPGetParams, IUnknown)
{				
	STDMETHOD_(LPVOID, GetIV)() PURE;	
	STDMETHOD_(LPVOID, GetContentKey)() PURE;	
	STDMETHOD(IsPAVPSessionAvailable)() PURE;
	STDMETHOD_(BOOL,   IsCryptoSessionAvailable)() PURE;
	STDMETHOD_(BOOL,   IsChannelOpened)() PURE;
};

