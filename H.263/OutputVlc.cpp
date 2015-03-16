#include "stdafx.h"

#include "OutputVlc.h"

#include <math.h>
#include <assert.h>
#include <direct.h>

extern CFile streamfile; /* the only global var we need here */

/* private data */
static unsigned char outbfr=0;//
static int outcnt;            //输出位数
static int bytecnt;           //字节数


/****************************************************************************
*  extern int put_mv (int)                  -输出mv
*  extern int put_cbpcm_intra (int, int)
*  extern int put_cbpcm_inter (int, int)
*  extern int put_cbpy (int, int)
*  extern int put_coeff (int, int, int)
*
*  extern void initbits()
*  extern void putbits (int, int)
*  extern int alignbits ()
*  extern int bitcount()
*  
*****************************************************************************/
int put_mv (int mvint)
{
  int sign = 0;
  int absmv;

  if (mvint >= 32) 
  {
    absmv = -mvint + 64;
    sign = 1;
  }
  else
    absmv = mvint;
  
  putbits (mvtab[absmv].len, mvtab[absmv].code);

  if (mvint != 0) 
  {
    putbits (1, sign);
    return mvtab[absmv].len + 1;
  }
  else
    return mvtab[absmv].len;
}

int put_cbpcm_intra (int cbpc, int mode)
{
  int index;

  index = ((mode & 7) >> 1) | ((cbpc & 3) << 2);

  putbits (cbpcm_intra_tab[index].len, cbpcm_intra_tab[index].code);
  
  return cbpcm_intra_tab[index].len;
}

int put_cbpcm_inter (int cbpc, int mode)
{
  int index;

  index = (mode & 7) | ((cbpc & 3) << 3);

  putbits (cbpcm_inter_tab[index].len, cbpcm_inter_tab[index].code);
  
  return cbpcm_inter_tab[index].len;
}


int put_cbpy (int cbp, int mode)
{
  int index;

  index = cbp >> 2;

  if (mode < 3)
    index ^= 15;
  
  putbits (cbpy_tab[index].len,	cbpy_tab[index].code);
  
  return cbpy_tab[index].len;
}

int put_coeff (int last, int run, int level)
{
  int length = 0;
 
   assert (level > 0 && level < 128);
  if (last == 0) {
    if (run < 2 && level < 13 ) {
      putbits (coeff_tab0[run][level-1].len,
	       coeff_tab0[run][level-1].code);

      length = coeff_tab0[run][level-1].len;
    }
    else if (run > 1 && run < 27 && level < 5) {
      putbits (coeff_tab1[run-2][level-1].len,
	       coeff_tab1[run-2][level-1].code);
      
      length = coeff_tab1[run-2][level-1].len;
    }
  }
  else if (last == 1) {
    if (run < 2 && level < 4) {
      putbits (coeff_tab2[run][level-1].len,
	       coeff_tab2[run][level-1].code);
      
      length = coeff_tab2[run][level-1].len;
    }
    else if (run > 1 && run < 42 && level == 1) {
      putbits (coeff_tab3[run-2].len,
	       coeff_tab3[run-2].code);
      
      length = coeff_tab3[run-2].len;
    }
  }
 
  return length;
}


 void putbits(int n, int value)
 {
	 int i;
	 unsigned int mask=0;
	 mask = 1 << (n-1); /* selects first (leftmost) bit */
     for (i=0; i<n; i++) {
     outbfr <<= 1;
     if (value & mask)
     outbfr|= 1;
     mask >>= 1; /* select next bit */
     outcnt--;
	 if (outcnt==0) /* 8 bit buffer full */ 
	 {
		 streamfile.Write(&outbfr,1);
         outcnt = 8;
         outbfr = 0; 
         bytecnt++;
	 }
	 }
 }
 

 void initbits()
 {
	 outcnt = 8;
     bytecnt = 0;
 }
/************************************************************************
* zero bit stuffing to next byte boundary  
************************************************************************/
 int alignbits ()
 {
	int ret_value;
    if (outcnt!=8) {
    ret_value = outcnt;	/* outcnt is reset in call to putbits () */
    putbits (outcnt, 0);
    return ret_value;
  }
  else
    return 0;
 }
/************************************************************************
* return total number of generated bits 
************************************************************************/
int bitcount(){ return 8*bytecnt + (8-outcnt);}