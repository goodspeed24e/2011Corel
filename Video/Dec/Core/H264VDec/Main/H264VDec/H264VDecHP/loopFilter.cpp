/*!
*************************************************************************************
* \file loopFilter.c
*
* \brief
*    Filter to reduce blocking artifacts on a macroblock level.
*    The filter strength is QP dependent.
*
* \author
*    Contributors:
*    - Peter List       Peter.List@t-systems.de:  Original code                                 (13-Aug-2001)
*    - Jani Lainema     Jani.Lainema@nokia.com:   Some bug fixing, removal of recusiveness      (16-Aug-2001)
*    - Peter List       Peter.List@t-systems.de:  inplace filtering and various simplifications (10-Jan-2002)
*    - Anthony Joch     anthony@ubvideo.com:      Simplified switching between filters and 
*                                                 non-recursive default filter.                 (08-Jul-2002)
*    - Cristina Gomila  cristina.gomila@thomson.net: Simplification of the chroma deblocking
*                                                    from JVT-E089                              (21-Nov-2002)
*************************************************************************************
*/
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "global.h"
#include "image.h"
#include "mb_access.h"
#include "loopfilter.h"
#include "clipping.h"
#include "defines.h"
#ifdef H264_ENABLE_INTRINSICS
#include <mmintrin.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#endif

// Disable "no EMMS" warning
#pragma warning ( disable : 4799 )

//#define diff(x,y) (x>y?x-y:y-x)

extern byte QP_SCALE_CR[52] ;

/*********************************************************************************************************/

//#define  IClip( Min, Max, Val) (((Val)<(Min))? (Min):(((Val)>(Max))? (Max):(Val)))
#define CQPOF(qp, uv) (__fast_iclip0_X(51, qp + p->chroma_qp_offset[uv]))


// NOTE: to change the tables below for instance when the QP doubling is changed from 6 to 8 values 
//       send an e-mail to Peter.List@t-systems.com to get a little program that calculates them automatically 

byte ALPHA_TABLE[52]  = {0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,4,4,5,6,  7,8,9,10,12,13,15,17,  20,22,25,28,32,36,40,45,  50,56,63,71,80,90,101,113,  127,144,162,182,203,226,255,255} ;
byte  BETA_TABLE[52]  = {0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,2,2,2,3,  3,3,3, 4, 4, 4, 6, 6,   7, 7, 8, 8, 9, 9,10,10,  11,11,12,12,13,13, 14, 14,   15, 15, 16, 16, 17, 17, 18, 18} ;
byte CLIP_TAB[52][5]  =
{
	{ 0, 0, 0, 0, 0},{ 0, 0, 0, 0, 0},{ 0, 0, 0, 0, 0},{ 0, 0, 0, 0, 0},{ 0, 0, 0, 0, 0},{ 0, 0, 0, 0, 0},{ 0, 0, 0, 0, 0},{ 0, 0, 0, 0, 0},
	{ 0, 0, 0, 0, 0},{ 0, 0, 0, 0, 0},{ 0, 0, 0, 0, 0},{ 0, 0, 0, 0, 0},{ 0, 0, 0, 0, 0},{ 0, 0, 0, 0, 0},{ 0, 0, 0, 0, 0},{ 0, 0, 0, 0, 0},
	{ 0, 0, 0, 0, 0},{ 0, 0, 0, 1, 1},{ 0, 0, 0, 1, 1},{ 0, 0, 0, 1, 1},{ 0, 0, 0, 1, 1},{ 0, 0, 1, 1, 1},{ 0, 0, 1, 1, 1},{ 0, 1, 1, 1, 1},
	{ 0, 1, 1, 1, 1},{ 0, 1, 1, 1, 1},{ 0, 1, 1, 1, 1},{ 0, 1, 1, 2, 2},{ 0, 1, 1, 2, 2},{ 0, 1, 1, 2, 2},{ 0, 1, 1, 2, 2},{ 0, 1, 2, 3, 3},
	{ 0, 1, 2, 3, 3},{ 0, 2, 2, 3, 3},{ 0, 2, 2, 4, 4},{ 0, 2, 3, 4, 4},{ 0, 2, 3, 4, 4},{ 0, 3, 3, 5, 5},{ 0, 3, 4, 6, 6},{ 0, 3, 4, 6, 6},
	{ 0, 4, 5, 7, 7},{ 0, 4, 5, 8, 8},{ 0, 4, 6, 9, 9},{ 0, 5, 7,10,10},{ 0, 6, 8,11,11},{ 0, 6, 8,13,13},{ 0, 7,10,14,14},{ 0, 8,11,16,16},
	{ 0, 9,12,18,18},{ 0,10,13,20,20},{ 0,11,15,23,23},{ 0,13,17,25,25}
} ;

int STRENGTH4[5] = {0x00000000, 0x01010101, 0x02020202, 0x03030303, 0x04040404} ; 

// index table for vertical mixed edge
byte blkQ_0[16] = {0,0,0,0,4,4,4,4,8,8,8,8,12,12,12,12};
byte	blkP_[4][16]  =  { 
	{3,3,3,3,3,3,3,3,7,7,7,7,7,7,7,7}, 
	{11,11,11,11,11,11,11,11,15,15,15,15,15,15,15,15}, 
	{3,3,7,7,11,11,15,15,3,3,7,7,11,11,15,15}, 
	{3,3,7,7,11,11,15,15,3,3,7,7,11,11,15,15} };
byte	lp_mb_offset[3][16] = { 
	{0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,}, 
	{0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,}, 
	{0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,} };

//int (*fp_GetStrength_v)(byte *Strength,Macroblock_s *MbQ,int MbQAddr,int mbx,int mby,Macroblock_s *MbP,int edge,int mvlimit,bool bMixedEdge);
//int (*fp_GetStrength_h) PARGS9(byte *Strength,Macroblock_s *MbQ,int MbQAddr,int mbx,int mby,Macroblock_s *MbP,int edge,int mvlimit,StorablePicture *p);
//int (*fp_GetStrengthInternal_v)(byte *Strength,Macroblock_s *MbQ,int edge,int mvlimit);
//int (*fp_GetStrengthInternal_h)(byte *Strength,Macroblock_s *MbQ,int edge,int mvlimit);



	Deblock_luma_h_t* Deblock_luma_h_fp[] = {Deblock_luma_h_0, Deblock_luma_h_1_c, Deblock_luma_h_1_c, Deblock_luma_h_1_c, Deblock_luma_h_4};
	Deblock_luma_v_t* Deblock_luma_v_fp[] = {Deblock_luma_v_0, Deblock_luma_v_1_c, Deblock_luma_v_1_c, Deblock_luma_v_1_c, Deblock_luma_v_4};
	Deblock_chroma_h_t* Deblock_chroma_h_fp[] = {Deblock_chroma_h_0, Deblock_chroma_h_1_c, Deblock_chroma_h_1_c, Deblock_chroma_h_1_c, Deblock_chroma_h_4};
	Deblock_chroma_v_t* Deblock_chroma_v_fp[] = {Deblock_chroma_v_0, Deblock_chroma_v_1_c, Deblock_chroma_v_1_c, Deblock_chroma_v_1_c, Deblock_chroma_v_4};
	Deblock_luma_h_t* Deblock_luma_h_p8_fp[] = {Deblock_luma_h_0, Deblock_luma_h_1_c, Deblock_luma_h_1_c, Deblock_luma_h_1_c, Deblock_luma_h_4};
	Deblock_luma_v_t* Deblock_luma_v_p8_fp[] = {Deblock_luma_v_0, Deblock_luma_v_1_c, Deblock_luma_v_1_c, Deblock_luma_v_1_c, Deblock_luma_v_4};
	Deblock_chroma_h_t* Deblock_chroma_h_p8_fp[] = {Deblock_chroma_h_0, Deblock_chroma_h_1_c, Deblock_chroma_h_1_c, Deblock_chroma_h_1_c, Deblock_chroma_h_4};
	Deblock_chroma_v_t* Deblock_chroma_v_p8_fp[] = {Deblock_chroma_v_0, Deblock_chroma_v_1_c, Deblock_chroma_v_1_c, Deblock_chroma_v_1_c, Deblock_chroma_v_4};

	//deblock U and V together
	Deblock_chromaUV_h_t* Deblock_chromaUV_h_fp[] = {Deblock_chromaUV_h_0, Deblock_chromaUV_h_1_c, Deblock_chromaUV_h_1_c, Deblock_chromaUV_h_1_c, Deblock_chromaUV_h_4};
	Deblock_chromaUV_v_t* Deblock_chromaUV_v_fp[] = {Deblock_chromaUV_v_0, Deblock_chromaUV_v_1_c, Deblock_chromaUV_v_1_c, Deblock_chromaUV_v_1_c, Deblock_chromaUV_v_4};
	Deblock_chromaUV_h_t* Deblock_chromaUV_h_p8_fp[] = {Deblock_chromaUV_h_0, Deblock_chromaUV_h_1_c, Deblock_chromaUV_h_1_c, Deblock_chromaUV_h_1_c, Deblock_chromaUV_h_4};
	Deblock_chromaUV_v_t* Deblock_chromaUV_v_p8_fp[] = {Deblock_chromaUV_v_0, Deblock_chromaUV_v_1_c, Deblock_chromaUV_v_1_c, Deblock_chromaUV_v_1_c, Deblock_chromaUV_v_4};

#define deblocking_luma_h_parallel_by_8_or_4(SrcPtrQ, inc, Alpha, Beta, Strength)\
	{\
	if(Strength[0] == Strength[1])\
	Deblock_luma_h_p8_fp[Strength[0]](SrcPtrQ, inc, Alpha, Beta, ClipTab[Strength[0]], 8);\
	else\
	{\
	Deblock_luma_h_fp[Strength[0]](SrcPtrQ, inc, Alpha, Beta, ClipTab[Strength[0]], 4);\
	Deblock_luma_h_fp[Strength[1]](SrcPtrQ+4, inc, Alpha, Beta, ClipTab[Strength[1]], 4);\
	}\
	if(Strength[2] == Strength[3])\
	Deblock_luma_h_p8_fp[Strength[2]](SrcPtrQ+8, inc, Alpha, Beta, ClipTab[Strength[2]], 8);\
	else\
	{\
	Deblock_luma_h_fp[Strength[2]](SrcPtrQ+8, inc, Alpha, Beta, ClipTab[Strength[2]], 4);\
	Deblock_luma_h_fp[Strength[3]](SrcPtrQ+12, inc, Alpha, Beta, ClipTab[Strength[3]], 4);\
	}\
	}

#define deblocking_luma_v_parallel_by_8_or_4(SrcPtrQ, dpel, Alpha, Beta, Strength)\
	{\
	if(Strength[0] == Strength[1])\
	{\
	Deblock_luma_v_p8_fp[Strength[0]](SrcPtrQ, dpel, Alpha, Beta, ClipTab[Strength[0]], 8);\
	SrcPtrQ += (dpel<<3);\
	}\
	else\
	{\
	Deblock_luma_v_fp[Strength[0]](SrcPtrQ, dpel, Alpha, Beta, ClipTab[Strength[0]], 4);\
	SrcPtrQ += (dpel<<2);\
	Deblock_luma_v_fp[Strength[1]](SrcPtrQ, dpel, Alpha, Beta, ClipTab[Strength[1]], 4);\
	SrcPtrQ += (dpel<<2);\
	}\
	if(Strength[2] == Strength[3])\
	Deblock_luma_v_p8_fp[Strength[2]](SrcPtrQ, dpel, Alpha, Beta, ClipTab[Strength[2]], 8);\
	else\
	{\
	Deblock_luma_v_fp[Strength[2]](SrcPtrQ, dpel, Alpha, Beta, ClipTab[Strength[2]], 4);\
	SrcPtrQ += (dpel<<2);\
	Deblock_luma_v_fp[Strength[3]](SrcPtrQ, dpel, Alpha, Beta, ClipTab[Strength[3]], 4);\
	}\
	}

#define deblocking_chroma_h_parallel_by_4_or_2(SrcPtrQ, inc, Alpha, Beta, Strength)\
	{\
	if(Strength[0] == Strength[1])\
	Deblock_chroma_h_p8_fp[Strength[0]](SrcPtrQ, inc, Alpha, Beta, ClipTab[Strength[0]], 4);\
	else\
	{\
	Deblock_chroma_h_fp[Strength[0]](SrcPtrQ, inc, Alpha, Beta, ClipTab[Strength[0]], 2);\
	Deblock_chroma_h_fp[Strength[1]](SrcPtrQ+4, inc, Alpha, Beta, ClipTab[Strength[1]], 2);\
	}\
	if(Strength[2] == Strength[3])\
	Deblock_chroma_h_p8_fp[Strength[2]](SrcPtrQ+8, inc, Alpha, Beta, ClipTab[Strength[2]], 4);\
	else\
	{\
	Deblock_chroma_h_fp[Strength[2]](SrcPtrQ+8, inc, Alpha, Beta, ClipTab[Strength[2]], 2);\
	Deblock_chroma_h_fp[Strength[3]](SrcPtrQ+12, inc, Alpha, Beta, ClipTab[Strength[3]], 2);\
	}\
	}

#define deblocking_chroma_v_parallel_by_4_or_2(SrcPtrQ, dpel, Alpha, Beta, Strength)\
	{\
	if(Strength[0] == Strength[1])\
	Deblock_chroma_v_p8_fp[Strength[0]](SrcPtrQ, dpel, Alpha, Beta, ClipTab[Strength[0]], 4);\
	else\
	{\
	Deblock_chroma_v_fp[Strength[0]](SrcPtrQ, dpel, Alpha, Beta, ClipTab[Strength[0]], 2);\
	Deblock_chroma_v_fp[Strength[1]](SrcPtrQ+(dpel<<1), dpel, Alpha, Beta, ClipTab[Strength[1]], 2);\
	}\
	if(Strength[2] == Strength[3])\
	Deblock_chroma_v_p8_fp[Strength[2]](SrcPtrQ+(dpel<<2), dpel, Alpha, Beta, ClipTab[Strength[2]], 4);\
	else\
	{\
	Deblock_chroma_v_fp[Strength[2]](SrcPtrQ+(dpel<<2), dpel, Alpha, Beta, ClipTab[Strength[2]], 2);\
	Deblock_chroma_v_fp[Strength[3]](SrcPtrQ+(dpel<<2)+(dpel<<1), dpel, Alpha, Beta, ClipTab[Strength[3]], 2);\
	}\
	}

#define deblocking_chromaUV_h_parallel_by_4_or_2(SrcUV, inc, Alpha, Beta, Strength)\
	{\
	if(Strength[0] == Strength[1])\
	Deblock_chromaUV_h_p8_fp[Strength[0]](SrcUV, inc, Alpha, Beta, ClipTab[Strength[0]], 4);\
	else\
	{\
	Deblock_chromaUV_h_fp[Strength[0]](SrcUV, inc, Alpha, Beta, ClipTab[Strength[0]], 2);\
	Deblock_chromaUV_h_fp[Strength[1]](SrcUV+4, inc, Alpha, Beta, ClipTab[Strength[1]], 2);\
	}\
	if(Strength[2] == Strength[3])\
	Deblock_chromaUV_h_p8_fp[Strength[2]](SrcUV+8, inc, Alpha, Beta, ClipTab[Strength[2]], 4);\
	else\
	{\
	Deblock_chromaUV_h_fp[Strength[2]](SrcUV+8, inc, Alpha, Beta, ClipTab[Strength[2]], 2);\
	Deblock_chromaUV_h_fp[Strength[3]](SrcUV+12, inc, Alpha, Beta, ClipTab[Strength[3]], 2);\
	}\
	}

#define deblocking_chromaUV_v_parallel_by_4_or_2(SrcUV, dpel, Alpha, Beta, Strength)\
	{\
	if(Strength[0] == Strength[1])\
	Deblock_chromaUV_v_p8_fp[Strength[0]](SrcUV, dpel, Alpha, Beta, ClipTab[Strength[0]], 4);\
	else\
	{\
	Deblock_chromaUV_v_fp[Strength[0]](SrcUV, dpel, Alpha, Beta, ClipTab[Strength[0]], 2);\
	Deblock_chromaUV_v_fp[Strength[1]](SrcUV+(dpel<<1), dpel, Alpha, Beta, ClipTab[Strength[1]], 2);\
	}\
	if(Strength[2] == Strength[3])\
	Deblock_chromaUV_v_p8_fp[Strength[2]](SrcUV+(dpel<<2), dpel, Alpha, Beta, ClipTab[Strength[2]], 4);\
	else\
	{\
	Deblock_chromaUV_v_fp[Strength[2]](SrcUV+(dpel<<2), dpel, Alpha, Beta, ClipTab[Strength[2]], 2);\
	Deblock_chromaUV_v_fp[Strength[3]](SrcUV+(dpel<<2)+(dpel<<1), dpel, Alpha, Beta, ClipTab[Strength[3]], 2);\
	}\
	}
