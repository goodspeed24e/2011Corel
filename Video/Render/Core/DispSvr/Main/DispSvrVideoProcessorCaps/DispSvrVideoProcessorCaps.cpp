//
// DispSvrVideoProcessorCaps is an utility to dump video processor capabilities from a registered DispSvr.
//

#include <tchar.h>
#include <atlbase.h>
#include "../DispSvr.h"
#include <initguid.h>
#include <dxva2api.h>
#include "../Exports/Inc/VideoMixer.h"

#define P(fmt, ...) printf(fmt, __VA_ARGS__)
#define STRING_BUFFER_LENGTH	100
using namespace DispSvr;

void VPAliasString(const GUID &guid, char *pStrBuf, size_t size)
{
	if (DispSvr_VideoProcStretchRect == guid)
		sprintf_s(pStrBuf, size, "DispSvr_VideoProcStretchRect");
	else if (DispSvr_VideoProcPCOM == guid)
		sprintf_s(pStrBuf, size, "DispSvr_VideoProcPCOM");
	else if (DispSvr_VideoProcFastComp == guid)
		sprintf_s(pStrBuf, size, "DispSvr_VideoProcFastComp");
	else if (DispSvr_VideoProcDxvaHD == guid)
		sprintf_s(pStrBuf, size, "DispSvr_VideoProcDxvaHD");
	else if (DXVA2_VideoProcProgressiveDevice == guid)
		sprintf_s(pStrBuf, size, "DXVA2_VideoProcProgressiveDevice");
	else if (DXVA2_VideoProcBobDevice == guid)
		sprintf_s(pStrBuf, size, "DXVA2_VideoProcBobDevice");
	else if (DXVA2_VideoProcSoftwareDevice == guid)
		sprintf_s(pStrBuf, size, "DXVA2_VideoProcSoftwareDevice");
	else
		sprintf_s(pStrBuf, size, "Unknown");
}

