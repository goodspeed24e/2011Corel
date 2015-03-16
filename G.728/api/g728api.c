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
//  Purpose: G.728 decode API
//
*/

#include <stdlib.h>
#include <stdio.h>
#include <ipps.h>
#include "g728api.h"
#include "owng728.h"
#include "g728tables.h"

static const IppSpchBitRate rate2ipp[3] = {
   IPP_SPCHBR_16000,
   IPP_SPCHBR_12800,
   IPP_SPCHBR_9600
};

#define ADD_ALIGN_MEM_BLOCK(align,addr,length) (addr + length + align -1)

G728_CODECFUN( APIG728_Status, apiG728Encoder_GetSize, 
         (G728Encoder_Obj* encoderObj, unsigned int *pCodecSize))
{
   if(NULL == encoderObj)
      return APIG728_StsBadArgErr;
   if(NULL == pCodecSize)
      return APIG728_StsBadArgErr;
   if(encoderObj->objPrm.key != ENC_KEY)
      return APIG728_StsNotInitialized;

   *pCodecSize = encoderObj->objPrm.objSize;
   return APIG728_StsNoErr;
}

G728_CODECFUN( APIG728_Status, apiG728Decoder_GetSize, 
         (G728Decoder_Obj* decoderObj, unsigned int *pCodecSize))
{
   if(NULL == decoderObj)
      return APIG728_StsBadArgErr;
   if(NULL == pCodecSize)
      return APIG728_StsBadArgErr;
   if(decoderObj->objPrm.key != DEC_KEY)
      return APIG728_StsNotInitialized;

   *pCodecSize = decoderObj->objPrm.objSize;
   return APIG728_StsNoErr;
}

G728_CODECFUN( APIG728_Status, apiG728Encoder_Alloc,(unsigned int* objSize))
{
   int rexpMemSize=IPP_MAX_32S;
   int rexpwMemSize=IPP_MAX_32S;
   int rexplgMemSize=IPP_MAX_32S;
   int combSize=IPP_MAX_32S;
   int iirMemSize=IPP_MAX_32S;
   char* eObj = NULL; 

   eObj = (char*)IPP_ALIGNED_PTR(eObj+sizeof(G728Encoder_Obj), 16);

   ippsIIR16sGetStateSize_G728_16s(&iirMemSize);
   eObj = (char*)IPP_ALIGNED_PTR(eObj+iirMemSize, 16);

   ippsCombinedFilterGetStateSize_G728_16s(&combSize);
   eObj = (char*)IPP_ALIGNED_PTR(eObj+combSize, 16);

   ippsWinHybridGetStateSize_G728_16s(LPCLG, NUPDATE, NONRLG, 0, &rexplgMemSize);
   eObj = (char*)IPP_ALIGNED_PTR(eObj+rexplgMemSize, 16);

   ippsWinHybridGetStateSize_G728_16s(LPCW, NFRSZ, NONRW, 0, &rexpwMemSize);
   eObj = (char*)IPP_ALIGNED_PTR(eObj+rexpwMemSize, 16);

   ippsWinHybridGetStateSize_G728_16s(LPC, NFRSZ, NONR, IDIM, &rexpMemSize);
   eObj = (char*)IPP_ALIGNED_PTR(eObj+rexpMemSize, 16);

   *objSize = (Ipp32u)(eObj - (char*)NULL);
   return APIG728_StsNoErr;
}

