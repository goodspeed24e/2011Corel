
/*!
*************************************************************************************
* \file annexb.h
*
* \brief
*    Annex B byte stream buffer handling.
*
*************************************************************************************
*/

#ifndef _ANNEXB_H_
#define _ANNEXB_H_

#include "nalucommon.h"

CREL_RETURN  GetAnnexbNALU PARGS2(NALU_t *one_nalu, int* output_pos);

#if defined(_HW_ACCEL_)
CREL_RETURN  DXVA_GetAnnexbNALU PARGS2(NALU_t *one_nalu, int* output_pos);
#endif

void OpenBitstreamFile (char *fn);
void CloseBitstreamFile();
void CheckZeroByteNonVCL PARGS1(NALU_t *one_nalu);
void CheckZeroByteVCL PARGS1(NALU_t *one_nalu);

#endif

