/*************************************************************************/
/*                                                                       */
/*                            LD-CELP  G.728                             */
/*                                                                       */
/*    Low-Delay Code Excitation Linear Prediction speech compression.    */
/*                                                                       */
/*                 Copyright: Analog Devices, Inc., 1993                 */
/*                                                                       */
/*                         Author: Alex Zatsman.                         */
/*                                                                       */
/*  This program was written mostly for testing  Analog Devices' g21k C  */
/*  compiler for the  ADSP21000 architecture  family. While the program  */
/*  works  on  Sparc and ADSP21020, it  has  NOT  been  tested with the  */
/*  official test data from CCITT/ITU.                                   */
/*                                                                       */
/*  The program  is   distributed as  is,  WITHOUT ANY WARRANTY, EITHER  */
/*  EXPLICIT OR IMPLIED.                                                 */
/*                                                                       */
/*************************************************************************/

			/* Play back for ez-lab */
#include <signal.h>
#include <macros.h>
#include <def21020.h>
#include "21kflags.h"
#include "common.h"
#include "parm.h"

volatile int button_pushed;

extern void initx_in();
extern void initx_out();

void irq2_handler(int);
void record();
void wait_for_button();
void light_on(int);
void light_off(int);

extern void encoder();
extern void init_encoder();
extern void decoder();
extern void init_decoder();

void main ()
{
    interrupt(SIG_IRQ2, irq2_handler);
    mode2.b.irq2_edge   = 1;
    mode2.b.flg1_output = 1;
    mode2.b.flg2_output = 1;
    mode2.b.flg3_output = 1;
    light_on(1);
    light_off(2);
    light_off(3);

    wait_for_button();
    light_off(1);
    light_on(3);
    initx_in();
    encoder();
    light_off(3);

    while(1) {
	light_on(2);
	wait_for_button();
	light_off(2);
	initx_out();
	decoder();
    }
}

void wait_for_button()
{
    button_pushed = 0;
    imask.b.irq3i = 0;
    imask.b.sft0i = 0;
    while(!button_pushed);
}

void irq2_handler(int x)
{
    extern int volatile encoder_done;
    extern int volatile decoder_done;
    encoder_done  = 1;
    decoder_done  = 1;
    button_pushed = 1;
}

void light_on(int n)
{
    switch(n) {
      case 1: astat.b.flg1_value = 1; break;
      case 2: astat.b.flg2_value = 1; break;
      case 3: astat.b.flg3_value = 1; break;
      default:
	astat.b.flg1_value = 1;
	astat.b.flg2_value = 1;
	astat.b.flg3_value = 1;
    }
}

void light_off(int n)
{
    switch(n) {
      case 1: astat.b.flg1_value = 0; break;
      case 2: astat.b.flg2_value = 0; break;
      case 3: astat.b.flg3_value = 0; break;
      default:
	astat.b.flg1_value = 0;
	astat.b.flg2_value = 0;
	astat.b.flg3_value = 0;
    }
}

/******************************** Storing and Retrieving Indices. *******/

#define NUMX 25000
int ix_in  = 0;
int ix_out = 0;
int index_data[NUMX];
   /* scratch volatile to make sure that asm's are not moved */
volatile int pm scratch;


void initx_out() 
{
    ix_out = 0;
}

void initx_in() 
{
    ix_in  = 0;
}

void
put_index(int x)
{
    int wnum,fnum;
    wnum = ix_in>>2;
    if (wnum<NUMX) {
	fnum = ix_in&3;
	if (fnum) {
	    int new, old = index_data[wnum];
	    scratch=old;
	    asm volatile ("px=pm(_scratch);");
	    new = old | x<<(fnum*10-8);
	    asm volatile ("px2=%0;" : : "d" (new));
	}
	else {
	    /* this clobbers the rest 30 bits, but since the index is */
	    /* always written sequentially, this should not matter much */
	    int 
		x1 = x<<8 & 0xff00,
		x2 = (x>>8)&3;
	    asm volatile ("px1=%0;" : : "d" (x1));
	    asm volatile ("px2=%0;" : : "d" (x2));
	}
	asm volatile ("pm(_scratch)=px;");
	index_data[wnum] = scratch;
	ix_in++;
    }
    else {
	extern int volatile encoder_done;
	encoder_done=1;
    }
}

int get_index()
{
    int wnum, fnum, result;
    wnum = ix_out>>2;
    if (ix_out < ix_in && wnum < NUMX) {
	fnum = ix_out&3;
	if (fnum) {
	    result  = (index_data[wnum]>>(fnum*10-8))& 0x3ff;
	} else {
	    int x1,x2;
	    scratch=index_data[wnum];
	    asm volatile ("px=pm(_scratch);");
	    asm volatile ("%0=px1;" : "=d" (x1));
	    asm volatile ("%0=px2;" : "=d" (x2));
	    result = ((x2&3)<<8) | (x1>>8);
	}
	ix_out++;
	return result;
    }
    else {
	extern int decoder_done;
	decoder_done=1;
	return 0;
    }
}