G728_CODECFUN( APIG728_Status, apiG728Encoder_Init,(G728Encoder_Obj* eObj, G728_Rate rate))
{     
   int rexpMemSize=IPP_MAX_32S;
   int rexpwMemSize=IPP_MAX_32S;
   int rexplgMemSize=IPP_MAX_32S;
   int combSize=IPP_MAX_32S;
   int iirMemSize=IPP_MAX_32S;
   char* tmpObjPtr = (char*)eObj; 
   unsigned int objSize;

   if((int)eObj & 0x7){ /* shall be at least 8 bytes aligned */ 
      return APIG728_StsNotInitialized;
   }

   ippsZero_16s((Ipp16s*) eObj,sizeof(G728Encoder_Obj)/2);

   eObj->objPrm.key = ENC_KEY;
   eObj->objPrm.rate = rate;

   eObj->h[0] = 8192;
   eObj->gp[1] = -16384;
   ippsSet_16s(-16384, eObj->gstate, LPCLG);
   ippsSet_16s(16, eObj->nlssttmp, 4);
   if(rate==G728_Rate_12800)     { 
      eObj->pGq = gq_128; 
      eObj->pNgq = nngq_128;
      eObj->pGcblg = gcblg_128;
   }
   else if(rate==G728_Rate_9600) { 
      eObj->pGq = gq_96;
      eObj->pNgq = nngq_96;
      eObj->pGcblg = gcblg_96;
   }
   else if(rate==G728_Rate_16000){ 
      eObj->pGq = gq;
      eObj->pNgq = nngq;
      eObj->pGcblg = gcb;
   }
   ippsImpulseResponseEnergy_G728_16s(eObj->h, eObj->y2);

   tmpObjPtr = (char*)IPP_ALIGNED_PTR(tmpObjPtr+sizeof(G728Encoder_Obj), 16);

   eObj->wgtMem = (IppsIIRState_G728_16s*)tmpObjPtr;
   ippsIIR16sGetStateSize_G728_16s(&iirMemSize);
   tmpObjPtr = (char*)IPP_ALIGNED_PTR(tmpObjPtr+iirMemSize, 16);

   eObj->combMem = (IppsCombinedFilterState_G728_16s*)tmpObjPtr;
   ippsCombinedFilterGetStateSize_G728_16s(&combSize);
   tmpObjPtr = (char*)IPP_ALIGNED_PTR(tmpObjPtr+combSize, 16);

   eObj->rexplgMem = (IppsWinHybridState_G728_16s*)tmpObjPtr;
   ippsWinHybridGetStateSize_G728_16s(LPCLG, NUPDATE, NONRLG, 0, &rexplgMemSize);
   tmpObjPtr = (char*)IPP_ALIGNED_PTR(tmpObjPtr+rexplgMemSize, 16);

   eObj->rexpwMem = (IppsWinHybridState_G728_16s*)tmpObjPtr;
   ippsWinHybridGetStateSize_G728_16s(LPCW, NFRSZ, NONRW, 0, &rexpwMemSize);
   tmpObjPtr = (char*)IPP_ALIGNED_PTR(tmpObjPtr+rexpwMemSize, 16);

   eObj->rexpMem = (IppsWinHybridState_G728_16s*)tmpObjPtr;
   ippsWinHybridGetStateSize_G728_16s(LPC, NFRSZ, NONR, IDIM, &rexpMemSize);
   tmpObjPtr = (char*)IPP_ALIGNED_PTR(tmpObjPtr+rexpMemSize, 16);
   
   eObj->objPrm.objSize = (Ipp32u)((char*)tmpObjPtr - (char*)eObj);
   apiG728Encoder_Alloc(&objSize); 
   if(objSize != eObj->objPrm.objSize){
      return APIG728_StsNotInitialized; /* must not occur */ 
   }
   ippsIIR16sInit_G728_16s(eObj->wgtMem);
   ippsCombinedFilterInit_G728_16s(eObj->combMem);
   ippsWinHybridInit_G728_16s(wnrlg,LPCLG, NUPDATE, NONRLG, 0, 12288, eObj->rexplgMem);
   ippsWinHybridInit_G728_16s(wnrw,LPCW, NFRSZ, NONRW, 0, 8192, eObj->rexpwMem);
   ippsWinHybridInit_G728_16s(wnr,LPC, NFRSZ, NONR, IDIM, 12288, eObj->rexpMem);


   return APIG728_StsNoErr;
}

static void prm2bits(const short* prm, unsigned char* bitstream, G728_Rate rate)
{
   if(rate==G728_Rate_12800)     { 
      bitstream[0] = (unsigned char) prm[0];                               
      bitstream[1] = (unsigned char) prm[1]; 
      bitstream[2] = (unsigned char) prm[2]; 
      bitstream[3] = (unsigned char) prm[3]; 
   }
   else if(rate==G728_Rate_9600) { 
      bitstream[0] = (unsigned char) (( prm[0] << 2 ) | ( prm[1] >> 4));       
      bitstream[1] = (unsigned char) (( (prm[1] & 0xf) << 4 ) | (prm[2] >> 2));
      bitstream[2] = (unsigned char) (( (prm[2] & 0x3) << 6 )| prm[3]);        
   }
   else if(rate==G728_Rate_16000){ 
      bitstream[0] = (unsigned char) ( prm[0] >> 2);                               
      bitstream[1] = (unsigned char) (( (prm[0] & 0x3) << 6 ) | (prm[1] >> 4)); 
      bitstream[2] = (unsigned char) (( (prm[1] & 0xf) << 4 ) | (prm[2] >> 6)); 
      bitstream[3] = (unsigned char) (( (prm[2] & 0x3f) << 2 )| (prm[3] >> 8)); 
      bitstream[4] = (unsigned char) (prm[3] & 0xff);                              
   }
}

