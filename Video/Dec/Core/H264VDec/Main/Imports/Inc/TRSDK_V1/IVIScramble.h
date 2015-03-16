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

#ifndef _IVISCRAMBLE_H
#define _IVISCRAMBLE_H

#include "IVIScrambleDef.h"
#include ".\BaseTR\IVITRComm.h"

#define iviTR_CHECKSUM(val, start_addr, end_addr, seed)\
		{ DWORD *dptr;\
		for(val = seed, dptr = (DWORD *) start_addr; dptr < (DWORD *) end_addr; dptr++)\
			{val += *dptr;  val = (((val>>17)^(val>>11)^val)&0xff)|(val<<8);}}
#define iviTR_SCRAMBLE(start_addr, end_addr, seed)\
		{ DWORD *dptr; DWORD val, oval;\
		for(val = seed, dptr = (DWORD *) start_addr; dptr < (DWORD *) end_addr; dptr++)\
			{ oval = val; val += *dptr; *dptr ^=oval; val = (((val>>17)^(val>>11)^val)&0xff)|(val<<8);}}
#define iviTR_DESCRAMBLE(start_addr, end_addr, seed)\
		{ DWORD *dptr; DWORD val;\
		for(val = seed, dptr = (DWORD *) start_addr; dptr < (DWORD *) end_addr; dptr++)\
			{ *dptr ^=val; val += *dptr;  val = (((val>>17)^(val>>11)^val)&0xff)|(val<<8);}}

// Extended checksum routines.
#define CHECKSUM_INSERT1(val, REG1, REG2) \
	__asm mov REG1, val \
	__asm mov REG2, REG1 \
	__asm shr REG2, 04h \
	__asm shr REG2, 02h \
	__asm xor REG2, REG1 \
	__asm shr REG2, 09h \
	__asm shr REG2, 02h \
	__asm xor REG2, REG1 \
	__asm and REG2, 0FFh \
	__asm shl REG1, 03h \
	__asm shl REG1, 05h \
	__asm or  REG1, REG2 \
	__asm mov val, REG1

#define CHECKSUM_INSERT2(val, REG1, REG2) \
	__asm mov REG1, val \
	__asm mov REG2, REG1 \
	__asm shr REG2, 05h \
	__asm shr REG2, 01h \
	__asm xor REG2, REG1 \
	__asm shr REG2, 04h \
	__asm shr REG2, 07h \
	__asm xor REG2, REG1 \
	__asm and REG2, 0FFh \
	__asm shl REG1, 02h \
	__asm shl REG1, 06h \
	__asm or  REG1, REG2 \
	__asm mov val, REG1

#define CHECKSUM_INSERT3(val, REG1, REG2) \
	__asm mov REG1, val \
	__asm mov REG2, REG1 \
	__asm or  REG2, 07h \
	__asm shr REG2, 03h \
	__asm shr REG2, 03h \
	__asm xor REG2, REG1 \
	__asm xor REG2, 3Ch \
	__asm shr REG2, 06h \
	__asm shr REG2, 05h \
	__asm xor REG2, REG1 \
	__asm and REG2, 0FFh \
	__asm shl REG1, 01h \
	__asm shl REG1, 07h \
	__asm or  REG1, REG2 \
	__asm mov val, REG1

#define CHECKSUM_INSERT4(val, REG1, REG2) \
	__asm mov REG1, val \
	__asm mov REG2, REG1 \
	__asm xor REG2, 13h \
	__asm shr REG2, 05h \
	__asm shr REG2, 01h \
	__asm xor REG2, REG1 \
	__asm shr REG2, 01h \
	__asm xor REG2, 2A9h \
	__asm or  REG2, 01Fh \
	__asm shr REG2, 0Ah \
	__asm xor REG2, REG1 \
	__asm and REG2, 0FFh \
	__asm shl REG1, 04h \
	__asm shl REG1, 04h \
	__asm or  REG1, REG2 \
	__asm mov val, REG1

#define iviTR_CHECKSUM_EXPRESS1(val) \
	CHECKSUM_INSERT1(val, ebx, esi)

#define iviTR_CHECKSUM1(val, start_addr, end_addr, seed) \
	{ \
		DWORD *dptr; \
		for (val = seed, dptr = (DWORD *)start_addr; dptr < (DWORD *)end_addr; dptr++) \
		{ \
			val += *dptr; \
			iviTR_CHECKSUM_EXPRESS1(val) \
		} \
	}

#define iviTR_CHECKSUM_EXPRESS2(val) \
	CHECKSUM_INSERT2(val, eax, edi)

#define iviTR_CHECKSUM2(val, start_addr, end_addr, seed) \
	{ \
		DWORD *dptr; \
		for (val = seed, dptr = (DWORD *)start_addr; dptr < (DWORD *)end_addr; dptr++) \
		{ \
			val += *dptr; \
			iviTR_CHECKSUM_EXPRESS2(val) \
		} \
	}

#define iviTR_CHECKSUM_EXPRESS3(val) \
	CHECKSUM_INSERT3(val, ebx, esi)

#define iviTR_CHECKSUM3(val, start_addr, end_addr, seed) \
	{ \
		DWORD *dptr; \
		for (val = seed, dptr = (DWORD *)start_addr; dptr < (DWORD *)end_addr; dptr++) \
		{ \
			val += *dptr; \
			iviTR_CHECKSUM_EXPRESS3(val) \
		} \
	}

#define iviTR_CHECKSUM_EXPRESS4(val) \
	CHECKSUM_INSERT4(val, eax, edi)

#define iviTR_CHECKSUM4(val, start_addr, end_addr, seed) \
	{ \
		DWORD *dptr; \
		for (val = seed, dptr = (DWORD *)start_addr; dptr < (DWORD *)end_addr; dptr++) \
		{ \
			val += *dptr; \
			iviTR_CHECKSUM_EXPRESS4(val) \
		} \
	}

#define iviTR_USE_CTR_MACRO_VERSION

#if (defined(__cplusplus) && !defined(iviTR_USE_CTR_MACRO_VERSION))
	#define iviTR_ChecksumRegion(start_addr, end_addr, seed, pval, pwTable)\
		iviCTR::Checksum(start_addr, end_addr, seed, pval, pwTable)
	#define iviTR_ChecksumRegion1(start_addr, end_addr, seed, pval, pwTable)\
		iviCTR::Checksum(start_addr, end_addr, seed, pval, pwTable)
	#define iviTR_ChecksumRegion2(start_addr, end_addr, seed, pval, pwTable)\
		iviCTR::Checksum(start_addr, end_addr, seed, pval, pwTable)
	#define iviTR_ChecksumRegion3(start_addr, end_addr, seed, pval, pwTable)\
		iviCTR::Checksum(start_addr, end_addr, seed, pval, pwTable)
	#define iviTR_ChecksumRegion4(start_addr, end_addr, seed, pval, pwTable)\
		iviCTR::Checksum(start_addr, end_addr, seed, pval, pwTable)
	#define iviTR_ScrambleRegion(start_addr, end_addr, seed, pwTable)\
		iviCTR::Scramble(start_addr, end_addr, seed, pwTable)
	#define iviTR_DescrambleRegion(start_addr, end_addr, seed, pwTable)\
		iviCTR::Descramble(start_addr, end_addr, seed, pwTable)
#else
	#define iviTR_ChecksumRegion(start_addr, end_addr, seed, pval, pwTable)\
		DO_CHECKSUM(start_addr, end_addr, seed, pval, pwTable)
	#define iviTR_ChecksumRegion1(start_addr, end_addr, seed, pval, pwTable)\
		DO_CHECKSUM1(start_addr, end_addr, seed, pval, pwTable)
	#define iviTR_ChecksumRegion2(start_addr, end_addr, seed, pval, pwTable)\
		DO_CHECKSUM2(start_addr, end_addr, seed, pval, pwTable)
	#define iviTR_ChecksumRegion3(start_addr, end_addr, seed, pval, pwTable)\
		DO_CHECKSUM3(start_addr, end_addr, seed, pval, pwTable)
	#define iviTR_ChecksumRegion4(start_addr, end_addr, seed, pval, pwTable)\
		DO_CHECKSUM4(start_addr, end_addr, seed, pval, pwTable)
	#define iviTR_ScrambleRegion(start_addr, end_addr, seed, pwTable)\
		DO_SCRAMBLE(start_addr, end_addr, seed, pwTable)
	#define iviTR_DescrambleRegion(start_addr, end_addr, seed, pwTable)\
		DO_DESCRAMBLE(start_addr, end_addr, seed, pwTable)
#endif

#ifdef iviTR_ENABLE_LIGHTWEIGHT_SCRAMBLE
	#define iviTR_Lightweight_ScrambleRegion(start_addr, end_addr, seed, pwTable, pwScrambleEIPTable, scrambleEIPTableSize)\
		iviTR_LIGHTWEIGHT_SCRAMBLE(start_addr, end_addr, seed, pwTable, pwScrambleEIPTable, scrambleEIPTableSize)
	#define iviTR_Lightweight_DescrambleRegion(start_addr, end_addr, seed, pwTable, pwScrambleEIPTable, scrambleEIPTableSize)\
		iviTR_LIGHTWEIGHT_DESCRAMBLE(start_addr, end_addr, seed, pwTable, pwScrambleEIPTable, scrambleEIPTableSize)
#else
	#define iviTR_Lightweight_ScrambleRegion(start_addr, end_addr, seed, pwTable, pwScrambleEIPTable, scrambleEIPTableSize)
	#define iviTR_Lightweight_DescrambleRegion(start_addr, end_addr, seed, pwTable, pwScrambleEIPTable, scrambleEIPTableSize)
#endif

/****************************************************
 *	macro equivalents of all functions in CTR class *
 ****************************************************/

#define DO_GETNEXTADDR(bSkipAddr,dwBase, pwTable)	\
{													\
	bSkipAddr = FALSE;								\
	if (*pwTable==0xfffe)							\
	{												\
		pwTable++;									\
		bSkipAddr = TRUE;							\
	}												\
	if (*pwTable==0xffff)							\
		dwBase = 0xffffffff;						\
	else											\
		dwBase += *pwTable++;						\
}

#define DO_GETUNDOOFFSET \
	(TR_Relocate_Base2==1) ? 0 : TR_Relocate_Base2 - PtrToUlong(TR_RelocateCode)

#ifdef _WIN32
#define DO_ISRELOCATED	\
	(TR_Relocate_Base2!=1 && TR_Relocate_Base2!=PtrToUlong(TR_RelocateCode))
#else
#define DO_ISRELOCATED	1
#endif

#define DO_RELOCATECODE(start_addr, pwTable, bUndo)			\
{															\
	if(pwTable && *pwTable!=0xffff && DO_ISRELOCATED)		\
	{														\
		WORD *_pwTable = pwTable;							\
		BOOL bSkipAddr;										\
		DWORD dwBase, dwUndo, dwChange;						\
		dwBase = PtrToUlong(start_addr);					\
		dwUndo = DO_GETUNDOOFFSET;							\
		dwChange = bUndo ? dwUndo : (DWORD)-(int)dwUndo;	\
		while(*_pwTable!=0xffff)							\
		{													\
			DO_GETNEXTADDR(bSkipAddr, dwBase, _pwTable);	\
			if(!bSkipAddr)									\
				*((DWORD *)UlongToPtr(dwBase)) += dwChange;				\
		}													\
	}														\
}

