//=============================================================================
//  THIS SOURCE CODE IS PROPRIETARY INFORMATION BELONGING TO INTERVIDEO, INC.
//  ANY USE INCLUDING BUT NOT LIMITED TO COPYING OF CODE, CONCEPTS, AND/OR
//  ALGORITHMS IS PROHIBITED EXCEPT WITH EXPRESS WRITTEN PERMISSION BY THE
//  COMPANY.
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
//  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
//  PURPOSE.
//
//  Copyright (c) 1998 - 2006  InterVideo Corporation.  All Rights Reserved.
//
//-----------------------------------------------------------------------------

//.	CryptoGraphicLibrary.h
//	Sonny Wang
//	2005/07/22
///////////////////////////////////////////////////////////////////////////////////

#ifndef _CRYPTOGRAPHICLIBRARY_H
#define _CRYPTOGRAPHICLIBRARY_H

#include "Crypto_DataStructures.h"
#include "Crypto_CommonFunctions.h"

class PreCryptographicLibrary;
class CommonFunction;

typedef void (*BlockCipher)(const BYTE *, BYTE *, const BYTE *);
typedef void (*BlockKeyExpansion)(const BYTE *, BYTE *);
class CryptographicLibrary
{
	// ============================================================================================= //
	//                                          Attributes                                           //
	// ============================================================================================= //
public:

private:
	PreCryptographicLibrary* m_pPreCrypto;
	CommonFunction* m_pCommonFunction;
	bool m_bSupportAESNI;
	BlockCipher AES128Encryption;
	BlockCipher AES128Decryption;
	BlockKeyExpansion KeyExpansionForEnc;
	BlockKeyExpansion KeyExpansionForDec;

	// ============================================================================================= //
	//                                          Operations                                           //
	// ============================================================================================= //
public:
	CryptographicLibrary();
	~CryptographicLibrary();


	//AES cypher - NIST, FIPS Publication 197
	//ECB Mode - NIST, Special Publication 800-38A
	void _cdecl AES_128E( const data_128_bit key, const data_128_bit data, data_128_bit result );	
	void _cdecl AES_128D( const data_128_bit key, const data_128_bit data, data_128_bit result );

	void _cdecl AES_128E_AACS_TWOWAY( const data_128_bit key, const data_128_bit data, data_128_bit result );

	// added for transforming MKB and SKB
	/* using ..\CPL\Source\LibSDH\UseNewAACSkey\aes_MMXoptimized.h 
	"TransformAesKeyForEncryption" and "TransformAesKeyForDecryption" functions to generate encryption and decryption key
	*/
	void _cdecl AES_128E_Transformed( const TransKeyBlcok176Byte key, const data_128_bit data, data_128_bit result );	
	void _cdecl AES_128D_Transformed( const TransKeyBlcok176Byte key, const data_128_bit data, data_128_bit result );	

	//CTR Mode - NIST, Special Publication 800-38A
	void _cdecl AES_128CTR( const data_128_bit key,		//128-bit encryption key
		const byte* pFrameData,		//a collection of plaintext/ciphertext 128-bit blocks
		byte* pResult,				//output encrypted/decrypted frame
		DWORD	nSize,				//# of bytes of pFrameData
		const data_128_bit IV    	//128-bit initialization vector
		);
	void _cdecl AES_128CTR_PAVP( const data_128_bit key,		//128-bit encryption key
		const byte* pFrameData,		//a collection of plaintext/ciphertext 128-bit blocks
		byte* pResult,				//output encrypted/decrypted frame
		DWORD	nSize,					//# of bytes of pFrameData
		DWORD   IV	//128-bit initialization vector
		);

	void _cdecl AES_128CTR_GPUCP( const data_128_bit key,		//128-bit encryption key
		const byte* pFrameData,		//a collection of plaintext/ciphertext 128-bit blocks
		byte* pResult,				//output encrypted/decrypted frame
		DWORD	nSize,				//# of bytes of pFrameData
		const data_128_bit IV    	//128-bit initialization vector
		);

	//CBC Mode - NIST, Special Publication 800-38A
	//Let input and output use the same pointer (input and output points to the same place(plaintext)), encrypted data will cover plaintext 
	void _cdecl AES_128CBCE( const data_128_bit key,		//128-bit encryption key
		const byte* pFrameData,		//a collection of plaintext 128-bit blocks
		byte* pResult,				//output encrypted frame
		int	nSize,					//# of bytes of pFrameData
		const data_128_bit IV 	    //128-bit initialization vector
		);

	void _cdecl AES_128CBCD( const data_128_bit key,
		const byte* pEncryptedFrameData,
		byte* pResult,				//output encrypted frame
		int	nSize,					//# of bytes of pFrameData
		const data_128_bit IV 	    //128-bit initialization vector
		);																	