static Ipp16s EncOncePerFrameProcessing(G728Encoder_Obj* eObj, const Ipp16s *src, 
                                        Ipp16s index){
   Ipp32s loggain;
   Ipp16s gain;
   Ipp16s nlsgain;
   Ipp16s nlstarget;
   Ipp16s ichan;
   Ipp32s ig, is, i;
   Ipp32s aa0;
   Ipp16s tmp;
   Ipp16s rate = eObj->objPrm.rate;
   Ipp16s nlset;

   IPP_ALIGNED_ARRAY(16, Ipp16s, sw, (IDIM +3)); /* IDIM + 3 = 16 byte */ 
   IPP_ALIGNED_ARRAY(16, Ipp16s, target, (IDIM +3));
   IPP_ALIGNED_ARRAY(16, Ipp16s, pn, (IDIM +3));
   IPP_ALIGNED_ARRAY(16, Ipp16s, tempZIR, (IDIM +3));
   IPP_ALIGNED_ARRAY(16, Ipp16s, et, (IDIM +3));

   /* Get backward-adapted gain */ 
   /* gstate[1:9] shifted down 1 position */ 
   Log_gain_linear_prediction(eObj->gp, eObj->gstate, &loggain);

   if(loggain > 14336) loggain = 14336;
   if(loggain < -16384) loggain = -16384;

   Inverse_logarithm_calc(loggain, &gain, &nlsgain);
   /* Synthesis filter with zero input combined with perceptual weighting filter */ 
   ippsCombinedFilterZeroInput_G728_16s(eObj->a,eObj->wgtA,tempZIR,eObj->combMem);
   /* Perceptual weighting filter */ 
   ippsIIR16s_G728_16s((Ipp16s*)eObj->wgtA, src, sw, IDIM, eObj->wgtMem);
   /* VQ target vector computation */ 
   VQ_target_vector_calc(sw, tempZIR, target);
   /* VQ target vector normalization */ 
   nlstarget = 2;
   VQ_target_vec_norm(gain, nlsgain, target, &nlstarget);
   /* Time-reversed convolution */ 
   Time_reversed_conv(eObj->h, target, nlstarget, pn);
   /* Excitation codebook search */ 
   ippsCodebookSearch_G728_16s(pn, eObj->y2, &is, &ig, &ichan, rate2ipp[rate]);
   /* Scale selected excitation codevector */ 
   aa0 = eObj->pGq[ig] * gain;
   aa0 = ShiftL_32s(aa0, eObj->pNgq[ig]);
   tmp = Cnvrt_NR_32s16s(aa0);
   nlset = nlsgain + eObj->pNgq[ig] + shape_all_nls[is] - 8;
   Excitation_VQ_and_gain_scaling(tmp, &shape_all_norm[is * IDIM], et);
   /* Memory update */ 
   ippsCombinedFilterZeroState_G728_16s(eObj->a, eObj->wgtA, 
      et, nlset, eObj->st, &eObj->nlsst, eObj->combMem);
   /* Update log-gain and gain predictor memory */ 
   eObj->gstate[0] = Log_gain_adder_and_limiter(loggain, (Ipp16s)ig, (Ipp16s)is, 
      eObj->pGcblg, shape);
   
   i = (index - 1) *IDIM;
   ippsCopy_16s(eObj->st,&eObj->sttmp[i],5);
   eObj->nlssttmp[index-1] = eObj->nlsst;
   
   /* stmp - cyclic buffer                                        */ 
   /* index   stmp                                                */ 
   /*      1            0,         0, src[ 0: 4],         0,      */ 
   /*      2            0,         0, src[ 0: 4],src[ 5: 9],      */ 
   /*      3   src[10:14],         0, src[ 0: 4],src[ 5: 9],      */ 
   /*      4   src[10:14],src[15:19], src[ 0: 4],src[ 5: 9],      */ 
   /*      1   src[10:14],src[15:19], src[20:24],src[ 5: 9],      */ 
   /*      2   src[10:14],src[15:19], src[20:24],src[25:29], .... */ 
   i = (index + 1) & 0x3;
   i *= IDIM;
   ippsCopy_16s(src,&eObj->stmp[i],5);
   return ichan;
}

