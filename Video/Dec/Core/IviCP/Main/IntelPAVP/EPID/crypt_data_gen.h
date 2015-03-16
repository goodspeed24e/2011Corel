#ifndef _CRYPT_DATA_GEN_H
#define _CRYPT_DATA_GEN_H

#include "cdg_status_codes.h"

#ifdef CRYPTODATAGEN_EXPORTS
#define CRYPTODATAGEN_API //__declspec(dllexport)
#else
#define CRYPTODATAGEN_API //__declspec(dllimport)
#endif

/*	Function:
 *		InitailizeCgd
 *	Description:
 *		Initialize the library - set the context and create dummy Intel's key pair
 *	Input arguments:
 *		None
 *	Return values:
 *		Operation status code (see cdg_status.h for details)
 */
CRYPTODATAGEN_API
CdgStatus InitializeCdg();

/*	Function:
 *		GetIntelKeyPair
 *	Description:
 *		Retrieves dummy Intel EC-DSA key pair
 *	Input arguments:
 *		OUT	*PrivKeyIntel		- pointer to the buffer for private key
 *		IN	PrivKeyIntelLen		- private key buffer length
 *		OUT *PubKeyIntel		- pointer to the buffer for public key
 *		IN	PubKeyIntelLen		- public key buffer length
 *	Return values:
 *		Operation status code (see cdg_status.h for details)
 */
CRYPTODATAGEN_API
CdgStatus GetIntelKeyPair(unsigned char*	PrivKeyIntel,
						  int				PrivKeyIntelLen,
						  unsigned char*	PubKeyIntel,
						  int				PubKeyIntelLen);

/*	Function:
 *		GenPrivKeyAndCert3p
 *	Description:
 *		Generate 3rd party private key and certificate
 *	Input arguments:
 *		OUT	*PrivKey3p			- pointer to the buffer for generated private key
 *		IN	PrivKey3pLen		- private key buffer length
 *		OUT *PubCert3p			- pointer to the buffer for generated certificate (type: Cert3p)
 *		IN	PubCert3pLen		- certificate buffer length = sizeof(Cert3p)
 *	Return values:
 *		Operation status code (see cdg_status.h for details)
 */
CRYPTODATAGEN_API
CdgStatus GenPrivKeyAndCert3p(unsigned char*	PrivKey3p,
							  int				PrivKey3pLen,
							  unsigned char*	PubCert3p,
							  int				PubCert3pLen);

/*	Function:
 *		VerifyCert3pSigIntel
 *	Description:
 *		Verify Intel's signature of the 3rd party certificate
 *	Input arguments:
 *		IN	*PubCert3p			- pointer to the buffer containing the certificate (type: Cert3p)
 *		IN	PubCert3pLen		- certificate buffer length = sizeof(Cert3p)
 *		OUT	*VerifRes			- pointer to the verification result
 *	Return values:
 *		Operation status code (see cdg_status.h for details)
 */
CRYPTODATAGEN_API
CdgStatus VerifyCert3pSigIntel(unsigned char*	PubCert3p,
							   int				PubCert3pLen,
							   CdgResult*		VerifRes);

/*	Function:
 *		VerifyCert3pSig3p
 *	Description:
 *		Verify 3rd party's signature of the 3rd party certificate
 *	Input arguments:
 *		IN	*PubCert3p			- pointer to the buffer containing the certificate (type: Cert3p)
 *		IN	PubCert3pLen		- certificate buffer length = sizeof(Cert3p)
 *		OUT	*VerifRes			- pointer to the verification result
 *	Return values:
 *		Operation status code (see cdg_status.h for details)
 */
CRYPTODATAGEN_API
CdgStatus VerifyCert3pSig3p(unsigned char*	PubCert3p,
						    int				PubCert3pLen,
						    CdgResult*		VerifRes);

