
#define COMPILE_MULTIMON_STUBS	//required in only one file, should be here for multimon!
#define MAXIMUM_DRIVERSION_LENGTH 24

#include <windows.h>
#include <tchar.h>
#include <d3d9.h>
#include <atlbase.h>
#include <ddraw.h>
#include <multimon.h>
#include "UtilGPU.h"
#include "GPUID.h"

#define DDGET32BITDRIVERNAME	11		// get a 32-bit driver name
#define DD_VERSION				0x00000200l
#define DEVICE_NAME_LENGTH		256

typedef BOOL (WINAPI * LPFNENUMDISPLAYSETTINGSEX)(LPCSTR lpszDeviceName, DWORD iModeNum, LPDEVMODE lpDevMode, DWORD dwflags);

const TCHAR* kSynchName = _T("___ENUMDEVSYNCH___");
const TCHAR* kScrOriName = _T("___IVI_SCREEN_ORIENTATION___");
volatile BOOL	CUtilGPU::m_bDeviceTableValid;
long			CUtilGPU::m_NumDevTableItems;
CDisplayDevItem	CUtilGPU::m_DeviceTable[kMaxDevices];
BOOL			CUtilGPU::m_bCloneMode;

HRESULT GetDeviceIdentifierEx(void* pp,void* pp2,DWORD dwFlags)
{
	// First check if we already have this info in registry
	//
	LPDIRECTDRAW4 dd4p= (LPDIRECTDRAW4) pp;
	LPDDDEVICEIDENTIFIER pddDev= (LPDDDEVICEIDENTIFIER) pp2;
	if ( pddDev )
		memset(pddDev, 0, sizeof(DDDEVICEIDENTIFIER));

	HRESULT hr= DD_OK;
	DISPLAY_DEVICE DisplayDevice; 
	const int szDriverName= sizeof(DisplayDevice.DeviceString);
	TCHAR *strDriverName, *strDeviceName;
	TCHAR buffer[MAX_DDDEVICEID_STRING];
	int vendorID = 0;
	int deviceID = 0;
	int numDevices= 0;
	int numAttached= 0;
	int numPrimary= 0;

	strDriverName = new TCHAR[szDriverName];
	strDeviceName = new TCHAR[szDriverName];
	if(!strDeviceName || !strDriverName)
	{
		hr = E_OUTOFMEMORY; 
		goto end;
	}
	ZeroMemory(&DisplayDevice,sizeof(DisplayDevice));
	DisplayDevice.cb = sizeof(DisplayDevice);
	while(EnumDisplayDevices(NULL, numDevices, &DisplayDevice, 0))
	{
		numDevices++;
		if(DisplayDevice.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP)
		{
			numAttached++;
			if(DisplayDevice.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE)
			{
				_tcscpy_s(strDriverName, szDriverName, DisplayDevice.DeviceString);
				_tcscpy_s(strDeviceName, szDriverName, DisplayDevice.DeviceName);
				numPrimary++;
			}
		}
	}

	if(numAttached > numPrimary)
	{	// This is a clearly multiple display adaptor case, no need to check registry
		hr = dd4p->GetDeviceIdentifier(pddDev, dwFlags);
		goto end;
	}

	if(_tcscmp(buffer, strDriverName) || !vendorID || !deviceID)
	{
		HRESULT hr= dd4p->GetDeviceIdentifier(pddDev, dwFlags);
		if(hr == DD_OK)
		{
			hr = DD_OK;
			goto end;
		}
	}

	if(hr == DD_OK)
	{
		USES_CONVERSION;
		pddDev->dwVendorId= vendorID;
		pddDev->dwDeviceId= deviceID;
		strcpy_s(pddDev->szDriver, MAX_DDDEVICEID_STRING, T2CA(strDeviceName));
		strcpy_s(pddDev->szDescription, MAX_DDDEVICEID_STRING, T2CA(strDriverName));
	}
end:
	if(strDriverName)
		delete[] strDriverName;
	if(strDeviceName)
		delete[] strDeviceName;

	return hr;
}



