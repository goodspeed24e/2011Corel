#pragma once

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef char			int8;
typedef unsigned char	uint8;
typedef short			int16;
typedef unsigned short	uint16;
typedef int				int32;
typedef unsigned int	uint32;

#define PAVP_EPID_API_VERSION_MAJOR    1
#define PAVP_EPID_API_VERSION_MINOR    5
#define PAVP_EPID_API_VERSION          ((PAVP_EPID_API_VERSION_MAJOR << 16) | (PAVP_EPID_API_VERSION_MINOR))
#define ECDSA_PRIVKEY_LEN				32
#define ECDSA_PUBKEY_LEN				64
#define ECDSA_SIGNATURE_LEN				64
#define EPID_PARAM_LEN                 780        // EPID cryptosystem context length
#define EPID_CERT_LEN                  392        // EPID certificate length
#define EPID_PUBKEY_LEN                328        // EPID cert length - EC-DSA sig length
#define EPID_SIG_LEN                   569        // EPID signature length
#define SIGMA_PUBLIC_KEY_LEN			64
#define SIGMA_SESSION_KEY_LEN			16
#define SIGMA_MAC_KEY_LEN				16
#define SIGMA_MAC_LEN					32
#define PAVP_EPID_STREAM_KEY_LEN       16
#define PAVP_EPID_PUBCERT3P_TYPE_PAVP  0x00000000
#define PAVP_EPID_PUBCERT3P_ISSUER_ID  0x00000000

// PAVP EPID Commands:
#define CMD_GET_HW_ECC_PUBKEY	0x00000003
#define CMD_EXCHG_HW_APP_CERT	0x00000004
#define CMD_CLOSE_SIGMA_SESSION	0x00000005
#define CMD_GET_STREAM_KEY		0x00000006
#define CMD_INV_STREAM_KEY		0x00000007

typedef unsigned char EcDsaPrivKey[ECDSA_PRIVKEY_LEN];
typedef unsigned char EcDsaPubKey[ECDSA_PUBKEY_LEN];
typedef unsigned char EcDsaSig[ECDSA_SIGNATURE_LEN];
typedef unsigned char EpidSig[EPID_SIG_LEN];
typedef unsigned char SigmaPubKey[SIGMA_PUBLIC_KEY_LEN];
typedef unsigned char SigmaSessionKey[SIGMA_SESSION_KEY_LEN];
typedef unsigned char SigmaMacKey[SIGMA_MAC_KEY_LEN];
typedef unsigned char HMAC[SIGMA_MAC_LEN];
typedef uint8 StreamKey[PAVP_EPID_STREAM_KEY_LEN];
typedef uint32 PAVPStreamId;
typedef uint32 PAVPSessId;

typedef enum 
{
	PAVP_VIDEO_PATH = 0,
	PAVP_AUDIO_PATH
} PAVPPathId;