/*	Function:
 *		MessageSign3p
 *	Description:
 *		Sign the message with 3rd party private key
 *	Input arguments:
 *		IN	*PrivKey3p			- pointer to the buffer containing 3rd party private key
 *		IN	PrivKey3pLen		- private key buffer length
 *		IN	*Message			- pointer to the buffer containing the message to be signed
 *		IN	MessageLen			- message buffer length
 *		OUT	*Signature			- pointer to the buffer for generated signature
 *		IN	SignatureLen		- signature buffer length
 *	Return values:
 *		Operation status code (see cdg_status.h for details)
 */
CRYPTODATAGEN_API
CdgStatus MessageSign3p(unsigned char*	PrivKey3p,
						int				PrivKey3pLen,
						unsigned char*	Message,
						int				MessageLen,
						unsigned char*	Signature,
						int				SignatureLen);

/*	Function:
 *		MessageVerify3p
 *	Description:
 *		Verify the 3rd party signature of the message
 *	Input arguments:
 *		IN	*PubKey3p			- pointer to the buffer containing 3rd party public key
 *		IN	PubKey3pLen			- public key buffer length
 *		IN	*Message			- pointer to the buffer containing the message that was signed
 *		IN	MessageLen			- message buffer length
 *		IN	*Signature			- pointer to the buffer containing the 3rd party signature of the message
 *		IN	SignatureLen		- signature buffer length
 *		OUT	*VerifRes			- pointer to the verification result
 *	Return values:
 *		Operation status code (see cdg_status.h for details)
 */
CRYPTODATAGEN_API
CdgStatus MessageVerify3p(unsigned char*	PubKey3p,
						  int				PubKey3pLen,
						  unsigned char*	Message,
						  int				MessageLen,
						  unsigned char*	Signature,
						  int				SignatureLen,
						  CdgResult*		VerifRes);

/*	Function:
 *		GenCertPch
 *	Description:
 *		Generate PCH SafeID certificate for the given group id
 *	Input arguments:
 *		IN	Gid					- SafeID group id
 *		OUT	*PubCertPch			- pointer to the buffer for generated certificate (type: SafeIdCert)
 *		IN	PubCertPchLen		- certificate buffer length = sizeof(SafeIdCert)
 *	Return values:
 *		Operation status code (see cdg_status.h for details)
 */
CRYPTODATAGEN_API
CdgStatus GenCertPch(int			Gid, 
					 unsigned char*	PubCertPch,
					 int			PubCertPchLen);

/*	Function:
 *		GenPrivKeyPch
 *	Description:
 *		Generate PCH SafeID private key from the current group. This should be called after the GenCertPch function.
 *		Example of use:
 *			Status = InitializeCdg();
 *			...
 *			Status = GenCertPch(0, CertPch0, ...);
 *			Status = GenPrivKeyPch(PrivKeyPch01, ...);	|
 *			Status = GenPrivKeyPch(PrivKeyPch02, ...);	|	Those private keys are related to public key from CertPch0
 *			...											|
 *			Status = GenPrivKeyPch(PrivKeyPch0n, ...);	|
 *
 *			Status = GenCertPch(1, CertPch1, ...);
 *			Status = GenPrivKeyPch(PrivKeyPch11, ...);	|
 *			Status = GenPrivKeyPch(PrivKeyPch12, ...);	|	Those private keys are related to public key from CertPch1
 *			...											|
 *			Status = GenPrivKeyPch(PrivKeyPch1n, ...);	|
 *	Input arguments:
 *		OUT	*PrivKeyPch			- pointer to the buffer for generated private key
 *		IN	PrivKeyPchLen		- private key buffer length
 *	Return values:
 *		Operation status code (see cdg_status.h for details)
 */
CRYPTODATAGEN_API
CdgStatus GenPrivKeyPch(unsigned char*	PrivKeyPch,
						int				PrivKeyPchLen);

