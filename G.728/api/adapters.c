/* /////////////////////////////////////////////////////////////////////////////
//
//                  INTEL CORPORATION PROPRIETARY INFORMATION
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Intel Corporation and may not be copied
//     or disclosed except in accordance with the terms of that agreement.
//          Copyright(c) 2002-2003 Intel Corporation. All Rights Reserved.
//
//     Intel® Integrated Performance Primitives G.728 Sample
//
//  By downloading and installing this sample, you hereby agree that the
//  accompanying Materials are being provided to you under the terms and
//  conditions of the End User License Agreement for the Intel® Integrated
//  Performance Primitives product previously accepted by you. Please refer
//  to the file ipplic.htm located in the root directory of your Intel® IPP
//  product installation for more information.
//
//  G.728 is an international standard promoted by ITU-T and other
//  organizations. Implementations of these standards, or the standard enabled
//  platforms may require licenses from various entities, including
//  Intel Corporation.
//
//
//  Purpose: G.728 function for adapters
//
*/

#include "owng728.h"
#include "g728tables.h"

void Weighting_filter_coef_calc(const Ipp16s *wzcfv, const Ipp16s *wpcfv, Ipp16s *awztmp,
                                Ipp16s nlsawztmp, Ipp16s *wgtA) {
   Ipp32s aa0, i;
   Ipp16s ws[10];
   Ipp16s *awz = wgtA;
   Ipp16s *awp = wgtA+LPCW;
   Ipp16s scale = 16 - nlsawztmp;
   Ipp32s ovfPos = IPP_MAX_32S >> scale;
   Ipp32s ovfNeg = IPP_MIN_32S >> scale;

   /* Do the numerator coefficients */ 
   for(i=0; i<6; i++){
      aa0 = wzcfv[i+1] * awztmp[i];
      /* if Overflow expected in ShiftL_32s Do not update */ 
      if(aa0 > ovfPos || aa0 < ovfNeg) return; 
      aa0 <<= scale;
      ws[i] = Cnvrt_NR_32s16s(aa0);
   }

   for(i=6; i<LPCW; i++){
      aa0 = wzcfv[i+1] * awztmp[i];
      aa0 <<= scale;
      ws[i] = Cnvrt_NR_32s16s(aa0);
   }

   for(i=0; i<LPCW; i++)
      awz[i] = ws[i];

   /* Do the denumerator coefficients */ 
   for(i=0; i<LPCW; i++){
      aa0 = wpcfv[i+1] * awztmp[i];
      aa0 <<= scale;
      awp[i] = Cnvrt_NR_32s16s(aa0);
   }

   return;
}

void Bandwidth_expansion_block45(Ipp16s* gptmp, Ipp16s nlsgptmp, Ipp16s* gp){
   Ipp32s i, aa0;
   Ipp16s scale = 16 - nlsgptmp;/* nlsgptmp = 13, 14 or 15 only */ 
   Ipp32s ovfPos = IPP_MAX_32S >> scale;
   Ipp32s ovfNeg = IPP_MIN_32S >> scale;

   for(i=1; i<=LPCLG; i++) {
      aa0 = facgpv[i] * gptmp[i];
      /* if Overflow expected in ShiftL_32s Do not update */ 
      if(aa0 > ovfPos || aa0 < ovfNeg) return; 
      aa0 <<= scale;
      gptmp[i] = Cnvrt_NR_32s16s(aa0);
   }

   for(i=1; i<=LPCLG; i++)
      gp[i] = gptmp[i];

   return;
}

void Log_gain_linear_prediction(Ipp16s *gp, Ipp16s *gstate, Ipp32s *loggain){
   Ipp32s aa0, i;

   /* predict log gain */ 
   aa0 = 0;
   for(i=LPCLG-1; i>0; i--) {
      aa0 = Sub_32s(aa0, gp[i+1] * gstate[i]);
      gstate[i] = gstate[i-1];
   }
   aa0 = Sub_32s(aa0, gp[1] * gstate[0]);
   *loggain = ShiftR_32s(aa0, 14);

   return;
}

#define c_c4     323

void Inverse_logarithm_calc(Ipp32s loggain, Ipp16s *gain, Ipp16s *nlsgain){
   Ipp32s aa0, aa1;
   Ipp32s x, z;
   Ipp16s tmp;

   /* add offset */ 
   z = loggain + GOFF;

   /* Compute gain = 10**(z/20) */ 
   aa0 = 10 * z;
   aa1 = 2*20649 * z;
   aa1 = Cnvrt_NR_32s16s(aa1);
   aa0 += aa1;
   aa1 = ShiftR_32s(aa0, 15);
   *nlsgain = 14 - aa1;
   
   aa1 = ShiftL_32s(aa1, 15);
   x = Sub_32s(aa0, aa1);

   aa0 = c_c4 * x;
   aa0 = ShiftL_32s(aa0, 1);
   aa0 = Add_32s(aa0, 1874<<16); /* + c_c3 */ 
   tmp = Cnvrt_NR_32s16s(aa0);
   
   aa0 = tmp * x;
   aa0 = ShiftL_32s(aa0, 1);
   aa0 = Add_32s(aa0, 7866<<16); /* + c_c2 */ 
   tmp = Cnvrt_NR_32s16s(aa0);

   aa0 = tmp * x;
   aa0 = ShiftL_32s(aa0, 1);
   aa0 = Add_32s(aa0, 22702<<16); /* + c_c1 */ 
   tmp = Cnvrt_NR_32s16s(aa0);

   aa0 = tmp * x;
   aa0 = Add_32s(aa0, 16384<<16); /* + c_c0 */ 
   *gain = Cnvrt_NR_32s16s(aa0);

   return;
}

Ipp16s Log_gain_adder_and_limiter(Ipp32s loggain, Ipp16s ig, Ipp16s is,
                                  const Ipp16s *gcblg, const Ipp16s *shapelg) {
   Ipp32s acc;

   acc = loggain << 7;
   acc += (gcblg[ig] << 5);
   acc += (shapelg[is] << 5);
   acc = ShiftR_32s(acc, 7);
   if(acc < -16384) acc = -16384;

   return (Ipp16s) acc;
}

void Bandwidth_expansion_block51(Ipp16s* atmp, Ipp16s nlsatmp, Ipp16s* a) {
   Ipp32s i, aa0;
   Ipp16s scale = 16 - nlsatmp; /* nlsatmp = 13,14 or 15 only */ 
   Ipp32s ovfPos = IPP_MAX_32S >> scale;
   Ipp32s ovfNeg = IPP_MIN_32S >> scale;

   for(i=1; i<=LPC; i++) {
      aa0 = facv[i] * atmp[i-1];
      /* if Overflow expected in ShiftL_32s Do not update */ 
      if(aa0 > ovfPos || aa0 < ovfNeg) return; 
      aa0 <<= scale;
      atmp[i-1] = Cnvrt_NR_32s16s(aa0);
   }

   for(i=1; i<=LPC; i++)
      a[i-1] = atmp[i-1];

   return;
}


