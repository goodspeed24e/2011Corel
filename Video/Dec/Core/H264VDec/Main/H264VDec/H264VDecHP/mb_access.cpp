
/*!
*************************************************************************************
* \file mb_access.c
*
* \brief
*    Functions for macroblock neighborhoods
*
*  \author
*      Main contributors (see contributors.h for copyright, address and affiliation details)
*      - Karsten Sühring          <suehring@hhi.de>
*************************************************************************************
*/
#include <assert.h>

#include "global.h"
#include "mbuffer.h"
#include "mb_access.h"

#define __FAST__POINTER_CALLING

/*!
************************************************************************
* \brief
*    returns 1 if the macroblock at the given address is available
************************************************************************
*/
int mb_is_available PARGS1(int mbAddr)
{
	if (mbAddr < IMGPAR currentSlice->start_mb_nr)
		return 0;

	return 1;
}

/*!
************************************************************************
* \brief
*    Checks the availability of neighboring macroblocks of
*    the current macroblock for prediction and context determination;
************************************************************************
*/
void CheckAvailabilityOfNeighbors_NonMBAff_Plane PARGS0()
{
	const int mb_nr = IMGPAR current_mb_nr_r;
	const int mb_x = IMGPAR mb_x_r;
	const int PicWidthInMbs = dec_picture->PicWidthInMbs;
	int	aA, aB, aC, aD;
	int start_mb_nr = IMGPAR currentSlice->start_mb_nr;	

	IMGPAR mbAddrA = aA = mb_nr - 1;
	IMGPAR mbAddrB = aB = mb_nr - PicWidthInMbs;
	IMGPAR mbAddrC = aC = aB + 1;
	IMGPAR mbAddrD = aD = aB - 1;

	if ( (mb_x>0) && (aA>=start_mb_nr) ) {
		IMGPAR pLeftMB_r = &(IMGPAR mb_decdata[aA]);
	} else {
		IMGPAR pLeftMB_r = NULL;
		IMGPAR mbAddrA = -1;
	};

	if ( (aB>=start_mb_nr) ) {
		IMGPAR pUpMB_r = &(IMGPAR mb_decdata[aB]);
	} else {
		IMGPAR pUpMB_r = NULL;
		IMGPAR mbAddrB = -1;
	};

	if ( (mb_x<PicWidthInMbs-1) && (aC>=start_mb_nr) ) {
		IMGPAR pUpRightMB_r = &(IMGPAR mb_decdata[aC]);
	} else {
		IMGPAR pUpRightMB_r = NULL;
		IMGPAR mbAddrC = -1;
	};

	if ( (mb_x>0) && (aD>=start_mb_nr) ) {
		IMGPAR pUpLeftMB_r = &(IMGPAR mb_decdata[aD]);
	} else {
		IMGPAR pUpLeftMB_r = NULL;
		IMGPAR mbAddrD = -1;
	};

	IMGPAR pLastMB_r = NULL;
	/*	
	#if defined (_COLLECT_PIC_)
	IMGPAR pLastMB_r = NULL;
	#else
	IMGPAR pLastMB_r = &(IMGPAR mb_decdata[mb_nr-1]);
	#endif
	*/
}

