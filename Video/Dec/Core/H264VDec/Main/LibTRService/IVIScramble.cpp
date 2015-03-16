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
//  Copyright (c) 1998 - 2000  InterVideo Corporation.  All Rights Reserved.
//
//-----------------------------------------------------------------------------

#include "IVITrWinDef.h"
#include "IVIScramble.h"
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

DWORD iviCTR::KeyASeed = TR_KEYA_INITIAL_XOR_SEED;
DWORD iviCTR::KeyBSeed = TR_KEYB_INITIAL_XOR_SEED;

// base location to determine if relocation has occurred.
// DWORD CTR::TR_RelocateBase = TR_CHECKSUM_RELOCATION; 

// DWORD __fastcall TR_RelocateCode()
// {
// 	return 10;
// }

// The code is moved to the header file to allow function inline
/*
#define iviTR_CHECKSUM(val, start_addr, end_addr, seed)\
		{ DWORD *dptr;\
		for(val = seed, dptr = (DWORD *) start_addr; dptr < (DWORD *) end_addr; dptr++)\
			{val += *dptr;  val = (((val>>17)^(val>>11)^val)&0xff)|(val<<8);}}
#define TR_SCRAMBLE(start_addr, end_addr, seed)\
		{ DWORD *dptr; DWORD val, oval;\
		for(val = seed, dptr = (DWORD *) start_addr; dptr < (DWORD *) end_addr; dptr++)\
			{ oval = val; val += *dptr; *dptr ^=oval; val = (((val>>17)^(val>>11)^val)&0xff)|(val<<8);}}
#define TR_DESCRAMBLE(start_addr, end_addr, seed)\
		{ DWORD *dptr; DWORD val;\
		for(val = seed, dptr = (DWORD *) start_addr; dptr < (DWORD *) end_addr; dptr++)\
			{ *dptr ^=val; val += *dptr;  val = (((val>>17)^(val>>11)^val)&0xff)|(val<<8);}}
#define TR_CHECKSUM_RELOCATION	0xf9183183

//
// The following code bits determine relocation
//

DWORD	TR_RelocateBase	= TR_CHECKSUM_RELOCATION; // base location to determine if relocation has occurred.

// the entrance dwBase is the "relocated base" of the scrambled segment.
DWORD __fastcall TR_RelocateCode()
	{
	return 10;
	}

//
// CTR class
//

DWORD CTR::KeyASeed = TR_KEYA_INITIAL_XOR_SEED;
DWORD CTR::KeyBSeed = TR_KEYB_INITIAL_XOR_SEED;

BOOL __fastcall CTR::IsRelocated()
	{
#ifdef _WIN32
	return TR_RelocateBase!=TR_CHECKSUM_RELOCATION && TR_RelocateBase!=(DWORD)TR_RelocateCode;
#else
	return 1; // NOTE: Linux shared libraries are ALWAYS relocated!
#endif
	}

DWORD __fastcall CTR::GetUndoOffset()
	{
	return TR_RelocateBase==TR_CHECKSUM_RELOCATION ? 0 : TR_RelocateBase - (DWORD)TR_RelocateCode;
	}

// updates bSkipAddr and dwBase from pwTable
static inline void GetNextAddr(BOOL &bSkipAddr, DWORD &dwBase, WORD *&pwTable)
	{
	bSkipAddr = FALSE;
	if (*pwTable==0xfffe)		// check escape code
		{
		pwTable++;
		bSkipAddr = TRUE;
		}
	if (*pwTable==0xffff)		// check end of data code
		dwBase = 0xffffffff;
	else 
		dwBase += *pwTable++;	// increment value
	}

// used by files to derive proper checksum in the presence of relocation.
void __fastcall CTR::Checksum(void *start_addr, void *end_addr, DWORD seed, DWORD *pval, WORD *pwTable)
	{
	DWORD val;

	if (pwTable && *pwTable!=0xffff && IsRelocated())
		{
		BOOL bSkipAddr;
		DWORD *dptr,dwUndo,dwBase,dwOrig,dwBaseBitOffset;

		dwUndo = GetUndoOffset();	// additive undo value
		dwBase = (DWORD)start_addr;
		GetNextAddr(bSkipAddr, dwBase, pwTable);
		for(val=seed, dptr=(DWORD *)start_addr; (DWORD)dptr<(DWORD)end_addr; dptr++)
			{
			// original datum comes from (dptr)
			dwOrig = *dptr;
			//
			// check if reloc address is in the low bits.
			//
			if ((DWORD)dptr>=dwBase)
				{
				dwBaseBitOffset = ((DWORD)dptr-dwBase)<<3;
				if (bSkipAddr)
					{	// skip this address!
					if(dwBaseBitOffset==0)
						dwOrig = 0;
					else
						dwOrig &= -1<<(32-dwBaseBitOffset);
					}
				else
					{	// relocate this address
					if(dwBaseBitOffset==0)
						dwOrig += dwUndo;	// perfect alignment.
					else
						{					// imperfect alignment.
						// mask high bytes (we'll be putting in low bytes later).
						dwOrig &= -1<<(32-dwBaseBitOffset);
						// get true address and shift to low bytes of value
						dwOrig |= (*((DWORD *)dwBase)+dwUndo)>>dwBaseBitOffset;
						}
					}
				// get next relocation
				GetNextAddr(bSkipAddr, dwBase, pwTable);
				}
			//
			//	check if reloc address is in the high bits of datum
			//  (this CAN occur even after reloc address is in low bits)
			//
			if ((DWORD)dptr>=dwBase-3)
				{
				if (bSkipAddr)
					dwOrig &= (1<<((dwBase-(DWORD)dptr)<<3)) - 1;
				else
					dwOrig += dwUndo<<((dwBase-(DWORD)dptr)<<3);
				}
			//
			// update add/lfsr
			//
			val += dwOrig;
			val = (((val>>17)^(val>>11)^val)&0xff)|(val<<8);
			}
		}
	else
		{
		iviTR_CHECKSUM(val, start_addr, end_addr, seed);
		}
	*pval = val;
	}

// start_addr is the "relocated base" of the scrambled segment.
BOOL __fastcall CTR::RelocateCode(void *start_addr, WORD *pwTable, BOOL bUndo)
	{
	if(pwTable && *pwTable!=0xffff && IsRelocated())
		{
		BOOL bSkipAddr;
		DWORD dwBase, dwUndo, dwChange;

		dwBase = (DWORD)start_addr;
		dwUndo = GetUndoOffset();
		dwChange = bUndo ? dwUndo : (DWORD)-(int)dwUndo;
		while(*pwTable!=0xffff)
			{
			GetNextAddr(bSkipAddr, dwBase, pwTable);
			if(!bSkipAddr)
				*((DWORD *)dwBase) += dwChange;
			}
		return TRUE;
		}
	return FALSE;
	}

void __fastcall CTR::Descramble(void *start_addr, void *end_addr, DWORD seed, WORD *pwTable)
	{
	DWORD val, *dptr;
	
	if(pwTable && *pwTable!=0xffff && IsRelocated())
		{
		BOOL bSkipAddr;
		WORD *pwOrigTable;
		DWORD dwBase,dwMask,dwBaseBitOffset;

		pwOrigTable = pwTable;
		RelocateCode(start_addr, pwOrigTable, TRUE);
		dwBase = (DWORD)start_addr;
		GetNextAddr(bSkipAddr, dwBase, pwTable);
		for(val=seed, dptr=(DWORD *)start_addr; (DWORD)dptr<(DWORD)end_addr; dptr++)
			{
			dwMask = 0xffffffff;
			//
			// check if reloc address is in the low bits.
			//
			if ((DWORD)dptr>=dwBase)
				{
				dwBaseBitOffset = ((DWORD)dptr-dwBase)<<3;
				if (bSkipAddr)
					{	// skip this address!
					if(dwBaseBitOffset==0)
						dwMask = 0;
					else
						dwMask &= -1<<(32-dwBaseBitOffset);
					}
				// get next relocation
				GetNextAddr(bSkipAddr, dwBase, pwTable);
				}
			//
			//	check if reloc address is in the high bits of datum
			//  (this CAN occur even after reloc address is in low bits)
			//
			if ((DWORD)dptr>=dwBase-3)
				{
				if (bSkipAddr)
					dwMask &= (1<<((dwBase-(DWORD)dptr)<<3)) - 1;
				}
			//
			// update add/lfsr
			//
			*dptr ^= val & dwMask;
			val += *dptr & dwMask;
			val = (((val>>17)^(val>>11)^val)&0xff)|(val<<8);
			}
		RelocateCode(start_addr, pwOrigTable, FALSE);
		}
	else
		{
		TR_DESCRAMBLE(start_addr,end_addr,seed);
		}
	}

void __fastcall CTR::Scramble(void *start_addr, void *end_addr, DWORD seed, WORD *pwTable)
	{
	DWORD *dptr, val, oval;

	if (pwTable && *pwTable!=0xffff && IsRelocated())
		{
		BOOL bSkipAddr;
		DWORD dwBase,dwMask,dwBaseBitOffset;

		dwBase = (DWORD)start_addr;
		GetNextAddr(bSkipAddr, dwBase, pwTable);
		for(val=seed, dptr=(DWORD *)start_addr; (DWORD)dptr<(DWORD)end_addr; dptr++)
			{
			dwMask = 0xffffffff;
			//
			// check if reloc address is in the low bits.
			//
			if ((DWORD)dptr>=dwBase)
				{
				dwBaseBitOffset = ((DWORD)dptr-dwBase)<<3;
				if (bSkipAddr)
					{	// skip this address!
					if(dwBaseBitOffset==0)
						dwMask = 0;
					else
						dwMask &= -1<<(32-dwBaseBitOffset);
					}
				// get next relocation
				GetNextAddr(bSkipAddr, dwBase, pwTable);
				}
			//
			//	check if reloc address is in the high bits of datum
			//  (this CAN occur even after reloc address is in low bits)
			//
			if ((DWORD)dptr>=dwBase-3)
				{
				if (bSkipAddr)
					dwMask &= (1<<((dwBase-(DWORD)dptr)<<3)) - 1;
				}
			//
			// update add/lfsr
			//
			oval = val;
			val += *dptr & dwMask;
			*dptr ^= oval & dwMask;
			val = (((val>>17)^(val>>11)^val)&0xff)|(val<<8);
			}
		}
	else
		{
		TR_SCRAMBLE(start_addr,end_addr,seed);
		}
	}
*/