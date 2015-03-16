#include "StdAfx.h"
#include <process.h>
#include "HVDTestCenter.h"
#include "CtrlPanel.h"

CommandInfo CommandGeneralInfoTable[] =
{
	{ KEY_NAME_AUTOCLOSE, FALSE, TRUE},
};

CommandInfo CommandTestItemInfoTable[] =
{
	{ KEY_NAME_DXVA1_OVERLAY, FALSE, TEST_ITEM_DXVA1_OVERLAY},
	{ KEY_NAME_DXVA1_VMR7, FALSE, TEST_ITEM_DXVA1_VMR7},
	{ KEY_NAME_DXVA1_VMR9WINDOWLESS, FALSE, TEST_ITEM_DXVA1_VMR9WINDOWLESS},
	{ KEY_NAME_DXVA1_VMR9CUSTOM, FALSE, TEST_ITEM_DXVA1_VMR9CUSTOM},
	{ KEY_NAME_DXVA1_EXTERNAL, FALSE, TEST_ITEM_DXVA1_EXTERNAL},
	{ KEY_NAME_DXVA2_DIRECT3D, FALSE, TEST_ITEM_DXVA2_D3D},
	{ KEY_NAME_DXVA2_DIRECTSHOW, FALSE, TEST_ITEM_DXVA2_DSHOW},
};

CommandInfo CommandCodecInfoTable[] =
{
	{ KEY_NAME_CODEC_MPEG2, FALSE, TEST_CODEC_MPEG2},
	{ KEY_NAME_CODEC_H264, FALSE, TEST_CODEC_H264},
	{ KEY_NAME_CODEC_VC1, FALSE, TEST_CODEC_VC1},
};


CHVDTestCenter::CHVDTestCenter(void)
{
	::CoInitialize(NULL);

	m_pParentWnd = NULL;
	m_pVideoWnd = NULL;
	m_hD3D9DLL = NULL;
	m_hHVDServiceDLL = NULL;

	m_dwTestItem = NULL;
	m_dwTestCodecType = NULL;
	m_bAutoLaunch = FALSE;
	m_bAutoClose = FALSE;
	ZeroMemory( &m_d3dDisplayMode, sizeof(m_d3dDisplayMode));

	m_pfnCreateHVDSerice = NULL;
	m_pfnDirect3DCreate9Ex = NULL;
	m_nThreadID = NULL;
	m_hLaunchEvent = CreateEvent(NULL, TRUE, FALSE, _T("Launch Event"));
	m_hExitEvent = CreateEvent(NULL, TRUE, FALSE, _T("Exit Event"));
	m_hLaunchTestThread = (HANDLE)_beginthreadex( NULL, 0, &LaunchTestThread, this, 0, &m_nThreadID);
//Codec Ini file
	ZeroMemory(m_CodecObjectInfo, sizeof(m_CodecObjectInfo));
	CodecInfo tempTable[] = 
	{
		{ 0, TEST_CODEC_MPEG2, _T("MPEG2"), KEY_NAME_MPEG2INI, _T("")},
		{ 1, TEST_CODEC_H264, _T("H264"), KEY_NAME_H264INI, _T("")},
		{ 2, TEST_CODEC_VC1, _T("VC1"), KEY_NAME_VC1, _T("")},
		{ 3, TEST_CODEC_MPEG4, _T("MPEG4"), KEY_NAME_MPEG4INI, _T("")},
	};
	memcpy_s(m_CodecObjectInfo, sizeof(m_CodecObjectInfo), tempTable, sizeof(tempTable));

//Disable DXVA1 item
	ZeroMemory(m_DisabledDXVA1Item, sizeof(m_DisabledDXVA1Item));
	DisabledItemInfo tempDisabledDXVA1Table[] = 
	{
		{ KEY_NAME_DISABLE_DXVA1_OVERLAY,			FALSE, TEST_ITEM_DXVA1_OVERLAY},
		{ KEY_NAME_DISABLE_DXVA1_VMR7,				FALSE, TEST_ITEM_DXVA1_VMR7},
		{ KEY_NAME_DISABLE_DXVA1_VMR9WINDOWLESS,	FALSE, TEST_ITEM_DXVA1_VMR9WINDOWLESS},
		{ KEY_NAME_DISABLE_DXVA1_VMR9CUSTOM,		FALSE, TEST_ITEM_DXVA1_VMR9CUSTOM},
		{ KEY_NAME_DISABLE_DXVA1_EXTERNAL,			FALSE, TEST_ITEM_DXVA1_EXTERNAL},
	};
	memcpy_s(m_DisabledDXVA1Item, sizeof(m_DisabledDXVA1Item), tempDisabledDXVA1Table, sizeof(tempDisabledDXVA1Table));

//Disable DXVA2 item
	ZeroMemory(m_DisabledDXVA2Item, sizeof(m_DisabledDXVA2Item));
	DisabledItemInfo tempDisabledDXVA2Table[] = 
	{
		{ KEY_NAME_DISABLE_DXVA2_DIRECT3D,			FALSE, TEST_ITEM_DXVA2_D3D},
		{ KEY_NAME_DISABLE_DXVA2_DIRECTSHOW,		FALSE, TEST_ITEM_DXVA2_DSHOW},
	};
	memcpy_s(m_DisabledDXVA2Item, sizeof(m_DisabledDXVA2Item), tempDisabledDXVA2Table, sizeof(tempDisabledDXVA2Table));

//Disable codec item
	ZeroMemory(m_DisabledCodec, sizeof(m_DisabledCodec));
	DisabledItemInfo tempDisabledCodecTable[] = 
	{
		{ KEY_NAME_DISABLE_CODEC_MPEG2,				FALSE, TEST_CODEC_MPEG2},
		{ KEY_NAME_DISABLE_CODEC_H264,				FALSE, TEST_CODEC_H264},
		{ KEY_NAME_DISABLE_CODEC_VC1,				FALSE, TEST_CODEC_VC1},
		{ KEY_NAME_DISABLE_CODEC_MPEG4,				FALSE, TEST_CODEC_MPEG4}
	
	};
	memcpy_s(m_DisabledCodec, sizeof(m_DisabledCodec), tempDisabledCodecTable, sizeof(tempDisabledCodecTable));

	TCHAR tcConfigFile[256];
	GetCurrentDirectory(256, tcConfigFile);
	_tcscat_s(tcConfigFile, 256, _T("\\"));
	_tcscat_s(tcConfigFile, 256, DEFAULT_CONFIG_FILE);
	LoadConfigFile(tcConfigFile);

	m_fpDecodeLog = NULL;
	m_fpDecodeLog = fopen("C:\\DecodingLog.txt", "wb");
}