/*!
*****************************************************************************************
* \brief
*    Filter all macroblocks in order of increasing macroblock address.
*****************************************************************************************
*/
void DeblockSlice PARGS3(StorablePicture *p, int start_mb, int num_mb)
{
	int i, mbx, mby;
	Macroblock_s  *MbQ, *MbTop, *MbLeft;	
	int b;


	// return, if filter is disabled
	if (IMGPAR currentSlice->LFDisableIdc==1)
		return;
	if(IMGPAR type == B_SLICE)
	{
		fp_GetStrength_v		   = GetStrength_v;
		fp_GetStrengthInternal_v = GetStrengthInternal_v;
		fp_GetStrength_h		   = GetStrength_h;
		fp_GetStrengthInternal_h = GetStrengthInternal_h;
//#if  defined(H264_ENABLE_INTRINSICS)
		if((cpu_type == CPU_LEVEL_SSE2) || (cpu_type == CPU_LEVEL_SSE3))
		{
			fp_GetStrengthInternal_v_3 = GetStrengthInternal_v_3;
			fp_GetStrengthInternal_h_3 = GetStrengthInternal_h_3;
		}
//#endif        
	}
	else
	{
		fp_GetStrength_v		   = GetStrength_v_P;
		fp_GetStrength_h		   = GetStrength_h_P;
		if((cpu_type == CPU_LEVEL_SSE2) || (cpu_type == CPU_LEVEL_SSE3))
		{
			fp_GetStrengthInternal_v = GetStrengthInternal_v_P;
			fp_GetStrengthInternal_h = GetStrengthInternal_h_P;
			fp_GetStrengthInternal_v_3 = GetStrengthInternal_v_P_3;
			fp_GetStrengthInternal_h_3 = GetStrengthInternal_h_P_3;
		}
		else
		{
			fp_GetStrengthInternal_v = GetStrengthInternal_v_P_c;
			fp_GetStrengthInternal_h = GetStrengthInternal_h_P_c;
		}
//#if  defined(H264_ENABLE_INTRINSICS)
		
//#endif
	}

	MbQ  = &(p->mb_data[start_mb]);


	if (dec_picture->MbaffFrameFlag)
	{	
		MbLeft = MbQ-2;
		MbTop  = MbQ-(p->PicWidthInMbs<<1);
		mby = ((start_mb>>1)/p->PicWidthInMbs)<<1;
		mbx = (start_mb>>1)%p->PicWidthInMbs;
		for(i=start_mb;i<start_mb+num_mb;i+=2)
		{
			DeblockMb ARGS7( p,   i, mbx,   mby, MbLeft, MbTop, MbQ++ );
			DeblockMb ARGS7( p, i+1, mbx, mby+1, MbLeft, MbTop, MbQ++ );			
			MbLeft+=2;
			MbTop +=2;
			b = (mbx + 1 - p->PicWidthInMbs)>>31; // if 0, you need to change line, if -1 just move to next pair of MBs
			mbx = (mbx-b)&b;
			mby = mby+2*b+2;
		}
	}
	else
	{
		MbLeft = MbQ-1;
		MbTop  = MbQ-p->PicWidthInMbs;

		mby = start_mb/p->PicWidthInMbs;
		mbx = start_mb%p->PicWidthInMbs;
		for(i=start_mb;i<start_mb+num_mb;i++)
		{
			DeblockMb ARGS7( p, i, mbx, mby, MbLeft, MbTop, MbQ++ );			
			MbLeft++;
			MbTop++;
			b = (mbx + 1 - p->PicWidthInMbs)>>31; // if 0, you need to change line, if -1 just move to next pair of MBs
			mbx = (mbx-b)&b;
			mby = mby+b+1;
		}
	}
}


/*!
*****************************************************************************************
* \brief
*    Deblocking filter for one macroblock.
*****************************************************************************************
*/

void DeblockMb PARGS7(StorablePicture *p, int MbQAddr, int mbx, int mby, Macroblock_s *Mb_left, Macroblock_s *Mb_top, Macroblock_s  *MbQ)
{
	const int	PelDeg=4;
	bool		mixedModeEdgeFlagV=0;
	bool		uvFlag = (p->chroma_qp_offset[0] == p->chroma_qp_offset[1]);	//check if the qp_offset of U and V are the same
	int         mvlimit, bExtraEdge=0;
	int			LEAoffset, LEBoffset;
	int         filterLeftMbEdgeFlag, filterTopMbEdgeFlag;
	int			mbx_pos, mby_pos, inc_y, inc_y4, inc_ch;
	int			stride=p->Y_stride, strideUV=p->UV_stride;
	int			fieldQ, QpQ;
	int			QP, IndexA[3], IndexB[3], IndexA4[6], IndexB4[6];
	int         StrengthSumV[4], StrengthSumH[5];
	byte        StrengthV[4][16],StrengthH[5][4];

	Macroblock_s  *MbPV=0,*MbPH=0;
	imgpel *SrcYQ, *SrcUVQ=0;

	//MbQ  = &(p->mb_data[MbQAddr]) ; // current Mb
	//assert(MbQ->mb_type != I8MB || MbQ->luma_transform_size_8x8_flag);	

	QpQ = MbQ->qp;
	fieldQ = MbQ->mb_field;
	LEAoffset = IMGPAR currentSlice->LFAlphaC0Offset;
	LEBoffset = IMGPAR currentSlice->LFBetaOffset;
	mvlimit = 4 >> (int) ((p->structure!=FRAME) || (p->MbaffFrameFlag && MbQ->mb_field)); // 2 if field mode

	filterLeftMbEdgeFlag  = (mbx != 0);
	filterTopMbEdgeFlag   = (mby != 0) && !(p->MbaffFrameFlag && mby==1 && MbQ->mb_field);

	if (IMGPAR currentSlice->LFDisableIdc==2)
	{
		filterLeftMbEdgeFlag = MbQ->filterLeftMbEdgeFlag;
		filterTopMbEdgeFlag  = MbQ->filterTopMbEdgeFlag;
	}

	StrengthSumV[0] = StrengthSumH[0] = StrengthSumH[4] = 0;
	// Start getting strength
	// Vertical external strength
	if(filterLeftMbEdgeFlag)
	{
		MbPV = Mb_left;	// temporary one
		if(fieldQ != MbPV->mb_field)
		{
			// mixed edge
			MbPV=Mb_left;
			mixedModeEdgeFlagV=1;
		}
		else
		{
			MbPV=Mb_left+(p->MbaffFrameFlag&&(MbQAddr&1));
		}
		//MbPV = &(p->mb_data[mbpaV]);	// temporary one		
		StrengthSumV[0] = fp_GetStrength_v(StrengthV[0],MbQ,MbQAddr,mbx,mby,MbPV, 0, mvlimit, mixedModeEdgeFlagV); // Strength for 4 blks in 1 stripe
	}

	// Vertical internal strength
	if(MbQ->luma_transform_size_8x8_flag)
	{
		StrengthSumV[2] = fp_GetStrengthInternal_v(StrengthV[2],MbQ,2, mvlimit); // Strength for 4 blks in 1 stripe
	}
	else
	{
//#if   defined(H264_ENABLE_INTRINSICS)
		if((cpu_type == CPU_LEVEL_SSE2) || (cpu_type == CPU_LEVEL_SSE3))
			fp_GetStrengthInternal_v_3(StrengthSumV, StrengthV[0], MbQ, mvlimit); // Strength for 4 blks in 1 stripe
		else
		{
			StrengthSumV[1] = fp_GetStrengthInternal_v(StrengthV[1],MbQ,1, mvlimit); // Strength for 4 blks in 1 stripe
			StrengthSumV[2] = fp_GetStrengthInternal_v(StrengthV[2],MbQ,2, mvlimit); // Strength for 4 blks in 1 stripe
			StrengthSumV[3] = fp_GetStrengthInternal_v(StrengthV[3],MbQ,3, mvlimit); // Strength for 4 blks in 1 stripe
		}
//#endif
	}

	// Horizontal external strength
	if(filterTopMbEdgeFlag)
	{
		MbPH = Mb_top;	// temporary one
		if(!p->MbaffFrameFlag)
		{
			MbPH = Mb_top;
		}
		else if(fieldQ)
		{
			MbPH = Mb_top + ((MbQAddr&1) || (!MbPH->mb_field));
		}
		else if(MbQAddr&1)
		{
			MbPH = MbQ-1;
		}
		else
		{
			bExtraEdge = MbPH->mb_field;
			MbPH = Mb_top + !bExtraEdge;
		}
		//MbPH = &(p->mb_data[mbpaH]);	// temporary one
		StrengthSumH[0] = fp_GetStrength_h ARGS9(StrengthH[0],MbQ,MbQAddr,mbx,mby,MbPH, 0, mvlimit, p); // Strength for 4 blks in 1 stripe
		if (bExtraEdge)
		{
			// this is the extra horizontal edge between a frame macroblock pair and a field above it
			StrengthSumH[4] = fp_GetStrength_h ARGS9(StrengthH[4],MbQ,MbQAddr,mbx,mby,MbPH+1, 4, mvlimit, p); // Strength for 4 blks in 1 stripe
		}
	}

	// Horizontal internal strength
	if(MbQ->luma_transform_size_8x8_flag)
	{
		StrengthSumH[2] = fp_GetStrengthInternal_h(StrengthH[2],MbQ,2, mvlimit); // Strength for 4 blks in 1 stripe
	}
	else
	{
//#if   defined(H264_ENABLE_INTRINSICS)
		if( (cpu_type == CPU_LEVEL_SSE2) || (cpu_type == CPU_LEVEL_SSE3))
			fp_GetStrengthInternal_h_3(StrengthSumH, StrengthH[0], MbQ, mvlimit); // Strength for 4 blks in 1 stripe
		else
		{
			StrengthSumH[1] = fp_GetStrengthInternal_h(StrengthH[1],MbQ,1, mvlimit); // Strength for 4 blks in 1 stripe
			StrengthSumH[2] = fp_GetStrengthInternal_h(StrengthH[2],MbQ,2, mvlimit); // Strength for 4 blks in 1 stripe
			StrengthSumH[3] = fp_GetStrengthInternal_h(StrengthH[3],MbQ,3, mvlimit); // Strength for 4 blks in 1 stripe
		}
//#endif
	}//end horizontal edge

	// Start Deblocking
	// get internal index first
#if 1
	IndexA[0] = __fast_iclip0_X(51, QpQ + LEAoffset);	
	IndexB[0] = __fast_iclip0_X(51, QpQ + LEBoffset);
	QP = QP_SCALE_CR[CQPOF(QpQ,0)];			// we do not handle UV offset, not in High profile
	IndexA[1] = __fast_iclip0_X(51, QP + LEAoffset);	
	IndexB[1] = __fast_iclip0_X(51, QP + LEBoffset);
	if(!uvFlag)
	{
		QP = QP_SCALE_CR[CQPOF(QpQ,1)];			// we do not handle UV offset, not in High profile
		IndexA[2] = __fast_iclip0_X(51, QP + LEAoffset);	
		IndexB[2] = __fast_iclip0_X(51, QP + LEBoffset);
	}
#else
	IndexA[0] = QP_LEA[QpQ];	
	IndexB[0] = QP_LEB[QpQ];
	QP = QP_SCALE_CR[CQPOF(MbQ->qp,0)];			// we do not handle UV offset, not in High profile
	IndexA[1] = QP_LEA[QP];	
	IndexB[1] = QP_LEB[QP];
	QP = QP_SCALE_CR[CQPOF(MbQ->qp,1)];			// we do not handle UV offset, not in High profile
	IndexA[2] = QP_LEA[QP];	
	IndexB[2] = QP_LEB[QP];
#endif
	// get source position
	if (dec_picture->MbaffFrameFlag && fieldQ && (mby&1))
	{
		mbx_pos = mbx << PelDeg;
		mby_pos = ((mby&~1) << PelDeg);
		SrcYQ = p->imgY+(mby_pos+1)*stride+mbx_pos;
		if(p->chroma_format_idc)
		{
			mbx_pos >>= 1;
			mby_pos >>= 1;
			SrcUVQ = p->imgUV +(mby_pos+1)*strideUV+(mbx_pos<<1);			
		}
	}
	else
	{
		mbx_pos = mbx << PelDeg;
		mby_pos = mby << PelDeg;
		SrcYQ = p->imgY+mby_pos*stride+mbx_pos;
		if(p->chroma_format_idc)
		{
			mbx_pos >>= 1;
			mby_pos >>= 1;
			SrcUVQ = p->imgUV+mby_pos*strideUV+(mbx_pos<<1);			
		}
	}
	//inc_y = stride<<(fieldQ);
	inc_y = stride<<(int) (fieldQ&&p->MbaffFrameFlag);
	inc_y4= inc_y<<2;
	inc_ch = strideUV<<(int) (fieldQ&&p->MbaffFrameFlag);

	//Vertical external deblocking
	if(StrengthSumV[0])	// only if one of the 16 Strength bytes is != 0
	{
		QP = (QpQ+MbPV->qp+1)>>1;
		IndexA4[0] = __fast_iclip0_X(51, QP + LEAoffset);	
		IndexB4[0] = __fast_iclip0_X(51, QP + LEBoffset);
		QP = (QP_SCALE_CR[CQPOF(QpQ,0)] + QP_SCALE_CR[CQPOF(MbPV->qp,0)] + 1) >> 1;
		IndexA4[2] = __fast_iclip0_X(51, QP + LEAoffset);	
		IndexB4[2] = __fast_iclip0_X(51, QP + LEBoffset);
		if(!uvFlag)
		{
			QP = (QP_SCALE_CR[CQPOF(QpQ,1)] + QP_SCALE_CR[CQPOF(MbPV->qp,1)] + 1) >> 1;
			IndexA4[4] = __fast_iclip0_X(51, QP + LEAoffset);	
			IndexB4[4] = __fast_iclip0_X(51, QP + LEBoffset);
		}
		if(mixedModeEdgeFlagV)
		{
			QP = (QpQ+(MbPV+1)->qp+1)>>1;
			IndexA4[1] = __fast_iclip0_X(51, QP + LEAoffset);	
			IndexB4[1] = __fast_iclip0_X(51, QP + LEBoffset);
			QP = (QP_SCALE_CR[CQPOF(QpQ,0)] + QP_SCALE_CR[CQPOF((MbPV+1)->qp,0)] + 1) >> 1;
			IndexA4[3] = __fast_iclip0_X(51, QP + LEAoffset);	
			IndexB4[3] = __fast_iclip0_X(51, QP + LEBoffset);
			if(!uvFlag)
			{
				QP = (QP_SCALE_CR[CQPOF(QpQ,1)] + QP_SCALE_CR[CQPOF((MbPV+1)->qp,1)] + 1) >> 1;
				IndexA4[5] = __fast_iclip0_X(51, QP + LEAoffset);	
				IndexB4[5] = __fast_iclip0_X(51, QP + LEBoffset);
			}
		}
		EdgeLoop_luma_v(SrcYQ, StrengthV[0], fieldQ, IndexA4, IndexB4, inc_y, mixedModeEdgeFlagV);
		if( SrcUVQ )  // check imgU for both UV
		{
			if(uvFlag)	//deblock U and V together
				EdgeLoop_chromaUV_v(SrcUVQ, StrengthV[0], fieldQ, IndexA4+2, IndexB4+2, inc_ch, mixedModeEdgeFlagV) ; 
			else
			{
				EdgeLoop_chroma_v(SrcUVQ, StrengthV[0], fieldQ, IndexA4+2, IndexB4+2, inc_ch, mixedModeEdgeFlagV) ;
				EdgeLoop_chroma_v(SrcUVQ+1, StrengthV[0], fieldQ, IndexA4+4, IndexB4+4, inc_ch, mixedModeEdgeFlagV) ; 				
			}
		}
	}
	//Vertical internal deblocking
	if( !MbQ->luma_transform_size_8x8_flag && StrengthSumV[1] )	// only if one of the 16 Strength bytes is != 0
	{
		EdgeLoop_luma_v(SrcYQ+4, StrengthV[1], fieldQ, IndexA, IndexB, inc_y, 0);
	}
	if( StrengthSumV[2])	// only if one of the 16 Strength bytes is != 0
	{
		EdgeLoop_luma_v(SrcYQ+8, StrengthV[2], fieldQ, IndexA, IndexB, inc_y, 0);
		if( SrcUVQ )  // check imgU for both UV
		{
			if(uvFlag)	//deblock U and V together
					EdgeLoop_chromaUV_v(SrcUVQ+8, StrengthV[2], fieldQ, IndexA+1, IndexB+1, inc_ch, 0) ; 
			else
			{				
				EdgeLoop_chroma_v(SrcUVQ+8, StrengthV[2], fieldQ, IndexA+1, IndexB+1, inc_ch, 0) ;
				EdgeLoop_chroma_v(SrcUVQ+9, StrengthV[2], fieldQ, IndexA+2, IndexB+2, inc_ch, 0) ; 				
			}
		}
	}
	if( !MbQ->luma_transform_size_8x8_flag && StrengthSumV[3] )	// only if one of the 16 Strength bytes is != 0
	{
		EdgeLoop_luma_v(SrcYQ+12, StrengthV[3], fieldQ, IndexA, IndexB, inc_y, 0);
	}//end Vertical deblocking


	//Horizontal external deblocking
	if( StrengthSumH[0])
	{
		QP = (QpQ+MbPH->qp+1)>>1;
		IndexA4[0] = __fast_iclip0_X(51, QP + LEAoffset);	
		IndexB4[0] = __fast_iclip0_X(51, QP + LEBoffset);
		QP = (QP_SCALE_CR[CQPOF(QpQ,0)] + QP_SCALE_CR[CQPOF(MbPH->qp,0)] + 1) >> 1;
		IndexA4[1] = __fast_iclip0_X(51, QP + LEAoffset);	
		IndexB4[1] = __fast_iclip0_X(51, QP + LEBoffset);
		if(!uvFlag)
		{
			QP = (QP_SCALE_CR[CQPOF(QpQ,1)] + QP_SCALE_CR[CQPOF(MbPH->qp,1)] + 1) >> 1;
			IndexA4[2] = __fast_iclip0_X(51, QP + LEAoffset);	
			IndexB4[2] = __fast_iclip0_X(51, QP + LEBoffset);
		}
		EdgeLoop_luma_h(SrcYQ, StrengthH[0], IndexA4, IndexB4, inc_y<<bExtraEdge);
		if( SrcUVQ )  // check imgU for both UV
		{
			if(uvFlag)	//deblock U and V together
				EdgeLoop_chromaUV_h(SrcUVQ, StrengthH[0], IndexA4+1, IndexB4+1, inc_ch<<bExtraEdge) ; 
			else
			{
				EdgeLoop_chroma_h(SrcUVQ, StrengthH[0], IndexA4+1, IndexB4+1, inc_ch<<bExtraEdge) ;
				EdgeLoop_chroma_h(SrcUVQ+1, StrengthH[0], IndexA4+2, IndexB4+2, inc_ch<<bExtraEdge) ; 
			}
		}
	}
	// Extra horizontal edge between a frame macroblock pair and a field above it
	if( StrengthSumH[4] )
	{
		MbPH++;
		QP = (QpQ+MbPH->qp+1)>>1;
		IndexA4[0] = __fast_iclip0_X(51, QP + LEAoffset);	
		IndexB4[0] = __fast_iclip0_X(51, QP + LEBoffset);
		QP = (QP_SCALE_CR[CQPOF(QpQ,0)] + QP_SCALE_CR[CQPOF(MbPH->qp,0)] + 1) >> 1;
		IndexA4[1] = __fast_iclip0_X(51, QP + LEAoffset);	
		IndexB4[1] = __fast_iclip0_X(51, QP + LEBoffset);
		if(!uvFlag)
		{
			QP = (QP_SCALE_CR[CQPOF(QpQ,1)] + QP_SCALE_CR[CQPOF(MbPH->qp,1)] + 1) >> 1;
			IndexA4[2] = __fast_iclip0_X(51, QP + LEAoffset);	
			IndexB4[2] = __fast_iclip0_X(51, QP + LEBoffset);
		}
		EdgeLoop_luma_h(SrcYQ+inc_y, StrengthH[4], IndexA4, IndexB4, inc_y<<bExtraEdge) ; 
		if( SrcUVQ )  // check imgU for both UV
		{
			if(uvFlag)	//deblock U and V together
				EdgeLoop_chromaUV_h(SrcUVQ+inc_ch, StrengthH[4], IndexA4+1, IndexB4+1, inc_ch<<bExtraEdge) ; 
			else
			{
				EdgeLoop_chroma_h(SrcUVQ+inc_ch, StrengthH[4], IndexA4+1, IndexB4+1, inc_ch<<bExtraEdge) ;
				EdgeLoop_chroma_h(SrcUVQ+inc_ch+1, StrengthH[4], IndexA4+2, IndexB4+2, inc_ch<<bExtraEdge) ; 
			}
		}
	}
	//Horizontal internal deblocking
	SrcYQ += inc_y4;
	if( !MbQ->luma_transform_size_8x8_flag && StrengthSumH[1] )
	{
		EdgeLoop_luma_h(SrcYQ, StrengthH[1], IndexA, IndexB, inc_y);
	}
	SrcYQ += inc_y4;
	if( StrengthSumH[2])
	{
		EdgeLoop_luma_h(SrcYQ, StrengthH[2], IndexA, IndexB, inc_y);
		if( SrcUVQ )  // check imgU for both UV
		{
			if(uvFlag)	//deblock U and V together
				EdgeLoop_chromaUV_h(SrcUVQ+(inc_ch<<2), StrengthH[2], IndexA+1, IndexB+1, inc_ch) ; 
			else
			{
				EdgeLoop_chroma_h(SrcUVQ+(inc_ch<<2), StrengthH[2], IndexA+1, IndexB+1, inc_ch) ;
				EdgeLoop_chroma_h(SrcUVQ+(inc_ch<<2)+1, StrengthH[2], IndexA+2, IndexB+2, inc_ch) ; 
			}
		}
	}
	SrcYQ += inc_y4;
	if( !MbQ->luma_transform_size_8x8_flag && StrengthSumH[3] )
	{
		EdgeLoop_luma_h(SrcYQ, StrengthH[3], IndexA, IndexB, inc_y);
	}//end horizontal deblocking
}

