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
#include "IVITRComm.h"

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

#define iviTR_USE_CTR_MACRO_VERSION

#if (defined(__cplusplus) && !defined(iviTR_USE_CTR_MACRO_VERSION))
	#define iviTR_ChecksumRegion(start_addr, end_addr, seed, pval, pwTable)\
		iviCTR::Checksum(start_addr, end_addr, seed, pval, pwTable)
	#define iviTR_ScrambleRegion(start_addr, end_addr, seed, pwTable)\
		iviCTR::Scramble(start_addr, end_addr, seed, pwTable)
	#define iviTR_DescrambleRegion(start_addr, end_addr, seed, pwTable)\
		iviCTR::Descramble(start_addr, end_addr, seed, pwTable)
#else
	#define iviTR_ChecksumRegion(start_addr, end_addr, seed, pval, pwTable)\
		DO_CHECKSUM(start_addr, end_addr, seed, pval, pwTable)
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
	(TR_RelocateBase==TR_CHECKSUM_RELOCATION) ? 0 : TR_RelocateBase - PtrToUlong(TR_RelocateCode)

#ifdef _WIN32
#define DO_ISRELOCATED	\
	(TR_RelocateBase!=TR_CHECKSUM_RELOCATION && TR_RelocateBase!=PtrToUlong(TR_RelocateCode))
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
		TR_DESCRAMBLE(start_addr,end_addr,seed);										\
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
		TR_SCRAMBLE(start_addr,end_addr,seed);											\
	}																					\
}
// updates bSkipAddr and dwBase from pwTable
#ifdef __cplusplus
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
#endif

