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
// 	Copyright (c) 2009 InterVideo Corporation.  All Rights Reserved.
//
//-----------------------------------------------------------------------------

#ifndef _I_MP4_VDEC_H_
#define _I_MP4_VDEC_H_

#include <windows.h>

enum E_MP4_DXVA_MODE
{
    E_MP4_DXVA_NVIDIA_VLD = 1,	// NVIDIA DXVA PROPRIETARY VLD level
    E_MP4_DXVA_ATI_VLD = 2,	// ATI DXVA PROPRIETARY VLD level
};

typedef struct MP4VDecASP_Frame_
{
    DWORD   dwCookie;
    int     nFrameIndex;
    int     nWidth;
    int     nHeight;
    unsigned int    progressive_frame;
    BOOL    bCurrentFrameVOPCoded;
    DWORD   dwRgbOutput;
    DWORD   adwWidth[3];
    DWORD   adwHeight[3];
    DWORD   adwStride[3];
    PBYTE   apbFrame[3];
    DWORD   dwReserved[31];

} MP4VDecASP_Frame;

typedef struct MP4VDecASP_OpenOptions_
{
    long    lDxvaVer;
    UINT    uiMP4VGACard;          /// Specify the vendor of VGA card.
    UINT    uiMP4DXVAMode;         /// Specify the DXVA profile mode.
    DWORD   dwBuffers;
    DWORD   dwMP4Variant;
    DWORD   dwReserved[31];

} MP4VDecASP_OpenOptions;

#endif //_I_MP4_VDEC_H_
