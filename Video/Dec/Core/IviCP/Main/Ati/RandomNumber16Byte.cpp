//////////////////////////////////////////////////////////////////
//	Confidential material.  Copyright 2005 Intervideo, Inc.
//	All Rights Reserved.
//	File:	RandomNumber16Byte.h

#include "RandomNumber16Byte.h"

namespace NLibDisplay {


void CRandomNumber16Byte::PrepareRandSeed1()
{
	PrepareRandSeed((DWORD *)RandNumber);
}

void CRandomNumber16Byte::PrepareRandSeed2()
{
	PrepareRandSeed(((DWORD *)RandNumber)+1);
}

void CRandomNumber16Byte::PrepareRandSeed3()
{
	PrepareRandSeed(((DWORD *)RandNumber)+2);
}

void CRandomNumber16Byte::GenerateRandomNumber()
{
	BYTE internalkey[16]= {1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4};
	PrepareRandSeed(((DWORD *)RandNumber)+3);
	AesEncrypt(internalkey, RandNumber, RandNumber);
}

void CRandomNumber16Byte::PrepareRandSeed(DWORD *RandSeed)
{
	__asm rdtsc
	__asm mov edx, RandSeed
	__asm mov dword ptr [edx], eax
}

}; //namespace NLibDisplay