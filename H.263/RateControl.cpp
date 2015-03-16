
#define OFFLINE_RATE_CONTROL//jwp
#include"stdafx.h"
#include "Global.h"


#ifdef OFFLINE_RATE_CONTROL

#include <stdio.h>
#include <math.h>

extern int FrameUpdateQP(int buf, int bits, int frames_left, int QP, int B,float seconds) 
{
  int newQP, dQP;
  float buf_rest, buf_rest_pic;

  buf_rest = seconds * B - (float)buf;

  newQP = QP;

  if (frames_left > 0) {
    buf_rest_pic = buf_rest / (float)frames_left;

    dQP = mmax(1,(int)(QP*0.1));

    if (bits > buf_rest_pic * 1.15) {
      newQP = mmin(31,QP+dQP);
    }
    else if (bits < buf_rest_pic / 1.15) {
      newQP = mmax(1,QP-dQP);
    }
    else {
    }
  }
  return newQP;
}

#else
/* 

   These routines are needed for the low-delay , variable frame rate,
   rate control specified in the TMN5 document

*/
#include <math.h>
/* rate control static variables */

static float B_prev;     /* number of bits spent for the previous frame */
static float B_target;   /* target number of bits/picture               */
static float global_adj; /* due to bits spent for the previous frame    */


void InitializeRateControl()
{
  B_prev = (float)0.0;
}

void UpdateRateControl(int bits)
{
  B_prev = (float)bits;
}

int InitializeQuantizer(int pict_type, float bit_rate, 
        float target_frame_rate, float QP_mean) 

/* QP_mean = mean quantizer parameter for the previous picture */
/* bitcount = current total bit count                          */
/* To calculate bitcount in coder.c, do something like this :  */
/* int bitcount;                                               */
/* AddBitsPicture(bits);                                       */
/* bitcount = bits->total;                                     */

{
  int newQP;

  if (pict_type == PCT_INTER) {

    B_target = bit_rate / target_frame_rate;

    /* compute picture buffer descrepency as of the previous picture */

    if (B_prev != 0.0) {
      global_adj = (B_prev - B_target) / (2*B_target);
    }
    else {
      global_adj = (float)0.0;
    }
    newQP = (int)(QP_mean * (1 + global_adj) + (float)0.5);
    newQP = mmax(1,mmin(31,newQP));  
  }
  else if (pict_type == PCT_INTRA) {
    newQP=4;
  }
  else  {
    newQP=4;
  }  
  return newQP;
}


/*********************************************************************
*   Name:          UpdateQuantizer
*
*
* Description: This function generates a new quantizer step size based
*                  on bits spent up until current macroblock and bits
*                  spent from the previous picture.  Note: this
*                  routine should be called at the beginning of each
*                  macroblock line as specified by TMN4. However, this
*                  can be done at any macroblock if so desired.
*
*  Input: current macroblock number (raster scan), mean quantizer
*  paramter for previous picture, bit rate, source frame rate,
*  hor. number of macroblocks, vertical number of macroblocks, total #
*  of bits used until now in the current picture.
*
*  Returns: Returns a new quantizer step size for the use of current
*  macroblock Note: adjustment to fit with 2-bit DQUANT should be done
*  in the calling program.
*
*  Side Effects:  
*
*  Date: 1/5/95    Author: Anurag Bist
*
**********************************************************************/


int UpdateQuantizer(int mb, float QP_mean, int pict_type, float bit_rate, 
                    int mb_width, int mb_height, int bitcount) 

/* mb = macroblock index number */
/* mb_width = macroblock numbers in width */
/* QP_mean = mean quantizer parameter for the previous picture */
/* bitcount = total # of bits used until now in the current picture */

{
  int newQP=4;
  float local_adj, descrepency, projection;
  
  if (pict_type == PCT_INTRA) {
    newQP = 4;
  }
  else if (pict_type == PCT_INTER) {
    /* compute expected buffer fullness */
    
    projection = mb * (B_target / (mb_width*mb_height));
    
    /* measure descrepency between current fullness and projection */
    descrepency= (bitcount - projection);
    
    /* scale */
    
    local_adj = 12 * descrepency / bit_rate;
    
    
    newQP = (int)(QP_mean * (1 + global_adj + local_adj) + 0.5);
    
  /* the update equation for newQP in TMN4 document section 3.7 */

  }
  else  {
   AfxMessageBox("Error (UpdateQuantizer): picture type unkown.\n");
  }
  
  newQP = mmax(1,mmin(31,newQP));  
  return newQP;
}

#endif
