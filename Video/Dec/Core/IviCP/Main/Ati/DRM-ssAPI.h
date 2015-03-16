////////////////////////////////////////////////////////////////////////////////////
// DRM-ssAPI.h
//
// Copyright AMD 2007 - AMD Confidential
// Copyright ATI Technologies 2006 - ATI Confidential
//
// Defines shared secret based authentication protocol between application & driver
//
// Refer to DRM-dxvaAPI.h: Application may use ATIDRM_SSCmds & Parameters within 
// ATIDRM_DXVA_CMDHEADER

#ifndef __DRM_SSAPI_H__
#define __DRM_SSAPI_H__

////////////////////////////////////////////////////////////////////////////////////
// Shared Secret based authentication scheme: 
// {E67B158E-6110-4fa7-BD32-B375BE996705}
DEFINE_GUID(ATIDRM_SS_AUTHSCHEME, 0xe67b158e, 0x6110, 0x4fa7, 0xbd, 0x32, 0xb3, 0x75, 0xbe, 0x99, 0x67, 0x5);

#define ATIDRM_SS_CMD_MASK	0x0100

// Supported (SS) Commands:
enum ATIDRM_SSCmds
{
	ATIDRM_SS_CMD_RECVKS2  = 0x0101,		// Param= ATIDRM_SS_PARAM_RECVKS2
	ATIDRM_SS_CMD_SENDKS1  = 0x0102			// Param= ATIDRM_SS_PARAM_SENDKS1
};

// Parameters for ATIDRM_SS_CMD_RECVKS2
typedef struct 
{
    BYTE    R1[16];							// [IN]  128bit random number
    BYTE    R2[16];							// [OUT] 128bit random number
    BYTE    e_DeviceID_R2[16];				// [OUT] Encrypted-Ka(DeviceID ^ R2) 
    BYTE    e_R1_Ks2[32];					// [OUT] Encrypted-Kx1(R1 << 128 || Ks2)
	BYTE	Reserved[64];
} 
ATIDRM_SS_PARAM_RECVKS2;

// Parameters for ATIDRM_SS_CMD_SENDKS1
typedef struct
{
    BYTE	e_R2_Ks1[32];					// [IN] Encrypted-Kx2(R2 << 128 || Ks1)
	BYTE	Reserved[64];
} 
ATIDRM_SS_PARAM_SENDKS1;

#endif