#define DO_CHECKSUM(start_addr, end_addr, seed, pval, pwTable)							\
{																						\
	DWORD val;																			\
	if (pwTable && *pwTable!=0xffff && DO_ISRELOCATED)									\
	{																					\
		WORD *_pwTable = pwTable;														\
		BOOL bSkipAddr;																	\
		DWORD *dptr,dwUndo,dwBase,dwOrig,dwBaseBitOffset;								\
		dwUndo = DO_GETUNDOOFFSET;														\
		dwBase = PtrToUlong(start_addr);												\
		DO_GETNEXTADDR(bSkipAddr, dwBase, _pwTable);									\
		for(val=seed, dptr=(DWORD *)start_addr; PtrToUlong(dptr)<PtrToUlong(end_addr); dptr++)	\
		{																				\
			dwOrig = *dptr;																\
			if (PtrToUlong(dptr)>=dwBase)													\
			{																			\
				dwBaseBitOffset = (PtrToUlong(dptr)-dwBase)<<3;								\
				if (bSkipAddr)															\
				{																		\
					if(dwBaseBitOffset==0)												\
						dwOrig = 0;														\
					else																\
						dwOrig &= -1<<(32-dwBaseBitOffset);								\
				}																		\
				else																	\
				{																		\
					if(dwBaseBitOffset==0)												\
					{																	\
						dwOrig += dwUndo;												\
					}																	\
					else																\
					{																	\
						dwOrig &= -1<<(32-dwBaseBitOffset);								\
						dwOrig |= (*((DWORD *)UlongToPtr(dwBase))+dwUndo)>>dwBaseBitOffset;			\
					}																	\
				}																		\
				DO_GETNEXTADDR(bSkipAddr, dwBase, _pwTable);							\
			}																			\
			if (PtrToUlong(dptr)>=dwBase-3)												\
			{																			\
				if (bSkipAddr)															\
					dwOrig &= (1<<((dwBase-PtrToUlong(dptr))<<3)) - 1;					\
				else																	\
					dwOrig += dwUndo<<((dwBase-PtrToUlong(dptr))<<3);					\
			}																			\
			val += dwOrig;																\
			val = (((val>>17)^(val>>11)^val)&0xff)|(val<<8);							\
		}																				\
	}																					\
	else																				\
		iviTR_CHECKSUM(val, start_addr, end_addr, seed);								\
	*(pval) = val;																		\
	iviTR_DUMP_CHECKSUM_LOG(val, seed);													\
}

#define DO_CHECKSUM1(start_addr, end_addr, seed, pval, pwTable)							\
{																						\
	DWORD val;																			\
	if (pwTable && *pwTable!=0xffff && DO_ISRELOCATED)									\
	{																					\
		WORD *_pwTable = pwTable;														\
		BOOL bSkipAddr;																	\
		DWORD *dptr,dwUndo,dwBase,dwOrig,dwBaseBitOffset;								\
		dwUndo = DO_GETUNDOOFFSET;														\
		dwBase = PtrToUlong(start_addr);												\
		DO_GETNEXTADDR(bSkipAddr, dwBase, _pwTable);									\
		for(val=seed, dptr=(DWORD *)start_addr; PtrToUlong(dptr)<PtrToUlong(end_addr); dptr++)	\
		{																				\
			dwOrig = *dptr;																\
			if (PtrToUlong(dptr)>=dwBase)													\
			{																			\
				dwBaseBitOffset = (PtrToUlong(dptr)-dwBase)<<3;								\
				if (bSkipAddr)															\
				{																		\
					if(dwBaseBitOffset==0)												\
						dwOrig = 0;														\
					else																\
						dwOrig &= -1<<(32-dwBaseBitOffset);								\
				}																		\
				else																	\
				{																		\
					if(dwBaseBitOffset==0)												\
					{																	\
						dwOrig += dwUndo;												\
					}																	\
					else																\
					{																	\
						dwOrig &= -1<<(32-dwBaseBitOffset);								\
						dwOrig |= (*((DWORD *)UlongToPtr(dwBase))+dwUndo)>>dwBaseBitOffset;			\
					}																	\
				}																		\
				DO_GETNEXTADDR(bSkipAddr, dwBase, _pwTable);							\
			}																			\
			if (PtrToUlong(dptr)>=dwBase-3)												\
			{																			\
				if (bSkipAddr)															\
					dwOrig &= (1<<((dwBase-PtrToUlong(dptr))<<3)) - 1;					\
				else																	\
					dwOrig += dwUndo<<((dwBase-PtrToUlong(dptr))<<3);					\
			}																			\
			val += dwOrig;																\
			iviTR_CHECKSUM_EXPRESS1(val)													\
		}																				\
	}																					\
	else																				\
		iviTR_CHECKSUM1(val, start_addr, end_addr, seed);								\
	*(pval) = val;																		\
	iviTR_DUMP_CHECKSUM_LOG(val, seed);													\
}

#define DO_CHECKSUM2(start_addr, end_addr, seed, pval, pwTable)							\
{																						\
	DWORD val;																			\
	if (pwTable && *pwTable!=0xffff && DO_ISRELOCATED)									\
	{																					\
		WORD *_pwTable = pwTable;														\
		BOOL bSkipAddr;																	\
		DWORD *dptr,dwUndo,dwBase,dwOrig,dwBaseBitOffset;								\
		dwUndo = DO_GETUNDOOFFSET;														\
		dwBase = PtrToUlong(start_addr);												\
		DO_GETNEXTADDR(bSkipAddr, dwBase, _pwTable);									\
		for(val=seed, dptr=(DWORD *)start_addr; PtrToUlong(dptr)<PtrToUlong(end_addr); dptr++)	\
		{																				\
			dwOrig = *dptr;																\
			if (PtrToUlong(dptr)>=dwBase)													\
			{																			\
				dwBaseBitOffset = (PtrToUlong(dptr)-dwBase)<<3;								\
				if (bSkipAddr)															\
				{																		\
					if(dwBaseBitOffset==0)												\
						dwOrig = 0;														\
					else																\
						dwOrig &= -1<<(32-dwBaseBitOffset);								\
				}																		\
				else																	\
				{																		\
					if(dwBaseBitOffset==0)												\
					{																	\
						dwOrig += dwUndo;												\
					}																	\
					else																\
					{																	\
						dwOrig &= -1<<(32-dwBaseBitOffset);								\
						dwOrig |= (*((DWORD *)UlongToPtr(dwBase))+dwUndo)>>dwBaseBitOffset;			\
					}																	\
				}																		\
				DO_GETNEXTADDR(bSkipAddr, dwBase, _pwTable);							\
			}																			\
			if (PtrToUlong(dptr)>=dwBase-3)												\
			{																			\
				if (bSkipAddr)															\
					dwOrig &= (1<<((dwBase-PtrToUlong(dptr))<<3)) - 1;					\
				else																	\
					dwOrig += dwUndo<<((dwBase-PtrToUlong(dptr))<<3);					\
			}																			\
			val += dwOrig;																\
			iviTR_CHECKSUM_EXPRESS2(val)													\
		}																				\
	}																					\
	else																				\
		iviTR_CHECKSUM2(val, start_addr, end_addr, seed);								\
	*(pval) = val;																		\
	iviTR_DUMP_CHECKSUM_LOG(val, seed);													\
}

#define DO_CHECKSUM3(start_addr, end_addr, seed, pval, pwTable)							\
{																						\
	DWORD val;																			\
	if (pwTable && *pwTable!=0xffff && DO_ISRELOCATED)									\
	{																					\
		WORD *_pwTable = pwTable;														\
		BOOL bSkipAddr;																	\
		DWORD *dptr,dwUndo,dwBase,dwOrig,dwBaseBitOffset;								\
		dwUndo = DO_GETUNDOOFFSET;														\
		dwBase = PtrToUlong(start_addr);												\
		DO_GETNEXTADDR(bSkipAddr, dwBase, _pwTable);									\
		for(val=seed, dptr=(DWORD *)start_addr; PtrToUlong(dptr)<PtrToUlong(end_addr); dptr++)	\
		{																				\
			dwOrig = *dptr;																\
			if (PtrToUlong(dptr)>=dwBase)													\
			{																			\
				dwBaseBitOffset = (PtrToUlong(dptr)-dwBase)<<3;								\
				if (bSkipAddr)															\
				{																		\
					if(dwBaseBitOffset==0)												\
						dwOrig = 0;														\
					else																\
						dwOrig &= -1<<(32-dwBaseBitOffset);								\
				}																		\
				else																	\
				{																		\
					if(dwBaseBitOffset==0)												\
					{																	\
						dwOrig += dwUndo;												\
					}																	\
					else																\
					{																	\
						dwOrig &= -1<<(32-dwBaseBitOffset);								\
						dwOrig |= (*((DWORD *)UlongToPtr(dwBase))+dwUndo)>>dwBaseBitOffset;			\
					}																	\
				}																		\
				DO_GETNEXTADDR(bSkipAddr, dwBase, _pwTable);							\
			}																			\
			if (PtrToUlong(dptr)>=dwBase-3)												\
			{																			\
				if (bSkipAddr)															\
					dwOrig &= (1<<((dwBase-PtrToUlong(dptr))<<3)) - 1;					\
				else																	\
					dwOrig += dwUndo<<((dwBase-PtrToUlong(dptr))<<3);					\
			}																			\
			val += dwOrig;																\
			iviTR_CHECKSUM_EXPRESS3(val)													\
		}																				\
	}																					\
	else																				\
		iviTR_CHECKSUM3(val, start_addr, end_addr, seed);								\
	*(pval) = val;																		\
	iviTR_DUMP_CHECKSUM_LOG(val, seed);													\
}

#define DO_CHECKSUM4(start_addr, end_addr, seed, pval, pwTable)							\
{																						\
	DWORD val;																			\
	if (pwTable && *pwTable!=0xffff && DO_ISRELOCATED)									\
	{																					\
		WORD *_pwTable = pwTable;														\
		BOOL bSkipAddr;																	\
		DWORD *dptr,dwUndo,dwBase,dwOrig,dwBaseBitOffset;								\
		dwUndo = DO_GETUNDOOFFSET;														\
		dwBase = PtrToUlong(start_addr);												\
		DO_GETNEXTADDR(bSkipAddr, dwBase, _pwTable);									\
		for(val=seed, dptr=(DWORD *)start_addr; PtrToUlong(dptr)<PtrToUlong(end_addr); dptr++)	\
		{																				\
			dwOrig = *dptr;																\
			if (PtrToUlong(dptr)>=dwBase)													\
			{																			\
				dwBaseBitOffset = (PtrToUlong(dptr)-dwBase)<<3;								\
				if (bSkipAddr)															\
				{																		\
					if(dwBaseBitOffset==0)												\
						dwOrig = 0;														\
					else																\
						dwOrig &= -1<<(32-dwBaseBitOffset);								\
				}																		\
				else																	\
				{																		\
					if(dwBaseBitOffset==0)												\
					{																	\
						dwOrig += dwUndo;												\
					}																	\
					else																\
					{																	\
						dwOrig &= -1<<(32-dwBaseBitOffset);								\
						dwOrig |= (*((DWORD *)UlongToPtr(dwBase))+dwUndo)>>dwBaseBitOffset;			\
					}																	\
				}																		\
				DO_GETNEXTADDR(bSkipAddr, dwBase, _pwTable);							\
			}																			\
			if (PtrToUlong(dptr)>=dwBase-3)												\
			{																			\
				if (bSkipAddr)															\
					dwOrig &= (1<<((dwBase-PtrToUlong(dptr))<<3)) - 1;					\
				else																	\
					dwOrig += dwUndo<<((dwBase-PtrToUlong(dptr))<<3);					\
			}																			\
			val += dwOrig;																\
			iviTR_CHECKSUM_EXPRESS4(val)													\
		}																				\
	}																					\
	else																				\
		iviTR_CHECKSUM4(val, start_addr, end_addr, seed);								\
	*(pval) = val;																		\
	iviTR_DUMP_CHECKSUM_LOG(val, seed);													\
}