void CheckAvailabilityOfNeighbors_NonMBAff_Plane_FMO PARGS0()
{
	const int mb_nr = IMGPAR current_mb_nr_r;
	const int mb_x = IMGPAR mb_x_r;
	const int PicWidthInMbs = dec_picture->PicWidthInMbs;
	int	aA, aB, aC, aD;
	int start_mb_nr = IMGPAR currentSlice->start_mb_nr;	

	IMGPAR mbAddrA = aA = mb_nr - 1;
	IMGPAR mbAddrB = aB = mb_nr - PicWidthInMbs;
	IMGPAR mbAddrC = aC = aB + 1;
	IMGPAR mbAddrD = aD = aB - 1;

	if ( (mb_x>0) && (aA>=start_mb_nr) ) {
		if(IMGPAR decoded_flag[IMGPAR mbAddrA])
			IMGPAR pLeftMB_r = &(IMGPAR mb_decdata[aA]);
		else
		{
			IMGPAR pLeftMB_r = NULL;
			IMGPAR mbAddrA = -1;
		}
	} else {
		IMGPAR pLeftMB_r = NULL;
		IMGPAR mbAddrA = -1;
	};

	if ( (aB>=start_mb_nr) ) {
		if(IMGPAR decoded_flag[IMGPAR mbAddrB])
			IMGPAR pUpMB_r = &(IMGPAR mb_decdata[aB]);
		else
		{
			IMGPAR pUpMB_r = NULL;
			IMGPAR mbAddrB = -1;
		}
	} else {
		IMGPAR pUpMB_r = NULL;
		IMGPAR mbAddrB = -1;
	};

	if ( (mb_x<PicWidthInMbs-1) && (aC>=start_mb_nr) ) {
		if(IMGPAR decoded_flag[IMGPAR mbAddrC])
			IMGPAR pUpRightMB_r = &(IMGPAR mb_decdata[aC]);
		else
		{
			IMGPAR pUpRightMB_r = NULL;
			IMGPAR mbAddrC = -1;
		}
	} else {
		IMGPAR pUpRightMB_r = NULL;
		IMGPAR mbAddrC = -1;
	};

	if ( (mb_x>0) && (aD>=start_mb_nr) ) {
		if(IMGPAR decoded_flag[IMGPAR mbAddrD])
			IMGPAR pUpLeftMB_r = &(IMGPAR mb_decdata[aD]);
		else
		{
			IMGPAR pUpLeftMB_r = NULL;
			IMGPAR mbAddrD = -1;
		}
	} else {
		IMGPAR pUpLeftMB_r = NULL;
		IMGPAR mbAddrD = -1;
	};

	IMGPAR pLastMB_r = NULL;
	/*	
	#if defined (_COLLECT_PIC_)
	IMGPAR pLastMB_r = NULL;
	#else
	IMGPAR pLastMB_r = &(IMGPAR mb_decdata[mb_nr-1]);
	#endif
	*/
}

void CheckAvailabilityOfNeighbors_NonMBAff_Row PARGS0()
{
	const int mb_nr = IMGPAR current_mb_nr_r;
	const int mb_x = IMGPAR mb_x_r;
	const int PicWidthInMbs = dec_picture->PicWidthInMbs;
	int	aA, aB, aC, aD, aRowB;
	int start_mb_nr = IMGPAR currentSlice->start_mb_nr;	

	IMGPAR mbAddrA = aA = mb_nr - 1;
	IMGPAR mbAddrB = aB = mb_nr - PicWidthInMbs;
	IMGPAR mbAddrC = aC = aB + 1;
	IMGPAR mbAddrD = aD = aB - 1;

	aRowB = mb_x;

	if ( (mb_x>0) && (aA>=start_mb_nr) ) {
		IMGPAR pLeftMB_r = IMGPAR pMBpair_left;
	} else {
		IMGPAR pLeftMB_r = NULL;
		IMGPAR mbAddrA = -1;
	};

	if ( (aB>=start_mb_nr) ) {
		IMGPAR pUpMB_r = &(IMGPAR mb_decdata[aRowB]);
	} else {
		IMGPAR pUpMB_r = NULL;
		IMGPAR mbAddrB = -1;
	};

	if ( (mb_x<PicWidthInMbs-1) && (aC>=start_mb_nr) ) {
		IMGPAR pUpRightMB_r = &(IMGPAR mb_decdata[aRowB + 1]);
	} else {
		IMGPAR pUpRightMB_r = NULL;
		IMGPAR mbAddrC = -1;
	};

	if ( (mb_x>0) && (aD>=start_mb_nr) ) {
		IMGPAR pUpLeftMB_r = &(IMGPAR mb_decdata[aRowB - 1]);
	} else {
		IMGPAR pUpLeftMB_r = NULL;
		IMGPAR mbAddrD = -1;
	};

	IMGPAR pLastMB_r = NULL;
}

