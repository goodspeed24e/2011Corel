#include <windows.h>
#include <atlbase.h>
#include <boost/test/unit_test.hpp>
#include "../DispSvr.h"
#include "../Exports/Inc/VideoMixer.h"
#include "../Exports/Inc/VideoPresenter.h"

using namespace std;
using namespace boost::unit_test;
using namespace DispSvr;

static HWND g_hwndDevice = 0;
struct CoInit
{
	CoInit() { CoInitialize(NULL); }
	~CoInit() { CoUninitialize(); }
};

void TestCoCreateInstance()
{
	HRESULT hr = S_OK;
	CoInit co;
	
	CComPtr<IDisplayServer>	pWizard;
	hr = pWizard.CoCreateInstance(CLSID_DisplayServer);
	BOOST_CHECK_MESSAGE(SUCCEEDED(hr) && pWizard, "CoCreateInstance CLSID_DisplayServer failed hr=0x" << hex << hr);

	CComPtr<IDisplayObject> pCompositeDisplayObject;
	hr = pCompositeDisplayObject.CoCreateInstance(CLSID_CompositeDisplayObject);
	BOOST_CHECK_MESSAGE(SUCCEEDED(hr) && pCompositeDisplayObject, "CoCreateInstance CLSID_CompositeDisplayObject failed hr=0x" << hex << hr);

	CComPtr<IDisplayObject> pVideoRootDisplayObject;
	hr = pVideoRootDisplayObject.CoCreateInstance(CLSID_VideoRootDisplayObject);
	BOOST_CHECK_MESSAGE(SUCCEEDED(hr) && pVideoRootDisplayObject, "CoCreateInstance CLSID_VideoRootDisplayObject failed hr=0x" << hex << hr);

	CComPtr<IDisplayVideoMixer> pDisplayVideoMixer;
	hr = pDisplayVideoMixer.CoCreateInstance(CLSID_DisplayVideoMixer);
	BOOST_CHECK_MESSAGE(SUCCEEDED(hr) && pDisplayVideoMixer, "CoCreateInstance CLSID_DisplayVideoMixer failed hr=0x" << hex << hr);

	CComPtr<IDisplayRenderEngine> pRenderEngine;
	hr = pRenderEngine.CoCreateInstance(CLSID_DisplayRenderEngine);
	BOOST_CHECK_MESSAGE(SUCCEEDED(hr) && pRenderEngine, "CoCreateInstance CLSID_DisplayRenderEngine failed hr=0x" << hex << hr);
}

void TestSingleton()
{
	CoInit co;

	HRESULT hr = S_OK;
	CComPtr<IDisplayServer>	pWizard[2];

	hr = pWizard[0].CoCreateInstance(CLSID_DisplayServer);
	hr = pWizard[1].CoCreateInstance(CLSID_DisplayServer);

	BOOST_CHECK_EQUAL(pWizard[0], pWizard[1]);
}

HRESULT CreateDispSvr(DWORD dwDispSvrFlags, CComPtr<IDisplayServer> &pDispSvr)
{
	HRESULT hr = pDispSvr.CoCreateInstance(CLSID_DisplayServer);
	BOOST_CHECK_MESSAGE(SUCCEEDED(hr) && pDispSvr, "CoCreateInstance CLSID_DisplayServer failed hr=0x" << hex << hr);

	hr = pDispSvr->Initialize(dwDispSvrFlags, g_hwndDevice, NULL);
	BOOST_CHECK_MESSAGE(SUCCEEDED(hr), "pDispSvr->Initialize failed hr=0x" << hex << hr);
	return hr;
}