G728_CODECFUN( APIG728_Status, apiG728Encode,
         (G728Encoder_Obj* eObj, short *src, unsigned char *dst))
{
   Ipp16s gtmp[4];
   Ipp16s foo;
   Ipp16s ichan[4];/* four codebook indexes */ 
   int index;

   /* four vectors of 5 samples length  (20 short integer)*/ 
   eObj->icount = eObj->icount & 3;
   eObj->icount++;
   index = 1;
      ichan[index-1] = EncOncePerFrameProcessing(eObj,src,(Ipp16s)index);
      gtmp[0] = eObj->gstate[3];
      gtmp[1] = eObj->gstate[2];
      gtmp[2] = eObj->gstate[1];
      gtmp[3] = eObj->gstate[0];
      /* Block 43 */ 
      eObj->illcondg=0;
      if(ippsWinHybrid_G728_16s(0, gtmp, eObj->r, eObj->rexplgMem ) != ippStsNoErr){
         eObj->illcondg=1;
      };
      ippsZero_16s(eObj->gptmp,LPCLG+1);
      LevinsonDurbin(eObj->r, 0, LPCLG, eObj->gptmp+1, &foo, &foo, 
         &eObj->nlsgptmp, &eObj->illcondp, &eObj->illcondg);

   src += 5;
   eObj->icount = eObj->icount & 3;
   eObj->icount++;
   index = 2;
      if(eObj->illcondg==0) {
         Bandwidth_expansion_block45(eObj->gptmp, eObj->nlsgptmp, eObj->gp);
      }
      ichan[index-1] = EncOncePerFrameProcessing(eObj,src,(Ipp16s)index);
      /* Block 36 */ 
      eObj->illcondw=0;
      if(ippsWinHybrid_G728_16s(0, eObj->stmp, eObj->r, eObj->rexpwMem ) != ippStsNoErr){
         eObj->illcondw=1;
      };
      ippsZero_16s(eObj->awztmp,LPCW);
      LevinsonDurbin(eObj->r, 0, LPCW, eObj->awztmp, &foo, &foo,
         &eObj->nlsawztmp, &eObj->illcondp, &eObj->illcondw);

   src += 5;
   eObj->icount = eObj->icount & 3;
   eObj->icount++;
   index = 3;
      if(eObj->illcond==0) 
         Bandwidth_expansion_block51(eObj->atmp, eObj->nlsatmp, eObj->a);
      if(eObj->illcondw==0)
         Weighting_filter_coef_calc( wzcfv, wpcfv, eObj->awztmp,
            eObj->nlsawztmp, eObj->wgtA);
      Impulse_response_vec_calc(eObj->a, eObj->wgtA, eObj->h);
      ippsImpulseResponseEnergy_G728_16s(eObj->h, eObj->y2);
      ichan[index-1]  = EncOncePerFrameProcessing(eObj,src,(Ipp16s)index);

   src += 5;
   eObj->icount = eObj->icount & 3;
   eObj->icount++;
   index = 4;
      ichan[index-1] = EncOncePerFrameProcessing(eObj,src,(Ipp16s)index);
      eObj->illcond = 0;
      if(ippsWinHybridBlock_G728_16s(0, eObj->sttmp, eObj->nlssttmp,
         eObj->rtmp, eObj->rexpMem ) != ippStsNoErr){
         eObj->illcond = 1;
      };
      ippsZero_16s(eObj->atmp,LPC);
      LevinsonDurbin(eObj->rtmp, 0, LPC, eObj->atmp, &foo, &foo,
         &eObj->nlsatmp, &eObj->illcondp, &eObj->illcond);

   /* pack indexes into bitstream */ 
   prm2bits(ichan,dst,eObj->objPrm.rate);
   /* End once-per-frame processing */ 
   return APIG728_StsNoErr;
}