void CheckAvailabilityOfNeighbors_NonMBAff_Row_FMO PARGS0()
{
	const int mb_nr = IMGPAR current_mb_nr_r;
	const int mb_x = IMGPAR mb_x_r;
	const int PicWidthInMbs = dec_picture->PicWidthInMbs;
	int	aA, aB, aC, aD, aRowB;
	int start_mb_nr = IMGPAR currentSlice->start_mb_nr;	

	IMGPAR mbAddrA = aA = mb_nr - 1;
	IMGPAR mbAddrB = aB = mb_nr - PicWidthInMbs;
	IMGPAR mbAddrC = aC = aB + 1;
	IMGPAR mbAddrD = aD = aB - 1;

	aRowB = mb_x;

	if ( (mb_x>0) && (aA>=start_mb_nr) ) {
		if (IMGPAR decoded_flag[IMGPAR mbAddrA])
			IMGPAR pLeftMB_r = IMGPAR pMBpair_left;
		else
		{
			IMGPAR pLeftMB_r = NULL;
			IMGPAR mbAddrA = -1;
		}
	} else {
		IMGPAR pLeftMB_r = NULL;
		IMGPAR mbAddrA = -1;
	};

	if ( (aB>=start_mb_nr) ) {
		if(IMGPAR decoded_flag[IMGPAR mbAddrB])
			IMGPAR pUpMB_r = &(IMGPAR mb_decdata[aRowB]);
		else
		{
			IMGPAR pUpMB_r = NULL;
			IMGPAR mbAddrB = -1;
		}
	} else {
		IMGPAR pUpMB_r = NULL;
		IMGPAR mbAddrB = -1;
	};

	if ( (mb_x<PicWidthInMbs-1) && (aC>=start_mb_nr) ) {
		if(IMGPAR decoded_flag[IMGPAR mbAddrC])
			IMGPAR pUpRightMB_r = &(IMGPAR mb_decdata[aRowB + 1]);
		else
		{
			IMGPAR pUpRightMB_r = NULL;
			IMGPAR mbAddrC = -1;
		}
	} else {
		IMGPAR pUpRightMB_r = NULL;
		IMGPAR mbAddrC = -1;
	};

	if ( (mb_x>0) && (aD>=start_mb_nr) ) {
		if(IMGPAR decoded_flag[IMGPAR mbAddrD])
			IMGPAR pUpLeftMB_r = &(IMGPAR mb_decdata[aRowB - 1]);
		else
		{
			IMGPAR pUpLeftMB_r = NULL;
			IMGPAR mbAddrD = -1;
		}
	} else {
		IMGPAR pUpLeftMB_r = NULL;
		IMGPAR mbAddrD = -1;
	};

	IMGPAR pLastMB_r = NULL;
}

/*!
************************************************************************
* \brief
*    Checks the availability of neighboring macroblocks of
*    the current macroblock for prediction and context determination;
************************************************************************
*/
void CheckAvailabilityOfNeighbors_MBAff_Plane PARGS0()
{
	const int mb_nr = IMGPAR current_mb_nr_r;
	const int half_mb_nr=(mb_nr>>1);
	const int mb_x = IMGPAR mb_x_r;
	const int PicWidthInMbs = dec_picture->PicWidthInMbs;
	int	aA, aB, aC, aD;
	int start_mb_nr = (IMGPAR currentSlice->start_mb_nr) << 1;


	IMGPAR mbAddrA = aA = (half_mb_nr - 1) << 1;
	IMGPAR mbAddrB = aB = (half_mb_nr - PicWidthInMbs) << 1;
	IMGPAR mbAddrC = aC = aB+2;
	IMGPAR mbAddrD = aD = aB-2;


	if ( (mb_x>0) && (aA>=start_mb_nr) ) {
		IMGPAR pLeftMB_r = &(IMGPAR mb_decdata[aA]);
	} else {
		IMGPAR pLeftMB_r = NULL;
		IMGPAR mbAddrA = -1;
	};

	if ( aB>=0 && (aB>=start_mb_nr) ) {
		IMGPAR pUpMB_r = &(IMGPAR mb_decdata[aB]);
	} else {
		IMGPAR pUpMB_r = NULL;
		IMGPAR mbAddrB = -1;
	};

	if ( aC>=0 &&  (mb_x<PicWidthInMbs-1) && (aC>=start_mb_nr) ) {
		IMGPAR pUpRightMB_r = &(IMGPAR mb_decdata[aC]);
	} else {
		IMGPAR pUpRightMB_r = NULL;
		IMGPAR mbAddrC = -1;
	};

	if ( aD>=0 && (mb_x>0) && (aD>=start_mb_nr) ) {
		IMGPAR pUpLeftMB_r = &(IMGPAR mb_decdata[aD]);
	} else {
		IMGPAR pUpLeftMB_r = NULL;
		IMGPAR mbAddrD = -1;
	};

	if (mb_nr & 1) {
		IMGPAR pLastMB_r = &(IMGPAR mb_decdata[half_mb_nr<<1]);
	} else {
		IMGPAR pLastMB_r = NULL;
	}	
}