// PAVP EPID Status Codes
typedef enum
{
	PAVP_STATUS_SUCCESS							= 0x0000,
	PAVP_STATUS_INTERNAL_ERROR					= 0x1000,
	PAVP_STATUS_UNKNOWN_ERROR					= 0x1001,
	PAVP_STATUS_INCORRECT_API_VERSION			= 0x1002,
	PAVP_STATUS_INVALID_FUNCTION				= 0x1003,
	PAVP_STATUS_INVALID_BUFFER_LENGTH			= 0x1004,
	PAVP_STATUS_INVALID_PARAMS					= 0x1005,
    PAVP_STATUS_EPID_INVALID_PUBKEY             = 0x3000,
	PAVP_STATUS_SIGMA_SESSION_LIMIT_REACHED		= 0x3002,
	PAVP_STATUS_SIGMA_SESSION_INVALID_HANDLE	= 0x3003,
	PAVP_STATUS_SIGMA_PUBKEY_GENERATION_FAILED	= 0x3004,
	PAVP_STATUS_SIGMA_INVALID_3PCERT_HMAC		= 0x3005,
	PAVP_STATUS_SIGMA_INVALID_SIG_INTEL			= 0x3006,
	PAVP_STATUS_SIGMA_INVALID_SIG_CERT			= 0x3007,
	PAVP_STATUS_SIGMA_EXPIRED_3PCERT			= 0x3008,
	PAVP_STATUS_SIGMA_INVALID_SIG_GAGB			= 0x3009,
    PAVP_STATUS_PAVP_EPID_INVALID_PATH_ID       = 0x4000,
    PAVP_STATUS_PAVP_EPID_INVALID_STREAM_ID     = 0x4001,
    PAVP_STATUS_PAVP_EPID_STREAM_SLOT_OWNED     = 0x4002,
    // PAVP Device Status Codes
    // Internal errors:
    PAVP_STATUS_BAD_POINTER                     = 0x5000,
    PAVP_STATUS_NOT_SUPPORTED                   = 0x5001,   // PAVP is not supported
    PAVP_STATUS_CRYPTO_DATA_GEN_ERROR           = 0x5002,   // An error happened in the Crypto Data Gen lib
    // Device state errors:
    PAVP_STATUS_PAVP_DEVICE_NOT_INITIALIZED     = 0x6000,   // Cannot perform this function without first initializing the device
    PAVP_STATUS_PAVP_SERVICE_NOT_CREATED        = 0x6001,   // Cannot perform this function without first creating PAVP service
    PAVP_STATUS_PAVP_KEY_NOT_EXCHANGED          = 0x6002,   // Cannot perform this function without first doing a key exchange
    // Key exchange protocol errors:
    PAVP_STATUS_KEY_EXCHANGE_TYPE_NOT_SUPPORTED = 0x7000,   // An invalid key exchange type was used
    PAVP_STATUS_PAVP_SERVICE_CREATE_ERROR       = 0x7001,   // A driver error occured when creating the PAVP service
    PAVP_STATUS_GET_PUBKEY_FAILED               = 0x7002,   // Failed to get g^a from PCH but no detailed error was given
    PAVP_STATUS_DERIVE_SIGMA_KEYS_FAILED        = 0x7003,   // Sigma keys could not be derived
    PAVP_STATUS_CERTIFICATE_EXCHANGE_FAILED     = 0x7004,   // Could not exchange certificates with the PCH but no detailed error was given
    PAVP_STATUS_PCH_HMAC_INVALID                = 0x7005,   // The PCH's HMAC was invalid
    PAVP_STATUS_PCH_CERTIFICATE_INVALID         = 0x7006,   // The PCH's certificate was not valid
    PAVP_STATUS_PCH_EPID_SIGNATURE_INVALID      = 0x7007,   // The PCH's EPID signature was invalid
    PAVP_STATUS_GET_STREAM_KEY_FAILED           = 0x7008,   // Failed to get a stream key from the PCH but no detailed error was given
    PAVP_STATUS_GET_CONNSTATE_FAILED            = 0x7009,   // Failed to get a connection state value from the hardware
    PAVP_STATUS_GET_CAPS_FAILED                 = 0x7010,   // Failed to get PAVP capabilities from the driver
    PAVP_STATUS_GET_FRESHNESS_FAILED            = 0x7011,   // Failed to get a key freshness value from the hardware
    PAVP_STATUS_SET_FRESHNESS_FAILED            = 0x7012,   // Failed to set the key freshness value
    PAVP_STATUS_SET_ENCRYPTION_MODE_FAILED      = 0x7013,   // Failed to set the encryption mode
    PAVP_STATUS_SET_PROTECTED_MEM_FAILED        = 0x7014,   // Failed to set protected memory on/off
    PAVP_STATUS_SET_PLANE_ENABLE_FAILED         = 0x7015,   // Failed to set plane enables
    PAVP_STATUS_SET_WINDOW_POSITION_FAILED      = 0x7016,   // Failed to set the window position
    PAVP_STATUS_AES_DECRYPT_FAILED              = 0x7017,   // Failed to decrypt  
    PAVP_STATUS_AES_ENCRYPT_FAILED              = 0x7018,   // Failed to encrypt  
    PAVP_STATUS_LEGACY_KEY_EXCHANGE_FAILED      = 0x7019    // Legacy key exchange call failed
} PavpEpidStatus;


#define PAVP_EPID_SUCCESS(Status) (((PavpEpidStatus)(Status)) == PAVP_STATUS_SUCCESS)
#define PAVP_EPID_FAILURE(Status) (((PavpEpidStatus)(Status)) != PAVP_STATUS_SUCCESS)


#pragma pack(push)
#pragma pack(1)

/*
**	3.1.1.1.3:	3rd Party Certificate
*/
// Type of the certificate: PAVP
#define PAVP_EPID_PUBCERT3P_TYPE_PAVP        0x00000000
// Issuer id: Intel
#define PAVP_EPID_PUBCERT3P_ISSUER_ID        0x00000000