#ifdef TR_TREXE_ENABLE
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
		volatile DWORD	TR_ChecksumEnable_##region_num	= region_num ^ 85;  \
		volatile DWORD	TR_ChecksumValue_##region_num = TR_CHECKSUM_VALUE_##region_num; \
		DWORD	TR_RelocateSize_##region_num =	dword_relocate; \
		WORD	TR_RelocateTable_##region_num[dword_relocate] = {0xffff}; \
		\
		int TR_RegionBegin_##region_num() \
		{ \
			volatile int play = region_num; \
			for (int i=0;i<region_num;i--) play=abs(play); \
			return play; \
		} \
		\
		int TR_RegionEnd_##region_num() \
		{ \
			volatile int play = region_num; \
			for (int i=0;i<region_num;i++) play+=play<<1; \
			return play; \
		}

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
		volatile DWORD	TR_ScrambleEnable_##region_num	= region_num^57; \
		DWORD	TR_RelocateSize_##region_num	= dword_relocate;	\
		WORD	TR_RelocateTable_##region_num[dword_relocate] = {0xffff};	\
		DWORD	TR_ScrambleEIPTableSize_##region_num = dword_relocate * 10;	\
		WORD	TR_ScrambleEIPTable_##region_num[dword_relocate * 10] = {0xffff};	\
		\
		int TR_RegionBegin_##region_num() \
		{	\
			volatile int play = region_num; \
			for (int i=0;i<region_num;i++) play+=play;	\
			return play;	\
		}	\
		\
		int TR_RegionEnd_##region_num()	\
		{	\
			volatile int play = region_num; \
			for (int i=0;i<region_num;i++) play^=play;	\
			return play;	\
		}
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
		volatile DWORD	TR_ScrambleEnable_##region_num	= region_num^57; \
		DWORD	TR_RelocateSize_##region_num	= dword_relocate;	\
		WORD	TR_RelocateTable_##region_num[dword_relocate] = {0xffff};	\
		\
		int TR_RegionBegin_##region_num() \
		{	\
			volatile int play = region_num; \
			for (int i=0;i<region_num;i++) play+=play;	\
			return play;	\
		}	\
		\
		int TR_RegionEnd_##region_num()	\
		{	\
			volatile int play = region_num; \
			for (int i=0;i<region_num;i++) play^=play;	\
			return play;	\
		}

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
		volatile DWORD	TR_ScrambleEnable_##region_num = region_num^99;	\
		volatile DWORD	TR_ChecksumEnable_##region_num = region_num^99;	\
		volatile DWORD	TR_ChecksumValue_##region_num  = TR_CHECKSUM_VALUE_##region_num;	\
		DWORD	TR_RelocateSize_##region_num = dword_relocate;	\
		WORD	TR_RelocateTable_##region_num[dword_relocate] = {0xffff};	\
		DWORD	TR_ScrambleEIPTableSize_##region_num = dword_relocate * 10;	\
		WORD	TR_ScrambleEIPTable_##region_num[dword_relocate * 10] = {0xffff};	\
		\
		int TR_RegionBegin_##region_num()	\
		{ \
			volatile int play = region_num; \
			for( int i=0; i<region_num; i++) \
				play|=play; \
			return play; \
		}	\
		\
		int TR_RegionEnd_##region_num() \
		{ \
			volatile int play = region_num; \
			for( int i=0; i<region_num; i++) \
				play+=play; \
			return play; \
		}	
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
		volatile DWORD	TR_ScrambleEnable_##region_num = region_num^99;	\
		volatile DWORD	TR_ChecksumEnable_##region_num = region_num^99;	\
		volatile DWORD	TR_ChecksumValue_##region_num  = TR_CHECKSUM_VALUE_##region_num;	\
		DWORD	TR_RelocateSize_##region_num = dword_relocate;	\
		WORD	TR_RelocateTable_##region_num[dword_relocate] = {0xffff};	\
		\
		int TR_RegionBegin_##region_num()	\
		{ \
			volatile int play = region_num; \
			for( int i=0; i<region_num; i++) \
				play|=play; \
			return play; \
		}	\
		\
		int TR_RegionEnd_##region_num() \
		{ \
			volatile int play = region_num; \
			for( int i=0; i<region_num; i++) \
				play+=play; \
			return play; \
		}	
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
		if (TR_ScrambleEnable_##region_num == TR_SCRAMBLE_ENABLE_##region_num)	\
		{	\
			TR_ScrambleEnable_##region_num ^= 1;	\
			iviTR_Lightweight_DescrambleRegion((void *)TR_RegionBegin_##region_num, (void *)TR_RegionEnd_##region_num, TR_SCRAMBLE_SEED_##region_num, TR_RelocateTable_##region_num, TR_ScrambleEIPTable_##region_num, TR_ScrambleEIPTableSize_##region_num);	\
		}
#else
	#define iviTR_TREXE_DESCRAMBLE(region_num) \
		if (TR_ScrambleEnable_##region_num == TR_SCRAMBLE_ENABLE_##region_num)	\
		{	\
			TR_ScrambleEnable_##region_num ^= 1;	\
			iviTR_DescrambleRegion((void *)TR_RegionBegin_##region_num, (void *)TR_RegionEnd_##region_num, TR_SCRAMBLE_SEED_##region_num, TR_RelocateTable_##region_num);	\
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
		if (TR_ScrambleEnable_##region_num == TR_SCRAMBLE_ENABLE_##region_num ^ 1)	\
		{	\
			TR_ScrambleEnable_##region_num ^= 1;	\
			iviTR_Lightweight_ScrambleRegion((void *)TR_RegionBegin_##region_num, (void *)TR_RegionEnd_##region_num, TR_SCRAMBLE_SEED_##region_num, TR_RelocateTable_##region_num, TR_ScrambleEIPTable_##region_num, TR_ScrambleEIPTableSize_##region_num);	\
		}
#else
	#define iviTR_TREXE_SCRAMBLE(region_num) \
		if (TR_ScrambleEnable_##region_num == TR_SCRAMBLE_ENABLE_##region_num ^ 1)	\
		{	\
			TR_ScrambleEnable_##region_num ^= 1;	\
			iviTR_ScrambleRegion((void *)TR_RegionBegin_##region_num, (void *)TR_RegionEnd_##region_num, TR_SCRAMBLE_SEED_##region_num, TR_RelocateTable_##region_num);	\
		}
#endif

#else
	#define iviTR_TREXE_DECLARE_CHECKSUMONLY(region_num, dword_relocate)
	#define iviTR_TREXE_EXTERN_CHECKSUMONLY(region_num, dword_relocate) 
	#define iviTR_TREXE_DECLARE_SCRAMBLEONLY(region_num, dword_relocate)	
	#define iviTR_TREXE_EXTERN_SCRAMBLEONLY(region_num, dword_relocate) 
	#define iviTR_TREXE_DECLARE(region_num, dword_relocate)	
	#define iviTR_TREXE_EXTERN(region_num, dword_relocate) 
	#define iviTR_TREXE_CHECK_CHECKSUM(region_num, bool_return) {}
	#define iviTR_TREXE_DESCRAMBLE(region_num) {}
	#define iviTR_TREXE_SCRAMBLE(region_num) {}
#endif

//
// Make backward compatible
//
#ifndef CTR
#define CTR iviCTR
#endif
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
		return TR_RelocateBase==TR_CHECKSUM_RELOCATION ? 0 : TR_RelocateBase - PtrToUlong(TR_RelocateCode);
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
				GetNextAddr(bSkipAddr, dwBase, pwTable);
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
		return TR_RelocateBase!=TR_CHECKSUM_RELOCATION && TR_RelocateBase!=PtrToUlong(TR_RelocateCode);
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
			GetNextAddr(bSkipAddr, dwBase, pwTable);
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
					GetNextAddr(bSkipAddr, dwBase, pwTable);
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
//#ifdef iviTR_ENABLE_LIGHTWEIGHT_SCRAMBLE
//		iviTR_LIGHTWEIGHT_DESCRAMBLE(start_addr, end_addr, seed, pwTable, 0, 0);
//#else
		DWORD val, *dptr;
		
		if(pwTable && *pwTable!=0xffff && IsRelocated())
			{
			BOOL bSkipAddr;
			WORD *pwOrigTable;
			DWORD dwBase,dwMask,dwBaseBitOffset;

			pwOrigTable = pwTable;
			RelocateCode(start_addr, pwOrigTable, TRUE);
			dwBase = PtrToUlong(start_addr);
			GetNextAddr(bSkipAddr, dwBase, pwTable);
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
					GetNextAddr(bSkipAddr, dwBase, pwTable);
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
			TR_DESCRAMBLE(start_addr,end_addr,seed);
			}
//#endif
	}
	
	// scrambles code in the presence of relocation
	static __forceinline void __fastcall Scramble(void *start_addr, void *end_addr, DWORD seed, WORD *pwTable)
	{
#ifdef iviTR_ENABLE_LIGHTWEIGHT_SCRAMBLE
//		iviTR_LIGHTWEIGHT_SCRAMBLE(start_addr, end_addr, seed, pwTable);
#else
		DWORD *dptr, val, oval;

		if (pwTable && *pwTable!=0xffff && IsRelocated())
			{
			BOOL bSkipAddr;
			DWORD dwBase,dwMask,dwBaseBitOffset;

			dwBase = PtrToUlong(start_addr);
			GetNextAddr(bSkipAddr, dwBase, pwTable);
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
					GetNextAddr(bSkipAddr, dwBase, pwTable);
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
			TR_SCRAMBLE(start_addr,end_addr,seed);
			}
#endif
	}
};
#endif

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
