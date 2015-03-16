#ifndef _UTIL_GPU_H_
#define _UTIL_GPU_H_

#if defined(_WIN32) && (!defined(_WIN32_WCE) || !defined(UNDER_CE) )
#include <windows.h>
#include <multimon.h>
#endif

// class to 
class CDisplayDevItem
{
public:
	enum {kMaxDevName = 64};
	TCHAR	m_DeviceName[kMaxDevName]; // will truncate if necessary
	DWORD	m_VendorId;
	DWORD	m_DeviceId;
	char	m_Description[512];	
	char	m_Driver[512];	
};

class CUtilGPU
{
public:
	static HRESULT WINAPI GetDisplayDeviceType(LPCTSTR szDevName, CDisplayDevItem& dev);
	static HRESULT WINAPI QueryVideoMemorySize(DWORD dwCaps, DWORD* pdwTotalMem, DWORD* pdwFreeMem);
	static HRESULT WINAPI AddDisplayDevice(const TCHAR* pDevName, const DWORD dwVendorId, DWORD dwDeviceId, char *szDescription, char *szDriver);

private:
	enum {kMaxDevices = 10};
	static int EnumerateDisplayDevices();
	static volatile BOOL m_bDeviceTableValid;
	static BOOL m_bCloneMode;
	static long m_NumDevTableItems;
	static CDisplayDevItem	m_DeviceTable[kMaxDevices];
};

HRESULT GetDeviceIdentifierEx(void*,void*,DWORD dwFlags);

#endif //_UTIL_GPU_H_
