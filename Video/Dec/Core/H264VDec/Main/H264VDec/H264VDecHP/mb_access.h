
/*!
*************************************************************************************
* \file mb_access.h
*
* \brief
*    Functions for macroblock neighborhoods
*
* \author
*     Main contributors (see contributors.h for copyright, address and affiliation details)
*     - Karsten Sühring          <suehring@hhi.de>
*************************************************************************************
*/

#ifndef _MB_ACCESS_H_
#define _MB_ACCESS_H_

void CheckAvailabilityOfNeighbors_ABCD_even PARGS0();
void CheckAvailabilityOfNeighbors_ABCD_odd PARGS0();
//void CheckAvailabilityOfNeighbors_CD PARGS1(int curr_mb_nr);

void getCurrNeighbourInside_Luma PARGS4(int curr_mb_nr, int xN, int yN, PixelPos *pix);
void getCurrNeighbourA_Luma PARGS3(int xN, int yN, PixelPos *pix);
void getCurrNeighbourB_Luma PARGS3(int xN, int yN, PixelPos *pix);
void getCurrNeighbourC_Luma PARGS3(int xN, int yN, PixelPos *pix);
void getCurrNeighbourD_Luma PARGS3(int xN, int yN, PixelPos *pix);

void getCurrNeighbourInside_Chroma PARGS4(int curr_mb_nr, int xN, int yN, PixelPos *pix);
void getCurrNeighbourA_Chroma PARGS3(int xN, int yN, PixelPos *pix);
void getCurrNeighbourB_Chroma PARGS3(int xN, int yN, PixelPos *pix);
void getCurrNeighbourC_Chroma PARGS3(int xN, int yN, PixelPos *pix);
void getCurrNeighbourD_Chroma PARGS3(int xN, int yN, PixelPos *pix);

int  mb_is_available PARGS1(int mbAddr);

#endif
