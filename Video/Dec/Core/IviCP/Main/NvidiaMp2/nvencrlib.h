//---------------------------------------------------------------------------
// nvencrlib.h
//
// This file contains the portable definitions of the external interfaces to the 
// Nvidia DXVA IDCT encryption library.
//
// Copyright (c) 2001 Nvidia, Inc.
//---------------------------------------------------------------------------

#if !defined(NVENCRLIB_H)
#define NVENCRLIB_H

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

// Inside the library
extern unsigned char IndexTable[64];     

/* Called at key exchange time
   wHostSKey - Host Session Key - is an random number decoder generated, it must be non-zero.*/
unsigned long __stdcall CreateH2MKey( unsigned short wHostSKey);  

/*Called at init time
  This function use dwM2Hkey as input, it initializes some internal data and validates M2Hkey. 
  It returns zero if dwM2Hkey is invalid key, in this case dwM2Hkey is not returned from NV17.  
  As long as dwH2Mkey is not changed, this function need not be called again.  
  The encryption engine is working at frame boundary. 
  Our current interface won't allow decoder to stop encryption at the middle of a frame. 
  The encryption engine need not reset as long as MPEG engine is not reset. 
  If MPEG engine is reset by CreateMoComp(), decoder must do key exchange to start encryption again. */
int __stdcall InitKeys( unsigned short wHostKey, unsigned long dwH2MKey);     

/*The following two function are called for at the beginning of each encrypted frame
Decoder need declare two variables dwlfsrB and dwlfsrI, then initialize them by these two functions.
Later on pass these two variables into Updatlfsr() and CodingData() functions. 
As long as wFrameKey is changed by the decoder, these functions should be called. 
But don't call these functions if wFrameKey is not changed even with new frame.  
It is not necessary to encrypt every frame. */
unsigned long __stdcall InitlfsrB( unsigned short wFrameKey, unsigned short wHostKey, unsigned long dwM2HKey);

unsigned long __stdcall InitlfsrI( unsigned short wFrameKey, unsigned short wHostKey, unsigned long dwM2HKey);

/*Called for each encrypted block
This function takes *dwlfsrB, *dwlfrsI and *wPadBlock as inputs.  
It updates dwlfsrB and dwlfsrI at the beginning of each DCT block and fills wPadBlock, 
which is declared by the decoder as a single WORD and used in CodingData() function. 
  Input wHostSKey, dwM2HKey
  Input/Output *dwlfsrB, *dwlfsrI
  Outpu *wPadBlock */
void __stdcall Updatelfsr( unsigned long *dwlfsrB, unsigned long *dwlfsrI, unsigned short *wPadBlock,
				unsigned short wHostSKey, unsigned long dwM2HKey);

/*Called to encrypt each data. bIndex is zero based DCT data index,it is the same DCT index that 
decoder sends into the driver. WORD data here is DCT data.  
  Decoder call the function with original DCT data, this function returns a encrypted data*/

__inline unsigned short __stdcall CodingData( unsigned short wHostSKey, unsigned short dwlfsr_I,
					   unsigned short wPadBlock, unsigned char bIndex, unsigned short data)
{
unsigned short wPadIndex;
unsigned char bIndexScr;

	bIndexScr = (unsigned char)((dwlfsr_I ) ^ IndexTable[bIndex]);
	wPadIndex = ( wHostSKey & 0xFC0) | (bIndexScr & 0x3F);
	
	return (unsigned short)(data ^ wPadBlock ^ wPadIndex);
}

#ifdef __cplusplus
}
#endif //__cplusplus

#endif // NVENCRYPT_H