/*	Function:
 *		VerifyCertPchSigIntel
 *	Description:
 *		Verify Intel's signature of the PCH SafeID certificate
 *	Input arguments:
 *		IN	*PubCertPch			- pointer to the buffer containing PCH certificate
 *		IN	PubCertPchLen		- certificate buffer length
 *		OUT	*VerifRes			- pointer to the verification result
 *	Return values:
 *		Operation status code (see cdg_status.h for details)
 */
CRYPTODATAGEN_API
CdgStatus VerifyCertPchSigIntel(unsigned char*	PubCertPch,
								int				PubCertPchLen,
								CdgResult*		VerifRes);

/*	Function:
 *		MessageSignPch
 *	Description:
 *		Sign the message with PCH SafeId private key
 *	Input arguments:
 *		IN	*PubKeyPch			- pointer to the buffer containing PCH SafeId public key (this is used to deserialize the private key)
 *		IN	PubKeyPchLen		- public key buffer length
 *		IN	*PrivKeyPch			- pointer to the buffer containing 3rd party private key
 *		IN	PrivKeyPchLen		- private key buffer length
 *		IN	*Message			- pointer to the buffer containing the message to be signed
 *		IN	MessageLen			- message buffer length
 *		IN	*Bsn				- pointer to the buffer containing the base name for the signature
 *		IN	BsnLen				- length of the base name buffer
 *		OUT	*Signature			- pointer to the buffer for generated signature
 *		IN	SignatureLen		- signature buffer length
 *	Return values:
 *		Operation status code (see cdg_status.h for details)
 */
CRYPTODATAGEN_API
CdgStatus MessageSignPch(unsigned char*	PubKeyPch,
						 int			PubKeyPchLen,
						 unsigned char*	PrivKeyPch,
						 int			PrivKeyPchLen,
						 unsigned char* Message,
						 int			MessageLen,
						 unsigned char*	Bsn,
						 int			BsnLen,
						 unsigned char* Signature,
						 int			SignatureLen);

/*	Function:
 *		MessageVerifyPch
 *	Description:
 *		Verify the PCH SafeId signature of the message
 *	Input arguments:
 *		IN	*PubKeyPch			- pointer to the buffer containing PCH SafeId public key (this is used to deserialize the private key)
 *		IN	PubKeyPchLen		- public key buffer length
 *		IN	*Message			- pointer to the buffer containing the message to be signed
 *		IN	MessageLen			- message buffer length
 *		IN	*Bsn				- pointer to the buffer containing the base name for the signature
 *		IN	BsnLen				- length of the base name buffer
 *		IN	*Signature			- pointer to the buffer for generated signature
 *		IN	SignatureLen		- signature buffer length
 *		OUT	*VerifRes			- pointer to the verification result
 *	Return values:
 *		Operation status code (see cdg_status.h for details)
 */
CRYPTODATAGEN_API
CdgStatus MessageVerifyPch(unsigned char*	PubKeyPch,
						   int				PubKeyPchLen,
						   unsigned char*	Message,
						   int				MessageLen,
						   unsigned char*	Bsn,
						   int				BsnLen,
						   unsigned char*	Signature,
						   int				SignatureLen,
						   CdgResult*		VerifRes);

/*	Function:
 *		CreateHmac
 *	Descritpion:
 *		Create HMAC of the message using SHA256 algorithm
 *	Input arguments:
 *		IN	*Message		- pointer to the buffer containing the message
 *		IN	MessageLen		- length of the message buffer
 *		IN	*Mk				- pointer to the buffer containing MAC key
 *		IN	MkLen			- MAC key buffer length
 *		OUT	*Hmac			- pointer to the buffer for generated HMAC of the message
 *		IN	HmacLen			- HMAC buffer length >= 32 bytes
 *	Return values:
 *		Operation status code (see cdg_status.h for details)
 */
