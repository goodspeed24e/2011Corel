#ifndef INTEL_AUXILIARY_DEVICE_INCLUDED
#define INTEL_AUXILIARY_DEVICE_INCLUDED

#include <windows.h>
#include <windowsx.h>
#include <dwmapi.h>
#include <mmsystem.h>
#include <tchar.h>
#include <strsafe.h>
#include <initguid.h>
#include <d3d9.h>
#include <dxva2api.h>
#include <dxva.h>

typedef HRESULT  (__stdcall *TpfnDXVA2CreateVideoService)(IDirect3DDevice9* pDD, REFIID riid, void** ppService);

// Debug Macro
#if defined(DBG) || defined(DEBUG) || defined(_DEBUG)
  #define DBGMSG(x)  //{DbgPrint(TEXT("%s(%u) : "), TEXT(__FILE__), __LINE__); DbgPrint x;}
  VOID DbgPrint(PCTSTR format, ...);
#else   // DBG || DEBUG || _DEBUG
  #define DBGMSG(x)
#endif  // DBG || DEBUG || _DEBUG

// Intel Auxiliary Device
// {A74CCAE2-F466-45ae-86F5-AB8BE8AF8483}
static const GUID DXVA2_Intel_Auxiliary_Device = 
{ 0xa74ccae2, 0xf466, 0x45ae, { 0x86, 0xf5, 0xab, 0x8b, 0xe8, 0xaf, 0x84, 0x83 } };


// DXVA2 Intel Auxiliary Device Function IDs
typedef enum tagAUXILIARY_DEVICE_FUNCTION_ID
{
    AUXDEV_GET_ACCEL_GUID_COUNT         = 1,
    AUXDEV_GET_ACCEL_GUIDS              = 2,
    AUXDEV_GET_ACCEL_RT_FORMAT_COUNT    = 3,
    AUXDEV_GET_ACCEL_RT_FORMATS         = 4,
    AUXDEV_GET_ACCEL_FORMAT_COUNT       = 5,
    AUXDEV_GET_ACCEL_FORMATS            = 6,
    AUXDEV_QUERY_ACCEL_CAPS             = 7,
    AUXDEV_CREATE_ACCEL_SERVICE         = 8,
    AUXDEV_DESTROY_ACCEL_SERVICE        = 9
} AUXILIARY_DEVICE_FUNCTION_ID;


// Intel Auxiliary Device Base Class
class CIntelAuxiliaryDevice
{
protected:
    IDirectXVideoDecoder        *m_pAuxiliaryDevice;
    IDirectXVideoDecoderService *m_pVideoDecoderService;

private:
    IDirect3DSurface9           *m_pRenderTarget;
    DXVA2_VideoDesc             m_sVideoDescr;
    DXVA2_ConfigPictureDecode   m_sPictureDecode;

public:
    CIntelAuxiliaryDevice(IDirect3DDevice9 *pD3DDevice9, HMODULE hAuxDXVA2);
    ~CIntelAuxiliaryDevice();

    HRESULT QueryAccelGuids    (GUID **ppAccelGuids, UINT *puAccelGuidCount);
    HRESULT QueryAccelRTFormats(CONST GUID *pAccelGuid, D3DFORMAT **ppFormats, UINT *puCount);
    HRESULT QueryAccelFormats  (CONST GUID *pAccelGuid, D3DFORMAT **ppFormats, UINT *puCount);
    HRESULT QueryAccelCaps     (CONST GUID *pAccelGuid, void *pCaps, UINT *puCapSize);
    HRESULT CreateAccelService (CONST GUID *pAccelGuid, void *pCreateParams, UINT *puCreateParamSize);
};

#endif // INTEL_AUXILIARY_DEVICE_INCLUDED