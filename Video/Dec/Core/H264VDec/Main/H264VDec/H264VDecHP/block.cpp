
/*!
***********************************************************************
*  \file
*      block.c
*
*  \brief
*      Block functions
*
*  \author
*      Main contributors (see contributors.h for copyright, address and affiliation details)
*      - Inge Lille-Langoy          <inge.lille-langoy@telenor.com>
*      - Rickard Sjoberg            <rickard.sjoberg@era.ericsson.se>
***********************************************************************
*/

#include <stdlib.h>
#include <string.h>

#include "global.h"
#include "block.h"
#include "image.h"
#include "mb_access.h"
#include "clipping.h"

#include <mmintrin.h>
#include <xmmintrin.h>
#include <emmintrin.h>

#define Q_BITS          15


DIAG_DOWN_RIGHT_PRED_PX *DIAG_DOWN_RIGHT_PRED_PDR;
DIAG_DOWN_LEFT_PRED_PX *DIAG_DOWN_LEFT_PRED_PDL;
VERT_RIGHT_PRED_PX *VERT_RIGHT_PRED_PVR;
VERT_LEFT_PRED_PX *VERT_LEFT_PRED_PVL;
HOR_UP_PRED_PX *HOR_UP_PRED_PHU;
HOR_DOWN_PRED_PX *HOR_DOWN_PRED_PHD;

static const int A[4][4] = {
	{ 16, 20, 16, 20},
	{ 20, 25, 20, 25},
	{ 16, 20, 16, 20},
	{ 20, 25, 20, 25}
};

UCHAR quant_intra_default[16] = {
	6,13,20,28,
	13,20,28,32,
	20,28,32,37,
	28,32,37,42
};

UCHAR quant_inter_default[16] = {
	10,14,20,24,
	14,20,24,27,
	20,24,27,30,
	24,27,30,34
};

UCHAR quant8_intra_default[64] = {
	6,10,13,16,18,23,25,27,
	10,11,16,18,23,25,27,29,
	13,16,18,23,25,27,29,31,
	16,18,23,25,27,29,31,33,
	18,23,25,27,29,31,33,36,
	23,25,27,29,31,33,36,38,
	25,27,29,31,33,36,38,40,
	27,29,31,33,36,38,40,42
};

UCHAR quant8_inter_default[64] = {
	9,13,15,17,19,21,22,24,
	13,13,17,19,21,22,24,25,
	15,17,19,21,22,24,25,27,
	17,19,21,22,24,25,27,28,
	19,21,22,24,25,27,28,30,
	21,22,24,25,27,28,30,32,
	22,24,25,27,28,30,32,33,
	24,25,27,28,30,32,33,35
};

UCHAR quant_org[16] = { //to be use if no q matrix is chosen
	16,16,16,16,
	16,16,16,16,
	16,16,16,16,
	16,16,16,16
};

UCHAR quant8_org[64] = { //to be use if no q matrix is chosen
	16,16,16,16,16,16,16,16,
	16,16,16,16,16,16,16,16,
	16,16,16,16,16,16,16,16,
	16,16,16,16,16,16,16,16,
	16,16,16,16,16,16,16,16,
	16,16,16,16,16,16,16,16,
	16,16,16,16,16,16,16,16,
	16,16,16,16,16,16,16,16
};

// Notation for comments regarding prediction and predictors.
// The pels of the 4x4 block are labelled a..p. The predictor pels above
// are labelled A..H, from the left I..L, and from above left X, as follows:
//
//  X A B C D E F G H
//  I a b c d
//  J e f g h
//  K i j k l
//  L m n o p
//

// Predictor array index definitions
#define P_L (PredPel[0])
#define P_K (PredPel[1])
#define P_J (PredPel[2])
#define P_I (PredPel[3])
#define P_X (PredPel[4])
#define P_A (PredPel[5])
#define P_B (PredPel[6])
#define P_C (PredPel[7])
#define P_D (PredPel[8])
#define P_E (PredPel[9])
#define P_F (PredPel[10])
#define P_G (PredPel[11])
#define P_H (PredPel[12])

/*!
***********************************************************************
* \brief
*    makes and returns 4x4 blocks with all 5 intra prediction modes
*
* \return
*    DECODING_OK   decoding of intraprediction mode was sucessfull            \n
*    SEARCH_SYNC   search next sync element as errors while decoding occured
***********************************************************************
*/

CREL_RETURN intrapred PARGS2(
														 int ioff,             //!< pixel offset X within MB
														 int joff             //!< pixel offset Y within MB
														 )

