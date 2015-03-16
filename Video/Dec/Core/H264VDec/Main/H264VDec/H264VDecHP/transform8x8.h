/*!
***************************************************************************
*
* \file transform8x8.h
*
* \brief
*    prototypes of 8x8 transform functions
*
* \date
*    9. October 2003
*
* \author
*    Main contributors (see contributors.h for copyright, address and affiliation details) 
*    - Yuri Vatis  vatis@hhi.de
**************************************************************************/

#ifndef _TRANSFORM8X8_H_
#define _TRANSFORM8X8_H_

#include "global.h"
#include "image.h"
#include "mb_access.h"

//for SSE optimization
typedef void inverse_transform8x8_t PARGS4(unsigned char *dest, unsigned char *pred, short *src, int stride);

extern inverse_transform8x8_t inverse_transform8x8_c;
extern inverse_transform8x8_t inverse_transform8x8_sse2; // for SSE2 optimization
extern inverse_transform8x8_t inverse_transform8x8_sse;
extern inverse_transform8x8_t *inverse_transform8x8;
CREL_RETURN intrapred8x8 PARGS1(int b8);


#endif