G728_CODECFUN( APIG728_Status, apiG728Decoder_Alloc,(unsigned int* objSize))
{
   int rexpMemSize=IPP_MAX_32S;
   int rexplgMemSize=IPP_MAX_32S;
   int iirMemSize=IPP_MAX_32S;
   int stpMemSize=IPP_MAX_32S;
   int syntMemSize=IPP_MAX_32S;
   char* dObj = NULL; 

   dObj = (char*)IPP_ALIGNED_PTR(dObj+sizeof(G728Decoder_Obj),8);

   ippSynthesisFilterGetStateSize_G728_16s(&syntMemSize);
   dObj = (char*)IPP_ALIGNED_PTR(dObj+syntMemSize,8);

   ippsPostFilterGetStateSize_G728_16s(&stpMemSize);
   ippsIIR16sGetStateSize_G728_16s(&iirMemSize);
   dObj = (char*)IPP_ALIGNED_PTR(dObj+IPP_MAX(stpMemSize,iirMemSize),8);

   ippsWinHybridGetStateSize_G728_16s(LPCLG, NUPDATE, NONRLG, 0, &rexplgMemSize);
   dObj = (char*)IPP_ALIGNED_PTR(dObj+rexplgMemSize,8);

   ippsWinHybridGetStateSize_G728_16s(LPC, NFRSZ, NONR, IDIM, &rexpMemSize);
   dObj = (char*)IPP_ALIGNED_PTR(dObj+rexpMemSize,8);

   *objSize = (Ipp32u)(dObj - (char*)NULL);

    return APIG728_StsNoErr;
}
G728_CODECFUN( APIG728_Status, apiG728Decoder_Init,(
              G728Decoder_Obj* dObj, G728_Type type, G728_Rate rate, int pst))
{  
   int rexpMemSize=IPP_MAX_32S;
   int rexplgMemSize=IPP_MAX_32S;
   int iirMemSize=IPP_MAX_32S;
   int stpMemSize=IPP_MAX_32S;
   int syntMemSize=IPP_MAX_32S;
   char* tmpObjPtr = (char*)dObj; 
   unsigned int objSize;

   if((int)dObj & 0x7){ /* shall be at least 8 bytes aligned */ 
      return APIG728_StsNotInitialized;
   }

   ippsZero_16s((Ipp16s*) dObj,sizeof(G728Decoder_Obj)/2);

   dObj->objPrm.key = DEC_KEY;
   dObj->objPrm.rate = rate;
   dObj->objPrm.type = type;
   

   dObj->gl = 16384;
   dObj->pst = pst;   
   dObj->gp[0] = 16384;
   dObj->gp[1] = -16384;
   dObj->ip = NPWSZ - NFRSZ + IDIM;
   dObj->kp1 = 50;
   dObj->scalefil = 16384;
   if(rate==G728_Rate_12800)     { 
      dObj->pGq = gq_128; 
      dObj->pNgq = nngq_128;
      dObj->pGcblg = gcblg_128;
   }
   else if(rate==G728_Rate_9600) { 
      dObj->pGq = gq_96;
      dObj->pNgq = nngq_96;
      dObj->pGcblg = gcblg_96;
   }
   else if(rate==G728_Rate_16000){ 
      dObj->pGq = gq;
      dObj->pNgq = nngq;
      dObj->pGcblg = gcb;
   }
   ippsSet_16s(16, dObj->nlssttmp, 4);
   ippsSet_16s(-16384, dObj->gstate, LPCLG);
   ippsZero_16s(dObj->etpast_buff, 140+IDIM);

   tmpObjPtr = (char*)IPP_ALIGNED_PTR(tmpObjPtr+sizeof(G728Decoder_Obj),8);

   dObj->syntState = (IppsSynthesisFilterState_G728_16s*)tmpObjPtr;
   ippSynthesisFilterGetStateSize_G728_16s(&syntMemSize);
   tmpObjPtr = (char*)IPP_ALIGNED_PTR(tmpObjPtr+syntMemSize,8);

   dObj->stpMem = (IppsPostFilterState_G728_16s*)tmpObjPtr;
   if(dObj->pst){
      ippsPostFilterInit_G728_16s(dObj->stpMem);
   }else{
      ippsIIR16sInit_G728_16s((IppsIIRState_G728_16s*)dObj->stpMem);
   }
   ippsPostFilterGetStateSize_G728_16s(&stpMemSize);
   ippsIIR16sGetStateSize_G728_16s(&iirMemSize);
   tmpObjPtr = (char*)IPP_ALIGNED_PTR(tmpObjPtr+IPP_MAX(stpMemSize,iirMemSize),8);

   dObj->rexplgMem = (IppsWinHybridState_G728_16s*)tmpObjPtr;
   ippsWinHybridGetStateSize_G728_16s(LPCLG, NUPDATE, NONRLG, 0, &rexplgMemSize);
   tmpObjPtr = (char*)IPP_ALIGNED_PTR(tmpObjPtr+rexplgMemSize,8);

   dObj->rexpMem = (IppsWinHybridState_G728_16s*)tmpObjPtr;
   ippsWinHybridGetStateSize_G728_16s(LPC, NFRSZ, NONR, IDIM, &rexpMemSize);
   tmpObjPtr = (char*)IPP_ALIGNED_PTR(tmpObjPtr+rexpMemSize,8);

   dObj->objPrm.objSize = (Ipp32u)((char*)tmpObjPtr - (char*)dObj);
   apiG728Decoder_Alloc(&objSize); 
   if(objSize != dObj->objPrm.objSize){ 
      return APIG728_StsNotInitialized; /* must not occur */ 
   }
   ippSynthesisFilterInit_G728_16s(dObj->syntState);
   ippsWinHybridInit_G728_16s(wnrlg,LPCLG, NUPDATE, NONRLG, 0, 12288, dObj->rexplgMem);
   ippsWinHybridInit_G728_16s(wnr,LPC, NFRSZ, NONR, IDIM, 12288, dObj->rexpMem);

   dObj->ferror = G728_FALSE;

   if (type == G728_Annex_I) {
      dObj->adcount = FESIZE;
      dObj->fecount = 0;
      dObj->afterfe = 0;
      dObj->ogaindb = -16384;
      dObj->seed = 11111;
      dObj->pTab = 0;
      dObj->fescale = 26214;
   }

   return APIG728_StsNoErr;
}