{
	int j;
	int s0;	
	//static imgpel __declspec(align(16)) PredPel[16] GCC_ALIGN(16);  // array of predictor pels	
	imgpel *dst;
	imgpel *imgY = IMGPAR m_imgY;
	int stride = dec_picture->Y_stride;

	PixelPos pix_a;
	PixelPos pix_b, pix_c, pix_d;

	int block_available_up;
	int block_available_left;
	int block_available_up_left;
	int block_available_up_right;

	int mb_nr=IMGPAR current_mb_nr_d;
	int left_stride;
	int is_field = IMGPAR MbaffFrameFlag&currMB_d->mb_field;

	imgpel *pSrc_a, *pSrc_b, *pSrc_c, *pSrc_d;	

	byte predmode = currMB_d->ipredmode[joff+(ioff>>2)];
	left_stride=stride<<is_field;	

	if (ioff)
	{	
		//getCurrNeighbourInside_Luma ARGS4(mb_nr, (ioff-1), joff,   &pix_a);
		pix_a.pMB = currMB_d;		
		block_available_left = 1;
		pSrc_a = &IMGPAR mpr[joff][(ioff-1)];
		left_stride = 16;

		if (joff)
		{
			//getCurrNeighbourInside_Luma ARGS4(mb_nr, ioff, (joff-1), &pix_b);
			pix_b.pMB = currMB_d;					
			pSrc_b = &IMGPAR mpr[(joff-1)][ioff];

			//getCurrNeighbourInside_Luma ARGS4(mb_nr, (ioff-1), (joff-1), &pix_d);
			pix_d.pMB = currMB_d;					
			pSrc_d = &IMGPAR mpr[(joff-1)][(ioff-1)];		

			if ((ioff+4)<16)
			{
				//getCurrNeighbourInside_Luma ARGS4(mb_nr, ioff+4, joff-1, &pix_c);
				pix_c.pMB = currMB_d;								
				pSrc_c = &IMGPAR mpr[(joff-1)][(ioff+4)];			
			}
			else
				pix_c.pMB = NULL;
		}
		else
		{
			getCurrNeighbourB_Luma ARGS3(ioff, joff-1, &pix_b);
			pSrc_b = imgY+pix_b.y*stride+pix_b.x;

			getCurrNeighbourB_Luma ARGS3(ioff-1, joff-1, &pix_d);
			pSrc_d = imgY+pix_d.y*stride+pix_d.x;			

			if ((ioff+4)<16)
				getCurrNeighbourB_Luma ARGS3(ioff+4, joff-1, &pix_c);
			else
				getCurrNeighbourC_Luma ARGS3(ioff+4, joff-1, &pix_c);
			pSrc_c = imgY+pix_c.y*stride+pix_c.x;			
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
			//getCurrNeighbourInside_Luma ARGS4(mb_nr, ioff, (joff-1), &pix_b);
			pix_b.pMB = currMB_d;					
			pSrc_b = &IMGPAR mpr[(joff-1)][ioff];

			getCurrNeighbourA_Luma ARGS3(ioff-1, joff-1, &pix_d);
			pSrc_d = imgY+pix_d.y*stride+pix_d.x;

			//getCurrNeighbourInside_Luma ARGS4(mb_nr, ioff+4, joff-1, &pix_c);
			pix_c.pMB = currMB_d;						
			pSrc_c = &IMGPAR mpr[(joff-1)][ioff+4];		
		}
		else
		{
			getCurrNeighbourB_Luma ARGS3(ioff, joff-1, &pix_b);
			getCurrNeighbourD_Luma ARGS3(ioff-1, joff-1, &pix_d);
			getCurrNeighbourB_Luma ARGS3(ioff+4, joff-1, &pix_c);		

			pSrc_b = imgY+pix_b.y*stride+pix_b.x;
			pSrc_c = imgY+pix_c.y*stride+pix_c.x;	
			pSrc_d = imgY+pix_d.y*stride+pix_d.x;			
		}
	}

	if (pix_c.pMB) {
		if ((((ioff&joff&4)^4) | ((ioff | joff)&(~12))) == 0) {
			pix_c.pMB = NULL;
		}
	}

	block_available_up       = (pix_b.pMB!=NULL);
	block_available_up_right = (pix_c.pMB!=NULL);
	block_available_up_left  = (pix_d.pMB!=NULL);


	// form predictor pels
	if (block_available_up)
	{		
		*(long*)&(P_A) = *((long*)pSrc_b);
	}
	else
	{
		memset(&(P_A),128,4*sizeof(imgpel));
	}

	if (block_available_up_right)
	{  		
		*(long*)&(P_E) = *((long*)pSrc_c);
	}
	else
	{
		// IoK - Warning: Intel memset has bug - may need to rewrite this
		memset(&(P_E),P_D,4*sizeof(imgpel));
	}

	if (block_available_left)
	{
		P_I = *(pSrc_a+left_stride*0);
		P_J = *(pSrc_a+left_stride*1);
		P_K = *(pSrc_a+left_stride*2);
		P_L = *(pSrc_a+left_stride*3);	
	}
	else
	{
		memset(&(P_L),128,4*sizeof(imgpel));
	}

	if (block_available_up_left)
	{
		P_X = *(pSrc_d);		
	}
	else
	{
		P_X = 128;
	}

	dst = &IMGPAR mpr[joff][ioff];


	switch (predmode)
	{
	case DC_PRED:                         /* DC prediction */

		s0 = 0;
		if (block_available_up && block_available_left)
		{   
			// no edge
			s0 = (P_A + P_B + P_C + P_D + P_I + P_J + P_K + P_L + 4)>>3;
		}
		else if (!block_available_up && block_available_left)
		{
			// upper edge
			s0 = (P_I + P_J + P_K + P_L + 2)>>2;             
		}
		else if (block_available_up && !block_available_left)
		{
			// left edge
			s0 = (P_A + P_B + P_C + P_D + 2)>>2;             
		}
		else //if (!block_available_up && !block_available_left)
		{
			// top left corner, nothing to predict from
			s0 = 128; // HP restriction
		}

		for (j=0; j < BLOCK_SIZE; j++)
		{
			// store DC prediction
			// IoK - Warning: Intel memset has bug - may need to rewrite this
			memset(dst+j*16,s0,4*sizeof(imgpel));
		}
		break;

	case VERT_PRED:                       /* vertical prediction from block above */
		if (!block_available_up) {
			DEBUG_SHOW_SW_INFO ("warning: Intra_4x4_Vertical prediction mode not allowed at mb %d\n",IMGPAR current_mb_nr_d);
		} else {
			for(j=0;j<BLOCK_SIZE;j++)
			{
				*(long*)&dst[j*16] = *((long*)&(P_A));
			}
		}

		break;

	case HOR_PRED:                        /* horizontal prediction from left block */
		if (!block_available_left) {
			DEBUG_SHOW_SW_INFO ("warning: Intra_4x4_Horizontal prediction mode not allowed at mb %d\n",IMGPAR current_mb_nr_d);
		} else {
			for(j=0;j<BLOCK_SIZE;j++)
				// IoK - Warning: Intel memset has bug - may need to rewrite this
				memset(&dst[j*16],*(&P_I-j),4*sizeof(imgpel));
		}

		break;

	case DIAG_DOWN_RIGHT_PRED:
		if ((!block_available_up)||(!block_available_left)||(!block_available_up_left)) {
			DEBUG_SHOW_SW_INFO ("warning: Intra_4x4_Diagonal_Down_Right prediction mode not allowed at mb %d\n",IMGPAR current_mb_nr_d);
		} else {
			DIAG_DOWN_RIGHT_PRED_PDR ARGS2(dst, PredPel);		
		}
		break;

	case DIAG_DOWN_LEFT_PRED:
		if (!block_available_up) {
			DEBUG_SHOW_SW_INFO ("warning: Intra_4x4_Diagonal_Down_Left prediction mode not allowed at mb %d\n",IMGPAR current_mb_nr_d);
		} else {
			DIAG_DOWN_LEFT_PRED_PDL ARGS2(dst, PredPel);
		}
		break;

	case  VERT_RIGHT_PRED:/* diagonal prediction -22.5 deg to horizontal plane */
		if ((!block_available_up)||(!block_available_left)||(!block_available_up_left)) {
			DEBUG_SHOW_SW_INFO ("warning: Intra_4x4_Vertical_Right prediction mode not allowed at mb %d\n",IMGPAR current_mb_nr_d);
		} else {
			VERT_RIGHT_PRED_PVR ARGS2(dst, PredPel);
		}
		break;

	case  VERT_LEFT_PRED:/* diagonal prediction -22.5 deg to horizontal plane */
		if (!block_available_up) {
			DEBUG_SHOW_SW_INFO ("warning: Intra_4x4_Vertical_Left prediction mode not allowed at mb %d\n",IMGPAR current_mb_nr_d);
		} else {
			VERT_LEFT_PRED_PVL ARGS2(dst, PredPel);
		}
		break;

	case  HOR_UP_PRED:/* diagonal prediction -22.5 deg to horizontal plane */
		if (!block_available_left) {
			DEBUG_SHOW_SW_INFO ("warning: Intra_4x4_Horizontal_Up prediction mode not allowed at mb %d\n",IMGPAR current_mb_nr_d);
		} else {
			HOR_UP_PRED_PHU ARGS1(dst);
		}
		break;

	case  HOR_DOWN_PRED:/* diagonal prediction -22.5 deg to horizontal plane */
		if ((!block_available_up)||(!block_available_left)||(!block_available_up_left)) {
			DEBUG_SHOW_SW_INFO ("warning: Intra_4x4_Horizontal_Down prediction mode not allowed at mb %d\n",IMGPAR current_mb_nr_d);
		} else {
			HOR_DOWN_PRED_PHD ARGS2(dst, PredPel);
		}
		break;

	default:
		DEBUG_SHOW_ERROR_INFO("Error: illegal intra_4x4 prediction mode: %d\n",predmode);
		return CREL_WARNING_H264_STREAMERROR_LEVEL_3;
		break;
	}

	return CREL_OK;
}


/*!
***********************************************************************
* \return
*    best SAD
***********************************************************************
*/

CREL_RETURN intrapred_luma_16x16 PARGS1(int predmode)        //!< prediction mode
{
	int s0=0,s1,s2;

	int i,j;

	int ih,iv;
	int ib,ic,iaa;

	imgpel *imgY=IMGPAR m_imgY;
	imgpel *pImgY0, *pImgY1;
	int stride=dec_picture->Y_stride;
	int ref_stride;

	int mb_nr = IMGPAR current_mb_nr_d;

	PixelPos up;          //!< pixel position p(0,-1)
	PixelPos left[2];    //!< pixel positions p(-1, -1..15)
	PixelPos left_up;
	int mode = 0;

	int up_avail, left_avail, left_up_avail;

	s1=s2=0;	

	//left-up
	getCurrNeighbourD_Luma ARGS3(-1, -1, &left_up);

	//left
	getCurrNeighbourA_Luma ARGS3(-1, 0, &left[0]);
	getCurrNeighbourA_Luma ARGS3(-1, 8, &left[1]);
	ref_stride = stride<<(currMB_d->mb_field&IMGPAR MbaffFrameFlag);	

	//up  
	getCurrNeighbourB_Luma ARGS3(0, -1, &up);

	if (!active_pps.constrained_intra_pred_flag)
	{
		up_avail   = (up.pMB != NULL);
		left_avail = (left[0].pMB != NULL);
		left_up_avail = (left_up.pMB != NULL);
	}
	else
	{
		up_avail      = (up.pMB != NULL);
		for (i=0, left_avail=1; i<2;i++)
			left_avail  &= (left[i].pMB != NULL);
		left_up_avail = (left_up.pMB != NULL);
	}

	switch (predmode)
	{
	case VERT_PRED_16:                       // vertical prediction from block above
		if (!up_avail)
		{
			DEBUG_SHOW_ERROR_INFO ("invalid 16x16 intra pred Mode VERT_PRED_16",500);
			return CREL_WARNING_H264_STREAMERROR_LEVEL_3;
		}
		pImgY0 = imgY+up.y*stride+up.x;
		for(j=0;j<MB_BLOCK_SIZE;j++)
			memcpy(&IMGPAR mpr[j][0], pImgY0, MB_BLOCK_SIZE);// store predicted 16x16 block
		break;

	case HOR_PRED_16:                        // horisontal prediction from left block
		if (!left_avail)
		{
			DEBUG_SHOW_ERROR_INFO ("invalid 16x16 intra pred Mode VERT_PRED_16",500);
			return CREL_WARNING_H264_STREAMERROR_LEVEL_3;
		}
		pImgY0 = imgY+left[0].y*stride+left[0].x;
		pImgY1 = imgY+left[1].y*stride+left[1].x;

		if(mode==0)
		{
			for(j=0;j<(MB_BLOCK_SIZE>>1);j++)
			{
				// Intel compiler 9.0 has fixed this
				memset(&IMGPAR mpr[j][0], *pImgY0, MB_BLOCK_SIZE);
				memset(&IMGPAR mpr[j+8][0], *pImgY1, MB_BLOCK_SIZE);
				pImgY0 += ref_stride;
				pImgY1 += ref_stride;
			}
		}
		else
		{
			for(j=0;j<MB_BLOCK_SIZE;j+=2)
			{
				// Intel compiler 9.0 has fixed this
				memset(&IMGPAR mpr[j][0], *pImgY0, MB_BLOCK_SIZE);
				memset(&IMGPAR mpr[j+1][0], *pImgY1, MB_BLOCK_SIZE);
				pImgY0 += ref_stride;
				pImgY1 += ref_stride;
			}
		}
		break;

	case DC_PRED_16:                         // DC prediction
		s1=s2=0;		

		if (up_avail)
		{
			pImgY0 = imgY+up.y*stride+up.x;
			for (i=0; i < MB_BLOCK_SIZE; i++)
				s1 += pImgY0[i];    // sum hor pix
		}

		if (left_avail)
		{
			pImgY0 = imgY+left[0].y*stride+left[0].x;
			pImgY1 = imgY+left[1].y*stride+left[1].x;

			for (i=0; i < (MB_BLOCK_SIZE>>1); i++)
			{
				s2 += *pImgY0;
				s2 += *pImgY1;
				pImgY0 += ref_stride;
				pImgY1 += ref_stride;
			}
		}

		if (up_avail && left_avail)
			s0=(s1+s2+16)>>5;       // no edge
		if (!up_avail && left_avail)
			s0=(s2+8)>>4;              // upper edge
		if (up_avail && !left_avail)
			s0=(s1+8)>>4;              // left edge
		if (!up_avail && !left_avail)
			s0=128; // HP restriction  // top left corner, nothing to predict from
		// Intel compiler 9.0 has fixed this
		memset(IMGPAR mpr, s0, 16*16);
		break;
	case PLANE_16:// 16 bit integer plan pred
		{		
			imgpel temp_left[16];
			int pred, pred0, pred1;
			if (!up_avail || !left_up_avail  || !left_avail)
			{
				DEBUG_SHOW_ERROR_INFO ("invalid 16x16 intra pred Mode PLANE_16",500);
				return CREL_WARNING_H264_STREAMERROR_LEVEL_3;
			}
			ih=0;
			iv=0;

			pImgY0 = imgY+left[0].y*stride+left[0].x;
			pImgY1 = imgY+left[1].y*stride+left[1].x;

			if(mode==0)
			{
				for(j=0;j<(MB_BLOCK_SIZE>>1);j++)
				{				
					temp_left[j  ]= *pImgY0;
					temp_left[j+8]= *pImgY1;				
					pImgY0 += ref_stride;
					pImgY1 += ref_stride;
				}
			}
			else
			{
				for(j=0;j<MB_BLOCK_SIZE;j+=2)
				{				
					temp_left[j  ]= *pImgY0;
					temp_left[j+1]= *pImgY1;				
					pImgY0 += ref_stride;
					pImgY1 += ref_stride;
				}
			}


			pImgY0 = imgY+up.y*stride+up.x;

			for (i=1;i<8;i++)
			{			
				ih += i*(pImgY0[7+i] - pImgY0[7-i]);
				iv += i*(temp_left[8+i-1] - temp_left[8-i-1]);
			}

			//i == 8
			ih += i*(pImgY0[7+8] - (*(imgY+left_up.y*stride+left_up.x)));
			iv += i*(temp_left[7+8] - (*(imgY+left_up.y*stride+left_up.x)));


			ib=(5*ih+32)>>6;
			ic=(5*iv+32)>>6;

			iaa=16*(pImgY0[15]+temp_left[15]);	

			pred0 = iaa-7*(ib+ic)+16; // HP restriction
			for (j=0;j< MB_BLOCK_SIZE;j++)
			{
				pred1 = pred0;
				for (i=0;i< MB_BLOCK_SIZE;i++)
				{
					pred = __fast_iclip0_255(pred1>>5);
					IMGPAR mpr[j][i]=pred;				
					pred1 += ib;
					//IMGPAR mpr[j][i]=max(0,min((iaa+(i-7)*ib +(j-7)*ic + 16)>>5, 255)); // HP restriction
				}
				pred0 += ic;
			}// store plane prediction
		}
		break;

	default:
		{                                    // indication of fault in bitstream,exit
			DEBUG_SHOW_ERROR_INFO("illegal 16x16 intra prediction mode input: %d\n",predmode);
			return CREL_WARNING_H264_STREAMERROR_LEVEL_3;
		}
	}

	return CREL_OK;
}


