//=============================================================================
//	THIS SOURCE CODE IS PROPRIETARY INFORMATION BELONGING TO INTERVIDEO, INC.
// 	ANY USE INCLUDING BUT NOT LIMITED TO COPYING OF CODE, CONCEPTS, AND/OR
//	ALGORITHMS IS PROHIBITED EXCEPT WITH EXPRESS WRITTEN PERMISSION BY THE 
//	COMPANY.
//
// 	THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
// 	KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// 	IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
// 	PURPOSE.
//
// 	Copyright (c) 2007 InterVideo Corporation.  All Rights Reserved.
//
//-----------------------------------------------------------------------------
// H264VDec.cpp : Defines the entry point for the DLL application.
//

#include <windows.h>
#include <atlbase.h>
#include <atlcom.h>

#include <Video/LibVdoCodecGMO/H264VDecGMO.h>
#include <Imports/Inc/TRService.h>

#if defined (_ENABLE_ISMPGMO_)
#include <Imports/Inc/ISMP/ISMPGMO.h>
#endif
DWORD g_dwTR_DLLCheckSum= 0;
HINSTANCE g_hInstance= 0;

#ifdef _MANAGED
#pragma managed(push, off)
#endif

#if !defined(TRSDK_VER) && defined(TR_BUILD_TARGET) && !defined(_DEBUG)
#pragma comment(lib,"./Imports/Lib/LibTR2005_R.lib")
#pragma message("[H264VDec]...link LibTR2005_R.lib")
#elif !defined(TRSDK_VER) && defined(TR_BUILD_TARGET) && defined(_DEBUG)
#pragma comment(lib,"./Imports/Lib/LibTR2005_D.lib")
#pragma message("[H264VDec]...link LibTR2005_D.lib")
#elif TRSDK_VER >= 1000 && _MSC_VER >= 1400 && defined(TR_BUILD_TARGET) && !defined(_DEBUG)
#pragma comment(lib,"./Imports/Lib/LIBTR.lib")
#pragma message("[H264VDec]...link LIBTR.lib")
#elif TRSDK_VER >= 1000 && _MSC_VER >= 1400 && defined(TR_BUILD_TARGET) && defined(_DEBUG)
#pragma comment(lib,"./Imports/Lib/LIBTRd.lib")
#pragma message("[H264VDec]...link LIBTRd.lib")
#elif TRSDK_VER >= 1000 && _MSC_VER >= 1400 && !defined(TR_BUILD_TARGET)
#pragma comment(lib,"./Imports/Lib/LIBTROff.lib")
#pragma message("[H264VDec]...link LIBTROff.lib")
#endif

#if defined (_ENABLE_ISMPGMO_)
#if !defined(TRSDK_VER) && !defined(_DEBUG)
#pragma comment(lib,"./Imports/Lib/LIBISMP_VC8.lib")
#pragma message("[H264VDec]...link LIBISMP_VC8.lib")
#elif !defined(TRSDK_VER) && defined(_DEBUG)
#pragma comment(lib,"./Imports/Lib/LIBISMP_VC8_D.lib")
#pragma message("[H264VDec]...link LIBISMP_VC8_D.lib")
#elif TRSDK_VER >= 1000 && _MSC_VER >= 1400 && !defined(_DEBUG)
iviTR_TREXE2_SPECIFY_LIB
#pragma comment(lib,"./Imports/Lib/LIBISMP_VC8_TRSDK.lib")
#pragma message("[H264VDec]...link LIBISMP_VC8_TRSDK.lib")
#endif
#endif

EXTERN_C
BOOL WINAPI DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	iviTR_VERIFY_SOFTICE(iviTR_DEFAULT_ACTION);
	iviTR_VERIFY_VTUNE_30(iviTR_DEFAULT_ACTION);

	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		g_hInstance = hModule;
		if (0 == g_dwTR_DLLCheckSum)
		{ 
			bool bRet = false;
			iviTR_GET_IAT_CRC32_BY_MODULE_HANDLE(hModule, g_dwTR_DLLCheckSum, bRet)
				if (!bRet) g_dwTR_DLLCheckSum = 0;
			//iviTR_REGISTER_IMAGESIZE(hModule);
		}
		DisableThreadLibraryCalls(g_hInstance);
	case DLL_PROCESS_DETACH:
		break;
	}
    return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif

EXTERN_C 
void* STDAPICALLTYPE CreateComponent(void)
{
	DWORD dwGetCheckSum=0;
	bool bGetIAT;
	if (0 == g_dwTR_DLLCheckSum) {
		iviTR_GET_IAT_CRC32_BY_MODULE_HANDLE(g_hInstance, g_dwTR_DLLCheckSum, bGetIAT)
			if (!bGetIAT) iviTR_CRASH();
	}
	iviTR_GET_IAT_CRC32_BY_MODULE_HANDLE(g_hInstance, dwGetCheckSum, bGetIAT)
		if (!bGetIAT || (0 != (dwGetCheckSum - g_dwTR_DLLCheckSum)))
			iviTR_CRASH();

	IGMediaObject *pGMediaObject=0;
#if defined(_ENABLE_ISMPGMO_)
	CISMPGMO<CH264VDecGMO> *pGMO= NULL;
	HRESULT hr = CISMPGMO<CH264VDecGMO>::Make(&pGMO);
#else
	CH264VDecGMO *pGMO = NULL;
	HRESULT hr = CH264VDecGMO::Make(&pGMO);
#endif
	if(SUCCEEDED(hr))
	{
		pGMO->QueryInterface(IID_IGMediaObject, reinterpret_cast<void **>(&pGMediaObject));
		if(pGMediaObject!=0)
		{	
			pGMO->Release();
			return pGMediaObject;
		}
	}

	return S_OK;
}