BOOL WINAPI DDEnumDevCallback(GUID FAR *lpGUID, LPSTR lpDriverDescription, LPSTR lpDriverName, LPVOID lpContext, HMONITOR hm)
{
	LPDIRECTDRAW ddp;
	LPDIRECTDRAW4 dd4p;
	DDDEVICEIDENTIFIER ddDev;
	BOOL ret = FALSE;

	if (DirectDrawCreate(lpGUID,&ddp,0) == DD_OK)
	{
		if(ddp->QueryInterface(IID_IDirectDraw4,(void **)&dd4p) == S_OK)
		{
			ZeroMemory(&ddDev,sizeof(ddDev));
			if(GetDeviceIdentifierEx(dd4p,&ddDev, DDGDI_GETHOSTIDENTIFIER) == DD_OK)
			{
				// add to table
				USES_CONVERSION;
				CUtilGPU::AddDisplayDevice(A2CT(lpDriverName), ddDev.dwVendorId, ddDev.dwDeviceId, ddDev.szDescription, ddDev.szDriver);
				ret = TRUE;
			}
			dd4p->Release();
		}
		ddp->Release();
	}

	return ret;
}

HRESULT	CUtilGPU::AddDisplayDevice(const TCHAR* pDevName, const DWORD dwVendorId, DWORD dwDeviceId, char *szDescription, char *szDriver)
{
	// note that UNICODE won't work here because of the input string is CHAR
	_tcsncpy_s(m_DeviceTable[m_NumDevTableItems].m_DeviceName, CDisplayDevItem::kMaxDevName, pDevName, _TRUNCATE);

	m_DeviceTable[m_NumDevTableItems].m_DeviceName[CDisplayDevItem::kMaxDevName-1] = '\0';
	m_DeviceTable[m_NumDevTableItems].m_VendorId = dwVendorId;
	m_DeviceTable[m_NumDevTableItems].m_DeviceId = dwDeviceId;
	strncpy_s(m_DeviceTable[m_NumDevTableItems].m_Description, sizeof(m_DeviceTable[m_NumDevTableItems].m_Description), szDescription, _TRUNCATE);
	strncpy_s(m_DeviceTable[m_NumDevTableItems].m_Driver, sizeof(m_DeviceTable[m_NumDevTableItems].m_Driver), szDriver, _TRUNCATE);
	InterlockedIncrement(&m_NumDevTableItems);

	return S_OK;
}

//The second part of this func is to detect clone mode.
//Below is some VGAs' behavior
//
//					Clone Mode			|		Extended Mode
//										|
//ATI:		nAttachedMonitors = 1		|	nAttachedMonitors = 2
//			nMaxAttachedDevices = N(>1)	|	nMaxAttachedDevices = 1
//										|
//NVIDIA:	nAttachedMonitors = 1		|	nAttachedMonitors = 2
//			nMaxAttachedDevices = N(>1)	|	nMaxAttachedDevices = N(>1)
//										|
//Intel:	nAttachedMonitors = 1		|	nAttachedMonitors = 1
//			nMaxAttachedDevices = 1		|	nMaxAttachedDevices = 1

int CUtilGPU::EnumerateDisplayDevices()
{
	HANDLE hSynch;
	
	if(m_bDeviceTableValid)
		return 0;

	// only permit one thread to enter here.
	if((hSynch = CreateMutex(NULL, FALSE, kSynchName))==0)
		return -1;

	WaitForSingleObject(hSynch, INFINITE);
	if(!m_bDeviceTableValid)
	{	// make sure that init has not been done while we were waiting for synch
		HINSTANCE hLib;
		LPDIRECTDRAWENUMERATEEXA lpDDEnumEx;
		int nDevices, nMonitors, nAttachedMonitors, nAttachedDevices, nMaxAttachedDevices;
		DISPLAY_DEVICE DisplayDevice;

		struct
		{
			DISPLAY_DEVICE DisplayDevice; 
			TCHAR DeviceID[128];
			TCHAR DeviceKey[128];
		} DisplayDevice2;
		
		m_bCloneMode = FALSE;
		m_NumDevTableItems = 0;
		ZeroMemory(m_DeviceTable, sizeof(m_DeviceTable));
		ZeroMemory(&DisplayDevice,sizeof(DisplayDevice));
		ZeroMemory(&DisplayDevice2,sizeof(DisplayDevice2));
		DisplayDevice.cb = sizeof(DisplayDevice);
		DisplayDevice2.DisplayDevice.cb = sizeof(DisplayDevice2);
		hLib=LoadLibrary(_T("ddraw.dll"));
		if(hLib)
		{
			if(lpDDEnumEx = (LPDIRECTDRAWENUMERATEEXA) GetProcAddress(hLib,"DirectDrawEnumerateExA"))
			{
				lpDDEnumEx(DDEnumDevCallback, NULL, DDENUM_ATTACHEDSECONDARYDEVICES);
				m_bDeviceTableValid = TRUE;
			}
			FreeLibrary(hLib);
		}

		for(nMaxAttachedDevices=nMonitors=nAttachedMonitors=0;
			EnumDisplayDevices(NULL, nMonitors, &DisplayDevice, 0);
			nMonitors++)
		{
			if(DisplayDevice.StateFlags&DISPLAY_DEVICE_ATTACHED_TO_DESKTOP)
			{
				for(nDevices=nAttachedDevices=0;
					EnumDisplayDevices(DisplayDevice.DeviceName, nDevices, (DISPLAY_DEVICE*)&DisplayDevice2, 0);
					nDevices++)
				{
					if(DISPLAY_DEVICE_ATTACHED_TO_DESKTOP & DisplayDevice2.DisplayDevice.StateFlags)
					{
						nAttachedDevices++;
#if 0
 						for (int i = sizeof(m_scasz16_9Monitor) / sizeof(m_scasz16_9Monitor[0]); i--;)
 							if(_tcsstr(DisplayDevice2.DeviceID, m_scasz16_9Monitor[i]))
 								m_b16by9MonitorConnected = TRUE;
#endif
					}
				}
				nMaxAttachedDevices = (nMaxAttachedDevices<nAttachedDevices)?nAttachedDevices:nMaxAttachedDevices;
				nAttachedMonitors++;
			}
		}

		if(nAttachedMonitors==1 && nMaxAttachedDevices>1)
			m_bCloneMode = TRUE;
		m_bDeviceTableValid = TRUE;
	}

	ReleaseMutex(hSynch);
	CloseHandle(hSynch);
	return m_bDeviceTableValid ? 0 : -1;
}

