#include "stdafx.h"
#include <multimon.h>
#include "SDKConfigDef.h"
#include "DispSvr.h"
#include "Exports/Inc/EngineProperty.h"
#include "RenderEngine.h"
#include "Wizard.h"
#include "DebugExports\Inc\DispSvrDbgDef.h"

#define CAutoLock _CAutoLock
class _CAutoLock
{
public:
	_CAutoLock(LPCRITICAL_SECTION csObject) : m_csObject(csObject)
	{
		EnterCriticalSection(m_csObject);
	}

	~_CAutoLock()
	{
		LeaveCriticalSection(m_csObject);
	}
private:
	LPCRITICAL_SECTION m_csObject;
};

CWizard::CWizard(LPUNKNOWN pUnk, HRESULT *phr)
        : CUnknown(NAME("Display Server"), pUnk)
        , m_pRenderEngine(NULL)
        , m_bInitialized (FALSE)
		, m_bExtRenderEng (FALSE)
		, m_pDispSvrDebug(NULL)
{
	InitializeCriticalSection(&m_ObjectLock);
	// because CWizard is a singleton in DispSvr global, we add a reference
	// here to prevent others from releasing the instance.
	AddRef();
}

CWizard::~CWizard()
{
	ReleaseDebugObject();
    Terminate();
	DeleteCriticalSection(&m_ObjectLock);
}

///////////////////////// IUnknown /////////////////////////////////////////

STDMETHODIMP CWizard::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
    HRESULT hr = E_NOINTERFACE;
    *ppv = NULL;

    if (riid == IID_IDisplayServer)
    {
        hr = GetInterface((IDisplayServer*) this, ppv);
    }
    else if (riid == IID_IDisplayLock)
    {
        hr = GetInterface((IDisplayLock*) this, ppv);
    }
    else
    {
        hr = CUnknown::NonDelegatingQueryInterface(riid, ppv);
    }
    return hr;
}

///////////////////////// IDisplayServer /////////////////////////////////

STDMETHODIMP CWizard::Initialize(
    DWORD dwFlags,
    HWND hWnd,
    IDisplayRenderEngine *pRenderEngine)
{
    HRESULT hr = S_OK;
    HWND hwndRE = NULL;

	CreateDebugObject();

    if (m_bInitialized)
    {
        DbgMsg("CWizard::Initialize: Wizard is already initialized");
        return S_FALSE;
    }

    if (FALSE == IsWindow(hWnd))
    {
        DbgMsg("CWizard::Initialize: Invalid handle to the video window");
        return E_INVALIDARG;
    }

    CAutoLock Lock(&m_ObjectLock);

    try
    {
        // initialize render engine. We assume that if custom render engine is provided,
        // it is already initialized
        m_pRenderEngine.Release();
        if (pRenderEngine) // user specified customized render engine
        {
            // check that pRenderEngine was initialized and they point to the same window
            hr = pRenderEngine->GetDisplayWindow(&hwndRE, NULL);
            CHECK_HR(hr, DbgMsg("CWizard::Initialize: Failed to get window handler from the provided RenderEngine, hr = 0x%08x", hr));

            hr = (hwndRE != hWnd ? E_FAIL : S_OK);
            CHECK_HR(hr, DbgMsg("CWizard::Initialize: specified render engine points to a different window than wizard"));

            m_pRenderEngine = pRenderEngine;
			m_bExtRenderEng = TRUE;
			if (m_pDispSvrDebug)
			{
				m_pDispSvrDebug->SetRenderEngine(m_pRenderEngine);
			}
        }
        else
        {
			m_pRenderEngine = new CRenderEngine(NULL, NULL);
			CHECK_HR(hr, DbgMsg("CWizard::Initialize: Failed to create DisplayRenderEngine object, error code %08x", hr));
			if (m_pDispSvrDebug)
			{
				m_pDispSvrDebug->SetRenderEngine(m_pRenderEngine);
				m_pDispSvrDebug->SetInitFlags(dwFlags);
				HANDLE hDispSvrDbgMutex = NULL;
				hDispSvrDbgMutex = ::CreateMutex(NULL, FALSE, DispSvrDbgGlobalMutexID);
				if (GetLastError() == ERROR_ALREADY_EXISTS) //DispSvr Debug Tool is enabled and want to modify initflags
				{
					if (IDOK == ::MessageBox(hWnd, _T("Please use DispSvr debug tool to modify init flags before you click OK or just Cancel it"),
						_T("DisplayServer Debug Tool Detected"), MB_OKCANCEL|MB_ICONINFORMATION|MB_SYSTEMMODAL))
					{
						m_pDispSvrDebug->GetInitFlags(&dwFlags);
					}
				}
				::CloseHandle(hDispSvrDbgMutex);
			}
			hr = m_pRenderEngine->Initialize(hWnd, 0, 0, this, dwFlags);
			CHECK_HR(hr, DbgMsg("CWizard::Initialize: failed to initialize default render engine, hr = 0x%08x", hr))
		}

        hr = m_pRenderEngine->SetDisplayServer(this);
        CHECK_HR(hr, DbgMsg("CWizard::Initialize: failed to set Display Server, hr = 0x%08x", hr));

        m_bInitialized = TRUE;
    }
    catch (HRESULT hrFailed)
    {
		if(m_pDispSvrDebug)
			m_pDispSvrDebug->SetRenderEngine(NULL);
		// m_pRenderEngine must be released if initialization failed
		m_pRenderEngine.Release();
		hr = hrFailed;
    }
    return hr;
}