/*!
*********************************************************************************************
* \brief
*    returns a buffer of 16 Strength values for one stripe in a mb (for different Frame types)
*********************************************************************************************
*/
#if   defined(H264_ENABLE_INTRINSICS)
__inline
#endif
int GetStrengthInternal_v(byte *Strength,Macroblock_s *MbQ,int edge, int mvlimit)
{
	if( !IS_INTRA(MbQ) )
	{
		int		idx;
		unsigned long cbp;

		MotionVector   *mv_p0 = &MbQ->pred_info.mv[0][edge-1];
		MotionVector   *mv_q0 = &MbQ->pred_info.mv[0][edge];
		MotionVector   *mv_p1 = &MbQ->pred_info.mv[1][edge-1];
		MotionVector   *mv_q1 = &MbQ->pred_info.mv[1][edge];

		char *pRefp0 = &MbQ->pred_info.ref_pic_id[0][(edge-1)>>1];
		char *pRefq0 = &MbQ->pred_info.ref_pic_id[0][edge>>1];
		char *pRefp1 = &MbQ->pred_info.ref_pic_id[1][(edge-1)>>1];
		char *pRefq1 = &MbQ->pred_info.ref_pic_id[1][edge>>1];

		cbp = ( MbQ->cbp_blk>>edge ) | ( MbQ->cbp_blk>>(edge-1) );

		for( idx=0 ; idx<4 ; idx++ )
		{
			if( cbp & 1 )
				Strength[idx] = 2;
			else
			{   // if no coefs, but vector difference >= 1 set Strength=1 
				int ref_p0,ref_p1,ref_q0,ref_q1;
				//short l0_mv0, l0_mv1, l0_mv2, l0_mv3, l1_mv0, l1_mv1, l1_mv2, l1_mv3;

				ref_p0 = pRefp0[idx&2];
				ref_q0 = pRefq0[idx&2];
				ref_p1 = pRefp1[idx&2];
				ref_q1 = pRefq1[idx&2];

				if ( ((ref_p0==ref_q0) && (ref_p1==ref_q1)) ||
					((ref_p0==ref_q1) && (ref_p1==ref_q0)))
				{
					// L0 and L1 reference pictures of p0 are different; q0 as well
					if (ref_p0 != ref_p1)
					{
						// compare MV for the same reference picture
						if (ref_p0==ref_q0)
						{
							Strength[idx] =	
								(fast_abs_short(( mv_q0[idx*4].x - mv_p0[idx*4].x)) >= 4) |
								(fast_abs_short(( mv_q0[idx*4].y - mv_p0[idx*4].y)) >= mvlimit) |
								(fast_abs_short(( mv_q1[idx*4].x - mv_p1[idx*4].x)) >= 4) |
								(fast_abs_short(( mv_q1[idx*4].y - mv_p1[idx*4].y)) >= mvlimit);
						}
						else
						{
							Strength[idx] = 
								(fast_abs_short(( mv_q0[idx*4].x - mv_p1[idx*4].x)) >= 4) |
								(fast_abs_short(( mv_q0[idx*4].y - mv_p1[idx*4].y)) >= mvlimit) |
								(fast_abs_short(( mv_q1[idx*4].x - mv_p0[idx*4].x)) >= 4) |
								(fast_abs_short(( mv_q1[idx*4].y - mv_p0[idx*4].y)) >= mvlimit);
						}
					}
					else
					{ // L0 and L1 reference pictures of p0 are the same; q0 as well
						Strength[idx] = 
							((fast_abs_short(( mv_q0[idx*4].x - mv_p0[idx*4].x)) >= 4) |
							(fast_abs_short(( mv_q0[idx*4].y - mv_p0[idx*4].y)) >= mvlimit ) |
							(fast_abs_short(( mv_q1[idx*4].x - mv_p1[idx*4].x)) >= 4) |
							(fast_abs_short(( mv_q1[idx*4].y - mv_p1[idx*4].y)) >= mvlimit))
							&&
							((fast_abs_short(( mv_q0[idx*4].x - mv_p1[idx*4].x)) >= 4) |
							(fast_abs_short(( mv_q0[idx*4].y - mv_p1[idx*4].y)) >= mvlimit) |
							(fast_abs_short(( mv_q1[idx*4].x - mv_p0[idx*4].x)) >= 4) |
							(fast_abs_short(( mv_q1[idx*4].y - mv_p0[idx*4].y)) >= mvlimit));
					}
				}
				else
				{
					Strength[idx] = 1;
				}
			}

			cbp >>= 4;
		}
		//return Strength[0]+Strength[1]+Strength[2]+Strength[3];
		return *(int*)Strength;
	}
	else	// intra edge
	{
		*(int*)Strength = 0x03030303;
		return 3;
	}
}



byte BLK_NUM[2][4][4]  = {{{0,4,8,12},{1,5,9,13},{2,6,10,14},{3,7,11,15}},{{0,1,2,3},{4,5,6,7},{8,9,10,11},{12,13,14,15}}} ;
byte BLK_4_TO_8[16]    = {0,0,1,1,0,0,1,1,2,2,3,3,2,2,3,3} ;

int GetStrength_v(byte *Strength,Macroblock_s *MbQ,int MbQAddr,int mbx,int mby,Macroblock_s *MbP, int edge, int mvlimit, bool bMixedEdge)
{
	int		blkP, blkQ, idx;
	int		blk_x, blk_x2, blk_y, blk_y2;
	int		bIntraQ;
	unsigned long cbp;
	byte	*mb_offset;
	byte	*blkP_offset;

	bIntraQ = IS_INTRA(MbQ);

	if (!bMixedEdge)
	{
		if( !bIntraQ && !IS_INTRA(MbP) )
		{
			blk_x  = (mbx<<2) + edge;
			blk_y  = (mby<<2);
			blk_x2 = blk_x-1;
			blk_y2 = blk_y;

			blkQ = ((blk_y&3)<<2) + (blk_x&3);
			blkP = ((blk_y2&3)<<2) + (blk_x2&3);

			cbp = (MbQ->cbp_blk >> blkQ) | (MbP->cbp_blk >> blkP);

			MotionVector   *mv_p0 = &MbP->pred_info.mv[0][3];
			MotionVector   *mv_p1 = &MbP->pred_info.mv[1][3];
			MotionVector   *mv_q0 = &MbQ->pred_info.mv[0][0];
			MotionVector   *mv_q1 = &MbQ->pred_info.mv[1][0];

			char *pRefp0 = &MbP->pred_info.ref_pic_id[0][1];
			char *pRefp1 = &MbP->pred_info.ref_pic_id[1][1];
			char *pRefq0 = &MbQ->pred_info.ref_pic_id[0][0];
			char *pRefq1 = &MbQ->pred_info.ref_pic_id[1][0];

			for( idx=0 ; idx<4 ; idx++ )
			{
				if( cbp & 1 )
					Strength[idx] = 2;
				else
				{   // if no coefs, but vector difference >= 1 set Strength=1 
					int ref_p0,ref_p1,ref_q0,ref_q1;  
					//short l0_mv0, l0_mv1, l0_mv2, l0_mv3, l1_mv0, l1_mv1, l1_mv2, l1_mv3;

					ref_p0 = pRefp0[idx&2];
					ref_q0 = pRefq0[idx&2];
					ref_p1 = pRefp1[idx&2];
					ref_q1 = pRefq1[idx&2];

					if ( ((ref_p0==ref_q0) && (ref_p1==ref_q1)) ||
						((ref_p0==ref_q1) && (ref_p1==ref_q0))) 
					{
						// L0 and L1 reference pictures of p0 are different; q0 as well
						if (ref_p0 != ref_p1) 
						{
							// compare MV for the same reference picture
							if (ref_p0==ref_q0) 
							{
								Strength[idx] =	
									(fast_abs_short(( mv_q0[idx*4].x - mv_p0[idx*4].x)) >= 4) |
									(fast_abs_short(( mv_q0[idx*4].y - mv_p0[idx*4].y)) >= mvlimit) |
									(fast_abs_short(( mv_q1[idx*4].x - mv_p1[idx*4].x)) >= 4) |
									(fast_abs_short(( mv_q1[idx*4].y - mv_p1[idx*4].y)) >= mvlimit);
							}
							else 
							{
								Strength[idx] = 
									(fast_abs_short(( mv_q0[idx*4].x - mv_p1[idx*4].x)) >= 4) |
									(fast_abs_short(( mv_q0[idx*4].y - mv_p1[idx*4].y)) >= mvlimit) |
									(fast_abs_short(( mv_q1[idx*4].x - mv_p0[idx*4].x)) >= 4) |
									(fast_abs_short(( mv_q1[idx*4].y - mv_p0[idx*4].y)) >= mvlimit);
							}
						}
						else 
						{ // L0 and L1 reference pictures of p0 are the same; q0 as well
							Strength[idx] =  
								((fast_abs_short(( mv_q0[idx*4].x - mv_p0[idx*4].x)) >= 4) |
								(fast_abs_short(( mv_q0[idx*4].y - mv_p0[idx*4].y)) >= mvlimit ) |
								(fast_abs_short(( mv_q1[idx*4].x - mv_p1[idx*4].x)) >= 4) |
								(fast_abs_short(( mv_q1[idx*4].y - mv_p1[idx*4].y)) >= mvlimit))
								&&
								((fast_abs_short(( mv_q0[idx*4].x - mv_p1[idx*4].x)) >= 4) |
								(fast_abs_short(( mv_q0[idx*4].y - mv_p1[idx*4].y)) >= mvlimit) |
								(fast_abs_short(( mv_q1[idx*4].x - mv_p0[idx*4].x)) >= 4) |
								(fast_abs_short(( mv_q1[idx*4].y - mv_p0[idx*4].y)) >= mvlimit));
						}
					}
					else 
					{
						Strength[idx] = 1;        
					}
				}
				cbp >>= 4;
			}
			//return Strength[0]+Strength[1]+Strength[2]+Strength[3];
			return *(int*)Strength;
		}
		else	// intra edge
		{
			*(int*)Strength = 0x04040404; 
			return 4;
		}
	}
	else	// mixed edge
	{
		int cbpQ = MbQ->cbp_blk;
		int cbpP[2] = {MbP->cbp_blk, (MbP+1)->cbp_blk};
		int bIntraP[2] = {IS_INTRA(MbP), IS_INTRA(MbP+1)};
		int index = ((MbQ->mb_field)<<1) + (MbQAddr&1);
		mb_offset = lp_mb_offset[index>>1];
		blkP_offset = blkP_[index];

		if( !bIntraQ )
		{
			for( idx=0 ; idx<16 ; idx++ )
			{
				if( !bIntraP[mb_offset[idx]] )
				{
					blkQ = blkQ_0[idx];
					blkP = blkP_offset[idx];

					Strength[idx] = 1 << (((cbpQ >> blkQ ) | (cbpP[mb_offset[idx]] >> blkP)) & 1);
				}
				else
				{
					Strength[idx] = 4;
				}
			}
		}
		else
		{
			*(int *) &Strength[0 ] = 0x04040404;
			*(int *) &Strength[4 ] = 0x04040404;
			*(int *) &Strength[8 ] = 0x04040404;
			*(int *) &Strength[12] = 0x04040404;
		}
		return 1;
	}
}


