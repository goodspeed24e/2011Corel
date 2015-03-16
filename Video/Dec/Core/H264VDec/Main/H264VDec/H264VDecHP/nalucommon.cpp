
/*!
************************************************************************
* \file  nalucommon.c
*
* \brief
*    Common NALU support functions
*
* \author
*    Main contributors (see contributors.h for copyright, address and affiliation details)
*    - Stephan Wenger   <stewe@cs.tu-berlin.de>
************************************************************************
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <malloc.h>

#include "global.h"
#include "nalu.h"
#include "memalloc.h"


/*! 
*************************************************************************************
* \brief
*    Allocates memory for a NALU
*
* \param buffersize
*     size of NALU buffer 
*
* \return
*    pointer to a NALU
*************************************************************************************
*/


NALU_t *AllocNALU(int buffersize)
{
	NALU_t *n;

	if ((n = (NALU_t*) _aligned_malloc (sizeof (NALU_t), 16)) == NULL) no_mem_exit ("AllocNALU: n");

	n->max_size=buffersize;

	if ((n->buf = (byte*) _aligned_malloc (buffersize*sizeof (byte), 16)) == NULL) no_mem_exit ("AllocNALU: n->buf");

	n->pos = 0;
	n->zero_count = 0;

	return n;
}


/*! 
*************************************************************************************
* \brief
*    Frees a NALU
*
* \param n 
*    NALU to be freed
*
*************************************************************************************
*/

void FreeNALU(NALU_t *n)
{
	if (n)
	{
		if (n->buf)
		{
			_aligned_free(n->buf);
			n->buf=NULL;
		}
		_aligned_free (n);
	}
}

void CopyNALU(NALU_t *dst, NALU_t *src)
{
	memcpy(dst->buf, src->buf, src->len);

	dst->startcodeprefix_len = src->startcodeprefix_len;
	dst->len = src->len;
	dst->max_size = src->max_size;
	dst->nal_unit_type = src->nal_unit_type;
	dst->nal_reference_idc = src->nal_reference_idc;
	dst->forbidden_bit = src->forbidden_bit;
	dst->pts = src->pts;
	dst->AU_HasSPS = src->AU_HasSPS;
	dst->pos = src->pos;
	dst->zero_count = src->zero_count;
}
