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
//  Purpose: G.728 pitch search function
//
*/

#include "owng728.h"

#define ITAPTH   26214
#define idx(NAME) NAME - 1 
#define idx_n(NAME) NAME 

void Pitch_period_extraction(Ipp16s *d, const Ipp16s *bl, const Ipp16s *al, 
         Ipp16s *lpffir, Ipp16s *lpfiir, Ipp16s *dec, Ipp32s *kp1) {

   Ipp32s k, j, n;
   Ipp32s aa0, aa1;
   Ipp32s m1, m2, kp, kmax, kptmp, cormax, cmax;
   Ipp16s nls, sum, tmp;

   for(k=NPWSZ-NFRSZ+1; k<=NPWSZ; k++) {
      aa0 = d[idx(k)] * bl[0];
      aa0 = aa0 + lpffir[0]*bl[1];
      aa0 = aa0 + lpffir[1]*bl[2];
      aa0 = aa0 + lpffir[2]*bl[3];
      aa0 = ShiftR_32s(aa0, 6);
      lpffir[2] = lpffir[1];
      lpffir[1] = lpffir[0];
      lpffir[0] = d[idx(k)];
      aa0 = aa0 - lpfiir[0]*al[0];
      aa0 = aa0 - lpfiir[1]*al[1];
      aa0 = aa0 - lpfiir[2]*al[2];
      aa0 = ShiftL_32s(aa0, 3);
      lpfiir[2] = lpfiir[1];
      lpfiir[1] = lpfiir[0];
      lpfiir[0] = Cnvrt_NR_32s16s(aa0);
      n = k >> 2;
      if(k == (n << 2)) dec[n-1] = lpfiir[0];
   }

   m1 = KPMIN / 4;
   m2 = KPMAX / 4;

   aa1 = IPP_MIN_32S;

   for(j=m1; j<=m2; j++) {
      aa0 = 0;
      for(n=1; n<=NPWSZ/4; n++)
         aa0 = aa0 + dec[idx(n)]*dec[idx(n-j)];
      if(aa0 > aa1) {
         aa1 = aa0;
         kmax = j;
      }
   }
   for(n=-m2+1; n<=(NPWSZ-NFRSZ)/4; n++)
      dec[idx(n)] = dec[idx(n+IDIM)];

   m1 = (kmax << 2) - 3;
   m2 = (kmax << 2) + 3;

   if(m1 < KPMIN) m1 = KPMIN;
   if(m2 > KPMAX) m2 = KPMAX;
   aa1 = IPP_MIN_32S;

   for(j=m1; j<=m2; j++) {
      aa0 = 0;
      for(k=1; k<=NPWSZ; k++)
         aa0 = aa0 + d[idx(k)]*d[idx(k-j)];
      if(aa0 > aa1) {
         aa1 = aa0;
         kp = j;
      }
   }

   cormax = aa1;
   m1 = (*kp1) - KPDELTA; /*kp1 from the previous frame*/ 
   m2 = (*kp1) + KPDELTA;

   if(kp >= (m2+1)) {
      if(m1 < KPMIN) m1 = KPMIN;
      if(m2 > KPMAX) m2 = KPMAX;
      aa1 = IPP_MIN_32S;

      for(j=m1; j<=m2; j++) {
         aa0 = 0;
         for(k=1; k<=NPWSZ; k++)
            aa0 = aa0 + d[idx(k)]*d[idx(k-j)];
         if(aa0 > aa1) {
            aa1 = aa0;
            kptmp = j;
         } 
      }
      cmax = aa1;

      aa0 = 0;
      aa1 = 0;
      for(k=1;k <= NPWSZ; k++) {
         aa0 = aa0 + d[idx(k-kp)]*d[idx(k-kp)];
         aa1 = aa1 + d[idx(k-kptmp)]*d[idx(k-kptmp)];
      }

      if(aa0 == 0) cormax = 0;
      if(aa1 == 0) cmax = 0;

      if(cormax > aa0) cormax = aa0;
      if(cormax < 0) cormax = 0;
      if(cmax > aa1) cmax = aa1;
      if(cmax < 0) cmax = 0;

      if(aa0 > aa1) {
         Vscale_32s(&aa0, 1, 1, 30, &aa0, &nls);
         aa1 = ShiftL_32s(aa1, nls);
      } else {
         Vscale_32s(&aa1, 1, 1, 30, &aa1, &nls);
         aa0 = ShiftL_32s(aa0, nls);
      }

      sum = ExtractHigh(aa0);
      tmp = ExtractHigh(aa1);

      aa0 = ShiftL_32s(cormax, nls);
      cormax = ExtractHigh(aa0);

      aa0 = ShiftL_32s(cmax, nls);
      cmax = ExtractHigh(aa0);

      aa1 = cormax * tmp;
      aa1 = ExtractHigh(aa1);
      aa1 = aa1 * ITAPTH;

      aa0 = cmax * sum;
      if(aa0 > aa1) kp = kptmp;
   }

   *kp1 = kp;

   for(k=-KPMAX+1; k<=(NPWSZ - NFRSZ); k++)
      d[idx(k)] = d[idx(k+NFRSZ)];
   
   return;
}

void LTP_coeffs_calc(const Ipp16s *sst, Ipp32s kp, Ipp16s *gl, Ipp16s *glb, Ipp16s *pTab){
   Ipp32s aa0, aa1;
   Ipp16s den, num;
   Ipp16s nlsden, nlsnum, nlsptab, nrs;
   Ipp16s b, nls;

   /* Pitch_predictor_tab_calc*/ 
   ippsDotProd_16s32s_Sfs(sst-NPWSZ-kp,sst-NPWSZ-kp,NPWSZ,&aa0,0);
   ippsDotProd_16s32s_Sfs(sst-NPWSZ,sst-NPWSZ-kp,NPWSZ,&aa1,0);
   if(aa0 == 0) {
      *pTab = 0;
   } else if(aa1 <= 0) {
      *pTab = 0;
   } else {
      if(aa1 >= aa0) *pTab = 16384;
      else {
         Vscale_32s(&aa0, 1, 1, 30, &aa0, &nlsden);
         Vscale_32s(&aa1, 1, 1, 30, &aa1, &nlsnum);
         num = Cnvrt_NR_32s16s(aa1);
         den = Cnvrt_NR_32s16s(aa0);
         Divide(num, nlsnum, den, nlsden, pTab, &nlsptab);
         nrs = nlsptab - 14;
         *pTab = ShiftR_16s(*pTab, nrs);
      }
   }
   /* Update long-term postfilter coefficients*/ 
   if(*pTab < PPFTH) aa0 = 0;
   else              aa0 = PPFZCF * (*pTab);
   b = ShiftR_32s(aa0, 14);
   aa0 = ShiftR_32s(aa0, 16);
   aa0 = aa0 + 16384;
   den = aa0;
   Divide(16384, 14, den, 14, gl, &nls);
   aa0 = *gl * b;
   *glb = ShiftR_32s(aa0, nls);
   nrs = nls - 14;
   if(nrs > 0) *gl = ShiftR_16s(*gl, nrs);
   return;
}