CREL_RETURN intrapred_chroma PARGS0()
{
	imgpel *imgUV;
	int stride_UV = dec_picture->UV_stride;
	int mb_nr=IMGPAR current_mb_nr_d;
	int i,j;
	imgpel js[2][2][2];
	int pred0_u, pred1_u;
	int pred0_v, pred1_v;
	imgpel pred;
	int ih_u, iv_u, ib_u, ic_u, iaa_u;
	int ih_v, iv_v, ib_v, ic_v, iaa_v;
	int s0, s1, s2, s3;
	imgpel *base_ptr;
	imgpel *pMpr;
	int uv;
	int left_x, left_y;
	int left_stride;
	imgpel *img_left;

	PixelPos up;        //!< pixel position  p(0,-1)
	PixelPos left[2];  //!< pixel positions p(-1, -1..7), YUV4:2:0 restriction

	int up_avail, left_avail[2], left_up_avail;	

	if(VERT_PRED_8!=currMB_d->c_ipred_mode)
	{
		getCurrNeighbourD_Chroma ARGS3(-1, -1, &left[0]);
		left_up_avail = (left[0].pMB != NULL);

		getCurrNeighbourA_Chroma ARGS3(-1, 0, &left[1]);
		if(left[1].pMB)
		{
			left_x = left[1].x;
			left_y = left[1].y;		

			//statusA = 3, 4, 5, 6, left_stride=(stride_UV<<1);
			left_stride = stride_UV<<(IMGPAR MbaffFrameFlag&currMB_d->mb_field);
			left_avail[0] = left_avail[1]=1;

			if (active_pps.constrained_intra_pred_flag)
			{
				if (IMGPAR MbaffFrameFlag)
				{
					switch(currMB_d->mbStatusA)
					{
					case 1:					
					case 2:
						left_avail[0] = left_avail[1] = (currMB_d->mbIntraA==0x03);
						break;
					case 3:					
					case 4:
						left_avail[0] = (currMB_d->mbIntraA&1);
						left_avail[1] = (currMB_d->mbIntraA>>1);
						break;				
					default:
						break;
					}								
				}				
			}
		}
		else
		{
			left_avail[0] = left_avail[1]=0;
		}
	}

	getCurrNeighbourB_Chroma ARGS3(0, -1, &up);

	up_avail	= (up.pMB != NULL);		

	pMpr = (imgpel*)IMGPAR mprUV;
	imgUV = (imgpel*)IMGPAR m_imgUV;

	left[0].x <<= 1;
	left[1].x <<= 1;
	up.x <<= 1;
	left_x <<= 1;

	switch (currMB_d->c_ipred_mode)
	{
	case PLANE_8:
		if (!left_up_avail || !left_avail[0] || !left_avail[1] || !up_avail)
		{
			DEBUG_SHOW_ERROR_INFO("unexpected PLANE_8 chroma intra prediction mode",-1);
			return CREL_WARNING_H264_STREAMERROR_LEVEL_3;
		}

		ih_u = 8/2*((*(imgUV+up.y*stride_UV+up.x+16-2)) - 
			(*(imgUV+left[0].y*stride_UV+left[0].x))); // HP restriction
		ih_v = 8/2*((*(imgUV+up.y*stride_UV+up.x+16-2+1)) - 
			(*(imgUV+left[0].y*stride_UV+left[0].x+1))); // HP restriction

		for (i=0;i<8/2-1;i++) // HP restriction
		{
			ih_u += (i+1)*(*(imgUV+up.y*stride_UV+up.x+((8/2  +i)<<1)) - 
				(*(imgUV+up.y*stride_UV+up.x+((8/2-2-i)<<1)))); // HP restriction
			ih_v += (i+1)*(*(imgUV+up.y*stride_UV+up.x+((8/2  +i)<<1)+1) - 
				(*(imgUV+up.y*stride_UV+up.x+((8/2-2-i)<<1)+1))); // HP restriction
		}

		img_left = imgUV+left_y*stride_UV+left_x;

		iv_u = 8/2*((*(img_left+(8-1)*left_stride)) - 
			(*(imgUV+left[0].y*stride_UV+left[0].x))); // HP restriction			
		iv_v = 8/2*((*(img_left+(8-1)*left_stride+1)) - 
			(*(imgUV+left[0].y*stride_UV+left[0].x+1))); // HP restriction

		for (i=0;i<8/2-1;i++) // HP restriction
		{
			iv_u += (i+1)*((*(img_left+(8/2+1+i-1)*left_stride)) - 
				(*(img_left+(8/2-1-i-1)*left_stride))); // HP restriction
			iv_v += (i+1)*((*(img_left+(8/2+1+i-1)*left_stride+1)) - 
				(*(img_left+(8/2-1-i-1)*left_stride+1))); // HP restriction
		}

		//ib= ((8 == 8?17:5)*ih+2*8)>>(8 == 8?5:6); // HP restriction
		//ic= ((8 == 8?17:5)*iv+2*8)>>(8 == 8?5:6); // HP restriction
		ib_u= (17*ih_u+16)>>(5);
		ic_u= (17*iv_u+16)>>(5);
		iaa_u= 16*((*(img_left+(8-1)*left_stride)) + 
			(*(imgUV+up.y*stride_UV+up.x+16-2))); // HP restriction

		ib_v= (17*ih_v+16)>>(5);
		ic_v= (17*iv_v+16)>>(5);		
		iaa_v= 16*((*(img_left+(8-1)*left_stride+1)) + 
			(*(imgUV+up.y*stride_UV+up.x+16-2+1))); // HP restriction

		pred0_u = iaa_u+(-8/2+1)*ib_u+(-8/2+1)*ic_u+16; // HP restriction
		pred0_v = iaa_v+(-8/2+1)*ib_v+(-8/2+1)*ic_v+16; // HP restriction

		for (j=0; j<8; j++) // HP restriction
		{
			pred1_u = pred0_u;
			pred1_v = pred0_v;
			for (i=0; i<16; i+=2) // HP restriction
			{
				pred = __fast_iclip0_255(pred1_u>>5);
				pMpr[i]=pred;
				pred = __fast_iclip0_255(pred1_v>>5);
				pMpr[i+1]=pred;

				pred1_u += ib_u;
				pred1_v += ib_v;
			}
			pred0_u += ic_u;
			pred0_v += ic_v;
			pMpr += 16;
		}
		break;
	case DC_PRED_8:
		//===== get prediction value =====
		img_left = imgUV+left_y*stride_UV+left_x;

		for(uv=0;uv<2;uv++)
		{
			if (up_avail)
			{
				imgpel *ptr_base = imgUV+up.y*stride_UV+up.x+uv;
				for (i=0,s0=0;i<(0+4);i++)
					s0 += ptr_base[i<<1];
				for (i=4,s1=0;i<(4+4);i++)
					s1 += ptr_base[i<<1];
				js[uv][0][1] = (s1 + 2) >> 2;
			}

			if (left_avail[0])
			{
				for (i=1,s2=0;i<(1+4);i++)
					s2 += *(img_left+(i-1)*left_stride+uv);
				if(up_avail)
				{
					js[uv][0][0] = (s0+s2+4) >> 3;
				}
				else
				{
					js[uv][0][0] = (s2 + 2) >> 2;
					js[uv][0][1] = (s2 + 2) >> 2;
				}
			}
			else
			{
				if(up_avail)
				{
					js[uv][0][0] = (s0 + 2) >> 2;
				}
				else
				{
					js[uv][0][0] = 128;
					js[uv][0][1] = 128;
				}
			}
			if (left_avail[1])
			{
				for (i=5,s3=0;i<(5+4);i++)
					s3 += *(img_left+(i-1)*left_stride+uv);
				js[uv][1][0] = (s3+   2) >> 2;
				if(up_avail)
				{
					js[uv][1][1] = (s1+s3+4) >> 3;
				}
				else
				{
					js[uv][1][1] = (s3 + 2) >> 2;
				}
			}
			else
			{
				if(up_avail)
				{
					js[uv][1][0] = (s0 + 2) >> 2;
					js[uv][1][1] = (s1 + 2) >> 2;
				}
				else
				{
					js[uv][1][0] = 128;
					js[uv][1][1] = 128;
				}
			}
		}

#if 0
		//Intel compiler 9.1 causes problem here
		for (i=0;i<2;i++)
		{
			pred0 = js[i][0];
			pred1 = js[i][1];
			pred0 = (pred0<<8) | (pred0);
			pred1 = (pred1<<8) | (pred1);
			pred0 = (pred0<<16) | (pred0);
			pred1 = (pred1<<16) | (pred1);
			for (j=i<<2;j<(i<<2)+4;j++)
			{					
				*((int *) (pMpr+(j<<4)+0))=pred0;
				*((int *) (pMpr+(j<<4)+4))=pred1;
			}
		}
#else			
		pred0_u = js[0][0][0];
		pred1_u = js[0][0][1];
		pred0_v = js[1][0][0];
		pred1_v = js[1][0][1];

		pred0_u = (pred0_v<<8) | (pred0_u);
		pred1_u = (pred1_v<<8) | (pred1_u);
		pred0_u = (pred0_u<<16) | (pred0_u);
		pred1_u = (pred1_u<<16) | (pred1_u);

		for (j=0;j<4;j++)
		{					
			*((int *) (pMpr+(j<<4)+0))=pred0_u;
			*((int *) (pMpr+(j<<4)+4))=pred0_u;
			*((int *) (pMpr+(j<<4)+8))=pred1_u;
			*((int *) (pMpr+(j<<4)+12))=pred1_u;
		}

		pred0_u = js[0][1][0];
		pred1_u = js[0][1][1];
		pred0_v = js[1][1][0];
		pred1_v = js[1][1][1];

		pred0_u = (pred0_v<<8) | (pred0_u);
		pred1_u = (pred1_v<<8) | (pred1_u);
		pred0_u = (pred0_u<<16) | (pred0_u);
		pred1_u = (pred1_u<<16) | (pred1_u);

		for (j=4;j<8;j++)
		{					
			*((int *) (pMpr+(j<<4)+0))=pred0_u;
			*((int *) (pMpr+(j<<4)+4))=pred0_u;
			*((int *) (pMpr+(j<<4)+8))=pred1_u;
			*((int *) (pMpr+(j<<4)+12))=pred1_u;
		}
#endif
		break;
	case HOR_PRED_8:
		if (!left_avail[0] || !left_avail[1])
		{
			DEBUG_SHOW_ERROR_INFO("unexpected HOR_PRED_8 chroma intra prediction mode",-1);
			return CREL_WARNING_H264_STREAMERROR_LEVEL_3;
		}

		img_left = imgUV+left_y*stride_UV+left_x;

		for (j=0;j<8;j++) // HP restriction
		{
			pred0_u = *(img_left+(1-1+j)*left_stride);
			pred0_v = *(img_left+(1-1+j)*left_stride+1);

			pred0_u = (pred0_v<<8) | (pred0_u);			
			pred0_u = (pred0_u<<16) | (pred0_u);				
			*((int *) (pMpr+0))=pred0_u;
			*((int *) (pMpr+4))=pred0_u;
			*((int *) (pMpr+8))=pred0_u;
			*((int *) (pMpr+12))=pred0_u;
			pMpr += 16;
		}
		break;
	case VERT_PRED_8:
		if (!up_avail)
		{
			DEBUG_SHOW_ERROR_INFO("unexpected VERT_PRED_8 chroma intra prediction mode",-1);
			return CREL_WARNING_H264_STREAMERROR_LEVEL_3;
		}

		base_ptr = imgUV+up.y*stride_UV+up.x;
		pred0_u = *((int *) (base_ptr));
		pred1_u = *((int *) (base_ptr+4));
		pred0_v = *((int *) (base_ptr+8));
		pred1_v = *((int *) (base_ptr+12));
		for (j=0;j<8;j++) // HP restriction
		{				
			*((int *) (pMpr+0))=pred0_u;
			*((int *) (pMpr+4))=pred1_u;
			*((int *) (pMpr+8))=pred0_v;
			*((int *) (pMpr+12))=pred1_v;
			pMpr += 16;
		}
		break;
	default:
		DEBUG_SHOW_ERROR_INFO("illegal chroma intra prediction mode", 600);
		return CREL_WARNING_H264_STREAMERROR_LEVEL_3;


	}

	return CREL_OK;
}


