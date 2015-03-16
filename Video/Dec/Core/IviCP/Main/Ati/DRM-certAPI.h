////////////////////////////////////////////////////////////////////////////////////
// DRM-certAPI.h
//
// Copyright AMD 2007 - AMD Confidential
// Copyright ATI Technologies 2006 - ATI Confidential
//
// Defines certificate based authentication protocol between application & driver
//
// Refer to DRM-dxvaAPI.h: Application may use ATIDRM_CERTCmds & Parameters within 
// ATIDRM_DXVA_CMDHEADER

#ifndef __DRM_CERTAPI_H__
#define __DRM_CERTAPI_H__

////////////////////////////////////////////////////////////////////////////////////
// Certificate based authentication scheme: 
// {3D323B44-E5DB-43b0-8B43-0D2A0806DFEF}
DEFINE_GUID (ATIDRM_CERT_AUTHSCHEME, 0x3d323b44, 0xe5db, 0x43b0, 0x8b, 0x43, 0xd, 0x2a, 0x8, 0x6, 0xdf, 0xef);

#define ATIDRM_CERT_CMD_MASK	0x0200

// Supported (CERT) Commands
enum ATIDRM_CERTCmds
{
	ATIDRM_CERT_CMD_RECVCERT	=0x0201,	// Param= ATIDRM_CERT_PARAM
	ATIDRM_CERT_CMD_SENDCERT	=0x0202,	// Param= ATIDRM_CERT_PARAM
	ATIDRM_CERT_CMD_RECVKEY		=0x0203,	// Param= ATIDRM_SESSIONKEY_PARAM
	ATIDRM_CERT_CMD_SENDKEY		=0x0204		// Param= ATIDRM_SESSIONKEY_PARAM
};

// Certificate Parameter
typedef struct 
{
	DWORD	dwCertFormat;					// Format of certificate 
	DWORD	dwCertSize;						// Size of certificate in bytes
	DWORD	dwAuthProtocol;					// Authentication Protocol
	BYTE    certData[8192];					// Certificate data 
	BYTE	Reserved[64];
} 
ATIDRM_CERT_PARAM;

// Session Key Parameter
typedef struct 
{
	DWORD	dwKeyES;						// Encryption scheme 
	DWORD	dwKeySize;						// Size of key data
	BYTE    keyData[2048];					// Encrypted key data 
	BYTE	Reserved[64];
} 
ATIDRM_SESSIONKEY_PARAM;


// Authentication Protocol for ATIDRM_CERT_PARAM::dwAuthProtocol
enum ATIDRM_CERTAuthProt
{
	ATIDRM_CERT_AUTHPROT_NONE			= 0x0000,   // Unknown protocol
	ATIDRM_CERT_AUTHPROT_1WAY  			= 0x0001,	// 1 Way Authentication (App->Device)
	ATIDRM_CERT_AUTHPROT_MUTUAL			= 0x0002,	// Mutual Authentication
	ATIDRM_CERT_AUTHPROT_MAX			= 0x0003 
};

// Certificate formats for ATIDRM_CERT_PARAM::dwCertFormat
enum ATIDRM_CERTFmt
{
	ATIDRM_CERT_FMT_NONE				= 0x0000,	// Unknown format
	ATIDRM_CERT_FMT_CARDEA				= 0x0001,	// Cardea (MS COPP) Certificate
	ATIDRM_CERT_FMT_X509				= 0x0002,	// X509 certificate
	ATIDRM_CERT_FMT_MAX 				= 0x0003	
};

// Key Encryption shemes for ATIDRM_SESSIONKEY_PARAM::dwKeyES
enum ATIDRM_CERTKeyES
{
	ATIDRM_CERT_ES_NONE					= 0x0000,   // Unknown scheme
	ATIDRM_CERT_ES_RSAES_ZERO_PADDING	= 0x0001,	// RSA Zero Padding
	ATIDRM_CERT_ES_MAX					= 0x0002 
};

#endif
