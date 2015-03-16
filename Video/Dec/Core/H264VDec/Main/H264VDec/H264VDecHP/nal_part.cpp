
/*!
************************************************************************
* \file  nal_part.c
*
* \brief
*    Network Adaptation layer for partition file
*
* \author
*    Main contributors (see contributors.h for copyright, address and affiliation details)
*    - Tobias Oelbaum <oelbaum@hhi.de, oelbaum@drehvial.de>
************************************************************************
*/

#include <string.h>

#include "global.h"
#include "elements.h"

/*!
************************************************************************
* \brief
*    Resets the entries in the DecodingEnvironment struct
************************************************************************

void free_Partition(DecodingEnvironment *dep)
{
dep->Dstrmlength = 0;
dep->Dbits_to_go = 0;
dep->Dcodestrm   = dep->Dbasestrm;
dep->Dei_flag    = 0;
dep->Dbuffer     = 0;
memset (dep->Dbasestrm, 0x00, MAX_CODED_FRAME_SIZE);
}
*/