#include "stdafx.h"
#include "DispSvrDbg.h"


#include "Wizard.h"
#include "MixerControl.h"
#include "CompositeDO.h"
#include "ServerStateDO.h"
#include "ShineDO.h"
#include "RenderClock.h"
#include "ServerStateEventSink.h"
#include "VideoSourceDO.h"
#include "RenderEngine.h"

using namespace DispSvr;

class IDispSvrClassFactory : public IClassFactory
{
public:
	STDMETHOD_(LONG, GetLockCount)() const = 0;
};

template<typename T>
class TClassFactoryImpl : public IDispSvrClassFactory
{
public:
	TClassFactoryImpl() : m_RefCount(1), m_LockCount(0)	{}
	virtual ~TClassFactoryImpl() {}

	// IUnknown methods
	STDMETHOD(QueryInterface)(REFIID riid, void ** ppv)
	{
		if (IID_IUnknown == riid)
			*ppv = static_cast<IUnknown *>(this);
		else if (IID_IClassFactory == riid)
			*ppv = static_cast<IClassFactory *>(this);
		else
		{
			*ppv = NULL;
			return E_NOINTERFACE;
		}
		static_cast<IUnknown *>(*ppv)->AddRef();
		return S_OK;
	}

	STDMETHOD_(ULONG, AddRef())
	{
		return InterlockedIncrement(&m_RefCount);
	}

	STDMETHOD_(ULONG, Release())
	{
		LONG ref = InterlockedDecrement(&m_RefCount);
		if (0 == ref)
		{
			delete this;
		}
		return ref;
	}

	// IClassFactory methods
	STDMETHOD(CreateInstance)(IUnknown * punkOuter, REFIID riid, void ** ppvObject)
	{
		HRESULT hr = S_OK;

		// we don't support aggregation
		if (NULL != punkOuter)
			return CLASS_E_NOAGGREGATION;

		CHECK_POINTER(ppvObject);

		*ppvObject = NULL;
		T *pObj = new T(punkOuter, &hr);
		if (NULL == pObj)
			return E_OUTOFMEMORY;

		// now QI for the requested interface.  If this fails, delete the object
		hr = pObj->QueryInterface(riid, ppvObject);
		if (FAILED(hr))
			delete pObj;

		return hr;
	}

	STDMETHOD(LockServer)(BOOL fLock)
	{
		if (fLock)
			InterlockedIncrement(&m_LockCount);
		else
			InterlockedDecrement(&m_LockCount);
		return S_OK;
	}

	STDMETHOD_(LONG, GetLockCount)() const { return m_LockCount; }

private:
	LONG m_RefCount, m_LockCount;
};

template<typename T>
class TSingletonClassFactoryImpl : public TClassFactoryImpl<T>
{
public:
	STDMETHOD(CreateInstance)(IUnknown * punkOuter, REFIID riid, void ** ppvObject)
	{
		HRESULT hr = S_OK;

		// we don't support aggregation
		if (NULL != punkOuter)
			return CLASS_E_NOAGGREGATION;

		CHECK_POINTER(ppvObject);

		*ppvObject = NULL;
		hr = T::GetInstance()->QueryInterface(riid, ppvObject);
		return hr;
	}
};

struct FactoryData
{
	CLSID clsID;
	TCHAR *pClassName;
	IDispSvrClassFactory *pFactory;
};

