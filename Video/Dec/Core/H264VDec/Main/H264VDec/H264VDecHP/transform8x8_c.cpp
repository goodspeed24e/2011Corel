
/*!
***************************************************************************
* \file transform8x8.c
*
* \brief
*    8x8 transform functions
*
* \author
*    Main contributors (see contributors.h for copyright, address and affiliation details) 
*    - Yuri Vatis          <vatis@hhi.de>
*    - Jan Muenster        <muenster@hhi.de>
*
* \date
*    12. October 2003
**************************************************************************
*/

#include <stdlib.h>
#include "global.h"
#include "transform8x8.h"
#include "clipping.h"

inverse_transform8x8_t *inverse_transform8x8;

#define Q_BITS_8        16
#define DQ_BITS_8       6 
#define DQ_ROUND_8      (1<<(DQ_BITS_8-1))

// Notation for comments regarding prediction and predictors.
// The pels of the 4x4 block are labelled a..p. The predictor pels above
// are labelled A..H, from the left I..P, and from above left X, as follows:
//
//  Z  A  B  C  D  E  F  G  H  I  J  K  L  M   N  O  P  
//  Q  a1 b1 c1 d1 e1 f1 g1 h1
//  R  a2 b2 c2 d2 e2 f2 g2 h2
//  S  a3 b3 c3 d3 e3 f3 g3 h3
//  T  a4 b4 c4 d4 e4 f4 g4 h4
//  U  a5 b5 c5 d5 e5 f5 g5 h5
//  V  a6 b6 c6 d6 e6 f6 g6 h6
//  W  a7 b7 c7 d7 e7 f7 g7 h7
//  X  a8 b8 c8 d8 e8 f8 g8 h8


// Predictor array index definitions
#define P_Z (PredPel[7])
#define P_A (PredPel[8])
#define P_B (PredPel[9])
#define P_C (PredPel[10])
#define P_D (PredPel[11])
#define P_E (PredPel[12])
#define P_F (PredPel[13])
#define P_G (PredPel[14])
#define P_H (PredPel[15])
#define P_I (PredPel[16])
#define P_J (PredPel[17])
#define P_K (PredPel[18])
#define P_L (PredPel[19])
#define P_M (PredPel[20])
#define P_N (PredPel[21])
#define P_O (PredPel[22])
#define P_P (PredPel[23])
#define P_Q (PredPel[24])
#define P_R (PredPel[25])
#define P_S (PredPel[26])
#define P_T (PredPel[27])
#define P_U (PredPel[28])
#define P_V (PredPel[29])
#define P_W (PredPel[30])
#define P_X (PredPel[31])

/*! 
*************************************************************************************
* \brief
*    Prefiltering for Intra8x8 prediction
*************************************************************************************
*/
static void LowPassForIntra8x8Pred PARGS3(int block_up_left, int block_up, int block_left)
{
	int i;
	//static __declspec (align(16)) byte LoopArray[32] GCC_ALIGN(16);

	memcpy(LoopArray, PredPel, sizeof(LoopArray));

	if(block_up_left)
	{
		if(block_up && block_left)
			LoopArray[7] = ((short) P_Q + (P_Z<<1) + P_A +2)>>2;
		else
			if(block_up)
				LoopArray[7] = ((short) P_Z + (P_Z<<1) + P_A +2)>>2;
			else
				if(block_left)
					LoopArray[7] = ((short) P_Z + (P_Z<<1) + P_Q +2)>>2;
	}

	if(block_up)
	{
		if(block_up_left)
			LoopArray[8] = ((short) P_Z + (P_A<<1) + P_B + 2)>>2;
		else
			LoopArray[8] = ((short) P_A + (P_A<<1) + P_B + 2)>>2;

		for(i = 9; i <23; i++)
		{
			LoopArray[i] = ((short) PredPel[i-1] + (PredPel[i]<<1) + PredPel[i+1] + 2)>>2;
		}
		LoopArray[23] = ((short) P_P + (P_P<<1) + P_O + 2)>>2;
	}

	if(block_left)
	{
		if(block_up_left)
			LoopArray[24] = ((short) P_Z + (P_Q<<1) + P_R + 2)>>2;
		else
			LoopArray[24] = ((short) P_Q + (P_Q<<1) + P_R + 2)>>2;

		for(i = 25; i <31; i++)
		{
			LoopArray[i] = ((short) PredPel[i-1] + (PredPel[i]<<1) + PredPel[i+1] + 2)>>2;
		}
		LoopArray[31] = ((short) P_W + (P_X<<1) + P_X + 2)>>2;
	}

	memcpy(PredPel, LoopArray, sizeof(LoopArray));
}



