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

#ifndef _IVISCRAMBLEOFF_H
#define _IVISCRAMBLEOFF_H

#define iviTR_CHECKSUM(val, start_addr, end_addr, seed) {}
#define iviTR_SCRAMBLE(start_addr, end_addr, seed) {}
#define iviTR_DESCRAMBLE(start_addr, end_addr, seed) {}

// Extended checksum routines.
#define CHECKSUM_INSERT1(val, REG1, REG2) 
#define CHECKSUM_INSERT2(val, REG1, REG2) 
#define CHECKSUM_INSERT3(val, REG1, REG2) 
#define CHECKSUM_INSERT4(val, REG1, REG2) 

#define iviTR_CHECKSUM_EXPRESS1(val) \
	CHECKSUM_INSERT1(val, ebx, esi)

#define iviTR_CHECKSUM1(val, start_addr, end_addr, seed) {}

#define iviTR_CHECKSUM_EXPRESS2(val) \
	CHECKSUM_INSERT2(val, eax, edi)

#define iviTR_CHECKSUM2(val, start_addr, end_addr, seed) {}

#define iviTR_CHECKSUM_EXPRESS3(val) \
	CHECKSUM_INSERT3(val, ebx, esi)

#define iviTR_CHECKSUM3(val, start_addr, end_addr, seed) {}

#define iviTR_CHECKSUM_EXPRESS4(val) \
	CHECKSUM_INSERT4(val, eax, edi)

#define iviTR_CHECKSUM4(val, start_addr, end_addr, seed) {}

#define iviTR_USE_CTR_MACRO_VERSION

#define iviTR_ChecksumRegion(start_addr, end_addr, seed, pval, pwTable) {}
#define iviTR_ChecksumRegion1(start_addr, end_addr, seed, pval, pwTable) {}
#define iviTR_ChecksumRegion2(start_addr, end_addr, seed, pval, pwTable) {}
#define iviTR_ChecksumRegion3(start_addr, end_addr, seed, pval, pwTable) {}
#define iviTR_ChecksumRegion4(start_addr, end_addr, seed, pval, pwTable) {}
#define iviTR_ScrambleRegion(start_addr, end_addr, seed, pwTable) {}
#define iviTR_DescrambleRegion(start_addr, end_addr, seed, pwTable) {}

#define iviTR_Lightweight_ScrambleRegion(start_addr, end_addr, seed, pwTable, pwScrambleEIPTable, scrambleEIPTableSize) {}
#define iviTR_Lightweight_DescrambleRegion(start_addr, end_addr, seed, pwTable, pwScrambleEIPTable, scrambleEIPTableSize) {}

#define DO_GETNEXTADDR(bSkipAddr,dwBase, pwTable) {}

#define DO_GETUNDOOFFSET  

#define DO_ISRELOCATED	1

#define DO_RELOCATECODE(start_addr, pwTable, bUndo) {}

#define DO_CHECKSUM(start_addr, end_addr, seed, pval, pwTable) {}
#define DO_CHECKSUM1(start_addr, end_addr, seed, pval, pwTable) {}
#define DO_CHECKSUM2(start_addr, end_addr, seed, pval, pwTable) {}
#define DO_CHECKSUM3(start_addr, end_addr, seed, pval, pwTable) {}
#define DO_CHECKSUM4(start_addr, end_addr, seed, pval, pwTable) {}

#define DO_DESCRAMBLE(start_addr, end_addr, seed, pwTable) {}
#define DO_SCRAMBLE(start_addr, end_addr, seed, pwTable) {}

#define iviTR_TREXE2_REGION_PFX ""
#define iviTR_TREXE2_REGION_DECLARE(GROUP, REGION) 
#define iviTR_TREXE2_REGION_DECLARE_BY_ORDER(GROUP, REGION, ORDER) 

#define iviTR_TREXE2_SPECIFY_LIB
#define iviTR_TREXE2_RUN_REGION_PFX ""
#define iviTR_TREXE2_DECLARE(GROUP, REGION, dword_relocate) 

#define iviTR_TREXE2_EXTERN(GROUP, REGION, dword_relocate) 
#define iviTR_TREXE2_DECLARE_CHECKSUMONLY(GROUP, REGION, dword_relocate) 
#define iviTR_TREXE2_EXTERN_CHECKSUMONLY(GROUP, REGION, dword_relocate)	

#define iviTR_TREXE2_CallDescramble(GROUP, REGION, dword_uniquenum) {}
#define iviTR_TREXE2_CallScramble(GROUP, REGION, dword_uniquenum) {}
#define iviTR_TREXE2_CallChecksum(GROUP, REGION, dword_uniquenum, bool_return) {}

#define iviTR_TREXE2_CallChecksum1(GROUP, REGION, dword_uniquenum, bool_return)	{}
#define iviTR_TREXE2_CallChecksum2(GROUP, REGION, dword_uniquenum, bool_return)	{}
#define iviTR_TREXE2_CallChecksum3(GROUP, REGION, dword_uniquenum, bool_return)	{}
#define iviTR_TREXE2_CallChecksum4(GROUP, REGION, dword_uniquenum, bool_return)	{}

#define iviTR_TREXE_REGION_BOUNDARY(region_num)	

#define iviTR_TREXE_DECLARE_CHECKSUMONLY(region_num, dword_relocate) 
#define iviTR_TREXE_EXTERN_CHECKSUMONLY(region_num, dword_relocate) 

#define iviTR_TREXE_DECLARE_SCRAMBLEONLY(region_num, dword_relocate) 
#define iviTR_TREXE_EXTERN_SCRAMBLEONLY(region_num, dword_relocate) 

#define iviTR_TREXE_DECLARE(region_num, dword_relocate)	
#define iviTR_TREXE_EXTERN(region_num, dword_relocate) 

#define iviTR_TREXE_CHECK_CHECKSUM(region_num, bool_return) 
#define iviTR_TREXE_CHECK_CHECKSUM1(region_num, bool_return) 
#define iviTR_TREXE_CHECK_CHECKSUM2(region_num, bool_return) 
#define iviTR_TREXE_CHECK_CHECKSUM3(region_num, bool_return) 
#define iviTR_TREXE_CHECK_CHECKSUM4(region_num, bool_return) 

#define iviTR_TREXE_DESCRAMBLE(region_num) 
#define iviTR_TREXE_SCRAMBLE(region_num) 

#define iviTR_WRAPAPI_REGION_BEGIN     
#define iviTR_WRAPAPI_REGION_ADDITEM   
#define iviTR_WRAPAPI_REGION_END  

#define iviTR_WRAPAPI_REGION_ADD_10_ITEM   
#define iviTR_WRAPAPI_REGION_ADD_500_ITEM   

#endif // _IVISCRAMBLE_H

