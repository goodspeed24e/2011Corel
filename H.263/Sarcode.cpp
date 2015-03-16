// Sarcode.cpp: implementation of the CSarcode class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "codedecoder.h"
#include "Sarcode.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#define   q1    16384
#define   q2    32768
#define   q3    49152
#define   top   65535


CSarcode::CSarcode()
{
     zerorun=0;
}

CSarcode::~CSarcode()
{

}


int CSarcode::decode_a_symbol(int cumul_freq[])
{
  length = high - low + 1;
  cum = (-1 + (code_value - low + 1) * cumul_freq[0]) / length;
  for (sacindex = 1; cumul_freq[sacindex] > cum; sacindex++);
  high = low - 1 + (length * cumul_freq[sacindex-1]) / cumul_freq[0];
  low += (length * cumul_freq[sacindex]) / cumul_freq[0];

  for ( ; ; ) 
  {  
    if (high < q2) ;
    else if (low >= q2) 
	{
      code_value -= q2; 
      low -= q2; 
      high -= q2;
    }
    else if (low >= q1 && high < q3) 
	{
      code_value -= q1; 
      low -= q1; 
      high -= q1;
    }
    else 
	    break;
     
    low *= 2; 
    high = 2*high + 1;
    bit_out_psc_layer(); 
    code_value = 2*code_value + bit;
  }

  return (sacindex-1);

}
///////
void CSarcode::bit_out_psc_layer()
{
  if (m_getbits.showbits(17)!=1)
  { // check for startcode in Arithmetic Decoder FIFO 

    bit = m_getbits.getbits(1);
    if(zerorun > 13) 
	{	/* if number of consecutive zeros = 14 */	 
      if (!bit) 
	  {
	    AfxMessageBox("PSC/GBSC, Header Data, or Encoded Stream Error !");
	    zerorun = 1;		
      }
      else 
	  { /* if there is a 'stuffing bit present */
	    AfxMessageBox("Removing Startcode Emulation Prevention bit!");
    	bit = m_getbits.getbits(1); 	/* overwrite the last bit */	
	    zerorun = !bit;	  	/* zerorun=1 if bit is a '0' */
      }
    }
    else
	{ /* if consecutive zero's not exceeded 14 */
      if (!bit)
    	zerorun++;
      else
	    zerorun = 0;
    }

  } /* end of if !(showbits(17)) */

  else 
  {
    bit = 0;
    AfxMessageBox("Startcode Found:Finishing Arithmetic Decoding using 'Garbage bits'!");
  }

}

void CSarcode::decoder_reset()
{
 int i;
  zerorun = 0;			/* clear consecutive zero's counter */
  code_value = 0;
  low = 0;
  high = top;
  for (i = 1; i <= 16; i++)
  {
    bit_out_psc_layer(); 
    code_value = 2 * code_value + bit;
  }
}
