
/*!
*************************************************************************************
* \file header.h
* 
* \brief
*    Prototypes for header.c
*************************************************************************************
*/

#ifndef _HEADER_H_
#define _HEADER_H_

void dec_ref_pic_marking PARGS0();

CREL_RETURN decode_poc PARGS0();
#if 0
int dumppoc();
#endif

#if !defined (_COLLECT_PIC_)
int FirstPartOfSliceHeader PARGS0();
int RestOfSliceHeader PARGS0();
#else
CREL_RETURN	ParseSliceHeader PARGS0();
CREL_RETURN	UseSliceParameter PARGS0();
CREL_RETURN CopyNaluExtToSlice PARGS1 (Slice* currSlice);
//void	RearrangeCCCode (bool isBSlice);
#endif

#endif