CHVDTestCenter::~CHVDTestCenter(void)
{
	CloseHVDTestCenter();
	::CoUninitialize();

	if(m_fpDecodeLog)
	{
		fclose(m_fpDecodeLog);
		m_fpDecodeLog = NULL;
	}
}

unsigned CHVDTestCenter::LaunchTestThread(void *pHVDTestCenter)
{
	::CoInitialize(NULL);
	CHVDTestCenter *pTempHVDTestCenter = (CHVDTestCenter *)pHVDTestCenter;
	BOOL bTestDone = FALSE;
	HANDLE hWaitEvent[2] = { pTempHVDTestCenter->m_hExitEvent, pTempHVDTestCenter->m_hLaunchEvent};
	DWORD dwReturnObject;
	while (1)
	{
		dwReturnObject = WaitForMultipleObjects( 2, hWaitEvent, FALSE, INFINITE);
		if (dwReturnObject == WAIT_OBJECT_0)
			break;

		pTempHVDTestCenter->LaunchDXVA1Test();
		if (WaitForSingleObject(pTempHVDTestCenter->m_hExitEvent, 0) == WAIT_OBJECT_0)
			break;
		pTempHVDTestCenter->LaunchDXVA2Test();

		pTempHVDTestCenter->HVDTestFinish();

		if (pTempHVDTestCenter->m_bAutoClose)
			pTempHVDTestCenter->HVDTestClose();

		ResetEvent(pTempHVDTestCenter->m_hLaunchEvent);

	}
	::CoUninitialize();
	_endthreadex(0);
	return 0;
}

void CHVDTestCenter::SetParentWnd(CWnd *pParentWnd)
{
	m_pParentWnd = pParentWnd;
}

void CHVDTestCenter::SetVideoWnd(CWnd *pVideoWnd)
{
	m_pVideoWnd = pVideoWnd;
}

void CHVDTestCenter::SetupHVDTestCenter()
{
	OpenHVDService();
	if (GetSystemVersion() > 5)
		OpenDirect3DDeviceEx();
}

void CHVDTestCenter::CloseHVDTestCenter()
{
	if (m_hExitEvent)
	{
		SetEvent(m_hExitEvent);
		WaitForSingleObject(m_hLaunchTestThread, INFINITE);
		CloseHandle( m_hLaunchTestThread );
		m_hLaunchTestThread = NULL;
		CloseHandle( m_hExitEvent );
		m_hExitEvent = NULL;
	}
	CloseHVDService();
	CloseDirect3DDeviceEx();
}

DWORD CHVDTestCenter::GetSystemVersion()
{
	OSVERSIONINFOEX VersionInfoEx;
	sizeof(&VersionInfoEx, sizeof(VersionInfoEx));
	VersionInfoEx.dwOSVersionInfoSize = sizeof(VersionInfoEx);
	GetVersionEx((OSVERSIONINFO *)&VersionInfoEx);
	return VersionInfoEx.dwMajorVersion;
}
DWORD CHVDTestCenter::GetSupportedTestItem()
{
	DWORD dwSupportedTestItem = NULL;
	UINT nItemSize = sizeof(m_DisabledDXVA1Item) / sizeof(m_DisabledDXVA1Item[0]);
	for (UINT i = 0; i < nItemSize; i++)
	{
		if (!m_DisabledDXVA1Item[i].bDisabled)
			dwSupportedTestItem |= m_DisabledDXVA1Item[i].dwValue; //set all non-disabled item
	}

	nItemSize = sizeof(m_DisabledDXVA2Item) / sizeof(m_DisabledDXVA2Item[0]);
	for (UINT i = 0; i < nItemSize; i++)
	{
		if (!m_DisabledDXVA2Item[i].bDisabled)
			dwSupportedTestItem |= m_DisabledDXVA2Item[i].dwValue; //set all non-disabled item
	}
	return dwSupportedTestItem;
}