int GetStrength_v_P(byte *Strength,Macroblock_s *MbQ,int MbQAddr,int mbx,int mby,Macroblock_s *MbP, int edge, int mvlimit, bool bMixedEdge)
{
	int		blkP, blkQ, idx;
	int		blk_x, blk_x2, blk_y, blk_y2;
	int		bIntraQ;
	byte	*mb_offset;
	byte	*blkP_offset;
	unsigned long cbp;

	bIntraQ = IS_INTRA(MbQ);

	if (!bMixedEdge)
	{
		if( !bIntraQ && !IS_INTRA(MbP))
		{
			blk_x   = (mbx<<2) + edge;
			blk_y   = (mby<<2);
			blk_x2  = blk_x-1;
			blk_y2  = blk_y;

			blkQ = ((blk_y&3)<<2) + (blk_x&3);
			blkP = ((blk_y2&3)<<2) + (blk_x2&3);

			cbp = (MbQ->cbp_blk >> blkQ) | (MbP->cbp_blk >> blkP);
			for( idx=0 ; idx<4 ; idx++ )
			{
				if( cbp & 1 )
					Strength[idx] = 2;
				else
				{   // if no coefs, but vector difference >= 1 set Strength=1 
					int ref_p0,ref_q0;
					//short l0_mv0, l0_mv1, l0_mv2, l0_mv3;

					ref_p0 = MbP->pred_info.ref_pic_id[0][(idx&2)+1];
					ref_q0 = MbQ->pred_info.ref_pic_id[0][(idx&2)];

					if (ref_p0==ref_q0)
					{
						// L0 and L1 reference pictures of p0 are the same; q0 as well
						Strength[idx] = 
							((fast_abs_short(( MbQ->pred_info.mv[0][idx*4].x - MbP->pred_info.mv[0][idx*4+3].x)) >= 4) |
							(fast_abs_short(( MbQ->pred_info.mv[0][idx*4].y - MbP->pred_info.mv[0][idx*4+3].y)) >= mvlimit )
							);
					}
					else
					{
						Strength[idx] = 1;
					}
				}
				cbp >>= 4;
			}
			//return Strength[0]+Strength[1]+Strength[2]+Strength[3];
			return *(int*)Strength;
		}
		else	// intra edge
		{
			*(int*)Strength = 0x04040404;
			return 4;
		}
	}
	else	// mixed edge
	{
		int cbpQ = MbQ->cbp_blk;
		int cbpP[2] = {MbP->cbp_blk, (MbP+1)->cbp_blk};
		int bIntraP[2] = {IS_INTRA(MbP), IS_INTRA(MbP+1)};
		int index = ((MbQ->mb_field)<<1) + (MbQAddr&1);
		mb_offset = lp_mb_offset[index>>1];
		blkP_offset = blkP_[index];

		if( !bIntraQ )
		{
			for( idx=0 ; idx<16 ; idx++ )
			{
				if( !bIntraP[mb_offset[idx]] )
				{
					blkQ = blkQ_0[idx];
					blkP = blkP_offset[idx];

					Strength[idx] = 1 << (((cbpQ >> blkQ ) | (cbpP[mb_offset[idx]] >> blkP)) & 1);
				}
				else
				{
					Strength[idx] = 4;
				}
			}
		}
		else
		{
			for( idx=0 ; idx<16 ; idx++ )
			{
				Strength[idx] = 4;
			}
		}
		return 1;
	}
}


int GetStrengthInternal_h(byte *Strength,Macroblock_s *MbQ,int edge, int mvlimit)
{
	if( !IS_INTRA(MbQ) )
	{
		int		idx;
		unsigned long cbp;

		MotionVector   *mv_p0 = &MbQ->pred_info.mv[0][(edge-1)*4];
		MotionVector   *mv_p1 = &MbQ->pred_info.mv[1][(edge-1)*4];
		MotionVector   *mv_q0 = &MbQ->pred_info.mv[0][edge*4];
		MotionVector   *mv_q1 = &MbQ->pred_info.mv[1][edge*4];

		char *pRefp0 = &MbQ->pred_info.ref_pic_id[0][(edge-1)&2];
		char *pRefp1 = &MbQ->pred_info.ref_pic_id[1][(edge-1)&2];
		char *pRefq0 = &MbQ->pred_info.ref_pic_id[0][edge&2];
		char *pRefq1 = &MbQ->pred_info.ref_pic_id[1][edge&2];

		cbp = ( MbQ->cbp_blk>>(4*edge) ) | ( MbQ->cbp_blk>>(4*(edge-1)) );

		for( idx=0 ; idx<4 ; idx++ )
		{
			if( cbp & 1 )
				Strength[idx] = 2 ;
			else
			{   // if no coefs, but vector difference >= 1 set Strength=1 
				int ref_p0,ref_p1,ref_q0,ref_q1;
				//short l0_mv0, l0_mv1, l0_mv2, l0_mv3, l1_mv0, l1_mv1, l1_mv2, l1_mv3;
				ref_p0 = pRefq0[idx>>1];
				ref_q0 = pRefp0[idx>>1];
				ref_p1 = pRefq1[idx>>1];
				ref_q1 = pRefp1[idx>>1];

				if ( ((ref_p0==ref_q0) && (ref_p1==ref_q1)) ||
					((ref_p0==ref_q1) && (ref_p1==ref_q0)))
				{
					// L0 and L1 reference pictures of p0 are different; q0 as well
					if (ref_p0 != ref_p1)
					{
						// compare MV for the same reference picture
						if (ref_p0==ref_q0)
						{
							Strength[idx] = 
								(fast_abs_short(( mv_q0[idx].x - mv_p0[idx].x)) >= 4) |
								(fast_abs_short(( mv_q0[idx].y - mv_p0[idx].y)) >= mvlimit) |
								(fast_abs_short(( mv_q1[idx].x - mv_p1[idx].x)) >= 4) |
								(fast_abs_short(( mv_q1[idx].y - mv_p1[idx].y)) >= mvlimit);
						}
						else
						{
							Strength[idx] = 
								(fast_abs_short(( mv_q0[idx].x - mv_p1[idx].x)) >= 4) |
								(fast_abs_short(( mv_q0[idx].y - mv_p1[idx].y)) >= mvlimit) |
								(fast_abs_short(( mv_q1[idx].x - mv_p0[idx].x)) >= 4) |
								(fast_abs_short(( mv_q1[idx].y - mv_p0[idx].y)) >= mvlimit);
						}
					}
					else
					{ // L0 and L1 reference pictures of p0 are the same; q0 as well

						Strength[idx] = 
							((fast_abs_short(( mv_q0[idx].x - mv_p0[idx].x)) >= 4) |
							(fast_abs_short(( mv_q0[idx].y - mv_p0[idx].y)) >= mvlimit ) |
							(fast_abs_short(( mv_q1[idx].x - mv_p1[idx].x)) >= 4) |
							(fast_abs_short(( mv_q1[idx].y - mv_p1[idx].y)) >= mvlimit))
							&&
							((fast_abs_short(( mv_q0[idx].x - mv_p1[idx].x)) >= 4) |
							(fast_abs_short(( mv_q0[idx].y - mv_p1[idx].y)) >= mvlimit) |
							(fast_abs_short(( mv_q1[idx].x - mv_p0[idx].x)) >= 4) |
							(fast_abs_short(( mv_q1[idx].y - mv_p0[idx].y)) >= mvlimit));
					}
				}
				else
				{
					Strength[idx] = 1;
				}
			}
			cbp >>= 1;
		}
		//return Strength[0]+Strength[1]+Strength[2]+Strength[3];
		return *(int*)Strength;
	}
	else	// intra edge
	{
		*(int*)Strength = 0x03030303;
		return 3;
	}
}


//#if   !defined(H264_ENABLE_INTRINSICS)
int GetStrengthInternal_v_P_c(byte *Strength,Macroblock_s *MbQ,int edge, int mvlimit)
{
	if(!IS_INTRA(MbQ))
	{
		int		idx;	
		unsigned long cbp;

		MotionVector   *mv_p0 = &MbQ->pred_info.mv[0][edge-1];
		MotionVector   *mv_q0 = &MbQ->pred_info.mv[0][edge];

		char *pRefp0 = &MbQ->pred_info.ref_pic_id[0][(edge-1)>>1];
		char *pRefq0 = &MbQ->pred_info.ref_pic_id[0][edge>>1];

		cbp = ( MbQ->cbp_blk>>edge ) | ( MbQ->cbp_blk>>(edge-1) );

		for( idx=0 ; idx<4 ; idx++ )
		{
			if( cbp & 1 )
				Strength[idx] = 2;
			else
			{                                                     // if no coefs, but vector difference >= 1 set Strength=1 
				int ref_p0,ref_q0;
				//short l0_mv0, l0_mv1, l0_mv2, l0_mv3, l1_mv0, l1_mv1, l1_mv2, l1_mv3;

				ref_p0 = pRefp0[idx&2];
				ref_q0 = pRefq0[idx&2];

				if (ref_p0==ref_q0)
				{
					// L0 and L1 reference pictures of p0 are the same; q0 as well
					Strength[idx] = 
						((fast_abs_short(( mv_q0[idx*4].x - mv_p0[idx*4].x)) >= 4) |
						(fast_abs_short(( mv_q0[idx*4].y - mv_p0[idx*4].y)) >= mvlimit )						
						);
				}
				else
				{
					Strength[idx]= 1;
				}
			}

			cbp >>= 4;
		}
		//return Strength[0]+Strength[1]+Strength[2]+Strength[3];
		return *(int*)Strength;
	}
	else	// intra edge
	{
		*(int*)Strength = 0x03030303;
		return 3;
	}
}

int GetStrengthInternal_h_P_c(byte *Strength,Macroblock_s *MbQ,int edge, int mvlimit)
{
	if( !IS_INTRA(MbQ) )
	{
		int		idx;
		unsigned long cbp;

		MotionVector   *mv_p0 = &MbQ->pred_info.mv[0][(edge-1)*4];
		MotionVector   *mv_q0 = &MbQ->pred_info.mv[0][edge*4];

		char *pRefp0 = &MbQ->pred_info.ref_pic_id[0][(edge-1)&2];
		char *pRefq0 = &MbQ->pred_info.ref_pic_id[0][edge&2];

		cbp = ( MbQ->cbp_blk>>(4*edge) ) | ( MbQ->cbp_blk>>(4*(edge-1)) );

		for( idx=0 ; idx<4 ; idx++ )
		{
			if( cbp & 1 )
				Strength[idx] = 2;
			else
			{   // if no coefs, but vector difference >= 1 set Strength=1 
				int ref_p0,ref_q0;      
				//short l0_mv0, l0_mv1, l0_mv2, l0_mv3, l1_mv0, l1_mv1, l1_mv2, l1_mv3;
				ref_p0 = pRefp0[idx>>1];
				ref_q0 = pRefq0[idx>>1];

				if (ref_p0==ref_q0)
				{
					// L0 and L1 reference pictures of p0 are the same; q0 as well
					Strength[idx] =  
						((fast_abs_short(( mv_q0[idx].x - mv_p0[idx].x)) >= 4) |
						(fast_abs_short(( mv_q0[idx].y - mv_p0[idx].y)) >= mvlimit )							
						);
				}
				else 
				{
					Strength[idx] = 1;        
				} 
			}

			cbp >>= 1;
		}
		//return Strength[0]+Strength[1]+Strength[2]+Strength[3];
		return *(int*)Strength;
	}
	else	// intra edge
	{
		*(int*)Strength = 0x03030303;
		return 3;
	}
}




//#else
__declspec(align(16)) const static unsigned long ncoeff_1[] = {0x01010101, 0x00000000};
__declspec(align(16)) const static unsigned long ncoeff_1_v[] = {0x00010001, 0x00000000};
__declspec(align(16)) const static unsigned long ncoeff_1_h[] = {0x01010101, 0x00000000};
__declspec(align(16)) const static unsigned long ncoeff_cbp_map[]= 
{0x00000000,0x00000002,0x00000200,0x00000202,
0x00020000,0x00020002,0x00020200,0x00020202,
0x02000000,0x02000002,0x02000200,0x02000202,
0x02020000,0x02020002,0x02020200,0x02020202,0x00000000
};
__declspec(align(16)) const static unsigned long ncoeff_4_and_mvlimit[5][4] = 
{{0x00000003, 0x00000003, 0x00000003, 0x00000003},//useless
{0x00000003, 0x00000003, 0x00000003, 0x00000003},//useless
{0x00010003, 0x00010003, 0x00010003, 0x00010003},
{0x00020003, 0x00020003, 0x00020003, 0x00020003},//useless
{0x00030003, 0x00030003, 0x00030003, 0x00030003},
};
int GetStrengthInternal_v_P(byte *Strength,Macroblock_s *MbQ,int edge, int mvlimit)
{
	__declspec(align(16)) unsigned long ncoeff_cbp_result[] = {0x00000000, 0x00000000};

	if( !IS_INTRA(MbQ) )
	{
		unsigned long
			cbp = (MbQ->cbp_blk | ( MbQ->cbp_blk>>1 )) & 0x2222; //edge 2
		if(cbp==0x2222)
		{		
			*(int*)Strength = 0x02020202;
			return *(int*)Strength;
		}
		////////////////////////////////////////////////////////
		__m64 mm1,  mm2, mm4,  mm5,  mm6,  mm7, mm8;
		__m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5;
		////////////////////////////////////////////////////////
		//load ref_pic_id.
		mm5 = *((__m64*)&MbQ->pred_info.ref_pic_id[0][0]);//p0
		//get p0,q0
		mm6 = _mm_srli_si64 (mm5, 8);//q0
		//compare (p0,q0)
		mm2 = _mm_cmpeq_pi8(mm5, mm6);//(p0,q0)
		mm5 = *((__m64*)&ncoeff_1_v[0]);
		mm2 = _mm_and_si64(mm2, mm5);
		mm4 = _mm_or_si64 (mm2, _mm_slli_si64 (mm2, 8));//get only (p0,q0)		
		//set strength to 1 if above is not true.		
		mm1 = *((__m64*)&ncoeff_cbp_map[(((cbp>>1)|(cbp>>4)|(cbp>>7)|(cbp>>10))&0x0F)]);//get "Strength=2"
		mm7 = mm6 = _mm_srli_si64 (mm1, 1);
		mm5 = *((__m64*)&ncoeff_1[0]);
		mm6 = _mm_xor_si64(mm6, mm5);//mask for "Strength!=2"
		mm8 = _mm_and_si64(mm4, mm6);//mask for "Strength!=2"&&(p0==q0)
		mm4 = _mm_xor_si64(mm4, mm5);//mask for "p0!=q0"

		//combine above result to "Strength=2"
		mm4 = _mm_and_si64(mm4, mm6);//if need "Strength=1"
		mm7 = _mm_or_si64(mm4, mm7);
		mm4 = _mm_or_si64(mm4, mm1);//final result of "Strength=2" or "Strength=1"
		*((__m64*)&ncoeff_cbp_result[0]) = mm7;
		////////////////////////////////////////////////////////
		if(ncoeff_cbp_result[0]!=0x01010101)
		{		
			xmm0 = *((__m128i*)&MbQ->pred_info.mv[0][0]); //0,1,2,3
			xmm1 = *((__m128i*)&MbQ->pred_info.mv[0][4]); //4,5,6,7
			xmm2 = *((__m128i*)&MbQ->pred_info.mv[0][8]); //8,9,10,11
			xmm3 = *((__m128i*)&MbQ->pred_info.mv[0][12]); //12,13,14,15

			xmm4 = _mm_unpacklo_epi32(xmm0, xmm1); //0,4,1,5
			xmm5 = _mm_unpackhi_epi32(xmm0, xmm1); //2,6,3,7
			xmm0 = _mm_unpacklo_epi32(xmm2, xmm3); //8,12,9,13
			xmm1 = _mm_unpackhi_epi32(xmm2, xmm3); //10,14,11,15

			xmm0 = _mm_unpackhi_epi64(xmm4, xmm0); //1,5,9,13
			xmm1 = _mm_unpacklo_epi64(xmm5, xmm1); //2,6,10,14

			//get ABS((q0q0q0q0)-(p0p0p0p0)).
			xmm2 = _mm_subs_epi16(xmm0, xmm1);
			xmm3 = _mm_subs_epi16(xmm1, xmm0);

			//set compare patten (4, mvlimit, 4, mvlimit).
			xmm4 = *((__m128i*)&ncoeff_4_and_mvlimit[mvlimit][0]);

			//compare great than.
			xmm2 = _mm_or_si128(_mm_cmpgt_epi16(xmm2, xmm4), _mm_cmpgt_epi16(xmm3, xmm4));

			//get strength.
			xmm3 = _mm_srli_si128 (xmm2, 2);
			xmm2 = _mm_or_si128(xmm3, xmm2);

			//get each 2-bytes strength to "&" with mask
			xmm3 = _mm_srli_si128 (xmm2, 4);
			xmm4 = _mm_unpacklo_epi8(xmm2, xmm3);
			xmm5 = _mm_unpackhi_epi8(xmm2, xmm3);
			xmm4 = _mm_slli_si128 (xmm4, 6); 
			xmm5 = _mm_slli_si128 (xmm5, 2);
			mm1 = _mm_movepi64_pi64(xmm4); //"strength" to __mm64
			mm2 = _mm_movepi64_pi64(xmm5); //"strength" to __mm64
			mm1 = _mm_srli_si64(mm1, 48);
			mm1 = _mm_or_si64(mm1, mm2);
			mm5 = _mm_and_si64(mm1, mm8); //"&" with "Strength!=2"&&(p0==q0) mask

			//put to output result
			mm4 = _mm_or_si64(mm5, mm4);
		}
		*(int*)Strength = _mm_cvtsi64_si32(mm4);
		////////////////////////////////////////////////////////
		return *(int*)Strength;
	}
	else	// intra edge
	{
		*(int*)Strength = 0x03030303;
		return 3;
	}
}
int GetStrengthInternal_v_3(int  *StrengthSumV, byte *Strength,Macroblock_s *MbQ, int mvlimit)
{
	StrengthSumV[1] = GetStrengthInternal_v(Strength+16,MbQ,1, mvlimit);
	StrengthSumV[2] = GetStrengthInternal_v(Strength+32,MbQ,2, mvlimit);
	StrengthSumV[3] = GetStrengthInternal_v(Strength+48,MbQ,3, mvlimit);
	return 0;
}