/*!
************************************************************************
* \brief
*    For mapping the q-matrix to the active id and calculate quantisation values
*
* \param pps
*    Picture parameter set
* \param sps
*    Sequence parameter set
*
************************************************************************
*/
void AssignQuantParam PARGS2(pic_parameter_set_rbsp_t* pps, seq_parameter_set_rbsp_t* sps)
{
	int i;

	if(!pps->pic_scaling_matrix_present_flag && !sps->seq_scaling_matrix_present_flag)
	{
		for(i=0; i<8; i++)
			qmatrix[i] = (i<6) ? quant_org:quant8_org;
	}
	else
	{
		if(sps->seq_scaling_matrix_present_flag) // check sps first
		{
			for(i=0; i<8; i++)
			{
				if(i<6)
				{
					if(!sps->seq_scaling_list_present_flag[i]) // fall-back rule A
					{
						if((i==0) || (i==3))
							qmatrix[i] = (i==0) ? quant_intra_default:quant_inter_default;
						else
							qmatrix[i] = qmatrix[i-1];
					}
					else
					{
						if(sps->UseDefaultScalingMatrix4x4Flag[i])
							qmatrix[i] = (i<3) ? quant_intra_default:quant_inter_default;
						else
							qmatrix[i] = sps->ScalingList4x4[i];
					}
				}
				else
				{
					if(!sps->seq_scaling_list_present_flag[i] || sps->UseDefaultScalingMatrix8x8Flag[i-6]) // fall-back rule A
						qmatrix[i] = (i==6) ? quant8_intra_default:quant8_inter_default;
					else
						qmatrix[i] = sps->ScalingList8x8[i-6];
				}
			}
		}

		if(pps->pic_scaling_matrix_present_flag) // then check pps
		{
			for(i=0; i<8; i++)
			{
				if(i<6)
				{
					if(!pps->pic_scaling_list_present_flag[i]) // fall-back rule B
					{
						if((i==0) || (i==3))
						{
							if(!sps->seq_scaling_matrix_present_flag)
								qmatrix[i] = (i==0) ? quant_intra_default:quant_inter_default;
						}
						else
							qmatrix[i] = qmatrix[i-1];
					}
					else
					{
						if(pps->UseDefaultScalingMatrix4x4Flag[i])
							qmatrix[i] = (i<3) ? quant_intra_default:quant_inter_default;
						else
							qmatrix[i] = pps->ScalingList4x4[i];
					}
				}
				else
				{
					if(!pps->pic_scaling_list_present_flag[i]) // fall-back rule B
					{
						if(!sps->seq_scaling_matrix_present_flag)
							qmatrix[i] = (i==6) ? quant8_intra_default:quant8_inter_default;
					}
					else if(pps->UseDefaultScalingMatrix8x8Flag[i-6])
						qmatrix[i] = (i==6) ? quant8_intra_default:quant8_inter_default;
					else
						qmatrix[i] = pps->ScalingList8x8[i-6];
				}
			}
		}
	}

	CalculateQuantParam ARGS0();
	if(pps->transform_8x8_mode_flag)
		CalculateQuant8Param ARGS0();
}

