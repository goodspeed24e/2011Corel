/*-------------------------------------------------------------------------
 * INTEL CONFIDENTIAL
 *
 * Copyright 2004 Intel Corporation All Rights Reserved.
 *
 * This source code and all documentation related to the source code 
 * ("Material") contains trade secrets and proprietary and confidential 
 * information of Intel and its suppliers and licensors. The Material is 
 * deemed highly confidential, and is protected by worldwide copyright and 
 * trade secret laws and treaty provisions. No part of the Material may be 
 * used, copied, reproduced, modified, published, uploaded, posted, 
 * transmitted, distributed, or disclosed in any way without Intel's prior 
 * express written permission.
 *
 * No license under any patent, copyright, trade secret or other 
 * intellectual property right is granted to or conferred upon you by 
 * disclosure or delivery of the Materials, either expressly, by 
 * implication, inducement, estoppel or otherwise. Any license under such 
 * intellectual property rights must be express and approved by Intel in 
 * writing.
 *-------------------------------------------------------------------------
 */
#ifndef __SCRAMBLE_CIPHER_H__
#define __SCRAMBLE_CIPHER_H__

#ifdef __cplusplus
extern "C" {
#endif

  
#define SCRAMBLE_BLOCK_SIZE		        64;
#define SCRAMBLE_HEADER_SIZE			16;

DEFINE_GUID( DXVA_Intel_Encryption,  
0xdd975063, 0x6552, 0x11d3, 0x94, 0x3b, 0x0, 0xa0, 0xc9, 0x39, 0xb4, 0x16);

DEFINE_GUID( DXVA_COPPSetIntelScrambleKey,
0xc780a76b, 0xbf76, 0x4f87, 0xbc, 0xa0, 0xb7, 0x5, 0x74, 0x0c, 0x81, 0x0c);

DEFINE_GUID( DXVA_COPPIntelScrambleLibVer,
0xee721570, 0x051f, 0x11da, 0x8c, 0xd6, 0x08, 0x0, 0x20, 0x0c, 0x9a, 0x66);


typedef struct _DXVA_Intel_Encryption_Protocol
{
	DXVA_EncryptProtocolHeader         header;
	GUID                               scrambleKey;
	ULONG                              scrambleLibVer;
}DXVA_Intel_Encryption_Protocol,*LPDXVA_Intel_Encryption_Protocol;



// /////////////////////////////////////////////////////////////////////////////
//        The following enumerator defines a status of scramble operations
//                     negative value means error
typedef enum {
     // errors 
    csdStsCpuNotSupportedErr    = -9999,  /* The target cpu is not supported */
	
	csdStsNoKeyErr				= -7,
    csdStsSizeErr               = -6,    /* Wrong value of data size */
    csdStsBadArgErr             = -5,    /* Function arg/param is bad */
    csdStsNoMemErr              = -4,    /* Not enough memory for the operation */
    csdStsOutputNotAlignedErr	= -3,    //output not aligned memory
    csdStsInputNotAlignedErr	= -2,    //input not aligned memory
    csdStsNULLArgErr            = -1,    //input argument is NULL
                                         /*  */
    // no errors                        /*  */
    csdStsNoErr                 =   0,   /* No error, it's OK */
                                         /*  */
    // warnings                         /*  */
    csdStsNoOperation			=   1,       /* No operation has been executed */
 } csdStatus;                             


typedef unsigned char   byte;
typedef unsigned long   ulong32;   //32


//
// set the key, caller can periodically change key 
//
csdStatus __cdecl SetScrambleKey(
	const byte *key); 

//
// encrypt
//
csdStatus __cdecl EncryptData(
	const byte *input,
	byte *output,
	int bufLen );

//
// Encryption version
//

csdStatus __cdecl GetScrambleLibVersion(
    ulong32 *ulVer);


#ifdef __cplusplus
}
#endif
 
#endif /* !__SCRAMBLE_CIPHER_H__ */