int GetStrengthInternal_v_P_3(int  *StrengthSumV, byte *Strength,Macroblock_s *MbQ, int mvlimit)
{
	__declspec(align(16)) unsigned long ncoeff_cbp_result[] = {0x00000000, 0x00000000};

	if( !IS_INTRA(MbQ) )
	{
		unsigned long
			cbp = (MbQ->cbp_blk | ( MbQ->cbp_blk>>1 ));
		unsigned long
			cbp_tmp = cbp&0x1111;
		////////////////////////////////////////////////////////
		__m64 mm1,  mm2, mm4,  mm5,  mm6,  mm7, mm8;
		__m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm6, xmm7, xmm8, xmm9;
		////////////////////////////////////////////////////////
		if(cbp_tmp==0x1111)
		{		
			StrengthSumV[1] =
				*(int*)(Strength+16) = 0x02020202;

			mm5 = *((__m64*)&ncoeff_1[0]);
			xmm6 = *((__m128i*)&MbQ->pred_info.mv[0][0]); //0,1,2,3
			xmm7 = *((__m128i*)&MbQ->pred_info.mv[0][4]); //4,5,6,7
			xmm8 = *((__m128i*)&MbQ->pred_info.mv[0][8]); //8,9,10,11
			xmm9 = *((__m128i*)&MbQ->pred_info.mv[0][12]); //12,13,14,15
			xmm4 = *((__m128i*)&ncoeff_4_and_mvlimit[mvlimit][0]);
		}
		else
		{	//Edge 1
			mm4 = *((__m64*)&ncoeff_cbp_map[(((cbp_tmp)|(cbp_tmp>>3)|(cbp_tmp>>6)|(cbp_tmp>>9))&0x0F)]);//get "Strength=2"
			mm6 = _mm_srli_si64 (mm4, 1);
			mm5 = *((__m64*)&ncoeff_1[0]);
			mm6 = _mm_xor_si64(mm6, mm5);//mask for "Strength!=2"
			////////////////////////////////////////////////////////

			xmm6 = *((__m128i*)&MbQ->pred_info.mv[0][0]); //0,1,2,3
			xmm7 = *((__m128i*)&MbQ->pred_info.mv[0][4]); //4,5,6,7
			xmm8 = *((__m128i*)&MbQ->pred_info.mv[0][8]); //8,9,10,11
			xmm9 = *((__m128i*)&MbQ->pred_info.mv[0][12]); //12,13,14,15

			xmm2 = _mm_unpacklo_epi32(xmm6, xmm7); //0,4,1,5
			xmm3 = _mm_unpacklo_epi32(xmm8, xmm9); //8,12,9,13

			xmm0 = _mm_unpacklo_epi64(xmm2, xmm3); //0,4,8,12
			xmm1 = _mm_unpackhi_epi64(xmm2, xmm3); //1,5,9,13

			//get ABS((q0q0q0q0)-(p0p0p0p0)).
			xmm2 = _mm_subs_epi16(xmm0, xmm1);
			xmm3 = _mm_subs_epi16(xmm1, xmm0);

			//set compare patten (4, mvlimit, 4, mvlimit).
			xmm4 = *((__m128i*)&ncoeff_4_and_mvlimit[mvlimit][0]);

			//compare great than.
			xmm2 = _mm_or_si128(_mm_cmpgt_epi16(xmm2, xmm4), _mm_cmpgt_epi16(xmm3, xmm4));

			//get strength.
			xmm3 = _mm_srli_si128 (xmm2, 2);
			xmm2 = _mm_or_si128(xmm3, xmm2);

			//get each 2-bytes strength to "&" with mask
			xmm3 = _mm_srli_si128 (xmm2, 4);
			xmm0 = _mm_unpacklo_epi8(xmm2, xmm3);
			xmm1 = _mm_unpackhi_epi8(xmm2, xmm3);
			xmm0 = _mm_slli_si128 (xmm0, 6); 
			xmm1 = _mm_slli_si128 (xmm1, 2);
			mm1 = _mm_movepi64_pi64(xmm0); //"strength" to __mm64
			mm2 = _mm_movepi64_pi64(xmm1); //"strength" to __mm64
			mm1 = _mm_srli_si64(mm1, 48);
			mm1 = _mm_or_si64(mm1, mm2);
			mm1 = _mm_and_si64(mm1, mm6); //"&" with "Strength!=2"&&(p0==q0) mask

			//put to output result
			mm4 = _mm_or_si64(mm1, mm4);
			StrengthSumV[1] = *(int*)(Strength+16) = _mm_cvtsi64_si32(mm4);
		}
		////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////
		cbp_tmp = cbp&0x2222;
		if(cbp_tmp==0x2222)
		{		
			StrengthSumV[2] =
				*(int*)(Strength+32) = 0x02020202;		
		}
		else
		{	//Edge 2
			//load ref_pic_id.
			mm2 = *((__m64*)&MbQ->pred_info.ref_pic_id[0][0]);//p0
			//get p0,q0
			mm1 = _mm_srli_si64 (mm2, 8);//q0
			//compare (p0,q0)
			mm2 = _mm_cmpeq_pi8(mm2, mm1);//(p0,q0)
			mm1 = *((__m64*)&ncoeff_1_v[0]);
			mm2 = _mm_and_si64(mm2, mm1);
			mm4 = _mm_or_si64 (mm2, _mm_slli_si64 (mm2, 8));//get only (p0,q0)		
			//set strength to 1 if above is not true.		
			mm1 = *((__m64*)&ncoeff_cbp_map[(((cbp_tmp>>1)|(cbp_tmp>>4)|(cbp_tmp>>7)|(cbp_tmp>>10))&0x0F)]);//get "Strength=2"
			mm7 = mm6 = _mm_srli_si64 (mm1, 1);
			mm6 = _mm_xor_si64(mm6, mm5);//mask for "Strength!=2"
			mm8 = _mm_and_si64(mm4, mm6);//mask for "Strength!=2"&&(p0==q0)
			mm4 = _mm_xor_si64(mm4, mm5);//mask for "p0!=q0"

			//combine above result to "Strength=2"
			mm4 = _mm_and_si64(mm4, mm6);//if need "Strength=1"
			mm7 = _mm_or_si64(mm4, mm7);
			mm4 = _mm_or_si64(mm4, mm1);//final result of "Strength=2" or "Strength=1"
			*((__m64*)&ncoeff_cbp_result[0]) = mm7;
			////////////////////////////////////////////////////////
			if(ncoeff_cbp_result[0]!=0x01010101)
			{		
				xmm0 = _mm_unpacklo_epi32(xmm6, xmm7); //0,4,1,5
				xmm1 = _mm_unpackhi_epi32(xmm6, xmm7); //2,6,3,7
				xmm2 = _mm_unpacklo_epi32(xmm8, xmm9); //8,12,9,13
				xmm3 = _mm_unpackhi_epi32(xmm8, xmm9); //10,14,11,15

				xmm0 = _mm_unpackhi_epi64(xmm0, xmm2); //1,5,9,13
				xmm1 = _mm_unpacklo_epi64(xmm1, xmm3); //2,6,10,14

				//get ABS((q0q0q0q0)-(p0p0p0p0)).
				xmm2 = _mm_subs_epi16(xmm0, xmm1);
				xmm3 = _mm_subs_epi16(xmm1, xmm0);

				//compare great than.
				xmm2 = _mm_or_si128(_mm_cmpgt_epi16(xmm2, xmm4), _mm_cmpgt_epi16(xmm3, xmm4));

				//get strength.
				xmm3 = _mm_srli_si128 (xmm2, 2);
				xmm2 = _mm_or_si128(xmm3, xmm2);

				//get each 2-bytes strength to "&" with mask
				xmm3 = _mm_srli_si128 (xmm2, 4);
				xmm0 = _mm_unpacklo_epi8(xmm2, xmm3);
				xmm1 = _mm_unpackhi_epi8(xmm2, xmm3);
				xmm0 = _mm_slli_si128 (xmm0, 6); 
				xmm1 = _mm_slli_si128 (xmm1, 2);
				mm1 = _mm_movepi64_pi64(xmm0); //"strength" to __mm64
				mm2 = _mm_movepi64_pi64(xmm1); //"strength" to __mm64
				mm1 = _mm_srli_si64(mm1, 48);
				mm1 = _mm_or_si64(mm1, mm2);
				mm1 = _mm_and_si64(mm1, mm8); //"&" with "Strength!=2"&&(p0==q0) mask

				//put to output result
				mm4 = _mm_or_si64(mm1, mm4);
			}
			StrengthSumV[2] = *(int*)(Strength+32) = _mm_cvtsi64_si32(mm4);
		}
		////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////
		cbp_tmp = cbp&0x4444;
		if(cbp_tmp==0x4444)
		{		

			StrengthSumV[3] =
				*(int*)(Strength+48) = 0x02020202;

			return 0;
		}
		else
		{	//Edge 3
			mm4 = *((__m64*)&ncoeff_cbp_map[(((cbp_tmp>>2)|(cbp_tmp>>5)|(cbp_tmp>>8)|(cbp_tmp>>11))&0x0F)]);//get "Strength=2"
			mm6 = _mm_srli_si64 (mm4, 1);
			mm6 = _mm_xor_si64(mm6, mm5);//mask for "Strength!=2"
			////////////////////////////////////////////////////////

			xmm2 = _mm_unpackhi_epi32(xmm6, xmm7); //2,6,3,7
			xmm3 = _mm_unpackhi_epi32(xmm8, xmm9); //10,14,11,15

			xmm0 = _mm_unpacklo_epi64(xmm2, xmm3); //2,6,10,14
			xmm1 = _mm_unpackhi_epi64(xmm2, xmm3); //3,7,11,15

			//get ABS((q0q0q0q0)-(p0p0p0p0)).
			xmm2 = _mm_subs_epi16(xmm0, xmm1);
			xmm3 = _mm_subs_epi16(xmm1, xmm0);

			//compare great than.
			xmm2 = _mm_or_si128(_mm_cmpgt_epi16(xmm2, xmm4), _mm_cmpgt_epi16(xmm3, xmm4));

			//get strength.
			xmm3 = _mm_srli_si128 (xmm2, 2);
			xmm2 = _mm_or_si128(xmm3, xmm2);

			//get each 2-bytes strength to "&" with mask
			xmm3 = _mm_srli_si128 (xmm2, 4);
			xmm0 = _mm_unpacklo_epi8(xmm2, xmm3);
			xmm1 = _mm_unpackhi_epi8(xmm2, xmm3);
			xmm0 = _mm_slli_si128 (xmm0, 6); 
			xmm1 = _mm_slli_si128 (xmm1, 2);
			mm1 = _mm_movepi64_pi64(xmm0); //"strength" to __mm64
			mm2 = _mm_movepi64_pi64(xmm1); //"strength" to __mm64
			mm1 = _mm_srli_si64(mm1, 48);
			mm1 = _mm_or_si64(mm1, mm2);
			mm1 = _mm_and_si64(mm1, mm6); //"&" with "Strength!=2"&&(p0==q0) mask

			//put to output result
			mm4 = _mm_or_si64(mm1, mm4);

			StrengthSumV[3] = *(int*)(Strength+48) = _mm_cvtsi64_si32(mm4);

			return 0;
		}
	}
	else	// intra edge
	{
		*(int*)(Strength+48) = 
			*(int*)(Strength+32) = 
			*(int*)(Strength+16) = 0x03030303;
		StrengthSumV[3] =
			StrengthSumV[2] =
			StrengthSumV[1] = 3;

		return 3;
	}
}
int GetStrengthInternal_h_P(byte *Strength,Macroblock_s *MbQ,int edge, int mvlimit)
{

	if( !IS_INTRA(MbQ) )
	{
		unsigned long		
			cbp = (( MbQ->cbp_blk>>8 ) | ( MbQ->cbp_blk>>4)) & 0x0F;
		if(cbp==0x0F)
		{		
			*(int*)Strength = 0x02020202;
			return *(int*)Strength;
		}
		////////////////////////////////////////////////////////
		__m64 mm1,  mm2, mm4,  mm5,  mm6, mm8;
		__m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5;
		////////////////////////////////////////////////////////
		//load ref_pic_id.
		mm5 = *((__m64*)&MbQ->pred_info.ref_pic_id[0][0]);
		//get p0,q0
		//mm6 = _mm_srli_si64 (mm5, (((edge-1)&2)<<3));//p0
		mm6 = _mm_srli_si64 (mm5, 16);//q0
		//compare (p0,q0)
		mm2 = _mm_cmpeq_pi8(mm5, mm6);//(p0,q0)
		mm4 = _mm_unpacklo_pi8(mm2, mm2);//get only (p0,q0) and remove (p1,q1)
		//set strength to 1 if above is not true.
		mm5 = *((__m64*)&ncoeff_1_h[0]);
		mm1 = *((__m64*)&ncoeff_cbp_map[cbp]);//get "Strength=2"
		mm6 = _mm_srli_si64 (mm1, 1);
		mm6 = _mm_xor_si64(mm6, mm5);//mask for "Strength!=2"
		//mm4 = _mm_and_si64(mm4, mm5);//mask for p0==q0
		mm8 = _mm_and_si64(mm4, mm6);//mask for "Strength!=2"&&(p0==q0)
		mm4 = _mm_xor_si64(mm4, mm5);//mask for "p0!=q0"

		//combine above result to "Strength=2"
		mm4 = _mm_and_si64(mm4, mm6);//if need "Strength=1"
		mm4 = _mm_or_si64(mm4, mm1);//final result of "Strength=2" or "Strength=1"
		//*((__m64*)&ncoeff_cbp_result[0]) = mm4;
		////////////////////////////////////////////////////////
		xmm0 = *((__m128i*)&MbQ->pred_info.mv[0][4]);//p0
		xmm1 = *((__m128i*)&MbQ->pred_info.mv[0][8]);//q0
		//set compare patten (4, mvlimit, 4, mvlimit).
		xmm4 = *((__m128i*)&ncoeff_4_and_mvlimit[mvlimit][0]);

		//get ABS((q0q0q0q0)-(p0p0p0p0)).
		xmm2 = _mm_subs_epi16(xmm0, xmm1);
		xmm3 = _mm_subs_epi16(xmm1, xmm0);

		//compare great than.
		xmm2 = _mm_or_si128(_mm_cmpgt_epi16(xmm2, xmm4), _mm_cmpgt_epi16(xmm3, xmm4));

		//get strength.
		xmm3 = _mm_srli_si128 (xmm2, 2);
		xmm2 = _mm_or_si128(xmm3, xmm2);

		//get each 2-bytes strength to "&" with mask
		xmm3 = _mm_srli_si128 (xmm2, 4);
		xmm4 = _mm_unpacklo_epi8(xmm2, xmm3);
		xmm5 = _mm_unpackhi_epi8(xmm2, xmm3);
		xmm4 = _mm_slli_si128 (xmm4, 6); 
		xmm5 = _mm_slli_si128 (xmm5, 2);
		mm1 = _mm_movepi64_pi64(xmm4); //"strength" to __mm64
		mm2 = _mm_movepi64_pi64(xmm5); //"strength" to __mm64
		mm1 = _mm_srli_si64(mm1, 48);
		mm1 = _mm_or_si64(mm1, mm2);
		mm5 = _mm_and_si64(mm1, mm8); //"&" with "Strength!=2"&&(p0==q0) mask

		//put to output result
		mm4 = _mm_or_si64(mm5, mm4);		
		*(int*)Strength = _mm_cvtsi64_si32(mm4);
		////////////////////////////////////////////////////////

		//return Strength[0]+Strength[1]+Strength[2]+Strength[3]
		return *(int*)Strength;
	}
	else	// intra edge
	{
		*(int*)Strength = 0x03030303;
		return 3;
	}
}
int GetStrengthInternal_h_3(int  *StrengthSumH, byte *Strength,Macroblock_s *MbQ, int mvlimit)
{
	StrengthSumH[1] = GetStrengthInternal_h(Strength+4,MbQ,1, mvlimit); // Strength for 4 blks in 1 stripe
	StrengthSumH[2] = GetStrengthInternal_h(Strength+8,MbQ,2, mvlimit); // Strength for 4 blks in 1 stripe
	StrengthSumH[3] = GetStrengthInternal_h(Strength+12,MbQ,3, mvlimit); // Strength for 4 blks in 1 stripe
	return 0;
}

