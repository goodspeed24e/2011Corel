#ifndef _IviCallBackFCN_H_
#define _IviCallBackFCN_H_
#include <windows.h>

#define _CALL_CONV __cdecl

#ifndef _VC1_TS_H_DEFINED
class VC1_TS;
#endif

namespace VC1VDecParam
{
	typedef
		HRESULT
		(_CALL_CONV *PFN_VC1_GET_DATA)(
		IN PVOID pvContext,
		OUT const BYTE **ppbOutBuffer,
		OUT DWORD *pcbNumberOfBytesRead,
		OUT BOOL *pbHasPTS,
		OUT VC1_TS *pTimeStamp
		);

	typedef
		HRESULT
		(__cdecl *PFN_VC1_GET_PARAM)(
		IN DWORD dwPropID,
		IN PVOID pvContext,
		OUT LPVOID *ppOutBuffer,
		OUT DWORD *pdwOutBufferLen,
		IN LPVOID pInBuffer,
		IN DWORD dwInBufferLen
		);

	typedef 
		HRESULT (_CALL_CONV *VC1VDec_OnOpenKeyFcn)(
		IN LPVOID pParam,
		IN BYTE reqData[16], 
		IN DWORD dwStrmType, 
		OUT BYTE rspData[16]
	);

};


#endif