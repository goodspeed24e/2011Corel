////////////////////////////////////////////////////////////////////////////////////
// DRM-common.h
//
// Copyright AMD 2007 - AMD Confidential
// Copyright ATI Technologies 2006 - ATI Confidential
//
// Defines definitions shared between ASPLib driver & Client application

#ifndef __DRM_COMMON_H__
#define __DRM_COMMON_H__

// Bus types returned by ATIDRM_DEVICE_CAPS::dwDeviceCaps
enum ATIDRM_DEVICECAPS
{
	ATIDRM_BUSTYPE_UNKNOWN		  =0x0000,  // None
	ATIDRM_BUSTYPE_AGP            =0x0001,  // Legacy Authentication
	ATIDRM_BUSTYPE_PCIe           =0x0002   // XOR based
};

// Cipher types returned by ATIDRM_DEVICE_CAPS::dwCipherCap
enum ATIDRM_CIPHERCAPS
{
	ATIDRM_ALG_NONE			      =0x0000,  // None
	ATIDRM_ALG_LEGACY             =0x0001,  // Legacy Authentication
	ATIDRM_ALG_XOR                =0x0002,  // XOR based
	ATIDRM_ALG_AESLITE            =0x0004,  // AES Lite         
	ATIDRM_ALG_AESCTR             =0x0008,  // AES CTR
	ATIDRM_ALG_IDCT_XOR           =0x0010   // IDCT Scrambling
};

// ATIDRM Decode Profile Configuration: {B9DE9680-FA07-4976-9DED-19E1DB6D966B}
#define DXVA_ATIDRM_Encrypt_GUID \
            0xb9de9680, 0xfa07, 0x4976, 0x9d, 0xed, 0x19, 0xe1, 0xdb, 0x6d, 0x96, 0x6b
DEFINE_GUID(DXVA_ATIDRM_Encrypt, 0xb9de9680, 0xfa07, 0x4976, 0x9d, 0xed, 0x19, 0xe1, 0xdb, 0x6d, 0x96, 0x6b);

// ATIDRM DRM Specific Profile: {5B23D46D-FA5F-4fdc-B78A-7EB2787942EC}
#define DXVA_ATIDRM_Profile_GUID \
            0x5b23d46d, 0xfa5f, 0x4fdc, 0xb7, 0x8a, 0x7e, 0xb2, 0x78, 0x79, 0x42, 0xec
DEFINE_GUID (DXVA_ATIDRM_Profile, 0x5b23d46d, 0xfa5f, 0x4fdc, 0xb7, 0x8a, 0x7e, 0xb2, 0x78, 0x79, 0x42, 0xec);

#endif