// Explicitly create global singletons.
// The order of creating global service is important.
static HINSTANCE g_hInst = NULL;
static CRegistryService s_RegSrv;
static CDynLibManager s_DynLibMgr;
static CResourceManager s_ResourceMgr;
static CRenderClock s_RenderClock;
static CServerStateEventSink s_ServerStateEventSink;
static CWizard s_Wizard(NULL, NULL);
static TSingletonClassFactoryImpl<CWizard> g_WizardFactory;
static TClassFactoryImpl<CRenderEngine> g_RenderEngineFactory;
static TClassFactoryImpl<CCompositeDisplayObject> g_CompositeDisplayObjectFactory;
static TClassFactoryImpl<CServerStateDO> g_ServerStateDOFactory;
static TClassFactoryImpl<CShineDO> g_ShineDOFactory;
static TClassFactoryImpl<CVideoMixer> g_MixerControlFactory;
static TClassFactoryImpl<CVideoRootDO> g_VideoRootDOFactory;
static TClassFactoryImpl<CVideoSourceDO> g_VideoSourceDOFactory;

static const FactoryData g_FactoryData[] =
{
	{ CLSID_DisplayServer, TEXT("Displayer Server"), &g_WizardFactory },
	{ CLSID_DisplayRenderEngine, TEXT("Display Render Engine"), &g_RenderEngineFactory },
	{ CLSID_CompositeDisplayObject, TEXT("Composite DO"), &g_CompositeDisplayObjectFactory },
	{ CLSID_ServerStateDisplayObject, TEXT("Display Server State DO"), &g_ServerStateDOFactory },
	{ CLSID_ShineDisplayObject, TEXT("Shine DO"), &g_ShineDOFactory },
	{ CLSID_DisplayVideoMixer, TEXT("Display Mixer Control"), &g_MixerControlFactory},
	{ CLSID_VideoRootDisplayObject, TEXT("VideoRoot DO"), &g_VideoRootDOFactory},
	{ CLSID_VideoSourceDisplayObject, TEXT("VideoSource DO"), &g_VideoSourceDOFactory},
};
static const int g_iNumFactory = sizeof(g_FactoryData) / sizeof(FactoryData);

int DispSvr::GetRegInt(const TCHAR *pKey, int iDefault)
{
#ifdef _DEBUG
	HKEY hKey;
	long hr;
	int iRet;
	DWORD dwType, dwLen = sizeof(iRet);

	hr = RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("software\\corel\\dvd10\\"), 0, KEY_READ, &hKey);
	if(ERROR_SUCCESS == hr)
	{
		hr = RegQueryValueEx(hKey, pKey, 0L, &dwType, (LPBYTE)&iRet, &dwLen);
		RegCloseKey(hKey);
		if (hr == ERROR_SUCCESS)
			  return iRet;
	}
#endif
	return iDefault;
}

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

	// For BD+ DiscoveryRAM Tags, BD+ Content Code would check the pattern at runtime.
	static unsigned char DATA_TAG_13[] =
	{0x8A, 0x72, 0x6E, 0xBA, 0x3D, 0x44, 0x37, 0x7A, 0xA8, 0x6E, 0x90, 0xA3, 0x96, 0x32, 0xE7, 0xA3};

    GetModuleFileName(g_hInst, achModule, sizeof(achModule));

#ifdef UNICODE
    wcscpy_s(wcModule, MAX_PATH, achModule);
#else
    MultiByteToWideChar(CP_ACP, 0, achModule, -1, wcModule, MAX_PATH);
#endif

    hr = StringFromCLSID(clsid, &wcCLSID);
	if (FAILED(hr))
	{
		DATA_TAG_13[3] += DATA_TAG_13[1];
		return E_UNEXPECTED;
	}

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
                                sizeof(TCHAR) * (_tcslen(achClassName) + 1));
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
                                    sizeof(TCHAR) * (_tcslen(achModule) + 1));
                err = RegSetValueEx(hSubKey, TEXT("ThreadingModel"), 0, REG_SZ, (BYTE *)achThreadingModel,
                                    sizeof(TCHAR) * (_tcslen(achThreadingModel) + 1));
                RegCloseKey(hSubKey);
            }
            RegCloseKey(hKey);
        }
    }
    CoTaskMemFree(wcCLSID);
	return err == ERROR_SUCCESS ? S_OK : SELFREG_E_TYPELIB;
}