CRYPTODATAGEN_API
CdgStatus CreateHmac(unsigned char*		Message,
					 int				MessageLen,
					 unsigned char*		Mk,
					 int				MkLen,
					 unsigned char*		Hmac,
					 int				HmacLen);


/*	Function:
 *		VerifyHmac
 *	Description:
 *		Verify the HMAC of the message using SHA256 algorithm
 *	Input arguments:
 *		IN	*Message			- pointer to the buffer containing the message which HMAC will be verified
 *		IN	MessageLen			- message buffer length
 *		IN	*Hmac				- pointer to the buffer containing the HMAC of the message
 *		IN	HmacLen				- length of the HMAC buffer
  *		IN	*MacKey				- pointer to the buffer containing the MAC key which was used to compute oryginal HMAC
 *		IN	MacKeyLen			- length of the MAC key buffer
 *		OUT	*VerifRes			- pointer to the verification result
 *	Return values:
 *		Operation status code (see cdg_status.h for details)
 */
CRYPTODATAGEN_API
CdgStatus VerifyHmac(unsigned char*	Message,
					 int			MessageLen,
					 unsigned char*	Hmac,
					 int			HmacLen,
					 unsigned char*	MacKey,
					 int			MacKeyLen,
					 CdgResult*		VerifRes);

/*	Function:
 *		GenDhPairPch
 *	Description:
 *		Generate DH (a, g^a) key pair in PCH
 *	Input arguments:
 *		IN	*a					- pioter to the buffer containing serialized a
 *		IN	aLen				- a buffer length
 *		OUT	*Ga					- pointer to the buffer for computed and serialized Ga
 *		IN	GaLen				- Ga buffer length
 *	Return values:
 *		Operation status code (see cdg_status.h for details)
 */
CRYPTODATAGEN_API
CdgStatus GenDhPairPch(unsigned char*	a,
					   int				aLen,
					   unsigned char*	Ga,
					   int				GaLen);

/*	Function:
 *		DeriveSigmaKeys
 *	Description:
 *		Derive SIGMA Session Key and MAC key using serialized Ga from PCH
 *	Input arguments:
 *		IN	*Ga					- pioter to the buffer containing serialized Ga
 *		IN	GaLen				- Ga buffer length
 *		OUT	*Gb					- pointer to the buffer for computed and serialized Gb
 *		IN	GbLen				- Gb buffer length
 *		OUT	*Sk					- pointer to the buffer for derived and serialized SIGMA Session Key
 *		IN	SkLen				- Session Key buffer length
 *		OUT	*Mk					- pointer to the buffer for derived and serialized SIGMA MAC Key
 *		IN	MkLen				- MAC Key buffer length
 *	Return values:
 *		Operation status code (see cdg_status.h for details)
 */
CRYPTODATAGEN_API
CdgStatus DeriveSigmaKeys(unsigned char*	Ga,
						  int				GaLen,
						  unsigned char*	Gb,
						  int				GbLen,
						  unsigned char*	Sk,
						  int				SkLen,
						  unsigned char*	Mk,
						  int				MkLen);

//<<<<< lchojnow

/*	Function:
 *		DeriveSigmaKeys
 *	Description:
 *		Derive SIGMA Session Key and MAC key using serialized Ga from PCH
 *	Input arguments:
 *		IN	*Ga					- pioter to the buffer containing serialized Ga
 *		IN	GaLen				- Ga buffer length
 *		OUT	*Gb					- pointer to the buffer for computed and serialized Gb
 *		IN	GbLen				- Gb buffer length
 *		OUT	*Sk					- pointer to the buffer for derived and serialized SIGMA Session Key
 *		IN	SkLen				- Session Key buffer length
 *		OUT	*Mk					- pointer to the buffer for derived and serialized SIGMA MAC Key
 *		IN	MkLen				- MAC Key buffer length
 *	Return values:
 *		Operation status code (see cdg_status.h for details)
 */

