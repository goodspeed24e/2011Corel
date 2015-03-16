//////////////////////////////////////////////////////////////////
//	Confidential material.  Copyright 2005 Intervideo, Inc.
//	All Rights Reserved.

#if !defined(LIBISMP_DEFINE__INCLUDED_)
#define LIBISMP_DEFINE__INCLUDED_

#ifdef __cplusplus
extern "C"
{
#endif 


#pragma once
typedef unsigned char    BYTE;
typedef unsigned short   WORD;
typedef unsigned long    DWORD;
typedef int              BOOL;
typedef unsigned __int64 ULONGLONG;
typedef unsigned char    byte;
typedef byte data_128_bit [16];
#define TRUE  1
#define FALSE 0

//////////////////////////////
// Percentage value define  //
//////////////////////////////
typedef enum
{
	PERCENTRATE_ZERO = 0xFFF0,
	PERCENTRATE_10   = 0xFFF1, 
	PERCENTRATE_20   = 0xFFF2, 
	PERCENTRATE_30   = 0xFFF4, 
	PERCENTRATE_40   = 0xFFF8, 
	PERCENTRATE_50   = 0xFF10, 
	PERCENTRATE_60   = 0xFF20, 
	PERCENTRATE_70   = 0xFF40, 
	PERCENTRATE_80   = 0xFF80, 
	PERCENTRATE_90   = 0xF100,
	PERCENTRATE_FULL = 0xFFFF	
}ISMP_PERCENTAGE;

///////////////////////////////////////
// Percentage internal value define  //
///////////////////////////////////////
typedef enum
{
	PERCENTRATE_INTERNAL_ZERO = 0,
	PERCENTRATE_INTERNAL_10   = 10, 
	PERCENTRATE_INTERNAL_20   = 20, 
	PERCENTRATE_INTERNAL_30   = 30, 
	PERCENTRATE_INTERNAL_40   = 40, 
	PERCENTRATE_INTERNAL_50   = 50, 
	PERCENTRATE_INTERNAL_60   = 60, 
	PERCENTRATE_INTERNAL_70   = 70, 
	PERCENTRATE_INTERNAL_80   = 80, 
	PERCENTRATE_INTERNAL_90   = 90,
	PERCENTRATE_INTERNAL_FULL = 100	
}ISMP_INTERNAL_PERCENTAGE;

enum
{ 
	OPINPROTECT_NOSESSION,
	OPINPROTECT_REQUESTVERIFIED,
	OPINPROTECT_SESSION_ESTABLISHED,
	OPINPROTECT_ENCRYPTION_ON
};

enum 
{ 
	IPINPROTECT_NOSESSION, 
	IPINPROTECT_REQUESTISSUED, 
	IPINPROTECT_SESSION_ESTABLISHED, 
	IPINPROTECT_DECRYPTION_ON
};

enum 
{ 
	ISMP_STMTYPE_LEGACY_AUDIO  = 0, 
	ISMP_STMTYPE_LEGACY_VIDEO  = 1, 
	ISMP_STMTYPE_EXAMPLE1      = 0x0880acca,
	ISMP_STMTYPE_EXAMPLE2      = 0x4e658d05,
	ISMP_STMTYPE_PLACEHOLDER1  = 0x7E0B230D,
	ISMP_STMTYPE_PLACEHOLDER2  = 0x33804996,
	ISMP_STMTYPE_PLACEHOLDER3  = 0x1C9F6296,
	ISMP_STMTYPE_PLACEHOLDER4  = 0xE9A308F7,
	ISMP_STMTYPE_PLACEHOLDER5  = 0x59A64371,
	ISMP_STMTYPE_PLACEHOLDER6  = 0x0BD266F4,
	ISMP_STMTYPE_PLACEHOLDER7  = 0xCDBBF705,
	ISMP_STMTYPE_PLACEHOLDER8  = 0xA05B4caf,
	ISMP_STMTYPE_PLACEHOLDER9  = 0x1BE092C5,
	ISMP_STMTYPE_PLACEHOLDER10 = 0x0367D518,
	ISMP_STMTYPE_PLACEHOLDER11 = 0x1E494550,
	ISMP_STMTYPE_PLACEHOLDER12 = 0x9983F44B,
	ISMP_STMTYPE_PLACEHOLDER13 = 0x07528269,
	ISMP_STMTYPE_PLACEHOLDER14 = 0x40EC4fe8,
	ISMP_STMTYPE_PLACEHOLDER15 = 0x1E432960,
};

void SimpleScramble(const BYTE *cipherKey, const BYTE *input, BYTE *output, DWORD cbBytes);
void SimpleDescramble(const BYTE *cipherKey, const BYTE *input, BYTE *output, DWORD cbBytes);

////////////////////////////////////////////////////////////////////////////
// INTERFACE DECLARATION for ISMP-OUTPUT                                  //
////////////////////////////////////////////////////////////////////////////

typedef struct IsmpOutputObject
{
	int   m_iSessionState;
	BYTE  m_aData[16];
	BYTE  m_aKeyschedule[4*4*11];
	int   m_iStreamType;
	BOOL  m_bIsDataScramble;
	BYTE  m_ParticipateRate;
	int   m_iAESMode;
	BYTE  m_aMaskSeed[20];
	BYTE  m_aMask[4*4*11];
	BYTE  m_IsAACS;
}CIsmpOutputObject;


// IsmpOutput_Open
//
//   Purpose: Open ISMP session for IsmpOutput object.
//   Params:  requestdata    [IN]  16 bytes of data coming from ISMP-input object
//            datastreamtype [IN]  32-bit unsigned integer that is shared between ISMP-input & ISMP-output objects.
//            responsedata   [OUT] Pointer to 16-byte buffer that receives response data.
//   Return:  true  = success in opening session
//            false = fail to open session
BOOL __stdcall IsmpOutput_Open(struct IsmpOutputObject *ctx, const BYTE *requestdata, DWORD datastreamtype, BYTE *responsedata);



// IsmpOutput_OpenSession
//
//   Purpose: Open ISMP session for IsmpOutput object.
//   Params:  requestdata    [IN]  16 bytes of data coming from ISMP-input object
//            datastreamtype [IN]  32-bit unsigned integer that is shared between ISMP-input & ISMP-output objects.
//            responsedata1   [OUT] Pointer to 16-byte buffer that receives response data 1 (the session key).
//            responsedata2   [OUT] Pointer to 16-byte buffer that receives response data 2 (the content key).
//   Return:  true  = success in opening session
//            false = fail to open session
BOOL __stdcall IsmpOutput_Open2(struct IsmpOutputObject *ctx, 
								const BYTE *requestdata, 
								DWORD datastreamtype, 
								BYTE *responsedata1,
								BYTE *responsedata2);


// IsmpOutput_OpenFromInput
//
//   Purpose: Open ISMP session for IsmpOutput object using key from input session.
//   Params:  responsedata1In [IN] 16 bytes of response data 1 (session key) from a previously opened session
//			  responsedata2In [IN] 16 bytes of response data 2 (content key) from a previously opened session
//			  requestdataIn   [IN]  16 bytes of data from the connection with the output object that the key was derived from
//			  requestdataOut  [IN]  16 bytes of data coming from ISMP-input object
//            datastreamtype  [IN]  32-bit unsigned integer that is shared between ISMP-input & ISMP-output objects.
//            responsedata1Out [OUT] Pointer to 16-byte buffer that receives response data (session key).
//            responsedata2Out [OUT] Pointer to 16-byte buffer that receives response data (content key).
//   Return:  true  = success in opening session
//            false = fail to open session
BOOL __stdcall IsmpOutput_OpenFromInput(BYTE *responsedata1In, 
										BYTE *responsedata2In,
										struct IsmpOutputObject *ctxOut, 
										const BYTE *requestdataOut, 
										DWORD datastreamtype, 
										BYTE *responsedata1Out,
										BYTE *responsedata2Out);



// IsmpOutput_Close
//
//   Purpose: Close ISMP session for IsmpOutput object.
//   Params:  N/A
//   Return:  true  = success in closing session
//            false = fail to close session
BOOL __stdcall IsmpOutput_Close(struct IsmpOutputObject *ctx);


// IsmpOutput_GetState
//
//   Purpose: Get the current state of ISMP-output session
//   Params:  N/A
//   Return:  the current state
int  __stdcall IsmpOutput_GetState(struct IsmpOutputObject *ctx);


// IsmpOutput_ScrambleData
//
//   Purpose: Scramble data stream.  It is OK if dst=src.
//   Params:  dst [OUT]  Pointer to output data buffer
//            src [IN]   Pointer to input data buffer
//            len [IN]   Length of both input and output buffers
//   Return:  true  = success
//            false = failed
BOOL __stdcall IsmpOutput_ScrambleData(struct IsmpOutputObject *ctx, BYTE *dst, const BYTE *src, int len);

// IsmpOutput_Partial_ScrambleData
//
//   Purpose: Use percentage to partial scramble data stream ..  It is OK if dst=src.
//   Params:  dst [OUT]  Pointer to output data buffer
//            src [IN]   Pointer to input data buffer
//            len [IN]   Length of both input and output buffers
//   Return:  true  = success
//            false = failed
BOOL __stdcall IsmpOutput_Partial_ScrambleData(struct IsmpOutputObject *ctx, BYTE *dst, const BYTE *src, int len);


// IsmpOutput_ParicipateRate
//
//   Purpose: Control the participate the stream
BOOL __stdcall IsmpOutput_ParticipateRate(struct IsmpOutputObject *ctx, ISMP_PERCENTAGE percent);



////////////////////////////////////////////////////////////////////////////
// INTERFACE DECLARATION for ISMP-INPUT                                   //
////////////////////////////////////////////////////////////////////////////

typedef struct IsmpInputObject
{
	int   m_iSessionState;
	BYTE  m_aData[16];
	BYTE  m_aKeyschedule[4*4*11];
	BYTE  m_aRandomSeed[8];
	int   m_iStreamType;
	BOOL  m_bIsDataScramble;
	BYTE  m_ParticipateRate;
	int   m_iAESMode;
	BYTE  m_aMaskSeed[20];
	BYTE  m_aMask[4*4*11];
	BYTE  m_IsAACS;
}CIsmpInputObject;


// IsmpInput_Open
//
//   Purpose: Open ISMP session for ISMP-input object.
//   Params:  requestdata    [OUT] 16 bytes of data generated by this function.
//            datastreamtype [IN]  32-bit unsigned integer that is shared between ISMP-input & ISMP-output objects.
//   Return:  true  = success in opening session
//            false = fail to open session
BOOL __stdcall IsmpInput_Open(struct IsmpInputObject *ctx, BYTE *requestdata, DWORD datastreamtype);


// IsmpInput_Close
//
//   Purpose: Close ISMP session for ISMP-input object.
//   Params:  N/A
//   Return:  true  = success in closing session
//            false = fail to close session
BOOL __stdcall IsmpInput_Close(struct IsmpInputObject *ctx);


// IsmpInput_VerifyResponse
//
//   Purpose: Verify the response coming from ISMP-output object
//   Params:  responsedata [IN] Pointer to 16 bytes of response data coming from ISMP-output object
//   Return:  true  = Authentication succeeded, and session key established.
//            false = Authentication failed; the ISMP-output object is not accepted.
BOOL __stdcall IsmpInput_VerifyResponse(struct IsmpInputObject *ctx, BYTE *responsedata);

// IsmpInput_VerifyResponse
//
//   Purpose: Verify the response coming from ISMP-output object
//   Params:  responsedata1 [IN] Pointer to 16 bytes of response data 1(session key) coming from ISMP-output object
//            responsedata2 [IN] Pointer to 16 bytes of response data 2 (content key) coming from ISMP-output object
//   Return:  true  = Authentication succeeded, and session key established.
//            false = Authentication failed; the ISMP-output object is not accepted.
BOOL __stdcall IsmpInput_VerifyResponse2(struct IsmpInputObject *ctx, BYTE *responsedata1, BYTE *responsedata2);


// IsmpInput_DescrambleData
//
//   Purpose: Descramble data stream.  It is OK if dst=src.
//   Params:  dst      [OUT] pointer to output buffer
//            src      [IN]  pointer to input buffer
//            datasize [IN]  length of both input and output buffer
//   Return:  true  = success
//            false = failed
BOOL __stdcall IsmpInput_DescrambleData(struct IsmpInputObject *ctx, BYTE *dst, const BYTE *src, int datasize);

// IsmpInput_Partial_DescrambleData
//
//   Purpose: Use percentage to partial descramble data stream.  It is OK if dst=src.
//   Params:  dst      [OUT] pointer to output buffer
//            src      [IN]  pointer to input buffer
//            datasize [IN]  length of both input and output buffer
//   Return:  true  = success
//            false = failed
BOOL __stdcall IsmpInput_Partial_DescrambleData(struct IsmpInputObject *ctx, BYTE *dst, const BYTE *src, int datasize);


// IsmpInput_GetState
//
//   Purpose: Get the current state of ISMP-input session
//   Params:  N/A
//   Return:  the current state
int  __stdcall IsmpInput_GetState(struct IsmpInputObject *ctx);

// Ismp_Mask
//
//   Purpose: Process full mask with input data.
//   Params:  dst      [OUT] pointer to output buffer
//            src      [IN]  pointer to input buffer
//            length   [IN]  length of both input and output buffer
//   Return:  true  = success
//            false = failed
void __stdcall Ismp_Mask(const BYTE *src, BYTE *dst, unsigned int length, BYTE* Mask);


#ifdef __cplusplus
}
#endif

#endif // !defined(LIBISMP_DEFINE__INCLUDED_)

