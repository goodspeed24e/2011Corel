//////////////////////////////////////////////////////////////////
//	Confidential material.  Copyright 2005 Intervideo, Inc.
//	All Rights Reserved.
//	File:	RandomNumber16Byte.h

#include "aes.h"

namespace NLibDisplay {

class CRandomNumber16Byte
{
public:
	void PrepareRandSeed1();
	void PrepareRandSeed2();
	void PrepareRandSeed3();
	void GenerateRandomNumber();
	BYTE RandNumber[16];
private:
	void PrepareRandSeed(DWORD *RandSeed);
};

}; //namespace NLibDisplay