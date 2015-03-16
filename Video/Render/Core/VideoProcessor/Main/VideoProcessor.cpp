// VideoProcessor.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include <unknwn.h>
#include "Exports/Inc/VideoProcessor.h"
#include "VideoProcessorNV12toYUY2.h"
#include "VideoProcessorNV12toYV12.h"
#include "VideoProcessorNV12toNV12.h"
#include "VideoProcessorYV12toYUY2.h"

#ifdef _MANAGED
#pragma managed(push, off)
#endif

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
    return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif

VIDEOPROCESSOR_API int CreateVideoProcessor(VideoProcessor::VIDEO_FORMAT InputVideoFormat, 
											VideoProcessor::VIDEO_FORMAT RenderTargetFormat,
											IVideoProcessorEventListener *pEvenListener, 
											void** ppVideoProcessor)
{
	if (!ppVideoProcessor)
		return -1;

	if (VideoProcessor::VIDEO_FORMAT_PRIVATE_NV12 == InputVideoFormat && VideoProcessor::VIDEO_FORMAT_YUY2 == RenderTargetFormat)
	{
		VideoProcessor::CVideoProcessorNV12toYUY2* pCVideoProcessorNV12toYUY2 = 0;
		pCVideoProcessorNV12toYUY2 = new VideoProcessor::CVideoProcessorNV12toYUY2;
		if (pCVideoProcessorNV12toYUY2)
		{
			HRESULT hr = pCVideoProcessorNV12toYUY2->QueryInterface(__uuidof(IVideoProcessor), ppVideoProcessor);
			if (SUCCEEDED(hr) && *ppVideoProcessor)
				return 0;
		}
	}
	else if (VideoProcessor::VIDEO_FORMAT_PRIVATE_NV12 == InputVideoFormat && VideoProcessor::VIDEO_FORMAT_YV12 == RenderTargetFormat)
	{
		VideoProcessor::CVideoProcessorNV12toYV12* pCVideoProcessorNV12toYV12 = 0;
		pCVideoProcessorNV12toYV12 = new VideoProcessor::CVideoProcessorNV12toYV12;
		if (pCVideoProcessorNV12toYV12)
		{
			HRESULT hr = pCVideoProcessorNV12toYV12->QueryInterface(__uuidof(IVideoProcessor), ppVideoProcessor);
			if (SUCCEEDED(hr) && *ppVideoProcessor)
				return 0;
		}
	}
	else if (VideoProcessor::VIDEO_FORMAT_PRIVATE_NV12 == InputVideoFormat && VideoProcessor::VIDEO_FORMAT_NV12 == RenderTargetFormat)
	{
		VideoProcessor::CVideoProcessorNV12toNV12* pCVideoProcessorNV12toNV12 = 0;
		pCVideoProcessorNV12toNV12 = new VideoProcessor::CVideoProcessorNV12toNV12;
		if (pCVideoProcessorNV12toNV12)
		{
			HRESULT hr = pCVideoProcessorNV12toNV12->QueryInterface(__uuidof(IVideoProcessor), ppVideoProcessor);
			if (SUCCEEDED(hr) && *ppVideoProcessor)
				return 0;
		}
	}
	else if (VideoProcessor::VIDEO_FORMAT_PRIVATE_YV12 == InputVideoFormat && VideoProcessor::VIDEO_FORMAT_YUY2 == RenderTargetFormat)
	{
		VideoProcessor::CVideoProcessorYV12toYUY2* pCVideoProcessorYV12toYUY2 = 0;
		pCVideoProcessorYV12toYUY2 = new VideoProcessor::CVideoProcessorYV12toYUY2;
		if (pCVideoProcessorYV12toYUY2)
		{
			HRESULT hr = pCVideoProcessorYV12toYUY2->QueryInterface(__uuidof(IVideoProcessor), ppVideoProcessor);
			if (SUCCEEDED(hr) && *ppVideoProcessor)
				return 0;
		}
	}

	return -1;
}