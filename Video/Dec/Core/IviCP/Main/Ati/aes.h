//////////////////////////////////////////////////////////////////
//	Confidential material.  Copyright 2004 Intervideo, Inc.
//	All Rights Reserved.
//	File: aes.h

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
typedef unsigned char BYTE;
typedef unsigned long DWORD;

namespace NLibDisplay
{

void __cdecl AesEncrypt(const BYTE *key,      //16 bytes
				const BYTE *input,    //16 bytes
				      BYTE *output);  //16 bytes

void __cdecl AesDecrypt(const BYTE *key,      //16 bytes
				const BYTE *input,    //16 bytes
				BYTE *output);        //16 bytes

void __cdecl AesEncryptKeyKx1(const BYTE *input, BYTE *output);

void __cdecl AesDecryptKeyKx2(const BYTE *input, BYTE *output);

void __cdecl AesDecryptKeyCommon(const BYTE *input, BYTE *output);

};//namespace NLibDisplay
