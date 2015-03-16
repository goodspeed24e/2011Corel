
/*!
***************************************************************************
*
* \file fmo.h
*
* \brief
*    Support for Flexilble Macroblock Ordering (FMO)
*
* \date
*    19 June, 2002
*
* \author
*    Stephan Wenger   stewe@cs.tu-berlin.de
**************************************************************************/

#ifdef __cplusplus
//extern "C" {
#endif

#ifndef _FMO_H_
#define _FMO_H_


int FmoInit PARGS2(pic_parameter_set_rbsp_t* pps, seq_parameter_set_rbsp_t* sps);
int FmoFinit PARGS0();

int FmoGetNumberOfSliceGroup PARGS0();
int FmoGetLastMBOfPicture PARGS0();
int FmoGetLastMBInSliceGroup PARGS1(int SliceGroup);
int FmoGetSliceGroupId PARGS1(int mb);
int FmoGetNextMBNr PARGS1(int CurrentMbNr);

#endif

#ifdef __cplusplus
//}
#endif