DWORD CHVDTestCenter::GetSupportedCodec()
{
	DWORD dwSupportedCodec = NULL;
	DWORD dwDisabledCodec = NULL;
	UINT nCodecSize = sizeof(m_CodecObjectInfo) / sizeof(m_CodecObjectInfo[0]);
	for (UINT i = 0; i < nCodecSize; i++)
	{
		if (_tcslen(m_CodecObjectInfo[i].tcConfigFile) != 0)
			dwSupportedCodec |= m_CodecObjectInfo[i].dwValue;
	}

	nCodecSize = sizeof(m_DisabledCodec) / sizeof(m_DisabledCodec[0]);
	for (UINT i = 0; i < nCodecSize; i++)
	{
		if (m_DisabledCodec[i].bDisabled)
			dwDisabledCodec |= m_DisabledCodec[i].dwValue;
	}
	return (dwSupportedCodec &= ~dwDisabledCodec);
}

HRESULT CHVDTestCenter::LaunchTest(DWORD dwTestItem, DWORD dwTestCodecType, BOOL bAutoClose)
{
	if (dwTestItem == NULL || dwTestCodecType == NULL)
		return E_INVALIDARG;

	m_dwTestItem = dwTestItem;
	m_dwTestCodecType = dwTestCodecType;
	m_bAutoClose = bAutoClose;

	if (WaitForSingleObject(m_hLaunchEvent, 0) == WAIT_TIMEOUT)
	{
		SetEvent(m_hLaunchEvent);
		HVDTestInProgress();
	}
	else
	{
		HVDTestInProgress();
		return E_ACCESSDENIED;
	}
	return S_OK;
}

HRESULT CHVDTestCenter::LaunchTest(TCHAR *ptcConfigFile)
{
	if (!ptcConfigFile) //Load Default File;
		return E_FAIL;
//load config info
	LoadConfigFile(ptcConfigFile);
//Parse command and launch
	return ParseConfigFileCommand(ptcConfigFile);
}

HRESULT CHVDTestCenter::StopTest(BOOL bWait)
{
	SetEvent(m_hExitEvent);
	DWORD dwRet = WaitForSingleObject(m_hLaunchTestThread, bWait ? INFINITE : 0);
	if (dwRet == WAIT_TIMEOUT)
		return S_OK;
	else
		return S_FALSE;
}

BOOL CHVDTestCenter::LoadConfigFile(TCHAR *ptcConfigFile)
{
	_tcscpy_s(m_tcSelectedConfigFile, 256, ptcConfigFile);
	
	if (_tcslen(m_tcSelectedConfigFile) == 0)
		return FALSE;

	UINT nTableSize = sizeof(m_CodecObjectInfo) / sizeof(m_CodecObjectInfo[0]);
	for (UINT i = 0; i < nTableSize; i++)
	{
		GetPrivateProfileString(SECTION_NAME_GENERAL, m_CodecObjectInfo[i].tcINIKeyName, _T(""),
			m_CodecObjectInfo[i].tcConfigFile, _MAX_PATH, m_tcSelectedConfigFile);
	}

	nTableSize = sizeof(m_DisabledDXVA1Item) / sizeof(m_DisabledDXVA1Item[0]);
	for (UINT i = 0; i < nTableSize; i++)
	{
		m_DisabledDXVA1Item[i].bDisabled = GetPrivateProfileInt(SECTION_NAME_GENERAL, m_DisabledDXVA1Item[i].tcINIKeyName, 0, m_tcSelectedConfigFile);
	}

	DWORD dwVer = GetSystemVersion();
	nTableSize = sizeof(m_DisabledDXVA2Item) / sizeof(m_DisabledDXVA2Item[0]);
	for (UINT i = 0; i < nTableSize; i++) // Disable all DXVA2 item on XP
	{
		m_DisabledDXVA2Item[i].bDisabled = (dwVer > 5) ? GetPrivateProfileInt(SECTION_NAME_GENERAL, m_DisabledDXVA2Item[i].tcINIKeyName, 0, m_tcSelectedConfigFile) : TRUE;
	}

	nTableSize = sizeof(m_DisabledCodec) / sizeof(m_DisabledCodec[0]);
	for (UINT i = 0; i < nTableSize; i++)
	{
		m_DisabledCodec[i].bDisabled = GetPrivateProfileInt(SECTION_NAME_GENERAL, m_DisabledCodec[i].tcINIKeyName, 0, m_tcSelectedConfigFile);
	}

	return TRUE;
}