int GetStrengthInternal_h_P_3(int  *StrengthSumH, byte *Strength,Macroblock_s *MbQ, int mvlimit)
{
	if( !IS_INTRA(MbQ) )
	{
		unsigned long
			cbp = (MbQ->cbp_blk | ( MbQ->cbp_blk>>4 ));

		////////////////////////////////////////////////////////
		__m64 mm1,  mm2, mm4,  mm5,  mm6,  mm7, mm8;
		__m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6;
		////////////////////////////////////////////////////////
		if((cbp&0xFF)==0xFF)
		{		
			//FILE *fp = fopen("c:\\str_new.txt", "at");
			//fprintf(fp, "%8x\n", *(int*)Strength);
			//fclose(fp);
			StrengthSumH[2] =
				StrengthSumH[1] =
				*(int*)(Strength+8) = 
				*(int*)(Strength+4) = 0x02020202;

			mm5 = *((__m64*)&ncoeff_1_h[0]);
			xmm0 = *((__m128i*)&MbQ->pred_info.mv[0][8]);//p0
			xmm6 = *((__m128i*)&ncoeff_4_and_mvlimit[mvlimit][0]);
		}
		else
		{
			// Edge 1 start:
			////set strength to 1 if above is not true.
			mm5 = *((__m64*)&ncoeff_1_h[0]);
			mm4 = *((__m64*)&ncoeff_cbp_map[(cbp&0x0F)]);//get "Strength=2"
			mm6 = _mm_srli_si64 (mm4, 1);
			mm6 = _mm_xor_si64(mm6, mm5);//mask for "Strength!=2"
			////////////////////////////////////////////////////////

			xmm0 = *((__m128i*)&MbQ->pred_info.mv[0][0]);//p0
			xmm1 = *((__m128i*)&MbQ->pred_info.mv[0][4]);//q0
			//set compare patten (4, mvlimit, 4, mvlimit).
			xmm6 = *((__m128i*)&ncoeff_4_and_mvlimit[mvlimit][0]);

			//get ABS((q0q0q0q0)-(p0p0p0p0)).
			xmm2 = _mm_subs_epi16(xmm0, xmm1);
			xmm3 = _mm_subs_epi16(xmm1, xmm0);

			//compare great than.
			xmm2 = _mm_or_si128(_mm_cmpgt_epi16(xmm2, xmm6), _mm_cmpgt_epi16(xmm3, xmm6));

			//get strength.
			xmm3 = _mm_srli_si128 (xmm2, 2);
			xmm2 = _mm_or_si128(xmm3, xmm2);

			//get each 2-bytes strength to "&" with mask
			xmm3 = _mm_srli_si128 (xmm2, 4);
			xmm4 = _mm_unpacklo_epi8(xmm2, xmm3);
			xmm5 = _mm_unpackhi_epi8(xmm2, xmm3);
			xmm4 = _mm_slli_si128 (xmm4, 6); 
			xmm5 = _mm_slli_si128 (xmm5, 2);
			mm1 = _mm_movepi64_pi64(xmm4); //"strength" to __mm64
			mm2 = _mm_movepi64_pi64(xmm5); //"strength" to __mm64
			mm1 = _mm_srli_si64(mm1, 48);
			mm1 = _mm_or_si64(mm1, mm2);
			mm1 = _mm_and_si64(mm1, mm6); //"&" with "Strength!=2" mask

			//put to output result
			mm4 = _mm_or_si64(mm1, mm4);		
			////////////////////////////////////////////////////////
			// Edge 2 start:
			//load ref_pic_id.
			mm6 = *((__m64*)&MbQ->pred_info.ref_pic_id[0][0]);
			//get p0,q0
			mm7 = _mm_srli_si64 (mm6, 16);//p0			
			//compare (p0,q0)
			mm7 = _mm_cmpeq_pi8(mm7, mm6);//(p0,q0)
			mm7 = _mm_unpacklo_pi8(mm7, mm7);//get only (p0,q0) and remove (p1,q1)
			//set strength to 1 if above is not true.
			mm1 = *((__m64*)&ncoeff_cbp_map[((cbp>>4)&0x0F)]);//get "Strength=2"
			mm6 = _mm_srli_si64 (mm1, 1);
			mm6 = _mm_xor_si64(mm6, mm5);//mask for "Strength!=2"			
			mm8 = _mm_and_si64(mm7, mm6);//mask for "Strength!=2"&&(p0==q0)
			mm7 = _mm_xor_si64(mm7, mm5);//mask for "p0!=q0"

			//combine above result to "Strength=2"
			mm7 = _mm_and_si64(mm7, mm6);//if need "Strength=1"
			mm7 = _mm_or_si64(mm7, mm1);//final result of "Strength=2" or "Strength=1"			
			////////////////////////////////////////////////////////

			xmm0 = *((__m128i*)&MbQ->pred_info.mv[0][8]);//q0

			//get ABS((q0q0q0q0)-(p0p0p0p0)).
			xmm2 = _mm_subs_epi16(xmm0, xmm1);
			xmm3 = _mm_subs_epi16(xmm1, xmm0);

			//compare great than.
			xmm2 = _mm_or_si128(_mm_cmpgt_epi16(xmm2, xmm6), _mm_cmpgt_epi16(xmm3, xmm6));

			//get strength.
			xmm3 = _mm_srli_si128 (xmm2, 2);
			xmm2 = _mm_or_si128(xmm3, xmm2);

			//get each 2-bytes strength to "&" with mask
			xmm3 = _mm_srli_si128 (xmm2, 4);
			xmm4 = _mm_unpacklo_epi8(xmm2, xmm3);
			xmm5 = _mm_unpackhi_epi8(xmm2, xmm3);
			xmm4 = _mm_slli_si128 (xmm4, 6); 
			xmm5 = _mm_slli_si128 (xmm5, 2);
			mm1 = _mm_movepi64_pi64(xmm4); //"strength" to __mm64
			mm2 = _mm_movepi64_pi64(xmm5); //"strength" to __mm64
			mm1 = _mm_srli_si64(mm1, 48);
			mm1 = _mm_or_si64(mm1, mm2);
			mm1 = _mm_and_si64(mm1, mm8); //"&" with "Strength!=2"&&(p0==q0) mask

			//put to output result
			mm7 = _mm_or_si64(mm1, mm7);

			mm4 = _mm_unpacklo_pi32 (mm4, mm7);
			*((__m64*)&StrengthSumH[1]) = *((__m64*)(Strength+4)) = mm4;
		}

		if((cbp&0x0F00)==0x0F00)
		{		
			StrengthSumH[3] = *(int*)(Strength+12) = 0x02020202;
			return 0;
		}
		else
		{
			////////////////////////////////////////////////////////
			// Edge 3 start:			
			////set strength to 1 if above is not true.
			mm4 = *((__m64*)&ncoeff_cbp_map[((cbp>>8)&0x0F)]);//get "Strength=2"
			mm6 = _mm_srli_si64 (mm4, 1);
			mm6 = _mm_xor_si64(mm6, mm5);//mask for "Strength!=2"
			////////////////////////////////////////////////////////

			xmm1 = *((__m128i*)&MbQ->pred_info.mv[0][12]);//q0

			//get ABS((q0q0q0q0)-(p0p0p0p0)).
			xmm2 = _mm_subs_epi16(xmm0, xmm1);
			xmm3 = _mm_subs_epi16(xmm1, xmm0);

			//compare great than.
			xmm2 = _mm_or_si128(_mm_cmpgt_epi16(xmm2, xmm6), _mm_cmpgt_epi16(xmm3, xmm6));

			//get strength.
			xmm3 = _mm_srli_si128 (xmm2, 2);
			xmm2 = _mm_or_si128(xmm3, xmm2);

			//get each 2-bytes strength to "&" with mask
			xmm3 = _mm_srli_si128 (xmm2, 4);
			xmm4 = _mm_unpacklo_epi8(xmm2, xmm3);
			xmm5 = _mm_unpackhi_epi8(xmm2, xmm3);
			xmm4 = _mm_slli_si128 (xmm4, 6); 
			xmm5 = _mm_slli_si128 (xmm5, 2);
			mm1 = _mm_movepi64_pi64(xmm4); //"strength" to __mm64
			mm2 = _mm_movepi64_pi64(xmm5); //"strength" to __mm64
			mm1 = _mm_srli_si64(mm1, 48);
			mm1 = _mm_or_si64(mm1, mm2);
			mm1 = _mm_and_si64(mm1, mm6); //"&" with "Strength!=2" mask

			//put to output result
			mm4 = _mm_or_si64(mm1, mm4);		
			StrengthSumH[3] = *(int*)(Strength+12) = _mm_cvtsi64_si32(mm4);
			////////////////////////////////////////////////////////
			return 0; //*(int*)Strength;
		}

	}
	else	// intra edge
	{
		*(int*)(Strength+12) = 
			*(int*)(Strength+8) = 
			*(int*)(Strength+4) = 0x03030303;
		StrengthSumH[3] =
			StrengthSumH[2] =
			StrengthSumH[1] = 3;

		return 3;
	}

}
//#endif
int GetStrength_h PARGS9(byte *Strength,Macroblock_s *MbQ,int MbQAddr,int mbx,int mby,Macroblock_s *MbP, int edge, int mvlimit, StorablePicture *p)
{
	int		blkP, blkQ, idx;
	int		blk_x, blk_x2, blk_y, blk_y2;
	BOOL	bIntraQ;
	unsigned long cbp;

	MotionVector   *mv_q0 = &MbQ->pred_info.mv[0][0];
	MotionVector   *mv_q1 = &MbQ->pred_info.mv[1][0];
	MotionVector   *mv_p0 = &MbP->pred_info.mv[0][12];
	MotionVector   *mv_p1 = &MbP->pred_info.mv[1][12];

	char *pRefp0 = &MbP->pred_info.ref_pic_id[0][2];
	char *pRefp1 = &MbP->pred_info.ref_pic_id[1][2];
	char *pRefq0 = &MbQ->pred_info.ref_pic_id[0][0];
	char *pRefq1 = &MbQ->pred_info.ref_pic_id[1][0];

	blk_x = (mbx<<2);
	blk_y = (mby<<2) + (edge&3);
	blk_x2 = blk_x;
	blk_y2 = blk_y-1;

	if (dec_picture->MbaffFrameFlag)
	{
		// when edge==0 & dir==1, MbP varies with different mode and position 
		blk_y2 = blk_y - 5 - ((MbQAddr&1)<<2);
		if(MbQ->mb_field)
		{
			if( (MbQAddr&1) || (!MbP->mb_field) ) 	// bottom field or frame(P)
			{
				blk_y2 += 4;
			}
		}
		else
		{
			if(MbQAddr&1)	
			{
				// bottom
				blk_y2 += 8;
			}
			else if(edge==4 || !MbP->mb_field)	// extra horizontal edge or frame(P)
			{
				// top
				blk_y2 += 4;
			}
		}
	}
	blkQ = ((blk_y&3)<<2) + (blk_x&3);
	blkP = ((blk_y2&3)<<2) + (blk_x2&3);
	bIntraQ = IS_INTRA(MbQ);

	if(MbQ->mb_field == MbP->mb_field)	// not mixed edge
	{
		if( !bIntraQ && !IS_INTRA(MbP) )
		{
			cbp = (MbQ->cbp_blk >> blkQ) | (MbP->cbp_blk >> blkP);
			for( idx=0 ; idx<4 ; idx++ )
			{
				if( cbp & 1 )
					Strength[idx] = 2 ;
				else
				{   // if no coefs, but vector difference >= 1 set Strength=1 
					int ref_p0,ref_p1,ref_q0,ref_q1;      
					//short l0_mv0, l0_mv1, l0_mv2, l0_mv3, l1_mv0, l1_mv1, l1_mv2, l1_mv3;

					ref_p0 = pRefp0[idx>>1];
					ref_q0 = pRefq0[idx>>1];
					ref_p1 = pRefp1[idx>>1];
					ref_q1 = pRefq1[idx>>1];

					if ( ((ref_p0==ref_q0) && (ref_p1==ref_q1)) ||
						((ref_p0==ref_q1) && (ref_p1==ref_q0))) 
					{
						// L0 and L1 reference pictures of p0 are different; q0 as well
						if (ref_p0 != ref_p1) 
						{ 
							// compare MV for the same reference picture
							if (ref_p0==ref_q0) 
							{
								Strength[idx] =  
									(fast_abs_short(( mv_q0[idx].x - mv_p0[idx].x)) >= 4) |
									(fast_abs_short(( mv_q0[idx].y - mv_p0[idx].y)) >= mvlimit) |
									(fast_abs_short(( mv_q1[idx].x - mv_p1[idx].x)) >= 4) |
									(fast_abs_short(( mv_q1[idx].y - mv_p1[idx].y)) >= mvlimit);
							}
							else
							{
								Strength[idx] =  
									(fast_abs_short(( mv_q0[idx].x - mv_p1[idx].x)) >= 4) |
									(fast_abs_short(( mv_q0[idx].y - mv_p1[idx].y)) >= mvlimit) |
									(fast_abs_short(( mv_q1[idx].x - mv_p0[idx].x)) >= 4) |
									(fast_abs_short(( mv_q1[idx].y - mv_p0[idx].y)) >= mvlimit);
							} 
						}
						else 
						{ // L0 and L1 reference pictures of p0 are the same; q0 as well
							Strength[idx] =  
								((fast_abs_short(( mv_q0[idx].x - mv_p0[idx].x)) >= 4) |
								(fast_abs_short(( mv_q0[idx].y - mv_p0[idx].y)) >= mvlimit ) |
								(fast_abs_short(( mv_q1[idx].x - mv_p1[idx].x)) >= 4) |
								(fast_abs_short(( mv_q1[idx].y - mv_p1[idx].y)) >= mvlimit))
								&&
								((fast_abs_short(( mv_q0[idx].x - mv_p1[idx].x)) >= 4) |
								(fast_abs_short(( mv_q0[idx].y - mv_p1[idx].y)) >= mvlimit) |
								(fast_abs_short(( mv_q1[idx].x - mv_p0[idx].x)) >= 4) |
								(fast_abs_short(( mv_q1[idx].y - mv_p0[idx].y)) >= mvlimit));
						}
					}
					else
					{
						Strength[idx] = 1;        
					} 
				}
				cbp >>= 1;
			}
			//return Strength[0]+Strength[1]+Strength[2]+Strength[3];
			return *(int*)Strength;
		}
		else	// intra edge
		{
			// Strength=3 or 4 for Mb-edge
			*(int*)Strength = STRENGTH4[3+((!p->MbaffFrameFlag && p->structure==FRAME) || (p->MbaffFrameFlag && !MbQ->mb_field))];
			return *(int*)Strength;
		}
	}
	else
	{
		if( bIntraQ || IS_INTRA(MbP) )
		{               
			*(int*)Strength = 0x03030303;
			return 3;
		}
		else
		{
			cbp = (MbQ->cbp_blk >> blkQ) | (MbP->cbp_blk >> blkP);
			for( idx=0 ; idx<4 ; idx++)
			{
				Strength[idx] = 1 << ( (cbp>>idx) & 1);
			}
			return 1;
		}
	}
}


int GetStrength_h_P PARGS9(byte *Strength,Macroblock_s *MbQ,int MbQAddr,int mbx,int mby,Macroblock_s *MbP, int edge, int mvlimit, StorablePicture *p)
{
	int		blkP, blkQ, idx;
	int		blk_x, blk_x2, blk_y, blk_y2;
	unsigned long cbp;
	BOOL	bIntraQ;

	MotionVector   *mv_p0 = &MbP->pred_info.mv[0][12];
	MotionVector   *mv_q0 = &MbQ->pred_info.mv[0][0];

	char *pRefp0 = &MbP->pred_info.ref_pic_id[0][2];
	char *pRefq0 = &MbQ->pred_info.ref_pic_id[0][0];	

	blk_x = (mbx<<2);
	blk_y = (mby<<2) + (edge&3);
	blk_x2 = blk_x;
	blk_y2 = blk_y-1;

	if (dec_picture->MbaffFrameFlag)
	{
		// when edge==0 & dir==1, MbP varies with different mode and position 
		blk_y2 = blk_y - 5 - ((MbQAddr&1)<<2);
		if(MbQ->mb_field)
		{
			if( (MbQAddr&1) || (!MbP->mb_field) ) 	// bottom field or frame(P)
			{
				blk_y2 += 4;
			}
		}
		else
		{
			if(MbQAddr&1)	
			{
				// bottom
				blk_y2 += 8;
			}
			else if(edge==4 || !MbP->mb_field)	// extra horizontal edge or frame(P)
			{
				// top
				blk_y2 += 4;
			}
		}
	}
	blkQ = ((blk_y&3)<<2) + (blk_x&3);
	blkP = ((blk_y2&3)<<2) + (blk_x2&3);
	bIntraQ = IS_INTRA(MbQ);

	if(MbQ->mb_field == MbP->mb_field)
	{
		if( !bIntraQ && !IS_INTRA(MbP) )
		{
			cbp = (MbQ->cbp_blk >> blkQ) | (MbP->cbp_blk >> blkP);
			for( idx=0 ; idx<4 ; idx++ )
			{
				if( cbp & 1 )
					Strength[idx] = 2 ;
				else
				{   // if no coefs, but vector difference >= 1 set Strength=1 
					int ref_p0,ref_q0;      
					//short l0_mv0, l0_mv1, l0_mv2, l0_mv3;

					ref_p0 = pRefp0[idx>>1];
					ref_q0 = pRefq0[idx>>1];

					if (ref_p0==ref_q0)
					{
						// L0 and L1 reference pictures of p0 are the same; q0 as well							
						Strength[idx] =  
							((fast_abs_short(( mv_q0[idx].x - mv_p0[idx].x)) >= 4) |
							(fast_abs_short(( mv_q0[idx].y - mv_p0[idx].y)) >= mvlimit )							
							);
					}
					else
					{
						Strength[idx] = 1;        
					}
				}
				cbp >>= 1;
			}
			//return Strength[0]+Strength[1]+Strength[2]+Strength[3];
			return *(int*)Strength;
		}
		else	// intra edge
		{
			// Strength=3 or 4 for Mb-edge
			// ATTENTION !!! originally was
			//(p->MbaffFrameFlag && !MbP->mb_field && !MbQ->mb_field)
			// but in this branch, MbP->mb_field == MbQ->mb_field
			*(int*)Strength = STRENGTH4[3+((!p->MbaffFrameFlag && p->structure==FRAME) || (p->MbaffFrameFlag && !MbQ->mb_field))];
			return *(int*)Strength;
		}
	}
	else
	{
		if( bIntraQ || IS_INTRA(MbP) )
		{
			*(int*)Strength = 0x03030303;
			return 3;
		}
		else
		{
			cbp = (MbQ->cbp_blk >> blkQ) | (MbP->cbp_blk >> blkP);
			for( idx=0 ; idx<4 ; idx++ )
			{
				Strength[idx] = 1 << ( (cbp>>idx) & 1);
			}
			return 1;
		}
	}
}

