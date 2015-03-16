/*!
************************************************************************
*  \file
*     loopfilter.h
*  \brief
*     external loop filter interface
************************************************************************
*/

#ifndef _LOOPFILTER_H_
#define _LOOPFILTER_H_

#include "global.h"
#include "mbuffer.h"

void DeblockSlice PARGS3(StorablePicture *p, int start_mb, int num_mb);
void DeblockMb PARGS7(StorablePicture *p, int MbQAddr, int mbx, int mby, Macroblock_s *Mb_left, Macroblock_s *Mb_top, Macroblock_s  *MbQ) ;
int  GetStrength_v(byte *Strength,Macroblock_s *MbQ,int MbQAddr,int mbx,int mby,Macroblock_s *MbP,int edge,int mvlimit,bool bMixedEdge);
int  GetStrength_v_P(byte *Strength,Macroblock_s *MbQ,int MbQAddr,int mbx,int mby,Macroblock_s *MbP,int edge,int mvlimit,bool bMixedEdge);
int  GetStrength_h_P PARGS9(byte *Strength,Macroblock_s *MbQ,int MbQAddr,int mbx,int mby,Macroblock_s *MbP,int edge,int mvlimit,StorablePicture *p);
int  GetStrength_h PARGS9(byte *Strength,Macroblock_s *MbQ,int MbQAddr,int mbx,int mby,Macroblock_s *MbP,int edge,int mvlimit,StorablePicture *p);
int  GetStrengthInternal_v(byte *Strength,Macroblock_s *MbQ,int edge,int mvlimit);
int  GetStrengthInternal_v_P_c(byte *Strength,Macroblock_s *MbQ,int edge,int mvlimit);
int  GetStrengthInternal_h(byte *Strength,Macroblock_s *MbQ,int edge,int mvlimit);
int  GetStrengthInternal_h_P_c(byte *Strength,Macroblock_s *MbQ,int edge,int mvlimit);
//#if   defined(H264_ENABLE_INTRINSICS)
int  GetStrengthInternal_h_P(byte *Strength,Macroblock_s *MbQ,int edge,int mvlimit);
int  GetStrengthInternal_v_P(byte *Strength,Macroblock_s *MbQ,int edge,int mvlimit);
int  GetStrengthInternal_v_3(int *StrengthSumV, byte *Strength,Macroblock_s *MbQ,int mvlimit);
int  GetStrengthInternal_v_P_3(int *StrengthSumV, byte *Strength,Macroblock_s *MbQ,int mvlimit);
int  GetStrengthInternal_h_3(int *StrengthSumH, byte *Strength,Macroblock_s *MbQ,int mvlimit);
int  GetStrengthInternal_h_P_3(int *StrengthSumH, byte *Strength,Macroblock_s *MbQ,int mvlimit);
//#endif
void EdgeLoop_luma_v(imgpel* SrcPtrQ, byte *Strength, int fieldQ, int *IndexA, int *IndexB, int dpel,bool bMixedEdge);
void EdgeLoop_luma_h(imgpel* SrcPtrQ, byte *Strength, int *IndexA, int *IndexB, int inc);
void EdgeLoop_chroma_v(imgpel* SrcPtrQ, byte *Strength, int fieldQ, int *IndexA, int *IndexB, int dpel,bool bMixedEdge);
void EdgeLoop_chroma_h(imgpel* SrcPtrQ, byte *Strength, int *IndexA, int *IndexB, int inc);
//deblock U and V together
void EdgeLoop_chromaUV_v(imgpel* SrcUV, byte *Strength, int fieldQ, int *IndexA, int *IndexB, int dpel, bool bMixedEdge);
void EdgeLoop_chromaUV_h(imgpel* SrcUV, byte *Strength, int *IndexA, int *IndexB, int inc);

typedef void Deblock_luma_h_t (imgpel* SrcPtrQ, int inc, int Alpha, int Beta, int C0, int pNum);
extern Deblock_luma_h_t Deblock_luma_h_0;			//without deblocking
extern Deblock_luma_h_t Deblock_luma_h_1_c;			//boundary strength = 1, 2, 3
extern Deblock_luma_h_t Deblock_luma_h_1_mmx;
extern Deblock_luma_h_t Deblock_luma_h_4;			//boundary strength = 4
extern Deblock_luma_h_t Deblock_luma_h_p8_1_sse;
extern Deblock_luma_h_t Deblock_luma_h_p8_1_sse2;