void CheckAvailabilityOfNeighbors_MBAff_Row PARGS0()
{
	const int mb_nr = IMGPAR current_mb_nr_r;
	const int half_mb_nr=(mb_nr>>1);
	const int mb_x = IMGPAR mb_x_r;
	const int PicWidthInMbs = dec_picture->PicWidthInMbs;
	int	aA, aB, aC, aD, aRowB;	
	int start_mb_nr = (IMGPAR currentSlice->start_mb_nr) << 1;

	IMGPAR mbAddrA = aA = (half_mb_nr - 1) << 1;
	IMGPAR mbAddrB = aB = (half_mb_nr - PicWidthInMbs) << 1;
	IMGPAR mbAddrC = aC = aB+2;
	IMGPAR mbAddrD = aD = aB-2;

	aRowB = mb_x<<1;

	if ( (mb_x>0) && (aA >= start_mb_nr) ) {
		IMGPAR pLeftMB_r = IMGPAR pMBpair_left;
	} else {
		IMGPAR pLeftMB_r = NULL;
		IMGPAR mbAddrA = -1;
	};

	if ( aB>=0 && (aB >= start_mb_nr) ) {
		IMGPAR pUpMB_r = &(IMGPAR mb_decdata[aRowB]);
	} else {
		IMGPAR pUpMB_r = NULL;
		IMGPAR mbAddrB = -1;
	};

	if ( aC>=0 && (mb_x<PicWidthInMbs-1) && (aC>=start_mb_nr) ) {
		IMGPAR pUpRightMB_r = &(IMGPAR mb_decdata[aRowB + 2]);
	} else {
		IMGPAR pUpRightMB_r = NULL;
		IMGPAR mbAddrC = -1;
	};

	if ( aD>=0 && (mb_x>0) && (aD>=start_mb_nr) ) {
		IMGPAR pUpLeftMB_r = &(IMGPAR mb_decdata[aRowB - 2]);
	} else {
		IMGPAR pUpLeftMB_r = NULL;
		IMGPAR mbAddrD = -1;
	};

	if (mb_nr & 1) {
		IMGPAR pLastMB_r = IMGPAR pMBpair_current;
	} else {
		IMGPAR pLastMB_r = NULL;
	}	
}

