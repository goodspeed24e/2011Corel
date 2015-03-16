//---------------------------------------------------------------------------
// nvencrypt.h
//
// This file contains the DSHOW specific definitions of the external interfaces
// to the Nvidia DXVA IDCT encryption interface
//
// Copyright (c) 2001 Nvidia, Inc.
//---------------------------------------------------------------------------

#if !defined(NVENCRYPT_H)
#define NVENCRYPT_H

// Nvidia Encryption GUID for DXVA IDCT
DEFINE_GUID(Nvidia_DXVA_IDCT_Encryption, 0x4b2e068f, 0x396a, 0x43df, 0xb8, 0x6b, 0x7d, 0xc6, 0xd2, 0xb6, 0x72, 0x6a);

// Nvidia Encryption GUID for Cipher DMA (AES-128)
DEFINE_GUID(Nvidia_DXVA_AES128_Encryption, 0x2724ac70, 0x88f1, 0x42d9, 0x98, 0xde, 0x31, 0x1a, 0x4b, 0xde, 0x5f, 0xff);

// Nvidia Encryption GUID for Lightweight Scrambling
DEFINE_GUID(Nvidia_DXVA_Scrambling_Encryption, 0x312b6a3a, 0x41de, 0x4a43, 0x9c, 0xf, 0xfb, 0x3d, 0x5f, 0xd, 0xfb, 0x9a);

// dwFunction's
enum {
        NV_ENCRYPT_GET_GUID = 1,
        NV_ENCRYPT_AUTHENTICATE,
        NV_ENCRYPT_ENABLE,
        NV_ENCRYPT_DISABLE,
        NV_ENCRYPT_FRAMEKEY,
        NV_ENCRYPT_SETAESKEY,
        NV_ENCRYPT_SETCIPHERCFG,
        NV_ENCRYPT_SET_PROTECTED_KEY,
        NV_ENCRYPT_SET_PROTECTED_IV,
};

#define NV_ENCRYPT_SETSCRAMBLINGKEY     NV_ENCRYPT_SET_PROTECTED_KEY
#define NV_ENCRYPT_SETSCRAMBLINGIV      NV_ENCRYPT_SET_PROTECTED_IV

// types for authenticate and framekey functions.
typedef struct {
        DXVA_EncryptProtocolHeader      EncryptHeader;
        DWORD  dwH2MKey;
        DWORD dwM2HKey;
} NV_EncryptAuthenticate;

typedef struct {
        DXVA_EncryptProtocolHeader      EncryptHeader;
        WORD  wFrameKey;
} NV_EncryptFrameKey;

// AES-128 modes
enum {
        NV_ENCRYPTMODE_ECB = 1,
        NV_ENCRYPTMODE_CTR32,
        NV_ENCRYPTMODE_CTR64,
        NV_ENCRYPTMODE_CBC
};
// 
// // types for AES-128 cipher config and mode data.
// typedef struct {
//         DXVA_EncryptProtocolHeader      EncryptHeader;
//         DWORD dwEncryptMode;
//         DWORD dwIV[4];
// } NV_EncryptCipherConfig;
// 
// typedef struct {
//         DXVA_EncryptProtocolHeader      EncryptHeader;
//         DWORD dwKey[4];
// } NV_EncryptSetKey;
// 

// enums and types for G7x scrambling methods
enum {
        NV_ENCRYPTKEYPROTECTION_NONE = 0,
        NV_ENCRYPTKEYPROTECTION_COPP,
};

typedef struct {
        DXVA_EncryptProtocolHeader      EncryptHeader;
        DWORD dwEncryptKeyProtectionMode;
        DWORD dwEncryptKeyProtectionIdentifier;
        DWORD dwKey[4];
} NV_EncryptSetProtectedKey;

typedef struct {
        GUID  guidEncryptProtocol;
        DWORD dwEncryptKeyProtectionMode;
        DWORD dwEncryptKeyProtectionIdentifier;
        DWORD dwKey[4];
        DWORD dwIV[4];
} NV_EncryptScramblingProperties;

/*
//
// Decoder calls driver to get encryption guid for device
//

        EncryptFunc.EncryptProtocolFlag = 0xFFFF00;
        EncryptFunc.bDXVA_Func = 1;

        EncryptHeader.dwFunction = NV_ENCRYPT_GET_GUID;

        Execute(EncryptFunc, &EncryptHeader, sizeof(EncryptHeader), &EncryptResult, sizeof(EncryptHeader), 0, NULL);

        // Driver should return encrypt guid in
        EncryptResult.guidEncryptProtocol


//
// Decoder calls drivers to pass host key, get h/w key
//

        EncryptFunc.EncryptProtocolFlag = 0xFFFF00;
        EncryptFunc.bDXVA_Func = 1;

        EncryptAuthenticate.dwFunction = NV_ENCRYPT_AUTHENTICATE;
        EncryptAuthenticate.wH2MKey = whatever;

        Execute(EncryptFunc, &EncryptAuthenticate, sizeof(EncryptAuthenticate), &EncryptResult, sizeof(NV_EncryptAuthenticate), 0, NULL);

        // Driver should return h/w key if decoder key valid
        EncryptResult.dwM2HKey


//
// Decoder calls drivers to pass frame key
//

        EncryptFunc.EncryptProtocolFlag = 0xFFFF00;
        EncryptFunc.bDXVA_Func = 1;

        EncryptFrameKey.dwFunction = NV_ENCRYPT_FRAMEKEY;
        EncryptFrameKey.wFrameKey = whatever;

        Execute(EncryptFunc, &EncryptFrameKey, sizeof(EncryptFrameKey), &EncryptResult, sizeof(EncryptHeader), 0, NULL);

//
// Decoder calls drivers to enable encryption
//

        EncryptFunc.EncryptProtocolFlag = 0xFFFF00;
        EncryptFunc.bDXVA_Func = 1;

        EncryptHeader.dwFunction = NV_ENCRYPT_ENABLE;

        Execute(EncryptFunc, &EncryptHeader, sizeof(EncryptHeader), &EncryptResult, sizeof(EncryptHeader), 0, NULL);


*/

#endif // NVENCRYPT_H