HRESULT CHVDTestCenter::ParseConfigFileCommand(TCHAR *ptcConfigFile)
{
	if (!ptcConfigFile)
		return E_FAIL;

	DWORD dwTestItem = NULL;
	DWORD dwTestCodecType = NULL;
	BOOL bAutoLaunch = FALSE;
	BOOL  bAutoClose = FALSE;
	UINT nTableSize = NULL;
	UINT nRet = NULL;
	
	bAutoLaunch = GetPrivateProfileInt( SECTION_NAME_COMMAND, KEY_NAME_AUTOLAUNCH, 0, m_tcSelectedConfigFile);
	if (!bAutoLaunch) // since auto-launch command is disabled, we should ignore all commands.
	{
		if (WaitForSingleObject(m_hLaunchEvent, 0) == WAIT_TIMEOUT) //Update control panel state back to default 
		{
			HVDTestFinish();
		}
		return S_FALSE;
	}
	bAutoClose = GetPrivateProfileInt( SECTION_NAME_COMMAND, KEY_NAME_AUTOCLOSE, 0, m_tcSelectedConfigFile);

	nTableSize = sizeof(CommandTestItemInfoTable) / sizeof(CommandTestItemInfoTable[0]);
	for (UINT i = 0; i < nTableSize; i++)
	{
		nRet = GetPrivateProfileInt( SECTION_NAME_COMMAND, CommandTestItemInfoTable[i].tcINIKeyName, 0, m_tcSelectedConfigFile);
		if (nRet)
			dwTestItem |= CommandTestItemInfoTable[i].dwValue;
	}

	nTableSize = sizeof(CommandCodecInfoTable) / sizeof(CommandCodecInfoTable[0]);
	for (UINT i = 0; i < nTableSize; i++)
	{
		nRet = GetPrivateProfileInt( SECTION_NAME_COMMAND, CommandCodecInfoTable[i].tcINIKeyName, 0, m_tcSelectedConfigFile);
		if (nRet)
			dwTestCodecType |= CommandCodecInfoTable[i].dwValue;
	}
		
	if (!dwTestItem || !dwTestCodecType)
	{
		if (WaitForSingleObject(m_hLaunchEvent, 0) == WAIT_TIMEOUT) //Update control panel state back to default 
		{
			HVDTestFinish();
		}
		return S_FALSE;
	}
	return LaunchTest(dwTestItem, dwTestCodecType, bAutoClose);
}

HRESULT CHVDTestCenter::OpenHVDService()
{
	if (m_pfnCreateHVDSerice)
		return S_OK;

	if (!m_hHVDServiceDLL)
	{
		m_hHVDServiceDLL = LoadLibrary(_T("HVDService.dll"));
	}

	if (m_hHVDServiceDLL)
	{
		if (!m_pfnCreateHVDSerice)
			m_pfnCreateHVDSerice = (PFN_CreateHVDService)GetProcAddress(m_hHVDServiceDLL, "CreateHVDService");
	}
	else
		return E_FAIL;
		 
	if (m_pfnCreateHVDSerice)
		return S_OK;
	else
		return E_FAIL;
}

HRESULT CHVDTestCenter::CloseHVDService()
{
	if (m_hHVDServiceDLL)
	{
		FreeLibrary(m_hHVDServiceDLL);
		m_hHVDServiceDLL = NULL;
	}
	return S_OK;
}

HRESULT CHVDTestCenter::OpenDirect3DDeviceEx()
{
	if (m_pDeviceEx)
		return S_OK;

	HRESULT hr = E_FAIL;
	if (!m_pD3D9Ex)
	{
		m_hD3D9DLL = LoadLibrary(TEXT("d3d9.dll"));
		if (m_hD3D9DLL)
		{
			m_pfnDirect3DCreate9Ex = (TpfnDirect3DCreate9Ex) GetProcAddress(m_hD3D9DLL, "Direct3DCreate9Ex");
		}
		else
			return hr;

		if (m_pfnDirect3DCreate9Ex)
			hr = m_pfnDirect3DCreate9Ex(D3D_SDK_VERSION, &m_pD3D9Ex);
		else
			return hr;

		if (FAILED(hr))
			return hr;
	}
	UINT nAdapter = D3DADAPTER_DEFAULT;
	D3DDEVTYPE type = D3DDEVTYPE_HAL;
	hr = m_pD3D9Ex->GetAdapterDisplayMode(D3DADAPTER_DEFAULT/*nAdapter*/, &m_d3dDisplayMode);
	if (FAILED(hr))
		return hr;
	D3DCAPS9 caps;
	hr = m_pD3D9Ex->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &caps);

	DWORD uDeviceCreateFlag = D3DCREATE_DISABLE_DRIVER_MANAGEMENT_EX;
	uDeviceCreateFlag |= (caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) ? D3DCREATE_HARDWARE_VERTEXPROCESSING : D3DCREATE_SOFTWARE_VERTEXPROCESSING;

	{
		D3DPRESENT_PARAMETERS pp = {0};
		HWND hWnd = m_pVideoWnd ? m_pVideoWnd->GetSafeHwnd() : NULL;
		pp.Windowed = TRUE;
		pp.hDeviceWindow = hWnd;
		pp.BackBufferWidth = m_d3dDisplayMode.Width;//m_sizeBackBuffer.cx;
		pp.BackBufferHeight = m_d3dDisplayMode.Height;//m_sizeBackBuffer.cy;
		pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
		pp.BackBufferCount = 1;
		pp.BackBufferFormat = m_d3dDisplayMode.Format;
		pp.EnableAutoDepthStencil = TRUE;//(m_dwConfigFlags & DISPSVR_USE_STECIL_BUFFER) ? TRUE : FALSE;
		pp.AutoDepthStencilFormat = D3DFMT_D24S8; //(m_dwConfigFlags & DISPSVR_USE_STECIL_BUFFER) ? D3DFMT_D24S8 : D3DFMT_D16;	// ignored if EnableAutoDepthStencil == FALSE
		pp.Flags = D3DPRESENTFLAG_DEVICECLIP;
		pp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

		hr = m_pD3D9Ex->CreateDeviceEx(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, uDeviceCreateFlag, &pp, NULL, &m_pDeviceEx);
	}
	return hr;
}