void CheckAvailabilityOfNeighbors_ABCD_even PARGS0()
{
	currMB_r->mbStatusA = 0;
	currMB_r->mbStatusB = 0;
	currMB_r->mbStatusC = 0;
	currMB_r->mbStatusD = 0;
	IMGPAR mb_up_factor = 0;

	if (IMGPAR MbaffFrameFlag)
	{
		int up_mb_addr, upleft_mb_addr;

		up_mb_addr = IMGPAR mbAddrB;
		upleft_mb_addr = IMGPAR mbAddrD;

		//A, B, C & D
		if (currMB_r->mb_field) {

			if (IMGPAR pLeftMB_r)
			{
				if (IMGPAR pLeftMB_r->mb_field){
					currMB_r->mbStatusA = 5;
				} else {
					currMB_r->mbStatusA = 3;
				}
			}

			if ( IMGPAR pUpMB_r && !IMGPAR pUpMB_r->mb_field ) {
				IMGPAR pUpMB_r++;
				up_mb_addr++; // Could be removed in future
			}
			currMB_r->mbStatusB = 1;

			if(IMGPAR pUpRightMB_r && !IMGPAR pUpRightMB_r->mb_field) {
				IMGPAR mbAddrC++;	//Could be removed in future
				IMGPAR pUpRightMB_r++;
				IMGPAR mb_up_factor = 1;
			}										
			currMB_r->mbStatusC = 1;


			if (IMGPAR pUpLeftMB_r && !IMGPAR pUpLeftMB_r->mb_field)
			{
				upleft_mb_addr++;	//Could be removed in future
				IMGPAR pUpLeftMB_r++;
				//currMB_r->mbStatusD = 6;
			}
			currMB_r->mbStatusD = 1;


		} else {		

			if (IMGPAR pLeftMB_r && IMGPAR pLeftMB_r->mb_field)	{
				currMB_r->mbStatusA = 1;
			}


			if (IMGPAR pUpMB_r) {
				up_mb_addr++; //Could be removed in future
				IMGPAR pUpMB_r++;
			}

			if(IMGPAR pUpRightMB_r) {
				IMGPAR mbAddrC++; //Could be removed in future
				IMGPAR pUpRightMB_r++;
			}

			if ( IMGPAR pUpLeftMB_r ) {
				upleft_mb_addr++;	//Could be removed in future
				IMGPAR pUpLeftMB_r++;
			}
		}

		//Set addr B
		IMGPAR mbAddrB = up_mb_addr;

		//Set addr D
		IMGPAR mbAddrD = upleft_mb_addr;

	}
}


void CheckAvailabilityOfNeighbors_ABCD_odd PARGS0()
{
	currMB_r->mbStatusA = 0;
	currMB_r->mbStatusB = 0;
	currMB_r->mbStatusC = 0;
	currMB_r->mbStatusD = 0;
	IMGPAR mb_up_factor = 0;

	if (IMGPAR MbaffFrameFlag)
	{
		int up_mb_addr, left_mb_addr, upleft_mb_addr;

		left_mb_addr = IMGPAR mbAddrA;
		up_mb_addr = IMGPAR mbAddrB;	
		upleft_mb_addr = IMGPAR mbAddrD;

		//A, B, C & D
		if(currMB_r->mb_field) {
			//A
			if (IMGPAR pLeftMB_r) {
				if(IMGPAR pLeftMB_r->mb_field) {
					left_mb_addr++;	// Could be removed in future
					IMGPAR pLeftMB_r++;
					currMB_r->mbStatusA = 6;
				} else {
					currMB_r->mbStatusA = 4;
				}
			}

			//B
			currMB_r->mbStatusB = 2;

			if (IMGPAR pUpMB_r) {
				up_mb_addr++;		// Could be removed in future
				IMGPAR pUpMB_r++;
			}


			//C
			if(IMGPAR pUpRightMB_r) {
				IMGPAR mbAddrC++;	// Could be removed in future
				IMGPAR pUpRightMB_r++;
				currMB_r->mbStatusC = 2;
			}

			//D
			currMB_r->mbStatusD = 2;
			if (IMGPAR pUpLeftMB_r) {
				upleft_mb_addr++;  //Could be removed in future
				IMGPAR pUpLeftMB_r++;
			}

		} else {	// bottom
			//A
			if (IMGPAR pLeftMB_r) {
				if(IMGPAR pLeftMB_r->mb_field) {					
					currMB_r->mbStatusA = 2;
				} else {
					left_mb_addr++; //Could be removed in future
					IMGPAR pLeftMB_r++;
				}
			}

			//B
			up_mb_addr = IMGPAR current_mb_nr_r - 1;
			IMGPAR pUpMB_r = IMGPAR pLastMB_r;

			//C
			if(IMGPAR pUpRightMB_r) {
				IMGPAR pUpRightMB_r = NULL;
				IMGPAR mbAddrC =-1;
			}

			//D
			if (IMGPAR pLeftMB_r)
			{
				if (!IMGPAR pLeftMB_r->mb_field)
				{
					upleft_mb_addr = IMGPAR mbAddrA;
					IMGPAR pUpLeftMB_r = IMGPAR pLeftMB_r;
				}
				else
				{
					upleft_mb_addr = IMGPAR mbAddrA + 1;
					IMGPAR pUpLeftMB_r = IMGPAR pLeftMB_r + 1;
					currMB_r->mbStatusD = 5;
				}
			}
		}

		//Set addr A and B
		IMGPAR mbAddrA = left_mb_addr;
		IMGPAR mbAddrB = up_mb_addr;

		//Set addr D
		IMGPAR mbAddrD = upleft_mb_addr;

	}
}

