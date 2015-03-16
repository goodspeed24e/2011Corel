#ifndef _CDG_STATUS_CODES_H
#define _CDG_STATUS_CODES_H

/*
 *	Crypto Data Generator status and result codes
 */
typedef enum {
	CdgStsOk = 0,					// No errors

	CdgStsNotInit,					// Library is not initialized
	CdgStsBadPtr,					// Bad pointer error

	CdgStsIntErr,					// Internal error (SafeId lib)

	CdgStsBuffTooSmall,				// Buffer specified for serialized key/certificate is too small
	CdgStsSerErr,					// Key/certificate serialization error
	CdgStsHashErr,					// Error computing hash

	CdgStsCryptCtxInitErr,			// Error initializing cryptosystem context
	CdgStsKeyPairGenErr,			// Error generating EC-DSA key pair
	CdqStsKeyPairVerErr,			// Error verifing EC-DSA key pair
	CdrStsKeyPairInv,				// Generated EC-DSA key pair is invalid
	CdgStsKeyPairSetErr,			// Error setting EC-DSA key pair in ECC context

	CdgStsSignErr,					// Error signing the message
	CdgStsVerifErr,					// Error verifying the signature

	CdgStsUndefined	= 0xFF			// None of above, initial state

} CdgStatus;

typedef enum {
	CdgValid = 0,
	CdgInvalid,
} CdgResult;

#endif