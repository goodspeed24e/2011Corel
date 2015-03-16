//=============================================================================
//	THIS SOURCE CODE IS PROPRIETARY INFORMATION BELONGING TO INTERVIDEO, INC.
// 	ANY USE INCLUDING BUT NOT LIMITED TO COPYING OF CODE, CONCEPTS, AND/OR
//	ALGORITHMS IS PROHIBITED EXCEPT WITH EXPRESS WRITTEN PERMISSION BY THE 
//	COMPANY.
//
// 	THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
// 	KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// 	IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
// 	PURPOSE.
//
// 	Copyright (c) 2007 InterVideo Corporation.  All Rights Reserved.
//
//-----------------------------------------------------------------------------

#include <windows.h>
#include "VdoCodecGMO.h"

CVdoCodecGMO::CVdoCodecGMO(IUnknown* pUnkOuter):CGMOUnknown(pUnkOuter)
{
	InitializeCriticalSection(&m_cs);

}

CVdoCodecGMO::~CVdoCodecGMO()
{
	DeleteCriticalSection(&m_cs);

}

//IMethodImpl Methods
HRESULT CVdoCodecGMO::InternalAllocateStreamingResources(void)
{
	return E_NOTIMPL;
}

HRESULT CVdoCodecGMO::InternalDiscontinuity(DWORD dwInputStreamIndex)
{
	return E_NOTIMPL;
}

HRESULT CVdoCodecGMO::InternalFlush(void)
{
	return E_NOTIMPL;
}

HRESULT CVdoCodecGMO::InternalFreeStreamingResources(void)
{
	return E_NOTIMPL;
}

HRESULT CVdoCodecGMO::InternalGetInputMaxLatency(DWORD dwInputStreamIndex, REFERENCE_TIME *prtMaxLatency)
{
	return E_NOTIMPL;
}

HRESULT CVdoCodecGMO::InternalSetInputMaxLatency(DWORD dwInputStreamIndex, REFERENCE_TIME rtMaxLatency)
{
	return E_NOTIMPL;
}

HRESULT CVdoCodecGMO::InternalGetInputSizeInfo(DWORD dwInputStreamIndex, DWORD *pcbSize, DWORD *pulSizeMaxLookahead, DWORD *pulSizeAlignment)
{
	return E_NOTIMPL;
}

HRESULT CVdoCodecGMO::InternalGetOutputSizeInfo(DWORD dwOutputStreamIndex, DWORD *pcbSize, DWORD *pulSizeAlignment)
{
	return E_NOTIMPL;
}

HRESULT CVdoCodecGMO::InternalGetInputStreamInfo(DWORD dwInputStreamIndex, DWORD *pdwFlags)
{
	return E_NOTIMPL;
}
HRESULT CVdoCodecGMO::InternalGetOutputStreamInfo(DWORD dwOutputStreamIndex, DWORD *pdwFlags)
{
	return E_NOTIMPL;
}
HRESULT CVdoCodecGMO::InternalGetInputType(DWORD dwInputStreamIndex, DWORD dwTypeIndex, GMO_MEDIA_TYPE *pmt)
{
	return E_NOTIMPL;
}

HRESULT CVdoCodecGMO::InternalGetOutputType(DWORD dwOutputStreamIndex, DWORD dwTypeIndex, GMO_MEDIA_TYPE *pmt)
{
	return E_NOTIMPL;
}

HRESULT CVdoCodecGMO::InternalProcessInput(DWORD dwInputStreamIndex, IGMediaBuffer *pBuffer, DWORD dwFlags, REFERENCE_TIME rtTimeStamp, REFERENCE_TIME rtTimeLength)
{
	return E_NOTIMPL;
}

HRESULT CVdoCodecGMO::InternalProcessOutput(DWORD dwFlags, DWORD OutputBufferCount, GMO_OUTPUT_DATA_BUFFER *pOutputBuffers, DWORD *pdwStatus)
{
	return E_NOTIMPL;
}