void getCurrNeighbourInside_Luma PARGS4(int curr_mb_nr, int xN, int yN, PixelPos *pix)
{
	pix->mb_addr = curr_mb_nr;
	pix->pMB = currMB_d;

	if (IMGPAR MbaffFrameFlag){	
		if (!currMB_d->mb_field) //frame
		{
			pix->x = IMGPAR pix_x_d + xN;
			pix->y = IMGPAR pix_y_d + yN;
		}
		else //field
		{
			if (curr_mb_nr&1) //bottom field
			{
				pix->x = IMGPAR pix_x_d + xN;
				pix->y = IMGPAR pix_y_d + (yN<<1) - 15;
			}
			else //top field	
			{
				pix->x = IMGPAR pix_x_d + xN;
				pix->y = IMGPAR pix_y_d + (yN<<1);
			}
		}	
	} else {	
		pix->x = IMGPAR pix_x_d + xN;
		pix->y = IMGPAR pix_y_d + yN;
	}
}


void getCurrNeighbourA_Luma PARGS3(int xN, int yN, PixelPos *pix)
{
	int yM = yN;

	if(currMB_d->mbStatusA<0)
		pix->pMB=0;
	else
	{
		//just set to a non-NULL value
		pix->pMB = currMB_d;

		if (IMGPAR MbaffFrameFlag)
		{
			if(active_pps.constrained_intra_pred_flag)
			{
				int field = currMB_d->mbIntraA;				
				switch(currMB_d->mbStatusA)
				{
				case 1:				
				case 2:
					if( !((field>>(yN&1))&1) )
						pix->pMB=0;					
					break;
				case 3:
					yM <<= 1;
					if( !((field>>(int)(yN >= 8))&1) )
						pix->pMB=0;	
					break;
				case 4:
					yM <<= 1;
					yM -= 15;
					if( !((field>>(int)(yN >= 8))&1) )
						pix->pMB=0;	
					break;
				case 5:
					/*
					if(!(field&1))
					pix->pMB=0;
					*/
					yM <<= 1;
					break;
				case 6:
					/*
					if(!(field&1))
					pix->pMB=0;
					*/
					yM <<= 1;
					yM -= 15;
					break;
				default:
					/*
					if(!(field&1))
					pix->pMB=0;	
					*/
					break;
				}
			}
			else
			{
				switch(currMB_d->mbStatusA)
				{				
				case 3:
					yM <<= 1;					
					break;
				case 4:
					yM <<= 1;
					yM -= 15;					
					break;
				case 5:
					yM <<= 1;
					break;
				case 6:
					yM <<= 1;
					yM -= 15;
					break;
				default:
					break;
				}
			}
			pix->x = IMGPAR pix_x_d + xN;
			pix->y = (IMGPAR pix_y_rows + yM)&63;
		}
		else
		{
			pix->x = IMGPAR pix_x_d + xN;
			pix->y = (IMGPAR pix_y_rows + yM)&31;
		}
	}
}


