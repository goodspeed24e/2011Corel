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

#ifndef _COMMONFUNCTIONS_H
#define _COMMONFUNCTIONS_H

#include "Crypto_DataStructures.h"

class CNumberTheory;

class CommonFunction
{
	// ============================================================================================= //
	//                                          Attributes                                           //
	// ============================================================================================= //
	public:

	private:
		CNumberTheory* m_pNT;

	// ============================================================================================= //
	//                                          Operations                                           //
	// ============================================================================================= //
	public:
		CommonFunction();
		~CommonFunction();

		void reverseBits( unsigned char* pSource, unsigned char* pDest, unsigned int nLength );

		void xor_128( const data_128_bit x1, const data_128_bit x2, data_128_bit result );
		void xor_128_SSE( const data_128_bit x1, const data_128_bit x2, data_128_bit result );
		//result can be in-place data
		void xor_128_X2_Onto_X1( const data_128_bit x1, const data_128_bit x2 );

		void xor_n( const byte * x1, const byte * x2, int nNumberOfBytes, byte * result );
		//result can be in-place data
		void xor_n_X2_Onto_X1( byte * x1, const byte * x2, int nNumberOfBytes );

		void hexBytesToUnsignedInt( const byte * number, int nBytes, unsigned int& toBeFilled ); 

		void unsignedIntToHexBytes( unsigned int nNumber, byte number[4] );

		bool isTwoBufferIdentical( const byte * buffer1, const byte * buffer2, int nSize );

		void flipBuffer( const byte * src, byte * dst, int nSize );

		void bigEndianBytesToSmallEndianDWORDS( const byte * src, unsigned long* dst, int nNumberOfDWORDS );

		void smallEndianDWORDSToBigEndianBytes( const unsigned long* dst, byte * src, int nNumberOfDWORDS );

		bool checkIfIDWithinRevokeIDRange(BYTE ID[6], BYTE lowerBound[6], BYTE upperBound[2]);

		void swapTwoDWORD(DWORD * index);

		bool writeToFile(const char * fileWrite, unsigned char * data, unsigned long length);

		bool readFileToData(const char * fileRead, unsigned char * data, unsigned long length);

		//output = pBeExp ^ pExp mod pMod
		void ModExp(unsigned char *output, unsigned char *pBeExp, unsigned char *pExp, unsigned int unLenofExpByte, unsigned char *pMod, unsigned int unLenofModByte);

};

#endif