static HRESULT DispSvrDbgRegisterTypeLib(BOOL bRegister)
{
	ITypeLib *pTLB;
	HRESULT hr = E_FAIL;
	WCHAR wcModule[MAX_PATH];
	WCHAR wcTlbName[MAX_PATH];
	if(!GetModuleFileNameW(g_hInst, wcModule, MAX_PATH))
		return hr;

	swprintf_s(wcTlbName, MAX_PATH, L"%s\\1", wcModule); //DispSvrdbg.tlb uses ID 1 in DispSvr.rc file

	hr=LoadTypeLib(wcTlbName,&pTLB);
	if(SUCCEEDED(hr))
	{
		ITypeInfo *pTypeInfo;
		hr = pTLB->GetTypeInfo( 0, &pTypeInfo);
		if (SUCCEEDED(hr) && pTypeInfo)
		{
			TYPEATTR *pTypeAttr = NULL;
			GUID libID = LIBID_DispSvrDbgLib;
			WORD wVerMajor = 1;
			WORD wVerMinor = 0;
			LCID licd = 0;
			SYSKIND syskind = SYS_WIN32;
			//hr = pTypeInfo->GetTypeAttr(&pTypeAttr); //GetTypeAttr cannot get correct information
			//if (SUCCEEDED(hr))
			//{
			//	libID = pTypeAttr->guid;
			//	wVerMajor = pTypeAttr->wMajorVerNum;
			//	wVerMinor = pTypeAttr->wMinorVerNum;
			//	licd = pTypeAttr->lcid;
			//	pTypeInfo->ReleaseTypeAttr(pTypeAttr);
			//}
			hr = UnRegisterTypeLib(libID, wVerMajor, wVerMinor, licd, syskind);
		}
		if (bRegister)
		{
			hr=RegisterTypeLib(pTLB, wcModule, NULL);
		}
		pTLB->Release();
	}

	return hr;

}

void DbgMsgInfo(char* szMessage, ...)
{
    char szFullMessage[MAX_PATH];
    char szFormatMessage[MAX_PATH];

    // format message
    va_list ap;
    va_start(ap, szMessage);
    _vsnprintf_s(szFormatMessage, MAX_PATH, _TRUNCATE, szMessage, ap);
    va_end(ap);
    strncat_s(szFormatMessage, MAX_PATH,"\n", MAX_PATH);
	strcpy_s(szFullMessage, MAX_PATH, "DispSvr: ");
    strcat_s(szFullMessage, MAX_PATH, szFormatMessage);
    OutputDebugStringA(szFullMessage);
}

//////////////////////////////////////////////////////////////////////////
// DLL Entry Point
//////////////////////////////////////////////////////////////////////////
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	if (DLL_PROCESS_ATTACH == dwReason)
	{
        DisableThreadLibraryCalls(hInstance);
		g_hInst = hInstance;
	}
	return (TRUE);
}

//
// stub entry points
//

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

	DispSvrDbgRegisterTypeLib(TRUE);
	return hr;
}

STDAPI DllUnregisterServer(void)
{
	// now go register components; they are in HKEY_CLASSES_ROOT\CLSID\{uuid}
	for (int i = 0; i < g_iNumFactory; i++)
		DispSvrRegisterClass(g_FactoryData[i].clsID, g_FactoryData[i].pClassName, FALSE);

	DispSvrDbgRegisterTypeLib(FALSE);
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
	int iObjCount = 0;
	iObjCount = CWizard::GetObjectCount() +
				CRenderEngine::GetObjectCount() +
				CCompositeDisplayObject::GetObjectCount() +
				CServerStateDO::GetObjectCount() +
				CShineDO::GetObjectCount() +
				CVideoMixer::GetObjectCount() +
				CVideoRootDO::GetObjectCount() +
				CVideoSourceDO::GetObjectCount();

	return iObjCount == 0  ? S_OK : S_FALSE;
}
