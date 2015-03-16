
/*!
************************************************************************
* \file  nalu.c
*
* \brief
*    Decoder NALU support functions
*
* \author
*    Main contributors (see contributors.h for copyright, address and affiliation details)
*    - Stephan Wenger   <stewe@cs.tu-berlin.de>
************************************************************************
*/

#include <assert.h>

#include "global.h"
#include "nalu.h"




/*! 
*************************************************************************************
* \brief
*    Converts a NALU to an RBSP
*
* \param 
*    one_nalu: nalu structure to be filled
*
* \return
*    length of the RBSP in bytes
*************************************************************************************
*/

int NALUtoRBSP (NALU_t *one_nalu)
{
	assert (one_nalu != NULL);

	one_nalu->len = EBSPtoRBSP (one_nalu->buf, one_nalu->len, 1) ; 

	return one_nalu->len ;
}

