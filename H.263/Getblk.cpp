// Getblk.cpp: implementation of the CGetblk class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "codedecoder.h"
#include "Getblk.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define INDICES
#include "indices.h"

#define SACTABLES
#include "sactbls.h"
#define mmax(a, b)  	((a) > (b) ? (a) : (b))
#define mmin(a, b)  	((a) < (b) ? (a) : (b))

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
unsigned char zig_zag_scan[64]= 
{
  0,1,8,16,9,2,3,10,17,24,32,25,
  18,11,4,5,12,19,26,33,40,48,41,
  34,27,20,13,6,7,14,21,28,35,42,
  49,56,57,50,43,36,29,22,15,23,
  30,37,44,51,58,59,52,45,38,31,39,
  46,53,60,61,54,47,55,62,63
};  
int quiet,quant,bquant;
extern int trace;
 int fault;
 extern ldecode *ld;

CGetblk::CGetblk()
{

}

CGetblk::~CGetblk()
{

}

void CGetblk::getblock(int comp, int mode)
{
  int val, i, j, sign;
  unsigned int code;
  VLCtable *tab;
  short *bp;
  int run, last, level, QP;
  short *qval;

  bp = ld->block[comp];   

  // 解码交流系数  
  for (i=(mode==0); ; i++) 
  {
    code = m_getbits.showbits(12);
    if (code>=512)
      tab = &DCT3Dtab0[(code>>5)-16];
    else if (code>=128)
      tab = &DCT3Dtab1[(code>>2)-32];
    else if (code>=8)
      tab = &DCT3Dtab2[(code>>0)-8];
    else 
	{
      fault = 1;
      return;
    }
   m_getbits.flushbits(tab->len);

    run = (tab->code >> 4) & 255;
    level = tab->code & 15;
    last = (tab->code >> 12) & 1;

   if (tab->code==deESCAPE) //Escape
   { 
      last = m_getbits.getbits1();
      i += run = m_getbits.getbits(6);
      level = m_getbits.getbits(8);

      if ((sign = (level>=128)))
        val = 256 - level;
      else 
    	val = level;
    }
    else 
	{
      i+= run;
      val = level;
      sign = m_getbits.getbits(1);
    }

    if (i >= 64)
    {
      fault = 1;
      return;
    }
    //反Zig_zag扫描 
    j = zig_zag_scan[i];
    qval = &bp[j];
    if (comp >= 6)//确定量化因子
      QP = mmax (1, mmin( 31, ( bquant_tab[bquant] * quant ) >> 2 ));
    else 
      QP = quant;
      
    // 反量化
    if ((QP % 2) == 1)
      *qval = ( sign ? -(QP * (2* val+1))  : QP * (2* val+1) );
    else
      *qval = ( sign ? -(QP * (2* val+1)-1): QP * (2* val+1)-1 );

    if (last)  // That's the last coeff
      return;
  }

}

void CGetblk::get_sac_block(int comp, int ptype)
{
  int position=0;
  int TCOEF_index, symbol_word;
  int last=0, QP, i, j;
  short *qval, *bp;
  RunCoef DCTcoef;

  bp = ld->block[comp];

  i = (ptype==0);

  while (!last) {	/* while there are DCT coefficients remaining */
    position++;	/* coefficient counter relates to Coeff. model */
    TCOEF_index = DecodeTCoef(position, !ptype);

    if (TCOEF_index == ESCAPE_INDEX) { 	/* ESCAPE code encountered */
      DCTcoef = Decode_Escape_Char(!ptype, &last);
      if (trace)
	printf("ESC: ");
    }
    else {
      symbol_word = tcoeftab[TCOEF_index];

      DCTcoef = vlc_word_decode(symbol_word,&last);
    }

    if (trace) {
      printf("val: %d, run: %d, sign: %d, last: %d\n", 
	     DCTcoef.val, DCTcoef.run, DCTcoef.sign, last);
    }

    i += DCTcoef.run;

    j = zig_zag_scan[i];

    qval = &bp[j];

    i++;

    if (comp >= 6)
      QP = mmax (1, mmin( 31, ( bquant_tab[bquant] * quant ) >> 2 ));
    else 
      QP = quant;

    if ((QP % 2) == 1)
      *qval = ( (DCTcoef.sign) ? -(QP * (2* (DCTcoef.val)+1))  : 
		QP * (2* (DCTcoef.val)+1) );
    else
      *qval = ( (DCTcoef.sign) ? -(QP * (2* (DCTcoef.val)+1)-1): 
		QP * (2* (DCTcoef.val)+1)-1 );
	
  }	
  return;
}

RunCoef CGetblk::vlc_word_decode(int symbol_word, int *last)
{
  int sign_index;
  RunCoef DCTcoef;

  *last = (symbol_word >> 12) & 01;
 
  DCTcoef.run = (symbol_word >> 4) & 255; 

  DCTcoef.val = (symbol_word) & 15;

  sign_index = m_sacode.decode_a_symbol(cumf_SIGN);	

  DCTcoef.sign = signtab[sign_index];
  return (DCTcoef);
}

RunCoef CGetblk::Decode_Escape_Char(int intra, int *last)
{
  int last_index, run, run_index, level, level_index;
  RunCoef DCTcoef;

  if (intra) {
    last_index = m_sacode.decode_a_symbol(cumf_LAST_intra);
    *last = last_intratab[last_index];
  }
  else {
    last_index = m_sacode.decode_a_symbol(cumf_LAST);
    *last = lasttab[last_index];
  }

  if (intra) 
    run_index = m_sacode.decode_a_symbol(cumf_RUN_intra);
  else
    run_index = m_sacode.decode_a_symbol(cumf_RUN);

  run = runtab[run_index];

  /*$if (mrun) run|=64;$*/

  DCTcoef.run = run;

  if (intra)
    level_index = m_sacode.decode_a_symbol(cumf_LEVEL_intra);
  else
    level_index = m_sacode.decode_a_symbol(cumf_LEVEL);

  if (trace)
    printf("level_idx: %d ",level_index);

  level = leveltab[level_index];

  if (level >128) 
    level -=256;

  if (level < 0) {
    DCTcoef.sign = 1;
    DCTcoef.val = abs(level);
  }

  else {	
    DCTcoef.sign = 0;
    DCTcoef.val = level;
  }

  return (DCTcoef);
	

}

int CGetblk::DecodeTCoef(int position, int intra)
{
  int index;
  switch (position) {
  case 1:
    {
      if (intra) 
	index = m_sacode.decode_a_symbol(cumf_TCOEF1_intra);
      else 
	index = m_sacode.decode_a_symbol(cumf_TCOEF1); 
      break; 
    }
  case 2:
    {
      if (intra) 
	index = m_sacode.decode_a_symbol(cumf_TCOEF2_intra);
      else
	index = m_sacode.decode_a_symbol(cumf_TCOEF2);
      break; 
    }
  case 3:
    {
      if (intra) 
	index = m_sacode.decode_a_symbol(cumf_TCOEF3_intra);
      else
	index = m_sacode.decode_a_symbol(cumf_TCOEF3);
      break; 
    }
  default: 
    {
      if (intra) 
	index = m_sacode.decode_a_symbol(cumf_TCOEFr_intra);
      else
	index = m_sacode.decode_a_symbol(cumf_TCOEFr);
      break; 
    }
  }
  return (index);
}
