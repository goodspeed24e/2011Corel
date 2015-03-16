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
//  Purpose: G.728 postfilter function
//
*/

#include "owng728.h"

void Sum_of_absolute_value_calc(Ipp16s *sst, Ipp16s *temp, Ipp32s *sumunfil, Ipp32s *sumfil){
   Ipp32s aa0, aa1, k;

   aa0 = 0;
   aa1 = 0;

   for(k=0; k<IDIM; k++) {
      aa0 = aa0 + Abs_32s(sst[k]);
      aa1 = aa1 + Abs_32s(temp[k]);
   }
   *sumunfil = aa0;
   *sumfil = aa1;

   return;
}

void Scaling_factor_calc(Ipp32s sumunfil, Ipp32s sumfil, Ipp16s *scale, Ipp16s *nlsscale)
{
   Ipp32s aa0, aa1;
   Ipp16s den, num;
   Ipp16s nlsden, nlsnum;

   if(sumfil > 4) {
      Vscale_32s(&sumfil, 1, 1, 30, &aa1, &nlsden);
      den = Cnvrt_NR_32s16s(aa1);

      Vscale_32s(&sumunfil, 1, 1, 30, &aa0, &nlsnum);
      num = Cnvrt_NR_32s16s(aa0);

      Divide(num, nlsnum, den, nlsden, scale, nlsscale);
   } else {
      *scale = 16384;
      *nlsscale = 14;
   }

   return;
}

void First_order_lowpass_filter_ouput_gain_scaling(Ipp16s scale, Ipp16s nlsscale, 
         Ipp16s *scalefil, Ipp16s *temp, Ipp16s *spf) {
   
   Ipp32s aa0, aa1, k;
   Ipp16s nrs;

   aa1 = AGCFAC1 * scale;
   nrs = nlsscale - 7; /* nrs = nlsscale - 14 + (21 - 14)*/ 
   aa1 = ShiftR_32s(aa1, nrs);

   for(k=0;k < IDIM; k++) {
      aa0 = aa1 + AGCFAC*(*scalefil);
      aa0 = aa0 << 2;
      *scalefil = Cnvrt_NR_32s16s(aa0);

      aa0 = (*scalefil) * temp[k];
      aa0 = aa0 << 2;
      spf[k] = Cnvrt_NR_32s16s(aa0);
   }
   return;
}

void LPC_inverse_filter(Ipp16s *ip, const Ipp16s* sst, Ipp16s* stlpci, 
                        const Ipp16s* apf, Ipp16s* d) {
   Ipp16s k, j, itmp;
   Ipp32s aa0;

   if(*ip==NPWSZ) *ip = NPWSZ - NFRSZ;

   for(k=0; k<IDIM; k++) {
      aa0 = sst[k];
      aa0 = ShiftL_32s(aa0, 13);
      for(j=9; j>0; j--) {
         aa0 = aa0 + stlpci[j]*apf[j+1];
         stlpci[j] = stlpci[j-1];
      }
      aa0 = aa0 + stlpci[0]*apf[1];
      stlpci[0] = sst[k];
      itmp = (*ip) + k;
      aa0 = aa0 << 2;
      d[itmp] = Cnvrt_NR_32s16s(aa0);
   }
   *ip = (*ip) + IDIM;
   return;
}

void Short_term_posfilter_coef_calc(const Ipp16s *spfpcfv, const Ipp16s *spfzcfv, 
                           Ipp16s *apf, Ipp16s nlsapf, 
                           Ipp16s *pstA, Ipp16s rc1, Ipp16s *tiltz)
{
   Ipp32s aa0, i, j;
   Ipp16s ws[3];
   Ipp16s *az = pstA;
   Ipp16s *ap = pstA+10;
   Ipp32s ovfPos = IPP_MAX_32S >> (16 - nlsapf); /* nlsapf (1...16)*/ 
   Ipp32s ovfNeg = IPP_MIN_32S >> (16 - nlsapf);

   for(i=1; i<3; i++) {
      aa0 = spfpcfv[i] * apf[i];
      if(aa0 > ovfPos || aa0 < ovfNeg){  /* if  Overflow in ShiftL_32s*/ 
         if(nlsapf==14) {
            for(j=1; j<11; j++) {
               aa0 = apf[j] << 15;
               apf[j] = Cnvrt_NR_32s16s(aa0);
            }
         }
         if(nlsapf==15) {
            for(j=1; j<11; j++) {
               aa0 = apf[j] << 14;
               apf[j] = Cnvrt_NR_32s16s(aa0);
            }
         }
         return;
      }
      aa0 = ShiftL_32s(aa0, (Ipp16s)(16 - nlsapf));
      ws[i] = Cnvrt_NR_32s16s(aa0);
   }
   for(i=1; i<3; i++)
      ap[i-1] = ws[i];

   for(i=3; i<11; i++) {
      aa0 = spfpcfv[i] * apf[i];
      aa0 = ShiftL_32s(aa0, (Ipp16s)(16 - nlsapf));
      ap[i-1] = Cnvrt_NR_32s16s(aa0);
   }

   for(i=1;i<11;i++) {
      aa0 = spfzcfv[i] * apf[i];
      aa0 = ShiftL_32s(aa0, (Ipp16s)(16 - nlsapf));
      az[i-1] = Cnvrt_NR_32s16s(aa0);
   }

   aa0 = TILTF * rc1;
   *tiltz = Cnvrt_NR_32s16s(aa0);

   if(nlsapf==14) {
      for(j=1; j<11; j++) {
         aa0 = apf[j] << 15;
         apf[j] = Cnvrt_NR_32s16s(aa0);
      }
   }
   if(nlsapf==15) {
      for(j=1; j<11; j++) {
         aa0 = apf[j] << 14;
         apf[j] = Cnvrt_NR_32s16s(aa0);
      }
   }

   return;
}

