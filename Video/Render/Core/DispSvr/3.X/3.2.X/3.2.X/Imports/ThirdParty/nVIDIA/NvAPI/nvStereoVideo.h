 /***************************************************************************\
|*                                                                           *|
|*      Copyright 2008 NVIDIA Corporation.  All rights reserved.        *|
|*                                                                           *|
|*   NOTICE TO USER:                                                         *|
|*                                                                           *|
|*   This source code is subject to NVIDIA ownership rights under U.S.       *|
|*   and international Copyright laws.                                       *|
|*                                                                           *|
|*   This software and the information contained herein is PROPRIETARY       *|
|*   and CONFIDENTIAL to NVIDIA and is being provided under the terms        *|
|*   and conditions of a Non-Disclosure Agreement. Any reproduction or       *|
|*   disclosure to any third party without the express written consent of    *|
|*   NVIDIA is prohibited.                                                   *|
|*                                                                           *|
|*   NVIDIA MAKES NO REPRESENTATION ABOUT THE SUITABILITY OF THIS SOURCE     *|
|*   CODE FOR ANY PURPOSE. IT IS PROVIDED "AS IS" WITHOUT EXPRESS OR         *|
|*   IMPLIED WARRANTY OF ANY KIND.  NVIDIA DISCLAIMS ALL WARRANTIES WITH     *|
|*   REGARD TO THIS SOURCE CODE, INCLUDING ALL IMPLIED WARRANTIES OF         *|
|*   MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR          *|
|*   PURPOSE. IN NO EVENT SHALL NVIDIA BE LIABLE FOR ANY SPECIAL,            *|
|*   INDIRECT, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES          *|
|*   WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN      *|
|*   AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING     *|
|*   OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOURCE      *|
|*   CODE.                                                                   *|
|*                                                                           *|
|*   U.S. Government End Users. This source code is a "commercial item"      *|
|*   as that term is defined at 48 C.F.R. 2.101 (OCT 1995), consisting       *| 
|*   of "commercial computer software" and "commercial computer software     *|
|*   documentation" as such terms are used in 48 C.F.R. 12.212 (SEPT 1995)   *|
|*   and is provided to the U.S. Government only as a commercial end item.   *|
|*   Consistent with 48 C.F.R.12.212 and 48 C.F.R. 227.7202-1 through        *|
|*   227.7202-4 (JUNE 1995), all U.S. Government End Users acquire the       *|
|*   source code with only those rights set forth herein.                    *|
|*                                                                           *|
|*                                                                           *|
 \***************************************************************************/
//
//  Module: nvStereoVideo.h
//      Definitions for private interface of NV stereo video control
//
// **************************************************************************
#ifndef _NVSTEREOVIDEO_H_
#define _NVSTEREOVIDEO_H_

#pragma pack(push,8) // Make sure we have consistent structure packings

#define NV_STEREO_VIDEO_VERSION_MAJOR   1
#define NV_STEREO_VIDEO_VERSION_MINOR   0
#define NV_STEREO_VIDEO_VERSION         (NV_STEREO_VIDEO_VERSION_MINOR | (NV_STEREO_VIDEO_VERSION_MAJOR << 16))

#define NV_STEREO_VIDEO_STATE_ENABLE_MASK   1
#define NV_STEREO_VIDEO_STATE_ENABLE_OFF    0
#define NV_STEREO_VIDEO_STATE_ENABLE_ON     1

typedef enum _NV_STEREO_VIDEO_FORMAT
{
    NV_STEREO_VIDEO_FORMAT_DEFAULT = 0,
    NV_STEREO_VIDEO_FORMAT_LAST,
} NV_STEREO_VIDEO_FORMAT;

typedef enum _NV_STEREO_VIDEO_FILTERING
{
    NV_STEREO_VIDEO_FILTERING_DEFAULT = 0,
    NV_STEREO_VIDEO_FILTERING_LAST,
} NV_STEREO_VIDEO_FILTERING;

typedef struct _NV_STEREO_VIDEO_DATA
{
    DWORD       version;
    DWORD       states;
    DWORD       format;
    DWORD       filtering;
    DWORD       reserved[4];
}  NV_STEREO_VIDEO_DATA;

// {681E31B1-070B-4c8e-94F0-1C47083512C7}
DEFINE_GUID(NVDA_StereoVideoControl, 
0x681e31b1, 0x70b, 0x4c8e, 0x94, 0xf0, 0x1c, 0x47, 0x8, 0x35, 0x12, 0xc7);

#define MAX_NV_VIDEO_PROCESS_PRIVATE_DATA_SIZE  1024

typedef struct _NV_VIDEO_PROCESS_PRIVATE_DATA
{
    GUID    Guid;
    UINT    DataSize;
    BYTE    Data[MAX_NV_VIDEO_PROCESS_PRIVATE_DATA_SIZE];
} NV_VIDEO_PROCESS_PRIVATE_DATA;

#define NV_STEREO_VIDEO_CONTROL_SURFACE_FORMAT      D3DFMT_BINARYBUFFER

#define NV_STEREO_VIDEO_CONTROL_SURFACE_WIDTH       sizeof(NV_VIDEO_PROCESS_PRIVATE_DATA)

#define NV_STEREO_VIDEO_CONTROL_SURFACE_HEIGHT      1

#pragma pack(pop)

#endif //_NVSTEREOVIDEO_H_