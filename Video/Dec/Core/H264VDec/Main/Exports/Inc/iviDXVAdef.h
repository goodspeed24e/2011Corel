#ifndef _IVI_DXVA_DEF_H_
#define _IVI_DXVA_DEF_H_

#pragma warning( disable: 4099 )

class IAMVideoAccelerator;
class IDirectXVideoDecoder;

typedef enum _TiviDxvaVer {IviNotDxva = 0, IviDxva1 = 1, IviDxva2 = 2} TiviDxvaVer;

typedef union _TiviDxvaP
{
	IAMVideoAccelerator*	pDxva1;
	IDirectXVideoDecoder*	pDxva2;
}TiviDxvaP;

typedef struct _TiviDXVA
{
	TiviDxvaVer dxvaVer;
	TiviDxvaP	pDxva;
	bool operator!() { return !pDxva.pDxva1;}
}TiviDxva;

typedef struct _DXVA_ConfigPictureDecode  DXVA_ConfigPictureDecode;
typedef struct _DXVA2_ConfigPictureDecode DXVA2_ConfigPictureDecode;

typedef union _TPiviDXVAConfigPictureDecode 
{
	DXVA_ConfigPictureDecode*  m_pDxva1;
	DXVA2_ConfigPictureDecode* m_pDxva2;
	_TPiviDXVAConfigPictureDecode(DXVA_ConfigPictureDecode*  pDxva1) : m_pDxva1(pDxva1) {}
	_TPiviDXVAConfigPictureDecode(DXVA2_ConfigPictureDecode* pDxva2) : m_pDxva2(pDxva2) {}
	_TPiviDXVAConfigPictureDecode(int x) : m_pDxva1(reinterpret_cast<DXVA_ConfigPictureDecode*>(x)) {}
	_TPiviDXVAConfigPictureDecode() {m_pDxva1 = 0;}
}TPiviDXVAConfigPictureDecode;

#ifndef __DXVA_GeneralInfo
#define __DXVA_GeneralInfo
typedef struct _DXVA_GeneralInfo {
	union TSurfaceType
	{
		const DWORD					*m_Dw;
		const enum _D3DFORMAT		*m_D3DFormat;
	}pSurfaceType;

	LPVOID						m_pCompBufferInfo;

	// DXVA
	BOOL						*m_bDecodeBufHold;
	DWORD						*m_dwAlphaBufWidth;
	DWORD						*m_dwAlphaBufHeight;
	DWORD						*m_dwAlphaBufSize;
	DWORD						*m_dwDecodeFrame;
	DWORD						*m_dwDecodeBufNum;
	BOOL						*m_bCssSafe;
	BOOL						*m_bUseEncryption;
	BOOL						*m_bConfigDecoder;
	DWORD						*m_dwSWDeinterlace;

	DWORD						*m_dwVendorID;			// may be 0 if unknown
	DWORD						*m_dwDeviceID;			// may be 0 if unknown

	BOOL						*m_bDxva2ReleaseCompBuffer; // in DXVA2, Comp buffer may have to be released before calling Execute
} DXVA_GeneralInfo, *LPDXVA_GeneralInfo;
#endif

#endif

