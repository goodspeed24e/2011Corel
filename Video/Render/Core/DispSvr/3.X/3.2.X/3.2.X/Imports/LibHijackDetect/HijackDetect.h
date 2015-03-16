#ifndef _HIJACKDETECT_H_
#define _HIJACKDETECT_H_

#include <windows.h>
#include <d3d9.h>
#include "Psapi.h"

#define HJ_TARGET_D3D7	0x1
#define HJ_TARGET_D3D9	0x2
#define HJ_TARGET_DDRAW	0x3

typedef BOOL (WINAPI *LPGETMODULEINFORMATION)(HANDLE, HMODULE, LPMODULEINFO, DWORD);

typedef struct _HJDETDESC
{
	DWORD Targets;
	HWND hWnd;
	LPDWORD pInterface;
} HJDETDESC;

typedef HJDETDESC FAR* LPHJDETDESC;

class CHijackDetect
{
public:
	CHijackDetect();
	virtual ~CHijackDetect();

	HRESULT SetDesc(LPHJDETDESC lpHJDD);
	HRESULT IsHiJacked(BOOL& bHiJacked);
	// To check IsHiJacked every 30 frames
	enum HiJackedCounter
	{
		IsHiJackedCounter = 30,
		IsHiJackedCounterReset = 90
	};

private:
	HRESULT DetectHiJackVT(LPDWORD pDevice, LPDWORD pHJFunc, DWORD dwHJFuncNum, LPBYTE pBase, DWORD dwRange, BOOL& bHiJacked);
	HRESULT DetectHiJackJMP(LPDWORD pVTable, DWORD dwVTableSize, LPBYTE pBase, DWORD dwRange, BOOL& bHiJacked);

	DWORD m_Targets;
	LPDWORD m_pInterface;	// DirectDraw, D3D9,... etc.
	LPDWORD m_pVTable;		// virtual table
	DWORD m_dwVTableSize;
	LPBYTE m_pDLLBase;
	DWORD m_dwDLLSize;

	HMODULE m_hmPSAPI;
	LPGETMODULEINFORMATION m_lpfGetModuleInformation;
	int m_iIsHiJackedCounter;
	HRESULT m_hrLastIsHiJackedStatus;
};

#endif // _UTIL_HIJACKDETECT_H_
