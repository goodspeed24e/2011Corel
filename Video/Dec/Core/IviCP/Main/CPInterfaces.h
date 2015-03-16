
#ifndef _CONTENT_PROTECTION_INTERFACES_H_
#define _CONTENT_PROTECTION_INTERFACES_H_

#include "CPGuids.h"
#include "CPDefines.h"

enum ECPFrameType
{		
	CP_I_TYPE = 1,
	CP_P_TYPE,
	CP_B_TYPE	
};

enum ECPOption
{
	E_CP_OPTION_OFF = 0,
	E_CP_OPTION_ON = 1,
	E_CP_OPTION_AUTO = 1<<1,
	E_CP_OPTION_DBG_MSG = 1<<2,
	E_CP_OPTION_I_ONLY = 1<<3,
	E_CP_OPTION_AUDIO = 1<<8
};

enum ECPObjID
{	
	E_CP_ID_UNKNOWN = 0,
	E_CP_ID_ATI,
	E_CP_ID_ATI_MP2IDCT,
	E_CP_ID_NVIDIA,
	E_CP_ID_NVIDIA_MP2IDCT,
	E_CP_ID_INTEL,
	E_CP_ID_INTEL_PAVP,
	E_CP_ID_S3,
	E_CP_ID_WIN7
};

enum VenderID
{
	VENDER_ID_DEFAULT = 0,
	VENDER_ID_NVIDIA = 1,
	VENDER_ID_ATI = 2,
	VENDER_ID_INTEL = 3,
	VENDER_ID_S3 = 4
};

struct CPOpenOptions
{
	IUnknown *pVA;
	IUnknown* pD3D9;
	GUID *pDecodeProfile;
	DWORD dwFrameWidth;
	DWORD dwDeviceID;
	DWORD dwVenderID;
	BOOL bIsOverlay;
	BOOL bProtectedBlt;
	DWORD dwCPOption;	
	unsigned int dwAudioStreamId;
    BOOL bIsFlipEx;
    BOOL bDShowMode;
};

DECLARE_INTERFACE_(ICPService, IUnknown)
{
	STDMETHOD(Open)(CPOpenOptions *pOpenOptions) PURE;
	STDMETHOD(Close)() PURE;
	STDMETHOD_(BOOL, IsScramblingRequired)(ECPFrameType eImgType=CP_I_TYPE) PURE;	
	STDMETHOD(EnableScrambling)() PURE;
	STDMETHOD(DisableScrambling)() PURE;
	STDMETHOD(ScrambleData)(BYTE *pDataOut, const BYTE *pDataIn, DWORD dwDataLen) PURE;	
	STDMETHOD_(BYTE, GetEncryptMode)() PURE;
	STDMETHOD(ProtectedBlt)(DWORD dwWidth, DWORD dwHeight, DWORD dwPitch, BYTE *pSrc) PURE;	
	STDMETHOD_(ECPObjID, GetObjID)() PURE;	
};

DECLARE_INTERFACE_(ICPServiceAtiMp2IDCT, ICPService)
{			
	STDMETHOD(SkipMBlock)(DWORD dwDeviceID) PURE;
	STDMETHOD(RescrambleDXVACoeff)(DWORD dwDeviceID, LPVOID pSrc, LPVOID pDest) PURE;
	STDMETHOD(RescrambleDXVABlock)(DWORD dwDeviceID, LPVOID pSrc, LPVOID pDest, DWORD dwNumOfBytes) PURE;		
};

DECLARE_INTERFACE_(ICPServiceNvidiaMp2IDCT, ICPService)
{			
	STDMETHOD(UpdatelfsrIB)(WORD *wPadBlock) PURE;
	STDMETHOD_(WORD, ScrambleDataIDCT)(WORD wPadBlock, BYTE byIndex, WORD wData) PURE;		
};

DECLARE_INTERFACE_(ICheckPavpDevice, IUnknown)
{				
	STDMETHOD_(BOOL, IsPAVPDevice)(BOOL bDShow, IUnknown *pUnknown, BOOL bIsAudio=FALSE) PURE;	
};

DECLARE_INTERFACE_(IGetParams, IUnknown)
{				
	STDMETHOD_(LPVOID, GetIV)() PURE;	
	STDMETHOD_(LPVOID, GetContentKey)() PURE;	
};

#endif