/*!
************************************************************************
* \brief
*    For calculating the quantisation values at frame level
*
************************************************************************
*/
void CalculateQuantParam PARGS0()
{
	int j, k;
	int j2;

	if((IMGPAR stream_global)->uiH264DXVAMode==E_H264_DXVA_MODE_C)
	{
		for(k=0; k<6; k++)
			for(j=0; j<16; j++)
			{
				j2 = j;

				InvLevelScale4x4Luma_Intra[k][j2]      = dequant_coef[k][j]*qmatrix[0][j];
				InvLevelScale4x4Chroma_Intra[0][k][j2] = dequant_coef[k][j]*qmatrix[1][j];
				InvLevelScale4x4Chroma_Intra[1][k][j2] = dequant_coef[k][j]*qmatrix[2][j];

				InvLevelScale4x4Luma_Inter[k][j2]      = dequant_coef[k][j]*qmatrix[3][j];
				InvLevelScale4x4Chroma_Inter[0][k][j2] = dequant_coef[k][j]*qmatrix[4][j];
				InvLevelScale4x4Chroma_Inter[1][k][j2] = dequant_coef[k][j]*qmatrix[5][j];
			}
	}
	else
	{
		for(k=0; k<6; k++)
			for(j=0; j<16; j++)
			{
#ifdef _PRE_TRANSPOSE_
				j2 = ((j&3)<<2) + (j>>2);
#else
				j2 = j;
#endif
				InvLevelScale4x4Luma_Intra[k][j2]      = dequant_coef[k][j]*qmatrix[0][j];
				InvLevelScale4x4Chroma_Intra[0][k][j2] = dequant_coef[k][j]*qmatrix[1][j];
				InvLevelScale4x4Chroma_Intra[1][k][j2] = dequant_coef[k][j]*qmatrix[2][j];

				InvLevelScale4x4Luma_Inter[k][j2]      = dequant_coef[k][j]*qmatrix[3][j];
				InvLevelScale4x4Chroma_Inter[0][k][j2] = dequant_coef[k][j]*qmatrix[4][j];
				InvLevelScale4x4Chroma_Inter[1][k][j2] = dequant_coef[k][j]*qmatrix[5][j];
			}
	}
}


void DIAG_DOWN_RIGHT_PRED_sse2 PARGS2(unsigned char *dst, byte *Pel)
{
	__m128i xmm0, xmm1, xmm2, xmm3, xmm4;
	long lTemp;
			xmm0 = *((__m128i*)Pel);
			//xmm0 = _mm_load_si128((__m128i*)&Pel);
			xmm4 = _mm_setzero_si128();
			xmm0 = _mm_unpacklo_epi8(xmm0, xmm4);
			xmm2 = xmm1 = xmm0;
			xmm1 = _mm_srli_si128(xmm0, 2);
			xmm2 = _mm_srli_si128(xmm1, 2);
			xmm2 = _mm_insert_epi16(xmm2, P_D, 6);
			xmm3 = _mm_set1_epi16(2);
			xmm1 = _mm_slli_epi16(xmm1, 1);

			xmm1 = _mm_add_epi16(xmm1, xmm0);
			xmm2 = _mm_add_epi16(xmm2, xmm1);
			xmm2 = _mm_add_epi16(xmm2, xmm3);
			xmm2 = _mm_srai_epi16(xmm2, 2);

			xmm4 = _mm_packus_epi16(xmm4, xmm2);

			xmm4 = _mm_srli_si128(xmm4, 8);
			lTemp = _mm_cvtsi128_si32(xmm4);
			*((long*)(dst+(3*16))) = lTemp;
			xmm4 = _mm_srli_si128(xmm4, 1);
			lTemp = _mm_cvtsi128_si32(xmm4);
			*((long*)(dst+(2*16))) = lTemp;
			xmm4 = _mm_srli_si128(xmm4, 1);
			lTemp = _mm_cvtsi128_si32(xmm4);
			*((long*)(dst+(1*16))) = lTemp;
			xmm4 = _mm_srli_si128(xmm4, 1);
			lTemp = _mm_cvtsi128_si32(xmm4);
			*((long*)dst) = lTemp;
}
void DIAG_DOWN_RIGHT_PRED_sse PARGS2(unsigned char *dst, byte *Pel)
{
	__m64 mm0, mm1, mm2, mm3, mm4, mm6, mm7;
	long lTemp;
			mm0 = *((__m64*)Pel);
			mm4 = _mm_setzero_si64();
			mm0 = _mm_unpacklo_pi8(mm0, mm4);
			mm6 = *((__m64*)(Pel+1));
			mm1 = _mm_unpacklo_pi8(mm6, mm4);
			mm6 = *((__m64*)(Pel+2));
			mm2 = _mm_unpacklo_pi8(mm6, mm4);
			mm3 = _mm_set1_pi16(2);
			mm1 = _mm_slli_pi16(mm1, 1);

			mm1 = _mm_add_pi16(mm1, mm0);
			mm2 = _mm_add_pi16(mm2, mm1);
			mm2 = _mm_add_pi16(mm2, mm3);
			mm7 = _mm_srai_pi16(mm2, 2);

			mm0 = *((__m64*)Pel);
			mm4 = _mm_setzero_si64();
			mm0 = _mm_unpackhi_pi8(mm0, mm4);
			mm2 = mm1 = mm0;
			mm1 = _mm_srli_si64(mm0, 16);
			mm2 = _mm_srli_si64(mm1, 16);
			mm2 = _mm_insert_pi16(mm2, P_D, 2);
			mm3 = _mm_set1_pi16(2);
			mm1 = _mm_slli_pi16(mm1, 1);

			mm1 = _mm_add_pi16(mm1, mm0);
			mm2 = _mm_add_pi16(mm2, mm1);
			mm2 = _mm_add_pi16(mm2, mm3);
			mm2 = _mm_srai_pi16(mm2, 2);

			mm4 = _mm_packs_pu16(mm7, mm2);

			lTemp = _mm_cvtsi64_si32(mm4);
			*((long*)(dst+(3*16))) = lTemp;
			mm4 = _mm_srli_si64(mm4, 8);
			lTemp = _mm_cvtsi64_si32(mm4);
			*((long*)(dst+(2*16))) = lTemp;
			mm4 = _mm_srli_si64(mm4, 8);
			lTemp = _mm_cvtsi64_si32(mm4);
			*((long*)(dst+(1*16))) = lTemp;
			mm4 = _mm_srli_si64(mm4, 8);
			lTemp = _mm_cvtsi64_si32(mm4);
			*((long*)dst) = lTemp;
}
void DIAG_DOWN_RIGHT_PRED_c PARGS2(unsigned char *dst, byte *Pel)
{
			dst[3*16+0] = (P_L + 2*P_K + P_J + 2) >>2; 
			dst[2*16+0] =
				dst[3*16+1] = (P_K + 2*P_J + P_I + 2) >>2; 
			dst[1*16+0] =
				dst[2*16+1] = 
				dst[3*16+2] = (P_J + 2*P_I + P_X + 2) >>2; 
			dst[0*16+0] =
				dst[1*16+1] =
				dst[2*16+2] =
				dst[3*16+3] = (P_I + 2*P_X + P_A + 2) >>2; 
			dst[0*16+1] =
				dst[1*16+2] =
				dst[2*16+3] = (P_X + 2*P_A + P_B + 2) >>2;
			dst[0*16+2] =
				dst[1*16+3] = (P_A + 2*P_B + P_C + 2) >>2;
			dst[0*16+3] = (P_B + 2*P_C + P_D + 2) >>2;
}
void DIAG_DOWN_LEFT_PRED_sse PARGS2(unsigned char *dst, byte *Pel)
{
	__m64 mm0, mm1, mm2, mm3, mm4, mm6, mm7;
	long lTemp;
			mm0 = *((__m64*)(Pel+5));
			mm4 = _mm_setzero_si64();
			mm0 = _mm_unpacklo_pi8(mm0, mm4);
			mm6 = *((__m64*)(Pel+6));
			mm1 = _mm_unpacklo_pi8(mm6, mm4);
			mm6 = *((__m64*)(Pel+7));
			mm2 = _mm_unpacklo_pi8(mm6, mm4);
			mm3 = _mm_set1_pi16(2);
			mm1 = _mm_slli_pi16(mm1, 1);

			mm1 = _mm_add_pi16(mm1, mm0);
			mm2 = _mm_add_pi16(mm2, mm1);
			mm2 = _mm_add_pi16(mm2, mm3);
			mm7 = _mm_srai_pi16(mm2, 2);

			mm0 = *((__m64*)(Pel+5));
			mm4 = _mm_setzero_si64();
			mm0 = _mm_unpackhi_pi8(mm0, mm4);
			
			mm1 = _mm_srli_si64(mm0, 16);
			mm2 = _mm_srli_si64(mm1, 16);
			mm2 = _mm_insert_pi16(mm2, P_H, 2);
			mm3 = _mm_set1_pi16(2);
			mm1 = _mm_slli_pi16(mm1, 1);

			mm1 = _mm_add_pi16(mm1, mm0);
			mm2 = _mm_add_pi16(mm2, mm1);
			mm2 = _mm_add_pi16(mm2, mm3);
			mm2 = _mm_srai_pi16(mm2, 2);
			
			mm4 = _mm_packs_pu16(mm7, mm2);

			lTemp = _mm_cvtsi64_si32(mm4);
			*((long*)dst) = lTemp;
			mm4 = _mm_srli_si64(mm4, 8);
			lTemp = _mm_cvtsi64_si32(mm4);
			*((long*)(dst+(1*16))) = lTemp;
			mm4 = _mm_srli_si64(mm4, 8);
			lTemp = _mm_cvtsi64_si32(mm4);
			*((long*)(dst+(2*16))) = lTemp;
			mm4 = _mm_srli_si64(mm4, 8);
			lTemp = _mm_cvtsi64_si32(mm4);
			*((long*)(dst+(3*16))) = lTemp;
}

