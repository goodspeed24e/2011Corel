/*************************************************************************/
/*                                                                       */
/*                            LD-CELP  G.728                             */
/*                                                                       */
/*    Low-Delay Code Excitation Linear Prediction speech compression.    */
/*                                                                       */
/*                 Copyright: Analog Devices, Inc., 1993                 */
/*                                                                       */
/*                         Author: Alex Zatsman.                         */
/*                                                                       */
/*  This program was written mostly for testing  Analog Devices' g21k C  */
/*  compiler for the  ADSP21000 architecture  family. While the program  */
/*  works  on  Sparc and ADSP21020, it  has  NOT  been  tested with the  */
/*  official test data from CCITT/ITU.                                   */
/*                                                                       */
/*  The program  is   distributed as  is,  WITHOUT ANY WARRANTY, EITHER  */
/*  EXPLICIT OR IMPLIED.                                                 */
/*                                                                       */
/*************************************************************************/

#include "common.h"
/*
  This data is used by both encoder and decoder. We have to define it 
  once for cases when encoder and decoder are linked together (like ezplay).
  */

#include "common.h"

ATOMIC(sf_coeff,     LPC+1,   COEFF_MEM);
ATOMIC(gp_coeff,     LPCLG+1, COEFF_MEM);
ATOMIC(pwf_z_coeff,  LPCW+1,  COEFF_MEM);
ATOMIC(pwf_p_coeff,  LPCW+1,  COEFF_MEM);
ATOMIC(shape_energy, NCWD, dm);
ATOMIC(imp_resp,     IDIM, dm);


real synspeech [QSIZE];		/* Synthesized Speech */
real qspeech [QSIZE];		/* Quantized  Speech */
real log_gains[QSIZE/IDIM];	/* Logarithm of Gains */

int VOLATILE ffase = -4;