#define DO_DESCRAMBLE(start_addr, end_addr, seed, pwTable)								\
{																						\
	DWORD val, *dptr;																	\
	if(pwTable && *pwTable!=0xffff && DO_ISRELOCATED)									\
	{																					\
		WORD *_pwTable = pwTable;														\
		BOOL bSkipAddr;																	\
		WORD *pwOrigTable;																\
		DWORD dwBase,dwMask,dwBaseBitOffset;											\
		pwOrigTable = pwTable;															\
		DO_RELOCATECODE(start_addr, pwOrigTable, TRUE);									\
		dwBase = PtrToUlong(start_addr);														\
		DO_GETNEXTADDR(bSkipAddr, dwBase, _pwTable);									\
		for(val=seed, dptr=(DWORD *)start_addr; PtrToUlong(dptr)<PtrToUlong(end_addr); dptr++)	\
		{																				\
			dwMask = 0xffffffff;														\
			if (PtrToUlong(dptr)>=dwBase)													\
			{																			\
				dwBaseBitOffset = (PtrToUlong(dptr)-dwBase)<<3;								\
				if (bSkipAddr)															\
				{																		\
					if(dwBaseBitOffset==0)												\
						dwMask = 0;														\
				else																	\
						dwMask &= -1<<(32-dwBaseBitOffset);								\
				}																		\
				DO_GETNEXTADDR(bSkipAddr, dwBase, _pwTable);							\
			}																			\
			if (PtrToUlong(dptr)>=dwBase-3)													\
			{																			\
				if (bSkipAddr)															\
					dwMask &= (1<<((dwBase-PtrToUlong(dptr))<<3)) - 1;						\
			}																			\
			*dptr ^= val & dwMask;														\
			val += *dptr & dwMask;														\
			val = (((val>>17)^(val>>11)^val)&0xff)|(val<<8);							\
		}																				\
		DO_RELOCATECODE(start_addr, pwOrigTable, FALSE);								\
	}																					\
	else																				\
	{																					\
		iviTR_DESCRAMBLE(start_addr,end_addr,seed);										\
	}																					\
}

#define DO_SCRAMBLE(start_addr, end_addr, seed, pwTable)								\
{																						\
	DWORD *dptr, val, oval;																\
	if (pwTable && *pwTable!=0xffff && DO_ISRELOCATED)									\
	{																					\
		WORD *_pwTable = pwTable;														\
		BOOL bSkipAddr;																	\
		DWORD dwBase,dwMask,dwBaseBitOffset;											\
		dwBase = PtrToUlong(start_addr);														\
		DO_GETNEXTADDR(bSkipAddr, dwBase, _pwTable);									\
		for(val=seed, dptr=(DWORD *)start_addr; PtrToUlong(dptr)<PtrToUlong(end_addr); dptr++)	\
		{																				\
			dwMask = 0xffffffff;														\
			if (PtrToUlong(dptr)>=dwBase)													\
			{																			\
				dwBaseBitOffset = (PtrToUlong(dptr)-dwBase)<<3;								\
				if (bSkipAddr)															\
				{																		\
					if(dwBaseBitOffset==0)												\
						dwMask = 0;														\
					else																\
						dwMask &= -1<<(32-dwBaseBitOffset);								\
				}																		\
				DO_GETNEXTADDR(bSkipAddr, dwBase, _pwTable);							\
			}																			\
			if (PtrToUlong(dptr)>=dwBase-3)													\
			{																			\
				if (bSkipAddr)															\
					dwMask &= (1<<((dwBase-PtrToUlong(dptr))<<3)) - 1;						\
			}																			\
			oval = val;																	\
			val += *dptr & dwMask;														\
			*dptr ^= oval & dwMask;														\
			val = (((val>>17)^(val>>11)^val)&0xff)|(val<<8);							\
		}																				\
	}																					\
	else																				\
	{																					\
		iviTR_SCRAMBLE(start_addr,end_addr,seed);											\
	}																					\
}

// #ifdef TR_TREXE_ENABLE
	struct TREXE_INITPARAMS
	{
		DWORD	dwSize;
		char	*pszRegionName;
		DWORD	dwStart;
		DWORD	dwRangeSize;
		DWORD	dwSeed;
		DWORD	dwExpectedChecksum;
		WORD	*pwRelocTable;
		WORD	*pwEIPTable;
		DWORD	dwDescramble;
	};

	#define iviTR_TREXE2_REGION_PFX "_@@TR_REGION_"
	///*!
	// The function calls this macro to indicate that it belongs to a region of TREXE protection.
	//
	// Return value - None
	//
	// -#GROUP - [IN] used to group the region by module (AACS, CODEC, ...)
	// -#REGION - [IN] represent which region need to be protected.
	//
	// Supported platform: All
	// Performance overhead: 
	// Security level: 
	//
	// Note:\n
	// 1. The macro must be placed at the beginning of the function.
	// 2. Do not use the macro in inline functions that use "inline", "__inline" or "__forceinline" specifiers.
	//
	// Usage:\n
	//		void somefunction()
	//		{
	//			iviTR_TREXE2_REGION_DECLARE(MODULEA, 18)
	//			...
	//		}
	//*/
	#define iviTR_TREXE2_REGION_DECLARE(GROUP, REGION) volatile LPSTR ___pRegion = iviTR_TREXE2_REGION_PFX#GROUP"_"#REGION; \
		__asm lea eax, ___pRegion

	///*!
	// The function calls this macro to indicate that it belongs to a region of TREXE protection in the specified order.
	// The order number has to be a positive number (e.g. 1, 2, or 3).  "1" specifies the function to be the first entry
	// in the region.  "2" specifies the second entry in the region and so on.
	//
	// Return value - None
	//
	// -#GROUP - [IN] used to group the region by module (AACS, CODEC, ...)
	// -#REGION - [IN] represent which region need to be protected.
	// -#ORDER - [IN] used to specify the order in the region.
	//
	// Supported platform: All
	// Performance overhead: 
	// Security level:
	//
	// Note:\n
	// 1. The macro must be placed at the beginning of the function.
	// 2. Do not use the macro in inline functions that use "inline", "__inline" or "__forceinline" specifiers.
	//
	// Usage:\n
	//		void somefunction()
	//		{
	//			iviTR_TREXE2_REGION_DECLARE(MODULEA, 20, 1)
	//			...
	//		}
	//*/
	#define iviTR_TREXE2_REGION_DECLARE_BY_ORDER(GROUP, REGION, ORDER) volatile LPSTR ___pRegion = iviTR_TREXE2_REGION_PFX#GROUP"_"#REGION"@_@"#ORDER; \
		__asm lea eax, ___pRegion		

	///*!
	// The source file places this macro to notify TREXE that "#pragma comment(lib" is used in a source file.
	// Use this to insure correct function order file generation.
	//
	// Return value - None
	//
	// -#GROUP - [IN] used to group the region by module(AACS, CODEC, ...)
	// -#REGION - [IN] represent which region need to be protected.
	//
	// Supported platform: All
	// Performance overhead: 
	// Security level:
	//
	// Note:\n
	// The macro is only needed once per source source.
	//
	// Usage:\n
	// iviTR_TREXE2_SPECIFY_LIB
	// #pragma comment(lib,"LibA.lib")
	//*/
	#define iviTR_TREXE2_SPECIFY_LIB

	#define iviTR_TREXE2_RUN_REGION_PFX "_!!TR_REGION_"

	///*!
	// Use it to identify the module used the suitable libTR with TRexe. 
	// TRTeam member should increase it one while modifying the following macors:
	// iviTR_TREXE2_CallDescramble, iviTR_TREXE2_CallScramble, 
	// iviTR_TREXE2_CallChecksum, iviTR_ChecksumRegion, 
	// iviTR_LIGHTWEIGHT_DESCRAMBLE2, and iviTR_LIGHTWEIGHT_SCRAMBLE2.
	// It forbids to be defined as 0
	//*/
	#define iviTR_TREXE2_VERSION 1

	///*!
	// Wraps the declaration of region for TREXE2.
	//
	// Return value - none.
	//
	// -#GROUP - [IN] used to group the region by module(AACS, CODEC, ...)
	// -#REGION - [IN] represent which region need to be protected.
	// -#dword_relocate - [IN] indicate the number of relocated items.
	//
	// Supported platform: All
	// Performance overhead: 
	// Security level: 
	//*/        
	#define iviTR_TREXE2_DECLARE(GROUP, REGION, dword_relocate)	\
		volatile WORD TR_EIPTable_##GROUP##REGION[dword_relocate * 10] = {0xffff};	\
		volatile DWORD TR_EIPTableSize_##GROUP##REGION = dword_relocate * 10;	\
		volatile WORD TR_RelocTable_##GROUP##REGION[dword_relocate] = {0xffff};	\
		volatile DWORD TR_RelocTableSize_##GROUP##REGION = dword_relocate;	\
		volatile DWORD TR_TREXE2_ID_##GROUP##_##REGION = iviTR_TREXE2_VERSION;

	///*!
	// Wraps the declaration of region for TREXE2.
	//
	// Return value - none.
	//
	// -#GROUP - [IN] used to group the region by module(AACS, CODEC, ...)
	// -#REGION - [IN] represent which region need to be protected.
	// -#dword_relocate - [IN] indicate the number of relocated items.
	//
	// Supported platform: All
	// Performance overhead: 
	// Security level: 
	//*/        
	#define iviTR_TREXE2_EXTERN(GROUP, REGION, dword_relocate)	\
		extern volatile WORD TR_EIPTable_##GROUP##REGION[dword_relocate * 10];	\
		extern volatile DWORD TR_EIPTableSize_##GROUP##REGION;	\
		extern volatile WORD TR_RelocTable_##GROUP##REGION[dword_relocate];	\
		extern volatile DWORD TR_RelocTableSize_##GROUP##REGION;	\
		extern volatile DWORD TR_TREXE2_ID_##GROUP##_##REGION;

	///*!
	// Wraps the declaration of region for TREXE2.  It is only for checking checksum.
	//
	// Return value - none.
	//
	// -#GROUP - [IN] used to group the region by module(AACS, CODEC, ...)
	// -#REGION - [IN] represent which region need to be protected.
	// -#dword_relocate - [IN] indicate the number of relocated items.
	//
	// Supported platform: All
	// Performance overhead: 
	// Security level: 
	//*/        
	#define iviTR_TREXE2_DECLARE_CHECKSUMONLY(GROUP, REGION, dword_relocate)	\
		volatile WORD TR_RelocTable_##GROUP##REGION[dword_relocate] = {0xffff};	\
		volatile DWORD TR_RelocTableSize_##GROUP##REGION = dword_relocate;	\
		volatile DWORD TR_TREXE2_ID_##GROUP##_##REGION = iviTR_TREXE2_VERSION;

	///*!
	// Wraps the declaration of region for TREXE2.  It is only for checking checksum.
	//
	// Return value - none.
	//
	// -#GROUP - [IN] used to group the region by module(AACS, CODEC, ...)
	// -#REGION - [IN] represent which region need to be protected.
	// -#dword_relocate - [IN] indicate the number of relocated items.
	//
	// Supported platform: All
	// Performance overhead: 
	// Security level: 
	//*/        
	#define iviTR_TREXE2_EXTERN_CHECKSUMONLY(GROUP, REGION, dword_relocate)	\
		extern volatile WORD TR_RelocTable_##GROUP##REGION[dword_relocate];	\
		extern volatile DWORD TR_RelocTableSize_##GROUP##REGION;	\
		extern volatile DWORD TR_TREXE2_ID_##GROUP##_##REGION;


#ifdef __cplusplus
	#define _TREXE_LOCKER_MANGR_GETINSTANCE() \
		CtrLockerManager *pLocker = CtrLockerManager::GetInstance();
	#define _TREXE_LOCKER_MANGR_LOCK(DWORD_KEY) \
		pLocker->Lock(DWORD_KEY);
	#define _TREXE_LOCKER_MANGR_UNLOCK(DWORD_KEY) \
		pLocker->Unlock(DWORD_KEY);
#else
	#define _TREXE_LOCKER_MANGR_GETINSTANCE()   
	#define _TREXE_LOCKER_MANGR_LOCK(DWORD_KEY)  
	#define _TREXE_LOCKER_MANGR_UNLOCK(DWORD_KEY)  