/*!
*****************************************************************************************
* \brief
*    Filters one edge of 16 (luma) or 8 (chroma) pel
*****************************************************************************************
*/

const int decode_chroma_scan[2][8]=
{
	{0,1,4,5,8,9,12,13},
	{0,2,4,6,8,10,12,14},
};

void EdgeLoop_chroma_h(imgpel* SrcPtrQ, byte *Strength, int *IndexA, int *IndexB, int inc)
{
	//const int PelNum=8;
	int      Alpha, Beta;
	byte*    ClipTab;   

	Alpha  =ALPHA_TABLE[*IndexA];
	Beta   =BETA_TABLE[*IndexB];
	ClipTab=CLIP_TAB[*IndexA];


	deblocking_chroma_h_parallel_by_4_or_2(SrcPtrQ, inc, Alpha, Beta, Strength);
}

void Deblock_chroma_h_0(imgpel* SrcPtrQ, int inc, int Alpha, int Beta, int C0, int pNum)
{
	//do nothing
}

void Deblock_chroma_h_1_c(imgpel* SrcPtrQ, int inc, int Alpha, int Beta, int C0, int pNum)
{
	int inc2 = (inc<<1);
	int L1, L0, R0, R1, c0, dif;

	for(int pel=0 ; pel<pNum ; pel++, SrcPtrQ+=2)
	{		
		L1  = SrcPtrQ[-inc2];
		L0  = SrcPtrQ[-inc];
		R0  = SrcPtrQ[0];
		R1  = SrcPtrQ[inc];    

		if(((abs(R0 - L0 ) < Alpha) & (abs( R0 - R1) < Beta ) & (abs(L0 - L1) < Beta )))
		{
			c0 = C0 + 1;
			dif = __fast_iclipX_X(c0, ( ((R0 - L0) << 2) + (L1 - R1) + 4) >> 3 );

			SrcPtrQ[-inc]  = CLIP0_255(L0 + dif);
			SrcPtrQ[0]  = CLIP0_255(R0 - dif); 
		}
	}
}

void Deblock_chroma_h_4(imgpel* SrcPtrQ, int inc, int Alpha, int Beta, int C0, int pNum)
{
	int L1, L0, R0, R1;
	int inc2 = (inc<<1);

	for(int pel=0 ; pel<pNum ; pel++, SrcPtrQ+=2)
	{	
		L1  = SrcPtrQ[-inc2] ;
		L0  = SrcPtrQ[-inc] ;
		R0  = SrcPtrQ[0] ;
		R1  = SrcPtrQ[ inc] ;    		

		if(((abs(R0 - L0 ) < Alpha) & (abs( R0 - R1) < Beta ) & (abs(L0 - L1) < Beta )))
		{
			SrcPtrQ[-inc] = ((L1 << 1) + L0 + R1 + 2) >> 2;                                          
			SrcPtrQ[   0] = ((R1 << 1) + R0 + L1 + 2) >> 2;			
		}
	}
}

void EdgeLoop_chromaUV_h(imgpel* SrcUV, byte *Strength, int *IndexA, int *IndexB, int inc)
{
	int      Alpha, Beta;
	byte*    ClipTab;

	Alpha  =ALPHA_TABLE[*IndexA];
	Beta   =BETA_TABLE[*IndexB];
	ClipTab=CLIP_TAB[*IndexA];

	deblocking_chromaUV_h_parallel_by_4_or_2(SrcUV, inc, Alpha, Beta, Strength);
}

void Deblock_chromaUV_h_0(imgpel* SrcUV, int inc, int Alpha, int Beta, int C0, int pNum)
{
	//do nothing
}

void Deblock_chromaUV_h_1_c(imgpel* SrcUV, int inc, int Alpha, int Beta, int C0, int pNum)
{
	int inc2 = (inc<<1);
	int L1, L0, R0, R1, c0, dif;
	imgpel* SrcPtrQ = SrcUV;

	for(int uv = 0; uv < 2; uv++)
	{
		for(int pel=0 ; pel<pNum ; pel++, SrcPtrQ+=2)
		{		
			L1  = SrcPtrQ[-inc2];
			L0  = SrcPtrQ[-inc];
			R0  = SrcPtrQ[0];
			R1  = SrcPtrQ[inc];    

			if(((abs(R0 - L0 ) < Alpha) & (abs( R0 - R1) < Beta ) & (abs(L0 - L1) < Beta )))
			{
				c0 = C0 + 1;
				dif = __fast_iclipX_X(c0, ( ((R0 - L0) << 2) + (L1 - R1) + 4) >> 3 );

				SrcPtrQ[-inc]  = CLIP0_255(L0 + dif);
				SrcPtrQ[0]  = CLIP0_255(R0 - dif); 
			}
		}
		SrcPtrQ = SrcUV+1;
	}
}

void Deblock_chromaUV_h_4(imgpel* SrcUV, int inc, int Alpha, int Beta, int C0, int pNum)
{
	int L1, L0, R0, R1;
	int inc2 = (inc<<1);
	imgpel* SrcPtrQ = SrcUV;

	for(int uv = 0; uv < 2; uv++)
	{
		for(int pel=0 ; pel<pNum ; pel++, SrcPtrQ+=2)
		{	
			L1  = SrcPtrQ[-inc2] ;
			L0  = SrcPtrQ[-inc] ;
			R0  = SrcPtrQ[0] ;
			R1  = SrcPtrQ[ inc] ;    		

			if(((abs(R0 - L0 ) < Alpha) & (abs( R0 - R1) < Beta ) & (abs(L0 - L1) < Beta )))
			{
				SrcPtrQ[-inc] = ((L1 << 1) + L0 + R1 + 2) >> 2;                                          
				SrcPtrQ[   0] = ((R1 << 1) + R0 + L1 + 2) >> 2;			
			}
		}
		SrcPtrQ = SrcUV+1;
	}
}

void EdgeLoop_chroma_v(imgpel* SrcPtrQ, byte *Strength, int fieldQ, int *IndexA, int *IndexB, int dpel, bool bMixedEdge)
{
	const int PelNum=8;
	int      pel, Strng ;
	int      C0, c0, Delta, dif, AbsDelta ;
	int      L1, L0, R0, R1, RL0;
	int      Alpha, Beta;
	byte*    ClipTab = NULL;   
	byte	 *mb_offset;
	const int *StrengthIdx;

	if(!bMixedEdge)
	{
		Alpha  =ALPHA_TABLE[*IndexA];
		Beta   =BETA_TABLE[*IndexB];
		ClipTab=CLIP_TAB[*IndexA];

#if defined(__INTEL_COMPILER) || defined(__GNUC__)
		deblocking_chroma_v_parallel_by_4_or_2(SrcPtrQ, dpel, Alpha, Beta, Strength);

#else
		for( pel=0 ; pel<PelNum ; pel++, SrcPtrQ+=dpel)
		{
			if( (Strng = Strength[(pel>>1)]) )
			{
				L1  = SrcPtrQ[-4] ;
				L0  = SrcPtrQ[-2] ;
				R0  = SrcPtrQ[ 0] ;	  
				R1  = SrcPtrQ[ 2] ;    

				AbsDelta  = abs( Delta = R0 - L0 )  ;

				if( AbsDelta < Alpha )
				{
					C0  = ClipTab[ Strng ];
					if( ((abs( R0 - R1) - Beta )  & (abs(L0 - L1) - Beta )) < 0  ) 
					{
						RL0             = L0 + R0 ;

						if(Strng == 4 )    // INTRA strong filtering
						{
							SrcPtrQ[-2] = ((L1 << 1) + L0 + R1 + 2) >> 2;                                           
							SrcPtrQ[ 0] = ((R1 << 1) + R0 + L1 + 2) >> 2; 
						}
						else                                                     // normal filtering
						{
							c0               = (C0+1);
							dif              = __fast_iclipX_X(c0, ( (Delta << 2) + (L1 - R1) + 4) >> 3 ) ;

							SrcPtrQ[-2]  = CLIP0_255(L0 + dif) ;
							SrcPtrQ[ 0]  = CLIP0_255(R0 - dif) ; 
						}
					}
				}
			}
		}
#endif		//__INTEL_COMPILER
	}
	else	// bMixedEdge is true
	{
		int		Alpha_2[2], Beta_2[2];
		byte	*ClipTab_2[2];

		Alpha_2[0]  =ALPHA_TABLE[*IndexA];
		Beta_2[0]   =BETA_TABLE[*IndexB];
		ClipTab_2[0]=CLIP_TAB[*IndexA];

		IndexA++;
		IndexB++;
		Alpha_2[1]  =ALPHA_TABLE[*IndexA];
		Beta_2[1]   =BETA_TABLE[*IndexB];
		ClipTab_2[1]=CLIP_TAB[*IndexA];

		mb_offset = lp_mb_offset[fieldQ<<1];

		StrengthIdx = decode_chroma_scan[fieldQ];	// special order if MbP is field
		for( pel=0 ; pel<PelNum ; pel++, SrcPtrQ+=dpel, mb_offset++, StrengthIdx++)
		{
			Alpha  = Alpha_2[*mb_offset];
			Beta   = Beta_2[*mb_offset];
			ClipTab= ClipTab_2[*mb_offset];

			L1  = SrcPtrQ[-4] ;
			L0  = SrcPtrQ[-2] ;
			R0  = SrcPtrQ[ 0] ;	  
			R1  = SrcPtrQ[ 2] ;    

			if( (Strng = Strength[*StrengthIdx]) )
			{
				AbsDelta  = abs( Delta = R0 - L0 )  ;

				if( AbsDelta < Alpha )
				{
					C0  = ClipTab[ Strng ];
					if( ((abs( R0 - R1) - Beta )  & (abs(L0 - L1) - Beta )) < 0  ) 
					{
						RL0             = L0 + R0 ;

						if(Strng == 4 )    // INTRA strong filtering
						{
							SrcPtrQ[-2] = ((L1 << 1) + L0 + R1 + 2) >> 2;                                           
							SrcPtrQ[ 0] = ((R1 << 1) + R0 + L1 + 2) >> 2; 
						}
						else                                                     // normal filtering
						{
							c0               = (C0+1);
							dif              = __fast_iclipX_X(c0, ( (Delta << 2) + (L1 - R1) + 4) >> 3 ) ;

							SrcPtrQ[-2]  = CLIP0_255(L0 + dif) ;
							SrcPtrQ[ 0]  = CLIP0_255(R0 - dif) ; 
						}
					} 
				}
			}
		}
	}
}

void Deblock_chroma_v_0(imgpel* SrcPtrQ, int dpel, int Alpha, int Beta, int C0, int pNum)
{
	//do nothing
}


void Deblock_chroma_v_1_c(imgpel* SrcPtrQ, int dpel, int Alpha, int Beta, int C0, int pNum)
{
	int L1, L0, R0, R1, c0, dif;

	for(int pel=0 ; pel<pNum ; pel++, SrcPtrQ+=dpel)
	{		
		L1  = SrcPtrQ[-4] ;
		L0  = SrcPtrQ[-2] ;
		R0  = SrcPtrQ[ 0] ;	  
		R1  = SrcPtrQ[ 2] ;

		if(((abs(R0 - L0 ) < Alpha) & (abs( R0 - R1) < Beta ) & (abs(L0 - L1) < Beta )))
		{		
			c0 = C0 + 1;
			dif = __fast_iclipX_X(c0, ( ((R0 - L0) << 2) + (L1 - R1) + 4) >> 3 ) ;					

			SrcPtrQ[-2]  = CLIP0_255(L0 + dif) ;
			SrcPtrQ[ 0]  = CLIP0_255(R0 - dif);
		}
	}
}

void Deblock_chroma_v_4(imgpel* SrcPtrQ, int dpel, int Alpha, int Beta, int C0, int pNum)
{
	int L1, L0, R0, R1;

	for(int pel=0 ; pel<pNum ; pel++, SrcPtrQ+=dpel)
	{		
		L1  = SrcPtrQ[-4] ;
		L0  = SrcPtrQ[-2] ;
		R0  = SrcPtrQ[ 0] ;	  
		R1  = SrcPtrQ[ 2] ;    

		if(((abs(R0 - L0 ) < Alpha) & (abs( R0 - R1) < Beta ) & (abs(L0 - L1) < Beta )))
		{		
			SrcPtrQ[-2] = ((L1 << 1) + L0 + R1 + 2) >> 2;                                           
			SrcPtrQ[ 0] = ((R1 << 1) + R0 + L1 + 2) >> 2;
		}
	}
}

void EdgeLoop_chromaUV_v(imgpel* SrcUV, byte Strength[16], int fieldQ, int *IndexA, int *IndexB, int dpel, bool bMixedEdge)
{
	const int PelNum=8;
	int      pel, Strng ;
	int      C0, c0, Delta, dif, AbsDelta ;
	int      L1, L0, R0, R1, RL0;
	int      Alpha, Beta;
	byte*    ClipTab = NULL;   
	byte	 *mb_offset;
	const int *StrengthIdx;
	imgpel* SrcPtrQ = SrcUV;

	if(!bMixedEdge)
	{
		Alpha  =ALPHA_TABLE[*IndexA];
		Beta   =BETA_TABLE[*IndexB];
		ClipTab=CLIP_TAB[*IndexA];

#if defined(__INTEL_COMPILER) || defined(__GNUC__)
		deblocking_chromaUV_v_parallel_by_4_or_2(SrcUV, dpel, Alpha, Beta, Strength);

#else
		for(int uv = 0; uv < 2; uv++)
		{
			for( pel=0 ; pel<PelNum ; pel++, SrcPtrQ+=dpel)
			{
				if( (Strng = Strength[(pel>>1)]) )
				{
					L1  = SrcPtrQ[-4] ;
					L0  = SrcPtrQ[-2] ;
					R0  = SrcPtrQ[ 0] ;	  
					R1  = SrcPtrQ[ 2] ;    

					AbsDelta  = abs( Delta = R0 - L0 )  ;
					if( AbsDelta < Alpha )
					{
						C0  = ClipTab[ Strng ];
						if( ((abs( R0 - R1) - Beta )  & (abs(L0 - L1) - Beta )) < 0  ) 
						{
							RL0 = L0 + R0 ;
							if(Strng == 4 )    // INTRA strong filtering
							{
								SrcPtrQ[-2] = ((L1 << 1) + L0 + R1 + 2) >> 2;                                           
								SrcPtrQ[ 0] = ((R1 << 1) + R0 + L1 + 2) >> 2; 
							}
							else                                                     // normal filtering
							{
								c0 = (C0+1);
								dif = __fast_iclipX_X(c0, ( (Delta << 2) + (L1 - R1) + 4) >> 3 ) ;

								SrcPtrQ[-2]  = CLIP0_255(L0 + dif) ;
								SrcPtrQ[ 0]  = CLIP0_255(R0 - dif) ; 
							}
						}
					}
				}
			}
			SrcPtrQ = SrcUV+1;
		}
#endif			//__INTEL_COMPILER
	}
	else	// bMixedEdge is true
	{
		int		Alpha_2[2], Beta_2[2];
		byte	*ClipTab_2[2];

		Alpha_2[0]  =ALPHA_TABLE[*IndexA];
		Beta_2[0]   =BETA_TABLE[*IndexB];
		ClipTab_2[0]=CLIP_TAB[*IndexA];

		Alpha_2[1]  =ALPHA_TABLE[*(IndexA+1)];
		Beta_2[1]   =BETA_TABLE[*(IndexB+1)];
		ClipTab_2[1]=CLIP_TAB[*(IndexA+1)];

		for(int uv = 0; uv < 2; uv++)
		{
			mb_offset = lp_mb_offset[fieldQ<<1];
			StrengthIdx = decode_chroma_scan[fieldQ];	// special order if MbP is field
			for( pel=0 ; pel<PelNum ; pel++, SrcPtrQ+=dpel, mb_offset++, StrengthIdx++)
			{
				Alpha  = Alpha_2[*mb_offset];
				Beta   = Beta_2[*mb_offset];
				ClipTab= ClipTab_2[*mb_offset];

				L1  = SrcPtrQ[-4] ;
				L0  = SrcPtrQ[-2] ;
				R0  = SrcPtrQ[ 0] ;	  
				R1  = SrcPtrQ[ 2] ;    
				if( (Strng = Strength[*StrengthIdx]) )
				{
					AbsDelta  = abs( Delta = R0 - L0 )  ;
					if( AbsDelta < Alpha )
					{
						C0  = ClipTab[ Strng ];
						if( ((abs( R0 - R1) - Beta )  & (abs(L0 - L1) - Beta )) < 0  ) 
						{
							RL0 = L0 + R0 ;

							if(Strng == 4 )    // INTRA strong filtering
							{
								SrcPtrQ[-2] = ((L1 << 1) + L0 + R1 + 2) >> 2;                                           
								SrcPtrQ[ 0] = ((R1 << 1) + R0 + L1 + 2) >> 2; 
							}
							else                                                     // normal filtering
							{
								c0               = (C0+1);
								dif              = __fast_iclipX_X(c0, ( (Delta << 2) + (L1 - R1) + 4) >> 3 ) ;

								SrcPtrQ[-2]  = CLIP0_255(L0 + dif) ;
								SrcPtrQ[ 0]  = CLIP0_255(R0 - dif) ; 
							}
						} 
					}
				}
			}
			SrcPtrQ = SrcUV+1;
		}
	}

}