HRESULT CUtilGPU::GetDisplayDeviceType(LPCTSTR szDevName, CDisplayDevItem& dev)
{

	dev.m_DeviceName[0] = '\0';
	dev.m_DeviceId = 0;
	dev.m_VendorId = 0;
	
	if(EnumerateDisplayDevices() < 0)
	{
		return E_FAIL;
	}

	if(szDevName == NULL || szDevName[0] == '\0')
	{
		dev = m_DeviceTable[0];
		return S_OK;
	}

	for(int i = 0; i < m_NumDevTableItems; i++)
	{
		// note that UNICODE won't work here because of the input string is CHAR
		if(_tcsncicmp(m_DeviceTable[i].m_DeviceName, szDevName, CDisplayDevItem::kMaxDevName) == 0)
		{
			dev = m_DeviceTable[i];
			return S_OK;
		}
	}

	// test for single monitor and szDevName == "DISPLAY1" or dual monitors and szDevName=="DISPLAY2"
	if(m_NumDevTableItems == 1 && _tcsncicmp(_T("\\\\.\\DISPLAY1"), szDevName, CDisplayDevItem::kMaxDevName) == 0)
	{
		dev = m_DeviceTable[0];
		return S_OK;
	}
	else if(m_NumDevTableItems >= 2 && _tcsncicmp(_T("\\\\.\\DISPLAY2"), szDevName, CDisplayDevItem::kMaxDevName) == 0)
	{
		dev = m_DeviceTable[1];
		return S_OK;
	}

	return E_FAIL;

}


HRESULT CUtilGPU::QueryVideoMemorySize(DWORD dwCaps, DWORD* pdwTotalMem, DWORD* pdwFreeMem)
{
	LPDIRECTDRAW lpdd = NULL;
	HRESULT hr = DirectDrawCreate(NULL, &lpdd, NULL);
	if(SUCCEEDED(hr))
	{
		LPDIRECTDRAW7 lpdd7 = NULL;
		hr = lpdd->QueryInterface(IID_IDirectDraw7, (LPVOID*)&lpdd7);
		if(SUCCEEDED(hr))
		{
			DDSCAPS2 ddsCaps2;
			DWORD dwTotal=0, dwFree=0;
			// Initialize the structure.
			ZeroMemory(&ddsCaps2, sizeof(ddsCaps2));
			ddsCaps2.dwCaps = dwCaps;
			hr = lpdd7->GetAvailableVidMem(&ddsCaps2, &dwTotal, &dwFree);
			if(SUCCEEDED(hr))
			{
				if(pdwTotalMem)
					*pdwTotalMem =(dwTotal>>20); //rounded to MB
				if(pdwFreeMem)
					*pdwFreeMem = (dwFree>>20); //rounded to MB
			}
			lpdd7->Release();
		}
		lpdd->Release();
	}
	return hr;
}