void DIAG_DOWN_LEFT_PRED_sse2 PARGS2(unsigned char *dst, byte *Pel)
{
	__m128i xmm0, xmm1, xmm2, xmm3, xmm4;
	long lTemp;
			xmm0 = *((__m128i*)Pel);
			xmm0 = _mm_srli_si128(xmm0, 5);
			xmm4 = _mm_setzero_si128();
			xmm0 = _mm_unpacklo_epi8(xmm0, xmm4);
			xmm2 = xmm1 = xmm0;
			xmm1 = _mm_srli_si128(xmm0, 2);
			xmm2 = _mm_srli_si128(xmm1, 2);
			xmm2 = _mm_insert_epi16(xmm2, P_H, 6);
			xmm3 = _mm_set1_epi16(2);
			xmm1 = _mm_slli_epi16(xmm1, 1);

			xmm1 = _mm_add_epi16(xmm1, xmm0);
			xmm2 = _mm_add_epi16(xmm2, xmm1);
			xmm2 = _mm_add_epi16(xmm2, xmm3);
			xmm2 = _mm_srai_epi16(xmm2, 2);

			xmm4 = _mm_packus_epi16(xmm4, xmm2);

			xmm4 = _mm_srli_si128(xmm4, 8);
			lTemp = _mm_cvtsi128_si32(xmm4);
			*((long*)dst) = lTemp;
			xmm4 = _mm_srli_si128(xmm4, 1);
			lTemp = _mm_cvtsi128_si32(xmm4);
			*((long*)(dst+(1*16))) = lTemp;
			xmm4 = _mm_srli_si128(xmm4, 1);
			lTemp = _mm_cvtsi128_si32(xmm4);
			*((long*)(dst+(2*16))) = lTemp;
			xmm4 = _mm_srli_si128(xmm4, 1);
			lTemp = _mm_cvtsi128_si32(xmm4);
			*((long*)(dst+(3*16))) = lTemp;
}

void DIAG_DOWN_LEFT_PRED_c PARGS2(unsigned char *dst, byte *Pel)
{
	

			dst[0*16+0] = (P_A + P_C + 2*(P_B) + 2) >>2;
			dst[0*16+1] = 
				dst[1*16+0] = (P_B + P_D + 2*(P_C) + 2) >>2;
			dst[0*16+2] =
				dst[1*16+1] =
				dst[2*16+0] = (P_C + P_E + 2*(P_D) + 2) >>2;
			dst[0*16+3] = 
				dst[1*16+2] = 
				dst[2*16+1] = 
				dst[3*16+0] = (P_D + P_F + 2*(P_E) + 2) >>2;
			dst[1*16+3] = 
				dst[2*16+2] = 
				dst[3*16+1] = (P_E + P_G + 2*(P_F) + 2) >>2;
			dst[2*16+3] = 
				dst[3*16+2] = (P_F + P_H + 2*(P_G) + 2) >>2;
			dst[3*16+3] = (P_G + 3*(P_H) + 2) >>2;
}

void VERT_RIGHT_PRED_sse PARGS2(unsigned char *dst, byte *Pel)
{
	__m64 mm0, mm1, mm2, mm3, mm4, mm5, mm6, mm7, Temp;
	long lTemp, nTemp1, nTemp2;
			mm0 = *((__m64*)(Pel+1));
			mm5 = _mm_setzero_si64();
			mm6 = _mm_setzero_si64();
			mm0 = _mm_unpacklo_pi8(mm0, mm5);
			mm1 = *((__m64*)(Pel+2));
			mm1 = _mm_unpacklo_pi8(mm1, mm5);
			mm2 = *((__m64*)(Pel+3));
			mm2 = _mm_unpacklo_pi8(mm2, mm5);

			mm3 = _mm_set1_pi16(2);
			mm4 = _mm_set1_pi16(1);

			mm0 = _mm_add_pi16(mm0, mm1);
			mm2 = _mm_add_pi16(mm2, mm0);
			mm2 = _mm_add_pi16(mm2, mm1);

			mm0 = _mm_add_pi16(mm0, mm4);
			mm2 = _mm_add_pi16(mm2, mm3);

			mm0 = _mm_srai_pi16(mm0, 1);
			mm2 = _mm_srai_pi16(mm2, 2);
			nTemp1 = _mm_extract_pi16(mm2, 0);
			nTemp2 = _mm_extract_pi16(mm2, 1);

			mm7 = mm0;
			Temp = mm2;

			mm0 = *((__m64*)(Pel+1));
			mm5 = _mm_setzero_si64();
			mm6 = _mm_setzero_si64();
			mm0 = _mm_unpackhi_pi8(mm0, mm5);
			mm1 = *((__m64*)(Pel+1));
			mm1 = _mm_srli_si64(mm1, 8);
			mm1 = _mm_unpackhi_pi8(mm1, mm5);
			mm2 = *((__m64*)(Pel+1));
			mm2 = _mm_srli_si64(mm2, 16);
			mm2 = _mm_unpackhi_pi8(mm2, mm5);

			mm3 = _mm_set1_pi16(2);
			mm4 = _mm_set1_pi16(1);

			mm0 = _mm_add_pi16(mm0, mm1);
			mm2 = _mm_add_pi16(mm2, mm0);
			mm2 = _mm_add_pi16(mm2, mm1);

			mm0 = _mm_add_pi16(mm0, mm4);
			mm2 = _mm_add_pi16(mm2, mm3);

			mm0 = _mm_srai_pi16(mm0, 1);
			mm2 = _mm_srai_pi16(mm2, 2);

			mm5 = _mm_packs_pu16(mm7, mm0);
			mm6 = _mm_packs_pu16(Temp, mm2);

			mm5 = _mm_srli_si64(mm5, 24);
			lTemp = _mm_cvtsi64_si32(mm5);
			*((long*)dst) = lTemp;
			mm6 = _mm_srli_si64(mm6, 16);
			lTemp = _mm_cvtsi64_si32(mm6);
			*((long*)(dst+(1*16))) = lTemp;
			mm5 = _mm_slli_si64(mm5, 8);
			lTemp = _mm_cvtsi64_si32(mm5);
			*((long*)(dst+(2*16))) = lTemp;
			mm6 = _mm_slli_si64(mm6, 8);
			lTemp = _mm_cvtsi64_si32(mm6);
			*((long*)(dst+(3*16))) = lTemp;

			dst[2*16+0] = (imgpel)nTemp2;
			dst[3*16+0] = (imgpel)nTemp1;
}


void VERT_RIGHT_PRED_sse2 PARGS2(unsigned char *dst, byte *Pel)
{
	__m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6;
	long lTemp, nTemp1, nTemp2;
			xmm0 = *((__m128i*)Pel);
			xmm0 = _mm_srli_si128(xmm0, 1);
			xmm5 = _mm_setzero_si128();
			xmm6 = _mm_setzero_si128();
			xmm0 = _mm_unpacklo_epi8(xmm0, xmm5);
			xmm2 = xmm1 = xmm0;
			xmm1 = _mm_srli_si128(xmm0, 2);
			xmm2 = _mm_srli_si128(xmm1, 2);

			xmm3 = _mm_set1_epi16(2);
			xmm4 = _mm_set1_epi16(1);

			xmm0 = _mm_add_epi16(xmm0, xmm1);
			xmm2 = _mm_add_epi16(xmm2, xmm0);
			xmm2 = _mm_add_epi16(xmm2, xmm1);

			xmm0 = _mm_add_epi16(xmm0, xmm4);
			xmm2 = _mm_add_epi16(xmm2, xmm3);

			xmm0 = _mm_srai_epi16(xmm0, 1);
			xmm2 = _mm_srai_epi16(xmm2, 2);
			nTemp1 = _mm_extract_epi16(xmm2, 0);
			nTemp2 = _mm_extract_epi16(xmm2, 1);

			xmm5 = _mm_packus_epi16(xmm5, xmm0);
			xmm6 = _mm_packus_epi16(xmm6, xmm2);

			xmm5 = _mm_srli_si128(xmm5, 11);
			lTemp = _mm_cvtsi128_si32(xmm5);
			*((long*)dst) = lTemp;
			xmm6 = _mm_srli_si128(xmm6, 10);
			lTemp = _mm_cvtsi128_si32(xmm6);
			*((long*)(dst+(1*16))) = lTemp;
			xmm5 = _mm_slli_si128(xmm5, 1);
			lTemp = _mm_cvtsi128_si32(xmm5);
			*((long*)(dst+(2*16))) = lTemp;
			xmm6 = _mm_slli_si128(xmm6, 1);
			lTemp = _mm_cvtsi128_si32(xmm6);
			*((long*)(dst+(3*16))) = lTemp;

			dst[2*16+0] = (imgpel)nTemp2;
			dst[3*16+0] = (imgpel)nTemp1;
}

void VERT_RIGHT_PRED_c PARGS2(unsigned char *dst, byte *Pel)
{
				dst[0*16+0] = 
				dst[2*16+1] = (P_X + P_A + 1) >>1;
			dst[0*16+1] = 
				dst[2*16+2] = (P_A + P_B + 1) >>1;
			dst[0*16+2] = 
				dst[2*16+3] = (P_B + P_C + 1) >>1;
			dst[0*16+3] = (P_C + P_D + 1) >>1;
			dst[1*16+0] = 
				dst[3*16+1] = (P_I + 2*P_X + P_A + 2) >>2;
			dst[1*16+1] = 
				dst[3*16+2] = (P_X + 2*P_A + P_B + 2) >>2;
			dst[1*16+2] = 
				dst[3*16+3] = (P_A + 2*P_B + P_C + 2) >>2;
			dst[1*16+3] = (P_B + 2*P_C + P_D + 2) >>2;
			dst[2*16+0] = (P_X + 2*P_I + P_J + 2) >>2;
			dst[3*16+0] = (P_I + 2*P_J + P_K + 2) >>2;
}