int PrintVideoProcessorCaps(IDispSvrVideoProcessor *pVP)
{
	UINT uCount = 0;
	GUID guidCurrentMode = GUID_NULL;
	HRESULT hr;
	UINT i, j;
	OLECHAR strBuffer[STRING_BUFFER_LENGTH];
	char cString[STRING_BUFFER_LENGTH];

	P("IDispSvrVideoProcessor interface is available.\n");
	hr = pVP->GetVideoProcessorMode(&guidCurrentMode);
	if (FAILED(hr))
	{
		P("IDispSvrVideoProcessor::GetVideoProcessorMode() failed. hr=0x%x\n", hr);
	}
	else
	{
		hr = StringFromGUID2(guidCurrentMode, strBuffer, sizeof(strBuffer));
		P("Current video processor mode: %S\n", strBuffer);
	}

	hr = pVP->GetAvailableVideoProcessorModeCount(&uCount);
	if (FAILED(hr))
	{
		P("IDispSvrVideoProcessor::GetAvailableVideoProcessorModeCount() failed. hr=0x%x\n", hr);
		return hr;
	}

	P("Available [%d] video processor modes:\n", uCount);
	if (uCount <= 0)
		return hr;

	GUID *pguidModes = new GUID[uCount];
	hr = pVP->GetAvailableVideoProcessorModes(pguidModes);
	if (FAILED(hr))
	{
		delete [] pguidModes;
		P("IDispSvrVideoProcessor::GetAvailableVideoProcessorModes() failed. hr=0x%x\n", hr);
		return hr;
	}

	for (i = 0; i < uCount; i++)
	{
		hr = StringFromGUID2(pguidModes[i], strBuffer, STRING_BUFFER_LENGTH);
		VPAliasString(pguidModes[i], cString, STRING_BUFFER_LENGTH);
		P("\t%d: %S %s\n", i + 1, strBuffer, cString);
	}
	P("\n");

	VideoProcessorCaps caps;
	ValueRange range;

	for (i = 0; i < uCount; i++)
	{
		P("\n");

		ZeroMemory(&caps, sizeof(caps));
		hr = StringFromGUID2(pguidModes[i], strBuffer, STRING_BUFFER_LENGTH);
		P("------------------------------ VP %d --------------------------------\n", i + 1);
		P("VP guid=%S\n", strBuffer);
		VPAliasString(pguidModes[i], cString, STRING_BUFFER_LENGTH);
		P("VP alias=%s\n", cString);
		hr = pVP->GetVideoProcessorCaps(&pguidModes[i], &caps);
		if (FAILED(hr))
		{
			P("IDispSvrVideoProcessor::GetVideoProcessorCaps(%S) failed. hr=0x%x\n", strBuffer, hr);
			continue;
		}

		if (caps.eType == PROCESSOR_TYPE_SOFTWARE)
		{
			P("Processor type: PROCESSOR_TYPE_SOFTWARE\n");
		}
		else if (caps.eType == PROCESSOR_TYPE_HARDWARE)
		{
			P("Processor type: PROCESSOR_TYPE_HARDWARE\n");
		}
		else
		{
			P("Processor type: unknown\n");
		}

		P("uNumBackwardSamples = %d\n", caps.uNumBackwardSamples);
		P("uNumForwardSamples = %d\n", caps.uNumForwardSamples);

		const int iProcessorCapsFlags[] = {PROCESSOR_CAPS_DEINTERLACE_BLEND, PROCESSOR_CAPS_DEINTERLACE_BOB,
			PROCESSOR_CAPS_DEINTERLACE_ADAPTIVE, PROCESSOR_CAPS_DEINTERLACE_MOTION_COMPENSATION};
		const char *strProcessorCaps[] = {"PROCESSOR_CAPS_DEINTERLACE_BLEND", "PROCESSOR_CAPS_DEINTERLACE_BOB",
			"PROCESSOR_CAPS_DEINTERLACE_ADAPTIVE", "PROCESSOR_CAPS_DEINTERLACE_MOTION_COMPENSATION"};
		P("uProcessorCaps = 0x%x\n", caps.uProcessorCaps);
		if (caps.uProcessorCaps == 0)
			P("\t*none*\n");
		else
			for (j = 0; j < sizeof(iProcessorCapsFlags) / sizeof(iProcessorCapsFlags[0]); j++)
			{
				if (caps.uProcessorCaps & iProcessorCapsFlags[j])
					P("\t%s\n", strProcessorCaps[j]);
			}
		P("\n");

		const VIDEO_FILTER iFilter[] = {VIDEO_FILTER_BRIGHTNESS, VIDEO_FILTER_CONTRAST,
			VIDEO_FILTER_HUE, VIDEO_FILTER_SATURATION,
			VIDEO_FILTER_NOISE_REDUCTION, VIDEO_FILTER_EDGE_ENHANCEMENT};
		const int iFilterCapsFlags[] = {FILTER_CAPS_BRIGHTNESS, FILTER_CAPS_CONTRAST,
			FILTER_CAPS_HUE, FILTER_CAPS_SATURATION,
			FILTER_CAPS_NOISE_REDUCTION, FILTER_CAPS_EDGE_ENHANCEMENT};
		const char *strFilterCaps[] = {"FILTER_CAPS_BRIGHTNESS", "FILTER_CAPS_CONTRAST",
			"FILTER_CAPS_HUE", "FILTER_CAPS_SATURATION",
			"FILTER_CAPS_NOISE_REDUCTION", "FILTER_CAPS_EDGE_ENHANCEMENT"};
		P("uFilterCaps = 0x%x\n", caps.uFilterCaps);
		if (caps.uFilterCaps == 0)
			P("\t*none*\n");
		else
			for (j = 0; j < sizeof(iFilterCapsFlags) / sizeof(iFilterCapsFlags[0]); j++)
			{
				if (caps.uFilterCaps & iFilterCapsFlags[j])
					P("\t%s\n", strFilterCaps[j]);
			}
		P("\n");

		if (caps.uFilterCaps != 0)
		{
			// use SetVideoProcessorMode to check GetFilterValueRange
			P("Filter value ranges:\n");
			hr = pVP->SetVideoProcessorMode(&pguidModes[i]);
			if (FAILED(hr))
			{
				P("IDispSvrVideoProcessor::SetVideoProcessorMode() failed. hr=0x%x\n", hr);
				continue;
			}

			for (j = 0; j < sizeof(iFilterCapsFlags) / sizeof(iFilterCapsFlags[0]); j++)
			{
				if (caps.uFilterCaps & iFilterCapsFlags[j])
				{
					hr = pVP->GetFilterValueRange(iFilter[j], &range);
					if (SUCCEEDED(hr))
					{
						P("%s: Default: %.3f, Min: %.3f, Max: %.3f, Step: %.3f\n", strFilterCaps[j],
							range.fDefaultValue, range.fMinValue, range.fMaxValue, range.fStepSize);
					}
				}
			}
		}
	}
	P("\n\n");

	delete [] pguidModes;
	return hr;
}

int main(int argc, char* argv[])
{
	CoInitialize(NULL);
	{

		DWORD dwDispSvrFlags = DISPSVR_USE_D3D9EX | DISPSVR_NO_RENDER_THREAD | DISPSVR_USE_CUSTOMIZED_OUTPUT;
		CComPtr<IDisplayServer> pDispSvr;
		HWND hwndDevice = CreateWindow(_T("STATIC"), _T("DispSvr"), 0, 0, 0, 16, 16, 0, 0, 0, 0);
		HRESULT hr = pDispSvr.CoCreateInstance(CLSID_DisplayServer);

		if (FAILED(hr))
		{
			P("CoCreateInstance of CLSID_DisplayServer failed. hr=0x%x\n", hr);
			return hr;
		}

		hr = pDispSvr->Initialize(dwDispSvrFlags, hwndDevice, NULL);
		if (FAILED(hr))
		{
			P("pDispSvr->Initialize failed. hr=0x%x\n", hr);
			return hr;
		}

		CComPtr<IDisplayRenderEngine> pRenderEngine;
		hr = pDispSvr->GetRenderEngine(&pRenderEngine);
		if (FAILED(hr))
		{
			P("pDispSvr->GetRenderEngine failed. hr=0x%x\n", hr);
			return hr;
		}

		CComQIPtr<IDispSvrVideoMixer> pVideoMixer = pRenderEngine;
		CComQIPtr<IDispSvrVideoProcessor> pProcessor = pVideoMixer;
		if (pProcessor)
		{
			PrintVideoProcessorCaps(pProcessor);
		}
		else
		{
			P("this DispSvr does not support IDispSvrVideoProcessor interface.\n");
		}

		pDispSvr->Terminate();
		CloseWindow(hwndDevice);

	}
	CoUninitialize();

	return 0;
}