void TestDispSvrVideoMixer()
{
	CoInit co;

	CComPtr<IDisplayServer> pDispSvr;
	HRESULT hr = CreateDispSvr(0, pDispSvr);
	if (FAILED(hr))
		return;

	CComPtr<IDisplayRenderEngine> pRenderEngine;
	hr = pDispSvr->GetRenderEngine(&pRenderEngine);
	if (FAILED(hr))
		return;

	if (SUCCEEDED(hr))
	{
		CComQIPtr<IDispSvrVideoMixer> pMixer = pRenderEngine;
		if (pMixer)
		{
			// format unknown *SHOULD NOT* be supported.
			PlaneCaps caps;
			hr = pMixer->QueryPlaneCaps(PLANE_MAINVIDEO, PLANE_FORMAT_UNKNOWN, &caps);
			BOOST_CHECK_MESSAGE(FAILED(hr), "IDispSvrVideoMixer::QueryPlaneCaps(PLANE_MAINVIDEO, PLANE_FORMAT_UNKNOWN, caps) failed. hr=0x" << hex << hr);

			// format YUY2 must be supported by main video.
			hr = pMixer->QueryPlaneCaps(PLANE_MAINVIDEO, PLANE_FORMAT_YUY2, &caps);
			BOOST_CHECK_MESSAGE(SUCCEEDED(hr), "IDispSvrVideoMixer::QueryPlaneCaps(PLANE_MAINVIDEO, PLANE_FORMAT_YUY2, caps) failed. hr=0x" << hex << hr);

			// format YUY2 must be supported by sub video.
			hr = pMixer->QueryPlaneCaps(PLANE_SUBVIDEO, PLANE_FORMAT_YUY2, &caps);
			BOOST_CHECK_MESSAGE(SUCCEEDED(hr), "IDispSvrVideoMixer::QueryPlaneCaps(PLANE_SUBVIDEO, PLANE_FORMAT_YUY2, caps) failed. hr=0x" << hex << hr);

			// format ARGB must be supported by background, graphics, interactive.
			hr = pMixer->QueryPlaneCaps(PLANE_BACKGROUND, PLANE_FORMAT_ARGB, &caps);
			BOOST_CHECK_MESSAGE(SUCCEEDED(hr), "IDispSvrVideoMixer::QueryPlaneCaps(PLANE_BACKGROUND, PLANE_FORMAT_ARGB, caps) failed. hr=0x" << hex << hr);

			hr = pMixer->QueryPlaneCaps(PLANE_GRAPHICS, PLANE_FORMAT_ARGB, &caps);
			BOOST_CHECK_MESSAGE(SUCCEEDED(hr), "IDispSvrVideoMixer::QueryPlaneCaps(PLANE_GRAPHICS, PLANE_FORMAT_ARGB, caps) failed. hr=0x" << hex << hr);

			hr = pMixer->QueryPlaneCaps(PLANE_INTERACTIVE, PLANE_FORMAT_ARGB, &caps);
			BOOST_CHECK_MESSAGE(SUCCEEDED(hr), "IDispSvrVideoMixer::QueryPlaneCaps(PLANE_INTERACTIVE, PLANE_FORMAT_ARGB, caps) failed. hr=0x" << hex << hr);

			// test case for dshow VideoSourceDO.
			CComPtr<IDispSvrVideoMixerPlane> pPlane;
			PlaneInit planeInit = {0};
			planeInit.PlaneID = PLANE_MAINVIDEO;
			planeInit.dwFlags = PLANE_INIT_EXTERNAL_SURFACE;
			planeInit.Format = PLANE_FORMAT_YUY2;	// YUY2 must be supported by main video.
			hr = pMixer->CreatePlane(&planeInit, __uuidof(IDispSvrVideoMixerPlane), (void **)&pPlane);
			BOOST_CHECK_MESSAGE(SUCCEEDED(hr), "IDispSvrVideoMixer::CreatePlane(PLANE_MAINVIDEO, PLANE_FORMAT_UNKNOWN) failed. hr=0x" << hex << hr);
		}
	}
	pDispSvr->Terminate();
}

test_suite* init_unit_test_suite(int argc, char* argv [])
{
	test_suite *pSuite = NULL;

	g_hwndDevice = CreateWindow(_T("STATIC"), _T("DispSvr"), 0, 0, 0, 16, 16, 0, 0, 0, 0);

	pSuite = BOOST_TEST_SUITE("DispSvr unit test");
	pSuite->add(BOOST_TEST_CASE(&TestSingleton));
	pSuite->add(BOOST_TEST_CASE(&TestCoCreateInstance));
	pSuite->add(BOOST_TEST_CASE(&TestDispSvrVideoMixer));

	return pSuite;
}
