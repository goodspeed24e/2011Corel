#ifndef CryptographicLibraryDataStructures_H
#define CryptographicLibraryDataStructures_H

#include "Crypto_PKCS7DataStructures.h"

typedef unsigned char			byte;
typedef unsigned char			*POINTER;
typedef unsigned char			BYTE;
typedef unsigned short			UINT2;
typedef unsigned long			UINT4;
typedef unsigned __int32                unint32;
typedef unsigned __int64                unint64;
typedef unsigned long			DWORD;
typedef byte					data_128_bit [16];
typedef byte					PrivateKey[20];
typedef byte					PublicKey[40];
typedef byte					Signature[40];
typedef byte					TransKeyBlcok176Byte [176];  
typedef byte					TransKeyBlcok256Byte [256];  
typedef data_128_bit			BusKey; 

#define SHA1_LEN_BYTES 20
#define SHA256_LEN_BYTES 32
#define SHA512_LEN_BYTES 64

//***********************
// RSA kind define
//***********************
enum E_RSA{RSA_Padding=1, RSA_OAEP_Padding=2};

#define MIN_RSA_MODULUS_BITS 508
#define MAX_RSA_MODULUS_BITS 4096
#define MAX_RSA_MODULUS_LEN ((MAX_RSA_MODULUS_BITS + 7) / 8)
#define MAX_RSA_PRIME_BITS ((MAX_RSA_MODULUS_BITS + 1) / 2)
#define MAX_RSA_PRIME_LEN ((MAX_RSA_PRIME_BITS + 7) / 8)

//***********************
// SHA kind define
//***********************
enum E_SHA{SHA_1=1, SHA256=256, SHA512=512};


//**********************************************************
//    RSA PUBLIC Key 
//        Using to encrypt / authentication message
//**********************************************************
typedef struct R_RSA_PUBLIC_KEY
{
	unsigned short int RSA_ModulusBit;
	unsigned char RSA_Modulus[MAX_RSA_MODULUS_LEN];
	unsigned char RSA_publicExponent[MAX_RSA_MODULUS_LEN];
} R_RSA_PUBLIC_KEY;

//************************************************************
//    RSA Private Key
//        Using to decrypt / signature message
//************************************************************
typedef struct R_RSA_PRIVATE_KEY
{
	unsigned short int RSA_ModulusBit;
	unsigned char RSA_Modulus[MAX_RSA_MODULUS_LEN];
	unsigned char RSA_publicExponent[MAX_RSA_MODULUS_LEN];
	unsigned char RSA_privateExponent[MAX_RSA_MODULUS_LEN];
	unsigned char RSA_Prime[2][MAX_RSA_PRIME_LEN];
	unsigned char RSA_primeCRT[2][MAX_RSA_PRIME_LEN];
	unsigned char RSA_coefficientCRT[MAX_RSA_PRIME_LEN];
} R_RSA_PRIVATE_KEY;

//********************************************
//             RC4 structure
//
//     Using to encrypt / decrypt message
//
//********************************************
typedef struct
{
	unsigned int x;
	unsigned int y;
	unsigned char S[256];
} RC4;

#endif