STDMETHODIMP CWizard::Terminate()
{
    HRESULT hr = S_OK;

    try
    {
        // Unadvise render engine
		if (m_pRenderEngine)
		{
			hr = m_pRenderEngine->SetDisplayServer(NULL);
			CHECK_HR(hr, DbgMsg("CWizard::Terminate: failed to unadvise WizardOwner for render engine, hr = 0x%08x", hr));

			if (!m_bExtRenderEng)
			{
				hr = m_pRenderEngine->Terminate();
				CHECK_HR(hr, DbgMsg("CWizard::Terminate: failed to terminate render engine, hr = 0x%08x", hr));

				m_pRenderEngine.Release();
			}
			else
			{
				hr = m_pRenderEngine->StopRenderingThread();
				m_pRenderEngine = NULL;
			}
		}
		//Release RenderEngine ref count in DispSvr Debug
		if (m_pDispSvrDebug)
			m_pDispSvrDebug->SetRenderEngine(NULL);

		m_bInitialized = FALSE;
    }
    catch (HRESULT hrFailed)
    {
        hr = hrFailed;
    }
    return hr;
}

STDMETHODIMP CWizard::BeginDeviceLoss()
{
	return E_NOTIMPL;
}

STDMETHODIMP CWizard::EndDeviceLoss(IUnknown* pDevice)
{
	return E_NOTIMPL;
}

STDMETHODIMP CWizard::GetRenderEngine(IDisplayRenderEngine** ppRenderEngine)
{
    if (!ppRenderEngine)
    {
        DbgMsg("CWizard::GetRenderEngine: first argument is NULL");
        return E_POINTER;
    }

    if (!m_bInitialized)
    {
        DbgMsg("CWizard::GetRenderEngine: Method 'Initialize' was never called");
        return VFW_E_WRONG_STATE;
    }

    *ppRenderEngine = m_pRenderEngine;
    (*ppRenderEngine)->AddRef();
    return S_OK;
}

STDMETHODIMP CWizard::GetMessageWindow(HWND* phwnd)
{
	if (!m_bInitialized)
	{
		DbgMsg("CWizard::GetMessageWindow: Method 'Initialize' was never called");
		return VFW_E_WRONG_STATE;
	}

	return m_pRenderEngine->GetMessageWindow(phwnd);
}

STDMETHODIMP CWizard::SetMessageWindow(HWND hwnd)
{
	if (!m_bInitialized)
	{
		DbgMsg("CWizard::SetMessageWindow: Method 'Initialize' was never called");
		return VFW_E_WRONG_STATE;
	}

	return m_pRenderEngine->SetMessageWindow(hwnd);
}

STDMETHODIMP CWizard::Lock()
{
	EnterCriticalSection(&m_ObjectLock);
    return S_OK;
}

STDMETHODIMP CWizard::Unlock()
{
	LeaveCriticalSection(&m_ObjectLock);
    return S_OK;
}

STDMETHODIMP CWizard::TryLock()
{
	return TryEnterCriticalSection(&m_ObjectLock) == 0 ? E_FAIL : S_OK;
}

HRESULT CWizard::CreateDebugObject()
{
	if (m_pDispSvrDebug)
		return S_FALSE;
#if defined(_DEBUG) || !defined(TR_ENABLE_NEWMACROS)
	m_pDispSvrDebug = new CDispSvrDebug(NULL, NULL);
	if (m_pDispSvrDebug)
	{
		m_pDispSvrDebug->AddIntoROT();
		return S_OK;
	}
	else
		return E_OUTOFMEMORY;
#endif
	return S_FALSE;

}

HRESULT CWizard::ReleaseDebugObject()
{
	if (m_pDispSvrDebug)
	{
		m_pDispSvrDebug->RemoveFromROT();
		SAFE_RELEASE(m_pDispSvrDebug);
		return S_OK;
	}
	else
		return S_FALSE;
}
