//=============================================================================
//    THIS SOURCE CODE IS PROPRIETARY INFORMATION BELONGING TO INTERVIDEO, INC.
//     ANY USE INCLUDING BUT NOT LIMITED TO COPYING OF CODE, CONCEPTS, AND/OR
//    ALGORITHMS IS PROHIBITED EXCEPT WITH EXPRESS WRITTEN PERMISSION BY THE 
//    COMPANY.
//
//     THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
//     KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
//     IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
//     PURPOSE.
//
//     Copyright (c) 2004 - 2005  InterVideo Corporation.  All Rights Reserved.
//
//-----------------------------------------------------------------------------

#pragma warning ( disable : 4995 )
#pragma warning ( disable : 4996 )
#include <windows.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <malloc.h>
#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#endif
#include "../../Imports/Inc/TRService.h" //The location of including TRSDK.h would be influenced by the include header order
#include <assert.h>
#include "H264VDecHP_impl.h"
#include "global.h"
#include "mbuffer.h"
#include "fmo.h"
#include "videoframe.h"
#include "mb_chroma.h"
#include "transform8x8.h"
#include "mb_average.h"
#include "loopfilter.h"
#include "output.h"
#include "get_block.h"
#include "mb_block4x4.h"
#include "mb_motcomp.h"
#include "block.h"
#include "h264dxva1.h"
#include "image.h"

#include "h264dxvabase.h"

#include "nalucommon.h"
#if !defined(_GLOBAL_IMG_)
#include <process.h>
#endif

#if defined(H264_ENABLE_INTRINSICS)
#include <mmintrin.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#endif

#include "ATSCDec.h"
#include "h264dxva2.h"

extern void BeginAnnexbNALUDecoding PARGS0();
extern void EndAnnexbNALUDecoding PARGS0();
extern void init_out_buffer PARGS0();
extern void uninit_out_buffer PARGS0();

BOOL m_bLast_Slice;

class Remove_OffsetMetadata: std::unary_function<H264_Offset_Metadata_Control, BOOL> 
{
public:
	Remove_OffsetMetadata(DWORD dwCookie)
	{ 
		m_dwCookie = dwCookie; 
	}
	result_type operator( ) ( argument_type metadata_control )
	{
		if(metadata_control.dwCookie == m_dwCookie)
		{
			if(metadata_control.meta_data.Plane_offset_direction)
				free(metadata_control.meta_data.Plane_offset_direction);
			if(metadata_control.meta_data.Plane_offset_value);
				free(metadata_control.meta_data.Plane_offset_value);

			return TRUE;
		}
		else
			return FALSE;
	}
private:
	DWORD m_dwCookie;
};

CH264VDecHP::CH264VDecHP()
{
    InitializeCriticalSection(&m_csDecode);
    m_lRefCount = 1;
    //g_framemgr = 0;
    cpu_type = GetCPULevel();
    
    InitializeVideoDecoder();

    m_pIHVDService = NULL;
}

CH264VDecHP::~CH264VDecHP()
{
    DeleteCriticalSection(&m_csDecode);
}

CPU_LEVEL CH264VDecHP::GetCPULevel()
{    
#if defined(_WIN32) || defined(__INTEL_COMPILER)
#define GetCPUIDRegs(IDReg, SRegs) {\
    _asm xor edx, edx\
    _asm xor ecx, ecx\
    _asm xor ebx, ebx\
    _asm mov eax, IDReg\
    _asm _emit 0x0f\
    _asm _emit 0xa2\
    _asm mov SRegs.uiEAX, eax\
    _asm mov SRegs.uiEBX, ebx\
    _asm mov SRegs.uiECX, ecx\
    _asm mov SRegs.uiEDX, edx\
}
#define GetCPUIDCapability(bResult) {\
    _asm pushfd\
    _asm pop eax\
    _asm mov ebx, eax\
    _asm xor eax, (1<<21)\
    _asm push eax\
    _asm popfd\
    _asm pushfd\
    _asm pop eax\
    _asm xor eax, ebx\
    _asm shr eax, 21\
    _asm and eax, 1\
    _asm mov bResult, eax\
}
#elif defined(__GNUC__) && defined(__i386__)
    // assuming gcc compiler for all Intel x86 CPUs (ie. Linux x86)
#define GetCPUIDRegs(IDReg, SRegs) {          \
    __asm__ (                                 \
    "xorl %%edx, %%edx\n\t"                   \
    "xorl %%ecx, %%ecx\n\t"                   \
    "xorl %%ebx, %%ebx\n\t"                   \
    ".byte 0x0f\n\t"                          \
    ".byte 0xa2\n\t"                          \
    : "=a" (SRegs.uiEAX), "=b" (SRegs.uiEBX), \
    "=c" (SRegs.uiECX), "=d" (SRegs.uiEDX)  \
    : "a" (IDReg) );                          \
}

#define GetCPUIDCapability(bResult) { \
    __asm__ (                         \
    "pushf\n\t"                       \
    "popl %%eax\n\t"                  \
    "movl %%eax, %%ebx\n\t"           \
    "xorl %1, %%eax\n\t"              \
    "pushl %%eax\n\t"                 \
    "popf\n\t"                        \
    "pushf\n\t"                       \
    "popl %%eax\n\t"                  \
    "xorl %%ebx, %%eax\n\t"           \
    "shrl $21, %%eax\n\t"             \
    "andl $1, %%eax\n\t"              \
    : "=a" (bResult)                  \
    : "i" (1<<21)                     \
    );                                \
}
#else
#define GetCPUIDCapability(bCPUIDCapable)
#define GetCPUIDRegs(a,b)
#warning GetCPUIDRegs/GetCPUIDCapability not defined for your architecture!
#endif
    enum CPUIDfeature
    {
        /* for feature register ecx */
        CPUID_PRESCOTT_FEATURE_SSE3                        = (1<<0),
        /* for feature register edx */
        CPUID_STANDARD_FEATURE_MMX                        = (1<<23),
        CPUID_STANDARD_FEATURE_SSE                        = (1<<25),
        CPUID_STANDARD_FEATURE_SSE2                        = (1<<26),
        CPUID_STANDARD_FEATURE_HT                        = (1<<28),
        /* for extended feature register edx */
        CPUID_EXTENDED_FEATURE_AMD_MMX_EXT        = (1<<22),
        CPUID_EXTENDED_FEATURE_3DNOW_EXT        = (1<<30),
        CPUID_EXTENDED_FEATURE_3DNOW            = (1<<31)
    };
    union _uStateTag
    {
        struct 
        {
            unsigned int uiEBX;
            unsigned int uiEDX;
            unsigned int uiECX;
            unsigned int uiEAX;
        } Regs;
        char     szIdString[16];
    } uState;
    unsigned long bCPUIDCapable = 0;
    unsigned int m_uiCPUIDMaxStandardLevel;
    unsigned int m_uiCPUIDPrescottFeature;
    unsigned int m_uiCPUIDStandardFeature;

    GetCPUIDCapability(bCPUIDCapable);
    if(!bCPUIDCapable)
        return CPU_LEVEL_NONE;
    {
        GetCPUIDRegs(0,uState.Regs);
        m_uiCPUIDMaxStandardLevel = uState.Regs.uiEAX;
        if(m_uiCPUIDMaxStandardLevel>=1)
        {
            GetCPUIDRegs(1,uState.Regs);
            m_uiCPUIDPrescottFeature = uState.Regs.uiECX;
            m_uiCPUIDStandardFeature = uState.Regs.uiEDX;
            if(m_uiCPUIDPrescottFeature&CPUID_PRESCOTT_FEATURE_SSE3)
                return CPU_LEVEL_SSE3;
            if(m_uiCPUIDStandardFeature&CPUID_STANDARD_FEATURE_SSE2)
                return CPU_LEVEL_SSE2;
            if(m_uiCPUIDStandardFeature&CPUID_STANDARD_FEATURE_SSE)
                return CPU_LEVEL_SSE;
            if(m_uiCPUIDStandardFeature&CPUID_STANDARD_FEATURE_MMX)
                return CPU_LEVEL_MMX;
        }
    }
    return CPU_LEVEL_NONE;
}

void CH264VDecHP::InitializeVideoDecoder()
{
    int i;

    switch(cpu_type)
    {
    case CPU_LEVEL_NONE:
        //C codes implementation
        inverse_transform4x4 = inverse_transform4x4_c;
        inverse_transform8x8 = inverse_transform8x8_c;
        mb_chroma_8xH_pred = mb_chroma_8xH_pred_c;

        mb_chroma_2xH_pred = mb_chroma_2xH_pred_c;
        mb_chroma_4xH_pred = mb_chroma_4xH_pred_c;
        average_16 = average_16_c;
        average_8 = average_8_c;
        average_4 = average_4_c;
        average_2 = average_2_c;


        Deblock_luma_h_fp[1] = Deblock_luma_h_fp[2] = Deblock_luma_h_fp[3] = Deblock_luma_h_1_c;
        Deblock_luma_v_fp[1] = Deblock_luma_v_fp[2] = Deblock_luma_v_fp[3] = Deblock_luma_v_1_c;
        Deblock_luma_h_p8_fp[1] = Deblock_luma_h_p8_fp[2] = Deblock_luma_h_p8_fp[3] = Deblock_luma_h_1_c;
        Deblock_luma_v_p8_fp[1] = Deblock_luma_v_p8_fp[2] = Deblock_luma_v_p8_fp[3] = Deblock_luma_v_1_c;
        Deblock_chroma_h_fp[1] = Deblock_chroma_h_fp[2] = Deblock_chroma_h_fp[3] = Deblock_chroma_h_1_c;
        Deblock_chroma_v_fp[1] = Deblock_chroma_v_fp[2] = Deblock_chroma_v_fp[3] = Deblock_chroma_v_1_c;
        Deblock_chroma_h_p8_fp[1] = Deblock_chroma_h_p8_fp[2] = Deblock_chroma_h_p8_fp[3] = Deblock_chroma_h_1_c;
        Deblock_chroma_v_p8_fp[1] = Deblock_chroma_v_p8_fp[2] = Deblock_chroma_v_p8_fp[3] = Deblock_chroma_v_1_c;
        Deblock_chromaUV_h_fp[1] = Deblock_chromaUV_h_fp[2] = Deblock_chromaUV_h_fp[3] = Deblock_chromaUV_h_1_c;
        Deblock_chromaUV_v_fp[1] = Deblock_chromaUV_v_fp[2] = Deblock_chromaUV_v_fp[3] = Deblock_chromaUV_v_1_c;
        Deblock_chromaUV_h_p8_fp[1] = Deblock_chromaUV_h_p8_fp[2] = Deblock_chromaUV_h_p8_fp[3] = Deblock_chromaUV_h_1_c;
        Deblock_chromaUV_v_p8_fp[1] = Deblock_chromaUV_v_p8_fp[2] = Deblock_chromaUV_v_p8_fp[3] = Deblock_chromaUV_v_1_c;

        weight_16 = weight_16_c;
        weight_8 = weight_8_c;
        weight_4 = weight_4_c;
        weight_2 = weight_2_c;
        weight_16_b = weight_16_b_c;
        weight_8_b = weight_8_b_c;
        weight_4_b = weight_4_b_c;
        weight_2_b = weight_2_b_c;

        weight_8_uv = weight_8_uv_c;
        weight_4_uv = weight_4_uv_c;
        weight_2_uv = weight_2_uv_c;
        weight_8_b_uv = weight_8_b_uv_c;
        weight_4_b_uv = weight_4_b_uv_c;
        weight_2_b_uv = weight_2_b_uv_c;

        MB_itrans4x4_Luma = MB_itrans4x4_Luma_c;
        MB_itrans8x8_Luma = MB_itrans8x8_Luma_c;

        MB_itrans4x4_Chroma = MB_itrans4x4_Chroma_c;

        for (i=0; i<16; i++)
        {
            get_block_4xh_fp[i] = get_block_4xh_c_fp[i];
            get_block_8xh_fp[i] = get_block_8xh_c_fp[i];
            get_block_16xh_fp[i] = get_block_16xh_c_fp[i];
        }

        DIAG_DOWN_RIGHT_PRED_PDR = DIAG_DOWN_RIGHT_PRED_c;
        DIAG_DOWN_LEFT_PRED_PDL = DIAG_DOWN_LEFT_PRED_c;
        VERT_RIGHT_PRED_PVR = VERT_RIGHT_PRED_c;
        VERT_LEFT_PRED_PVL = VERT_LEFT_PRED_c;
        HOR_UP_PRED_PHU = HOR_UP_PRED_PHU_c;
        HOR_DOWN_PRED_PHD = HOR_DOWN_PRED_PHD_c;    

        pad_boundary_chroma = pad_boundary_chroma_c;
        pad_boundary_luma = pad_boundary_luma_c;

    break;
#ifdef H264_ENABLE_ASM
    case CPU_LEVEL_MMX:
        //MMX codes implementation
        inverse_transform4x4 = inverse_transform4x4_c;
        inverse_transform8x8 = inverse_transform8x8_c;

        mb_chroma_2xH_pred = mb_chroma_2xH_pred_mmx;
        mb_chroma_4xH_pred = mb_chroma_4xH_pred_mmx;
        mb_chroma_8xH_pred = mb_chroma_8xH_pred_c;

        average_16 = average_16_mmx;
        average_8 = average_8_mmx;
        average_4 = average_4_mmx;
        average_2 = average_2_mmx;

        Deblock_luma_h_fp[1] = Deblock_luma_h_fp[2] = Deblock_luma_h_fp[3] = Deblock_luma_h_1_mmx;
        Deblock_luma_v_fp[1] = Deblock_luma_v_fp[2] = Deblock_luma_v_fp[3] = Deblock_luma_v_1_mmx;
        Deblock_luma_h_p8_fp[1] = Deblock_luma_h_p8_fp[2] = Deblock_luma_h_p8_fp[3] = Deblock_luma_h_1_c;
        Deblock_luma_v_p8_fp[1] = Deblock_luma_v_p8_fp[2] = Deblock_luma_v_p8_fp[3] = Deblock_luma_v_p8_1_mmx;
        Deblock_chroma_h_fp[1] = Deblock_chroma_h_fp[2] = Deblock_chroma_h_fp[3] = Deblock_chroma_h_1_mmx;
        Deblock_chroma_v_fp[1] = Deblock_chroma_v_fp[2] = Deblock_chroma_v_fp[3] = Deblock_chroma_v_1_mmx;        
        Deblock_chroma_h_p8_fp[1] = Deblock_chroma_h_p8_fp[2] = Deblock_chroma_h_p8_fp[3] = Deblock_chroma_h_p8_1_mmx;
        Deblock_chroma_v_p8_fp[1] = Deblock_chroma_v_p8_fp[2] = Deblock_chroma_v_p8_fp[3] = Deblock_chroma_v_p8_1_mmx;
        Deblock_chromaUV_h_fp[1] = Deblock_chromaUV_h_fp[2] = Deblock_chromaUV_h_fp[3] = Deblock_chromaUV_h_1_mmx;
        Deblock_chromaUV_v_fp[1] = Deblock_chromaUV_v_fp[2] = Deblock_chromaUV_v_fp[3] = Deblock_chromaUV_v_1_mmx;        
        Deblock_chromaUV_h_p8_fp[1] = Deblock_chromaUV_h_p8_fp[2] = Deblock_chromaUV_h_p8_fp[3] = Deblock_chromaUV_h_p8_1_mmx;
        Deblock_chromaUV_v_p8_fp[1] = Deblock_chromaUV_v_p8_fp[2] = Deblock_chromaUV_v_p8_fp[3] = Deblock_chromaUV_v_p8_1_mmx;

        weight_16 = weight_16_mmx;
        weight_8 = weight_8_mmx;
        weight_4 = weight_4_mmx;
        weight_2 = weight_2_c;
        weight_16_b = weight_16_b_mmx;
        weight_8_b = weight_8_b_mmx;
        weight_4_b = weight_4_b_mmx;
        weight_2_b = weight_2_b_c;

        weight_8_uv = weight_8_uv_c;
        weight_4_uv = weight_4_uv_mmx;
        weight_2_uv = weight_2_uv_mmx;
        weight_8_b_uv = weight_8_b_uv_c;
        weight_4_b_uv = weight_4_b_uv_c;
        weight_2_b_uv = weight_2_b_uv_mmx;

        MB_itrans4x4_Luma = MB_itrans4x4_Luma_c;
        MB_itrans8x8_Luma = MB_itrans8x8_Luma_c;
        MB_itrans4x4_Chroma = MB_itrans4x4_Chroma_mmx;

        for (i=0; i<16; i++)
        {
            get_block_4xh_fp[i] = get_block_4xh_mmx_fp[i];
            get_block_8xh_fp[i] = get_block_8xh_c_fp[i];
            get_block_16xh_fp[i] = get_block_16xh_c_fp[i];
        }

        DIAG_DOWN_RIGHT_PRED_PDR = DIAG_DOWN_RIGHT_PRED_sse;
        DIAG_DOWN_LEFT_PRED_PDL = DIAG_DOWN_LEFT_PRED_sse;
        VERT_RIGHT_PRED_PVR = VERT_RIGHT_PRED_sse;
        VERT_LEFT_PRED_PVL = VERT_LEFT_PRED_sse;
        HOR_UP_PRED_PHU = HOR_UP_PRED_PHU_sse;
        HOR_DOWN_PRED_PHD = HOR_DOWN_PRED_PHD_sse;

        pad_boundary_chroma = pad_boundary_chroma_sse;
        pad_boundary_luma = pad_boundary_luma_sse;

        break;
#endif //H264_ENABLE_ASM
#ifdef H264_ENABLE_INTRINSICS
    case CPU_LEVEL_SSE:
        //SSE codes implementation
        inverse_transform4x4 = inverse_transform4x4_sse;
        inverse_transform8x8 = inverse_transform8x8_sse;
        mb_chroma_8xH_pred = mb_chroma_8xH_pred_mmx;
#ifdef H264_ENABLE_ASM
        mb_chroma_2xH_pred = mb_chroma_2xH_pred_mmx;
        mb_chroma_4xH_pred = mb_chroma_4xH_pred_mmx;
        average_16 = average_16_mmx;
        average_8 = average_8_mmx;
        average_4 = average_4_mmx;
        average_2 = average_2_mmx;
#else
        mb_chroma_2xH_pred = mb_chroma_2xH_pred_c;
        mb_chroma_4xH_pred = mb_chroma_4xH_pred_c;
        mb_chroma_8xH_pred = mb_chroma_8xH_pred_c;
        average_16 = average_16_c;
        average_8 = average_8_c;
        average_4 = average_4_c;
        average_2 = average_2_c;
#endif

        Deblock_luma_h_fp[1] = Deblock_luma_h_fp[2] = Deblock_luma_h_fp[3] = Deblock_luma_h_1_mmx;
        Deblock_luma_v_fp[1] = Deblock_luma_v_fp[2] = Deblock_luma_v_fp[3] = Deblock_luma_v_1_mmx;
        Deblock_luma_h_p8_fp[1] = Deblock_luma_h_p8_fp[2] = Deblock_luma_h_p8_fp[3] = Deblock_luma_h_p8_1_sse;
        Deblock_luma_v_p8_fp[1] = Deblock_luma_v_p8_fp[2] = Deblock_luma_v_p8_fp[3] = Deblock_luma_v_p8_1_mmx;
        Deblock_chroma_h_fp[1] = Deblock_chroma_h_fp[2] = Deblock_chroma_h_fp[3] = Deblock_chroma_h_1_mmx;
        Deblock_chroma_v_fp[1] = Deblock_chroma_v_fp[2] = Deblock_chroma_v_fp[3] = Deblock_chroma_v_1_mmx;        
        Deblock_chroma_h_p8_fp[1] = Deblock_chroma_h_p8_fp[2] = Deblock_chroma_h_p8_fp[3] = Deblock_chroma_h_p8_1_mmx;
        Deblock_chroma_v_p8_fp[1] = Deblock_chroma_v_p8_fp[2] = Deblock_chroma_v_p8_fp[3] = Deblock_chroma_v_p8_1_mmx;
        Deblock_chromaUV_h_fp[1] = Deblock_chromaUV_h_fp[2] = Deblock_chromaUV_h_fp[3] = Deblock_chromaUV_h_1_mmx;
        Deblock_chromaUV_v_fp[1] = Deblock_chromaUV_v_fp[2] = Deblock_chromaUV_v_fp[3] = Deblock_chromaUV_v_1_mmx;        
        Deblock_chromaUV_h_p8_fp[1] = Deblock_chromaUV_h_p8_fp[2] = Deblock_chromaUV_h_p8_fp[3] = Deblock_chromaUV_h_p8_1_mmx;
        Deblock_chromaUV_v_p8_fp[1] = Deblock_chromaUV_v_p8_fp[2] = Deblock_chromaUV_v_p8_fp[3] = Deblock_chromaUV_v_p8_1_mmx;

        weight_16 = weight_16_mmx;
        weight_8 = weight_8_mmx;
        weight_4 = weight_4_mmx;
        weight_2 = weight_2_sse;
        weight_16_b = weight_16_b_mmx;
        weight_8_b = weight_8_b_mmx;
        weight_4_b = weight_4_b_mmx;
        weight_2_b = weight_2_b_sse;

        weight_8_uv = weight_8_uv_sse;
        weight_4_uv = weight_4_uv_mmx;
        weight_2_uv = weight_2_uv_mmx;
        weight_8_b_uv = weight_8_b_uv_sse;
        weight_4_b_uv = weight_4_b_uv_sse;
        weight_2_b_uv = weight_2_b_uv_mmx;

        MB_itrans4x4_Luma = MB_itrans4x4_Luma_sse;
        MB_itrans8x8_Luma = MB_itrans8x8_Luma_sse;
#ifdef H264_ENABLE_ASM
        MB_itrans4x4_Chroma = MB_itrans4x4_Chroma_mmx;
#else
        MB_itrans4x4_Chroma = MB_itrans4x4_Chroma_c;
#endif
        for (i=0; i<16; i++)
        {
            get_block_4xh_fp[i] = get_block_4xh_mmx_fp[i];
            get_block_8xh_fp[i] = get_block_8xh_sse_fp[i];
            get_block_16xh_fp[i] = get_block_16xh_sse_fp[i];
        }

        DIAG_DOWN_RIGHT_PRED_PDR = DIAG_DOWN_RIGHT_PRED_sse;
        DIAG_DOWN_LEFT_PRED_PDL = DIAG_DOWN_LEFT_PRED_sse;
        VERT_RIGHT_PRED_PVR = VERT_RIGHT_PRED_sse;
        VERT_LEFT_PRED_PVL = VERT_LEFT_PRED_sse;
        HOR_UP_PRED_PHU = HOR_UP_PRED_PHU_sse;
        HOR_DOWN_PRED_PHD = HOR_DOWN_PRED_PHD_sse;

        pad_boundary_chroma = pad_boundary_chroma_sse;
        pad_boundary_luma = pad_boundary_luma_sse;

        break;
    case CPU_LEVEL_SSE2:
        //SSE2 codes implementation
        inverse_transform4x4 = inverse_transform4x4_sse;
        inverse_transform8x8 = inverse_transform8x8_sse2;
        mb_chroma_8xH_pred = mb_chroma_8xH_pred_sse2;
#ifdef H264_ENABLE_ASM
        mb_chroma_2xH_pred = mb_chroma_2xH_pred_mmx;
        mb_chroma_4xH_pred = mb_chroma_4xH_pred_sse2;
        average_16 = average_16_mmx;
        average_8 = average_8_mmx;
        average_4 = average_4_mmx;
        average_2 = average_2_mmx;
#else
        mb_chroma_2xH_pred = mb_chroma_2xH_pred_c;
        mb_chroma_4xH_pred = mb_chroma_4xH_pred_c;
        mb_chroma_8xH_pred = mb_chroma_8xH_pred_c;
        average_16 = average_16_c;
        average_8 = average_8_c;
        average_4 = average_4_c;
        average_2 = average_2_c;
#endif

        Deblock_luma_h_fp[1] = Deblock_luma_h_fp[2] = Deblock_luma_h_fp[3] = Deblock_luma_h_1_mmx;
        Deblock_luma_v_fp[1] = Deblock_luma_v_fp[2] = Deblock_luma_v_fp[3] = Deblock_luma_v_1_mmx;
        Deblock_luma_h_p8_fp[1] = Deblock_luma_h_p8_fp[2] = Deblock_luma_h_p8_fp[3] = Deblock_luma_h_p8_1_sse2;
        Deblock_luma_v_p8_fp[1] = Deblock_luma_v_p8_fp[2] = Deblock_luma_v_p8_fp[3] = Deblock_luma_v_p8_1_mmx;
        Deblock_chroma_h_fp[1] = Deblock_chroma_h_fp[2] = Deblock_chroma_h_fp[3] = Deblock_chroma_h_1_mmx;
        Deblock_chroma_v_fp[1] = Deblock_chroma_v_fp[2] = Deblock_chroma_v_fp[3] = Deblock_chroma_v_1_sse2;        
        Deblock_chroma_h_p8_fp[1] = Deblock_chroma_h_p8_fp[2] = Deblock_chroma_h_p8_fp[3] = Deblock_chroma_h_p8_1_sse2;
        Deblock_chroma_v_p8_fp[1] = Deblock_chroma_v_p8_fp[2] = Deblock_chroma_v_p8_fp[3] = Deblock_chroma_v_p8_1_sse2;
        Deblock_chromaUV_h_fp[1] = Deblock_chromaUV_h_fp[2] = Deblock_chromaUV_h_fp[3] = Deblock_chromaUV_h_1_mmx;
        Deblock_chromaUV_v_fp[1] = Deblock_chromaUV_v_fp[2] = Deblock_chromaUV_v_fp[3] = Deblock_chromaUV_v_1_sse2;        
        Deblock_chromaUV_h_p8_fp[1] = Deblock_chromaUV_h_p8_fp[2] = Deblock_chromaUV_h_p8_fp[3] = Deblock_chromaUV_h_p8_1_sse2;
        Deblock_chromaUV_v_p8_fp[1] = Deblock_chromaUV_v_p8_fp[2] = Deblock_chromaUV_v_p8_fp[3] = Deblock_chromaUV_v_p8_1_sse2;

        weight_16 = weight_16_mmx;
        weight_8 = weight_8_mmx;
        weight_4 = weight_4_mmx;
        weight_2 = weight_2_c;
        weight_16_b = weight_16_b_mmx;
        weight_8_b = weight_8_b_mmx;
        weight_4_b = weight_4_b_mmx;
        weight_2_b = weight_2_b_c;

        weight_8_uv = weight_8_uv_sse2;
        weight_4_uv = weight_4_uv_mmx;
        weight_2_uv = weight_2_uv_mmx;
        weight_8_b_uv = weight_8_b_uv_sse;
        weight_4_b_uv = weight_4_b_uv_sse2;
        weight_2_b_uv = weight_2_b_uv_mmx;

        MB_itrans4x4_Luma = MB_itrans4x4_Luma_sse2;
        MB_itrans8x8_Luma = MB_itrans8x8_Luma_sse2;
#ifdef H264_ENABLE_ASM
        MB_itrans4x4_Chroma = MB_itrans4x4_Chroma_mmx;
#else
        MB_itrans4x4_Chroma = MB_itrans4x4_Chroma_c;
#endif
        for (i=0; i<16; i++)
        {
            get_block_4xh_fp[i] = get_block_4xh_mmx_fp[i];
            get_block_8xh_fp[i] = get_block_8xh_sse2_fp[i];
            get_block_16xh_fp[i] = get_block_16xh_sse2_fp[i];
        }
        DIAG_DOWN_RIGHT_PRED_PDR = DIAG_DOWN_RIGHT_PRED_sse2;
        DIAG_DOWN_LEFT_PRED_PDL = DIAG_DOWN_LEFT_PRED_sse2;
        VERT_RIGHT_PRED_PVR = VERT_RIGHT_PRED_sse2;
        VERT_LEFT_PRED_PVL = VERT_LEFT_PRED_sse2;
        HOR_UP_PRED_PHU = HOR_UP_PRED_PHU_sse2;
        HOR_DOWN_PRED_PHD = HOR_DOWN_PRED_PHD_sse2;    

        pad_boundary_chroma = pad_boundary_chroma_sse2;
        pad_boundary_luma = pad_boundary_luma_sse2;

        break;
    case CPU_LEVEL_SSE3:
        //SSE3 codes implementation
        inverse_transform4x4 = inverse_transform4x4_sse;
        inverse_transform8x8 = inverse_transform8x8_sse2;
        mb_chroma_8xH_pred = mb_chroma_8xH_pred_sse2;
#ifdef H264_ENABLE_ASM
        mb_chroma_2xH_pred = mb_chroma_2xH_pred_mmx;
        mb_chroma_4xH_pred = mb_chroma_4xH_pred_sse2;
        average_16 = average_16_mmx;
        average_8 = average_8_mmx;
        average_4 = average_4_mmx;
        average_2 = average_2_mmx;
#else
        mb_chroma_2xH_pred = mb_chroma_2xH_pred_c;
        mb_chroma_4xH_pred = mb_chroma_4xH_pred_c;
        mb_chroma_8xH_pred = mb_chroma_8xH_pred_c;
        average_16 = average_16_c;
        average_8 = average_8_c;
        average_4 = average_4_c;
        average_2 = average_2_c;
#endif

        Deblock_luma_h_fp[1] = Deblock_luma_h_fp[2] = Deblock_luma_h_fp[3] = Deblock_luma_h_1_mmx;
        Deblock_luma_v_fp[1] = Deblock_luma_v_fp[2] = Deblock_luma_v_fp[3] = Deblock_luma_v_1_mmx;
        Deblock_luma_h_p8_fp[1] = Deblock_luma_h_p8_fp[2] = Deblock_luma_h_p8_fp[3] = Deblock_luma_h_p8_1_sse2;
        Deblock_luma_v_p8_fp[1] = Deblock_luma_v_p8_fp[2] = Deblock_luma_v_p8_fp[3] = Deblock_luma_v_p8_1_mmx;
        Deblock_chroma_h_fp[1] = Deblock_chroma_h_fp[2] = Deblock_chroma_h_fp[3] = Deblock_chroma_h_1_mmx;
        Deblock_chroma_v_fp[1] = Deblock_chroma_v_fp[2] = Deblock_chroma_v_fp[3] = Deblock_chroma_v_1_sse2;
        Deblock_chroma_h_p8_fp[1] = Deblock_chroma_h_p8_fp[2] = Deblock_chroma_h_p8_fp[3] = Deblock_chroma_h_p8_1_sse2;
        Deblock_chroma_v_p8_fp[1] = Deblock_chroma_v_p8_fp[2] = Deblock_chroma_v_p8_fp[3] = Deblock_chroma_v_p8_1_sse2;
        Deblock_chromaUV_h_fp[1] = Deblock_chromaUV_h_fp[2] = Deblock_chromaUV_h_fp[3] = Deblock_chromaUV_h_1_mmx;
        Deblock_chromaUV_v_fp[1] = Deblock_chromaUV_v_fp[2] = Deblock_chromaUV_v_fp[3] = Deblock_chromaUV_v_1_sse2;
        Deblock_chromaUV_h_p8_fp[1] = Deblock_chromaUV_h_p8_fp[2] = Deblock_chromaUV_h_p8_fp[3] = Deblock_chromaUV_h_p8_1_sse2;
        Deblock_chromaUV_v_p8_fp[1] = Deblock_chromaUV_v_p8_fp[2] = Deblock_chromaUV_v_p8_fp[3] = Deblock_chromaUV_v_p8_1_sse2;

        weight_16 = weight_16_mmx;
        weight_8 = weight_8_mmx;
        weight_4 = weight_4_mmx;
        weight_2 = weight_2_c;
        weight_16_b = weight_16_b_mmx;
        weight_8_b = weight_8_b_mmx;
        weight_4_b = weight_4_b_mmx;
        weight_2_b = weight_2_b_c;

        weight_8_uv = weight_8_uv_sse2;
        weight_4_uv = weight_4_uv_mmx;
        weight_2_uv = weight_2_uv_mmx;
        weight_8_b_uv = weight_8_b_uv_sse2;
        weight_4_b_uv = weight_4_b_uv_sse2;
        weight_2_b_uv = weight_2_b_uv_mmx;

        MB_itrans4x4_Luma = MB_itrans4x4_Luma_sse2;
        MB_itrans8x8_Luma = MB_itrans8x8_Luma_sse2;
#ifdef H264_ENABLE_ASM
        MB_itrans4x4_Chroma = MB_itrans4x4_Chroma_sse2;
#else
        MB_itrans4x4_Chroma = MB_itrans4x4_Chroma_c;
#endif

        for (i=0; i<16; i++)
        {
            get_block_4xh_fp[i] = get_block_4xh_mmx_fp[i];
            get_block_8xh_fp[i] = get_block_8xh_sse2_fp[i];
            get_block_16xh_fp[i] = get_block_16xh_sse2_fp[i];
        }

        DIAG_DOWN_RIGHT_PRED_PDR = DIAG_DOWN_RIGHT_PRED_sse2;
        DIAG_DOWN_LEFT_PRED_PDL = DIAG_DOWN_LEFT_PRED_sse2;
        VERT_RIGHT_PRED_PVR = VERT_RIGHT_PRED_sse2;
        VERT_LEFT_PRED_PVL = VERT_LEFT_PRED_sse2;
        HOR_UP_PRED_PHU = HOR_UP_PRED_PHU_sse2;
        HOR_DOWN_PRED_PHD = HOR_DOWN_PRED_PHD_sse2;    

        pad_boundary_chroma = pad_boundary_chroma_sse2;
        pad_boundary_luma = pad_boundary_luma_sse2;

        break;
#endif //H264_ENABLE_ASM
    default:
        //C codes implementation
        inverse_transform4x4 = inverse_transform4x4_c;
        inverse_transform8x8 = inverse_transform8x8_c;

        mb_chroma_2xH_pred = mb_chroma_2xH_pred_c;
        mb_chroma_4xH_pred = mb_chroma_4xH_pred_c;
        mb_chroma_8xH_pred = mb_chroma_8xH_pred_c;

        average_16 = average_16_c;
        average_8 = average_8_c;
        average_4 = average_4_c;
        average_2 = average_2_c;

        Deblock_luma_h_fp[1] = Deblock_luma_h_fp[2] = Deblock_luma_h_fp[3] = Deblock_luma_h_1_c;
        Deblock_luma_v_fp[1] = Deblock_luma_v_fp[2] = Deblock_luma_v_fp[3] = Deblock_luma_v_1_c;
        Deblock_luma_h_p8_fp[1] = Deblock_luma_h_p8_fp[2] = Deblock_luma_h_p8_fp[3] = Deblock_luma_h_1_c;
        Deblock_luma_v_p8_fp[1] = Deblock_luma_v_p8_fp[2] = Deblock_luma_v_p8_fp[3] = Deblock_luma_v_1_c;
        Deblock_chroma_h_fp[1] = Deblock_chroma_h_fp[2] = Deblock_chroma_h_fp[3] = Deblock_chroma_h_1_c;
        Deblock_chroma_v_fp[1] = Deblock_chroma_v_fp[2] = Deblock_chroma_v_fp[3] = Deblock_chroma_v_1_c;    
        Deblock_chroma_h_p8_fp[1] = Deblock_chroma_h_p8_fp[2] = Deblock_chroma_h_p8_fp[3] = Deblock_chroma_h_1_c;
        Deblock_chroma_v_p8_fp[1] = Deblock_chroma_v_p8_fp[2] = Deblock_chroma_v_p8_fp[3] = Deblock_chroma_v_1_c;
        Deblock_chromaUV_h_fp[1] = Deblock_chromaUV_h_fp[2] = Deblock_chromaUV_h_fp[3] = Deblock_chromaUV_h_1_c;
        Deblock_chromaUV_v_fp[1] = Deblock_chromaUV_v_fp[2] = Deblock_chromaUV_v_fp[3] = Deblock_chromaUV_v_1_c;    
        Deblock_chromaUV_h_p8_fp[1] = Deblock_chromaUV_h_p8_fp[2] = Deblock_chromaUV_h_p8_fp[3] = Deblock_chromaUV_h_1_c;
        Deblock_chromaUV_v_p8_fp[1] = Deblock_chromaUV_v_p8_fp[2] = Deblock_chromaUV_v_p8_fp[3] = Deblock_chromaUV_v_1_c;

        weight_16 = weight_16_c;
        weight_8 = weight_8_c;
        weight_4 = weight_4_c;
        weight_2 = weight_2_c;
        weight_16_b = weight_16_b_c;
        weight_8_b = weight_8_b_c;
        weight_4_b = weight_4_b_c;
        weight_2_b = weight_2_b_c;

        weight_8_uv = weight_8_uv_c;
        weight_4_uv = weight_4_uv_c;
        weight_2_uv = weight_2_uv_c;
        weight_8_b_uv = weight_8_b_uv_c;
        weight_4_b_uv = weight_4_b_uv_c;
        weight_2_b_uv = weight_2_b_uv_c;

        MB_itrans4x4_Luma = MB_itrans4x4_Luma_c;
        MB_itrans8x8_Luma = MB_itrans8x8_Luma_c;
        MB_itrans4x4_Chroma = MB_itrans4x4_Chroma_c;

        for (i=0; i<16; i++)
        {
            get_block_4xh_fp[i] = get_block_4xh_c_fp[i];
            get_block_8xh_fp[i] = get_block_8xh_c_fp[i];
            get_block_16xh_fp[i] = get_block_16xh_c_fp[i];
        }

        DIAG_DOWN_RIGHT_PRED_PDR = DIAG_DOWN_RIGHT_PRED_c;
        DIAG_DOWN_LEFT_PRED_PDL = DIAG_DOWN_LEFT_PRED_c;
        VERT_RIGHT_PRED_PVR = VERT_RIGHT_PRED_c;
        VERT_LEFT_PRED_PVL = VERT_LEFT_PRED_c;
        HOR_UP_PRED_PHU = HOR_UP_PRED_PHU_c;
        HOR_DOWN_PRED_PHD = HOR_DOWN_PRED_PHD_c;

        pad_boundary_chroma = pad_boundary_chroma_c;
        pad_boundary_luma = pad_boundary_luma_c;

        break;    
    };

    set_mb_comp_function_pointers();
}

void *CH264VDecHP::operator new(size_t s)
{
    return VirtualAlloc(0,s,MEM_COMMIT,PAGE_READWRITE);
}

void CH264VDecHP::operator delete(void *p)
{
    if(p)
        VirtualFree(p,0,MEM_RELEASE);
}

STDMETHODIMP_(ULONG) CH264VDecHP::AddRef()
{
    return InterlockedIncrement(&m_lRefCount);
}

STDMETHODIMP_(ULONG) CH264VDecHP::Release()
{
    LONG relval;

    relval = InterlockedDecrement(&m_lRefCount);
    if(relval<=0)
    {
        delete this;
        return 0;
    }
    return relval;
}

#if !defined(_COLLECT_PIC_)
#ifdef _GLOBAL_IMG_
HRESULT CH264VDecHP::Open(IN const H264VDecHP_OpenOptionsEx *pOptions,
                                                    IN const DWORD dwSize)
#else
HRESULT CH264VDecHP::Open(IN const H264VDecHP_OpenOptionsEx *pOptions,
                                                    IN const DWORD dwSize, ImageParameters **imgp)
#endif

#else
HRESULT CH264VDecHP::Open(IN const H264VDecHP_OpenOptionsEx *pOptions,
                                                    IN const DWORD dwSize, void **imgp)
#endif
{
    DEBUG_INFO("CH264VDecHP::Open");

#if !defined(_GLOBAL_IMG_)
    ImageParameters *img;
#endif
    int qsize;
    H264VDecHP_OpenOptionsEx Options;

#ifdef _COLLECT_PIC_    
    stream_par *stream;
#endif

    if(pOptions==0 || pOptions->pvDataContext==0)
        return E_FAIL;
    if(dwSize<sizeof(Options))
    {
        ZeroMemory(&Options,sizeof(Options));
        memcpy(&Options,pOptions,dwSize);
        pOptions = &Options;
    }

    //KevinChien : stream_par
#ifdef _COLLECT_PIC_    
    unsigned int i;
    (*imgp) = (unsigned char *) _aligned_malloc(sizeof(struct stream_par), 16);
    if((*imgp)==NULL)
        return E_OUTOFMEMORY;
    stream = (stream_par*) *imgp;
    stream_global = stream;    
    memset(stream_global, 0, sizeof(struct stream_par));
    stream_global->nalu_mvc_extension.valid = FALSE;
    stream_global->nalu_mvc_extension.bIsPrefixNALU = FALSE;
    stream_global->bMVC = FALSE;
    stream_global->m_Initial_Flag = TRUE;
    stream_global->m_iStop_Decode = 0;
    g_bEOS = FALSE;
    stream_global->m_bMultiThreadModeSwitch = 0;
    stream_global->m_iExecute_Of_I_Frame = FALSE;
    stream_global->pre_sps_width = (1920>>4)-1;
    stream_global->pre_sps_height = (1088>>4)-1;
    stream_global->num_views = 1;
    stream_global->dpb_pre_alloc_views = 0;
    memset(stream_global->m_pbValidViews, 0, 16 * sizeof(BOOL));
    stream_global->m_CurrOutputViewIndex = -1;
    stream_global->m_iPOCofDroppedFrame = 0x80000000;
    stream_global->m_iFrameNeedtoSkip = 0;

    InitializeCriticalSection( &stream_global->crit_dyna_expand );
    InitializeCriticalSection( &stream_global->m_csExit_MB );
	InitializeCriticalSection( &stream_global->m_csOffsetMetadata );

    nalu_global = AllocNALU(MAX_CODED_FRAME_SIZE);
    nalu_global_available = 0;

    if(pOptions->dwSingleThreadMode == 1)
        stream_global->m_bIsSingleThreadMode = TRUE;
    else
        stream_global->m_bIsSingleThreadMode = FALSE;

    int nIMG_num = (stream_global->m_bIsSingleThreadMode == TRUE)? 1: IMG_NUM;

    img_array = (img_par**) _aligned_malloc(nIMG_num*sizeof(img_par*), 16);
    if(img_array==NULL)
        return E_OUTOFMEMORY;

    for (i=0; i<nIMG_num; i++)
    {
        // Align img pointer to units of 16 bytes
        img_array[i] = (struct img_par *) _aligned_malloc(sizeof(struct img_par), 16);    
        if(img_array[i]==NULL)
            return E_OUTOFMEMORY;
        img = img_array[i];
        IMGPAR array_index = i;
        IMGPAR stream_global = stream_global;
#if defined(_HW_ACCEL_)
        memset(IMGPAR m_pBufferInfo, 0, 6*sizeof(AMVABUFFERINFO));
#endif
        img->m_active_pps[0].Valid = NULL;
        img->m_active_pps[1].Valid = NULL;
        img->m_active_sps[0].Valid = NULL;
        img->m_active_sps[1].Valid = NULL;
        prev_dec_picture = NULL;
    }
#endif


#if defined(_HW_ACCEL_)
    if(IviNotDxva != pOptions->dxvaVer)
    {
        g_DXVAVer = pOptions->dxvaVer;

        if(g_DXVAVer==IviDxva1)
        {
            if(pOptions->dwBuffers>5)
            {
                HW_BUF_SIZE = qsize = pOptions->dwBuffers; // the buffer number is passed from video render
                g_framemgr = new CH264VideoFrameMgr(max(qsize*4, 36));
            }
            else
                return -1;    // fail DXVA if too few buffers
        }
        else if(g_DXVAVer==IviDxva2)
        {
            HW_BUF_SIZE = qsize = pOptions->dwBuffers; // the buffer number is passed from video render
            g_framemgr = new CH264VideoFrameMgr(60);
        }
    }
    else
#endif
    {
        qsize = pOptions->dwBuffers+8; //add 8 for GMO interface due to performance issue
        if(qsize < 3)
            qsize = 3;
        if(qsize > 16) 
            qsize = 16;
        qsize += 11+2;
        qsize *= 3;
        g_framemgr = new CH264VideoFrameMgr(qsize);
#ifdef _HW_ACCEL_
        g_DXVAVer = IviNotDxva;
#endif
    }

    g_pCCDec = new CATSCDec();

    p_out = -1;
    p_ref = -1;

    init_out_buffer ARGS0();

    // time for total decoding session
    tot_time = 0;
    //~KevinChien : End of stream_par

#ifdef _COLLECT_PIC_
    for (i=0; i<nIMG_num; i++)
    {        
        img = img_array[i];
#endif
        // IoK, 9/22/2005: temporary place to insert all initialization of (former) global variables
        // IoK: INITIALIZATION VALUES - collected from different files.
        // Someone needs to run a sanity check on them...
        //get_data_fcn = NULL; // Set a few lines below
        //H264_pvDataContext=NULL; // Set a few lines below
        //g_framemgr = 0; // Set a few lines below
        //IsFirstByteStreamNALU=1; // Same as previous
        IMGPAR FileFormat = PAR_OF_ANNEXB;
        LastAccessUnitExists=0;
        NALUCount=0;
        last_dquant = 0;
        MbToSliceGroupMap = NULL;
        MapUnitToSliceGroupMap = NULL;
        global_init_done = 0;
        mb_pos_table_size = -1;
        pic_width_in_mbs=-1;
        Co_located_MB = NULL;
        pending_output = NULL;
        pending_output_state = FRAME;
        // ~IoK, 9/22/2005
        nalu = AllocNALU(MAX_CODED_FRAME_SIZE);

        get_data_fcn  = pOptions->pfnDataCallback;
        H264_pvDataContext = pOptions->pvDataContext;

        get_param_fcn = pOptions->pfnGetParamCallback;

#if !defined(_COLLECT_PIC_)
        // Allocate Slice data struct
        malloc_slice ARGS0();
#else
        IMGPAR currentSlice = NULL;
#endif

        BeginAnnexbNALUDecoding ARGS0();        

#ifdef _COLLECT_PIC_
        IMGPAR slice_number = 0;
        IMGPAR firstSlice = NULL;
        IMGPAR currentSlice = NULL;
        IMGPAR prevSlice = NULL;    
        IMGPAR ori_nalu_buf = nalu->buf;

        //g_DXVAVer = m_lDXVAVer;
        g_DXVAMode = pOptions->uiH264DXVAMode;
        IMGPAR UnCompress_Idx = -1;
#endif

        // This is moved here from Create
#ifdef _WIN32
        switch (pOptions->dwH264RegKey & 0x00000007)
        {
        case H264_REG_SMART_DEC_0:    g_dwSmartDecLevel = SMART_DEC_LEVEL_0;    break;
        case H264_REG_SMART_DEC_1:    g_dwSmartDecLevel = SMART_DEC_LEVEL_1;    break;
        case H264_REG_SMART_DEC_2:    g_dwSmartDecLevel = SMART_DEC_LEVEL_2;    break;
        case H264_REG_SMART_DEC_3:    g_dwSmartDecLevel = SMART_DEC_LEVEL_3;    break;
        case H264_REG_SMART_DEC_6:    g_dwSmartDecLevel = SMART_DEC_LEVEL_6;    break;
        default: g_dwSmartDecLevel = SMART_DEC_DISABLE;// = 0
        }
#if defined (_COLLECT_PIC_)
        stream_global->m_dwSavedSmartDecLevel = g_dwSmartDecLevel;
        stream_global->m_resize_width_height_reg = (pOptions->dwH264RegKey & H264_REG_RESIZE_SD) ? 1 : ((pOptions->dwH264RegKey & H264_REG_RESIZE_HEIGHT) ? 2 : 0);
        stream_global->m_resize_width_height = stream_global->m_resize_width_height_reg;
#endif
        IMGPAR smart_dec = g_dwSmartDecLevel;
        IMGPAR de_blocking_flag = (pOptions->dwH264RegKey & H264_REG_DEBLOCKING) ? 1 : 0;    // only for single thread mode now
        IMGPAR Hybrid_Decoding = (pOptions->dwH264RegKey & H264_REG_HYBRID_DECODE3)? 5 : (((pOptions->dwH264RegKey & H264_REG_HYBRID_DECODE1) + (pOptions->dwH264RegKey & H264_REG_HYBRID_DECODE2))>>7);
        //Hybrid_Decoding = 1  :  SW/I                    &        HW/PB
        //Hybrid_Decoding = 2  :  SW/IP                &        HW/B
        IMGPAR SkipControlB = (pOptions->dwH264RegKey & H264_REG_DROP_FRAME) ? 1 : 0;
        IMGPAR bResidualDataFormat = (pOptions->dwH264RegKey & H264_REG_NVFORMAT) ? 1 : (pOptions->dwH264RegKey & H264_REG_INTELFORMAT) ? 2 : 0;
        //DP("This ResidualDataForamt is %d", IMGPAR bResidualDataFormat);
        IMGPAR set_dpb_buffer_size = (pOptions->dwH264RegKey & H264_REG_SET_DPBBUFSIZE) ? 1 : 0;        
#else
#if defined (_COLLECT_PIC_)
        stream_global->m_resize_width_height_reg = 0;
        stream_global->m_resize_width_height = 0;
#endif
        IMGPAR smart_dec = SMART_DEC_DISABLE;
        IMGPAR de_blocking_flag = 1;
        IMGPAR Hybrid_Decoding = 0;
        IMGPAR SkipControlB = 0;
#endif

#if defined(_HW_ACCEL_)
        imgDXVAVer = g_DXVAVer;        

        if(IviNotDxva != g_DXVAVer)
        {
            IMGPAR FP_Open                    = HW_Open;
            IMGPAR FP_Close                    = HW_Close;
            IMGPAR FP_BeginDecodeFrame        = HW_BeginDecodeFrame;
            IMGPAR FP_EndDecodeFrame        = HW_EndDecodeFrame;
            IMGPAR FP_ReleaseDecodeFrame    = HW_ReleaseDecodeFrame;
            IMGPAR FP_DeblockSlice            = HW_DeblockSlice;

            IMGPAR FP_ReadMV_16x16            = ReadMotionInfo16x16;
            IMGPAR FP_ReadMV_16x8            = ReadMotionInfo16x8;
            IMGPAR FP_ReadMV_8x16            = ReadMotionInfo8x16;
            IMGPAR FP_ReadMV_8x8            = ReadMotionInfo8x8;

            IMGPAR FP_ReadMV_Direct_Spatial        = mb_direct_spatial_mv_pred;
            IMGPAR FP_ReadMV_Direct_Temproal    = mb_direct_temporal_mv_pred;

            if (g_dwSmartDecLevel >= SMART_DEC_LEVEL_4 && g_dwSmartDecLevel != SMART_DEC_LEVEL_0)            
                IMGPAR smart_dec =     stream_global->m_dwSavedSmartDecLevel = g_dwSmartDecLevel = SMART_DEC_LEVEL_3;

            if(g_DXVAVer && g_DXVAMode==E_H264_DXVA_ATI_PROPRIETARY_E || g_DXVAMode==E_H264_DXVA_MODE_E || g_DXVAMode==E_H264_DXVA_INTEL_MODE_E) {
                g_pfnDecodePicture = DecodePicture_HW_BA;
            } else {
#ifdef IP_RD_MERGE    
                g_pfnDecodePicture = DecodePicture_MultiThread_SingleSlice_IPRD_Merge;
#else
                g_pfnDecodePicture = DecodePicture_MultiThread_SingleSlice_IPRD_Seperate;
#endif
                //g_pfnDecodePicture = DecodePicture_MultiThread_MultiSlice;
            }
        }
        else
#endif
        {
            IMGPAR FP_Open                    = SW_Open;
            IMGPAR FP_Close                    = SW_Close;
            IMGPAR FP_BeginDecodeFrame        = SW_BeginDecodeFrame;
            IMGPAR FP_EndDecodeFrame        = SW_EndDecodeFrame;
            IMGPAR FP_ReleaseDecodeFrame    = SW_ReleaseDecodeFrame;
            IMGPAR FP_DeblockSlice            = DeblockSlice;

            IMGPAR FP_ReadMV_16x16            = ReadMotionInfo16x16;
            IMGPAR FP_ReadMV_16x8            = ReadMotionInfo16x8;
            IMGPAR FP_ReadMV_8x16            = ReadMotionInfo8x16;
            IMGPAR FP_ReadMV_8x8            = ReadMotionInfo8x8;

            IMGPAR FP_ReadMV_Direct_Spatial        = mb_direct_spatial_mv_pred;
            IMGPAR FP_ReadMV_Direct_Temproal    = mb_direct_temporal_mv_pred;

#if !defined (_COLLECT_PIC_)
            g_pfnDecodePicture = DecodePicture_SingleThread;
#else
#ifdef IP_RD_MERGE    
            g_pfnDecodePicture = DecodePicture_MultiThread_SingleSlice_IPRD_Merge;
#else
            g_pfnDecodePicture = DecodePicture_MultiThread_SingleSlice_IPRD_Seperate;
#endif
            //g_pfnDecodePicture = DecodePicture_MultiThread_MultiSlice;
#endif
        }

        IMGPAR dwFillFrameNumGap = pOptions->dwFillFrameNumGap;

        IMGPAR PrevPicOrderCntLsb = 0;
        IMGPAR PrevPicOrderCntMsb = 0;

        //For NV_SECOP
        IMGPAR dwSliceCountForSECOP = 0;

#ifdef _COLLECT_PIC_
    }
#endif
    //~KevinChien 10/11/2005

    g_Initial_Flag = TRUE;

#if (!defined(__linux__)) && defined(_HW_ACCEL_)
    IMGPAR FP_Open ARGS4(qsize, pOptions->uiH264VGACard, pOptions->uiH264DXVAMode, pOptions->pIviCP);
#else
    IMGPAR FP_Open ARGS3(qsize, pOptions->uiH264VGACard, pOptions->uiH264DXVAMode);
#endif

#if defined(_HW_ACCEL_)
    stream_global->frame_execute_count = HW_BUF_SIZE;
#endif
    
    Reset ARGS0();

#ifdef _COLLECT_PIC_
    unsigned int    thread_id_pic_ip;
    unsigned int    thread_id_pic_b0, thread_id_pic_b1;

#ifdef DEBUG_SHOW_PROCESS_TIME
#ifdef WIN32
    _ftime (&(IMGPAR tstruct_start));             // start time ms
#else
    ftime (&(IMGPAR tstruct_start));              // start time ms
#endif
    time( &(IMGPAR ltime_start));                // start time s

    int tmp_time_img = (IMGPAR ltime_start*1000+IMGPAR tstruct_start.millitm);
#endif
    g_event_exit_flag = 0;

    stream_global->m_event_read_start_ip = (HANDLE) CreateEvent(NULL, true, false, NULL);
    stream_global->m_event_read_finish_ip = (HANDLE) CreateEvent(NULL, true, false, NULL);
    stream_global->m_event_for_field_ip = (HANDLE) CreateEvent(NULL, true, false, NULL);

    for (i=0; i<2; i++) //for read b
    {
        stream_global->m_event_read_start_b[i] = (HANDLE) CreateEvent(NULL, true, false, NULL);
        stream_global->m_event_read_finish_b[i] = (HANDLE) CreateEvent(NULL, true, false, NULL);
    }
    read_handle_ip = (HANDLE) _beginthreadex(NULL, 0, decode_picture_read_ip, (void*)stream_global, 0, &(thread_id_pic_ip));
    read_handle_b0 = (HANDLE) _beginthreadex(NULL, 0, decode_picture_read_b0, (void*)stream_global, 0, &(thread_id_pic_b0));
    read_handle_b1 = (HANDLE) _beginthreadex(NULL, 0, decode_picture_read_b1, (void*)stream_global, 0, &(thread_id_pic_b1));

    stream_global->m_event_decode_start_ip = (HANDLE) CreateEvent(NULL, true, false, NULL);
    stream_global->m_event_decode_finish_ip = (HANDLE) CreateEvent(NULL, true, false, NULL);

    for (i=0; i<2; i++) //for decode ipb
    {
        stream_global->m_event_decode_start_b[i] = (HANDLE) CreateEvent(NULL, true, false, NULL);
        stream_global->m_event_decode_finish_b[i] = (HANDLE) CreateEvent(NULL, true, false, NULL);
    }
    decode_handle_ip = (HANDLE) _beginthreadex(NULL, 0, decode_picture_decode_ip, (void*)stream_global, 0, &(thread_id_pic_ip));
    decode_handle_b0 = (HANDLE) _beginthreadex(NULL, 0, decode_picture_decode_b0, (void*)stream_global, 0, &(thread_id_pic_b0));
    decode_handle_b1 = (HANDLE) _beginthreadex(NULL, 0, decode_picture_decode_b1, (void*)stream_global, 0, &(thread_id_pic_b1));

    event_RB_1stfield_decode_complete = (HANDLE) CreateEvent(NULL, true, false, NULL);
    event_RB_2ndfield_read_complete = (HANDLE) CreateEvent(NULL, true, false, NULL);
    event_RB_wait_clear = (HANDLE) CreateEvent(NULL, true, false, NULL);

    stream_global->m_event_decode_thread = (HANDLE) CreateEvent(NULL, true, false, NULL);
    stream_global->m_event_begin_decode_thread = (HANDLE) CreateEvent(NULL, true, false, NULL);
    stream_global->m_event_finish_decode_thread = (HANDLE) CreateEvent(NULL, true, false, NULL);

    unsigned int decode_thread_id;
    stream_global->m_handle_decode_thread = (HANDLE) _beginthreadex(NULL, 0, decode_thread, (void*)stream_global, 0, &(decode_thread_id));

    stream_global->m_event_start_pic_RB = (HANDLE) CreateEvent(NULL, true, false, NULL);
    stream_global->m_event_finish_pic_RB = (HANDLE) CreateEvent(NULL, true, false, NULL);

    stream_global->m_handle_decode_picture_RB = (HANDLE) _beginthreadex(NULL, 0, decode_picture_RB, (void*)stream_global, 0, &(decode_thread_id));

    for (i=0; i<6; i++)
    {
        stream_global->m_event_start_slice[i] = (HANDLE) CreateEvent(NULL, true, false, NULL);
        stream_global->m_event_end_slice[i] = (HANDLE) CreateEvent(NULL, true, false, NULL);
    }

    unsigned int thread_id_slice_0, thread_id_slice_1, thread_id_slice_2, thread_id_slice_3, thread_id_slice_4, thread_id_slice_5;
    decode_slice_handle_0 = (HANDLE) _beginthreadex(NULL, 0, decode_slice_0, (void*)stream_global, 0, &thread_id_slice_0);
    decode_slice_handle_1 = (HANDLE) _beginthreadex(NULL, 0, decode_slice_1, (void*)stream_global, 0, &thread_id_slice_1);
    decode_slice_handle_2 = (HANDLE) _beginthreadex(NULL, 0, decode_slice_2, (void*)stream_global, 0, &thread_id_slice_2);
    decode_slice_handle_3 = (HANDLE) _beginthreadex(NULL, 0, decode_slice_3, (void*)stream_global, 0, &thread_id_slice_3);
    decode_slice_handle_4 = (HANDLE) _beginthreadex(NULL, 0, decode_slice_4, (void*)stream_global, 0, &thread_id_slice_4);
    decode_slice_handle_5 = (HANDLE) _beginthreadex(NULL, 0, decode_slice_5, (void*)stream_global, 0, &thread_id_slice_5);

    stream_global->hFinishDisplaySemaphore = CreateSemaphore(NULL, 0, 1, NULL);

    /*HANDLE        currentProcess = GetCurrentProcess();
    DWORD_PTR    ProcessAffinityMask;
    DWORD_PTR    SystemAffinityMask;

    GetProcessAffinityMask( currentProcess, &ProcessAffinityMask, &SystemAffinityMask );
    if ((ProcessAffinityMask & 0x08))
    {
    SetProcessAffinityMask( GetCurrentThread(), (ProcessAffinityMask&0x01));
    SetThreadAffinityMask(  decode_mb_handle_ip, (ProcessAffinityMask&0x01));
    //SetThreadAffinityMask(  decode_mb_handle_b, (ProcessAffinityMask&0x04));
    SetThreadAffinityMask(  decode_mb_handle_ip1, (ProcessAffinityMask&0x04));
    }
    else if (ProcessAffinityMask & 0x02)
    {
    SetProcessAffinityMask( GetCurrentThread(), (ProcessAffinityMask&0x01));
    SetThreadAffinityMask(  decode_mb_handle_ip, (ProcessAffinityMask&0x01));
    //SetThreadAffinityMask(  decode_mb_handle_b, (ProcessAffinityMask&0x02));
    SetThreadAffinityMask(  decode_mb_handle_ip1, (ProcessAffinityMask&0x02));
    }*/

    next_image_type = -1;
    g_bNextImgForRef = 1;

    stream_global->m_bDoSkipPB = FALSE;
    stream_global->m_bSeekIDR = FALSE;
    stream_global->m_bRefBWaitB = FALSE;
    stream_global->m_bRefBWaitP = FALSE;
    stream_global->m_bRefBWaitBRead = FALSE;
#endif

#if defined(_SHOW_THREAD_TIME_)
    QueryPerformanceFrequency(&stream_global->t_freq);
#endif

#ifdef _COLLECT_PIC_
    stream_global->m_bTrickEOS = false;
#endif

    OPEN_RECORD_FILE;

    g_bNormalSpeed = TRUE;

    g_bSkipFirstB = -3;

    g_bDisplayed = FALSE;

    g_dwYCCBufferLen = 0;

    //Initial pts for frames have no pts
    g_pts.ts = 333667;
    g_pts.freq = 10000000;
    g_pts.tslength = 64;
    //g_pts.flags = 2;
    //g_pts.unused1 = 205;
    //g_pts.unused2 = 205;

    m_bSetNormalSpeed = TRUE;
    stream_global->m_SPSchecked = FALSE;

    return S_OK;
}

#ifdef _COLLECT_PIC_
#ifndef IP_RD_MERGE
void FlushAllFrames PARGS0()
{
    DEBUG_SHOW_SW_INFO("Flush All Frames");
    int nImgIP_d, nImgB_rd0, nImgB_rd1;
    StreamParameters *stream_global = IMGPAR stream_global;

    while (stream_global->bHasB && stream_global->b_Count)
    {
        if (stream_global->nNeedToDecodeIP)
        {
            nImgIP_d = !(stream_global->pic_ip_num);
            gImgIP_d = img_array[nImgIP_d];
            SetEvent(stream_global->m_event_decode_start_ip);
        }

        nImgB_rd0 = (stream_global->nSwitchB<<1) + 2;
        nImgB_rd1 = (stream_global->nSwitchB<<1) + 3;

        if (stream_global->b_Count == 3 && stream_global->two_B_Flag)
        {
            gImgB_r1 = img_array[nImgB_rd1];
            SetEvent(stream_global->m_event_read_start_b[1]);
        }
        if (stream_global->b_Count <= 2)
        {
            gImgB_r0 = img_array[nImgB_rd0];
            SetEvent(stream_global->m_event_read_start_b[0]);
            if (stream_global->two_B_Flag)
            {
                gImgB_r1 = img_array[nImgB_rd1];
                SetEvent(stream_global->m_event_read_start_b[1]);
            }
        }

        WaitForSingleObject(stream_global->m_event_decode_finish_b[0], INFINITE);
        if (stream_global->two_B_Flag)
            WaitForSingleObject(stream_global->m_event_decode_finish_b[1], INFINITE);
        ResetEvent(stream_global->m_event_decode_finish_b[0]);
        ResetEvent(stream_global->m_event_decode_finish_b[1]);

        stream_global->nSwitchB = !(stream_global->nSwitchB);
        stream_global->b_Count -= 2;

        if (stream_global->nNeedToDecodeIP)
        {
            SetEvent(stream_global->m_event_for_field_ip);
            WaitForSingleObject(stream_global->m_event_decode_finish_ip, INFINITE);
            ResetEvent(stream_global->m_event_decode_finish_ip);
            stream_global->nNeedToDecodeIP = 0;
        }

        if (stream_global->b_Count >= 2)
        {
            stream_global->bHasB = 1;
            if (stream_global->b_Count & 1)
                stream_global->two_B_Flag = 0;
            else
                stream_global->two_B_Flag = 1;
        }
        else
            stream_global->bHasB = 0;
    }

    if (stream_global->nNeedToDecodeIP)
    {
        nImgIP_d = !(stream_global->pic_ip_num);
        gImgIP_d = img_array[nImgIP_d];
        SetEvent(stream_global->m_event_decode_start_ip);
        SetEvent(stream_global->m_event_for_field_ip);
        WaitForSingleObject(stream_global->m_event_decode_finish_ip, INFINITE);
        ResetEvent(stream_global->m_event_decode_finish_ip);
        stream_global->nNeedToDecodeIP = 0;
    }

    if (stream_global->b_Count>0)
    {
        if (stream_global->b_Count & 1)
            stream_global->two_B_Flag = 0;
        else
            stream_global->two_B_Flag = 1;

        nImgB_rd0 = (stream_global->nSwitchB<<1) + 2;
        nImgB_rd1 = (stream_global->nSwitchB<<1) + 3;

        gImgB_r0 = img_array[nImgB_rd0];
        SetEvent(stream_global->m_event_read_start_b[0]);
        if (stream_global->two_B_Flag)
        {
            gImgB_r1 = img_array[nImgB_rd1];
            SetEvent(stream_global->m_event_read_start_b[1]);
        }

        WaitForSingleObject(stream_global->m_event_decode_finish_b[0], INFINITE);
        if (stream_global->two_B_Flag)
            WaitForSingleObject(stream_global->m_event_decode_finish_b[1], INFINITE);
        ResetEvent(stream_global->m_event_decode_finish_b[0]);
        ResetEvent(stream_global->m_event_decode_finish_b[1]);

        stream_global->nSwitchB = !(stream_global->nSwitchB);
    }

    stream_global->is_first = 1;
    stream_global->bHasB = 0;
    stream_global->b_Count = 0;
    stream_global->nSwitchB = 0;
    stream_global->two_B_Flag = 0;
    stream_global->nNeedToDecodeIP = 0;
    stream_global->nCollectB = 0;

}
#else 
// Start over Merge IP read & decode
void FlushAllFrames PARGS0()
{
    DEBUG_SHOW_SW_INFO("Flush All Frames");
    int nImgB_rd0, nImgB_rd1;
    StreamParameters *stream_global = IMGPAR stream_global;

    gImgB_r0 = img_array[2];
    WaitForSingleObject(stream_global->m_event_decode_finish_b[0], INFINITE);
    stream_global->b_Count--;


    if ( stream_global->b_Count ) {
        gImgB_r0 = img_array[3];
        WaitForSingleObject(stream_global->m_event_decode_finish_b[1], INFINITE);
        stream_global->b_Count--;        
    }

    ResetEvent(stream_global->m_event_decode_finish_b[0]);
    ResetEvent(stream_global->m_event_decode_finish_b[1]);

    //stream_global->is_first = 1;
    stream_global->bHasB = 0;
    //stream_global->b_Count = 0;
    stream_global->nSwitchB = 0;
    stream_global->two_B_Flag = 0;
    stream_global->nNeedToDecodeIP = 0;
    stream_global->nCollectB = 0;


}
#endif
#endif

#if defined (_USE_SCRAMBLE_DATA_)
STDMETHODIMP CH264VDecHP::OpenKey(IN H264VDec_OnOpenKeyFcn pCallBack, IN LPVOID pParam)
{
    //unsigned char reqData[16], respData[16];

    //g_bIsmpEnabled = false;

    /*if(!pCallBack)
    return FALSE;

    BOOL bRet = IsmpInput_Open(&g_IsmpData, reqData, ISMP_STMTYPE_EXAMPLE2);

    if(bRet)
    bRet = pCallBack(pParam, reqData, ISMP_STMTYPE_EXAMPLE2, respData) == S_OK ? TRUE : FALSE;

    if(bRet)
    bRet = IsmpInput_VerifyResponse(&g_IsmpData, respData);

    g_bIsmpEnabled = bRet;*/

    //#ifdef _DEBUG
    //    if(bRet)
    //        Beep(440, 250);
    //#endif

    return E_FAIL;//bRet ? S_OK: E_FAIL;
}
#endif

int CH264VDecHP::Reset PARGS0()
{
    unsigned int i;
    DEBUG_SHOW_SW_INFO("CH264VDecHP::Reset()");

#if defined(_HW_ACCEL_)
#if defined(_USE_QUEUE_FOR_DXVA2_) || defined(_USE_QUEUE_FOR_DXVA1_)
    if(g_DXVAVer && (g_DXVAMode!=E_H264_DXVA_ATI_PROPRIETARY_E && g_DXVAMode!=E_H264_DXVA_MODE_E && g_DXVAMode!=E_H264_DXVA_NVIDIA_PROPRIETARY_A && g_DXVAMode!=E_H264_DXVA_INTEL_MODE_E))
    {
        //reset queue
        stream_global->m_dxva_queue_reset = 1;
        ReleaseSemaphore(stream_global->m_queue_semaphore, 1, NULL);
        WaitForSingleObject(stream_global->h_dxva_queue_reset_done, INFINITE);
        i = 20;
        while(i)
            SetEvent(stream_global->hReadyForRender[--i]);
    }
#endif
#endif

#ifdef _COLLECT_PIC_        
    //Set Default
    if(g_DXVAVer && g_DXVAMode==E_H264_DXVA_ATI_PROPRIETARY_E || g_DXVAMode==E_H264_DXVA_MODE_E || g_DXVAMode==E_H264_DXVA_INTEL_MODE_E) {
        g_pfnDecodePicture = DecodePicture_HW_BA;
    } else {
#ifdef IP_RD_MERGE    
        g_pfnDecodePicture = DecodePicture_MultiThread_SingleSlice_IPRD_Merge;
#else
        g_pfnDecodePicture = DecodePicture_MultiThread_SingleSlice_IPRD_Seperate;
#endif
        //g_pfnDecodePicture = DecodePicture_MultiThread_MultiSlice;
    }

    int nIMG_num = (stream_global->m_bIsSingleThreadMode == TRUE)? 1: IMG_NUM;

    for(i=0; i<nIMG_num; i++)
    {
        img = img_array[i];
#endif
        dec_picture = NULL;
        IMGPAR m_dec_picture_top = NULL;
        IMGPAR m_dec_picture_bottom = NULL;
        g_dwSkipFrameCounter = 0;
        streamptscounter = 0;
        IMGPAR number = 0;
        IMGPAR type = I_SLICE;
        IMGPAR dec_ref_pic_marking_buffer = 0;
        IMGPAR current_slice_nr = 0;
        IMGPAR current_mb_nr_r = IMGPAR current_mb_nr_d = -4711;                // initialized to an impossible value for debugging -- correct value is taken from slice header
        IMGPAR error_mb_nr = -4711;
#if defined(_COLLECT_PIC_)
        IMGPAR m_pic_combine_status = -1;
#endif
        IMGPAR SkipThisFrame = 0;
#if !defined(_COLLECT_PIC_)
        IMGPAR currentSlice->next_header = -8888;    // initialized to an impossible value for debugging -- correct value is taken from slice header
#endif
        IMGPAR num_dec_mb = 0;
        IMGPAR do_co_located = 0;

        init_old_slice ARGS0();

#if !defined(_COLLECT_PIC_)
        m_active_pps[0].Valid = NULL;
        m_active_pps[1].Valid = NULL;
        m_active_sps[0].Valid = NULL;
        m_active_sps[1].Valid = NULL;
#else
        img->m_active_pps[0].Valid = NULL;
        img->m_active_pps[1].Valid = NULL;
        img->m_active_sps[0].Valid = NULL;
        img->m_active_sps[1].Valid = NULL;
#endif

        g_bEOS = FALSE;

#ifdef _COLLECT_PIC_
        IMGPAR slice_number = 0;
        nalu->buf = IMGPAR ori_nalu_buf;
    }

	nalu_global_available = 0;
	stream_global->number = 0;
	stream_global->pic_ip_num = 0;
	stream_global->pic_b_num = 0;
	stream_global->is_first = 1;
	stream_global->bHasB = 0;
	stream_global->b_Count = 0;
	stream_global->nSwitchB = 0;
	stream_global->two_B_Flag = 0; //two b frames or not
	stream_global->nNeedToDecodeIP = 0;
	stream_global->nCollectB = 0;
	stream_global->last_has_mmco_5 = 0;
	for ( i = 0; i < stream_global->num_views; i++) {
		stream_global->m_active_sps_on_view[i]= 0;
	}
	next_image_type = -1;

	stream_global->pre_frame_num = 0;
	for ( i = 0; i<MAX_NUM_VIEWS; i++)
	{
		stream_global->PreviousFrameNum[i] = 0;
		stream_global->PreviousFrameNumOffset[i] = 0;
		stream_global->PreviousPOC[i] = 0;    
		stream_global->ThisPOC[i] = 0;
		stream_global->PrevPicOrderCntLsb[i] = 0;
		stream_global->PrevPicOrderCntMsb[i] = 0;
	}
	stream_global->last_has_mmco_5 = 0;	
	stream_global->profile_idc = 0;
	g_bReceivedFirst = 0;
	stream_global->m_bTrickEOS = false;
	stream_global->bSeekToOpenGOP = 0;
#if defined(_HW_ACCEL_)
    stream_global->frame_execute_count = HW_BUF_SIZE;
#endif
    stream_global->m_iExecute_Of_I_Frame = FALSE;
    stream_global->m_is_MTMS = -1;

#endif

    for(i = 0; i< MAXPPS; i++)
    {
        if (PicParSet[i].Valid == TRUE && PicParSet[i].slice_group_id != NULL)
        {
            _aligned_free (PicParSet[i].slice_group_id);
            PicParSet[i].slice_group_id = NULL;
        }
    }

    g_has_pts = 0;
    g_framerate1000 = 29970;
    //g_llDispCount = 0;
    //g_llDispPulldownCount = 0;
    memset(g_llDispCount, 0, 16 * sizeof(unsigned __int64));
    memset(g_llDispPulldownCount, 0, 16 * sizeof(unsigned __int64));
    memset(g_uSkipBFrameCounter, 0, MAX_NUM_VIEWS * 2 * sizeof(unsigned int));
    g_PulldownRate = 1000;
    g_pulldown = FALSE;
    memset(&g_ref_pts, 0, sizeof(H264_TS));
    memset(g_llDtsCount, 0, MAX_NUM_VIEWS * sizeof(unsigned __int64));
    memset(g_llDecCount, 0, MAX_NUM_VIEWS * sizeof(unsigned __int64));

    g_bSkipFirstB = -3;

    g_pCCDec->Reset();

    g_bDisplayed = FALSE;

    g_dwYCCBufferLen = 0;

    memset(stream_global->m_pbValidViews, 0, 16 * sizeof(BOOL));
    stream_global->m_CurrOutputViewIndex = -1;
    stream_global->m_iPOCofDroppedFrame = 0x80000000;
    stream_global->m_iFrameNeedtoSkip = 0;

    DEBUG_SHOW_SW_INFO("CH264VDecHP::Reset() END");

    return 0;
}




HRESULT CH264VDecHP::Close PARGS0()
{
    DEBUG_INFO("CH264VDecHP::Close()");

#if !defined(_COLLECT_PIC_)
    CREL_RETURN ret;
    IMGPAR FP_Close ARGS0();
    Stop ARGS1(0);

    FmoFinit ARGS0();
    free_global_buffers ARGS0();

    ret = flush_dpb ARGS1(0);

#ifdef PAIR_FIELDS_IN_OUTPUT
    flush_pending_output(p_out);
#endif

    if(p_out!=-1)
        close(p_out);
    //  fclose(p_out2);
    if(p_ref!=-1)
        close(p_ref);

    free_dpb ARGS1(0);
    uninit_out_buffer ARGS0();

    if(Co_located_MB)
    {
        free_colocated_MB(Co_located_MB);
        Co_located_MB = 0;
    }
    if(nalu)
        FreeNALU(nalu);
    if(g_framemgr)
    {
        delete g_framemgr;
        g_framemgr = 0;
    }

#else

    unsigned int i;

    Stop ARGS1(0);

    free_dpb ARGS1(0);        //Only base view, to revise!

    int nIMG_num = (stream_global->m_bIsSingleThreadMode == TRUE)? 1: IMG_NUM;

    for (i=0; i<nIMG_num; i++)
    {
        img = img_array[i];
        IMGPAR FP_Close ARGS0();
        if (global_init_done)
        {
            FmoFinit ARGS0();
            free_global_buffers ARGS0();

            if(Co_located_MB)
            {
                free_colocated_MB(Co_located_MB);
                Co_located_MB = 0;
            }        
        }
        if(nalu)
            FreeNALU(nalu);
        //_aligned_free(img);
    }
    //_aligned_free(img_array);

    

#ifdef PAIR_FIELDS_IN_OUTPUT
    flush_pending_output(p_out);
#endif

    if(p_out!=-1)
        close(p_out);
    //  fclose(p_out2);
    if(p_ref!=-1)
        close(p_ref);

    uninit_out_buffer ARGS0();

    if(g_framemgr)
    {
        delete g_framemgr;
        g_framemgr = 0;
    }

    if(nalu_global)
        FreeNALU(nalu_global);

    delete g_pCCDec;
    g_pCCDec = NULL;

    //Force to call _endthreadex()    
    g_event_exit_flag = 1;
    SetEvent(stream_global->m_event_read_start_ip);    
    WaitForSingleObject(stream_global->m_event_read_finish_ip, INFINITE);
    CloseHandle(stream_global->m_event_read_start_ip);
    CloseHandle(stream_global->m_event_read_finish_ip);    
    for (i=0; i<2; i++)
    {
        SetEvent(stream_global->m_event_read_start_b[i]);    
        WaitForSingleObject(stream_global->m_event_read_finish_b[i], INFINITE);
        CloseHandle(stream_global->m_event_read_start_b[i]);
        CloseHandle(stream_global->m_event_read_finish_b[i]);    
    }
    WaitForSingleObject(read_handle_ip, INFINITE);
    WaitForSingleObject(read_handle_b0, INFINITE);
    WaitForSingleObject(read_handle_b1, INFINITE);
    CloseHandle(read_handle_ip);    
    CloseHandle(read_handle_b0);    
    CloseHandle(read_handle_b1);    

    CloseHandle(stream_global->m_event_for_field_ip);

    g_event_exit_flag = 1;
    SetEvent(stream_global->m_event_decode_start_ip);
    WaitForSingleObject(stream_global->m_event_decode_finish_ip, INFINITE);
    CloseHandle(stream_global->m_event_decode_start_ip);
    CloseHandle(stream_global->m_event_decode_finish_ip);
    for (i=0; i<2; i++)
    {
        SetEvent(stream_global->m_event_decode_start_b[i]);
        WaitForSingleObject(stream_global->m_event_decode_finish_b[i], INFINITE);
        CloseHandle(stream_global->m_event_decode_start_b[i]);
        CloseHandle(stream_global->m_event_decode_finish_b[i]);
    }
    WaitForSingleObject(decode_handle_ip, INFINITE);
    WaitForSingleObject(decode_handle_b0, INFINITE);
    WaitForSingleObject(decode_handle_b1, INFINITE);
    CloseHandle(decode_handle_ip);
    CloseHandle(decode_handle_b0);
    CloseHandle(decode_handle_b1);

    CloseHandle(event_RB_1stfield_decode_complete);
    CloseHandle(event_RB_2ndfield_read_complete);
    CloseHandle(event_RB_wait_clear);

    SetEvent(stream_global->m_event_begin_decode_thread);
    WaitForSingleObject(stream_global->m_event_finish_decode_thread, INFINITE);
    WaitForSingleObject(stream_global->m_handle_decode_thread, INFINITE);
    CloseHandle(stream_global->m_handle_decode_thread);
    CloseHandle(stream_global->m_event_decode_thread);
    CloseHandle(stream_global->m_event_begin_decode_thread);
    CloseHandle(stream_global->m_event_finish_decode_thread);

    SetEvent(stream_global->m_event_start_pic_RB);
    WaitForSingleObject(stream_global->m_event_finish_pic_RB, INFINITE);
    WaitForSingleObject(stream_global->m_handle_decode_picture_RB, INFINITE);
    CloseHandle(stream_global->m_handle_decode_picture_RB);
    CloseHandle(stream_global->m_event_start_pic_RB);
    CloseHandle(stream_global->m_event_finish_pic_RB);

    for (i=0; i<6; i++)
    {
        SetEvent(stream_global->m_event_start_slice[i]);
        WaitForSingleObject(stream_global->m_event_end_slice[i], INFINITE);
        CloseHandle(stream_global->m_event_start_slice[i]);
        CloseHandle(stream_global->m_event_end_slice[i]);
    }

    WaitForSingleObject(decode_slice_handle_0, INFINITE);
    WaitForSingleObject(decode_slice_handle_1, INFINITE);
    WaitForSingleObject(decode_slice_handle_2, INFINITE);
    WaitForSingleObject(decode_slice_handle_3, INFINITE);
    WaitForSingleObject(decode_slice_handle_4, INFINITE);
    WaitForSingleObject(decode_slice_handle_5, INFINITE);
    CloseHandle(decode_slice_handle_0);
    CloseHandle(decode_slice_handle_1);
    CloseHandle(decode_slice_handle_2);
    CloseHandle(decode_slice_handle_3);
    CloseHandle(decode_slice_handle_4);
    CloseHandle(decode_slice_handle_5);

    CloseHandle(stream_global->hFinishDisplaySemaphore);

    DeleteCriticalSection( &stream_global->crit_dyna_expand );
    DeleteCriticalSection( &stream_global->m_csExit_MB );
	DeleteCriticalSection( &stream_global->m_csOffsetMetadata);

    for (i=0; i<nIMG_num; i++)
        _aligned_free(img_array[i]);

    _aligned_free(img_array);

    if(stream_global->dpb_pre_alloc_views)
    {
        if(dpb.init_done)
            free(dpb.init_done);
        if(dpb.size_on_view)
            _aligned_free(dpb.size_on_view);
        if(dpb.used_size_on_view)
            _aligned_free(dpb.used_size_on_view);
        if(dpb.ref_frames_in_buffer_on_view)
            _aligned_free(dpb.ref_frames_in_buffer_on_view);
        if(dpb.fs_on_view)
            _aligned_free(dpb.fs_on_view);
        if(dpb.fs_ltref_on_view)
            _aligned_free(dpb.fs_ltref_on_view);
        if(dpb.fs_ref_on_view)
            _aligned_free(dpb.fs_ref_on_view);
        if(dpb.ltref_frames_in_buffer_on_view)
            _aligned_free(dpb.ltref_frames_in_buffer_on_view);
        if(dpb.max_long_term_pic_idx_on_view)
            _aligned_free(dpb.max_long_term_pic_idx_on_view);
		if(dpb.last_picture)
			_aligned_free(dpb.last_picture);
    }

	for ( i = 0; i < MAXSPS; i++) {
		FreeSPS_MVC_Related(&SeqParSet[i]);
		FreeSPS_MVC_Related(&SeqParSubset[i]);
	}

    _aligned_free(stream_global);
#endif

    CLOSE_RECORD_FILE;

    DEBUG_INFO("CH264VDecHP::Close() END");

    return S_OK;
}

HRESULT CH264VDecHP::DecodeFrame PARGS4(IN const H264VDecHP_DecodeOptions *pOptions,
                                                                                IN const DWORD dwSize,
                                                                                OUT DWORD *pdwNumberOfDisplayableFrames,
                                                                                OUT DWORD *pdwNumberOfSkippedFrames)
{ 
    int res;
    HRESULT hr = S_OK;
    H264VDecHP_DecodeOptions Options;

    if(pOptions==0)
        return E_FAIL;
    if(dwSize<sizeof(Options))
    {
        ZeroMemory(&Options,sizeof(Options));
        memcpy(&Options,pOptions,dwSize);
        pOptions = &Options;
    }

    EnterCriticalSection(&m_csDecode);
#if !defined(_COLLECT_PIC_)
    IMGPAR stop_indicator=0;
#else
    for (unsigned int i=0; i<IMG_NUM; i++)
    {
        stream_global->m_img[i]->stop_indicator=0;
    }
#endif

    DEBUG_INFO("CH264VDecHP::DecodeFrame\n m_nFlushDPBsize: %d  GetDisplayCount: %d  dpb_size: %d  GetFreeCount: %d", g_framemgr->m_nFlushDPBsize, g_framemgr->GetDisplayCount(), dpb.size_on_view[0], g_framemgr->GetFreeCount());

    //Set g_nNormalSpeed here to ensure that don't corrupt previous GOP status!!
    g_bNormalSpeed = m_bSetNormalSpeed;

    while(((g_framemgr->m_nFlushDPBsize && g_framemgr->GetDisplayCount() < (int) dpb.size_on_view[0]) || (g_framemgr->GetDisplayCount()<1)) && g_framemgr->GetFreeCount() >= 12 )
    {
#if defined(_HW_ACCEL_)
        if((g_pH264DXVA) && (g_framemgr->GetDisplayCount()>=2) && (((CH264DXVABase*)g_pH264DXVA)->GetHWBufCount()<3))
            break;
#endif
        res = g_pfnDecodePicture ARGS0();

        if(res<0 && g_bRewindDecoder==FALSE)
        {
            if (stream_global->nNeedToDecodeIP || stream_global->b_Count)
                FlushAllFrames ARGS0();

            if (g_bDisplayed == FALSE && dpb.used_size_on_view[0])        //Base view first, to revise
                output_one_frame_from_dpb ARGS1(0);

            stream_global->pic_ip_num = 0;
            stream_global->pic_b_num = 0;

            hr = E_FAIL;
            break;
        }
        else if (res == 2)
        {
            hr = ((HRESULT)0x00000002L);
        }

        if(g_framemgr->m_nFlushDPBsize)
            break;
    }
    *pdwNumberOfDisplayableFrames = g_framemgr->GetDisplayCount();
    *pdwNumberOfSkippedFrames = g_dwSkipFrameCounter;
    LeaveCriticalSection(&m_csDecode);

    DEBUG_INFO("End DecodeFrame");

#if defined(_MSC_VER) || defined(__INTEL_COMPILER)
    _asm emms;
#elif defined(__GNUC__) && defined(__i386__)
    __asm__ ("emms");
#endif

    return hr; 
}

HRESULT CH264VDecHP::GetFrame PARGS5(
                                                                         IN const H264VDecHP_GetFrameOptions *pOptions,
                                                                         IN const DWORD dwSize,
                                                                         OUT H264VDecHP_Frame *pFrame,
                                                                         IN const DWORD dwFrameSize,
                                                                         IN unsigned int view_index)
{
    int res;
    H264VDecHP_GetFrameOptions Options;    

    DEBUG_INFO("CH264VDecHP::GetFrame");

    if(dwSize<sizeof(Options))
    {
        ZeroMemory(&Options,sizeof(Options));
        memcpy(&Options,pOptions,dwSize);
        pOptions = &Options;
    }
    if(g_framemgr->GetDisplayCount()==0)
        return E_FAIL;

    // PERFORM_FRAME_POINTER_SCRAMBLING
    DWORD dwFrame = (DWORD)pFrame;
    pFrame = (H264VDecHP_Frame *) INLINE_DESCRAMBLE(dwFrame,SCRAMBLE_PARAM_1,SCRAMBLE_PARAM_2,SCRAMBLE_PARAM_3);

    res = GetPicture ARGS2(pFrame, view_index);    // possible to get non-existing frame!!!
    if (g_framemgr->m_nFlushDPBsize)
        g_framemgr->m_nFlushDPBsize--;

    DEBUG_INFO("CH264VDecHP::GetFrame End");

    return res<0 ? E_FAIL : S_OK;
}

BOOL CH264VDecHP::GetDisplayStatus PARGS1(
    IN const DWORD dwIdx
    )
{
    DEBUG_INFO("CH264VDecHP::GetDisplayStatus");

#if defined (_HW_ACCEL_)
#if defined(_USE_QUEUE_FOR_DXVA2_ ) || defined( _USE_QUEUE_FOR_DXVA1_)
    if(g_DXVAMode==E_H264_DXVA_MODE_A||g_DXVAMode==E_H264_DXVA_MODE_C||g_DXVAMode==E_H264_DXVA_ATI_PROPRIETARY_A)
        WaitForSingleObject(stream_global->hReadyForRender[dwIdx], INFINITE);
#endif
#endif

    DEBUG_INFO("CH264VDecHP::GetDisplayStatus End");

    return TRUE;
}

BOOL CH264VDecHP::FinishDisplay PARGS1(IN DWORD dwEstimateNextFrame)
{
    DEBUG_INFO("CH264VDecHP::FinishDisplay The EstimateNextFrame: %d", dwEstimateNextFrame);

#if defined (_HW_ACCEL_)
    long lCount;
    stream_global->m_iEstimateNextFrame = dwEstimateNextFrame;
    QueryPerformanceCounter(&stream_global->m_liRecodeTime0);

    ReleaseSemaphore(stream_global->hFinishDisplaySemaphore, 1, &lCount);
#endif

    DEBUG_INFO("CH264VDecHP::FinishDisplay End");
    return S_OK;
}

HRESULT CH264VDecHP::Stop PARGS1(IN const DWORD dwStopFlags)
{ 
    DEBUG_SHOW_SW_INFO("CH264VDecHP::Stop()");
#if !defined(_COLLECT_PIC_)
    unsigned i;
    CREL_RETURN ret;

    EnterCriticalSection(&m_csDecode);
    EndAnnexbNALUDecoding ARGS0();
    if(dpb.init_done)
    {
        if(dpb.used_size)
            ret = flush_dpb ARGS1(0);
        for(i=0; i<dpb.used_size; i++)
        {
            free_frame_store ARGS2(dpb.fs[i], -1);
            dpb.fs[i] = alloc_frame_store();
        }
        for(i=0; i<dpb.ref_frames_in_buffer; i++)
        {
            dpb.fs_ref[i] = 0;
        }
        for(i=0; i<dpb.ltref_frames_in_buffer; i++)
        {
            dpb.fs_ltref[i] = 0;
        }
        dpb.init_done = 0;
        dpb.used_size = 0;
        dpb.ref_frames_in_buffer = 0;
        dpb.ltref_frames_in_buffer = 0;
        dpb.last_picture = 0;
        dpb.last_output_poc = INT_MIN;
        dpb.max_long_term_pic_idx = -1;
    }

    //For seeking, we should release the dec_picture in the decoder.
    if(dec_picture)
        release_storable_picture ARGS2(dec_picture, 1);

    Reset ARGS0();
    LeaveCriticalSection(&m_csDecode);
    g_framemgr->FlushDisplayQueue(g_pH264DXVA);
    IMGPAR stop_indicator = 1;
    return S_OK; 

#else

    unsigned     int i;
    int            j;
    unsigned int nTemp;

    EnterCriticalSection(&m_csDecode);
    if (stream_global->nNeedToDecodeIP || stream_global->b_Count)
        FlushAllFrames ARGS0();

    int nIMG_num = (stream_global->m_bIsSingleThreadMode == TRUE)? 1: IMG_NUM;

    for (i=0; i<nIMG_num; i++)
    {
        img = img_array[i];
        EndAnnexbNALUDecoding ARGS0();
    }

    img = img_array[0];
    if(IMGPAR stop_indicator == 0)
    {
        if(g_DXVAVer && (g_DXVAMode==E_H264_DXVA_ATI_PROPRIETARY_E || g_DXVAMode==E_H264_DXVA_MODE_E || g_DXVAMode==E_H264_DXVA_INTEL_MODE_E) && IMGPAR UnCompress_Idx!=-1) 
        {
            DEBUG_SHOW_SW_INFO("[CH264VDecHP::Stop()] UnCompress_Idx: %d pic_store_idx: %d", IMGPAR UnCompress_Idx, dec_picture->pic_store_idx);

			/* For bug#90718.
			   Here we manually release un-compression buffer which is not yet been put into three of them:
			    1) dpb buffer
				2) out_buffer ... this is temp buffer for direct output B case
				3) m_displayqueue

			   We return un-compression buffers of the reset cases through:
			    1) flush_dpb()  ... out_buffer also flush inside
				2) FlushDisplayQueue()

			   We assign "UnCompress_Idx" when we get un-compression buffer, and reset "UnCompress_Idx" to -1 when collect whole frame or a field after BA_ExecuteBuffers().
			   Thus, a value "Uncompress_Idx != -1" when stop invoked means decoder experience an unexpected interrupt while parsing bitstream data (ex: seek, change title).

			   The following if conditions are those cases we need to manual release un-compression buffer.
			    Case 1): Frame coding while stop invoked during GetItem() ~ collecting whole frame.
			    Case 2): Field coding while stop invoked during GetItem() ~ collecting the whole first field.
			*/
			int view_index = (stream_global->bMVC && IMGPAR firstSlice)? IMGPAR firstSlice->viewIndex: 0;
			if (dec_picture->structure == FRAME ||													  //Frame coding before stored in dpb buffer and display queue when call stop.
				(dec_picture->structure != FRAME && (dpb.last_picture[view_index]==0 && out_buffer->is_used==0))) //Field coding before stored in dpb buffer, out_buffer (for direct output B), and display queue when call stop.
			{
				((CH264DXVABase*)g_pH264DXVA)->m_pUncompBufQueue->PutItem(IMGPAR UnCompress_Idx);
			}
			dec_picture->pic_store_idx = IMGPAR UnCompress_Idx = -1;
			((CH264DXVABase*)g_pH264DXVA)->BA_ResetLastCompBufStaus();
		}

        for(int view_index = 0; view_index < stream_global->num_views; view_index++)
        {

			if(dpb.size_on_view)
			{
				nTemp = dpb.size_on_view[view_index];    //Base view first, to revise

				if(dpb.used_size_on_view[view_index])        //Base voew first, to revise
				{
					for (unsigned int i=0; i <storable_picture_count; i++)                
						storable_picture_map[i]->used_for_first_field_reference = 0;

					flush_dpb ARGS1(view_index);
				}
				for(i=0; i<nTemp; i++)
				{
					if(img->m_dec_picture != NULL && img->m_dec_picture->frame != NULL)
						free_frame_store ARGS2(dpb.fs_on_view[view_index][i], img->m_dec_picture->frame->pic_store_idx);
					else
						free_frame_store ARGS2(dpb.fs_on_view[view_index][i], -1);
					//dpb.fs[i] = alloc_frame_store();
				}
				for(i=0; i<dpb.ref_frames_in_buffer_on_view[view_index]; i++)
				{
					dpb.fs_ref_on_view[view_index][i] = 0;
				}
				for(i=0; i<dpb.ltref_frames_in_buffer_on_view[view_index]; i++)
				{
					dpb.fs_ltref_on_view[view_index][i] = 0;
				}

				dpb.size_on_view[view_index] = 0;
				dpb.num_ref_frames = 0;

				dpb.init_done[view_index] = 0;
				dpb.used_size_on_view[view_index] = 0;
				dpb.ref_frames_in_buffer_on_view[view_index] = 0;
				dpb.ltref_frames_in_buffer_on_view[view_index] = 0;
				dpb.last_picture[view_index] = 0;
				dpb.last_output_poc = INT_MIN;
				dpb.max_long_term_pic_idx_on_view[view_index] = -1;
			}
        }
    }

    for (i=0; i<nIMG_num; i++)
    {
        img = img_array[i];
        if (IMGPAR slice_number > 0)
        {
            IMGPAR currentSlice = IMGPAR firstSlice;
            for (j=0; j< IMGPAR slice_number; j++)
            {
                //active_pps = &PicParSet[IMGPAR currentSlice->pic_parameter_set_id];
                memcpy(&active_pps, &PicParSet[IMGPAR currentSlice->pic_parameter_set_id], sizeof(pic_parameter_set_rbsp_t));
                IMGPAR prevSlice = IMGPAR currentSlice;
                IMGPAR currentSlice = (Slice*) IMGPAR currentSlice->nextSlice;
                free_ref_pic_list_reordering_buffer(IMGPAR prevSlice);
                free_new_slice(IMGPAR prevSlice);
            }
            IMGPAR firstSlice = IMGPAR currentSlice = IMGPAR prevSlice = NULL;
            IMGPAR slice_number = 0;
        }
        //For seeking, we should release the dec_picture in the decoder.
        if(dec_picture)
        {
            release_storable_picture ARGS2(dec_picture, 1);
            dec_picture = NULL;
        }
    }

    Reset ARGS0();

    CLOSE_RECORD_FILE;

    OPEN_RECORD_FILE;

    LeaveCriticalSection(&m_csDecode);

    g_framemgr->FlushDisplayQueue(g_pH264DXVA);

    IMGPAR stop_indicator = 1;
    stream_global->m_SPSchecked = FALSE;

    Sleep(30);

    DEBUG_SHOW_SW_INFO("CH264VDecHP::Stop() END");

    return S_OK; 
#endif
}

ULONG CH264VDecHP::ReleaseFrame PARGS1(IN DWORD dwCookie)
{ 

    int nRefCount;
    if(dwCookie==0)
        return -1;
    int nPicStoreIdx = reinterpret_cast<CH264VideoFrame *>(dwCookie)->pic_store_idx;
    nRefCount = reinterpret_cast<CH264VideoFrame *>(dwCookie)->Release();
    
    if((nPicStoreIdx>=0) && (nRefCount == 0))
        IMGPAR FP_ReleaseDecodeFrame ARGS1(nPicStoreIdx);
    return 0;
}

ULONG CH264VDecHP::AddRefFrame(
                                                             IN DWORD dwCookie)
{
    if(dwCookie==0)
        return 0;
    return reinterpret_cast<CH264VideoFrame *>(dwCookie)->AddRef();
}

STDMETHODIMP CH264VDecHP::ReleaseBuffer (IN const DWORD dwReleaseFlags, OUT LPBYTE *ppBufferPtr)
{
    //extern const unsigned char *buf_begin, *buf_end;

    if(ppBufferPtr)
        *ppBufferPtr = const_cast<LPBYTE>(buf_begin);
    buf_end = buf_begin;
    return S_OK;
}

STDMETHODIMP CH264VDecHP::Get(DWORD dwPropID, OUT LPVOID pPropData, IN DWORD cbPropData, OUT DWORD *pcbReturned)
{
    return E_PROP_ID_UNSUPPORTED;
}

STDMETHODIMP CH264VDecHP::Set(DWORD dwPropID, OUT LPVOID pPropData, IN DWORD cbPropData)
{
    HRESULT hr = E_PROP_ID_UNSUPPORTED;
    DEBUG_SHOW_SW_INFO("CH264VDecHP::Set()");

    switch(dwPropID)
    {
#if defined(_COLLECT_PIC_)
        case H264_PROPID_DOWNSAMPLE:
            if(cbPropData == sizeof(DWORD))
            {
                hr = S_OK;
                if(0 == stream_global->m_resize_width_height_reg)
                    stream_global->m_resize_width_height = *((DWORD*)pPropData);
            }
            else
                hr = E_FAIL;    

            DEBUG_SHOW_SW_INFO("Set m_resize_width_height to %d", stream_global->m_resize_width_height);

            break;
#endif  //End: defined(_COLLECT_PIC_)
        case H264_PROPID_SMART_DECODE:
            if (g_bNormalSpeed != TRUE)
                return S_OK; //don't reset SmartDecLevel on non-normal speed mode

            if(cbPropData == sizeof(DWORD))
            {
                hr = S_OK;
                switch (*((DWORD*)pPropData))
                {
                case SMART_DEC_DISABLE:        g_dwSmartDecLevel = SMART_DEC_DISABLE;    break;
                case H264_REG_SMART_DEC_1:    g_dwSmartDecLevel = SMART_DEC_LEVEL_1;    break;
                case H264_REG_SMART_DEC_2:    g_dwSmartDecLevel = SMART_DEC_LEVEL_2;    break;
                case H264_REG_SMART_DEC_3:    g_dwSmartDecLevel = SMART_DEC_LEVEL_3;    break;
                case H264_REG_SMART_DEC_6:    g_dwSmartDecLevel = SMART_DEC_LEVEL_6;    break;
                default: hr = E_FAIL;    // out of the setting range
                }
            }
            else
                hr = E_FAIL;

#ifdef _HW_ACCEL_
            if (g_DXVAVer && (hr == S_OK) && (g_dwSmartDecLevel >= SMART_DEC_LEVEL_4 && g_dwSmartDecLevel != SMART_DEC_LEVEL_0))
                g_dwSmartDecLevel = SMART_DEC_LEVEL_3;
#endif

#if defined (_COLLECT_PIC_)
            stream_global->m_dwSavedSmartDecLevel = g_dwSmartDecLevel;
#endif

            DEBUG_SHOW_SW_INFO("Set SmartDecLevel to %d(%d)", *((DWORD*)pPropData), g_dwSmartDecLevel);
            break;
        case H264_PROPID_EOS: //set EOS by upper layer.
#if defined (_COLLECT_PIC_)
            stream_global->m_bTrickEOS = *((bool*)pPropData);
#endif
            break;

        case H264_PROPID_FLUSH_EMPTY_STREAM_DATA:
            g_bNormalSpeed = FALSE;
            break;

        case H264_PROPID_NVCTRL_SPEED:
#if defined (_COLLECT_PIC_)
            if(cbPropData == sizeof(DWORD))
            {
                hr = S_OK;
                switch (*((DWORD*)pPropData))
                {
                case 1000: //normal playback
                    g_dwSmartDecLevel = stream_global->m_dwSavedSmartDecLevel;
                    //Use m_bSetNormalSpeed to record normal speed status!!
                    m_bSetNormalSpeed = TRUE;
                    break;
                default: //Others
                    g_dwSmartDecLevel = SMART_DEC_LEVEL_5;
                    if ((*((DWORD*)pPropData) > 0) && (*((DWORD*)pPropData) <= 2000))
                    {
                        //Use m_bSetNormalSpeed to record normal speed status!!
                        m_bSetNormalSpeed = TRUE;
                    }
                    else
                    {
                        //Use m_bSetNormalSpeed to record normal speed status!!
                        m_bSetNormalSpeed = FALSE;
                        g_bDisplayed = FALSE; //set to output frame on trick mode
                    }
                    break;
                }
                DEBUG_SHOW_SW_INFO("Set Forward Speed to %d", *((DWORD*)pPropData));
            }
            else
                hr = E_FAIL;
#endif
			break;	
		default:
			break;
	}

    DEBUG_SHOW_SW_INFO("CH264VDecHP::Set() END");
    return hr;
}

void DownSample_Widht_Height(H264VDecHP_Frame *pFrame, StorablePicture *pSP)
{
//#if defined(H264_ENABLE_INTRINSICS)
    if((cpu_type == CPU_LEVEL_MMX) || (cpu_type == CPU_LEVEL_SSE) || (cpu_type == CPU_LEVEL_SSE2) || (cpu_type == CPU_LEVEL_SSE3))
    {
        static const long num_zero[] = {0x00FF00FF, 0x00FF00FF};
        __m64 mm0, mm1, mm2, mm3, mm_zero;
        mm_zero = *((__m64*)&num_zero[0]);

        unsigned int i, j;    
        unsigned int width;
        int res_w;    
        PBYTE srcY = pFrame->apbFrame[0];
        PBYTE srcUV = pFrame->apbFrame[1];
        PBYTE dstY = pFrame->apbFrame[0] = pSP->pDownSampleY;
        PBYTE dstUV = pFrame->apbFrame[1] = pSP->pDownSampleUV;


        res_w = pFrame->adwWidth[0] & 0x0F;
        width = (pFrame->adwWidth[0] - res_w) >> 4;

        for(i=0; i<pFrame->adwHeight[0]; i+=2)
        {
            for(j=0; j<width; j++) //process 16 bytes one time.
            {
                mm0 = *((__m64*)&srcY[(j<<4)]);
                mm1 = *((__m64*)&srcY[(j<<4)+8]);
                mm0 = _mm_and_si64(mm0, mm_zero);
                mm1 = _mm_and_si64(mm1, mm_zero);
                mm0 = _mm_packs_pu16(mm0,mm1);

                *((__m64*)&dstY[(j<<3)]) = mm0;
            }
            for(j=(pFrame->adwWidth[0] - res_w); j<pFrame->adwWidth[0]; j+=2)
            dstY[(j>>1)] = srcY[j];

            srcY += (pFrame->adwStride[0] + pFrame->adwStride[0]); //height become half.
            dstY += pFrame->adwStride[0];
        }

        //merge UV mode now
        res_w = (pFrame->adwWidth[1]<<1) & 0x0F;
        width = ((pFrame->adwWidth[1]<<1) - res_w) >> 4;

        for(i=0; i<pFrame->adwHeight[1]; i+=2)
        {
            for(j=0; j<width; j++) //process 16 bytes one time.
            {
                mm0 = *((__m64*)&srcUV[(j<<4)]);
                mm1 = *((__m64*)&srcUV[(j<<4)+8]);
                mm2 = _mm_unpacklo_pi16(mm0, mm1);
                mm3 = _mm_unpackhi_pi16(mm0, mm1);
                mm0 = _mm_unpacklo_pi16(mm2, mm3);

                *((__m64*)&dstUV[(j<<3)]) = mm0;
            }
            for(j=((pFrame->adwWidth[1]<<1) - res_w); j<(pFrame->adwWidth[1]<<1); j+=4)
            {
            dstUV[(j>>1)+0] = srcUV[j+0];    
            dstUV[(j>>1)+1] = srcUV[j+1];
            }

            srcUV += (pFrame->adwStride[1]+pFrame->adwStride[1]);
            dstUV += (pFrame->adwStride[1]);
        }

    //pFrame->adwWidth[0]  =  pFrame->adwWidth[0] >> 1;  //p->size_x-crop_left-crop_right;
    //pFrame->adwStride[0] =  (pFrame->adwWidth[0]+2*BOUNDARY_EXTENSION_X) *symbol_size_in_bytes; // p->Y_stride*symbol_size_in_bytes;
    //pFrame->adwHeight[0] = pFrame->adwHeight[0] >> 1;//p->size_y-crop_top-crop_bottom;
        pFrame->adwLeft[0]   = pFrame->adwLeft[0] >> 1;//crop_left;
        pFrame->adwTop[0]    = pFrame->adwTop[0] >> 1;//crop_top;
        pFrame->adwLeft[1]   = pFrame->adwLeft[0] >> 1;//crop_left;
        pFrame->adwTop[1]    = pFrame->adwTop[0] >> 1;//crop_top;
        pFrame->adwLeft[2]   = pFrame->adwLeft[0] >> 1;//crop_left;
        pFrame->adwTop[2]    = pFrame->adwTop[0] >> 1;//crop_top;
    //pFrame->chroma_format_idc = p->chroma_format_idc;
#ifdef __SUPPORT_YUV400__
    if (pFrame->chroma_format_idc!=YUV400)
    {
#endif
        //pFrame->adwWidth[1]  = pFrame->adwWidth[1] >> 1;//p->size_x_cr-crop_left-crop_right;
        //pFrame->adwHeight[1] = pFrame->adwHeight[1]>>1;//p->size_y_cr-crop_top-crop_bottom;

        //pFrame->adwWidth[2]  = pFrame->adwWidth[2]>>1;//p->size_x_cr-crop_left-crop_right;
        //pFrame->adwHeight[2] = pFrame->adwHeight[2] >>1;//p->size_y_cr-crop_top-crop_bottom;

        //Grant - chroma crop_left is the same as luma if merge UV
        pFrame->adwLeft[1]   = pFrame->adwLeft[1];//crop_left;
        pFrame->adwTop[1]    = pFrame->adwTop[1] >>1;//crop_top;
        pFrame->adwLeft[2]   = pFrame->adwLeft[2];//crop_left;
        pFrame->adwTop[2]    = pFrame->adwTop[2] >>1;//crop_top;
#ifdef __SUPPORT_YUV400__
    }
#endif
    }
    else
    {
        int i, j;
        int symbol_size_in_bytes = 1; // HP restriction
        PBYTE srcY  = pFrame->apbFrame[0];
        PBYTE srcUV = pFrame->apbFrame[1];
        PBYTE dstY  = pFrame->apbFrame[0] = pSP->pDownSampleY;
        PBYTE dstUV = pFrame->apbFrame[1] = pSP->pDownSampleUV;

        for(i=0; i<pFrame->adwHeight[0]; i+=2)
        {
            for(j=0; j<pFrame->adwWidth[0] / 2; j++)
            {
                dstY[j] = srcY[(j<<1)];
    
            }
            srcY += (pFrame->adwStride[0] << 1);
            dstY += pFrame->adwStride[0];
        }

        for(i=0; i<pFrame->adwHeight[1]; i+=2)
        {
            for(j=0; j<pFrame->adwWidth[1]; j+=2)
            {
                dstUV[j+0] = srcUV[(j<<1)+0];
                dstUV[j+1] = srcUV[(j<<1)+1];
            }
            srcUV += (pFrame->adwStride[1]<<1);
            dstUV += (pFrame->adwStride[1]);
        }

    //pFrame->adwWidth[0]  =  pFrame->adwWidth[0] >> 1;  //p->size_x-crop_left-crop_right;
    //pFrame->adwStride[0] =  (pFrame->adwWidth[0]+2*BOUNDARY_EXTENSION_X) *symbol_size_in_bytes; // p->Y_stride*symbol_size_in_bytes;
    //pFrame->adwHeight[0] = pFrame->adwHeight[0] >> 1;//p->size_y-crop_top-crop_bottom;
        pFrame->adwLeft[0]   = pFrame->adwLeft[0] >> 1;//crop_left;
        pFrame->adwTop[0]    = pFrame->adwTop[0] >> 1;//crop_top;
        pFrame->adwLeft[1]   = pFrame->adwLeft[0] >> 1;//crop_left;
        pFrame->adwTop[1]    = pFrame->adwTop[0] >> 1;//crop_top;
        pFrame->adwLeft[2]   = pFrame->adwLeft[0] >> 1;//crop_left;
        pFrame->adwTop[2]    = pFrame->adwTop[0] >> 1;//crop_top;
    //pFrame->chroma_format_idc = p->chroma_format_idc;
#ifdef __SUPPORT_YUV400__
    if (pFrame->chroma_format_idc!=YUV400)
    {
#endif
        //pFrame->adwWidth[1]  = pFrame->adwWidth[1] >> 1;//p->size_x_cr-crop_left-crop_right;
        //pFrame->adwHeight[1] = pFrame->adwHeight[1]>>1;//p->size_y_cr-crop_top-crop_bottom;

        //pFrame->adwWidth[2]  = pFrame->adwWidth[2]>>1;//p->size_x_cr-crop_left-crop_right;
        //pFrame->adwHeight[2] = pFrame->adwHeight[2] >>1;//p->size_y_cr-crop_top-crop_bottom;

        //Grant - chroma crop_left is the same as luma if merge UV
        pFrame->adwLeft[1]   = pFrame->adwLeft[1];//crop_left;
        pFrame->adwTop[1]    = pFrame->adwTop[1] >>1;//crop_top;
        pFrame->adwLeft[2]   = pFrame->adwLeft[2];//crop_left;
        pFrame->adwTop[2]    = pFrame->adwTop[2] >>1;//crop_top;
#ifdef __SUPPORT_YUV400__
    }
#endif
    }
//#endif
}

void DownSample_Height(H264VDecHP_Frame *pFrame, StorablePicture *pSP)
{
    unsigned int i;
    int symbol_size_in_bytes = 1; // HP restriction
    PBYTE srcY  = pFrame->apbFrame[0];
    PBYTE srcUV = pFrame->apbFrame[1];
    PBYTE dstY  = pFrame->apbFrame[0] = pSP->pDownSampleY;
    PBYTE dstUV = pFrame->apbFrame[1] = pSP->pDownSampleUV;

    for(i=0; i<pFrame->adwHeight[0]; i+=2)
    {
        memcpy(dstY, srcY, pFrame->adwWidth[0]);
        srcY += (pFrame->adwStride[0] << 1);
        dstY += pFrame->adwStride[0];
    }

    for(i=0; i<pFrame->adwHeight[1]; i+=2)
    {
        memcpy(dstUV, srcUV, (pFrame->adwWidth[1]<<1));
        srcUV += (pFrame->adwStride[1]<<1);
        dstUV += (pFrame->adwStride[1]);
    }

    //pFrame->adwHeight[0] = pFrame->adwHeight[0] >> 1;//p->size_y-crop_top-crop_bottom;
    pFrame->adwTop[0]    = pFrame->adwTop[0] >> 1;//crop_top;
    pFrame->adwTop[1]    = pFrame->adwTop[0] >> 1;//crop_top;
    pFrame->adwTop[2]    = pFrame->adwTop[0] >> 1;//crop_top;

#ifdef __SUPPORT_YUV400__
    if (pFrame->chroma_format_idc!=YUV400)
    {
#endif
        //pFrame->adwHeight[1] = pFrame->adwHeight[1]>>1;//p->size_y_cr-crop_top-crop_bottom;
        //pFrame->adwHeight[2] = pFrame->adwHeight[2] >>1;//p->size_y_cr-crop_top-crop_bottom;

        pFrame->adwTop[1]    = pFrame->adwTop[1] >>1;//crop_top;
        pFrame->adwTop[2]    = pFrame->adwTop[2] >>1;//crop_top;
#ifdef __SUPPORT_YUV400__
    }
#endif
}

int CH264VDecHP::GetPicture PARGS2(H264VDecHP_Frame *pFrame, unsigned int view_index)
{
    CH264VideoFrame *f;
    StorablePicture *p;
    int SubWidthC  [2]= { 1, 2 }; // only YUV400 and YUV420
    int SubHeightC [2]= { 1, 2 };
    int crop_left, crop_right, crop_top, crop_bottom;
    int symbol_size_in_bytes = 1; // HP restriction
    

    DEBUG_INFO("CH264VDecHP::GetPicture");

#ifdef _COLLECT_PIC_
    img = img_array[0];
#endif

    f = g_framemgr->GetDisplayFrame(view_index);
    if(f==0)
    {
        DEBUG_SHOW_SW_INFO("nonexisting DisplayFrame!\n");
        return -1;
    }
    p = static_cast<StorablePicture *>(f);
    if(p->non_existing)
    {
        DEBUG_SHOW_SW_INFO("nonexisting frame!\n");
        f->Release();
        return -1;
    }
    pFrame->dwCookie = (DWORD)f;
    pFrame->bRgbOutput = (active_sps.vui_seq_parameters.matrix_coefficients==0);
#if defined(_HW_ACCEL_)
    pFrame->frame_index = g_DXVAVer ? p->pic_store_idx : -1; // -1 means invalid
#else
    pFrame->frame_index = -1; // -1 means invalid
#endif
    pFrame->apbFrame[0] = p->imgY;
    pFrame->apbFrame[1] = p->imgUV;    
    pFrame->apbFrame[2] = NULL;
    pFrame->symbol_size_in_bytes = symbol_size_in_bytes;
    pFrame->frame_num = p->frame_num;
    pFrame->pic_num = p->pic_num;
    pFrame->dwPictureOrderCounter = p->poc;
    pFrame->slice_type = p->slice_type;
    pFrame->top_field_first = p->top_field_first;
    pFrame->framerate1000 = p->framerate1000;
    pFrame->dwXAspect = p->dwXAspect;
    pFrame->dwYAspect = p->dwYAspect;
    pFrame->dwBitRate = p->dwBitRate;
    pFrame->progressive_frame = p->progressive_frame;
    pFrame->pts = p->pts;
    pFrame->dpb_buffer_size = dpb.size_on_view[view_index];
    pFrame->bRepeatFirstField = p->repeat_first_field; //2-3 Pull Down

    pFrame->mb_info = (H264DecHP_Macroblock*)p->mb_data;

    if(pFrame->mb_info->mb_type != 0)
        pFrame->mb_info->mb_type = pFrame->mb_info->mb_type;

        
    if(p->frame_cropping_flag)
    {
        crop_left   = SubWidthC[p->chroma_format_idc] * p->frame_cropping_rect_left_offset;
        crop_right  = SubWidthC[p->chroma_format_idc] * p->frame_cropping_rect_right_offset;
        crop_top    = SubHeightC[p->chroma_format_idc]*( 2 - p->frame_mbs_only_flag ) * p->frame_cropping_rect_top_offset;
        crop_bottom = SubHeightC[p->chroma_format_idc]*( 2 - p->frame_mbs_only_flag ) * p->frame_cropping_rect_bottom_offset;
    }
    else
    {
        crop_left = crop_right = crop_top = crop_bottom = 0;
    }
    if(pFrame->bRgbOutput)
    {
        crop_left   = p->frame_cropping_rect_left_offset;
        crop_right  = p->frame_cropping_rect_right_offset;
        crop_top    = ( 2 - p->frame_mbs_only_flag ) * p->frame_cropping_rect_top_offset;
        crop_bottom = ( 2 - p->frame_mbs_only_flag ) * p->frame_cropping_rect_bottom_offset;

        if (active_sps.frame_cropping_flag)
        {
            crop_left   = SubWidthC[p->chroma_format_idc] * p->frame_cropping_rect_left_offset;
            crop_right  = SubWidthC[p->chroma_format_idc] * p->frame_cropping_rect_right_offset;
            crop_top    = SubHeightC[p->chroma_format_idc]*( 2 - p->frame_mbs_only_flag ) * p->frame_cropping_rect_top_offset;
            crop_bottom = SubHeightC[p->chroma_format_idc]*( 2 - p->frame_mbs_only_flag ) * p->frame_cropping_rect_bottom_offset;
        }
        else
        {
            crop_left = crop_right = crop_top = crop_bottom = 0;
        }
    }
    pFrame->adwWidth[0]  = p->size_x-crop_left-crop_right;
    pFrame->adwStride[0] = p->Y_stride*symbol_size_in_bytes;
    pFrame->adwHeight[0] = p->size_y-crop_top-crop_bottom;
    pFrame->adwLeft[0]   = crop_left;
    pFrame->adwTop[0]    = crop_top;
    pFrame->adwLeft[1]   = crop_left;
    pFrame->adwTop[1]    = crop_top;
    pFrame->adwLeft[2]   = crop_left;
    pFrame->adwTop[2]    = crop_top;
    pFrame->chroma_format_idc = p->chroma_format_idc;
#ifdef __SUPPORT_YUV400__
    if (p->chroma_format_idc!=YUV400)
    {
#endif
        crop_left   = p->frame_cropping_rect_left_offset;
        crop_right  = p->frame_cropping_rect_right_offset;
        crop_top    = ( 2 - p->frame_mbs_only_flag ) * p->frame_cropping_rect_top_offset;
        crop_bottom = ( 2 - p->frame_mbs_only_flag ) * p->frame_cropping_rect_bottom_offset;
        pFrame->adwWidth[1]  = p->size_x_cr-crop_left-crop_right;
        pFrame->adwStride[1] = p->UV_stride*symbol_size_in_bytes;
        pFrame->adwHeight[1] = p->size_y_cr-crop_top-crop_bottom;
        pFrame->adwWidth[2]  = p->size_x_cr-crop_left-crop_right;
        pFrame->adwStride[2] = p->UV_stride*symbol_size_in_bytes;
        pFrame->adwHeight[2] = p->size_y_cr-crop_top-crop_bottom;
        //Grant - chroma crop_left is the same as luma if merge UV
        //pFrame->adwLeft[1]   = crop_left;
        pFrame->adwTop[1]    = crop_top;
        //pFrame->adwLeft[2]   = crop_left;
        pFrame->adwTop[2]    = crop_top;
#ifdef __SUPPORT_YUV400__
    }
#endif

    // Get Line21 CC data
    memcpy(pFrame->m_cc1buf, &(p->m_CCCode.ccbuf[0]), (p->m_CCCode.cclen));
    pFrame->m_cc1len = (p->m_CCCode.cclen);


#if defined(_COLLECT_PIC_)
#if defined(_HW_ACCEL_)
    if(g_DXVAVer == IviNotDxva || IMGPAR Hybrid_Decoding ==5 )
#endif
    {
        if(1 == stream_global->m_resize_width_height && (pFrame->adwWidth[0] * pFrame->adwHeight[0]> 960*540))
        {
            DownSample_Widht_Height(pFrame, p);
#if defined(_MSC_VER) || defined(__INTEL_COMPILER)
            _asm emms;
#elif defined(__GNUC__) && defined(__i386__)
            __asm__ ("emms");
#endif
        }
        else if(2 == stream_global->m_resize_width_height && (pFrame->adwWidth[0] * pFrame->adwHeight[0]> 960*540))
        {
            DownSample_Height(pFrame, p);
        }
    }
#endif

    if (p->dwYCCBufferLen)
    {
        memcpy((void*)pFrame->pbYCCBuffer, (void*)p->pbYCCBuffer, 12);
        pFrame->dwYCCBufferLen = p->dwYCCBufferLen;
    }

    DEBUG_INFO("CH264VDecHP::GetPicture End");

    return 0;
}

CREL_RETURN DecodeFrameSingleThread PARGS0()
{
    int i, j;
    int nSliceCount = IMGPAR slice_number;
    Slice *currSlice;
    BOOL bSmartDecode4Finished = FALSE;
    StreamParameters *stream_global = IMGPAR stream_global;
    CREL_RETURN ret = CREL_OK;
    unsigned int view_index = (IMGPAR firstSlice) ? (IMGPAR firstSlice->viewIndex) : 0;

    IMGPAR currentSlice = IMGPAR firstSlice;
    IMGPAR prevSlice    = NULL;
    IMGPAR current_slice_nr = 0;


    while(nSliceCount && (!IMGPAR SkipThisFrame))
    {
        for (i=0; i<1; i++)
        {
            ret = initial_image ARGS0();

            if (FAILED(ret)) {
                break;
            }

            if ((IMGPAR smart_dec & SMART_DEC_INT_PEL_Y) && (IMGPAR currentSlice->picture_type == B_SLICE) && !IMGPAR nal_reference_idc)
            {
                get_block_4xh  = get_block_4xh_int;
                get_block_8xh  = get_block_8xh_int;
                get_block_16xh = get_block_16xh_int;
            }
            else
            {
                get_block_4xh  = get_block_4xh_full;
                get_block_8xh  = get_block_8xh_full;
                get_block_16xh = get_block_16xh_full;
            }
            set_4xH_mc_function_ptr ARGS0();

            if ((IMGPAR smart_dec & SMART_DEC_INT_PEL_UV) && (IMGPAR currentSlice->picture_type == B_SLICE) && !IMGPAR nal_reference_idc)
            {
                mb_chroma_2xH = mb_chroma_2xH_int;
                mb_chroma_4xH = mb_chroma_4xH_int;
                mb_chroma_8xH = mb_chroma_8xH_int;
            }
            else
            {
                mb_chroma_2xH = mb_chroma_2xH_full;
                mb_chroma_4xH = mb_chroma_4xH_full;
                mb_chroma_8xH = mb_chroma_8xH_full;
            }
            set_2xH_mc_function_ptr ARGS0();

            if (IMGPAR currentSlice->header == SOP)
                IMGPAR FP_BeginDecodeFrame ARGS0();

            if ((IMGPAR smart_dec & SMART_DEC_SKIP_FIL_B) 
                && IMGPAR currentSlice->picture_type == B_SLICE 
                && IMGPAR currentSlice->structure != FRAME 
                && IMGPAR currentSlice->m_pic_combine_status == 0  
                && !IMGPAR nal_reference_idc
                && bSmartDecode4Finished == FALSE)
            {
                imgpel *pImgY, *pImgUV;

                currMB_d = &IMGPAR mb_decdata[IMGPAR current_mb_nr_d];
                currMB_s_d = &dec_picture->mb_data[IMGPAR current_mb_nr_d];

                for(i=0; i<IMGPAR PicSizeInMbs; i++)
                {
                    memset(currMB_s_d->pred_info.mv, 0, 2*BLOCK_SIZE*BLOCK_SIZE*2*sizeof(short));
                    memset(currMB_s_d->pred_info.ref_idx, -1, 2*2*BLOCK_SIZE*BLOCK_SIZE/4);
                    IMGPAR current_mb_nr_d++;
                    currMB_d++;
                    currMB_s_d++;
                }

                if(IMGPAR structure == TOP_FIELD)
                {
                    pImgY = IMGPAR m_dec_picture_bottom->imgY;
                    pImgUV = IMGPAR m_dec_picture_bottom->imgUV;
                }
                else//(IMGPAR structure == BOTTOM_FIELD)
                {
                    pImgY = IMGPAR m_dec_picture_top->imgY;
                    pImgUV = IMGPAR m_dec_picture_top->imgUV;
                }

#if defined(_HW_ACCEL_)
                if(g_DXVAVer == IviNotDxva || IMGPAR Hybrid_Decoding == 5 )
#endif
                {
                    for (i=0; i<dec_picture->size_y; i++)
                        memcpy(dec_picture->imgY+(i*dec_picture->Y_stride), pImgY+(i*dec_picture->Y_stride), dec_picture->Y_stride>>1);
                    for (i=0; i<dec_picture->size_y_cr; i++)
                        memcpy(dec_picture->imgUV+(i*dec_picture->UV_stride), pImgUV+(i*dec_picture->UV_stride), dec_picture->UV_stride>>1);
                }

                bSmartDecode4Finished = TRUE;

                break;
            }

            if (SUCCEEDED(ret))
            {
                exit_slice ARGS0();
                SetEvent(stream_global->m_event_start_slice[0]);
            }
        }

        if (FAILED(ret)) 
        {
            WaitForSingleObject(stream_global->m_event_end_slice[0], INFINITE);
            ResetEvent(stream_global->m_event_end_slice[0]);
            nSliceCount--;
            break;
        }

        if (bSmartDecode4Finished == FALSE)
        {

            WaitForSingleObject(stream_global->m_event_end_slice[0], INFINITE);
            ResetEvent(stream_global->m_event_end_slice[0]);
            nSliceCount--;
            //m_bLast_Slice = (j==i);
            img = img_array[0];

            currSlice = IMGPAR currentSlice;


            if(active_pps.num_slice_groups_minus1 == 0)
            {
#if defined(_HW_ACCEL_)
                if((!( (IMGPAR smart_dec & SMART_DEC_NODEBLK_ALL) || ((IMGPAR smart_dec & SMART_DEC_NODEBLK_B) && (currSlice->picture_type == B_SLICE)) )) && (g_DXVAVer==IviNotDxva))
#else
                if(!( (IMGPAR smart_dec & SMART_DEC_NODEBLK_ALL) || ((IMGPAR smart_dec & SMART_DEC_NODEBLK_B) && (currSlice->picture_type == B_SLICE)) ))
#endif
                {
                    if ( (IMGPAR error_mb_nr < 0) &&(stream_global->m_bSeekIDR == FALSE)) {
                        if (IMGPAR MbaffFrameFlag)
                            IMGPAR FP_DeblockSlice ARGS3(dec_picture, currSlice->start_mb_nr<<1, currSlice->read_mb_nr );
                        else
                            IMGPAR FP_DeblockSlice ARGS3(dec_picture, currSlice->start_mb_nr, currSlice->read_mb_nr );
                    }
                }
            }

        }
        else
        {
            int nNumOfSlice = nSliceCount;
            for (j=0; j<nNumOfSlice; j++)
            {
                nSliceCount--;
                IMGPAR current_slice_nr++;
                IMGPAR prevSlice = IMGPAR currentSlice;
                IMGPAR currentSlice = (Slice*) (IMGPAR currentSlice->nextSlice);
            }
            IMGPAR currentSlice = IMGPAR prevSlice;
        }

        if (IMGPAR currentSlice->exit_flag)
        {
            if(active_pps.num_slice_groups_minus1 > 0)
            {
#if defined(_HW_ACCEL_)
                if((!( (IMGPAR smart_dec & SMART_DEC_NODEBLK_ALL) || ((IMGPAR smart_dec & SMART_DEC_NODEBLK_B) && (currSlice->picture_type == B_SLICE)) )) && (g_DXVAVer==IviNotDxva))
#else
                if(!( (IMGPAR smart_dec & SMART_DEC_NODEBLK_ALL) || ((IMGPAR smart_dec & SMART_DEC_NODEBLK_B) && (currSlice->picture_type == B_SLICE)) ))
#endif
                {
                    if ( (IMGPAR error_mb_nr < 0) &&(stream_global->m_bSeekIDR == FALSE)) {
                        if (IMGPAR MbaffFrameFlag)
                            IMGPAR FP_DeblockSlice ARGS3(dec_picture, 0, dec_picture->decoded_mb_number );
                        else
                            IMGPAR FP_DeblockSlice ARGS3(dec_picture, 0, dec_picture->decoded_mb_number );
                    }
                }
            }
#if defined (_HW_ACCEL_)
            if(g_DXVAVer && IMGPAR Hybrid_Decoding)
            {
                if(IMGPAR Hybrid_Decoding == 5 || (IMGPAR Hybrid_Decoding == 2 && IMGPAR type != B_SLICE) || IMGPAR type == I_SLICE)
                    DMA_Transfer ARGS0();
            }
#endif
            ret = exit_picture ARGS0();
            if (FAILED(ret)) {
                //return ret;
                break;
            }
        }

        if (FAILED(ret)) {
            break;
        }

        if(nSliceCount)
        {
            IMGPAR current_slice_nr++;
            IMGPAR prevSlice = IMGPAR currentSlice;
            IMGPAR currentSlice = (Slice*) (IMGPAR currentSlice->nextSlice);
        }
    }

    if (FAILED(ret)) 
    {
        DEBUG_SHOW_ERROR_INFO("[ERROR] DecodeOneFrame Fail! Reset Buffer!");

        if (IMGPAR structure == FRAME) 
        {

            if(dec_picture)
            {
                if ((dpb.used_size_on_view[view_index] == 0) ||dpb.fs_on_view[view_index][dpb.used_size_on_view[view_index] - 1]->frame != dec_picture) {
                    release_storable_picture ARGS2(dec_picture, 1);
                }
                dec_picture = NULL;
            }
            img->m_dec_picture_top = NULL;
            img->m_dec_picture_bottom = NULL;


        } 
        else 
        {

            if (img->m_dec_picture_top && (img->m_dec_picture_top->top_field_first)) 
            {

                if(img->m_dec_picture_top && img->m_dec_picture_bottom)
                {
                    if ((dpb.used_size_on_view[view_index] == 0) ||dpb.fs_on_view[view_index][dpb.used_size_on_view[view_index] - 1]->bottom_field != img->m_dec_picture_bottom) 
                    {
                        release_storable_picture ARGS2(img->m_dec_picture_bottom, 1);
                    }

                } 
                else 
                {

                    if ((dpb.used_size_on_view[view_index] == 0) ||dpb.fs_on_view[view_index][dpb.used_size_on_view[view_index] - 1]->top_field != img->m_dec_picture_top) 
                    {
                        release_storable_picture ARGS2(img->m_dec_picture_top, 1);                    
                    }

                }
                img->m_dec_picture_top = NULL;
                img->m_dec_picture_bottom = NULL;


            } 
            else 
            {

                if(img->m_dec_picture_top && img->m_dec_picture_bottom)
                {
                    if ((dpb.used_size_on_view[view_index] == 0) ||dpb.fs_on_view[view_index][dpb.used_size_on_view[view_index] - 1]->top_field != img->m_dec_picture_top) 
                    {
                        release_storable_picture ARGS2(img->m_dec_picture_top, 1);
                    }

                } 
                else 
                {

                    if ((dpb.used_size_on_view[view_index] == 0) ||dpb.fs_on_view[view_index][dpb.used_size_on_view[view_index] - 1]->bottom_field != img->m_dec_picture_bottom) 
                    {
                        release_storable_picture ARGS2(img->m_dec_picture_bottom, 1);
                    }

                }
                img->m_dec_picture_top = NULL;
                img->m_dec_picture_bottom = NULL;

            }        

            dec_picture = NULL;

        }    

    }


    img = img_array[0];

    IMGPAR currentSlice = IMGPAR firstSlice;
    for (j=0; j< IMGPAR slice_number; j++)
    {
        memcpy(&active_pps, &PicParSet[IMGPAR currentSlice->pic_parameter_set_id], sizeof(pic_parameter_set_rbsp_t));
        IMGPAR prevSlice = IMGPAR currentSlice;
        IMGPAR currentSlice = (Slice*) IMGPAR currentSlice->nextSlice;
        free_new_slice(IMGPAR prevSlice);
    }

    IMGPAR firstSlice = IMGPAR currentSlice = IMGPAR prevSlice = NULL;
    IMGPAR current_mb_nr_r = IMGPAR current_mb_nr_d = -4712;   // impossible value for debugging, StW
    IMGPAR error_mb_nr = -4712;
    IMGPAR current_slice_nr = 0;
    IMGPAR slice_number = 0;
    img->m_active_pps[0].Valid = NULL;
    img->m_active_pps[1].Valid = NULL;
    dec_picture = NULL;
    img->m_dec_picture_top = NULL;
    img->m_dec_picture_bottom = NULL;

    if (IMGPAR structure != FRAME)
    {
        IMGPAR mb_decdata = IMGPAR mb_decdata_ori;
        IMGPAR cof_array = IMGPAR cof_array_ori;
    }


    return ret;
}

CREL_RETURN DecodeOneFrame PARGS0()
{
    int i, j;
    int nSliceCount = IMGPAR slice_number;
    Slice *currSlice;
    BOOL bSmartDecode4Finished = FALSE;
    int nNeedToReleaseSlice;
    StreamParameters *stream_global = IMGPAR stream_global;
    CREL_RETURN ret = CREL_OK;
    int num_active_threads;
    unsigned int view_index = (IMGPAR firstSlice) ? (IMGPAR firstSlice->viewIndex) : 0;

	int icurrent_img_index = 0;
	if (nSliceCount >= 6)
		nNeedToReleaseSlice = 6;
	else
		nNeedToReleaseSlice = nSliceCount;
	
	IMGPAR currentSlice = IMGPAR firstSlice;
	IMGPAR prevSlice    = NULL;
	IMGPAR current_slice_nr = 0;

	int num_views = 1;
	BOOL bIsMVCFieldPic = FALSE;
	if(stream_global->bMVC == TRUE && IMGPAR firstSlice->field_pic_flag == TRUE)
	{
		num_views = stream_global->num_views;
		img = img_array[view_index];
		bIsMVCFieldPic = TRUE;
	}
    
    while(nSliceCount && (!IMGPAR SkipThisFrame))
    {
        num_active_threads = 0;

        int loop_number = min(nSliceCount, 6);
        for (i=0; i<loop_number; i++)
        {
			icurrent_img_index = i;

			if(bIsMVCFieldPic == TRUE)
			{
				if(i > 0)
					icurrent_img_index = num_views + i - 1;
				else
					icurrent_img_index = view_index;
			}
			
            ret = initial_image ARGS0();

            if (FAILED(ret)) {
                break;
            }

            if ((IMGPAR smart_dec & SMART_DEC_INT_PEL_Y) && (IMGPAR currentSlice->picture_type == B_SLICE) && !IMGPAR nal_reference_idc)
            {
                get_block_4xh  = get_block_4xh_int;
                get_block_8xh  = get_block_8xh_int;
                get_block_16xh = get_block_16xh_int;
            }
            else
            {
                get_block_4xh  = get_block_4xh_full;
                get_block_8xh  = get_block_8xh_full;
                get_block_16xh = get_block_16xh_full;
            }
            set_4xH_mc_function_ptr ARGS0();

            if ((IMGPAR smart_dec & SMART_DEC_INT_PEL_UV) && (IMGPAR currentSlice->picture_type == B_SLICE) && !IMGPAR nal_reference_idc)
            {
                mb_chroma_2xH = mb_chroma_2xH_int;
                mb_chroma_4xH = mb_chroma_4xH_int;
                mb_chroma_8xH = mb_chroma_8xH_int;
            }
            else
            {
                mb_chroma_2xH = mb_chroma_2xH_full;
                mb_chroma_4xH = mb_chroma_4xH_full;
                mb_chroma_8xH = mb_chroma_8xH_full;
            }
            set_2xH_mc_function_ptr ARGS0();

            if (IMGPAR currentSlice->header == SOP)
                IMGPAR FP_BeginDecodeFrame ARGS0();

            if ((IMGPAR smart_dec & SMART_DEC_SKIP_FIL_B) 
                && IMGPAR currentSlice->picture_type == B_SLICE 
                && IMGPAR currentSlice->structure != FRAME 
                && IMGPAR currentSlice->m_pic_combine_status == 0  
                && !IMGPAR nal_reference_idc
                && bSmartDecode4Finished == FALSE)
            {
                imgpel *pImgY, *pImgUV;

                currMB_d = &IMGPAR mb_decdata[IMGPAR current_mb_nr_d];
                currMB_s_d = &dec_picture->mb_data[IMGPAR current_mb_nr_d];

                for(i=0; i<IMGPAR PicSizeInMbs; i++)
                {
                    memset(currMB_s_d->pred_info.mv, 0, 2*BLOCK_SIZE*BLOCK_SIZE*2*sizeof(short));
                    memset(currMB_s_d->pred_info.ref_idx, -1, 2*2*BLOCK_SIZE*BLOCK_SIZE/4);
                    IMGPAR current_mb_nr_d++;
                    currMB_d++;
                    currMB_s_d++;
                }

                if(IMGPAR structure == TOP_FIELD)
                {
                    pImgY = IMGPAR m_dec_picture_bottom->imgY;
                    pImgUV = IMGPAR m_dec_picture_bottom->imgUV;
                }
                else//(IMGPAR structure == BOTTOM_FIELD)
                {
                    pImgY = IMGPAR m_dec_picture_top->imgY;
                    pImgUV = IMGPAR m_dec_picture_top->imgUV;
                }

#if defined(_HW_ACCEL_)
                if(g_DXVAVer == IviNotDxva || IMGPAR Hybrid_Decoding == 5 )
#endif
                {
                    for (i=0; i<dec_picture->size_y; i++)
                        memcpy(dec_picture->imgY+(i*dec_picture->Y_stride), pImgY+(i*dec_picture->Y_stride), dec_picture->Y_stride>>1);
                    for (i=0; i<dec_picture->size_y_cr; i++)
                        memcpy(dec_picture->imgUV+(i*dec_picture->UV_stride), pImgUV+(i*dec_picture->UV_stride), dec_picture->UV_stride>>1);
                }

                bSmartDecode4Finished = TRUE;

                break;
            }

            if (SUCCEEDED(ret))
            {
                exit_slice ARGS0();

				int iNext_img_index = icurrent_img_index + 1;
				if(bIsMVCFieldPic == TRUE)
					iNext_img_index = num_views + i;

				if (iNext_img_index == loop_number || IMGPAR currentSlice->exit_flag)
				{
					SetEvent(stream_global->m_event_start_slice[icurrent_img_index]);
					break;
				}
				else
				{
					img_array[iNext_img_index]->m_pic_combine_status = pic_combine_status;
					img_array[iNext_img_index]->m_dec_picture = IMGPAR m_dec_picture;
					img_array[iNext_img_index]->m_dec_picture_top = IMGPAR m_dec_picture_top;
					img_array[iNext_img_index]->m_dec_picture_bottom = IMGPAR m_dec_picture_bottom;
					img_array[iNext_img_index]->m_prev_dec_picture = IMGPAR m_prev_dec_picture;
					img_array[iNext_img_index]->current_slice_nr = IMGPAR current_slice_nr;
					img_array[iNext_img_index]->firstSlice = IMGPAR firstSlice;
					img_array[iNext_img_index]->prevSlice = IMGPAR prevSlice;
					img_array[iNext_img_index]->currentSlice = IMGPAR currentSlice;
					img_array[iNext_img_index]->slice_number = IMGPAR slice_number;
					img_array[iNext_img_index]->SkipThisFrame = IMGPAR SkipThisFrame;
					img_array[iNext_img_index]->m_clip_max_x = IMGPAR m_clip_max_x;
					img_array[iNext_img_index]->m_clip_max_x_cr = IMGPAR m_clip_max_x_cr;
					img_array[iNext_img_index]->smart_dec = IMGPAR smart_dec;
					img_array[iNext_img_index]->number = IMGPAR number;
					img_array[iNext_img_index]->pre_frame_num = IMGPAR pre_frame_num;
					img_array[iNext_img_index]->PreviousFrameNum = IMGPAR PreviousFrameNum;
					img_array[iNext_img_index]->PreviousFrameNumOffset = IMGPAR PreviousFrameNumOffset;
					img_array[iNext_img_index]->PreviousPOC = IMGPAR PreviousPOC;
					img_array[iNext_img_index]->ThisPOC = IMGPAR ThisPOC;
					img_array[iNext_img_index]->last_has_mmco_5 = IMGPAR last_has_mmco_5;
					img_array[iNext_img_index]->PrevPicOrderCntMsb = IMGPAR PrevPicOrderCntMsb;
					img_array[iNext_img_index]->PrevPicOrderCntLsb = IMGPAR PrevPicOrderCntLsb;
					img_array[iNext_img_index]->structure = IMGPAR structure;
#if defined(_HW_ACCEL_)
					if ( (IviDxva1==g_DXVAVer) && (IMGPAR Hybrid_Decoding!=5) )
					{
						img_array[iNext_img_index]->m_lpMBLK_Intra_Luma = img_array[icurrent_img_index]->m_lpMBLK_Intra_Luma;
						img_array[iNext_img_index]->m_lpMV = img_array[icurrent_img_index]->m_lpMV;
						if ( g_DXVAMode!=E_H264_DXVA_ATI_PROPRIETARY_A )
							img_array[iNext_img_index]->m_lpRESD_Intra_Luma = img_array[icurrent_img_index]->m_lpRESD_Intra_Luma;
					}
					img_array[iNext_img_index]->m_iFirstCompBufIndex = img_array[icurrent_img_index]->m_iFirstCompBufIndex;
					img_array[iNext_img_index]->m_Intra_lCompBufIndex = img_array[icurrent_img_index]->m_Intra_lCompBufIndex;
					img_array[iNext_img_index]->m_lFrame_Counter = img_array[icurrent_img_index]->m_lFrame_Counter;
					img_array[iNext_img_index]->UnCompress_Idx = img_array[icurrent_img_index]->UnCompress_Idx;
					img_array[iNext_img_index]->m_lpRESD_DXVA = img_array[icurrent_img_index]->m_lpRESD_DXVA;
					img_array[iNext_img_index]->m_pUnCompressedBuffer = img_array[icurrent_img_index]->m_pUnCompressedBuffer;
					img_array[iNext_img_index]->m_UnCompressedBufferStride = img_array[icurrent_img_index]->m_UnCompressedBufferStride;

#endif					
					SetEvent(stream_global->m_event_start_slice[icurrent_img_index]);
					num_active_threads++;

					img = img_array[iNext_img_index];

					ret = activate_sps ARGS2(&(img_array[icurrent_img_index]->m_active_sps[IMGPAR structure == BOTTOM_FIELD]), IMGPAR currentSlice->viewId);		//Now for Base View only
					if (FAILED(ret)) {
						break;
					}
					activate_pps ARGS2(&(img_array[icurrent_img_index]->m_active_pps[IMGPAR structure == BOTTOM_FIELD]), IMGPAR currentSlice->viewIndex);
					IMGPAR current_slice_nr++;
					IMGPAR prevSlice = IMGPAR currentSlice;
					IMGPAR currentSlice = (Slice*) (IMGPAR currentSlice->nextSlice);
				}
			}
		}

        if (FAILED(ret)) {

			for ( j=0; j<num_active_threads; j++) {
				int iactive_index = j;
				if(bIsMVCFieldPic == TRUE)
				{
					if(j > 0)
						iactive_index = num_views + j - 1;
					else
						iactive_index = view_index;
				}
				WaitForSingleObject(stream_global->m_event_end_slice[iactive_index], INFINITE);
				ResetEvent(stream_global->m_event_end_slice[iactive_index]);
				nSliceCount--;
			}
			break;
		}

		if (bSmartDecode4Finished == FALSE)
		{
			for (j=0; j<i+1; j++)
			{
				int iactive_index = j;
				if(bIsMVCFieldPic == TRUE)
				{
					if(j > 0)
						iactive_index = num_views + j - 1;
					else
						iactive_index = view_index;
				}

				WaitForSingleObject(stream_global->m_event_end_slice[iactive_index], INFINITE);
				ResetEvent(stream_global->m_event_end_slice[iactive_index]);
				nSliceCount--;
				m_bLast_Slice = (iactive_index==icurrent_img_index);
				img = img_array[iactive_index];

                currSlice = IMGPAR currentSlice;

#if defined(_HW_ACCEL_)
				if(j!=0)
				{
					int ipre_active_index = iactive_index - 1;
					if(bIsMVCFieldPic == TRUE)
					{
						if(j == 1)
							ipre_active_index = view_index;
					}

					IMGPAR m_lpDEBLK_Luma = img_array[ipre_active_index]->m_lpDEBLK_Luma;
					IMGPAR m_lpDEBLK_Chroma = img_array[ipre_active_index]->m_lpDEBLK_Chroma;
				}

#endif
                if(active_pps.num_slice_groups_minus1 == 0)
                {
#if defined(_HW_ACCEL_)
                    if((!( (IMGPAR smart_dec & SMART_DEC_NODEBLK_ALL) || ((IMGPAR smart_dec & SMART_DEC_NODEBLK_B) && (currSlice->picture_type == B_SLICE)) )) && (g_DXVAVer==IviNotDxva))
#else
                    if(!( (IMGPAR smart_dec & SMART_DEC_NODEBLK_ALL) || ((IMGPAR smart_dec & SMART_DEC_NODEBLK_B) && (currSlice->picture_type == B_SLICE)) ))
#endif
                    {
                        if ( (IMGPAR error_mb_nr < 0) &&(stream_global->m_bSeekIDR == FALSE)) {
                            if (IMGPAR MbaffFrameFlag)
                                IMGPAR FP_DeblockSlice ARGS3(dec_picture, currSlice->start_mb_nr<<1, currSlice->read_mb_nr );
                            else
                                IMGPAR FP_DeblockSlice ARGS3(dec_picture, currSlice->start_mb_nr, currSlice->read_mb_nr );
                        }
                    }
                }
            }
        }
        else
        {
            int nNumOfSlice = nSliceCount;
            for (j=0; j<nNumOfSlice; j++)
            {
                nSliceCount--;
                IMGPAR current_slice_nr++;
                IMGPAR prevSlice = IMGPAR currentSlice;
                IMGPAR currentSlice = (Slice*) (IMGPAR currentSlice->nextSlice);
            }
            IMGPAR currentSlice = IMGPAR prevSlice;
        }

        if (IMGPAR currentSlice->exit_flag)
        {
            if(active_pps.num_slice_groups_minus1 > 0)
            {
#if defined(_HW_ACCEL_)
                if((!( (IMGPAR smart_dec & SMART_DEC_NODEBLK_ALL) || ((IMGPAR smart_dec & SMART_DEC_NODEBLK_B) && (currSlice->picture_type == B_SLICE)) )) && (g_DXVAVer==IviNotDxva))
#else
                if(!( (IMGPAR smart_dec & SMART_DEC_NODEBLK_ALL) || ((IMGPAR smart_dec & SMART_DEC_NODEBLK_B) && (currSlice->picture_type == B_SLICE)) ))
#endif
                {
                    if ( (IMGPAR error_mb_nr < 0) &&(stream_global->m_bSeekIDR == FALSE)) {
                        if (IMGPAR MbaffFrameFlag)
                            IMGPAR FP_DeblockSlice ARGS3(dec_picture, 0, dec_picture->decoded_mb_number );
                        else
                            IMGPAR FP_DeblockSlice ARGS3(dec_picture, 0, dec_picture->decoded_mb_number );
                    }
                }
            }
#if defined (_HW_ACCEL_)
            if(g_DXVAVer && IMGPAR Hybrid_Decoding)
            {
                if(IMGPAR Hybrid_Decoding == 5 || (IMGPAR Hybrid_Decoding == 2 && IMGPAR type != B_SLICE) || IMGPAR type == I_SLICE)
                    DMA_Transfer ARGS0();
            }
#endif
            ret = exit_picture ARGS0();
            if (FAILED(ret)) {
                //return ret;
                break;
            }
        }

        if (FAILED(ret)) {
            break;
        }

		if (nSliceCount)
		{
			int istart_index = 0;

			if(bIsMVCFieldPic == TRUE)
				istart_index = view_index;

			img_array[istart_index]->m_pic_combine_status = pic_combine_status;
			img_array[istart_index]->m_dec_picture = IMGPAR m_dec_picture;
			img_array[istart_index]->m_dec_picture_top = IMGPAR m_dec_picture_top;
			img_array[istart_index]->m_dec_picture_bottom = IMGPAR m_dec_picture_bottom;
			img_array[istart_index]->m_prev_dec_picture = IMGPAR m_prev_dec_picture;
			img_array[istart_index]->current_slice_nr = IMGPAR current_slice_nr;
			img_array[istart_index]->firstSlice = IMGPAR firstSlice;
			img_array[istart_index]->prevSlice = IMGPAR prevSlice;
			img_array[istart_index]->currentSlice = IMGPAR currentSlice;
			img_array[istart_index]->slice_number = IMGPAR slice_number;
			img_array[istart_index]->SkipThisFrame = IMGPAR SkipThisFrame;
			img_array[istart_index]->m_clip_max_x = IMGPAR m_clip_max_x;
			img_array[istart_index]->m_clip_max_x_cr = IMGPAR m_clip_max_x_cr;
			img_array[istart_index]->smart_dec = IMGPAR smart_dec;
			img_array[istart_index]->number = IMGPAR number;
			img_array[istart_index]->pre_frame_num = IMGPAR pre_frame_num;
			img_array[istart_index]->PreviousFrameNum = IMGPAR PreviousFrameNum;
			img_array[istart_index]->PreviousFrameNumOffset = IMGPAR PreviousFrameNumOffset;
			img_array[istart_index]->PreviousPOC = IMGPAR PreviousPOC;
			img_array[istart_index]->ThisPOC = IMGPAR ThisPOC;
			img_array[istart_index]->last_has_mmco_5 = IMGPAR last_has_mmco_5;
			img_array[istart_index]->PrevPicOrderCntMsb = IMGPAR PrevPicOrderCntMsb;
			img_array[istart_index]->PrevPicOrderCntLsb = IMGPAR PrevPicOrderCntLsb;
#if defined(_HW_ACCEL_)
			if ( (IviDxva1==g_DXVAVer)&& (IMGPAR Hybrid_Decoding!=5) )
			{
				img_array[istart_index]->m_lpMBLK_Intra_Luma = IMGPAR m_lpMBLK_Intra_Luma;
				img_array[istart_index]->m_lpMV = IMGPAR m_lpMV;
				if ( g_DXVAMode!=E_H264_DXVA_ATI_PROPRIETARY_A )
					img_array[istart_index]->m_lpRESD_Intra_Luma = IMGPAR m_lpRESD_Intra_Luma;
			}
			img_array[istart_index]->m_iFirstCompBufIndex = IMGPAR m_iFirstCompBufIndex;
			img_array[istart_index]->m_Intra_lCompBufIndex = IMGPAR m_Intra_lCompBufIndex;
			img_array[istart_index]->m_lFrame_Counter = IMGPAR m_lFrame_Counter;
			img_array[istart_index]->UnCompress_Idx = IMGPAR UnCompress_Idx;
			img_array[istart_index]->m_lpRESD_DXVA = IMGPAR m_lpRESD_DXVA;
			img_array[istart_index]->m_pUnCompressedBuffer = IMGPAR m_pUnCompressedBuffer;
			img_array[istart_index]->m_UnCompressedBufferStride = IMGPAR m_UnCompressedBufferStride;
#endif	

			img = img_array[istart_index];
			IMGPAR current_slice_nr++;
			IMGPAR prevSlice = IMGPAR currentSlice;
			IMGPAR currentSlice = (Slice*) (IMGPAR currentSlice->nextSlice);
		}
	}

    if (FAILED(ret)) {
        DEBUG_SHOW_ERROR_INFO("[ERROR] DecodeOneFrame Fail! Reset Buffer!");

        if (IMGPAR structure == FRAME) {
            
            if(dec_picture)
            {
                if ((dpb.used_size_on_view[view_index] == 0) ||dpb.fs_on_view[view_index][dpb.used_size_on_view[view_index] - 1]->frame != dec_picture) {
                    release_storable_picture ARGS2(dec_picture, 1);
                }
                dec_picture = NULL;
            }
            img->m_dec_picture_top = NULL;
            img->m_dec_picture_bottom = NULL;


        } else {

            if (img->m_dec_picture_top && (img->m_dec_picture_top->top_field_first)) {

                if(img->m_dec_picture_top && img->m_dec_picture_bottom)
                {
                    if ((dpb.used_size_on_view[view_index] == 0) ||dpb.fs_on_view[view_index][dpb.used_size_on_view[view_index] - 1]->bottom_field != img->m_dec_picture_bottom) {
                        release_storable_picture ARGS2(img->m_dec_picture_bottom, 1);
                    }
                    
                } else {

                    if ((dpb.used_size_on_view[view_index] == 0) ||dpb.fs_on_view[view_index][dpb.used_size_on_view[view_index] - 1]->top_field != img->m_dec_picture_top) {
                        release_storable_picture ARGS2(img->m_dec_picture_top, 1);                    
                    }

                }
                img->m_dec_picture_top = NULL;
                img->m_dec_picture_bottom = NULL;


            } else {

                if(img->m_dec_picture_top && img->m_dec_picture_bottom)
                {
                    if ((dpb.used_size_on_view[view_index] == 0) ||dpb.fs_on_view[view_index][dpb.used_size_on_view[view_index] - 1]->top_field != img->m_dec_picture_top) {
                        release_storable_picture ARGS2(img->m_dec_picture_top, 1);
                    }

                } else {

                    if ((dpb.used_size_on_view[view_index] == 0) ||dpb.fs_on_view[view_index][dpb.used_size_on_view[view_index] - 1]->bottom_field != img->m_dec_picture_bottom) {
                        release_storable_picture ARGS2(img->m_dec_picture_bottom, 1);
                    }

                }
                img->m_dec_picture_top = NULL;
                img->m_dec_picture_bottom = NULL;


            }        


            dec_picture = NULL;

        }    


    }

	for (i=0 ;i<nNeedToReleaseSlice; i++)
	{
		int irelease_index = i;
		if(bIsMVCFieldPic == TRUE)
		{
			if(i > 0)
				irelease_index = num_views + i - 1;
			else
				irelease_index = view_index;
		}
		
		img = img_array[irelease_index];
		if (i==0)
		{
			IMGPAR currentSlice = IMGPAR firstSlice;
			for (j=0; j< IMGPAR slice_number; j++)
			{
				//active_pps = &PicParSet[IMGPAR currentSlice->pic_parameter_set_id];
				memcpy(&active_pps, &PicParSet[IMGPAR currentSlice->pic_parameter_set_id], sizeof(pic_parameter_set_rbsp_t));
				IMGPAR prevSlice = IMGPAR currentSlice;
				IMGPAR currentSlice = (Slice*) IMGPAR currentSlice->nextSlice;
				free_new_slice(IMGPAR prevSlice);
			}
		}
		IMGPAR firstSlice = IMGPAR currentSlice = IMGPAR prevSlice = NULL;
		IMGPAR current_mb_nr_r = IMGPAR current_mb_nr_d = -4712;   // impossible value for debugging, StW
		IMGPAR error_mb_nr = -4712;
		IMGPAR current_slice_nr = 0;
		IMGPAR slice_number = 0;
		img->m_active_pps[0].Valid = NULL;
		img->m_active_pps[1].Valid = NULL;
		dec_picture = NULL;
		img->m_dec_picture_top = NULL;
		img->m_dec_picture_bottom = NULL;

        if (IMGPAR structure != FRAME)
        {
            IMGPAR mb_decdata = IMGPAR mb_decdata_ori;
            IMGPAR cof_array = IMGPAR cof_array_ori;
        }
    }

    return ret;
}

#if defined(_COLLECT_PIC_)
BOOL CheckSkipAndPTSProcess PARGS1(BOOL bIsForReference)
{
    stream_par  *stream_global = img->stream_global;
    BOOL bSkip=FALSE;
    unsigned int view_index = (IMGPAR firstSlice) ? (IMGPAR firstSlice->viewIndex) : 0;

    if(IMGPAR firstSlice->picture_type == B_SLICE && (bIsForReference == FALSE || stream_global->bSeekToOpenGOP))            
    {
        //Terry: we should skip all B-frames between I and P frames after seeking.
        if(stream_global->bSeekToOpenGOP)
        {
            IMGPAR SkipThisFrame = TRUE;
            g_llDecCount[view_index]--;
        }
        else if ((IMGPAR smart_dec & SMART_DEC_SKIP_1B) || (IMGPAR smart_dec & SMART_DEC_SKIP_2B))
        {
            DEBUG_SHOW_SW_INFO("==Do FF==");
            if ((IMGPAR smart_dec & SMART_DEC_SKIP_2B) || (g_uSkipBFrameCounter[0] == 0))
            {
                g_llDtsCount[view_index]--;
                IMGPAR SkipThisFrame = TRUE;
                g_dwSkipFrameCounter++;
                g_uSkipBFrameCounter[view_index][0]++;
                g_uSkipBFrameCounter[view_index][1] += IMGPAR firstSlice->NumClockTs;
            }
        }
        else if(g_llDecCount[view_index] > NO_SKIP_FRAME_FOR_PULLDOWN)
        {
            if(IMGPAR smart_dec & (SMART_DEC_SKIP_PB | SMART_DEC_ONLY_IDR))
            {
                g_llDtsCount[view_index]--;
                IMGPAR SkipThisFrame = TRUE;
                g_dwSkipFrameCounter++;
                g_uSkipBFrameCounter[view_index][0]++;
                g_uSkipBFrameCounter[view_index][1] += IMGPAR firstSlice->NumClockTs;
            }
            else if(IMGPAR SkipControlB && (stream_global->nCollectB&1 || g_bSkipFirstB))
            {
                if(g_bSkipFirstB && g_uSkipBFrameCounter[view_index][0])
                {
                    IMGPAR SkipedBFrames[view_index][0] = g_uSkipBFrameCounter[view_index][0];
                    IMGPAR SkipedBFrames[view_index][1] = g_uSkipBFrameCounter[view_index][1];
                    g_uSkipBFrameCounter[view_index][0] = g_uSkipBFrameCounter[view_index][1] = 0;
                }
                else
                {    //Terry: We just support check to skip B frame or not, now.
                    if(IMGPAR firstSlice->nal_reference_idc == 0)
                    {}//(*get_skip_fcn)(H264_pvSkipFrameContext, IMGPAR firstSlice->picture_type, &(IMGPAR firstSlice->dts), &bSkip);
                    if(bSkip)
                    {
                        g_llDtsCount[view_index]--;
                        IMGPAR SkipThisFrame = TRUE;
                        g_dwSkipFrameCounter++;
                        g_uSkipBFrameCounter[view_index][0]++;
                        g_uSkipBFrameCounter[view_index][1] += IMGPAR firstSlice->NumClockTs;
                    }
                }
                DEBUG_SHOW_SW_INFO("This B frame (%I64d) has DTS: %I64d and is skiped: %d.\n", g_llDtsCount, IMGPAR firstSlice->dts.ts, bSkip);
            }
        }
    }
    else if( (g_llDecCount[view_index] > NO_SKIP_FRAME_FOR_PULLDOWN) && 
        ( ((IMGPAR smart_dec & SMART_DEC_SKIP_PB) && IMGPAR firstSlice->picture_type == P_SLICE) || 
        ((IMGPAR smart_dec & SMART_DEC_ONLY_IDR) && !IMGPAR firstSlice->idr_flag) ) )
    {
        g_llDtsCount[view_index]--;
        IMGPAR SkipThisFrame = TRUE;
        g_dwSkipFrameCounter++;
        //g_uSkipBFrameCounter[0]++;
        //g_uSkipBFrameCounter[1] += IMGPAR firstSlice->NumClockTs;
    } 
    else if(g_uSkipBFrameCounter[view_index][0]) 
    {
        if(IMGPAR smart_dec & (SMART_DEC_SKIP_PB | SMART_DEC_ONLY_IDR))
        {
            IMGPAR SkipedBFrames[view_index][0] = g_uSkipBFrameCounter[view_index][0];
            IMGPAR SkipedBFrames[view_index][1] = g_uSkipBFrameCounter[view_index][1];
        }
        else if (IMGPAR SkipControlB)
        {
            if(stream_global->m_is_MTMS && dpb.used_size_on_view[view_index]) 
            {
                unsigned int pos = 0;
                int poc = INT_MIN;
                //Terry: We use the POC to find out the next and closest I/P frame in the dpb.
                for(unsigned int i=0; i<dpb.used_size_on_view[view_index]; i++)
                {
                    if(dpb.fs_on_view[view_index][i]->poc > poc)
                    {
                        poc = dpb.fs_on_view[view_index][i]->poc;
                        pos = i;
                    }
                }
                (dpb.fs_on_view[view_index][pos]->frame)->SkipedBFrames[view_index][0] = g_uSkipBFrameCounter[view_index][0];
                (dpb.fs_on_view[view_index][pos]->frame)->SkipedBFrames[view_index][1] = g_uSkipBFrameCounter[view_index][1];
            }
            else
            {
                img_array[(stream_global->pic_ip_num+1)&1]->SkipedBFrames[view_index][0] = g_uSkipBFrameCounter[view_index][0];
                img_array[(stream_global->pic_ip_num+1)&1]->SkipedBFrames[view_index][1] = g_uSkipBFrameCounter[view_index][1];
            }
        }
        g_uSkipBFrameCounter[view_index][0] = g_uSkipBFrameCounter[view_index][1] = 0;
    }

    DEBUG_SHOW_SW_INFO("SW Check Skip, SkipThisFrame = %d, POC = %d, Type = %d, bIsForReference = %d", IMGPAR SkipThisFrame, IMGPAR ThisPOC, IMGPAR firstSlice->picture_type, bIsForReference);

    return bSkip;
}

void RearrangePBCCBuf PARGS0()
{
    //LINE21BUF_SIZE-1 = 1; rearranged 
    //LINE21BUF_SIZE-3 = nBUnit;
    //LINE21BUF_SIZE-4 = nPUnit;
    stream_par  *stream_global = img->stream_global;

    BOOL bRearranged; 
    int nBUnit = 0, nIPUnit;

    if (g_pArrangedPCCCode) {
        bRearranged = g_pArrangedPCCCode->ccbuf[LINE21BUF_SIZE-1];
        nIPUnit =g_pArrangedPCCCode->ccbuf[20]&0x1f;
    } else {
        return;
    }

    if (bRearranged)
    {
        nBUnit = g_pArrangedPCCCode->ccbuf[LINE21BUF_SIZE-3]; 
        nIPUnit = g_pArrangedPCCCode->ccbuf[LINE21BUF_SIZE-4];
    }
    int offset =19;
    unsigned char *payload =  &(IMGPAR m_CCCode.ccbuf[0]); // B ccbuf to be insterted

    int nInCCCount = payload[20]&0x1f;
    if (nInCCCount==0) return ;    //content error.

    if (g_pArrangedPCCCode->cclen == 0) //previous I or P frame has no any CCCode
    {
        memcpy(g_pArrangedPCCCode, &(IMGPAR m_CCCode), sizeof(H264_CC));

        g_pArrangedPCCCode->ccbuf[LINE21BUF_SIZE-3] = nInCCCount;
        g_pArrangedPCCCode->ccbuf[LINE21BUF_SIZE-4] = 0;
        g_pArrangedPCCCode->ccbuf[LINE21BUF_SIZE-1] = 1; // Rearranged

        payload[20] = 0;
        payload[21] = 0;
        payload[22] = 0xff;
        IMGPAR m_CCCode.cclen = 23;
        IMGPAR m_CCCode.ccnum = 0;

        return;
    }

    offset+=2; //skip 1byte nCCCount and 1byte em_data;

    int nCCount = g_pArrangedPCCCode->ccbuf[20]&0x1f;
    nCCount+= nInCCCount;
    g_pArrangedPCCCode->ccbuf[20] = 0;
    g_pArrangedPCCCode->ccbuf[20] |= nCCount; 

    g_pArrangedPCCCode->ccnum = nCCount;

    int nMarkPos = g_pArrangedPCCCode->cclen>0?g_pArrangedPCCCode->cclen-1:0;
    while(g_pArrangedPCCCode->ccbuf[nMarkPos]!=0xff && nMarkPos>0)
        nMarkPos--;
    g_pArrangedPCCCode->cclen = nMarkPos + 1;

    int nBPos =g_pArrangedPCCCode->cclen -1, nIPPos=0;

    for (int jj=0; jj<nIPUnit; jj++)
    {
        while(g_pArrangedPCCCode->ccbuf[nMarkPos]!=0xfc && g_pArrangedPCCCode->ccbuf[nMarkPos]!=0xfd && nMarkPos>21)
            nMarkPos--;
        nIPPos = nMarkPos--;
    }

    for (int n=0; n<nInCCCount; n++)
    {
        offset = 22 + n*3;

        g_pArrangedPCCCode->ccbuf[g_pArrangedPCCCode->cclen-1] = payload[offset];
        g_pArrangedPCCCode->ccbuf[g_pArrangedPCCCode->cclen]   = payload[offset+1]&0x7f;
        g_pArrangedPCCCode->ccbuf[g_pArrangedPCCCode->cclen+1] = payload[offset+2]&0x7f;
        g_pArrangedPCCCode->ccbuf[g_pArrangedPCCCode->cclen+2] =0xff;
        g_pArrangedPCCCode->cclen += 3;

        nBUnit++;
    }

    g_pArrangedPCCCode->ccbuf[LINE21BUF_SIZE-3] = nBUnit;
    g_pArrangedPCCCode->ccbuf[LINE21BUF_SIZE-4] = nIPUnit;

    if (nBUnit) 
        g_pArrangedPCCCode->ccbuf[LINE21BUF_SIZE-1]=1; // Rearranged
    if (nIPUnit)
    {
        memcpy( g_pArrangedPCCCode->ccbuf + g_pArrangedPCCCode->cclen-1, g_pArrangedPCCCode->ccbuf +nIPPos, g_pArrangedPCCCode->cclen-nIPPos);
        memcpy( g_pArrangedPCCCode->ccbuf + nIPPos, g_pArrangedPCCCode->ccbuf + nBPos, g_pArrangedPCCCode->cclen-nIPPos );
    }
    g_pArrangedPCCCode->ccbuf[g_pArrangedPCCCode->cclen-1] = 0xff;

    payload[20] = 0;
    payload[21] = 0;
    payload[22] = 0xff;
    IMGPAR m_CCCode.cclen = 23;
    IMGPAR m_CCCode.ccnum = 0;
}

void ProcessSkipAndPTS PARGS3(BOOL &bSkip, BOOL bIsForReference, BOOL bFastSeek)
{
    stream_par  *stream_global = img->stream_global;
    unsigned int view_index = (IMGPAR firstSlice) ? (IMGPAR firstSlice->viewIndex) : 0;

    if(IMGPAR firstSlice->picture_type == B_SLICE && (bIsForReference == FALSE || stream_global->bSeekToOpenGOP))            
    {
        //Terry: we should skip all B-frames between I and P frames after seeking.
        if(stream_global->bSeekToOpenGOP)
        {
            IMGPAR SkipThisFrame = TRUE;
            g_llDecCount[view_index]--;
        }
        else if ((IMGPAR smart_dec & SMART_DEC_SKIP_1B) || (IMGPAR smart_dec & SMART_DEC_SKIP_2B))
        {
            DEBUG_SHOW_SW_INFO("==Do FF==");
            if ((IMGPAR smart_dec & SMART_DEC_SKIP_2B) || (g_uSkipBFrameCounter[view_index][0] == 0))
            {
                g_llDtsCount[view_index]--;
                IMGPAR SkipThisFrame = TRUE;
                g_dwSkipFrameCounter++;
                g_uSkipBFrameCounter[view_index][0]++;
                g_uSkipBFrameCounter[view_index][1] += IMGPAR firstSlice->NumClockTs;
            }
        }
        else if (bSkip && bFastSeek)
        {
            g_llDtsCount[view_index]--;
            IMGPAR SkipThisFrame = TRUE;
            g_dwSkipFrameCounter++;
        }
        else if(g_llDecCount[view_index] > NO_SKIP_FRAME_FOR_PULLDOWN)
        {
            if(IMGPAR smart_dec & (SMART_DEC_SKIP_PB | SMART_DEC_ONLY_IDR))
            {
                g_llDtsCount[view_index]--;
                IMGPAR SkipThisFrame = TRUE;
                g_dwSkipFrameCounter++;
                g_uSkipBFrameCounter[view_index][0]++;
                g_uSkipBFrameCounter[view_index][1] += IMGPAR firstSlice->NumClockTs;
            }
            else if(IMGPAR SkipControlB && (stream_global->nCollectB&1 || g_bSkipFirstB))
            {
                if(g_bSkipFirstB && g_uSkipBFrameCounter[view_index][0])
                {
                    IMGPAR SkipedBFrames[view_index][0] = g_uSkipBFrameCounter[view_index][0];
                    IMGPAR SkipedBFrames[view_index][1] = g_uSkipBFrameCounter[view_index][1];
                    g_uSkipBFrameCounter[view_index][0] = g_uSkipBFrameCounter[view_index][1] = 0;
                }
                else
                {
                    if(bSkip)
                    {
                        g_llDtsCount[view_index]--;
                        IMGPAR SkipThisFrame = TRUE;
                        g_dwSkipFrameCounter++;
                        g_uSkipBFrameCounter[view_index][0]++;
                        g_uSkipBFrameCounter[view_index][1] += IMGPAR firstSlice->NumClockTs;
                    }
                }
                DEBUG_SHOW_SW_INFO("This B frame (%I64d) has DTS: %I64d and is skiped: %d.\n", g_llDtsCount, IMGPAR firstSlice->dts.ts, bSkip);
            }
        }
    }
    else if ( (g_llDecCount[view_index] > NO_SKIP_FRAME_FOR_PULLDOWN) && bSkip && IMGPAR firstSlice->picture_type != I_SLICE && bFastSeek != TRUE)
    {
        g_llDtsCount[view_index]--;
        IMGPAR SkipThisFrame = TRUE;
        g_dwSkipFrameCounter++;
    }
    else if( (g_llDecCount[view_index] > NO_SKIP_FRAME_FOR_PULLDOWN) &&
        ( ((IMGPAR smart_dec & SMART_DEC_SKIP_PB) && IMGPAR firstSlice->picture_type == P_SLICE) || 
        ((IMGPAR smart_dec & SMART_DEC_ONLY_IDR) && !IMGPAR firstSlice->idr_flag) ) )
    {
        g_llDtsCount[view_index]--;
        IMGPAR SkipThisFrame = TRUE;
        g_dwSkipFrameCounter++;
        //g_uSkipBFrameCounter[0]++;
        //g_uSkipBFrameCounter[1] += IMGPAR firstSlice->NumClockTs;
    } 
    else if(g_uSkipBFrameCounter[view_index][0]) 
    {
        if(IMGPAR smart_dec & (SMART_DEC_SKIP_PB | SMART_DEC_ONLY_IDR))
        {
            IMGPAR SkipedBFrames[view_index][0] = g_uSkipBFrameCounter[view_index][0];
            IMGPAR SkipedBFrames[view_index][1] = g_uSkipBFrameCounter[view_index][1];
        }
        else if (IMGPAR SkipControlB)
        {
            if((stream_global->m_is_MTMS || stream_global->m_bIsSingleThreadMode) && dpb.used_size_on_view[view_index]) 
            {
                unsigned int pos = 0;
                int poc = INT_MIN;
                //Terry: We use the POC to find out the next and closest I/P frame in the dpb.
                for(unsigned int i=0; i<dpb.used_size_on_view[view_index]; i++)
                {
                    if(dpb.fs_on_view[view_index][i]->poc > poc)
                    {
                        poc = dpb.fs_on_view[view_index][i]->poc;
                        pos = i;
                    }
                }
                (dpb.fs_on_view[view_index][pos]->frame)->SkipedBFrames[view_index][0] = g_uSkipBFrameCounter[view_index][0];
                (dpb.fs_on_view[view_index][pos]->frame)->SkipedBFrames[view_index][1] = g_uSkipBFrameCounter[view_index][1];
            }
            else
            {
                img_array[(stream_global->pic_ip_num+1)&1]->SkipedBFrames[view_index][0] = g_uSkipBFrameCounter[view_index][0];
                img_array[(stream_global->pic_ip_num+1)&1]->SkipedBFrames[view_index][1] = g_uSkipBFrameCounter[view_index][1];
            }
        }
        g_uSkipBFrameCounter[view_index][0] = g_uSkipBFrameCounter[view_index][1] = 0;
    }
    bSkip = IMGPAR SkipThisFrame;
    DEBUG_SHOW_SW_INFO("SW Check Skip, SkipThisFrame = %d, POC = %d, Type = %d, bIsForReference = %d", IMGPAR SkipThisFrame, IMGPAR ThisPOC, IMGPAR firstSlice->picture_type, bIsForReference);
}

void UpdatePTS PARGS0()
{
    stream_par *stream_global = IMGPAR stream_global;
    unsigned int view_index = (IMGPAR firstSlice) ? (IMGPAR firstSlice->viewIndex) : 0;

	if(view_index > 0)
	{
		IMGPAR currentSlice->pts = g_pts_baseview;
		IMGPAR currentSlice->dts = g_dts_baseview;
		IMGPAR currentSlice->has_pts = g_has_pts_baseview;
	}
	else
	{
    if (g_has_pts)
    {
        g_has_pts = 0;
        nalu->pts = g_pts;
        DEBUG_INFO("[Update PTS]: ts: %I64d freq: %d, tslength: %d flags: %d unused1: %d unused2: %d", nalu->pts.ts, nalu->pts.freq, nalu->pts.tslength, nalu->pts.flags, nalu->pts.unused1, nalu->pts.unused2);
    }

    IMGPAR currentSlice->dts = nalu->pts;

    if(g_bReceivedFirst == 0)
        g_has_pts_poc = IMGPAR ThisPOC;

    if(nalu->pts.ts)
    {
        IMGPAR currentSlice->has_pts = 1;
        IMGPAR currentSlice->pts = nalu->pts;
        g_has_pts_poc = IMGPAR ThisPOC;
    }
    else
    {
        IMGPAR currentSlice->dts = g_pts;
        IMGPAR currentSlice->has_pts = 0;
        IMGPAR currentSlice->dts.ts += (unsigned __int64) (g_llDtsCount[view_index] * (((unsigned __int64)IMGPAR currentSlice->dts.freq * g_PulldownRate) / IMGPAR currentSlice->framerate1000));
        IMGPAR currentSlice->pts = IMGPAR currentSlice->dts;  //Just let first frame has ts value for g_ref_pts.ts != 0 in write_out_picture_h264_interface(), for frame has no pts
        // For special P frames after I, their POC are smaller than the I's.
        if ( (IMGPAR currentSlice->picture_type == P_SLICE) && (IMGPAR ThisPOC < g_has_pts_poc) )
        {
            IMGPAR currentSlice->pts.ts = g_pts.ts;
            IMGPAR currentSlice->pts.freq = g_pts.freq;
        }
    }
	}

	if(view_index == 0)
	{
		g_pts_baseview = IMGPAR currentSlice->pts;
		g_dts_baseview = IMGPAR currentSlice->dts;
		g_has_pts_baseview = IMGPAR currentSlice->has_pts;
	}
}

//void CH264VDecHP::DecodeProcessBB PARGS0()
void DecodeProcessBB PARGS0()
{
    stream_par  *stream_global = img->stream_global;

    int nImgB_rd0 = (stream_global->nSwitchB<<1) + 2;
    int nImgB_rd1 = (stream_global->nSwitchB<<1) + 3;

    if (stream_global->b_Count == 3 && stream_global->two_B_Flag)
    {
        gImgB_r1 = img_array[nImgB_rd1];
        SetEvent(stream_global->m_event_read_start_b[1]);
    }
    if (stream_global->b_Count <= 2)
    {
        gImgB_r0 = img_array[nImgB_rd0];
        SetEvent(stream_global->m_event_read_start_b[0]);
        if (stream_global->two_B_Flag)
        {
            gImgB_r1 = img_array[nImgB_rd1];
            SetEvent(stream_global->m_event_read_start_b[1]);
        }
    }

    WaitForSingleObject(stream_global->m_event_decode_finish_b[0], INFINITE);
    if (stream_global->two_B_Flag)
        WaitForSingleObject(stream_global->m_event_decode_finish_b[1], INFINITE);
    ResetEvent(stream_global->m_event_decode_finish_b[0]);
    ResetEvent(stream_global->m_event_decode_finish_b[1]);

    stream_global->nSwitchB = !(stream_global->nSwitchB);
    stream_global->b_Count -= 2;
}

//void CH264VDecHP::DecodeProcessRB PARGS0()
void DecodeProcessRB PARGS0()
{
    stream_par  *stream_global = img->stream_global;
    int nImgB_rd0, nImgB_rd1;
    int nImgIP_r = stream_global->pic_ip_num;
    int nImgIP_d = !(nImgIP_r);

    stream_global->b_Count--;

    if (stream_global->nNeedToDecodeIP)
    {
        gImgIP_d = img_array[nImgIP_d];
        SetEvent(stream_global->m_event_decode_start_ip);

        //B read & decode
        if (stream_global->bHasB)
            DecodeProcessBB ARGS0();

        SetEvent(stream_global->m_event_for_field_ip);
        WaitForSingleObject(stream_global->m_event_decode_finish_ip, INFINITE);
        ResetEvent(stream_global->m_event_decode_finish_ip);
        stream_global->nNeedToDecodeIP = 0;
    }

    if (stream_global->b_Count & 1)
    {
        stream_global->pic_b_num++;
        if (stream_global->pic_b_num == 4)
            stream_global->pic_b_num = 0;
    }

    if (stream_global->b_Count)
    {
        if (stream_global->b_Count & 1)
            stream_global->two_B_Flag = 0;
        else
            stream_global->two_B_Flag = 1;

        nImgB_rd0 = (stream_global->nSwitchB<<1) + 2;
        nImgB_rd1 = (stream_global->nSwitchB<<1) + 3;

        gImgB_r0 = img_array[nImgB_rd0];
        SetEvent(stream_global->m_event_read_start_b[0]);
        if (stream_global->two_B_Flag)
        {
            gImgB_r1 = img_array[nImgB_rd1];
            SetEvent(stream_global->m_event_read_start_b[1]);
        }

        WaitForSingleObject(stream_global->m_event_decode_finish_b[0], INFINITE);
        if (stream_global->two_B_Flag)
            WaitForSingleObject(stream_global->m_event_decode_finish_b[1], INFINITE);
        ResetEvent(stream_global->m_event_decode_finish_b[0]);
        ResetEvent(stream_global->m_event_decode_finish_b[1]);

        stream_global->nSwitchB = !(stream_global->nSwitchB);
        stream_global->b_Count -= stream_global->two_B_Flag == 0 ? 1:2;
    }

    //Process this Reference-B
#if defined(_HW_ACCEL_)
    if (g_DXVAVer != IviNotDxva)
    {
        gImgB_r0 = img_array[6];
        SetEvent(stream_global->m_event_read_start_b[0]);
        WaitForSingleObject(stream_global->m_event_decode_finish_b[0], INFINITE);
        ResetEvent(stream_global->m_event_decode_finish_b[0]);
    }
    else
#endif
    {
        SetEvent(stream_global->m_event_start_pic_RB);
        WaitForSingleObject(stream_global->m_event_finish_pic_RB, INFINITE);
        ResetEvent(stream_global->m_event_finish_pic_RB);
    }

    if (stream_global->b_Count >= 2)
        stream_global->bHasB = 1;
    else
        stream_global->bHasB = 0;
}
#endif

#if !defined(_COLLECT_PIC_)
int DecodePicture_SingleThread PARGS0()
{
    int header, pre_framecount, framecount;
    Slice *currSlice;

    pre_framecount = g_framemgr->GetDisplayCount();
    if(g_framemgr->m_nFlushDPBsize)
        pre_framecount += dpb.used_size;

    if (g_dwSmartDecLevel&SMART_DEC_INT_PEL_Y)
    {
        get_block_4xh = get_block_4xh_int;
        get_block_8xh = get_block_8xh_int;
        get_block_16xh = get_block_16xh_int;
    }
    else
    {
        get_block_4xh = get_block_4xh_full;
        get_block_8xh = get_block_8xh_full;
        get_block_16xh = get_block_16xh_full;
    }

    set_4xH_mc_function_ptr ARGS0();

    if (g_dwSmartDecLevel&SMART_DEC_INT_PEL_UV)
    {
        mb_chroma_2xH = mb_chroma_2xH_int;
        mb_chroma_4xH = mb_chroma_4xH_int;
        mb_chroma_8xH = mb_chroma_8xH_int;
    }
    else
    {
        mb_chroma_2xH = mb_chroma_2xH_full;
        mb_chroma_4xH = mb_chroma_4xH_full;
        mb_chroma_8xH = mb_chroma_8xH_full;
    }

    set_2xH_mc_function_ptr ARGS0();

    currSlice = IMGPAR currentSlice;
    while(currSlice->next_header!=EOS && currSlice->next_header!=SOP)
    {
        header = read_new_slice ARGS0();
        if(header<0)
            return header;
        // IoK: check for HD-profile violation due to incorrect nal_unit_type (2-4), ASO or incorrect slice types (SI/SP)
        // current_header == -1, it means HD-profile violation and the reason is recorded in g_HDProfileFault.
        else if(header==EOS)
        {
            g_bEOS = true;
            ret = exit_picture ARGS0();
            if (FAILED(ret)) {
                return ret;
            }

            if(dpb.used_size)
            {
                flush_dpb ARGS1(0);
                return 0;
            }
            return -1;    // no more frames
        }
        read_slice ARGS1(header);

#ifdef _HW_ACCEL_
        if(IMGPAR de_blocking_flag && (g_DXVAVer==IviNotDxva))
#else
        if(IMGPAR de_blocking_flag)
#endif
        {
            IMGPAR FP_DeblockSlice    = DeblockSlice;

            if (IMGPAR MbaffFrameFlag)
                IMGPAR FP_DeblockSlice ARGS3(dec_picture, currSlice->start_mb_nr<<1, (IMGPAR current_mb_nr_r-(currSlice->start_mb_nr<<1)) );
            else
                IMGPAR FP_DeblockSlice ARGS3(dec_picture, currSlice->start_mb_nr, (IMGPAR current_mb_nr_r-currSlice->start_mb_nr) );
        }
#if defined(_HW_ACCEL_)
        if(g_DXVAVer && IMGPAR Hybrid_Decoding && (IMGPAR type == I_SLICE))
        {
            if(IMGPAR de_blocking_flag)
            {
                IMGPAR FP_DeblockSlice    = DeblockSlice;

                if (IMGPAR MbaffFrameFlag)
                    IMGPAR FP_DeblockSlice ARGS3(dec_picture, currSlice->start_mb_nr<<1, (IMGPAR current_mb_nr_r-(currSlice->start_mb_nr<<1)) );
                else
                    IMGPAR FP_DeblockSlice ARGS3(dec_picture, currSlice->start_mb_nr, (IMGPAR current_mb_nr_r-currSlice->start_mb_nr) );
            }
            DMA_Transfer ARGS0();
        }
#endif

        IMGPAR current_slice_nr++;        
        framecount = g_framemgr->GetDisplayCount();
        if(g_framemgr->m_nFlushDPBsize)
            framecount += dpb.used_size;
        if(framecount > pre_framecount)
            return 0;        
    }        
    ret = exit_picture ARGS0();
    if (FAILED(ret)) {
        return ret;
    }
    IMGPAR current_slice_nr = 0;
    IMGPAR current_mb_nr_r = IMGPAR current_mb_nr_d = -4711;     // initialized to an impossible value for debugging -- correct value is taken from slice header
    IMGPAR num_dec_mb = 0;
    currSlice->next_header = -8888; // initialized to an impossible value for debugging -- correct value is taken from slice header

#if !defined(ONE_COF)
    IMGPAR cof_r = IMGPAR cof_d = NULL;
#endif

    return 0;    // okay
}

#else

#if defined(_HW_ACCEL_)
int DecodePicture_HW_BA PARGS0() {

    int header=0, pre_framecount, framecount;
    int j, ret=0;    
    stream_par  *stream_global = img->stream_global;
    unsigned int view_index = (IMGPAR firstSlice) ? (IMGPAR firstSlice->viewIndex) : 0;

    pre_framecount = g_framemgr->GetDisplayCount();
    if(g_framemgr->m_nFlushDPBsize)
        pre_framecount += dpb.used_size_on_view[view_index];

    int pic_stor_idx;

    IMGPAR SkipedBFrames[view_index][0] = IMGPAR SkipedBFrames[view_index][1] = 0;
    IMGPAR SkipThisFrame = FALSE;

    stream_global->m_is_MTMS = 0;

	while(1)
	{
		IMGPAR number								= stream_global->number;
		IMGPAR pre_frame_num						= stream_global->pre_frame_num;
		IMGPAR PreviousFrameNum						= stream_global->PreviousFrameNum[0];
		IMGPAR PreviousFrameNumOffset				= stream_global->PreviousFrameNumOffset[0];
		IMGPAR PreviousPOC							= stream_global->PreviousPOC[0];    
		IMGPAR ThisPOC								= stream_global->ThisPOC[0];
		//Need to be reset in eject but can't be reset in seek, move to open
		//IMGPAR PrevPicOrderCntLsb					= stream_global->PrevPicOrderCntLsb;
		//IMGPAR PrevPicOrderCntMsb					= stream_global->PrevPicOrderCntMsb;
		IMGPAR last_has_mmco_5						= stream_global->last_has_mmco_5;

        //ret = HW_decode_one_picture ARGS1(&header);
        stream_global->number ++;

        if (IMGPAR slice_number == 0 || FAILED(ret))
        {
            DEBUG_SHOW_SW_INFO("[ERROR] Slice Number is 0 ==> return -1");
            return CREL_ERROR_H264_UNDEFINED;
        }

        g_llDtsCount[view_index] = ((IMGPAR firstSlice->has_pts) && (IMGPAR firstSlice->picture_type == I_SLICE)) ? 0 : (g_llDtsCount[view_index]+1);
        g_llDecCount[view_index]++;

        pic_stor_idx = IMGPAR UnCompress_Idx;

        if (stream_global->is_first)
        {
            stream_global->is_first = 0;
            stream_global->bSeekToOpenGOP = 1;
        }

        if(header==0 || pic_stor_idx==-1) // Frame skipped case
            g_dwSkipFrameCounter++;

        // release slice buffer
        IMGPAR currentSlice = IMGPAR firstSlice;
        for (j=0; j< IMGPAR slice_number; j++)
        {
            //active_pps = &PicParSet[IMGPAR currentSlice->pic_parameter_set_id];
            memcpy(&active_pps, &PicParSet[IMGPAR currentSlice->pic_parameter_set_id], sizeof(pic_parameter_set_rbsp_t));
            IMGPAR prevSlice = IMGPAR currentSlice;
            IMGPAR currentSlice = (Slice*) IMGPAR currentSlice->nextSlice;
            free_new_slice(IMGPAR prevSlice);
        }
        IMGPAR firstSlice = IMGPAR currentSlice = IMGPAR prevSlice = NULL;
        IMGPAR current_mb_nr_r = IMGPAR current_mb_nr_d = -4712;   // impossible value for debugging, StW
        IMGPAR error_mb_nr = -4712;
        IMGPAR current_slice_nr = 0;
        IMGPAR slice_number = 0;

        dec_picture = NULL;

        if (IMGPAR structure != FRAME)
        {
            IMGPAR mb_decdata = IMGPAR mb_decdata_ori;
            IMGPAR cof_array = IMGPAR cof_array_ori;
        }
        //end of release slice buffer

        if(header==EOS)
        {
            if(dpb.used_size_on_view[view_index]) {
                ret = flush_dpb ARGS1(0);
                if (FAILED(ret)) {
                    return ret;
                }
            }

            nalu_global_available = 0;    
            g_bReceivedFirst = 0;

            return -1;
        }

        framecount = g_framemgr->GetDisplayCount();
        if(g_framemgr->m_nFlushDPBsize)
            framecount += dpb.used_size_on_view[view_index];
        if(framecount > pre_framecount)
            break;
    }
    //check EOS to decode the last picture
    return 0;
}
#endif

int DecodePicture_MultiThread_MultiSlice PARGS0() {

    int nNumOfSkipBFrames = 0;
    int header, pre_framecount, framecount;
    int j, ret = 0;    
    int currThreadForB = 0;
    BOOL bSkip;    
    BOOL bIsForReference;
    stream_par  *stream_global = img->stream_global;
    static BOOL bAlreadyReset;
    int view_index = (IMGPAR firstSlice) ? (IMGPAR firstSlice->viewIndex) : 0;

    pre_framecount = g_framemgr->GetDisplayCount();
    if(g_framemgr->m_nFlushDPBsize)
        pre_framecount += dpb.used_size_on_view[view_index];

    g_bRewindDecoder = FALSE;

    //check EOS to decode the last picture
    if ( g_bEOS )
    {
        DEBUG_SHOW_SW_INFO("Enter EOS Flush Stage!!");
        img = img_array[0];

        if(dpb.used_size_on_view[view_index])
        {
            for (unsigned int i=0; i <storable_picture_count; i++)                
                storable_picture_map[i]->used_for_first_field_reference = 0;

            ret = flush_dpb ARGS1(view_index);
            if (FAILED(ret)) {
                return ret;
            }
        }

		nalu_global_available = 0;
		stream_global->number = 0;
		stream_global->pic_ip_num = 0;
		stream_global->pic_b_num = 0;
		stream_global->is_first = 1;
		stream_global->bHasB = 0;
		stream_global->b_Count = 0;
		stream_global->nSwitchB = 0;
		stream_global->two_B_Flag = 0; //two b frames or not
		stream_global->nNeedToDecodeIP = 0;
		stream_global->nCollectB = 0;
		next_image_type = -1;

        g_bReceivedFirst = 0;
        stream_global->m_is_MTMS = -1;


        g_bEOS = FALSE;
        return CREL_ERROR_H264_UNDEFINED;
    }//last picture

    while(1)
    {
        if (nalu_global->nal_reference_idc != 0)
            bIsForReference = TRUE;
        else
            bIsForReference = FALSE;

        if (stream_global->m_bSeekIDR == FALSE) {
            bAlreadyReset = FALSE;
        }

        if (stream_global->m_bSeekIDR && (bAlreadyReset == FALSE)){
            dpb.last_output_poc = INT_MIN;

            flush_dpb ARGS1(view_index);
            update_ref_list ARGS1(view_index);
            update_ltref_list ARGS1(view_index);        

            for (int i = 0; i < 4; i++ ) {

                img = img_array[i];
                
                IMGPAR currentSlice = IMGPAR firstSlice;
                for (int j=0; j< IMGPAR slice_number; j++)
                {
                    //active_pps = &PicParSet[IMGPAR currentSlice->pic_parameter_set_id];
                    memcpy(&active_pps, &PicParSet[IMGPAR currentSlice->pic_parameter_set_id], sizeof(pic_parameter_set_rbsp_t));
                    IMGPAR prevSlice = IMGPAR currentSlice;
                    IMGPAR currentSlice = (Slice*) IMGPAR currentSlice->nextSlice;
                    free_ref_pic_list_reordering_buffer(IMGPAR prevSlice);
                    free_new_slice(IMGPAR prevSlice);                    

                }

                if (IMGPAR structure == FRAME) {

                    if(dec_picture)
                    {
                        if ((dpb.used_size_on_view[view_index] == 0) ||dpb.fs_on_view[view_index][dpb.used_size_on_view[view_index] - 1]->frame != dec_picture) {
                            release_storable_picture ARGS2(dec_picture, 1);
                        }
                        dec_picture = NULL;
                    }
                    img->m_dec_picture_top = NULL;
                    img->m_dec_picture_bottom = NULL;

                } else {

                    if (img->m_dec_picture_top) {                        

                        release_storable_picture ARGS2(img->m_dec_picture_top, 1);                        
                        img->m_dec_picture_top = NULL;
                    }

                    if (img->m_dec_picture_bottom) {

                        release_storable_picture ARGS2(img->m_dec_picture_bottom, 1);                        
                        img->m_dec_picture_bottom = NULL;
                    }
                    dec_picture = NULL;

                }    

                IMGPAR firstSlice = IMGPAR currentSlice = IMGPAR prevSlice = NULL;
                IMGPAR current_mb_nr_r = IMGPAR current_mb_nr_d = -4712;   // impossible value for debugging, StW
                IMGPAR error_mb_nr = -4712;
                IMGPAR current_slice_nr = 0;
                IMGPAR slice_number = 0;
                img->m_active_pps[0].Valid = NULL;
                img->m_active_pps[1].Valid = NULL;
                //dec_picture = NULL;

                if (IMGPAR structure != FRAME)
                {
                    IMGPAR mb_decdata = IMGPAR mb_decdata_ori;
                    IMGPAR cof_array = IMGPAR cof_array_ori;
                }

            }

            img = img_array[0];

            bAlreadyReset = TRUE;
        }        


		IMGPAR number								= stream_global->number;
		IMGPAR pre_frame_num						= stream_global->pre_frame_num;
		IMGPAR PreviousFrameNum						= stream_global->PreviousFrameNum[0];
		IMGPAR PreviousFrameNumOffset				= stream_global->PreviousFrameNumOffset[0];
		IMGPAR PreviousPOC							= stream_global->PreviousPOC[0];    
		IMGPAR ThisPOC								= stream_global->ThisPOC[0];
		IMGPAR PrevPicOrderCntLsb					= stream_global->PrevPicOrderCntLsb[0];
		IMGPAR PrevPicOrderCntMsb					= stream_global->PrevPicOrderCntMsb[0];
		IMGPAR last_has_mmco_5						= stream_global->last_has_mmco_5;		
		for(j=0; j<6; j++)
			IMGPAR m_listXsize[j] = stream_global->m_listXsize[j];

        stream_global->last_has_mmco_5 = 0;

        //Terry: for multi-slice case, smart_dec should be assigned before collect picture.
        IMGPAR smart_dec = g_dwSmartDecLevel;

        if (stream_global->m_bMultiThreadModeSwitch == 0) {

            ret = read_new_picture ARGS1(&header);

            if (ISERROR(ret)) {
                return ret;
            }

            if ( ( header == SOP ) || ( header == NALU_TYPE_SPS ) ) {
                Check_MultiThreadModel ARGS0 ();
            }

            if (stream_global->m_bMultiThreadModeSwitch) {
                stream_global->m_is_MTMS = 0;

                return g_pfnDecodePicture ARGS0();
            }

        } else {
            header = SOP;
            stream_global->m_bMultiThreadModeSwitch = 0;
        }


        if (IMGPAR slice_number == 0 || FAILED(ret))
        {
            DEBUG_SHOW_SW_INFO("[ERROR] Slice Number is 0 ==> return -1");
            return CREL_ERROR_H264_UNDEFINED;
        }
        else if (IMGPAR currentSlice->m_pic_combine_status != FRAME && IMGPAR firstSlice->structure != FRAME)
        {
            DEBUG_SHOW_SW_INFO("[ERROR] Field Coding Frame! But Only receive one Picture!!");

            IMGPAR currentSlice = IMGPAR firstSlice;
            for (int i=0; i< IMGPAR slice_number; i++)
            {
                //active_pps = &PicParSet[IMGPAR currentSlice->pic_parameter_set_id];
                memcpy(&active_pps, &PicParSet[IMGPAR currentSlice->pic_parameter_set_id], sizeof(pic_parameter_set_rbsp_t));
                IMGPAR prevSlice = IMGPAR currentSlice;
                IMGPAR currentSlice = (Slice*) IMGPAR currentSlice->nextSlice;
                free_ref_pic_list_reordering_buffer(IMGPAR prevSlice);
                free_new_slice(IMGPAR prevSlice);
            }
            IMGPAR firstSlice = IMGPAR currentSlice = IMGPAR prevSlice = NULL;
            IMGPAR current_mb_nr_r = IMGPAR current_mb_nr_d = -4712;   // impossible value for debugging, StW
            IMGPAR error_mb_nr = -4712;
            IMGPAR current_slice_nr = 0;
            IMGPAR slice_number = 0;

            stream_global->m_bSeekIDR = TRUE;

            return CREL_ERROR_H264_UNDEFINED;
        }

        DEBUG_SHOW_SW_INFO("Collected One Picture\n header: %d  Slice Number: %d  SmartDecLevel: %d ", header, IMGPAR slice_number, g_dwSmartDecLevel);

        IMGPAR SkipedBFrames[view_index][0] = IMGPAR SkipedBFrames[view_index][1] = 0;
        IMGPAR SkipThisFrame = FALSE;

        g_llDtsCount[view_index] = ((IMGPAR firstSlice->has_pts) && (IMGPAR firstSlice->picture_type == I_SLICE)) ? 0 : (g_llDtsCount[view_index]+1);
        g_llDecCount[view_index]++;

        get_block_4xh  = get_block_4xh_full;
        get_block_8xh  = get_block_8xh_full;
        get_block_16xh = get_block_16xh_full;

        set_4xH_mc_function_ptr ARGS0();

        mb_chroma_2xH = mb_chroma_2xH_full;
        mb_chroma_4xH = mb_chroma_4xH_full;
        mb_chroma_8xH = mb_chroma_8xH_full;

        set_2xH_mc_function_ptr ARGS0();

        if (stream_global->m_is_MTMS)
        {
            DEBUG_SHOW_SW_INFO("This is MTMS");
            //if(g_HDProfileFault)
            //    return -1;    // IoK: check for HD-profile violation due to incorrect nal_unit_type (2-4), ASO or incorrect slice types (SI/SP)

            if(header==EOS)
            {
                DEBUG_SHOW_SW_INFO("Received EOS");
                if (IMGPAR slice_number > 0 && IMGPAR currentSlice->m_pic_combine_status==0) {
                    ret = DecodeOneFrame ARGS0();
                }

                stream_global->m_is_MTMS = -1;
                g_bEOS = TRUE;

                if(dpb.used_size_on_view[view_index])
                {
                    for (unsigned int i=0; i <storable_picture_count; i++)                
                        storable_picture_map[i]->used_for_first_field_reference = 0;
                    ret = flush_dpb ARGS1(view_index);
                    if (FAILED(ret)) {
                        return ret;
                    }
                    return CREL_OK;
                }
                return CREL_ERROR_H264_UNDEFINED;    // no more frames
            }

            //Check Skip Flag and PTS Process
            bSkip = CheckSkipAndPTSProcess ARGS1(bIsForReference);
            if (IMGPAR firstSlice->picture_type==B_SLICE  && dpb.used_size_on_view[view_index])
            {
                StorablePicture *pPreFrame = NULL;
            
                if ( dpb.fs_on_view[view_index][dpb.used_size_on_view[view_index]-1]->frame ) {    // Field Combined or Frame Split work may not be done if that picture has error.
                    pPreFrame = dpb.fs_on_view[view_index][dpb.used_size_on_view[view_index]-1]->frame;
                } else if (dpb.fs_on_view[view_index][dpb.used_size_on_view[view_index]-1]->top_field) {
                    pPreFrame = dpb.fs_on_view[view_index][dpb.used_size_on_view[view_index]-1]->top_field;
                } else {
                    pPreFrame = dpb.fs_on_view[view_index][dpb.used_size_on_view[view_index]-1]->bottom_field;
                }

                if(pPreFrame && (pPreFrame->slice_type==P_SLICE || pPreFrame->slice_type==I_SLICE))
                    g_pArrangedPCCCode = &(pPreFrame->m_CCCode);

                RearrangePBCCBuf ARGS0();
            }

            if (IMGPAR firstSlice->picture_type != B_SLICE)
            {
                //stream_global->number++;
                stream_global->bSeekToOpenGOP = 0;
                stream_global->nCollectB = 0;
            }
            else
                stream_global->nCollectB++;

            stream_global->number++;

            stream_global->pre_frame_num    = IMGPAR firstSlice->frame_num;

            if (IMGPAR firstSlice->picture_type == I_SLICE)
            {
                stream_global->m_listXsize[0] = stream_global->m_listXsize[1] = stream_global->m_listXsize[2] = 
                    stream_global->m_listXsize[3] = stream_global->m_listXsize[4] = stream_global->m_listXsize[5] = 0;
            }
            else
            {                
                stream_global->m_listXsize[0] = IMGPAR firstSlice->num_ref_idx_l0_active;
                stream_global->m_listXsize[2] = stream_global->m_listXsize[4] = IMGPAR firstSlice->num_ref_idx_l0_active * 2;
                stream_global->m_listXsize[1] = IMGPAR firstSlice->num_ref_idx_l1_active;
                stream_global->m_listXsize[3] = stream_global->m_listXsize[5] = IMGPAR firstSlice->num_ref_idx_l1_active * 2;
            }


            ret = DecodeOneFrame ARGS0();

            if (FAILED(ret)) {    //Only 2 options: Error or Level1 Warning

                if (ISWARNINGLEVEL_1(ret)) {
                    stream_global->m_bSeekIDR = TRUE;                    
                } else if (ISERROR(ret)) {
                    return ret;
                }
            }

            if (stream_global->is_first)
            {
                stream_global->is_first = 0;
                if (next_image_type == B_SLICE)
                    stream_global->bSeekToOpenGOP = 1;
            }

            next_image_type = -1; //reset

            framecount = g_framemgr->GetDisplayCount();
            if(g_framemgr->m_nFlushDPBsize)
                framecount += dpb.used_size_on_view[view_index];
            if(framecount > pre_framecount)
                return CREL_OK;
        }


        // Terry: For smart decoding level 6 (IDR_ONLY) of PIP case, we should return this I frame to reduce the responsive time and to avoid the EOS ending.
        if ( (IMGPAR smart_dec == SMART_DEC_LEVEL_6) )//&& (IMGPAR firstSlice->picture_type == I_SLICE) && !IMGPAR firstSlice->idr_flag)
        {
            DEBUG_SHOW_SW_INFO("DecodePicture: For smart decoding level 6, return the skipped frame.");
            return CREL_ERROR_H264_UNDEFINED;;
        }
    }

#if !defined(ONE_COF)
    IMGPAR cof_r = IMGPAR cof_d = NULL;
#endif

    return CREL_OK;    // okay
}

int DecodePicture_MultiThread_SingleSlice_IPRD_Merge PARGS0() {

    int nNumOfSkipBFrames = 0;
    //int nNumOfInterpolateBFrames = 0;

    int header, pre_framecount, framecount;
    int j, ret = 0;    
    int currThreadForB = 0;
    BOOL bSkip;
    int    nSeekFlag, fill_frame_gap_flag;
    int nImgB_rd0, nImgB_rd1;
    BOOL bIsForReference;
    stream_par  *stream_global = img->stream_global;
    int view_index = (IMGPAR firstSlice) ? (IMGPAR firstSlice->viewIndex) : 0;

    pre_framecount = g_framemgr->GetDisplayCount();
    if(g_framemgr->m_nFlushDPBsize)
        pre_framecount += dpb.used_size_on_view[view_index];

    g_bRewindDecoder = FALSE;

    // Start over IP Read & Decode Merge

    //check EOS to decode the last picture
    if ( g_bEOS )
    {
        DEBUG_SHOW_SW_INFO("Enter EOS Flush Stage!!");
        if (next_image_type == B_SLICE)
        {
            if ( get_param_fcn && !g_bNormalSpeed) {    //To guarantee conformance test at console mode        -Haihua
                //20070125-Byby: we need to skip the last B frame while EOS to prevent corruption caused by trick mode.
                if(dpb.used_size_on_view[view_index])
                {
                    for (unsigned int i=0; i <storable_picture_count; i++)                
                        storable_picture_map[i]->used_for_first_field_reference = 0;

                    ret = flush_dpb ARGS1(view_index);
                    if (FAILED(ret)) {
                        return ret;
                    }
                }
                nalu_global_available = 0;    
                stream_global->number = 0;
                stream_global->pic_ip_num = 0;
                stream_global->pic_b_num = 0;
                stream_global->is_first = 1;
                stream_global->bHasB = 0;
                stream_global->b_Count = 0;
                stream_global->nSwitchB = 0;
                stream_global->two_B_Flag = 0; //two b frames or not
                stream_global->nNeedToDecodeIP = 0;
                stream_global->nCollectB = 0;
                next_image_type = -1;

                g_bReceivedFirst = 0;

                g_bEOS = FALSE;

                //2006-03-31 KevinChien: Temporal solution for FF/RW
                //We need to change next_image_type modal
                return -1;
                //~KevinChien
            }

            img = img_array[stream_global->pic_b_num + 2];
            currThreadForB = 1;
        } else {
            // img = img_array[stream_global->pic_ip_num];
            img = img_array[0];
        }

        //output the last frame
        if ((IMGPAR slice_number > 0 && IMGPAR structure == FRAME) || (IMGPAR slice_number > 1 && IMGPAR structure != FRAME))
        {

            if (currThreadForB)
            {
                if ( stream_global->pic_b_num == 1) {
                    gImgB_r1 = img_array[3];
                    SetEvent(stream_global->m_event_decode_finish_b[0]);
                    SetEvent(stream_global->m_event_read_start_b[1]);
                    WaitForSingleObject(stream_global->m_event_decode_finish_b[1], INFINITE);
                    ResetEvent(stream_global->m_event_decode_finish_b[0]);
                    ResetEvent(stream_global->m_event_decode_finish_b[1]);

                } else {
                    gImgB_r0 = img_array[2];
                    SetEvent(stream_global->m_event_read_start_b[0]);
                    WaitForSingleObject(stream_global->m_event_decode_finish_b[0], INFINITE);
                    ResetEvent(stream_global->m_event_decode_finish_b[0]);
                }                    

            }
            else
            {
                //gImgIP_d = gImgIP_r = img_array[stream_global->pic_ip_num];
                gImgIP_d = gImgIP_r = img_array[0];
                SetEvent(stream_global->m_event_read_start_ip);
                WaitForSingleObject(stream_global->m_event_read_finish_ip, INFINITE);
                ResetEvent(stream_global->m_event_read_finish_ip);

                SetEvent(stream_global->m_event_decode_start_ip);
                SetEvent(stream_global->m_event_for_field_ip);
                WaitForSingleObject(stream_global->m_event_decode_finish_ip, INFINITE);
                ResetEvent(stream_global->m_event_decode_finish_ip);
            }
        }

        if(dpb.used_size_on_view[view_index])
        {
            for (unsigned int i=0; i <storable_picture_count; i++)                
                storable_picture_map[i]->used_for_first_field_reference = 0;

            ret = flush_dpb ARGS1(view_index);
            if (FAILED(ret)) {
                return ret;
            }
        }

        nalu_global_available = 0;    
        stream_global->number = 0;
        stream_global->pic_ip_num = 0;
        stream_global->pic_b_num = 0;
        stream_global->is_first = 1;
        stream_global->bHasB = 0;
        stream_global->b_Count = 0;
        stream_global->nSwitchB = 0;
        stream_global->two_B_Flag = 0; //two b frames or not
        stream_global->nNeedToDecodeIP = 0;
        stream_global->nCollectB = 0;
        next_image_type = -1;

        g_bReceivedFirst = 0;

        stream_global->m_is_MTMS = -1;

        g_bEOS = FALSE;

        return -1;
    }//last picture

    while(1)
    {
        if (nalu_global->nal_reference_idc != 0 || g_bReceivedFirst==0)
            bIsForReference = TRUE;
        else
            bIsForReference = FALSE;

        if (next_image_type == B_SLICE)
        {
            if (bIsForReference)
            {
                img = img_array[6];
            }
            else
            {
                if (stream_global->nCollectB == 2)
                {
                    //nImgB_rd0 = (stream_global->nSwitchB<<1) + 2;
                    //nImgB_rd1 = (stream_global->nSwitchB<<1) + 3;

                    gImgB_r0 = img_array[2];
                    //SetEvent(stream_global->m_event_read_start_b[0]);
                    WaitForSingleObject(stream_global->m_event_decode_finish_b[0], INFINITE);

                    gImgB_r1 = img_array[3];
                    //SetEvent(stream_global->m_event_read_start_b[1]);
                    WaitForSingleObject(stream_global->m_event_decode_finish_b[1], INFINITE);

                    ResetEvent(stream_global->m_event_decode_finish_b[0]);
                    ResetEvent(stream_global->m_event_decode_finish_b[1]);

                    //stream_global->nSwitchB = !(stream_global->nSwitchB);
                    stream_global->b_Count -= 2;
                    stream_global->two_B_Flag = 0;

                    stream_global->nCollectB = 0;

                    framecount = g_framemgr->GetDisplayCount();
                    if(g_framemgr->m_nFlushDPBsize)
                        framecount += dpb.used_size_on_view[view_index];
                    if(framecount > pre_framecount)
                        return 0;
                }

                //img = img_array[stream_global->b_Count+2];        
                img = img_array[stream_global->pic_b_num + 2];
            }
            currThreadForB = 1;
        }
        else
        {
            img = img_array[0];            // Merge IP read & decode
            currThreadForB = 0;
        }

		IMGPAR number								= stream_global->number;
		IMGPAR pre_frame_num						= stream_global->pre_frame_num;
		IMGPAR PreviousFrameNum						= stream_global->PreviousFrameNum[0];
		IMGPAR PreviousFrameNumOffset				= stream_global->PreviousFrameNumOffset[0];
		IMGPAR PreviousPOC							= stream_global->PreviousPOC[0];    
		IMGPAR ThisPOC								= stream_global->ThisPOC[0];
		IMGPAR PrevPicOrderCntLsb					= stream_global->PrevPicOrderCntLsb[0];
		IMGPAR PrevPicOrderCntMsb					= stream_global->PrevPicOrderCntMsb[0];
		IMGPAR last_has_mmco_5						= stream_global->last_has_mmco_5;		
		for(j=0; j<6; j++)
			IMGPAR m_listXsize[j] = stream_global->m_listXsize[j];

        stream_global->last_has_mmco_5 = 0;

        //Terry: for multi-slice case, smart_dec should be assigned before collect picture.
        IMGPAR smart_dec = g_dwSmartDecLevel;

        if (stream_global->m_bMultiThreadModeSwitch == 0) {

            ret = read_new_picture ARGS1(&header);

            if (ISERROR(ret)) {
                return ret;
            }

            if ( ( header == SOP ) || ( header == NALU_TYPE_SPS ) ) {
                Check_MultiThreadModel ARGS0 ();
            }

            if (stream_global->m_bMultiThreadModeSwitch ) {
                if (stream_global->nNeedToDecodeIP || stream_global->b_Count)
                    FlushAllFrames ARGS0();
                return g_pfnDecodePicture ARGS0();
            }

        } else {
            header = SOP;
            stream_global->m_bMultiThreadModeSwitch = 0;
        }

        if (IMGPAR slice_number == 0 || FAILED(ret))
        {
            DEBUG_SHOW_SW_INFO("[ERROR] Slice Number is 0 ==> return -1");
            return -1;
        } 
        else if (IMGPAR currentSlice->m_pic_combine_status != FRAME && IMGPAR firstSlice->structure != FRAME)
        {
            DEBUG_SHOW_SW_INFO("[ERROR] Field Coding Frame! But Only receive one Picture!!");

            IMGPAR currentSlice = IMGPAR firstSlice;
            for (int i=0; i< IMGPAR slice_number; i++)
            {
                //active_pps = &PicParSet[IMGPAR currentSlice->pic_parameter_set_id];
                memcpy(&active_pps, &PicParSet[IMGPAR currentSlice->pic_parameter_set_id], sizeof(pic_parameter_set_rbsp_t));
                IMGPAR prevSlice = IMGPAR currentSlice;
                IMGPAR currentSlice = (Slice*) IMGPAR currentSlice->nextSlice;
                free_ref_pic_list_reordering_buffer(IMGPAR prevSlice);
                free_new_slice(IMGPAR prevSlice);
            }
            IMGPAR firstSlice = IMGPAR currentSlice = IMGPAR prevSlice = NULL;
            IMGPAR current_mb_nr_r = IMGPAR current_mb_nr_d = -4712;   // impossible value for debugging, StW
            IMGPAR error_mb_nr = -4712;
            IMGPAR current_slice_nr = 0;
            IMGPAR slice_number = 0;

            stream_global->m_bSeekIDR = TRUE;

            return CREL_ERROR_H264_UNDEFINED;
        }

        DEBUG_SHOW_SW_INFO("Collected One Picture\n header: %d  Slice Number: %d  SmartDecLevel: %d ", header, IMGPAR slice_number, g_dwSmartDecLevel);

        IMGPAR SkipedBFrames[view_index][0] = IMGPAR SkipedBFrames[view_index][1] = 0;
        IMGPAR SkipThisFrame = FALSE;

        g_llDtsCount[view_index] = ((IMGPAR firstSlice->has_pts) && (IMGPAR firstSlice->picture_type == I_SLICE)) ? 0 : (g_llDtsCount[view_index]+1);
        g_llDecCount[view_index]++;

        get_block_4xh  = get_block_4xh_full;
        get_block_8xh  = get_block_8xh_full;
        get_block_16xh = get_block_16xh_full;

        set_4xH_mc_function_ptr ARGS0();

        mb_chroma_2xH = mb_chroma_2xH_full;
        mb_chroma_4xH = mb_chroma_4xH_full;
        mb_chroma_8xH = mb_chroma_8xH_full;

        set_2xH_mc_function_ptr ARGS0();


        DEBUG_SHOW_SW_INFO("This is MT SingleSlice");
        //if(g_HDProfileFault)
        //    return -1;    // IoK: check for HD-profile violation due to incorrect nal_unit_type (2-4), ASO or incorrect slice types (SI/SP)

        if(header==EOS)
        {
            DEBUG_SHOW_SW_INFO("Received EOS");
            //if (stream_global->nNeedToDecodeIP || stream_global->b_Count)
            if (stream_global->b_Count)
                FlushAllFrames ARGS0();

            g_bEOS = TRUE;
            return 2; //Special value for EOS
        }
        else if (header == NALU_TYPE_SPS)
        {
            DEBUG_SHOW_SW_INFO("Received SPS Header");
            //if (stream_global->nNeedToDecodeIP || stream_global->b_Count)
            if (stream_global->b_Count)
                FlushAllFrames ARGS0();

            if (currThreadForB)
            {
                stream_global->pre_frame_num    = IMGPAR firstSlice->frame_num;

                stream_global->m_listXsize[0] = IMGPAR firstSlice->num_ref_idx_l0_active;
                stream_global->m_listXsize[2] = stream_global->m_listXsize[4] = IMGPAR firstSlice->num_ref_idx_l0_active * 2;
                stream_global->m_listXsize[1] = IMGPAR firstSlice->num_ref_idx_l1_active;
                stream_global->m_listXsize[3] = stream_global->m_listXsize[5] = IMGPAR firstSlice->num_ref_idx_l1_active * 2;
            }
            else
            {
                stream_global->number++;
                stream_global->pre_frame_num    = IMGPAR firstSlice->frame_num;
                if (IMGPAR firstSlice->picture_type == I_SLICE)
                {
                    stream_global->m_listXsize[0] = stream_global->m_listXsize[1] = stream_global->m_listXsize[2] = 
                        stream_global->m_listXsize[3] = stream_global->m_listXsize[4] = stream_global->m_listXsize[5] = 0;
                }
                else
                {
                    stream_global->m_listXsize[0] = IMGPAR firstSlice->num_ref_idx_l0_active;
                    stream_global->m_listXsize[2] = stream_global->m_listXsize[4] = IMGPAR firstSlice->num_ref_idx_l0_active * 2;
                    stream_global->m_listXsize[1] = IMGPAR firstSlice->num_ref_idx_l1_active;
                    stream_global->m_listXsize[3] = stream_global->m_listXsize[5] = IMGPAR firstSlice->num_ref_idx_l1_active * 2;
                }
            }

            if (IMGPAR slice_number > 0)
            {
                if (currThreadForB)
                {
                    nImgB_rd0 = stream_global->pic_b_num + 2;

                    gImgB_r0 = img_array[nImgB_rd0];
                    SetEvent(stream_global->m_event_read_start_b[0]);
                    WaitForSingleObject(stream_global->m_event_decode_finish_b[0], INFINITE);
                    ResetEvent(stream_global->m_event_decode_finish_b[0]);
                }
                else
                {
                    //gImgIP_d = gImgIP_r = img_array[stream_global->pic_ip_num];
                    gImgIP_d = img_array[0];    // Merge IP read & decode
                    /*
                    SetEvent(stream_global->m_event_read_start_ip);
                    WaitForSingleObject(stream_global->m_event_read_finish_ip, INFINITE);
                    ResetEvent(stream_global->m_event_read_finish_ip);
                    */

                    SetEvent(stream_global->m_event_decode_start_ip);
                    SetEvent(stream_global->m_event_for_field_ip);
                    WaitForSingleObject(stream_global->m_event_decode_finish_ip, INFINITE);
                    ResetEvent(stream_global->m_event_decode_finish_ip);
                }
            }

            stream_global->m_is_MTMS = -1;

            stream_global->pic_ip_num = 0;
            stream_global->pic_b_num = 0;

            next_image_type = -1;

            if(dpb.used_size_on_view[view_index])
            {
                for (unsigned int i=0; i <storable_picture_count; i++)                
                    storable_picture_map[i]->used_for_first_field_reference = 0;

                ret = flush_dpb ARGS1(view_index);
                if (FAILED(ret)) {
                    return ret;
                }
            }

            return 0;
        }

        //Check Skip Flag and PTS Process
        bSkip = CheckSkipAndPTSProcess ARGS1(bIsForReference);
        if (IMGPAR firstSlice->picture_type==B_SLICE)
        {
            ImageParameters *pPreImg =  img_array[!(stream_global->pic_ip_num)];
            g_pArrangedPCCCode = &(pPreImg->m_CCCode);

            RearrangePBCCBuf ARGS0();
        }

        DEBUG_SHOW_SW_INFO("Start Send Events");

        if (currThreadForB)
        {
            DEBUG_SHOW_SW_INFO("This Picture is for B");

            if (IMGPAR smart_dec & SMART_DEC_INT_PEL_Y)
            {
                get_block_4xh  = get_block_4xh_int;
                get_block_8xh  = get_block_8xh_int;
                get_block_16xh = get_block_16xh_int;

                set_4xH_mc_function_ptr ARGS0();
            }

            if (IMGPAR smart_dec & SMART_DEC_INT_PEL_UV)
            {
                mb_chroma_2xH = mb_chroma_2xH_int;
                mb_chroma_4xH = mb_chroma_4xH_int;
                mb_chroma_8xH = mb_chroma_8xH_int;

                set_2xH_mc_function_ptr ARGS0();
            }

            stream_global->nCollectB++;
            stream_global->b_Count++;

            stream_global->pre_frame_num    = IMGPAR firstSlice->frame_num;

            stream_global->m_listXsize[0] = IMGPAR firstSlice->num_ref_idx_l0_active;
            stream_global->m_listXsize[2] = stream_global->m_listXsize[4] = IMGPAR firstSlice->num_ref_idx_l0_active * 2;
            stream_global->m_listXsize[1] = IMGPAR firstSlice->num_ref_idx_l1_active;
            stream_global->m_listXsize[3] = stream_global->m_listXsize[5] = IMGPAR firstSlice->num_ref_idx_l1_active * 2;

			seq_parameter_set_rbsp_t *sps =  (IMGPAR firstSlice->bIsBaseView) ? &SeqParSet[PicParSet[IMGPAR firstSlice->pic_parameter_set_id].seq_parameter_set_id] :
				&SeqParSubset[PicParSet[IMGPAR firstSlice->pic_parameter_set_id].seq_parameter_set_id];
            IMGPAR MaxFrameNum = 1 << (sps->log2_max_frame_num_minus4+4);

            if (bIsForReference == TRUE)
            {
                stream_global->nCollectB = 0;

                stream_global->b_Count--;

                //nImgIP_r = stream_global->pic_ip_num;
                //nImgIP_d = !(nImgIP_r);
                /*            Merge IP Read & Decode
                if (stream_global->nNeedToDecodeIP)
                {
                //gImgIP_d = img_array[nImgIP_d];
                gImgIP_d = img_array[0];    // Merge IP read & decode
                SetEvent(stream_global->m_event_decode_start_ip);
                }
                */

                if (stream_global->bHasB)
                {
                    /*
                    nImgB_rd0 = (stream_global->nSwitchB<<1) + 2;
                    nImgB_rd1 = (stream_global->nSwitchB<<1) + 3;

                    if (stream_global->b_Count == 3 && stream_global->two_B_Flag)
                    {
                    gImgB_r1 = img_array[nImgB_rd1];
                    SetEvent(stream_global->m_event_read_start_b[1]);
                    }
                    if (stream_global->b_Count <= 2)
                    {
                    gImgB_r0 = img_array[nImgB_rd0];
                    SetEvent(stream_global->m_event_read_start_b[0]);
                    if (stream_global->two_B_Flag)
                    {
                    gImgB_r1 = img_array[nImgB_rd1];
                    SetEvent(stream_global->m_event_read_start_b[1]);
                    }
                    }

                    WaitForSingleObject(stream_global->m_event_decode_finish_b[0], INFINITE);
                    if (stream_global->two_B_Flag)
                    WaitForSingleObject(stream_global->m_event_decode_finish_b[1], INFINITE);
                    ResetEvent(stream_global->m_event_decode_finish_b[0]);
                    ResetEvent(stream_global->m_event_decode_finish_b[1]);

                    stream_global->nSwitchB = !(stream_global->nSwitchB);
                    stream_global->b_Count -= 2;
                    */                    

                    gImgB_r0 = img_array[2];
                    WaitForSingleObject(stream_global->m_event_decode_finish_b[0], INFINITE);
                    stream_global->b_Count --;

                    if ( stream_global->b_Count ) {
                        gImgB_r1 = img_array[3];
                        WaitForSingleObject(stream_global->m_event_decode_finish_b[1], INFINITE);
                        ResetEvent(stream_global->m_event_decode_finish_b[1]);
                        stream_global->b_Count --;
                    }    

                    ResetEvent(stream_global->m_event_decode_finish_b[0]);    
                    stream_global->pic_b_num = 0;


                }
                /*            Merge IP Read & Decode
                if (stream_global->nNeedToDecodeIP)
                {
                SetEvent(stream_global->m_event_for_field_ip);
                WaitForSingleObject(stream_global->m_event_decode_finish_ip, INFINITE);
                ResetEvent(stream_global->m_event_decode_finish_ip);
                stream_global->nNeedToDecodeIP = 0;
                }
                */
                if (stream_global->b_Count & 1)
                {
                    stream_global->pic_b_num++;
                    //if (stream_global->pic_b_num == 4)
                    if (stream_global->pic_b_num == 2)
                        stream_global->pic_b_num = 0;
                }

                if (stream_global->b_Count)
                {
                    if (stream_global->b_Count & 1)
                        stream_global->two_B_Flag = 0;
                    else
                        stream_global->two_B_Flag = 1;

                    nImgB_rd0 = (stream_global->nSwitchB<<1) + 2;
                    nImgB_rd1 = (stream_global->nSwitchB<<1) + 3;

                    gImgB_r0 = img_array[nImgB_rd0];
                    SetEvent(stream_global->m_event_read_start_b[0]);
                    if (stream_global->two_B_Flag)
                    {
                        gImgB_r1 = img_array[nImgB_rd1];
                        SetEvent(stream_global->m_event_read_start_b[1]);
                    }

                    WaitForSingleObject(stream_global->m_event_decode_finish_b[0], INFINITE);
                    if (stream_global->two_B_Flag)
                        WaitForSingleObject(stream_global->m_event_decode_finish_b[1], INFINITE);
                    ResetEvent(stream_global->m_event_decode_finish_b[0]);
                    ResetEvent(stream_global->m_event_decode_finish_b[1]);

                    stream_global->nSwitchB = !(stream_global->nSwitchB);
                    stream_global->b_Count -= stream_global->two_B_Flag == 0 ? 1:2;
                }
#if defined(_HW_ACCEL_)
                if (g_DXVAVer != IviNotDxva)
                {
                    gImgB_r0 = img_array[6];
                    SetEvent(stream_global->m_event_read_start_b[0]);
                    WaitForSingleObject(stream_global->m_event_decode_finish_b[0], INFINITE);
                    ResetEvent(stream_global->m_event_decode_finish_b[0]);
                }
                else
#endif
                {
                    SetEvent(stream_global->m_event_start_pic_RB);
                    WaitForSingleObject(stream_global->m_event_finish_pic_RB, INFINITE);
                    ResetEvent(stream_global->m_event_finish_pic_RB);
                }

                if (stream_global->b_Count >= 2)
                    stream_global->bHasB = 1;
                else
                    stream_global->bHasB = 0;



                framecount = g_framemgr->GetDisplayCount();
                stream_global->number++;

                if(g_framemgr->m_nFlushDPBsize)
                    framecount += dpb.used_size_on_view[view_index];
                if(framecount > pre_framecount)
                    return 0;
            }
            else
            {
                /*
                if (stream_global->bHasB)
                {
                nImgB_rd0 = (stream_global->nSwitchB<<1) + ((stream_global->b_Count&1) ? 2:3);

                if ((stream_global->b_Count&1) || ((stream_global->b_Count&1) == 0 && stream_global->two_B_Flag))
                {
                if (nImgB_rd0&1)
                {
                gImgB_r1 = img_array[nImgB_rd0];
                SetEvent(stream_global->m_event_read_start_b[1]);
                }
                else
                {
                gImgB_r0 = img_array[nImgB_rd0];
                SetEvent(stream_global->m_event_read_start_b[0]);
                }
                }
                }
                */
                stream_global->bHasB = 1;
                if (stream_global->b_Count == 1) {
                    gImgB_r0 = img_array[2];
                    SetEvent(stream_global->m_event_read_start_b[0]);                            
                } else {
                    gImgB_r1 = img_array[3];
                    stream_global->two_B_Flag = 1;
                    SetEvent(stream_global->m_event_read_start_b[1]);
                }


                stream_global->pic_b_num++;
                //if (stream_global->pic_b_num == 4)
                if (stream_global->pic_b_num == 2)
                    stream_global->pic_b_num = 0;

                stream_global->number++;
            }

        }
        else
        {
            DEBUG_SHOW_SW_INFO("This Picture is for IP");
            if (stream_global->m_bDoSkipPB == TRUE && IMGPAR firstSlice->picture_type == I_SLICE)
                g_dwSmartDecLevel = stream_global->m_dwSavedSmartDecLevel;

            stream_global->nCollectB = 0;

            stream_global->number++;
            stream_global->pre_frame_num    = IMGPAR firstSlice->frame_num;

            if (IMGPAR firstSlice->picture_type == I_SLICE)
            {
                stream_global->m_listXsize[0] = stream_global->m_listXsize[1] = stream_global->m_listXsize[2] = 
                    stream_global->m_listXsize[3] = stream_global->m_listXsize[4] = stream_global->m_listXsize[5] = 0;
            }
            else
            {            
                stream_global->m_listXsize[0] = IMGPAR firstSlice->num_ref_idx_l0_active;
                stream_global->m_listXsize[2] = stream_global->m_listXsize[4] = IMGPAR firstSlice->num_ref_idx_l0_active * 2;
                stream_global->m_listXsize[1] = IMGPAR firstSlice->num_ref_idx_l1_active;
                stream_global->m_listXsize[3] = stream_global->m_listXsize[5] = IMGPAR firstSlice->num_ref_idx_l1_active * 2;
            }

            //nImgIP_r = stream_global->pic_ip_num;
            //nImgIP_d = !(nImgIP_r);
            // Merge IP read & decode

            if (!stream_global->is_first)
            {
                if(stream_global->bSeekToOpenGOP && IMGPAR firstSlice->picture_type != B_SLICE)  //We should not set bSeekToOpenGOP to 0 in reference-B
                    stream_global->bSeekToOpenGOP = 0;

                if (stream_global->m_bDoSkipPB == TRUE && IMGPAR firstSlice->picture_type == I_SLICE)  //Skip all B frames between I and P when m_bDoSkipPB == TRUE
                    stream_global->bSeekToOpenGOP = 1;

                if ( img->framepoc == 295 ) {    //Debugging only
                    img->framepoc = img->framepoc;
                }

                // P read
                fill_frame_gap_flag = 0;
				seq_parameter_set_rbsp_t *sps =  (IMGPAR firstSlice->bIsBaseView) ? &SeqParSet[PicParSet[IMGPAR firstSlice->pic_parameter_set_id].seq_parameter_set_id] :
					&SeqParSubset[PicParSet[IMGPAR firstSlice->pic_parameter_set_id].seq_parameter_set_id];
                IMGPAR MaxFrameNum = 1 << (sps->log2_max_frame_num_minus4+4);
                if (IMGPAR firstSlice->frame_num != IMGPAR firstSlice->pre_frame_num && IMGPAR firstSlice->frame_num != (IMGPAR firstSlice->pre_frame_num + 1) % IMGPAR MaxFrameNum) 
                {                                                    
                    nSeekFlag = g_framemgr->GetDisplayCount() + dpb.used_size_on_view[view_index];
                    // Terry: For seeking case (nSeekFlag==0), the first frame should be I frame.
                    if((nSeekFlag == 0) && (IMGPAR firstSlice->picture_type != I_SLICE))
                        return -2;
                    // Terry: For seeking case (nSeekFlag==0), we should skip this function.
                    if( (IMGPAR smart_dec <= SMART_DEC_LEVEL_3 || IMGPAR smart_dec == SMART_DEC_LEVEL_0) && nSeekFlag && IMGPAR dwFillFrameNumGap)
                        fill_frame_gap_flag= 1;
                }
                if (!fill_frame_gap_flag)     //Non fill_frame_num_gap
                {
                    gImgIP_d = img_array[0];
                    SetEvent(stream_global->m_event_decode_start_ip);
                }

                //B read & decode
                if (stream_global->bHasB)
                {
                    /*
                    nImgB_rd0 = (stream_global->nSwitchB<<1) + 2;
                    nImgB_rd1 = (stream_global->nSwitchB<<1) + 3;

                    if (stream_global->b_Count == 3 && stream_global->two_B_Flag)
                    {
                    gImgB_r1 = img_array[nImgB_rd1];
                    SetEvent(stream_global->m_event_read_start_b[1]);
                    }
                    if (stream_global->b_Count <= 2)
                    {
                    gImgB_r0 = img_array[nImgB_rd0];
                    SetEvent(stream_global->m_event_read_start_b[0]);
                    if (stream_global->two_B_Flag)
                    {
                    gImgB_r1 = img_array[nImgB_rd1];
                    SetEvent(stream_global->m_event_read_start_b[1]);
                    }
                    }

                    WaitForSingleObject(stream_global->m_event_decode_finish_b[0], INFINITE);
                    if (stream_global->two_B_Flag)
                    WaitForSingleObject(stream_global->m_event_decode_finish_b[1], INFINITE);
                    //ResetEvent(stream_global->m_event_decode_finish_b[0]);
                    //ResetEvent(stream_global->m_event_decode_finish_b[1]);

                    stream_global->nSwitchB = !(stream_global->nSwitchB);
                    stream_global->b_Count -= 2;
                    */
                    gImgB_r0 = img_array[2];
                    WaitForSingleObject(stream_global->m_event_decode_finish_b[0], INFINITE);
                    stream_global->b_Count --;

                    if ( stream_global->b_Count ) {
                        gImgB_r1 = img_array[3];
                        WaitForSingleObject(stream_global->m_event_decode_finish_b[1], INFINITE);
                        stream_global->b_Count --;
                    }

                }

                if (fill_frame_gap_flag) {
                    gImgIP_d = img_array[0];
                    SetEvent(stream_global->m_event_decode_start_ip);
                }

                //P decode finish
                SetEvent(stream_global->m_event_for_field_ip);
                WaitForSingleObject(stream_global->m_event_decode_finish_ip, INFINITE);
                //stream_global->nNeedToDecodeIP = 0;

                // P read finish
                if (!fill_frame_gap_flag)      //Non fill_frame_num_gap
                {
                    ResetEvent(stream_global->m_event_decode_finish_ip);
                } else {

                    //fill_frame_num_gap
                    ResetEvent(stream_global->m_event_decode_finish_ip);

					//store these parameters to next collect_pic
					stream_global->PreviousFrameNum[0]				= IMGPAR PreviousFrameNum;
					stream_global->PreviousFrameNumOffset[0]	= IMGPAR PreviousFrameNumOffset;
					stream_global->PreviousPOC[0]						= IMGPAR PreviousPOC;
					stream_global->ThisPOC[0]								= IMGPAR ThisPOC; 
					stream_global->PrevPicOrderCntLsb[0]			= IMGPAR PrevPicOrderCntLsb;
					stream_global->PrevPicOrderCntMsb[0]			= IMGPAR PrevPicOrderCntMsb;
				}

                if (stream_global->bHasB)
                {
                    ResetEvent(stream_global->m_event_decode_finish_b[0]);
                    ResetEvent(stream_global->m_event_decode_finish_b[1]);
                }

                //stream_global->nNeedToDecodeIP = 1;

                //if (img_array[nImgIP_d]->SkipThisFrame && m_bDoSkipPB == FALSE)
                if (img_array[0]->SkipThisFrame && stream_global->m_bDoSkipPB == FALSE)// Merge IP read & decode
                {
                    stream_global->m_dwSavedSmartDecLevel = g_dwSmartDecLevel;
                    g_dwSmartDecLevel = SMART_DEC_SKIP_PB;
                    stream_global->m_bDoSkipPB = TRUE;
                }
                else if (g_dwSmartDecLevel != SMART_DEC_SKIP_PB)
                    stream_global->m_bDoSkipPB = FALSE;

                stream_global->bHasB = 0;
                stream_global->two_B_Flag = 0;
                stream_global->pic_b_num = 0;
                /*
                if (stream_global->b_Count & 1)
                {
                stream_global->two_B_Flag = 0;
                stream_global->b_Count++;
                stream_global->pic_b_num++;
                if (stream_global->pic_b_num == 4)
                stream_global->pic_b_num = 0;
                }
                else
                stream_global->two_B_Flag = 1;

                if (stream_global->b_Count >= 2)
                stream_global->bHasB = 1;
                else
                stream_global->bHasB = 0;
                */

#if defined(_SHOW_THREAD_TIME_)
                DEBUG_INFO("PR:%I64d, PD:%I64d, B1R:%I64d, B1D:%I64d, B2R:%I64d, B2D:%I64d",
                    (1000 * (t_ip[1].QuadPart - t_ip[0].QuadPart) / freq.QuadPart),
                    (1000 * (t_ip[3].QuadPart - t_ip[2].QuadPart) / freq.QuadPart),
                    (1000 * (t_b[1].QuadPart - t_b[0].QuadPart) / freq.QuadPart),
                    (1000 * (t_b[5].QuadPart - t_b[4].QuadPart) / freq.QuadPart),
                    (1000 * (t_b[3].QuadPart - t_b[2].QuadPart) / freq.QuadPart),
                    (1000 * (t_b[7].QuadPart - t_b[6].QuadPart) / freq.QuadPart)
                    );
#endif
                framecount = g_framemgr->GetDisplayCount();
                if(g_framemgr->m_nFlushDPBsize)
                    framecount += dpb.used_size_on_view[view_index];
                if(framecount > pre_framecount)
                    return 0;
            } else {
                //KevinChien: 060722 To fix Fujitsu next chapter
                if(next_image_type == B_SLICE)
                    stream_global->bSeekToOpenGOP = 1;
                //End of 060722

                gImgIP_d = img_array[0];    //Merge IP read & decode
                SetEvent(stream_global->m_event_decode_start_ip);
                SetEvent(stream_global->m_event_for_field_ip);
                WaitForSingleObject(stream_global->m_event_decode_finish_ip, INFINITE);
                ResetEvent(stream_global->m_event_decode_finish_ip);

                //if (img_array[nImgIP_d]->SkipThisFrame && m_bDoSkipPB == FALSE)    //We should not set m_bDoSkipPB in reference-B
                if (img_array[0]->SkipThisFrame && img_array[0]->firstSlice->picture_type != B_SLICE && stream_global->m_bDoSkipPB == FALSE)// Merge IP read & decode
                {
                    stream_global->m_dwSavedSmartDecLevel = g_dwSmartDecLevel;
                    g_dwSmartDecLevel = SMART_DEC_SKIP_PB;
                    stream_global->m_bDoSkipPB = TRUE;
                }
                else if (g_dwSmartDecLevel != SMART_DEC_SKIP_PB)
                    stream_global->m_bDoSkipPB = FALSE;

                stream_global->is_first = 0;
                /*
                stream_global->pic_ip_num++;            // Merge IP read & decode
                if (stream_global->pic_ip_num == 2)
                stream_global->pic_ip_num = 0;
                */
                //stream_global->nNeedToDecodeIP = 1;
            }
        }        



        // Terry: For smart decoding level 6 (IDR_ONLY) of PIP case, we should return this I frame to reduce the responsive time and to avoid the EOS ending.
        if ( (IMGPAR smart_dec == SMART_DEC_LEVEL_6) )//&& (IMGPAR firstSlice->picture_type == I_SLICE) && !IMGPAR firstSlice->idr_flag)
        {
            DEBUG_SHOW_SW_INFO("DecodePicture: For smart decoding level 6, return the skipped frame.");
            return -2;
        }
    }

#if !defined(ONE_COF)
    IMGPAR cof_r = IMGPAR cof_d = NULL;
#endif

    return 0;    // okay

}


int DecodePicture_MultiThread_SingleSlice_IPRD_Seperate PARGS0()
{
    int nNumOfSkipBFrames = 0;
    //int nNumOfInterpolateBFrames = 0;

    int header, pre_framecount, framecount;
    int j, ret = 0;    
    int currThreadForB = 0;
    BOOL bSkip;
    int    fill_frame_gap_flag;
    int nImgIP_r, nImgIP_d, nImgB_rd0, nImgB_rd1;
    BOOL bIsForReference;
    stream_par  *stream_global = img->stream_global;
    static BOOL bAlreadyReset;
    int view_index = (IMGPAR firstSlice) ? (IMGPAR firstSlice->viewIndex) : 0;

    pre_framecount = g_framemgr->GetDisplayCount();
    if(g_framemgr->m_nFlushDPBsize)
        pre_framecount += dpb.used_size_on_view[view_index];

    g_bRewindDecoder = FALSE;

    //check EOS to decode the last picture
    if ( g_bEOS )
    {
        DEBUG_SHOW_SW_INFO("Enter EOS Flush Stage!!");
        if (next_image_type == B_SLICE)
        {
            if ( get_param_fcn && !g_bNormalSpeed) {    //To guarantee conformance test at console mode        -Haihua
                //20070125-Byby: we need to skip the last B frame while EOS to prevent corruption caused by trick mode.
                if(dpb.used_size_on_view[view_index])
                {
                    for (unsigned int i=0; i <storable_picture_count; i++)                
                        storable_picture_map[i]->used_for_first_field_reference = 0;

                    ret = flush_dpb ARGS1(view_index);
                    if (FAILED(ret)) {
                        return ret;
                    }
                }
                nalu_global_available = 0;    
                stream_global->number = 0;
                stream_global->pic_ip_num = 0;
                stream_global->pic_b_num = 0;
                stream_global->is_first = 1;
                stream_global->bHasB = 0;
                stream_global->b_Count = 0;
                stream_global->nSwitchB = 0;
                stream_global->two_B_Flag = 0; //two b frames or not
                stream_global->nNeedToDecodeIP = 0;
                stream_global->nCollectB = 0;
                next_image_type = -1;

                g_bReceivedFirst = 0;

                g_bEOS = FALSE;
                //2006-03-31 KevinChien: Temporal solution for FF/RW
                //We need to change next_image_type modal
                return CREL_ERROR_H264_UNDEFINED;
                //~KevinChien
            }

            img = img_array[stream_global->pic_b_num + 2];
            currThreadForB = 1;
        }
        else
            img = img_array[stream_global->pic_ip_num];

        //output the last frame
        if ((IMGPAR slice_number > 0 && IMGPAR structure == FRAME) || (IMGPAR slice_number > 1 && IMGPAR structure != FRAME))
        {
            if (currThreadForB)
            {
                nImgB_rd0 = stream_global->pic_b_num + 2;

                gImgB_r0 = img_array[nImgB_rd0];
                SetEvent(stream_global->m_event_read_start_b[0]);
                WaitForSingleObject(stream_global->m_event_decode_finish_b[0], INFINITE);
                ResetEvent(stream_global->m_event_decode_finish_b[0]);
            }
            else
            {
                gImgIP_d = gImgIP_r = img_array[stream_global->pic_ip_num];
                SetEvent(stream_global->m_event_read_start_ip);
                WaitForSingleObject(stream_global->m_event_read_finish_ip, INFINITE);
                ResetEvent(stream_global->m_event_read_finish_ip);

                SetEvent(stream_global->m_event_decode_start_ip);
                SetEvent(stream_global->m_event_for_field_ip);
                WaitForSingleObject(stream_global->m_event_decode_finish_ip, INFINITE);
                ResetEvent(stream_global->m_event_decode_finish_ip);
            }
        }

        if(dpb.used_size_on_view[view_index])
        {
            for (unsigned int i=0; i <storable_picture_count; i++)                
                storable_picture_map[i]->used_for_first_field_reference = 0;

            ret = flush_dpb ARGS1(view_index);
            if (FAILED(ret)) {
                return ret;
            }
        }

        nalu_global_available = 0;    
        stream_global->number = 0;
        stream_global->pic_ip_num = 0;
        stream_global->pic_b_num = 0;
        stream_global->is_first = 1;
        stream_global->bHasB = 0;
        stream_global->b_Count = 0;
        stream_global->nSwitchB = 0;
        stream_global->two_B_Flag = 0; //two b frames or not
        stream_global->nNeedToDecodeIP = 0;
        stream_global->nCollectB = 0;
        next_image_type = -1;

        g_bReceivedFirst = 0;

        stream_global->m_is_MTMS = -1;

        g_bEOS = FALSE;
        return CREL_ERROR_H264_UNDEFINED;
    }//last picture

    while(1)
    {
        if (nalu_global->nal_reference_idc != 0 || g_bReceivedFirst==0)
            bIsForReference = TRUE;
        else
            bIsForReference = FALSE;

        if (stream_global->m_bSeekIDR == FALSE) {
            bAlreadyReset = FALSE;
        }

        if ( stream_global ->m_bMultiThreadModeSwitch == 2 ) {
            img = img_array[0];
        } else if ( (next_image_type == B_SLICE) && (!bIsForReference) && (stream_global->m_bSeekIDR == FALSE)) {

            if (stream_global->nCollectB==2 && stream_global->b_Count>=2)
            {
                if (stream_global->nNeedToDecodeIP)
                {
                    nImgIP_d = !(stream_global->pic_ip_num);

                    gImgIP_d = img_array[nImgIP_d];
                    SetEvent(stream_global->m_event_decode_start_ip);

                    //B read & decode
                    if (stream_global->bHasB)
                        DecodeProcessBB ARGS0();

                    SetEvent(stream_global->m_event_for_field_ip);
                    WaitForSingleObject(stream_global->m_event_decode_finish_ip, INFINITE);
                    ResetEvent(stream_global->m_event_decode_finish_ip);

                    stream_global->nNeedToDecodeIP = 0;

                    framecount = g_framemgr->GetDisplayCount();
                    if(g_framemgr->m_nFlushDPBsize)
                        framecount += dpb.used_size_on_view[view_index];
                    if(framecount > pre_framecount)
                        return CREL_OK;
                }

                //B read & decode
                nImgB_rd0 = (stream_global->nSwitchB<<1) + 2;
                nImgB_rd1 = (stream_global->nSwitchB<<1) + 3;

                stream_global->two_B_Flag = 1;    //Bug fixed by Haihua to cover BBetweenP = 3, 5, 7, .... Odd and larger than 2

                gImgB_r0 = img_array[nImgB_rd0];
                gImgB_r1 = img_array[nImgB_rd1];
                SetEvent(stream_global->m_event_read_start_b[0]);
                SetEvent(stream_global->m_event_read_start_b[1]);

                WaitForSingleObject(stream_global->m_event_decode_finish_b[0], INFINITE);
                WaitForSingleObject(stream_global->m_event_decode_finish_b[1], INFINITE);
                ResetEvent(stream_global->m_event_decode_finish_b[0]);
                ResetEvent(stream_global->m_event_decode_finish_b[1]);

                stream_global->nSwitchB = !(stream_global->nSwitchB);
                stream_global->b_Count -= 2;

                stream_global->two_B_Flag = 0;
                stream_global->bHasB = 0;

                stream_global->nCollectB = 0;

                framecount = g_framemgr->GetDisplayCount();
                if(g_framemgr->m_nFlushDPBsize)
                    framecount += dpb.used_size_on_view[view_index];
                if(framecount > pre_framecount)
                    return CREL_OK;
            }

            img = img_array[stream_global->pic_b_num+2];

            currThreadForB = 1;

        } else {            

            if (stream_global->m_bSeekIDR && (bAlreadyReset == FALSE)) {
                stream_global->pic_ip_num = 0;
                stream_global->is_first = 1;
                dpb.last_output_poc = INT_MIN;
                stream_global->nNeedToDecodeIP = 0;
                next_image_type = -1;
            }

            img = img_array[stream_global->pic_ip_num];    
            currThreadForB = 0;

            

            if (stream_global->m_bSeekIDR && (bAlreadyReset == FALSE) ) {

                if (stream_global->m_bRefBWaitB) {
                    SetEvent(event_RB_1stfield_decode_complete);
                    WaitForSingleObject(event_RB_wait_clear, INFINITE);
                    ResetEvent(event_RB_wait_clear);
                    WaitForSingleObject(stream_global->m_event_read_finish_ip, INFINITE);
                    stream_global->m_bRefBWaitB = FALSE;
                }

                if (stream_global->bHasB && (stream_global->m_is_MTMS == 0)) {
                    DecodeProcessBB ARGS0();
                }

                stream_global->bHasB = 0;
                stream_global->b_Count = 0;
                stream_global->nSwitchB = 0;
                stream_global->two_B_Flag = 0;                
                stream_global->nCollectB = 0;                
                stream_global->pic_b_num = 0;


                flush_dpb ARGS1(view_index);
                update_ref_list ARGS1(view_index);
                update_ltref_list ARGS1(view_index);        

                ResetEvent(event_RB_1stfield_decode_complete);
                ResetEvent(stream_global->m_event_read_finish_ip);

                for (int i = 0; i < 6; i++ ) {

                    img = img_array[i];

                    IMGPAR currentSlice = IMGPAR firstSlice;
                    for (int j=0; j< IMGPAR slice_number; j++)
                    {
                        //active_pps = &PicParSet[IMGPAR currentSlice->pic_parameter_set_id];
                        memcpy(&active_pps, &PicParSet[IMGPAR currentSlice->pic_parameter_set_id], sizeof(pic_parameter_set_rbsp_t));
                        IMGPAR prevSlice = IMGPAR currentSlice;
                        IMGPAR currentSlice = (Slice*) IMGPAR currentSlice->nextSlice;
                        free_ref_pic_list_reordering_buffer(IMGPAR prevSlice);
                        free_new_slice(IMGPAR prevSlice);                    

                    }

                    if (IMGPAR structure == FRAME) {

                        if(dec_picture)
                        {
                            if ((dpb.used_size_on_view[view_index] == 0) ||dpb.fs_on_view[view_index][dpb.used_size_on_view[view_index] - 1]->frame != dec_picture) {
                                release_storable_picture ARGS2(dec_picture, 1);
                            }
                            dec_picture = NULL;
                        }
                        img->m_dec_picture_top = NULL;
                        img->m_dec_picture_bottom = NULL;

                    } else {

                        if (img->m_dec_picture_top) {                        

                            release_storable_picture ARGS2(img->m_dec_picture_top, 1);                        
                            img->m_dec_picture_top = NULL;
                        }

                        if (img->m_dec_picture_bottom) {

                            release_storable_picture ARGS2(img->m_dec_picture_bottom, 1);                        
                            img->m_dec_picture_bottom = NULL;
                        }
                        dec_picture = NULL;

                    }    

                    IMGPAR firstSlice = IMGPAR currentSlice = IMGPAR prevSlice = NULL;
                    IMGPAR current_mb_nr_r = IMGPAR current_mb_nr_d = -4712;   // impossible value for debugging, StW
                    IMGPAR error_mb_nr = -4712;
                    IMGPAR current_slice_nr = 0;
                    IMGPAR slice_number = 0;
                    img->m_active_pps[0].Valid = NULL;
                    img->m_active_pps[1].Valid = NULL;
                    //dec_picture = NULL;

                    if (IMGPAR structure != FRAME)
                    {
                        IMGPAR mb_decdata = IMGPAR mb_decdata_ori;
                        IMGPAR cof_array = IMGPAR cof_array_ori;
                    }

                }

                img = img_array[0];
                bAlreadyReset = TRUE;
            }
        }

		IMGPAR number								= stream_global->number;
		IMGPAR pre_frame_num						= stream_global->pre_frame_num;
		IMGPAR PreviousFrameNum						= stream_global->PreviousFrameNum[0];
		IMGPAR PreviousFrameNumOffset				= stream_global->PreviousFrameNumOffset[0];
		IMGPAR PreviousPOC							= stream_global->PreviousPOC[0];    
		IMGPAR ThisPOC								= stream_global->ThisPOC[0];
		IMGPAR PrevPicOrderCntLsb					= stream_global->PrevPicOrderCntLsb[0];
		IMGPAR PrevPicOrderCntMsb					= stream_global->PrevPicOrderCntMsb[0];
		IMGPAR last_has_mmco_5						= stream_global->last_has_mmco_5;		
		for(j=0; j<6; j++)
			IMGPAR m_listXsize[j] = stream_global->m_listXsize[j];

        stream_global->last_has_mmco_5 = 0;

        //Terry: for multi-slice case, smart_dec should be assigned before collect picture.
        IMGPAR smart_dec = g_dwSmartDecLevel;


        if (stream_global->m_bMultiThreadModeSwitch == 0) {

            ret = read_new_picture ARGS1(&header);
/*
            if (stream_global->number >= 440) {
                stream_global->number = stream_global->number;
            }
*/

            if (ISERROR(ret)) {
                return ret;
            }

            if ( ( header == SOP ) || ( header == NALU_TYPE_SPS ) ) {
                Check_MultiThreadModel ARGS0 ();
            }


            if (stream_global->m_bMultiThreadModeSwitch) {
                if (stream_global->nNeedToDecodeIP || stream_global->b_Count)
                    FlushAllFrames ARGS0();

                stream_global->m_is_MTMS = 1; 

                if (img != img_array[0]) {
                    img_array[0]->slice_number = img->slice_number;
                    img_array[0]->firstSlice = img->firstSlice;
                    img_array[0]->currentSlice = img->currentSlice;

                    img->firstSlice = img->currentSlice = NULL;
                    img->slice_number = 0;

                    img = img_array[0];
                }
                    
                return g_pfnDecodePicture ARGS0();
            }
        } else {
            header = SOP;
            stream_global->m_bMultiThreadModeSwitch = 0;
        }


        if (IMGPAR slice_number == 0 || FAILED(header) )
        {
            DEBUG_SHOW_SW_INFO("[ERROR] Slice Number is 0 ==> return -1");
            return CREL_ERROR_H264_UNDEFINED;
        }
        else if (IMGPAR currentSlice->m_pic_combine_status != FRAME && IMGPAR firstSlice->structure != FRAME)
        {
            DEBUG_SHOW_SW_INFO("[ERROR] Field Coding Frame! But Only receive one Picture!!");

            IMGPAR currentSlice = IMGPAR firstSlice;
            for (int i=0; i< IMGPAR slice_number; i++)
            {
                //active_pps = &PicParSet[IMGPAR currentSlice->pic_parameter_set_id];
                memcpy(&active_pps, &PicParSet[IMGPAR currentSlice->pic_parameter_set_id], sizeof(pic_parameter_set_rbsp_t));
                IMGPAR prevSlice = IMGPAR currentSlice;
                IMGPAR currentSlice = (Slice*) IMGPAR currentSlice->nextSlice;
                free_ref_pic_list_reordering_buffer(IMGPAR prevSlice);
                free_new_slice(IMGPAR prevSlice);
            }
            IMGPAR firstSlice = IMGPAR currentSlice = IMGPAR prevSlice = NULL;
            IMGPAR current_mb_nr_r = IMGPAR current_mb_nr_d = -4712;   // impossible value for debugging, StW
            IMGPAR error_mb_nr = -4712;
            IMGPAR current_slice_nr = 0;
            IMGPAR slice_number = 0;

            stream_global->m_bSeekIDR = TRUE;

            if (stream_global->m_bRefBWaitB) {
                SetEvent(event_RB_1stfield_decode_complete);
                WaitForSingleObject(event_RB_wait_clear, INFINITE);
                ResetEvent(event_RB_wait_clear);
                WaitForSingleObject(stream_global->m_event_read_finish_ip, INFINITE);
                stream_global->m_bRefBWaitB = FALSE;
            }

            return CREL_ERROR_H264_UNDEFINED;
        }


        DEBUG_SHOW_SW_INFO("Collected One Picture\n header: %d  Slice Number: %d  SmartDecLevel: %d ", header, IMGPAR slice_number, g_dwSmartDecLevel);

        IMGPAR SkipedBFrames[view_index][0] = IMGPAR SkipedBFrames[view_index][1] = 0;
        IMGPAR SkipThisFrame = FALSE;

        g_llDtsCount[view_index] = ((IMGPAR firstSlice->has_pts) && (IMGPAR firstSlice->picture_type == I_SLICE)) ? 0 : (g_llDtsCount[view_index]+1);
        g_llDecCount[view_index]++;

        get_block_4xh  = get_block_4xh_full;
        get_block_8xh  = get_block_8xh_full;
        get_block_16xh = get_block_16xh_full;

        set_4xH_mc_function_ptr ARGS0();

        mb_chroma_2xH = mb_chroma_2xH_full;
        mb_chroma_4xH = mb_chroma_4xH_full;
        mb_chroma_8xH = mb_chroma_8xH_full;

        set_2xH_mc_function_ptr ARGS0();

        DEBUG_SHOW_SW_INFO("This is MT SingleSlice");
        //if(g_HDProfileFault)
        //    return -1;    // IoK: check for HD-profile violation due to incorrect nal_unit_type (2-4), ASO or incorrect slice types (SI/SP)

        if(header==EOS)
        {
            DEBUG_SHOW_SW_INFO("Received EOS");
            if (stream_global->nNeedToDecodeIP || stream_global->b_Count)
                FlushAllFrames ARGS0();

            g_bEOS = TRUE;
            return 2; //Special value for EOS
        }
        else if (header == NALU_TYPE_SPS)
        {
            DEBUG_SHOW_SW_INFO("Received SPS Header");
            if (stream_global->nNeedToDecodeIP || stream_global->b_Count)
                FlushAllFrames ARGS0();

            if (currThreadForB)
            {
                stream_global->pre_frame_num    = IMGPAR firstSlice->frame_num;

                stream_global->m_listXsize[0] = IMGPAR firstSlice->num_ref_idx_l0_active;
                stream_global->m_listXsize[2] = stream_global->m_listXsize[4] = IMGPAR firstSlice->num_ref_idx_l0_active * 2;
                stream_global->m_listXsize[1] = IMGPAR firstSlice->num_ref_idx_l1_active;
                stream_global->m_listXsize[3] = stream_global->m_listXsize[5] = IMGPAR firstSlice->num_ref_idx_l1_active * 2;
            }
            else
            {
                stream_global->number++;
                stream_global->pre_frame_num    = IMGPAR firstSlice->frame_num;
                if (IMGPAR firstSlice->picture_type == I_SLICE)
                {
                    stream_global->m_listXsize[0] = stream_global->m_listXsize[1] = stream_global->m_listXsize[2] = 
                        stream_global->m_listXsize[3] = stream_global->m_listXsize[4] = stream_global->m_listXsize[5] = 0;
                }
                else
                {
                    stream_global->m_listXsize[0] = IMGPAR firstSlice->num_ref_idx_l0_active;
                    stream_global->m_listXsize[2] = stream_global->m_listXsize[4] = IMGPAR firstSlice->num_ref_idx_l0_active * 2;
                    stream_global->m_listXsize[1] = IMGPAR firstSlice->num_ref_idx_l1_active;
                    stream_global->m_listXsize[3] = stream_global->m_listXsize[5] = IMGPAR firstSlice->num_ref_idx_l1_active * 2;
                }
            }

            if (IMGPAR slice_number > 0)
            {
                if (currThreadForB)
                {
                    nImgB_rd0 = stream_global->pic_b_num + 2;

                    gImgB_r0 = img_array[nImgB_rd0];
                    SetEvent(stream_global->m_event_read_start_b[0]);
                    WaitForSingleObject(stream_global->m_event_decode_finish_b[0], INFINITE);
                    ResetEvent(stream_global->m_event_decode_finish_b[0]);
                }
                else
                {
                    gImgIP_d = gImgIP_r = img_array[stream_global->pic_ip_num];
                    SetEvent(stream_global->m_event_read_start_ip);
                    WaitForSingleObject(stream_global->m_event_read_finish_ip, INFINITE);
                    ResetEvent(stream_global->m_event_read_finish_ip);

                    SetEvent(stream_global->m_event_decode_start_ip);
                    SetEvent(stream_global->m_event_for_field_ip);
                    WaitForSingleObject(stream_global->m_event_decode_finish_ip, INFINITE);
                    ResetEvent(stream_global->m_event_decode_finish_ip);
                }
            }


            stream_global->m_is_MTMS = -1;
            stream_global->pic_ip_num = 0;
            stream_global->pic_b_num = 0;

            next_image_type = -1;

            if(dpb.used_size_on_view[view_index])
            {
                for (unsigned int i=0; i <storable_picture_count; i++)                
                    storable_picture_map[i]->used_for_first_field_reference = 0;

                ret = flush_dpb ARGS1(view_index);
                if (FAILED(ret)) {
                    return ret;
                }
            }

            return CREL_ERROR_H264_UNDEFINED;
        }

        //Check Skip Flag and PTS Process
        bSkip = CheckSkipAndPTSProcess ARGS1(bIsForReference);
        if (IMGPAR firstSlice->picture_type==B_SLICE)
        {
            ImageParameters *pPreImg =  img_array[!(stream_global->pic_ip_num)];
            g_pArrangedPCCCode = &(pPreImg->m_CCCode);

            RearrangePBCCBuf ARGS0();
        }

        DEBUG_SHOW_SW_INFO("Start Send Events");

        if (currThreadForB)
        {
            DEBUG_SHOW_SW_INFO("This Picture is for B");

            if (!stream_global->is_first) {

                if (IMGPAR smart_dec & SMART_DEC_INT_PEL_Y)
                {
                    get_block_4xh  = get_block_4xh_int;
                    get_block_8xh  = get_block_8xh_int;
                    get_block_16xh = get_block_16xh_int;

                    set_4xH_mc_function_ptr ARGS0();
                }

                if (IMGPAR smart_dec & SMART_DEC_INT_PEL_UV)
                {
                    mb_chroma_2xH = mb_chroma_2xH_int;
                    mb_chroma_4xH = mb_chroma_4xH_int;
                    mb_chroma_8xH = mb_chroma_8xH_int;

                    set_2xH_mc_function_ptr ARGS0();
                }

                stream_global->nCollectB++;
                stream_global->b_Count++;

                stream_global->pre_frame_num    = IMGPAR firstSlice->frame_num;

                stream_global->m_listXsize[0] = IMGPAR firstSlice->num_ref_idx_l0_active;
                stream_global->m_listXsize[2] = stream_global->m_listXsize[4] = IMGPAR firstSlice->num_ref_idx_l0_active * 2;
                stream_global->m_listXsize[1] = IMGPAR firstSlice->num_ref_idx_l1_active;
                stream_global->m_listXsize[3] = stream_global->m_listXsize[5] = IMGPAR firstSlice->num_ref_idx_l1_active * 2;

				seq_parameter_set_rbsp_t *sps =  (IMGPAR firstSlice->bIsBaseView) ? &SeqParSet[PicParSet[IMGPAR firstSlice->pic_parameter_set_id].seq_parameter_set_id] :
					&SeqParSubset[PicParSet[IMGPAR firstSlice->pic_parameter_set_id].seq_parameter_set_id];
                IMGPAR MaxFrameNum = 1 << (sps->log2_max_frame_num_minus4+4);

                if (stream_global->bHasB)
                {
                    nImgB_rd0 = (stream_global->nSwitchB<<1) + ((stream_global->b_Count&1) ? 2:3);

                    if ((stream_global->b_Count&1) || ((stream_global->b_Count&1) == 0 && stream_global->two_B_Flag))
                    {
                        if (nImgB_rd0&1)
                        {
                            gImgB_r1 = img_array[nImgB_rd0];
                            SetEvent(stream_global->m_event_read_start_b[1]);
                        }
                        else
                        {
                            gImgB_r0 = img_array[nImgB_rd0];
                            SetEvent(stream_global->m_event_read_start_b[0]);
                        }
                    }
                }

                stream_global->pic_b_num++;
                if (stream_global->pic_b_num == 4)
                    stream_global->pic_b_num = 0;

                stream_global->number++;
            }

        }
        else
        {
            DEBUG_SHOW_SW_INFO("This Picture is for IP");

            if (stream_global->m_bDoSkipPB == TRUE && IMGPAR firstSlice->picture_type == I_SLICE)
                g_dwSmartDecLevel = stream_global->m_dwSavedSmartDecLevel;

            stream_global->nCollectB = 0;

            stream_global->number++;
            stream_global->pre_frame_num    = IMGPAR firstSlice->frame_num;

            if (IMGPAR firstSlice->picture_type == I_SLICE)
            {
                stream_global->m_listXsize[0] = stream_global->m_listXsize[1] = stream_global->m_listXsize[2] = 
                    stream_global->m_listXsize[3] = stream_global->m_listXsize[4] = stream_global->m_listXsize[5] = 0;
            }
            else
            {            
                stream_global->m_listXsize[0] = IMGPAR firstSlice->num_ref_idx_l0_active;
                stream_global->m_listXsize[2] = stream_global->m_listXsize[4] = IMGPAR firstSlice->num_ref_idx_l0_active * 2;
                stream_global->m_listXsize[1] = IMGPAR firstSlice->num_ref_idx_l1_active;
                stream_global->m_listXsize[3] = stream_global->m_listXsize[5] = IMGPAR firstSlice->num_ref_idx_l1_active * 2;
            }

            nImgIP_r = stream_global->pic_ip_num;
            nImgIP_d = !(nImgIP_r);

        
            if (!stream_global->is_first)
            {
                if(stream_global->bSeekToOpenGOP && IMGPAR firstSlice->picture_type != B_SLICE)  //We should not set bSeekToOpenGOP to 0 in reference-B
                    stream_global->bSeekToOpenGOP = 0;

                if (stream_global->m_bDoSkipPB == TRUE && IMGPAR firstSlice->picture_type == I_SLICE)  //Skip all B frames between I and P when m_bDoSkipPB == TRUE
                    stream_global->bSeekToOpenGOP = 1;                                                   //Same behavior as seek to open GOP

                // P decode
                if (stream_global->nNeedToDecodeIP)    {

                    gImgIP_d = img_array[nImgIP_d];
                    SetEvent(stream_global->m_event_decode_start_ip);                    
                }

                // P read
                fill_frame_gap_flag = 0;
				seq_parameter_set_rbsp_t *sps =  (IMGPAR firstSlice->bIsBaseView) ? &SeqParSet[PicParSet[IMGPAR firstSlice->pic_parameter_set_id].seq_parameter_set_id] :
					&SeqParSubset[PicParSet[IMGPAR firstSlice->pic_parameter_set_id].seq_parameter_set_id];
                IMGPAR MaxFrameNum = 1 << (sps->log2_max_frame_num_minus4+4);
                if (IMGPAR firstSlice->frame_num != IMGPAR firstSlice->pre_frame_num && IMGPAR firstSlice->frame_num != (IMGPAR firstSlice->pre_frame_num + 1) % IMGPAR MaxFrameNum) 
                {
                    if( (IMGPAR smart_dec <= SMART_DEC_LEVEL_3 || IMGPAR smart_dec == SMART_DEC_LEVEL_0) && IMGPAR dwFillFrameNumGap)
                        fill_frame_gap_flag= 1;
                }
                if (!fill_frame_gap_flag)     //Non fill_frame_num_gap
                {
                    gImgIP_r = img_array[nImgIP_r];
                    SetEvent(stream_global->m_event_read_start_ip);    
                }

                //B read & decode
                if (stream_global->bHasB)
                    DecodeProcessBB ARGS0();

                //P decode finish
                if (stream_global->nNeedToDecodeIP)
                {
                    SetEvent(stream_global->m_event_for_field_ip);
                    WaitForSingleObject(stream_global->m_event_decode_finish_ip, INFINITE);
                    stream_global->nNeedToDecodeIP = 0;
                }

                // P read finish
                if (!fill_frame_gap_flag)      //Non fill_frame_num_gap
                {
                    if ( stream_global->m_bRefBWaitB && stream_global->m_bSeekIDR ) {
                        SetEvent(event_RB_1stfield_decode_complete);
                        WaitForSingleObject(event_RB_wait_clear, INFINITE);
                        ResetEvent(event_RB_wait_clear);
                        WaitForSingleObject(stream_global->m_event_read_finish_ip, INFINITE);
                        stream_global->m_bRefBWaitB = FALSE;

                    }
                    
                    WaitForSingleObject(stream_global->m_event_read_finish_ip, INFINITE);
                    ResetEvent(stream_global->m_event_read_finish_ip);
                    ResetEvent(stream_global->m_event_decode_finish_ip);
                }
                else
                {
                    //fill_frame_num_gap
                    ResetEvent(stream_global->m_event_decode_finish_ip);    //Moved from

                    gImgIP_r = img_array[nImgIP_r];
                    SetEvent(stream_global->m_event_read_start_ip);
                    if ( stream_global->m_bRefBWaitB && stream_global->m_bSeekIDR ) {
                        SetEvent(event_RB_1stfield_decode_complete);
                        WaitForSingleObject(event_RB_wait_clear, INFINITE);
                        ResetEvent(event_RB_wait_clear);
                        stream_global->m_bRefBWaitB = FALSE;
                    }

                    WaitForSingleObject(stream_global->m_event_read_finish_ip, INFINITE);

                    //ResetEvent(stream_global->m_event_decode_finish_ip);    //Moved to ?
                    
                    ResetEvent(stream_global->m_event_read_finish_ip);        

					//store these parameters to next collect_pic
					stream_global->PreviousFrameNum[0]				= IMGPAR PreviousFrameNum;
					stream_global->PreviousFrameNumOffset[0]	= IMGPAR PreviousFrameNumOffset;
					stream_global->PreviousPOC[0]						= IMGPAR PreviousPOC;
					stream_global->ThisPOC[0]								= IMGPAR ThisPOC; 
					stream_global->PrevPicOrderCntLsb[0]			= IMGPAR PrevPicOrderCntLsb;
					stream_global->PrevPicOrderCntMsb[0]			= IMGPAR PrevPicOrderCntMsb;
				}

                stream_global->pic_ip_num = nImgIP_d;
                stream_global->nNeedToDecodeIP = 1;

                //We should not set m_bDoSkipPB in reference-B
                if (img_array[nImgIP_r]->SkipThisFrame && img_array[nImgIP_r]->firstSlice->picture_type != B_SLICE && stream_global->m_bDoSkipPB == FALSE)
                {
                    stream_global->m_dwSavedSmartDecLevel = g_dwSmartDecLevel;
                    g_dwSmartDecLevel = SMART_DEC_SKIP_PB;
                    stream_global->m_bDoSkipPB = TRUE;
                }
                else if (g_dwSmartDecLevel != SMART_DEC_SKIP_PB)
                    stream_global->m_bDoSkipPB = FALSE;

                if (stream_global->b_Count & 1)
                {
                    stream_global->two_B_Flag = 0;
                    stream_global->b_Count++;
                    stream_global->pic_b_num++;
                    if (stream_global->pic_b_num == 4)
                        stream_global->pic_b_num = 0;
                }
                else
                    stream_global->two_B_Flag = 1;

                if (stream_global->b_Count >= 2)
                    stream_global->bHasB = 1;
                else
                    stream_global->bHasB = 0;

                if (g_bSkipFirstB < 0 && stream_global->b_Count) //encounter whether it is a GOP with one B
                {
                    g_bSkipFirstB+=2; //add 2 on each, ex: (-3) -> (-1) -> 1
                    if (stream_global->two_B_Flag)
                        g_bSkipFirstB = FALSE; //0
                }

#if defined(_SHOW_THREAD_TIME_)
                DEBUG_INFO("PR:%I64d, PD:%I64d, B1R:%I64d, B1D:%I64d, B2R:%I64d, B2D:%I64d",
                    (1000 * (t_ip[1].QuadPart - t_ip[0].QuadPart) / freq.QuadPart),
                    (1000 * (t_ip[3].QuadPart - t_ip[2].QuadPart) / freq.QuadPart),
                    (1000 * (t_b[1].QuadPart - t_b[0].QuadPart) / freq.QuadPart),
                    (1000 * (t_b[5].QuadPart - t_b[4].QuadPart) / freq.QuadPart),
                    (1000 * (t_b[3].QuadPart - t_b[2].QuadPart) / freq.QuadPart),
                    (1000 * (t_b[7].QuadPart - t_b[6].QuadPart) / freq.QuadPart)
                    );
#endif


                if ( stream_global->m_bMultiThreadModeSwitch == 1 ) {
                    // Complete remaining picture decoding, then jump to multi-slice model in next call

                    if (stream_global->nNeedToDecodeIP)    {
                        gImgIP_d = img_array[nImgIP_r];
                        SetEvent(stream_global->m_event_decode_start_ip);
                    }

                    //B read & decode
                    if (stream_global->bHasB)
                        DecodeProcessBB ARGS0();

                    //P decode finish
                    if (stream_global->nNeedToDecodeIP)
                    {
                        SetEvent(stream_global->m_event_for_field_ip);
                        WaitForSingleObject(stream_global->m_event_decode_finish_ip, INFINITE);
                        stream_global->nNeedToDecodeIP = 0;
                    }

                    stream_global->m_is_MTMS = 1;
                    stream_global->m_bMultiThreadModeSwitch = 0;

                }


                framecount = g_framemgr->GetDisplayCount();
                if(g_framemgr->m_nFlushDPBsize)
                    framecount += dpb.used_size_on_view[view_index];
                if(framecount > pre_framecount)
                    return CREL_OK;
            } else {
                DEBUG_SHOW_SW_INFO("Received First Frame!!");

                //KevinChien: 060722 To fix Fujitsu next chapter
                if(next_image_type == B_SLICE)
                    stream_global->bSeekToOpenGOP = 1;
                //End of 060722

                gImgIP_r = img_array[0];
                SetEvent(stream_global->m_event_read_start_ip);
                WaitForSingleObject(stream_global->m_event_read_finish_ip, INFINITE);
                ResetEvent(stream_global->m_event_read_finish_ip);

                if (img_array[0]->SkipThisFrame && stream_global->m_bDoSkipPB == FALSE)
                {
                    stream_global->m_dwSavedSmartDecLevel = g_dwSmartDecLevel;
                    g_dwSmartDecLevel = SMART_DEC_SKIP_PB;
                    stream_global->m_bDoSkipPB = TRUE;
                }
                else if (g_dwSmartDecLevel != SMART_DEC_SKIP_PB)
                    stream_global->m_bDoSkipPB = FALSE;

                stream_global->is_first = 0;
                stream_global->pic_ip_num++;
                if (stream_global->pic_ip_num == 2)
                    stream_global->pic_ip_num = 0;
                stream_global->nNeedToDecodeIP = 1;
            }
        }        



        // Terry: For smart decoding level 6 (IDR_ONLY) of PIP case, we should return this I frame to reduce the responsive time and to avoid the EOS ending.
        if ( (IMGPAR smart_dec == SMART_DEC_LEVEL_6) )//&& (IMGPAR firstSlice->picture_type == I_SLICE) && !IMGPAR firstSlice->idr_flag)
        {
            DEBUG_SHOW_SW_INFO("DecodePicture: For smart decoding level 6, return the skipped frame.");
            return CREL_ERROR_H264_UNDEFINED;
        }
    }        


#if !defined(ONE_COF)
    IMGPAR cof_r = IMGPAR cof_d = NULL;
#endif

    return CREL_OK;    // okay

}

#endif

/////////////////////////////////////////////////

// called from write_out_picture() in output.c.
int write_out_picture_h264_interface PARGS1(StorablePicture *p)
{
    CH264VideoFrame *f;

#ifdef _COLLECT_PIC_
    StreamParameters *stream_global = IMGPAR stream_global;
#endif

    f = static_cast<CH264VideoFrame *>(p);
    if(f==0 || g_framemgr==0)
        return E_FAIL;

    int view_index = p->view_index;

    bool bCalculatePullDownRate = (p->pull_down_flag&&p->progressive_frame);

    if (!g_pulldown && bCalculatePullDownRate && (g_llDispCount[view_index] != g_llDispPulldownCount[view_index]) && (g_llDispPulldownCount[view_index] >= 4))
    {
        g_pulldown = TRUE;
        g_PulldownRate = (unsigned int) ((1000 * g_llDispCount[view_index]) / g_llDispPulldownCount[view_index]);
        g_PulldownRate /= 2; //due to NumClockTS based on 16ms
        g_llDispCount[view_index] = g_llDispPulldownCount[view_index];

        DEBUG_SHOW_SW_INFO("This frame has a pull down PTS, rate = %u.\n", g_PulldownRate);
    }

    if (IMGPAR SkipControlB)
    {
        g_llDispCount[view_index] += (g_pulldown) ? p->SkipedBFrames[view_index][0] : p->SkipedBFrames[view_index][1];
        g_llDispPulldownCount[view_index] += p->SkipedBFrames[view_index][0];
    }
                       //If no pts (p->has_pts == 0), the g_ref_pts.ts should be set in first frame (g_llDispCount == 0) 
    if (p->has_pts || !g_llDispCount[view_index])
    {
        g_llDispCount[view_index] = 0;
        g_llDispPulldownCount[view_index] = 0;
        g_ref_pts = p->pts;
        DEBUG_SHOW_SW_INFO("This frame has a PTS.\n");
    }
    else if (g_ref_pts.ts != 0) // For special P frames after I, we do not need to give new pts here.
    {
        p->pts = g_ref_pts;
        //p->pts.ts += (unsigned __int64) (g_llDispCount * (((unsigned __int64)p->pts.freq * g_PulldownRate) / p->framerate1000));
        p->pts.ts += (unsigned __int64) ( ( ((float)g_llDispCount[view_index]) + (view_index == 0 ? 0 : 0.5)  ) * ( ((unsigned __int64)p->pts.freq * g_PulldownRate) / (p->framerate1000*(g_pulldown==FALSE ? (p->pull_down_flag+1):1)) ) );
    }

    DEBUG_SHOW_SW_INFO("Disp[%3I64d]: Type = %d, PDCount = %I64d, ClockTs = %d, SkipedB[0] = %u, SkipedB[1] = %u, pts.ts = %I64d, HW_index : %d\n", g_llDispCount[view_index], p->slice_type, g_llDispPulldownCount[view_index], p->NumClockTs, p->SkipedBFrames[0], p->SkipedBFrames[1], p->pts.ts, p->pic_store_idx);

    if (g_pulldown)
        g_llDispCount[view_index]++;
    else
    {
        g_llDispCount[view_index] += p->NumClockTs;
        g_llDispPulldownCount[view_index]++;
    }

    p->is_output = 1;
    if(p->mem_layout==0)
    {
        if(p->top_field && p->top_field->mem_layout)
        {
            f->dep[0] = static_cast<CH264VideoFrame *>(p->top_field);
            f->dep[0]->AddRef();
        }
        if(p->bottom_field && p->bottom_field->mem_layout)
        {
            f->dep[1] = static_cast<CH264VideoFrame *>(p->bottom_field);
            f->dep[1]->AddRef();
        }
        if(p->frame && p->frame->mem_layout)
        {
            f->dep[2] = static_cast<CH264VideoFrame *>(p->frame);
            f->dep[2]->AddRef();
        }
    }

#if defined (_HW_ACCEL_)
    f->m_DXVAVer = imgDXVAVer;
#endif
    g_framemgr->PutDisplayFrame(f);

    g_bDisplayed = TRUE;

    return S_OK;
}

void init_storable_picture(StorablePicture *s, PictureStructure structure, int size_x, int size_y, int size_x_cr, int size_y_cr, int stride, int stride_UV)
{
    if(structure!=FRAME)
    {
        size_y /= 2;
        size_y_cr /= 2; 
    }
    s->PicSizeInMbs = (size_x*size_y)>>8;
    s->structure = structure;
    s->size_x = size_x;
    s->size_y = size_y;
    s->size_x_cr = size_x_cr;
    s->size_y_cr = size_y_cr; 
    s->pic_num = 0;
    s->frame_num = (unsigned int)-1;
    s->long_term_frame_idx = 0;
    s->long_term_pic_num = 0;
    s->used_for_reference = 0;
    s->is_long_term = 0;
    s->non_existing = 0;
    s->is_output = 0;
    s->max_slice_id = 0;
    s->top_field = 0;
    s->bottom_field = 0;
    s->frame = 0;
    s->dec_ref_pic_marking_buffer = 0;
    s->coded_frame = 0;
    s->MbaffFrameFlag = 0;
    s->Y_stride = stride;
    s->UV_stride = stride_UV;
    s->pic_store_idx = -1;    
    s->pull_down_flag = 0;
    s->repeat_first_field = 0;
    s->top_field_first = 1;
    s->dwXAspect = 0;
    s->dwYAspect = 0;
    s->decoded_mb_number = 0;
    s->dwYCCBufferLen = 0;
    s->has_PB_slice = 0;
}

StorablePicture *get_storable_picture PARGS7(PictureStructure structure, int size_x, int size_y, int size_x_cr, int size_y_cr, int mem_layout, int non_exist)
{
    StorablePicture *p;
    int iReallocate;
    int istruct = structure;
    int imem = mem_layout;
    DEBUG_SHOW_SW_INFO("Get (%s) StorablePicture", structure==FRAME ? "FRAME" : (structure==TOP_FIELD ? "TOP FIELD" : "BOTTOM FIELD"));

#ifdef _COLLECT_PIC_
    StreamParameters *stream_global = IMGPAR stream_global;
#endif

    p = g_framemgr->GetFreeFrame(&istruct, &imem);
/*
    if ((img->stream_global->m_img[5]->m_listX) && ((unsigned int)(img->stream_global->m_img[5]->m_listX[1]) != 0xcdcdcdcd) && (p == img->stream_global->m_img[5]->m_listX[1][0]) && (img == img->stream_global->m_img[4])) {
        p = p;
    }
*/
    if(p==0)
    {
        DEBUG_SHOW_SW_INFO("ALLOCATION ERROR!\n");
        return 0;
    }
    if(p->imgY==0)
        alloc_storable_picture ARGS9(structure, size_x, size_y, size_x_cr, size_y_cr, p, mem_layout, 0, non_exist);
    iReallocate = 0;
    if(mem_layout!=p->mem_layout || structure!=p->structure)
        iReallocate = 1;
    else if(structure==FRAME)
    {
        if(p->size_x!=size_x || p->size_y!=size_y || p->size_x_cr!=size_x_cr || p->size_y_cr!=size_y_cr)
            iReallocate = 1;
    }
    else
    {
        if(p->size_x!=size_x || (p->size_y*2)!=size_y || p->size_x_cr!=size_x_cr || (p->size_y_cr*2)!=size_y_cr)
            iReallocate = 1;
    }
    if ((iReallocate==1) && (!non_exist))
    {
        DEBUG_SHOW_SW_INFO("reallocating memory\n");
        free_storable_picture(p,false);
        alloc_storable_picture ARGS9(structure, size_x, size_y, size_x_cr, size_y_cr, p, mem_layout, 1, 0);
    }
    init_storable_picture(p,structure,size_x,size_y,size_x_cr,size_y_cr,p->Y_stride,p->UV_stride);
    p->non_existing = non_exist;

    clip_max_x      = size_x+1;
    clip_max_x_cr   = size_x_cr-1;
    return p;
}

void release_storable_picture PARGS2(StorablePicture *p, BYTE flag)
{
    int temp;
    CH264VideoFrame *f;
    f = static_cast<CH264VideoFrame *>(p);
    if(f==0)
        return;
    int nPicStoreIdx = f->pic_store_idx;
    temp = f->Release();
    if(temp==0 && flag==1 && nPicStoreIdx>=0)
        IMGPAR FP_ReleaseDecodeFrame ARGS1(nPicStoreIdx);
}

//////////////////////////////////////////////////////////////////////////
// CH264VDec class implementation
//////////////////////////////////////////////////////////////////////////

#define GMI_FREQ_90KHZ 90000

void CH264VDec::SetDefaultOpenOptions()
{
    m_pOpenOptions->dwThreads = 1;
    m_pOpenOptions->dwThreadAffinity = 0;
    m_pOpenOptions->dwBuffers = 8;
    m_pOpenOptions->dwFillFrameNumGap = 0;

    m_pOpenOptions->pfnDataCallback = NULL;
    m_pOpenOptions->pvDataContext = NULL;
    m_pOpenOptions->dwH264RegKey = 2; //set smart_dec=2 as default
    m_pOpenOptions->dxvaVer = IviNotDxva;

    m_pOpenOptions->uiH264VGACard = 0;
    m_pOpenOptions->uiH264DXVAMode = 0;

    m_pOpenOptions->pfnGetParamCallback = NULL;
    m_pOpenOptions->pIviCP = NULL;

    m_pOpenOptions->dwSingleThreadMode = 0;
}

CH264VDec::CH264VDec()
{
    m_pOpenOptions = new H264VDecHP_OpenOptionsEx;

    SetDefaultOpenOptions();

    m_pStream = NULL;

    bGotNextNalu = FALSE;

	m_iMVCFieldEOSActiveIndex = 0;

	m_bMVCFieldEOS = FALSE;

	m_bIsAppConstrantsBD3D = FALSE;
}

CH264VDec::~CH264VDec()
{
    delete(m_pOpenOptions);

}

BOOL CH264VDec::IsMVCFieldPicture()
{
	if (!m_pStream || !stream_global->m_active_sps_on_view[1])
		return FALSE;

#if !defined(_COLLECT_PIC_)
	ImageParameters *img = (ImageParameters*) m_pStream;
#else
	StreamParameters *pStream = (StreamParameters*) m_pStream;
	ImageParameters *img = pStream->m_img[m_iMVCFieldEOSActiveIndex];
#endif

	if(stream_global->bMVC && IMGPAR firstSlice)
		return IMGPAR firstSlice->field_pic_flag;
	else
		return FALSE;
}

/*
Return Value:
CREL_OK: collected one frame and buf_begin==buf_end
S_FALSE: collected one frame and still exist more data(buf_begin!=buf_end)
CREL_ERROR_H264_UNDEFINED: only buf_begin==buf_end
*/
HRESULT CH264VDec::CollectOneFrame(BOOL bEOS, NALU_t *one_nalu)
{
    DEBUG_INFO("[CH264VDec] CollectOneFrame() EOS: %d", bEOS);

    if (!m_pStream)
        return E_FAIL;

#if !defined(_COLLECT_PIC_)
    ImageParameters *img = (ImageParameters*) m_pStream;
#else
    StreamParameters *pStream = (StreamParameters*) m_pStream;
    ImageParameters *img = pStream->m_img[0];
#endif

    if (!g_has_pts && one_nalu->pts.ts)
    {
        g_pts = one_nalu->pts;
        g_has_pts = 1;
    }

	CREL_RETURN ret = CREL_ERROR_H264_UNDEFINED;
	if(m_bMVCFieldEOS)
	{
		HRESULT hr;

		if(m_iMVCFieldEOSActiveIndex >= stream_global->num_views)
			return CREL_ERROR_H264_UNDEFINED;

		img = img_array[m_iMVCFieldEOSActiveIndex];

		if (IMGPAR slice_number == 0)
			return CREL_ERROR_H264_UNDEFINED;

		IMGPAR currentSlice->exit_flag = 1;

		if(m_iMVCFieldEOSActiveIndex < stream_global->num_views - 1)
			hr = S_FALSE;
		else// the last frame
			hr = S_OK;
		
		m_iMVCFieldEOSActiveIndex++;

		m_pActiveImg = img;

		return hr;
	}
	else if (bEOS)
	{
		if (!nalu_global_available)
		{
			if (next_image_type==B_SLICE && !g_bNextImgForRef)
				img = img_array[stream_global->pic_b_num+2];
			else
				img = img_array[stream_global->pic_ip_num];

            if (IMGPAR slice_number == 0)
                return CREL_ERROR_H264_UNDEFINED;

            IMGPAR currentSlice->exit_flag = 1;

            //collected one frame and store its IMGPAR
            m_pActiveImg = img;

            return CREL_OK;
        }
        else
        {
            DEBUG_SHOW_SW_INFO("[CH264VDec] CollectOneFrame() on EOS!!");
            DoCollectOneFrame ARGS0();
            m_pActiveImg = img;
            return CREL_OK; //Force to enter DecodeThisFrame to flush all frames!
        }
    }
    else
    {
        if (!nalu_global_available)
            CopyNALU(nalu_global, one_nalu);

        ret = DoCollectOneFrame ARGS0();

        return ret;
    }
}

HRESULT CH264VDec::DoCollectOneFrame PARGS0()
{
    CREL_RETURN ret;
    BOOL bIsForReference;
    int i, header;
    static BOOL bAlreadyReset;
    
    if (g_bNextImgForRef)
        bIsForReference = TRUE;
    else
        bIsForReference = FALSE;

    if (stream_global->m_bSeekIDR == FALSE) {
        bAlreadyReset = FALSE;
    }

    if (next_image_type==B_SLICE && !g_bNextImgForRef && (stream_global->m_bSeekIDR == FALSE))
    {
        if (stream_global->nCollectB==2 && stream_global->b_Count>=2)
        {
            if (stream_global->nNeedToDecodeIP)
            {
                int nImgIP_d = !(stream_global->pic_ip_num);

                gImgIP_d = img_array[nImgIP_d];
                SetEvent(stream_global->m_event_decode_start_ip);

                //B read & decode
                if (stream_global->bHasB)
                    DecodeProcessBB ARGS0();

                SetEvent(stream_global->m_event_for_field_ip);
                WaitForSingleObject(stream_global->m_event_decode_finish_ip, INFINITE);
                ResetEvent(stream_global->m_event_decode_finish_ip);

                stream_global->nNeedToDecodeIP = 0;
            }

            //B read & decode
            int nImgB_rd0 = (stream_global->nSwitchB<<1) + 2;
            int nImgB_rd1 = (stream_global->nSwitchB<<1) + 3;

            stream_global->two_B_Flag = 1;    //Bug fixed by Haihua to cover BBetweenP = 3, 5, 7, .... Odd and larger than 2

            gImgB_r0 = img_array[nImgB_rd0];
            gImgB_r1 = img_array[nImgB_rd1];
            SetEvent(stream_global->m_event_read_start_b[0]);
            SetEvent(stream_global->m_event_read_start_b[1]);

            WaitForSingleObject(stream_global->m_event_decode_finish_b[0], INFINITE);
            WaitForSingleObject(stream_global->m_event_decode_finish_b[1], INFINITE);
            ResetEvent(stream_global->m_event_decode_finish_b[0]);
            ResetEvent(stream_global->m_event_decode_finish_b[1]);

            stream_global->nSwitchB = !(stream_global->nSwitchB);
            stream_global->b_Count -= 2;

            stream_global->two_B_Flag = 0;
            stream_global->bHasB = 0;

            stream_global->nCollectB = 0;
        }

        img = img_array[stream_global->pic_b_num+2];
    }
    else {

        if (stream_global->m_bSeekIDR && (bAlreadyReset == FALSE) ) {
            stream_global->pic_ip_num = 0;
            stream_global->is_first = 1;
            dpb.last_output_poc = INT_MIN;
            stream_global->nNeedToDecodeIP = 0;
            next_image_type = -1;
        }

        img = img_array[stream_global->pic_ip_num];

        if (stream_global->m_bSeekIDR && (bAlreadyReset == FALSE) ) {

            if (stream_global->m_bRefBWaitB) {
                SetEvent(event_RB_1stfield_decode_complete);
                WaitForSingleObject(event_RB_wait_clear, INFINITE);
                ResetEvent(event_RB_wait_clear);
                WaitForSingleObject(stream_global->m_event_read_finish_ip, INFINITE);
                stream_global->m_bRefBWaitB = FALSE;
            }

            if (stream_global->bHasB && (stream_global->m_is_MTMS == 0 && stream_global->m_bIsSingleThreadMode == FALSE)) {
                DecodeProcessBB ARGS0();
            }

            stream_global->bHasB = 0;
            stream_global->b_Count = 0;
            stream_global->nSwitchB = 0;
            stream_global->two_B_Flag = 0;                
            stream_global->nCollectB = 0;                
            stream_global->pic_b_num = 0;


            for ( i = 0; i < stream_global->num_views; i++) {
                flush_dpb ARGS1(i);
                update_ref_list ARGS1(0);
                update_ltref_list ARGS1(0);        
            }
            

            ResetEvent(event_RB_1stfield_decode_complete);
            ResetEvent(stream_global->m_event_read_finish_ip);
            
            if (stream_global->m_is_MTMS >= 0 || stream_global->m_bIsSingleThreadMode == TRUE)
            {
                int nIMG_num = (stream_global->m_bIsSingleThreadMode == TRUE)? 1: 6;
                for (int i = 0; i < nIMG_num; i++ ) {

                    img = img_array[i];

                    IMGPAR currentSlice = IMGPAR firstSlice;
                    for (int j=0; j< IMGPAR slice_number; j++)
                    {
                        //active_pps = &PicParSet[IMGPAR currentSlice->pic_parameter_set_id];
                        memcpy(&active_pps, &PicParSet[IMGPAR currentSlice->pic_parameter_set_id], sizeof(pic_parameter_set_rbsp_t));
                        IMGPAR prevSlice = IMGPAR currentSlice;
                        IMGPAR currentSlice = (Slice*) IMGPAR currentSlice->nextSlice;
                        free_ref_pic_list_reordering_buffer(IMGPAR prevSlice);
                        free_new_slice(IMGPAR prevSlice);                    

                    }

                    if (IMGPAR structure == FRAME) {

                        if(dec_picture)
                        {
                            int view_index = (IMGPAR firstSlice) ? (IMGPAR firstSlice->viewIndex) : 0;
                            if ((dpb.used_size_on_view[view_index] == 0) ||dpb.fs_on_view[view_index][dpb.used_size_on_view[view_index] - 1]->frame != dec_picture) {
                                release_storable_picture ARGS2(dec_picture, 1);
                            }
                            dec_picture = NULL;
                        }
                        img->m_dec_picture_top = NULL;
                        img->m_dec_picture_bottom = NULL;

                    } else {

                        if (img->m_dec_picture_top) {                        

                            release_storable_picture ARGS2(img->m_dec_picture_top, 1);                        
                            img->m_dec_picture_top = NULL;
                        }

                        if (img->m_dec_picture_bottom) {

                            release_storable_picture ARGS2(img->m_dec_picture_bottom, 1);                        
                            img->m_dec_picture_bottom = NULL;
                        }
                        dec_picture = NULL;

                    }    

                    IMGPAR firstSlice = IMGPAR currentSlice = IMGPAR prevSlice = NULL;
                    IMGPAR current_mb_nr_r = IMGPAR current_mb_nr_d = -4712;   // impossible value for debugging, StW
                    IMGPAR error_mb_nr = -4712;
                    IMGPAR current_slice_nr = 0;
                    IMGPAR slice_number = 0;
                    img->m_active_pps[0].Valid = NULL;
                    img->m_active_pps[1].Valid = NULL;
                    //dec_picture = NULL;

                    if (IMGPAR structure != FRAME)
                    {
                        IMGPAR mb_decdata = IMGPAR mb_decdata_ori;
                        IMGPAR cof_array = IMGPAR cof_array_ori;
                    }

                }

            }//if (stream_global->m_is_MTMS >= 0 || stream_global->m_bIsSingleThreadMode == TRUE)

            img = img_array[0];
            bAlreadyReset = TRUE;
            
        }

    }

	IMGPAR number                 = stream_global->number;
	IMGPAR pre_frame_num          = stream_global->pre_frame_num;
	IMGPAR PreviousFrameNum       = stream_global->PreviousFrameNum[0];
	IMGPAR PreviousFrameNumOffset = stream_global->PreviousFrameNumOffset[0];
	IMGPAR PreviousPOC            = stream_global->PreviousPOC[0];    
	IMGPAR ThisPOC                = stream_global->ThisPOC[0];
	IMGPAR PrevPicOrderCntLsb     = stream_global->PrevPicOrderCntLsb[0];
	IMGPAR PrevPicOrderCntMsb     = stream_global->PrevPicOrderCntMsb[0];
	IMGPAR last_has_mmco_5        = stream_global->last_has_mmco_5;
	for(i=0; i<6; i++)
		IMGPAR m_listXsize[i] = stream_global->m_listXsize[i];

    stream_global->last_has_mmco_5 = 0;

    //Terry: for multi-slice case, smart_dec should be assigned before collect picture.
    IMGPAR smart_dec = g_dwSmartDecLevel;

	ret = DoProcessOneNALU ARGS1(&header);

    //collected one frame and store its IMGPAR
    if (SUCCEEDED(ret)) {
        
/*
        if (img->stream_global->number >= 3235) {
            img->number = img->number;
        }
*/
		if(stream_global->bMVC == TRUE && IMGPAR firstSlice->field_pic_flag == TRUE )
			img = img_array[stream_global->m_iMVCFieldPicActiveIndex];
		
		m_pActiveImg = img;
	}

    return ret;
}

void CH264VDec::SetNextOutputViewIndex()
{
    stream_global->m_CurrOutputViewIndex++;
    if(stream_global->m_CurrOutputViewIndex == 16)
        stream_global->m_CurrOutputViewIndex = 0;

    while (stream_global->m_pbValidViews[stream_global->m_CurrOutputViewIndex] == 0)
    {
        stream_global->m_CurrOutputViewIndex++;
        if(stream_global->m_CurrOutputViewIndex == 16)
            stream_global->m_CurrOutputViewIndex = 0;
    }
}

HRESULT CH264VDec::DoProcessOneNALU PARGS1(int *header)
{
	DecodingEnvironment *dep;		
	BOOL bFirstSlice = (IMGPAR slice_number==0) ? TRUE:FALSE;
	Slice *newSlice;
	CREL_RETURN ret;
	int primary_pic_type = -1;
	static BOOL bSeekIDR_backup;

    if (bFirstSlice)
        nalu->buf = IMGPAR ori_nalu_buf;

    nalu->startcodeprefix_len = nalu_global->startcodeprefix_len;
    nalu->len                 = nalu_global->len;
    nalu->max_size            = nalu_global->max_size;
    nalu->nal_unit_type       = nalu_global->nal_unit_type;
    nalu->nal_reference_idc   = nalu_global->nal_reference_idc;
    nalu->forbidden_bit       = nalu_global->forbidden_bit;
    nalu->pts                 = nalu_global->pts;
    memcpy(nalu->buf, nalu_global->buf, nalu_global->len);

    nalu_global_available = 0;

    switch (nalu->nal_unit_type)
    {
    case NALU_TYPE_SLICE:
    case NALU_TYPE_IDR:
    case NALU_TYPE_SLICE_EXT:

        DEBUG_SHOW_SW_INFO("-- NALU_TYPE_SLICE -- NALU_TYPE_IDR --");

        ret = malloc_new_slice(&newSlice);

        // Frame management: This is the part we have to reset pic_combine_status
        //IMGPAR idr_flag = (nalu->nal_unit_type == NALU_TYPE_IDR);
        if(nalu->nal_unit_type == NALU_TYPE_IDR)
        { 
            DEBUG_SHOW_SW_INFO("-- This is IDR --");
            IMGPAR idr_flag = newSlice->idr_flag = 1;
            stream_global->m_bSeekIDR = FALSE;
        }
        else
        {
            IMGPAR idr_flag = newSlice->idr_flag = 0;

        }

        IMGPAR nal_reference_idc = newSlice->nal_reference_idc = nalu->nal_reference_idc;
        IMGPAR disposable_flag = newSlice->disposable_flag = (nalu->nal_reference_idc == NALU_PRIORITY_DISPOSABLE);

        newSlice->dp_mode = PAR_DP_1;
        newSlice->max_part_nr = 1;
        newSlice->ei_flag = 0;
        newSlice->nextSlice = NULL;
        dep              = newSlice->g_dep;
        dep->Dei_flag    = 0;
        dep->Dbits_to_go = 0;
        dep->Dbuffer     = 0;            
        if (nalu->nal_unit_type == NALU_TYPE_SLICE_EXT) {

            ret = ProcessNaluExt ARGS1(nalu);
            stream_global->nalu_mvc_extension.bIsPrefixNALU = FALSE;
            dep->Dbasestrm = &(nalu->buf[4]);
            dep->Dstrmlength = nalu->len-4;
        } else {
            dep->Dbasestrm = &(nalu->buf[1]);
            dep->Dstrmlength = nalu->len-1;
        }
        
        ret = RBSPtoSODB(dep->Dbasestrm, &(dep->Dstrmlength));

        dep->Dcodestrm   = dep->Dbasestrm;        

        if (nalu->nal_unit_type == NALU_TYPE_SLICE_EXT) {
            
            CopyNaluExtToSlice ARGS1 (newSlice);
			newSlice->bIsBaseView = false;
            //stream_global->nalu_mvc_extension.valid = FALSE;
        }
        else
        {
			newSlice->bIsBaseView = true;
            if(stream_global->m_active_sps_on_view[1] == 0)
            {
                newSlice->viewId = GetBaseViewId ARGS0 ();
                if(stream_global->nalu_mvc_extension.valid && stream_global->nalu_mvc_extension.bIsPrefixNALU && newSlice->viewId == stream_global->nalu_mvc_extension.viewId)
                {
                    newSlice->interViewFlag = stream_global->nalu_mvc_extension.interViewFlag;
                    newSlice->anchorPicFlag = stream_global->nalu_mvc_extension.anchorPicFlag;
                }
                else
                {
                    newSlice->interViewFlag = 1;
                    newSlice->anchorPicFlag = 0;
                }
            }
            else
            {
                if(stream_global->nalu_mvc_extension.valid && stream_global->nalu_mvc_extension.bIsPrefixNALU)
                {
                    newSlice->viewId = stream_global->nalu_mvc_extension.viewId;
                    newSlice->interViewFlag = stream_global->nalu_mvc_extension.interViewFlag;
                    newSlice->anchorPicFlag = stream_global->nalu_mvc_extension.anchorPicFlag;
                }
                else
                {
                    newSlice->viewId = stream_global->m_active_sps_on_view[1]->view_id[0];
                    newSlice->interViewFlag = 1;
                    newSlice->anchorPicFlag = 0;
                }
            }
        }
        newSlice->viewIndex = GetViewIndex ARGS1 (newSlice->viewId);

        stream_global->m_pbValidViews[newSlice->viewIndex] = 1;
        if(stream_global->m_CurrOutputViewIndex == -1)
            stream_global->m_CurrOutputViewIndex = newSlice->viewIndex;

		if(newSlice->viewIndex > 0)
		{
			if(img != img_array[newSlice->viewIndex])
			{
				BOOL bFieldPicFlag = (IMGPAR firstSlice)? IMGPAR firstSlice->field_pic_flag: FALSE;
				if(bFieldPicFlag)
				{
					img = img_array[newSlice->viewIndex];
					ret = DoProcessOneNALU ARGS1(header);
					return ret;
				}
			}
		}
		
		if (bFirstSlice)
		{
			IMGPAR prevSlice  = NULL;
			IMGPAR firstSlice = newSlice;
			IMGPAR currentSlice  = IMGPAR firstSlice;
		}
		else
		{
			IMGPAR prevSlice  = IMGPAR currentSlice;
			IMGPAR currentSlice->nextSlice = (void*)newSlice;
			IMGPAR currentSlice = (Slice*) (IMGPAR currentSlice->nextSlice);
		}

        if (bFirstSlice) {
            bSeekIDR_backup = stream_global->m_bSeekIDR;
        }    

        ret = ParseSliceHeader ARGS0();
        
        if ( FAILED(ret) ) {

            if (g_has_pts)
            {
                g_has_pts = 0;
                DEBUG_INFO("[Drop PTS]: ts: %I64d freq: %d, tslength: %d flags: %d unused1: %d unused2: %d", nalu->pts.ts, nalu->pts.freq, nalu->pts.tslength, nalu->pts.flags, nalu->pts.unused1, nalu->pts.unused2);
            }

            if (bSeekIDR_backup) {
                free_new_slice(newSlice);
                newSlice = NULL;        

                stream_global->m_bSeekIDR = TRUE;

                nalu->buf = IMGPAR ori_nalu_buf;                    
                IMGPAR slice_number = 0;
                break;

            }

            if ( ISWARNING(ret) ) {                    

                free_new_slice(newSlice);
                newSlice = NULL;                    

                if (stream_global->m_bSeekIDR) {
                    nalu->buf = IMGPAR ori_nalu_buf;                    
                    IMGPAR slice_number = 0;
                    
                } else if (nalu->nal_unit_type == NALU_TYPE_IDR) {
                    stream_global->m_bSeekIDR = TRUE;
                    nalu->buf = IMGPAR ori_nalu_buf;

                }                

                break;

            } else {
                return ret;
            }

        }

        //this picture type is for CheckSkipFrame().
        IMGPAR currentSlice->AU_type = IMGPAR currentSlice->picture_type;            
        if(IMGPAR currentSlice->idr_flag)
            IMGPAR currentSlice->AU_type = IDR_SLICE;
        if(IMGPAR currentSlice->picture_type == B_SLICE && IMGPAR currentSlice->nal_reference_idc )
            IMGPAR currentSlice->AU_type = RB_SLICE;

		//Error Check Code
		if ((stream_global->profile_idc == 0) || 
			((g_bReceivedFirst == 0) && (IMGPAR firstSlice->picture_type != I_SLICE) && !(stream_global->bMVC && IMGPAR firstSlice->viewIndex != 0 && IMGPAR firstSlice->field_pic_flag == TRUE))
			)
		{
			DEBUG_SHOW_SW_INFO("ERROR! First frame is not intra after reset decoder!!");
			DEBUG_SHOW_SW_INFO("profile_idc: %d  number: %d  type: %d  ref_idx: %d", stream_global->profile_idc, stream_global->number, IMGPAR type, IMGPAR num_ref_idx_l0_active);

            if (nalu->pts.ts)
                g_has_pts = 1;

            nalu->buf = IMGPAR ori_nalu_buf;

            free_new_slice(newSlice);

            newSlice = NULL;

            IMGPAR slice_number = 0;

            //Yolk: Reset this value!
            for ( unsigned int i = 0; i < stream_global->num_views; i++) {
                stream_global->m_active_sps_on_view[i]= 0;
            }

            if (g_has_pts)
            {
                g_has_pts = 0;
                DEBUG_INFO("[Drop PTS]: ts: %I64d freq: %d, tslength: %d flags: %d unused1: %d unused2: %d", nalu->pts.ts, nalu->pts.freq, nalu->pts.tslength, nalu->pts.flags, nalu->pts.unused1, nalu->pts.unused2);
            }

            return CREL_ERROR_H264_UNDEFINED;
        }

        if(is_new_picture ARGS0())
        {
            seq_parameter_set_rbsp_t *sps;
            pic_parameter_set_rbsp_t *pps;

            DEBUG_SHOW_SW_INFO("This is new Picture");
            ret = decode_poc ARGS0();
            if (FAILED(ret)) {

                if (g_has_pts)
                {
                    g_has_pts = 0;
                    DEBUG_INFO("[Drop PTS]: ts: %I64d freq: %d, tslength: %d flags: %d unused1: %d unused2: %d", nalu->pts.ts, nalu->pts.freq, nalu->pts.tslength, nalu->pts.flags, nalu->pts.unused1, nalu->pts.unused2);
                }

                if (bSeekIDR_backup) {                            

                    stream_global->m_bSeekIDR = TRUE;
                    nalu->buf = IMGPAR ori_nalu_buf;                    
                    IMGPAR slice_number = 0;

                }

                free_new_slice(newSlice);
                newSlice = NULL;

                break;
            }

            DEBUG_SHOW_SW_INFO( "This Picture POC: %d", IMGPAR ThisPOC);

            IMGPAR currentSlice->framerate1000 = g_framerate1000;

            if (IMGPAR prevSlice)
                IMGPAR prevSlice->exit_flag = 1;

            *header = SOP;
            //Picture or Field
            if(IMGPAR currentSlice->field_pic_flag)
            {
                //Field Picture
                if( (IMGPAR prevSlice!= NULL && IMGPAR prevSlice->field_pic_flag)
                    &&                    
                    ( (IMGPAR currentSlice->structure==BOTTOM_FIELD && IMGPAR prevSlice!=NULL && 
                    IMGPAR prevSlice->frame_num == IMGPAR currentSlice->frame_num && IMGPAR prevSlice->m_pic_combine_status==TOP_FIELD)
                    ||
                    (IMGPAR currentSlice->structure==TOP_FIELD && IMGPAR prevSlice!=NULL && 
                    IMGPAR prevSlice->frame_num == IMGPAR currentSlice->frame_num && IMGPAR prevSlice->m_pic_combine_status==BOTTOM_FIELD) )
                    )
                {
                    //Second Filed of New Picture
                    UpdatePTS ARGS0();

                    //store these parameters to next collect_pic
                    if (bSeekIDR_backup && img->currentSlice->picture_type == B_SLICE) {
                        free_new_slice(newSlice);
                        newSlice = NULL;        

                        stream_global->m_bSeekIDR = TRUE;

                        nalu->buf = IMGPAR ori_nalu_buf;                    
                        IMGPAR slice_number = 0;
                        break;
                    }

					//For MVC field coding, different view index would be collected in different IMAGE array. 
					//So the following temp variables will be saved in different view index.
					//For non-MVC field coding, these will be saved in the index 0.
					int iMVCFieldViewindex = 0;
					if(stream_global->bMVC && IMGPAR firstSlice)
						iMVCFieldViewindex = IMGPAR firstSlice->viewIndex;
					stream_global->PreviousFrameNum[iMVCFieldViewindex]				= IMGPAR PreviousFrameNum;
					stream_global->PreviousFrameNumOffset[iMVCFieldViewindex]		= IMGPAR PreviousFrameNumOffset;
					stream_global->PreviousPOC[iMVCFieldViewindex]					= IMGPAR PreviousPOC;
					stream_global->ThisPOC[iMVCFieldViewindex]						= IMGPAR ThisPOC; 
					stream_global->PrevPicOrderCntLsb[iMVCFieldViewindex]			= IMGPAR PrevPicOrderCntLsb;
					stream_global->PrevPicOrderCntMsb[iMVCFieldViewindex]			= IMGPAR PrevPicOrderCntMsb;

                    //Combine Two Filed
                    IMGPAR currentSlice->header = SOP;
                    IMGPAR currentSlice->m_pic_combine_status = 0; //Picture is Full
                    IMGPAR slice_number++;

                    nalu->buf += nalu->len;

                    g_bReceivedFirst = 1;

					sps =  (IMGPAR currentSlice->bIsBaseView) ? &SeqParSet[PicParSet[IMGPAR currentSlice->pic_parameter_set_id].seq_parameter_set_id] :
						&SeqParSubset[PicParSet[IMGPAR currentSlice->pic_parameter_set_id].seq_parameter_set_id];
                    pps =  &PicParSet[IMGPAR currentSlice->pic_parameter_set_id];
                    activate_sps ARGS2(sps, IMGPAR currentSlice->viewId);
                    activate_pps ARGS2(pps, IMGPAR currentSlice->viewIndex);

                    return CREL_ERROR_H264_UNDEFINED;
                }
                else if (IMGPAR slice_number == 0)                
                {
                    //First Filed of New Picture
                    UpdatePTS ARGS0();

                    ZeroMemory(&(IMGPAR m_CCCode), sizeof(H264_CC));
                    SetH264CCCode ARGS0();

                    IMGPAR currentSlice->header = SOP;
                    IMGPAR currentSlice->m_pic_combine_status = IMGPAR currentSlice->structure; //first field
                    IMGPAR slice_number++;

                    nalu->buf += nalu->len;

					sps =  (IMGPAR currentSlice->bIsBaseView) ? &SeqParSet[PicParSet[IMGPAR currentSlice->pic_parameter_set_id].seq_parameter_set_id] :
						&SeqParSubset[PicParSet[IMGPAR currentSlice->pic_parameter_set_id].seq_parameter_set_id];
                    pps =  &PicParSet[IMGPAR currentSlice->pic_parameter_set_id];
                    activate_sps ARGS2(sps, IMGPAR currentSlice->viewId);
                    activate_pps ARGS2(pps, IMGPAR currentSlice->viewIndex);

                    IMGPAR currentSlice->m_nDispPicStructure = sps->vui_seq_parameters.picture_structure;

					return CREL_ERROR_H264_UNDEFINED;
				}
				else
				{					
					//Cpoy naul to nalu_global					
					nalu_global->startcodeprefix_len = nalu->startcodeprefix_len;
					nalu_global->len                 = nalu->len;
					nalu_global->max_size            = nalu->max_size;
					nalu_global->nal_unit_type       = nalu->nal_unit_type;
					nalu_global->nal_reference_idc   = nalu->nal_reference_idc;
					nalu_global->forbidden_bit       = nalu->forbidden_bit;
					nalu_global->pts                 = nalu->pts;
					memcpy(nalu_global->buf, nalu->buf, nalu->len);

                    nalu_global_available = 1;
					int iMVCFieldViewindex = 0;
					if(stream_global->bMVC)
					{
						iMVCFieldViewindex = (IMGPAR firstSlice)? IMGPAR firstSlice->viewIndex: 0;
						if(iMVCFieldViewindex == stream_global->num_views - 1)
							g_bNextImgForRef = (stream_global->m_img[0]->nal_reference_idc != 0);
						else
							g_bNextImgForRef = (stream_global->m_img[iMVCFieldViewindex+1]->nal_reference_idc != 0);
					}
					else
						g_bNextImgForRef = (nalu_global->nal_reference_idc!=0);
					
					//For MVC field coding, different view index would be collected in different IMAGE array. 
					//So the following temp variables will be saved in different view index.
					//For non-MVC field coding, these will be saved in the index 0.
					
					IMGPAR PreviousFrameNum				= stream_global->PreviousFrameNum[iMVCFieldViewindex];
					IMGPAR PreviousFrameNumOffset		= stream_global->PreviousFrameNumOffset[iMVCFieldViewindex];
					IMGPAR PreviousPOC					= stream_global->PreviousPOC[iMVCFieldViewindex];
					IMGPAR ThisPOC						= stream_global->ThisPOC[iMVCFieldViewindex]; 
					IMGPAR PrevPicOrderCntLsb			= stream_global->PrevPicOrderCntLsb[iMVCFieldViewindex];
					IMGPAR PrevPicOrderCntMsb			= stream_global->PrevPicOrderCntMsb[iMVCFieldViewindex];
					stream_global->m_iMVCFieldPicActiveIndex = iMVCFieldViewindex;

					if(stream_global->bMVC)
					{
						if(iMVCFieldViewindex == stream_global->num_views - 1)
							next_image_type = stream_global->m_img[0]->type;
						else
							next_image_type = stream_global->m_img[iMVCFieldViewindex+1]->type;
					}
					else
						next_image_type = IMGPAR type;

                    IMGPAR currentSlice = IMGPAR prevSlice;
                    IMGPAR currentSlice->nextSlice = NULL;

                    free_new_slice(newSlice);
                    newSlice = NULL;

                    return CREL_OK;
                }
            }
            else 
            {
                //Frame Picture
                if (IMGPAR slice_number)
                {                    
                    //Cpoy naul to nalu_global
					
                    nalu_global->startcodeprefix_len = nalu->startcodeprefix_len;
                    nalu_global->len                 = nalu->len;
                    nalu_global->max_size            = nalu->max_size;
                    nalu_global->nal_unit_type       = nalu->nal_unit_type;
                    nalu_global->nal_reference_idc   = nalu->nal_reference_idc;
                    nalu_global->forbidden_bit       = nalu->forbidden_bit;
                    nalu_global->pts                 = nalu->pts;
                    memcpy(nalu_global->buf, nalu->buf, nalu->len);

                    nalu_global_available = 1;
                    g_bNextImgForRef = (nalu_global->nal_reference_idc!=0);

                    stream_global->m_iNextPOC = IMGPAR ThisPOC;
                    stream_global->m_iNextViewIndex = IMGPAR currentSlice->viewIndex;

					IMGPAR PreviousFrameNum				= stream_global->PreviousFrameNum[0];
					IMGPAR PreviousFrameNumOffset = stream_global->PreviousFrameNumOffset[0];
					IMGPAR PreviousPOC						= stream_global->PreviousPOC[0];
					IMGPAR ThisPOC								= stream_global->ThisPOC[0]; 
					IMGPAR PrevPicOrderCntLsb			= stream_global->PrevPicOrderCntLsb[0];
					IMGPAR PrevPicOrderCntMsb			= stream_global->PrevPicOrderCntMsb[0];

					next_image_type = IMGPAR type;

                    IMGPAR currentSlice = IMGPAR prevSlice;
                    IMGPAR currentSlice->nextSlice = NULL;

                    free_new_slice(newSlice);
                    newSlice = NULL;

                    return CREL_OK;
                }
                else
                {
                    UpdatePTS ARGS0();

                    ZeroMemory(&(IMGPAR m_CCCode), sizeof(H264_CC));
                    SetH264CCCode ARGS0();

                    IMGPAR currentSlice->header = SOP;
                    IMGPAR currentSlice->m_pic_combine_status = 0;
                    IMGPAR slice_number++;

                    nalu->buf += nalu->len;

					sps =  (IMGPAR currentSlice->bIsBaseView) ? &SeqParSet[PicParSet[IMGPAR currentSlice->pic_parameter_set_id].seq_parameter_set_id] :
						&SeqParSubset[PicParSet[IMGPAR currentSlice->pic_parameter_set_id].seq_parameter_set_id];
                    pps =  &PicParSet[IMGPAR currentSlice->pic_parameter_set_id];
                    activate_sps ARGS2(sps, IMGPAR currentSlice->viewId);
                    activate_pps ARGS2(pps, IMGPAR currentSlice->viewIndex);

					stream_global->PreviousFrameNum[0]				= IMGPAR PreviousFrameNum;
					stream_global->PreviousFrameNumOffset[0]	= IMGPAR PreviousFrameNumOffset;
					stream_global->PreviousPOC[0]						= IMGPAR PreviousPOC;
					stream_global->ThisPOC[0]								= IMGPAR ThisPOC; 
					stream_global->PrevPicOrderCntLsb[0]			= IMGPAR PrevPicOrderCntLsb;
					stream_global->PrevPicOrderCntMsb[0]			= IMGPAR PrevPicOrderCntMsb;

                    IMGPAR currentSlice->m_nDispPicStructure = sps->vui_seq_parameters.picture_structure;

                    g_bReceivedFirst = 1;

                    return CREL_ERROR_H264_UNDEFINED;
                }
            }                                                
        }
        else //Slice
        {
            if (bSeekIDR_backup && img->currentSlice->picture_type == B_SLICE) {
                free_new_slice(newSlice);
                newSlice = NULL;        

                stream_global->m_bSeekIDR = TRUE;

                nalu->buf = IMGPAR ori_nalu_buf;                    
                IMGPAR slice_number = 0;
                break;
            }

            IMGPAR currentSlice->header = *header = SOS;                
            IMGPAR currentSlice->m_pic_combine_status = IMGPAR prevSlice->m_pic_combine_status;
            IMGPAR slice_number++;

            if (IMGPAR prevSlice)
            {
                IMGPAR prevSlice->exit_flag = 0;
                IMGPAR currentSlice->pts = IMGPAR prevSlice->pts;
                IMGPAR currentSlice->dts = IMGPAR prevSlice->dts;
                IMGPAR currentSlice->has_pts = IMGPAR prevSlice->has_pts;
                IMGPAR currentSlice->framerate1000 = IMGPAR prevSlice->framerate1000;
                IMGPAR currentSlice->NumClockTs = IMGPAR prevSlice->NumClockTs;
            }
            nalu->buf += nalu->len;

            IMGPAR currentSlice->m_nDispPicStructure = IMGPAR prevSlice->m_nDispPicStructure;

            return CREL_ERROR_H264_UNDEFINED;
        }

        break;
    case NALU_TYPE_DPA:
        return CREL_ERROR_H264_UNDEFINED;
        break;
    case NALU_TYPE_DPC:            
        return CREL_ERROR_H264_UNDEFINED;
        break;
    case NALU_TYPE_SEI:
        DEBUG_SHOW_SW_INFO("-- NALU_TYPE_SEI --");
        DEBUG_SHOW_SW_INFO_DETAIL("read_new_slice: Found NALU_TYPE_SEI, len %d\n", nalu->len);
        
        ret = InterpretSEIMessage ARGS2(nalu->buf,nalu->len);
        //Terry: For PIP's sub bitstream, it may has no SPS before pasing SEI nalu during enable fast forward (FF) function.
        if (FAILED(ret)) {

            if (ISWARNING(ret)) {

                if (ISWARNINGLEVEL_1(ret)) {
                    stream_global->m_bSeekIDR = TRUE;
                    nalu->buf = IMGPAR ori_nalu_buf;
                    for ( unsigned int i = 0; i < stream_global->num_views; i++) {
                        stream_global->m_active_sps_on_view[i]= 0;
                    }
                }

                break;

            } else {
                return ret;
            }
        }            
        break;
    case NALU_TYPE_PPS:
        DEBUG_SHOW_SW_INFO("-- NALU_TYPE_PPS --");
        ret = ProcessPPS ARGS1(nalu);
        if (FAILED(ret)) {
            if (ISWARNING(ret)) {

                if (ISWARNINGLEVEL_1(ret)) {
                    stream_global->m_bSeekIDR = TRUE;
                    nalu->buf = IMGPAR ori_nalu_buf;
                }

                break;

            } else {
                return ret;
            }
        }                
        break;
    case NALU_TYPE_SPS:
        DEBUG_SHOW_SW_INFO("-- NALU_TYPE_SPS --");

        //If bitstream is IDR only and IDR are all the same, decoder needs to return here.
        if (IMGPAR slice_number > 0 && IMGPAR firstSlice->idr_flag)
        {
            nalu_global->startcodeprefix_len = nalu->startcodeprefix_len;
            nalu_global->len                 = nalu->len;
            nalu_global->max_size            = nalu->max_size;
            nalu_global->nal_unit_type       = nalu->nal_unit_type;
            nalu_global->nal_reference_idc   = nalu->nal_reference_idc;
            nalu_global->forbidden_bit       = nalu->forbidden_bit;
            nalu_global->pts                 = nalu->pts;
            memcpy(nalu_global->buf, nalu->buf, nalu->len);
            nalu_global_available = 1;

            IMGPAR currentSlice->exit_flag = 1;

            return CREL_OK;
        }

        ret = ProcessSPS ARGS1(nalu);
        if (FAILED(ret)) {

            if (ISWARNING(ret)) {

                if (ISWARNINGLEVEL_1(ret)) {
                    stream_global->m_bSeekIDR = TRUE;
                    nalu->buf = IMGPAR ori_nalu_buf;
                }

                break;

            } else {
                return ret;
            }
        }        
        break;
    case NALU_TYPE_PREFIX:
        
        ret = ProcessNaluExt ARGS1(nalu);
        stream_global->nalu_mvc_extension.bIsPrefixNALU = TRUE;

        if (FAILED(ret)) {

            if (ISWARNING(ret)) {

                if (ISWARNINGLEVEL_1(ret)) {
                    stream_global->m_bSeekIDR = TRUE;
                    nalu->buf = IMGPAR ori_nalu_buf;
                }

                break;

            } else {
                return ret;
            }
        }

        

        break;

    case NALU_TYPE_SPS_SUBSET:
        DEBUG_SHOW_SW_INFO("-- NALU_TYPE_SPS_SUBSET --");

        //If bitstream is IDR only and IDR are all the same, decoder needs to return here.
        if (IMGPAR slice_number > 0 && IMGPAR firstSlice->idr_flag)
        {
            nalu_global->startcodeprefix_len = nalu->startcodeprefix_len;
            nalu_global->len                 = nalu->len;
            nalu_global->max_size            = nalu->max_size;
            nalu_global->nal_unit_type       = nalu->nal_unit_type;
            nalu_global->nal_reference_idc   = nalu->nal_reference_idc;
            nalu_global->forbidden_bit       = nalu->forbidden_bit;
            nalu_global->pts                 = nalu->pts;
            memcpy(nalu_global->buf, nalu->buf, nalu->len);
            nalu_global_available = 1;

            IMGPAR currentSlice->exit_flag = 1;

            return CREL_OK;
        }

        ret = ProcessSPSSubset ARGS1(nalu);
        if (FAILED(ret)) {

            if (ISWARNING(ret)) {

                if (ISWARNINGLEVEL_1(ret)) {
                    stream_global->m_bSeekIDR = TRUE;
                    nalu->buf = IMGPAR ori_nalu_buf;
                }

                break;

            } else {
                return ret;
            }
        }
        
        break;
    case NALU_TYPE_AUD:
        DEBUG_SHOW_SW_INFO("-- NALU_TYPE_AUD --");
        ret = ProcessAUD ARGS2(nalu, &primary_pic_type);
        if (FAILED(ret)) {
            return ret;
        }    
        break;
    case NALU_TYPE_EOSEQ:
        DEBUG_SHOW_SW_INFO_DETAIL ("read_new_slice: Found 'End of Sequence' NAL unit, len %d, ignored\n", nalu->len);
        break;
    case NALU_TYPE_EOSTREAM:
        DEBUG_SHOW_SW_INFO_DETAIL ("read_new_slice: Found 'End of Stream' NAL unit, len %d, ignored\n", nalu->len);
        break;
    case NALU_TYPE_FILL:
        DEBUG_SHOW_SW_INFO_DETAIL ("read_new_slice: Found NALU_TYPE_FILL, len %d\n", nalu->len);
        DEBUG_SHOW_SW_INFO_DETAIL ("Skipping these filling bits, proceeding w/ next NALU\n");
        break;
    case NALU_TYPE_DRD:
        break;
    default:
        DEBUG_SHOW_SW_INFO_DETAIL("-- NALU_TYPE_default --");
        DEBUG_SHOW_SW_INFO_DETAIL ("Found NALU type %d, len %d undefined, ignore NALU, moving on\n", nalu->nal_unit_type, nalu->len);
    }

    return CREL_ERROR_H264_UNDEFINED;
}

HRESULT CH264VDec::DoEOSFlush PARGS0()
{
    DEBUG_INFO("[CH264VDec] DoEOSFlush() - Enter EOS Flush Stage!!");
    BOOL bCurrThreadForB = FALSE;
    CREL_RETURN ret;
    unsigned int view_index;

    if (stream_global->m_is_MTMS || stream_global->m_bIsSingleThreadMode)
        img = img_array[0];
    else
    {
        if (next_image_type == B_SLICE)
        {
            if ( get_param_fcn && !g_bNormalSpeed) {    //To guarantee conformance test at console mode        -Haihua
                //20070125-Byby: we need to skip the last B frame while EOS to prevent corruption caused by trick mode.


                for (view_index = 0; view_index < stream_global->num_views; view_index ++) {

                    if(dpb.used_size_on_view[view_index]){    //Only check Base view, might be revised

                        for (unsigned int i=0; i <storable_picture_count; i++)                
                            storable_picture_map[i]->used_for_first_field_reference = 0;                    

                        ret = flush_dpb ARGS1(view_index);
                        if (FAILED(ret)) {
                            return ret;
                        }
                    }
                }
                g_bEOS = FALSE;
                //2006-03-31 KevinChien: Temporal solution for FF/RW
                //We need to change next_image_type modal
                return E_ABORT;
                //~KevinChien
            }

            img = img_array[stream_global->pic_b_num + 2];
            bCurrThreadForB = TRUE;
        }
        else
        {
            img = img_array[stream_global->pic_ip_num];
            bCurrThreadForB = FALSE;
        }

        //output the last frame
        if ((IMGPAR slice_number > 0 && IMGPAR structure == FRAME) || (IMGPAR slice_number > 1 && IMGPAR structure != FRAME))
        {
            if (bCurrThreadForB)
            {
                int nImgB_rd0 = stream_global->pic_b_num + 2;

                gImgB_r0 = img_array[nImgB_rd0];
                SetEvent(stream_global->m_event_read_start_b[0]);
                WaitForSingleObject(stream_global->m_event_decode_finish_b[0], INFINITE);
                ResetEvent(stream_global->m_event_decode_finish_b[0]);
            }
            else
            {
                gImgIP_d = gImgIP_r = img_array[stream_global->pic_ip_num];
                SetEvent(stream_global->m_event_read_start_ip);
                WaitForSingleObject(stream_global->m_event_read_finish_ip, INFINITE);
                ResetEvent(stream_global->m_event_read_finish_ip);

                SetEvent(stream_global->m_event_decode_start_ip);
                SetEvent(stream_global->m_event_for_field_ip);
                WaitForSingleObject(stream_global->m_event_decode_finish_ip, INFINITE);
                ResetEvent(stream_global->m_event_decode_finish_ip);
            }
        }
    }

    for (view_index = 0; view_index < stream_global->num_views; view_index ++) {
        if(dpb.used_size_on_view[view_index])    //Only check Base view, might be revised
        {
            for (unsigned int i=0; i <storable_picture_count; i++)                
                storable_picture_map[i]->used_for_first_field_reference = 0;

            ret = flush_dpb ARGS1(view_index);            
            if (FAILED(ret)) {
                return ret;
            }

            ret = init_dpb ARGS1(view_index);
            if (FAILED(ret)) {
                return ret;
            }
        }
    }

    nalu_global_available = 0;    
    stream_global->number = 0;
    stream_global->pic_ip_num = 0;
    stream_global->pic_b_num = 0;
    stream_global->is_first = 1;
    stream_global->bHasB = 0;
    stream_global->b_Count = 0;
    stream_global->nSwitchB = 0;
    stream_global->two_B_Flag = 0; //two b frames or not
    stream_global->nNeedToDecodeIP = 0;
    stream_global->nCollectB = 0;
    next_image_type = -1;

    g_bReceivedFirst = 0;

    stream_global->m_is_MTMS = -1;

    g_bEOS = FALSE;

    return CREL_OK;
}

HRESULT CH264VDec::DecodeThisFrame(BOOL bEOS, BOOL &bSkip, BOOL bFastSeek)
{    
    DEBUG_INFO("[CH264VDec] DecodeThisFrame() EOS: %d Skip: %d", bEOS, bSkip);

    if (!m_pStream)
        return E_FAIL;

#if !defined(_COLLECT_PIC_)
    ImageParameters *img = (ImageParameters*) m_pStream;
#else
    StreamParameters *pStream = (StreamParameters*) m_pStream;
    ImageParameters *img = pStream->m_img[0];
#endif

    BOOL bIsForReference;
    BOOL bCurrThreadForB;
    int nImgIP_r, nImgIP_d, nImgB_rd0/*, nImgB_rd1*/;
    CREL_RETURN ret;


    int nIMG_num = (stream_global->m_bIsSingleThreadMode == TRUE)? 1: IMG_NUM;
#if !defined(_COLLECT_PIC_)
    IMGPAR stop_indicator=0;
#else
    for (unsigned int i=0; i<nIMG_num; i++)
    {
        img_array[i]->stop_indicator=0;
    }
#endif

    m_dwNextImageType = next_image_type;

    img = m_pActiveImg;

    int view_index = (IMGPAR firstSlice) ? (IMGPAR firstSlice->viewIndex) : 0;

    bIsForReference = IMGPAR firstSlice->nal_reference_idc;

    if (IMGPAR firstSlice->picture_type == B_SLICE && !bIsForReference && (stream_global->m_bSeekIDR == FALSE)) {
        bCurrThreadForB = 1;
    } else {
        bCurrThreadForB = 0;
    }

    if (IMGPAR slice_number == 0)
    {
        DEBUG_SHOW_SW_INFO("[ERROR] Slice Number is 0 ==> return CREL_ERROR_H264_UNDEFINED");
        return CREL_ERROR_H264_UNDEFINED;
    }
    else if (IMGPAR currentSlice && (IMGPAR currentSlice->m_pic_combine_status != FRAME && IMGPAR firstSlice->structure != FRAME))
    {
        DEBUG_SHOW_SW_INFO("[ERROR] Field Coding Frame! But Only receive one Picture!!");

        IMGPAR currentSlice = IMGPAR firstSlice;
        for (int i=0; i< IMGPAR slice_number; i++)
        {
            //active_pps = &PicParSet[IMGPAR currentSlice->pic_parameter_set_id];
            memcpy(&active_pps, &PicParSet[IMGPAR currentSlice->pic_parameter_set_id], sizeof(pic_parameter_set_rbsp_t));
            IMGPAR prevSlice = IMGPAR currentSlice;
            IMGPAR currentSlice = (Slice*) IMGPAR currentSlice->nextSlice;
            free_ref_pic_list_reordering_buffer(IMGPAR prevSlice);
            free_new_slice(IMGPAR prevSlice);
        }
        IMGPAR firstSlice = IMGPAR currentSlice = IMGPAR prevSlice = NULL;
        IMGPAR current_mb_nr_r = IMGPAR current_mb_nr_d = -4712;   // impossible value for debugging, StW
        IMGPAR error_mb_nr = -4712;
        IMGPAR current_slice_nr = 0;
        IMGPAR slice_number = 0;

        stream_global->m_bSeekIDR = TRUE;

        if (stream_global->m_bRefBWaitB) {
            SetEvent(event_RB_1stfield_decode_complete);
            WaitForSingleObject(event_RB_wait_clear, INFINITE);
            ResetEvent(event_RB_wait_clear);
            WaitForSingleObject(stream_global->m_event_read_finish_ip, INFINITE);
            stream_global->m_bRefBWaitB = FALSE;
        }

        return CREL_ERROR_H264_UNDEFINED;
    }

    if (stream_global->m_is_MTMS < 0)
    {
        if ((IMGPAR firstSlice->structure==0 && IMGPAR slice_number >= 4) || (IMGPAR firstSlice->structure!=0 && IMGPAR slice_number >= 8) || stream_global->bMVC) {
            stream_global->m_is_MTMS = 1;
        } else {
            stream_global->m_is_MTMS = 0;
        }
    }

    DEBUG_SHOW_SW_INFO("Collected One Picture\n Slice Number: %d  SmartDecLevel: %d ", IMGPAR slice_number, g_dwSmartDecLevel);

    IMGPAR SkipedBFrames[view_index][0] = IMGPAR SkipedBFrames[view_index][1] = 0;
    IMGPAR SkipThisFrame = FALSE;

    g_llDtsCount[view_index] = ((IMGPAR firstSlice->has_pts) && (IMGPAR firstSlice->picture_type == I_SLICE)) ? 0 : (g_llDtsCount[view_index]+1);
    g_llDecCount[view_index]++;

    get_block_4xh  = get_block_4xh_full;
    get_block_8xh  = get_block_8xh_full;
    get_block_16xh = get_block_16xh_full;

    set_4xH_mc_function_ptr ARGS0();

    mb_chroma_2xH = mb_chroma_2xH_full;
    mb_chroma_4xH = mb_chroma_4xH_full;
    mb_chroma_8xH = mb_chroma_8xH_full;

    set_2xH_mc_function_ptr ARGS0();

    if(stream_global->m_bIsSingleThreadMode && g_DXVAVer == IviNotDxva)
    {
        if(bEOS)
        {
            DEBUG_SHOW_SW_INFO("Received EOS");

            if (IMGPAR slice_number > 0 && IMGPAR currentSlice->m_pic_combine_status==0)
                ret = DecodeFrameSingleThread ARGS0();

            return DoEOSFlush ARGS0();
        }

        //Process Skip Flag and PTS
        ProcessSkipAndPTS ARGS3(bSkip, bIsForReference, bFastSeek);
        if ((IMGPAR firstSlice->picture_type==B_SLICE)  && dpb.used_size_on_view[view_index])
        {
            StorablePicture *pPreFrame = NULL;
            
            
            if ( dpb.fs_on_view[view_index][dpb.used_size_on_view[view_index]-1]->frame ) {    // Field Combined or Frame Split work may not be done if that picture has error.
                pPreFrame = dpb.fs_on_view[view_index][dpb.used_size_on_view[view_index]-1]->frame;
            } else if (dpb.fs_on_view[view_index][dpb.used_size_on_view[view_index]-1]->top_field) {
                pPreFrame = dpb.fs_on_view[view_index][dpb.used_size_on_view[view_index]-1]->top_field;
            } else {
                pPreFrame = dpb.fs_on_view[view_index][dpb.used_size_on_view[view_index]-1]->bottom_field;
            }
            
            if(pPreFrame && (pPreFrame->slice_type==P_SLICE || pPreFrame->slice_type==I_SLICE))
            {
                g_pArrangedPCCCode = &(pPreFrame->m_CCCode);
                g_pArrangedPCCCode->ccbuf[LINE21BUF_SIZE-1] = 0;
            }

            if (g_pArrangedPCCCode)
                RearrangePBCCBuf ARGS0();
        }

        if (IMGPAR firstSlice->picture_type != B_SLICE)
        {
            //stream_global->number++;
            stream_global->bSeekToOpenGOP = 0;
            stream_global->nCollectB = 0;
        }
        else
            stream_global->nCollectB++;

        stream_global->number++;

        stream_global->pre_frame_num    = IMGPAR firstSlice->frame_num;

        if (IMGPAR firstSlice->picture_type == I_SLICE)
        {
            stream_global->m_listXsize[0] = stream_global->m_listXsize[1] = stream_global->m_listXsize[2] = 
                stream_global->m_listXsize[3] = stream_global->m_listXsize[4] = stream_global->m_listXsize[5] = 0;
        }
        else
        {                
            stream_global->m_listXsize[0] = IMGPAR firstSlice->num_ref_idx_l0_active;
            stream_global->m_listXsize[2] = stream_global->m_listXsize[4] = IMGPAR firstSlice->num_ref_idx_l0_active * 2;
            stream_global->m_listXsize[1] = IMGPAR firstSlice->num_ref_idx_l1_active;
            stream_global->m_listXsize[3] = stream_global->m_listXsize[5] = IMGPAR firstSlice->num_ref_idx_l1_active * 2;
        }


        ret = DecodeFrameSingleThread ARGS0();
        if (FAILED(ret)) {    //Only 2 options: Error or Level1 Warning
            img = img_array[0];

            if (ISWARNINGLEVEL_1(ret)) {
                stream_global->m_bSeekIDR = TRUE;        
            } else if (ISERROR(ret)) {
                return ret;
            }
        }

        if (stream_global->is_first)
        {
            stream_global->is_first = 0;
            if (next_image_type == B_SLICE)
                stream_global->bSeekToOpenGOP = 1;
        }

        next_image_type = -1; //reset

    }
    else if (stream_global->m_is_MTMS)
    {
        DEBUG_SHOW_SW_INFO("This is MT MultiSlice");
        //if(g_HDProfileFault)
        //    return E_ABORT;    // IoK: check for HD-profile violation due to incorrect nal_unit_type (2-4), ASO or incorrect slice types (SI/SP)

        if(bEOS)
        {
            DEBUG_SHOW_SW_INFO("Received EOS");

            if (IMGPAR slice_number > 0 && IMGPAR currentSlice->m_pic_combine_status==0)
                ret = DecodeOneFrame ARGS0();

            return DoEOSFlush ARGS0();
        }

        //Process Skip Flag and PTS
        ProcessSkipAndPTS ARGS3(bSkip, bIsForReference, bFastSeek);
        if ((IMGPAR firstSlice->picture_type==B_SLICE)  && dpb.used_size_on_view[view_index])
        {
            StorablePicture *pPreFrame = NULL;
            
            
            if ( dpb.fs_on_view[view_index][dpb.used_size_on_view[view_index]-1]->frame ) {    // Field Combined or Frame Split work may not be done if that picture has error.
                pPreFrame = dpb.fs_on_view[view_index][dpb.used_size_on_view[view_index]-1]->frame;
            } else if (dpb.fs_on_view[view_index][dpb.used_size_on_view[view_index]-1]->top_field) {
                pPreFrame = dpb.fs_on_view[view_index][dpb.used_size_on_view[view_index]-1]->top_field;
            } else {
                pPreFrame = dpb.fs_on_view[view_index][dpb.used_size_on_view[view_index]-1]->bottom_field;
            }
            
            if(pPreFrame && (pPreFrame->slice_type==P_SLICE || pPreFrame->slice_type==I_SLICE))
            {
                g_pArrangedPCCCode = &(pPreFrame->m_CCCode);
                g_pArrangedPCCCode->ccbuf[LINE21BUF_SIZE-1] = 0;
            }

            if (g_pArrangedPCCCode)
                RearrangePBCCBuf ARGS0();
        }

        if (IMGPAR firstSlice->picture_type != B_SLICE)
        {
            //stream_global->number++;
            stream_global->bSeekToOpenGOP = 0;
            stream_global->nCollectB = 0;
        }
        else
            stream_global->nCollectB++;

        stream_global->number++;

        stream_global->pre_frame_num    = IMGPAR firstSlice->frame_num;

        if (IMGPAR firstSlice->picture_type == I_SLICE)
        {
            stream_global->m_listXsize[0] = stream_global->m_listXsize[1] = stream_global->m_listXsize[2] = 
                stream_global->m_listXsize[3] = stream_global->m_listXsize[4] = stream_global->m_listXsize[5] = 0;
        }
        else
        {                
            stream_global->m_listXsize[0] = IMGPAR firstSlice->num_ref_idx_l0_active;
            stream_global->m_listXsize[2] = stream_global->m_listXsize[4] = IMGPAR firstSlice->num_ref_idx_l0_active * 2;
            stream_global->m_listXsize[1] = IMGPAR firstSlice->num_ref_idx_l1_active;
            stream_global->m_listXsize[3] = stream_global->m_listXsize[5] = IMGPAR firstSlice->num_ref_idx_l1_active * 2;
        }


        ret = DecodeOneFrame ARGS0();
        if (FAILED(ret)) {    //Only 2 options: Error or Level1 Warning
            img = img_array[0];

            if (ISWARNINGLEVEL_1(ret)) {
                stream_global->m_bSeekIDR = TRUE;        
            } else if (ISERROR(ret)) {
                return ret;
            }
        }

        if (stream_global->is_first && (!stream_global->bMVC || (view_index == stream_global->num_views - 1)))
        {
            stream_global->is_first = 0;
            if (next_image_type == B_SLICE)
                stream_global->bSeekToOpenGOP = 1;
        }

        next_image_type = -1; //reset

    }
    else
    {
        DEBUG_SHOW_SW_INFO("This is MT SingleSlice");
        //if(g_HDProfileFault)
        //    return E_ABORT;    // IoK: check for HD-profile violation due to incorrect nal_unit_type (2-4), ASO or incorrect slice types (SI/SP)

        if(bEOS)
        {
            DEBUG_SHOW_SW_INFO("Received EOS");

            if (stream_global->nNeedToDecodeIP || stream_global->b_Count)
                FlushAllFrames ARGS0();

            return DoEOSFlush ARGS0();
        }

        //Process Skip Flag and PTS
        ProcessSkipAndPTS ARGS3(bSkip, bIsForReference, bFastSeek);
        if (IMGPAR firstSlice->picture_type==B_SLICE)
        {
            ImageParameters *pPreImg =  img_array[!(stream_global->pic_ip_num)];
            g_pArrangedPCCCode = &(pPreImg->m_CCCode);

            RearrangePBCCBuf ARGS0();
        }
        else
            IMGPAR m_CCCode.ccbuf[LINE21BUF_SIZE-1] = 0;

        DEBUG_SHOW_SW_INFO("Start Send Events");
        if (bCurrThreadForB)
        {
            DEBUG_SHOW_SW_INFO("This Picture is for B");

            if (IMGPAR smart_dec & SMART_DEC_INT_PEL_Y)
            {
                get_block_4xh  = get_block_4xh_int;
                get_block_8xh  = get_block_8xh_int;
                get_block_16xh = get_block_16xh_int;

                set_4xH_mc_function_ptr ARGS0();
            }

            if (IMGPAR smart_dec & SMART_DEC_INT_PEL_UV)
            {
                mb_chroma_2xH = mb_chroma_2xH_int;
                mb_chroma_4xH = mb_chroma_4xH_int;
                mb_chroma_8xH = mb_chroma_8xH_int;

                set_2xH_mc_function_ptr ARGS0();
            }

            stream_global->nCollectB++;
            stream_global->b_Count++;

            stream_global->pre_frame_num    = IMGPAR firstSlice->frame_num;

            stream_global->m_listXsize[0] = IMGPAR firstSlice->num_ref_idx_l0_active;
            stream_global->m_listXsize[2] = stream_global->m_listXsize[4] = IMGPAR firstSlice->num_ref_idx_l0_active * 2;
            stream_global->m_listXsize[1] = IMGPAR firstSlice->num_ref_idx_l1_active;
            stream_global->m_listXsize[3] = stream_global->m_listXsize[5] = IMGPAR firstSlice->num_ref_idx_l1_active * 2;

			seq_parameter_set_rbsp_t *sps =  (IMGPAR firstSlice->bIsBaseView) ? &SeqParSet[PicParSet[IMGPAR firstSlice->pic_parameter_set_id].seq_parameter_set_id] :
				&SeqParSubset[PicParSet[IMGPAR firstSlice->pic_parameter_set_id].seq_parameter_set_id];
            IMGPAR MaxFrameNum = 1 << (sps->log2_max_frame_num_minus4+4);

            if (stream_global->bHasB)
            {
                nImgB_rd0 = (stream_global->nSwitchB<<1) + ((stream_global->b_Count&1) ? 2:3);

                if ((stream_global->b_Count&1) || ((stream_global->b_Count&1) == 0 && stream_global->two_B_Flag))
                {
                    if (nImgB_rd0&1)
                    {
                        gImgB_r1 = img_array[nImgB_rd0];
                        SetEvent(stream_global->m_event_read_start_b[1]);
                    }
                    else
                    {
                        gImgB_r0 = img_array[nImgB_rd0];
                        SetEvent(stream_global->m_event_read_start_b[0]);
                    }
                }
            }

            stream_global->pic_b_num++;
            if (stream_global->pic_b_num == 4)
                stream_global->pic_b_num = 0;

            stream_global->number++;
        }
        else
        {
            DEBUG_SHOW_SW_INFO("This Picture is for IP");

            if (stream_global->m_bDoSkipPB == TRUE && IMGPAR firstSlice->picture_type == I_SLICE)
                g_dwSmartDecLevel = stream_global->m_dwSavedSmartDecLevel;

            stream_global->nCollectB = 0;

            stream_global->number++;
            stream_global->pre_frame_num    = IMGPAR firstSlice->frame_num;

            if (IMGPAR firstSlice->picture_type == I_SLICE)
            {
                stream_global->m_listXsize[0] = stream_global->m_listXsize[1] = stream_global->m_listXsize[2] = 
                    stream_global->m_listXsize[3] = stream_global->m_listXsize[4] = stream_global->m_listXsize[5] = 0;
            }
            else
            {            
                stream_global->m_listXsize[0] = IMGPAR firstSlice->num_ref_idx_l0_active;
                stream_global->m_listXsize[2] = stream_global->m_listXsize[4] = IMGPAR firstSlice->num_ref_idx_l0_active * 2;
                stream_global->m_listXsize[1] = IMGPAR firstSlice->num_ref_idx_l1_active;
                stream_global->m_listXsize[3] = stream_global->m_listXsize[5] = IMGPAR firstSlice->num_ref_idx_l1_active * 2;
            }

            nImgIP_r = stream_global->pic_ip_num;
            nImgIP_d = !(nImgIP_r);

            if (!stream_global->is_first)
            {

                if(stream_global->bSeekToOpenGOP && IMGPAR firstSlice->picture_type != B_SLICE)  //We should not set bSeekToOpenGOP to 0 in reference-B
                    stream_global->bSeekToOpenGOP = 0;

                if (stream_global->m_bDoSkipPB == TRUE && IMGPAR firstSlice->picture_type == I_SLICE)  //Skip all B frames between I and P when m_bDoSkipPB == TRUE
                    stream_global->bSeekToOpenGOP = 1;                                                   //Same behavior as seek to open GOP

                // P decode
                if (stream_global->nNeedToDecodeIP)
                {
                    gImgIP_d = img_array[nImgIP_d];
                    SetEvent(stream_global->m_event_decode_start_ip);
                }

                // P read
                int fill_frame_gap_flag = 0;
				seq_parameter_set_rbsp_t *sps =  (IMGPAR firstSlice->bIsBaseView) ? &SeqParSet[PicParSet[IMGPAR firstSlice->pic_parameter_set_id].seq_parameter_set_id] :
					&SeqParSubset[PicParSet[IMGPAR firstSlice->pic_parameter_set_id].seq_parameter_set_id];
                IMGPAR MaxFrameNum = 1 << (sps->log2_max_frame_num_minus4+4);
                if (IMGPAR firstSlice->frame_num != IMGPAR firstSlice->pre_frame_num && IMGPAR firstSlice->frame_num != (IMGPAR firstSlice->pre_frame_num + 1) % IMGPAR MaxFrameNum) 
                {
                    /* int nSeekFlag = g_framemgr->GetDisplayCount() + dpb.used_size;    //Covered by latest ER
                    // Terry: For seeking case (nSeekFlag==0), the first frame should be I frame.
                        if((nSeekFlag == 0) && (IMGPAR firstSlice->picture_type != I_SLICE)) {

                            if (stream_global->nNeedToDecodeIP) {
                                SetEvent(stream_global->m_event_for_field_ip);
                                WaitForSingleObject(stream_global->m_event_decode_finish_ip, INFINITE);    
                                ResetEvent(stream_global->m_event_decode_finish_ip);
                            }
                            stream_global->nNeedToDecodeIP = 0;

                            IMGPAR currentSlice = IMGPAR firstSlice;
                            for (int j=0; j< IMGPAR slice_number; j++)
                            {
                                //active_pps = &PicParSet[IMGPAR currentSlice->pic_parameter_set_id];
                                memcpy(&active_pps, &PicParSet[IMGPAR currentSlice->pic_parameter_set_id], sizeof(pic_parameter_set_rbsp_t));
                                IMGPAR prevSlice = IMGPAR currentSlice;
                                IMGPAR currentSlice = (Slice*) IMGPAR currentSlice->nextSlice;
                                free_ref_pic_list_reordering_buffer(IMGPAR prevSlice);
                                free_new_slice(IMGPAR prevSlice);                    

                            }

                            IMGPAR firstSlice = IMGPAR currentSlice = IMGPAR prevSlice = NULL;
                            IMGPAR current_mb_nr_r = IMGPAR current_mb_nr_d = -4712;   // impossible value for debugging, StW
                            IMGPAR error_mb_nr = -4712;
                            IMGPAR current_slice_nr = 0;
                            IMGPAR slice_number = 0;
                            img->m_active_pps[0].Valid = NULL;
                            img->m_active_pps[1].Valid = NULL;
                            //dec_picture = NULL;

                            if (IMGPAR structure != FRAME)
                            {
                                IMGPAR mb_decdata = IMGPAR mb_decdata_ori;
                                IMGPAR cof_array = IMGPAR cof_array_ori;
                            }

                            return CREL_ERROR_H264_UNDEFINED;
                        }
                    // Terry: For seeking case (nSeekFlag==0), we should skip this function.
                    */
                    //if( (IMGPAR smart_dec <= SMART_DEC_LEVEL_3 || IMGPAR smart_dec == SMART_DEC_LEVEL_0) && nSeekFlag && IMGPAR dwFillFrameNumGap)
                    if( (IMGPAR smart_dec <= SMART_DEC_LEVEL_3 || IMGPAR smart_dec == SMART_DEC_LEVEL_0) && IMGPAR dwFillFrameNumGap)
                            fill_frame_gap_flag= 1;
                }
                if (!fill_frame_gap_flag)     //Non fill_frame_num_gap
                {
                    gImgIP_r = img_array[nImgIP_r];
                    SetEvent(stream_global->m_event_read_start_ip);    
                }

                //B read & decode
                if (stream_global->bHasB)
                    DecodeProcessBB ARGS0();

                //P decode finish
                if (stream_global->nNeedToDecodeIP)
                {
                    SetEvent(stream_global->m_event_for_field_ip);
                    WaitForSingleObject(stream_global->m_event_decode_finish_ip, INFINITE);
                    stream_global->nNeedToDecodeIP = 0;
                }

                // P read finish
                if (!fill_frame_gap_flag)      //Non fill_frame_num_gap
                {
                    if ( stream_global->m_bRefBWaitB && stream_global->m_bSeekIDR ) {
                        SetEvent(event_RB_1stfield_decode_complete);
                        WaitForSingleObject(event_RB_wait_clear, INFINITE);
                        ResetEvent(event_RB_wait_clear);
                        WaitForSingleObject(stream_global->m_event_read_finish_ip, INFINITE);
                        stream_global->m_bRefBWaitB = FALSE;

                    }
                    
                    WaitForSingleObject(stream_global->m_event_read_finish_ip, INFINITE);
                    ResetEvent(stream_global->m_event_read_finish_ip);
                    ResetEvent(stream_global->m_event_decode_finish_ip);
                }
                else
                {
                    //fill_frame_num_gap
                    ResetEvent(stream_global->m_event_decode_finish_ip);    //Moved from

                    gImgIP_r = img_array[nImgIP_r];
                    SetEvent(stream_global->m_event_read_start_ip);
                    if ( stream_global->m_bRefBWaitB && stream_global->m_bSeekIDR ) {
                        SetEvent(event_RB_1stfield_decode_complete);
                        WaitForSingleObject(event_RB_wait_clear, INFINITE);
                        ResetEvent(event_RB_wait_clear);
                        stream_global->m_bRefBWaitB = FALSE;
                    }

                    WaitForSingleObject(stream_global->m_event_read_finish_ip, INFINITE);

                    //ResetEvent(stream_global->m_event_decode_finish_ip);    //Moved to ?
                    
                    ResetEvent(stream_global->m_event_read_finish_ip);        

					//store these parameters to next collect_pic
					stream_global->PreviousFrameNum[0]		= IMGPAR PreviousFrameNum;
					stream_global->PreviousFrameNumOffset[0]	= IMGPAR PreviousFrameNumOffset;
					stream_global->PreviousPOC[0]						= IMGPAR PreviousPOC;
					stream_global->ThisPOC[0]								= IMGPAR ThisPOC; 
					stream_global->PrevPicOrderCntLsb[0]			= IMGPAR PrevPicOrderCntLsb;
					stream_global->PrevPicOrderCntMsb[0]			= IMGPAR PrevPicOrderCntMsb;
				}

                stream_global->pic_ip_num = nImgIP_d;
                stream_global->nNeedToDecodeIP = 1;

                //We should not set m_bDoSkipPB in reference-B
                if (img_array[nImgIP_r]->SkipThisFrame && img_array[nImgIP_r]->firstSlice->picture_type != B_SLICE && stream_global->m_bDoSkipPB == FALSE)
                {
                    stream_global->m_dwSavedSmartDecLevel = g_dwSmartDecLevel;
                    g_dwSmartDecLevel = SMART_DEC_SKIP_PB;
                    stream_global->m_bDoSkipPB = TRUE;
                }
                else if (g_dwSmartDecLevel != SMART_DEC_SKIP_PB)
                    stream_global->m_bDoSkipPB = FALSE;

                if (stream_global->b_Count & 1)
                {
                    stream_global->two_B_Flag = 0;
                    stream_global->b_Count++;
                    stream_global->pic_b_num++;
                    if (stream_global->pic_b_num == 4)
                        stream_global->pic_b_num = 0;
                }
                else
                    stream_global->two_B_Flag = 1;

                if (stream_global->b_Count >= 2)
                    stream_global->bHasB = 1;
                else
                    stream_global->bHasB = 0;

                if (g_bSkipFirstB < 0 && stream_global->b_Count) //encounter whether it is a GOP with one B
                {
                    g_bSkipFirstB+=2; //add 2 on each, ex: (-3) -> (-1) -> 1
                    if (stream_global->two_B_Flag)
                        g_bSkipFirstB = FALSE; //0
                }

#if defined(_SHOW_THREAD_TIME_)
                DEBUG_INFO("PR:%I64d, PD:%I64d, B1R:%I64d, B1D:%I64d, B2R:%I64d, B2D:%I64d",
                    (1000 * (t_ip[1].QuadPart - t_ip[0].QuadPart) / freq.QuadPart),
                    (1000 * (t_ip[3].QuadPart - t_ip[2].QuadPart) / freq.QuadPart),
                    (1000 * (t_b[1].QuadPart - t_b[0].QuadPart) / freq.QuadPart),
                    (1000 * (t_b[5].QuadPart - t_b[4].QuadPart) / freq.QuadPart),
                    (1000 * (t_b[3].QuadPart - t_b[2].QuadPart) / freq.QuadPart),
                    (1000 * (t_b[7].QuadPart - t_b[6].QuadPart) / freq.QuadPart)
                    );
#endif


                if ( stream_global->m_bMultiThreadModeSwitch == 1 ) {
                    // Complete remaining picture decoding, then jump to multi-slice model in next call

                    if (stream_global->nNeedToDecodeIP)    {
                        gImgIP_d = img_array[nImgIP_r];
                        SetEvent(stream_global->m_event_decode_start_ip);
                    }

                    //B read & decode
                    if (stream_global->bHasB)
                        DecodeProcessBB ARGS0();

                    //P decode finish
                    if (stream_global->nNeedToDecodeIP)
                    {
                        SetEvent(stream_global->m_event_for_field_ip);
                        WaitForSingleObject(stream_global->m_event_decode_finish_ip, INFINITE);
                        stream_global->nNeedToDecodeIP = 0;
                    }

                    stream_global->m_is_MTMS = 1;
                    stream_global->m_bMultiThreadModeSwitch = 0;
                }

            } else {
                //KevinChien: 060722 To fix Fujitsu next chapter
                if(next_image_type == B_SLICE)
                    stream_global->bSeekToOpenGOP = 1;
                //End of 060722

                gImgIP_r = img_array[0];
                SetEvent(stream_global->m_event_read_start_ip);
                WaitForSingleObject(stream_global->m_event_read_finish_ip, INFINITE);
                ResetEvent(stream_global->m_event_read_finish_ip);

                if (img_array[0]->SkipThisFrame && stream_global->m_bDoSkipPB == FALSE)
                {
                    stream_global->m_dwSavedSmartDecLevel = g_dwSmartDecLevel;
                    g_dwSmartDecLevel = SMART_DEC_SKIP_PB;
                    stream_global->m_bDoSkipPB = TRUE;
                }
                else if (g_dwSmartDecLevel != SMART_DEC_SKIP_PB)
                    stream_global->m_bDoSkipPB = FALSE;

                stream_global->is_first = 0;
                stream_global->pic_ip_num++;
                if (stream_global->pic_ip_num == 2)
                    stream_global->pic_ip_num = 0;
                stream_global->nNeedToDecodeIP = 1;
            }
        }
    }

    return CREL_OK;
}

HRESULT CH264VDec::OpenH264VDec()
{
    DEBUG_INFO("[CH264VDec] OpenH264VDec()");
#ifdef TR_ENABLE_NEWMACROS
    DWORD dwRet = 0;
    bool bDaemon = false;
    iviTR_FIND_INT_3H_UNHANDLEDEXCEPTIONFILTER(dwRet);
    iviTR_DAEMON_FINDER(bDaemon);
    if(dwRet==4||bDaemon)
        iviTR_EXIT_PROCESS();
#endif

    m_pDecImpl = new CH264VDecHP;
    m_pDecImpl->Open(m_pOpenOptions, sizeof(H264VDecHP_OpenOptionsEx), &m_pStream);
#if defined(TR_ENABLE_NEWMACROS) && defined(TRSDK_VER) && TRSDK_VER >= 1000 && TRSDK_VER < 2000
    bool bDebugBreak = false, bRegCheck = true;
    iviTR_DEBUGBREAK_CHECK(bDebugBreak);
    iviTR_SOFTICE_ICEEXT_REGISTRY_CHECK_INV(bRegCheck);
    if(bDebugBreak||!bRegCheck)
        iviTR_EXIT_PROCESS();
#endif
    stream_global = (StreamParameters*)m_pStream;

	stream_global->m_pListOffsetMetadata = &m_ListOffsetMetadata;

	if(m_bIsAppConstrantsBD3D == TRUE)
		stream_global->m_bIsBD3D = TRUE;

    img_par *img = img_array[0];
    if (g_pH264DXVA)
    {
        ((CH264DXVABase*)g_pH264DXVA)->SetIHVDService(reinterpret_cast<IHVDService*>(m_pIHVDService), m_pOpenOptions);
        ((CH264DXVABase*)g_pH264DXVA)->Open ARGS1(m_pOpenOptions->dwBuffers);
    }
#if defined(TR_ENABLE_NEWMACROS) && defined(TRSDK_VER) && TRSDK_VER >= 1000 && TRSDK_VER < 2000
    bool bSystemCheck = false, bRegCheckEngine = false;
    iviTR_SYSTEM_DEBUG_REGISTRY_CHECK(bSystemCheck);
    iviTR_VERIFY_SYSER_SYSERBOOT(bRegCheckEngine);
    if(bSystemCheck||bRegCheckEngine)
        iviTR_EXIT_PROCESS();
#endif
    m_pActiveImg = img;
    m_dwNextImageType = I_SLICE;

    return S_OK;
}

HRESULT CH264VDec::CloseH264VDec()
{
    DEBUG_INFO("[CH264VDec] CloseH264VDec()");

    if (!m_pStream)
        return E_POINTER;

#if !defined(_COLLECT_PIC_)
    ImageParameters *img = (ImageParameters*) m_pStream;
#else
    StreamParameters *pStream = (StreamParameters*) m_pStream;
    ImageParameters *img = pStream->m_img[0];
#endif

    m_pDecImpl->Close ARGS0();
    m_pDecImpl->Release();

    m_pStream = NULL;

    return S_OK;
}

HRESULT CH264VDec::StopH264VDec(DWORD dwStopFlag)
{
    DEBUG_INFO("[CH264VDec] StopH264VDec()");

    if (!m_pStream)
        return E_POINTER;

#if !defined(_COLLECT_PIC_)
    ImageParameters *img = (ImageParameters*) m_pStream;
#else
    StreamParameters *pStream = (StreamParameters*) m_pStream;
    ImageParameters *img = pStream->m_img[0];
#endif

    m_dwNextImageType = I_SLICE;

	m_bIsAppConstrantsBD3D = FALSE;

	while(m_ListOffsetMetadata.empty() != TRUE)
	{
		H264_Offset_Metadata_Control Metadata_Control = m_ListOffsetMetadata.front();
		if(Metadata_Control.meta_data.Plane_offset_direction)
			free(Metadata_Control.meta_data.Plane_offset_direction);
		if(Metadata_Control.meta_data.Plane_offset_value)
			free(Metadata_Control.meta_data.Plane_offset_value);
		m_ListOffsetMetadata.pop_front();
	}

	m_iMVCFieldEOSActiveIndex = 0;

	m_bMVCFieldEOS = FALSE;

    return m_pDecImpl->Stop ARGS1(dwStopFlag);
}

HRESULT CH264VDec::SetAppConstrants(void *pValue)
{
	HRESULT hr = S_OK;
	switch (*((DWORD*)pValue))
	{
	case H264_MVC_NO_CONSTRAINTS:
		m_bIsAppConstrantsBD3D = FALSE;
		break;
	case H264_BD:
		m_bIsAppConstrantsBD3D = FALSE;
		break;
	case MVC_3DBD:    
		m_bIsAppConstrantsBD3D = TRUE;
		break;
	default: 
		hr = E_FAIL;    
		break;
	}
	
	return hr;
}

HRESULT CH264VDec::SetH264VDec(DWORD dwPropID, void *pValue)
{
    DEBUG_INFO("[CH264VDec] SetH264VDec()");

    if (!m_pStream)
        return E_POINTER;

    return m_pDecImpl->Set(dwPropID, pValue, sizeof(DWORD));
}

HRESULT CH264VDec::GetH264VFrame(H264VDecHP_Frame *pH264VFrame, unsigned int view_index)
{
    DEBUG_INFO("[CH264VDec] GetH264VFrame()");

    if (!m_pStream)
        return E_POINTER;

#if !defined(_COLLECT_PIC_)
    ImageParameters *img = (ImageParameters*) m_pStream;
#else
    StreamParameters *pStream = (StreamParameters*) m_pStream;
    ImageParameters *img = pStream->m_img[0];
#endif

    if(m_pDecImpl->GetPicture ARGS2(pH264VFrame, view_index) == -1)
        return E_FAIL;

    if (g_framemgr->m_nFlushDPBsize)
        g_framemgr->m_nFlushDPBsize--;

    return S_OK;
}

HRESULT CH264VDec::GetH264VFrameEx(H264VDecHP_FrameEx *pH264VFrameEx, unsigned int view_index)
{
    DEBUG_INFO("[CH264VDec] GetH264VFrameEx()");

    if (!m_pStream)
        return E_POINTER;

#if !defined(_COLLECT_PIC_)
    ImageParameters *img = (ImageParameters*) m_pStream;
#else
    StreamParameters *pStream = (StreamParameters*) m_pStream;
    ImageParameters *img = pStream->m_img[0];
#endif

    H264VDecHP_Frame H264VFrame;
    memset(&H264VFrame, 0, sizeof(H264VDecHP_Frame));
    if(m_pDecImpl->GetPicture ARGS2(&H264VFrame, view_index) == -1)
        return E_FAIL;

    //Divide width/height into decoded width/height and display width/height
    int SubWidthC  [2]= { 1, 2 }; // only YUV400 and YUV420
    int SubHeightC [2]= { 1, 2 };
    int crop_left, crop_right, crop_top, crop_bottom;
    StorablePicture *p = static_cast<StorablePicture *>((void*)H264VFrame.dwCookie);

    //Luma
    if(p->frame_cropping_flag)
    {
        crop_left   = SubWidthC[p->chroma_format_idc] * p->frame_cropping_rect_left_offset;
        crop_right  = SubWidthC[p->chroma_format_idc] * p->frame_cropping_rect_right_offset;
        crop_top    = SubHeightC[p->chroma_format_idc]*( 2 - p->frame_mbs_only_flag ) * p->frame_cropping_rect_top_offset;
        crop_bottom = SubHeightC[p->chroma_format_idc]*( 2 - p->frame_mbs_only_flag ) * p->frame_cropping_rect_bottom_offset;
    }
    else
    {
        crop_left = crop_right = crop_top = crop_bottom = 0;
    }
    pH264VFrameEx->dwDisplayWidth = H264VFrame.adwWidth[0];
    pH264VFrameEx->dwDisplayHeight = H264VFrame.adwHeight[0];
    H264VFrame.adwWidth[0] += (crop_left+crop_right);
    H264VFrame.adwHeight[0] += (crop_top+crop_bottom);

    //chroma
    crop_left   = p->frame_cropping_rect_left_offset;
    crop_right  = p->frame_cropping_rect_right_offset;
    crop_top    = ( 2 - p->frame_mbs_only_flag ) * p->frame_cropping_rect_top_offset;
    crop_bottom = ( 2 - p->frame_mbs_only_flag ) * p->frame_cropping_rect_bottom_offset;
    H264VFrame.adwWidth[1] += (crop_left+crop_right);
    H264VFrame.adwHeight[1] += (crop_top+crop_bottom);
    H264VFrame.adwWidth[2] += (crop_left+crop_right);
    H264VFrame.adwHeight[2] += (crop_top+crop_bottom);

    //convert H264VFrame to H264VFrameEx
    pH264VFrameEx->dwCookie = H264VFrame.dwCookie;
    pH264VFrameEx->dwErrorStatus = H264VFrame.dwCookie;
    pH264VFrameEx->dwPictureOrderCounter = H264VFrame.dwPictureOrderCounter;
    memcpy(&(pH264VFrameEx->adwWidth[0]), &(H264VFrame.adwWidth[0]), 3*sizeof(DWORD));
    memcpy(&(pH264VFrameEx->adwHeight[0]), &(H264VFrame.adwHeight[0]), 3*sizeof(DWORD));
    memcpy(&(pH264VFrameEx->adwStride[0]), &(H264VFrame.adwStride[0]), 3*sizeof(DWORD));
    memcpy(&(pH264VFrameEx->adwLeft[0]), &(H264VFrame.adwLeft[0]), 3*sizeof(DWORD));
    memcpy(&(pH264VFrameEx->adwTop[0]), &(H264VFrame.adwTop[0]), 3*sizeof(DWORD));
    pH264VFrameEx->dwXAspect = H264VFrame.dwXAspect;
    pH264VFrameEx->dwYAspect = H264VFrame.dwYAspect;

    memcpy(&(pH264VFrameEx->apbFrame[0]), &(H264VFrame.apbFrame[0]), 3*sizeof(PBYTE));
    pH264VFrameEx->dwBitRate = H264VFrame.dwBitRate;

    pH264VFrameEx->pts = H264VFrame.pts;

    pH264VFrameEx->bRgbOutput = H264VFrame.bRgbOutput;
    pH264VFrameEx->bRepeatFirstField = H264VFrame.bRepeatFirstField;
    pH264VFrameEx->bTopFieldFirst = H264VFrame.top_field_first;
    pH264VFrameEx->bProgressiveFrame = H264VFrame.progressive_frame;
    pH264VFrameEx->sChromaFormatIdc = H264VFrame.chroma_format_idc;
    pH264VFrameEx->sSliceType = H264VFrame.slice_type;
    pH264VFrameEx->uiFrameNum = H264VFrame.frame_num;
    pH264VFrameEx->uiPicNum = H264VFrame.pic_num;

    pH264VFrameEx->uiUnCompFrameIndex = H264VFrame.frame_index;
    pH264VFrameEx->uiDPBBufferSize = H264VFrame.dpb_buffer_size;
    pH264VFrameEx->uiSymbolSizeInBytes = H264VFrame.symbol_size_in_bytes;
    pH264VFrameEx->dwFrameRate1000 = H264VFrame.framerate1000;

    memcpy(&(pH264VFrameEx->pbCC1Buf[0]), &(H264VFrame.m_cc1buf[0]), LINE21BUF_SIZE*sizeof(BYTE));
    pH264VFrameEx->dwCC1Len = H264VFrame.m_cc1len;
    memcpy(&(pH264VFrameEx->pbYCCBuffer[0]), &(H264VFrame.pbYCCBuffer[0]), 12*sizeof(BYTE));
    pH264VFrameEx->dwYCCBufferLen = H264VFrame.dwYCCBufferLen;

    pH264VFrameEx->mb_info = H264VFrame.mb_info;

    if (g_framemgr->m_nFlushDPBsize)
        g_framemgr->m_nFlushDPBsize--;

    pH264VFrameEx->uiViewId = view_index;

    return S_OK;
}

HRESULT CH264VDec::GetOffsetMetadata(H264VDecHP_Offset_Metadata *poffset_metadata, unsigned __int64 pts, DWORD dwCookie)
{
	HRESULT hr = E_FAIL;

	EnterCriticalSection(&stream_global->m_csOffsetMetadata);
	int i = 0;

	int size = m_ListOffsetMetadata.size();

	std::list<H264_Offset_Metadata_Control>::iterator metadataIterator;

	if(size > 0)
		metadataIterator = m_ListOffsetMetadata.begin();
	else
	{
		LeaveCriticalSection(&stream_global->m_csOffsetMetadata);
		return hr;
	}

	for(i = 0; i < size; i++)
	{
		if(metadataIterator->dwCookie != 0)
		{
			metadataIterator++;
			continue;
		}
		else
			break;
	}

	for(; i != size && metadataIterator != m_ListOffsetMetadata.end();)
	{
		__int64 i64PTSdiff;

		if(pts > metadataIterator->meta_data.pts)
			i64PTSdiff = pts - metadataIterator->meta_data.pts;
		else
			i64PTSdiff = metadataIterator->meta_data.pts - pts;

		int iFirst32bitsPTSdiff = i64PTSdiff & 0x7FFFFFFF00000000;
		int iLast32bitsPTSdiff = i64PTSdiff & 0x00000000FFFFFFFF;
		if( iFirst32bitsPTSdiff == 0 && iLast32bitsPTSdiff /100 == 0) //PTS accuracy
		{
			DEBUG_INFO("[H264INFO][Offset_MetaData] Output pts: %I64d", metadataIterator->meta_data.pts);

			poffset_metadata->number_of_offset_sequences = metadataIterator->meta_data.number_of_offset_sequences;

			poffset_metadata->Plane_offset_direction = metadataIterator->meta_data.Plane_offset_direction;

			poffset_metadata->Plane_offset_value = metadataIterator->meta_data.Plane_offset_value;

			poffset_metadata->pts = metadataIterator->meta_data.pts;

			metadataIterator->dwCookie = dwCookie;

			hr = S_OK;

			break;
		}
		else if(pts > metadataIterator->meta_data.pts)
		{
			DEBUG_INFO("[H264INFO][Offset_MetaData] Drop pts: %I64d and the original frame pts: %I64d", metadataIterator->meta_data.pts, pts);

			std::list<H264_Offset_Metadata_Control>::iterator releaseIterator = metadataIterator;
			metadataIterator++;

			if(releaseIterator->meta_data.Plane_offset_direction)
				free(releaseIterator->meta_data.Plane_offset_direction);
			if(releaseIterator->meta_data.Plane_offset_value)
				free(releaseIterator->meta_data.Plane_offset_value);

			m_ListOffsetMetadata.erase(releaseIterator);


		}
		else
		{
			DEBUG_INFO("[H264INFO][Offset_MetaData] metadata pts: %I64d > the original frame pts: %I64d", metadataIterator->meta_data.pts, pts);
			break;
		}

	}
	LeaveCriticalSection(&stream_global->m_csOffsetMetadata);
	return hr;
}



void CH264VDec::Release_Metadata(DWORD dwCookie)
{
	EnterCriticalSection(&stream_global->m_csOffsetMetadata);
	m_ListOffsetMetadata.remove_if(Remove_OffsetMetadata(dwCookie));
	LeaveCriticalSection(&stream_global->m_csOffsetMetadata);
}

HRESULT CH264VDec::ReleaseH264VFrame(DWORD dwCookie)
{
    DEBUG_INFO("[CH264VDec] ReleaseH264VFrame()");

    if (!m_pStream)
        return E_FAIL;

#if !defined(_COLLECT_PIC_)
    ImageParameters *img = (ImageParameters*) m_pStream;
#else
    StreamParameters *pStream = (StreamParameters*) m_pStream;
    ImageParameters *img = pStream->m_img[0];
#endif

	if(dwCookie !=0 )
	{
		int view_index = reinterpret_cast<CH264VideoFrame *>(dwCookie)->view_index;
		if(view_index > 0)
			Release_Metadata(dwCookie);
	}
	

    return m_pDecImpl->ReleaseFrame ARGS1(dwCookie);
}

HRESULT CH264VDec::ResetDataBuffer(DWORD *pValue)
{
    DEBUG_INFO("[CH264VDec] ResetDataBuffer()");

    if (!m_pStream)
        return E_FAIL;

    *pValue = GetRemainBufferLength();

    return m_pDecImpl->ReleaseBuffer(0, NULL);
}

DWORD CH264VDec::GetDisplayCount()
{
    return (DWORD)g_framemgr->GetDisplayCount();
}

DWORD CH264VDec::GetDisplayFrame(int view_index)
{
    return (DWORD)g_framemgr->GetDisplayFrame(view_index);
}

DWORD CH264VDec::GetCurrentImgType()
{
    DEBUG_INFO("[CH264VDec] GetCurrentImgType()");

    if (!m_pActiveImg)
        return E_FAIL;

    ImageParameters *img = m_pActiveImg;

    return (DWORD)IMGPAR firstSlice->AU_type;
}

DWORD CH264VDec::GetNextImgType()
{
    DEBUG_INFO("[CH264VDec] GetNextImgType() %d", m_dwNextImageType);

    return m_dwNextImageType;
}

BOOLEAN CH264VDec::IsNextBaseView()
{
    return (stream_global->m_iNextViewIndex == 0);
}

void CH264VDec::SetNextSkipPOC()
{
    stream_global->m_iPOCofDroppedFrame = stream_global->m_iNextPOC;
    stream_global->m_iFrameNeedtoSkip++;
}

BOOLEAN CH264VDec::IsNextDenpendentViewSkipped()
{
    if(stream_global->m_iFrameNeedtoSkip > 0 && stream_global->m_iPOCofDroppedFrame == stream_global->m_iNextPOC)
    {
        stream_global->m_iFrameNeedtoSkip--;
        return TRUE;
    }
    else
        return FALSE;
}

DWORD CH264VDec::GetRemainBufferLength()
{
    DEBUG_INFO("[CH264VDec] GetRemainBufferLength() %d", (int)(buf_end-buf_begin));

    if (!m_pStream)
        return 0;

    return (DWORD)(buf_end-buf_begin);
}

void CH264VDec::UpdateOpenOption(void *pOpenOption)
{
    DEBUG_INFO("[CH264VDec] UpdateOpenOption()");

    H264VDecHP_OpenOptions *pOptions = (H264VDecHP_OpenOptions*)pOpenOption;
    m_pOpenOptions->dwThreads = pOptions->dwThreads;
    m_pOpenOptions->dwThreadAffinity = pOptions->dwThreadAffinity;
    m_pOpenOptions->dwBuffers = pOptions->dwBuffers;
    m_pOpenOptions->dwFillFrameNumGap = pOptions->dwFillFrameNumGap;

    m_pOpenOptions->pfnDataCallback = pOptions->pfnDataCallback;
    m_pOpenOptions->pvDataContext = pOptions->pvDataContext;
    m_pOpenOptions->dwH264RegKey = pOptions->dwH264RegKey;
    m_pOpenOptions->dxvaVer = pOptions->pAccel.dxvaVer;

    m_pOpenOptions->uiH264VGACard = pOptions->uiH264VGACard;
    m_pOpenOptions->uiH264DXVAMode = pOptions->uiH264DXVAMode;

    m_pOpenOptions->pfnGetParamCallback = NULL;
    m_pOpenOptions->pIviCP = pOptions->pIviCP;

    m_pOpenOptions->dwSingleThreadMode = pOptions->dwSingleThreadMode;
}

void CH264VDec::UpdateOpenOptionEx(void *pOpenOption)
{
    DEBUG_INFO("[CH264VDec] UpdateOpenOptionEx()");

    memcpy(m_pOpenOptions, pOpenOption, sizeof(H264VDecHP_OpenOptionsEx));
}

//////////////////////////////////////////////////////////////////////////
// DXVA related
DWORD CH264VDec::GetDXVAMode()
{
    return (DWORD)m_pOpenOptions->uiH264DXVAMode;
}


DWORD CH264VDec::GetStreamBufferSize()
{
    stream_global = (StreamParameters*)m_pStream;    
    
    return DWORD(stream_global->m_nalu_global->max_size);        

    
}

HRESULT CH264VDec::DecodeBistreamDXVA(CCollectOneNalu *pCollectNalu, BOOL bEOS, BOOL *pSkip)
{
    DEBUG_INFO("[CH264VDec] DecodeBistreamDXVA(), EOS: %d", bEOS);

    BOOL bSkip = *pSkip;

    if (!m_pStream)
        return E_FAIL;

#if !defined(_COLLECT_PIC_)
    ImageParameters *img = (ImageParameters*) m_pStream;
#else
    StreamParameters *pStream = (StreamParameters*) m_pStream;
    ImageParameters *img = pStream->m_img[0];
#endif

    HRESULT hr = CREL_OK;
    BYTE startcode[3] = {0, 0, 1};
    int header = -1;
    int pic_stor_idx;
    int ret = CREL_FAIL;

#if !defined(_COLLECT_PIC_)
    IMGPAR stop_indicator=0;
#else
    for (unsigned int i=0; i<IMG_NUM; i++)
    {
        img_array[i]->stop_indicator=0;
    }
#endif

    memset(IMGPAR SkipedBFrames, 0, MAX_NUM_VIEWS * 2 * sizeof(unsigned int));
    IMGPAR SkipThisFrame = FALSE;

    stream_global->m_is_MTMS = 0;

	if(m_bMVCFieldEOS)
	{
		img = img_array[m_iMVCFieldEOSActiveIndex];
		if(IMGPAR UnCompress_Idx == -1)
		{
			header = EOS;
			m_bMVCFieldEOS = FALSE;
		}
	}
    else if (bEOS)
    {
        int pos;
        hr = pCollectNalu->DXVA_CollectOneNALU(bEOS, &pos, (LPVOID)&(((CH264DXVABase*)g_pH264DXVA)->m_lpnalu[3]), (LPVOID)&(((CH264DXVA2*)g_pH264DXVA)->m_nNALUSkipByte));
        
        if (stream_global->m_bTrickEOS && (( (g_bReceivedFirst==1) && ( IMGPAR currentSlice == NULL))|| (g_bReceivedFirst==2)))
            bSkip = TRUE;

        if (SUCCEEDED(hr) && pos>0)
        {
            CopyNALU(nalu_global, pCollectNalu->m_pnalu);
            ret = HW_DecodeOnePicture ARGS2(&header, bSkip);
        }
        else if (pCollectNalu->GetDataStatus() == E_H264_DATA_STATUS_DATA_DISCONTINUITY)
        {
            DEBUG_SHOW_ERROR_INFO("[ERROR] Skip this frame due to data discontinuity on EOS!!");
            bSkip = TRUE;
        }

        HRESULT hrEOSProcess = DoEOSBistreamDXVA(bSkip, ret, &header);
        if (hrEOSProcess != S_OK)
            return hrEOSProcess;
    }
    else
    {
        memcpy(((CH264DXVABase*)g_pH264DXVA)->m_lpnalu, startcode, 3);
        while(1)
        {
            if (!nalu_global_available)
            {
                int pos;
                hr = pCollectNalu->DXVA_CollectOneNALU(bEOS, &pos, (LPVOID)&(((CH264DXVABase*)g_pH264DXVA)->m_lpnalu[3]), (LPVOID)&(((CH264DXVA2*)g_pH264DXVA)->m_nNALUSkipByte));
                if (pos == 0) //Received EndOfStream NALU
                {
                    HRESULT hrEOSProcess = DoEOSBistreamDXVA(bSkip, ret, &header);
                    if (hrEOSProcess != S_OK)
                        return hrEOSProcess;
                    else
                        break;
                }
            }
            else
                hr = CREL_OK;

            if (SUCCEEDED(hr))
            {
                if (!nalu_global_available)
                    CopyNALU(nalu_global, pCollectNalu->m_pnalu);
            }
            else
                return hr;

            if (stream_global->m_bTrickEOS && (( (g_bReceivedFirst==1) && ( IMGPAR currentSlice == NULL))|| (g_bReceivedFirst==2)))
                bSkip = TRUE;

            ret = HW_DecodeOnePicture ARGS2(&header, bSkip);

            if (m_dwNextImageType != next_image_type)
            {
                m_dwNextImageType = next_image_type;
                *pSkip = bSkip = 0; //Reset Skip Flag
            }

			if (ret == CREL_ERROR_H264_SYNCNOTFOUND)
				continue;
			else
			{
				int iMVCFieldViewindex = 0;

				if(stream_global->bMVC == TRUE && IMGPAR firstSlice)
				{
					if(IMGPAR firstSlice->field_pic_flag == TRUE)
					{
						img = img_array[stream_global->m_iMVCFieldPicActiveIndex];
						iMVCFieldViewindex = IMGPAR firstSlice->viewIndex;
					}
				}

				IMGPAR number                  = stream_global->number;
				IMGPAR pre_frame_num           = stream_global->pre_frame_num;
				IMGPAR PreviousFrameNum        = stream_global->PreviousFrameNum[iMVCFieldViewindex];
				IMGPAR PreviousFrameNumOffset  = stream_global->PreviousFrameNumOffset[iMVCFieldViewindex];
				IMGPAR PreviousPOC             = stream_global->PreviousPOC[iMVCFieldViewindex];    
				IMGPAR ThisPOC                 = stream_global->ThisPOC[iMVCFieldViewindex];
				//Need to be reset in eject but can't be reset in seek, move to open
				//IMGPAR PrevPicOrderCntLsb      = stream_global->PrevPicOrderCntLsb;
				//IMGPAR PrevPicOrderCntMsb      = stream_global->PrevPicOrderCntMsb;
				IMGPAR last_has_mmco_5         = stream_global->last_has_mmco_5;

                break;
            }
        }
    }

    if((SUCCEEDED(ret)|| header == EOS || m_bMVCFieldEOS == TRUE) && IMGPAR UnCompress_Idx!=-1)    
    {
		if(stream_global->bMVC == TRUE && IMGPAR firstSlice->field_pic_flag == TRUE)
		{
			if((header == EOS || m_bMVCFieldEOS == TRUE) && IviDxva2 == g_DXVAVer)
			{
				if(m_iMVCFieldEOSActiveIndex < stream_global->num_views)
				{
					img = img_array[m_iMVCFieldEOSActiveIndex];
				
					IMGPAR number                  = stream_global->number;
					IMGPAR pre_frame_num           = stream_global->pre_frame_num;
					IMGPAR PreviousFrameNum        = stream_global->PreviousFrameNum[m_iMVCFieldEOSActiveIndex];
					IMGPAR PreviousFrameNumOffset  = stream_global->PreviousFrameNumOffset[m_iMVCFieldEOSActiveIndex];
					IMGPAR PreviousPOC             = stream_global->PreviousPOC[m_iMVCFieldEOSActiveIndex];    
					IMGPAR ThisPOC                 = stream_global->ThisPOC[m_iMVCFieldEOSActiveIndex];
					IMGPAR last_has_mmco_5         = stream_global->last_has_mmco_5;

					if(((CH264DXVA2*)g_pH264DXVA)->m_iMVCFieldNeedExecuteIndex == m_iMVCFieldEOSActiveIndex)
					{
						ret = ((CH264DXVABase*)g_pH264DXVA)->BA_ExecuteBuffers ARGS0();
						if(FAILED(ret))
						{
							DEBUG_SHOW_ERROR_INFO("[ERROR] MVC field coding execute failed when EOS");
							return ret;
						}
						dec_picture->pic_store_idx = IMGPAR UnCompress_Idx;
						ret = store_picture_in_dpb ARGS1(dec_picture);
						if(FAILED(ret))
						{
							DEBUG_SHOW_ERROR_INFO("[ERROR] MVC field coding store picture failed when EOS");
							return ret;
						}
						((CH264DXVA2*)g_pH264DXVA)->m_iMVCFieldNeedExecuteIndex = -1;
					}

					m_iMVCFieldEOSActiveIndex++;

	
					if(m_iMVCFieldEOSActiveIndex < stream_global->num_views)
					{
						header = SOP;
						m_bMVCFieldEOS = TRUE;
					}
					else
					{
						header = EOS;
						m_bMVCFieldEOS = FALSE;
					}
				}

			}
			else
				ret = S_OK;
		}
		else
			ret = ((CH264DXVABase*)g_pH264DXVA)->BA_ExecuteBuffers ARGS0();

        stream_global->number++;        

        IMGPAR UnCompress_Idx = -1;
    }
    else if(IMGPAR UnCompress_Idx!=-1) 
    {
        DEBUG_SHOW_ERROR_INFO("[ERROR] DecodeBistreamDXVA Fail! Reset Buffer!");
        ((CH264DXVABase*)g_pH264DXVA)->m_pUncompBufQueue->PutItem(IMGPAR UnCompress_Idx);
        IMGPAR UnCompress_Idx = -1;
        ((CH264DXVABase*)g_pH264DXVA)->BA_ResetLastCompBufStaus();
    }

    if (IMGPAR slice_number == 0)
    {
        DEBUG_SHOW_SW_INFO("[ERROR] Slice Number is 0 ==> return -1");
        return CREL_ERROR_H264_UNDEFINED;
    }

    int view_index = (IMGPAR firstSlice) ? (IMGPAR firstSlice->viewIndex) : 0;
    g_llDtsCount[view_index] = ((IMGPAR firstSlice->has_pts) && (IMGPAR firstSlice->picture_type == I_SLICE)) ? 0 : (g_llDtsCount[view_index]+1);
    g_llDecCount[view_index]++;

    pic_stor_idx = IMGPAR UnCompress_Idx;

    if (stream_global->is_first && (!stream_global->bMVC || (view_index == stream_global->num_views - 1)))
    {
        stream_global->is_first = 0;
        stream_global->bSeekToOpenGOP = 1;
    }

    if(header==0 || pic_stor_idx==-1) // Frame skipped case
        g_dwSkipFrameCounter++;

    // release slice buffer
    IMGPAR currentSlice = IMGPAR firstSlice;
    for (int j=0; j< IMGPAR slice_number; j++)
    {
        //active_pps = &PicParSet[IMGPAR currentSlice->pic_parameter_set_id];
        memcpy(&active_pps, &PicParSet[IMGPAR currentSlice->pic_parameter_set_id], sizeof(pic_parameter_set_rbsp_t));
        IMGPAR prevSlice = IMGPAR currentSlice;
        IMGPAR currentSlice = (Slice*) IMGPAR currentSlice->nextSlice;
        free_new_slice(IMGPAR prevSlice);
    }
    IMGPAR firstSlice = IMGPAR currentSlice = IMGPAR prevSlice = NULL;
    IMGPAR current_mb_nr_r = IMGPAR current_mb_nr_d = -4712;   // impossible value for debugging, StW
    IMGPAR error_mb_nr = -4712;
    IMGPAR current_slice_nr = 0;
    IMGPAR slice_number = 0;

    dec_picture = NULL;

    if (IMGPAR structure != FRAME)
    {
        IMGPAR mb_decdata = IMGPAR mb_decdata_ori;
        IMGPAR cof_array = IMGPAR cof_array_ori;
    }
    //end of release slice buffer

    if(header==EOS)
    {
        DEBUG_SHOW_HW_INFO("[DecodeBitstreamDXVA] Flush DPB on header==EOS!");

        int hr = S_OK;
        for(int i = 0; i < stream_global->num_views; i++)
        {
            if(dpb.used_size_on_view[i]) 
            {
                ret = flush_dpb ARGS1(i);
                if (FAILED(ret)) 
                {
                    hr = ret;
                }
            }

            ret = init_dpb ARGS1(i);
            if (FAILED(ret)) 
            {
                hr = ret;
            }
        }

        if(hr != S_OK)
            return hr;

        nalu_global_available = 0;    
        g_bReceivedFirst = 0;

        return CREL_ERROR_H264_UNDEFINED;
    }

    //check EOS to decode the last picture    
    return ret;
}

HRESULT CH264VDec::DoEOSBistreamDXVA(BOOL bSkip, int nRetDecodeOnePicture, int *pHeader)
{
#if !defined(_COLLECT_PIC_)
    ImageParameters *img = (ImageParameters*) m_pStream;
#else
    StreamParameters *pStream = (StreamParameters*) m_pStream;
    ImageParameters *img = pStream->m_img[0];
#endif

    int nRet = CREL_FAIL;

	BOOL bMVCFiled = (IMGPAR firstSlice)? IMGPAR firstSlice->field_pic_flag && stream_global->bMVC: 0;
    if(!bSkip)
    {
        if (dec_picture && IMGPAR UnCompress_Idx != -1)
        {
            *pHeader = EOS;
            if(FAILED(nRetDecodeOnePicture) && !bMVCFiled)
            {
                dec_picture->pic_store_idx = IMGPAR UnCompress_Idx;
                nRet = store_picture_in_dpb ARGS1(dec_picture);
                if (FAILED(nRet)) {
                    return nRet;
                }
            }
        }
        else
        {
            DEBUG_SHOW_HW_INFO("[DecodeBitstreamDXVA] Flush DPB on EOS!");
            if(dpb.used_size_on_view && dpb.used_size_on_view[0]) {
                nRet = flush_dpb ARGS1(0);
                if (FAILED(nRet)) {
                    return nRet;
                }
            }

            nalu_global_available = 0;    
            g_bReceivedFirst = 0;

            return CREL_ERROR_H264_UNDEFINED;
        }
    }
    else
    {
        if (dec_picture && IMGPAR UnCompress_Idx != -1)
        {
            if (IMGPAR firstSlice->m_pic_combine_status != IMGPAR currentSlice->m_pic_combine_status) //previous picture is ready to execute()
            {
                DEBUG_SHOW_HW_INFO("[DecodeBitstreamDXVA] Skip this frame, but execute previous frame! %d", IMGPAR firstSlice->m_pic_combine_status);
                *pHeader = EOS;
                if(FAILED(nRet) && !bMVCFiled)
                {
                    dec_picture->pic_store_idx = IMGPAR UnCompress_Idx;
                    nRet = store_picture_in_dpb ARGS1(dec_picture);
                    if (FAILED(nRet)) {
                        return nRet;
                    }
                }
            }
            else
            {
                DEBUG_SHOW_ERROR_INFO("[ERROR] Skip this %s coding frame! Reset Buffer!", IMGPAR firstSlice->m_pic_combine_status==FRAME ? "FRAME":"FIELD");

                ((CH264DXVABase*)g_pH264DXVA)->m_pUncompBufQueue->PutItem(IMGPAR UnCompress_Idx);
                IMGPAR UnCompress_Idx = -1;
                ((CH264DXVABase*)g_pH264DXVA)->BA_ResetLastCompBufStaus();

                if (IMGPAR firstSlice->m_pic_combine_status == TOP_FIELD)
                    release_storable_picture ARGS2(IMGPAR m_dec_picture_top, 1);
                else if (IMGPAR firstSlice->m_pic_combine_status == BOTTOM_FIELD)
                    release_storable_picture ARGS2(IMGPAR m_dec_picture_bottom, 1);
                else if (IMGPAR firstSlice->m_pic_combine_status == FRAME)
                    release_storable_picture ARGS2(dec_picture, 1);

                dec_picture = NULL;
            }
        }
        else
        {
            DEBUG_SHOW_HW_INFO("[DecodeBitstreamDXVA] Flush DPB on EOS!");
            if(dpb.used_size_on_view && dpb.used_size_on_view[0]) {
                nRet = flush_dpb ARGS1(0);
                if (FAILED(nRet)) {
                    return nRet;
                }
            }

            nalu_global_available = 0;    
            g_bReceivedFirst = 0;

            return CREL_ERROR_H264_UNDEFINED;
        }
    }

    return S_OK;
}

//sync with render
BOOL CH264VDec::GetDisplayStatus(DWORD dwIndex)
{
    DEBUG_INFO("[CH264VDec] GetDisplayStatus()");

    if (!m_pStream)
        return 0;

#if !defined(_COLLECT_PIC_)
    ImageParameters *img = (ImageParameters*) m_pStream;
#else
    StreamParameters *pStream = (StreamParameters*) m_pStream;
    ImageParameters *img = pStream->m_img[0];
#endif


    return m_pDecImpl->GetDisplayStatus ARGS1(dwIndex);
}

BOOL CH264VDec::FinishDisplay(DWORD dwEstimateNextFrame)
{
    DEBUG_INFO("[CH264VDec] FinishDisplay()");

    if (!m_pStream)
        return 0;

#if !defined(_COLLECT_PIC_)
    ImageParameters *img = (ImageParameters*) m_pStream;
#else
    StreamParameters *pStream = (StreamParameters*) m_pStream;
    ImageParameters *img = pStream->m_img[0];
#endif

    return m_pDecImpl->FinishDisplay ARGS1(dwEstimateNextFrame);
}

DWORD CH264VDec::GetCurrentTotalByteCount_SECOP()
{
	DEBUG_INFO("[CH264VDec] GetCurrentTotalByteCount_SECOP()");

	if (!m_pStream)
		return 0;

#if !defined(_COLLECT_PIC_)
	ImageParameters *img = (ImageParameters*) m_pStream;
#else
	StreamParameters *pStream = (StreamParameters*) m_pStream;
	ImageParameters *img = pStream->m_img[0];
#endif

	DWORD dwCurrentTotalByteCount = 0;
	if (g_pH264DXVA && (g_DXVAVer == IviDxva2) && (E_H264_VGACARD_NVIDIA == ((CH264DXVABase*)g_pH264DXVA)->GetVGAType()))
		dwCurrentTotalByteCount = ((CH264DXVA2*)g_pH264DXVA)->GetCurrentTotalByteCount_SECOP();

	return dwCurrentTotalByteCount;
}

//////////////////////////////////////////////////////////////////////////
// CCollectOneNalu implementation
CCollectOneNalu::CCollectOneNalu()
{
    buffer_begin = buffer_end = 0;

    //m_pnalu = AllocNALU(MAX_CODED_FRAME_SIZE);
    m_pnalu = NULL;
    m_state_NAL.begin = 0;
    m_state_NAL.buf = NULL;
    m_state_NAL.bStartCodeFound = 0;
    m_state_NAL.buf_size = 0;
    m_state_NAL.pos = 0;

    m_bReceivedEOS = FALSE;
    m_dwDataStatus = E_H264_DATA_STATUS_SUCCESS;
}

CCollectOneNalu::~CCollectOneNalu()
{
    if (m_state_NAL.buf)
    {
        _aligned_free(m_state_NAL.buf);
        m_state_NAL.buf = NULL;
    }
    FreeNALU(m_pnalu);
}

void CCollectOneNalu::SetBufferPointer(BYTE *pbDataBuffer, DWORD dwInputLength)
{
    DEBUG_SHOW_SW_INFO_DETAIL("[CCollectOneNalu] This InputBuffer Len: %d", dwInputLength);

    buffer_begin = pbDataBuffer;
    buffer_end = pbDataBuffer + dwInputLength;
}

void CCollectOneNalu::SetGlobalTimeStamp(REFERENCE_TIME rtTimeStamp, REFERENCE_TIME rtTimeLength)
{
    DEBUG_INFO("[CCollectOneNalu] TimeStamp: %d", rtTimeStamp);
    //workaround for ulead demux will send negative PTS into decoder
    if (rtTimeStamp < 0)
        rtTimeStamp = 3003;

    H264_TS pts;
    memset(&pts, 0, sizeof(H264_TS));
    pts.ts = rtTimeStamp;
    pts.freq = GMI_FREQ_90KHZ;
    pts.tslength = rtTimeLength;

    m_QueueTS.push(pts);
}

HRESULT CCollectOneNalu::CollectOneNALU(BOOL bEOS, int* pos)
{
    CREL_RETURN ret = GetAnnexbNALU(m_pnalu, bEOS, pos);
    if (FAILED(ret)) {
        return E_FAIL;
    }

    return S_OK;
}

HRESULT CCollectOneNalu::DXVA_CollectOneNALU(BOOL bEOS, int* pos, LPVOID lpPrivateInputData, LPVOID lpPrivateOutputData)
{
    CREL_RETURN ret = DXVA_GetAnnexbNALU(m_pnalu, bEOS, pos, lpPrivateInputData, lpPrivateOutputData);
    if (FAILED(ret)) {
        return E_FAIL;
    }

    return S_OK;
}

CREL_RETURN CCollectOneNalu::GetAnnexbNALU(NALU_t *one_nalu, BOOL bEOS, int* output_pos)
{
    int pos, zero_count, nalu_type;
    CREL_RETURN ret = CREL_OK;

    *output_pos = -1;

    pos = one_nalu->pos;
    zero_count = one_nalu->zero_count;

    if (bEOS)
    {
        if (pos && m_dwDataStatus == E_H264_DATA_STATUS_SUCCESS)
        {
            one_nalu->len = pos - zero_count;
            one_nalu->forbidden_bit        = (one_nalu->buf[0]>>7) & 1;
            one_nalu->nal_reference_idc    = (one_nalu->buf[0]>>5) & 3;
            one_nalu->nal_unit_type        = (one_nalu->buf[0]) & 0x1f;
            one_nalu->pos = 0;
            one_nalu->zero_count = 0;

            *output_pos = zero_count>2 ? pos-3 : pos-2;

            return (*output_pos);
        }
        else
        {
            if (m_dwDataStatus == E_H264_DATA_STATUS_DATA_DISCONTINUITY)
                DEBUG_SHOW_ERROR_INFO("[ERROR] The input data is discontinuity!!");
            return CREL_ERROR_H264_UNDEFINED;
        }
    }

    ret = AllocNALUDecodingBuffer(one_nalu);
    if (FAILED(ret)) {
        return ret;
    }

    if(!m_state_NAL.bStartCodeFound)
    {
        ret = ConsumeAnnexbNALStartCode(one_nalu);
        if (FAILED(ret))
        {
            DEBUG_SHOW_SW_INFO("ERROR! Failed to find start code");
            return ret;        // failed to find start code.
        }

        m_state_NAL.bStartCodeFound = TRUE;

        WRITE_BITSTREAM(0, 0, 1);
    }

    if(buffer_begin==buffer_end || !buffer_begin)
        return -1;

    if (!pos)
    {
        WRITE_BITSTREAM(buffer_begin, 1, 0);
        one_nalu->buf[pos++] = *buffer_begin++;
        nalu_type = one_nalu->nal_unit_type = one_nalu->buf[0] & 0x1f;
        memset(&one_nalu->pts, 0x0, sizeof(H264_TS));

        if (m_bReceivedEOS==FALSE && (nalu_type == NALU_TYPE_EOSEQ || nalu_type == NALU_TYPE_EOSTREAM))
        {
            DEBUG_SHOW_SW_INFO("Read EOS[%d] NALU!!", nalu_type);
            m_bReceivedEOS = TRUE;

            *output_pos = 0;
            buffer_begin--;
            return 0;    //trigger EOS
        }
        else
        {
            m_bReceivedEOS = FALSE;

            if ((nalu_type == NALU_TYPE_SLICE || nalu_type == NALU_TYPE_IDR) && m_QueueTS.size())
            {
                if (m_QueueTS.size() > 1)
                    DEBUG_SHOW_ERROR_INFO("[ERROR] PTS over received!! %d", m_QueueTS.size());
                one_nalu->pts = m_QueueTS.front();
                m_QueueTS.pop();
            }
        }
    }

    for(;;)
    {
        if(buffer_begin==buffer_end)
        {
            DEBUG_SHOW_SW_INFO("buf_begin: %d  buf_end: %d", buffer_begin, buffer_end);
            one_nalu->pos = pos;
            one_nalu->zero_count = zero_count;
            return CREL_ERROR_H264_UNDEFINED;
        }
        if(*buffer_begin==0)
            zero_count++;
        else
        {
            if(zero_count>1)
            {
                if(zero_count==ZEROBYTES_SHORTSTARTCODE && *buffer_begin==0x03)
                {
                    WRITE_BITSTREAM(buffer_begin, 1, 0);
                    zero_count = 0;
                    buffer_begin++;
                    continue;
                }
                else if(*buffer_begin==1)
                {                    
                    WRITE_BITSTREAM(buffer_begin, 1, 0);
                    one_nalu->buf[pos] = *buffer_begin++;
                    break;
                }
            }
            zero_count = 0;
        }
        if (pos >= one_nalu->max_size) {

            one_nalu->pos = 0;
            one_nalu->zero_count = 0;
            m_state_NAL.bStartCodeFound = FALSE;
            return CREL_ERROR_H264_UNDEFINED;

        }
        WRITE_BITSTREAM(buffer_begin, 1, 0);
        one_nalu->buf[pos++] = *buffer_begin++;
    }

    // Here the leading zeros(if any), Start code, the complete NALU, trailing zeros(if any)
    // and the next start code is in the stNAL.buf.
    // The size of stNAL.buf is pos, pos+rewind are the number of bytes excluding the next
    // start code, and (pos+rewind)-TrailingZero8Bits
    // is the size of the NALU.
    one_nalu->len = pos - zero_count;
    one_nalu->forbidden_bit        = (one_nalu->buf[0]>>7) & 1;
    one_nalu->nal_reference_idc    = (one_nalu->buf[0]>>5) & 3;
    one_nalu->nal_unit_type        = (one_nalu->buf[0]) & 0x1f;
    // IoK: This is constrained to be different than 2,3,4 for HD-DVD streams
    if(one_nalu->nal_unit_type==NALU_TYPE_DPA || one_nalu->nal_unit_type==NALU_TYPE_DPB || one_nalu->nal_unit_type==NALU_TYPE_DPC)
        return CREL_ERROR_H264_NOTSUPPORTED|CREL_ERROR_H264_HD_ProfileFault;

    one_nalu->pos = 0;
    one_nalu->zero_count = 0;

    *output_pos = zero_count>2 ? pos-3 : pos-2;
    return CREL_OK;
}

CREL_RETURN CCollectOneNalu::DXVA_GetAnnexbNALU(NALU_t *one_nalu, BOOL bEOS, int* output_pos, LPVOID lpPrivateInputData, LPVOID lpPrivateOutputData)
{
	int pos, zero_count, nalu_type=0;
    CREL_RETURN ret = CREL_OK;
    BYTE *pbPrivateData = (BYTE*)lpPrivateInputData;
    DWORD *pdwNALUSkipByte = (DWORD*)(lpPrivateOutputData);
    int nPrivatePos;

    *output_pos = -1;

    pos = one_nalu->pos;
    zero_count = one_nalu->zero_count;

    if (bEOS)
    {
        if (pos && m_dwDataStatus == E_H264_DATA_STATUS_SUCCESS)
        {
            one_nalu->len = pos - zero_count;
            if (((one_nalu->buf[0]) & 0x1f) != NALU_TYPE_SECOP)
            {
            	one_nalu->forbidden_bit        = (one_nalu->buf[0]>>7) & 1;
            	one_nalu->nal_reference_idc    = (one_nalu->buf[0]>>5) & 3;
            	one_nalu->nal_unit_type        = (one_nalu->buf[0]) & 0x1f;
            }
            else
            {
            	one_nalu->forbidden_bit        = (one_nalu->buf[1]>>7) & 1;
            	one_nalu->nal_reference_idc    = (one_nalu->buf[1]>>5) & 3;
            	one_nalu->nal_unit_type        = (one_nalu->buf[0]) & 0x1f;
            }
            one_nalu->pos = 0;
            one_nalu->zero_count = 0;

            *output_pos = zero_count>2 ? pos-3 : pos-2;

            return (*output_pos);
        }
        else
        {
            if (m_dwDataStatus == E_H264_DATA_STATUS_DATA_DISCONTINUITY)
                DEBUG_SHOW_ERROR_INFO("[ERROR] The input data is discontinuity!!");
            return CREL_ERROR_H264_UNDEFINED;
        }
    }

    ret = AllocNALUDecodingBuffer(one_nalu);
    if (FAILED(ret)) {
        return ret;
    }

    if(!m_state_NAL.bStartCodeFound)
    {
        ret = ConsumeAnnexbNALStartCode(one_nalu);
        if (FAILED(ret))
        {
            DEBUG_SHOW_SW_INFO("ERROR! Failed to find start code");
            return ret;        // failed to find start code.
        }

        m_state_NAL.bStartCodeFound = TRUE;

        WRITE_BITSTREAM(0, 0, 1);
    }

    if(buffer_begin==buffer_end || !buffer_begin)
        return -1;

    if (!pos)
    {
        WRITE_BITSTREAM(buffer_begin, 1, 0);
        (*pdwNALUSkipByte) = 0;
        nPrivatePos = pos;
        pbPrivateData[nPrivatePos++] = one_nalu->buf[pos++] = *buffer_begin++;
        nalu_type = one_nalu->nal_unit_type = one_nalu->buf[0] & 0x1f;
        memset(&one_nalu->pts, 0x0, sizeof(H264_TS));

        if (m_bReceivedEOS==FALSE && (nalu_type == NALU_TYPE_EOSEQ || nalu_type == NALU_TYPE_EOSTREAM))
        {
            DEBUG_SHOW_SW_INFO("Read EOS[%d] NALU!!", nalu_type);
            m_bReceivedEOS = TRUE;

            *output_pos = 0;
            buffer_begin--;
            return 0;    //trigger EOS
        }
        else
        {
            m_bReceivedEOS = FALSE;

            if ((nalu_type == NALU_TYPE_SLICE || nalu_type == NALU_TYPE_IDR || (nalu_type == NALU_TYPE_SECOP)) && m_QueueTS.size())
            {
                if (m_QueueTS.size() > 1)
                    DEBUG_SHOW_ERROR_INFO("[ERROR] PTS over received!! %d", m_QueueTS.size());
                one_nalu->pts = m_QueueTS.front();
                m_QueueTS.pop();
            }
        }
    }
    else
        nPrivatePos = pos + (*pdwNALUSkipByte);

    for(;;)
    {
        if(buffer_begin==buffer_end)
        {
            DEBUG_SHOW_SW_INFO("buf_begin: %d  buf_end: %d", buffer_begin, buffer_end);
            one_nalu->pos = pos;
            one_nalu->zero_count = zero_count;
            return CREL_ERROR_H264_UNDEFINED;
        }
        if(*buffer_begin==0)
            zero_count++;
        else
        {
            if(zero_count>1)
            {
                if(zero_count==ZEROBYTES_SHORTSTARTCODE && *buffer_begin==0x03)
                {
                    WRITE_BITSTREAM(buffer_begin, 1, 0);
                    zero_count = 0;
                    pbPrivateData[nPrivatePos++] = *buffer_begin++;
                    (*pdwNALUSkipByte)++;
                    continue;
                }
                else if(*buffer_begin==1)
                {                    
                    WRITE_BITSTREAM(buffer_begin, 1, 0);
                    pbPrivateData[nPrivatePos] = one_nalu->buf[pos] = *buffer_begin++;
                    break;
                }
            }
            zero_count = 0;
        }
        if (pos >= one_nalu->max_size) {

            one_nalu->pos = 0;
            one_nalu->zero_count = 0;
            m_state_NAL.bStartCodeFound = FALSE;
            return CREL_ERROR_H264_UNDEFINED;

        }
        WRITE_BITSTREAM(buffer_begin, 1, 0);
        pbPrivateData[nPrivatePos++] = one_nalu->buf[pos++] = *buffer_begin++;
    }

    // Here the leading zeros(if any), Start code, the complete NALU, trailing zeros(if any)
    // and the next start code is in the stNAL.buf.
    // The size of stNAL.buf is pos, pos+rewind are the number of bytes excluding the next
    // start code, and (pos+rewind)-TrailingZero8Bits
    // is the size of the NALU.
    one_nalu->len = pos - zero_count;
    if (((one_nalu->buf[0]) & 0x1f) != NALU_TYPE_SECOP)
    {
    	one_nalu->forbidden_bit        = (one_nalu->buf[0]>>7) & 1;
    	one_nalu->nal_reference_idc    = (one_nalu->buf[0]>>5) & 3;
    	one_nalu->nal_unit_type        = (one_nalu->buf[0]) & 0x1f;
    }
    else
    {
    	one_nalu->forbidden_bit        = (one_nalu->buf[1]>>7) & 1;
    	one_nalu->nal_reference_idc    = (one_nalu->buf[1]>>5) & 3;
    	one_nalu->nal_unit_type        = (one_nalu->buf[0]) & 0x1f;
    }
    // IoK: This is constrained to be different than 2,3,4 for HD-DVD streams
    if(one_nalu->nal_unit_type==NALU_TYPE_DPA || one_nalu->nal_unit_type==NALU_TYPE_DPB || one_nalu->nal_unit_type==NALU_TYPE_DPC)
        return CREL_ERROR_H264_NOTSUPPORTED|CREL_ERROR_H264_HD_ProfileFault;

    one_nalu->pos = 0;
    one_nalu->zero_count = 0;

    *output_pos = zero_count>2 ? pos-3 : pos-2;
    return CREL_OK;
}

CREL_RETURN CCollectOneNalu::AllocNALUDecodingBuffer(NALU_t *one_nalu)
{
    int pos = m_state_NAL.pos;

    if(one_nalu->max_size <= m_state_NAL.buf_size)
        return CREL_OK;

    if(pos > 0)
    {
        memcpy(one_nalu->buf,m_state_NAL.buf+m_state_NAL.begin,m_state_NAL.pos-m_state_NAL.begin);
    }
    if(m_state_NAL.buf)
        _aligned_free(m_state_NAL.buf);
    m_state_NAL.buf = (char*) _aligned_malloc(one_nalu->max_size, 16);
    if (m_state_NAL.buf == NULL) {
        return CREL_ERROR_H264_NOMEMORY;
    }
    m_state_NAL.buf_size = one_nalu->max_size;
    if(pos > 0)
    {
        int size = m_state_NAL.pos-m_state_NAL.begin;
        memcpy(m_state_NAL.buf, one_nalu->buf, size);
        m_state_NAL.begin = 0;
        m_state_NAL.pos = size;
    }
    else
    {
        m_state_NAL.begin = 0;
        m_state_NAL.pos = 0;
    }

    return CREL_OK;
}

CREL_RETURN CCollectOneNalu::ConsumeAnnexbNALStartCode(NALU_t *one_nalu)
{
    int zero_count;

    for(zero_count=0;;)
    {
        if(buffer_begin==buffer_end || !buffer_begin)
        {
            return CREL_ERROR_H264_SYNCNOTFOUND;
        }
        if(*buffer_begin==0)
            zero_count++;
        else
        {
            if(*buffer_begin==1 && zero_count>1)
            {
                buffer_begin++;
                return CREL_OK;
            }
            zero_count = 0;
        }
        buffer_begin++;
    }
}

DWORD CCollectOneNalu::GetRemainBufferLength()
{
    DEBUG_SHOW_SW_INFO_DETAIL("[CCollectOneNalu] GetRemainBufferLength() %d", (int)(buffer_end-buffer_begin));

    return (DWORD)(buffer_end-buffer_begin);
}

void CCollectOneNalu::Reset()
{
    DEBUG_INFO("[CCollectOneNalu] Reset()");

    m_pnalu->pos = 0;
    m_pnalu->zero_count = 0;

    buffer_begin = buffer_end;

    while (m_QueueTS.empty() == false)
        m_QueueTS.pop();

    m_dwDataStatus = E_H264_DATA_STATUS_SUCCESS;
}
