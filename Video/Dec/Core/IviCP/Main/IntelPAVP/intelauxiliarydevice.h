#ifndef INTEL_AUXILIARY_DEVICE_INCLUDED
#define INTEL_AUXILIARY_DEVICE_INCLUDED

#include <windows.h>
#include <windowsx.h>
#include <dwmapi.h>
#include <mmsystem.h>
#include <initguid.h>
#include <d3d9.h>
#include <dxva2api.h>
#include <dxva.h>
#include "iviDxva2.h"
#include <stdio.h>

// Intel Auxiliary Device
// {A74CCAE2-F466-45ae-86F5-AB8BE8AF8483}
static GUID DXVA2_Intel_Auxiliary_Device = 
{ 0xa74ccae2, 0xf466, 0x45ae, { 0x86, 0xf5, 0xab, 0x8b, 0xe8, 0xaf, 0x84, 0x83 } };

// DXVA2 Intel Auxiliary Device Function IDs
typedef enum tagAUXILIARY_DEVICE_FUNCTION_ID
{
    AUXDEV_GET_ACCEL_GUID_COUNT         = 1,
    AUXDEV_GET_ACCEL_GUIDS	            = 2,
    AUXDEV_GET_ACCEL_RT_FORMAT_COUNT	= 3,
    AUXDEV_GET_ACCEL_RT_FORMATS         = 4,
    AUXDEV_GET_ACCEL_FORMAT_COUNT       = 5,
    AUXDEV_GET_ACCEL_FORMATS            = 6,
    AUXDEV_QUERY_ACCEL_CAPS             = 7,
    AUXDEV_CREATE_ACCEL_SERVICE         = 8,
    AUXDEV_DESTROY_ACCEL_SERVICE        = 9
} AUXILIARY_DEVICE_FUNCTION_ID;


// Intel Auxiliary Device Base Class
class CIntelAuxiliaryDevice : public CiviDxva2Decoder
{
protected:
	IDirectXVideoDecoder*			m_pAuxiliaryDevice;
	BOOL m_bShowMsg;

private:
	DXVA2_ConfigPictureDecode*		m_pPictureDecode;
	DXVA2_VideoDesc					m_sDxva2VidDesc;

protected:
	CIntelAuxiliaryDevice(IMFGetService *pD3DDevice9);
    CIntelAuxiliaryDevice(IDirect3DDevice9 *pD3DDevice9);
    ~CIntelAuxiliaryDevice();

    HRESULT QueryAccelGuids    (GUID **ppAccelGuids, UINT *puAccelGuidCount);
    HRESULT QueryAccelRTFormats(CONST GUID *pAccelGuid, D3DFORMAT **ppFormats, UINT *puCount);
    HRESULT QueryAccelCaps     (CONST GUID *pAccelGuid, void *pCaps, UINT *puCapSize);
    HRESULT CreateAccelService (CONST GUID *pAccelGuid, void *pCreateParams, UINT *puCreateParamSize);
	HRESULT ExecuteFunction    (UINT uiFuncID, void *pDataIn, UINT uiSizeIn, void *pOutputData, UINT uiSizeOut);
	void PrepareAuxFuncStruct  (DXVA2_DecodeExecuteParams * const pExecParams, DXVA2_DecodeExtensionData *pExtData, 
								UINT funcId, void const * const pGuid, INT sizeIn, void const * const pDataOut, INT	sizeOut);
	VOID	DP(CHAR* szMsg, ...);	
	
	};

#endif // INTEL_AUXILIARY_DEVICE_INCLUDED