typedef struct _Cert3p 
{
	// 3rd Party signed part
	struct _SignBy3p 
	{
		unsigned int	CertificateType;
		unsigned char	TimeValidStart[8];
		unsigned char	TimeValidEnd[8];
		unsigned int	Id3p;
		unsigned int	IssuerId;
		EcDsaPubKey		PubKey3p;
	} SignBy3p;
	EcDsaSig			Sign3p;
	// Intel signed part
	struct _SignByIntel 
	{
		unsigned char	TimeValidStart[8];
		unsigned char	TimeValidEnd[8];
		EcDsaPubKey		PubKeyVerify3p;
	} SignByIntel;
	EcDsaSig			SignIntel;
} Cert3p;

/*
**    3.1.1.2.1    EPID Certificate
*/
typedef struct _EpidCert 
{
    unsigned char   PubKeyEpid[EPID_PUBKEY_LEN];
	EcDsaSig		SignIntel;
} EpidCert;

/*
**	3.2.4		Input/output message buffer common header
*/
typedef struct _PAVPCmdHdr 
{
	uint32	ApiVersion;
	uint32	CommandId;
	uint32	Status;
	uint32	BufferLength;
} PAVPCmdHdr;

/*
**	3.2.7.1.1	Command: CMD_GET_HW_ECC_PUBKEY
**
**	InBuffer:
*/
typedef struct _GetHwEccPubKeyInBuff 
{
	PAVPCmdHdr	Header;
} GetHwEccPubKeyInBuff;
/*
**	OutBuffer:
*/
typedef struct _GetHwEccPubKeyOutBuff 
{
	PAVPCmdHdr	Header;
	PAVPSessId	SigmaSessionId;
	SigmaPubKey	Ga;
} GetHwEccPubKeyOutBuff;

/*
**	3.2.7.1.2	Command: CMD_EXCHG_HW_APP_CERT
**
**	InBuffer:
*/
typedef struct _HwAppExchgCertsInBuff 
{
	PAVPCmdHdr	Header;
	PAVPSessId	SigmaSessionId;
	SigmaPubKey	Gb;
	Cert3p		Certificate3p;
	HMAC		Certificate3pHmac;
	EcDsaSig	EcDsaSigGaGb;
} HwAppExchgCertsInBuff;
/*
**	OutBuffer:
*/
typedef struct _HwAppExchgCertsOutBuff 
{
	PAVPCmdHdr	Header;
	PAVPSessId	SigmaSessionId;
    EpidCert    CertificatePch;
	HMAC		CertificatePchHmac;
    EpidSig     EpidSigGaGb;
} HwAppExchgCertsOutBuff;

/*
**	3.2.7.1.3	Command: CMD_CLOSE_SIGMA_SESSION
**
**	InBuffer:
*/
typedef struct _CloseSigmaSessionInBuff 
{
	PAVPCmdHdr	Header;
	PAVPSessId	SigmaSessionId;
} CloseSigmaSessionInBuff;
/*
**	OutBuffer:
*/
typedef struct _CloseSigmaSessionOutBuff
{
	PAVPCmdHdr	Header;
	PAVPSessId	SigmaSessionId;
} CloseSigmaSessionOutBuff;

/*
**	3.2.7.2.1	Command: CMD_GET_STREAM_KEY
**
**	InBuffer:
*/
typedef struct _GetStreamKeyInBuff
{
	PAVPCmdHdr		Header;
	PAVPSessId		SigmaSessionId;
	PAVPStreamId	StreamId;
	PAVPPathId		MediaPathId;
} GetStreamKeyInBuff;
/*
**	OutBuffer:
*/
typedef struct _GetStreamKeyOutBuff
{
	PAVPCmdHdr	Header;
	PAVPSessId	SigmaSessionId;
	StreamKey	WrappedStreamKey;
	HMAC		WrappedStreamKeyHmac;
} GetStreamKeyOutBuff;

/*
**	3.2.7.2.2	Command: CMD_INV_STREAM_KEY
**
**	InBuffer:
*/
typedef struct _InvStreamKeyInBuff
{
	PAVPCmdHdr		Header;
	PAVPSessId		SigmaSessionId;
	PAVPStreamId	StreamId;
	PAVPPathId		MediaPathId;
} InvStreamKeyInBuff;
/*
**	OutBuffer:
*/
typedef struct _InvStreamKeyOutBuff
{
	PAVPCmdHdr		Header;
	PAVPSessId		SigmaSessionId;
} InvStreamKeyOutBuff;


/* 
**	GUID defining the Intel PAVP Device extension
**	See PAVP API/DDI design for more information
**	{7460004-7533-4E1A-B8E3-FF206BF5CE47}
*/
//const GUID DXVA2_Intel_Pavp = 
//{ 0x7460004, 0x7533, 0x4e1a, { 0xbd, 0xe3, 0xff, 0x20, 0x6b, 0xf5, 0xce, 0x47 } };
//Defined under ..\CPServiceIntelPAVP.cpp 