#endif

	///*!
	// The descramble operation for TREXE2. iviTR_TREXE2_DECLARE must 
	// be called, first.
	// Return value - none.
	//
	// -#GROUP - [IN] used to group the region by module(AACS, CODEC, ...)
	// -#REGION - [IN] represent which region need to be protected.
	// -#dword_uniquenum - [IN]  it must be different from other descramble operations with same region in the same function.
	//
	// Supported platform: All
	// Performance overhead: 
	// Security level: 
	//*/     
	#define iviTR_TREXE2_CallDescramble(GROUP, REGION, dword_uniquenum)	\
	{	\
		DWORD dwOffset = DO_GETUNDOOFFSET;	\
		_TREXE_LOCKER_MANGR_GETINSTANCE()	\
		TREXE_INITPARAMS param;	\
		param.pszRegionName = iviTR_TREXE2_RUN_REGION_PFX#GROUP"_"#REGION;	\
		param.pwRelocTable = (WORD*)TR_RelocTable_##GROUP##REGION;	\
		param.pwEIPTable = (WORD*)TR_EIPTable_##GROUP##REGION;	\
		param.dwDescramble = 0;	\
		param.dwStart = 0xfefefefe;	\
		param.dwRangeSize = 0xfefefefe;	\
		param.dwSeed = 0xfefefefe;	\
		param.dwSize = sizeof(TREXE_INITPARAMS);	\
		__asm {\
			__asm call trCall_Descramble_Entry	\
			__asm jmp	TRDescrambleout_##GROUP##REGION##dword_uniquenum	\
		}	\
		param.pwRelocTable = (WORD*)((DWORD)(param.pwRelocTable) + dwOffset);	\
		param.pwEIPTable = (WORD*)((DWORD)(param.pwEIPTable) + dwOffset);	\
		__asm	{\
			__asm mov eax, param.pwRelocTable	\
			__asm xor eax, 0xfcfcfcfc	\
			__asm mov param.pwRelocTable, eax	\
			__asm mov eax, param.pwEIPTable	\
			__asm xor eax, 0xfcfcfcfc	\
			__asm mov param.pwEIPTable, eax	\
			__asm mov eax, param.dwDescramble	\
			__asm xor eax, 0xfcfcfcfc	\
			__asm mov param.dwDescramble, eax	\
			__asm mov eax, param.dwStart	\
			__asm xor eax, 0xfcfcfcfc	\
			__asm mov param.dwStart, eax	\
			__asm mov eax, param.dwRangeSize	\
			__asm xor eax, 0xfcfcfcfc	\
			__asm mov param.dwRangeSize, eax	\
			__asm mov eax, param.dwSeed	\
			__asm xor eax, 0xfcfcfcfc	\
			__asm mov param.dwSeed, eax	\
		}\
		static DWORD dwScramble_##GROUP##REGION = __LINE__;\
		_TREXE_LOCKER_MANGR_LOCK((DWORD)&dwScramble_##GROUP##REGION)	\
		param.pwRelocTable = (WORD*)((DWORD)(param.pwRelocTable) - dwOffset);	\
		param.pwEIPTable = (WORD*)((DWORD)(param.pwEIPTable) - dwOffset);	\
		param.dwStart = param.dwStart - dwOffset;	\
 		WORD *pRelocTableEnd = param.pwRelocTable;	\
 		int nRelocIndex = 0;	\
 		while (*(param.pwRelocTable + nRelocIndex) != 0xffff)	\
 		{	\
 			nRelocIndex++;	\
 			pRelocTableEnd = param.pwRelocTable + nRelocIndex;	\
 		}	\
 		WORD wDescramble = *(pRelocTableEnd + param.dwDescramble);	\
 		if (wDescramble == 0)	\
		{	\
			void *pStart = (void*)(param.dwStart);	\
			void *pEnd = (void*)((DWORD)pStart + param.dwRangeSize);	\
			iviTR_LIGHTWEIGHT_DESCRAMBLE2(pStart, pEnd, param.dwSeed, param.pwRelocTable, param.pwEIPTable); \
			*(pRelocTableEnd + param.dwDescramble) = 1;\
		}	\
		_TREXE_LOCKER_MANGR_UNLOCK((DWORD)&dwScramble_##GROUP##REGION)	\
		__asm TRDescrambleout_##GROUP##REGION##dword_uniquenum:	\
	}

	///*!
	// The scramble operation for TREXE2. iviTR_TREXE2_DECLARE must 
	// be called, first.
	// Return value - none.
	//
	// -#GROUP - [IN] used to group the region by module(AACS, CODEC, ...)
	// -#REGION - [IN] represent which region need to be protected.
	// -#dword_uniquenum - [IN]  it must be different from other scramble operations with same region in the same function.
	//
	// Supported platform: All
	// Performance overhead: 
	// Security level: 
	//*/     
	#define iviTR_TREXE2_CallScramble(GROUP, REGION, dword_uniquenum)	\
	{	\
		DWORD dwOffset = DO_GETUNDOOFFSET;	\
		_TREXE_LOCKER_MANGR_GETINSTANCE()	\
		TREXE_INITPARAMS param;	\
		param.pszRegionName = iviTR_TREXE2_RUN_REGION_PFX#GROUP"_"#REGION;	\
		param.pwRelocTable = (WORD*)TR_RelocTable_##GROUP##REGION;	\
		param.pwEIPTable = (WORD*)TR_EIPTable_##GROUP##REGION;	\
		param.dwDescramble = 0;\
		param.dwStart = 0xfefefefe;	\
		param.dwRangeSize = 0xfefefefe;	\
		param.dwSeed = 0xfefefefe;	\
		param.dwSize = sizeof(TREXE_INITPARAMS);	\
		__asm {\
			__asm call trCall_Scramble_Entry	\
			__asm jmp	TRScrambleout_##GROUP##REGION##dword_uniquenum	\
		}	\
		param.pwRelocTable = (WORD*)((DWORD)(param.pwRelocTable) + dwOffset);	\
		param.pwEIPTable = (WORD*)((DWORD)(param.pwEIPTable) + dwOffset);	\
		__asm	{\
			__asm mov eax, param.pwRelocTable	\
			__asm xor eax, 0xfcfcfcfc	\
			__asm mov param.pwRelocTable, eax	\
			__asm mov eax, param.pwEIPTable	\
			__asm xor eax, 0xfcfcfcfc	\
			__asm mov param.pwEIPTable, eax	\
			__asm mov eax, param.dwDescramble	\
			__asm xor eax, 0xfcfcfcfc	\
			__asm mov param.dwDescramble, eax	\
			__asm mov eax, param.dwStart	\
			__asm xor eax, 0xfcfcfcfc	\
			__asm mov param.dwStart, eax	\
			__asm mov eax, param.dwRangeSize	\
			__asm xor eax, 0xfcfcfcfc	\
			__asm mov param.dwRangeSize, eax	\
			__asm mov eax, param.dwSeed	\
			__asm xor eax, 0xfcfcfcfc	\
			__asm mov param.dwSeed, eax	\
		}\
		static DWORD dwScramble_##GROUP##REGION = __LINE__;\
		_TREXE_LOCKER_MANGR_LOCK((DWORD)&dwScramble_##GROUP##REGION)	\
		param.pwRelocTable = (WORD*)((DWORD)(param.pwRelocTable) - dwOffset);	\
		param.pwEIPTable = (WORD*)((DWORD)(param.pwEIPTable) - dwOffset);	\
		param.dwStart = param.dwStart - dwOffset;	\
		WORD *pRelocTableEnd = param.pwRelocTable;	\
		int nRelocIndex = 0;	\
		while (*(param.pwRelocTable + nRelocIndex) != 0xffff)	\
		{	\
			nRelocIndex++;	\
			pRelocTableEnd = param.pwRelocTable + nRelocIndex;	\
		}	\
		WORD wDescramble = *(pRelocTableEnd + param.dwDescramble);	\
		if (wDescramble == 1)	\
		{	\
			void *pStart = (void*)(param.dwStart);	\
			void *pEnd = (void*)((DWORD)pStart + param.dwRangeSize);	\
			iviTR_LIGHTWEIGHT_SCRAMBLE2(pStart, pEnd, param.dwSeed, param.pwRelocTable, param.pwEIPTable); \
			*(pRelocTableEnd + param.dwDescramble) = 0;\
		}	\
		_TREXE_LOCKER_MANGR_UNLOCK((DWORD)&dwScramble_##GROUP##REGION)	\
		__asm TRScrambleout_##GROUP##REGION##dword_uniquenum:	\
	}

	///*!
	// The checksum operation for TREXE2. iviTR_TREXE2_DECLARE or iviTR_TREXE2_DECLARE_CHECKONLY must 
	// be called, first.
	// Return value - none.
	//
	// -#GROUP - [IN] used to group the region by module(AACS, CODEC, ...)
	// -#REGION - [IN] represent which region need to be protected.
	// -#dword_uniquenum - [IN] it must be different from other checksum operations with same region in the same function.
	// -#bool_return - [IN, OUT] return value, if true, represent that check checksum is OK, else means that
	// it fails. suggest to call iviTR_CRASH() to crash AP.
	//
	// Supported platform: All
	// Performance overhead: 
	// Security level: 
	//*/     
	#define iviTR_TREXE2_CallChecksum(GROUP, REGION, dword_uniquenum, bool_return)	\
	{	\
		DWORD dwOffset = DO_GETUNDOOFFSET;	\
		TREXE_INITPARAMS param;	\
		param.pszRegionName = iviTR_TREXE2_RUN_REGION_PFX#GROUP"_"#REGION;	\
		param.pwRelocTable = (WORD*)TR_RelocTable_##GROUP##REGION;	\
		param.dwStart = 0xfefefefe;	\
		param.dwRangeSize = 0xfefefefe;	\
		param.dwSeed = 0xfefefefe;	\
		param.dwExpectedChecksum = 0xfefefefe;	\
		param.dwSize = sizeof(TREXE_INITPARAMS);	\
		__asm {\
			__asm call trCall_Checksum_Entry	\
			__asm jmp	TRChecksumout_##GROUP##REGION##dword_uniquenum	\
		}	\
		param.pwRelocTable = (WORD*)((DWORD)(param.pwRelocTable) + dwOffset);	\
		__asm	{\
			__asm mov eax, param.pwRelocTable	\
			__asm xor eax, 0xfcfcfcfc	\
			__asm mov param.pwRelocTable, eax	\
			__asm mov eax, param.dwStart	\
			__asm xor eax, 0xfcfcfcfc	\
			__asm mov param.dwStart, eax	\
			__asm mov eax, param.dwRangeSize	\
			__asm xor eax, 0xfcfcfcfc	\
			__asm mov param.dwRangeSize, eax	\
			__asm mov eax, param.dwSeed	\
			__asm xor eax, 0xfcfcfcfc	\
			__asm mov param.dwSeed, eax	\
			__asm mov eax, param.dwExpectedChecksum	\
			__asm xor eax, 0xfcfcfcfc	\
			__asm mov param.dwExpectedChecksum, eax	\
		}\
		param.pwRelocTable = (WORD*)((DWORD)(param.pwRelocTable) - dwOffset);	\
		param.dwStart = param.dwStart - dwOffset;	\
		void *pStart = (void*)(param.dwStart);	\
		void *pEnd = (void*)((DWORD)pStart + param.dwRangeSize);	\
		DWORD dwChecksum = 0;	\
		iviTR_ChecksumRegion(pStart, pEnd, param.dwSeed, &dwChecksum, param.pwRelocTable);	\
 		WORD *pRelocTableEnd = param.pwRelocTable;	\
 		int nRelocIndex = 0;	\
 		while (*(param.pwRelocTable + nRelocIndex) != 0xffff)	\
 		{	\
 			nRelocIndex++;	\
 			pRelocTableEnd = param.pwRelocTable + nRelocIndex;	\
 		}	\
 		DWORD dwExpectedChecksum = MAKELONG(*(pRelocTableEnd + LOWORD(param.dwExpectedChecksum)), *(pRelocTableEnd + HIWORD(param.dwExpectedChecksum)));	\
 		if (dwChecksum != dwExpectedChecksum)	\
		{	\
			bool_return = false;	\
		}	\
		else	\
			bool_return = true;		\
		__asm TRChecksumout_##GROUP##REGION##dword_uniquenum:	\
	}

	///*!
	// The checksum operation for TREXE2. iviTR_TREXE2_DECLARE or iviTR_TREXE2_DECLARE_CHECKONLY must 
	// be called, first.
	// Return value - none.
	//
	// -#GROUP - [IN] used to group the region by module(AACS, CODEC, ...)
	// -#REGION - [IN] represent which region need to be protected.
	// -#dword_uniquenum - [IN] it must be different from other checksum operations with same region in the same function.
	// -#bool_return - [IN, OUT] return value, if true, represent that check checksum is OK, else means that
	// it fails. suggest to call iviTR_CRASH() to crash AP.
	//
	// Supported platform: All
	// Performance overhead: 
	// Security level: 
	//*/     
	#define iviTR_TREXE2_CallChecksum1(GROUP, REGION, dword_uniquenum, bool_return)	\
	{	\
		DWORD dwOffset = DO_GETUNDOOFFSET;	\
		TREXE_INITPARAMS param;	\
		param.pszRegionName = iviTR_TREXE2_RUN_REGION_PFX#GROUP"_"#REGION;	\
		param.pwRelocTable = (WORD*)TR_RelocTable_##GROUP##REGION;	\
		param.dwStart = 0xfefefefe;	\
		param.dwRangeSize = 0xfefefefe;	\
		param.dwSeed = 0xfefefefe;	\
		param.dwExpectedChecksum = 0xfefefefe;	\
		param.dwSize = sizeof(TREXE_INITPARAMS);	\
		__asm {\
			__asm call trCall_Checksum_Entry	\
			__asm jmp	TRChecksumout_##GROUP##REGION##dword_uniquenum	\
		}	\
		param.pwRelocTable = (WORD*)((DWORD)(param.pwRelocTable) + dwOffset);	\
		__asm	{\
			__asm mov eax, param.pwRelocTable	\
			__asm xor eax, 0xfcfcfcfc	\
			__asm mov param.pwRelocTable, eax	\
			__asm mov eax, param.dwStart	\
			__asm xor eax, 0xfcfcfcfc	\
			__asm mov param.dwStart, eax	\
			__asm mov eax, param.dwRangeSize	\
			__asm xor eax, 0xfcfcfcfc	\
			__asm mov param.dwRangeSize, eax	\
			__asm mov eax, param.dwSeed	\
			__asm xor eax, 0xfcfcfcfc	\
			__asm mov param.dwSeed, eax	\
			__asm mov eax, param.dwExpectedChecksum	\
			__asm xor eax, 0xfcfcfcfc	\
			__asm mov param.dwExpectedChecksum, eax	\
		}\
		param.pwRelocTable = (WORD*)((DWORD)(param.pwRelocTable) - dwOffset);	\
		param.dwStart = param.dwStart - dwOffset;	\
		void *pStart = (void*)(param.dwStart);	\
		void *pEnd = (void*)((DWORD)pStart + param.dwRangeSize);	\
		DWORD dwChecksum = 0;	\
		iviTR_ChecksumRegion1(pStart, pEnd, param.dwSeed, &dwChecksum, param.pwRelocTable);	\
 		WORD *pRelocTableEnd = param.pwRelocTable;	\
 		int nRelocIndex = 0;	\
 		while (*(param.pwRelocTable + nRelocIndex) != 0xffff)	\
 		{	\
 			nRelocIndex++;	\
 			pRelocTableEnd = param.pwRelocTable + nRelocIndex;	\
 		}	\
 		DWORD dwExpectedChecksum = MAKELONG(*(pRelocTableEnd + LOWORD(param.dwExpectedChecksum)), *(pRelocTableEnd + HIWORD(param.dwExpectedChecksum)));	\
 		if (dwChecksum != dwExpectedChecksum)	\
		{	\
			bool_return = false;	\
		}	\
		else	\
			bool_return = true;		\
		__asm TRChecksumout_##GROUP##REGION##dword_uniquenum:	\
	}

	///*!
	// The checksum operation for TREXE2. iviTR_TREXE2_DECLARE or iviTR_TREXE2_DECLARE_CHECKONLY must 
	// be called, first.
	// Return value - none.
	//
	// -#GROUP - [IN] used to group the region by module(AACS, CODEC, ...)
	// -#REGION - [IN] represent which region need to be protected.
	// -#dword_uniquenum - [IN] it must be different from other checksum operations with same region in the same function.
	// -#bool_return - [IN, OUT] return value, if true, represent that check checksum is OK, else means that
	// it fails. suggest to call iviTR_CRASH() to crash AP.
	//
	// Supported platform: All
	// Performance overhead: 
	// Security level: 
	//*/     
	#define iviTR_TREXE2_CallChecksum2(GROUP, REGION, dword_uniquenum, bool_return)	\
	{	\
		DWORD dwOffset = DO_GETUNDOOFFSET;	\
		TREXE_INITPARAMS param;	\
		param.pszRegionName = iviTR_TREXE2_RUN_REGION_PFX#GROUP"_"#REGION;	\
		param.pwRelocTable = (WORD*)TR_RelocTable_##GROUP##REGION;	\
		param.dwStart = 0xfefefefe;	\
		param.dwRangeSize = 0xfefefefe;	\
		param.dwSeed = 0xfefefefe;	\
		param.dwExpectedChecksum = 0xfefefefe;	\
		param.dwSize = sizeof(TREXE_INITPARAMS);	\
		__asm {\
			__asm call trCall_Checksum_Entry	\
			__asm jmp	TRChecksumout_##GROUP##REGION##dword_uniquenum	\
		}	\
		param.pwRelocTable = (WORD*)((DWORD)(param.pwRelocTable) + dwOffset);	\
		__asm	{\
			__asm mov eax, param.pwRelocTable	\
			__asm xor eax, 0xfcfcfcfc	\
			__asm mov param.pwRelocTable, eax	\
			__asm mov eax, param.dwStart	\
			__asm xor eax, 0xfcfcfcfc	\
			__asm mov param.dwStart, eax	\
			__asm mov eax, param.dwRangeSize	\
			__asm xor eax, 0xfcfcfcfc	\
			__asm mov param.dwRangeSize, eax	\
			__asm mov eax, param.dwSeed	\
			__asm xor eax, 0xfcfcfcfc	\
			__asm mov param.dwSeed, eax	\
			__asm mov eax, param.dwExpectedChecksum	\
			__asm xor eax, 0xfcfcfcfc	\
			__asm mov param.dwExpectedChecksum, eax	\
		}\
		param.pwRelocTable = (WORD*)((DWORD)(param.pwRelocTable) - dwOffset);	\
		param.dwStart = param.dwStart - dwOffset;	\
		void *pStart = (void*)(param.dwStart);	\
		void *pEnd = (void*)((DWORD)pStart + param.dwRangeSize);	\
		DWORD dwChecksum = 0;	\
		iviTR_ChecksumRegion2(pStart, pEnd, param.dwSeed, &dwChecksum, param.pwRelocTable);	\
 		WORD *pRelocTableEnd = param.pwRelocTable;	\
 		int nRelocIndex = 0;	\
 		while (*(param.pwRelocTable + nRelocIndex) != 0xffff)	\
 		{	\
 			nRelocIndex++;	\
 			pRelocTableEnd = param.pwRelocTable + nRelocIndex;	\
 		}	\
 		DWORD dwExpectedChecksum = MAKELONG(*(pRelocTableEnd + LOWORD(param.dwExpectedChecksum)), *(pRelocTableEnd + HIWORD(param.dwExpectedChecksum)));	\
 		if (dwChecksum != dwExpectedChecksum)	\
		{	\
			bool_return = false;	\
		}	\
		else	\
			bool_return = true;		\
		__asm TRChecksumout_##GROUP##REGION##dword_uniquenum:	\
	}

	///*!
	// The checksum operation for TREXE2. iviTR_TREXE2_DECLARE or iviTR_TREXE2_DECLARE_CHECKONLY must 
	// be called, first.
	// Return value - none.
	//
	// -#GROUP - [IN] used to group the region by module(AACS, CODEC, ...)
	// -#REGION - [IN] represent which region need to be protected.
	// -#dword_uniquenum - [IN] it must be different from other checksum operations with same region in the same function.
	// -#bool_return - [IN, OUT] return value, if true, represent that check checksum is OK, else means that
	// it fails. suggest to call iviTR_CRASH() to crash AP.
	//
	// Supported platform: All
	// Performance overhead: 
	// Security level: 
	//*/     
	#define iviTR_TREXE2_CallChecksum3(GROUP, REGION, dword_uniquenum, bool_return)	\
	{	\
		DWORD dwOffset = DO_GETUNDOOFFSET;	\
		TREXE_INITPARAMS param;	\
		param.pszRegionName = iviTR_TREXE2_RUN_REGION_PFX#GROUP"_"#REGION;	\
		param.pwRelocTable = (WORD*)TR_RelocTable_##GROUP##REGION;	\
		param.dwStart = 0xfefefefe;	\
		param.dwRangeSize = 0xfefefefe;	\
		param.dwSeed = 0xfefefefe;	\
		param.dwExpectedChecksum = 0xfefefefe;	\
		param.dwSize = sizeof(TREXE_INITPARAMS);	\
		__asm {\
			__asm call trCall_Checksum_Entry	\
			__asm jmp	TRChecksumout_##GROUP##REGION##dword_uniquenum	\
		}	\
		param.pwRelocTable = (WORD*)((DWORD)(param.pwRelocTable) + dwOffset);	\
		__asm	{\
			__asm mov eax, param.pwRelocTable	\
			__asm xor eax, 0xfcfcfcfc	\
			__asm mov param.pwRelocTable, eax	\
			__asm mov eax, param.dwStart	\
			__asm xor eax, 0xfcfcfcfc	\
			__asm mov param.dwStart, eax	\
			__asm mov eax, param.dwRangeSize	\
			__asm xor eax, 0xfcfcfcfc	\
			__asm mov param.dwRangeSize, eax	\
			__asm mov eax, param.dwSeed	\
			__asm xor eax, 0xfcfcfcfc	\
			__asm mov param.dwSeed, eax	\
			__asm mov eax, param.dwExpectedChecksum	\
			__asm xor eax, 0xfcfcfcfc	\
			__asm mov param.dwExpectedChecksum, eax	\
		}\
		param.pwRelocTable = (WORD*)((DWORD)(param.pwRelocTable) - dwOffset);	\
		param.dwStart = param.dwStart - dwOffset;	\
		void *pStart = (void*)(param.dwStart);	\
		void *pEnd = (void*)((DWORD)pStart + param.dwRangeSize);	\
		DWORD dwChecksum = 0;	\
		iviTR_ChecksumRegion3(pStart, pEnd, param.dwSeed, &dwChecksum, param.pwRelocTable);	\
 		WORD *pRelocTableEnd = param.pwRelocTable;	\
 		int nRelocIndex = 0;	\
 		while (*(param.pwRelocTable + nRelocIndex) != 0xffff)	\
 		{	\
 			nRelocIndex++;	\
 			pRelocTableEnd = param.pwRelocTable + nRelocIndex;	\
 		}	\
 		DWORD dwExpectedChecksum = MAKELONG(*(pRelocTableEnd + LOWORD(param.dwExpectedChecksum)), *(pRelocTableEnd + HIWORD(param.dwExpectedChecksum)));	\
 		if (dwChecksum != dwExpectedChecksum)	\
		{	\
			bool_return = false;	\
		}	\
		else	\
			bool_return = true;		\
		__asm TRChecksumout_##GROUP##REGION##dword_uniquenum:	\
	}

	///*!
	// The checksum operation for TREXE2. iviTR_TREXE2_DECLARE or iviTR_TREXE2_DECLARE_CHECKONLY must 
	// be called, first.
	// Return value - none.
	//
	// -#GROUP - [IN] used to group the region by module(AACS, CODEC, ...)
	// -#REGION - [IN] represent which region need to be protected.
	// -#dword_uniquenum - [IN] it must be different from other checksum operations with same region in the same function.
	// -#bool_return - [IN, OUT] return value, if true, represent that check checksum is OK, else means that
	// it fails. suggest to call iviTR_CRASH() to crash AP.
	//
	// Supported platform: All
	// Performance overhead: 
	// Security level: 
	//*/     
	#define iviTR_TREXE2_CallChecksum4(GROUP, REGION, dword_uniquenum, bool_return)	\
	{	\
		DWORD dwOffset = DO_GETUNDOOFFSET;	\
		TREXE_INITPARAMS param;	\
		param.pszRegionName = iviTR_TREXE2_RUN_REGION_PFX#GROUP"_"#REGION;	\
		param.pwRelocTable = (WORD*)TR_RelocTable_##GROUP##REGION;	\
		param.dwStart = 0xfefefefe;	\
		param.dwRangeSize = 0xfefefefe;	\
		param.dwSeed = 0xfefefefe;	\
		param.dwExpectedChecksum = 0xfefefefe;	\
		param.dwSize = sizeof(TREXE_INITPARAMS);	\
		__asm {\
			__asm call trCall_Checksum_Entry	\
			__asm jmp	TRChecksumout_##GROUP##REGION##dword_uniquenum	\
		}	\
		param.pwRelocTable = (WORD*)((DWORD)(param.pwRelocTable) + dwOffset);	\
		__asm	{\
			__asm mov eax, param.pwRelocTable	\
			__asm xor eax, 0xfcfcfcfc	\
			__asm mov param.pwRelocTable, eax	\
			__asm mov eax, param.dwStart	\
			__asm xor eax, 0xfcfcfcfc	\
			__asm mov param.dwStart, eax	\
			__asm mov eax, param.dwRangeSize	\
			__asm xor eax, 0xfcfcfcfc	\
			__asm mov param.dwRangeSize, eax	\
			__asm mov eax, param.dwSeed	\
			__asm xor eax, 0xfcfcfcfc	\
			__asm mov param.dwSeed, eax	\
			__asm mov eax, param.dwExpectedChecksum	\
			__asm xor eax, 0xfcfcfcfc	\
			__asm mov param.dwExpectedChecksum, eax	\
		}\
		param.pwRelocTable = (WORD*)((DWORD)(param.pwRelocTable) - dwOffset);	\
		param.dwStart = param.dwStart - dwOffset;	\
		void *pStart = (void*)(param.dwStart);	\
		void *pEnd = (void*)((DWORD)pStart + param.dwRangeSize);	\
		DWORD dwChecksum = 0;	\
		iviTR_ChecksumRegion4(pStart, pEnd, param.dwSeed, &dwChecksum, param.pwRelocTable);	\
 		WORD *pRelocTableEnd = param.pwRelocTable;	\
 		int nRelocIndex = 0;	\
 		while (*(param.pwRelocTable + nRelocIndex) != 0xffff)	\
 		{	\
 			nRelocIndex++;	\
 			pRelocTableEnd = param.pwRelocTable + nRelocIndex;	\
 		}	\
 		DWORD dwExpectedChecksum = MAKELONG(*(pRelocTableEnd + LOWORD(param.dwExpectedChecksum)), *(pRelocTableEnd + HIWORD(param.dwExpectedChecksum)));	\
 		if (dwChecksum != dwExpectedChecksum)	\
		{	\
			bool_return = false;	\
		}	\
		else	\
			bool_return = true;		\
		__asm TRChecksumout_##GROUP##REGION##dword_uniquenum:	\
	}

	///*!
	// Gather TR_RegionBegin_xx and TR_RegionEnd_xx for TREXE macros.
	//
	// Return value - none.
	// 
	// Supported platform: All
	// Performance overhead: 
	// Security level: 
	//*/        
	#define iviTR_TREXE_REGION_BOUNDARY(region_num)	\
		int __declspec(naked) TR_RegionBegin_##region_num() \
		{ \
			__asm push ebp	\
			__asm mov ebp, esp	\
			__asm sub esp, 0x08	\
			__asm mov eax, region_num	\
			__asm mov [esp - 4], eax	\
			__asm push esi	\
			__asm push edi	\
			__asm test edi, edi	\
			__asm add ecx, 0x08	\
			__asm xor esi, 0xffffffff	\
			__asm push eax	\
			__asm push ebx	\
			__asm push ecx	\
			__asm push edx	\
			__asm pop ebp	\
			__asm ret	\
		} \
		\
		int __declspec(naked) TR_RegionEnd_##region_num() \
		{ \
			__asm push ebp	\
			__asm mov ebp, esp	\
			__asm sub esp, 0x08	\
			__asm mov ebx, region_num	\
			__asm mov [esp - 4], eax	\
			__asm push esi	\
			__asm push edi	\
			__asm test esi, esi	\
			__asm add eax, 0x08	\
			__asm xor edi, 0xffffffff	\
			__asm push ebx	\
			__asm push eax	\
			__asm push edx	\
			__asm push ecx	\
			__asm pop ebp	\
			__asm ret	\
		}
	///*!
	// wrap the declaration of region for TRExe. it is only for checking checksum.
	//
	// Return value - none.
	// 
	// -# region_num - [IN] represent the region which you want to apply TRExe.
	// -# dword_relocate - [IN] indicate the number of relocated items.
	//
	// Supported platform: All
	// Performance overhead: 
	// Security level: 
	//*/        
	#define iviTR_TREXE_DECLARE_CHECKSUMONLY(region_num, dword_relocate) \
		volatile DWORD	TR_ChecksumEnable_##region_num	= region_num ^ 101;  \
		volatile DWORD	TR_ChecksumValue_##region_num = TR_CHECKSUM_VALUE_##region_num; \
		DWORD	TR_RelocateSize_##region_num =	dword_relocate; \
		WORD	TR_RelocateTable_##region_num[dword_relocate] = {0xffff}; \
		iviTR_TREXE_REGION_BOUNDARY(region_num)	
	///*!
	// extern the declaration of region for TRExe and call this after calling iviTR_TREXE_DECLARE_CHECKSUMONLY
	// at other place within the same module. it is only for checking checksum.
	//
	// Return value - none.
	// 
	// -# region_num - [IN] represent the region which you want to apply TRExe.
	// -# dword_relocate - [IN] indicate the number of relocated items.
	//
	// Supported platform: All
	// Performance overhead: 
	// Security level: 
	//*/        
	#define iviTR_TREXE_EXTERN_CHECKSUMONLY(region_num, dword_relocate) \
		extern volatile DWORD	TR_ChecksumEnable_##region_num;	\
		extern volatile DWORD	TR_ChecksumValue_##region_num;	\
		extern DWORD	TR_RelocateSize_##region_num;	\
		extern WORD	TR_RelocateTable_##region_num[dword_relocate];	\
		extern int TR_RegionBegin_##region_num(); \
		extern int TR_RegionEnd_##region_num();

#ifdef iviTR_ENABLE_LIGHTWEIGHT_SCRAMBLE
	///*!
	// [Light-Weight Scramble] wrap the declaration of region for TRExe. it is only for scramble case.
	//
	// Return value - none.
	// 
	// -# region_num - [IN] represent the region which you want to apply TRExe.
	// -# dword_relocate - [IN] indicate the number of relocated items.
	// -# dword_scrambleeip - [IN] indicate the number of scrambled instruction. it is only usable with Light-weight Scramble, and set it 0 with another.
	//
	// Supported platform: All
	// Performance overhead: 
	// Security level: 
	//*/        
	#define iviTR_TREXE_DECLARE_SCRAMBLEONLY(region_num, dword_relocate)	\
		volatile DWORD	TR_ScrambleEnable_##region_num	= region_num^111; \
		DWORD	TR_RelocateSize_##region_num	= dword_relocate;	\
		WORD	TR_RelocateTable_##region_num[dword_relocate] = {0xffff};	\
		DWORD	TR_ScrambleEIPTableSize_##region_num = dword_relocate * 10;	\
		WORD	TR_ScrambleEIPTable_##region_num[dword_relocate * 10] = {0xffff};	\
		iviTR_TREXE_REGION_BOUNDARY(region_num)
#else
	///*!
	// wrap the declaration of region for TRExe. it is only for scramble case.
	//
	// Return value - none.
	// 
	// -# region_num - [IN] represent the region which you want to apply TRExe.
	// -# dword_relocate - [IN] indicate the number of relocated items.
	//
	// Supported platform: All
	// Performance overhead: 
	// Security level: 
	//*/        
	#define iviTR_TREXE_DECLARE_SCRAMBLEONLY(region_num, dword_relocate)	\
		volatile DWORD	TR_ScrambleEnable_##region_num	= region_num^111; \
		DWORD	TR_RelocateSize_##region_num	= dword_relocate;	\
		WORD	TR_RelocateTable_##region_num[dword_relocate] = {0xffff};	\
		iviTR_TREXE_REGION_BOUNDARY(region_num)
#endif

#ifdef iviTR_ENABLE_LIGHTWEIGHT_SCRAMBLE
	///*!
	// [Light-Weight Scramble] extern the declaration of region for TRExe and call this after calling iviTR_TREXE_DECLARE_SCRAMBLEONLY 
	// at other place within the same module. it is only for scramble case.
	//
	// Return value - none.
	// 
	// -# region_num - [IN] represent the region which you want to apply TRExe.
	// -# dword_relocate - [IN] indicate the number of relocated items.
	// -# dword_scrambleeip - [IN] indicate the number of scrambled instruction. it is only usable with Light-weight Scramble, and set it 0 with another.
	//
	// Supported platform: All
	// Performance overhead: 
	// Security level: 
	//*/        
	#define iviTR_TREXE_EXTERN_SCRAMBLEONLY(region_num, dword_relocate) \
		extern volatile DWORD	TR_ScrambleEnable_##region_num; \
		extern DWORD	TR_RelocateSize_##region_num;	\
		extern WORD	TR_RelocateTable_##region_num[dword_relocate];	\
		extern int TR_RegionBegin_##region_num(); \
		extern int TR_RegionEnd_##region_num();	\
		extern	DWORD	TR_ScrambleEIPTableSize_##region_num;	\
		extern	WORD	TR_ScrambleEIPTable_##region_num[dword_relocate * 10];	
#else
	///*!
	// extern the declaration of region for TRExe and call this after calling iviTR_TREXE_DECLARE_SCRAMBLEONLY 
	// at other place within the same module. it is only for scramble case.
	//
	// Return value - none.
	// 
	// -# region_num - [IN] represent the region which you want to apply TRExe.
	// -# dword_relocate - [IN] indicate the number of relocated items.
	//
	// Supported platform: All
	// Performance overhead: 
	// Security level: 
	//*/        
	#define iviTR_TREXE_EXTERN_SCRAMBLEONLY(region_num, dword_relocate) \
		extern volatile DWORD	TR_ScrambleEnable_##region_num; \
		extern DWORD	TR_RelocateSize_##region_num;	\
		extern WORD	TR_RelocateTable_##region_num[dword_relocate];	\
		extern int TR_RegionBegin_##region_num(); \
		extern int TR_RegionEnd_##region_num();
#endif

#ifdef iviTR_ENABLE_LIGHTWEIGHT_SCRAMBLE
	///*!
	// [Light-Weight Scramble] wrap the declaration of region for TRExe.
	//
	// Return value - none.
	// 
	// -# region_num - [IN] represent the region which you want to apply TRExe.
	// -# dword_relocate - [IN] indicate the number of relocated items.
	// -# dword_scrambleeip - [IN] indicate the number of scrambled instruction. it is only usable with Light-weight Scramble, and set it 0 with another.
	//
	// Supported platform: All
	// Performance overhead: 
	// Security level: 
	//*/        
	#define iviTR_TREXE_DECLARE(region_num, dword_relocate)	\
		volatile DWORD	TR_ScrambleEnable_##region_num = region_num^121;	\
		volatile DWORD	TR_ChecksumEnable_##region_num = region_num^131;	\
		volatile DWORD	TR_ChecksumValue_##region_num  = TR_CHECKSUM_VALUE_##region_num;	\
		DWORD	TR_RelocateSize_##region_num = dword_relocate;	\
		WORD	TR_RelocateTable_##region_num[dword_relocate] = {0xffff};	\
		DWORD	TR_ScrambleEIPTableSize_##region_num = dword_relocate * 10;	\
		WORD	TR_ScrambleEIPTable_##region_num[dword_relocate * 10] = {0xffff};	\
		iviTR_TREXE_REGION_BOUNDARY(region_num)
#else
	///*!
	// wrap the declaration of region for TRExe.
	//
	// Return value - none.
	// 
	// -# region_num - [IN] represent the region which you want to apply TRExe.
	// -# dword_relocate - [IN] indicate the number of relocated items.
	//
	// Supported platform: All
	// Performance overhead: 
	// Security level: 
	//*/        
	#define iviTR_TREXE_DECLARE(region_num, dword_relocate)	\
		volatile DWORD	TR_ScrambleEnable_##region_num = region_num^121;	\
		volatile DWORD	TR_ChecksumEnable_##region_num = region_num^131;	\
		volatile DWORD	TR_ChecksumValue_##region_num  = TR_CHECKSUM_VALUE_##region_num;	\
		DWORD	TR_RelocateSize_##region_num = dword_relocate;	\
		WORD	TR_RelocateTable_##region_num[dword_relocate] = {0xffff};	\
		iviTR_TREXE_REGION_BOUNDARY(region_num)
#endif

#ifdef iviTR_ENABLE_LIGHTWEIGHT_SCRAMBLE
	///*!
	// [Light-Weight Scramble] extern the declaration of region for TRExe and call this after calling iviTR_TREXE_DECLARE at other
	// place within the same module.
	//
	// Return value - none.
	// 
	// -# region_num - [IN] represent the region which you want to apply TRExe.
	// -# dword_relocate - [IN] indicate the number of relocated items.
	// -# dword_scrambleeip - [IN] indicate the number of scrambled instruction. it is only usable with Light-weight Scramble, and set it 0 with another.
	//
	// Supported platform: All
	// Performance overhead: 
	// Security level: 
	//*/        
	#define iviTR_TREXE_EXTERN(region_num, dword_relocate) \
		extern volatile DWORD	TR_ScrambleEnable_##region_num; \
		extern volatile DWORD	TR_ChecksumEnable_##region_num;	\
		extern volatile DWORD	TR_ChecksumValue_##region_num;	\
		extern DWORD	TR_RelocateSize_##region_num;	\
		extern WORD	TR_RelocateTable_##region_num[dword_relocate];	\
		extern int TR_RegionBegin_##region_num(); \
		extern int TR_RegionEnd_##region_num(); \
		extern	DWORD	TR_ScrambleEIPTableSize_##region_num;	\
		extern	WORD	TR_ScrambleEIPTable_##region_num[dword_relocate * 10];	
#else
	///*!
	// extern the declaration of region for TRExe and call this after calling iviTR_TREXE_DECLARE at other
	// place within the same module.
	//
	// Return value - none.
	// 
	// -# region_num - [IN] represent the region which you want to apply TRExe.
	// -# dword_relocate - [IN] indicate the number of relocated items.
	//
	// Supported platform: All
	// Performance overhead: 
	// Security level: 
	//*/        
	#define iviTR_TREXE_EXTERN(region_num, dword_relocate) \
		extern volatile DWORD	TR_ScrambleEnable_##region_num; \
		extern volatile DWORD	TR_ChecksumEnable_##region_num;	\
		extern volatile DWORD	TR_ChecksumValue_##region_num;	\
		extern DWORD	TR_RelocateSize_##region_num;	\
		extern WORD	TR_RelocateTable_##region_num[dword_relocate];	\
		extern int TR_RegionBegin_##region_num(); \
		extern int TR_RegionEnd_##region_num();

#endif

	///*!
	// the checksum operation. the macro of iviTR_TREXE_DECLARE or iviTR_TREXE_DECLARE_CHECKSUMONLY must 
	// be called, first.
	// Return value - none.
	// 
	// -# region_num - [IN] represent the region which you want to apply TRExe.
	// -# bool_return - [IN, OUT] return value, if true, represent that check checksum is OK, else means that
	// it fails. suggest to call iviTR_CRASH() to crash AP.
	//
	// Supported platform: All
	// Performance overhead: 
	// Security level: 
	//*/        
	#define iviTR_TREXE_CHECK_CHECKSUM(region_num, bool_return) \
	{	\
		unsigned long checksum = 0; \
		if(TR_ChecksumEnable_##region_num == TR_CHECKSUM_ENABLE_##region_num)	\
		{	\
			iviTR_ChecksumRegion((void *)TR_RegionBegin_##region_num, (void *)TR_RegionEnd_##region_num, TR_CHECKSUM_SEED_##region_num, &checksum, TR_RelocateTable_##region_num);	\
			if ((TR_ChecksumValue_##region_num^checksum)!=TR_CHECKSUM_VALUE_##region_num)	\
			{	\
				iviTR_DUMP_CHECKSUM_LOG_ERR(checksum, TR_CHECKSUM_SEED_##region_num);	\
				bool_return = false;	\
			} else	\
				bool_return = true; \
		}	\
	}

	///*!
	// the checksum operation. the macro of iviTR_TREXE_DECLARE or iviTR_TREXE_DECLARE_CHECKSUMONLY must 
	// be called, first.
	// Return value - none.
	// 
	// -# region_num - [IN] represent the region which you want to apply TRExe.
	// -# bool_return - [IN, OUT] return value, if true, represent that check checksum is OK, else means that
	// it fails. suggest to call iviTR_CRASH() to crash AP.
	//
	// Supported platform: All
	// Performance overhead: 
	// Security level: 
	//*/        
	#define iviTR_TREXE_CHECK_CHECKSUM1(region_num, bool_return) \
	{	\
		unsigned long checksum = 0; \
		if(TR_ChecksumEnable_##region_num == TR_CHECKSUM_ENABLE_##region_num)	\
		{	\
			iviTR_ChecksumRegion1((void *)TR_RegionBegin_##region_num, (void *)TR_RegionEnd_##region_num, TR_CHECKSUM_SEED_##region_num, &checksum, TR_RelocateTable_##region_num);	\
			if ((TR_ChecksumValue_##region_num^checksum)!=TR_CHECKSUM_VALUE_##region_num)	\
			{	\
				iviTR_DUMP_CHECKSUM_LOG_ERR(checksum, TR_CHECKSUM_SEED_##region_num);	\
				bool_return = false;	\
			} else	\
				bool_return = true; \
		}	\
	}	

	///*!
	// the checksum operation. the macro of iviTR_TREXE_DECLARE or iviTR_TREXE_DECLARE_CHECKSUMONLY must 
	// be called, first.
	// Return value - none.
	// 
	// -# region_num - [IN] represent the region which you want to apply TRExe.
	// -# bool_return - [IN, OUT] return value, if true, represent that check checksum is OK, else means that
	// it fails. suggest to call iviTR_CRASH() to crash AP.
	//
	// Supported platform: All
	// Performance overhead: 
	// Security level: 
	//*/        
	#define iviTR_TREXE_CHECK_CHECKSUM2(region_num, bool_return) \
	{	\
		unsigned long checksum = 0; \
		if(TR_ChecksumEnable_##region_num == TR_CHECKSUM_ENABLE_##region_num)	\
		{	\
			iviTR_ChecksumRegion2((void *)TR_RegionBegin_##region_num, (void *)TR_RegionEnd_##region_num, TR_CHECKSUM_SEED_##region_num, &checksum, TR_RelocateTable_##region_num);	\
			if ((TR_ChecksumValue_##region_num^checksum)!=TR_CHECKSUM_VALUE_##region_num)	\
			{	\
				iviTR_DUMP_CHECKSUM_LOG_ERR(checksum, TR_CHECKSUM_SEED_##region_num);	\
				bool_return = false;	\
			} else	\
				bool_return = true; \
		}	\
	}	

	///*!
	// the checksum operation. the macro of iviTR_TREXE_DECLARE or iviTR_TREXE_DECLARE_CHECKSUMONLY must 
	// be called, first.
	// Return value - none.
	// 
	// -# region_num - [IN] represent the region which you want to apply TRExe.
	// -# bool_return - [IN, OUT] return value, if true, represent that check checksum is OK, else means that
	// it fails. suggest to call iviTR_CRASH() to crash AP.
	//
	// Supported platform: All
	// Performance overhead: 
	// Security level: 
	//*/        
	#define iviTR_TREXE_CHECK_CHECKSUM3(region_num, bool_return) \
	{	\
		unsigned long checksum = 0; \
		if(TR_ChecksumEnable_##region_num == TR_CHECKSUM_ENABLE_##region_num)	\
		{	\
			iviTR_ChecksumRegion3((void *)TR_RegionBegin_##region_num, (void *)TR_RegionEnd_##region_num, TR_CHECKSUM_SEED_##region_num, &checksum, TR_RelocateTable_##region_num);	\
			if ((TR_ChecksumValue_##region_num^checksum)!=TR_CHECKSUM_VALUE_##region_num)	\
			{	\
				iviTR_DUMP_CHECKSUM_LOG_ERR(checksum, TR_CHECKSUM_SEED_##region_num);	\
				bool_return = false;	\
			} else	\
				bool_return = true; \
		}	\
	}	

	///*!
	// the checksum operation. the macro of iviTR_TREXE_DECLARE or iviTR_TREXE_DECLARE_CHECKSUMONLY must 
	// be called, first.
	// Return value - none.
	// 
	// -# region_num - [IN] represent the region which you want to apply TRExe.
	// -# bool_return - [IN, OUT] return value, if true, represent that check checksum is OK, else means that
	// it fails. suggest to call iviTR_CRASH() to crash AP.
	//
	// Supported platform: All
	// Performance overhead: 
	// Security level: 
	//*/        
	#define iviTR_TREXE_CHECK_CHECKSUM4(region_num, bool_return) \
	{	\
		unsigned long checksum = 0; \
		if(TR_ChecksumEnable_##region_num == TR_CHECKSUM_ENABLE_##region_num)	\
		{	\
			iviTR_ChecksumRegion4((void *)TR_RegionBegin_##region_num, (void *)TR_RegionEnd_##region_num, TR_CHECKSUM_SEED_##region_num, &checksum, TR_RelocateTable_##region_num);	\
			if ((TR_ChecksumValue_##region_num^checksum)!=TR_CHECKSUM_VALUE_##region_num)	\
			{	\
				iviTR_DUMP_CHECKSUM_LOG_ERR(checksum, TR_CHECKSUM_SEED_##region_num);	\
				bool_return = false;	\
			} else	\
				bool_return = true; \
		}	\
	}	

	///*!
	// the descramble operation. the macro of iviTR_TREXE_DECLARE or iviTR_TREXE_DECLARE_SCRAMBLEONLY must 
	// be called, first.
	// Return value - none.
	// 
	// -# region_num - [IN] represent the region which you want to apply TRExe.
	//
	// Supported platform: All
	// Performance overhead: 
	// Security level: 
	//*/     
#ifdef iviTR_ENABLE_LIGHTWEIGHT_SCRAMBLE
	#define iviTR_TREXE_DESCRAMBLE(region_num) \
	{	\
		_TREXE_LOCKER_MANGR_GETINSTANCE()	\
		static DWORD dwScramble##region_num = __LINE__;\
		_TREXE_LOCKER_MANGR_LOCK((DWORD)&dwScramble##region_num);	\
		if (TR_ScrambleEnable_##region_num == TR_SCRAMBLE_ENABLE_##region_num)	\
		{	\
			TR_ScrambleEnable_##region_num ^= 1;	\
			iviTR_Lightweight_DescrambleRegion((void *)TR_RegionBegin_##region_num, (void *)TR_RegionEnd_##region_num, TR_SCRAMBLE_SEED_##region_num, TR_RelocateTable_##region_num, TR_ScrambleEIPTable_##region_num, TR_ScrambleEIPTableSize_##region_num);	\
		}	\
		_TREXE_LOCKER_MANGR_UNLOCK((DWORD)&dwScramble##region_num);	\
	}
#else
	#define iviTR_TREXE_DESCRAMBLE(region_num) \
	{	\
		_TREXE_LOCKER_MANGR_GETINSTANCE()	\
		static DWORD dwScramble##region_num = __LINE__;\
		_TREXE_LOCKER_MANGR_LOCK((DWORD)&dwScramble##region_num);	\
		if (TR_ScrambleEnable_##region_num == TR_SCRAMBLE_ENABLE_##region_num)	\
		{	\
			TR_ScrambleEnable_##region_num ^= 1;	\
			iviTR_DescrambleRegion((void *)TR_RegionBegin_##region_num, (void *)TR_RegionEnd_##region_num, TR_SCRAMBLE_SEED_##region_num, TR_RelocateTable_##region_num);	\
		}	\
		_TREXE_LOCKER_MANGR_UNLOCK((DWORD)&dwScramble##region_num);	\
	}
#endif


	///*!
	// the scramble operation. the macro of iviTR_TREXE_DECLARE or iviTR_TREXE_DECLARE_SCRAMBLEONLY must 
	// be called, first.
	// Return value - none.
	// 
	// -# region_num - [IN] represent the region which you want to apply TRExe.
	//
	// Supported platform: All
	// Performance overhead: 
	// Security level: 
	//*/        
#ifdef iviTR_ENABLE_LIGHTWEIGHT_SCRAMBLE
	#define iviTR_TREXE_SCRAMBLE(region_num) \
	{	\
		_TREXE_LOCKER_MANGR_GETINSTANCE()	\
		static DWORD dwScramble##region_num = __LINE__;\
		_TREXE_LOCKER_MANGR_LOCK((DWORD)&dwScramble##region_num);	\
		if (TR_ScrambleEnable_##region_num == TR_SCRAMBLE_ENABLE_##region_num ^ 1)	\
		{	\
			TR_ScrambleEnable_##region_num ^= 1;	\
			iviTR_Lightweight_ScrambleRegion((void *)TR_RegionBegin_##region_num, (void *)TR_RegionEnd_##region_num, TR_SCRAMBLE_SEED_##region_num, TR_RelocateTable_##region_num, TR_ScrambleEIPTable_##region_num, TR_ScrambleEIPTableSize_##region_num);	\
		}	\
		_TREXE_LOCKER_MANGR_UNLOCK((DWORD)&dwScramble##region_num);	\
	}
#else
	#define iviTR_TREXE_SCRAMBLE(region_num) \
	{	\
		_TREXE_LOCKER_MANGR_GETINSTANCE()	\
		static DWORD dwScramble##region_num = __LINE__;\
		_TREXE_LOCKER_MANGR_LOCK((DWORD)&dwScramble##region_num);	\
		if (TR_ScrambleEnable_##region_num == TR_SCRAMBLE_ENABLE_##region_num ^ 1)	\
		{	\
			TR_ScrambleEnable_##region_num ^= 1;	\
			iviTR_ScrambleRegion((void *)TR_RegionBegin_##region_num, (void *)TR_RegionEnd_##region_num, TR_SCRAMBLE_SEED_##region_num, TR_RelocateTable_##region_num);	\
		}	\
		_TREXE_LOCKER_MANGR_UNLOCK((DWORD)&dwScramble##region_num);	\
	}
#endif

/*
#else
	#define iviTR_TREXE_DECLARE_CHECKSUMONLY(region_num, dword_relocate)
	#define iviTR_TREXE_EXTERN_CHECKSUMONLY(region_num, dword_relocate) 
	#define iviTR_TREXE_DECLARE_SCRAMBLEONLY(region_num, dword_relocate)	
	#define iviTR_TREXE_EXTERN_SCRAMBLEONLY(region_num, dword_relocate) 
	#define iviTR_TREXE_DECLARE(region_num, dword_relocate)	
	#define iviTR_TREXE_EXTERN(region_num, dword_relocate) 
	#define iviTR_TREXE_CHECK_CHECKSUM(region_num, bool_return)
	#define iviTR_TREXE_DESCRAMBLE(region_num)
	#define iviTR_TREXE_SCRAMBLE(region_num)

	#define iviTR_TREXE2_REGION_DECLARE(GROUP, REGION)
	#define iviTR_TREXE2_REGION_DECLARE_BY_ORDER(GROUP, REGION, ORDER)
	#define iviTR_TREXE2_SPECIFY_LIB
	#define iviTR_TREXE2_DECLARE(GROUP, REGION, dword_relocate)
	#define iviTR_TREXE2_EXTERN(GROUP, REGION, dword_relocate)
	#define iviTR_TREXE2_DECLARE_CHECKSUMONLY(GROUP, REGION, dword_relocate)
	#define iviTR_TREXE2_EXTERN_CHECKSUMONLY(GROUP, REGION, dword_relocate)
	#define iviTR_TREXE2_CallDescramble(GROUP, REGION, dword_uniquenum)
	#define iviTR_TREXE2_CallScramble(GROUP, REGION, dword_uniquenum)
	#define iviTR_TREXE2_CallChecksum(GROUP, REGION, dword_uniquenum)
#endif
*/

//
// Make backward compatible
//
#ifndef TR_USE_BY_TREXE
#ifndef CTR
#define CTR iviCTR
#endif

NS_TRLIB_BEGIN

// updates bSkipAddr and dwBase from pwTable
/*
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
*/
void __inline GetNextAddr(BOOL *pbSkipAddr, DWORD *pdwBase, WORD *pwTable)
{
	WORD *pwTbl = pwTable;
	*pbSkipAddr = FALSE;
 	if (*pwTbl==0xfffe)		// check escape code
	{
 		pwTbl++;
 		*pbSkipAddr = TRUE;
	}
	if (*pwTbl==0xffff)		// check end of data code
 		*pdwBase = 0xffffffff;
 	else 
 		*pdwBase += *pwTbl++;	// increment value
}

//
// iviCTR
//
#ifdef __cplusplus

class iviCTR
{
public:
	static DWORD KeyASeed, KeyBSeed; //, TR_RelocateBase;

	// returns offset for relocate UNDO 
	static __forceinline DWORD __fastcall GetUndoOffset()
	{
		return TR_Relocate_Base2==1 ? 0 : TR_Relocate_Base2 - PtrToUlong(TR_RelocateCode);
	}
	
	// relocates code
	static __forceinline BOOL __fastcall RelocateCode(void *start_addr, WORD *pwTable, BOOL bUndo)
	{
		if(pwTable && *pwTable!=0xffff && IsRelocated())
			{
			BOOL bSkipAddr;
			DWORD dwBase, dwUndo, dwChange;

			dwBase = PtrToUlong(start_addr);
			dwUndo = GetUndoOffset();
			dwChange = bUndo ? dwUndo : (DWORD)-(int)dwUndo;
			while(*pwTable!=0xffff)
				{
				GetNextAddr(&bSkipAddr, &dwBase, pwTable);
				if(!bSkipAddr)
					*((DWORD *)UlongToPtr(dwBase)) += dwChange;
				}
			return TRUE;
			}
		return FALSE;
	}
	
	static __forceinline void __fastcall EnableCSS()
	{
		KeyASeed = TR_KEYA_ACTUAL_XOR_SEED;
		KeyBSeed = TR_KEYB_ACTUAL_XOR_SEED;
	}
	
	// returns TRUE if the code has been relocated
	static __forceinline BOOL __fastcall IsRelocated()
	{
	#ifdef _WIN32
		return TR_Relocate_Base2!=1 && TR_Relocate_Base2!=PtrToUlong(TR_RelocateCode);
	#else
		return 1; // NOTE: Linux shared libraries are ALWAYS relocated!
	#endif
	}

	// updates pval checksum in presence of relocation (only if pwTable is non-null)
	static __forceinline void __fastcall Checksum(void *start_addr, void *end_addr, DWORD seed, DWORD *pval, WORD *pwTable)
	{
		DWORD val;

		if (pwTable && *pwTable!=0xffff && IsRelocated())
			{
			BOOL bSkipAddr;
			DWORD *dptr,dwUndo,dwBase,dwOrig,dwBaseBitOffset;

			dwUndo = GetUndoOffset();	// additive undo value
			dwBase = PtrToUlong(start_addr);
			GetNextAddr(&bSkipAddr, &dwBase, pwTable);
			for(val=seed, dptr=(DWORD *)start_addr; PtrToUlong(dptr)<PtrToUlong(end_addr); dptr++)
				{
				// original datum comes from (dptr)
				dwOrig = *dptr;
				//
				// check if reloc address is in the low bits.
				//
				if (PtrToUlong(dptr)>=dwBase)
					{
					dwBaseBitOffset = (PtrToUlong(dptr)-dwBase)<<3;
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
							dwOrig |= (*((DWORD *)UlongToPtr(dwBase))+dwUndo)>>dwBaseBitOffset;
							}
						}
					// get next relocation
					GetNextAddr(&bSkipAddr, &dwBase, pwTable);
					}
				//
				//	check if reloc address is in the high bits of datum
				//  (this CAN occur even after reloc address is in low bits)
				//
				if (PtrToUlong(dptr)>=dwBase-3)
					{
					if (bSkipAddr)
						dwOrig &= (1<<((dwBase-PtrToUlong(dptr))<<3)) - 1;
					else
						dwOrig += dwUndo<<((dwBase-PtrToUlong(dptr))<<3);
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
	
	// descrambles code in presence of relocation (only if pwTable is non-null)
	static __forceinline void __fastcall Descramble(void *start_addr, void *end_addr, DWORD seed, WORD *pwTable)
	{
		DWORD val, *dptr;
		
		if(pwTable && *pwTable!=0xffff && IsRelocated())
			{
			BOOL bSkipAddr;
			WORD *pwOrigTable;
			DWORD dwBase,dwMask,dwBaseBitOffset;

			pwOrigTable = pwTable;
			RelocateCode(start_addr, pwOrigTable, TRUE);
			dwBase = PtrToUlong(start_addr);
			GetNextAddr(&bSkipAddr, &dwBase, pwTable);
			for(val=seed, dptr=(DWORD *)start_addr; PtrToUlong(dptr)<PtrToUlong(end_addr); dptr++)
				{
				dwMask = 0xffffffff;
				//
				// check if reloc address is in the low bits.
				//
				if (PtrToUlong(dptr)>=dwBase)
					{
					dwBaseBitOffset = (PtrToUlong(dptr)-dwBase)<<3;
					if (bSkipAddr)
						{	// skip this address!
						if(dwBaseBitOffset==0)
							dwMask = 0;
						else
							dwMask &= -1<<(32-dwBaseBitOffset);
						}
					// get next relocation
					GetNextAddr(&bSkipAddr, &dwBase, pwTable);
					}
				//
				//	check if reloc address is in the high bits of datum
				//  (this CAN occur even after reloc address is in low bits)
				//
				if (PtrToUlong(dptr)>=dwBase-3)
					{
					if (bSkipAddr)
						dwMask &= (1<<((dwBase-PtrToUlong(dptr))<<3)) - 1;
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
			iviTR_DESCRAMBLE(start_addr,end_addr,seed);
			}
	}
	
	// scrambles code in the presence of relocation
	static __forceinline void __fastcall Scramble(void *start_addr, void *end_addr, DWORD seed, WORD *pwTable)
	{
		DWORD *dptr, val, oval;

		if (pwTable && *pwTable!=0xffff && IsRelocated())
			{
			BOOL bSkipAddr;
			DWORD dwBase,dwMask,dwBaseBitOffset;

			dwBase = PtrToUlong(start_addr);
			GetNextAddr(&bSkipAddr, &dwBase, pwTable);
			for(val=seed, dptr=(DWORD *)start_addr; PtrToUlong(dptr)<PtrToUlong(end_addr); dptr++)
				{
				dwMask = 0xffffffff;
				//
				// check if reloc address is in the low bits.
				//
				if (PtrToUlong(dptr)>=dwBase)
					{
					dwBaseBitOffset = (PtrToUlong(dptr)-dwBase)<<3;
					if (bSkipAddr)
						{	// skip this address!
						if(dwBaseBitOffset==0)
							dwMask = 0;
						else
							dwMask &= -1<<(32-dwBaseBitOffset);
						}
					// get next relocation
					GetNextAddr(&bSkipAddr, &dwBase, pwTable);
					}
				//
				//	check if reloc address is in the high bits of datum
				//  (this CAN occur even after reloc address is in low bits)
				//
				if (PtrToUlong(dptr)>=dwBase-3)
					{
					if (bSkipAddr)
						dwMask &= (1<<((dwBase-PtrToUlong(dptr))<<3)) - 1;
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
			iviTR_SCRAMBLE(start_addr,end_addr,seed);
			}
	}
};
#endif // __cplusplus

NS_TRLIB_END

#endif // TR_USE_BY_TREXE

#define iviTR_WRAPAPI_REGION_BEGIN     void __declspec(naked) TR_WRAPAPI_REGION() { 
#define iviTR_WRAPAPI_REGION_ADDITEM   __asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h
#define iviTR_WRAPAPI_REGION_END       }

#define iviTR_WRAPAPI_REGION_ADD_10_ITEM \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h

#define iviTR_WRAPAPI_REGION_ADD_100_ITEM \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h 

#define iviTR_WRAPAPI_REGION_ADD_500_ITEM \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h \
	__asm _emit 0bbh __asm _emit 025h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h __asm _emit 000h


#endif // _IVISCRAMBLE_H