/*!
************************************************************************
* \brief
*    Make intra 8x8 prediction according to all 9 prediction modes.
*    The routine uses left and upper neighbouring points from
*    previous coded blocks to do this (if available). Notice that
*    inaccessible neighbouring points are signalled with a negative
*    value in the predmode array .
*
*  \par Input:
*     Starting point of current 8x8 block image posision
*
************************************************************************
*/
CREL_RETURN intrapred8x8 PARGS1( int b8)
{
	int i,j;
	byte s0;
	//static __declspec (align(16)) byte PredPel[32] GCC_ALIGN(16);  // array of predictor pels
	imgpel *imgY = IMGPAR m_imgY;  // For MB level frame/field coding tools -- set default to imgY
	int stride=dec_picture->Y_stride;

	int mb_nr=IMGPAR current_mb_nr_d;

	PixelPos pix_a;
	PixelPos pix_b, pix_c, pix_d;

	int block_available_up;
	int block_available_left;
	int block_available_up_left;
	int block_available_up_right;
	int block4_x = (b8&1)<<1;
	int block4_y = b8&2;
	int ioff = block4_x<<2;
	int joff = block4_y<<2;
	unsigned char *ptr;
	int left_stride;
	int is_field = IMGPAR MbaffFrameFlag&currMB_d->mb_field;
	imgpel *pSrc_a, *pSrc_b, *pSrc_c, *pSrc_d;		

	byte predmode = currMB_d->ipredmode[joff+block4_x];
	left_stride=stride<<is_field;	

	if (ioff)
	{
		//getCurrNeighbourInside_Luma ARGS4(mb_nr, ioff-1, joff,   &pix_a);
		pix_a.pMB = currMB_d;		
		pSrc_a = &IMGPAR mpr[joff][(ioff-1)];
		left_stride=16;
		block_available_left = 1;

		if (joff)
		{
			//getCurrNeighbourInside_Luma ARGS4(mb_nr, ioff, joff-1, &pix_b);
			pix_b.pMB = currMB_d;
			pSrc_b = &IMGPAR mpr[(joff-1)][ioff];

			//getCurrNeighbourInside_Luma ARGS4(mb_nr, (ioff-1), (joff-1), &pix_d);		
			pix_d.pMB = currMB_d;
			pSrc_d = &IMGPAR mpr[(joff-1)][(ioff-1)];

			pix_c.pMB = NULL;
		}
		else
		{
			getCurrNeighbourB_Luma ARGS3(ioff, joff-1, &pix_b);
			getCurrNeighbourB_Luma ARGS3(ioff-1, joff-1, &pix_d);
			getCurrNeighbourC_Luma ARGS3(ioff+8, joff-1, &pix_c);
			pSrc_b = imgY+pix_b.y*stride+pix_b.x;
			pSrc_c = imgY+pix_c.y*stride+pix_c.x;	
			pSrc_d = imgY+pix_d.y*stride+pix_d.x;
		}
	}
	else
	{
		getCurrNeighbourA_Luma ARGS3(ioff-1, joff,   &pix_a);
		pSrc_a = imgY+pix_a.y*stride+pix_a.x;	

		if(pix_a.pMB)
		{
			block_available_left = 1;	

			if (active_pps.constrained_intra_pred_flag)
			{
				if (IMGPAR MbaffFrameFlag)
				{									
					switch(currMB_d->mbStatusA)
					{
					case 1:					
					case 2:
						block_available_left = (currMB_d->mbIntraA==0x03);
						break;							
					default:
						break;
					}			
				}				
			}
		}
		else
			block_available_left = 0;


		if (joff)
		{
			//getCurrNeighbourInside_Luma ARGS4(mb_nr, ioff, joff-1, &pix_b);
			pix_b.pMB = currMB_d;
			pSrc_b = &IMGPAR mpr[(joff-1)][ioff];

			getCurrNeighbourA_Luma ARGS3(ioff-1, joff-1, &pix_d);
			pSrc_d = imgY+pix_d.y*stride+pix_d.x;			

			//getCurrNeighbourInside_Luma ARGS4(mb_nr, ioff+8, joff-1, &pix_c);
			pix_c.pMB = currMB_d;
			pSrc_c = &IMGPAR mpr[(joff-1)][ioff+8];
		}
		else
		{
			getCurrNeighbourB_Luma ARGS3(ioff, joff-1, &pix_b);
			getCurrNeighbourD_Luma ARGS3(ioff-1, joff-1, &pix_d);
			getCurrNeighbourB_Luma ARGS3(ioff+8, joff-1, &pix_c);
			pSrc_b = imgY+pix_b.y*stride+pix_b.x;
			pSrc_c = imgY+pix_c.y*stride+pix_c.x;	
			pSrc_d = imgY+pix_d.y*stride+pix_d.x;
		}
	}	

	block_available_up       = (pix_b.pMB!=NULL);
	block_available_up_right = (pix_c.pMB!=NULL);
	block_available_up_left  = (pix_d.pMB!=NULL);

	//  *left_available = block_available_left;
	//  *up_available   = block_available_up;
	//  *all_available  = block_available_up && block_available_left && block_available_up_left;

	// form predictor pels

	if (block_available_up)
	{
		ptr = pSrc_b;

		P_A = ptr[0];
		P_B = ptr[1];
		P_C = ptr[2];
		P_D = ptr[3];
		P_E = ptr[4];
		P_F = ptr[5];
		P_G = ptr[6];
		P_H = ptr[7];
	}
	else
	{
		P_A = P_B = P_C = P_D = P_E = P_F = P_G = P_H = 128; // HP restriction
	}

	if (block_available_up_right)
	{
		ptr = pSrc_c;

		P_I = ptr[0];
		P_J = ptr[1];
		P_K = ptr[2];
		P_L = ptr[3];
		P_M = ptr[4];
		P_N = ptr[5];
		P_O = ptr[6];
		P_P = ptr[7];

	}
	else
	{
		P_I = P_J = P_K = P_L = P_M = P_N = P_O = P_P = P_H;
	}

	if (block_available_left)
	{
		ptr = pSrc_a;		
		P_Q = ptr[0*left_stride];
		P_R = ptr[1*left_stride];
		P_S = ptr[2*left_stride];
		P_T = ptr[3*left_stride];
		P_U = ptr[4*left_stride];
		P_V = ptr[5*left_stride];
		P_W = ptr[6*left_stride];
		P_X = ptr[7*left_stride];
	}
	else
	{
		P_Q = P_R = P_S = P_T = P_U = P_V = P_W = P_X = 128; // HP restriction
	}

	if (block_available_up_left)
	{
		P_Z = *(pSrc_d);
	}
	else
	{
		P_Z = 128; // HP restriction
	}

	LowPassForIntra8x8Pred ARGS3(block_available_up_left, block_available_up, block_available_left);

	ptr = &IMGPAR mpr[joff][ioff];

	switch(predmode)
	{
	case DC_PRED:
		s0 = 0;
		if (block_available_up && block_available_left)
		{   
			// no edge
			s0 = ((short) P_A + P_B + P_C + P_D + P_E + P_F + P_G + P_H + P_Q + P_R + P_S + P_T + P_U + P_V + P_W + P_X + 8) >> 4;
		}
		else if (!block_available_up && block_available_left)
		{
			// upper edge
			s0 = ((short) P_Q + P_R + P_S + P_T + P_U + P_V + P_W + P_X + 4) >> 3;             
		}
		else if (block_available_up && !block_available_left)
		{
			// left edge
			s0 = ((short) P_A + P_B + P_C + P_D + P_E + P_F + P_G + P_H + 4) >> 3;             
		}
		else //if (!block_available_up && !block_available_left)
		{
			// top left corner, nothing to predict from
			s0 = 128; // HP restriction
		}
		for(j = 0; j < 2*BLOCK_SIZE; j++)
		{
			ptr[0] = ptr[1] = ptr[2] = ptr[3] = ptr[4] = ptr[5] = ptr[6] = ptr[7] = s0;
			ptr += 16;
		}
		break;

	case VERT_PRED:
		if (!block_available_up) {
			DEBUG_SHOW_SW_INFO ("warning: Intra_8x8_Vertical prediction mode not allowed at mb %d\n",IMGPAR current_mb_nr_d);
			return CREL_WARNING_H264_STREAMERROR_LEVEL_3;
		}

		for (i=0; i < 2*BLOCK_SIZE; i++)
		{
			ptr[i] = ptr[16+i] = ptr[32+i] = ptr[48+i] = ptr[64+i] = ptr[80+i] = ptr[96+i] = ptr[112+i] = (&P_A)[i];
		}
		break;
	case HOR_PRED:
		if (!block_available_left) {
			DEBUG_SHOW_SW_INFO ("warning: Intra_8x8_Horizontal prediction mode not allowed at mb %d\n",IMGPAR current_mb_nr_d);
			return CREL_WARNING_H264_STREAMERROR_LEVEL_3;
		}

		for (j=0; j < 2*BLOCK_SIZE; j++)
		{
			ptr[0] = ptr[1] = ptr[2] = ptr[3] = ptr[4] = ptr[5] = ptr[6] = ptr[7] = (&P_Q)[j];
			ptr += 16;
		}
		break;

	case DIAG_DOWN_LEFT_PRED:
		if (!block_available_up) {
			DEBUG_SHOW_SW_INFO ("warning: Intra_8x8_Diagonal_Down_Left prediction mode not allowed at mb %d\n",IMGPAR current_mb_nr_d);
			return CREL_WARNING_H264_STREAMERROR_LEVEL_3;
		}
		// Mode DIAG_DOWN_LEFT_PRED
		ptr[0*16+0] = ((short) P_A + (P_B<<1) + P_C + 2) >> 2;
		ptr[1*16+0] = 
			ptr[0*16+1] = ((short) P_B + (P_C<<1) + P_D + 2) >> 2;
		ptr[2*16+0] =
			ptr[1*16+1] =
			ptr[0*16+2] = ((short) P_C + (P_D<<1) + P_E + 2) >> 2;
		ptr[3*16+0] = 
			ptr[2*16+1] = 
			ptr[1*16+2] = 
			ptr[0*16+3] = ((short) P_D + (P_E<<1) + P_F + 2) >> 2;
		ptr[4*16+0] = 
			ptr[3*16+1] = 
			ptr[2*16+2] = 
			ptr[1*16+3] = 
			ptr[0*16+4] = ((short) P_E + (P_F<<1) + P_G + 2) >> 2;
		ptr[5*16+0] = 
			ptr[4*16+1] = 
			ptr[3*16+2] = 
			ptr[2*16+3] = 
			ptr[1*16+4] = 
			ptr[0*16+5] = ((short) P_F + (P_G<<1) + P_H + 2) >> 2;
		ptr[6*16+0] = 
			ptr[5*16+1] = 
			ptr[4*16+2] = 
			ptr[3*16+3] = 
			ptr[2*16+4] = 
			ptr[1*16+5] = 
			ptr[0*16+6] = ((short) P_G + (P_H<<1) + P_I + 2) >> 2;
		ptr[7*16+0] = 
			ptr[6*16+1] = 
			ptr[5*16+2] = 
			ptr[4*16+3] = 
			ptr[3*16+4] = 
			ptr[2*16+5] = 
			ptr[1*16+6] = 
			ptr[0*16+7] = ((short) P_H + (P_I<<1) + P_J + 2) >> 2;
		ptr[7*16+1] = 
			ptr[6*16+2] = 
			ptr[5*16+3] = 
			ptr[4*16+4] = 
			ptr[3*16+5] = 
			ptr[2*16+6] = 
			ptr[1*16+7] = ((short) P_I + (P_J<<1) + P_K + 2) >> 2;
		ptr[7*16+2] = 
			ptr[6*16+3] = 
			ptr[5*16+4] = 
			ptr[4*16+5] = 
			ptr[3*16+6] = 
			ptr[2*16+7] = ((short) P_J + (P_K<<1) + P_L + 2) >> 2;
		ptr[7*16+3] = 
			ptr[6*16+4] = 
			ptr[5*16+5] = 
			ptr[4*16+6] = 
			ptr[3*16+7] = ((short) P_K + (P_L<<1) + P_M + 2) >> 2;
		ptr[7*16+4] = 
			ptr[6*16+5] = 
			ptr[5*16+6] = 
			ptr[4*16+7] = ((short) P_L + (P_M<<1) + P_N + 2) >> 2;
		ptr[7*16+5] = 
			ptr[6*16+6] = 
			ptr[5*16+7] = ((short) P_M + (P_N<<1) + P_O + 2) >> 2;
		ptr[7*16+6] = 
			ptr[6*16+7] = ((short) P_N + (P_O<<1) + P_P + 2) >> 2;
		ptr[7*16+7] = ((short) P_O + (P_P<<1) + P_P + 2) >> 2;
		break;

	case VERT_LEFT_PRED:
		if (!block_available_up) {
			DEBUG_SHOW_SW_INFO ("warning: Intra_4x4_Vertical_Left prediction mode not allowed at mb %d\n",IMGPAR current_mb_nr_d);
			return CREL_WARNING_H264_STREAMERROR_LEVEL_3;
		}

		ptr[0*16+0] = ((short) P_A + P_B + 1) >> 1;
		ptr[0*16+1] = 
			ptr[2*16+0] = ((short) P_B + P_C + 1) >> 1;
		ptr[0*16+2] = 
			ptr[2*16+1] = 
			ptr[4*16+0] = ((short) P_C + P_D + 1) >> 1;
		ptr[0*16+3] = 
			ptr[2*16+2] = 
			ptr[4*16+1] = 
			ptr[6*16+0] = ((short) P_D + P_E + 1) >> 1;
		ptr[0*16+4] = 
			ptr[2*16+3] = 
			ptr[4*16+2] = 
			ptr[6*16+1] = ((short) P_E + P_F + 1) >> 1;
		ptr[0*16+5] = 
			ptr[2*16+4] = 
			ptr[4*16+3] = 
			ptr[6*16+2] = ((short) P_F + P_G + 1) >> 1;
		ptr[0*16+6] = 
			ptr[2*16+5] = 
			ptr[4*16+4] = 
			ptr[6*16+3] = ((short) P_G + P_H + 1) >> 1;
		ptr[0*16+7] = 
			ptr[2*16+6] = 
			ptr[4*16+5] = 
			ptr[6*16+4] = ((short) P_H + P_I + 1) >> 1;
		ptr[2*16+7] = 
			ptr[4*16+6] = 
			ptr[6*16+5] = ((short) P_I + P_J + 1) >> 1;
		ptr[4*16+7] = 
			ptr[6*16+6] = ((short) P_J + P_K + 1) >> 1;
		ptr[6*16+7] = ((short) P_K + P_L + 1) >> 1;
		ptr[1*16+0] = ((short) P_A + (P_B<<1) + P_C + 2) >> 2;
		ptr[1*16+1] = 
			ptr[3*16+0] = ((short) P_B + (P_C<<1) + P_D + 2) >> 2;
		ptr[1*16+2] = 
			ptr[3*16+1] = 
			ptr[5*16+0] = ((short) P_C + (P_D<<1) + P_E + 2) >> 2;
		ptr[1*16+3] = 
			ptr[3*16+2] = 
			ptr[5*16+1] = 
			ptr[7*16+0] = ((short) P_D + (P_E<<1) + P_F + 2) >> 2;
		ptr[1*16+4] = 
			ptr[3*16+3] = 
			ptr[5*16+2] = 
			ptr[7*16+1] = ((short) P_E + (P_F<<1) + P_G + 2) >> 2;
		ptr[1*16+5] = 
			ptr[3*16+4] = 
			ptr[5*16+3] = 
			ptr[7*16+2] = ((short) P_F + (P_G<<1) + P_H + 2) >> 2;
		ptr[1*16+6] = 
			ptr[3*16+5] = 
			ptr[5*16+4] = 
			ptr[7*16+3] = ((short) P_G + (P_H<<1) + P_I + 2) >> 2;
		ptr[1*16+7] = 
			ptr[3*16+6] = 
			ptr[5*16+5] = 
			ptr[7*16+4] = ((short) P_H + (P_I<<1) + P_J + 2) >> 2;
		ptr[3*16+7] = 
			ptr[5*16+6] = 
			ptr[7*16+5] = ((short) P_I + (P_J<<1) + P_K + 2) >> 2;
		ptr[5*16+7] = 
			ptr[7*16+6] = ((short) P_J + (P_K<<1) + P_L + 2) >> 2;
		ptr[7*16+7] = ((short) P_K + (P_L<<1) + P_M + 2) >> 2;
		break;


	case DIAG_DOWN_RIGHT_PRED:
		if ((!block_available_up)||(!block_available_left)||(!block_available_up_left)) {
			DEBUG_SHOW_SW_INFO ("warning: Intra_8x8_Diagonal_Down_Right prediction mode not allowed at mb %d\n",IMGPAR current_mb_nr_d);
			return CREL_WARNING_H264_STREAMERROR_LEVEL_3;
		}

		// Mode DIAG_DOWN_RIGHT_PRED
		ptr[7*16+0] = ((short) P_V + (P_W<<1) + P_X + 2) >> 2;
		ptr[6*16+0] = 
			ptr[7*16+1] = ((short) P_U + (P_V<<1) + P_W + 2) >> 2;
		ptr[5*16+0] = 
			ptr[6*16+1] = 
			ptr[7*16+2] = ((short) P_T + (P_U<<1) + P_V + 2) >> 2;
		ptr[4*16+0] = 
			ptr[5*16+1] = 
			ptr[6*16+2] = 
			ptr[7*16+3] = ((short) P_S + (P_T<<1) + P_U + 2) >> 2;
		ptr[3*16+0] = 
			ptr[4*16+1] = 
			ptr[5*16+2] = 
			ptr[6*16+3] = 
			ptr[7*16+4] = ((short) P_R + (P_S<<1) + P_T + 2) >> 2;
		ptr[2*16+0] = 
			ptr[3*16+1] = 
			ptr[4*16+2] = 
			ptr[5*16+3] = 
			ptr[6*16+4] = 
			ptr[7*16+5] = ((short) P_Q + (P_R<<1) + P_S + 2) >> 2;
		ptr[1*16+0] = 
			ptr[2*16+1] = 
			ptr[3*16+2] = 
			ptr[4*16+3] = 
			ptr[5*16+4] = 
			ptr[6*16+5] = 
			ptr[7*16+6] = ((short) P_Z + (P_Q<<1) + P_R + 2) >> 2;
		ptr[0*16+0] = 
			ptr[1*16+1] = 
			ptr[2*16+2] = 
			ptr[3*16+3] = 
			ptr[4*16+4] = 
			ptr[5*16+5] = 
			ptr[6*16+6] = 
			ptr[7*16+7] = ((short) P_Q + (P_Z<<1) + P_A + 2) >> 2;
		ptr[0*16+1] = 
			ptr[1*16+2] = 
			ptr[2*16+3] = 
			ptr[3*16+4] = 
			ptr[4*16+5] = 
			ptr[5*16+6] = 
			ptr[6*16+7] = ((short) P_Z + (P_A<<1) + P_B + 2) >> 2;
		ptr[0*16+2] = 
			ptr[1*16+3] = 
			ptr[2*16+4] = 
			ptr[3*16+5] = 
			ptr[4*16+6] = 
			ptr[5*16+7] = ((short) P_A + (P_B<<1) + P_C + 2) >> 2;
		ptr[0*16+3] = 
			ptr[1*16+4] = 
			ptr[2*16+5] = 
			ptr[3*16+6] = 
			ptr[4*16+7] = ((short) P_B + (P_C<<1) + P_D + 2) >> 2;
		ptr[0*16+4] = 
			ptr[1*16+5] = 
			ptr[2*16+6] = 
			ptr[3*16+7] = ((short) P_C + (P_D<<1) + P_E + 2) >> 2;
		ptr[0*16+5] = 
			ptr[1*16+6] = 
			ptr[2*16+7] = ((short) P_D + (P_E<<1) + P_F + 2) >> 2;
		ptr[0*16+6] = 
			ptr[1*16+7] = ((short) P_E + (P_F<<1) + P_G + 2) >> 2;
		ptr[0*16+7] = ((short) P_F + (P_G<<1) + P_H + 2) >> 2;
		break;

	case  VERT_RIGHT_PRED:/* diagonal prediction -22.5 deg to horizontal plane */
		if ((!block_available_up)||(!block_available_left)||(!block_available_up_left)) {
			DEBUG_SHOW_SW_INFO ("warning: Intra_8x8_Vertical_Right prediction mode not allowed at mb %d\n",IMGPAR current_mb_nr_d);
			return CREL_WARNING_H264_STREAMERROR_LEVEL_3;
		}

		ptr[0*16+0] = 
			ptr[2*16+1] = 
			ptr[4*16+2] = 
			ptr[6*16+3] = ((short) P_Z + P_A + 1) >> 1;
		ptr[0*16+1] = 
			ptr[2*16+2] = 
			ptr[4*16+3] = 
			ptr[6*16+4] = ((short) P_A + P_B + 1) >> 1;
		ptr[0*16+2] = 
			ptr[2*16+3] = 
			ptr[4*16+4] = 
			ptr[6*16+5] = ((short) P_B + P_C + 1) >> 1;
		ptr[0*16+3] = 
			ptr[2*16+4] = 
			ptr[4*16+5] = 
			ptr[6*16+6] = ((short) P_C + P_D + 1) >> 1;
		ptr[0*16+4] = 
			ptr[2*16+5] = 
			ptr[4*16+6] = 
			ptr[6*16+7] = ((short) P_D + P_E + 1) >> 1;
		ptr[0*16+5] = 
			ptr[2*16+6] = 
			ptr[4*16+7] = ((short) P_E + P_F + 1) >> 1;
		ptr[0*16+6] = 
			ptr[2*16+7] = ((short) P_F + P_G + 1) >> 1;
		ptr[0*16+7] = ((short) P_G + P_H + 1) >> 1;
		ptr[1*16+0] = 
			ptr[3*16+1] = 
			ptr[5*16+2] = 
			ptr[7*16+3] = ((short) P_Q + (P_Z<<1) + P_A + 2) >> 2;
		ptr[1*16+1] = 
			ptr[3*16+2] = 
			ptr[5*16+3] = 
			ptr[7*16+4] = ((short) P_Z + (P_A<<1) + P_B + 2) >> 2;
		ptr[1*16+2] = 
			ptr[3*16+3] = 
			ptr[5*16+4] = 
			ptr[7*16+5] = ((short) P_A + (P_B<<1) + P_C + 2) >> 2;
		ptr[1*16+3] = 
			ptr[3*16+4] = 
			ptr[5*16+5] = 
			ptr[7*16+6] = ((short) P_B + (P_C<<1) + P_D + 2) >> 2;
		ptr[1*16+4] = 
			ptr[3*16+5] = 
			ptr[5*16+6] = 
			ptr[7*16+7] = ((short) P_C + (P_D<<1) + P_E + 2) >> 2;
		ptr[1*16+5] = 
			ptr[3*16+6] = 
			ptr[5*16+7] = ((short) P_D + (P_E<<1) + P_F + 2) >> 2;
		ptr[1*16+6] = 
			ptr[3*16+7] = ((short) P_E + (P_F<<1) + P_G + 2) >> 2;
		ptr[1*16+7] = ((short) P_F + (P_G<<1) + P_H + 2) >> 2;
		ptr[2*16+0] =
			ptr[4*16+1] =
			ptr[6*16+2] = ((short) P_Z + (P_Q<<1) + P_R + 2) >> 2;
		ptr[3*16+0] =
			ptr[5*16+1] =
			ptr[7*16+2] = ((short) P_Q + (P_R<<1) + P_S + 2) >> 2;
		ptr[4*16+0] =
			ptr[6*16+1] = ((short) P_R + (P_S<<1) + P_T + 2) >> 2;
		ptr[5*16+0] =
			ptr[7*16+1] = ((short) P_S + (P_T<<1) + P_U + 2) >> 2;
		ptr[6*16+0] = ((short) P_T + (P_U<<1) + P_V + 2) >> 2;
		ptr[7*16+0] = ((short) P_U + (P_V<<1) + P_W + 2) >> 2;
		break;

	case  HOR_DOWN_PRED:/* diagonal prediction -22.5 deg to horizontal plane */
		if ((!block_available_up)||(!block_available_left)||(!block_available_up_left)) {
			DEBUG_SHOW_SW_INFO ("warning: Intra_8x8_Horizontal_Down prediction mode not allowed at mb %d\n",IMGPAR current_mb_nr_d);
			return CREL_WARNING_H264_STREAMERROR_LEVEL_3;
		}

		ptr[0*16+0] = 
			ptr[1*16+2] = 
			ptr[2*16+4] = 
			ptr[3*16+6] = ((short) P_Z + P_Q + 1) >> 1;
		ptr[1*16+0] = 
			ptr[2*16+2] = 
			ptr[3*16+4] = 
			ptr[4*16+6] = ((short) P_Q + P_R + 1) >> 1;
		ptr[2*16+0] = 
			ptr[3*16+2] = 
			ptr[4*16+4] = 
			ptr[5*16+6] = ((short) P_R + P_S + 1) >> 1;
		ptr[3*16+0] = 
			ptr[4*16+2] = 
			ptr[5*16+4] = 
			ptr[6*16+6] = ((short) P_S + P_T + 1) >> 1;
		ptr[4*16+0] = 
			ptr[5*16+2] = 
			ptr[6*16+4] = 
			ptr[7*16+6] = ((short) P_T + P_U + 1) >> 1;
		ptr[5*16+0] = 
			ptr[6*16+2] = 
			ptr[7*16+4] = ((short) P_U + P_V + 1) >> 1;
		ptr[6*16+0] = 
			ptr[7*16+2] = ((short) P_V + P_W + 1) >> 1;
		ptr[7*16+0] = ((short) P_W + P_X + 1) >> 1;
		ptr[0*16+1] =
			ptr[1*16+3] =
			ptr[2*16+5] =
			ptr[3*16+7] = ((short) P_A + (P_Z<<1) + P_Q + 2) >> 2;
		ptr[1*16+1] =
			ptr[2*16+3] =
			ptr[3*16+5] =
			ptr[4*16+7] = ((short) P_Z + (P_Q<<1) + P_R + 2) >> 2;
		ptr[2*16+1] =
			ptr[3*16+3] =
			ptr[4*16+5] =
			ptr[5*16+7] = ((short) P_Q + (P_R<<1) + P_S + 2) >> 2;
		ptr[3*16+1] =
			ptr[4*16+3] =
			ptr[5*16+5] =
			ptr[6*16+7] = ((short) P_R + (P_S<<1) + P_T + 2) >> 2;
		ptr[4*16+1] =
			ptr[5*16+3] =
			ptr[6*16+5] =
			ptr[7*16+7] = ((short) P_S + (P_T<<1) + P_U + 2) >> 2;
		ptr[5*16+1] =
			ptr[6*16+3] =
			ptr[7*16+5] = ((short) P_T + (P_U<<1) + P_V + 2) >> 2;
		ptr[6*16+1] =
			ptr[7*16+3] = ((short) P_U + (P_V<<1) + P_W + 2) >> 2;
		ptr[7*16+1] = ((short) P_V + (P_W<<1) + P_X + 2) >> 2;
		ptr[0*16+2] = 
			ptr[1*16+4] = 
			ptr[2*16+6] = ((short) P_Z + (P_A<<1) + P_B + 2) >> 2;
		ptr[0*16+3] = 
			ptr[1*16+5] = 
			ptr[2*16+7] = ((short) P_A + (P_B<<1) + P_C + 2) >> 2;
		ptr[0*16+4] = 
			ptr[1*16+6] = ((short) P_B + (P_C<<1) + P_D + 2) >> 2;
		ptr[0*16+5] = 
			ptr[1*16+7] = ((short) P_C + (P_D<<1) + P_E + 2) >> 2;
		ptr[0*16+6] = ((short) P_D + (P_E<<1) + P_F + 2) >> 2;
		ptr[0*16+7] = ((short) P_E + (P_F<<1) + P_G + 2) >> 2;
		break;

	case  HOR_UP_PRED:/* diagonal prediction -22.5 deg to horizontal plane */
		if (!block_available_left) {
			DEBUG_SHOW_SW_INFO ("warning: Intra_8x8_Horizontal_Up prediction mode not allowed at mb %d\n",IMGPAR current_mb_nr_d);
			return CREL_WARNING_H264_STREAMERROR_LEVEL_3;
		}

		ptr[0*16+0] = ((short) P_Q + P_R + 1) >> 1;
		ptr[1*16+0] =
			ptr[0*16+2] = ((short) P_R + P_S + 1) >> 1;
		ptr[2*16+0] =
			ptr[1*16+2] =
			ptr[0*16+4] = ((short) P_S + P_T + 1) >> 1;
		ptr[3*16+0] =
			ptr[2*16+2] =
			ptr[1*16+4] =
			ptr[0*16+6] = ((short) P_T + P_U + 1) >> 1;
		ptr[4*16+0] =
			ptr[3*16+2] =
			ptr[2*16+4] =
			ptr[1*16+6] = ((short) P_U + P_V + 1) >> 1;
		ptr[5*16+0] =
			ptr[4*16+2] =
			ptr[3*16+4] =
			ptr[2*16+6] = ((short) P_V + P_W + 1) >> 1;
		ptr[6*16+0] =
			ptr[5*16+2] =
			ptr[4*16+4] =
			ptr[3*16+6] = ((short) P_W + P_X + 1) >> 1;
		ptr[4*16+6] =
			ptr[4*16+7] =
			ptr[5*16+4] =
			ptr[5*16+5] =
			ptr[5*16+6] =
			ptr[5*16+7] =
			ptr[6*16+2] =
			ptr[6*16+3] =
			ptr[6*16+4] =
			ptr[6*16+5] =
			ptr[6*16+6] =
			ptr[6*16+7] =
			ptr[7*16+0] =
			ptr[7*16+1] =
			ptr[7*16+2] =
			ptr[7*16+3] =
			ptr[7*16+4] =
			ptr[7*16+5] =
			ptr[7*16+6] =
			ptr[7*16+7] = P_X;
		ptr[6*16+1] =
			ptr[5*16+3] =
			ptr[4*16+5] =
			ptr[3*16+7] = ((short) P_X + (P_X<<1) + P_W + 2) >> 2;
		ptr[5*16+1] =
			ptr[4*16+3] =
			ptr[3*16+5] =
			ptr[2*16+7] = ((short) P_V + (P_W<<1) + P_X + 2) >> 2;
		ptr[4*16+1] =
			ptr[3*16+3] =
			ptr[2*16+5] =
			ptr[1*16+7] = ((short) P_U + (P_V<<1) + P_W + 2) >> 2;
		ptr[3*16+1] =
			ptr[2*16+3] =
			ptr[1*16+5] =
			ptr[0*16+7] = ((short) P_T + (P_U<<1) + P_V + 2) >> 2;
		ptr[2*16+1] =
			ptr[1*16+3] =
			ptr[0*16+5] = ((short) P_S + (P_T<<1) + P_U + 2) >> 2;
		ptr[1*16+1] =
			ptr[0*16+3] = ((short) P_R + (P_S<<1) + P_T + 2) >> 2;
		ptr[0*16+1] = ((short) P_Q + (P_R<<1) + P_S + 2) >> 2;
		break;

	default:
		DEBUG_SHOW_ERROR_INFO("Error: illegal intra_4x4 prediction mode: %d\n",predmode);
		return CREL_WARNING_H264_STREAMERROR_LEVEL_3;
		break;
	}
	return CREL_OK;
}