HRESULT CHVDTestCenter::CloseDirect3DDeviceEx()
{
	m_pDeviceEx = NULL;
	m_pDevice = NULL;
	m_pD3D9Ex = NULL;
	m_pD3D9 = NULL;

	if (m_hD3D9DLL)
	{
		FreeLibrary(m_hD3D9DLL);
		m_hD3D9DLL = NULL;
	}
	return S_OK;
}
HRESULT CHVDTestCenter::LaunchDXVA1Test()
{
	if (!(m_dwTestItem & TEST_ITEM_DXVA1))
		return S_FALSE;

	if (FAILED(OpenHVDService()))
		return E_FAIL;

	HRESULT hr = E_FAIL;
	CComPtr<IHVDService> pHVDService;
	m_pfnCreateHVDSerice((DWORD)HVDService::HVD_SERVICE_DXVA1, (void **)&pHVDService);
	CComQIPtr<IHVDServiceDxva1> pServiceDxva1 = pHVDService;
	if (pHVDService && pServiceDxva1)
	{
		typedef struct
		{
			UINT nTestID;
			UINT nExtInitFlags;
			UINT nRendererType;
		} DXVA1MappingInfo;

		DXVA1MappingInfo DXVA1MappingTable[] =
		{
			{TEST_ITEM_DXVA1_OVERLAY,			NULL,								HVDService::HVD_DXVA1_RENDERER_OVERLAY},
			{TEST_ITEM_DXVA1_VMR7,				NULL,								HVDService::HVD_DXVA1_RENDERER_VMR7},
			{TEST_ITEM_DXVA1_VMR9WINDOWLESS,	NULL,								HVDService::HVD_DXVA1_RENDERER_VMR9WIDNOWLESS},
			{TEST_ITEM_DXVA1_VMR9CUSTOM,		NULL,								HVDService::HVD_DXVA1_RENDERER_VMR9CUSTOM},
			{TEST_ITEM_DXVA1_EXTERNAL,			HVDService::HVD_INIT_BASEFILTER,	HVDService::HVD_DXVA1_RENDERER_EXTERNAL},
		};

		UINT nTableSize = sizeof(DXVA1MappingTable) /sizeof(DXVA1MappingTable[0]);
		HVDService::HVDInitConfig InitConfig;
		HVDService::HVDDecodeConfig DecodeConfig;
		memset(&InitConfig, 0, sizeof(InitConfig));
		InitConfig.dwFlags = HVDService::HVD_INIT_DXVA1_FLAGS;
		InitConfig.m_Dxva1Flags.dwFlags = HVDService::HVD_INIT_DXVA1_RENDERER_TYPE;
		InitConfig.hwnd = m_pVideoWnd->GetSafeHwnd();
		for (UINT i = 0; i < nTableSize; i++)
		{
			if (DXVA1MappingTable[i].nTestID & m_dwTestItem) //Test item is being selected.
			{
				if (WaitForSingleObject(m_hExitEvent, 0) == WAIT_OBJECT_0) //leave if receive exit command
					return S_FALSE;

				InitConfig.dwFlags |= DXVA1MappingTable[i].nExtInitFlags;
				InitConfig.m_Dxva1Flags.dwDxva1RendererType = DXVA1MappingTable[i].nRendererType;
				hr = pHVDService->Initialize(&InitConfig);
				if (SUCCEEDED(hr))
				{
					if (m_dwTestCodecType & TEST_CODEC_MPEG2) // Test MPEG2
					{
						CMP2DXVATestProgram* pTest = new CMP2DXVATestProgram;
						TCHAR tcConfigFile[256];
						TCHAR InputDir[256];
						TCHAR OutputDir[256];
						UINT uiTotalClipNum = 0;
						UINT uiClipCounter = 1;

						pTest->SetDecodeLogFP(m_fpDecodeLog);

						GetCurrentDirectory(256, tcConfigFile);
						_tcscat_s(tcConfigFile, 256, _T("\\"));
						_tcscat_s(tcConfigFile, 256, _T("MP2VDec.ini"));
						pTest->SetConfigFile(tcConfigFile);
						uiTotalClipNum = pTest->GetClipNum();

						while(uiTotalClipNum--)
						{
							TCHAR tcClipName[256];
							_stprintf_s(tcClipName, 256, _T("CLIP%d"), uiClipCounter);

							memset(&DecodeConfig, 0, sizeof(DecodeConfig));
							DecodeConfig.dwWidth = pTest->GetClipWidth(tcClipName);
							DecodeConfig.dwHeight = (((pTest->GetClipHeight(tcClipName)+15)>>4)<<4);
							DecodeConfig.dwMode = HVDService::HVD_MODE_MPEG2;
							DecodeConfig.dwFlags = HVDService::HVD_DECODE_MAXSURFACECOUNT | HVDService::HVD_DECODE_MINSURFACECOUNT | HVDService::HVD_DECODE_MODE;
							DecodeConfig.dwLevel = HVDService::HVD_LEVEL_AUTO;
							DecodeConfig.dwMaxSurfaceCount = 12;

							hr = pHVDService->StartService(&DecodeConfig);
							if (SUCCEEDED(hr))
							{
								// Do Something for MPEG2 testing.
								pTest->SetHVDService(pHVDService);
								pTest->MP2Decoder(uiClipCounter);
							}
							uiClipCounter++;
						}

						delete pTest;
					}

					if (WaitForSingleObject(m_hExitEvent, 0) == WAIT_OBJECT_0) //leave if receive exit command
					{
						pHVDService->Uninitialize();
						return S_FALSE;
					}

					if (m_dwTestCodecType & TEST_CODEC_H264) // Test H264
					{
						CH264DXVATestProgram* pTest = new CH264DXVATestProgram;
						TCHAR tcConfigFile[256];
						TCHAR InputDir[256];
						TCHAR OutputDir[256];
						UINT uiTotalClipNum = 0;
						UINT uiClipCounter = 1;

						pTest->SetDecodeLogFP(m_fpDecodeLog);

						GetCurrentDirectory(256, tcConfigFile);
						_tcscat_s(tcConfigFile, 256, _T("\\"));
						_tcscat_s(tcConfigFile, 256, _T("H264VDec.ini"));
						pTest->SetConfigFile(tcConfigFile);

						uiTotalClipNum = pTest->GetClipNum();
				
						while(uiTotalClipNum--)
						{
							TCHAR tcClipName[256];
							_stprintf_s(tcClipName, 256, _T("CLIP%d"), uiClipCounter);

							memset(&DecodeConfig, 0, sizeof(DecodeConfig));
							DecodeConfig.dwWidth = pTest->GetClipWidth(tcClipName);
							DecodeConfig.dwHeight = (((pTest->GetClipHeight(tcClipName)+15)>>4)<<4);
							DecodeConfig.dwMode = HVDService::HVD_MODE_H264;
							DecodeConfig.dwFlags = HVDService::HVD_DECODE_MAXSURFACECOUNT | HVDService::HVD_DECODE_MINSURFACECOUNT | HVDService::HVD_DECODE_MODE;
							DecodeConfig.dwLevel = HVDService::HVD_LEVEL_AUTO;
							DecodeConfig.dwMaxSurfaceCount = 12;					
							hr = pHVDService->StartService(&DecodeConfig);
							if (SUCCEEDED(hr))
							{
								// Do Something for H264 testing.
								pTest->SetHVDService(pHVDService);
								pTest->H264Decoder(uiClipCounter);
							}
							uiClipCounter++;
						}

						delete pTest;
					}

					if (WaitForSingleObject(m_hExitEvent, 0) == WAIT_OBJECT_0) //leave if receive exit command
					{
						pHVDService->Uninitialize();
						return S_FALSE;
					}
					if (m_dwTestCodecType & TEST_CODEC_VC1) // Test VC1
					{
						CVC1DXVATestProgram* pTest = new CVC1DXVATestProgram;
						TCHAR tcConfigFile[256];
						TCHAR InputDir[256];
						TCHAR OutputDir[256];
						UINT uiTotalClipNum = 0;
						UINT uiClipCounter = 1;

						pTest->SetDecodeLogFP(m_fpDecodeLog);

						GetCurrentDirectory(256, tcConfigFile);
						_tcscat_s(tcConfigFile, 256, _T("\\"));
						_tcscat_s(tcConfigFile, 256, _T("VC1VDec.ini"));
						pTest->SetConfigFile(tcConfigFile);
						uiTotalClipNum = pTest->GetClipNum();
						
						while(uiTotalClipNum--)
						{
							TCHAR tcClipName[256];
							_stprintf_s(tcClipName, 256, _T("CLIP%d"), uiClipCounter);

							memset(&DecodeConfig, 0, sizeof(DecodeConfig));
							DecodeConfig.dwWidth = pTest->GetClipWidth(tcClipName);
							DecodeConfig.dwHeight = (((pTest->GetClipHeight(tcClipName)+15)>>4)<<4);
							DecodeConfig.dwMode = HVDService::HVD_MODE_VC1;
							DecodeConfig.dwFlags = HVDService::HVD_DECODE_MAXSURFACECOUNT | HVDService::HVD_DECODE_MINSURFACECOUNT | HVDService::HVD_DECODE_MODE;
							DecodeConfig.dwLevel = HVDService::HVD_LEVEL_AUTO;
							DecodeConfig.dwMaxSurfaceCount = 12;

							hr = pHVDService->StartService(&DecodeConfig);
							if (SUCCEEDED(hr))
							{
								// Do Something for VC1 testing.
								pTest->SetHVDService(pHVDService);
								pTest->VC1Decoder(uiClipCounter);
							}
							uiClipCounter++;
						}

						delete pTest;
					}

					pHVDService->Uninitialize();
				}
			}
		}
	}
	return S_OK; //Test Done!
}