void getCurrNeighbourB_Luma PARGS3(int xN, int yN, PixelPos *pix)
{
	int yM = yN;

	if(currMB_d->mbStatusB<0)
		pix->pMB=0;
	else
	{
		pix->pMB = currMB_d;

		if (IMGPAR MbaffFrameFlag) {

			if (currMB_d->mbStatusB == 1)
				yM *= 2;
			else if (currMB_d->mbStatusB == 2)
				yM -= 16;

			pix->x = IMGPAR pix_x_d + xN;
			pix->y = (IMGPAR pix_y_rows + yM)&63;
		}
		else
		{
			pix->x = IMGPAR pix_x_d + xN;
			pix->y = (IMGPAR pix_y_rows + yM)&31;
		}
	}
}

void getCurrNeighbourC_Luma PARGS3(int xN, int yN, PixelPos *pix)
{
	int yM = yN;

	if (currMB_d->mbStatusC<0)
		pix->pMB=0;
	else
	{
		//just set to a non-NULL value
		pix->pMB = currMB_d;

		if (IMGPAR MbaffFrameFlag){

			if (currMB_d->mbStatusC == 1)
				yM *= 2;
			else if (currMB_d->mbStatusC == 2)
			{
				yM *= 2;
				yM -= 15;
			}

			pix->x = IMGPAR pix_x_d + xN;
			pix->y = (IMGPAR pix_y_rows + yM)&63;
		}
		else
		{
			pix->x = IMGPAR pix_x_d + xN;
			pix->y = (IMGPAR pix_y_rows + yM)&31;
		}
	}
}

void getCurrNeighbourD_Luma PARGS3(int xN, int yN, PixelPos *pix)
{
	int yM = yN;

	if (currMB_d->mbStatusD<0)
		pix->pMB=0;
	else
	{
		//just set to a non-NULL value
		pix->pMB = currMB_d;

		if (IMGPAR MbaffFrameFlag) {
			if (currMB_d->mbStatusD == 1) {
				yM *= 2;
			} else if (currMB_d->mbStatusD == 2) {
				yM *= 2;
				yM -= 15;
			}
			pix->x = IMGPAR pix_x_d + xN;
			pix->y = (IMGPAR pix_y_rows + yM)&63;
		}
		else
		{
			pix->x = IMGPAR pix_x_d + xN;
			pix->y = (IMGPAR pix_y_rows + yM)&31;
		}
	}
}

void getCurrNeighbourInside_Chroma PARGS4(int curr_mb_nr, int xN, int yN, PixelPos *pix)
{
	pix->mb_addr = curr_mb_nr;
	pix->pMB = currMB_d;

	if (IMGPAR MbaffFrameFlag)
	{
		if (!currMB_d->mb_field) //frame
		{
			pix->x = IMGPAR pix_c_x_d + xN;
			pix->y = IMGPAR pix_c_y_d + yN;
		}
		else //field
		{
			if (curr_mb_nr&1) //bottom field
			{
				pix->x = IMGPAR pix_c_x_d + xN;
				pix->y = IMGPAR pix_c_y_d + (yN<<1) - 7;
			}
			else //top field	
			{
				pix->x = IMGPAR pix_c_x_d + xN;
				pix->y = IMGPAR pix_c_y_d + (yN<<1);
			}
		}	
	}
	else
	{
		pix->x = IMGPAR pix_c_x_d + xN;
		pix->y = IMGPAR pix_c_y_d + yN;
	}
}

