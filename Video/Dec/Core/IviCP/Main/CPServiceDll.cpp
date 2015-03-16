#include <Windows.h>
#include <ObjBase.h>
#include <tchar.h>
#include "CPServiceWin7.h"
#include "CPServiceAti.h"
#include "CPServiceAtiMp2IDCT.h"
#include "CPServiceNvidia.h"
#include "CPServiceNvidiaMp2IDCT.h"
#include "CPServiceIntel.h"
#include "CPServiceIntelPavp.h"
#include "CPServiceS3.h"


DECLARE_INTERFACE_(ICPClassFactory, IClassFactory)
{
	STDMETHOD_(LONG, GetLockCount)() const PURE;
};

template<typename T>
class TClassFactoryImpl : public ICPClassFactory
{
public:
	TClassFactoryImpl() : m_lRefCount(1), m_lLockCount(0) {}
	virtual ~TClassFactoryImpl() {}

	// IUnknown methods
	STDMETHOD(QueryInterface)(REFIID riid, void ** ppInterface)
	{
		if (IID_IUnknown == riid)
			*ppInterface = static_cast<IUnknown *>(this);
		else if (IID_IClassFactory == riid)
			*ppInterface = static_cast<IClassFactory *>(this);
		else
		{
			*ppInterface = NULL;
			return E_NOINTERFACE;
		}
		static_cast<IUnknown *>(*ppInterface)->AddRef();
		return S_OK;
	}

	STDMETHOD_(ULONG, AddRef())
	{
		return InterlockedIncrement(&m_lRefCount);
	}

	STDMETHOD_(ULONG, Release())
	{
		LONG ref = InterlockedDecrement(&m_lRefCount);
		if (0 == ref)
		{
			delete this;
		}
		return ref;
	}

	// IClassFactory methods
	STDMETHOD(CreateInstance)(IUnknown * punkOuter, REFIID riid, void ** ppInterface)
	{
		HRESULT hr = S_OK;

		// we don't support aggregation
		if (NULL != punkOuter)
			return CLASS_E_NOAGGREGATION;
			
		T *pObj = new T();
		if (NULL == pObj)
			return E_OUTOFMEMORY;

		// now QI for the requested interface.  If this fails, delete the object
		hr = pObj->QueryInterface(riid, ppInterface);
		if (FAILED(hr))
			delete pObj;

		return hr;
	}

	STDMETHOD(LockServer)(BOOL fLock)
	{
		if (fLock)
			InterlockedIncrement(&m_lLockCount);
		else
			InterlockedDecrement(&m_lLockCount);
		return S_OK;
	}

	STDMETHOD_(LONG, GetLockCount)() const
	{
		return m_lLockCount; 
	}

private:
	LONG m_lRefCount, m_lLockCount;
};

struct FactoryData
{
	CLSID clsID;
	TCHAR *pClassName;
	ICPClassFactory *pFactory;
};

// Explicitly create global singletons.
// The order of creating global service is important.
static HINSTANCE g_hInst = NULL;
static TClassFactoryImpl<CoCPServiceWin7> g_CPWin7Factory;
static TClassFactoryImpl<CoCPServiceAti> g_CPAtiFactory;
static TClassFactoryImpl<CoCPServiceAtiMp2IDCT> g_CPAtiMp2IDCTFactory;
static TClassFactoryImpl<CoCPServiceNvidia> g_CPNvidiaFactory;
static TClassFactoryImpl<CoCPServiceNvidiaMp2IDCT> g_CPNvidiaMp2IDCTFactory;
static TClassFactoryImpl<CoCPServiceIntel> g_CPIntelFactory;
static TClassFactoryImpl<CoCPServiceIntelPAVP> g_CPIntelPavpFactory;
static TClassFactoryImpl<CoCPServiceS3> g_CPS3Factory;
static TClassFactoryImpl<CoCPServiceWin7PAVP> g_CPWin7PAVPFactory;

static const FactoryData g_FactoryData[] =
{
	{ CLSID_IVICP_WIN7, TEXT("IVICP Win7"), &g_CPWin7Factory },
	{ CLSID_IVICP_ATI, TEXT("IVICP Ati"), &g_CPAtiFactory },
	{ CLSID_IVICP_ATI_MP2IDCT, TEXT("IVICP AtiMp2IDCT"), &g_CPAtiMp2IDCTFactory },
	{ CLSID_IVICP_NVIDIA, TEXT("IVICP Nvidia"), &g_CPNvidiaFactory },
	{ CLSID_IVICP_NVIDIA_MP2IDCT, TEXT("IVICP NvidiaMp2IDCT"), &g_CPNvidiaMp2IDCTFactory },
	{ CLSID_IVICP_INTEL, TEXT("IVICP Intel"), &g_CPIntelFactory },
	{ CLSID_IVICP_INTEL_PAVP, TEXT("IVICP IntelPavp"), &g_CPIntelPavpFactory },
	{ CLSID_IVICP_S3, TEXT("IVICP S3"), &g_CPS3Factory },
	{ CLSID_IVICP_GPUCP_PAVP, TEXT("IVICP PAVP"), &g_CPWin7PAVPFactory }
};
static const int g_iNumFactory = sizeof(g_FactoryData) / sizeof(FactoryData);