HRESULT CHVDTestCenter::LaunchDXVA2Test()
{
	if (!(m_dwTestItem & TEST_ITEM_DXVA2))
		return S_FALSE;

	if (FAILED(OpenDirect3DDeviceEx()))
		return E_FAIL;

	if (FAILED(OpenHVDService()))
		return E_FAIL;

	HRESULT hr = E_FAIL;

	CComPtr<IHVDService> pHVDService;
	m_pfnCreateHVDSerice((DWORD)HVDService::HVD_SERVICE_DXVA2, (void **)&pHVDService);
	CComQIPtr<IHVDServiceDxva2> pServiceDxva2 = pHVDService;
	if (pHVDService && pServiceDxva2)
	{
		typedef struct
		{
			UINT nTestID;
			HVDService::HVD_INIT_FLAGS nInitFlags;
			IUnknown*	pIUnk;
		} DXVA1MappingInfo;

		DXVA1MappingInfo DXVA2MappingTable[] =
		{
			{TEST_ITEM_DXVA2_D3D,				HVDService::HVD_INIT_D3DDEVICE,		(m_pDeviceEx) ? m_pDeviceEx.p : m_pDevice.p},
			{TEST_ITEM_DXVA2_DSHOW,				HVDService::HVD_INIT_MFGETSERVICE,	m_pMFGetService.p},
		};
		HVDService::HVDInitConfig InitConfig;
		HVDService::HVDDecodeConfig DecodeConfig;
		memset(&InitConfig, 0, sizeof(InitConfig));
		memset(&DecodeConfig, 0, sizeof(DecodeConfig));
		UINT nTableSize = sizeof(DXVA2MappingTable) /sizeof(DXVA2MappingTable[0]);

		for (UINT i = 0; i < nTableSize; i++)
		{
			if (DXVA2MappingTable[i].nTestID & m_dwTestItem)
			{
				InitConfig.dwFlags = DXVA2MappingTable[i].nInitFlags;
				InitConfig.pExternalDevice = DXVA2MappingTable[i].pIUnk;
				hr = pHVDService->Initialize(&InitConfig);
				if (SUCCEEDED(hr))
				{
					if (m_dwTestCodecType & TEST_CODEC_MPEG2) // Test MPEG2
					{
						CMP2DXVATestProgram* pTest = new CMP2DXVATestProgram;
						TCHAR tcConfigFile[256];
						TCHAR InputDir[256];
						TCHAR OutputDir[256];
						UINT uiTotalClipNum = 0;
						UINT uiClipCounter = 1;

						pTest->SetDecodeLogFP(m_fpDecodeLog);

						GetCurrentDirectory(256, tcConfigFile);
						_tcscat_s(tcConfigFile, 256, _T("\\"));
						_tcscat_s(tcConfigFile, 256, _T("MP2VDec.ini"));
						pTest->SetConfigFile(tcConfigFile);
						uiTotalClipNum = pTest->GetClipNum();
						
						while(uiTotalClipNum--)
						{
							TCHAR tcClipName[256];
							_stprintf_s(tcClipName, 256, _T("CLIP%d"), uiClipCounter);

							memset(&DecodeConfig, 0, sizeof(DecodeConfig));
							DecodeConfig.dwWidth = pTest->GetClipWidth(tcClipName);
							DecodeConfig.dwHeight = (((pTest->GetClipHeight(tcClipName)+15)>>4)<<4);
							DecodeConfig.dwMode = HVDService::HVD_MODE_MPEG2;
							DecodeConfig.dwFlags = HVDService::HVD_DECODE_MAXSURFACECOUNT | HVDService::HVD_DECODE_MINSURFACECOUNT | HVDService::HVD_DECODE_MODE;
							DecodeConfig.dwLevel = HVDService::HVD_LEVEL_AUTO;
							DecodeConfig.dwMaxSurfaceCount = 12;

							hr = pHVDService->StartService(&DecodeConfig);
							if (SUCCEEDED(hr))
							{
								// Do Something for MPEG2 testing.
								pTest->SetHVDService(pHVDService);
								pTest->MP2Decoder(uiClipCounter);
							}
							uiClipCounter++;
						}

						delete pTest;
					}

					if (WaitForSingleObject(m_hExitEvent, 0) == WAIT_OBJECT_0) //leave if receive exit command
					{
						pHVDService->Uninitialize();
						return S_FALSE;
					}

					if (m_dwTestCodecType & TEST_CODEC_H264) // Test H264
					{
						CH264DXVATestProgram* pTest = new CH264DXVATestProgram;
						TCHAR tcConfigFile[256];
						TCHAR InputDir[256];
						TCHAR OutputDir[256];
						UINT uiTotalClipNum = 0;
						UINT uiClipCounter = 1;

						pTest->SetDecodeLogFP(m_fpDecodeLog);

						GetCurrentDirectory(256, tcConfigFile);
						_tcscat_s(tcConfigFile, 256, _T("\\"));
						_tcscat_s(tcConfigFile, 256, _T("H264VDec.ini"));
						pTest->SetConfigFile(tcConfigFile);
						uiTotalClipNum = pTest->GetClipNum();
						
						while(uiTotalClipNum--)
						{
							TCHAR tcClipName[256];
							_stprintf_s(tcClipName, 256, _T("CLIP%d"), uiClipCounter);

							memset(&DecodeConfig, 0, sizeof(DecodeConfig));
							DecodeConfig.dwWidth = pTest->GetClipWidth(tcClipName);
							DecodeConfig.dwHeight = (((pTest->GetClipHeight(tcClipName)+15)>>4)<<4);
							DecodeConfig.dwMode = HVDService::HVD_MODE_H264;
							DecodeConfig.dwFlags = HVDService::HVD_DECODE_MAXSURFACECOUNT | HVDService::HVD_DECODE_MINSURFACECOUNT | HVDService::HVD_DECODE_MODE;
							DecodeConfig.dwLevel = HVDService::HVD_LEVEL_AUTO;
							DecodeConfig.dwMaxSurfaceCount = 12;
							
							hr = pHVDService->StartService(&DecodeConfig);
							if (SUCCEEDED(hr))
							{
								// Do Something for H264 testing.
								pTest->SetHVDService(pHVDService);
								pTest->H264Decoder(uiClipCounter);
							}
							uiClipCounter++;
						}

						delete pTest;
					}

					if (WaitForSingleObject(m_hExitEvent, 0) == WAIT_OBJECT_0) //leave if receive exit command
					{
						pHVDService->Uninitialize();
						return S_FALSE;
					}
					if (m_dwTestCodecType & TEST_CODEC_VC1) // Test VC1
					{
						CVC1DXVATestProgram* pTest = new CVC1DXVATestProgram;
						TCHAR tcConfigFile[256];
						TCHAR InputDir[256];
						TCHAR OutputDir[256];
						UINT uiTotalClipNum = 0;
						UINT uiClipCounter = 1;

						pTest->SetDecodeLogFP(m_fpDecodeLog);

						GetCurrentDirectory(256, tcConfigFile);
						_tcscat_s(tcConfigFile, 256, _T("\\"));
						_tcscat_s(tcConfigFile, 256, _T("VC1VDec.ini"));
						pTest->SetConfigFile(tcConfigFile);
						uiTotalClipNum = pTest->GetClipNum();
						
						while(uiTotalClipNum--)
						{
							TCHAR tcClipName[256];
							_stprintf_s(tcClipName, 256, _T("CLIP%d"), uiClipCounter);

							memset(&DecodeConfig, 0, sizeof(DecodeConfig));
							DecodeConfig.dwWidth = pTest->GetClipWidth(tcClipName);
							DecodeConfig.dwHeight = (((pTest->GetClipHeight(tcClipName)+15)>>4)<<4);
							DecodeConfig.dwMode = HVDService::HVD_MODE_VC1;
							DecodeConfig.dwFlags = HVDService::HVD_DECODE_MAXSURFACECOUNT | HVDService::HVD_DECODE_MINSURFACECOUNT | HVDService::HVD_DECODE_MODE;
							DecodeConfig.dwLevel = HVDService::HVD_LEVEL_AUTO;
							DecodeConfig.dwMaxSurfaceCount = 12;

							hr = pHVDService->StartService(&DecodeConfig);
							if (SUCCEEDED(hr))
							{
								// Do Something for VC1 testing.
								pTest->SetHVDService(pHVDService);
								pTest->VC1Decoder(uiClipCounter);
							}
							uiClipCounter++;
						}

						delete pTest;
					}

					if (WaitForSingleObject(m_hExitEvent, 0) == WAIT_OBJECT_0) //leave if receive exit command
					{
						pHVDService->Uninitialize();
						return S_FALSE;
					}

					if (m_dwTestCodecType & TEST_CODEC_MPEG4) // Test MPEG4
					{
						CMP4DXVATestProgram* pTest = new CMP4DXVATestProgram;
						TCHAR tcConfigFile[256];
						TCHAR InputDir[256];
						TCHAR OutputDir[256];
						UINT uiTotalClipNum = 0;
						UINT uiClipCounter = 1;

						pTest->SetDecodeLogFP(m_fpDecodeLog);

						GetCurrentDirectory(256, tcConfigFile);
						_tcscat_s(tcConfigFile, 256, _T("\\"));
						_tcscat_s(tcConfigFile, 256, _T("MP4VDec.ini"));
						pTest->SetConfigFile(tcConfigFile);
						uiTotalClipNum = pTest->GetClipNum();

						while(uiTotalClipNum--)
						{
							TCHAR tcClipName[256];
							_stprintf_s(tcClipName, 256, _T("CLIP%d"), uiClipCounter);

							memset(&DecodeConfig, 0, sizeof(DecodeConfig));
							DecodeConfig.dwWidth = pTest->GetClipWidth(tcClipName);
							DecodeConfig.dwHeight = (((pTest->GetClipHeight(tcClipName)+15)>>4)<<4);
							DecodeConfig.dwMode = HVDService::HVD_MODE_MPEG4;
							DecodeConfig.dwFlags = HVDService::HVD_DECODE_MAXSURFACECOUNT | HVDService::HVD_DECODE_MINSURFACECOUNT | HVDService::HVD_DECODE_MODE;
							DecodeConfig.dwLevel = HVDService::HVD_LEVEL_AUTO;
							DecodeConfig.dwMaxSurfaceCount = 12;

							hr = pHVDService->StartService(&DecodeConfig);
							if (SUCCEEDED(hr))
							{
								// Do Something for H264 testing.
								pTest->SetHVDService(pHVDService);
								pTest->MP4Decoder(uiClipCounter);
							}
							uiClipCounter++;
						}

						delete pTest;
					}

					pHVDService->Uninitialize();
				}
			}
		}
	}
	return S_OK;
}

void CHVDTestCenter::HVDTestInProgress()
{
	if (m_pParentWnd)
		::PostMessage(m_pParentWnd->GetSafeHwnd(), WM_APP_HVDTEST_IN_PROGRESS, NULL, NULL);
}

void CHVDTestCenter::HVDTestFinish()
{
	if (m_pParentWnd)
		::PostMessage(m_pParentWnd->GetSafeHwnd(), WM_APP_HVDTEST_FINISH, NULL, NULL);
}

void CHVDTestCenter::HVDTestClose()
{
	if (m_pParentWnd)
		::PostMessage(m_pParentWnd->GetSafeHwnd(), WM_APP_HVDTEST_CLOSE, NULL, NULL);
}