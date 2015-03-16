#pragma once

#include <d3d9.h>
#include <mfidl.h>
#include <evr.h>
#include "HVDTestDef.h"
#include "DXVATestProgram.h"
#include "../../Exports/Inc/HVDService.h"

typedef HVDSERVICE_API int (*PFN_CreateHVDService)(DWORD dwService, void** ppHVDService);
typedef HRESULT  (__stdcall *TpfnDirect3DCreate9)(UINT uVersion);
typedef HRESULT  (__stdcall *TpfnDirect3DCreate9Ex)(UINT uVersion, IDirect3D9Ex **ppService);

typedef struct
{
	UINT  nIndex;
	DWORD dwValue;
	TCHAR tcName[50];
	TCHAR tcINIKeyName[50];
	TCHAR tcConfigFile[_MAX_PATH];
} CodecInfo;

typedef struct  
{
	TCHAR tcINIKeyName[50];
	BOOL  bDisabled;
	DWORD dwValue;
} DisabledItemInfo;

typedef struct  
{
	TCHAR tcINIKeyName[50];
	BOOL  bExecute;
	DWORD dwValue;
} CommandInfo;

class CHVDTestCenter
{
public:
	CHVDTestCenter(void);
public:
	~CHVDTestCenter(void);

public:
	static unsigned __stdcall LaunchTestThread(void *pHVDTestCenter);

	void SetParentWnd(CWnd *pParentWnd);
	void SetVideoWnd(CWnd *pVideoWnd);
	void SetupHVDTestCenter();
	void CloseHVDTestCenter();
	DWORD GetSystemVersion();
	DWORD GetSupportedTestItem();
	DWORD GetSupportedCodec();
	DWORD GetSelectedTestItem();
	DWORD GetSelectedCodec();
	HRESULT LaunchTest(DWORD dwTestItem, DWORD dwTestCodecType, BOOL bAutoClose);
	HRESULT LaunchTest(TCHAR *ptcConfigFile = NULL);
	HRESULT StopTest(BOOL bWait);
protected:
	BOOL LoadConfigFile(TCHAR *ptcConfigFile);
	HRESULT ParseConfigFileCommand(TCHAR *ptcConfigFile);
	HRESULT OpenHVDService();
	HRESULT CloseHVDService();
	HRESULT OpenDirect3DDeviceEx();
	HRESULT CloseDirect3DDeviceEx();

	HRESULT LaunchDXVA1Test();
	HRESULT LaunchDXVA2Test();

	void HVDTestInProgress();
	void HVDTestFinish();
	void HVDTestClose();
protected:
	HMODULE m_hD3D9DLL;
	HMODULE m_hHVDServiceDLL;

	PFN_CreateHVDService m_pfnCreateHVDSerice;
	TpfnDirect3DCreate9Ex m_pfnDirect3DCreate9Ex;

	CComPtr<IDirect3D9> m_pD3D9;
	CComPtr<IDirect3D9Ex> m_pD3D9Ex;
	CComPtr<IDirect3DDevice9> m_pDevice;
	CComPtr<IDirect3DDevice9Ex> m_pDeviceEx;
	CComPtr<IMFGetService> m_pMFGetService;

	D3DDISPLAYMODE m_d3dDisplayMode; // Display mode for current device.

	CodecInfo m_CodecObjectInfo[CODEC_TEST_ITEM_NUM];
	DisabledItemInfo m_DisabledDXVA1Item[DXVA1_TEST_ITEM_NUM];
	DisabledItemInfo m_DisabledDXVA2Item[DXVA2_TEST_ITEM_NUM];
	DisabledItemInfo m_DisabledCodec[CODEC_TEST_ITEM_NUM];
	TCHAR m_tcSelectedConfigFile[256];

protected:
	HANDLE m_hLaunchTestThread;
	HANDLE m_hLaunchEvent;
	HANDLE m_hExitEvent;
	unsigned m_nThreadID;

	CWnd *m_pParentWnd;
	CWnd *m_pVideoWnd;

	DWORD m_dwTestItem;
	DWORD m_dwTestCodecType;
	BOOL m_bAutoLaunch;
	BOOL m_bAutoClose;

	FILE* m_fpDecodeLog;
};