void inverse_transform8x8_c PARGS4(byte *dest, byte *pred, short *src, int stride)
{
	int i;
	short m7[64];
	short m8[64];
	short a[8], b[8];
	short * mm, * mm_stop;
	short x0, x1, x2, x3, x4, x5, x6, x7;
	static const unsigned short R = ~((short) 255);

#if defined(_PRE_TRANSPOSE_)
	for(i=0; i<8; i++)                                    
	{
		m8[0*8+i] = src[0];
		m8[1*8+i] = src[1];
		m8[2*8+i] = src[2];
		m8[3*8+i] = src[3];
		m8[4*8+i] = src[4];
		m8[5*8+i] = src[5];
		m8[6*8+i] = src[6];
		m8[7*8+i] = src[7];

		src+=8;
	}

	src-=64;

	// horizontal
	for(i=0, mm = m7; i < 8; i++,src+=8, mm++)
	{


		a[0] =  m8[0+i*8]+m8[4+i*8];                          
		a[4] =  m8[0+i*8]-m8[4+i*8];
		a[2] = (m8[2+i*8]>>1)-m8[6+i*8];
		a[6] =  m8[2+i*8]+(m8[6+i*8]>>1);
		a[1] = -m8[3+i*8]+m8[5+i*8]-m8[7+i*8]-(m8[7+i*8]>>1);
		a[3] =  m8[1+i*8]+m8[7+i*8]-m8[3+i*8]-(m8[3+i*8]>>1);
		a[5] = -m8[1+i*8]+m8[7+i*8]+m8[5+i*8]+(m8[5+i*8]>>1);
		a[7] =  m8[3+i*8]+m8[5+i*8]+m8[1+i*8]+(m8[1+i*8]>>1);



		b[0] = a[0] + a[6];
		b[2] = a[4] + a[2];
		b[4] = a[4] - a[2];
		b[6] = a[0] - a[6];
		b[1] = a[1] + (a[7]>>2);
		b[7] = -(a[1]>>2) + a[7];
		b[3] = a[3] + (a[5]>>2);
		b[5] = (a[3]>>2) - a[5];

		mm[0]  = b[0] + b[7];
		mm[8]  = b[2] + b[5];
		mm[16] = b[4] + b[3];
		mm[24] = b[6] + b[1];
		mm[32] = b[6] - b[1];
		mm[40] = b[4] - b[3];
		mm[48] = b[2] - b[5];
		mm[56] = b[0] - b[7];
	}
#else
	//horizontal
	for(i=0, mm = m7; i < 8; i++, mm++)
	{
      
		a[0] =  src[0+i*8]+src[4+i*8];                          
		a[4] =  src[0+i*8]-src[4+i*8];
		a[2] = (src[2+i*8]>>1)-src[6+i*8];
		a[6] =  src[2+i*8]+(src[6+i*8]>>1);
		a[1] = -src[3+i*8]+src[5+i*8]-src[7+i*8]-(src[7+i*8]>>1);
		a[3] =  src[1+i*8]+src[7+i*8]-src[3+i*8]-(src[3+i*8]>>1);
		a[5] = -src[1+i*8]+src[7+i*8]+src[5+i*8]+(src[5+i*8]>>1);
		a[7] =  src[3+i*8]+src[5+i*8]+src[1+i*8]+(src[1+i*8]>>1);
	
		b[0] = a[0] + a[6];
		b[2] = a[4] + a[2];
		b[4] = a[4] - a[2];
		b[6] = a[0] - a[6];
		b[1] = a[1] + (a[7]>>2);
		b[7] = -(a[1]>>2) + a[7];
		b[3] = a[3] + (a[5]>>2);
		b[5] = (a[3]>>2) - a[5];

		mm[0]  = b[0] + b[7];
		mm[8]  = b[2] + b[5];
		mm[16] = b[4] + b[3];
		mm[24] = b[6] + b[1];
		mm[32] = b[6] - b[1];
		mm[40] = b[4] - b[3];
		mm[48] = b[2] - b[5];
		mm[56] = b[0] - b[7];
	}
	src+=64;
#endif
	memset(src-64, 0, 8*8*sizeof(short));

	// vertical
	for (mm = m7, mm_stop = m7 + 64; mm < mm_stop; mm += 8)
	{
		a[0] =  mm[0] + mm[4];
		a[4] =  mm[0] - mm[4];
		a[2] = (mm[2]>>1) - mm[6];
		a[6] =  mm[2] + (mm[6]>>1);
		a[1] = -mm[3] + mm[5] - mm[7] - (mm[7]>>1);
		a[3] =  mm[1] + mm[7] - mm[3] - (mm[3]>>1);
		a[5] = -mm[1] + mm[7] + mm[5] + (mm[5]>>1);
		a[7] =  mm[3] + mm[5] + mm[1] + (mm[1]>>1);

		b[0] = a[0] + a[6];
		b[2] = a[4] + a[2];
		b[4] = a[4] - a[2];
		b[6] = a[0] - a[6];
		b[1] = a[1] + (a[7]>>2);
		b[7] = -(a[1]>>2) + a[7];
		b[3] = a[3] + (a[5]>>2);
		b[5] = (a[3]>>2) - a[5];


		mm[0] = b[0] + b[7];
		mm[1] = b[2] + b[5];
		mm[2] = b[4] + b[3];
		mm[3] = b[6] + b[1];
		mm[4] = b[6] - b[1];
		mm[5] = b[4] - b[3];
		mm[6] = b[2] - b[5];
		mm[7] = b[0] - b[7];


		x0=((mm[0]+32)>>6)+ (short)pred[0];              
		x1=((mm[1]+32)>>6)+ (short)pred[16];
		x2=((mm[2]+32)>>6)+ (short)pred[32]; 
		x3=((mm[3]+32)>>6)+ (short)pred[48];
		x4=((mm[4]+32)>>6)+ (short)pred[64]; 
		x5=((mm[5]+32)>>6)+ (short)pred[80];
		x6=((mm[6]+32)>>6)+ (short)pred[96]; 
		x7=((mm[7]+32)>>6)+ (short)pred[112];
		


		if (!((x0|x1|x2|x3|x4|x5|x6|x7)&R))
		{
			dest[0*stride] = (byte) x0;
			dest[1*stride] = (byte) x1;
			dest[2*stride] = (byte) x2;
			dest[3*stride] = (byte) x3;
			dest[4*stride] = (byte) x4;
			dest[5*stride] = (byte) x5;
			dest[6*stride] = (byte) x6;
			dest[7*stride] = (byte) x7;

		}
		else
		{
			dest[0*stride] = CLIP0_255(x0);
			dest[1*stride] = CLIP0_255(x1);
			dest[2*stride] = CLIP0_255(x2);
			dest[3*stride] = CLIP0_255(x3);
			dest[4*stride] = CLIP0_255(x4);
			dest[5*stride] = CLIP0_255(x5);
			dest[6*stride] = CLIP0_255(x6);
			dest[7*stride] = CLIP0_255(x7);
		}
		dest++;
		pred++;
	} 

}