///
/// Delete a key and all of its descendents.
///
/// @param hKeyParent Parent of key to delete
/// @param achKeyChild Key to delete
static LONG recursiveDeleteKey(HKEY hKeyParent, TCHAR* achKeyChild)
{
	// Open the child.
	HKEY hKeyChild;
	LONG lRes = RegOpenKeyEx(hKeyParent, achKeyChild, 0,
		KEY_ALL_ACCESS, &hKeyChild);
	if (lRes != ERROR_SUCCESS)
	{
		return lRes;
	}

	// Enumerate all of the decendents of this child.
	FILETIME time;
	TCHAR achBuffer[256];
	DWORD dwSize = 256;

	while (RegEnumKeyEx(hKeyChild, 0, achBuffer, &dwSize, NULL,
		NULL, NULL, &time) == ERROR_SUCCESS)
	{
		// Delete the decendents of this child.
		lRes = recursiveDeleteKey(hKeyChild, achBuffer);
		if (lRes != ERROR_SUCCESS)
		{
			// Cleanup before exiting.
			RegCloseKey(hKeyChild);
			return lRes;
		}
		dwSize = 256;
	}

	// Close the child.
	RegCloseKey(hKeyChild);

	// Delete this child.
	return RegDeleteKey(hKeyParent, achKeyChild);
}

///
/// Add entry to the registry about CLSID object
///
static HRESULT DispSvrRegisterClass(const CLSID& clsid, TCHAR *achClassName, BOOL bRegister)
{
	HRESULT hr = S_OK;
	TCHAR achModule[MAX_PATH];
	WCHAR wcModule[MAX_PATH];
	WCHAR wcKey[MAX_PATH];
	TCHAR achKey[MAX_PATH];
	TCHAR achThreadingModel[] = TEXT("Both");
	long err = 0;
	LPOLESTR wcCLSID = NULL;
	HKEY hKey = NULL;
	HKEY hSubKey = NULL;

	GetModuleFileName(g_hInst, achModule, sizeof(achModule));

#ifdef UNICODE
	wcscpy_s(wcModule, MAX_PATH, achModule);
#else
	MultiByteToWideChar(CP_ACP, 0, achModule, -1, wcModule, MAX_PATH);
#endif

	hr = StringFromCLSID(clsid, &wcCLSID);
	if (FAILED(hr))
		return E_UNEXPECTED;

	wcscpy_s(wcKey, MAX_PATH, L"CLSID\\");
	wcscat_s(wcKey, MAX_PATH, wcCLSID);

#ifdef UNICODE
	_tcscpy_s(achKey, MAX_PATH, wcKey);
#else
	WideCharToMultiByte(CP_ACP, 0, wcKey, -1, achKey, MAX_PATH, 0, 0);
#endif

	// first, delete wcKey if it exists
	err = recursiveDeleteKey(HKEY_CLASSES_ROOT, achKey);

	if (bRegister)
	{
		// create new key
		err = RegCreateKeyEx(HKEY_CLASSES_ROOT,
			achKey,
			NULL,
			NULL,
			REG_OPTION_NON_VOLATILE,
			KEY_ALL_ACCESS,
			NULL,
			&hKey,
			NULL);
		if (ERROR_SUCCESS == err)
		{
			err = RegSetValueEx(hKey, NULL, 0, REG_SZ, (BYTE *)achClassName,
				(DWORD)sizeof(TCHAR) * (_tcslen(achClassName) + 1));
			err = RegCreateKeyEx(hKey,
				TEXT("InprocServer32"),
				NULL,
				NULL,
				REG_OPTION_NON_VOLATILE,
				KEY_ALL_ACCESS,
				NULL,
				&hSubKey,
				NULL);
			if (ERROR_SUCCESS == err)
			{
				err = RegSetValueEx(hSubKey, NULL, 0, REG_SZ, (BYTE *)achModule,
					(DWORD)sizeof(TCHAR) * (_tcslen(achModule) + 1));
				err = RegSetValueEx(hSubKey, TEXT("ThreadingModel"), 0, REG_SZ, (BYTE *)achThreadingModel,
					(DWORD)sizeof(TCHAR) * (_tcslen(achThreadingModel) + 1));
				RegCloseKey(hSubKey);
			}
			RegCloseKey(hKey);
		}
	}
	CoTaskMemFree(wcCLSID);
	return err == ERROR_SUCCESS ? S_OK : E_FAIL;
}

//////////////////////////////////////////////////////////////////////////
// DLL Entry Point
//////////////////////////////////////////////////////////////////////////
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	if (DLL_PROCESS_ATTACH == dwReason)
	{
		g_hInst = hInstance;
		DisableThreadLibraryCalls(hInstance);
	}
	return (TRUE);
}

STDAPI DllRegisterServer(void)
{
	HRESULT hr = S_OK;

	// now go register components; they are in HKEY_CLASSES_ROOT\CLSID\{uuid}
	for (int i = 0; i < g_iNumFactory; i++)
	{
		hr = DispSvrRegisterClass(g_FactoryData[i].clsID, g_FactoryData[i].pClassName, TRUE);
		if (FAILED(hr))
			break;
	}
	
	return hr;
}

STDAPI DllUnregisterServer(void)
{
	// now go register components; they are in HKEY_CLASSES_ROOT\CLSID\{uuid}
	for (int i = 0; i < g_iNumFactory; i++)
		DispSvrRegisterClass(g_FactoryData[i].clsID, g_FactoryData[i].pClassName, FALSE);
	
	return S_OK;
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID * ppv)
{
	for (int i = 0; i < g_iNumFactory; i++)
		if (rclsid == g_FactoryData[i].clsID)
			return g_FactoryData[i].pFactory->QueryInterface(riid, ppv);

	*ppv = NULL;
	return CLASS_E_CLASSNOTAVAILABLE;
}

STDAPI DllCanUnloadNow(void)
{
	int iLockCount = 0;
	for (int i = 0; i < g_iNumFactory; i++)
		iLockCount += g_FactoryData[i].pFactory->GetLockCount();

	return iLockCount == 0 ? S_OK : S_FALSE;
}