#ifndef __IVITRCOMM_H_
#define __IVITRCOMM_H_
	#ifdef _WIN32
		#include <windows.h>
	#endif // _WIN32
 	#include "IVITrWinDef.h"
	#ifndef iviTR_DUMP_ERROR_TR_MACRO_ENTRY
		#ifdef TR_TRACE_ENABLE

		#define LOGID_iviTR_VERIFY_SOFTICE								0
		#define LOGID_iviTR_VERIFY_ATTACHED_DEBUGGER					1
		#define LOGID_iviTR_VERIFY_VTUNE_30								2 
		#define LOGID_iviTR_CHECK_MICROSOFT_PIN							3 
		#define LOGID_iviTR_VERIFY_SOFTICE_SIWVIDSTART_A				4 
		#define LOGID_iviTR_VERIFY_SOFTICE_SIWVIDSTART_DD_A				5 
		#define LOGID_iviTR_VERIFY_SOFTICE_SICE_A						6 
		#define LOGID_iviTR_VERIFY_SOFTICE_SICE_DD_A					7 
		#define LOGID_iviTR_VERIFY_SOFTICE_NTICE_A						8 
		#define LOGID_iviTR_VERIFY_SOFTICE_NTICE_DD_A					9 			
		#define LOGID_iviTR_VERIFY_SOFTICE_SIWVID_A						10 
		#define LOGID_iviTR_VERIFY_SOFTICE_SIWVID_DD_A					11 
		#define LOGID_iviTR_VERIFY_SOFTICE_SIWDEBUG_A					12 
		#define LOGID_iviTR_VERIFY_SOFTICE_SIWDEBUG_DD_A				13 
		#define LOGID_iviTR_VERIFY_SOFTICE_SIWVIDSTART_A_INV			14 
		#define LOGID_iviTR_VERIFY_SOFTICE_SIWVIDSTART_DD_A_INV 		15 
		#define LOGID_iviTR_VERIFY_SOFTICE_SICE_A_INV					16 
		#define LOGID_iviTR_VERIFY_SOFTICE_SICE_DD_A_INV				17 
		#define LOGID_iviTR_VERIFY_SOFTICE_NTICE_A_INV					18 
		#define LOGID_iviTR_VERIFY_SOFTICE_NTICE_DD_A_INV				19 
		#define LOGID_iviTR_VERIFY_SOFTICE_SIWVID_A_INV					20 
		#define LOGID_iviTR_VERIFY_SOFTICE_SIWVID_DD_A_INV				21 
		#define LOGID_iviTR_VERIFY_SOFTICE_SIWDEBUG_A_INV				22 
		#define LOGID_iviTR_VERIFY_SOFTICE_SIWDEBUG_DD_A_INV			23 
		#define LOGID_iviTR_VERIFY_SOFTICE_SIWVIDSTART_W				24 
		#define LOGID_iviTR_VERIFY_SOFTICE_SIWVIDSTART_DD_W				25 
		#define LOGID_iviTR_VERIFY_SOFTICE_SICE_W						26 
		#define LOGID_iviTR_VERIFY_SOFTICE_SICE_DD_W					27 
		#define LOGID_iviTR_VERIFY_SOFTICE_NTICE_W						28 
		#define LOGID_iviTR_VERIFY_SOFTICE_NTICE_DD_W					29 
		#define LOGID_iviTR_VERIFY_SOFTICE_SIWVID_W						30 
		#define LOGID_iviTR_VERIFY_SOFTICE_SIWVID_DD_W					31 
		#define LOGID_iviTR_VERIFY_SOFTICE_SIWDEBUG_W					32 
		#define LOGID_iviTR_VERIFY_SOFTICE_SIWDEBUG_DD_W				33 
		#define LOGID_iviTR_VERIFY_SOFTICE_SIWVIDSTART_W_INV			34 
		#define LOGID_iviTR_VERIFY_SOFTICE_SIWVIDSTART_DD_W_INV			35 
		#define LOGID_iviTR_VERIFY_SOFTICE_SICE_W_INV					36 
		#define LOGID_iviTR_VERIFY_SOFTICE_SICE_DD_W_INV				37 
		#define LOGID_iviTR_VERIFY_SOFTICE_NTICE_W_INV					38 
		#define LOGID_iviTR_VERIFY_SOFTICE_NTICE_DD_W_INV				39 
		#define LOGID_iviTR_VERIFY_SOFTICE_SIWVID_W_INV					40 
		#define LOGID_iviTR_VERIFY_SOFTICE_SIWVID_DD_W_INV				41 
		#define LOGID_iviTR_VERIFY_SOFTICE_SIWDEBUG_W_INV				42 
		#define LOGID_iviTR_VERIFY_SOFTICE_SIWDEBUG_DD_W_INV			43 
		#define LOGID_iviTR_VERIFY_SOFTICE_LCREAT						44 
		#define LOGID_iviTR_VERIFY_SOFTICE_LCREAT1						45 
		#define LOGID_iviTR_VERIFY_SOFTICE_LCREAT2						46 
		#define LOGID_iviTR_VERIFY_SOFTICE_LCREAT3						47 
		#define LOGID_iviTR_VERIFY_SOFTICE_LCREAT4						48 
		#define LOGID_iviTR_VERIFY_SOFTICE_LCREAT_INV					49 			
		#define LOGID_iviTR_VERIFY_SOFTICE_LCREAT1_INV					50 
		#define LOGID_iviTR_VERIFY_SOFTICE_LCREAT2_INV					51 
		#define LOGID_iviTR_VERIFY_SOFTICE_LCREAT3_INV					52 
		#define LOGID_iviTR_VERIFY_SOFTICE_LCREAT4_INV					53 
		#define LOGID_iviTR_VERIFY_SOFTICE_LOPEN						54 
		#define LOGID_iviTR_VERIFY_SOFTICE_LOPEN1						55 
		#define LOGID_iviTR_VERIFY_SOFTICE_LOPEN2						56 
		#define LOGID_iviTR_VERIFY_SOFTICE_LOPEN3						57 
		#define LOGID_iviTR_VERIFY_SOFTICE_LOPEN4						58 
		#define LOGID_iviTR_VERIFY_SOFTICE_LOPEN_INV					59 
		#define LOGID_iviTR_VERIFY_SOFTICE_LOPEN1_INV					60 
		#define LOGID_iviTR_VERIFY_SOFTICE_LOPEN2_INV					61 
		#define LOGID_iviTR_VERIFY_SOFTICE_LOPEN3_INV					62 
		#define LOGID_iviTR_VERIFY_SOFTICE_LOPEN4_INV					63 
		#define LOGID_iviTR_VERIFY_SOFTICE_NTICE_SERVICE				64 
		#define LOGID_iviTR_VERIFY_SOFTICE_NTICE_SERVICE_INV			65 
		#define LOGID_iviTR_VERIFY_SYSER_A								66 
		#define LOGID_iviTR_VERIFY_SYSER_DD_A							67 
		#define LOGID_iviTR_VERIFY_SYSER_W								68 
		#define LOGID_iviTR_VERIFY_SYSER_DD_W							69 				
		#define LOGID_iviTR_VERIFY_SYSER_LCREAT							70 
		#define LOGID_iviTR_VERIFY_SYSER_LOPEN							71 
		#define LOGID_iviTR_FIND_INT_3H_UNHANDLEDEXCEPTIONFILTER		72 
		#define LOGID_iviTR_DAEMON_FINDER								73 
		#define LOGID_iviTR_DEBUG_REGISTER_CHECK						74 
		#define LOGID_iviTR_DETECT_TRACER_TRAP_FLAG						75 
		#define LOGID_iviTR_INT1H_CHECK									76 
		#define LOGID_iviTR_API_ISDEBUGGERPRESENT_CHECK					77 
		#define LOGID_iviTR_DEBUGBREAK_CHECK							78 
		#define LOGID_iviTR_SOFTWARE_NUMEGA_SOFTICE_REGISTRY_CHECK		79 
		#define LOGID_iviTR_SOFTWARE_SYSERSOFT_SYSER_REGISTRY_CHECK1	80 
		#define LOGID_iviTR_SOFTWARE_SYSERSOFT_SYSER_REGISTRY_CHECK2	81 
		#define LOGID_iviTR_SOFTWARE_SYSERSOFT_SYSER_REGISTRY_CHECK3	82 
		#define LOGID_iviTR_SOFTWARE_SYSERSOFT_SYSER_REGISTRY_CHECK4	83 
		#define LOGID_iviTR_SOFTWARE_SYSERSOFT_SYSER_REGISTRY_CHECK5	84 
		#define LOGID_iviTR_MEMORY_ADDRESS_BREAKPOINT_DETECT			85 
		#define LOGID_iviTR_CHECK_RETURN_ADDR							86 
		#define LOGID_iviTR_DETECT_CALLER_VALIDATION					87 
		#define LOGID_iviTR_DETECT_VTABLE_HOOK							88 
		#define LOGID_iviTR_DETECT_HOOK_COM								89 			
		#define LOGID_iviTR_GET_IAT_CRC32								90 
		#define LOGID_iviTR_GET_IAT_CRC32_BY_MODULE_HANDLE				91 
		#define LOGID_iviTR_INSIDEVMWARE_CHECK							92
		#define LOGID_iviTR_INSIDEVIRTUALPC_CHECK						93 
		#define LOGID_iviTR_DEBUG_REGISTER_CHECK2						94 
		#define LOGID_iviTR_DEBUG_REGISTER_CLEAR						95 

		#define	LOGID_iviTR_HIDEAPI										499								
		#define LOGID_iviTR_ANTIATTACH									500							
		#define LOGID_iviTR_PEB_IMAGESIZE_CHECK							1000
		#define LOGID_iviTR_PE_IMAGESIZE_CHECK							1001

		#define LOGID_iviTR_CRASH										2000
		#define LOGID_iviTR_EXIT_PROCESS								2001	

		///*!
		// get the address of iviTR_DUMP_ERROR_TR_MACRO_ENTRY
		//
		// Return value - [DWORD] address of current.
		// 
		// Supported platform: All
		//*/        
		DWORD __cdecl trGetEIP();
		///*!
		// dump the TR macro name to "c:\TR_ErrDump.txt".
		//
		// Return value - [bool] true if the operation of dump error log is successful.
		// 
		// -# szMacroName - [IN] the macro name dumped.
		// -# dwAddr - [IN] the address of calling this macro.
		//
		// Supported platform: All
		//*/        
		bool __cdecl trDumpErrorTRMacroEntry(DWORD dwMacroID, DWORD dwAddr);

		///*!
		// dump the TR macro name to "c:\TR_ErrDump.txt" if the 'expression' is true.
		//
		// Return value - None
		// 
		// -# expression - [IN] the expression to determine if dump the TR macro name to file or not.
		// -# TR_Macro_Name - [IN] the TR macro name will be dumpped to file
		//
		// Supported platform:		All Windows
		// Performance overhead:	High
		// Security level:			Very low
		// Usage scope:				Macro scope
		//
		//*/        
		#define iviTR_DUMP_ERROR_TR_MACRO_ENTRY(expression, TR_Macro_Name) \
		{ \
			if (expression) \
			{ \
				\
				DWORD dwRet; \
				dwRet = trGetEIP(); \
				\
				trDumpErrorTRMacroEntry(LOGID_##TR_Macro_Name, dwRet); \
			} \
		}

		#else
			#define iviTR_DUMP_ERROR_TR_MACRO_ENTRY(expression, TR_Macro_Name)
		#endif
	#endif // iviTR_DUMP_ERROR_TR_MACRO_ENTRY


	///*!
	// dump log for checksum region
	// 
	// -# dwChecksumValue - [IN] the checksum dumped.
	// -# dwChecksumSeed - [IN] the seed of checksum dumped.
	//Return Value - [bool] true if the operation of dump log is successful.
	//
	// Supported platform: All
	//
	//*/
	bool __cdecl trDumpChecksumLog(DWORD dwChecksumValue, DWORD dwChecksumSeed);

	///*!
	// dump error log for checksum region
	// 
	// -# dwChecksumValue - [IN] the checksum dumped.
	// -# dwChecksumSeed - [IN] the seed of checksum dumped.
	//Return Value - [bool] true if the operation of dump error log is successful.
	//
	// Supported platform: All
	//
	//*/
	bool __cdecl trDumpChecksumLogErr(DWORD dwChecksumValue, DWORD dwChecksumSeed);

	#ifndef iviTR_DUMP_CHECKSUM_LOG
		#ifdef TR_TRACE_ENABLE
		///*!
		// dump error log for checksum region
		// 
		// -# dwChecksumValue - [IN] the checksum dumped.
		// -# dwChecksumSeed - [IN] the seed of checksum dumped.
		//
		//Return Value - [bool] true if the operation of dump error log is successful.
		//
		// Supported platform:		All Windows
		// Performance overhead:	High
		// Security level:			Very low
		// Usage scope:				Macro scope
		//
		//*/
		#define iviTR_DUMP_CHECKSUM_LOG(dword_checksumvalue, dword_checksumseed) \
			trDumpChecksumLog(dword_checksumvalue, dword_checksumseed)
	#else
			#define iviTR_DUMP_CHECKSUM_LOG(dword_checksumvalue, dword_checksumseed) {}
		#endif
	#endif // iviTR_DUMP_CHECKSUM_LOG

	#ifndef iviTR_DUMP_CHECKSUM_LOG_ERR
		#ifdef TR_TRACE_ENABLE
		///*!
		// dump error log for checksum region
		// 
		// -# dwChecksumValue - [IN] the checksum dumped.
		// -# dwChecksumSeed - [IN] the seed of checksum dumped.
		//
		//Return Value - [bool] true if the operation of dump error log is successful.
		//*/
		#define iviTR_DUMP_CHECKSUM_LOG_ERR(dword_checksumvalue, dword_checksumseed) \
			trDumpChecksumLogErr(dword_checksumvalue, dword_checksumseed)
	#else
			#define iviTR_DUMP_CHECKSUM_LOG_ERR(dword_checksumvalue, dword_checksumseed) {}
		#endif
	#endif // iviTR_DUMP_CHECKSUM_LOG
#endif //__IVITRCOMM_H_