typedef void Deblock_luma_v_t (imgpel* SrcPtrQ, int dpel, int Alpha, int Beta, int C0, int pNum);
extern Deblock_luma_v_t Deblock_luma_v_0;			//without deblocking
extern Deblock_luma_v_t Deblock_luma_v_1_c;			//boundary strength = 1, 2, 3
extern Deblock_luma_v_t Deblock_luma_v_1_mmx;
extern Deblock_luma_v_t Deblock_luma_v_4;			//boundary strength = 4
extern Deblock_luma_v_t Deblock_luma_v_p8_1_mmx;

typedef void Deblock_chroma_h_t (imgpel* SrcPtrQ, int inc, int Alpha, int Beta, int C0, int pNum);
extern Deblock_chroma_h_t Deblock_chroma_h_0;		//without deblocking
extern Deblock_chroma_h_t Deblock_chroma_h_1_c;		//boundary strength = 1, 2, 3
extern Deblock_chroma_h_t Deblock_chroma_h_1_mmx;
extern Deblock_chroma_h_t Deblock_chroma_h_4;		//boundary strength = 4
extern Deblock_chroma_h_t Deblock_chroma_h_p8_1_mmx;
extern Deblock_chroma_h_t Deblock_chroma_h_p8_1_sse2;

typedef void Deblock_chroma_v_t (imgpel* SrcPtrQ, int dpel, int Alpha, int Beta, int C0, int pNum);
extern Deblock_chroma_v_t Deblock_chroma_v_0;		//without deblocking
extern Deblock_chroma_v_t Deblock_chroma_v_1_c;		//boundary strength = 1, 2, 3
extern Deblock_chroma_v_t Deblock_chroma_v_1_mmx;
extern Deblock_chroma_v_t Deblock_chroma_v_1_sse2;
extern Deblock_chroma_v_t Deblock_chroma_v_4;		//boundary strength = 4
extern Deblock_chroma_v_t Deblock_chroma_v_p8_1_mmx;
extern Deblock_chroma_v_t Deblock_chroma_v_p8_1_sse2;

typedef void Deblock_chromaUV_h_t (imgpel* SrcUV, int inc, int Alpha, int Beta, int C0, int pNum);
extern Deblock_chromaUV_h_t Deblock_chromaUV_h_0;		//without deblocking
extern Deblock_chromaUV_h_t Deblock_chromaUV_h_1_c;		//boundary strength = 1, 2, 3
extern Deblock_chromaUV_h_t Deblock_chromaUV_h_1_mmx;
extern Deblock_chromaUV_h_t Deblock_chromaUV_h_4;		//boundary strength = 4
extern Deblock_chromaUV_h_t Deblock_chromaUV_h_p8_1_mmx;
extern Deblock_chromaUV_h_t Deblock_chromaUV_h_p8_1_sse2;

typedef void Deblock_chromaUV_v_t (imgpel* SrcUV, int dpel, int Alpha, int Beta, int C0, int pNum);
extern Deblock_chromaUV_v_t Deblock_chromaUV_v_0;		//without deblocking
extern Deblock_chromaUV_v_t Deblock_chromaUV_v_1_c;		//boundary strength = 1, 2, 3
extern Deblock_chromaUV_v_t Deblock_chromaUV_v_1_mmx;
extern Deblock_chromaUV_v_t Deblock_chromaUV_v_1_sse2;
extern Deblock_chromaUV_v_t Deblock_chromaUV_v_4;		//boundary strength = 4
extern Deblock_chromaUV_v_t Deblock_chromaUV_v_p8_1_mmx;
extern Deblock_chromaUV_v_t Deblock_chromaUV_v_p8_1_sse2;

extern Deblock_luma_h_t* Deblock_luma_h_fp[5];
extern Deblock_luma_v_t* Deblock_luma_v_fp[5];
extern Deblock_chroma_h_t* Deblock_chroma_h_fp[5];
extern Deblock_chroma_v_t* Deblock_chroma_v_fp[5];

extern Deblock_luma_h_t* Deblock_luma_h_p8_fp[5];
extern Deblock_luma_v_t* Deblock_luma_v_p8_fp[5];
extern Deblock_chroma_h_t* Deblock_chroma_h_p8_fp[5];
extern Deblock_chroma_h_t* Deblock_chroma_v_p8_fp[5];

extern Deblock_chromaUV_h_t* Deblock_chromaUV_h_fp[5];
extern Deblock_chromaUV_v_t* Deblock_chromaUV_v_fp[5];
extern Deblock_chromaUV_h_t* Deblock_chromaUV_h_p8_fp[5];
extern Deblock_chromaUV_h_t* Deblock_chromaUV_v_p8_fp[5];

#endif //_LOOPFILTER_H_