static void bits2prm(const unsigned char* bitstream, short* prm, G728_Rate rate)
{
   if(rate==G728_Rate_12800)     { 
      prm[0] = bitstream[0];   
      prm[1] = bitstream[1]; 
      prm[2] = bitstream[2]; 
      prm[3] = bitstream[3]; 
   }
   else if(rate==G728_Rate_9600) { 
      prm[0] = (bitstream[0] >> 2);                               
      prm[1] = ( (bitstream[0] & 0x3) << 4 ) | (bitstream[1] >> 4); 
      prm[2] = ( (bitstream[1] & 0xf) << 2 ) | (bitstream[2] >> 6);  
      prm[3] = (bitstream[2] & 0x3f);                             
   }
   else if(rate==G728_Rate_16000){ 
      prm[0] = (bitstream[0] << 2) | (bitstream[1] >> 6);            
      prm[1] = ( (bitstream[1] & 0x3f) << 4 ) | (bitstream[2] >> 4); 
      prm[2] = ( (bitstream[2] & 0xf) << 6 ) | (bitstream[3] >> 2);  
      prm[3] = ( (bitstream[3] & 0x3) << 8 ) | bitstream[4];         
   }
}


G728_CODECFUN( APIG728_Status, apiG728Decode, (G728Decoder_Obj* dObj,
              unsigned char *packet, short *dst))
{
   Ipp32s is, ig, loggain;
   Ipp16s gain, nlsgain;
   Ipp16s nlset;
   Ipp16s nlsst;
   Ipp32s sumunfil, sumfil;
   Ipp16s scale, nlsscale;
   G728_Rate rate = dObj->objPrm.rate;
   G728_Type codecType = dObj->objPrm.type;
   Ipp32s i;
   Ipp32s aa0;
   Ipp16s tmp;
   Ipp16s foo;
   Ipp16s *sst = &dObj->sst_buff[239+1+1];
   Ipp16s *d = &dObj->d_buff[139+1];
   Ipp16s *etpast = &dObj->etpast_buff[140];
   Ipp16s  tiltz;
   Ipp16s ichan[4];/* four codebook indexes */ 
   int index = 0;

   IPP_ALIGNED_ARRAY(16, Ipp16s, et, (IDIM +3));
   IPP_ALIGNED_ARRAY(16, Ipp16s, st, (IDIM +3));
   IPP_ALIGNED_ARRAY(16, Ipp16s, temp, (IDIM +3));
   IPP_ALIGNED_ARRAY(16, Ipp16s, spf, (IDIM +3));
   IPP_ALIGNED_ARRAY(16, Ipp16s, gtmp, (4 +4));
   IPP_ALIGNED_ARRAY(16, Ipp16s, r, (11 +5));
   IPP_ALIGNED_ARRAY(16, Ipp16s, rtmp, (LPC+1 +5));

   /* unpack bitstream into indexes */ 
   if(codecType == G728_Annex_I) {
      bits2prm(&packet[1],ichan,rate);
   } else {
      bits2prm(packet,ichan,rate);
   }

   if(codecType == G728_Annex_I) {
      if((dObj->ferror == G728_FALSE)&&(dObj->afterfe > 0))
         dObj->afterfe -= 1;
      if(dObj->adcount == FESIZE) {
         dObj->adcount = 0;
         /* read dObj->ferror */ 
         dObj->ferror = (Ipp16s)packet[0];
               
         if((dObj->ferror == G728_FALSE)&&(dObj->fecount > 0)) {
            dObj->afterfe += dObj->fecount;
            if(dObj->afterfe > AFTERFEMAX)
               dObj->afterfe = AFTERFEMAX;
            dObj->fecount = 0;
            dObj->seed = 11111;
         }
      }
      if(dObj->ferror == G728_TRUE) {
         dObj->fecount += 1;
         if((dObj->fecount & 3) == 1)
            Set_Flags_and_Scalin_Factor_for_Frame_Erasure(dObj->fecount, dObj->pTab, 
               (Ipp16s)dObj->kp1, &dObj->fedelay, &dObj->fescale, &dObj->nlsfescale, 
               &dObj->voiced, etpast,&dObj->avmag, &dObj->nlsavmag);
      }
      dObj->adcount += 1;
   }

   for(index=1;  index < 5; index++){
      dObj->icount = dObj->icount & 3;
      dObj->icount++;
      /* Check wether to update filter coefficients */ 
      if(index==3) {
         if((dObj->ferror == G728_FALSE)&&(dObj->illcond==G728_FALSE))
            Bandwidth_expansion_block51(dObj->atmp, dObj->nlsatmp, dObj->a);
         if((dObj->ferror == G728_TRUE)&&((dObj->fecount & 3) == 1))
            Bandwidth_expansion_block51FE(dObj->fecount, dObj->illcond, dObj->atmp, dObj->nlsatmp, dObj->a);
      }
      if((index==2)&&(dObj->illcondg==0)&&(dObj->ferror == G728_FALSE))
         Bandwidth_expansion_block45(dObj->gptmp, dObj->nlsgptmp, dObj->gp);
      /* Obtain the shape index IS and gain index IG from ICHAN */ 
      Get_shape_and_gain_idxs(ichan[index-1], &is, &ig, (Ipp16s)rate);
      /* Get backward-adapted gain */ 
      /* GSTATE[1:9] shifted down 1 position */ 
      Log_gain_linear_prediction(dObj->gp, dObj->gstate, &loggain); /* Block 46 */ 
      if(dObj->ferror == G728_FALSE) {
         Log_gain_Limiter_after_erasure(&loggain, dObj->ogaindb, dObj->afterfe);
         dObj->ogaindb = loggain;
         Inverse_logarithm_calc(loggain, &gain, &nlsgain);
         
         aa0 = dObj->pGq[ig] * gain;
         aa0 = ShiftL_32s(aa0, dObj->pNgq[ig]);
         tmp = Cnvrt_NR_32s16s(aa0);
         nlset = nlsgain + dObj->pNgq[ig] + shape_all_nls[is] - 8;
         Excitation_VQ_and_gain_scaling(tmp, &shape_all_norm[is * IDIM], et);
      }

      if(dObj->ferror == G728_TRUE)
         Excitation_signal_extrapolation(dObj->voiced, &dObj->fedelay, dObj->fescale, dObj->nlsfescale,
                                       etpast, et, &nlset, &dObj->seed);

      Excitation_Signal_Update(etpast, et, nlset);

      ippsSyntesisFilterZeroInput_G728_16s(dObj->a, et, nlset, st, &nlsst,dObj->syntState);
      /* Update short-term postfilter coefficiens */ 
      if(dObj->pst==1){
         if(index==1 && dObj->illcondp != 1) 
            Short_term_posfilter_coef_calc(spfpcfv, spfzcfv,dObj->apf, dObj->nlsapf, 
                           dObj->pstA, dObj->rc1, &tiltz);
         for(i=0;i < IDIM; i++) {
            if (18 > nlsst)
               aa0 = ShiftL_32s(st[i], (Ipp16s)(18 - nlsst));
            else
               aa0 = ShiftR_32s(st[i], (Ipp16s)(nlsst - 18));
            sst[i] = Cnvrt_NR_32s16s(aa0);
         }
         LPC_inverse_filter(&dObj->ip, sst, dObj->stlpci, dObj->apf, d);

         if(index==3) {
            /*  Pitch period extraction */ 
            Pitch_period_extraction(d, bl, al, dObj->lpffir, dObj->lpfiir,
                      &dObj->dec_buff[35], &dObj->kp1);
            /* Compute long-term postfilter coefficients */ 
            LTP_coeffs_calc(sst, dObj->kp1, &dObj->gl, &dObj->glb, &dObj->pTab);
         }  
         /* Long-term and short-term postfilters */ 
         ippsPostFilter_G728_16s(dObj->gl, dObj->glb, (Ipp16s)dObj->kp1, 
               tiltz, dObj->pstA, sst,  temp, dObj->stpMem);
      } else { /*  dObj->pst == 0 */ 
         /* Upscale on Q3 */ 
         for(i=0;i < IDIM; i++) { 
            if (19 > nlsst)
               aa0 = ShiftL_32s(st[i], (Ipp16s)(19 - nlsst));
            else 
               aa0 = ShiftR_32s(st[i], (Ipp16s)(nlsst - 19));
            sst[i] = Cnvrt_NR_32s16s(aa0);
         }
         /* only short term postfilter */ 
         ippsIIR16s_G728_16s(dObj->pstA,sst,temp,5,(IppsIIRState_G728_16s*)dObj->stpMem);
      }

      for(i=-NPWSZ-KPMAX;i < -4; i++)
         sst[i] = sst[i+IDIM];
      for(i=-5; i < 0; i++)
         sst[i] = sst[i+IDIM] >> 2;

      if(dObj->pst==1) {
         /* Calculate sums of absolute values */ 
         Sum_of_absolute_value_calc(sst, temp, &sumunfil, &sumfil);
         /* Ratio of sums of absolute values */ 
         Scaling_factor_calc(sumunfil, sumfil, &scale, &nlsscale);
         /* Low-pass filter of scaling factor */ 
         /* Gain control of postfilter output */ 
         First_order_lowpass_filter_ouput_gain_scaling(scale, nlsscale, &dObj->scalefil,
            temp, spf);
      }
      /* Update log-gain and gain predictor memory */ 
      if(dObj->ferror == G728_FALSE)
         dObj->gstate[0] = Log_gain_adder_and_limiter(loggain, (Ipp16s)ig, 
            (Ipp16s)is, dObj->pGcblg, shape);

      if(dObj->ferror == G728_TRUE) {
         Update_GSTATE_Freame_Erasures(et, dObj->gstate);
         dObj->ogaindb = dObj->gstate[0];
      }

      i = (index - 1) *IDIM;
      /* sttmp - cyclic buffer                                    */ 
      /* icount   sttmp                                           */ 
      /*      1   st[ 0: 4],        0,         0,        0,       */ 
      /*      2   st[ 0: 4],st[ 5: 9],         0,        0,       */ 
      /*      3   st[ 0: 4],st[ 5: 9], st[10:14],        0,       */ 
      /*      4   st[ 0: 4],st[ 5: 9], st[10:14],st[15:19],       */ 
      /*      1   st[20:24],st[ 5: 9], st[10:14],st[15:19], ....  */ 
      ippsCopy_16s(st,&dObj->sttmp[i],5);
      dObj->nlssttmp[index-1] = nlsst;
      /* Start once-per-frame processing */ 
      if(index == 4) {
         short alphatmp;
         dObj->illcond = 0;
         if(ippsWinHybridBlock_G728_16s(dObj->ferror, dObj->sttmp, dObj->nlssttmp, 
            rtmp, dObj->rexpMem ) != ippStsNoErr){
            dObj->illcond = 1;
         }
         ippsZero_16s(dObj->atmp,LPC);
         if(dObj->pst==1) {
            LevinsonDurbin(rtmp, 0, 10, dObj->atmp, &dObj->rc1, &alphatmp,
               &dObj->nlsatmp, &dObj->illcondp, &dObj->illcond);
            /* Save the 10th-order predictor for postfilter use later */ 
            dObj->nlsapf = dObj->nlsatmp;
            ippsCopy_16s(&dObj->atmp[0],&dObj->apf[1],10);
            /* Continue to finish Levinson Durbin */ 
            if(dObj->ferror == G728_FALSE)
               LevinsonDurbin(rtmp, 10, LPC, &dObj->atmp[0],  &foo,
                  &alphatmp,&dObj->nlsatmp, &dObj->illcondp, &dObj->illcond);
         } else {
            LevinsonDurbin(rtmp, 0, LPC, dObj->atmp, &foo, &foo,
               &dObj->nlsatmp, &dObj->illcondp, &dObj->illcond);
         }
      }

      if(index == 1) {
         Ipp16s foo;
         gtmp[0] = dObj->gstate[3];
         gtmp[1] = dObj->gstate[2];
         gtmp[2] = dObj->gstate[1];
         gtmp[3] = dObj->gstate[0];
         /* Block 43 */ 
         dObj->illcondg=0;
         if(ippsWinHybrid_G728_16s(dObj->ferror, gtmp, r, dObj->rexplgMem) != ippStsNoErr){
            dObj->illcondg=1;
         };
         if(dObj->ferror == G728_FALSE) {
            ippsZero_16s(dObj->gptmp,LPCLG+1);
            LevinsonDurbin(r, 0, LPCLG, dObj->gptmp+1, &foo, &foo,
               &dObj->nlsgptmp, &dObj->illcondp, &dObj->illcondg);
         }
      }
      /* End once-per-frame processing */ 
      if(dObj->pst==0){
         dst[0] = sst[0];
         dst[1] = sst[1];
         dst[2] = sst[2];
         dst[3] = sst[3];
         dst[4] = sst[4];
      }else{
         dst[0] = ShiftL_16s(spf[0], 1);
         dst[1] = ShiftL_16s(spf[1], 1);
         dst[2] = ShiftL_16s(spf[2], 1);
         dst[3] = ShiftL_16s(spf[3], 1);
         dst[4] = ShiftL_16s(spf[4], 1);
      }
      dst += 5;
   }
   return APIG728_StsNoErr;
}