void VERT_LEFT_PRED_sse PARGS2(unsigned char *dst, byte *Pel)
{
	__m64 mm0, mm1, mm2, mm3, mm4, mm5, mm6, mm7;
	__m64 tmp;
	long lTemp;
			mm0 = *((__m64*)(Pel+5));
			//mm0 = _mm_srli_si64(mm0, 5);
			mm5 = _mm_setzero_si64();
			mm6 = _mm_setzero_si64();
			mm0 = _mm_unpacklo_pi8(mm0, mm5);
			mm1 = *((__m64*)(Pel+6));
			mm1 = _mm_unpacklo_pi8(mm1, mm5);
			mm2 = *((__m64*)(Pel+7));
			mm2 = _mm_unpacklo_pi8(mm2, mm5);
			
			mm3 = _mm_set1_pi16(2);
			mm4 = _mm_set1_pi16(1);

			mm0 = _mm_add_pi16(mm0, mm1);
			mm2 = _mm_add_pi16(mm2, mm0);
			mm2 = _mm_add_pi16(mm2, mm1);

			mm0 = _mm_add_pi16(mm0, mm4);
			mm2 = _mm_add_pi16(mm2, mm3);

			mm0 = _mm_srai_pi16(mm0, 1);
			mm2 = _mm_srai_pi16(mm2, 2);

			mm7 = mm0;
			tmp = mm2;

			mm0 = *((__m64*)(Pel+5));
			//mm0 = _mm_srli_si64(mm0, 5);
			mm5 = _mm_setzero_si64();
			mm6 = _mm_setzero_si64();
			mm0 = _mm_unpackhi_pi8(mm0, mm5);
			mm1 = _mm_srli_si64(mm0, 16);
			mm2 = _mm_srli_si64(mm1, 16);
			
			mm3 = _mm_set1_pi16(2);
			mm4 = _mm_set1_pi16(1);

			mm0 = _mm_add_pi16(mm0, mm1);
			mm2 = _mm_add_pi16(mm2, mm0);
			mm2 = _mm_add_pi16(mm2, mm1);

			mm0 = _mm_add_pi16(mm0, mm4);
			mm2 = _mm_add_pi16(mm2, mm3);

			mm0 = _mm_srai_pi16(mm0, 1);
			mm2 = _mm_srai_pi16(mm2, 2);
			
			mm5 = _mm_packs_pu16(mm7, mm0);
			mm6 = _mm_packs_pu16(tmp, mm2);

			//mm5 = _mm_srli_si64(mm5, 8);
			lTemp = _mm_cvtsi64_si32(mm5);
			*((long*)dst) = lTemp;
			//mm6 = _mm_srli_si64(mm6, 8);
			lTemp = _mm_cvtsi64_si32(mm6);
			*((long*)(dst+(1*16))) = lTemp;
			mm5 = _mm_srli_si64(mm5, 8);
			lTemp = _mm_cvtsi64_si32(mm5);
			*((long*)(dst+(2*16))) = lTemp;
			mm6 = _mm_srli_si64(mm6, 8);
			lTemp = _mm_cvtsi64_si32(mm6);
			*((long*)(dst+(3*16))) = lTemp;
}
void VERT_LEFT_PRED_sse2 PARGS2(unsigned char *dst, byte *Pel)
{
	__m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6;
	long lTemp;
			xmm0 = *((__m128i*)Pel);
			xmm0 = _mm_srli_si128(xmm0, 5);
			xmm5 = _mm_setzero_si128();
			xmm6 = _mm_setzero_si128();
			xmm0 = _mm_unpacklo_epi8(xmm0, xmm5);
			xmm2 = xmm1 = xmm0;
			xmm1 = _mm_srli_si128(xmm0, 2);
			xmm2 = _mm_srli_si128(xmm1, 2);

			xmm3 = _mm_set1_epi16(2);
			xmm4 = _mm_set1_epi16(1);

			xmm0 = _mm_add_epi16(xmm0, xmm1);
			xmm2 = _mm_add_epi16(xmm2, xmm0);
			xmm2 = _mm_add_epi16(xmm2, xmm1);

			xmm0 = _mm_add_epi16(xmm0, xmm4);
			xmm2 = _mm_add_epi16(xmm2, xmm3);

			xmm0 = _mm_srai_epi16(xmm0, 1);
			xmm2 = _mm_srai_epi16(xmm2, 2);

			xmm5 = _mm_packus_epi16(xmm5, xmm0);
			xmm6 = _mm_packus_epi16(xmm6, xmm2);

			xmm5 = _mm_srli_si128(xmm5, 8);
			lTemp = _mm_cvtsi128_si32(xmm5);
			*((long*)dst) = lTemp;
			xmm6 = _mm_srli_si128(xmm6, 8);
			lTemp = _mm_cvtsi128_si32(xmm6);
			*((long*)(dst+(1*16))) = lTemp;
			xmm5 = _mm_srli_si128(xmm5, 1);
			lTemp = _mm_cvtsi128_si32(xmm5);
			*((long*)(dst+(2*16))) = lTemp;
			xmm6 = _mm_srli_si128(xmm6, 1);
			lTemp = _mm_cvtsi128_si32(xmm6);
			*((long*)(dst+(3*16))) = lTemp;
}

void VERT_LEFT_PRED_c PARGS2(unsigned char *dst, byte *Pel)
{
				dst[0*16+0] = (P_A + P_B + 1) >>1;
			dst[0*16+1] = 
				dst[2*16+0] = (P_B + P_C + 1) >>1;
			dst[0*16+2] = 
				dst[2*16+1] = (P_C + P_D + 1) >>1;
			dst[0*16+3] = 
				dst[2*16+2] = (P_D + P_E + 1) >>1;
			dst[2*16+3] = (P_E + P_F + 1) >>1;
			dst[1*16+0] = (P_A + 2*P_B + P_C + 2) >>2;
			dst[1*16+1] = 
				dst[3*16+0] = (P_B + 2*P_C + P_D + 2) >>2;
			dst[1*16+2] = 
				dst[3*16+1] = (P_C + 2*P_D + P_E + 2) >>2;
			dst[1*16+3] = 
				dst[3*16+2] = (P_D + 2*P_E + P_F + 2) >>2;
			dst[3*16+3] = (P_E + 2*P_F + P_G + 2) >>2;

}
void HOR_UP_PRED_PHU_sse PARGS1(unsigned char *dst)
{
	__m64 mm0, mm1, mm2, mm3, mm4;
	long lTemp;
			mm0 = _mm_set_pi16((short)P_L, (short)P_K, (short)P_J, (short)P_I);
			mm2 = mm1 = mm0;
			mm1 = _mm_set_pi16((short)P_L, (short)P_L, (short)P_K, (short)P_J);
			mm2 = _mm_set_pi16(0, (short)P_L, (short)P_L, (short)P_K);

			mm3 = _mm_set1_pi16(2);
			mm4 = _mm_set1_pi16(1);

			mm0 = _mm_add_pi16(mm0, mm1);
			mm2 = _mm_add_pi16(mm2, mm0);
			mm2 = _mm_add_pi16(mm2, mm1);

			mm0 = _mm_add_pi16(mm0, mm4);
			mm2 = _mm_add_pi16(mm2, mm3);

			mm0 = _mm_srai_pi16(mm0, 1);
			mm2 = _mm_srai_pi16(mm2, 2);

			mm2 = _mm_slli_si64(mm2, 8);
			mm2 = _mm_or_si64(mm2, mm0);

			lTemp = _mm_cvtsi64_si32(mm2);
			*((long*)dst) = lTemp;
			mm2 = _mm_srli_si64(mm2, 16);
			lTemp = _mm_cvtsi64_si32(mm2);
			*((long*)(dst+(1*16))) = lTemp;
			mm2 = _mm_srli_si64(mm2, 16);
			lTemp = _mm_cvtsi64_si32(mm2);
			*((long*)(dst+(2*16))) = lTemp;
			mm2 = _mm_srli_si64(mm2, 16);
			lTemp = _mm_cvtsi64_si32(mm2);
			*((long*)(dst+(3*16))) = lTemp;

			dst[2*16+2] = 
				dst[2*16+3] = 
				dst[3*16+0] = 
				dst[3*16+1] = 
				dst[3*16+2] = 
				dst[3*16+3] = P_L;

}