void getCurrNeighbourA_Chroma PARGS3(int xN, int yN, PixelPos *pix)
{
	int yM = yN;

	if(currMB_d->mbStatusA<0)
		pix->pMB=0;
	else
	{
		//just set to a non-NULL value
		pix->pMB = currMB_d;

		if (IMGPAR MbaffFrameFlag)
		{
			if(active_pps.constrained_intra_pred_flag)
			{				
				int field = currMB_d->mbIntraA;
				switch(currMB_d->mbStatusA)
				{
				case 1:				
				case 2:
					if( !((field>>(yN&1))&1) )
						pix->pMB=0;					
					break;
				case 3:
					yM <<= 1;
					/*
					if( !((field>>(int)(yN >= 4))&1) )
					pix->pMB=0;	
					*/
					break;
				case 4:
					yM <<= 1;
					yM -= 7;
					/*
					if( !((field>>(int)(yN >= 4))&1) )
					pix->pMB=0;	
					*/
					break;
				case 5:
					/*
					if(!(field&1))
					pix->pMB=0;
					*/
					yM <<= 1;
					break;
				case 6:
					/*
					if(!(field&1))
					pix->pMB=0;
					*/
					yM <<= 1;
					yM -= 7;
					break;
				default:
					/*
					if(!(field&1))
					pix->pMB=0;	
					*/
					break;
				}
			}
			else
			{
				switch(currMB_d->mbStatusA)
				{				
				case 3:
					yM <<= 1;					
					break;
				case 4:
					yM <<= 1;
					yM -= 7;					
					break;
				case 5:
					yM <<= 1;
					break;
				case 6:
					yM <<= 1;
					yM -= 7;
					break;
				default:
					break;
				}
			}

			pix->x = IMGPAR pix_c_x_d + xN;
			pix->y = (IMGPAR pix_c_y_rows + yM)&31;
		}
		else
		{	
			pix->x = IMGPAR pix_c_x_d + xN;
			pix->y = (IMGPAR pix_c_y_rows + yM)&15;
		}
	}
}

void getCurrNeighbourB_Chroma PARGS3(int xN, int yN, PixelPos *pix)
{
	int yM = yN;

	if (currMB_d->mbStatusB<0)
		pix->pMB=0;
	else
	{
		//just set to a non-NULL value
		pix->pMB = currMB_d;

		if (IMGPAR MbaffFrameFlag)
		{
			if (currMB_d->mbStatusB == 1)
				yM *= 2;
			else if (currMB_d->mbStatusB == 2)
				yM -= 8;

			pix->x = IMGPAR pix_c_x_d + xN;
			pix->y = (IMGPAR pix_c_y_rows + yM)&31;
		}
		else
		{
			pix->x = IMGPAR pix_c_x_d + xN;
			pix->y = (IMGPAR pix_c_y_rows + yM)&15;
		}
	}
}

void getCurrNeighbourC_Chroma PARGS3(int xN, int yN, PixelPos *pix)
{
	int yM = yN;

	if (currMB_d->mbStatusC<0)
		pix->pMB=0;
	else
	{
		//just set to a non-NULL value
		pix->pMB = currMB_d;	

		if (IMGPAR MbaffFrameFlag)
		{
			if (currMB_d->mbStatusC == 1)
				yM *= 2;
			else if (currMB_d->mbStatusC == 2)
			{
				yM *= 2;
				yM -= 7;
			}

			pix->x = IMGPAR pix_c_x_d + xN;
			pix->y = (IMGPAR pix_c_y_rows + yM)&31;
		}
		else
		{
			pix->x = IMGPAR pix_c_x_d + xN;
			pix->y = (IMGPAR pix_c_y_rows + yM)&15;
		}
	}		
}

void getCurrNeighbourD_Chroma PARGS3(int xN, int yN, PixelPos *pix)
{
	int yM = yN;

	if (currMB_d->mbStatusD<0)
		pix->pMB=0;
	else
	{
		//just set to a non-NULL value
		pix->pMB = currMB_d;

		if (IMGPAR MbaffFrameFlag)
		{
			if (currMB_d->mbStatusD == 1)
				yM *= 2;
			else if (currMB_d->mbStatusD == 2)
			{
				yM *= 2;
				yM -= 7;
			}

			pix->x = IMGPAR pix_c_x_d + xN;
			pix->y = (IMGPAR pix_c_y_rows + yM)&31;
		}
		else
		{
			pix->x = IMGPAR pix_c_x_d + xN;
			pix->y = (IMGPAR pix_c_y_rows + yM)&15;
		}
	}
}