	//SHA-1 algorithm - FIPS 180-2																
	void _cdecl SHA( const byte* input, unsigned long messageDigest[5], int nInputLengthInByte );
	void _cdecl SHA( const byte* input, byte* messageDigest, unint64 nInputLengthInByte, E_SHA SHAKind=SHA_1);
	//note that the unsigned int is little endian, be aware of that when comparing int with bytes[]

	//CMAC based on NIST SP800-38B draft (using transformed 176-byte key)
	void _cdecl CMAC_128( const data_128_bit key,
		const byte*		B,		//the input message
		unsigned int MlenBits,	//number of bits in message
		data_128_bit T );		//output MAC

	//CMAC_OMAC1_128 based on NIST SP800-38B Specification
	void _cdecl CMAC_OMAC1_128( const BYTE *Key, //[input]:  128-bit
		const BYTE *data,           //[input]:  datasize bytes
		int datasize,               //[input]:  integer(byte)
		BYTE *signature);           //[output]: 16 bytes

	//OMAC1 - NIST, Special Publication 800-38B
	bool _cdecl OMAC1_128(const BYTE *key,  //[input]:  16 bytes
		const BYTE *data, //[input]:  datasize bytes
		int datasize,     //[input]:  integer
		BYTE *signature); //[output]: 16 bytes

	//the pseudo-random number generator defined in x9.31 
	void _cdecl RNG_ANSI_X931_128( const data_128_bit key, data_128_bit R);

	void _cdecl ECDSA_Sign( const PrivateKey privateKey, const byte* pDataToBeSigned, int nDataLength, Signature signature, const int CurveChosen = 0 );
	bool _cdecl ECDSA_Verify( const PublicKey publicKey, const byte* pDataToBeVerified, int nDataLength, const Signature signature, const int CurveChosen = 0 );

	// added for transforming ECDSA
	// find Hamer to generate ECDSA transformed private key and public key
	void _cdecl ECDSA_SignTransformed( const PrivateKey privateKey, const byte* pDataToBeSigned, int nDataLength, Signature signature, const int CurveChosen = 0 );
	bool _cdecl ECDSA_VerifyTransformed( const PublicKey publicKey, const byte* pDataToBeVerified, int nDataLength, const Signature signature, const int CurveChosen = 0 );

	//DTCP-ECDH based on IEEE 1363-2000
	void _cdecl DTCP_GetFirstPhaseValue( const PrivateKey privateKey, //[output] OurFirstPhase private key
		const PublicKey publicKey,   //[output] OurFirstPhase public key								
		const int CurveChosen = 2 );	
	bool _cdecl DTCP_GetSharedSecret( const PrivateKey privateKey,  // [input] OurFirstPhase private key Xk (160-bit)
		const PublicKey publicKey,    // [input] TheirFirstPhase public key Yv (320-bit)
		byte* SharedKey,			    // [output] x-coordinate of Xk*Yv (the least significant 96-bit of shared data)
		const int CurveChosen = 2);

	// RSA key generation, encryption, decryption
	bool _cdecl RSA_GenerateKey(R_RSA_PUBLIC_KEY *OutputPublickey, R_RSA_PRIVATE_KEY *OutputPrivatekey, unsigned short int InputKeyBits);
	bool _cdecl RSA_Encryption(unsigned char *Output, unsigned int *OutputLen, unsigned char *Input, unsigned int InputLen, R_RSA_PUBLIC_KEY *InputPublickey, E_RSA Padding=RSA_Padding, E_SHA SHAKind=SHA_1);
	bool _cdecl RSA_Decryption(unsigned char *Output, unsigned int *OutputLen, unsigned char *Input, unsigned int InputLen, R_RSA_PRIVATE_KEY *InputPrivatekey, E_RSA Padding=RSA_Padding, E_SHA SHAKind=SHA_1);
	bool _cdecl RSA_SIGNATURE(unsigned char *Output, unsigned int *OutputLen, unsigned char *Input, unsigned int InputLen, R_RSA_PRIVATE_KEY *InputPrivatekey, E_SHA SHAKind=SHA_1);
	//Input_Content_Hash = 0 (Input_Content is not a hash value), Input_Content_Hash = 1 (Input_Content is a hash value)
	bool _cdecl RSA_VERIFICATION(unsigned char *Input_Content, unsigned int Input_Content_Len, unsigned char *Input_Sign, unsigned int Input_SignLen, R_RSA_PUBLIC_KEY *InputPublickey, int Input_Content_Hash = 0, E_SHA SHAKind=SHA_1);

	//RC4 encryption and decryption
	bool RC4_StreamCipher(const unsigned char* input, //the plaintext/ciphertext
		unsigned char* output,      //the plaintext/ciphertext
		unsigned int nSize,         //the plaintext and ciphertext size
		unsigned char* key,         //variable key 
		unsigned int keysize);      //key size (1byte <= keysize <= 256byte)

	// added for Signature verify
	bool _cdecl HDCP_VERIFY( byte* Messg, unsigned int MessgLen,
		byte* rSignature, unsigned int rSignatureLen,
		byte* sSignature, unsigned int sSignatureLen);

