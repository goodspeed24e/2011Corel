#ifndef _IVI_DETECTCOMMODULE_H
#define _IVI_DETECTCOMMODULE_H

#include "IVITRComm.h"
#include "IVITRUtil.h"

#ifdef _WIN32

	///*!
	// [Internal function. Do not call it.]
	//
	// call this to register module handle to the register table in SQPlus for detecting modification of embed manifest.\n
	//
	//-# szCLSID - [IN] CLSID of COM module encrypted by XOR with bytePattern.\n
	//-# hHandle - [IN] handle of caller, we need it to get dll name of COM from embed manifest via FindResource(RT_MANIFEST),LoadResource()\n
	//-# bytePattern - [IN] a pattern to encrypt/decrypt code by XOR.\n 
	// Return value - [bool] not use.\n
	//
	// Supported platform:		All Windows
	//
	//*/
	bool __cdecl trRegisterModuleHandle(char *szCLSID, HMODULE hHandle, BYTE byPattern);

	///*!
	// [Internal function. Do not call it.]
	//
	// call this to check if the hook action exists.\n
	//
	//-# szCLSID - [IN] CLSID of com module encrypted by XOR with bytePattern.\n
	//-# szPath - [IN] full path of com module encrypted by XOR with bytePattern.\n
	//-# bytePattern - [IN] a pattern to encrypt/decrypt code by XOR.\n 
	// Return value - [bool] not use.\n
	//
	// Supported platform:		All Windows
	//
	//*/
	bool __cdecl trDetectHookCom(char *szCLSID, char *szPath, BYTE bytePattern);

	///*!
	// [Internal function. Do not call it.]
	//
	// call this to get module handle from registry table in SQPlus.\n
	//
	//-# szCLSID - [IN] CLSID of COM module, it should be decrypted externally.\n
	//-# phHandle - [IN] handle of caller, we need it to get dll name of COM from embed manifest via FindResource(RT_MANIFEST),LoadResource()\n
	//-# ppPosition - [IN] a pointer records the privious position of handle in registry table.\n 
	// Return value - [bool] not use.\n
	//
	// Supported platform:		All Windows
	//
	//*/
	bool __cdecl gtrGetModuleHandle(char *szCLSID, HMODULE *phHandle, void **ppPosition);


	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	///*!
	// call this to register module handle to the register table in SQPlus for detecting modification of embed manifest.\n
	//
	//-# sz_clsid - [IN] CLSID of COM module encrypted by XOR with byte_pattern. 
	//-#	it can be encrypted in advance with the macro of iviTR_ENCRYPT_CONTENT(sz_clsid, byte_pattern, 0) definded later.\n
	//-# h_handle - [IN] handle of caller, we need it to get dll name of COM from embed manifest via FindResource(RT_MANIFEST),LoadResource()\n
	//-# byte_pattern - [IN] a pattern to encrypt/decrypt code by XOR.\n
	// Return value - None\n
	//
	// Supported platform:		All Windows
	// Performance overhead:	Low
	// Security level:			None
	// Usage scope:				Global/process scope
	//
	// Note:\n
	// 
	//*/
	#define iviTR_REGISTER_CALLER_HANDLE(sz_value, h_handle, byte_pattern)\
	{\
		trRegisterModuleHandle(sz_value, h_handle, byte_pattern);\
	}

	///*!
	// detect if the com module has been hooked.\n
	//
	//-# sz_clsid - [IN] CLSID of com module encrypted by XOR with byte_pattern.\n
	//-#	it can be encrypted in advance with the macro of iviTR_ENCRYPT_CONTENT(sz_clsid, byte_pattern, 0) definded later.\n
	//-# sz_path - [IN] full path of com module encrypted by XOR with byte_pattern.\n
	//-#	it can be encrypted in advance with the macro of iviTR_ENCRYPT_CONTENT(sz_path, byte_pattern, 1) definded later.\n
	//-# byte_pattern - [IN] a pattern to encrypt/decrypt code by XOR.\n
	//-# bool_return - [IN] can not be NULL, return check result.\n
	// Return value - None\n
	//
	// Supported platform:		All Windows
	// Performance overhead:	High
	// Security level:			Medium
	// Usage scope:				Macro scope
	//
	// Note:\n
	// 
	// example for hooking detection 
	// Caller side
	// void fnCallerFunc()
	// {
	//		...
	//		byte bytePattern = 133;
	//		iviTR_ENCRYPT_CONTENT(sz_clsid, bytePattern, 0);
	//		...
	//		iviTR_REGISTER_CALLER_HANDLE(sz_clsid, h_handle, bytePattern); //sz_clsid
	//		...
	//		...
	//		iviTR_ENCRYPT_CONTENT(sz_path, bytePattern, 1);
	//		...
	//		iviTR_DETECT_HOOK_COM(sz_clsid, sz_path, bytePattern, bool_return); // befor calling CoCreateInstance
	//		if (bool_return)
	//			iviTR_CRASH();
	//		...
	//		CoCreateInstance(...);
	//		...
	// }
	//
	// Callee side
	// void fnCalleeFunc()
	// {
	//		...
	//		byte bytePattern = 200;
	//		iviTR_ENCRYPT_CONTENT(sz_clsid, bytePattern, 0);
	//		...
	//		iviTR_ENCRYPT_CONTENT(sz_path, bytePattern, 1);
	//		...
	//		iviTR_DETECT_HOOK_COM(sz_clsid, sz_path, bytePattern, bool_return);
	//		if (bool_return)
	//			iviTR_CRASH();
	//		...
	// }
	//
	//*/
	#define iviTR_DETECT_HOOK_COM(sz_clsid, sz_path, byte_pattern, bool_return) \
	{\
		bool_return = trDetectHookCom(sz_clsid, sz_path, byte_pattern);\
		iviTR_DUMP_ERROR_TR_MACRO_ENTRY(bool_return, iviTR_DETECT_HOOK_COM) \
	}

#elif defined(__linux__)
	#define iviTR_DETECT_HOOK_COM(sz_clsid, sz_path, byte_pattern, bool_return) {bool_return = false;}
	#define iviTR_REGISTER_CALLER_HANDLE(sz_value, h_handle, byte_pattern)
#endif // _WIN32

#endif