void HOR_UP_PRED_PHU_sse2 PARGS1(unsigned char *dst)
{
	__m128i xmm0, xmm1, xmm2, xmm3, xmm4;
	long lTemp;
			xmm0 = _mm_set_epi16(0, 0, 0, (short)P_L, (short)P_L, (short)P_K, (short)P_J, (short)P_I);
			xmm2 = xmm1 = xmm0;
			xmm1 = _mm_srli_si128(xmm0, 2);
			xmm2 = _mm_srli_si128(xmm1, 2);

			xmm3 = _mm_set1_epi16(2);
			xmm4 = _mm_set1_epi16(1);

			xmm0 = _mm_add_epi16(xmm0, xmm1);
			xmm2 = _mm_add_epi16(xmm2, xmm0);
			xmm2 = _mm_add_epi16(xmm2, xmm1);

			xmm0 = _mm_add_epi16(xmm0, xmm4);
			xmm2 = _mm_add_epi16(xmm2, xmm3);

			xmm0 = _mm_srai_epi16(xmm0, 1);
			xmm2 = _mm_srai_epi16(xmm2, 2);

			xmm2 = _mm_slli_si128(xmm2, 1);
			xmm2 = _mm_or_si128(xmm2, xmm0);

			lTemp = _mm_cvtsi128_si32(xmm2);
			*((long*)dst) = lTemp;
			xmm2 = _mm_srli_si128(xmm2, 2);
			lTemp = _mm_cvtsi128_si32(xmm2);
			*((long*)(dst+(1*16))) = lTemp;
			xmm2 = _mm_srli_si128(xmm2, 2);
			lTemp = _mm_cvtsi128_si32(xmm2);
			*((long*)(dst+(2*16))) = lTemp;
			xmm2 = _mm_srli_si128(xmm2, 2);
			lTemp = _mm_cvtsi128_si32(xmm2);
			*((long*)(dst+(3*16))) = lTemp;

			dst[2*16+2] = 
				dst[2*16+3] = 
				dst[3*16+0] = 
				dst[3*16+1] = 
				dst[3*16+2] = 
				dst[3*16+3] = P_L;

}

void HOR_UP_PRED_PHU_c PARGS1(unsigned char *dst)
{
			dst[0*16+0] = (P_I + P_J + 1) >>1;
			dst[0*16+1] = (P_I + 2*P_J + P_K + 2) >>2;
			dst[0*16+2] = 
				dst[1*16+0] = (P_J + P_K + 1) >>1;
			dst[0*16+3] = 
				dst[1*16+1] = (P_J + 2*P_K + P_L + 2) >>2;
			dst[1*16+2] = 
				dst[2*16+0] = (P_K + P_L + 1) >>1;
			dst[1*16+3] = 
				dst[2*16+1] = (P_K + 2*P_L + P_L + 2) >>2;
			dst[2*16+3] = 
				dst[3*16+1] = 
				dst[3*16+0] = 
				dst[2*16+2] = 
				dst[3*16+2] = 
				dst[3*16+3] = P_L;

}
void HOR_DOWN_PRED_PHD_sse PARGS2(unsigned char *dst, byte *Pel)
{
	__m64 mm0, mm1, mm2, mm3, mm4, mm5, mm6, mm7;
	long lTemp, nTemp1, nTemp2, nTemp3;
			mm0 = *((__m64*)Pel);
			mm5 = _mm_setzero_si64();
			mm0 = _mm_unpacklo_pi8(mm0, mm5);
			mm1 = _mm_slli_si64(mm0, 16);
			mm2 = _mm_slli_si64(mm1, 16);
			
			mm3 = _mm_set1_pi16(2);
			mm4 = _mm_set1_pi16(1);

			mm0 = _mm_add_pi16(mm0, mm1);
			mm2 = _mm_add_pi16(mm2, mm0);
			mm2 = _mm_add_pi16(mm2, mm1);

			mm0 = _mm_add_pi16(mm0, mm4);
			mm2 = _mm_add_pi16(mm2, mm3);

			mm6 = _mm_srai_pi16(mm0, 1);
			mm7 = _mm_srai_pi16(mm2, 2);

			mm0 = *((__m64*)Pel);
			mm5 = _mm_setzero_si64();
			mm0 = _mm_unpackhi_pi8(mm0, mm5);
			mm1 = *((__m64*)(Pel+3));
			mm1 = _mm_unpacklo_pi8(mm1, mm5);
			mm2 = *((__m64*)(Pel+2));
			mm2 = _mm_unpacklo_pi8(mm2, mm5);
			
			mm3 = _mm_set1_pi16(2);
			mm4 = _mm_set1_pi16(1);

			mm0 = _mm_add_pi16(mm0, mm1);
			mm2 = _mm_add_pi16(mm2, mm0);
			mm2 = _mm_add_pi16(mm2, mm1);

			mm0 = _mm_add_pi16(mm0, mm4);
			mm2 = _mm_add_pi16(mm2, mm3);

			mm0 = _mm_srai_pi16(mm0, 1);
			mm2 = _mm_srai_pi16(mm2, 2);

			lTemp = _mm_extract_pi16(mm6, 1);
			nTemp1 = _mm_extract_pi16(mm6, 2);
			nTemp2 = _mm_extract_pi16(mm6, 3);
			nTemp3 = _mm_extract_pi16(mm0, 0);
			mm6 = _mm_set_pi16(nTemp3, nTemp2, nTemp1, lTemp);
			mm0 = _mm_srli_si64(mm0, 16);

			lTemp = _mm_extract_pi16(mm7, 2);
			nTemp1 = _mm_extract_pi16(mm7, 3);
			nTemp2 = _mm_extract_pi16(mm2, 0);
			nTemp3 = _mm_extract_pi16(mm2, 1);
			mm7 = _mm_set_pi16(nTemp3, nTemp2, nTemp1, lTemp);
			mm2 = _mm_srli_si64(mm2, 32);

			nTemp1 = _mm_extract_pi16(mm2, 0);
			nTemp2 = _mm_extract_pi16(mm2, 1);
			mm2 = _mm_slli_si64(mm7, 8);
			mm2 = _mm_or_si64(mm2, mm6);

			lTemp = _mm_cvtsi64_si32(mm2);
			*((long*)(dst+(3*16))) = lTemp;
			mm2 = _mm_srli_si64(mm2, 16);
			lTemp = _mm_cvtsi64_si32(mm2);
			*((long*)(dst+(2*16))) = lTemp;
			mm2 = _mm_srli_si64(mm2, 16);
			lTemp = _mm_cvtsi64_si32(mm2);
			*((long*)(dst+(1*16))) = lTemp;
			mm2 = _mm_srli_si64(mm2, 16);
			lTemp = _mm_cvtsi64_si32(mm2);
			*((long*)dst) = lTemp;

			dst[2] = (imgpel)nTemp1;
			dst[3] = (imgpel)nTemp2;
}

void HOR_DOWN_PRED_PHD_sse2 PARGS2(unsigned char *dst, byte *Pel)
{
	__m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5;
	long lTemp, nTemp1, nTemp2;
			xmm0 = *((__m128i*)Pel);
			xmm5 = _mm_setzero_si128();
			xmm0 = _mm_unpacklo_epi8(xmm0, xmm5);
			xmm2 = xmm1 = xmm0;
			xmm1 = _mm_slli_si128(xmm0, 2);
			xmm2 = _mm_slli_si128(xmm1, 2);

			xmm3 = _mm_set1_epi16(2);
			xmm4 = _mm_set1_epi16(1);

			xmm0 = _mm_add_epi16(xmm0, xmm1);
			xmm2 = _mm_add_epi16(xmm2, xmm0);
			xmm2 = _mm_add_epi16(xmm2, xmm1);

			xmm0 = _mm_add_epi16(xmm0, xmm4);
			xmm2 = _mm_add_epi16(xmm2, xmm3);

			xmm0 = _mm_srai_epi16(xmm0, 1);
			xmm2 = _mm_srai_epi16(xmm2, 2);

			xmm0 = _mm_srli_si128(xmm0, 2);
			xmm2 = _mm_srli_si128(xmm2, 4);
			nTemp1 = _mm_extract_epi16(xmm2, 4);
			nTemp2 = _mm_extract_epi16(xmm2, 5);
			xmm2 = _mm_slli_si128(xmm2, 1);
			xmm2 = _mm_or_si128(xmm2, xmm0);

			lTemp = _mm_cvtsi128_si32(xmm2);
			*((long*)(dst+(3*16))) = lTemp;
			xmm2 = _mm_srli_si128(xmm2, 2);
			lTemp = _mm_cvtsi128_si32(xmm2);
			*((long*)(dst+(2*16))) = lTemp;
			xmm2 = _mm_srli_si128(xmm2, 2);
			lTemp = _mm_cvtsi128_si32(xmm2);
			*((long*)(dst+(1*16))) = lTemp;
			xmm2 = _mm_srli_si128(xmm2, 2);
			lTemp = _mm_cvtsi128_si32(xmm2);
			*((long*)dst) = lTemp;

			dst[2] = (imgpel)nTemp1;
			dst[3] = (imgpel)nTemp2;
}

void HOR_DOWN_PRED_PHD_c PARGS2(unsigned char *dst, byte *Pel)
{
		dst[0*16+0] = 
				dst[1*16+2] = (P_X + P_I + 1) >>1;
			dst[0*16+1] = 
				dst[1*16+3] = (P_I + 2*P_X + P_A + 2) >>2;
			dst[0*16+2] = (P_X + 2*P_A + P_B + 2) >>2;
			dst[0*16+3] = (P_A + 2*P_B + P_C + 2) >>2;
			dst[1*16+0] = 
				dst[2*16+2] = (P_I + P_J + 1) >>1;
			dst[1*16+1] = 
				dst[2*16+3] = (P_X + 2*P_I + P_J + 2) >>2;
			dst[2*16+0] = 
				dst[3*16+2] = (P_J + P_K + 1) >>1;
			dst[2*16+1] = 
				dst[3*16+3] = (P_I + 2*P_J + P_K + 2) >>2;
			dst[3*16+0] = (P_K + P_L + 1) >>1;
			dst[3*16+1] = (P_J + 2*P_K + P_L + 2) >>2;
}