	/* PKCS7 operation */
	//Get certificate chain in PKCS7 structure
	PKCS7_st* _cdecl GetPKCS7_CertificateChain(unsigned char *Input, int InputLen);
	//Validate the certificate chain up to the trusted root
	long _cdecl PKCS7_CertChain_CHECK(PKCS7_st* p7);
	//Check extended key usage(EKU) of the certificate
	long _cdecl PKCS7_EKU_CHECK(PKCS7_st* p7, const char* EKU);
	//Check the GRL integrity
	long _cdecl PKCS7_GRL_Integrity_CHECK(PKCS7_st* p7, unsigned char* Input_HeaderCore, int Input_HeaderCoreLen);
	//Check each certificate is or not in revocation list
	long _cdecl PKCS7_Revocation_List_CHECK(PKCS7_st *p7, unsigned char* GRL, int GRL_Num);
	//Get leaf certificate pubkey(ASN.1)and transform into RSA public key structure
	void _cdecl PKCS7_Get_LeafCert_PubKey(PKCS7_st *p7, R_RSA_PUBLIC_KEY &Output_pubKey);
	//Free PKCS7 structure memory
	void _cdecl PKCS7_FREE(PKCS7_st *p7);

	//Change: Temporary mark
	/*
	// key expansion to 176 bytes
	void _cdecl KeyExpansion(const BYTE *cipherkey,		// input:  16     bytes
	BYTE *KeySchedule);	// output: 4*4*11 bytes
	*/
	/*void _cdecl CMAC_128_VM( const OBAES *myaes,
	const byte*		B,		//the input message
	unsigned int MlenBits,	//number of bits in message
	data_128_bit T );		//output MAC
	*/



	//bool _cdecl COPP_VERIFY(byte* Pub, unsigned int PubLen, byte* Mod, unsigned int ModLen, byte* Sig, unsigned int SigLen, byte* Messg, unsigned int MessgLen);	
};

#define DATA_MASKING_16BYTES(p) \
{ \
	((DWORD *)(p))[0] ^= 0xcdcdcdcd; \
	((DWORD *)(p))[1] ^= 0xcdcdcdcd; \
	((DWORD *)(p))[2] ^= 0xcdcdcdcd; \
	((DWORD *)(p))[3] ^= 0xcdcdcdcd; \
}

#define DATA_DEMASKING_16BYTES(p) \
{ \
	((DWORD *)(p))[0] ^= 0xcdcdcdcd; \
	((DWORD *)(p))[1] ^= 0xcdcdcdcd; \
	((DWORD *)(p))[2] ^= 0xcdcdcdcd; \
	((DWORD *)(p))[3] ^= 0xcdcdcdcd; \
}

#define DATA_MASKING_MULTI_16BYTES(pin, pout, numofbytes) \
	for(DWORD *p=(DWORD *)(pin), *p2=(DWORD *)(pout); p < (DWORD *)(pin)+(numofbytes)/4; p+=4, p2+=4) \
{ \
	p2[0] = (p[0] ^ 0xcdcdcdcd); \
	p2[1] = (p[1] ^ 0xcdcdcdcd); \
	p2[2] = (p[2] ^ 0xcdcdcdcd); \
	p2[3] = (p[3] ^ 0xcdcdcdcd); \
}

#define DATA_MASKING_MULTI_BYTES(pin, pout, numofbytes) \
{ \
	DWORD *p, *p2; \
	BYTE *pb, *pb2; \
	for(p=(DWORD *)(pin), p2=(DWORD *)(pout); p < (DWORD *)(pin)+(numofbytes)/4; p++,p2++) \
	*p2 = *p ^ 0xcdcdcdcd; \
	for(pb=(BYTE *)p, pb2=(BYTE *)p2; pb < (BYTE *)(pin)+(numofbytes); pb++,pb2++) \
	*pb2 = *pb ^ 0xcd; \
}

#define DATA_DEMASKING_MULTI_BYTES(pin, pout, numofbytes) DATA_MASKING_MULTI_BYTES(pin, pout, numofbytes)

extern byte   GeKey92[16], GeKey93[16], GeKey94[16], GeKey95[16], GeKey96[16], GeKey97[16];

//Key_Mask for AACS AV stream
#define KEY_FOR_MASK_16BYTES(KeyMask)\
	for(int j=0;j<16;j++) \
{ \
	KeyMask[j] = GeKey92[j]^GeKey93[j]^GeKey96[j]^GeKey97[j]; \
}

//Mask AACS AV stream Key_Mask
#define MASK_KEY_16BYTES(tempKeyMask) \
{ \
	DWORD *t_pdwKey=(DWORD*)tempKeyMask; \
	KEY_FOR_MASK_16BYTES(tempKeyMask);\
	t_pdwKey[3]^=t_pdwKey[2]; \
	t_pdwKey[2]^=t_pdwKey[1]; \
	t_pdwKey[1]^=t_pdwKey[0]; \
	t_pdwKey[0]^=t_pdwKey[3]; \
}

#endif
