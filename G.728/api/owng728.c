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
//  Purpose: G.728 functions
//
*/

#include <ipps.h>
#include "owng728.h"

void Vscale_16s(const Ipp16s *in, Ipp32s len, Ipp32s slen, Ipp16s mls, Ipp16s *out, Ipp16s *nls)
{
   Ipp16s aa0, aa1, maxi, mini;
   Ipp32s i;

   aa0 = aa1 = in[0];
   if(slen!=1) {
      for(i=0; i<slen; i++) {
         if(in[i] > aa0) aa0 = in[i];
         if(in[i] < aa1) aa1 = in[i];
      }
   }

   if((aa0==0)&&(aa1==0)) {
      for(i=0; i<len; i++) out[i] = 0;
      *nls = mls + 1;
      return;
   }

   *nls = 0;

   if((aa0 < 0)||(aa1 < Negate_32s(aa0))) {
      maxi = Negate_16s((short)(1 << mls));
      mini = maxi << 1;
      if (aa1 < mini) {
         while(aa1 < mini) {
            aa1 = ShiftR_16s(aa1, 1);
            *nls = (*nls) - 1;
         } 
         for(i=0;i<len;i++)
            out[i] = ShiftR_16s(in[i], Negate_16s(*nls));
         return;
      } else {
         while(aa1 >= maxi) {
            aa1 = ShiftL_16s(aa1, 1);
            *nls = (*nls) + 1;
         } 
         for(i=0;i<len;i++)
            out[i] = ShiftL_16s(in[i], *nls);

      }
   } else {
      mini = (Ipp16s)(1 << mls);
      maxi = mini - 1;
      maxi = maxi + mini;

      if (aa0 > maxi) {
         while (aa0 > maxi) {
            aa0 = ShiftR_16s(aa0, 1);
            *nls = (*nls) - 1;
         } 
         for(i=0; i<len; i++)
            out[i] = ShiftR_16s(in[i], Negate_16s(*nls));
         return;
      } else {
         while(aa0 < mini) {
            aa0 = ShiftL_16s(aa0, 1);
            *nls = (*nls) + 1;
         } 
         for(i=0;i<len;i++)
            out[i] = ShiftL_16s(in[i], *nls);
      }
      return;
   }
   return;
}

void Vscale_32s(const Ipp32s *in, Ipp32s len, Ipp32s slen, Ipp16s mls, Ipp32s *out, Ipp16s *nls)
{
   Ipp32s aa0, aa1, maxi, mini;
   Ipp32s i;

   aa0 = aa1 = in[0];
   if(slen!=1) {
      for(i=0; i<slen; i++) {
         if(in[i] > aa0) aa0 = in[i];
         if(in[i] < aa1) aa1 = in[i];
      }
   }

   if((aa0==0)&&(aa1==0)) {
      for(i=0; i<len; i++) out[i] = 0;
      *nls = mls + 1;
      return;
   }

   *nls = 0;

   if((aa0 < 0)||(aa1 < Negate_32s(aa0))) {
      maxi = Negate_32s(1 << mls);
      mini = maxi << 1;
      if (aa1 < mini) {
         while(aa1 < mini) {
            aa1 = ShiftR_32s(aa1, 1);
            *nls = (*nls) - 1;
         } 
         for(i=0; i<len; i++)
            out[i] = ShiftR_32s(in[i], Negate_16s(*nls));
         return;
      } else {
         while(aa1 >= maxi) {
            aa1 = ShiftL_32s(aa1, 1);
            *nls = (*nls) + 1;
         } 
         for(i=0;i<len;i++)
            out[i] = ShiftL_32s(in[i], *nls);
      }
   } else {
      mini = 1 << mls;
      maxi = mini - 1;
      maxi = maxi + mini;

      if (aa0 > maxi) {
         while(aa0 > maxi) {
            aa0 = ShiftR_32s(aa0, 1);
            *nls = (*nls) - 1;
         } 
         for(i=0; i<len; i++)
            out[i] = ShiftR_32s(in[i], Negate_16s(*nls));
         return;
      } else {
         while(aa0 < mini) {
            aa0 = ShiftL_32s(aa0, 1);
            *nls = (*nls) + 1;
         } 
         for(i=0;i<len;i++)
            out[i] = ShiftL_32s(in[i],*nls);
      }
      return;
   }
   return;
}

void Divide(Ipp16s num, Ipp16s numnls, Ipp16s den, Ipp16s dennls, Ipp16s *quo, Ipp16s *quonls)
{
   Ipp16s sign = 1;
   Ipp32s p, a0, a1, i;

   p = num * den;
   if(p < 0) sign = -1;

   *quonls = numnls - dennls + 14;

   a0 = Abs_16s(num);
   a1 = Abs_16s(den);

   if(a0 < a1) {
      *quonls = (*quonls) + 1;
      a0 = a0 << 1;
   }

   *quo = 0;
   for(i = 0; i < 15; i++) {
      *quo = ShiftL_16s(*quo, 1);
      if(a0 >= a1) {
         *quo = Add_16s(*quo, 1);
         a0 = a0 - a1;
      }
      a0 = a0 << 1;
   }

   if(a0 >= a1) *quo = Add_16s(*quo, 1);
   if(sign < 0) *quo = (short)(-(*quo));
   return;
}
/*
void SimpDiv(Ipp16s num, Ipp16s den, Ipp32s *aa0) {
   Ipp32s result, L_den, L_num, k;

   result = 0;
   L_num = num;
   L_den = den;
   for(k = 0; k < 16;k++) {
      result = ShiftL_32s(result, 1);
      L_num = ShiftL_32s(L_num, 1);
      if(L_num >= L_den) {
         L_num = Sub_32s(L_num, L_den);
         result = Add_32s(result,1);
      }
   }
   *aa0 = result;
   return;
}
*/
void LevinsonDurbin(const short *rtmp, int ind1, int ind2, short *atmp, short *rc1, 
         short *alphatmp, short *nlsatmp, short *illcondp, short *illcond) {

   if ((*illcond==1) || (rtmp[0]<=0)) {
      *illcond = 1;
   } else {
      IppStatus sts;
      if(ind1 > 1)
         sts = ippsLevinsonDurbin_G728_16s_ISfs(rtmp, ind1, ind2, atmp, alphatmp, nlsatmp);
      else
         sts = ippsLevinsonDurbin_G728_16s_Sfs(rtmp, ind2, atmp, rc1,alphatmp, nlsatmp);
      if(sts != ippStsNoErr){
         *illcond = 1;
         if(sts == ippStsLPCCalcErr)
            *illcondp = 1;
      }
   }
}

void Get_shape_and_gain_idxs(Ipp16s ichan, Ipp32s *is, Ipp32s *ig, Ipp16s rate) {

   if(rate==G728_Rate_12800){
      Ipp32s is1;

      *is = ichan >> 2,
      *ig = ichan - (*is*NG_128);

      is1 = NCWD;
      is1 = is1 >> 1;
      *is = (*is) + is1;
   } else if(rate==G728_Rate_9600) {
      Ipp32s is1, is2, is3;

      is3 = ichan >> 2,
      *ig = ichan - (is3 * NG_96);

      is1 = is3 >> 2;
      is2 = is3 & 0x3;
      is1 = is1 * 8;
      is1 = is1 + is2;
      *is = is1 + NCWD3_4;
   } else if (rate==G728_Rate_16000){
      *is = ichan >> 3,
      *ig = ichan & 7;
   }
}




