
/*!
************************************************************************
* \file block.h
*
* \brief
*    definitions for block decoding functions
*
* \author
*  Inge Lille-Langoy               <inge.lille-langoy@telenor.com>    \n
*  Telenor Satellite Services                                         \n
*  P.O.Box 6914 St.Olavs plass                                        \n
*  N-0130 Oslo, Norway
*
************************************************************************
*/

#ifndef _BLOCK_H_
#define _BLOCK_H_

#include "global.h"

#define DQ_BITS         6
#define DQ_ROUND        (1<<(DQ_BITS-1))

extern byte QP_SCALE_CR[52] ;
extern int  dequant_coef[6][16];

typedef void DIAG_DOWN_RIGHT_PRED_PX PARGS2(unsigned char *dest, byte *Pel);
extern DIAG_DOWN_RIGHT_PRED_PX *DIAG_DOWN_RIGHT_PRED_PDR;
extern DIAG_DOWN_RIGHT_PRED_PX DIAG_DOWN_RIGHT_PRED_sse2;
extern DIAG_DOWN_RIGHT_PRED_PX DIAG_DOWN_RIGHT_PRED_sse;
extern DIAG_DOWN_RIGHT_PRED_PX DIAG_DOWN_RIGHT_PRED_c;

typedef void DIAG_DOWN_LEFT_PRED_PX PARGS2(unsigned char *dest, byte *Pel);
extern DIAG_DOWN_LEFT_PRED_PX *DIAG_DOWN_LEFT_PRED_PDL;
extern DIAG_DOWN_LEFT_PRED_PX DIAG_DOWN_LEFT_PRED_sse2;
extern DIAG_DOWN_LEFT_PRED_PX DIAG_DOWN_LEFT_PRED_sse;
extern DIAG_DOWN_LEFT_PRED_PX DIAG_DOWN_LEFT_PRED_c;

typedef void VERT_RIGHT_PRED_PX PARGS2(unsigned char *dest, byte *Pel);
extern VERT_RIGHT_PRED_PX *VERT_RIGHT_PRED_PVR;
extern VERT_RIGHT_PRED_PX VERT_RIGHT_PRED_sse2;
extern VERT_RIGHT_PRED_PX VERT_RIGHT_PRED_sse;
extern VERT_RIGHT_PRED_PX VERT_RIGHT_PRED_c;

typedef void VERT_LEFT_PRED_PX PARGS2(unsigned char *dest, byte *Pel);
extern VERT_LEFT_PRED_PX *VERT_LEFT_PRED_PVL;
extern VERT_LEFT_PRED_PX VERT_LEFT_PRED_sse2;
extern VERT_LEFT_PRED_PX VERT_LEFT_PRED_sse;
extern VERT_LEFT_PRED_PX VERT_LEFT_PRED_c;

typedef void HOR_UP_PRED_PX PARGS1(unsigned char *dest);
extern HOR_UP_PRED_PX *HOR_UP_PRED_PHU;
extern HOR_UP_PRED_PX HOR_UP_PRED_PHU_sse2;
extern HOR_UP_PRED_PX HOR_UP_PRED_PHU_sse;
extern HOR_UP_PRED_PX HOR_UP_PRED_PHU_c;

typedef void HOR_DOWN_PRED_PX PARGS2(unsigned char *dest, byte *Pel);
extern HOR_DOWN_PRED_PX *HOR_DOWN_PRED_PHD;
extern HOR_DOWN_PRED_PX HOR_DOWN_PRED_PHD_sse2;
extern HOR_DOWN_PRED_PX HOR_DOWN_PRED_PHD_sse;
extern HOR_DOWN_PRED_PX HOR_DOWN_PRED_PHD_c;

#endif