void Deblock_chromaUV_v_0(imgpel* SrcUV, int dpel, int Alpha, int Beta, int C0, int pNum)
{
	//do nothing
}


void Deblock_chromaUV_v_1_c(imgpel* SrcUV, int dpel, int Alpha, int Beta, int C0, int pNum)
{
	int L1, L0, R0, R1, c0, dif;
	imgpel* SrcPtrQ = SrcUV;

	for(int uv = 0; uv < 2; uv++)
	{
		for(int pel=0 ; pel<pNum ; pel++, SrcPtrQ+=dpel)
		{		
			L1  = SrcPtrQ[-4] ;
			L0  = SrcPtrQ[-2] ;
			R0  = SrcPtrQ[ 0] ;	  
			R1  = SrcPtrQ[ 2] ;	
			if(((abs(R0 - L0 ) < Alpha) & (abs( R0 - R1) < Beta ) & (abs(L0 - L1) < Beta )))
			{		
				c0 = C0 + 1;
				dif = __fast_iclipX_X(c0, ( ((R0 - L0) << 2) + (L1 - R1) + 4) >> 3 ) ;					
				SrcPtrQ[-2]  = CLIP0_255(L0 + dif) ;
				SrcPtrQ[ 0]  = CLIP0_255(R0 - dif);
			}
		}
		SrcPtrQ = SrcUV+1;
	}
}

void Deblock_chromaUV_v_4(imgpel* SrcUV, int dpel, int Alpha, int Beta, int C0, int pNum)
{
	int L1, L0, R0, R1;
	imgpel* SrcPtrQ = SrcUV;

	for(int uv = 0; uv < 2; uv++)
	{
		for(int pel=0 ; pel<pNum ; pel++, SrcPtrQ+=dpel)
		{		
			L1  = SrcPtrQ[-4] ;
			L0  = SrcPtrQ[-2] ;
			R0  = SrcPtrQ[ 0] ;	  
			R1  = SrcPtrQ[ 2] ;    	
			if(((abs(R0 - L0 ) < Alpha) & (abs( R0 - R1) < Beta ) & (abs(L0 - L1) < Beta )))
			{		
				SrcPtrQ[-2] = ((L1 << 1) + L0 + R1 + 2) >> 2;                                           
				SrcPtrQ[ 0] = ((R1 << 1) + R0 + L1 + 2) >> 2;
			}
		}
		SrcPtrQ = SrcUV+1;
	}
}

void EdgeLoop_luma_h(imgpel* SrcPtrQ, byte *Strength, int *IndexA, int *IndexB, int inc)
{
	//const int PelNum=16;
	int      Alpha, Beta;
	byte*    ClipTab;   

	Alpha  =ALPHA_TABLE[*IndexA];
	Beta   =BETA_TABLE[*IndexB];
	ClipTab=CLIP_TAB[*IndexA];

	deblocking_luma_h_parallel_by_8_or_4(SrcPtrQ, inc, Alpha, Beta, Strength);
}

void Deblock_luma_h_0(imgpel* SrcPtrQ, int inc, int Alpha, int Beta, int C0, int pNum)
{
	//do nothing
}

void Deblock_luma_h_1_c(imgpel* SrcPtrQ, int inc, int Alpha, int Beta, int C0, int pNum)
{
	int inc2 = (inc<<1), inc3 = (inc<<1) + inc;
	int c0, dif, aq, ap;
	int L2, L1, L0, R0, R1, R2;

	for(int pel=0 ; pel<pNum ; pel++, SrcPtrQ++)
	{
		L2  = SrcPtrQ[-inc3];
		L1  = SrcPtrQ[-inc2];
		L0  = SrcPtrQ[-inc];
		R0  = SrcPtrQ[0];
		R1  = SrcPtrQ[ inc];
		R2  = SrcPtrQ[ inc2];

		if( (abs(R0 - L0 ) < Alpha) & (abs( R0 - R1) < Beta ) & (abs(L0 - L1) < Beta ))
		{
			aq  =	(abs( R0 - R2)	- Beta ) < 0 ;
			ap  =	(abs( L0 - L2)	- Beta ) < 0 ;

			c0 = (C0 + aq + ap) ;
			dif = __fast_iclipX_X(c0, ( ((R0 - L0 ) << 2) + (L1 - R1) + 4) >> 3 );

			SrcPtrQ[-inc] = CLIP0_255(L0 + dif) ;
			SrcPtrQ[0] = CLIP0_255(R0 - dif) ;

			if( ap )
				SrcPtrQ[-inc2] += __fast_iclipX_X(C0, ( L2 + ((L0 + R0 + 1) >> 1) - (L1<<1)) >> 1);

			if( aq )
				SrcPtrQ[ inc] += __fast_iclipX_X(C0, ( R2 + ((L0 + R0 + 1) >> 1) - (R1<<1)) >> 1 );	
		}
	}
}

void Deblock_luma_h_4(imgpel* SrcPtrQ, int inc, int Alpha, int Beta, int C0, int pNum)
{
	int aq, ap, Delta, AbsDelta, small_gap;
	int L3, L2, L1, L0, R0, R1, R2, R3, RL0;
	int inc2 = (inc<<1), inc3 = (inc<<1) + inc, inc4 = (inc<<2);

	for(int pel=0 ; pel<pNum ; pel++, SrcPtrQ++)
	{
		L0  = SrcPtrQ[-inc] ;
		R0  = SrcPtrQ[0] ;

		AbsDelta  = abs( Delta = R0 - L0 )  ;

		if( AbsDelta < Alpha )
		{
			L1  = SrcPtrQ[-inc2] ;
			R1  = SrcPtrQ[ inc] ;

			if( ((abs( R0 - R1) - Beta )  & (abs(L0 - L1) - Beta )) < 0  ) 
			{
				L2  = SrcPtrQ[-inc3] ;
				R2  = SrcPtrQ[ inc2] ;
				aq  = (abs( R0 - R2) - Beta ) < 0  ;
				ap  = (abs( L0 - L2) - Beta ) < 0  ;

				RL0             = L0 + R0 ;          
				// Luma    
				small_gap = (AbsDelta < ((Alpha >> 2) + 2));

				aq &= small_gap;
				ap &= small_gap;

				L3  = SrcPtrQ[-inc4] ;
				R3  = SrcPtrQ[ inc3] ;
				if (ap)
				{
					SrcPtrQ[-inc3] = (((L3 + L2) <<1) + L2 + L1 + RL0 + 4) >> 3;
					SrcPtrQ[-inc2] = ( L2 + L1 + L0 + R0 + 2) >> 2;
					SrcPtrQ[-inc ] = ( R1 + ((L1 + RL0) << 1) +  L2 + 4) >> 3;
				}
				else
				{
					SrcPtrQ[-inc ] = ((L1 << 1) + L0 + R1 + 2) >> 2 ;
				}    
				if (aq)
				{
					SrcPtrQ[0]      = ( L1 + ((R1 + RL0) << 1) +  R2 + 4) >> 3;
					SrcPtrQ[inc ]  = ( R2 + R0 + R1 + L0 + 2) >> 2;
					SrcPtrQ[inc2] = (((R3 + R2) <<1) + R2 + R1 + RL0 + 4) >> 3;
				}
				else
				{
					SrcPtrQ[0]      = ((R1 << 1) + R0 + L1 + 2) >> 2 ;
				}
			}
		}
	}
}

void EdgeLoop_luma_v(imgpel* SrcPtrQ, byte *Strength, int fieldQ, int *IndexA, int *IndexB, int dpel, bool bMixedEdge)
{
	const int PelNum=16;
	int      pel, ap, aq, Strng ;
	int      C0, c0, Delta, dif, AbsDelta ;
	int      L2 = 0, L1, L0, R0, R1, R2 = 0, RL0, L3, R3 ;
	int      Alpha = 0, Beta = 0 ;
	byte*    ClipTab = NULL;   
	int      small_gap;
	byte	*mb_offset;

	if(!bMixedEdge)
	{
		Alpha  =ALPHA_TABLE[*IndexA];
		Beta   =BETA_TABLE[*IndexB];
		ClipTab=CLIP_TAB[*IndexA];

		deblocking_luma_v_parallel_by_8_or_4(SrcPtrQ, dpel, Alpha, Beta, Strength);
	}
	else //bMixedEdge is true
	{
		int		Alpha_2[2], Beta_2[2];
		byte	*ClipTab_2[2];

		Alpha_2[0]  =ALPHA_TABLE[*IndexA];
		Beta_2[0]   =BETA_TABLE[*IndexB];
		ClipTab_2[0]=CLIP_TAB[*IndexA];

		IndexA++;
		IndexB++;
		Alpha_2[1]  =ALPHA_TABLE[*IndexA];
		Beta_2[1]   =BETA_TABLE[*IndexB];
		ClipTab_2[1]=CLIP_TAB[*IndexA];

		mb_offset = lp_mb_offset[fieldQ];

		for( pel=0 ; pel<PelNum ; pel++, SrcPtrQ+=dpel, mb_offset++)
		{
			Alpha  = Alpha_2[*mb_offset];
			Beta   = Beta_2[*mb_offset];
			ClipTab= ClipTab_2[*mb_offset];

			if( (Strng = Strength[pel]) )
			{
				L1  = SrcPtrQ[-2] ;
				L0  = SrcPtrQ[-1] ;
				R0  = SrcPtrQ[0] ;
				R1  = SrcPtrQ[1] ;

				AbsDelta  = abs( Delta = R0 - L0 )  ;

				if( AbsDelta < Alpha )
				{
					C0  = ClipTab[ Strng ];
					if( ((abs( R0 - R1) - Beta )  & (abs(L0 - L1) - Beta )) < 0  ) 
					{
						L2  = SrcPtrQ[-3] ;
						R2  = SrcPtrQ[2] ;
						aq  = (abs( R0 - R2) - Beta ) < 0  ;
						ap  = (abs( L0 - L2) - Beta ) < 0  ;

						RL0             = L0 + R0 ;

						if(Strng == 4 )    // INTRA strong filtering
						{           
							// Luma    
							small_gap = (AbsDelta < ((Alpha >> 2) + 2));

							aq &= small_gap;
							ap &= small_gap;

							if (ap)
							{
								L3  = SrcPtrQ[-4] ;
								SrcPtrQ[-3] = (((L3 + L2) <<1) + L2 + L1 + RL0 + 4) >> 3;
								SrcPtrQ[-2] = ( L2 + L1 + L0 + R0 + 2) >> 2;
								SrcPtrQ[-1] = ( R1 + ((L1 + RL0) << 1) +  L2 + 4) >> 3;
							}
							else
							{
								SrcPtrQ[-1] = ((L1 << 1) + L0 + R1 + 2) >> 2 ;
							}    
							if (aq)
							{
								R3  = SrcPtrQ[3] ;
								SrcPtrQ[0] = ( L1 + ((R1 + RL0) << 1) +  R2 + 4) >> 3;
								SrcPtrQ[1] = ( R2 + R0 + R1 + L0 + 2) >> 2;
								SrcPtrQ[2] = (((R3 + R2) <<1) + R2 + R1 + RL0 + 4) >> 3;
							}
							else
							{
								SrcPtrQ[0]      = ((R1 << 1) + R0 + L1 + 2) >> 2 ;
							}

						}
						else                                                     // normal filtering
						{
							c0               = (C0 + ap + aq) ;
							dif              = __fast_iclipX_X(c0, ( (Delta << 2) + (L1 - R1) + 4) >> 3 ) ;

							if( ap )
								SrcPtrQ[-2] += __fast_iclipX_X(C0, ( L2 + ((RL0 + 1) >> 1) - (L1<<1)) >> 1 ) ;
							SrcPtrQ[-1]  = CLIP0_255(L0 + dif) ;
							SrcPtrQ[ 0]  = CLIP0_255(R0 - dif) ;              
							if( aq  )
								SrcPtrQ[ 1] += __fast_iclipX_X(C0, ( R2 + ((RL0 + 1) >> 1) - (R1<<1)) >> 1 ) ;
						}
					} 
				}
			}
		}
	}
}

void Deblock_luma_v_0(imgpel* SrcPtrQ, int dpel, int Alpha, int Beta, int C0, int pNum)
{
	//do nothing
}

void Deblock_luma_v_1_c(imgpel* SrcPtrQ, int dpel, int Alpha, int Beta, int C0, int pNum)
{
	int c0, dif, aq, ap;
	int L2, L1, L0, R0, R1, R2;

	for(int pel=0 ; pel<pNum ; pel++, SrcPtrQ+=dpel)
	{
		L2  = SrcPtrQ[-3] ;
		L1  = SrcPtrQ[-2] ;
		L0  = SrcPtrQ[-1] ;
		R0  = SrcPtrQ[0] ;
		R1  = SrcPtrQ[1] ;
		R2  = SrcPtrQ[2] ;

		if( (abs(R0 - L0 ) < Alpha) & (abs( R0 - R1) < Beta ) & (abs(L0 - L1) < Beta ))
		{
			aq  = (abs( R0 - R2) - Beta ) < 0  ;
			ap  = (abs( L0 - L2) - Beta ) < 0  ;

			c0 = (C0 + ap + aq) ;
			dif = __fast_iclipX_X(c0, ( ((R0 - L0) << 2) + (L1 - R1) + 4) >> 3 ) ;

			SrcPtrQ[-1]  = CLIP0_255(L0 + dif) ;
			SrcPtrQ[ 0]  = CLIP0_255(R0 - dif) ; 
			if( ap )
				SrcPtrQ[-2] += __fast_iclipX_X(C0, ( L2 + ((L0 + R0 + 1) >> 1) - (L1<<1)) >> 1 ) ;         
			if( aq  )
				SrcPtrQ[ 1] += __fast_iclipX_X(C0, ( R2 + ((L0 + R0 + 1) >> 1) - (R1<<1)) >> 1 ) ;
		}
	}
}

void Deblock_luma_v_4(imgpel* SrcPtrQ, int dpel, int Alpha, int Beta, int C0, int pNum)
{
	int aq, ap, Delta, AbsDelta, small_gap;
	int L3, L2, L1, L0, R0, R1, R2, R3, RL0;

	for(int pel=0 ; pel<pNum ; pel++, SrcPtrQ+=dpel)
	{
		L1  = SrcPtrQ[-2] ;
		L0  = SrcPtrQ[-1] ;
		R0  = SrcPtrQ[0] ;
		R1  = SrcPtrQ[1] ;

		AbsDelta  = abs( Delta = R0 - L0 )  ;

		if( AbsDelta < Alpha )
		{
			if( ((abs( R0 - R1) - Beta )  & (abs(L0 - L1) - Beta )) < 0  ) 
			{
				L2  = SrcPtrQ[-3] ;
				R2  = SrcPtrQ[2] ;
				aq  = (abs( R0 - R2) - Beta ) < 0  ;
				ap  = (abs( L0 - L2) - Beta ) < 0  ;

				RL0             = L0 + R0 ;

				// Luma    
				small_gap = (AbsDelta < ((Alpha >> 2) + 2));

				aq &= small_gap;
				ap &= small_gap;

				if (ap)
				{
					L3  = SrcPtrQ[-4] ;
					SrcPtrQ[-3] = (((L3 + L2) <<1) + L2 + L1 + RL0 + 4) >> 3;
					SrcPtrQ[-2] = ( L2 + L1 + L0 + R0 + 2) >> 2;
					SrcPtrQ[-1] = ( R1 + ((L1 + RL0) << 1) +  L2 + 4) >> 3;
				}
				else
				{
					SrcPtrQ[-1] = ((L1 << 1) + L0 + R1 + 2) >> 2 ;
				}    
				if (aq)
				{
					R3  = SrcPtrQ[3] ;
					SrcPtrQ[0] = ( L1 + ((R1 + RL0) << 1) +  R2 + 4) >> 3;
					SrcPtrQ[1] = ( R2 + R0 + R1 + L0 + 2) >> 2;
					SrcPtrQ[2] = (((R3 + R2) <<1) + R2 + R1 + RL0 + 4) >> 3;
				}
				else
				{
					SrcPtrQ[0]      = ((R1 << 1) + R0 + L1 + 2) >> 2 ;
				}		
			} 
		}		
	}
}
