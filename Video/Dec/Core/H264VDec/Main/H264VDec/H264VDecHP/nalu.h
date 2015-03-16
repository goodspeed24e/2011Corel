
/*!
**************************************************************************************
* \file
*    parset.h
* \brief
*    Picture and Sequence Parameter Sets, encoder operations
*    This code reflects JVT version xxx
*  \date 25 November 2002
* \author
*    Main contributors (see contributors.h for copyright, address and affiliation details) 
*      - Stephan Wenger        <stewe@cs.tu-berlin.de>
***************************************************************************************
*/


#ifndef _NALU_H_
#define _NALU_H_

#include <stdio.h>
#include "nalucommon.h"

//int GetAnnexbNALU PARGS1(NALU_t *one_nalu);
int NALUtoRBSP (NALU_t *one_nalu);

#endif
