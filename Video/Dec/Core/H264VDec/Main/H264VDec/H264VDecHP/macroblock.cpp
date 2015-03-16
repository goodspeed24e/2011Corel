#pragma warning ( disable : 4804 )
/*!
***********************************************************************
* \file macroblock.c
*
* \brief
*     Decode a Macroblock
*
* \author
*    Main contributors (see contributors.h for copyright, address and affiliation details)
*    - Inge Lille-Langøy               <inge.lille-langoy@telenor.com>
*    - Rickard Sjoberg                 <rickard.sjoberg@era.ericsson.se>
*    - Jani Lainema                    <jani.lainema@nokia.com>
*    - Sebastian Purreiter             <sebastian.purreiter@mch.siemens.de>
*    - Thomas Wedi                     <wedi@tnt.uni-hannover.de>
*    - Detlev Marpe                    <marpe@hhi.de>
*    - Gabi Blaettermann               <blaetter@hhi.de>
*    - Ye-Kui Wang                     <wyk@ieee.org>
*    - Lowell Winger                   <lwinger@lsil.com>
***********************************************************************
*/

#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "global.h"
#include "mbuffer.h"
#include "elements.h"
#include "macroblock.h"
#include "fmo.h"
#include "cabac.h"
#include "vlc.h"
#include "image.h"
#include "mb_access.h"
#include "biaridecod.h"
#include "clipping.h"
#include "mb_motcomp.h"
//#include "get_neighbors.h"
#include "mb_average.h"
#include "get_block.h"
#include "emmintrin.h"

#define __FAST_MEM_ACCESS__

#define Set16MotionVector(MVectors, mv) \
	MVectors[0 ].mv_comb =  \
	MVectors[1 ].mv_comb =  \
	MVectors[2 ].mv_comb =  \
	MVectors[3 ].mv_comb =  \
	MVectors[4 ].mv_comb =  \
	MVectors[5 ].mv_comb =  \
	MVectors[6 ].mv_comb =  \
	MVectors[7 ].mv_comb =  \
	MVectors[8 ].mv_comb =  \
	MVectors[9 ].mv_comb =  \
	MVectors[10].mv_comb =  \
	MVectors[11].mv_comb =  \
	MVectors[12].mv_comb =  \
	MVectors[13].mv_comb =  \
	MVectors[14].mv_comb =  \
	MVectors[15].mv_comb = mv.mv_comb;

#define Set16x8MotionVector(MVectors, mv) \
	MVectors[0].mv_comb = \
	MVectors[1].mv_comb = \
	MVectors[2].mv_comb = \
	MVectors[3].mv_comb = \
	MVectors[4].mv_comb = \
	MVectors[5].mv_comb = \
	MVectors[6].mv_comb = \
	MVectors[7].mv_comb = mv.mv_comb;

#define Set8x16MotionVector(MVectors, mv) \
	MVectors[0 ].mv_comb = \
	MVectors[1 ].mv_comb = \
	MVectors[4 ].mv_comb = \
	MVectors[5 ].mv_comb = \
	MVectors[8 ].mv_comb = \
	MVectors[9 ].mv_comb = \
	MVectors[12].mv_comb = \
	MVectors[13].mv_comb = mv.mv_comb;

static void SetMotionVectorPredictor PARGS7(MotionVector    *pmv,
																						char            ref_frame,
																						byte            list,
																						int             block_x,
																						int             block_y,
																						int             blockshape_x,
																						int             blockshape_y);
static void SetMotionVectorPredictor_block00_shape16x16 PARGS6(MotionVector  *pmv,
																															 char           ref_frame,
																															 byte           list,
																															 int		    current_mb_nr,
																															 Macroblock	   *currMB,
																															 Macroblock_s  *currMB_s);
static void SetMotionVectorPredictor_block00_shape16x8 PARGS6(MotionVector  *pmv,
																															char           ref_frame,
																															byte           list,
																															int		    current_mb_nr,
																															Macroblock	   *currMB,
																															Macroblock_s  *currMB_s);
static void SetMotionVectorPredictor_block02_shape16x8 PARGS6(MotionVector  *pmv,
																															char           ref_frame,
																															byte           list,
																															int		    current_mb_nr,
																															Macroblock	   *currMB,
																															Macroblock_s  *currMB_s);
static void SetMotionVectorPredictor_block00_shape8x16 PARGS6(MotionVector  *pmv,
																															char           ref_frame,
																															byte           list,
																															int		    current_mb_nr,
																															Macroblock	   *currMB,
																															Macroblock_s  *currMB_s);
static void SetMotionVectorPredictor_block20_shape8x16 PARGS6(MotionVector  *pmv,
																															char           ref_frame,
																															byte           list,
																															int		    current_mb_nr,
																															Macroblock	   *currMB,
																															Macroblock_s  *currMB_s);


unsigned char cbp_blk_chroma[8][4] = {
	{16, 17, 18, 19},
	{20, 21, 22, 23},
	{24, 25, 26, 27},
	{28, 29, 30, 31},
	{32, 33, 34, 35},
	{36, 37, 38, 39},
	{40, 41, 42, 43},
	{44, 45, 46, 47} 
};

static const byte divmod6[104] = {
	0,0,0,1,0,2,0,3,0,4,0,5,
	1,0,1,1,1,2,1,3,1,4,1,5,
	2,0,2,1,2,2,2,3,2,4,2,5,
	3,0,3,1,3,2,3,3,3,4,3,5,
	4,0,4,1,4,2,4,3,4,4,4,5,
	5,0,5,1,5,2,5,3,5,4,5,5,
	6,0,6,1,6,2,6,3,6,4,6,5,
	7,0,7,1,7,2,7,3,7,4,7,5,
	8,0,8,1,8,2,8,3
};

static const byte peano_raster[16][2] = {
	{0, 0}, {0, 1}, {1, 0}, {1, 1},
	{0, 2}, {0, 3}, {1, 2}, {1, 3},
	{2, 0}, {2, 1}, {3, 0}, {3, 1},
	{2, 2}, {2, 3}, {3, 2}, {3, 3}
};

static const int j4_i4[4][4] = {
	{ 0,  1,  2,  3},
	{ 4,  5,  6,  7},
	{ 8,  9, 10, 11},
	{12, 13, 14, 15}
};

CREL_RETURN record_reference_picIds PARGS2(int list_offset, Macroblock_s *currMB_s);
///////////////////////////////////////////////////////////////////////////////
typedef CREL_RETURN (*pf_MB_IntraPred) 
PARGS2(imgpel * imgY, 
			 int stride);

typedef void (*pf_MB_InterPred) 
PARGS3(int vec_x_base,
			 int vec_y_base,
			 int list_offset);

static pf_MB_IntraPred fp_MB_IntraPred_I[]  = {
	&MB_I4MB_Luma,
	&MB_I16MB_Luma,
	NULL,
	NULL,
	&MB_I8MB_Luma
};
static pf_MB_InterPred fp_MB_InterPred_P0[] = {
	&MB_InterPred16x16,
	&MB_InterPred16x16,
	&MB_InterPred16x8,
	&MB_InterPred8x16,
	&MB_InterPred_b8mode_P0
};
static pf_MB_InterPred fp_MB_InterPred_P1[] = {
	&MB_InterPred16x16_1,
	&MB_InterPred16x16_1,
	&MB_InterPred16x8_1,
	&MB_InterPred8x16_1,
	&MB_InterPred_b8mode_P1
};
static pf_MB_InterPred fp_MB_InterPred_B0[] = {
	&MB_InterPred_b8mode_B0,
	&MB_InterPred16x16,
	&MB_InterPred16x8,
	&MB_InterPred8x16,
	&MB_InterPred_b8mode_B0
};
static pf_MB_InterPred fp_MB_InterPred_B1[] = {
	&MB_InterPred_b8mode_B1,
	&MB_InterPred16x16_1,
	&MB_InterPred16x8_1,
	&MB_InterPred8x16_1,
	&MB_InterPred_b8mode_B1
};
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
typedef int (*pf_ReadAndStoreCBPBlockBit) 
PARGS0();

static pf_ReadAndStoreCBPBlockBit fp_read_and_store_CBP_block_bit[]= 
{
	&read_and_store_CBP_block_bit_LUMA_16DC, // LUMA_16DC: 0
	&read_and_store_CBP_block_bit_LUMA_16AC, // LUMA_16AC: 1
	&read_and_store_CBP_block_bit_LUMA_8x8, // LUMA_8x8: 2
	NULL, // 3
	NULL, // 4
	&read_and_store_CBP_block_bit_LUMA_4x4, // LUMA_4x4: 5
	&read_and_store_CBP_block_bit_CHROMA_DC, // CHROMA_DC: 6
	&read_and_store_CBP_block_bit_CHROMA_AC // CHROMA_AC: 7
};
///////////////////////////////////////////////////////////////////////////////
/*!
***********************************************************************
* \brief
*    Luma DC inverse transform
***********************************************************************
*/
static void itrans_2 PARGS0()
{
	int i,j;
	int M5[4];
	int M6[4];

	int qp_per;
	int qp_rem;

	qp_per    = divmod6[(currMB_s_r->qp - MIN_QP)<<1];
	qp_rem    = divmod6[((currMB_s_r->qp - MIN_QP)<<1)+1];

	short *ptr;

	// horizontal
#if defined(ONE_COF)
	ptr = &IMGPAR cof[0][0][0][0];
#else
	ptr = IMGPAR cof_r;
#endif
	for (j=0;j<4;j++)
	{
		M5[0]=ptr[0 ];
		M5[1]=ptr[16];
		M5[2]=ptr[32];
		M5[3]=ptr[48];

		M6[0]=M5[0]+M5[2];
		M6[1]=M5[0]-M5[2];
		M6[2]=M5[1]-M5[3];
		M6[3]=M5[1]+M5[3];

		ptr[0 ]=M6[0]+M6[3];
		ptr[48]=M6[0]-M6[3];
		ptr[16]=M6[1]+M6[2];
		ptr[32]=M6[1]-M6[2];

		ptr += 64;
	}

	// vertical
#if defined(ONE_COF)
	ptr = &IMGPAR cof[0][0][0][0];
#else
	ptr = IMGPAR cof_r;
#endif

	qp_rem = InvLevelScale4x4Luma_Intra[qp_rem][0];
	if(qp_per<6)
	{
		int qp_const = 1<<(5-qp_per);
		qp_per = 6-qp_per;
		for (i=0;i<4;i++)
		{
			M5[0]=ptr[0];
			M5[1]=ptr[64];
			M5[2]=ptr[128];
			M5[3]=ptr[192];

			M6[0]=M5[0]+M5[2];
			M6[1]=M5[0]-M5[2];
			M6[2]=M5[1]-M5[3];
			M6[3]=M5[1]+M5[3];

			ptr[0]  =((M6[0]+M6[3])*qp_rem+qp_const)>>qp_per;
			ptr[192]=((M6[0]-M6[3])*qp_rem+qp_const)>>qp_per;
			ptr[64] =((M6[1]+M6[2])*qp_rem+qp_const)>>qp_per;
			ptr[128]=((M6[1]-M6[2])*qp_rem+qp_const)>>qp_per;

			ptr += 16;
		}
	}
	else
	{
		qp_per = qp_per-6;
		for (i=0;i<4;i++)
		{
			M5[0]=ptr[0];
			M5[1]=ptr[64];
			M5[2]=ptr[128];
			M5[3]=ptr[192];

			M6[0]=M5[0]+M5[2];
			M6[1]=M5[0]-M5[2];
			M6[2]=M5[1]-M5[3];
			M6[3]=M5[1]+M5[3];

			ptr[0]  =((M6[0]+M6[3])*qp_rem)<<qp_per;
			ptr[192]=((M6[0]-M6[3])*qp_rem)<<qp_per;
			ptr[64] =((M6[1]+M6[2])*qp_rem)<<qp_per;
			ptr[128]=((M6[1]-M6[2])*qp_rem)<<qp_per;

			ptr += 16;
		}
	}
}

/*!
************************************************************************
* \brief
*    initializes the current macroblock
************************************************************************
*/
#if defined (_COLLECT_PIC_)
CREL_RETURN start_macroblock_MBAff PARGS0()
{
#if !defined(__MB_POS_TABLE_LOOKUP__)
	int quotient=0;
	int width=IMGPAR PicWidthInMbs;
#endif
	int remainder=IMGPAR current_mb_nr_r;

	if (IMGPAR current_mb_nr_r >= IMGPAR PicSizeInMbs) {
		return CREL_WARNING_H264_STREAMERROR_LEVEL_3;
	}

#if defined (IP_RD_MERGE)
	if(!(IMGPAR type == B_SLICE && IMGPAR nal_reference_idc )){
#else		
	if ( (IMGPAR stream_global->m_is_MTMS == 1 || IMGPAR stream_global->m_bIsSingleThreadMode == TRUE) || (IMGPAR type == B_SLICE && (!IMGPAR nal_reference_idc || imgDXVAVer)) || (IMGPAR array_index >= 2)) {	// Working on MB based buffer
#endif	
		if (IMGPAR current_mb_nr_r & 1) {	//Odd MB
			currMB_r = currMB_d = IMGPAR pMBpair_current + 1;
		} else {
			currMB_r = currMB_d = IMGPAR pMBpair_current;
		}
		currMB_s_r = currMB_s_d = &dec_picture->mb_data[IMGPAR current_mb_nr_r];
		IMGPAR cof_r = IMGPAR cof_d = IMGPAR cof_array;

	} else {		// Working on Plane based buffer
		currMB_r   = &IMGPAR mb_decdata[IMGPAR current_mb_nr_r]; 
		currMB_s_r = &dec_picture->mb_data[IMGPAR current_mb_nr_r]; 

#if defined (IP_RD_MERGE)
		if ( img->firstSlice->picture_type != B_SLICE || IMGPAR nal_reference_idc ) {

			currMB_d = &IMGPAR mb_decdata[IMGPAR current_mb_nr_d];
			currMB_s_d = &IMGPAR m_dec_picture->mb_data[IMGPAR current_mb_nr_d];

		}
#endif

		IMGPAR cof_r = (IMGPAR cof_array + IMGPAR current_mb_nr_r*384);
	}

#if !defined(ONE_COF)
	//IMGPAR cof_r = (IMGPAR cof_array + IMGPAR current_mb_nr_r*384);
#endif

	/* Update coordinates of the current macroblock */
#if !defined(__MB_POS_TABLE_LOOKUP__)
	width = 2*width; 
	while(remainder >= width)
	{
		remainder -= width;
		quotient++;
	}    
	IMGPAR mb_x_r = remainder;
	IMGPAR mb_y_r = quotient<<1;

	if (IMGPAR mb_x_r & 1)
	{
		IMGPAR mb_y_r++;
	}

	IMGPAR mb_x_r >>= 1;
#else
	IMGPAR mb_x_r = mb_pos_table[remainder>>1].x;
	IMGPAR mb_y_r = ((mb_pos_table[remainder>>1].y)<<1) + (remainder & 0x01);
#endif

	/* Define vertical positions */
	//IMGPAR block_y_r = IMGPAR mb_y_r * BLOCK_SIZE;      /* luma block position */
	//IMGPAR pix_y_r   = IMGPAR mb_y_r * MB_BLOCK_SIZE;   /* luma macroblock position */
	//IMGPAR pix_c_y_r = IMGPAR mb_y_r * (MB_BLOCK_SIZE/2); /* chroma macroblock position */

	/* Define horizontal positions */
	//IMGPAR block_x_r = IMGPAR mb_x_r * BLOCK_SIZE;      /* luma block position */
	//IMGPAR pix_x_r   = IMGPAR mb_x_r * MB_BLOCK_SIZE;   /* luma pixel position */
	//IMGPAR pix_c_x_r = IMGPAR mb_x_r * (MB_BLOCK_SIZE/2); /* chroma pixel position */

	// Save the slice number of this macroblock. When the macroblock below
	// is coded it will use this to decide if prediction for above is possible
	currMB_s_r->slice_nr = IMGPAR current_slice_nr;

	if (IMGPAR current_slice_nr >= MAX_NUM_SLICES)
	{
		DEBUG_SHOW_ERROR_INFO ("[ERROR]maximum number of supported slices exceeded, please recompile with increased value for MAX_NUM_SLICES", 200);
		return CREL_WARNING_H264_STREAMERROR_LEVEL_3;
	}

	if (IMGPAR current_slice_nr > dec_picture->max_slice_id)
	{
		dec_picture->max_slice_id=IMGPAR current_slice_nr;
	}

	//CheckAvailabilityOfNeighbors_MBAff ARGS0();
	CheckAvailabilityOfNeighbors ARGS0();

	// Reset syntax element entries in MB struct
	currMB_s_r->qp				= IMGPAR qp ;
	currMB_s_r->mb_type = currMB_r->mb_type     = 0;  
	currMB_r->cbp         = 0;
	currMB_s_r->cbp_blk   = 0;
	currMB_r->c_ipred_mode= DC_PRED_8; //GB
	//memset(&currMB_r->mvd[0][0],0,32*sizeof(currMB_r->mvd[0][0]));

	currMB_r->cbp_bits   = 0;

	// store filtering parameters for this MB 
	return CREL_OK;
}
CREL_RETURN start_macroblock_NonMBAff PARGS0()
{
#if !defined(__MB_POS_TABLE_LOOKUP__)
	int quotient=0;
	int width=IMGPAR PicWidthInMbs;
#endif
	int remainder=IMGPAR current_mb_nr_r;

	if (IMGPAR current_mb_nr_r >= IMGPAR PicSizeInMbs) {
		return CREL_WARNING_H264_STREAMERROR_LEVEL_3;
	}

#if defined (IP_RD_MERGE)
	if(!(IMGPAR type == B_SLICE && IMGPAR nal_reference_idc )) {
#else		
	if ( (IMGPAR stream_global->m_is_MTMS == 1 || IMGPAR stream_global->m_bIsSingleThreadMode == TRUE) || (IMGPAR type == B_SLICE && (!IMGPAR nal_reference_idc || imgDXVAVer)) || (IMGPAR array_index >=2)) {	//Working on MB based buffer
#endif	
		currMB_r = currMB_d = IMGPAR pMBpair_current;
		currMB_s_r = currMB_s_d = &dec_picture->mb_data[IMGPAR current_mb_nr_r];

		IMGPAR cof_r = IMGPAR cof_d = IMGPAR cof_array;

	} else {		// Working on Plane based buffer
		currMB_r   = &IMGPAR mb_decdata[IMGPAR current_mb_nr_r]; 
		currMB_s_r = &dec_picture->mb_data[IMGPAR current_mb_nr_r]; 

#if defined (IP_RD_MERGE)
		if ( img->firstSlice->picture_type != B_SLICE || IMGPAR nal_reference_idc ) {

			currMB_d = &IMGPAR mb_decdata[IMGPAR current_mb_nr_d];
			currMB_s_d = &IMGPAR m_dec_picture->mb_data[IMGPAR current_mb_nr_d];

		}
#endif

		IMGPAR cof_r = (IMGPAR cof_array + IMGPAR current_mb_nr_r*384);
	}

	//currMB_r   = &IMGPAR mb_decdata[IMGPAR current_mb_nr_r]; 
	//currMB_s_r = &dec_picture->mb_data[IMGPAR current_mb_nr_r]; 
#if !defined(ONE_COF)
	//IMGPAR cof_r = (IMGPAR cof_array + IMGPAR current_mb_nr_r*384);
#endif

	/* Update coordinates of the current macroblock */
#if !defined(__MB_POS_TABLE_LOOKUP__)
	while(remainder >= width)
	{
		remainder -= width;
		quotient++;
	}    
	IMGPAR mb_x_r = remainder;
	IMGPAR mb_y_r = quotient;
#else
	IMGPAR mb_x_r = (((int)mb_pos_table[remainder].x));   
	IMGPAR mb_y_r = (((int)mb_pos_table[remainder].y));
#endif

	/* Define vertical positions */
	//IMGPAR block_y_r = IMGPAR mb_y_r * BLOCK_SIZE;      /* luma block position */
	//IMGPAR pix_y_r   = IMGPAR mb_y_r * MB_BLOCK_SIZE;   /* luma macroblock position */
	//IMGPAR pix_c_y_r = IMGPAR mb_y_r * (MB_BLOCK_SIZE/2); /* chroma macroblock position */

	/* Define horizontal positions */
	//IMGPAR block_x_r = IMGPAR mb_x_r * BLOCK_SIZE;      /* luma block position */
	//IMGPAR pix_x_r   = IMGPAR mb_x_r * MB_BLOCK_SIZE;   /* luma pixel position */
	//IMGPAR pix_c_x_r = IMGPAR mb_x_r * (MB_BLOCK_SIZE/2); /* chroma pixel position */

	// Save the slice number of this macroblock. When the macroblock below
	// is coded it will use this to decide if prediction for above is possible
	currMB_s_r->slice_nr = IMGPAR current_slice_nr;

	if (IMGPAR current_slice_nr >= MAX_NUM_SLICES)
	{
		DEBUG_SHOW_ERROR_INFO ("[ERROR]maximum number of supported slices exceeded, please recompile with increased value for MAX_NUM_SLICES", 200);
		return CREL_WARNING_H264_STREAMERROR_LEVEL_3;
	}

	if (IMGPAR current_slice_nr > dec_picture->max_slice_id)
	{
		dec_picture->max_slice_id=IMGPAR current_slice_nr;
	}

	//CheckAvailabilityOfNeighbors_NonMBAff ARGS0();
	CheckAvailabilityOfNeighbors ARGS0();

	// Reset syntax element entries in MB struct
	currMB_s_r->qp				= IMGPAR qp ;
	currMB_s_r->mb_type = currMB_r->mb_type    = 0;  
	currMB_r->cbp         = 0;
	currMB_s_r->cbp_blk   = 0;
	currMB_r->c_ipred_mode= DC_PRED_8; //GB
	//memset(&currMB_r->mvd[0][0],0,32*sizeof(currMB_r->mvd[0][0]));

	currMB_r->cbp_bits   = 0;

	// store filtering parameters for this MB 
	return CREL_OK;
}
#else
CREL_RETURN start_macroblock PARGS0()
{
#if !defined(__MB_POS_TABLE_LOOKUP__)
	int quotient=0;
	int width=IMGPAR PicWidthInMbs;
#endif
	int remainder=IMGPAR current_mb_nr_r;

	if (IMGPAR current_mb_nr_r >= IMGPAR PicSizeInMbs) {
		return CREL_WARNING_H264_STREAMERROR_LEVEL_3;
	}


	//currMB_r   = &IMGPAR mb_decdata[IMGPAR current_mb_nr_r]; 
	//currMB_s_r = &dec_picture->mb_data[IMGPAR current_mb_nr_r]; 
	if (IMGPAR MbaffFrameFlag && (IMGPAR current_mb_nr_r & 1)) {
		currMB_r = currMB_d = IMGPAR pMBpair_current + 1;
	} else {
		currMB_r = currMB_d = IMGPAR pMBpair_current;
	}
	currMB_s_r = currMB_s_d = &dec_picture->mb_data[IMGPAR current_mb_nr_r];

	/* Update coordinates of the current macroblock */
	if (IMGPAR MbaffFrameFlag)
	{
#if !defined(__MB_POS_TABLE_LOOKUP__)
		width = 2*width; 
		while(remainder >= width)
		{
			remainder -= width;
			quotient++;
		}    
		IMGPAR mb_x_r = remainder;
		IMGPAR mb_y_r = quotient<<1;

		if (IMGPAR mb_x_r & 1)
		{
			IMGPAR mb_y_r++;
		}

		IMGPAR mb_x_r >>= 1;
#else
		IMGPAR mb_x_r = mb_pos_table[remainder>>1].x;
		IMGPAR mb_y_r = ((mb_pos_table[remainder>>1].y)<<1) + (remainder & 0x01);

#endif
	}
	else
	{
#if !defined(__MB_POS_TABLE_LOOKUP__)

		while(remainder >= width)
		{
			remainder -= width;
			quotient++;
		}    
		IMGPAR mb_x_r = remainder;
		IMGPAR mb_y_r = quotient;
#else
		IMGPAR mb_x_r = (((int)mb_pos_table[remainder].x));   
		IMGPAR mb_y_r = (((int)mb_pos_table[remainder].y));
#endif
	}

	// Save the slice number of this macroblock. When the macroblock below
	// is coded it will use this to decide if prediction for above is possible
	currMB_s_r->slice_nr = IMGPAR current_slice_nr;

	if (IMGPAR current_slice_nr >= MAX_NUM_SLICES)
	{
		DEBUG_SHOW_ERROR_INFO ("[ERROR]maximum number of supported slices exceeded, please recompile with increased value for MAX_NUM_SLICES", 200);
	}

	if (IMGPAR current_slice_nr > dec_picture->max_slice_id)
	{
		dec_picture->max_slice_id=IMGPAR current_slice_nr;
	}

	CheckAvailabilityOfNeighbors ARGS0();

	// Reset syntax element entries in MB struct
	currMB_s_r->qp				= IMGPAR qp ;
	currMB_s_r->mb_type = currMB_r->mb_type     = 0;  
	currMB_r->cbp         = 0;
	currMB_s_r->cbp_blk   = 0;
	currMB_r->c_ipred_mode= DC_PRED_8; //GB
	//memset(&currMB_r->mvd[0][0],0,32*sizeof(currMB_r->mvd[0][0]));

	currMB_r->cbp_bits   = 0;

	// store filtering parameters for this MB 

	return CREL_OK;
}
#endif


/*!
************************************************************************
* function: void SetIntraPredDecMode PARGS0()
* - created by Grant
*  set neighboring condition for intra prediction depending on constrained_intra_pred_flag
*  constrained_intra_pred_flag = 0:
*		if the neighbor is not available, set mbStatus to -1
*  constrained_intra_pred_flag = 1:
*		if the neighbor is not available or  neighbor is not intra, set mbStatus to -1.
*       for the left neighbors, use flag mbIntraA (2 bits) to store intra_block information
************************************************************************
*/
void SetIntraPredDecMode PARGS0()
{
	if(active_pps.constrained_intra_pred_flag)
	{
		if (IMGPAR pLeftMB_r==0)
		{
			currMB_r->mbStatusA = -1;
		}
		else
		{
			if (IMGPAR MbaffFrameFlag)
			{				
				switch(currMB_r->mbStatusA)
				{
				case 1:						
				case 2:						
				case 3:						
				case 4:					
					currMB_r->mbIntraA  = IMGPAR pLeftMB_r->intra_block;
					currMB_r->mbIntraA |= ((IMGPAR pLeftMB_r+1)->intra_block<<1);
					break;				
				default:						
					if(!IMGPAR pLeftMB_r->intra_block)
						currMB_r->mbStatusA = -1;						
					break;
				}							
			}
			else
			{				
				if(!IMGPAR pLeftMB_r->intra_block)
					currMB_r->mbStatusA = -1;				
			}
		}

		if (IMGPAR pUpMB_r==0)
		{
			currMB_r->mbStatusB = -1;
		}
		else
		{
			if(!IMGPAR pUpMB_r->intra_block)
				currMB_r->mbStatusB = -1;			
		}

		if (IMGPAR pUpRightMB_r==0)
		{
			currMB_r->mbStatusC = -1;
		}
		else
		{
			if(!IMGPAR pUpRightMB_r->intra_block)
				currMB_r->mbStatusC = -1;
		}

		if (IMGPAR pUpLeftMB_r==0)
		{
			currMB_r->mbStatusD = -1;
		}
		else
		{
			if(!IMGPAR pUpLeftMB_r->intra_block)
				currMB_r->mbStatusD = -1;		
		}
	}
	else
	{
		if (IMGPAR pLeftMB_r==0)
		{
			currMB_r->mbStatusA = -1;
		}		

		if (IMGPAR pUpMB_r==0)
		{
			currMB_r->mbStatusB = -1;
		}		

		if (IMGPAR pUpRightMB_r==0)
		{
			currMB_r->mbStatusC = -1;
		}

		if (IMGPAR pUpLeftMB_r==0)
		{
			currMB_r->mbStatusD = -1;
		}		
	}	
}

/*!
************************************************************************
* \brief
*    set coordinates of the next macroblock
*    check end_of_slice condition 
************************************************************************
*/
CREL_RETURN exit_macroblock PARGS2(int eos_bit, BOOL* slice_end)
{
	//! The if() statement below resembles the original code, which tested 
	//! IMGPAR current_mb_nr_r == IMGPAR PicSizeInMbs.  Both is, of course, nonsense
	//! In an error prone environment, one can only be sure to have a new
	//! picture by checking the tr of the next slice header!

	IMGPAR num_dec_mb++;


#if defined (_COLLECT_PIC_)
#if defined (IP_RD_MERGE)
	if(!(IMGPAR type == B_SLICE && IMGPAR nal_reference_idc )) {
#else		
	if ( (IMGPAR stream_global->m_is_MTMS == 1 || IMGPAR stream_global->m_bIsSingleThreadMode == TRUE) || (IMGPAR type == B_SLICE && (!IMGPAR nal_reference_idc || imgDXVAVer)) ) {
#endif	
#else
	if (1) {
#endif
		int mb_row_pos;

		Macroblock *temp;

		if ( IMGPAR MbaffFrameFlag ) {
			if ( IMGPAR current_mb_nr_d & 1 ) {
				mb_row_pos = IMGPAR mb_x_r<<1;
				if ( (IMGPAR mb_x_r+ 1) == IMGPAR PicWidthInMbs ) {	// Last MB pair in MB Row
					memcpy(IMGPAR mb_decdata + mb_row_pos - 2, IMGPAR pMBpair_left, sizeof(Macroblock)*2);		//Left -> TopLeft
					memcpy(IMGPAR mb_decdata + mb_row_pos, IMGPAR pMBpair_current, sizeof(Macroblock)*2);		//Current -> Top					
				} else {
					if ( mb_row_pos ) {				//First MB in Row no Left->TopLeft coppy
						memcpy(IMGPAR mb_decdata + mb_row_pos - 2, IMGPAR pMBpair_left, sizeof(Macroblock)*2);		//Left -> TopLeft
					}
					temp=IMGPAR pMBpair_left;					
					IMGPAR pMBpair_left=IMGPAR pMBpair_current;
					IMGPAR pMBpair_current=temp;
				}
			}
		} else {
			mb_row_pos = IMGPAR mb_x_r;
			if ( (mb_row_pos+ 1) == IMGPAR PicWidthInMbs ) {	// Last MB pair in MB Row
				memcpy(IMGPAR mb_decdata + mb_row_pos - 1, IMGPAR pMBpair_left, sizeof(Macroblock));		//Left -> TopLeft
				memcpy(IMGPAR mb_decdata + mb_row_pos, IMGPAR pMBpair_current, sizeof(Macroblock));		//Current -> Top					
			} else {
				if ( mb_row_pos ) {					//First MB in Row no Left->TopLeft coppy
					memcpy(IMGPAR mb_decdata + mb_row_pos - 1, IMGPAR pMBpair_left, sizeof(Macroblock));		//Left -> TopLeft
				}
				temp=IMGPAR pMBpair_left;
				IMGPAR pMBpair_left=IMGPAR pMBpair_current;
				IMGPAR pMBpair_current=temp;					
			}
		}
	}
	/*
	if (IMGPAR num_dec_mb > 390 ) {
	IMGPAR num_dec_mb = IMGPAR num_dec_mb;
	}
	*/
#if !defined(_COLLECT_PIC_)
	if (IMGPAR num_dec_mb == IMGPAR PicSizeInMbs)
#else
	if ((IMGPAR num_dec_mb + IMGPAR m_start_mb_nr == IMGPAR PicSizeInMbs) ||
		(img->currentSlice->nextSlice && (IMGPAR num_dec_mb + (IMGPAR m_start_mb_nr<<IMGPAR MbaffFrameFlag)) == ((Slice*)img->currentSlice->nextSlice)->start_mb_nr << IMGPAR MbaffFrameFlag ))

#endif
		//  if (IMGPAR current_mb_nr_r == FmoGetLastMBOfPicture(currSlice->structure))
	{
		//thb
		/*
		if (currSlice->next_header != EOS)
		currSlice->next_header = SOP;
		*/
		//the
		IMGPAR current_mb_nr_r++; //Grant, add for loopfilter
#if !defined(_COLLECT_PIC_)
		IMGPAR current_mb_nr_d = IMGPAR current_mb_nr_r;
#endif
		//assert (nal_startcode_follows ARGS1(eos_bit) == TRUE);
		IMGPAR bReadCompleted = TRUE;
		*slice_end = TRUE;
		return CREL_OK;
	}
	// ask for last mb in the slice  UVLC
	else
	{

		//		IMGPAR current_mb_nr_r = FmoGetNextMBNr ARGS1((IMGPAR current_mb_nr_r));

		IMGPAR current_mb_nr_r++;
		if (IMGPAR current_mb_nr_r >= IMGPAR PicSizeInMbs)
			IMGPAR current_mb_nr_r = -1;

#if !defined(_COLLECT_PIC_)
		IMGPAR current_mb_nr_d = IMGPAR current_mb_nr_r;
#endif

		if (IMGPAR current_mb_nr_r == -1)     // End of Slice group, MUST be end of slice
		{
			if (nal_startcode_follows ARGS1(eos_bit) == FALSE) {
				return CREL_WARNING_H264_STREAMERROR_LEVEL_3;
			}		
			*slice_end = TRUE;
			return CREL_OK;
		}

		if ( (nal_startcode_follows ARGS1(eos_bit) == FALSE) || (IMGPAR cod_counter>0) ){
			*slice_end = FALSE;
			return CREL_OK;
		} else {
			return CREL_WARNING_H264_STREAMERROR_LEVEL_3;
			//*slice_end = *slice_end;
		}

		/*
		if(IMGPAR type == I_SLICE || active_pps.entropy_coding_mode_flag == CABAC) {
		*slice_end = TRUE;
		return CREL_OK;
		}

		if(IMGPAR cod_counter<=0) {
		*slice_end = TRUE;
		return CREL_OK;
		}

		*slice_end = FALSE;
		return CREL_OK;
		*/

	}
}

CREL_RETURN exit_macroblock_FMO PARGS2(int eos_bit, BOOL* slice_end)
{
	//! The if() statement below resembles the original code, which tested 
	//! IMGPAR current_mb_nr_r == IMGPAR PicSizeInMbs.  Both is, of course, nonsense
	//! In an error prone environment, one can only be sure to have a new
	//! picture by checking the tr of the next slice header!

	IMGPAR num_dec_mb++;

	EnterCriticalSection( &(IMGPAR stream_global->m_csExit_MB) );
	dec_picture->decoded_mb_number++;
	LeaveCriticalSection( &(IMGPAR stream_global->m_csExit_MB) );


#if defined (_COLLECT_PIC_)
#if defined (IP_RD_MERGE)
	if(!(IMGPAR type == B_SLICE && IMGPAR nal_reference_idc )) {
#else		
	if ( (IMGPAR stream_global->m_is_MTMS == 1 || IMGPAR stream_global->m_bIsSingleThreadMode == TRUE) || (IMGPAR type == B_SLICE && (!IMGPAR nal_reference_idc || imgDXVAVer)) ) {
#endif	
#else
	if (1) {
#endif
		int mb_row_pos;

		Macroblock *temp;

		if ( IMGPAR MbaffFrameFlag ) {
			if ( IMGPAR current_mb_nr_d & 1 ) {
				mb_row_pos = IMGPAR mb_x_r<<1;
				if ( (IMGPAR mb_x_r+ 1) == IMGPAR PicWidthInMbs ) {	// Last MB pair in MB Row
					memcpy(IMGPAR mb_decdata + mb_row_pos - 2, IMGPAR pMBpair_left, sizeof(Macroblock)*2);		//Left -> TopLeft
					memcpy(IMGPAR mb_decdata + mb_row_pos, IMGPAR pMBpair_current, sizeof(Macroblock)*2);		//Current -> Top					
				} else {
					if ( mb_row_pos ) {				//First MB in Row no Left->TopLeft coppy
						memcpy(IMGPAR mb_decdata + mb_row_pos - 2, IMGPAR pMBpair_left, sizeof(Macroblock)*2);		//Left -> TopLeft
					}
					temp=IMGPAR pMBpair_left;					
					IMGPAR pMBpair_left=IMGPAR pMBpair_current;
					IMGPAR pMBpair_current=temp;
				}
			}
		} else {
			mb_row_pos = IMGPAR mb_x_r;
			if ( (mb_row_pos+ 1) == IMGPAR PicWidthInMbs ) {	// Last MB pair in MB Row
				memcpy(IMGPAR mb_decdata + IMGPAR mb_left_x_r_FMO /*mb_row_pos - 1*/, IMGPAR pMBpair_left, sizeof(Macroblock));		//Left -> TopLeft
				memcpy(IMGPAR mb_decdata + mb_row_pos, IMGPAR pMBpair_current, sizeof(Macroblock));		//Current -> Top					
			} else {
				if ( mb_row_pos ) {					//First MB in Row no Left->TopLeft coppy
					memcpy(IMGPAR mb_decdata + IMGPAR mb_left_x_r_FMO /*mb_row_pos - 1*/, IMGPAR pMBpair_left, sizeof(Macroblock));		//Left -> TopLeft
				}
				temp=IMGPAR pMBpair_left;
				IMGPAR pMBpair_left=IMGPAR pMBpair_current;
				IMGPAR pMBpair_current=temp;					
			}
		}
		IMGPAR mb_left_x_r_FMO = IMGPAR mb_x_r;
	}

	/*
	if (IMGPAR num_dec_mb > 390 ) {
	IMGPAR num_dec_mb = IMGPAR num_dec_mb;
	}
	*/
#if !defined(_COLLECT_PIC_)
	if (IMGPAR num_dec_mb == IMGPAR PicSizeInMbs)
#else
	if ((dec_picture->decoded_mb_number == IMGPAR PicSizeInMbs) /*||
		(img->currentSlice->nextSlice && (IMGPAR num_dec_mb + (IMGPAR m_start_mb_nr<<IMGPAR MbaffFrameFlag)) == ((Slice*)img->currentSlice->nextSlice)->start_mb_nr << IMGPAR MbaffFrameFlag )*/)

#endif
		//if (IMGPAR current_mb_nr_r == FmoGetLastMBOfPicture ARGS0(IMGPAR currentSlice->structure))
	{
		//thb
		/*
		if (currSlice->next_header != EOS)
		currSlice->next_header = SOP;
		*/
		//the
		IMGPAR current_mb_nr_r = FmoGetNextMBNr ARGS1((IMGPAR current_mb_nr_r));
		//IMGPAR current_mb_nr_r++; //Grant, add for loopfilter
#if !defined(_COLLECT_PIC_)
		IMGPAR current_mb_nr_d = IMGPAR current_mb_nr_r;
#endif
		//assert (nal_startcode_follows ARGS1(eos_bit) == TRUE);
		IMGPAR bReadCompleted = TRUE;
		*slice_end = TRUE;

		return CREL_OK;
	}
	// ask for last mb in the slice  UVLC
	else
	{

		IMGPAR current_mb_nr_r = FmoGetNextMBNr ARGS1((IMGPAR current_mb_nr_r));

		//IMGPAR current_mb_nr_r++;
		if (IMGPAR current_mb_nr_r >= IMGPAR PicSizeInMbs)
			IMGPAR current_mb_nr_r = -1;

#if !defined(_COLLECT_PIC_)
		IMGPAR current_mb_nr_d = IMGPAR current_mb_nr_r;
#endif

		if (IMGPAR current_mb_nr_r == -1)     // End of Slice group, MUST be end of slice
		{
			if (nal_startcode_follows ARGS1(eos_bit) == FALSE) {
				return CREL_WARNING_H264_STREAMERROR_LEVEL_3;
			}
			*slice_end = TRUE;
			return CREL_OK;
		}

		if ( (nal_startcode_follows ARGS1(eos_bit) == FALSE) || (IMGPAR cod_counter>0) ){
			*slice_end = FALSE;
			return CREL_OK;
		} /*else {
		  return CREL_WARNING_H264_STREAMERROR_LEVEL_3;
		  //*slice_end = *slice_end;
		  }*/

		if(IMGPAR type == I_SLICE || active_pps.entropy_coding_mode_flag == CABAC) {
			*slice_end = TRUE;			
			return CREL_OK;			
		}

		if(IMGPAR cod_counter<=0) {
			*slice_end = TRUE;
			return CREL_OK;
		}

		*slice_end = FALSE;
		return CREL_OK;
	}
}

static const int ICBPTAB[6] = {0,16,32,15,31,47};

/*!
************************************************************************
* \brief
*    Interpret the mb mode for P-Frames
************************************************************************
*/
CREL_RETURN interpret_mb_mode_P PARGS0()
{
	int mbmode = currMB_r->mb_type;

#define ZERO_P8x8     (mbmode==5)
#define USE_LOOKUP
#ifdef USE_LOOKUP
	static const int mb_types[] = { 
		0,							// P_Skip
		PB_16x16, PB_16x8, PB_8x16,		// P_L0_16x16, P_L0_L0_16x8, P_L0_L0_8x16
		PB_8x8, PB_8x8,					// P_8x8, P_8x8ref0
		I4MB,						//  0
		I16MB, I16MB, I16MB, I16MB,	//  1- 4
		I16MB, I16MB, I16MB, I16MB,	//  5- 8
		I16MB, I16MB, I16MB, I16MB,	//  9-12
		I16MB, I16MB, I16MB, I16MB,	// 13-16
		I16MB, I16MB, I16MB, I16MB,	// 17-20
		I16MB, I16MB, I16MB, I16MB,	// 21-24
		IPCM						// 25
	};
	static const int b8modes[] =  { 
		0,									// P_Skip
		PB_16x16|(PB_16x16<<8)|(PB_16x16<<16)|(PB_16x16<<24),// P_L0_16x16
		PB_16x8 |(PB_16x8<<8) |(PB_16x8<<16) |(PB_16x8<<24), // P_L0_L0_16x8
		PB_8x16 |(PB_8x16<<8) |(PB_8x16<<16) |(PB_8x16<<24), // P_L0_L0_8x16
		PB_8x8  |(PB_8x8<<8)  |(PB_8x8<<16)  |(PB_8x8<<24),  // P_8x8
		PB_8x8  |(PB_8x8<<8)  |(PB_8x8<<16)  |(PB_8x8<<24),  // P_8x8ref0
		IBLOCK|(IBLOCK<<8)|(IBLOCK<<16)|(IBLOCK<<24),//  0
		0, 0, 0, 0,							//  1- 4
		0, 0, 0, 0,							//  5- 8
		0, 0, 0, 0,							//  9-12
		0, 0, 0, 0,							// 13-16
		0, 0, 0, 0,							// 17-20
		0, 0, 0, 0,							// 21-24
		0									// 25
	};
	static const int b8pdirs[] =  { 
		0x00000000,							//  0    : P_Skip
		0x00000000, 0x00000000, 0x00000000,	//  1- 3 : P_L0_16x16, P_L0_L0_16x8, P_L0_L0_8x16
		0x00000000, 0x00000000,				//  4- 5 : P_8x8, P_8x8ref0
		-1,									//  6    :
		-1, -1, -1, -1,						//  7-10 :
		-1, -1, -1, -1,						// 11-14 :
		-1, -1, -1, -1,						// 15-18 :
		-1, -1, -1, -1,						// 19-22 :
		-1, -1, -1, -1,						// 23-26 :
		-1, -1, -1, -1,						// 27-30 :
		-1									// 31    :
	};
	static const int cbps[] =		{ 
		0,				// P_Skip
		0, 0, 0,		// P_L0_16x16, P_L0_L0_16x8, P_L0_L0_8x16
		0, 0,			// P_8x8, P_8x8ref0
		0,				//  0
		0, 0, 0, 0,		//  1- 4
		16,16,16,16,	//  5- 8
		32,32,32,32,	//  9-12
		15,15,15,15,	// 13-16
		31,31,31,31,	// 17-20
		47,47,47,47,	// 21-24
		-1				// 25
	};
	static const int i16modes[] = {
		0,			// P_Skip
		0, 0, 0,	// P_L0_16x16, P_L0_L0_16x8, P_L0_L0_8x16
		0, 0,		// P_8x8, P_8x8ref0
		0,			//  0
		0, 1, 2, 3,	//  1- 4
		0, 1, 2, 3,	//  5- 8
		0, 1, 2, 3,	//  9-12
		0, 1, 2, 3,	// 13-16
		0, 1, 2, 3,	// 17-20
		0, 1, 2, 3,	// 21-24
		0			// 25
	};

	if (mbmode > 31) {
		return CREL_WARNING_H264_STREAMERROR_LEVEL_3;
	}
	currMB_s_r->mb_type =  currMB_r->mb_type = mb_types[mbmode];
	*(int *) &currMB_r->b8mode[0] = b8modes[mbmode];
	*(int *) &currMB_r->b8pdir[0] = b8pdirs[mbmode];
	currMB_r->cbp                 = cbps[mbmode];
	currMB_r->i16mode             = i16modes[mbmode];
	IMGPAR allrefzero             = ZERO_P8x8;

#else

	int i;

#define MODE_IS_P8x8  (mbmode==4 || mbmode==5)
#define MODE_IS_I4x4  (mbmode==6)
#define I16OFFSET     (mbmode-7)
#define MODE_IS_IPCM  (mbmode==31)

	if(mbmode <4)
	{
		currMB_r->mb_type = mbmode;
		for (i=0;i<4;i++)
		{
			currMB_r->b8mode[i]   = mbmode;
			currMB_r->b8pdir[i]   = 0;
		}
	}
	else if(MODE_IS_P8x8)
	{
		currMB_r->mb_type = PB_8x8;
		IMGPAR allrefzero = ZERO_P8x8;
	}
	else if(MODE_IS_I4x4)
	{
		currMB_r->mb_type = I4MB;
		for (i=0;i<4;i++)
		{
			currMB_r->b8mode[i] = IBLOCK;
			currMB_r->b8pdir[i] = -1;
		}
	}
	else if(MODE_IS_IPCM)
	{
		currMB_r->mb_type=IPCM;

		for (i=0;i<4;i++) 
		{
			currMB_r->b8mode[i]=0; 
			currMB_r->b8pdir[i]=-1; 
		}
		currMB_r->cbp= -1;
		currMB_r->i16mode = 0;
	}
	else
	{
		currMB_r->mb_type = I16MB;
		for (i=0;i<4;i++) 
		{
			currMB_r->b8mode[i]=0; 
			currMB_r->b8pdir[i]=-1;
		}
		currMB_r->cbp= ICBPTAB[(I16OFFSET)>>2];
		currMB_r->i16mode = (I16OFFSET) & 0x03;
	}
#endif
	return CREL_OK;
}

/*!
************************************************************************
* \brief
*    Interpret the mb mode for I-Frames
************************************************************************
*/
CREL_RETURN interpret_mb_mode_I PARGS0()
{
	int mbmode   = currMB_r->mb_type;
#ifdef USE_LOOKUP
	static const int mb_types[] = { 
		I4MB,						//  0
		I16MB, I16MB, I16MB, I16MB,	//  1- 4
		I16MB, I16MB, I16MB, I16MB,	//  5- 8
		I16MB, I16MB, I16MB, I16MB,	//  9-12
		I16MB, I16MB, I16MB, I16MB,	// 13-16
		I16MB, I16MB, I16MB, I16MB,	// 17-20
		I16MB, I16MB, I16MB, I16MB,	// 21-24
		IPCM						// 25
	};
	static const int b8modes[] =  { 
		IBLOCK|(IBLOCK<<8)|(IBLOCK<<16)|(IBLOCK<<24),//  0
		0, 0, 0, 0,	//  1- 4
		0, 0, 0, 0,	//  5- 8
		0, 0, 0, 0,	//  9-12
		0, 0, 0, 0,	// 13-16
		0, 0, 0, 0,	// 17-20
		0, 0, 0, 0,	// 21-24
		0			// 25
	};
	static const int b8pdirs[] =  { 
		-1,				//  0
		-1, -1, -1, -1,	//  1- 4
		-1, -1, -1, -1,	//  5- 8
		-1, -1, -1, -1,	//  9-12
		-1, -1, -1, -1,	// 13-16
		-1, -1, -1, -1,	// 17-20
		-1, -1, -1, -1,	// 21-24
		-1				// 25
	};
	static const int cbps[] =	  { 
		0,				//  0
		0, 0, 0, 0,		//  1- 4
		16,16,16,16,	//  5- 8
		32,32,32,32,	//  9-12
		15,15,15,15,	// 13-16
		31,31,31,31,	// 17-20
		47,47,47,47,	// 21-24
		-1				// 25
	};
	static const int i16modes[] = {
		0,			//  0
		0, 1, 2, 3,	//  1- 4
		0, 1, 2, 3,	//  5- 8
		0, 1, 2, 3,	//  9-12
		0, 1, 2, 3,	// 13-16
		0, 1, 2, 3,	// 17-20
		0, 1, 2, 3,	// 21-24
		0			// 25
	};

	if (mbmode > 25) {
		return CREL_WARNING_H264_STREAMERROR_LEVEL_3;
	}
	currMB_s_r->mb_type = currMB_r->mb_type             = mb_types[mbmode];
	*(int *) &currMB_r->b8mode[0] = b8modes[mbmode];
	*(int *) &currMB_r->b8pdir[0] = -1; //b8pdirs[mbmode];
	currMB_r->cbp                 = cbps[mbmode];
	currMB_r->i16mode             = i16modes[mbmode];

#else

	int i;
	if (mbmode==0)
	{
		currMB_r->mb_type = I4MB;
		for (i=0;i<4;i++) {currMB_r->b8mode[i]=IBLOCK; currMB_r->b8pdir[i]=-1; }
	}
	else if(mbmode==25)
	{
		currMB_r->mb_type=IPCM;

		for (i=0;i<4;i++) {currMB_r->b8mode[i]=0; currMB_r->b8pdir[i]=-1; }
		currMB_r->cbp= -1;
		currMB_r->i16mode = 0;

	}
	else
	{
		currMB_r->mb_type = I16MB;
		for (i=0;i<4;i++) {currMB_r->b8mode[i]=0; currMB_r->b8pdir[i]=-1; }
		currMB_r->cbp= ICBPTAB[(mbmode-1)>>2];
		currMB_r->i16mode = (mbmode-1) & 0x03;
	}
#endif
	return CREL_OK;
}

/*!
************************************************************************
* \brief
*    Interpret the mb mode for B-Frames
************************************************************************
*/
CREL_RETURN interpret_mb_mode_B PARGS0()
{
	int mbmode;
#ifdef USE_LOOKUP
	mbmode  = currMB_r->mb_type;
	static const int mb_types[] = { 
		0,							//  0    : direct
		PB_16x16,PB_16x16,PB_16x16,		//  1- 3 : 16x16
		PB_16x8, PB_8x16, PB_16x8, PB_8x16,	//  4- 7
		PB_16x8, PB_8x16, PB_16x8, PB_8x16,	//  8-11
		PB_16x8, PB_8x16, PB_16x8, PB_8x16,	// 12-15
		PB_16x8, PB_8x16, PB_16x8, PB_8x16,	// 16-19
		PB_16x8, PB_8x16,				// 20-21 : 16x8, 8x16 interleaved
		PB_8x8,						// 22    :  8x8
		I4MB,						// 23    :
		I16MB, I16MB, I16MB, I16MB,	// 24-27 :
		I16MB, I16MB, I16MB, I16MB,	// 28-31 :
		I16MB, I16MB, I16MB, I16MB,	// 32-35 :
		I16MB, I16MB, I16MB, I16MB,	// 36-39 :
		I16MB, I16MB, I16MB, I16MB,	// 40-43 :
		I16MB, I16MB, I16MB, I16MB,	// 44-47 :
		IPCM						// 48    :
	};
	static const int b8modes[] =  { 
		0x00000000,										//  0    : direct
		PB_16x16|(PB_16x16<<8)|(PB_16x16<<16)|(PB_16x16<<24),	//  1    : 16x16
		PB_16x16|(PB_16x16<<8)|(PB_16x16<<16)|(PB_16x16<<24),	//  2    : 16x16
		PB_16x16|(PB_16x16<<8)|(PB_16x16<<16)|(PB_16x16<<24),	//  3    : 16x16
		PB_16x8 |(PB_16x8<<8) |(PB_16x8<<16) |(PB_16x8<<24),	//  4    : 16x8
		PB_8x16 |(PB_8x16<<8) |(PB_8x16<<16) |(PB_8x16<<24),	//  5    : 8x16
		PB_16x8 |(PB_16x8<<8) |(PB_16x8<<16) |(PB_16x8<<24),	//  6    : 16x8
		PB_8x16 |(PB_8x16<<8) |(PB_8x16<<16) |(PB_8x16<<24),	//  7    : 8x16
		PB_16x8 |(PB_16x8<<8) |(PB_16x8<<16) |(PB_16x8<<24),	//  8    : 16x8
		PB_8x16 |(PB_8x16<<8) |(PB_8x16<<16) |(PB_8x16<<24),	//  9    : 8x16
		PB_16x8 |(PB_16x8<<8) |(PB_16x8<<16) |(PB_16x8<<24),	// 10    : 16x8
		PB_8x16 |(PB_8x16<<8) |(PB_8x16<<16) |(PB_8x16<<24),	// 11    : 8x16
		PB_16x8 |(PB_16x8<<8) |(PB_16x8<<16) |(PB_16x8<<24),	// 12    : 16x8
		PB_8x16 |(PB_8x16<<8) |(PB_8x16<<16) |(PB_8x16<<24),	// 13    : 8x16
		PB_16x8 |(PB_16x8<<8) |(PB_16x8<<16) |(PB_16x8<<24),	// 14    : 16x8
		PB_8x16 |(PB_8x16<<8) |(PB_8x16<<16) |(PB_8x16<<24),	// 15    : 8x16
		PB_16x8 |(PB_16x8<<8) |(PB_16x8<<16) |(PB_16x8<<24),	// 16    : 16x8
		PB_8x16 |(PB_8x16<<8) |(PB_8x16<<16) |(PB_8x16<<24),	// 17    : 8x16
		PB_16x8 |(PB_16x8<<8) |(PB_16x8<<16) |(PB_16x8<<24),	// 18    : 16x8
		PB_8x16 |(PB_8x16<<8) |(PB_8x16<<16) |(PB_8x16<<24),	// 19    : 8x16
		PB_16x8 |(PB_16x8<<8) |(PB_16x8<<16) |(PB_16x8<<24),	// 20    : 16x8
		PB_8x16 |(PB_8x16<<8) |(PB_8x16<<16) |(PB_8x16<<24),	// 21    : 8x16
		0,												// 22    :  8x8
		IBLOCK|(IBLOCK<<8)|(IBLOCK<<16)|(IBLOCK<<24),	// 23
		0, 0, 0, 0,										// 24-27 :
		0, 0, 0, 0,										// 28-31 :
		0, 0, 0, 0,										// 32-35 :
		0, 0, 0, 0,										// 36-39 :
		0, 0, 0, 0,										// 40-43 :
		0, 0, 0, 0,										// 44-47 :
		0												// 48    :
	};
	static const int b8pdirs[] =  { 
		0x02020202,										//  0    : direct
		0x00000000, 0x01010101, 0x02020202,				//  1- 3 : 16x16
		0x00000000, 0x00000000, 0x01010101, 0x01010101,	//  4- 7
		0x01010000, 0x01000100, 0x00000101, 0x00010001,	//  8-11
		0x02020000, 0x02000200, 0x02020101, 0x02010201,	// 12-15
		0x00000202, 0x00020002, 0x01010202, 0x01020102,	// 16-19
		0x02020202, 0x02020202,							// 20-21 : 16x8, 8x16 interleaved
		0,												// 22    :  8x8
		-1,												// 23    :
		-1, -1, -1, -1,									// 24-27 :
		-1, -1, -1, -1,									// 28-31 :
		-1, -1, -1, -1,									// 32-35 :
		-1, -1, -1, -1,									// 36-39 :
		-1, -1, -1, -1,									// 40-43 :
		-1, -1, -1, -1,									// 44-47 :
		-1												// 48    :
	};
	static const int cbps[] =	{ 
		0,			//  0    : direct
		0, 0, 0,	//  1- 3 : 16x16
		0, 0, 0, 0,	//  4- 7
		0, 0, 0, 0,	//  8-11
		0, 0, 0, 0,	// 12-15
		0, 0, 0, 0,	// 16-19
		0, 0,		// 20-21 : 16x8, 8x16 interleaved
		0,			// 22    :  8x8,
		0,			//  0
		0, 0, 0, 0,	//  1- 4
		16,16,16,16,//  5- 8
		32,32,32,32,//  9-12
		15,15,15,15,// 13-16
		31,31,31,31,// 17-20
		47,47,47,47,// 21-24
		-1			// 25
	};
	static const int i16modes[] = { 
		0,			//  0    : direct
		0, 0, 0,	//  1- 3 : 16x16
		0, 0, 0, 0,	//  4- 7
		0, 0, 0, 0,	//  8-11
		0, 0, 0, 0,	// 12-15
		0, 0, 0, 0,	// 16-19
		0, 0,		// 20-21 : 16x8, 8x16 interleaved
		0,			// 22    :  8x8,
		0,			//  0
		0, 1, 2, 3,	//  1- 4
		0, 1, 2, 3,	//  5- 8
		0, 1, 2, 3,	//  9-12
		0, 1, 2, 3,	// 13-16
		0, 1, 2, 3,	// 17-20
		0, 1, 2, 3,	// 21-24
		0			// 25
	};

	if (mbmode > 48) {
		return CREL_WARNING_H264_STREAMERROR_LEVEL_3;
	}

	currMB_s_r->mb_type = currMB_r->mb_type = mb_types[mbmode];
	*(int *) &currMB_r->b8mode[0] = b8modes[mbmode];
	*(int *) &currMB_r->b8pdir[0] = b8pdirs[mbmode];
	currMB_r->cbp                 = cbps[mbmode];
	currMB_r->i16mode             = i16modes[mbmode];

#else
	int i;
	static const char offset2pdir16x16[12]   = {0, 0, 1, 2, 0,0,0,0,0,0,0,0};
	static const char offset2pdir16x8[22][2] = {{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{1,1},{0,0},{0,1},{0,0},{1,0},
	{0,0},{0,2},{0,0},{1,2},{0,0},{2,0},{0,0},{2,1},{0,0},{2,2},{0,0}};
	static const char offset2pdir8x16[22][2] = {{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{1,1},{0,0},{0,1},{0,0},
	{1,0},{0,0},{0,2},{0,0},{1,2},{0,0},{2,0},{0,0},{2,1},{0,0},{2,2}};

	int mbtype  = currMB_r->mb_type;
	byte  *b8mode = currMB_r->b8mode;
	char  *b8pdir = currMB_r->b8pdir;

	//--- set mbtype, b8type, and b8pdir ---
	if (mbtype==0)       // direct
	{
		mbmode=0;       for(i=0;i<4;i++) {b8mode[i]=0;          b8pdir[i]=2; }
	}
	else if (mbtype==23) // intra4x4
	{
		mbmode=I4MB;    for(i=0;i<4;i++) {b8mode[i]=IBLOCK;     b8pdir[i]=-1; }
	}
	else if ((mbtype>23) && (mbtype<48) ) // intra16x16
	{
		mbmode=I16MB;   for(i=0;i<4;i++) {b8mode[i]=0;          b8pdir[i]=-1; }
		currMB_r->cbp     = ICBPTAB[(mbtype-24)>>2];
		currMB_r->i16mode = (mbtype-24) & 0x03;
	}
	else if (mbtype==22) // 8x8(+split)
	{
		mbmode=PB_8x8;       // b8mode and pdir is transmitted in additional codewords
	}
	else if (mbtype<4)   // 16x16
	{
		mbmode=PB_16x16;       for(i=0;i<4;i++) {b8mode[i]=1;          b8pdir[i]=offset2pdir16x16[mbtype]; }
	}
	else if(mbtype==48)
	{
		mbmode=IPCM;
		for (i=0;i<4;i++) {currMB_r->b8mode[i]=0; currMB_r->b8pdir[i]=-1; }
		currMB_r->cbp= -1;
		currMB_r->i16mode = 0;
	}

	else if (mbtype%2==0) // 16x8
	{
		mbmode=PB_16x8;       for(i=0;i<4;i++) {b8mode[i]=2;          b8pdir[i]=offset2pdir16x8 [mbtype][i/2]; }
	}
	else
	{
		mbmode=PB_8x16;       for(i=0;i<4;i++) {b8mode[i]=3;          b8pdir[i]=offset2pdir8x16 [mbtype][i%2]; }
	}
	currMB_r->mb_type = mbmode;
#endif
	return CREL_OK;
}

/*!
************************************************************************
* \brief
*    init macroblock I and P frames
************************************************************************
*/
#if 1
void init_macroblock PARGS0()
{
#if 1
	//IoK - 1/22/2007
	// This segment needs debugging and eventually split into a separate
	// SSE2 code
	// It should be approximately 4 times faster than the C code
	static const __declspec(align(16)) int Neighbour_lookup[8*4*4] = {
		// pMB, mb_addr, x, y
		// currMB_r->mbStatusA = 0, or IMGPAR MbaffFrameFlag = 0
		0, 0, 3, 0,
		0, 0, 3, 1,
		0, 0, 3, 2,
		0, 0, 3, 3,
		// currMB_r->mbStatusA = 1, and IMGPAR MbaffFrameFlag = 1
		0, 0, 3, 0,
		0, 0, 3, 0,
		0, 0, 3, 1,
		0, 0, 3, 1,
		// currMB_r->mbStatusA = 2, and IMGPAR MbaffFrameFlag = 1
		0, 0, 3, 2,
		0, 0, 3, 2,
		0, 0, 3, 3,
		0, 0, 3, 3,
		// currMB_r->mbStatusA = 3, and IMGPAR MbaffFrameFlag = 1
		0, 0, 3, 0,
		0, 0, 3, 2,
		sizeof(Macroblock), 1, 3, 0,
		sizeof(Macroblock), 1, 3, 2,
		// currMB_r->mbStatusA = 4, and IMGPAR MbaffFrameFlag = 1
		0, 0, 3, 0,
		0, 0, 3, 2,
		sizeof(Macroblock), 1, 3, 0,
		sizeof(Macroblock), 1, 3, 2,
		// currMB_r->mbStatusA = 5, and IMGPAR MbaffFrameFlag = 1
		0, 0, 3, 0,
		0, 0, 3, 1,
		0, 0, 3, 2,
		0, 0, 3, 3,
		// currMB_r->mbStatusA = 6, and IMGPAR MbaffFrameFlag = 1
		0, 0, 3, 0,
		0, 0, 3, 1,
		0, 0, 3, 2,
		0, 0, 3, 3,
		// For up
		0, 0, 0, 3,
		0, 0, 1, 3,
		0, 0, 2, 3,
		0, 0, 3, 3
	};
	int nIndex;
	__m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7;

	// Following line allows for debugging of release builds
	// Remember to comment, otherwise code will crash
	//_asm int 3;
	xmm0 = _mm_setzero_si128(); // 0
	// IoK: Following line is useless, but prevents Intel Compiler from failing...
	xmm1 = _mm_setzero_si128(); // 0
	xmm1 = _mm_cmpeq_epi8(xmm1, xmm1); // -1
	_mm_store_si128((__m128i *) &currMB_s_r->pred_info.mv[0][ 0], xmm0);
	_mm_store_si128((__m128i *) &currMB_s_r->pred_info.mv[0][ 4], xmm0);
	_mm_store_si128((__m128i *) &currMB_s_r->pred_info.mv[0][ 8], xmm0);
	_mm_store_si128((__m128i *) &currMB_s_r->pred_info.mv[0][12], xmm0);
	_mm_store_si128((__m128i *) &currMB_s_r->pred_info.mv[1][ 0], xmm0);
	_mm_store_si128((__m128i *) &currMB_s_r->pred_info.mv[1][ 4], xmm0);
	_mm_store_si128((__m128i *) &currMB_s_r->pred_info.mv[1][ 8], xmm0);
	_mm_store_si128((__m128i *) &currMB_s_r->pred_info.mv[1][12], xmm0);
	//xmm0 = _mm_subs_epi8(xmm0, xmm1); // 1
	//xmm0 = _mm_subs_epi8(xmm0, xmm1); // 2
	//_mm_storel_epi64((__m128i *) &currMB_s_r->pred_info.ref_idx[0][0], xmm1);
	_mm_store_si128((__m128i *) &currMB_s_r->pred_info.ref_idx[0][0], xmm1);
	//_mm_store_si128((__m128i *) &currMB_r->ipredmode[0], xmm0);
	//memset(currMB_r->ipredmode, DC_PRED, BLOCK_SIZE*BLOCK_SIZE);

	// reset vectors and pred. modes
	//memset(currMB_s_r->pred_info.mv, 0, 2*BLOCK_SIZE*BLOCK_SIZE*2*sizeof(short));
	//memset(currMB_s_r->pred_info.ref_idx, -1, 2*2*BLOCK_SIZE*BLOCK_SIZE/8);

	nIndex = currMB_r->mbStatusA&(-IMGPAR MbaffFrameFlag);
	xmm0 = _mm_cvtsi32_si128((int) IMGPAR pLeftMB_r);
	xmm1 = _mm_cvtsi32_si128(IMGPAR mbAddrA);
	xmm2 = _mm_cvtsi32_si128((int) IMGPAR pUpMB_r);
	xmm3 = _mm_cvtsi32_si128(IMGPAR mbAddrB);
	xmm1 = _mm_slli_si128(xmm1, 4);
	xmm3 = _mm_slli_si128(xmm3, 4);
	xmm0 = _mm_or_si128(xmm0, xmm1);
	xmm2 = _mm_or_si128(xmm2, xmm3);
	xmm4 = _mm_load_si128((__m128i *) &Neighbour_lookup[nIndex*16+ 0]);
	xmm5 = _mm_load_si128((__m128i *) &Neighbour_lookup[nIndex*16+ 4]);
	xmm6 = _mm_load_si128((__m128i *) &Neighbour_lookup[nIndex*16+ 8]);
	xmm7 = _mm_load_si128((__m128i *) &Neighbour_lookup[nIndex*16+12]);
	xmm4 = _mm_add_epi32(xmm4, xmm0);
	xmm5 = _mm_add_epi32(xmm5, xmm0);
	xmm6 = _mm_add_epi32(xmm6, xmm0);
	xmm7 = _mm_add_epi32(xmm7, xmm0);
	_mm_store_si128((__m128i *) &IMGPAR left[0], xmm4);
	_mm_store_si128((__m128i *) &IMGPAR left[1], xmm5);
	_mm_store_si128((__m128i *) &IMGPAR left[2], xmm6);
	_mm_store_si128((__m128i *) &IMGPAR left[3], xmm7);
	/*
	xmm4 = _mm_load_si128((__m128i *) &Neighbour_lookup[7*16+ 0]);
	xmm5 = _mm_load_si128((__m128i *) &Neighbour_lookup[7*16+ 4]);
	xmm6 = _mm_load_si128((__m128i *) &Neighbour_lookup[7*16+ 8]);
	xmm7 = _mm_load_si128((__m128i *) &Neighbour_lookup[7*16+12]);
	xmm4 = _mm_add_epi32(xmm4, xmm2);
	xmm5 = _mm_add_epi32(xmm5, xmm2);
	xmm6 = _mm_add_epi32(xmm6, xmm2);
	xmm7 = _mm_add_epi32(xmm7, xmm2);
	_mm_store_si128((__m128i *) &IMGPAR up[0], xmm4);
	_mm_store_si128((__m128i *) &IMGPAR up[1], xmm5);
	_mm_store_si128((__m128i *) &IMGPAR up[2], xmm6);
	_mm_store_si128((__m128i *) &IMGPAR up[3], xmm7);
	*/

#else
	static const int NeighbourA_block_y[7][4] = {
		{ 0, 1, 2, 3},
		{ 0, 0, 1, 1},
		{ 2, 2, 3, 3},
		{ 0, 2, 0, 2},
		{ 0, 2, 0, 2},
		{ 0, 1, 2, 3},
		{ 0, 1, 2, 3}
	};
	static PixelPos Template;

	//memset(currMB_r->ipredmode, DC_PRED, BLOCK_SIZE*BLOCK_SIZE);

	// reset vectors and pred. modes
	memset(currMB_s_r->pred_info.mv, 0, 2*BLOCK_SIZE*BLOCK_SIZE*2*sizeof(short));
	// We set both ref_idx & ref_pic_id to -1 at the same time
	memset(currMB_s_r->pred_info.ref_idx, -1, 2*2*BLOCK_SIZE*BLOCK_SIZE/4);
	//memset(currMB_s_r->pred_info.ref_idx, -1, 2*BLOCK_SIZE*BLOCK_SIZE/4);	
	//memset(currMB_s_r->pred_info.ref_pic_id, 0xff, 2*BLOCK_SIZE*BLOCK_SIZE/4);

	if (IMGPAR MbaffFrameFlag)
	{
		int nIndex;

		nIndex = currMB_r->mbStatusA;
		Template.pMB     = IMGPAR pLeftMB_r;
		Template.mb_addr = IMGPAR mbAddrA;
		Template.x       = 3;
		Template.y       = NeighbourA_block_y[nIndex][0];
		memcpy(&IMGPAR left[0], &Template, sizeof(Template));
		Template.y       = NeighbourA_block_y[nIndex][1];
		memcpy(&IMGPAR left[1], &Template, sizeof(Template));
		if(nIndex==3 || nIndex==4)
		{
			Template.pMB++;
			Template.mb_addr++;
		}
		Template.y = NeighbourA_block_y[nIndex][2];
		memcpy(&IMGPAR left[2], &Template, sizeof(Template));
		Template.y = NeighbourA_block_y[nIndex][3];
		memcpy(&IMGPAR left[3], &Template, sizeof(Template));
	}
	else
	{
		Template.pMB     = IMGPAR pLeftMB_r;
		Template.mb_addr = IMGPAR mbAddrA;
		Template.x       = 3;
		Template.y       = 0;
		memcpy(&IMGPAR left[0], &Template, sizeof(Template));
		Template.y++;
		memcpy(&IMGPAR left[1], &Template, sizeof(Template));
		Template.y++;
		memcpy(&IMGPAR left[2], &Template, sizeof(Template));
		Template.y++;
		memcpy(&IMGPAR left[3], &Template, sizeof(Template));
	}

	/*
	Template.pMB     = IMGPAR pUpMB_r;
	Template.mb_addr = IMGPAR mbAddrB;
	Template.x       = 0;
	Template.y       = 3;
	memcpy(&IMGPAR up[0], &Template, sizeof(Template));
	Template.x++;
	memcpy(&IMGPAR up[1], &Template, sizeof(Template));
	Template.x++;
	memcpy(&IMGPAR up[2], &Template, sizeof(Template));
	Template.x++;
	memcpy(&IMGPAR up[3], &Template, sizeof(Template));
	*/
#endif
}
#else
void init_macroblock PARGS0()
{
	int nIndex;
	static const int NeighbourA_block_y[7][4] = {
		{ 0, 1, 2, 3},
		{ 0, 0, 1, 1},
		{ 2, 2, 3, 3},
		{ 0, 2, 0, 2},
		{ 0, 2, 0, 2},
		{ 0, 1, 2, 3},
		{ 0, 1, 2, 3}
	};

	//memset(currMB_r->ipredmode, DC_PRED, BLOCK_SIZE*BLOCK_SIZE);

	// reset vectors and pred. modes
	memset(currMB_s_r->pred_info.mv, 0, 2*BLOCK_SIZE*BLOCK_SIZE*2*sizeof(short));
	memset(currMB_s_r->pred_info.ref_idx, -1, 2*2*BLOCK_SIZE*BLOCK_SIZE/4);
	//memset(currMB_s_r->pred_info.ref_idx, -1, 2*BLOCK_SIZE*BLOCK_SIZE/4);	
	//memset(currMB_s_r->pred_info.ref_pic_id, 0xff, 2*BLOCK_SIZE*BLOCK_SIZE/4);

	if (IMGPAR MbaffFrameFlag)
	{
		nIndex = currMB_r->mbStatusA;
		IMGPAR left[0].mb_addr = IMGPAR left[1].mb_addr = IMGPAR mbAddrA;
		IMGPAR left[0].pMB = IMGPAR left[1].pMB = IMGPAR pLeftMB_r;
		IMGPAR left[0].x = 3;
		IMGPAR left[0].y = NeighbourA_block_y[nIndex][0];		
		IMGPAR left[1].x = 3;
		IMGPAR left[1].y = NeighbourA_block_y[nIndex][1];
		IMGPAR left[2].mb_addr = IMGPAR left[3].mb_addr = (nIndex==3 || nIndex==4) ? IMGPAR mbAddrA+1 : IMGPAR mbAddrA;
		IMGPAR left[2].pMB = IMGPAR left[3].pMB = (nIndex==3 || nIndex==4) ? IMGPAR pLeftMB_r+1 : IMGPAR pLeftMB_r;
		IMGPAR left[2].x = 3;
		IMGPAR left[2].y = NeighbourA_block_y[nIndex][2];		
		IMGPAR left[3].x = 3;
		IMGPAR left[3].y = NeighbourA_block_y[nIndex][3];

		nIndex = IMGPAR mbAddrB;
		IMGPAR up[0].mb_addr = IMGPAR up[1].mb_addr = IMGPAR up[2].mb_addr = IMGPAR up[3].mb_addr = nIndex; // Could be removed in future
		IMGPAR up[0].pMB = IMGPAR up[1].pMB = IMGPAR up[2].pMB = IMGPAR up[3].pMB = IMGPAR pUpMB_r;
		IMGPAR up[0].x = 0;
		IMGPAR up[0].y = 3;

		IMGPAR up[1].x = 1;
		IMGPAR up[1].y = 3;

		IMGPAR up[2].x = 2;
		IMGPAR up[2].y = 3;

		IMGPAR up[3].x = 3;
		IMGPAR up[3].y = 3;
	}
	else
	{
		nIndex = IMGPAR mbAddrA;
		IMGPAR left[0].mb_addr = IMGPAR left[1].mb_addr = IMGPAR left[2].mb_addr = IMGPAR left[3].mb_addr = nIndex; // Could be removed in future
		IMGPAR left[0].pMB = IMGPAR left[1].pMB = IMGPAR left[2].pMB = IMGPAR left[3].pMB = IMGPAR pLeftMB_r;
		IMGPAR left[0].x = 3;
		IMGPAR left[0].y = 0;

		IMGPAR left[1].x = 3;
		IMGPAR left[1].y = 1;

		IMGPAR left[2].x = 3;
		IMGPAR left[2].y = 2;

		IMGPAR left[3].x = 3;
		IMGPAR left[3].y = 3;

		nIndex = IMGPAR mbAddrB;
		IMGPAR up[0].mb_addr = IMGPAR up[1].mb_addr = IMGPAR up[2].mb_addr = IMGPAR up[3].mb_addr = nIndex;	// Could be removed in future
		IMGPAR up[0].pMB = IMGPAR up[1].pMB = IMGPAR up[2].pMB = IMGPAR up[3].pMB = IMGPAR pUpMB_r;

		IMGPAR up[0].x = 0;
		IMGPAR up[0].y = 3;

		IMGPAR up[1].x = 1;
		IMGPAR up[1].y = 3;

		IMGPAR up[2].x = 2;
		IMGPAR up[2].y = 3;

		IMGPAR up[3].x = 3;
		IMGPAR up[3].y = 3;
	}
}
#endif

static const char p_v2b8pd [ 8][2] = {{4,0}, {5,0}, {6,0}, {7,0}, {IBLOCK,-1}, {-1,-1}, {-1,-1}, {-1,-1}};
static const char b_v2b8pd [16][2] = {{0,2}, {4,0}, {4,1}, {4,2}, {5,0}, {6,0}, {5,1}, {6,1},
{5,2}, {6,2}, {7,0}, {7,1}, {7,2}, {IBLOCK,-1}, {-1,-1}, {-1,-1}};
void SelectB8Mode_P PARGS0()
{
	v2b8pd = (char *) &p_v2b8pd[0][0];
}

void SelectB8Mode_B PARGS0()
{
	v2b8pd = (char *) &b_v2b8pd[0][0];
}

/*!
************************************************************************
* \brief
*    Get the syntax elements from the NAL
************************************************************************
*/
#if 1
#if defined (_COLLECT_PIC_)
CREL_RETURN read_one_macroblock_UVLC_I_odd PARGS0()
{
	int mb_nr = IMGPAR current_mb_nr_r;	
	CREL_RETURN ret;
#ifdef EI_FLAG  
	DecodingEnvironment *dep;
	dep = &(IMGPAR g_dep);
#endif
	g_read_functions = &(IMGPAR currentSlice->readSyntax);

#ifdef _COLLECT_PIC_
	currMB_s_r->do_record = 0;
#endif

	//currMB_r->mb_field = IMGPAR mb_decdata[mb_nr-1].mb_field;
	if ( IMGPAR MbaffFrameFlag ) {
		currMB_r->mb_field = IMGPAR pLastMB_r->mb_field;
	} else {
		currMB_r->mb_field  = ( IMGPAR currentSlice->structure != FRAME );
	}

	//currMB_s_r->qp = IMGPAR qp ;

	//  read MB type
	currMB_r->mb_type = read_raw_mb_uvlc ARGS0();
#ifdef EI_FLAG  
	if(!dep->Dei_flag)
		currMB_r->ei_flag = 0;
#endif

	currMB_s_r->mb_field = currMB_r->mb_field;
	CheckAvailabilityOfNeighbors_ABCD_odd ARGS0();

	if (IMGPAR currentSlice->LFDisableIdc==2)
	{
		// don't filter at slice boundaries
		currMB_s_r->filterLeftMbEdgeFlag = (IMGPAR pLeftMB_r != NULL);
		// if this the bottom of a frame macroblock pair then always filter the top edge
		currMB_s_r->filterTopMbEdgeFlag  = IMGPAR pUpMB_r || (IMGPAR MbaffFrameFlag && !currMB_s_r->mb_field);
	}

	ret = interpret_mb_mode_I ARGS0();
	if (FAILED(ret)) {
		return ret;
	}

	if(IMGPAR MbaffFrameFlag && currMB_r->mb_field)
	{
		IMGPAR num_ref_idx_l0_active <<=1;
		IMGPAR num_ref_idx_l1_active <<=1;
	}

	currMB_r->intra_block = 1;

	currMB_r->NoMbPartLessThan8x8Flag = 1;

	//============= Transform Size Flag for INTRA MBs =============
	//-------------------------------------------------------------
	//transform size flag for INTRA_4x4 and INTRA_8x8 modes
	if (currMB_r->mb_type == I4MB && IMGPAR Transform8x8Mode)
	{
		currMB_r->luma_transform_size_8x8_flag = currMB_s_r->luma_transform_size_8x8_flag =
			read_transform_size_uvlc ARGS0();

		if (currMB_r->luma_transform_size_8x8_flag)
		{
			currMB_r->mb_type = I8MB;
			*(int *) &currMB_r->b8mode[0] = I8MB | (I8MB<<8) | (I8MB<<16) | (I8MB<<24);
			*(int *) &currMB_r->b8pdir[0] = (int) -1;
		}
	}
	else
	{
		currMB_r->luma_transform_size_8x8_flag = currMB_s_r->luma_transform_size_8x8_flag = 0;
	}

	//--- init macroblock data ---
	init_macroblock ARGS0();

	if(currMB_r->mb_type!=IPCM)
	{
		// intra prediction modes for a macroblock 4x4 **********************************************
		g_read_functions->read_ipred_modes ARGS0();

		// read CBP and Coeffs  ***************************************************************
		readCBPandCoeffsFromNAL_UVLC ARGS0();
	}
	else
	{
		//read pcm_alignment_zero_bit and pcm_byte[i]

		// here dep is assigned with the same dep as SE_MBTYPE, because IPCM syntax is in the
		// same category as MBTYPE
		readIPCMcoeffsFromNAL_UVLC ARGS0();
#ifdef _COLLECT_PIC_
		memset(IMGPAR nz_coeff + IMGPAR current_mb_nr_r, 16, sizeof(IMGPAR nz_coeff[0]));
#endif
	}

	return CREL_OK;
}
CREL_RETURN read_one_macroblock_UVLC_I_even PARGS0()
{
	int mb_nr = IMGPAR current_mb_nr_r;	
	CREL_RETURN ret;
#ifdef EI_FLAG  
	DecodingEnvironment *dep;
	dep = &(IMGPAR g_dep);
#endif
	g_read_functions = &(IMGPAR currentSlice->readSyntax);

#ifdef _COLLECT_PIC_
	currMB_s_r->do_record = 0;
#endif

	//currMB_r->mb_field = 0;

	//currMB_s_r->qp = IMGPAR qp ;

	//  read MB mode *****************************************************************
	// read MB aff
	if (IMGPAR MbaffFrameFlag)
		currMB_r->mb_field = readSyntaxElement_FLC ARGS1(1);
	else
		currMB_r->mb_field  = ( IMGPAR currentSlice->structure != FRAME );

	//  read MB type
	currMB_r->mb_type = read_raw_mb_uvlc ARGS0();

#ifdef EI_FLAG  
	if(!dep->Dei_flag)
		currMB_r->ei_flag = 0;
#endif

	currMB_s_r->mb_field = currMB_r->mb_field;

	CheckAvailabilityOfNeighbors_ABCD_even ARGS0();

	if (IMGPAR currentSlice->LFDisableIdc==2)
	{
		// don't filter at slice boundaries
		currMB_s_r->filterLeftMbEdgeFlag = (IMGPAR pLeftMB_r != NULL);
		// if this the bottom of a frame macroblock pair then always filter the top edge
		currMB_s_r->filterTopMbEdgeFlag  = (IMGPAR pUpMB_r != NULL);
	}

	ret = interpret_mb_mode_I ARGS0();
	if (FAILED(ret)) {
		return ret;
	}

	if(IMGPAR MbaffFrameFlag && currMB_r->mb_field)
	{
		IMGPAR num_ref_idx_l0_active <<=1;
		IMGPAR num_ref_idx_l1_active <<=1;
	}

	currMB_r->intra_block = 1;

	currMB_r->NoMbPartLessThan8x8Flag = 1;

	//============= Transform Size Flag for INTRA MBs =============
	//-------------------------------------------------------------
	//transform size flag for INTRA_4x4 and INTRA_8x8 modes
	if (currMB_r->mb_type == I4MB && IMGPAR Transform8x8Mode)
	{
		currMB_r->luma_transform_size_8x8_flag = currMB_s_r->luma_transform_size_8x8_flag =
			read_transform_size_uvlc ARGS0();

		if (currMB_r->luma_transform_size_8x8_flag)
		{
			currMB_r->mb_type = I8MB;
			*(int *) &currMB_r->b8mode[0] = I8MB | (I8MB<<8) | (I8MB<<16) | (I8MB<<24);
			*(int *) &currMB_r->b8pdir[0] = (int) -1;
		}
	}
	else
	{
		currMB_r->luma_transform_size_8x8_flag = currMB_s_r->luma_transform_size_8x8_flag = 0;
	}

	//--- init macroblock data ---
	init_macroblock ARGS0();

	if(currMB_r->mb_type!=IPCM)
	{
		// intra prediction modes for a macroblock 4x4 **********************************************
		g_read_functions->read_ipred_modes ARGS0();
		// read CBP and Coeffs  ***************************************************************
		readCBPandCoeffsFromNAL_UVLC ARGS0();
	}
	else
	{
		//read pcm_alignment_zero_bit and pcm_byte[i]

		// here dep is assigned with the same dep as SE_MBTYPE, because IPCM syntax is in the
		// same category as MBTYPE
		readIPCMcoeffsFromNAL_UVLC ARGS0();
#ifdef _COLLECT_PIC_
		memset(IMGPAR nz_coeff + IMGPAR current_mb_nr_r, 16, sizeof(IMGPAR nz_coeff[0]));
#endif
	}

	return CREL_OK;
}
#else
CREL_RETURN read_one_macroblock_UVLC_I PARGS0()
{
	int mb_nr = IMGPAR current_mb_nr_r;	
#ifdef EI_FLAG  
	DecodingEnvironment *dep;
	dep = &(IMGPAR g_dep);
#endif
	g_read_functions = &(IMGPAR currentSlice->readSyntax);

#ifdef _COLLECT_PIC_
	currMB_s_r->do_record = 0;
#endif

	//currMB_s_r->qp = IMGPAR qp ;

	//  read MB mode *****************************************************************
	// read MB aff
	if ( IMGPAR MbaffFrameFlag ) {
		if (mb_nr&1)
			currMB_r->mb_field = IMGPAR pLastMB_r->mb_field;
		else
			currMB_r->mb_field = readSyntaxElement_FLC ARGS1(1);
	} else {
		currMB_r->mb_field  = ( IMGPAR currentSlice->structure != FRAME );
	}

	//  read MB type
	currMB_r->mb_type = read_raw_mb_uvlc ARGS0();
#ifdef EI_FLAG  
	if(!dep->Dei_flag)
		currMB_r->ei_flag = 0;
#endif

	currMB_s_r->mb_field = currMB_r->mb_field;
	if (mb_nr&1)
		CheckAvailabilityOfNeighbors_ABCD_odd ARGS0();
	else
		CheckAvailabilityOfNeighbors_ABCD_even ARGS0();

	if (IMGPAR currentSlice->LFDisableIdc==2)
	{
		// don't filter at slice boundaries
		currMB_s_r->filterLeftMbEdgeFlag = (IMGPAR pLeftMB_r != NULL);
		// if this the bottom of a frame macroblock pair then always filter the top edge
		currMB_s_r->filterTopMbEdgeFlag  = IMGPAR pUpMB_r || (IMGPAR MbaffFrameFlag && !currMB_s_r->mb_field && (mb_nr & 1));
	}

	ret = interpret_mb_mode_I ARGS0();
	if (FAILED(ret)) {
		return ret;
	}

	if(IMGPAR MbaffFrameFlag && currMB_r->mb_field)
	{
		IMGPAR num_ref_idx_l0_active <<=1;
		IMGPAR num_ref_idx_l1_active <<=1;
	}

	currMB_r->intra_block = 1;

	currMB_r->NoMbPartLessThan8x8Flag = 1;

	//============= Transform Size Flag for INTRA MBs =============
	//-------------------------------------------------------------
	//transform size flag for INTRA_4x4 and INTRA_8x8 modes
	if (currMB_r->mb_type == I4MB && IMGPAR Transform8x8Mode)
	{
		currMB_r->luma_transform_size_8x8_flag = currMB_s_r->luma_transform_size_8x8_flag =
			read_transform_size_uvlc ARGS0();

		if (currMB_r->luma_transform_size_8x8_flag)
		{
			currMB_r->mb_type = I8MB;
			*(int *) &currMB_r->b8mode[0] = I8MB | (I8MB<<8) | (I8MB<<16) | (I8MB<<24);
			*(int *) &currMB_r->b8pdir[0] = (int) -1;
		}
	}
	else
	{
		currMB_r->luma_transform_size_8x8_flag = currMB_s_r->luma_transform_size_8x8_flag = 0;
	}

	//--- init macroblock data ---
	init_macroblock       ARGS0();

	if(currMB_r->mb_type!=IPCM)
	{
		// intra prediction modes for a macroblock 4x4 **********************************************
		g_read_functions->read_ipred_modes ARGS0();

		// read CBP and Coeffs  ***************************************************************
		readCBPandCoeffsFromNAL_UVLC ARGS0();
	}
	else
	{
		//read pcm_alignment_zero_bit and pcm_byte[i]

		// here dep is assigned with the same dep as SE_MBTYPE, because IPCM syntax is in the
		// same category as MBTYPE
		readIPCMcoeffsFromNAL_UVLC ARGS0();
#ifdef _COLLECT_PIC_
		memset(IMGPAR nz_coeff + IMGPAR current_mb_nr_r, 16, sizeof(IMGPAR nz_coeff[0]));
#endif
	}

	return CREL_OK;
}
#endif

#if defined (_COLLECT_PIC_)
CREL_RETURN read_one_macroblock_UVLC_P_odd PARGS0()
{
	int mb_nr = IMGPAR current_mb_nr_r;	
#ifdef EI_FLAG  
	DecodingEnvironment *dep;
	dep = &(IMGPAR g_dep);
#endif
	CREL_RETURN ret;

	g_read_functions = &(IMGPAR currentSlice->readSyntax);

#ifdef _COLLECT_PIC_
	currMB_s_r->do_record = 0;
#endif

	if ( IMGPAR MbaffFrameFlag ) {
		currMB_r->mb_field = IMGPAR pLastMB_r->mb_field;
	} else {
		currMB_r->mb_field  = ( IMGPAR currentSlice->structure != FRAME );
		//currMB_r->mb_field = 0;
	}

	//currMB_s_r->qp = IMGPAR qp ;

	//  read MB mode *****************************************************************
	if(IMGPAR cod_counter == -1)
	{
		IMGPAR cod_counter = readSyntaxElement_VLC_ue ARGS0();
	}
	if (IMGPAR cod_counter==0)
	{
		// read MB aff
		if (IMGPAR MbaffFrameFlag && ((IMGPAR type!=B_SLICE && IMGPAR pLastMB_r->mb_type==0)))
		{
			currMB_r->mb_field = readSyntaxElement_FLC ARGS1(1);
		}

		// read MB type
		currMB_r->mb_type = readSyntaxElement_VLC_ue ARGS0();
		currMB_r->mb_type++;

#ifdef EI_FLAG  
		if(!dep->Dei_flag)
			currMB_r->ei_flag = 0;
#endif
		IMGPAR cod_counter--;
		currMB_r->skip_flag = 0;
	}
	else
	{
		IMGPAR cod_counter--;
		currMB_r->mb_type = 0;
#ifdef EI_FLAG
		currMB_r->ei_flag = 0;
#endif
		currMB_r->skip_flag = 1;
	}

	currMB_s_r->mb_field = currMB_r->mb_field;

	CheckAvailabilityOfNeighbors_ABCD_odd ARGS0(); 

	if (IMGPAR currentSlice->LFDisableIdc==2)
	{
		// don't filter at slice boundaries
		currMB_s_r->filterLeftMbEdgeFlag = (IMGPAR pLeftMB_r != NULL);
		// if this the bottom of a frame macroblock pair then always filter the top edge
		currMB_s_r->filterTopMbEdgeFlag  = IMGPAR pUpMB_r || (IMGPAR MbaffFrameFlag && !currMB_s_r->mb_field);
	}

	ret = interpret_mb_mode_P ARGS0();
	if (FAILED(ret)) {
		return ret;
	}

	if(IMGPAR MbaffFrameFlag && currMB_r->mb_field)
	{
		IMGPAR num_ref_idx_l0_active <<=1;
		IMGPAR num_ref_idx_l1_active <<=1;
	}

	if ((currMB_r->mb_type&INTRA_MODE) == 0)
	{
		currMB_r->NoMbPartLessThan8x8Flag = 1;

		currMB_r->luma_transform_size_8x8_flag = currMB_s_r->luma_transform_size_8x8_flag = 0;

		if(active_pps.constrained_intra_pred_flag)        // inter frame
		{
			currMB_r->intra_block = 0;
		}

		//--- init macroblock data ---
		init_macroblock ARGS0();

		//====== READ 8x8 SUB-PARTITION MODES (modes of 8x8 blocks) and Intra VBST block modes ======
		if (IS_P8x8 (currMB_r))
		{
			for (int i=0; i<4; i++)
			{
				int sub_mb_type = readSyntaxElement_VLC_ue ARGS0();
				currMB_r->b8mode[i]   = reinterpret_cast<unsigned char(*)[2]>(v2b8pd)[sub_mb_type][0];
				currMB_r->b8pdir[i]   = reinterpret_cast<unsigned char(*)[2]>(v2b8pd)[sub_mb_type][1];

				//set NoMbPartLessThan8x8Flag for PB_8x8 mode
				currMB_r->NoMbPartLessThan8x8Flag &= (currMB_r->b8mode[i]==0 && active_sps.direct_8x8_inference_flag) || 
					(currMB_r->b8mode[i]==4);
			}

			ret = readMotionInfoFromNAL ARGS0();
			if (FAILED(ret)) {
				return ret;
			}
			readCBPandCoeffsFromNAL_UVLC ARGS0();
		}
		else if (currMB_r->mb_type == 0) //COPY mode
		{
			MotionVector pmv;
			int zeroMotionAbove;
			int zeroMotionLeft;
			PixelPos mb_a;
			PixelPos* p_mb_a = &mb_a;
			Macroblock_s *mbs_a, *mbs_b;
			int      a_mv_y = 0;
			int      a_ref_idx = 0;
			int      b_mv_y = 0;
			int      b_ref_idx = 0;
			int      list_offset = ((IMGPAR MbaffFrameFlag)&&(currMB_r->mb_field))? 4 : 0;

			p_mb_a = &(IMGPAR left[0]);

			if (p_mb_a->pMB)
			{
				mbs_a     = &dec_picture->mb_data[p_mb_a->mb_addr];
				a_mv_y    = mbs_a->pred_info.mv[LIST_0][p_mb_a->y*4+p_mb_a->x].y;
				a_ref_idx = mbs_a->pred_info.ref_idx[LIST_0][l_16_4[p_mb_a->y*4+p_mb_a->x]];

				if (currMB_s_r->mb_field ^ mbs_a->mb_field)
				{
					if(currMB_s_r->mb_field)
					{
						a_mv_y   /= 2;
						a_ref_idx <<= 1;
					}
					else
					{
						a_mv_y   <<= 1;
						a_ref_idx >>= 1;
					}
				}
			}

			if (IMGPAR pUpMB_r)
			{
				mbs_b     = &dec_picture->mb_data[IMGPAR mbAddrB];
				b_mv_y    = mbs_b->pred_info.mv[LIST_0][3*4+0].y;
				b_ref_idx = mbs_b->pred_info.ref_idx[LIST_0][l_16_4[3*4+0]]; 

				if (currMB_s_r->mb_field ^ mbs_b->mb_field)
				{
					if(currMB_s_r->mb_field)
					{
						b_mv_y    /= 2;
						b_ref_idx  <<= 1;
					}
					else
					{
						b_mv_y    <<= 1;
						b_ref_idx  >>= 1;
					}
				}
			}

			zeroMotionLeft  = p_mb_a->pMB ? (a_ref_idx==0 && mbs_a->pred_info.mv[LIST_0][p_mb_a->y*4+p_mb_a->x].x==0 && a_mv_y==0 ? 1 : 0) : 1;
			zeroMotionAbove = IMGPAR pUpMB_r ? (b_ref_idx==0 && mbs_b->pred_info.mv[LIST_0][3*4+0].x==0 && b_mv_y==0 ? 1 : 0) : 1;

			currMB_r->cbp = 0;
			memset(IMGPAR nz_coeff+mb_nr, 0, sizeof(IMGPAR nz_coeff[0]));

			if (zeroMotionAbove || zeroMotionLeft)
			{
				memset(currMB_s_r->pred_info.mv[0], 0, sizeof(currMB_s_r->pred_info.mv[0]));
			}
			else
			{
				SetMotionVectorPredictor_block00_shape16x16 ARGS6(&pmv,0, LIST_0, IMGPAR current_mb_nr_r, currMB_r, currMB_s_r);

				Set16MotionVector(currMB_s_r->pred_info.mv[0], pmv);
			}

			// Intel compiler 9.0 has fixed this
			memset(&(currMB_s_r->pred_info.ref_idx[0][0]),0,4);
#if !defined(_COLLECT_PIC_)
			memset(&(currMB_s_r->pred_info.ref_pic_id[0][0]),(char) listX[LIST_0+list_offset][0]->unique_id,4);
#endif
		}
		else
		{
			if (IS_INTERMV (currMB_r)) {
				ret = readMotionInfoFromNAL ARGS0();
				if (FAILED(ret)) {
					return ret;
				}
			}

			readCBPandCoeffsFromNAL_UVLC ARGS0();
		}
	}
	else
	{
		currMB_r->NoMbPartLessThan8x8Flag = 1;
		currMB_r->intra_block = 1;

		//============= Transform Size Flag for INTRA MBs =============
		//-------------------------------------------------------------
		//transform size flag for INTRA_4x4 and INTRA_8x8 modes
		if (currMB_r->mb_type == I4MB && IMGPAR Transform8x8Mode)
		{
			currMB_r->luma_transform_size_8x8_flag = currMB_s_r->luma_transform_size_8x8_flag =
				read_transform_size_uvlc ARGS0();

			if (currMB_r->luma_transform_size_8x8_flag)
			{
				currMB_r->mb_type = I8MB;
				*(int *) &currMB_r->b8mode[0] = I8MB | (I8MB<<8) | (I8MB<<16) | (I8MB<<24);
				*(int *) &currMB_r->b8pdir[0] = (int) -1;
			}
		}
		else
		{
			currMB_r->luma_transform_size_8x8_flag = currMB_s_r->luma_transform_size_8x8_flag = 0;
		}

		//--- init macroblock data ---
		init_macroblock ARGS0();

		if(currMB_r->mb_type!=IPCM)
		{
			// intra prediction modes for a macroblock 4x4 **********************************************
			g_read_functions->read_ipred_modes ARGS0();
			// read CBP and Coeffs  ***************************************************************
			readCBPandCoeffsFromNAL_UVLC ARGS0();
		}
		else
		{
			//read pcm_alignment_zero_bit and pcm_byte[i]

			// here dep is assigned with the same dep as SE_MBTYPE, because IPCM syntax is in the
			// same category as MBTYPE
			readIPCMcoeffsFromNAL_UVLC ARGS0();
#ifdef _COLLECT_PIC_
			memset(IMGPAR nz_coeff + IMGPAR current_mb_nr_r, 16, sizeof(IMGPAR nz_coeff[0]));
#endif
		}
	}

	return CREL_OK;
}
CREL_RETURN read_one_macroblock_UVLC_P_even PARGS0()
{
	int mb_nr = IMGPAR current_mb_nr_r;	
#ifdef EI_FLAG  
	DecodingEnvironment *dep;
	dep = &(IMGPAR g_dep);
#endif
	CREL_RETURN ret;
	g_read_functions = &(IMGPAR currentSlice->readSyntax);

#ifdef _COLLECT_PIC_
	currMB_s_r->do_record = 0;
#endif

	//currMB_r->mb_field = 0;
	currMB_r->mb_field  = ( IMGPAR currentSlice->structure != FRAME );


	//currMB_s_r->qp = IMGPAR qp ;

	//  read MB mode *****************************************************************
	if(IMGPAR cod_counter == -1)
	{
		IMGPAR cod_counter = readSyntaxElement_VLC_ue ARGS0();
	}
	if (IMGPAR cod_counter==0)
	{
		// read MB aff
		if (IMGPAR MbaffFrameFlag)
			currMB_r->mb_field = readSyntaxElement_FLC ARGS1(1);

		// read MB type
		currMB_r->mb_type = readSyntaxElement_VLC_ue ARGS0();
		currMB_r->mb_type++;
		

#ifdef EI_FLAG  
		if(!dep->Dei_flag)
			currMB_r->ei_flag = 0;
#endif
		IMGPAR cod_counter--;
		currMB_r->skip_flag = 0;
	}
	else
	{
		IMGPAR cod_counter--;
		currMB_r->mb_type = 0;
#ifdef EI_FLAG
		currMB_r->ei_flag = 0;
#endif
		currMB_r->skip_flag = 1;

		// read field flag of bottom block
		if(IMGPAR MbaffFrameFlag)
		{
			if(IMGPAR cod_counter == 0)
			{
				// The following doesn't advance pointer
				currMB_r->mb_field = peekSyntaxElement_FLC ARGS1(1);
			}
			else if(IMGPAR cod_counter > 0)
			{
				// check left macroblock pair first
				//if (mb_is_available ARGS1(mb_nr-2)&&((mb_nr%(IMGPAR PicWidthInMbs*2))!=0))
				if ( IMGPAR pLeftMB_r )	{
					//currMB_r->mb_field = IMGPAR mb_decdata[mb_nr-2].mb_field;
					currMB_r->mb_field = IMGPAR pLeftMB_r->mb_field;
				} else {
					// check top macroblock pair
					//if (mb_is_available ARGS1(mb_nr-2*IMGPAR PicWidthInMbs))
					if ( IMGPAR pUpMB_r )	{
						//currMB_r->mb_field = IMGPAR mb_decdata[mb_nr-2*IMGPAR PicWidthInMbs].mb_field;
						currMB_r->mb_field = IMGPAR pUpMB_r->mb_field;
					} else {
						currMB_r->mb_field = 0;
					}
				}
			}
		}

	}

	currMB_s_r->mb_field = currMB_r->mb_field;

	CheckAvailabilityOfNeighbors_ABCD_even ARGS0();

	if (IMGPAR currentSlice->LFDisableIdc==2)
	{
		// don't filter at slice boundaries
		currMB_s_r->filterLeftMbEdgeFlag = (IMGPAR pLeftMB_r != NULL);
		// if this the bottom of a frame macroblock pair then always filter the top edge
		currMB_s_r->filterTopMbEdgeFlag  = (IMGPAR pUpMB_r != NULL);
	}

	ret = interpret_mb_mode_P ARGS0();
	if (FAILED(ret)) {
		return ret;
	}

	if(IMGPAR MbaffFrameFlag && currMB_r->mb_field)
	{
		IMGPAR num_ref_idx_l0_active <<=1;
		IMGPAR num_ref_idx_l1_active <<=1;
	}

	if ((currMB_r->mb_type&INTRA_MODE) == 0)
	{
		currMB_r->NoMbPartLessThan8x8Flag = 1;

		currMB_r->luma_transform_size_8x8_flag = currMB_s_r->luma_transform_size_8x8_flag = 0;

		if(active_pps.constrained_intra_pred_flag)        // inter frame
		{
			currMB_r->intra_block = 0;
		}

		//--- init macroblock data ---
		init_macroblock ARGS0();

		//====== READ 8x8 SUB-PARTITION MODES (modes of 8x8 blocks) and Intra VBST block modes ======
		if (IS_P8x8 (currMB_r))
		{
			for (int i=0; i<4; i++)
			{
				int sub_mb_type = readSyntaxElement_VLC_ue ARGS0();
				currMB_r->b8mode[i]   = reinterpret_cast<unsigned char(*)[2]>(v2b8pd)[sub_mb_type][0];
				currMB_r->b8pdir[i]   = reinterpret_cast<unsigned char(*)[2]>(v2b8pd)[sub_mb_type][1];

				//set NoMbPartLessThan8x8Flag for PB_8x8 mode
				currMB_r->NoMbPartLessThan8x8Flag &= (currMB_r->b8mode[i]==0 && active_sps.direct_8x8_inference_flag) || 
					(currMB_r->b8mode[i]==4);
			}

			ret = readMotionInfoFromNAL ARGS0();
			if (FAILED(ret)) {
				return ret;
			}
			readCBPandCoeffsFromNAL_UVLC ARGS0();
		}
		else if (currMB_r->mb_type == 0) //COPY mode
		{
			MotionVector pmv;
			int zeroMotionAbove;
			int zeroMotionLeft;
			PixelPos mb_a;
			PixelPos* p_mb_a = &mb_a;
			Macroblock_s *mbs_a, *mbs_b;
			int      a_mv_y = 0;
			int      a_ref_idx = 0;
			int      b_mv_y = 0;
			int      b_ref_idx = 0;
			int      list_offset = ((IMGPAR MbaffFrameFlag)&&(currMB_r->mb_field))? 2 : 0;

			p_mb_a = &(IMGPAR left[0]);

			if (p_mb_a->pMB)
			{
				mbs_a     = &dec_picture->mb_data[p_mb_a->mb_addr];
				a_mv_y    = mbs_a->pred_info.mv[LIST_0][p_mb_a->y*4+p_mb_a->x].y;
				a_ref_idx = mbs_a->pred_info.ref_idx[LIST_0][l_16_4[p_mb_a->y*4+p_mb_a->x]];

				if (currMB_s_r->mb_field ^ mbs_a->mb_field)
				{
					if(currMB_s_r->mb_field)
					{
						a_mv_y   /= 2;
						a_ref_idx <<= 1;
					}
					else
					{
						a_mv_y   <<= 1;
						a_ref_idx >>= 1;
					}
				}
			}

			if (IMGPAR pUpMB_r)
			{
				mbs_b     = &dec_picture->mb_data[IMGPAR mbAddrB];
				b_mv_y    = mbs_b->pred_info.mv[LIST_0][3*4+0].y;
				b_ref_idx = mbs_b->pred_info.ref_idx[LIST_0][l_16_4[3*4+0]]; 

				if (currMB_s_r->mb_field ^ mbs_b->mb_field)
				{
					if(currMB_s_r->mb_field)
					{
						b_mv_y    /= 2;
						b_ref_idx  <<= 1;
					}
					else
					{
						b_mv_y    <<= 1;
						b_ref_idx  >>= 1;
					}
				}
			}

			zeroMotionLeft  = p_mb_a->pMB ? (a_ref_idx==0 && mbs_a->pred_info.mv[LIST_0][p_mb_a->y*4+p_mb_a->x].x==0 && a_mv_y==0 ? 1 : 0) : 1;
			zeroMotionAbove = IMGPAR pUpMB_r ? (b_ref_idx==0 && mbs_b->pred_info.mv[LIST_0][3*4+0].x==0 && b_mv_y==0 ? 1 : 0) : 1;

			currMB_r->cbp = 0;
			memset(IMGPAR nz_coeff+mb_nr, 0, sizeof(IMGPAR nz_coeff[0]));

			if (zeroMotionAbove || zeroMotionLeft)
			{
				memset(currMB_s_r->pred_info.mv[0], 0, sizeof(currMB_s_r->pred_info.mv[0]));
			}
			else
			{
				SetMotionVectorPredictor_block00_shape16x16 ARGS6(&pmv,0, LIST_0, IMGPAR current_mb_nr_r, currMB_r, currMB_s_r);

				Set16MotionVector(currMB_s_r->pred_info.mv[0], pmv);
			}

			// Intel compiler 9.0 has fixed this
			memset(&(currMB_s_r->pred_info.ref_idx[0][0]),0,4);
#if !defined(_COLLECT_PIC_)
			memset(&(currMB_s_r->pred_info.ref_pic_id[0][0]),(char) listX[LIST_0+list_offset][0]->unique_id,4);
#endif
		}
		else
		{
			if (IS_INTERMV (currMB_r)) {
				ret = readMotionInfoFromNAL ARGS0();
				if (FAILED(ret)) {
					return ret;
				}
			}

			readCBPandCoeffsFromNAL_UVLC ARGS0();
		}
	}
	else
	{
		currMB_r->intra_block = 1;

		currMB_r->NoMbPartLessThan8x8Flag = 1;

		//============= Transform Size Flag for INTRA MBs =============
		//-------------------------------------------------------------
		//transform size flag for INTRA_4x4 and INTRA_8x8 modes
		if (currMB_r->mb_type == I4MB && IMGPAR Transform8x8Mode)
		{
			currMB_r->luma_transform_size_8x8_flag = currMB_s_r->luma_transform_size_8x8_flag =
				read_transform_size_uvlc ARGS0();

			if (currMB_r->luma_transform_size_8x8_flag)
			{
				currMB_r->mb_type = I8MB;
				*(int *) &currMB_r->b8mode[0] = I8MB | (I8MB<<8) | (I8MB<<16) | (I8MB<<24);
				*(int *) &currMB_r->b8pdir[0] = (int) -1;
			}
		}
		else
		{
			currMB_r->luma_transform_size_8x8_flag = currMB_s_r->luma_transform_size_8x8_flag = 0;
		}

		//--- init macroblock data ---
		init_macroblock ARGS0();

		if(currMB_r->mb_type!=IPCM)
		{
			// intra prediction modes for a macroblock 4x4 **********************************************
			g_read_functions->read_ipred_modes ARGS0();
			// read CBP and Coeffs  ***************************************************************
			readCBPandCoeffsFromNAL_UVLC ARGS0();
		}
		else
		{
			//read pcm_alignment_zero_bit and pcm_byte[i]

			// here dep is assigned with the same dep as SE_MBTYPE, because IPCM syntax is in the
			// same category as MBTYPE
			readIPCMcoeffsFromNAL_UVLC ARGS0();
#ifdef _COLLECT_PIC_
			memset(IMGPAR nz_coeff + IMGPAR current_mb_nr_r, 16, sizeof(IMGPAR nz_coeff[0]));
#endif
		}
	}

	return CREL_OK;
}
#else
CREL_RETURN read_one_macroblock_UVLC_P PARGS0()
{
	int mb_nr = IMGPAR current_mb_nr_r;	
#ifdef EI_FLAG  
	DecodingEnvironment *dep;
	dep = &(IMGPAR g_dep);
#endif
	CREL_RETURN ret;
	g_read_functions = &(IMGPAR currentSlice->readSyntax);

#ifdef _COLLECT_PIC_
	currMB_s_r->do_record = 0;
#endif

	if ( IMGPAR MbaffFrameFlag ) {
		if (mb_nr&1)
			currMB_r->mb_field = IMGPAR pLastMB_r->mb_field;
		else
			currMB_r->mb_field  = ( IMGPAR currentSlice->structure != FRAME );
	} else {
		currMB_r->mb_field  = ( IMGPAR currentSlice->structure != FRAME );
	}

	//currMB_s_r->qp = IMGPAR qp ;

	//  read MB mode *****************************************************************
	if(IMGPAR cod_counter == -1)
	{
		IMGPAR cod_counter = readSyntaxElement_VLC_ue ARGS0();
	}
	if (IMGPAR cod_counter==0)
	{
		// read MB aff
		if (IMGPAR MbaffFrameFlag && (((mb_nr&1) == 0) || (IMGPAR type!=B_SLICE && IMGPAR pLastMB_r->mb_type==0)))
		{
			currMB_r->mb_field = readSyntaxElement_FLC ARGS1(1);
		}

		// read MB type
		currMB_r->mb_type = readSyntaxElement_VLC_ue ARGS0();
		currMB_r->mb_type++;

#ifdef EI_FLAG  
		if(!dep->Dei_flag)
			currMB_r->ei_flag = 0;
#endif
		IMGPAR cod_counter--;
		currMB_r->skip_flag = 0;
	}
	else
	{
		IMGPAR cod_counter--;
		currMB_r->mb_type = 0;
#ifdef EI_FLAG
		currMB_r->ei_flag = 0;
#endif
		currMB_r->skip_flag = 1;

		// read field flag of bottom block
		if(IMGPAR MbaffFrameFlag && ((mb_nr&1) == 0))
		{
			if(IMGPAR cod_counter == 0)
			{
				// The following doesn't advance pointer
				currMB_r->mb_field = peekSyntaxElement_FLC ARGS1(1);
			}
			else if(IMGPAR cod_counter > 0)
			{
				// check left macroblock pair first
				if (mb_is_available ARGS1(mb_nr-2)&&((mb_nr%(IMGPAR PicWidthInMbs*2))!=0))
				{
					currMB_r->mb_field = IMGPAR pLeftMB_r->mb_field;//IMGPAR mb_decdata[mb_nr-2].mb_field;
				}
				else
				{
					// check top macroblock pair
					if (mb_is_available ARGS1(mb_nr-2*IMGPAR PicWidthInMbs))
					{
						currMB_r->mb_field = IMGPAR pUpMB_r->mb_field;//IMGPAR mb_decdata[mb_nr-2*IMGPAR PicWidthInMbs].mb_field;
					}
					else
						currMB_r->mb_field = 0;
				}
			}
		}
	}

	currMB_s_r->mb_field = currMB_r->mb_field;
	if (mb_nr&1)
		CheckAvailabilityOfNeighbors_ABCD_odd ARGS0();
	else
		CheckAvailabilityOfNeighbors_ABCD_even ARGS0();

	if (IMGPAR currentSlice->LFDisableIdc==2)
	{
		// don't filter at slice boundaries
		currMB_s_r->filterLeftMbEdgeFlag = (IMGPAR pLeftMB_r != NULL);
		// if this the bottom of a frame macroblock pair then always filter the top edge
		currMB_s_r->filterTopMbEdgeFlag  = IMGPAR pUpMB_r || (IMGPAR MbaffFrameFlag && !currMB_s_r->mb_field && (mb_nr & 1));
	}

	ret = interpret_mb_mode_P ARGS0();
	if (FAILED(ret)) {
		return ret;
	}

	if(IMGPAR MbaffFrameFlag && currMB_r->mb_field)
	{
		IMGPAR num_ref_idx_l0_active <<=1;
		IMGPAR num_ref_idx_l1_active <<=1;
	}

	if ((currMB_r->mb_type&INTRA_MODE) == 0)
	{
		currMB_r->NoMbPartLessThan8x8Flag = 1;

		currMB_r->luma_transform_size_8x8_flag = currMB_s_r->luma_transform_size_8x8_flag = 0;

		if(active_pps.constrained_intra_pred_flag)        // inter frame
		{
			currMB_r->intra_block = 0;
		}

		//--- init macroblock data ---
		init_macroblock       ARGS0();

		//====== READ 8x8 SUB-PARTITION MODES (modes of 8x8 blocks) and Intra VBST block modes ======
		if (IS_P8x8 (currMB_r))
		{
			for (int i=0; i<4; i++)
			{
				int sub_mb_type = readSyntaxElement_VLC_ue ARGS0();
				currMB_r->b8mode[i]   = reinterpret_cast<unsigned char(*)[2]>(v2b8pd)[sub_mb_type][0];
				currMB_r->b8pdir[i]   = reinterpret_cast<unsigned char(*)[2]>(v2b8pd)[sub_mb_type][1];

				//set NoMbPartLessThan8x8Flag for PB_8x8 mode
				currMB_r->NoMbPartLessThan8x8Flag &= (currMB_r->b8mode[i]==0 && active_sps.direct_8x8_inference_flag) || 
					(currMB_r->b8mode[i]==4);
			}

			ret = readMotionInfoFromNAL ARGS0();
			if (FAILED(ret)) {
				return ret;
			}
			readCBPandCoeffsFromNAL_UVLC ARGS0();
		}
		else if (currMB_r->mb_type == 0) //COPY mode
		{
			MotionVector pmv;
			int zeroMotionAbove;
			int zeroMotionLeft;
			PixelPos mb_a;
			PixelPos* p_mb_a = &mb_a;
			Macroblock_s *mbs_a, *mbs_b;
			int      a_mv_y = 0;
			int      a_ref_idx = 0;
			int      b_mv_y = 0;
			int      b_ref_idx = 0;
			int      list_offset = ((IMGPAR MbaffFrameFlag)&&(currMB_r->mb_field))? (mb_nr&1) ? 4 : 2 : 0;

#if !defined(REC_NEIGHBOR)
			getLuma4x4Neighbour ARGS7(mb_nr,0,0,-1, 0,&mb_a, currMB_r);
			getLuma4x4Neighbour ARGS7(mb_nr,0,0, 0,-1,&mb_b, currMB_r);
#else

			p_mb_a = &(IMGPAR left[0]);

			//if (!(IMGPAR left_set[0]))
			//{
			//	getLuma4x4Neighbour_left_out ARGS7(mb_nr, 0, 0, -1,  0, p_mb_a, currMB_r);
			//	IMGPAR left_set[0] = 1;
			//}

			//if (!(IMGPAR up_set[0]))
			//{
			//	getLuma4x4Neighbour_top_out ARGS7(mb_nr, 0, 0,  0, -1, p_mb_b, currMB_r);
			//	IMGPAR up_set[0] = 1;
			//}
#endif

			if (p_mb_a->pMB)
			{
				mbs_a     = &dec_picture->mb_data[p_mb_a->mb_addr];
				a_mv_y    = mbs_a->pred_info.mv[LIST_0][p_mb_a->y*4+p_mb_a->x].y;
				a_ref_idx = mbs_a->pred_info.ref_idx[LIST_0][l_16_4[p_mb_a->y*4+p_mb_a->x]];

				if (currMB_s_r->mb_field ^ mbs_a->mb_field)
				{
					if(currMB_s_r->mb_field)
					{
						a_mv_y   /= 2;
						a_ref_idx <<= 1;
					}
					else
					{
						a_mv_y   <<= 1;
						a_ref_idx >>= 1;
					}
				}
			}

			if (IMGPAR pUpMB_r)
			{
				mbs_b     = &dec_picture->mb_data[IMGPAR mbAddrB];
				b_mv_y    = mbs_b->pred_info.mv[LIST_0][3*4+0].y;
				b_ref_idx = mbs_b->pred_info.ref_idx[LIST_0][l_16_4[3*4+0]]; 

				if (currMB_s_r->mb_field ^ mbs_b->mb_field)
				{
					if(currMB_s_r->mb_field)
					{
						b_mv_y    /= 2;
						b_ref_idx  <<= 1;
					}
					else
					{
						b_mv_y    <<= 1;
						b_ref_idx  >>= 1;
					}
				}
			}

			zeroMotionLeft  = p_mb_a->pMB ? (a_ref_idx==0 && mbs_a->pred_info.mv[LIST_0][p_mb_a->y*4+p_mb_a->x].x==0 && a_mv_y==0 ? 1 : 0) : 1;
			zeroMotionAbove = IMGPAR pUpMB_r ? (b_ref_idx==0 && mbs_b->pred_info.mv[LIST_0][3*4+0].x==0 && b_mv_y==0 ? 1 : 0) : 1;

			currMB_r->cbp = 0;
			memset(IMGPAR nz_coeff+mb_nr, 0, sizeof(IMGPAR nz_coeff[0]));

			if (zeroMotionAbove || zeroMotionLeft)
			{
				memset(currMB_s_r->pred_info.mv[0], 0, sizeof(currMB_s_r->pred_info.mv[0]));
			}
			else
			{
				SetMotionVectorPredictor_block00_shape16x16 ARGS6(&pmv,0, LIST_0, IMGPAR current_mb_nr_r, currMB_r, currMB_s_r);

				Set16MotionVector(currMB_s_r->pred_info.mv[0], pmv);
			}

			// Intel compiler 9.0 has fixed this
			memset(&(currMB_s_r->pred_info.ref_idx[0][0]),0,4);
#if !defined(_COLLECT_PIC_)
			memset(&(currMB_s_r->pred_info.ref_pic_id[0][0]),(char) listX[LIST_0+list_offset][0]->unique_id,4);
#endif
		}
		else
		{
			if (IS_INTERMV (currMB_r)) {
				ret = readMotionInfoFromNAL ARGS0();
				if (FAILED(ret)) {
					return ret;
				}
			}

			readCBPandCoeffsFromNAL_UVLC ARGS0();
		}
	}
	else
	{
		currMB_r->intra_block = 1;

		currMB_r->NoMbPartLessThan8x8Flag = 1;

		//============= Transform Size Flag for INTRA MBs =============
		//-------------------------------------------------------------
		//transform size flag for INTRA_4x4 and INTRA_8x8 modes
		if (currMB_r->mb_type == I4MB && IMGPAR Transform8x8Mode)
		{
			currMB_r->luma_transform_size_8x8_flag = currMB_s_r->luma_transform_size_8x8_flag =
				read_transform_size_uvlc ARGS0();

			if (currMB_r->luma_transform_size_8x8_flag)
			{
				currMB_r->mb_type = I8MB;
				*(int *) &currMB_r->b8mode[0] = I8MB | (I8MB<<8) | (I8MB<<16) | (I8MB<<24);
				*(int *) &currMB_r->b8pdir[0] = (int) -1;
			}
		}
		else
		{
			currMB_r->luma_transform_size_8x8_flag = currMB_s_r->luma_transform_size_8x8_flag = 0;
		}

		//--- init macroblock data ---
		init_macroblock       ARGS0();

		if(currMB_r->mb_type!=IPCM)
		{
			// intra prediction modes for a macroblock 4x4 **********************************************
			g_read_functions->read_ipred_modes ARGS0();

			// read CBP and Coeffs  ***************************************************************
			readCBPandCoeffsFromNAL_UVLC ARGS0();
		}
		else
		{
			//read pcm_alignment_zero_bit and pcm_byte[i]

			// here dep is assigned with the same dep as SE_MBTYPE, because IPCM syntax is in the
			// same category as MBTYPE
			readIPCMcoeffsFromNAL_UVLC ARGS0();
#ifdef _COLLECT_PIC_
			memset(IMGPAR nz_coeff + IMGPAR current_mb_nr_r, 16, sizeof(IMGPAR nz_coeff[0]));
#endif
		}
	}

	return CREL_OK;
}
#endif

#if defined (_COLLECT_PIC_)
CREL_RETURN read_one_macroblock_UVLC_B_odd PARGS0()
{
	int mb_nr = IMGPAR current_mb_nr_r;	
#ifdef EI_FLAG  
	DecodingEnvironment *dep;
	dep = &(IMGPAR g_dep);
#endif
	CREL_RETURN ret;
	g_read_functions = &(IMGPAR currentSlice->readSyntax);

#ifdef _COLLECT_PIC_
	currMB_s_r->do_record = 0;
#endif

	if ( IMGPAR MbaffFrameFlag ) {
		currMB_r->mb_field = IMGPAR pLastMB_r->mb_field;
	} else {
		currMB_r->mb_field  = ( IMGPAR currentSlice->structure != FRAME );
		//currMB_r->mb_field = 0;
	}

	//currMB_s_r->qp = IMGPAR qp ;

	//  read MB mode *****************************************************************
	if(IMGPAR cod_counter == -1)
	{
		IMGPAR cod_counter = readSyntaxElement_VLC_ue ARGS0();
	}
	if (IMGPAR cod_counter==0)
	{
		// read MB aff
		if (IMGPAR MbaffFrameFlag && (IMGPAR type==B_SLICE && IMGPAR pLastMB_r->skip_flag))
		{
			currMB_r->mb_field = readSyntaxElement_FLC ARGS1(1);
		}

		// read MB type
		currMB_r->mb_type = readSyntaxElement_VLC_ue ARGS0();

#ifdef EI_FLAG  
		if(!dep->Dei_flag)
			currMB_r->ei_flag = 0;
#endif
		IMGPAR cod_counter--;
		currMB_r->skip_flag = 0;
	}
	else
	{
		IMGPAR cod_counter--;
		currMB_r->mb_type = 0;
#ifdef EI_FLAG
		currMB_r->ei_flag = 0;
#endif
		currMB_r->skip_flag = 1;
	}

	currMB_s_r->mb_field = currMB_r->mb_field;

	CheckAvailabilityOfNeighbors_ABCD_odd ARGS0();

	if (IMGPAR currentSlice->LFDisableIdc==2)
	{
		// don't filter at slice boundaries
		currMB_s_r->filterLeftMbEdgeFlag = (IMGPAR pLeftMB_r != NULL);
		// if this the bottom of a frame macroblock pair then always filter the top edge
		currMB_s_r->filterTopMbEdgeFlag  = IMGPAR pUpMB_r || (IMGPAR MbaffFrameFlag && !currMB_s_r->mb_field);
	}

	ret = interpret_mb_mode_B ARGS0();
	if (FAILED(ret)) {
		return ret;
	}
	IMGPAR do_co_located = 1;

	if(IMGPAR MbaffFrameFlag && currMB_r->mb_field)
	{
		IMGPAR num_ref_idx_l0_active <<=1;
		IMGPAR num_ref_idx_l1_active <<=1;
	}

	if ((currMB_r->mb_type&INTRA_MODE) == 0)
	{
		
		if(!currMB_r->mb_type)
			compute_colocated_SUBMB ARGS6(Co_located_MB, listX,0,0,4,4);

		currMB_r->NoMbPartLessThan8x8Flag = ((currMB_r->mb_type == 0) && !(active_sps.direct_8x8_inference_flag))? 0: 1;

		currMB_r->luma_transform_size_8x8_flag = currMB_s_r->luma_transform_size_8x8_flag = 0;

		if(active_pps.constrained_intra_pred_flag)
		{
			currMB_r->intra_block = 0;
		}

		//--- init macroblock data ---
		init_macroblock ARGS0();

		//====== READ 8x8 SUB-PARTITION MODES (modes of 8x8 blocks) and Intra VBST block modes ======
		if (IS_P8x8 (currMB_r))
		{
			for (int i=0; i<4; i++)
			{
				int sub_mb_type = readSyntaxElement_VLC_ue ARGS0();
				currMB_r->b8mode[i]   = reinterpret_cast<unsigned char(*)[2]>(v2b8pd)[sub_mb_type][0];
				currMB_r->b8pdir[i]   = reinterpret_cast<unsigned char(*)[2]>(v2b8pd)[sub_mb_type][1];

				if(!currMB_r->b8mode[i])
					compute_colocated_SUBMB ARGS6(Co_located_MB, listX, i&2, ((i&1)<<1), 2, 2);

				//set NoMbPartLessThan8x8Flag for PB_8x8 mode
				currMB_r->NoMbPartLessThan8x8Flag &= (currMB_r->b8mode[i]==0 && active_sps.direct_8x8_inference_flag) || 
					(currMB_r->b8mode[i]==4);
			}

			ret = readMotionInfoFromNAL ARGS0();
			if (FAILED(ret)) {
				return ret;
			}
			readCBPandCoeffsFromNAL_UVLC ARGS0();
		}
		else if ((currMB_r->mb_type == 0) && (IMGPAR cod_counter >= 0)) //DIRECT mode
		{
			currMB_r->cbp = 0;
			memset(IMGPAR nz_coeff+mb_nr, 0, sizeof(IMGPAR nz_coeff[0]));
		}
		else
		{
			if (IS_INTERMV (currMB_r)) {
				ret = readMotionInfoFromNAL ARGS0();
				if (FAILED(ret)) {
					return ret;
				}
			}

			readCBPandCoeffsFromNAL_UVLC ARGS0();
		}

#ifdef _COLLECT_PIC_
		if (IMGPAR do_co_located)
		{
			IMGPAR do_co_located = 0;

			if ((IS_DIRECT (currMB_r) || 
				(IS_P8x8(currMB_r) && !(currMB_r->b8mode[0] && currMB_r->b8mode[1] && currMB_r->b8mode[2] && currMB_r->b8mode[3]))))
			{
				if(IMGPAR direct_spatial_mv_pred_flag)
				{
					ret = IMGPAR FP_ReadMV_Direct_Spatial ARGS3(mb_nr, currMB_r, currMB_s_r);
				} else {
					ret = IMGPAR FP_ReadMV_Direct_Temproal ARGS3(mb_nr, currMB_r, currMB_s_r);
				}

				if (FAILED(ret)) {
					return ret;
				}

				currMB_s_r->do_record = 1;
				//record_reference_picIds ARGS2(list_offset, currMB_s);
			}
		}
#endif
	}
	else
	{
		currMB_r->intra_block = 1;

		currMB_r->NoMbPartLessThan8x8Flag = 1;

		//============= Transform Size Flag for INTRA MBs =============
		//-------------------------------------------------------------
		//transform size flag for INTRA_4x4 and INTRA_8x8 modes
		if (currMB_r->mb_type == I4MB && IMGPAR Transform8x8Mode)
		{
			currMB_r->luma_transform_size_8x8_flag = currMB_s_r->luma_transform_size_8x8_flag =
				read_transform_size_uvlc ARGS0();

			if (currMB_r->luma_transform_size_8x8_flag)
			{
				currMB_r->mb_type = I8MB;
				*(int *) &currMB_r->b8mode[0] = I8MB | (I8MB<<8) | (I8MB<<16) | (I8MB<<24);
				*(int *) &currMB_r->b8pdir[0] = (int) -1;
			}
		}
		else
		{
			currMB_r->luma_transform_size_8x8_flag = currMB_s_r->luma_transform_size_8x8_flag = 0;
		}

		//--- init macroblock data ---
		init_macroblock ARGS0();

		if(currMB_r->mb_type!=IPCM)
		{
			// intra prediction modes for a macroblock 4x4 **********************************************
			g_read_functions->read_ipred_modes ARGS0();
			// read CBP and Coeffs  ***************************************************************
			readCBPandCoeffsFromNAL_UVLC ARGS0();
		}
		else
		{
			//read pcm_alignment_zero_bit and pcm_byte[i]

			// here dep is assigned with the same dep as SE_MBTYPE, because IPCM syntax is in the
			// same category as MBTYPE
			readIPCMcoeffsFromNAL_UVLC ARGS0();
#ifdef _COLLECT_PIC_
			memset(IMGPAR nz_coeff + IMGPAR current_mb_nr_r, 16, sizeof(IMGPAR nz_coeff[0]));
#endif
		}
	}

	return CREL_OK;
}
CREL_RETURN read_one_macroblock_UVLC_B_even PARGS0()
{
	int mb_nr = IMGPAR current_mb_nr_r;	
#ifdef EI_FLAG  
	DecodingEnvironment *dep;
	dep = &(IMGPAR g_dep);
#endif
	CREL_RETURN ret;
	g_read_functions = &(IMGPAR currentSlice->readSyntax);

#ifdef _COLLECT_PIC_
	currMB_s_r->do_record = 0;
#endif

	//currMB_r->mb_field = 0;
	currMB_r->mb_field  = ( IMGPAR currentSlice->structure != FRAME );


	//currMB_s_r->qp = IMGPAR qp ;

	//  read MB mode *****************************************************************
	if(IMGPAR cod_counter == -1)
	{
		IMGPAR cod_counter = readSyntaxElement_VLC_ue ARGS0();
	}
	if (IMGPAR cod_counter==0)
	{
		// read MB aff
		if (IMGPAR MbaffFrameFlag)
		{
			currMB_r->mb_field = readSyntaxElement_FLC ARGS1(1);
		} 

		// read MB type
		currMB_r->mb_type = readSyntaxElement_VLC_ue ARGS0();

#ifdef EI_FLAG  
		if(!dep->Dei_flag)
			currMB_r->ei_flag = 0;
#endif
		IMGPAR cod_counter--;
		currMB_r->skip_flag = 0;
	}
	else
	{
		IMGPAR cod_counter--;
		currMB_r->mb_type = 0;
#ifdef EI_FLAG
		currMB_r->ei_flag = 0;
#endif
		currMB_r->skip_flag = 1;

		// read field flag of bottom block
		if(IMGPAR MbaffFrameFlag)
		{
			if(IMGPAR cod_counter == 0)
			{
				// The following doesn't advance pointer
				currMB_r->mb_field = peekSyntaxElement_FLC ARGS1(1);
			}
			else if(IMGPAR cod_counter > 0)
			{
				// check left macroblock pair first
				//if (mb_is_available ARGS1(mb_nr-2)&&((mb_nr%(IMGPAR PicWidthInMbs*2))!=0))
				if (IMGPAR pLeftMB_r)
				{
					//currMB_r->mb_field = IMGPAR mb_decdata[mb_nr-2].mb_field;
					currMB_r->mb_field = IMGPAR pLeftMB_r->mb_field;
				}
				else
				{
					// check top macroblock pair
					//if (mb_is_available ARGS1(mb_nr-2*IMGPAR PicWidthInMbs))
					if (IMGPAR pUpMB_r) {
						//currMB_r->mb_field = IMGPAR mb_decdata[mb_nr-2*IMGPAR PicWidthInMbs].mb_field;
						currMB_r->mb_field = IMGPAR pUpMB_r->mb_field;
					}
					else
						currMB_r->mb_field = 0;
				}
			}
		}
	}

	currMB_s_r->mb_field = currMB_r->mb_field;

	CheckAvailabilityOfNeighbors_ABCD_even ARGS0();

	if (IMGPAR currentSlice->LFDisableIdc==2)
	{
		// don't filter at slice boundaries
		currMB_s_r->filterLeftMbEdgeFlag = (IMGPAR pLeftMB_r != NULL);
		// if this the bottom of a frame macroblock pair then always filter the top edge
		currMB_s_r->filterTopMbEdgeFlag  = (IMGPAR pUpMB_r != NULL);
	}

	interpret_mb_mode_B ARGS0();
	IMGPAR do_co_located = 1;

	if(IMGPAR MbaffFrameFlag && currMB_r->mb_field)
	{
		IMGPAR num_ref_idx_l0_active <<=1;
		IMGPAR num_ref_idx_l1_active <<=1;
	}

	if ((currMB_r->mb_type&INTRA_MODE) == 0)
	{
		
		if(!currMB_r->mb_type)
			compute_colocated_SUBMB ARGS6(Co_located_MB, listX,0,0,4,4);

		currMB_r->NoMbPartLessThan8x8Flag = ((currMB_r->mb_type == 0) && !(active_sps.direct_8x8_inference_flag))? 0: 1;

		currMB_r->luma_transform_size_8x8_flag = currMB_s_r->luma_transform_size_8x8_flag = 0;

		if(active_pps.constrained_intra_pred_flag)
		{
			currMB_r->intra_block = 0;
		}

		//--- init macroblock data ---
		init_macroblock ARGS0();

		//====== READ 8x8 SUB-PARTITION MODES (modes of 8x8 blocks) and Intra VBST block modes ======
		if (IS_P8x8 (currMB_r))
		{
			for (int i=0; i<4; i++)
			{
				int sub_mb_type = readSyntaxElement_VLC_ue ARGS0();
				currMB_r->b8mode[i]   = reinterpret_cast<unsigned char(*)[2]>(v2b8pd)[sub_mb_type][0];
				currMB_r->b8pdir[i]   = reinterpret_cast<unsigned char(*)[2]>(v2b8pd)[sub_mb_type][1];

				if(!currMB_r->b8mode[i])
					compute_colocated_SUBMB ARGS6(Co_located_MB, listX, i&2, ((i&1)<<1), 2, 2);

				//set NoMbPartLessThan8x8Flag for PB_8x8 mode
				currMB_r->NoMbPartLessThan8x8Flag &= (currMB_r->b8mode[i]==0 && active_sps.direct_8x8_inference_flag) || 
					(currMB_r->b8mode[i]==4);
			}

			ret = readMotionInfoFromNAL ARGS0();
			if (FAILED(ret)) {
				return ret;
			}
			readCBPandCoeffsFromNAL_UVLC ARGS0();
		}
		else if ((currMB_r->mb_type == 0) && (IMGPAR cod_counter >= 0)) //DIRECT mode
		{
			currMB_r->cbp = 0;
			memset(IMGPAR nz_coeff+mb_nr, 0, sizeof(IMGPAR nz_coeff[0]));
		}
		else
		{
			if (IS_INTERMV (currMB_r)) {
				ret = readMotionInfoFromNAL ARGS0();
				if (FAILED(ret)) {
					return ret;
				}
			}

			readCBPandCoeffsFromNAL_UVLC ARGS0();
		}

#ifdef _COLLECT_PIC_
		if (IMGPAR do_co_located)
		{
			IMGPAR do_co_located = 0;

			if ((IS_DIRECT (currMB_r) || 
				(IS_P8x8(currMB_r) && !(currMB_r->b8mode[0] && currMB_r->b8mode[1] && currMB_r->b8mode[2] && currMB_r->b8mode[3]))))
			{
				if(IMGPAR direct_spatial_mv_pred_flag)
				{
					ret = IMGPAR FP_ReadMV_Direct_Spatial ARGS3(mb_nr, currMB_r, currMB_s_r);
				} else {
					ret = IMGPAR FP_ReadMV_Direct_Temproal ARGS3(mb_nr, currMB_r, currMB_s_r);
				}

				if (FAILED(ret)) {
					return ret;
				}

				currMB_s_r->do_record = 1;
				//record_reference_picIds ARGS2(list_offset, currMB_s);
			}
		}
#endif
	}
	else
	{
		currMB_r->intra_block = 1;

		currMB_r->NoMbPartLessThan8x8Flag = 1;

		//============= Transform Size Flag for INTRA MBs =============
		//-------------------------------------------------------------
		//transform size flag for INTRA_4x4 and INTRA_8x8 modes
		if (currMB_r->mb_type == I4MB && IMGPAR Transform8x8Mode)
		{
			currMB_r->luma_transform_size_8x8_flag = currMB_s_r->luma_transform_size_8x8_flag =
				read_transform_size_uvlc ARGS0();

			if (currMB_r->luma_transform_size_8x8_flag)
			{
				currMB_r->mb_type = I8MB;
				*(int *) &currMB_r->b8mode[0] = I8MB | (I8MB<<8) | (I8MB<<16) | (I8MB<<24);
				*(int *) &currMB_r->b8pdir[0] = (int) -1;
			}
		}
		else
		{
			currMB_r->luma_transform_size_8x8_flag = currMB_s_r->luma_transform_size_8x8_flag = 0;
		}

		//--- init macroblock data ---
		init_macroblock ARGS0();

		if(currMB_r->mb_type!=IPCM)
		{
			// intra prediction modes for a macroblock 4x4 **********************************************
			g_read_functions->read_ipred_modes ARGS0();
			// read CBP and Coeffs  ***************************************************************
			readCBPandCoeffsFromNAL_UVLC ARGS0();
		}
		else
		{
			//read pcm_alignment_zero_bit and pcm_byte[i]

			// here dep is assigned with the same dep as SE_MBTYPE, because IPCM syntax is in the
			// same category as MBTYPE
			readIPCMcoeffsFromNAL_UVLC ARGS0();
#ifdef _COLLECT_PIC_
			memset(IMGPAR nz_coeff + IMGPAR current_mb_nr_r, 16, sizeof(IMGPAR nz_coeff[0]));
#endif
		}
	}

	return CREL_OK;
}
#else
CREL_RETURN read_one_macroblock_UVLC_B PARGS0()
{
	int mb_nr = IMGPAR current_mb_nr_r;	
#ifdef EI_FLAG  
	DecodingEnvironment *dep;
	dep = &(IMGPAR g_dep);
#endif
	CREL_RETURN ret;
	g_read_functions = &(IMGPAR currentSlice->readSyntax);

#ifdef _COLLECT_PIC_
	currMB_s_r->do_record = 0;
#endif

	if ( IMGPAR MbaffFrameFlag ) {
		if (mb_nr&1)
			currMB_r->mb_field = IMGPAR pLastMB_r->mb_field;
		else
			currMB_r->mb_field  = ( IMGPAR currentSlice->structure != FRAME );
	} else {
		currMB_r->mb_field  = ( IMGPAR currentSlice->structure != FRAME );
	}

	//currMB_s_r->qp = IMGPAR qp ;

	//  read MB mode *****************************************************************
	if(IMGPAR cod_counter == -1)
	{
		IMGPAR cod_counter = readSyntaxElement_VLC_ue ARGS0();
	}
	if (IMGPAR cod_counter==0)
	{
		// read MB aff
		if (IMGPAR MbaffFrameFlag && (((mb_nr&1) == 0) || (IMGPAR type==B_SLICE && IMGPAR pLastMB_r->skip_flag)))
		{
			currMB_r->mb_field = readSyntaxElement_FLC ARGS1(1);
		}

		// read MB type
		currMB_r->mb_type = readSyntaxElement_VLC_ue ARGS0();

#ifdef EI_FLAG  
		if(!dep->Dei_flag)
			currMB_r->ei_flag = 0;
#endif
		IMGPAR cod_counter--;
		currMB_r->skip_flag = 0;
	}
	else
	{
		IMGPAR cod_counter--;
		currMB_r->mb_type = 0;
#ifdef EI_FLAG
		currMB_r->ei_flag = 0;
#endif
		currMB_r->skip_flag = 1;

		// read field flag of bottom block
		if(IMGPAR MbaffFrameFlag && ((mb_nr&1) == 0))
		{
			if(IMGPAR cod_counter == 0)
			{
				// The following doesn't advance pointer
				currMB_r->mb_field = peekSyntaxElement_FLC ARGS1(1);
			}
			else if(IMGPAR cod_counter > 0)
			{
				// check left macroblock pair first
				if (mb_is_available ARGS1(mb_nr-2)&&((mb_nr%(IMGPAR PicWidthInMbs*2))!=0))
				{
					currMB_r->mb_field = IMGPAR pLeftMB_r->mb_field;//IMGPAR mb_decdata[mb_nr-2].mb_field;
				}
				else
				{
					// check top macroblock pair
					if (mb_is_available ARGS1(mb_nr-2*IMGPAR PicWidthInMbs))
					{
						currMB_r->mb_field = IMGPAR pUpMB_r->mb_field;//IMGPAR mb_decdata[mb_nr-2*IMGPAR PicWidthInMbs].mb_field;
					}
					else
						currMB_r->mb_field = 0;
				}
			}
		}
	}

	currMB_s_r->mb_field = currMB_r->mb_field;
	if (mb_nr&1)
		CheckAvailabilityOfNeighbors_ABCD_odd ARGS0();
	else
		CheckAvailabilityOfNeighbors_ABCD_even ARGS0();

	if (IMGPAR currentSlice->LFDisableIdc==2)
	{
		// don't filter at slice boundaries
		currMB_s_r->filterLeftMbEdgeFlag = (IMGPAR pLeftMB_r != NULL);
		// if this the bottom of a frame macroblock pair then always filter the top edge
		currMB_s_r->filterTopMbEdgeFlag  = IMGPAR pUpMB_r || (IMGPAR MbaffFrameFlag && !currMB_s_r->mb_field && (mb_nr & 1));
	}

	ret = interpret_mb_mode_B ARGS0();
	if (FAILED(ret)) {
		return ret;
	}
	IMGPAR do_co_located = 1;

	if(IMGPAR MbaffFrameFlag && currMB_r->mb_field)
	{
		IMGPAR num_ref_idx_l0_active <<=1;
		IMGPAR num_ref_idx_l1_active <<=1;
	}

	if ((currMB_r->mb_type&INTRA_MODE) == 0)
	{
		
		if(!currMB_r->mb_type)
			compute_colocated_SUBMB ARGS6(Co_located_MB, listX,0,0,4,4);

		currMB_r->NoMbPartLessThan8x8Flag = ((currMB_r->mb_type == 0) && !(active_sps.direct_8x8_inference_flag))? 0: 1;

		currMB_r->luma_transform_size_8x8_flag = currMB_s_r->luma_transform_size_8x8_flag = 0;

		if(active_pps.constrained_intra_pred_flag)
		{
			currMB_r->intra_block = 0;
		}

		//--- init macroblock data ---
		init_macroblock       ARGS0();

		//====== READ 8x8 SUB-PARTITION MODES (modes of 8x8 blocks) and Intra VBST block modes ======
		if (IS_P8x8 (currMB_r))
		{
			for (int i=0; i<4; i++)
			{
				int sub_mb_type = readSyntaxElement_VLC_ue ARGS0();
				currMB_r->b8mode[i]   = reinterpret_cast<unsigned char(*)[2]>(v2b8pd)[sub_mb_type][0];
				currMB_r->b8pdir[i]   = reinterpret_cast<unsigned char(*)[2]>(v2b8pd)[sub_mb_type][1];

				if(!currMB_r->b8mode[i])
					compute_colocated_SUBMB ARGS6(Co_located_MB, listX, i&2, ((i&1)<<1), 2, 2);

				//set NoMbPartLessThan8x8Flag for PB_8x8 mode
				currMB_r->NoMbPartLessThan8x8Flag &= (currMB_r->b8mode[i]==0 && active_sps.direct_8x8_inference_flag) || 
					(currMB_r->b8mode[i]==4);
			}

			ret = readMotionInfoFromNAL ARGS0();
			if (FAILED(ret)) {
				return ret;
			}
			readCBPandCoeffsFromNAL_UVLC ARGS0();
		}
		else if ((currMB_r->mb_type == 0) && (IMGPAR cod_counter >= 0)) //DIRECT mode
		{
			currMB_r->cbp = 0;
			memset(IMGPAR nz_coeff+mb_nr, 0, sizeof(IMGPAR nz_coeff[0]));
		}
		else
		{
			if (IS_INTERMV (currMB_r)) {
				ret = readMotionInfoFromNAL ARGS0();
				if (FAILED(ret)) {
					return ret;
				}
			}

			readCBPandCoeffsFromNAL_UVLC ARGS0();
		}

#ifdef _COLLECT_PIC_
		if (IMGPAR do_co_located)
		{
			IMGPAR do_co_located = 0;

			if ((IS_DIRECT (currMB_r) || 
				(IS_P8x8(currMB_r) && !(currMB_r->b8mode[0] && currMB_r->b8mode[1] && currMB_r->b8mode[2] && currMB_r->b8mode[3]))))
			{
				if(IMGPAR direct_spatial_mv_pred_flag){
					ret = IMGPAR FP_ReadMV_Direct_Spatial ARGS3(mb_nr, currMB_r, currMB_s_r);
				} else {
					ret = IMGPAR FP_ReadMV_Direct_Temproal ARGS3(mb_nr, currMB_r, currMB_s_r);
				}

				currMB_s_r->do_record = 1;
				//record_reference_picIds ARGS2(list_offset, currMB_s);
			}
		}
#endif
	}
	else
	{
		currMB_r->intra_block = 1;

		currMB_r->NoMbPartLessThan8x8Flag = 1;

		//============= Transform Size Flag for INTRA MBs =============
		//-------------------------------------------------------------
		//transform size flag for INTRA_4x4 and INTRA_8x8 modes
		if (currMB_r->mb_type == I4MB && IMGPAR Transform8x8Mode)
		{
			currMB_r->luma_transform_size_8x8_flag = currMB_s_r->luma_transform_size_8x8_flag =
				read_transform_size_uvlc ARGS0();

			if (currMB_r->luma_transform_size_8x8_flag)
			{
				currMB_r->mb_type = I8MB;
				*(int *) &currMB_r->b8mode[0] = I8MB | (I8MB<<8) | (I8MB<<16) | (I8MB<<24);
				*(int *) &currMB_r->b8pdir[0] = (int) -1;
			}
		}
		else
		{
			currMB_r->luma_transform_size_8x8_flag = currMB_s_r->luma_transform_size_8x8_flag = 0;
		}

		//--- init macroblock data ---
		init_macroblock       ARGS0();

		if(currMB_r->mb_type!=IPCM)
		{
			// intra prediction modes for a macroblock 4x4 **********************************************
			g_read_functions->read_ipred_modes ARGS0();

			// read CBP and Coeffs  ***************************************************************
			readCBPandCoeffsFromNAL_UVLC ARGS0();
		}
		else
		{
			//read pcm_alignment_zero_bit and pcm_byte[i]

			// here dep is assigned with the same dep as SE_MBTYPE, because IPCM syntax is in the
			// same category as MBTYPE
			readIPCMcoeffsFromNAL_UVLC ARGS0();
#ifdef _COLLECT_PIC_
			memset(IMGPAR nz_coeff + IMGPAR current_mb_nr_r, 16, sizeof(IMGPAR nz_coeff[0]));
#endif
		}
	}

	return DECODE_MB;
}
#endif
#else
CREL_RETURN read_one_macroblock_UVLC PARGS0()
{
	int i;
	int mb_nr = IMGPAR current_mb_nr_r;	
	Slice *currSlice = IMGPAR currentSlice;
	CREL_RETURN ret;
#ifdef EI_FLAG  
	DecodingEnvironment *dep;
	dep = &(IMGPAR g_dep);
#endif

	read_functions_t *read_functions = g_read_functions = &(currSlice->readSyntax);

#ifdef _COLLECT_PIC_
	currMB_s_r->do_record = 0;
#endif

	int  img_block_y;

	if (mb_nr&1)
		currMB_r->mb_field = IMGPAR mb_decdata[mb_nr-1].mb_field;
	else
		currMB_r->mb_field = 0;

	//currMB_s_r->qp = IMGPAR qp ;

	//  read MB mode *****************************************************************
	if(IMGPAR type == I_SLICE)
	{
		// read MB aff
		if (IMGPAR MbaffFrameFlag && ((mb_nr&1)==0))
		{
			currMB_r->mb_field = readSyntaxElement_FLC ARGS1(1);
		}

		//  read MB type
		currMB_r->mb_type = read_raw_mb_uvlc ARGS0();
#ifdef EI_FLAG  
		if(!dep->Dei_flag)
			currMB_r->ei_flag = 0;
#endif
	}
	else
	{
		if(IMGPAR cod_counter == -1)
		{
			IMGPAR cod_counter = readSyntaxElement_VLC_ue ARGS0();
		}
		if (IMGPAR cod_counter==0)
		{
			// read MB aff
			if (IMGPAR MbaffFrameFlag && ((mb_nr&1) == 0||(IMGPAR type==B_SLICE && IMGPAR mb_decdata[mb_nr-1].skip_flag)||(IMGPAR type!=B_SLICE && IMGPAR mb_decdata[mb_nr-1].mb_type==0)))
			{
				currMB_r->mb_field = readSyntaxElement_FLC ARGS1(1);
			}

			// read MB type
			currMB_r->mb_type = readSyntaxElement_VLC_ue ARGS0();
			if(IMGPAR type == P_SLICE)
				currMB_r->mb_type++;
#ifdef EI_FLAG  
			if(!dep->Dei_flag)
				currMB_r->ei_flag = 0;
#endif
			IMGPAR cod_counter--;
			currMB_r->skip_flag = 0;
		}
		else
		{
			IMGPAR cod_counter--;
			currMB_r->mb_type = 0;
#ifdef EI_FLAG
			currMB_r->ei_flag = 0;
#endif
			currMB_r->skip_flag = 1;

			// read field flag of bottom block
			if(IMGPAR MbaffFrameFlag && ((mb_nr&1) == 0))
			{
				if(IMGPAR cod_counter == 0)
				{
					// The following doesn't advance pointer
					currMB_r->mb_field = peekSyntaxElement_FLC ARGS1(1);
				}
				else if(IMGPAR cod_counter > 0)
				{
					// check left macroblock pair first
					if (mb_is_available ARGS1(mb_nr-2)&&((mb_nr%(IMGPAR PicWidthInMbs*2))!=0))
					{
						currMB_r->mb_field = IMGPAR mb_decdata[mb_nr-2].mb_field;
					}
					else
					{
						// check top macroblock pair
						if (mb_is_available ARGS1(mb_nr-2*IMGPAR PicWidthInMbs))
						{
							currMB_r->mb_field = IMGPAR mb_decdata[mb_nr-2*IMGPAR PicWidthInMbs].mb_field;
						}
						else
							currMB_r->mb_field = 0;
					}
				}
			}
		}
	}

	currMB_s_r->mb_field = currMB_r->mb_field;
	if (mb_nr&1)
		CheckAvailabilityOfNeighbors_ABCD_odd ARGS0();
	else
		CheckAvailabilityOfNeighbors_ABCD_even ARGS0();

	if (IMGPAR currentSlice->LFDisableIdc==2)
	{
		// don't filter at slice boundaries
		currMB_s_r->filterLeftMbEdgeFlag = (IMGPAR pLeftMB_r != NULL);
		// if this the bottom of a frame macroblock pair then always filter the top edge
		currMB_s_r->filterTopMbEdgeFlag  = IMGPAR pUpMB_r || (IMGPAR MbaffFrameFlag && !currMB_s_r->mb_field && (mb_nr & 1));
	}

	if ((IMGPAR type==P_SLICE )) {    // inter frame
		ret = interpret_mb_mode_P ARGS0();
		if (FAILED(ret)) {
			return ret;
		}
	} else if (IMGPAR type==I_SLICE) {   // intra frame
		ret = interpret_mb_mode_I ARGS0();
		if (FAILED(ret)) {
			return ret;
		}
	} else if ((IMGPAR type==B_SLICE)) // B frame
	{
		ret = interpret_mb_mode_B ARGS0();
		if (FAILED(ret)) {
			return ret;
		}
		IMGPAR do_co_located = 1;
	}

	
	if(IMGPAR type == B_SLICE && !currMB_r->mb_type)
		compute_colocated_SUBMB ARGS6(Co_located_MB, listX,0,0,4,4);

	if(IMGPAR MbaffFrameFlag && currMB_r->mb_field)
	{
		IMGPAR num_ref_idx_l0_active <<=1;
		IMGPAR num_ref_idx_l1_active <<=1;
	}

	//init NoMbPartLessThan8x8Flag
	currMB_r->NoMbPartLessThan8x8Flag = (IS_DIRECT(currMB_r) && !(active_sps.direct_8x8_inference_flag))? 0: 1;

	//====== READ 8x8 SUB-PARTITION MODES (modes of 8x8 blocks) and Intra VBST block modes ======
	if (IS_P8x8 (currMB_r))
	{
		for (i=0; i<4; i++)
		{
			int sub_mb_type = readSyntaxElement_VLC_ue ARGS0();
			currMB_r->b8mode[i]   = reinterpret_cast<unsigned char(*)[2]>(v2b8pd)[sub_mb_type][0];
			currMB_r->b8pdir[i]   = reinterpret_cast<unsigned char(*)[2]>(v2b8pd)[sub_mb_type][1];

			if(IMGPAR type == B_SLICE && !currMB_r->b8mode[i])
				compute_colocated_SUBMB ARGS6(Co_located_MB, listX, i&2, ((i&1)<<1), 2, 2);

			//set NoMbPartLessThan8x8Flag for PB_8x8 mode
			currMB_r->NoMbPartLessThan8x8Flag &= (currMB_r->b8mode[i]==0 && active_sps.direct_8x8_inference_flag) || 
				(currMB_r->b8mode[i]==4);
		}

		//--- init macroblock data ---
		init_macroblock       ARGS0();
		ret = readMotionInfoFromNAL ARGS0();
		if (FAILED(ret)) {
			return ret;
		}
	}

	//============= Transform Size Flag for INTRA MBs =============
	//-------------------------------------------------------------
	//transform size flag for INTRA_4x4 and INTRA_8x8 modes
	if (currMB_r->mb_type == I4MB && IMGPAR Transform8x8Mode)
	{
		currMB_r->luma_transform_size_8x8_flag = currMB_s_r->luma_transform_size_8x8_flag =
			read_transform_size_uvlc ARGS0();

		if (currMB_r->luma_transform_size_8x8_flag)
		{
			currMB_r->mb_type = I8MB;
			*(int *) &currMB_r->b8mode[0] = I8MB | (I8MB<<8) | (I8MB<<16) | (I8MB<<24);
			*(int *) &currMB_r->b8pdir[0] = (int) -1;
		}
	}
	else
	{
		currMB_r->luma_transform_size_8x8_flag = currMB_s_r->luma_transform_size_8x8_flag = 0;
	}

	if(active_pps.constrained_intra_pred_flag && (IMGPAR type==P_SLICE|| IMGPAR type==B_SLICE))        // inter frame
	{
		if( !IS_INTRA(currMB_r) )
		{
			currMB_r->intra_block = 0;
		}
	}

	//--- init macroblock data ---
	if (!IS_P8x8 (currMB_r))
		init_macroblock ARGS0();

	if (IS_DIRECT (currMB_r) && IMGPAR cod_counter >= 0)
	{
		currMB_r->cbp = 0;

		memset(IMGPAR nz_coeff+mb_nr, 0, sizeof(IMGPAR nz_coeff[0]));
#ifdef _COLLECT_PIC_
		if (IMGPAR do_co_located)
		{
			IMGPAR do_co_located = 0;

			if ((IS_DIRECT (currMB_r) || 
				(IS_P8x8(currMB_r) && !(currMB_r->b8mode[0] && currMB_r->b8mode[1] && currMB_r->b8mode[2] && currMB_r->b8mode[3]))))
			{
				if(IMGPAR direct_spatial_mv_pred_flag)
				{
					ret = IMGPAR FP_ReadMV_Direct_Spatial ARGS3(mb_nr, currMB_r, currMB_s_r);
				} else {
					ret = IMGPAR FP_ReadMV_Direct_Temproal ARGS3(mb_nr, currMB_r, currMB_s_r);
				}

				currMB_s_r->do_record = 1;
				//record_reference_picIds ARGS2(list_offset, currMB_s);
			}
		}
#endif
		return DECODE_MB;
	}

	if (IS_COPY (currMB_r)) //keep last macroblock
	{
		MotionVector pmv;
		int zeroMotionAbove;
		int zeroMotionLeft;
		PixelPos mb_a, mb_b;
		PixelPos* p_mb_a = &mb_a;
		PixelPos* p_mb_b = &mb_b;
		Macroblock_s *mbs_a, *mbs_b;
		int      a_mv_y = 0;
		int      a_ref_idx = 0;
		int      b_mv_y = 0;
		int      b_ref_idx = 0;
		int      list_offset = ((IMGPAR MbaffFrameFlag)&&(currMB_r->mb_field))? (mb_nr&1) ? 4 : 2 : 0;

#if !defined(REC_NEIGHBOR)
		getLuma4x4Neighbour ARGS7(mb_nr,0,0,-1, 0,&mb_a, currMB_r);
		getLuma4x4Neighbour ARGS7(mb_nr,0,0, 0,-1,&mb_b, currMB_r);
#else

		p_mb_a = &(IMGPAR left[0]);

		//if (!(IMGPAR left_set[0]))
		//{
		//	getLuma4x4Neighbour_left_out ARGS7(mb_nr, 0, 0, -1,  0, p_mb_a, currMB_r);
		//	IMGPAR left_set[0] = 1;
		//}

		p_mb_b = &(IMGPAR up[0]);
		//if (!(IMGPAR up_set[0]))
		//{
		//	getLuma4x4Neighbour_top_out ARGS7(mb_nr, 0, 0,  0, -1, p_mb_b, currMB_r);
		//	IMGPAR up_set[0] = 1;
		//}
#endif

		if (p_mb_a->pMB)
		{
			mbs_a     = &dec_picture->mb_data[p_mb_a->mb_addr];
			a_mv_y    = mbs_a->pred_info.mv[LIST_0][p_mb_a->y*4+p_mb_a->x].y;
			a_ref_idx = mbs_a->pred_info.ref_idx[LIST_0][l_16_4[p_mb_a->y*4+p_mb_a->x]];

			if (currMB_s_r->mb_field ^ mbs_a->mb_field)
			{
				if(currMB_s_r->mb_field)
				{
					a_mv_y   /= 2;
					a_ref_idx <<= 1;
				}
				else
				{
					a_mv_y   <<= 1;
					a_ref_idx >>= 1;
				}
			}
		}

		if (p_mb_b->pMB)
		{
			mbs_b     = &dec_picture->mb_data[p_mb_b->mb_addr];
			b_mv_y    = mbs_b->pred_info.mv[LIST_0][p_mb_b->y*4+p_mb_b->x].y;
			b_ref_idx = mbs_b->pred_info.ref_idx[LIST_0][l_16_4[p_mb_b->y*4+p_mb_b->x]]; 

			if (currMB_s_r->mb_field ^ mbs_b->mb_field)
			{
				if(currMB_s_r->mb_field)
				{
					b_mv_y    /= 2;
					b_ref_idx  <<= 1;
				}
				else
				{
					b_mv_y    <<= 1;
					b_ref_idx  >>= 1;
				}
			}
		}

		zeroMotionLeft  = p_mb_a->pMB ? (a_ref_idx==0 && mbs_a->pred_info.mv[LIST_0][p_mb_a->y*4+p_mb_a->x].x==0 && a_mv_y==0 ? 1 : 0) : 1;
		zeroMotionAbove = p_mb_b->pMB ? (b_ref_idx==0 && mbs_b->pred_info.mv[LIST_0][p_mb_b->y*4+p_mb_b->x].x==0 && b_mv_y==0 ? 1 : 0) : 1;

		currMB_r->cbp = 0;
		memset(IMGPAR nz_coeff+mb_nr, 0, sizeof(IMGPAR nz_coeff[0]));

		img_block_y   = IMGPAR block_y_r;

		if (zeroMotionAbove || zeroMotionLeft)
		{
			memset(currMB_s_r->pred_info.mv[0], 0, sizeof(currMB_s_r->pred_info.mv[0]));
		}
		else
		{
			SetMotionVectorPredictor_block00_shape16x16 ARGS6(&pmv,0, LIST_0, IMGPAR current_mb_nr_r, currMB_r, currMB_s_r);

			Set16MotionVector(currMB_s_r->pred_info.mv[0], pmv);
		}

		// Intel compiler 9.0 has fixed this
		memset(&(currMB_s_r->pred_info.ref_idx[0][0]),0,4);
#if !defined(_COLLECT_PIC_)
		memset(&(currMB_s_r->pred_info.ref_pic_id[0][0]),(char) listX[LIST_0+list_offset][0]->unique_id,4);
#endif
		return DECODE_MB;
	}
	if(currMB_r->mb_type!=IPCM)
	{
		// intra prediction modes for a macroblock 4x4 **********************************************
		g_read_functions->read_ipred_modes ARGS0();

		// read inter frame vector data *********************************************************
		if (IS_INTERMV (currMB_r) && (!IS_P8x8(currMB_r)))
		{
			ret = readMotionInfoFromNAL ARGS0();
			if (FAILED(ret)) {
				return ret;
			}
		}
		// read CBP and Coeffs  ***************************************************************
		readCBPandCoeffsFromNAL_UVLC ARGS0();
#ifdef _COLLECT_PIC_
		if (IMGPAR do_co_located)
		{
			IMGPAR do_co_located = 0;

			if ((IS_DIRECT (currMB_r) || 
				(IS_P8x8(currMB_r) && !(currMB_r->b8mode[0] && currMB_r->b8mode[1] && currMB_r->b8mode[2] && currMB_r->b8mode[3]))))
			{
				if(IMGPAR direct_spatial_mv_pred_flag)
				{
					ret = IMGPAR FP_ReadMV_Direct_Spatial ARGS3(mb_nr, currMB_r, currMB_s_r);
				} else {
					ret = IMGPAR FP_ReadMV_Direct_Temproal ARGS3(mb_nr, currMB_r, currMB_s_r);
				}

				currMB_s_r->do_record = 1;
				//record_reference_picIds ARGS2(list_offset, currMB_s);
			}
		}
#endif
	}
	else
	{
		//read pcm_alignment_zero_bit and pcm_byte[i]

		// here dep is assigned with the same dep as SE_MBTYPE, because IPCM syntax is in the
		// same category as MBTYPE
		readIPCMcoeffsFromNAL_UVLC ARGS0();
#ifdef _COLLECT_PIC_
		memset(IMGPAR nz_coeff + IMGPAR current_mb_nr_r, 16, sizeof(IMGPAR nz_coeff[0]));
#endif
	}

	return DECODE_MB;
}
#endif
/*!
************************************************************************
* \brief
*    Get the syntax elements from the NAL
************************************************************************
*/
#if 1
#if defined (_COLLECT_PIC_)
CREL_RETURN read_one_macroblock_CABAC_I_odd PARGS0()
{
	int mb_nr = IMGPAR current_mb_nr_r;
	CREL_RETURN ret;
#ifdef EI_FLAG  
	DecodingEnvironment *dep;
	dep = &(IMGPAR g_dep);
#endif
	g_read_functions = &(IMGPAR currentSlice->readSyntax);

#ifdef _COLLECT_PIC_
	currMB_s_r->do_record = 0;
#endif

	if ( IMGPAR MbaffFrameFlag ) {
		currMB_r->mb_field = IMGPAR pLastMB_r->mb_field;
	} else {
		currMB_r->mb_field  = ( IMGPAR currentSlice->structure != FRAME );
	}

	//  read MB mode *****************************************************************
#ifdef CONFIG_BIARI_ENABLE_MMX
	store_dep ARGS0();
#endif

	CheckAvailabilityOfNeighborsCABAC_odd ARGS0();
	CheckAvailabilityOfNeighbors_ABCD_odd ARGS0();

	//  read MB type
	currMB_r->mb_type = readMB_typeInfo_CABAC ARGS0();
#ifdef EI_FLAG
	if(!dep->Dei_flag)
		currMB_r->ei_flag = 0;
#endif

	currMB_s_r->mb_field = currMB_r->mb_field;	

	if (IMGPAR currentSlice->LFDisableIdc==2)
	{
		// don't filter at slice boundaries
		currMB_s_r->filterLeftMbEdgeFlag = (IMGPAR pLeftMB_r != NULL);
		// if this the bottom of a frame macroblock pair then always filter the top edge
		currMB_s_r->filterTopMbEdgeFlag  = IMGPAR pUpMB_r || (IMGPAR MbaffFrameFlag && !currMB_s_r->mb_field);
	}

	ret = interpret_mb_mode_I ARGS0();
	if (FAILED(ret)) {
		return ret;
	}

	if(IMGPAR MbaffFrameFlag && currMB_r->mb_field)
	{
		IMGPAR num_ref_idx_l0_active <<=1;
		IMGPAR num_ref_idx_l1_active <<=1;
	}

	currMB_r->intra_block = 1;

	currMB_r->NoMbPartLessThan8x8Flag = 1;

	//============= Transform Size Flag for INTRA MBs =============
	//-------------------------------------------------------------
	//transform size flag for INTRA_4x4 and INTRA_8x8 modes
	if (currMB_r->mb_type == I4MB && IMGPAR Transform8x8Mode)
	{
		currMB_r->luma_transform_size_8x8_flag = currMB_s_r->luma_transform_size_8x8_flag =
			readMB_transform_size_flag_CABAC ARGS0();

		if (currMB_r->luma_transform_size_8x8_flag)
		{
			currMB_r->mb_type = I8MB;
			*(int *) &currMB_r->b8mode[0] = I8MB | (I8MB<<8) | (I8MB<<16) | (I8MB<<24);
			*(int *) &currMB_r->b8pdir[0] = (int) -1;
			//for (i=0;i<4;i++) {currMB_r->b8mode[i]=I8MB; currMB_r->b8pdir[i]=-1; }
		}
	}
	else
	{
		currMB_r->luma_transform_size_8x8_flag = currMB_s_r->luma_transform_size_8x8_flag = 0;
	}

	//--- init macroblock data ---
	init_macroblock ARGS0();

	if(currMB_r->mb_type!=IPCM)
	{
		// intra prediction modes for a macroblock 4x4 **********************************************
		g_read_functions->read_ipred_modes ARGS0();
		// read CBP and Coeffs  ***************************************************************
		readCBPandCoeffsFromNAL_CABAC ARGS0();
	}
	else
	{
		//read pcm_alignment_zero_bit and pcm_byte[i]

		// here dep is assigned with the same dep as SE_MBTYPE, because IPCM syntax is in the 
		// same category as MBTYPE
#ifdef CONFIG_BIARI_ENABLE_MMX
		load_dep ARGS0();
#endif
		ret = readIPCMcoeffsFromNAL_CABAC ARGS0();
#ifdef CONFIG_BIARI_ENABLE_MMX
		store_dep ARGS0();
#endif
		if (FAILED(ret)) {
			return ret;
		}
	}

#ifdef CONFIG_BIARI_ENABLE_MMX
	load_dep ARGS0();
#endif

	return CREL_OK;
}
CREL_RETURN read_one_macroblock_CABAC_I_even PARGS0()
{
	int mb_nr = IMGPAR current_mb_nr_r;
	CREL_RETURN ret;
#ifdef EI_FLAG  
	DecodingEnvironment *dep;
	dep = &(IMGPAR g_dep);
#endif
	g_read_functions = &(IMGPAR currentSlice->readSyntax);

#ifdef _COLLECT_PIC_
	currMB_s_r->do_record = 0;
#endif

	currMB_r->mb_field  = ( IMGPAR currentSlice->structure != FRAME );	

	//  read MB mode *****************************************************************
#ifdef CONFIG_BIARI_ENABLE_MMX
	store_dep ARGS0();
#endif

	// read MB aff
	if (IMGPAR MbaffFrameFlag)
	{
#ifdef EI_FLAG
		if (dep->Dei_flag)
#endif
		{
			currMB_r->mb_field = readFieldModeInfo_CABAC ARGS0();
		}
	} 

	CheckAvailabilityOfNeighborsCABAC_even ARGS0();
	CheckAvailabilityOfNeighbors_ABCD_even ARGS0();

	//  read MB type
	currMB_r->mb_type = readMB_typeInfo_CABAC ARGS0();
#ifdef EI_FLAG
	if(!dep->Dei_flag)
		currMB_r->ei_flag = 0;
#endif

	currMB_s_r->mb_field = currMB_r->mb_field;


	if (IMGPAR currentSlice->LFDisableIdc==2)
	{
		// don't filter at slice boundaries
		currMB_s_r->filterLeftMbEdgeFlag = (IMGPAR pLeftMB_r != NULL);
		// if this the bottom of a frame macroblock pair then always filter the top edge
		currMB_s_r->filterTopMbEdgeFlag  = (IMGPAR pUpMB_r != NULL);
	}

	ret = interpret_mb_mode_I ARGS0();
	if (FAILED(ret)) {
		return ret;
	}

	if(IMGPAR MbaffFrameFlag && currMB_r->mb_field)
	{
		IMGPAR num_ref_idx_l0_active <<=1;
		IMGPAR num_ref_idx_l1_active <<=1;
	}

	currMB_r->intra_block = 1;

	currMB_r->NoMbPartLessThan8x8Flag = 1;

	//============= Transform Size Flag for INTRA MBs =============
	//-------------------------------------------------------------
	//transform size flag for INTRA_4x4 and INTRA_8x8 modes
	if (currMB_r->mb_type == I4MB && IMGPAR Transform8x8Mode)
	{
		currMB_r->luma_transform_size_8x8_flag = currMB_s_r->luma_transform_size_8x8_flag =
			readMB_transform_size_flag_CABAC ARGS0();

		if (currMB_r->luma_transform_size_8x8_flag)
		{
			currMB_r->mb_type = I8MB;
			*(int *) &currMB_r->b8mode[0] = I8MB | (I8MB<<8) | (I8MB<<16) | (I8MB<<24);
			*(int *) &currMB_r->b8pdir[0] = (int) -1;
			//for (i=0;i<4;i++) {currMB_r->b8mode[i]=I8MB; currMB_r->b8pdir[i]=-1; }
		}
	}
	else
	{
		currMB_r->luma_transform_size_8x8_flag = currMB_s_r->luma_transform_size_8x8_flag = 0;
	}

	//--- init macroblock data ---
	init_macroblock ARGS0();

	if(currMB_r->mb_type!=IPCM)
	{
		// intra prediction modes for a macroblock 4x4 **********************************************
		g_read_functions->read_ipred_modes ARGS0();
		// read CBP and Coeffs  ***************************************************************
		readCBPandCoeffsFromNAL_CABAC ARGS0();
	}
	else
	{
		//read pcm_alignment_zero_bit and pcm_byte[i]

		// here dep is assigned with the same dep as SE_MBTYPE, because IPCM syntax is in the 
		// same category as MBTYPE
#ifdef CONFIG_BIARI_ENABLE_MMX
		load_dep ARGS0();
#endif
		ret = readIPCMcoeffsFromNAL_CABAC ARGS0();
#ifdef CONFIG_BIARI_ENABLE_MMX
		store_dep ARGS0();
#endif
		if (FAILED(ret)) {
			return ret;
		}
	}

#ifdef CONFIG_BIARI_ENABLE_MMX
	load_dep ARGS0();
#endif

	return CREL_OK;
}
#else
CREL_RETURN read_one_macroblock_CABAC_I PARGS0()
{
	int mb_nr = IMGPAR current_mb_nr_r;
	CREL_RETURN ret;
#ifdef EI_FLAG  
	DecodingEnvironment *dep;
	dep = &(IMGPAR g_dep);
#endif
	g_read_functions = &(IMGPAR currentSlice->readSyntax);

#ifdef _COLLECT_PIC_
	currMB_s_r->do_record = 0;
#endif

	if ( IMGPAR MbaffFrameFlag ) {
		if (mb_nr&1)
			currMB_r->mb_field = IMGPAR pLastMB_r->mb_field;
		else
			currMB_r->mb_field = readFieldModeInfo_CABAC ARGS0();
	} else {
		currMB_r->mb_field  = ( IMGPAR currentSlice->structure != FRAME );
	}

	//  read MB mode *****************************************************************
#ifdef CONFIG_BIARI_ENABLE_MMX
	store_dep ARGS0();
#endif

	if (mb_nr&1)
	{
		CheckAvailabilityOfNeighborsCABAC_odd ARGS0();
		CheckAvailabilityOfNeighbors_ABCD_odd ARGS0();
	}
	else
	{
		CheckAvailabilityOfNeighborsCABAC_even ARGS0();
		CheckAvailabilityOfNeighbors_ABCD_even ARGS0();
	}

	//  read MB type
	currMB_r->mb_type = readMB_typeInfo_CABAC ARGS0();
#ifdef EI_FLAG
	if(!dep->Dei_flag)
		currMB_r->ei_flag = 0;
#endif

	currMB_s_r->mb_field = currMB_r->mb_field;

	if (IMGPAR currentSlice->LFDisableIdc==2)
	{
		// don't filter at slice boundaries
		currMB_s_r->filterLeftMbEdgeFlag = (IMGPAR pLeftMB_r != NULL);
		// if this the bottom of a frame macroblock pair then always filter the top edge
		currMB_s_r->filterTopMbEdgeFlag  = IMGPAR pUpMB_r || (IMGPAR MbaffFrameFlag && !currMB_s_r->mb_field && (mb_nr & 1));
	}

	ret = interpret_mb_mode_I ARGS0();
	if (FAILED(ret)) {
		return ret;
	}

	if(IMGPAR MbaffFrameFlag && currMB_r->mb_field)
	{
		IMGPAR num_ref_idx_l0_active <<=1;
		IMGPAR num_ref_idx_l1_active <<=1;
	}

	currMB_r->intra_block = 1;

	currMB_r->NoMbPartLessThan8x8Flag = 1;

	//============= Transform Size Flag for INTRA MBs =============
	//-------------------------------------------------------------
	//transform size flag for INTRA_4x4 and INTRA_8x8 modes
	if (currMB_r->mb_type == I4MB && IMGPAR Transform8x8Mode)
	{
		currMB_r->luma_transform_size_8x8_flag = currMB_s_r->luma_transform_size_8x8_flag =
			readMB_transform_size_flag_CABAC ARGS0();

		if (currMB_r->luma_transform_size_8x8_flag)
		{
			currMB_r->mb_type = I8MB;
			*(int *) &currMB_r->b8mode[0] = I8MB | (I8MB<<8) | (I8MB<<16) | (I8MB<<24);
			*(int *) &currMB_r->b8pdir[0] = (int) -1;
			//for (i=0;i<4;i++) {currMB_r->b8mode[i]=I8MB; currMB_r->b8pdir[i]=-1; }
		}
	}
	else
	{
		currMB_r->luma_transform_size_8x8_flag = currMB_s_r->luma_transform_size_8x8_flag = 0;
	}

	//--- init macroblock data ---
	init_macroblock ARGS0();

	if(currMB_r->mb_type!=IPCM)
	{
		// intra prediction modes for a macroblock 4x4 **********************************************
		g_read_functions->read_ipred_modes ARGS0();

		// read CBP and Coeffs  ***************************************************************
		readCBPandCoeffsFromNAL_CABAC ARGS0();
	}
	else
	{
		//read pcm_alignment_zero_bit and pcm_byte[i]

		// here dep is assigned with the same dep as SE_MBTYPE, because IPCM syntax is in the 
		// same category as MBTYPE
#ifdef CONFIG_BIARI_ENABLE_MMX
		load_dep ARGS0();
#endif
		ret = readIPCMcoeffsFromNAL_CABAC ARGS0();
#ifdef CONFIG_BIARI_ENABLE_MMX
		store_dep ARGS0();
#endif
		if (FAILED(ret)) {
			return ret;
		}
	}

#ifdef CONFIG_BIARI_ENABLE_MMX
	load_dep ARGS0();
#endif

	return CREL_OK;
}
#endif

#if defined (_COLLECT_PIC_)
CREL_RETURN read_one_macroblock_CABAC_P_odd PARGS0()
{
	int mb_nr = IMGPAR current_mb_nr_r;
	CREL_RETURN ret;
#ifdef EI_FLAG  
	DecodingEnvironment *dep;
	dep = &(IMGPAR g_dep);
#endif
	g_read_functions = &(IMGPAR currentSlice->readSyntax);

#ifdef _COLLECT_PIC_
	currMB_s_r->do_record = 0;
#endif

	if ( IMGPAR MbaffFrameFlag ) {
		currMB_r->mb_field = IMGPAR pLastMB_r->mb_field;
	} else {
		currMB_r->mb_field  = ( IMGPAR currentSlice->structure != FRAME );
	}

	//  read MB mode *****************************************************************
#ifdef CONFIG_BIARI_ENABLE_MMX
	store_dep ARGS0();
#endif

	if (IMGPAR MbaffFrameFlag)
	{
		if ((IMGPAR type!=B_SLICE && IMGPAR pLastMB_r->mb_type == 0))
		{
			//field_flag_inference();			
			if (IMGPAR pLeftMB_r)
			{
				currMB_r->mb_field = IMGPAR pLeftMB_r->mb_field;
			}
			else
			{
				// check top macroblock pair
				if (IMGPAR pUpMB_r)
				{
					currMB_r->mb_field = IMGPAR pUpMB_r->mb_field;
				}
				else
					currMB_r->mb_field = 0;
			}
		}
	}

	CheckAvailabilityOfNeighborsCABAC_odd ARGS0();
	currMB_r->skip_flag = !readMB_skip_flagInfo_CABAC ARGS0();


#ifdef EI_FLAG
	if(!dep->Dei_flag)
		currMB_r->ei_flag = 0;
#endif

	// read MB AFF
	if (IMGPAR MbaffFrameFlag)
	{
		if (IMGPAR pLastMB_r->skip_flag && (!currMB_r->skip_flag)) {
			char mb_field_pred = currMB_r->mb_field;
			currMB_r->mb_field = readFieldModeInfo_CABAC ARGS0();			
		}
	}

	CheckAvailabilityOfNeighbors_ABCD_odd ARGS0();

	// read MB type
	if (currMB_r->skip_flag == 0 ) 
	{
		currMB_r->mb_type = readMB_typeInfo_CABAC ARGS0();

#ifdef EI_FLAG
		if(!dep->Dei_flag)
			currMB_r->ei_flag = 0;
#endif
	} else {
		currMB_r->mb_type = 0;
	}


	currMB_s_r->mb_field = currMB_r->mb_field;

	if (IMGPAR currentSlice->LFDisableIdc==2)
	{
		// don't filter at slice boundaries
		currMB_s_r->filterLeftMbEdgeFlag = (IMGPAR pLeftMB_r != NULL);
		// if this the bottom of a frame macroblock pair then always filter the top edge
		currMB_s_r->filterTopMbEdgeFlag  = IMGPAR pUpMB_r || (IMGPAR MbaffFrameFlag && !currMB_s_r->mb_field);
	}

	ret = interpret_mb_mode_P ARGS0();
	if (FAILED(ret)) {
		return ret;
	}

	if(IMGPAR MbaffFrameFlag && currMB_r->mb_field)
	{
		IMGPAR num_ref_idx_l0_active <<=1;
		IMGPAR num_ref_idx_l1_active <<=1;
	}

	if ((currMB_r->mb_type&INTRA_MODE) == 0)
	{
		
		currMB_r->NoMbPartLessThan8x8Flag = 1;

		currMB_r->luma_transform_size_8x8_flag = currMB_s_r->luma_transform_size_8x8_flag = 0;

		if(active_pps.constrained_intra_pred_flag)
		{
			currMB_r->intra_block = 0;
		}

		//--- init macroblock data ---
		init_macroblock ARGS0();

		//====== READ 8x8 SUB-PARTITION MODES (modes of 8x8 blocks) and Intra VBST block modes ======
		if (IS_P8x8 (currMB_r))
		{
			for (int i=0; i<4; i++)
			{
				int sub_mb_type = readB8_typeInfo_CABAC ARGS0();
				currMB_r->b8mode[i]   = reinterpret_cast<unsigned char(*)[2]>(v2b8pd)[sub_mb_type][0];
				currMB_r->b8pdir[i]   = reinterpret_cast<unsigned char(*)[2]>(v2b8pd)[sub_mb_type][1];

				//set NoMbPartLessThan8x8Flag for PB_8x8 mode
				currMB_r->NoMbPartLessThan8x8Flag &= (currMB_r->b8mode[i]==0 && active_sps.direct_8x8_inference_flag) || 
					(currMB_r->b8mode[i]==4);
			}

			ret = readMotionInfoFromNAL ARGS0();
			if (FAILED(ret)) {
				return ret;
			}
			readCBPandCoeffsFromNAL_CABAC ARGS0();
		}
		else if (currMB_r->mb_type == 0) //COPY mode
		{
			MotionVector pmv;
			int zeroMotionAbove;
			int zeroMotionLeft;
			PixelPos mb_a;
			PixelPos* p_mb_a = &mb_a;
			Macroblock_s *mbs_a, *mbs_b;
			int      a_mv_y = 0;
			int      a_ref_idx = 0;
			int      b_mv_y = 0;
			int      b_ref_idx = 0;
			int      list_offset = ((IMGPAR MbaffFrameFlag)&&(currMB_r->mb_field))? 4 : 0;



			p_mb_a = &(IMGPAR left[0]);

			if (p_mb_a->pMB)
			{
				mbs_a     = &dec_picture->mb_data[p_mb_a->mb_addr];
				a_mv_y    = mbs_a->pred_info.mv[LIST_0][p_mb_a->y*4+p_mb_a->x].y;
				a_ref_idx = mbs_a->pred_info.ref_idx[LIST_0][l_16_4[p_mb_a->y*4+p_mb_a->x]];

				if (currMB_s_r->mb_field ^ mbs_a->mb_field)
				{
					if(currMB_s_r->mb_field)
					{
						a_mv_y   /= 2;
						a_ref_idx <<= 1;
					}
					else
					{
						a_mv_y   <<= 1;
						a_ref_idx >>= 1;
					}
				}
			}

			if (IMGPAR pUpMB_r)
			{
				mbs_b     = &dec_picture->mb_data[IMGPAR mbAddrB];
				b_mv_y    = mbs_b->pred_info.mv[LIST_0][3*4+0].y;
				b_ref_idx = mbs_b->pred_info.ref_idx[LIST_0][l_16_4[3*4+0]]; 

				if (currMB_s_r->mb_field ^ mbs_b->mb_field)
				{
					if(currMB_s_r->mb_field)
					{
						b_mv_y    /= 2;
						b_ref_idx  <<= 1;
					}
					else
					{
						b_mv_y    <<= 1;
						b_ref_idx  >>= 1;
					}
				}
			}

			zeroMotionLeft  = p_mb_a->pMB ? (a_ref_idx==0 && mbs_a->pred_info.mv[LIST_0][p_mb_a->y*4+p_mb_a->x].x==0 && a_mv_y==0 ? 1 : 0) : 1;
			zeroMotionAbove = IMGPAR pUpMB_r ? (b_ref_idx==0 && mbs_b->pred_info.mv[LIST_0][3*4+0].x==0 && b_mv_y==0 ? 1 : 0) : 1;

			currMB_r->cbp = 0;

			if (zeroMotionAbove || zeroMotionLeft)
			{
				memset(currMB_s_r->pred_info.mv[0], 0, sizeof(currMB_s_r->pred_info.mv[0]));
			}
			else
			{
				SetMotionVectorPredictor_block00_shape16x16 ARGS6(&pmv,0, LIST_0, IMGPAR current_mb_nr_r, currMB_r, currMB_s_r);

				Set16MotionVector(currMB_s_r->pred_info.mv[0], pmv);
			}

			// Intel compiler 9.0 has fixed this
			memset(&(currMB_s_r->pred_info.ref_idx[0][0]),0,4);
#if !defined(_COLLECT_PIC_)
			memset(&(currMB_s_r->pred_info.ref_pic_id[0][0]),(char) listX[LIST_0+list_offset][0]->unique_id,4);
#endif
		}
		else
		{
			if (IS_INTERMV (currMB_r)) {
				ret = readMotionInfoFromNAL ARGS0();
				if (FAILED(ret)) {
					return ret;
				}
			}

			readCBPandCoeffsFromNAL_CABAC ARGS0();
		}
	}
	else
	{
		currMB_r->intra_block = 1;

		currMB_r->NoMbPartLessThan8x8Flag = 1;

		//============= Transform Size Flag for INTRA MBs =============
		//-------------------------------------------------------------
		//transform size flag for INTRA_4x4 and INTRA_8x8 modes
		if (currMB_r->mb_type == I4MB && IMGPAR Transform8x8Mode)
		{
			currMB_r->luma_transform_size_8x8_flag = currMB_s_r->luma_transform_size_8x8_flag =
				readMB_transform_size_flag_CABAC ARGS0();

			if (currMB_r->luma_transform_size_8x8_flag)
			{
				currMB_r->mb_type = I8MB;
				*(int *) &currMB_r->b8mode[0] = I8MB | (I8MB<<8) | (I8MB<<16) | (I8MB<<24);
				*(int *) &currMB_r->b8pdir[0] = (int) -1;
				//for (i=0;i<4;i++) {currMB_r->b8mode[i]=I8MB; currMB_r->b8pdir[i]=-1; }
			}
		}
		else
		{
			currMB_r->luma_transform_size_8x8_flag = currMB_s_r->luma_transform_size_8x8_flag = 0;
		}

		//--- init macroblock data ---
		init_macroblock ARGS0();

		if(currMB_r->mb_type!=IPCM)
		{
			// intra prediction modes for a macroblock 4x4 **********************************************
			g_read_functions->read_ipred_modes ARGS0();
			// read CBP and Coeffs  ***************************************************************
			readCBPandCoeffsFromNAL_CABAC ARGS0();
		}
		else
		{
			//read pcm_alignment_zero_bit and pcm_byte[i]

			// here dep is assigned with the same dep as SE_MBTYPE, because IPCM syntax is in the 
			// same category as MBTYPE
#ifdef CONFIG_BIARI_ENABLE_MMX
			load_dep ARGS0();
#endif
			ret = readIPCMcoeffsFromNAL_CABAC ARGS0();
#ifdef CONFIG_BIARI_ENABLE_MMX
			store_dep ARGS0();
#endif
			if (FAILED(ret)) {
				return ret;
			}
		}
	}

#ifdef CONFIG_BIARI_ENABLE_MMX
	load_dep ARGS0();
#endif

	return CREL_OK;
}
CREL_RETURN read_one_macroblock_CABAC_P_even PARGS0()
{
	int mb_nr = IMGPAR current_mb_nr_r;
	CREL_RETURN ret;

#ifdef EI_FLAG  
	DecodingEnvironment *dep;
	dep = &(IMGPAR g_dep);
#endif
	g_read_functions = &(IMGPAR currentSlice->readSyntax);

#ifdef _COLLECT_PIC_
	currMB_s_r->do_record = 0;
#endif


	currMB_r->mb_field  = ( IMGPAR currentSlice->structure != FRAME );	

	//  read MB mode *****************************************************************
#ifdef CONFIG_BIARI_ENABLE_MMX
	store_dep ARGS0();
#endif

	if (IMGPAR MbaffFrameFlag)
	{
		//field_flag_inference();
		if (IMGPAR pLeftMB_r)
		{			
			currMB_r->mb_field = IMGPAR pLeftMB_r->mb_field;
		}
		else
		{
			// check top macroblock pair
			if (IMGPAR pUpMB_r)	{
				currMB_r->mb_field = IMGPAR pUpMB_r->mb_field;				
			} else {
				currMB_r->mb_field = 0;
			}			
		}
	}

	CheckAvailabilityOfNeighborsCABAC_even ARGS0();

	currMB_r->skip_flag = !readMB_skip_flagInfo_CABAC ARGS0();
	//currMB_r->skip_flag = !(currMB_r->mb_type);

#ifdef EI_FLAG
	if(!dep->Dei_flag)
		currMB_r->ei_flag = 0;
#endif

	// read MB AFF
	if (IMGPAR MbaffFrameFlag)
	{
		char mb_field_pred = currMB_r->mb_field;
		if (!currMB_r->skip_flag)
			currMB_r->mb_field = readFieldModeInfo_CABAC ARGS0();
		else
			check_next_mb_and_get_field_mode_CABAC_even ARGS0();
		/*
		if ( mb_field_pred != currMB_r->mb_field ) {
		//CheckAvailabilityOfNeighborsCABAC_even ARGS0();
		Macroblock* pUpMB;

		pUpMB = IMGPAR pUpMB_r;
		if ( pUpMB == NULL ) {
		currMB_r->mb_available_up = NULL;
		} else {
		if(!(currMB_r->mb_field && pUpMB->mb_field) ) {
		pUpMB++;
		}
		currMB_r->mb_available_up = pUpMB;
		}
		}
		*/
	}

	CheckAvailabilityOfNeighbors_ABCD_even ARGS0();

	// read MB type
	//if (currMB_r->mb_type != 0 )
	if (currMB_r->skip_flag == 0 )
	{
		currMB_r->mb_type = readMB_typeInfo_CABAC ARGS0();

#ifdef EI_FLAG
		if(!dep->Dei_flag)
			currMB_r->ei_flag = 0;
#endif
	} else {
		currMB_r->mb_type = 0;
	}

	currMB_s_r->mb_field = currMB_r->mb_field;

	if (IMGPAR currentSlice->LFDisableIdc==2)
	{
		// don't filter at slice boundaries
		currMB_s_r->filterLeftMbEdgeFlag = (IMGPAR pLeftMB_r != NULL);
		// if this the bottom of a frame macroblock pair then always filter the top edge
		currMB_s_r->filterTopMbEdgeFlag  = (IMGPAR pUpMB_r != NULL);
	}

	ret = interpret_mb_mode_P ARGS0();
	if (FAILED(ret)) {
		return ret;
	}

	if(IMGPAR MbaffFrameFlag && currMB_r->mb_field)
	{
		IMGPAR num_ref_idx_l0_active <<=1;
		IMGPAR num_ref_idx_l1_active <<=1;
	}

	if ((currMB_r->mb_type&INTRA_MODE) == 0)
	{
		
		currMB_r->NoMbPartLessThan8x8Flag = 1;

		currMB_r->luma_transform_size_8x8_flag = currMB_s_r->luma_transform_size_8x8_flag = 0;

		if(active_pps.constrained_intra_pred_flag)
		{
			currMB_r->intra_block = 0;
		}

		//--- init macroblock data ---
		init_macroblock  ARGS0();

		//====== READ 8x8 SUB-PARTITION MODES (modes of 8x8 blocks) and Intra VBST block modes ======
		if (IS_P8x8 (currMB_r))
		{
			for (int i=0; i<4; i++)
			{
				int sub_mb_type = readB8_typeInfo_CABAC ARGS0();
				currMB_r->b8mode[i]   = reinterpret_cast<unsigned char(*)[2]>(v2b8pd)[sub_mb_type][0];
				currMB_r->b8pdir[i]   = reinterpret_cast<unsigned char(*)[2]>(v2b8pd)[sub_mb_type][1];

				//set NoMbPartLessThan8x8Flag for PB_8x8 mode
				currMB_r->NoMbPartLessThan8x8Flag &= (currMB_r->b8mode[i]==0 && active_sps.direct_8x8_inference_flag) || 
					(currMB_r->b8mode[i]==4);
			}

			ret = readMotionInfoFromNAL ARGS0();
			if (FAILED(ret)) {
				return ret;
			}
			readCBPandCoeffsFromNAL_CABAC ARGS0();
		}
		else if (currMB_r->mb_type == 0) //COPY mode
		{
			MotionVector pmv;
			int zeroMotionAbove;
			int zeroMotionLeft;
			PixelPos mb_a;
			PixelPos* p_mb_a = &mb_a;
			Macroblock_s *mbs_a, *mbs_b;
			int      a_mv_y = 0;
			int      a_ref_idx = 0;
			int      b_mv_y = 0;
			int      b_ref_idx = 0;
			int      list_offset = ((IMGPAR MbaffFrameFlag)&&(currMB_r->mb_field))? 2 : 0;			

			p_mb_a = &(IMGPAR left[0]);

			if (p_mb_a->pMB)
			{
				mbs_a     = &dec_picture->mb_data[p_mb_a->mb_addr];
				a_mv_y    = mbs_a->pred_info.mv[LIST_0][p_mb_a->y*4+p_mb_a->x].y;
				a_ref_idx = mbs_a->pred_info.ref_idx[LIST_0][l_16_4[p_mb_a->y*4+p_mb_a->x]];

				if (currMB_s_r->mb_field ^ mbs_a->mb_field)
				{
					if(currMB_s_r->mb_field)
					{
						a_mv_y   /= 2;
						a_ref_idx <<= 1;
					}
					else
					{
						a_mv_y   <<= 1;
						a_ref_idx >>= 1;
					}
				}
			}

			if (IMGPAR pUpMB_r)
			{
				mbs_b     = &dec_picture->mb_data[IMGPAR mbAddrB];
				b_mv_y    = mbs_b->pred_info.mv[LIST_0][3*4+0].y;
				b_ref_idx = mbs_b->pred_info.ref_idx[LIST_0][l_16_4[3*4+0]]; 

				if (currMB_s_r->mb_field ^ mbs_b->mb_field)
				{
					if(currMB_s_r->mb_field)
					{
						b_mv_y    /= 2;
						b_ref_idx  <<= 1;
					}
					else
					{
						b_mv_y    <<= 1;
						b_ref_idx  >>= 1;
					}
				}
			}

			zeroMotionLeft  = p_mb_a->pMB ? (a_ref_idx==0 && mbs_a->pred_info.mv[LIST_0][p_mb_a->y*4+p_mb_a->x].x==0 && a_mv_y==0 ? 1 : 0) : 1;
			zeroMotionAbove = IMGPAR pUpMB_r ? (b_ref_idx==0 && mbs_b->pred_info.mv[LIST_0][3*4+0].x==0 && b_mv_y==0 ? 1 : 0) : 1;

			currMB_r->cbp = 0;

			if (zeroMotionAbove || zeroMotionLeft)
			{
				memset(currMB_s_r->pred_info.mv[0], 0, sizeof(currMB_s_r->pred_info.mv[0]));
			}
			else
			{
				SetMotionVectorPredictor_block00_shape16x16 ARGS6(&pmv,0, LIST_0, IMGPAR current_mb_nr_r, currMB_r, currMB_s_r);

				Set16MotionVector(currMB_s_r->pred_info.mv[0], pmv);
			}

			// Intel compiler 9.0 has fixed this
			memset(&(currMB_s_r->pred_info.ref_idx[0][0]),0,4);
#if !defined(_COLLECT_PIC_)
			memset(&(currMB_s_r->pred_info.ref_pic_id[0][0]),(char) listX[LIST_0+list_offset][0]->unique_id,4);
#endif
		}
		else
		{
			if (IS_INTERMV (currMB_r)) {
				ret = readMotionInfoFromNAL ARGS0();
				if (FAILED(ret)) {
					return ret;
				}
			}

			readCBPandCoeffsFromNAL_CABAC ARGS0();
		}
	}
	else
	{
		currMB_r->intra_block = 1;

		currMB_r->NoMbPartLessThan8x8Flag = 1;

		//============= Transform Size Flag for INTRA MBs =============
		//-------------------------------------------------------------
		//transform size flag for INTRA_4x4 and INTRA_8x8 modes
		if (currMB_r->mb_type == I4MB && IMGPAR Transform8x8Mode)
		{
			currMB_r->luma_transform_size_8x8_flag = currMB_s_r->luma_transform_size_8x8_flag =
				readMB_transform_size_flag_CABAC ARGS0();

			if (currMB_r->luma_transform_size_8x8_flag)
			{
				currMB_r->mb_type = I8MB;
				*(int *) &currMB_r->b8mode[0] = I8MB | (I8MB<<8) | (I8MB<<16) | (I8MB<<24);
				*(int *) &currMB_r->b8pdir[0] = (int) -1;
				//for (i=0;i<4;i++) {currMB_r->b8mode[i]=I8MB; currMB_r->b8pdir[i]=-1; }
			}
		}
		else
		{
			currMB_r->luma_transform_size_8x8_flag = currMB_s_r->luma_transform_size_8x8_flag = 0;
		}

		//--- init macroblock data ---
		init_macroblock ARGS0();

		if(currMB_r->mb_type!=IPCM)
		{
			// intra prediction modes for a macroblock 4x4 **********************************************
			g_read_functions->read_ipred_modes ARGS0();
			// read CBP and Coeffs  ***************************************************************
			readCBPandCoeffsFromNAL_CABAC ARGS0();
		}
		else
		{
			//read pcm_alignment_zero_bit and pcm_byte[i]

			// here dep is assigned with the same dep as SE_MBTYPE, because IPCM syntax is in the 
			// same category as MBTYPE
#ifdef CONFIG_BIARI_ENABLE_MMX
			load_dep ARGS0();
#endif
			ret = readIPCMcoeffsFromNAL_CABAC ARGS0();
#ifdef CONFIG_BIARI_ENABLE_MMX
			store_dep ARGS0();
#endif
			if (FAILED(ret)) {
				return ret;
			}
		}
	}

#ifdef CONFIG_BIARI_ENABLE_MMX
	load_dep ARGS0();
#endif

	return CREL_OK;
}
#else
int read_one_macroblock_CABAC_P PARGS0()
{
	int mb_nr = IMGPAR current_mb_nr_r;
	CREL_RETURN ret;
#ifdef EI_FLAG  
	DecodingEnvironment *dep;
	dep = &(IMGPAR g_dep);
#endif
	g_read_functions = &(IMGPAR currentSlice->readSyntax);

#ifdef _COLLECT_PIC_
	currMB_s_r->do_record = 0;
#endif

	int  check_bottom, read_bottom, read_top;

	if ( IMGPAR MbaffFrameFlag ) {
		if (mb_nr&1)
			currMB_r->mb_field = IMGPAR pLastMB_r->mb_field;
		else
			currMB_r->mb_field  = ( IMGPAR currentSlice->structure != FRAME );
	} else {
		currMB_r->mb_field  = ( IMGPAR currentSlice->structure != FRAME );
	}

	//currMB_s_r->qp = IMGPAR qp;

	//  read MB mode *****************************************************************
#ifdef CONFIG_BIARI_ENABLE_MMX
	store_dep ARGS0();
#endif

	if (IMGPAR MbaffFrameFlag)
	{
		if (((mb_nr&1) == 0) || (IMGPAR type!=B_SLICE && IMGPAR pLastMB_r->mb_type == 0))
		{
			//field_flag_inference();
			if (IMGPAR pLeftMB_r)
			{
				currMB_r->mb_field = IMGPAR pLeftMB_r->mb_field;
			}
			else
			{
				// check top macroblock pair
				if (IMGPAR pUpMB_r)
				{
					currMB_r->mb_field = IMGPAR pUpMB_r->mb_field;
				}
				else
					currMB_r->mb_field = 0;
			}
		}
	}

	if (mb_nr&1)
		CheckAvailabilityOfNeighborsCABAC_odd ARGS0();
	else
		CheckAvailabilityOfNeighborsCABAC_even ARGS0();

	currMB_r->mb_type = readMB_skip_flagInfo_CABAC ARGS0();
	currMB_r->skip_flag = !(currMB_r->mb_type);

#ifdef EI_FLAG
	if(!dep->Dei_flag)
		currMB_r->ei_flag = 0;
#endif

	// read MB AFF
	if (IMGPAR MbaffFrameFlag)
	{
		check_bottom=read_bottom=read_top=0;
		if ((mb_nr&1)==0)
		{
			check_bottom =  currMB_r->skip_flag;
			read_top = !check_bottom;
		}
		else
		{
			read_bottom = (IMGPAR pLastMB_r->skip_flag && (!currMB_r->skip_flag));
		}

		if (read_bottom || read_top)
		{
			currMB_r->mb_field = readFieldModeInfo_CABAC ARGS0();
		}
		if (check_bottom)
			check_next_mb_and_get_field_mode_CABAC_even ARGS0();
	}

	if (mb_nr&1)
		CheckAvailabilityOfNeighbors_ABCD_odd ARGS0();
	else
		CheckAvailabilityOfNeighbors_ABCD_even ARGS0();

	// read MB type
	if (currMB_r->mb_type != 0 )
	{
		currMB_r->mb_type = readMB_typeInfo_CABAC ARGS0();

#ifdef EI_FLAG
		if(!dep->Dei_flag)
			currMB_r->ei_flag = 0;
#endif
	}

	currMB_s_r->mb_field = currMB_r->mb_field;

	if (IMGPAR currentSlice->LFDisableIdc==2)
	{
		// don't filter at slice boundaries
		currMB_s_r->filterLeftMbEdgeFlag = (IMGPAR pLeftMB_r != NULL);
		// if this the bottom of a frame macroblock pair then always filter the top edge
		currMB_s_r->filterTopMbEdgeFlag  = IMGPAR pUpMB_r || (IMGPAR MbaffFrameFlag && !currMB_s_r->mb_field && (mb_nr & 1));
	}

	ret = interpret_mb_mode_P ARGS0();
	if (FAILED(ret)) {
		return ret;
	}

	if(IMGPAR MbaffFrameFlag && currMB_r->mb_field)
	{
		IMGPAR num_ref_idx_l0_active <<=1;
		IMGPAR num_ref_idx_l1_active <<=1;
	}

	if ((currMB_r->mb_type&INTRA_MODE) == 0)
	{
		
		currMB_r->NoMbPartLessThan8x8Flag = 1;

		currMB_r->luma_transform_size_8x8_flag = currMB_s_r->luma_transform_size_8x8_flag = 0;

		if(active_pps.constrained_intra_pred_flag)
		{
			currMB_r->intra_block = 0;
		}

		//--- init macroblock data ---
		init_macroblock       ARGS0();

		//====== READ 8x8 SUB-PARTITION MODES (modes of 8x8 blocks) and Intra VBST block modes ======
		if (IS_P8x8 (currMB_r))
		{
			for (int i=0; i<4; i++)
			{
				int sub_mb_type = readB8_typeInfo_CABAC ARGS0();
				currMB_r->b8mode[i]   = reinterpret_cast<unsigned char(*)[2]>(v2b8pd)[sub_mb_type][0];
				currMB_r->b8pdir[i]   = reinterpret_cast<unsigned char(*)[2]>(v2b8pd)[sub_mb_type][1];

				//set NoMbPartLessThan8x8Flag for PB_8x8 mode
				currMB_r->NoMbPartLessThan8x8Flag &= (currMB_r->b8mode[i]==0 && active_sps.direct_8x8_inference_flag) || 
					(currMB_r->b8mode[i]==4);
			}

			ret = readMotionInfoFromNAL ARGS0();
			if (FAILED(ret)) {
				return ret;
			}
			readCBPandCoeffsFromNAL_CABAC ARGS0();
		}
		else if (currMB_r->mb_type == 0) //COPY mode
		{
			MotionVector pmv;
			int zeroMotionAbove;
			int zeroMotionLeft;
			PixelPos mb_a;
			PixelPos* p_mb_a = &mb_a;
			Macroblock_s *mbs_a, *mbs_b;
			int      a_mv_y = 0;
			int      a_ref_idx = 0;
			int      b_mv_y = 0;
			int      b_ref_idx = 0;
			int      list_offset = ((IMGPAR MbaffFrameFlag)&&(currMB_r->mb_field))? (mb_nr&1) ? 4 : 2 : 0;

#if !defined(REC_NEIGHBOR)
			getLuma4x4Neighbour ARGS7(mb_nr,0,0,-1, 0,&mb_a, currMB_r);
			getLuma4x4Neighbour ARGS7(mb_nr,0,0, 0,-1,&mb_b, currMB_r);
#else

			p_mb_a = &(IMGPAR left[0]);

			//if (!(IMGPAR left_set[0]))
			//{
			//	getLuma4x4Neighbour_left_out ARGS7(mb_nr, 0, 0, -1,  0, p_mb_a, currMB_r);
			//	IMGPAR left_set[0] = 1;
			//}

			//if (!(IMGPAR up_set[0]))
			//{
			//	getLuma4x4Neighbour_top_out ARGS7(mb_nr, 0, 0,  0, -1, p_mb_b, currMB_r);
			//	IMGPAR up_set[0] = 1;
			//}
#endif

			if (p_mb_a->pMB)
			{
				mbs_a     = &dec_picture->mb_data[p_mb_a->mb_addr];
				a_mv_y    = mbs_a->pred_info.mv[LIST_0][p_mb_a->y*4+p_mb_a->x].y;
				a_ref_idx = mbs_a->pred_info.ref_idx[LIST_0][l_16_4[p_mb_a->y*4+p_mb_a->x]];

				if (currMB_s_r->mb_field ^ mbs_a->mb_field)
				{
					if(currMB_s_r->mb_field)
					{
						a_mv_y   /= 2;
						a_ref_idx <<= 1;
					}
					else
					{
						a_mv_y   <<= 1;
						a_ref_idx >>= 1;
					}
				}
			}

			if (IMGPAR pUpMB_r)
			{
				mbs_b     = &dec_picture->mb_data[IMGPAR mbAddrB];
				b_mv_y    = mbs_b->pred_info.mv[LIST_0][3*4+0].y;
				b_ref_idx = mbs_b->pred_info.ref_idx[LIST_0][l_16_4[3*4+0]]; 

				if (currMB_s_r->mb_field ^ mbs_b->mb_field)
				{
					if(currMB_s_r->mb_field)
					{
						b_mv_y    /= 2;
						b_ref_idx  <<= 1;
					}
					else
					{
						b_mv_y    <<= 1;
						b_ref_idx  >>= 1;
					}
				}
			}

			zeroMotionLeft  = p_mb_a->pMB ? (a_ref_idx==0 && mbs_a->pred_info.mv[LIST_0][p_mb_a->y*4+p_mb_a->x].x==0 && a_mv_y==0 ? 1 : 0) : 1;
			zeroMotionAbove = IMGPAR pUpMB_r ? (b_ref_idx==0 && mbs_b->pred_info.mv[LIST_0][3*4+0].x==0 && b_mv_y==0 ? 1 : 0) : 1;

			currMB_r->cbp = 0;

			if (zeroMotionAbove || zeroMotionLeft)
			{
				memset(currMB_s_r->pred_info.mv[0], 0, sizeof(currMB_s_r->pred_info.mv[0]));
			}
			else
			{
				SetMotionVectorPredictor_block00_shape16x16 ARGS6(&pmv,0, LIST_0, IMGPAR current_mb_nr_r, currMB_r, currMB_s_r);

				Set16MotionVector(currMB_s_r->pred_info.mv[0], pmv);
			}

			// Intel compiler 9.0 has fixed this
			memset(&(currMB_s_r->pred_info.ref_idx[0][0]),0,4);
#if !defined(_COLLECT_PIC_)
			memset(&(currMB_s_r->pred_info.ref_pic_id[0][0]),(char) listX[LIST_0+list_offset][0]->unique_id,4);
#endif
		}
		else
		{
			if (IS_INTERMV (currMB_r)) {
				ret = readMotionInfoFromNAL ARGS0();
				if (FAILED(ret)) {
					return ret;
				}
			}

			readCBPandCoeffsFromNAL_CABAC ARGS0();
		}
	}
	else
	{
		currMB_r->intra_block = 1;

		currMB_r->NoMbPartLessThan8x8Flag = 1;

		//============= Transform Size Flag for INTRA MBs =============
		//-------------------------------------------------------------
		//transform size flag for INTRA_4x4 and INTRA_8x8 modes
		if (currMB_r->mb_type == I4MB && IMGPAR Transform8x8Mode)
		{
			currMB_r->luma_transform_size_8x8_flag = currMB_s_r->luma_transform_size_8x8_flag =
				readMB_transform_size_flag_CABAC ARGS0();

			if (currMB_r->luma_transform_size_8x8_flag)
			{
				currMB_r->mb_type = I8MB;
				*(int *) &currMB_r->b8mode[0] = I8MB | (I8MB<<8) | (I8MB<<16) | (I8MB<<24);
				*(int *) &currMB_r->b8pdir[0] = (int) -1;
				//for (i=0;i<4;i++) {currMB_r->b8mode[i]=I8MB; currMB_r->b8pdir[i]=-1; }
			}
		}
		else
		{
			currMB_r->luma_transform_size_8x8_flag = currMB_s_r->luma_transform_size_8x8_flag = 0;
		}

		//--- init macroblock data ---
		init_macroblock ARGS0();

		if(currMB_r->mb_type!=IPCM)
		{
			// intra prediction modes for a macroblock 4x4 **********************************************
			g_read_functions->read_ipred_modes ARGS0();

			// read CBP and Coeffs  ***************************************************************
			readCBPandCoeffsFromNAL_CABAC ARGS0();
		}
		else
		{
			//read pcm_alignment_zero_bit and pcm_byte[i]

			// here dep is assigned with the same dep as SE_MBTYPE, because IPCM syntax is in the 
			// same category as MBTYPE
#ifdef CONFIG_BIARI_ENABLE_MMX
			load_dep ARGS0();
#endif
			ret = readIPCMcoeffsFromNAL_CABAC ARGS0();
#ifdef CONFIG_BIARI_ENABLE_MMX
			store_dep ARGS0();
#endif
			if (FAILED(ret)) {
				return ret;
			}
		}
	}

#ifdef CONFIG_BIARI_ENABLE_MMX
	load_dep ARGS0();
#endif

	return CREL_OK;
}
#endif

#if defined (_COLLECT_PIC_)
CREL_RETURN read_one_macroblock_CABAC_B_odd PARGS0()
{
	int mb_nr = IMGPAR current_mb_nr_r;
	CREL_RETURN ret;
#ifdef EI_FLAG  
	DecodingEnvironment *dep;
	dep = &(IMGPAR g_dep);
#endif
	g_read_functions = &(IMGPAR currentSlice->readSyntax);

#ifdef _COLLECT_PIC_
	currMB_s_r->do_record = 0;
#endif

	int  read_bottom;

	if ( IMGPAR MbaffFrameFlag ) {
		currMB_r->mb_field = IMGPAR pLastMB_r->mb_field;
	} else {
		currMB_r->mb_field  = ( IMGPAR currentSlice->structure != FRAME );		
	}



	//  read MB mode *****************************************************************
#ifdef CONFIG_BIARI_ENABLE_MMX
	store_dep ARGS0();
#endif

	if (IMGPAR MbaffFrameFlag)
	{
		if ((IMGPAR type==B_SLICE && IMGPAR pLastMB_r->skip_flag)) {
			if (IMGPAR pLeftMB_r)
			{
				currMB_r->mb_field = IMGPAR pLeftMB_r->mb_field;
			} else {
				// check top macroblock pair
				if (IMGPAR pUpMB_r)
				{
					currMB_r->mb_field = IMGPAR pUpMB_r->mb_field;
				}
				else
					currMB_r->mb_field = 0;
			}
		}
	}	

	CheckAvailabilityOfNeighborsCABAC_odd ARGS0();

	currMB_r->skip_flag = !readMB_skip_flagInfo_CABAC ARGS0();


#ifdef EI_FLAG
	if(!dep->Dei_flag)
		currMB_r->ei_flag = 0;
#endif

	if (currMB_r->skip_flag) {
		IMGPAR cod_counter=0;
	}

	// read MB AFF
	if (IMGPAR MbaffFrameFlag)
	{	

		read_bottom = (IMGPAR pLastMB_r->skip_flag && (!currMB_r->skip_flag));

		if (read_bottom )
		{
			char mb_field_pred = currMB_r->mb_field;
			currMB_r->mb_field = readFieldModeInfo_CABAC ARGS0();	
		}
	}

	CheckAvailabilityOfNeighbors_ABCD_odd ARGS0();

	// read MB type
	if (currMB_r->skip_flag == 0 ) 
	{
		currMB_r->mb_type = readMB_typeInfo_CABAC ARGS0();

#ifdef EI_FLAG
		if(!dep->Dei_flag)
			currMB_r->ei_flag = 0;
#endif
	} else {
		currMB_r->mb_type = 0;
	}

	currMB_s_r->mb_field = currMB_r->mb_field;

	if (IMGPAR currentSlice->LFDisableIdc==2)
	{
		// don't filter at slice boundaries
		currMB_s_r->filterLeftMbEdgeFlag = (IMGPAR pLeftMB_r != NULL);
		// if this the bottom of a frame macroblock pair then always filter the top edge
		currMB_s_r->filterTopMbEdgeFlag  = IMGPAR pUpMB_r || (IMGPAR MbaffFrameFlag && !currMB_s_r->mb_field);
	}

	ret = interpret_mb_mode_B ARGS0();
	if (FAILED(ret)) {
		return ret;
	}
	IMGPAR do_co_located = 1;

	if(IMGPAR MbaffFrameFlag && currMB_r->mb_field)
	{
		IMGPAR num_ref_idx_l0_active <<=1;
		IMGPAR num_ref_idx_l1_active <<=1;
	}

	if ((currMB_r->mb_type&INTRA_MODE) == 0)
	{
		
		if(!currMB_r->mb_type)
			compute_colocated_SUBMB ARGS6(Co_located_MB, listX,0,0,4,4);

		currMB_r->NoMbPartLessThan8x8Flag = (IS_DIRECT(currMB_r) && !(active_sps.direct_8x8_inference_flag))? 0: 1;

		currMB_r->luma_transform_size_8x8_flag = currMB_s_r->luma_transform_size_8x8_flag = 0;

		if(active_pps.constrained_intra_pred_flag)
		{
			currMB_r->intra_block = 0;
		}

		//--- init macroblock data ---
		init_macroblock ARGS0();

		//====== READ 8x8 SUB-PARTITION MODES (modes of 8x8 blocks) and Intra VBST block modes ======
		if (IS_P8x8 (currMB_r))
		{
			for (int i=0; i<4; i++)
			{
				int sub_mb_type = readB8_typeInfo_CABAC ARGS0();
				currMB_r->b8mode[i]   = reinterpret_cast<unsigned char(*)[2]>(v2b8pd)[sub_mb_type][0];
				currMB_r->b8pdir[i]   = reinterpret_cast<unsigned char(*)[2]>(v2b8pd)[sub_mb_type][1];

				if(!currMB_r->b8mode[i])
					compute_colocated_SUBMB ARGS6(Co_located_MB, listX, i&2, ((i&1)<<1), 2, 2);

				//set NoMbPartLessThan8x8Flag for PB_8x8 mode
				currMB_r->NoMbPartLessThan8x8Flag &= (currMB_r->b8mode[i]==0 && active_sps.direct_8x8_inference_flag) || 
					(currMB_r->b8mode[i]==4);
			}

			ret = readMotionInfoFromNAL ARGS0();
			if (FAILED(ret)) {
				return ret;
			}
			readCBPandCoeffsFromNAL_CABAC ARGS0();
		}
		else if (currMB_r->mb_type == 0 && IMGPAR cod_counter >= 0) //DIRECT mode
		{
			currMB_r->cbp = 0;

			IMGPAR cod_counter=-1;
		}
		else
		{
			if (IS_INTERMV (currMB_r)) {
				ret = readMotionInfoFromNAL ARGS0();
				if (FAILED(ret)) {
					return ret;
				}
			}

			readCBPandCoeffsFromNAL_CABAC ARGS0();
		}

#ifdef _COLLECT_PIC_
		if (IMGPAR do_co_located)
		{
			IMGPAR do_co_located = 0;

			if ((IS_DIRECT (currMB_r) || 
				(IS_P8x8(currMB_r) && !(currMB_r->b8mode[0] && currMB_r->b8mode[1] && currMB_r->b8mode[2] && currMB_r->b8mode[3]))))
			{
				if(IMGPAR direct_spatial_mv_pred_flag)
				{
					ret = IMGPAR FP_ReadMV_Direct_Spatial ARGS3(mb_nr, currMB_r, currMB_s_r);
				} else {
					ret = IMGPAR FP_ReadMV_Direct_Temproal ARGS3(mb_nr, currMB_r, currMB_s_r);
				}

				if (FAILED(ret)) {
					return ret;
				}

				currMB_s_r->do_record = 1;
				//record_reference_picIds ARGS2(list_offset, currMB_s);
			}
		}
#endif
	}
	else
	{
		currMB_r->intra_block = 1;

		currMB_r->NoMbPartLessThan8x8Flag = 1;

		//============= Transform Size Flag for INTRA MBs =============
		//-------------------------------------------------------------
		//transform size flag for INTRA_4x4 and INTRA_8x8 modes
		if (currMB_r->mb_type == I4MB && IMGPAR Transform8x8Mode)
		{
			currMB_r->luma_transform_size_8x8_flag = currMB_s_r->luma_transform_size_8x8_flag =
				readMB_transform_size_flag_CABAC ARGS0();

			if (currMB_r->luma_transform_size_8x8_flag)
			{
				currMB_r->mb_type = I8MB;
				*(int *) &currMB_r->b8mode[0] = I8MB | (I8MB<<8) | (I8MB<<16) | (I8MB<<24);
				*(int *) &currMB_r->b8pdir[0] = (int) -1;
				//for (i=0;i<4;i++) {currMB_r->b8mode[i]=I8MB; currMB_r->b8pdir[i]=-1; }
			}
		}
		else
		{
			currMB_r->luma_transform_size_8x8_flag = currMB_s_r->luma_transform_size_8x8_flag = 0;
		}

		//--- init macroblock data ---
		init_macroblock ARGS0();

		if(currMB_r->mb_type!=IPCM)
		{
			// intra prediction modes for a macroblock 4x4 **********************************************
			g_read_functions->read_ipred_modes ARGS0();
			// read CBP and Coeffs  ***************************************************************
			readCBPandCoeffsFromNAL_CABAC ARGS0();
		}
		else
		{
			//read pcm_alignment_zero_bit and pcm_byte[i]

			// here dep is assigned with the same dep as SE_MBTYPE, because IPCM syntax is in the 
			// same category as MBTYPE
#ifdef CONFIG_BIARI_ENABLE_MMX
			load_dep ARGS0();
#endif
			ret = readIPCMcoeffsFromNAL_CABAC ARGS0();
#ifdef CONFIG_BIARI_ENABLE_MMX
			store_dep ARGS0();
#endif
			if (FAILED(ret)) {
				return ret;
			}
		}
	}

#ifdef CONFIG_BIARI_ENABLE_MMX
	load_dep ARGS0();
#endif

	return CREL_OK;
}
CREL_RETURN read_one_macroblock_CABAC_B_even PARGS0()
{
	int mb_nr = IMGPAR current_mb_nr_r;
	CREL_RETURN ret;

#ifdef EI_FLAG  
	DecodingEnvironment *dep;
	dep = &(IMGPAR g_dep);
#endif
	g_read_functions = &(IMGPAR currentSlice->readSyntax);

#ifdef _COLLECT_PIC_
	currMB_s_r->do_record = 0;
#endif	

	currMB_r->mb_field  = ( IMGPAR currentSlice->structure != FRAME );	

	//  read MB mode *****************************************************************
#ifdef CONFIG_BIARI_ENABLE_MMX
	store_dep ARGS0();
#endif

	if (IMGPAR MbaffFrameFlag)
	{
		//field_flag_inference();
		if (IMGPAR pLeftMB_r)	{
			currMB_r->mb_field = IMGPAR pLeftMB_r->mb_field;
		} else {
			// check top macroblock pair
			if (IMGPAR pUpMB_r){
				currMB_r->mb_field = IMGPAR pUpMB_r->mb_field;
			} else {
				currMB_r->mb_field  = 0;
			}
		}
	}

	CheckAvailabilityOfNeighborsCABAC_even ARGS0();
	currMB_r->skip_flag = !readMB_skip_flagInfo_CABAC ARGS0();	


#ifdef EI_FLAG
	if(!dep->Dei_flag)
		currMB_r->ei_flag = 0;
#endif

	if (currMB_r->skip_flag) {
		IMGPAR cod_counter=0;
	}

	// read MB AFF
	if (IMGPAR MbaffFrameFlag)
	{
		char mb_field_pred = currMB_r->mb_field;

		if (currMB_r->skip_flag)
			check_next_mb_and_get_field_mode_CABAC_even ARGS0();
		else
			currMB_r->mb_field = readFieldModeInfo_CABAC ARGS0();
	}

	CheckAvailabilityOfNeighbors_ABCD_even ARGS0();

	// read MB type
	if (currMB_r->skip_flag == 0 )
	{
		currMB_r->mb_type = readMB_typeInfo_CABAC ARGS0();

#ifdef EI_FLAG
		if(!dep->Dei_flag)
			currMB_r->ei_flag = 0;
#endif
	} else {
		currMB_r->mb_type = 0;

	}

	currMB_s_r->mb_field = currMB_r->mb_field;

	if (IMGPAR currentSlice->LFDisableIdc==2)
	{
		// don't filter at slice boundaries
		currMB_s_r->filterLeftMbEdgeFlag = (IMGPAR pLeftMB_r != NULL);
		// if this the bottom of a frame macroblock pair then always filter the top edge
		currMB_s_r->filterTopMbEdgeFlag  = (IMGPAR pUpMB_r != NULL);
	}
	/*
	if (( img->number == 620) && (img->current_mb_nr_d == 2 )) {
	img->current_mb_nr_d = img->current_mb_nr_d;
	}
	*/
	ret = interpret_mb_mode_B ARGS0();
	if (FAILED(ret)) {
		return ret;
	}
	IMGPAR do_co_located = 1;

	if(IMGPAR MbaffFrameFlag && currMB_r->mb_field)
	{
		IMGPAR num_ref_idx_l0_active <<=1;
		IMGPAR num_ref_idx_l1_active <<=1;
	}

	if ((currMB_r->mb_type&INTRA_MODE) == 0)
	{
		
		if(!currMB_r->mb_type)
			compute_colocated_SUBMB ARGS6(Co_located_MB, listX,0,0,4,4);

		currMB_r->NoMbPartLessThan8x8Flag = (IS_DIRECT(currMB_r) && !(active_sps.direct_8x8_inference_flag))? 0: 1;

		currMB_r->luma_transform_size_8x8_flag = currMB_s_r->luma_transform_size_8x8_flag = 0;

		if(active_pps.constrained_intra_pred_flag)
		{
			currMB_r->intra_block = 0;
		}

		//--- init macroblock data ---
		init_macroblock ARGS0();

		//====== READ 8x8 SUB-PARTITION MODES (modes of 8x8 blocks) and Intra VBST block modes ======
		if (IS_P8x8 (currMB_r))
		{
			for (int i=0; i<4; i++)
			{
				int sub_mb_type = readB8_typeInfo_CABAC ARGS0();
				currMB_r->b8mode[i]   = reinterpret_cast<unsigned char(*)[2]>(v2b8pd)[sub_mb_type][0];
				currMB_r->b8pdir[i]   = reinterpret_cast<unsigned char(*)[2]>(v2b8pd)[sub_mb_type][1];

				if(!currMB_r->b8mode[i])
					compute_colocated_SUBMB ARGS6(Co_located_MB, listX, i&2, ((i&1)<<1), 2, 2);

				//set NoMbPartLessThan8x8Flag for PB_8x8 mode
				currMB_r->NoMbPartLessThan8x8Flag &= (currMB_r->b8mode[i]==0 && active_sps.direct_8x8_inference_flag) || 
					(currMB_r->b8mode[i]==4);
			}

			ret = readMotionInfoFromNAL ARGS0();
			if (FAILED(ret)) {
				return ret;
			}

			readCBPandCoeffsFromNAL_CABAC ARGS0();
		}
		else if (currMB_r->mb_type == 0 && IMGPAR cod_counter >= 0) //DIRECT mode
		{
			currMB_r->cbp = 0;

			IMGPAR cod_counter=-1;
		}
		else
		{
			if (IS_INTERMV (currMB_r)) {
				ret = readMotionInfoFromNAL ARGS0();
				if (FAILED(ret)) {
					return ret;
				}
			}

			readCBPandCoeffsFromNAL_CABAC ARGS0();
		}

#ifdef _COLLECT_PIC_
		if (IMGPAR do_co_located)
		{
			IMGPAR do_co_located = 0;

			if ((IS_DIRECT (currMB_r) || 
				(IS_P8x8(currMB_r) && !(currMB_r->b8mode[0] && currMB_r->b8mode[1] && currMB_r->b8mode[2] && currMB_r->b8mode[3]))))
			{
				if(IMGPAR direct_spatial_mv_pred_flag) {
					ret = IMGPAR FP_ReadMV_Direct_Spatial ARGS3(mb_nr, currMB_r, currMB_s_r);
				} else {
					ret = IMGPAR FP_ReadMV_Direct_Temproal ARGS3(mb_nr, currMB_r, currMB_s_r);
				}

				if (FAILED(ret)) {
					return ret;
				}

				currMB_s_r->do_record = 1;
				//record_reference_picIds ARGS2(list_offset, currMB_s);
			}
		}
#endif
	}
	else
	{
		currMB_r->intra_block = 1;

		currMB_r->NoMbPartLessThan8x8Flag = 1;

		//============= Transform Size Flag for INTRA MBs =============
		//-------------------------------------------------------------
		//transform size flag for INTRA_4x4 and INTRA_8x8 modes
		if (currMB_r->mb_type == I4MB && IMGPAR Transform8x8Mode)
		{
			currMB_r->luma_transform_size_8x8_flag = currMB_s_r->luma_transform_size_8x8_flag =
				readMB_transform_size_flag_CABAC ARGS0();

			if (currMB_r->luma_transform_size_8x8_flag)
			{
				currMB_r->mb_type = I8MB;
				*(int *) &currMB_r->b8mode[0] = I8MB | (I8MB<<8) | (I8MB<<16) | (I8MB<<24);
				*(int *) &currMB_r->b8pdir[0] = (int) -1;
				//for (i=0;i<4;i++) {currMB_r->b8mode[i]=I8MB; currMB_r->b8pdir[i]=-1; }
			}
		}
		else
		{
			currMB_r->luma_transform_size_8x8_flag = currMB_s_r->luma_transform_size_8x8_flag = 0;
		}

		//--- init macroblock data ---
		init_macroblock ARGS0();

		if(currMB_r->mb_type!=IPCM)
		{
			// intra prediction modes for a macroblock 4x4 **********************************************
			g_read_functions->read_ipred_modes ARGS0();
			// read CBP and Coeffs  ***************************************************************
			readCBPandCoeffsFromNAL_CABAC ARGS0();
		}
		else
		{
			//read pcm_alignment_zero_bit and pcm_byte[i]

			// here dep is assigned with the same dep as SE_MBTYPE, because IPCM syntax is in the 
			// same category as MBTYPE
#ifdef CONFIG_BIARI_ENABLE_MMX
			load_dep ARGS0();
#endif
			ret = readIPCMcoeffsFromNAL_CABAC ARGS0();
#ifdef CONFIG_BIARI_ENABLE_MMX
			store_dep ARGS0();
#endif
			if (FAILED(ret)) {
				return ret;
			}
		}
	}

#ifdef CONFIG_BIARI_ENABLE_MMX
	load_dep ARGS0();
#endif

	return CREL_OK;
}
#else
CREL_RETURN read_one_macroblock_CABAC_B PARGS0()
{
	int mb_nr = IMGPAR current_mb_nr_r;
	CREL_RETURN ret;
#ifdef EI_FLAG  
	DecodingEnvironment *dep;
	dep = &(IMGPAR g_dep);
#endif
	g_read_functions = &(IMGPAR currentSlice->readSyntax);

#ifdef _COLLECT_PIC_
	currMB_s_r->do_record = 0;
#endif

	int  check_bottom, read_bottom, read_top;

	if ( IMGPAR MbaffFrameFlag ) {
		if (mb_nr&1)
			currMB_r->mb_field = IMGPAR pLastMB_r->mb_field;
		else
			currMB_r->mb_field  = ( IMGPAR currentSlice->structure != FRAME );
	} else {
		currMB_r->mb_field  = ( IMGPAR currentSlice->structure != FRAME );		
	}

	//currMB_s_r->qp = IMGPAR qp;

	//  read MB mode *****************************************************************
#ifdef CONFIG_BIARI_ENABLE_MMX
	store_dep ARGS0();
#endif

	if (IMGPAR MbaffFrameFlag)
	{
		if (((mb_nr&1) == 0) || (IMGPAR type==B_SLICE && IMGPAR pLastMB_r->skip_flag))
		{
			//field_flag_inference();
			if (IMGPAR pLeftMB_r)
			{
				currMB_r->mb_field = IMGPAR pLeftMB_r->mb_field;
			}
			else
			{
				// check top macroblock pair
				if (IMGPAR pUpMB_r)
				{
					currMB_r->mb_field = IMGPAR pUpMB_r->mb_field;
				}
				else
					currMB_r->mb_field = 0;
			}
		}
	}

	if (mb_nr&1)
		CheckAvailabilityOfNeighborsCABAC_odd ARGS0();
	else
		CheckAvailabilityOfNeighborsCABAC_even ARGS0();

	currMB_r->mb_type = readMB_skip_flagInfo_CABAC ARGS0();
	currMB_r->skip_flag = !(currMB_r->mb_type);
	currMB_r->cbp = currMB_r->mb_type;

#ifdef EI_FLAG
	if(!dep->Dei_flag)
		currMB_r->ei_flag = 0;
#endif

	if (currMB_r->mb_type==0)
		IMGPAR cod_counter=0;

	// read MB AFF
	if (IMGPAR MbaffFrameFlag)
	{
		check_bottom=read_bottom=read_top=0;
		if ((mb_nr&1)==0)
		{
			check_bottom =  currMB_r->skip_flag;
			read_top = !check_bottom;
		}
		else
		{
			read_bottom = (IMGPAR pLastMB_r->skip_flag && (!currMB_r->skip_flag));
		}

		if (read_bottom || read_top)
			currMB_r->mb_field = readFieldModeInfo_CABAC ARGS0();

		if (check_bottom)
			check_next_mb_and_get_field_mode_CABAC_even ARGS0();
	}

	if (mb_nr&1)
		CheckAvailabilityOfNeighbors_ABCD_odd ARGS0();
	else
		CheckAvailabilityOfNeighbors_ABCD_even ARGS0();

	// read MB type
	if (currMB_r->mb_type != 0 )
	{
		currMB_r->mb_type = readMB_typeInfo_CABAC ARGS0();

#ifdef EI_FLAG
		if(!dep->Dei_flag)
			currMB_r->ei_flag = 0;
#endif
	}

	currMB_s_r->mb_field = currMB_r->mb_field;

	if (IMGPAR currentSlice->LFDisableIdc==2)
	{
		// don't filter at slice boundaries
		currMB_s_r->filterLeftMbEdgeFlag = (IMGPAR pLeftMB_r != NULL);
		// if this the bottom of a frame macroblock pair then always filter the top edge
		currMB_s_r->filterTopMbEdgeFlag  = IMGPAR pUpMB_r || (IMGPAR MbaffFrameFlag && !currMB_s_r->mb_field && (mb_nr & 1));
	}

	ret = interpret_mb_mode_B ARGS0();
	if (FAILED(ret)) {
		return ret;
	}
	IMGPAR do_co_located = 1;

	if(IMGPAR MbaffFrameFlag && currMB_r->mb_field)
	{
		IMGPAR num_ref_idx_l0_active <<=1;
		IMGPAR num_ref_idx_l1_active <<=1;
	}

	if ((currMB_r->mb_type&INTRA_MODE) == 0)
	{
		
		if(!currMB_r->mb_type)
			compute_colocated_SUBMB ARGS6(Co_located_MB, listX,0,0,4,4);

		currMB_r->NoMbPartLessThan8x8Flag = (IS_DIRECT(currMB_r) && !(active_sps.direct_8x8_inference_flag))? 0: 1;

		currMB_r->luma_transform_size_8x8_flag = currMB_s_r->luma_transform_size_8x8_flag = 0;

		if(active_pps.constrained_intra_pred_flag)
		{
			currMB_r->intra_block = 0;
		}

		//--- init macroblock data ---
		init_macroblock       ARGS0();

		//====== READ 8x8 SUB-PARTITION MODES (modes of 8x8 blocks) and Intra VBST block modes ======
		if (IS_P8x8 (currMB_r))
		{
			for (int i=0; i<4; i++)
			{
				int sub_mb_type = readB8_typeInfo_CABAC ARGS0();
				currMB_r->b8mode[i]   = reinterpret_cast<unsigned char(*)[2]>(v2b8pd)[sub_mb_type][0];
				currMB_r->b8pdir[i]   = reinterpret_cast<unsigned char(*)[2]>(v2b8pd)[sub_mb_type][1];

				if(!currMB_r->b8mode[i])
					compute_colocated_SUBMB ARGS6(Co_located_MB, listX, i&2, ((i&1)<<1), 2, 2);

				//set NoMbPartLessThan8x8Flag for PB_8x8 mode
				currMB_r->NoMbPartLessThan8x8Flag &= (currMB_r->b8mode[i]==0 && active_sps.direct_8x8_inference_flag) || 
					(currMB_r->b8mode[i]==4);
			}

			ret = readMotionInfoFromNAL ARGS0();
			if (FAILED(ret)) {
				return ret;
			}
			readCBPandCoeffsFromNAL_CABAC ARGS0();
		}
		else if (currMB_r->mb_type == 0 && IMGPAR cod_counter >= 0) //DIRECT mode
		{
			currMB_r->cbp = 0;

			IMGPAR cod_counter=-1;
		}
		else
		{
			if (IS_INTERMV (currMB_r)) {
				ret = readMotionInfoFromNAL ARGS0();
				if (FAILED(ret)) {
					return ret;
				}
			}

			readCBPandCoeffsFromNAL_CABAC ARGS0();
		}

#ifdef _COLLECT_PIC_
		if (IMGPAR do_co_located)
		{
			IMGPAR do_co_located = 0;

			if ((IS_DIRECT (currMB_r) || 
				(IS_P8x8(currMB_r) && !(currMB_r->b8mode[0] && currMB_r->b8mode[1] && currMB_r->b8mode[2] && currMB_r->b8mode[3]))))
			{
				if(IMGPAR direct_spatial_mv_pred_flag) {
					ret = IMGPAR FP_ReadMV_Direct_Spatial ARGS3(mb_nr, currMB_r, currMB_s_r);
				} else {
					ret = IMGPAR FP_ReadMV_Direct_Temproal ARGS3(mb_nr, currMB_r, currMB_s_r);
				}

				currMB_s_r->do_record = 1;
				//record_reference_picIds ARGS2(list_offset, currMB_s);
			}
		}
#endif
	}
	else
	{
		currMB_r->intra_block = 1;

		currMB_r->NoMbPartLessThan8x8Flag = 1;

		//============= Transform Size Flag for INTRA MBs =============
		//-------------------------------------------------------------
		//transform size flag for INTRA_4x4 and INTRA_8x8 modes
		if (currMB_r->mb_type == I4MB && IMGPAR Transform8x8Mode)
		{
			currMB_r->luma_transform_size_8x8_flag = currMB_s_r->luma_transform_size_8x8_flag =
				readMB_transform_size_flag_CABAC ARGS0();

			if (currMB_r->luma_transform_size_8x8_flag)
			{
				currMB_r->mb_type = I8MB;
				*(int *) &currMB_r->b8mode[0] = I8MB | (I8MB<<8) | (I8MB<<16) | (I8MB<<24);
				*(int *) &currMB_r->b8pdir[0] = (int) -1;
				//for (i=0;i<4;i++) {currMB_r->b8mode[i]=I8MB; currMB_r->b8pdir[i]=-1; }
			}
		}
		else
		{
			currMB_r->luma_transform_size_8x8_flag = currMB_s_r->luma_transform_size_8x8_flag = 0;
		}

		//--- init macroblock data ---
		init_macroblock ARGS0();

		if(currMB_r->mb_type!=IPCM)
		{
			// intra prediction modes for a macroblock 4x4 **********************************************
			g_read_functions->read_ipred_modes ARGS0();

			// read CBP and Coeffs  ***************************************************************
			readCBPandCoeffsFromNAL_CABAC ARGS0();
		}
		else
		{
			//read pcm_alignment_zero_bit and pcm_byte[i]

			// here dep is assigned with the same dep as SE_MBTYPE, because IPCM syntax is in the 
			// same category as MBTYPE
#ifdef CONFIG_BIARI_ENABLE_MMX
			load_dep ARGS0();
#endif
			ret = readIPCMcoeffsFromNAL_CABAC ARGS0();
#ifdef CONFIG_BIARI_ENABLE_MMX
			store_dep ARGS0();
#endif
			if (FAILED(ret)) {
				return ret;
			}
		}
	}

#ifdef CONFIG_BIARI_ENABLE_MMX
	load_dep ARGS0();
#endif

	return CREL_OK;
}
#endif
#else
CREL_RETURN read_one_macroblock_CABAC PARGS0()
{
	int i;
	int mb_nr = IMGPAR current_mb_nr_r;
	CREL_RETURN ret;
	Slice *currSlice = IMGPAR currentSlice;
#ifdef EI_FLAG  
	DecodingEnvironment *dep;
	dep = &(IMGPAR g_dep);
#endif
	read_functions_t *read_functions = g_read_functions = &(currSlice->readSyntax);

#ifdef _COLLECT_PIC_
	currMB_s_r->do_record = 0;
#endif

	int  img_block_y;
	int  check_bottom, read_bottom, read_top;

	if (mb_nr&1)
		currMB_r->mb_field = IMGPAR mb_decdata[mb_nr-1].mb_field;
	else
		currMB_r->mb_field = 0;

	//currMB_s_r->qp = IMGPAR qp;

	//  read MB mode *****************************************************************
#ifdef CONFIG_BIARI_ENABLE_MMX
	store_dep ARGS0();
#endif

	if(IMGPAR type == I_SLICE)
	{
		// read MB aff
		if (IMGPAR MbaffFrameFlag && ((mb_nr&1)==0))
		{
#ifdef EI_FLAG
			if (dep->Dei_flag)
#else
			if (0)
#endif
			{
				currMB_r->mb_field = readSyntaxElement_FLC ARGS1(1);
			}
			else
			{
				currMB_r->mb_field = readFieldModeInfo_CABAC ARGS0();
			}
		}

		CheckAvailabilityOfNeighborsCABAC ARGS0();

		//  read MB type
		currMB_r->mb_type = readMB_typeInfo_CABAC ARGS0();
#ifdef EI_FLAG
		if(!dep->Dei_flag)
			currMB_r->ei_flag = 0;
#endif
	}
	// non I/SI-slice CABAC
	else
	{
		// read MB skip_flag
		//if (IMGPAR MbaffFrameFlag && ((mb_nr&1) == 0||(IMGPAR type==B_SLICE && IMGPAR mb_decdata[mb_nr-1].skip_flag)||(IMGPAR type!=B_SLICE && IMGPAR mb_decdata[mb_nr-1].mb_type==0)))
		if (IMGPAR MbaffFrameFlag)
		{
			if ((mb_nr&1) == 0||(IMGPAR type==B_SLICE && IMGPAR mb_decdata[mb_nr-1].skip_flag)||(IMGPAR type!=B_SLICE && IMGPAR mb_decdata[mb_nr-1].mb_type==0))
			{
				//field_flag_inference();
				if (IMGPAR pLeftMB_r)
				{
					currMB_r->mb_field = IMGPAR mb_decdata[IMGPAR mbAddrA].mb_field;
				}
				else
				{
					// check top macroblock pair
					if (IMGPAR pUpMB_r)
					{
						currMB_r->mb_field = IMGPAR mb_decdata[IMGPAR mbAddrB].mb_field;
					}
					else
						currMB_r->mb_field = 0;
				}
			}
		}

		CheckAvailabilityOfNeighborsCABAC ARGS0();

		currMB_r->mb_type = readMB_skip_flagInfo_CABAC ARGS0();
		currMB_r->skip_flag = !(currMB_r->mb_type);

		if (IMGPAR type==B_SLICE)
			currMB_r->cbp = currMB_r->mb_type;

#ifdef EI_FLAG
		if(!dep->Dei_flag)
			currMB_r->ei_flag = 0;
#endif

		if (IMGPAR type==B_SLICE && currMB_r->mb_type==0)
			IMGPAR cod_counter=0;

		// read MB AFF
		if (IMGPAR MbaffFrameFlag)
		{
			check_bottom=read_bottom=read_top=0;
			if ((mb_nr&1)==0)
			{
				check_bottom =  currMB_r->skip_flag;
				read_top = !check_bottom;
			}
			else
			{
				read_bottom = (IMGPAR mb_decdata[mb_nr-1].skip_flag && (!currMB_r->skip_flag));
			}

			if (read_bottom || read_top)
				currMB_r->mb_field = readFieldModeInfo_CABAC ARGS0();

			if (check_bottom)
				check_next_mb_and_get_field_mode_CABAC ARGS0();
		}

		CheckAvailabilityOfNeighborsCABAC ARGS0();

		// read MB type
		if (currMB_r->mb_type != 0 )
		{
			currMB_r->mb_type = readMB_typeInfo_CABAC ARGS0();

#ifdef EI_FLAG
			if(!dep->Dei_flag)
				currMB_r->ei_flag = 0;
#endif
		}
	}

	currMB_s_r->mb_field = currMB_r->mb_field;
	if (mb_nr&1)
		CheckAvailabilityOfNeighbors_ABCD_odd ARGS0();
	else
		CheckAvailabilityOfNeighbors_ABCD_even ARGS0();

	if (IMGPAR currentSlice->LFDisableIdc==2)
	{
		// don't filter at slice boundaries
		currMB_s_r->filterLeftMbEdgeFlag = (IMGPAR pLeftMB_r != NULL);
		// if this the bottom of a frame macroblock pair then always filter the top edge
		currMB_s_r->filterTopMbEdgeFlag  = IMGPAR pUpMB_r || (IMGPAR MbaffFrameFlag && !currMB_s_r->mb_field && (mb_nr & 1));
	}

	if ((IMGPAR type==P_SLICE )) {    // inter frame
		ret = interpret_mb_mode_P ARGS0();
		if (FAILED(ret)) {
			return ret;
		}
	} else if (IMGPAR type==I_SLICE) {  // intra frame
		ret = interpret_mb_mode_I ARGS0();
		if (FAILED(ret)) {
			return ret;
		}
	} else if ((IMGPAR type==B_SLICE)) // B frame
	{
		ret = interpret_mb_mode_B ARGS0();
		if (FAILED(ret)) {
			return ret;
		}
		IMGPAR do_co_located = 1;
	}

	
	if(IMGPAR type == B_SLICE && !currMB_r->mb_type)
		compute_colocated_SUBMB ARGS6(Co_located_MB, listX,0,0,4,4);

	if(IMGPAR MbaffFrameFlag && currMB_r->mb_field)
	{
		IMGPAR num_ref_idx_l0_active <<=1;
		IMGPAR num_ref_idx_l1_active <<=1;
	}

	//init NoMbPartLessThan8x8Flag
	currMB_r->NoMbPartLessThan8x8Flag = (IS_DIRECT(currMB_r) && !(active_sps.direct_8x8_inference_flag))? 0: 1;

	//====== READ 8x8 SUB-PARTITION MODES (modes of 8x8 blocks) and Intra VBST block modes ======
	if (IS_P8x8 (currMB_r))
	{
		for (i=0; i<4; i++)
		{
			int sub_mb_type = readB8_typeInfo_CABAC ARGS0();
			currMB_r->b8mode[i]   = reinterpret_cast<unsigned char(*)[2]>(v2b8pd)[sub_mb_type][0];
			currMB_r->b8pdir[i]   = reinterpret_cast<unsigned char(*)[2]>(v2b8pd)[sub_mb_type][1];

			if(IMGPAR type == B_SLICE && !currMB_r->b8mode[i])
				compute_colocated_SUBMB ARGS6(Co_located_MB, listX, i&2, ((i&1)<<1), 2, 2);

			//set NoMbPartLessThan8x8Flag for PB_8x8 mode
			currMB_r->NoMbPartLessThan8x8Flag &= (currMB_r->b8mode[i]==0 && active_sps.direct_8x8_inference_flag) || 
				(currMB_r->b8mode[i]==4);
		}

		//--- init macroblock data ---
		init_macroblock       ARGS0();
		ret = readMotionInfoFromNAL ARGS0();
		if (FAILED(ret)) {
			return ret;
		}
	}

	//============= Transform Size Flag for INTRA MBs =============
	//-------------------------------------------------------------
	//transform size flag for INTRA_4x4 and INTRA_8x8 modes
	if (currMB_r->mb_type == I4MB && IMGPAR Transform8x8Mode)
	{
		currMB_r->luma_transform_size_8x8_flag = currMB_s_r->luma_transform_size_8x8_flag =
			readMB_transform_size_flag_CABAC ARGS0();

		if (currMB_r->luma_transform_size_8x8_flag)
		{
			currMB_r->mb_type = I8MB;
			*(int *) &currMB_r->b8mode[0] = I8MB | (I8MB<<8) | (I8MB<<16) | (I8MB<<24);
			*(int *) &currMB_r->b8pdir[0] = (int) -1;
			//for (i=0;i<4;i++) {currMB_r->b8mode[i]=I8MB; currMB_r->b8pdir[i]=-1; }
		}
	}
	else
	{
		currMB_r->luma_transform_size_8x8_flag = currMB_s_r->luma_transform_size_8x8_flag = 0;
	}

	if(active_pps.constrained_intra_pred_flag && (IMGPAR type==P_SLICE|| IMGPAR type==B_SLICE))        // inter frame
	{
		if( !IS_INTRA(currMB_r) )
		{
			currMB_r->intra_block = 0;
		}
	}

	//--- init macroblock data ---
	if (!IS_P8x8 (currMB_r))
		init_macroblock ARGS0();

	if (IS_DIRECT (currMB_r) && IMGPAR cod_counter >= 0)
	{
		currMB_r->cbp = 0;

		IMGPAR cod_counter=-1;

#ifdef _COLLECT_PIC_
		if (IMGPAR do_co_located)
		{
			IMGPAR do_co_located = 0;

			if ((IS_DIRECT (currMB_r) || 
				(IS_P8x8(currMB_r) && !(currMB_r->b8mode[0] && currMB_r->b8mode[1] && currMB_r->b8mode[2] && currMB_r->b8mode[3]))))
			{
				if(IMGPAR direct_spatial_mv_pred_flag) {
					ret = IMGPAR FP_ReadMV_Direct_Spatial ARGS3(mb_nr, currMB_r, currMB_s_r);
				} else {
					ret = IMGPAR FP_ReadMV_Direct_Temproal ARGS3(mb_nr, currMB_r, currMB_s_r);
				}

				currMB_s_r->do_record = 1;
				//record_reference_picIds ARGS2(list_offset, currMB_s);
			}
		}
#endif

#ifdef CONFIG_BIARI_ENABLE_MMX
		load_dep ARGS0();
#endif

		return DECODE_MB;
	}

	if (IS_COPY (currMB_r)) //keep last macroblock
	{
		MotionVector pmv;
		int zeroMotionAbove;
		int zeroMotionLeft;
		PixelPos mb_a, mb_b;
		PixelPos* p_mb_a = &mb_a;
		PixelPos* p_mb_b = &mb_b;
		Macroblock_s *mbs_a, *mbs_b;
		int      a_mv_y = 0;
		int      a_ref_idx = 0;
		int      b_mv_y = 0;
		int      b_ref_idx = 0;
		int      list_offset = ((IMGPAR MbaffFrameFlag)&&(currMB_r->mb_field))? (mb_nr&1) ? 4 : 2 : 0;

#if !defined(REC_NEIGHBOR)
		getLuma4x4Neighbour ARGS7(mb_nr,0,0,-1, 0,&mb_a, currMB_r);
		getLuma4x4Neighbour ARGS7(mb_nr,0,0, 0,-1,&mb_b, currMB_r);
#else

		p_mb_a = &(IMGPAR left[0]);

		//if (!(IMGPAR left_set[0]))
		//{
		//	getLuma4x4Neighbour_left_out ARGS7(mb_nr, 0, 0, -1,  0, p_mb_a, currMB_r);
		//	IMGPAR left_set[0] = 1;
		//}

		p_mb_b = &(IMGPAR up[0]);
		//if (!(IMGPAR up_set[0]))
		//{
		//	getLuma4x4Neighbour_top_out ARGS7(mb_nr, 0, 0,  0, -1, p_mb_b, currMB_r);
		//	IMGPAR up_set[0] = 1;
		//}
#endif

		if (p_mb_a->pMB)
		{
			mbs_a     = &dec_picture->mb_data[p_mb_a->mb_addr];
			a_mv_y    = mbs_a->pred_info.mv[LIST_0][p_mb_a->y*4+p_mb_a->x].y;
			a_ref_idx = mbs_a->pred_info.ref_idx[LIST_0][l_16_4[p_mb_a->y*4+p_mb_a->x]];

			if (currMB_s_r->mb_field ^ mbs_a->mb_field)
			{
				if(currMB_s_r->mb_field)
				{
					a_mv_y   /= 2;
					a_ref_idx <<= 1;
				}
				else
				{
					a_mv_y   <<= 1;
					a_ref_idx >>= 1;
				}
			}
		}

		if (p_mb_b->pMB)
		{
			mbs_b     = &dec_picture->mb_data[p_mb_b->mb_addr];
			b_mv_y    = mbs_b->pred_info.mv[LIST_0][p_mb_b->y*4+p_mb_b->x].y;
			b_ref_idx = mbs_b->pred_info.ref_idx[LIST_0][l_16_4[p_mb_b->y*4+p_mb_b->x]]; 

			if (currMB_s_r->mb_field ^ mbs_b->mb_field)
			{
				if(currMB_s_r->mb_field)
				{
					b_mv_y   /= 2;
					b_ref_idx <<= 1;
				}
				else
				{
					b_mv_y   <<= 1;
					b_ref_idx >>= 1;
				}
			}
		}

		zeroMotionLeft  = p_mb_a->pMB ? (a_ref_idx==0 && mbs_a->pred_info.mv[LIST_0][p_mb_a->y*4+p_mb_a->x].x==0 && a_mv_y==0 ? 1 : 0) : 1;
		zeroMotionAbove = p_mb_b->pMB ? (b_ref_idx==0 && mbs_b->pred_info.mv[LIST_0][p_mb_b->y*4+p_mb_b->x].x==0 && b_mv_y==0 ? 1 : 0) : 1;

		currMB_r->cbp = 0;

		img_block_y   = IMGPAR block_y_r;

		if (zeroMotionAbove || zeroMotionLeft)
		{
			memset(currMB_s_r->pred_info.mv[0], 0, sizeof(currMB_s_r->pred_info.mv[0]));
		}
		else
		{
			SetMotionVectorPredictor_block00_shape16x16 ARGS6(&pmv,0, LIST_0, IMGPAR current_mb_nr_r, currMB_r, currMB_s_r);

			Set16MotionVector(currMB_s_r->pred_info.mv[0], pmv);
		}

		// Intel compiler 9.0 has fixed this
		memset(&(currMB_s_r->pred_info.ref_idx[0][0]),0,4);
#if !defined(_COLLECT_PIC_)
		memset(&(currMB_s_r->pred_info.ref_pic_id[0][0]),(char) listX[LIST_0+list_offset][0]->unique_id,4);
#endif

#ifdef CONFIG_BIARI_ENABLE_MMX
		load_dep ARGS0();
#endif

		return DECODE_MB;
	}
	if(currMB_r->mb_type!=IPCM)
	{
		// intra prediction modes for a macroblock 4x4 **********************************************
		g_read_functions->read_ipred_modes ARGS0();

		// read inter frame vector data *********************************************************
		if (IS_INTERMV (currMB_r) && (!IS_P8x8(currMB_r)))
		{
			ret = readMotionInfoFromNAL ARGS0();
			if (FAILED(ret)) {
				return ret;
			}
		}
		// read CBP and Coeffs  ***************************************************************
		readCBPandCoeffsFromNAL_CABAC ARGS0();
#ifdef _COLLECT_PIC_
		if (IMGPAR do_co_located)
		{
			IMGPAR do_co_located = 0;

			if ((IS_DIRECT (currMB_r) || 
				(IS_P8x8(currMB_r) && !(currMB_r->b8mode[0] && currMB_r->b8mode[1] && currMB_r->b8mode[2] && currMB_r->b8mode[3]))))
			{
				if(IMGPAR direct_spatial_mv_pred_flag)
				{
					IMGPAR FP_ReadMV_Direct_Spatial ARGS3(mb_nr, currMB_r, currMB_s_r);
				}
				else
				{
					IMGPAR FP_ReadMV_Direct_Temproal ARGS3(mb_nr, currMB_r, currMB_s_r);
				}

				currMB_s_r->do_record = 1;
				//record_reference_picIds ARGS2(list_offset, currMB_s);
			}
		}
#endif 
	}
	else
	{
		//read pcm_alignment_zero_bit and pcm_byte[i]

		// here dep is assigned with the same dep as SE_MBTYPE, because IPCM syntax is in the 
		// same category as MBTYPE
#ifdef CONFIG_BIARI_ENABLE_MMX
		load_dep ARGS0();
#endif
		ret = readIPCMcoeffsFromNAL_CABAC ARGS0();
#ifdef CONFIG_BIARI_ENABLE_MMX
		store_dep ARGS0();
#endif
		if (FAILED(ret)) {
			return ret;
		}
	}
#ifdef CONFIG_BIARI_ENABLE_MMX
	load_dep ARGS0();
#endif

	return CREL_OK;
}
#endif
/*!
************************************************************************
* \brief
*    Initialize decoding engine after decoding an IPCM macroblock
*    (for IPCM CABAC  28/11/2003)
*
* \author
*    Dong Wang <Dong.Wang@bristol.ac.uk>  
************************************************************************
*/
CREL_RETURN init_decoding_engine_IPCM PARGS0()
{
	Slice *currSlice = IMGPAR currentSlice;
	DecodingEnvironment *dep;
	int PartitionNumber;
	int i;

	if(currSlice->dp_mode==PAR_DP_1)
		PartitionNumber=1;
	else if(currSlice->dp_mode==PAR_DP_3)
		PartitionNumber=3;
	else
	{
		DEBUG_SHOW_ERROR_INFO("Partition Mode is not supported\n");
		//exit(1);
		return CREL_ERROR_H264_UNDEFINED;
	}

	for(i=0;i<PartitionNumber;i++)
	{
		dep = &(IMGPAR g_dep);
		// Byte-align
		dep->Dcodestrm -= dep->Dbits_to_go>>3;
		arideco_start_decoding ARGS0();
	}

	return CREL_OK;
}



/*!
************************************************************************
* \brief
*    Read IPCM pcm_alignment_zero_bit and pcm_byte[i] from stream to IMGPAR cof
*    (for IPCM CAVLC)
*
* \author
*    Dong Wang <Dong.Wang@bristol.ac.uk>
************************************************************************
*/

void readIPCMcoeffsFromNAL_UVLC PARGS0()
{
	int i;
	unsigned char *cof_ptr;

	//read bits to let stream byte aligned

	if(IMGPAR g_dep.Dbits_to_go&7)
		readSyntaxElement_FLC ARGS1((IMGPAR g_dep.Dbits_to_go&7));

#if defined(ONE_COF)
	cof_ptr = (unsigned char *) &IMGPAR cof[0][0][0][0];
#else
	cof_ptr = (unsigned char *) IMGPAR cof_r;
#endif
	for(i=0;i<16*16;i++)
	{
		cof_ptr[i] = (unsigned char) readSyntaxElement_FLC ARGS1(8);
	}
#ifdef __SUPPORT_YUV400__
	if(active_sps.chroma_format_idc !=YUV400)
	{ // Support for YUV420 only
#endif
		// U
#if defined(ONE_COF)
		cof_ptr = (unsigned char *) &IMGPAR cof[4][0][0][0];
#else
		cof_ptr = (unsigned char *) (IMGPAR cof_r+ 256);
#endif
		for(i=0;i<8*8;i++)
		{
			cof_ptr[i] = (unsigned char) readSyntaxElement_FLC ARGS1(8);
		}
		// V
#if defined(ONE_COF)
		cof_ptr = (unsigned char *) &IMGPAR cof[5][0][0][0];
#else
		cof_ptr = (unsigned char *) (IMGPAR cof_r + 320);
#endif
		for(i=0;i<8*8;i++)
		{
			cof_ptr[i] = (unsigned char) readSyntaxElement_FLC ARGS1(8);
		}
#ifdef __SUPPORT_YUV400__
	}
#endif
}



/*!
************************************************************************
* \brief
*    Read IPCM pcm_alignment_zero_bit and pcm_byte[i] from stream to IMGPAR cof
*    (for IPCM CABAC)
*
* \author
*    Dong Wang <Dong.Wang@bristol.ac.uk>
************************************************************************
*/

CREL_RETURN readIPCMcoeffsFromNAL_CABAC PARGS0()
{
	int i;
	unsigned char *cof_ptr;
	CREL_RETURN ret;

	//For CABAC, we don't need to read bits to let stream byte aligned
	//  because we have variable for integer bytes position
	//read luma and chroma IPCM coefficients

#if defined(ONE_COF)
	cof_ptr = (unsigned char *) &IMGPAR cof[0][0][0][0];
#else
	cof_ptr = (unsigned char *) IMGPAR cof_r;
#endif
	for(i=0;i<16*16;i++)
	{
		cof_ptr[i] = readIPCMBytes_CABAC ARGS0();
	}
#ifdef __SUPPORT_YUV400__
	if(active_sps.chroma_format_idc !=YUV400)
	{ // Support for YUV420 only
#endif
		// U
#if defined(ONE_COF)
		cof_ptr = (unsigned char *) &IMGPAR cof[4][0][0][0];
#else
		cof_ptr = (unsigned char *) (IMGPAR cof_r+256);
#endif
		for(i=0;i<8*8;i++)
		{
			cof_ptr[i] = readIPCMBytes_CABAC ARGS0();
		}
		// V
#if defined(ONE_COF)
		cof_ptr = (unsigned char *) &IMGPAR cof[5][0][0][0];
#else
		cof_ptr = (unsigned char *) (IMGPAR cof_r+320);
#endif
		for(i=0;i<8*8;i++)
		{
			cof_ptr[i] = readIPCMBytes_CABAC ARGS0();
		}
#ifdef __SUPPORT_YUV400__
	}
#endif

	//If the decoded MB is IPCM MB, decoding engine is initialized

	// here the decoding engine is directly initialized without checking End of Slice
	// The reason is that, whether current MB is the last MB in slice or not, there is
	// at least one 'end of slice' syntax after this MB. So when fetching bytes in this 
	// initialisation process, we can guarantee there is bits available in bitstream. 

	ret = init_decoding_engine_IPCM ARGS0();

	return ret;

}

CREL_RETURN read_ipred_modes_NonMbAFF PARGS0()
{
	int b8,i,bx,by,dec;
	Slice *currSlice;
	int mostProbableIntraPredMode;
	int upIntraPredMode;
	int leftIntraPredMode;
	int IntraChromaPredModeFlag;
	int block_available = 0;
	int constrained_intra_pred_flag = active_pps.constrained_intra_pred_flag;
	int mb_nr = IMGPAR current_mb_nr_r;

	//PixelPos top_block;
	PixelPos *p_left_block;

	IntraChromaPredModeFlag = IS_INTRA(currMB_r);

	currSlice = IMGPAR currentSlice;

	read_functions_t *read_functions = &(currSlice->readSyntax);

	if(currMB_r->b8mode[0]==IBLOCK)
	{
		p_left_block = &(IMGPAR left[0]);
		for(i=0;i<16;i++)  //loop 4x4 blocks, peano-scan order
		{
			//get from stream
			int ipred4x4_mode_luma = read_functions->raw_ipred4x4_mode_luma ARGS0();

			by = peano_raster[i][0];
			bx = peano_raster[i][1];

			// left block
			if (bx)
			{
				leftIntraPredMode  = currMB_r->ipredmode[by*4+bx-1];
			}
			else
			{
				if(p_left_block->pMB)
				{
					int mb_type=p_left_block->pMB->mb_type;
					if (constrained_intra_pred_flag)
					{						
						if (p_left_block->pMB->intra_block)
						{
							if(mb_type&(I4MB|I8MB))
								leftIntraPredMode = p_left_block->pMB->ipredmode[(p_left_block->y<<2)+3];
							else
								leftIntraPredMode = DC_PRED;
						}
						else
							leftIntraPredMode = -1;
					}
					else
					{						
						if(mb_type&(I4MB|I8MB))
							leftIntraPredMode = p_left_block->pMB->ipredmode[(p_left_block->y<<2)+3];
						else
							leftIntraPredMode = DC_PRED;						
					}
				}
				else
					leftIntraPredMode = -1;


				p_left_block++;
			}
			// top block
			if (by)
			{
				upIntraPredMode = currMB_r->ipredmode[(by-1)*4+bx];	
			}
			else
			{
				if(IMGPAR pUpMB_r)
				{
					int mb_type=IMGPAR pUpMB_r->mb_type;
					if (constrained_intra_pred_flag)
					{						
						if (IMGPAR pUpMB_r->intra_block)
						{
							if(mb_type&(I4MB|I8MB))
								upIntraPredMode = IMGPAR pUpMB_r->ipredmode[12+bx];
							else
								upIntraPredMode = DC_PRED;
						}
						else
							upIntraPredMode  = -1;	
					}
					else
					{						
						if(mb_type&(I4MB|I8MB))
							upIntraPredMode = IMGPAR pUpMB_r->ipredmode[12+bx];	
						else
							upIntraPredMode = DC_PRED;						
					}
				}
				else
					upIntraPredMode = -1;
			}

			mostProbableIntraPredMode  = (upIntraPredMode < 0 || leftIntraPredMode < 0) ? DC_PRED : __fast_min(upIntraPredMode,leftIntraPredMode);

			dec = (ipred4x4_mode_luma == -1) ? mostProbableIntraPredMode : ipred4x4_mode_luma + (ipred4x4_mode_luma >= mostProbableIntraPredMode);

			currMB_r->ipredmode[by*4+bx]=dec;
		}
	}
	else if(currMB_r->b8mode[0]==I8MB)
	{
		p_left_block = &(IMGPAR left[0]);
		for(b8=0;b8<4;b8++)  //loop 8x8 blocks
		{      
			int ipred4x4_mode_luma = read_functions->raw_ipred4x4_mode_luma ARGS0();

			bx = ((b8&1)<<1);
			by = (b8&2);

			// left block
			if (bx)
			{
				leftIntraPredMode  = currMB_r->ipredmode[by*4+bx-1];
			}
			else
			{
				if(p_left_block->pMB)
				{
					int mb_type=p_left_block->pMB->mb_type;
					if (constrained_intra_pred_flag)
					{						
						if (p_left_block->pMB->intra_block)
						{
							if(mb_type&(I4MB|I8MB))
								leftIntraPredMode = p_left_block->pMB->ipredmode[(p_left_block->y<<2)+3];
							else
								leftIntraPredMode = DC_PRED;
						}
						else
							leftIntraPredMode = -1;
					}
					else
					{						
						if(mb_type&(I4MB|I8MB))
							leftIntraPredMode = p_left_block->pMB->ipredmode[(p_left_block->y<<2)+3];
						else
							leftIntraPredMode = DC_PRED;						
					}
				}
				else
					leftIntraPredMode = -1;

				p_left_block+=2;
			}
			// top block
			if (by)
			{
				upIntraPredMode = currMB_r->ipredmode[(by-1)*4+bx];	
			}
			else
			{
				if(IMGPAR pUpMB_r)
				{
					int mb_type=IMGPAR pUpMB_r->mb_type;
					if (constrained_intra_pred_flag)
					{						
						if (IMGPAR pUpMB_r->intra_block)
						{
							if(mb_type&(I4MB|I8MB))
								upIntraPredMode = IMGPAR pUpMB_r->ipredmode[12+bx];
							else
								upIntraPredMode = DC_PRED;
						}
						else
							upIntraPredMode  = -1;	
					}
					else
					{						
						if(mb_type&(I4MB|I8MB))
							upIntraPredMode = IMGPAR pUpMB_r->ipredmode[12+bx];	
						else
							upIntraPredMode = DC_PRED;						
					}
				}
				else
					upIntraPredMode = -1;
			}

			mostProbableIntraPredMode  = (upIntraPredMode < 0 || leftIntraPredMode < 0) ? DC_PRED : __fast_min(upIntraPredMode,leftIntraPredMode);

			dec = (ipred4x4_mode_luma == -1) ? mostProbableIntraPredMode : ipred4x4_mode_luma + (ipred4x4_mode_luma >= mostProbableIntraPredMode);

			//set
			currMB_r->ipredmode[(by+0)*4+bx+0]=
				currMB_r->ipredmode[(by+0)*4+bx+1]=
				currMB_r->ipredmode[(by+1)*4+bx+0]=
				currMB_r->ipredmode[(by+1)*4+bx+1]=dec;
		}
	}

	if (IntraChromaPredModeFlag
#ifdef __SUPPORT_YUV400__
		&& dec_picture->chroma_format_idc != YUV400
#endif
		)
	{
		currMB_r->c_ipred_mode = read_functions->raw_ipred4x4_mode_chroma ARGS0();

		if (currMB_r->c_ipred_mode < DC_PRED_8 || currMB_r->c_ipred_mode > PLANE_8)
		{
			DEBUG_SHOW_ERROR_INFO("[ERROR]illegal chroma intra pred mode!\n", 600);
			return CREL_WARNING_H264_STREAMERROR_LEVEL_3;
		}
	}

	SetIntraPredDecMode ARGS0();
	return CREL_OK;
}

CREL_RETURN read_ipred_modes_MbAFF PARGS0()
{
	int b8,i,bx,by,dec;
	Slice *currSlice;
	int mostProbableIntraPredMode;
	int upIntraPredMode;
	int leftIntraPredMode;
	int IntraChromaPredModeFlag;
	int block_available = 0;
	int constrained_intra_pred_flag = active_pps.constrained_intra_pred_flag;
	int mb_nr = IMGPAR current_mb_nr_r;

	PixelPos *p_left_block;

	IntraChromaPredModeFlag = IS_INTRA(currMB_r);

	currSlice = IMGPAR currentSlice;

	read_functions_t *read_functions = &(currSlice->readSyntax);

	if(currMB_r->b8mode[0]==IBLOCK )
	{
		p_left_block = &(IMGPAR left[0]);
		for(i=0;i<16;i++)  //loop 4x4 blocks, peano scan
		{
			//get from stream
			int ipred4x4_mode_luma = read_functions->raw_ipred4x4_mode_luma ARGS0();

			by = peano_raster[i][0];
			bx = peano_raster[i][1];

			// left block
			if (bx)
			{
				leftIntraPredMode  = currMB_r->ipredmode[by*4+bx-1];
			}
			else
			{
				if(p_left_block->pMB)
				{
					int mb_type=p_left_block->pMB->mb_type;
					if (constrained_intra_pred_flag)
					{						
						if (p_left_block->pMB->intra_block)
						{
							if(mb_type&(I4MB|I8MB))
								leftIntraPredMode = p_left_block->pMB->ipredmode[(p_left_block->y<<2)+3];
							else
								leftIntraPredMode = DC_PRED;
						}
						else
							leftIntraPredMode = -1;
					}
					else
					{						
						if(mb_type&(I4MB|I8MB))
							leftIntraPredMode = p_left_block->pMB->ipredmode[(p_left_block->y<<2)+3];
						else
							leftIntraPredMode = DC_PRED;						
					}
				}
				else
					leftIntraPredMode = -1;

				p_left_block++;
			}
			// top block
			if (by)
			{
				upIntraPredMode = currMB_r->ipredmode[(by-1)*4+bx];	
			}
			else
			{
				if(IMGPAR pUpMB_r)
				{
					int mb_type=IMGPAR pUpMB_r->mb_type;
					if (constrained_intra_pred_flag)
					{						
						if (IMGPAR pUpMB_r->intra_block)
						{
							if(mb_type&(I4MB|I8MB))
								upIntraPredMode = IMGPAR pUpMB_r->ipredmode[12+bx];
							else
								upIntraPredMode = DC_PRED;
						}
						else
							upIntraPredMode  = -1;	
					}
					else
					{						
						if(mb_type&(I4MB|I8MB))
							upIntraPredMode = IMGPAR pUpMB_r->ipredmode[12+bx];	
						else
							upIntraPredMode = DC_PRED;						
					}
				}
				else
					upIntraPredMode  = -1;
			}
			mostProbableIntraPredMode  = (upIntraPredMode < 0 || leftIntraPredMode < 0) ? DC_PRED : __fast_min(upIntraPredMode,leftIntraPredMode);

			dec = (ipred4x4_mode_luma == -1) ? mostProbableIntraPredMode : ipred4x4_mode_luma + (ipred4x4_mode_luma >= mostProbableIntraPredMode);

			//set
			currMB_r->ipredmode[by*4+bx]=dec;
		}
	}
	else if(currMB_r->b8mode[0]==I8MB)
	{
		p_left_block = &(IMGPAR left[0]);
		for(b8=0;b8<4;b8++)  //loop 8x8 blocks
		{      
			//get from stream
			int ipred4x4_mode_luma = read_functions->raw_ipred4x4_mode_luma ARGS0();

			bx = ((b8&1)<<1);
			by = (b8&2);

			// left block
			if (bx)
			{
				leftIntraPredMode  = currMB_r->ipredmode[by*4+bx-1];
			}
			else
			{
				if(p_left_block->pMB)
				{
					int mb_type=p_left_block->pMB->mb_type;
					if (constrained_intra_pred_flag)
					{						
						if (p_left_block->pMB->intra_block)
						{
							if(mb_type&(I4MB|I8MB))
								leftIntraPredMode = p_left_block->pMB->ipredmode[(p_left_block->y<<2)+3];
							else
								leftIntraPredMode = DC_PRED;
						}
						else
							leftIntraPredMode = -1;
					}
					else
					{						
						if(mb_type&(I4MB|I8MB))
							leftIntraPredMode = p_left_block->pMB->ipredmode[(p_left_block->y<<2)+3];
						else
							leftIntraPredMode = DC_PRED;						
					}
				}
				else
					leftIntraPredMode = -1;

				p_left_block+=2;
			}
			// top block
			if (by)
			{
				upIntraPredMode = currMB_r->ipredmode[(by-1)*4+bx];	
			}
			else
			{
				if(IMGPAR pUpMB_r)
				{
					int mb_type=IMGPAR pUpMB_r->mb_type;
					if (constrained_intra_pred_flag)
					{						
						if (IMGPAR pUpMB_r->intra_block)
						{
							if(mb_type&(I4MB|I8MB))
								upIntraPredMode = IMGPAR pUpMB_r->ipredmode[12+bx];
							else
								upIntraPredMode = DC_PRED;
						}
						else
							upIntraPredMode  = -1;	
					}
					else
					{						
						if(mb_type&(I4MB|I8MB))
							upIntraPredMode = IMGPAR pUpMB_r->ipredmode[12+bx];	
						else
							upIntraPredMode = DC_PRED;						
					}
				}
				else
					upIntraPredMode  = -1;
			}
			mostProbableIntraPredMode  = (upIntraPredMode < 0 || leftIntraPredMode < 0) ? DC_PRED : __fast_min(upIntraPredMode,leftIntraPredMode);

			dec = (ipred4x4_mode_luma == -1) ? mostProbableIntraPredMode : ipred4x4_mode_luma + (ipred4x4_mode_luma >= mostProbableIntraPredMode);

			currMB_r->ipredmode[(by+0)*4+bx+0]=
				currMB_r->ipredmode[(by+0)*4+bx+1]=
				currMB_r->ipredmode[(by+1)*4+bx+0]=
				currMB_r->ipredmode[(by+1)*4+bx+1]=dec;
		}
	}

	if (IntraChromaPredModeFlag
#ifdef __SUPPORT_YUV400__
		&& dec_picture->chroma_format_idc != YUV400
#endif
		)
	{
		currMB_r->c_ipred_mode = read_functions->raw_ipred4x4_mode_chroma ARGS0();

		if (currMB_r->c_ipred_mode < DC_PRED_8 || currMB_r->c_ipred_mode > PLANE_8)
		{
			DEBUG_SHOW_ERROR_INFO("[ERROR]illegal chroma intra pred mode!\n", 600);
			return CREL_WARNING_H264_STREAMERROR_LEVEL_3;
		}
	}

	SetIntraPredDecMode ARGS0();
	return CREL_OK;
}

const int tNeighbourA_block_y[7][4] = {
	{0, 1, 2, 3},
	{0, 0, 1, 1},
	{2, 2, 3, 3},
	{1, 3, 1, 3},
	{1, 3, 1, 3},
	{0, 1, 2, 3},
	{0, 1, 2, 3},
};

/*!
************************************************************************
* \brief
*    Set motion vector predictor
************************************************************************
*/
static void SetMotionVectorPredictor PARGS7(MotionVector  *pmv,
																						char           ref_frame,
																						byte           list,
																						int            block_x,
																						int            block_y,
																						int            blockshape_x,
																						int            blockshape_y)
{
	static int block_yx[4][4] = { {-1, -1, -1, -1}, {8, 4, 8, 4}, {16, 16, 8, 4}, {8, 4, 8, 4} };

	int mb_x                 = BLOCK_SIZE*block_x;
	int mb_y                 = BLOCK_SIZE*block_y;	
	int mb_nr				  = IMGPAR current_mb_nr_r;

	MotionVector mv_a, mv_b, mv_c;
	int mvPredType, rFrameL, rFrameU, rFrameUR;

	PixelPos block_c;
	PixelPos *p_block_a;
	Macroblock_s *mb_a, *mb_b, *mb_c;

	mb_a = mb_b = mb_c = 0;

	if(block_x)
	{
		rFrameL = currMB_s_r->pred_info.ref_idx[list][l_16_4[(block_y<<2)+block_x-1]];
		mv_a.x   = currMB_s_r->pred_info.mv[list][(block_y<<2)+block_x-1].x;
		mv_a.y   = currMB_s_r->pred_info.mv[list][(block_y<<2)+block_x-1].y;
	}
	else
	{
		p_block_a = &(IMGPAR left[block_y]);
		if(p_block_a->pMB)
		{
			mb_a = &dec_picture->mb_data[p_block_a->mb_addr];
			rFrameL = mb_a->pred_info.ref_idx[list][l_16_4[(p_block_a->y<<2)+p_block_a->x]];
			mv_a.x   = mb_a->pred_info.mv[list][(p_block_a->y<<2)+p_block_a->x].x;
			mv_a.y   = mb_a->pred_info.mv[list][(p_block_a->y<<2)+p_block_a->x].y;

			if (currMB_s_r->mb_field ^ mb_a->mb_field)
			{
				if(currMB_s_r->mb_field)
				{
					rFrameL <<= 1;
					mv_a.y /= 2;
				}
				else
				{
					rFrameL >>=  1;
					mv_a.y <<= 1;
				}
			}
		}
		else
		{
			mv_a.mv_comb = 0;
			rFrameL = -1;
		}
	}

	if(block_y)
	{
		rFrameU = currMB_s_r->pred_info.ref_idx[list][l_16_4[((block_y-1)<<2)+block_x]];
		mv_b.x   = currMB_s_r->pred_info.mv[list][((block_y-1)<<2)+block_x].x;
		mv_b.y   = currMB_s_r->pred_info.mv[list][((block_y-1)<<2)+block_x].y;
	}
	else
	{
		if(IMGPAR pUpMB_r)
		{
			mb_b = &dec_picture->mb_data[IMGPAR mbAddrB];

			rFrameU = mb_b->pred_info.ref_idx[list][l_16_4[(3<<2)+block_x]];
			mv_b.x   = mb_b->pred_info.mv[list][(3<<2)+block_x].x;
			mv_b.y   = mb_b->pred_info.mv[list][(3<<2)+block_x].y;

			if (currMB_s_r->mb_field ^ mb_b->mb_field)
			{
				if(currMB_s_r->mb_field)
				{
					rFrameU <<= 1;
					mv_b.y /= 2;
				}
				else
				{
					rFrameU >>=  1;
					mv_b.y <<= 1;
				}
			}
		}
		else
		{
			mv_b.mv_comb = 0;
			rFrameU = -1;
		}		
	}

	if ((mb_x+blockshape_x)<16)
	{
		if (mb_y)
		{
			block_c.pMB = currMB_r;
			block_c.mb_addr = mb_nr;
			block_c.x = (((block_x<<2)+blockshape_x)>>2);
			block_c.y = block_y-1;
		}
		else
		{
			block_c.pMB = IMGPAR pUpMB_r;
			block_c.mb_addr = IMGPAR mbAddrB;
			block_c.x = ((block_x<<2)+blockshape_x)>>2;
			block_c.y = 3;			
		}
	}
	else
	{
		if (mb_y)
			block_c.pMB = NULL;
		else
		{
			block_c.pMB = IMGPAR pUpRightMB_r;
			block_c.mb_addr = IMGPAR mbAddrC;
			block_c.x = 0;
			block_c.y = 3;
		}
	}

	if(block_c.pMB)
	{	
		mb_c = &dec_picture->mb_data[block_c.mb_addr];	 
		// first column of 8x8 blocks
		if( block_yx[block_y][block_x] == blockshape_x )
			block_c.pMB = NULL;
	}

	if (block_c.pMB == NULL)
	{
		mb_c = 0;

		if (mb_x)
		{
			if (mb_y)
			{
				block_c.pMB = currMB_r;
				block_c.mb_addr = mb_nr;
				block_c.x = block_x-1;
				block_c.y = block_y-1;
			}
			else
			{
				block_c.pMB = IMGPAR pUpMB_r;
				block_c.mb_addr = IMGPAR mbAddrB;
				block_c.x = block_x-1;
				block_c.y = 3;
			}
		}
		else
		{
			if (mb_y)
			{
				block_c.pMB = IMGPAR pLeftMB_r;
				if (block_c.pMB)
				{
					block_c.mb_addr = IMGPAR mbAddrA;
					if (currMB_r->mbStatusA)
					{
						if (block_y == 3 && currMB_r->mbStatusA < 5)
						{
							block_c.pMB++;
							block_c.mb_addr++;
						}
						else if (currMB_r->mbStatusA < 3)
						{
							block_c.pMB++;
							block_c.mb_addr++;
						}
					}
					block_c.x = 3;
					block_c.y = tNeighbourA_block_y[currMB_r->mbStatusA][block_y-1];
				}
			}
			else
			{
				block_c.pMB = IMGPAR pUpLeftMB_r;
				block_c.mb_addr = IMGPAR mbAddrD;
				block_c.x = 3;
				block_c.y = 3;
			}
		}

		if(block_c.pMB)
			mb_c = &dec_picture->mb_data[block_c.mb_addr];		 
	}

	if(mb_c)
	{
		rFrameUR = mb_c->pred_info.ref_idx[list][l_16_4[(block_c.y<<2)+block_c.x]];
		mv_c.x    = mb_c->pred_info.mv[list][(block_c.y<<2)+block_c.x].x;
		mv_c.y    = mb_c->pred_info.mv[list][(block_c.y<<2)+block_c.x].y;

		if (currMB_s_r->mb_field ^ mb_c->mb_field)
		{
			if(currMB_s_r->mb_field)
			{	
				rFrameUR <<= 1;
				mv_c.y /= 2;
			}
			else
			{
				rFrameUR >>=  1;
				mv_c.y <<= 1;
			}
		}
	}
	else
	{
		mv_c.mv_comb = 0;
		rFrameUR = -1;
	}


	/* Prediction if only one of the neighbors uses the reference frame
	* we are checking
	*/
	const static int PredType[2][2][2] = {
		{{MVPRED_MEDIAN, MVPRED_UR},{MVPRED_U, MVPRED_MEDIAN}}, 
		{{MVPRED_L, MVPRED_MEDIAN},{MVPRED_MEDIAN, MVPRED_MEDIAN}}
	};
	mvPredType = PredType[(rFrameL==ref_frame)][(rFrameU==ref_frame)][(rFrameUR==ref_frame)];

	//if(rFrameL == ref_frame && rFrameU != ref_frame && rFrameUR != ref_frame)       
	//	mvPredType = MVPRED_L;
	//else if(rFrameL != ref_frame && rFrameU == ref_frame && rFrameUR != ref_frame)  
	//	mvPredType = MVPRED_U;
	//else if(rFrameL != ref_frame && rFrameU != ref_frame && rFrameUR == ref_frame)  
	//	mvPredType = MVPRED_UR;
	//else
	//	mvPredType = MVPRED_MEDIAN;

	// Directional predictions 
	//Grant - this case will never happen 
	/*
	if(blockshape_x == 8 && blockshape_y == 16)
	{
	if(mb_x == 0)
	{
	if(rFrameL == ref_frame)
	mvPredType = MVPRED_L;
	}
	else
	{
	if( rFrameUR == ref_frame)
	mvPredType = MVPRED_UR;
	}
	}
	else if(blockshape_x == 16 && blockshape_y == 8)
	{
	if(mb_y == 0)
	{
	if(rFrameU == ref_frame)
	mvPredType = MVPRED_U;
	}
	else
	{
	if(rFrameL == ref_frame)
	mvPredType = MVPRED_L;
	}
	}
	*/


	switch (mvPredType)
	{
	case MVPRED_MEDIAN:
		if(!(mb_b || mb_c))
		{
			pmv->mv_comb = mv_a.mv_comb;
		}
		else
		{
#if 1

//#if defined (H264_ENABLE_INTRINSICS)
			if((cpu_type == CPU_LEVEL_MMX) || (cpu_type == CPU_LEVEL_SSE) || (cpu_type == CPU_LEVEL_SSE2) || (cpu_type == CPU_LEVEL_SSE3))
			{
				__m64 mm0, mm1, mm2, mm3, mm4, mm5;

				mm0 = _m_from_int(mv_a.mv_comb); 
				mm1 = _m_from_int(mv_b.mv_comb); 
				mm2 = _m_from_int(mv_c.mv_comb); 

				mm3 = _mm_max_pi16 (mm0, mm1); //max(a,b)
				mm5 = _mm_min_pi16 (mm0, mm1); //min(a,b)		
				mm4 = _mm_min_pi16 (mm3, mm2); //min( max(a,b), c)				
				mm0 = _mm_max_pi16 (mm4, mm5); //max( min( max(a,b), c), min(a,b))	

				pmv->mv_comb = _m_to_int(mm0);
			}
			else
			{
				pmv->x = (mv_a.x>mv_b.x) ? ((mv_c.x>mv_a.x) ? (mv_a.x) : ((mv_b.x>mv_c.x) ? (mv_b.x) : (mv_c.x))) :
				((mv_c.x>mv_b.x) ? (mv_b.x) : ((mv_a.x>mv_c.x) ? (mv_a.x) : (mv_c.x)));
				pmv->y = (mv_a.y>mv_b.y) ? ((mv_c.y>mv_a.y) ? (mv_a.y) : ((mv_b.y>mv_c.y) ? (mv_b.y) : (mv_c.y))) :
				((mv_c.y>mv_b.y) ? (mv_b.y) : ((mv_a.y>mv_c.y) ? (mv_a.y) : (mv_c.y)));
			}

//#endif

#else
			int tmp_min,tmp_max;

			tmp_min = __fast_min(mvx_a,mvx_b);
			tmp_max = mvx_a+mvx_b-tmp_min;
			tmp_min = __fast_min(mvx_c,tmp_min);
			tmp_max = __fast_max(mvx_c,tmp_max);
			pmv->x = mvx_a+mvx_b+mvx_c-tmp_min-tmp_max;

			tmp_min = __fast_min(mvy_a,mvy_b);
			tmp_max = mvy_a+mvy_b-tmp_min;
			tmp_min = __fast_min(mvy_c,tmp_min);
			tmp_max = __fast_max(mvy_c,tmp_max);
			pmv->y = mvy_a+mvy_b+mvy_c-tmp_min-tmp_max;
#endif
		}
		break;
	case MVPRED_L:
		pmv->mv_comb = mv_a.mv_comb;
		break;
	case MVPRED_U:
		pmv->mv_comb = mv_b.mv_comb;
		break;
	case MVPRED_UR:
		pmv->mv_comb = mv_c.mv_comb;
		break;
	default:
		pmv->mv_comb = 0;
		break;
	}
}

static void SetMotionVectorPredictor_block00_shape16x16 PARGS6(MotionVector  *pmv,
																															 char           ref_frame,
																															 byte           list,
																															 int		    current_mb_nr,
																															 Macroblock	   *currMB,
																															 Macroblock_s  *currMB_s)
{
	int mb_nr                = current_mb_nr;

	MotionVector mv_a, mv_b, mv_c;
	int mvPredType, rFrameL, rFrameU, rFrameUR;

	PixelPos block_c;
	PixelPos *p_block_a;
	Macroblock_s *mb_a, *mb_b, *mb_c;

	mb_a = mb_b = mb_c = 0;

	//(block_x==0)
	p_block_a = &(IMGPAR left[0]);
	if(p_block_a->pMB)
		mb_a = &dec_picture->mb_data[p_block_a->mb_addr];

	//(block_y==0)
	if(IMGPAR pUpMB_r)
		mb_b = &dec_picture->mb_data[IMGPAR mbAddrB];

	block_c.pMB = IMGPAR pUpRightMB_r;
	block_c.mb_addr = IMGPAR mbAddrC;
	block_c.x = 0;
	block_c.y = 3;

	if(block_c.pMB)
	{	
		mb_c = &dec_picture->mb_data[block_c.mb_addr];	 
	}
	else	
	{
		block_c.pMB = IMGPAR pUpLeftMB_r;
		block_c.mb_addr = IMGPAR mbAddrD;
		block_c.x = 3;
		if (IMGPAR MbaffFrameFlag && currMB_r->mbStatusD == 5)
			block_c.y = 1;
		else
			block_c.y = 3;

		if(block_c.pMB)
			mb_c = &dec_picture->mb_data[block_c.mb_addr];		 
	}	

	if(mb_a)
	{
		rFrameL = mb_a->pred_info.ref_idx[list][l_16_4[(p_block_a->y<<2)+p_block_a->x]];
		mv_a.x   = mb_a->pred_info.mv[list][(p_block_a->y<<2)+p_block_a->x].x;
		mv_a.y   = mb_a->pred_info.mv[list][(p_block_a->y<<2)+p_block_a->x].y;

		if (currMB_s->mb_field ^ mb_a->mb_field)
		{
			if(currMB_s->mb_field)
			{
				rFrameL <<= 1;
				mv_a.y /= 2;
			}
			else
			{
				rFrameL >>=  1;
				mv_a.y <<= 1;
			}
		}
	}
	else
	{
		mv_a.mv_comb = 0;
		rFrameL = -1;
	}

	if(mb_b)
	{
		rFrameU = mb_b->pred_info.ref_idx[list][l_16_4[(3<<2)+0]];
		mv_b.x   = mb_b->pred_info.mv[list][(3<<2)+0].x;
		mv_b.y   = mb_b->pred_info.mv[list][(3<<2)+0].y;

		if (currMB_s->mb_field ^ mb_b->mb_field)
		{
			if(currMB_s->mb_field)
			{
				rFrameU <<= 1;
				mv_b.y /= 2;
			}
			else
			{
				rFrameU >>=  1;
				mv_b.y <<= 1;
			}
		}
	}
	else
	{
		mv_b.mv_comb = 0;
		rFrameU = -1;
	}

	if(mb_c)
	{
		rFrameUR = mb_c->pred_info.ref_idx[list][l_16_4[(block_c.y<<2)+block_c.x]];
		mv_c.x    = mb_c->pred_info.mv[list][(block_c.y<<2)+block_c.x].x;
		mv_c.y    = mb_c->pred_info.mv[list][(block_c.y<<2)+block_c.x].y;

		if (currMB_s->mb_field ^ mb_c->mb_field)
		{
			if(currMB_s->mb_field)
			{	
				rFrameUR <<= 1;
				mv_c.y /= 2;
			}
			else
			{
				rFrameUR >>=  1;
				mv_c.y <<= 1;
			}
		}
	}
	else
	{
		mv_c.mv_comb = 0;
		rFrameUR = -1;
	}

	/* Prediction if only one of the neighbors uses the reference frame
	* we are checking
	*/
	const static int PredType[2][2][2] = {
		{{MVPRED_MEDIAN, MVPRED_UR},{MVPRED_U, MVPRED_MEDIAN}}, 
		{{MVPRED_L, MVPRED_MEDIAN},{MVPRED_MEDIAN, MVPRED_MEDIAN}}
	};

	mvPredType = PredType[(rFrameL==ref_frame)][(rFrameU==ref_frame)][(rFrameUR==ref_frame)];

	switch (mvPredType)
	{
	case MVPRED_MEDIAN:
		if(!(mb_b || mb_c))
		{
			pmv->mv_comb = mv_a.mv_comb;
		}
		else
		{
//#if defined (H264_ENABLE_INTRINSICS)
			if((cpu_type == CPU_LEVEL_MMX) || (cpu_type == CPU_LEVEL_SSE) || (cpu_type == CPU_LEVEL_SSE2) || (cpu_type == CPU_LEVEL_SSE3))
			{
				__m64 mm0, mm1, mm2, mm3, mm4, mm5;

				mm0 = _m_from_int(mv_a.mv_comb); 
				mm1 = _m_from_int(mv_b.mv_comb); 
				mm2 = _m_from_int(mv_c.mv_comb); 

				mm3 = _mm_max_pi16 (mm0, mm1); //max(a,b)
				mm5 = _mm_min_pi16 (mm0, mm1); //min(a,b)		
				mm4 = _mm_min_pi16 (mm3, mm2); //min( max(a,b), c)				
				mm0 = _mm_max_pi16 (mm4, mm5); //max( min( max(a,b), c), min(a,b))	

				pmv->mv_comb = _m_to_int(mm0);
			}
			else
			{
				pmv->x = (mv_a.x>mv_b.x) ? ((mv_c.x>mv_a.x) ? (mv_a.x) : ((mv_b.x>mv_c.x) ? (mv_b.x) : (mv_c.x))) :
				((mv_c.x>mv_b.x) ? (mv_b.x) : ((mv_a.x>mv_c.x) ? (mv_a.x) : (mv_c.x)));
				pmv->y = (mv_a.y>mv_b.y) ? ((mv_c.y>mv_a.y) ? (mv_a.y) : ((mv_b.y>mv_c.y) ? (mv_b.y) : (mv_c.y))) :
				((mv_c.y>mv_b.y) ? (mv_b.y) : ((mv_a.y>mv_c.y) ? (mv_a.y) : (mv_c.y)));
			}

//#endif
		}
		break;
	case MVPRED_L:
		pmv->mv_comb = mv_a.mv_comb;
		break;
	case MVPRED_U:
		pmv->mv_comb = mv_b.mv_comb;
		break;
	case MVPRED_UR:
		pmv->mv_comb = mv_c.mv_comb;
		break;
	default:
		pmv->mv_comb = 0;
		break;
	}
}
static void SetMotionVectorPredictor_block00_shape16x8 PARGS6(MotionVector  *pmv,
																															char           ref_frame,
																															byte           list,
																															int		    current_mb_nr,
																															Macroblock	   *currMB,
																															Macroblock_s  *currMB_s)
{
	int mb_nr                = current_mb_nr;

	MotionVector mv_a, mv_b, mv_c;
	int mvPredType, rFrameL, rFrameU, rFrameUR;

	PixelPos block_c;
	PixelPos *p_block_a;
	Macroblock_s *mb_a, *mb_b, *mb_c;

	mb_a = mb_b = mb_c = 0;

	//(block_x==0)
	p_block_a = &(IMGPAR left[0]);
	if(p_block_a->pMB)
		mb_a = &dec_picture->mb_data[p_block_a->mb_addr];

	//(block_y==0)
	if(IMGPAR pUpMB_r)
		mb_b = &dec_picture->mb_data[IMGPAR mbAddrB];

	block_c.pMB = IMGPAR pUpRightMB_r;
	block_c.mb_addr = IMGPAR mbAddrC;
	block_c.x = 0;
	block_c.y = 3;

	if(block_c.pMB)
	{	
		mb_c = &dec_picture->mb_data[block_c.mb_addr];	 
	}
	else
	{		
		block_c.pMB = IMGPAR pUpLeftMB_r;
		block_c.mb_addr = IMGPAR mbAddrD;
		block_c.x = 3;
		if (IMGPAR MbaffFrameFlag && currMB_r->mbStatusD == 5)
			block_c.y = 1;
		else
			block_c.y = 3;

		if(block_c.pMB)
			mb_c = &dec_picture->mb_data[block_c.mb_addr];		 
	}	

	if(mb_a)
	{
		rFrameL = mb_a->pred_info.ref_idx[list][l_16_4[(p_block_a->y<<2)+p_block_a->x]];
		mv_a.x   = mb_a->pred_info.mv[list][(p_block_a->y<<2)+p_block_a->x].x;
		mv_a.y   = mb_a->pred_info.mv[list][(p_block_a->y<<2)+p_block_a->x].y;

		if (currMB_s->mb_field ^ mb_a->mb_field)
		{
			if(currMB_s->mb_field)
			{
				rFrameL <<= 1;
				mv_a.y /= 2;
			}
			else
			{
				rFrameL >>=  1;
				mv_a.y <<= 1;
			}
		}
	}
	else
	{
		mv_a.mv_comb = 0;
		rFrameL = -1;
	}

	if(mb_b)
	{
		rFrameU = mb_b->pred_info.ref_idx[list][l_16_4[(3<<2)+0]];
		mv_b.x   = mb_b->pred_info.mv[list][(3<<2)+0].x;
		mv_b.y   = mb_b->pred_info.mv[list][(3<<2)+0].y;

		if (currMB_s->mb_field ^ mb_b->mb_field)
		{
			if(currMB_s->mb_field)
			{
				rFrameU <<= 1;
				mv_b.y /= 2;
			}
			else
			{
				rFrameU >>=  1;
				mv_b.y <<= 1;
			}
		}
	}
	else
	{
		mv_b.mv_comb = 0;
		rFrameU = -1;
	}

	if(mb_c)
	{
		rFrameUR = mb_c->pred_info.ref_idx[list][l_16_4[(block_c.y<<2)+block_c.x]];
		mv_c.x    = mb_c->pred_info.mv[list][(block_c.y<<2)+block_c.x].x;
		mv_c.y    = mb_c->pred_info.mv[list][(block_c.y<<2)+block_c.x].y;

		if (currMB_s->mb_field ^ mb_c->mb_field)
		{
			if(currMB_s->mb_field)
			{	
				rFrameUR <<= 1;
				mv_c.y /= 2;
			}
			else
			{
				rFrameUR >>=  1;
				mv_c.y <<= 1;
			}
		}
	}
	else
	{
		mv_c.mv_comb = 0;
		rFrameUR = -1;
	}


	/* Prediction if only one of the neighbors uses the reference frame
	* we are checking
	*/
	// plus Directional predictions 
	const static int PredType[2][2][2] = {
		{{MVPRED_MEDIAN, MVPRED_UR},{MVPRED_U, MVPRED_U}}, 
		{{MVPRED_L, MVPRED_MEDIAN},{MVPRED_U, MVPRED_U}}
	};

	mvPredType = PredType[(rFrameL==ref_frame)][(rFrameU==ref_frame)][(rFrameUR==ref_frame)];
	//if(rFrameU == ref_frame)
	//	mvPredType = MVPRED_U;

	switch (mvPredType)
	{
	case MVPRED_MEDIAN:
		if(!(mb_b || mb_c))
		{
			pmv->mv_comb = mv_a.mv_comb;
		}
		else
		{
//#if defined (H264_ENABLE_INTRINSICS)
			if((cpu_type == CPU_LEVEL_MMX) || (cpu_type == CPU_LEVEL_SSE) || (cpu_type == CPU_LEVEL_SSE2) || (cpu_type == CPU_LEVEL_SSE3))
			{
				__m64 mm0, mm1, mm2, mm3, mm4, mm5;

				mm0 = _m_from_int(mv_a.mv_comb); 
				mm1 = _m_from_int(mv_b.mv_comb); 
				mm2 = _m_from_int(mv_c.mv_comb); 

				mm3 = _mm_max_pi16 (mm0, mm1); //max(a,b)
				mm5 = _mm_min_pi16 (mm0, mm1); //min(a,b)		
				mm4 = _mm_min_pi16 (mm3, mm2); //min( max(a,b), c)				
				mm0 = _mm_max_pi16 (mm4, mm5); //max( min( max(a,b), c), min(a,b))	

				pmv->mv_comb = _m_to_int(mm0);
			}
			else
			{
				pmv->x = (mv_a.x>mv_b.x) ? ((mv_c.x>mv_a.x) ? (mv_a.x) : ((mv_b.x>mv_c.x) ? (mv_b.x) : (mv_c.x))) :
				((mv_c.x>mv_b.x) ? (mv_b.x) : ((mv_a.x>mv_c.x) ? (mv_a.x) : (mv_c.x)));
				pmv->y = (mv_a.y>mv_b.y) ? ((mv_c.y>mv_a.y) ? (mv_a.y) : ((mv_b.y>mv_c.y) ? (mv_b.y) : (mv_c.y))) :
				((mv_c.y>mv_b.y) ? (mv_b.y) : ((mv_a.y>mv_c.y) ? (mv_a.y) : (mv_c.y)));
			}

//#endif
		}
		break;
	case MVPRED_L:
		pmv->mv_comb = mv_a.mv_comb;		
		break;
	case MVPRED_U:
		pmv->mv_comb = mv_b.mv_comb;	
		break;
	case MVPRED_UR:
		pmv->mv_comb = mv_c.mv_comb;	
		break;
	default:
		pmv->mv_comb = 0;
		break;
	}
}
static void SetMotionVectorPredictor_block02_shape16x8 PARGS6(MotionVector  *pmv,
																															char           ref_frame,
																															byte           list,
																															int		    current_mb_nr,
																															Macroblock	   *currMB,
																															Macroblock_s  *currMB_s)
{
	//int mb_y                 = 8;
	int mb_nr                = current_mb_nr;

	MotionVector mv_a, mv_b, mv_c;
	int mvPredType, rFrameL, rFrameU, rFrameUR;

	PixelPos block_c;
	PixelPos *p_block_a;
	Macroblock_s *mb_a, *mb_b, *mb_c;

	mb_a = mb_b = mb_c = 0;

	//(block_x==0)
	p_block_a = &(IMGPAR left[2]);
	if(p_block_a->pMB)
		mb_a = &dec_picture->mb_data[p_block_a->mb_addr];

	//(block_y==2)
	//p_block_b = &block_b;		
	//block_b.x = 0;
	//block_b.y = 1;
	mb_b = currMB_s;	
	rFrameU = mb_b->pred_info.ref_idx[list][0];
	mv_b.x   = mb_b->pred_info.mv[list][4].x;
	mv_b.y   = mb_b->pred_info.mv[list][4].y;
	if (currMB_s->mb_field ^ mb_b->mb_field)
	{
		if(currMB_s->mb_field)
		{
			rFrameU <<= 1;
			mv_b.y /= 2;
		}
		else
		{
			rFrameU >>=  1;
			mv_b.y <<= 1;
		}
	}

	//(-1, 7)
	block_c.pMB = IMGPAR pLeftMB_r;
	if (block_c.pMB)
	{
		block_c.mb_addr = IMGPAR mbAddrA;
		if (currMB_r->mbStatusA && currMB_r->mbStatusA < 3)
		{
			block_c.pMB++;
			block_c.mb_addr++;
		}
		block_c.x = 3;
		block_c.y = tNeighbourA_block_y[currMB_r->mbStatusA][1];
	}

	if(block_c.pMB)
		mb_c = &dec_picture->mb_data[block_c.mb_addr];		 

	if(mb_a)
	{
		rFrameL = mb_a->pred_info.ref_idx[list][l_16_4[(p_block_a->y<<2)+p_block_a->x]];
		mv_a.x   = mb_a->pred_info.mv[list][(p_block_a->y<<2)+p_block_a->x].x;
		mv_a.y   = mb_a->pred_info.mv[list][(p_block_a->y<<2)+p_block_a->x].y;

		if (currMB_s->mb_field ^ mb_a->mb_field)
		{
			if(currMB_s->mb_field)
			{
				rFrameL <<= 1;
				mv_a.y /= 2;
			}
			else
			{
				rFrameL >>=  1;
				mv_a.y <<= 1;
			}
		}
	}
	else
	{
		mv_a.mv_comb = 0;
		rFrameL = -1;
	}

	if(mb_c)
	{
		rFrameUR = mb_c->pred_info.ref_idx[list][l_16_4[(block_c.y<<2)+block_c.x]];
		mv_c.x    = mb_c->pred_info.mv[list][(block_c.y<<2)+block_c.x].x;
		mv_c.y    = mb_c->pred_info.mv[list][(block_c.y<<2)+block_c.x].y;

		if (currMB_s->mb_field ^ mb_c->mb_field)
		{
			if(currMB_s->mb_field)
			{	
				rFrameUR <<= 1;
				mv_c.y /= 2;
			}
			else
			{
				rFrameUR >>=  1;
				mv_c.y <<= 1;
			}
		}
	}
	else
	{
		mv_c.mv_comb = 0;
		rFrameUR = -1;
	}

	/* Prediction if only one of the neighbors uses the reference frame
	* we are checking
	*/
	//plus Directional predictions
	const static int PredType[2][2][2] = {
		{{MVPRED_MEDIAN, MVPRED_UR},{MVPRED_U, MVPRED_MEDIAN}}, 
		{{MVPRED_L, MVPRED_L},{MVPRED_L, MVPRED_L}}
	};
	mvPredType = PredType[(rFrameL==ref_frame)][(rFrameU==ref_frame)][(rFrameUR==ref_frame)];
	//if(rFrameL == ref_frame)
	//	mvPredType = MVPRED_L;

	switch (mvPredType)
	{
	case MVPRED_MEDIAN:
//#if defined (H264_ENABLE_INTRINSICS)
		if((cpu_type == CPU_LEVEL_MMX) || (cpu_type == CPU_LEVEL_SSE) || (cpu_type == CPU_LEVEL_SSE2) || (cpu_type == CPU_LEVEL_SSE3))
		{
			__m64 mm0, mm1, mm2, mm3, mm4, mm5;

			mm0 = _m_from_int(mv_a.mv_comb); 
			mm1 = _m_from_int(mv_b.mv_comb); 
			mm2 = _m_from_int(mv_c.mv_comb); 

			mm3 = _mm_max_pi16 (mm0, mm1); //max(a,b)
			mm5 = _mm_min_pi16 (mm0, mm1); //min(a,b)		
			mm4 = _mm_min_pi16 (mm3, mm2); //min( max(a,b), c)				
			mm0 = _mm_max_pi16 (mm4, mm5); //max( min( max(a,b), c), min(a,b))	

			pmv->mv_comb = _m_to_int(mm0);
		}
		else
		{
			pmv->x = (mv_a.x>mv_b.x) ? ((mv_c.x>mv_a.x) ? (mv_a.x) : ((mv_b.x>mv_c.x) ? (mv_b.x) : (mv_c.x))) :
			((mv_c.x>mv_b.x) ? (mv_b.x) : ((mv_a.x>mv_c.x) ? (mv_a.x) : (mv_c.x)));
			pmv->y = (mv_a.y>mv_b.y) ? ((mv_c.y>mv_a.y) ? (mv_a.y) : ((mv_b.y>mv_c.y) ? (mv_b.y) : (mv_c.y))) :
			((mv_c.y>mv_b.y) ? (mv_b.y) : ((mv_a.y>mv_c.y) ? (mv_a.y) : (mv_c.y)));
		}
//#endif
		break;
	case MVPRED_L:
		pmv->mv_comb = mv_a.mv_comb;
		break;
	case MVPRED_U:
		pmv->mv_comb = mv_b.mv_comb;
		break;
	case MVPRED_UR:
		pmv->mv_comb = mv_c.mv_comb;
		break;
	default:
		pmv->mv_comb = 0;
		break;
	}
}
static void SetMotionVectorPredictor_block00_shape8x16 PARGS6(MotionVector  *pmv,
																															char           ref_frame,
																															byte           list,
																															int		    current_mb_nr,
																															Macroblock	   *currMB,
																															Macroblock_s  *currMB_s)
{
	int mb_nr                = current_mb_nr;

	MotionVector mv_a, mv_b, mv_c;
	int mvPredType, rFrameL, rFrameU, rFrameUR;

	PixelPos block_c;
	PixelPos *p_block_a;
	Macroblock_s *mb_a, *mb_b, *mb_c;

	mb_a = mb_b = mb_c = 0;

	//(block_x==0)
	p_block_a = &(IMGPAR left[0]);
	if(p_block_a->pMB)
		mb_a = &dec_picture->mb_data[p_block_a->mb_addr];

	//(block_y==0)
	if(IMGPAR pUpMB_r)
		mb_b = &dec_picture->mb_data[IMGPAR mbAddrB];



	if(IMGPAR pUpMB_r)
	{	
		mb_c = &dec_picture->mb_data[IMGPAR mbAddrB];	 
		block_c.pMB = IMGPAR pUpMB_r;
		block_c.mb_addr = IMGPAR mbAddrB;
		block_c.x = 2;
		block_c.y = 3;
	}
	else
	{		
		block_c.pMB = IMGPAR pUpLeftMB_r;
		block_c.mb_addr = IMGPAR mbAddrD;
		block_c.x = 3;
		if (IMGPAR MbaffFrameFlag && currMB_r->mbStatusD == 5)
			block_c.y = 1;
		else
			block_c.y = 3;

		if(block_c.pMB)
			mb_c = &dec_picture->mb_data[block_c.mb_addr];		 
	}	

	if(mb_a)
	{
		rFrameL = mb_a->pred_info.ref_idx[list][l_16_4[(p_block_a->y<<2)+p_block_a->x]];
		mv_a.x   = mb_a->pred_info.mv[list][(p_block_a->y<<2)+p_block_a->x].x;
		mv_a.y   = mb_a->pred_info.mv[list][(p_block_a->y<<2)+p_block_a->x].y;

		if (currMB_s->mb_field ^ mb_a->mb_field)
		{
			if(currMB_s->mb_field)
			{
				rFrameL <<= 1;
				mv_a.y /= 2;
			}
			else
			{
				rFrameL >>=  1;
				mv_a.y <<= 1;
			}
		}
	}
	else
	{
		mv_a.mv_comb = 0;
		rFrameL = -1;
	}

	if(mb_b)
	{
		rFrameU = mb_b->pred_info.ref_idx[list][l_16_4[(3<<2)+0]];
		mv_b.x   = mb_b->pred_info.mv[list][(3<<2)+0].x;
		mv_b.y   = mb_b->pred_info.mv[list][(3<<2)+0].y;

		if (currMB_s->mb_field ^ mb_b->mb_field)
		{
			if(currMB_s->mb_field)
			{
				rFrameU <<= 1;
				mv_b.y /= 2;
			}
			else
			{
				rFrameU >>=  1;
				mv_b.y <<= 1;
			}
		}
	}
	else
	{
		mv_b.mv_comb = 0;
		rFrameU = -1;
	}

	if(mb_c)
	{
		rFrameUR = mb_c->pred_info.ref_idx[list][l_16_4[(block_c.y<<2)+block_c.x]];
		mv_c.x   = mb_c->pred_info.mv[list][(block_c.y<<2)+block_c.x].x;
		mv_c.y   = mb_c->pred_info.mv[list][(block_c.y<<2)+block_c.x].y;

		if (currMB_s->mb_field ^ mb_c->mb_field)
		{
			if(currMB_s->mb_field)
			{	
				rFrameUR <<= 1;
				mv_c.y /= 2;
			}
			else
			{
				rFrameUR >>=  1;
				mv_c.y <<= 1;
			}
		}
	}
	else
	{
		mv_c.mv_comb = 0;
		rFrameUR = -1;
	}


	/* Prediction if only one of the neighbors uses the reference frame
	* we are checking
	*/
	//plus Directional predictions 
	const static int PredType[2][2][2] = {
		{{MVPRED_MEDIAN, MVPRED_UR},{MVPRED_U, MVPRED_MEDIAN}}, 
		{{MVPRED_L, MVPRED_L},{MVPRED_L, MVPRED_L}}
	};
	mvPredType = PredType[(rFrameL==ref_frame)][(rFrameU==ref_frame)][(rFrameUR==ref_frame)];
	//if(rFrameL == ref_frame)
	//	mvPredType = MVPRED_L;

	switch (mvPredType)
	{
	case MVPRED_MEDIAN:
		if(!(mb_b || mb_c))
		{
			pmv->mv_comb = mv_a.mv_comb;
		}
		else
		{
//#if defined (H264_ENABLE_INTRINSICS)
			if((cpu_type == CPU_LEVEL_MMX) || (cpu_type == CPU_LEVEL_SSE) || (cpu_type == CPU_LEVEL_SSE2) || (cpu_type == CPU_LEVEL_SSE3))
			{
				__m64 mm0, mm1, mm2, mm3, mm4, mm5;

				mm0 = _m_from_int(mv_a.mv_comb); 
				mm1 = _m_from_int(mv_b.mv_comb); 
				mm2 = _m_from_int(mv_c.mv_comb); 

				mm3 = _mm_max_pi16 (mm0, mm1); //max(a,b)
				mm5 = _mm_min_pi16 (mm0, mm1); //min(a,b)		
				mm4 = _mm_min_pi16 (mm3, mm2); //min( max(a,b), c)				
				mm0 = _mm_max_pi16 (mm4, mm5); //max( min( max(a,b), c), min(a,b))	

				pmv->mv_comb = _m_to_int(mm0);
			}
			else
			{
				pmv->x = (mv_a.x>mv_b.x) ? ((mv_c.x>mv_a.x) ? (mv_a.x) : ((mv_b.x>mv_c.x) ? (mv_b.x) : (mv_c.x))) :
				((mv_c.x>mv_b.x) ? (mv_b.x) : ((mv_a.x>mv_c.x) ? (mv_a.x) : (mv_c.x)));
				pmv->y = (mv_a.y>mv_b.y) ? ((mv_c.y>mv_a.y) ? (mv_a.y) : ((mv_b.y>mv_c.y) ? (mv_b.y) : (mv_c.y))) :
				((mv_c.y>mv_b.y) ? (mv_b.y) : ((mv_a.y>mv_c.y) ? (mv_a.y) : (mv_c.y)));
			}
//#endif
		}
		break;
	case MVPRED_L:
		pmv->mv_comb = mv_a.mv_comb;
		break;
	case MVPRED_U:
		pmv->mv_comb = mv_b.mv_comb;
		break;
	case MVPRED_UR:
		pmv->mv_comb = mv_c.mv_comb;
		break;
	default:
		pmv->mv_comb = 0;
		break;
	}
}
static void SetMotionVectorPredictor_block20_shape8x16 PARGS6(MotionVector  *pmv,
																															char           ref_frame,
																															byte           list,
																															int		    current_mb_nr,
																															Macroblock	   *currMB,
																															Macroblock_s  *currMB_s)
{
	//int mb_x                 = 8;	
	int mb_nr                = current_mb_nr;

	MotionVector mv_a, mv_b, mv_c;
	int mvPredType, rFrameL, rFrameU, rFrameUR;

	PixelPos block_c;
	Macroblock_s *mb_a, *mb_b, *mb_c;

	mb_b = 0;

	//(block_x==2)
	//p_block_a = &block_a;		
	//block_a.x = 1;
	//block_a.y = 0;
	mb_a = currMB_s;

	rFrameL = mb_a->pred_info.ref_idx[list][0];
	mv_a.x   = mb_a->pred_info.mv[list][1].x;
	mv_a.y   = mb_a->pred_info.mv[list][1].y;
	if (currMB_s->mb_field ^ mb_a->mb_field)
	{
		if(currMB_s->mb_field)
		{
			rFrameL <<= 1;
			mv_a.y /= 2;
		}
		else
		{
			rFrameL >>=  1;
			mv_a.y <<= 1;
		}
	}

	//(block_y==0)
	if(IMGPAR pUpMB_r)
		mb_b = &dec_picture->mb_data[IMGPAR mbAddrB];

	block_c.pMB = IMGPAR pUpRightMB_r;
	block_c.mb_addr = IMGPAR mbAddrC;
	block_c.x = 0;
	block_c.y = 3;

	if(block_c.pMB)
		mb_c = &dec_picture->mb_data[block_c.mb_addr];	 
	else
	{
		mb_c = 0;
		block_c.pMB = IMGPAR pUpMB_r;
		block_c.mb_addr = IMGPAR mbAddrB;
		block_c.x = 1;
		block_c.y = 3;

		if(block_c.pMB)
			mb_c = &dec_picture->mb_data[block_c.mb_addr];		 
	}	

	if(mb_b)
	{
		rFrameU = mb_b->pred_info.ref_idx[list][l_16_4[(3<<2)+2]];
		mv_b.x   = mb_b->pred_info.mv[list][(3<<2)+2].x;
		mv_b.y   = mb_b->pred_info.mv[list][(3<<2)+2].y;

		if (currMB_s->mb_field ^ mb_b->mb_field)
		{
			if(currMB_s->mb_field)
			{
				rFrameU <<= 1;
				mv_b.y /= 2;
			}
			else
			{
				rFrameU >>=  1;
				mv_b.y <<= 1;
			}
		}
	}
	else
	{
		mv_b.mv_comb = 0;
		rFrameU = -1;
	}

	if(mb_c)
	{
		rFrameUR = mb_c->pred_info.ref_idx[list][l_16_4[(block_c.y<<2)+block_c.x]];
		mv_c.x    = mb_c->pred_info.mv[list][(block_c.y<<2)+block_c.x].x;
		mv_c.y    = mb_c->pred_info.mv[list][(block_c.y<<2)+block_c.x].y;

		if (currMB_s->mb_field ^ mb_c->mb_field)
		{
			if(currMB_s->mb_field)
			{	
				rFrameUR <<= 1;
				mv_c.y /= 2;
			}
			else
			{
				rFrameUR >>=  1;
				mv_c.y <<= 1;
			}
		}
	}
	else
	{
		mv_c.mv_comb = 0;
		rFrameUR = -1;
	}

	/* Prediction if only one of the neighbors uses the reference frame
	* we are checking
	*/
	//plus Directional predictions 
	const static int PredType[2][2][2] = {
		{{MVPRED_MEDIAN, MVPRED_UR},{MVPRED_U, MVPRED_UR}}, 
		{{MVPRED_L, MVPRED_UR},{MVPRED_MEDIAN, MVPRED_UR}}
	};
	mvPredType = PredType[(rFrameL==ref_frame)][(rFrameU==ref_frame)][(rFrameUR==ref_frame)];
	//if( rFrameUR == ref_frame)
	//	mvPredType = MVPRED_UR;

	switch (mvPredType)
	{
	case MVPRED_MEDIAN:
		if(!(mb_b || mb_c))
		{
			pmv->mv_comb = mv_a.mv_comb;
		}
		else
		{
#if 0
			pmv->x = (mv_a.x>mv_b.x) ? ((mv_c.x>mv_a.x) ? (mv_a.x) : ((mv_b.x>mv_c.x) ? (mv_b.x) : (mv_c.x))) :
				((mv_c.x>mv_b.x) ? (mv_b.x) : ((mv_a.x>mv_c.x) ? (mv_a.x) : (mv_c.x)));
		pmv->y = (mv_a.y>mv_b.y) ? ((mv_c.y>mv_a.y) ? (mv_a.y) : ((mv_b.y>mv_c.y) ? (mv_b.y) : (mv_c.y))) :
			((mv_c.y>mv_b.y) ? (mv_b.y) : ((mv_a.y>mv_c.y) ? (mv_a.y) : (mv_c.y)));

#else			
			__m64 xmm0, xmm1, xmm2, xmm3, xmm4, xmm5;

			xmm0 = _m_from_int(mv_a.mv_comb); 
			xmm1 = _m_from_int(mv_b.mv_comb); 
			xmm2 = _m_from_int(mv_c.mv_comb); 

			xmm3 = _mm_max_pi16 (xmm0, xmm1); //max(a,b)
			xmm5 = _mm_min_pi16 (xmm0, xmm1); //min(a,b)		
			xmm4 = _mm_min_pi16 (xmm3, xmm2); //min( max(a,b), c)				
			xmm0 = _mm_max_pi16 (xmm4, xmm5); //max( min( max(a,b), c), min(a,b))	

			pmv->mv_comb = _m_to_int(xmm0);
#endif
		}
		break;
	case MVPRED_L:
		pmv->mv_comb = mv_a.mv_comb;
		break;
	case MVPRED_U:
		pmv->mv_comb = mv_b.mv_comb;
		break;
	case MVPRED_UR:
		pmv->mv_comb = mv_c.mv_comb;
		break;
	default:
		pmv->mv_comb = 0;
		break;
	}
}

/*!
************************************************************************
* \brief
*    Set context for reference frames
************************************************************************
*/
int
BType2CtxRef (int btype)
{
	if (btype<4)  return 0;
	else          return 1;
}

CREL_RETURN mb_direct_spatial_mv_pred PARGS3(int current_mb_nr, Macroblock *currMB, Macroblock_s *currMB_s)
{	
	int j4, i4;
	int k;
	int mb_nr = current_mb_nr;
	int fw_rFrameL, fw_rFrameU, fw_rFrameUL, fw_rFrameUR;
	int bw_rFrameL, bw_rFrameU, bw_rFrameUL, bw_rFrameUR;

	PixelPos mb_left, mb_upleft;
	PixelPos *p_mb_left = &mb_left;

	int upright_y;

	int  fw_rFrame,bw_rFrame;
	MotionVector pmvfw, pmvbw;	

	MotionVector *pmv;

	Macroblock_s *mb_a, *mb_b, *mb_c, *mb_d;

	int off_x, off_y;

	Pred_s_info *info = &currMB_s->pred_info;

	int l_16_4_idx, idx;

	p_mb_left = &(IMGPAR left[0]);

	mb_c = 0;
	//getLuma4x4Neighbour ARGS6(mb_nr, 0, 0, 16, -1, &mb_upright);

	mb_upleft.pMB = IMGPAR pUpLeftMB_r;
	mb_upleft.mb_addr = IMGPAR mbAddrD;
	mb_upleft.x = 3;
	if (IMGPAR MbaffFrameFlag && currMB_r->mbStatusD == 5)
		mb_upleft.y = 1;
	else
		mb_upleft.y = 3;

	if(p_mb_left->pMB)
		mb_a = &dec_picture->mb_data[p_mb_left->mb_addr];
	if(IMGPAR pUpMB_r)
		mb_b = &dec_picture->mb_data[IMGPAR mbAddrB];
	if (IMGPAR pUpRightMB_r) {
		mb_c = &dec_picture->mb_data[IMGPAR mbAddrC];
		upright_y = ((-1<<IMGPAR mb_up_factor)+16)>>2;
	}
	if(mb_upleft.pMB)
		mb_d = &dec_picture->mb_data[mb_upleft.mb_addr];

	if (!IMGPAR MbaffFrameFlag)
	{
		if(mb_upleft.pMB)
		{
			l_16_4_idx = l_16_4[(mb_upleft.y<<2)+3];
			fw_rFrameUL = mb_d->pred_info.ref_idx[0][l_16_4_idx];
			bw_rFrameUL = mb_d->pred_info.ref_idx[1][l_16_4_idx];	
		}
		else
		{
			fw_rFrameUL = bw_rFrameUL = -1;
		}

		if(IMGPAR pUpMB_r)
		{
			l_16_4_idx = l_16_4[3<<2];
			fw_rFrameU = mb_b->pred_info.ref_idx[0][l_16_4_idx];
			bw_rFrameU = mb_b->pred_info.ref_idx[1][l_16_4_idx];
		}
		else
		{
			fw_rFrameU = bw_rFrameU = -1;
		}

		if(mb_c)
		{
			l_16_4_idx = l_16_4[upright_y<<2];
			fw_rFrameUR = mb_c->pred_info.ref_idx[0][l_16_4_idx];
			bw_rFrameUR = mb_c->pred_info.ref_idx[1][l_16_4_idx];
		}
		else
		{
			fw_rFrameUR = fw_rFrameUL;
			bw_rFrameUR = bw_rFrameUL;
		}

		if(p_mb_left->pMB)
		{
			l_16_4_idx = l_16_4[(p_mb_left->y<<2)+3];
			fw_rFrameL = mb_a->pred_info.ref_idx[0][l_16_4_idx];
			bw_rFrameL = mb_a->pred_info.ref_idx[1][l_16_4_idx];	
		}
		else
		{
			fw_rFrameL = bw_rFrameL = -1;
		}
	}
	else
	{
		if (currMB_s->mb_field)
		{
			if(mb_upleft.pMB)
			{
				l_16_4_idx = l_16_4[(mb_upleft.y<<2)+3];
				fw_rFrameUL = mb_d->pred_info.ref_idx[0][l_16_4_idx] << (!mb_d->mb_field);
				bw_rFrameUL = mb_d->pred_info.ref_idx[1][l_16_4_idx] << (!mb_d->mb_field);
			}
			else
			{
				fw_rFrameUL = bw_rFrameUL = -1;
			}

			if(IMGPAR pUpMB_r)
			{
				l_16_4_idx = l_16_4[3<<2];
				fw_rFrameU = mb_b->pred_info.ref_idx[0][l_16_4_idx] << (!mb_b->mb_field);
				bw_rFrameU = mb_b->pred_info.ref_idx[1][l_16_4_idx] << (!mb_b->mb_field);
			}
			else
			{
				fw_rFrameU = bw_rFrameU = -1;
			}

			if(mb_c)
			{
				l_16_4_idx = l_16_4[upright_y<<2];
				fw_rFrameUR = mb_c->pred_info.ref_idx[0][l_16_4_idx] << (!mb_c->mb_field);
				bw_rFrameUR = mb_c->pred_info.ref_idx[1][l_16_4_idx] << (!mb_c->mb_field);
			}
			else
			{
				fw_rFrameUR = fw_rFrameUL;
				bw_rFrameUR = bw_rFrameUL;
			}

			if(p_mb_left->pMB)
			{
				l_16_4_idx = l_16_4[(p_mb_left->y<<2)+3];
				fw_rFrameL = mb_a->pred_info.ref_idx[0][l_16_4_idx] << (!mb_a->mb_field);
				bw_rFrameL = mb_a->pred_info.ref_idx[1][l_16_4_idx] << (!mb_a->mb_field);
			}
			else
			{
				fw_rFrameL = bw_rFrameL = -1;
			}

		}
		else
		{
			if(mb_upleft.pMB)
			{
				l_16_4_idx = l_16_4[(mb_upleft.y<<2)+3];
				fw_rFrameUL = mb_d->pred_info.ref_idx[0][l_16_4_idx] >> (mb_d->mb_field);
				bw_rFrameUL = mb_d->pred_info.ref_idx[1][l_16_4_idx] >> (mb_d->mb_field);
			}
			else
			{
				fw_rFrameUL = bw_rFrameUL = -1;
			}

			if(IMGPAR pUpMB_r)
			{
				l_16_4_idx = l_16_4[3<<2];
				fw_rFrameU = mb_b->pred_info.ref_idx[0][l_16_4_idx] >> (mb_b->mb_field);
				bw_rFrameU = mb_b->pred_info.ref_idx[1][l_16_4_idx] >> (mb_b->mb_field);
			}
			else
			{
				fw_rFrameU = bw_rFrameU = -1;
			}

			if(mb_c)
			{
				l_16_4_idx = l_16_4[upright_y<<2];
				fw_rFrameUR = mb_c->pred_info.ref_idx[0][l_16_4_idx] >> (mb_c->mb_field);
				bw_rFrameUR = mb_c->pred_info.ref_idx[1][l_16_4_idx] >> (mb_c->mb_field);
			}
			else
			{
				fw_rFrameUR = fw_rFrameUL;
				bw_rFrameUR = bw_rFrameUL;
			}

			if(p_mb_left->pMB)
			{
				l_16_4_idx = l_16_4[(p_mb_left->y<<2)+3];
				fw_rFrameL = mb_a->pred_info.ref_idx[0][l_16_4_idx] >> (mb_a->mb_field);
				bw_rFrameL = mb_a->pred_info.ref_idx[1][l_16_4_idx] >> (mb_a->mb_field);
			}
			else
			{
				fw_rFrameL = bw_rFrameL = -1;
			}
		}
	}

	fw_rFrame = (fw_rFrameL >= 0 && fw_rFrameU >= 0) ? min(fw_rFrameL,fw_rFrameU): max(fw_rFrameL,fw_rFrameU);
	fw_rFrame = (fw_rFrame >= 0 && fw_rFrameUR >= 0) ? min(fw_rFrame,fw_rFrameUR): max(fw_rFrame,fw_rFrameUR);

	bw_rFrame = (bw_rFrameL >= 0 && bw_rFrameU >= 0) ? min(bw_rFrameL,bw_rFrameU): max(bw_rFrameL,bw_rFrameU);
	bw_rFrame = (bw_rFrame >= 0 && bw_rFrameUR >= 0) ? min(bw_rFrame,bw_rFrameUR): max(bw_rFrame,bw_rFrameUR);

	if (fw_rFrame >=0)
		SetMotionVectorPredictor_block00_shape16x16 ARGS6(&pmvfw, fw_rFrame, LIST_0, current_mb_nr, currMB, currMB_s);
	else
		pmvfw.mv_comb = 0;

	if (bw_rFrame >=0)
		SetMotionVectorPredictor_block00_shape16x16 ARGS6(&pmvbw, bw_rFrame, LIST_1, current_mb_nr, currMB, currMB_s);
	else
		pmvbw.mv_comb = 0;

	for (k=0;k<4;k++)
	{
		if (currMB->b8mode[k] == 0)	//Direct Mode for this 8x8
		{
			off_x = ((k&1) <<1) ;
			off_y = k&2 ;

			if (active_sps.direct_8x8_inference_flag)
			{	// frame_mbs_only_flag can be 0 or 1
				if ( (fw_rFrame >= 0) && (bw_rFrame >= 0) )	{
					pmv = &info->mv[0][b8_idx[k]];
					if  (!fw_rFrame  && (!Co_located_MB->moving_block[off_y][off_x])) {
						*(int*)pmv = 0;
					} else {
						*(int*)pmv = pmvfw.mv_comb;							
					}

					*(int*)(pmv+1) = *(int*)(pmv+4) = *(int*)(pmv+5) = *(int*)pmv;
					info->ref_idx[0][k] = fw_rFrame;
					//					temp1 = pmv->mv_comb;	//Fill MV Plane for ATI DXVA		

					pmv = &info->mv[1][b8_idx[k]];
					if  (!bw_rFrame && (!Co_located_MB->moving_block[off_y][off_x])) {
						*(int*)pmv = 0;
					} else {
						*(int*)pmv = pmvbw.mv_comb;
					}

					*(int*)(pmv+1) = *(int*)(pmv+4) = *(int*)(pmv+5) = *(int*)pmv;
					info->ref_idx[1][k] = bw_rFrame;

				} else if ( (fw_rFrame < 0) && (bw_rFrame >= 0) ) {

					if  (!bw_rFrame && (!Co_located_MB->moving_block[off_y][off_x])) {
						info->mv[1][b8_idx[k]].mv_comb = 0;
					} else {
						info->mv[1][b8_idx[k]].mv_comb = pmvbw.mv_comb;
					}
					info->ref_idx[1][k] = bw_rFrame;
					pmv = &info->mv[1][b8_idx[k]];
					*(pmv+1) = *(pmv+4) = *(pmv+5) = *pmv;

				} else if ( (fw_rFrame >= 0) && (bw_rFrame < 0) ) {

					if  (!fw_rFrame  && (!Co_located_MB->moving_block[off_y][off_x])) {
						info->mv[0][b8_idx[k]].mv_comb = 0;
					} else {
						info->mv[0][b8_idx[k]].mv_comb = pmvfw.mv_comb;
					}
					info->ref_idx[0][k] = fw_rFrame;
					pmv = &info->mv[0][b8_idx[k]];
					*(pmv+1) = *(pmv+4) = *(pmv+5) = *pmv;

				} else {
					info->ref_idx[0][k] = 0;
					info->ref_idx[1][k] = 0;	

				}

			}
			else
			{   // frame_mbs_only_flag=1

				for(j4=off_y; j4<off_y+2; j4++)
				{
					for(i4=off_x; i4<off_x+2; i4++)
					{
						idx = j4_i4[j4][i4];	// 4*j4 + i4						

						if ( (fw_rFrame >= 0) && (bw_rFrame >= 0) ) {
							if  (!fw_rFrame  && (!Co_located_MB->moving_block[j4][i4]))
							{
								info->mv[0][idx].mv_comb = 0;
							} else {
								info->mv[0][idx].mv_comb = pmvfw.mv_comb;
							}
							info->ref_idx[0][k] = fw_rFrame;

							if  (bw_rFrame==0 && (!Co_located_MB->moving_block[j4][i4]))
							{
								info->mv[1][idx].mv_comb = 0;
							}
							else
							{
								info->mv[1][idx].mv_comb = pmvbw.mv_comb;
							}
							info->ref_idx[1][k] = bw_rFrame;							
						} else if ( (fw_rFrame < 0) && (bw_rFrame >= 0) ) {
							if  (bw_rFrame==0 && (!Co_located_MB->moving_block[j4][i4]))
							{
								info->mv[1][idx].mv_comb = 0;
							}
							else
							{
								info->mv[1][idx].mv_comb = pmvbw.mv_comb;
							}
							info->ref_idx[1][k] = bw_rFrame;
						} else if ( (fw_rFrame >= 0) && (bw_rFrame < 0) ) {
							if  (!fw_rFrame  && (!Co_located_MB->moving_block[j4][i4]))
							{
								info->mv[0][idx].mv_comb = 0;
							} else {
								info->mv[0][idx].mv_comb = pmvfw.mv_comb;
							}
							info->ref_idx[0][k] = fw_rFrame;

						} else {
							info->ref_idx[0][k] = 0;
							info->ref_idx[1][k] = 0;
						}

					}
				}
			}
		}
	}

	return CREL_OK;
}

CREL_RETURN mb_direct_temporal_mv_pred PARGS3(int current_mb_nr, Macroblock *currMB, Macroblock_s *currMB_s)
{
	int list_offset;// = ((IMGPAR MbaffFrameFlag)&&(currMB->mb_field))? IMGPAR current_mb_nr%2 ? 4 : 2 : 0;

	Pred_s_info *info = &currMB_s->pred_info;

	Pred_s_info *co_info;

	co_info = &Co_located_MB->pred_info;

	int i, j;
	int b4, b8, i_1, j_1;

#ifdef DO_REF_PIC_NUM
	int64 *pRef_pic_num;
#endif
	int num_ref;

	if ((IMGPAR MbaffFrameFlag)&&(currMB_s->mb_field))
	{
		list_offset = (current_mb_nr & 1) ? 4 : 2;
	}
	else
	{
		list_offset = 0;
	}


#ifdef DO_REF_PIC_NUM
	pRef_pic_num = dec_picture->ref_pic_num[IMGPAR current_slice_nr][LIST_0 + list_offset];
#endif

	num_ref = min(IMGPAR num_ref_idx_l0_active,listXsize[LIST_0 + list_offset]);

	if(active_sps.direct_8x8_inference_flag)
	{   // frame_mbs_only_flag can be 0 or 1
		MotionVector *pmv;
		//char *pred_idx;

		for (b8=0;b8<4;b8++)
		{
			if (currMB->b8mode[b8] == 0)	//Direct Mode for this 8x8 sub-macroblock
			{
				i = ((b8&1)<<1);
				j = (b8&2);
				int refList = (co_info->ref_idx[0][b8]== -1 ? LIST_1 : LIST_0);
				int co_loc_refidx = co_info->ref_idx[refList][b8];
				int co_loc_refpicid = co_info->ref_pic_id[refList][b8];
				MotionVector two_mv;
				two_mv.mv_comb = co_info->mv[refList][b8_idx[b8]].mv_comb;

				if(co_loc_refidx==-1) // co-located is intra mode
				{
					info->mv[0][b8_idx[b8]].mv_comb = 0;
					info->mv[1][b8_idx[b8]].mv_comb = 0;

					info->ref_idx[0][b8] = 0;
					info->ref_idx[1][b8] = 0;
				}
				else // co-located skip or inter mode
				{
					int mapped_idx=0;
					int iref;
					int mv_scale;

					for (iref=0;iref<num_ref;iref++)
					{
						if (listX[LIST_0 + list_offset][iref]->unique_id == co_loc_refpicid)
						{
							mapped_idx=iref;
							break;
						}
						else //! invalid index. Default to zero even though this case should not happen
						{
							mapped_idx=INVALIDINDEX;
						}
					}
					if (INVALIDINDEX == mapped_idx)
					{
						DEBUG_SHOW_ERROR_INFO("[ERROR]temporal direct error\ncolocated block has ref that is unavailable",-1111);
						return CREL_WARNING_H264_STREAMERROR_LEVEL_3;
					}

					mv_scale  = IMGPAR mvscale[LIST_0 + list_offset][mapped_idx];

					//! In such case, an array is needed for each different reference.
					if (mv_scale == 9999 || listX[LIST_0+list_offset][mapped_idx]->is_long_term)
					{
						info->mv[0][b8_idx[b8]].mv_comb = two_mv.mv_comb;
						info->mv[1][b8_idx[b8]].mv_comb = 0;
					}
					else
					{
						info->mv[0][b8_idx[b8]].x=(mv_scale * two_mv.x + 128 ) >> 8;
						info->mv[0][b8_idx[b8]].y=(mv_scale * two_mv.y + 128 ) >> 8;

						info->mv[1][b8_idx[b8]].x=info->mv[0][b8_idx[b8]].x - two_mv.x;
						info->mv[1][b8_idx[b8]].y=info->mv[0][b8_idx[b8]].y - two_mv.y;
					}

					info->ref_idx[0][b8] = mapped_idx; //listX[1][0]->ref_idx[refList][j4][i4];
					info->ref_idx[1][b8] = 0;
				}

				pmv = &info->mv[0][b8_idx[b8]];
				*(pmv+1) = *(pmv+4) = *(pmv+5) = *pmv;

				pmv = &info->mv[1][b8_idx[b8]];
				*(pmv+1) = *(pmv+4) = *(pmv+5) = *pmv;

			}
		}
	}
	else
	{   // frame_mbs_only_flag=1
		for (b8=0;b8<4;b8++)
		{
			if (currMB->b8mode[b8] == 0)	//Direct Mode for this 8x8 sub-macroblock
			{
				i_1 = ((b8&1)<<1);
				j_1 = (b8&2);

				for (b4=0;b4<4;b4++)
				{
					i    = i_1 + (b4&1);
					j    = j_1 + (b4>>1);

					int refList = (co_info->ref_idx[0][b8]== -1 ? LIST_1 : LIST_0);
					int idx = j4_i4[j][i];	// 4*j + i

					if(co_info->ref_idx[refList][b8]==-1) // co-located is intra mode
					{
						info->mv[0][idx].mv_comb = 0;
						info->mv[1][idx].mv_comb = 0;

						info->ref_idx[0][b8] = 0;
						info->ref_idx[1][b8] = 0;
					}
					else // co-located skip or inter mode
					{
						int mapped_idx=0;
						int iref;
						int mv_scale;

						for (iref=0;iref<num_ref;iref++)
						{
							if (listX[LIST_0 + list_offset][iref]->unique_id == co_info->ref_pic_id[refList][b8])
							{
								mapped_idx=iref;
								break;
							}
							else //! invalid index. Default to zero even though this case should not happen
							{                        
								mapped_idx=INVALIDINDEX;
							}
						}
						if (INVALIDINDEX == mapped_idx)
						{
							DEBUG_SHOW_ERROR_INFO("[ERROR]temporal direct error\ncolocated block has ref that is unavailable",-1111);
							return CREL_WARNING_H264_STREAMERROR_LEVEL_3;
						}					

						mv_scale  = IMGPAR mvscale[LIST_0 + list_offset][mapped_idx];

						//! In such case, an array is needed for each different reference.
						if (mv_scale == 9999 || listX[LIST_0+list_offset][mapped_idx]->is_long_term)
						{
							info->mv[0][idx].mv_comb = co_info->mv[refList][idx].mv_comb;
							info->mv[1][idx].mv_comb = 0;
						}
						else
						{
							MotionVector two_mv;
							two_mv.mv_comb = co_info->mv[refList][idx].mv_comb;

							info->mv[0][idx].x=(mv_scale * two_mv.x + 128 ) >> 8;
							info->mv[0][idx].y=(mv_scale * two_mv.y + 128 ) >> 8;

							info->mv[1][idx].x=info->mv[0][idx].x - two_mv.x;
							info->mv[1][idx].y=info->mv[0][idx].y - two_mv.y;
						}

						info->ref_idx[0][b8] = mapped_idx; //listX[1][0]->ref_idx[refList][j4][i4];
						info->ref_idx[1][b8] = 0;
					}
				}
			}
		}
	}

	return CREL_OK;
}

CREL_RETURN record_reference_picIds PARGS2(int list_offset, Macroblock_s *currMB_s)
{
	char *pref_idx;
	char *pref_pic_id;
	StorablePicture **pList;
	int list_size;
#ifdef DO_REF_PIC_NUM
	int64 *ref_pic_num;
#endif

	pref_idx = (char*)currMB_s->pred_info.ref_idx[0];
	pref_pic_id = (char*)currMB_s->pred_info.ref_pic_id[LIST_0];
#ifdef DO_REF_PIC_NUM
	ref_pic_num = (int64*)&dec_picture->ref_pic_num[IMGPAR current_slice_nr][LIST_0+list_offset];	
#endif

	pList = listX[LIST_0+list_offset];
	list_size = listXsize[LIST_0+list_offset];	

	if(pref_idx[0]>=0) {
		if (pref_idx[0] < list_size) {
			pref_pic_id[0] = pList[pref_idx[0]]->unique_id;
		} else {	//Error either reference idex (from bitstream) or DPB list error, try to conceal the error to avoid SeekIDR
			if ( list_size == 0 ) {
				return CREL_WARNING_H264_STREAMERROR_LEVEL_1;
			}
			if ( pList[list_size-1] ) {
				pref_pic_id[0] = pList[list_size-1]->unique_id;
				pref_idx[0] = list_size - 1;
			} else {
				pref_pic_id[0] = pList[0]->unique_id;
				pref_idx[0] = 0;
			}
		}
	}
	if(pref_idx[1]>=0) {
		if (pref_idx[1] < list_size) {
			pref_pic_id[1] = pList[pref_idx[1]]->unique_id;
		} else {	//Error either reference idex (from bitstream) or DPB list error, try to conceal the error to avoid SeekIDR
			if ( list_size == 0 ) {
				return CREL_WARNING_H264_STREAMERROR_LEVEL_1;
			}

			if ( pList[list_size-1] ) {
				pref_pic_id[1] = pList[list_size-1]->unique_id;
				pref_idx[1] = list_size - 1;
			} else {
				pref_pic_id[1] = pList[0]->unique_id;
				pref_idx[1] = 0;
			}
		}
	}
	if(pref_idx[2]>=0) {
		if (pref_idx[2] < list_size) {
			pref_pic_id[2] = pList[pref_idx[2]]->unique_id;
		} else {	//Error either reference idex (from bitstream) or DPB list error, try to conceal the error to avoid SeekIDR
			if ( list_size == 0 ) {
				return CREL_WARNING_H264_STREAMERROR_LEVEL_1;
			}

			if ( pList[list_size-1] ) {
				pref_pic_id[2] = pList[list_size-1]->unique_id;
				pref_idx[2] = list_size - 1;
			} else {
				pref_pic_id[2] = pList[0]->unique_id;
				pref_idx[2] = 0;
			}
		}
	}		
	if(pref_idx[3]>=0) {
		if (pref_idx[3] < list_size) {
			pref_pic_id[3] = pList[pref_idx[3]]->unique_id;
		} else {	//Error either reference idex (from bitstream) or DPB list error, try to conceal the error to avoid SeekIDR
			if ( list_size == 0 ) {
				return CREL_WARNING_H264_STREAMERROR_LEVEL_1;
			}

			if ( pList[list_size-1] ) {
				pref_pic_id[3] = pList[list_size-1]->unique_id;
				pref_idx[3] = list_size - 1;
			} else {
				pref_pic_id[3] = pList[0]->unique_id;
				pref_idx[3] = 0;
			}
		}
	}

	if(IMGPAR type == B_SLICE)
	{
		pref_idx = (char*)currMB_s->pred_info.ref_idx[1];
		pref_pic_id = (char*)currMB_s->pred_info.ref_pic_id[LIST_1];
#ifdef DO_REF_PIC_NUM
		ref_pic_num = (int64*)&dec_picture->ref_pic_num[IMGPAR current_slice_nr][LIST_1+list_offset];
#endif
		pList = listX[LIST_1+list_offset];
		list_size = listXsize[LIST_1+list_offset];

		if(pref_idx[0]>=0) {
			if (pref_idx[0] < list_size) {
				pref_pic_id[0] = pList[pref_idx[0]]->unique_id;
			} else {	//Error either reference idex (from bitstream) or DPB list error, try to conceal the error to avoid SeekIDR
				if ( list_size == 0 ) {
					return CREL_WARNING_H264_STREAMERROR_LEVEL_1;
				}

				if ( pList[list_size-1] ) {
					pref_pic_id[0] = pList[list_size-1]->unique_id;
					pref_idx[0] = list_size - 1;
				} else {
					pref_pic_id[0] = pList[0]->unique_id;
					pref_idx[0] = 0;
				}
			}
		}
		if(pref_idx[1]>=0) {
			if (pref_idx[1] < list_size) {
				pref_pic_id[1] = pList[pref_idx[1]]->unique_id;
			} else {	//Error either reference idex (from bitstream) or DPB list error, try to conceal the error to avoid SeekIDR
				if ( list_size == 0 ) {
					return CREL_WARNING_H264_STREAMERROR_LEVEL_1;
				}

				if ( pList[list_size-1] ) {
					pref_pic_id[1] = pList[list_size-1]->unique_id;
					pref_idx[1] = list_size - 1;
				} else {
					pref_pic_id[1] = pList[0]->unique_id;
					pref_idx[1] = 0;
				}
			}
		}
		if(pref_idx[2]>=0) {
			if (pref_idx[2] < list_size) {
				pref_pic_id[2] = pList[pref_idx[2]]->unique_id;
			} else {	//Error either reference idex (from bitstream) or DPB list error, try to conceal the error to avoid SeekIDR
				if ( list_size == 0 ) {
					return CREL_WARNING_H264_STREAMERROR_LEVEL_1;
				}

				if ( pList[list_size-1] ) {
					pref_pic_id[2] = pList[list_size-1]->unique_id;
					pref_idx[2] = list_size - 1;
				} else {
					pref_pic_id[2] = pList[0]->unique_id;
					pref_idx[2] = 0;
				}
			}
		}		
		if(pref_idx[3]>=0) {
			if (pref_idx[3] < list_size) {
				pref_pic_id[3] = pList[pref_idx[3]]->unique_id;
			} else {	//Error either reference idex (from bitstream) or DPB list error, try to conceal the error to avoid SeekIDR
				if ( list_size == 0 ) {
					return CREL_WARNING_H264_STREAMERROR_LEVEL_1;
				}

				if ( pList[list_size-1] ) {
					pref_pic_id[3] = pList[list_size-1]->unique_id;
					pref_idx[3] = list_size - 1;
				} else {
					pref_pic_id[3] = pList[0]->unique_id;
					pref_idx[3] = 0;
				}
			}
		}
	}

	return CREL_OK;
}

CREL_RETURN readRefInfo_UVLC PARGS3(int flag_mode, int list, Pred_s_info *info)
{
	int k;
	int refframe;

	switch(currMB_r->mb_type)
	{
	case PB_16x16:
		k=0;
		if (((currMB_r->b8pdir[k]+1)&(list+1)))
		{
			IMGPAR subblock_x = 0;
			IMGPAR subblock_y = 0;

			if(flag_mode)
				refframe = 1 - readSyntaxElement_FLC ARGS1(1);
			else
				refframe = readSyntaxElement_VLC_ue ARGS0();
			/*
			if (refframe >= IMGPAR m_listXsize[list]) {
			return CREL_WARNING_H264_STREAMERROR_LEVEL_3;
			}
			*/
			info->ref_idx[list][0]=info->ref_idx[list][1]=
				info->ref_idx[list][2]=info->ref_idx[list][3]=refframe;
		}
		break;
	case PB_16x8:
		k=0;
		if (((currMB_r->b8pdir[k]+1)&(list+1)))
		{
			IMGPAR subblock_x = 0;
			IMGPAR subblock_y = 0;

			if(flag_mode)
				refframe = 1 - readSyntaxElement_FLC ARGS1(1);
			else
				refframe = readSyntaxElement_VLC_ue ARGS0();

			/*
			if (refframe >= IMGPAR m_listXsize[list]) {
			return CREL_WARNING_H264_STREAMERROR_LEVEL_3;
			}
			*/

			info->ref_idx[list][0]=info->ref_idx[list][1]=refframe;			
		}

		k=2;
		if (((currMB_r->b8pdir[k]+1)&(list+1)))
		{
			IMGPAR subblock_x = 0;
			IMGPAR subblock_y = 2;

			if(flag_mode)
				refframe = 1 - readSyntaxElement_FLC ARGS1(1);
			else
				refframe = readSyntaxElement_VLC_ue ARGS0();

			/*
			if (refframe >= IMGPAR m_listXsize[list]) {
			return CREL_WARNING_H264_STREAMERROR_LEVEL_3;
			}
			*/

			info->ref_idx[list][2]=info->ref_idx[list][3]=refframe;
		}
		break;
	case PB_8x16:
		k=0;
		if (((currMB_r->b8pdir[k]+1)&(list+1)))
		{
			IMGPAR subblock_x = 0;
			IMGPAR subblock_y = 0;

			if(flag_mode)
				refframe = 1 - readSyntaxElement_FLC ARGS1(1);
			else
				refframe = readSyntaxElement_VLC_ue ARGS0();

			/*
			if (refframe >= IMGPAR m_listXsize[list]) {
			return CREL_WARNING_H264_STREAMERROR_LEVEL_3;
			}
			*/

			info->ref_idx[list][0]=info->ref_idx[list][2]=refframe;			
		}

		k=1;
		if (((currMB_r->b8pdir[k]+1)&(list+1)))
		{
			IMGPAR subblock_x = 2;
			IMGPAR subblock_y = 0;

			if(flag_mode)
				refframe = 1 - readSyntaxElement_FLC ARGS1(1);
			else
				refframe = readSyntaxElement_VLC_ue ARGS0();

			/*
			if (refframe >= IMGPAR m_listXsize[list]) {
			return CREL_WARNING_H264_STREAMERROR_LEVEL_3;
			}
			*/

			info->ref_idx[list][1]=info->ref_idx[list][3]=refframe;
		}
		break;
	case PB_8x8:		
		for(k=0;k<4;k++)
		{
			if (((currMB_r->b8pdir[k]+1)&(list+1)) && currMB_r->b8mode[k]!=0)
			{
				IMGPAR subblock_x = (k&1)<<1;
				IMGPAR subblock_y = (k&2);

				if(flag_mode)
					refframe = 1 - readSyntaxElement_FLC ARGS1(1);
				else
					refframe = readSyntaxElement_VLC_ue ARGS0();

				/*
				if (refframe >= IMGPAR m_listXsize[list]) {
				return CREL_WARNING_H264_STREAMERROR_LEVEL_3;
				}
				*/

				info->ref_idx[list][k] = refframe;							
			}
		}
		break;
	}

	return CREL_OK;
}


CREL_RETURN readRefInfo_CABAC PARGS3(int flag_mode, int list, Pred_s_info *info)
{
	int k;
	int refframe;	

	switch(currMB_r->mb_type)
	{
	case PB_16x16:
		k=0;
		if (((currMB_r->b8pdir[k]+1)&(list+1)))
		{
			IMGPAR subblock_x = 0;
			IMGPAR subblock_y = 0;

			refframe = readRefFrame_CABAC ARGS1(list);
			/*
			if (refframe >= IMGPAR m_listXsize[list]) {
			return CREL_WARNING_H264_STREAMERROR_LEVEL_3;
			}
			*/
			info->ref_idx[list][0]=info->ref_idx[list][1]=
				info->ref_idx[list][2]=info->ref_idx[list][3]=refframe;
		}
		break;
	case PB_16x8:
		k=0;
		if (((currMB_r->b8pdir[k]+1)&(list+1)))
		{
			IMGPAR subblock_x = 0;
			IMGPAR subblock_y = 0;

			refframe = readRefFrame_CABAC ARGS1(list);
			/*
			if (refframe >= IMGPAR m_listXsize[list]) {
			return CREL_WARNING_H264_STREAMERROR_LEVEL_3;
			}
			*/
			info->ref_idx[list][0]=info->ref_idx[list][1]=refframe;			
		}

		k=2;
		if (((currMB_r->b8pdir[k]+1)&(list+1)))
		{
			IMGPAR subblock_x = 0;
			IMGPAR subblock_y = 2;

			refframe = readRefFrame_CABAC ARGS1(list);		
			/*
			if (refframe >= IMGPAR m_listXsize[list]) {
			return CREL_WARNING_H264_STREAMERROR_LEVEL_3;
			}
			*/
			info->ref_idx[list][2]=info->ref_idx[list][3]=refframe;
		}
		break;
	case PB_8x16:
		k=0;
		if (((currMB_r->b8pdir[k]+1)&(list+1)))
		{
			IMGPAR subblock_x = 0;
			IMGPAR subblock_y = 0;

			refframe = readRefFrame_CABAC ARGS1(list);
			/*
			if (refframe >= IMGPAR m_listXsize[list]) {
			return CREL_WARNING_H264_STREAMERROR_LEVEL_3;
			}
			*/
			info->ref_idx[list][0]=info->ref_idx[list][2]=refframe;			
		}

		k=1;
		if (((currMB_r->b8pdir[k]+1)&(list+1)))
		{
			IMGPAR subblock_x = 2;
			IMGPAR subblock_y = 0;

			refframe = readRefFrame_CABAC ARGS1(list);		

			info->ref_idx[list][1]=info->ref_idx[list][3]=refframe;
		}
		break;
	case PB_8x8:		
		for(k=0;k<4;k++)
		{
			if (((currMB_r->b8pdir[k]+1)&(list+1)) && currMB_r->b8mode[k]!=0)
			{
				IMGPAR subblock_x = (k&1)<<1;
				IMGPAR subblock_y = (k&2);

				refframe = readRefFrame_CABAC ARGS1(list);
				/*
				if (refframe >= IMGPAR m_listXsize[list]) {
				return CREL_WARNING_H264_STREAMERROR_LEVEL_3;
				}
				*/
				info->ref_idx[list][k] = refframe;							
			}
		}
		break;
	}

	return CREL_OK;

}


void ReadMotionInfo16x16 PARGS2(Pred_s_info *info, int list)
{
	MotionVector vec;
	MotionVector curr_mvd;	
	int refframe;

	refframe = info->ref_idx[list][0];

	IMGPAR subblock_y = 0; // position used for context determination
	IMGPAR subblock_x = 0; // position used for context determination

	// first make mv-prediction
	SetMotionVectorPredictor_block00_shape16x16 ARGS6(&vec, refframe, list, IMGPAR current_mb_nr_r, currMB_r, currMB_s_r);

	// set motion vectors (x, y)	

	g_read_functions->readMVD ARGS2(list,&curr_mvd);
	// x
	vec.x = vec.x + curr_mvd.x;
	// y
	vec.y = vec.y + curr_mvd.y;


	Set16MotionVector(info->mv[list], vec);
	/*
	DWORD *pDestArray = (unsigned long *) &info->mv[0];
	__asm 
	{
	MOV eax, vec;
	MOV eax, [eax];
	MOV ecx, 16;
	REP STOS pDestArray;
	}
	//*/

	Set16MotionVector(IMGPAR curr_mvd[list], curr_mvd);
}

void ReadMotionInfo16x8 PARGS2(Pred_s_info *info, int list)
{
	MotionVector vec;
	MotionVector curr_mvd;	
	int refframe;	
	MotionVector *pMvTemp;

	if ((currMB_r->b8pdir[0]==list || currMB_r->b8pdir[0]==2))//has forward vector
	{
		refframe = info->ref_idx[list][0];
		pMvTemp = &info->mv[list][0];		

		IMGPAR subblock_x = 0; // position used for context determination
		IMGPAR subblock_y = 0; // position used for context determination

		// first make mv-prediction
		SetMotionVectorPredictor_block00_shape16x8 ARGS6(&vec, refframe, list, IMGPAR current_mb_nr_r, currMB_r, currMB_s_r);

		// set motion vectors (x, y)	
		// x
		g_read_functions->readMVD ARGS2(list,&curr_mvd);
		// x
		vec.x = vec.x + curr_mvd.x;
		// y
		vec.y = vec.y + curr_mvd.y;

		Set16x8MotionVector(pMvTemp, vec);

		pMvTemp = &IMGPAR curr_mvd[list][0];

		Set16x8MotionVector(pMvTemp, curr_mvd);
	}
	else
	{
		pMvTemp = &IMGPAR curr_mvd[list][0];
		curr_mvd.mv_comb=0;
		Set16x8MotionVector(pMvTemp, curr_mvd);
	}

	if ((currMB_r->b8pdir[2]==list || currMB_r->b8pdir[2]==2))//has forward vector
	{
		refframe = info->ref_idx[list][2];
		pMvTemp = &info->mv[list][8];

		IMGPAR subblock_x = 0; // position used for context determination
		IMGPAR subblock_y = 2; // position used for context determination

		// first make mv-prediction
		SetMotionVectorPredictor_block02_shape16x8 ARGS6(&vec, refframe, list, IMGPAR current_mb_nr_r, currMB_r, currMB_s_r);

		// set motion vectors (x, y)	
		// x
		g_read_functions->readMVD ARGS2(list,&curr_mvd);
		// x
		vec.x = vec.x + curr_mvd.x;
		// y
		vec.y = vec.y + curr_mvd.y;

		Set16x8MotionVector(pMvTemp, vec);

		pMvTemp = &IMGPAR curr_mvd[list][8];

		Set16x8MotionVector(pMvTemp, curr_mvd);
	}
	else
	{
		pMvTemp = &IMGPAR curr_mvd[list][8];
		curr_mvd.mv_comb=0;
		Set16x8MotionVector(pMvTemp, curr_mvd);
	}
}

void ReadMotionInfo8x16 PARGS2(Pred_s_info *info, int list)
{
	MotionVector vec;
	MotionVector curr_mvd;
	int refframe;
	MotionVector *pMvTemp;	

	if ((currMB_r->b8pdir[0]==list || currMB_r->b8pdir[0]==2))//has forward vector
	{
		refframe = info->ref_idx[list][0];
		pMvTemp = &info->mv[list][0];

		IMGPAR subblock_x = 0; // position used for context determination
		IMGPAR subblock_y = 0; // position used for context determination

		// first make mv-prediction
		SetMotionVectorPredictor_block00_shape8x16 ARGS6(&vec, refframe, list, IMGPAR current_mb_nr_r, currMB_r, currMB_s_r);

		// set motion vectors (x, y)	
		// x
		g_read_functions->readMVD ARGS2(list,&curr_mvd);
		// x
		vec.x = vec.x + curr_mvd.x;
		// y
		vec.y = vec.y + curr_mvd.y;

		Set8x16MotionVector(pMvTemp, vec);

		/*
		*(int*)pMvTemp		= temp;
		*(int*)(pMvTemp+1)	= temp;

		pMvTemp += stride;

		*(int*)pMvTemp		= temp;
		*(int*)(pMvTemp+1)	= temp;

		pMvTemp += stride;

		*(int*)pMvTemp		= temp;
		*(int*)(pMvTemp+1)	= temp;

		pMvTemp += stride;

		*(int*)pMvTemp		= temp;
		*(int*)(pMvTemp+1)	= temp;	
		*/


		pMvTemp = &IMGPAR curr_mvd[list][0];		
		Set8x16MotionVector(pMvTemp, curr_mvd);
	}
	else
	{
		pMvTemp = &IMGPAR curr_mvd[list][0];
		curr_mvd.mv_comb=0;
		Set8x16MotionVector(pMvTemp, curr_mvd);
	}

	if ((currMB_r->b8pdir[1]==list || currMB_r->b8pdir[1]==2))//has forward vector
	{
		refframe = info->ref_idx[list][1];
		pMvTemp = &info->mv[list][2];

		IMGPAR subblock_x = 2; // position used for context determination
		IMGPAR subblock_y = 0; // position used for context determination

		// first make mv-prediction
		SetMotionVectorPredictor_block20_shape8x16 ARGS6(&vec, refframe, list, IMGPAR current_mb_nr_r, currMB_r, currMB_s_r);

		// set motion vectors (x, y)	
		// x
		g_read_functions->readMVD ARGS2(list,&curr_mvd);
		// x
		vec.x = vec.x + curr_mvd.x;
		// y
		vec.y = vec.y + curr_mvd.y;

		Set8x16MotionVector(pMvTemp, vec);
		/*
		*(int*)pMvTemp = temp;
		*(int*)(pMvTemp+1) = temp;			

		pMvTemp += stride;
		*(int*)pMvTemp = temp;
		*(int*)(pMvTemp+1) = temp;			

		pMvTemp += stride;
		*(int*)pMvTemp = temp;
		*(int*)(pMvTemp+1) = temp;			

		pMvTemp += stride;
		*(int*)pMvTemp = temp;
		*(int*)(pMvTemp+1) = temp;	
		*/

		pMvTemp = &IMGPAR curr_mvd[list][2];		
		Set8x16MotionVector(pMvTemp, curr_mvd);
	}
	else
	{
		pMvTemp = &IMGPAR curr_mvd[list][2];
		curr_mvd.mv_comb=0;
		Set8x16MotionVector(pMvTemp, curr_mvd);
	}
}

void ReadMotionInfo8x8 PARGS2(Pred_s_info *info, int list)
{
	MotionVector vec;
	MotionVector curr_mvd;
	int refframe;
	char *pb8dir = &(currMB_r->b8pdir[0]);
	byte *pb8mode = &(currMB_r->b8mode[0]);
	int i,j,i0,j0;
	int k;	
	MotionVector *pmvd;	

	for (k=0; k<4; k++)
	{
		i0 = (k&1)<<1;
		j0 = (k&2);
		if ((*(pb8dir+k) + list != 1) && (*(pb8mode+k) !=0))//has forward vector or backward vector
		{			
			refframe = info->ref_idx[list][k];

			switch (*(pb8mode+k)){
				case 4: //(2,2)
					IMGPAR subblock_y = j0; 
					IMGPAR subblock_x = i0;					

					SetMotionVectorPredictor ARGS7(&vec, refframe, list, i0, j0, 8, 8);

					g_read_functions->readMVD ARGS2(list,&curr_mvd);			

					vec.x = vec.x + curr_mvd.x;		
					vec.y = vec.y + curr_mvd.y;

					// x, y mvds
					pmvd = &IMGPAR curr_mvd[list][j0*4+i0];
					(pmvd+0)->mv_comb = (pmvd+1)->mv_comb = (pmvd+4)->mv_comb = (pmvd+5)->mv_comb = curr_mvd.mv_comb;

					// x, y vectors
					pmvd = &info->mv[list][j0*4+i0];
					(pmvd+0)->mv_comb = (pmvd+1)->mv_comb = (pmvd+4)->mv_comb = (pmvd+5)->mv_comb = vec.mv_comb;

					break;
				case 5: //(2,1)

					i = i0;
					j = j0;

					IMGPAR subblock_x = i;		
					IMGPAR subblock_y = j; 

					SetMotionVectorPredictor ARGS7(&vec, refframe, list, i, j, 8, 4);

					g_read_functions->readMVD ARGS2(list,&curr_mvd);			

					vec.x = vec.x + curr_mvd.x;		
					vec.y = vec.y + curr_mvd.y;

					// x, y mvds
					pmvd = &IMGPAR curr_mvd[list][j*4+i];
					(pmvd+0)->mv_comb = (pmvd+1)->mv_comb = curr_mvd.mv_comb;

					// x, y vectors
					pmvd = &info->mv[list][j*4+i];
					(pmvd+0)->mv_comb = (pmvd+1)->mv_comb = vec.mv_comb;

					i = i0;
					j = j0+1;

					IMGPAR subblock_x = i;		
					IMGPAR subblock_y = j; 

					SetMotionVectorPredictor ARGS7(&vec, refframe, list, i, j, 8, 4);

					g_read_functions->readMVD ARGS2(list,&curr_mvd);			

					vec.x = vec.x + curr_mvd.x;		
					vec.y = vec.y + curr_mvd.y;

					// x, y mvds
					pmvd = &IMGPAR curr_mvd[list][j*4+i];
					(pmvd+0)->mv_comb = (pmvd+1)->mv_comb = curr_mvd.mv_comb;

					// x, y vectors
					pmvd = &info->mv[list][j*4+i];
					(pmvd+0)->mv_comb = (pmvd+1)->mv_comb = vec.mv_comb;

					break;
				case 6: //(1,2)

					i = i0;
					j = j0;

					IMGPAR subblock_x = i;		
					IMGPAR subblock_y = j; 

					SetMotionVectorPredictor ARGS7(&vec, refframe, list, i, j, 4, 8);

					g_read_functions->readMVD ARGS2(list,&curr_mvd);			

					vec.x = vec.x + curr_mvd.x;		
					vec.y = vec.y + curr_mvd.y;

					// x, y mvds
					pmvd = &IMGPAR curr_mvd[list][j*4+i];
					(pmvd+0)->mv_comb = (pmvd+4)->mv_comb = curr_mvd.mv_comb;

					// x, y vectors
					pmvd = &info->mv[list][j*4+i];
					(pmvd+0)->mv_comb = (pmvd+4)->mv_comb = vec.mv_comb;

					i = i0+1;
					j = j0;

					IMGPAR subblock_x = i;		
					IMGPAR subblock_y = j; 

					SetMotionVectorPredictor ARGS7(&vec, refframe, list, i, j, 4, 8);

					g_read_functions->readMVD ARGS2(list,&curr_mvd);			

					vec.x = vec.x + curr_mvd.x;		
					vec.y = vec.y + curr_mvd.y;

					// x, y mvds
					pmvd = &IMGPAR curr_mvd[list][j*4+i];
					(pmvd+0)->mv_comb = (pmvd+4)->mv_comb = curr_mvd.mv_comb;

					// x, y vectors
					pmvd = &info->mv[list][j*4+i];
					(pmvd+0)->mv_comb = (pmvd+4)->mv_comb = vec.mv_comb;

					break;
				case 7: //(1,1)

					for (j=j0; j<j0+2; j++)
					{
						IMGPAR subblock_y = j;
						for (i=i0; i<i0+2; i++)
						{
							IMGPAR subblock_x = i;

							SetMotionVectorPredictor ARGS7(&vec, refframe, list, i, j, 4, 4);

							g_read_functions->readMVD ARGS2(list,&curr_mvd);			

							vec.x = vec.x + curr_mvd.x;		
							vec.y = vec.y + curr_mvd.y;

							// x, y mvds							
							IMGPAR curr_mvd[list][j*4+i].mv_comb = curr_mvd.mv_comb;

							// x, y vectors							
							info->mv[list][j*4+i].mv_comb = vec.mv_comb;
						}
					}
					break;

				default:
					break;
			}
		} else {
			pmvd = &IMGPAR curr_mvd[list][j0*4+i0];		//Small trick there => k*2 + (k&0x02)*2 => (k + (k&2))*2
			(pmvd+0)->mv_comb = (pmvd+1)->mv_comb = (pmvd+4)->mv_comb = (pmvd+5)->mv_comb = 0;
		}
	}
}

/*!
************************************************************************
* \brief
*    Read motion info
************************************************************************
*/
CREL_RETURN readMotionInfoFromNAL PARGS0()
{
	// We are only calling this function with mb_type = 1/2/3/4
	int k;
	int mb_nr = IMGPAR current_mb_nr_r;
	int bframe          = (IMGPAR type==B_SLICE);
	//int partmode        = (IS_P8x8(currMB_r)?4:currMB_r->mb_type);
	int partmode        = currMB_r->mb_type;

	int flag_mode;
	int list_offset = ((IMGPAR MbaffFrameFlag)&&(currMB_s_r->mb_field))? (mb_nr&1) ? 4 : 2 : 0;

	Pred_s_info *info = &currMB_s_r->pred_info;
	CREL_RETURN ret;

	IMGPAR do_co_located = 0;
	/*
	if (( img->number == 620) && (img->current_mb_nr_d == 2 )) {
	img->current_mb_nr_d = img->current_mb_nr_d;
	}
	*/
	if (bframe && IS_P8x8 (currMB_r))
	{
		if (IMGPAR direct_spatial_mv_pred_flag) {
			ret = IMGPAR FP_ReadMV_Direct_Spatial ARGS3(IMGPAR current_mb_nr_r, currMB_r, currMB_s_r);			
		} else {
			ret = IMGPAR FP_ReadMV_Direct_Temproal ARGS3(IMGPAR current_mb_nr_r, currMB_r, currMB_s_r);
		}
		if (FAILED(ret)) {
			return ret;
		}
	}

	//  If multiple ref. frames, read reference frame for the MB *********************************
	if(IMGPAR num_ref_idx_l0_active>1 && (bframe || !IMGPAR allrefzero))
	{
		//flag_mode = ( IMGPAR num_ref_idx_l0_active == 2 ? 1 : 0);
		flag_mode = 1+(( 2 - IMGPAR num_ref_idx_l0_active )>>31);
		ret = readRefInfo ARGS3(flag_mode, 0, info);		
		if (FAILED(ret)) {
			return ret;
		}
	}
	else
	{
		for(k=0;k<4;k++)
		{
			if ((currMB_r->b8pdir[k]==0 || currMB_r->b8pdir[k]==2) && currMB_r->b8mode[k]!=0)
			{
				info->ref_idx[0][k] = 0;
			}
		}
	}
	//Just for Debugging
	/*
	for ( k = 0; k < 4; k++ ) {
	if ( info->ref_idx[0][k] >= 0 ) {
	//if (info->ref_idx[0][k] > img->stream_global->m_listXsize[list_offset]) {
	if (info->ref_idx[0][k] >= img->m_listXsize[list_offset]) {
	info->ref_idx[0][k] = info->ref_idx[0][k];
	}
	}
	}
	*/

	//  If backward multiple ref. frames, read backward reference frame for the MB *********************************
	if(IMGPAR num_ref_idx_l1_active>1)
	{
		//flag_mode = ( IMGPAR num_ref_idx_l1_active == 2 ? 1 : 0);
		flag_mode = 1+(( 2 - IMGPAR num_ref_idx_l1_active )>>31);
		ret = readRefInfo ARGS3(flag_mode, 1, info);		
		if (FAILED(ret)) {
			return ret;
		}
	}
	else
	{
		for(k=0;k<4;k++)
		{
			if ((currMB_r->b8pdir[k]==1 || currMB_r->b8pdir[k]==2) && currMB_r->b8mode[k]!=0)
			{
				info->ref_idx[1][k] = 0;
			}
		}
	}

	/*
	for ( k = 0; k < 4; k++ ) {
	if ( info->ref_idx[1][k] > 0 ) {
	//if (info->ref_idx[1][k] >= img->stream_global->m_listXsize[list_offset+1]) {
	if (info->ref_idx[1][k] >= img->m_listXsize[list_offset+1]) {
	info->ref_idx[1][k] = info->ref_idx[1][k];
	}
	}
	}
	*/
	//=====  READ FORWARD MOTION VECTORS =====
	if(currMB_r->mb_type == PB_16x16)
	{
		if (currMB_r->b8pdir[0]==0 || currMB_r->b8pdir[0]==2)//has forward vector
			IMGPAR FP_ReadMV_16x16 ARGS2(info, 0);
		else
			memset(&IMGPAR curr_mvd[0][0],0,16*sizeof(IMGPAR curr_mvd[0][0]));

		if (currMB_r->b8pdir[0]==1 || currMB_r->b8pdir[0]==2)//has backward vector
			IMGPAR FP_ReadMV_16x16 ARGS2(info, 1);
		else
			memset(&IMGPAR curr_mvd[1][0],0,16*sizeof(IMGPAR curr_mvd[0][0]));
	}
	else if(currMB_r->mb_type == PB_16x8)
	{
		IMGPAR FP_ReadMV_16x8 ARGS2(info, 0);
		IMGPAR FP_ReadMV_16x8 ARGS2(info, 1);
	}
	else if(currMB_r->mb_type == PB_8x16)
	{
		IMGPAR FP_ReadMV_8x16 ARGS2(info, 0);
		IMGPAR FP_ReadMV_8x16  ARGS2(info, 1);
	}
	else
	{
		IMGPAR FP_ReadMV_8x8 ARGS2(info, 0);
		IMGPAR FP_ReadMV_8x8  ARGS2(info, 1);
	}

	memcpy(currMB_r->upmvd[0], &IMGPAR curr_mvd[0][12], 4*4);
	memcpy(currMB_r->upmvd[1], &IMGPAR curr_mvd[1][12], 4*4);

	IMGPAR left_mvd[mb_nr&IMGPAR mvd_pairs_mask][0][0].mv_comb = IMGPAR curr_mvd[0][3].mv_comb;
	IMGPAR left_mvd[mb_nr&IMGPAR mvd_pairs_mask][0][1].mv_comb = IMGPAR curr_mvd[0][7].mv_comb;
	IMGPAR left_mvd[mb_nr&IMGPAR mvd_pairs_mask][0][2].mv_comb = IMGPAR curr_mvd[0][11].mv_comb;
	IMGPAR left_mvd[mb_nr&IMGPAR mvd_pairs_mask][0][3].mv_comb = IMGPAR curr_mvd[0][15].mv_comb;

	IMGPAR left_mvd[mb_nr&IMGPAR mvd_pairs_mask][1][0].mv_comb = IMGPAR curr_mvd[1][3].mv_comb;
	IMGPAR left_mvd[mb_nr&IMGPAR mvd_pairs_mask][1][1].mv_comb = IMGPAR curr_mvd[1][7].mv_comb;
	IMGPAR left_mvd[mb_nr&IMGPAR mvd_pairs_mask][1][2].mv_comb = IMGPAR curr_mvd[1][11].mv_comb;
	IMGPAR left_mvd[mb_nr&IMGPAR mvd_pairs_mask][1][3].mv_comb = IMGPAR curr_mvd[1][15].mv_comb;


	// record reference picture Ids for deblocking decisions
#if !defined(_COLLECT_PIC_)
	record_reference_picIds ARGS2(list_offset, currMB_s_r);
#else
	currMB_s_r->do_record = 1;
	//record_reference_picIds ARGS2(list_offset, currMB_s_r);
#endif

	return CREL_OK;
}

/*!
************************************************************************
* \brief
*    Get the Prediction from the Neighboring Blocks for Number of Nonzero Coefficients 
*    
*    Luma Blocks
************************************************************************
*/
int predict_nnz PARGS2(int i,int j)
{
	PixelPos pix_a, pix_b;
	PixelPos *p_pix_a = &pix_a;

	int pred_nnz = 0;
	int mb_nr    = IMGPAR current_mb_nr_r;
	int block_available;

	// left block
	if(i)
	{
		pix_a.mb_addr = mb_nr;
		pix_a.x = i-1;
		pix_a.y = j;
		pix_a.pMB = currMB_r;
	}
	else
	{
		p_pix_a = &(IMGPAR left[j]);
	}

	if (p_pix_a->pMB)
	{
		block_available = 1;
		if ( active_pps.constrained_intra_pred_flag && (IMGPAR currentSlice->dp_mode==PAR_DP_3))
			block_available &= p_pix_a->pMB->intra_block;

		if (block_available)
			pred_nnz = IMGPAR nz_coeff[p_pix_a->mb_addr].nz_coeff_num[(p_pix_a->y<<2) + p_pix_a->x];
	}

	// top block
	if(j)
	{
		pix_b.mb_addr = mb_nr;
		pix_b.x = i;
		pix_b.y = j-1;
		pix_b.pMB = currMB_r;
	}
	else
	{
		pix_b.mb_addr = IMGPAR mbAddrB;
		pix_b.x = i;
		pix_b.y = 3;
		pix_b.pMB = IMGPAR pUpMB_r;
	}

	if (pix_b.pMB)
	{
		block_available = 1;
		if( active_pps.constrained_intra_pred_flag && (IMGPAR currentSlice->dp_mode==PAR_DP_3))
			block_available &= pix_b.pMB->intra_block;

		if (block_available)
		{
			pred_nnz += IMGPAR nz_coeff[pix_b.mb_addr].nz_coeff_num[(pix_b.y<<2) + pix_b.x];
			if (p_pix_a->pMB)
				pred_nnz = (pred_nnz+1)>>1; 
		}
	}

	return pred_nnz;
}


/*!
************************************************************************
* \brief
*    Get the Prediction from the Neighboring Blocks for Number of Nonzero Coefficients 
*    
*    Chroma Blocks   
************************************************************************
*/
int predict_nnz_chroma PARGS2(int i, int j)
{
	PixelPos pix;

	int pred_nnz = 0;
	int cnt      = 0;
	int mb_nr    = IMGPAR current_mb_nr_r;
	int block_available;

	//YUV420 - HP restriction
	// left block
	if (i&1)
	{
		pix.mb_addr = mb_nr;
		pix.x = (i&1)-1;
		pix.y = i>>1;
		pix.pMB = currMB_r;
	}
	else
	{
		memcpy(&pix, &(IMGPAR left[i]), sizeof(PixelPos));
		pix.x >>= 1;
		pix.y >>= 1;
	}

	if (pix.pMB)
	{
		block_available = 1;
		if (active_pps.constrained_intra_pred_flag && (IMGPAR currentSlice->dp_mode==PAR_DP_3))
			block_available &= pix.pMB->intra_block;

		if (block_available)
		{
			pred_nnz = IMGPAR nz_coeff[pix.mb_addr].nz_coeff_num[(j<<2) + (pix.y<<1) + pix.x]; // HP restriction
			cnt = 1;
		}
	}

	// top block
	if (i>>1)
	{
		pix.mb_addr = mb_nr;
		pix.x = i&1;
		pix.y = (i>>1)-1;
		pix.pMB = currMB_r;
	}
	else
	{
		pix.mb_addr = IMGPAR mbAddrB;
		pix.x = ((i&1)<<1)>>1;
		pix.y = 3>>1;
		pix.pMB = IMGPAR pUpMB_r;
	}

	if (pix.pMB)
	{
		block_available = 1;
		if (active_pps.constrained_intra_pred_flag && (IMGPAR currentSlice->dp_mode==PAR_DP_3))
			block_available &= pix.pMB->intra_block;

		if (block_available)
		{
			pred_nnz += IMGPAR nz_coeff[pix.mb_addr].nz_coeff_num[(j<<2) + (pix.y<<1) + pix.x]; // HP restriction
			if (cnt)
				pred_nnz = (pred_nnz + 1)>>1;
		}
	}

	return pred_nnz;
}


/*!
************************************************************************
* \brief
*    Reads coeff of an 4x4 block (CAVLC)
*
* \author
*    Karl Lillevold <karll@real.com>
*    contributions by James Au <james@ubvideo.com>
************************************************************************
*/

typedef int (*FP_predict_nnz_t) PARGS2(int i, int j);
static FP_predict_nnz_t FP_predict_nnz[2] = {&predict_nnz, &predict_nnz_chroma};

void readCoeff4x4_CAVLC PARGS6(int block_type, 
															 int i, 
															 int j,
															 short levarr[16],
															 short runarr[16],
															 int *number_coefficients)
{
	int mb_nr = IMGPAR current_mb_nr_r;
	int k, code, vlcnum;
	int numcoeff, numtrailingones;
	int totzeros, level;
	int max_coeff_num, nnz;
	static int incVlc[7] = {0,3,6,12,24,48,32768};
	int IsCDC = (block_type == CHROMA_DC_CAVLC) ? 1:0;
	int IsCAC = (block_type == CHROMA_AC_CAVLC) ? 1:0;
	static int mapTable[5][3] = {{SE_LUM_AC_INTER, SE_LUM_AC_INTRA, 16},
	{SE_LUM_DC_INTRA, SE_LUM_DC_INTRA, 16},
	{SE_LUM_AC_INTRA, SE_LUM_AC_INTRA, 15},
	{SE_CHR_DC_INTER, SE_CHR_DC_INTRA,  4},
	{SE_CHR_AC_INTER, SE_CHR_AC_INTRA, 15}};
	static int numcoeffvlc_tab[17] = {0, 0, 1, 1, 2, 2, 2, 2,
		3, 3, 3, 3, 3, 3, 3, 3, 3};

	max_coeff_num = mapTable[block_type][2];

	if (IsCDC)
	{
		// chroma DC
		numcoeff = readSyntaxElement_NumCoeffTrailingOnesChromaDC ARGS0();

		numtrailingones = numcoeff>>5;
		numcoeff       &= 0x1F;
		if (numcoeff > max_coeff_num) {
			numcoeff = max_coeff_num;		//Error Found
		}

		IMGPAR nz_coeff[mb_nr].nz_coeff_num[(j<<2) + i] = 0;
	}
	else
	{
		// luma or chroma AC
		nnz = FP_predict_nnz[IsCAC] ARGS2(i, j);

		numcoeff = readSyntaxElement_NumCoeffTrailingOnes ARGS1(numcoeffvlc_tab[nnz]);

		numtrailingones = numcoeff>>5;
		numcoeff       &= 0x1F;

		if (numcoeff > max_coeff_num) {
			numcoeff = max_coeff_num;		//Error Found
		}

		IMGPAR nz_coeff[mb_nr].nz_coeff_num[(j<<2) + i] = numcoeff;
	}

	//memset(levarr, 0, numcoeff<<1);
	//memset(runarr, 0, numcoeff<<1);

	*number_coefficients = numcoeff;

	if (numcoeff)
	{
		if (numtrailingones)
		{

			code = readSyntaxElement_FLC ARGS1(numtrailingones);

#if 0
			ntr = numtrailingones;
			for (k = numcoeff-1; k > numcoeff-1-numtrailingones; k--)
			{
				ntr --;
				levarr[k] = 1-(((code>>ntr)&1)<<1);
			}
#else
			code <<= 1;
			for (k = numcoeff-numtrailingones; k < numcoeff; k++)
			{
				levarr[k] = 1-(code&2);
				code >>= 1;
			}
#endif
		}

		// decode levels
		if (numcoeff > 10 && numtrailingones < 3)
			vlcnum = 1;
		else
			vlcnum = 0;

		k = numcoeff - 1 - numtrailingones;
		if(k>=0)
		{
			// Can only be called with vlcnum = 1 or vlcnum = 0
			level = readSyntaxElement_Level_VLC ARGS1(vlcnum);

			if (numcoeff <= 3 || numtrailingones != 3)
			{
				if (level > 0)
					level ++;
				else
					level --;
			}

			levarr[k] = level;

			// update VLC table
			if (abs(level)>incVlc[vlcnum])
				vlcnum++;

			if (abs(level)>3)
				vlcnum = 2;
		}

		for (k--; k >= 0; k--)
		{
			level = readSyntaxElement_Level_VLC ARGS1(vlcnum);

			levarr[k] = level;

			// update VLC table
			// vlcnum is constrained to be in the range [0..6]
			if (abs(level)>incVlc[vlcnum])
				vlcnum++;
		}

		if (numcoeff < max_coeff_num)
		{
			// decode total run
			if (IsCDC)
				totzeros = readSyntaxElement_TotalZerosChromaDC ARGS1(numcoeff-1);
			else
				totzeros = readSyntaxElement_TotalZeros ARGS1(numcoeff-1);

			// decode run before each coefficient
			for(i = numcoeff-1; totzeros != 0 && i != 0; i--)
			{
				// select VLC for runbefore
				vlcnum = RUNBEFORE_NUM + (((totzeros - RUNBEFORE_NUM)>>31)&(totzeros - RUNBEFORE_NUM)) - 1;

				runarr[i] = readSyntaxElement_Run ARGS1(vlcnum);

				totzeros -= runarr[i];
			}
			runarr[i] = totzeros;
		}
		else
		{
			i = numcoeff;
		}
		memset (runarr, 0, i<<1);
	} // if numcoeff
}

/*!
************************************************************************
* \brief
*    Calculate the quantisation and inverse quantisation parameters
*
************************************************************************
*/
void CalculateQuant8Param PARGS0()
{
	int i, j, k, temp;

	if((IMGPAR stream_global)->uiH264DXVAMode==E_H264_DXVA_MODE_C)
	{
		for(k=0; k<6; k++)
			for(j=0; j<8; j++)
			{
				for(i=0; i<8; i++)
				{
					temp = (j<<3)+i;

					InvLevelScale8x8Luma_Intra[k][j][i] = dequant_coef8[k][j][i]*qmatrix[6][temp];
					InvLevelScale8x8Luma_Inter[k][j][i] = dequant_coef8[k][j][i]*qmatrix[7][temp];
				}
			}
	}
	else
	{
		for(k=0; k<6; k++)
			for(j=0; j<8; j++)
			{
				for(i=0; i<8; i++)
				{
					temp = (j<<3)+i;
#ifdef _PRE_TRANSPOSE_
					InvLevelScale8x8Luma_Intra[k][i][j] = dequant_coef8[k][j][i]*qmatrix[6][temp];
					InvLevelScale8x8Luma_Inter[k][i][j] = dequant_coef8[k][j][i]*qmatrix[7][temp];
#else
					InvLevelScale8x8Luma_Intra[k][j][i] = dequant_coef8[k][j][i]*qmatrix[6][temp];
					InvLevelScale8x8Luma_Inter[k][j][i] = dequant_coef8[k][j][i]*qmatrix[7][temp];
#endif
				}
			}
	}
}

//extern int coeff_ctr;
//extern unsigned char coeff_pos[64];
//extern short coeff_level[64];

/*!
************************************************************************
* \brief
*    Get coefficients (run/level) of one 8x8 block  
*    from the NAL (CABAC Mode)
************************************************************************
*/
void readLumaCoeff8x8_CABAC PARGS0()
{
	int b8;
	int cbp = currMB_r->cbp;

	int dq_lshift, dq_rshift, dq_round;

	int qp_per;
	int qp_rem;

	qp_per    = divmod6[(currMB_s_r->qp - MIN_QP)<<1];
	qp_rem    = divmod6[((currMB_s_r->qp - MIN_QP)<<1)+1];

	IMGPAR is_intra_block = IS_INTRA(currMB_r);

	int frame_scan = ((IMGPAR structure == FRAME) && (!currMB_s_r->mb_field));
	//const byte *scan_ptr = &SNGL_SCAN8x8_2D[frame_scan][0];
	const byte *scan_ptr;
//#if defined(CONFIG_H264_C_ONLY)
	
//#else
	if((cpu_type == CPU_LEVEL_MMX) || (cpu_type == CPU_LEVEL_SSE)||(cpu_type == CPU_LEVEL_SSE2) || (cpu_type == CPU_LEVEL_SSE3))
	{
		if((IMGPAR stream_global)->uiH264DXVAMode==E_H264_DXVA_MODE_C) //For Intel mode C, pre-transpose is unnecessary
			scan_ptr = &SNGL_SCAN8x8_2D_Intel[frame_scan][0];
		else
			scan_ptr = &SNGL_SCAN8x8_2D[frame_scan][0];
	}
	else
		scan_ptr = &SNGL_SCAN8x8_2D[frame_scan][0];	
//#endif

	short *Inv_table;
	short *img_cof_ptr;

	if (IMGPAR is_intra_block)
	{
		Inv_table = &InvLevelScale8x8Luma_Intra[qp_rem][0][0];
	}
	else
	{
		Inv_table = &InvLevelScale8x8Luma_Inter[qp_rem][0][0];
	}

	if (qp_per < 6)
	{
		dq_rshift = 6 - qp_per;
		dq_round  = 1<<(5-qp_per);
		for(b8=0;b8<4;b8++)
		{
			if (cbp & (1<<b8))  // are there any coeff in current block at all
			{
				IMGPAR subblock_x = (b8&1)<<1; // position for coeff_count ctx
				IMGPAR subblock_y = (b8&2);    // position for coeff_count ctx

				if(read_and_store_CBP_block_bit_LUMA_8x8 ARGS0())
				{
#if 0
					int coeff_ctr = read_significance_map_coefficients ARGS0();

					currMB_s_r->cbp_blk |= 51 << ((b8+(b8&2))<<1); // corresponds to 110011, as if all four 4x4 blocks

#if defined(ONE_COF)
					img_cof_ptr = &IMGPAR cof[b8][0][0][0];
#else
					img_cof_ptr = (IMGPAR cof_r + (b8<<6));
#endif
					for(k=0;k<coeff_ctr;k++)
					{
						i=scan_ptr[coeff_pos[k]];
						img_cof_ptr[i] = (coeff_level[k]*Inv_table[i]+dq_round)>>dq_rshift; // dequantization
					}
#else
					currMB_s_r->cbp_blk |= 51 << ((b8+(b8&2))<<1); // corresponds to 110011, as if all four 4x4 blocks

#if defined(ONE_COF)
					img_cof_ptr = &IMGPAR cof[b8][0][0][0];
#else					
					img_cof_ptr = (IMGPAR cof_r + (b8<<6));
#endif
					read_significance_map_coefficients_qp_s_4 ARGS5(scan_ptr, Inv_table, img_cof_ptr, dq_rshift, dq_round);
#endif
				}
			}
		} // for(b8=0;b8<4;b8++)
	}
	else
	{ // qp_rem>=6
		dq_lshift = qp_per - 6;
		for(b8=0;b8<4;b8++)
		{
			if (cbp & (1<<b8))  // are there any coeff in current block at all
			{
				IMGPAR subblock_x = (b8&1)<<1; // position for coeff_count ctx
				IMGPAR subblock_y = (b8&2);    // position for coeff_count ctx

				if(read_and_store_CBP_block_bit_LUMA_8x8 ARGS0())
				{
#if 0
					int coeff_ctr = read_significance_map_coefficients ARGS0();

					currMB_s_r->cbp_blk |= 51 << ((b8+(b8&2))<<1); // corresponds to 110011, as if all four 4x4 blocks

#if defined(ONE_COF)
					img_cof_ptr = &IMGPAR cof[b8][0][0][0];
#else
					img_cof_ptr = (IMGPAR cof_r + (b8<<6));
#endif
					for(k=0;k<coeff_ctr;k++)
					{
						i=scan_ptr[coeff_pos[k]];
						img_cof_ptr[i] = (coeff_level[k]*Inv_table[i])<<dq_lshift; // dequantization
					}
#else
					currMB_s_r->cbp_blk |= 51 << ((b8+(b8&2))<<1); // corresponds to 110011, as if all four 4x4 blocks

#if defined(ONE_COF)
					img_cof_ptr = &IMGPAR cof[b8][0][0][0];
#else					
					img_cof_ptr = (IMGPAR cof_r + (b8<<6));
#endif
					read_significance_map_coefficients_qp_l_4 ARGS4(scan_ptr, Inv_table, img_cof_ptr, dq_lshift);	
#endif
				}
			}
		} // for(b8=0;b8<4;b8++)
	}
}

/*!
************************************************************************
* \brief
*    Get coded block pattern and coefficients (run/level)
*    from the NAL
************************************************************************
*/
void readCBPandCoeffsFromNAL_UVLC PARGS0()
{
	int i,j,k,l;
	int level;
	int cbp;

	int coef_ctr, i0, j0, b8;

	int block_x,block_y;
	int start_scan;
	__declspec(align(16)) short levarr[32] GCC_ALIGN(16), runarr[32] GCC_ALIGN(16);
	int numcoeff;

	int qp_const;
	int qp_per; // HP restriction
	int qp_rem;

	int uv;
	int qp_uv[2];
	int qp_const_uv[2];
	int qp_per_uv[2];
	int qp_rem_uv[2];
	int qp_c[2];

	int intra     = IS_INTRA (currMB_r);
	int new_intra = IS_NEWINTRA (currMB_r);
	int temp[4];

	int b4;
	int yuv = dec_picture->chroma_format_idc;
	int cofuv[4];

	int need_transform_size_flag;


	int frame_scan = (IMGPAR structure == FRAME) && (!currMB_r->mb_field);

	int delta_quant;
	short *pCof;

	IMGPAR is_intra_block = intra; //running flag for current MB

	// read CBP if not new intra mode
	if (!new_intra)
	{
		//=====   C B P   =====
		//---------------------
		currMB_r->cbp = cbp = read_raw_cbp_uvlc ARGS0();

		//============= Transform size flag for INTER MBs =============
		//-------------------------------------------------------------
		need_transform_size_flag = (((currMB_r->mb_type >= 1 && currMB_r->mb_type <= 3)||
			(IS_DIRECT(currMB_r) && active_sps.direct_8x8_inference_flag) ||
			(currMB_r->NoMbPartLessThan8x8Flag))                                
			&& currMB_r->mb_type != I8MB && currMB_r->mb_type != I4MB
			&& (currMB_r->cbp&15)
			&& IMGPAR Transform8x8Mode);

		if (need_transform_size_flag)
		{
			currMB_r->luma_transform_size_8x8_flag = currMB_s_r->luma_transform_size_8x8_flag = 
				read_transform_size_uvlc ARGS0(); 
		}

		//=====   DQUANT   =====
		//----------------------
		// Delta quant only if nonzero coeffs
		if (cbp !=0)
		{
			delta_quant = read_raw_dquant_uvlc ARGS0();	
			//if ((delta_quant < -(26 + IMGPAR bitdepth_luma_qp_scale/2)) || (delta_quant > (25 + IMGPAR bitdepth_luma_qp_scale/2)))
			//	DEBUG_SHOW_ERROR_INFO ("[ERROR]mb_qp_delta is out of range");
			IMGPAR qp = currMB_s_r->qp = ((currMB_s_r->qp + delta_quant + 52 )%(52));
		}
		start_scan = 0;
	}
	else  //
	{
		cbp = currMB_r->cbp;
		delta_quant = read_raw_dquant_uvlc ARGS0();
		//if ((delta_quant < -(26 + IMGPAR bitdepth_luma_qp_scale/2)) || (delta_quant > (25 + IMGPAR bitdepth_luma_qp_scale/2)))
		//	error ("mb_qp_delta is out of range", 500);
		IMGPAR qp = currMB_s_r->qp = ((currMB_s_r->qp + delta_quant + 52 )%(52));

		memset(&currMB_r->ipredmode[0], DC_PRED, 16);

		readCoeff4x4_CAVLC ARGS6(LUMA_INTRA16x16DC, 0, 0,
			levarr, runarr, &numcoeff);

		coef_ctr=-1;
		level = 1;                            // just to get inside the loop
#if defined(ONE_COF)
		pCof = &IMGPAR cof[0][0][0][0];
#else
		pCof = IMGPAR cof_r;
#endif
		for(k = 0; k < numcoeff; k++)
		{
			if (levarr[k] != 0)                     // leave if len=1
			{
				coef_ctr += runarr[k]+1;
				i0=SNGL_SCAN_2Dx16[frame_scan][coef_ctr];
				pCof[i0]=levarr[k];// add new intra DC coeff
			}
		}
		if(numcoeff != 0){
			// HP restriction
			if ((IMGPAR stream_global)->uiH264DXVAMode!=E_H264_DXVA_MODE_C)
				itrans_2 ARGS0();// transform new intra DC
			currMB_s_r->cbp_blk |= 0xFFFF;
		}
		start_scan = 1;
	}

	qp_per    = divmod6[(currMB_s_r->qp - MIN_QP)<<1];
	qp_rem    = divmod6[((currMB_s_r->qp - MIN_QP)<<1)+1];

	qp_const  = 1<<(3-qp_per);

	//currMB_s_r->qp = IMGPAR qp;

	// luma coefficients
	static const byte x_idx[4]={0,2,0,2};
	static const byte y_idx[4]={0,0,2,2};
	static const byte i_idx[4]={0,1,0,1};
	static const byte j_idx[4]={0,0,1,1};

	static const byte peano_raster_2Dx16[16] = {0*16, 1*16, 4*16, 5*16,
		2*16, 3*16, 6*16, 7*16,
		8*16, 9*16,12*16,13*16,
		10*16,11*16,14*16,15*16 };
	if(currMB_r->luma_transform_size_8x8_flag)
	{
		short *Inv_table;
		if(intra)
		{
			Inv_table = &InvLevelScale8x8Luma_Intra[qp_rem][0][0];
		}
		else
		{
			Inv_table = &InvLevelScale8x8Luma_Inter[qp_rem][0][0];
		}
		int cnt;
		//const byte *scan_ptr = &SNGL_SCAN8x8_2D[frame_scan][0];
		const byte *scan_ptr;
//#if defined(CONFIG_H264_C_ONLY)
		if((cpu_type == CPU_LEVEL_MMX) || (cpu_type == CPU_LEVEL_SSE)||(cpu_type == CPU_LEVEL_SSE2) || (cpu_type == CPU_LEVEL_SSE3))
		{
			if((IMGPAR stream_global)->uiH264DXVAMode==E_H264_DXVA_MODE_C) //For Intel mode C, pre-transpose is unnecessary
			scan_ptr = &SNGL_SCAN8x8_2D_Intel[frame_scan][0];
		else
			scan_ptr = &SNGL_SCAN8x8_2D[frame_scan][0];
		}
		else
			scan_ptr = &SNGL_SCAN8x8_2D[frame_scan][0];		
		
//#endif

		for (cnt=0, b8=0 ; b8<4 ; b8++) /* all modes */
		{
			block_y = y_idx[b8];//((b8>>1)<<1);
			block_x = x_idx[b8];//((b8&1)<<1);
#if defined(ONE_COF)
			short *img_cof_ptr = &IMGPAR cof[b8][0][0][0];
#else
			short *img_cof_ptr = (IMGPAR cof_r + (b8<<6));
#endif
			if (cbp & (1<<b8))  /* are there any coeff in current block at all */
			{
				int dq_rshift, dq_round, dq_lshift;
				for(l=0 ; l<4 ; l++, cnt++)
				{
					j = peano_raster[cnt][0];
					i = peano_raster[cnt][1];

					readCoeff4x4_CAVLC ARGS6((start_scan<<1), i, j, 
						levarr, runarr, &numcoeff);

					if (numcoeff)
					{
						coef_ctr = start_scan-1;
						int b4, iz;
						b4 = 2*(j-block_y)+(i-block_x);
						currMB_s_r->cbp_blk  |= 51 << ((block_y<<2) + block_x);
						if (qp_per>=6) // HP restriction
						{
							dq_lshift = qp_per - 6;
							for (k = 0; k < numcoeff; k++)
							{
								coef_ctr += runarr[k]+1;
								// new inverse quant for 8x8 transform
								// do same as CABAC for deblocking: any coeff in the 8x8 marks all the 4x4s
								//as containing coefficients
								iz=scan_ptr[coef_ctr*4+b4];
								img_cof_ptr[iz] = levarr[k]*Inv_table[iz]<<dq_lshift; // dequantization
							}
						}
						else //qp_per<6
						{
							dq_rshift = 6 - qp_per;
							dq_round  = 1<<(5-qp_per);
							for (k = 0; k < numcoeff; k++)
							{
								coef_ctr += runarr[k]+1;
								// new inverse quant for 8x8 transform
								// do same as CABAC for deblocking: any coeff in the 8x8 marks all the 4x4s
								//as containing coefficients
								iz=scan_ptr[coef_ctr*4+b4];
								img_cof_ptr[iz] = (levarr[k]*Inv_table[iz]+dq_round)>>dq_rshift; // dequantization
							}
						}
					}
				}
			}
			else
			{
				for(l=0 ; l<4 ; l++, cnt++)
				{
					j = peano_raster[cnt][0];
					i = peano_raster[cnt][1];
					IMGPAR nz_coeff[IMGPAR current_mb_nr_r].nz_coeff_num[(j<<2) + i] = 0;
				}
			}
		}
	}
	else
	{
		short *Inv_table;
		if(intra)
		{
			Inv_table = &InvLevelScale4x4Luma_Intra[qp_rem][0];
		}
		else
		{
			Inv_table = &InvLevelScale4x4Luma_Inter[qp_rem][0];
		}
		int cnt;

		//const byte *scan_ptr = &SNGL_SCAN_2D[frame_scan][0];
		const byte *scan_ptr;
//#if defined(CONFIG_H264_C_ONLY)
		
//#else
		if((cpu_type == CPU_LEVEL_MMX) || (cpu_type == CPU_LEVEL_SSE)||(cpu_type == CPU_LEVEL_SSE2) || (cpu_type == CPU_LEVEL_SSE3))
		{
			if((IMGPAR stream_global)->uiH264DXVAMode==E_H264_DXVA_MODE_C) //For Intel mode C, pre-transpose is unnecessary
				scan_ptr = &SNGL_SCAN_2D_Intel[frame_scan][0];
			else
				scan_ptr = &SNGL_SCAN_2D[frame_scan][0];
		}
		else
			scan_ptr = &SNGL_SCAN_2D[frame_scan][0];
//#endif

		for (cnt=0, b8=0 ; b8<4 ; b8++) /* all modes */
		{
			for(l=0 ; l<4 ; l++, cnt++)
			{
				j = peano_raster[cnt][0];
				i = peano_raster[cnt][1];

				if (cbp & (1<<b8))  /* are there any coeff in current block at all */
				{
					readCoeff4x4_CAVLC ARGS6((start_scan<<1), i, j, 
						levarr, runarr, &numcoeff);

					if (numcoeff)
					{
						coef_ctr = start_scan-1;
#if defined(ONE_COF)
						short *img_cof_ptr = &IMGPAR cof[j][i][0][0];
#else
						short *img_cof_ptr = (IMGPAR cof_r + ( (j<<6)+ (i<<4) ));
#endif
						currMB_s_r->cbp_blk    |= 1 << ((j<<2) + i);
						if(qp_per<4) // HP restriction
						{
							int qp_shift =4-qp_per;
							for (k = 0; k < numcoeff; k++)
							{
								coef_ctr += runarr[k]+1;								  								  
								j0 = scan_ptr[coef_ctr];								  								  							  
								img_cof_ptr[j0]= (levarr[k]*Inv_table[j0]+qp_const)>>qp_shift;//(4-qp_per);
							}
						}
						else
						{
							int qp_shift =qp_per-4;
							for (k = 0; k < numcoeff; k++)
							{
								coef_ctr += runarr[k]+1;
								j0 = scan_ptr[coef_ctr];
								img_cof_ptr[j0]= (levarr[k]*Inv_table[j0])<<qp_shift;//(qp_per-4);
							}
						}
					}
				}
				else
				{
					IMGPAR nz_coeff[IMGPAR current_mb_nr_r].nz_coeff_num[(j<<2) + i] = 0;
				}
			}

		}
	}
#ifdef __SUPPORT_YUV400__
	if (yuv != YUV400)
	{
#endif
		//Move to Upper part
		//qp_const_uv[0] = 1<<(3-qp_per_uv[0]);
		//qp_const_uv[1] = 1<<(3-qp_per_uv[1]);


		//========================== CHROMA DC ============================
		//-----------------------------------------------------------------
		// chroma DC coeff
		if(cbp>15)
		{
			for (uv=0;uv<2;uv++)
			{
				qp_uv[uv] = currMB_s_r->qp + dec_picture->chroma_qp_offset[uv];
				qp_uv[uv] = Clip3(0, 51, qp_uv[uv]); // HP restriction
				//    qp_c[uv]  = (qp_uv[uv] < 0)? qp_uv[uv] : QP_SCALE_CR[qp_uv[uv]-MIN_QP];
				qp_c[uv]  = (QP_SCALE_CR[qp_uv[uv]-MIN_QP]<<1);
				qp_per_uv[uv] = divmod6[qp_c[uv]]; // HP restriction
				qp_const_uv[uv] = 1<<(3-qp_per_uv[uv]);
				qp_rem_uv[uv] = divmod6[qp_c[uv]+1]; // HP restriction
				//===================== CHROMA DC YUV420 ======================
				memset(cofuv, 0, 4*sizeof(int));

				readCoeff4x4_CAVLC ARGS6(CHROMA_DC_CAVLC, 0, 0, 
					levarr, runarr, &numcoeff);
				if (numcoeff)
				{
					coef_ctr=-1;
					currMB_s_r->cbp_blk |= 0xf0000 << (uv<<2) ;
					for(k = 0; k < numcoeff; k++)
					{
						coef_ctr += runarr[k]+1;
						cofuv[coef_ctr]=levarr[k];
					}

					if ((IMGPAR stream_global)->uiH264DXVAMode!=E_H264_DXVA_MODE_C)
					{
						temp[0]=(cofuv[0]+cofuv[1]+cofuv[2]+cofuv[3]);
						temp[1]=(cofuv[0]-cofuv[1]+cofuv[2]-cofuv[3]);
						temp[2]=(cofuv[0]+cofuv[1]-cofuv[2]-cofuv[3]);
						temp[3]=(cofuv[0]-cofuv[1]-cofuv[2]+cofuv[3]);

						short *Inv_table;
						if(intra)
						{
							Inv_table = &InvLevelScale4x4Chroma_Intra[uv][qp_rem_uv[uv]][0];
						}
						else
						{
							Inv_table = &InvLevelScale4x4Chroma_Inter[uv][qp_rem_uv[uv]][0];
						}
						int qp_shift;
						if(qp_per_uv[uv]<5)
						{
							qp_shift = 5-qp_per_uv[uv];
#if defined(ONE_COF)
							IMGPAR cof[4+uv][0][0][0]=(temp[0]*Inv_table[0])>>qp_shift;
							IMGPAR cof[4+uv][1][0][0]=(temp[1]*Inv_table[0])>>qp_shift;
							IMGPAR cof[4+uv][2][0][0]=(temp[2]*Inv_table[0])>>qp_shift;
							IMGPAR cof[4+uv][3][0][0]=(temp[3]*Inv_table[0])>>qp_shift;
#else
							short *ptr = (IMGPAR cof_r + ((4+uv)<<6));
							*ptr = (temp[0]*Inv_table[0])>>qp_shift;
							ptr += 16;
							*ptr = (temp[1]*Inv_table[0])>>qp_shift;
							ptr += 16;
							*ptr = (temp[2]*Inv_table[0])>>qp_shift;
							ptr += 16;
							*ptr = (temp[3]*Inv_table[0])>>qp_shift;
#endif
						}
						else
						{
							qp_shift = qp_per_uv[uv]-5;
#if defined(ONE_COF)
							IMGPAR cof[4+uv][0][0][0]=(temp[0]*Inv_table[0])<<qp_shift;
							IMGPAR cof[4+uv][1][0][0]=(temp[1]*Inv_table[0])<<qp_shift;
							IMGPAR cof[4+uv][2][0][0]=(temp[2]*Inv_table[0])<<qp_shift;
							IMGPAR cof[4+uv][3][0][0]=(temp[3]*Inv_table[0])<<qp_shift;
#else
							short *ptr = (IMGPAR cof_r + ((4+uv)<<6));
							*ptr = (temp[0]*Inv_table[0])<<qp_shift;
							ptr += 16;
							*ptr = (temp[1]*Inv_table[0])<<qp_shift;
							ptr += 16;
							*ptr = (temp[2]*Inv_table[0])<<qp_shift;
							ptr += 16;
							*ptr = (temp[3]*Inv_table[0])<<qp_shift;
#endif
						}
					}
					else //DXVA Mode C
					{
						short *ptr = (IMGPAR cof_r + ((4+uv)<<6));
						*ptr = cofuv[0];
						ptr += 16;
						*ptr = cofuv[1];
						ptr += 16;
						*ptr = cofuv[2];
						ptr += 16;
						*ptr = cofuv[3];
					}
				}
			}//for (uv=0;uv<2;uv++)
		}

		// chroma AC coeff, all zero fram start_scan
		if (cbp<=31)
		{
			char *pChar = IMGPAR nz_coeff[IMGPAR current_mb_nr_r].nz_coeff_num;
			memset(pChar + 16, 0, 8*sizeof(char));
		}
		//========================== CHROMA AC ============================
		//-----------------------------------------------------------------
		// chroma AC coeff, all zero fram start_scan
		else// (cbp>31)
		{
			//const byte *scan_ptr = &SNGL_SCAN_2D[frame_scan][0];
			const byte *scan_ptr;
//#if  defined(CONFIG_H264_C_ONLY)

			
//#else
		if((cpu_type == CPU_LEVEL_MMX) || (cpu_type == CPU_LEVEL_SSE)||(cpu_type == CPU_LEVEL_SSE2) || (cpu_type == CPU_LEVEL_SSE3))
		{
			if((IMGPAR stream_global)->uiH264DXVAMode==E_H264_DXVA_MODE_C) //For Intel mode C, pre-transpose is unnecessary
				scan_ptr = &SNGL_SCAN_2D_Intel[frame_scan][0];
			else
				scan_ptr = &SNGL_SCAN_2D[frame_scan][0];
		}
		else
			scan_ptr = &SNGL_SCAN_2D[frame_scan][0];
//#endif

			short *Inv_table_2[2];
			if(intra)
			{
				Inv_table_2[0] = &InvLevelScale4x4Chroma_Intra[0][qp_rem_uv[0]][0];
				Inv_table_2[1] = &InvLevelScale4x4Chroma_Intra[1][qp_rem_uv[1]][0];
			}
			else
			{
				Inv_table_2[0] = &InvLevelScale4x4Chroma_Inter[0][qp_rem_uv[0]][0];
				Inv_table_2[1] = &InvLevelScale4x4Chroma_Inter[1][qp_rem_uv[1]][0];
			}
			for (b8=0; b8 < 2; b8++) // HP restriction
			{
				short *Inv_table = Inv_table_2[b8];
				for (b4 = 0; b4 < 4; b4++)
				{
					uv = 4+b8;

					readCoeff4x4_CAVLC ARGS6(CHROMA_AC_CAVLC, b4, uv,
						levarr, runarr, &numcoeff);
					if (numcoeff)
					{
#if defined(ONE_COF)
						pCof = &IMGPAR cof[uv][b4][0][0];
#else
						pCof = (IMGPAR cof_r + ((uv<<6)+ (b4<<4)));
#endif
						coef_ctr=0;
						level=1;
						currMB_s_r->cbp_blk |= 1 << cbp_blk_chroma[b8][b4];
						if(qp_per_uv[b8]<4) // HP restriction
						{
							int uv_qp_shift=4-qp_per_uv[b8];
							int uv_qp_const=qp_const_uv[b8];
							for(k = 0; k < numcoeff;k++)
							{
								coef_ctr += runarr[k]+1;
								j0=scan_ptr[coef_ctr];
								pCof[j0]=(levarr[k]*Inv_table[j0]+uv_qp_const)>>uv_qp_shift;//(4-qp_per_uv[b8]);
							}
						}
						else
						{
							int uv_qp_shift=qp_per_uv[b8]-4;
							for(k = 0; k < numcoeff;k++)
							{
								coef_ctr += runarr[k]+1;
								j0=scan_ptr[coef_ctr];
								pCof[j0]=(levarr[k]*Inv_table[j0])<<uv_qp_shift;//(qp_per_uv[b8]-4);
							}
						}
					}
				} //for (b4=0; b4 < 4; b4++)
			} //for (b8=0; b8 < 2; b8++) // HP restriction
		} //if (cbp>31)
#ifdef __SUPPORT_YUV400__
	} //if (dec_picture->chroma_format_idc != YUV400)
#endif
}


/*!
************************************************************************
* \brief
*    Get coded block pattern and coefficients (run/level)
*    from the NAL
************************************************************************
*/

void readCBPandCoeffsFromNAL_CABAC PARGS0()
{
	int i,j,k,l;
	int cbp;
	int context;

	int i0, b8;

	int start_scan;

	int qp_shift;
	int qp_const;
	int qp_per; // HP restriction
	int qp_rem;

	int qp_uv[2];
	int qp_const_uv[2];
	int qp_per_uv[2];
	int qp_rem_uv[2];
	int qp_c[2];

	int intra     = IS_INTRA (currMB_r);
	int new_intra = IS_NEWINTRA (currMB_r);
	int temp[4];

	int b4;
	int cofuv[4];

	int need_transform_size_flag;

	int frame_scan = (IMGPAR structure == FRAME) && (!currMB_r->mb_field);

	int delta_quant;
	short *pCof;	

	const byte *scan_ptr;
	const short *Inv_table;	

	IMGPAR is_intra_block = intra; //running flag for current MB

	// read CBP if not new intra mode
	if (!new_intra)
	{
		//=====   C B P   =====
		//---------------------
		currMB_r->cbp = cbp = readCBP_CABAC ARGS0();

		//============= Transform size flag for INTER MBs =============
		//-------------------------------------------------------------
		need_transform_size_flag = (((currMB_r->mb_type >= 1 && currMB_r->mb_type <= 3)||
			(IS_DIRECT(currMB_r) && active_sps.direct_8x8_inference_flag) ||
			(currMB_r->NoMbPartLessThan8x8Flag))                                
			&& currMB_r->mb_type != I8MB && currMB_r->mb_type != I4MB
			&& (currMB_r->cbp&15)
			&& IMGPAR Transform8x8Mode);

		if (need_transform_size_flag)
		{
			currMB_r->luma_transform_size_8x8_flag = currMB_s_r->luma_transform_size_8x8_flag = 
				readMB_transform_size_flag_CABAC ARGS0();
		}

		//=====   DQUANT   =====
		//----------------------
		// Delta quant only if nonzero coeffs
		if (cbp !=0)
		{
			delta_quant = readDquant_CABAC ARGS0();
			//if ((delta_quant < -(26 + IMGPAR bitdepth_luma_qp_scale/2)) || (delta_quant > (25 + IMGPAR bitdepth_luma_qp_scale/2)))
			//	DEBUG_SHOW_ERROR_INFO ("[ERROR]mb_qp_delta is out of range");
			IMGPAR qp = currMB_s_r->qp = ((currMB_s_r->qp + delta_quant + 52 )%(52));
		}
	}
	else  // new_intra
	{
		cbp = currMB_r->cbp;
		// read DC coeffs for new intra modes
		delta_quant = readDquant_CABAC ARGS0();
		//if ((delta_quant < -(26 + IMGPAR bitdepth_luma_qp_scale/2)) || (delta_quant > (25 + IMGPAR bitdepth_luma_qp_scale/2)))
		//	error ("mb_qp_delta is out of range", 500);
		IMGPAR qp = currMB_s_r->qp = ((currMB_s_r->qp + delta_quant + 52 )%(52));

		memset(&currMB_r->ipredmode[0], DC_PRED, 16);

		scan_ptr = &SNGL_SCAN_2Dx16[frame_scan][0];

		if(read_and_store_CBP_block_bit_LUMA_16DC ARGS0())	  
		{
			set_significance_map_ctx ARGS2(frame_scan, LUMA_16DC);
			int coeff_ctr = read_significance_map_coefficients ARGS0();
#if defined(ONE_COF)
			pCof = &IMGPAR cof[0][0][0][0];
#else
			pCof = IMGPAR cof_r;
#endif
			for(k=0;k<coeff_ctr;k++)
			{
				i0=scan_ptr[coeff_pos[k]];
				pCof[i0]=coeff_level[k];			
			}
			// HP restriction
			if ((IMGPAR stream_global)->uiH264DXVAMode!=E_H264_DXVA_MODE_C)
				itrans_2 ARGS0();// transform new intra DC
			currMB_s_r->cbp_blk |= 0xFFFF;
		}		
	}

	qp_per    = divmod6[(currMB_s_r->qp - MIN_QP)<<1];
	qp_rem    = divmod6[((currMB_s_r->qp - MIN_QP)<<1)+1];

	//currMB_s_r->qp = IMGPAR qp;

	// luma coefficients
	static const byte x_idx[4]={0,2,0,2};
	static const byte y_idx[4]={0,0,2,2};
	static const byte i_idx[4]={0,1,0,1};
	static const byte j_idx[4]={0,0,1,1};

	if(currMB_r->luma_transform_size_8x8_flag)
	{
		if (currMB_r->cbp&0xF)
		{
			set_significance_map_ctx ARGS2(frame_scan, LUMA_8x8);
			readLumaCoeff8x8_CABAC ARGS0();
		}
	}
	else
	{
		//scan_ptr = &SNGL_SCAN_2D[frame_scan][0];
//#if defined(CONFIG_H264_C_ONLY)
		
//#else
		if((cpu_type == CPU_LEVEL_MMX) || (cpu_type == CPU_LEVEL_SSE)||(cpu_type == CPU_LEVEL_SSE2) || (cpu_type == CPU_LEVEL_SSE3))
		{
			if((IMGPAR stream_global)->uiH264DXVAMode==E_H264_DXVA_MODE_C) //For Intel mode C, pre-transpose is unnecessary
				scan_ptr = &SNGL_SCAN_2D_Intel[frame_scan][0];
			else
				scan_ptr = &SNGL_SCAN_2D[frame_scan][0];
		}
		else
			scan_ptr = &SNGL_SCAN_2D[frame_scan][0];
//#endif
		if (new_intra)
		{
			context    = LUMA_16AC;
			start_scan = 1; // skip DC coeff
		}
		else
		{
			context	 = LUMA_4x4;
			start_scan = 0; // take all coeffs
		}

		if(intra)
		{
			Inv_table = &InvLevelScale4x4Luma_Intra[qp_rem][0];
		}
		else
		{
			Inv_table = &InvLevelScale4x4Luma_Inter[qp_rem][0];
		}

		set_significance_map_ctx ARGS2(frame_scan, context);

		if(qp_per<4) // HP restriction
		{
			qp_const  = 1<<(3-qp_per);
			qp_shift = (4-qp_per);
			for (b8=0 ; b8<4 ; b8++) /* all modes */
			{
				//block_y = ((b8>>1)<<1);
				//block_x = ((b8&1)<<1);		  
				if (cbp & (1<<b8))  // are there any coeff in current block at all
				{
					for (l=0 ; l<4 ; l++)
					{
						IMGPAR subblock_y = j = j_idx[l]+y_idx[b8];//(l>>1) + block_y;
						IMGPAR subblock_x = i = i_idx[l]+x_idx[b8];//(l&1) + block_x;

						if(fp_read_and_store_CBP_block_bit[context] ARGS0())
						{
#if 0
							int coeff_ctr = read_significance_map_coefficients ARGS0();

							currMB_s_r->cbp_blk      |= 1 << ((j<<2) + i) ;

#if defined(ONE_COF)
							pCof = &IMGPAR cof[j][i][0][0];
#else
							pCof = (IMGPAR cof_r + ((j<<6)+ (i<<4)));
#endif

							for(k=0;k<coeff_ctr;k++)
							{
								j0=scan_ptr[coeff_pos[k]];
								pCof[j0] = (coeff_level[k]*Inv_table[j0]+qp_const)>>qp_shift;
							}
#else			
							currMB_s_r->cbp_blk      |= 1 << ((j<<2) + i) ;				

#if defined(ONE_COF)
							pCof = &IMGPAR cof[j][i][0][0];
#else
							pCof = (IMGPAR cof_r + ((j<<6)+ (i<<4)));
#endif
							read_significance_map_coefficients_qp_s_4 ARGS5(scan_ptr, Inv_table, pCof, qp_shift, qp_const);
#endif													
						}					
					}
				}
			} //b8
		}
		else
		{
			qp_shift = (qp_per-4);
			for (b8=0 ; b8<4 ; b8++) /* all modes */
			{
				//block_y = ((b8>>1)<<1);
				//block_x = ((b8&1)<<1);		  
				if (cbp & (1<<b8))  // are there any coeff in current block at all
				{
					for (l=0 ; l<4 ; l++)
					{
						IMGPAR subblock_y = j = j_idx[l]+y_idx[b8];//(l>>1) + block_y;
						IMGPAR subblock_x = i = i_idx[l]+x_idx[b8];//(l&1) + block_x;

						if(fp_read_and_store_CBP_block_bit[context] ARGS0())
						{
#if 0
							int coeff_ctr = read_significance_map_coefficients ARGS0();

							currMB_s_r->cbp_blk      |= 1 << ((j<<2) + i) ;

#if defined(ONE_COF)
							pCof = &IMGPAR cof[j][i][0][0];
#else
							pCof = (IMGPAR cof_r + ((j<<6)+ (i<<4)));
#endif

							for(k=0;k<coeff_ctr;k++)
							{
								j0=scan_ptr[coeff_pos[k]];
								pCof[j0] = (coeff_level[k]*Inv_table[j0])<<qp_shift;
							}
#else				
							currMB_s_r->cbp_blk      |= 1 << ((j<<2) + i) ;					

#if defined(ONE_COF)
							pCof = &IMGPAR cof[j][i][0][0];
#else
							pCof = (IMGPAR cof_r + ((j<<6)+ (i<<4)));
#endif
							read_significance_map_coefficients_qp_l_4 ARGS4(scan_ptr, Inv_table, pCof, qp_shift);													
#endif							
						}
					}
				}
			} //b8
		}
	}

#ifdef __SUPPORT_YUV400__
	if (dec_picture->chroma_format_idc != YUV400)
	{
#endif
		if(cbp>15)
		{
			//========================== CHROMA DC ============================
			//-----------------------------------------------------------------
			// chroma DC coeff
			short *InvLevelScale4x4Chroma;
			for (b8=0;b8<2;b8++)
			{
				//init constants for every chroma qp offset
				qp_uv[b8] = currMB_s_r->qp + dec_picture->chroma_qp_offset[b8];
				qp_uv[b8] = Clip3(0, 51, qp_uv[b8]); // HP restriction
				//      qp_c[b8]  = (qp_uv[b8] < 0)? qp_uv[b8] : QP_SCALE_CR[qp_uv[b8]-MIN_QP];
				qp_c[b8]  = (QP_SCALE_CR[qp_uv[b8]-MIN_QP]<<1);
				qp_per_uv[b8] = divmod6[qp_c[b8]]; // HP restriction
				qp_const_uv[b8] = 1<<(3-qp_per_uv[b8]);
				qp_rem_uv[b8] = divmod6[qp_c[b8]+1]; // HP restriction
				//===================== CHROMA DC YUV420 ======================

				IMGPAR is_v_block     = b8;

				if(read_and_store_CBP_block_bit_CHROMA_DC ARGS0())
				{
					memset(cofuv, 0, 4*sizeof(int));
					set_significance_map_ctx ARGS2(frame_scan, CHROMA_DC);
					int coeff_ctr = read_significance_map_coefficients ARGS0();

					currMB_s_r->cbp_blk |= 0xf0000 << (b8<<2) ;

					for(k=0;k<coeff_ctr;k++)
					{
						cofuv[coeff_pos[k]]=coeff_level[k];
					}

					if ((IMGPAR stream_global)->uiH264DXVAMode!=E_H264_DXVA_MODE_C)
					{
						temp[0]=(cofuv[0]+cofuv[1]+cofuv[2]+cofuv[3]);
						temp[1]=(cofuv[0]-cofuv[1]+cofuv[2]-cofuv[3]);
						temp[2]=(cofuv[0]+cofuv[1]-cofuv[2]-cofuv[3]);
						temp[3]=(cofuv[0]-cofuv[1]-cofuv[2]+cofuv[3]);

						if(intra)
						{
							Inv_table = &InvLevelScale4x4Chroma_Intra[b8][qp_rem_uv[b8]][0];
						}
						else
						{
							Inv_table = &InvLevelScale4x4Chroma_Inter[b8][qp_rem_uv[b8]][0];
						}
						if(qp_per_uv[b8]<5)
						{
							qp_shift = 5-qp_per_uv[b8];
#if defined(ONE_COF)
							IMGPAR cof[4+b8][0][0][0]=(temp[0]*Inv_table[0])>>qp_shift;
							IMGPAR cof[4+b8][1][0][0]=(temp[1]*Inv_table[0])>>qp_shift;
							IMGPAR cof[4+b8][2][0][0]=(temp[2]*Inv_table[0])>>qp_shift;
							IMGPAR cof[4+b8][3][0][0]=(temp[3]*Inv_table[0])>>qp_shift;
#else
							short *ptr = (IMGPAR cof_r + ((4+b8)<<6));

							*ptr = (temp[0]*Inv_table[0])>>qp_shift;
							ptr += 16;
							*ptr = (temp[1]*Inv_table[0])>>qp_shift;
							ptr += 16;
							*ptr = (temp[2]*Inv_table[0])>>qp_shift;
							ptr += 16;
							*ptr = (temp[3]*Inv_table[0])>>qp_shift;
#endif
						}
						else
						{
							qp_shift = qp_per_uv[b8]-5;
#if defined(ONE_COF)
							IMGPAR cof[4+b8][0][0][0]=(temp[0]*Inv_table[0])<<qp_shift;
							IMGPAR cof[4+b8][1][0][0]=(temp[1]*Inv_table[0])<<qp_shift;
							IMGPAR cof[4+b8][2][0][0]=(temp[2]*Inv_table[0])<<qp_shift;
							IMGPAR cof[4+b8][3][0][0]=(temp[3]*Inv_table[0])<<qp_shift;
#else
							short *ptr = (IMGPAR cof_r + ((4+b8)<<6));

							*ptr = (temp[0]*Inv_table[0])<<qp_shift;
							ptr += 16;
							*ptr = (temp[1]*Inv_table[0])<<qp_shift;
							ptr += 16;
							*ptr = (temp[2]*Inv_table[0])<<qp_shift;
							ptr += 16;
							*ptr = (temp[3]*Inv_table[0])<<qp_shift;
#endif
						}
					}
					else //DXVA Mode C
					{
						short *ptr = (IMGPAR cof_r + ((4+b8)<<6));
						*ptr = cofuv[0];
						ptr += 16;
						*ptr = cofuv[1];
						ptr += 16;
						*ptr = cofuv[2];
						ptr += 16;
						*ptr = cofuv[3];
					}
				}
			}//for (b8=0;b8<2;b8++)

			//========================== CHROMA AC ============================
			//-----------------------------------------------------------------
			// chroma AC coeff, all zero fram start_scan
			if (cbp>31)
			{
				//scan_ptr = &SNGL_SCAN_2D[frame_scan][0];
//#if defined(CONFIG_H264_C_ONLY)
			
//#else
		if((cpu_type == CPU_LEVEL_MMX) || (cpu_type == CPU_LEVEL_SSE)||(cpu_type == CPU_LEVEL_SSE2) || (cpu_type == CPU_LEVEL_SSE3))
		{			
			if((IMGPAR stream_global)->uiH264DXVAMode==E_H264_DXVA_MODE_C) //For Intel mode C, pre-transpose is unnecessary
					scan_ptr = &SNGL_SCAN_2D_Intel[frame_scan][0];
			else
						scan_ptr = &SNGL_SCAN_2D[frame_scan][0];
		}
		else
			scan_ptr = &SNGL_SCAN_2D[frame_scan][0];	
//#endif

				set_significance_map_ctx ARGS2(frame_scan, CHROMA_AC);

				if(intra)
				{
					InvLevelScale4x4Chroma = &InvLevelScale4x4Chroma_Intra[0][0][0];
				}
				else
				{
					InvLevelScale4x4Chroma = &InvLevelScale4x4Chroma_Inter[0][0][0];
				}
				for (b8=0; b8 < 2; b8++) // HP restriction
				{
					Inv_table = &InvLevelScale4x4Chroma[(b8*6+qp_rem_uv[b8])<<4];
					IMGPAR is_v_block = b8;
					if(qp_per_uv[b8]<4) // HP restriction
					{
						qp_const = qp_const_uv[b8];
						qp_shift = (4-qp_per_uv[b8]);
						for (b4=0; b4 < 4; b4++)
						{	
							IMGPAR subblock_y = b4>>1;
							IMGPAR subblock_x = b4&1;
							if(read_and_store_CBP_block_bit_CHROMA_AC ARGS0())
							{
#if 0
								int coeff_ctr = read_significance_map_coefficients ARGS0();

								currMB_s_r->cbp_blk |= 1 << cbp_blk_chroma[b8][b4];

#if defined(ONE_COF)
								pCof = &IMGPAR cof[4+b8][b4][0][0];
#else
								pCof = (IMGPAR cof_r + (((4+b8)<<6) + (b4<<4)));
#endif

								for(k=0;k<coeff_ctr;k++)
								{
									j0=scan_ptr[coeff_pos[k]];
									pCof[j0]=(coeff_level[k]*Inv_table[j0]+qp_const)>>qp_shift;
								}
#else
								currMB_s_r->cbp_blk |= 1 << cbp_blk_chroma[b8][b4];								

#if defined(ONE_COF)
								pCof = &IMGPAR cof[4+b8][b4][0][0];
#else
								pCof = (IMGPAR cof_r + (((4+b8)<<6) + (b4<<4)));
#endif
								read_significance_map_coefficients_qp_s_4 ARGS5(scan_ptr, Inv_table, pCof, qp_shift, qp_const);	
#endif
							}
						} //for (b4=0; b4 < 4; b4++)
					}
					else
					{ // qp_per_uv[b8]>=4
						qp_shift = qp_per_uv[b8]-4;
						for (b4=0; b4 < 4; b4++)
						{
							IMGPAR subblock_y = b4>>1;
							IMGPAR subblock_x = b4&1;

							if(read_and_store_CBP_block_bit_CHROMA_AC ARGS0())
							{
#if 0
								int coeff_ctr = read_significance_map_coefficients ARGS0();

								currMB_s_r->cbp_blk |= 1 << cbp_blk_chroma[b8][b4];

#if defined(ONE_COF)
								pCof = &IMGPAR cof[4+b8][b4][0][0];
#else
								pCof = (IMGPAR cof_r + (((4+b8)<<6) + (b4<<4)));
#endif

								for(k=0;k<coeff_ctr;k++)
								{
									j0=scan_ptr[coeff_pos[k]];
									pCof[j0]=(coeff_level[k]*Inv_table[j0])<<qp_shift;
								}
#else
								currMB_s_r->cbp_blk |= 1 << cbp_blk_chroma[b8][b4];					
#if defined(ONE_COF)
								pCof = &IMGPAR cof[4+b8][b4][0][0];
#else
								pCof = (IMGPAR cof_r + (((4+b8)<<6) + (b4<<4)));
#endif
								read_significance_map_coefficients_qp_l_4 ARGS4(scan_ptr, Inv_table, pCof, qp_shift);						
#endif								
							}
						} //for (b4=0; b4 < 4; b4++)
					}
				} // for (b8)
			} //if (cbp>31)
		} //if (cbp>15)
#ifdef __SUPPORT_YUV400__
	} //if (dec_picture->chroma_format_idc != YUV400)
#endif
}


/*!
************************************************************************
* \brief
*    Copy IPCM coefficients to decoded picture buffer and set parameters for this MB
*    (for IPCM CABAC and IPCM CAVLC  28/11/2003)
*
* \author
*    Dong Wang <Dong.Wang@bristol.ac.uk>
************************************************************************
*/

void decode_ipcm_mb PARGS4(imgpel * imgY, int stride, imgpel * imgUV, int stride_UV)
{
	int i, j;

	//Copy coefficients to decoded picture buffer
	//IPCM coefficients are stored in IMGPAR cof which is set in function readIPCMcoeffsFromNAL()

#if defined(ONE_COF)
	unsigned char *ptr_cof = (unsigned char *) &IMGPAR cof[0][0][0][0];
#else
	unsigned char *ptr_cof = (unsigned char *) IMGPAR cof_d;
#endif
	for(i=0;i<16;i++)
	{
		memcpy(imgY+i*stride, ptr_cof+i*16, 16);
	}
	memset((unsigned char *) ptr_cof, 0, 4*BLOCK_SIZE*BLOCK_SIZE*BLOCK_SIZE*sizeof(short));//yolk
#ifdef __SUPPORT_YUV400__
	if (dec_picture->chroma_format_idc != YUV400)
	{
#endif
		// U
#if defined(ONE_COF)
		ptr_cof = (unsigned char *) &IMGPAR cof[4][0][0][0];
#else
		ptr_cof = (unsigned char *) (IMGPAR cof_d + 256);
#endif
		for(i=0;i<8;i++) // HP restriction
		{
			for(j=0;j<8;j++)
				*(imgUV+(i*stride_UV)+(j<<1)) = *(ptr_cof+i*8+j);
		}

		// V
#if defined(ONE_COF)
		ptr_cof = (unsigned char *) &IMGPAR cof[5][0][0][0];
#else
		ptr_cof = (unsigned char *) (IMGPAR cof_d + 320);
#endif
		for(i=0;i<8;i++) // HP restriction
		{
			for(j=0;j<8;j++)
				*(imgUV+(i*stride_UV)+(j<<1)+1) = *(ptr_cof+i*8+j);
		}
#if defined(ONE_COF)
		memset((unsigned char *) ((short *) IMGPAR cof+256), 0, 2*BLOCK_SIZE*BLOCK_SIZE*BLOCK_SIZE*sizeof(short));
#else
		memset((unsigned char *) (IMGPAR cof_d+256), 0, 2*BLOCK_SIZE*BLOCK_SIZE*BLOCK_SIZE*sizeof(short));
#endif
#ifdef __SUPPORT_YUV400__
	}
#endif

	// for deblocking filter
	currMB_s_d->qp=0;

	// for CAVLC: Set the nz_coeff to 16. 
	// These parameters are to be used in CAVLC decoding of neighbour blocks
	if(active_pps.entropy_coding_mode_flag == UVLC)
		memset(IMGPAR nz_coeff + IMGPAR current_mb_nr_d, 16, sizeof(IMGPAR nz_coeff[0]));

	// for CABAC decoding of MB skip flag 
	currMB_d->skip_flag = 0;

	//for deblocking filter CABAC
	currMB_s_d->cbp_blk=0xFFFF;

	//For CABAC decoding of Dquant
	last_dquant=0;
}

/*!
************************************************************************
* \brief
*    decode one macroblock
************************************************************************
*/
#undef min
#undef max
#define min __fast_min
#define max __fast_max

//#define NEW_BILINEAR
//#define bil_final(frac,p,q)	(((frac)*((q)-(p))+8*(p)+32)/64)
//#define bil_final(frac,p,q)	(((frac)*((q)-(p)) + ((p)<<3) + 32)>>(6))

/////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
CREL_RETURN decode_one_macroblock_I PARGS0()
{
	imgpel * imgY;
	int stride = dec_picture->Y_stride;
	int mb_nr = IMGPAR current_mb_nr_d;
	imgpel *imgUV;
	int stride_UV = dec_picture->UV_stride;	
	CREL_RETURN ret;

	// add for postAffProc

	if ((currMB_d->mb_field)&&(dec_picture->MbaffFrameFlag))
	{	
		if (mb_nr&1) // bottom field
		{
			imgY     = IMGPAR m_imgY + IMGPAR pix_x_d + ((IMGPAR pix_y_rows-15)*stride);
			imgUV = IMGPAR m_imgUV + ((IMGPAR pix_c_y_rows-7)*stride_UV) + (IMGPAR pix_c_x_d<<1); 			
		}
		else
		{
			imgY     = IMGPAR m_imgY + IMGPAR pix_x_d + (IMGPAR pix_y_rows*stride);
			imgUV = IMGPAR m_imgUV + IMGPAR pix_c_y_rows*stride_UV + (IMGPAR pix_c_x_d<<1); 			
		}	
		stride = stride<<1;
		stride_UV <<= 1;
	}	
	else
	{	
		imgY     = IMGPAR m_imgY + IMGPAR pix_x_d + (IMGPAR pix_y_rows*stride);
		imgUV = IMGPAR m_imgUV + IMGPAR pix_c_y_rows*stride_UV + (IMGPAR pix_c_x_d<<1);
	}

	if(currMB_d->mb_type==IPCM)
	{
		//copy readed data into imgY and set parameters
		decode_ipcm_mb ARGS4(imgY, stride, imgUV, stride_UV);
		if ( (IMGPAR mb_x_d+ 1) == IMGPAR PicWidthInMbs ) {
			IMGPAR FP_StoreImgRowToImgPic ARGS2(IMGPAR start_mb_x, IMGPAR mb_x_d);
		}
		return CREL_OK;
	}

	// get prediction for INTRA_MB_16x16
	ret = fp_MB_IntraPred_I[(currMB_d->mb_type >> 4)] ARGS2(imgY, stride);
	if (FAILED(ret)) {
		return ret;
	}

#ifdef __SUPPORT_YUV400__
	if (dec_picture->chroma_format_idc != YUV400)
#endif
	{
		ret = MB_I4MB_Chroma ARGS2(imgUV, stride_UV);
		if (FAILED(ret)) {
			return ret;

		}
	}

	if ( (IMGPAR mb_x_d+ 1) == IMGPAR PicWidthInMbs ) {
		IMGPAR FP_StoreImgRowToImgPic ARGS2(IMGPAR start_mb_x, IMGPAR mb_x_d);
	}

	return CREL_OK;
}

#if 1
// MBAFF (0/1) = 2
// structure (0/1/2)  = 3
// curr_mb_field (0/1) = 2
// mb_nr&1 (0/1) = 2
// list[k]->structure (0/1/2) = 3
static const char cva[2][3][2][2][3] = { 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // MBAFF=0, structure=FRAME
	-2, 0,-2,-2, 0,-2,-2, 0,-2,-2, 0,-2, // MBAFF=0, structure=TOP_FIELD
	2, 2, 0, 2, 2, 0, 2, 2, 0, 2, 2, 0, // MBAFF=0, structure=BOTTOM_FIELD
	0, 0, 0, 0, 0, 0, 0, 0,-2, 0, 2, 0, // MBAFF=1, structure=FRAME
	0, 0, 0, 0, 0, 0, 0, 0,-2, 0, 2, 0, // MBAFF=1, structure=TOP_FIELD
	0, 0, 0, 0, 0, 0, 0, 0,-2, 0, 2, 0  // MBAFF=1, structure=BOTTOM_FIELD
};
void calc_chroma_vector_adjustment PARGS2(int list_offset, int curr_mb_field)
{
	StorablePicture **list;
	int l, k;
	char *ptr = (char *) &cva[IMGPAR MbaffFrameFlag][IMGPAR structure][curr_mb_field][IMGPAR current_mb_nr_d&1][0];

	for(l=list_offset;l<list_offset+2;l++)
	{
		list = listX[l];
		for(k = 0; k < listXsize[l]; k++)
		{
			IMGPAR cr_vector_adjustment[l][k] = ptr[list[k]->structure];
		}
	}
}
#else
void calc_chroma_vector_adjustment PARGS2(int list_offset, int curr_mb_field)
{
	StorablePicture **list;
	const int MBAffFlag = IMGPAR MbaffFrameFlag;
	const int mb_nr = IMGPAR current_mb_nr_d;
	const int sturcture = IMGPAR structure;
	int l, k;
	if (!MBAffFlag)
	{
		if(sturcture == TOP_FIELD)
		{
			for(l=list_offset;l<list_offset+2;l++)
			{				
				list = listX[l];
				for(k = 0; k < listXsize[l]; k++)
				{
					if(list[k]->structure != TOP_FIELD)
						IMGPAR cr_vector_adjustment[l][k] = -2;
					else
						IMGPAR cr_vector_adjustment[l][k] = 0;
				}
			} 
		}
		else if(sturcture == BOTTOM_FIELD)
		{
			for(l=list_offset;l<list_offset+2;l++)
			{
				list = listX[l];
				for(k = 0; k < listXsize[l]; k++)
				{
					if(list[k]->structure != BOTTOM_FIELD)
						IMGPAR cr_vector_adjustment[l][k] = 2;
					else
						IMGPAR cr_vector_adjustment[l][k] = 0;
				}
			}
		}
		else
		{
			for(l=list_offset;l<list_offset+2;l++)
			{				
				list = listX[l];
				for(k = 0; k < listXsize[l]; k++)
				{
					IMGPAR cr_vector_adjustment[l][k] = 0;
				}
			}
		}
	}
	else
	{
		if (curr_mb_field)
		{
			if((mb_nr&1) == 0)
			{
				for(l=list_offset;l<list_offset+2;l++)
				{					
					list = listX[l];
					for(k = 0; k < listXsize[l]; k++)
					{
						if(list[k]->structure == BOTTOM_FIELD)
							IMGPAR cr_vector_adjustment[l][k] = -2;
						else
							IMGPAR cr_vector_adjustment[l][k] = 0;
					}
				}
			}
			else // ((IMGPAR current_mb_nr&1) == 1)
			{
				for(l=list_offset;l<list_offset+2;l++)
				{
					list = listX[l];
					for(k = 0; k < listXsize[l]; k++)
					{
						if(list[k]->structure == TOP_FIELD)
							IMGPAR cr_vector_adjustment[l][k] = 2;
						else
							IMGPAR cr_vector_adjustment[l][k] = 0;
					}
				}
			}
		}
		else
		{
			for(l=list_offset;l<list_offset+2;l++)
			{				
				list = listX[l];
				for(k = 0; k < listXsize[l]; k++)
				{
					IMGPAR cr_vector_adjustment[l][k] = 0;
				}
			}
		}
	}
}
#endif

CREL_RETURN decode_one_macroblock_B_0 PARGS0()
{
	imgpel * imgY;
	int stride = dec_picture->Y_stride;
	imgpel *imgUV;
	int stride_UV = dec_picture->UV_stride;	


	int mb_nr       = IMGPAR current_mb_nr_d;
	int list_offset;
	int vec_x_base, vec_y_base;

	int curr_mb_field = ((IMGPAR MbaffFrameFlag)&&(currMB_s_d->mb_field));

	// Base position for 0-motion vectors
	vec_x_base = IMGPAR pix_x_d;
	vec_y_base = IMGPAR pix_y_d;
	// find out the correct list offsets
	if (curr_mb_field)
	{
		vec_y_base >>= 1;
		if(mb_nr & 1)
		{
			list_offset = 4; // top field mb
			vec_y_base -= 8;
			imgY     = IMGPAR m_imgY + IMGPAR pix_x_d + ((IMGPAR pix_y_rows-15)*stride);
			imgUV = IMGPAR m_imgUV + ((IMGPAR pix_c_y_rows-7)*stride_UV) + (IMGPAR pix_c_x_d<<1); 
		}
		else
		{
			list_offset = 2; // bottom field mb
			imgY     = IMGPAR m_imgY + IMGPAR pix_x_d + (IMGPAR pix_y_rows*stride);
			imgUV = IMGPAR m_imgUV + IMGPAR pix_c_y_rows*stride_UV + (IMGPAR pix_c_x_d<<1); 
		}
		clip_max_y    = (dec_picture->size_y>>1)-1;
		clip_max_y_cr = (dec_picture->size_y_cr>>1)-1;
		stride = stride<<1;
		stride_UV <<= 1;
	}
	else
	{
		list_offset   = 0;  // no mb aff or frame mb
		clip_max_y    = dec_picture->size_y-1;
		clip_max_y_cr = dec_picture->size_y_cr-1;
		imgY     = IMGPAR m_imgY + IMGPAR pix_x_d + (IMGPAR pix_y_rows*stride);
		imgUV = IMGPAR m_imgUV + IMGPAR pix_c_y_rows*stride_UV + (IMGPAR pix_c_x_d<<1);
	}
	//clip_max_x      = dec_picture->size_x+1;
	//clip_max_x_cr   = dec_picture->size_x_cr-1;	

	calc_chroma_vector_adjustment ARGS2(list_offset, curr_mb_field);

	// luma decoding **************************************************
#if !defined(_COLLECT_PIC_)
	if(IMGPAR do_co_located)
	{
		if ((IS_DIRECT (currMB_d) || 
			(IS_P8x8(currMB_d) && !(currMB_d->b8mode[0] && currMB_d->b8mode[1] && currMB_d->b8mode[2] && currMB_d->b8mode[3]))))
		{
			if(IMGPAR direct_spatial_mv_pred_flag)
				IMGPAR FP_ReadMV_Direct_Spatial ARGS3(IMGPAR current_mb_nr_d, currMB_d, currMB_s_d);
			else
				IMGPAR FP_ReadMV_Direct_Temproal ARGS3(IMGPAR current_mb_nr_d, currMB_d, currMB_s_d);
			record_reference_picIds ARGS2(list_offset, currMB_s_d);
		}
	}//IMGPAR do_co_located
#endif

	fp_MB_InterPred_B0[currMB_d->mb_type] ARGS3(vec_x_base, vec_y_base, list_offset);

	IMGPAR FP_MB_itrans_Luma ARGS2(imgY, stride);
#ifdef __SUPPORT_YUV400__
	if (dec_picture->chroma_format_idc != YUV400)
#endif
		MB_itrans4x4_Chroma ARGS2(imgUV, stride_UV);

	if ( (IMGPAR mb_x_d+ 1) == IMGPAR PicWidthInMbs ) {
		StoreImgRowToImgPic ARGS2(IMGPAR start_mb_x, IMGPAR mb_x_d);
	}

	return CREL_OK;
}

CREL_RETURN decode_one_macroblock_B_1 PARGS0()
{
	imgpel * imgY;
	imgpel *imgUV;

	int list_offset;
	int vec_x_base, vec_y_base;

	
	int stride = dec_picture->Y_stride;
	int stride_UV = dec_picture->UV_stride;
	int mb_nr = IMGPAR current_mb_nr_d;
	int curr_mb_field = ((IMGPAR MbaffFrameFlag)&&(currMB_s_d->mb_field));

	// Base position for 0-motion vectors
	vec_x_base = IMGPAR pix_x_d;
	vec_y_base = IMGPAR pix_y_d;
	// find out the correct list offsets
	if (curr_mb_field)
	{
		vec_y_base >>= 1;
		if(mb_nr & 1)
		{
			list_offset = 4; // top field mb
			vec_y_base -= 8;
			imgY     = IMGPAR m_imgY + IMGPAR pix_x_d + ((IMGPAR pix_y_rows-15)*stride);
			imgUV = IMGPAR m_imgUV + ((IMGPAR pix_c_y_rows-7)*stride_UV) + (IMGPAR pix_c_x_d<<1); 
		}
		else
		{
			list_offset = 2; // bottom field mb
			imgY     = IMGPAR m_imgY + IMGPAR pix_x_d + (IMGPAR pix_y_rows*stride);
			imgUV = IMGPAR m_imgUV + IMGPAR pix_c_y_rows*stride_UV + (IMGPAR pix_c_x_d<<1); 
		}
		clip_max_y    = (dec_picture->size_y>>1)-1;
		clip_max_y_cr = (dec_picture->size_y_cr>>1)-1;
		stride = stride<<1;
		stride_UV <<= 1;
	}
	else
	{
		list_offset   = 0;  // no mb aff or frame mb
		clip_max_y    = dec_picture->size_y-1;
		clip_max_y_cr = dec_picture->size_y_cr-1;
		imgY     = IMGPAR m_imgY + IMGPAR pix_x_d + (IMGPAR pix_y_rows*stride);
		imgUV = IMGPAR m_imgUV + IMGPAR pix_c_y_rows*stride_UV + (IMGPAR pix_c_x_d<<1);
	}
	//clip_max_x      = dec_picture->size_x+1;
	//clip_max_x_cr   = dec_picture->size_x_cr-1;

	calc_chroma_vector_adjustment ARGS2(list_offset, curr_mb_field);

	// luma decoding **************************************************
#if !defined(_COLLECT_PIC_)
	if(IMGPAR do_co_located)
	{
		if ((IS_DIRECT (currMB_d) || 
			(IS_P8x8(currMB_d) && !(currMB_d->b8mode[0] && currMB_d->b8mode[1] && currMB_d->b8mode[2] && currMB_d->b8mode[3]))))
		{
			if(IMGPAR direct_spatial_mv_pred_flag)
				ret = IMGPAR FP_ReadMV_Direct_Spatial ARGS3(IMGPAR current_mb_nr_d, currMB_d, currMB_s_d);
			else
				ret = IMGPAR FP_ReadMV_Direct_Temproal ARGS3(IMGPAR current_mb_nr_d, currMB_d, currMB_s_d);
			record_reference_picIds ARGS2(list_offset, currMB_s_d);
		}
	}//IMGPAR do_co_located
#endif

	fp_MB_InterPred_B1[currMB_d->mb_type] ARGS3(vec_x_base, vec_y_base, list_offset);

	IMGPAR FP_MB_itrans_Luma ARGS2(imgY, stride);
#ifdef __SUPPORT_YUV400__
	if (dec_picture->chroma_format_idc != YUV400)
#endif
		MB_itrans4x4_Chroma ARGS2(imgUV, stride_UV);

	if ( (IMGPAR mb_x_d+ 1) == IMGPAR PicWidthInMbs ) {
		StoreImgRowToImgPic ARGS2(IMGPAR start_mb_x, IMGPAR mb_x_d);
	}

	return CREL_OK;
}

//////////////////////////////////////////////////////////////////////////
CREL_RETURN decode_one_macroblock_P_0 PARGS0()
{
	imgpel * imgY;
	int stride       = dec_picture->Y_stride;
	imgpel *imgUV;
	int stride_UV    = dec_picture->UV_stride;

	int vec_x_base, vec_y_base;
	int list_offset;

	int mb_nr              = IMGPAR current_mb_nr_d;
	int curr_mb_field      = ((IMGPAR MbaffFrameFlag)&&(currMB_s_d->mb_field));

	// Base position for 0-motion vectors
	vec_x_base = IMGPAR pix_x_d;
	vec_y_base = IMGPAR pix_y_d;
	// find out the correct list offsets
	if (curr_mb_field)
	{
		vec_y_base >>= 1;
		if(mb_nr & 1)
		{
			list_offset = 4; // top field mb
			vec_y_base -= 8;
			imgY     = IMGPAR m_imgY + IMGPAR pix_x_d + ((IMGPAR pix_y_rows-15)*stride);
			imgUV = IMGPAR m_imgUV + ((IMGPAR pix_c_y_rows-7)*stride_UV) + (IMGPAR pix_c_x_d<<1); 
		}
		else
		{
			list_offset = 2; // bottom field mb
			imgY     = IMGPAR m_imgY + IMGPAR pix_x_d + (IMGPAR pix_y_rows*stride);
			imgUV = IMGPAR m_imgUV + IMGPAR pix_c_y_rows*stride_UV + (IMGPAR pix_c_x_d<<1); 
		}
		clip_max_y    = (dec_picture->size_y>>1)-1;
		clip_max_y_cr = (dec_picture->size_y_cr>>1)-1;
		stride = stride<<1;
		stride_UV <<= 1;
	}
	else
	{
		list_offset   = 0;  // no mb aff or frame mb
		clip_max_y    = dec_picture->size_y-1;
		clip_max_y_cr = dec_picture->size_y_cr-1;
		imgY     = IMGPAR m_imgY + IMGPAR pix_x_d + (IMGPAR pix_y_rows*stride);
		imgUV = IMGPAR m_imgUV + IMGPAR pix_c_y_rows*stride_UV + (IMGPAR pix_c_x_d<<1);
	}
	//clip_max_x      = dec_picture->size_x+1;
	//clip_max_x_cr   = dec_picture->size_x_cr-1;	

	calc_chroma_vector_adjustment ARGS2(list_offset, curr_mb_field);


	fp_MB_InterPred_P0[currMB_d->mb_type] ARGS3(vec_x_base, vec_y_base, list_offset);

	IMGPAR FP_MB_itrans_Luma ARGS2(imgY, stride);
#ifdef __SUPPORT_YUV400__
	if (dec_picture->chroma_format_idc != YUV400)
#endif
		MB_itrans4x4_Chroma ARGS2(imgUV, stride_UV);

	if ( (IMGPAR mb_x_d+ 1) == IMGPAR PicWidthInMbs ) {
		IMGPAR FP_StoreImgRowToImgPic ARGS2(IMGPAR start_mb_x, IMGPAR mb_x_d);
	}

	return CREL_OK;
}

CREL_RETURN decode_one_macroblock_P_1 PARGS0()
{
	imgpel * imgY;
	int stride       = dec_picture->Y_stride;
	imgpel *imgUV;
	int stride_UV    = dec_picture->UV_stride;

	int vec_x_base, vec_y_base;

	int list_offset;

	int mb_nr              = IMGPAR current_mb_nr_d;
	int curr_mb_field      = ((IMGPAR MbaffFrameFlag)&&(currMB_s_d->mb_field));

	// Base position for 0-motion vectors
	vec_x_base = IMGPAR pix_x_d;
	vec_y_base = IMGPAR pix_y_d;
	// find out the correct list offsets
	if (curr_mb_field)
	{
		vec_y_base >>= 1;
		if(mb_nr & 1)
		{
			list_offset = 4; // top field mb
			vec_y_base -= 8;
			imgY     = IMGPAR m_imgY + IMGPAR pix_x_d + ((IMGPAR pix_y_rows-15)*stride);
			imgUV = IMGPAR m_imgUV + ((IMGPAR pix_c_y_rows-7)*stride_UV) + (IMGPAR pix_c_x_d<<1); 
		}
		else
		{
			list_offset = 2; // bottom field mb
			imgY     = IMGPAR m_imgY + IMGPAR pix_x_d + (IMGPAR pix_y_rows*stride);
			imgUV = IMGPAR m_imgUV + IMGPAR pix_c_y_rows*stride_UV + (IMGPAR pix_c_x_d<<1); 
		}
		clip_max_y    = (dec_picture->size_y>>1)-1;
		clip_max_y_cr = (dec_picture->size_y_cr>>1)-1;
		stride = stride<<1;
		stride_UV <<= 1;
	}
	else
	{
		list_offset   = 0;  // no mb aff or frame mb
		clip_max_y    = dec_picture->size_y-1;
		clip_max_y_cr = dec_picture->size_y_cr-1;
		imgY     = IMGPAR m_imgY + IMGPAR pix_x_d + (IMGPAR pix_y_rows*stride);
		imgUV = IMGPAR m_imgUV + IMGPAR pix_c_y_rows*stride_UV + (IMGPAR pix_c_x_d<<1);
	}
	//clip_max_x      = dec_picture->size_x+1;
	//clip_max_x_cr   = dec_picture->size_x_cr-1;	

	calc_chroma_vector_adjustment ARGS2(list_offset, curr_mb_field);

	fp_MB_InterPred_P1[currMB_d->mb_type] ARGS3(vec_x_base, vec_y_base, list_offset);

	IMGPAR FP_MB_itrans_Luma ARGS2(imgY, stride);
#ifdef __SUPPORT_YUV400__
	if (dec_picture->chroma_format_idc != YUV400)
#endif
		MB_itrans4x4_Chroma ARGS2(imgUV, stride_UV);

	if ( (IMGPAR mb_x_d+ 1) == IMGPAR PicWidthInMbs ) {
		IMGPAR FP_StoreImgRowToImgPic ARGS2(IMGPAR start_mb_x, IMGPAR mb_x_d);
	}

	return CREL_OK;
}

CREL_RETURN decode_one_macroblock_I_FMO PARGS0()
{
	imgpel * imgY;
	int stride = dec_picture->Y_stride;
	int mb_nr = IMGPAR current_mb_nr_d;
	imgpel *imgUV;
	int stride_UV = dec_picture->UV_stride;	
	CREL_RETURN ret;

	imgpel * dec_imgY;
	imgpel * dec_imgUV;
	int j;

	// add for postAffProc

	if ((currMB_d->mb_field)&&(dec_picture->MbaffFrameFlag))
	{	
		if (mb_nr&1) // bottom field
		{
			imgY     = IMGPAR m_imgY + IMGPAR pix_x_d + ((IMGPAR pix_y_rows-15)*stride);
			imgUV = IMGPAR m_imgUV + ((IMGPAR pix_c_y_rows-7)*stride_UV) + (IMGPAR pix_c_x_d<<1); 			
		}
		else
		{
			imgY     = IMGPAR m_imgY + IMGPAR pix_x_d + (IMGPAR pix_y_rows*stride);
			imgUV = IMGPAR m_imgUV + IMGPAR pix_c_y_rows*stride_UV + (IMGPAR pix_c_x_d<<1); 			
		}	
		stride = stride<<1;
		stride_UV <<= 1;
	}	
	else
	{	
		imgY     = IMGPAR m_imgY + IMGPAR pix_x_d + (IMGPAR pix_y_rows*stride);
		imgUV = IMGPAR m_imgUV + IMGPAR pix_c_y_rows*stride_UV + (IMGPAR pix_c_x_d<<1);
	}

	if(currMB_d->mb_type==IPCM)
	{
		//copy readed data into imgY and set parameters
		decode_ipcm_mb ARGS4(imgY, stride, imgUV, stride_UV);
		if ( (IMGPAR mb_x_d+ 1) == IMGPAR PicWidthInMbs ) {
			IMGPAR FP_StoreImgRowToImgPic ARGS2(IMGPAR start_mb_x, IMGPAR mb_x_d);
		}
		return CREL_OK;
	}

	// get prediction for INTRA_MB_16x16
	ret = fp_MB_IntraPred_I[(currMB_d->mb_type >> 4)] ARGS2(imgY, stride);
	if (FAILED(ret)) {
		return ret;
	}

#ifdef __SUPPORT_YUV400__
	if (dec_picture->chroma_format_idc != YUV400)
#endif
	{
		ret = MB_I4MB_Chroma ARGS2(imgUV, stride_UV);
		if (FAILED(ret)) {
			return ret;

		}
	}

	dec_imgY     = dec_picture->imgY + IMGPAR pix_y_d*stride + IMGPAR pix_x_d;
	dec_imgUV = dec_picture->imgUV + IMGPAR pix_c_y_d*stride_UV + IMGPAR pix_x_d;

	for(j=0;j<16;j++)
	{
		memcpy(dec_imgY, imgY, 16);
		dec_imgY += stride;
		imgY+=stride;
	}

	for(j=0;j<8;j++)
	{
		memcpy(dec_imgUV, imgUV, 16);
		dec_imgUV += stride_UV;
		imgUV+=stride_UV;
	}

	//if ( (IMGPAR mb_x_d+ 1) == IMGPAR PicWidthInMbs ) {
	//	IMGPAR FP_StoreImgRowToImgPic ARGS2(IMGPAR start_mb_x, IMGPAR mb_x_d);
	//}

	return CREL_OK;
}

CREL_RETURN decode_one_macroblock_P_0_FMO PARGS0()
{
	imgpel * imgY;
	int stride       = dec_picture->Y_stride;
	imgpel *imgUV;
	int stride_UV    = dec_picture->UV_stride;

	int vec_x_base, vec_y_base;
	int list_offset;

	int mb_nr              = IMGPAR current_mb_nr_d;
	int curr_mb_field      = ((IMGPAR MbaffFrameFlag)&&(currMB_s_d->mb_field));

	imgpel * dec_imgY;
	imgpel * dec_imgUV;
	int j;

	// Base position for 0-motion vectors
	vec_x_base = IMGPAR pix_x_d;
	vec_y_base = IMGPAR pix_y_d;
	// find out the correct list offsets
	if (curr_mb_field)
	{
		vec_y_base >>= 1;
		if(mb_nr & 1)
		{
			list_offset = 4; // top field mb
			vec_y_base -= 8;
			imgY     = IMGPAR m_imgY + IMGPAR pix_x_d + ((IMGPAR pix_y_rows-15)*stride);
			imgUV = IMGPAR m_imgUV + ((IMGPAR pix_c_y_rows-7)*stride_UV) + (IMGPAR pix_c_x_d<<1); 
		}
		else
		{
			list_offset = 2; // bottom field mb
			imgY     = IMGPAR m_imgY + IMGPAR pix_x_d + (IMGPAR pix_y_rows*stride);
			imgUV = IMGPAR m_imgUV + IMGPAR pix_c_y_rows*stride_UV + (IMGPAR pix_c_x_d<<1); 
		}
		clip_max_y    = (dec_picture->size_y>>1)-1;
		clip_max_y_cr = (dec_picture->size_y_cr>>1)-1;
		stride = stride<<1;
		stride_UV <<= 1;
	}
	else
	{
		list_offset   = 0;  // no mb aff or frame mb
		clip_max_y    = dec_picture->size_y-1;
		clip_max_y_cr = dec_picture->size_y_cr-1;
		imgY     =  IMGPAR m_imgY + IMGPAR pix_x_d + (IMGPAR pix_y_rows*stride);
		imgUV =  IMGPAR m_imgUV + IMGPAR pix_c_y_rows*stride_UV + (IMGPAR pix_c_x_d<<1);
	}
	//clip_max_x      = dec_picture->size_x+1;
	//clip_max_x_cr   = dec_picture->size_x_cr-1;	

	calc_chroma_vector_adjustment ARGS2(list_offset, curr_mb_field);


	fp_MB_InterPred_P0[currMB_d->mb_type] ARGS3(vec_x_base, vec_y_base, list_offset);

	IMGPAR FP_MB_itrans_Luma ARGS2(imgY, stride);
#ifdef __SUPPORT_YUV400__
	if (dec_picture->chroma_format_idc != YUV400)
#endif
		MB_itrans4x4_Chroma ARGS2(imgUV, stride_UV);

	dec_imgY     = dec_picture->imgY + IMGPAR pix_y_d*stride + IMGPAR pix_x_d;
	dec_imgUV = dec_picture->imgUV + IMGPAR pix_c_y_d*stride_UV + IMGPAR pix_x_d;

	for(j=0;j<16;j++)
	{
		memcpy(dec_imgY, imgY, 16);
		dec_imgY += stride;
		imgY+=stride;
	}

	for(j=0;j<8;j++)
	{
		memcpy(dec_imgUV, imgUV, 16);
		dec_imgUV += stride_UV;
		imgUV+=stride_UV;
	}

	//if ( (IMGPAR mb_x_d+ 1) == IMGPAR PicWidthInMbs ) {
	//	IMGPAR FP_StoreImgRowToImgPic ARGS2(IMGPAR start_mb_x, IMGPAR mb_x_d);
	//}

	return CREL_OK;
}

CREL_RETURN decode_one_macroblock_P_1_FMO PARGS0()
{
	imgpel * imgY;
	int stride       = dec_picture->Y_stride;
	imgpel *imgUV;
	int stride_UV    = dec_picture->UV_stride;

	int vec_x_base, vec_y_base;

	int list_offset;

	int mb_nr              = IMGPAR current_mb_nr_d;
	int curr_mb_field      = ((IMGPAR MbaffFrameFlag)&&(currMB_s_d->mb_field));

	imgpel * dec_imgY;
	imgpel * dec_imgUV;
	int j;

	// Base position for 0-motion vectors
	vec_x_base = IMGPAR pix_x_d;
	vec_y_base = IMGPAR pix_y_d;
	// find out the correct list offsets
	if (curr_mb_field)
	{
		vec_y_base >>= 1;
		if(mb_nr & 1)
		{
			list_offset = 4; // top field mb
			vec_y_base -= 8;
			imgY     = IMGPAR m_imgY + IMGPAR pix_x_d + ((IMGPAR pix_y_rows-15)*stride);
			imgUV = IMGPAR m_imgUV + ((IMGPAR pix_c_y_rows-7)*stride_UV) + (IMGPAR pix_c_x_d<<1); 
		}
		else
		{
			list_offset = 2; // bottom field mb
			imgY     = IMGPAR m_imgY + IMGPAR pix_x_d + (IMGPAR pix_y_rows*stride);
			imgUV = IMGPAR m_imgUV + IMGPAR pix_c_y_rows*stride_UV + (IMGPAR pix_c_x_d<<1); 
		}
		clip_max_y    = (dec_picture->size_y>>1)-1;
		clip_max_y_cr = (dec_picture->size_y_cr>>1)-1;
		stride = stride<<1;
		stride_UV <<= 1;
	}
	else
	{
		list_offset   = 0;  // no mb aff or frame mb
		clip_max_y    = dec_picture->size_y-1;
		clip_max_y_cr = dec_picture->size_y_cr-1;
		imgY     =  IMGPAR m_imgY + IMGPAR pix_x_d + (IMGPAR pix_y_rows*stride);
		imgUV =  IMGPAR m_imgUV + IMGPAR pix_c_y_rows*stride_UV + (IMGPAR pix_c_x_d<<1);
	}
	//clip_max_x      = dec_picture->size_x+1;
	//clip_max_x_cr   = dec_picture->size_x_cr-1;	

	calc_chroma_vector_adjustment ARGS2(list_offset, curr_mb_field);

	fp_MB_InterPred_P1[currMB_d->mb_type] ARGS3(vec_x_base, vec_y_base, list_offset);

	IMGPAR FP_MB_itrans_Luma ARGS2(imgY, stride);
#ifdef __SUPPORT_YUV400__
	if (dec_picture->chroma_format_idc != YUV400)
#endif
		MB_itrans4x4_Chroma ARGS2(imgUV, stride_UV);

	dec_imgY     = dec_picture->imgY + IMGPAR pix_y_d*stride + IMGPAR pix_x_d;
	dec_imgUV = dec_picture->imgUV + IMGPAR pix_c_y_d*stride_UV + IMGPAR pix_x_d;

	for(j=0;j<16;j++)
	{
		memcpy(dec_imgY, imgY, 16);
		dec_imgY += stride;
		imgY+=stride;
	}

	for(j=0;j<8;j++)
	{
		memcpy(dec_imgUV, imgUV, 16);
		dec_imgUV += stride_UV;
		imgUV+=stride_UV;
	}

	//if ( (IMGPAR mb_x_d+ 1) == IMGPAR PicWidthInMbs ) {
	//	IMGPAR FP_StoreImgRowToImgPic ARGS2(IMGPAR start_mb_x, IMGPAR mb_x_d);
	//}

	return CREL_OK;
}

#if defined(_M56_OPT_)
void mb_direct_spatial_mv_pred_DXVA1ATI PARGS3(int current_mb_nr, Macroblock *currMB, Macroblock_s *currMB_s)
{	
	int j4, i4;
	int k;
	int mb_nr = current_mb_nr;
	int fw_rFrameL, fw_rFrameU, fw_rFrameUL, fw_rFrameUR;
	int bw_rFrameL, bw_rFrameU, bw_rFrameUL, bw_rFrameUR;

	PixelPos mb_left, mb_upleft;
	PixelPos *p_mb_left = &mb_left;
	//PixelPos *p_mb_up = &mb_up;

	int upright_y;

	int  fw_rFrame,bw_rFrame;
	MotionVector pmvfw, pmvbw;	

	MotionVector *pmv;

	Macroblock_s *mb_a, *mb_b, *mb_c, *mb_d;

	int off_x, off_y;

	Pred_s_info *info = &currMB_s->pred_info;

	int l_16_4_idx, idx;
	MotionVector *pMvTempForward, *pMvTempBackward;
	int temp1, temp2, i;

	int stride = IMGPAR PicWidthInMbs << 2;
	int a = IMGPAR mb_y_r * 4;
	int b = IMGPAR mb_x_r * 4;

	p_mb_left = &(IMGPAR left[0]);
//	p_mb_up = &(IMGPAR up[0]);

	mb_c = 0;
	//getLuma4x4Neighbour ARGS6(mb_nr, 0, 0, 16, -1, &mb_upright);

	mb_upleft.pMB = IMGPAR pUpLeftMB_r;
	mb_upleft.mb_addr = IMGPAR mbAddrD;
	mb_upleft.x = 3;
	if (IMGPAR MbaffFrameFlag && currMB_r->mbStatusD == 5)
		mb_upleft.y = 1;
	else
		mb_upleft.y = 3;

	if(p_mb_left->pMB)
		mb_a = &dec_picture->mb_data[p_mb_left->mb_addr];
	if(IMGPAR pUpMB_r)
		mb_b = &dec_picture->mb_data[IMGPAR mbAddrB];
	if (IMGPAR pUpRightMB_r) {
		mb_c = &dec_picture->mb_data[IMGPAR mbAddrC];
		upright_y = ((-1<<IMGPAR mb_up_factor)+16)>>2;
	}
	if(mb_upleft.pMB)
		mb_d = &dec_picture->mb_data[mb_upleft.mb_addr];

	if (!IMGPAR MbaffFrameFlag)
	{
		if(mb_upleft.pMB)
		{
			l_16_4_idx = l_16_4[(mb_upleft.y<<2)+3];
			fw_rFrameUL = mb_d->pred_info.ref_idx[0][l_16_4_idx];
			bw_rFrameUL = mb_d->pred_info.ref_idx[1][l_16_4_idx];	
		}
		else
		{
			fw_rFrameUL = bw_rFrameUL = -1;
		}

		if(IMGPAR pUpMB_r)
		{
			l_16_4_idx = l_16_4[3<<2];
			fw_rFrameU = mb_b->pred_info.ref_idx[0][l_16_4_idx];
			bw_rFrameU = mb_b->pred_info.ref_idx[1][l_16_4_idx];
		}
		else
		{
			fw_rFrameU = bw_rFrameU = -1;
		}

		if(mb_c)
		{
			l_16_4_idx = l_16_4[upright_y<<2];
			fw_rFrameUR = mb_c->pred_info.ref_idx[0][l_16_4_idx];
			bw_rFrameUR = mb_c->pred_info.ref_idx[1][l_16_4_idx];
		}
		else
		{
			fw_rFrameUR = fw_rFrameUL;
			bw_rFrameUR = bw_rFrameUL;
		}

		if(p_mb_left->pMB)
		{
			l_16_4_idx = l_16_4[(p_mb_left->y<<2)+3];
			fw_rFrameL = mb_a->pred_info.ref_idx[0][l_16_4_idx];
			bw_rFrameL = mb_a->pred_info.ref_idx[1][l_16_4_idx];	
		}
		else
		{
			fw_rFrameL = bw_rFrameL = -1;
		}
	}
	else
	{
		if (currMB_s->mb_field)
		{
			if(mb_upleft.pMB)
			{
				l_16_4_idx = l_16_4[(mb_upleft.y<<2)+3];
				fw_rFrameUL = mb_d->pred_info.ref_idx[0][l_16_4_idx] << (!mb_d->mb_field);
				bw_rFrameUL = mb_d->pred_info.ref_idx[1][l_16_4_idx] << (!mb_d->mb_field);
			}
			else
			{
				fw_rFrameUL = bw_rFrameUL = -1;
			}

			if(IMGPAR pUpMB_r)
			{
				l_16_4_idx = l_16_4[3<<2];
				fw_rFrameU = mb_b->pred_info.ref_idx[0][l_16_4_idx] << (!mb_b->mb_field);
				bw_rFrameU = mb_b->pred_info.ref_idx[1][l_16_4_idx] << (!mb_b->mb_field);
			}
			else
			{
				fw_rFrameU = bw_rFrameU = -1;
			}

			if(mb_c)
			{
				l_16_4_idx = l_16_4[upright_y<<2];
				fw_rFrameUR = mb_c->pred_info.ref_idx[0][l_16_4_idx] << (!mb_c->mb_field);
				bw_rFrameUR = mb_c->pred_info.ref_idx[1][l_16_4_idx] << (!mb_c->mb_field);
			}
			else
			{
				fw_rFrameUR = fw_rFrameUL;
				bw_rFrameUR = bw_rFrameUL;
			}

			if(p_mb_left->pMB)
			{
				l_16_4_idx = l_16_4[(p_mb_left->y<<2)+3];
				fw_rFrameL = mb_a->pred_info.ref_idx[0][l_16_4_idx] << (!mb_a->mb_field);
				bw_rFrameL = mb_a->pred_info.ref_idx[1][l_16_4_idx] << (!mb_a->mb_field);
			}
			else
			{
				fw_rFrameL = bw_rFrameL = -1;
			}

		}
		else
		{
			if(mb_upleft.pMB)
			{
				l_16_4_idx = l_16_4[(mb_upleft.y<<2)+3];
				fw_rFrameUL = mb_d->pred_info.ref_idx[0][l_16_4_idx] >> (mb_d->mb_field);
				bw_rFrameUL = mb_d->pred_info.ref_idx[1][l_16_4_idx] >> (mb_d->mb_field);
			}
			else
			{
				fw_rFrameUL = bw_rFrameUL = -1;
			}

			if(IMGPAR pUpMB_r)
			{
				l_16_4_idx = l_16_4[3<<2];
				fw_rFrameU = mb_b->pred_info.ref_idx[0][l_16_4_idx] >> (mb_b->mb_field);
				bw_rFrameU = mb_b->pred_info.ref_idx[1][l_16_4_idx] >> (mb_b->mb_field);
			}
			else
			{
				fw_rFrameU = bw_rFrameU = -1;
			}

			if(mb_c)
			{
				l_16_4_idx = l_16_4[upright_y<<2];
				fw_rFrameUR = mb_c->pred_info.ref_idx[0][l_16_4_idx] >> (mb_c->mb_field);
				bw_rFrameUR = mb_c->pred_info.ref_idx[1][l_16_4_idx] >> (mb_c->mb_field);
			}
			else
			{
				fw_rFrameUR = fw_rFrameUL;
				bw_rFrameUR = bw_rFrameUL;
			}

			if(p_mb_left->pMB)
			{
				l_16_4_idx = l_16_4[(p_mb_left->y<<2)+3];
				fw_rFrameL = mb_a->pred_info.ref_idx[0][l_16_4_idx] >> (mb_a->mb_field);
				bw_rFrameL = mb_a->pred_info.ref_idx[1][l_16_4_idx] >> (mb_a->mb_field);
			}
			else
			{
				fw_rFrameL = bw_rFrameL = -1;
			}
		}
	}

	fw_rFrame = (fw_rFrameL >= 0 && fw_rFrameU >= 0) ? min(fw_rFrameL,fw_rFrameU): max(fw_rFrameL,fw_rFrameU);
	fw_rFrame = (fw_rFrame >= 0 && fw_rFrameUR >= 0) ? min(fw_rFrame,fw_rFrameUR): max(fw_rFrame,fw_rFrameUR);

	bw_rFrame = (bw_rFrameL >= 0 && bw_rFrameU >= 0) ? min(bw_rFrameL,bw_rFrameU): max(bw_rFrameL,bw_rFrameU);
	bw_rFrame = (bw_rFrame >= 0 && bw_rFrameUR >= 0) ? min(bw_rFrame,bw_rFrameUR): max(bw_rFrame,bw_rFrameUR);

	if (fw_rFrame >=0)
		SetMotionVectorPredictor_block00_shape16x16 ARGS6(&pmvfw, fw_rFrame, LIST_0, current_mb_nr, currMB, currMB_s);
	else
		pmvfw.mv_comb = 0;

	if (bw_rFrame >=0)
		SetMotionVectorPredictor_block00_shape16x16 ARGS6(&pmvbw, bw_rFrame, LIST_1, current_mb_nr, currMB, currMB_s);
	else
		pmvbw.mv_comb = 0;

	for (k=0;k<4;k++)
	{
		if (currMB->b8mode[k] == 0)	//Direct Mode for this 8x8
		{
			off_x = ((k&1) <<1) ;
			off_y = k&2 ;

			if (active_sps.direct_8x8_inference_flag)
			{	// frame_mbs_only_flag can be 0 or 1
				if ( (fw_rFrame >= 0) && (bw_rFrame >= 0) )	{
					pmv = &info->mv[0][b8_idx[k]];
					if  (!fw_rFrame  && (!Co_located_MB->moving_block[off_y][off_x])) {
						*(int*)pmv = 0;
					} else {
						*(int*)pmv = pmvfw.mv_comb;							
					}

					*(int*)(pmv+1) = *(int*)(pmv+4) = *(int*)(pmv+5) = *(int*)pmv;
					info->ref_idx[0][k] = fw_rFrame;
					temp1 = pmv->mv_comb;	//Fill MV Plane for ATI DXVA		

					pmv = &info->mv[1][b8_idx[k]];
					if  (!bw_rFrame && (!Co_located_MB->moving_block[off_y][off_x])) {
						*(int*)pmv = 0;
					} else {
						*(int*)pmv = pmvbw.mv_comb;
					}

					*(int*)(pmv+1) = *(int*)(pmv+4) = *(int*)(pmv+5) = *(int*)pmv;
					info->ref_idx[1][k] = bw_rFrame;
					temp2 = pmv->mv_comb;		//Fill MV Plane for ATI DXVA

					pMvTempForward = ((MotionVector*)dec_picture->mv_data0) + ((a + off_y)* stride) + b + off_x;
					pMvTempBackward = pMvTempForward + (IMGPAR PicSizeInMbs << 4);
					for ( i = 0; i < 2; i++ ) {
						*(int*)pMvTempForward = temp1;
						*(int*)(pMvTempForward+1) = temp1;

						*(int*)pMvTempBackward = temp2;
						*(int*)(pMvTempBackward+1) = temp2;

						pMvTempForward += stride;
						pMvTempBackward += stride;
					}
				} else if ( (fw_rFrame < 0) && (bw_rFrame >= 0) ) {
					
					if  (!bw_rFrame && (!Co_located_MB->moving_block[off_y][off_x])) {
						info->mv[1][b8_idx[k]].mv_comb = 0;
					} else {
						info->mv[1][b8_idx[k]].mv_comb = pmvbw.mv_comb;
					}
					info->ref_idx[1][k] = bw_rFrame;
					pmv = &info->mv[1][b8_idx[k]];
					*(pmv+1) = *(pmv+4) = *(pmv+5) = *pmv;
					
					temp2 = pmv->mv_comb;		//Fill MV Plane for ATI DXVA

					pMvTempBackward = ((MotionVector*)dec_picture->mv_data1) + ((a + off_y)* stride) + b + off_x;
					for ( i = 0; i < 2; i++ ) {
						*(int*)pMvTempBackward = temp2;
						*(int*)(pMvTempBackward+1) = temp2;
						pMvTempBackward += stride;
					}

				} else if ( (fw_rFrame >= 0) && (bw_rFrame < 0) ) {

					if  (!fw_rFrame  && (!Co_located_MB->moving_block[off_y][off_x])) {
						info->mv[0][b8_idx[k]].mv_comb = 0;
					} else {
						info->mv[0][b8_idx[k]].mv_comb = pmvfw.mv_comb;
					}
					info->ref_idx[0][k] = fw_rFrame;
					pmv = &info->mv[0][b8_idx[k]];
					*(pmv+1) = *(pmv+4) = *(pmv+5) = *pmv;

					temp1 = pmv->mv_comb;	//Fill MV Plane for ATI DXVA		

					pMvTempBackward = ((MotionVector*)dec_picture->mv_data1) + ((a + off_y)* stride) + b + off_x;
					for ( i = 0; i < 2; i++ ) {
						*(int*)pMvTempBackward = temp1;
						*(int*)(pMvTempBackward+1) = temp1;
						pMvTempBackward += stride;
					}

				} else {
					info->ref_idx[0][k] = 0;
					info->ref_idx[1][k] = 0;	

				}

			}
			else
			{   // frame_mbs_only_flag=1

				pMvTempForward = ((MotionVector*)dec_picture->mv_data0) + ((a + off_y)* stride) + b + off_x;
				pMvTempBackward =  pMvTempForward + (IMGPAR PicSizeInMbs << 4);


				for(j4=off_y; j4<off_y+2; j4++)
				{
					for(i4=off_x; i4<off_x+2; i4++)
					{
						idx = j4_i4[j4][i4];	// 4*j4 + i4						

						if ( (fw_rFrame >= 0) && (bw_rFrame >= 0) ) {
							if  (!fw_rFrame  && (!Co_located_MB->moving_block[j4][i4]))
							{
								info->mv[0][idx].mv_comb = 0;
								*(int*)pMvTempForward = 0;
							} else {
								info->mv[0][idx].mv_comb = pmvfw.mv_comb;
								*(int*)pMvTempForward =  pmvfw.mv_comb;
							}
							info->ref_idx[0][k] = fw_rFrame;

							if  (bw_rFrame==0 && (!Co_located_MB->moving_block[j4][i4]))
							{
								info->mv[1][idx].mv_comb = 0;
								*(int*)pMvTempBackward = 0;
							}
							else
							{
								info->mv[1][idx].mv_comb = pmvbw.mv_comb;
								*(int*)pMvTempBackward = pmvbw.mv_comb;
							}
							info->ref_idx[1][k] = bw_rFrame;							
						} else if ( (fw_rFrame < 0) && (bw_rFrame >= 0) ) {
							if  (bw_rFrame==0 && (!Co_located_MB->moving_block[j4][i4]))
							{
								info->mv[1][idx].mv_comb = 0;
								*(int*)pMvTempBackward = 0;
							}
							else
							{
								info->mv[1][idx].mv_comb = pmvbw.mv_comb;
								*(int*)pMvTempBackward = pmvbw.mv_comb;
							}
							info->ref_idx[1][k] = bw_rFrame;
						} else if ( (fw_rFrame >= 0) && (bw_rFrame < 0) ) {
							if  (!fw_rFrame  && (!Co_located_MB->moving_block[j4][i4]))
							{
								info->mv[0][idx].mv_comb = 0;
								*(int*)pMvTempBackward = 0;
							} else {
								info->mv[0][idx].mv_comb = pmvfw.mv_comb;
								*(int*)pMvTempBackward =  pmvfw.mv_comb;
							}
							info->ref_idx[0][k] = fw_rFrame;

						} else {
							info->ref_idx[0][k] = 0;
							info->ref_idx[1][k] = 0;
						}

						pMvTempForward ++;
						pMvTempBackward ++;
					}
					pMvTempForward += (stride - 1);
					pMvTempBackward += (stride - 1);
				}
			}
		}
	}
}

void mb_direct_temporal_mv_pred_DXVA1ATI PARGS3(int current_mb_nr, Macroblock *currMB, Macroblock_s *currMB_s)
{
	int list_offset;// = ((IMGPAR MbaffFrameFlag)&&(currMB->mb_field))? IMGPAR current_mb_nr%2 ? 4 : 2 : 0;

	Pred_s_info *info = &currMB_s->pred_info;

	Pred_s_info *co_info;

	co_info = &Co_located_MB->pred_info;

	int i, j;
	int b4, b8, i_1, j_1;
	MotionVector *pMvTempForward, *pMvTempBackward;
	int temp1, temp2;

#ifdef DO_REF_PIC_NUM
	int64 *pRef_pic_num;
#endif
	int num_ref;

	if ((IMGPAR MbaffFrameFlag)&&(currMB_s->mb_field))
	{
		list_offset = (current_mb_nr & 1) ? 4 : 2;
	}
	else
	{
		list_offset = 0;
	}


#ifdef DO_REF_PIC_NUM
	pRef_pic_num = dec_picture->ref_pic_num[IMGPAR current_slice_nr][LIST_0 + list_offset];
#endif

	num_ref = min(IMGPAR num_ref_idx_l0_active,listXsize[LIST_0 + list_offset]);

	if(active_sps.direct_8x8_inference_flag)
	{   // frame_mbs_only_flag can be 0 or 1
		MotionVector *pmv;
		//char *pred_idx;

		for (b8=0;b8<4;b8++)
		{
			if (currMB->b8mode[b8] == 0)	//Direct Mode for this 8x8 sub-macroblock
			{
				i = ((b8&1)<<1);
				j = (b8&2);
				int refList = (co_info->ref_idx[0][b8]== -1 ? LIST_1 : LIST_0);
				int co_loc_refidx = co_info->ref_idx[refList][b8];
				int co_loc_refpicid = co_info->ref_pic_id[refList][b8];
				MotionVector two_mv;
				two_mv.mv_comb = co_info->mv[refList][b8_idx[b8]].mv_comb;

				if(co_loc_refidx==-1) // co-located is intra mode
				{
					info->mv[0][b8_idx[b8]].mv_comb = 0;
					info->mv[1][b8_idx[b8]].mv_comb = 0;

					info->ref_idx[0][b8] = 0;
					info->ref_idx[1][b8] = 0;
				}
				else // co-located skip or inter mode
				{
					int mapped_idx=0;
					int iref;
					int mv_scale;

					for (iref=0;iref<num_ref;iref++)
					{
						if (listX[LIST_0 + list_offset][iref]->unique_id == co_loc_refpicid)
						{
							mapped_idx=iref;
							break;
						}
						else //! invalid index. Default to zero even though this case should not happen
						{
							mapped_idx=INVALIDINDEX;
						}
					}
					if (INVALIDINDEX == mapped_idx)
					{
						DEBUG_SHOW_ERROR_INFO("temporal direct error\ncolocated block has ref that is unavailable");
					}

					mv_scale  = IMGPAR mvscale[LIST_0 + list_offset][mapped_idx];

					//! In such case, an array is needed for each different reference.
					if (mv_scale == 9999 || listX[LIST_0+list_offset][mapped_idx]->is_long_term)
					{
						info->mv[0][b8_idx[b8]].mv_comb = two_mv.mv_comb;
						info->mv[1][b8_idx[b8]].mv_comb = 0;
					}
					else
					{
						info->mv[0][b8_idx[b8]].x=(mv_scale * two_mv.x + 128 ) >> 8;
						info->mv[0][b8_idx[b8]].y=(mv_scale * two_mv.y + 128 ) >> 8;

						info->mv[1][b8_idx[b8]].x=info->mv[0][b8_idx[b8]].x - two_mv.x;
						info->mv[1][b8_idx[b8]].y=info->mv[0][b8_idx[b8]].y - two_mv.y;
					}

					info->ref_idx[0][b8] = mapped_idx; //listX[1][0]->ref_idx[refList][j4][i4];
					info->ref_idx[1][b8] = 0;
				}

				pmv = &info->mv[0][b8_idx[b8]];
				*(pmv+1) = *(pmv+4) = *(pmv+5) = *pmv;

				temp1 = pmv->mv_comb;


				pmv = &info->mv[1][b8_idx[b8]];
				*(pmv+1) = *(pmv+4) = *(pmv+5) = *pmv;
				
//Fill MV Plane for ATI DXVA				

				temp2 = pmv->mv_comb;

				pMvTempForward = ((MotionVector*)dec_picture->mv_data0) + ((IMGPAR mb_y_r *4 + j)* IMGPAR PicWidthInMbs * 4) + IMGPAR mb_x_r *4 + i;
				pMvTempBackward = pMvTempForward + (IMGPAR PicSizeInMbs << 4);
				for ( i = 0; i < 2; i++ ) {
					*(int*)pMvTempForward = temp1;
					*(int*)(pMvTempForward+1) = temp1;

					*(int*)pMvTempBackward = temp2;
					*(int*)(pMvTempBackward+1) = temp2;

					pMvTempForward += IMGPAR PicWidthInMbs * 4;
					pMvTempBackward += IMGPAR PicWidthInMbs * 4;
				}
			}
		}
	}
	else
	{   // frame_mbs_only_flag=1
		for (b8=0;b8<4;b8++)
		{
			if (currMB->b8mode[b8] == 0)	//Direct Mode for this 8x8 sub-macroblock
			{
				i_1 = ((b8&1)<<1);
				j_1 = (b8&2);

				for (b4=0;b4<4;b4++)
				{
					i    = i_1 + (b4&1);
					j    = j_1 + (b4>>1);

					pMvTempForward = ((MotionVector*)dec_picture->mv_data0) + ((IMGPAR mb_y_r *4 + j)* IMGPAR PicWidthInMbs * 4) + IMGPAR mb_x_r *4 + i;
					pMvTempBackward = pMvTempForward + (IMGPAR PicSizeInMbs << 4);

					int refList = (co_info->ref_idx[0][b8]== -1 ? LIST_1 : LIST_0);
					int idx = j4_i4[j][i];	// 4*j + i

					if(co_info->ref_idx[refList][b8]==-1) // co-located is intra mode
					{
						info->mv[0][idx].mv_comb = 0;
						info->mv[1][idx].mv_comb = 0;

						*(int*)pMvTempForward = 0;
						*(int*)pMvTempBackward = 0;

						info->ref_idx[0][b8] = 0;
						info->ref_idx[1][b8] = 0;
					}
					else // co-located skip or inter mode
					{
						int mapped_idx=0;
						int iref;
						int mv_scale;

						for (iref=0;iref<num_ref;iref++)
						{
							if (listX[LIST_0 + list_offset][iref]->unique_id == co_info->ref_pic_id[refList][b8])
							{
								mapped_idx=iref;
								break;
							}
							else //! invalid index. Default to zero even though this case should not happen
							{                        
								mapped_idx=INVALIDINDEX;
							}
						}
						if (INVALIDINDEX == mapped_idx)
						{
							DEBUG_SHOW_ERROR_INFO("temporal direct error\ncolocated block has ref that is unavailable");
						}					

						mv_scale  = IMGPAR mvscale[LIST_0 + list_offset][mapped_idx];

						//! In such case, an array is needed for each different reference.
						if (mv_scale == 9999 || listX[LIST_0+list_offset][mapped_idx]->is_long_term)
						{
							info->mv[0][idx].mv_comb = co_info->mv[refList][idx].mv_comb;
							info->mv[1][idx].mv_comb = 0;

							*(int*)pMvTempForward = info->mv[0][idx].mv_comb;
							*(int*)pMvTempBackward = 0;
						}
						else
						{
							MotionVector two_mv;
							two_mv.mv_comb = co_info->mv[refList][idx].mv_comb;

							info->mv[0][idx].x=(mv_scale * two_mv.x + 128 ) >> 8;
							info->mv[0][idx].y=(mv_scale * two_mv.y + 128 ) >> 8;

							info->mv[1][idx].x=info->mv[0][idx].x - two_mv.x;
							info->mv[1][idx].y=info->mv[0][idx].y - two_mv.y;
							
							*(int*)pMvTempForward = info->mv[0][idx].mv_comb;
							*(int*)pMvTempBackward = info->mv[1][idx].mv_comb;
						}

						info->ref_idx[0][b8] = mapped_idx; //listX[1][0]->ref_idx[refList][j4][i4];
						info->ref_idx[1][b8] = 0;
					}
				}
			}
		}
	}
}

void ReadMotionInfo16x16_DXVA1ATI PARGS2(Pred_s_info *info, int list)
{
	MotionVector vec;
	MotionVector curr_mvd;	
	int refframe;
	int temp;
	MotionVector *pMvTemp;
	int i;	
	int stride = IMGPAR PicWidthInMbs << 2;

	refframe = info->ref_idx[list][0];

	IMGPAR subblock_y = 0; // position used for context determination
	IMGPAR subblock_x = 0; // position used for context determination

	// first make mv-prediction
	SetMotionVectorPredictor_block00_shape16x16 ARGS6(&vec, refframe, list, IMGPAR current_mb_nr_r, currMB_r, currMB_s_r);

	// set motion vectors (x, y)	

	g_read_functions->readMVD ARGS2(list,&curr_mvd);
	// x
	vec.x = vec.x + curr_mvd.x;
	// y
	vec.y = vec.y + curr_mvd.y;


	Set16MotionVector(info->mv[list], vec);

	//temp = (DWORD)vec.x & 0x0000ffff;
	//temp |= ((DWORD)vec.y) << 16;

	temp = vec.mv_comb;

	pMvTemp = ((MotionVector*)dec_picture->mv_data1) + (IMGPAR mb_y_r * IMGPAR PicWidthInMbs * 16) + IMGPAR mb_x_r *4;	

	if ((currMB_r->b8pdir[0] == 2) && (list == 0)) {
		pMvTemp -= (IMGPAR PicSizeInMbs << 4);
	}
	
	for ( i = 0; i < 4; i++ ) {
		*(int*)pMvTemp = temp;
		*(int*)(pMvTemp+1) = temp;
		*(int*)(pMvTemp+2) = temp;
		*(int*)(pMvTemp+3) = temp;

		pMvTemp += stride;
	}
	/*
	DWORD *pDestArray = (unsigned long *) &info->mv[0];
	__asm 
	{
	MOV eax, vec;
	MOV eax, [eax];
	MOV ecx, 16;
	REP STOS pDestArray;
	}
	//*/

	Set16MotionVector(IMGPAR curr_mvd[list], curr_mvd);
}

void ReadMotionInfo16x8_DXVA1ATI PARGS2(Pred_s_info *info, int list)
{
	MotionVector vec;
	MotionVector curr_mvd;	
	MotionVector *pMvTemp;
	int refframe;
	int i, temp;
	int stride = IMGPAR PicWidthInMbs * 4;

	if ((currMB_r->b8pdir[0]==list || currMB_r->b8pdir[0]==2) && (currMB_r->b8mode[0] !=0))//has forward vector
	{
		refframe = info->ref_idx[list][0];
		pMvTemp = &info->mv[list][0];		

		IMGPAR subblock_x = 0; // position used for context determination
		IMGPAR subblock_y = 0; // position used for context determination

		// first make mv-prediction
		SetMotionVectorPredictor_block00_shape16x8 ARGS6(&vec, refframe, list, IMGPAR current_mb_nr_r, currMB_r, currMB_s_r);

		// set motion vectors (x, y)	
		// x
		g_read_functions->readMVD ARGS2(list,&curr_mvd);
		// x
		vec.x = vec.x + curr_mvd.x;
		// y
		vec.y = vec.y + curr_mvd.y;

		Set16x8MotionVector(pMvTemp, vec);

		//temp = (DWORD)vec.x & 0x0000ffff;
		//temp |= ((DWORD)vec.y) << 16;

		temp = vec.mv_comb;

		pMvTemp = ((MotionVector*)dec_picture->mv_data1) + (IMGPAR mb_y_r * IMGPAR PicWidthInMbs * 16) + IMGPAR mb_x_r *4;	

		if ((currMB_r->b8pdir[0] == 2) && (list == 0)) {
			pMvTemp -= (IMGPAR PicSizeInMbs << 4);
		}
	
		for ( i = 0; i < 2; i++ ) {
			*(int*)pMvTemp = temp;
			*(int*)(pMvTemp+1) = temp;
			*(int*)(pMvTemp+2) = temp;
			*(int*)(pMvTemp+3) = temp;

			pMvTemp += stride;
		}
		
	} else {
		curr_mvd.mv_comb = 0;
	}

	pMvTemp = &IMGPAR curr_mvd[list][0];
	Set16x8MotionVector(pMvTemp, curr_mvd);

	if ((currMB_r->b8pdir[2]==list || currMB_r->b8pdir[2]==2) && (currMB_r->b8mode[2] !=0))//has forward vector
	{
		refframe = info->ref_idx[list][2];
		pMvTemp = &info->mv[list][8];

		IMGPAR subblock_x = 0; // position used for context determination
		IMGPAR subblock_y = 2; // position used for context determination

		// first make mv-prediction
		SetMotionVectorPredictor_block02_shape16x8 ARGS6(&vec, refframe, list, IMGPAR current_mb_nr_r, currMB_r, currMB_s_r);

		// set motion vectors (x, y)	
		// x
		g_read_functions->readMVD ARGS2(list,&curr_mvd);
		// x
		vec.x = vec.x + curr_mvd.x;
		// y
		vec.y = vec.y + curr_mvd.y;

		Set16x8MotionVector(pMvTemp, vec);
		temp = vec.mv_comb;

		//temp = (DWORD)vec.x & 0x0000ffff;
		//temp |= ((DWORD)vec.y) << 16;

		pMvTemp = ((MotionVector*)dec_picture->mv_data1) + ((IMGPAR mb_y_r * 4 + 2)* stride) + IMGPAR mb_x_r *4;	

		if ((currMB_r->b8pdir[2] == 2) && (list == 0)) {
			pMvTemp -= (IMGPAR PicSizeInMbs << 4);
		}
	
		for ( i = 0; i < 2; i++ ) {
			*(int*)pMvTemp = temp;
			*(int*)(pMvTemp+1) = temp;
			*(int*)(pMvTemp+2) = temp;
			*(int*)(pMvTemp+3) = temp;

			pMvTemp += stride;
		}
		
	}  else {
		curr_mvd.mv_comb = 0;
	}

	pMvTemp = &IMGPAR curr_mvd[list][8];
	Set16x8MotionVector(pMvTemp, curr_mvd);
}

void ReadMotionInfo8x16_DXVA1ATI PARGS2(Pred_s_info *info, int list)
{
	MotionVector vec;
	MotionVector curr_mvd;
	MotionVector *pMvTemp;
	int i;
	int refframe;
	int temp;
	int stride = IMGPAR PicWidthInMbs * 4;
	//char *pb8dir = &(currMB_r->b8pdir[0]);
	//byte *pb8mode = &(currMB_r->b8mode[0]);

	if ((currMB_r->b8pdir[0]==list || currMB_r->b8pdir[0]==2) && (currMB_r->b8mode[0] !=0))//has forward vector
	{
		refframe = info->ref_idx[list][0];
		pMvTemp = &info->mv[list][0];

		IMGPAR subblock_x = 0; // position used for context determination
		IMGPAR subblock_y = 0; // position used for context determination

		// first make mv-prediction
		SetMotionVectorPredictor_block00_shape8x16 ARGS6(&vec, refframe, list, IMGPAR current_mb_nr_r, currMB_r, currMB_s_r);

		// set motion vectors (x, y)	
		// x
		g_read_functions->readMVD ARGS2(list,&curr_mvd);
		// x
		vec.x = vec.x + curr_mvd.x;
		// y
		vec.y = vec.y + curr_mvd.y;

		Set8x16MotionVector(pMvTemp, vec);
		
//		temp = (DWORD)vec.x & 0x0000ffff;
//		temp |= ((DWORD)vec.y) << 16;
		temp = vec.mv_comb;

		pMvTemp = ((MotionVector*)dec_picture->mv_data1) + ((IMGPAR mb_y_r * 4)* stride) + IMGPAR mb_x_r *4;	

		if ((currMB_r->b8pdir[0] == 2) && (list == 0)) {
			pMvTemp -= (IMGPAR PicSizeInMbs << 4);
		}
	
		for ( i = 0; i < 4; i++ ) {
			*(int*)pMvTemp = temp;
			*(int*)(pMvTemp+1) = temp;

			//pMvTemp += IMGPAR PicWidthInMbs * 4;
			pMvTemp += stride;
		}
	/*
		*(int*)pMvTemp		= temp;
		*(int*)(pMvTemp+1)	= temp;

		pMvTemp += stride;

		*(int*)pMvTemp		= temp;
		*(int*)(pMvTemp+1)	= temp;

		pMvTemp += stride;

		*(int*)pMvTemp		= temp;
		*(int*)(pMvTemp+1)	= temp;

		pMvTemp += stride;

		*(int*)pMvTemp		= temp;
		*(int*)(pMvTemp+1)	= temp;	
	*/
	} else {
		curr_mvd.mv_comb = 0;
	}

	pMvTemp = &IMGPAR curr_mvd[list][0];		
	Set8x16MotionVector(pMvTemp, curr_mvd);
	

	if ((currMB_r->b8pdir[1]==list || currMB_r->b8pdir[1]==2) && (currMB_r->b8mode[1] !=0))//has forward vector
	{
		refframe = info->ref_idx[list][1];
		pMvTemp = &info->mv[list][2];

		IMGPAR subblock_x = 2; // position used for context determination
		IMGPAR subblock_y = 0; // position used for context determination

		// first make mv-prediction
		SetMotionVectorPredictor_block20_shape8x16 ARGS6(&vec, refframe, list, IMGPAR current_mb_nr_r, currMB_r, currMB_s_r);

		// set motion vectors (x, y)	
		// x
		g_read_functions->readMVD ARGS2(list,&curr_mvd);
		// x
		vec.x = vec.x + curr_mvd.x;
		// y
		vec.y = vec.y + curr_mvd.y;

		Set8x16MotionVector(pMvTemp, vec);
		temp = (DWORD)vec.x & 0x0000ffff;
		temp |= ((DWORD)vec.y) << 16;

		pMvTemp = ((MotionVector*)dec_picture->mv_data1) + ((IMGPAR mb_y_r * 4)* stride) + IMGPAR mb_x_r *4 + 2;	

		if ((currMB_r->b8pdir[1] == 2) && (list == 0)) {
			pMvTemp -= (IMGPAR PicSizeInMbs << 4);
		}

		for ( i = 0; i < 4; i++ ) {
			*(int*)pMvTemp = temp;
			*(int*)(pMvTemp+1) = temp;

			//pMvTemp += IMGPAR PicWidthInMbs * 4;
			pMvTemp += stride;
		}
/*
		*(int*)pMvTemp = temp;
		*(int*)(pMvTemp+1) = temp;			

		pMvTemp += stride;
		*(int*)pMvTemp = temp;
		*(int*)(pMvTemp+1) = temp;			

		pMvTemp += stride;
		*(int*)pMvTemp = temp;
		*(int*)(pMvTemp+1) = temp;			

		pMvTemp += stride;
		*(int*)pMvTemp = temp;
		*(int*)(pMvTemp+1) = temp;	
*/		
	} else {
		curr_mvd.mv_comb = 0;
	}

	pMvTemp = &IMGPAR curr_mvd[list][2];		
	Set8x16MotionVector(pMvTemp, curr_mvd);
}

void ReadMotionInfo8x8_DXVA1ATI PARGS2(Pred_s_info *info, int list)
{
	MotionVector vec;
	MotionVector curr_mvd;
	MotionVector *pmvd;	
	MotionVector *pMvTemp;
	char *pb8dir = &(currMB_r->b8pdir[0]);
	byte *pb8mode = &(currMB_r->b8mode[0]);
	int refframe;
	int i,j,i0,j0;
	int k;	
	int stride = IMGPAR PicWidthInMbs * 4;
	int a = IMGPAR mb_y_r * 4;
	int b = IMGPAR mb_x_r * 4;

	for (k=0; k<4; k++)
	{
		i0 = (k&1)<<1;
		j0 = (k&2);
		if ((*(pb8dir+k) + list != 1) && (*(pb8mode+k) !=0))//has forward vector or backward vector
		{			
			refframe = info->ref_idx[list][k];

			pMvTemp = ((MotionVector*)dec_picture->mv_data1) + ((a + j0) * stride) + b + i0;								
			if (*(pb8dir+k) - list == 2) {
				pMvTemp -= (IMGPAR PicSizeInMbs << 4);
			}

			switch (*(pb8mode+k)){
				case 4: //(2,2)
					IMGPAR subblock_y = j0; 
					IMGPAR subblock_x = i0;					
					
					SetMotionVectorPredictor ARGS7(&vec, refframe, list, i0, j0, 8, 8);

					g_read_functions->readMVD ARGS2(list,&curr_mvd);			

					vec.x = vec.x + curr_mvd.x;		
					vec.y = vec.y + curr_mvd.y;

					// x, y mvds
					pmvd = &IMGPAR curr_mvd[list][j0*4+i0];
					(pmvd+0)->mv_comb = (pmvd+1)->mv_comb = (pmvd+4)->mv_comb = (pmvd+5)->mv_comb = curr_mvd.mv_comb;

					// x, y vectors
					pmvd = &info->mv[list][j0*4+i0];
					(pmvd+0)->mv_comb = (pmvd+1)->mv_comb = (pmvd+4)->mv_comb = (pmvd+5)->mv_comb = vec.mv_comb;

					*(int*)(pMvTemp+0) = *(int*)(pMvTemp+1) = *(int*)(pMvTemp+stride) = *(int*)(pMvTemp+stride+1) = vec.mv_comb;

					break;
				case 5: //(2,1)

					i = i0;
					j = j0;
					
					IMGPAR subblock_x = i;		
					IMGPAR subblock_y = j; 
					
					SetMotionVectorPredictor ARGS7(&vec, refframe, list, i, j, 8, 4);

					g_read_functions->readMVD ARGS2(list,&curr_mvd);			

					vec.x = vec.x + curr_mvd.x;		
					vec.y = vec.y + curr_mvd.y;

					// x, y mvds
					pmvd = &IMGPAR curr_mvd[list][j*4+i];
					(pmvd+0)->mv_comb = (pmvd+1)->mv_comb = curr_mvd.mv_comb;

					// x, y vectors
					pmvd = &info->mv[list][j*4+i];
					(pmvd+0)->mv_comb = (pmvd+1)->mv_comb = vec.mv_comb;

					*(int*)(pMvTemp+0) = *(int*)(pMvTemp+1) = vec.mv_comb;

					i = i0;
					j = j0+1;
					
					IMGPAR subblock_x = i;		
					IMGPAR subblock_y = j; 
					
					SetMotionVectorPredictor ARGS7(&vec, refframe, list, i, j, 8, 4);

					g_read_functions->readMVD ARGS2(list,&curr_mvd);			

					vec.x = vec.x + curr_mvd.x;		
					vec.y = vec.y + curr_mvd.y;

					// x, y mvds
					pmvd = &IMGPAR curr_mvd[list][j*4+i];
					(pmvd+0)->mv_comb = (pmvd+1)->mv_comb = curr_mvd.mv_comb;

					// x, y vectors
					pmvd = &info->mv[list][j*4+i];
					(pmvd+0)->mv_comb = (pmvd+1)->mv_comb = vec.mv_comb;

					pMvTemp += stride;
					*(int*)(pMvTemp+0) = *(int*)(pMvTemp+1) = vec.mv_comb;

					break;
				case 6: //(1,2)

					i = i0;
					j = j0;
					
					IMGPAR subblock_x = i;		
					IMGPAR subblock_y = j; 
					
					SetMotionVectorPredictor ARGS7(&vec, refframe, list, i, j, 4, 8);

					g_read_functions->readMVD ARGS2(list,&curr_mvd);			

					vec.x = vec.x + curr_mvd.x;		
					vec.y = vec.y + curr_mvd.y;

					// x, y mvds
					pmvd = &IMGPAR curr_mvd[list][j*4+i];
					(pmvd+0)->mv_comb = (pmvd+4)->mv_comb = curr_mvd.mv_comb;

					// x, y vectors
					pmvd = &info->mv[list][j*4+i];
					(pmvd+0)->mv_comb = (pmvd+4)->mv_comb = vec.mv_comb;
					*(int*)(pMvTemp+0) = *(int*)(pMvTemp+stride) = vec.mv_comb;

					i = i0+1;
					j = j0;
					
					IMGPAR subblock_x = i;		
					IMGPAR subblock_y = j; 
					
					SetMotionVectorPredictor ARGS7(&vec, refframe, list, i, j, 4, 8);

					g_read_functions->readMVD ARGS2(list,&curr_mvd);			

					vec.x = vec.x + curr_mvd.x;		
					vec.y = vec.y + curr_mvd.y;

					// x, y mvds
					pmvd = &IMGPAR curr_mvd[list][j*4+i];
					(pmvd+0)->mv_comb = (pmvd+4)->mv_comb = curr_mvd.mv_comb;

					// x, y vectors
					pmvd = &info->mv[list][j*4+i];
					(pmvd+0)->mv_comb = (pmvd+4)->mv_comb = vec.mv_comb;

					pMvTemp++;
					*(int*)(pMvTemp+0) = *(int*)(pMvTemp+stride) = vec.mv_comb;

					break;
				case 7: //(1,1)

					for (j=j0; j<j0+2; j++)
					{
						IMGPAR subblock_y = j;
						for (i=i0; i<i0+2; i++)
						{
							IMGPAR subblock_x = i;

							SetMotionVectorPredictor ARGS7(&vec, refframe, list, i, j, 4, 4);

							g_read_functions->readMVD ARGS2(list,&curr_mvd);			

							vec.x = vec.x + curr_mvd.x;		
							vec.y = vec.y + curr_mvd.y;

							// x, y mvds							
							IMGPAR curr_mvd[list][j*4+i].mv_comb = curr_mvd.mv_comb;

							// x, y vectors							
							info->mv[list][j*4+i].mv_comb = vec.mv_comb;

							*(int*)(pMvTemp) = vec.mv_comb;
							pMvTemp++;
						}

						pMvTemp += (stride - 2);
					}
					break;

				default:
					break;
			}
		} else {
			pmvd = &IMGPAR curr_mvd[list][j0*4+i0];		//Small trick there => k*2 + (k&0x02)*2 => (k + (k&2))*2
			(pmvd+0)->mv_comb = (pmvd+1)->mv_comb = (pmvd+4)->mv_comb = (pmvd+5)->mv_comb = 0;
		}
	}
}
#endif

//#endif
//////////////////////////////////////////////////////////////////////////