CRYPTODATAGEN_API
CdgStatus DeriveSigmaKeysNull_b(unsigned char*	Ga,
								int				GaLen,
								unsigned char*	Gb,
								int				GbLen,
								unsigned char*	Sk,
								int				SkLen,
								unsigned char*	Mk,
								int				MkLen);
//>>>>> lchojnow

/*	Function:
 *		DeriveSigmaKeysPch
 *	Description:
 *		Derive SIGMA Session Key and MAC key in PCH using serialized Gb from 3p
 *	Input arguments:
 *		IN	*a					- pioter to the buffer containing serialized a
 *		IN	aLen				- a buffer length
 *		OUT	*Gb					- pointer to the buffer for computed and serialized Gb
 *		IN	GbLen				- Gb buffer length
 *		OUT	*Sk					- pointer to the buffer for derived and serialized SIGMA Session Key
 *		IN	SkLen				- Session Key buffer length
 *		OUT	*Mk					- pointer to the buffer for derived and serialized SIGMA MAC Key
 *		IN	MkLen				- MAC Key buffer length
 *	Return values:
 *		Operation status code (see cdg_status.h for details)
 */
CRYPTODATAGEN_API
CdgStatus DeriveSigmaKeysPch(unsigned char*	a,
							 int			aLen,
							 unsigned char*	Gb,
							 int			GbLen,
							 unsigned char*	Sk,
							 int			SkLen,
							 unsigned char*	Mk,
							 int			MkLen);

/*	Function:
 *		Aes128EcbEncrypt
 *	Description:
 *		Encrypt a message using AES-128 ECB
 *	Input arguments:
 *		IN	*ClrMsg				- pointer to the buffer containing a message to be encrypted
 *		IN	ClrMsgLen			- clear message buffer length
 *		OUT	*EncMsg				- pointer to the buffer for encrypted message
 *		IN	EncMsgLen			- encrypted message buffer length
 *		IN	*AesKey				- pointer to the buffer containing AES encryption key
 *		IN	AesKeyLen			- AES encryption key length
 *	Return values:
 *		Operation status code (see cdg_status.h for details)
 */
CRYPTODATAGEN_API
CdgStatus Aes128EcbEncrypt(unsigned char*	ClrMsg,
						   int				ClrMsgLen,
						   unsigned char*	EncMsg,
						   int				EncMsgLen,
						   unsigned char*	AesKey,
						   int				AesKeyLen);

/*	Function:
 *		Aes128EcbDecrypt
 *	Description:
 *		Decrypt a message using AES-128 ECB
 *	Input arguments:
 *		OUT	*EncMsg				- pointer to the buffer containing a message to be decrypted
 *		IN	EncMsgLen			- encrypted message buffer length
 *		IN	*ClrMsg				- pointer to the buffer for decrypted message
 *		IN	ClrMsgLen			- clear message buffer length
 *		IN	*AesKey				- pointer to the buffer containing AES decryption key
 *		IN	AesKeyLen			- AES decryption key length
 *	Return values:
 *		Operation status code (see cdg_status.h for details)
 */
CRYPTODATAGEN_API
CdgStatus Aes128EcbDecrypt(unsigned char*	EncMsg,
						   int				EncMsgLen,
						   unsigned char*	ClrMsg,
						   int				ClrMsgLen,
						   unsigned char*	AesKey,
						   int				AesKeyLen);

CRYPTODATAGEN_API
CdgStatus Aes128CtrEncrypt(unsigned char* ClrMsg, int ClrMsgLen, unsigned char* EncMsg, int EncMsgLen, unsigned char* AesKey, int AesKeyLen, unsigned int CtrVal);

CRYPTODATAGEN_API
CdgStatus Aes128CtrDecrypt(unsigned char* EncMsg, int EncMsgLen, unsigned char* ClrMsg, int ClrMsgLen, unsigned char* AesKey, int AesKeyLen, unsigned int CtrVal);

#endif