typedef enum
{
    PAVP_MEMORY_PROTECTION_NONE     = 0,
    PAVP_MEMORY_PROTECTION_LITE     = 1,
    PAVP_MEMORY_PROTECTION_STATIC   = 2,
    PAVP_MEMORY_PROTECTION_DYNAMIC_IMPLICIT  = 4,
    PAVP_MEMORY_PROTECTION_DYNAMIC_EXPLICIT  = 8
} PAVP_MEMORY_PROTECTION_MASK;

typedef enum
{
    PAVP_STATIC_MEMORY_0M       = 0,
    PAVP_STATIC_MEMORY_96M      = 1,
    PAVP_STATIC_MEMORY_128M     = 2
} PAVP_STATIC_MEMORY_SIZE_MASK;

typedef enum
{
    PAVP_KEY_EXCHANGE_DEFAULT       = 0,
    PAVP_KEY_EXCHANGE_CANTIGA       = 1,
    PAVP_KEY_EXCHANGE_EAGLELAKE     = 2,
    PAVP_KEY_EXCHANGE_LARRABEE      = 4,
    PAVP_KEY_EXCHANGE_IRONLAKE      = 8,
    PAVP_KEY_EXCHANGE_DAA           = 16
} PAVP_KEY_EXCHANGE_MASK;

typedef struct tagPAVP_QUERY_CAPS
{
    UINT AvailableMemoryProtection;     // Indicates supported protection modes
    UINT AvailableKeyExchangeProtocols; // Indicates supported key exchange method
    UINT AvailableStaticMemorySizes;    // Indicates supported allocation sizes
    PAVP_MEMORY_PROTECTION_MASK     eCurrentMemoryMode; // only 1 bit set
    PAVP_STATIC_MEMORY_SIZE_MASK    eCurrentMemorySize; // only 1 bit set
} PAVP_QUERY_CAPS;

// Decoder Extension Function Codes
typedef enum tagPAVP_FUNCTION_ID
{
    PAVP_KEY_EXCHANGE          = 0x200,
    PAVP_USE_PROTECTED_MEMORY  = 0x201,
    PAVP_GET_CONNECTION_STATE  = 0x202,
    PAVP_GET_FRESHNESS         = 0x203,
    PAVP_SET_FRESHNESS         = 0x204,
    PAVP_SET_WINDOW_POSITION   = 0x205,
    PAVP_SET_MEMORY_MODE       = 0x206,
    PAVP_SET_PLANE_ENABLE      = 0x208,
    PAVP_SET_ENCRYPTION_MODE   = 0x209
} PAVP_FUNCTION_ID;

// Device creation parameters
typedef struct tagPAVP_CREATE_DEVICE
{
    PAVP_KEY_EXCHANGE_MASK	eDesiredKeyExchangeMode;  // Only one bit should be set
} PAVP_CREATE_DEVICE, *PPAVP_CREATE_DEVICE;

// Fixed key exchange parameters
typedef struct tagPAVP_FIXED_EXCHANGE_PARAMS
{
    DWORD   FixedKey[4];
    DWORD   SessionKey[4];
} PAVP_FIXED_EXCHANGE_PARAMS;

typedef struct _DXVA_Intel_Pavp_Protocol
{
	DXVA_EncryptProtocolHeader	EncryptProtocolHeader;
	DWORD						dwBufferSize;
	DWORD						dwAesCounter;
}DXVA_Intel_Pavp_Protocol;

// Use PAVP protected memory during allocation struct
typedef struct tagPAVP_USE_PROTECTED_MEMORY_PARAMS
{
    BOOL    bUseProtectedMemory;
} PAVP_USE_PROTECTED_MEMORY_PARAMS;

// PAVP get connection state struct
typedef struct tagPAVP_GET_CONNECTION_STATE_PARAMS
{
	DWORD   Nonce;
    DWORD   ProtectedMemoryStatus[4];
    DWORD   PortStatusType;
    DWORD   DisplayPortStatus[4];
} PAVP_GET_CONNECTION_STATE_PARAMS;

// PAVP get freshness struct
typedef struct tagPAVP_GET_FRESHNESS_PARAMS
{
    DWORD   Freshness[4];
} PAVP_GET_FRESHNESS_PARAMS;

#pragma pack(pop)

#ifdef __cplusplus
}
#endif